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
 *   AudioAnalogAfe.h
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
 * 04 10 2012 weiguo.li
 * [ALPS00266592] [Need Patch] [Volunteer Patch] ICS_MP patchback to ALPS.ICS of Audio
 * .
 *
 *******************************************************************************/

#ifndef ANDROID_AUDIO_ANALOGAFE_H
#define ANDROID_AUDIO_ANALOGAFE_H

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
#include "AudioYusuDef.h"


/*****************************************************************************
*                          C O N S T A N T S
******************************************************************************
*/
#ifndef ABB_MDSYS_BASE
#define ABB_MDSYS_BASE           (0xFD114000)
#endif
#define MIX_ABB_REGION           (0x9B4)
#define MIX_PMU_REGION           (0x724)

#define MASK_ALL                  (0xFFFFFFFF)


/*****************************************************************************
*                         D A T A   T Y P E S
******************************************************************************
*/
typedef enum
{
   AnalogAFE_MUTE_L    = 0,
   AnalogAFE_MUTE_R    = 1,
   AnalogAFE_MUTE_ALL  = 2,
   AnalogAFE_MUTE_NONE = 3
} AnalogAFE_MUTE;

typedef enum
{
    AUDIO_PATH,
    VOICE_PATH,
    FM_PATH_MONO,
    FM_PATH_STEREO,
    FM_MONO_AUDIO,
    FM_STEREO_AUDIO,
    NONE_PATH
}AnalogAFE_Mux;


/*****************************************************************************
*                  R E G I S T E R       D E F I N E
******************************************************************************
*/
// AP_MDSYS
#define WR_PATH0         (ABB_MDSYS_BASE+0x0100)
#define WR_PATH1         (ABB_MDSYS_BASE+0x0104)
#define WR_PATH2         (ABB_MDSYS_BASE+0x0108)
#define ABIST_MON_CON0   (ABB_MDSYS_BASE+0x0220)
#define ABIST_MON_CON1   (ABB_MDSYS_BASE+0x0224)
#define ABIST_MON_CON2   (ABB_MDSYS_BASE+0x0228)
#define ABIST_MON_CON3   (ABB_MDSYS_BASE+0x022C)
#define ABIST_MON_CON4   (ABB_MDSYS_BASE+0x0230)
#define ABIST_MON_CON5   (ABB_MDSYS_BASE+0x0234)
#define ABIST_MON_CON6   (ABB_MDSYS_BASE+0x0238)
#define ABIST_MON_CON7   (ABB_MDSYS_BASE+0x023C)
#define ABIST_MON_CON8   (ABB_MDSYS_BASE+0x0240)
#define AUDIO_CON0       (ABB_MDSYS_BASE+0x0300)
#define AUDIO_CON1       (ABB_MDSYS_BASE+0x0304)
#define AUDIO_CON2       (ABB_MDSYS_BASE+0x0308)
#define AUDIO_CON3       (ABB_MDSYS_BASE+0x030C)
#define AUDIO_CON4       (ABB_MDSYS_BASE+0x0310)
#define AUDIO_CON5       (ABB_MDSYS_BASE+0x0314)
#define AUDIO_CON6       (ABB_MDSYS_BASE+0x0318)
#define AUDIO_CON7       (ABB_MDSYS_BASE+0x031C)
#define AUDIO_CON8       (ABB_MDSYS_BASE+0x0320)
#define AUDIO_CON9       (ABB_MDSYS_BASE+0x0324)
#define AUDIO_CON10      (ABB_MDSYS_BASE+0x0328)
#define AUDIO_CON11      (ABB_MDSYS_BASE+0x032C)
#define AUDIO_CON12      (ABB_MDSYS_BASE+0x0330)
#define AUDIO_CON13      (ABB_MDSYS_BASE+0x0334)
#define AUDIO_CON14      (ABB_MDSYS_BASE+0x0338)
#define AUDIO_CON15      (ABB_MDSYS_BASE+0x033C)
#define AUDIO_CON16      (ABB_MDSYS_BASE+0x0340)
#define AUDIO_CON17      (ABB_MDSYS_BASE+0x0344)
#define AUDIO_CON20      (ABB_MDSYS_BASE+0x0380)
#define AUDIO_CON21      (ABB_MDSYS_BASE+0x0384)
#define AUDIO_CON22      (ABB_MDSYS_BASE+0x0388)
#define AUDIO_CON23      (ABB_MDSYS_BASE+0x038C)
#define AUDIO_CON24      (ABB_MDSYS_BASE+0x0390)
#define AUDIO_CON25      (ABB_MDSYS_BASE+0x0394)
#define AUDIO_CON26      (ABB_MDSYS_BASE+0x0398)
#define AUDIO_CON27      (ABB_MDSYS_BASE+0x039C)
#define AUDIO_CON28      (ABB_MDSYS_BASE+0x03A0)
#define AUDIO_CON29      (ABB_MDSYS_BASE+0x03A4)
#define AUDIO_CON30      (ABB_MDSYS_BASE+0x03A8)
#define AUDIO_CON31      (ABB_MDSYS_BASE+0x03AC)
#define AUDIO_CON32      (ABB_MDSYS_BASE+0x03B0)
#define AUDIO_CON33      (ABB_MDSYS_BASE+0x03B4)
#define AUDIO_CON34      (ABB_MDSYS_BASE+0x03B8)
#define AUDIO_CON35      (ABB_MDSYS_BASE+0x03BC)

