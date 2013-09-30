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

#ifndef __EMI_HW_H__
#define __EMI_HW_H__

#define EMI_base	EMI_BASE

//typedef volatile unsigned int *      P_U32;

#define EMI_CONA                	(( P_U32)(EMI_base+0x0000))  /* Address Mapping */
#define EMI_CONB                	(( P_U32)(EMI_base+0x0008))  /* Data transfer Overhead for specific read data size */
#define EMI_CONC                	(( P_U32)(EMI_base+0x0010))  /* Data transfer Overhead for specific read data size */
#define EMI_COND                	(( P_U32)(EMI_base+0x0018))  /* Data transfer Overhead for specific write data size */
#define EMI_CONE                	(( P_U32)(EMI_base+0x0020))  /* Data transfer Overhead for specific write data size */
#define EMI_TESTC                	(( P_U32)(EMI_base+0x00F0))  /* Test mode C*/
#define EMI_TESTD                	(( P_U32)(EMI_base+0x00F8))  /* Test mode D*/
#define EMI_ARBA                	(( P_U32)(EMI_base+0x0100))  /* EMI Bandwidth Filter Control ARM */
#define EMI_ARBB                	(( P_U32)(EMI_base+0x0108))  /* EMI Bandwidth Filter Control ARM 1 */
#define EMI_ARBC                	(( P_U32)(EMI_base+0x0110))  /* EMI Bandwidth Filter Control Peripheral */
#define EMI_ARBD                	(( P_U32)(EMI_base+0x0118))  /* EMI Bandwidth Filter Control Modem */
#define EMI_ARBE                	(( P_U32)(EMI_base+0x0120))  /* EMI Bandwidth Filter Control Multimedia */
#define EMI_ARBF                	(( P_U32)(EMI_base+0x0128))  /* EMI Bandwidth Filter Control Multimedia 1*/
#define EMI_ARBG                	(( P_U32)(EMI_base+0x0130))  /* EMI Bandwidth Filter Control Multimedia 2*/
#define EMI_ARBI                	(( P_U32)(EMI_base+0x0140))  /* Filter Priority Encode */
#define EMI_ARBJ                	(( P_U32)(EMI_base+0x0148))  /* Turn around command number */
#define EMI_ARBK                	(( P_U32)(EMI_base+0x0150))  /* Page miss control */
#define EMI_SLCT                	(( P_U32)(EMI_base+0x0158))  /* EMI slave control registers */

#define EMI_MPUA                	(( P_U32)(EMI_base+0x0160))  /* Memory protect unit control registers A */
#define EMI_MPUB                	(( P_U32)(EMI_base+0x0168))  /* Memory protect unit control registers B */
#define EMI_MPUC                	(( P_U32)(EMI_base+0x0170))  /* Memory protect unit control registers C */
#define EMI_MPUD                	(( P_U32)(EMI_base+0x0178))  /* Memory protect unit control registers D */
#define EMI_MPUE                	(( P_U32)(EMI_base+0x0180))  /* Memory protect unit control registers E */
#define EMI_MPUF                	(( P_U32)(EMI_base+0x0188))  /* Memory protect unit control registers F */
#define EMI_MPUG                	(( P_U32)(EMI_base+0x0190))  /* Memory protect unit control registers G */
#define EMI_MPUH                	(( P_U32)(EMI_base+0x0198))  /* Memory protect unit control registers H */
#define EMI_MPUI                	(( P_U32)(EMI_base+0x01A0))  /* Memory protect unit control registers I */
#define EMI_MPUJ                	(( P_U32)(EMI_base+0x01A8))  /* Memory protect unit control registers J */
#define EMI_MPUK                	(( P_U32)(EMI_base+0x01B0))  /* Memory protect unit control registers K */
#define EMI_MPUL                	(( P_U32)(EMI_base+0x01B8))  /* Memory protect unit control registers L */
#define EMI_MPUM                	(( P_U32)(EMI_base+0x01C0))  /* Memory protect unit control registers M */
#define EMI_MPUN                	(( P_U32)(EMI_base+0x01C8))  /* Memory protect unit control registers N */
#define EMI_MPUO                	(( P_U32)(EMI_base+0x01D0))  /* Memory protect unit control registers O */
#define EMI_MPUP                	(( P_U32)(EMI_base+0x01D8))  /* Memory protect unit control registers P */
#define EMI_MPUQ                	(( P_U32)(EMI_base+0x01E0))  /* Memory protect unit control registers Q */
#define EMI_MPUR                	(( P_U32)(EMI_base+0x01E8))  /* Memory protect unit control registers R */
#define EMI_MPUS                	(( P_U32)(EMI_base+0x01F0))  /* Memory protect unit control registers S */
#define EMI_MPUT                	(( P_U32)(EMI_base+0x01F8))  /* Memory protect unit control registers T */

#define EMI_BMEN                	(( P_U32)(EMI_base+0x0400))  /* EMI bus monitor enable */
#define EMI_BCNT                	(( P_U32)(EMI_base+0x0408))  /* EMI bus cycle counter */
#define EMI_TACT                	(( P_U32)(EMI_base+0x0410))  /* EMI transaction counter for all masters */
#define EMI_TSCT                	(( P_U32)(EMI_base+0x0418))  /* EMI first transaction counter for selected masters */
                                                        
