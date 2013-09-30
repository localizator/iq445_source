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

#ifndef _MTK_CAM_DEVICE_H_
#define _MTK_CAM_DEVICE_H_
//


namespace android {
/******************************************************************************
*
*******************************************************************************/
class CameraParameters;
class ICamDevice;
class CamBufProvider;
class ICamAdapter;
class IDisplayAdapter;
class ClientCallback;


/******************************************************************************
*   Camera Device
*******************************************************************************/
class CamDevice : public ICamDevice
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Common Attributes.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////    Data Members.
    //
    String8 const                   mDevName;       //  device name.
    int32_t const                   mi4OpenId;      //  device open ID.
    CamDevInfo const                mDevInfo;       //  device information.

public:     ////    Operations.
    //
    inline  char const* getDevName() const  { return mDevName.string(); }
    inline  int32_t     getOpenId() const   { return mi4OpenId; }
    inline  EDeviceId   getDevId() const    { return mDevInfo.eDevID; }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data Members.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////
    //
    CamBufProvider*                 mpBufProvider;
    sp<IDisplayAdapter>             mpDisplayAdapter;
    sp<ICamAdapter>                 mpCamAdapter;
    sp<ClientCallback>              mpClientCbHandler;
    CameraParameters&               mrParameters;
    //
    bool                            mIsPreviewEnabled;

protected:  ////    Callback & Messages.
    //
    volatile int32_t                mMsgEnabled;
    //
    void*                           mCallbackCookie;
    camera_notify_callback          mNotifyCb;
    camera_data_callback            mDataCb;
    camera_data_timestamp_callback  mDataCbTimestamp;
    camera_request_memory           mfpRequestMemory;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////    Instantiation.

                        CamDevice(
                            String8 const&          rDevName, 
                            int32_t const           i4OpenId, 
                            CamDevInfo const&       rDevInfo, 
                            sp<ICamAdapter> const&  rpCamAdapter
                        );
                        ~CamDevice();

protected:  ////    Instantiation.
    /**
     * Initialize the device resources owned by this object.
     */
    virtual bool        onInit();

    /**
     * Uninitialize the device resources owned by this object. Note that this is
     * *not* done in the destructor.
     */
    virtual bool        onUninit();

    virtual bool        queryPreviewSize(int32_t& ri4Width, int32_t& ri4Height);

    virtual bool        queryRecordingHint() const;

    virtual status_t    initDisplayAdapter(preview_stream_ops* window);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////

    virtual void        setCallbacks(camera_notify_callback notify_cb,
                            camera_data_callback data_cb,
                            camera_data_timestamp_callback data_cb_timestamp,
                            camera_request_memory get_memory,
                            void *user
                        );
    virtual void        enableMsgType(int32_t msgType);
    virtual void        disableMsgType(int32_t msgType);
    virtual bool        msgTypeEnabled(int32_t msgType);
    //
    virtual status_t    setPreviewWindow(preview_stream_ops* window);
    //
    virtual status_t    startPreview();
    virtual void        stopPreview();
    virtual bool        previewEnabled();
    //
    virtual status_t    startRecording();
    virtual void        stopRecording();
    virtual bool        recordingEnabled();
    virtual void        releaseRecordingFrame(const void *opaque);
    //
    virtual status_t    autoFocus();
    virtual status_t    cancelAutoFocus();
    //
    virtual status_t    takePicture();
    virtual status_t    cancelPicture();
    //
    virtual status_t    setParameters(const char* params);
    virtual char*       getParameters();
    virtual void        putParameters(char *params);
    //
    virtual status_t    sendCommand(int32_t cmd, int32_t arg1, int32_t arg2);
    virtual status_t    dump(int fd) const;
};


}; // namespace android
#endif  //_MTK_CAM_DEVICE_H_
