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
#include "inc/IDisplayAdapter.h"
#include "PVWDisplayAdapter.h"
#include "TgtBufQue.h"
#include "CameraProfile.h"
//
#include <cutils/properties.h>
//
using namespace android;
using namespace MtkCamUtils;
//


/******************************************************************************
*
*******************************************************************************/
#define ENABLE_LOG      (1)


/******************************************************************************
*
*******************************************************************************/
/*
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[PVWDisplayAdapter::%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[PVWDisplayAdapter::%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[PVWDisplayAdapter::%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[PVWDisplayAdapter::%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[PVWDisplayAdapter::%s] "fmt, __FUNCTION__, ##arg)
*/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s] "fmt, __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, arg...)    if (cond) { MY_LOGV(arg); }
#define MY_LOGD_IF(cond, arg...)    if (cond) { MY_LOGD(arg); }
#define MY_LOGI_IF(cond, arg...)    if (cond) { MY_LOGI(arg); }
#define MY_LOGW_IF(cond, arg...)    if (cond) { MY_LOGW(arg); }
#define MY_LOGE_IF(cond, arg...)    if (cond) { MY_LOGE(arg); }
//
#define PVW_DISP_NATIVE_3D_SUPPORT      "Camera.Native.3D.Support"
#define PVW_DISP_NATIVE_3D_SUPPORT_N    "0"
#define PVW_DISP_NATIVE_3D_SUPPORT_Y    "1"

/******************************************************************************
*
*******************************************************************************/
PVWDisplayAdapter::
PVWDisplayAdapter()
    : IDisplayAdapter()
    //
    , mpPreviewWindow(NULL)
    , mi4HalPixelFormat(0)
    , mi4WndWidth(0)
    , mi4WndHeight(0)
    //
    , mi4MaxFrameCount(0)
    //
    , mpTgtBufQue()
    //
    , mMutex()
    , mIsDispEnabled(false)
    //
    , mpFrameProvider(NULL)
    //
    , mIsRecordingHint(false)
    , mIsNative3D(false)
{
    MY_LOGD("tid(%d)", ::gettid());
}


/******************************************************************************
*
*******************************************************************************/
PVWDisplayAdapter::
~PVWDisplayAdapter()
{
    MY_LOGD("tid(%d)", ::gettid());
    mpFrameProvider = NULL;
}


/******************************************************************************
*
*******************************************************************************/
bool
PVWDisplayAdapter::
init()
{
    status_t status = NO_ERROR;
    //
    Mutex::Autolock _l(mMutex);
    //
    //
    mpTgtBufQue = new TgtBufQue;
    if  ( mpTgtBufQue == 0 )
    {
        MY_LOGE("Cannot new a TgtBufQue");
        status = NO_MEMORY;
        goto lbExit;
    }
    //
    status = mpTgtBufQue->run();
    if  ( NO_ERROR != status )
    {
        MY_LOGE("Cannot run TgtBufQue: %s (%d)", ::strerror(-status), -status);
        goto lbExit;
    }

lbExit:
    MY_LOGD("- status(%d)", status);
    return  (NO_ERROR==status);
}


/******************************************************************************
* Uninitialize the display adapter.
*******************************************************************************/
bool
PVWDisplayAdapter::
uninit()
{
    MY_LOGI("+");
    //
    disableDisplay(true);
    //
    Mutex::Autolock _l(mMutex);
    //
    if  ( mpTgtBufQue != 0 )
    {
        //  Notes:
        //  requestExitAndWait() in ICS has bugs. Use requestExit()/join() instead.
        mpTgtBufQue->requestExit();
        mpTgtBufQue->join();
        MY_LOGD("join() exit");
        mpTgtBufQue.clear();
    }
    //
    mpFrameProvider = NULL;
    //
    MY_LOGI("-");
    return  true;
}


/******************************************************************************
*
*******************************************************************************/
bool
PVWDisplayAdapter::
setWindow(
    preview_stream_ops*const window, 
    int32_t const   wndWidth, 
    int32_t const   wndHeight, 
    int32_t const   i4HalPixelFormat, 
    int32_t const   maxFrameCount
)
{
    MY_LOGI(
        "+ window(%p), WxH=%dx%d, format=0x%x(%s)", 
        window, wndWidth, wndHeight, 
        i4HalPixelFormat, MtkCamUtils::CamFmtMap::inst().keyFor(i4HalPixelFormat).string()
    );
    //
    return  setWindowInfo(window, wndWidth, wndHeight, i4HalPixelFormat, maxFrameCount)
        &&  setWindow(window)
            ;
}

