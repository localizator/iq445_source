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

#ifndef _MTK_DISPLAY_ADAPTER_TARGET_BUF_QUEUE_H_
#define _MTK_DISPLAY_ADAPTER_TARGET_BUF_QUEUE_H_
//
#include <utils/Errors.h>
#include <utils/RefBase.h>
#include <utils/threads.h>
#include <utils/String8.h>
#include <utils/List.h>
//
#include <hardware/camera.h>
#include <system/camera.h>


/******************************************************************************
*
*******************************************************************************/
#define TGTBUFQUE_DEBUG_ON  (0)


/******************************************************************************
*
*******************************************************************************/
namespace android {
/******************************************************************************
*
*******************************************************************************/
#define SCOPE_TGTBUFQUE     PVWDisplayAdapter


/******************************************************************************
*
*******************************************************************************/
class   SCOPE_TGTBUFQUE::TgtBufQue : public Thread 
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Buffer Info.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////
    struct BufInfo : public SCOPE_TGTBUFQUE::IQueBufInfo
    {
    public:     ////    Operations.
        virtual void*               getBuf() const                              { return mBuf; }
        virtual void                setBuf(void*const pbuf)                     { mBuf = pbuf; }
        inline  buffer_handle_t     getBufHndl() const                          { return *mBufHndlPtr; }
        inline  buffer_handle_t*    getBufHndlPtr() const                       { return mBufHndlPtr; }
        inline  void                setBufHndlPtr(buffer_handle_t*const pbh)    { mBufHndlPtr = pbh;  }
        inline  int                 getStride() const                           { return mStride; }
        //
        bool                        operator==(BufInfo const& rhs) const {
                                        return rhs.getBufHndlPtr() == getBufHndlPtr()
                                            && rhs.getBuf() == getBuf()
                                            && rhs.getStride() == getStride()
                                                ;
                                    }
        bool                        operator!=(BufInfo const& rhs) const { return ! (rhs == *this); }

