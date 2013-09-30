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

#ifndef _MTK_DISPLAY_ADAPTER_PREVIEW_WINDOW_DISPLAY_ADAPTER_H_
#define _MTK_DISPLAY_ADAPTER_PREVIEW_WINDOW_DISPLAY_ADAPTER_H_
//
#include <utils/Errors.h>
#include <utils/threads.h>
#include <utils/String8.h>
//
#include <hardware/camera.h>
#include <system/camera.h>


/******************************************************************************
*
*******************************************************************************/
namespace android {
/******************************************************************************
*
*******************************************************************************/


/******************************************************************************
* Preview Window Display Adapter
*******************************************************************************/
class PVWDisplayAdapter : public IDisplayAdapter
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Inherited Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////

    /**
     * Initialize the display adapter to create any resources required.
     */
    virtual bool    init();

    /**
     * Uninitialize the display adapter.
     */
    virtual bool    uninit();

    /**
     * Set the preview_stream_ops to which frames are sent
     */
    virtual bool    setWindowInfo(
                        preview_stream_ops*const window, 
                        int32_t const   wndWidth, 
                        int32_t const   wndHeight, 
                        int32_t const   i4HalPixelFormat, 
                        int32_t const   maxFrameCount
                    );
	
    /**
	 *Reset preview frame format
     */
    virtual bool    resetPreviewFmt(
                        int32_t const   wndWidth, 
                        int32_t const   wndHeight, 
                        int32_t const   i4HalPixelFormat 
                    );
    /**
     *
     */
    virtual bool    enableDisplay(bool const isRecordingHint);
    virtual bool    disableDisplay(bool const cancel_buffer = true);
    /**
     *
     */
    virtual bool    setFrameProvider(FrameNotifier*const frameNotifier);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////
                    PVWDisplayAdapter();
    virtual         ~PVWDisplayAdapter();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Type Definitions.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////    Target Buffer Queue.

    class   TgtBufQue;

//------------------------------------------------------------------------------
protected:  ////    Queue Buffer Interface.

    struct  IQueBufInfo
    {
    virtual void*   getBuf() const  = 0;
    //
    virtual         ~IQueBufInfo() {}
    };

//------------------------------------------------------------------------------
protected:  ////    Display Frame

    struct  DispFrame
    {
    public:     ////    fields.
        uint32_t        mType;
        void*           mBuf;
        int32_t         mi4HalPixelFormat;
        int32_t         mWidth;
        int32_t         mHeight;
        size_t          mLength;
        int64_t         mTimestamp;
        //
    public:     ////    Operations.
                        DispFrame()
                            : mType(0)
                            , mBuf(NULL)
                            , mi4HalPixelFormat(0)
                            , mWidth(0)
                            , mHeight(0)
                            , mLength(0)
                            , mTimestamp(0)
                        {}
    };

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////    Window.
    virtual bool        setWindow(
							preview_stream_ops*const window,
                            int32_t const   wndWidth, 
                            int32_t const   wndHeight, 
                            int32_t const   i4HalPixelFormat, 
                            int32_t const   maxFrameCount
                        );
    virtual bool        setWindow(preview_stream_ops*const window);

protected:  ////    Window Info.
    //
    preview_stream_ops* mpPreviewWindow;
    int32_t             mi4HalPixelFormat;
    int32_t             mi4WndWidth;
    int32_t             mi4WndHeight;
    //
    int32_t             mi4MaxFrameCount;
    //
//------------------------------------------------------------------------------
protected:  ////    Target Buffer Queue.
    //
    sp<TgtBufQue>       mpTgtBufQue;

//------------------------------------------------------------------------------
protected:  ////    Source -> Target Transform for Display
    //
    Mutex               mMutex;
    bool                mIsDispEnabled;

protected:  ////    
    virtual bool        copyFrame(int const iDstStride, void* pDstBuf, DispFrame const& rDispFrame) = 0;

//------------------------------------------------------------------------------
protected:  ////    Frame Provider.
    //
    static  void        onFrameCallback(CamFrame*const pCamFrame);
    void                onFrame(CamFrame*const pCamFrame);
    bool                postFrame(DispFrame& rFrame);

protected:  ////
    sp<FrameProvider>   mpFrameProvider;

//------------------------------------------------------------------------------
protected:  ////
    //
    virtual bool        onDestroy();

//------------------------------------------------------------------------------
protected:  ////    States & Flags.
    bool                mIsRecordingHint;
    bool                mIsNative3D;

};


}; // namespace android
#endif  //_MTK_DISPLAY_ADAPTER_PREVIEW_WINDOW_DISPLAY_ADAPTER_H_