bool
PVWDisplayAdapter::
resetPreviewFmt(
    int32_t const   wndWidth, 
    int32_t const   wndHeight, 
    int32_t const   i4HalPixelFormat 
)
{
#if 0
    if  (
        i4HalPixelFormat   != mi4HalPixelFormat
    ||  wndWidth           != mi4WndWidth
    ||  wndHeight          != mi4WndHeight
    )
    {
	    MY_LOGI(
	        "reset preview frame format, WxH=%dx%d, format=0x%x(%s)", 
	        wndWidth, wndHeight, 
	        i4HalPixelFormat, MtkCamUtils::CamFmtMap::inst().keyFor(i4HalPixelFormat).string()
	    );
	    
	    if(!setWindow(mpPreviewWindow, wndWidth, wndHeight, i4HalPixelFormat, mi4MaxFrameCount))
	        return false;
	
	    if (mpTgtBufQue != NULL)
	        mpTgtBufQue->prepareQueueAsync();
    }
#endif
    return true;
}
/******************************************************************************
*
*******************************************************************************/
bool
PVWDisplayAdapter::
setWindowInfo(
    preview_stream_ops*const window,
    int32_t const   wndWidth, 
    int32_t const   wndHeight, 
    int32_t const   i4HalPixelFormat, 
    int32_t const   maxFrameCount
)
{
    Mutex::Autolock _l(mMutex);
    //
    mi4HalPixelFormat   = i4HalPixelFormat;
    //
    mi4WndWidth         = wndWidth;
    mi4WndHeight        = wndHeight;
    //
    mi4MaxFrameCount    = maxFrameCount;
    //
    mpPreviewWindow = window;
    //
    return  true;
}


/******************************************************************************
*
*******************************************************************************/
bool
PVWDisplayAdapter::
setWindow(preview_stream_ops*const window)
{
    AutoCPTLog cptlog(Event_DispAdpt_setWindow);
    CamProfile profile(__FUNCTION__, "PVWDisplayAdapter");

    MY_LOGI("+ window(%p)", window);
    //
    bool ret = false;
    //
    //  [1] Check to see whether the passed window is valid or not.
    if  ( ! window )
    {
        MY_LOGW("NULL window passed to DisplayAdapter");
        return  false;
    }
    //
    //  [2]
    Mutex::Autolock _l(mMutex);
    //
    //  [3] Destroy the existing window object, if it exists
    CPTLogStr(Event_DispAdpt_setWindow, CPTFlagSeparator, "onDestroy");
    onDestroy();
    //
    //  [4.1]
    if  ( mpTgtBufQue == 0 )
    {
        MY_LOGE("NULL mpTgtBufQue");
        ret = false;
        goto lbExit;
    }
    //
    //  [4.2]
    CPTLogStr(Event_DispAdpt_setWindow, CPTFlagSeparator, "TgtBufQue setWindow");
    ret = mpTgtBufQue->setWindow(
            window, mi4WndWidth, mi4WndHeight, 
            HAL_PIXEL_FORMAT_YV12, 
//            mi4HalPixelFormat, 
            mi4MaxFrameCount
        );
    if  ( ! ret )
    {
        goto lbExit;
    }
    //
    //
    mpPreviewWindow = window;
    //
    ret = true;
lbExit:
    profile.print();
    return  ret;
}


