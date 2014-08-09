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

#include <typedefs.h>
#include <platform.h>
#include <mt_emi.h>
#include <emi_hw.h>
#include <dramc.h>
#include <platform.h>
#include <mtk_pll.h>


extern int num_of_emi_records;
extern EMI_SETTINGS emi_settings[];

/* select the corresponding memory devices */
extern int dramc_calib(void);

/*
 * init_dram: Do initialization for LPDDR.
 */
static void init_dram1(EMI_SETTINGS *emi_setting) 
{
    return;
}

#define EMI_BASE                    0x10203000
#define DRAMC0_BASE                 0x10004000
#define DDRPHY_BASE                 0x10011000
#define DRAMC_NAO_BASE              0x1020F000
#define EMI_CONA                    0x10203000
#define EMI_CONM                    0x10203060
#define EMI_CONF                    0x10203028

/*
 * init_dram: Do initialization for LPDDR2.
 */
static void init_dram2(EMI_SETTINGS *emi_setting) 
{
    unsigned int reg_val;
    unsigned int factor_val;
/* number of RA, CA, BA */
*(volatile unsigned *)(EMI_CONA    ) = emi_setting->EMI_CONA_VAL; /*one rank:0x0000A3AE. dual rank = 0x0002A3AE*/
*(volatile unsigned *)(EMI_CONM    ) = 0x00000500; 
*(volatile unsigned *)(EMI_CONF    ) = 0x00421000; /*Enable EMI address scrambling*/

#ifdef MEMPLL_CLK_533
    printf("[MEM] 1066 MHZ\n");
#endif
#ifdef MEMPLL_CLK_400
    printf("[MEM] 800 MHZ\n");
#endif
#ifdef MEMPLL_CLK_250
    printf("[MEM] 500 MHZ\n");
#endif
#ifdef MEMPLL_CLK_586
#define MEMPLL_CLK_533
        printf("[MEM] 1172 MHZ\n");
#endif


#ifdef DRAMC_ASYNC
    *((volatile unsigned *)(DRAMC0_BASE + 0x00fc)) = emi_setting->DRAMC_MISCTL0_VAL;  //ASYNC
    *((volatile unsigned *)(DDRPHY_BASE + 0x00fc)) = emi_setting->DRAMC_MISCTL0_VAL;  //ASYNC
#else
    *((volatile unsigned *)(DRAMC0_BASE + 0x00fc)) = emi_setting->DRAMC_MISCTL0_VAL;
    *((volatile unsigned *)(DDRPHY_BASE + 0x00fc)) = emi_setting->DRAMC_MISCTL0_VAL;
#endif

    *((volatile unsigned *)(DRAMC0_BASE + 0x0048)) = 0x0000110d;
    *((volatile unsigned *)(DDRPHY_BASE + 0x0048)) = 0x0000110d;

    *((volatile unsigned *)(DRAMC0_BASE + 0x00d8)) = 0x80500900;    //[31:30]pinmux1-0="10"
    *((volatile unsigned *)(DDRPHY_BASE + 0x00d8)) = 0x80500900;    //[31:30]pinmux1-0="10"

    *((volatile unsigned *)(DRAMC0_BASE + 0x00f0)) = 0x80000000;     //[31]4 bit swap =1
    *((volatile unsigned *)(DDRPHY_BASE + 0x00f0)) = 0x80000000;     //[31]4 bit swap =1

#ifdef DRAMC_ASYNC
    *((volatile unsigned *)(DRAMC0_BASE + 0x00e4)) = 0x00000010;
    *((volatile unsigned *)(DDRPHY_BASE + 0x00e4)) = 0x00000010;
#else
    *((volatile unsigned *)(DRAMC0_BASE + 0x00e4)) = 0x00000000;
    *((volatile unsigned *)(DDRPHY_BASE + 0x00e4)) = 0x00000000;
#endif

    *((volatile unsigned *)(DRAMC0_BASE + 0x008c)) = 0x00000001;
    *((volatile unsigned *)(DDRPHY_BASE + 0x008c)) = 0x00000001; //modify

    *((volatile unsigned *)(DRAMC0_BASE + 0x0090)) = 0x00000000;
    *((volatile unsigned *)(DDRPHY_BASE + 0x0090)) = 0x00000000;

    *((volatile unsigned *)(DRAMC0_BASE + 0x0094)) = 0x80000000;
    *((volatile unsigned *)(DDRPHY_BASE + 0x0094)) = 0x80000000;

    *((volatile unsigned *)(DRAMC0_BASE + 0x00dc)) = 0x83100100;
    *((volatile unsigned *)(DDRPHY_BASE + 0x00dc)) = 0x83100100;

#ifdef SYSTEM_26M //Edwin
    *((volatile unsigned *)(DRAMC0_BASE + 0x00e0)) = 0x1b004004;
    *((volatile unsigned *)(DDRPHY_BASE + 0x00e0)) = 0x1b004004;

    *((volatile unsigned *)(DRAMC0_BASE + 0x0124)) = 0xaa080022;
    *((volatile unsigned *)(DDRPHY_BASE + 0x0124)) = 0xaa080022;
#else

    *((volatile unsigned *)(DRAMC0_BASE + 0x00e0)) = 0x1b004004;
    *((volatile unsigned *)(DDRPHY_BASE + 0x00e0)) = 0x1b004004;

    #ifdef DRAMC_ASYNC
        *((volatile unsigned *)(DRAMC0_BASE + 0x0124)) = 0xaa0800aa;    //postsim ;124
        *((volatile unsigned *)(DDRPHY_BASE + 0x0124)) = 0xaa0800aa;    //postsim
    #else
        *((volatile unsigned *)(DRAMC0_BASE + 0x0124)) = 0xaa0800aa;    //postsim
        *((volatile unsigned *)(DDRPHY_BASE + 0x0124)) = 0xaa0800aa;    //postsim
    #endif

   #ifdef MEMPLL_CLK_500
       *((volatile unsigned *)(DRAMC0_BASE + 0x0018)) = 0x1B181B17;  // gating window fine tune value
       *((volatile unsigned *)(DDRPHY_BASE + 0x0018)) = 0x1B181B17; 
       *((volatile unsigned *)(DRAMC0_BASE + 0x001c)) = 0x1B181B17; 
       *((volatile unsigned *)(DDRPHY_BASE + 0x001c)) = 0x1B181B17; 
   #endif

   #ifdef MEMPLL_CLK_533
       *((volatile unsigned *)(DRAMC0_BASE + 0x0018)) = 0x1B181B17; 
       *((volatile unsigned *)(DDRPHY_BASE + 0x0018)) = 0x1B181B17; 
       *((volatile unsigned *)(DRAMC0_BASE + 0x001c)) = 0x1B181B17; 
       *((volatile unsigned *)(DDRPHY_BASE + 0x001c)) = 0x1B181B17; 
   #endif

   #ifdef MEMPLL_CLK_400
       *((volatile unsigned *)(DRAMC0_BASE + 0x0018)) = 0x20202020; 
       *((volatile unsigned *)(DDRPHY_BASE + 0x0018)) = 0x20202020; 
       *((volatile unsigned *)(DRAMC0_BASE + 0x001c)) = 0x20202020; 
       *((volatile unsigned *)(DDRPHY_BASE + 0x001c)) = 0x20202020; 
   #endif

   #ifdef MEMPLL_CLK_250
       *((volatile unsigned *)(DRAMC0_BASE + 0x0018)) = 0x40404040; 
       *((volatile unsigned *)(DDRPHY_BASE + 0x0018)) = 0x40404040; 
       *((volatile unsigned *)(DRAMC0_BASE + 0x001c)) = 0x40404040; 
       *((volatile unsigned *)(DDRPHY_BASE + 0x001c)) = 0x40404040; 
   #endif
#endif


    #ifdef DRAMC_ASYNC
        *((volatile unsigned *)(DRAMC0_BASE + 0x00f4)) = emi_setting->DRAMC_GDDR3CTL1_VAL;    
        *((volatile unsigned *)(DDRPHY_BASE + 0x00f4)) = emi_setting->DRAMC_GDDR3CTL1_VAL;
    #else
        *((volatile unsigned *)(DRAMC0_BASE + 0x00f4)) = emi_setting->DRAMC_GDDR3CTL1_VAL;    //3PLL sync mode
        *((volatile unsigned *)(DDRPHY_BASE + 0x00f4)) = emi_setting->DRAMC_GDDR3CTL1_VAL;
    #endif

    *((volatile unsigned *)(DRAMC0_BASE + 0x0168)) = 0x00000010;
    *((volatile unsigned *)(DDRPHY_BASE + 0x0168)) = 0x00000010;

//    *((volatile unsigned *)(DRAMC0_BASE + 0x00d8)) = 0x00700900;
//    *((volatile unsigned *)(DDRPHY_BASE + 0x00d8)) = 0x00700900;


    *((volatile unsigned *)(DRAMC0_BASE + 0x0004)) = emi_setting->DRAMC_CONF1_VAL;
    *((volatile unsigned *)(DDRPHY_BASE + 0x0004)) = emi_setting->DRAMC_CONF1_VAL;


    #ifdef DRAMC_ASYNC
      *((volatile unsigned *)(DRAMC0_BASE + 0x007c)) = 0xc0063211;    //DLE,after ECO
      *((volatile unsigned *)(DDRPHY_BASE + 0x007c)) = 0xc0063211;
    #else
      *((volatile unsigned *)(DRAMC0_BASE + 0x007c)) = emi_setting->DRAMC_DDR2CTL_VAL;    //performance, postsim
      *((volatile unsigned *)(DDRPHY_BASE + 0x007c)) = emi_setting->DRAMC_DDR2CTL_VAL;    //performance, postsim
    #endif

    *((volatile unsigned *)(DRAMC0_BASE + 0x0028)) = 0xf1200f01;
    *((volatile unsigned *)(DDRPHY_BASE + 0x0028)) = 0xf1200f01;

    *((volatile unsigned *)(DRAMC0_BASE + 0x01e0)) = 0x30003fff;  //performance
    *((volatile unsigned *)(DDRPHY_BASE + 0x01e0)) = 0x30003fff;  //performance

    *((volatile unsigned *)(DRAMC0_BASE + 0x0158)) = 0x00000000;
    *((volatile unsigned *)(DDRPHY_BASE + 0x0158)) = 0x00000000;


    *((volatile unsigned *)(DRAMC0_BASE + 0x0400)) = 0x00111100;
    *((volatile unsigned *)(DDRPHY_BASE + 0x0400)) = 0x00111100;

    *((volatile unsigned *)(DRAMC0_BASE + 0x0404)) = 0x00000002;
    *((volatile unsigned *)(DDRPHY_BASE + 0x0404)) = 0x00000002;

    *((volatile unsigned *)(DRAMC0_BASE + 0x0408)) = 0x00222222;
    *((volatile unsigned *)(DDRPHY_BASE + 0x0408)) = 0x00222222;

    *((volatile unsigned *)(DRAMC0_BASE + 0x040c)) = 0x33330000;
    *((volatile unsigned *)(DDRPHY_BASE + 0x040c)) = 0x33330000;

    *((volatile unsigned *)(DRAMC0_BASE + 0x0410)) = 0x33330000;
    *((volatile unsigned *)(DDRPHY_BASE + 0x0410)) = 0x33330000;


//************************************************/
//******** IO driving setting & VREF setting  ****/
//************************************************/
//*TT:
//E_PRE=1
//DRVP_PRE[1:0]=(1,1)
//DRVN_PRE[1:0]=(1,1)
//DRVP[3:0]=(1,0,0,1)
//DRVN[3: 0]=(1,0,0,1)

//E_PRE=1
// set E_PRE in MEMPLL
//    *((volatile unsigned *)(DRAMC0_BASE + 0x0640)) = 0x00003f03;   //[13:8]="111111",default: 0x00000003
//    *((volatile unsigned *)(DDRPHY_BASE + 0x0640)) = 0x00003f03;
//DRVP[3:0]=(1,0,0,1)
//DRVN[3:0]=(1,0,0,1)
//DQ16-31 [15:12]DRVP/[11:8]DRVN
    *((volatile unsigned *)(DRAMC0_BASE + 0x00b4)) = emi_setting->DRAMC_DRVCTL0_VAL;   //[15:12]DRVP/[11:8]DRVN, default: 0xaa22aa22
    *((volatile unsigned *)(DDRPHY_BASE + 0x00b4)) = emi_setting->DRAMC_DRVCTL0_VAL;
    //*((volatile unsigned *)(DRAMC0_BASE + 0x00b4)) = 0xaa229922;   //[15:12]DRVP/[11:8]DRVN, default: 0xaa22aa22
    //*((volatile unsigned *)(DDRPHY_BASE + 0x00b4)) = 0xaa229922;

//DQS [31:28]DRVP/[27:24]DRVN
//DQ0-15 [15:12]DRVP/[11:8]DRVN
    *((volatile unsigned *)(DRAMC0_BASE + 0x00b8)) = emi_setting->DRAMC_DRVCTL0_VAL;   //[31:28]DQS DRVP/[27:24]DQS DRVN;[15:12]DQ DRVP/[11:8]DQ DRVN, default: 0xaa22aa22
    *((volatile unsigned *)(DDRPHY_BASE + 0x00b8)) = emi_setting->DRAMC_DRVCTL0_VAL;
    //*((volatile unsigned *)(DRAMC0_BASE + 0x00b8)) = 0x99229922;   //[31:28]DQS DRVP/[27:24]DQS DRVN;[15:12]DQ DRVP/[11:8]DQ DRVN, default: 0xaa22aa22
    //*((volatile unsigned *)(DDRPHY_BASE + 0x00b8)) = 0x99229922;

//CLK [31:28]DRVP/[27:24]DRVN;
//CMD [15:12]DRVP/[11:8]DRVN
    *((volatile unsigned *)(DRAMC0_BASE + 0x00bc)) = 0xCC00CC00;   //[31:28]CLK DRVP/[27:24]CLK DRVN;[15:12]CMD DRVP/[11:8]CMD DRVN;, default: 0xaa22aa22
    *((volatile unsigned *)(DDRPHY_BASE + 0x00bc)) = 0xCC00CC00;
    //*((volatile unsigned *)(DRAMC0_BASE + 0x00bc)) = 0x99229922;   //[31:28]CLK DRVP/[27:24]CLK DRVN;[15:12]CMD DRVP/[11:8]CMD DRVN;, default: 0xaa22aa22
    //*((volatile unsigned *)(DDRPHY_BASE + 0x00bc)) = 0x99229922;
//CMP
    *((volatile unsigned *)(DRAMC0_BASE + 0x00c0)) = 0x00000000;   //disable CMP;default: 0x00000000
    *((volatile unsigned *)(DDRPHY_BASE + 0x00c0)) = 0x00000000;
//VERF
    *((volatile unsigned *)(DRAMC0_BASE + 0x0644)) = 0x00000000;   //disable internal VREF; default: 0x00000000
    *((volatile unsigned *)(DDRPHY_BASE + 0x0644)) = 0x00000000;
//test agent

    *((volatile unsigned *)(DRAMC_NAO_BASE + 0x03C)) = 0x00000000;   //disable internal VREF; default: 0x00000000
    *((volatile unsigned *)(DRAMC_NAO_BASE + 0x040)) = 0x000007FF;   //disable internal VREF; default: 0x00000000

//***************************************************/
//******** IO driving setting & VREF setting End ****/
//***************************************************/

#ifdef SINGLE_RANK_MODE
    *((volatile unsigned *)(DRAMC0_BASE + 0x0110)) = 0x0b051100;  //[2:0] rankmode=0, [4]mrs2rk=0
    *((volatile unsigned *)(DDRPHY_BASE + 0x0110)) = 0x0b051100;
#else
    *((volatile unsigned *)(DRAMC0_BASE + 0x0110)) = 0x0b051111;  //[2:0] rankmode=1, [4]mrs2rk=1
    *((volatile unsigned *)(DDRPHY_BASE + 0x0110)) = 0x0b051111;
#endif


#ifdef DRAMC_ASYNC
    *((volatile unsigned *)(DRAMC0_BASE + 0x00e4)) = 0x00000015;   //CKEBYCTL
    *((volatile unsigned *)(DDRPHY_BASE + 0x00e4)) = 0x00000015;   //CKEBYCTL
#else
    *((volatile unsigned *)(DRAMC0_BASE + 0x00e4)) = 0x00000005;   //CKEBYCTL
    *((volatile unsigned *)(DDRPHY_BASE + 0x00e4)) = 0x00000005;   //CKEBYCTL
#endif

    __asm__ __volatile__ ("dsb" : : : "memory");
    gpt_busy_wait_us(200);//Wait > 200us
    *((volatile unsigned *)(DRAMC0_BASE + 0x0088)) = 0x0000003f;
    *((volatile unsigned *)(DDRPHY_BASE + 0x0088)) = 0x0000003f;

    *((volatile unsigned *)(DRAMC0_BASE + 0x01e4)) = 0x00000001;
    *((volatile unsigned *)(DDRPHY_BASE + 0x01e4)) = 0x00000001;

    __asm__ __volatile__ ("dsb" : : : "memory");
    gpt_busy_wait_us(10);//Wait > 10us
    *((volatile unsigned *)(DRAMC0_BASE + 0x01e4)) = 0x00000000;
    *((volatile unsigned *)(DDRPHY_BASE + 0x01e4)) = 0x00000000;


    #if 1
        //for chip select 0: ZQ calibration
        *((volatile unsigned *)(DRAMC0_BASE + 0x0110)) &= (~0x7);
        *((volatile unsigned *)(DDRPHY_BASE + 0x0110)) &= (~0x7);
  
  
        *((volatile unsigned *)(DRAMC0_BASE + 0x0088)) = 0x00ff000a;
        *((volatile unsigned *)(DDRPHY_BASE + 0x0088)) = 0x00ff000a;
  
        *((volatile unsigned *)(DRAMC0_BASE + 0x01e4)) = 0x00000001;
        *((volatile unsigned *)(DDRPHY_BASE + 0x01e4)) = 0x00000001;
  
        __asm__ __volatile__ ("dsb" : : : "memory");
        gpt_busy_wait_us(1);//Wait > 1us
        *((volatile unsigned *)(DRAMC0_BASE + 0x01e4)) = 0x00000000;
        *((volatile unsigned *)(DDRPHY_BASE + 0x01e4)) = 0x00000000;
        if ( emi_setting->EMI_CONA_VAL & 20000)
        { 
            //for chip select 1: ZQ calibration
            *((volatile unsigned *)(DRAMC0_BASE + 0x0110)) |= (0x8);
            *((volatile unsigned *)(DDRPHY_BASE + 0x0110)) |= (0x8);

            *((volatile unsigned *)(DRAMC0_BASE + 0x0088)) = 0x00ff000a;
            *((volatile unsigned *)(DDRPHY_BASE + 0x0088)) = 0x00ff000a;

            *((volatile unsigned *)(DRAMC0_BASE + 0x01e4)) = 0x00000001;
            *((volatile unsigned *)(DDRPHY_BASE + 0x01e4)) = 0x00000001;

            __asm__ __volatile__ ("dsb" : : : "memory");
            gpt_busy_wait_us(1);//Wait > 100us
            *((volatile unsigned *)(DRAMC0_BASE + 0x01e4)) = 0x00000000;
            *((volatile unsigned *)(DDRPHY_BASE + 0x01e4)) = 0x00000000;
            //swap back
            *((volatile unsigned *)(DRAMC0_BASE + 0x0110)) &= (~0x8);
            *((volatile unsigned *)(DDRPHY_BASE + 0x0110)) &= (~0x8);
#ifndef SINGLE_RANK_MODE
            *((volatile unsigned *)(DRAMC0_BASE + 0x0110)) |= (0x1);
            *((volatile unsigned *)(DDRPHY_BASE + 0x0110)) |= (0x1);
#endif
        }
    #endif

    __asm__ __volatile__ ("dsb" : : : "memory");
    gpt_busy_wait_us(1);//Wait > 1us


    *((volatile unsigned *)(DRAMC0_BASE + 0x0088)) = emi_setting->DDR2_MODE_REG_1;
    *((volatile unsigned *)(DDRPHY_BASE + 0x0088)) = emi_setting->DDR2_MODE_REG_1;

    *((volatile unsigned *)(DRAMC0_BASE + 0x01e4)) = 0x00000001;
    *((volatile unsigned *)(DDRPHY_BASE + 0x01e4)) = 0x00000001;

    __asm__ __volatile__ ("dsb" : : : "memory");
    gpt_busy_wait_us(1);//Wait > 1us
    *((volatile unsigned *)(DRAMC0_BASE + 0x01e4)) = 0x00000000;
    *((volatile unsigned *)(DDRPHY_BASE + 0x01e4)) = 0x00000000;


    *((volatile unsigned *)(DRAMC0_BASE + 0x0088)) = emi_setting->DDR2_MODE_REG_2;
    *((volatile unsigned *)(DDRPHY_BASE + 0x0088)) = emi_setting->DDR2_MODE_REG_2;

    *((volatile unsigned *)(DRAMC0_BASE + 0x01e4)) = 0x00000001;
    *((volatile unsigned *)(DDRPHY_BASE + 0x01e4)) = 0x00000001;

    __asm__ __volatile__ ("dsb" : : : "memory");
    gpt_busy_wait_us(1);//Wait > 1us
    *((volatile unsigned *)(DRAMC0_BASE + 0x01e4)) = 0x00000000;
    *((volatile unsigned *)(DDRPHY_BASE + 0x01e4)) = 0x00000000;

//MR3: dram driving

    *((volatile unsigned *)(DRAMC0_BASE + 0x0088)) = emi_setting->DDR2_MODE_REG_3;
    *((volatile unsigned *)(DDRPHY_BASE + 0x0088)) = emi_setting->DDR2_MODE_REG_3;

    *((volatile unsigned *)(DRAMC0_BASE + 0x01e4)) = 0x00000001;
    *((volatile unsigned *)(DDRPHY_BASE + 0x01e4)) = 0x00000001;

    __asm__ __volatile__ ("dsb" : : : "memory");
    gpt_busy_wait_us(1);//Wait > 1us
    *((volatile unsigned *)(DRAMC0_BASE + 0x01e4)) = 0x00001100;
    *((volatile unsigned *)(DDRPHY_BASE + 0x01e4)) = 0x00001100;

#ifdef SINGLE_RANK_MODE
    *((volatile unsigned *)(DRAMC0_BASE + 0x0110)) = 0x00111100;  //[2:0] rankmode=0, [4]mrs2rk=0
    *((volatile unsigned *)(DDRPHY_BASE + 0x0110)) = 0x00111100;
#else
    *((volatile unsigned *)(DRAMC0_BASE + 0x0110)) = 0x00111301;  //[2:0] rankmode=1, [4]mrs2rk=0
    *((volatile unsigned *)(DDRPHY_BASE + 0x0110)) = 0x00111301;
#endif


#ifdef DRAMC_ASYNC
    *((volatile unsigned *)(DRAMC0_BASE + 0x00e4)) = 0x00000011;  //CKEBYCTL
    *((volatile unsigned *)(DDRPHY_BASE + 0x00e4)) = 0x00000011;  //CKEBYCTL
#else
#ifdef  MEMPLL_CLK_533
    *((volatile unsigned *)(DRAMC0_BASE + 0x00e4)) = 0x00000011;  //CKEBYCTL
    *((volatile unsigned *)(DDRPHY_BASE + 0x00e4)) = 0x00000011;  //CKEBYCTL
#else
    *((volatile unsigned *)(DRAMC0_BASE + 0x00e4)) = 0x00000001;  //CKEBYCTL
    *((volatile unsigned *)(DDRPHY_BASE + 0x00e4)) = 0x00000001;  //CKEBYCTL
#endif
#endif
    *((volatile unsigned *)(DRAMC0_BASE + 0x01ec)) = 0x00000001;    //for performance
    *((volatile unsigned *)(DDRPHY_BASE + 0x01ec)) = 0x00000001;    //for performance

    *((volatile unsigned *)(DRAMC0_BASE + 0x0084)) = 0x00000a56;
    *((volatile unsigned *)(DDRPHY_BASE + 0x0084)) = 0x00000a56;

    *((volatile unsigned *)(DRAMC0_BASE + 0x01e4)) = 0x00000000;
    *((volatile unsigned *)(DDRPHY_BASE + 0x01e4)) = 0x00000000;

    *((volatile unsigned *)(DRAMC0_BASE + 0x000c)) = 0x00000000;
    *((volatile unsigned *)(DDRPHY_BASE + 0x000c)) = 0x00000000;

    *((volatile unsigned *)(DRAMC0_BASE + 0x0000)) = emi_setting->DRAMC_ACTIM_VAL;
    *((volatile unsigned *)(DDRPHY_BASE + 0x0000)) = emi_setting->DRAMC_ACTIM_VAL;

    *((volatile unsigned *)(DRAMC0_BASE + 0x01dc)) = emi_setting->DRAMC_PD_CTRL_VAL;    //for REFCNT_FR_CLK
    *((volatile unsigned *)(DDRPHY_BASE + 0x01dc)) = emi_setting->DRAMC_PD_CTRL_VAL;    //for REFCNT_FR_CLK
    //*((volatile unsigned *)(DRAMC0_BASE + 0x01dc)) = 0x10672342;    //for REFCNT_FR_CLK
    //*((volatile unsigned *)(DDRPHY_BASE + 0x01dc)) = 0x10672342;    //for REFCNT_FR_CLK

    *((volatile unsigned *)(DRAMC0_BASE + 0x0044)) = emi_setting->DRAMC_TEST2_3_VAL;    //for performance
    *((volatile unsigned *)(DDRPHY_BASE + 0x0044)) = emi_setting->DRAMC_TEST2_3_VAL;    //for performance


    *((volatile unsigned *)(DRAMC0_BASE + 0x01e8)) = emi_setting->DRAMC_ACTIM1_VAL;     //For tRFCpb=90, 8Gb lpddr2
    *((volatile unsigned *)(DDRPHY_BASE + 0x01e8)) = emi_setting->DRAMC_ACTIM1_VAL;


    *((volatile unsigned *)(DRAMC0_BASE + 0x0008)) = emi_setting->DRAMC_CONF2_VAL;
    *((volatile unsigned *)(DDRPHY_BASE + 0x0008)) = emi_setting->DRAMC_CONF2_VAL;

    *((volatile unsigned *)(DRAMC0_BASE + 0x0010)) = 0x0000EEEE;
    *((volatile unsigned *)(DDRPHY_BASE + 0x0010)) = 0x0000EEEE;

    *((volatile unsigned *)(DRAMC0_BASE + 0x0084)) = 0x00000a56;
    *((volatile unsigned *)(DDRPHY_BASE + 0x0084)) = 0x00000a56;

    *((volatile unsigned *)(DRAMC0_BASE + 0x00f8)) = 0xedcb000f;
    *((volatile unsigned *)(DDRPHY_BASE + 0x00f8)) = 0xedcb000f;

    *((volatile unsigned *)(DRAMC0_BASE + 0x0020)) = 0x00000000;
    *((volatile unsigned *)(DDRPHY_BASE + 0x0020)) = 0x00000000;
    //DQS output delay
    *((volatile unsigned *)(DRAMC0_BASE + 0x0014)) = emi_setting->DRAMC_PADCTL3_VAL;
    *((volatile unsigned *)(DDRPHY_BASE + 0x0014)) = emi_setting->DRAMC_PADCTL3_VAL;

  #ifdef MEMPLL_CLK_500
      //fine tune value
      *((volatile unsigned *)(DRAMC0_BASE + 0x094)) = 0x56565656;
      *((volatile unsigned *)(DDRPHY_BASE + 0x094)) = 0x56565656;
  #elif defined(MEMPLL_CLK_533)
      //fine tune value
      *((volatile unsigned *)(DRAMC0_BASE + 0x094)) = 0x48484848;
      *((volatile unsigned *)(DDRPHY_BASE + 0x094)) = 0x48484848;
      *((volatile unsigned *)(DRAMC0_BASE + 0x098)) = 0x48484848;
      *((volatile unsigned *)(DDRPHY_BASE + 0x098)) = 0x48484848;
  #else
      *((volatile unsigned *)(DRAMC0_BASE + 0x094)) = 0xA8282828;
      *((volatile unsigned *)(DDRPHY_BASE + 0x094)) = 0xA8282828;
  #endif


  #ifdef MEMPLL_CLK_533
      // DQ output delay
      *((volatile unsigned *)(DDRPHY_BASE + 0x0200)) = emi_setting->DRAMC_DQODLY_VAL;
      *((volatile unsigned *)(DDRPHY_BASE + 0x0204)) = emi_setting->DRAMC_DQODLY_VAL;
      *((volatile unsigned *)(DDRPHY_BASE + 0x0208)) = emi_setting->DRAMC_DQODLY_VAL;
      *((volatile unsigned *)(DDRPHY_BASE + 0x020C)) = emi_setting->DRAMC_DQODLY_VAL;


      // DQ input delay
      *((volatile unsigned *)(DRAMC0_BASE + 0x0210)) = 0x03020101;
      *((volatile unsigned *)(DRAMC0_BASE + 0x0214)) = 0x00020206;
      *((volatile unsigned *)(DRAMC0_BASE + 0x0218)) = 0x02000708;
      *((volatile unsigned *)(DRAMC0_BASE + 0x021c)) = 0x03010105;
      *((volatile unsigned *)(DRAMC0_BASE + 0x0220)) = 0x05080005;
      *((volatile unsigned *)(DRAMC0_BASE + 0x0224)) = 0x03040a07;
      *((volatile unsigned *)(DRAMC0_BASE + 0x0228)) = 0x02030203;
      *((volatile unsigned *)(DRAMC0_BASE + 0x022c)) = 0x04040006;
  #endif


}

