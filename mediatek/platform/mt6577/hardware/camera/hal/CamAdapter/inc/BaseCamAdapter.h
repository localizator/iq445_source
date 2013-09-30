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

#ifndef _MTK_BASE_CAM_ADAPTER_H_
#define _MTK_BASE_CAM_ADAPTER_H_
//
#include <utils/Errors.h>
#include <utils/RefBase.h>
#include <utils/String8.h>
#include <utils/threads.h>
#include <utils/KeyedVector.h>
//
#include <hardware/camera.h>
#include <system/camera.h>


namespace android {


class BaseCamAdapter : public ICamAdapter
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  FrameNotifier Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////
    virtual void        subscribeMsgType(int32_t const msgType, void* cookie, cam_frame_callback const frameCb);
    virtual void        unsubscribeMsgType(int32_t const msgType, void* cookie);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  CamAdapter Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////

    virtual void        setBufferProvider(IBufferProvider*const pBufferProvider);
    virtual void        setCallbacks(camera_notify_callback notify_cb,
                            camera_data_callback data_cb,
                            camera_data_timestamp_callback data_cb_timestamp,
                            void *user
                        );
    virtual void        enableMsgType(int32_t msgType);
    virtual void        disableMsgType(int32_t msgType);
    virtual bool        msgTypeEnabled(int32_t msgType);
    virtual status_t    sendCommand(int32_t cmd, int32_t arg1, int32_t arg2);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////    Instantiation.
    //
                                    BaseCamAdapter(
                                        String8 const& rName, 
                                        int32_t const i4OpenId, 
                                        CamDevInfo const& rDevInfo
                                    );
    virtual                         ~BaseCamAdapter();

protected:  ////    Operations.
    void                            showParameters(String8 const& param) const;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Common Attributes.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////    Attributes.
    String8 const                   mName;
    int32_t const                   mi4OpenId;
    CamDevInfo const                mDevInfo;       //  device information.

public:     ////    Attributes.
    inline  char const*             getName()const      { return mName.string(); }
    inline  int32_t                 getOpenId() const   { return mi4OpenId; }
    inline  EDeviceId               getDevId()  const   { return mDevInfo.eDevID; }

//------------------------------------------------------------------------------
protected:  ////    Subscribers for FrameNotifier
    typedef KeyedVector<void*, cam_frame_callback>  FrameObservers_t;
    mutable RWLock                  mRWLockFrameNotifier;
    FrameObservers_t                mvFrameObservers;

protected:  ////    Operations.
    bool                            postBuffer(
                                        sp<IImageBufferPool> const& rpImageBufferPool, 
                                        int32_t const bufIndex, 
                                        int64_t const bufTimestamp, 
                                        int32_t const msgType
                                    );
    bool                            sendFrameToObservers(CamFrame*const frame);
    bool                            sendFrameToObservers(CamFrame*const frame, FrameObservers_t const& rObservers);

//------------------------------------------------------------------------------
protected:  ////    Callback & Messages.
    //
    volatile int32_t                mMsgEnabled;
    //
    void*                           mCallbackCookie;
    camera_notify_callback          mNotifyCb;
    camera_data_callback            mDataCb;
    camera_data_timestamp_callback  mDataCbTimestamp;

//------------------------------------------------------------------------------
protected:  ////    Memory Management.
    //
    IBufferProvider*                mpBufProvider;

//------------------------------------------------------------------------------
};


}; // namespace android
#endif  //_MTK_BASE_CAM_ADAPTER_H_

