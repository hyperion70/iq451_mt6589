/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * MediaTek Inc. (C) 2010. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#include <typedefs.h>
#include <mt_rtc.h>
#include <mtk_timer.h>
#include <mt_pmic_wrap_init.h>
#include <platform.h>
#include <cust_rtc.h>
#include <mtk_pmic_6320.h>
#include <mt6589.h>

#define RTC_RELPWR_WHEN_XRST	1   /* BBPU = 0 when xreset_rstb goes low */

#define RTC_GPIO_USER_MASK	  (((1U << 13) - 1) & 0xff00)
static int XOSC=0;

static U16 RTC_Read(U16 addr)
{
	U32 rdata=0;
	pwrap_read((U32)addr, &rdata);
	return (U16)rdata;
}

static void RTC_Write(U16 addr, U16 data)
{
	pwrap_write((U32)addr, (U32)data);
}


#define rtc_busy_wait()							 \
do {												\
	while (RTC_Read(RTC_BBPU) & RTC_BBPU_CBUSY);   \
} while (0)
#if defined (MTK_KERNEL_POWER_OFF_CHARGING)
extern kal_bool kpoc_flag ;
#endif

static unsigned long rtc_mktime(int yea, int mth, int dom, int hou, int min, int sec)
{
	unsigned long d1, d2, d3;

	mth -= 2;
	if (mth <= 0) {
		mth += 12;
		yea -= 1;
	}

	d1 = (yea - 1) * 365 + (yea / 4 - yea / 100 + yea / 400);
	d2 = (367 * mth / 12 - 30) + 59;
	d3 = d1 + d2 + (dom - 1) - 719162;
	
	return ((d3 * 24 + hou) * 60 + min) * 60 + sec;
}

static void Write_trigger(void)
{
	RTC_Write(RTC_WRTGR, 1);
	rtc_busy_wait();
}

static void Writeif_unlock(void)
{
	RTC_Write(RTC_PROT, 0x586a);
	Write_trigger();
	RTC_Write(RTC_PROT, 0x9136);
	Write_trigger();
}

static void rtc_xosc_write(U16 val, bool reload)
{
	U16 bbpu;

	RTC_Write(RTC_OSC32CON, 0x1a57);
	rtc_busy_wait();
	RTC_Write(RTC_OSC32CON, 0x2b68);
	rtc_busy_wait();

	RTC_Write(RTC_OSC32CON, val);
	rtc_busy_wait();

	if (reload) {
		bbpu = RTC_Read(RTC_BBPU) | RTC_BBPU_KEY | RTC_BBPU_RELOAD;
		RTC_Write(RTC_BBPU, bbpu);
		Write_trigger();
	}
}

static void rtc_android_init(void)
{
	U16 irqsta;

	RTC_Write(RTC_IRQ_EN, 0);
	RTC_Write(RTC_CII_EN, 0);
	RTC_Write(RTC_AL_MASK, 0);

	RTC_Write(RTC_AL_YEA, 1970 - RTC_MIN_YEAR);
	RTC_Write(RTC_AL_MTH, 1);
	RTC_Write(RTC_AL_DOM, 1);
	RTC_Write(RTC_AL_DOW, 1);
	RTC_Write(RTC_AL_HOU, 0);
	RTC_Write(RTC_AL_MIN, 0);
	RTC_Write(RTC_AL_SEC, 0);

	RTC_Write(RTC_PDN1, 0x8000);   /* set Debug bit */
	RTC_Write(RTC_PDN2, ((1970 - RTC_MIN_YEAR) << 8) | 1);
	RTC_Write(RTC_SPAR0, 0);
	RTC_Write(RTC_SPAR1, (1 << 11));

	RTC_Write(RTC_DIFF, 0);
	RTC_Write(RTC_CALI, 0);
	Write_trigger();

	irqsta = RTC_Read(RTC_IRQ_STA);	/* read clear */

	/* init time counters after resetting RTC_DIFF and RTC_CALI */
	RTC_Write(RTC_TC_YEA, RTC_DEFAULT_YEA - RTC_MIN_YEAR);
	RTC_Write(RTC_TC_MTH, RTC_DEFAULT_MTH);
	RTC_Write(RTC_TC_DOM, RTC_DEFAULT_DOM);
	RTC_Write(RTC_TC_DOW, 1);
	RTC_Write(RTC_TC_HOU, 0);
	RTC_Write(RTC_TC_MIN, 0);
	RTC_Write(RTC_TC_SEC, 0);
	Write_trigger();
}

