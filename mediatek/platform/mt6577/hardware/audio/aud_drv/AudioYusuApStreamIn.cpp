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

#define LOG_TAG "AudioYusuApStreamIn"
#include <utils/Log.h>
#include <utils/String8.h>

#include "AudioYusuHardware.h"
#include "AudioAfe.h"
#include "AudioAnalogAfe.h"
#include "AudioYusuStreamHandler.h"
#include "AudioYusuVolumeController.h"
#include "AudioYusuApStreamIn.h"
#include "AudioYusuStreamInInterface.h"
#include "AudioAnalogAfe.h"
#include "audio_custom_exp.h"
#include <assert.h>
#include <cutils/properties.h>
#include <media/mediarecorder.h>

#ifdef MTK_AP_SPEECH_ENHANCEMENT
#include "AudioCustParam.h"
#endif

//#ifdef ENABLE_LOG_APSTREAMIN
#if 1
#undef ALOGV
#define ALOGV(...) ALOGD(__VA_ARGS__)
#endif

namespace android {

static const unsigned short UNITY_GAIN = 0x0400; // 6.10 for 36dB";
static const int VolumeShiftBits = 10;
static const int UNITY_GAIN_Index = 35;

// step for 36dB , 1db per step
static const unsigned short FM_Map_gain[] =
{
    0xe0ef,
    0xc879,
    0xb2ac,
    0x9f3e,
    0x8dec,
    0x7e7d,
    0x70bc,
    0x6479,
    0x598c,
    0x4fcf,
    0x4721,
    0x3f65,
    0x3880,
    0x325b,
    0x2ce1,
    0x2800,
    0x23a6,
    0x1fc5,
    0x1c51,
    0x193d,
    0x167e,
    0x140c,
    0x11de,
    0xfec,
    0xe31,
    0xca6,
    0xb46,
    0xa0c,
    0x8f4,
    0x7fb,
    0x71c,
    0x656,
    0x5a6,
    0x509,
    0x47c,
    0x400,
};

#ifdef MTK_AP_SPEECH_ENHANCEMENT
static const unsigned long HDRecordEnhanceParasCommon[]=
{
    0,
    0,
    0,
    10752,
    32769,
    0,
    0,
    0,
    0,
    0,
    0,
    0
};
#endif

static short clamp16(int sample)
{
    if ((sample>>15) ^ (sample>>31))
        sample = 0x7FFF ^ (sample>>31);
    return sample;
}

// apply volume with byes in count
int AudioYusuApStreamIn::StreamInApplyVolume(short *pcm , int count,unsigned short volume)
{
    //ALOGD("StreamInApplyVolume count = %d volume = %x",count,volume);
    float Volume_inverse = 0.0;
    bool volume_change = false;
    int consumebytes = 0;
    count = count >> 1;
    if(mFm_Volume != volume){  // volume change
        Volume_inverse = (float)(volume-mFm_Volume)/count;
        volume_change = true;
    }
    short *pPcm = pcm;
    while(count){
        int value = 0;
        if(volume_change){
            value = *pPcm * (mFm_Volume+(Volume_inverse*consumebytes));
        }
        else{
            value = *pPcm * volume;
        }
        *pPcm = clamp16(value>>VolumeShiftBits);
        pPcm++;
        count--;
        consumebytes++;
    }
    mFm_Volume = volume;
    return consumebytes<<1;
}


status_t AudioYusuApStreamIn::set(
        AudioYusuHardware *hw,
	int fd,
	uint32_t devices,
	int *pFormat,
	uint32_t *pChannels,
	uint32_t *pRate,
	android_audio_legacy::AudioSystem::audio_in_acoustics acoustics)
{
    Mutex::Autolock lock(mLock);
    bool ret = true;
    ALOGD("AudioYusuApStreamIn::set(%p, %d, %d, %d, %u)", hw, fd, *pFormat, *pChannels, *pRate);
    // check values


    ALOGD("+set(%p, %d, %d, %d, %u),devices=%d,mInputSource=%d", hw, fd, *pFormat, *pChannels, *pRate,devices,mInputSource);

#ifdef MTK_AP_SPEECH_ENHANCEMENT
    if (*pFormat != android_audio_legacy::AudioSystem::PCM_16_BIT|| *pChannels != channels() ||(*pRate != 48000)) {
        YAD_LOGE("Error opening input channel");
        goto SET_EXIT;
    }
#else
    if (*pFormat != android_audio_legacy::AudioSystem::PCM_16_BIT|| *pChannels != channels() ||*pRate != 16000) {
        YAD_LOGE("Error opening input channel");
        goto SET_EXIT;
    }
#endif

    /* set parameters */
    mSampleRate = *pRate;
    if(!SupportedSampleRate(mSampleRate)){
        mApSampleRate = GetRecordSampleRate(*pRate);
    }
    else{
        mApSampleRate = mSampleRate;
    }
    mFormat = *pFormat;
    if(*pChannels == android_audio_legacy::AudioSystem::CHANNEL_IN_MONO){
        mChNum = 1;
    }
    else{
        mChNum = 2;
    }
    mFd = ::open(kAudioDeviceName, O_RDWR);
    mHw = hw;
    mReadBufferSize = (*pRate>>4)&0xfffffff0;
    mDevice =devices;
    if(mInputSource  == AUDIO_SOURCE_VOICE_RECOGNITION){
        mMutecount =(MUTE_APAUDIO_PCM_MS*mSampleRate*mChNum<<1)/1000;
    }
    else{
        mMutecount =(MUTE_APAUDIO_PCM_MS_NORMAL*mSampleRate*mChNum<<1)/1000;

}
    ALOGD("mFd = %d mHw = %p mReadBufferSize = %d mMutecount = %d mChNum = %d mSampleRate = %d",
    mFd,mHw,mReadBufferSize,mMutecount,mChNum,mSampleRate);

    // ADC and AWB record setting
    mHw->mAfe_handle->Afe_Set_Stream_Attribute(mFormat, *pChannels,mApSampleRate,AFE_MODE_ADC);
    mHw->mAfe_handle->Afe_Set_Stream_Attribute(mFormat, *pChannels,mApSampleRate,AFE_MODE_AWB);
    #ifdef DIGITAL_MIC
    SetDigitalMic();
    #endif

    if(mApSampleRate != mSampleRate){
        uint32 srcBufLen;

        pSrcReadSize = (mSampleRate * mReadBufferSize / mApSampleRate)&0xfffffff0;
        pInputbuflen = (pSrcReadSize<<1);
        pInputSrcWrite =0;
        pInputSrcRead =0;
        ALOGD("mApSampleRate=%d mSampleRate=%d need src srcBufSize = %d pSrcReadSize = %d",
            mApSampleRate,mSampleRate,pInputbuflen,pSrcReadSize);
        pInputbuf = new int8[pInputbuflen];
        BLI_GetMemSize( mApSampleRate, mChNum, mSampleRate, mChNum, &srcBufLen);
        pSrcBuf = new int8[srcBufLen];
        pSrcHdl = BLI_Open( mApSampleRate, mChNum, mSampleRate, mChNum, pSrcBuf);
        if ( !pSrcHdl ) {
            return false;
        }
    }
    mFm_Volume = UNITY_GAIN;
    mFm_Vlume_Index = UNITY_GAIN_Index;

#ifdef MTK_DUAL_MIC_SUPPORT
    mbLRChannelSwitch = mHw->mbInLRSwitch;
    mHw->mbInLRSwitch = false;

    miSpecificMicChoose = mHw->miUseSpecMIC;
    mHw->miUseSpecMIC = 0;
#endif
    
#ifdef MTK_AP_SPEECH_ENHANCEMENT
    if(mHw && !CanRecordFM())
    {        
        LoadHdRecordParams();        
        mHw->mpSPELayer->SetSampleRate(SPE_MODE_REC, mApSampleRate);

#ifdef  MTK_AP_SPEECH_ENHANCEMENT_VOIP        
        if(mHw->GetCommunicationStatus())
            mHw->mpSPELayer->SetSampleRate(SPE_MODE_VOIP, mApSampleRate);   //VoIP only support 16K samplerate
#endif            
        //mHw->mpSPELayer->SetChannel(mChNum);
     }
#endif     
    return NO_ERROR;

SET_EXIT:
    // modify default paramters and let Audiorecord openagina for reampler.
   *pFormat = android_audio_legacy::AudioSystem::PCM_16_BIT;
   *pChannels =  channels();
   *pRate = GetRecordSampleRate(*pRate);
    return BAD_VALUE;
}

#ifdef MTK_AP_SPEECH_ENHANCEMENT
uint32 AudioYusuApStreamIn::GetRecordSampleRate(uint32_t pRate, AudioYusuHardware *hw)
{
    uint32 retSamplerate = 48000;
    if(hw)
    {
#ifdef MTK_AP_SPEECH_ENHANCEMENT_VOIP
        if(hw->GetCommunicationStatus())    //communication status, use 16K sample rate
        {
            retSamplerate = 16000;
        }
#endif        
    }
    else
    {
        retSamplerate = 48000;
    }
    

    ALOGD("GetRecordSampleRate,pRate=%d, use %d",pRate,retSamplerate);
    return retSamplerate;
 
}
#else
uint32 AudioYusuApStreamIn::GetRecordSampleRate(uint32_t pRate)
{
    return 16000;
}

#endif

bool AudioYusuApStreamIn::SupportedSampleRate(uint32 SampleRate)
{
    if(SampleRate == 8000 || SampleRate == 16000 || SampleRate == 32000 || SampleRate == 48000){
        return true;
    }
    else{
       return false;
    }
}

void AudioYusuApStreamIn::getDataFromModem(void)
{
    usleep(30*1000);
    ALOGW("AP should not get data from modem.");
}


void AudioYusuApStreamIn::getVoiceRecordDataFromExtModem(uint8 *pBuf, int s4Size)
{
    usleep(30*1000);
    ALOGW("AP should not get data from ext modem.");
}

uint32_t AudioYusuApStreamIn::sampleRate() const
{
   if(mSampleRate)
       return mSampleRate;
   else
       return 16000;
}

uint32_t AudioYusuApStreamIn::channels() const
{
    return android_audio_legacy::AudioSystem::CHANNEL_IN_STEREO;
}

AudioYusuApStreamIn::AudioYusuApStreamIn()
{
    ALOGV("AudioYusuApStreamIn Constructor");
    int ret =0;

    ret = pthread_mutex_init(&mReadMutex, NULL);
    if ( ret != 0 )
        ALOGE("Failed to initialize  mReadMutex");
    mHw = NULL;
    mFd = 0;
    mSampleRate = 0;
    mInputSource =0;
    mReadBufferSize = 1024;
    mstandby = true;
    mChNum =0;
    pSrcBuf = NULL;
    pSrcHdl = NULL;
    pInputbuf = NULL;
    // here to maaping FM volume in DB
    #if 1 //fix compile error
	FM_Mapping_table[0]=8;
	FM_Mapping_table[1]=8;
	FM_Mapping_table[2]=14;
	FM_Mapping_table[3]=18;
	FM_Mapping_table[4]=21;
	FM_Mapping_table[5]=23;
	FM_Mapping_table[6]=25;
	FM_Mapping_table[7]=27;
	FM_Mapping_table[8]=28;
	FM_Mapping_table[9]=29;
	FM_Mapping_table[10]=30;
	FM_Mapping_table[11]=31;
	FM_Mapping_table[12]=32;
	FM_Mapping_table[13]=33;
	FM_Mapping_table[14]=34;
	FM_Mapping_table[15]=35;
	#else
    FM_Mapping_table = {8,8,14,18,21,23,25,27,28,29,30,31,32,33,34,35};
	#endif
	
	#ifdef MTK_AP_SPEECH_ENHANCEMENT
    mStartReadTestFile = false;
    mTestFileLength = 0;
    mTestFileSizeRead = 0;
    mHDRecordModeIndex = -1;
	#endif    
	mNeedStereo = false;
	mfp = NULL;

#ifdef MTK_DUAL_MIC_SUPPORT    
        mbLRChannelSwitch = false;
        miSpecificMicChoose = 0;
#endif  
}

status_t  AudioYusuApStreamIn::standby()
{
    ALOGV("AudioYusuApStreamIn standby");
    Mutex::Autolock lock(mLock);
    if(mstandby == false){
        RecClose ();
        mstandby = true;
        mMutecount =(MUTE_APAUDIO_PCM_MS*mSampleRate*mChNum<<1)/1000;
    }
    usleep(1*1000);
    return NO_ERROR;
}


AudioYusuApStreamIn::~AudioYusuApStreamIn()
{
    Mutex::Autolock lock(mLock);
    #ifdef DUMP_AP_STREAMIN
    if(pOutFile){
        fclose(pOutFile);
        pOutFile = NULL;
    }
    #endif
    if(mstandby == false){
        RecClose();
    }
    if(pSrcHdl!= NULL){
        BLI_Close(pSrcHdl);
        pSrcHdl = NULL;
    }
    if(pSrcBuf!=NULL){
        delete[] pSrcBuf;
        pSrcBuf = NULL;
    }
    if(mFd ){
        close(mFd);
        mFd = 0;
    }
    if(mfp)
    {
		fclose(mfp);
		mfp = NULL;
    }
#ifdef MTK_AP_SPEECH_ENHANCEMENT
    if(mHw)
    {
        ALOGD("~AudioYusuApStreamIn stop SPElayver");
        mHw->mpSPELayer->Stop();
        mHw->mHdRecScene = -1; //reset record scene
    }
    else
        ALOGD("~AudioYusuApStreamIn no HW handle ");

    mStartReadTestFile = false;
    mTestFileLength = 0;
    mTestFileSizeRead = 0;   
#endif    
    ALOGV("~AudioYusuApStreamIn");
}

uint32 AudioYusuApStreamIn::GetSrcbufvalidSize()
{
    if(pInputbuf != NULL){
        if(pInputSrcWrite >=pInputSrcRead){
            return pInputSrcWrite-pInputSrcRead;
        }
        else{
            return pInputSrcRead+pInputbuflen -pInputSrcWrite;
        }
    }
    ALOGW("pInputbuf == NULL");
    return 0;
}

uint32 AudioYusuApStreamIn::GetSrcbufFreeSize()
{
    if(pInputbuf != NULL){
        return pInputbuflen - GetSrcbufvalidSize();
    }
    ALOGW("pInputbuf == NULL");
    return 0;
}

// here copy SRCbuf to input buffer , and return how many buffer copied
uint32 AudioYusuApStreamIn::CopySrcBuf(char *buffer,uint32 *bytes, char *SrcBuf , uint32 *length)
{
    uint32 consume =0;
    uint32 outputbyes = *bytes;
    //ALOGD("+CopySrcBuf consume = %d bytes = %d length = %d",consume,*bytes,*length);
    consume = BLI_Convert(pSrcHdl,(short*)SrcBuf,length,(short*)buffer,bytes);
    //ALOGD("-CopySrcBuf consume = %d bytes = %d length = %d",consume,*bytes,*length);
    pInputSrcRead += consume;
    if(pInputSrcRead >= pInputbuflen){
        //ALOGD("pInputSrcRead = %d pInputbuflen = %d",pInputSrcRead,pInputbuflen);
        pInputSrcRead -= pInputbuflen;
    }
    *bytes = outputbyes -*bytes;
    return consume;
}

void AudioYusuApStreamIn::StereoToMono(short *buffer , int length)
{
    short left , right;
    int sum;
    int copysize = length>>2;

    while(copysize){
        left = *(buffer);
        right = *(buffer+1);
        sum = (left+right) >> 1;
        *(buffer) = (short)sum;
        *(buffer+1) = (short)sum;
        buffer+=2;
        copysize--;
    }

}

void AudioYusuApStreamIn::CheckNeedMute(void* buffer, int bytes)
{
    if(mMutecount > 0){
        memset(buffer,0,bytes);
        mMutecount -=bytes;
    }
}
static int checkAndCreateDirectory(const char * pC)
{
	char tmp[PATH_MAX];
	int i = 0;

	while(*pC)
	{
		tmp[i] = *pC;

		if(*pC == '/' && i)
		{
			tmp[i] = '\0';
			if(access(tmp, F_OK) != 0)
			{
				if(mkdir(tmp, 0770) == -1)
				{
					ALOGE("AudioDumpPCM: mkdir error! %s\n",(char*)strerror(errno));
					return -1;
				}
			}
			tmp[i] = '/';
		}
		i++;
		pC++;
	}
	return 0;
}
void AudioYusuApStreamIn::Dump_StreamIn_PCM(void* buffer, int bytes)
{
   //set this property to dump data
   char value[PROPERTY_VALUE_MAX];
   int ret;
   property_get("streamin.pcm.dump", value, "0");
   int bflag=atoi(value);
   if(bflag)
   {
      if(mfp==NULL)
      {
		char filename[]="/sdcard/mtklog/audio_dump/StreamIn_Dump.pcm";    		
		char value[PROPERTY_VALUE_MAX];
   		property_get("streamin.pcm.dump", value, "0");
   		bflag=atoi(value);
   		if(bflag)
   		{        
		  ret = checkAndCreateDirectory(filename);
		  if(ret<0)
		  {
			  ALOGE("AudioYusuApStreamIn Dump_StreamIn_PCM checkAndCreateDirectory() fail!!!");
		  }
		  else
		  {
      		mfp= fopen(filename, "ab+");
      		if(mfp==NULL)      		
      		{
        		ALOGD("AudioYusuApStreamIn open StreamIn_Dump.pcm fail"); 
      		}
		  }
   		}
      }
      
      if(mfp!=NULL)
      {
         fwrite(buffer,bytes,1,mfp);         
      }
      else
      {
         ALOGE("no mfp");
      }
   }
}

#ifdef MTK_AP_SPEECH_ENHANCEMENT
void AudioYusuApStreamIn::Parse_Test_STEREO_PCM(void* buffer, int bytes)
{
    //set this property to parse data
    char value[PROPERTY_VALUE_MAX];
    property_get("SPEInputTest.pcm", value, "0");
    int bflag=atoi(value);
    if(bflag)
    {
        char filename[]="/sdcard/inputtest.pcm";
        FILE * fp= fopen(filename, "rb+");
        if(fp!=NULL)
        {
            if(!mStartReadTestFile)
            {            
                mStartReadTestFile = true;
                mTestFileSizeRead = 0;
                fseek(fp,0,SEEK_END);
                mTestFileLength = ftell(fp);        
            //ALOGE("Parse_Test_STEREO_PCM, mTestFileLength=%d, mTestFileSizeRead=%d",mTestFileLength,mTestFileSizeRead);
            }

            if(mTestFileSizeRead<=mTestFileLength)
            {
            
                ALOGE("mTestFileLength=%d, mTestFileSizeRead=%d",mTestFileLength,mTestFileSizeRead);
                fseek(fp,mTestFileSizeRead,SEEK_SET);
                if(mTestFileSizeRead + bytes<=mTestFileLength)
                {
                    //int readbytes = fread(buffer,sizeof(short),bytes>>1,fp);
                    int readbytes = fread(buffer,1,bytes,fp);
                    mTestFileSizeRead+=bytes;
                    //ALOGE("readbytes = %d",readbytes);
                }
                else
                {
                    fread(buffer,1,mTestFileLength-mTestFileSizeRead,fp);
                    fseek(fp,0,SEEK_SET);
                    fread(buffer+mTestFileLength-mTestFileSizeRead,1,bytes-(mTestFileLength-mTestFileSizeRead),fp);
                    //memset(buffer+mTestFileLength-mTestFileSizeRead,0,bytes-(mTestFileLength-mTestFileSizeRead));
                    mTestFileSizeRead=bytes-(mTestFileLength-mTestFileSizeRead);
                            ALOGE("at the end of inputtest.pcm");
                    //mStartReadTestFile = false;
                }
                fclose(fp);
            }
            else    //parse run outoff pcm data
            {
                mStartReadTestFile = false;
                mTestFileLength = 0;
                mTestFileSizeRead = 0;
                memcpy(buffer,0,bytes);
                fclose(fp);
                ALOGE("run out off inputtest.pcm");
            }

        }
        else
        {
            ALOGE("open inputtest.pcm fail");
        }
   }
}

void AudioYusuApStreamIn::Parse_Test_MONO_PCM(short *buffer, int bytes)
{
    //set this property to parse data
    char value[PROPERTY_VALUE_MAX];
    property_get("SPEInputTestMono.pcm", value, "0");
    int bflag=atoi(value);
    if(bflag)
    {
        char filename[]="/sdcard/inputtestmono.pcm";
        FILE * fp= fopen(filename, "rb+");
        if(fp!=NULL)
        {
            int16* pTempInputBuf;
            pTempInputBuf = new int16[bytes>>2];
            int16* pTempInputBufRecord = pTempInputBuf;
            
            if(!mStartReadTestFile)
            {            
                mStartReadTestFile = true;
                mTestFileSizeRead = 0;
                fseek(fp,0,SEEK_END);
                mTestFileLength = ftell(fp);        
//                LOGD("Parse_Test_MONO_PCM, mTestFileLength=%d, mTestFileSizeRead=%d",mTestFileLength,mTestFileSizeRead);
            }

            if(mTestFileSizeRead<=mTestFileLength)
            {
            
                
                int readbytes;
                if((mTestFileSizeRead + (bytes>>1))<=mTestFileLength)
                {
//                    LOGD("mTestFileLength=%d, mTestFileSizeRead=%d,bytes=%d",mTestFileLength,mTestFileSizeRead,bytes);
                    fseek(fp,mTestFileSizeRead,SEEK_SET);
                    readbytes = fread(pTempInputBuf,sizeof(short),bytes>>2,fp);

                    mTestFileSizeRead+=(bytes>>1);
                    
                    int count = bytes>>2;
                    //LOGD("readbytes=%d, mTestFileSizeRead=%d,count=%d",readbytes,mTestFileSizeRead,count);
                    while(count)
                    {
                        *(buffer) = *pTempInputBuf;
                        //*((short*)buffer+1) = *pTempInputBuf;
                        //*(buffer) = 0;
                        *(buffer+1) = 0;
                        
                        buffer+=2;
                        pTempInputBuf++;
                        count--;                        
                    }
                }
                else
                {

                    //LOGD("2mTestFileLength=%d, mTestFileSizeRead=%d,bytes=%d,pTempInputBuf=%p",mTestFileLength,mTestFileSizeRead,bytes,pTempInputBuf);
                    fseek(fp,mTestFileSizeRead,SEEK_SET);
                    readbytes = fread(pTempInputBuf,sizeof(short),(mTestFileLength-mTestFileSizeRead)>>1,fp);
                    pTempInputBuf = pTempInputBuf+readbytes;                    

                    //LOGD("readbytes=%d, mTestFileSizeRead=%d, pTempInputBuf=%p",readbytes,mTestFileSizeRead,pTempInputBuf);
                    
                    fseek(fp,0,SEEK_SET);
                    //readbytes = fread(pTempInputBuf,sizeof(short),(bytes-(mTestFileLength-mTestFileSizeRead))>>2,fp);
                    readbytes = fread(pTempInputBuf,sizeof(short),((bytes>>2)-((mTestFileLength-mTestFileSizeRead)>>1)),fp);
                    mTestFileSizeRead=(bytes>>1)-(mTestFileLength-mTestFileSizeRead);
                    int count = bytes>>2;
                    //LOGD("readbytes=%d, mTestFileSizeRead=%d,count=%d",readbytes,mTestFileSizeRead,count);
                    while(count)
                    {
                        *(buffer) = *pTempInputBufRecord;
                        *(buffer+1) = 0;
                        buffer+=2;
                        pTempInputBufRecord++;
                        count--;                        
                    }
                }
                fclose(fp);
            }
            else    //parse run outoff pcm data
            {
                mStartReadTestFile = false;
                mTestFileLength = 0;
                mTestFileSizeRead = 0;
                memcpy(buffer,0,bytes);
                fclose(fp);
                ALOGE("run out off inputtest.pcm");
            }

        }
        else
        {
            ALOGE("open inputtest.pcm fail");
        }
   }
}
#endif

ssize_t AudioYusuApStreamIn::read(void* buffer, ssize_t bytes)
{
    mLock.lock ();
    ssize_t Read_Size;
    if(mstandby == true){
        //open hardware and start record
        RecOpen ();
        mstandby = false;
        ALOGD("AudioYusuApStreamIn::read mstandby = %d",mstandby);
    }
    mLock.unlock ();
    uint32 readbytes = bytes;
    usleep(1*1000);
    Mutex::Autolock _l(mLock);
        
    uint32 CopyBufferwriteIdx =0,emptybufLen=0;
    if(pSrcHdl && (mApSampleRate != mSampleRate)){
        do{
            //  here need to do SRC and copy to buffer,check if there is any buf in src , exhaust it.
            if(GetSrcbufvalidSize()){
                uint32 bufLen =0;
                uint32 ConsumeReadbytes =0;
                if(pInputSrcWrite >= pInputSrcRead){
                    bufLen = GetSrcbufvalidSize();
                    ALOGV("pInputSrcWrite = %d pInputSrcRead = %d pInputbuflen = %d readbytes = %d CopyBufferwriteIdx = %d",
                        pInputSrcWrite,pInputSrcRead,pInputbuflen,readbytes,CopyBufferwriteIdx);
                    ConsumeReadbytes = readbytes;
                    CopySrcBuf((char*)buffer+CopyBufferwriteIdx,&readbytes,pInputbuf+pInputSrcRead,&bufLen);
                    CopyBufferwriteIdx +=ConsumeReadbytes - readbytes;
                    if(readbytes == 0){
						if(CanRecordFM ()){
    					int Volume_FM = mHw->mVolumeController->GetFmVolume ();
						StreamInApplyVolume((short*)buffer,bytes,FM_Map_gain[FM_Mapping_table[Volume_FM]]);
					    }
#ifdef MTK_AP_SPEECH_ENHANCEMENT					    
					    else{//not process FM record data                            
                            mHw->mpSPELayer->Process(UPLINK,(short*)buffer,bytes);
                        }
#endif                        
                        #ifdef MTK_DUAL_MIC_SUPPORT                       
                        CheckNeedDataSwitch((short*)(buffer),bytes);
                        #endif    

                        Dump_StreamIn_PCM(buffer,bytes);
                        CheckNeedMute(buffer,bytes);
                        return bytes;
                    }
                }
                else{
                    bufLen = pInputbuflen - pInputSrcRead;
                    ALOGV("pInputSrcWrite = %d pInputSrcRead = %d pInputbuflen = %d readbytes = %d CopyBufferwriteIdx = %d",
                        pInputSrcWrite,pInputSrcRead,pInputbuflen,readbytes,CopyBufferwriteIdx);
                    ConsumeReadbytes = readbytes;
                    CopySrcBuf((char*)buffer+CopyBufferwriteIdx,&readbytes,pInputbuf+pInputSrcRead,&bufLen);
                    CopyBufferwriteIdx +=ConsumeReadbytes - readbytes;
                    if(readbytes == 0){
						if(CanRecordFM ()){
    					int Volume_FM = mHw->mVolumeController->GetFmVolume ();
						StreamInApplyVolume((short*)buffer,bytes,FM_Map_gain[FM_Mapping_table[Volume_FM]]);
					    }
#ifdef MTK_AP_SPEECH_ENHANCEMENT					    
					    else{//not process FM record data                            
                            mHw->mpSPELayer->Process(UPLINK,(short*)buffer,bytes);
                        }
#endif                        
                        #ifdef MTK_DUAL_MIC_SUPPORT                       
                        CheckNeedDataSwitch((short*)(buffer),bytes);
                        #endif    

                        Dump_StreamIn_PCM(buffer,bytes);
                        CheckNeedMute(buffer,bytes);
                        return bytes;
                    }
                    else{
                        bufLen = pInputSrcWrite - pInputSrcRead;
                        ALOGV("pInputSrcWrite = %d pInputSrcRead = %d pInputbuflen = %d readbytes = %d CopyBufferwriteIdx = %d",
                            pInputSrcWrite,pInputSrcRead,pInputbuflen,readbytes,CopyBufferwriteIdx);
                        ConsumeReadbytes = readbytes;
                        CopySrcBuf((char*)buffer+CopyBufferwriteIdx,&readbytes,pInputbuf+pInputSrcRead,&bufLen);
                        CopyBufferwriteIdx +=ConsumeReadbytes - readbytes;
                        if(readbytes == 0){
							if(CanRecordFM ()){
    						int Volume_FM = mHw->mVolumeController->GetFmVolume ();
							StreamInApplyVolume((short*)buffer,bytes,FM_Map_gain[FM_Mapping_table[Volume_FM]]);
					    	}
#ifdef MTK_AP_SPEECH_ENHANCEMENT					    	
					    	else{//not process FM record data                                
                                 mHw->mpSPELayer->Process(UPLINK,(short*)buffer,bytes);
                             }
#endif                             
                            #ifdef MTK_DUAL_MIC_SUPPORT                       
                            CheckNeedDataSwitch((short*)(buffer),bytes);
                            #endif    

                            Dump_StreamIn_PCM(buffer,bytes);
                            CheckNeedMute(buffer,bytes);
                            return bytes;
                        }
                    }
                }
            }

            if(pInputSrcRead >= pInputSrcWrite){
                 emptybufLen = GetSrcbufFreeSize();
                 ALOGV("read emptybufLen = %d pSrcReadSize = %d readbytes = %d pSrcReadSize =%d",
                     emptybufLen,pSrcReadSize,readbytes,pSrcReadSize);
                 emptybufLen = (pSrcReadSize>= emptybufLen)? emptybufLen : pSrcReadSize;
                 Read_Size = ::read(mFd,pInputbuf+pInputSrcWrite,emptybufLen);
                 if(!mNeedStereo)
                 StereoToMono((short*)(pInputbuf+pInputSrcWrite),Read_Size);
                 pInputSrcWrite += Read_Size;
                     if(pInputSrcWrite >= pInputbuflen){
                     pInputSrcWrite -= pInputbuflen;
                 }
                 ALOGV("read pInputSrcWrite = %d pInputSrcRead = %d pInputbuflen = %d readbytes = %d",
                     pInputSrcWrite,pInputSrcRead,pInputbuflen,readbytes);
             }
             else{
                 emptybufLen = pInputbuflen - pInputSrcWrite;
                 Read_Size = ::read(mFd,pInputbuf+pInputSrcWrite,emptybufLen);
                 if(!mNeedStereo)
                 StereoToMono((short*)(pInputbuf+pInputSrcWrite),Read_Size);
                 pInputSrcWrite += Read_Size;
                 if(pInputSrcWrite >= pInputbuflen){
                     pInputSrcWrite -= pInputbuflen;
                 }
                 ALOGV("read pInputSrcWrite = %d pInputSrcRead = %d pInputbuflen = %d readbytes = %d",
                     pInputSrcWrite,pInputSrcRead,pInputbuflen,readbytes);
             }

             // error handle for read error
             if(Read_Size ==0){
                 ALOGW("Read_Size return 0...");
                 usleep(50*1000);
				 if(CanRecordFM ()){
    			 int Volume_FM = mHw->mVolumeController->GetFmVolume ();
				 StreamInApplyVolume((short*)buffer,bytes,FM_Map_gain[FM_Mapping_table[Volume_FM]]);
				 }
#ifdef MTK_AP_SPEECH_ENHANCEMENT				 
				 else{//not process FM record data                     
                     mHw->mpSPELayer->Process(UPLINK,(short*)buffer,bytes);
                 }
#endif           
                 #ifdef MTK_DUAL_MIC_SUPPORT                       
                 CheckNeedDataSwitch((short*)(buffer),bytes);
                 #endif    

                 Dump_StreamIn_PCM(buffer,bytes);
                 CheckNeedMute(buffer,bytes);
                 return bytes;
             }
         }while(readbytes);
     }
    else{
        Read_Size = ::read(mFd,buffer,bytes);
        if(CanRecordFM ()){
            // here to aplpy volume with fn recording
            int Volume_FM = mHw->mVolumeController->GetFmVolume ();
            StreamInApplyVolume((short*)buffer,Read_Size,FM_Map_gain[FM_Mapping_table[Volume_FM]]);
        }
#ifdef MTK_AP_SPEECH_ENHANCEMENT        
        else{//not process FM record data
            //Parse_Test_STEREO_PCM(buffer,bytes);
            //Parse_Test_MONO_PCM((short*)buffer,bytes);            
            mHw->mpSPELayer->Process(UPLINK,(short*)buffer,bytes);
        }        
#endif
        #ifdef MTK_DUAL_MIC_SUPPORT					    
        CheckNeedDataSwitch((short*)(buffer),bytes);
        #endif	
        if(!mNeedStereo)    //if APP needs mono, transfer stereo to mono
            StereoToMono((short*)(buffer),bytes);        
        Dump_StreamIn_PCM(buffer,bytes);
        CheckNeedMute(buffer,bytes);
        
        return bytes;
    }

	if(CanRecordFM ()){
    int Volume_FM = mHw->mVolumeController->GetFmVolume ();
	StreamInApplyVolume((short*)buffer,bytes,FM_Map_gain[FM_Mapping_table[Volume_FM]]);
    }
#ifdef MTK_AP_SPEECH_ENHANCEMENT    
    else{//not process FM record data         
            mHw->mpSPELayer->Process(UPLINK,(short*)buffer,bytes);
    }
#endif    
    #ifdef MTK_DUAL_MIC_SUPPORT                       
    CheckNeedDataSwitch((short*)(buffer),bytes);
    #endif    

    Dump_StreamIn_PCM(buffer,bytes);
    CheckNeedMute(buffer,bytes);
    return bytes;
}

status_t AudioYusuApStreamIn::dump(int fd, const Vector<String16>& args)
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

status_t AudioYusuApStreamIn::setParameters(const String8& keyValuePairs)
{
    AudioParameter param = AudioParameter(keyValuePairs);
    String8 key = String8(AudioParameter::keyRouting);
    String8 key_input_device = String8(AudioParameter::keyInputSource);

    int input_source;
    int device;
    status_t status = NO_ERROR;
    ALOGV("setParameters() %s", keyValuePairs.string());

    if (param.getInt(key, device) == NO_ERROR) {
        mDevice = device;
        param.remove(key);
    }

    if (param.getInt(key_input_device, input_source) == NO_ERROR) {
        ALOGD("setParameters, input_source(%d)", input_source);
        mInputSource = input_source;
        if(mInputSource == AUDIO_SOURCE_VOICE_RECOGNITION){
            mMutecount =(MUTE_APAUDIO_PCM_MS*mSampleRate*mChNum<<1)/1000;
            ALOGD("setParameters, VOICE_RECOGNITION(%d) mMutecount = %d", input_source,mMutecount);
        }
        else{
            mMutecount =(MUTE_APAUDIO_PCM_MS_NORMAL*mSampleRate*mChNum<<1)/1000;
            ALOGD("setParameters, %d mMutecount = %d", input_source,mMutecount);
        }

        param.remove(key_input_device);
    }

    if (param.size()) {
        status = BAD_VALUE;
    }
    return status;
}

String8 AudioYusuApStreamIn::getParameters(const String8& keys)
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

bool AudioYusuApStreamIn::IsStandby(void)
{
    return mstandby;
}

void AudioYusuApStreamIn::SetDigitalMic()
{
    uint32 ul_src =0;
    ul_src |= BIT_01;       // set SDM_3_LEVEL
    ul_src |= BIT_21;     // set UL3P25M_MODE CH1
    ul_src |= BIT_22;     // set UL3P25M_MODE CH2
    ul_src |= BIT_23;     // set TWO_WIRE_DIG_MIC
    ul_src &= ~(BIT_05); // use 3.25
    mHw->mAfe_handle->SetAfeReg (AFE_UL_SRC_1,ul_src ,(BIT_01|BIT_05|BIT_21|BIT_22|BIT_23));
}

void AudioYusuApStreamIn::SetMicSource(int mictype){
    ALOGD("SetMicSource micsource = %d",mictype);
#ifdef MTK_DUAL_MIC_SUPPORT
	if(mictype == LADIN_Microphone1)
		mictype = LADIN_DualAnalogMic;
	ALOGD("MTK_DUAL_MIC_SUPPORT micsource = %d",mictype);
#endif
    mApInputSource = mictype;
    ioctl(mFd,AUD_SET_ANA_CLOCK,1);
    switch (mictype){
        case LADIN_FM_Radio:{
            mHw->mAnaReg->SetAnaReg(AUDIO_CON23,0xDD00,0xffffffff);
            mHw->mAnaReg->SetAnaReg(AUDIO_CON3,0x000C,0x000C);       // power up level shift buffer
            usleep(200);
            mHw->mVolumeController->SetLevelShiftBufferGain ();
            mHw->mAnaReg->SetAnaReg(AUDIO_CON5,0x0001,0xf);       // Select Level-shift buffer input as line-in L channels
            mHw->mAnaReg->SetAnaReg(AUDIO_CON6,0x2533,0xffff);       // Select Level-shift buffer input as line-in R ch
            usleep(200);
            mHw->mAnaReg->SetAnaReg(AUDIO_CON17,0X0008,0xffff);
            break;
        }
        case LADIN_Microphone1:{
            mHw->mAnaReg->SetAnaReg(AUDIO_CON24,0x0,0xffffffff);
            mHw->mAnaReg->SetAnaReg(AUDIO_CON23,0xCC00,0xffffffff);
            usleep(200);
            mHw->mAnaReg->SetAnaReg(AUDIO_CON21,0x1100,0xffffffff);  // left preamp input selection
            
            //set the interconnection let right channel get the same data (interconnection does not effected by stereo/mono channel)
            mHw->mAnaReg->SetAnaReg(AUDIO_CON22,0x1100,0xffffffff);  // right preamp input selection
//                mHw->mAnaReg->SetAnaReg(AUDIO_CON22,0x1200,0xffffffff);  // right preamp input selection
            
            break;
        }
        case LADIN_Microphone2:{
            uint32 AudioReg;
            mHw->mAnaReg->SetAnaReg(AUDIO_CON24,0x0,0xffffffff);
            mHw->mAnaReg->SetAnaReg(AUDIO_CON23,0xCC00,0xffffffff);
            usleep(200);
            mHw->mAnaReg->SetAnaReg(AUDIO_CON21,0x1300,0xffffffff);  // left preamp input selection
            mHw->mAnaReg->SetAnaReg(AUDIO_CON22,0x1300,0xffffffff);  // right preamp input selection
            break;
        }
        case LADIN_BTIn:{
            // record BT??
            break;
        }
        case LADIN_SingleDigitalMic:{
            mHw->mAnaReg->SetAnaReg(AUDIO_CON24,0x1,0xffffffff);
            mHw->mAnaReg->SetAnaReg(AUDIO_CON23,0xCC00,0xffffffff);
            mHw->mAnaReg->SetAnaReg(AUDIO_CON21,0x1100,0xffffffff);  // left preamp input selection
            mHw->mAnaReg->SetAnaReg(AUDIO_CON22,0x1100,0xffffffff);  // right preamp input selection
            break;
        }
        case LADIN_DualAnalogMic:{
            mHw->mAnaReg->SetAnaReg(AUDIO_CON24,0x0,0xffffffff);
            mHw->mAnaReg->SetAnaReg(AUDIO_CON23,0xCC00,0xffffffff);
            usleep(200);
            //mHw->mAnaReg->SetAnaReg(AUDIO_CON21,0x1100,0xffffffff);  // left preamp input selection	left channel use Ref MIC
            //mHw->mAnaReg->SetAnaReg(AUDIO_CON22,0x1200,0xffffffff);  // right preamp input selection	right channel use main MIC
            mHw->mAnaReg->SetAnaReg(AUDIO_CON21,0x1200,0xffffffff);  // left preamp input selection,	left channel use main MIC
            mHw->mAnaReg->SetAnaReg(AUDIO_CON22,0x1100,0xffffffff);  // right preamp input selection,	right channel use Ref MIC
            break;
        }
        case LADIN_DualDigitalMic:{
            mHw->mAnaReg->SetAnaReg(AUDIO_CON24,0x4,0xffffffff);
            mHw->mAnaReg->SetAnaReg(AUDIO_CON23,0xCC00,0xffffffff);
            mHw->mAnaReg->SetAnaReg(AUDIO_CON21,0x1100,0xffffffff);  // left preamp input selection
            mHw->mAnaReg->SetAnaReg(AUDIO_CON22,0x1200,0xffffffff);  // right preamp input selection
            break;
        }
        case LADIN_FM_AnalogIn_Stereo:{
            mHw->mAnaReg->SetAnaReg(AUDIO_CON23,0xDD00,0xffffffff);
            mHw->mAnaReg->SetAnaReg(AUDIO_CON3,0x000C,0x000C);       // power up level shift buffer
            usleep(200);
            mHw->mVolumeController->SetLevelShiftBufferGain ();
            usleep(200);
            mHw->mAnaReg->SetAnaReg(AUDIO_CON5,0x0001,0xf);       // Select Level-shift buffer input as line-in L channels
            mHw->mAnaReg->SetAnaReg(AUDIO_CON6,0x2533,0xffff);       // Select Level-shift buffer input as line-in R ch
            mHw->mAnaReg->SetAnaReg(AUDIO_CON17,0X0008,0xffff);
            break;
        }
        default:
            break;
    }
    ioctl(mFd,AUD_SET_ANA_CLOCK,0);
}

void AudioYusuApStreamIn::CloseRecordDevicePower()
{
    ALOGD("CloseRecordDevicePower mApInputSource = %d",mApInputSource);
    ioctl(mFd,AUD_SET_ANA_CLOCK,1);
    switch(mApInputSource){
        case LADIN_FM_Radio:
        case LADIN_FM_AnalogIn_Stereo:
        {
            if(mHw->mStreamHandler->GetSrcBlockRunning () == false && mHw->GetAnalogLineinEnable ()== false){
                mHw->mAnaReg->SetAnaReg(AUDIO_NCP0,0x0,0xffff);
                mHw->mAnaReg->SetAnaReg(AUDIO_NCP1,0x0,0xffff);
                mHw->mAnaReg->SetAnaReg(AUDIO_LDO0,0x0 ,0xffffffff);
                mHw->mAnaReg->SetAnaReg(AUDIO_LDO1,0x0 ,0xffffffff);
                mHw->mAnaReg->SetAnaReg(AUDIO_LDO2,0x0 ,0xffffffff);
                mHw->mAnaReg->SetAnaReg(AUDIO_GLB0,0x0,0xffff);
                mHw->mAnaReg->SetAnaReg(AUDIO_GLB1,0x0,0xffff);
                mHw->mAnaReg->SetAnaReg(AUDIO_GLB2,0x0,0xffff);
                mHw->mAnaReg->SetAnaReg(AUDIO_CON14,0x0 ,0xc0c0); // uplink preamp gain
            }

            usleep(200);
            mHw->mAnaReg->SetAnaReg(AUDIO_CON31,0x0 ,0xffffffff);
            usleep(200);
            mHw->mAnaReg->SetAnaReg(AUDIO_CON20,0x0 ,0xffffffff); // uplink preamp gain
            mHw->mAnaReg->SetAnaReg(AUDIO_CON34,0x0,0xffffffff);  // VMIC con34
            mHw->mAnaReg->SetAnaReg(AUDIO_CON23,0x5500,0xffffffff); //close MIC power
            usleep(200);
            mHw->mAnaReg->SetAnaReg(AUDIO_CON3,0,0x000c);
            mHw->mAnaReg->SetAnaReg(AUDIO_CON4,0x0404,0x3f3f);
            mHw->mAnaReg->SetAnaReg(AUDIO_CON5,0,0x000f);
            mHw->mAnaReg->SetAnaReg(AUDIO_CON6,0,0xf000);
            break;
        }
        case LADIN_Microphone1:
        case LADIN_Microphone2:
        case LADIN_DualAnalogMic:
        {
            if(mHw->mStreamHandler->GetSrcBlockRunning () == false){
                mHw->mAnaReg->SetAnaReg(AUDIO_LDO1,0x0 ,0xffffffff);
                mHw->mAnaReg->SetAnaReg(AUDIO_LDO2,0x0 ,0xffffffff);
                mHw->mAnaReg->SetAnaReg(AUDIO_CON31,0x0 ,0xffffffff);
                mHw->mAnaReg->SetAnaReg(AUDIO_CON14,0x0 ,0xc0c0); // uplink preamp gain
            }
            usleep(200);
            mHw->mAnaReg->SetAnaReg(AUDIO_CON20,0x0 ,0xffffffff); // uplink preamp gain
            mHw->mAnaReg->SetAnaReg(AUDIO_CON34,0x0,0xffffffff);  // VMIC con34
            mHw->mAnaReg->SetAnaReg(AUDIO_CON23,0x4400,0xffffffff); //close MIC power
            usleep(200);

            if(mHw->mStreamHandler->GetSrcBlockRunning () == false){
                mHw->mAnaReg->SetAnaReg(AUDIO_NCP0,0x0,0xffff);
                mHw->mAnaReg->SetAnaReg(AUDIO_NCP1,0x0,0x0E00);
                mHw->mAnaReg->SetAnaReg(AUDIO_LDO0,0x0,0x1fff);
                mHw->mAnaReg->SetAnaReg(AUDIO_LDO1,0x0,0xffff);
                mHw->mAnaReg->SetAnaReg(AUDIO_LDO2,0x0,0x0013);
                mHw->mAnaReg->SetAnaReg(AUDIO_GLB0,0x0,0xffff);
                mHw->mAnaReg->SetAnaReg(AUDIO_GLB1,0x0,0xffff);
                mHw->mAnaReg->SetAnaReg(AUDIO_REG1,0x0,0x0001);
            }
            break;
        }
        case LADIN_SingleDigitalMic:
        case LADIN_DualDigitalMic:
        {
            mHw->mAnaReg->SetAnaReg(AUDIO_CON33,0x0,0xffffffff);  // VMIC con34
            mHw->mAnaReg->SetAnaReg(AUDIO_CON34,0x0,0xffffffff);  // VMIC con34
            mHw->mAnaReg->SetAnaReg(AUDIO_CON23,0x4400,0xffffffff); //close MIC power
            break;
        }
        case LADIN_BTIn:
        {
            break;
        }
        default:
        {
            ALOGW("openRecordDevicePower with no config mHw->GetApMicType () = %d",mHw->GetApMicType ());
        }
    }
    ioctl(mFd,AUD_SET_ANA_CLOCK,0);
}

void AudioYusuApStreamIn::openRecordDevicePower()
{
    LadInPutDevice_Line Device;
    ioctl(mFd,AUD_SET_ANA_CLOCK,1);
    if(CanRecordFM()){
        Device = LADIN_FM_Radio;
    }
    else{
        Device = (LadInPutDevice_Line)mHw->GetApMicType ();
    }
    ALOGD("openRecordDevicePower device = %d",Device);
    switch(Device){
        case LADIN_FM_Radio:
        case LADIN_FM_AnalogIn_Stereo:
        {
            if(mHw->mStreamHandler->GetSrcRunningNumber () == false){
                mHw->mAnaReg->SetAnaReg(AUDIO_NCP0,0x102B,0xffff);
                mHw->mAnaReg->SetAnaReg(AUDIO_NCP1,0x0000,0xffff);
                mHw->mAnaReg->SetAnaReg(AUDIO_LDO0,0x1030 ,0xffffffff);
                mHw->mAnaReg->SetAnaReg(AUDIO_LDO1,0x3010 ,0xffffffff);
                mHw->mAnaReg->SetAnaReg(AUDIO_LDO2,0x0013 ,0xffffffff);
                mHw->mAnaReg->SetAnaReg(AUDIO_GLB0,0x3D30,0xffff);
                mHw->mAnaReg->SetAnaReg(AUDIO_GLB1,0x0000,0xffff);
                mHw->mAnaReg->SetAnaReg(AUDIO_GLB2,0x0001,0xffff);
            }
            else{
                mHw->mAnaReg->SetAnaReg(AUDIO_LDO2,0x0013 ,0xffffffff);
                mHw->mAnaReg->SetAnaReg(AUDIO_GLB0,0x3D30,0xffff);
            }
            usleep(200);
            mHw->mAnaReg->SetAnaReg(AUDIO_CON31,0x8000 ,0xffffffff);
            usleep(200);

            mHw->mAnaReg->SetAnaReg(AUDIO_CON20,0x0c0c ,0xffffffff); // uplink preamp gain
            mHw->mAnaReg->SetAnaReg(AUDIO_CON34,0x8000,0xffffffff);  // VMIC con34
            mHw->mAnaReg->SetAnaReg(AUDIO_CON14,0xc0 ,0xc0c0); // uplink preamp gain

            //mHw->mAnaReg->SetAnaReg(AUDIO_CON9,0x3 ,0xf);
            mHw->mAnaReg->SetAnaReg(AUDIO_CON6,0x2a33 ,0xffffffff);
            usleep(200);
            break;
        }
        case LADIN_Microphone1:
        case LADIN_Microphone2:
        case LADIN_DualAnalogMic:
        {
            mHw->mAnaReg->SetAnaReg(AUDIO_LDO1,0x3000 ,0xffffffff);
            mHw->mAnaReg->SetAnaReg(AUDIO_LDO2,0x0013 ,0xffffffff);
            mHw->mAnaReg->SetAnaReg(AUDIO_CON31,0x8000 ,0xffffffff);
            usleep(200);

            mHw->mAnaReg->SetAnaReg(AUDIO_CON20,0x3f3f ,0xffffffff); // uplink preamp gain
            mHw->mAnaReg->SetAnaReg(AUDIO_CON34,0x8000,0xffffffff);  // VMIC con34
            mHw->mAnaReg->SetAnaReg(AUDIO_CON14,0xc0 ,0xc0c0); // uplink preamp gain
            usleep(200);
            if(mHw->mStreamHandler->GetSrcRunningNumber () == false){
                mHw->mAnaReg->SetAnaReg(AUDIO_NCP0,0x102B,0xffff);
                mHw->mAnaReg->SetAnaReg(AUDIO_NCP1,0x0600,0x0E00);
                mHw->mAnaReg->SetAnaReg(AUDIO_LDO0,0x1030,0x1fff);
                mHw->mAnaReg->SetAnaReg(AUDIO_LDO1,0x3010,0xffff);
                mHw->mAnaReg->SetAnaReg(AUDIO_LDO2,0x0013,0x0013);
                mHw->mAnaReg->SetAnaReg(AUDIO_GLB0,0x2920,0xffff);
                mHw->mAnaReg->SetAnaReg(AUDIO_GLB1,0x0000,0xffff);
                mHw->mAnaReg->SetAnaReg(AUDIO_REG1,0x0001,0x0001);
            }
            else{
                mHw->mAnaReg->SetAnaReg(AUDIO_GLB0,0x2920,0xffff);
            }
            break;
        }
        case LADIN_SingleDigitalMic:
        case LADIN_DualDigitalMic:
        {
            mHw->mAnaReg->SetAnaReg(AUDIO_CON33,0x0042,0xffffffff);  // VMIC con34
            mHw->mAnaReg->SetAnaReg(AUDIO_CON34,0x8000,0xffffffff);  // VMIC con34
            break;
        }
        case LADIN_BTIn:
        {
            break;
        }
        default:
        {
            ALOGW("openRecordDevicePower with no config mHw->GetApMicType () = %d",mHw->GetApMicType ());
        }
    }
    ioctl(mFd,AUD_SET_ANA_CLOCK,0);
}

void AudioYusuApStreamIn::SetStreamInputSource(int Inputsource)
{
    ALOGD("SetStreamInputSource Inputsource= %d mApInputSource = %d",Inputsource,mApInputSource);
#ifdef MTK_DUAL_MIC_SUPPORT
	if(Inputsource == LADIN_Microphone1)
		Inputsource = LADIN_DualAnalogMic;
	ALOGD("MTK_DUAL_MIC_SUPPORT Inputsource = %d",Inputsource);
#endif
    if(Inputsource == mApInputSource){
        return;
    }
    // now
    else if(mApInputSource == LADIN_FM_Radio){
        return;
    }
    else{
        Mutex::Autolock lock(mLock);
        // close and open with new input
        CloseRecordDevicePower ();
        //RecClose ();
        usleep(5*1000);
        mApInputSource = Inputsource;
        //RecOpen ();
#ifdef MTK_AP_SPEECH_ENHANCEMENT        
        if(CanRecordFM()){
            SetAPMicGain(LADIN_FM_Radio);
            mNeedStereo = false;
        }
        else
        {            
            int32 mode;
            mHw->getMode(&mode);
            bool bStereoRecOn = false;
            int modeIndex;

#ifdef  MTK_AP_SPEECH_ENHANCEMENT_VOIP    
            if(mode == android_audio_legacy::AudioSystem::MODE_IN_COMMUNICATION)    //VoIP
            {
//                mHw->mpSPELayer->LoadSPEParameter();    //load voip parameter
                mHw->mpSPELayer->SetPGAGain(SPE_MODE_VOIP, SetApMicGain_HDRecord(mApInputSource));
//                SetAPMicGain(mApInputSource);
                if(mApInputSource == LADIN_Microphone2)    //headset
                {
                    mHw->mpSPELayer->SetRoute(ROUTE_HEADSET);
                }
                else if(mApInputSource == LADIN_BTIn)
                {
                    mHw->mpSPELayer->SetRoute(ROUTE_BT);
                }
                else
                {
                    if(mHw->GetCurrentDevice () == android_audio_legacy::AudioSystem::DEVICE_OUT_SPEAKER)
                        mHw->mpSPELayer->SetRoute(ROUTE_SPEAKER);
                    else
                        mHw->mpSPELayer->SetRoute(ROUTE_NORMAL);
                    
                }
            }
            else    //normal record
#endif            
            {
                modeIndex = SetHdrecordingMode(mode,&bStereoRecOn);
                if((modeIndex != mHDRecordModeIndex)&&(modeIndex>=0))
                {
                    mHDRecordModeIndex = modeIndex;
                    if(IsStereoMIC()&& bStereoRecOn)
                        mNeedStereo = true;
                    else
                        mNeedStereo = false;
                    
                    UpdateSPEParams();                
                }
            
                mHw->mpSPELayer->SetPGAGain(SPE_MODE_REC, SetApMicGain_HDRecord(mApInputSource));
//                SetAPMicGain(mApInputSource);
            }
            if(mHw->mpSPELayer->IsSPERunning())
            {
                ALOGD("restart SPE for new parameters");
                mHw->mpSPELayer->Stop();
#ifdef  MTK_AP_SPEECH_ENHANCEMENT_VOIP                    
                if(mode == android_audio_legacy::AudioSystem::MODE_IN_COMMUNICATION)    //VoIP
                    mHw->mpSPELayer->Start(SPE_MODE_VOIP);
                else
#endif                
                {                
                    mHw->mpSPELayer->Start(SPE_MODE_REC);
                }
            }
        }
#else
        SetAPMicGain(mApInputSource);
        if(IsStereoMIC())
			mNeedStereo = true;
		else
			mNeedStereo = false;
#endif
        usleep(1*1000);
        SetMicSource (mHw->GetApMicType ());
        openRecordDevicePower ();
    }
}


bool AudioYusuApStreamIn::RecOpen(void)
{
    ALOGD("RecOpen mstandby = %d",mstandby);
    if(mstandby == true)
    {
#ifdef MTK_AP_SPEECH_ENHANCEMENT 
        int32 mode;
        mHw->getMode(&mode);
#endif
        mHw->mStreamHandler->OutputStreamLock();
        mHw->SwitchAudioClock(true);
        mHw->Set_Recovery_Record(true);
        ioctl(mFd,AUD_SET_ANA_CLOCK,1);
        // start ADC and AWB write back
        ALOGD("RecOpen SET_AWB_INPUT_STREAM_STATE");
        ::ioctl(mFd,SET_AWB_INPUT_STREAM_STATE,1);
        usleep(200);

        #ifdef AUDIO_HQA_SUPPORT
        ALOGD("RecOpen,AP InputSource");
        SetMicSource (mHw->GetApMicType ());
        #else
        if(CanRecordFM()){
            SetMicSource (LADIN_FM_Radio);
        }
        else{
            SetMicSource (mHw->GetApMicType ());
        }
        #endif

#ifdef MTK_AP_SPEECH_ENHANCEMENT         
        // set mic gain for recordingAllowed
        if(CanRecordFM()){
            SetAPMicGain(LADIN_FM_Radio);
            mNeedStereo = false;
        }
        else
        {            
            bool bStereoRecOn = false;
            int modeIndex;
            int MicType = mHw->GetApMicType();

#ifdef  MTK_AP_SPEECH_ENHANCEMENT_VOIP    
            if(mode == android_audio_legacy::AudioSystem::MODE_IN_COMMUNICATION)    //VoIP
            {
//                mHw->mpSPELayer->LoadSPEParameter();    //load voip parameter
                mHw->mpSPELayer->SetPGAGain(SPE_MODE_VOIP, SetApMicGain_HDRecord(mApInputSource));
//                SetAPMicGain(mApInputSource);

                if(mApInputSource == LADIN_Microphone2)    //headset
                {
                    mHw->mpSPELayer->SetRoute(ROUTE_HEADSET);
                }
                else if(mApInputSource == LADIN_BTIn)
                {
                    mHw->mpSPELayer->SetRoute(ROUTE_BT);
                }
                else
                {
                    if(mHw->GetCurrentDevice () == android_audio_legacy::AudioSystem::DEVICE_OUT_SPEAKER)
                        mHw->mpSPELayer->SetRoute(ROUTE_SPEAKER);
                    else
                        mHw->mpSPELayer->SetRoute(ROUTE_NORMAL);
                    
                }
            }
            else    //record
#endif            
            {
                modeIndex = SetHdrecordingMode(mode,&bStereoRecOn);
                if((modeIndex != mHDRecordModeIndex)&&(modeIndex>=0))
                {
                    mHDRecordModeIndex = modeIndex;
                    if(IsStereoMIC()&& bStereoRecOn)
                        mNeedStereo = true;
                    else
                        mNeedStereo = false;
                        
                    UpdateSPEParams();
                }          

                mHw->mpSPELayer->SetPGAGain(SPE_MODE_REC, SetApMicGain_HDRecord(mApInputSource));
//                SetAPMicGain(mApInputSource);
            }

        }
#else
        SetAPMicGain(mApInputSource);
        if(IsStereoMIC())
			mNeedStereo = true;
		else
			mNeedStereo = false;
#endif

        //mHw->mAfe_handle->SetAfeReg(AUDIO_TOP_CON0,0x0,0xffffffff);
        //usleep(200);
        

        //mHw->mAfe_handle->SetAfeReg(AFE_UL_SRC_1,0x08df8df8,0xffffffff);
        mHw->mAfe_handle->SetAWBConenction (true);
        mHw->mAfe_handle->SetAWBChannel(mChNum);
        usleep(200);
        if((mHw->GetApMicType()==LADIN_SingleDigitalMic)||(mHw->GetApMicType()==LADIN_DualDigitalMic)){
            mHw->mAfe_handle->Afe_UL_Start (AFE_MODE_DMIC);
        }else{
            mHw->mAfe_handle->Afe_UL_Start (AFE_MODE_ADC);
        }
        mHw->mAfe_handle->Afe_UL_Start (AFE_MODE_AWB);
        usleep(200);

        openRecordDevicePower ();
        ioctl(mFd,AUD_SET_ANA_CLOCK,0);
        mHw->mStreamHandler->OutputStreamUnLock();

#ifdef MTK_AP_SPEECH_ENHANCEMENT  
        if(!CanRecordFM())
        {
#ifdef  MTK_AP_SPEECH_ENHANCEMENT_VOIP    
            if(mode == android_audio_legacy::AudioSystem::MODE_IN_COMMUNICATION)    //VoIP
            {
                ALOGD("AudioYusuApStreamIn VOIP start");
                mHw->mpSPELayer->Start(SPE_MODE_VOIP);
            }
            else
#endif            
            {
                ALOGD("AudioYusuApStreamIn RecOpen start SPElayver");
                mHw->mpSPELayer->Start(SPE_MODE_REC);
            }
        }
#endif
    }
    return true;
}

bool AudioYusuApStreamIn::RecClose(void)
{
    ALOGD("RecClose mstandby = %d",mstandby);
    if(mstandby == false){
        ALOGD("RecClose SET_AWB_INPUT_STREAM_STATE");
        mHw->mAfe_handle->Afe_UL_Stop (AFE_MODE_ADC);
        usleep(200);
        mHw->mAfe_handle->Afe_UL_Stop (AFE_MODE_AWB);
        ::ioctl(mFd,SET_AWB_INPUT_STREAM_STATE,0);
        usleep(200);
        CloseRecordDevicePower ();
        mstandby = true;
        mHw->SwitchAudioClock(false);
        mHw->Set_Recovery_Record(false);
    }
    return true;
}

bool AudioYusuApStreamIn::CanRecordFM()
{
    int mode;
    mHw->getMode (&mode);
    //ALOGD("+CanRecordFM mode:%d, mDevice:%x, FmEnable:%d",mode,mDevice,mHw->GetAnalogLineinEnable());

   if(mode != android_audio_legacy::AudioSystem::MODE_NORMAL)
       return false;

   //if(mHw->GetAnalogLineinEnable() && mHw->GetRecordFM () &&mDevice == android_audio_legacy::AudioSystem::DEVICE_IN_FM )
   if(mHw->GetAnalogLineinEnable() && mHw->GetRecordFM ())
   {
      //ALOGD("CanRecordFM return true, GetAnalogLineinEnable=%x,GetRecordFM=%x",mHw->GetAnalogLineinEnable(),mHw->GetRecordFM());
      return true;
   }
   else
   {
      return false;
   }
}


bool AudioYusuApStreamIn::MutexLock(void)
{
    mLock.lock ();
    return true;
}
bool AudioYusuApStreamIn::MutexUnlock(void)
{
    mLock.unlock ();
    return true;
}


void AudioYusuApStreamIn::SetAPMicGain(int mictype)
{
    ALOGD("SetAPMicGain mictype = %d, mInputSource=%d",mictype,mInputSource);
    
    if(mHw != NULL){
        int mode = 0;
        mHw->getMode (&mode);
        // handle for voice reg situation
        if(mInputSource == AUDIO_SOURCE_VOICE_RECOGNITION){
            if(mictype  == LADIN_Microphone2){
                mHw->mVolumeController->SetApMicGain(Voice_Rec_Mic_Headset);
            }
            else{
                mHw->mVolumeController->SetApMicGain(Voice_Rec_Mic_Handset);
            }
            return;
        }
        /*
        // handle for camera record situation, tempory not use
        if(mInputSource == AUDIO_SOURCE_CAMCORDER){
            if(mictype  == LADIN_Microphone2){
                mHw->mVolumeController->SetApMicGain(Idle_Video_Record_Headset);
            }
            else{
                mHw->mVolumeController->SetApMicGain(Idle_Video_Record_Handset);
            }
            return;
        }*/

        ALOGD("SetAPMicGain mode = %d",mode);
        switch(mode){
            case android_audio_legacy::AudioSystem::MODE_NORMAL:
            case android_audio_legacy::AudioSystem::MODE_RINGTONE:
                if(mictype  == LADIN_FM_Radio){
                    mHw->mVolumeController->SetApMicGain(Fm_Record);
                }
                else if(mictype  == LADIN_Microphone2){
                    mHw->mVolumeController->SetApMicGain(Idle_Headset_Record);
                }
                else{
                    mHw->mVolumeController->SetApMicGain(Idle_Normal_Record);
                }
                break;
            case android_audio_legacy::AudioSystem::MODE_IN_CALL:
                break;
            case android_audio_legacy::AudioSystem::MODE_IN_COMMUNICATION:
                if(mictype  == LADIN_Microphone2){
                    mHw->mVolumeController->SetApMicGain(VOIP_Headset_Mic);
                }
                else{
                    if(mHw->GetSpeakerStatus ()){
                    mHw->mVolumeController->SetApMicGain(VOIP_Handfree_Mic);
                    }
                    else{
                        mHw->mVolumeController->SetApMicGain(VOIP_Normal_Mic);
                    }
                }
                break;
            default:
                ALOGD("no match mode?");
                mHw->mVolumeController->SetApMicGain(Idle_Normal_Record);
                break;
            }
        }    
}

#ifdef MTK_AP_SPEECH_ENHANCEMENT  

#define NORMAL_RECORDING_DEFAULT_MODE    (1)
#define VOICE_REC_RECORDING_DEFAULT_MODE (0)

int AudioYusuApStreamIn::SetApMicGain_HDRecord(int mictype)
{
    ALOGD("SetApMicGain_HDRecord mictype = %d, mInputSource=%d",mictype,mInputSource);
    int agc_ul_gain = 0;
    
    if(mHw != NULL){
        int mode = 0;
        mHw->getMode (&mode);
        // handle for voice reg situation
        if(mInputSource == AUDIO_SOURCE_VOICE_RECOGNITION){
            if(mictype == LADIN_Microphone2){
                agc_ul_gain = mHw->mVolumeController->SetApMicGain_HDRecord(Voice_Rec_Mic_Headset);
            }
            else{
                agc_ul_gain = mHw->mVolumeController->SetApMicGain_HDRecord(Voice_Rec_Mic_Handset);
            }
            return agc_ul_gain;
        }
/*
        // handle for camera record situation, tempory not enable
        if(mInputSource == AUDIO_SOURCE_CAMCORDER){
            if(mictype  == LADIN_Microphone2){
                agc_ul_gain = mHw->mVolumeController->SetApMicGain_HDRecord(Idle_Video_Record_Headset);
            }
            else{
                agc_ul_gain = mHw->mVolumeController->SetApMicGain_HDRecord(Idle_Video_Record_Handset);
            }
            return agc_ul_gain;
        }
*/
        ALOGD("SetApMicGain_HDRecord mode = %d",mode);
        switch(mode){
            case android_audio_legacy::AudioSystem::MODE_NORMAL:
            case android_audio_legacy::AudioSystem::MODE_RINGTONE:
                if(mictype == LADIN_FM_Radio){
                    agc_ul_gain = mHw->mVolumeController->SetApMicGain_HDRecord(Fm_Record);
                }
                else if(mictype == LADIN_Microphone2){
                    agc_ul_gain = mHw->mVolumeController->SetApMicGain_HDRecord(Idle_Headset_Record);
                }
                else if(mictype == LADIN_SingleDigitalMic || mictype == LADIN_DualDigitalMic){
                    agc_ul_gain = mHw->mVolumeController->SetApMicGain_HDRecord(Idle_Normal_Record, true);
                }
                else{
                    agc_ul_gain = mHw->mVolumeController->SetApMicGain_HDRecord(Idle_Normal_Record);
                }
                break;
            case android_audio_legacy::AudioSystem::MODE_IN_CALL:
                break;
            case android_audio_legacy::AudioSystem::MODE_IN_COMMUNICATION:
                if(mictype  == LADIN_Microphone2){
                    agc_ul_gain = mHw->mVolumeController->SetApMicGain_HDRecord(VOIP_Headset_Mic);
                }
                else if(mictype == LADIN_SingleDigitalMic || mictype == LADIN_DualDigitalMic){
                    agc_ul_gain = mHw->mVolumeController->SetApMicGain_HDRecord(VOIP_Normal_Mic, true);
                }
                else{
                    if(mHw->GetSpeakerStatus ()){
                        agc_ul_gain = mHw->mVolumeController->SetApMicGain_HDRecord(VOIP_Handfree_Mic);
                    }
                    else{
                        agc_ul_gain = mHw->mVolumeController->SetApMicGain_HDRecord(VOIP_Normal_Mic);
                    }
                }
                break;
            default:
                ALOGD("no match mode?");
                agc_ul_gain = mHw->mVolumeController->SetApMicGain_HDRecord(Idle_Normal_Record);
                break;
            }
        }    

    return agc_ul_gain;
}

int AudioYusuApStreamIn::SetHdrecordingMode(int mode , bool *bStereoRecOn)
{
    if( mode == android_audio_legacy::AudioSystem::MODE_IN_CALL ||
            mode == android_audio_legacy::AudioSystem::MODE_IN_COMMUNICATION ){
        ALOGD("SetHdrecordingMode mode = %d return ",mode);
        return -1;
    }

    // HD Record
    uint8 modeIndex = 0;
    int32 u4SceneIdx = 0;

    if(!vGetHdRecordModeInfo(&modeIndex, bStereoRecOn)){
    //can not get match HD record mode, use the default one
        // check if 3rd party camcorder
        if (mInputSource != AUDIO_SOURCE_CAMCORDER) {
            //not camcorder
            if(mHw->GetApMicType() == LADIN_Microphone2){ //headset
                modeIndex = mhdRecordSceneTable.scene_table[NORMAL_RECORDING_DEFAULT_MODE][HD_REC_DEVICE_SOURCE_HEADSET];
            }
            else{   //default use internal one
                modeIndex = mhdRecordSceneTable.scene_table[NORMAL_RECORDING_DEFAULT_MODE][HD_REC_DEVICE_SOURCE_HANDSET];
            }
        }
        else {//camcoder
            u4SceneIdx = mhdRecordSceneTable.num_voice_rec_scenes + NORMAL_RECORDING_DEFAULT_MODE;//1:cts verifier offset
            if(mHw->GetApMicType() == LADIN_Microphone2){ //headset
                modeIndex = mhdRecordSceneTable.scene_table[u4SceneIdx][HD_REC_DEVICE_SOURCE_HEADSET];
            }
            else{   //default use internal one
                modeIndex = mhdRecordSceneTable.scene_table[u4SceneIdx][HD_REC_DEVICE_SOURCE_HANDSET];
            }
        }           
    }
    ALOGD("SetHdrecordingMode,modeIndex=%d,APMicType=%d",modeIndex,mHw->GetApMicType());
    return modeIndex;    
}

bool AudioYusuApStreamIn::vGetHdRecordModeInfo(uint8 *modeIndex, bool *bStereoRecOn)
{
    bool  ret = false;
    int32 u4SceneIdx = mHw->mHdRecScene;
    *bStereoRecOn = false;

    ALOGD("(RecOpen)+vGetHdRecordModeInfo: u4SceneIdx = %d", u4SceneIdx);
    if(mInputSource == AUDIO_SOURCE_VOICE_RECOGNITION){
        ALOGD("voice recognition case");
        u4SceneIdx=VOICE_REC_RECORDING_DEFAULT_MODE;
    }

    if(u4SceneIdx >= 0)
    {        
        // get mode index
        uint32 u4CurrentDevice = mHw->Audio_Match_Input_device(mHw->mInputAvailable, true);

        if (mhdRecordSceneTable.scene_table[u4SceneIdx][HD_REC_DEVICE_SOURCE_HEADSET] != 0xFF
            && u4CurrentDevice == android_audio_legacy::AudioSystem::DEVICE_IN_WIRED_HEADSET)
        {
            *modeIndex = mhdRecordSceneTable.scene_table[u4SceneIdx][HD_REC_DEVICE_SOURCE_HEADSET];
            ALOGD("(RecOpen)vGetHdRecordModeInfo: HEADSET,  modeIndex = %d", *modeIndex);
        }
        // Handset Mic
        else if (mhdRecordSceneTable.scene_table[u4SceneIdx][HD_REC_DEVICE_SOURCE_HANDSET] != 0xFF)
        {
            *modeIndex = mhdRecordSceneTable.scene_table[u4SceneIdx][HD_REC_DEVICE_SOURCE_HANDSET];
            ALOGD("(RecOpen)vGetHdRecordModeInfo: HANDSET,  modeIndex = %d", *modeIndex);
#if defined(MTK_DUAL_MIC_SUPPORT)
            /* only stereo flag is true, the stereo record is enabled */
            if(mhdRecordParam.hd_rec_map_to_stereo_flag[*modeIndex]!=0){
                *bStereoRecOn = true;
            }
#endif
        }
        else {
            ALOGD("(RecOpen)vGetHdRecordModeInfo: Handset mode index shoule not be -1");
        }

#if 1 // Debug print
        ALOGD("(RecOpen)vGetHdRecordModeInfo: map_fir_ch1=%d, map_fir_ch2=%d, device_mode=%d",
            mhdRecordParam.hd_rec_map_to_fir_for_ch1[*modeIndex],
            mhdRecordParam.hd_rec_map_to_fir_for_ch2[*modeIndex],
            mhdRecordParam.hd_rec_map_to_dev_mode[*modeIndex]);
#endif

        ret = true;
    }else{
        *modeIndex = 0;
        ret = false;
    }
    
    ALOGD("(RecOpen)-vGetHdRecordModeInfo: ENUM_HD_Record_Mode = %d, bStereoRecOn = %d", *modeIndex, *bStereoRecOn);
    return ret;
}

void AudioYusuApStreamIn::LoadHdRecordParams(void)
{
    uint8 modeIndex;
    uint8 total_num_scenes = MAX_HD_REC_SCENES;
    ALOGD("LoadHdRecordParams");
    // get scene table
    GetHdRecordSceneTableFromNV(&mhdRecordSceneTable);

    // get hd rec param
    GetHdRecordParamFromNV(&mhdRecordParam);

    for(int i=0; i<total_num_scenes; i++){
        // Handset
        modeIndex = mhdRecordSceneTable.scene_table[i][HD_REC_DEVICE_SOURCE_HANDSET];
        if (modeIndex != 0xFF) {
            mhdRecordParam.hd_rec_map_to_fir_for_ch1[modeIndex] = SPH_FIR_COEFF_NORMAL;
#if defined(MTK_DUAL_MIC_SUPPORT)            
            mhdRecordParam.hd_rec_map_to_fir_for_ch2[modeIndex] = SPH_FIR_COEFF_HANDSET_MIC2;
#else       
            mhdRecordParam.hd_rec_map_to_fir_for_ch2[modeIndex] = SPH_FIR_COEFF_NORMAL;
#endif            
            mhdRecordParam.hd_rec_map_to_dev_mode[modeIndex] = SPH_MODE_NORMAL;
        }

        // Headset
        modeIndex = mhdRecordSceneTable.scene_table[i][HD_REC_DEVICE_SOURCE_HEADSET];
        if (modeIndex != 0xFF) {
            mhdRecordParam.hd_rec_map_to_fir_for_ch1[modeIndex] = SPH_FIR_COEFF_HEADSET;
            mhdRecordParam.hd_rec_map_to_fir_for_ch2[modeIndex] = SPH_FIR_COEFF_HEADSET;
            mhdRecordParam.hd_rec_map_to_dev_mode[modeIndex] = SPH_MODE_EARPHONE;
        }

    }
    // specific for voice recognitaion
    
    // fix me , need hardware core for voice rec??
    int i=VOICE_REC_RECORDING_DEFAULT_MODE;
    // Handset
    modeIndex = mhdRecordSceneTable.scene_table[i][HD_REC_DEVICE_SOURCE_HANDSET];
    ALOGD("modeIndex = %d",modeIndex);
    if (modeIndex != 0xFF) {
        mhdRecordParam.hd_rec_map_to_fir_for_ch1[modeIndex] = SPH_FIR_COEFF_VOICE_REC;
        mhdRecordParam.hd_rec_map_to_fir_for_ch2[modeIndex] = SPH_FIR_COEFF_VOICE_REC;

        mhdRecordParam.hd_rec_map_to_dev_mode[modeIndex] = SPH_MODE_NORMAL;
    }

    // Headset
    modeIndex = mhdRecordSceneTable.scene_table[i][HD_REC_DEVICE_SOURCE_HEADSET];
    if (modeIndex != 0xFF) {
        mhdRecordParam.hd_rec_map_to_fir_for_ch1[modeIndex] = SPH_FIR_COEFF_VOICE_REC;
        mhdRecordParam.hd_rec_map_to_fir_for_ch2[modeIndex] = SPH_FIR_COEFF_VOICE_REC;
        mhdRecordParam.hd_rec_map_to_dev_mode[modeIndex] = SPH_MODE_EARPHONE;
    }
    
#if 1 // Debug print
        for(int i=0; i<total_num_scenes; i++)
            for(int j=0; j<NUM_HD_REC_DEVICE_SOURCE; j++)
                ALOGD("vGetHdRecordModeInfo, scene_table[%d][%d] = %d", i,j,mhdRecordSceneTable.scene_table[i][j]);
#endif
}

bool AudioYusuApStreamIn::UpdateSPEParams()
{
    bool ret = false;
    unsigned long HDRecordEnhanceParas[EnhanceParasNum]={0};
    short HDRecordCompenFilter[CompenFilterNum]={0};

    ALOGD("AudioYusuApStreamIn::UpdateSPEParams,mNeedStereo=%d",mNeedStereo);
    for(int i=0;i<EnhanceParasNum;i++)//EnhanceParasNum = 16+12(common parameters)
    {
        if(i<SPEECH_PARA_NUM){        
            //specific parameters
            HDRecordEnhanceParas[i]= mhdRecordParam.hd_rec_speech_mode_para[mHDRecordModeIndex][i];
        }
        else{
            //common parameters
            HDRecordEnhanceParas[i]= HDRecordEnhanceParasCommon[i-SPEECH_PARA_NUM];
        }
        ALOGD("HDRecordEnhanceParas[%d]=%d",i,HDRecordEnhanceParas[i]);
    }

#ifdef ALL_USING_AP_ENHANCEMENT
     //FIR filter param get from NVRam, 48K and 16K use the same params
    for(int i=0; i<WB_FIR_NUM; i++)
    {
        HDRecordCompenFilter[i]= mhdRecordParam.hd_rec_fir[mhdRecordParam.hd_rec_map_to_fir_for_ch1[mHDRecordModeIndex]][i];
        ALOGD("HDRecordCompenFilter[%d]=%d",i,HDRecordCompenFilter[i]);
        if(mNeedStereo) //stereo, DL2 use different FIR filter
            HDRecordCompenFilter[i+WB_FIR_NUM]= mhdRecordParam.hd_rec_fir[mhdRecordParam.hd_rec_map_to_fir_for_ch2[mHDRecordModeIndex]][i];
        else    //mono, DL2 use the same FIR filter
            HDRecordCompenFilter[i+WB_FIR_NUM]= mhdRecordParam.hd_rec_fir[mhdRecordParam.hd_rec_map_to_fir_for_ch1[mHDRecordModeIndex]][i];
    }

    mHw->mpSPELayer->SetCompFilter(SPE_MODE_REC,HDRecordCompenFilter);    
#else
    //48K,use default one, not from NVRam
    mHw->mpSPELayer->SetRecCompFilter(mhdRecordParam.hd_rec_map_to_fir_for_ch1[mHDRecordModeIndex],mhdRecordParam.hd_rec_map_to_fir_for_ch2[mHDRecordModeIndex],mNeedStereo);
#endif    
    
    mHw->mpSPELayer->SetEnhPara(SPE_MODE_REC,HDRecordEnhanceParas);    
    return ret;
}
#endif

bool AudioYusuApStreamIn::IsStereoMIC()
{
	ALOGD("IsStereoMIC GetApMicType=%d, mApInputSource=%d",mHw->GetApMicType(),mApInputSource);
    return((mApInputSource==LADIN_DualAnalogMic) ||(mApInputSource==LADIN_DualDigitalMic));   
}

#ifdef MTK_DUAL_MIC_SUPPORT
void AudioYusuApStreamIn::CheckNeedDataSwitch(short *buffer , int length)
{
    short left , right;
    int copysize = length>>2;
        
    char value[PROPERTY_VALUE_MAX];
    property_get("streamin.micchoose", value, "0");
    int bflag = atoi(value);
    
    char value1[PROPERTY_VALUE_MAX];
    property_get("streamin.LRSwitch", value1, "0");
    int bflag1 = atoi(value1);
    
    //ALOGD("mbLRChannelSwitch=%d, miSpecificMicChoose=%d,bflag %d, bflag1 %d",mbLRChannelSwitch,miSpecificMicChoose,bflag,bflag1);
    
    if(miSpecificMicChoose==1 || bflag==1)    //use main MIC
    {
        while(copysize){
        left = *(buffer);
        *(buffer) = left;
        *(buffer+1) = left;
        buffer+=2;
        copysize--;
        }
    }
    else if(miSpecificMicChoose==2 || bflag==2)   //use ref MIC
    {
        while(copysize){
        right = *(buffer+1);
        *(buffer) = right;
        *(buffer+1) = right;
        buffer+=2;
        copysize--;
        }
    }

    if((bflag1||mbLRChannelSwitch) && (miSpecificMicChoose==0 && bflag!=1&& bflag!=2))   //channel switch
    {
        while(copysize){
        left = *(buffer);
        right = *(buffer+1);
        *(buffer) = right;
        *(buffer+1) = left;
        buffer+=2;
        copysize--;
        }
    }    
}
#endif    

// ----------------------------------------------------------------------------
}

