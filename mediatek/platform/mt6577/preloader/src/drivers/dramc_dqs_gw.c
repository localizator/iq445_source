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

#include <stdlib.h>
#include <typedefs.h>
#include <mt6577.h>
#include <dramc.h>
#include <emi_hw.h>

unsigned int score, high_score;

#define RANK_SIZE 0x20000000 /*4Gb DRAM*/

int ett_recursive_factor_tuning(unsigned int n, tuning_factor *tuning_factors, print_callbacks *cbs) 
{
    unsigned int i;
    int result;

    //Print banner.
    cbs->ett_print_banner(n);

    if (n == 0) {
        //Before starting the most inner loop, print something.
        cbs->ett_print_before_start_loop_zero();
    }

    for ( i = 0 ; tuning_factors[n].factor_tbl[i] != NULL ; i++) {
        tuning_factors[n].factor_handler(tuning_factors[n].factor_tbl[i]);
        tuning_factors[n].curr_val = tuning_factors[n].factor_tbl[i];
        if (n == 0) {//The most inner loop
            //Before each round of the most inner loop, print something.
            cbs->ett_print_before_each_round_of_loop_zero();
            score += cbs->ett_print_result();
            //After each round of the most inner loop, print something.
            cbs->ett_print_after_each_round_of_loop_zero();
        } else {//Other loops. Call this function recursively.
            ett_recursive_factor_tuning(n-1, tuning_factors, cbs);
        }
    }

    cbs->ett_calc_opt_value(n, &score, &high_score);

    if (n == 0) {
        //After finishing the most inner loop, print something.
        cbs->ett_print_after_finish_loop_zero();
    }
}

/* DQS gating window (coarse) */
char *dqsi_gw_dly_coarse_tbl[] =
{
    "1", "2", "4", "8",
    "16", "32", "64", "128",
    "256", "512", "1024", "2048",
    NULL,
};

void dqsi_gw_dly_coarse_factor_handler(char *factor_value) 
{
    int curr_val = atoi(factor_value);

    WRITE_REG((READ_REG(DRAMC_DQSCTL0/* 0xDC */) & 0xFF000000)    /* Reserve original values for DRAMC_DQSCTL0[24:31] */
        | ((curr_val & 0xFFF) << 0)            /* DQS0CTL: DRAMC_DQSCTL0[0:11],        12 bits */
        | ((curr_val & 0xFFF) << 12),            /* DQS1CTL: DRAMC_DQSCTL0[12:23],       12 bits */
        DRAMC_DQSCTL0/* 0xDC */);

    WRITE_REG((READ_REG(DRAMC_DQSCTL1/* 0xE0 */) & 0xFF000000)    /* Reserve original values for DRAMC_DQSCTL1[24:31] */
        | ((curr_val & 0xFFF) << 0)            /* DQS2CTL: DRAMC_DQSCTL1[0:11],        12 bits */
        | ((curr_val & 0xFFF) << 12),            /* DQS3CTL: DRAMC_DQSCTL1[12:23],       12 bits */
        DRAMC_DQSCTL1/* 0xE0 */);
}

/* DQS gating window (fine) */
char *dqsi_gw_dly_fine_tbl[] =
{
    "0",/* "1", "2", "3", "4", "5", "6", "7",*/ "8",/* "9", "10", "11", "12", "13", "14", "15",*/
    "16",/* "17", "18", "19", "20", "21", "22", "23",*/ "24",/* "25", "26", "27", "28", "29", "30", "31",*/
    "32",/* "33", "34", "35", "36", "37", "38", "39",*/ "40",/* "41", "42", "43", "44", "45", "46", "47",*/
    "48",/* "49", "50", "51", "52", "53", "54", "55",*/ "56",/* "57", "58", "59", "60", "61", "62", "63",*/
    "64",/* "65", "66", "67", "68", "69", "70", "71",*/ "72", /* "73", "74", "75", "76", "77", "78", "79",*/
    "80",/* "81", "82", "83", "84", "85", "86", "87",*/ "88",/* "89", "90", "91", "92", "93", "94", "95",*/
    "96",/* "97", "98", "99", "100", "101", "102", "103",*/ "104",/* "105", "106", "107", "108", "109", "110", "111",*/
    "112",/* "113", "114", "115", "116", "117", "118", "119",*/ "120",/* "121", "122", "123", "124", "125", "126", "127",*/
    NULL,
};

