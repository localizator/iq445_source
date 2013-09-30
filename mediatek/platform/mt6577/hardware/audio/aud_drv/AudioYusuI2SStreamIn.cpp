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

#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sched.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <time.h>
#include <sys/time.h>

#define LOG_TAG "AudioYusuI2SStreamIn"
#include <utils/Log.h>
#include <utils/String8.h>

#include "AudioYusuHardware.h"
#include "AudioAfe.h"
#include "AudioAnalogAfe.h"
#include "AudioYusuStreamHandler.h"
#include "AudioYusuVolumeController.h"
#include "AudioYusuI2SStreamIn.h"
#include "audio_custom_exp.h"

#include <assert.h>

#ifdef ENABLE_LOG_I2SSTREAMIN
#undef ALOGV
#define ALOGV(...) ALOGD(__VA_ARGS__)
#endif

namespace android {

status_t AudioYusuI2SStreamIn::set(
        AudioYusuHardware *hw,
	int fd,
	uint32_t devices,
	int *pFormat,
	uint32_t *pChannels,
	uint32_t *pRate,
	android_audio_legacy::AudioSystem::audio_in_acoustics acoustics)
{
   bool ret = true;
   ALOGD("AudioYusuI2SStreamIn::set(%p, %d, %d, %d, %u)", hw, fd, *pFormat, *pChannels, *pRate);
   // check values


   if ((*pFormat != android_audio_legacy::AudioSystem::PCM_16_BIT) || (*pChannels != channels()) || (*pRate != sampleRate()) ) {
      ALOGE("Error opening input Rate(%d), ch(%d)",sampleRate(),channels());
      goto SET_EXIT;
   }


    mSourceType = mI2SHandler->GetSourceType();

    mI2Sid = mI2SHandler->open();
    ALOGV("AudioI2S mI2Sid = %d",mI2Sid);
    mI2SHandler->start(mI2Sid,(I2STYPE)mSourceType);
    ALOGV("AudioYusuI2SStreamIn for I2S if work and return ok");
    return NO_ERROR;

SET_EXIT:
    // modify default paramters and let Audiorecord openagina for reampler.
   *pFormat != android_audio_legacy::AudioSystem::PCM_16_BIT;
   *pChannels =  channels();
   *pRate = sampleRate ();
    return BAD_VALUE;

}

AudioYusuI2SStreamIn::AudioYusuI2SStreamIn()
{
    ALOGV("AudioYusuI2SStreamIn Constructor");
    int ret =0;
    mI2Sid = NULL;
    // init with default pcm parameters
    mI2SHandler = AudioI2S::getInstance();


    mSampleRate = mI2SHandler->samplerate();
    mChannels = android_audio_legacy::AudioSystem::CHANNEL_IN_STEREO;
    ALOGD("Constructor mSampleRate=%d, mChannels=%d",mSampleRate,mChannels);


    mReadBufferSize = mI2SHandler->GetReadBufferSize();
    ret = pthread_mutex_init(&mReadMutex, NULL);
    if ( ret != 0 )
        ALOGE("Failed to initialize  mReadMutex");
    mHw = NULL;
    mFd = -1;
    mSourceType=-1;
}

 status_t  AudioYusuI2SStreamIn::standby()
{
    ALOGV("standby");
    return NO_ERROR;
}

AudioYusuI2SStreamIn::~AudioYusuI2SStreamIn()
{
    ALOGV("~AudioYusuI2SStreamIn");
    if(mI2SHandler != NULL && mI2Sid != 0){
        /*
        if(mI2SHandler->GetState () == I2S0_INPUT_START){
            mI2SHandler->stop(mI2Sid,(I2STYPE)mSourceType);
        }
        */
        mI2SHandler->stop(mI2Sid,(I2STYPE)mSourceType);

        mI2SHandler->close(mI2Sid);
        mI2SHandler = NULL;
    }
}

ssize_t AudioYusuI2SStreamIn::read(void* buffer, ssize_t bytes)
{
   int readsize = 0;
   ALOGD("before read bytes = %d readsize = %d",bytes,readsize);
   readsize = mI2SHandler->read(mI2Sid, buffer,mReadBufferSize);
   ALOGD("after read bytes = %d readsize = %d",bytes,readsize);
   return readsize;
}

status_t AudioYusuI2SStreamIn::dump(int fd, const Vector<String16>& args)
{
    const size_t SIZE = 256;
    char buffer[SIZE];
    String8 result;
    snprintf(buffer, SIZE, "AudioYusuStreamIn::dump\n");
    result.append(buffer);
    snprintf(buffer, SIZE, "\tsample rate: %d\n", sampleRate());
    result.append(buffer);
    snprintf(buffer, SIZE, "\tbuffer size: %d\n", bufferSize());
    result.append(buffer);
    snprintf(buffer, SIZE, "\tchannel count: %d\n", channels());
    result.append(buffer);
    snprintf(buffer, SIZE, "\tformat: %d\n", format());
    result.append(buffer);
    snprintf(buffer, SIZE, "\tmAudioHardware: %p\n", mHw);
    result.append(buffer);
    snprintf(buffer, SIZE, "\tmFd: %d\n", mFd);
    result.append(buffer);
    ::write(fd, result.string(), result.size());
    return NO_ERROR;
}

status_t AudioYusuI2SStreamIn::setParameters(const String8& keyValuePairs)
{
    AudioParameter param = AudioParameter(keyValuePairs);
    String8 key = String8(AudioParameter::keyRouting);
    status_t status = NO_ERROR;
    int device;
    ALOGV("setParameters() %s", keyValuePairs.string());

    if (param.getInt(key, device) == NO_ERROR) {
        mDevice = device;
        param.remove(key);
    }

    if (param.size()) {
        status = BAD_VALUE;
    }
    return status;
}

String8 AudioYusuI2SStreamIn::getParameters(const String8& keys)
{
    AudioParameter param = AudioParameter(keys);
    String8 value;
    String8 key = String8(AudioParameter::keyRouting);

    if (param.get(key, value) == NO_ERROR) {
        param.addInt(key, (int)mDevice);
    }

    ALOGV("getParameters() %s", param.toString().string());
    return param.toString();
}


// ----------------------------------------------------------------------------
}

