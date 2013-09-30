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

#include "typedefs.h"
#include "mtk_timer.h"
#include "uart.h"
#include "pll.h"
#include "platform.h"

#define MOD "PLL"

void mt_mode_switch(unsigned int mode)
{
    kal_uint32 temp = 0;

    switch (mode)
    {
        case ASYNC_MODE:
            temp = *CLK_CTL0;
            temp &= 0xFFF8FFFF;
            *CLK_CTL0 = temp; // set syncmodereq_acp = syncmodereq_m1 = syncmodereq_mo = async mode

            temp = *CLK_CTL0;
            temp &= 0xFFFFFFE0;
            *CLK_CTL0 = temp; // set div_sel = 0
            break;
        case SYNC_MODE:
            temp = *CLK_CTL0;
            temp &= 0xFFFFFFE0;
            temp |= (*TOP_CKDIV0 & 0x0000001F);
            *CLK_CTL0 = temp; // set div_sel in CLK_CTL0 to the value of ckdiv0_sel in TOP_CKDIV0

            while (!(*CLK_CTL0 & 0x08000000)); // check if the CLK_CTL0[27] is 1 (optional)

            temp = *CLK_CTL0;
            temp &= 0xFFF8FFFF;
            temp |= 0x00070000; // set syncmodereq_acp = syncmodereq_m1 = syncmodereq_mo = sync mode
            *CLK_CTL0 = temp;
            break;
        default:
            break;
    }
}

#define WPLL197_FREQ 197000 // 3G PLL frequency is 197Mhz
#define CLKSQ_FREQ   26000  // Default frequency is 26Mhz

kal_uint32 mt_get_bus_freq(void)
{
    kal_uint32 fout = 0, fbdiv = 0, fbsel = 0, prediv = 0, postdiv = 0, ckdiv = 0;

    if ((*TOP_CKMUXSEL & 0x3) == 0x0) // Using CLKSQ
    {
        fout = CLKSQ_FREQ;
    }
    else if ((*TOP_CKMUXSEL & 0x3) == 0x2) // Using WPLL
    {
        fout = WPLL197_FREQ;
    }
    else
    {
        fbdiv = ((*MAINPLL_CON0) & 0x7F00) >> 8;

        fbsel = ((*MAINPLL_CON0) & 0x0030) >> 4;
        if (fbsel == 0)
            fbsel = 1;
        else if (fbsel == 1)
            fbsel = 2;
        else
            fbsel = 4;

        prediv = ((*MAINPLL_CON0) & 0x00C0) >> 6;
        if (prediv == 0)
            prediv = 1;
        else if (prediv == 1)
            prediv = 2;
        else
            prediv = 4;

        postdiv = ((*MAINPLL_CON1) & 0x0030) >> 4;
        if (postdiv == 0)
            postdiv = 1;
        else if (postdiv == 1)
            postdiv = 2;
        else
            postdiv = 4;

        fout = CLKSQ_FREQ * (fbdiv + 1) * fbsel / prediv / postdiv; // KHz
    }

    ckdiv = ((*TOP_CKDIV0) & 0x00000018) >> 3;

    switch (ckdiv)
    {
        case 0:
            fout = fout;     // bus clock will not be divided
            break;
        case 1:
            fout = fout / 4; // bus clock will be divided by 4
            break;
        case 2:
            fout = fout / 5; // bus clock will be divided by 5
            break;
        case 3:
            fout = fout / 6; // bus clock will be divided by 6
            break;
        default:
            break; 
    }

    return fout;
}

