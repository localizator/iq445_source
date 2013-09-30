/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2009
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/


/*******************************************************************************
 *
 * Filename:
 * ---------
 * AudioYusuStreamHandler.cpp
 *
 * Project:
 * --------
 *   Yusu
 *
 * Description:
 * ------------
 *   class and typedef of stream
 *
 * Author:
 * -------
 *   ChiPeng Chang (mtk02308)
 *
 *
 *------------------------------------------------------------------------------
 * $Revision: #24 $
 * $Modtime:$
 * $Log:$
 *
 * 04 10 2012 weiguo.li
 * [ALPS00266592] [Need Patch] [Volunteer Patch] ICS_MP patchback to ALPS.ICS of Audio
 * .
 *
 * 04 06 2012 weiguo.li
 * [ALPS00264069] A52ΪػٶȵŻ
 * .
 *
 * 02 13 2012 weiguo.li
 * [ALPS00233775] [mATV][ICS][Line in]Preview will freeze when mute/unmute
 * .
 *
 * 01 05 2012 weiguo.li
 * [ALPS00108538] [Need Patch] [Volunteer Patch]patch from GB2 to alpsDev4.0
 * .
 *
 * 11 17 2011 weiguo.li
 * [ALPS00090606] [WW FT][MT6575][Quanzhou][Overnight]Native (NE) in call
 * .
 *
 * 10 20 2011 weiguo.li
 * [ALPS00081607] [Need Patch] [Volunteer Patch]add headphone control
 * .
 *
 * 07 13 2010 chipeng.chang
 * [ALPS00121732][FactoryMode] Error in headset when enter it with headset plugged in
 * fix for factory mode .
 *
 * 07 07 2010 chipeng.chang
 * [ALPS00002905][Need Patch] [Volunteer Patch] add for input log
 * update for audio input logging.
 *
 * 06 11 2010 chipeng.chang
 * [ALPS00008012][Phone sound] The volume is very small and have noise.
 * when create track with 0 samplerate  , return hardware samplerate as track's samplerate.
 *
 * 06 09 2010 chipeng.chang
 * [ALPS00007771][Music] Have no sound when play imy file.
 * modify mode change open oudpsk sequence.
 *
 * 06 01 2010 chipeng.chang
 * [ALPS00002042][Patch rule]

If you fix the issue, please check-in your code and choose the correct type on the CR you use to check in BU Spec tab, thanks.


 * patch for I2S driver and add AudioyusuI2sTreamin for record
 *
 * 05 04 2010 chipeng.chang
 * [ALPS00001963][Need Patch] [Volunteer Patch] ALPS.10X.W10.11 Volunteer patch for
 * modify pvplayer audio when pause , it will read partial when clock is set to pause.
 *
 * 04 30 2010 chipeng.chang
 * [ALPS00005299][Call](PhoneApp)] It show Call not sent and  native exception
 * add mutex for Ladplyer let audiohardware don't delete it.
 *
 *******************************************************************************/
#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sched.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#define LOG_TAG "AudioStreamHandler"

#include <utils/Log.h>
#include <utils/String8.h>
#include <AudioYusuHardware.h>
#include <AudioYusuStreamHandler.h>
#include "AudioYusuDef.h"
#include "AudioYusuLadPlayer.h"
#include "AudioYusuLad.h"
#include "AudioStreamInHandler.h"

#ifdef ENABLE_LOG_STREAMHANDLER
    #define LOG_STREAMHANDLER ALOGD
#else
    #define LOG_STREAMHANDLER ALOGV
#endif

#define HEADPHONE_CONTROL

