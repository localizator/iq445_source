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

#ifndef _FAKECCCI_H_
#define _FAKECCCI_H_



#include "AudioYusuDef.h"

namespace android {

/*
#if defined(PC_EMULATION)
#include "windows.h"
#else
#include "unistd.h"
#include "pthread.h"
#endif
*/

#define FAKECCCI_PLAY_FRAME_LEN  320

/* constant definition */
extern const uint16 sineTable[320];

class FakeCCCI
{
private:

   uint32 fHdl;                  /* file handle for CCCI user space interface */
   uint32 mMsg[4];



public:

   uint32 mSendMsg;

#if defined(PC_EMULATION)
   CRITICAL_SECTION fMsgMutex;

   HANDLE hCCCIRecThread;
   HANDLE hCCCIPlayThread;

   HANDLE hEventDataOK;
   HANDLE hEventSendBack;
   HANDLE hEventPlayDataNotify;
#else
   pthread_mutex_t fMsgMutex;

   pthread_t hCCCIRecThread;
   pthread_t hCCCIPlayThread;

   pthread_cond_t  hEventSendBack;
   pthread_cond_t  hEventDataOK;
   pthread_cond_t  hEventPlayDataNotify;
   pthread_mutex_t fdataMutex;
   pthread_mutex_t fbkMutex;
   pthread_mutex_t fpdnMutex;
   int32 fbkMsgFlag;
   int32 fdataMsgFlag;
   int32 fpdnMsgFlag;
#endif

   // recording thread use
   int8  mRunFlag;

   // playback thread use
   int8  mpRunFlag;

   int8* pBufBase;     /* a pointer to fake share buffer */
   int8* pM2ABufBase;
   int8* pA2MBufBase;
   uint32 offset;
   uint32 validDataLen;

   // playback thread use
   uint32 playDataOffset;
   uint32 playDataLen;


   FakeCCCI();
   ~FakeCCCI();
   uint32 GetBaseAddress();
   bool SendMessage(uint32 *pMsg);
   bool ReadMessage(uint32 *pMsg);
   bool CreateCCCIFakeRecThread();
   bool CreateCCCIFakePlayThread();
   void MessageLock();
   void MessageUnLock();
   int32 FakeCCCI_RecSendbackEvent_Set();
   int32 FakeCCCI_RecSendbackEvent_Wait();
   int32 FakeCCCI_ModemSideEvent_Set();
   int32 FakeCCCI_ModemSideEvent_Wait();
   int32 FakeCCCI_PlayDataNotifyEvent_Set();
   int32 FakeCCCI_PlayDataNotifyEvent_Wait();
};

}; // namespace android

#endif   //_AUDIOCCCI_H_
