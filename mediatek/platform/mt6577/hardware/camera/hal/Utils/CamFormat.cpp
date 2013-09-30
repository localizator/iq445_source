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

#define LOG_TAG "MtkCam/MtkCamUtils"
//
#include <system/graphics.h>
#include <camera/MtkCameraParameters.h>
//
#include "inc/_CamLog.h"
#include "inc/_CamFormat.h"
//
#include <math.h>
//


/******************************************************************************
*
*******************************************************************************/
#define MY_LOGV(fmt, arg...)    CAM_LOGV("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)    CAM_LOGD("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)    CAM_LOGI("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)    CAM_LOGW("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)    CAM_LOGE("[%s] "fmt, __FUNCTION__, ##arg)


/******************************************************************************
*
*******************************************************************************/
namespace android {
namespace MtkCamUtils {


/******************************************************************************
*   Map: CameraParameters format to HAL_PIXEL_FORMAT_xxx
*******************************************************************************/
class CamFmtMapImp : public CamFmtMap
{
    friend  class   CamFmtMap;
protected:  ////
    DefaultKeyedVector<KEY, VALUE>  mVector_Cam2Hal;
    DefaultKeyedVector<VALUE, KEY>  mVector_Hal2Cam;
    //
    CamFmtMapImp()
        : CamFmtMap()
        , mVector_Cam2Hal(0)
        , mVector_Hal2Cam(String8(""))
    {
        mVector_Cam2Hal.clear();
        mVector_Hal2Cam.clear();
        //
#define DO_MAP(k, v)    \
        do { \
            mVector_Cam2Hal.add(String8(k), v); \
            mVector_Hal2Cam.add(v, String8(k)); \
        } while (0)
        //
        DO_MAP(MtkCameraParameters::PIXEL_FORMAT_YUV420P,   HAL_PIXEL_FORMAT_YV12);
        DO_MAP(MtkCameraParameters::PIXEL_FORMAT_YUV420SP,  HAL_PIXEL_FORMAT_YCrCb_420_SP);
        DO_MAP(MtkCameraParameters::PIXEL_FORMAT_YUV422I,   HAL_PIXEL_FORMAT_YCbCr_422_I);
        DO_MAP(MtkCameraParameters::PIXEL_FORMAT_RGB565,    HAL_PIXEL_FORMAT_RGB_565);
        DO_MAP(MtkCameraParameters::PIXEL_FORMAT_RGBA8888,  HAL_PIXEL_FORMAT_RGBA_8888);
        DO_MAP(MtkCameraParameters::PIXEL_FORMAT_YUV422SP,  HAL_PIXEL_FORMAT_YCbCr_422_SP);
        DO_MAP(MtkCameraParameters::PIXEL_FORMAT_YUV420I,   HAL_PIXEL_FORMAT_I420);
        //
#undef  DO_MAP
    }

public:     ////    Interfaces.
    //
    virtual VALUE           valueFor(KEY const& key) const
    {
        int const HalPixelFormat = mVector_Cam2Hal.valueFor(key);
        switch  (HalPixelFormat)
        {
        case HAL_PIXEL_FORMAT_YV12:
            MY_LOGV("(YV12) HAL_PIXEL_FORMAT_YV12 format selected");
            break;
        //
        case HAL_PIXEL_FORMAT_YCrCb_420_SP:
            MY_LOGV("(NV21) HAL_PIXEL_FORMAT_YCrCb_420_SP format selected");
            break;
        //
        case HAL_PIXEL_FORMAT_YCbCr_422_I:
            MY_LOGV("(YUY2) HAL_PIXEL_FORMAT_YCbCr_422_I format selected");
            break;
        //
        case HAL_PIXEL_FORMAT_RGB_565:
            MY_LOGV("(RGB565) HAL_PIXEL_FORMAT_RGB_565 format selected");
            break;
        //
        case HAL_PIXEL_FORMAT_RGBA_8888:
            MY_LOGV("(RGBA8888) HAL_PIXEL_FORMAT_RGBA_8888 format selected");
            break;
        //
        case HAL_PIXEL_FORMAT_YCbCr_422_SP:
            MY_LOGV("(NV16) HAL_PIXEL_FORMAT_YCbCr_422_SP format selected");
            break;
        //
        case HAL_PIXEL_FORMAT_I420:
            MY_LOGV("(I420) HAL_PIXEL_FORMAT_I420 format selected");
            break;
        //
        default:
            MY_LOGW("Unsupported camera format (%s); %x selected as default.", key.string(), HalPixelFormat);
            break;
        }
        //
        return  HalPixelFormat;
    }
    //
    virtual KEY const&      keyFor(VALUE const& value) const
    {
        return  mVector_Hal2Cam.valueFor(value);
    }
};


CamFmtMap&
CamFmtMap::inst()
{
    static CamFmtMapImp singleton;
    return singleton;
}


/******************************************************************************
*   map CameraParameters format to Hal pixel format.
*******************************************************************************/
int
mapCameraToPixelFormat(String8 const& rs8CameraFormat)
{
    return  CamFmtMap::inst().valueFor(rs8CameraFormat);
}


/******************************************************************************
*   query bits per pixel for a given Hal pixel format.
*******************************************************************************/
int queryBitsPerPixel(int const HalPixelFormat)
{
    switch (HalPixelFormat)
    {
    case HAL_PIXEL_FORMAT_YCbCr_422_SP:
        return 16;
    case HAL_PIXEL_FORMAT_YCbCr_422_I:
        return 16;
    //
    case HAL_PIXEL_FORMAT_I420:
    case HAL_PIXEL_FORMAT_YV12:
        return 12;
    case HAL_PIXEL_FORMAT_YCrCb_420_SP:
        return 12;
    //
    case HAL_PIXEL_FORMAT_RGB_565:
        return 16;
    case HAL_PIXEL_FORMAT_RGB_888:
        return 24;
    case HAL_PIXEL_FORMAT_RGBX_8888:
    case HAL_PIXEL_FORMAT_RGBA_8888:
    case HAL_PIXEL_FORMAT_BGRA_8888:
        return 32;
    default:
        MY_LOGW("Unsupported HAL_PIXEL_FORMAT: 0x%08x", HalPixelFormat);
        break;
    }
    return 0;
}


/******************************************************************************
*   convert Mtk YUV to Yuv 420sp
*******************************************************************************/
bool convertMtkYuvToYuv420sp(uint8_t *pbufIn, uint8_t *pbufOut, uint32_t width, uint32_t height)
{
    uint32_t i, j;
    uint32_t idx, stride;
    uint8_t *pbufY, *pbufU, *pbufV;

    MY_LOGV("[convertMtkYuvToYuv420sp] pbufIn: 0x%x, pbufOut: 0x%x, width: %d, height: %d \n\n", (uint32_t) pbufIn, (uint32_t) pbufOut, width, height);

    stride = width << 2;
    pbufY = pbufIn;
    for (i = 0; i < height; i++) {
        for (j = 0; j < width; j+=4) {
            //idx = 16 * (j / 4) + ((i % 4) * 4) + ((i / 4) * 1280);
            idx = ((j >> 2) << 4) + ((i & 0x03) << 2) + ((i >> 2) * stride);
            *pbufOut++ = pbufY[idx++];
            *pbufOut++ = pbufY[idx++];
            *pbufOut++ = pbufY[idx++];
            *pbufOut++ = pbufY[idx++];
        }
    }

    stride = width << 1;
    pbufU = pbufIn + width * height;
    pbufV = pbufIn + (width * height) * 5 / 4;
    for (i = 0; i < height / 2; i++) {
        for (j = 0; j < width / 2; j+=4) {
            //idx = width * height + 16 * (j / 4) + ((i % 4) * 4) + ((i / 4) * 1280 / 2);
            idx = ((j >> 2) << 4) + ((i & 0x03) << 2) + ((i >> 2) * stride);
            *pbufOut++ = pbufV[idx];
            *pbufOut++ = pbufU[idx++];
            *pbufOut++ = pbufV[idx];
            *pbufOut++ = pbufU[idx++];
            *pbufOut++ = pbufV[idx];
            *pbufOut++ = pbufU[idx++];
            *pbufOut++ = pbufV[idx];
            *pbufOut++ = pbufU[idx++];
        }
    }

    #if 0
    stride = width << 1;
    pbufV = pbufIn + (width * height) * 5 / 4;
    for (i = 0; i < height / 2; i++) {
        for (j = 0; j < width / 2; j+=4) {
            //idx = width * height + (width/2) * (height/2) + 16 * (j / 4) + ((i % 4) * 4) + ((i / 4) * 1280 / 2);
            idx = ((j >> 2) << 4) + ((i & 0x03) << 2) + ((i >> 2) * stride);
            *pbufOut++ = pbufV[idx++];
            *pbufOut++ = pbufV[idx++];
            *pbufOut++ = pbufV[idx++];
            *pbufOut++ = pbufV[idx++];
        }
    }
    #endif
    return  true;
}


/******************************************************************************
*   convert i420 Yuv to Yuv 420sp (NV21)
*******************************************************************************/
bool convertYuv420iToYuv420sp(uint8_t *pbufIn, uint8_t *pbufOut, uint32_t width, uint32_t height)
{
    uint32_t i, j;
    uint8_t *pbufInY, *pbufInU, *pbufInV;
    uint8_t *pbufOutY, *pbufOutVU; 

    MY_LOGV("+ pbufIn(%p), pbufOut(%p), %dx%d", pbufIn, pbufOut, width, height);
    pbufInY = pbufIn;
    pbufInU = pbufIn + width * height; 
    pbufInV = pbufIn + (width * height) * 5 / 4; 

    pbufOutY = pbufOut; 
    pbufOutVU = pbufOut + width * height; 

    //Y
    memcpy(pbufOutY, pbufInY, width * height); 
    
    //VU 
    for (i = 0; i < width * height / 4; i++) {
        *pbufOutVU++ = *pbufInV++; 
        *pbufOutVU++ = *pbufInU++; 
    }

    MY_LOGV("-");
    return  true;
}


/******************************************************************************
*   convert i420 Yuv to Yuv 420p (YV12)
*******************************************************************************/
bool convertYuv420iToYuv420p(uint8_t *pbufIn, uint8_t *pbufOut, uint32_t width, uint32_t height)
{
    MY_LOGV("+ pbufIn(%p), pbufOut(%p), %dx%d", pbufIn, pbufOut, width, height);
    //
    size_t const yStridei   = width;
    size_t const uvStridei  = yStridei / 2;
    size_t const ySizei     = width*height;
    size_t const uvSizei    = ySizei / 4;
    /*
    *   uint32_t yStride = (uint32_t)ceil((double)u4ImgWidth / 16.0) * 16;
    *   uint32_t uvStride = (uint32_t)ceil((double)yStride / 2 / 16.0) * 16;
    */
    size_t const yStrideo   = (width + 16-1) & ~(16-1);
    size_t const uvStrideo  = ((yStrideo >> 1) + 16-1) & ~(16-1);
    size_t const ySizeo     = yStrideo * height;
    size_t const uvSizeo    = uvStrideo * height / 2;
    //
    for (uint32_t idx = 0; idx < height; idx++)
    {
        uint8_t*const pbufInY   = pbufIn + yStridei*idx;
        uint8_t*const pbufOutY  = pbufOut + yStrideo*idx;
        ::memcpy(pbufOutY, pbufInY, yStridei);

        if (idx < height/2)
        {
            uint8_t*const pbufInU   = pbufIn  + ySizei           + uvStridei*idx;
            uint8_t*const pbufInV   = pbufIn  + ySizei + uvSizei + uvStridei*idx;
            uint8_t*const pbufOutV  = pbufOut + ySizeo           + uvStrideo*idx;
            uint8_t*const pbufOutU  = pbufOut + ySizeo + uvSizeo + uvStrideo*idx;

            ::memcpy(pbufOutV, pbufInV, uvStridei);
            ::memcpy(pbufOutU, pbufInU, uvStridei);
        }        
    }
    //
    MY_LOGV("-");
    return  true;
}


};  // namespace MtkCamUtils
}; // namespace android