void mt_pll_init(void)
{
    kal_uint32 temp = 0;

    *ARMPLL_CON2 = 0x0652;          // ARMPLL_BR = 1; ARMPLL_BP = 1; ARMPLL_DIVEN = 2
    *MAINPLL_CON2 = 0x0652;         // MAINPLL_BR = 1; MAINPLL_BP = 1; MAINPLL_DIVEN = 2
    *IPLL_CON2 = 0x0652;            // IPLL_BR = 1; IPLL_BP = 1; IPLL_DIVEN = 2

    *PLL_CON2 = 0x0000;             // WPLL, MDPLL, MEMPLL, MAINPLL, ARMPLL from ap_sleep control. CLKSQ_EN from SRCLKEN

    *PLL_CON9 = 0x271B;             // TVDDS stable time0 for tuning TVDDS enable timing
    *PLL_CON10 = 0x0229;            // TVDDS stable time2 for tuning TVDDS enable timing

    if (platform_chip_ver() >= CHIP_6577_E1)
    {
        if ((DRV_Reg32(HW_RESV) & (0x1 << 23)) && ((DRV_Reg32(HW_RESV) & (0x1 << 20)) == 0))
        {
            *ARMPLL_CON0 = 0x5CA0;
        }
        else
        {
            if (DRV_Reg32(HW_RESV) & (0x1 << 12))
            {
                if ((DRV_Reg32(HW_RESV) & (0x1 << 17)) && ((DRV_Reg32(HW_RESV) & (0x1 << 16)) == 0))
                {
                    *ARMPLL_CON0 = 0x5CA0;
                }
                else
                {
                    *ARMPLL_CON0 = 0x4CA0;
                }
            }
            else
            {
                *ARMPLL_CON0 = 0x4CA0;
            }
        }
        gpt_busy_wait_us(120);
    }
    else
    {
        if ((DRV_Reg32(HW_RESV) & (0x1 << 28)))
        {
            *ARMPLL_CON0 = 0x48A0;
        }
        else
        {
            *ARMPLL_CON0 = 0x4CA0;
        }
        gpt_busy_wait_us(120);
    }

    *MAINPLL_CON0 = 0x2460;         // MAINPLL 962Mhz
    gpt_busy_wait_us(120);          // wait 80us (min delay is 20us)

    *IPLL_CON1 = 0x0036;            // 2^6 * Tin
    *IPLL_CON0 = 0x5741;            // init ISPLL 143Mhz but default disable
    gpt_busy_wait_us(80);           // wait 80 (min delay is 20us)

    *UPLL_CON0 = 0x1710;            // USBPLL 624MHz (for USB 2.0 and 1.0 Phy)
    gpt_busy_wait_us(80);           // wait 80us (min delay is 20us)
    *UPLL_CON0 = 0x1712;            // Set USBPLL to 48Mhz output enable

    #if 0
        *MDPLL_CON0 = 0x1310;       // enable MDPLL 1040Mhz
        gpt_busy_wait_us(80);       // wait 80us (min delay is 20us)
        *MDPLL_CON0 = 0x1312;       // Set MDPLL to 297.14Mhz output enable
    #else
        if (CHIP_VER_E1 != platform_chip_ver())
            *MDPLL_CON0 = 0x1310;   // enable MDPLL 1040Mhz
        else
            *MDPLL_CON0 = 0x1311;   // restore to default value to turn off MDPLL and controlled by SRCLKEN
    #endif

    if (CHIP_VER_E1 != platform_chip_ver()) {
        *TOPCKGEN_CON3 = 0xC000;    //[14] = 1, choose MDPLL instead of MAINPLL. [15] = 1, Enable MDPLL clock path
    }

    *WPLL_CON0 = 0x403B;            // 3GPLL 197Mhz, 492Mhz, 61Mhz enable, turn on enable bit
    gpt_busy_wait_us(120);          // wait 120us (min delay is 30us)

    #if 0
        *WPLL_CON0 = 0x403E;        // 3GPLL 197Mhz, 281Mhz, 492Mhz, 61Mhz enable
    #endif

    *PLL_CON1 = 0x4113;             // Reference clock is from ANA 26Mhz (Clock to MEMPLL enable)

    *MEMPLL_CON0 = 0x0910;          // VCO = (BUS Freq/10) * (9+1) * 4 / 1
    *MEMPLL_CON1 = 0x0026;          // MEMPLL = VCO/4
    gpt_busy_wait_us(80);           // wait 80us (min delay is 20us)

    if (platform_chip_ver() >= CHIP_6577_E1)
    {
        *TOP_MISC = 0x102;              // Force MUX1 to use 26Mhz and turn on enable_gen
    }
    else
    {
        *TOP_MISC = 0x101;              // Force MUX1 to use 26Mhz and turn on enable_gen
    }

    *TOP_CKDIV0 = 0x10;             // BUS clock will divide by 5

    *TOP_CKDIV1 = 0xA;              // CA9 clock will divide by 2

    if (CHIP_VER_E1 != platform_chip_ver())
        *TOP_CKDIV23 = 0x1;         // enable clock divider 2 to export /4 clock to mm system
    else
        *TOP_CKDIV23 = 0x0;         // disable clock divider 2 to export /4 clock to mm system

    if (CHIP_VER_E1 != platform_chip_ver())
        *TOP_CKMUXSEL = 0x259;      // Mux0 using MAINPLL, Mux1 using ARMPLL, MUX2 using MAINPLL, Mux3 From Mux1, MUX4 using CLKSQ, MUX6 using CLKSQ
    else
        *TOP_CKMUXSEL = 0x249;      // Mux0 using MAINPLL, Mux1 using ARMPLL, MUX2 using CLKSQ, Mux3 From Mux1, MUX4 using CLKSQ, MUX6 using CLKSQ

    *TVDAC_CON0 = 0x1012;           // Power down TVDAC for power saving

    temp = *CLK_CTL0;
    temp |= 0x00000200;
    *CLK_CTL0 = temp;               // enable the L2C clock gating while SCU is in idle state

    temp = *PERI_PDN0;
    *PERI_PDN0 = temp | 0x7C;       // gate all PWM clock bits
}