namespace android {
// ----------------------------------------------------------------------------

AudioAttribute::AudioAttribute()
{
    mFormat =0;
    mSampleRate=0;
    mChannelCount=0;
}

AudioAttribute::~AudioAttribute()
{
}

InputStreamAttribute::InputStreamAttribute()
{
    mFormat =0;
    mSampleRate=0;
    mChannelCount=0;
    mDevices =0;
    mAcoustics = android_audio_legacy::AudioSystem::AGC_DISABLE;
}

InputStreamAttribute::~InputStreamAttribute()
{
}


AudioStreamHandler::AudioStreamHandler(int Fd,AudioYusuHardware *hw, AudioAfe *Afehdr, AudioAnalog *AnaReg )
{
   int err=-1;
   mFd =0;
   for(int i=0;i<MAX_OUTPUT_STREAM_NUM ; i++)
      mOutput[i] = NULL;
   for(int i=0;i<MAX_INPUT_STREAM_NUM ; i++){
      mInput[i] = NULL;
      mI2SInput[i] = NULL;
   }
   mAudioHardware = hw;
   mSrcBlockOccupy = 0;
   mSrcBlockRunning = false;
   mFd = Fd;
   mAfe_handle = Afehdr;
   mAnaReg = AnaReg;
   mSrcBlockRunningNum = 0;
   mLadPlayerMode = 0;
   mI2SInstance = NULL;
   I2SClient = 0;

   pthread_mutex_init(&mInputstreamMutex, NULL);
   pthread_mutex_init(&mOutputstreamMutex, NULL);
   mI2SInstance = AudioI2S::getInstance ();
   I2SClient = mI2SInstance->open();
   LOG_STREAMHANDLER("AudioStreamHandler constructor ");
}

AudioStreamHandler::~AudioStreamHandler( )
{
   // Delete all input/output stream
   for(int i=0;i<MAX_OUTPUT_STREAM_NUM;i++)
   {
      if(mOutput != NULL){
         delete mOutput[i];
         mOutput[i] = NULL;
     	}
   }
   for(int i=0;i<MAX_INPUT_STREAM_NUM;i++)
   {
      if(mInput[i]!= NULL){
         delete mInput[i];
         mInput[i] = NULL;
      }
      if(mI2SInput[i]!= NULL){
         delete mI2SInput[i];
         mI2SInput[i] = NULL;
      }
   }
}

int AudioStreamHandler::FindFreeOutputStream()
{
   int i;
   for(i=0; i<MAX_OUTPUT_STREAM_NUM;i++)
   {
      if(mOutput[i] == NULL)
      	break;
   }
   if(i < MAX_OUTPUT_STREAM_NUM)
   	return i;
   else
   	return -1;
}

int AudioStreamHandler::FindMatchOutputStream(android_audio_legacy::AudioStreamOut*  out)
{
   int i = -1;
   for(i=0 ; i <  MAX_OUTPUT_STREAM_NUM ; i++)
   {
      if(mOutput[i] == out)
      	break;
   }
   if(i < MAX_OUTPUT_STREAM_NUM)
   	return i;
   else
   	return -1;
}

int AudioStreamHandler::FindFreeInputStream()
{
   int i = -1;
   for(i=0 ; i <  MAX_INPUT_STREAM_NUM ; i++)
   {
      if(mInput[i] == NULL)
      	break;
   }
   if(i < MAX_INPUT_STREAM_NUM)
   	return i;
   else
   	return -1;
}


int AudioStreamHandler::FindFreeI2SInputStream()
{
   int i = -1;
   for(i=0 ; i <  MAX_INPUT_STREAM_NUM ; i++)
   {
      if(mI2SInput[i] == NULL)
      	break;
   }
   if(i < MAX_INPUT_STREAM_NUM)
   	return i;
   else
   	return -1;
}


int AudioStreamHandler::FindMatchInputStream(android_audio_legacy::AudioStreamIn* in)
{
   int i;
   for(i=0 ; i <  MAX_INPUT_STREAM_NUM ; i++)
   {
      if(mInput[i] == in){
         break;
      }
   }
   if(i < MAX_INPUT_STREAM_NUM)
   	return i;
   else
   	return -1;
}

int AudioStreamHandler::FindMatchInputI2SStream(android_audio_legacy::AudioStreamIn* in)
{
   int i;
   for(i=0 ; i <  MAX_INPUT_STREAM_NUM ; i++)
   {
      if(mI2SInput[i] == in){
         break;
      }
   }
   if(i < MAX_INPUT_STREAM_NUM)
   	return i;
   else
   	return -1;
}

bool AudioStreamHandler::SetI2SControl(bool bEnable,int type,uint32 SampleRate)
{
    if(mI2SInstance == NULL){
        ALOGE("SetI2SControl mI2SInstance == NULL");
        return false;
    }
    if(bEnable == true){
        return  mI2SInstance->start (I2SClient, (I2STYPE)type, SampleRate);
    }
    else if (bEnable == false){
        return  mI2SInstance->stop (I2SClient, (I2STYPE)type);
    }
    return true;
}


AnalogAFE_Mux AudioStreamHandler::FindDeviceWithMux(int device)
{
    ALOGD("FindDeviceWithMux device = %d",device);
    if(device == android_audio_legacy::AudioSystem::DEVICE_OUT_EARPIECE){
        return VOICE_PATH;
    }
    else if(device ==android_audio_legacy::AudioSystem::DEVICE_OUT_SPEAKER || device == android_audio_legacy::AudioSystem::DEVICE_OUT_WIRED_HEADSET||
                   device == android_audio_legacy::AudioSystem::DEVICE_OUT_WIRED_HEADPHONE){
        return AUDIO_PATH;
    }
    else if(device==android_audio_legacy::AudioSystem::DEVICE_OUT_BLUETOOTH_SCO || device == android_audio_legacy::AudioSystem::DEVICE_OUT_BLUETOOTH_SCO_HEADSET ||
                     device  == android_audio_legacy::AudioSystem::DEVICE_OUT_BLUETOOTH_SCO_CARKIT){
        return AUDIO_PATH;
    }
    else{
        ALOGW("FindDeviceWithMux warning ");
        return AUDIO_PATH;
    }
}


void AudioStreamHandler::AudioStartRunning(unsigned int OutStreamIndex,unsigned int InterruptCounter)
{
   LOG_STREAMHANDLER("+AudioStartRunning InterruptCounter = %d",InterruptCounter);
   int mode =0, mCurrentDevice=0;
   float volume = mAudioHardware->mVolumeController->getMasterVolume();
   pthread_mutex_lock(&mAudioHardware->MasterVolumeMutex);
   int voice_status = mAudioHardware->GetVoiceStatus();
   int comunication_status = mAudioHardware->GetCommunicationStatus();
   mAudioHardware->getMode (&mode);
   mAudioHardware->mAnaReg->AnalogAFE_Request_ANA_CLK();
   //[ALPS073048]Hardly no sound in alarm after IPO
   if(voice_status == false){
      ::ioctl(mFd,AUDDRV_BEE_IOCTL,0x10);
      mAudioHardware->setMasterVolume(volume);//restore analog gain.
      UINT32 u4AnaCon_1,u4AnaCon_2;
      mAnaReg->GetAnaReg(AUDIO_CON1,&u4AnaCon_1);
      mAnaReg->GetAnaReg(AUDIO_CON2,&u4AnaCon_2);
      LOG_STREAMHANDLER("AudioStartRunning::Con1(0x%x),Con2(0x%x)",u4AnaCon_1,u4AnaCon_2);
   }
   SetSrcBlockRunning(true);

   LOG_STREAMHANDLER("AudioStartRunning, +Afe_DL_Start");

   // situation to open FM pinmux ,
   if( mAudioHardware->GetFmAnalogInEnable() && mode == android_audio_legacy::AudioSystem::MODE_NORMAL){
      //No Need to open audio hw, causing pop noise. (ALPS00237555)
      //mAnaReg->AnalogAFE_Depop(FM_STEREO_AUDIO,false);
      //mAnaReg->AnalogAFE_Depop(AUDIO_PATH,true);
   }
   // should open with COMMUNCATION mode , COMMUNICATION mode can open handset , headset.
   else if(mode == android_audio_legacy::AudioSystem::MODE_IN_COMMUNICATION){
      int device = mAudioHardware->GetCurrentDevice ();
      device = mAudioHardware->Audio_Find_Communcation_Output_Device(device);
      mAnaReg->AnalogAFE_Open(FindDeviceWithMux(device));
   }
   else{
	   int device = mAudioHardware->GetCurrentDevice ();
	   // do not open audiopath if earpiece is used in normal mode because voiepath is opened
	   if(device == android_audio_legacy::AudioSystem::DEVICE_OUT_EARPIECE &&
          (mode == android_audio_legacy::AudioSystem::MODE_NORMAL || mode == android_audio_legacy::AudioSystem::MODE_RINGTONE))
	   {
           LOG_STREAMHANDLER("AudioStartRunning,Open VOICE_PATH");
           mAnaReg->AnalogAFE_Open(VOICE_PATH);
	   }
	   else
	   {
           mAnaReg->AnalogAFE_Open(AUDIO_PATH);
	   }
   }
   /*
    * Speaker must be enabled after analog part has been started.
    */
   if((mode <= android_audio_legacy::AudioSystem::MODE_RINGTONE && (mAudioHardware->GetFmSpeakerStatus()&& mAudioHardware->GetFmRxStatus())) == false){
      mAudioHardware->EnableSpeaker();
   }
   else if(mode <= android_audio_legacy::AudioSystem::MODE_NORMAL && (mAudioHardware->GetFmSpeakerStatus()&& mAudioHardware->GetFmRxStatus())){
      mAudioHardware->SetFmSpeaker();
   }
   else{
      mAudioHardware->EnableSpeaker();
   }

#ifdef HEADPHONE_CONTROL
   if(!mAudioHardware->GetFmSpeakerStatus() ||(mAudioHardware->GetFmSpeakerStatus() && mAudioHardware->GetFmRxStatus()==false))
   {
      mAudioHardware->EnableHeadPhone(); //mark weiguo
   }
#endif

   usleep(100);

   mCurrentDevice = mAudioHardware->GetCurrentDevice() ;
   ALOGD("AudioStartRunning mCurrentDevice = 0x%x", mCurrentDevice);

   // Priority: HDMI > FM-Tx > Speaker
   if(mAudioHardware->GetHDMIAudioStatus() == true)
   {
      // use for HDMI case
      mAfe_handle->Afe_DL_Start(AFE_MODE_I2S1_OUT_HDMI);
   }
   else if(mCurrentDevice == android_audio_legacy::AudioSystem::DEVICE_OUT_AUX_DIGITAL)
   {
#ifdef FM_DIGITAL_OUT_SUPPORT
      mAfe_handle->Afe_DL_Start(AFE_MODE_I2S0_OUT);
#else
      mAfe_handle->Afe_DL_Start(AFE_MODE_DAC);
#endif
   }
   else if(comunication_status)
   {
       ALOGD("comunication_status  mAfe_handle->Afe_DL_Start();");
       int device = mAudioHardware->GetCurrentDevice ();
       if(device == android_audio_legacy::AudioSystem::DEVICE_OUT_BLUETOOTH_SCO){
           mAfe_handle->Afe_Set_Timer(IRQ1_MCU,InterruptCounter);
           mAfe_handle->Afe_DL_Start(AFE_MODE_DAI);
       }
       else{
           mAfe_handle->Afe_DL_Start(AFE_MODE_DAC);
       }
       mAudioHardware->setVoiceVolume(mAudioHardware->mVolumeController->getVoiceVolume ());
   }
  
   else if(mCurrentDevice==android_audio_legacy::AudioSystem::DEVICE_OUT_BLUETOOTH_SCO || 
   	       mCurrentDevice==android_audio_legacy::AudioSystem::DEVICE_OUT_BLUETOOTH_SCO_HEADSET ||
   	       mCurrentDevice==android_audio_legacy::AudioSystem::DEVICE_OUT_BLUETOOTH_SCO_CARKIT) //ccc skype
   {
	   ALOGD("AudioStartRunning DEVICE_OUT_BLUETOOTH_SCO  mAfe_handle->Afe_DL_Start();");
       mAfe_handle->Afe_Set_Timer(IRQ1_MCU,InterruptCounter);
       mAfe_handle->Afe_DL_Start(AFE_MODE_DAI);
   }
  
   else
   {
       #ifdef ENABLE_EXT_DAC
       ALOGD("ENABLE_EXT_DAC");
       mAfe_handle->Afe_DL_Start(AFE_MODE_I2S1_OUT);
       #endif
       mAfe_handle->Afe_DL_Start(AFE_MODE_DAC);
   }
//   mAfe_handle->Afe_DL_Unmute(AFE_MODE_DAC);
   mAudioHardware->mAnaReg->AnalogAFE_Release_ANA_CLK();
   pthread_mutex_unlock(&mAudioHardware->MasterVolumeMutex);

   ::ioctl(mFd,AUDDRV_BEE_IOCTL,0x11);
   int WaitForInterrupt = ((InterruptCounter*1000)+ (mOutput[0]->sampleRate()>>1) ) / mOutput[0]->sampleRate();
   usleep(WaitForInterrupt*1000);

   LOG_STREAMHANDLER("-AudioStartRunning");
}

void AudioStreamHandler::AudioStopRunning()
{
   int voice_status, mCurrentDevice=0,mPreViousDevice =0;
   LOG_STREAMHANDLER("!! AudioStopRunning");
   Mutex::Autolock _l(mLock);
   mAfe_handle->Afe_DL_Mute(AFE_MODE_DAC);
   usleep(10*1000);
   voice_status = mAudioHardware->GetVoiceStatus();
   pthread_mutex_lock(&mAudioHardware->MasterVolumeMutex);

   float mVolume = mAudioHardware->mVolumeController->getMasterVolume();
/* //Move to AnalogAFE_Close
   if(voice_status == false){
      mAudioHardware->setMasterVolume(0);
   }
*/
	::ioctl(mFd,AUDDRV_BEE_IOCTL,0x30);
   SetSrcBlockRunning(false);
   //Not to enable speaker while analog line-in is enabled to prevent pop noise.
   if(mAudioHardware->GetAnalogLineinEnable() == false)
   {
       mAudioHardware->EnableSpeaker();
#ifdef HEADPHONE_CONTROL
       mAudioHardware->EnableHeadPhone();
#endif
   }
   mAnaReg->AnalogAFE_Close(AUDIO_PATH);

   mCurrentDevice  = mAudioHardware->GetCurrentDevice() ;
   mPreViousDevice = mAudioHardware->GetPreviousDevice();
   /*
   ALOGD("AudioStopRunning mCurrentDevice = 0x%x mPreViousDevice = %x prestate = %d",
       mCurrentDevice,mPreViousDevice,mAudioHardware->getPreviousMode());*/

   //if((mAudioHardware->getPreviousMode() == android_audio_legacy::AudioSystem::MODE_IN_COMMUNICATION) &&
   if(((mAudioHardware->getPreviousMode() == android_audio_legacy::AudioSystem::MODE_IN_COMMUNICATION) || (mAudioHardware->getPreviousMode() == android_audio_legacy::AudioSystem::MODE_NORMAL)) &&   //ccc skype
          (mPreViousDevice == android_audio_legacy::AudioSystem::DEVICE_OUT_BLUETOOTH_SCO_HEADSET || mPreViousDevice == android_audio_legacy::AudioSystem::DEVICE_OUT_BLUETOOTH_SCO
           || mPreViousDevice == android_audio_legacy::AudioSystem::DEVICE_OUT_BLUETOOTH_SCO_CARKIT)&&(!voice_status)){
          ALOGD("AudioStopRunning DEVICE_OUT_BLUETOOTH_SCO mAfe_handle->Afe_DL_Stop(AFE_MODE_DAI)");
        mAfe_handle->Afe_DL_Stop(AFE_MODE_DAI);
   }

   // Priority: HDMI > FM-Tx > Speaker
   if(mAudioHardware->GetHDMIAudioStatus() == true)
   {
      // use for HDMI case
      mAfe_handle->Afe_DL_Stop(AFE_MODE_I2S1_OUT_HDMI);
   }
   else if(mCurrentDevice == android_audio_legacy::AudioSystem::DEVICE_OUT_AUX_DIGITAL)
   {
#ifdef FM_DIGITAL_OUT_SUPPORT
      mAfe_handle->Afe_DL_Stop(AFE_MODE_I2S0_OUT);
#else
      mAfe_handle->Afe_DL_Stop(AFE_MODE_DAC);
#endif
   }
   else
   {
       #ifdef ENABLE_EXT_DAC
       ALOGD("ENABLE_EXT_DAC");
       mAfe_handle->Afe_DL_Stop(AFE_MODE_I2S1_OUT);
       #endif
       mAfe_handle->Afe_DL_Stop(AFE_MODE_DAC);
   }
   mAfe_handle->Afe_DL_Unmute(AFE_MODE_DAC);
/* //Move to AnalogAFE_Close
   if(voice_status == false){
      mAudioHardware->setMasterVolume(mVolume);
   }
*/
   pthread_mutex_unlock(&mAudioHardware->MasterVolumeMutex);
	::ioctl(mFd,AUDDRV_BEE_IOCTL,0x31);
}

void AudioStreamHandler::SetSrcBlockRunning(bool Running)
{
   mSrcBlockRunning = Running;
}

bool AudioStreamHandler::GetSrcBlockRunning(void)
{
   return mSrcBlockRunning;
}

void AudioStreamHandler::AddSrcRunningNumber(void)
{
   mSrcBlockRunningNum++;
   ALOGD("AddSrcRunningNumber mSrcBlockRunningNum:%d",mSrcBlockRunningNum);
}

void AudioStreamHandler::MinusSrcRunningNumber(void)
{
   mSrcBlockRunningNum--;
   ALOGD("MinusSrcRunningNumber mSrcBlockRunningNum:%d",mSrcBlockRunningNum);
}

int AudioStreamHandler::GetSrcRunningNumber(void)
{
   ALOGD("GetSrcRunningNumber, mSrcBlockRunningNum:%d",mSrcBlockRunningNum);
   return mSrcBlockRunningNum;
}

android_audio_legacy::AudioStreamOut* AudioStreamHandler::openOutputStream(
	uint32 devices,int *format, uint32_t *channelCount, uint32_t *sampleRate, status_t *status)
{
   int Stream_index = this->FindFreeOutputStream();
   ALOGD("openOutputStream ");
   // find a free stream
   if(Stream_index < 0 || Stream_index >= MAX_OUTPUT_STREAM_NUM){
      LOG_STREAMHANDLER("openOutputStream, no free stream available");
      return NULL;
   }

   // open and init an output stream
   AudioYusuStreamOut* out = new AudioYusuStreamOut();
   if(out->InitOutStream(Stream_index,mFd)!= true)
      LOG_STREAMHANDLER("openOutputStream, initStream fail");

   // Check the Attribute
   if (*format == 0)
   	*format = android_audio_legacy::AudioSystem::PCM_16_BIT;
   if (*channelCount == 0)
   	*channelCount = android_audio_legacy::AudioSystem::CHANNEL_OUT_STEREO;
   if (*sampleRate == 0) {
      LOG_STREAMHANDLER("openOutputStream, set SR defauult(44100)");
   	*sampleRate = 44100;
   }

   ALOGD("openOutputStream, format=%d ,channels=%d, rate=%d",*format,*channelCount,*sampleRate);

   status_t lStatus = out->set(this->mAudioHardware, mFd,devices, format, channelCount, sampleRate);

   if(status)
   {
      *status = lStatus;
   }

   if(lStatus == NO_ERROR){
      mOutput[Stream_index] = out;
   }
   else{
      delete out;  // impossible
   }

   return mOutput[Stream_index];
}

void  AudioStreamHandler::closeOutputStream(android_audio_legacy::AudioStreamOut* out)
{
   int Stream_index = this->FindMatchOutputStream(out);
   LOG_STREAMHANDLER("closeOutputStream");
   if(Stream_index <0)
   {
      LOG_STREAMHANDLER("closeOutputStream, no match stream available");
      return;
   }
   if(out != null){
      delete out;
      out = NULL;
   }
}

void AudioStreamHandler::StoreInputStreamAttribute(uint32 devices,
                                        int *format,
                                        uint32_t *channelCount,
                                        uint32_t *sampleRate,
                                        android_audio_legacy::AudioSystem::audio_in_acoustics acoustics)
{
    ALOGD("StoreInputStreamAttribute (%d, %u, %u)", *format, *channelCount, *sampleRate);
    mInputAttribure[0].mSampleRate =*sampleRate;
    mInputAttribure[0].mFormat=*format;
    mInputAttribure[0].mChannelCount= *channelCount;
    mInputAttribure[0].mDevices = devices;
    mInputAttribure[0].mAcoustics= acoustics;
}

android_audio_legacy::AudioStreamIn *AudioStreamHandler::openInputStream(
        uint32  devices,int *format, uint32 *channelCount, uint32_t *sampleRate, status_t *status,
        android_audio_legacy::AudioSystem::audio_in_acoustics acoustics)
{
   int Stream_index = -1;
   ALOGD("+openInputStream devices = %x format = %d channelCount = %d samplerate = %d",
   devices,*format,*channelCount,*sampleRate);

   // handle for use digital input stream
   if(devices == android_audio_legacy::AudioSystem::DEVICE_IN_AUX_DIGITAL)
   {
      ALOGD("openInputStream devices == android_audio_legacy::AudioSystem::DEVICE_IN_AUX_DIGITAL");
      // no input available
      Stream_index = this->FindFreeInputStream();
      if(Stream_index < 0 || Stream_index >=MAX_INPUT_STREAM_NUM)
      {
         ALOGD("openInputStream :: no free stream available");
         return NULL;
      }
      // only one input stream allowed
      if (mI2SInput[Stream_index]) {
         if(status) {
            *status = INVALID_OPERATION;
         }
         return 0;
      }

      // create new Input stream
      AudioYusuI2SStreamIn* in = new AudioYusuI2SStreamIn();
      status_t lStatus = in->set(this->mAudioHardware, mFd, devices,format, channelCount, sampleRate, acoustics);
      if (status) {
         *status = lStatus;
      }

      if (lStatus == NO_ERROR) {
         mI2SInput[Stream_index] = in;
      }
      else {
         delete in;
      }
      return mI2SInput[Stream_index];
   }

   Stream_index = this->FindFreeInputStream();
   if(Stream_index < 0 || Stream_index >=MAX_INPUT_STREAM_NUM)
   {
      ALOGD("openInputStream :: no free stream available");
      return NULL;
   }

   // only one input stream allowed
   if(mInput[Stream_index])
   {
      if(status) {
         *status = INVALID_OPERATION;
      }
      return 0;
   }

   AudioStreamInInterface *in = NULL;
   status_t lStatus = NO_ERROR;

   // create new Input stream , base on samplerate and vm flag
   int mode = 0;
   mAudioHardware->getMode (&mode);
   in = new AudioStreamInHandler(this->mAudioHardware);
   lStatus = in->set(this->mAudioHardware, mFd, devices,format, channelCount, sampleRate, acoustics);
   ALOGD("set return lStatus = %d",lStatus);
   StoreInputStreamAttribute(devices,format, channelCount, sampleRate, acoustics);

   if(status){
      *status = lStatus;
   }
   if(lStatus == NO_ERROR){
     mInput[Stream_index] = in;
   }

   else{
     delete in;
     return 0;
   }

   ALOGD("-openInputStream, mInput create");
   return mInput[Stream_index];
}

void  AudioStreamHandler::closeInputStream(android_audio_legacy::AudioStreamIn* in)
{
   int Stream_index = this->FindMatchInputStream(in);
   ALOGD("closeInputStream:index=%x",Stream_index);
   if(Stream_index <0){
      ALOGD("closeInputStream:no match instream");
   }
   else{
      if(mInput[Stream_index] != NULL ){
         ALOGD("closeInputStream +Hdl lock");
         StreamHandlerLock();//cr ALPS00090606
         delete mInput[Stream_index];
         mInput[Stream_index] = NULL;
         StreamHandlerUnLock();
         ALOGD("closeInputStream -Hdl lock");
         return;
      }
   }
   Stream_index = this->FindMatchInputI2SStream(in);
   if(Stream_index <0)
   {
      LOG_STREAMHANDLER("::closeInputStream :: no match I2Sinstream available");
   }
   else{
      if(mI2SInput[Stream_index] != NULL ){
         LOG_STREAMHANDLER("closeInputStream::delete mI2SInput");
         delete mI2SInput[Stream_index];
         mI2SInput[Stream_index] = NULL;
         return;
      }
   }
}


void AudioStreamHandler::SetLadPlayer(int mMode)
{
   LOG_STREAMHANDLER("+::SetLadPlayer(mMode=%d)",mMode);
   pthread_mutex_lock(&mAudioHardware->LadMutex);
   pthread_mutex_lock(&mAudioHardware->LadBufferMutex);
   mLadPlayerMode = mMode;  //record the mode of setplayer
   if ( mLadPlayerMode == android_audio_legacy::AudioSystem::MODE_IN_CALL)  // enter  mode is ringtone
   {
   // if background is close,open.
      if(mAudioHardware->GetBgsStatus() == false && GetSrcRunningNumber())
      {
         LOG_STREAMHANDLER("SetLadPlayer1, GetBgsStatus=0");
         for(int i=0; i<MAX_OUTPUT_STREAM_NUM; i++)
         {
            if(mOutput[i] != NULL)
            {
               mOutput[i]->InitLadplayerBuffer();
            }
         }
         mAudioHardware->SetBgsStatus(true);
         mAudioHardware->pLadPlayer->LADPlayer_SetBGSoundGain(-180,-6);
         mAudioHardware->pLadPlayer->LADPlayer_Open(mLadPlayerMode);
      }
   }
   pthread_mutex_unlock(&mAudioHardware->LadBufferMutex);
   pthread_mutex_unlock(&mAudioHardware->LadMutex);
   LOG_STREAMHANDLER("-::SetLadPlayer(mMode=%d)",mMode);
}

void AudioStreamHandler::SetLadPlayer()
{
//   LOG_STREAMHANDLER("+::SetLadPlayer(mLadPlayerMode = %d)  ",mLadPlayerMode);
   pthread_mutex_lock( &mAudioHardware->LadMutex);
   pthread_mutex_lock( &mAudioHardware->LadBufferMutex);
   if ( mLadPlayerMode == android_audio_legacy::AudioSystem::MODE_IN_CALL)  // enter  mode is ringtone
   {
      // if background is close,open.
      if(mAudioHardware->GetBgsStatus() == false && GetSrcRunningNumber())
      {
         LOG_STREAMHANDLER("SetLadPlayer, GetBgsStatus:0");
         for(int i=0 ; i <  MAX_OUTPUT_STREAM_NUM ; i++)
         {
            if(mOutput[i] != NULL){
            	mOutput[i]->InitLadplayerBuffer();
            }
         }
         mAudioHardware->mForceBGSoff = true;
         mAudioHardware->SetBgsStatus(true);
         mAudioHardware->pLadPlayer->LADPlayer_SetBGSoundGain(-180,-6);
         mAudioHardware->pLadPlayer->LADPlayer_Open(mLadPlayerMode);
      }
   }
   pthread_mutex_unlock( &mAudioHardware->LadBufferMutex);
   pthread_mutex_unlock( &mAudioHardware->LadMutex);
//   LOG_STREAMHANDLER("-::SetLadPlayer(mLadPlayerMode = %d)  ",mLadPlayerMode);
}

void AudioStreamHandler::ForceSetLadPlayer()
{
   LOG_STREAMHANDLER("::ForceSetLadPlayer(mLadPlayerMode=%d) ",mLadPlayerMode);
   pthread_mutex_lock( &mAudioHardware->LadMutex);
   pthread_mutex_lock( &mAudioHardware->LadBufferMutex);
   if ( mLadPlayerMode == android_audio_legacy::AudioSystem::MODE_IN_CALL)  // enter  mode is ringtone
   {
      // if background is close,open.
      if(mAudioHardware->GetBgsStatus() == false && GetSrcRunningNumber())
      {
         LOG_STREAMHANDLER("ForceSetLadPlayer, GetBgsStatus:0");
         for(int i=0 ; i <  MAX_OUTPUT_STREAM_NUM ; i++)
         {
            if(mOutput[i] != NULL){
            	mOutput[i]->InitLadplayerBuffer();
            }
         }

         mAudioHardware->SetBgsStatus(true);
         mAudioHardware->pLadPlayer->LADPlayer_SetBGSoundGain(-180,-6);
         mAudioHardware->pLadPlayer->LADPlayer_Open(mLadPlayerMode);
      }
   }
   pthread_mutex_unlock( &mAudioHardware->LadBufferMutex);
   pthread_mutex_unlock( &mAudioHardware->LadMutex);
}


void AudioStreamHandler::ResetLadPlayer(int mMode)
{
   LOG_STREAMHANDLER("+::ResetLadPlayer(mMode=%d, mLadPlayerMode=%d)",mMode,mLadPlayerMode);
   pthread_mutex_lock( &mAudioHardware->LadMutex);
   pthread_mutex_lock( &mAudioHardware->LadBufferMutex);

   LOG_STREAMHANDLER("::ResetLadPlayer(GetBgsStatus=%d, GetSrcRunningNumber=%d)",mAudioHardware->GetBgsStatus(),GetSrcRunningNumber());
   if(mLadPlayerMode==android_audio_legacy::AudioSystem::MODE_IN_CALL && mMode!=mLadPlayerMode )
   {
      // if background is open ,close.
      if(mAudioHardware->GetBgsStatus() == true  && GetSrcRunningNumber())
      {
         LOG_STREAMHANDLER("ResetLadPlayer1, GetBgsStatus=1");
         mAudioHardware->pLadPlayer->LADPlayer_Close();
         mAudioHardware->SetBgsStatus(false);
         for(int i=0;i<MAX_OUTPUT_STREAM_NUM;i++)
         {
            if(mOutput[i] != NULL)
            {
               mOutput[i]->DeinitLadplayerBuffer();
            }
         }
      }
   }
   mLadPlayerMode = mMode;  //record the mode of setplayer
   pthread_mutex_unlock( &mAudioHardware->LadBufferMutex);
   pthread_mutex_unlock( &mAudioHardware->LadMutex);
   LOG_STREAMHANDLER("-::ResetLadPlayer(mMode=%d, mLadPlayerMode=%d)",mMode,mLadPlayerMode);
}

void AudioStreamHandler::ResetLadPlayer()
{
   LOG_STREAMHANDLER("+::ResetLadPlayer(mLadPlayerMode=%d)  ",mLadPlayerMode);
   pthread_mutex_lock( &mAudioHardware->LadMutex);
   pthread_mutex_lock( &mAudioHardware->LadBufferMutex);
   if(mLadPlayerMode == android_audio_legacy::AudioSystem::MODE_IN_CALL)
   {
      // if background is open ,close, GetSrcRunningNumber is little tricky , in Audioyusustreamout.cpp , I minus SrcRunningNumber
      // and call ResetLadPlayer
      if(mAudioHardware->GetBgsStatus() == true && GetSrcRunningNumber()== 0 ){
         LOG_STREAMHANDLER("ResetLadPlayer, GetBgsStatus=1");
         mAudioHardware->pLadPlayer->LADPlayer_Close();
         mAudioHardware->SetBgsStatus(false);
         for(int i=0 ; i <  MAX_OUTPUT_STREAM_NUM ; i++){
            if(mOutput[i] != NULL){
               mOutput[i]->DeinitLadplayerBuffer();
            }
         }
      }
   }
   pthread_mutex_unlock( &mAudioHardware->LadBufferMutex);
   pthread_mutex_unlock( &mAudioHardware->LadMutex);
   LOG_STREAMHANDLER("-::ResetLadPlayer(mLadPlayerMode=%d)  ",mLadPlayerMode);
}

void AudioStreamHandler::ForceResetLadPlayer(int mMode)
{
   LOG_STREAMHANDLER("+::ForceResetLadPlayer(mLadPlayerMode=%d, mMode=%d)",mLadPlayerMode,mMode);
   pthread_mutex_lock( &mAudioHardware->LadMutex);
   pthread_mutex_lock( &mAudioHardware->LadBufferMutex);

   if(mLadPlayerMode == android_audio_legacy::AudioSystem::MODE_IN_CALL)
   {
      // if background is open ,close, GetSrcRunningNumber is little tricky , in Audioyusustreamout.cpp , I minus SrcRunningNumber
      // and call ResetLadPlayer
      if(mAudioHardware->GetBgsStatus() == true && GetSrcRunningNumber() ){
         LOG_STREAMHANDLER("ForceResetLadPlayer, GetBgsStatus=1");
         mAudioHardware->pLadPlayer->LADPlayer_Close();
         mAudioHardware->SetBgsStatus(false);
         for(int i=0 ; i <  MAX_OUTPUT_STREAM_NUM ; i++){
            if(mOutput[i] != NULL){
               mOutput[i]->DeinitLadplayerBuffer();
            }
         }
      }
   }

   mLadPlayerMode = mMode;  //record the mode of setplayer
   pthread_mutex_unlock( &mAudioHardware->LadBufferMutex);
   pthread_mutex_unlock( &mAudioHardware->LadMutex);
   LOG_STREAMHANDLER("-::ForceResetLadPlayer(mLadPlayerMode=%d)  ",mLadPlayerMode);
}

bool AudioStreamHandler::IsRecordOn()
{
   for(int i=0;i<MAX_INPUT_STREAM_NUM;i++)
   {
      if(mInput[i] != NULL ){
         if(mInput[i]->IsStandby () == false)
    		return true;
      }
   }
   return false;
}

bool AudioStreamHandler::RecordClose(void)
{
    ALOGD("RecordClose");
    bool result = false;
    for(int i=0;i<MAX_INPUT_STREAM_NUM;i++){
        // have inputstream and not instandby
        if(mInput[i] ){
            mInput[i]->RecClose ();
            result = true;
        }
    }
    return result;
}

bool AudioStreamHandler::RecordOpen(void)
{
    ALOGD("RecordOpen");
    bool result = false;
    for(int i=0;i<MAX_INPUT_STREAM_NUM;i++){
        // have inputstream and in standby
        if(mInput[i] ){
            mInput[i]->RecOpen ();
            result = true;
        }
    }
    return result;
}

bool AudioStreamHandler::SetCloseRec(int mPreviousMode,int mMode)
{
   ALOGD("::SetCloseRec mPreviousMode=%d, mMode=%d",mPreviousMode,mMode);
   return RecordClose();
}

bool AudioStreamHandler::SetOpenRec(int mPreviousMode,int mMode)
{
   ALOGD("::SetOpenRec mPreviousMode = %d , mMode = %d",mPreviousMode,mMode);
   return RecordOpen();
}

/*
when Streamin is exist , need change input device base on inputsource.
*/
int AudioStreamHandler::SetMicType(int InputSource)
{
    if(mInput[0]){
        mInput[0]->SetStreamInputSource (InputSource);
    }
    return true;
}

void AudioStreamHandler::SetOutputSamplerate(uint32 samplerate)
{
   LOG_STREAMHANDLER("AudioStreamHandler::SetOutputSamplerate samplerate = %d",samplerate);
   int ret =0;
   for(int i=0; i< MAX_OUTPUT_STREAM_NUM ; i++)
   {
      if(mOutput[i]!= NULL)
      {
         ret = mOutput[i]->SetOutSamplerate(samplerate);
         if(ret <0)
            ALOGE("::SetOutputSamplerate ERROR!!!");
      }
   }
}

void AudioStreamHandler::StreamHandlerLock()
{
	mHandlerLock.lock();
}
 void AudioStreamHandler::StreamHandlerUnLock()
{
	mHandlerLock.unlock();
}

void AudioStreamHandler::InputStreamLock()
{
    if(mInput[0] != NULL){
        mInput[0]->MutexLock ();
    }
}

void AudioStreamHandler::InputStreamUnLock()
{
    if(mInput[0] != NULL){
        mInput[0]->MutexUnlock ();
    }
}

void AudioStreamHandler::OutputStreamLock()
{
    pthread_mutex_lock(&mOutputstreamMutex);
}

void AudioStreamHandler::OutputStreamUnLock()
{
    pthread_mutex_unlock(&mOutputstreamMutex);
}

void AudioStreamHandler::SetOutputStreamToBT(void)
{
    ALOGD("SetOutputStreamToBT");
    OutputStreamLock();
    uint32 BTSCO_SAMPLERATE = (mAfe_handle->mDaiBtMode==0) ? 8000 : 16000;
    if(GetSrcBlockRunning ()){
        this->AudioStopRunning ();
        ::ioctl(mFd,STANDBY_DL1_STREAM,0);
        usleep(1*1000);
        ioctl(mFd,START_DL1_STREAM,0);
        usleep(5*1000);
        for(int i=0; i <MAX_OUTPUT_STREAM_NUM ; i++){
            mOutput[i]->SetBTscoSamplerate (BTSCO_SAMPLERATE);
        }
        int interval = ASM_BUFFER_SIZE/3;  // ccc skype
        interval = interval >>2;
        this->AudioStartRunning (0,interval);
    }
    mOutput[0]->SetFillinZero (true);
    OutputStreamUnLock();
}

void AudioStreamHandler::RestoreOutputStream(void)
{
    ALOGD("RestoreOutputStream");
    OutputStreamLock();
    if(GetSrcBlockRunning ()){
        this->AudioStopRunning ();
       ::ioctl(mFd,STANDBY_DL1_STREAM,0);
        usleep(1*1000);
        ioctl(mFd,START_DL1_STREAM,0);
        usleep(5*1000);
        for(int i=0; i <MAX_OUTPUT_STREAM_NUM ; i++){
            mOutput[i]->RestoreOutSamplerate ();
        }
        int interval = mOutput[0]->GetInterrupttime ();
		ALOGD("RestoreOutputStream interval=%d\n",interval);
		mAfe_handle->Afe_Set_Timer(IRQ1_MCU,interval); //ccc skype
        this->AudioStartRunning (0,interval);
    }
    OutputStreamUnLock();
}

// ----------------------------------------------------------------------------
};

