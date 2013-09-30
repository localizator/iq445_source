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

#define LOG_TAG "MtkCam/DisplayAdapter"
//
#include <semaphore.h>
#include <sys/prctl.h>
#include <linux/rtpm_prio.h>
//
#include <camera/CameraParameters.h>
#include <ui/GraphicBuffer.h>
#include <ui/GraphicBufferMapper.h>
//
#include "Utils/inc/CamUtils.h"
#include <cutils/properties.h>
#include "inc/IDisplayAdapter.h"
#include "PVWDisplayAdapter.h"
#include "TgtBufQue.h"
#include "CameraProfile.h"
//
using namespace android;


/******************************************************************************
*
*******************************************************************************/
#define ENABLE_LOG                  (0)


/******************************************************************************
*
*******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[TgtBufQue::%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[TgtBufQue::%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[TgtBufQue::%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[TgtBufQue::%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[TgtBufQue::%s] "fmt, __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, arg...)    if (cond) { MY_LOGV(arg); }
#define MY_LOGD_IF(cond, arg...)    if (cond) { MY_LOGD(arg); }
#define MY_LOGI_IF(cond, arg...)    if (cond) { MY_LOGI(arg); }
#define MY_LOGW_IF(cond, arg...)    if (cond) { MY_LOGW(arg); }
#define MY_LOGE_IF(cond, arg...)    if (cond) { MY_LOGE(arg); }


/******************************************************************************
*
*******************************************************************************/
#define CAMERA_GRALLOC_USAGE    ( GRALLOC_USAGE_SW_READ_RARELY  \
                                | GRALLOC_USAGE_SW_WRITE_NEVER  \
                                | GRALLOC_USAGE_HW_TEXTURE )

#define TGTBUFQUE_SENSOR_DEGREE_MAIN    "debug.camera.main"
#define TGTBUFQUE_SENSOR_DEGREE_0       "0"
#define TGTBUFQUE_SENSOR_DEGREE_90      "90"
#define TGTBUFQUE_SENSOR_DEGREE_180     "180"
#define TGTBUFQUE_SENSOR_DEGREE_270     "270"

#define TGTBUFQUE_STEREO_3D_ENABLE      "Camera.Stereo.3D.Enable"
#define TGTBUFQUE_STEREO_3D_ENABLE_N    "0"
#define TGTBUFQUE_STEREO_3D_ENABLE_Y    "1"

/******************************************************************************
*
*******************************************************************************/
#if TGTBUFQUE_DEBUG_ON
bool
SCOPE_TGTBUFQUE::
TgtBufQue::
dbgCompare(List<BufInfo>& rFifoDbgQue, BufInfo const& rBuf)
{
    MY_LOGD("[TGTBUFQUE_DEBUG] + rFifoDbgQue.size(%d)", rFifoDbgQue.size());
    //
    if  ( rFifoDbgQue.empty() )
    {
        MY_LOGW("[TGTBUFQUE_DEBUG] rFifoDbgQue is empty");
        return  false;
    }
    //
    BufInfo const head = *rFifoDbgQue.begin();
    rFifoDbgQue.erase(rFifoDbgQue.begin());
    if  ( head != rBuf )
    {
        MY_LOGW("[TGTBUFQUE_DEBUG] the current buf to queue != the head in debug que");
        MY_LOGW("[TGTBUFQUE_DEBUG] BufHndlPtr: (current,head)=(%p,%p)", rBuf.getBufHndlPtr(), head.getBufHndlPtr());
        MY_LOGW("[TGTBUFQUE_DEBUG] Buffer: (current,head)=(%p,%p)", rBuf.getBuf(), head.getBuf());
        return  false;
    }
    MY_LOGD("[TGTBUFQUE_DEBUG] - rFifoDbgQue.size(%d)", rFifoDbgQue.size());
    return  true;
}


