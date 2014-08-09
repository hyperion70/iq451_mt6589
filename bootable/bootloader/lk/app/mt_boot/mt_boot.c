#include <app.h>
#include <debug.h>
#include <arch/arm.h>
#include <dev/udc.h>
#include <reg.h>
#include <string.h>
#include <stdlib.h>
#include <kernel/thread.h>
#include <arch/ops.h>

#include <target.h>
#include <platform.h>

#include <platform/mt_reg_base.h>
#include <platform/boot_mode.h>
#include <mt_partition.h>
#include <platform/mt_disp_drv.h>
#include <platform/env.h>
#include <target/cust_usb.h>
#ifdef MTK_KERNEL_POWER_OFF_CHARGING
#include "platform/mtk_wdt.h"
extern int kernel_charging_boot(void);
extern int pmic_detect_powerkey(void);
extern void mt6575_power_off(void);
extern void mt65xx_backlight_off(void);
#endif

char g_CMDLINE [200] = COMMANDLINE_TO_KERNEL;

/* Please define SN_BUF_LEN in cust_usb.h */
#ifndef SN_BUF_LEN
#define SN_BUF_LEN	13	/* fastboot use 13 bytes as default, max is 19 */
#endif

#ifdef CONFIG_MTK_USB_UNIQUE_SERIAL
#define SERIALNO_LEN	38	/* from preloader */
char sn_buf[SN_BUF_LEN] = "";	/* will read from EFUSE_CTR_BASE */
#else
char sn_buf[SN_BUF_LEN] = FASTBOOT_DEVNAME;
#endif

static struct udc_device surf_udc_device = {
	.vendor_id	= USB_VENDORID,
	.product_id	= USB_PRODUCTID,
	.version_id	= USB_VERSIONID,
	.manufacturer	= USB_MANUFACTURER,
	.product	= USB_PRODUCTID,
};

void msg_header_error(char *img_name)
{
	printf ("[MBOOT] Load '%s' partition Error\n", img_name);
	printf("\n*******************************************************\n");
	printf("ERROR.ERROR.ERROR.ERROR.ERROR.ERROR.ERROR.ERROR.ERROR\n");
	printf("*******************************************************\n");
	printf("> If you use NAND boot\n");
	printf("> (1) %s is wrong !!!! \n", img_name);        
	printf("> (2) please make sure the image you've downloaded is correct\n");
	printf("\n> If you use MSDC boot\n");
	printf("> (1) %s is not founded in SD card !!!! \n",img_name);       
	printf("> (2) please make sure the image is put in SD card\n");
	printf("*******************************************************\n");        
	printf("ERROR.ERROR.ERROR.ERROR.ERROR.ERROR.ERROR.ERROR.ERROR\n");
	printf("*******************************************************\n");        
	while(1);
}

void msg_img_error(char *img_name)
{
	printf ("[MBOOT] Load '%s' partition Error\n", img_name);
	printf("\n*******************************************************\n");
	printf("ERROR.ERROR.ERROR.ERROR.ERROR.ERROR.ERROR.ERROR.ERROR\n");
	printf("*******************************************************\n");
	printf("> Please check kernel and rootfs in %s are both correct.\n",img_name);
	printf("*******************************************************\n");        
	printf("ERROR.ERROR.ERROR.ERROR.ERROR.ERROR.ERROR.ERROR.ERROR\n");
	printf("*******************************************************\n");        
	while(1);
}

//*********
//* Notice : it's kernel start addr (and not include any debug header)
extern unsigned int g_kmem_off;

//*********
//* Notice : it's rootfs start addr (and not include any debug header)
extern unsigned int g_rmem_off;
extern unsigned int g_rimg_sz;
extern int g_nr_bank;
extern int g_rank_size[4];
extern unsigned int boot_time;
extern BOOT_ARGUMENT *g_boot_arg;
#ifdef MTK_KERNEL_POWER_OFF_CHARGING
extern bool g_boot_reason_change;
#endif
extern int has_set_p2u;

