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

#ifndef ANDROID_AUDIO_YUSU_USER_CASE_H
#define ANDROID_AUDIO_YUSU_USER_CASE_H

#include <utils/threads.h>
#include <AudioUcmInterface.h>
#include <utils/KeyedVector.h>

namespace android_audio_legacy {
// ----------------------------------------------------------------------------
using android::Mutex;
using android::KeyedVector;

class AudioYusuUserCaseManager : public AudioUserCaseManagerInterface
{
public:
	 AudioYusuUserCaseManager();
	 ~AudioYusuUserCaseManager();
         virtual status_t    initCheck();

         // tell UCM which mode is, when system boot up , default for normal mode/
         virtual status_t  setPhoneMode(int32_t mode);

         // tell UCM which device is attached and remove.
         virtual status_t  setInputDevice(audio_devices_t device);
         virtual status_t  setOutputDevice(audio_devices_t device);

         // init stream index and volume
         virtual status_t initStreamVol(AudioSystem::stream_type stream, int indexMin , int indexMax);

         // tell UCM which stream start or stop.
         virtual status_t streamStart(audio_io_handle_t output,AudioSystem::stream_type stream);
         virtual status_t streamStop(audio_io_handle_t output,AudioSystem::stream_type stream);


         // tell UCM when Record start / stop
         virtual status_t recordStart();
         virtual status_t recordStop();

         virtual status_t setStreamVolIndex(AudioSystem::stream_type stream, uint32_t level,audio_devices_t device);

         // set and get parameter
         virtual status_t setParameters(const String8& keyValuePairs);
         virtual String8 getParameters(const String8& keys);

		 // compute volume
		 virtual float volIndexToDigitalVol(audio_devices_t device,AudioSystem::stream_type stream,int index);
		 virtual int volIndexToAnalogVol(audio_devices_t device,int index=0);

private:
	     static audio_devices_t getDeviceForVolume(audio_devices_t device);
		 static float linearToLog(int volume);
	     int getStreamMaxLevel(AudioSystem::stream_type stream);
		 int getGainOutputDevice(audio_devices_t device);
		 int getAmpValidMask(int phoneState, audio_devices_t device);
		 int selectStreamForAnalog();
		 bool streamActive(int stream);
         // for debug , dump all information
         void dumpStreamGainTable();
         void dumpMiscGainTable(int gainMisc);
		 void dumpMicroPhoneGainTable();
         void dumpSidetoneGainTable();
         void dumpspeechGainTable();
protected:
	// stream descriptor used for volume control
	class UCMStreamDescriptor
	{
	public:
		UCMStreamDescriptor();
		int getVolumeIndex(audio_devices_t device);
		void dump(char* buffer, size_t size);
		int mIndexMin;		// min volume index
		int mIndexMax;	// max volume index
		KeyedVector<audio_devices_t, int> mIndexCur;   // current volume index per device
		bool mCanBeMuted;	// true is the stream can be muted
		int mActiveCounter; // if stream start , counter will increase  1 ,when stop counter will decrease on.
	};
    UCMStreamDescriptor mStreams[AudioSystem::NUM_STREAM_TYPES];// stream descriptors for volume control
    int mPhoneState;
	audio_devices_t mOutputDevice;
	audio_devices_t mInputDevice;
	Mutex   mLock;
	AUDIO_GAIN_TABLE_STRUCT *pGainTable;
};

// ----------------------------------------------------------------------------
}; // namespace android

#endif
