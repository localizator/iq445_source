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

#ifndef _AUDIO_YUSU_CCCI_H_
#define _AUDIO_YUSU_CCCI_H_


#include <stdio.h>
#include "ccci.h"
#include <pthread.h>

#include "AudioYusuDef.h"


#if defined(PC_EMULATION)
#include "windows.h"
#endif

#if defined(FAKE_CCCI)
#include "fake_ccci.h"
#endif

namespace android {

/* constant definition */
#define CCCI_PCM_SHARE_BUF_LEN      16384
#define M2A_SHARE_BUF_LEN           ( CCCI_PCM_SHARE_BUF_LEN >> 1 )
#define A2M_SHARE_BUF_LEN           ( CCCI_PCM_SHARE_BUF_LEN >> 1 )
#define A2M_SHARE_BUF_EMPAREA_LEN   512

#define A2M_SHARE_BUF_PCMAREA_LEN   A2M_SHARE_BUF_LEN


//For VT case, the CCCI message for every 20ms, UL/DL have 2 CCCI message (Put to Speaker / Get from Mic)
//For BGS off ack message, the worst case maybe pending 150 ms. And for other change device control. (BGSoff,2WAY off,SPH off,...)
//The total message maybe > 20 for this period. So enlarge the total CCCI message queue.
//For the CCCI queue in CCCI kernel driver, the size is 60.
//For the CCCI queue in Modem side, the size is 32.
//Modem side would keep on optimized the BGS off ack period.
#define AUDCCCI_MAX_QUEUE_LEN       60

   // for VT + BG Sound case (A2M Shared buffer usage)
   // BG Sound use address 0~1408(BGS use 1408 bytes). PCM4WAY_play use address 1408~2048. (4WAY playback path: (640)320+320 bytes)
   // But for the better solution, A2M shared buffer should use ring buffer.
#define A2M_SHARED_BUFFER_OFFSET  (1408)

// for VT + BG Sound + Set Speech Parameter case (A2M Shared buffer usage)
// BG Sound use address 0~1408(BGS use 1408 bytes). PCM4WAY_play use address 1408~2048. (4WAY playback path: (640)320+320 bytes)
// Speech Par use address1408~3455. (The overlap of 2way and sph par should be safe.)
// But for the better solution, A2M shared buffer should use ring buffer.
#define SPH_PAR_A2M_SHARED_BUFFER_OFFSET  (1408)

/* used to pack CCCI messages, MSG1 corresponds to 1-parameter, MSG2 corresponds to 2-parameter messages */
/*
#define CCCI_MSG1( Function, Param1) (((unsigned int)Function<<0x18)|(((unsigned int)Param1&0xffffff)))
#define CCCI_MSG2( Function, Param1, Param2)   (((unsigned int)Function<<0x18)|(((unsigned int)Param1&0xfff)<<0xC)|(((unsigned int)Param2&0xfff)))

#define CCCI_MESSAGE_TYPE_PARSER(CCCI_MESSAGE) (((uint32)CCCI_MESSAGE)>>28)
#define CCCI_MESSAGE_FUNCTION_PARSER(CCCI_MESSAGE) ( ((uint32)CCCI_MESSAGE) >> 24 )
#define CCCI_MESSAGE_OFFSET_PARSER(CCCI_MESSAGE) ( (((uint32)CCCI_MESSAGE) << 6) >> 19)
#define CCCI_MESSAGE_LENGTH_PARSER(CCCI_MESSAGE) ( (((uint32)CCCI_MESSAGE) << 19) >> 19)
*/

//Pack CCCI Message
extern uint32 CCCI_MSG1(uint32 Function, uint32 Param1);
extern uint32 CCCI_MSG2(uint32 Function, uint32 Param1, uint32 Param2);
extern uint32 CCCI_MSG3(uint32 Function, uint32 Param1, uint32 Param2);

//Parse CCCI Message
extern uint32 CCCI_MESSAGE_TYPE_PARSER(uint32 CCCI_MESSAGE);
extern uint32 CCCI_MESSAGE_FUNCTION_PARSER(uint32 CCCI_MESSAGE);
extern uint32 CCCI_MESSAGE_OFFSET_PARSER(uint32 CCCI_MESSAGE);
extern uint32 CCCI_MESSAGE_LENGTH_PARSER(uint32 CCCI_MESSAGE);
extern uint32 CCCI_MESSAGE_PARAM1_PARSER(uint32 CCCI_MESSAGE);


class AudioYusuHardware;

#if defined(FAKE_CCCI)
class FakeCCCI;
#endif

struct qele {
   uint32 msg;
   uint32 ack;
};

enum{
   NB_SPHCOEF = 0,
   WB_SPHCOEF,
   DMNR_SPHCOEF,
   HDREC_SPHCOEF,
   MAX_SPHCOEF
};

class AudioCCCI
{
private:

