 
#include "tpd.h"
#include <linux/interrupt.h>
#include <cust_eint.h>
#include <linux/i2c.h>
#include <linux/sched.h>
#include <linux/time.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/rtpm_prio.h>
#include <linux/wait.h>

#include <linux/dma-mapping.h>

#if 1 // defined(MT6577)
#include <mach/mt_pm_ldo.h>
#include <mach/mt_typedefs.h>
#include <mach/mt_boot.h>
#elif defined(MT6575)
#include <mach/mt6575_pm_ldo.h>
#include <mach/mt6575_typedefs.h>
#include <mach/mt6575_boot.h>
#elif defined(CONFIG_ARCH_MT6573)
#include <mach/mt6573_boot.h>
#endif

#include <linux/miscdevice.h>
#include <asm/uaccess.h>

#include "tpd_custom_ft5x06.h"

 extern void mt65xx_eint_unmask(unsigned int line);
 extern void mt65xx_eint_mask(unsigned int line);

#define ISP_FLASH_SIZE	0x8000 //32KB

#define FT5X06_FIRMWAIR_VERSION_D

/*==========================================*/
static tinno_ts_data *g_pts = NULL;

//LINE<JIRA_ID><DATE20130320><add charger flag>zenghaihui
extern kal_uint8 g_need_refresh_tp_flag;

//LINE<JIRA_ID><DATE20130418><jiemian V12 tp upgrade problem>zenghaihui
#ifdef FT6X06_DOWNLOAD
//#include "ft6x06_download_lib.h"
//#include "ft5x16_download_lib.h"
static struct i2c_client *g_i2c_client = NULL;
static int g_tpd_fw_vendor = 0;
static int g_tpd_tpid = 0xff;
extern int tpd_fw_version;
#endif

static int fts_i2c_read_a_byte_data(struct i2c_client *client, char reg)
{
	int ret;
	uint8_t iic_data;
	ret = i2c_smbus_read_i2c_block_data(client, reg, 1, &iic_data);
	if (iic_data < 0 || ret < 0){
		CTP_DBG("%s: i2c error, ret=%d\n", __func__, ret);
		return -1;
	}
	return (int)iic_data;
}

static inline int _lock(atomic_t *excl)
{
	if (atomic_inc_return(excl) == 1) {
		return 0;
	} else {
		atomic_dec(excl);
		return -1;
	}
}

static inline void _unlock(atomic_t *excl)
{
	atomic_dec(excl);
}

static ssize_t fts_isp_read(struct file *file, char __user *buf, size_t count, loff_t *offset)
{	
	//ret = copy_to_user(buf,&acc, sizeof(acc));
	CTP_DBG("");
	return -EIO;
}

static DECLARE_WAIT_QUEUE_HEAD(waiter_write);
static volatile	int write_flag;

 static int isp_thread(void *para)
 {
	int rc = 0;
	struct sched_param param = { .sched_priority = RTPM_PRIO_TPD };
	tinno_ts_data *ts = (tinno_ts_data *)para;
	sched_setscheduler(current, SCHED_RR, &param);
	set_current_state(TASK_RUNNING);
		
	rc = fts_i2c_write_block(ts->isp_pBuffer, ts->isp_code_count);
	if (rc < ts->isp_code_count) {
		CTP_DBG("i2c_transfer failed(%d)", rc);
		ts->isp_code_count = -EAGAIN;
	} else{
		ts->isp_code_count = rc;
	}
	write_flag = 1;
	wake_up_interruptible(&waiter_write);
		
	return 0;
 }

static ssize_t fts_isp_write(struct file *file, const char __user *buf, size_t count, loff_t *offset)
{
	int rc = 0;
	tinno_ts_data *ts = file->private_data;
	char __user *start = buf;
	CTP_DBG("count = %d, offset=%d", count, (int)(*offset));
	
	if ( count > ISP_FLASH_SIZE ){
		CTP_DBG("isp code is too long.");
		return -EDOM;
	}

	if ( copy_from_user(ts->isp_pBuffer, start, count) ){
		CTP_DBG("copy failed(%d)", rc);
		return -EACCES;
	}

	ts->isp_code_count = count;
	write_flag = 0;
	ts->thread_isp= kthread_run(isp_thread, ts, TPD_DEVICE"-ISP");
	if (IS_ERR(ts->thread_isp)){ 
		rc = PTR_ERR(ts->thread_isp);
		CTP_DBG(" failed to create kernel thread: %d\n", rc);
		return rc;
	} 

	//block user thread
	wait_event_interruptible(waiter_write, write_flag!=0);
	
	return ts->isp_code_count;
}