void dqsi_gw_dly_fine_factor_handler(char *factor_value) 
{
    int curr_val = atoi(factor_value);

    WRITE_REG((READ_REG(DRAMC_DQSIEN/* 0x94 */) & 0x80000000)  /* Reserve original values for DRAMC_DQSIEN[31] */
        | ((curr_val & 0x7F) << 0)        /* DQS0IEN: DRAMC_DQSIEN[0:6],   7 bits */
        | ((curr_val & 0x7F) << 8)    /* DQS1IEN: DRAMC_DQSIEN[8:14],  7 bits */
        | ((curr_val & 0x7F) << 16)    /* DQS2IEN: DRAMC_DQSIEN[16:22], 7 bits */
        | ((curr_val & 0x7F) << 24),    /* DQS3IEN: DRAMC_DQSIEN[24:30], 7 bits */
        DRAMC_DQSIEN/* 0x94 */);
}

struct dqs_gw_pass_win
{
    int coarse_end;
    int fine_end;
    int size;   /* gating window's size in this range */
};

void Sequence_Read(unsigned int start, unsigned int len, void *ext_arg)
{
     int i;
    volatile int rval;	
        /* DQS gating window counter reset */
    *(volatile unsigned int *)DRAMC_SPCMD |= (1 << 9);
    *(volatile unsigned int *)DRAMC_SPCMD &= ~(1 << 9);
     DDR_PHY_RESET();
     for (i = 0 ; i < len ; i++) {
        rval = *(volatile unsigned int *)(start);
    }

}
int Read_Test(unsigned int start, unsigned int len, void *ext_arg)
{
       int err = 0;
       Sequence_Read(start,len,ext_arg);
        if (READ_REG(DRAMC_OFFDLY6/* 0x3C0 */) == 0x14141414) {
            Sequence_Read(start,len,ext_arg);
           if (READ_REG(DRAMC_OFFDLY6/* 0x3C0 */) == 0x14141414) {
                err = 0;
            } else {
               err = -1;
            }
        } else {
               err = -1;
        }

    return err;
}

#define DQS_GW_COARSE_MAX 12
#define DQS_GW_FINE_MAX 16
#define DQS_GW_FINE_CHK_RANGE 4
static const int HW_DQS_GW_COUNTER = 0;
static unsigned int dqs_gw[DQS_GW_COARSE_MAX];
static int dqs_gw_coarse, dqs_gw_fine;
static struct dqs_gw_pass_win cur_pwin, max_pwin;

/*
 * dramc_ta2: Run DRAMC test agent 2.
 * @start: test start address
 * @len: test length
 * @ext_arg: extend argument (0: don't check read/write results; 1: check)
 * Return error code.
 */
