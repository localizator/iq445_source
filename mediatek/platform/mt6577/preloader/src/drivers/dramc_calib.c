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

#include <dramc.h>
#include <typedefs.h>
#include <mt6577.h>
#include <emi_hw.h>

#define DEBUG_DRAMC_CALIB
#define DEC_DQS_VALUE (6)
#define DDR3_DEC_DQS_VALUE (13)
#define DEFAULT_DQSSTEP_VALUE 4
#define HW_DQ_DQS_CALIB_TIMEOUT 10000

/*
 * set_dqs_input_delay: Get DQS input delay value according to the calibration result.
 * @min_lsb: LSB part of the min value from calibration 
 * @min_msb: MSB part of the min value from calibration 
 * Return the DQS input delay value for success. Return negative values for failure.
 */
static int dqs_input_delay(unsigned int min_lsb, unsigned int min_msb)
{
    unsigned char data1[8], data2[8];
    int i;
    unsigned int val1, val2, max;

    *(unsigned int *)&(data1[0]) = *(volatile unsigned long long *)min_lsb;
    *(unsigned int *)&(data1[4]) = *(volatile unsigned long long *)min_msb;
    *(unsigned int *)&(data2[0]) = *(volatile unsigned long long *)(min_lsb + 32);
    *(unsigned int *)&(data2[4]) = *(volatile unsigned long long *)(min_msb + 32);

    max = 0;
    for (i = 0; i < 8; i++) {
        val1 = (unsigned int)(data1[i]);
        val1 &= 0x000000FF;
        val2 = (unsigned int)(data2[i]);
        val2 &= 0x000000FF;

        if (val1 > val2) {
            dbg_print("Critical error in DRAMC calibration. Min input delay > max input delay.\n\r");
            return -1;
        } else if ((val1 + (val2 - val1) / 2) > max) {
            max = (val1 + (val2 - val1) / 2);
        }
    }

    return max;
}

/*
 * set_dq_input_delay: Set DQ input delay value according to the calibration result.
 * @from: calibration result register
 * @to: DQ input delay register
 * @dqsi_dly: DQS input delay
 * @dqs_cal_min: DQS input calibration register
 */
static int set_dq_input_delay(unsigned int from, unsigned int to, unsigned int dqsi_dly, unsigned int dqs_cal_min)
{
    unsigned int data, b0, b1, b2, b3;
    unsigned int val1, val2;
    unsigned int dqs_cal_max = dqs_cal_min + 32;
    unsigned int dqsi_dly_per_bit;

    data = *(volatile unsigned int *)from;

    b3 = (data >> 24) & 0x000000FF;
    val2 = ((*(volatile unsigned int *)dqs_cal_max) >> 24) & 0x000000FF;
    val1 = ((*(volatile unsigned int *)dqs_cal_min) >> 24) & 0x000000FF;
    if (val1 > val2) {
        dbg_print("Critical error in DRAMC calibration. Min input delay > max input delay.\n\r");
        return -1;
    } else {
        dqsi_dly_per_bit = val1 + (val2 - val1) / 2;
    }
    if (dqsi_dly < dqsi_dly_per_bit) {
        dbg_print("Critical error in DRAMC calibration. DQS input delay (per bit) > max dqs input delay.\n\r");
    } else {
        b3 += dqsi_dly - dqsi_dly_per_bit;
    }

    b2 = (data >> 16) & 0x000000FF;
    val2 = ((*(volatile unsigned int *)dqs_cal_max) >> 16) & 0x000000FF;
    val1 = ((*(volatile unsigned int *)dqs_cal_min) >> 16) & 0x000000FF;
    if (val1 > val2) {
        dbg_print("Critical error in DRAMC calibration. Min input delay > max input delay.\n\r");
        return -1;
    } else {
        dqsi_dly_per_bit = val1 + (val2 - val1) / 2;
    }
    if (dqsi_dly < dqsi_dly_per_bit) {
        dbg_print("Critical error in DRAMC calibration. DQS input delay (per bit) > max dqs input delay.\n\r");
    } else {
        b2 += dqsi_dly - dqsi_dly_per_bit;
    }

    b1 = (data >> 8) & 0x000000FF;
    val2 = ((*(volatile unsigned int *)dqs_cal_max) >> 8) & 0x000000FF;
    val1 = ((*(volatile unsigned int *)dqs_cal_min) >> 8) & 0x000000FF;
    if (val1 > val2) {
        dbg_print("Critical error in DRAMC calibration. Min input delay > max input delay.\n\r");
        return -1;
    } else {
        dqsi_dly_per_bit = val1 + (val2 - val1) / 2;
    }
    if (dqsi_dly < dqsi_dly_per_bit) {
        dbg_print("Critical error in DRAMC calibration. DQS input delay (per bit) > max dqs input delay.\n\r");
    } else {
        b1 += dqsi_dly - dqsi_dly_per_bit;
    }

    b0 = (data >> 8) & 0x000000FF;
    val2 = ((*(volatile unsigned int *)dqs_cal_max)) & 0x000000FF;
    val1 = ((*(volatile unsigned int *)dqs_cal_min)) & 0x000000FF;
    if (val1 > val2) {
        dbg_print("Critical error in DRAMC calibration. Min input delay > max input delay.\n\r");
        return -1;
    } else {
        dqsi_dly_per_bit = val1 + (val2 - val1) / 2;
    }
    if (dqsi_dly < dqsi_dly_per_bit) {
        dbg_print("Critical error in DRAMC calibration. DQS input delay (per bit) > max dqs input delay.\n\r");
    } else {
        b0 += dqsi_dly - dqsi_dly_per_bit;
    }

    *(volatile unsigned int *)to = (b3 << 24) | (b2 << 16) | (b1 << 8) | (b0);

    return 0;
}