static int fts_isp_open(struct inode *inode, struct file *file)
{
	CTP_DBG("try to open isp.");

	if ( atomic_read( &g_pts->ts_sleepState ) ){
		CTP_DBG("TP is in sleep state, please try again latter.");
		return -EAGAIN;
	}

	if (_lock(&g_pts->isp_opened)){
		CTP_DBG("isp is already opened.");
		return -EBUSY;
	}
		
	file->private_data = g_pts;

	g_pts->isp_pBuffer = (uint8_t *)kmalloc(ISP_FLASH_SIZE, GFP_KERNEL);
	if ( NULL == g_pts->isp_pBuffer ){
		_unlock ( &g_pts->isp_opened );
		CTP_DBG("no memory for isp.");
		return -ENOMEM;
	}
	
//	mt65xx_eint_mask(CUST_EINT_TOUCH_PANEL_NUM);
	
//	ft5x06_complete_unfinished_event();
	
#ifdef CONFIG_TOUCHSCREEN_FT5X05_DISABLE_KEY_WHEN_SLIDE
		fts_5x06_key_cancel();
#endif

	wake_lock(&g_pts->wake_lock);

	CTP_DBG("isp open success.");
	return 0;
}

static int fts_isp_close(struct inode *inode, struct file *file)
{
	tinno_ts_data *ts = file->private_data;
	
	CTP_DBG("try to close isp.");
	
	if ( !atomic_read( &g_pts->isp_opened ) ){
		CTP_DBG("no opened isp.");
		return -ENODEV;
	}
	
	kfree(ts->isp_pBuffer);
	ts->isp_pBuffer = NULL;
	
	file->private_data = NULL;
	
	_unlock ( &ts->isp_opened );
	
	wake_unlock(&ts->wake_lock);
	
//	mt65xx_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM);  
	
	CTP_DBG("close isp success!");
	return 0;
}

static int fts_switch_to_update(tinno_ts_data *ts)
{
	int ret = 0, i=0;
	uint8_t arrCommand[] = {0x55, 0xaa};
	
	CTP_DBG("");
	
	
	/*write 0xaa to register 0xfc*/
	ret = fts_write_reg(0xFC, 0xAA);
	if (ret < 0) {
		CTP_DBG("write 0xaa to register 0xfc failed");
		goto err;
	}
	msleep(50);

	/*write 0x55 to register 0xfc*/
	ret = fts_write_reg(0xFC, 0x55);
	if (ret < 0) {
		CTP_DBG("write 0x55 to register 0xfc failed");
		goto err;
	}
	msleep(30);
	do{
		mutex_lock(&ts->mutex);
		ret = i2c_master_send(ts->client, (const char*)arrCommand, sizeof(arrCommand));
		mutex_unlock(&ts->mutex);
		++i;
            msleep(5);
	}while(ret <= 0 && i < 5);

	ret = 0;
err:
	return ret;
}

static int fts_mode_switch(tinno_ts_data *ts, int iMode)
{
	int ret = 0;
	
	CTP_DBG("iMode=%d", iMode);
	
	if ( FTS_MODE_OPRATE == iMode ){
	}
	else if (FTS_MODE_UPDATE == iMode){
		ret = fts_switch_to_update(ts);
	}
	else if (FTS_MODE_SYSTEM == iMode){
	}
	else{
		CTP_DBG("unsupport mode %d", iMode);
	}
	return ret;
}