#define EMI_WACT                	(( P_U32)(EMI_base+0x0420))  /* EMI double word counter for all masters */
#define EMI_WSCT                	(( P_U32)(EMI_base+0x0428))  /* EMI first double word counter for selected masters */
#define EMI_BACT                	(( P_U32)(EMI_base+0x0430))  /* EMI bus-busy counter for all masters */
#define EMI_BSCT                	(( P_U32)(EMI_base+0x0438))  /* EMI bus-busy counter for selected masters */
                                                        
#define EMI_MSEL                 	(( P_U32)(EMI_base+0x0440))  /* EMI master selection for the second and third counters */
#define EMI_TSCT2                   (( P_U32)(EMI_base+0x0448))  /* EMI second transaction counter for selected masters */
#define EMI_TSCT3                	(( P_U32)(EMI_base+0x0450))  /* EMI third transaction counter for selected masters */
#define EMI_WSCT2                   (( P_U32)(EMI_base+0x0458))  /* EMI second double word counter for selected masters */
#define EMI_WSCT3                   (( P_U32)(EMI_base+0x0460))  /* EMI third double word counter for selected masters */
#define EMI_WSCT4                   (( P_U32)(EMI_base+0x0464))  /* EMI fourth double word counter for selected masters */

#define EMI_MSEL2                	(( P_U32)(EMI_base+0x0468))  /* EMI master selection for the fourth and fifth counters */
#define EMI_MSEL3                	(( P_U32)(EMI_base+0x0470))  /* EMI master selection for the sixth and seventh counter */
#define EMI_MSEL4                   (( P_U32)(EMI_base+0x0478))  /* EMI master selection for the eighteen and ninth counter */
#define EMI_MSEL5                   (( P_U32)(EMI_base+0x0480))  /* EMI master selection for the 10th and 11th counter */
#define EMI_MSEL6                   (( P_U32)(EMI_base+0x0488))  /* EMI master selection for the 12th and 13th counter */
#define EMI_MSEL7                   (( P_U32)(EMI_base+0x0490))  /* EMI master selection for the 14th and 15th counter */
#define EMI_MSEL8                	(( P_U32)(EMI_base+0x0498))  /* EMI master selection for the 16th and 17th counter */
#define EMI_MSEL9                   (( P_U32)(EMI_base+0x04A0))  /* EMI master selection for the 18th and 19th counter */
#define EMI_MSEL10                  (( P_U32)(EMI_base+0x04A8))  /* EMI master selection for the 20th and 21st counter */
                                    
#define EMI_BMID0                   (( P_U32)(EMI_base+0x04B0))  /* EMI ID selection for 1 ~ 4 transaction type counters */
#define EMI_BMID1                   (( P_U32)(EMI_base+0x04B8))  /* EMI ID selection for 5 ~ 8 transaction type counters */
#define EMI_BMID2                   (( P_U32)(EMI_base+0x04C0))  /* EMI ID selection for 9 ~ 12 transaction type counters */
#define EMI_BMID3                   (( P_U32)(EMI_base+0x04C8))  /* EMI ID selection for 13 ~ 16 transaction type counters */
#define EMI_BMID4                   (( P_U32)(EMI_base+0x04D0))  /* EMI ID selection for 17 ~ 20 transaction type counters */
#define EMI_BMID5                   (( P_U32)(EMI_base+0x04D8))  /* EMI ID selection for 21 transaction type counters */

#define EMI_BMEN1                	(( P_U32)(EMI_base+0x04E0))  
#define EMI_BMEN2                	(( P_U32)(EMI_base+0x04E8)) 
                                    
#define EMI_TTYPE1                  (( P_U32)(EMI_base+0x0500))  /* EMI 1st transaction type counter for selected masters */
#define EMI_TTYPE2                  (( P_U32)(EMI_base+0x0508))  /* EMI 2nd transaction type counter for selected masters */                                                       
#define EMI_TTYPE3                  (( P_U32)(EMI_base+0x0510))  /* EMI 3th transaction type counter for selected masters */
#define EMI_TTYPE4                  (( P_U32)(EMI_base+0x0518))  /* EMI 4th transaction type counter for selected masters */
#define EMI_TTYPE5                  (( P_U32)(EMI_base+0x0520))  /* EMI 5th transaction type counter for selected masters */
#define EMI_TTYPE6                  (( P_U32)(EMI_base+0x0528))  /* EMI 6th transaction type counter for selected masters */
#define EMI_TTYPE7                  (( P_U32)(EMI_base+0x0530))  /* EMI 7th transaction type counter for selected masters */
#define EMI_TTYPE8                  (( P_U32)(EMI_base+0x0538))  /* EMI 8th transaction type counter for selected masters */
#define EMI_TTYPE9                  (( P_U32)(EMI_base+0x0540))  /* EMI 9th transaction type counter for selected masters */
#define EMI_TTYPE10                 (( P_U32)(EMI_base+0x0548))  /* EMI 10th transaction type counter for selected masters */
#define EMI_TTYPE11                 (( P_U32)(EMI_base+0x0550))  /* EMI 11th transaction type counter for selected masters */
#define EMI_TTYPE12                 (( P_U32)(EMI_base+0x0558))  /* EMI 12th transaction type counter for selected masters */
#define EMI_TTYPE13                 (( P_U32)(EMI_base+0x0560))  /* EMI 13th transaction type counter for selected masters */
#define EMI_TTYPE14                 (( P_U32)(EMI_base+0x0568))  /* EMI 14th transaction type counter for selected masters */
#define EMI_TTYPE15                 (( P_U32)(EMI_base+0x0570))  /* EMI 15th transaction type counter for selected masters */
#define EMI_TTYPE16                 (( P_U32)(EMI_base+0x0578))  /* EMI 16th transaction type counter for selected masters */
#define EMI_TTYPE17                 (( P_U32)(EMI_base+0x0580))  /* EMI 17th transaction type counter for selected masters */
#define EMI_TTYPE18                 (( P_U32)(EMI_base+0x0588))  /* EMI 18th transaction type counter for selected masters */
#define EMI_TTYPE19                 (( P_U32)(EMI_base+0x0590))  /* EMI 19th transaction type counter for selected masters */
#define EMI_TTYPE20                 (( P_U32)(EMI_base+0x0598))  /* EMI 20th transaction type counter for selected masters */
#define EMI_TTYPE21                 (( P_U32)(EMI_base+0x05A0))  /* EMI 21st transaction type counter for selected masters */

