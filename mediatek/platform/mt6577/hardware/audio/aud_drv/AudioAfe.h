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
 *   AudioAfe.h
 *
 * Project:
 * --------
 *   Android Audio Driver
 *
 * Description:
 * ------------
 *   AFE register
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
 *
 *******************************************************************************/

#ifndef ANDROID_AUDIO_AFE_H
#define ANDROID_AUDIO_AFE_H

/*****************************************************************************
*                     C O M P I L E R   F L A G S
******************************************************************************
*/
//#define LOG_ASM

/*****************************************************************************
*                E X T E R N A L   R E F E R E N C E S
******************************************************************************
*/
#include <stdint.h>
#include <sys/types.h>
#include "AudioYusuDef.h"
#include "AudioAfe.h"
#include <pthread.h>

/*****************************************************************************
*                         D A T A   T Y P E S
******************************************************************************
*/
typedef enum
{
// for playback
   AFE_MODE_DAC = 0,
   AFE_MODE_I2S0_OUT,
   AFE_MODE_I2S1_OUT,
   AFE_MODE_I2S1_OUT_HDMI,
   AFE_MODE_DAI,
   AFE_MODE_SIGMADSP,
   AFE_MODE_FTM_I2S,   // factory mode use
// for recording
   AFE_MODE_ADC,
   AFE_MODE_DMIC,
   AFE_MODE_I2S_IN,
   AFE_MODE_DAI_IN,
   AFE_MODE_AWB,
   AFE_MODE_NONE,
} AFE_MODE;

enum FMTX_Command
{
    FREQ_NONE = 0,
    FREQ_1K_HZ,
    FREQ_2K_HZ,
    FREQ_3K_HZ,
    FREQ_4K_HZ,
    FREQ_5K_HZ,
    FREQ_6K_HZ,
    FREQ_7K_HZ,
    FREQ_8K_HZ,
    FREQ_9K_HZ,
    FREQ_10K_HZ,
    FREQ_11K_HZ,
    FREQ_12K_HZ,
    FREQ_13K_HZ,
    FREQ_14K_HZ,
    FREQ_15K_HZ
};

enum UL_SAMPLERATE_INDEX
{
    UPLINK8K = 0,
    UPLINK16K,
    UPLINK32K,
    UPLINK48K,
    UPLINK_UNDEF
};


/*****************************************************************************
*                          C O N S T A N T S
******************************************************************************
*/
#define MASK_ALL (0xFFFFFFFF)
#define UPLINK_DIR    0
#define DOWNLINK_DIR  1

#define	FS_8000HZ   0
#define	FS_11025HZ  1
#define	FS_12000HZ  2
#define	FS_16000HZ  3
#define	FS_22050HZ  4
#define	FS_24000HZ  5
#define	FS_32000HZ  6
#define	FS_44100HZ  7
#define	FS_48000HZ  8

#ifndef AFE_BASE
#define AFE_BASE           0xFD000000
#endif

#ifndef AFE_DATA_BASE
#define AFE_DATA_BASE      0xFD400000
#endif

#define AFE_REGION     0x3CC
#define IRQ1_MCU 1
#define IRQ2_MCU 2
#define IRQ3_MCU 3

/*****************************************************************************
*                         M A C R O
******************************************************************************
*/

#define Enable(value,bits)          (value|=bits)
#define Disable(value,bits)         (value&= ~bits)

#define Connect_(value,bits)        (value|=bits)
#define Disconnect_(value,bits)     (value&= ~bits)

/*****************************************************************************
*                  R E G I S T E R       D E F I N I T I O N
******************************************************************************
*/

#define AUDIO_TOP_CON0    (0x0000)
#define AFE_DAC_CON0      (0x0010)
#define AFE_DAC_CON1      (0x0014)
#define AFE_I2S_IN_CON    (0x0018)
#define AFE_FOC_CON       (0x0170)
#define AFE_DAIBT_CON     (0x001c)

#define AFE_CONN0         (0x0020)
#define AFE_CONN1         (0x0024)
#define AFE_CONN2         (0x0028)
#define AFE_CONN3         (0x002C)
#define AFE_CONN4         (0x0030)
#define AFE_I2S_OUT_CON   (0x0034)
#define AFE_DL1_BASE      (0x0040)
#define AFE_DL1_CUR       (0x0044)
#define AFE_DL1_END       (0x0048)
#define AFE_DL2_BASE      (0x0050)
#define AFE_DL2_CUR       (0x0054)
#define AFE_DL2_END       (0x0058)
#define AFE_I2S_BASE      (0x0060)
#define AFE_I2S_CUR       (0x0064)
#define AFE_I2S_END       (0x0068)
#define AFE_AWB_BASE      (0x0070)
#define AFE_AWB_CUR       (0x0074)
#define AFE_AWB_END       (0x0078)
#define AFE_VUL_CUR       (0x0084)
#define AFE_DAI_CUR       (0x0094)