/*
 * init_dram: Do initialization for DDR3.
 */
#define DDR3_MODE_0 0x00000000
#define DDR3_MODE_1 0x00400000
#define DDR3_MODE_2 0x00800000
#define DDR3_MODE_3 0x00c00000
unsigned int ddr_type=0;

//#ifdef MTK_DDR3_SUPPORT
#define EMI_BASE           0x10203000
#define DRAMC0_BASE        0x10004000
#define DDRPHY_BASE        0x10011000
#define DRAMC_NAO_BASE     0x1020F000
#define EMI_CONA           0x10203000
#define EMI_CONM           0x10203060
#define EMI_TESTB          0x102030E8
#define EMI_TESTD          0x102030F8

//#define DRAMC_ASYNC 
//#define DDR3_16B
#define SINGLE_RANK_MODE
//#define DDR3_SCRAMBLE_SUPPORT
//#define DDR3_ZQCS_SUPPORT
//#define DDR3_AUTOREF_SUPPORT
//#define A15_ENABLE  //need use in 2 rank mode
//#endif
static void init_dram3(EMI_SETTINGS *emi_setting) 
{
//#ifdef MTK_DDR3_SUPPORT

  #ifdef DDR3_16B
  dbg_print("init ddr3 x16\n");
  #else
  dbg_print("init ddr3 x32\n");
  #endif

  delay_a_while(2000000);//test 20121015
  dbg_print("Delay test\n");
#ifdef SINGLE_RANK_MODE
	#ifdef DDR3_16B
    //*(volatile unsigned int*)(EMI_CONA) = 0x00002128; //dw32_en=0, row 14bit, bank 3bit, column 10bit : 2Gb x16 
    //*(volatile unsigned int*)(EMI_CONA) = 0x11303128;  //dw32_en=0, row 15bit, bank 3bit, column 10bit : 4Gb x16   
	#else
    //*(volatile unsigned int*)(EMI_CONA) = 0x0000212A;  //dw32_en=1, row 14bit, bank 3bit, column 10bit : 2Gb x16 + 2Gb x16
    *(volatile unsigned int*)(EMI_CONA) = emi_setting->EMI_CONA_VAL;   //dw32_en=1, row 15bit, bank 3bit, column 10bit : 4Gb x16 + 4Gb x16  
	#endif
#else
	#ifdef DDR3_16B
	//*(volatile unsigned int*)(EMI_CONA) = 0x0002A3A8; //dw32_en=0, row 14bit, bank 3bit, column 10bit : 2Gb x16 
	*(volatile unsigned int*)(EMI_CONA) = emi_setting->EMI_CONA_VAL;  //dw32_en=0, row 15bit, bank 3bit, column 10bit : 4Gb x16	 
	#else
	//*(volatile unsigned int*)(EMI_CONA) = 0x0002A3AA;  //dw32_en=1, row 14bit, bank 3bit, column 10bit : 2Gb x16 + 2Gb x16
	*(volatile unsigned int*)(EMI_CONA) = emi_setting->EMI_CONA_VAL;   //dw32_en=1, row 15bit, bank 3bit, column 10bit : 4Gb x16 + 4Gb x16  
	#endif

	#ifdef A15_ENABLE
	*(volatile unsigned int*)(EMI_TESTB) = 0x80; //rank bit as A15
	#endif
#endif

  *(volatile unsigned *)(EMI_CONF    ) = 0x00421000; /*Enable EMI address scrambling*/

    *(volatile unsigned int*)(EMI_CONM) = 0x500; //enable EMI/DRAMC ACCESS

#ifdef DDR3_SCRAMBLE_SUPPORT
	#ifdef DDR3_16B
	*(volatile unsigned int*)(EMI_TESTD) = 0x80000000; //bit31 64bit DLE enable
	#endif
	*(volatile unsigned int *)(DRAMC0_BASE + (0x0100)) = 0x18880000; //bit28 data scramble enable
#endif

    *(volatile unsigned int *)(DRAMC0_BASE + (0x0048)) = 0x1e00110d;
    *(volatile unsigned int *)(DDRPHY_BASE + (0x0048)) = 0x1e00110d;

#ifdef DDR3_NO_PINMUX
    *(volatile unsigned int *)(DRAMC0_BASE + (0x00D8)) = 0x00500900;
    *(volatile unsigned int *)(DDRPHY_BASE + (0x00D8)) = 0x00500900;
#else
    *(volatile unsigned int *)(DRAMC0_BASE + (0x00D8)) = 0x40500900;
    *(volatile unsigned int *)(DDRPHY_BASE + (0x00D8)) = 0x40500900;
#endif

    *(volatile unsigned int *)(DRAMC0_BASE + (0x00E4)) = 0x000000a3;  //CKEBYCTL
    *(volatile unsigned int *)(DDRPHY_BASE + (0x00E4)) = 0x000000a3;  //reset on
    //delay_a_while(2000);//24.5us, DDR3-1066
	//delay_a_while(12000);//104.5us,  DDR3-1066
	  delay_a_while(11500);//100.5us, DDR3-1066
	
    *(volatile unsigned int *)(DRAMC0_BASE + (0x0090)) = 0x00000000;
    *(volatile unsigned int *)(DDRPHY_BASE + (0x0090)) = 0x00000000;

    *(volatile unsigned int *)(DRAMC0_BASE + (0x0094)) = 0x80000000;
    *(volatile unsigned int *)(DDRPHY_BASE + (0x0094)) = 0x80000000;

    *(volatile unsigned int *)(DRAMC0_BASE + (0x00DC)) = 0x83000000;
    *(volatile unsigned int *)(DDRPHY_BASE + (0x00DC)) = 0x83000000;

    *(volatile unsigned int *)(DRAMC0_BASE + (0x00E0)) = 0x13000000;
    *(volatile unsigned int *)(DDRPHY_BASE + (0x00E0)) = 0x13000000;

	#ifdef DRAMC_ASYNC
	*(volatile unsigned int *)(DRAMC0_BASE + (0x00F4)) =  0x01000000;
	*(volatile unsigned int *)(DDRPHY_BASE + (0x00F4)) =  0x01000000;
	#else
	*(volatile unsigned int *)(DRAMC0_BASE + (0x00F4)) =  emi_setting->DRAMC_GDDR3CTL1_VAL;
	*(volatile unsigned int *)(DDRPHY_BASE + (0x00F4)) =  emi_setting->DRAMC_GDDR3CTL1_VAL;
	#endif 
	#ifdef DRAMC_ASYNC
	 *((volatile unsigned int *)(DRAMC0_BASE + (0x00FC))) = 0x07110000;   //async_en
	 *((volatile unsigned int *)(DDRPHY_BASE + (0x00FC))) = 0x07110000;
	#else
	 *((volatile unsigned int *)(DRAMC0_BASE + (0x00FC))) = emi_setting->DRAMC_MISCTL0_VAL;
	 *((volatile unsigned int *)(DDRPHY_BASE + (0x00FC))) = emi_setting->DRAMC_MISCTL0_VAL;
	#endif

    *(volatile unsigned int *)(DRAMC0_BASE + (0x0168)) = 0x00000010;
    *(volatile unsigned int *)(DDRPHY_BASE + (0x0168)) = 0x00000010;

    *(volatile unsigned int *)(DRAMC0_BASE + (0x0130)) = 0x30000000;//clock enable
    *(volatile unsigned int *)(DDRPHY_BASE + (0x0130)) = 0x30000000;//clock enable
    delay_a_while(1000);

    *(volatile unsigned int *)(DRAMC0_BASE + (0x00D8)) = 0x40700900;
    *(volatile unsigned int *)(DDRPHY_BASE + (0x00D8)) = 0x40700900;

	#ifdef DDR3_16B
    *(volatile unsigned int *)(DRAMC0_BASE + (0x0004)) = 0xf07482e0;  //DM64BIT=0
    *(volatile unsigned int *)(DDRPHY_BASE + (0x0004)) = 0xf07482e0;
	
    *(volatile unsigned int *)(DRAMC0_BASE + (0x01EC)) = 0x00000611;//[12]cs2rank=0
    *(volatile unsigned int *)(DDRPHY_BASE + (0x01EC)) = 0x00000611;//[12]cs2rank=0

	*(volatile unsigned int *)(DRAMC0_BASE + (0x008C)) = 0x00100000;//[20]DQ16COM1
	*(volatile unsigned int *)(DDRPHY_BASE + (0x008C)) = 0x00100000;//[20]DQ16COM1

	#else
    *(volatile unsigned int *)(DRAMC0_BASE + (0x0004)) = emi_setting->DRAMC_CONF1_VAL;  //DM64BIT=1
    *(volatile unsigned int *)(DDRPHY_BASE + (0x0004)) = emi_setting->DRAMC_CONF1_VAL;
	
    *(volatile unsigned int *)(DRAMC0_BASE + (0x01EC)) = 0x00001611;//[12]cs2rank=1,for ddr3_16x2 use cs0/cs1 as 32 bit data bus
    *(volatile unsigned int *)(DDRPHY_BASE + (0x01EC)) = 0x00001611;//[12]cs2rank=1,for ddr3_16x2 use cs0/cs1 as 32 bit data bus

	*(volatile unsigned int *)(DRAMC0_BASE + (0x008C)) = 0x00000000;//[20]DQ16COM0
	*(volatile unsigned int *)(DDRPHY_BASE + (0x008C)) = 0x00000000;//[20]DQ16COM0
	#endif

    *(volatile unsigned int *)(DRAMC0_BASE + (0x0124)) = 0x80000011;
    *(volatile unsigned int *)(DDRPHY_BASE + (0x0124)) = 0x80000011;

    *(volatile unsigned int *)(DRAMC0_BASE + (0x0094)) = 0x40404040;
    *(volatile unsigned int *)(DDRPHY_BASE + (0x0094)) = 0x40404040;

    *(volatile unsigned int *)(DRAMC0_BASE + (0x01C0)) = 0x00000000;
    *(volatile unsigned int *)(DDRPHY_BASE + (0x01C0)) = 0x00000000;

    *(volatile unsigned int *)(DRAMC0_BASE + (0x007C)) = emi_setting->DRAMC_DDR2CTL_VAL;
    *(volatile unsigned int *)(DDRPHY_BASE + (0x007C)) = emi_setting->DRAMC_DDR2CTL_VAL;

    *(volatile unsigned int *)(DRAMC0_BASE + (0x0028)) = 0xf1200f01;   //[21]WCKSEL2, only pinmux use
    *(volatile unsigned int *)(DDRPHY_BASE + (0x0028)) = 0xf1200f01;

    *(volatile unsigned int *)(DRAMC0_BASE + (0x0158)) = 0x00000000;
    *(volatile unsigned int *)(DDRPHY_BASE + (0x0158)) = 0x00000000;

    *(volatile unsigned int *)(DRAMC0_BASE + (0x01E0)) = 0x08000000;
    *(volatile unsigned int *)(DDRPHY_BASE + (0x01E0)) = 0x08000000;

    *((volatile unsigned int *)(DRAMC0_BASE + (0x00E4))) = 0x000000a7;//CKE on
    *((volatile unsigned int *)(DDRPHY_BASE + (0x00E4))) = 0x000000a7;//CKE on
    delay_a_while(2000);

//************************************************/
//******** IO driving setting & VREF setting  ****/
//************************************************/
//*TT:
//E_PRE=1
//DRVP_PRE[1:0]=(1,1)
//DRVN_PRE[1:0]=(1,1)
//DRVP[3:0]=(1,0,0,1)
//DRVN[3: 0]=(1,0,0,1)

//E_PRE=1
//    *((volatile unsigned int *)(DRAMC0_BASE + 0x0640)) = 0x00003f03;   //[13:8]="111111",default: 0x00000003
//    *((volatile unsigned int *)(DDRPHY_BASE + 0x0640)) = 0x00003f03;
//DRVP[3:0]=(1,0,0,1)
//DRVN[3:0]=(1,0,0,1)
//DQ16-31 [15:12]DRVP/[11:8]DRVN
    *((volatile unsigned int *)(DRAMC0_BASE + 0x00b4)) = emi_setting->DRAMC_DRVCTL0_VAL;   //[15:12]DRVP/[11:8]DRVN, default: 0xaa22aa22
    *((volatile unsigned int *)(DDRPHY_BASE + 0x00b4)) = emi_setting->DRAMC_DRVCTL0_VAL;

//DQS [31:28]DRVP/[27:24]DRVN
//DQ0-15 [15:12]DRVP/[11:8]DRVN
    *((volatile unsigned int *)(DRAMC0_BASE + 0x00b8)) = emi_setting->DRAMC_DRVCTL0_VAL;   //[31:28]DQS DRVP/[27:24]DQS DRVN;[15:12]DQ DRVP/[11:8]DQ DRVN, default: 0xaa22aa22
    *((volatile unsigned int *)(DDRPHY_BASE + 0x00b8)) = emi_setting->DRAMC_DRVCTL0_VAL;

//CLK [31:28]DRVP/[27:24]DRVN;
//CMD [15:12]DRVP/[11:8]DRVN
    *((volatile unsigned int *)(DRAMC0_BASE + 0x00bc)) = 0x99009900;   //[31:28]CLK DRVP/[27:24]CLK DRVN;[15:12]CMD DRVP/[11:8]CMD DRVN;, default: 0xaa22aa22
    *((volatile unsigned int *)(DDRPHY_BASE + 0x00bc)) = 0x99009900;
//CMP
    *((volatile unsigned int *)(DRAMC0_BASE + 0x00c0)) = 0x00000000;   //disable CMP;default: 0x00000000
    *((volatile unsigned int *)(DDRPHY_BASE + 0x00c0)) = 0x00000000;
//VERF
    *((volatile unsigned int *)(DRAMC0_BASE + 0x0644)) = 0x00000000;   //disable internal VREF; default: 0x00000000
    *((volatile unsigned int *)(DDRPHY_BASE + 0x0644)) = 0x00000000;

//***************************************************/
//******** IO driving setting & VREF setting End ****/
//***************************************************/

#ifdef SINGLE_RANK_MODE
	#ifdef DDR3_16B
	*((volatile unsigned int *)(DRAMC0_BASE + 0x0110)) = 0x0b051111;  //[2:0] rankmode=1, [4]mrs2rk=1
	*((volatile unsigned int *)(DDRPHY_BASE + 0x0110)) = 0x0b051111;
	#else
    *((volatile unsigned int *)(DRAMC0_BASE + 0x0110)) = 0x0b051100;  //[2:0] rankmode=0, [4]mrs2rk=0
    *((volatile unsigned int *)(DDRPHY_BASE + 0x0110)) = 0x0b051100;
	#endif
#else
	#ifdef DDR3_16B
	*((volatile unsigned int *)(DRAMC0_BASE + 0x0110)) = 0x0b051111;  //[2:0] rankmode=1, [4]mrs2rk=1
	*((volatile unsigned int *)(DDRPHY_BASE + 0x0110)) = 0x0b051111;
	#else
    *((volatile unsigned int *)(DRAMC0_BASE + 0x0110)) = 0x0b051111;  //[2:0] rankmode=1, [4]mrs2rk=1
    *((volatile unsigned int *)(DDRPHY_BASE + 0x0110)) = 0x0b051111;
	#endif
#endif

    *((volatile unsigned int *)(DRAMC0_BASE + 0x0088)) = emi_setting->DDR3_MODE_REG2;
    *((volatile unsigned int *)(DDRPHY_BASE + 0x0088)) = emi_setting->DDR3_MODE_REG2;

    *((volatile unsigned int *)(DRAMC0_BASE + 0x01e4)) = 0x00000001;
    *((volatile unsigned int *)(DDRPHY_BASE + 0x01e4)) = 0x00000001;

    delay_a_while(1000);

    *((volatile unsigned int *)(DRAMC0_BASE + 0x01e4)) = 0x00000000;
    *((volatile unsigned int *)(DDRPHY_BASE + 0x01e4)) = 0x00000000;

    *((volatile unsigned int *)(DRAMC0_BASE + 0x0088)) = emi_setting->DDR3_MODE_REG3;
    *((volatile unsigned int *)(DDRPHY_BASE + 0x0088)) = emi_setting->DDR3_MODE_REG3;

    *((volatile unsigned int *)(DRAMC0_BASE + 0x01e4)) = 0x00000001;
    *((volatile unsigned int *)(DDRPHY_BASE + 0x01e4)) = 0x00000001;

    delay_a_while(1000);

    *((volatile unsigned int *)(DRAMC0_BASE + 0x01e4)) = 0x00000000;
    *((volatile unsigned int *)(DDRPHY_BASE + 0x01e4)) = 0x00000000;

    *((volatile unsigned int *)(DRAMC0_BASE + 0x0088)) = emi_setting->DDR3_MODE_REG1;
    *((volatile unsigned int *)(DDRPHY_BASE + 0x0088)) = emi_setting->DDR3_MODE_REG1;

    *((volatile unsigned int *)(DRAMC0_BASE + 0x01e4)) = 0x00000001;
    *((volatile unsigned int *)(DDRPHY_BASE + 0x01e4)) = 0x00000001;

    delay_a_while(1000);

    *((volatile unsigned int *)(DRAMC0_BASE + 0x01e4)) = 0x00000000;
    *((volatile unsigned int *)(DDRPHY_BASE + 0x01e4)) = 0x00000000;

    *((volatile unsigned int *)(DRAMC0_BASE + 0x0088)) = 0x00001941;
    *((volatile unsigned int *)(DDRPHY_BASE + 0x0088)) = 0x00001941;

    *((volatile unsigned int *)(DRAMC0_BASE + 0x01e4)) = 0x00000001;
    *((volatile unsigned int *)(DDRPHY_BASE + 0x01e4)) = 0x00000001;

    delay_a_while(1000);

    *((volatile unsigned int *)(DRAMC0_BASE + 0x01e4)) = 0x00000000;
    *((volatile unsigned int *)(DDRPHY_BASE + 0x01e4)) = 0x00000000;

    *((volatile unsigned int *)(DRAMC0_BASE + 0x0088)) = 0x00000400;
    *((volatile unsigned int *)(DDRPHY_BASE + 0x0088)) = 0x00000400;

    *((volatile unsigned int *)(DRAMC0_BASE + 0x01e4)) = 0x00000010;
    *((volatile unsigned int *)(DDRPHY_BASE + 0x01e4)) = 0x00000010;

    delay_a_while(1000);

    *((volatile unsigned int *)(DRAMC0_BASE + 0x01e4)) = 0x00000000;
    *((volatile unsigned int *)(DDRPHY_BASE + 0x01e4)) = 0x00000000;

    delay_a_while(1000);

    *((volatile unsigned int *)(DRAMC0_BASE + 0x01e4)) = 0x00001100;
    *((volatile unsigned int *)(DDRPHY_BASE + 0x01e4)) = 0x00001100;

    *((volatile unsigned int *)(DRAMC0_BASE + (0x00E4))) = 0x000007a3;  //CKEBYCTL
    *((volatile unsigned int *)(DDRPHY_BASE + (0x00E4))) = 0x000007a3;
    #if 0//test command
    *((volatile unsigned int *)(DRAMC0_BASE + (0x0088))) = 0x0000ffff;
    *((volatile unsigned int *)(DDRPHY_BASE + (0x0088))) = 0x0000ffff;

    *((volatile unsigned int *)(DRAMC0_BASE + (0x01E4))) = 0x00000020;
    *((volatile unsigned int *)(DDRPHY_BASE + (0x01E4))) = 0x00000020;

    *((volatile unsigned int *)(DRAMC0_BASE + (0x01E4))) = 0x00000000;
    *((volatile unsigned int *)(DDRPHY_BASE + (0x01E4))) = 0x00000000;
    #endif
	#ifdef A15_ENABLE
    *((volatile unsigned int *)(DRAMC0_BASE + (0x01DC))) = 0x146728C2;//bit7 A15 output from cs1
    *((volatile unsigned int *)(DDRPHY_BASE + (0x01DC))) = 0x146728C2;//bit7 A15 output from cs1
	#else
    *((volatile unsigned int *)(DRAMC0_BASE + (0x01DC))) = emi_setting->DRAMC_PD_CTRL_VAL;//CLK always on for boot up
    *((volatile unsigned int *)(DDRPHY_BASE + (0x01DC))) = emi_setting->DRAMC_PD_CTRL_VAL;//CLK always on for boot up
    #endif
   *((volatile unsigned int *)(DRAMC0_BASE + (0x000C))) = 0x00000000;
   *((volatile unsigned int *)(DDRPHY_BASE + (0x000C))) = 0x00000000;

    *((volatile unsigned int *)(DRAMC0_BASE + (0x0000))) = emi_setting->DRAMC_ACTIM_VAL;
    *((volatile unsigned int *)(DDRPHY_BASE + (0x0000))) = emi_setting->DRAMC_ACTIM_VAL;

    *((volatile unsigned int *)(DRAMC0_BASE + (0x0044))) = emi_setting->DRAMC_TEST2_3_VAL;
    *((volatile unsigned int *)(DDRPHY_BASE + (0x0044))) = emi_setting->DRAMC_TEST2_3_VAL;

    *((volatile unsigned int *)(DRAMC0_BASE + (0x01E8))) = emi_setting->DRAMC_ACTIM1_VAL;
    *((volatile unsigned int *)(DDRPHY_BASE + (0x01E8))) = emi_setting->DRAMC_ACTIM1_VAL;

    *((volatile unsigned int *)(DRAMC0_BASE + (0x0010))) = 0x00000000;
    *((volatile unsigned int *)(DDRPHY_BASE + (0x0010))) = 0x00000000;

    *((volatile unsigned int *)(DRAMC0_BASE + (0x00F8))) = 0xedcb000f;
    *((volatile unsigned int *)(DDRPHY_BASE + (0x00F8))) = 0xedcb000f;

    *((volatile unsigned int *)(DRAMC0_BASE + (0x01D8))) = 0x00c80008;
    *((volatile unsigned int *)(DDRPHY_BASE + (0x01D8))) = 0x00c80008;

	*((volatile unsigned int *)(DRAMC0_BASE + (0x0008))) = emi_setting->DRAMC_CONF2_VAL;//refresh period[7:0]
	*((volatile unsigned int *)(DDRPHY_BASE + (0x0008))) = emi_setting->DRAMC_CONF2_VAL;//refresh period[7:0]
#ifdef DDR3_AUTOREF_SUPPORT
	*((volatile unsigned int *)(DRAMC0_BASE + 0x01e4)) = 0x00000008;//auto refresh enable
	*((volatile unsigned int *)(DDRPHY_BASE + 0x01e4)) = 0x00000008;//auto refresh enable
	delay_a_while(1000);
	*((volatile unsigned int *)(DRAMC0_BASE + 0x01e4)) = 0x00000000;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x01e4)) = 0x00000000;
	delay_a_while(1000);
