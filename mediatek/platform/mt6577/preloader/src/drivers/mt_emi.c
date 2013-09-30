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
#include <platform.h>
#include <mt_emi.h>
#include <dramc.h>
#include <platform.h>

#define GRAY_ENCODED(a) ((a)^((a)>>1))

extern int num_of_emi_records;
extern EMI_SETTINGS emi_settings[];

/* select the corresponding memory devices */
//#define HYNIX_LPDDR
//#define MICRON_LPDDR
//#define SAMSUNG_LPDDR
//#define MICRON_LPDDR2
//#define HYNIX_LPDDR2
//#define SAMSUNG_LPDDR2

//#if defined(HYNIX_LPDDR) || defined(MICRON_LPDDR) || defined(SAMSUNG_LPDDR)
//#define USE_LPDDR
//#elif defined(HYNIX_LPDDR2) || defined(MICRON_LPDDR2) || defined(SAMSUNG_LPDDR2)
//#define USE_LPDDR2
//#endif


#if defined(HYNIX_LPDDR)
#define EMI_CONA_VAL 0x00022022
#define DRAMC_DRVCTL0_VAL 0x88008800    /* TX I/O driving = 8 */
#define DRAMC_DRVCTL1_VAL 0x88008800    /* TX I/O driving = 8 */
#define DRAMC_ACTIM_VAL 0x22824154  /* tRCD=3T, tRP=3T, tFAW="don't care", tWR=3T, CL=3, tWTR=2T, tRC=11T, tRAS=8T */
#define DRAMC_GDDR3CTL1_VAL 0x00000000  /* 4 banks */
#define DRAMC_CONF1_VAL 0xF0040560  /* Disable power down. BL=4, tRRD=2T */ 
#define DRAMC_DDR2CTL_VAL 0x8283405C    /* DLE = 5 */
#define DRAM_MODE_REG_VAL 0x00000032    /* BL=4, CL=3 */
#define DRAM_EXT_MODE_REG_VAL 0x00000020    /* 1/2 driving */
#define DRAMC_TEST2_3_VAL 0x9F068CA0    /* tRFC=27T */
#define DRAMC_CONF2_VAL 0x00403361  /* refresh period = 1552 */
#define DRAMC_PD_CTR_VAL 0x11642842 /* refresh period = 100 */
#endif

#if defined(MICRON_LPDDR)
#define EMI_CONA_VAL 0x00022022
#define DRAMC_DRVCTL0_VAL 0x88008800    /* TX I/O driving = 8 */
#define DRAMC_DRVCTL1_VAL 0x88008800    /* TX I/O driving = 8 */
#define DRAMC_ACTIM_VAL 0x22824154  /* tRCD=3T, tRP=3T, tFAW="don't care", tWR=3T, CL=3, tWTR=2T, tRC=11T, tRAS=8T */
#define DRAMC_GDDR3CTL1_VAL 0x00000000  /* 4 banks */
#define DRAMC_CONF1_VAL 0xF0040560  /* BL=4, tRRD=2T */
#define DRAMC_DDR2CTL_VAL 0x8283405C    /* DLE = 5 */
#define DRAM_MODE_REG_VAL 0x00000032    /* BL=4, CL=3 */
#define DRAM_EXT_MODE_REG_VAL 0x00000020    /* 1/2 driving */
#define DRAMC_TEST2_3_VAL 0x9F008CA0    /* tRFC=15T */
#define DRAMC_CONF2_VAL 0x00403361  /* refresh period = 1552 */
#define DRAMC_PD_CTR_VAL 0x11642842 /* refresh period = 100 */
#endif

#if defined(SAMSUNG_LPDDR)
#define EMI_CONA_VAL 0x00022022
#define DRAMC_DRVCTL0_VAL 0x88008800    /* TX I/O driving = 8 */
#define DRAMC_DRVCTL1_VAL 0x88008800    /* TX I/O driving = 8 */
#define DRAMC_ACTIM_VAL 0x33824165  /* tRCD=4T, tRP=4T, tFAW="don't care", tWR=3T, CL=3, tWTR=2T, tRC=12T, tRAS=9T */
#define DRAMC_GDDR3CTL1_VAL 0x00000000  /* 4 banks */
#define DRAMC_CONF1_VAL 0xF00405A0  /* BL=4, tRRD=3T */
#define DRAMC_DDR2CTL_VAL 0x8283405C    /* DLE = 5 */
#define DRAM_MODE_REG_VAL 0x00000032    /* BL=4, CL=3 */
#define DRAM_EXT_MODE_REG_VAL 0x00000020    /* 1/2 driving */
#define DRAMC_TEST2_3_VAL 0x9F048CA0    /* tRFC=25T */
#define DRAMC_CONF2_VAL 0x00403361  /* refresh period = 1552 */
#define DRAMC_PD_CTR_VAL 0x11642842 /* refresh period = 100 */
#endif

#if defined(MICRON_LPDDR2)
#define EMI_CONA_VAL 0x00022112 /* 1 channel, dual rank, 14 raw, 3 bank, 9 column */
#define DRAMC_DRVCTL0_VAL 0xAA00AA00    /* TX I/O driving = 0xA */
#define DRAMC_DRVCTL1_VAL 0xAA00AA00    /* TX I/O driving = 0xA */
#define CMD_ADDR_OUTPUT_DLY 3
#define CLK_OUTPUT_DLY 0
#define DRAMC_ACTIM_VAL 0x45D844B8  /* tRCD = 5T, tRP = 6T, tFAW = 14T, tWR = 4T, CL = 3, tWTR = 2T, tRC = 17T, tRAS = 12T */
#define DRAMC_GDDR3CTL1_VAL 0x01000000  /* 8 banks */
#define DRAMC_CONF1_VAL 0xF00407A0  /* BL = 4, tRRD = 3T */
#define DRAMC_DDR2CTL_VAL 0xA0064170    /* tRTP = 2T, DLE = 7 */
#define DRAM_MODE_REG_63_VAL 0x003F0000 /* reset */
#define DRAM_MODE_REG_10_VAL 0x000A00FF /* I/O calibration */
#define DRAM_MODE_REG_1_VAL 0x00010032  /* nWR = 3, no wrap, sequential, BL = 4 */
#define DRAM_MODE_REG_2_VAL 0x00020002  /* RL = 4, WL = 2 */
#define DRAM_MODE_REG_3_VAL 0x00030003  /* 48 ohm */
#define DRAMC_TEST2_3_VAL 0x9F0E8CA0    /* tRFC = 35T */
#define DRAMC_CONF2_VAL 0x00406340  /* refresh period = 1024 */
#define DRAMC_PD_CTR_VAL 0x21352842 /* refresh period = 106 */
#endif

