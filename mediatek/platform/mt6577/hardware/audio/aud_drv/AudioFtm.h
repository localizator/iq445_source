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
 *   AudioFtm.h
 *
 * Project:
 * --------
 *   Android Audio Driver
 *
 * Description:
 * ------------
 *   Factory Mode
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
 * 01 11 2012 donglei.ji
 * [ALPS00106007] [Need Patch] [Volunteer Patch]DMNR acoustic loopback feature
 * .
 *
 *
 *******************************************************************************/

#ifndef ANDROID_AUDIO_FTM_H
#define ANDROID_AUDIO_FTM_H

/*****************************************************************************
*                     C O M P I L E R   F L A G S
******************************************************************************
*/

/*****************************************************************************
*                E X T E R N A L   R E F E R E N C E S
******************************************************************************
*/
#include <stdint.h>
#include <sys/types.h>
#include "AudioYusuLad.h"
#include "AudioYusuDef.h"
#include "AudioAfe.h"
#include <pthread.h>



/*****************************************************************************
*                          C O N S T A N T S
******************************************************************************
*/
typedef enum
{
    ANALOG_MIC	= 0x0 ,
    DIGITAL_MIC1 = 0x10,
    DIGITAL_MIC2 = 0x20,
} FTM_MICMODE;


/*****************************************************************************
*                         M A C R O
******************************************************************************
*/

/*****************************************************************************
*                  R E G I S T E R       D E F I N I T I O N
******************************************************************************
*/

/*****************************************************************************
*                        F U N C T I O N   D E F I N I T I O N
******************************************************************************
*/
namespace android {


/*****************************************************************************
*                         D A T A   T Y P E S
******************************************************************************
*/

typedef struct
{
   uint32 rAUDIO_TOP_CON0;
   uint32 rAFE_DAC_CON0;
   uint32 rAFE_TOP_CONTROL_0;
   uint32 rAFE_DL_SRC1_1;
   uint32 rAFE_DL_SRC2_1;
   uint32 rAFE_DL_SRC2_2;
   uint32 rAFE_UL_SRC_1;
   uint32 rAFE_UL_SRC_0;
   uint32 rAFE_UL_AGC0;
   uint32 rAFE_SDM_GAIN_STAGE;
   uint32 rAFE_DL_SDM_CON0;
   uint32 rAFE_CONN1;
   uint32 rAFE_CONN2;
   uint32 rAFE_CONN3;
}FTM_AFE_Rec;

typedef struct
{
   uint32 rAUDIO_CON0;
   uint32 rAUDIO_CON1;
   uint32 rAUDIO_CON2;
   uint32 rAUDIO_CON3;
   uint32 rAUDIO_CON4;
   uint32 rAUDIO_CON5;
   uint32 rAUDIO_CON6;
   uint32 rAUDIO_CON7;
   uint32 rAUDIO_CON8;
   uint32 rAUDIO_CON9;
   uint32 rAUDIO_CON10;
   uint32 rAUDIO_CON14;
   uint32 rAUDIO_CON17;
   uint32 rAUDIO_CON20;
   uint32 rAUDIO_CON21;
   uint32 rAUDIO_CON22;
   uint32 rAUDIO_CON23;
   uint32 rAUDIO_CON24;
   uint32 rAUDIO_CON28;
   uint32 rAUDIO_CON29;
   uint32 rAUDIO_CON30;
   uint32 rAUDIO_CON31;
   uint32 rAUDIO_CON32;
   uint32 rAUDIO_CON33;
   uint32 rAUDIO_CON34;
   uint32 rWR_PATH0;
   uint32 rAUDIO_NCP0;
   uint32 rAUDIO_NCP1;
   uint32 rAUDIO_LDO0;
   uint32 rAUDIO_LDO1;
   uint32 rAUDIO_LDO2;
   uint32 rAUDIO_GLB0;
   uint32 rAUDIO_GLB1;
   uint32 rAUDIO_REG1;
}FTM_Analog_Rec;


// for loopback 
enum ENUM_LOOPBACK_TYPES
{
    NO_LOOPBACK                                 = 0,

    // AP control AFE Loopback
    AP_MAIN_MIC_AFE_LOOPBACK                    = 1,
    AP_HEADSET_MIC_AFE_LOOPBACK                 = 2,
    AP_REF_MIC_AFE_LOOPBACK                     = 3,
    
    // MD control AFE Loopback
    MD_MAIN_MIC_AFE_LOOPBACK                    = 11,
    MD_HEADSET_MIC_AFE_LOOPBACK                 = 12,
    MD_DUAL_MIC_AFE_LOOPBACK                    = 13,