//LINE<JIRA_ID><DATE20130320><BUG_INFO>zenghaihui
int fts_ft5316_switch_charger_status(kal_uint8 charger_flag)
{
    int ret = 0;
    kal_uint8 vl_read_charger_flag = 0;

    CTP_DBG("charger_flag =  %d\n",charger_flag);


    ret = fts_write_reg(0x8b, charger_flag);
    if (ret < 0) {
        CTP_DBG("write  charger_flag(%d) to register 0x8b failed", charger_flag);
        goto err;
    }
    msleep(50);

// read check
#if 0
    ret = fts_read_reg(0x8b, &vl_read_charger_flag);
    if (ret < 0){
        CTP_DBG("read  0x8b failed");
        goto err;
    }
    
    CTP_DBG("read  vl_read_charger_flag = %d, from register 0x8b", vl_read_charger_flag);
#endif    

err:
	return ret;
}



static int fts_ctpm_auto_clb(void)
{
    unsigned char uc_temp[1];
    unsigned char i ;

    CTP_DBG("start auto CLB.\n");
    msleep(200);
    fts_write_reg(0, 0x40);  
    mdelay(100);   //make sure already enter factory mode
    fts_write_reg(2, 0x4);  //write command to start calibration
    mdelay(300);
    for(i=0;i<100;i++)
    {
        fts_read_reg(0,uc_temp);
        if ( ((uc_temp[0]&0x70)>>4) == 0x0)  //return to normal mode, calibration finish
        {
            break;
        }
        mdelay(200);
        CTP_DBG("waiting calibration %d\n",i);
        
    }
    CTP_DBG("calibration OK.\n");
    
    msleep(300);
    fts_write_reg(0, 0x40);  //goto factory mode
    mdelay(100);   //make sure already enter factory mode
    fts_write_reg(2, 0x5);  //store CLB result
    mdelay(300);
    fts_write_reg(0, 0x0); //return to normal mode 
    msleep(300);
    CTP_DBG("store CLB result OK.\n");
    return 0;
}

static int ft5x06_get_tp_id(tinno_ts_data *ts, int *ptp_id)
{
	int rc;
	char tp_id[2];
	*ptp_id = -1;
	
	CTP_DBG("Try to get TPID!");
	
	rc = fts_cmd_write(0x90, 0x00, 0x00, 0x00, 4);
	if (rc < 4) {
		CTP_DBG("i2c_master_send failed(%d)", rc);
		return -EIO;
	} 
	
	ts->client->addr = ts->client->addr & I2C_MASK_FLAG;
	rc = i2c_master_recv(ts->client, tp_id, 2);
	if (rc < 2) {
		CTP_DBG("i2c_master_recv failed(%d)", rc);
		return -EIO;
	} 

	*ptp_id = (( int )tp_id[0] << 8) | (( int )tp_id[1]);

            
//LINE<JIRA_ID><DATE20130418><jiemian V12 tp upgrade problem>zenghaihui
#ifdef FT6X06_DOWNLOAD
    g_tpd_tpid = *ptp_id; 
#endif
            
	return 0;
}

static int ft5x06_get_fw_version(tinno_ts_data *ts)
{
	int ret;
	uint8_t fw_version;
	ret = fts_read_reg(0xA6, &fw_version);
	if (ret < 0){
		CTP_DBG("i2c error, ret=%d\n", ret);
		return -1;
	}
	CTP_DBG("fw_version=0x%X\n", fw_version);
	return (int)fw_version;
}

int get_fw_version_ext(void)
{
	int version = -1;
	if(g_pts)
		version = ft5x06_get_fw_version(g_pts);
	CTP_DBG("fw_version=0x%X\n", version);
	return version;
}
EXPORT_SYMBOL(get_fw_version_ext);

