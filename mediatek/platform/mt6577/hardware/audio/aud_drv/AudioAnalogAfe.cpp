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
 *   AudioAnalogAfe.cpp
 *
 * Project:
 * --------
 *   Android
 *
 * Description:
 * ------------
 *   Analog AFE Control
 *
 * Author:
 * -------
 *   Stan Huang (mtk01728)
 *
 *------------------------------------------------------------------------------
 * $Revision:$
 * $Modtime:$
 * $Log:$
 * 
 * 07 29 2012 weiguo.li
 * [ALPS00319405] ALPS.JB.BSP.PRA check in CR for Jades
 * .
 *
 * 05 18 2012 weiguo.li
 * [ALPS00279899] FM ʱFMж
 * .
 *
 * 05 16 2012 weiguo.li
 * [ALPS00282348] [MT6577_PHONE]Music output from device before FM launched
 * .
 *
 * 04 10 2012 weiguo.li
 * [ALPS00266592] [Need Patch] [Volunteer Patch] ICS_MP patchback to ALPS.ICS of Audio
 * .
 *
 * 10 20 2011 weiguo.li
 * [ALPS00081607] [Need Patch] [Volunteer Patch]add headphone control
 * .
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
#include <sys/ioctl.h>

#include <utils/Log.h>
#include <utils/String8.h>
#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG  "AudioAnalogAFE"

#ifndef ENABLE_EXT_DAC
#define ENABLE_DEPOP
#endif

#ifdef ENABLE_LOG_ANA
    #define LOG_ANA ALOGD
#else
    #define LOG_ANA ALOGV
#endif

#define HEADPHONE_CONTROL

#include "AudioYusuHardware.h"
#include "AudioAnalogAfe.h"
#include "AudioIoctl.h"
#include "AudioCustomizationInterface.h"

#include "audio_custom_exp.h"

/*****************************************************************************
*                          C O N S T A N T S
******************************************************************************
*/

#define LINE_IN_DEPOP_DELAY (300*1000)      //Uint in us
/*****************************************************************************
*                        F U N C T I O N   D E F I N I T I O N
******************************************************************************
*/

