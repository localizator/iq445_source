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
#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sched.h>
#include <fcntl.h>

#include <linux/fm.h>

#define LOG_TAG  "AudioYusuVolumeController"
#include <sys/ioctl.h>
#include <utils/Log.h>
#include <utils/String8.h>
#include <media/AudioSystem.h>


#include "AudioYusuHardware.h"
#include "AudioYusuVolumeController.h"
#include "AudioYusuHardware.h"
#include "AudioYusuStreamHandler.h"
#include "AudioYusuDef.h"
#include "audio_custom_exp.h"
#include "AudioCustParam.h"
#include "AudioYusuLad.h"

#include "audio_custom_exp.h"

#include "AudioAnalogAfe.h"
#include "AudioAfe.h"
#include "AudioIoctl.h"

#ifdef ENABLE_LOG_VOLUMECONTROLLER
    #define LOG_VOLUMECONTROLLER ALOGD
#else
    #define LOG_VOLUMECONTROLLER ALOGV
#endif

#if defined(MTK_DT_SUPPORT)
#define MT6252_MAX_ANALOG_GAIN 8
#define MT6252_MIN_ANALOG_GAIN -22
#define MT6252_ANALOG_GAIN_RANGE (MT6252_MAX_ANALOG_GAIN-MT6252_MIN_ANALOG_GAIN)
#endif

#ifdef MTK_AP_SPEECH_ENHANCEMENT
#define HDRECORD_AP_MIN_PGA_GAIN    0xA
#define HDRECORD_AP_SW_AGC_GAIN_DEFAULT 11
#define HDRECORD_AP_TRANSPARENT_AGC_GAIN_OFFSET 33  //for digital microphone
#define HDRECORD_AP_MAX_PGA_GAIN    HDRECORD_AP_MIN_PGA_GAIN+30
#endif

