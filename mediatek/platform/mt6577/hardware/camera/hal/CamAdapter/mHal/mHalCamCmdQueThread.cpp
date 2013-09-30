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

/*
**
** Copyright 2008, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/
#define LOG_TAG "MtkCam/MCamHw"
//
#include "Utils/inc/CamUtils.h"
//
#include "inc/ICamAdapter.h"
#include "inc/BaseCamAdapter.h"
#include "inc/mHalBaseAdapter.h"
#include "inc/mHalCamAdapter.h"
#include "mHalCamCmdQueThread.h"
#include "./mHalCamUtils.h"
//
#include <sys/prctl.h>

/******************************************************************************
*
*******************************************************************************/
#define MY_LOGV(fmt, arg...)    CAM_LOGV("[CmdQueThread::%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)    CAM_LOGD("[CmdQueThread::%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)    CAM_LOGI("[CmdQueThread::%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)    CAM_LOGW("[CmdQueThread::%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)    CAM_LOGE("[CmdQueThread::%s] "fmt, __FUNCTION__, ##arg)


/******************************************************************************
*
*******************************************************************************/
namespace android {


/******************************************************************************
*
*******************************************************************************/
char const*
CmdQueThread::
Command::
getName(EIdentity const _eId)
{
#define CMD_NAME(x) case x: return #x
    switch  (_eId)
    {
    CMD_NAME(eID_UNKNOWN);
    CMD_NAME(eID_EXIT);
    CMD_NAME(eID_TAKE_PICTURE);
    CMD_NAME(eID_TAKE_PRV_PIC);
    CMD_NAME(eID_SMOOTH_ZOOM);
    CMD_NAME(eID_DO_PANORAMA);
    default:
        break;
    }
#undef  CMD_NAME
    return  "";
}


/******************************************************************************
*
*******************************************************************************/
CmdQueThread::
CmdQueThread(mHalCamAdapter*const pHardware, FPTRCommandHandler_t const fpCommandHandler)
    : Thread(false)
    //
    , mpHardware(pHardware)
    , mi4Tid(0)
    //
    , mfpCommandHandler(fpCommandHandler)
    , mCmdQue()
    , mCmdCond()
    , mCmdLock()
{
    MY_LOGD(
        "tid(%d) mpHardware->getStrongCount(%d), mpHardware(%p)"
        , ::gettid(), mpHardware->getStrongCount(), mpHardware.get()
    );
}


/******************************************************************************
*
*******************************************************************************/
CmdQueThread::
~CmdQueThread()
{
    MY_LOGD(
        "tid(%d) mpHardware->getStrongCount(%d), mpHardware(%p)"
        , ::gettid(), mpHardware->getStrongCount(), mpHardware.get()
    );
    mpHardware.clear();
}


/******************************************************************************
*
*******************************************************************************/
void
CmdQueThread::
postCommand(Command const& rCmd)
{
    Mutex::Autolock autoLock(mCmdLock);

    MY_LOGD("+ tid(%d), que size(%d)", ::gettid(), mCmdQue.size());
    if  ( ! mCmdQue.empty() )
    {
        Command const& rBegCmd = *mCmdQue.begin();
        MY_LOGW("que size:%d with begin cmd::%s", mCmdQue.size(), rBegCmd.name());
    }

    mCmdQue.push_back(rCmd);
    mCmdCond.broadcast();

    MY_LOGD("- new command::%s (ext1, ext2)=(0x%x, 0x%x)", rCmd.name(), rCmd.u4Ext1, rCmd.u4Ext2);
}


/******************************************************************************
*
*******************************************************************************/
bool
CmdQueThread::
getCommand(Command& rCmd)
{
    bool ret = false;

    Mutex::Autolock autoLock(mCmdLock);

    MY_LOGD("+ tid(%d), que size(%d)", ::gettid(), mCmdQue.size());
    //
    //  Wait until the queue is not empty or this thread will exit.
    while   ( mCmdQue.empty() && ! exitPending() )
    {
        status_t status = mCmdCond.wait(mCmdLock);
        if  ( NO_ERROR != status )
        {
            MY_LOGW("wait status(%d), que size(%d), exitPending(%d)", status, mCmdQue.size(), exitPending());
        }
    }
    //
    if  ( ! mCmdQue.empty() )
    {
        //  If the queue is not empty, take the first command from the queue.
        ret = true;
        rCmd = *mCmdQue.begin();
        mCmdQue.erase(mCmdQue.begin());
        MY_LOGD("command:%s (ext1, ext2)=(0x%x, 0x%x)", rCmd.name(), rCmd.u4Ext1, rCmd.u4Ext2);
    }

    MY_LOGD("- tid(%d), que size(%d), ret(%d)", ::gettid(), mCmdQue.size(), ret);
    return  ret;
}


/******************************************************************************
*
*******************************************************************************/
// Ask this object's thread to exit. This function is asynchronous, when the
// function returns the thread might still be running. Of course, this
// function can be called from a different thread.
void
CmdQueThread::
requestExit()
{
    MY_LOGD("tid(%d)", ::gettid());
    Thread::requestExit();
    postCommand(Command(Command::eID_EXIT));
}


/******************************************************************************
*
*******************************************************************************/
// Good place to do one-time initializations
status_t
CmdQueThread::
readyToRun()
{
    ::prctl(PR_SET_NAME,"Camera@CmdQue",0,0,0);
    //
    mi4Tid = ::gettid();
    //  thread policy & priority
    //  Notes:
    //      Even if pthread_create() with SCHED_OTHER policy, a newly-created thread 
    //      may inherit the non-SCHED_OTHER policy & priority of the thread creator.
    //      And thus, we must set the expected policy & priority after a thread creation.
    int const expected_policy = SCHED_OTHER;
    int const expected_priority = 0;
    int policy = 0, priority = 0;
    getPriority(policy, priority);
    if  ( expected_policy != policy ) {
        MY_LOGD("created policy(%d) != expected_policy(%d)", policy, expected_policy);
    }
    setPriority(expected_policy, expected_priority);
    getPriority(policy, priority);
    MY_LOGD("- (tid, policy, priority)=(%d, %d, %d)", mi4Tid, policy, priority);

    return NO_ERROR;
}


/******************************************************************************
*
*******************************************************************************/
// Derived class must implement threadLoop(). The thread starts its life
// here. There are two ways of using the Thread object:
// 1) loop: if threadLoop() returns true, it will be called again if
//          requestExit() wasn't called.
// 2) once: if threadLoop() returns false, the thread will exit upon return.
bool
CmdQueThread::
threadLoop()
{
    Command cmd;
    if  ( getCommand(cmd) )
    {
        MY_LOGD(
            "tid(%d), mpHardware(%p), Command::%s (ext1, ext2)=(0x%x, 0x%x)"
            , mi4Tid, mpHardware.get(), cmd.name(), cmd.u4Ext1, cmd.u4Ext2
        );
        switch  (cmd.eId)
        {
        case Command::eID_UNKNOWN:
            MY_LOGD("Command::eID_UNKNOWN");
            break;
        //
        case Command::eID_EXIT:
            MY_LOGD("Command::eID_EXIT");
            break;
        //
        default:
            if  ( mpHardware.get() )
            {
                (*mpHardware.*mfpCommandHandler)(&cmd);
            }
            else
            {
                MY_LOGE("cannot handle cmd due to mpHardware==NULL");
            }
            break;
        }
    }

    MY_LOGD("- tid(%d), mpHardware(%p)", mi4Tid, mpHardware.get());
    return  true;
}


}; // namespace android

