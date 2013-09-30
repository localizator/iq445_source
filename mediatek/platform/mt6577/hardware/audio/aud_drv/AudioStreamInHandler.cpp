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

#define LOG_TAG "AudioStreamInHandler"
#include <utils/Log.h>
#include <utils/String8.h>

#include "AudioYusuHardware.h"
#include "AudioAfe.h"
#include "AudioAnalogAfe.h"
#include "AudioYusuStreamHandler.h"
#include "AudioYusuVolumeController.h"
#include "AudioStreamInHandler.h"
#include "AudioYusuStreamInInterface.h"
#include "AudioAnalogAfe.h"
#include "audio_custom_exp.h"
#include "AudioYusuStreamHandler.h"
#include <assert.h>

#ifdef ENABLE_HIGH_SAMPLERATE_RECORD
#define AP_RECORD_SAMPLERATE (4000)
#else
#define AP_RECORD_SAMPLERATE (96000)
#endif

//#ifdef ENABLE_LOG_APSTREAMIN
#if 1
#undef ALOGV
#define ALOGV(...) ALOGD(__VA_ARGS__)
#endif

namespace android {

#ifdef DUMP_STREAMIN
int AudioStreamInHandler::record_num = 1;
#endif


bool AudioStreamInHandler::ModemRecordCondition(uint32_t RecordSampleRate,uint32_t devices)
{
    if( mHw->GetVmFlag ()|| mHw->GetVoiceStatus () || mHw->GetCommunicationStatus ()){
        return true;
    }
    else if((devices&android_audio_legacy::AudioSystem::DEVICE_IN_AMBIENT) || (devices&android_audio_legacy::AudioSystem::DEVICE_IN_FM))
    {
        return false;
    }
    else
    {
#ifdef MTK_AP_SPEECH_ENHANCEMENT
#ifndef ALL_USING_AP_ENHANCEMENT
        //only 48K use AP side record
        if(RecordSampleRate == 48000)
            return false;
        else
            return true;
#else
        return false;
#endif
#else
        return true;
#endif  
    }
}

status_t AudioStreamInHandler::set(
        AudioYusuHardware *hw,
	int fd,
	uint32_t devices,
	int *pFormat,
	uint32_t *pChannels,
	uint32_t *pRate,
	android_audio_legacy::AudioSystem::audio_in_acoustics acoustics)
{
    status_t lStatus = NO_ERROR;
    Mutex::Autolock lock(mRecordHandlerLock);

    AudioStreamInInterface *in = NULL;
    ALOGD("+set(%p, %d, %d, %d, %u)", hw, fd, *pFormat, *pChannels, *pRate);

    // here save file descriptor and hardware pointer
    mFd = fd;
    mHw = hw;

    // make ajudge to open Ap or modem side record_num
    if(ModemRecordCondition(*pRate,devices)){
        ALOGD("new AudioYusuStreamIn ");
        in = new AudioYusuStreamIn();
        lStatus = in->set(this->mHw, mFd, devices,pFormat,pChannels, pRate, acoustics);
        mActiveStream = Active_Modem_StreamIn;
    }
    else{
        in = new AudioYusuApStreamIn();
        lStatus = in->set(this->mHw, mFd, devices,pFormat,pChannels, pRate, acoustics);
        mActiveStream = Active_Ap_StreamIn;
    }

    // save set stream parameters
    msetAttribute.mSampleRate =in->sampleRate ();
    msetAttribute.mFormat =in->format ();
    msetAttribute.mChannelCount = in->channels ();
    ALOGD("msetAttribute mSampleRate= %d ,mFormat = %d mChannelCount = %d",
       msetAttribute.mSampleRate,msetAttribute.mFormat ,msetAttribute.mChannelCount );

    if(lStatus == NO_ERROR){
        ALOGD("set lStatus == NO_ERROR");
        // store set attribute
        StoreStreamAttribute(devices,pFormat,pChannels, pRate,(uint32)acoustics);
        ALOGD("msetAttribute mSampleRate= %d ,mFormat = %d mChannelCount = %d",
            msetAttribute.mSampleRate,msetAttribute.mFormat ,msetAttribute.mChannelCount );
        mStreamInput = in;
        mstandby = false;
        #ifdef DUMP_STREAMIN
        openDumpStreamFile();
        #endif
    }
    else{
        delete in;
    }

    return lStatus;

SET_EXIT:

    return BAD_VALUE;
}

uint32_t AudioStreamInHandler::sampleRate() const
{
   if(mStreamInput != NULL)
       return mStreamInput->sampleRate ();
   else if(msetAttribute.mSampleRate)
       return msetAttribute.mSampleRate;
   else
       return 16000;
}

uint32_t AudioStreamInHandler::channels() const
{
    if(mStreamInput != NULL){
        return mStreamInput->channels ();
    }
    else{
        return android_audio_legacy::AudioSystem::CHANNEL_IN_STEREO;;
    }
}

AudioStreamInHandler::AudioStreamInHandler()
{
    ALOGV("AudioStreamInHandler Constructor");
    int ret =0;

    ret = pthread_mutex_init(&mReadMutex, NULL);
    if ( ret != 0 )
        ALOGE("Failed to initialize  mReadMutex");
    mHw = NULL;
    mFd = 0;
    memset((void*)&msetAttribute,0,sizeof(msetAttribute));
    mstandby = true;
    mActiveStream = Active_None_StreamIn;
    mStreamInput = NULL;

    #ifdef DUMP_STREAMIN
    pOutFile = NULL;
    #endif
}

int AudioStreamInHandler::format() const
{
    if(mStreamInput != NULL)
    {
        return mStreamInput->format ();
    }
    else
    {
        return android_audio_legacy::AudioSystem::PCM_16_BIT;
    }
}

AudioStreamInHandler::AudioStreamInHandler(AudioYusuHardware *hw)
{
    ALOGV("AudioStreamInHandler Constructor");
    int ret =0;

    ret = pthread_mutex_init(&mReadMutex, NULL);
    if ( ret != 0 )
        ALOGE("Failed to initialize  mReadMutex");
    mHw = hw;
    mFd = 0;
    memset((void*)&msetAttribute,0,sizeof(msetAttribute));
    mstandby = true;
    mActiveStream = Active_None_StreamIn;
    mStreamInput = NULL;
    mHw->SwitchAudioClock (true);

    #ifdef DUMP_STREAMIN
    pOutFile = NULL;
    #endif
}


status_t  AudioStreamInHandler::standby()
{
    ALOGV("AudioStreamInHandler standby");
    Mutex::Autolock lock(mRecordHandlerLock);
    if(mStreamInput != NULL){
        return mStreamInput->standby();
    }
    return NO_ERROR;
}

AudioStreamInHandler::~AudioStreamInHandler()
{
    #ifdef DUMP_STREAMIN
    if(pOutFile){
        fclose(pOutFile);
        pOutFile = NULL;
    }
    #endif
    if(mStreamInput != NULL){
        ALOGD("mStreamInput delete");
        delete mStreamInput;
        mStreamInput = NULL;
    }
    mHw->SwitchAudioClock (false);
    mActiveStream = Active_None_StreamIn;
    ALOGV("~AudioStreamInHandler");
}

void AudioStreamInHandler::StoreStreamAttribute(
        uint32_t devices,
	int *pFormat,
	uint32_t *pChannels,
	uint32_t *pRate,
	uint32_t  acoustic
	)
{
    ALOGD("StoreStreamAttribute");
    msetAttribute.mChannelCount = *pChannels;
    msetAttribute.mdevices= devices;
    msetAttribute.mFormat = *pFormat;
    msetAttribute.mSampleRate= *pRate;
    msetAttribute.macoustic= acoustic;
}


void AudioStreamInHandler::getDataFromModem()
{
    if(mStreamInput != NULL){
         mStreamInput->getDataFromModem ();
    }
    else{
        usleep(30*1000);
        ALOGW("wait... getDataFromModem");
    }
}

void AudioStreamInHandler::getVoiceRecordDataFromExtModem(uint8 *pBuf, int s4Size)
{
    if(mStreamInput != NULL){
         mStreamInput->getVoiceRecordDataFromExtModem (pBuf, s4Size);
    }
    else{
        usleep(30*1000);
        ALOGW("wait... getVoiceRecordDataFromExtModem");
    }
}

ssize_t AudioStreamInHandler::read(void* buffer, ssize_t bytes)
{
    ssize_t Read_Size = bytes;
    if(mstandby == true){
        ALOGD("read but mstandby == true");
        usleep(50*1000);
        return bytes;
    }
    usleep(2*1000);
    Mutex::Autolock lock(mReadLock);
    // direct read from minputstream, so it will call Ap or modem read stream.
    if(mStreamInput != NULL){
        Read_Size = mStreamInput->read(buffer,bytes);


        #ifdef DUMP_STREAMIN
        int wriiten = fwrite((void*)buffer, 1, bytes,pOutFile);
        #endif
    }
    else{
        ALOGW("read but no streamin");
        usleep(20*1000);
    }
    return Read_Size;
}

size_t  AudioStreamInHandler::bufferSize() const
{
    if(mStreamInput != NULL){
        return mStreamInput->bufferSize ();
    }
    else{
        return 640;
    }
}

status_t AudioStreamInHandler::dump(int fd, const Vector<String16>& args)
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

status_t AudioStreamInHandler::setParameters(const String8& keyValuePairs)
{
    AudioParameter param = AudioParameter(keyValuePairs);
    String8 key = String8(AudioParameter::keyRouting);
    status_t status = NO_ERROR;
    int device;
    ALOGV("setParameters() %s", keyValuePairs.string());

    if(mStreamInput){
        return mStreamInput->setParameters (keyValuePairs);
    }
    return status;
}

String8 AudioStreamInHandler::getParameters(const String8& keys)
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

bool AudioStreamInHandler::IsStandby(void)
{
    if(mStreamInput != NULL){
        return mStreamInput->IsStandby ();
    }
    else{
        // no stream exist means standby
        return true;
    }
}


#ifdef DUMP_STREAMIN
void  AudioStreamInHandler::openDumpStreamFile(void)
{
    if(pOutFile == NULL){
        ALOGD("AudioStreamInHandler::write pinputFile == NULLrecord_num = %d",record_num);
        struct tm *timeinfo;
        time_t rawtime;
        time(&rawtime);
        timeinfo=localtime(&rawtime);
        char path[80];
        memset((void*)path,0,80);
        sprintf (path,"/sdcard/APstreamin_%d.pcm",record_num);
        //strftime (path,80,"/sdcard/%a_%b_%Y__%H_%M_%S_APstream_in.pcm",timeinfo);
        ALOGD( "fopen path is : %s", path);
        pOutFile = fopen(path,"w");
        if(pOutFile ==  NULL){
            ALOGE("create pinputFile error");

        }
        record_num++;
    }
    if(record_num > 5){
        record_num = 0;
    }
}
#endif

void AudioStreamInHandler::SetMicSource(int mictype){
    ALOGD("SetMicSource micsource = %d",mictype);
}

void AudioStreamInHandler::SetStreamInputSource(int inputSource)
{
    ALOGD("SetStreamInputSource micsource = %d",inputSource);
    if(mStreamInput != NULL){
         mStreamInput->SetStreamInputSource (inputSource);
    }
}

bool AudioStreamInHandler::RecOpen(void)
{
    ALOGD("RecOpen");
    int mMode;
    Mutex::Autolock lock(mReadLock);
    if(mStreamInput != NULL){
        mStreamInput->RecOpen ();
    }
    else if(mStreamInput == NULL)
    {
        status_t lStatus = NO_ERROR;
        AudioStreamInInterface *in = NULL;
        // make ajudge to open Ap or modem side record_num
        if(ModemRecordCondition(msetAttribute.mSampleRate,msetAttribute.mdevices)){
            ALOGD("new AudioYusuStreamIn");
            in = new AudioYusuStreamIn();
            lStatus = in->set(this->mHw, mFd, msetAttribute.mdevices,&msetAttribute.mFormat,&msetAttribute.mChannelCount,
                &msetAttribute.mSampleRate, (android_audio_legacy::AudioSystem::audio_in_acoustics )msetAttribute.macoustic);
            mActiveStream = Active_Modem_StreamIn;
        }
        else{
            ALOGD("new AudioYusuApStreamIn");
            in = new AudioYusuApStreamIn();
            lStatus = in->set(this->mHw, mFd, msetAttribute.mdevices,&msetAttribute.mFormat,&msetAttribute.mChannelCount,
                &msetAttribute.mSampleRate, (android_audio_legacy::AudioSystem::audio_in_acoustics )msetAttribute.macoustic);
            mActiveStream = Active_Ap_StreamIn;
        }

        if(lStatus == NO_ERROR){
            ALOGD("set lStatus == NO_ERROR");
            mStreamInput = in;
        }
        else{
            delete in;
            ALOGW("RecOpen but cannot open stream");
        }
    }
    mstandby = false;
    return true;
}

bool AudioStreamInHandler::RecClose(void)
{
    ALOGD("RecClose");
    Mutex::Autolock lock(mReadLock);
    mstandby = true;
    if(mStreamInput != NULL){
        mStreamInput->standby ();
        delete mStreamInput;
        mStreamInput = NULL;
        return true;
    }
    else{
        return false;
    }
}

bool AudioStreamInHandler::MutexLock(void)
{
    ALOGD("AudioStreamInHandler MutexLock");
    return mRecordHandlerLock.lock ();
}

bool AudioStreamInHandler::MutexUnlock(void)
{
    ALOGD("AudioStreamInHandler MutexUnlock");
    mRecordHandlerLock.unlock ();
    return true;
}

// ----------------------------------------------------------------------------
}