static void rtc_gpio_init(void)
{
	U16 con;

	/* GPI mode and pull enable + pull down */
	con = RTC_Read(RTC_CON) & 0x800c;
	con &= ~(RTC_CON_GOE | RTC_CON_GPU);
	con |= RTC_CON_GPEN | RTC_CON_F32KOB;
	RTC_Write(RTC_CON, con);
	Write_trigger();
}

static U16 get_frequency_meter(U16 val)
{
	U16 ret;
	int i;

	if(val!=0)
		rtc_xosc_write(val, true);

	RTC_Write(TOP_RST_CON, 0x0100);
	while( !(RTC_Read(FQMTR_CON2)==0) );
	RTC_Write(TOP_RST_CON, 0x0000);

	RTC_Write(FQMTR_CON1, 0x0000);
	RTC_Write(FQMTR_CON0, 0x8004);

	mdelay(1);
	while( (0x8&RTC_Read(FQMTR_CON0))==0x8 );
	
	ret = RTC_Read(FQMTR_CON2);
	print("[RTC] get_frequency_meter: input=0x%x, ouput=%d\n",val, ret);

	return ret;
}

static U16 eosc_cali(void)
{
	U16 val, diff;
	int middle;
	int i, j;

	print("[RTC] EOSC_Cali: TOP_CKCON1=0x%x\n", RTC_Read(TOP_CKCON1));

	int left = 0x24C0, right=0x24DF;

	while( left<=(right) )
	{
		middle = (right + left) / 2;
		if(middle == left)
			break;

		val = get_frequency_meter(middle);
		if ((val>792) && (val<796))
			break;
		if (val > 795)
			//right = middle - 1;
			right = middle;
		else
			//left = middle + 1;
			left = middle;
	}
	
	if ((val>792) && (val<796))
		return middle;
	
	val=get_frequency_meter(left);
	diff=793-val;
	val=get_frequency_meter(right);
	if(diff<(val-793))
		return left;
	else
		return right;	
}

static void rtc_osc_init(void)
{
	U16 con;

	//con = RTC_Read(RTC_OSC32CON);

	/* disable 32K export if there are no RTC_GPIO users */
	if (!(RTC_Read(RTC_PDN1) & RTC_GPIO_USER_MASK))
		rtc_gpio_init();

	if(XOSC==1)
	{
		RTC_Write(TOP_CKTST2, 0x0);	
		con = RTC_Read(RTC_OSC32CON);
		if ((con & 0x000f) != 0x0) {	/* check XOSCCALI */
			rtc_xosc_write(0x0003, false);  /* crystal exist mode + XOSCCALI = 0x3 */
			gpt_busy_wait_us(200);
		}
	
		rtc_xosc_write(0x0000, true);  /* crystal exist mode + XOSCCALI = 0x0 */
	}
	else
	{
		U16 val;
		RTC_Write(TOP_CKTST2, 0x80);
		val = eosc_cali();
		print("[RTC] EOSC cali val = 0x%x\n", val);
		//EMB_HW_Mode
		val = (val & 0x001f)|0x2400;
		rtc_xosc_write(val, true);		
	}
}

static void rtc_lpd_init(void)
{
	U16 con;

	con = RTC_Read(RTC_CON) | RTC_CON_LPEN;
	con &= ~RTC_CON_LPRST;
	RTC_Write(RTC_CON, con);
	Write_trigger();

	con |= RTC_CON_LPRST;
	RTC_Write(RTC_CON, con);
	Write_trigger();

	con &= ~RTC_CON_LPRST;
	RTC_Write(RTC_CON, con);
	Write_trigger();
}

