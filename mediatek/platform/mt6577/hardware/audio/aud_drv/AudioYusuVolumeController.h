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

#ifndef ANDROID_AUDIO_YUSU_VOLUME_CONTROLLER_H
#define ANDROID_AUDIO_YUSU_VOLUME_CONTROLLER_H

#include <sys/ioctl.h>
#include <utils/Log.h>
#include <utils/String8.h>
#include <media/AudioSystem.h>


#include "AudioAnalogAfe.h"
#include "AudioAfe.h"
#include "AudioIoctl.h"

#include "CFG_AUDIO_File.h"
#include <hardware_legacy/AudioSystemLegacy.h>

#ifdef MATV_AUDIO_LINEIN_PATH
#include <IATVCtrlService.h>
#include <binder/IServiceManager.h>
#endif


#define  NUM_AUDIO_MODES_TYPES  (4)
/*-----------------------------------------------
MODE_NORMAL = 0,
MODE_RINGTONE,
MODE_IN_CALL,
MODE_COMMUNCATION
NOTE:: Mode FM is a special mode for output
----------------------------------------------------*/

#define  NUM_AUDIO_ROUTE  (13)
#define  NUM_AUDIO_ROUTE_TYPES  (0x1000)

/*-----------------------------------------------
DEVICE_OUT_EARPIECE = 0x1,
DEVICE_OUT_SPEAKER = 0x2,
DEVICE_OUT_WIRED_HEADSET = 0x4,
DEVICE_OUT_WIRED_HEADPHONE = 0x8,
DEVICE_OUT_BLUETOOTH_SCO = 0x10,
DEVICE_OUT_BLUETOOTH_SCO_HEADSET = 0x20,
DEVICE_OUT_BLUETOOTH_SCO_CARKIT = 0x40,
DEVICE_OUT_BLUETOOTH_A2DP = 0x80,
DEVICE_OUT_BLUETOOTH_A2DP_HEADPHONES = 0x100,
DEVICE_OUT_BLUETOOTH_A2DP_SPEAKER = 0x200,
DEVICE_OUT_AUX_DIGITAL = 0x400,
----------------------------------------------------*/
#define VOLUME_STEP 		      (0xFFFF)
#define VOLUME_STEP_NUMBER    (200)

// AUDIO_VOLUME_MAX , for audio setting , value bigger than AUDIO_VOLUME_MAX will bound with AUDIO_VOLUME_MAX
// degrade one DB step range
#define AUDIO_VOLUME_MAX       (176)
#define AUDIO_ONEDB_STEP         (4)
#define VOICE_VOLUME_MAX       (176)
#define VOICE_ONEDB_STEP         (4)
#define UPLINK_GAIN_MAX            (252)
#define UPLINK_ONEDB_STEP         (4)
#define SIDETONE_GAIN_MAX            (240)
#define SIDETONE_ONEDB_STEP         (4)
#define SIDETONE_MAX_GAIN_DEGRADE   (193/2)
#define LEVEL_SHIFT_BUFFER_GAIN_MAX   (240)
#define LEVEL_SHIFT_ONEDB_STEP              (4)
#define LEVEL_SHIFT_BUFFER_MAXDB        (12)
#define LEVEL_SHIFT_BUFFER_DEGRADE_MAXDB        (21)
#define LINEIN_GAIN_MAX                       (240)
#define LINEIN_GAIN_DB_STEP                (12)
#define LINEIN_MAX_GAIN_DEGRADE         (7)




#define MIC_GAIN_MAX                 (63)
#define MIC_GAIN_MIN                 (0)