#define AFE_DL_SRC1_1     (0x0100) //reserved
#define AFE_DL_SRC1_2     (0x0104) //reserved
#define AFE_DL_SRC2_1     (0x0108)
#define AFE_DL_SRC2_2     (0x010C)
#define AFE_DL_SDM_CON0   (0x0110)
#define AFE_UL_SRC_0      (0x0114)
#define AFE_UL_SRC_1      (0x0118)

#define AFE_IRQ_CON       (0x03A0)
#define AFE_IR_STATUS     (0x03A4)
#define AFE_IR_CLR        (0x03A8)
#define AFE_IRQ_CNT1      (0x03AC)
#define AFE_IRQ_CNT2      (0x03B0)
#define AFE_IRQ_MON       (0x03B8)

// IRQ for Modem part
#define AFE_MODEM_IRQ_CON       (0x00A0)
#define AFE_MODEM_IR_STATUS     (0x00A4)
#define AFE_MODEM_IR_CLR        (0x00A8)
#define AFE_MODEM_IRQ_CNT1      (0x00AC)
#define AFE_MODEM_IRQ_CNT2      (0x00B0)

//Register : AGC
#define AFE_UL_AGC0        (0x020c)
#define AFE_UL_AGC1        (0x0120)
#define AFE_UL_AGC2        (0x0124)
#define AFE_UL_AGC3        (0x0128)
#define AFE_UL_AGC4        (0x012C)
#define AFE_UL_AGC5        (0x0130)
#define AFE_UL_AGC6        (0x0134)
#define AFE_UL_AGC7        (0x0138)
#define AFE_UL_AGC8        (0x013C)
#define AFE_UL_AGC9        (0x0140)
#define AFE_UL_AGC10       (0x0144)
#define AFE_UL_AGC11       (0x0148)
#define AFE_UL_AGC12       (0x014C)
#define AFE_UL_AGC13       (0x0150)
#define AFE_UL_AGC14       (0x0154)
#define AFE_UL_AGC15       (0x0158)
#define AFE_UL_AGC16       (0x015C)
#define AFE_UL_AGC17       (0x0160)
#define AFE_UL_AGC18       (0x0164)
#define AFE_VAGC_CON0      (0x020C)
#define AFE_SDM_GAIN_STAGE (0x0168) //[31]: 0, [5:0]: gain :default:0x10 -6dB

// Register : SIDETONE
#define AFE_SIDETONE_CON0  (0x01E0)
#define AFE_SIDETONE_CON1  (0x01E4)

// Register : TOP CONTROL
#define AFE_TOP_CONTROL_0  (0x0200)

// AFE_CONN
#define Connect(x, y, value, mask)        x##_##y##_S(value,mask,|)
#define Disconnect(x, y, value, mask)     x##_##y##_S(value,mask,& ~)
#define Connect_S2M(x, y, value, mask)     x##_##y##_R(value,mask,|)
#define Disconnect_S2M(x, y, value, mask)  x##_##y##_R(value,mask,& ~)


//**************
// AFE_IRQ_CON  AFE+0x03A0
//**************
#define SET_IRQ1_MCU_ON_on(value)   Enable(value,BIT_00);
#define SET_IRQ1_MCU_ON_off(value)  Disable(value,BIT_00);
#define SET_IRQ1_MCU_ON_mask(mask)  Enable(mask,BIT_00);

#define SET_IRQ2_MCU_ON_on(value)   Enable(value,BIT_01);
#define SET_IRQ2_MCU_ON_off(value)  Disable(value,BIT_01);
#define SET_IRQ2_MCU_ON_mask(mask)  Enable(mask,BIT_01);

#define SET_IRQ3_MCU_ON_on(value)    //BIT_02
#define SET_IRQ3_MCU_ON_off(value)

#define SET_IRQ1_MCU_MODE(value,index)  value = (value & ( ~(0xf<<4) )) | (index << 4); // bit4 ~ bit7, IRQ1_MCU_MODE
#define SET_IRQ2_MCU_MODE(value,index)  value = (value & ( ~(0xf<<8) )) | (index << 8);  // bit8 ~ bit11, IRQ2_MCU_MODE