void
SCOPE_TGTBUFQUE::
TgtBufQue::
dbgDump(List<BufInfo> const& rFifoDbgQue, char const*const pszDbgText)
{
    MY_LOGD("[TGTBUFQUE_DEBUG][%s] + rFifoDbgQue.size(%d)", pszDbgText, rFifoDbgQue.size());
    if  ( ! rFifoDbgQue.empty() )
    {
        MY_LOGW("[TGTBUFQUE_DEBUG] NOT EMPTY rFifoDbgQue.size(%d)", rFifoDbgQue.size());
        for (List<BufInfo>::const_iterator it = rFifoDbgQue.begin(); it != rFifoDbgQue.end(); it++)
        {
            MY_LOGW("[TGTBUFQUE_DEBUG] (BufHndlPtr/Buf)=(%p/%p)", it->getBufHndlPtr(), it->getBuf());
        }
    }
    MY_LOGD("[TGTBUFQUE_DEBUG][%s] -", pszDbgText);
}
#endif


/******************************************************************************
*
*******************************************************************************/
SCOPE_TGTBUFQUE::
TgtBufQue::
TgtBufQue()
    : Thread(false)
    //
    , mCmdQue()
    , mCmdQueMutex()
    , mCmdQueCond()
    //
    , mLBQue()
    , mLBQueMutex()
    , mLBQueCond()
    //
    , mWndMutex()
    , mPreviewWindow(NULL)
    , mi4WndBufCount_InTotal(0)
    , mi4WndBufCount_Locked(0)
#if TGTBUFQUE_DEBUG_ON
    , mWndFifoDbgQue()
#endif
    //
    , mi4WndFmt(0)
    , mi4WndWidth(0)
    , mi4WndHeight(0)
    //
    , mi4Tid(0)
{
}


/******************************************************************************
*
*******************************************************************************/
SCOPE_TGTBUFQUE::
TgtBufQue::
~TgtBufQue()
{
    MY_LOGD("+ tid(%d)", ::gettid());
//------------------------------------------------------------------------------
#if TGTBUFQUE_DEBUG_ON
    dbgDump(mWndFifoDbgQue, "~TgtBufQue()");
    mWndFifoDbgQue.clear();
#endif
//------------------------------------------------------------------------------
}


/******************************************************************************
* Ask this object's thread to exit. This function is asynchronous, when the
* function returns the thread might still be running. Of course, this
* function can be called from a different thread.
*******************************************************************************/
void
SCOPE_TGTBUFQUE::
TgtBufQue::
requestExit()
{
    MY_LOGD("+ tid(%d)", ::gettid());

    Thread::requestExit();

    //  broadcast all conditions.
    //  (1)
    putCommand(Command(Command::eID_EXIT));
    //
    //  (2)
    {
        Mutex::Autolock autoLock(mLBQueMutex);
        mLBQueCond.broadcast();
    }
}


/******************************************************************************
* Good place to do one-time initializations
*******************************************************************************/
status_t
SCOPE_TGTBUFQUE::
TgtBufQue::
readyToRun()
{
    ::prctl(PR_SET_NAME, "Camera@Display", 0, 0, 0);
    //
    mi4Tid = gettid();

    //  thread policy & priority
    //  Notes:
    //      Even if pthread_create() with SCHED_OTHER policy, a newly-created thread 
    //      may inherit the non-SCHED_OTHER policy & priority of the thread creator.
    //      And thus, we must set the expected policy & priority after a thread creation.
    int const policy = SCHED_RR;
    int const priority = RTPM_PRIO_CAMERA_PREVIEW;
    //
    struct sched_param sched_p;
    ::sched_getparam(0, &sched_p);
    //
    //  set
    sched_p.sched_priority = priority;  //  Note: "priority" is real-time priority.
    ::sched_setscheduler(0, policy, &sched_p);
    //
    //  get
    ::sched_getparam(0, &sched_p);
    //
    MY_LOGD(
        "- tid:(%d, %d), policy:(expect, result)=(%d, %d), priority:(expect, result)=(%d, %d)", 
        ::gettid(), mi4Tid, 
        policy, ::sched_getscheduler(0), 
        priority, sched_p.sched_priority
    );
    return NO_ERROR;
}


