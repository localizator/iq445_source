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
/*******************************************************************************
 *
 * Filename:
 * ---------
 *   AudioAfe.cpp
 *
 * Project:
 * --------
 *   Android Audio Driver
 *
 * Description:
 * ------------
 *   AFE register Control
 *
 * Author:
 * -------
 *   Stan Huang (mtk01728)
 *
 *------------------------------------------------------------------------------
 * $Revision: #5 $
 * $Modtime:$
 * $Log:$
 *
 * 04 18 2012 weiguo.li
 * [ALPS00263357] [HDMI pretest]Apps¡úPhone, press number keys continuously, sometimes no audio output on TV.
 * .
 *
 * 04 10 2012 weiguo.li
 * [ALPS00266592] [Need Patch] [Volunteer Patch] ICS_MP patchback to ALPS.ICS of Audio
 * .
 *
 *
 *******************************************************************************/

/*****************************************************************************
*                     C O M P I L E R   F L A G S
******************************************************************************
*/


/*****************************************************************************
*                E X T E R N A L   R E F E R E N C E S
******************************************************************************
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
#include <sys/ioctl.h>
#include <utils/Log.h>
#include <utils/String8.h>

#include "AudioYusuHardware.h"
#include <hardware_legacy/AudioHardwareInterface.h>
#include "AudioAfe.h"
#include "AudioAnalogAfe.h"
#include "AudioIoctl.h"
#include "AudioYusuDef.h"
#include "audio_custom_exp.h"
#include "AudioYusuStreamOut.h"
#include "AudioI2S.h"


#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG  "AudioAfe"
//#define ENABLE_LOG_AFE

#ifdef ENABLE_LOG_AFE
    #define LOG_AFE ALOGD
#else
    #define LOG_AFE ALOGV
#endif
/*****************************************************************************
*                          C O N S T A N T S
******************************************************************************
*/
static struct fm_i2s_setting Setting_FM;

/*****************************************************************************
*                        F U N C T I O N   D E F I N I T I O N
******************************************************************************
*/