//MT6577 EMI default values
#define EMI_CONB_DEFAULT	0x00000000
#define EMI_CONC_DEFAULT	0x00000000
#define EMI_COND_DEFAULT	0x00000000
#define EMI_CONE_DEFAULT	0x00000000
#define EMI_TESTC_DEFAULT	0x00000000
#define EMI_TESTD_DEFAULT	0x00000000
#define EMI_ARBA_DEFAULT	0x00004000
#define EMI_ARBC_DEFAULT	0x00004800
#define EMI_ARBD_DEFAULT	0x00004000
#define EMI_ARBE_DEFAULT	0x00004000
#define EMI_ARBF_DEFAULT	0x00004000
#define EMI_ARBG_DEFAULT	0x00004000
#define EMI_ARBI_DEFAULT	0x00000000
#define EMI_ARBJ_DEFAULT	0x00684848
#define EMI_ARBK_DEFAULT	0x00000C7C
#define EMI_SLCT_DEFAULT	0x00000000
#define EMI_MPUA_DEFAULT	0x00000000
#define EMI_MPUB_DEFAULT	0x00000000
#define EMI_MPUC_DEFAULT	0x00000000
#define EMI_MPUD_DEFAULT	0x00000000
#define EMI_MPUE_DEFAULT	0x00000000
#define EMI_MPUF_DEFAULT	0x00000000
#define EMI_MPUG_DEFAULT	0x00000000
#define EMI_MPUH_DEFAULT	0x00000000
#define EMI_MPUI_DEFAULT	0x00000000
#define EMI_MPUJ_DEFAULT	0x00000000
#define EMI_MPUK_DEFAULT	0x00000000
#define EMI_MPUL_DEFAULT	0x00000000
#define EMI_MPUM_DEFAULT	0x00000000
#define EMI_MPUN_DEFAULT	0x00000000
#define EMI_MPUO_DEFAULT	0x00000000
#define EMI_MPUP_DEFAULT	0x00000000
#define EMI_MPUQ_DEFAULT	0x00000000
#define EMI_MPUR_DEFAULT	0x00000000
#define EMI_MPUS_DEFAULT	0x00000000
#define EMI_MPUT_DEFAULT	0x00000000
#define EMI_BMEN_DEFAULT	0x00000000
#define EMI_BCNT_DEFAULT	0x00000000
#define EMI_TACT_DEFAULT	0x00000000
#define EMI_TSCT_DEFAULT	0x00000000
#define EMI_WACT_DEFAULT	0x00000000
#define EMI_WSCT_DEFAULT	0x00000000
#define EMI_BACT_DEFAULT	0x00000000
#define EMI_BSCT_DEFAULT	0x00000000
#define EMI_MSEL_DEFAULT	0x00000000
#define EMI_TSCT2_DEFAULT	0x00000000
#define EMI_TSCT3_DEFAULT	0x00000000
#define EMI_WSCT2_DEFAULT	0x00000000
#define EMI_WSCT3_DEFAULT	0x00000000
#define EMI_WSCT4_DEFAULT	0x00000000
#define EMI_MSEL2_DEFAULT	0x00000000
#define EMI_MSEL3_DEFAULT	0x00000000
#define EMI_MSEL4_DEFAULT	0x00000000
#define EMI_MSEL5_DEFAULT	0x00000000
#define EMI_MSEL6_DEFAULT	0x00000000
#define EMI_MSEL7_DEFAULT	0x00000000
#define EMI_MSEL8_DEFAULT	0x00000000
#define EMI_MSEL9_DEFAULT	0x00000000
#define EMI_MSEL10_DEFAULT	0x00000000
#define EMI_BMID0_DEFAULT	0x00000000
#define EMI_BMID1_DEFAULT	0x00000000
#define EMI_BMID2_DEFAULT	0x00000000
#define EMI_BMID3_DEFAULT	0x00000000
#define EMI_BMID4_DEFAULT	0x00000000
#define EMI_BMID5_DEFAULT	0x00000000
#define EMI_BMEN1_DEFAULT	0x00000000
#define EMI_BMEN2_DEFAULT	0x00000000
#define EMI_TTYPE1_DEFAULT	0x00000000
#define EMI_TTYPE2_DEFAULT	0x00000000
#define EMI_TTYPE3_DEFAULT	0x00000000
#define EMI_TTYPE4_DEFAULT	0x00000000
#define EMI_TTYPE5_DEFAULT	0x00000000
#define EMI_TTYPE6_DEFAULT	0x00000000
#define EMI_TTYPE7_DEFAULT	0x00000000
#define EMI_TTYPE8_DEFAULT	0x00000000
#define EMI_TTYPE9_DEFAULT	0x00000000
#define EMI_TTYPE10_DEFAULT	0x00000000
#define EMI_TTYPE11_DEFAULT	0x00000000
#define EMI_TTYPE12_DEFAULT	0x00000000
#define EMI_TTYPE13_DEFAULT	0x00000000
#define EMI_TTYPE14_DEFAULT	0x00000000
#define EMI_TTYPE15_DEFAULT	0x00000000
#define EMI_TTYPE16_DEFAULT	0x00000000
#define EMI_TTYPE17_DEFAULT	0x00000000
#define EMI_TTYPE18_DEFAULT	0x00000000
#define EMI_TTYPE19_DEFAULT	0x00000000
#define EMI_TTYPE20_DEFAULT	0x00000000
#define EMI_TTYPE21_DEFAULT	0x00000000