/******************************************************************************
*
* Derived class must implement threadLoop(). The thread starts its life
* here. There are two ways of using the Thread object:
* 1) loop: if threadLoop() returns true, it will be called again if
*          requestExit() wasn't called.
* 2) once: if threadLoop() returns false, the thread will exit upon return.
*******************************************************************************/
bool
SCOPE_TGTBUFQUE::
TgtBufQue::
threadLoop()
{
    Command cmd;
    if  ( getCommand(cmd) )
    {
        MY_LOGD_IF(ENABLE_LOG, "tid(%d), Command::%s (ext1, ext2)=(0x%x, 0x%x)", mi4Tid, cmd.name(), cmd.u4Ext1, cmd.u4Ext2);
        switch  (cmd.eId)
        {
        case Command::eID_DEQUE_WINDOW_ONE:
            dequeWindowOne();
            if  ( cmd.u4Ext1 )
            {
                ::sem_post(reinterpret_cast<sem_t*>(cmd.u4Ext1));
            }
            break;
        //
        case Command::eID_DEQUE_WINDOW_ALL:
            dequeWindowAll();
            if  ( cmd.u4Ext1 )
            {
                ::sem_post(reinterpret_cast<sem_t*>(cmd.u4Ext1));
            }
            break;
        //
        case Command::eID_CANCEL_WINDOW_ALL:
            cancelWindowAll();
            if  ( cmd.u4Ext1 )
            {
                ::sem_post(reinterpret_cast<sem_t*>(cmd.u4Ext1));
            }
            break;
        //
        case Command::eID_EXIT:
            MY_LOGD("Command::eID_EXIT");
            break;
        //
        default:
        case Command::eID_UNKNOWN:
            MY_LOGD("Command::eID_UNKNOWN");
            break;
        //
        }
    }

    MY_LOGD_IF(ENABLE_LOG, "- tid(%d)", mi4Tid);
    return  true;
}


/******************************************************************************
*
*******************************************************************************/
void
SCOPE_TGTBUFQUE::
TgtBufQue::
putCommand(Command const& rCmd)
{
    Mutex::Autolock autoLock(mCmdQueMutex);

    MY_LOGD_IF(ENABLE_LOG, "+ tid(%d), que size(%d)", ::gettid(), mCmdQue.size());

    if  ( ! mCmdQue.empty() )
    {
        Command const& rBegCmd = *mCmdQue.begin();
        MY_LOGW_IF(0, "que size:%d with begin cmd::%s", mCmdQue.size(), rBegCmd.name());
    }

    mCmdQue.push_back(rCmd);
    mCmdQueCond.broadcast();

    MY_LOGD_IF(ENABLE_LOG, "- new command::%s (ext1, ext2)=(0x%x, 0x%x)", rCmd.name(), rCmd.u4Ext1, rCmd.u4Ext2);
}


/******************************************************************************
*
*******************************************************************************/
bool
SCOPE_TGTBUFQUE::
TgtBufQue::
getCommand(Command& rCmd)
{
    bool ret = false;

    Mutex::Autolock autoLock(mCmdQueMutex);

    MY_LOGD_IF(ENABLE_LOG, "+ tid(%d), que size(%d)", ::gettid(), mCmdQue.size());
    //
    //  Wait until the queue is not empty or this thread will exit.
    while   ( mCmdQue.empty() && ! exitPending() )
    {
        status_t status = mCmdQueCond.wait(mCmdQueMutex);
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
        MY_LOGI_IF(ENABLE_LOG, "command:%s (ext1, ext2)=(0x%x, 0x%x)", rCmd.name(), rCmd.u4Ext1, rCmd.u4Ext2);
    }
    //
    MY_LOGD_IF(ENABLE_LOG, "- tid(%d), que size(%d), ret(%d)", ::gettid(), mCmdQue.size(), ret);
    return  ret;
}


/******************************************************************************
*
*******************************************************************************/
void
SCOPE_TGTBUFQUE::
TgtBufQue::
cancelWindow(BufInfo const& rBuf)
{
    CamProfile profile(__FUNCTION__, "TgtBufQue");
    //
    MY_LOGD("+ mi4WndBufCount_Locked(%d), rBuf.getBuf(%p)", mi4WndBufCount_Locked, rBuf.getBuf());
    //
    Mutex::Autolock autoLock(mWndMutex);
    //
    profile.print_overtime(1, "autoLock(mWndMutex)");

    if  ( 0 < mi4WndBufCount_Locked && NULL != mPreviewWindow )
    {
        //
        //  [1] unlock buffer before canceling.
        GraphicBufferMapper::get().unlock(rBuf.getBufHndl());
        //
        profile.print_overtime(1, "GraphicBufferMapper::unlock");
        //
        //  [2] cancel buffer.
        status_t err = mPreviewWindow->cancel_buffer(mPreviewWindow, rBuf.getBufHndlPtr());
        if  ( err )
        {
            MY_LOGW("cancel_buffer failed: %s (%d)", ::strerror(-err), -err);
        }
        //
        profile.print_overtime(1, "mPreviewWindow->cancel_buffer");
        //
        //  [3] Update the number of buffers dequeued from window.
        ::android_atomic_dec(&mi4WndBufCount_Locked);

//------------------------------------------------------------------------------
#if TGTBUFQUE_DEBUG_ON
        dbgCompare(mWndFifoDbgQue, rBuf);
#endif
//------------------------------------------------------------------------------
    }
    //
    MY_LOGD("- mi4WndBufCount_Locked(%d)", mi4WndBufCount_Locked);
}