#endif

#ifdef DDR3_ZQCS_SUPPORT
	*((volatile unsigned int *)(DRAMC0_BASE + 0x01e4)) = 0x000A0010;//ZQ calibration enable
	*((volatile unsigned int *)(DDRPHY_BASE + 0x01e4)) = 0x000A0010;//ZQ calibration enable
	delay_a_while(1000);
	*((volatile unsigned int *)(DRAMC0_BASE + 0x01e4)) = 0x000A0000;
	*((volatile unsigned int *)(DDRPHY_BASE + 0x01e4)) = 0x000A0000;
	delay_a_while(1000);
#endif
    //Gating DQS coarse tune and fine tune
	*(volatile unsigned  int*)(DRAMC0_BASE+0x0E0)=0x13000000; //fill best coarse
	*(volatile unsigned int *)(DRAMC0_BASE+0x124)=0x00000000; //fill best coarse
	*(volatile unsigned int *)(DRAMC0_BASE+0x94)=0x40404040;
	*(volatile unsigned int *)(DRAMC0_BASE+0x1E4) |= (1 << 8); //enable counter

	//RX DQS, DQ, DM delay setting
    *((volatile unsigned int *)(DRAMC0_BASE + 0x0210)) = 0x0F0E0B0D;
    *((volatile unsigned int *)(DRAMC0_BASE + 0x0214)) = 0x0C0D0D0E;
    *((volatile unsigned int *)(DRAMC0_BASE + 0x0218)) = 0x0F0F0E0F;
    *((volatile unsigned int *)(DRAMC0_BASE + 0x021c)) = 0x0C0E0A0B;
    *((volatile unsigned int *)(DRAMC0_BASE + 0x0220)) = 0x0B0F0A0B;
    *((volatile unsigned int *)(DRAMC0_BASE + 0x0224)) = 0x0A0C0B0C;
    *((volatile unsigned int *)(DRAMC0_BASE + 0x0228)) = 0x0C0A090F;
    *((volatile unsigned int *)(DRAMC0_BASE + 0x022c)) = 0x0A0A090C;
	*((volatile unsigned int *)(DRAMC0_BASE + 0x0018)) = 0x24212621;    //postsim
	*((volatile unsigned int *)(DDRPHY_BASE + 0x0018)) = 0x24212621;    //postsim

	//TX DQS, DQ, DM delay setting
    *((volatile unsigned int *)(DDRPHY_BASE + 0x0010)) = 0x00008898;
    *((volatile unsigned int *)(DDRPHY_BASE + 0x0014)) = emi_setting->DRAMC_PADCTL3_VAL;
    *((volatile unsigned int *)(DDRPHY_BASE + 0x0200)) = emi_setting->DRAMC_DQODLY_VAL;
    *((volatile unsigned int *)(DDRPHY_BASE + 0x0204)) = emi_setting->DRAMC_DQODLY_VAL;
    *((volatile unsigned int *)(DDRPHY_BASE + 0x0208)) = emi_setting->DRAMC_DQODLY_VAL;
    *((volatile unsigned int *)(DDRPHY_BASE + 0x020c)) = emi_setting->DRAMC_DQODLY_VAL;	

	#ifndef SINGLE_RANK_MODE
	*(volatile unsigned int *)(DRAMC0_BASE+0x124)=0x00000000; //fill best coarse
	*(volatile unsigned int *)(DRAMC0_BASE+0x98)=0x40404040;

	*((volatile unsigned int *)(DRAMC0_BASE + 0x001C)) = 0x24212621;    //postsim
	*((volatile unsigned int *)(DDRPHY_BASE + 0x001C)) = 0x24212621;    //postsim
	#endif