extern unsigned int ddr_type; 
int dramc_ta2(unsigned int start, unsigned int len, void *ext_arg)
{
    int err = 0;
    int check_result = (int)ext_arg;
     /* cpu read test */
 
   if ((check_result == 0) && (ddr_type == 3))  // for DDR3 only
   	{
	    err = Read_Test(start, len, ext_arg);
	    *(volatile unsigned int *)DRAMC_SPCMD |= (1 << 9);
           *(volatile unsigned int *)DRAMC_SPCMD &= ~(1 << 9);
           DDR_PHY_RESET();	
	    if(err !=0)		
    		{
	         return err;
    		}
   	}


    *(volatile unsigned int *)DRAMC_LPDDR2 &= ~0x20000000;
    *(volatile unsigned int *)DRAMC_LPDDR2 |= 0x80000000;

    *(volatile unsigned int *)DRAMC_CONF2 |= (1 << 30) | (1 << 31);

    while (!((*(volatile unsigned int *)DRAMC_TESTRPT) & (1 << 10)));

    /* 
     * NoteXXX: Need to wait for at least 400 ns 
     *          After checking the simulation result, 
     *          there will be a delay on DLE_CNT_OK/DM_CMP_ERR updates after getting DM_CMP_CPT.
     *          i.e; After getting the complete status, need to wait for a while before reading DLE_CNT_OK/DM_CMP_ERR in the TESTRPT register.
     */
    delay_a_while(400);

    if (check_result) {
        if (*(volatile unsigned int *)DRAMC_TESTRPT & (1 << 14)) {
            err = -1;
        } else if (!(*(volatile unsigned int *)DRAMC_TESTRPT & (1 << 18))) {
            err = -1;
        }
    }

    *(volatile unsigned int *)DRAMC_CONF2 &= ~((1 << 30) | (1 << 31));

    *(volatile unsigned int *)DRAMC_LPDDR2 &= ~0x80000000;
    *(volatile unsigned int *)DRAMC_LPDDR2 |= 0x20000000;

    DDR_PHY_RESET();

    if (!err) {
        if ((*(volatile unsigned int *)DRAMC_OFFDLY6 == HW_DQS_GW_COUNTER)
            && (*(volatile unsigned int *)DRAMC_OFFDLY7 == HW_DQS_GW_COUNTER)) {
            err = 0;
        } else {
            err = -1;
        }
    }

    /* DQS gating window counter reset */
    *(volatile unsigned int *)DRAMC_SPCMD |= (1 << 9);
    *(volatile unsigned int *)DRAMC_SPCMD &= ~(1 << 9);

    DDR_PHY_RESET();

    if (!err) {
        if ((*(volatile unsigned int *)DRAMC_OFFDLY6 == HW_DQS_GW_COUNTER)
            && (*(volatile unsigned int *)DRAMC_OFFDLY7 == HW_DQS_GW_COUNTER)) {
            err = 0;
        } else {
            err = -1;
        }
    }

dramc_ta2_exit:
    return err;
}

tuning_factor dqs_gw_tuning_factors[] =
{
    {
        .name = "DQS Gating Window Delay (Fine Scale)",
        .factor_tbl = dqsi_gw_dly_fine_tbl,
        .curr_val = NULL,
        .opt_val = NULL,
        .factor_handler = dqsi_gw_dly_fine_factor_handler,
    },
    {
        .name = "DQS Gating Window Delay (Coarse Scale)",
        .factor_tbl = dqsi_gw_dly_coarse_tbl,
        .curr_val = NULL,
        .opt_val = NULL,
        .factor_handler = dqsi_gw_dly_coarse_factor_handler,
    },
};

test_case dqs_gw_test_cases_1[] = 
{
    {
        .test_case = dramc_ta2, 
        .start = 0x0, 
        .range = 0xA, 
        .ext_arg = (void *)0,
    },
};

test_case dqs_gw_test_cases_2[] = 
{
    {
        .test_case = dramc_ta2, 
        .start = 0x0, 
        .range = 0xA, 
        .ext_arg = (void *)1,
    },
};

/* For 6Gb DRAM*/
test_case dqs_gw_test_cases_2_2ranks[] = 
{
    {
        .test_case = dramc_ta2,
        .start = 0x0,
        .range = 0xA,
        .ext_arg = (void *)1,
    },
    {
        .test_case = dramc_ta2,
        .start = RANK_SIZE,
        .range = 0xA,
        .ext_arg = (void *)1,
    },
};

