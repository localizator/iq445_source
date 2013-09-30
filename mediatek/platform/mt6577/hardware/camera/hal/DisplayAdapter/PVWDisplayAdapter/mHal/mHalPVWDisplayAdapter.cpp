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
#include "CameraProfile.h"
//
using namespace android;
using namespace MtkCamUtils;
//


/******************************************************************************
*
*******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[mHalPVWDisplayAdapter::%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[mHalPVWDisplayAdapter::%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[mHalPVWDisplayAdapter::%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[mHalPVWDisplayAdapter::%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[mHalPVWDisplayAdapter::%s] "fmt, __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, arg...)    if (cond) { MY_LOGV(arg); }
#define MY_LOGD_IF(cond, arg...)    if (cond) { MY_LOGD(arg); }
#define MY_LOGI_IF(cond, arg...)    if (cond) { MY_LOGI(arg); }
#define MY_LOGW_IF(cond, arg...)    if (cond) { MY_LOGW(arg); }
#define MY_LOGE_IF(cond, arg...)    if (cond) { MY_LOGE(arg); }


/******************************************************************************
*
*******************************************************************************/
IDisplayAdapter*
IDisplayAdapter::
createInstance()
{
    MY_LOGI("+ tid(%d)", ::gettid());
    return  new mHalPVWDisplayAdapter();
}


/******************************************************************************
*
*******************************************************************************/
mHalPVWDisplayAdapter::
mHalPVWDisplayAdapter()
    : PVWDisplayAdapter()
    //
#if (USE_HW_MEMCPY_MAP_BUF_DST)
    , mDstImgBufMap("Dst")
#endif
    //
#if (USE_HW_MEMCPY_MAP_BUF_SRC)
    , mSrcImgBufMap("Src")
#endif
{
}


/******************************************************************************
*
*******************************************************************************/
mHalPVWDisplayAdapter::
~mHalPVWDisplayAdapter()
{
}


/******************************************************************************
*
*******************************************************************************/
bool
mHalPVWDisplayAdapter::
copyFrame(int const iDstStride, void* pDstBuf, DispFrame const& rDispFrame)
{
    AutoCPTLog cptlog(Event_DispAdpt_copyFrame);
    CamProfile profile(__FUNCTION__, "mHalPVWDisplayAdapter");
    //
    //  Use hardware bitblt for memcpy
#if (USE_HW_MEMCPY) //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //
if  ( !(mIsNative3D && mIsRecordingHint)) { // N3D video cannnot use HW memory copy

    if  ( mIsRecordingHint )
    {
        if  ( CAMERA_MSG_PREVIEW_FRAME == rDispFrame.mType )
        {
        #if (USE_HW_MEMCPY_MAP_BUF_DST)     //
            mDstImgBufMap.add_IfNotFound(pDstBuf, true, rDispFrame.mWidth, rDispFrame.mHeight, rDispFrame.mi4HalPixelFormat);
        #endif  //USE_HW_MEMCPY_MAP_BUF_DST //
            //
        #if (USE_HW_MEMCPY_MAP_BUF_SRC)     //
            mSrcImgBufMap.add_IfNotFound(rDispFrame.mBuf, false, rDispFrame.mWidth, rDispFrame.mHeight, rDispFrame.mi4HalPixelFormat);
        #endif  //USE_HW_MEMCPY_MAP_BUF_SRC //
        }
    }
    //
    bool ret = hwBitblt(
        iDstStride, pDstBuf, rDispFrame.mBuf, 
        rDispFrame.mWidth, rDispFrame.mHeight, 
        MHAL_FORMAT_IMG_YV12, 
        mapTomHalBitbltFormat(rDispFrame.mi4HalPixelFormat)
    );
    //
    profile.print_overtime(10, "- memcpy display frame by hardware");
    //
    if (ret) {
        return  true;
    }

}
#endif  //USE_HW_MEMCPY //------------------------------------------------------
    //
#if 0
    ::memcpy(pDstBuf, rDispFrame.mBuf, rDispFrame.mLength);
    profile.print_overtime(5, "- memcpy display frame by cpu");
#elif 1
    MY_LOGE("- do nothing");
#else
    {
        int const wdith3D= iDstStride;
        int const width  = rDispFrame.mWidth;
        int const height = rDispFrame.mHeight;
        //  I420 (no stride) to YV12 (3D)
        uint8_t* psrc = reinterpret_cast<uint8_t*>(rDispFrame.mBuf);
        uint8_t* psrcY = psrc;
        uint8_t* psrcU = psrcY + (height * width);
        uint8_t* psrcV = psrcU + (height * width) / 4;
    
        uint8_t* pdst = reinterpret_cast<uint8_t*>(pDstBuf);
        uint8_t* pdstY = pdst;
        uint8_t* pdstV = pdstY + (height * wdith3D);
        uint8_t* pdstU = pdstV + (height * wdith3D) / 4;
    
        for (int i = 0; i < height; i++)
        {
            ::memcpy(&pdstY[i*wdith3D], &psrcY[i*width], width);
        }
        for (int i = 0; i < height/2; i++)
        {
            ::memcpy(&pdstV[i*wdith3D/2], &psrcV[i*width/2], width/2);
            ::memcpy(&pdstU[i*wdith3D/2], &psrcU[i*width/2], width/2);
        }
    }
#endif
    return  true;
}


/******************************************************************************
*
*******************************************************************************/
bool
mHalPVWDisplayAdapter::
onDestroy()
{
    MY_LOGD("+");
    //
#if (USE_HW_MEMCPY_MAP_BUF_DST)
    mDstImgBufMap.clear();
#endif
    //
#if (USE_HW_MEMCPY_MAP_BUF_SRC)
    mSrcImgBufMap.clear();
#endif
    //
    bool ret = PVWDisplayAdapter::onDestroy();
    //
    MY_LOGD("- ret(%d)", ret);
    return  ret;
}

