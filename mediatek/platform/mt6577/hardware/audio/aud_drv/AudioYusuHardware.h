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
**
** Copyright 2007, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#ifndef ANDROID_AUDIO_YUSU_HARDWARE_H
#define ANDROID_AUDIO_YUSU_HARDWARE_H

#define MAX_OUTPUT_STREAM_NUM (1)
#define MAX_INPUT_STREAM_NUM (1)
#define MAX_ASM_GAIN (0xFFFF)

#if defined(MT6573)
#define  AUD_DL1_USE_SLAVE
#endif

#ifdef  AUD_DL1_USE_SLAVE
#define ASM_BUFFER_SIZE (0x2700) // 9984 bytes fixed by AudioSys
#else
#define ASM_BUFFER_SIZE (0x3000) // 12k bytes
#endif
#define AUD_TTY_OFF   0
#define AUD_TTY_FULL  1
#define AUD_TTY_VCO   2
#define AUD_TTY_HCO   4
#define AUD_TTY_ERR   -1

#define MODEM_REC_VM_MASK        0x0001
#define MODEM_REC_CTM4WAY_MASK   0x0002

#if defined(MTK_DUAL_MIC_SUPPORT)||defined(MTK_AUDIO_HD_REC_SUPPORT)
#define MAX_MIC_GAIN_DB   49
#define MIN_MIC_GAIN_DB   -14
#define MAX_RECEIVER_GAIN_DB   12
#define MIN_RECEIVER_GAIN_DB   -32
#define MAX_HEADSET_GAIN_DB   12
#define MIN_HEADSET_GAIN_DB   -32
#define DMSD_RECBUF_SIZE 64001
#define DMSD_PLAYBUF_SIZE 64001

#define DMMETA_RECEIVER_OUT 0
#define DMMETA_HEADSET_OUT 1
#define DMMETA_2WAY_NORMAL 0
#define DMMETA_2WAY_RECONLY 1
#endif


#include <stdint.h>
#include <sys/types.h>
#include <utils/threads.h>
#include <hardware_legacy/AudioHardwareBase.h>
#include <hardware_legacy/AudioSystemLegacy.h>

#include "AudioYusuStreamOut.h"
#include "AudioYusuStreamIn.h"
#include "AudioYusuI2SStreamIn.h"
#include "AudioYusuApStreamIn.h"
#include "AudioYusuStreamHandler.h"
#include "AudioYusuVolumeController.h"
#include "AudioYusuLad.h"
#include <media/AudioRecord.h>
#include <media/AudioSystem.h>

//#include "audio_customization.h"
//#include "audio_custom_exp.h"

#include "AudioAMPControlInterface.h"
#include "HeadSetDetect.h"

// for Audio Param Tuning Tool
#include "AudioParamTuning.h"

#ifdef MTK_AP_SPEECH_ENHANCEMENT  
#include "AudioSpeechEnhLayer.h"
#endif
#define ENABLE_VOIP_BTSCO