void ett_print_dqs_gw_banner(unsigned int n)
{
    if (n == 1) {
#if 0
        unsigned int i;
        print("=============================================\n");
        for ( i = 2 ; i < ETT_TUNING_FACTOR_NUMS(dqs_gw_tuning_factors) ; i++) {
            print("%s = %d\n", dqs_gw_tuning_factors[i].name, atoi(dqs_gw_tuning_factors[i].curr_val));
        }
        print("X-axis: %s\n", dqs_gw_tuning_factors[0].name);
        if (ETT_TUNING_FACTOR_NUMS(dqs_gw_tuning_factors) > 1) {
            print("Y-axis: %s\n", dqs_gw_tuning_factors[1].name);
        }
        print("=============================================\n");
#endif
        print("          0    8   16   24   32   40   48   56   64   72   80   88   96  104  112  120\n");
        print("      --------------------------------------------------------------------------------\n");
    }
}

void ett_print_dqs_gw_before_start_loop_zero(void) 
{
    print("%H:|", atoi(dqs_gw_tuning_factors[1].curr_val));
}

void ett_print_dqs_gw_before_each_round_of_loop_zero(void) 
{
    /* DQS gating window counter reset */
    *(volatile unsigned int *)DRAMC_SPCMD |= (1 << 9);
    *(volatile unsigned int *)DRAMC_SPCMD &= ~(1 << 9);

    DDR_PHY_RESET();
}

static unsigned int __ett_print_dqs_gw_result(test_case *test_cases, int nr_ts)
{
    unsigned int i, score = 1;

    for (i = 0; i < nr_ts; i++) {
        if (test_cases[i].test_case(test_cases[i].start, test_cases[i].range, test_cases[i].ext_arg) < 0) {
            print("    0");
            score = 0;
            break;
        }
        /* Swap CS0->CS1. */
        *(volatile unsigned int*)DRAMC_RKCFG = 0x0805114B;
        *(volatile unsigned int *)DRAMC_TEST0_1 |= (0xF << 20);
        *(volatile unsigned int *)DRAMC_TEST1_1 |= (0xF << 20);
        *(volatile unsigned int *)DRAMC_TEST2_1 |= (0xF << 20);
    }

     /* Swap CS0<-CS1. */
    *(volatile unsigned int *)DRAMC_RKCFG = 0x08051143;
    *(volatile unsigned int *)DRAMC_TEST0_1 &= ~(0xF << 20);
    *(volatile unsigned int *)DRAMC_TEST1_1 &= ~(0xF << 20);
    *(volatile unsigned int *)DRAMC_TEST2_1 &= ~(0xF << 20);

    /*
     * DQS GW calibration rule 1: Identify a pass-window with the max gw.
     */
    if (score != 0) {
        cur_pwin.size++;
    } else if (cur_pwin.size != 0) {
        /* end of the pass-window */
        if (dqs_gw_fine) {
            cur_pwin.coarse_end = dqs_gw_coarse;
            cur_pwin.fine_end = dqs_gw_fine - 1;
        } else {
            if (dqs_gw_coarse == 0) {
                /* never happen */ 
                print("Critical error! dqs_gw_coarse = 0 but dqs_gw_fine = 0!\n");
            }
            cur_pwin.coarse_end = dqs_gw_coarse - 1;
            cur_pwin.fine_end = DQS_GW_FINE_MAX - 1;
        }

        /* update the max pass-window */
        if (cur_pwin.size > max_pwin.size) {
            memcpy((void *)&max_pwin, (void *)&cur_pwin, sizeof(struct dqs_gw_pass_win));
        }

        memset((void *)&cur_pwin, 0, sizeof(struct dqs_gw_pass_win));
    }

    if (score != 0) {
        print("    1");
        dqs_gw[dqs_gw_coarse] |= (1 << dqs_gw_fine);
    }

    dqs_gw_fine++;
    if (dqs_gw_fine >= DQS_GW_FINE_MAX) {
        dqs_gw_coarse++;
        dqs_gw_fine &= (DQS_GW_FINE_MAX - 1);
    }
    if (dqs_gw_coarse > DQS_GW_COARSE_MAX) {
        print("Critical error!! dqs_gw_coarse > DQS_GW_COARSE_MAX\n");
    }

    return score;
}

