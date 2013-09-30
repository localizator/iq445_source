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

/* for Playback through LAD */
#ifndef _AUDIO_YUSU_LADPLAY_H_
#define _AUDIO_YUSU_LADPLAY_H_

#if defined(PC_EMULATION)
#include "windows.h"
#else
#include "pthread.h"
#endif

#include "AudioYusuDef.h"
#include "AudioYusuLad.h"
#include "AudioYusuHardware.h"
extern "C" {
#include "bli_exp.h"
}

namespace android {

#define MAX_LAD_PLAY_BUFFER_NUM  2

#define LAD_PLAY_BUFFER_LEN      2048


#define LAD_PLAY_DATA_OFFSET     0        //share buffer offset
#define LAD_SAMPLE_RATE          8000
#define LAD_CHANNEL_NUM          1

#define MINUS_90_DB              -180     // it is in Q31.1 format

// for debug
//#define DUMP_BGS_DATA
//#define DUMP_BGS_BLI_BUF
//#define BGS_USE_SINE_WAVE
/*=============================================================================
 *                              Class definition
 *===========================================================================*/

class AudioYusuHardware;
class LADPlayer;
class LAD;

class LADPlayBuffer
{
private:

    LADPlayer* pLadPlayer;
    int32 mIdx;

public:

   // ring buffer parameters
   rb mBuf;

   int8* pSrcBuf;
   BLI_HANDLE *pSrcHdl;
   int32 mFormat;

#if defined(PC_EMULATION)
   CRITICAL_SECTION bMutex;
#else
   pthread_mutex_t bMutex;
#endif

   int32 LADPlayWrite(int8* buf, int32 num);
   int32 LADPlayWriteVM(int8* buf, int32 num);
   bool LADPlayBufferInit(LADPlayer *playPointer, uint32 idx, uint32 sampleRate, uint32 chNum, int32 mFormat, uint32 targetSR);
   LADPlayBuffer();
   ~LADPlayBuffer();
   void BuffLock();
   void BuffUnLock();

#ifdef DUMP_BGS_BLI_BUF
   FILE *p_OutFile1;
   FILE *p_OutFile2;
#endif
};


class LADPlayer
{
private:

   AudioYusuHardware *mHw;    /* pointer to HW */
   LAD *pLad;
   int32 mFormat;
   int32 mAudioMode;

   //backup Background Sound UL and DL gain
   //bcs we set them to zero when normal recording
   //we need to set it back when phone call recording
   uint32 mBGSUplinkGain;
   uint32 mBGSDownlinkGain;

public:

   int32 runFlag;
   int8* pShareBuf;
   pthread_mutex_t  mLadPlayerWriteMutex; // use for write and reset LadPlayerBuffer
   LADPlayBuffer* mBufTable[MAX_LAD_PLAY_BUFFER_NUM];

   ~LADPlayer();
   LADPlayer(AudioYusuHardware *hw);
   bool LADPlayer_Open(int32 audioMode);
   bool LADPlayer_Close();
   int32 LADPlayer_WaitPlayRequestEvent();
   void memmix16(int8* tBuf, int8* sBuf, int32 size, bool mix);
   void RingBufMixToLinear(int8* targetBuf, LADPlayBuffer* sourceBuf, int32 size, bool mix);
   int32 LADPlayer_DoMix(int32 requiredDataCount);
   int32 LADPlayer_MixAndPlay(uint32 numDataRequired);
   bool LADPlayer_PlayerIsNotEmpty();
   void LADPlayer_SetBGSoundGain(int32 uplinkGain, int32 downlinkGain);
   bool LADPlayer_CheckBufferEmpty(void);

   LADPlayBuffer* LADPlayer_CreateBuffer(uint32 sampleRate, uint32 chNum, int32 format, uint32 targetSR);

#ifdef DUMP_BGS_DATA
   FILE *pOutFile;
#endif
};

}; // namespace android

#endif //_AUDIO_YUSU_LADPLAY_H_