/******************************************************************************
*
*******************************************************************************/
void
SCOPE_TGTBUFQUE::
TgtBufQue::
queueWindow(BufInfo const& rBuf, int64_t i8Timestamp)
{
    static DebugFps dbgFsp(__FUNCTION__);
    dbgFsp.checkFps();
    ///
    AutoCPTLog cptlog(Event_DispAdpt_queueWindow);
    CamProfile profile(__FUNCTION__, "TgtBufQue");
    //
    MY_LOGD_IF(ENABLE_LOG, "+ mi4WndBufCount_Locked(%d), rBuf.getBuf(%d)", mi4WndBufCount_Locked, rBuf.getBuf());
    //
    status_t    err = 0;
    //
    {
        //  [0]
        Mutex::Autolock autoLock(mWndMutex);
        //
        profile.print_overtime(1, "autoLock(mWndMutex)");
        //
        //  [1] unlock buffer before sending to display
        GraphicBufferMapper::get().unlock(rBuf.getBufHndl());
        //
        profile.print_overtime(1, "GraphicBufferMapper::unlock");
        //
        err = mPreviewWindow->set_timestamp(mPreviewWindow, i8Timestamp);
        if  ( err )
        {
            MY_LOGE("set_timestamp failed: %s (%d)", ::strerror(-err), -err);
        }
        //
        profile.print_overtime(2, "mPreviewWindow->set_timestamp");
        //
        //  [2] unlocks and post the buffer to display.
        CPTLog(Event_PW_enqueue_buffer, CPTFlagStart);
        err = mPreviewWindow->enqueue_buffer(mPreviewWindow, rBuf.getBufHndlPtr());
        CPTLog(Event_PW_enqueue_buffer, CPTFlagEnd);        
        if  ( err )
        {
            MY_LOGE("enqueue_buffer failed: %s (%d)", ::strerror(-err), -err);
        }
        //
        profile.print_overtime(10, "mPreviewWindow->enqueue_buffer");
        //
        //  [3] Update the number of buffers dequeued & locked from window.
        ::android_atomic_dec(&mi4WndBufCount_Locked);

//------------------------------------------------------------------------------
#if TGTBUFQUE_DEBUG_ON
        dbgCompare(mWndFifoDbgQue, rBuf);
#endif
//------------------------------------------------------------------------------
    }
    //
    //  [4] Request PIC to dequeue a buffer immediately.
    putCommand(Command(Command::eID_DEQUE_WINDOW_ONE));
    //
    MY_LOGD_IF(ENABLE_LOG, "-");
}


