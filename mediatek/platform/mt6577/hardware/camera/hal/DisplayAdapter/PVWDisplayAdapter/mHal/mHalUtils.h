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

#ifndef _MTK_DISPLAY_ADAPTER_MHAL_MHAL_UTILS_H_
#define _MTK_DISPLAY_ADAPTER_MHAL_MHAL_UTILS_H_
//
#include <mhal/inc/MediaHal.h>


/******************************************************************************
*
*******************************************************************************/
namespace android {
/******************************************************************************
*
*******************************************************************************/

/******************************************************************************
*
*******************************************************************************/
static
inline
bool
hwBitblt(
    int const       iDstStride, 
    void*const      pDstBuf, 
    void*const      pSrcBuf, 
    uint32_t const  width, 
    uint32_t const  height, 
    int32_t const   mDstHalBitbltFormat, 
    int32_t const   mSrcHalBitbltFormat
)
{
    mHalBltParam_t  bltParam;
    ::memset(&bltParam, 0, sizeof(bltParam));
    //
    bltParam.srcAddr    = reinterpret_cast<uint32_t>(pSrcBuf);
    bltParam.srcX       = 0;
    bltParam.srcY       = 0;
    bltParam.srcW       = width;
    bltParam.srcWStride = width;
    bltParam.srcH       = height;
    bltParam.srcHStride = height;
    bltParam.srcFormat  = mSrcHalBitbltFormat;
    //
    bltParam.dstAddr    = reinterpret_cast<uint32_t>(pDstBuf);
    bltParam.dstW       = width;
    bltParam.dstH       = height;
    bltParam.dstFormat  = mDstHalBitbltFormat;
    bltParam.pitch      = iDstStride;
    bltParam.orientation= MHAL_BITBLT_ROT_0;
    //
    int32_t status = ::mHalIoCtrl(MHAL_IOCTL_BITBLT, &bltParam, sizeof(bltParam), NULL, 0, NULL);
    if  (0 != status)
    {
        CAM_LOGW("[%s] MHAL_IOCTL_BITBLT - status(%x)", __FUNCTION__, status);
        return  false;
    }
    return  true;
}


/******************************************************************************
*   map Hal pixel format to MHAL_BITBLT_FORMAT_ENUM
*******************************************************************************/
static
inline
int
mapTomHalBitbltFormat(int const HalPixelFormat)
{
    switch (HalPixelFormat)
    {
//------------------------------------------------------------------------------
#if (0) //  the same endian
    //
    case HAL_PIXEL_FORMAT_RGB_565:
        return  MHAL_FORMAT_RGB_565;
    //
    case HAL_PIXEL_FORMAT_RGB_888:
        return  MHAL_FORMAT_RGB_888;
    //
    case HAL_PIXEL_FORMAT_RGBX_8888:
    case HAL_PIXEL_FORMAT_RGBA_8888:
        return  MHAL_FORMAT_RGBA_8888;
    //
    case HAL_PIXEL_FORMAT_BGRA_8888:
        return  MHAL_FORMAT_BGRA_8888;
    //
//------------------------------------------------------------------------------
#else   //  different endian
    //
    case HAL_PIXEL_FORMAT_RGB_565:
        return  MHAL_FORMAT_BGR_565;
    //
    case HAL_PIXEL_FORMAT_RGB_888:
        return  MHAL_FORMAT_BGR_888;
    //
    case HAL_PIXEL_FORMAT_RGBX_8888:
    case HAL_PIXEL_FORMAT_RGBA_8888:
        return  MHAL_FORMAT_ABGR_8888;
    //
    case HAL_PIXEL_FORMAT_BGRA_8888:
        return  MHAL_FORMAT_ARGB_8888;
    //
#endif
//------------------------------------------------------------------------------
    //
    case HAL_PIXEL_FORMAT_I420:
        return  MHAL_FORMAT_YUV_420;
    //
    case HAL_PIXEL_FORMAT_YCrCb_420_SP:
        return  MHAL_FORMAT_YUV_420_SP;
    //
    case HAL_PIXEL_FORMAT_YV12:
        return  MHAL_FORMAT_ANDROID_YV12;
    //
    HAL_PIXEL_FORMAT_YCbCr_422_I:
        return  MHAL_FORMAT_YUY2;
    //
    HAL_PIXEL_FORMAT_YCbCr_422_SP:
    default:
        CAM_LOGW("[%s] Unsupported HAL_PIXEL_FORMAT: 0x%08x", __FUNCTION__, HalPixelFormat);
        break;
    }
    //
    return  MHAL_FORMAT_ERROR;
}


}; // namespace android
#endif  //_MTK_DISPLAY_ADAPTER_MHAL_MHAL_UTILS_H_