#if defined(HYNIX_LPDDR2)
#define EMI_CONA_VAL 0x00022112 /* 1 channel, dual rank, 14 raw, 3 bank, 9 column */
#define DRAMC_DRVCTL0_VAL 0xAA00AA00    /* TX I/O driving = 0xA */
#define DRAMC_DRVCTL1_VAL 0xAA00AA00    /* TX I/O driving = 0xA */
#define CMD_ADDR_OUTPUT_DLY 3
#define CLK_OUTPUT_DLY 0
#define DRAMC_ACTIM_VAL 0x45D844B8  /* tRCD = 5T, tRP = 6T, tFAW = 14T, tWR = 4T, CL = 3, tWTR = 2T, tRC = 17T, tRAS = 12T */
#define DRAMC_GDDR3CTL1_VAL 0x01000000  /* 8 banks */
#define DRAMC_CONF1_VAL 0xF00407A0  /* BL = 4, tRRD = 3T */
#define DRAMC_DDR2CTL_VAL 0xA0064170    /* tRTP = 2T, DLE = 7 */
#define DRAM_MODE_REG_63_VAL 0x003F0000 /* reset */
#define DRAM_MODE_REG_10_VAL 0x000A00FF /* I/O calibration */
#define DRAM_MODE_REG_1_VAL 0x00010032  /* nWR = 3, no wrap, sequential, BL = 4 */
#define DRAM_MODE_REG_2_VAL 0x00020002  /* RL = 4, WL = 2 */
#define DRAM_MODE_REG_3_VAL 0x00030003  /* 48 ohm */
#define DRAMC_TEST2_3_VAL 0x9F0E8CA0    /* tRFC = 35T */
#define DRAMC_CONF2_VAL 0x00406340  /* refresh period = 1036 */
#define DRAMC_PD_CTR_VAL 0x21322842 /* refresh period = 101 */
#endif

#if defined(SAMSUNG_LPDDR2)
#define EMI_CONA_VAL 0x00022122 /* 1 channel, dual rank, 14 raw, 3 bank, 10 column */
#define DRAMC_DRVCTL0_VAL 0xAA00AA00    /* TX I/O driving = 0xA */
#define DRAMC_DRVCTL1_VAL 0xAA00AA00    /* TX I/O driving = 0xA */
#define CMD_ADDR_OUTPUT_DLY 3
#define CLK_OUTPUT_DLY 0
#define DRAMC_ACTIM_VAL 0x45D844B8  /* tRCD = 5T, tRP = 6T, tFAW = 14T, tWR = 4T, CL = 3, tWTR = 2T, tRC = 17T, tRAS = 12T */
#define DRAMC_GDDR3CTL1_VAL 0x01000000  /* 8 banks */
#define DRAMC_CONF1_VAL 0xF00407A0  /* BL = 4, tRRD = 3T */
#define DRAMC_DDR2CTL_VAL 0xA0064170    /* tRTP = 2T, DLE = 7 */
#define DRAM_MODE_REG_63_VAL 0x003F0000 /* reset */
#define DRAM_MODE_REG_10_VAL 0x000A00FF /* I/O calibration */
#define DRAM_MODE_REG_1_VAL 0x00010032  /* nWR = 3, no wrap, sequential, BL = 4 */
#define DRAM_MODE_REG_2_VAL 0x00020002  /* RL = 4, WL = 2 */
#define DRAM_MODE_REG_3_VAL 0x00030003  /* 48 ohm */
#define DRAMC_TEST2_3_VAL 0x9F0E8CA0    /* tRFC = 35T */
#define DRAMC_CONF2_VAL 0x00406341  /* refresh period = 1027 */
#define DRAMC_PD_CTR_VAL 0x21332842 /* refresh period = 102 */
#endif

extern int dramc_calib(void);

/*
 * init_dram: Do initialization for LPDDR.
 */