#define SET_IRQ1_MCU_MODE_mask(mask)  Enable(mask,BIT_04);Enable(mask,BIT_05);Enable(mask,BIT_06);Enable(mask,BIT_07);
#define SET_IRQ2_MCU_MODE_mask(mask)  Enable(mask,BIT_08);Enable(mask,BIT_09);Enable(mask,BIT_10);Enable(mask,BIT_11);

//**************
// AFE_IR_CLR  AFE+0x03A8
//**************
#define SET_IRQ1_MCU_CLR(value)      Enable(value,BIT_00);
#define SET_IRQ1_MCU_CLR_mask(mask)  Enable(mask,BIT_00);

#define SET_IRQ2_MCU_CLR(value)      Enable(value,BIT_01);
#define SET_IRQ2_MCU_CLR_mask(mask)  Enable(mask,BIT_01);

//**************
// AFE_DL_SRC2_1  AFE+0x0108
//**************
#define SET_DL2_SRC_ON_on(value)       Enable(value,BIT_00);  // BIT_00
#define SET_DL2_SRC_ON_off(value)      Disable(value,BIT_00);
#define SET_DL2_SRC_ON_mask(mask)      Enable(mask,BIT_00);

#define SET_DL2_GAIN_ON_on(value)       Enable(value,BIT_01);  // BIT_01
#define SET_DL2_GAIN_ON_off(value)      Disable(value,BIT_01);
#define SET_DL2_GAIN_ON_mask(mask)      Enable(mask,BIT_01);

#define SET_DL2_VOICE_MODE_voice(value)     Enable(value,BIT_05);
#define SET_DL2_VOICE_MODE_audio(value)     Disable(value,BIT_05);
#define SET_DL2_VOICE_MODE_mask(mask)       Enable(mask,BIT_05);

#define SET_DL2_INPUT_MODE_SEL(value,index) value=(value & ( ~(0xf<<28) )) | (index << 28);
#define SET_DL2_INPUT_MODE_mask(mask)       Enable(mask,BIT_28);Enable(mask,BIT_29);Enable(mask,BIT_30);Enable(mask,BIT_31);

#define SET_DL2_MUTE_CH1_ON_on(value)       Enable(value,BIT_04);
#define SET_DL2_MUTE_CH2_ON_on(value)       Enable(value,BIT_03);
#define SET_DL2_MUTE_CH1_ON_off(value)      Disable(value,BIT_04);
#define SET_DL2_MUTE_CH2_ON_off(value)      Disable(value,BIT_03);

#define SET_DL2_MUTE_CH1_ON_mask(mask)      Enable(mask,BIT_03);
#define SET_DL2_MUTE_CH2_ON_mask(mask)      Enable(mask,BIT_04);

//**************
// AFE_DAC_CON1  AFE+0x0014
//**************
#define SET_DL1_MODE(value,index)  value = (value & ( ~(0xf<<0) )) | (index << 0);
#define SET_DL1_MODE_mask(mask)    Enable(mask,BIT_00);Enable(mask,BIT_01);Enable(mask,BIT_02);Enable(mask,BIT_03);

#define SET_I2S_MODE(value,index)  value = (value & ( ~(0xf<<8) )) | (index << 8);
#define SET_I2S_MODE_mask(mask)    Enable(mask,BIT_08);Enable(mask,BIT_09);Enable(mask,BIT_10);Enable(mask,BIT_11);


#define SET_DL1_DATA_mono(value)      Enable(value,BIT_20);
#define SET_DL1_DATA_stereo(value)    Disable(value,BIT_20);
#define SET_DL1_DATA_mask(mask)       Enable(mask,BIT_20);

#define SET_I2S_DATA_mono(value)      Enable(value,BIT_22);
#define SET_I2S_DATA_stereo(value)    Disable(value,BIT_22);
#define SET_I2S_DATA_mask(mask)       Enable(mask,BIT_22);

#define SET_DL1_OPMODE_Master(value)  Enable(value,BIT_19);
#define SET_DL1_OPMODE_Slave(value)   Disable(value,BIT_19);
#define SET_DL1_OPMODE_mask(mask)     Enable(mask,BIT_19);

//**************
// AFE_DAC_CON0  AFE+0x0010
//**************
#define SET_AFE_ON_on(value)   Enable(value,BIT_00);  // BIT_00
#define SET_AFE_ON_off(value)  Disable(value,BIT_00);
#define SET_AFE_ON_mask(mask)  Enable(mask,BIT_00);

#define SET_DL1_ON_on(value)   Enable(value,BIT_01);  // BIT_01
#define SET_DL1_ON_off(value)  Disable(value,BIT_01);
#define SET_DL1_ON_mask(mask)  Enable(mask,BIT_01);

