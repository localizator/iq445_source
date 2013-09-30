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
 * AudioParamTuning.cpp
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

#include <unistd.h>
#include <sched.h>
#include <sys/prctl.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <cutils/xlog.h>

#include "AudioParamTuning.h"
#include "AudioCustParam.h"
#include "AudioYusuCcci.h"
#include "AudioYusuDef.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define PLAYBUF_SIZE 6400l
#define A2M_SHARED_BUFFER_OFFSET  (1408)
#define WAV_HEADER_SIZE 44

#define LOG_TAG "AudioParamTuning"
namespace android {

static void *Play_PCM_With_SpeechEnhance_Routine(void *arg)
{
    SXLOGD("Play_PCM_With_SpeechEnhance_Routine in +");
    AudioParamTuning *pAUDParamTuning = (AudioParamTuning*)arg;	
	
    if(pAUDParamTuning == NULL) {
        SXLOGE("Play_PCM_With_SpeechEnhance_Routine pAUDParamTuning = NULL arg = %x",arg);
        return 0;
    }

    uint32 PCM_BUF_SIZE = pAUDParamTuning->m_bWBMode?(2*PLAYBUF_SIZE):(PLAYBUF_SIZE);
    unsigned long sleepTime = (PLAYBUF_SIZE/320)*20*1000;
    // open AudioRecord
    pthread_mutex_lock(&pAUDParamTuning->mPPSMutex);

    // Adjust thread priority
    prctl(PR_SET_NAME, (unsigned long)"PlaybackWithSphEnRoutine", 0, 0, 0);
    setpriority(PRIO_PROCESS, 0, ANDROID_PRIORITY_AUDIO);
    //Prepare file pointer
    FILE* pFd = pAUDParamTuning->m_pInputFile;                 //file for input

    // Prepare input & output memory
    rb* playBuf = &pAUDParamTuning->m_sPlayBuf;
    playBuf->pBufBase = new char[PCM_BUF_SIZE];
    playBuf->pRead    = playBuf->pBufBase;
    playBuf->pWrite   = playBuf->pBufBase;
    playBuf->bufLen   = PCM_BUF_SIZE;
    memset((void*)playBuf->pBufBase , 0, PCM_BUF_SIZE);

    // ----start the loop --------
    pAUDParamTuning->m_bPPSThreadExit = false;
    int numOfBytesPlay =0;
    int playBufFreeCnt = 0;
    int totalBytes =0;
    int cntR = 0;
    char* tmp = new char[PCM_BUF_SIZE+1];
    fread(tmp, sizeof(char), WAV_HEADER_SIZE, pFd);
    memset(tmp, 0, PCM_BUF_SIZE+1);

    SXLOGD("pthread_cond_signal(&pAUDParamTuning->mPPSExit_Cond), buffer size=%d",PCM_BUF_SIZE);
    pthread_cond_signal(&pAUDParamTuning->mPPSExit_Cond); // wake all thread
    pthread_mutex_unlock(&pAUDParamTuning->mPPSMutex);

    while((!pAUDParamTuning->m_bPPSThreadExit) && pFd) {
        pthread_mutex_lock(&pAUDParamTuning->mPlayBufMutex);

        //handling playback buffer
        playBufFreeCnt = playBuf->bufLen - rb_getDataCount(playBuf) - 1;
        cntR = fread(tmp, sizeof(char), playBufFreeCnt, pFd);
        if (cntR>0&&playBuf->pBufBase!=NULL) {
            rb_copyFromLinear(playBuf, tmp, cntR);
            numOfBytesPlay += cntR;
        }
        SXLOGD(" Playback buffer, readReq:%d, read:%d, total play:%d", playBufFreeCnt, cntR, numOfBytesPlay);
        pthread_mutex_unlock(&pAUDParamTuning->mPlayBufMutex);

        if (cntR<playBufFreeCnt) {
            SXLOGD("File reach the end");
            usleep(sleepTime); ////wait to all data is played
            break;
        }

        usleep(sleepTime/2);
    }

    // free buffer
    if (tmp!=NULL) {
        delete[] tmp;
        tmp = NULL;
    }
	
    pthread_mutex_lock(&pAUDParamTuning->mPlayBufMutex);
    if (playBuf->pBufBase!=NULL) {
        delete[] playBuf->pBufBase;
        playBuf->pBufBase = NULL;
    }
    pthread_mutex_unlock(&pAUDParamTuning->mPlayBufMutex);

    if (!pAUDParamTuning->m_bPPSThreadExit) {
        pAUDParamTuning->m_bPPSThreadExit = true;
        pAUDParamTuning->enableModemPlaybackVIASPHPROC(false);
        AudioTasteTuningStruct sRecoveryParam;
        sRecoveryParam.cmd_type = (unsigned short)AUD_TASTE_STOP;
        sRecoveryParam.wb_mode  = pAUDParamTuning->m_bWBMode;
        pAUDParamTuning->updataOutputFIRCoffes(&sRecoveryParam);      
    }
	
    //exit thread
    SXLOGD( "playbackRoutine pthread_mutex_lock");
    pthread_mutex_lock(&pAUDParamTuning->mPPSMutex);
    SXLOGD("pthread_cond_signal(&pAUDParamTuning->mPPSExit_Cond)");
    pthread_cond_signal(&pAUDParamTuning->mPPSExit_Cond); // wake all thread
    pthread_mutex_unlock(&pAUDParamTuning->mPPSMutex);

    return 0;
}

#ifdef MTK_DUAL_MIC_SUPPORT
static void *DMNR_Play_Rec_Routine(void *arg)
{
    SXLOGD("DMNR_Play_Rec_Routine in +");
    AudioParamTuning *pDMNRTuning = (AudioParamTuning*)arg;	
    if(pDMNRTuning == NULL) {
        SXLOGE("DMNR_Play_Rec_Routine pDMNRTuning = NULL arg = %x",arg);
        return 0;
    }

    uint32 PCM_BUF_SIZE = pDMNRTuning->m_bWBMode?(2*PLAYBUF_SIZE):(PLAYBUF_SIZE);
    unsigned long sleepTime = (PLAYBUF_SIZE/320)*20*1000;
    // open AudioRecord
    pthread_mutex_lock(&pDMNRTuning->mDMNRMutex);

    // Adjust thread priority
    prctl(PR_SET_NAME, (unsigned long)"DualMicCalibrationRoutine", 0, 0, 0);
    setpriority(PRIO_PROCESS, 0, ANDROID_PRIORITY_AUDIO);
	
    //Prepare file pointer
    FILE* pInFp = pDMNRTuning->m_pInputFile;      //file for input
    FILE* pOutFp = pDMNRTuning->m_pOutputFile;    //file for input

    // Prepare input & output memory
    rb* playBuf = &pDMNRTuning->m_sPlayBuf;
    playBuf->pBufBase = new char[PCM_BUF_SIZE];
    playBuf->pRead    = playBuf->pBufBase;
    playBuf->pWrite   = playBuf->pBufBase;
    memset((void*)playBuf->pBufBase , 0, PCM_BUF_SIZE);
    playBuf->bufLen = PCM_BUF_SIZE;
    
    rb* recBuf = &pDMNRTuning->m_sRecBuf;
    recBuf->pBufBase = new char[PCM_BUF_SIZE];
    memset((void*)recBuf->pBufBase , 0, PCM_BUF_SIZE);
    recBuf->pRead    = recBuf->pBufBase;
    recBuf->pWrite   = recBuf->pBufBase;
    recBuf->bufLen = PCM_BUF_SIZE;

    // ----start the loop --------
    char* tmp = new char[PCM_BUF_SIZE];
    pDMNRTuning->m_bDMNRThreadExit = false;
    int numOfBytesPlay =0;
    int numOfBytesRec =0;
    int totalBytes =0;

    SXLOGD("pthread_cond_signal(&pDMNRTuning->mDMNRExit_Cond)");
    pthread_cond_signal(&pDMNRTuning->mDMNRExit_Cond); // wake all thread
    pthread_mutex_unlock(&pDMNRTuning->mDMNRMutex);

    while((!pDMNRTuning->m_bDMNRThreadExit)&&pOutFp) {
        //handling playback buffer
        pthread_mutex_lock(&pDMNRTuning->mPlayBufMutex);
        if (pInFp) {
            int playBufFreeCnt = playBuf->bufLen - rb_getDataCount(playBuf) -1;
            int cntR = fread(tmp, sizeof(char), playBufFreeCnt, pInFp);
            rb_copyFromLinear(playBuf, tmp, cntR);
            numOfBytesPlay += cntR;
            SXLOGD(" Playback buffer, free:%d, read from :%d, total play:%d", playBufFreeCnt, cntR, numOfBytesPlay);
        }
        pthread_mutex_unlock(&pDMNRTuning->mPlayBufMutex);
		
        // handling record buffer
        pthread_mutex_lock(&pDMNRTuning->mRecBufMutex);
        int recBufDataCnt = rb_getDataCount(recBuf);
        rb_copyToLinear(tmp, recBuf, recBufDataCnt);
        int cntW = fwrite((void*)tmp,sizeof(char),recBufDataCnt,pOutFp);
        numOfBytesRec += cntW;
        SXLOGD(" Record buffer, available:%d, write to file:%d, total rec:%d", recBufDataCnt, cntW, numOfBytesRec);
        pthread_mutex_unlock(&pDMNRTuning->mRecBufMutex);

        usleep(sleepTime/2);
    }

    // free buffer
    pthread_mutex_lock(&pDMNRTuning->mPlayBufMutex);
    delete[] tmp;
    tmp = NULL;
    playBuf->pBufBase = NULL;
    delete[] playBuf->pBufBase;
    pthread_mutex_unlock(&pDMNRTuning->mPlayBufMutex);

    pthread_mutex_lock(&pDMNRTuning->mRecBufMutex);
    delete[] recBuf->pBufBase;
    recBuf->pBufBase = NULL;
    pthread_mutex_unlock(&pDMNRTuning->mRecBufMutex);

    //exit thread
    SXLOGD( "VmRecordRoutine pthread_mutex_lock");
    pthread_mutex_lock(&pDMNRTuning->mDMNRMutex);
    SXLOGD("pthread_cond_signal(&mDMNRExit_Cond)");
    pthread_cond_signal(&pDMNRTuning->mDMNRExit_Cond); // wake all thread
    pthread_mutex_unlock(&pDMNRTuning->mDMNRMutex);

    return 0;
}
#endif
AudioParamTuning::AudioParamTuning(AudioYusuHardware *hw) :
	mMode(0),
	mSideTone(0xFFFFFF40),
	m_bPlaying(false),
	m_bWBMode(false),
	m_bPPSThreadExit(false),
	m_pInputFile(NULL),
	m_pHW(hw)
{
    SXLOGD("AudioParamTuning in +");
    m_pLad = m_pHW->pLad;
    m_pAudioVolumeController = m_pHW->mVolumeController;

    memset(mOutputVolume, 0, MODE_NUM*sizeof(uint32));
    memset(m_strInputFileName, 0, FILE_NAME_LEN_MAX*sizeof(char));

    int ret = pthread_mutex_init(&mP2WMutex, NULL);
    if ( ret != 0 ) 
        SXLOGE("Failed to initialize pthread mP2WMutex!");

    ret = pthread_mutex_init(&mPPSMutex, NULL);
    if ( ret != 0 ) 
        SXLOGE("Failed to initialize mPPSMutex!");
	
    ret = pthread_mutex_init(&mPlayBufMutex, NULL);
    if ( ret != 0 ) 
        SXLOGE("Failed to initialize mPlayBufMutex!");
	
    ret = pthread_cond_init(&mPPSExit_Cond, NULL);
    if ( ret != 0 ) 
        SXLOGE("Failed to initialize mPPSExit_Cond!");

#ifdef MTK_DUAL_MIC_SUPPORT	
    m_bDMNRPlaying = false;
    m_bDMNRThreadExit = false;
    m_pOutputFile = NULL;
    memset(m_strOutFileName, 0, FILE_NAME_LEN_MAX*sizeof(char));

    AUDIO_VOLUME_CUSTOM_STRUCT VolumeCustomParam;//volume custom data
    GetAudioCustomParamFromNV(&VolumeCustomParam);

    mDualMicTool_micGain = VolumeCustomParam.audiovolume_mic[VOL_NORMAL][3];
    if (mDualMicTool_micGain>UPLINK_GAIN_MAX)
        mDualMicTool_micGain = UPLINK_GAIN_MAX;

    mDualMicTool_receiverGain = VolumeCustomParam.audiovolume_sph[VOL_NORMAL][CUSTOM_VOLUME_STEP-1];
    if (mDualMicTool_receiverGain>MAX_VOICE_VOLUME)
        mDualMicTool_receiverGain = MAX_VOICE_VOLUME;
	
    mDualMicTool_headsetGain = VolumeCustomParam.audiovolume_sph[VOL_HEADSET][3];
    if (mDualMicTool_headsetGain>MAX_VOICE_VOLUME)
        mDualMicTool_headsetGain = MAX_VOICE_VOLUME;

    ret = pthread_mutex_init(&mDMNRMutex, NULL);
    if ( ret != 0 ) 
        SXLOGE("Failed to initialize mDMNRMutex!");
	
    ret = pthread_mutex_init(&mRecBufMutex, NULL);
    if ( ret != 0 ) 
        SXLOGE("Failed to initialize mRecBufMutex!");
	
    ret = pthread_cond_init(&mDMNRExit_Cond, NULL);
    if ( ret != 0 ) 
        SXLOGE("Failed to initialize mDMNRExit_Cond!");
	
    SXLOGD("AudioYusuParamTuning: default mic gain:%d, receiver gain:%d, headset Gain:%d", mDualMicTool_micGain, mDualMicTool_receiverGain, mDualMicTool_headsetGain);
#endif

}
	
AudioParamTuning::~AudioParamTuning()
{
    SXLOGD("~AudioParamTuning in +");
}
	
//for taste tool
bool AudioParamTuning::isPlaying()
{
    SXLOGV("isPlaying - playing:%d", m_bPlaying);
    bool ret = false;
    pthread_mutex_lock(&mP2WMutex);
#ifdef MTK_DUAL_MIC_SUPPORT	
    SXLOGV("isPlaying - DMNR playing:%d", m_bDMNRPlaying);
    ret = (m_bPlaying | m_bDMNRPlaying)?true:false;
#else
    ret = m_bPlaying;
#endif
    pthread_mutex_unlock(&mP2WMutex);
    return ret;
}

uint32 AudioParamTuning::getMode()
{
    SXLOGD("getMode - mode:%d", mMode);
    pthread_mutex_lock(&mP2WMutex);
    uint32 ret = mMode;
    pthread_mutex_unlock(&mP2WMutex);
    return ret;
}

status_t AudioParamTuning::setMode(uint32 mode)
{
    SXLOGD("setMode - mode:%d", mode);
    pthread_mutex_lock(&mP2WMutex);
    mMode = mode;
    pthread_mutex_unlock(&mP2WMutex);
    return NO_ERROR;
}

status_t AudioParamTuning::setPlaybackFileName(const char * fileName)
{
    SXLOGD("setPlaybackFileName in +");
    pthread_mutex_lock(&mP2WMutex);
    if (fileName!=NULL && strlen(fileName)<FILE_NAME_LEN_MAX-1) {
        SXLOGD("input file name:%s", fileName);
        memset(m_strInputFileName, 0, FILE_NAME_LEN_MAX);
        strcpy(m_strInputFileName,fileName);
    }else {
        SXLOGE("input file name NULL or too long!");
        pthread_mutex_unlock(&mP2WMutex);
        return BAD_VALUE;
    }
    pthread_mutex_unlock(&mP2WMutex);
    return NO_ERROR;
}

status_t AudioParamTuning::setDLPGA(uint32 gain)
{
    SXLOGD("setDLPGA in +");
    uint32 outputDev = 0;
	
    if (gain>MAX_VOICE_VOLUME) {
        SXLOGE("setDLPGA gain error  gain=%x",gain);
        return BAD_VALUE;
    }
	
    pthread_mutex_lock(&mP2WMutex);
    mOutputVolume[mMode] = 0xFFFFFFFF - (MAX_VOICE_VOLUME - gain)/2;
    SXLOGD("setDLPGA mode=%d, gain=%d, lad volume=0x%x", mMode, gain, mOutputVolume[mMode]);

    if (m_bPlaying) {
        SXLOGD("setDLPGA lad_Volume=%x",mOutputVolume[mMode]);
        m_pLad->LAD_SetOutputVolume(mOutputVolume[mMode]);
    }

    pthread_mutex_unlock(&mP2WMutex);
    return NO_ERROR;
}
	 
void AudioParamTuning::updataOutputFIRCoffes(AudioTasteTuningStruct *pCustParam)
{
    SXLOGD("updataOutputFIRCoffes in +");

    int ret = 0;
    unsigned short mode = pCustParam->phone_mode;
    unsigned short cmdType = pCustParam->cmd_type;

    pthread_mutex_lock(&mP2WMutex);

    if (m_bPlaying&&mode==mMode){
        m_pLad->LAD_PCM2WayOff(); // trun off PCM2Way
        usleep(10*1000);   //wait to make sure all message is processed
    }
	
    if (pCustParam->wb_mode) {
#if defined(MTK_WB_SPEECH_SUPPORT)
        AUDIO_CUSTOM_WB_PARAM_STRUCT sCustWbParam;
        GetCustWBParamFromNV(&sCustWbParam);
        if(cmdType&&sCustWbParam.speech_mode_wb_para[mode][7]!=pCustParam->dlDigitalGain) {
            SXLOGD("updataOutputFIRCoffes mode=%d, ori dlDG gain=%d, new dlDG gain=%d", mode, sCustWbParam.speech_mode_wb_para[mode][7],pCustParam->dlDigitalGain);
            sCustWbParam.speech_mode_wb_para[mode][7] = pCustParam->dlDigitalGain;
        }
        ret = m_pLad->LAD_SetWBSpeechParameters(&sCustWbParam);
#endif
    }else {
        AUDIO_CUSTOM_PARAM_STRUCT sCustParam;
        AUDIO_PARAM_MED_STRUCT sCustMedParam;
        unsigned short index = pCustParam->slected_fir_index;
        unsigned short dlGain = pCustParam->dlDigitalGain;
        GetCustParamFromNV(&sCustParam);
        GetMedParamFromNV(&sCustMedParam);

        if ((cmdType==(unsigned short)AUD_TASTE_START||cmdType==(unsigned short)AUD_TASTE_INDEX_SETTING)&&sCustMedParam.select_FIR_output_index[mode]!=index) {
            SXLOGD("updataOutputFIRCoffes mode=%d, old index=%d, new index=%d", mode, sCustMedParam.select_FIR_output_index[mode], index);
            //save  index to MED with different mode.
            sCustMedParam.select_FIR_output_index[mode]= index;

            SXLOGD("updataOutputFIRCoffes ori sph_out_fir[%d][0]=%d, ori sph_out_fir[%d][44]", mode,sCustParam.sph_out_fir[mode][0],mode,sCustParam.sph_out_fir[mode][44]);
            //copy med data into audio_custom param
            memcpy((void*)sCustParam.sph_out_fir[mode],(void*)sCustMedParam.speech_output_FIR_coeffs[mode][index],sizeof(sCustParam.sph_out_fir[index]));
            SXLOGD("updataOutputFIRCoffes new sph_out_fir[%d][0]=%d, new sph_out_fir[%d][44]", mode,sCustParam.sph_out_fir[mode][0],mode,sCustParam.sph_out_fir[mode][44]);
            SetCustParamToNV(&sCustParam);
            SetMedParamToNV(&sCustMedParam);
        }

        if ((cmdType==(unsigned short)AUD_TASTE_START||cmdType==(unsigned short)AUD_TASTE_DLDG_SETTING)&&sCustParam.speech_mode_para[mode][7]!=dlGain) {
            SXLOGD("updataOutputFIRCoffes mode=%d, old dlDGGain=%d, new dlDGGain=%d", mode, sCustParam.speech_mode_para[mode][7], dlGain);
            sCustParam.speech_mode_para[mode][7] = dlGain;
        }
        SXLOGD("updataOutputFIRCoffes  sph_out_fir[%d][0]=%d, sph_out_fir[%d][44]", mode,sCustParam.sph_out_fir[mode][0],mode,sCustParam.sph_out_fir[mode][44]);
        ret = m_pLad->LAD_SetSpeechParameters(&sCustParam);
    }

    if (m_bPlaying&&mode==mMode){
         m_pLad->LAD_SetInputSource(LADIN_Microphone1);
        switch(mMode) {
          case SPH_MODE_NORMAL:
          {
              m_pLad->LAD_SetOutputDevice(LADOUT_SPEAKER1);
              
              m_pLad->LAD_SetSidetoneVolume(mSideTone);                                                                                                         // in 0.5dB
              m_pLad->LAD_SetOutputVolume(mOutputVolume[mMode]);
              SXLOGD("speech mode: %d, receiver side tone: 0x%x, lad_Volume: 0x%x", mMode, mSideTone,mOutputVolume[mMode]);

              m_pLad->LAD_SetSpeechMode(SPH_MODE_NORMAL);
              break;
          }
          case SPH_MODE_EARPHONE:
          {
              m_pLad->LAD_SetOutputDevice(LADOUT_SPEAKER2);

              m_pLad->LAD_SetSidetoneVolume(mSideTone);
              m_pLad->LAD_SetOutputVolume(mOutputVolume[mMode]);
              SXLOGD("speech mode: %d, receiver side tone: 0x%x, lad_Volume: 0x%x", mMode, mSideTone,mOutputVolume[mMode]);

              m_pLad->LAD_SetSpeechMode(SPH_MODE_EARPHONE);
              break;
          }
          case SPH_MODE_LOUDSPK:
              m_pLad->LAD_SetOutputDevice(LADOUT_LOUDSPEAKER);

              m_pLad->LAD_SetSidetoneVolume(mSideTone);
              m_pLad->LAD_SetOutputVolume(mOutputVolume[mMode]);
              SXLOGD("speech mode: %d, receiver side tone: 0x%x, lad_Volume: 0x%x", mMode, mSideTone,mOutputVolume[mMode]);

              m_pLad->LAD_SetSpeechMode(SPH_MODE_LOUDSPK);
              break;
          default:
              break;
        }
#if defined(MTK_SPH_EHN_CTRL_SUPPORT)
        SPH_ENH_INFO_T eSphEnhInfo;
        eSphEnhInfo.spe_usr_mask = 0xFFFF;
        eSphEnhInfo.spe_usr_subFunc_mask = 0xFFFFFFFF;
        m_pLad->LAD_Set_Speech_Enhancement_Info(eSphEnhInfo);
#endif		
        m_pLad->LAD_PCM2WayOn(m_bWBMode); // start PCM 2 way
#if defined(MTK_SPH_EHN_CTRL_SUPPORT)
        m_pLad->LAD_Set_Speech_Enhancement_Enable(true);
#endif
    }
    pthread_mutex_unlock(&mP2WMutex);
}

status_t AudioParamTuning::enableModemPlaybackVIASPHPROC(bool bEnable, bool bWB)
{
    SXLOGD("enableModemPlaybackVIASPHPROC bEnable:%d, bWBMode:%d", bEnable, bWB);
    uint32 lad_Volume = 0;
    int degradegain = SIDETONE_MAX_GAIN_DEGRADE;
    int ret = 0;

    // 3 sec for creat thread timeout
    struct timeval now;
    struct timespec timeout;
    gettimeofday(&now,NULL);
    timeout.tv_sec = now.tv_sec + 3;
    timeout.tv_nsec = now.tv_usec * 1000;

    if (bEnable&&(isPlaying()==false)) {
        pthread_mutex_lock(&mP2WMutex);
        m_pInputFile = fopen(m_strInputFileName,"rb");
        if(m_pInputFile == NULL) {
            m_pInputFile = fopen("/mnt/sdcard2/test.wav","rb");
            if (m_pInputFile==NULL) {
            SXLOGD("open input file fail!!");
            pthread_mutex_unlock(&mP2WMutex);
            return BAD_VALUE;
          }
        }
        m_bWBMode = bWB;
        // do basic setting to modem side
        mSideTone = -1 * degradegain<<1;
		
        m_pHW->SwitchAudioClock(true);  // Enable the audio power
        m_pLad->LAD_SetInputSource(LADIN_Microphone1);
        switch(mMode) {
          case SPH_MODE_NORMAL:
          {
              m_pLad->LAD_SetOutputDevice(LADOUT_SPEAKER1);
              
              m_pLad->LAD_SetSidetoneVolume(mSideTone);
              m_pLad->LAD_SetOutputVolume(mOutputVolume[mMode]);
              SXLOGD("speech mode: %d, receiver side tone: 0x%x, lad_Volume: 0x%x", mMode, mSideTone,mOutputVolume[mMode]);

              m_pLad->LAD_SetSpeechMode(SPH_MODE_NORMAL);
              break;
          }
          case SPH_MODE_EARPHONE:
          {
              m_pLad->LAD_SetOutputDevice(LADOUT_SPEAKER2);

              m_pLad->LAD_SetSidetoneVolume(mSideTone);
              m_pLad->LAD_SetOutputVolume(mOutputVolume[mMode]);
              SXLOGD("speech mode: %d, receiver side tone: 0x%x, lad_Volume: 0x%x", mMode, mSideTone,mOutputVolume[mMode]);

              m_pLad->LAD_SetSpeechMode(SPH_MODE_EARPHONE);
              break;
          }
          case SPH_MODE_LOUDSPK:
              m_pHW->ForceEnableSpeaker();
              m_pLad->LAD_SetOutputDevice(LADOUT_LOUDSPEAKER);

              m_pLad->LAD_SetSidetoneVolume(mSideTone);
              m_pLad->LAD_SetOutputVolume(mOutputVolume[mMode]);
              SXLOGD("speech mode: %d, receiver side tone: 0x%x, lad_Volume: 0x%x", mMode, mSideTone,mOutputVolume[mMode]);

              m_pLad->LAD_SetSpeechMode(SPH_MODE_LOUDSPK);
              break;
          default:
              break;
        }

        SXLOGD("open taste_threadloop thread~");
        pthread_mutex_lock(&mPPSMutex);
        ret = pthread_create(&mTasteThreadID, NULL,Play_PCM_With_SpeechEnhance_Routine,(void*)this);
        if(ret !=0)
        {
            SXLOGE("Play_PCM_With_SpeechEnhance_Routine thread pthread_create error!!");
            pthread_mutex_unlock(&mP2WMutex);
            return UNKNOWN_ERROR;
        }

        SXLOGD("+mPPSExit_Cond wait");
        ret = pthread_cond_timedwait(&mPPSExit_Cond, &mPPSMutex, &timeout);
        SXLOGD("-mPPSExit_Cond receive ret=%d",ret);
        pthread_mutex_unlock(&mPPSMutex);
        usleep(100*1000);

        // really enable the process
        if (mMode==SPH_MODE_LOUDSPK)
            m_pHW->ForceEnableSpeaker();
        m_bPlaying = true;
#if defined(MTK_SPH_EHN_CTRL_SUPPORT)
        SPH_ENH_INFO_T eSphEnhInfo;
        eSphEnhInfo.spe_usr_mask = 0xFFFF;
        eSphEnhInfo.spe_usr_subFunc_mask = 0xFFFFFFFF;
        m_pLad->LAD_Set_Speech_Enhancement_Info(eSphEnhInfo);
#endif		
        m_pLad->LAD_PCM2WayOn(bWB); // start PCM 2 way
#if defined(MTK_SPH_EHN_CTRL_SUPPORT)
        m_pLad->LAD_Set_Speech_Enhancement_Enable(true);
#endif
        pthread_mutex_unlock(&mP2WMutex);
    }else if((!bEnable)&&m_bPlaying){
        pthread_mutex_lock(&mP2WMutex);
        pthread_mutex_lock(&mPPSMutex);
        if(!m_bPPSThreadExit) {
            m_bPPSThreadExit = true;
            SXLOGD("+mPPSExit_Cond wait");
            ret = pthread_cond_timedwait(&mPPSExit_Cond,&mPPSMutex, &timeout);
            SXLOGD("-mPPSExit_Cond receive ret=%d",ret);
        }
        pthread_mutex_unlock(&mPPSMutex);

        m_pLad->LAD_PCM2WayOff();
        usleep(200*1000);   //wait to make sure all message is processed
        if (mMode==SPH_MODE_LOUDSPK)
            m_pHW->ForceDisableSpeaker();

        m_bPlaying = false;
        m_pHW->SwitchAudioClock(false);  // Disable the audio power

        m_pHW->doOutputDeviceRouting();

        if (m_pInputFile) fclose(m_pInputFile); 
        m_pInputFile = NULL;
        
        pthread_mutex_unlock(&mP2WMutex);
    }else {
        SXLOGD("The Audio Taste Tool State is error, bEnable=%d, playing=%d", bEnable,m_bPlaying);
        return BAD_VALUE;
    }

    return NO_ERROR;
}

status_t AudioParamTuning::putDataToModem()
{
    SXLOGV("putDataToModem in +" );

    int OutputBufDataCount = 0;
    uint32 PCMdataToModemSize = (m_bWBMode==true)? PCM2WAY_BUF_SIZE*2 : PCM2WAY_BUF_SIZE;
    uint32 offset=0, len=0;
    int header_size = 0;
    int8* pA2M_BufPtr;
    int32 maxBufLen;

    pthread_mutex_lock(&mPlayBufMutex);

    // BG Sound use address 0~1408(BGS use 1408 bytes). PCM4WAY_play use address 1408~2048. (4WAY playback path: 320+320 bytes)
    pA2M_BufPtr = (int8 *)m_pLad->pCCCI->GetA2MShareBufAddress() + A2M_SHARED_BUFFER_OFFSET;
    maxBufLen = m_pLad->pCCCI->GetA2MShareBufLen();

    // check if the dataReq size is larger than modem/AP share buffer size
    if ( PCMdataToModemSize > maxBufLen ){
       SXLOGD("AudTaste_PutDataToModem PCMdataToModemSize=%d",PCMdataToModemSize );
       PCMdataToModemSize = maxBufLen;
    }

    // check the output buffer data count
    OutputBufDataCount = rb_getDataCount(&m_sPlayBuf);
    SXLOGV("AudTaste_PutDataToModem OutputBufDataCount=%d",OutputBufDataCount );

    // if output buffer's data is not enough, fill it with zero to PCMdataToModemSize (ex: 320 bytes)
    if ((OutputBufDataCount<(int)PCMdataToModemSize)&&(m_sPlayBuf.pBufBase!=NULL)) {
       rb_writeDataValue(&m_sPlayBuf, 0, PCMdataToModemSize - OutputBufDataCount);
       SXLOGD("AudTaste_PutDataToModem underflow OutBufSize:%d", OutputBufDataCount);
    }

    header_size = WriteShareBufHeader(pA2M_BufPtr, (int16)LADBUFID_PCM_FillSpk, (int16)PCMdataToModemSize, (int16)A2M_BUF_HEADER);

    // header size (6 bytes)
    pA2M_BufPtr += header_size;
    if (m_sPlayBuf.pBufBase==NULL) {
        memset(pA2M_BufPtr, 0, PCMdataToModemSize);
    }else {
        rb_copyToLinear(pA2M_BufPtr, &m_sPlayBuf, PCMdataToModemSize);
    }
    offset = A2M_SHARED_BUFFER_OFFSET;
    len = PCMdataToModemSize + header_size;  // PCMdataToModemSize=320. header_size=LAD_SHARE_HEADER_LEN(6)
    m_pLad->LAD_PCM2WayDataNotify(offset, len);

    SXLOGV("OutputBuf B:0x%x, R:%d, W:%d, L:%d\n",m_sPlayBuf.pBufBase, m_sPlayBuf.pRead-m_sPlayBuf.pBufBase, m_sPlayBuf.pWrite-m_sPlayBuf.pBufBase, m_sPlayBuf.bufLen);
    SXLOGV("pA2M_BufPtr B:0x%x, Offset:%d, L:%d\n",pA2M_BufPtr, offset, len);

    pthread_mutex_unlock(&mPlayBufMutex);
    return NO_ERROR;
}

FILE_FORMAT AudioParamTuning::playbackFileFormat()
{
    SXLOGD("playbackFileFormat - playback file name:%s", m_strInputFileName);
    FILE_FORMAT ret = UNSUPPORT_FORMAT;
    char *pFileSuffix = m_strInputFileName;

    strsep(&pFileSuffix, ".");
    if (pFileSuffix!=NULL) {
        if (strcmp(pFileSuffix,"pcm")==0 || strcmp(pFileSuffix,"PCM")==0) {
            SXLOGD("playbackFileFormat - playback file format is pcm");
            ret = PCM_FORMAT;
        }else if (strcmp(pFileSuffix,"wav")==0 || strcmp(pFileSuffix,"WAV")==0) {
            SXLOGD("playbackFileFormat - playback file format is wav");
            ret = WAVE_FORMAT;
        }else {
            SXLOGD("playbackFileFormat - playback file format is unsupport");
            ret = UNSUPPORT_FORMAT;
        }
    }
    
    return ret;
}

#ifdef MTK_DUAL_MIC_SUPPORT
// For DMNR Tuning	
status_t AudioParamTuning::setRecordFileName(const char *fileName)
{
    SXLOGD("setRecordFileName in+");
    pthread_mutex_lock(&mP2WMutex);
    if (fileName!=NULL && strlen(fileName)<FILE_NAME_LEN_MAX-1) {
        SXLOGD("input file name:%s", fileName);
        memset(m_strOutFileName, 0, FILE_NAME_LEN_MAX);
        strcpy(m_strOutFileName,fileName);
    }else {
        SXLOGE("input file name NULL or too long!");
        pthread_mutex_unlock(&mP2WMutex);
        return BAD_VALUE;
    }

    pthread_mutex_unlock(&mP2WMutex);	
    return NO_ERROR;
}

status_t AudioParamTuning::setDMNRGain(unsigned short type, unsigned short value)
{
    SXLOGD("setDMNRGain: type=%d, gain=%d", type, value);
    status_t ret = NO_ERROR;

    if (value<0)
        return BAD_VALUE;

    pthread_mutex_lock(&mP2WMutex);
    switch(type) {
      case AUD_MIC_GAIN:
          mDualMicTool_micGain = (value > UPLINK_GAIN_MAX) ? UPLINK_GAIN_MAX : value;
          break;
      case AUD_RECEIVER_GAIN:
          mDualMicTool_receiverGain = (value > MAX_VOICE_VOLUME) ? MAX_VOICE_VOLUME : value;
          break;
      case AUD_HS_GAIN:
          mDualMicTool_headsetGain = (value > MAX_VOICE_VOLUME) ? MAX_VOICE_VOLUME : value;
          break;
      default:
          SXLOGW("setDMNRGain unknown type");
          ret = BAD_VALUE;
          break;
    }
    pthread_mutex_unlock(&mP2WMutex);
    return ret;
}

status_t AudioParamTuning::getDMNRGain(unsigned short type, unsigned short *value)
{
    SXLOGD("getDMNRGain: type=%d", type);
    status_t ret = NO_ERROR;

    pthread_mutex_lock(&mP2WMutex);
    switch(type) {
      case AUD_MIC_GAIN:
          *value = mDualMicTool_micGain;  
          break;
      case AUD_RECEIVER_GAIN:
          *value = mDualMicTool_receiverGain;
          break;
      case AUD_HS_GAIN:
          *value = mDualMicTool_headsetGain;
          break;
      default:
          SXLOGW("getDMNRGain unknown type");
          ret = BAD_VALUE;
          break;
    }
    pthread_mutex_unlock(&mP2WMutex);
    return ret;
}

status_t AudioParamTuning::enableDMNRModem2Way(bool bEnable, bool bWBMode, unsigned short outputDevice, unsigned short workMode)
{
    SXLOGD("enableDMNRModem2Way bEnable:%d, wb mode:%d, work mode:%d", bEnable, bWBMode, workMode);

    // 3 sec for timeout
    struct timeval now;
    struct timespec timeout;
    gettimeofday(&now,NULL);
    timeout.tv_sec = now.tv_sec + 3;
    timeout.tv_nsec = now.tv_usec * 1000;
    int ret;

    if(bEnable&&(isPlaying()==false)) {
        pthread_mutex_lock(&mP2WMutex);
        // open output file
        if (!workMode) {
            m_pInputFile = fopen(m_strInputFileName,"rb");
            SXLOGD("[Dual-Mic] open input file filename:%s", m_strInputFileName);
            if(m_pInputFile == NULL) {
                SXLOGW("[Dual-Mic] open input file fail!!");
                pthread_mutex_unlock(&mP2WMutex);
                return BAD_VALUE;
            }
            FILE_FORMAT fileType = playbackFileFormat();
            char waveHeader[WAV_HEADER_SIZE];
            if (fileType==WAVE_FORMAT) {
                fread(waveHeader, sizeof(char), WAV_HEADER_SIZE, m_pInputFile);
            }else if(fileType==UNSUPPORT_FORMAT) {
                SXLOGW("[Dual-Mic] playback file format is not support");
                pthread_mutex_unlock(&mP2WMutex);
                return BAD_VALUE;
            }
        }

        m_pOutputFile = fopen(m_strOutFileName,"wb");
        SXLOGD("[Dual-Mic] open output file filename:%s", m_strOutFileName);
        if(m_pOutputFile == NULL) {
            SXLOGW("[Dual-Mic] open output file fail!!");
            fclose(m_pInputFile);
            pthread_mutex_unlock(&mP2WMutex);
            return BAD_VALUE;
        }

        // do basic setting to modem side
        uint32 lad_level = 0;
        m_bWBMode = bWBMode;
        m_pHW->SwitchAudioClock(true);  // Enable the audio power
        m_pLad->LAD_SetInputSource(LADIN_DualAnalogMic);
        lad_level = 0xFFFFFFFF - (UPLINK_GAIN_MAX - mDualMicTool_micGain)/2;
        m_pLad->LAD_SetMicrophoneVolume(lad_level);  //set mic gain in dB
        SXLOGD("Play+Rec set dual mic, mic gain: 0x%x",lad_level);

        //set output and output gain in dB
        if (outputDevice==P2W_RECEIVER_OUT) {
            m_pLad->LAD_SetOutputDevice(LADOUT_SPEAKER1);
            m_pLad->LAD_SetSidetoneVolume(mSideTone); 
            lad_level = 0xFFFFFFFF - (MAX_VOICE_VOLUME - mDualMicTool_receiverGain)/2;                                                                                                      // in 0.5dB
            m_pLad->LAD_SetOutputVolume(lad_level);
            SXLOGD("Play+Rec set dual mic, receiver gain: 0x%x",lad_level);
        }else {
            m_pLad->LAD_SetOutputDevice(LADOUT_SPEAKER2);
            m_pLad->LAD_SetSidetoneVolume(mSideTone);
            lad_level = 0xFFFFFFFF - (MAX_VOICE_VOLUME - mDualMicTool_headsetGain)/2; 
            m_pLad->LAD_SetOutputVolume(lad_level);
            //m_pHW->mAnaReg->SetAnaReg(AUDIO_CON1,mDualMicTool_headsetGain,0x3f); // left audio gain
            //m_pHW->mAnaReg->SetAnaReg(AUDIO_CON2,mDualMicTool_headsetGain,0x3f); // right audio gain
            SXLOGD("Play+Rec set dual mic, headset gain: 0x%x",lad_level);
        }

        // open buffer thread
        SXLOGD("open DMNR_Tuning_threadloop thread~");
        pthread_mutex_lock(&mDMNRMutex);
        ret = pthread_create(&mDMNRThreadID, NULL,DMNR_Play_Rec_Routine,(void*)this);
        if(ret !=0)
            SXLOGE("DMNR_threadloop pthread_create error!!");

        SXLOGD("+mDMNRExit_Cond wait");
        ret = pthread_cond_timedwait(&mDMNRExit_Cond, &mDMNRMutex, &timeout);
        SXLOGD("-mDMNRExit_Cond receive ret=%d",ret);
        pthread_mutex_unlock(&mDMNRMutex);

        m_bDMNRPlaying = true;
        usleep(100*1000);

        // really enable the process
        m_pLad->LAD_DualMic_2WayOn(bWBMode, workMode);
        pthread_mutex_unlock(&mP2WMutex);
    } else if (!bEnable&&m_bDMNRPlaying){ 
        pthread_mutex_lock(&mP2WMutex);
        //stop buffer thread
        SXLOGD("close DMNR_tuning_threadloop");
        pthread_mutex_lock(&mDMNRMutex);
        if(!m_bDMNRThreadExit) {
            m_bDMNRThreadExit = true;
            SXLOGD("+mDMNRExit_Cond wait");
            ret = pthread_cond_timedwait(&mDMNRExit_Cond,&mDMNRMutex, &timeout);
            SXLOGD("-mDMNRExit_Cond receive ret=%d",ret);
        }
        pthread_mutex_unlock(&mDMNRMutex);

        m_pLad->LAD_DualMic_2WayOff();
        usleep(200*1000);                     //wait to make sure all message is processed

        m_bDMNRPlaying = false;
        m_pHW->SwitchAudioClock(false);  // Disable the audio power
        m_pHW->doOutputDeviceRouting();
        m_pHW->doInputDeviceRouting();

        if (m_pInputFile) fclose(m_pInputFile);
        if (m_pOutputFile) fclose(m_pOutputFile);
        m_pInputFile = NULL;
        m_pOutputFile = NULL;
        pthread_mutex_unlock(&mP2WMutex);
    }else {
        SXLOGD("The DMNR Tuning State is error, bEnable=%d, playing=%d", bEnable,m_bPlaying);
        return BAD_VALUE;
    }

    return NO_ERROR;
}

status_t AudioParamTuning::readDataFromModem(uint32 offset, uint32 len)
{
    SXLOGV("readDataFromModem in +" );
    int recBufFreeCnt = 0;

    int8* pM2A_BufPtr = (int8*)m_pLad->pCCCI->GetM2AShareBufAddress();
    uint32 m2aBufLen  = m_pLad->pCCCI->GetM2AShareBufLen();
    int8 *pBufRead = pM2A_BufPtr + offset + LAD_SHARE_HEADER_LEN;
    int pcmDataCnt = len - LAD_SHARE_HEADER_LEN;
    SXLOGD("readDataFromModem sharebuffer ptr=0x%x, bufferLen=%d, offset=%d, datacnt=%d", pM2A_BufPtr, m2aBufLen, offset, len);

	if ((offset+len)>m2aBufLen) {
        SXLOGW("readDataFromModem, no data... offset=%d, M2ABufLen=%d, data len=%d", offset, m2aBufLen, len);
        return BAD_VALUE;
    }

    pthread_mutex_lock(&mRecBufMutex);
    recBufFreeCnt = m_sRecBuf.bufLen - rb_getDataCount(&m_sRecBuf) - 1;
    SXLOGD("readDataFromModem recBufFreeCount=%d",recBufFreeCnt );

    // if rec buffer's free space is not enough
    if (recBufFreeCnt>=pcmDataCnt) {
        SXLOGD("read pcm from share buffer, free count=%d, data count=%d", recBufFreeCnt, pcmDataCnt);
        if (m_sRecBuf.pBufBase)
            rb_copyFromLinear(&m_sRecBuf, pBufRead, pcmDataCnt);
    }else {
        SXLOGW("record buffer overflow, do not copy, free count=%d, data count=%d", recBufFreeCnt, pcmDataCnt);
    }
    pthread_mutex_unlock(&mRecBufMutex);

    //send back
    m_pLad->LAD_PCM2WayDataSendBack();
    return NO_ERROR;
}
#endif
};