static void init_dram1(EMI_SETTINGS *emi_setting) 
{
    /* number of RA, CA, BA */
    *(volatile unsigned int*) 0xC0003000 = emi_setting->EMI_CONA_VAL;  //--> customized from SPEC.

    if((emi_setting->DRAM_RANK_SIZE[0] + emi_setting->DRAM_RANK_SIZE[1]) == 0x30000000) {
          /* 6G ECO test*/
          *(volatile unsigned int*) 0xC0004114 |= (1 << 30);
          print("6G DDR1 Memory\n");

    }
    __asm__ __volatile__ ("dsb" : : : "memory");
    gpt_busy_wait_us(200);
    /* jitter calibration default value */
    *(volatile unsigned int*) 0xC00040D4 = 0x00000008;
    __asm__ __volatile__ ("dsb" : : : "memory");
    gpt_busy_wait_us(200);

    /* Tx IO driving */
    *(volatile unsigned int*) 0xC00040B8 = emi_setting->DRAMC_DRVCTL0_VAL;
    *(volatile unsigned int*) 0xC00040BC = emi_setting->DRAMC_DRVCTL1_VAL;

    /* TX DQS Delay */
    *(volatile unsigned int*) 0xC0004014 = emi_setting->DRAMC_PADCTL3_VAL;

    /* TX DQ Delay */
    *(volatile unsigned int*) 0xC0004200 = emi_setting->DRAMC_DQODLY_VAL;
    *(volatile unsigned int*) 0xC0004204 = emi_setting->DRAMC_DQODLY_VAL;
    *(volatile unsigned int*) 0xC0004208 = emi_setting->DRAMC_DQODLY_VAL;
    *(volatile unsigned int*) 0xC000420C = emi_setting->DRAMC_DQODLY_VAL;

    /* AC timing */
    *(volatile unsigned int*) 0xC0004000 = emi_setting->DRAMC_ACTIM_VAL;  //--> customized from SPEC.
    /* Use default value? */
    *(volatile unsigned int*) 0xC0004048 = 0x0000110D;
    /* PINMUX=1: LPDDR1. DRAM disable */
    *(volatile unsigned int*) 0xC00040D8 = 0x40500900;  //--> customized DRAM type to LPDDR1 PINXNUM=1
    /* Read FIFO length=8 */
    *(volatile unsigned int*) 0xC000408C = 0x00000001;
    /* ??? */
    *(volatile unsigned int*) 0xC0004090 = 0x80000000;
    /* DQS gating window fine tune */
    *(volatile unsigned int*) 0xC0004094 = 0xC0404040;
    /* DQS gating window coarse tune */
    *(volatile unsigned int*) 0xC00040DC = 0x83002002;
    /* DQS gating window coarse tune */
    *(volatile unsigned int*) 0xC00040E0 = 0x10002002;
    /* Use default value? */
    *(volatile unsigned int*) 0xC00040F0 = 0x00000000;
    /* 4-bank device, 1->PHYSYNM */
    *(volatile unsigned int*) 0xC00040F4 = emi_setting->DRAMC_GDDR3CTL1_VAL;
    /* ??? */
    *(volatile unsigned int*) 0xC0004168 = 0x00000000;
    /* DRAM enable */
    *(volatile unsigned int*) 0xC00040D8 = 0x40700900;  //--> customized DRAM type to LPDDR1 PINXNUM=1
    /* power down and AC timing */
    *(volatile unsigned int*) 0xC0004004 = emi_setting->DRAMC_CONF1_VAL;  //--> customized from SPEC.
    /* TR2W, TRTP, DLE */ 
    *(volatile unsigned int*) 0xC000407C = (emi_setting->DRAMC_DDR2CTL_VAL & 0xFFFFFF8F) | ((emi_setting->DRAMC_DLE_VAL & 0x7) << 4);
    /* DLL??? No use??? */
    *(volatile unsigned int*) 0xC0004028 = 0xF1200F01;
    /* CTO protocol enable. LPDDR2 disable. */
    *(volatile unsigned int*) 0xC00041E0 = 0x64000000;
    /* Use default value? */
    *(volatile unsigned int*) 0xC0004158 = 0x00000000;
    /* Use default value? */
    *(volatile unsigned int*) 0xC00040e4 = 0x00000004 | ((emi_setting->DRAMC_DLE_VAL & 0x8) << 1);
 
    __asm__ __volatile__ ("dsb" : : : "memory");
    gpt_busy_wait_us(200);

   
    /* Send NOP command. */
    *(volatile unsigned int*) 0xC00041e4 = 0x00000010;
    *(volatile unsigned int*) 0xC00041e4 = 0x00000000;
    /* Send precharge all command. */
    *(volatile unsigned int*) 0xC00041e4 = 0x00000004;
    *(volatile unsigned int*) 0xC00041e4 = 0x00000000;
    /* Send 1st auto refresh command. */
    *(volatile unsigned int*) 0xC00041e4 = 0x00000008;
    *(volatile unsigned int*) 0xC00041e4 = 0x00000000;
    /* Send 2nd auto refresh command. */
    *(volatile unsigned int*) 0xC00041e4 = 0x00000008;
    *(volatile unsigned int*) 0xC00041e4 = 0x00000000;

    /* set DRAM mode reigster */
    *(volatile unsigned int*) 0xC0004080 = 0x00000000; 
    *(volatile unsigned int*) 0xC0004088 = emi_setting->DDR1_MODE_REG;
    *(volatile unsigned int*) 0xC00041e4 = 0x00000001;
    *(volatile unsigned int*) 0xC00041e4 = 0x00000000; 

    /* Set DRAM extended mode register. */
    *(volatile unsigned int*) 0xC0004080 = 0x00800000;
    *(volatile unsigned int*) 0xC0004088 = emi_setting->DDR1_EXT_MODE_REG;
    *(volatile unsigned int*) 0xC00041e4 = 0x00000001;
    *(volatile unsigned int*) 0xC00041e4 = 0x00001100;

    /* Use default value? */
    *(volatile unsigned int*) 0xC000400C = 0x00000000;
    /* adv. precharge and AC timing */
    *(volatile unsigned int*) 0xC0004044 = emi_setting->DRAMC_TEST2_3_VAL;  //--> customized from SPEC.
    /*
     * refresh count
     *
     * NoteXXX: There are 2 methods for refresh count. 
     *          Use the new method in which a free-run clock (26MHz) is used for counting.
     *          Don't use the old method in which the DRAM clock (ex: 200.2MHz) is used for counting. (Otherwise, DCM cannot work.)
     *          The counter for the old method (in DRAMC_CONF2) is still set here.
     */
    //*(volatile unsigned int*) 0xC0004008 = emi_setting->DRAMC_CONF2_VAL;  //--> customized from SPEC.
    //*(volatile unsigned int*) 0xC00041DC = emi_setting->DRAMC_PD_CTRL_VAL;  //--> customized from SPEC.
    /* Use default value? No use??? */
    *(volatile unsigned int*) 0xC0004010 = 0x00000000;
    /* Loop back mode disable. DRAMON disable. */
    *(volatile unsigned int*) 0xC00040F8 = 0xEDCB000F;
    /* 0-> sync mode */
    *(volatile unsigned int*) 0xC00040FC = 0x00000000;
    /* RXDQ */
    *(volatile unsigned int*) 0xC0004210 = 0x00000000;
    *(volatile unsigned int*) 0xC0004214 = 0x00000000;
    *(volatile unsigned int*) 0xC0004218 = 0x00000000;
    *(volatile unsigned int*) 0xC000421C = 0x00000000;
    *(volatile unsigned int*) 0xC0004220 = 0x00000000;
    *(volatile unsigned int*) 0xC0004224 = 0x00000000;
    *(volatile unsigned int*) 0xC0004228 = 0x00000000;
    *(volatile unsigned int*) 0xC000422C = 0x00000000;
    /* RXDQSI : gray(0d60)==22 */
    *(volatile unsigned int*) 0xC0004018 = 0x3C3C3C3C;
    *(volatile unsigned int*) 0xC000403C = 0x01010000;
    *(volatile unsigned int*) 0xC0004040 = 0x00000080;
    *(volatile unsigned int*) 0xC0004048 = 0x0000D10D;

    /*Follow JEDEC SPEC, tINIT3 init sequence*/
    *(volatile unsigned int*) 0xC00040E4 &= (~0x00000004);
    return;
}