//#endif
}

static char id[22];
static int emmc_nand_id_len=16;
static int fw_id_len;
static int mt_get_mdl_number (void)
{
    static int found = 0;
    static int mdl_number = -1;
    int i;
    int j;


    if (!found)
    {
        int result=0;
        result = platform_get_mcp_id (id, emmc_nand_id_len,&fw_id_len);

        for (i = 0; i < num_of_emi_records; i++)
        {
            if (emi_settings[i].type != 0)
            {
                if ((emi_settings[i].type & 0xF00) != 0x000)
                {
                    if (result == 0)
                    {   /* valid ID */

                        if ((emi_settings[i].type & 0xF00) == 0x100)
                        {
                            /* NAND */
                            if (memcmp(id, emi_settings[i].ID, emi_settings[i].id_length) == 0){
                                memset(id + emi_settings[i].id_length, 0, sizeof(id) - emi_settings[i].id_length);                                
                                break; /* found */
                            }
                        }
                        else
                        {
                            
                            /* eMMC */
                            if (memcmp(id, emi_settings[i].ID, emi_settings[i].id_length) == 0)
                            {
                                printf("fw id len:%d\n",emi_settings[i].fw_id_length);
#if 0
                                if (emi_settings[i].fw_id_length > 0)
                                {
                                    char fw_id[6];
                                    memset(fw_id, 0, sizeof(fw_id));
                                    memcpy(fw_id,id+emmc_nand_id_len,fw_id_len);
                                    for (j = 0; j < sizeof(id);j ++){
                                        printf("0x%x ",fw_id[j]); 
                                    }
                                    if(memcmp(fw_id,emi_settings[i].fw_id,emi_settings[i].fw_id_length) == 0)
                                        break; /* found */
                                    else
                                        printf("fw id match failed\n");
                                }
                                else
                                {
                                    break; /* found */
                                }
#else
                                    break; /* found */
#endif
                            }
                            else{
                            
                                        //printf("emmc id match failed\n");
                            }
                            
                        }
                    }
                }
                else
                {
                    /* Discrete DDR */
                    break;
                }
            }
        }
        mdl_number = i;
        found = 1;
        printf("found:%d,i:%d\n",found,i);    
    }
    

    return mdl_number;

}