namespace android
{

// ----------------------------------------------------------------------------
char const * const kAudioDeviceName = "/dev/eac";

class LAD;
class LADPlayer;
class AudioAfe ;
class AudioAnalog ;
class AudioI2Sinput ;
class AudioCustomization;
class AudioFtm;
class AudioUserCaseManagerInterFace;

class Play2Way;
class Record2Way;
class AudioParamTuning;

enum InputSourceType
{
    YUSU_DEFAULT =0,
    YUSU_MIC	  =1,
    YUSU_VOICE_CALL = 4,
    YUSU_VOICE_DOWNLINK =3,
    YUSU_VOICE_UPLINK = 2,
    YUSU_FM =5
};

enum AudioCommand
{
    AUDIOCOMMAND_NONE =0,
    SETOUTPUTFIRINDEX = 1,
    GETOUTPUTFIRINDEX = 2,
    SETMEDDATA = 3,
    GETMEDDATA = 4,
    GETAUDIOCUSTOMDATASIZE = 5,
    SETAUDIOCUSTOMDATA = 6,
    GETAUDIOCUSTOMDATA = 7,
    START_FMTX_SINEWAVE = 8,
    STOP_FMTX_SINEWAVE  = 9,
    // use for MED output FIR
    GETNORMALOUTPUTFIRINDEX = 0x10,
    GETHEADSETOUTPUTFIRINDEX = 0x11,
    GETSPEAKEROUTPUTFIRINDEX = 0x12,
    SETNORMALOUTPUTFIRINDEX = 0x20,
    SETHEADSETOUTPUTFIRINDEX = 0x21,
    SETSPEAKEROUTPUTFIRINDEX = 0x22,
    GET_DUAL_MIC_PARAMETER   = 0x30,
    SET_DUAL_MIC_PARAMETER   = 0x31,
    GET_WB_SPEECH_PARAMETER   = 0x40,
    SET_WB_SPEECH_PARAMETER   = 0x41,
    SET_LOAD_VOLUME_SETTING    = 0x50,
    // used for Speech Logger
    SET_SPEECH_VM_ENABLE       = 0x60,
    SET_DUMP_SPEECH_DEBUG_INFO = 0x61,
    // used for Audio Logger
    SET_DUMP_AUDIO_DEBUG_INFO  = 0x62,
    SET_DUMP_AUDIO_STREAM_OUT  = 0x63,
    GET_DUMP_AUDIO_STREAM_OUT  = 0x64,
    SET_DUMP_AUDIO_MIXER_BUF   = 0x65,
    GET_DUMP_AUDIO_MIXER_BUF   = 0x66,
    SET_DUMP_AUDIO_TRACK_BUF   = 0x67,
    GET_DUMP_AUDIO_TRACK_BUF   = 0x68,
    SET_DUMP_A2DP_STREAM_OUT   = 0x69,
    GET_DUMP_A2DP_STREAM_OUT   = 0x6A,
    SET_DUMP_AUDIO_STREAM_IN   = 0x6B,
    GET_DUMP_AUDIO_STREAM_IN   = 0x6C,
    SET_DUMP_IDLE_VM_RECORD    = 0x6D,
    GET_DUMP_IDLE_VM_RECORD    = 0x6E,
    // for audio taste tool
    AUD_TASTE_TUNING = 0x70,
	//used for Gain Table in advanced meta 
	GET_GAIN_TABLE_CTRPOINT_NUM     =0x80,
	GET_GAIN_TABLE_CTRPOINT_BITS    =0x81,
	GET_GAIN_TABLE_CTRPOINT_TABLE   =0x82
};

class AudioYusuHardware : public android_audio_legacy::AudioHardwareBase
{
public:
    AudioYusuHardware();
    virtual             ~AudioYusuHardware();
    virtual status_t    initCheck();
    virtual status_t    setVoiceVolume(float volume);
    virtual status_t    setMasterVolume(float volume);

    virtual status_t    getMode(int* mode);
    virtual status_t    setMode(int  mode);
    virtual int getPreviousMode(void);

    // mic mute
    virtual status_t    setMicMute(bool state);
    virtual status_t    getMicMute(bool* state);

    virtual status_t    setParameters(const String8& keyValuePairs);
    virtual String8     getParameters(const String8& keys);

    virtual size_t    getInputBufferSize(uint32_t sampleRate, int format, int channelCount);

    // create I/O streams
    virtual android_audio_legacy::AudioStreamOut* openOutputStream(
        uint32_t devices,
        int *format=0,
        uint32_t *channels=0,
        uint32_t *sampleRate=0,
        status_t *status=0);

    virtual android_audio_legacy::AudioStreamIn* openInputStream(
        uint32_t devices,
        int *format,
        uint32_t *channels,
        uint32_t *sampleRate,
        status_t *status,
        android_audio_legacy::AudioSystem::audio_in_acoustics acoustics);

    virtual  void  closeOutputStream(android_audio_legacy::AudioStreamOut* out);
    virtual  void  closeInputStream(android_audio_legacy::AudioStreamIn* in);

    // when moderouting , should take froceuse ,when device routing , don't use force use ,
    // because force use will take effect when device  change , but will not take effect in mode change.
    int Audio_Match_Output_device(uint32_t mRoutes, uint32 Mode, bool CheckForceUse);
    int Audio_Match_Input_device(uint32_t mRoutes,bool CheckForceUse);

//speaker
    void SetSpeakerStatus(bool on);
    bool GetSpeakerStatus(void);
    void EnableSpeaker(void);
    void ForceDisableSpeaker(void);
    void ForceEnableSpeaker(void);
    bool GetForceSpeakerStatus(void);
    void SwitchAudioClock(bool enable);
    bool SetFmSpeaker(void);
    void ForceSpeaker(bool bEnable);
    static void AudioEarphoneCallback(void * user,bool bEarphoneEnable);

//headphone
    void SetHeadPhoneStatus(bool on);
    bool GetHeadPhoneStatus(void);
    void EnableHeadPhone(void);
    void ForceDisableHeadPhone();
    void ForceEnableHeadPhone(void);
    bool IsHeadsetPlugin(int mode);

//Receiver
    void SetEarpieceStatus(bool on);
    bool GetEarpieceStatus(void);
    void EnableEarpiece();
    void ForceDisableEarpiece();
// amp
	void SetModeToAMP(int mode);
//
    void SetBgsStatus(bool flag);
    bool GetBgsStatus(void);
    void CheckPinmux(int mPreviousMode,int mMode);
    void SetVoiceStatus(bool Enable);
    bool GetVoiceStatus(void);
    void SetCommunicationStatus(bool Enable);
    bool GetCommunicationStatus(void);