static void rtc_power_inconsistent_init(void)
{
	rtc_gpio_init();
	if(XOSC==1)
	{
		RTC_Write(TOP_CKTST2, 0);
		rtc_xosc_write(0x0003, false);  /* crystal exist mode + XOSCCALI = 0x3 */
	}
	else
	{
		RTC_Write(TOP_CKTST2, 0x80);
		rtc_xosc_write(0x240F, false); /*crystal not exist + eosc cali = 0xF*/
	}	
	rtc_android_init();

	/* write powerkeys */
	RTC_Write(RTC_POWERKEY1, RTC_POWERKEY1_KEY);
	RTC_Write(RTC_POWERKEY2, RTC_POWERKEY2_KEY);
	Write_trigger();

	if(XOSC==1)
	{
//		gpt_busy_wait_us(200);
//		rtc_xosc_write(0x0005, false);
//		gpt_busy_wait_us(200);
//		rtc_xosc_write(0x0003, false);
//		gpt_busy_wait_us(200);
		rtc_xosc_write(0x0000, true);	/* crystal exist mode + XOSCCALI = 0 */
	}
	else
	{
		U16 val;
		val = eosc_cali();
		print("[RTC] EOSC cali val = 0x%x\n", val);
		//EMB_HW_Mode
		val = (val & 0x001f)|0x2400;
		rtc_xosc_write(val, true);
	}

	rtc_lpd_init();
}

static void rtc_bbpu_power_down(void)
{
	U16 bbpu;

	/* pull PWRBB low */
	bbpu = RTC_BBPU_KEY | RTC_BBPU_AUTO | RTC_BBPU_PWREN;
	Writeif_unlock();
	RTC_Write(RTC_BBPU, bbpu);
	Write_trigger();
}

void rtc_bbpu_power_on(void)
{
	U16 bbpu;

	/* pull PWRBB high */
#if RTC_RELPWR_WHEN_XRST
	bbpu = RTC_BBPU_KEY | RTC_BBPU_AUTO | RTC_BBPU_BBPU | RTC_BBPU_PWREN;
#else
	bbpu = RTC_BBPU_KEY | RTC_BBPU_BBPU | RTC_BBPU_PWREN;
#endif
	RTC_Write(RTC_BBPU, bbpu);
	Write_trigger();
	print("[RTC] rtc_bbpu_power_on done\n");
}

void rtc_mark_bypass_pwrkey(void)
{
	U16 pdn1;

	pdn1 = RTC_Read(RTC_PDN1) | 0x0040;
	RTC_Write(RTC_PDN1, pdn1);
	Write_trigger();
}

static void rtc_clean_mark(void)
{
	U16 pdn1, pdn2;

	pdn1 = RTC_Read(RTC_PDN1) & ~0x8040;   /* also clear Debug bit */
	pdn2 = RTC_Read(RTC_PDN2) & ~0x0010;
	RTC_Write(RTC_PDN1, pdn1);
	RTC_Write(RTC_PDN2, pdn2);
	Write_trigger();
}

U16 rtc_rdwr_uart_bits(U16 *val)
{
	U16 pdn2;

	if (RTC_Read(RTC_CON) & RTC_CON_LPSTA_RAW)
		return 3;   /* UART bits are invalid due to RTC uninit */

	if (val) {
		pdn2 = RTC_Read(RTC_PDN2) & ~0x0060;
		pdn2 |= (*val & 0x0003) << 5;
		RTC_Write(RTC_PDN2, pdn2);
		Write_trigger();
	}

	return (RTC_Read(RTC_PDN2) & 0x0060) >> 5;
}