        //
    public:     ////    Operations.
        BufInfo(void*const pbuf = NULL, buffer_handle_t*const pbh = NULL, int stride = 0)
            : mBufHndlPtr(pbh)
            , mBuf(pbuf)
            , mStride(stride)
        {}
        //
    protected:  ////    Fields.
        buffer_handle_t*            mBufHndlPtr;    //  Pointer to the locked buffer handle.
        void*                       mBuf;           //  Pointer to the locked buffer base address.
        int                         mStride;
        //
    };

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Commands.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////
    struct Command
    {
        //  Command ID
        enum ID
        {
            eID_UNKNOWN      = 0, 
            eID_EXIT, 
            eID_DEQUE_WINDOW_ONE, 
            eID_DEQUE_WINDOW_ALL, 
            eID_CANCEL_WINDOW_ALL, 
            eID_NUM
        };
        //
        char const*name() const
        {
            #define CASE_ID_NAME(x) case x: return #x
            switch  (eId)
            {
            CASE_ID_NAME(eID_UNKNOWN);
            CASE_ID_NAME(eID_EXIT);
            CASE_ID_NAME(eID_DEQUE_WINDOW_ONE);
            CASE_ID_NAME(eID_DEQUE_WINDOW_ALL);
            CASE_ID_NAME(eID_CANCEL_WINDOW_ALL);
            default:
                break;
            }
            #undef CASE_ID_NAME
            return  "";
        }
        //
        //  Operations.
        Command(ID _eId = eID_UNKNOWN, uint32_t _u4Ext1 = 0, uint32_t _u4Ext2 = 0)
            : eId(_eId), u4Ext1(_u4Ext1), u4Ext2(_u4Ext2)
        {}
        //  Data Members.
        ID          eId;
        uint32_t    u4Ext1;
        uint32_t    u4Ext2;
    };

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
                            TgtBufQue();
                            ~TgtBufQue();
    /**
     *  Notes:
     *  (1) Return false if NULL window is passed into.
     *  (2) Return false if a window has been set before. In this case, 
     *      callers must make sure to return all dequeued buffers, and then
     *      call cleanupQueue().
     */
    bool                    setWindow(
                                preview_stream_ops*const window, 
                                int32_t const   width, 
                                int32_t const   height, 
                                int32_t const   format, 
                                int32_t const   desired_buf_count
                            );
    /**
     *  Notes:
     *      Callers must make sure to return all dequeued buffers, and then
     *      call cleanupQueue().
     */
    bool                    cleanupQueue();
    /**
     *
     */
    bool                    prepareQueue();
    bool                    prepareQueueAsync();
    //
    inline  void            cancelBuffer(BufInfo const& rBuf)   { cancelWindow(rBuf); }
    inline  void            enqueueBuffer(BufInfo const& rBuf, int64_t i8Timestamp)  { queueWindow(rBuf, i8Timestamp); }
    bool                    dequeueBuffer(BufInfo& rBuf);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Command Queue.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////
    typedef List<Command>   CmdQue_t;
    CmdQue_t                mCmdQue;
    Mutex                   mCmdQueMutex;
    Condition               mCmdQueCond;    //  Condition to wait: mCmdQue.empty()

protected:  ////    Operations.
    void                    putCommand(Command const& rCmd);
    bool                    getCommand(Command& rCmd);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Locked Buffer Queue.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////
    typedef List<BufInfo>   LBQue_t;
    LBQue_t                 mLBQue;
    Mutex                   mLBQueMutex;
    Condition               mLBQueCond;         //  Condition to wait: mvLBQue.empty()

protected:  ////    Operations.
    void                    queueLBQue(BufInfo const& rBuf);
    bool                    dequeLBQue(BufInfo& rBuf, bool const isNotToWait = false);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Window Queuing.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////
    Mutex                   mWndMutex;
    //
    preview_stream_ops_t*   mPreviewWindow;
    //
    int32_t                 mi4WndBufCount_InTotal;
    volatile int32_t        mi4WndBufCount_Locked;  //  mi4WndBufCount_Locked <= mi4WndBufCount_InTotal
    //
#if TGTBUFQUE_DEBUG_ON
    List<BufInfo>           mWndFifoDbgQue;
    //
    bool                    dbgCompare(List<BufInfo>& rFifoDbgQue, BufInfo const& rBuf);
    void                    dbgDump(List<BufInfo> const& rFifoDbgQue, char const*const pszDbgText);
#endif

protected:  ////    Operations.
    void                    cancelWindow(BufInfo const& rBuf);
    void                    queueWindow(BufInfo const& rBuf, int64_t i8Timestamp);
    bool                    dequeWindow(BufInfo& rBuf);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interaction between Locked Buffer Queue and Window Queuing.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////
    bool                    dequeWindowOne();
    bool                    dequeWindowAll();
    bool                    cancelWindowAll();

protected:  ////    Window Info.
    int32_t                 mi4WndFmt;
    int32_t                 mi4WndWidth;
    int32_t                 mi4WndHeight;

    struct DebugFps
    {
        char const*const    mName;
        int32_t             mFrameCount;
        nsecs_t             mStartTime;
        nsecs_t             mEndTime;
        //
                            DebugFps(char const*const name)
                                : mName(name), mFrameCount(0), mStartTime(systemTime()), mEndTime(systemTime())
                            {}
        void                checkFps()
                            {
                                mFrameCount++;
                                mEndTime = systemTime();
                                if  ( mFrameCount >= 60 )
                                {
                                    nsecs_t const diff = mEndTime - mStartTime;
                                    CAM_LOGD("[%s] fps: %3f = %d / %lld ns", mName, ((float)1000000000LL*mFrameCount)/diff, mFrameCount, diff);
                                    mFrameCount = 0;
                                    mStartTime = mEndTime;
                                }
                            }
    };

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Operations in base class Thread
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////
    // Ask this object's thread to exit. This function is asynchronous, when
    // the function returns the thread might still be running. Of course, 
    // this function can be called from a different thread.
    virtual void            requestExit();

    // Good place to do one-time initializations
    virtual status_t        readyToRun();

private:    ////
    // Derived class must implement threadLoop(). The thread starts its life
    // here. There are two ways of using the Thread object:
    // 1) loop: if threadLoop() returns true, it will be called again if
    //          requestExit() wasn't called.
    // 2) once: if threadLoop() returns false, the thread will exit upon return.
    virtual bool            threadLoop();

//------------------------------------------------------------------------------
private:    ////    Attributes.
    int32_t                 mi4Tid;
    //
public:     ////    Attributes.
    inline  int32_t         getTid() const { return mi4Tid; }
};


}; // namespace android
#endif  //_MTK_DISPLAY_ADAPTER_TARGET_BUF_QUEUE_H_

