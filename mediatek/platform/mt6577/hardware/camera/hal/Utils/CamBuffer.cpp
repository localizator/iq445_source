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
#include <stdlib.h>
#include <hardware/camera.h>
//
#include "inc/_CamLog.h"
#include "inc/_CamTypes.h"
#include "inc/_ICamBuffer.h"
#include "inc/_CamBuffer.h"
#include "inc/_CamFormat.h"
using namespace android;
using namespace MtkCamUtils;
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
CamMemBufPool::
CamMemBufPool(
    camera_memory_t const mem, 
    size_t const bufSize, 
    uint32_t const bufCount, 
    char const*const poolName /*= ""*/
)
    : IMemoryBufferPool()
    //
    , mvRefCount()
    //
    , ms8PoolName(poolName)
    , mBufSize(bufSize)
    , mBufCount(bufCount)
    , mu4VirBase(reinterpret_cast<uint32_t>(mem.data))
    , mu4PhyBase(reinterpret_cast<uint32_t>(mem.data))
    , mCamMem(mem)
{
    //
    mvRefCount.setCapacity(mBufCount);
    ::memset(mvRefCount.editArray(), 0, mvRefCount.capacity() * sizeof(int32_t));
    //
    MY_LOGD(
        "%s, base:0x%08x, size: %d x %d = %d", 
        getPoolName(), getVirAddr(0), getBufCount(), getBufSize(), getPoolSize()
    );
    //
    if  ( getPoolSize() < getBufSize() * getBufCount() )
    {
        MY_LOGE("PoolSize(%d) < BufSize(%d) x BufCount(%d)", getPoolSize(), getBufSize(), getBufCount());
    }
}


/******************************************************************************
*
*******************************************************************************/
CamMemBufPool::
~CamMemBufPool()
{
    //
    MY_LOGD(
        "[tid(%d)] freeing %s, base:0x%08x, size: %d x %d = %d - mCamMem.release(%p)", 
        ::gettid(), getPoolName(), getVirAddr(0), getBufCount(), getBufSize(), getPoolSize(), mCamMem.release
    );
    //
    if  ( mCamMem.release )
    {
        mCamMem.release(&mCamMem);
        mCamMem.release = NULL;
    }
}


/******************************************************************************
*
*******************************************************************************/
int32_t
CamMemBufPool::
getRefCount(uint_t const index) const
{
    return  mvRefCount[index];
}


/******************************************************************************
*
*******************************************************************************/
int32_t
CamMemBufPool::
incRefCount(uint_t const index)
{
    return ::android_atomic_inc(&mvRefCount.editItemAt(index));
}


/******************************************************************************
*
*******************************************************************************/
int32_t
CamMemBufPool::
decRefCount(uint_t const index)
{
    int32_t const c = ::android_atomic_dec(&mvRefCount.editItemAt(index));
    return c;
}


/******************************************************************************
*
*******************************************************************************/
CamImgBufPool::
CamImgBufPool(
    camera_memory_t const mem, 
    size_t const bufSize, 
    uint32_t const bufCount, 
    uint32_t const u4ImgWidth, 
    uint32_t const u4ImgHeight, 
    char const*const ImgFormat, 
    char const*const poolName /*= ""*/
)
    : IImageBufferPool()
    //
    , mvRefCount()
    //
    , ms8PoolName(poolName)
    , mBufSize(bufSize)
    , mBufCount(bufCount)
    , mu4VirBase(reinterpret_cast<uint32_t>(mem.data))
    , mu4PhyBase(reinterpret_cast<uint32_t>(mem.data))
    , mCamMem(mem)
    //
    , ms8ImgFormat(ImgFormat)
    , mu4ImgWidth(u4ImgWidth)
    , mu4ImgHeight(u4ImgHeight)
    , mu4BitsPerPixel( queryBitsPerPixel( mapCameraToPixelFormat(ms8ImgFormat) ) )
{
    //
    mvRefCount.setCapacity(mBufCount);
    ::memset(mvRefCount.editArray(), 0, mvRefCount.capacity() * sizeof(int32_t));
    //
    MY_LOGD(
        "%s, base:0x%08x, size: %d x %d = %d", 
        getPoolName(), getVirAddr(0), getBufCount(), getBufSize(), getPoolSize()
    );
    //
    MY_LOGD(
        "(%s) image format=%s, image size=(%dx%d), bits per pixel=%d", 
        getPoolName(), getImgFormat().string(), getImgWidth(), getImgHeight(), getBitsPerPixel()
    );
    //
    if  ( getPoolSize() < getBufSize() * getBufCount() )
    {
        MY_LOGE("PoolSize(%d) < BufSize(%d) x BufCount(%d)", getPoolSize(), getBufSize(), getBufCount());
    }
    //
    if  ( getBufSize() < ((getImgWidth() * getImgHeight() * getBitsPerPixel())>>3) )
    {
        MY_LOGE("BufSize(%d) < %d x %d x %d / 8", getBufSize(), getImgWidth(), getImgHeight(), getBitsPerPixel());
    }
    //
}


/******************************************************************************
*
*******************************************************************************/
CamImgBufPool::
~CamImgBufPool()
{
    MY_LOGD(
        "[tid(%d)] freeing %s, base:0x%08x, size: %d x %d = %d - mCamMem.release(%p)", 
        ::gettid(), getPoolName(), getVirAddr(0), getBufCount(), getBufSize(), getPoolSize(), mCamMem.release
    );
    //
    if  ( mCamMem.release )
    {
        mCamMem.release(&mCamMem);
        mCamMem.release = NULL;
    }
}


/******************************************************************************
*
*******************************************************************************/
int32_t
CamImgBufPool::
getRefCount(uint_t const index) const
{
    return  mvRefCount[index];
}


/******************************************************************************
*
*******************************************************************************/
int32_t
CamImgBufPool::
incRefCount(uint_t const index)
{
    return ::android_atomic_inc(&mvRefCount.editItemAt(index));
}


/******************************************************************************
*
*******************************************************************************/
int32_t
CamImgBufPool::
decRefCount(uint_t const index)
{
    int32_t const c = ::android_atomic_dec(&mvRefCount.editItemAt(index));
    return c;
}