static unsigned int ett_print_dqs_gw_result_1(void)
{
   return __ett_print_dqs_gw_result(dqs_gw_test_cases_1, ETT_TEST_CASE_NUMS(dqs_gw_test_cases_1)); 
}

static unsigned int ett_print_dqs_gw_result_2(void)
{
   /*tmp[30] = 1, if DRAM size == 6Gb && DRAM type == 1*/
   /*tmp[29] = 1, if DRAM size == 6Gb && DRAM type == 2*/
   unsigned int tmp = (*(volatile unsigned int*) DRAMC_CKPH0) & (3 << 29); 
   if(!tmp) {
        return __ett_print_dqs_gw_result(dqs_gw_test_cases_2, ETT_TEST_CASE_NUMS(dqs_gw_test_cases_2)); 
    }else {
        return __ett_print_dqs_gw_result(dqs_gw_test_cases_2_2ranks, ETT_TEST_CASE_NUMS(dqs_gw_test_cases_2_2ranks));
    }
}

void ett_print_dqs_gw_after_each_round_of_loop_zero(void) 
{
}

void ett_calc_dqs_gw_opt_value(unsigned int n, unsigned int *score, unsigned int *high_score)
{
}

void ett_print_dqs_gw_after_finish_loop_n(unsigned int n) 
{
    if (n == 0) {
        print("\n");
    }
}

void ett_print_dqs_gw_fine_after_finish_loop_zero(void)
{
    print("\n");
}

/*
 * nr_bit_set: Get the number of bits set in the given value.
 * @val: the gieven value
 * Return the number of bits set.
 */
static int nr_bit_set(unsigned int val)
{
    int i, cnt;

    for (cnt = 0, i = 0; i < (8 * sizeof(unsigned int)); i++) {
        if (val & (1 << i)) {
            cnt++;
        }
    }

    return cnt;
}

/*
 * first_bit_set: Get the first bit set in the given value.
 * @val: the gieven value
 * Return the first bit set.
 */
static int first_bit_set(unsigned int val)
{
    int i;

    for (i = 0; i < (8 * sizeof(unsigned int)); i++) {
        if (val & (1 << i)) {
            return i;
        }
    }

    return -1;
}

/*
 * __do_dqs_gw_calib: do DQS gating window calibration.
 * @cbs: pointer to the print_callbacks structure.
 * Return error code.
 */
