/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2008
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/

#ifndef BUILD_LK
#include <linux/string.h>
#endif
#include "lcm_drv.h"

#ifdef BUILD_LK
	#include <platform/mt_gpio.h>
#elif defined(BUILD_UBOOT)
	#include <asm/arch/mt_gpio.h>
#else
	#include <mach/mt_gpio.h>
#endif
// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------

#define FRAME_WIDTH  (720)
#define FRAME_HEIGHT (1280)

#define LCM_ID_HX8394 (0x94)

#ifndef TRUE
    #define TRUE 1
#endif

#ifndef FALSE
    #define FALSE 0
#endif

static unsigned int lcm_esd_test = FALSE;      ///only for ESD test

// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util = {0};

#define SET_RESET_PIN(v)    (lcm_util.set_reset_pin((v)))

#define UDELAY(n) (lcm_util.udelay(n))
#define MDELAY(n) (lcm_util.mdelay(n))


// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------

#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)	        lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)		lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)										lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)					lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg(cmd)											lcm_util.dsi_dcs_read_lcm_reg(cmd)
#define read_reg_v2(cmd, buffer, buffer_size)   				lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)   


#define   LCM_DSI_CMD_MODE							0


static void init_lcm_registers(void)
{
	unsigned int data_array[16];

#if 1
	data_array[0] = 0x00110500; // Sleep Out
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(200);

	data_array[0] = 0x00043902;                          
      data_array[1] = 0x9483ffb9;                 
      dsi_set_cmdq(data_array, 2, 1);
	
	//{0xB9,	3,	{0xFF,0x83,0x94}},			//set password

	data_array[0] = 0x00123902;                          
      data_array[1] = 0x008313ba;      
      data_array[2] = 0x0811a616;   
      data_array[3] = 0x03240fff;   
      data_array[4] = 0x20252421;   
      data_array[5] = 0x00001002;   	  
      dsi_set_cmdq(data_array, 6, 1);
	  
  	//{0xBA,	17,	{0x13, 0x83, 0x00, 0x16,		// set mipi control
  				//  0xA6, 0x11, 0x08, 0xFF,
  				 // 0x0F, 0x24, 0x03, 0x21,
  				 // 0x24, 0x25, 0x20, 0x02,
  				 // 0x10}},    

     data_array[0] = 0x00103902;                          
      data_array[1] = 0x24007cb1;      
      data_array[2] = 0x11110189;   //0x11110109  //<20130318>wangyanhui
      data_array[3] = 0x26263e36;   
      data_array[4] = 0xe6011a57;   //0a
      dsi_set_cmdq(data_array, 5, 1);
	  
  	//{0xB1,	15,	{0x7C, 0x00, 0x24, 0x09,		// set power
  				  //0x01, 0x11, 0x11, 0x36,
  				  //0x3E, 0x26, 0x26, 0x57,
  				  //0x0A, 0x01, 0xE6}},    

      data_array[0]=0x00200500; 					//line <> <20121215> lishengli  
	dsi_set_cmdq(&data_array, 1, 1);


      //data_array[0] = 0x00023902;                     	//line <> <20121215> lishengli     
      //data_array[1] = 0x00000a36;          	  
      //dsi_set_cmdq(data_array, 2, 1);    //wangyanhui delete

      data_array[0] = 0x00023902;                     	//line <> <20121215> lishengli     
      data_array[1] = 0x000009cc;          	  
      dsi_set_cmdq(data_array, 2, 1);    //wangyanhui delete

      //data_array[0] = 0x00023902;		   //wangyanhui add 
      //data_array[1] = 0x000009cc;          	  
      //dsi_set_cmdq(data_array, 2, 1);  
      
      data_array[0] = 0x00073902;                          
      data_array[1] = 0x04c80fb2;      
      data_array[2] = 0x00810004;     	  
      dsi_set_cmdq(data_array, 3, 1);
	  
  	//{0xB2,	6,	{0x0F, 0xC8, 0x04, 0x04,
  				 // 0x00, 0x81}},    

      data_array[0] = 0x00033902;                          
      data_array[1] = 0x001006bf;      	  
      dsi_set_cmdq(data_array, 2, 1);  
  	//{0xBF,	2,	{0x06, 0x10}},    				// increase driving abilty


     data_array[0] = 0x00133902;                          
      data_array[1] = 0x000000b4;      
      data_array[2] = 0x42410605;   
      data_array[3] = 0x43424102;   
      data_array[4] = 0x58581947;   //0x60581947 //<20130318>wangyanhui
      data_array[5] = 0x00108508;   	  
      dsi_set_cmdq(data_array, 6, 1);
	  
  	//{0xB4,	18,	{0x00, 0x00, 0x00, 0x05,		// SET CYC 
  				 // 0x06, 0x41, 0x42, 0x02,
  				  //0x41, 0x42, 0x43, 0x47,
  				  //0x19, 0x58, 0x60, 0x08,
  				  //0x85, 0x10}},    
    
     data_array[0] = 0x00193902;                          
      data_array[1] = 0x00014cd5;    //0x07014cd5   //<20130318>wangyanhui
      data_array[2] = 0xef23cd01;   
      data_array[3] = 0xab896745;   
      data_array[4] = 0x10dc0011;   
      data_array[5] = 0x98ba32fe;   
      data_array[6] = 0x11005476;   	
      data_array[7] = 0x00000040;   		  
      dsi_set_cmdq(data_array, 8, 1);
	  
	//{0xD5,	24,	{0x4C, 0x01, 0x07, 0x01,		// SET GIP
				 //0xCD, 0x23, 0xEF, 0x45,
				 //0x67, 0x89, 0xAB, 0x11,
				 //0x00, 0xDC, 0x10, 0xFE,
				 //0x32, 0xBA, 0x98, 0x76,
				 //0x54, 0x00, 0x11, 0x40}},
     data_array[0] = 0x00233902;                          
      data_array[1] = 0x363324e0;      
      data_array[2] = 0x3c3f3f3f;   
      data_array[3] = 0x0e0c0556;   
      data_array[4] = 0x14121311;   
      data_array[5] = 0x33241e12;   
      data_array[6] = 0x3f3f3f36;   	
      data_array[7] = 0x0c05563c;   	
      data_array[8] = 0x1213110e; 	
      data_array[9] = 0x001e1214; 	  
      dsi_set_cmdq(data_array, 10, 1);
	  
	//{0xE0, 	34,	{0x24, 0x33, 0x36, 0x3F,		// R Gamma
				  //0x3f, 0x3f, 0x3c, 0x56,
				  //0x05, 0x0c, 0x0e, 0x11,
				  //0x13, 0x12, 0x14, 0x12,
				  //0x1e, 0x24, 0x33, 0x36,
				 // 0x3f, 0x3f, 0x3f, 0x3c,
				  //0x56, 0x05, 0x0c, 0x0e,
				  //0x11, 0x13, 0x12, 0x14, 
				  //0x12, 0x1e}},
	
      data_array[0] = 0x00023902;                          
      data_array[1] = 0x000001e3;         	  
      dsi_set_cmdq(data_array, 2, 1);  
	//{0xE3,	1, {0x01}},						// ENABLE CE
	//{REGFLAG_DELAY, 5, {}},
	MDELAY(5);
	data_array[0] = 0x00133902;                          
      data_array[1] = 0x040000e5;      
      data_array[2] = 0x80000204;   
      data_array[3] = 0x00200020;   
      data_array[4] = 0x04060800;   
      data_array[5] = 0x000e8000;   	  
      dsi_set_cmdq(data_array, 6, 1);
	  
	//{0xE5,	18,	{0x00, 0x00, 0x04, 0x04, 		// CE PARAMETER	
				 // 0x02, 0x00, 0x80, 0x20, 
				  //0x00, 0x20, 0x00, 0x00, 
				  //0x08, 0x06, 0x04, 0x00, 
				  //0x80, 0x0E}}, 
	//{REGFLAG_DELAY, 5, {}},
	

      data_array[0] = 0x00033902;                          
      data_array[1] = 0x003000c7;          	  
      dsi_set_cmdq(data_array, 2, 1);  
	//{0xC7, 	2,	{0x00, 0x30}},				// Set TCON_OPT 


// add cmd-c6 
	data_array[0] = 0x00043902; 						 
	data_array[1] = 0x800004C6; 				
	dsi_set_cmdq(data_array, 2, 1); 
	MDELAY(1);
//add end //wangyanhui add 

      data_array[0] = 0x00023902;                          
      data_array[1] = 0x00002ab6;       //0x00002ab6
      dsi_set_cmdq(data_array, 2, 1);  

//add  CE START
#if 0
      data_array[0] = 0x00023902;                          
      data_array[1] = 0x000001e6;       //0x00002ab6
      dsi_set_cmdq(data_array, 2, 1);  


      data_array[0] = 0x00023902;                          
      data_array[1] = 0x000003e4;       //0x00002ab6
      dsi_set_cmdq(data_array, 2, 1);  
#endif	  
//add  CE  END
	  
	//{0xB6, 	1,	{0x2A}},						//Set VCOM
	data_array[0] = 0x00110500; // Sleep Out
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(120);
	  
	data_array[0] = 0x00290500; // Display On
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(120);
	//{0x29,	0,	{}},
	//{REGFLAG_DELAY, 120, {}},
#endif	

}