/******************************************************************************
*
*******************************************************************************/
bool
SCOPE_TGTBUFQUE::
TgtBufQue::
dequeWindow(BufInfo& rBuf)
{
    AutoCPTLog cptlog(Event_DispAdpt_dequeWindow);
    CamProfile profile(__FUNCTION__, "TgtBufQue");
    //
    bool            ret = false;
    status_t        err = 0;
    buffer_handle_t*phandle = NULL;
    void*           address = NULL;
    int             stride  = 0;    // dummy variable to get stride
    Rect const      bounds(mi4WndWidth, mi4WndHeight);
    //
    MY_LOGD_IF(ENABLE_LOG, "+");
    //
    //  [0]
    Mutex::Autolock autoLock(mWndMutex);
    //
    profile.print_overtime(1, "autoLock(mWndMutex)");
    //
    if  ( ! mPreviewWindow )
    {
        MY_LOGW("NULL mPreviewWindow");
        ret = true;     //  should we return true?
        goto lbExit;
    }
    //
    //  [1] dequeue_buffer
    CPTLog(Event_PW_dequeue_buffer, CPTFlagStart);
    err = mPreviewWindow->dequeue_buffer(mPreviewWindow, &phandle, &stride);
    profile.print_overtime(1, "mPreviewWindow->dequeue_buffer");
    CPTLog(Event_PW_dequeue_buffer, CPTFlagEnd);
    if  ( err || NULL == phandle )
    {
        MY_LOGW(
            "tid(%d), dequeue_buffer failed with phandle=%p: %s (%d), locked count(%d)", 
            ::gettid(), phandle, ::strerror(-err), -err, mi4WndBufCount_Locked
        );
        goto lbExit;
    }
    //
    //  [2] lock buffers
    err = mPreviewWindow->lock_buffer(mPreviewWindow, phandle);
    profile.print_overtime(1, "mPreviewWindow->lock_buffer");
    if  ( err )
    {
        MY_LOGE("[TgtBufQue::%s] lock_buffer failed: %s (%d)", ::strerror(-err), -err);
        mPreviewWindow->cancel_buffer(mPreviewWindow, phandle);
        goto lbExit;
    }
    //
    //  [3] Now let the graphics framework to lock the buffer, and provide
    //  us with the framebuffer data address.
    GraphicBufferMapper::get().lock(*phandle, CAMERA_GRALLOC_USAGE, bounds, &address);
    profile.print_overtime(1, "GraphicBufferMapper::lock");
    if  ( err )
    {
        MY_LOGE("[TgtBufQue::%s] GraphicBufferMapper.lock failed: %s (%d)", ::strerror(-err), -err);
        mPreviewWindow->cancel_buffer(mPreviewWindow, phandle);
        goto lbExit;
    }
    //
    //  [4] Update the number of buffers dequeued from window.
    ::android_atomic_inc(&mi4WndBufCount_Locked);
    //
    //  [5] Return the locked pair of phandle/address
    rBuf = BufInfo(address, phandle, stride);
    //
//------------------------------------------------------------------------------
#if TGTBUFQUE_DEBUG_ON
    mWndFifoDbgQue.push_back(rBuf);
#endif
//------------------------------------------------------------------------------
    //
    ret = true;
lbExit:
//    MY_LOGD_IF(ENABLE_LOG, "- ret(%d), (address, phandle)=(%p, %p)", ret, address, phandle);
    MY_LOGD_IF(ENABLE_LOG, "- ret(%d), (address, phandle)=(%p, %p), stride(%d), W/H=%d/%d", ret, address, phandle, stride, mi4WndWidth, mi4WndHeight);
    return  ret;
}


/******************************************************************************
*
*******************************************************************************/
void
SCOPE_TGTBUFQUE::
TgtBufQue::
queueLBQue(BufInfo const& rBuf)
{
    Mutex::Autolock autoLock(mLBQueMutex);
    //
    MY_LOGD_IF(ENABLE_LOG, "+ tid(%d), Locked-buf que size(%d)", ::gettid(), mLBQue.size());
    //
    mLBQue.push_back(rBuf);
    mLBQueCond.broadcast();
}


/******************************************************************************
*
*******************************************************************************/
bool
SCOPE_TGTBUFQUE::
TgtBufQue::
dequeLBQue(BufInfo& rBuf, bool const isNotToWait /*= false*/)
{
    bool ret = false;

    Mutex::Autolock autoLock(mLBQueMutex);
    //
    MY_LOGD_IF(ENABLE_LOG, "+ tid(%d), Locked-buf que size(%d)", ::gettid(), mLBQue.size());
    //
    //
    //  Wait until the queue is not empty or this thread will exit.
    while   ( mLBQue.empty() && ! exitPending() )
    {
        if  ( isNotToWait )
        {
            MY_LOGD("not to wait");
            break;
        }

        status_t status = mLBQueCond.wait(mLBQueMutex);
        if  ( NO_ERROR != status )
        {
            MY_LOGW("wait status(%d), Locked-buf que size(%d), exitPending(%d)", status, mLBQue.size(), exitPending());
        }
    }
    //
    if  ( ! mLBQue.empty() )
    {
        //  If the queue is not empty, take the first buffer from the queue.
        ret = true;
        rBuf = *mLBQue.begin();
        mLBQue.erase(mLBQue.begin());
        MY_LOGD_IF(ENABLE_LOG, "(hndl, buf)=(%p, %p)", rBuf.getBufHndlPtr(), rBuf.getBuf());
    }
    //
    MY_LOGD_IF(ENABLE_LOG, "- tid(%d), Locked-buf que size(%d), ret(%d)", ::gettid(), mLBQue.size(), ret);
    return  ret;
}


