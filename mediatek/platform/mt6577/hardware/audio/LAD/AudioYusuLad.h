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
 * AudioYusuLad.h
 *
 * Project:
 * --------
 *   Android Speech Driver
 *
 * Description:
 * ------------
 *   This file implements LAD(Legacy Audio Driver).
 *
 * Author:
 * -------
 *   HP Cheng (mtk01752)
 *
 *------------------------------------------------------------------------------
 * $Revision: #0 $
 * $Modtime:$
 * $Log:$
 * 
 * 08 15 2012 donglei.ji
 * [ALPS00337843] [Need Patch] [Volunteer Patch] MM Command Handler JB migration
 * MM Command Handler JB migration.
 *
 * 04 10 2012 weiguo.li
 * [ALPS00266592] [Need Patch] [Volunteer Patch] ICS_MP patchback to ALPS.ICS of Audio
 * .
 *
 *
 *******************************************************************************/

#ifndef _AUDIO_YUSU_LAD_H_
#define _AUDIO_YUSU_LAD_H_


/*=============================================================================
 *                              Include Files
 *===========================================================================*/
#include <sys/stat.h>
#include <pthread.h>

#include "AudioYusuDef.h"
#include "CFG_AUDIO_File.h"

namespace android {

/*=============================================================================
 *                             Constant definition
 *===========================================================================*/

#define AUDMSG "[AudMsg] "

/* A2M FUNCTION TYPE */

#define A2M_SetOutputVolume                   0x00
#define A2M_SetMicrophoneVolume               0x01
#define A2M_MuteMicrophone                    0x02
#define A2M_SetSidetoneVolume                 0x03
#define A2M_SetGainTableVolume                0x04
#define A2M_SetGainTableMicVolume             0x05
#define A2M_MuteSpeaker                       0x06

#define A2M_SetOutputDevice                   0x10
#define A2M_SetInputSource                    0x11
#define A2M_SetMicBias                        0x12
#define A2A_SetVCM                            0x13
#define A2M_SetLoudSPK_RefMicMode             0x14
#define A2M_SetDaiBtCfg                       0x15


#define A2M_Speech_On                         0x20
#define A2M_Speech_Off                        0x21
#define A2M_Set_DAI_Mode                      0x22
#define A2M_SetAfeLoopback                    0x23
#define A2M_SetSpeechMode                     0x24
#define A2M_SetReceiverTest                   0x25
#define A2M_SetSphEnhancementSpecific         0x26
#define A2M_SetSphEnhancementAll              0x27
#define A2M_SetAcousticLoopback               0x28

#define A2M_GetMDCapability                   0x29
#define A2M_SetMDCapability                   0x2a
#define A2M_LogMDSphCoeff                     0x2b

#define A2M_PCM_OPEN                          0x30
#define A2M_PCM_Close                         0x31
#define A2M_playback_turn_on                  0x32
#define A2M_playback_turn_off                 0x33
#define A2M_Record_turn_on                    0x34
#define A2M_Record_turn_off                   0x35
#define A2M_DualRecPlay_turn_on                   0x36
#define A2M_DualRecPlay_turn_off                   0x37
#define A2M_DualRecOnly_turn_on              0x38
#define A2M_DualRecOnly_turn_off              0x39
#define A2M_Record_Drop_Frame              0x3A


#define A2M_CTM_Open	                         0x40
#define A2M_CTM_Close	                      0x41
#define A2M_CTM_StartNegotiation	             0x42
#define A2M_BGSND_Open	                      0x43
#define A2M_BGSND_Close	                      0x44
#define A2M_BGSND_Config	                   0x45


//Data Notification
#define A2M_PCM_DataNotify	                   0x50
#define A2M_Playback_DataNotify               0x54
#define A2M_BGSND_DataNotify	                0x58
#define A2M_CTM_TxPutText	                   0x5C

//Send back type
#define A2M_PCM_DataSendBack                  0x60
#define A2M_Record_DataSendBack               0x64
#define A2M_CTM_DataSendBack                  0x68


//Misc.
#define BT_CALIBARTE                            0x70
#define AUDIO_CUSTOM_PARAMETER                  0x74
#define AUDIO_WB_CUSTOM_PARAMETER               0x7C
#define AUDIO_HD_RECORD_PARAMETER               0x7C // same as AUDIO_WB_CUSTOM_PARAMETER, hence use the different LADBufferId in share buffer header
#define AUDIO_DUAL_MIC_CUSTOM_PARAMETER         0x78

/* M2A MESSAGE TYPE */
#define M2A_DataRequest	                            0x8
#define M2A_DataNotification                        0x9
#define M2A_SpeechOnCompleted_Notification          0xa0
#define M2A_SpeechOffCompleted_Notification         0xa1
#define M2A_AfeLoopbackCompleted_Notification       0xa3
#define M2A_SpeechModeCompleted_Notification        0xa4
#define M2A_SphEnhanceSpecificCompleted_Notification  0xa6
#define M2A_SphEnhanceAllCompleted_Notification       0xa7
#define M2A_AcousticLoopbackCompleted_Notification  0xa8
#define M2A_GetMDCapability_Notification            0xa9
#define M2A_SetMDCapability_Notification            0xaa

#define M2A_PcmOpenCompleted_Notification           0xb0
#define M2A_PcmCloseCompleted_Notification          0xb1
#define M2A_RecordOnCompleted_Notification          0xb4
#define M2A_RecordOffCompleted_Notification         0xb5
#define M2A_DualRecPlay_turn_on_Notification        0xb6
#define M2A_DualRecPlay_turn_off_Notification       0xb7
#define M2A_CTMOpenCompleted_Notification           0xc0
#define M2A_CTMCloseCompleted_Notification          0xc1
#define M2A_BgsndOpenCompleted_Notification         0xc3
#define M2A_BgsndCloseCompleted_Notification        0xc4

#define M2A_AudCusParaCompleted_Notification        0xF4
#define M2A_AudDualMicParaCompleted_Notification    0xF8
#define M2A_AudWBParaCompleted_Notification         0xFC
#define M2A_AudHdRecordParaCompleted_Notification   0xFC // same as M2A_AudWBParaCompleted_Notification!!

//DataRequest
#define M2A_PCM_Request	                        0x80
#define M2A_Playback_Request	                  0x81
#define M2A_BGSND_Request	                     0x82

#define M2A_MD_Reset_Notify						   0x84

#define M2A_PCM_DataNotify                      0x90
#define M2A_Record_DataNotify                   0x94
#define M2A_CTM_RxGetText                       0x98

#define A2M_BUF_HEADER                          0xA2A2
#define M2A_BUF_HEADER                          0x2A2A
#define LAD_SHARE_HEADER_LEN                         6

//specail define for pcm2way state
#define PCM2WAY_PLAY_ON                         (1 << 1)
#define PCM2WAY_REC_ON                          (1 << 2)
#if defined(MTK_WB_SPEECH_SUPPORT)
#define PCM2WAY_WB_PLAY_ON                      (1 << 4)
#endif

//define for calibration format
typedef enum {
  P2W_FORMAT_NORMAL = 0,
  P2W_FORMAT_VOIP,
  P2W_FORMAT_CAL,          //calibration
  P2W_FORMAT_WB_CAL        //WB calibration
}P2W_Format;

#define PCM2WAY_BUF_SIZE                        320

#define DUALMIC2WAY_STOP                     0
#define DUALMIC2WAY_REC_ONLY            1
#define DUALMIC2WAY_PLAYREC               2



/*=============================================================================
 *                              Type definition
 *===========================================================================*/

enum LADBufferId
{
   LADBUFID_PCM_FillSE = 0,
   LADBUFID_PCM_FillSpk,
   LADBUFID_PCM_GetFromMic,
   LADBUFID_PCM_GetfromSD,
   LADBUFID_CCCI_VM_TYPE,
   LADBUFID_CCCI_PCM_TYPE,
   LADBUFID_CCCI_BGS_TYPE,
   LADBUFID_CCCI_EM_PARAM,
   LADBUFID_CCCI_MAX_TYPE

};

#define LADBUFID_CCCI_HD_RECORD (LADBUFID_CCCI_EM_PARAM | 0x10)

enum LADDataType
{
   LADDATA_PCM =  0,
   LADDATA_VM = 1,
   LADDATA_DUAL_MIC_VM = 2,
   LADDATA_CTM_4WAY = 3,

};

enum LadOutPutDevice_Line
{
   LADOUT_SPEAKER1 =  0,    // Earpiece(Receiver)
   LADOUT_SPEAKER2  = 1,    // Earphone
   LADOUT_LOUDSPEAKER = 2,  // Phone Loud Speaker
   LADOUT_BTOut = 3,

};

enum LadInPutDevice_Line
{
    LADIN_FM_Radio = 0,       // 0: FM analog line in Mono
    LADIN_Microphone1,        // 1: Phone Mic
    LADIN_Microphone2,        // 2: Earphone Mic
    LADIN_BTIn,               // 3: BT Mic
    LADIN_SingleDigitalMic,   // 4: Single digital Microphone
    LADIN_DualAnalogMic,      // 5: Dual analog Microphone
    LADIN_DualDigitalMic,     // 6: Dual digital Microphone
    LADIN_FM_AnalogIn_Stereo,  // 7: FM analog line in Stereo
    NUM_OF_InputDevice
};

enum ENUM_SPH_FIR_COEFF
{
    SPH_FIR_COEFF_NORMAL            = 0,
    SPH_FIR_COEFF_HEADSET           = 1,
    //SPH_FIR_COEFF_HANDFREE        = 2,
    SPH_FIR_COEFF_BT                = 3,
    //SPH_FIR_COEFF_VOIP_NORMAL     = 4,
    //SPH_FIR_COEFF_VOIP_HANDFREE   = 5,
    SPH_FIR_COEFF_HANDSET_MIC2      = 6,
    SPH_FIR_COEFF_VOICE_REC      = 7 ,
    SPH_FIR_COEFF_NONE              = 0xFF
};

enum ENUM_Speech_Mode
{
   SPH_MODE_NORMAL = 0,
   SPH_MODE_EARPHONE,
   SPH_MODE_LOUDSPK,
   SPH_MODE_BT_EARPHONE,
   SPH_MODE_BT_CORDLESS,
   SPH_MODE_BT_CARKIT,
   SPH_MODE_PRESERVED_1,
   SPH_MODE_PRESERVED_2,
   SPH_MODE_NO_CONNECT,
};

#define HD_RECORD_MODE_OFFSET SPH_MODE_NO_CONNECT // should be fixed at 8

enum PCM_Record_SamplingRate
{
   Record_8k = 0,
   Record_16k
};


/// ---> speech control speech enhancement information
#if defined(MTK_SPH_EHN_CTRL_SUPPORT)
enum ENUM_SPH_ENH_MASK
{
     SPH_ENH_MASK_BKF      = (1 << 0),
     SPH_ENH_MASK_AEC      = (1 << 1),
     SPH_ENH_MASK_NDC      = (1 << 2),
     SPH_ENH_MASK_DMNR     = (1 << 3),
     SPH_ENH_MASK_DGA      = (1 << 4),
     SPH_ENH_MASK_PLC      = (1 << 5),
     SPH_ENH_MASK_AGC      = (1 << 6),
     SPH_ENH_MASK_TDNC     = (1 << 7),
     SPH_ENH_MASK_SIDETONE = (1 << 8),
     SPH_ENH_MASK_ALL      = 0xFFFF
};

enum ENUM_SPH_ENH_SUBFUNC_MASK
{
     SPH_ENH_SUBFUNC_MASK_VCE = (1 << 0),
     SPH_ENH_SUBFUNC_MASK_ALL = 0xFFFFFFFF
};

typedef struct _SPH_ENH_INFO_T
{
    uint16 spe_usr_mask;            // BKF,AEC,NDC,DMNR,DGA,PLC,AGC,TDNC,SIDETONE, ...
    uint32 spe_usr_subFunc_mask;    // VCE, ...
} SPH_ENH_INFO_T;

enum ENUM_SPH_ENH_CTRL
{
    //SPH_ENH_DISABLE  = 0, // disable speech enhancement on application running, not used now
    //SPH_ENH_ENABLE   = 1, // enable speech enhancement on application running,  not used now
    SPH_ENH_USR_CFG1   = 2, // set spe_usr_mask
    SPH_ENH_USR_CFG2_1 = 3, // set spe_usr_subFunc_mask[0:15]
    SPH_ENH_USR_CFG2_2 = 4, // set spe_usr_subFunc_mask[16:31]
    SPH_ENH_APPLY      = 5  // apply setting
};
#endif //MTK_SPH_EHN_CTRL_SUPPORT
/// <--- speech control speech enhancement information


/*=============================================================================
 *                              Class definition
 *===========================================================================*/
class AudioYusuHardware;
class AudioCCCI;
class LADPlayer;
class AudioYusuHeadSetMessager;
#if defined(MTK_DT_SUPPORT)
class AudioATCommand;
#endif

class LAD
{
private:
   int32 mPCM2WayState;