// ---------------------------------------------------------------------------
//  LCM Driver Implementations
// ---------------------------------------------------------------------------

static void lcm_set_util_funcs(const LCM_UTIL_FUNCS *util)
{
    memcpy(&lcm_util, util, sizeof(LCM_UTIL_FUNCS));
}


static void lcm_get_params(LCM_PARAMS *params)
{
		memset(params, 0, sizeof(LCM_PARAMS));
	
		params->type   = LCM_TYPE_DSI;

		params->width  = FRAME_WIDTH;
		params->height = FRAME_HEIGHT;

       //1 SSD2075 has no TE Pin
		// enable tearing-free
		params->dbi.te_mode 				= LCM_DBI_TE_MODE_DISABLED;
		params->dbi.te_edge_polarity		= LCM_POLARITY_RISING;

        #if (LCM_DSI_CMD_MODE)
		params->dsi.mode   = CMD_MODE;
        #else
		//params->dsi.mode   = SYNC_PULSE_VDO_MODE;
		params->dsi.mode   = BURST_VDO_MODE;
		//params->dsi.mode   = SYNC_EVENT_VDO_MODE; 
		
        #endif
	
		// DSI
		/* Command mode setting */
		//1 Three lane or Four lane
		params->dsi.LANE_NUM				= LCM_FOUR_LANE;
		//The following defined the fomat for data coming from LCD engine.
		params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
		params->dsi.data_format.trans_seq   = LCM_DSI_TRANS_SEQ_MSB_FIRST;
		params->dsi.data_format.padding     = LCM_DSI_PADDING_ON_LSB;
		params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB888;

		// Highly depends on LCD driver capability.
		// Not support in MT6573
		params->dsi.packet_size=256;

		// Video mode setting		
		params->dsi.intermediat_buffer_num = 0;//because DSI/DPI HW design change, this parameters should be 0 when video mode in MT658X; or memory leakage

		params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;
		params->dsi.word_count=720*3;	

		//BEGIN<20130325>wangyanhui modify
		params->dsi.vertical_sync_active				= 3;  //---3
		params->dsi.vertical_backporch					= 11; //---14
		params->dsi.vertical_frontporch					= 8;  //----8
		params->dsi.vertical_active_line				= FRAME_HEIGHT; 

		params->dsi.horizontal_sync_active				= 10;		// 2;  //----2
		params->dsi.horizontal_backporch				= 80;			// 28; //----28
		params->dsi.horizontal_frontporch				= 80;			// 50; //----50
		params->dsi.horizontal_active_pixel				= FRAME_WIDTH;
		//END<20130325>wangyanhui modify

        //	params->dsi.HS_PRPR=6;
	    params->dsi.LPX=8; 
		//params->dsi.HS_PRPR=5;
		//params->dsi.HS_TRAIL=13;

		// Bit rate calculation
		//1 Every lane speed
		params->dsi.pll_div1=	0;		// div1=0,1,2,3;div1_real=1,2,4,4 ----0: 546Mbps  1:273Mbps
		params->dsi.pll_div2=	1;		// div2=0,1,2,3;div1_real=1,2,4,4	
		params->dsi.fbk_div =	17;		//19;    // fref=26MHz, fvco=fref*(fbk_div+1)*2/(div1_real*div2_real)	

}