/******************************************************************************
*
*******************************************************************************/
bool
PVWDisplayAdapter::
enableDisplay(bool const isRecordingHint)
{
    AutoCPTLog cptlog(Event_DispAdpt_enableDisplay);
    bool    ret = false;
    char    value_3d[32] = {'\0'};
    
    if  ( ! setWindow(mpPreviewWindow) )
    {
        return ret;
    }
    //
    Mutex::Autolock _l(mMutex);
    //
    MY_LOGD("+ mIsDispEnabled(%d), isRecordingHint(%d)", mIsDispEnabled, isRecordingHint);
    //
    //  [1] Prepare Target Buffer Queue.
    if  ( ! mpTgtBufQue->prepareQueue() )
    {
        goto lbExit;
    }
    //
    //  [2.1]
    if  ( mpFrameProvider == 0 )
    {
        MY_LOGE("NULL Frame Provider");
        goto lbExit;
    }
    //  [2.2] Enable frame notification.
    mpFrameProvider->enableFrameNotification(CAMERA_MSG_PREVIEW_FRAME | CAMERA_MSG_POSTVIEW_FRAME);
    //
    //  [3]
    mIsDispEnabled = true;
    mIsRecordingHint = isRecordingHint;
    //
#ifdef  MTK_NATIVE_3D_SUPPORT
    //  [4] 
    property_get(PVW_DISP_NATIVE_3D_SUPPORT, value_3d, PVW_DISP_NATIVE_3D_SUPPORT_N);
    mIsNative3D = (bool)(atoi(value_3d));
    MY_LOGD("PVW_DISP_NATIVE_3D_SUPPORT(%d)", mIsNative3D);
#endif
    //
    ret = true;
lbExit:
    MY_LOGD("- ret(%d)", ret);
    return  ret;
}


/******************************************************************************
*
*******************************************************************************/
bool
PVWDisplayAdapter::
disableDisplay(bool const cancel_buffer /*= true*/)
{
    Mutex::Autolock _l(mMutex);
    //
    MY_LOGD("+ mIsDispEnabled(%d), cancel_buffer(%d)", mIsDispEnabled, cancel_buffer);
    //
    if  ( ! mIsDispEnabled )
    {
        MY_LOGD("Display is already disabled");
        goto lbExit;
    }
    //
    //  Disable frame notification.
    mpFrameProvider->disableFrameNotification(CAMERA_MSG_PREVIEW_FRAME | CAMERA_MSG_POSTVIEW_FRAME);
    //
    //
    mIsDispEnabled = false;
    //
    if  ( cancel_buffer )
    {
        onDestroy();
    }
    //
lbExit:
    return  true;
}


/******************************************************************************
*
*******************************************************************************/
bool
PVWDisplayAdapter::
onDestroy()
{
    MY_LOGD("+");
    //
    bool ret = true;
    //
    if  ( mpTgtBufQue != 0 )
    {
        ret = mpTgtBufQue->cleanupQueue();
    }
    //
    MY_LOGD("- ret(%d)", ret);
    return  ret;
}


/******************************************************************************
*
*******************************************************************************/
bool
PVWDisplayAdapter::
setFrameProvider(FrameNotifier*const frameNotifier)
{
    bool ret = false;
    //
    if  ( ! frameNotifier )
    {
        MY_LOGW("NULL frameNotifier");
        goto lbExit;
    }

    //  Release any previous frame providers.
    mpFrameProvider = NULL;

    //
    MY_LOGD("(frameNotifier, PVWDisplayAdapter, onFrameCallback)=(%p, %p, %p)", frameNotifier, this, onFrameCallback);
    mpFrameProvider = new FrameProvider(frameNotifier, this, onFrameCallback);

    ret = true;
lbExit:
    return  ret;
}


/******************************************************************************
*
*******************************************************************************/
void
PVWDisplayAdapter::
onFrameCallback(CamFrame*const pCamFrame)
{
    MY_LOGV_IF(0, "+ pCamFrame(%p)", pCamFrame);
    //
    if  ( ! pCamFrame )
    {
        MY_LOGW("NULL Camera Frame");
        return;
    }
    //
    if  ( PVWDisplayAdapter* da = reinterpret_cast<PVWDisplayAdapter*>(pCamFrame->mCbCookie) )
    {
        da->onFrame(pCamFrame);
    }
    else
    {
        MY_LOGW("Invalid Callback Cookie in Camera Frame = %p, CbCookie = %p", pCamFrame, pCamFrame->mCbCookie);
        return;
    }
}


/******************************************************************************
*
*******************************************************************************/
void
PVWDisplayAdapter::
onFrame(CamFrame*const pCamFrame)
{
    if  ( pCamFrame->mBufCookie )
    {
        DispFrame df;
        //
        IImageBufferPool*const pImgBufPool = reinterpret_cast<IImageBufferPool*>(pCamFrame->mBufCookie);
        df.mType            = pCamFrame->mType;
        df.mBuf             = pImgBufPool->getVirAddr(pCamFrame->mBufIndex);
        df.mi4HalPixelFormat= MtkCamUtils::CamFmtMap::inst().valueFor(pImgBufPool->getImgFormat());
        df.mWidth           = pImgBufPool->getImgWidth();
        df.mHeight          = pImgBufPool->getImgHeight();
        df.mLength          = pImgBufPool->getBufSize();
        df.mTimestamp       = pCamFrame->mTimestamp;
        postFrame(df);
    }
    else
    {
        MY_LOGW("NULL mBufCookie for CamFrame(%p)", pCamFrame);
    }
}


