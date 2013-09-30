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

#define LOG_TAG "MtkCam/ClientCallback"
//
#include <camera/MtkCameraParameters.h>
//
#include "Utils/inc/CamUtils.h"
#include "inc/ClientCallback.h"
//
using namespace android;


/******************************************************************************
*
*******************************************************************************/
#define ENABLE_LOG      (0)


/******************************************************************************
*
*******************************************************************************/
#define MY_LOGV(fmt, arg...)            CAM_LOGV("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)            CAM_LOGD("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)            CAM_LOGI("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)            CAM_LOGW("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)            CAM_LOGE("[%s] "fmt, __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, arg...)        if (cond) { MY_LOGV(arg); }
#define MY_LOGD_IF(cond, arg...)        if (cond) { MY_LOGD(arg); }
#define MY_LOGI_IF(cond, arg...)        if (cond) { MY_LOGI(arg); }
#define MY_LOGW_IF(cond, arg...)        if (cond) { MY_LOGW(arg); }
#define MY_LOGE_IF(cond, arg...)        if (cond) { MY_LOGE(arg); }


/******************************************************************************
*
*******************************************************************************/
#define DEBUG_DUMP_PREVIEW_FRAME_TO_FILE        (0) // n-th frame to write
#define DEBUG_DUMP_POSTVIEW_FRAME_TO_FILE       (0)


/******************************************************************************
*
*******************************************************************************/
namespace
{
#if DEBUG_DUMP_PREVIEW_FRAME_TO_FILE
    uint_t      g_uDbgPreviewFrameCount = 0;
#endif
}


/******************************************************************************
*
*******************************************************************************/
ClientCallback::
ClientCallback()
    : RefBase()
    //
    , mMtxLock()
    //
    , mpBufProvider(NULL)
    , mpFrameProvider(NULL)
    //
    , mMsgEnabled()
    , mClientCbCookie(NULL)
    , mNotifyCb(NULL)
    , mDataCb(NULL)
    , mDataCbTimestamp(NULL)
    //
    , ms8PreviewTgtFmt("")
    , ms8PreviewSrcFmt("")
    , mi4PreviewWidth(0)
    , mi4PreviewHeight(0)
    , mpPreviewBufPool()
    , muPreviewBufsIdx(0)
    //
    , mi4PreviewCbRefCount(0)
    , mi8PreviewCbTimeInMs(0)
    //
{
    MY_LOGD("+ this(%p)", this);
}


/******************************************************************************
*
*******************************************************************************/
ClientCallback::
~ClientCallback()
{
    MY_LOGD("- tid(%d)", ::gettid());
}


/******************************************************************************
*
*******************************************************************************/
bool
ClientCallback::
init()
{
    MY_LOGI("+ tid(%d)", ::gettid());
    //
    return  true;
}


/******************************************************************************
*
*******************************************************************************/
bool
ClientCallback::
uninit()
{
    MY_LOGI("+ tid(%d), getStrongCount(%d)", ::gettid(), getStrongCount());
    //
    Mutex::Autolock _l(mMtxLock);
    //
    //  Release any previous frame providers.
    mpFrameProvider = NULL;
    //
    if  ( 0 != mi4PreviewCbRefCount )
    {
        int64_t const i8CurrentTimeInMs = MtkCamUtils::getTimeInMs();
        MY_LOGW(
            "Preview Callback: ref count(%d)!=0, the last callback before %lld ms, timestamp:(the last, current)=(%lld ms, %lld ms)", 
            mi4PreviewCbRefCount, (i8CurrentTimeInMs-mi8PreviewCbTimeInMs), mi8PreviewCbTimeInMs, i8CurrentTimeInMs
        );
    }
    //
    MY_LOGI("- tid(%d), getStrongCount(%d)", ::gettid(), getStrongCount());
    return  true;
}


/******************************************************************************
* Set the notification and data callbacks
*******************************************************************************/
void
ClientCallback::
setCallbacks(
    camera_notify_callback notify_cb,
    camera_data_callback data_cb,
    camera_data_timestamp_callback data_cb_timestamp,
    void *user
)
{
    //
    Mutex::Autolock _l(mMtxLock);
    //
    mNotifyCb       = notify_cb;
    mDataCb         = data_cb;
    mDataCbTimestamp= data_cb_timestamp;
    mClientCbCookie = user;
}