namespace android {
// ----------------------------------------------------------------------------
class AudioYusuHardware;
class LAD;

static const uint16 GainMap[] =
{
	0xf1ad, // 0: -0.50 dB
	0xe429, // 1: -1.00 dB
	0xd765, // 2: -1.50 dB
	0xcb59, // 3: -2.00 dB
	0xbff9, // 4: -2.50 dB
	0xb53b, // 5: -3.00 dB
	0xab18, // 6: -3.50 dB
	0xa186, // 7: -4.00 dB
	0x987d, // 8: -4.50 dB
	0x8ff5, // 9: -5.00 dB
	0x87e8, // 10: -5.50 dB
	0x804d, // 11: -6.00 dB
	0x7920, // 12: -6.50 dB
	0x7259, // 13: -7.00 dB
	0x6bf4, // 14: -7.50 dB
	0x65ea, // 15: -8.00 dB
	0x6036, // 16: -8.50 dB
	0x5ad5, // 17: -9.00 dB
	0x55c0, // 18: -9.50 dB
	0x50f4, // 19: -10.00 dB
	0x4c6d, // 20: -10.50 dB
	0x4826, // 21: -11.00 dB
	0x441d, // 22: -11.50 dB
	0x404d, // 23: -12.00 dB
	0x3cb5, // 24: -12.50 dB
	0x394f, // 25: -13.00 dB
	0x361a, // 26: -13.50 dB
	0x3314, // 27: -14.00 dB
	0x3038, // 28: -14.50 dB
	0x2d86, // 29: -15.00 dB
	0x2afa, // 30: -15.50 dB
	0x2892, // 31: -16.00 dB
	0x264d, // 32: -16.50 dB
	0x2429, // 33: -17.00 dB
	0x2223, // 34: -17.50 dB
	0x203a, // 35: -18.00 dB
	0x1e6c, // 36: -18.50 dB
	0x1cb9, // 37: -19.00 dB
	0x1b1d, // 38: -19.50 dB
	0x1999, // 39: -20.00 dB
	0x182a, // 40: -20.50 dB
	0x16d0, // 41: -21.00 dB
	0x158a, // 42: -21.50 dB
	0x1455, // 43: -22.00 dB
	0x1332, // 44: -22.50 dB
	0x121f, // 45: -23.00 dB
	0x111c, // 46: -23.50 dB
	0x1027, // 47: -24.00 dB
	0x0f3f, // 48: -24.50 dB
	0x0e65, // 49: -25.00 dB
	0x0d97, // 50: -25.50 dB
	0x0cd4, // 51: -26.00 dB
	0x0c1c, // 52: -26.50 dB
	0x0b6f, // 53: -27.00 dB
	0x0acb, // 54: -27.50 dB
	0x0a31, // 55: -28.00 dB
	0x099f, // 56: -28.50 dB
	0x0915, // 57: -29.00 dB
	0x0893, // 58: -29.50 dB
	0x0818, // 59: -30.00 dB
	0x07a4, // 60: -30.50 dB
	0x0737, // 61: -31.00 dB
	0x06cf, // 62: -31.50 dB
	0x066e, // 63: -32.00 dB
	0x0612, // 64: -32.50 dB
	0x05bb, // 65: -33.00 dB
	0x0569, // 66: -33.50 dB
	0x051b, // 67: -34.00 dB
	0x04d2, // 68: -34.50 dB
	0x048d, // 69: -35.00 dB
	0x044c, // 70: -35.50 dB
	0x040e, // 71: -36.00 dB
	0x03d4, // 72: -36.50 dB
	0x039d, // 73: -37.00 dB
	0x0369, // 74: -37.50 dB
	0x0339, // 75: -38.00 dB
	0x030a, // 76: -38.50 dB
	0x02df, // 77: -39.00 dB
	0x02b6, // 78: -39.50 dB
	0x028f, // 79: -40.00 dB
	0x026a, // 80: -40.50 dB
	0x0248, // 81: -41.00 dB
	0x0227, // 82: -41.50 dB
	0x0208, // 83: -42.00 dB
	0x01eb, // 84: -42.50 dB
	0x01cf, // 85: -43.00 dB
	0x01b6, // 86: -43.50 dB
	0x019d, // 87: -44.00 dB
	0x0186, // 88: -44.50 dB
	0x0170, // 89: -45.00 dB
	0x015b, // 90: -45.50 dB
	0x0148, // 91: -46.00 dB
	0x0136, // 92: -46.50 dB
	0x0124, // 93: -47.00 dB
	0x0114, // 94: -47.50 dB
	0x0104, // 95: -48.00 dB
	0x00f6, // 96: -48.50 dB
	0x00e8, // 97: -49.00 dB
	0x00db, // 98: -49.50 dB
	0x00cf, // 99: -50.00 dB
	0x00c3, // 100: -50.50 dB
	0x00b8, // 101: -51.00 dB
	0x00ae, // 102: -51.50 dB
	0x00a4, // 103: -52.00 dB
	0x009b, // 104: -52.50 dB
	0x0092, // 105: -53.00 dB
	0x008a, // 106: -53.50 dB
	0x0082, // 107: -54.00 dB
	0x007b, // 108: -54.50 dB
	0x0074, // 109: -55.00 dB
	0x006e, // 110: -55.50 dB
	0x0067, // 111: -56.00 dB
	0x0062, // 112: -56.50 dB
	0x005c, // 113: -57.00 dB
	0x0057, // 114: -57.50 dB
	0x0052, // 115: -58.00 dB
	0x004d, // 116: -58.50 dB
	0x0049, // 117: -59.00 dB
	0x0045, // 118: -59.50 dB
	0x0041, // 119: -60.00 dB
	0x003d, // 120: -60.50 dB
	0x003a, // 121: -61.00 dB
	0x0037, // 122: -61.50 dB
	0x0034, // 123: -62.00 dB
	0x0031, // 124: -62.50 dB
	0x002e, // 125: -63.00 dB
	0x002b, // 126: -63.50 dB
	0x0029, // 127: -64.00 dB
	0x0027, // 128: -64.50 dB
	0x0024, // 129: -65.00 dB
	0x0022, // 130: -65.50 dB
	0x0020, // 131: -66.00 dB
	0x001f, // 132: -66.50 dB
	0x001d, // 133: -67.00 dB
	0x001b, // 134: -67.50 dB
	0x001a, // 135: -68.00 dB
	0x0018, // 136: -68.50 dB
	0x0017, // 137: -69.00 dB
	0x0015, // 138: -69.50 dB
	0x0014, // 139: -70.00 dB
	0x0013, // 140: -70.50 dB
	0x0012, // 141: -71.00 dB
	0x0011, // 142: -71.50 dB
	0x0010, // 143: -72.00 dB
	0x000f, // 144: -72.50 dB
	0x000e, // 145: -73.00 dB
	0x000d, // 146: -73.50 dB
	0x000d, // 147: -74.00 dB
	0x000c, // 148: -74.50 dB
	0x000b, // 149: -75.00 dB
	0x000b, // 150: -75.50 dB
	0x000a, // 151: -76.00 dB
	0x0009, // 152: -76.50 dB
	0x0009, // 153: -77.00 dB
	0x0008, // 154: -77.50 dB
	0x0008, // 155: -78.00 dB
	0x0007, // 156: -78.50 dB
	0x0007, // 157: -79.00 dB
	0x0006, // 158: -79.50 dB
	0x0006, // 159: -80.00 dB
	0x0006, // 160: -80.50 dB
	0x0005, // 161: -81.00 dB
	0x0005, // 162: -81.50 dB
	0x0005, // 163: -82.00 dB
	0x0004, // 164: -82.50 dB
	0x0004, // 165: -83.00 dB
	0x0004, // 166: -83.50 dB
	0x0004, // 167: -84.00 dB
	0x0003, // 168: -84.50 dB
	0x0003, // 169: -85.00 dB
	0x0003, // 170: -85.50 dB
	0x0003, // 171: -86.00 dB
	0x0003, // 172: -86.50 dB
	0x0002, // 173: -87.00 dB
	0x0002, // 174: -87.50 dB
	0x0002, // 175: -88.00 dB
	0x0002, // 176: -88.50 dB
	0x0002, // 177: -89.00 dB
	0x0002, // 178: -89.50 dB
	0x0002, // 179: -90.00 dB
	0x0001, // 180: -90.50 dB
	0x0001, // 181: -91.00 dB
	0x0001, // 182: -91.50 dB
	0x0001, // 183: -92.00 dB
	0x0001, // 184: -92.50 dB
	0x0001, // 185: -93.00 dB
	0x0001, // 186: -93.50 dB
	0x0001, // 187: -94.00 dB
	0x0001, // 188: -94.50 dB
	0x0001, // 189: -95.00 dB
	0x0001, // 190: -95.50 dB
	0x0001, // 191: -96.00 dB
	0x0000, // 192: -96.50 dB
	0x0000, // 193: -97.00 dB
	0x0000, // 194: -97.50 dB
	0x0000, // 195: -98.00 dB
	0x0000, // 196: -98.50 dB
	0x0000, // 197: -99.00 dB
	0x0000, // 198: -99.50 dB
	0x0000, // 199: -100.00 dB
};

typedef enum
{
    LINE_GAIN_FM= 0x0,
    LINE_GAIN_MATV
}LINE_IN_GAIN_MODE;


typedef enum
{
    ROUTE_OUT_EARPIECE = 0x0,
    ROUTE_OUT_SPEAKER ,
    ROUTE_OUT_WIRED_HEADSET ,
    ROUTE_OUT_WIRED_HEADPHONE,
    ROUTE_OUT_BLUETOOTH_SCO,
    ROUTE_OUT_BLUETOOTH_SCO_HEADSET,
    ROUTE_OUT_BLUETOOTH_SCO_CARKIT,
    ROUTE_OUT_BLUETOOTH_A2DP,
    ROUTE_OUT_BLUETOOTH_A2DP_HEADPHONES,
    ROUTE_OUT_BLUETOOTH_A2DP_SPEAKER,
    ROUTE_OUT_AUX_DIGITAL
}_YUSU_ROUTE;

typedef enum
{
    Idle_Normal_Record =0,
    Idle_Headset_Record,
    Normal_Mic,
    Headset_Mic,
    Handfree_Mic,
    Fm_Record,
    TTY_CTM_Mic,
    VOIP_Normal_Mic,
    VOIP_Headset_Mic,
    VOIP_Handfree_Mic,
    Level_Shift_buffer,
    Voice_Rec_Mic_Handset,
    Voice_Rec_Mic_Headset,
    Idle_Video_Record_Handset,
    Idle_Video_Record_Headset,
    Num_Mic_Gain
}MIC_GAIN_MODE;


class AudioYusuVolumeController
{
public:

    AudioYusuVolumeController(int Fd, AudioYusuHardware *hw, AudioAfe *AsmReg, AudioAnalog *AnaReg);
    ~AudioYusuVolumeController();
    bool InitVolumeController();
    bool setMasterVolume(float v, uint32 mode,uint32 routes);
    bool ModeSetVoiceVolume(void);
    bool setVoiceVolume(float v, uint32 mode,uint32 routes);
    bool setVoIPVolume(float volume, uint32 mode,uint32 routes);
    void ApplyGain(uint16 Gain,uint32 mode,uint32 routes);
    void ApplyGainInCall(uint16 Gain,uint32 mode,uint32 routes);
    uint32 GetMultiplier(uint32 dBAttenTotal);

    // apply gain to mode normal and speaker , use for meta
    void ApplyGain(uint16 Gain,uint32 routes);
    void ApplyHandsetGain(uint16 Gain,uint32 mode,uint32 routes);
    bool SetVolumeRange(uint32 mode,uint32 route,int32 MaxVolume, int32 MinVolume);
    float getMasterVolume(void);
    float getVoiceVolume(void);
    uint32 SetVolumeRange(uint16 Gain,uint32 mode ,uint32 routes);
    uint32 Calgain(uint16 DegradedBGain,uint32 mode ,uint32 routes);
    uint32 CalVoicegain(uint16 DegradedBGain,uint32 mode ,uint32 routes);
    bool SetMicGain(uint32  MicMode, uint32 Gain);
    bool SetLevelShiftBufferGain(uint32 MicMode, uint32 Gain);