/******************************************************************************
*
*******************************************************************************/
bool
PVWDisplayAdapter::
postFrame(DispFrame& rFrame)
{
    AutoCPTLog cptlog(Event_DispAdpt_postFrame);

    static int64_t i8Timestamp1_ = rFrame.mTimestamp;
    int64_t const  i8Timestamp2_ = rFrame.mTimestamp;
    int64_t const  i8TimestampDiff_= i8Timestamp2_ - i8Timestamp1_;
    //
    static int64_t i8Timestamp1 = MtkCamUtils::getTimeInMs();
    int64_t const  i8Timestamp2 = MtkCamUtils::getTimeInMs();
    int64_t const  i8TimestampDiff = i8Timestamp2 - i8Timestamp1;
    //
    MY_LOGD_IF(
        ENABLE_LOG, 
        "+ %s(%lld/%lld/%lld) %s(%lld/%lld/%lld)", 
        //
        (i8TimestampDiff > 34) ? "Post Interval > 34 ms" : "", 
        i8Timestamp1, i8Timestamp2, i8TimestampDiff, 
        //
        (i8TimestampDiff_<0) ? "timestamp inversion" : "", 
        i8Timestamp1_/1000000, i8Timestamp2_/1000000, i8TimestampDiff_/1000000
    );
    //
    i8Timestamp1_ = i8Timestamp2_;
    i8Timestamp1 = i8Timestamp2;
    //
    bool ret = false;
    //
    if  ( ! rFrame.mBuf )
    {
        MY_LOGW("NULL sent to postFrame");
        goto lbExit;
    }
    else
    {
        TgtBufQue::BufInfo currTgtBuf;
        //
        //
        if  (
                rFrame.mWidth           != mi4WndWidth
            ||  rFrame.mHeight          != mi4WndHeight
#if 0
            ||  rFrame.mi4HalPixelFormat!= mi4HalPixelFormat
#endif
            )
        {
            MY_LOGI("Window geometry != frame geometry; reset the window");
            MY_LOGI("Window geometry: WxH=%dx%d, format=%s", mi4WndWidth, mi4WndHeight, MtkCamUtils::CamFmtMap::inst().keyFor(mi4HalPixelFormat).string());
            MY_LOGI("Frame geometry: WxH=%dx%d, format=%s", rFrame.mWidth, rFrame.mHeight, MtkCamUtils::CamFmtMap::inst().keyFor(rFrame.mi4HalPixelFormat).string());
            if  ( ! setWindow(mpPreviewWindow, rFrame.mWidth, rFrame.mHeight, rFrame.mi4HalPixelFormat, mi4MaxFrameCount) )
            {
                goto lbExit;
            }
        }
        //
        //
        Mutex::Autolock _l(mMutex);
        //
        if  ( ! mIsDispEnabled || mpTgtBufQue == 0 )
        {
            MY_LOGW("do nothing since display is already disabled - (mIsDispEnabled, mpTgtBufQue.get)=(%d, %p)", mIsDispEnabled, mpTgtBufQue.get());
            goto lbExit;
        }
        //
        //  Get the Display Target Buffer.
        if  ( ! mpTgtBufQue->dequeueBuffer(currTgtBuf) )
        {
            MY_LOGW("dequeueBuffer fail, break operation");
            goto lbExit;
        }
        //
        //  Frame --> Display Target Buffer.
        MY_LOGD_IF(0, "disp buf(%p) <-- frame buf(%p), length(%d)", currTgtBuf.getBuf(), rFrame.mBuf, rFrame.mLength);
        copyFrame(currTgtBuf.getStride(), currTgtBuf.getBuf(), rFrame);
        mpTgtBufQue->enqueueBuffer(currTgtBuf, rFrame.mTimestamp);
        //
        mpTgtBufQue->prepareQueueAsync();
    }
    //
    ret = true;
lbExit:
    mpFrameProvider->returnFrame(rFrame.mType, rFrame.mBuf);
    MY_LOGD_IF(ENABLE_LOG, "- ret(%d)", ret);
    return  ret;
}