static int ft5x06_get_vendor_from_bootloader(tinno_ts_data *ts, uint8_t *pfw_vendor, uint8_t *pfw_version)
{
	int rc = 0, tp_id;
	uint8_t version_id = 0, buf[5];
	
	CTP_DBG("Try to switch to update mode!");
	
	rc = fts_mode_switch(ts, FTS_MODE_UPDATE);
	if(rc)
	{
		CTP_DBG("switch to update mode error");
		goto err;
	}
	
	rc = ft5x06_get_tp_id(ts, &tp_id);
	CTP_DBG("TPID=0x%X!", tp_id);
	if(rc)
	{
		CTP_DBG("Get tp ID error(%d)", rc);
		goto err;
	}
	if ( FTS_CTP_FIRWARE_ID != tp_id ){
		CTP_DBG("Tp ID is error(0x%x)", tp_id);
		rc = -EINVAL;
		goto err;
	}

	//Get vendor ID.
	rc = fts_cmd_write(0xcd, 0x00, 0x00, 0x00, 1);
	if (rc < 1) {
		CTP_DBG("i2c_master_send failed(%d)", rc);
		goto err;
	} 
	rc = i2c_master_recv(ts->client, &version_id, 1);
	if (rc < 1) {
		CTP_DBG("i2c_master_recv failed(%d)", rc);
		goto err;
	} 
	
	//*pfw_version = version_id;
	*pfw_version = -1;//Force to update.
	CTP_DBG("bootloader version = 0x%x\n", version_id);

	/* --------- read current project setting  ---------- */
	//set read start address
	rc = fts_cmd_write(0x03, 0x00, 0x78, 0x00, 4);
	if (rc < 4) {
		CTP_DBG("i2c_master_send failed(%d)", rc);
		goto err;
	} 
	
	rc = i2c_master_recv(g_pts->client, buf, sizeof(buf));
	if (rc < 0){
		CTP_DBG("i2c_master_recv failed(%d)", rc);
		goto err;
	}

	CTP_DBG("vendor_id = 0x%x\n", buf[4]);
	
	*pfw_vendor = buf[4];
	
	CTP_DBG("Try to reset TP!");
	rc = fts_cmd_write(0x07,0x00,0x00,0x00,1);
	if (rc < 0) {
		CTP_DBG("reset failed");
		goto err;
	}
	msleep(200);
	
	return 0;
err:
	return rc;
}

int ft5x06_get_vendor_version(tinno_ts_data *ts, uint8_t *pfw_vendor, uint8_t *pfw_version)
{
	int ret;
	*pfw_version = ft5x06_get_fw_version(ts);

	ret = fts_read_reg(0xA8, pfw_vendor);
	if (ret < 0){
		CTP_DBG("i2c error, ret=%d\n", ret);
		return ret;
	}
	if ( 0xA8 == *pfw_vendor || 0x00 == *pfw_vendor ){
		CTP_DBG("FW in TP has problem, get factory ID from bootloader.\n");
		ret = ft5x06_get_vendor_from_bootloader(ts, pfw_vendor, pfw_version);
		if (ret){
			CTP_DBG("ft5x06_get_vendor_from_bootloader error, ret=%d\n", ret);
			return -EFAULT;
		}
	}
	
	printk("[CTP-FT5X06] fw_vendor=0x%X, fw_version=0x%X\n", *pfw_vendor, *pfw_version);

//LINE<JIRA_ID><DATE20130418><jiemian V12 tp upgrade problem>zenghaihui
#ifdef FT6X06_DOWNLOAD
    g_tpd_fw_vendor = *pfw_vendor; 
    tpd_fw_version = *pfw_version; 
#endif

	return 0;
}
EXPORT_SYMBOL(ft5x06_get_vendor_version);