/*
 * do_hw_dq_dqs_calib: Do HW calibration for DQ/DQS input delay.
 * Return 0 for success. Return negative values for failure.
 */
unsigned int dq_dqs_cal=0;
static int do_hw_dq_dqs_calib(void)
{
    int result, temp;
    unsigned int data, backup;
    int timeout;
    unsigned int dqsi_dly0, dqsi_dly1, dqsi_dly2, dqsi_dly3;

    result = 0;

    /* block all EMI accesses */
    backup = *(volatile unsigned int *)0xC0003060;
    *(volatile unsigned int *)0xC0003060 = 0x00;

    /* turn off CTO protocol */
    data = *(volatile unsigned int *)0xC00041E0;
    data &= ~0x20000000;
    data |= 0x80000000;
    *(volatile unsigned int *)0xC00041E0 = data;

    DDR_PHY_RESET();

    /* enable HW calibration */
    data = *(volatile unsigned int *)DRAMC_TEST2_3;
    data = (data & 0xFFFFF8EF) | ((DEFAULT_DQSSTEP_VALUE & 0x7) << 8) | (1 << 4);
    *(volatile unsigned int *)DRAMC_TEST2_3 = data;
    timeout = HW_DQ_DQS_CALIB_TIMEOUT;
    do {
        data = *(volatile unsigned int *)DRAMC_TESTRPT;
        if (data & (1 << 28)) {
            break;
        }
        timeout--;
    } while (timeout > 0);
    if (timeout == 0) {
        print("Critical error in DRAMC calibration. HW calibration timeout.\n\r");
        result = -1;
        goto calib_out;
    }

    /* set DQS input delay according to the calibration result */
    temp = dqs_input_delay((unsigned int)DRAMC_DQS_CAL_MIN_0, (unsigned int)DRAMC_DQS_CAL_MIN_1);
    if (temp < 0) {
        result = -1;
        goto calib_out;
    } else {
        dqsi_dly0 = temp;
        data = GRAY_ENCODED(temp);
    }
    temp = dqs_input_delay((unsigned int)DRAMC_DQS_CAL_MIN_2, (unsigned int)DRAMC_DQS_CAL_MIN_3);
    if (temp < 0) {
        result = -1;
        goto calib_out;
    } else {
        dqsi_dly1 = temp;
        data |= GRAY_ENCODED(temp) << 8;
    }
    temp = dqs_input_delay((unsigned int)DRAMC_DQS_CAL_MIN_4, (unsigned int)DRAMC_DQS_CAL_MIN_5);
    if (temp < 0) {
        result = -1;
        goto calib_out;
    } else {
        dqsi_dly2 = temp;
        data |= GRAY_ENCODED(temp) << 16;
    }
    temp = dqs_input_delay((unsigned int)DRAMC_DQS_CAL_MIN_6, (unsigned int)DRAMC_DQS_CAL_MIN_7);
    if (temp < 0) {
        result = -1;
        goto calib_out;
    } else {
        dqsi_dly3 = temp;
        data |= GRAY_ENCODED(temp) << 24;
    }
    *(volatile unsigned int *)DRAMC_DELDLY1 = data;

    /* set DQ input delay according to the calibration result */
    if (set_dq_input_delay((unsigned int)DRAMC_DQ_CAL_MAX_0, (unsigned int)DRAMC_DQIDLY1, dqsi_dly0, (unsigned int)DRAMC_DQS_CAL_MIN_0) < 0) {
        result = -1;
        goto calib_out;
    }
    if (set_dq_input_delay((unsigned int)DRAMC_DQ_CAL_MAX_1, (unsigned int)DRAMC_DQIDLY2, dqsi_dly0, (unsigned int)DRAMC_DQS_CAL_MIN_1) < 0) {
        result = -1;
        goto calib_out;
    }
    if (set_dq_input_delay((unsigned int)DRAMC_DQ_CAL_MAX_2, (unsigned int)DRAMC_DQIDLY3, dqsi_dly1, (unsigned int)DRAMC_DQS_CAL_MIN_2) < 0) {
        result = -1;
        goto calib_out;
    }
    if (set_dq_input_delay((unsigned int)DRAMC_DQ_CAL_MAX_3, (unsigned int)DRAMC_DQIDLY4, dqsi_dly1, (unsigned int)DRAMC_DQS_CAL_MIN_3) < 0) {
        result = -1;
        goto calib_out;
    }
    if (set_dq_input_delay((unsigned int)DRAMC_DQ_CAL_MAX_4, (unsigned int)DRAMC_DQIDLY5, dqsi_dly2, (unsigned int)DRAMC_DQS_CAL_MIN_4) < 0) {
        result = -1;
        goto calib_out;
    }
    if (set_dq_input_delay((unsigned int)DRAMC_DQ_CAL_MAX_5, (unsigned int)DRAMC_DQIDLY6, dqsi_dly2, (unsigned int)DRAMC_DQS_CAL_MIN_5) < 0) {
        result = -1;
        goto calib_out;
    }
    if (set_dq_input_delay((unsigned int)DRAMC_DQ_CAL_MAX_6, (unsigned int)DRAMC_DQIDLY7, dqsi_dly3, (unsigned int)DRAMC_DQS_CAL_MIN_6) < 0) {
        result = -1;
        goto calib_out;
    }
    if (set_dq_input_delay((unsigned int)DRAMC_DQ_CAL_MAX_7, (unsigned int)DRAMC_DQIDLY8, dqsi_dly3, (unsigned int)DRAMC_DQS_CAL_MIN_7) < 0) {
        result = -1;
        goto calib_out;
    }

    /* refine the dqs */
 if ((mt_get_dram_type() == 3))
    	{
    	  if(dq_dqs_cal==1)
    	    {
    	     print("*dqsi_dly0 = 0x%x, dqsi_dly1 = 0x%x, dqsi_dly2 = 0x%x, dqsi_dly3 = 0x%x\n", dqsi_dly0,dqsi_dly1,dqsi_dly2,dqsi_dly3);
    	     print("*DDR3_DEC_DQS_VALUE = 0x%x\n", DDR3_DEC_DQS_VALUE);
             data = (dqsi_dly0 > DDR3_DEC_DQS_VALUE ? GRAY_ENCODED(dqsi_dly0 - DDR3_DEC_DQS_VALUE) : GRAY_ENCODED(0));
             data |= (dqsi_dly1 > DDR3_DEC_DQS_VALUE ? GRAY_ENCODED(dqsi_dly1 - DDR3_DEC_DQS_VALUE) : GRAY_ENCODED(0)) << 8; 
             data |= (dqsi_dly2 > DDR3_DEC_DQS_VALUE ? GRAY_ENCODED(dqsi_dly2 - DDR3_DEC_DQS_VALUE) : GRAY_ENCODED(0)) << 16;
             data |= (dqsi_dly3 > DDR3_DEC_DQS_VALUE ? GRAY_ENCODED(dqsi_dly3 - DDR3_DEC_DQS_VALUE) : GRAY_ENCODED(0)) << 24; 
             *(volatile unsigned int *)DRAMC_DELDLY1 = data;
           }
    	}
    else    	
    	{	
    data = (dqsi_dly0 > DEC_DQS_VALUE ? GRAY_ENCODED(dqsi_dly0 - DEC_DQS_VALUE) : GRAY_ENCODED(0));
    data |= (dqsi_dly1 > DEC_DQS_VALUE ? GRAY_ENCODED(dqsi_dly1 - DEC_DQS_VALUE) : GRAY_ENCODED(0)) << 8; 
    data |= (dqsi_dly2 > DEC_DQS_VALUE ? GRAY_ENCODED(dqsi_dly2 - DEC_DQS_VALUE) : GRAY_ENCODED(0)) << 16;
    data |= (dqsi_dly3 > DEC_DQS_VALUE ? GRAY_ENCODED(dqsi_dly3 - DEC_DQS_VALUE) : GRAY_ENCODED(0)) << 24; 
    *(volatile unsigned int *)DRAMC_DELDLY1 = data;
       }

calib_out:
    /* disable HW calibration */
    data = *(volatile unsigned int *)DRAMC_TEST2_3;
    data &= ~(1 << 4);
    *(volatile unsigned int *)DRAMC_TEST2_3 = data;

    DDR_PHY_RESET();

    /* turn on CTO protocol */
    data = *(volatile unsigned int *)0xC00041E0;
    data |= 0x20000000;
    data &= ~0x80000000;
    *(volatile unsigned int *)0xC00041E0 = data;

    /* unblock EMI accesses */
    *(volatile unsigned int *)0xC0003060 = backup;

#if defined(DEBUG_DRAMC_CALIB)
    print("*DQIDLY1 = 0x%x\n", *(volatile unsigned int *)DRAMC_DQIDLY1);
    print("*DQIDLY2 = 0x%x\n", *(volatile unsigned int *)DRAMC_DQIDLY2);
    print("*DQIDLY3 = 0x%x\n", *(volatile unsigned int *)DRAMC_DQIDLY3);
    print("*DQIDLY4 = 0x%x\n", *(volatile unsigned int *)DRAMC_DQIDLY4);
    print("*DQIDLY5 = 0x%x\n", *(volatile unsigned int *)DRAMC_DQIDLY5);
    print("*DQIDLY6 = 0x%x\n", *(volatile unsigned int *)DRAMC_DQIDLY6);
    print("*DQIDLY7 = 0x%x\n", *(volatile unsigned int *)DRAMC_DQIDLY7);
    print("*DQIDLY8 = 0x%x\n", *(volatile unsigned int *)DRAMC_DQIDLY8);
    print("*DRAMC_DELDLY1 = 0x%x\n", *(volatile unsigned int *)DRAMC_DELDLY1);
#endif
    dq_dqs_cal++;
    return result;
}