/******************************************************************************
*
*******************************************************************************/
bool
SCOPE_TGTBUFQUE::
TgtBufQue::
dequeWindowOne()
{
    MY_LOGD_IF(ENABLE_LOG, "+ tid(%d), locked count(%d)", ::gettid(), mi4WndBufCount_Locked);
    int32_t buf_count_to_dequeue = 0;
    {
        Mutex::Autolock autoLock(mWndMutex);
        buf_count_to_dequeue = mi4WndBufCount_InTotal - mi4WndBufCount_Locked;
    }
    //
    MY_LOGD_IF(ENABLE_LOG, "tid(%d), buf_count_to_dequeue(%d)", ::gettid(), buf_count_to_dequeue);
    //
    if  (0 < buf_count_to_dequeue)
    {
        BufInfo buf;
        if  ( dequeWindow(buf) )
        {
            //  Push the locked buffer into our target queue.
            queueLBQue(buf);
        }
    }
    //
    MY_LOGD_IF(ENABLE_LOG, "- tid(%d), locked count(%d)", ::gettid(), mi4WndBufCount_Locked);
    return  true;
}


/******************************************************************************
*
*******************************************************************************/
bool
SCOPE_TGTBUFQUE::
TgtBufQue::
dequeWindowAll()
{
    MY_LOGD_IF(ENABLE_LOG, "+ tid(%d)", ::gettid());
    int32_t buf_count_to_dequeue = 0;
    {
        Mutex::Autolock autoLock(mWndMutex);
        buf_count_to_dequeue = mi4WndBufCount_InTotal - mi4WndBufCount_Locked;
    }
    //
    MY_LOGD_IF(ENABLE_LOG, "tid(%d), buf_count_to_dequeue(%d)", ::gettid(), buf_count_to_dequeue);
    //
    for (int i = 0; ! exitPending() && i < buf_count_to_dequeue; i++)
    {
        BufInfo buf;
        if  ( ! dequeWindow(buf) )
        {
            MY_LOGW("dequeWindow returns false - total count(%d), locked count(%d)", mi4WndBufCount_InTotal, mi4WndBufCount_Locked);
            break;
        }
        //  Push the locked buffer into our target queue.
        queueLBQue(buf);
    }
    //
    MY_LOGD_IF(ENABLE_LOG, "- tid(%d), buf_count_to_dequeue(%d)", ::gettid(), buf_count_to_dequeue);
    return  true;
}


/******************************************************************************
*
*******************************************************************************/
bool
SCOPE_TGTBUFQUE::
TgtBufQue::
cancelWindowAll()
{
    MY_LOGD("+ tid(%d) mi4WndBufCount_Locked(%d), Locked-buf que size(%d)", ::gettid(), mi4WndBufCount_Locked, mLBQue.size());
    //
    status_t    err = 0;
    BufInfo     buf;
    //
    while   ( dequeLBQue(buf, true) )
    {
        //  [0]
        Mutex::Autolock autoLock(mWndMutex);

        if  ( 0 < mi4WndBufCount_Locked && NULL != mPreviewWindow )
        {
            //
            //  [1] unlock buffer before canceling.
            GraphicBufferMapper::get().unlock(buf.getBufHndl());
            //
            //  [2] cancel buffer.
            err = mPreviewWindow->cancel_buffer(mPreviewWindow, buf.getBufHndlPtr());
            if  ( err )
            {
                MY_LOGW("cancel_buffer failed: %s (%d)", ::strerror(-err), -err);
            }
            //
            //  [3] Update the number of buffers dequeued from window.
            ::android_atomic_dec(&mi4WndBufCount_Locked);

//------------------------------------------------------------------------------
#if TGTBUFQUE_DEBUG_ON
            dbgCompare(mWndFifoDbgQue, buf);
#endif
//------------------------------------------------------------------------------
        }
    }
    //
    if  ( 0 < mi4WndBufCount_Locked && ! mLBQue.empty() )
    {
        MY_LOGW("cannot cancel all: mi4WndBufCount_Locked(%d), mLBQue.size(%d)", mi4WndBufCount_Locked, mLBQue.size());
        //
        mLBQue.clear();
        Mutex::Autolock autoLock(mWndMutex);
        mi4WndBufCount_Locked = 0;
    }
    //
    return  true;
}