#define AUDIO_NCP0        (ABB_MDSYS_BASE+0x0400)
#define AUDIO_NCP1        (ABB_MDSYS_BASE+0x0404)
#define AUDIO_LDO0        (ABB_MDSYS_BASE+0x0440)
#define AUDIO_LDO1        (ABB_MDSYS_BASE+0x0444)
#define AUDIO_LDO2        (ABB_MDSYS_BASE+0x0448)
#define AUDIO_LDO3        (ABB_MDSYS_BASE+0x044C)
#define AUDIO_GLB0        (ABB_MDSYS_BASE+0x0480)
#define AUDIO_GLB1        (ABB_MDSYS_BASE+0x0484)
#define AUDIO_GLB2        (ABB_MDSYS_BASE+0x0488)
#define AUDIO_REG0        (ABB_MDSYS_BASE+0x04C0)
#define AUDIO_REG1        (ABB_MDSYS_BASE+0x04C4)

#define BBRX_CON0         (ABB_MDSYS_BASE+0x0A00)
#define BBRX_CON1         (ABB_MDSYS_BASE+0x0A04)
#define BBRX_CON2         (ABB_MDSYS_BASE+0x0A08)
#define BBRX_CON3         (ABB_MDSYS_BASE+0x0A0C)
#define BBRX_CON4         (ABB_MDSYS_BASE+0x0A10)
#define BBRX_CON5         (ABB_MDSYS_BASE+0x0A14)
#define BBRX_CON6         (ABB_MDSYS_BASE+0x0A18)
#define BBRX_CON7         (ABB_MDSYS_BASE+0x0A1C)
#define BBRX_CON8         (ABB_MDSYS_BASE+0x0A20)
#define BBRX_CON9         (ABB_MDSYS_BASE+0x0A24)
#define BBRX_CON10        (ABB_MDSYS_BASE+0x0A28)
#define BBRX_CON11        (ABB_MDSYS_BASE+0x0A2C)
#define BBRX_CON12        (ABB_MDSYS_BASE+0x0A30)
#define BBRX_CON13        (ABB_MDSYS_BASE+0x0A34)
#define BBRX_CON14        (ABB_MDSYS_BASE+0x0A38)
#define BBRX_CON15        (ABB_MDSYS_BASE+0x0A3C)
#define BBRX_CON16        (ABB_MDSYS_BASE+0x0A40)
#define BBRX_CON17        (ABB_MDSYS_BASE+0x0A44)

#define BBTX_CON0         (ABB_MDSYS_BASE+0x0A80)
#define BBTX_CON1         (ABB_MDSYS_BASE+0x0A84)
#define BBTX_CON2         (ABB_MDSYS_BASE+0x0A88)
#define BBTX_CON3         (ABB_MDSYS_BASE+0x0A8C)
#define BBTX_CON4         (ABB_MDSYS_BASE+0x0A90)
#define BBTX_CON5         (ABB_MDSYS_BASE+0x0A94)
#define BBTX_CON6         (ABB_MDSYS_BASE+0x0A98)
#define BBTX_CON7         (ABB_MDSYS_BASE+0x0A9C)
#define BBTX_CON9         (ABB_MDSYS_BASE+0x0AA4)
#define BBTX_CON10        (ABB_MDSYS_BASE+0x0AA8)
#define BBTX_CON11        (ABB_MDSYS_BASE+0x0AAC)
#define BBTX_CON12        (ABB_MDSYS_BASE+0x0AB0)
#define BBTX_CON13        (ABB_MDSYS_BASE+0x0AB4)
#define BBTX_CON14        (ABB_MDSYS_BASE+0x0AB8)
#define BBTX_CON15        (ABB_MDSYS_BASE+0x0ABC)
#define BBTX_CON16        (ABB_MDSYS_BASE+0x0AC0)