/*
 * dramc_calib: Do DRAMC calibration.
 * Return error code;
 */
int dramc_calib(void)
{
    int err;
#if defined(DEBUG_DRAMC_CALIB)
    int temp;

    /* enable ARM CA9 PMU */
    asm volatile(
        "MRC p15, 0, %0, c9, c12, 0\n"
        "BIC %0, %0, #1 << 0\n"   /* disable */
        "ORR %0, %0, #1 << 2\n"   /* reset cycle count */
        "BIC %0, %0, #1 << 3\n"   /* count every clock cycle */
        "MCR p15, 0, %0, c9, c12, 0\n"
        : "+r"(temp)
        :
        : "cc"
    );
    asm volatile(
        "MRC p15, 0, %0, c9, c12, 0\n"
        "ORR %0, %0, #1 << 0\n"   /* enable */
        "MCR p15, 0, %0, c9, c12, 0\n"
        "MRC p15, 0, %0, c9, c12, 1\n"
        "ORR %0, %0, #1 << 31\n"
        "MCR p15, 0, %0, c9, c12, 1\n"
        : "+r"(temp)
        :
        : "cc"
    );
#endif

    /* do calibration for DQS gating window (phase 1) */
    err = do_dqs_gw_calib_1();
    if (err < 0) {
        goto dramc_calib_exit;
    }

    /* do HW calibration for DQ/DQS input delay */
    err = do_hw_dq_dqs_calib();
    if (err < 0) {
        goto dramc_calib_exit;
    }

    /* do calibration for DQS gating window (phase 2) */
    err = do_dqs_gw_calib_2();
    if (err < 0) {
        goto dramc_calib_exit;
    }

    /* do HW calibration for DQ/DQS input delay */
    err = do_hw_dq_dqs_calib();
    if (err < 0) {
        goto dramc_calib_exit;
    }

dramc_calib_exit:

#if defined(DEBUG_DRAMC_CALIB)
    /* get CPU cycle count from the ARM CA9 PMU */
    asm volatile(
        "MRC p15, 0, %0, c9, c12, 0\n"
        "BIC %0, %0, #1 << 0\n"   /* disable */
        "MCR p15, 0, %0, c9, c12, 0\n"
        "MRC p15, 0, %0, c9, c13, 0\n"
        : "+r"(temp)
        :
        : "cc"
    );
    print("DRAMC calibration takes %d CPU cycles\n\r", temp);
#endif

    return err;
}