static void lcm_init(void)
{

	SET_RESET_PIN(1);
	SET_RESET_PIN(0);
	MDELAY(1);
	
	SET_RESET_PIN(1);
	MDELAY(20);      

	init_lcm_registers();

}



static void lcm_suspend(void)
{
	unsigned int data_array[16];

	data_array[0] = 0x00100500; // Sleep In
	dsi_set_cmdq(&data_array, 1, 1);
	MDELAY(200);

}


static void lcm_resume(void)
{
   //1 do lcm init again to solve some display issue

	SET_RESET_PIN(1);
	SET_RESET_PIN(0);
	MDELAY(1);
	
	SET_RESET_PIN(1);
	MDELAY(20);      

	init_lcm_registers();

}
         
#if (LCM_DSI_CMD_MODE)
static void lcm_update(unsigned int x, unsigned int y,
                       unsigned int width, unsigned int height)
{
	unsigned int x0 = x;
	unsigned int y0 = y;
	unsigned int x1 = x0 + width - 1;
	unsigned int y1 = y0 + height - 1;

	unsigned char x0_MSB = ((x0>>8)&0xFF);
	unsigned char x0_LSB = (x0&0xFF);
	unsigned char x1_MSB = ((x1>>8)&0xFF);
	unsigned char x1_LSB = (x1&0xFF);
	unsigned char y0_MSB = ((y0>>8)&0xFF);
	unsigned char y0_LSB = (y0&0xFF);
	unsigned char y1_MSB = ((y1>>8)&0xFF);
	unsigned char y1_LSB = (y1&0xFF);

	unsigned int data_array[16];

	data_array[0]= 0x00053902;
	data_array[1]= (x1_MSB<<24)|(x0_LSB<<16)|(x0_MSB<<8)|0x2a;
	data_array[2]= (x1_LSB);
	dsi_set_cmdq(&data_array, 3, 1);
	
	data_array[0]= 0x00053902;
	data_array[1]= (y1_MSB<<24)|(y0_LSB<<16)|(y0_MSB<<8)|0x2b;
	data_array[2]= (y1_LSB);
	dsi_set_cmdq(&data_array, 3, 1);

	data_array[0]= 0x00290508; //HW bug, so need send one HS packet
	dsi_set_cmdq(&data_array, 1, 1);
	
	data_array[0]= 0x002c3909;
	dsi_set_cmdq(&data_array, 1, 0);

}
#endif

