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

#define LOG_TAG "MtkCam/DisplayAdapter"
//
#include "Utils/inc/CamUtils.h"
//
#include "mHalPVWDisplayAdapter.h"
#include "mHalUtils.h"
//
using namespace android;
using namespace MtkCamUtils;
//
/******************************************************************************
*
*******************************************************************************/
#if (USE_IMG_BUF_MAPPER)
//
#include <mhal/inc/MediaHal.h>


/******************************************************************************
*
*******************************************************************************/
#define MY_LOGV(fmt, arg...)    CAM_LOGV("(%s)[ImgBufMapper::%s] "fmt, ms8Name.string(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)    CAM_LOGD("(%s)[ImgBufMapper::%s] "fmt, ms8Name.string(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)    CAM_LOGI("(%s)[ImgBufMapper::%s] "fmt, ms8Name.string(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)    CAM_LOGW("(%s)[ImgBufMapper::%s] "fmt, ms8Name.string(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)    CAM_LOGE("(%s)[ImgBufMapper::%s] "fmt, ms8Name.string(), __FUNCTION__, ##arg)


/******************************************************************************
*
*******************************************************************************/
ImgBufMapper::
ImgBufMapper(char const* name /*= ""*/)
    : mMutex()
    , ms8Name(name)
    , mMap(NULL)
{
    MY_LOGD("tid(%d)", ::gettid());
}


/******************************************************************************
*
*******************************************************************************/
ImgBufMapper::
~ImgBufMapper()
{
    MY_LOGD("+ tid(%d)", ::gettid());
    clear();
    MY_LOGD("- tid(%d)", ::gettid());
}


/******************************************************************************
*
*******************************************************************************/
void
ImgBufMapper::
clear()
{
    Mutex::Autolock _l(mMutex);
    //
    MY_LOGD("+ mMap.size(%d)", mMap.size());
    //
    for (size_t i = 0; i < mMap.size(); i++)
    {
        mHalRegisterLoopMemoryObj_t* pRegMemObj = reinterpret_cast<mHalRegisterLoopMemoryObj_t*>(mMap.valueAt(i));
        void*                              pBuf = mMap.keyAt(i);
        //
        if  ( ! pRegMemObj )
        {
            MY_LOGW("NULL pRegMemObj");
            continue;
        }
        //
        CamProfile profile(__FUNCTION__, "ImgBufMapper");
        mHalMdp_UnRegisterLoopMemory(MHAL_MLM_CLIENT_PVCPY, pRegMemObj);
        profile.print_overtime(0, "(%s) mHalMdp_UnRegisterLoopMemory - i(%d), pBuf(%p) --> pRegMemObj(%p)", ms8Name.string(), i, pBuf, pRegMemObj);
        delete  pRegMemObj;
    }
    //
    mMap.clear();
    MY_LOGD("- mMap.size(%d)", mMap.size());
}


/******************************************************************************
*
*******************************************************************************/
bool
ImgBufMapper::
add(
    void*const      pBuf, 
    bool const      bIsOut, 
    int32_t const   i4Width, 
    int32_t const   i4Height, 
    int32_t const   i4HalPixelFormat
)
{
    bool ret = false;
    mHalRegisterLoopMemoryObj_t* pRegMemObj = NULL;
    mHalRegisterLoopMemory_t     RegMem;
    //
    MY_LOGD(
        "+ mMap.size(%d), pBuf(%p), WxH=%dx%d, format(0x%08x), out(%d)", 
        mMap.size(), pBuf, i4Width, i4Height, i4HalPixelFormat, bIsOut
    );
    //
    CamProfile profile(__FUNCTION__, "ImgBufMapper");
    //
    if  ( ! pBuf )
    {
        MY_LOGW("NULL pBuf");
        ret = true;
        goto lbExit;
    }
    //
    pRegMemObj = new mHalRegisterLoopMemoryObj_t;
    if  ( ! pRegMemObj )
    {
        MY_LOGW("fail to new mHalRegisterLoopMemoryObj_t");
        goto lbExit;
    }
    //
    RegMem.mem_type     = bIsOut ? MHAL_MEM_TYPE_OUTPUT : MHAL_MEM_TYPE_INPUT;
    RegMem.addr         = reinterpret_cast<uint32_t>(pBuf);
    RegMem.buffer_size  = (i4Width * i4Height * queryBitsPerPixel(i4HalPixelFormat)) >> 3;
    RegMem.mhal_color   = (MHAL_BITBLT_FORMAT_ENUM)mapTomHalBitbltFormat(i4HalPixelFormat);
    RegMem.img_size     = mHalMdpSize(i4Width, i4Height);
    RegMem.img_roi      = mHalMdpRect(0, 0, i4Width, i4Height);
    RegMem.rotate       = 0;
    //
    if  ( mHalMdp_RegisterLoopMemory(MHAL_MLM_CLIENT_PVCPY, &RegMem, pRegMemObj) < 0 )
    {
        MY_LOGE("mHalMdp_RegisterLoopMemory(MHAL_MLM_CLIENT_PVCPY) fails");
        delete  pRegMemObj;
        pRegMemObj = NULL;
        goto lbExit;
    }
    //
    mMap.add(pBuf, pRegMemObj);
    MY_LOGI("  mMap.size(%d): pBuf(%p) --> pRegMemObj(%p)", mMap.size(), pBuf, pRegMemObj);
    //
    ret = true;
lbExit:
    profile.print_overtime(1, "(%s) - mHalMdp_RegisterLoopMemory", ms8Name.string());
    MY_LOGD("- mMap.size(%d), pBuf(%p), ret(%d)", mMap.size(), pBuf, ret);
    return  ret;
}


/******************************************************************************
*
*******************************************************************************/
bool
ImgBufMapper::
add_IfNotFound(
    void*const      pBuf, 
    bool const      bIsOut, 
    int32_t const   i4Width, 
    int32_t const   i4Height, 
    int32_t const   i4HalPixelFormat
)
{
    Mutex::Autolock _l(mMutex);
    //
    if  ( ! valueFor(pBuf) )
    {
        return  add(pBuf, bIsOut, i4Width, i4Height, i4HalPixelFormat);
    }
    return  true;
}


/******************************************************************************
*
*******************************************************************************/
#endif  //USE_IMG_BUF_MAPPER