/******************************************************************************
* Enable a message, or set of messages.
*******************************************************************************/
void
ClientCallback::
enableMsgType(int32_t msgType)
{
    MY_LOGD("+ mMsgEnabled = 0x%08x, msgType = 0x%08x", mMsgEnabled, msgType);
    ::android_atomic_or(msgType, &mMsgEnabled);
}


/******************************************************************************
* Disable a message, or a set of messages.
*
* Once received a call to disableMsgType(CAMERA_MSG_VIDEO_FRAME), camera hal
* should not rely on its client to call releaseRecordingFrame() to release
* video recording frames sent out by the cameral hal before and after the
* disableMsgType(CAMERA_MSG_VIDEO_FRAME) call. Camera hal clients must not
* modify/access any video recording frame after calling
* disableMsgType(CAMERA_MSG_VIDEO_FRAME).
*******************************************************************************/
void
ClientCallback::
disableMsgType(int32_t msgType)
{
    MY_LOGD("+ mMsgEnabled = 0x%08x, msgType = 0x%08x", mMsgEnabled, msgType);
    ::android_atomic_and(~msgType, &mMsgEnabled);
}


/******************************************************************************
* Query whether a message, or a set of messages, is enabled.
* Note that this is operates as an AND, if any of the messages
* queried are off, this will return false.
*******************************************************************************/
bool
ClientCallback::
msgTypeEnabled(int32_t msgType)
{
    return  msgType == ( msgType & mMsgEnabled );
}


/******************************************************************************
* 
*******************************************************************************/
void
ClientCallback::
setBufferProvider(IBufferProvider*const pBufferProvider)
{
    //
    Mutex::Autolock _l(mMtxLock);
    //
    mpBufProvider = pBufferProvider;
}


/******************************************************************************
*
*******************************************************************************/
bool
ClientCallback::
setFrameProvider(FrameNotifier*const frameNotifier)
{
    bool ret = false;
    //
    if  ( ! frameNotifier )
    {
        MY_LOGW("NULL frameNotifier");
        goto lbExit;
    }
    else
    {
        Mutex::Autolock _l(mMtxLock);
        //
        //  Release any previous frame providers.
        mpFrameProvider = NULL;
        //
        MY_LOGD("(frameNotifier, ClientCallback, onFrameCallback)=(%p, %p, %p)", frameNotifier, this, onFrameCallback);
        mpFrameProvider = new FrameProvider(frameNotifier, this, onFrameCallback);
        //
        if  ( mpFrameProvider == 0 )
        {
            MY_LOGE("cannot new FrameProvider");
            goto lbExit;
        }
        //
        ret = true;
    }

lbExit:
    return  ret;
}


/******************************************************************************
*
*******************************************************************************/
sp<FrameProvider>
ClientCallback::
promoteFrameProvider()  const
{
    Mutex::Autolock _l(mMtxLock);
    return  mpFrameProvider;
}


/******************************************************************************
*
*******************************************************************************/
bool
ClientCallback::
startPreview(
    String8 const&  rs8PreviewTargetFormat, 
    String8 const&  rs8PreviewSourceFormat, 
    int32_t const   i4PreviewWidth, 
    int32_t const   i4PreviewHeight
)
{
    MY_LOGD("+ preview format:(target/source)=(%s/%s), WxH=%dx%d", rs8PreviewTargetFormat.string(), rs8PreviewSourceFormat.string(), i4PreviewWidth, i4PreviewHeight);
    //
#if DEBUG_DUMP_PREVIEW_FRAME_TO_FILE
    MY_LOGD("DEBUG_DUMP_PREVIEW_FRAME_TO_FILE = %d", DEBUG_DUMP_PREVIEW_FRAME_TO_FILE);
    g_uDbgPreviewFrameCount = 0;
#endif
    //
    if  (
            0 >= i4PreviewWidth
        ||  0 >= i4PreviewHeight
        ||  rs8PreviewTargetFormat.isEmpty()
        ||  rs8PreviewSourceFormat.isEmpty()
        )
    {
        MY_LOGE("bad arguments");
        return  false;
    }
    //
    Mutex::Autolock _l(mMtxLock);
    //
    if  (
            mpPreviewBufPool        == 0
        ||  rs8PreviewTargetFormat  != ms8PreviewTgtFmt
        ||  rs8PreviewSourceFormat  != ms8PreviewSrcFmt
        ||  i4PreviewWidth          != mi4PreviewWidth
        ||  i4PreviewHeight         != mi4PreviewHeight
        )
    {
        mpPreviewBufPool.clear();
        //
        bool ret = mpBufProvider->allocBuffer(
                        mpPreviewBufPool, rs8PreviewTargetFormat, 
                        i4PreviewWidth, i4PreviewHeight, 
                        eMAX_PREVIEW_BUFFER_NUM, "PreviewClientCb"
                    );
        if  ( ! ret || mpPreviewBufPool == 0 )
        {
            MY_LOGE("cannot allocate mpPreviewBufPool");
            return  false;
        }
    }
    muPreviewBufsIdx    = 0;
    ms8PreviewTgtFmt    = rs8PreviewTargetFormat;
    ms8PreviewSrcFmt    = rs8PreviewSourceFormat;
    mi4PreviewWidth     = i4PreviewWidth;
    mi4PreviewHeight    = i4PreviewHeight;
    //
    //
    MY_LOGD("Enable CAMERA_MSG_PREVIEW_FRAME");
    mpFrameProvider->enableFrameNotification(CAMERA_MSG_PREVIEW_FRAME);

    return  true;
}