/*
 * init_dram: Do initialization for LPDDR2.
 */
static void init_dram2(EMI_SETTINGS *emi_setting) 
{
    unsigned int reg_val;
    unsigned int factor_val;

    /* number of RA, CA, BA */
    *(volatile unsigned int *)0xC0003000 = emi_setting->EMI_CONA_VAL;

    if((emi_setting->DRAM_RANK_SIZE[0] + emi_setting->DRAM_RANK_SIZE[1]) == 0x30000000) {
            /* 6G ECO test*/
            *(volatile unsigned int*) 0xC0004114 |= (1 << 29);
            print("6G DDR2 Memory\n");

    }

    /* wait at least 200 us */
    __asm__ __volatile__ ("dsb" : : : "memory");
    gpt_busy_wait_us(200);
    /* jitter calibration default value */
    *(volatile unsigned int*) 0xC00040D4 = 0x00000008;
    /* wait at least 200 us */
    __asm__ __volatile__ ("dsb" : : : "memory");
    gpt_busy_wait_us(200);

    /* Tx IO driving */
    *(volatile unsigned int *)0xC00040B8 = emi_setting->DRAMC_DRVCTL0_VAL;
    *(volatile unsigned int *)0xC00040BC = emi_setting->DRAMC_DRVCTL1_VAL;

    /* TX DQS Delay */
    *(volatile unsigned int*) 0xC0004014 = emi_setting->DRAMC_PADCTL3_VAL;

    /* TX DQ Delay */
    *(volatile unsigned int*) 0xC0004200 = emi_setting->DRAMC_DQODLY_VAL;
    *(volatile unsigned int*) 0xC0004204 = emi_setting->DRAMC_DQODLY_VAL;
    *(volatile unsigned int*) 0xC0004208 = emi_setting->DRAMC_DQODLY_VAL;
    *(volatile unsigned int*) 0xC000420C = emi_setting->DRAMC_DQODLY_VAL;

#if 0
    if (platform_chip_ver() == CHIP_VER_E2) {
        /* E2 */
    } else {
        /* E1 */
#endif
    if (1){
        /* CMD/ADDR output delay */
        factor_val = GRAY_ENCODED(emi_setting->DRAMC_ADDR_OUTPUT_DLY);

        reg_val = *(volatile unsigned int *)0xC00041A8;
        reg_val &= 0xE0E0E0E0;
        reg_val |= (factor_val  << 24) | (factor_val << 16) | (factor_val << 8) | factor_val;
        *(volatile unsigned int *)0xC00041A8 = reg_val;

        reg_val = *(volatile unsigned int *)0xC00041AC;
        reg_val &= 0xE0E0E0E0;
        reg_val |= (factor_val  << 24) | (factor_val << 16) | (factor_val << 8) | factor_val;
        *(volatile unsigned int *)0xC00041AC = reg_val;

        reg_val = *(volatile unsigned int *)0xC00041B0;
        reg_val &= 0xE0E0E0E0;
        reg_val |= (factor_val  << 24) | (factor_val << 16) | (factor_val << 8) | factor_val;
        *(volatile unsigned int *)0xC00041B0 = reg_val;

        reg_val = *(volatile unsigned int *)0xC00041B4;
        reg_val &= 0xE0E0E0E0;
        reg_val |= (factor_val  << 24) | (factor_val << 16) | (factor_val << 8) | factor_val;
        *(volatile unsigned int *)0xC00041B4 = reg_val;

        reg_val = *(volatile unsigned int *)0xC00041B8;
        reg_val &= 0xE0E0E0E0;
        reg_val |= (factor_val  << 24) | (factor_val << 16) | (factor_val << 8) | factor_val;
        *(volatile unsigned int *)0xC00041B8 = reg_val;

        reg_val = *(volatile unsigned int *)0xC00041BC;
        reg_val &= 0xFFFFFFE0;
        reg_val |= (factor_val << 16) | factor_val;
        *(volatile unsigned int *)0xC00041BC = reg_val;

        /* CLK output delay */
        factor_val = GRAY_ENCODED(emi_setting->DRAMC_CLK_OUTPUT_DLY);
        reg_val = *(volatile unsigned int *)0xC000400C;
        reg_val &= 0xF0FFFFFF;
        reg_val |= factor_val << 24;
        *(volatile unsigned int *)0xC000400C = reg_val;
        /* need to set DQO and DQSO together if CLK output delay is not 0 */
#if 0
        dqo_dly_factor_handler(factor_value);
        dqso_dly_factor_handler(factor_value);
#endif
    }

    /* AC timing */
    *(volatile unsigned int *)0xC0004000 = emi_setting->DRAMC_ACTIM_VAL;  //--> customized from SPEC.
    *(volatile unsigned int *)0xC0004048 = 0x0000110D;
    *(volatile unsigned int *)0xC00040D8 = 0x00500900; 
    *(volatile unsigned int *)0xC000408C = 0x00000001;
    *(volatile unsigned int *)0xC0004090 = 0x00000000;
    /* DQS gating window fine tune */
    *(volatile unsigned int *)0xC0004094 = 0xC0404040;
    /* DQS gating window coarse tune */
    *(volatile unsigned int *)0xC00040DC = 0x82010010;
    /* DQS gating window coarse tune */
    *(volatile unsigned int *)0xC00040E0 = 0x11010010;
    *(volatile unsigned int *)0xC00040F0 = 0x00000000;
    *(volatile unsigned int *)0xC00040F4 = emi_setting->DRAMC_GDDR3CTL1_VAL;
    *(volatile unsigned int *)0xC0004168 = 0x00000000;
    *(volatile unsigned int *)0xC00040D8 = 0x00700900;
    *(volatile unsigned int *)0xC0004004 = emi_setting->DRAMC_CONF1_VAL;  //--> customized from SPEC.
    /* TR2W, TRTP, DLE */ 
    *(volatile unsigned int*) 0xC000407C = (emi_setting->DRAMC_DDR2CTL_VAL & 0xFFFFFF8F) | ((emi_setting->DRAMC_DLE_VAL & 0x7) << 4);
    *(volatile unsigned int *)0xC0004028 = 0xF1200F01;
    *(volatile unsigned int *)0xC00041E0 = 0x3101CFF5;
    *(volatile unsigned int *)0xC0004158 = 0x00000000;

    /*Follow JEDEC SPEC, tINIT3 init sequence*/
    *(volatile unsigned int*) 0xC00040E4 = 0x00000004 | ((emi_setting->DRAMC_DLE_VAL & 0x8) << 1);
    __asm__ __volatile__ ("dsb" : : : "memory");
    gpt_busy_wait_us(200);

#if 1
    /* Send precharge all command. */
    *(volatile unsigned int*) 0xC00041E4 = 0x00000004;
    *(volatile unsigned int*) 0xC00041E4 = 0x00000000;
#endif

    
    *(volatile unsigned int *)0xC0004088 = emi_setting->DDR2_MODE_REG63;
    *(volatile unsigned int *)0xC00041E4 = 0x00000001;
    *(volatile unsigned int *)0xC00041E4 = 0x00000000;

    /* wait 10 us */
    __asm__ __volatile__ ("dsb" : : : "memory");
    gpt_busy_wait_us(10);

    *(volatile unsigned int *)0xC0004088 = emi_setting->DDR2_MODE_REG10;
    *(volatile unsigned int *)0xC00041E4 = 0x00000001;
    *(volatile unsigned int *)0xC00041E4 = 0x00000000;

    /* wait 1 us */
    __asm__ __volatile__ ("dsb" : : : "memory");
    gpt_busy_wait_us(1);

    *(volatile unsigned int *)0xC0004088 = emi_setting->DDR2_MODE_REG1;
    *(volatile unsigned int *)0xC00041E4 = 0x00000001;
    *(volatile unsigned int *)0xC00041E4 = 0x00000000;

    *(volatile unsigned int *)0xC0004088 = emi_setting->DDR2_MODE_REG2;
    *(volatile unsigned int *)0xC00041E4 = 0x00000001;
    *(volatile unsigned int *)0xC00041E4 = 0x00000000;

    *(volatile unsigned int *)0xC0004088 = emi_setting->DDR2_MODE_REG3;
    *(volatile unsigned int *)0xC00041E4 = 0x00000001;
    *(volatile unsigned int *)0xC00041E4 = 0x00000000;

    *(volatile unsigned int *)0xC0004084 = 0x00000A56;
    *(volatile unsigned int *)0xC000400C = 0x00000000;
    /* adv. precharge and AC timing */
    *(volatile unsigned int *)0xC0004044 = emi_setting->DRAMC_TEST2_3_VAL;  //--> customized from SPEC.
    /*
     * refresh count
     *
     * NoteXXX: There are 2 methods for refresh count. 
     *          Use the new method in which a free-run clock (26MHz) is used for counting.
     *          Don't use the old method in which the DRAM clock (ex: 200.2MHz) is used for counting. (Otherwise, DCM cannot work.)
     *          The counter for the old method (in DRAMC_CONF2) is still set here.
     */
    //*(volatile unsigned int *)0xC0004008 = emi_setting->DRAMC_CONF2_VAL;  //--> customized from SPEC.
    //*(volatile unsigned int *)0xC00041DC = emi_setting->DRAMC_PD_CTRL_VAL;  //--> customized from SPEC.
    *(volatile unsigned int *)0xC0004010 = 0x00000000;
    /* Loop back mode disable. DRAMON disable. */
    *(volatile unsigned int *)0xC00040F8 = 0xEDCB000F;
    *(volatile unsigned int *)0xC00040FC = 0x00010000;
    /* RXDQ */
    *(volatile unsigned int *)0xC0004210 = 0x00000000;
    *(volatile unsigned int *)0xC0004214 = 0x00000000;
    *(volatile unsigned int *)0xC0004218 = 0x00000000;
    *(volatile unsigned int *)0xC000421C = 0x00000000;
    *(volatile unsigned int *)0xC0004220 = 0x00000000;
    *(volatile unsigned int *)0xC0004224 = 0x00000000;
    *(volatile unsigned int *)0xC0004228 = 0x00000000;
    *(volatile unsigned int *)0xC000422C = 0x00000000;
    /* RXDQSI : gray(0d44)==3A */
    *(volatile unsigned int *)0xC0004018 = 0x3A3A3A3A;
    *(volatile unsigned int *)0xC000403C = 0x01010000;
    *(volatile unsigned int *)0xC0004040 = 0x00000080;
    *(volatile unsigned int *)0xC0004048 = 0x0000D10D;

    *(volatile unsigned int *)0xC00040CC |= 0x000000C3;

    /*Follow JEDEC SPEC, tINIT3 init sequence*/
    *(volatile unsigned int*) 0xC00040E4 &= (~0x00000004); 
}

/*
 * init_dram: Do initialization for DDR3.
 */
#define DDR3_MODE_0 0x00000000
#define DDR3_MODE_1 0x00400000
#define DDR3_MODE_2 0x00800000
#define DDR3_MODE_3 0x00c00000
unsigned int ddr_type=0;
static void init_dram3(EMI_SETTINGS *emi_setting) 
{
    unsigned int reg_val;
    unsigned int factor_val;
    ddr_type=3;
    /* number of RA, CA, BA */
    *(volatile unsigned int *)0xC0003000 = emi_setting->EMI_CONA_VAL;

    /* wait at least 200 us */
    delay_a_while(200000);

    /* jitter calibration default value */
    *(volatile unsigned int*) 0xC00040D4 = 0x00000008;
    /* wait at least 200 us */
    delay_a_while(200000);

    /* Tx IO driving */
    *(volatile unsigned int *)0xC00040B8 = emi_setting->DRAMC_DRVCTL0_VAL;
    *(volatile unsigned int *)0xC00040BC = emi_setting->DRAMC_DRVCTL1_VAL;

    /* TX DQS Delay */
    *(volatile unsigned int*) 0xC0004014 = emi_setting->DRAMC_PADCTL3_VAL;

    /* TX DQ Delay */
    *(volatile unsigned int*) 0xC0004200 = emi_setting->DRAMC_DQODLY_VAL;
    *(volatile unsigned int*) 0xC0004204 = emi_setting->DRAMC_DQODLY_VAL;
    *(volatile unsigned int*) 0xC0004208 = emi_setting->DRAMC_DQODLY_VAL;
    *(volatile unsigned int*) 0xC000420C = emi_setting->DRAMC_DQODLY_VAL;

#if 0
    if (platform_chip_ver() == CHIP_VER_E2) {
        /* E2 */
    } else {
        /* E1 */
#endif
    if (1){
        /* E1 */
        /* CMD/ADDR output delay */
        factor_val = GRAY_ENCODED(emi_setting->DRAMC_ADDR_OUTPUT_DLY);

        reg_val = *(volatile unsigned int *)0xC00041A8;
        reg_val &= 0xE0E0E0E0;
        reg_val |= (factor_val  << 24) | (factor_val << 16) | (factor_val << 8) | factor_val;
        *(volatile unsigned int *)0xC00041A8 = reg_val;

        reg_val = *(volatile unsigned int *)0xC00041AC;
        reg_val &= 0xE0E0E0E0;
        reg_val |= (factor_val  << 24) | (factor_val << 16) | (factor_val << 8) | factor_val;
        *(volatile unsigned int *)0xC00041AC = reg_val;

        reg_val = *(volatile unsigned int *)0xC00041B0;
        reg_val &= 0xE0E0E0E0;
        reg_val |= (factor_val  << 24) | (factor_val << 16) | (factor_val << 8) | factor_val;
        *(volatile unsigned int *)0xC00041B0 = reg_val;

        reg_val = *(volatile unsigned int *)0xC00041B4;
        reg_val &= 0xE0E0E0E0;
        reg_val |= (factor_val  << 24) | (factor_val << 16) | (factor_val << 8) | factor_val;
        *(volatile unsigned int *)0xC00041B4 = reg_val;

        reg_val = *(volatile unsigned int *)0xC00041B8;
        reg_val &= 0xE0E0E0E0;
        reg_val |= (factor_val  << 24) | (factor_val << 16) | (factor_val << 8) | factor_val;
        *(volatile unsigned int *)0xC00041B8 = reg_val;

        reg_val = *(volatile unsigned int *)0xC00041BC;
        reg_val &= 0xFFFFFFE0;
        reg_val |= (factor_val << 16) | factor_val;
        *(volatile unsigned int *)0xC00041BC = reg_val;

        /* CLK output delay */
        factor_val = GRAY_ENCODED(emi_setting->DRAMC_CLK_OUTPUT_DLY);
        reg_val = *(volatile unsigned int *)0xC000400C;
        reg_val &= 0xF0FFFFFF;
        reg_val |= factor_val << 24;
        *(volatile unsigned int *)0xC000400C = reg_val;
        /* need to set DQO and DQSO together if CLK output delay is not 0 */
#if 0
        dqo_dly_factor_handler(factor_value);
        dqso_dly_factor_handler(factor_value);
#endif
    }

    /* AC timing */
    *(volatile unsigned int *)0xC0004000 = emi_setting->DRAMC_ACTIM_VAL;  //--> customized from SPEC.
    *(volatile unsigned int *)0xC0004048 = 0x0000110D;      //mempll, test agent
    *(volatile unsigned int *)0xC00040D8 = 0x80100900;      //pinmux to ddr3,disable ODT,DMPLL divider
    *(volatile unsigned int *)0xC000408C = 0x00000001;      //FIFO length 8
    *(volatile unsigned int *)0xC0004090 = 0x00000000;
    /* DQS gating window fine tune */
    *(volatile unsigned int *)0xC0004094 = 0x80000000;
    /* DQS gating window coarse tune */
    *(volatile unsigned int *)0xC00040DC = 0x83020020;
    /* DQS gating window coarse tune */
    *(volatile unsigned int *)0xC00040E0 = 0x11020020;       // Burst Mode
    *(volatile unsigned int *)0xC00040F0 = 0x00000000;       //PHY enable, DQS input h.w. control
    *(volatile unsigned int *)0xC00040F4 = emi_setting->DRAMC_GDDR3CTL1_VAL;
    *(volatile unsigned int *)0xC0004168 = 0x00000000;
    *(volatile unsigned int *)0xC00040D8 = 0x80300900;       //dram enable
    *(volatile unsigned int *)0xC0004004 = emi_setting->DRAMC_CONF1_VAL;  //--> customized from SPEC.
    /* TR2W, TRTP, DLE */ 
    *(volatile unsigned int*) 0xC000407C = (emi_setting->DRAMC_DDR2CTL_VAL ) | ((emi_setting->DRAMC_DLE_VAL & 0x7) << 4);
    *(volatile unsigned int *)0xC0004028 = 0xF1200F01;       //DLL config
    *(volatile unsigned int *)0xC00041E0 = 0x68000000;       //LPDDR2 Disable
    *(volatile unsigned int *)0xC0004158 = 0x00000000;

    *(volatile unsigned int*) 0xC00040E4 = 0x000000A8 | ((emi_setting->DRAMC_DLE_VAL & 0x8) << 1); //DDR3EN , CKE always OFF
    /* wait at least 200 us */
    delay_a_while(200000);
     /* wait at least 10 us */
    delay_a_while(10000);
    *(volatile unsigned int*) 0xC00040E4 = 0x000000AA | ((emi_setting->DRAMC_DLE_VAL & 0x8) << 1); //Reset Enable , CKE always OFF
    
    /* wait at least 500 us */
    delay_a_while(500000);	                              
    *(volatile unsigned int*) 0xC00040E4 = 0x000000A6 | ((emi_setting->DRAMC_DLE_VAL & 0x8) << 1); //DDR3EN , CKE always ON
    delay_a_while(270);	                                          //do NOP for tXPR (64T + 10 ns)

	  /* Mode Register Setting */	  
	  *(volatile unsigned int *)0xC0004080 = DDR3_MODE_2;           
	  *(volatile unsigned int *)0xC0004088 = emi_setting->DDR3_MODE_REG2;                //CAS Write Latency=5    //Dynamic ODT=Disable
    *(volatile unsigned int *)0xC00041E4 = 0x00000001;            //Auto Self Refresh=Disable
    *(volatile unsigned int *)0xC00041E4 = 0x00000000;            //Self Refresh Temperature=Normal
    delay_a_while(20);                                            //tMRD (4T)
	                                                                    
	  *(volatile unsigned int *)0xC0004080 = DDR3_MODE_3;           //MultiPurpose Register Select=Pre-defined pattern
	  *(volatile unsigned int *)0xC0004088 = emi_setting->DDR3_MODE_REG3;                //MultiPurpose Register Enable=Disabled
    *(volatile unsigned int *)0xC00041E4 = 0x00000001;
    *(volatile unsigned int *)0xC00041E4 = 0x00000000;                                                  
    delay_a_while(20);	                                          //tMRD (4T)
    
	  *(volatile unsigned int *)0xC0004080 = DDR3_MODE_1;           //DLL Enable = Enabled              //Write Levelization=Disable
	  *(volatile unsigned int *)0xC0004088 = emi_setting->DDR3_MODE_REG1;                //Output Drive Strength=40 ohm      //TDQS Enable=Disable
    *(volatile unsigned int *)0xC00041E4 = 0x00000001;            //ODT Rtt=Disabled                  //Qoff=Enabled 
    *(volatile unsigned int *)0xC00041E4 = 0x00000000;	          //Additive Latency=0	                                                                   	                                                                    	                                                                     	                                                  
    delay_a_while(20);		                                        //tMRD (4T)
	                                                                    
	  *(volatile unsigned int *)0xC0004080 = DDR3_MODE_0;           //Burst Length= Fixed 4 (chop)      //Write Recovery=5
	  *(volatile unsigned int *)0xC0004088 = emi_setting->DDR3_MODE_REG0;                //Burst Order=Sequential            //Power Down Mode=DLL off
    *(volatile unsigned int *)0xC00041E4 = 0x00000001;            //CAS Latency=6
    *(volatile unsigned int *)0xC00041E4 = 0x00000000;	 	        //DLL Reset=Reset DLL
    delay_a_while(50);	                                          //tMOD (12T)

    
    *(volatile unsigned int *)0xC0004004 = (emi_setting->DRAMC_CONF1_VAL & 0xFFDFFFFF); //ZQ long=1
    *(volatile unsigned int *)0xC0004088 = 0x00000400;
    *(volatile unsigned int *)0xC00041E4 = 0x00000010;
    *(volatile unsigned int *)0xC00041E4 = 0x00000000;           //initialization finish
    delay_a_while(2000);                                             //tZQinit (512T) 

    *(volatile unsigned int *)0xC0004004 = emi_setting->DRAMC_CONF1_VAL;
    *(volatile unsigned int *)0xC00041E4 = 0x00001100;     	         //Enable IO ring counter, DQS gating window counter
    //*(volatile unsigned int *)0xC00040E4 = 0x00000082; 	             //DDR3 Enable and reset
    *(volatile unsigned int*) 0xC00040E4 = 0x000000A2 | ((emi_setting->DRAMC_DLE_VAL & 0x8) << 1); //DDR3EN , CKE H.W. Control
    	       
    *(volatile unsigned int *)0xC0004088 = 0x0000FFFF;  	       
    *(volatile unsigned int *)0xC00041E4 = 0x00000020;     
    *(volatile unsigned int *)0xC00041E4 = 0x00000000;    

    *(volatile unsigned int *)0xC0004004 = (emi_setting->DRAMC_CONF1_VAL & 0xFF0FFFFF); 
    *(volatile unsigned int *)0xC000400C = 0x00000000;	

    /* adv. precharge and AC timing */
    *(volatile unsigned int *)0xC0004044 = emi_setting->DRAMC_TEST2_3_VAL;  //--> customized from SPEC.
    /*
     * refresh count
     *
     * NoteXXX: There are 2 methods for refresh count. 
     *          Use the new method in which a free-run clock (26MHz) is used for counting.
     *          Don't use the old method in which the DRAM clock (ex: 200.2MHz) is used for counting. (Otherwise, DCM cannot work.)
     *          The counter for the old method (in DRAMC_CONF2) is still set here.
     */
    *(volatile unsigned int *)0xC0004008 = emi_setting->DRAMC_CONF2_VAL;  //--> customized from SPEC.
    *(volatile unsigned int *)0xC00041DC = emi_setting->DRAMC_PD_CTRL_VAL;  //--> customized from SPEC.
    *(volatile unsigned int *)0xC0004010 = 0x00000000;                     //DQ IO PAD delay
    /* Loop back mode disable. DRAMON disable. */
    *(volatile unsigned int *)0xC00040F8 = 0xEDCB000F;
    *(volatile unsigned int *)0xC00040FC = 0x00000000;

    /* RXDQ */
    *(volatile unsigned int *)0xC0004210 = 0x00000000;
    *(volatile unsigned int *)0xC0004214 = 0x00000000;
    *(volatile unsigned int *)0xC0004218 = 0x00000000;
    *(volatile unsigned int *)0xC000421C = 0x00000000;
    *(volatile unsigned int *)0xC0004220 = 0x00000000;
    *(volatile unsigned int *)0xC0004224 = 0x00000000;
    *(volatile unsigned int *)0xC0004228 = 0x00000000;
    *(volatile unsigned int *)0xC000422C = 0x00000000;
    /* RXDQSI : gray(0d44)==3A */
    *(volatile unsigned int *)0xC0004018 = 0x3A3A3A3A;
    *(volatile unsigned int *)0xC000403C = 0x01010000;
    *(volatile unsigned int *)0xC0004040 = 0x00000080;
    *(volatile unsigned int *)0xC0004048 = 0x0000D10D;
    print("DDR3 bus clock = %d Mhz\n", (mt_get_bus_freq()/1000));   
}

static char id[16];

static int mt_get_mdl_number (void)
{
    static int found = 0;
    static int mdl_number = -1;
    int i;

    if (!found)
    {
        int result;
#if 0
        mdl_number = 0; found = 1; return mdl_number;
#endif
        result = platform_get_mcp_id (id, sizeof(id));

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
                            if (memcmp(id, emi_settings[i].ID, 9) == 0)
                                break; /* found */
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

void get_dram_rank_size (int dram_rank_size[])
{
    int index,/* bits,*/ rank_nr, i;
    //int emi_cona;

    
    index = mt_get_mdl_number ();

    if (index < 0 || index >=  num_of_emi_records)
    {
        return;
    }

    rank_nr = get_dram_rank_nr();

    for(i = 0; i < rank_nr; i++)
        dram_rank_size[i] = emi_settings[index].DRAM_RANK_SIZE[i];
}

/*
 * mt_set_emi: Set up EMI/DRAMC.
 */
void mt_set_emi (void)
{
    int index = 0;
    EMI_SETTINGS *emi_set;

    if (platform_chip_ver() != CHIP_VER_E1) {
        /* reset the max pending count */
        *(volatile unsigned int *)0xC0004168 = 0x0;
    }

    print("[EMI] DDR%d\r\n", mt_get_dram_type ());

    print("[EMI] eMMC/NAND ID = %x,%x,%x,%x,%x,%x,%x,%x,%x\r\n", id[0], id[1], id[2], id[3], id[4], id[5], id[6], id[7], id[8]);

    index = mt_get_mdl_number ();

    if (index < 0 || index >=  num_of_emi_records)
    {
        print("[EMI] setting failed 0x%x\r\n", index);
        return;
    }

    print("[EMI] MDL number = %d\r\n", index);

    emi_set = &emi_settings[index];

    //Disable auto-refresh 
    *(volatile unsigned int *)0xC0004008 &= ~(0xFF << 0);
    *(volatile unsigned int *)0xC00041DC &= ~(0xFF << 16);

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

    if (emi_set->EMI_CONA_VAL & 0x20000) /* check dual rank support */
    {
        /* swap CS0<->CS1 */
        *(volatile unsigned int *)0xC0004110 = 0x0805114B;
       
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
            print("FIXME : 6577 doesn't support dual rank DDR3\n");
            while(1);
        }

        /* swap CS1<->CS0 */
        *(volatile unsigned int *)0xC0004110 = 0x08051143;
    }
    //Enable auto-refresh 
    *(volatile unsigned int *)0xC0004008 = emi_set->DRAMC_CONF2_VAL;  //--> customized from SPEC.
    *(volatile unsigned int *)0xC00041DC = emi_set->DRAMC_PD_CTRL_VAL;  //--> customized from SPEC.

    if(mt_get_dram_type() == 1)
        *(volatile unsigned int*) 0xC0003148 = 0x00680F0F;
    else if(mt_get_dram_type() == 2)
        *(volatile unsigned int*) 0xC0003148 = 0x00460F1F;
    else if(mt_get_dram_type() == 3)
        *(volatile unsigned int*) 0xC0003148 = 0x00680F0F;
        
    if(mt_get_dram_type() == 1)
    *(volatile unsigned int *)0xC0003150 = 0x00000830;
    else if (mt_get_dram_type() == 2)
    *(volatile unsigned int *)0xC0003150 = 0x00000830;
    else if (mt_get_dram_type() == 3)
        *(volatile unsigned int *)0xC0003150 = 0x00000845;

#ifdef MTK_DISPLAY_HIGH_RESOLUTION
        *(volatile unsigned int *)0xC0003158 = 0x00004000;
#endif
 
    *(volatile unsigned int*) 0xC00030F0 = 0x04080008;
    if (platform_chip_ver() == CHIP_VER_E1) {
        *(volatile unsigned int *)0xC00030F8 = 0x90000000; 
        *(volatile unsigned int *)0xC0003060 = 0x00000500;
    } else {
        if ((emi_set->type & 0xF) == 3)
        {
        *(volatile unsigned int *)0xC00030F8 = 0x0; 
        }
        else
        {
        *(volatile unsigned int *)0xC00030F8 = 0x80000000; 
        }
        if ((emi_set->type & 0xF) == 1)
        {
            *(volatile unsigned int *)0xC0003060 = 0x00000580;
        }
        else
        {
            /* because of H/W speed limitation, EMI scramble can't be enabled */
            *(volatile unsigned int *)0xC0003060 = 0x00000500;
        }
    }
    /* for reducing leakage current */
    *(volatile unsigned int *)0xC000407C |= 0x00000080;

    /* DRAMC calibration */
    if (dramc_calib() < 0) {
        print("[EMI] DRAMC calibration failed\n\r");
    } else {
        print("[EMI] DRAMC calibration passed\n\r");
    }

    /* FIXME: modem exception occurs if set the max pending count */
#if 1
    if (platform_chip_ver() != CHIP_VER_E1) {
        /* set the max pending count */
        if(mt_get_dram_type() == DDR1)
            *(volatile unsigned int *)0xC0004168 = 0x8;
        else if(mt_get_dram_type() == DDR2)
            *(volatile unsigned int *)0xC0004168 = 0xC;
        else if(mt_get_dram_type() == DDR3)
            *(volatile unsigned int *)0xC0004168 = 0xC;
    }
#endif
    /*Fix Vm12 drop issue*/
    if(mt_get_dram_type() == DDR2) {
        *(volatile unsigned int *)0xC00041DC |= (1 << 26);
    }
}
void get_dram_id(char* id1)
{
     memcpy(id1,id,9);
}
void mt_set_emi_try (int sencond)
{
	int index = 0;
       EMI_SETTINGS *emi_set;
	print("[EMI] mt_set_emi_try \n\r");
	index = mt_get_mdl_number ();

   	 if (index < 0 || index >=  num_of_emi_records)
    	{
        	print("[EMI] setting failed 0x%x\r\n", index);
        	return;
    	}

    	print("[EMI] MDL number = %d\r\n", index);
       
    	emi_set = &emi_settings[index];
	if(sencond){
		emi_set->EMI_CONA_VAL|=(unsigned int)(0x20000);     //0x0002212E
		emi_set->DRAM_RANK_SIZE[1]=emi_set->DRAM_RANK_SIZE[0];
	}
	else{
              emi_set->EMI_CONA_VAL&=(unsigned int)(~0x20000);     //0x0000212E
              emi_set->DRAM_RANK_SIZE[1]=0;
	}
       *(volatile unsigned int *)0xC0004110=0x8051100;
	mt_set_emi();
}