#define SET_VUL_ON_on(value)   Enable(value,BIT_03);  // BIT_03
#define SET_VUL_ON_off(value)  Disable(value,BIT_03);
#define SET_VUL_ON_mask(mask)  Enable(mask,BIT_03);

#define SET_DAI_ON_on(value)   Enable(value,BIT_04);  // BIT_04
#define SET_DAI_ON_off(value)  Disable(value,BIT_04);
#define SET_DAI_ON_mask(mask)  Enable(mask,BIT_04a);

#define SET_I2S_ON_on(value)   Enable(value,BIT_05);  // BIT_05
#define SET_I2S_ON_off(value)  Disable(value,BIT_05);
#define SET_I2S_ON_mask(mask)  Enable(mask,BIT_05);

#define SET_AWB_ON_on(value)   Enable(value,BIT_06);  // BIT_06
#define SET_AWB_ON_off(value)  Disable(value,BIT_06);
#define SET_AWB_ON_mask(mask)  Enable(mask,BIT_06);

//**************
// AFE_DL_SRC2_2  AFE+0x010C
//**************
#define DL2_GAIN(value,gain)  value=(value & ( ~(0xffff<<16) )) | (gain << 16);

/**********************************
 *  Other Definitions             *
 **********************************/
#define BIT_00	0x00000001        /* ---- ---- ---- ---- ---- ---- ---- ---1 */
#define BIT_01	0x00000002        /* ---- ---- ---- ---- ---- ---- ---- --1- */
#define BIT_02	0x00000004        /* ---- ---- ---- ---- ---- ---- ---- -1-- */
#define BIT_03	0x00000008        /* ---- ---- ---- ---- ---- ---- ---- 1--- */
#define BIT_04	0x00000010        /* ---- ---- ---- ---- ---- ---- ---1 ---- */
#define BIT_05	0x00000020        /* ---- ---- ---- ---- ---- ---- --1- ---- */
#define BIT_06	0x00000040        /* ---- ---- ---- ---- ---- ---- -1-- ---- */
#define BIT_07	0x00000080        /* ---- ---- ---- ---- ---- ---- 1--- ---- */
#define BIT_08	0x00000100        /* ---- ---- ---- ---- ---- ---1 ---- ---- */
#define BIT_09	0x00000200        /* ---- ---- ---- ---- ---- --1- ---- ---- */
#define BIT_10	0x00000400        /* ---- ---- ---- ---- ---- -1-- ---- ---- */
#define BIT_11	0x00000800        /* ---- ---- ---- ---- ---- 1--- ---- ---- */
#define BIT_12	0x00001000        /* ---- ---- ---- ---- ---1 ---- ---- ---- */
#define BIT_13	0x00002000        /* ---- ---- ---- ---- --1- ---- ---- ---- */
#define BIT_14	0x00004000        /* ---- ---- ---- ---- -1-- ---- ---- ---- */
#define BIT_15	0x00008000        /* ---- ---- ---- ---- 1--- ---- ---- ---- */
#define BIT_16	0x00010000        /* ---- ---- ---- ---1 ---- ---- ---- ---- */
#define BIT_17	0x00020000        /* ---- ---- ---- --1- ---- ---- ---- ---- */
#define BIT_18	0x00040000        /* ---- ---- ---- -1-- ---- ---- ---- ---- */
#define BIT_19	0x00080000        /* ---- ---- ---- 1--- ---- ---- ---- ---- */
#define BIT_20	0x00100000        /* ---- ---- ---1 ---- ---- ---- ---- ---- */
#define BIT_21	0x00200000        /* ---- ---- --1- ---- ---- ---- ---- ---- */
#define BIT_22	0x00400000        /* ---- ---- -1-- ---- ---- ---- ---- ---- */
#define BIT_23	0x00800000        /* ---- ---- 1--- ---- ---- ---- ---- ---- */
#define BIT_24	0x01000000        /* ---- ---1 ---- ---- ---- ---- ---- ---- */
#define BIT_25	0x02000000        /* ---- --1- ---- ---- ---- ---- ---- ---- */
#define BIT_26	0x04000000        /* ---- -1-- ---- ---- ---- ---- ---- ---- */
#define BIT_27	0x08000000        /* ---- 1--- ---- ---- ---- ---- ---- ---- */
#define BIT_28	0x10000000        /* ---1 ---- ---- ---- ---- ---- ---- ---- */
#define BIT_29	0x20000000        /* --1- ---- ---- ---- ---- ---- ---- ---- */
#define BIT_30	0x40000000        /* -1-- ---- ---- ---- ---- ---- ---- ---- */
#define BIT_31	0x80000000        /* 1--- ---- ---- ---- ---- ---- ---- ---- */

