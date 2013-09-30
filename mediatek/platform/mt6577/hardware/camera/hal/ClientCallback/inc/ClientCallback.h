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

#ifndef _MTK_CLIENT_CALLBACK_H_
#define _MTK_CLIENT_CALLBACK_H_
//
#include <utils/Errors.h>
#include <utils/RefBase.h>
#include <utils/String8.h>
#include <utils/threads.h>
//
#include <hardware/camera.h>
#include <system/camera.h>
//
#include <Utils/inc/CamUtils.h>
using namespace android::MtkCamUtils;


namespace android {


/******************************************************************************
*   Camera Client Callback Handler.
*******************************************************************************/
class ClientCallback : public virtual RefBase
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////

    virtual bool                    init();
    virtual bool                    uninit();

    /**
     *
     */
    virtual bool                    setFrameProvider(FrameNotifier*const frameNotifier);
    /**
     *
     */
    virtual void                    setBufferProvider(IBufferProvider*const pBufferProvider);
    /**
     *
     */
    virtual void                    setCallbacks(camera_notify_callback notify_cb,
                                        camera_data_callback data_cb,
                                        camera_data_timestamp_callback data_cb_timestamp,
                                        void *user
                                    );
    virtual void                    enableMsgType(int32_t msgType);
    virtual void                    disableMsgType(int32_t msgType);
    virtual bool                    msgTypeEnabled(int32_t msgType);
    /**
     *
     */
    virtual bool                    startPreview(
                                        String8 const&  rs8PreviewTargetFormat, 
                                        String8 const&  rs8PreviewSourceFormat, 
                                        int32_t const   i4PreviewWidth, 
                                        int32_t const   i4PreviewHeight
                                    );
    virtual bool                    stopPreview();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////    Instantiation.
    //
                                    ClientCallback();
    virtual                         ~ClientCallback();

protected:  ////    Frame Provider.
    static  void                    onFrameCallback(CamFrame*const pCamFrame);
    void                            onFrame(CamFrame*const pCamFrame);
protected:  ////    Handler.
    virtual void                    handlePreviewFrame(CamFrame*const pCamFrame);
    virtual void                    handlePostviewFrame(CamFrame*const pCamFrame);
    virtual void                    convertAndSendPreviewFrame(CamFrame*const pCamFrame, int32_t const msgType);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data Members.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////    Common Info.
    mutable Mutex                   mMtxLock;

protected:  ////    Buffer Provider.
    IBufferProvider*                mpBufProvider;

protected:  ////    Frame Provider.
    sp<FrameProvider>               mpFrameProvider;
    sp<FrameProvider>               promoteFrameProvider()  const;

protected:  ////    Callback & Messages.
    //
    volatile int32_t                mMsgEnabled;
    //
    void*                           mClientCbCookie;
    camera_notify_callback          mNotifyCb;
    camera_data_callback            mDataCb;
    camera_data_timestamp_callback  mDataCbTimestamp;

protected:  ////    Preview Info.
    String8                         ms8PreviewTgtFmt;   //  Target Format.
    String8                         ms8PreviewSrcFmt;   //  Source Format.
    int32_t                         mi4PreviewWidth;    //  Width.
    int32_t                         mi4PreviewHeight;   //  Height.
    //
    enum    { eMAX_PREVIEW_BUFFER_NUM = 2 };
    sp<IImageBufferPool>            mpPreviewBufPool;
    uint_t                          muPreviewBufsIdx;
    //
    int32_t                         mi4PreviewCbRefCount;
    int64_t                         mi8PreviewCbTimeInMs;   //  The timestamp in millisecond of last preview callback.
};


}; // namespace android
#endif  //_MTK_CLIENT_CALLBACK_H_

