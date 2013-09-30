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

#ifndef _MTK_EVENT_CALLBACK_H_
#define _MTK_EVENT_CALLBACK_H_

#include <utils/Errors.h>
#include <utils/threads.h>
//
#include <hardware/camera.h>
#include <system/camera.h>
#include <utils/List.h>
#include <utils/Vector.h>

#include <Utils/inc/CamUtils.h>
using namespace android::MtkCamUtils;
//

namespace android {

/******************************************************************************
*   Event Callback Handler ( current version is for SD and FD)
*******************************************************************************/
class EventCallback : public Thread {

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:

    EventCallback();

    virtual ~EventCallback();
	
    // Ask this object's thread to exit. This function is asynchronous, when the
    // function returns the thread might still be running. Of course, this
    // function can be called from a different thread.	
    virtual void      requestExit();

    // Good place to do one-time initializations
    virtual status_t  readyToRun();

public:     
            bool      waitCommand();
            int       getTid() const { return mi4Tid; }
            void      setDataCallback(camera_data_callback cb) {mDataCb = cb;}
            void      setNotifyCallback(camera_notify_callback cb) {mNotifyCb = cb;}
            void      setBufferProvider(IBufferProvider*const pBufferProvider);

            void      setDataBuf(int msg_type,
                                 void *data, 
                                 int index,
                                 void *src, 
                                 void *cookie
                      );
			
            void      setNotifyBuf(int msg_type,
                                   int ext1,
                                   int ext2,
                                   void *cookie
                      );			
private:
	
    // Derived class must implement threadLoop(). The thread starts its life
    // here. There are two ways of using the Thread object:
    // 1) loop: if threadLoop() returns true, it will be called again if
    //          requestExit() wasn't called.
    // 2) once: if threadLoop() returns false, the thread will exit upon return.
    virtual bool      threadLoop();
			

private:	

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//	Commands.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    struct FDCommand {

	    FDCommand(
                int                      _MsgType,
                void                     *_Data,
                unsigned int             _Index,
                void                     *_MetaData,	
                int                      _Cookie 
        )
        : MsgType(_MsgType), Index(_Index), Cookie(_Cookie)
        {
            if (_MetaData) {
                memcpy(&MetaData, _MetaData, sizeof(camera_frame_metadata_t));
            }
            // _data is T.B.D (not needed in FD command)
        }

    //--------------------------------//
        int                      MsgType;
        void                     *Data;
        unsigned int             Index;
        camera_frame_metadata_t  MetaData;	
        int                      Cookie; 
    };

    struct NotifyCommand {
		
        NotifyCommand(
            int _msg_type = 0,
            int _ext1 = 0,
            int _ext2 = 0,
            int _Cookie = 0
        )
        : msg_type(_msg_type), ext1(_ext1), ext2(_ext2), Cookie(_Cookie)
        {}
    //-------------------------------//
        int msg_type;
        int ext1;
        int ext2;
        int Cookie;
    };

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//	
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    List<FDCommand>              mDataQ;
    List<NotifyCommand>          mNotifyQ;
    Vector<int>                  mMsgQ;
    
    camera_data_callback         mDataCb;
    camera_notify_callback       mNotifyCb;	
    IBufferProvider*             mpBufProvider;
    int                          mi4Tid;
    mutable Mutex                mMtxLock;
    Condition                    mCmdCond;

};


}; // end of namespace
#endif 