    LadInPutDevice_Line ChooseInputSource(int LadSource);
    void SetInputSource(int LadSource);

    bool AddOutputDevices(int device);
    bool RemoveOutputDevices(int device);
    bool AddInputDevices(int device);
    bool RemoveInputDevices(int device);

    bool SetForceUse(uint32_t usage,uint32_t config);
    int Audio_Match_Force_device(uint32_t Force_config);// find output device
    int Audio_Match_ForceInput_device(uint32_t Force_config);// find input deivce
    int Audio_Find_Incall_Output_Device(uint32_t mRoutes);
    int Audio_Find_Communcation_Output_Device(uint32_t mRoutes);
    int Audio_Find_Normal_Output_Device(uint32_t mRoutes);
    int Audio_Find_Ringtone_Output_Device(uint32_t mRoutes);
    int Audio_Find_Input_Device(uint32_t mRoutes);
    int Audio_Find_Input_Communication_Device(uint32_t mRoutes);
    int GetForceUseOutputDevice(uint32_t config);
    int GetForceUseInputDevice(uint32_t config);
    bool SetApMicType(int mictype);
    int GetApMicType();

    virtual status_t SetEMParameter(void *ptr, int len);
    virtual status_t GetEMParameter(void *ptr, int len);
    virtual status_t SetAudioCommand(int par1, int par2);
    virtual status_t GetAudioCommand(int parameters1);
    virtual status_t SetAudioData(int par1,size_t len,void *ptr);
    virtual status_t GetAudioData(int par1,size_t len,void *ptr);
    virtual status_t SetACFPreviewParameter(void *ptr, int len);
    virtual status_t SetHCFPreviewParameter(void *ptr, int len);

    bool SetMicGain(uint32_t mic_type);
    bool setHwAnalogGain(uint32_t gain,  bool audioBuffer=true);
	bool setAmpAnalogGain(uint32_t gain);
	bool setAnalogGain(uint32_t analogGain);
    bool setSpeechGaintableVolume(uint32_t gain);


    // TTY-CTM related function
    int SetTtyCtmMode(int tty_mode);
    void SetTTYSpeechCtrl(char Gain_Mode_Mic);

    // fm related function
    virtual status_t SetFmEnable(bool enable);
    bool GetFmAnalogInEnable(void);
    virtual status_t SetFmDigitalInEnable(bool enable);
    virtual status_t SetFmDigitalOutEnable(bool enable);
    bool GetFmDigitalInEnable(void);
    bool GetFmSpeakerStatus(void);
    bool GetFmTxStatus(void);
    bool Get_FMPower_info(void);
    bool GetFmRxStatus(void);
    bool GetRecordFM(void);
    bool GetHaveExtDac(void);

    //MATV relate function
    void SetMatvAnalogEnable(bool enable);
    void SetMatvDigitalEnable(bool enable);
    bool GetMatvAnalogEnable(void);
    bool GetMatvDigitalEnable(void);
    bool GetMatvStatus(void);
    bool SetMatvSpeaker(void);

    bool GetAnalogLineinEnable(void);

    // routing related function
    status_t doModeRouting(void);
    status_t doOutputDeviceRouting(void);
    status_t doOutputDeviceRouting(bool checkPredevice, bool checknowDevice);
    status_t doInputDeviceRouting(void);

    //recovery mechanism
    void Set_Recovery_State(void);
    void Get_Recovery_State(void);
    int Get_Recovery_Speech(void);
    void Set_Recovery_Speech(int mEnable);
    int Get_Recovery_Bgs(void);
    void Set_Recovery_Bgs(int mEnable);
    int Get_Recovery_Record(void);
    void Set_Recovery_Record(int mEnable);
    void Set_Recovery_Tty(int tty_mode);

    int Get_Recovery_VT(void);
    void Set_Recovery_VT(int mEnable);

    void Recover_State(void);
    bool GetVmFlag(void);
    bool GetCtm4WayFlag(void);

    // add for headser driver
    bool IsHeadsetConnect(void);