    // MD control Acoustic Loopback
    MD_MAIN_MIC_ACOUSTIC_LOOPBACK               = 21,
    MD_HEADSET_MIC_ACOUSTIC_LOOPBACK            = 22,
    MD_DUAL_MIC_ACOUSTIC_LOOPBACK_WITHOUT_DMNR  = 23,
    MD_DUAL_MIC_ACOUSTIC_LOOPBACK_WITH_DMNR     = 24,

    MD_BT_LOOPBACK                              = 31 // Not support yet
};


/*****************************************************************************
*                        C L A S S   D E F I N I T I O N
******************************************************************************
*/
class AudioYusuHardware;
class AudioAfe;
class AudioAnalog;

class AudioFtm
{
public:
   AudioFtm(AudioYusuHardware *hw, AudioAfe *pAfe, AudioAnalog *pAnaAfe);
   ~AudioFtm();

   bool AudFtm_Init(uint32 Fd, LAD *pLad);

   // for FM Tx and HDMI  factory mode & Meta mode
   bool WavGen_SW_SineWave(bool Enable, uint32 Freq, int type);
   bool WavGen_SWPattern(bool Enable, uint32 Freq, int type);
   static void *FmTx_thread_create(void *arg);
   void FmTx_thread_digital_out(void);
   void FmTx_thread_analog_out(void);
   static void *HDMI_thread_create(void *arg);
   void HDMI_thread_I2SOutput(void);
   void WavGen_AudioRead(char * pBuffer, unsigned int bytes);
   unsigned int WavGen_AudioWrite(void* buffer, unsigned int bytes);

   // for factory mode & Meta mode (Digital part)
   void Afe_Enable_SineWave( bool bEnable );
   void FTM_AFE_Loopback(bool bEnable);
   void FTM_AFE_UL_Loopback(bool bEnable , int digital_mic =0);

   // for factory mode & Meta mode (Analog part)
   void Meta_Open_Analog(AnalogAFE_Mux mux);
   void Meta_Close_Analog(void);
   void FTM_Ana_Loopback(int bEnable);
   void FTM_AnaLpk_on(void);
   void FTM_AnaLpk_off(void);

   // for loopback
   bool SetLoopbackOn(ENUM_LOOPBACK_TYPES eLoopbackType, bool bEarphoneInserted);
   bool SetLoopbackOff();

   void Audio_Set_Speaker_Vol(int level);
   void Audio_Set_Speaker_On(int Channel);
   void Audio_Set_Speaker_Off(int Channel);
   void Audio_Set_HeadPhone_On(int Channel);
   void Audio_Set_HeadPhone_Off(int Channel);
   void Audio_Set_Earpiece_On();
   void Audio_Set_Earpiece_Off();

   int PhoneMic_Receiver_Loopback(char echoflag);
   int PhoneMic_EarphoneLR_Loopback(char echoflag);
   int PhoneMic_SpkLR_Loopback(char echoflag);
   int HeadsetMic_EarphoneLR_Loopback(char bEnable, char bHeadsetMic);
   int HeadsetMic_SpkLR_Loopback(char echoflag);


   pthread_t m_WaveThread;
   bool mAudioSinWave_thread;
   unsigned int IdxAudioPattern;
   unsigned int SizeAudioPattern;
   unsigned char* g_i2VDL_DATA;
   char* mAudioBuffer;
   bool mFlag_Aud_DL1_SlaveOn;

   bool mIsLoopbackDefaultUseLoudSpeaker; // true for laud receiver, otherwise for receiver

private:
   int    mFd;
   int    mSamplingRate;
   AudioYusuHardware *mAudioHardware;
   AudioAfe *mAfe;
   AudioAnalog *mAnalog;
   int    mSineWaveStatus;

   FTM_AFE_Rec ftm_rec_reg;
   FTM_Analog_Rec ftm_ana;

   LAD    *mLad;

   ENUM_LOOPBACK_TYPES eCurrentLoopbackType;
   
   bool bSwapADDA;
   bool bConnectAllADDA;
   bool bUseDMNR;

   pthread_mutex_t mLoopbackMutex;


   bool isMdLoopback(ENUM_LOOPBACK_TYPES eLoopbackType);

   void Aud_Loopback(char b_phone_mic_sel,
                     char b_headset_mic_in,
                     char b_spk_out,
                     char b_receiver_out,
                     char b_headset_out);
};

}; // namespace android

#endif

