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

#ifndef ANDROID_AUDIO_YUSU_APSTREAM_IN_H
#define ANDROID_AUDIO_YUSU_APSTREAM_IN_H

#include <stdint.h>
#include <sys/types.h>
#include <utils/threads.h>

#include <hardware_legacy/AudioHardwareBase.h>
#include "AudioYusuVolumeController.h"
#include "AudioYusuDef.h"
#include "AudioYusuStreamInInterface.h"

#include "AudioAnalogAfe.h"
#include "AudioAfe.h"
#include "AudioIoctl.h"
extern "C" {
#include "bli_exp.h"
}

//#define DIGITAL_MIC
#define MUTE_APAUDIO_PCM_MS   400
#define MUTE_APAUDIO_PCM_MS_NORMAL   1000


namespace android {
// ----------------------------------------------------------------------------

class AudioYusuHardware;

class AudioYusuApStreamIn : public AudioStreamInInterface {
public:
         AudioYusuApStreamIn() ;
     ~AudioYusuApStreamIn();

    virtual status_t    set(
	AudioYusuHardware *hw,
	int mFd,
	uint32_t devices,
	int *pFormat,
	uint32_t *pChannels,
	uint32_t *pRate,
	android_audio_legacy::AudioSystem::audio_in_acoustics acoustics);

    uint32_t    sampleRate() const ;
#ifdef MTK_AP_SPEECH_ENHANCEMENT	
    uint32     GetRecordSampleRate(uint32_t pRate,AudioYusuHardware *hw = 0);
	void  Parse_Test_STEREO_PCM(void* buffer, int bytes);
	void  Parse_Test_MONO_PCM(short *buffer, int bytes);
#else
    uint32     GetRecordSampleRate(uint32_t pRate);
#endif
    virtual size_t      bufferSize() const { return mReadBufferSize; }
    virtual uint32_t    channels() const;
    virtual int         format() const { return android_audio_legacy::AudioSystem::PCM_16_BIT; }
    virtual status_t    setGain(float gain) { return INVALID_OPERATION; }
    virtual ssize_t     read(void* buffer, ssize_t bytes);
    virtual status_t    dump(int fd, const Vector<String16>& args);
    virtual status_t    standby();
    virtual status_t    setParameters(const String8& keyValuePairs);
    virtual String8     getParameters(const String8& keys);
    virtual unsigned int  getInputFramesLost() const { return 0; }
    virtual void          openRecordDevicePower(void);
    virtual void          CloseRecordDevicePower(void);
    virtual void          getDataFromModem(void);
    virtual void          getVoiceRecordDataFromExtModem(uint8 *pBuf, int s4Size);
    virtual void SetDigitalMic(void);

    virtual bool  IsStandby(void);
    virtual bool  RecOpen(void);
    virtual bool  RecClose(void);
    virtual bool CanRecordFM(void);
    virtual bool MutexLock(void);
    virtual bool MutexUnlock(void);
    virtual void SetStreamInputSource(int Inputsource);

    bool SupportedSampleRate(uint32 SampleRate);
    void SetMicSource(int micsource);

    void StereoToMono(short *buffer , int length);

    uint32 GetSrcbufvalidSize(void);
    uint32 GetSrcbufFreeSize(void);

    uint32 CopySrcBuf(char *buffer,uint32 *bytes, char *SrcBuf , uint32 *length);
    void  Dump_StreamIn_PCM(void* buffer, int bytes);
	
	bool IsStereoMIC(void);
    void CheckNeedMute(void* buffer, int bytes);

    virtual status_t addAudioEffect(effect_handle_t effect){return NO_ERROR;}
    virtual status_t removeAudioEffect(effect_handle_t effect){return NO_ERROR;}

    int StreamInApplyVolume(short *pcm , int count,unsigned short volume);

	void SetAPMicGain(int mictype);
	int SetApMicGain_HDRecord(int mictype);

#ifdef MTK_DUAL_MIC_SUPPORT
	void CheckNeedDataSwitch(short *buffer , int length);
#endif

    AudioYusuHardware *mHw;

private:
    mutable Mutex	mLock;
    mutable Mutex	mRecordLock;
    int      mFd;              /* file ID for write */
    uint32      mSampleRate;      //samping rate running in this bit-stream
    int      mFormat;
    int      mChNum;
    int      mApInputSource;
    uint32   mInputSource;   // this for android audio_source
    uint32   mApSampleRate;
    uint32   mReadBufferSize;
    uint32   mDevice;
    android_audio_legacy::AudioSystem::audio_in_acoustics mAcoustics;
    bool   mstandby;
    int      mMutecount;
    // add for apply fm volume
    unsigned short  mFm_Volume;
    int mFm_Vlume_Index;
    int FM_Mapping_table[16];

    pthread_mutex_t mReadMutex;

    // for SRC
    int8*	 pSrcBuf;
    BLI_HANDLE *pSrcHdl;

    // for src , read buffer
    int8*     pInputbuf;
    uint32   pSrcReadSize;
    uint32   pInputbuflen;
    uint32   pInputSrcWrite;
    uint32   pInputSrcRead;

#ifdef MTK_AP_SPEECH_ENHANCEMENT
	bool	mStartReadTestFile;
	int	mTestFileLength;
	int	mTestFileSizeRead;
	bool vGetHdRecordModeInfo(uint8 *eModeIndex, bool *bStereoRecOn);
	void LoadHdRecordParams(void);
	AUDIO_HD_RECORD_SCENE_TABLE_STRUCT mhdRecordSceneTable;
	AUDIO_HD_RECORD_PARAM_STRUCT mhdRecordParam;
	int      mHDRecordModeIndex;
	bool UpdateSPEParams();
	// base on hardware mode to set recording mode
	int SetHdrecordingMode(int mode, bool *bStereoRecOn);
#endif	
	bool mNeedStereo;
	FILE * mfp;

#ifdef MTK_DUAL_MIC_SUPPORT
	bool	mbLRChannelSwitch;
	int	miSpecificMicChoose;
#endif		
};

// ----------------------------------------------------------------------------
}; // namespace android

#endif