static unsigned int lcm_compare_id(void)
{
	unsigned int id0,id1,id=0;
	unsigned char buffer[2];
	unsigned int array[16];  

	SET_RESET_PIN(1);
	SET_RESET_PIN(0);
	MDELAY(1);
	SET_RESET_PIN(1);
	MDELAY(20);

	array[0] = 0x00043902;                          
	array[1] = 0x9483ffb9;                 
	dsi_set_cmdq(array, 2, 1);
	  
	array[0] = 0x00123902;                          
	array[1] = 0x008313ba;      
	array[2] = 0x0811a616;   
	array[3] = 0x03240fff;   
	array[4] = 0x20252421;   
	array[5] = 0x00001002;   	  
	dsi_set_cmdq(array, 6, 1);
	  
	MDELAY(1);
	memset(buffer, 0, sizeof(buffer));
	array[0] = 0x00023700;// return byte number
	dsi_set_cmdq(&array, 1, 1);
	//MDELAY(5);

	read_reg_v2(0xf4, buffer, 1);
	id=buffer[0];
    #ifdef BUILD_LK
	printf("%s, LK hx8394a id0 = 0x%08x\n", __func__, id0);
	printf("%s, LK hx8394a id1 = 0x%08x\n", __func__, id1);
	printf("%s, LK hx8394a id = 0x%08x\n", __func__, id);
   #else
	printk("%s, Kernel hx8394a id0 = 0x%08x\n", __func__, id0);
	printk("%s, Kernel hx8394a id1 = 0x%08x\n", __func__, id1);
	printk("%s, Kernel hx8394a id = 0x%08x\n", __func__, id);
   #endif

	return (LCM_ID_HX8394 == id)?1:0;
}

#if 0
static unsigned int lcm_esd_check(void)
{
#ifndef BUILD_LK
	char  buffer[3];
	int   array[4];

	if(lcm_esd_test)
	{
		lcm_esd_test = FALSE;
		return TRUE;
	}

	/// please notice: the max return packet size is 1
	/// if you want to change it, you can refer to the following marked code
	/// but read_reg currently only support read no more than 4 bytes....
	/// if you need to read more, please let BinHan knows.
	/*
			unsigned int data_array[16];
			unsigned int max_return_size = 1;
			
			data_array[0]= 0x00003700 | (max_return_size << 16);	
			
			dsi_set_cmdq(&data_array, 1, 1);
	*/
	array[0] = 0x00023700;// read id return two byte,version and id
	dsi_set_cmdq(array, 1, 1);

	read_reg_v2(0xA1, buffer, 2);
	if(buffer[0]==0x20 && buffer[1] == 0x75)
	{
		return FALSE;
	}
	else
	{			 
		return TRUE;
	}
#endif

}

static unsigned int lcm_esd_recover(void)
{
	lcm_init();
	lcm_resume();

	return TRUE;
}
#endif

LCM_DRIVER hx8394a_dsi_vdo_lcm_drv = 
{
    .name			= "hx8394a_dsi_vdo",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,
	.compare_id    = lcm_compare_id,
//	.esd_check = lcm_esd_check,
//	.esd_recover = lcm_esd_recover,
    #if (LCM_DSI_CMD_MODE)
    .update         = lcm_update,
    #endif
    };