    uint32_t GetCurrentDevice(void);
    uint32_t GetPreviousDevice(void);
    // AudioCommand FunctionType_Alloc
    bool UpdateOutputFIR(int mode , int index);
    status_t SetFMTX_SineWave(bool Enable, uint16 Freq);

    // for HDMI
    int enableHDMIAudio(bool mEnable);
	bool isHDMIEnabled();
    bool GetHDMIAudioStatus();
    int SetHDMIAudioConnect(bool connect);
    int MuteHDMI( int timeMs);

    // dump speech Parameters
    void DumpSpeechNBParameters(AUDIO_CUSTOM_PARAM_STRUCT sndParam);
    void DumpSpeechWBParameters(AUDIO_CUSTOM_WB_PARAM_STRUCT sndWbParam);
    void DumpSpeechCoefficient(void);
    void DumpSpeechParatoFile(void);

    // use for temp sulution for VM_record
#ifdef TEMP_SOLUTION_VM
    void Enable_Modem_Record(bool Enable);
#endif

#if defined(MTK_DUAL_MIC_SUPPORT)||defined(MTK_AUDIO_HD_REC_SUPPORT)
    int Get_NVRAM_DLGain_For_DualMic(int index);
    int Get_NVRAM_ULGain_For_DualMic(int index);
    void META_Set_DualMic_Test_Mode(int value);
    bool META_Get_DualMic_Test_Mode(void);
    void META_Set_DualMic_WB(int value);
    bool META_Get_DualMic_WB(void);

    void MMI_Set_Dual_Mic_Enable(int value);

    void Enable_Modem_2Way_Dual_Mic(bool Enable, int OutDevice, int WBMode, int WorkMode);
    void Enable_Modem_Dual_Mic_Rec(bool Enable);
    void SetDualMicToolMicGain(int value);
    void SetDualMicToolReceiverGain(int value);
    void SetDualMicToolHeadsetGain(int value);
    void GetDualMicToolMicGain(int *value);
    void GetDualMicToolReceiverGain(int *value);
    void GetDualMicToolHeadsetGain(int *value);
#endif
    void EnableStereoOutput(int value);

#if defined(MTK_SPH_EHN_CTRL_SUPPORT)
    void MMI_Set_VCE_Enable(bool bEnable);
#endif

    int Set_VTSpeechCall(int vt_mode);
    /////////////////////////////////////////////////////////////////////////
    //    for PCMxWay Interface API
    /////////////////////////////////////////////////////////////////////////
    int xWayPlay_Start(int sample_rate);
    int xWayPlay_Stop(void);
    int xWayPlay_Write(void *buffer, int size_bytes);
    int xWayPlay_GetFreeBufferCount(void);

    int xWayRec_Start(int sample_rate);
    int xWayRec_Stop(void);
    int xWayRec_Read(void *buffer, int size_bytes);

#if defined(MTK_DT_SUPPORT)
    status_t SetFlag_SecondModemPhoneCall(int value);
    int      GetFlag_SecondModemPhoneCall(void);
    bool     SpeechHwCtrlLock();
    void     SpeechHwCtrlUnLock();
    status_t SetWarningTone(int toneIdx);
    status_t StopWarningTone(int toneIdx);
    status_t SetFlag_SkipEndCallDelay(int value);
#endif

    AudioAfe *mAfe_handle;
    AudioAnalog *mAnaReg;
    AudioFtm *mAudFtm;

    AudioI2Sinput *mI2S;
    AudioStreamHandler    *mStreamHandler;
    AudioYusuVolumeController  *mVolumeController ;

    // for audio param tuning  tool
    AudioParamTuning *mAudParamTuning;

// for event handling
#if defined(PC_EMULATION)
    HANDLE LadRecNotifyEvent;
    HANDLE LadPlayRequestEvent;

#else
    // event declare
    pthread_cond_t  LadRecNotifyEvent;
    pthread_cond_t  LadPlayRequestEvent;

    pthread_mutex_t lrnMutex;
    pthread_mutex_t lprMutex;
    pthread_mutex_t LadMutex; // use for outstream write and ladbuffer
    pthread_mutex_t LadBufferMutex;  // use for cci thread and ladbuffer

    pthread_mutex_t ParametersMutex;  // use for cci thread and ladbuffer
    pthread_mutex_t StreaminMutex; // use for record mutex
    pthread_mutex_t ModeMutex; // use for record mutex
    pthread_mutex_t MasterVolumeMutex; // use for record mutex
    pthread_mutex_t ttyMutex; // use for tty turn off mutex