#define APC_CON0          (ABB_MDSYS_BASE+0x0C00)
#define APC_CON1          (ABB_MDSYS_BASE+0x0C04)
#define VBIAS_CON0        (ABB_MDSYS_BASE+0x0C40)
#define VBIAS_CON1        (ABB_MDSYS_BASE+0x0C44)
#define AFC_CON0          (ABB_MDSYS_BASE+0x0CC0)
#define AFC_CON1          (ABB_MDSYS_BASE+0x0CC4)
#define AFC_CON2          (ABB_MDSYS_BASE+0x0CC8)

#define BBTX_CON17        (ABB_MDSYS_BASE+0x0F00)
#define BBTX_CON18        (ABB_MDSYS_BASE+0x0F04)
/*****************************************************************************
*                        F U N C T I O N   D E F I N I T I O N
******************************************************************************
*/
namespace android {

class AudioYusuHardware;

class AudioAnalog
{
public:

   AudioAnalog(AudioYusuHardware *hw);
   ~AudioAnalog();

   void SetAnaReg(uint32 offset,uint32 value, uint32 mask);
   void GetAnaReg(uint32 offset,uint32 *value);
   void AnalogAFE_Open(AnalogAFE_Mux mux);
   void AnalogAFE_Close(AnalogAFE_Mux mux);
   void AnalogAFE_ChangeMux(AnalogAFE_Mux mux);
   void AnalogAFE_Depop(AnalogAFE_Mux mux, bool Enable);
   void AnalogAFE_Set_Mute(AnalogAFE_MUTE MuteType);
   void AnalogAFE_EnableHeadset(bool Enable);
   bool AnalogAFE_Init(uint32 Fd);
   bool AnalogAFE_Deinit(void);
   void EnableHeadset(bool Enable);
   void AnalogAFE_Set_DL_AUDHPL_PGA_Gain(int gain_value);
   void AnalogAFE_Set_DL_AUDHPR_PGA_Gain(int gain_value);
   void AnalogAFE_Set_DL_AUDHS_PGA_Gain(int gain_value);
   void AnalogAFE_Recover(void);
   void AnalogAFE_Set_LineIn_Gain(int gain_value);
   void AnalogAFE_Request_ANA_CLK(void);
   void AnalogAFE_Release_ANA_CLK(void);


//#ifdef AUDIO_HQA_SUPPORT
   void HQA_AFE_Set_DL_AUDHS_PGA_Gain(int gain_value);
   void HQA_AFE_Set_DL_AUDHPL_PGA_Gain(int gain_value);
   void HQA_AFE_Set_DL_AUDHPR_PGA_Gain(int gain_value);
   void HQA_AFE_Set_AUD_LineIn_Gain(int gain_value);
   void Afe_Set_AUD_Level_Shift_Buf_L_Gain(int gain_value);
   void Afe_Set_AUD_Level_Shift_Buf_R_Gain(int gain_value);
   void HQA_AFE_Set_AUD_UL_ANA_PreAMP_L_Gain(int gain_value);
   void HQA_AFE_Set_AUD_UL_ANA_PreAMP_R_Gain(int gain_value);
   void HQA_Analog_AFE_Select_Audio_Voice_Buffer(int gain_value);    // 1:Audio Buffer,  2:Voice Buffer
   void HQA_Audio_LineIn_Record(int bEnable);
   void HQA_Audio_LineIn_Play(int bEnable);
//#endif

private:
   AudioYusuHardware *mAudioHardware;
   int    mFd;
   pthread_mutex_t depopMutex; //used for depop flow
//#ifdef AUDIO_HQA_SUPPORT
   int m_audio_voice_DAC_sel;
//#endif
};

}; // namespace android

#endif