static void check_hibernation(char *cmdline)
{
    int hibboot = 0;

    hibboot = get_env("hibboot") == NULL ? 0 : atoi(get_env("hibboot"));

    if (g_boot_mode == RECOVERY_BOOT)
        goto SKIP_HIB_BOOT;
    if (g_boot_mode == FACTORY_BOOT)
        goto SKIP_HIB_BOOT;
    if (g_boot_mode == ALARM_BOOT)
        goto SKIP_HIB_BOOT;
    if (g_boot_arg->boot_reason == BR_RTC)
        goto SKIP_HIB_BOOT;
#ifdef MTK_KERNEL_POWER_OFF_CHARGING
    if (g_boot_mode == KERNEL_POWER_OFF_CHARGING_BOOT || g_boot_mode == LOW_POWER_OFF_CHARGING_BOOT)
        goto SKIP_HIB_BOOT;
#endif

    if(get_env("resume") != NULL) {
        if (1 == hibboot) {
            sprintf(cmdline,"%s%s%s",cmdline," resume=", get_env("resume"));
            //sprintf(cmdline, "%s%s", cmdline, " no_console_suspend");
        } else if (0 != hibboot)
            printf("resume = %s but hibboot = %s\n", get_env("resume"), get_env("hibboot"));
    } else {
        printf("resume = NULL \n");
    }

    return;

SKIP_HIB_BOOT:
    if (hibboot != 0)
        if (set_env("hibboot", "0") != 0)
            printf("lk_env hibboot set failed!!!\n");
    if (get_env("resume") != NULL)
        if (set_env("resume", '\0') != 0)
            printf("lk_evn resume set resume failed!!!\n");
}

void boot_linux(void *kernel, unsigned *tags,
		char *cmdline, unsigned machtype,
		void *ramdisk, unsigned ramdisk_size)
{
    unsigned int i;
    unsigned *ptr = tags;
    unsigned pcount = 0;
    void (*entry)(unsigned,unsigned,unsigned*) = kernel;
    struct ptable *ptable;
    int pause_at_bootup = 0;
    unsigned int lk_t = 0;
    unsigned int pl_t = 0;
    unsigned int boot_reason = 0;			

	/* CORE */
    *ptr++ = 2;
    *ptr++ = 0x54410001;	

    ptr = target_atag_boot(ptr);
    ptr = target_atag_mem(ptr);

    if(g_boot_mode == META_BOOT || g_boot_mode == ADVMETA_BOOT || g_boot_mode == ATE_FACTORY_BOOT || g_boot_mode == FACTORY_BOOT)
    {
      ptr = target_atag_meta(ptr);
    }

    ptr = target_atag_devinfo_data(ptr);
    
    /*Append pre-loader boot time to kernel command line*/
    pl_t = g_boot_arg->boot_time;
    sprintf(cmdline, "%s%s%d", cmdline, " pl_t=", pl_t);
  
    /*Append lk boot time to kernel command line*/
    lk_t = ((unsigned int)get_timer(boot_time));
    sprintf(cmdline, "%s%s%d", cmdline, " lk_t=", lk_t);
#ifdef LK_PROFILING
    printf("[PROFILE] ------- boot_time takes %d ms -------- \n", lk_t);
#endif
    if (!has_set_p2u) {
#ifdef USER_BUILD
        sprintf(cmdline,"%s%s",cmdline," printk.disable_uart=1");
#else
        sprintf(cmdline,"%s%s",cmdline," printk.disable_uart=0");
#endif 
    }
	/*Append pre-loader boot reason to kernel command line*/
#ifdef MTK_KERNEL_POWER_OFF_CHARGING	
	if (g_boot_reason_change) {
		boot_reason = 4;
	}
	else
#endif 
	{
		boot_reason = g_boot_arg->boot_reason;
	}
    sprintf(cmdline, "%s%s%d", cmdline, " boot_reason=", boot_reason);
    check_hibernation(cmdline);
    ptr = target_atag_commmandline(ptr, cmdline);
    ptr = target_atag_initrd(ptr, ramdisk, ramdisk_size);
    ptr = target_atag_videolfb(ptr);

	/* END */
    *ptr++ = 0;
    *ptr++ = 0;

    printf("booting linux @ %p, ramdisk @ %p (%d)\n",
		kernel, ramdisk, ramdisk_size);  

    enter_critical_section();
    /* do any platform specific cleanup before kernel entry */
    platform_uninit();
#ifdef HAVE_CACHE_PL310	
    l2_disable();
#endif

    arch_disable_cache(UCACHE);
    arch_disable_mmu();    

    
#ifdef MTK_KERNEL_POWER_OFF_CHARGING
	/*Prevent the system jumps to Kernel if we unplugged Charger/USB before*/
	if(kernel_charging_boot() == -1)
	{
		printf("[%s] Unplugged Usb/Charger in Kernel Charging Mode Before Jumping to Kernel, Power Off\n", __func__);
#ifndef NO_POWER_OFF
		mt6575_power_off();
#endif

	}
	if(kernel_charging_boot() == 1)
	{
		if(pmic_detect_powerkey())
		{
			printf("[%s] PowerKey Pressed in Kernel Charging Mode Before Jumping to Kernel, Reboot Os\n", __func__);
			mt65xx_backlight_off();
			mt_disp_power(0);
			mtk_arch_reset(1);
		}
	}
#endif
    printf("DRAM Rank :%d\n", g_nr_bank);
    for(i = 0; i < g_nr_bank; i++) {
         printf("DRAM Rank[%d] Size = 0x%x\n", i, g_rank_size[i]);
    }
    printf("cmdline: %s\n", cmdline);
    printf("lk boot time = %d ms\n", lk_t);
    printf("lk boot mode = %d\n", g_boot_mode);
    printf("lk finished --> jump to linux kernel\n\n");
    entry(0, machtype, tags);
}

