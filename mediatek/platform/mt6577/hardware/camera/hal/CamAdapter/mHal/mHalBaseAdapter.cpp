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

#define LOG_TAG "MtkCam/CamAdapter"
//
#include "inc/ICamAdapter.h"
#include "inc/BaseCamAdapter.h"
#include "inc/mHalBaseAdapter.h"
//
#include <camera/MtkCameraParameters.h>
//
#include <mhal/inc/camera.h>
#include <mhal/inc/camera/ioctl.h>


/******************************************************************************
*
*******************************************************************************/
#define MY_LOGV(fmt, arg...)    CAM_LOGV("(%s)[mHalBaseAdapter::%s] "fmt, getName(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)    CAM_LOGD("(%s)[mHalBaseAdapter::%s] "fmt, getName(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)    CAM_LOGI("(%s)[mHalBaseAdapter::%s] "fmt, getName(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)    CAM_LOGW("(%s)[mHalBaseAdapter::%s] "fmt, getName(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)    CAM_LOGE("(%s)[mHalBaseAdapter::%s] "fmt, getName(), __FUNCTION__, ##arg)


/******************************************************************************
*
*******************************************************************************/
namespace android {


/******************************************************************************
*
*******************************************************************************/
mHalBaseAdapter::
mHalBaseAdapter(String8 const& rName, int32_t const i4OpenId, CamDevInfo const& rDevInfo)
    : BaseCamAdapter(rName, i4OpenId, rDevInfo)
    , mmHalFd(NULL)
{
}


/******************************************************************************
*
*******************************************************************************/
mHalBaseAdapter::
~mHalBaseAdapter()
{
}


/******************************************************************************
*
*******************************************************************************/
bool
mHalBaseAdapter::
init()
{
    bool ret = false;
    status_t status = NO_ERROR;
    //
    MY_LOGI("caller tid(%d), OpenId(%d), getStrongCount(%d)", ::gettid(), getOpenId(), getStrongCount());
    //
    //
    mmHalFd = ::mHalOpen();
    if  ( ! mmHalFd )
    {
        MY_LOGE("mHalOpen() return NULL");
        goto lbExit;
    }
    //
    status = ::mHalIoctl(mmHalFd, MHAL_IOCTL_INIT_CAMERA, (void*)getOpenId(), 0, NULL, 0, NULL);
    if  ( status )
    {
        MY_LOGE("MHAL_IOCTL_INIT_CAMERA err(%x)", status);
        goto lbExit;
    }
    //
    //
    onInitParameters();
    //
    //
    ret = true;
lbExit:
    return  ret;
}


/******************************************************************************
*
*******************************************************************************/
bool
mHalBaseAdapter::
uninit()
{
    MY_LOGI("caller tid(%d), OpenId(%d), getStrongCount(%d)", ::gettid(), getOpenId(), getStrongCount());
    //
    //
    if  ( mmHalFd )
    {
        //
        ::mHalIoctl(mmHalFd, MHAL_IOCTL_UNINIT_CAMERA, NULL, 0, NULL, 0, NULL);
        //
        ::mHalClose(mmHalFd);
        mmHalFd = NULL;
    }
    //
    return  true;
}


/******************************************************************************
*
*******************************************************************************/
String8
mHalBaseAdapter::
mapPixelFormat_mHal_to_string(uint32_t const u4Format)
{
    using namespace NSCamera;
    //
    String8 s8PixelFormat;
    switch  (u4Format)
    {
    case ePIXEL_FORMAT_I420:
        s8PixelFormat = MtkCameraParameters::PIXEL_FORMAT_YUV420I;
        break;
        //
    case ePIXEL_FORMAT_YV12:
        s8PixelFormat = MtkCameraParameters::PIXEL_FORMAT_YUV420P;
        break;
        //
    case ePIXEL_FORMAT_NV21:
        s8PixelFormat = MtkCameraParameters::PIXEL_FORMAT_YUV420SP;
        break;
        //
    case ePIXEL_FORMAT_RGBA8888:
        s8PixelFormat = MtkCameraParameters::PIXEL_FORMAT_RGBA8888;
        break;
        //
    default:
        CAM_LOGW("Unsupported pixel format from mhal: 0x%08X", u4Format);
        break;
    }
    return  s8PixelFormat;
}


/******************************************************************************
*
*******************************************************************************/
uint32_t
mHalBaseAdapter::
mapPixelFormat_string_to_mHal(String8 const& s8Format)
{
    using namespace NSCamera;
    //
    if  ( s8Format == MtkCameraParameters::PIXEL_FORMAT_YUV420I )
    {
        return  ePIXEL_FORMAT_I420;
    }
    //
    if  ( s8Format == MtkCameraParameters::PIXEL_FORMAT_YUV420P )
    {
        return  ePIXEL_FORMAT_YV12;
    }
    //
    if  ( s8Format == MtkCameraParameters::PIXEL_FORMAT_YUV420SP )
    {
        return  ePIXEL_FORMAT_NV21;
    }
    //
    if  ( s8Format == MtkCameraParameters::PIXEL_FORMAT_RGBA8888 )
    {
        return  ePIXEL_FORMAT_RGBA8888;
    }
    //
//    if  ( s8Format == "mtkyuv" )
//    {
//        return  MHAL_CAM_BUF_FMT_MTKYUV;
//    }
    //
    CAM_LOGW("Unsupported pixel format: %s", s8Format.string());
    return  ePIXEL_FORMAT_UNKNOWN;
}


/******************************************************************************
*
*******************************************************************************/
sp<IImageBufferPool>
mHalBaseAdapter::
allocImagePool(
    int const poolID, 
    int const frmW, 
    int const frmH, 
    char const*const szName
) const
{
    status_t status = NO_ERROR;
    //
    int i = 0, retry = 10;
    int const us = 200 * 1000;
    //
    MY_LOGD("poolID = %d, poolName = %s, frmW = %d, frmH = %d", poolID, szName, frmW, frmH);
    //
    mhalCamBufMemInfo_t camBufInfo; 
    ::memset(&camBufInfo, 0, sizeof(mhalCamBufMemInfo_t));
    camBufInfo.bufID = poolID;
    camBufInfo.frmW = frmW;
    camBufInfo.frmH = frmH;
    //
    status = ::mHalIoctl(mmHalFd, MHAL_IOCTL_GET_CAM_BUF_MEM_INFO, NULL, 0, &camBufInfo, sizeof(mhalCamBufMemInfo_t), NULL);
    if (status != NO_ERROR) {
        MY_LOGE("MHAL_IOCTL_GET_CAM_BUF_INFO fail, status(%d)", status);
        return  NULL;
    }
    //
    if  ( NSCamera::ePIXEL_FORMAT_UNKNOWN == camBufInfo.camBufFmt )
    {
        MY_LOGE("mhal format: ePIXEL_FORMAT_UNKNOWN");
        return  NULL;
    }
    //
    //
    sp<IImageBufferPool> mem;
    String8 const s8PixelFormat = mapPixelFormat_mHal_to_string(camBufInfo.camBufFmt);
    //
    //
    do  {
        mpBufProvider->allocBuffer(mem, s8PixelFormat, frmW, frmH, camBufInfo.camBufCount, szName);
        //
        if  ( mem != NULL && 0 != mem->getPoolSize() && 0 != mem->getVirAddr() ) {
            break; 
        }
        ::usleep(us);
        CAM_LOGV("retry: %d", i++);
    } while ((--retry) > 0);
    //
    if  ( mem == NULL || 0 == mem->getPoolSize() || 0 == mem->getVirAddr() ) {
        // Should not happen
        MY_LOGE("alloc mem fail, should not happen");
        mem = NULL;
    }
    //
    return  mem;
}


}; // namespace android