static int fts_isp_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	void __user *argp = (void __user *)arg;
	tinno_ts_data *ts = file->private_data;
	int flag;
	int rc = 0;
	
	if ( !atomic_read( &g_pts->isp_opened ) ){
		CTP_DBG("no opened isp.");
		return -ENODEV;
	}
	
	/* check cmd */
	if(_IOC_TYPE(cmd) != TOUCH_IO_MAGIC)	
	{
		CTP_DBG("cmd magic type error");
		return -EINVAL;
	}
	if(_IOC_NR(cmd) > FT5X06_IOC_MAXNR)
	{
		CTP_DBG("cmd number error");
		return -EINVAL;
	}

	if(_IOC_DIR(cmd) & _IOC_READ)
		rc = !access_ok(VERIFY_WRITE,(void __user*)arg, _IOC_SIZE(cmd));
	else if(_IOC_DIR(cmd) & _IOC_WRITE)
		rc = !access_ok(VERIFY_READ, (void __user*)arg, _IOC_SIZE(cmd));
	if(rc)
	{
		CTP_DBG("cmd access_ok error");
		return -EINVAL;
	}

	switch (cmd) {
	case FT5X06_IOCTL_SWITCH_TO:
		CTP_DBG("Try to switch to update mode!(%lu)", arg);
		rc = fts_mode_switch(ts, (int)arg);
		if(rc)
		{
			CTP_DBG("switch to update mode error");
			return -EIO;
		}
		break;
	case FT5X06_IOCTL_WRITE_PROTECT:
		CTP_DBG("Try to set write protect mode!(%lu)", arg);
		rc = -EINVAL;
		break;
	case FT5X06_IOCTL_ERASE:
		CTP_DBG("Try to erase flash!");
		rc = fts_cmd_write(0x61, 0, 0, 0, 1);
		if (rc < 1) {
			CTP_DBG("erase failed");
			break;
		}
		msleep(1500);
		break;
	case FT5X06_IOCTL_GET_STATUS:
		CTP_DBG("Try to get status!");
		flag = fts_i2c_read_a_byte_data(ts->client, 0x05);
		if (flag < 0) {
			CTP_DBG("read check status failed");
		}
		CTP_DBG("status=%d!", flag);
		if(copy_to_user(argp,&flag,sizeof(int))!=0)
		{
			CTP_DBG("copy_to_user error");
			rc = -EFAULT;
		}
		break;
		
	case FT5X06_IOCTL_GET_VENDOR_VERSION:
		{
			int rc;
			uint8_t vendor = 0;
			uint8_t version = 0;
			CTP_DBG("Try to get vendor_version!");
			rc = ft5x06_get_vendor_version(ts, &vendor, &version);
			if ( rc < 0 ){
				rc = -EFAULT;
				break;
			}
			CTP_DBG("vendor version =%d, version=%d!", vendor, version);
			flag = (vendor<<8)|version;
			if(copy_to_user(argp,&flag, sizeof(int))!=0)
			{
				CTP_DBG("copy_to_user error");
				rc = -EFAULT;
			}

			break;
		}
	case FT5X06_IOCTL_GET_CHECKSUM:
		{
			uint8_t check_sum;
			CTP_DBG("Try to get checksum!");
			fts_cmd_write(0xCC,0x00,0x00,0x00,1);
			ts->client->addr = ts->client->addr & I2C_MASK_FLAG;
			rc = i2c_master_recv(ts->client, &check_sum, 1);
			if (rc < 0) {
				CTP_DBG("read checksum failed");
			}
			CTP_DBG("checksum=%d!", check_sum);
			flag = check_sum;
			if(copy_to_user(argp,&flag, sizeof(int))!=0)
			{
				CTP_DBG("copy_to_user error");
				rc = -EFAULT;
			}

			break;
		}
	case FT5X06_IOCTL_RESET:
		CTP_DBG("Try to reset TP!");
		rc = fts_cmd_write(0x07,0x00,0x00,0x00,1);
		if (rc < 0) {
			CTP_DBG("reset failed");
		}
            else
            {
                //LINE<JIRA_ID><DATE20130320><add charger flag>zenghaihui
                g_need_refresh_tp_flag = 1;
            }
        
		break;
		
	case FT5X06_IOCTL_AUTO_CAL:
		CTP_DBG("Try to calibrate TP!");
		msleep(300);

		fts_ctpm_auto_clb();
		
		fts_5x06_hw_reset();
        
            //LINE<JIRA_ID><DATE20130320><add charger flag>zenghaihui
            g_need_refresh_tp_flag = 1;
            
		break;
	case FT5X06_IOCTL_GET_TPID:
		{
			rc = ft5x06_get_tp_id(ts, &flag);
			CTP_DBG("TPID=0x%X!", flag);
                    
                //LINE<JIRA_ID><DATE20130418><jiemian V12 tp upgrade problem>zenghaihui
                #ifdef FT6X06_DOWNLOAD
                if( 0x00 ==flag)
                {
                    uint8_t vendor = 0;
                    uint8_t version = 0;
                    CTP_DBG("Try to get vendor_version!");
                    rc = ft5x06_get_vendor_version(ts, &vendor, &version);
                    if ( rc < 0 ){
                        //
                    }
                    else if((FTS_CTP_VENDOR_JIEMIAN == vendor)
                        && (0x12 == version)
                        )
                    {
                        CTP_DBG("set tpid == 7907 for jiemian V12 tp upgrade problem");
                        flag = 0x7907;
                    }
                }
                #endif  
                    
			if(rc)
			{
				CTP_DBG("Get tp ID error(%d)", rc);
				rc = -EIO;
			} 
			if(copy_to_user(argp,&flag,sizeof(int))!=0)
			{
				CTP_DBG("copy_to_user error");
				rc = -EFAULT;
			}
			break;
		}
	
	default:
		CTP_DBG("invalid command %d", _IOC_NR(cmd));
		rc = -EINVAL;
		break;
	}

	return rc;
}