int boot_linux_from_storage(void)
{
    int ret;
    char *commanline = g_CMDLINE;
    int strlen;
#ifdef LK_PROFILING
    unsigned int time_load_recovery;
    unsigned int time_load_bootimg;
    unsigned int time_load_factory;
    time_load_recovery = get_timer(0);
    time_load_bootimg = get_timer(0);
#endif

#if 1

    switch(g_boot_mode)
    {
        case NORMAL_BOOT:
        case META_BOOT:
        case ADVMETA_BOOT:
        case SW_REBOOT:  
        case ALARM_BOOT:
#ifdef MTK_KERNEL_POWER_OFF_CHARGING
		case KERNEL_POWER_OFF_CHARGING_BOOT:
		case LOW_POWER_OFF_CHARGING_BOOT:	
#endif        	
#if defined(CFG_NAND_BOOT)
            strlen += sprintf(commandline, "%s%s%x%s%x", 
                commandline, NAND_MANF_CMDLINE, nand_flash_man_code, NAND_DEV_CMDLINE, nand_flash_dev_id);
#endif
            ret = mboot_android_load_bootimg_hdr(PART_BOOTIMG, CFG_BOOTIMG_LOAD_ADDR);
            if (ret < 0) {
                msg_header_error("Android Boot Image");
            }
            
            ret = mboot_android_load_bootimg(PART_BOOTIMG, CFG_BOOTIMG_LOAD_ADDR);            
            if (ret < 0) {
                msg_img_error("Android Boot Image");
            }
#ifdef LK_PROFILING
            printf("[PROFILE] ------- load boot.img takes %d ms -------- \n", get_timer(time_load_bootimg));
#endif
        break;    
    
        case RECOVERY_BOOT:
            ret = mboot_android_load_recoveryimg_hdr(PART_RECOVERY, CFG_BOOTIMG_LOAD_ADDR);
            if (ret < 0) {
                msg_header_error("Android Recovery Image");
            }
           
            ret = mboot_android_load_recoveryimg(PART_RECOVERY, CFG_BOOTIMG_LOAD_ADDR);            
            if (ret < 0) {
                msg_img_error("Android Recovery Image");
            }
#ifdef LK_PROFILING
            printf("[PROFILE] ------- load recovery.img takes %d ms -------- \n", get_timer(time_load_recovery));
#endif
        break;

        case FACTORY_BOOT:
        case ATE_FACTORY_BOOT:  
#if defined(CFG_NAND_BOOT)
            strlen += sprintf(commandline, "%s%s%x%s%x", 
                commandline, NAND_MANF_CMDLINE, nand_flash_man_code, NAND_DEV_CMDLINE, nand_flash_dev_id);
#endif
            ret = mboot_android_load_factoryimg_hdr(CFG_FACTORY_NAME, CFG_BOOTIMG_LOAD_ADDR);
            if (ret < 0) {
                printf("factory image doesn't exist in SD card\n"); 
                
                ret = mboot_android_load_bootimg_hdr(PART_BOOTIMG, CFG_BOOTIMG_LOAD_ADDR);            
                if (ret < 0) {
                    msg_header_error("Android Boot Image");
                }
                
                ret = mboot_android_load_bootimg(PART_BOOTIMG, CFG_BOOTIMG_LOAD_ADDR);            
                if (ret < 0) {
                    msg_img_error("Android Boot Image");
	        }          
            } else {	
                ret = mboot_android_load_factoryimg(CFG_FACTORY_NAME, CFG_BOOTIMG_LOAD_ADDR);            
                if (ret < 0) {
                    msg_img_error("Android Factory Image");
                }
            }
#ifdef LK_PROFILING
            printf("[PROFILE] ------- load factory.img takes %d ms -------- \n", get_timer(time_load_factory));
#endif
         break;
    }


    /* relocate rootfs (ignore rootfs header) */
    memcpy((char *)CFG_RAMDISK_LOAD_ADDR, (char *)(g_rmem_off), g_rimg_sz);
    g_rmem_off = CFG_RAMDISK_LOAD_ADDR;

#endif

    custom_port_in_kernel(g_boot_mode, commanline);
    strlen += sprintf(commanline, "%s lcm=%1d-%s", commanline, DISP_IsLcmFound(), mt_disp_get_lcm_id());
    strlen += sprintf(commanline, "%s fps=%1d", commanline, mt_disp_get_lcd_time());
 

    boot_linux((void *)CFG_BOOTIMG_LOAD_ADDR, (unsigned *)CFG_BOOTARGS_ADDR,
		   (char *)commanline, board_machtype(),
		   (void *)CFG_RAMDISK_LOAD_ADDR, g_rimg_sz);
  
}