int mt_get_dram_type (void)
{

    int n;

    n = mt_get_mdl_number ();

    if (n < 0  || n >= num_of_emi_records)
    {
        return 0; /* invalid */
    }

    return (emi_settings[n].type & 0xF);

}

int get_dram_rank_nr (void)
{
    int index;
    int emi_cona;
    index = mt_get_mdl_number ();
    if (index < 0 || index >=  num_of_emi_records)
    {
        return -1;
    }

    emi_cona = emi_settings[index].EMI_CONA_VAL;
    return (emi_cona & 0x20000) ? 2 : 1;

}

int get_dram_rank_size (int dram_rank_size[])
{
 int index,/* bits,*/ rank_nr, i;
    //int emi_cona;

    index = mt_get_mdl_number ();

    if (index < 0 || index >=  num_of_emi_records)
    {
        return;
    }

    rank_nr = get_dram_rank_nr();

    for(i = 0; i < rank_nr; i++){
        dram_rank_size[i] = emi_settings[index].DRAM_RANK_SIZE[i];

        printf("%d:dram_rank_size:%x\n",i,dram_rank_size[i]);
    }

    return;
}
void print_DBG_info(){
    unsigned int addr = 0x0;
        printf("=====================DBG=====================\n");
    for(addr = 0x0; addr < 0x600; addr +=4){
    printf("addr:%x, value:%x\n",addr, DRAMC_READ_REG(addr));
}
    printf("=============================================\n");
}

