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


#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "meta_sdcard.h"

static void sdcard_info_callback(SDCARD_CNF *cnf)
{
    char *speed_class[] = {"Class 0", "Class 2", "Class 4", "Class 6"};
    char *au_size[] = {"Not Defined", "16KB", "32KB", "64KB", "128KB", "256KB", 
        "512KB", "1MB", "2MB", "4MB"};
    char *card_type[] = {"Regular SD RD/WR card", "SD ROM card"};

    if (cnf->status != META_SUCCESS)
        return;

    printf("\n");
    printf("BusWidth: %d (0: 1-bit, 2: 4-bits)\n", cnf->DatBusWidth);
    printf("CardType: %d (%s)\n", cnf->CardTpye, card_type[cnf->CardTpye & 0x1]);
    printf("SpdClass: %d (%s)\n", cnf->SpeedClass, 
        cnf->SpeedClass > 3 ? "Reserved" : speed_class[cnf->SpeedClass]);
    printf("AU Size : %d (%s)\n", cnf->AUSize, cnf->AUSize > 9 ? "Reserved" :
        au_size[cnf->AUSize]);
    printf("PerfMove: %d (%d MB/sec)\n", cnf->Performance_move, cnf->Performance_move);
    printf("ProtSize: 0x%.8x\n", cnf->SizeOfProtectErea);
    printf("SecMode : %s mode\n", cnf->IsSecuredMode ? "Secure" : "Non-Secure");    
}

int main(int argc, const char** argv)
{
    int id = 0;
    SDCARD_REQ req;

    memset(&req, 0, sizeof(SDCARD_REQ));

    id = atoi(argv[0]);
    id = id < MIN_SDCARD_IDX ? MIN_SDCARD_IDX : id;
    req.dwSDHCIndex = id > MAX_SDCARD_IDX ? MAX_SDCARD_IDX : id;

    Meta_SDcard_Register(sdcard_info_callback);

    if (Meta_SDcard_Init(&req) == false) {
        printf("[SD%d] init failed\n", id);
        goto exit;
    }

    Meta_SDcard_OP(&req, NULL, 0);

    if (Meta_SDcard_Deinit() == false) {
        printf("[SD%d] deinit failed\n", id);
        goto exit;
    }

exit:
    Meta_SDcard_Register(NULL);
    
    return 0;
}