namespace android {

AudioAnalog::AudioAnalog(AudioYusuHardware *hw)
{
   mFd = -1;
   mAudioHardware = hw;
   int ret =0;
   ret |= pthread_mutex_init(&depopMutex, NULL);
   if ( ret != 0 ) ALOGE("Failed to initialize pthread depopMutex!");

   return;
}
AudioAnalog::~AudioAnalog()
{
   return;
}

bool AudioAnalog::AnalogAFE_Init(uint32 Fd)
{
   LOG_ANA("+AnalogAFE_Init \n");
   mFd = Fd;
   if( mFd <0 )
   {
    LOG_ANA("AnalogAFE_Init Fail \n");
    return false;
   }
   return true;
}

bool AudioAnalog::AnalogAFE_Deinit(void)
{
   LOG_ANA("+AnalogAFE_Deinit \n");
   if( mFd <0 )
   {
      LOG_ANA("AnalogAFE_Deinit Fail \n");
      return false;
   }
   close(mFd);
   return true;
}

void AudioAnalog::SetAnaReg(uint32 offset,uint32 value, uint32 mask)
{
   LOG_ANA("SetAnaReg offset=%x, value=%x, mask=%x \n",offset,value,mask);
   Register_Control Reg_Data;
   Reg_Data.offset = offset;
   Reg_Data.value = value;
   Reg_Data.mask = mask;
   ::ioctl(mFd,SET_ANAAFE_REG,&Reg_Data);
}

void AudioAnalog::GetAnaReg(uint32 offset,uint32 *value)
{
   Register_Control Reg_Data;
   Reg_Data.offset = offset;
   Reg_Data.value = 0;
   Reg_Data.mask = 0xffff;
   ::ioctl(mFd,GET_ANAAFE_REG,&Reg_Data);
   *value = Reg_Data.value;
   LOG_ANA("GetAnaReg offset=%x, value=%x \n",offset,*value);
}

void AudioAnalog::AnalogAFE_Open(AnalogAFE_Mux pinmux)
{
   // Change Path
   ALOGD("!!! AnalogAFE_Open pinmux=%d \n",pinmux);
   ::ioctl(mFd,AUDDRV_BEE_IOCTL,0x31);
   AnalogAFE_Request_ANA_CLK();

   if(pinmux == VOICE_PATH){
      AnalogAFE_ChangeMux(pinmux);
   }
   else if(mAudioHardware->GetVoiceStatus() == false){
      AnalogAFE_ChangeMux(pinmux);
   }
   else{
      AnalogAFE_ChangeMux(pinmux);
   }
#ifndef ENABLE_DEPOP
   // For MT6573 E2. Need to set AUDIO_CON9 = 0x3.
   // Need to remove when de pop noise solution is applied.
   SetAnaReg(AUDIO_CON9,0x0003,0xffff);
   SetAnaReg(VAUDN_CON0,0x0001,0xffff);
   SetAnaReg(VAUDP_CON0,0x0001,0xffff);
   SetAnaReg(VAUDP_CON1,0x0000,0xffff);
   SetAnaReg(VAUDP_CON2,0x012B,0xffff);
   ::ioctl(mFd,AUDDRV_BEE_IOCTL,0x61);

#endif
   ::ioctl(mFd,AUDDRV_BEE_IOCTL,0x32);
   if(pinmux == AUDIO_PATH)
   {
#ifdef  ENABLE_EXT_DAC
      mAudioHardware->mAudioCustomFunction->EXT_DAC_TurnOnSpeaker (EXT_DAC_I2S,AUDIO_EXP_DAC_BUF0);
      mAudioHardware->mAudioCustomFunction->EXT_DAC_TurnOnSpeaker (EXT_DAC_I2S,AUDIO_EXP_DAC_BUF1);
#elif defined(ENABLE_DEPOP)  //Add by Charlie Lu.
      //Apply Depop Flow
      AnalogAFE_Depop(AUDIO_PATH,true);
#else
      SetAnaReg(AUDIO_CON3,0x0000,0x0002); // Enable Audio Bias (normal)
      SetAnaReg(AUDIO_CON3,0x0180,0x01f0); // Enable Audio DAC (L/R)
      SetAnaReg(AUDIO_CON3,0x0030,0x0030); // Enable Audio Buffer (L/R)
      SetAnaReg(AUDIO_CON10,0x0020,0xffff); // Zero-Padding off
#endif
   }
   else if(pinmux == FM_PATH_MONO)
   {
      ALOGD("!!! AnalogAFE_Open FM_PATH_MONO \n");
#ifdef ENABLE_DEPOP  //Add by Charlie Lu.
      //Apply Depop Flow
      AnalogAFE_Depop(pinmux,true);
#else
      SetAnaReg(AUDIO_CON3,0x0030,0x0030);  // enable audio left/right buffer
#endif

#ifdef  ENABLE_EXT_DAC
      mAudioHardware->mAudioCustomFunction->EXT_DAC_TurnOnSpeaker (EXT_DAC_LINEIN,AUDIO_EXP_DAC_BUF0);
      mAudioHardware->mAudioCustomFunction->EXT_DAC_TurnOnSpeaker (EXT_DAC_LINEIN,AUDIO_EXP_DAC_BUF1);
#endif
   }
   else if(pinmux == FM_PATH_STEREO)
   {
      ALOGD("!!! AnalogAFE_Open FM_PATH_STEREO \n");
#ifdef ENABLE_DEPOP  //Add by Charlie Lu.
      //Apply Depop Flow
      AnalogAFE_Depop(pinmux,true);
#else
      SetAnaReg(AUDIO_CON3,0x0030,0x0030);  // enable audio left/right buffer
#endif

#ifdef  ENABLE_EXT_DAC
      mAudioHardware->mAudioCustomFunction->EXT_DAC_TurnOnSpeaker (EXT_DAC_LINEIN,AUDIO_EXP_DAC_BUF0);
      mAudioHardware->mAudioCustomFunction->EXT_DAC_TurnOnSpeaker (EXT_DAC_LINEIN,AUDIO_EXP_DAC_BUF1);
#endif
   }
   else if(pinmux == FM_MONO_AUDIO)
   {
      ALOGD("!!! AnalogAFE_Open FM_MONO_AUDIO \n");
#ifdef ENABLE_DEPOP  //Add by Charlie Lu.
      //Apply Depop Flow
      AnalogAFE_Depop(pinmux,true);
#else
      SetAnaReg(AUDIO_CON3,0x0180,0x01f0); // Enable Audio DAC (L/R)
      SetAnaReg(AUDIO_CON3,0x0030,0x0030); // Enable Audio Buffer (L/R)
      SetAnaReg(AUDIO_CON10,0x0020,0xffff); // Zero-Padding off
#endif

#ifdef  ENABLE_EXT_DAC
      mAudioHardware->mAudioCustomFunction->EXT_DAC_TurnOnSpeaker (EXT_DAC_I2S,AUDIO_EXP_DAC_BUF0);
      mAudioHardware->mAudioCustomFunction->EXT_DAC_TurnOnSpeaker (EXT_DAC_I2S,AUDIO_EXP_DAC_BUF1);
      mAudioHardware->mAudioCustomFunction->EXT_DAC_TurnOnSpeaker (EXT_DAC_LINEIN,AUDIO_EXP_DAC_BUF0);
      mAudioHardware->mAudioCustomFunction->EXT_DAC_TurnOnSpeaker (EXT_DAC_LINEIN,AUDIO_EXP_DAC_BUF1);
#endif
   }
   else if(pinmux == FM_STEREO_AUDIO)
   {
      ALOGD("!!! AnalogAFE_Open FM_STEREO_AUDIO \n");
#ifdef ENABLE_DEPOP  //Add by Charlie Lu.
      //Apply Depop Flow
      AnalogAFE_Depop(pinmux,true);
#else
      SetAnaReg(AUDIO_CON3,0x0180,0x01f0); // Enable Audio DAC (L/R)
      SetAnaReg(AUDIO_CON3,0x0030,0x0030); // Enable Audio Buffer (L/R)
      SetAnaReg(AUDIO_CON10,0x0020,0xffff); // Zero-Padding off
#endif

#ifdef  ENABLE_EXT_DAC
      mAudioHardware->mAudioCustomFunction->EXT_DAC_TurnOnSpeaker (EXT_DAC_I2S,AUDIO_EXP_DAC_BUF0);
      mAudioHardware->mAudioCustomFunction->EXT_DAC_TurnOnSpeaker (EXT_DAC_I2S,AUDIO_EXP_DAC_BUF1);
      mAudioHardware->mAudioCustomFunction->EXT_DAC_TurnOnSpeaker (EXT_DAC_LINEIN,AUDIO_EXP_DAC_BUF0);
      mAudioHardware->mAudioCustomFunction->EXT_DAC_TurnOnSpeaker (EXT_DAC_LINEIN,AUDIO_EXP_DAC_BUF1);
#endif
   }

   else if(pinmux == VOICE_PATH)
   {
#ifdef ENABLE_DEPOP  //Add by Charlie Lu.
      //Apply Depop Flow
      AnalogAFE_Depop(pinmux,true);
#else
      SetAnaReg(AUDIO_CON3,0x0000,0x0002); // Enable Audio Bias (normal)
      SetAnaReg(AUDIO_CON3,0x0180,0x01f0); // Enable Voice DAC (R)
      SetAnaReg(AUDIO_CON3,0x0040,0x0040); // Enable Voice Buffer
      SetAnaReg(AUDIO_CON10,0x0020,0xffff); // Zero-Padding off
#endif
#ifdef  ENABLE_EXT_DAC
      mAudioHardware->mAudioCustomFunction->EXT_DAC_TurnOnSpeaker (EXT_DAC_LINEIN,AUDIO_EXP_DAC_BUF0);
      mAudioHardware->mAudioCustomFunction->EXT_DAC_TurnOnSpeaker (EXT_DAC_LINEIN,AUDIO_EXP_DAC_BUF1);
#endif
   }
   else if(pinmux == NONE_PATH)
   {

   }
   else
   {
      ALOGD("AnalogAFE_Open none \n");
   }

   EnableHeadset(true);

    AnalogAFE_Release_ANA_CLK();
	::ioctl(mFd,AUDDRV_BEE_IOCTL,0x33);
}

//this function only work when use external headset amp
void AudioAnalog::EnableHeadset(bool Enable)
{
   ::ioctl(mFd,SET_HEADSET_,Enable);// enable speaker
}

void AudioAnalog::AnalogAFE_Set_DL_AUDHPL_PGA_Gain(int gain_value)
{
   ALOGD("AnalogAFE_Set_DL_AUDHPL_PGA_Gain, gain_value:0x%x \n",gain_value);
   SetAnaReg(AUDIO_CON1,gain_value,0x003f);   // voice buffer gain
}

void AudioAnalog::AnalogAFE_Set_DL_AUDHPR_PGA_Gain(int gain_value)
{
   ALOGD("AnalogAFE_Set_DL_AUDHPR_PGA_Gain, gain_value:0x%x \n",gain_value);
   SetAnaReg(AUDIO_CON2,gain_value,0x003f);   // left audio buffer gain
}

void AudioAnalog::AnalogAFE_Set_DL_AUDHS_PGA_Gain(int gain_value)
{
	ALOGD("AnalogAFE_Set_DL_AUDHS_PGA_Gain, gain_value:0x%x \n",gain_value);
	SetAnaReg(AUDIO_CON1,(gain_value<<8),0x3f00);   // voice buffer gain
}

void AudioAnalog::AnalogAFE_ChangeMux(AnalogAFE_Mux pinmux)
{
   ALOGD("AnalogAFE_ChangeMux pinmux=%d \n",pinmux);
   int fm_mono = 0;//0:stereo, 1:mono
/*
#ifndef ENABLE_STEREO_SPEAKER
   if(mAudioHardware->GetFmSpeakerStatus()){//mono
       fm_mono = 1;
   }
#endif
*/
   switch(pinmux)
   {
      case VOICE_PATH:{
         ALOGD("!!~~~VOICE_PATH, AnalogAFE_ChangeMux pinmux=%d \n",pinmux);
         if(mAudioHardware->GetAnalogLineinEnable()== true)
         {
            SetAnaReg(AUDIO_CON5,0x4600,0xFF00); // Change mux to voice+FM (voice+FM playback)
         }
         else
         {
            SetAnaReg(AUDIO_CON5,0x4440,0xFFF0); // Change mux to voice (voice playback)
         }
         break;
      }
      case AUDIO_PATH:{
         if(mAudioHardware->GetAnalogLineinEnable()== true)
         {
            if(fm_mono){//mono
            SetAnaReg(AUDIO_CON5,0x0660,0x0FF0); // Change mux to audio+FM (audio+FM playback)
            }else{ //stereo
                SetAnaReg(AUDIO_CON5,0x0550,0x0FF0); // Change mux to audio+FM (audio+FM playback)
            }
         }
         else
         {
            SetAnaReg(AUDIO_CON5,0x0440,0x0FF0); // Change mux to audio (audio playback)
         }
         break;
      }
      case FM_PATH_MONO:
      case FM_MONO_AUDIO:
      case FM_PATH_STEREO:
      case FM_STEREO_AUDIO:
      {
         if(fm_mono){//mono
             SetAnaReg(AUDIO_CON5,0x0660,0x0FF0); // Change mux to audio+FM (audio+FM playback)
         }else{ //stereo
             SetAnaReg(AUDIO_CON5,0x0550,0x0FF0); // Change mux to audio+FM (audio+FM playback)
         }
         break;
      }
      default:{
         LOG_ANA("AnalogAFE_ChangeMux with default \n");
         break;
      }
   }
}

void AudioAnalog::AnalogAFE_Depop(AnalogAFE_Mux pinmux, bool bEnable)
{
    UINT32 u4AnaCon_1,u4AnaCon_2;
    LOG_ANA("AnalogAFE_Depop pinmux=%d enable=%d\n",pinmux,bEnable);
    ioctl(mFd,AUD_SET_CLOCK,1);
    AnalogAFE_Request_ANA_CLK();
    pthread_mutex_lock(&depopMutex);
    if (bEnable != false)//depop turn-on sequence
    {
        //Store register
        GetAnaReg(AUDIO_CON1,&u4AnaCon_1);
        GetAnaReg(AUDIO_CON2,&u4AnaCon_2);
        SetAnaReg(AUDIO_CON1,0x2c2c,0xffff);    //Set HS buffer gain (Mute), HPL buffer gain (Mute)
        SetAnaReg(AUDIO_CON2,0x002c,0x00ff);    //Set HPR buffer gain (Mute)
        switch(pinmux)
        {
          case VOICE_PATH:{//Used by VoIP
              SetAnaReg(AUDIO_NCP0,0x102B,0xffff);
              SetAnaReg(AUDIO_NCP1,0x0000/*0x0600*/,0x0E00);
              SetAnaReg(AUDIO_LDO0,0x1030,0x1fff);
              SetAnaReg(AUDIO_LDO1,0x3010,0xffff);
              SetAnaReg(AUDIO_LDO2,0x0011/*0x0013*/,0x0011);
              SetAnaReg(AUDIO_GLB0,0x3D30/*0x2920*/,0xffff);
              SetAnaReg(AUDIO_GLB1,0x0000,0xffff);
              SetAnaReg(AUDIO_REG1,0x0001,0x0001);

              SetAnaReg(AUDIO_CON5 ,0x0000,0xfff0);

              SetAnaReg(AUDIO_CON0,0x0200,0x0f00);//set line-in gain and select ZCD source
              //SetAnaReg(AUDIO_CON1,0x0c0c,0xFFFF);
              //SetAnaReg(AUDIO_CON2,0x000c,0x00FF);
              SetAnaReg(AUDIO_CON6,0x0a33,0x0FFF);
              SetAnaReg(AUDIO_CON7,0x003F,0x003F);
              SetAnaReg(AUDIO_CON8,0x0000,0xffff);
              SetAnaReg(AUDIO_CON14,0x00C0,0x00C0);
              SetAnaReg(AUDIO_CON10,0x01A1,0xffff);
              SetAnaReg(AUDIO_CON9,0x0052,0xfffe);
              usleep(1);
              SetAnaReg(AUDIO_CON9,0x0001,0x0001);


              SetAnaReg(AUDIO_CON3 ,0x0040,0x00f2);
              SetAnaReg(AUDIO_CON17,0x1008,0xffff);
              usleep(10);

              SetAnaReg(AUDIO_CON3 ,0x0160,0x0160);
              usleep(10);

              SetAnaReg(AUDIO_CON5,0x0400,0x0f00);
              SetAnaReg(AUDIO_CON17,0x0008,0xffff);
              usleep(10);
              AnalogAFE_ChangeMux(pinmux);
              break;
          }
          case AUDIO_PATH:
          case FM_MONO_AUDIO:
          case FM_STEREO_AUDIO:
          case FM_PATH_MONO:
          case FM_PATH_STEREO:
          {
              SetAnaReg(AUDIO_NCP0,0x102B,0xffff);
              SetAnaReg(AUDIO_NCP1,0x0000/*0x0600*/,0x0E00);
              SetAnaReg(AUDIO_LDO0,0x1030,0x1fff);
              SetAnaReg(AUDIO_LDO1,0x3010,0xffff);
              SetAnaReg(AUDIO_LDO2,0x0011/*0x0013*/,0x0011);
              SetAnaReg(AUDIO_GLB0,0x3D30/*0x2920*/,0xffff);
              SetAnaReg(AUDIO_GLB1,0x0000,0xffff);
              SetAnaReg(AUDIO_REG1,0x0001,0x0001);

              SetAnaReg(AUDIO_CON5 ,0x0000,0xfff0);

              SetAnaReg(AUDIO_CON0,0x0200,0x0f00);//set line-in gain and select ZCD source
              //SetAnaReg(AUDIO_CON1,0x0c0c,0xFFFF);
              //SetAnaReg(AUDIO_CON2,0x000c,0x00FF);
              SetAnaReg(AUDIO_CON6,0x0a33,0x0FFF);
              SetAnaReg(AUDIO_CON7,0x003F,0x003F);
              SetAnaReg(AUDIO_CON8,0x0000,0xffff);
              SetAnaReg(AUDIO_CON14,0x00C0,0x00C0);
              SetAnaReg(AUDIO_CON10,0x01A1,0xffff);
              SetAnaReg(AUDIO_CON9,0x0052,0xfffe);
              usleep(1);
              SetAnaReg(AUDIO_CON9,0x0001,0x0001);

              SetAnaReg(AUDIO_CON3 ,0x0040,0x00f2);
              SetAnaReg(AUDIO_CON17,0x10C8,0xffff);
              usleep(10);

              SetAnaReg(AUDIO_CON3 ,0x01F0,0x01F0);
              usleep(10);
              if((pinmux == FM_MONO_AUDIO)||(pinmux == FM_STEREO_AUDIO)||(pinmux == FM_PATH_MONO)||(pinmux == FM_PATH_STEREO))
              {
                  usleep(LINE_IN_DEPOP_DELAY);
              }
              AnalogAFE_ChangeMux(pinmux);
              SetAnaReg(AUDIO_CON17,0x0008,0xffff);
              SetAnaReg(AUDIO_CON3,0x01B0,0x01F0);
              break;
          }
          default:{
              ALOGD("AnalogAFE_ChangeMux with default \n");
              break;
          }
        }
        //Restore buffer gain
        SetAnaReg(AUDIO_CON1,u4AnaCon_1,0xffff);    //Restore HS/HPL buffer gain
        SetAnaReg(AUDIO_CON2,u4AnaCon_2,0xffff);    //Restore HPR buffer gain
        SetAnaReg(AUDIO_CON0,0x0000,0x0040);        //set timeout selection = 5ms
		SetAnaReg(AUDIO_CON0,0x0003,0x0003);        //enable ZCD
    }else //depop turn-off sequence
    {
        //Store register
        GetAnaReg(AUDIO_CON1,&u4AnaCon_1);
        GetAnaReg(AUDIO_CON2,&u4AnaCon_2);
        if((pinmux != FM_MONO_AUDIO)&&(pinmux != FM_STEREO_AUDIO))
        {
            //<1>SW Amp Gain Ramp-down
            UINT32 u4Gain = u4AnaCon_2 & 0x00ff;
            while(u4Gain < 0x2c)
            {
                u4Gain+=2;
                SetAnaReg(AUDIO_CON1,u4Gain,0x00ff);
                SetAnaReg(AUDIO_CON1,u4Gain<<16,0xff00);
                SetAnaReg(AUDIO_CON2,u4Gain,0x00ff);
                usleep(100);
            }
        }
        //<2>
        SetAnaReg(AUDIO_CON5,0x0000,0xfff0);	//Forec to set HP's pinmux as open + HS
        SetAnaReg(AUDIO_CON3,0x0000,0x01f0);	//power down dac and buffer
        SetAnaReg(AUDIO_CON3,0x0002,0x0002);	//power down audio bias
        usleep(10);
        //<3>Turn off Audio PMU

        SetAnaReg(AUDIO_REG1,0x0000,0x0001);
        SetAnaReg(AUDIO_NCP1,0x0800,0x0E00);
        SetAnaReg(AUDIO_NCP0,0x002B,0x1000);
        //SetAnaReg(AUDIO_LDO2,0x0013,0x0013);//Not to disable LD02 for record mute issue.
        if(mAudioHardware==NULL) //In factory mode, this handle is null.
        {
            SetAnaReg(AUDIO_LDO2,0x0000,0x0013);
        }
        else if(mAudioHardware->Get_Recovery_Record() == false )
        {
            SetAnaReg(AUDIO_LDO2,0x0000,0x0013);
        }
        SetAnaReg(AUDIO_CON9,0x0008,0x000b);
        SetAnaReg(AUDIO_LDO1,0x0000,0x0010);
        SetAnaReg(AUDIO_CON14,0x0,0x00c0);//power down audio low-noise biasgen

        usleep(100);
        //Restore buffer gain
        SetAnaReg(AUDIO_CON1,u4AnaCon_1,0xffff);	  //Restore HS/HPL buffer gain
        SetAnaReg(AUDIO_CON2,u4AnaCon_2,0xffff);	  //Restore HPR buffer gain
    }
    pthread_mutex_unlock(&depopMutex);
    AnalogAFE_Release_ANA_CLK();
    ioctl(mFd,AUD_SET_CLOCK,0);
}

void AudioAnalog::AnalogAFE_Close(AnalogAFE_Mux pinmux)
{
   ALOGD("!!! AnalogAFE_Close pinmux=%d \n",pinmux);

   ::ioctl(mFd,AUDDRV_BEE_IOCTL,0x40);
   if(mAudioHardware->Get_Recovery_Speech() == true){
      ALOGD("AnalogAFE_Close Speech=true \n");
   }
   if(mAudioHardware->Get_Recovery_Record() == true){
      ALOGD("AnalogAFE_Close Record=true \n");
   }
   if(mAudioHardware->Get_Recovery_Bgs() == true){
      ALOGD("AnalogAFE_Close BGS=true \n");
   }
   if(mAudioHardware->Get_Recovery_VT() == true){
      ALOGD("AnalogAFE_Close VT=true \n");
   }

   if((pinmux == FM_PATH_MONO) || (pinmux == FM_PATH_STEREO) )
   {
      if(mAudioHardware->GetVoiceStatus() == false )
      {
         EnableHeadset (false);
         ALOGD("!!! AnalogAFE_Close, FM_PATH_MONO, FM_PATH_STEREO \n");
         //This will cause pop noise
         //AnalogAFE_ChangeMux(AUDIO_PATH);
#ifdef ENABLE_DEPOP
         AnalogAFE_Depop(FM_PATH_STEREO, false);
#else
         SetAnaReg(AUDIO_CON3,0x0000,0x0030);   // disable Audio Buffer
         SetAnaReg(AUDIO_CON3,0x0000,0x0180);   // disable DAC
//         SetAnaReg(AUDIO_CON3,0x0002,0x0002);   // audio bias power down
#endif
#ifdef ENABLE_EXT_DAC
         mAudioHardware->mAudioCustomFunction->EXT_DAC_TurnOffSpeaker (EXT_DAC_LINEIN,AUDIO_EXP_DAC_BUF0);
         mAudioHardware->mAudioCustomFunction->EXT_DAC_TurnOffSpeaker (EXT_DAC_LINEIN,AUDIO_EXP_DAC_BUF1);
#endif
      }
   }
   else if(pinmux == AUDIO_PATH)
   {
      if(mAudioHardware->Get_Recovery_Speech() == false && mAudioHardware->GetVoiceStatus() == false && mAudioHardware->GetAnalogLineinEnable() == false )
      {// no voice and no FM
#ifdef HEADPHONE_CONTROL
        mAudioHardware->ForceDisableHeadPhone();
#endif
        mAudioHardware->ForceDisableSpeaker();
         EnableHeadset (false);
#ifdef ENABLE_DEPOP
         AnalogAFE_Depop(AUDIO_PATH, false);
#else
         SetAnaReg(AUDIO_CON3,0x0000,0x0030);   // disable Audio Buffer
         SetAnaReg(AUDIO_CON3,0x0000,0x0180);   // disable DAC
//         SetAnaReg(AUDIO_CON3,0x0002,0x0002);   // audio bias power down
#endif
#ifdef ENABLE_EXT_DAC
         mAudioHardware->mAudioCustomFunction->EXT_DAC_TurnOffSpeaker (EXT_DAC_I2S,AUDIO_EXP_DAC_BUF0);
         mAudioHardware->mAudioCustomFunction->EXT_DAC_TurnOffSpeaker (EXT_DAC_I2S,AUDIO_EXP_DAC_BUF1);
#endif
      }
      else if(mAudioHardware->GetAnalogLineinEnable()==true && mAudioHardware->GetVoiceStatus() == false)
      {// no voice and Enable FM
         EnableHeadset (false);
         //Note: AnalogAFE_Depop can't be used because duing fm analog, click tone will interrupt fm audio
         //SetAnaReg(AUDIO_CON3,0x0030,0x0030);// FM on --> turn on buffer
         //AnalogAFE_ChangeMux(AUDIO_PATH);//FM_PATH_MONO or FM_MONO_AUDIO

         EnableHeadset (true);
         if(mAudioHardware->GetFmAnalogInEnable () &&
            (mAudioHardware->GetFmSpeakerStatus() || (mAudioHardware->GetCurrentDevice() == android_audio_legacy::AudioSystem::DEVICE_OUT_SPEAKER)))
         {
            mAudioHardware->ForceEnableSpeaker ();
         }
         else if(mAudioHardware->GetMatvAnalogEnable())
         {
         	mAudioHardware->SetMatvSpeaker();
         }
         else
         {
            mAudioHardware->ForceDisableSpeaker();
            ALOGW("AnalogAFE_Close GetAnalogLineinEnable, no SPK");
         }

#ifdef ENABLE_EXT_DAC
         mAudioHardware->mAudioCustomFunction->EXT_DAC_TurnOffSpeaker (EXT_DAC_LINEIN,AUDIO_EXP_DAC_BUF0);
         mAudioHardware->mAudioCustomFunction->EXT_DAC_TurnOffSpeaker (EXT_DAC_LINEIN,AUDIO_EXP_DAC_BUF1);
         mAudioHardware->mAudioCustomFunction->EXT_DAC_TurnOnSpeaker (EXT_DAC_LINEIN,AUDIO_EXP_DAC_BUF0);
         mAudioHardware->mAudioCustomFunction->EXT_DAC_TurnOnSpeaker (EXT_DAC_LINEIN,AUDIO_EXP_DAC_BUF1);
#endif
      }
      else if(mAudioHardware->GetVoiceStatus() == true)
      {// enable voice
         /* //Disable. In phone call, it's not needed to set hw. (HW is controlled by Modem.)
         ::ioctl(mFd,SET_SPEAKER_OFF,Channel_Stereo);// disable SPEAKER_CHANNEL
         EnableHeadset (false);
         SetAnaReg(AUDIO_CON3,0x0180,0x01f0);  // Enable Audio DAC (L/R)
         SetAnaReg(AUDIO_CON3,0x0030,0xffff);  // turn on Audio bufferNumber
         EnableHeadset (true);
         mAudioHardware->EnableSpeaker();
         */
         #ifdef ENABLE_EXT_DAC
         mAudioHardware->mAudioCustomFunction->EXT_DAC_TurnOffSpeaker (EXT_DAC_LINEIN,AUDIO_EXP_DAC_BUF0);
         mAudioHardware->mAudioCustomFunction->EXT_DAC_TurnOffSpeaker (EXT_DAC_LINEIN,AUDIO_EXP_DAC_BUF1);
         #endif
      }
      else
      {
         /* This will cause pop noise
         SetAnaReg(AUDIO_CON3,0x0000,0x0030);   // disable Audio Buffer
         SetAnaReg(AUDIO_CON3,0x0000,0x0180);   // disable DAC
         */
      }
   }
   else if(pinmux == NONE_PATH)
   {
      SetAnaReg(AUDIO_CON3,0x0000,0x0180);   // disable DAC
      SetAnaReg(AUDIO_CON3,0x0000,0x0030);   // disable Audio Buffer
   }
   else
   {
      EnableHeadset (false);
      SetAnaReg(AUDIO_CON3,0x0000,0x0030);   // disable Audio Buffer
      SetAnaReg(AUDIO_CON3,0x0000,0x0180);   // disable DAC

#ifdef  ENABLE_EXT_DAC
      mAudioHardware->mAudioCustomFunction->EXT_DAC_TurnOffSpeaker (EXT_DAC_I2S,AUDIO_EXP_DAC_BUF0);
      mAudioHardware->mAudioCustomFunction->EXT_DAC_TurnOffSpeaker (EXT_DAC_I2S,AUDIO_EXP_DAC_BUF1);
#endif
   }

   ::ioctl(mFd,AUDDRV_BEE_IOCTL,0x41);
}

void AudioAnalog::AnalogAFE_Set_Mute(AnalogAFE_MUTE MuteType)
{
   LOG_ANA("!!! AnalogAFE_Set_Mute MuteType=%d \n",MuteType);

   static uint32 Mute_L;
   static uint32 Mute_R;
   switch (MuteType)
   {
      case (AnalogAFE_MUTE_L):
         GetAnaReg(AUDIO_CON1,&Mute_L);
         SetAnaReg(AUDIO_CON1,0x003F,0xffff);
         break;
      case (AnalogAFE_MUTE_R):
         GetAnaReg(AUDIO_CON2,&Mute_R);
         SetAnaReg(AUDIO_CON2,0x003F,0xffff);
         break;
      case (AnalogAFE_MUTE_ALL):
         GetAnaReg(AUDIO_CON1,&Mute_L);
         GetAnaReg(AUDIO_CON2,&Mute_R);
         SetAnaReg(AUDIO_CON1,0x003F,0xffff);
         SetAnaReg(AUDIO_CON2,0x003F,0xffff);
         break;
      case (AnalogAFE_MUTE_NONE):
         SetAnaReg(AUDIO_CON1,Mute_L,0xffff);
         SetAnaReg(AUDIO_CON2,Mute_R,0xffff);
         break;
      default:
         LOG_ANA("AnalogAFE_Set_Mute default not find \n");
   }

}

void AudioAnalog::AnalogAFE_Set_LineIn_Gain(int gain_value)
{
    ALOGD("AnalogAFE_Set_LineIn_Gain gain_value = %d",gain_value);
    SetAnaReg(AUDIO_CON0,gain_value<<12,0xf000);
}

void AudioAnalog::AnalogAFE_Recover(void)
{
    ::ioctl(mFd,AUDDRV_MD_RST_RECOVERY,0);
}

void AudioAnalog::AnalogAFE_Request_ANA_CLK(void)
{
    int mANA_CLK_CNT = 0;
    ::ioctl(mFd,AUD_SET_ANA_CLOCK,1);
    mANA_CLK_CNT = ::ioctl(mFd,AUD_GET_ANA_CLOCK_CNT,0);
    LOG_ANA("AnalogAFE_RequestANACLK = %d\n",mANA_CLK_CNT);
}

void AudioAnalog::AnalogAFE_Release_ANA_CLK(void)
{
    ::ioctl(mFd,AUD_SET_ANA_CLOCK,0);
}

#if 1
void AudioAnalog::HQA_AFE_Set_DL_AUDHS_PGA_Gain(int gain_value)
{
   ALOGD("HQA_AFE_Set_DL_AUDHS_PGA_Gain, gain_value:0x%x \n",gain_value);
   SetAnaReg(AUDIO_CON1,(gain_value<<8),0x3f00);   // voice buffer gain
}

void AudioAnalog::HQA_AFE_Set_DL_AUDHPL_PGA_Gain(int gain_value)
{
   ALOGD("HQA_AFE_Set_DL_AUDHPL_PGA_Gain, gain_value:0x%x \n",gain_value);
   SetAnaReg(AUDIO_CON1,gain_value,0x003f);   // left audio buffer gain
}

void AudioAnalog::HQA_AFE_Set_DL_AUDHPR_PGA_Gain(int gain_value)
{
   ALOGD("HQA_AFE_Set_DL_AUDHPR_PGA_Gain, gain_value:0x%x \n",gain_value);
   SetAnaReg(AUDIO_CON2,gain_value,0x003f);   // right audio buffer gain
}

void AudioAnalog::HQA_AFE_Set_AUD_LineIn_Gain(int gain_value)
{
   ALOGD("HQA_AFE_Set_AUD_LineIn_Gain, gain_value:0x%x \n",gain_value);
   SetAnaReg(AUDIO_CON0,0x0003,0x000f);   // AUD ZCD enable
   SetAnaReg(AUDIO_CON0,0x0040,0x00f0);   // AUD ZCD timeout Select
   SetAnaReg(AUDIO_CON0,0x0100,0xf00);    // AUD ZCD enable
   SetAnaReg(AUDIO_CON0,(gain_value<<12),0xf000);
}

void AudioAnalog::Afe_Set_AUD_Level_Shift_Buf_L_Gain(int gain_value)
{
   ALOGD("Afe_Set_AUD_Level_Shift_Buf_L_Gain, gain_value:0x%x \n",gain_value);
   SetAnaReg(AUDIO_CON4,(gain_value<<8),0x3f00);   // left audio buffer gain
}

void AudioAnalog::Afe_Set_AUD_Level_Shift_Buf_R_Gain(int gain_value)
{
   ALOGD("Afe_Set_AUD_Level_Shift_Buf_R_Gain, gain_value:0x%x \n",gain_value);
   SetAnaReg(AUDIO_CON4,gain_value,0x003f);   // right audio buffer gain
}

void AudioAnalog::HQA_AFE_Set_AUD_UL_ANA_PreAMP_L_Gain(int gain_value)
{
   ALOGD("HQA_AFE_Set_AUD_UL_ANA_PreAMP_L_Gain, gain_value:0x%x \n",gain_value);
   SetAnaReg(AUDIO_CON20,(gain_value<<8),0x3f00);   // left ADC audio pre-amplifier buffer gain
}

void AudioAnalog::HQA_AFE_Set_AUD_UL_ANA_PreAMP_R_Gain(int gain_value)
{
   ALOGD("HQA_AFE_Set_AUD_UL_ANA_PreAMP_R_Gain, gain_value:0x%x \n",gain_value);
   SetAnaReg(AUDIO_CON20,gain_value,0x003f);   // right ADC audio pre-amplifier buffer gain
}

void AudioAnalog::HQA_Analog_AFE_Select_Audio_Voice_Buffer(int audio_voice_DAC_sel)    // 1:Audio Buffer,  2:Voice Buffer
{
   ALOGD("HQA_Analog_AFE_Select_Audio_Voice_Buffer, audio_voice_DAC_sel:0x%x \n",audio_voice_DAC_sel);

   m_audio_voice_DAC_sel = audio_voice_DAC_sel;

   if(audio_voice_DAC_sel == 1)  // 1:Audio Buffer,  2:Voice Buffer
   {
      // Pin Mux enable for audio/voice
      SetAnaReg(AUDIO_CON5,0x0440,0xFFF0); // Change mux to audio/voice
      // Enable Audio DAC (L/R) and Voice DAC
      SetAnaReg(AUDIO_CON3,0x01B0,0x01f0);
      SetAnaReg(AUDIO_CON1,0x0C0C,MASK_ALL);
      SetAnaReg(AUDIO_CON2,0x000C,MASK_ALL);
      SetAnaReg(AUDIO_CON10,0x0020,0x0020); // Zero-Padding off
   }
   else if(audio_voice_DAC_sel == 2)  // 1:Audio Buffer,  2:Voice Buffer
   {
      // Pin Mux enable for audio/voice
      SetAnaReg(AUDIO_CON5,0x4400,0xFFF0); // Change mux to audio/voice
      // Enable Audio DAC (L/R) and Voice DAC
      SetAnaReg(AUDIO_CON3,0x0160,0x01f0);
      SetAnaReg(AUDIO_CON1,0x0C0C,MASK_ALL);
      SetAnaReg(AUDIO_CON2,0x000C,MASK_ALL);
      SetAnaReg(AUDIO_CON10,0x0020,0x0020); // Zero-Padding off
   }
}

void AudioAnalog::HQA_Audio_LineIn_Record(int bEnable)
{
   if(bEnable == true)
   {
      mAudioHardware->mRecordFM = bEnable;
      mAudioHardware->mFmStatus = bEnable;

      ioctl(mFd,START_DL1_STREAM,0);        // init DL1 Stream
      // enable Digital AFE
      // ...
      // enable Analog AFE
      SetAnaReg(AUDIO_CON0,(0x4<<12),0xf000);
      SetAnaReg(AUDIO_CON1,0x0C0C,MASK_ALL);
      SetAnaReg(AUDIO_CON2,0x000C,MASK_ALL);
      SetAnaReg(AUDIO_CON3,0x0070,MASK_ALL);  // enable voice buffer, audio left/right buffer
//      SetAnaReg(AUDIO_CON5,0x0220,MASK_ALL);  // FM mono playback (analog line in)
      SetAnaReg(AUDIO_CON5,0x0110,MASK_ALL);  // FM stereo playback (analog line in)
	    AnalogAFE_Depop(FM_PATH_STEREO,true);
      mAudioHardware->mAfe_handle->Afe_DL_Unmute(AFE_MODE_DAC);
      mAudioHardware->ForceEnableSpeaker();
   }
   else
   {
      mAudioHardware->mRecordFM = bEnable;
      mAudioHardware->mFmStatus = bEnable;

      mAudioHardware->mAfe_handle->Afe_DL_Mute(AFE_MODE_DAC);
      mAudioHardware->ForceDisableSpeaker();
      // disable Digital AFE
      // ...
      // disable Analog AFE
      SetAnaReg(AUDIO_CON0,0,0xf000);
      SetAnaReg(AUDIO_CON3,0x0000,0x00f0);  // disable voice buffer, audio left/right buffer
      SetAnaReg(AUDIO_CON5,0x0440,0x0ff0);  // disable FM mono playback (analog line in)
	    AnalogAFE_Depop(FM_PATH_STEREO,false);
      ioctl(mFd,STANDBY_DL1_STREAM,0);
   }
}

void AudioAnalog::HQA_Audio_LineIn_Play(int bEnable)
{
   if(bEnable == true)
   {
      ioctl(mFd,START_DL1_STREAM,0);        // init DL1 Stream
      // enable Digital AFE
      // ...
      // enable Analog AFE
      SetAnaReg(AUDIO_CON0,(0x4<<12),0xf000);
      SetAnaReg(AUDIO_CON1,0x0C0C,MASK_ALL);
      SetAnaReg(AUDIO_CON2,0x000C,MASK_ALL);
      SetAnaReg(AUDIO_CON3,0x0070,MASK_ALL);  // enable voice buffer, audio left/right buffer
//      SetAnaReg(AUDIO_CON5,0x0220,MASK_ALL);  // FM mono playback (analog line in)
      SetAnaReg(AUDIO_CON5,0x0110,MASK_ALL);  // FM stereo playback (analog line in)
	    AnalogAFE_Depop(FM_PATH_STEREO,true);
      mAudioHardware->mAfe_handle->Afe_DL_Unmute(AFE_MODE_DAC);
      mAudioHardware->ForceEnableSpeaker();
   }
   else
   {
      mAudioHardware->mAfe_handle->Afe_DL_Mute(AFE_MODE_DAC);
      mAudioHardware->ForceDisableSpeaker();
      // disable Digital AFE
      // ...
      // disable Analog AFE
      SetAnaReg(AUDIO_CON0,0,0xf000);
      SetAnaReg(AUDIO_CON3,0x0000,0x00f0);  // disable voice buffer, audio left/right buffer
      SetAnaReg(AUDIO_CON5,0x0440,0x0ff0);  // disable FM mono playback (analog line in)
	    AnalogAFE_Depop(FM_PATH_STEREO,false);
      ioctl(mFd,STANDBY_DL1_STREAM,0);
   }
}
#endif


}