#define DRAMC_ACTIM			(( P_U32)(dramc0_BASE+0x0))	/* DRAM AC TIMING SETTING */
#define DRAMC_CONF1			(( P_U32)(dramc0_BASE+0x4))	/* DRAM CONFIGURATION 1 */
#define DRAMC_CONF2			(( P_U32)(dramc0_BASE+0x8))	/* DRAM CONFIGURATION 2 */
#define __DRAMC_PADCTL1			(( P_U32)(dramc0_BASE+0xC))	/* DRAM PAD CONTROL 1 */
#define __DRAMC_PADCTL2			(( P_U32)(dramc0_BASE+0x10))	/* DRAM PAD CONTROL 2 */
#define __DRAMC_PADCTL3			(( P_U32)(dramc0_BASE+0x14))	/* DRAM PAD CONTROL 3 */
#define DRAMC_DELDLY1			(( P_U32)(dramc0_BASE+0x18))	/* DQS INPUT DELAY CHAIN SETTING 1 */
#define DRAMC_DELDLY2			(( P_U32)(dramc0_BASE+0x1C))	/* DQS INPUT DELAY CHAIN SETTING 2 */
#define DRAMC_DIFDLY1			(( P_U32)(dramc0_BASE+0x20))	/* DQS INPUT DELAY CHAIN OFFSET SETTING 1 */
#define DRAMC_DIFDLY2			(( P_U32)(dramc0_BASE+0x24))	/* DQS INPUT DELAY CHAIN OFFSET SETTING 2 */
#define DRAMC_DLLCONF			(( P_U32)(dramc0_BASE+0x28))	/* DLL CONFIGURATION */
#define DRAMC_TEST0_1			(( P_U32)(dramc0_BASE+0x2C))	/* TEST AGENT 0 CONFIGURATION 1 */
#define DRAMC_TEST0_2			(( P_U32)(dramc0_BASE+0x30))	/* TEST AGENT 0 CONFIGURATION 2 */
#define DRAMC_TEST1_1			(( P_U32)(dramc0_BASE+0x34))	/* TEST AGENT 1 CONFIGURATION 1 */
#define DRAMC_TEST1_2			(( P_U32)(dramc0_BASE+0x38))	/* TEST AGENT 1 CONFIGURATION 2 */
#define DRAMC_TEST2_1			(( P_U32)(dramc0_BASE+0x3C))	/* TEST AGENT 2 CONFIGURATION 1 */
#define DRAMC_TEST2_2			(( P_U32)(dramc0_BASE+0x40))	/* TEST AGENT 2 CONFIGURATION 2 */
#define DRAMC_TEST2_3			(( P_U32)(dramc0_BASE+0x44))	/* TEST AGENT 2 CONFIGURATION 3 */
#define DRAMC_TEST2_4			(( P_U32)(dramc0_BASE+0x48))	/* TEST AGENT 2 CONFIGURATION 4 */
#define DRAMC_DDR2CTL			(( P_U32)(dramc0_BASE+0x7C))	/* DDR2 CONTROL REGISTER */
#define DRAMC_MRSBK			(( P_U32)(dramc0_BASE+0x80))	/* MRS BANK ADDRESS SETTING */
#define DRAMC_MRS			(( P_U32)(dramc0_BASE+0x88))	/* MRS VALUE SETTING */
#define DRAMC_PHYCTL2			(( P_U32)(dramc0_BASE+0x8C))	/* SET FOR IO CLOCK DELAY AND PHY FIFO DEPTH */
#define DRAMC_DQSIEN			(( P_U32)(dramc0_BASE+0x94))	/* DQS INPUT RANGE FINE TUNER */
#define DRAMC_WCKOFF0			(( P_U32)(dramc0_BASE+0x98))	/* OUTPUT CLOCK DELAY CHAIN SETTING 0 */
#define DRAMC_WCKOFF1			(( P_U32)(dramc0_BASE+0x9C))	/* OUTPUT CLOCK DELAY CHAIN SETTING 1 */
#define DRAMC_WCKOFF2			(( P_U32)(dramc0_BASE+0xA0))	/* OUTPUT CLOCK DELAY CHAIN SETTING 2 */
#define DRAMC_WCKOFF3			(( P_U32)(dramc0_BASE+0xA4))	/* OUTPUT CLOCK DELAY CHAIN SETTING 3 */
#define DRAMC_DRVCTL0			(( P_U32)(dramc0_BASE+0xB8))	/* PAD DRIVING CONTROL SETTING 0 */
#define DRAMC_DRVCTL1			(( P_U32)(dramc0_BASE+0xBC))	/* PAD DRIVING CONTROL SETTING 1 */
#define DRAMC_DLLSEL			(( P_U32)(dramc0_BASE+0xC0))	/* DLL SELECTION SETTING */
#define DRAMC_DELAYLINE			(( P_U32)(dramc0_BASE+0xD4))	/* DELAY LINE REGISTER */
#define DRAMC_MCKDLY			(( P_U32)(dramc0_BASE+0xD8))	/* MEMORY CLOCK DELAY CHAIN SETTING */
#define DRAMC_DQSCTL0			(( P_U32)(dramc0_BASE+0xDC))	/* DQS INPUT RANGE CONTROL 0 */
#define DRAMC_DQSCTL1			(( P_U32)(dramc0_BASE+0xE0))	/* DQS INPUT RANGE CONTROL 1 */
#define DRAMC_PADCTL1			(( P_U32)(dramc0_BASE+0xE4))	/* DRAM PAD CONTROL 1 */
#define DRAMC_PADCTL2			(( P_U32)(dramc0_BASE+0xE8))	/* DRAM PAD CONTROL 2 */
#define DRAMC_PADCTL3			(( P_U32)(dramc0_BASE+0xEC))	/* DRAM PAD CONTROL 3 */
#define DRAMC_PHYCTL1			(( P_U32)(dramc0_BASE+0xF0))	/* DDR PHY CONTROL 1 */
#define DRAMC_GDDR3CTL1			(( P_U32)(dramc0_BASE+0xF4))	/* GDDR3 CONTROL 1 */
#define DRAMC_PADCTL4			(( P_U32)(dramc0_BASE+0xF8))	/* DRAM PAD CONTROL 4 */
#define DRAMC_OCDK			(( P_U32)(dramc0_BASE+0x100))	/* OCD CALIBRATION CONTROL */
#define DRAMC_LBWDAT0			(( P_U32)(dramc0_BASE+0x104))	/* LOOP BACK DATA 0 */
#define DRAMC_LBWDAT1			(( P_U32)(dramc0_BASE+0x108))	/* LOOP BACK DATA 1 */
#define DRAMC_LBWDAT2			(( P_U32)(dramc0_BASE+0x10C))	/* LOOP BACK DATA 1 */
#define DRAMC_RKCFG			(( P_U32)(dramc0_BASE+0x110))	/* RANK CONFIGURATION */
#define DRAMC_CKPH0			(( P_U32)(dramc0_BASE+0x114))	/* CLOCK PHASE DETECTION 0 */
#define DRAMC_DQSISEL			(( P_U32)(dramc0_BASE+0x158))	/* DQS SELECTION */
#define DRAMC_CMDDLY0			(( P_U32)(dramc0_BASE+0x1A8))	/* COMMAND DELAY CONTROL 0 */
#define DRAMC_CMDDLY1			(( P_U32)(dramc0_BASE+0x1AC))	/* COMMAND DELAY CONTROL 0 */
#define DRAMC_CMDDLY2			(( P_U32)(dramc0_BASE+0x1B0))	/* COMMAND DELAY CONTROL 0 */
#define DRAMC_CMDDLY3			(( P_U32)(dramc0_BASE+0x1B4))	/* COMMAND DELAY CONTROL 0 */
#define DRAMC_CMDDLY4			(( P_U32)(dramc0_BASE+0x1B8))	/* COMMAND DELAY CONTROL 0 */
#define DRAMC_CMDDLY5			(( P_U32)(dramc0_BASE+0x1BC))	/* COMMAND DELAY CONTROL 0 */
#define DRAMC_DMMONITOR			(( P_U32)(dramc0_BASE+0x1D8))	/* MONITOR PARAMETER */
#define DRAMC_LPDDR2			(( P_U32)(dramc0_BASE+0x1E0))	/* LPDDR2 SETTING */
#define DRAMC_SPCMD			(( P_U32)(dramc0_BASE+0x1E4))	/* SPECIAL COMMAND MODE */
#define DRAMC_DQODLY1			(( P_U32)(dramc0_BASE+0x200))	/* DQ OUTPUT DELAY1 CHAIN SETTING, UNIT: 30ps */
#define DRAMC_DQODLY2			(( P_U32)(dramc0_BASE+0x204))	/* DQ OUTPUT DELAY2 CHAIN SETTING, UNIT: 30ps */
#define DRAMC_DQODLY3			(( P_U32)(dramc0_BASE+0x208))	/* DQ OUTPUT DELAY3 CHAIN SETTING, UNIT: 30ps */
#define DRAMC_DQODLY4			(( P_U32)(dramc0_BASE+0x20C))	/* DQ OUTPUT DELAY4 CHAIN SETTING, UNIT: 30ps */
#define DRAMC_DQIDLY1			(( P_U32)(dramc0_BASE+0x210))	/* DQ INPUT DELAY1 CHAIN SETTING, UNIT: 30ps */
#define DRAMC_DQIDLY2			(( P_U32)(dramc0_BASE+0x214))	/* DQ INPUT DELAY2 CHAIN SETTING, UNIT: 30ps */
#define DRAMC_DQIDLY3			(( P_U32)(dramc0_BASE+0x218))	/* DQ INPUT DELAY3 CHAIN SETTING, UNIT: 30ps */
#define DRAMC_DQIDLY4			(( P_U32)(dramc0_BASE+0x21C))	/* DQ INPUT DELAY4 CHAIN SETTING, UNIT: 30ps */
#define DRAMC_DQIDLY5			(( P_U32)(dramc0_BASE+0x220))	/* DQ INPUT DELAY5 CHAIN SETTING, UNIT: 30ps */
#define DRAMC_DQIDLY6			(( P_U32)(dramc0_BASE+0x224))	/* DQ INPUT DELAY6 CHAIN SETTING, UNIT: 30ps */
#define DRAMC_DQIDLY7			(( P_U32)(dramc0_BASE+0x228))	/* DQ INPUT DELAY7 CHAIN SETTING, UNIT: 30ps */
#define DRAMC_DQIDLY8			(( P_U32)(dramc0_BASE+0x22C))	/* DQ INPUT DELAY8 CHAIN SETTING, UNIT: 30ps */
#define DRAMC_JMETER_ST			(( P_U32)(dramc0_BASE+0x2BC))	/* JITTER METER STATUS */
#define DRAMC_DQ_CAL_MAX_0		(( P_U32)(dramc0_BASE+0x2C0))	/* DQ INPUT CALIBRATION PER BIT 0-3 */
#define DRAMC_DQ_CAL_MAX_1		(( P_U32)(dramc0_BASE+0x2C4))	/* DQ INPUT CALIBRATION PER BIT 4-7 */
#define DRAMC_DQ_CAL_MAX_2		(( P_U32)(dramc0_BASE+0x2C8))	/* DQ INPUT CALIBRATION PER BIT 8-11 */
#define DRAMC_DQ_CAL_MAX_3		(( P_U32)(dramc0_BASE+0x2CC))	/* DQ INPUT CALIBRATION PER BIT 12-15 */
#define DRAMC_DQ_CAL_MAX_4		(( P_U32)(dramc0_BASE+0x2D0))	/* DQ INPUT CALIBRATION PER BIT 16-19 */
#define DRAMC_DQ_CAL_MAX_5		(( P_U32)(dramc0_BASE+0x2D4))	/* DQ INPUT CALIBRATION PER BIT 20-23 */
#define DRAMC_DQ_CAL_MAX_6		(( P_U32)(dramc0_BASE+0x2D8))	/* DQ INPUT CALIBRATION PER BIT 24-27 */
#define DRAMC_DQ_CAL_MAX_7		(( P_U32)(dramc0_BASE+0x2DC))	/* DQ INPUT CALIBRATION PER BIT 28-31 */
#define DRAMC_DQS_CAL_MIN_0		(( P_U32)(dramc0_BASE+0x2E0))	/* DQS INPUT CALIBRATION PER BIT 0-3 */
#define DRAMC_DQS_CAL_MIN_1		(( P_U32)(dramc0_BASE+0x2E4))	/* DQS INPUT CALIBRATION PER BIT 4-7 */
#define DRAMC_DQS_CAL_MIN_2		(( P_U32)(dramc0_BASE+0x2E8))	/* DQS INPUT CALIBRATION PER BIT 8-11 */
#define DRAMC_DQS_CAL_MIN_3		(( P_U32)(dramc0_BASE+0x2EC))	/* DQS INPUT CALIBRATION PER BIT 12-15 */
#define DRAMC_DQS_CAL_MIN_4		(( P_U32)(dramc0_BASE+0x2F0))	/* DQS INPUT CALIBRATION PER BIT 16-19 */
#define DRAMC_DQS_CAL_MIN_5		(( P_U32)(dramc0_BASE+0x2F4))	/* DQS INPUT CALIBRATION PER BIT 20-23 */
#define DRAMC_DQS_CAL_MIN_6		(( P_U32)(dramc0_BASE+0x2F8))	/* DQS INPUT CALIBRATION PER BIT 24-27 */
#define DRAMC_DQS_CAL_MIN_7		(( P_U32)(dramc0_BASE+0x2FC))	/* DQS INPUT CALIBRATION PER BIT 28-31 */
#define DRAMC_DQS_CAL_MAX_0		(( P_U32)(dramc0_BASE+0x300))	/* DQS INPUT CALIBRATION PER BIT 0-3 */
#define DRAMC_DQS_CAL_MAX_1		(( P_U32)(dramc0_BASE+0x304))	/* DQS INPUT CALIBRATION PER BIT 4-7 */
#define DRAMC_DQS_CAL_MAX_2		(( P_U32)(dramc0_BASE+0x308))	/* DQS INPUT CALIBRATION PER BIT 8-11 */
#define DRAMC_DQS_CAL_MAX_3		(( P_U32)(dramc0_BASE+0x30C))	/* DQS INPUT CALIBRATION PER BIT 12-15 */
#define DRAMC_DQS_CAL_MAX_4		(( P_U32)(dramc0_BASE+0x310))	/* DQS INPUT CALIBRATION PER BIT 16-19 */
#define DRAMC_DQS_CAL_MAX_5		(( P_U32)(dramc0_BASE+0x314))	/* DQS INPUT CALIBRATION PER BIT 20-23 */
#define DRAMC_DQS_CAL_MAX_6		(( P_U32)(dramc0_BASE+0x318))	/* DQS INPUT CALIBRATION PER BIT 24-27 */
#define DRAMC_DQS_CAL_MAX_7		(( P_U32)(dramc0_BASE+0x31C))	/* DQS INPUT CALIBRATION PER BIT 28-31 */
#define DRAMC_DQICAL0			(( P_U32)(dramc0_BASE+0x350))	/* DQS INPUT CALIBRATION 0 */
#define DRAMC_DQICAL1			(( P_U32)(dramc0_BASE+0x354))	/* DQS INPUT CALIBRATION 1 */
#define DRAMC_DQICAL2			(( P_U32)(dramc0_BASE+0x358))	/* DQS INPUT CALIBRATION 2 */
#define DRAMC_DQICAL3			(( P_U32)(dramc0_BASE+0x35C))	/* DQS INPUT CALIBRATION 3 */
#define DRAMC_CMP_ERR			(( P_U32)(dramc0_BASE+0x370))	/* CMP ERROR */
#define DRAMC_BUSMON			(( P_U32)(dramc0_BASE+0x380))	/* BUS MONITOR CYCLE */
#define DRAMC_STBEN0			(( P_U32)(dramc0_BASE+0x38C))	/* DQS RING COUNTER 0 */
#define DRAMC_STBEN1			(( P_U32)(dramc0_BASE+0x390))	/* DQS RING COUNTER 1 */
#define DRAMC_STBEN2			(( P_U32)(dramc0_BASE+0x394))	/* DQS RING COUNTER 2 */
#define DRAMC_STBEN3			(( P_U32)(dramc0_BASE+0x398))	/* DQS RING COUNTER 3 */
#define DRAMC_DQSDLY0			(( P_U32)(dramc0_BASE+0x3A0))	/* DQS INPUT DELAY SETTING 0 */
#define DRAMC_DQSDLY1			(( P_U32)(dramc0_BASE+0x3A4))	/* DQS INPUT DELAY SETTING 1 */
#define DRAMC_OFFDLY0			(( P_U32)(dramc0_BASE+0x3A8))	/* OUTPUT CLOCK DELAY CHAIN SETTING 0 */
#define DRAMC_OFFDLY1			(( P_U32)(dramc0_BASE+0x3AC))	/* OUTPUT CLOCK DELAY CHAIN SETTING 1 */
#define DRAMC_OFFDLY2			(( P_U32)(dramc0_BASE+0x3B0))	/* OUTPUT CLOCK DELAY CHAIN SETTING 2 */
#define DRAMC_OFFDLY3			(( P_U32)(dramc0_BASE+0x3B4))	/* OUTPUT CLOCK DELAY CHAIN SETTING 3 */
#define DRAMC_OFFDLY4			(( P_U32)(dramc0_BASE+0x3B8))	/* OUTPUT CLOCK DELAY CHAIN SETTING 4 */
#define DRAMC_OFFDLY5			(( P_U32)(dramc0_BASE+0x3BC))	/* OUTPUT CLOCK DELAY CHAIN SETTING 5 */
#define DRAMC_OFFDLY6			(( P_U32)(dramc0_BASE+0x3C0))	/* OUTPUT CLOCK DELAY CHAIN SETTING 6 */
#define DRAMC_OFFDLY7			(( P_U32)(dramc0_BASE+0x3C4))	/* OUTPUT CLOCK DELAY CHAIN SETTING 7 */
#define DRAMC_DLLCNT0			(( P_U32)(dramc0_BASE+0x3DC))	/* DLL STATUS 0 */
#define DRAMC_DLLCNT1			(( P_U32)(dramc0_BASE+0x3E0))	/* DLL STATUS 1 */
#define DRAMC_CKPHCNT			(( P_U32)(dramc0_BASE+0x3E8))	/* CLOCK PHASE DETECTION RESULT */
#define DRAMC_TESTRPT			(( P_U32)(dramc0_BASE+0x3FC))	/* TEST AGENT STATUS */


