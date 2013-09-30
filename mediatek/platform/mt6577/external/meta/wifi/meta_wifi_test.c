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
#include "meta_wifi.h"

static void wifi_info_callback(FT_WM_WIFI_CNF *cnf, void *buf, unsigned int size)
{
    unsigned int i;
    char *type[] = { "WIFI_CMD_SET_OID", "WIFI_CMD_QUERY_OID" };
    OID_STRUC *poid;

    printf("[META_WIFI] <CNF> %s, Drv Status: %d, Status: %d\n", type[cnf->type],     
        cnf->drv_status, cnf->status);

    if (buf) {
        poid = (OID_STRUC *)buf;
        printf("META_WIFI] <CNF> OID: %d, data len: %d\n",
            poid->QueryOidPara.oid, poid->QueryOidPara.dataLen);
        for (i = 0; i < poid->QueryOidPara.dataLen; i++) {
            printf("META_WIFI] <CNF> Data[%d] = 0x%x\n",
                i, poid->QueryOidPara.data[i]);            
        }
    }
}

int main(int argc, const char** argv)
{
    FT_WM_WIFI_REQ req;

    memset(&req, 0, sizeof(FT_WM_WIFI_REQ));

    META_WIFI_Register(wifi_info_callback);

    if (META_WIFI_init(&req) == false) {
        printf("WLAN init failed\n");
        return -1;
    }

    #if 0
    req.type = WIFI_CMD_SET_OID;
    META_WIFI_OP(&req, NULL, 0);

    req.type = WIFI_CMD_QUERY_OID;
    META_WIFI_OP(&req, NULL, 0);
    #endif

    META_WIFI_deinit();
    META_WIFI_Register(NULL);
    
    return 0;
}