/******************************************************************************
*  Notes:
*  (1) Return false if NULL window is passed into.
*  (2) Return false if a window has been set before. In this case,  callers must
*      make sure to return all dequeued buffers, and then call cleanupQueue().
*******************************************************************************/
bool
SCOPE_TGTBUFQUE::
TgtBufQue::
setWindow(
    preview_stream_ops*const window, 
    int32_t const   width, 
    int32_t const   height, 
    int32_t const   format, 
    int32_t const   desired_buf_count
)
{
    bool        ret = false;
    status_t    err = 0;
    int32_t     min_undequeued_buf_count = 0;
    char value[256] = {'\0'};
    bool Enable3D;
    uint32_t SensorDegree = 0;
    //
    if  ( ! window )
    {
        MY_LOGE("NULL window passed into");
        return  false;
    }
    //
    if  ( 0 >= width || 0 >= height || 0 >= desired_buf_count )
    {
        MY_LOGE("bad arguments - (width, height, desired_buf_count)=(%d, %d, %d)", width, height, desired_buf_count);
        return  false;
    }
    //
    //
    Mutex::Autolock autoLock(mWndMutex);
    if  ( 0 != mi4WndBufCount_Locked )
    {
        MY_LOGE(
            "locked window buffer count (%d) != 0, "
            "callers must return all dequeued buffers, "
            "and then call cleanupQueue()"
            , mi4WndBufCount_Locked
        );
        goto lbExit;
    }
    //
    mPreviewWindow          = window;
    //
    mi4WndFmt               = format;
    mi4WndWidth             = width;
    mi4WndHeight            = height;
    //
    //  Set gralloc usage bits for window. 
    //
    property_get(TGTBUFQUE_SENSOR_DEGREE_MAIN,value,TGTBUFQUE_SENSOR_DEGREE_0);
    SensorDegree = (atoi(value));
    MY_LOGD("TGTBUFQUE_SENSOR_DEGREE_MAIN(%d)",SensorDegree);
    //
    property_get(TGTBUFQUE_STEREO_3D_ENABLE,value,TGTBUFQUE_STEREO_3D_ENABLE_N);
    Enable3D = (bool)(atoi(value));
    MY_LOGD("TGTBUFQUE_STEREO_3D_ENABLE(%d)",Enable3D);
    //
    if(Enable3D)
    {
        //Here suppose all sensor will output "side by side" format in preview mode.
        if(SensorDegree == 0 || SensorDegree == 180)
        {
            err = mPreviewWindow->set_usage(mPreviewWindow, CAMERA_GRALLOC_USAGE|GRALLOC_USAGE_S3D_SIDE_BY_SIDE);
        }
        else
        {
            err = mPreviewWindow->set_usage(mPreviewWindow, CAMERA_GRALLOC_USAGE|GRALLOC_USAGE_S3D_TOP_AND_BOTTOM);
        }

    }
    else
    {
    err = mPreviewWindow->set_usage(mPreviewWindow, CAMERA_GRALLOC_USAGE);
    }
    //
    if  ( err )
    {
        MY_LOGE("set_usage failed: %s (%d)", ::strerror(-err), -err);
        if  ( ENODEV == err )
        {
            MY_LOGD("Preview surface abandoned");
            mPreviewWindow = NULL;
        }
        goto lbExit;
    }
    //
    //  Get minimum undequeue buffer count
    err = mPreviewWindow->get_min_undequeued_buffer_count(
            mPreviewWindow, 
            &min_undequeued_buf_count
        );
    if  ( err )
    {
        MY_LOGE("get_min_undequeued_buffer_count failed: %s (%d)", ::strerror(-err), -err);
        if ( ENODEV == err )
        {
            MY_LOGD("Preview surface abandoned!");
            mPreviewWindow = NULL;
        }
        goto lbExit;
    }
    //
    //  Set the number of buffers needed for display.
    MY_LOGI(
        "set_buffer_count(%d) = desired_buf_count(%d) + min_undequeued_buf_count(%d)", 
        desired_buf_count+min_undequeued_buf_count, desired_buf_count, min_undequeued_buf_count
    );
    err = mPreviewWindow->set_buffer_count(mPreviewWindow, desired_buf_count+min_undequeued_buf_count);
    if  ( err )
    {
        MY_LOGE("set_buffer_count failed: %s (%d)", ::strerror(-err), -err);
        if ( ENODEV == err )
        {
            MY_LOGD("Preview surface abandoned!");
            mPreviewWindow = NULL;
        }
        goto lbExit;
    }
    //
    //  Set window geometry
    err = mPreviewWindow->set_buffers_geometry(
            mPreviewWindow, width, height, format
        );
    if  ( err )
    {
        MY_LOGE("set_buffers_geometry failed: %s (%d)", ::strerror(-err), -err);
        if ( ENODEV == err )
        {
            MY_LOGD("Preview surface abandoned!");
            mPreviewWindow = NULL;
        }
        goto lbExit;
    }
    //
    mi4WndBufCount_InTotal  = desired_buf_count;
    mi4WndBufCount_Locked   = 0;
    //
    ret = true;
lbExit:
    return  ret;
}