   pthread_mutex_t mSendSphParaProcedureMutex;

public:
   AudioCCCI *pCCCI;      /* pointer to CCCI interface */
#if defined(MTK_DT_SUPPORT	)
   AudioATCommand *pATCmd;      /* pointer to AudioATCommand interface */
#endif
   AudioYusuHeadSetMessager *mHeadSetMessager;
   AudioYusuHardware *mHw;    /* pointer to HW */
   LAD(AudioYusuHardware *hw);
   ~LAD();
   bool LAD_Initial();
   bool LAD_Deinitial();
   bool LAD_OpenNormalRecPath(int32 format, int32 sampleRate);
   bool LAD_CloseNormalRecPath();
   bool LAD_NormalRecDataSendBack();
   bool LAD_SetRecDropFrameCount( int32 frame_count);
   bool LAD_SetOutputVolume(uint32 Gain);
   bool LAD_SetGainTableOutputVolume(int32 Gain);
   bool LAD_SetGainTableMicVolume(int32 Gain);
   bool LAD_SetMicrophoneVolume(uint32 Gain);
   bool LAD_MuteMicrophone(bool ENABLE);
   bool LAD_MuteSpeaker(bool ENABLE);
   bool LAD_SetSidetoneVolume(uint32 Gain);
   bool LAD_SetOutputDevice(LadOutPutDevice_Line HW_Mode);
   bool LAD_SetInputSource(LadInPutDevice_Line HW_Mode);
   bool LAD_ForceSetInputSource(LadInPutDevice_Line HW_Mode);
   bool LAD_PlaybackTurnOn(int32 data_type);
   bool LAD_PlaybackTurnOff();
   bool LAD_PlaybackDataNotify(uint32 offset, uint32 len);
   bool LAD_SetSpeechMode(ENUM_Speech_Mode Speech_Mode);
   bool LAD_Speech_On(int sp_mode);
   bool LAD_Speech_Off();
   bool LAD_SwitchMicBias(int32 TurnOn);
   bool LAD_PCM2WayPlayOn();
   bool LAD_PCM2WayPlayOff();
   bool LAD_PCM2WayRecordOn();
   bool LAD_PCM2WayRecordOff();