    bool SetApMicGain(uint32 MicMode);
#ifdef MTK_AP_SPEECH_ENHANCEMENT 	
	int SetApMicGain_HDRecord(int MicMode, bool isDigitalMic = 0);
#endif
    uint32 GetMicGain(uint32 MicMode);
    bool SetLevelShiftBufferGain(void);
    bool HandleFmFd(bool Enable);
    void SetLineInGain(uint32 gain);
    void SetLineInGainByMode(uint32 mode);

    bool SetLadMicGain(int MicType);
    bool SetSideTone(uint32  Mode, uint32 Gain);

    bool SetFmVolume(int volume);
    int GetFmVolume();
    bool setMatvVolume(int volume);
    int GetMatvVolume(void);
    bool SetFmMute(bool Enable);
    int32 AudioGetRoute(uint32 routes);

    bool SetMatvMute(bool b_mute);
    int GetExtDacVolume(int volume);

    AUDIO_CUSTOM_PARAM_STRUCT sndParam;//volume data
    AUDIO_VOLUME_CUSTOM_STRUCT VolumeCustomParam;//volume custom data

    #ifdef MATV_AUDIO_LINEIN_PATH
    void GetMatvService(void);
    #endif

private:

    #ifdef MATV_AUDIO_LINEIN_PATH
    // Keeping pointer to ATVCtrlService
    sp<IATVCtrlService> spATVCtrlService;
    #endif
	
#ifdef MTK_AUDIO_GAIN_TABLE
	AUDIO_GAIN_TABLE_STRUCT mGainTable;
#endif

    float mVolume;
    float mVoiceVolume;
    int mFd;
    int mfmFd;
    int mFmVolume;
    int mMatvVolume;
    int mFmMute;
    AudioAfe *mAfe_handle;
    AudioAnalog *mAnaReg;
    AudioYusuHardware *mAudioHardware;
    int32 mVolumeMax[android_audio_legacy::AudioSystem::NUM_MODES][NUM_AUDIO_ROUTE];
    int32 mVolumeMin[android_audio_legacy::AudioSystem::NUM_MODES][NUM_AUDIO_ROUTE];
    int32 mVolumeRange[android_audio_legacy::AudioSystem::NUM_MODES][NUM_AUDIO_ROUTE];
    uint32 mMicGain[Num_Mic_Gain];
    uint32 mSideTone[NUM_AUDIO_ROUTE];
    bool mMatvMute;
};

// ----------------------------------------------------------------------------
}; // namespace android

#endif