   // file handle for CCCI user space interface
   int32 fHdl;                   //for file read
   int32 fHdlAck;                //for file write
   int32 hPcmRx;                 //for mmap

   // share buffer base and len
   uint32 mShareBufAdd;
   uint32 mShareBufLen;

   // for message queue
   qele pQueue[AUDCCCI_MAX_QUEUE_LEN];
   int32 qLen;
   int32 iQRead;
   int32 iQWrite;

   // for sph coef debug
   int8 iSPHFlag[MAX_SPHCOEF];

#if defined(PC_EMULATION)
   CRITICAL_SECTION fQMutex;
   CRITICAL_SECTION mStreaminMutex;
   HANDLE hReadThread;
#else
   pthread_mutex_t fQMutex;
   pthread_mutex_t mStreaminMutex;
   pthread_t hReadThread;
#endif

#if defined(FAKE_CCCI)
   FakeCCCI *pFakeCCCI;     /* a pointer to a fake ccci */
#endif

   pthread_mutex_t m2WayRecMutex;
   pthread_mutex_t m2WayPlayMutex;
   pthread_mutex_t mA2MShareBufMutex;
   pthread_mutex_t mSetSpeechCoeffMutex;

   pthread_t hSendSphThread;

   char GetModemCurrentStatus(); 
public:

   int8 *pShareR;
   int8 *pShareW;
   bool CCCIEnable;
   bool b_modem_reset;
   int  mFd;
   AudioYusuHardware *mHw;    /* pointer to HW */
   uint32 m_last_write_ccci_msg;
   uint32 m_last_read_ccci_msg;
   int32 m_MDReset_recover_record;


   AudioCCCI(AudioYusuHardware *hw);
   ~AudioCCCI();
   bool Initial();
   bool Deinitial();
   bool WaitUntilModemReady();   
   //uint32 GetBaseAddress();
   //uint32 GetShareBufLen();
   uint32 GetM2AShareBufLen();
   uint32 GetA2MShareBufLen();
   uint32 GetM2AShareBufAddress();
   uint32 GetA2MShareBufAddress();
   int SendMessage(uint32 MSG);
   int ReadMessage(uint32 *msg);
   bool CreateReadingThread();
   int SendMessageInQueue(uint32 MSG);
   int32 GetQueueCount();
   bool CheckOffsetandLength(uint32 MSG);

   bool MDReset_CheckMessageInQueue();
   bool MDReset_FlushMessageInQueue();
   bool ConsumeMessageInQueue();
   bool BGS_On_MSG_ACK_CheckMessageInQueue();
   int32 searchPrevAckQElement(int32 idx);
   int32 JudgeAckOfMsg(int32 MSG);
   void MsgQueueLock();
   void MsgQueueUnLock();
   void RecordLock();
   void RecordUnLock();

   void Record2WayLock();
   void Record2WayUnLock();
   void Play2WayLock();
   void Play2WayUnLock();

   bool A2MBufLock();
   void A2MBufUnLock();

   bool SpeechCoeffLock();
   void SpeechCoeffUnLock();

   void ResetSphCoefFlag();
   void GetMDSideSphCoefArrival(int type);
   bool CheckMDSideSphCoefArrival();
   void SendMsgFailErrorHandling(uint32 MSG);

   int GetTxFd();
   int GetRxFd();

   bool CreateSendSphParaThread();
#if defined(MTK_DUAL_MIC_SUPPORT)||defined(MTK_AUDIO_HD_REC_SUPPORT)
   void DualMic_WriteDataToFile(int32 msg);
   void DualMic_ReadDataFromFile();
   void DualMic_WriteDataToRecBuf(int32 msg);
   void DualMic_ReadDataFromPlayBuf();
#endif

};

}; // namespace android

#endif   //_AUDIO_YUSU_CCCI_H_