#define DRAMC_ACTIM_DEFAULT	0x2256C154
#define DRAMC_CONF1_DEFAULT	0x00000000
#define DRAMC_CONF2_DEFAULT	0x00000000
#define __DRAMC_PADCTL1_DEFAULT	0x00000000
#define __DRAMC_PADCTL2_DEFAULT	0x00000000
#define __DRAMC_PADCTL3_DEFAULT	0x00000000
#define DRAMC_DELDLY1_DEFAULT	0x00000000
#define DRAMC_DELDLY2_DEFAULT	0x00000000
#define DRAMC_DIFDLY1_DEFAULT	0x00000000
#define DRAMC_DIFDLY2_DEFAULT	0x00000000
#define DRAMC_DLLCONF_DEFAULT	0x0000FF01
#define DRAMC_TEST0_1_DEFAULT	0x55010000
#define DRAMC_TEST0_2_DEFAULT	0x33000FFF
#define DRAMC_TEST1_1_DEFAULT	0x55020000
#define DRAMC_TEST1_2_DEFAULT	0x33000FFF
#define DRAMC_TEST2_1_DEFAULT	0x01200000
#define DRAMC_TEST2_2_DEFAULT	0x00010000
#define DRAMC_TEST2_3_DEFAULT	0xC8050000
#define DRAMC_TEST2_4_DEFAULT	0x0000110D
#define DRAMC_DDR2CTL_DEFAULT	0x00000000
#define DRAMC_MRSBK_DEFAULT	0x00000000
#define DRAMC_MRS_DEFAULT	0x00000000
#define DRAMC_PHYCTL2_DEFAULT	0x00000000
#define DRAMC_DQSIEN_DEFAULT	0x00000000
#define DRAMC_WCKOFF0_DEFAULT	0x00000000
#define DRAMC_WCKOFF1_DEFAULT	0x00000000
#define DRAMC_WCKOFF2_DEFAULT	0x00000000
#define DRAMC_WCKOFF3_DEFAULT	0x00000000
#define DRAMC_DRVCTL0_DEFAULT	0xAA22AA22
#define DRAMC_DRVCTL1_DEFAULT	0xAA22AA22
#define DRAMC_DLLSEL_DEFAULT	0x00000000
#define DRAMC_MCKDLY_DEFAULT	0x00000000
#define DRAMC_DQSCTL0_DEFAULT	0x00000000
#define DRAMC_DQSCTL1_DEFAULT	0x00000000
#define DRAMC_PADCTL1_DEFAULT	0x00000000
#define DRAMC_PADCTL2_DEFAULT	0x00000000
#define DRAMC_PADCTL3_DEFAULT	0x00000000
#define DRAMC_PHYCTL1_DEFAULT	0x00000000
#define DRAMC_GDDR3CTL1_DEFAULT	0x00000000
#define DRAMC_PADCTL4_DEFAULT	0x00000000
#define DRAMC_OCDK_DEFAULT	0x00000000
#define DRAMC_LBWDAT0_DEFAULT	0x00000000
#define DRAMC_LBWDAT1_DEFAULT	0x00000000
#define DRAMC_LBWDAT2_DEFAULT	0x00000000
#define DRAMC_RKCFG_DEFAULT	0x00051100
#define DRAMC_CKPH0_DEFAULT	0x00000000
#define DRAMC_DQSISEL_DEFAULT	0x00000000
#define DRAMC_CMDDLY0_DEFAULT	0x00000000
#define DRAMC_CMDDLY1_DEFAULT	0x00000000
#define DRAMC_CMDDLY2_DEFAULT	0x00000000
#define DRAMC_CMDDLY3_DEFAULT	0x00000000
#define DRAMC_CMDDLY4_DEFAULT	0x00000000
#define DRAMC_CMDDLY5_DEFAULT	0x00000000
#define DRAMC_LPDDR2_DEFAULT	0x00000000
#define DRAMC_SPCMD_DEFAULT	0x00000000
#define DRAMC_DQODLY1_DEFAULT	0x00000000
#define DRAMC_DQODLY2_DEFAULT	0x00000000
#define DRAMC_DQODLY3_DEFAULT	0x00000000
#define DRAMC_DQODLY4_DEFAULT	0x00000000
#define DRAMC_DQIDLY1_DEFAULT	0x00000000
#define DRAMC_DQIDLY2_DEFAULT	0x00000000
#define DRAMC_DQIDLY3_DEFAULT	0x00000000
#define DRAMC_DQIDLY4_DEFAULT	0x00000000
#define DRAMC_DQIDLY5_DEFAULT	0x00000000
#define DRAMC_DQIDLY6_DEFAULT	0x00000000
#define DRAMC_DQIDLY7_DEFAULT	0x00000000
#define DRAMC_DQIDLY8_DEFAULT	0x00000000
#define DRAMC_DQICAL0_DEFAULT	0x00000000
#define DRAMC_DQICAL1_DEFAULT	0x00000000
#define DRAMC_DQICAL2_DEFAULT	0x00000000
#define DRAMC_DQICAL3_DEFAULT	0x00000000
#define DRAMC_CMP_ERR_DEFAULT	0x00000000
#define DRAMC_BUSMON_DEFAULT	0x00000000
#define DRAMC_STBEN0_DEFAULT	0x00000000
#define DRAMC_STBEN1_DEFAULT	0x00000000
#define DRAMC_STBEN2_DEFAULT	0x00000000
#define DRAMC_STBEN3_DEFAULT	0x00000000
#define DRAMC_DQSDLY0_DEFAULT	0x00000000
#define DRAMC_DQSDLY1_DEFAULT	0x00000000
#define DRAMC_OFFDLY0_DEFAULT	0x00000000
#define DRAMC_OFFDLY1_DEFAULT	0x00000000
#define DRAMC_OFFDLY2_DEFAULT	0x00000000
#define DRAMC_OFFDLY3_DEFAULT	0x00000000
#define DRAMC_OFFDLY4_DEFAULT	0x00000000
#define DRAMC_OFFDLY5_DEFAULT	0x00000000
#define DRAMC_OFFDLY6_DEFAULT	0x00000000
#define DRAMC_OFFDLY7_DEFAULT	0x00000000
#define DRAMC_DLLCNT0_DEFAULT	0x00000000
#define DRAMC_DLLCNT1_DEFAULT	0x00000000
#define DRAMC_CKPHCNT_DEFAULT	0x00000000
#define DRAMC_TESTRPT_DEFAULT	0x00000000

#endif	// __EMI_HW_H__
