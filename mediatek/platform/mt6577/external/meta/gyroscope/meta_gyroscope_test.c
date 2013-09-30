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

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include "meta_gyroscope.h"
#include "meta_gyroscope_para.h"
#include "libhwm.h"
#define TAG "[GYROTST] "
#define NULL_FT {0,0}
#define GET_ERR(X) ((X == 0) ? ("OKAY") : ("FAIL"))

static void gyro_cnf_handler(GYRO_CNF *cnf)
{
    printf("-------------------------------------------------------------------\n");
    if (cnf->op == GYRO_OP_CALI) {
        printf("OP_CALI: %d %d %s\n", cnf->gyro_err, cnf->status, GET_ERR(cnf->status));
        printf("(%+6d, %+6d, %+6d) => (%+9.4f, %+9.4f, %+9.4f)\n", 
                cnf->ack.cali.x, cnf->ack.cali.y, cnf->ack.cali.z,
                LSB_TO_GRA(cnf->ack.cali.x), LSB_TO_GRA(cnf->ack.cali.y), LSB_TO_GRA(cnf->ack.cali.z));
    } else if (cnf->op == GYRO_OP_READ_RAW) {
        printf("OP_READ_RAW: %d %d %s\n", cnf->gyro_err, cnf->status, GET_ERR(cnf->status));
        printf("(%+6d, %+6d, %+6d) => (%+9.4f, %+9.4f, %+9.4f)\n", 
                cnf->ack.readraw.x, cnf->ack.cali.y, cnf->ack.cali.z,
                LSB_TO_GRA(cnf->ack.readraw.x), LSB_TO_GRA(cnf->ack.cali.y), LSB_TO_GRA(cnf->ack.cali.z));    
    } else if (cnf->op == GYRO_OP_WRITE_NVRAM) {
        printf("GYRO_OP_WRITE_NVRAM: %d %d %s\n", cnf->gyro_err, cnf->status, GET_ERR(cnf->status));
    } else if (cnf->op == GYRO_OP_READ_NVRAM) {
        printf("OP_READ_NVRAM: %d %d %s\n", cnf->gyro_err, cnf->status, GET_ERR(cnf->status));
        printf("(%+6d, %+6d, %+6d) => (%+9.4f, %+9.4f, %+9.4f)\n", 
                cnf->ack.readnv.x, cnf->ack.readnv.y, cnf->ack.readnv.z,
                LSB_TO_GRA(cnf->ack.readnv.x), LSB_TO_GRA(cnf->ack.readnv.y), LSB_TO_GRA(cnf->ack.readnv.z));        
    }
}

int main(int argc, const char** argv)
{
    int id, idx = 0;
    GYRO_REQ req;

    printf("[VERSION] %s %s\n", __DATE__, __TIME__);
    for (idx = 0; idx < argc; idx++)
        printf("[%2d] %s\n", idx, argv[idx]);

    memset(&req, 0, sizeof(GYRO_REQ));

    if (argc >= 2) {
        id = atoi(argv[1]);
        id = (id < GYRO_OP_END) ? (id) : (GYRO_OP_END);
    } else {
        printf("Usage: %s [ID] [ARGYRO..]\n", argv[0]);
        printf("       %s 0 [DELAY] [NUM] [TOLERANCE]: calibration\n",argv[0]);
        printf("       %s 1 [NUM]: read raw data\n",argv[0]);
        printf("       %s 2 [X] [Y] [Z]: write nvram\n",argv[0]);
        printf("       %s 3 [X] [Y] [Z]: read  nvram\n",argv[0]);
        return 0;
    }

    printf("register: %p\n", gyro_cnf_handler);
    Meta_Gyroscope_Register(gyro_cnf_handler);
    if (Meta_Gyroscope_Open() == false) {
        printf("[GYRO] open failed\n");
        goto exit;
    }
    printf("main handler\n");
    if ((id == GYRO_OP_CALI) || (id == GYRO_OP_END)) {
        req.cmd.cali.delay = 50;
        req.cmd.cali.num = 20;
        req.cmd.cali.tolerance = 20;
        req.op = GYRO_OP_CALI;
        if (argc >= 3)
            req.cmd.cali.delay = atoi(argv[2]);
        if (argc >= 4)
            req.cmd.cali.num   = atoi(argv[3]);
        if (argc >= 5)
            req.cmd.cali.tolerance = atoi(argv[4]);
        
        Meta_Gyroscope_OP(&req, NULL, 0);
    }
    if ((id == GYRO_OP_READ_RAW) || (id == GYRO_OP_END)) {
        int idx, num = 1;
        req.cmd.readraw.dummy = 0;
        req.op = GYRO_OP_READ_RAW;
        if (argc >= 3)
            num = atoi(argv[2]);
        for (idx = 0; idx < num; idx++)        
            Meta_Gyroscope_OP(&req, NULL, 0);
    }
    if ((id == GYRO_OP_WRITE_NVRAM) || (id == GYRO_OP_END)) {
        req.op = GYRO_OP_WRITE_NVRAM;
        if (argc >= 5) {
            req.cmd.writenv.x = atoi(argv[2]);
            req.cmd.writenv.y = atoi(argv[3]);
            req.cmd.writenv.z = atoi(argv[4]);            
        }
        Meta_Gyroscope_OP(&req, NULL, 0);
    } 
    if ((id == GYRO_OP_READ_NVRAM) || (id == GYRO_OP_END)) {
        req.op = GYRO_OP_READ_NVRAM;
        Meta_Gyroscope_OP(&req, NULL, 0);
    } 
    
    if (Meta_Gyroscope_Close() == false) {
        printf("[GYRO] close failed\n");
        goto exit;
    }

exit:
    Meta_Gyroscope_Register(NULL);
    return 0;
}