   bool LAD_PCM2WayOn(bool bWB=false);
   bool LAD_PCM2WayOff();
   bool LAD_PCM2WayDataSendBack();
   bool LAD_PCM2WayDataNotify(uint32 offset, uint32 len);

#if defined(MTK_DUAL_MIC_SUPPORT)||defined(MTK_AUDIO_HD_REC_SUPPORT)
   // for META tool dual-mic
   int32 mDualMic2WayState;
   bool LAD_DualMic_2WayOn(int WBMode, int PlayRec);
   bool LAD_DualMic_2WayOff();
#endif

#if defined(MTK_DUAL_MIC_SUPPORT)
   int LAD_SetDualMicParameters(AUDIO_CUSTOM_EXTRA_PARAM_STRUCT* pParam);
#endif

   bool LAD_SetAfeLoopback(bool bLoopbackOn, bool bSwapADDA, bool bConnectAllADDA);
   bool LAD_SetAcousticLoopback(bool bLoopbackOn, bool bUseDMNR);
   bool LAD_SetReceiverTest(bool turn_ON);
   bool LAD_Set_DAI_Mode(bool turn_ON);
   bool LAD_SwitchVCM(int32 TurnOn);
   bool LAD_Set_LoudSPK_RefMicMode(bool turn_ON);
   bool LAD_SetDaiBtCfg(bool bDaiBtMode, bool bBtLongSync);

