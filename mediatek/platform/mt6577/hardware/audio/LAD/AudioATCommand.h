/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */
#ifndef _AUDIO_ATCOMMAND_H_
#define _AUDIO_ATCOMMAND_H_

#include <stdio.h>
#include <utils/threads.h>
#include "AudioYusuDef.h"

namespace android
{
    using android::Mutex;
    using android::AutoMutex;

class AudioYusuHardware;

enum ENUM_SPC_GET_PARAMS
{
    SPC_GET_PARAMS_SR        = 0,
    SPC_GET_PARAMS_CH        = 1,
    SPC_GET_PARAMS_REC_STATE = 2,
    SPC_GET_PARAMS_NUM
};

class AudioATCommand
{
private:

    int   openDevice(char *deviceName);
    void  closeDevice(int fd);
    int   sendAtCommand(int fd, char* command);

    int   openRecDevice(char *deviceName);
    void  closeRecDevice();

    // file handle for UART user space interface
    int32 fHdl;                   //for file read
    int32 fRecHdl;                //for record file read
    bool  bATCmdDeviceOpen;
    bool  bRecDeviceOpen;
    bool  bRecordState;
    int32 mMicGain;
    FILE *pRecFile;
    mutable Mutex mLock;
public:
    AudioATCommand(AudioYusuHardware *hw);
    ~AudioATCommand();

    bool Spc_Speech_On( uint8 RAT_Mode );
    bool Spc_Speech_Off();
    bool Spc_SetMicrophoneVolume( uint8 mic_volume );
    bool Spc_SetSidetoneVolume( uint8 sidetone_volume );
    bool Spc_Default_Tone_Play(uint8 toneIdx);
    bool Spc_Default_Tone_Stop();
    bool Spc_SetOutputVolume( uint8 volume, int8 digital_gain_index )  ;
    bool Spc_SetOutputVolume( uint32 Gain )  ;
    bool Spc_SetInputSource( uint8 src );
    bool Spc_SetOutputDevice( uint8 device );
    bool Spc_SetSpeechMode_Adaptation( uint8 mode );
    bool Spc_MuteMicrophone( uint8 enable);
    int  Spc_OpenNormalRecPath(uint8 format, uint8 sampleRate);
    int  Spc_CloseNormalRecPath();
    /* Return actually read size */
    int  Spc_ReadRNormalRecData(uint16 *pBuf, uint16 u2Size);
    int  Spc_GetParameters(ENUM_SPC_GET_PARAMS enum_idx);

    bool bmodem_reset;
    AudioYusuHardware *mHw;    /* pointer to HW */
};

}; // namespace android

#endif   //_AUDIO_YUSU_UART_H_
