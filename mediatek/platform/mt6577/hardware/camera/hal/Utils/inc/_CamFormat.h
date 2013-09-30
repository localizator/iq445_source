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

#ifndef _MTK_UTILS_INC__CAM_FORMAT_H_
#define _MTK_UTILS_INC__CAM_FORMAT_H_
//
#include <utils/String8.h>


namespace android {
namespace MtkCamUtils {
/******************************************************************************
*
*******************************************************************************/


/******************************************************************************
*   Map: CameraParameters format to HAL_PIXEL_FORMAT_xxx
*******************************************************************************/
class CamFmtMap
{
public:     ////    Interfaces.
    //
    typedef String8         KEY;    //  CameraParameters format
    typedef int32_t         VALUE;  //  HAL_PIXEL_FORMAT_xxx
    //
    static  CamFmtMap&      inst();
    //
    virtual                 ~CamFmtMap() {}
    //
    //  map CameraParameters format to Hal pixel format.
    virtual VALUE           valueFor(KEY const& key) const      = 0;
    //
    //  map Hal pixel format to CameraParameters format.
    virtual KEY const&      keyFor(VALUE const& value) const    = 0;
    //
};


/******************************************************************************
*   map CameraParameters format to Hal pixel format.
*******************************************************************************/
int mapCameraToPixelFormat(String8 const& rs8CameraFormat);


/******************************************************************************
*   query bits per pixel for a given Hal pixel format.
*******************************************************************************/
int queryBitsPerPixel(int const HalPixelFormat);


/******************************************************************************
*   convert Mtk YUV to Yuv 420sp
*******************************************************************************/
bool convertMtkYuvToYuv420sp(uint8_t *pbufIn, uint8_t *pbufOut, uint32_t width, uint32_t height);


/******************************************************************************
*   convert i420 Yuv to Yuv 420sp (NV21)
*******************************************************************************/
bool convertYuv420iToYuv420sp(uint8_t *pbufIn, uint8_t *pbufOut, uint32_t width, uint32_t height);


/******************************************************************************
*   convert i420 Yuv to Yuv 420p (YV12)
*******************************************************************************/
bool convertYuv420iToYuv420p(uint8_t *pbufIn, uint8_t *pbufOut, uint32_t width, uint32_t height);


};  // namespace MtkCamUtils
};  // namespace android
#endif  //_MTK_UTILS_INC__CAM_FORMAT_H_