static int __do_dqs_gw_calib(print_callbacks *cbs)
{
    int err;
    int i, c, f, cnt, max;

    err = -1;

    dqs_gw_coarse = 0;
    dqs_gw_fine = 0;
    for (i = 0; i < DQS_GW_COARSE_MAX; i++) {
        dqs_gw[i] = 0;
    }
    memset((void *)&cur_pwin, 0, sizeof(struct dqs_gw_pass_win));
    memset((void *)&max_pwin, 0, sizeof(struct dqs_gw_pass_win));

    /* enable DQS gating window counter */
    *(volatile unsigned int *)DRAMC_SPCMD |= (1 << 8);

    if (ETT_TUNING_FACTOR_NUMS(dqs_gw_tuning_factors) > 0) {
        ett_recursive_factor_tuning(ETT_TUNING_FACTOR_NUMS(dqs_gw_tuning_factors)-1, dqs_gw_tuning_factors, cbs);
    }

    if (max_pwin.size > 0) {
        /*
         * DQS GW calibration rule 2: From the lastest pass-index, 
         *                            decrease 0.5T (i.e; coarse value - 1),
         *                            check continuous 4 passed window.
         */
#if 0
        print("max pass-window: coarse_end = %d, fine_end = %d\n", max_pwin.coarse_end, max_pwin.fine_end);
#endif
        c = max_pwin.coarse_end;
        for (cnt = 0, f = max_pwin.fine_end; cnt < DQS_GW_FINE_CHK_RANGE && f >= 0; cnt++, f--) {
            if (!(dqs_gw[c - 1] & (1 << f))) {
                break;
            }
#if 0
            print("dqs_gw[%d][%d] = 1'b1\n", c - 1, f);
#endif
            if (f == 0) {
                if (c == 0) {
                    break;
                } else {
                    f = DQS_GW_FINE_MAX - 1;
                    c--;
                }
            }
        }

        if (cnt == DQS_GW_FINE_CHK_RANGE) {
            print("coarse = %s\n", dqsi_gw_dly_coarse_tbl[max_pwin.coarse_end - 1]);
            print("fine = %s\n", dqsi_gw_dly_fine_tbl[max_pwin.fine_end]);
            dqsi_gw_dly_coarse_factor_handler(dqsi_gw_dly_coarse_tbl[max_pwin.coarse_end - 1]);
            dqsi_gw_dly_fine_factor_handler(dqsi_gw_dly_fine_tbl[max_pwin.fine_end]);
            err = 0;
            goto __do_dqs_gw_calib_exit;
        }

        /*
         * DQS GW calibration rule 3: Select a coarse value with the max passed window.
         *                            Select a fine value from the middle of the passed window.
         */
        c = 0;
        for (i = 0; i < DQS_GW_COARSE_MAX; i++) {
            cnt = nr_bit_set(dqs_gw[i]);
#if 0
            print("nr_bit_set(dqs_gw[%d]) = %d\n", i, cnt);
#endif
            if (cnt >= max) {
                max = cnt;
                c = i;
            }
        }
        cnt = nr_bit_set(dqs_gw[c]);
        if (cnt) {
            f = first_bit_set(dqs_gw[c]) + cnt / 2;
            print("coarse = %s\n", dqsi_gw_dly_coarse_tbl[c]);
            print("fine = %s\n", dqsi_gw_dly_fine_tbl[f]);
            dqsi_gw_dly_coarse_factor_handler(dqsi_gw_dly_coarse_tbl[c]);
            dqsi_gw_dly_fine_factor_handler(dqsi_gw_dly_fine_tbl[f]);
            err = 0;
        }
    } else {
        print("Cannot find any pass-window\n");
    }

__do_dqs_gw_calib_exit:
    return err;
}

/*
 * do_dqs_gw_calib_1: do DQS gating window calibration (phase 1).
 * Return error code.
 */
int do_dqs_gw_calib_1(void)
{
    print_callbacks cbs = {
        .ett_print_banner = ett_print_dqs_gw_banner,
        .ett_print_before_start_loop_zero = ett_print_dqs_gw_before_start_loop_zero,
        .ett_print_before_each_round_of_loop_zero = ett_print_dqs_gw_before_each_round_of_loop_zero,
        .ett_print_result = ett_print_dqs_gw_result_1,
        .ett_print_after_each_round_of_loop_zero = ett_print_dqs_gw_after_each_round_of_loop_zero,
        .ett_calc_opt_value = ett_calc_dqs_gw_opt_value,
        .ett_print_after_finish_loop_zero = ett_print_dqs_gw_fine_after_finish_loop_zero,
    };

    return __do_dqs_gw_calib(&cbs);
}

/*
 * do_dqs_gw_calib_2: do DQS gating window calibration (phase 2).
 * Return error code.
 */
int do_dqs_gw_calib_2(void)
{
    print_callbacks cbs = {
        .ett_print_banner = ett_print_dqs_gw_banner,
        .ett_print_before_start_loop_zero = ett_print_dqs_gw_before_start_loop_zero,
        .ett_print_before_each_round_of_loop_zero = ett_print_dqs_gw_before_each_round_of_loop_zero,
        .ett_print_result = ett_print_dqs_gw_result_2,
        .ett_print_after_each_round_of_loop_zero = ett_print_dqs_gw_after_each_round_of_loop_zero,
        .ett_calc_opt_value = ett_calc_dqs_gw_opt_value,
        .ett_print_after_finish_loop_zero = ett_print_dqs_gw_fine_after_finish_loop_zero,
    };

    return __do_dqs_gw_calib(&cbs);
}