static const struct file_operations fts_isp_fops = {
	.owner = THIS_MODULE,
	.read = fts_isp_read,
	.write = fts_isp_write,
	.open = fts_isp_open,
	.release = fts_isp_close,
	.unlocked_ioctl = fts_isp_ioctl,
};

static struct miscdevice fts_isp_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "fts_isp",
	.fops = &fts_isp_fops,
};

 /* called when loaded into kernel */
 int fts_5x06_isp_init( tinno_ts_data *ts ) 
 {
 	int ret;
	CTP_DBG("MediaTek FT5x06 touch panel isp init\n");
	
	wake_lock_init(&ts->wake_lock, WAKE_LOCK_SUSPEND, "fts_tp_isp");
	
	ret = misc_register(&fts_isp_device);
	if (ret) {
		misc_deregister(&fts_isp_device);
		printk(KERN_ERR "fts_isp_device device register failed (%d)\n", ret);
		goto exit_misc_device_register_failed;
	}
	
	g_pts = ts;

//LINE<JIRA_ID><DATE20130418><jiemian V12 tp upgrade problem>zenghaihui
#ifdef FT6X06_DOWNLOAD
    g_i2c_client = g_pts->client;
#endif

	return 0;
	
exit_misc_device_register_failed:
	misc_deregister(&fts_isp_device);
	fts_isp_device.minor = MISC_DYNAMIC_MINOR;
	wake_lock_destroy(&ts->wake_lock);
	return ret;
 }
 
 /* should never be called */
 void fts_5x06_isp_exit(void) 
{
	CTP_DBG("MediaTek FT5x06 touch panel isp exit\n");
	if ( g_pts ){
		misc_deregister(&fts_isp_device);
		fts_isp_device.minor = MISC_DYNAMIC_MINOR;
		wake_lock_destroy(&g_pts->wake_lock);
		g_pts = NULL;
	}
}


//LINE<JIRA_ID><DATE20130418><jiemian V12 tp upgrade problem>zenghaihui
#ifdef FT6X06_DOWNLOAD
//#include <mach/gpio.h>
//#include <mach/map.h>
//#include <mach/regs-clock.h>
//#include <mach/regs-gpio.h>
//#include <plat/gpio-cfg.h>
//#include <mach/gpio-bank-q.h>
//#include "ft6x06_download_lib.h"
//#include "ft5x16_download_lib.h"

static unsigned char CTPM_MAIN_FW[]=
{
	//#include "ft6x06_all.i"
	#include "S9201_Jtouch_Ver0x19_20130328_all.i"
};

//#define RESET_GPIO_PIN	S3C64XX_GPQ(4)//wake GPQ4
void ft6x06_InitResetPin(void)
{
#if 0        
	#if 1
	int ret = 0;
	ret = gpio_request(RESET_GPIO_PIN, "FTS_RESET");
	if (ret) {
		pr_err("%s: request GPIO %d for RESET failed, ret = %d\n",
				__func__, RESET_GPIO_PIN, ret);
		return;
	}
	s3c_gpio_cfgpin(RESET_GPIO_PIN, S3C_GPIO_OUTPUT);
	
	gpio_set_value(RESET_GPIO_PIN, 1);
	#else
	gpio_direction_output(RESET_GPIO_PIN, 1);
	gpio_set_value(RESET_GPIO_PIN, 1);
	#endif
#endif    
}
static int ft6x06_CtrlResetPin(unsigned int value)
{
	//gpio_set_value(RESET_GPIO_PIN, value);
	mt_set_gpio_out(GPIO_CTP_RST_PIN, value);
	return 0;
}
void ft6x06_FreeResetPin(void)
{
    fts_5x06_hw_reset();
    #if 0
	ft6x06_CtrlResetPin(1);
	delay_qt_ms(150);
	gpio_free(RESET_GPIO_PIN);
    #endif
}

