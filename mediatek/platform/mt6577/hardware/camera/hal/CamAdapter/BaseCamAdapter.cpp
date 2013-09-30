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


/******************************************************************************
*
*******************************************************************************/
#define ENABLE_LOG_PER_FRAME        (1)


/******************************************************************************
*
*******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("(%s)[BaseCamAdapter::%s] "fmt, getName(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%s)[BaseCamAdapter::%s] "fmt, getName(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("(%s)[BaseCamAdapter::%s] "fmt, getName(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%s)[BaseCamAdapter::%s] "fmt, getName(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%s)[BaseCamAdapter::%s] "fmt, getName(), __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, arg...)    if (cond) { MY_LOGV(arg); }
#define MY_LOGD_IF(cond, arg...)    if (cond) { MY_LOGD(arg); }
#define MY_LOGI_IF(cond, arg...)    if (cond) { MY_LOGI(arg); }
#define MY_LOGW_IF(cond, arg...)    if (cond) { MY_LOGW(arg); }
#define MY_LOGE_IF(cond, arg...)    if (cond) { MY_LOGE(arg); }


/******************************************************************************
*
*******************************************************************************/
namespace android {


/******************************************************************************
*
*******************************************************************************/
BaseCamAdapter::
BaseCamAdapter(String8 const& rName, int32_t const i4OpenId, CamDevInfo const& rDevInfo)
    : ICamAdapter()
    , mName(rName)
    , mi4OpenId(i4OpenId)
    , mDevInfo(rDevInfo)
    //
    , mRWLockFrameNotifier()
    , mvFrameObservers()
    //
    , mMsgEnabled(0)
    , mCallbackCookie(NULL)
    , mNotifyCb(NULL)
    , mDataCb(NULL)
    , mDataCbTimestamp(NULL)
    //
    , mpBufProvider(NULL)
    //
{
    MY_LOGD("tid(%d), (OpenId, DevId)=(%d, %d)", ::gettid(), getOpenId(), getDevId());
}


/******************************************************************************
*
*******************************************************************************/
BaseCamAdapter::
~BaseCamAdapter()
{
    MY_LOGD("tid(%d), (OpenId, DevId)=(%d, %d)", ::gettid(), getOpenId(), getDevId());
    //
    RWLock::AutoWLock _l(mRWLockFrameNotifier);
    mvFrameObservers.clear();
}


/******************************************************************************
*
*******************************************************************************/
void
BaseCamAdapter::
subscribeMsgType(int32_t const msgType, void* cookie, cam_frame_callback const frameCb)
{
    MY_LOGI(
        "+ tid(%d), (msgType, cookie, frameCb)=(0x%x, %p, %p) "
        "mvFrameObservers.size(%d)", 
        ::gettid(), msgType, cookie, frameCb, 
        mvFrameObservers.size()
    );
    //
    RWLock::AutoWLock _l(mRWLockFrameNotifier);
    //
    //  Now we just support one-shot setting for only one message.
    switch  (msgType)
    {
    case CAMERA_MSG_PREVIEW_FRAME | CAMERA_MSG_POSTVIEW_FRAME:
    case CAMERA_MSG_PREVIEW_FRAME:
    case CAMERA_MSG_POSTVIEW_FRAME:
        mvFrameObservers.add(cookie, frameCb);
        break;
    case CAMERA_MSG_ALL_MSGS:
        mvFrameObservers.add(cookie, frameCb);
        break;
    default:
        MY_LOGE("Unsupported type(0x%x).", msgType);
        break;
    }
    //
    MY_LOGI(
        "- tid(%d), (msgType, cookie, frameCb)=(0x%x, %p, %p) "
        "mvFrameObservers.size(%d)", 
        ::gettid(), msgType, cookie, frameCb, 
        mvFrameObservers.size()
    );
}


/******************************************************************************
*
*******************************************************************************/
void
BaseCamAdapter::
unsubscribeMsgType(int32_t const msgType, void* cookie)
{
    MY_LOGI(
        "+ tid(%d), (msgType, cookie)=(0x%x, %p) "
        "mvFrameObservers.size(%d)", 
        ::gettid(), msgType, cookie, 
        mvFrameObservers.size()
    );
    //
    RWLock::AutoWLock _l(mRWLockFrameNotifier);
    //
    //  Now we just support one-shot setting for only one message.
    switch  (msgType)
    {
    case CAMERA_MSG_PREVIEW_FRAME | CAMERA_MSG_POSTVIEW_FRAME:
    case CAMERA_MSG_PREVIEW_FRAME:
    case CAMERA_MSG_POSTVIEW_FRAME:
        mvFrameObservers.removeItem(cookie);
        break;
    case CAMERA_MSG_ALL_MSGS:
        mvFrameObservers.removeItem(cookie);
        break;
    default:
        MY_LOGE("Unsupported type(%x).", msgType);
        break;
    }
    //
    MY_LOGI(
        "- tid(%d), (msgType, cookie)=(0x%x, %p) "
        "mvFrameObservers.size(%d)", 
        ::gettid(), msgType, cookie, 
        mvFrameObservers.size()
    );
}


/******************************************************************************
* Set the notification and data callbacks
*******************************************************************************/
void
BaseCamAdapter::
setCallbacks(
    camera_notify_callback notify_cb,
    camera_data_callback data_cb,
    camera_data_timestamp_callback data_cb_timestamp,
    void *user
)
{
    mNotifyCb       = notify_cb;
    mDataCb         = data_cb;
    mDataCbTimestamp= data_cb_timestamp;
    mCallbackCookie = user;
}


/******************************************************************************
* Enable a message, or set of messages.
*******************************************************************************/
void
BaseCamAdapter::
enableMsgType(int32_t msgType)
{
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
BaseCamAdapter::
disableMsgType(int32_t msgType)
{
    ::android_atomic_and(~msgType, &mMsgEnabled);
}


/******************************************************************************
* Query whether a message, or a set of messages, is enabled.
* Note that this is operates as an AND, if any of the messages
* queried are off, this will return false.
*******************************************************************************/
bool
BaseCamAdapter::
msgTypeEnabled(int32_t msgType)
{
    return  msgType == ( msgType & mMsgEnabled );
}


/******************************************************************************
* Send command to camera driver.
*******************************************************************************/
status_t
BaseCamAdapter::
sendCommand(int32_t cmd, int32_t arg1, int32_t arg2)
{
    status_t status = NO_ERROR;
    switch  (cmd)
    {
    case CAMERA_CMD_PLAY_RECORDING_SOUND:
        break;
    default:
        MY_LOGW("tid(%d), bad command(0x%x), (arg1, arg2)=(0x%x, 0x%x)", ::gettid(), cmd, arg1, arg2);
        status = INVALID_OPERATION;
        break;
    }
    return  status;
}


/******************************************************************************
*
*******************************************************************************/
void
BaseCamAdapter::
showParameters(String8 const& param) const
{
    static char bufParam[4096];

    const int count = 768;
    char *pbuf = bufParam;
    char ch;
    uint32_t len;
    uint32_t i, loop;

    pbuf[0] = 0;
    len = param.length();
    if ( len > 4096) {
        len = 4096;
    }

    ::strcpy(pbuf, param.string());
    loop = (len + count - 1) / count;

    CAM_LOGD("[showParameters]: %d, %d \n", loop, len);
    for (i = 0; i < loop; i++) {
        ch = pbuf[count];
        pbuf[count] = 0;
        CAM_LOGD("(%s) \n", pbuf);
        pbuf[count] = ch;
        pbuf += count;
    }
}


/******************************************************************************
* 
*******************************************************************************/
void
BaseCamAdapter::
setBufferProvider(IBufferProvider*const pBufferProvider)
{
    mpBufProvider = pBufferProvider;
}


/******************************************************************************
*
*******************************************************************************/
bool
BaseCamAdapter::
postBuffer(
    sp<IImageBufferPool> const& rpImageBufferPool, 
    int32_t const bufIndex, 
    int64_t const bufTimestamp, 
    int32_t const msgType
)
{
    CamFrame frame;
    //
    frame.mType     = msgType;
    frame.mCbCookie = NULL;                     //  callback cookie.
    frame.mBufCookie= rpImageBufferPool.get();  //  buffer cookie.
    frame.mBufIndex = bufIndex;                 //  buffer index.
    frame.mTimestamp= bufTimestamp;             //  timestamp
    //
    return  sendFrameToObservers(&frame);
}


/******************************************************************************
*
*******************************************************************************/
bool
BaseCamAdapter::
sendFrameToObservers(CamFrame*const frame)
{
    bool ret = false;
    //
    if  ( ! frame )
    {
        MY_LOGW("null frame");
        return  false;
    }
    //
    switch  ( frame->mType )
    {
    case CAMERA_MSG_PREVIEW_FRAME:
    case CAMERA_MSG_POSTVIEW_FRAME:
        ret = sendFrameToObservers(frame, mvFrameObservers);
        break;
    default:
        MY_LOGE("Unsupported frame type(0x%x)", frame->mType);
        break;
    }
    //
    return  ret;
}


/******************************************************************************
*
*******************************************************************************/
bool
BaseCamAdapter::
sendFrameToObservers(CamFrame*const frame, FrameObservers_t const& rObservers)
{
    MY_LOGV_IF(ENABLE_LOG_PER_FRAME, "+ frameType(0x%x)", frame->mType);
    //
    cam_frame_callback callback = NULL;
    //
    for (uint_t i = 0; i < rObservers.size(); i++)
    {
        {
            RWLock::AutoRLock _l(mRWLockFrameNotifier);
            //
            if  ( i >= rObservers.size() )
            {
                MY_LOGW("break loop since i(%d) >= rObservers.size(%d)", i, rObservers.size());
                break;
            }
            //
            frame->mCbCookie = rObservers.keyAt(i);
            callback = rObservers.valueAt(i);
        }
        //
        MY_LOGV_IF(ENABLE_LOG_PER_FRAME, 
            "i(%d), (frameType, cookie, callback)=(0x%x, %p, %p)", 
            i, frame->mType, frame->mCbCookie, callback
        );
        //
        if  ( ! callback )
        {
            MY_LOGW("null callback for frameType(0x%x)", frame->mType);
            continue;
        }
        //
        frame->incRef();
        callback(frame);
    }
    //
    return  true;
}


}; // namespace android

