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
#include <camera/CameraParameters.h>
#include <camera/MtkCameraParameters.h>
#include <binder/IMemory.h>
#include <cutils/properties.h>
//
#include <camera/MtkCamera.h>
//
#include "Utils/inc/CamUtils.h"
//
#include "ClientCallback/inc/ClientCallback.h"
#include "CamAdapter/inc/ICamAdapter.h"
#include "DisplayAdapter/inc/IDisplayAdapter.h"
//
#include "inc/ICamDevice.h"
#include "inc/CamDevice.h"
#include "CamBufProvider.h"
#include "CameraProfile.h"
//
using namespace android::MtkCamUtils;


/******************************************************************************
*
*******************************************************************************/
#define MY_LOGV(fmt, arg...)    CAM_LOGV("[CamDevice::%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)    CAM_LOGD("[CamDevice::%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)    CAM_LOGI("[CamDevice::%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)    CAM_LOGW("[CamDevice::%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)    CAM_LOGE("[CamDevice::%s] "fmt, __FUNCTION__, ##arg)


/******************************************************************************
*
*******************************************************************************/
#define MAX_DISPLAY_FRAME_COUNT (3) //  FIXME:


/******************************************************************************
*
*******************************************************************************/
namespace android {


/******************************************************************************
*
*******************************************************************************/
CamDevice::
CamDevice(
    String8 const&          rDevName, 
    int32_t const           i4OpenId, 
    CamDevInfo const&       rDevInfo, 
    sp<ICamAdapter> const&  rpCamAdapter
)
    : ICamDevice()
    //
    , mDevName(rDevName)
    , mi4OpenId(i4OpenId)
    , mDevInfo(rDevInfo)
    //
    , mpBufProvider(NULL)
    , mpDisplayAdapter()
    , mpCamAdapter(rpCamAdapter)
    , mpClientCbHandler()
    , mrParameters(rpCamAdapter->getParameters())
    //
    , mIsPreviewEnabled(false)
    //
    , mMsgEnabled(0)
    , mCallbackCookie(NULL)
    , mNotifyCb(NULL)
    , mDataCb(NULL)
    , mDataCbTimestamp(NULL)
    , mfpRequestMemory(NULL)
{
    MY_LOGD("(%s:%d), (OpenId, DevId)=(%d, %d)", getDevName(), ::gettid(), getOpenId(), getDevId());
}


/******************************************************************************
*
*******************************************************************************/
CamDevice::
~CamDevice()
{
    MY_LOGD("(%s:%d), (OpenId, DevId)=(%d, %d)", getDevName(), ::gettid(), getOpenId(), getDevId());
    mpDisplayAdapter.clear();
    mpCamAdapter.clear();
    delete  mpBufProvider;
}


/******************************************************************************
*
*******************************************************************************/
bool
CamDevice::
onInit()
{
    MY_LOGI("+ (%s:%d), (OpenId, DevId)=(%d, %d)", getDevName(), ::gettid(), getOpenId(), getDevId());
    //
    bool    ret = false;
    //
    //
    mpBufProvider = new CamBufProvider;
    if  ( mpBufProvider == 0 )
    {
        MY_LOGE("fail to new CamBufProvider");
        goto lbExit;
    }
    //
    //
    if  ( mpCamAdapter != 0 && mpCamAdapter->init() )
    {
        mpCamAdapter->setBufferProvider(mpBufProvider);
    }
    else
    {
        MY_LOGE("mpCamAdapter(%p)->init() fail", mpCamAdapter.get());
        goto lbExit;
    }
    //
    //
    if  ( mpClientCbHandler == 0 )
    {
        mpClientCbHandler = new ClientCallback;
        if  ( mpClientCbHandler == 0 || ! mpClientCbHandler->init() )
        {
            MY_LOGE("NULL mpClientCbHandler");
            goto lbExit;
        }
    }
    mpClientCbHandler->setBufferProvider(mpBufProvider);
    mpClientCbHandler->setFrameProvider(mpCamAdapter.get());

    //
    //
    ret = true;
lbExit:
    if  ( ! ret )
    {
        onUninit();
    }
    MY_LOGI("- ret(%d)", ret);
    return  ret;
}


/******************************************************************************
*
*******************************************************************************/
bool
CamDevice::
onUninit()
{
    MY_LOGI("+ (%s:%d), (OpenId, DevId)=(%d, %d)", getDevName(), ::gettid(), getOpenId(), getDevId());
    //
    //
    if  ( mpClientCbHandler != 0 )
    {
        mpClientCbHandler->uninit();
        mpClientCbHandler.clear();
    }
    //
    //
    if  ( mpDisplayAdapter != 0 )
    {
        mpDisplayAdapter->uninit();
        mpDisplayAdapter.clear();
    }
    //
    //
    if  ( mpCamAdapter != 0 )
    {
        mpCamAdapter->uninit();
        mpCamAdapter.clear();
    }
    //
    //
    delete  mpBufProvider;
    mpBufProvider = NULL;
    //
    MY_LOGI("-");
    return  true;
}


/******************************************************************************
* Set the notification and data callbacks
*******************************************************************************/
void
CamDevice::
setCallbacks(
    camera_notify_callback notify_cb,
    camera_data_callback data_cb,
    camera_data_timestamp_callback data_cb_timestamp,
    camera_request_memory get_memory,
    void *user
)
{
    mNotifyCb       = notify_cb;
    mDataCb         = data_cb;
    mDataCbTimestamp= data_cb_timestamp;
    mfpRequestMemory= get_memory;
    mCallbackCookie = user;
    //
    mpBufProvider->setAllocator(get_memory);
    //
    if  ( mpCamAdapter != 0 )
    {
        mpCamAdapter->setCallbacks(notify_cb, data_cb, data_cb_timestamp, user);
    }
    //
    if  ( mpClientCbHandler != 0 )
    {
        mpClientCbHandler->setCallbacks(notify_cb, data_cb, data_cb_timestamp, user);
    }
}


/******************************************************************************
* Enable a message, or set of messages.
*******************************************************************************/
void
CamDevice::
enableMsgType(int32_t msgType)
{
    ::android_atomic_or(msgType, &mMsgEnabled);
    //
    if  ( mpCamAdapter != 0 )
    {
        mpCamAdapter->enableMsgType(msgType);
    }
    //
    if  ( mpClientCbHandler != 0 )
    {
        mpClientCbHandler->enableMsgType(msgType);
    }
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
CamDevice::
disableMsgType(int32_t msgType)
{
    ::android_atomic_and(~msgType, &mMsgEnabled);
    //
    if  ( mpCamAdapter != 0 )
    {
        mpCamAdapter->disableMsgType(msgType);
    }
    //
    if  ( mpClientCbHandler != 0 )
    {
        mpClientCbHandler->disableMsgType(msgType);
    }
}


/******************************************************************************
* Query whether a message, or a set of messages, is enabled.
* Note that this is operates as an AND, if any of the messages
* queried are off, this will return false.
*******************************************************************************/
bool
CamDevice::
msgTypeEnabled(int32_t msgType)
{
    return  msgType == ( msgType & mMsgEnabled );
}


/******************************************************************************
* Set the preview_stream_ops to which preview frames are sent.
*******************************************************************************/
status_t
CamDevice::
setPreviewWindow(preview_stream_ops* window)
{
    AutoCPTLog cptlog(Event_CamDev_setPreviewWindow);
    MY_LOGI("+ (%s:%d), window(%p)", getDevName(), ::gettid(), window);
    //
    status_t status = initDisplayAdapter(window);
    if  ( NO_ERROR == status && mpDisplayAdapter != 0 && previewEnabled() )
    {
        //  enable display
        if  ( ! mpDisplayAdapter->enableDisplay(queryRecordingHint()) )
        {
            status = INVALID_OPERATION;
        }
    }
    //
    return  status;
}


/******************************************************************************
* Start preview mode.
*******************************************************************************/
status_t
CamDevice::
startPreview()
{
    MY_LOGI("+ (%s:%d), (OpenId, DevId)=(%d, %d)", getDevName(), ::gettid(), getOpenId(), getDevId());
    //
    status_t status = NO_ERROR;
    //
    if  ( previewEnabled() )
    {
        MY_LOGD("Preview already running");
        status = ALREADY_EXISTS;
        return  status;
    }
    /*
    if  ( mpDisplayAdapter != 0 )
    {
        //  enable display
        if  ( ! mpDisplayAdapter->enableDisplay(queryRecordingHint()) )
        {
            status = INVALID_OPERATION;
            goto lbExit;
        }
    }
    else
    {
        MY_LOGD("DisplayAdapter is not ready.");
    }
    */
    if  ( mpCamAdapter == 0 )
    {
        MY_LOGE("NULL Camera Adapter");
        status = DEAD_OBJECT;
        goto lbExit;
    }
    //
    if  ( mpClientCbHandler != 0 )
    {
        String8 const s8PreviewTargetFormat( mrParameters.getPreviewFormat() );
        String8 const s8PreviewSourceFormat( mrParameters.get(MtkCameraParameters::KEY_PREVIEW_INT_FORMAT) );
        int32_t i4PreviewWidth = 0, i4PreviewHeight = 0;
        //
        //  Get preview size.
        if  ( ! queryPreviewSize(i4PreviewWidth, i4PreviewHeight) )
        {
            MY_LOGE("queryPreviewSize");
            status = DEAD_OBJECT;
            goto lbExit;
        }
        //
        mpClientCbHandler->startPreview(s8PreviewTargetFormat, s8PreviewSourceFormat, i4PreviewWidth, i4PreviewHeight);
    //TODO: Start the callback notifier
//    mAppCallbackNotifier->startPreviewCallbacks(mParameters, mPreviewBufs, mPreviewOffsets, mPreviewFd, mPreviewLength, required_buffer_count);
    }
    //
    //  allocate buffers and then pass them to Camera Adapter.
    //  startPreview in Camera Adapter.
    status = mpCamAdapter->startPreview();
    if  ( NO_ERROR != status )
    {
        MY_LOGE("startPreview() in CameraAdapter returns: %s (%d)", ::strerror(-status), -status);
        goto lbExit;
    }
    //
    if  ( mpDisplayAdapter != 0 )
    {
        //  enable display
        if  ( ! mpDisplayAdapter->enableDisplay(queryRecordingHint()) )
        {
            status = INVALID_OPERATION;
            goto lbExit;
        }
    }
    else
    {
        MY_LOGD("DisplayAdapter is not ready.");
    }
    //
    mIsPreviewEnabled = true;
    //
    enableMsgType(CAMERA_MSG_PREVIEW_METADATA);
    //
    status = NO_ERROR;
lbExit:
    if  ( NO_ERROR != status )
    {
        MY_LOGD("Cleanup after error");
        //
        if  ( mpDisplayAdapter != 0 )
        {
            mpDisplayAdapter->disableDisplay(false);
        }
    }
    //
    MY_LOGI("- status(%d)", status);
    return  status;
}


/******************************************************************************
* Stop a previously started preview.
*******************************************************************************/
void
CamDevice::
stopPreview()
{
    MY_LOGI("+ (%s:%d), (OpenId, DevId)=(%d, %d)", getDevName(), ::gettid(), getOpenId(), getDevId());

    // 
    disableMsgType(CAMERA_MSG_PREVIEW_METADATA);

    //
    if  ( ! previewEnabled() )
    {
        MY_LOGD("Preview already stopped, perhaps!");
        MY_LOGD("We still force to clean up again.");
    }
    //
    if  ( mpDisplayAdapter != 0 )
    {
        mpDisplayAdapter->disableDisplay();
    }
    //
    if  ( mpClientCbHandler != 0 )
    {
        mpClientCbHandler->stopPreview();
//  TODO:
//        //Stop the callback sending
//        mAppCallbackNotifier->stop();
//        mAppCallbackNotifier->flushAndReturnFrames();
//        mAppCallbackNotifier->stopPreviewCallbacks();
    }
    //
    //  stop preview & FD, and then free buffers.
    if  ( mpCamAdapter != 0 )
    {
        mpCamAdapter->stopPreview();
    }
lbExit:
    //  Always set it to false.
    mIsPreviewEnabled = false;
}


/******************************************************************************
* Returns true if preview is enabled.
*******************************************************************************/
bool
CamDevice::
previewEnabled()
{
    MY_LOGV("mIsPreviewEnabled(%d), mpCamAdapter(%p)", mIsPreviewEnabled, mpCamAdapter.get());
    //
    if  ( ! mIsPreviewEnabled )
    {
        return  false;
    }
    //
    return  ( mpCamAdapter == 0 )
        ?   false
        :   mpCamAdapter->previewEnabled()
        ;
}


/******************************************************************************
* Start record mode. When a record image is available a CAMERA_MSG_VIDEO_FRAME
* message is sent with the corresponding frame. Every record frame must be released
* by a cameral hal client via releaseRecordingFrame() before the client calls
* disableMsgType(CAMERA_MSG_VIDEO_FRAME). After the client calls
* disableMsgType(CAMERA_MSG_VIDEO_FRAME), it is camera hal's responsibility
* to manage the life-cycle of the video recording frames, and the client must
* not modify/access any video recording frames.
*******************************************************************************/
status_t
CamDevice::
startRecording()
{
    MY_LOGI("+ (%s:%d), (OpenId, DevId)=(%d, %d)", getDevName(), ::gettid(), getOpenId(), getDevId());
    return  ( mpCamAdapter == 0 )
        ?   (status_t)DEAD_OBJECT
        :   mpCamAdapter->startRecording()
        ;
}


/******************************************************************************
* Stop a previously started recording.
*******************************************************************************/
void
CamDevice::
stopRecording()
{
    MY_LOGI("+ (%s:%d), (OpenId, DevId)=(%d, %d)", getDevName(), ::gettid(), getOpenId(), getDevId());
    if  ( mpCamAdapter != 0 )
    {
        mpCamAdapter->stopRecording();
    }
}


/******************************************************************************
* Returns true if recording is enabled.
*******************************************************************************/
bool
CamDevice::
recordingEnabled()
{
    return  ( mpCamAdapter == 0 )
        ?   false
        :   mpCamAdapter->recordingEnabled()
        ;
}


/******************************************************************************
* Release a record frame previously returned by CAMERA_MSG_VIDEO_FRAME.
*
* It is camera hal client's responsibility to release video recording
* frames sent out by the camera hal before the camera hal receives
* a call to disableMsgType(CAMERA_MSG_VIDEO_FRAME). After it receives
* the call to disableMsgType(CAMERA_MSG_VIDEO_FRAME), it is camera hal's
* responsibility of managing the life-cycle of the video recording
* frames.
*******************************************************************************/
void
CamDevice::
releaseRecordingFrame(const void *opaque)
{
    if  ( mpCamAdapter != 0 )
    {
        mpCamAdapter->releaseRecordingFrame(opaque);
    }
}


/******************************************************************************
* Start auto focus, the notification callback routine is called
* with CAMERA_MSG_FOCUS once when focusing is complete. autoFocus()
* will be called again if another auto focus is needed.
*******************************************************************************/
status_t
CamDevice::
autoFocus()
{
    disableMsgType(CAMERA_MSG_PREVIEW_METADATA);

    return  ( mpCamAdapter == 0 )
        ?   (status_t)DEAD_OBJECT
        :   mpCamAdapter->autoFocus()
        ;
}


/******************************************************************************
* Cancels auto-focus function. If the auto-focus is still in progress,
* this function will cancel it. Whether the auto-focus is in progress
* or not, this function will return the focus position to the default.
* If the camera does not support auto-focus, this is a no-op.
*******************************************************************************/
status_t
CamDevice::
cancelAutoFocus()
{
    enableMsgType(CAMERA_MSG_PREVIEW_METADATA);

    return  ( mpCamAdapter == 0 )
        ?   (status_t)DEAD_OBJECT
        :   mpCamAdapter->cancelAutoFocus()
        ;
}


/******************************************************************************
* Take a picture.
*******************************************************************************/
status_t
CamDevice::
takePicture()
{
    disableMsgType(CAMERA_MSG_PREVIEW_METADATA);

    status_t status = NO_ERROR;

    if( mpCamAdapter == 0 )
        return (status_t)DEAD_OBJECT;

    status = mpCamAdapter->takePicture();

    if(NO_ERROR != status)
        return status;

    int32_t i4QuickviewWidth, i4QuickviewHeight;
    String8 s8QuickviewFmt;

    if(NO_ERROR == mpCamAdapter->queryQvFmt(i4QuickviewWidth, i4QuickviewHeight, s8QuickviewFmt))
    {
        if (mpDisplayAdapter != 0)
        {
            if  ( !mpDisplayAdapter->resetPreviewFmt(i4QuickviewWidth, i4QuickviewHeight, MtkCamUtils::CamFmtMap::inst().valueFor(s8QuickviewFmt)))
            {
                MY_LOGE("resetPreviewFmt fail");
                status = INVALID_OPERATION;
            }
        }
    }
    
    return status; 
}


/******************************************************************************
* Cancel a picture that was started with takePicture.  Calling this
* method when no picture is being taken is a no-op.
*******************************************************************************/
status_t
CamDevice::
cancelPicture()
{
    return  ( mpCamAdapter == 0 )
        ?   (status_t)DEAD_OBJECT
        :   mpCamAdapter->cancelPicture()
        ;
}


/******************************************************************************
* Set the camera parameters. This returns BAD_VALUE if any parameter is
* invalid or not supported.
*******************************************************************************/
status_t
CamDevice::
setParameters(const char* params)
{
    status_t status = DEAD_OBJECT;

    //    
    if  ( mpCamAdapter != 0 )
    {
#if 0
        char resolution[96];
        ::property_get("camera.prv.size", resolution, "640 480");
        int width = 0, height = 0;
        ::sscanf(resolution, "%d %d", &width, &height);
        CAM_LOGI("[setParameters]Preview Size %s = %d/%d", resolution, width, height);
        CameraParameters s( String8(  (const char*)params ) );
        s.setPreviewSize(width, height);
        status = mpCamAdapter->setParameters(s.flatten());
#else
        status = mpCamAdapter->setParameters(String8(params));
#endif
    }
    
    return  status;
}


/******************************************************************************
* Return the camera parameters.
*******************************************************************************/
char*
CamDevice::
getParameters()
{
    String8 params_str8 = mrParameters.flatten();
    // camera service frees this string...
    uint32_t const params_len = sizeof(char) * (params_str8.length()+1);
    char*const params_string = (char*)::malloc(params_len);
    if  ( params_string )
    {
        ::strcpy(params_string, params_str8.string());
    }
    //
    MY_LOGV("- params(%p), len(%d)", params_string, params_len);
    return params_string;
}


/******************************************************************************
* Put the camera parameters.
*******************************************************************************/
void
CamDevice::
putParameters(char *params)
{
    if  ( params )
    {
        ::free(params);
    }
    MY_LOGV("- params(%p)", params);
}


/******************************************************************************
* Send command to camera driver.
*******************************************************************************/
status_t
CamDevice::
sendCommand(int32_t cmd, int32_t arg1, int32_t arg2)
{
    status_t status = DEAD_OBJECT;
    //
    sp<ICamAdapter> pCamAdapter = mpCamAdapter;
    if  ( pCamAdapter != 0 )
    {
        status = pCamAdapter->sendCommand(cmd, arg1, arg2);
    }
    //
#if 0
    if  ( NO_ERROR != status )
    {
        switch  (cmd)
        {
        case CAMERA_CMD_PLAY_RECORDING_SOUND:
            status = NO_ERROR;
            break;
        default:
            MY_LOGW("tid(%d), bad command(0x%x), (arg1, arg2)=(0x%x, 0x%x)", gettid(), cmd, arg1, arg2);
            status = INVALID_OPERATION;
            break;
        }
    }
#endif
    //
    return  status;
}


/******************************************************************************
* Dump state of the camera hardware
*******************************************************************************/
status_t
CamDevice::
dump(int fd) const
{
    return NO_ERROR;
}


/******************************************************************************
*
*******************************************************************************/
bool
CamDevice::
queryPreviewSize(int32_t& ri4Width, int32_t& ri4Height)
{
    if  ( mpCamAdapter == 0 )
    {
        MY_LOGE("NULL Camera Adapter");
        return  false;
    }
    //
    mrParameters.getPreviewSize(&ri4Width, &ri4Height); 
    return  true;
}


/******************************************************************************
*
*******************************************************************************/
bool
CamDevice::
queryRecordingHint() const
{
    char const*valstr = NULL;
    valstr = mrParameters.get(CameraParameters::KEY_RECORDING_HINT);
    //
    if  ( ! valstr ) {
        MY_LOGD("No KEY_RECORDING_HINT");
        return  false;
    }
    //
    if ( ::strcmp(valstr, CameraParameters::TRUE) != 0 ) {
        MY_LOGD("KEY_RECORDING_HINT = false");
        return  false;
    }
    //
    MY_LOGD("KEY_RECORDING_HINT = true");
    return  true;
}


/******************************************************************************
*
*******************************************************************************/
status_t
CamDevice::
initDisplayAdapter(preview_stream_ops* window)
{
    AutoCPTLog cptlog(Event_CamDev_initDisplayAdapter);
    status_t status = NO_ERROR;
    int32_t i4PreviewWidth = 0, i4PreviewHeight = 0;
    String8 s8PreviewFormat;
    //
    MY_LOGD("+ window(%p)", window);
    //
    //
    //  [1] Check to see whether the passed window is NULL or not.
    if  ( ! window )
    {
        MY_LOGW("NULL window is passed into...");
        //
        if  ( mpDisplayAdapter != 0 )
        {
            MY_LOGW("destroy the display adapter...");
            mpDisplayAdapter->uninit();
            mpDisplayAdapter.clear();
        }
        status = NO_ERROR;
        goto lbExit;
    }
    //
    //
    //  [2] Check to see whether Camera Adapter is alive or not.
    if  ( mpCamAdapter == 0 )
    {
        MY_LOGE("NULL Camera Adapter");
        status = DEAD_OBJECT;
        goto lbExit;
    }
    //  [2.1] Get preview size.
    if  ( ! queryPreviewSize(i4PreviewWidth, i4PreviewHeight) )
    {
        MY_LOGE("queryPreviewSize");
        status = DEAD_OBJECT;
        goto lbExit;
    }
    //  [2.2] Get preview format.
    s8PreviewFormat = mrParameters.get(MtkCameraParameters::KEY_PREVIEW_INT_FORMAT);
    //
    //
    //  [3] Initialize Display Adapter.
    if  ( mpDisplayAdapter != 0 )
    {
        if  ( previewEnabled() )
        {
            MY_LOGW("Do nothing since Display Adapter(%p) is already created after startPreview()", &*mpDisplayAdapter.get());
//          This method must be called before startPreview(). The one exception is that 
//          if the preview surface texture is not set (or set to null) before startPreview() is called, 
//          then this method may be called once with a non-null parameter to set the preview surface.
            status = NO_ERROR;
            goto lbExit;
        }
        else
        {
            MY_LOGW("New window is set after stopPreview(). Destroy the current display adapter...");
            mpDisplayAdapter->uninit();
            mpDisplayAdapter.clear();
        }
    }
    //  [3.1] create a Display Adapter.
    mpDisplayAdapter = IDisplayAdapter::createInstance();
    if  ( mpDisplayAdapter == 0 )
    {
        MY_LOGE("Cannot create DisplayAdapter");
        status = NO_MEMORY;
        goto lbExit;
    }
    //  [3.2] initialize the newly-created Display Adapter.
    if  ( ! mpDisplayAdapter->init() )
    {
        MY_LOGE("DisplayAdapter init() failed");
        mpDisplayAdapter->uninit();
        mpDisplayAdapter.clear();
        status = NO_MEMORY;
        goto lbExit;
    }
    //  [3.3] set preview_stream_ops & related window info.
    if  ( ! mpDisplayAdapter->setWindowInfo(window, i4PreviewWidth, i4PreviewHeight, MtkCamUtils::CamFmtMap::inst().valueFor(s8PreviewFormat), MAX_DISPLAY_FRAME_COUNT) )
    {
        status = INVALID_OPERATION;
        goto lbExit;
    }
    //  [3.4] set frame provider
    if  ( ! mpDisplayAdapter->setFrameProvider(mpCamAdapter.get()) )
    {
        status = INVALID_OPERATION;
        goto lbExit;
    }
    //
    //
    status = NO_ERROR;
    //
lbExit:
    MY_LOGD("- status(%d)", status);
    return  status;
}


}; // namespace android