#ifdef CONFIG_MTK_USB_UNIQUE_SERIAL
static char udc_chr[32] = {"ABCDEFGHIJKLMNOPQRSTUVWSYZ456789"};
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

int get_serial(u64 hwkey, u32 chipid, char ser[SERIALNO_LEN])
{
	u16 hashkey[4];
	int idx, ser_idx;
	u32 digit, id;
	u64 tmp = hwkey;

	memset(ser, 0x00, SERIALNO_LEN);

	/* split to 4 key with 16-bit width each */
	tmp = hwkey;
	for (idx = 0; idx < ARRAY_SIZE(hashkey); idx++) {
		hashkey[idx] = (u16)(tmp & 0xffff);
		tmp >>= 16;
	}

	/* hash the key with chip id */
	id = chipid;
	for (idx = 0; idx < ARRAY_SIZE(hashkey); idx++) {
		digit = (id % 10);
		hashkey[idx] = (hashkey[idx] >> digit) | (hashkey[idx] << (16-digit));
		id = (id / 10);
	}

	/* generate serail using hashkey */
	ser_idx = 0;
	for (idx = 0; idx < ARRAY_SIZE(hashkey); idx++) {
		ser[ser_idx++] = (hashkey[idx] & 0x001f);
		ser[ser_idx++] = (hashkey[idx] & 0x00f8) >> 3;
		ser[ser_idx++] = (hashkey[idx] & 0x1f00) >> 8;
		ser[ser_idx++] = (hashkey[idx] & 0xf800) >> 11;
	}
	for (idx = 0; idx < ser_idx; idx++)
		ser[idx] = udc_chr[(int)ser[idx]];
	ser[ser_idx] = 0x00;
	return 0;
}
#endif /* CONFIG_MTK_USB_UNIQUE_SERIAL */

void mt_boot_init(const struct app_descriptor *app)
{
	unsigned usb_init = 0;
	unsigned sz = 0;
#ifdef CONFIG_MTK_USB_UNIQUE_SERIAL
	u64 key;
	u8 serial_num[SERIALNO_LEN];
	unsigned clk_efuse;
#endif

#ifdef CONFIG_MTK_USB_UNIQUE_SERIAL
	/* EFUSE clock enable to read */
	clk_efuse = readl(PDN_CTR); 
	writel(PDN_CTR, (clk_efuse & 0x7FFF));

	/* serial string adding */

	key = readl(SERIAL_KEY_HI);
	key = (key << 32) | readl(SERIAL_KEY_LO);

	if (key != 0)
		get_serial(key, 6575, serial_num);
	else
		memcpy(serial_num, "MTK0123456789ABCDEF", SN_BUF_LEN);
	/* copy serial from serial_num to sn_buf */
	memcpy(sn_buf, serial_num, SN_BUF_LEN);
#endif
	surf_udc_device.serialno = sn_buf;

#ifdef MTK_EMMC_SUPPORT
    sec_func_init(1);
#else
    sec_func_init(0);
#endif

    sec_boot_check();

	if (g_boot_mode == FASTBOOT)
		goto fastboot;

	/* Will not return */
	boot_linux_from_storage();

fastboot:
	target_fastboot_init();
	if(!usb_init)
		/*Hong-Rong: wait for porting*/
		udc_init(&surf_udc_device);	

	mt_part_dump();
/*test*/
#if 0

{
	char buf[2048];
	char buf_t[2048];
	int i;
	part_t *part;
	part_dev_t *dev = mt_part_get_device();
	u64 start_addr;
	bool ret = true;
	memset(buf,0x00,2048);
	memset(buf_t,0x00,2048);

	part = mt_part_get_partition(PART_LOGO);
	start_addr = (u64)part->startblk*BLK_SIZE;
	printf("---partition test -----%s %llx\n",PART_LOGO,start_addr);
	
	
	dev->write(dev,buf,start_addr,2048);
	for(i=0;i<2048;i++){
		buf[i]= i;
	}
	dev->write(dev,buf,start_addr+4,1024);
	dev->read(dev,start_addr+4,buf_t,1024);

	for(i=0;i<1024;i++){
		if(buf[i]!=buf_t[i]){
			printf("compare error. s=%x,d=%x\n",buf[i],buf_t[i]);
			ret = false;
		}
		
	}
	if(ret == true){
		printf("---partition test sucess\n-----");
	}

	
}
#endif
/*test*/
	sz = target_get_max_flash_size();
	fastboot_init(target_get_scratch_address(), sz);
	udc_start();
  
}


APP_START(mt_boot)
	.init = mt_boot_init,
APP_END