int ft6x06_Enter_Debug(void)
{
	CTP_DBG("ft6x06 start enter debug");
	CTP_DBG("reset low");
	ft6x06_CtrlResetPin(0);
	delay_qt_ms(4);
	ft6x06_CtrlResetPin(1);
	CTP_DBG("reset high");

	//return ft6x06_Lib_Enter_Download_Mode();
	return ft5x16_Lib_Enter_Download_Mode();
}
//if return 0, main flash is ok, else download.
int ft6x06_IsDownloadMain(void)
{
    CTP_DBG(" g_tpd_fw_vendor = %x, tpd_fw_version = %x,  g_tpd_tpid = %x", g_tpd_fw_vendor, tpd_fw_version, g_tpd_tpid);
    
    if((FTS_CTP_VENDOR_JIEMIAN == g_tpd_fw_vendor)
        && (0x12 == tpd_fw_version)
        && (0x00 == g_tpd_tpid)
        )
        
    {
        CTP_DBG("jiemian V12 tp upgrade flag");
        
        return 1;
    }
    
    return 0;
}
int ft6x06_DownloadMain(void)
{
	short fwlen = 0;
	if (ft6x06_Enter_Debug() < 0) {
		pr_err("-----enter debug mode failed\n");
		return -1;
	}
	fwlen = sizeof(CTPM_MAIN_FW);
	pr_info("----fwlen=%d\n", fwlen);

    SetFtsPacketLen(2);
    
	//return ft6x06_Lib_DownloadMain(CTPM_MAIN_FW, fwlen);
	return ft5x16_Lib_DownloadMain(CTPM_MAIN_FW, fwlen);
}


int ft6x06_download_i2c_Read(unsigned char *writebuf,
		    int writelen, unsigned char *readbuf, int readlen)
{
	int ret;

	if (writelen > 0) {
		struct i2c_msg msgs[] = {
			{
			 .addr = g_i2c_client->addr, // 0x38,
			 .flags = 0,
			 .len = writelen,
			 .buf = writebuf,
			 },
			{
			 .addr = g_i2c_client->addr, // 0x38,
			 .flags = I2C_M_RD,
			 .len = readlen,
			 .buf = readbuf,
			 },
		};
		ret = i2c_transfer(g_i2c_client->adapter, msgs, 2);
        
        CTP_DBG(" ret = %x", ret);
        
		if (ret < 0)
			dev_err(&g_i2c_client->dev, "f%s: i2c read error.\n",
				__func__);
	} else {
		struct i2c_msg msgs[] = {
			{
			 .addr = g_i2c_client->addr, // 0x38,
			 .flags = I2C_M_RD,
			 .len = readlen,
			 .buf = readbuf,
			 },
		};
		ret = i2c_transfer(g_i2c_client->adapter, msgs, 1);
        
        CTP_DBG(" ret = %x", ret);
        
		if (ret < 0)
			dev_err(&g_i2c_client->dev, "%s:i2c read error.\n", __func__);
	}
	return ret;
}
/*write data by i2c*/
int ft6x06_download_i2c_Write(unsigned char *writebuf, int writelen)
{
	int ret;

	struct i2c_msg msg[] = {
		{
		 .addr = g_i2c_client->addr, // 0x38,
		 .flags = 0,
		 .len = writelen,
		 .buf = writebuf,
		 },
	};

	ret = i2c_transfer(g_i2c_client->adapter, msg, 1);
    
    CTP_DBG(" ret = %x", ret);
    
	if (ret < 0)
		dev_err(&g_i2c_client->dev, "%s i2c write error.\n", __func__);

	return ret;
}


void ft6x06_Delay (unsigned long msec)
{
    //mdelay(1);
    udelay(20);
}


#endif