namespace android {

// for matv define
enum
{
    MATV_AUD_VOLUME,
    MATV_VDO_BRIGHTNESS,
    MATV_VDO_CONTRAST,
    MATV_VDO_SATURATION
};

// ----------------------------------------------------------------------------
class AudioYusuHardware;

uint32 AudioYusuVolumeController::GetMultiplier(uint32 dBAttenTotal)
{
    uint32 fGainMultiplier;
    if (dBAttenTotal==0)
    {
        fGainMultiplier = 0x10000;
    }
    else if (dBAttenTotal>200)
    {
        fGainMultiplier = 0;
    }
    else
    {
        fGainMultiplier = (uint32)GainMap[dBAttenTotal-1];
    }
    return fGainMultiplier;
}


static uint16 AudioMapVolume(float volume)
{
    uint16 Gain = (uint16)(volume * (float)VOLUME_STEP);
    //   LOG_VOLUMECONTROLLER("AudioMapVolume, Gain=%d",Gain);

    return Gain;
}

// AudioGetRoutesnned to map Audiosystem device to array value
// 1-->0 2-->1 4-->2  8->3 .....
 int32 AudioYusuVolumeController::AudioGetRoute(uint32 routes)
{
    if(routes <=0)
    {
        ALOGE("AudioGetRoute routes = %d",routes);
        return -1;
    }

    int i=0;
    int mode = -1;
    mAudioHardware->getMode (&mode); // get mode

    if(mAudioHardware->mOuputForceuse[mode] != 0)
    {
        routes = mAudioHardware->GetForceUseOutputDevice (mAudioHardware->mOuputForceuse[mode]);}
    else
    {
        routes = mAudioHardware->Audio_Match_Output_device (routes, mode,false);
    }
    //LOG_VOLUMECONTROLLER("routes = %x ",routes);
    while(routes >1){
        routes = routes>> 1;
        i++;
    }
    if(i < NUM_AUDIO_ROUTE){
        return i;
    }
    else
       return 0;
}

int AudioYusuVolumeController::GetExtDacVolume(int volume)
{
    int Volume_range = DEVICE_MAX_VOLUME-DEVICE_MIN_VOLUME;
    return 255*(Volume_range-volume)/Volume_range;
}

uint32 AudioYusuVolumeController::Calgain(uint16 DegradedBGain,uint32 mode ,uint32 routes)
{
    int32 HW_Gain=0;
    if(mode >= NUM_AUDIO_MODES_TYPES || routes >= NUM_AUDIO_ROUTE_TYPES){
	LOG_VOLUMECONTROLLER(" Calgain out of boundary");
	return -1;
    }
    routes = AudioGetRoute(routes);
    DegradedBGain = (DEVICE_VOLUME_STEP - DegradedBGain) / VOICE_ONEDB_STEP;
    ALOGD("Calgain DegradedBGain = %x  mode = %d routes = %d",DegradedBGain,mode,routes);
    if(DegradedBGain >= mVolumeRange[mode][routes]){
        DegradedBGain = mVolumeRange[mode][routes];
    }
    HW_Gain = DegradedBGain <<1; // 0.5db  1 unit
    return (0xffffffff - HW_Gain);
}

uint32 AudioYusuVolumeController::CalVoicegain(uint16 DegradedBGain,uint32 mode ,uint32 routes)
{
    int32 HW_Gain=0;
    if(mode >= NUM_AUDIO_MODES_TYPES || routes >= NUM_AUDIO_ROUTE_TYPES){
	LOG_VOLUMECONTROLLER(" Calgain out of boundary");
	return -1;
    }
    routes = AudioGetRoute(routes);
    DegradedBGain = (DEVICE_VOLUME_STEP - DegradedBGain) / VOICE_ONEDB_STEP;
    ALOGD("CalVoicegain DegradedBGain = %x  mode = %d routes = %d",DegradedBGain,mode,routes);
    if(DegradedBGain >= mVolumeRange[mode][routes]){
        DegradedBGain = mVolumeRange[mode][routes];
    }
    HW_Gain = DegradedBGain <<1; // 0.5db  1 unit
    return (0xffffffff - HW_Gain);
}

void AudioYusuVolumeController::ApplyHandsetGain(uint16 DegradedBGain,uint32 mode,uint32 routes)
{
    int32 HW_Gain=0;
    int Setting_value =0;

    if(mode >= NUM_AUDIO_MODES_TYPES || routes >= NUM_AUDIO_ROUTE_TYPES){
	LOG_VOLUMECONTROLLER(" Calgain out of boundary");
	return ;
    }
    routes = AudioGetRoute(routes);
    DegradedBGain = (DEVICE_VOLUME_STEP - DegradedBGain) / VOICE_ONEDB_STEP;
    if(DegradedBGain >= mVolumeRange[mode][routes]){
        DegradedBGain = mVolumeRange[mode][routes];
    }
    HW_Gain = mVolumeMax[mode][routes] - DegradedBGain;

    int32 HW_Value[] = {12, 11, 10,  9,   8,   7,   6,   5,   4,   3,   2,   1,  0,
            -1,  -2,  -3,  -4,  -5,  -6,  -7,  -8,  -9, -10,
            -11,  -12, -13, -14, -15, -16, -17, -18, -19, -20,
            -21, -22,  -23, -24, -25, -26, -27, -28, -29, -30, -31, -32};

    for( Setting_value = 0; Setting_value <  44; Setting_value ++ )
    {
        if( HW_Gain >= HW_Value[Setting_value])
            break;
    }
    ALOGD("::ApplyHandsetGain Setting_value=%x",Setting_value);

    mAnaReg->AnalogAFE_Request_ANA_CLK();

   mAnaReg->SetAnaReg(AUDIO_CON1,(Setting_value<<8),0x3f00); // left audio gain,

#ifdef ENABLE_EXT_DAC
    int ExtDacVolume = GetExtDacVolume(Setting_value);
    mAudioHardware->mAudioCustomFunction->EXT_DAC_SetVolume (AUDIO_EXP_DAC_BUF0, ExtDacVolume);
    mAudioHardware->mAudioCustomFunction->EXT_DAC_SetVolume (AUDIO_EXP_DAC_BUF1, ExtDacVolume);
#endif

    mAnaReg->AnalogAFE_Release_ANA_CLK();

    mAfe_handle->SetAfeReg(AFE_DL_SRC2_2,0xffff,0xffff);

}

void AudioYusuVolumeController::ApplyGain(uint16 DegradedBGain,uint32 mode,uint32 routes)
{
    int32 HW_Gain=0;
    int Setting_value =0;

    if(mode >= NUM_AUDIO_MODES_TYPES || routes >= NUM_AUDIO_ROUTE_TYPES){
	LOG_VOLUMECONTROLLER(" Calgain out of boundary");
	return;
    }
    routes = AudioGetRoute(routes);
    DegradedBGain = (DEVICE_VOLUME_STEP - DegradedBGain) / VOICE_ONEDB_STEP;
    //ALOGD("ApplyGain u4Gain = %x  mode = %d routes = %d",DegradedBGain,mode,routes);
    if(DegradedBGain >= mVolumeRange[mode][routes]){
        DegradedBGain = mVolumeRange[mode][routes];
    }
    HW_Gain = mVolumeMax[mode][routes] - DegradedBGain;

    int32 HW_Value[] = {12, 11, 10,  9,   8,   7,   6,   5,   4,   3,   2,   1,  0,
            -1,  -2,  -3,  -4,  -5,  -6,  -7,  -8,  -9, -10,
            -11,  -12, -13, -14, -15, -16, -17, -18, -19, -20,
            -21, -22,  -23, -24, -25, -26, -27, -28, -29, -30, -31, -32};

    for( Setting_value = 0; Setting_value <  44; Setting_value ++ )
    {
        if( HW_Gain >= HW_Value[Setting_value])
            break;
    }
    ALOGD("ApplyGain Setting_value=%x",Setting_value);

    mAnaReg->AnalogAFE_Request_ANA_CLK();


   mAnaReg->SetAnaReg(AUDIO_CON1,Setting_value,0x003f); // left audio gain,
   mAnaReg->SetAnaReg(AUDIO_CON2,Setting_value,0x003f); // right audio gain,

   #ifdef ENABLE_EXT_DAC
       int ExtDacVolume = GetExtDacVolume(Setting_value);
       mAudioHardware->mAudioCustomFunction->EXT_DAC_SetVolume (AUDIO_EXP_DAC_BUF0, ExtDacVolume);
       mAudioHardware->mAudioCustomFunction->EXT_DAC_SetVolume (AUDIO_EXP_DAC_BUF1, ExtDacVolume);
   #endif

    mAnaReg->AnalogAFE_Release_ANA_CLK();

    mAfe_handle->SetAfeReg(AFE_DL_SRC2_2,0xffff,0xffff);

}

void AudioYusuVolumeController::ApplyGainInCall(uint16 DegradedBGain,uint32 mode,uint32 routes)
{
    int32 HW_Gain=0;
    int Setting_value =0;

    if(mode >= NUM_AUDIO_MODES_TYPES || routes >= NUM_AUDIO_ROUTE_TYPES){
	LOG_VOLUMECONTROLLER(" Calgain out of boundary");
	return;
    }
    routes = AudioGetRoute(routes);
    DegradedBGain = (DEVICE_VOLUME_STEP - DegradedBGain) / VOICE_ONEDB_STEP;
    //ALOGD("ApplyGainInCall u4Gain = %x  mode = %d routes = %d",DegradedBGain,mode,routes);
    if(DegradedBGain >= mVolumeRange[mode][routes]){
        DegradedBGain = mVolumeRange[mode][routes];
    }
    HW_Gain = mVolumeMax[mode][routes] - DegradedBGain;

    int32 HW_Value[] = {12, 11, 10,  9,   8,   7,   6,   5,   4,   3,   2,   1,  0,
            -1,  -2,  -3,  -4,  -5,  -6,  -7,  -8,  -9, -10,
            -11,  -12, -13, -14, -15, -16, -17, -18, -19, -20,
            -21, -22,  -23, -24, -25, -26, -27, -28, -29, -30, -31, -32};

    for( Setting_value = 0; Setting_value <  45; Setting_value ++ )
    {
        if( HW_Gain >= HW_Value[Setting_value])
            break;
    }

    if(Setting_value > 0){
        Setting_value = Setting_value-1;
    }
    ALOGD("ApplyGainInCall, Setting_value=%x HW_Value = %x",Setting_value,HW_Value[Setting_value]);
    mAnaReg->AnalogAFE_Request_ANA_CLK();
    mAnaReg->SetAnaReg(AUDIO_CON1,Setting_value,0x3f); // left audio gain
    mAnaReg->SetAnaReg(AUDIO_CON2,Setting_value,0x3f); // right audio gain
    mAnaReg->AnalogAFE_Release_ANA_CLK();

    #ifdef ENABLE_EXT_DAC
    int ExtDacVolume = GetExtDacVolume(Setting_value);
    mAudioHardware->mAudioCustomFunction->EXT_DAC_SetVolume (AUDIO_EXP_DAC_BUF0, ExtDacVolume);
    mAudioHardware->mAudioCustomFunction->EXT_DAC_SetVolume (AUDIO_EXP_DAC_BUF1, ExtDacVolume);
    #endif
}

// this is use for meta mode
void AudioYusuVolumeController::ApplyGain(uint16 Gain, uint32 routes)
{
    int32 Setting_value,HW_Gain=0,Final_gain_value;
    uint32 u4Gain  = VOLUME_STEP - Gain;  // find degarde volume
    LOG_VOLUMECONTROLLER(":ApplyGain Gain:%x, u4Gain=%x",Gain,u4Gain);

    int32 HW_Value[] = {12, 11, 10,  9,   8,   7,   6,   5,   4,   3,   2,   1,  0,
            -1,  -2,  -3,  -4,  -5,  -6,  -7,  -8,  -9, -10,
            -11,  -12, -13, -14, -15, -16, -17, -18, -19, -20,
            -21, -22,  -23, -24, -25, -26, -27, -28, -29, -30, -31, -32};
    if(Gain !=  0)
    {
        u4Gain = u4Gain * mVolumeRange[android_audio_legacy::AudioSystem::MODE_NORMAL][routes]; // appli to normal mode
//        LOG_VOLUMECONTROLLER(":ApplyGain u4Gain=%x, mVolumeRange:%d",u4Gain,mVolumeRange[android_audio_legacy::AudioSystem::MODE_NORMAL][routes]);
        HW_Gain = (u4Gain /  VOLUME_STEP) ; //actual DB
        //LOG_VOLUMECONTROLLER(":ApplyGain HW_Gain=%d, mVolumeMax:%d",HW_Gain,mVolumeMax[android_audio_legacy::AudioSystem::MODE_NORMAL][routes]);
        HW_Gain = mVolumeMax[android_audio_legacy::AudioSystem::MODE_NORMAL][routes] - HW_Gain ;
        //LOG_VOLUMECONTROLLER(":ApplyGain HW_Gain=%d",HW_Gain);
    }
    else
    {
        HW_Gain = -100;
    }
    for( Setting_value = 0; Setting_value <  44; Setting_value ++ )
    {
        if( HW_Gain >= HW_Value[Setting_value])
            break;
    }
    ALOGD(":ApplyGain Setting_value = %d",Setting_value);

    mAnaReg->AnalogAFE_Request_ANA_CLK();

   mAnaReg->SetAnaReg(AUDIO_CON1,Setting_value,0x003f); // left audio gain
   mAnaReg->SetAnaReg(AUDIO_CON2,Setting_value,0x003f); // right audio gain

    mAnaReg->AnalogAFE_Release_ANA_CLK();

    Final_gain_value = HW_Value[Setting_value] - HW_Gain;
//    LOG_VOLUMECONTROLLER(":ApplyGain !! Final_gain_value=%d, HW_Value:%d",Final_gain_value,HW_Value[Setting_value]);

    Final_gain_value = (Final_gain_value <<1 );
    HW_Gain = (GetMultiplier(Final_gain_value) + 0x8 )>> 4;
//    LOG_VOLUMECONTROLLER(":ApplyGain !! HW_Gain=0x%x",HW_Gain);

    mAfe_handle->SetAfeReg(AFE_DL_SRC2_2,0xffff,0xffff);

#ifdef ENABLE_EXT_DAC
    int ExtDacVolume = GetExtDacVolume(Setting_value);
    mAudioHardware->mAudioCustomFunction->EXT_DAC_SetVolume (AUDIO_EXP_DAC_BUF0, ExtDacVolume);
    mAudioHardware->mAudioCustomFunction->EXT_DAC_SetVolume (AUDIO_EXP_DAC_BUF1, ExtDacVolume);
#endif
}


AudioYusuVolumeController::AudioYusuVolumeController(int Fd,AudioYusuHardware *hw, AudioAfe *AfeHdr, AudioAnalog *AnaReg)
{
    LOG_VOLUMECONTROLLER("Constructor");
    mVolume = 1.0 ; // need modify
    mAfe_handle = AfeHdr;
    mAnaReg = AnaReg;
    mFd =Fd;
    mAudioHardware =hw;
    mFmVolume = 5;
    mFmMute = false;
    mMatvVolume =0;
    mfmFd = -1;
    mMatvMute = false;
    memset((void*)&sndParam,0, sizeof(AUDIO_CUSTOM_PARAM_STRUCT));
#ifdef MTK_AUDIO_GAIN_TABLE
    memset(&mGainTable,0, sizeof(AUDIO_GAIN_TABLE_STRUCT));
#endif
}

bool AudioYusuVolumeController::HandleFmFd(bool Enable)
{
    ALOGD("HandleFmFd mfmFd = %d",mfmFd);
    if(Enable && mfmFd <= 0){
        mfmFd = open(FM_DEVICE_NAME, O_RDWR);
        if (mfmFd < 0)
            ALOGE("FAIL open mfmFd  failed");
        else{
            ALOGD("open(FM_DEVICE_NAME, O_RDWR) success mfmFd = %d",mfmFd);
        }
    }
    else if (Enable && mfmFd >0){
        ALOGD("HandleFmFd  Enable and mfmFd = %d",mfmFd);
    }
    else{
        if(mfmFd){
            close(mfmFd);
            mfmFd =0;
            ALOGD("close mfmFd and mfmFd = %d",mfmFd);
        }
        else
            ALOGE("HandleFmFd mfmFd <0");
    }
    return true;
}



AudioYusuVolumeController::~AudioYusuVolumeController(){
    return;
}

bool AudioYusuVolumeController::SetVolumeRange(uint32 mode,uint32 route,int32 MaxVolume, int32 MinVolume){
    if(mode >= NUM_AUDIO_MODES_TYPES)
    {
        LOG_VOLUMECONTROLLER("SetVolumeRange mode >= NUM_AUDIO_MODES_TYPES");
        return false;
    }
    if(route >= NUM_AUDIO_ROUTE)
    {
        LOG_VOLUMECONTROLLER("SetVolumeRange mode >= NUM_AUDIO_ROUTE_TYPES");
        return false;
    }
    mVolumeMax[mode][route] = MaxVolume;
    mVolumeMin[mode][route] = MinVolume;
    mVolumeRange[mode][route] = DEVICE_VOLUME_RANGE;
    LOG_VOLUMECONTROLLER("SetVolumeRange mode=%x, route=%x, MaxVolume=%d, MinVolume=%d ",mode,route,MaxVolume,MinVolume);
    return true;
}

//this function will map 0~255 ==> 100~0
static float Mamp255Volumeto0(char Volume)
{
    float floatVolume  = (float)Volume;
    floatVolume =  100.0 - (Volume)/255.0*100.0 + 0.5;
    ALOGD("Mamp255Volumeto0 floatVolume = %f Volume = %d",floatVolume,Volume);
    return floatVolume;
}

//this function map 255 ==> Audiocustomvolume
static float MampAudioVolume(char Volume)
{
    if(Volume > AUDIO_VOLUME_MAX){
        Volume = AUDIO_VOLUME_MAX;
    }
    float DegradedB = AUDIO_VOLUME_MAX - Volume;
    DegradedB = DegradedB / AUDIO_ONEDB_STEP; // how many dB degrade

    ALOGD("Volume = %d MampAudioVolume DegradedB = %f ",Volume,DegradedB);
    return DegradedB;
}

//this function map 255 ==> Audiocustomvolume
static float MampVoiceVolume(char Volume)
{
    if(Volume > VOICE_VOLUME_MAX){
        Volume = VOICE_VOLUME_MAX;
    }
    float DegradedB = VOICE_VOLUME_MAX - Volume;
    DegradedB = DegradedB / VOICE_ONEDB_STEP; // how many dB degrade

    ALOGD("Volume = %d MampVoiceVolume DegradedB = %f ",Volume,DegradedB);
    return DegradedB;
}

//this function map 255 ==> Audiocustomvolume
static float MampUplinkGain(char Volume)
{
#ifndef MTK_AUDIO_GAIN_TABLE
    if(Volume > UPLINK_GAIN_MAX){
        Volume = UPLINK_GAIN_MAX;
    }
    float DegradedB = UPLINK_GAIN_MAX - Volume;
    DegradedB = DegradedB / UPLINK_ONEDB_STEP; // how many dB degrade
#else
    if(Volume > MIC_GAIN_MAX){
        Volume = MIC_GAIN_MAX;
    }
    float DegradedB = MIC_GAIN_MAX - Volume;
#endif
    ALOGD("Volume = %d UPLINK_GAIN_MAX DegradedB = %f ",Volume,DegradedB);
    return DegradedB;
}

//this function map 255 ==> Audiocustomvolume
static float MampSidetoneGain(char Volume)
{
    float DegradedB;
    if(Volume > SIDETONE_GAIN_MAX){
        Volume = SIDETONE_GAIN_MAX;
    }
    if (Volume != 0){
        DegradedB = SIDETONE_GAIN_MAX - Volume;
        DegradedB = DegradedB / SIDETONE_ONEDB_STEP; // how many dB degrade
    }
    else{
        DegradedB = SIDETONE_MAX_GAIN_DEGRADE;
    }
    ALOGD("Volume = %d MampSidetoneGain DegradedB = %f ",Volume,DegradedB);
    return DegradedB;
}

//this function map 255 ==> Audiocustomvolume
static float MampLevelShiftBufferGain(char Volume)
{
    float DegradedB;
#ifndef MTK_AUDIO_GAIN_TABLE
    if(Volume > LEVEL_SHIFT_BUFFER_GAIN_MAX){
        Volume = LEVEL_SHIFT_BUFFER_GAIN_MAX;
    }
    if (Volume != 0){
        DegradedB = LEVEL_SHIFT_BUFFER_GAIN_MAX - Volume;
        DegradedB = DegradedB / LEVEL_SHIFT_ONEDB_STEP; // how many dB degrade
    }
    if(DegradedB > LEVEL_SHIFT_BUFFER_DEGRADE_MAXDB)
        DegradedB = LEVEL_SHIFT_BUFFER_DEGRADE_MAXDB;
#else
    if(Volume > LINEIN_MAX_GAIN_DEGRADE){
        Volume = LINEIN_MAX_GAIN_DEGRADE;
    }
    DegradedB = Volume;
#endif
    ALOGD("Volume = %d MampLevelShiftBufferGain DegradedB = %f ",Volume,DegradedB);
    return DegradedB;
}

// this will return
static uint32 MapLineInGain(unsigned char Volume)
{
    uint32 gain =0;
    if(Volume > LINEIN_GAIN_MAX){
        Volume = LINEIN_GAIN_MAX;
    }
    if(Volume != 0){
        gain +=  (LINEIN_GAIN_MAX - Volume)/LINEIN_GAIN_DB_STEP;
    }
    if(gain > LINEIN_MAX_GAIN_DEGRADE){
        gain = LINEIN_MAX_GAIN_DEGRADE;
    }
    return gain;
}


static float Mamp0Volumeto100(char Volume)
{
    float floatVolume  = (float)Volume;
    floatVolume =  255.0 - (Volume)/100.0*255.0 + 0.5;
    ALOGD("Mamp0Volumeto100 floatVolume = %f Volume = %d",floatVolume,Volume);
    return floatVolume;
}

bool AudioYusuVolumeController::InitVolumeController()
{
    LOG_VOLUMECONTROLLER("InitVolumeController");
    GetCustParamFromNV(&sndParam);
    GetAudioCustomParamFromNV(&VolumeCustomParam);

    // for normal platyback , let audio drvier can achevie maximun volume , and let computecustomvolume to
    // set mastervolume

    float MaxdB =0.0,MindB =0.0,micgain=0.0;
    int degradegain =0;

    SetVolumeRange(android_audio_legacy::AudioSystem::MODE_NORMAL,ROUTE_OUT_EARPIECE,
         DEVICE_VOICE_MAX_VOLUME ,DEVICE_VOICE_MIN_VOLUME);

    degradegain =(unsigned char)MampAudioVolume(VolumeCustomParam.audiovolume_media[VOL_NORMAL][6]);
    SetVolumeRange(android_audio_legacy::AudioSystem::MODE_NORMAL,ROUTE_OUT_SPEAKER,
        DEVICE_MAX_VOLUME - degradegain ,DEVICE_MIN_VOLUME);
    degradegain =(unsigned char)MampAudioVolume(VolumeCustomParam.audiovolume_media[VOL_NORMAL][5]);
    SetVolumeRange(android_audio_legacy::AudioSystem::MODE_NORMAL,ROUTE_OUT_WIRED_HEADSET,
        DEVICE_MAX_VOLUME - degradegain ,DEVICE_MIN_VOLUME);
    SetVolumeRange(android_audio_legacy::AudioSystem::MODE_NORMAL,ROUTE_OUT_WIRED_HEADPHONE,
        DEVICE_MAX_VOLUME - degradegain ,DEVICE_MIN_VOLUME);

    //---------------temp for no use---------------------------------
    SetVolumeRange(android_audio_legacy::AudioSystem::MODE_NORMAL,ROUTE_OUT_BLUETOOTH_SCO,DEVICE_MAX_VOLUME,DEVICE_MIN_VOLUME);
    SetVolumeRange(android_audio_legacy::AudioSystem::MODE_NORMAL,ROUTE_OUT_BLUETOOTH_SCO_HEADSET,DEVICE_MAX_VOLUME,DEVICE_MIN_VOLUME);
    SetVolumeRange(android_audio_legacy::AudioSystem::MODE_NORMAL,ROUTE_OUT_BLUETOOTH_A2DP,DEVICE_MAX_VOLUME,DEVICE_MIN_VOLUME);
    SetVolumeRange(android_audio_legacy::AudioSystem::MODE_NORMAL,ROUTE_OUT_BLUETOOTH_A2DP_HEADPHONES,DEVICE_MAX_VOLUME,DEVICE_MIN_VOLUME);
    SetVolumeRange(android_audio_legacy::AudioSystem::MODE_NORMAL,ROUTE_OUT_BLUETOOTH_A2DP_SPEAKER,DEVICE_MAX_VOLUME,DEVICE_MIN_VOLUME);
    SetVolumeRange(android_audio_legacy::AudioSystem::MODE_NORMAL,ROUTE_OUT_AUX_DIGITAL,DEVICE_MAX_VOLUME,DEVICE_MIN_VOLUME);
   //----------------------------------------------------------


   SetVolumeRange(android_audio_legacy::AudioSystem::MODE_NORMAL,ROUTE_OUT_EARPIECE,
        DEVICE_VOICE_MAX_VOLUME ,DEVICE_VOICE_MIN_VOLUME);

    // when ringtone
    degradegain =(unsigned char)MampAudioVolume(VolumeCustomParam.audiovolume_media[VOL_NORMAL][6]);
    SetVolumeRange(android_audio_legacy::AudioSystem::MODE_RINGTONE,ROUTE_OUT_SPEAKER,
        DEVICE_MAX_VOLUME - degradegain ,DEVICE_MIN_VOLUME);
    SetVolumeRange(android_audio_legacy::AudioSystem::MODE_RINGTONE,ROUTE_OUT_WIRED_HEADSET,
        DEVICE_MAX_VOLUME - degradegain ,DEVICE_MIN_VOLUME);
    SetVolumeRange(android_audio_legacy::AudioSystem::MODE_RINGTONE,ROUTE_OUT_WIRED_HEADPHONE,
        DEVICE_MAX_VOLUME - degradegain ,DEVICE_MIN_VOLUME);

    //---------------temp for no use---------------------------------
    SetVolumeRange(android_audio_legacy::AudioSystem::MODE_RINGTONE,ROUTE_OUT_BLUETOOTH_SCO,DEVICE_MAX_VOLUME,DEVICE_MIN_VOLUME);
    SetVolumeRange(android_audio_legacy::AudioSystem::MODE_RINGTONE,ROUTE_OUT_BLUETOOTH_SCO_HEADSET,DEVICE_MAX_VOLUME,DEVICE_MIN_VOLUME);
    SetVolumeRange(android_audio_legacy::AudioSystem::MODE_RINGTONE,ROUTE_OUT_BLUETOOTH_A2DP,DEVICE_MAX_VOLUME,DEVICE_MIN_VOLUME);
    SetVolumeRange(android_audio_legacy::AudioSystem::MODE_RINGTONE,ROUTE_OUT_BLUETOOTH_A2DP_HEADPHONES,DEVICE_MAX_VOLUME,DEVICE_MIN_VOLUME);
    SetVolumeRange(android_audio_legacy::AudioSystem::MODE_RINGTONE,ROUTE_OUT_BLUETOOTH_A2DP_SPEAKER,DEVICE_MAX_VOLUME,DEVICE_MIN_VOLUME);
    SetVolumeRange(android_audio_legacy::AudioSystem::MODE_RINGTONE,ROUTE_OUT_AUX_DIGITAL,DEVICE_MAX_VOLUME,DEVICE_MIN_VOLUME);
    //---------------------------------------------------------------------

    SetVolumeRange(android_audio_legacy::AudioSystem::MODE_IN_CALL,ROUTE_OUT_EARPIECE, DEVICE_VOICE_MAX_VOLUME ,DEVICE_VOICE_MIN_VOLUME);
    SetVolumeRange(android_audio_legacy::AudioSystem::MODE_IN_CALL,ROUTE_OUT_SPEAKER,  DEVICE_MAX_VOLUME ,DEVICE_MIN_VOLUME);
    SetVolumeRange(android_audio_legacy::AudioSystem::MODE_IN_CALL,ROUTE_OUT_WIRED_HEADSET,  DEVICE_MAX_VOLUME ,DEVICE_MIN_VOLUME);
    SetVolumeRange(android_audio_legacy::AudioSystem::MODE_IN_CALL,ROUTE_OUT_WIRED_HEADPHONE, DEVICE_MAX_VOLUME ,DEVICE_MIN_VOLUME);

    //---------------temp for no use---------------------------------
    SetVolumeRange(android_audio_legacy::AudioSystem::MODE_IN_CALL,ROUTE_OUT_BLUETOOTH_SCO,DEVICE_MAX_VOLUME,DEVICE_MIN_VOLUME);
    SetVolumeRange(android_audio_legacy::AudioSystem::MODE_IN_CALL,ROUTE_OUT_BLUETOOTH_SCO_HEADSET,DEVICE_MAX_VOLUME,DEVICE_MIN_VOLUME);
    SetVolumeRange(android_audio_legacy::AudioSystem::MODE_IN_CALL,ROUTE_OUT_BLUETOOTH_A2DP,DEVICE_MAX_VOLUME,DEVICE_MIN_VOLUME);
    SetVolumeRange(android_audio_legacy::AudioSystem::MODE_IN_CALL,ROUTE_OUT_BLUETOOTH_A2DP_HEADPHONES,DEVICE_MAX_VOLUME,DEVICE_MIN_VOLUME);
    SetVolumeRange(android_audio_legacy::AudioSystem::MODE_IN_CALL,ROUTE_OUT_BLUETOOTH_A2DP_SPEAKER,DEVICE_MAX_VOLUME,DEVICE_MIN_VOLUME);
    SetVolumeRange(android_audio_legacy::AudioSystem::MODE_IN_CALL,ROUTE_OUT_AUX_DIGITAL,DEVICE_MAX_VOLUME,DEVICE_MIN_VOLUME);
    //-------------------------------------------------------------


    //  Commucation mode
     // Fix me , is that commication mode needs a custom volume structure
    degradegain =(unsigned char)MampAudioVolume(VolumeCustomParam.audiovolume_media[VOL_NORMAL][0]);
    SetVolumeRange(android_audio_legacy::AudioSystem::MODE_IN_COMMUNICATION,ROUTE_OUT_EARPIECE,
        DEVICE_MAX_VOLUME - degradegain ,DEVICE_MIN_VOLUME);
    degradegain =(unsigned char)MampAudioVolume(VolumeCustomParam.audiovolume_media[VOL_NORMAL][1]);
    SetVolumeRange(android_audio_legacy::AudioSystem::MODE_IN_COMMUNICATION,ROUTE_OUT_SPEAKER,
        DEVICE_MAX_VOLUME - degradegain ,DEVICE_MIN_VOLUME);
    degradegain =(unsigned char)MampAudioVolume(VolumeCustomParam.audiovolume_media[VOL_NORMAL][2]);
    SetVolumeRange(android_audio_legacy::AudioSystem::MODE_IN_COMMUNICATION,ROUTE_OUT_WIRED_HEADSET,
        DEVICE_MAX_VOLUME - degradegain ,DEVICE_MIN_VOLUME);
    SetVolumeRange(android_audio_legacy::AudioSystem::MODE_IN_COMMUNICATION,ROUTE_OUT_WIRED_HEADPHONE,
        DEVICE_MAX_VOLUME - degradegain ,DEVICE_MIN_VOLUME);

    //---------------temp for no use---------------------------------
    SetVolumeRange(android_audio_legacy::AudioSystem::MODE_IN_COMMUNICATION,ROUTE_OUT_BLUETOOTH_SCO,DEVICE_MAX_VOLUME,DEVICE_MIN_VOLUME);
    SetVolumeRange(android_audio_legacy::AudioSystem::MODE_IN_COMMUNICATION,ROUTE_OUT_BLUETOOTH_SCO_HEADSET,DEVICE_MAX_VOLUME,DEVICE_MIN_VOLUME);
    SetVolumeRange(android_audio_legacy::AudioSystem::MODE_IN_COMMUNICATION,ROUTE_OUT_BLUETOOTH_A2DP,DEVICE_MAX_VOLUME,DEVICE_MIN_VOLUME);
    SetVolumeRange(android_audio_legacy::AudioSystem::MODE_IN_COMMUNICATION,ROUTE_OUT_BLUETOOTH_A2DP_HEADPHONES,DEVICE_MAX_VOLUME,DEVICE_MIN_VOLUME);
    SetVolumeRange(android_audio_legacy::AudioSystem::MODE_IN_COMMUNICATION,ROUTE_OUT_BLUETOOTH_A2DP_SPEAKER,DEVICE_MAX_VOLUME,DEVICE_MIN_VOLUME);
    SetVolumeRange(android_audio_legacy::AudioSystem::MODE_IN_COMMUNICATION,ROUTE_OUT_AUX_DIGITAL,DEVICE_MAX_VOLUME,DEVICE_MIN_VOLUME);
   //----------------------------------------------------------

    // init Mic gaindata
    #ifndef MTK_AUDIO_GAIN_TABLE
    ALOGD(" not define MTK_AUDIO_GAIN_TABLE");
    degradegain =(unsigned char)MampUplinkGain(VolumeCustomParam.audiovolume_mic[VOL_NORMAL][4]);
    SetMicGain(Idle_Normal_Record, -1 * degradegain <<1);
    degradegain =(unsigned char)MampUplinkGain(VolumeCustomParam.audiovolume_mic[VOL_HEADSET][4]);
    SetMicGain(Idle_Headset_Record, -1 * degradegain <<1);
    degradegain =(unsigned char)MampUplinkGain(VolumeCustomParam.audiovolume_mic[VOL_NORMAL][3]);
    SetMicGain(Normal_Mic,         -1 *  degradegain<<1);
    degradegain =(unsigned char)MampUplinkGain(VolumeCustomParam.audiovolume_mic[VOL_HEADSET][3]);
    SetMicGain(Headset_Mic,        -1 * degradegain<<1);
    degradegain =(unsigned char)MampUplinkGain(VolumeCustomParam.audiovolume_mic[VOL_HANDFREE][3]);
    SetMicGain(Handfree_Mic,       -1 * degradegain<<1);
    degradegain =(unsigned char)MampUplinkGain(VolumeCustomParam.audiovolume_mic[VOL_NORMAL][0]);
    SetMicGain(TTY_CTM_Mic ,      -1 * degradegain<<1);

    // voice reconition usage
    degradegain =(unsigned char)MampUplinkGain(VolumeCustomParam.audiovolume_mic[VOL_NORMAL][5]);
    SetMicGain(Voice_Rec_Mic_Handset, -1 * degradegain <<1);
    degradegain =(unsigned char)MampUplinkGain(VolumeCustomParam.audiovolume_mic[VOL_NORMAL][5]);
    SetMicGain(Voice_Rec_Mic_Headset, -1 * degradegain <<1);

    // idle video recording usage
    degradegain =(unsigned char)MampUplinkGain(VolumeCustomParam.audiovolume_mic[VOL_NORMAL][2]);
    SetMicGain(Idle_Video_Record_Handset, -1 * degradegain <<1);
    degradegain =(unsigned char)MampUplinkGain(VolumeCustomParam.audiovolume_mic[VOL_HEADSET][2]);
    SetMicGain(Idle_Video_Record_Headset, -1 * degradegain <<1);

    // add by chiepeng for VOIP mode
    degradegain =(unsigned char)MampUplinkGain(VolumeCustomParam.audiovolume_mic[VOL_NORMAL][6]);
    SetMicGain(VOIP_Normal_Mic,    -1 * degradegain<<1);
    degradegain =(unsigned char)MampUplinkGain(VolumeCustomParam.audiovolume_mic[VOL_HEADSET][6]);
    SetMicGain(VOIP_Headset_Mic,   -1 * degradegain<<1);
    degradegain =(unsigned char)MampUplinkGain(VolumeCustomParam.audiovolume_mic[VOL_HANDFREE][6]);
    SetMicGain(VOIP_Handfree_Mic, -1 * degradegain<<1);

    // level shift buffer, level shift buffer gain is used degrade gain.
    degradegain =(unsigned char)MampLevelShiftBufferGain(VolumeCustomParam.audiovolume_mic[VOL_NORMAL][1]);
    SetLevelShiftBufferGain(Level_Shift_buffer,  degradegain);

    for(int i= 0; i<Num_Mic_Gain ; i++)
        ALOGD("micgain %d = %x",i,mMicGain[i]);

    // init sidetone data
    degradegain =(unsigned char)MampSidetoneGain(VolumeCustomParam.audiovolume_sid[VOL_NORMAL][3]);
    SetSideTone( 0   , -1 * degradegain<<1);
    degradegain =(unsigned char)MampSidetoneGain(VolumeCustomParam.audiovolume_sid[VOL_HEADSET][3]);
    SetSideTone( 2   , -1 * degradegain<<1);
    degradegain = 96;
    SetSideTone( 3   , -1 * degradegain<<1); // set headphone sidetone to mute
    degradegain =(unsigned char)MampSidetoneGain(VolumeCustomParam.audiovolume_sid[VOL_HANDFREE][3]);
    SetSideTone( 1   , -1 * degradegain<<1);

    #else
    ALOGD(" MTK_AUDIO_GAIN_TABLE");
    GetAudioGainTableParamFromNV(&mGainTable);
    degradegain = (unsigned char)MampUplinkGain(mGainTable.microphoneGain.micGain[GAIN_IDLE_RECORD_MIC]);
    SetMicGain(Idle_Normal_Record, -1 * degradegain <<1);
    degradegain = (unsigned char)MampUplinkGain(mGainTable.microphoneGain.micGain[GAIN_IDLE_RECORD_HEADSET]);
    SetMicGain(Idle_Headset_Record, -1 * degradegain <<1);
    degradegain = (unsigned char)MampUplinkGain(mGainTable.microphoneGain.micGain[GAIN_INCALL_RECEIVER]);
    SetMicGain(Normal_Mic,         -1 *  degradegain<<1);
    degradegain = (unsigned char)MampUplinkGain(mGainTable.microphoneGain.micGain[GAIN_INCALL_HEADSET]);
    SetMicGain(Headset_Mic,        -1 * degradegain<<1);
    degradegain = (unsigned char)MampUplinkGain(mGainTable.microphoneGain.micGain[GAIN_INCALL_SPEAKER]);
    SetMicGain(Handfree_Mic,       -1 * degradegain<<1);
    //degradegain =(unsigned char)MampUplinkGain(mGainTable.microphoneGain.micGain[GAIN_TTY_DEVICE]);
    degradegain = (unsigned char)MampUplinkGain(mGainTable.microphoneGain.micGain[GAIN_IDLE_RECORD_MIC]);
    SetMicGain(TTY_CTM_Mic ,      -1 * degradegain<<1);
    degradegain = (unsigned char)MampLevelShiftBufferGain(mGainTable.microphoneGain.micGain[GAIN_FM_RECORDING]);
    SetLevelShiftBufferGain(Level_Shift_buffer, degradegain);
    degradegain = (unsigned char)MampUplinkGain(mGainTable.microphoneGain.micGain[GAIN_VOIP_RECEIVER]);
    SetMicGain(VOIP_Normal_Mic,    -1 * degradegain<<1);
    degradegain = (unsigned char)MampUplinkGain(mGainTable.microphoneGain.micGain[GAIN_VOIP_HEADSET]);
    SetMicGain(VOIP_Headset_Mic,   -1 * degradegain<<1);
    degradegain = (unsigned char)MampUplinkGain(mGainTable.microphoneGain.micGain[GAIN_VOIP_SPEAKER]);
    SetMicGain(VOIP_Handfree_Mic, -1 * degradegain<<1);
    //degradegain = (unsigned char)MampUplinkGain(mGainTable.microphoneGain.micGain[GAIN_VOICE_RECOGNITION]);
    degradegain = (unsigned char)MampUplinkGain(mGainTable.microphoneGain.micGain[GAIN_IDLE_RECORD_MIC]);
    SetMicGain(Voice_Rec_Mic_Handset, -1 * degradegain<<1);

    // init sidetone data
    degradegain =(unsigned char)MampSidetoneGain(mGainTable.sidetoneGain.sidetoneGain[GAIN_SIDETONE_RECEIVER]);
    SetSideTone( 0   , -1 * degradegain<<1);
    degradegain =(unsigned char)MampSidetoneGain(mGainTable.sidetoneGain.sidetoneGain[GAIN_SIDETONE_HEADSET]);
    SetSideTone( 1   , -1 * degradegain<<1);
    degradegain =(unsigned char)MampSidetoneGain(mGainTable.sidetoneGain.sidetoneGain[GAIN_SIDETONE_SPEAKER]);
    SetSideTone( 2   , -1 * degradegain<<1);
    #endif

    for(int i= 0; i<Num_Mic_Gain ; i++)
        ALOGD("micgain %d = %x",i,mMicGain[i]);

    return true;
}

bool AudioYusuVolumeController::SetLadMicGain(int MicType)
{
    //need modify for add headset mic
    int32 MicMode;
    mAudioHardware->getMode(&MicMode);
    ALOGD("SetLadMicGain MicMode = %d MicType = %d",MicMode,MicType);
    if(MicMode == android_audio_legacy::AudioSystem::MODE_IN_CALL) // incall mode
    {
        ALOGD("SetLadMicGain mMicGain[%d]=%d (%x)",MicType,mMicGain[MicType],mMicGain[MicType]);
        mAudioHardware->pLad->LAD_SetMicrophoneVolume( mMicGain[MicType]);
        return true;
    }
    else if(MicMode == android_audio_legacy::AudioSystem::MODE_IN_COMMUNICATION)
    {
        ALOGD("SetLadMicGain mMicGain[%d]=%d (%x)",MicType,mMicGain[MicType],mMicGain[MicType]);
        mAudioHardware->pLad->LAD_SetMicrophoneVolume( mMicGain[MicType]);
        return true;
    }
    else // normal
    {
        ALOGD("SetLadMicGain mMicGain=%d (%x)",mMicGain[MicType],mMicGain[MicType]);
        mAudioHardware->pLad->LAD_SetMicrophoneVolume( mMicGain[MicType]);
        return true;
    }
    return true;
}

void AudioYusuVolumeController::SetLineInGainByMode(uint32 mode)
{
    switch(mode){
        case LINE_GAIN_FM:
            SetLineInGain(VolumeCustomParam.audiovolume_mic[VOL_HEADSET][5]);
            break;
        case LINE_GAIN_MATV:
            SetLineInGain(VolumeCustomParam.audiovolume_mic[VOL_HANDFREE][5]);
            break;
        default:
            ALOGW("SetLineInGainByMode nut sno mode select %d",mode);
            break;
    }
}

void AudioYusuVolumeController::SetLineInGain(uint32 gain)
{
    gain = MapLineInGain(gain);
    mAudioHardware->mAnaReg->AnalogAFE_Set_LineIn_Gain (gain);
}

bool AudioYusuVolumeController::SetApMicGain(uint32 MicMode)
{
    //only in normal mode record
    uint32 Pga_gain =0;
    uint32 degradegain =0;
#ifndef MTK_AUDIO_GAIN_TABLE
    switch (MicMode){
        case Idle_Normal_Record:
            degradegain = (uint32)MampUplinkGain(VolumeCustomParam.audiovolume_mic[VOL_NORMAL][4]);
            break;
        case Idle_Headset_Record:
            degradegain = (uint32)MampUplinkGain(VolumeCustomParam.audiovolume_mic[VOL_HEADSET][4]);
            break;
        case Fm_Record:
            degradegain = (uint32)MampUplinkGain(VolumeCustomParam.audiovolume_mic[VOL_HEADSET][5]);
            break;
        case Voice_Rec_Mic_Handset:
            degradegain = (uint32)MampUplinkGain(VolumeCustomParam.audiovolume_mic[VOL_NORMAL][5]);
            break;
        case Voice_Rec_Mic_Headset:
            degradegain = (uint32)MampUplinkGain(VolumeCustomParam.audiovolume_mic[VOL_HEADSET][4]);
            break;    
        case VOIP_Normal_Mic:
            degradegain = (uint32)MampUplinkGain(VolumeCustomParam.audiovolume_mic[VOL_NORMAL][6]);
            break;
        case VOIP_Headset_Mic:
            degradegain = (uint32)MampUplinkGain(VolumeCustomParam.audiovolume_mic[VOL_HEADSET][6]);
            break;
        case VOIP_Handfree_Mic:
            degradegain = (uint32)MampUplinkGain(VolumeCustomParam.audiovolume_mic[VOL_HANDFREE][6]);
            break;
        case Idle_Video_Record_Handset:
            degradegain = (uint32)MampUplinkGain(VolumeCustomParam.audiovolume_mic[VOL_NORMAL][2]);
            break;
        case Idle_Video_Record_Headset:
            degradegain = (uint32)MampUplinkGain(VolumeCustomParam.audiovolume_mic[VOL_HEADSET][2]);
            break;  
        default:
            break;
    }
#else
switch (MicMode){
    case Idle_Normal_Record:
        degradegain = (uint32)MampUplinkGain(mGainTable.microphoneGain.micGain[GAIN_IDLE_RECORD_MIC]);
        break;
    case Idle_Headset_Record:
        degradegain = (uint32)MampUplinkGain(mGainTable.microphoneGain.micGain[GAIN_IDLE_RECORD_HEADSET]);
        break;
    case Fm_Record:
        degradegain = (uint32)MampUplinkGain(mGainTable.microphoneGain.micGain[GAIN_FM_RECORDING]);
        
        break;
    case Voice_Rec_Mic_Handset:
        degradegain = (uint32)MampUplinkGain(mGainTable.microphoneGain.micGain[GAIN_VOICE_RECOGNITION]);
        break;
    case VOIP_Normal_Mic:
        degradegain = (uint32)MampUplinkGain(mGainTable.microphoneGain.micGain[GAIN_VOIP_RECEIVER]);
        break;
    case VOIP_Headset_Mic:
        degradegain = (uint32)MampUplinkGain(mGainTable.microphoneGain.micGain[GAIN_VOIP_HEADSET]);
        break;
    case VOIP_Handfree_Mic:
        degradegain = (uint32)MampUplinkGain(mGainTable.microphoneGain.micGain[GAIN_VOIP_SPEAKER]);
        break;
    default:
        break;
}
#endif
    
    Pga_gain = MIC_GAIN_MAX -degradegain;
    if(Pga_gain > MIC_GAIN_MAX){
        Pga_gain = MIC_GAIN_MAX;
    }
    if(Pga_gain < MIC_GAIN_MIN){
        Pga_gain = MIC_GAIN_MIN;
    }
    ALOGD("SetApMicGain degradegain  = %d Pga_gain = %d,MicMode=%d",degradegain,Pga_gain,MicMode);
    mAudioHardware->mAfe_handle->HQA_AFE_Set_UL_ADC_Minima_PGA_Gain (MIC_GAIN_MIN);
    mAudioHardware->mAfe_handle->HQA_AFE_Set_UL_ADC_PGA_Gain (Pga_gain);
    return true;
}

#ifdef MTK_AP_SPEECH_ENHANCEMENT
int AudioYusuVolumeController::SetApMicGain_HDRecord(int MicMode, bool isDigitalMic)
{
    ALOGD("SetApMicGain_HDRecord MicMode = %d",MicMode);
    int mic_volume =0;
    uint32 pga_ul_gain = 0;
    int agc_ul_gain = 0;
    int i = 0;
            

    switch (MicMode){
        case Idle_Normal_Record:
            mic_volume = VolumeCustomParam.audiovolume_mic[VOL_NORMAL][4]>>2;
            break;
        case Idle_Headset_Record:
            mic_volume = VolumeCustomParam.audiovolume_mic[VOL_HEADSET][4]>>2;
            break;
        case Fm_Record:
            mic_volume = VolumeCustomParam.audiovolume_mic[VOL_HEADSET][5]>>2;
            break;
        case Voice_Rec_Mic_Handset:
            mic_volume = VolumeCustomParam.audiovolume_mic[VOL_NORMAL][5]>>2;
            break;
        case Voice_Rec_Mic_Headset:
            mic_volume = VolumeCustomParam.audiovolume_mic[VOL_HEADSET][4]>>2;
            break;    
        case VOIP_Normal_Mic:
            mic_volume = VolumeCustomParam.audiovolume_mic[VOL_NORMAL][6]>>2;
            break;
        case VOIP_Headset_Mic:
            mic_volume = VolumeCustomParam.audiovolume_mic[VOL_HEADSET][6]>>2;
            break;
        case VOIP_Handfree_Mic:
            mic_volume = VolumeCustomParam.audiovolume_mic[VOL_HANDFREE][6]>>2;
            break;
        case Idle_Video_Record_Handset:
            mic_volume = VolumeCustomParam.audiovolume_mic[VOL_NORMAL][2]>>2;
            break;
        case Idle_Video_Record_Headset:
            mic_volume = VolumeCustomParam.audiovolume_mic[VOL_HEADSET][2]>>2;
            break;  
        default:
            break;
                }
                     
    if(isDigitalMic)
    {
        if(mic_volume>HDRECORD_AP_TRANSPARENT_AGC_GAIN_OFFSET)
            agc_ul_gain = mic_volume - HDRECORD_AP_TRANSPARENT_AGC_GAIN_OFFSET;
        else
                agc_ul_gain = 0;
    }
            else
        {
            pga_ul_gain = HDRECORD_AP_MIN_PGA_GAIN;
            i = mic_volume - HDRECORD_AP_SW_AGC_GAIN_DEFAULT;
            i = i - pga_ul_gain;

            if(i>3)
            {
                for(; i > 3; i-=6)
                {
                    pga_ul_gain += 6;
                }
            }
            if(pga_ul_gain > HDRECORD_AP_MAX_PGA_GAIN)
                pga_ul_gain = HDRECORD_AP_MAX_PGA_GAIN;
                     
            if(mic_volume < pga_ul_gain)
                agc_ul_gain = 0;
            else
                agc_ul_gain = mic_volume - pga_ul_gain;

            ALOGD("SetApMicGain_HDRecord agc_ul_gain  = %d  Pga_gain = %d,mic_volume=%d",agc_ul_gain,pga_ul_gain,mic_volume);
            mAudioHardware->mAfe_handle->HQA_AFE_Set_UL_ADC_Minima_PGA_Gain (HDRECORD_AP_MIN_PGA_GAIN);
            mAudioHardware->mAfe_handle->HQA_AFE_Set_UL_ADC_PGA_Gain (pga_ul_gain);
    }

    return agc_ul_gain;
}
#endif

bool AudioYusuVolumeController::SetLevelShiftBufferGain(){
    int LevelShiftBufferGain = 0;
#ifndef MTK_AUDIO_GAIN_TABLE
    LevelShiftBufferGain =  mMicGain[Level_Shift_buffer]/3;
#else
    LevelShiftBufferGain =  mMicGain[Level_Shift_buffer];
#endif
    ALOGD("SetLevelShiftBufferGain = %d",LevelShiftBufferGain);
    mAudioHardware->mAnaReg->Afe_Set_AUD_Level_Shift_Buf_L_Gain (LevelShiftBufferGain);
    mAudioHardware->mAnaReg->Afe_Set_AUD_Level_Shift_Buf_R_Gain (LevelShiftBufferGain);
    return true;
}


bool AudioYusuVolumeController::SetSideTone(uint32 Devices, uint32 Gain)
{
    if(Gain == 0){
        Gain --;
    }
    LOG_VOLUMECONTROLLER("SetSideTone Mode=%d, Gain=%x",Devices,Gain);
    mSideTone[Devices] = Gain;
    return true;
}

bool AudioYusuVolumeController::SetMicGain(uint32 MicMode, uint32 Gain)
{
    if(MicMode >=Num_Mic_Gain)
    {
        LOG_VOLUMECONTROLLER("SetMicGain error");
        return false;
    }
    if(Gain ==0)
        Gain--;
    LOG_VOLUMECONTROLLER("SetMicGain MicMode=%d, Gain=%x",MicMode,Gain);
    mMicGain[MicMode] = Gain;
    return true;
}

bool AudioYusuVolumeController::SetLevelShiftBufferGain(uint32 MicMode, uint32 Gain)
{
    if(MicMode !=Level_Shift_buffer)
    {
        LOG_VOLUMECONTROLLER("SetLevelShiftBufferGain error");
        return false;
    }
    LOG_VOLUMECONTROLLER("SetLevelShiftBufferGain MicMode=%d, Gain=%x",MicMode,Gain);
    mMicGain[Level_Shift_buffer] = Gain;
    return true;
}

uint32 AudioYusuVolumeController::GetMicGain(uint32 MicMode)
{
    if(MicMode >=Num_Mic_Gain)
    {
        LOG_VOLUMECONTROLLER("GetMicGain error");
    }
    return  mMicGain[MicMode];
}

bool AudioYusuVolumeController::setMasterVolume(float volume, uint32 mode,uint32 routes)  // volume should range 0.0 ~ 1.0
{
    int MapVolume;
    LOG_VOLUMECONTROLLER("setMasterVolume mode=%d, routes=%d",mode,routes);
    mVolume = volume;
    //MapVolume = AudioMapVolume(volume); // map volume to 0xffff degree
    MapVolume = AudioSystem::logToLinear (volume);
    //LOG_VOLUMECONTROLLER("setMasterVolume MapVolume=%x",MapVolume);

    switch(mode)
    {
    case (android_audio_legacy::AudioSystem::MODE_NORMAL) : // normal mode
        {
            switch(routes)
            {
            case(android_audio_legacy::AudioSystem::DEVICE_OUT_WIRED_HEADSET):{
                ApplyGain(MapVolume,  mode, routes);
                break;
            }
            case (android_audio_legacy::AudioSystem::DEVICE_OUT_WIRED_HEADPHONE):{
                ApplyGain(MapVolume,  mode, routes);
                break;
            }
            case (android_audio_legacy::AudioSystem::DEVICE_OUT_BLUETOOTH_SCO):{
                uint32 Lad_Volume = Calgain(MapVolume,mode,routes);
                LOG_VOLUMECONTROLLER("setMasterVolume Lad_Volume=%ul ",Lad_Volume);
                mAudioHardware->pLad->LAD_SetOutputVolume(Lad_Volume);
                break;
            }
            case (android_audio_legacy::AudioSystem::DEVICE_OUT_SPEAKER) :{
                ApplyGain(MapVolume,  mode, routes);
                break;
            }
            case (android_audio_legacy::AudioSystem::DEVICE_OUT_EARPIECE):{
                ApplyHandsetGain(MapVolume,  mode, routes);
                break;
            }
            default:{
                LOG_VOLUMECONTROLLER(":setMasterVolume default");
                break;
                    }
            }
            break;
        }
    case (android_audio_legacy::AudioSystem::MODE_RINGTONE): // ringtone mode
        {
            switch(routes)
            {
            case (android_audio_legacy::AudioSystem::DEVICE_OUT_WIRED_HEADSET) :{
                ApplyGain(MapVolume,  mode, routes);
                break;
                }
                case (android_audio_legacy::AudioSystem::DEVICE_OUT_WIRED_HEADPHONE):{
                    ApplyGain(MapVolume,  mode, routes);
                    break;
                }
            case (android_audio_legacy::AudioSystem::DEVICE_OUT_BLUETOOTH_SCO) :{
                uint32 Lad_Volume = Calgain(MapVolume,mode,routes);
                LOG_VOLUMECONTROLLER("setMasterVolume  Lad_Volume=%x",Lad_Volume) ;
                mAudioHardware->pLad->LAD_SetOutputVolume(Lad_Volume);
                break;
                }
            case (android_audio_legacy::AudioSystem::DEVICE_OUT_SPEAKER) :{
                ApplyGain(MapVolume,  mode, routes);
                break;
                }
            case (android_audio_legacy::AudioSystem::DEVICE_OUT_EARPIECE) :{
                break;
                }
            default:{
                LOG_VOLUMECONTROLLER("setMasterVolume default");
                break;
                }
            }
            break;
        }
    case (android_audio_legacy::AudioSystem::MODE_IN_CALL):  // in call mode
        {
            float VoiceVolume = getVoiceVolume();
            //MapVolume = AudioMapVolume(VoiceVolume); // remap volume to 0xffff degree
            LOG_VOLUMECONTROLLER("setMasterVolume, Ic-Call voice MapVolume=%x",MapVolume);
            switch(routes)
            {
            case (android_audio_legacy::AudioSystem::DEVICE_OUT_WIRED_HEADSET) :{
#ifdef ALL_USING_VOICEBUFFER_INCALL
                uint32 Lad_Volume = Calgain(MapVolume,mode,routes);
                ALOGD("setMasterVolume, Lad_Volume = %x",Lad_Volume) ;
                mAudioHardware->pLad->LAD_SetOutputVolume(Lad_Volume);
#else
                ApplyGain(MapVolume,  mode, routes);
#endif
                break;
                }
            case (android_audio_legacy::AudioSystem::DEVICE_OUT_WIRED_HEADPHONE) :{
#ifdef ALL_USING_VOICEBUFFER_INCALL
                uint32 Lad_Volume = Calgain(MapVolume,mode,routes);
                ALOGD("setMasterVolume, Lad_Volume = %x",Lad_Volume) ;
                mAudioHardware->pLad->LAD_SetOutputVolume(Lad_Volume);
#else
                ApplyGain(MapVolume,  mode, routes);
#endif
                break;
                }
            case (android_audio_legacy::AudioSystem::DEVICE_OUT_BLUETOOTH_SCO) :{
                uint32 Lad_Volume = Calgain(MapVolume,mode,routes);
                LOG_VOLUMECONTROLLER("setMasterVolume  Lad_Volume = %x",Lad_Volume) ;
                mAudioHardware->pLad->LAD_SetOutputVolume(Lad_Volume);
                break;
                }
            case (android_audio_legacy::AudioSystem::DEVICE_OUT_SPEAKER) :{
#ifdef ALL_USING_VOICEBUFFER_INCALL
                uint32 Lad_Volume = Calgain(MapVolume,mode,routes);
                ALOGD("setMasterVolume, Lad_Volume = %x",Lad_Volume) ;
                mAudioHardware->pLad->LAD_SetOutputVolume(Lad_Volume);
#else
                ApplyGain(MapVolume,  mode, routes);
#endif
                break;
                }
            case (android_audio_legacy::AudioSystem::DEVICE_OUT_EARPIECE) :{
                    uint32 Lad_Volume = Calgain(MapVolume,mode,routes);
                    ALOGD("setMasterVolume, Lad_Volume = %x",Lad_Volume) ;
                    mAudioHardware->pLad->LAD_SetOutputVolume(Lad_Volume);
                    break;
                }
            default:{
                LOG_VOLUMECONTROLLER("setMasterVolume default");
                break;
                }
            }
            break;
        }
        case (android_audio_legacy::AudioSystem::MODE_IN_COMMUNICATION):  // in call mode
        {
            float VoiceVolume = getVoiceVolume();
            //MapVolume = AudioMapVolume(VoiceVolume); // remap volume to 0xffff degree
            LOG_VOLUMECONTROLLER("setMasterVolume, Ic-Call voice MapVolume=%x",MapVolume);
            switch(routes)
            {
            case (android_audio_legacy::AudioSystem::DEVICE_OUT_WIRED_HEADSET) :{
                ApplyGain(MapVolume,  mode, routes);
                break;
                }
            case (android_audio_legacy::AudioSystem::DEVICE_OUT_BLUETOOTH_SCO) :{
                uint32 Lad_Volume = Calgain(MapVolume,mode,routes);
                LOG_VOLUMECONTROLLER("setMasterVolume  Lad_Volume = %x",Lad_Volume) ;
                mAudioHardware->pLad->LAD_SetOutputVolume(Lad_Volume);
                break;
                }
            case (android_audio_legacy::AudioSystem::DEVICE_OUT_SPEAKER) :{
                ApplyGain(MapVolume,  mode, routes);
                break;
                }
            case (android_audio_legacy::AudioSystem::DEVICE_OUT_EARPIECE) :{
                ApplyHandsetGain(MapVolume, mode, routes);
                break;
                }
            default:{
                LOG_VOLUMECONTROLLER("setMasterVolume default");
                break;
                }
            }
            break;
        }

    }

    return true;
}

float AudioYusuVolumeController::getMasterVolume(void)
{
    return mVolume;
}

float AudioYusuVolumeController::getVoiceVolume(void)
{
    return mVoiceVolume;
}

bool AudioYusuVolumeController::ModeSetVoiceVolume(void)
{
    int mode =0;
    mAudioHardware->getMode (&mode);
    if(mode == android_audio_legacy::AudioSystem::MODE_IN_CALL || mode == android_audio_legacy::AudioSystem::MODE_IN_COMMUNICATION){
        return true;
    }
    else{
        return false;
    }
}

bool AudioYusuVolumeController::setVoiceVolume(float volume, uint32 mode,uint32 routes)  // volume should range 0.0 ~ 1.0
{
    mVoiceVolume = volume;
    uint32 Lad_Volume=0;
    int MapVolume;
    // when mode is not incall , save voice volume and return
    if(ModeSetVoiceVolume() == false){
        return true;
    }

    int32 GetDevice = AudioGetRoute(routes);
    LOG_VOLUMECONTROLLER("setVoiceVolume volume=%f, mode=%d, routes=%x, GetDevice=%d volume in 256 = %d",
    volume,mode,routes,GetDevice,AudioSystem::logToLinear (volume)) ;
    //MapVolume = AudioMapVolume(volume); // map volume to 0xffff degree
    MapVolume = AudioSystem::logToLinear (volume);

    if(GetDevice == ROUTE_OUT_EARPIECE)
    {
    	Lad_Volume= CalVoicegain(MapVolume,mode,routes); // calculate in_call mode volume
        LOG_VOLUMECONTROLLER("setVoiceVolume Lad_Volume=%x, mSideTone[GetDevice]=%x",Lad_Volume,mSideTone[GetDevice]) ;
        mAudioHardware->pLad->LAD_SetOutputVolume(Lad_Volume);
        mAudioHardware->pLad->LAD_SetSidetoneVolume(mSideTone[GetDevice]);
        return true;
    }
    else if( GetDevice == ROUTE_OUT_SPEAKER ||  GetDevice == ROUTE_OUT_WIRED_HEADSET
        ||GetDevice == ROUTE_OUT_WIRED_HEADPHONE)  // adjust for analog gain
    {
#ifdef ALL_USING_VOICEBUFFER_INCALL
        Lad_Volume= CalVoicegain(MapVolume,mode,routes); // calculate in_call mode volume
        LOG_VOLUMECONTROLLER("setVoiceVolume Lad_Volume=%x, mSideTone[GetDevice]=%x",Lad_Volume,mSideTone[GetDevice]) ;
        mAudioHardware->pLad->LAD_SetOutputVolume(Lad_Volume);
        mAudioHardware->pLad->LAD_SetSidetoneVolume(mSideTone[GetDevice]);
#else

#if defined(MTK_DT_SUPPORT)
        if (mAudioHardware->GetFlag_SecondModemPhoneCall()!=0){
            Lad_Volume= 0xffffffEC; //-20 dB in MT6252
        }else{
            Lad_Volume= 0xffffffE8; //  0 dB in MT6575
        }
        mAudioHardware->pLad->LAD_SetOutputVolume(Lad_Volume);
#else
        mAudioHardware->pLad->LAD_SetOutputVolume(0xffffffE8);
#endif
        //ALOGD("setVoiceVolume sidetone:%x [...ApplyGainInCall]",mSideTone[GetDevice]);
        ApplyGainInCall(MapVolume, mode, routes);
        mAudioHardware->pLad->LAD_SetSidetoneVolume(mSideTone[GetDevice]);
#endif
        return true;
    }
    else if( GetDevice == ROUTE_OUT_BLUETOOTH_SCO ||  GetDevice == ROUTE_OUT_BLUETOOTH_SCO_HEADSET
        ||GetDevice == ROUTE_OUT_BLUETOOTH_SCO_CARKIT)
    {
        ALOGW("setVoiceVolume with ROUTE_OUT_BLUETOOTH_SCO");
        return true;
    }
    else
    {
        ALOGW("setVoiceVolume with no default");
        return true;
    }
    return false;
}

bool AudioYusuVolumeController::setVoIPVolume(float volume, uint32 mode,uint32 routes)  // volume should range 0.0 ~ 1.0
{
    mVoiceVolume = volume;
    uint32 Lad_Volume=0;
    uint16 MapVolume;
    // when mode is not incall , save voice volume and return
    if(ModeSetVoiceVolume() == false){
        return true;
    }
    // when in communication mode , set to max volume and adjust digital
    volume = 1.0;

    int32 GetDevice = AudioGetRoute(routes);
    LOG_VOLUMECONTROLLER("setVoIPVolume volume=%f, mode=%d, routes=%x, GetDevice=%d",volume,mode,routes,GetDevice) ;
    //MapVolume = AudioMapVolume(volume); // map volume to 0xffff degree
    MapVolume = AudioSystem::logToLinear (volume);

    if(GetDevice == ROUTE_OUT_EARPIECE){
        ApplyHandsetGain(MapVolume, mode, routes);
        return true;
    }
    if(GetDevice == ROUTE_OUT_SPEAKER ||    GetDevice == ROUTE_OUT_WIRED_HEADSET
        ||GetDevice == ROUTE_OUT_WIRED_HEADPHONE)  // adjust for analog gain
    {
        ApplyGain(MapVolume, mode, routes);
        return true;
    }
    else if( GetDevice == ROUTE_OUT_BLUETOOTH_SCO ||  GetDevice == ROUTE_OUT_BLUETOOTH_SCO_HEADSET
        ||GetDevice == ROUTE_OUT_BLUETOOTH_SCO_CARKIT)
    {
        ALOGW("setVoIPVolume with ROUTE_OUT_BLUETOOTH_SCO");
        return true;
    }
    else
    {
        ALOGW("setVoIPVolume with no default");
        return true;
    }
    return false;
}


bool AudioYusuVolumeController::SetFmVolume(int volume)
{
   int ret=0;
   ALOGD("!!SetFmVolume=%d, mfmFd=%d",volume,mfmFd);

   //when fm is dissable , just save volume and return
   if(mAudioHardware->GetFmRxStatus () == false){
      mFmVolume = volume;
      return true;
   }

// if use MT519x FM, don't cotnrol the MT66xx FM volume
#if defined(MT5192_FM) || defined(MT5193_FM)
// ToDo
   ALOGD("!!MT519x, SetFmVolume(%d)",volume);
   if(spATVCtrlService == NULL){
      ALOGD("SetFmVolume, spATVCtrlService(NULL)");
      GetMatvService ();
      if(spATVCtrlService == NULL){
         ALOGD("SetFmVolume, fail mATV service");
         return false;
      }
   }
   spATVCtrlService->ATVCS_matv_adjust(MATV_AUD_VOLUME, volume);
#else
   if(volume < 0)
      return true;
   else if (volume >=15)
      volume = 15;

   ALOGD("!!MT66xx, SetFmVolume(%d)",volume);
   //open fm driver for adjust volume
   HandleFmFd (true);
   if(mfmFd > 0 )
   {
      mFmMute = (volume == 0)?true:false;
      ret = ::ioctl(mfmFd,FM_IOCTL_SETVOL,(uint32_t*)&volume);
   }
   HandleFmFd(false);
#endif

   mFmVolume = volume;
   return true;
}

#ifdef MATV_AUDIO_LINEIN_PATH
void AudioYusuVolumeController::GetMatvService()
{
     sp<IServiceManager> sm = defaultServiceManager();
     sp<IBinder> binder;
     do{
        binder = sm->getService(String16("media.ATVCtrlService"));
        if (binder != 0)
           break;
        ALOGW("ATVCtrlService not published, waiting...");
        usleep(1000*1000); // 1 s
     } while(true);
     spATVCtrlService = interface_cast<IATVCtrlService>(binder);
}
#endif

bool AudioYusuVolumeController::setMatvVolume(int volume)
{
    ALOGD("setMatvVolume volume=%d",volume);

#ifdef MATV_AUDIO_LINEIN_PATH
    if(spATVCtrlService == NULL){
        ALOGW("setMatvVolume but spATVCtrlService == NULL");
        GetMatvService ();
        if(spATVCtrlService == NULL){
            ALOGE("setMatvVolume cannot get matv service");
            return false;
        }
    }

    if(mMatvMute == false)
      spATVCtrlService->ATVCS_matv_adjust(MATV_AUD_VOLUME, volume);
#endif

    mMatvVolume = volume;
    return true;
}

int AudioYusuVolumeController::GetMatvVolume(void)
{
    return mMatvVolume;
}


int AudioYusuVolumeController::GetFmVolume()
{
    //ALOGD("!!GetFmVolume=%d",mFmVolume);
    return mFmVolume;
}

bool AudioYusuVolumeController::SetFmMute(bool Enable)
{
    int ret;
    if(mfmFd > 0)
    {
        ALOGD("SetFmMute = %d",Enable);
        ret = ioctl(mfmFd,FM_IOCTL_MUTE,Enable);
    }
    return true;
}

bool AudioYusuVolumeController::SetMatvMute(bool b_mute)
{
   ALOGD("SetMatvMute(%d), mMatvVolume(%d)",b_mute,mMatvVolume);

#ifdef MATV_AUDIO_LINEIN_PATH
   if(spATVCtrlService == NULL){
     ALOGW("SetMatvMute, can't get spATVCtrlService");
     GetMatvService ();
     if(spATVCtrlService == NULL){
         ALOGE("SetMatvMute cannot get matv service");
         return false;
     }
   }

   if(b_mute == true)
   {
      mMatvMute = true;
      spATVCtrlService->ATVCS_matv_adjust(MATV_AUD_VOLUME, 0);
   }
   else
   {
      mMatvMute = false;
      spATVCtrlService->ATVCS_matv_adjust(MATV_AUD_VOLUME, mMatvVolume);
   }
#endif

   return true;
}

 // ---------------------------------------------------------------------------
}