    Mutex   AudiomLock;// use for Audiostartrunning and stoprunning

#ifdef TEMP_SOLUTION_VM
    // for VM record thread
    pthread_t mMdReadThread;
#endif

    int lrnFlag;
    int lprFlag;
#endif

    LADPlayer          *pLadPlayer;
    LAD                *pLad;
    Play2Way           *pPlay2Way;
    Record2Way         *pRecord2Way;
#ifdef MTK_AP_SPEECH_ENHANCEMENT  	
	SPELayer 	*mpSPELayer;
#endif
    uint32_t	mOutputAvailable[android_audio_legacy::AudioSystem::NUM_MODES];
    uint32_t	mOuputForceuse[android_audio_legacy::AudioSystem::NUM_MODES];
	int        mOuputForceuseRef[android_audio_legacy::AudioSystem::NUM_MODES];
    uint32_t	mInputAvailable;
    uint32_t	mInputForceuse;
    int        mTty_Ctm;
    int        mVT_Speech;
    int        mPcm2Way_Enable;
    bool       mFmStatus;
    bool       mRecordFM;
    int        mic_type;
    int        mPhoneMode;
    int        mFmTxStatus;
    bool       mFMRXForceDisableFMTX;
    int        mForceBGSoff;
    bool       m_AnalogIn_ATV_Enable_during_InCall;
    bool       m_Speaker_Enable_during_InCall;
    bool       mMicMute_ModemStatus;

#if defined(MTK_DT_SUPPORT)
    int        mFlag_SkipEndCallDelay;
    int        mFlag_SecondModemPhoneCall;
    pthread_mutex_t mSpeechCtrlMutex;
#endif

#if defined(MTK_DUAL_MIC_SUPPORT)||defined(MTK_AUDIO_HD_REC_SUPPORT)
    bool       mDualMic_Test;
    bool       mDualMicMetaWB;
//    int        mMMI_DualMic_Setting;
    int        mHAL_DualMic_Setting;
    FILE       *m_DualMic_pOutFile;
    FILE       *m_DualMic_pInFile;
    String8   mDmTmpOutStr8;
    String8   mDmTmpInStr8;
    const char *m_str_pOutFile;
    const char *m_str_pInFile;
    int mDualMicTool_micGain;
    int mDualMicTool_receiverGain;
    int mDualMicTool_headsetGain;
    pthread_mutex_t dmicMutex;

    // for Dmsd record thread
    rb playDmsdBuf;
    rb recDmsdBuf;
    pthread_t mDmsdThread;
    pthread_mutex_t mDmsdBufMutex;  // protect Dmsd buffers
    bool DmsdThreadExit;
    pthread_cond_t mDmsdExit_Cond;
    pthread_mutex_t mDmsdMutex;

#endif
    // for EnableStereoOutput
    bool mEnableStereoOutput;

#ifdef  ENABLE_EXT_DAC
    AudioCustomizationInterface *mAudioCustomFunction;
#endif

#if defined(MTK_AUDIO_HD_REC_SUPPORT)
    int32 mHdRecScene; // for HD Record
#endif

#ifdef MTK_DUAL_MIC_SUPPORT
	bool mbInLRSwitch;
	int miUseSpecMIC;
#endif

protected:
    virtual status_t     dump(int fd, const Vector<String16>& args);

private:
    status_t             dumpInternals(int fd, const Vector<String16>& args);
    Mutex                mLock;
    int                     mFd;
    uint32_t            mDevice;
    uint32_t            mPredevice;
    uint32_t            mPreviousMode;
    bool                  mMicMute;
    bool                  mUpdateVolume;
    bool				mEarpieceStatus;
    bool				mHeadPhoneStatus;
    uint32_t            mSpeakerStatus;
    AudioAMPControlInterface *mAuioDevice;
    bool                  mBgsStatus;
    bool                  mFmDigitalInStatus;
    bool                  mFmDigitalOutStatus;
    bool                  mFmSpeakerStatus;
    bool                  mMatvAnalogstatus;
    bool                  mMatvDigitalstatus;
    bool                  mVoiceStatus;
    bool                  mCommunicationStatus;
    bool                  mForceSpeakerStatus;
    int                     ApRecordMicYype;
    SPH_Control     mAudio_Control_State;
	bool             mFakeMode; //used for receiver in normalmode
	sp<HeadSetDetect> mHeadsetDetect;
    int               mRoutingDevices;
	Mutex             mHDMILock;
	bool              mHDMIConnect;
    int               mHDMIAudioEnable;
};

// ----------------------------------------------------------------------------

}; // namespace android

#endif // ANDROID_AUDIO_HARDWARE_GENERIC_H
