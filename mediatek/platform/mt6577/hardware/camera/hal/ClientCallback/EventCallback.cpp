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

#define LOG_TAG "MtkCam/EventCallback"

#include "inc/EventCallback.h"
#include "Utils/inc/CamUtils.h"
#include <binder/MemoryBase.h>
#include "camera/MtkCamera.h"
#include <sys/prctl.h>

using namespace android;

#define EXIT_MSG 0
/*******************************************************************************
*
********************************************************************************/
#define MY_LOGV(fmt, arg...)            CAM_LOGV("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)            CAM_LOGD("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)            CAM_LOGI("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)            CAM_LOGW("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)            CAM_LOGE("[%s] "fmt, __FUNCTION__, ##arg)

/*******************************************************************************
*
********************************************************************************/
EventCallback::EventCallback()
    : Thread(false),
      mDataQ(),
      mNotifyQ(),
      mMsgQ(),
      mpBufProvider(NULL)
{
    MY_LOGD("EventCallback construction");
}


/*******************************************************************************
*
********************************************************************************/
EventCallback::~EventCallback()
{

    mDataQ.clear();
    mNotifyQ.clear();
    mMsgQ.clear();

    MY_LOGD("EventCallback destruction");   
}


/*******************************************************************************
*
********************************************************************************/
status_t
EventCallback::readyToRun()
{
    // name thread
    ::prctl(PR_SET_NAME,"CameraEventCallback", 0, 0, 0);   
	
    // set policy and priority
    const int expected_policy = SCHED_OTHER;
    const int expected_priority = 0;
    setPriority(expected_policy, expected_priority);
    // 
    mi4Tid = ::gettid();
    MY_LOGD("EventCallback tid: %d", mi4Tid);

    return NO_ERROR;
}



/*******************************************************************************
*
********************************************************************************/
bool
EventCallback::waitCommand()
{
    Mutex::Autolock _l(mMtxLock);
	
    while( mMsgQ.empty() && !exitPending() ) {
        mCmdCond.wait(mMtxLock);
    }

	// going to exit
    if ( exitPending() ) {
        mMsgQ.clear();
        mDataQ.clear();
        mNotifyQ.clear();
        return false;
    }
    else {  // normal condition: mMsgQ is not empty
        return true;
    }
}

/*******************************************************************************
*
********************************************************************************/
bool
EventCallback::threadLoop()
{
    if (waitCommand())
    {
        Mutex::Autolock _l(mMtxLock);
        switch (*mMsgQ.begin()) {
		   
            case CAMERA_MSG_PREVIEW_METADATA:
            {
                if (!mDataQ.empty())
                {
                    FDCommand rCmd = *mDataQ.begin();	
                    camera_memory_t dummyBuffer;
                    if ( mpBufProvider!= NULL) {
                    mpBufProvider->allocBuffer(dummyBuffer, 1, 1);
                   
                    mDataCb(
                           rCmd.MsgType,
                           &dummyBuffer,
                           rCmd.Index,
                           &rCmd.MetaData,	
                           (void*)(rCmd.Cookie)
                    );
                    dummyBuffer.release(&dummyBuffer);
                    //mpBufProvider->freeBuffer(dummyBuffer);
                    }
                    mDataQ.erase(mDataQ.begin());
                    mMsgQ.erase(mMsgQ.begin());
                    MY_LOGD("[dispatchCallback]MHAL_CAM_CB_FD -");
                }
                break;
            }

		   
            case MTK_CAMERA_MSG_EXT_NOTIFY:
            {
                if (!mNotifyQ.empty())
                {
                    NotifyCommand rCmd = *mNotifyQ.begin();	
                    mNotifyCb(
                           rCmd.msg_type,
                           rCmd.ext1,
                           rCmd.ext2,
                           (void*)(rCmd.Cookie)
                    );		
                    mNotifyQ.erase(mNotifyQ.begin());
                    mMsgQ.erase(mMsgQ.begin());
                    MY_LOGD("[dispatchCallback] MTK_CAMERA_MSG_EXT_NOTIFY_SMILE_DETECT -");
                }
                break;
            } 
      
            case EXIT_MSG:
                MY_LOGD("Exit message");
                break;
			
            default:
                MY_LOGD("Unexpected Msg Type");
                break;		   
       	} // end of switch
    } // end of if
    else {
        MY_LOGD("EventCallback is going to leave thread loop");
    }
    return true;
}


/*******************************************************************************
*
********************************************************************************/
void 
EventCallback::requestExit()
{
	MY_LOGD("EventCallback(tid: %d) is requested exit", mi4Tid);
    Thread::requestExit();
    mMsgQ.push_back(EXIT_MSG);
    mCmdCond.broadcast();
}


/*******************************************************************************
*
********************************************************************************/
void
EventCallback::setDataBuf(int msg_type,
                 void *data, 
                 int index,
                 void *metadata_t, 
                 void *user)
{
    //
    Mutex::Autolock _l(mMtxLock);
    //
    FDCommand rCmd(msg_type, data, index, metadata_t, (int)user);
    mDataQ.push_back(rCmd);
    mMsgQ.push_back(msg_type);
    mCmdCond.broadcast();
}


/*******************************************************************************
*
********************************************************************************/
void
EventCallback::setNotifyBuf(int msg_type, 
                            int ext1,
                            int ext2,
                            void *cookie)
{
    //
    Mutex::Autolock _l(mMtxLock);
    //
    NotifyCommand rCmd(msg_type, ext1, ext2, (int)cookie);
    mNotifyQ.push_back(rCmd);
    mMsgQ.push_back(msg_type);
    mCmdCond.broadcast();
}


/******************************************************************************
* 
*******************************************************************************/
void
EventCallback::
setBufferProvider(IBufferProvider*const pBufferProvider)
{	
    mpBufProvider = pBufferProvider;
}

