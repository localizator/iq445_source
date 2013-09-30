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

/*******************************************************************************
 *
 * Filename:
 * ---------
 * AudioParamTuning.h
 *
 * Project:
 * --------
 *   Android
 *
 * Description:
 * ------------
 *   This file implements the method for  handling param tuning.
 *
 * Author:
 * -------
 *   Donglei Ji (mtk80823)
 *******************************************************************************/

#ifndef _AUDIO_PARAM_TUNING_H_
#define _AUDIO_PARAM_TUNING_H_

#include <utils/threads.h>

#include "AudioYusuVolumeController.h"
#include "AudioYusuHardware.h"
#include "AudioYusuLad.h"

#define MAX_VOICE_VOLUME VOICE_VOLUME_MAX
#define FILE_NAME_LEN_MAX 128
#define P2W_RECEIVER_OUT 0
#define P2W_HEADSET_OUT 1
#define P2W_NORMAL 0
#define P2W_RECONLY 1


typedef struct {
    unsigned short cmd_type;
    unsigned short slected_fir_index;
    unsigned short dlDigitalGain;
    unsigned short dlPGA;
    unsigned short phone_mode;
    unsigned short wb_mode;
    char input_file[FILE_NAME_LEN_MAX];
}AudioTasteTuningStruct;

typedef enum {
    AUD_TASTE_STOP = 0,
    AUD_TASTE_START,
    AUD_TASTE_DLDG_SETTING,
    AUD_TASTE_DLPGA_SETTING,
    AUD_TASTE_INDEX_SETTING,

    AUD_TASTE_CMD_NUM
}AUD_TASTE_CMD_TYPE;

typedef enum {
    PCM_FORMAT = 0,
    WAVE_FORMAT,

    UNSUPPORT_FORMAT
}FILE_FORMAT;

#ifdef MTK_DUAL_MIC_SUPPORT
typedef enum {
    AUD_MIC_GAIN = 0,
    AUD_RECEIVER_GAIN,
    AUD_HS_GAIN,

    AUD_GAIN_TYPE_NUM
}DMNRGainType;

typedef enum {
	DUAL_MIC_REC_PLAY_STOP=	0,
	DUAL_MIC_REC,
	DUAL_MIC_REC_PLAY,
	DUAL_MIC_REC_PLAY_HS,
	   
	DMNR_TUNING_CMD_CNT
}DMNRTuningCmdType;
#endif

namespace android {
#define MODE_NUM 3

class LAD;
class AudioYusuHardware;
class AudioYusuVolumeController;

class AudioParamTuning
{
public:
    AudioParamTuning(AudioYusuHardware *hw);
    ~AudioParamTuning();
   
    //for taste tool
    bool isPlaying();
    status_t setMode(uint32 mode);
    uint32 getMode();
    status_t setPlaybackFileName(const char *fileName);
    status_t setDLPGA(uint32 gain);
    void updataOutputFIRCoffes(AudioTasteTuningStruct *pCustParam);
    status_t enableModemPlaybackVIASPHPROC(bool bEnable, bool bWB=false);
    status_t putDataToModem();
    FILE_FORMAT playbackFileFormat();

    // for playback thread
    rb m_sPlayBuf;

    pthread_mutex_t mPlayBufMutex;  // protect Play PCM With Speech Enhancement buffers
    pthread_cond_t mPPSExit_Cond;
    pthread_mutex_t mPPSMutex;
    pthread_mutex_t mP2WMutex;

    bool m_bPPSThreadExit;
    bool m_bWBMode;
    FILE *m_pInputFile;

#ifdef MTK_DUAL_MIC_SUPPORT
    // For DMNR Tuning	
    status_t setRecordFileName(const char *fileName);
    status_t setDMNRGain(unsigned short type, unsigned short value); //for DMNR
    status_t getDMNRGain(unsigned short type, unsigned short *value); //for DMNR
    status_t enableDMNRModem2Way(bool bEnable, bool bWBMode, unsigned short outputDevice, unsigned short workMode);
    status_t readDataFromModem(uint32 offset, uint32 len);

    // for DMNR playback+record thread
    rb m_sRecBuf;

    pthread_mutex_t mDMNRMutex;	
    pthread_mutex_t mRecBufMutex;  // protect DMNR Playback+record buffers
    pthread_cond_t mDMNRExit_Cond;

    bool m_bDMNRThreadExit;
    FILE *m_pOutputFile;
#endif
	
private:
    LAD *m_pLad;
    AudioYusuHardware *m_pHW;
    AudioYusuVolumeController *m_pAudioVolumeController;

    uint32 mSideTone;
    uint32 mOutputVolume[MODE_NUM];
    uint32 mMode;
    char m_strInputFileName[FILE_NAME_LEN_MAX];
    bool m_bPlaying;

    pthread_t mTasteThreadID;

#ifdef MTK_DUAL_MIC_SUPPORT
    bool m_bDMNRPlaying;
    char m_strOutFileName[FILE_NAME_LEN_MAX]; // for reord
    unsigned short mDualMicTool_micGain;
    unsigned short mDualMicTool_receiverGain;
    unsigned short mDualMicTool_headsetGain;
    
    pthread_t mDMNRThreadID;
#endif

};

};

#endif