/*
* mt_set_emi: Set up EMI/DRAMC.
*/
void mt_set_emi (void)
{
int index = 0;
unsigned int val1,val2;
EMI_SETTINGS *emi_set;

print("[EMI] DDR%d\r\n", mt_get_dram_type ());

index = mt_get_mdl_number ();
print("[EMI] eMMC/NAND ID = %x,%x,%x,%x,%x,%x,%x,%x,%x\r\n", id[0], id[1], id[2], id[3], id[4], id[5], id[6], id[7], id[8]);
if (index < 0 || index >=  num_of_emi_records)
{
    print("[EMI] setting failed 0x%x\r\n", index);
    return;
}

print("[EMI] MDL number = %d\r\n", index);
emi_set = &emi_settings[index];
if ((emi_set->type & 0xF) == 1)
{
    init_dram1(emi_set);
}
else if ((emi_set->type & 0xF) == 2)
{
    init_dram2(emi_set);
}
else
{
     init_dram3(emi_set);
}
//print_DBG_info();
#if 1
    if (dramc_calib() < 0) {
        print("[EMI] DRAMC calibration failed\n\r");
    } else {
        print("[EMI] DRAMC calibration passed\n\r");
    }
    if ( *(volatile unsigned *)(EMI_CONA)& 0x20000) /* check dual rank support */
    {
    /*For 2RANK: Set RANKINCTL (REG.1C4[19:16]) = DQSINCTL + 2*/
        val1 = DRAMC_READ_REG(0xe0) & 0x07000000;
        val1 = ((val1 >> 24)+2) <<16; 
        print("[EMI] DQSINCTL:%x\n\r",val1);
        val2 = DRAMC_READ_REG(0x1c4) & 0xFFF0FFFF | val1;
        DRAMC_WRITE_REG(val2,0x1c4);
    }
#endif

#if 1
    /* Enable Gating windown HW calibration function */
    val1 = DRAMC_READ_REG(0x1c0) & 0x7FFFFFFF | (0x1 << 31);
    DRAMC_WRITE_REG(val1,0x1c0);
#endif
#if 1
    /* Enable out of order function */
    val1 = (DRAMC_READ_REG(0x1eC) & 0xFFFFF8EF) | (0x1 << 4) | (0x1 << 10) | (0x1 << 8) | (0x1 << 9);
    DRAMC_WRITE_REG(val1,0x1ec);
#endif
#if 1
    /* Enable EROT */
    val1 = (DRAMC_READ_REG(0x7c) | (0x1 << 7));
    DRAMC_WRITE_REG(val1,0x7c);
#endif
#if 1
    /* disable CMPPD  */
    val1 = (DRAMC_READ_REG(0x1E4) | (0x1 << 13));
    DRAMC_WRITE_REG(val1,0x1E4);
#endif

    //print_DBG_info();
    //pmic_voltage_read(1);//check the pmic setting
    //print("[MEM]mt_get_dram_type:%d\n",mt_get_dram_type());
    /* FIXME: modem exception occurs if set the max pending count */
#if 0
#endif
}