namespace android {
bool AudioAfe::FirstInit = true;

static void FM_I2S_Setting()
{
    int fd_FM = -1, ret_FM = 0;

    fd_FM = open(FM_DEVICE_NAME, O_RDWR);
    if(fd_FM < 0){
        ALOGE("Open 'dev/fm' Fail !fd_FM(%d)", fd_FM);
    }
    else
    {
        ret_FM = ioctl(fd_FM, FM_IOCTL_I2S_SETTING, &Setting_FM);
        if(ret_FM){
            ALOGE("set ioctl FM_IOCTL_I2S_SETTING Fail !ret_FM(%d)", ret_FM);
        }
    }
    close(fd_FM);
}

AudioAfe::AudioAfe(AudioYusuHardware *hw)
{
   mFd = -1;
   mSamplingRate = 0;
   mCh           = 0;
   mFormat       = 0;
   mAudioHardware = hw;
   mAnalog        = new AudioAnalog(NULL);
   mAudioFmTx_thread = false;
   mAudioBuffer = NULL;
   mI2SInstance = NULL;
   mDaiBtMode = 0; // default 8k
   I2SClient = 0;

   return;
}

AudioAfe::~AudioAfe()
{
   return;
}

bool AudioAfe::Afe_Init(uint32 Fd)
{
   LOG_AFE("Afe_Init \n");
   mFd = Fd;
   if( mFd <0 )
   {
      LOG_AFE("AfeInit Fail \n");
      return false;
   }
   if(FirstInit == true){
       FirstInit = false;
       mI2SInstance = AudioI2S::getInstance ();
       if(mI2SInstance){
           I2SClient  = mI2SInstance->open ();
       }
   }
   return true;
}

bool AudioAfe::Afe_Deinit(void)
{
   LOG_AFE("Afe_Deinit \n");
   if( mFd <0 )
   {
      LOG_AFE("Afe_Deinit Fail \n");
      return false;
   }
   close(mFd);
   return true;
}

void AudioAfe::Afe_Set_Stream_Gain(uint32 gain)
{
   uint32 value = 0;
   DL2_GAIN(value,gain);
   SetAfeReg(AFE_DL_SRC2_2,value,0xffff0000);  // bit31~bit16 : DL2_GAIN
   SetAfeReg(AFE_DL_SRC2_1,0x2,0x2);    // --> DL2_GAIN_ON
   SetAfeReg(AFE_DL_SRC2_1,0x03000000,0x03000000);  // bit24~bit25:  DL2_OUTPUT_SEL    //up-sampleing x8   bit24 ~ bit25
   LOG_AFE("Afe_Set_Stream_Gain gain=%d \n",gain);
}

bool AudioAfe::Afe_Set_Stream_Format(uint32 *src, uint32 format)
{
    if(format == android_audio_legacy::AudioSystem::PCM_16_BIT){
       *src = format;
    }
    else{
       LOG_AFE("Afe_Set_Stream_Format error format=%d \n",format);
       return false;
    }
    return true;
}

bool AudioAfe::Afe_Set_Stream_Channel(uint32 *src, uint32 channelCount)
{
    if(channelCount ==android_audio_legacy::AudioSystem::CHANNEL_OUT_MONO){
       *src = channelCount;
    }
    else if (channelCount == android_audio_legacy::AudioSystem::CHANNEL_OUT_STEREO){
       *src = channelCount;
    }
    else{
        LOG_AFE("Afe_Set_Stream_Channel error ch=%d \n",channelCount);
       return false;
    }
    return true;
}

bool AudioAfe::Afe_Set_Stream_SampleRate(uint32 *src, uint32 sampleRate)
{
    switch(sampleRate)
    {
       case 8000:
       case 11025:
       case 12000:
       case 16000:
       case 22050:
       case 24000:
       case 32000:
       case 44100:
       case 48000:
           *src = sampleRate;
           break;
       default:
           LOG_AFE("AudioAfe::Afe_Set_Stream_SampleRate error sampleRate=%d \n",sampleRate);
           return false;
     }
     return true;
}

bool AudioAfe::Afe_Set_Stream_Attribute(uint32 format,uint32 channelCount,uint32 sampleRate)
{
   uint32 value = 0;
   bool result = true;
   LOG_AFE("Afe_Set_Stream_Attribute f:%d, ch=%d, SR=%d \n", format, channelCount, sampleRate);

   if(format == android_audio_legacy::AudioSystem::PCM_16_BIT){
      mFormat = format;
   }
   else{
      ALOGE("Afe_Set_Stream_Attribute error format=%d \n", format);
      result = false;
   }

   if(channelCount ==android_audio_legacy::AudioSystem::CHANNEL_OUT_MONO){
      mCh = channelCount;
   }
   else if (channelCount == android_audio_legacy::AudioSystem::CHANNEL_OUT_STEREO){
      mCh = channelCount;
   }
   else{
  	  ALOGE("Afe_Set_Stream_Attribute error ch=%d \n", channelCount);
      result = false;
   }

   switch(sampleRate)
   {
      case 8000:
      case 11025:
      case 12000:
      case 16000:
      case 22050:
      case 24000:
      case 32000:
      case 44100:
      case 48000:
         mSamplingRate = sampleRate;
        	break;
      default:
         ALOGE("AudioAfe::Afe_Set_Stream_Attribute error sampleRate=%d \n", sampleRate);
         result = false;
    }
    return result;
}

bool AudioAfe::Afe_Set_Stream_Attribute(uint32 format,uint32 channelCount,uint32 sampleRate, AFE_MODE Open_Mode)
{
    LOG_AFE("Afe_Set_Stream_Attribute f:%d, ch=%d, SR=%d \n",format,channelCount,sampleRate,AFE_MODE_AWB);
    switch(Open_Mode){
        case AFE_MODE_DAC:
        {
            Afe_Set_Stream_Format(&mFormat,format);
            Afe_Set_Stream_Channel (&mCh,channelCount);
            Afe_Set_Stream_SampleRate(&mSamplingRate,sampleRate);
            break;
        }
        case AFE_MODE_I2S0_OUT:
        {
            Afe_Set_Stream_Format(&I2S0output_Stream.Format,format);
            Afe_Set_Stream_Channel (&I2S0output_Stream.Channel,channelCount);
            Afe_Set_Stream_SampleRate(&I2S0output_Stream.SampleRate,sampleRate);
            break;
        }
        case AFE_MODE_I2S1_OUT:
        {
            Afe_Set_Stream_Format(&I2S1output_Stream.Format,format);
            Afe_Set_Stream_Channel (&I2S1output_Stream.Channel,channelCount);
            Afe_Set_Stream_SampleRate(&I2S1output_Stream.SampleRate,sampleRate);
            break;
        }
        case AFE_MODE_ADC:
        {
            Afe_Set_Stream_Format(&ADC_Stream.Format,format);
            Afe_Set_Stream_Channel (&ADC_Stream.Channel,channelCount);
            Afe_Set_Stream_SampleRate(&ADC_Stream.SampleRate,sampleRate);
            break;
        }
        case AFE_MODE_AWB:
        {
            Afe_Set_Stream_Format(&AWB_Stream.Format,format);
            Afe_Set_Stream_Channel (&AWB_Stream.Channel,channelCount);
            Afe_Set_Stream_SampleRate(&AWB_Stream.SampleRate,sampleRate);
            break;
        }
        case AFE_MODE_I2S_IN:
        {
            Afe_Set_Stream_Format(&I2Sinput_Stream.Format,format);
            Afe_Set_Stream_Channel (&I2Sinput_Stream.Channel,channelCount);
            Afe_Set_Stream_SampleRate(&I2Sinput_Stream.SampleRate,sampleRate);
        }
        default:
            ALOGW("Afe_Set_Stream_Attribute default");
    }
     return true;
}

void AudioAfe::SetAfeReg(uint32 offset,uint32 value, uint32 mask)
{
   LOG_AFE("SetAfeReg offset=%x, value=%x, mask=%x \n",offset,value,mask);

   if( offset > AFE_REGION )
   {
      LOG_AFE("SetAfeReg offset > AFE_REGION(0x3CC) \n");
      return;
   }
   Register_Control Reg_Data;
   Reg_Data.offset = offset;
   Reg_Data.value = value;
   Reg_Data.mask = mask;

   ::ioctl(mFd,SET_AUDSYS_REG,&Reg_Data);
}

void AudioAfe::GetAfeReg(uint32 offset,uint32 *value)
{
   if( offset > AFE_REGION )
   {
   	//LOG_AFE("GetAfeReg offset > AFE_REGION \n");
   	return;
   }
   Register_Control Reg_Data;
   Reg_Data.offset = offset;
   Reg_Data.value = 0;
   Reg_Data.mask = 0xffff;
   ::ioctl(mFd,GET_AUDSYS_REG,&Reg_Data);
   *value = Reg_Data.value;
   //LOG_AFE("GetAfeReg offset=%x, value=%x \n",offset,*value);
}

void AudioAfe::SetAWBChannel(int channels)
{
    if(channels == 1){
        SetAfeReg(AFE_DAC_CON1,1<<23,1<<23);
    }
    else if (channels == 2){
        SetAfeReg(AFE_DAC_CON1,0<<23,1<<23);
    }
    else{
        ALOGW("error SetAWBChannel");
    }
}


void AudioAfe::SetAWBConenction(bool Connect)
{
    ALOGV("SetAWBConenction Connect = %d",Connect);
    if(Connect){
        SetAfeReg(AFE_CONN2,0x84000000,0x84000000);

        /*
        SetAfeReg(AFE_CONN2,1<<27,1<<27);
        SetAfeReg(AFE_CONN3,1,1);
        */

    }
    else{
        /*
        SetAfeReg(AFE_CONN2,0,1<<27);
        SetAfeReg(AFE_CONN3,0,1);
        */

        SetAfeReg(AFE_CONN2,0x0,0x84000000);
    }
    //SetAfeReg(AFE_DAC_CON0,val,0x00000001);
}

void AudioAfe::SetAWBMemoryInterfaceEnable(bool Enable,uint32 bitmask)
{
    ALOGV("SetAWBEnable Enable = %d bitmask = %x",Enable,bitmask);
    uint32 val = 0;
    uint32 reg =0;
    GetAfeReg(AFE_DAC_CON0,&val);
    int afe_on = val & 0x01;

    if(Enable){
        if(afe_on){
            SetAfeReg(AFE_DAC_CON0,bitmask,bitmask);
        }
        else{
            SetAfeReg(AFE_DAC_CON0,bitmask|BIT_00,bitmask|BIT_00);
        }
    }
    else{
        val &= 0x1f;
        // check if other still in use
        if(val){
            SetAfeReg(AFE_DAC_CON0,0,bitmask);
        }
        else{
            // only AWB is enable close afe
            SetAfeReg(AFE_DAC_CON0,0,bitmask|BIT_00);
        }
    }
}

void AudioAfe::Afe_Set_Timer(uint IRQ_cnt,uint32 InterruptCounter)
{
   LOG_AFE("Afe_Set_Timer InterruptCounter=%d \n", InterruptCounter);
   uint32 value = 0x0;
   uint32  mask  = 0x0;
   uint32  SR_index = 0;
   uint32  SamplingRateConvert[9] = {0, 1, 2, 4, 5, 6, 8, 9, 10};
   // Set Sampling rate
   switch(mSamplingRate)
   {
      case 8000:
         SR_index = FS_8000HZ;
         break;
      case 11025:
         SR_index = FS_11025HZ;
         break;
      case 12000:
         SR_index = FS_12000HZ;
         break;
      case 16000:
         SR_index = FS_16000HZ;
         break;
      case 22050:
         SR_index = FS_22050HZ;
         break;
      case 24000:
         SR_index = FS_24000HZ;
         break;
      case 32000:
         SR_index = FS_32000HZ;
         break;
      case 44100:
         SR_index = FS_44100HZ;
         break;
      case 48000:
         SR_index = FS_48000HZ;
         break;
      default:
         LOG_AFE("Afe_Start error mSamplingRate=%d \n", mSamplingRate);
   }

   if(IRQ_cnt==IRQ1_MCU)
   {
      SR_index = SamplingRateConvert[SR_index];
      LOG_AFE("Afe_Set_Timer SR_index=%d \n", SR_index);
      value = 0;
      mask = 0;
      SET_IRQ1_MCU_MODE(value,SR_index);
      SET_IRQ1_MCU_MODE_mask(mask);
//      SetAfeReg(AFE_IRQ_CON,value,mask);     // bit7~bit4: Set IRQ1 mode (frequency)

      SetAfeReg(AFE_IRQ_CON, SR_index<<4, 0x00f0);     // bit7~bit4: Set IRQ1 mode (frequency)

      // Set timer (interrupt counter)
      SetAfeReg(AFE_IRQ_CNT1, InterruptCounter, 0xffffffff);  // bit17~bit0: AFE_IRQ_MCU_CNT1 (Set Timer)
   }
   else if(IRQ_cnt==IRQ2_MCU)
   {

   }
   else if(IRQ_cnt==IRQ3_MCU)
   {

   }
   else
   {

   }

}




void AudioAfe::Afe_Set_FmTx(int32 Enable, int32 DAC_Reopen)
{
#ifdef FM_DIGITAL_OUT_SUPPORT
        Afe_Set_FmTx_DigitalOut(Enable, DAC_Reopen);

#elif defined( FM_ANALOG_OUT_SUPPORT)
        Afe_Set_FmTx_AnalogOut(Enable, DAC_Reopen);

#else
        ALOGD("FM TX not supported!!!");

#endif
}

void AudioAfe::Afe_Set_FmTx_AnalogOut(int32 Enable, int32 DAC_Reopen)
{
    ALOGD("Afe_Set_FmTx_AnalogOut, Enable=%d \n",Enable);
    if(Enable >0){
    //set interconenct
    SetAfeReg(AFE_CONN1,(1<<27),(1<<27));  // Connect -- DL1_R to DAC_L, bit27: I06-O03
    SetAfeReg(AFE_CONN2,(1<<17),(1<<17));  // Connect -- DL1_L to DAC_R, bit17: I05-O04
    }
}

void AudioAfe::Afe_Set_FmTx_DigitalOut(int32 Enable, int32 DAC_Reopen)
{
#ifndef FMTX_SINEWAVE_TEST
    uint32 Reg, val, afe_on, i2s_on;
    ALOGD("Afe_Set_FmTx_DigitalOut, Enable=%d \n",Enable);
    if(Enable >0){
        SetAfeReg(AFE_CONN0,1<<22, 1<<22);  // Connect -- DL1_I_L to I2S_R,  bit5: I05-O01
        SetAfeReg(AFE_CONN0,1<<6, 1<<6);  // Connect -- DL1_I_R to I2S_L,  bit23: I06-O00

        // set I2S sampling rate
        GetAfeReg(AFE_DAC_CON1, &Reg);
        Reg = Reg & 0xF;

        if((Reg>=8)&&(Reg<=10))//32k, 44.1k, 48k
        {
            Setting_FM.sample = Reg - 8;
        }
        else
        {
            Setting_FM.sample = 1;//default 44.1k
        }
        Reg = Reg <<8;
        SetAfeReg(AFE_DAC_CON1, Reg, 0xf00);
        SetI2SControl (true,(I2STYPE)I2S0OUTPUT,mSamplingRate);

        //------------------------------------------------------------------
        // Enable Memory Interface I2S/AFE
        GetAfeReg(AFE_DAC_CON0,&val);
        afe_on = val & 0x01;
        i2s_on = val & 0x20;

        if(i2s_on == 0)
        {
           // Turn on I2S ON
           LOG_AFE("Afe_Set_FmTx, AFE_DAC_CON0, I2S ON \n");
           SetAfeReg(AFE_DAC_CON0, 0x20, 0x20);  //bit5: I2S ON
        }
        usleep(100);
        if(afe_on == 0)
        {
           // Turn on AFE_ON
           LOG_AFE("Afe_Set_FmTx, AFE_DAC_CON0, AFE ON \n");
           SetAfeReg(AFE_DAC_CON0, 0x1, 0x1);  // bit0: AFE_ON
        }
        //------------------------------------------------------------------

        Setting_FM.onoff = 0;
        Setting_FM.mode = 1;
        FM_I2S_Setting();
        //set gpio 200,201,202
        mAudioHardware->SetFmDigitalOutEnable(true);
    }
    else{
        //disable
        //set i2s 0 master out
        SetAfeReg(AFE_DAC_CON1, 0, 0xf00);
        SetI2SControl (false,(I2STYPE)I2S0OUTPUT,mSamplingRate);
        if(mAudioHardware->mStreamHandler->mI2SInstance->StateInI2S1OutputStart () == false){
        SetAfeReg(AFE_CONN0, 0<<22, 1<<22);  // Connect -- DL1_I_L to I2S_R,  bit5: I05-O01
        SetAfeReg(AFE_CONN0, 0<<6, 1<<6);  // Connect -- DL1_I_R to I2S_L,  bit23: I06-O00
            SetAfeReg(AFE_DAC_CON0, 0, 0x20);
        }
        GetAfeReg(AFE_DAC_CON0, &val);
        if( (val & 0x7E) == 0 )
        {
           SetAfeReg(AFE_DAC_CON0, 0x0, 0x1); // AFE_ON
        }
        //reset gpio
        mAudioHardware->SetFmDigitalOutEnable(false);
        }

#else
        //use digital sineware to test
        if(Enable >0){
            mAnalog->AnalogAFE_Open(AUDIO_PATH);
            Afe_FmTx_SineWave(1, 1);
        }
        else
        {
            mAnalog->AnalogAFE_Close(AUDIO_PATH);
            Afe_FmTx_SineWave(0, 1);
        }
#endif
}

uint32 AudioAfe::Afe_Get_UL_SR_Index(uint32 SamplingRate)
{
    uint32 SR_index =0;
    switch(SamplingRate)
    {
        case 8000:
           SR_index = UPLINK8K;
           break;
       case 16000:
           SR_index = UPLINK16K;
           break;
       case 32000:
           SR_index = UPLINK32K;
           break;
       case 48000:
           SR_index = UPLINK48K;
           break;
       default:
           LOG_AFE("Afe_DL_Start error mSamplingRate=%d \n",mSamplingRate);
           SR_index = UPLINK_UNDEF;
   }
   return SR_index;
}

uint32 AudioAfe::Afe_Get_DL_SR_Index(uint32 SamplingRate)
{
    uint32 SR_index =0;
    switch(SamplingRate)
    {
       case 8000:
          SR_index = FS_8000HZ;
          break;
       case 11025:
          SR_index = FS_11025HZ;
          break;
       case 12000:
          SR_index = FS_12000HZ;
          break;
       case 16000:
          SR_index = FS_16000HZ;
          break;
       case 22050:
          SR_index = FS_22050HZ;
          break;
       case 24000:
          SR_index = FS_24000HZ;
          break;
       case 32000:
          SR_index = FS_32000HZ;
          break;
       case 44100:
          SR_index = FS_44100HZ;
          break;
       case 48000:
          SR_index = FS_48000HZ;
          break;
       default:
          LOG_AFE("Afe_DL_Start error mSamplingRate=%d \n",mSamplingRate);
          SR_index = FS_44100HZ;  // use default
    }
    return SR_index;
}

void AudioAfe::Afe_DL_Start(AFE_MODE OpenMode)
{
   ALOGD("!!! Afe_DL_Start OpenMode=%d \n",OpenMode);
   ::ioctl(mFd,AUDDRV_BEE_IOCTL,0x20);
   uint32  value = 0x0;
   uint32  mask  = 0x0;
   uint32  SR_index = 0;
   uint32  SR_index_mode = 0;
   uint32 SamplingRateConvert[9] = {0, 1, 2, 4, 5, 6, 8, 9, 10};
   uint32 val, afe_on, dl1_on, i2s_on, dai_on;
   mFlag_Aud_DL1_SlaveOn = ::ioctl(mFd, GET_DL1_SLAVE_MODE,0);

   if(OpenMode == AFE_MODE_DAC)
   {
      uint32 val =0;
      ::ioctl(mFd,SET_DL1_AFE_BUFFER,0);
      SR_index = Afe_Get_DL_SR_Index(mSamplingRate);
      SR_index_mode = SamplingRateConvert[SR_index];
      ALOGD("Afe_DL_Start AFE_MODE_DAC SR_index = %d SR_index_mode = %d",SR_index,SR_index_mode);

      // Open DL1->DAC Path
      SetAfeReg(AFE_CONN1, 1<<27, 1<<27);
      SetAfeReg(AFE_CONN2, 1<<17, 1<<17);

      // For Low Power Audio Setting
      SetAfeReg(AFE_DL_SRC1_1,0x1,0x1);       // tuen on SDM
      SetAfeReg(AFE_DL_SDM_CON0,0x08800000,0xffffffff); //  2-Order 9-Bit Scrambler and No Dithering
      SetAfeReg(AFE_SDM_GAIN_STAGE,0x0000001e,0xffffffff);
      ::ioctl(mFd,AUDDRV_BEE_IOCTL,0x21);//by Charlie
      mAnalog->SetAnaReg(WR_PATH0,0x8020,0x8020); // FIFO Clock Edge Control
      ::ioctl(mFd,AUDDRV_BEE_IOCTL,0x22);//by Charlie
      // Set sampling rate (AFE_DAC_CON1)
      if (mFlag_Aud_DL1_SlaveOn)
      {
          value = ((SR_index_mode) &~ (1<<19));           // bit0~bit3: DL1_MODE
      }
      else
      {
          value = ((SR_index_mode) | (1<<19));           // bit0~bit3: DL1_MODE
      }
      value = (value & ~(1<<18)) | (mDaiBtMode<<18);     // DAI mode
      LOG_AFE("Afe_DL_Start, AFE_DAC_CON1, DL1_OPMODE \n");
      SetAfeReg(AFE_DAC_CON1,value,0x0008000f);      // bit19: DL1_OPMODE  1:Master mode , 0:Slave mode

      value = 0;
      mask = 0;
      if(mCh == android_audio_legacy::AudioSystem::CHANNEL_OUT_MONO)
      {
         SET_DL1_DATA_mono(value);                  //bit20: memory interface DL1 set mono
         SET_DL1_DATA_mask(mask);
      }
      else if(mCh == android_audio_legacy::AudioSystem::CHANNEL_OUT_STEREO)
      {
         SET_DL1_DATA_stereo(value);                //bit20: memory interface DL1 set stereo
         SET_DL1_DATA_mask(mask);
      }
      else
      {
         LOG_AFE("Afe_DL_Start error mCh=%d \n",mCh);
      }
      LOG_AFE("Afe_DL_Start AFE_DAC_CON1 DL1_stereo \n");
      SetAfeReg(AFE_DAC_CON1,value,mask);
	  ::ioctl(mFd,AUDDRV_BEE_IOCTL,0x23);

      //------------------------------------------------------------------
      // Enable Memory Interface AFE/DL1 On
      GetAfeReg(AFE_DAC_CON0,&val);
      LOG_AFE("Afe_DL_Start, AFE_DAC_CON0 (0x%x) \n",val);
      afe_on = val & 0x01;
      dl1_on = val & 0x02;
      if(dl1_on == 0)
      {
         // Turn on DL1_ON
         LOG_AFE("Afe_DL_Start, AFE_DAC_CON0, DL1 ON \n");
         SetAfeReg(AFE_DAC_CON0,0x2,0x2);  // bit1: DL1_ON
      }
      if(afe_on == 0)
      {
         // Turn on AFE_ON
         LOG_AFE("Afe_DL_Start, AFE_DAC_CON0, AFE ON \n");
         SetAfeReg(AFE_DAC_CON0,0x1,0x1);  // bit0: AFE_ON
      }
      //------------------------------------------------------------------
      usleep(200);
      //------------------------------------------------------------------
      // Turn on DL SRC
      GetAfeReg(AFE_DL_SRC2_1,&val);
      ALOGD("Afe_DL_Start, +AFE_DL_SRC2_1 (0x%x) \n",val);
      SetAfeReg(AFE_DL_SRC2_1,(SR_index << 28),0xf0000000);  // bit31~bit28:  DL2_INPUT_MODE_SEL
      SetAfeReg(AFE_DL_SRC2_1,0x03000000,0x03000000);  // bit24~bit25:  DL2_OUTPUT_SEL    //up-sampleing x8
      SetAfeReg(AFE_DL_SRC2_1,0x0002,0x0022);  // bit1: DL_2_GAIN_ON, DL_2_VOICE_MODE
                                               // bit5: DL_2_VOICE_MODE-- 0 for audio, 1 for voice
      //------------------------------------------------------------------
      SetAfeReg(AFE_DL_SRC2_1,0x1,0x1);  // bit0: DL_2_SRC_ON
      GetAfeReg(AFE_DL_SRC2_1,&val);
      ALOGD("Afe_DL_Start, -AFE_DL_SRC2_1 (0x%x) \n",val);
      //------------------------------------------------------------------
      // Enable MCU IRQ
      LOG_AFE("Afe_DL_Start, AFE_IRQ_CON, IRQ1_MCU_ON \n");
      SetAfeReg(AFE_IRQ_CON,(SR_index_mode << 4),0xf0);     // bit7~bit4: Set IRQ1 mode (frequency)
      SetAfeReg(AFE_IRQ_CON,0x1,0x1);    // bit0: IRQ1_MCU_ON
      //------------------------------------------------------------------
      ::ioctl(mFd,AUDDRV_BEE_IOCTL,0x24);
   }

   else if(OpenMode == AFE_MODE_I2S0_OUT)
   {
       SR_index = Afe_Get_DL_SR_Index(mSamplingRate);
       SR_index_mode = SamplingRateConvert[SR_index];

       // For Low Power Audio Setting
       SetAfeReg(AFE_DL_SRC1_1,0x1,0x1);       // tuen on SDM
       SetAfeReg(AFE_DL_SDM_CON0,0x08800000,0xffffffff); //  2-Order 9-Bit Scrambler and No Dithering
       SetI2SControl (true,I2S0OUTPUT,mSamplingRate);
       // Set sampling rate (AFE_DAC_CON1)

       if (mFlag_Aud_DL1_SlaveOn)
       {
           value = ((SR_index_mode) &~ (1<<19));           // bit0~bit3: DL1_MODE
       }
       else
       {
             value = ((SR_index_mode) | (1<<19));           // bit0~bit3: DL1_MODE
       }
       value = value |(SR_index_mode<<8);           // bit8~bit11: I2S_MODE

       LOG_AFE("Afe_DL_Start, AFE_DAC_CON1, DL1_OPMODE \n");
       SetAfeReg(AFE_DAC_CON1,value,0x00080f0f);      // bit19: DL1_OPMODE  1:Master mode , 0:Slave mode

       value = 0;
       mask = 0;
       if(mCh == android_audio_legacy::AudioSystem::CHANNEL_OUT_MONO)
       {
          SET_DL1_DATA_mono(value);                  //bit20: memory interface DL1 set mono
          SET_DL1_DATA_mask(mask);
          SET_I2S_DATA_mono(value);                  //bit22: memory interface I2S set mono
          SET_I2S_DATA_mask(mask);
       }
       else if(mCh == android_audio_legacy::AudioSystem::CHANNEL_OUT_STEREO)
       {
          SET_DL1_DATA_stereo(value);                //bit20: memory interface DL1 set stereo
          SET_DL1_DATA_mask(mask);
          SET_I2S_DATA_stereo(value);                //bit22: memory interface I2S set stereo
          SET_I2S_DATA_mask(mask);
       }
       else
       {
          LOG_AFE("Afe_DL_Start error mCh=%d \n", mCh);
       }
       SetAfeReg(AFE_DAC_CON1,value,mask);

       //------------------------------------------------------------------
       // Set ConnectionRecord
         SetAfeReg(AFE_CONN0,1<<22, 1<<22);  // Connect -- DL1_I_L to I2S_R,  bit5: I05-O01
         SetAfeReg(AFE_CONN0,1<<6, 1<<6);    // Connect -- DL1_I_R to I2S_L,  bit23: I06-O00

       //------------------------------------------------------------------
         // Turn on DL SRC
         GetAfeReg(AFE_DL_SRC2_1,&val);
         ALOGD("Afe_DL_Start, +AFE_DL_SRC2_1 (0x%x) \n",val);
         SetAfeReg(AFE_DL_SRC2_1,(SR_index << 28),0xf0000000);  // bit31~bit28:  DL2_INPUT_MODE_SEL
         SetAfeReg(AFE_DL_SRC2_1,0x0001,0x0021);  // bit0: DL_2_SRC_ON, // bit1: DL_2_GAIN_ON, DL_2_VOICE_MODE
       // bit5: DL_2_VOICE_MODE-- 0 for audio, 1 for voice_factor
       SetAfeReg(AFE_DL_SRC2_1,0x03000000,0x03000000);  // bit24~bit25:  DL2_OUTPUT_SEL    //up-sampleing x8   bit24 ~ BIT25
         ALOGD("Afe_DL_Start, -AFE_DL_SRC2_1 (0x%x) \n",val);

       //------------------------------------------------------------------
       // Enable Memory Interface AFE/DL1/I2S On
       GetAfeReg(AFE_DAC_CON0,&val);
       // Turn on DL1_ON
       afe_on = val & 0x01;
       dl1_on = val & 0x02;
       i2s_on = val & 0x20;
       if(dl1_on == 0)
       {
        // Turn on DL1_ON
          LOG_AFE("Afe_DL_Start, AFE_DAC_CON0, DL1 ON \n");
          SetAfeReg(AFE_DAC_CON0,0x2,0x2);  // bit1: DL1_ON
       }
       if(i2s_on == 0)
       {
        // Turn on DL1_ON
          LOG_AFE("Afe_DL_Start, AFE_DAC_CON0, I2S ON \n");
          SetAfeReg(AFE_DAC_CON0,0x20,0x20);  // bit5: I2S_ON
       }
       usleep(100);
       if(afe_on == 0)
       {
        // Turn on AFE_ON
        LOG_AFE("Afe_DL_Start, AFE_DAC_CON0, AFE ON \n");
        SetAfeReg(AFE_DAC_CON0,0x1,0x1);  // bit0: AFE_ON
       }
       //------------------------------------------------------------------
       if((SR_index_mode>=8)&&(SR_index_mode<=10))//32k, 44.1k, 48k
       {
           Setting_FM.sample = SR_index_mode - 8;
       }
       else
       {
           Setting_FM.sample = 1;//default 44.1k
       }
       Setting_FM.onoff = 0;
       Setting_FM.mode = 1;
       FM_I2S_Setting();

       //------------------------------------------------------------------
       // Enable MCU IRQ
       LOG_AFE("Afe_DL_Start, AFE_IRQ_CON, IRQ1_MCU_ON \n");
       SetAfeReg(AFE_IRQ_CON,(SR_index_mode << 4),0xf0);     // bit7~bit4: Set IRQ1 mode (frequency)
       SetAfeReg(AFE_IRQ_CON,0x1,0x1);    // bit0: IRQ1_MCU_ON
       //------------------------------------------------------------------

   }
   else if(OpenMode == AFE_MODE_I2S1_OUT)
   {
       uint32 val =0;
       SR_index = Afe_Get_DL_SR_Index(I2S1output_Stream.SampleRate);
       SR_index_mode = SamplingRateConvert[SR_index];

       // Set connection
       SetAfeReg(AFE_CONN0,1<<22, 1<<22);  // Connect -- DL1_I_L to I2S_R,  bit5: I05-O01
       SetAfeReg(AFE_CONN0,1<<6, 1<<6);    // Connect -- DL1_I_R to I2S_L,  bit23: I06-O00
       SetI2SControl (true,I2S1OUTPUT,I2S1output_Stream.SampleRate);
       }
   else if(OpenMode == AFE_MODE_I2S1_OUT_HDMI)  // DL1 --> I2S1 output
   {
#ifndef MTK_TABLET_PLATFORM
      ::ioctl(mFd, AUD_SET_HDMI_CLOCK, 1);
#endif

      uint32 val =0;
      ::ioctl(mFd,SET_DL1_AFE_BUFFER,0);
      SR_index = Afe_Get_DL_SR_Index(mSamplingRate);
      SR_index_mode = SamplingRateConvert[SR_index];
      ALOGD("Afe_DL_Start, HDMI, SR_index_mode(%d)",SR_index_mode);

      //------------------------------------------------------------------
      // For SDM Setting
//      SetAfeReg(AFE_DL_SRC1_1,0x1,0x1);       // tuen on SDM
//      SetAfeReg(AFE_DL_SDM_CON0,0x08800000,0xffffffff); //  2-Order 9-Bit Scrambler and No Dithering
//      SetAfeReg(AFE_SDM_GAIN_STAGE,0x0000001e,0xffffffff);

      //------------------------------------------------------------------
      // Set sampling rate (AFE_DAC_CON1)
      if (mFlag_Aud_DL1_SlaveOn)
      {
         value = ((SR_index_mode) &~ (1<<19));           // bit0~bit3: DL1_MODE
      }
      else
      {
         value = ((SR_index_mode) | (1<<19));           // bit0~bit3: DL1_MODE
      }

      value = (value & ~(1<<18)) | (mDaiBtMode<<18);    // DAI mode
      value = value |(SR_index_mode<<8);           // bit8~bit11: I2S_MODE
      LOG_AFE("Afe_DL_Start, HDMI, AFE_DAC_CON1, DL1_OPMODE \n");
      SetAfeReg(AFE_DAC_CON1,value,0x00080f0f);      // bit19: DL1_OPMODE  1:Master mode , 0:Slave mode

      value = 0;
      mask = 0;
      if(mCh == android_audio_legacy::AudioSystem::CHANNEL_OUT_MONO)
      {
         SET_DL1_DATA_mono(value);                  //bit20: memory interface DL1 set mono
         SET_DL1_DATA_mask(mask);
      }
      else if(mCh == android_audio_legacy::AudioSystem::CHANNEL_OUT_STEREO)
      {
         SET_DL1_DATA_stereo(value);                //bit20: memory interface DL1 set stereo
         SET_DL1_DATA_mask(mask);
      }
      else
      {
         LOG_AFE("Afe_DL_Start, HDMI error mCh=%d \n", mCh);
      }
      SetAfeReg(AFE_DAC_CON1,value,mask);

      //------------------------------------------------------------------
      // Set connection
      SetAfeReg(AFE_CONN0,1<<22, 1<<22);  // Connect -- DL1_I_L to I2S_R,  bit5: I05-O01
      SetAfeReg(AFE_CONN0,1<<6, 1<<6);    // Connect -- DL1_I_R to I2S_L,  bit23: I06-O00

      //------------------------------------------------------------------
      // Enable I2S1 output, master mode
      SetAfeReg(AFE_I2S_OUT_CON,SR_index_mode << 8,0x0F00);  // bit8~11: I2S1 output mode
      SetAfeReg(AFE_I2S_OUT_CON,0x0009,0x000F);              // bit0: 1:enable I2S1

      //------------------------------------------------------------------
      // Set HDMI Tx driver's sampling rate
      // to do
      ALOGD("Afe_DL_Start, AUD_SET_HDMI_SR (%d) \n",mSamplingRate);
      ::ioctl(mFd, AUD_SET_HDMI_SR, mSamplingRate);

      // mute HDMI device some time if audio start playing (avoid HDMI audio noise)
      //::ioctl(mFd, AUD_SET_HDMI_MUTE, 100); // 100ms

      //------------------------------------------------------------------
      // Don't Turn on DL SRC
      GetAfeReg(AFE_DL_SRC2_1,&val);
      ALOGD("Afe_DL_Start, HDMI +AFE_DL_SRC2_1 (0x%x) \n",val);
      SetAfeReg(AFE_DL_SRC2_1,(SR_index << 28),0xf0000000);  // bit31~bit28:  DL2_INPUT_MODE_SEL
      SetAfeReg(AFE_DL_SRC2_1,0x0002,0x0022);  // bit0: DL_2_SRC_ON, // bit1: DL_2_GAIN_ON, DL_2_VOICE_MODE
                                               // bit5: DL_2_VOICE_MODE-- 0 for audio, 1 for voice
      SetAfeReg(AFE_DL_SRC2_1,0x03000000,0x03000000);  // bit24~bit25:  DL2_OUTPUT_SEL    //up-sampleing x8   bit24 ~ bit25

      //SetAfeReg(AFE_DL_SRC2_1,0x1,0x1);  // bit0: DL_2_SRC_ON
      ALOGD("Afe_DL_Start, HDMI -AFE_DL_SRC2_1 (0x%x) \n",val);
      //------------------------------------------------------------------
      usleep(200);
      //------------------------------------------------------------------
      // Enable Memory Interface AFE/DL1
      GetAfeReg(AFE_DAC_CON0,&val);
      // Turn on DL1_ON
      afe_on = val & 0x01;
      dl1_on = val & 0x02;
      if(dl1_on == 0)
      {
         // Turn on DL1_ON
         LOG_AFE("Afe_DL_Start, HDMI AFE_DAC_CON0, DL1 ON \n");
         SetAfeReg(AFE_DAC_CON0,0x2,0x2);  // bit1: DL1_ON
      }
      usleep(100);
      if(afe_on == 0)
      {
         // Turn on AFE_ON
         LOG_AFE("Afe_DL_Start, HDMI AFE_DAC_CON0, AFE ON \n");
         SetAfeReg(AFE_DAC_CON0,0x1,0x1);  // bit0: AFE_ON
      }

      //------------------------------------------------------------------
      // Enable MCU IRQ
      LOG_AFE("Afe_DL_Start, HDMI AFE_IRQ_CON, IRQ1_MCU_ON \n");
      SetAfeReg(AFE_IRQ_CON,(SR_index_mode << 4),0xf0);     // bit7~bit4: Set IRQ1 mode (frequency)
      SetAfeReg(AFE_IRQ_CON,0x1,0x1);    // bit0: IRQ1_MCU_ON
      //------------------------------------------------------------------

   }
   else if(OpenMode == AFE_MODE_DAI)
   {
       uint32 val =0;
       // add for BT_DAI
       SR_index = (mDaiBtMode==0) ? FS_8000HZ : FS_16000HZ;
       SR_index_mode = SamplingRateConvert[SR_index];
       ioctl(mFd,SET_DL1_AFE_BUFFER,0);
       // For Low Power Audio settingsAllowed

	   #if 0 	
       SetAfeReg(AFE_CONN1,1<<3,1<<3);  // Connect -- DL1_I_L to DAI_O,  bit3: I05-O02
	   #else   //ccc skype
	   SetAfeReg(AFE_CONN1,1<<3,1<<3);  // Connect -- DL1_I_L to DAI_O,  bit3: I05-O02
	   SetAfeReg(AFE_CONN1,1<<4,1<<4);  // Connect -- DL1_I_L to DAI_O,  bit3: I06-O02
	   SetAfeReg(AFE_CONN1,1<<14,1<<14);  // Connect -- DL1_I_L to DAI_O,  bit3: I06-O02 right shift one bits
	   #endif
       SetAfeReg(AFE_CONN1,1<<13,1<<13);  // Connect -- DL1_I_L to DAI_O,  bit3: I05-O02 right shift one bits

       SetAfeReg(AFE_DL_SRC1_1,0x1,0x1);       // tuen on SDM
       SetAfeReg(AFE_DL_SDM_CON0,0x08800000,0xffffffff); //  2-Order 9-Bit Scrambler and No Dithering
       mAnalog->SetAnaReg(WR_PATH0,0x8020,0x8020); // FIFO Clock Edge Control
       if (mFlag_Aud_DL1_SlaveOn)
       {
           value = ((SR_index_mode) &~ (1<<19));           // bit0~bit3: DL1_MODE
       }
       else
       {
           value = ((SR_index_mode) | (1<<19));           // bit0~bit3: DL1_MODE
       }

       value = (value & ~(1<<18)) | (mDaiBtMode<<18);     // DAI mode
       ALOGD("Afe_DL_Start, AFE_DAC_CON1 value = %x",value);
       SetAfeReg(AFE_DAC_CON1,value,0x000C000f);      // bit19: DL1_OPMODE  1:Master mode , 0:Slave mode

       value = 0;
       mask = 0;
       if(mCh == android_audio_legacy::AudioSystem::CHANNEL_OUT_MONO)
       {
           SET_DL1_DATA_mono(value);                  //bit20: memory interface DL1 set mono
           SET_DL1_DATA_mask(mask);
       }
       else if(android_audio_legacy::AudioSystem::CHANNEL_OUT_STEREO)
       {
           SET_DL1_DATA_stereo(value);                //bit20: memory interface DL1 set stereo
           SET_DL1_DATA_mask(mask);
       }
       else
       {
           LOG_AFE("Afe_DL_Start error mCh=%d \n",mCh);
       }

       LOG_AFE("Afe_DL_Start AFE_DAC_CON1 DL1_stereo \n");
       SetAfeReg(AFE_DAC_CON1,value,mask);


	   #if 0 //ccc skype test

		SetAfeReg(AFE_CONN1,(1<<27),(1<<27));  // Connect -- DL1_R to DAC_L, bit27: I06-O03
       SetAfeReg(AFE_CONN2,(1<<17),(1<<17));  // Connect -- DL1_L to DAC_R, bit17: I05-O04
	
      //------------------------------------------------------------------
      usleep(200);
      //------------------------------------------------------------------
      // Turn on DL SRC
      GetAfeReg(AFE_DL_SRC2_1,&val);
      LOGD("Afe_DL_Start, +AFE_DL_SRC2_1 (0x%x) \n",val);
      SetAfeReg(AFE_DL_SRC2_1,(0x3 << 28),0xf0000000);  // bit31~bit28:  DL2_INPUT_MODE_SEL
      SetAfeReg(AFE_DL_SRC2_1,0x03000000,0x03000000);  // bit24~bit25:  DL2_OUTPUT_SEL    //up-sampleing x8
      SetAfeReg(AFE_DL_SRC2_1,0x0002,0x0022);  // bit1: DL_2_GAIN_ON, DL_2_VOICE_MODE
                                               // bit5: DL_2_VOICE_MODE-- 0 for audio, 1 for voice
      //------------------------------------------------------------------
      SetAfeReg(AFE_DL_SRC2_1,0x1,0x1);  // bit0: DL_2_SRC_ON
      GetAfeReg(AFE_DL_SRC2_1,&val);
      LOGD("Afe_DL_Start, -AFE_DL_SRC2_1 (0x%x) \n",val);
	   #endif

       //------------------------------------------------------------------
       // Turn on BT_DAI
       ioctl(mFd,AUDDRV_START_DAI_OUTPUT,0);

       GetAfeReg(AFE_DAIBT_CON,&val);
       val &= ~0xFC0b;
       SetAfeReg(AFE_DAIBT_CON,val,0xffff);
       ALOGD("Afe_DL_Start, -AFE_DAIBT_CON0 (0x%x) \n",val);
       val &= ~(1<<8);// high word
       val &= ~(1<<2);// BTSYNC short
       #if 1
       val &= ~(1<<3);// DATA_RDY : not ready
	   #else //ccc skype
       val |= (1<<3);// DATA_RDY : ready , can set to 1 here since DL1 buffer is cleared by START_DL1_STREAM in SetOutputStreamToBT()
       LOGD("set DATA_RDY to 1! \n",val);
       #endif
       val = (val & ~(1<<9)) | (mDaiBtMode<<9) ; // DAIBT mode
       val |= (1<<1) ; // BT_ON
       val |= (1<<0) ; // enable DAI_BT
       SetAfeReg(AFE_DAIBT_CON,val,0xffff); // enable for DAI_BT function , BT_ON=1 , data ready =1
       ALOGD("Afe_DL_Start, -AFE_DAIBT_CON0 (0x%x) \n",val);

      //------------------------------------------------------------------

      //------------------------------------------------------------------
      // Enable Memory Interface AFE/DL1/DAI On
      GetAfeReg(AFE_DAC_CON0,&val);
      afe_on = val & 0x01;
      dl1_on = val & 0x02;
      dai_on = val & 0x10;
      if(dl1_on == 0)
      {
       // Turn on DL1_ON
         LOG_AFE("Afe_DL_Start, AFE_DAC_CON0, DL1 ON \n");
         SetAfeReg(AFE_DAC_CON0,0x2,0x2);  // bit1: DL1_ON
      }
      if(dai_on == 0)
      {
         // Turn on DAI ON
         LOG_AFE("Afe_DL_Start, AFE_DAC_CON0, DAI ON \n");
         SetAfeReg(AFE_DAC_CON0,0x10,0x10);  //bit4: DAI ON
      }
      usleep(100);
      if(afe_on == 0)
      {
         // Turn on AFE_ON
         LOG_AFE("Afe_DL_Start, AFE_DAC_CON0, AFE ON \n");
         SetAfeReg(AFE_DAC_CON0,0x1,0x1);  // bit0: AFE_ON
      }
      //------------------------------------------------------------------

      //------------------------------------------------------------------
      // Enable MCU IRQ
      LOG_AFE("Afe_DL_Start, AFE_IRQ_CON, IRQ1_MCU_ON \n");
      SetAfeReg(AFE_IRQ_CON,(SR_index_mode << 4),0xf0);     // bit7~bit4: Set IRQ1 mode (frequency)
      SetAfeReg(AFE_IRQ_CON,0x00000000,0x00040000);     // bits 18 ,  set 8k
      SetAfeReg(AFE_IRQ_CON,0x1,0x1);    // bit0: IRQ1_MCU_ON
      SetAfeReg(AFE_IRQ_CON,0x4,0x4);    // bit0: IRQ3_MCU_ON
      //------------------------------------------------------------------
   }
   else if(OpenMode == AFE_MODE_FTM_I2S)
   {
      ::ioctl(mFd, AUD_SET_I2S_CLOCK, 1);// turn on audio clock

      int FM_Sampling_Rate = 48000;
      uint32  value = 0x0;
      uint32  SR_index = 0;
      uint32  SR_index_mode = 0;
      uint32 SamplingRateConvert[9] = {0, 1, 2, 4, 5, 6, 8, 9, 10};

#ifdef I2S0_OUTPUT_USE_FLEXL2
             ::ioctl(mFd, SET_I2S_Output_BUFFER, 0);
#else
             ::ioctl(mFd,OPEN_I2S_INPUT_STREAM,EDI_BUFFER_SIZE);
             ::ioctl(mFd,START_I2S_INPUT_STREAM,0);
#endif

      // set sampling rate (AFE_DL_SRC2_1)
      switch(FM_Sampling_Rate)
      {
         case 8000:
            SR_index = FS_8000HZ;
            break;
         case 11025:
            SR_index = FS_11025HZ;
            break;
         case 12000:
            SR_index = FS_12000HZ;
            break;
         case 16000:
            SR_index = FS_16000HZ;
            break;
         case 22050:
            SR_index = FS_22050HZ;
            break;
         case 24000:
            SR_index = FS_24000HZ;
            break;
         case 32000:
            SR_index = FS_32000HZ;
            break;
         case 44100:
            SR_index = FS_44100HZ;
            break;
         case 48000:
            SR_index = FS_48000HZ;
            break;
         default:
            LOG_AFE("Afe_DL_Start error mSamplingRate=%d \n",mSamplingRate);
            SR_index = FS_44100HZ;  // use default
      }
      SR_index_mode = SamplingRateConvert[SR_index];

      // For Low Power Audio Setting
      SetAfeReg(AFE_DL_SRC1_1,0x1,0x1);       // tuen on SDM
      SetAfeReg(AFE_DL_SDM_CON0,0x08800000,0xffffffff); //  2-Order 9-Bit Scrambler and No Dithering

      // Set sampling rate (AFE_DAC_CON1)
      value = ( (SR_index_mode) | (SR_index_mode << 8) ); // bit8~bit11: I2S_MODE   // bit0~bit3: DL1_MODE
      LOG_AFE("Afe_DL_Start, AFE_DAC_CON1 \n");
      SetAfeReg(AFE_DAC_CON1,value,0x00000f0f);      // bit19: DL1_OPMODE  1:Master mode , 0:Slave mode

      // Turn on DAC
      SetAfeReg(AFE_DL_SRC2_1,(SR_index << 28),0xf0000000);  // bit31~bit28:  DL2_INPUT_MODE_SEL
      SetAfeReg(AFE_DL_SRC2_1,0x0003,0x0023);  // bit0: DL_2_SRC_ON,
                                               // bit1: DL_2_GAIN_ON, DL_2_VOICE_MODE
                                               // bit5: DL_2_VOICE_MODE-- 0 for audio, 1 for voice
      SetAfeReg(AFE_DL_SRC2_1,0x03000000,0x03000000);  // bit24~bit25:  DL2_OUTPUT_SEL    //up-sampleing x8   bit24 ~ bit25

      // Set connection
      SetAfeReg(AFE_CONN1,1<<21,1<<21);  // Connect -- I2SIn_L to DAC_L, bit21: I00-O03
      SetAfeReg(AFE_CONN2,1<<13,1<<13);  // Connect -- I2SIn_R to DAC_R,bit13: I01-O04

      // Turn on device . Enable I2S input
      // bit0: (SRC)   0: disable I2S, 1: enable I2S
      // bit1: (SRC)   0: 16 bits, 1: 32 bit
      // bit2: (SRC)   0 for master mode(clock from AFE), 1 for slave mode(clock from other device)
      // bit3: (FMT)   1 for I2S, 0 for EIAJ
      // bit4: (DIR)   0 for output mode, 1 for input mode
      int mt6573_chip_ver;
      mt6573_chip_ver = ::ioctl(mFd,AUDDRV_MT6573_CHIP_VER,0);  //ioctl to stanby mode

      if(mt6573_chip_ver == 0x8a00){
         ALOGD("Afe_DL_Start, AFE_MODE_FTM_I2S, MT6573 E1 Chip (slave mode) ");
         SetAfeReg(AFE_I2S_IN_CON,0x1d,0x1f);   // bit0: 1:enable I2S input
      }
      else if(mt6573_chip_ver == 0xca10){
         ALOGD("Afe_DL_Start, AFE_MODE_FTM_I2S, MT6573 E2 Chip (master mode) ");
         SetAfeReg(AFE_I2S_IN_CON,0x19,0x1f);   // bit0: 1:enable I2S input
      }
      else // default use master mode
      {
         ALOGD("Afe_DL_Start, AFE_MODE_FTM_I2S, No Chip ver (master mode) ");
         SetAfeReg(AFE_I2S_IN_CON,0x19,0x1f);   // bit0: 1:enable I2S input
      }

      SetAfeReg(AFE_FOC_CON,0x5a05,0xffff);

      SetAfeReg(AFE_CONN4,0x0,0x40000000);  // only for slave mode
      SetAfeReg(AFE_SDM_GAIN_STAGE,0x0000001e,0xffffffff);

      //------------------------------------------------------------------
      // Enable Memory Interface AFE/DL1/I2S On
      GetAfeReg(AFE_DAC_CON0,&val);
      // Turn on DL1_ON
      afe_on = val & 0x01;
      i2s_on = val & 0x20;
      if(i2s_on == 0)
      {
       // Turn on DL1_ON
         LOG_AFE("Afe_DL_Start, AFE_DAC_CON0, I2S ON \n");
         SetAfeReg(AFE_DAC_CON0,0x20,0x20);  // bit5: I2S_ON
      }
      usleep(100);
      if(afe_on == 0)
      {
       // Turn on AFE_ON
       LOG_AFE("Afe_DL_Start, AFE_DAC_CON0, AFE ON \n");
       SetAfeReg(AFE_DAC_CON0,0x1,0x1);  // bit0: AFE_ON
      }
      //------------------------------------------------------------------
   }
   else if(OpenMode == AFE_MODE_SIGMADSP)
   {

   }
   else
   {
      LOG_AFE("Afe_DL_Start Error no this mode");
   }

    ::ioctl(mFd,AUDDRV_BEE_IOCTL,0x25);
}

void AudioAfe::Afe_UL_Start(AFE_MODE OpenMode)
{
   ALOGD("Afe_UL_Start OpenMode=%d",OpenMode);

   uint32 value = 0x0;
   uint32 mask  = 0x0;
   uint32 SR_index = 0;
   uint32 SR_index_mode = 0;
   uint32 SamplingRateConvert[9] = {0, 1, 2, 4, 5, 6, 8, 9, 10};
   uint32 val, afe_on, dl1_on, i2s_on;
   mFlag_Aud_DL1_SlaveOn = ::ioctl(mFd, GET_DL1_SLAVE_MODE,0);

   if(OpenMode == AFE_MODE_ADC)
   {
      // only used for Factory mode
      SR_index_mode = Afe_Get_UL_SR_Index(ADC_Stream.SampleRate);
      ALOGD("Afe_UL_Start AFE_MODE_ADC SR_index_mode = %d",SR_index_mode);

      //SetAfeReg(AFE_UL_SRC_0,0x0400,0x0400);  // UL IIR DC REMOVAL
      SetAfeReg(AFE_UL_SRC_0,0x0,0x0400);  //disable UL IIR DC REMOVAL
      SetAfeReg(AFE_UL_SRC_0,0,0x380);      // UL_IIR_MODE
      SetAfeReg(AFE_UL_SRC_0,SR_index_mode<<17,0x00060000);
      SetAfeReg(AFE_UL_SRC_0,SR_index_mode<<19,0x00180000);
      SetAfeReg(AFE_DAC_CON1,SR_index_mode<<16,0x00030000);
      SetAfeReg(AFE_DAC_CON0,1<<3,1<<3);        // bit0: 1:enable VUL
      SetAfeReg(AFE_UL_SRC_0,0x1,0x1);   // UL_SRC_ON
   }
   else if(OpenMode == AFE_MODE_I2S_IN)
   {
      ioctl(mFd,SET_I2S_Input_BUFFER,0);
      SR_index = Afe_Get_DL_SR_Index(I2Sinput_Stream.SampleRate);
      SR_index_mode = SamplingRateConvert[SR_index];

      // Set connection
      SetAfeReg(AFE_CONN3,1<<4,1<<4);  // Connect -- I2S_I_L to I2S_L,  bit4: I00-O07
      SetAfeReg(AFE_CONN3,1<<5,1<<5);  // Connect -- I2S_I_R to I2S_R,  bit5: I01-O08

      // Turn on device . Enable I2S input
      SetAfeReg(AFE_I2S_IN_CON,0x1,0x1);        // bit0: 1:enable I2S input
      SetAfeReg(AFE_FOC_CON,0x5a05,0xffff);

      SetAfeReg(AFE_CONN4,0x0,0x40000000);  // only for slave mode
      SetAfeReg(AFE_SDM_GAIN_STAGE,0x0000001e,0xffffffff);

      // Set sampling rate and channel (AFE_DAC_CON1)
      value = 0;
      mask = 0;
      if(I2Sinput_Stream.Channel == android_audio_legacy::AudioSystem::CHANNEL_OUT_MONO)
      {
         SET_I2S_DATA_mono(value);                  //bit22: memory interface I2S set mono
         SET_I2S_DATA_mask(mask);
      }
      else if(I2Sinput_Stream.Channel == android_audio_legacy::AudioSystem::CHANNEL_OUT_STEREO)
      {
         SET_I2S_DATA_stereo(value);                //bit22: memory interface I2S set stereo
         SET_I2S_DATA_mask(mask);
      }
      else
      {
         LOG_AFE("Afe_UL_Start error mCh=%d \n",mCh);
      }
      SetAfeReg(AFE_DAC_CON1,value,mask);

      // Set sampling rate (AFE_DAC_CON1)

      if (mFlag_Aud_DL1_SlaveOn)
      {
          value = ( (SR_index_mode << 8) &~ (1<<19));        // bit8~bit11: I2S_MODE
      }
      else
      {
          value = ( (SR_index_mode << 8) | (1<<19));        // bit8~bit11: I2S_MODE
      }

      LOG_AFE("Afe_UL_Start, AFE_DAC_CON1, value:%d \n",value);
      SetAfeReg(AFE_DAC_CON1,value,0x00000f00);         // No need to set DL1_OPMODE
      //------------------------------------------------------------------
      // Enable Memory Interface I2S/AFE
      GetAfeReg(AFE_DAC_CON0,&val);
      afe_on = val & 0x01;
      i2s_on = val & 0x20;

      if(i2s_on == 0)
      {
         // Turn on I2S ON
         ALOGD("Afe_UL_Start, AFE_DAC_CON0, I2S ON \n");
         SetAfeReg(AFE_DAC_CON0,0x20,0x20);  //bit5: I2S ON
      }
      usleep(100);
      if(afe_on == 0)
      {
         // Turn on AFE_ON
         LOG_AFE("Afe_UL_Start, AFE_DAC_CON0, AFE ON \n");
         SetAfeReg(AFE_DAC_CON0,0x1,0x1);  // bit0: AFE_ON
      }
      //------------------------------------------------------------------

      // Enable IRQ
      SetAfeReg(AFE_IRQ_CON,(SR_index_mode << 8),0xf00);     // bit11~bit8: Set IRQ2 mode (frequency)
      SetAfeReg(AFE_IRQ_CON,0x2,0x2);    // bit1: IRQ2_MCU_ON
   }
   else if(OpenMode == AFE_MODE_DAI_IN)
   {

   }
   else if(OpenMode == AFE_MODE_AWB)
   {
       SR_index = Afe_Get_DL_SR_Index(AWB_Stream.SampleRate);
       SR_index_mode = SamplingRateConvert[SR_index];
       ALOGD("Afe_UL_Start AFE_MODE_AWB SR_index = %d SR_index_mode = %d",SR_index,SR_index_mode);
       // Set connection
       SetAfeReg(AFE_DAC_CON1,SR_index_mode<<12,0x0000f000); // set AWB mode
       usleep(200);
       SetAWBMemoryInterfaceEnable(true,BIT_06);
   }
   else
   {
      LOG_AFE("Afe_UL_Start Error no this mode \n");
   }

}

void AudioAfe::Afe_DL_Stop(AFE_MODE OpenMode)
{
   ALOGD("!!! Afe_DL_Stop OpenMode=%d",OpenMode);
   uint32 value = 0x0;
   uint32 mask  = 0x0;
   int sph_on = false;
   int bgs_on = false;
   int rec_on = false;
   int vt_on = false;

   ::ioctl(mFd,AUDDRV_BEE_IOCTL,0x50);
   if(OpenMode == AFE_MODE_DAC)
   {
      // Disable interrupt (timer)
      ALOGD("Afe_DL_Stop, AFE_IRQ_CON, Disable interrupt \n");
      SetAfeReg(AFE_IRQ_CON,0x0,0x1);

      // clear IRQ
      LOG_AFE("Afe_DL_Stop, AFE_IR_CLR, clear IRQ \n");
      SetAfeReg(AFE_IR_CLR,0x1,0x1);

      if(mAudioHardware != NULL){
         sph_on = mAudioHardware->Get_Recovery_Speech();
         bgs_on = mAudioHardware->Get_Recovery_Bgs();
         rec_on = mAudioHardware->Get_Recovery_Record();
         vt_on  = mAudioHardware->Get_Recovery_VT();
      }

      ALOGD("Afe_DL_Stop, sph_on:%d, bgs_on:%d, rec_on:%d, vt_on:%d \n",sph_on,bgs_on,rec_on,vt_on);
//      if( (sph_on==false) && (bgs_on==false) && (rec_on==false) && (vt_on==false) )
      if( (sph_on==false) && (bgs_on==false) && (vt_on==false) )
      {
         // Close device
         LOG_AFE("Afe_DL_Stop, AFE_DL_SRC2_1, close device \n");
         SetAfeReg(AFE_DL_SRC2_1,0x0,0x1);   // DL2_SRC2_ON=0
         usleep(200);
         // Turn off memory interface
         LOG_AFE("Afe_DL_Stop, AFE_DAC_CON0, close memory interface \n");
         SetAfeReg(AFE_DAC_CON0,0x0,0x2);  // DL1_ON=0
      }

   }
   else if(OpenMode == AFE_MODE_I2S0_OUT)//FM TX
   {
       // Disable interrupt (timer)
       ALOGD("Afe_DL_Stop  AFE_MODE_I2S0_OUT, AFE_IRQ_CON, Disable interrupt \n");
       SetAfeReg(AFE_IRQ_CON,0x0,0x1);

       // clear IRQ
       LOG_AFE("Afe_DL_Stop  AFE_MODE_I2S0_OUT, AFE_IR_CLR, clear IRQ \n");
       //------------------------------------------------------------------
       // Set connection
       SetAfeReg(AFE_CONN0,0<<22,1<<22);  // Connect -- DL1_I_L to I2S_R,  bit5: I05-O01
       SetAfeReg(AFE_CONN0,0<<6,1<<6);    // Connect -- DL1_I_R to I2S_L,  bit23: I06-O00
       SetI2SControl (false,I2S0OUTPUT);

       //------------------------------------------------------------------
       // Turn off DL SRC
       SetAfeReg(AFE_DL_SRC2_1,0x0,0x1);  // bit0: DL_2_SRC_ON, // bit1: DL_2_GAIN_ON, DL_2_VOICE_MODE
       //------------------------------------------------------------------

       SetAfeReg(AFE_IR_CLR,0x1,0x1);
       if(mAudioHardware != NULL){
          sph_on = mAudioHardware->Get_Recovery_Speech();
          bgs_on = mAudioHardware->Get_Recovery_Bgs();
          rec_on = mAudioHardware->Get_Recovery_Record();
          vt_on  = mAudioHardware->Get_Recovery_VT();
       }
       ALOGD("Afe_DL_Stop, sph_on:%d, bgs_on:%d, rec_on:%d, vt_on:%d \n",sph_on,bgs_on,rec_on,vt_on);
       //      if( (sph_on==false) && (bgs_on==false) && (rec_on==false) && (vt_on==false) )
       if( (sph_on==false) && (bgs_on==false) && (vt_on==false) )
       {
           // Close device
           LOG_AFE("Afe_DL_Stop, AFE_DL_SRC2_1, close device \n");
           SetAfeReg(AFE_DL_SRC2_1,0x0,0x1);   // DL2_SRC2_ON=0
           usleep(150);
           // Turn off memory interface
           LOG_AFE("Afe_DL_Stop, AFE_DAC_CON0, close memory interface \n");
           SetAfeReg(AFE_DAC_CON0,0x0,0x2);  // DL1_ON=0
        }
       SetAfeReg(AFE_DAC_CON0,0x0,0x20);  // I2S0_ON=0
   }
   else if(OpenMode == AFE_MODE_I2S1_OUT)
   {
       SetI2SControl (false,I2S1OUTPUT);
   }
   else if(OpenMode == AFE_MODE_I2S1_OUT_HDMI)  // for HDMI use
   {
      //------------------------------------------------------------------
      // Disable interrupt (timer)
      ALOGD("Afe_DL_Stop  HDMI, Disable interrupt \n");
      SetAfeReg(AFE_IRQ_CON,0x0,0x1);

      //------------------------------------------------------------------
      // clear IRQ
      LOG_AFE("Afe_DL_Stop  HDMI, clr IRQ \n");
      SetAfeReg(AFE_IR_CLR,0x1,0x1);

      //------------------------------------------------------------------
      // Turn off device . Disable I2S0 master output
#ifndef MTK_TABLET_PLATFORM
      SetAfeReg(AFE_I2S_OUT_CON,0x0000,0x0001);        // bit0: 1:enable I2S1
#endif

      //------------------------------------------------------------------
      // Set connection
      SetAfeReg(AFE_CONN0,0<<22,1<<22);  // Connect -- DL1_I_L to I2S_R,  bit5: I05-O01
      SetAfeReg(AFE_CONN0,0<<6,1<<6);    // Connect -- DL1_I_R to I2S_L,  bit23: I06-O00

      //------------------------------------------------------------------
      // Turn off DL SRC
      SetAfeReg(AFE_DL_SRC2_1,0x0,0x1);  // bit0: DL_2_SRC_ON, // bit1: DL_2_GAIN_ON, DL_2_VOICE_MODE

      if(mAudioHardware != NULL){
         sph_on = mAudioHardware->Get_Recovery_Speech();
         bgs_on = mAudioHardware->Get_Recovery_Bgs();
         rec_on = mAudioHardware->Get_Recovery_Record();
         vt_on  = mAudioHardware->Get_Recovery_VT();
      }
      ALOGD("Afe_DL_Stop, HDMI, sph_on:%d, bgs_on:%d, rec_on:%d, vt_on:%d \n",sph_on,bgs_on,rec_on,vt_on);

      if( (sph_on==false) && (bgs_on==false) && (vt_on==false) )
      {
         //------------------------------------------------------------------
         // Close device
         LOG_AFE("Afe_DL_Stop, HDMI, AFE_DL_SRC2_1, close device \n");
         SetAfeReg(AFE_DL_SRC2_1,0x0,0x1);   // DL2_SRC2_ON=0
         usleep(150);
         //------------------------------------------------------------------
         // Turn off memory interface
         LOG_AFE("Afe_DL_Stop, HDMI, AFE_DAC_CON0, close memory IF \n");
         SetAfeReg(AFE_DAC_CON0,0x0,0x2);  // DL1_ON=0
      }

      // turn off HDMI clock
  #ifndef MTK_TABLET_PLATFORM
      ::ioctl(mFd, AUD_SET_HDMI_CLOCK, 0);
  #endif
   }
   else if(OpenMode == AFE_MODE_DAI)
   {
       uint32 val;
       uint32 regvalue;
       ioctl(mFd,AUDDRV_STOP_DAI_OUTPUT,0); // tell driver DAI stop
       GetAfeReg(AFE_DAIBT_CON,&val);
       ALOGD("Afe_DL_Stop, GetAfeReg AFE_DAIBT_CON = %x",val);
       val &=~ (1<<1);  // BT_ff
       val &= ~(1); // disable DAI_BT
       val &= ~(1<<3); // disable DAI_BT
       SetAfeReg(AFE_DAIBT_CON,val,0xffff); // enable for DAI_BT function , BT_ON=1 , data ready =1
       ALOGD("Afe_DL_Stop, AFE_MODE_DAI, Disable interrupt val = %d",val);
       GetAfeReg(AFE_DAIBT_CON,&regvalue);
       ALOGD("Afe_DL_Stop, GetAfeReg AFE_DAIBT_CON = %x",regvalue);

       // Disable interrupt (timer)
       SetAfeReg(AFE_IRQ_CON,0x0,0x1);
       SetAfeReg(AFE_IRQ_CON,0x0,0x4);// bit0: IRQ3_MCU_OFF
       SetAfeReg(AFE_DAC_CON0,0x00,0x10);  // disable DAI

       if(mAudioHardware != NULL){
           sph_on = mAudioHardware->Get_Recovery_Speech();
           bgs_on = mAudioHardware->Get_Recovery_Bgs();
           rec_on = mAudioHardware->Get_Recovery_Record();
           vt_on  = mAudioHardware->Get_Recovery_VT();
       }
       ALOGD("Afe_DL_Stop, sph_on:%d, bgs_on:%d, rec_on:%d, vt_on:%d \n",sph_on,bgs_on,rec_on,vt_on);
       if( (sph_on==false) && (bgs_on==false) && (vt_on==false) )
       {
           // Close device
           LOG_AFE("Afe_DL_Stop, AFE_DL_SRC2_1, close device \n");
           SetAfeReg(AFE_DL_SRC2_1,0x0,0x1);   // DL2_SRC2_ON=0
           usleep(150);
           // Turn off memory interface
           LOG_AFE("Afe_DL_Stop, AFE_DAC_CON0, close memory interface \n");
           SetAfeReg(AFE_DAC_CON0,0x0,0x2);  // DL1_ON=0
       }
   }
   else if(OpenMode == AFE_MODE_SIGMADSP)
   {

   }
   else if(OpenMode == AFE_MODE_FTM_I2S)
   {
      // Close device
      LOG_AFE("Afe_DL_Stop, AFE_DL_SRC2_1, close device \n");
      SetAfeReg(AFE_DL_SRC2_1,0x0,0x1);   // DL2_SRC2_ON=0

      // Disable connection
      SetAfeReg(AFE_CONN1,0<<21,1<<21);  // Disconnect -- I2SIn_L to DAC_L, bit21: I00-O03
      SetAfeReg(AFE_CONN2,0<<13,1<<13);  // Disconnect -- I2SIn_R to DAC_R,bit13: I01-O04

      // Turn off device . Disable I2S input
      SetAfeReg(AFE_I2S_IN_CON,0x0,0x1);        // bit0: 0:disable I2S input
      SetAfeReg(AFE_FOC_CON,0x5a00,0xffff);

      SetAfeReg(AFE_DAC_CON0, 0x0, 0x20);
#ifndef I2S0_OUTPUT_USE_FLEXL2
             ::ioctl(mFd, STANDBY_I2S_INPUT_STREAM, 0);
             ::ioctl(mFd, CLOSE_I2S_INPUT_STREAM, 0);
#endif

      ::ioctl(mFd, AUD_SET_I2S_CLOCK, 0);// turn on audio clock
   }
   else
   {
      LOG_AFE("Afe_Stop Error no this mode \n");
   }

   uint32 val_dac = 0x0;
   GetAfeReg(AFE_DAC_CON0,&val_dac);
   ALOGD("Afe_DL_Stop, AFE_DAC_CON0 (0x%x) \n",val_dac);
#ifdef MTK_TABLET_PLATFORM
   if(( (val_dac & 0x7E) == 0 )&&(OpenMode != AFE_MODE_I2S1_OUT_HDMI))
#else
   if( (val_dac & 0x7E) == 0 )
#endif
   {
      ALOGD("Afe_DL_Stop, AFE_DAC_CON0, Disable AFE_ON \n");
      SetAfeReg(AFE_DAC_CON0,0x0,0x1); // AFE_ON
   }
	::ioctl(mFd,AUDDRV_BEE_IOCTL,0x51);
}

void AudioAfe::Afe_UL_Stop(AFE_MODE OpenMode)
{
   ALOGD("Afe_UL_Stop OpenMode=%d \n",OpenMode);

   uint32 value = 0x0;
   uint32 mask  = 0x0;

   if(OpenMode == AFE_MODE_ADC)
   {
       uint32 val_dac = 0x0;
      // Used for facotry mode
      SetAfeReg(AFE_UL_SRC_0,0x0,0x1);   // UL_SRC_OFF

      SetAfeReg(AFE_DAC_CON0,0x0,0x8);   // UL_OFF

      // Disable interrupt (timer)
      ALOGD("Afe_UL_Stop, AFE_IRQ_CON, Disable interrupt \n");

      GetAfeReg(AFE_DAC_CON0,&val_dac);
      if( (val_dac & 0x7E) == 0 )
      {
          // clear IRQ1_MCU
          LOG_AFE("Afe_UL_Stop, AFE_IR_CLR, clear IRQ \n");
          SetAfeReg(AFE_IRQ_CON,0x0,0x1);
          SetAfeReg(AFE_IR_CLR,0x1,0x1);
      }

      // Close device
      LOG_AFE("Afe_UL_Stop, AFE_UL_SRC_0, close device \n");
      SetAfeReg(AFE_UL_SRC_0,0x0,0x1);   // UL_SRC_ON=0

   }
   else if(OpenMode == AFE_MODE_I2S_IN)
   {
      // Disable interrupt (timer)
      SetAfeReg(AFE_IRQ_CON,0x0,0x2);  // bit1: I2S,  IRQ2_MCU_ON

      // clear IRQ
      SetAfeReg(AFE_IR_CLR,0x2,0x2);           // bit1: I2S,  IRQ1_MCU_CLR

      // Turn off device . Disable I2S input
      SetAfeReg(AFE_I2S_IN_CON,0x0,0x1);        // bit0: 0:disable I2S input
      SetAfeReg(AFE_FOC_CON,0x5a00,0xffff);

      // Turn off memory interface
      SetAfeReg(AFE_DAC_CON0,0x0,0x20);  // I2S_ON

      // Disable Inter Connect
      SetAfeReg(AFE_CONN3,0<<4,1<<4);  // Connect -- I2S_I_L to I2S_L,  bit4: I00-O07
      SetAfeReg(AFE_CONN3,0<<5,1<<5);  // Connect -- I2S_I_R to I2S_R,  bit5: I01-O08
   }
   else if (OpenMode == AFE_MODE_AWB)
   {
       uint32 val_dac = 0x0;
       GetAfeReg(AFE_DAC_CON0,&val_dac);
       if( (val_dac & 0x7E) == 0 )
       {
           SetAWBConenction (false);
       }
       SetAWBMemoryInterfaceEnable(false,BIT_06);
   }
   else if(OpenMode == AFE_MODE_DAI_IN)
   {

   }
   else
   {
      LOG_AFE("Afe_UL_Stop Error no this mode \n");
   }

   uint32 val_dac = 0x0;
   GetAfeReg(AFE_DAC_CON0,&val_dac);
   ALOGD("Afe_UL_Stop, AFE_DAC_CON0 (0x%x) \n",val_dac);
   if( (val_dac & 0x7E) == 0 )
   {
      //ALOGD("Afe_UL_Stop, AFE_DAC_CON0, Disable AFE_ON \n");
      SetAfeReg(AFE_DAC_CON0,0x0,0x1); // AFE_ON
   }
}

bool AudioAfe::SetI2SControl(bool bEnable,int type,uint32 SampleRate)
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


#ifdef ENABLE_AUDIO_SW_STEREO_TO_MONO
void AudioAfe::Afe_Set_Stereo(void)
{
   uint32 tEnableStereoOutput = true;
   //prevent factory mode to use null mAudioHardware
   if(mAudioHardware != NULL)
   {
     int32 i4CurrentDevice = mAudioHardware->GetCurrentDevice();
     ALOGV("Afe_Set_Stereo CurrentDevice(%x)\n",i4CurrentDevice);
     if((mAudioHardware->mEnableStereoOutput==false)//For CMCC speaker mono/stereo
#if !defined(ENABLE_STEREO_SPEAKER)
        ||(i4CurrentDevice == android_audio_legacy::AudioSystem::DEVICE_OUT_SPEAKER)
        ||(i4CurrentDevice == android_audio_legacy::AudioSystem::DEVICE_OUT_EARPIECE)
#endif
     )
     {
        tEnableStereoOutput = false;
     }
   }
   //~
   if(tEnableStereoOutput == false)
   {
       ALOGD("!!! Afe_Set_Stereo is forced to Afe_Set_Mono().\n");
       SetAfeReg(AFE_CONN1,(1<<27),(1<<27));  // Connect -- DL1_R to DAC_L, bit27: I06-O03
       SetAfeReg(AFE_CONN2,(1<<17),(1<<17));  // Connect -- DL1_L to DAC_R, bit17: I05-O04
       mAudioStereoToMono = true;
   }
   else
   {
       ALOGD("!!! Afe_Set_Stereo \n");
//   SetAfeReg(AFE_CONN2,(0<<4),(1<<4)); // Disconnect -- DL1_L to DAC_L, bit4: I05-O03
//   SetAfeReg(AFE_CONN2,(0<<5),(1<<5)); // Disconnect -- DL1_R to DAC_R, bit5: I06-O03
//   SetAfeReg(AFE_CONN1,(0<<27),(1<<27));  // Disconnect -- DL1_R to DAC_L, bit27: I06-O03
       SetAfeReg(AFE_CONN1,(1<<27),(1<<27));  // Connect -- DL1_R to DAC_L, bit27: I06-O03
       SetAfeReg(AFE_CONN2,(1<<17),(1<<17));  // Connect -- DL1_L to DAC_R, bit17: I05-O04
       mAudioStereoToMono = false;
   }
}

//always LCH->DACL, RCH->DACR
void AudioAfe::Afe_Set_Mono(void)
{

   ALOGD("!!! MTK_AUDIO_SW_S2M  Afe_Set_Mono \n");
   Afe_Set_Stereo();
   mAudioStereoToMono = true;

}

#else
void AudioAfe::Afe_Set_Stereo(void)
{
   uint32 reg_AFE_CONN1,reg_AFE_CONN2;
   LOG_AFE("!!! Afe_Set_Stereo \n");

   SetAfeReg(AFE_CONN2,(0<<4),(1<<4)); // Disconnect -- DL1_L to DAC_L, bit4: I05-O03
   SetAfeReg(AFE_CONN2,(0<<5),(1<<5)); // Disconnect -- DL1_R to DAC_R, bit5: I06-O03

   SetAfeReg(AFE_CONN1,(1<<27),(1<<27));  // Connect -- DL1_R to DAC_L, bit27: I06-O03
   SetAfeReg(AFE_CONN2,(1<<17),(1<<17));  // Connect -- DL1_L to DAC_R, bit17: I05-O04

   GetAfeReg(AFE_CONN1, &reg_AFE_CONN1);
   GetAfeReg(AFE_CONN2, &reg_AFE_CONN2);
   ALOGD("Afe_Set_Stereo AFE_CONN1=%x, AFE_CONN2=%x,\n",reg_AFE_CONN1,reg_AFE_CONN2);
   mAudioStereoToMono = false;
}

//LCH: HW stereo to mono
//RCH: only RCH PCM
void AudioAfe::Afe_Set_Mono(void)
{

   LOG_AFE("!!! Afe_Set_Mono \n");
   Afe_Set_Stereo();
   mAudioStereoToMono = true;

}

#endif

void AudioAfe::Afe_DL_Mute(AFE_MODE OpenMode)
{
   LOG_AFE("!!! Afe_DL_Mute OpenMode=%d \n",OpenMode);
   uint32 value = 0x0;
   uint32 mask  = 0x0;
   if(OpenMode == AFE_MODE_DAC)
   {
/*
      // control MUTE speed
      LOG_AFE("Afe_DL_Mute DL_SRC2_1 mute speed \n");
      SetAfeReg(AFE_DL_SRC2_1,0x600,0x600);  // bit9~10: DL2_MUTE_SPEED (0.002 per sample)
      SET_DL2_MUTE_CH1_ON_on(value);
      SET_DL2_MUTE_CH2_ON_on(value);
      SET_DL2_MUTE_CH1_ON_mask(mask);
      SET_DL2_MUTE_CH2_ON_mask(mask);
      LOG_AFE("Afe_DL_Mute DL_SRC2_CH1_CH2 on \n");
      SetAfeReg(AFE_DL_SRC2_1,value,mask);
*/

      SetAfeReg(AFE_DL_SRC2_1,0x600,0x600);  // bit9~10: DL2_MUTE_SPEED (0.002 per sample)
      SetAfeReg(AFE_DL_SRC2_1,0x18,0x18);    // bit4:DL2_MUTE_CH1_ON, bit3:DL2_MUTE_CH2_ON
   }
   else if(OpenMode == AFE_MODE_I2S0_OUT)
   {

   }
   else if(OpenMode == AFE_MODE_I2S1_OUT)
   {

   }
   else if(OpenMode == AFE_MODE_DAI)
   {

   }
   else if(OpenMode == AFE_MODE_SIGMADSP)
   {

   }
   else
   {
      LOG_AFE("Afe_Stop Error no this mode \n");
   }
}

void AudioAfe::Afe_DL_Unmute(AFE_MODE OpenMode)
{
   LOG_AFE("!!! Afe_DL_Unmute OpenMode=%d \n",OpenMode);
   uint32 value = 0x0;
   uint32 mask  = 0x0;
   if(OpenMode == AFE_MODE_DAC)
   {
/*
      SET_DL2_MUTE_CH1_ON_off(value);
      SET_DL2_MUTE_CH2_ON_off(value);
      SET_DL2_MUTE_CH1_ON_mask(mask);
      SET_DL2_MUTE_CH2_ON_mask(mask);
      LOG_AFE("Afe_DL_Unmute DL_SRC2_CH1_CH2 off \n");
      SetAfeReg(AFE_DL_SRC2_1,value,mask);
*/
      SetAfeReg(AFE_DL_SRC2_1,0x0,0x18);    // bit4:DL2_MUTE_CH1_ON, bit3:DL2_MUTE_CH2_ON
   }
   else if(OpenMode == AFE_MODE_I2S0_OUT)
   {

   }
   else if(OpenMode == AFE_MODE_I2S1_OUT)
   {

   }
   else if(OpenMode == AFE_MODE_DAI)
   {

   }
   else if(OpenMode == AFE_MODE_SIGMADSP)
   {

   }
   else
   {
      LOG_AFE("Afe_Stop Error no this mode \n");
   }
}

void AudioAfe::Afe_UL_Mute(AFE_MODE OpenMode)
{
   LOG_AFE("Afe_UL_Mute OpenMode=%d \n",OpenMode);
}

void AudioAfe::Afe_UL_Unmute(AFE_MODE OpenMode)
{
   LOG_AFE("Afe_UL_Unmute OpenMode=%d \n",OpenMode);
}


void AudioAfe::HQA_AFE_Set_UL_ADC_PGA_Gain(int gain_value)
{
   ALOGD("HQA_AFE_Set_UL_ADC_PGA_Gain, gain_value:0x%x \n",gain_value);
   SetAfeReg(AFE_VAGC_CON0,gain_value<<4,0x000003F0);
   SetAfeReg(AFE_VAGC_CON0,gain_value<<20,0x03F00000);
}
void AudioAfe::HQA_AFE_Set_UL_ADC_Minima_PGA_Gain(int gain_value)
{
   ALOGD("HQA_AFE_Set_UL_ADC_Minima_PGA_Gain, gain_value:0x%x \n",gain_value);
   SetAfeReg(AFE_VAGC_CON0,gain_value<<10,0x0000FC00);
   SetAfeReg(AFE_VAGC_CON0,gain_value<<26,0xFC000000);
}

void AudioAfe::HQA_AFE_Set_I2SRecPly_PATH(int I2SRec_Enable)
{
   ALOGD("HQA_AFE_Set_I2S_PATH, I2SRec_Enable:0x%x \n",I2SRec_Enable);
   m_I2SRec_Enable = I2SRec_Enable;
   if(I2SRec_Enable == 1)
   {
      SetAfeReg(AFE_CONN1,(0<<27),(1<<27)); // Connect -- DL1_R to DAC_L, bit27: I06-O03
      SetAfeReg(AFE_CONN2,(0<<17),(1<<17));   // Connect -- DL1_L to DAC_R, bit17: I05-O04

      mAnalog->SetAnaReg(AUDIO_CON1,0x0C0C,MASK_ALL);
      mAnalog->SetAnaReg(AUDIO_CON2,0x000C,MASK_ALL);
      // Set connection
      SetAfeReg(AFE_CONN1,1<<21,1<<21);  // Connect -- I2SIn_L to DAC_L, bit21: I00-O03
      SetAfeReg(AFE_CONN2,1<<13,1<<13);  // Connect -- I2SIn_R to DAC_R,bit13: I01-O04
   }
   else
   {

   }
}

}