int mt_pll_init2 (void)
{
    /*********************************************/
    /* 2nd setting to make CA9 be divided by 1   */
    /*********************************************/

    *TOP_CKDIV1 = 0x0;              // CA9 clock will divide by 1

    /*********************************************/
    /* 2nd setting according to dram information */
    /*********************************************/

    if (mt_get_dram_type() == 2)
    {
        *MAINPLL_CON0 = 0x2760;     // MAINPLL 1040Mhz
        gpt_busy_wait_us(120);      // wait 80us (min delay is 20us)

        *TOP_CKDIV0 = 0x8;          // BUS clock will divide by 4
        *TOP_CKDIV23 = 0x1;         // enable clock divider 2 to export /4 clock to mm system
        *TOP_CKMUXSEL = 0x259;      // Mux0 using MAINPLL, Mux1 using ARMPLL, MUX2 using MAINPLL, Mux3 From Mux1, MUX4 using CLKSQ, MUX6 using CLKSQ

        return 1;
    }
    else if (mt_get_dram_type() == 3)
    {
        *MAINPLL_CON0 = 0x4CA0;     // MAINPLL 1001\Mhz => 1001/6=166Mhz
        //*MAINPLL_CON0 = 0x58A0;   // MAINPLL 1196\Mhz => 1157/6=192.8Mhz
        //*MAINPLL_CON0 = 0x5BA0;   // MAINPLL 1196\Mhz => 1196/6=199.33Mhz
        gpt_busy_wait_us(120);      // wait 80us (min delay is 20us)

        *TOP_CKDIV0 = 0x18;         // BUS clock will divide by 6
        *TOP_CKDIV23 = 0x1;         // enable clock divider 2 to export /4 clock to mm system
        *TOP_CKMUXSEL = 0x259;      // Mux0 using MAINPLL, Mux1 using ARMPLL, MUX2 using MAINPLL, Mux3 From Mux1, MUX4 using CLKSQ, MUX6 using CLKSQ        
        gpt_busy_wait_us(240);      // wait 240us (min delay is 60us)        

        *MEMPLL_CON1 = 0x0016;      // MEMPLL = VCO/4   //2X Mode
        *MEMPLL_CON0 = 0x1310;      // VCO = (BUS Freq/10) * (9+1) * 4 / 1 //2X MODE

        gpt_busy_wait_us(80);       // wait 80us (min delay is 20us)

        return 1;
    }

    return 0;
}
