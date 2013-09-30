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
 * Copyright (C) 2007 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef ANDROID_AUDIO_YUSU_STREAM_IN_H
#define ANDROID_AUDIO_YUSU_STREAM_IN_H

#include <stdint.h>
#include <sys/types.h>
#include <utils/threads.h>

#include <AudioYusuStreamHandler.h>
#include <hardware_legacy/AudioHardwareBase.h>
#include "AudioYusuVolumeController.h"
#include "AudioYusuDef.h"


#include "AudioAnalogAfe.h"
#include "AudioAfe.h"
#include "AudioIoctl.h"
#include "AudioYusuLad.h"

extern "C" {
#include "bli_exp.h"
}

#define AUDIO_IN_BUFFER_LEN   2048 * 32
#define INPUT_SOURCE_FM (5)

#define MD_RECORD_WAKELOCK_NAME    "MD_RECORD_WAKELOCK"


namespace android {

class AudioYusuHardware;
class AudioYusuStreamHandler;
class LAD;
struct rb;

class AudioYusuStreamIn : public AudioStreamInInterface
{
public:

   AudioYusuStreamIn() ;
    ~AudioYusuStreamIn();

   virtual status_t    set(AudioYusuHardware *hw,
                        	int mFd,
                        	uint32_t devices,
                        	int *pFormat,
                        	uint32_t *pChannels,
                        	uint32_t *pRate,
                        	android_audio_legacy::AudioSystem::audio_in_acoustics acoustics);

   uint32_t            sampleRate() const;
   virtual size_t      bufferSize() const ;
//   virtual uint32_t    channels() const {return android_audio_legacy::AudioSystem::CHANNEL_IN_MONO;}
   virtual uint32_t    channels() const;

   virtual int         format() const;
   virtual status_t    setGain(float gain) { return INVALID_OPERATION; }
   virtual ssize_t     read(void* buffer, ssize_t bytes);
   virtual status_t    dump(int fd, const Vector<String16>& args);
   virtual status_t    standby();
   virtual status_t    setParameters(const String8& keyValuePairs);
   virtual String8     getParameters(const String8& keys);
   virtual unsigned int  getInputFramesLost() const { return 0; }
   virtual status_t addAudioEffect(effect_handle_t effect){return NO_ERROR;}
   virtual status_t removeAudioEffect(effect_handle_t effect){return NO_ERROR;}
   // base on hardware mode to set recording mode
   void SetHdrecordingMode(int mode, bool *bStereoRecOn);
   AudioYusuHardware   *mHw;

   void  getSharePointer();
   int   myWaitEvent();

   virtual void getDataFromModem();
   virtual void getVoiceRecordDataFromExtModem(uint8 *pBuf, int s4Size);
   void  InBuffLock();
   void  InBuffUnLock();
   void ControlLock();
   void ControlUnLock();
   bool  RecOpen();
   bool  RecClose();
   bool CanRecordFM(int mode);
   bool IsStandby() {return mstandby;}
   virtual bool  MutexLock();
   virtual bool  MutexUnlock();
   virtual void  SetStreamInputSource(int Inputsource);

#ifdef MTK_DUAL_MIC_SUPPORT
   void CheckNeedDataSwitch(short *buffer , int length);
#endif   

private:
   LAD      *pLad;
   pthread_mutex_t mInputMutex;
   int      mFd;              /* file ID for write */
   int      mSampleRate;      //samping rate running in this bit-stream
   int      mSampleRateModem; //samping rate running in modem, if different from mSampleRate, SRC is required
   int      mFormat;
   int      mChNum;
   android_audio_legacy::AudioSystem::audio_in_acoustics mAcoustics;
   uint32_t mDevice;
   uint32_t mInputSource;
   bool     mSetSphEnh;
   int      data_nofity_count;
   bool   mstandby;
   int    m_mute_count;
   int    mBufDuration;

   // for buffer transfer
   char     *pCIBufBase;           /* buffer base to write */
   rb       inBuf;                 /* internal ring buffer for audio in */
   rb       shareBuf;              /* share ring buffer */

   // for SRC
   int8*    pSrcBuf;
   BLI_HANDLE *pSrcHdl;

    // lock for internal buffer access
#if defined(PC_EMULATION)
   CRITICAL_SECTION ibMutex;
#else
   pthread_mutex_t ibMutex;
   pthread_mutex_t mControlMutex;

#endif
   FILE *pOutFile;
   int  m_no_data_count;
   int  m_DropFrameCount;

#if defined(MTK_AUDIO_HD_REC_SUPPORT)
   bool vGetHdRecordModeInfo(uint8 *eModeIndex, bool *bStereoRecOn);
   bool bUpdateHdRecordMappingTable(
      AUDIO_HD_RECORD_SCENE_TABLE_STRUCT *hdRecordSceneTable,
      AUDIO_HD_RECORD_PARAM_STRUCT *hdRecordParam );
   bool vGetHdSpecifictModeInfo(uint8 *modeIndex, bool *bStereoRecOn, int AssignedModeIndex);
   FILE *pVMFile;
#endif

#ifdef MTK_DUAL_MIC_SUPPORT
	bool	mbLRChannelSwitch;
	int	miSpecificMicChoose;

#endif

};


}; // namespace android

#endif
