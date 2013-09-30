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

#define LOG_TAG "AudioYusuUCM"
#define LOG_NDEBUG 0

#include <utils/Log.h>
#include <CFG_AUDIO_File.h>

#include "AudioUcm.h"
#include "audio_custom_exp.h"
#include "AudioCustParam.h"
#include <math.h>


namespace android_audio_legacy {

static const int MIN_HW_ANALOG_GAIN = 4;
static const int MAX_HW_ANALOG_GAIN = 44;
static const int MAX_DIGITAL_STEP = 255;

// change this value to change volume scaling
static const float dBPerStep = 0.25f;

// shouldn't need to touch these
static const float dBConvert = -dBPerStep * 2.302585093f / 20.0f;
static const float dBConvertInverse = 1.0f / dBConvert;

AudioYusuUserCaseManager::AudioYusuUserCaseManager()
    :mPhoneState(AudioSystem::MODE_NORMAL)
{
    ALOGV("AudioYusuUserCaseManager constructor");
    //set default device
    mOutputDevice =AUDIO_DEVICE_OUT_SPEAKER;
    mInputDevice =AUDIO_DEVICE_IN_BUILTIN_MIC;
    pGainTable = NULL;
    pGainTable = (AUDIO_GAIN_TABLE_STRUCT *)malloc(sizeof(AUDIO_GAIN_TABLE_STRUCT));

    if(pGainTable){
        android::GetAudioGainTableParamFromNV(pGainTable); //read data from NVRAM
    }

    dumpStreamGainTable();
    dumpMiscGainTable(AUDIO_GAIN_MIC);
    dumpMiscGainTable(AUDIO_GAIN_SIDETONE);
    dumpMiscGainTable(AUDIO_GAIN_SPEECH);

}

AudioYusuUserCaseManager::~AudioYusuUserCaseManager()
{
    if(pGainTable){
        free(pGainTable);
        pGainTable = NULL;
    }
}

status_t AudioYusuUserCaseManager::initCheck()
{
    ALOGV("init cehck");
    return NO_ERROR;
}

bool AudioYusuUserCaseManager::streamActive(int stream)
{
    if(stream > AUDIO_GAIN_MAX_STREAM)
    {
        return false;
    }
    if(mStreams[stream].mActiveCounter){
        return true;
    }
    else{
        return false;
    }

}

/*********************************************************************
     This function defines which stream gain is referenced to set analog and amp  gain.
      Incall mode , use speech gain. In other mode ,use the active stream gain.
      THe priority can be adjusted.
***********************************************************************/
int AudioYusuUserCaseManager::selectStreamForAnalog()
{
	if(streamActive(AUDIO_GAIN_VOICE_CALL))
		return AudioSystem::VOICE_CALL;
	else if(streamActive(AUDIO_GAIN_BLUETOOTH_SCO))
		return AudioSystem::BLUETOOTH_SCO;
    else if(streamActive(AUDIO_GAIN_RING))
        return AudioSystem::RING;
	else if(streamActive(AUDIO_GAIN_ALARM))
        return AudioSystem::ALARM;
    else if(streamActive(AUDIO_GAIN_NOTIFICATION))
        return AudioSystem::NOTIFICATION;
    else if(streamActive(AUDIO_GAIN_ENFORCED_AUDIBLE))
        return AudioSystem::ENFORCED_AUDIBLE;
    else if(streamActive(AUDIO_GAIN_MUSIC))
        return AudioSystem::MUSIC;
    else if(streamActive(AUDIO_GAIN_TTS))
        return AudioSystem::TTS;
    else if(streamActive(AUDIO_GAIN_SYSTEM))
    	return AudioSystem::SYSTEM;
	else if(streamActive(AUDIO_GAIN_FM))
        return AudioSystem::FM;
	else if(streamActive(AUDIO_GAIN_DTMF))
        return AudioSystem::DTMF;
    else
        return AudioSystem::DEFAULT;
}


status_t  AudioYusuUserCaseManager::setPhoneMode(int32_t mode)
{
	Mutex::Autolock _l(mLock);
    ALOGV("SetPhoneMode mode = %d",mode);
    mPhoneState = mode;
    return NO_ERROR;
}

status_t AudioYusuUserCaseManager::setInputDevice(audio_devices_t device)
{
    ALOGV("SetInputDevice device = %x",device);
	Mutex::Autolock _l(mLock);
    mInputDevice = device;
    return NO_ERROR;
}
status_t AudioYusuUserCaseManager::setOutputDevice(audio_devices_t device)
{
    ALOGV("SetOutputDevice device = %x",device);
    // do set analog gain id device is cahnge
    Mutex::Autolock _l(mLock);
    mOutputDevice = device;
    return NO_ERROR;
}

status_t AudioYusuUserCaseManager::initStreamVol(android_audio_legacy::AudioSystem::stream_type stream, int indexMin , int indexMax)
{
    ALOGV("InitStreamLevel stream = %d indexMin = %d indexMax = %d",stream,indexMin,indexMax);
	Mutex::Autolock _l(mLock);
    mStreams[stream].mIndexMin = indexMin;
    mStreams[stream].mIndexMax = indexMax;
    return NO_ERROR;
}

status_t AudioYusuUserCaseManager::streamStart(audio_io_handle_t output,AudioSystem::stream_type stream)
{
    ALOGV("StreamStart stream = %d",stream);
	Mutex::Autolock _l(mLock);
    mStreams[stream].mActiveCounter++;
    return NO_ERROR;
}

status_t AudioYusuUserCaseManager::streamStop(audio_io_handle_t output,android_audio_legacy::AudioSystem::stream_type stream)
{
    ALOGV("StreamStop stream = %d",stream);
	Mutex::Autolock _l(mLock);
    if(mStreams[stream].mActiveCounter >0)
    {
        mStreams[stream].mActiveCounter--;
    }
    return NO_ERROR;
}

status_t AudioYusuUserCaseManager::recordStart()
{
    ALOGV("recordStart");
	Mutex::Autolock _l(mLock);
    return NO_ERROR;
}

status_t AudioYusuUserCaseManager::recordStop()
{
    ALOGV("recordStop");
	Mutex::Autolock _l(mLock);
    return NO_ERROR;
}

// tell UCM which steam start or stop.
status_t AudioYusuUserCaseManager::setStreamVolIndex(AudioSystem::stream_type stream, uint32_t index,audio_devices_t device)
{
    ALOGV("setStreamLevel stream = %d index = %d device %d",stream,index,device);
	Mutex::Autolock _l(mLock);
	// if device is AUDIO_DEVICE_OUT_DEFAULT set default value and
    // clear all device specific values
    if (device == AUDIO_DEVICE_OUT_DEFAULT) {
        mStreams[stream].mIndexCur.clear();
    }
    mStreams[stream].mIndexCur.add(device, index);
    return NO_ERROR;
}

status_t AudioYusuUserCaseManager::setParameters(const String8& keyValuePairs)
{
    status_t status = NO_ERROR;
    return status;
}

String8 AudioYusuUserCaseManager::getParameters(const String8& keys)
{
    AudioParameter param = AudioParameter(keys);
    String8 value;
    String8 key = String8("true");

    if (param.get(key, value) == NO_ERROR) {
        param.addInt(key, (int)NULL);
    }

    ALOGV("getParameters() %s", param.toString().string());
    return param.toString();
}

int AudioYusuUserCaseManager::getStreamMaxLevel(AudioSystem::stream_type stream)
{
    static int stream_level[AUDIO_GAIN_MAX_STREAM+1]={ VOICE_GAIN_TABLE_LEVEL,
                                                       SYSTEM_GAIN_TABLE_LEVEL,
                                                       RING_GAIN_TABLE_LEVEL,
                                                       MUSIC_GAIN_TABLE_LEVEL,
                                                       ALARM_GAIN_TABLE_LEVEL,
                                                       NOTIFICATION_GAIN_TABLE_LEVEL,
                                                       BLUETOOTHSCO_GAIN_TABLE_LEVEL,
                                                       ENFORCEAUDIBLE_GAIN_TABLE_LEVEL,
                                                       DTMF_GAIN_TABLE_LEVEL,
                                                       TTS_GAIN_TABLE_LEVEL,
                                                       FM_GAIN_TABLE_LEVEL};
    if(stream > AUDIO_GAIN_MAX_STREAM)
        return -1;
    return stream_level[stream];
}


float AudioYusuUserCaseManager::linearToLog(int volume)
{
    return volume ? exp(float(255 - volume) * dBConvert) : 0;
}

audio_devices_t AudioYusuUserCaseManager::getDeviceForVolume(audio_devices_t device)
{
    if (device == 0) {
        // this happens when forcing a route update and no track is active on an output.
        // In this case the returned category is not important.
        device =  AUDIO_DEVICE_OUT_SPEAKER;
    } else if (AudioSystem::popCount(device) > 1) {
        // Multiple device selection is either:
        //  - speaker + one other device: give priority to speaker in this case.
        //  - one A2DP device + another device: happens with duplicated output. In this case
        // retain the device on the A2DP output as the other must not correspond to an active
        // selection if not the speaker.
        if (device & AUDIO_DEVICE_OUT_SPEAKER) {
            device = AUDIO_DEVICE_OUT_SPEAKER;
        } else {
            device = (audio_devices_t)(device & AUDIO_DEVICE_OUT_ALL_A2DP);
        }
    }
    ALOGW_IF(AudioSystem::popCount(device) != 1,
            "getDeviceForVolume() invalid device combination: %08x",
            device);

    return device;
}


int AudioYusuUserCaseManager::getGainOutputDevice(audio_devices_t device)
{
    ALOGV("getGainOutputDevice device 0x%x",device);
    switch(getDeviceForVolume(device)) {
        case AUDIO_DEVICE_OUT_EARPIECE:
            return GAIN_OUTPUT_RECEIVER;
        case AUDIO_DEVICE_OUT_WIRED_HEADSET:
        case AUDIO_DEVICE_OUT_WIRED_HEADPHONE:
        case AUDIO_DEVICE_OUT_BLUETOOTH_SCO:
        case AUDIO_DEVICE_OUT_BLUETOOTH_SCO_HEADSET:
        case AUDIO_DEVICE_OUT_BLUETOOTH_A2DP:
        case AUDIO_DEVICE_OUT_BLUETOOTH_A2DP_HEADPHONES:
            return GAIN_OUTPUT_HEADSET;
        case AUDIO_DEVICE_OUT_SPEAKER:
        case AUDIO_DEVICE_OUT_BLUETOOTH_SCO_CARKIT:
        case AUDIO_DEVICE_OUT_BLUETOOTH_A2DP_SPEAKER:
        case AUDIO_DEVICE_OUT_AUX_DIGITAL:
        case AUDIO_DEVICE_OUT_USB_ACCESSORY:
        case AUDIO_DEVICE_OUT_USB_DEVICE:
        default:
            return GAIN_OUTPUT_SPEAKER;
    }
}

float AudioYusuUserCaseManager::volIndexToDigitalVol(audio_devices_t device,AudioSystem::stream_type stream,int index)
{
    ALOGV("volIndexToDigitalVol device =0x%x stream =%d, index =%d", device,stream,index);
	Mutex::Autolock _l(mLock);

    if(mPhoneState == AudioSystem::MODE_IN_CALL){
        return 1.0; // do not adjust  volume, fixed digital vol when in call ????
    }
    int step = MAX_DIGITAL_STEP;
    if(stream <= AUDIO_GAIN_MAX_STREAM)
    {
        UCMStreamDescriptor &streamDesc = mStreams[stream];
        int level = getStreamMaxLevel(stream)-1;
        int gainOutputDevice = getGainOutputDevice(device);
        if(gainOutputDevice == GAIN_OUTPUT_RECEIVER && stream != AudioSystem::VOICE_CALL)
        {
            // sreams use headset param if  it is not VOICE CALL;
            gainOutputDevice = GAIN_OUTPUT_HEADSET;
        }
        STREAM_GAIN_CONTROL_STRUCT *pStream = &pGainTable->voiceCall + stream;
        
        float indexPercent = index * level/ (streamDesc.mIndexMax - streamDesc.mIndexMin); 
        int indexInt = (int)indexPercent;
        float indexFrac = indexPercent - indexInt;
        if(indexInt >= level )
        {
            step =  (MAX_DIGITAL_STEP - pStream->streamGain[gainOutputDevice][level].u8Gain_digital);
            ALOGV("streamGain[%d][%d].gigital %u, step %d",gainOutputDevice,level,
                pStream->streamGain[gainOutputDevice][level].u8Gain_digital,step);
        }
        else
        {
            step = (MAX_DIGITAL_STEP-pStream->streamGain[gainOutputDevice][indexInt].u8Gain_digital) +  
                    (pStream->streamGain[gainOutputDevice][indexInt].u8Gain_digital - pStream->streamGain[gainOutputDevice][indexInt+1].u8Gain_digital) * indexFrac ;
            ALOGV("streamGain[%d][%d].gigital 0x%x, streamGain[%d][%d].gigital 0x%x,indexFrac %f step %d",
                gainOutputDevice,indexInt,pStream->streamGain[gainOutputDevice][indexInt].u8Gain_digital,
                gainOutputDevice,indexInt+1,pStream->streamGain[gainOutputDevice][indexInt+1].u8Gain_digital,indexFrac,step);

        }
    }
    return linearToLog(step);
}

int AudioYusuUserCaseManager::getAmpValidMask(int phoneState, audio_devices_t device)
{
    ALOGV("getAmpValidMask phoneState %d, device 0x%x",phoneState,device );
    //use 5bits for valid amp gain
    //INPUT1, INPUT2,  SPEAKER, HP
    //       x ,       x,               x,    x,       (4bits)
    int mask =0; //00000 (not use)
    if(device & AudioSystem::DEVICE_OUT_SPEAKER || device == 0 )
    {
        if(phoneState == AudioSystem::MODE_IN_CALL)
        {
            mask = 0x6; //0110
        }
        else
        {
            mask = 0xA;//1010
        }
    }
    if(device & AudioSystem::DEVICE_OUT_WIRED_HEADSET || device & AudioSystem::DEVICE_OUT_WIRED_HEADPHONE)
    {
        if(phoneState == AudioSystem::MODE_IN_CALL)
        {
            mask = mask | 0x5; //0101
        }
        else
        {
            mask = mask | 0x9;//1001
        }

    }
    if(device & AudioSystem::DEVICE_OUT_EARPIECE)
    {
        mask = 0;  //00000  not use
    }
    return mask;
}

int AudioYusuUserCaseManager::volIndexToAnalogVol(audio_devices_t device,int index)
{
    ALOGV("volIndexToAnalogVol device 0x%x",device);
	Mutex::Autolock _l(mLock);
    uint32_t vol = 0;
    if(mPhoneState == AudioSystem::MODE_IN_CALL){
        //use speech param and voice call level
        UCMStreamDescriptor &streamDesc = mStreams[AudioSystem::VOICE_CALL];
        STREAM_SPEECH_GAIN_CONTROL_STRUCT *pSpeech = &pGainTable->speechGain;
        if( AudioSystem::popCount(device) >= 2         && 
            device & AudioSystem::DEVICE_OUT_SPEAKER   &&
            device &(AudioSystem::DEVICE_OUT_WIRED_HEADPHONE | AudioSystem::DEVICE_OUT_WIRED_HEADSET))
        {
            int voiceIndex = streamDesc.getVolumeIndex(getDeviceForVolume(device));
            //speaker and headphon are all open.
            //     Analog, INPUT1,  INPUT2 , SPEAKER ,   HPL  ,   HPR,    resolved
            //xxxxxxxx,     xx,         xx,      xxxxx,       xxxxx,  xxxxx,  xxxxx   (32 bits)
           uint32_t pgaSpeaker = pSpeech->speechGain[GAIN_OUTPUT_SPEAKER][voiceIndex].u32Gain_PGA_Amp;
           uint32_t pgaHP = pSpeech->speechGain[GAIN_OUTPUT_HEADSET][voiceIndex].u32Gain_PGA_Amp;
           vol = ((pgaSpeaker >> 15)<<15) | (pgaHP & 0x7FFF); // combile hp gain 
           //  Analog, INPUT1,  INPUT2 , SPEAKER ,   HP  ,   Digital,      mask
           //xxxxxx,     xx,         xx,      xxxxx,       xxxxx,  xxxxxxxx, xxxx   (32 bits)
           uint8_t digital = pSpeech->speechGain[GAIN_OUTPUT_SPEAKER][voiceIndex].u8Gain_digital;
           int mask = getAmpValidMask(mPhoneState, device);
           vol =( (vol >> 10) << 12) | (digital<<4) | mask; //allocate space for digital and mask 
           ALOGV("volIndexToAnalogVol Incall level %d PGASpeaker 0x%x PAGHp 0x%x, dig=0x%x, mask =0x%x,vol =0x%x",
               voiceIndex,pgaSpeaker,pgaHP,digital,mask,vol);
        }
        else
        {
            int voiceIndex = streamDesc.getVolumeIndex(getDeviceForVolume(device));
            int gainOutputDevice = getGainOutputDevice(device);
            uint32_t pga = pSpeech->speechGain[gainOutputDevice][voiceIndex].u32Gain_PGA_Amp;
            //  Analog, INPUT1,  INPUT2 , SPEAKER ,   HP  ,   Digital,      mask
            //xxxxxx,     xx,         xx,      xxxxx,       xxxxx,  xxxxxxxx, xxxx   (32 bits)
            uint8_t digital = pSpeech->speechGain[gainOutputDevice][voiceIndex].u8Gain_digital;
            int mask = getAmpValidMask(mPhoneState, device);

            vol = ((pga >> 10) << 12) | (digital<<4) | mask; //allocate space for digital and mask 
            //vol = ((pga>>4)<<4) | mask; //use resolved bits to store valid mask.
             ALOGV("volIndexToAnalogVol Incall gainOutputDevice %d,index %d, pga 0x%x , dig=0x%x,mask =0x%x,  vol =0x%x",
                   gainOutputDevice,voiceIndex,pga,digital,mask,vol);
        }
        return vol;
    }
    
    int stream = selectStreamForAnalog();
    ALOGV("volIndexToAnalogVol stream %d for Analog ",stream);
    if(stream > AudioSystem::DEFAULT)
    {
        UCMStreamDescriptor &streamDesc = mStreams[stream];
        STREAM_GAIN_CONTROL_STRUCT *pStream = &pGainTable->voiceCall + stream;
        if( AudioSystem::popCount(device) >= 2         && 
            device & AudioSystem::DEVICE_OUT_SPEAKER   &&
            device &(AudioSystem::DEVICE_OUT_WIRED_HEADPHONE | AudioSystem::DEVICE_OUT_WIRED_HEADSET))
        {
            int streameIndex = streamDesc.getVolumeIndex(getDeviceForVolume(device));
            //speaker and headphon are all open.
            //     Analog, INPUT1,  INPUT2 , SPEAKER ,   HP  ,   Resolved,    resolved
            //xxxxxxxx,     xx,         xx,      xxxxx,       xxxxx,  xxxxx,  xxxxx   (32 bits)
           uint32_t pgaSpeaker = pStream->streamGain[GAIN_OUTPUT_SPEAKER][streameIndex].u32Gain_PGA_Amp;
           uint32_t pgaHP = pStream->streamGain[GAIN_OUTPUT_HEADSET][streameIndex].u32Gain_PGA_Amp;
           vol = ((pgaSpeaker >> 15)<<15) | (pgaHP & 0x7FFF); // combile hp gain 
           int mask = getAmpValidMask(mPhoneState, device);
           vol = ((vol>>4)<<4) | mask; //use resolved bits to store valid mask.
           ALOGV("volIndexToAnalogVol  index %d PGASpeaker 0x%x PAGHp 0x%x, mask =0x%x, vol =0x%x",
           streameIndex,pgaSpeaker,pgaHP,mask,vol);
        }
        else
        {
            int streameIndex = streamDesc.getVolumeIndex(getDeviceForVolume(device));
            int gainOutputDevice = getGainOutputDevice(device);
            if(gainOutputDevice == GAIN_OUTPUT_RECEIVER && stream != AudioSystem::VOICE_CALL)
            {
                // sreams use headset param if  it is not VOICE CALL;
                gainOutputDevice = GAIN_OUTPUT_HEADSET;
            }
            uint32_t pga = pStream->streamGain[gainOutputDevice][streameIndex].u32Gain_PGA_Amp;
            int mask = getAmpValidMask(mPhoneState, device);
            vol = ((pga>>4)<<4) | mask; //use resolved bits to store valid mask.
            ALOGV("volIndexToAnalogVol gainOutputDevice %d,index %d, pga 0x%x , mask =0x%x, vol =0x%x",
            gainOutputDevice,streameIndex,pga,mask,vol);
        }
    }
    
    return vol;

}

void AudioYusuUserCaseManager::dumpStreamGainTable()
{
    ALOGV("dumpStreamGainTable streamType ");
    for(int k=0;k<=AUDIO_GAIN_MAX_STREAM; k++)
    {   STREAM_GAIN_CONTROL_STRUCT *pStreamGainTable = &(pGainTable->voiceCall)+k;
        for(int i=0; i < NUM_GAIN_OUTPUT_DEVICES; i++){
            for(int j=0 ; j < GAIN_TABLE_LEVEL; j++){
                AUDIO_GAIN_CONTROL_STRUCT *ptemp = &(pStreamGainTable->streamGain[i][j]);
                ALOGV("level = %d gain = %x gain = %x",
                    j,ptemp->u8Gain_digital,ptemp->u32Gain_PGA_Amp);
            }
        }
   }
}
void AudioYusuUserCaseManager::dumpMiscGainTable(int gainMisc)
{
    switch(gainMisc)
    {
        case AUDIO_GAIN_MIC:
            dumpMicroPhoneGainTable();
            break;
        case AUDIO_GAIN_SIDETONE:
            dumpSidetoneGainTable();
            break;
        case AUDIO_GAIN_SPEECH:
            dumpspeechGainTable();
            break;
        default:
            break;  
    }
    return ;
}

void AudioYusuUserCaseManager::dumpMicroPhoneGainTable()
{
    ALOGV("dumpMicroPhoneGainTable");
    STREAM_MICROPHONE_GAIN_CONTROL_STRUCT *pMicroPhoneGainTable = &(pGainTable->microphoneGain);
    for(int i=0 ; i < NUM_OF_MICGAINS; i+=2){
        ALOGV("gain[%d] = %x, gain[%d] = %x",
                i,pMicroPhoneGainTable->micGain[i],i+1,pMicroPhoneGainTable->micGain[i+1]);
     }
}

void AudioYusuUserCaseManager::dumpSidetoneGainTable()
{
    ALOGV("DumpSidetoneGainTable");
    STREAM_SIDETONE_GAIN_CONTROL_STRUCT *pSidetoneGainTable = &(pGainTable->sidetoneGain);
    for(int i=0 ; i < NUM_OF_SIDETONEGAINS; i+=2){
            ALOGV("gain[%d] = %x, gain[%d] = %x",
                i,pSidetoneGainTable->sidetoneGain[i],i+1,pSidetoneGainTable->sidetoneGain[i+1]);
     }
}

void AudioYusuUserCaseManager::dumpspeechGainTable()
{
    ALOGV("dumpspeechGainTable");
    STREAM_SPEECH_GAIN_CONTROL_STRUCT *pSpeechGainTable = &(pGainTable->speechGain);
    for(int i=0 ; i < NUM_OF_SPEECHSGAINS; i++){
         for(int j=0 ; j < GAIN_TABLE_LEVEL; j++){
             AUDIO_GAIN_CONTROL_STRUCT *ptemp = &(pSpeechGainTable->speechGain[i][j]);
               ALOGV("level = %d digiGain = %x pgaGain = %x",
                   j,ptemp->u8Gain_digital,ptemp->u32Gain_PGA_Amp);
         }
     }

}


AudioYusuUserCaseManager::UCMStreamDescriptor::UCMStreamDescriptor()
    :   mIndexMin(0), mIndexMax(1), mCanBeMuted(true)
{
    mIndexCur.add(AUDIO_DEVICE_OUT_DEFAULT, 0);
}

int AudioYusuUserCaseManager::UCMStreamDescriptor::getVolumeIndex(audio_devices_t device)
{
    device = AudioYusuUserCaseManager::getDeviceForVolume(device);
    // there is always a valid entry for AUDIO_DEVICE_OUT_DEFAULT
    if (mIndexCur.indexOfKey(device) < 0) {
        device = AUDIO_DEVICE_OUT_DEFAULT;
    }
    return mIndexCur.valueFor(device);
}


// ----------------------------------------------------------------------------
}