/*****************************************************************************
*                        F U N C T I O N   D E F I N I T I O N
******************************************************************************
*/
namespace android {

typedef struct
{
    uint32 Channel;
    uint32 SampleRate;
    uint32 Format;
}Stream_Attribute;

class AudioYusuHardware;
class AudioAnalog;
class AudioI2S;

class AudioAfe
{
public:
   AudioAfe(AudioYusuHardware *hw);
   ~AudioAfe();

   void SetAfeReg(uint32 offset,uint32 value, uint32 mask);
   void GetAfeReg(uint32 offset,uint32 *value);

   void Afe_Set_Stream_Gain(uint32 gain);
   bool Afe_Set_Stream_Attribute(uint32 format,uint32 channelCount,uint32 sampleRate);
   bool Afe_Set_Stream_Attribute(uint32 format,uint32 channelCount,uint32 sampleRate,AFE_MODE OpenMode);
   bool Afe_Set_Stream_Format(uint32 *src, uint32 Format) ;
   bool Afe_Set_Stream_Channel(uint32 *src, uint32 Channel);
   bool Afe_Set_Stream_SampleRate(uint32 *src, uint32 sampleRate);
   uint32 Afe_Get_DL_SR_Index(uint32 SamplingRate);
   uint32 Afe_Get_UL_SR_Index(uint32 SamplingRate);
   void Afe_Set_Timer(uint32 IRQ_cnt,uint32 InterruptCounter);
   void Afe_Set_FmTx(int32 Enable, int32 DAC_Reopen);
   void Afe_Set_FmTx_DigitalOut(int32 Enable, int32 DAC_Reopen);
   void Afe_Set_FmTx_AnalogOut(int32 Enable, int32 DAC_Reopen);

   void Afe_DL_Start(AFE_MODE OpenMode);
   void Afe_UL_Start(AFE_MODE OpenMode);
   void Afe_DL_Stop(AFE_MODE OpenMode);
   void Afe_UL_Stop(AFE_MODE OpenMode);
   void Afe_DL_Mute(AFE_MODE OpenMode);
   void Afe_UL_Mute(AFE_MODE OpenMode);
   void Afe_DL_Unmute(AFE_MODE OpenMode);
   void Afe_UL_Unmute(AFE_MODE OpenMode);

   void Afe_Set_Stereo(void);
   void Afe_Set_Mono(void);

   bool Afe_Init(uint32 Fd);
   bool Afe_Deinit(void);
   bool SetI2SControl(bool bEnable,int type,uint32 SampleRate=0);

   // Set Interconnection for AWB
   void SetAWBConenction(bool Connect);
   void SetAWBChannel(int channels);

   // this will set AFE_DAC_CON0 with bits,
   void SetAWBMemoryInterfaceEnable(bool Enable,uint32 bitmask);

   // use for AP record
   void HQA_AFE_Set_UL_ADC_PGA_Gain(int gain_value);
   void HQA_AFE_Set_UL_ADC_Minima_PGA_Gain(int gain_value);
   void HQA_AFE_Set_I2SRecPly_PATH(int bEnable);

  //used for AP I2S out
   bool HQA_Afe_DigitalOut(bool Enable, int SampleRate);

   AudioAnalog *mAnalog;
   bool mAudioStereoToMono;
   pthread_t mFMTXThread;
   bool mAudioFmTx_thread;
   unsigned int IdxAudioPattern;
   unsigned int SizeAudioPattern;
   unsigned char* g_i2VDL_DATA;
   char* mAudioBuffer;
   bool mFlag_Aud_DL1_SlaveOn;
   static bool FirstInit;
   bool mDaiBtMode;

private:
    mutable Mutex	mLock;
    int    mFd;
    unsigned int mSamplingRate;
    unsigned int mCh;
    unsigned int mFormat;
    Stream_Attribute AWB_Stream;
    Stream_Attribute ADC_Stream;
    Stream_Attribute I2S0output_Stream;
    Stream_Attribute I2S1output_Stream;
    Stream_Attribute I2Sinput_Stream;
    AudioYusuHardware *mAudioHardware;

    AudioI2S *mI2SInstance;
    uint32 I2SClient;
//#ifdef AUDIO_HQA_SUPPORT
   int m_I2SRec_Enable;
//#endif
};


}; // namespace android

#endif /*ANDROID_AUDIO_RESAMPLER_SINC_H*/