bool rtc_boot_check(void)
{
	U16 irqsta, pdn1, pdn2, spar0, spar1, Rdata;

	//Disable RTC CLK gating
	RTC_Write(TOP_CKPDN, 0);
	RTC_Write(TOP_CKPDN2, 0);

	RTC_Write(TOP_CKCON2, (RTC_Read(TOP_CKCON2)|0x8000) );//how many 26M pulse in one 32K pulse
#if 0
	//reset freq. meter
	RTC_Write(TOP_RST_CON, 0x0100);
	while( !(RTC_Read(FQMTR_CON2)==0) );
	RTC_Write(TOP_RST_CON, 0x0000);

	//Select fix clock source
	RTC_Write(TOP_CKTST2, 0x80); //swtich to EOSC32 
	RTC_Write(TOP_CKTST2, 0x00); //swtich to XOSC32_DET
	RTC_Write(FQMTR_CON1, 0); //set freq. meter window value (0=1X32K(fix clock))
	RTC_Write(FQMTR_CON0, 0x8004); //enable freq. meter, set measure clock to 26Mhz
	
	mdelay(1);

	while( (0x8 & RTC_Read(FQMTR_CON0))==0x8); //read until busy = 0
	Rdata = RTC_Read(FQMTR_CON2); //read data should be closed to 26M/32k = 812.5
	print("[RTC] FQMTR_CON2=0x%x\n", Rdata);
#endif
	
	RTC_Write(TOP_CKTST2, 0x00); //swtich to XOSC32_DET
	
	Rdata = RTC_Read(TOP_CKCON1);
	if(! (Rdata&0x0010) )
	{	
		print("[RTC] TOP_CKCON1=0x%x\n", Rdata);
		RTC_Write(TOP_CKCON1, Rdata|0x0010);
		mdelay(1);
	}
	Rdata = get_frequency_meter(0);
	if(Rdata==0)
	{
		//crystal doesn't exist
		XOSC=0;
		print("[RTC] External crystal doesn't exist\n");
	}
	else
	{
		//crystal exist
		XOSC=1;
		print("[RTC] External crystal exist\n");
	}

	print("[RTC] bbpu = 0x%x, con = 0x%x\n", RTC_Read(RTC_BBPU), RTC_Read(RTC_CON));

	/* normally HW reload is done in BROM but check again here */
	while (RTC_Read(RTC_BBPU) & RTC_BBPU_CBUSY);

	Writeif_unlock();

	if (RTC_Read(RTC_POWERKEY1) != RTC_POWERKEY1_KEY ||
		RTC_Read(RTC_POWERKEY2) != RTC_POWERKEY2_KEY ||
		(RTC_Read(RTC_CON) & RTC_CON_LPSTA_RAW)) 
	{
		print("[RTC] powerkey1 = 0x%x, powerkey2 = 0x%x\n",
			RTC_Read(RTC_POWERKEY1), RTC_Read(RTC_POWERKEY2));
		rtc_power_inconsistent_init();
	} else {
		RTC_Write(RTC_BBPU, RTC_Read(RTC_BBPU) | RTC_BBPU_KEY | RTC_BBPU_RELOAD);
		Write_trigger();
		print("[RTC] RTC_OSC32CON=0x%x\n", RTC_Read(RTC_OSC32CON));
		//print("xosc = 0x%x\n", RTC_Read(RTC_OSC32CON));
		rtc_osc_init();
		rtc_clean_mark();
	}

	//set register to let MD know 32k status
	spar0 = RTC_Read(RTC_SPAR0);
	if(XOSC==1)
		RTC_Write(RTC_SPAR0, (spar0 | 0x0040) );
	else
		RTC_Write(RTC_SPAR0, (spar0 & 0xffbf) );
	Write_trigger();


	irqsta = RTC_Read(RTC_IRQ_STA);	/* Read clear */
	pdn1 = RTC_Read(RTC_PDN1);
	pdn2 = RTC_Read(RTC_PDN2);
	spar0 = RTC_Read(RTC_SPAR0);
	spar1 = RTC_Read(RTC_SPAR1);
	print("[RTC] irqsta = 0x%x, pdn1 = 0x%x, pdn2 = 0x%x, spar0 = 0x%x, spar1 = 0x%x\n",
		  irqsta, pdn1, pdn2, spar0, spar1);
	print("[RTC] new_spare0 = 0x%x, new_spare1 = 0x%x, new_spare2 = 0x%x, new_spare3 = 0x%x",
		  RTC_Read(RTC_AL_HOU), RTC_Read(RTC_AL_DOM), RTC_Read(RTC_AL_DOW), RTC_Read(RTC_AL_MTH));



	if (irqsta & RTC_IRQ_STA_AL) {
#if RTC_RELPWR_WHEN_XRST
		/* set AUTO bit because AUTO = 0 when PWREN = 1 and alarm occurs */
		U16 bbpu = RTC_Read(RTC_BBPU) | RTC_BBPU_KEY | RTC_BBPU_AUTO;
		RTC_Write(RTC_BBPU, bbpu);
		Write_trigger();
#endif

		if (pdn1 & 0x0080) {	/* power-on time is available */
			U16 now_sec, now_min, now_hou, now_dom, now_mth, now_yea;
			U16 irqen, sec, min, hou, dom, mth, yea;
			unsigned long now_time, time;

			now_sec = RTC_Read(RTC_TC_SEC);
			now_min = RTC_Read(RTC_TC_MIN);
			now_hou = RTC_Read(RTC_TC_HOU);
			now_dom = RTC_Read(RTC_TC_DOM);
			now_mth = RTC_Read(RTC_TC_MTH);
			now_yea = RTC_Read(RTC_TC_YEA) + RTC_MIN_YEAR;
			if (RTC_Read(RTC_TC_SEC) < now_sec) {  /* SEC has carried */
				now_sec = RTC_Read(RTC_TC_SEC);
				now_min = RTC_Read(RTC_TC_MIN);
				now_hou = RTC_Read(RTC_TC_HOU);
				now_dom = RTC_Read(RTC_TC_DOM);
				now_mth = RTC_Read(RTC_TC_MTH);
				now_yea = RTC_Read(RTC_TC_YEA) + RTC_MIN_YEAR;
			}

			sec = spar0 & 0x003f;
			min = spar1 & 0x003f;
			hou = (spar1 & 0x07c0) >> 6;
			dom = (spar1 & 0xf800) >> 11;
			mth = pdn2 & 0x000f;
			yea = ((pdn2 & 0x7f00) >> 8) + RTC_MIN_YEAR;

			now_time = rtc_mktime(now_yea, now_mth, now_dom, now_hou, now_min, now_sec);
			time = rtc_mktime(yea, mth, dom, hou, min, sec);

			print("[RTC] now = %d/%d/%d %d:%d:%d (%u)\n",
				  now_yea, now_mth, now_dom, now_hou, now_min, now_sec, now_time);
			print("[RTC] power-on = %d/%d/%d %d:%d:%d (%u)\n",
				  yea, mth, dom, hou, min, sec, time);

			if (now_time >= time - 1 && now_time <= time + 4) {	 /* power on */
				pdn1 = (pdn1 & ~0x0080) | 0x0040;
				RTC_Write(RTC_PDN1, pdn1);
				RTC_Write(RTC_PDN2, pdn2 | 0x0010);
				Write_trigger();
				if (!(pdn2 & 0x8000))   /* no logo means ALARM_BOOT */
					g_boot_mode = ALARM_BOOT;
#if defined (MTK_KERNEL_POWER_OFF_CHARGING)
			if ((pdn1 & 0x4000) == 0x4000) {   
				kpoc_flag = false;
				RTC_Write(RTC_PDN1, pdn1 & ~0x4000);
				Write_trigger();
			}
#endif
				return true;
			} else if (now_time < time) {   /* set power-on alarm */
				RTC_Write(RTC_AL_YEA, yea - RTC_MIN_YEAR);
				RTC_Write(RTC_AL_MTH, (RTC_Read(RTC_AL_MTH)&0xff00)|mth);
				RTC_Write(RTC_AL_DOM, (RTC_Read(RTC_AL_DOM)&0xff00)|dom);
				RTC_Write(RTC_AL_HOU, (RTC_Read(RTC_AL_HOU)&0xff00)|hou);
				RTC_Write(RTC_AL_MIN, min);
				RTC_Write(RTC_AL_SEC, sec);
				RTC_Write(RTC_AL_MASK, 0x0010);	/* mask DOW */
				Write_trigger();
				irqen = RTC_Read(RTC_IRQ_EN) | RTC_IRQ_EN_ONESHOT_AL;
				RTC_Write(RTC_IRQ_EN, irqen);
				Write_trigger();
			}
		}
	}

	if ((pdn1 & 0x0030) == 0x0010) {	/* factory data reset */
		/* keep bit 4 set until rtc_boot_check() in U-Boot */
		return true;
	}
#if defined (MTK_KERNEL_POWER_OFF_CHARGING)
	if ((pdn1 & 0x4000) == 0x4000) {   
		kpoc_flag = true;
		return false;
	}
#endif

	return false;
}

void pl_power_off(void)
{
	print("[RTC] pl_power_off\n");

	rtc_bbpu_power_down();

	while (1);
}