   bool LAD_BGSoundOn();
   bool LAD_BGSoundOff();
   bool LAD_BGSoundDataNotify(uint32 offset, uint32 len);
   bool LAD_BGSoundConfig(uint32 UL_gain, uint32 DL_gain);

   int LAD_SetSpeechParameters(AUDIO_CUSTOM_PARAM_STRUCT* pParam);

   int LAD_Set_Speech_Enhancement(bool bEnable);
#if defined(MTK_SPH_EHN_CTRL_SUPPORT)
   void LoadSphEnhMasks();
   void SaveSphEnhMasks();

   int LAD_Set_Speech_Enhancement_Info(const SPH_ENH_INFO_T &eSphEnhInfo);
   int LAD_Set_Speech_Enhancement_MainFunction(uint32 MainConfig);
   int LAD_Set_Speech_Enhancement_SubFunctionHiBits(uint32 SubConfig);
   int LAD_Set_Speech_Enhancement_SubFunctionLowBits(uint32 SubConfig);
   int LAD_Set_Speech_Enhancement_FunctionEnable(void);
   int LAD_Set_Speech_Enhancement_Enable(bool bEnable);
#endif

   int LAD_GetMDCapability(void);
   int LAD_SetMDCapability(int32 value);
   int LAD_LogMDSphCoeff(void);

#if defined(MTK_WB_SPEECH_SUPPORT)
   int LAD_SetWBSpeechParameters(AUDIO_CUSTOM_WB_PARAM_STRUCT* pWBParam);
#endif

#if defined(MTK_AUDIO_HD_REC_SUPPORT)
   int LAD_SetHdRecordParameters(AUDIO_HD_RECORD_PARAM_STRUCT* pParam);
#endif

   bool LAD_SendSphParaProcedure();

   bool LAD_TtyCtmOn(int tty_mode);
   bool LAD_TtyCtmOff();
   int  LAD_RecordSR();
#if defined(MTK_DT_SUPPORT)
   bool LAD_Default_Tone_Play(uint8 toneIdx);
   bool LAD_Default_Tone_Stop();
   bool LAD_ATCMD_Test();
   pthread_t h2ndMD_ReadRecDataThread;
#endif

   bool mTty_used_in_phone_call;

   bool mSph_Enh_enable;
#if defined(MTK_SPH_EHN_CTRL_SUPPORT)
   SPH_ENH_INFO_T mSphEnhInfo;
#endif

   uint32 m_SideToneGain;
   uint32 m_MicGain;
   int m_InputSrc;
};

int32 WriteShareBufHeader(int8 *ptr, int16 type, int16 len, int16 sync);

};

#endif /*_AUDIO_YUSU_LAD_H_*/