/******************************************************************************
* Notes:
*   Callers must make sure to return all dequeued buffers, and then call 
*   cleanupQueue().
*******************************************************************************/
bool
SCOPE_TGTBUFQUE::
TgtBufQue::
cleanupQueue()
{
    MY_LOGD_IF(ENABLE_LOG, "+");
    //
    sem_t semDone;
    ::sem_init(&semDone, 0, 0);
    //
    putCommand(Command(Command::eID_CANCEL_WINDOW_ALL, reinterpret_cast<uint32_t>(&semDone)));
    //
    ::sem_wait(&semDone);
    ::sem_destroy(&semDone);
    //
//------------------------------------------------------------------------------
#if TGTBUFQUE_DEBUG_ON
    dbgDump(mWndFifoDbgQue, "cleanupQueue()");
#endif
//------------------------------------------------------------------------------
    //
    MY_LOGD_IF(ENABLE_LOG, "-");
    return  true;
}


/******************************************************************************
*
*******************************************************************************/
bool
SCOPE_TGTBUFQUE::
TgtBufQue::
prepareQueue()
{
    AutoCPTLog cptlog(Event_DispAdpt_prepareQueue);
    MY_LOGD_IF(ENABLE_LOG, "+");
    //
    sem_t semDone;
    ::sem_init(&semDone, 0, 0);
    //
    putCommand(Command(Command::eID_DEQUE_WINDOW_ALL, reinterpret_cast<uint32_t>(&semDone)));
    //
    ::sem_wait(&semDone);
    ::sem_destroy(&semDone);
    //
    MY_LOGD_IF(ENABLE_LOG, "-");
    return  true;
}


/******************************************************************************
*
*******************************************************************************/
bool
SCOPE_TGTBUFQUE::
TgtBufQue::
prepareQueueAsync()
{
    MY_LOGD_IF(ENABLE_LOG, "+");
    //
    putCommand(Command(Command::eID_DEQUE_WINDOW_ALL));
    //
    MY_LOGD_IF(ENABLE_LOG, "-");
    return  true;
}


/******************************************************************************
*
*******************************************************************************/
bool
SCOPE_TGTBUFQUE::
TgtBufQue::
dequeueBuffer(BufInfo& rBuf)
{
    AutoCPTLog cptlog(Event_DispAdpt_dequeueBuffer);
    bool ret = false;
    //
    ret = dequeLBQue(rBuf, true);
    if  ( ! ret )
    {
        sem_t semDone;
        ::sem_init(&semDone, 0, 0);
        //
        putCommand(Command(Command::eID_DEQUE_WINDOW_ONE, reinterpret_cast<uint32_t>(&semDone)));
        //
        ::sem_wait(&semDone);
        ::sem_destroy(&semDone);
        //
        ret = dequeLBQue(rBuf, true);
    }
    return  ret;
}

