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

#define LOG_TAG "MtkCam/CamDevice"
//
#include "Utils/inc/CamUtils.h"
using namespace android;
using namespace android::MtkCamUtils;
//
#include <camera/MtkCameraParameters.h>
//
#include "CamBufProvider.h"
#include <math.h>


/******************************************************************************
*
*******************************************************************************/
#define MY_LOGV(fmt, arg...)    CAM_LOGV("[CamBufProvider::%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)    CAM_LOGD("[CamBufProvider::%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)    CAM_LOGI("[CamBufProvider::%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)    CAM_LOGW("[CamBufProvider::%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)    CAM_LOGE("[CamBufProvider::%s] "fmt, __FUNCTION__, ##arg)


/******************************************************************************
*
*******************************************************************************/
CamBufProvider::
CamBufProvider()
    : IBufferProvider()
    , mfpRequestMemory(NULL)
{
}


/******************************************************************************
*
*******************************************************************************/
void
CamBufProvider::
setAllocator(camera_request_memory const get_memory)
{
    mfpRequestMemory = get_memory;
}


/******************************************************************************
* Allocate memory from camera framework.
*******************************************************************************/
bool
CamBufProvider::
allocBuffer(camera_memory_t& rBuf, size_t const bufSize, uint32_t const bufCount) const
{
    if  ( 0 == bufSize || 0 == bufCount )
    {
        MY_LOGE("bad arguments: (bufSize,bufCount)=(%d,%d)", bufSize, bufCount);
        return  false;
    }
    //
    if  ( ! mfpRequestMemory )
    {
        MY_LOGE("no initialization");
        return  false;
    }
    //
    camera_memory_t* pmem = mfpRequestMemory(-1, bufSize, bufCount, NULL);
    if  ( ! pmem )
    {
        MY_LOGE("Cannot request memory (bufSize, bufCount)=(%d,%d)", bufSize, bufCount);
        return  false;
    }
    //
    if  ( ! pmem->data )
    {
        MY_LOGE("NULL data for requested memory(%p) with (bufSize, bufCount)=(%d,%d)", pmem, bufSize, bufCount);
        pmem->release(pmem);
        return  false;
    }
    //
    rBuf = *pmem;
    return  true;
}


/******************************************************************************
* Release memory to camera framework.
*******************************************************************************/
bool
CamBufProvider::
freeBuffer(camera_memory_t& rBuf) const
{
    rBuf.release(&rBuf);
    ::memset(&rBuf, 0, sizeof(camera_memory_t));
    return  true;
}


/******************************************************************************
* 
*******************************************************************************/
bool
CamBufProvider::
allocBuffer(
    sp<IMemoryBufferPool>&  rpBuf, 
    size_t const            bufSize, 
    uint32_t const          bufCount, 
    char const*const        szName /*= ""*/
) const
{
    //
    IMemoryBufferPool* pBuf = NULL;
    camera_memory_t mem;
    if  ( ! allocBuffer(mem, bufSize, bufCount) )
    {
        return  false;
    }
    //
    //
    pBuf = new CamMemBufPool(mem, bufSize, bufCount, szName);
    if  ( ! pBuf )
    {
        MY_LOGE("cannot new CamMemBufPool");
        pBuf = NULL;
        mem.release(&mem);
        return  false;
    }
    //
    if  (
            NULL == pBuf->getVirAddr()
        ||  pBuf->getPoolSize() < pBuf->getBufCount() * pBuf->getBufSize()
        )
    {
        MY_LOGE(
            "%s, base:0x%08x, size: %d x %d = %d", 
            pBuf->getPoolName(), pBuf->getVirAddr(), pBuf->getBufCount(), pBuf->getBufSize(), pBuf->getPoolSize()
        );
        delete  pBuf;
        pBuf = NULL;
        return  false;
    }
    //
    rpBuf = pBuf;
    //
    return  true;
}


/******************************************************************************
* 
*******************************************************************************/
bool
CamBufProvider::
allocBuffer(
    sp<IImageBufferPool>&   rpBuf,
    char const*const        szImgFormat, 
    uint32_t const          u4ImgWidth, 
    uint32_t const          u4ImgHeight, 
    uint32_t const          bufCount, 
    char const*const        szName /*= ""*/
) const
{
    //
    IImageBufferPool* pBuf = NULL;
    //
    uint32_t const u4BitsPerPixel = queryBitsPerPixel( mapCameraToPixelFormat(String8(szImgFormat)) );
    //
    size_t bufSize = ( (u4ImgWidth * u4ImgHeight * u4BitsPerPixel) >> 3 );
    if ( strcmp(szImgFormat, MtkCameraParameters::PIXEL_FORMAT_YUV420P) == 0 )
    {
        /*
        *   uint32_t yStride = (uint32_t)ceil((double)u4ImgWidth / 16.0) * 16;
        *   uint32_t uvStride = (uint32_t)ceil((double)yStride / 2 / 16.0) * 16;
        */
        uint32_t yStride    = (u4ImgWidth + 16-1) & ~(16-1);
        uint32_t uvStride   = ((yStride >> 1) + 16-1) & ~(16-1);
        uint32_t ySize = yStride * u4ImgHeight;
        uint32_t uvSize = uvStride * u4ImgHeight / 2;
        bufSize = ySize + uvSize * 2;
    }
    MY_LOGD("bufSize=%d for %d-bit %dx%d image", bufSize, u4BitsPerPixel, u4ImgWidth, u4ImgHeight);
    //
    camera_memory_t mem;
    if  ( ! allocBuffer(mem, bufSize, bufCount) )
    {
        return  false;
    }
    //
    //
    pBuf = new CamImgBufPool(mem, bufSize, bufCount, u4ImgWidth, u4ImgHeight, szImgFormat, szName);
    if  ( ! pBuf )
    {
        MY_LOGE("cannot new CamImgBufPool");
        pBuf = NULL;
        mem.release(&mem);
        return  false;
    }
    //
    if  (
            NULL == pBuf->getVirAddr()
        ||  pBuf->getPoolSize() < pBuf->getBufCount() * pBuf->getBufSize()
        )
    {
        MY_LOGE(
            "%s, base:0x%08x, size: %d x %d = %d", 
            pBuf->getPoolName(), pBuf->getVirAddr(), pBuf->getBufCount(), pBuf->getBufSize(), pBuf->getPoolSize()
        );
        delete  pBuf;
        pBuf = NULL;
        return  false;
    }
    //
    rpBuf = pBuf;
    //
    return  true;
}

