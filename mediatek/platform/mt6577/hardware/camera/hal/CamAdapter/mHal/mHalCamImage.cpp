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

/*
**
** Copyright 2008, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#define LOG_TAG "MtkCam/MCamHw"
//
#include <mhal/inc/MediaHal.h>
//
#include "Utils/inc/CamUtils.h"
//
#include "./core/SkBitmap.h"
#include "SkStream.h"
#include "SkImageEncoder.h"


/******************************************************************************
*
*******************************************************************************/
#define MY_LOGV(fmt, arg...)    CAM_LOGV("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)    CAM_LOGD("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)    CAM_LOGI("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)    CAM_LOGW("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)    CAM_LOGE("[%s] "fmt, __FUNCTION__, ##arg)


/******************************************************************************
* convert image format by HW Bitblt 
*******************************************************************************/
bool hwConvertImageFormat(uint8_t *pbufIn, uint32_t inFmt, uint32_t width, uint32_t height, uint8_t *pbufOut, uint32_t outFmt)
{
    int32_t status = 0;

    // It's hw convert, the buffer address must be physical address
    MY_LOGV("[converImgFmt] pbufIn: 0x%x, inFmt = 0x%d, width: %d, height: %d , pbufOut: 0x%x, outFmt = %d\n",
                          (uint32_t) pbufIn, inFmt, width, height, (uint32_t) pbufOut, outFmt);

    mHalBltParam_t bltParam;
    memset(&bltParam, 0, sizeof(bltParam));
    bltParam.srcAddr   = (MUINT32)(pbufIn);
    bltParam.srcX      = 0;
    bltParam.srcY      = 0;
    bltParam.srcW      = width;
    bltParam.srcWStride= width;
    bltParam.srcH      = height;
    bltParam.srcHStride= height;
    bltParam.srcFormat = inFmt;
    bltParam.dstAddr   = (MUINT32)(pbufOut);
    bltParam.dstW      = width;
    bltParam.dstH      = height;
    bltParam.dstFormat = outFmt;
    bltParam.pitch     = width;
    bltParam.orientation = MHAL_BITBLT_ROT_0;

    status = mHalIoCtrl(MHAL_IOCTL_BITBLT, &bltParam, sizeof(bltParam), NULL, 0, NULL);
    if (0 != status) {
        MY_LOGE("[converImgFmt] err: %d, can't do bitblt operation \n", status);
    }
    return (status != 0) ? true : false;
}


/******************************************************************************
*   convert rgb565 to jpeg file 
*******************************************************************************/
bool rgb565toJpeg(uint8_t *pbufIn, uint8_t *pbufOut, uint32_t width, uint32_t height, uint32_t *psize)
{
    uint32_t size = 0;

    // It's sw encode, the buffer address must be virtual address
    CAM_LOGD("[rgb565toJpeg] pbufIn: 0x%x, pbufOut: 0x%x, width: %d, height: %d \n", (uint32_t) pbufIn, (uint32_t) pbufOut, width, height);

    SkImageEncoder::Type fm =  SkImageEncoder::kJPEG_Type;
    bool success = false;
    SkImageEncoder *pencoder = SkImageEncoder::Create(fm);
    if (NULL != pencoder) {
        SkMemoryWStream *pskMemStream = new SkMemoryWStream(pbufOut, width * height * 2);
        if (NULL != pskMemStream) {
            SkBitmap *pbitmap = new SkBitmap;
            if (NULL != pbitmap) {
                pbitmap->setConfig(SkBitmap::kRGB_565_Config, width, height);
                pbitmap->setPixels(pbufIn);
                success = pencoder->encodeStream(pskMemStream, *pbitmap, 85);
                if (success) {
                    size = pskMemStream->bytesWritten();
                }
            }
            delete pbitmap;
        }
        delete pskMemStream;
        delete pencoder;
    }

    *psize = size;
    CAM_LOGD(" size: %d \n", size);
    return success;
}