/******************************************************************************
*
*******************************************************************************/
bool
ClientCallback::
stopPreview()
{
    MY_LOGD("+");
    //
    Mutex::Autolock _l(mMtxLock);
    //
    //
    MY_LOGD("Disable CAMERA_MSG_PREVIEW_FRAME");
    mpFrameProvider->disableFrameNotification(CAMERA_MSG_PREVIEW_FRAME);
    //
    //
    if  ( mpPreviewBufPool != 0 )
    {
        MY_LOGD("Release Preview Callback Memory");
        //  TODO: should we make sure all callback memorys are returned from applications before releasing them?
        mpPreviewBufPool.clear();
        muPreviewBufsIdx = 0;
    }
    //
    //
    return  true;
}


/******************************************************************************
*
*******************************************************************************/
void
ClientCallback::
onFrameCallback(CamFrame*const pCamFrame)
{
    MY_LOGV_IF(ENABLE_LOG, "+ pCamFrame(%p)", pCamFrame);
    //
    if  ( ! pCamFrame )
    {
        MY_LOGW("NULL Camera Frame");
        return;
    }
    //
    sp<ClientCallback> handler = reinterpret_cast<ClientCallback*>(pCamFrame->mCbCookie);
    if  ( handler != 0 && handler->getStrongCount() > 0 )
    {
        int32_t const i4StrongCountBeforeCB = handler->getStrongCount();
        handler->onFrame(pCamFrame);
        int32_t const i4StrongCountAfterCB = handler->getStrongCount();
        if  ( 1 == i4StrongCountAfterCB )
        {
            MY_LOGI(
                "tid(%d): After callback, ClientCallback(%p) StrongCount: %d --> 1", 
                ::gettid(), handler.get(), i4StrongCountBeforeCB
            );
        }
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
ClientCallback::
onFrame(CamFrame*const pCamFrame)
{
    if  ( pCamFrame->mBufCookie )
    {
        switch  ( pCamFrame->mType )
        {
        case CAMERA_MSG_PREVIEW_FRAME:
            handlePreviewFrame(pCamFrame);
            break;
        case CAMERA_MSG_POSTVIEW_FRAME:
            handlePostviewFrame(pCamFrame);
            break;
        default:
            MY_LOGW("Unsupported frame type(0x%x)", pCamFrame->mType);
            break;
        }
    }
    else
    {
        MY_LOGW("NULL mBufCookie for CamFrame(%p)", pCamFrame);
    }
}


/******************************************************************************
*
*******************************************************************************/
void
ClientCallback::
handlePreviewFrame(CamFrame*const pCamFrame)
{
    MY_LOGV_IF(ENABLE_LOG, "+ CAMERA_MSG_PREVIEW_FRAME(0x%x), mMsgEnabled(0x%x)", CAMERA_MSG_PREVIEW_FRAME, mMsgEnabled);
    //
    sp<IImageBufferPool> const pImgBufPool = reinterpret_cast<IImageBufferPool*>(pCamFrame->mBufCookie);
    //
#if DEBUG_DUMP_PREVIEW_FRAME_TO_FILE
    #warning "[Notice] DEBUG_DUMP_PREVIEW_FRAME_TO_FILE"
    if  ( 0 == (++g_uDbgPreviewFrameCount % DEBUG_DUMP_PREVIEW_FRAME_TO_FILE) )
    {
        String8 s8FilePath("/data/PreviewSource");
        s8FilePath.appendFormat(
            "_%s_%dx%d_%d.yuv", 
            pImgBufPool->getImgFormat().string(), 
            pImgBufPool->getImgWidth(), 
            pImgBufPool->getImgHeight(), 
            g_uDbgPreviewFrameCount
        );
        MtkCamUtils::saveBufToFile(s8FilePath, 
            (uint8_t*)pImgBufPool->getVirAddr(pCamFrame->mBufIndex), 
            pImgBufPool->getBufSize()
        );
    }
#endif
    //
    //
    sp<FrameProvider> pFrameProvider = promoteFrameProvider();
    if  ( pFrameProvider == 0 )
    {
        MY_LOGW("Frame Provider is released.");
        return;
    }
    //
    if  ( msgTypeEnabled(CAMERA_MSG_PREVIEW_FRAME) )
    {
        convertAndSendPreviewFrame(pCamFrame, CAMERA_MSG_PREVIEW_FRAME);
    }
    else
    {
        //  Since preview callback is not enabled by clients, we just return the frame to the provider.
        MY_LOGV_IF(ENABLE_LOG, "CAMERA_MSG_PREVIEW_FRAME is not enabled by clients.");
    }
    //
    pFrameProvider->returnFrame(CAMERA_MSG_PREVIEW_FRAME, pImgBufPool->getVirAddr(pCamFrame->mBufIndex));
}


/******************************************************************************
*
*******************************************************************************/
void
ClientCallback::
handlePostviewFrame(CamFrame*const pCamFrame)
{
    MY_LOGV("+ CAMERA_MSG_POSTVIEW_FRAME(0x%x), mMsgEnabled(0x%x)", CAMERA_MSG_POSTVIEW_FRAME, mMsgEnabled);
    //
    sp<IImageBufferPool> const pImgBufPool = reinterpret_cast<IImageBufferPool*>(pCamFrame->mBufCookie);
    //
#if DEBUG_DUMP_POSTVIEW_FRAME_TO_FILE
    #warning "[Notice] DEBUG_DUMP_POSTVIEW_FRAME_TO_FILE"
    String8 s8FilePath("/data/Postview");
    s8FilePath.appendFormat(
        "_%s_%dx%d", 
        pImgBufPool->getImgFormat().string(), 
        pImgBufPool->getImgWidth(), 
        pImgBufPool->getImgHeight()
    );
    //
    MtkCamUtils::saveBufToFile(s8FilePath, 
        (uint8_t*)pImgBufPool->getVirAddr(pCamFrame->mBufIndex), 
        pImgBufPool->getBufSize()
    );
#endif
    //
    //
    sp<FrameProvider> pFrameProvider = promoteFrameProvider();
    if  ( pFrameProvider == 0 )
    {
        MY_LOGW("Frame Provider is released.");
        return;
    }
    //
    if  ( msgTypeEnabled(CAMERA_MSG_POSTVIEW_FRAME) )
    {
        convertAndSendPreviewFrame(pCamFrame, CAMERA_MSG_POSTVIEW_FRAME);
    }
    else
    {
        //  Since postview callback is not enabled by clients, we just return the frame to the provider.
        MY_LOGV("CAMERA_MSG_POSTVIEW_FRAME is not enabled by clients.");
    }
    //
    pFrameProvider->returnFrame(CAMERA_MSG_POSTVIEW_FRAME, pImgBufPool->getVirAddr(pCamFrame->mBufIndex));
}


/******************************************************************************
*
*******************************************************************************/
void
ClientCallback::
convertAndSendPreviewFrame(CamFrame*const pCamFrame, int32_t const msgType)
{
    CamProfile profile(__FUNCTION__, "ClientCallback");
    //
    IImageBufferPool*const pImgBufPool = reinterpret_cast<IImageBufferPool*>(pCamFrame->mBufCookie);
    //
    //
    //  Check to see whether the target and source formats are the same or not.
    if  ( ms8PreviewTgtFmt == pImgBufPool->getImgFormat() )
    {
        //  Since both preview formats of target and source are the same, 
        //  we just perform callback to applications without converting the format.
        MY_LOGV("Both preview target & source formats are the same: %s", ms8PreviewTgtFmt.string());
        //
        ::android_atomic_inc(&mi4PreviewCbRefCount);
        mi8PreviewCbTimeInMs = MtkCamUtils::getTimeInMs();
        mDataCb(msgType, pImgBufPool->get_camera_memory(), pCamFrame->mBufIndex, NULL, mClientCbCookie);
        ::android_atomic_dec(&mi4PreviewCbRefCount);
        profile.print_overtime(10, "mDataCb(%x) directly - index(%d) format(%s)", msgType, pCamFrame->mBufIndex, ms8PreviewTgtFmt.string());
        return;
    }
    //
    //--------------------------------------------------------------------------
    //
    //  Here the target format and source format are different. so we need to 
    //  convert the formats before performing callback to clients.
    //
    //--------------------------------------------------------------------------
    //
    //  [0] hold a strong pointer to the pool before using them & callback.
    sp<IImageBufferPool> pPreviewBufPool = mpPreviewBufPool;
    if  ( pPreviewBufPool == 0 )
    {
        MY_LOGW(
            "No buffer pool available for format conversion: %s -> %s", 
            pImgBufPool->getImgFormat().string(), ms8PreviewTgtFmt.string()
        );
        return;
    }
    //
    //  Source: I420
    if  ( pImgBufPool->getImgFormat() == MtkCameraParameters::PIXEL_FORMAT_YUV420I )
    {
        uint_t  uCallbackBufIdx = 0;
        //
        //  [1] get the available buffer index.
        //  TODO: make sure the current index is available.
        {
            Mutex::Autolock _l(mMtxLock);
            //
            uCallbackBufIdx = muPreviewBufsIdx;
            //  Iterate to the next buffer.
            muPreviewBufsIdx = (muPreviewBufsIdx + 1) % eMAX_PREVIEW_BUFFER_NUM;
        }
        //
        //  [2] Format Converting: check target format
        if  ( ms8PreviewTgtFmt == CameraParameters::PIXEL_FORMAT_YUV420SP )
        {   //  Source(I420) --> Target(NV21)
            convertYuv420iToYuv420sp(
                reinterpret_cast<uint8_t*>(pImgBufPool->getVirAddr(pCamFrame->mBufIndex)), 
                reinterpret_cast<uint8_t*>(pPreviewBufPool->getVirAddr(uCallbackBufIdx)), 
                pPreviewBufPool->getImgWidth(), 
                pPreviewBufPool->getImgHeight()
            );
        }
        else if ( ms8PreviewTgtFmt == CameraParameters::PIXEL_FORMAT_YUV420P )
        {   //  Source(I420) --> Target(YV12)
            convertYuv420iToYuv420p(
                reinterpret_cast<uint8_t*>(pImgBufPool->getVirAddr(pCamFrame->mBufIndex)), 
                reinterpret_cast<uint8_t*>(pPreviewBufPool->getVirAddr(uCallbackBufIdx)), 
                pPreviewBufPool->getImgWidth(), 
                pPreviewBufPool->getImgHeight()
            );
        }
        else
        {
            MY_LOGW("now we don't support format converting: %s -> %s", pImgBufPool->getImgFormat().string(), ms8PreviewTgtFmt.string());
        }

        //
        //  [3] Client callback
        ::android_atomic_inc(&mi4PreviewCbRefCount);
        mi8PreviewCbTimeInMs = MtkCamUtils::getTimeInMs();
        mDataCb(msgType, pPreviewBufPool->get_camera_memory(), uCallbackBufIdx, NULL, mClientCbCookie);
        ::android_atomic_dec(&mi4PreviewCbRefCount);
        profile.print_overtime(10, "mDataCb(%x) - index(%d) formats(%s -> %s)", msgType, uCallbackBufIdx, pImgBufPool->getImgFormat().string(), ms8PreviewTgtFmt.string());

        //  [4] Dump for Debug if needed.
#if DEBUG_DUMP_PREVIEW_FRAME_TO_FILE
        #warning "[Notice] DEBUG_DUMP_PREVIEW_FRAME_TO_FILE"
        if  ( 0 == (g_uDbgPreviewFrameCount % DEBUG_DUMP_PREVIEW_FRAME_TO_FILE) )
        {
            String8 s8FilePath("/data/PreviewTarget");
            s8FilePath.appendFormat(
                "_%s_%dx%d_%d.yuv", 
                pPreviewBufPool->getImgFormat().string(), 
                pPreviewBufPool->getImgWidth(), 
                pPreviewBufPool->getImgHeight(), 
                g_uDbgPreviewFrameCount
            );
            MtkCamUtils::saveBufToFile(s8FilePath, 
                (uint8_t*)pPreviewBufPool->getVirAddr(uCallbackBufIdx), 
                pPreviewBufPool->getBufSize()
            );
        }
#endif
    }
    //--------------------------------------------------------------------------
    //  Unsupported
    else
    {
        MY_LOGW("now we don't support format converting: %s -> %s", pImgBufPool->getImgFormat().string(), ms8PreviewTgtFmt.string());
    }
}

