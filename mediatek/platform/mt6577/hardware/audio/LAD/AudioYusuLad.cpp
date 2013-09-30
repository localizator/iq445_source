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
 * AudioYusuLad.cpp
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
 * $Revision: #13 $
 * $Modtime:$
 * $Log:$
 * 
 * 08 15 2012 donglei.ji
 * [ALPS00337843] [Need Patch] [Volunteer Patch] MM Command Handler JB migration
 * MM Command Handler JB migration.
 *
 * 04 23 2012 weiguo.li
 * [ALPS00273205] 
 * .
 *
 * 04 10 2012 weiguo.li
 * [ALPS00266592] [Need Patch] [Volunteer Patch] ICS_MP patchback to ALPS.ICS of Audio
 * .
 *
 *
 *******************************************************************************/

/*=============================================================================
 *                              Include Files
 *===========================================================================*/

#define LOG_TAG "AudioLAD"
#include <sys/resource.h>
#include <sys/time.h>
#include <fcntl.h>
#include <sys/types.h>
#include <errno.h>
#include "AudioYusuLad.h"
#include "CFG_AUDIO_File.h"
#include "AudioCustParam.h"
#include "AudioYusuHeadsetMessage.h"
#include "AudioYusuCcci.h"
#include "AudioYusuLadPlayer.h"
#include "audio_custom_exp.h"
#include <cutils/properties.h>
#if defined(MTK_DT_SUPPORT)
#include "AudioATCommand.h"
#endif

#if defined(MTK_SPH_EHN_CTRL_SUPPORT)
static const char PROPERTY_KEY_SPH_ENH_MASKS[] = "persist.af.sph_enh.masks";
#endif

#ifdef ENABLE_LOG_LAD
    #define LOG_LAD ALOGD
#else
    #define LOG_LAD ALOGV
#endif

namespace android {

/*=============================================================================
 *                             Public Function
 *===========================================================================*/
#if defined(MTK_DT_SUPPORT)
void *Read2ndMDVoiceRecordingThread(void *arg)
{
  LAD *pLAD = (LAD *)arg;
  AudioATCommand *pATCommand = (AudioATCommand*)pLAD->pATCmd;
  AudioStreamInInterface *inStream;
  uint16 vr_buf[320];
  int read_size = 0;
  while(1){
      read_size = pATCommand->Spc_ReadRNormalRecData(vr_buf,sizeof(vr_buf));

      //get pointer to input stream
      inStream = pLAD->mHw->mStreamHandler->mInput[0];
      if( inStream != NULL)
      {
    	  inStream->getVoiceRecordDataFromExtModem((uint8*)&vr_buf[0],read_size);
      }
      else
      {
         ALOGD(AUDMSG"Unable to send ExtMD voice rec data, inStream=NULL!!!\n");
      }
      if(pATCommand->Spc_GetParameters(SPC_GET_PARAMS_REC_STATE) == false){
          ALOGE(AUDMSG"Leave Read2ndMDVoiceRecordingThread bRecordState(%d)\n",pATCommand->Spc_GetParameters(SPC_GET_PARAMS_REC_STATE));
          break;
      }/*
      else{
          usleep(20*1000);
      }
      */
  }
  pthread_exit(NULL);
  return null;
}
#endif
LAD::LAD(AudioYusuHardware *hw)
{
   /* set hw pointer */
   mHw = hw;

   //set TTY state in phone call
   mTty_used_in_phone_call = false;

   // modem side Speech enhancement enable. Default=on
   mSph_Enh_enable = true;
   m_SideToneGain = 0;
   m_MicGain = 0;
   m_InputSrc = (int)0;

#if defined(MTK_SPH_EHN_CTRL_SUPPORT)
   LoadSphEnhMasks();
#endif

   /* create HeadsetMessage*/
   mHeadSetMessager = new AudioYusuHeadSetMessager(this);
   LOG_LAD("AudioYusuHeadSetMessager init  finish");
   /* create CCCI */
   pCCCI = new AudioCCCI(hw);

#if defined(MTK_DT_SUPPORT)
   /* create AudioATCommand */
   pATCmd = new AudioATCommand(hw);
#endif

   pthread_mutex_init(&mSendSphParaProcedureMutex, NULL);
}


//This function should be called immediately after the creation of LAD
bool LAD::LAD_Initial()
{
    LOG_LAD("LAD_Initial start");

    int ret;

    // initialize CCCI
    ret = pCCCI->Initial();
    if (ret == false) {
        ALOGD("LAD Initial CCCI fail");
        return ret;
    }

    LOG_LAD("LAD_Initial success");
    return true;
}

bool LAD::LAD_SendSphParaProcedure()
{
    pthread_mutex_lock_timeout_np(&mSendSphParaProcedureMutex, 10000); // wait 10 sec

    bool ret = false;

    // Wait until modem ready
    pCCCI->WaitUntilModemReady();

    // AUDIO_CUSTOM_PARAMETER
    AUDIO_CUSTOM_PARAM_STRUCT sndParam;
    GetCustParamFromNV(&sndParam);
    ret = LAD_SetSpeechParameters(&(sndParam));
    LOG_LAD("LAD_SetSpeechParameters ret=%d", ret);

#if defined(MTK_DUAL_MIC_SUPPORT)
    // AUDIO_DUAL_MIC_CUSTOM_PARAMETER
    AUDIO_CUSTOM_EXTRA_PARAM_STRUCT dualMicParam;
    Read_DualMic_CustomParam_From_NVRAM(&dualMicParam);
    ret = LAD_SetDualMicParameters(&(dualMicParam));
    LOG_LAD("LAD_SetDualMicParameters ret=%d", ret);
#endif

#if defined(MTK_WB_SPEECH_SUPPORT)
    // AUDIO_WB_CUSTOM_PARAMETER
    AUDIO_CUSTOM_WB_PARAM_STRUCT sndWbParam;
    GetCustWBParamFromNV(&sndWbParam);
    ret = LAD_SetWBSpeechParameters(&(sndWbParam));
    LOG_LAD("LAD_SetWBSpeechParameters ret=%d", ret);
#endif

#if defined(MTK_AUDIO_HD_REC_SUPPORT)
    LAD_SetMDCapability(1);
    // AUDIO_HD_RECORD_PARAMETER
    AUDIO_HD_RECORD_PARAM_STRUCT hdRecordParam;
    GetHdRecordParamFromNV(&hdRecordParam);
    ret = LAD_SetHdRecordParameters(&hdRecordParam);
    LOG_LAD("LAD_SetHdRecordParameters, ret=%d", ret);
    //LAD_GetMDCapability();
#endif

    // Set speech mode
    LAD_SetSpeechMode(SPH_MODE_NORMAL);

    // Set LoudSPK Mic mode
#if defined(USE_REFMIC_IN_LOUDSPK)
    LAD_Set_LoudSPK_RefMicMode(USE_REFMIC_IN_LOUDSPK);
#endif

#if defined(MTK_SPH_EHN_CTRL_SUPPORT)
    // Set speech enhancement parameters' mask to modem side
    SPH_ENH_INFO_T eSphEnhInfo = mSphEnhInfo;
    LAD_Set_Speech_Enhancement_Info(eSphEnhInfo);
#endif

    // [HFP 1.6] BT WB/NB follow mAfe_handle or default 8K. always short sync
    bool bDaiBtMode = (pCCCI->mHw != NULL) ? pCCCI->mHw->mAfe_handle->mDaiBtMode : 0;
    LAD_SetDaiBtCfg(bDaiBtMode, false);

    // Use lock to ensure the previous command with share buffer control is completed
    if (pCCCI->A2MBufLock() == false) {
        ALOGE(AUDMSG"LAD_SendSphParaProcedure fail to get A2M Buffer for ensuring share buffer control is completed");
    }
    else {
        pCCCI->A2MBufUnLock();
    }

    pthread_mutex_unlock(&mSendSphParaProcedureMutex);
    return ret;
}

//This function should be called immediately after the creation of LAD
bool LAD::LAD_Deinitial()
{
   bool ret;
   // deinitialize CCCI
   ret = pCCCI->Deinitial();
   if(ret == false) {
   	ALOGD("LAD_Deinitial fail");
   	return ret;
   }

   return true;
}

LAD::~LAD()
{
   delete(pCCCI);
}


/*---------------Recording Functions---------------------*/

/* open recording */
bool LAD::LAD_OpenNormalRecPath(int32 format, int32 sampleRate)
{
#if defined(MTK_DT_SUPPORT)
   int Flag_SecondModemPhoneCall = 0;
   int ret = 0,ret2;
   Flag_SecondModemPhoneCall = pCCCI->mHw->GetFlag_SecondModemPhoneCall();
   if(Flag_SecondModemPhoneCall == 1){
       ALOGD("2nd MD LAD_OpenNormalRecPath: format(%d)/SR(%d)\n", format, sampleRate);
       ret = pATCmd->Spc_OpenNormalRecPath(format, sampleRate);
       ret2 = pthread_create( &h2ndMD_ReadRecDataThread, NULL, Read2ndMDVoiceRecordingThread, (void*)this);
       if ( ret2 != 0 ){
           ALOGE(AUDMSG"Fail to Create 2ndMD_ReadRecDataThread!\n");
       }
       return ret;
   }
#endif
   pCCCI->CheckMDSideSphCoefArrival(); // must pass; otherwise, set wrong speech mode (only this time)
   pCCCI->mHw->SwitchAudioClock(true);
   //SendMsgInQueue( DataTransfer(M2A_Record_turn_on, (Mode<<0xc)));
   int data = (sampleRate << 12) | format;
   ALOGD(AUDMSG"LAD Record_On (0x%x), data=%x", A2M_Record_turn_on, data);
   return pCCCI->SendMessageInQueue( CCCI_MSG1(A2M_Record_turn_on, data));
}

bool LAD::LAD_CloseNormalRecPath()
{
#if defined(MTK_DT_SUPPORT)
   int Flag_SecondModemPhoneCall = 0;
   int ret = 0;
   Flag_SecondModemPhoneCall = pCCCI->mHw->GetFlag_SecondModemPhoneCall();
   if(Flag_SecondModemPhoneCall == 1){
       ALOGD("2nd MD LAD_CloseNormalRecPath");
       ret = pATCmd->Spc_CloseNormalRecPath();
       return ret;
   }
#endif
   m_MicGain = 0;
   m_InputSrc = (int)0;
   //SendMsgInQueue( CCCI_MSG1(M2A_Record_turn_off, 0));
   ALOGD(AUDMSG"LAD Record_Off (0x%x)", A2M_Record_turn_off);
   return pCCCI->SendMessageInQueue( CCCI_MSG1(A2M_Record_turn_off, 0));
}

bool LAD::LAD_NormalRecDataSendBack()
{
#if defined(MTK_DT_SUPPORT)
   int Flag_SecondModemPhoneCall = 0;
   Flag_SecondModemPhoneCall = pCCCI->mHw->GetFlag_SecondModemPhoneCall();
   if(Flag_SecondModemPhoneCall != 0){
      ALOGD("LAD_NormalRecDataSendBack: not support! Flag_SecondModemPhoneCall(%d)", Flag_SecondModemPhoneCall);
      return true;
   }
#endif
//   LOG_LAD(AUDMSG"LAD_NormalRecDataSendBack, f:%x",A2M_Record_DataSendBack);
   return pCCCI->SendMessageInQueue( CCCI_MSG1(A2M_Record_DataSendBack, 0));
}

bool LAD::LAD_SetRecDropFrameCount( int32 frame_count)
{
   int data = (frame_count<0)? 0: frame_count;
#if defined(MTK_DT_SUPPORT)
   int Flag_SecondModemPhoneCall = 0;
   Flag_SecondModemPhoneCall = pCCCI->mHw->GetFlag_SecondModemPhoneCall();
   if(Flag_SecondModemPhoneCall != 0){
      ALOGD("LAD_SetRecDropFrameCount: not support! Flag_SecondModemPhoneCall(%d)", Flag_SecondModemPhoneCall);
      return true;
   }
#endif
   ALOGD(AUDMSG"LAD LAD_SetRecDropFrameCount (0x%x), data=%x", A2M_Record_Drop_Frame, data);
   return pCCCI->SendMessageInQueue( CCCI_MSG1(A2M_Record_Drop_Frame, data));
}

/*-----------------PCM 2Way Functions---------------------*/

bool LAD::LAD_PCM2WayPlayOn()
{
#if defined(MTK_DT_SUPPORT)
   int Flag_SecondModemPhoneCall = 0;
   Flag_SecondModemPhoneCall = pCCCI->mHw->GetFlag_SecondModemPhoneCall();
   if(Flag_SecondModemPhoneCall != 0){
      ALOGD("LAD_PCM2WayPlayOn: not support! Flag_SecondModemPhoneCall(%d)", Flag_SecondModemPhoneCall);
      return true;
   }
#endif
   if( mPCM2WayState == 0 ) {
      //nothing is on, just turn it on
      mPCM2WayState |= PCM2WAY_PLAY_ON;
      LOG_LAD(AUDMSG"LAD_PCM2WayPlayOn, A2M_PCM_OPEN1");
      return pCCCI->SendMessageInQueue(CCCI_MSG1(A2M_PCM_OPEN, mPCM2WayState));
   }

   else if ( mPCM2WayState == PCM2WAY_PLAY_ON  )  {
      // only play on, return trun;
      return true;
   }

   else if ( mPCM2WayState == PCM2WAY_REC_ON  )  {
      // only rec is on, turn off, modify state and turn on again
      bool ret;
      mPCM2WayState |= PCM2WAY_PLAY_ON;
      LOG_LAD(AUDMSG"LAD_PCM2WayPlayOn, A2M_PCM_Close");
      ret = pCCCI->SendMessageInQueue(CCCI_MSG1(A2M_PCM_Close, 0));
      LOG_LAD(AUDMSG"LAD_PCM2WayPlayOn, A2M_PCM_OPEN2");
      ret &= pCCCI->SendMessageInQueue(CCCI_MSG1(A2M_PCM_OPEN, mPCM2WayState));
      return ret;
   }

   else if ( mPCM2WayState == (PCM2WAY_REC_ON | PCM2WAY_PLAY_ON) ) {
      // both on, return true
      return true;
   }

   else {
      return false;
   }
}

bool LAD::LAD_PCM2WayPlayOff()
{
#if defined(MTK_DT_SUPPORT)
   int Flag_SecondModemPhoneCall = 0;
   Flag_SecondModemPhoneCall = pCCCI->mHw->GetFlag_SecondModemPhoneCall();
   if(Flag_SecondModemPhoneCall != 0){
      ALOGD("LAD_PCM2WayPlayOff: not support! Flag_SecondModemPhoneCall(%d)", Flag_SecondModemPhoneCall);
      return true;
   }
#endif
   if( mPCM2WayState == 0 ) {
      //nothing is on, return true
      return true;
   }

   else if ( mPCM2WayState == PCM2WAY_PLAY_ON  )  {
      // only play on, return trun;
      LOG_LAD(AUDMSG"LAD_PCM2WayPlayOff, A2M_PCM_Close1");
      return pCCCI->SendMessageInQueue(CCCI_MSG1(A2M_PCM_Close, 0));
   }

   else if ( mPCM2WayState == PCM2WAY_REC_ON  )  {
      // only rec on, return trun;
      return true;
   }

   else if ( mPCM2WayState == (PCM2WAY_REC_ON | PCM2WAY_PLAY_ON) ) {
      // both rec and play on, turn off, modify state and turn on again
      bool ret;
      mPCM2WayState &= (~PCM2WAY_PLAY_ON);
      LOG_LAD(AUDMSG"LAD_PCM2WayPlayOff, A2M_PCM_Close2");
      ret = pCCCI->SendMessageInQueue(CCCI_MSG1(A2M_PCM_Close, 0));
      LOG_LAD(AUDMSG"LAD_PCM2WayPlayOff, A2M_PCM_OPEN");
      ret &= pCCCI->SendMessageInQueue(CCCI_MSG1(A2M_PCM_OPEN, mPCM2WayState));
      return ret;
   }
   else {
      return false;
   }
}



bool LAD::LAD_PCM2WayRecordOn()
{
#if defined(MTK_DT_SUPPORT)
   int Flag_SecondModemPhoneCall = 0;
   Flag_SecondModemPhoneCall = pCCCI->mHw->GetFlag_SecondModemPhoneCall();
   if(Flag_SecondModemPhoneCall != 0){
      ALOGD("LAD_PCM2WayRecordOn: not support! Flag_SecondModemPhoneCall(%d)", Flag_SecondModemPhoneCall);
      return true;
   }
#endif
   if( mPCM2WayState == 0 ) {
      //nothing is on, just turn it on
      mPCM2WayState |= PCM2WAY_REC_ON;
      LOG_LAD(AUDMSG"LAD_PCM2WayRecordOn, A2M_PCM_OPEN1");
      return pCCCI->SendMessageInQueue(CCCI_MSG1(A2M_PCM_OPEN, mPCM2WayState));
   }

   else if ( mPCM2WayState == PCM2WAY_PLAY_ON  )  {
      // only play is on, turn off, modify state and turn on again
      bool ret;
      mPCM2WayState |= PCM2WAY_REC_ON;
      LOG_LAD(AUDMSG"LAD_PCM2WayRecordOn, A2M_PCM_Close");
      ret = pCCCI->SendMessageInQueue(CCCI_MSG1(A2M_PCM_Close, 0));
      LOG_LAD(AUDMSG"LAD_PCM2WayRecordOn, A2M_PCM_OPEN2");
      ret &= pCCCI->SendMessageInQueue(CCCI_MSG1(A2M_PCM_OPEN, mPCM2WayState));
      return ret;
   }

   else if ( mPCM2WayState == PCM2WAY_REC_ON  )  {
     // only rec on, return true;
      return true;
   }

   else if ( mPCM2WayState == (PCM2WAY_REC_ON | PCM2WAY_PLAY_ON) ) {
      return true;
   }

   else {
      return false;
   }
}

bool LAD::LAD_PCM2WayRecordOff()
{
#if defined(MTK_DT_SUPPORT)
   int Flag_SecondModemPhoneCall = 0;
   Flag_SecondModemPhoneCall = pCCCI->mHw->GetFlag_SecondModemPhoneCall();
   if(Flag_SecondModemPhoneCall != 0){
      ALOGD("LAD_PCM2WayRecordOff: not support! Flag_SecondModemPhoneCall(%d)", Flag_SecondModemPhoneCall);
      return true;
   }
#endif
   if( mPCM2WayState == 0 ) {
      //nothing is on, return true
      return true;
   }

   else if ( mPCM2WayState == PCM2WAY_PLAY_ON  )  {
      // only play on, return trun;
      return true;
   }

   else if ( mPCM2WayState == PCM2WAY_REC_ON  )  {
      // already rec on, return trun;
      LOG_LAD(AUDMSG"LAD_PCM2WayRecordOff, A2M_PCM_Close1");
      return pCCCI->SendMessageInQueue(CCCI_MSG1(A2M_PCM_Close, 0));
   }

   else if ( mPCM2WayState == (PCM2WAY_REC_ON | PCM2WAY_PLAY_ON) ) {
      // both rec and play on, turn off, modify state and turn on again
      bool ret;
      mPCM2WayState &= (~PCM2WAY_REC_ON);
      LOG_LAD(AUDMSG"LAD_PCM2WayRecordOff, A2M_PCM_Close2");
      ret = pCCCI->SendMessageInQueue(CCCI_MSG1(A2M_PCM_Close, 0));
      LOG_LAD(AUDMSG"LAD_PCM2WayRecordOff, A2M_PCM_OPEN");
      ret &= pCCCI->SendMessageInQueue(CCCI_MSG1(A2M_PCM_OPEN, mPCM2WayState));
      return ret;
   }
   else {
      return false;
   }
}


bool LAD::LAD_PCM2WayOn(bool bWB)
{
#if defined(MTK_DT_SUPPORT)
   int Flag_SecondModemPhoneCall = 0;
   Flag_SecondModemPhoneCall = pCCCI->mHw->GetFlag_SecondModemPhoneCall();
   if(Flag_SecondModemPhoneCall != 0){
      ALOGD("LAD_PCM2WayOn: not support! Flag_SecondModemPhoneCall(%d)", Flag_SecondModemPhoneCall);
      return true;
   }
#endif
   mPCM2WayState = (PCM2WAY_PLAY_ON | PCM2WAY_REC_ON);
#if defined(MTK_WB_SPEECH_SUPPORT)
   mPCM2WayState = bWB ? (mPCM2WayState | PCM2WAY_WB_PLAY_ON):(mPCM2WayState);
#endif
   LOG_LAD(AUDMSG"LAD PCM2WayOn (0x%x), mPCM2WayState:%d",A2M_PCM_OPEN,mPCM2WayState);
   return pCCCI->SendMessageInQueue(CCCI_MSG1(A2M_PCM_OPEN, mPCM2WayState));
}

bool LAD::LAD_PCM2WayOff()
{
#if defined(MTK_DT_SUPPORT)
   int Flag_SecondModemPhoneCall = 0;
   Flag_SecondModemPhoneCall = pCCCI->mHw->GetFlag_SecondModemPhoneCall();
   if(Flag_SecondModemPhoneCall != 0){
      ALOGD("LAD_PCM2WayOff: not support! Flag_SecondModemPhoneCall(%d)", Flag_SecondModemPhoneCall);
      return true;
   }
#endif
   mPCM2WayState = 0;
   LOG_LAD(AUDMSG"LAD PCM2WayOff (0x%x), mPCM2WayState:%d",A2M_PCM_Close,mPCM2WayState);
   return pCCCI->SendMessageInQueue(CCCI_MSG1(A2M_PCM_Close, 0));
}


bool LAD::LAD_PCM2WayDataSendBack()
{
#if defined(MTK_DT_SUPPORT)
   int Flag_SecondModemPhoneCall = 0;
   Flag_SecondModemPhoneCall = pCCCI->mHw->GetFlag_SecondModemPhoneCall();
   if(Flag_SecondModemPhoneCall != 0){
      ALOGD("LAD_PCM2WayDataSendBack: not support! Flag_SecondModemPhoneCall(%d)", Flag_SecondModemPhoneCall);
      return true;
   }
#endif
//   LOG_LAD("LAD_PCM2WayDataSendBack (Mic), f:0x%x",A2M_PCM_DataSendBack);
   return pCCCI->SendMessageInQueue( CCCI_MSG1(A2M_PCM_DataSendBack, 0));
}


bool LAD::LAD_PCM2WayDataNotify(uint32 offset, uint32 len)
{
#if defined(MTK_DT_SUPPORT)
   int Flag_SecondModemPhoneCall = 0;
   Flag_SecondModemPhoneCall = pCCCI->mHw->GetFlag_SecondModemPhoneCall();
   if(Flag_SecondModemPhoneCall != 0){
      ALOGD("LAD_PCM2WayDataNotify: not support! Flag_SecondModemPhoneCall(%d)", Flag_SecondModemPhoneCall);
      return true;
   }
#endif
//   LOG_LAD("LAD_PCM2WayDataNotify (Spk)");
   return pCCCI->SendMessageInQueue(CCCI_MSG2(A2M_PCM_DataNotify, offset, len));
}

#if defined(MTK_DUAL_MIC_SUPPORT)||defined(MTK_AUDIO_HD_REC_SUPPORT)
// ToDo: for dual mic
bool LAD::LAD_DualMic_2WayOn(int WBMode, int PlayRec)
{
#if defined(MTK_DT_SUPPORT)
   int Flag_SecondModemPhoneCall = 0;
   Flag_SecondModemPhoneCall = pCCCI->mHw->GetFlag_SecondModemPhoneCall();
   if(Flag_SecondModemPhoneCall != 0){
      ALOGD("LAD_DualMic_2WayOn: not support! Flag_SecondModemPhoneCall(%d)", Flag_SecondModemPhoneCall);
      return true;
   }
#endif
   //mPCM2WayState = (PCM2WAY_PLAY_ON | PCM2WAY_REC_ON);
   LOG_LAD("LAD_DualMic_2WayOn (0x%x), type:%d, wbmode:%d, PlayRec:%d",A2M_PCM_OPEN, P2W_FORMAT_CAL, WBMode, PlayRec);

   //prevent 'on' for second time cause problem
   if (mDualMic2WayState) {
      ALOGW("LAD_DualMic_2WayOn when record is ongoing: %d, ignore", mDualMic2WayState);
      return TRUE;
   }
   if (PlayRec == DMMETA_2WAY_NORMAL) {
      mDualMic2WayState = DUALMIC2WAY_PLAYREC;
      if (WBMode == FALSE) {
         return pCCCI->SendMessageInQueue(CCCI_MSG1(A2M_DualRecPlay_turn_on, P2W_FORMAT_CAL));
      }
      else {
         return pCCCI->SendMessageInQueue(CCCI_MSG1(A2M_DualRecPlay_turn_on, P2W_FORMAT_WB_CAL));
      }
   }
   else {
      mDualMic2WayState = DUALMIC2WAY_REC_ONLY;
      if (WBMode == FALSE) {
         return pCCCI->SendMessageInQueue(CCCI_MSG1(A2M_DualRecOnly_turn_on, P2W_FORMAT_CAL));
      }
      else {
         return pCCCI->SendMessageInQueue(CCCI_MSG1(A2M_DualRecOnly_turn_on, P2W_FORMAT_WB_CAL));
      }
   }
}
bool LAD::LAD_DualMic_2WayOff()
{
#if defined(MTK_DT_SUPPORT)
   int Flag_SecondModemPhoneCall = 0;
   Flag_SecondModemPhoneCall = pCCCI->mHw->GetFlag_SecondModemPhoneCall();
   if(Flag_SecondModemPhoneCall != 0){
      ALOGD("LAD_DualMic_2WayOff: not support! Flag_SecondModemPhoneCall(%d)", Flag_SecondModemPhoneCall);
      return true;
   }
#endif
    LOG_LAD("LAD_DualMic_2WayOff, (0x%x) type:%d",A2M_PCM_Close,0);
    if (mDualMic2WayState == DUALMIC2WAY_STOP) {
       ALOGW("LAD_DualMic_2WayOff when record is off: %d, ignore", mDualMic2WayState);
       return TRUE;
    }

    mDualMic2WayState = DUALMIC2WAY_STOP;
    if ( mDualMic2WayState == DUALMIC2WAY_PLAYREC ) {
       return pCCCI->SendMessageInQueue(CCCI_MSG1(A2M_DualRecPlay_turn_off, 0));
    }
    else {
       return pCCCI->SendMessageInQueue(CCCI_MSG1(A2M_DualRecOnly_turn_off, 0));
    }
}
#endif

/*-----------------Background Sound Functions---------------------*/

bool LAD::LAD_BGSoundOn()
{
#if defined(MTK_DT_SUPPORT)
   int Flag_SecondModemPhoneCall = 0;
   Flag_SecondModemPhoneCall = pCCCI->mHw->GetFlag_SecondModemPhoneCall();
   if(Flag_SecondModemPhoneCall != 0){
      ALOGD("LAD_BGSoundOn: not support! Flag_SecondModemPhoneCall(%d)", Flag_SecondModemPhoneCall);
      return true;
   }
#endif
   pCCCI->mHw->SwitchAudioClock(true);
   ALOGD(AUDMSG"LAD BGSoundOn, A2M_BGSND_Open(0x%x)",A2M_BGSND_Open);
   return pCCCI->SendMessageInQueue(CCCI_MSG1(A2M_BGSND_Open, 0));
}

bool LAD::LAD_BGSoundOff()
{
#if defined(MTK_DT_SUPPORT)
   int Flag_SecondModemPhoneCall = 0;
   Flag_SecondModemPhoneCall = pCCCI->mHw->GetFlag_SecondModemPhoneCall();
   if(Flag_SecondModemPhoneCall != 0){
      ALOGD("LAD_BGSoundOff: not support! Flag_SecondModemPhoneCall(%d)", Flag_SecondModemPhoneCall);
      return true;
   }
#endif
   bool ret;
   ALOGD(AUDMSG"LAD BGSoundOff, A2M_BGSND_Close(0x%x)",A2M_BGSND_Close);
   ret = pCCCI->SendMessageInQueue(CCCI_MSG1(A2M_BGSND_Close, 0));
   return ret;
}

bool LAD::LAD_BGSoundDataNotify(uint32 offset, uint32 len)
{
#if defined(MTK_DT_SUPPORT)
   int Flag_SecondModemPhoneCall = 0;
   Flag_SecondModemPhoneCall = pCCCI->mHw->GetFlag_SecondModemPhoneCall();
   if(Flag_SecondModemPhoneCall != 0){
      ALOGD("LAD_BGSoundDataNotify: not support! Flag_SecondModemPhoneCall(%d)", Flag_SecondModemPhoneCall);
      return true;
   }
#endif
//   LOG_LAD("LAD_BGSoundDataNotify, A2M_BGSND_DataNotify offset:%d, len:%d",offset,len);
   return pCCCI->SendMessageInQueue(CCCI_MSG2(A2M_BGSND_DataNotify, offset, len));
}

bool LAD::LAD_BGSoundConfig(uint32 UL_gain, uint32 DL_gain)
{
#if defined(MTK_DT_SUPPORT)
   int Flag_SecondModemPhoneCall = 0;
   Flag_SecondModemPhoneCall = pCCCI->mHw->GetFlag_SecondModemPhoneCall();
   if(Flag_SecondModemPhoneCall != 0){
      ALOGD("LAD_BGSoundConfig: not support! Flag_SecondModemPhoneCall(%d)", Flag_SecondModemPhoneCall);
      return true;
   }
#endif
   LOG_LAD(AUDMSG"LAD_BGSoundConfig, A2M_BGSND_Config ul:%x, dl:%x",UL_gain,DL_gain);
   return pCCCI->SendMessageInQueue(CCCI_MSG3(A2M_BGSND_Config, UL_gain, DL_gain));
}


/*-----------------Playback Functions---------------------*/

bool LAD::LAD_PlaybackTurnOn(int32 data_type)
{
#if defined(MTK_DT_SUPPORT)
   int Flag_SecondModemPhoneCall = 0;
   Flag_SecondModemPhoneCall = pCCCI->mHw->GetFlag_SecondModemPhoneCall();
   if(Flag_SecondModemPhoneCall != 0){
      ALOGD("LAD_PlaybackTurnOn: not support! Flag_SecondModemPhoneCall(%d)", Flag_SecondModemPhoneCall);
      return true;
   }
#endif
   LOG_LAD(AUDMSG"LAD_PlaybackTurnOn, A2M_playback_turn_on");
   return pCCCI->SendMessageInQueue(CCCI_MSG1(A2M_playback_turn_on, (int32)data_type));
}

bool LAD::LAD_PlaybackTurnOff()
{
#if defined(MTK_DT_SUPPORT)
   int Flag_SecondModemPhoneCall = 0;
   Flag_SecondModemPhoneCall = pCCCI->mHw->GetFlag_SecondModemPhoneCall();
   if(Flag_SecondModemPhoneCall != 0){
      ALOGD("LAD_PlaybackTurnOff: not support! Flag_SecondModemPhoneCall(%d)", Flag_SecondModemPhoneCall);
      return true;
   }
#endif
   LOG_LAD(AUDMSG"LAD_PlaybackTurnOff, A2M_playback_turn_off");
   return pCCCI->SendMessageInQueue(CCCI_MSG1(A2M_playback_turn_off, 0));
}

bool LAD::LAD_PlaybackDataNotify(uint32 offset, uint32 len)
{
#if defined(MTK_DT_SUPPORT)
   int Flag_SecondModemPhoneCall = 0;
   Flag_SecondModemPhoneCall = pCCCI->mHw->GetFlag_SecondModemPhoneCall();
   if(Flag_SecondModemPhoneCall != 0){
      ALOGD("LAD_PlaybackDataNotify: not support! Flag_SecondModemPhoneCall(%d)", Flag_SecondModemPhoneCall);
      return true;
   }
#endif
   LOG_LAD(AUDMSG"LAD_PlaybackDataNotify, A2M_Playback_DataNotify");
   return pCCCI->SendMessageInQueue(CCCI_MSG2(A2M_Playback_DataNotify, offset, len));
}


/*------------------Volume Control -----------------------*/

bool LAD::LAD_SetOutputVolume(uint32 Gain)
{
#if defined(MTK_DT_SUPPORT)
   int    Flag_SecondModemPhoneCall = 0;
   Flag_SecondModemPhoneCall = pCCCI->mHw->GetFlag_SecondModemPhoneCall();
   if(Flag_SecondModemPhoneCall != 0){
       return pATCmd->Spc_SetOutputVolume(Gain);
   }
#endif
   ALOGD(AUDMSG"LAD SetOutputVolume (0x%x), gain:%x",A2M_SetOutputVolume,Gain);
   return pCCCI->SendMessageInQueue( CCCI_MSG1(A2M_SetOutputVolume, Gain) );
}

bool LAD::LAD_SetGainTableOutputVolume(int32 Gain)
{
#if defined(MTK_DT_SUPPORT)
    int Flag_SecondModemPhoneCall = 0;
    Flag_SecondModemPhoneCall = pCCCI->mHw->GetFlag_SecondModemPhoneCall();
    if(Flag_SecondModemPhoneCall != 0){
        ALOGD("LAD_SetGainTableOutputVolume: not support! Flag_SecondModemPhoneCall(%d)", Flag_SecondModemPhoneCall);
        return true;
    }
#endif
    ALOGD(AUDMSG"LAD_SetGainTableOutputVolume (0x%x), gain:%x",A2M_SetGainTableVolume,Gain);
    return pCCCI->SendMessageInQueue( CCCI_MSG1(A2M_SetGainTableVolume, Gain) );
}

bool LAD::LAD_SetGainTableMicVolume(int32 Gain)
{
#if defined(MTK_DT_SUPPORT)
    int Flag_SecondModemPhoneCall = 0;
    Flag_SecondModemPhoneCall = pCCCI->mHw->GetFlag_SecondModemPhoneCall();
    if(Flag_SecondModemPhoneCall != 0){
        ALOGD("LAD_SetGainTableMicVolume: not support! Flag_SecondModemPhoneCall(%d)", Flag_SecondModemPhoneCall);
        return true;
    }
#endif
    ALOGD(AUDMSG"LAD_SetGainTableMicVolume (0x%x), gain:%x",A2M_SetGainTableMicVolume,Gain);
    return pCCCI->SendMessageInQueue( CCCI_MSG1(A2M_SetGainTableMicVolume, Gain) );
}

bool LAD::LAD_SetMicrophoneVolume(uint32 Gain)
{
#if defined(MTK_DT_SUPPORT)
    int Flag_SecondModemPhoneCall = 0;
    Flag_SecondModemPhoneCall = pCCCI->mHw->GetFlag_SecondModemPhoneCall();
    if(Flag_SecondModemPhoneCall != 0){
        ALOGD("2nd MD LAD SetMicrophoneVolume (0x%x), gain:%x",A2M_SetMicrophoneVolume,Gain);
        return pATCmd->Spc_SetMicrophoneVolume(Gain);
    }
#endif
   if(m_MicGain == Gain)
      return true;

   ALOGD(AUDMSG"LAD SetMicrophoneVolume (0x%x), gain:%x",A2M_SetMicrophoneVolume,Gain);
   m_MicGain = Gain;
   return pCCCI->SendMessageInQueue( CCCI_MSG1(A2M_SetMicrophoneVolume, Gain) );
}


bool LAD::LAD_MuteMicrophone(bool ENABLE)
{
#if defined(MTK_DT_SUPPORT)
   int Flag_SecondModemPhoneCall = 0;
   Flag_SecondModemPhoneCall = pCCCI->mHw->GetFlag_SecondModemPhoneCall();
   if(Flag_SecondModemPhoneCall != 0){
       ALOGD("2nd LAD_MuteMicrophone (0x%x), en:%x",A2M_MuteMicrophone,ENABLE);
       return pATCmd->Spc_MuteMicrophone(ENABLE);
   }
#endif
   ALOGD(AUDMSG"LAD_MuteMicrophone (0x%x), en:%x",A2M_MuteMicrophone,ENABLE);
   return pCCCI->SendMessageInQueue( CCCI_MSG1(A2M_MuteMicrophone, ENABLE) );
}


bool LAD::LAD_MuteSpeaker(bool ENABLE)
{
#if defined(MTK_DT_SUPPORT)
   int Flag_SecondModemPhoneCall = 0;
   Flag_SecondModemPhoneCall = pCCCI->mHw->GetFlag_SecondModemPhoneCall();
   if(Flag_SecondModemPhoneCall != 0){
      ALOGD("LAD_MuteSpeaker: not support! Flag_SecondModemPhoneCall(%d)", Flag_SecondModemPhoneCall);
      return true;
   }
#endif
   ALOGD(AUDMSG"LAD_MuteSpeaker (0x%x), en:%x",A2M_MuteSpeaker,ENABLE);
   return pCCCI->SendMessageInQueue( CCCI_MSG1(A2M_MuteSpeaker, ENABLE) );
}


bool LAD::LAD_SetSidetoneVolume(uint32 Gain)
{
#if defined(MTK_DT_SUPPORT)
   int Flag_SecondModemPhoneCall = 0;
   Flag_SecondModemPhoneCall = pCCCI->mHw->GetFlag_SecondModemPhoneCall();
   if(Flag_SecondModemPhoneCall != 0){
     ALOGD("2nd MD LAD_SetSidetoneVolume, gain:%x",Gain);
     pATCmd->Spc_SetSidetoneVolume(Gain);
     return true;
   }
#endif
   //ALOGD(AUDMSG"LAD_SetSidetoneVolume, gain:%x",Gain);
   if(m_SideToneGain == Gain)
      return true;

   ALOGD(AUDMSG"LAD_SetSidetoneVolume, gain:%x",Gain);
   m_SideToneGain = Gain;
   return pCCCI->SendMessageInQueue( CCCI_MSG1(A2M_SetSidetoneVolume, Gain) );
}

/*---------------------Device Control--------------------------*/

bool LAD::LAD_SetOutputDevice(LadOutPutDevice_Line HW_Mode)
{
#if defined(MTK_DT_SUPPORT)
   int Flag_SecondModemPhoneCall = 0;
   Flag_SecondModemPhoneCall = pCCCI->mHw->GetFlag_SecondModemPhoneCall();
   if(Flag_SecondModemPhoneCall != 0){
      ALOGD("2nd MD LAD_SetOutputDevice (0x%x), HW_Mode:%x",A2M_SetOutputDevice,HW_Mode);
      pATCmd->Spc_SetOutputDevice(HW_Mode);
      return true;
   }
#endif
   ALOGD(AUDMSG"LAD_SetOutputDevice (0x%x), HW_Mode:%x",A2M_SetOutputDevice,HW_Mode);
   return pCCCI->SendMessageInQueue(  CCCI_MSG1(A2M_SetOutputDevice, HW_Mode));
}

bool LAD::LAD_SetInputSource(LadInPutDevice_Line HW_Mode)
{
/*
   LADIN_FM_Radio = 0,       // 0: FM analog line in Mono
   LADIN_Microphone1,        // 1: Phone Mic
   LADIN_Microphone2,        // 2: Earphone Mic
   LADIN_BTIn,               // 3: BT Mic
   LADIN_SingleDigitalMic,   // 4: Single digital Microphone
   LADIN_DualAnalogMic,      // 5: Dual analog Microphone
   LADIN_DualDigitalMic,     // 6: Dual digital Microphone
   LADIN_FM_AnalogIn_Stereo  // 7: FM analog line in Stereo
*/


#if defined(MTK_DUAL_MIC_SUPPORT)
   //
   // For the dual mic solution, always replace the "Phone Mic" setting as "Dual Mic" setting.
   // For the single mic solution, set the mode as "Phone Mic".
   //
   if(pCCCI->mHw!=NULL)
   {
#if defined(MTK_AUDIO_HD_REC_SUPPORT)
       int mode;
       pCCCI->mHw->getMode(&mode);
       if(mode == android_audio_legacy::AudioSystem::MODE_IN_CALL){
           if(pCCCI->mHw->mHAL_DualMic_Setting == true){ // integration with MMI setting
               if(HW_Mode == LADIN_Microphone1){
                   //ALOGD(AUDMSG"LAD_SetInputSource, Set LADIN_DualAnalogMic");
                   HW_Mode = LADIN_DualAnalogMic;
               }
           }
       }else{
           if(HW_Mode == LADIN_Microphone1){
               HW_Mode = LADIN_DualAnalogMic;
           }
       }
#else
       if(pCCCI->mHw->mHAL_DualMic_Setting == true){ // integration with MMI setting
           if(HW_Mode == LADIN_Microphone1){
               //ALOGD(AUDMSG"LAD_SetInputSource, Set LADIN_DualAnalogMic");
               HW_Mode = LADIN_DualAnalogMic;
           }
       }
#endif
    }
#endif

#if MTK_DIGITAL_MIC_SUPPORT
   if(HW_Mode == LADIN_Microphone1)
   {
      HW_Mode = LADIN_SingleDigitalMic;
   }
   else if(HW_Mode == LADIN_DualAnalogMic)
   {
      HW_Mode = LADIN_DualDigitalMic;
   }
#endif


   if(m_InputSrc == (int)HW_Mode)
      return true;

#if defined(MTK_DT_SUPPORT)
   int Flag_SecondModemPhoneCall = 0;
   unsigned char cHW_Mode=0;
   Flag_SecondModemPhoneCall = pCCCI->mHw->GetFlag_SecondModemPhoneCall();
   if(Flag_SecondModemPhoneCall != 0){
       ALOGD(AUDMSG"2nd MD LAD_SetInputSource, A2M_SetInputSource(0x%x) HW_Mode:%x",A2M_SetInputSource,HW_Mode);
       return pATCmd->Spc_SetInputSource(HW_Mode);
   }
#endif

   m_InputSrc = (int)HW_Mode;
   ALOGD(AUDMSG"LAD_SetInputSource (0x%x), HW_Mode(%x)(%x)",A2M_SetInputSource,HW_Mode,m_InputSrc);
   return pCCCI->SendMessageInQueue(  CCCI_MSG1(A2M_SetInputSource, HW_Mode));
}

bool LAD::LAD_ForceSetInputSource(LadInPutDevice_Line HW_Mode)
{
#if MTK_DIGITAL_MIC_SUPPORT
   if(HW_Mode == LADIN_Microphone1)
   {
      HW_Mode = LADIN_SingleDigitalMic;
   }
   else if(HW_Mode == LADIN_DualAnalogMic)
   {
      HW_Mode = LADIN_DualDigitalMic;
   }
#endif

   if (m_InputSrc == (int)HW_Mode)
       return true;

   m_InputSrc = (int)HW_Mode;
   ALOGD(AUDMSG"LAD_ForceSetInputSource (0x%x), HW_Mode(%x)(%x)",A2M_SetInputSource,HW_Mode,m_InputSrc);
   return pCCCI->SendMessageInQueue(  CCCI_MSG1(A2M_SetInputSource, HW_Mode));
}

/*-------------------Speech Control----------------------------*/
bool LAD::LAD_SetSpeechMode(ENUM_Speech_Mode Speech_Mode)
{
   //m_SideToneGain = 0;
   //m_MicGain = 0;
   //m_InputSrc = (int)0;
#if defined(MTK_DT_SUPPORT)
   int Flag_SecondModemPhoneCall = 0;
   Flag_SecondModemPhoneCall = pCCCI->mHw->GetFlag_SecondModemPhoneCall();
   if(Flag_SecondModemPhoneCall != 0){
      ASSERT(Speech_Mode < 7);
      ALOGD(AUDMSG"2nd MD LAD_SetSpeechMode, A2M_SetSpeechMode(0x%x) HW_Mode:%x",A2M_SetSpeechMode,Speech_Mode);
      return pATCmd->Spc_SetSpeechMode_Adaptation(Speech_Mode);
   }
#endif
   ALOGD(AUDMSG"LAD SetSpeechMode (0x%x), HW_Mode:%x",A2M_SetSpeechMode,Speech_Mode);
   return pCCCI->SendMessageInQueue(CCCI_MSG1(A2M_SetSpeechMode, Speech_Mode));
}


bool LAD::LAD_Speech_On(int sp_mode)
{
   int mode=0;
#if defined(MTK_DT_SUPPORT)
    int Flag_SecondModemPhoneCall = 0, ret = 0;
    Flag_SecondModemPhoneCall = pCCCI->mHw->GetFlag_SecondModemPhoneCall();
    if(Flag_SecondModemPhoneCall == 1){
       mode = 0;
       ALOGD("2nd MD LAD_Speech_On, A2M_Speech_On(0x%x) mode:%x", A2M_Speech_On, mode);
       //pCCCI->mHw->mAnaReg->SetAnaReg(AUDIO_CON34,0x8000,0x8000); // enable mic bias
       LAD_SwitchMicBias(1);
       pCCCI->mHw->mMicMute_ModemStatus = false;
       ret = pATCmd->Spc_Speech_On(mode);  // Stan,  0:2G, 1:3G
       pCCCI->mHw->Set_Recovery_Speech(true);
       return ret;
    }
    else if(Flag_SecondModemPhoneCall!=0){
       ALOGE("LAD_Speech_On, A2M_Speech_On(0x%x) Invalid Flag_SecondModemPhoneCall:%d !!!!", A2M_Speech_On,Flag_SecondModemPhoneCall);
       return -1;
    }
#endif
   pCCCI->CheckMDSideSphCoefArrival(); // must pass; otherwise, set wrong speech mode (only this time)
   pCCCI->mHw->SwitchAudioClock(true);

   LOG_LAD(AUDMSG"LAD Speech_On (0x%x), sp_mode:%x",A2M_Speech_On,sp_mode);

// In l1sp.c, the phone call status define as follow.
// #define RAT_2G_MODE  0
// #define RAT_3G_MODE  1
// #define RAT_3G324M_MODE 2 --> VT
// For the VT phone call, AP side must ask the modem side l1audio drvier that this is VT phone call.

   // sp_mode=2 --> this is VT phone call
   if(sp_mode == 2){
   // for VT phone call. Set mode as 2. AP side must info modem side that it is VT call
      mode=2;
   }
   else{
   // for 2G/3G phone call. Set mode as 1.  Modem side can query 2G/3G phone call.
      mode=0;
   }
   return pCCCI->SendMessageInQueue(CCCI_MSG1(A2M_Speech_On, mode));  // Stan,  0:2G, 1:3G

}

bool LAD::LAD_Speech_Off()
{
   m_SideToneGain = 0;
   m_MicGain = 0;
   m_InputSrc = (int)0;
#if defined(MTK_DT_SUPPORT)
    int Flag_SecondModemPhoneCall = 0, ret = 0;
    Flag_SecondModemPhoneCall = pCCCI->mHw->GetFlag_SecondModemPhoneCall();
    if(Flag_SecondModemPhoneCall == 1){
        ret = pATCmd->Spc_Speech_Off();
        //pCCCI->mHw->mAnaReg->SetAnaReg(AUDIO_CON34,0x0000,0x8000); // disable mic bias
        LAD_SwitchMicBias(0);
        pCCCI->mHw->mMicMute_ModemStatus = true;
        pCCCI->mHw->SetFlag_SecondModemPhoneCall(0);
        pCCCI->mHw->Set_Recovery_Speech(false);
        return ret;
    }
    else if(Flag_SecondModemPhoneCall!=0){
        ALOGE("LAD_Speech_Off, A2M_Speech_Off(0x%x) Invalid Flag_SecondModemPhoneCall:%d !!!!", A2M_Speech_Off, Flag_SecondModemPhoneCall);
        return -1;
    }
#endif
   LOG_LAD(AUDMSG"LAD Speech_Off (0x%x) ",A2M_Speech_Off);
   return pCCCI->SendMessageInQueue(CCCI_MSG1(A2M_Speech_Off, 0));
}

int LAD::LAD_Set_Speech_Enhancement(bool bEnable)
{
   if(mSph_Enh_enable == bEnable){
      LOG_LAD(AUDMSG"LAD_Set_Speech_Enh, mSph_Enh_enable(%d)=bEnable(%d) return",mSph_Enh_enable,bEnable);
      return true;
   }

   mSph_Enh_enable = bEnable;
   LOG_LAD(AUDMSG"LAD_Set_Speech_Enh (0x%x),(%d) ",A2M_SetSphEnhancementAll,bEnable);
   int ret = 0;
   ret = pCCCI->SendMessageInQueue(CCCI_MSG1(A2M_SetSphEnhancementAll, bEnable));
   return ret;
}
#if defined(MTK_DT_SUPPORT)
bool LAD::LAD_Default_Tone_Play(uint8 toneIdx)
{
    int Flag_SecondModemPhoneCall = 0, ret = 0;
    Flag_SecondModemPhoneCall = pCCCI->mHw->GetFlag_SecondModemPhoneCall();
    ALOGD("LAD_Default_Tone_Play:Flag_SecondModemPhoneCall(%d), toneIdx(%d)", Flag_SecondModemPhoneCall, toneIdx);
    if(Flag_SecondModemPhoneCall == 0)
    {
        ALOGD("LAD_Default_Tone_Play: not support! Flag_SecondModemPhoneCall(%d)", Flag_SecondModemPhoneCall);
        return true;
    }
    else if(Flag_SecondModemPhoneCall == 1)
    {
        ret = pATCmd->Spc_Default_Tone_Play(toneIdx);
        return ret;
    }
    else
    {
        ALOGE("LAD_Default_Tone_Play, Invalid Flag_SecondModemPhoneCall:%d !!!!", Flag_SecondModemPhoneCall);
        return -1;
    }
}

bool LAD::LAD_Default_Tone_Stop()
{
    int Flag_SecondModemPhoneCall = 0, ret = 0;
    Flag_SecondModemPhoneCall = pCCCI->mHw->GetFlag_SecondModemPhoneCall();
    ALOGD("LAD_Default_Tone_Stop:Flag_SecondModemPhoneCall(%d)", Flag_SecondModemPhoneCall);
    if(Flag_SecondModemPhoneCall == 0)
    {
        ALOGD("LAD_Default_Tone_Stop: not support! Flag_SecondModemPhoneCall(%d)", Flag_SecondModemPhoneCall);
        return true;
    }
    else if(Flag_SecondModemPhoneCall == 1)
    {
        ret = pATCmd->Spc_Default_Tone_Stop();
        return ret;
    }
    else
    {
        ALOGE("LAD_Default_Tone_Stop, Invalid Flag_SecondModemPhoneCall:%d !!!!", Flag_SecondModemPhoneCall);
        return -1;
    }
}

bool LAD::LAD_ATCMD_Test()
{
    int mode = 0, Flag_SecondModemPhoneCall = 0;
    ALOGD("Test MD LAD_ATCMD_Test, A2M_Speech_On(0x%x) mode:%x", A2M_Speech_On, mode);
    return pATCmd->Spc_Speech_On(mode);  // Stan,  0:2G, 1:3G
}
#endif


#if defined(MTK_SPH_EHN_CTRL_SUPPORT)
/// get sppech enhancement masks from property
void LAD::LoadSphEnhMasks()
{
    // property value
    char property_value_sph_enh_masks[PROPERTY_VALUE_MAX]; 

    // default value (all enhancement on)
    char property_default_value_sph_enh_masks[PROPERTY_VALUE_MAX];
    sprintf(property_default_value_sph_enh_masks, "%u %u", SPH_ENH_MASK_ALL, SPH_ENH_SUBFUNC_MASK_ALL); 

    // get property   
    property_get(PROPERTY_KEY_SPH_ENH_MASKS, property_value_sph_enh_masks, property_default_value_sph_enh_masks);
    sscanf(property_value_sph_enh_masks, "%u %u", &mSphEnhInfo.spe_usr_mask, &mSphEnhInfo.spe_usr_subFunc_mask);

    ALOGD("property_get sph enh masks = (0x%x 0x%x)", mSphEnhInfo.spe_usr_mask, mSphEnhInfo.spe_usr_subFunc_mask);
}

/// set sppech enhancement masks to property
void LAD::SaveSphEnhMasks()
{
    // property value
    char property_value_sph_enh_masks[PROPERTY_VALUE_MAX]; 

    // set property
    sprintf(property_value_sph_enh_masks, "%u %u", mSphEnhInfo.spe_usr_mask, mSphEnhInfo.spe_usr_subFunc_mask); 
    property_set(PROPERTY_KEY_SPH_ENH_MASKS, property_value_sph_enh_masks);

    ALOGD("property_set sph enh masks = (0x%x 0x%x)", mSphEnhInfo.spe_usr_mask, mSphEnhInfo.spe_usr_subFunc_mask);
}


/// Set specific enhancement configuration
int LAD::LAD_Set_Speech_Enhancement_Info(const SPH_ENH_INFO_T &eSphEnhInfo)
{
    bool retval = true;

    uint32 param;

    /// (Ctrl 2) set spe_usr_mask to decide each enhancement function to be on/off
    param = (eSphEnhInfo.spe_usr_mask << 4) | SPH_ENH_USR_CFG1;
    LOG_LAD(AUDMSG"LAD_Set_Speech_Enhancement_Info (0x%x),(0x%x) ",A2M_SetSphEnhancementSpecific, param);
    retval &= pCCCI->SendMessageInQueue(CCCI_MSG1(A2M_SetSphEnhancementSpecific, param));

    /// (Ctrl 3) set spe_usr_subFunc_mask[0:15] to decide each enhancement function to be on/off
    param = ((eSphEnhInfo.spe_usr_subFunc_mask & 0xFFFF) << 4) | SPH_ENH_USR_CFG2_1;
    LOG_LAD(AUDMSG"LAD_Set_Speech_Enhancement_Info (0x%x),(0x%x) ",A2M_SetSphEnhancementSpecific, param);
    retval &= pCCCI->SendMessageInQueue(CCCI_MSG1(A2M_SetSphEnhancementSpecific, param));

    /// (Ctrl 4) set spe_usr_subFunc_mask[16:31] to decide each enhancement function to be on/off
    param = ((eSphEnhInfo.spe_usr_subFunc_mask >> 16) << 4) | SPH_ENH_USR_CFG2_2;
    LOG_LAD(AUDMSG"LAD_Set_Speech_Enhancement_Info (0x%x),(0x%x) ",A2M_SetSphEnhancementSpecific, param);
    retval &= pCCCI->SendMessageInQueue(CCCI_MSG1(A2M_SetSphEnhancementSpecific, param));

    /// (Ctrl 5) after ENUM_SPH_ENH_CTRL={2,3,4} is done, apply these configure
    LOG_LAD(AUDMSG"LAD_Set_Speech_Enhancement_Info (0x%x),(0x%x) ",A2M_SetSphEnhancementSpecific, SPH_ENH_APPLY);
    retval &= pCCCI->SendMessageInQueue(CCCI_MSG1(A2M_SetSphEnhancementSpecific, SPH_ENH_APPLY));

    /// update new config
    mSphEnhInfo = eSphEnhInfo;
    SaveSphEnhMasks();

    return retval;
}

int LAD::LAD_Set_Speech_Enhancement_MainFunction(uint32 MainConfig)
{
    int ret  = true;
    /// (Ctrl 2) set spe_usr_mask to decide each enhancement function to be on/off
    MainConfig= (MainConfig << 4) | SPH_ENH_USR_CFG1;
    LOG_LAD(AUDMSG"LAD_Set_Speech_Enhancement_MainFunction (0x%x),(0x%x) ",A2M_SetSphEnhancementSpecific, MainConfig);
    ret &= pCCCI->SendMessageInQueue(CCCI_MSG1(A2M_SetSphEnhancementSpecific, MainConfig));
    return ret;
}


int LAD::LAD_Set_Speech_Enhancement_SubFunctionHiBits(uint32 SubConfig)
{
    int ret  = true;
    /// (Ctrl 4) set spe_usr_subFunc_mask[16:31] to decide each enhancement function to be on/off
    SubConfig = ((SubConfig >> 16) << 4) | SPH_ENH_USR_CFG2_2;
    LOG_LAD(AUDMSG"LAD_Set_Speech_Enhancement_Info (0x%x),(0x%x) ",A2M_SetSphEnhancementSpecific, SubConfig);
    ret &= pCCCI->SendMessageInQueue(CCCI_MSG1(A2M_SetSphEnhancementSpecific, SubConfig));
    return ret;
}


int LAD::LAD_Set_Speech_Enhancement_SubFunctionLowBits(uint32 SubConfig)
{
    int ret  = true;
    /// (Ctrl 3) set spe_usr_subFunc_mask[0:15] to decide each enhancement function to be on/off
    SubConfig = ((SubConfig & 0xFFFF) << 4) | SPH_ENH_USR_CFG2_1;
    LOG_LAD(AUDMSG"LAD_Set_Speech_Enhancement_Info (0x%x),(0x%x) ",A2M_SetSphEnhancementSpecific, SubConfig);
    ret &= pCCCI->SendMessageInQueue(CCCI_MSG1(A2M_SetSphEnhancementSpecific, SubConfig));
    return ret;
}

int LAD::LAD_Set_Speech_Enhancement_FunctionEnable( )
{
    int ret=0;
    /// (Ctrl 5) after ENUM_SPH_ENH_CTRL={2,3,4} is done, apply these configure
    LOG_LAD(AUDMSG"LAD_Set_Speech_Enhancement_Info (0x%x),(0x%x) ",A2M_SetSphEnhancementSpecific, SPH_ENH_APPLY);
    ret &= pCCCI->SendMessageInQueue(CCCI_MSG1(A2M_SetSphEnhancementSpecific, SPH_ENH_APPLY));
    return ret;
}

int LAD::LAD_Set_Speech_Enhancement_Enable(bool bEnable)
{
    int ret=0;
    LOG_LAD(AUDMSG"LAD_Set_Speech_Enhancement_Enable (0x%x),(0x%x) ",A2M_SetSphEnhancementSpecific, bEnable);
    ret = pCCCI->SendMessageInQueue(CCCI_MSG1(A2M_SetSphEnhancementSpecific, bEnable));
    return ret;
}
#endif //MTK_SPH_EHN_CTRL_SUPPORT


bool LAD::LAD_SetAfeLoopback(bool bLoopbackOn, bool bSwapADDA, bool bConnectAllADDA)
{
    uint8 param = (bConnectAllADDA << 2) | (bSwapADDA << 1) | bLoopbackOn;
    // 0: afe loopback (ADC_1 => DAC_1,   ADC_2 => DAC_2)   off
    // 1: afe loopback (ADC_1 => DAC_1,   ADC_2 => DAC_2)   on

    // 2: afe loopback (ADC_1 => DAC_2,   ADC_2 => DAC_1)   off
    // 3: afe loopback (ADC_1 => DAC_2,   ADC_2 => DAC_1)   on

    // 4: afe loopback (ADC_1 => DAC_1+2, ADC_2 => DAC_1+2) off
    // 5: afe loopback (ADC_1 => DAC_1+2, ADC_2 => DAC_1+2) on
#if defined(MTK_DT_SUPPORT)
    int Flag_SecondModemPhoneCall = 0;
    Flag_SecondModemPhoneCall = pCCCI->mHw->GetFlag_SecondModemPhoneCall();
    if(Flag_SecondModemPhoneCall != 0){
       ALOGD("LAD_SetAfeLoopback: not support! Flag_SecondModemPhoneCall(%d)", Flag_SecondModemPhoneCall);
       return true;
    }
#endif

    LOG_LAD(AUDMSG"LAD_SetAfeLoopback, A2M_SetAfeLoopback param:%d", param);
    return pCCCI->SendMessageInQueue(CCCI_MSG1(A2M_SetAfeLoopback, param));
}


bool LAD::LAD_SetAcousticLoopback(bool bLoopbackOn, bool bUseDMNR)
{
    uint8 param = (bUseDMNR << 1) | bLoopbackOn;
    // 0: acoustic loopback (without DMNR) off
    // 1: acoustic loopback (without DMNR) on
    // 2: acoustic loopback (with    DMNR) off
    // 3: acoustic loopback (with    DMNR) on
#if defined(MTK_DT_SUPPORT)
    int Flag_SecondModemPhoneCall = 0;
    Flag_SecondModemPhoneCall = pCCCI->mHw->GetFlag_SecondModemPhoneCall();
    if(Flag_SecondModemPhoneCall != 0){
       ALOGD("LAD_SetAcousticLoopback: not support! Flag_SecondModemPhoneCall(%d)", Flag_SecondModemPhoneCall);
       return true;
    }
#endif
    pCCCI->CheckMDSideSphCoefArrival(); // must pass; otherwise, set wrong speech mode (only this time)
    LOG_LAD(AUDMSG"LAD_SetAcousticLoopback, A2M_SetAcousticLoopback param:%d", param);
    return pCCCI->SendMessageInQueue(CCCI_MSG1(A2M_SetAcousticLoopback, param));
}


bool LAD::LAD_SetReceiverTest(bool turn_ON)
{
#if defined(MTK_DT_SUPPORT)
   int Flag_SecondModemPhoneCall = 0;
   Flag_SecondModemPhoneCall = pCCCI->mHw->GetFlag_SecondModemPhoneCall();
   if(Flag_SecondModemPhoneCall != 0){
      ALOGD("LAD_SetReceiverTest: not support! Flag_SecondModemPhoneCall(%d)", Flag_SecondModemPhoneCall);
      return true;
   }
#endif
   LOG_LAD(AUDMSG"LAD_SetReceiverTest, turn_ON:%x",turn_ON);
   return pCCCI->SendMessageInQueue(  CCCI_MSG1(A2M_SetReceiverTest, turn_ON));
}

bool LAD::LAD_Set_DAI_Mode(bool turn_ON)
{
#if defined(MTK_DT_SUPPORT)
    int Flag_SecondModemPhoneCall = 0;
    Flag_SecondModemPhoneCall = pCCCI->mHw->GetFlag_SecondModemPhoneCall();
    if(Flag_SecondModemPhoneCall != 0){
       ALOGD("LAD_Set_DAI_Mode: not support! Flag_SecondModemPhoneCall(%d)", Flag_SecondModemPhoneCall);
       return true;
    }
#endif
   LOG_LAD(AUDMSG"LAD_Set_DAI_Mode, turn_ON:%x",turn_ON);
   return pCCCI->SendMessageInQueue(  CCCI_MSG1(A2M_Set_DAI_Mode, turn_ON));
}

bool LAD::LAD_SwitchVCM(int32 TurnOn)
{
#if defined(MTK_DT_SUPPORT)
   int Flag_SecondModemPhoneCall = 0;
   Flag_SecondModemPhoneCall = pCCCI->mHw->GetFlag_SecondModemPhoneCall();
   if(Flag_SecondModemPhoneCall != 0){
      ALOGD("LAD_SwitchVCM: not support! Flag_SecondModemPhoneCall(%d)", Flag_SecondModemPhoneCall);
      return true;
   }
#endif
   ALOGD(AUDMSG"LAD_SwitchVCM, TurnOn:%x",TurnOn);
   return pCCCI->SendMessageInQueue(CCCI_MSG1(A2A_SetVCM, TurnOn));
}

bool LAD::LAD_Set_LoudSPK_RefMicMode(bool TurnOn)
{
#if defined(MTK_DT_SUPPORT)
   int Flag_SecondModemPhoneCall = 0;
   Flag_SecondModemPhoneCall = pCCCI->mHw->GetFlag_SecondModemPhoneCall();
   if(Flag_SecondModemPhoneCall != 0){
      ALOGD("LAD_Set_LoudSPK_RefMicMode: not support! Flag_SecondModemPhoneCall(%d)", Flag_SecondModemPhoneCall);
      return true;
   }
#endif
   ALOGD(AUDMSG"LAD_Set_LoudSPK_RefMicMode, TurnOn:%x",TurnOn);
   return pCCCI->SendMessageInQueue(CCCI_MSG1(A2M_SetLoudSPK_RefMicMode, TurnOn));
}


bool LAD::LAD_SetDaiBtCfg(bool bDaiBtMode, bool bBtLongSync)
{
    // bit 0 : 0=NB, 1=WB
    // bit 1 : 0=short sync, 1=long sync
    uint32 Param = ((bBtLongSync<<1) | bDaiBtMode); 
    
    ALOGD(AUDMSG"LAD_SetDaiBtCfg(0x%x), Param:%x",A2M_SetDaiBtCfg, Param);
    return pCCCI->SendMessageInQueue(CCCI_MSG1(A2M_SetDaiBtCfg, Param));
}

/*-----------------Proprietary Control-----------------------*/
bool LAD::LAD_SwitchMicBias(int32 TurnOn)
{
#if defined(MTK_DT_SUPPORT)
   int Flag_SecondModemPhoneCall = 0;
   Flag_SecondModemPhoneCall = pCCCI->mHw->GetFlag_SecondModemPhoneCall();
   if(Flag_SecondModemPhoneCall != 0){
      ALOGD("LAD_SwitchMicBias(0x%x), TurnOn:%d, Flag_SecondModemPhoneCall(%d)",A2M_SetMicBias, TurnOn , Flag_SecondModemPhoneCall);
      return pCCCI->SendMessageInQueue(CCCI_MSG1(A2M_SetMicBias, TurnOn));
   }
#endif
   ALOGD(AUDMSG"LAD_SwitchMicBias (0x%x), TurnOn:%d",A2M_SetMicBias,TurnOn);
   return pCCCI->SendMessageInQueue(CCCI_MSG1(A2M_SetMicBias, TurnOn));
}


/*-----------------Set Speech Parameters-----------------------*/
int LAD::LAD_SetSpeechParameters(AUDIO_CUSTOM_PARAM_STRUCT* pParam)
{
#if defined(MTK_DT_SUPPORT)
   int Flag_SecondModemPhoneCall = 0;
   Flag_SecondModemPhoneCall = pCCCI->mHw->GetFlag_SecondModemPhoneCall();
   if(Flag_SecondModemPhoneCall != 0){
      ALOGD("LAD_SetSpeechParameters: not support! Flag_SecondModemPhoneCall(%d)", Flag_SecondModemPhoneCall);
      return true;
   }
#endif
   int32 offset = (pCCCI->GetM2AShareBufLen()>=4096)?SPH_PAR_A2M_SHARED_BUFFER_OFFSET:0;
   int32 len = LAD_SHARE_HEADER_LEN + sizeof(AUDIO_CUSTOM_PARAM_STRUCT);
   int8 *pShareBuf = (int8 *)pCCCI->GetA2MShareBufAddress() + offset;
   //get buffer lock to prevent overwrite other's data
   if(pCCCI->A2MBufLock() == true){
       // write header
       WriteShareBufHeader(pShareBuf, (int16)LADBUFID_CCCI_EM_PARAM, (int16)sizeof(AUDIO_CUSTOM_PARAM_STRUCT), (int16)A2M_BUF_HEADER);

       // increment for the header
       pShareBuf += LAD_SHARE_HEADER_LEN;

       // write parameters
       memcpy((void*)pShareBuf, (void*)pParam, sizeof(AUDIO_CUSTOM_PARAM_STRUCT));

       // send message to modem
       ALOGD(AUDMSG"LAD_SetSpeechParameters (0x%x)",AUDIO_CUSTOM_PARAMETER);
       int32 ret = pCCCI->SendMessageInQueue(CCCI_MSG2(AUDIO_CUSTOM_PARAMETER, offset, len));
       if(ret<=0)
           ALOGE(AUDMSG"LAD_SetSpeechParameters, Send CCCI Msg failed!\n");
       return ret;
   }
   else{
       ALOGE(AUDMSG"LAD_SetSpeechParameters fail due to unalbe get A2M Buffer\n");
       return false;
   }
}

#if defined(MTK_DUAL_MIC_SUPPORT)
int LAD::LAD_SetDualMicParameters(AUDIO_CUSTOM_EXTRA_PARAM_STRUCT* pParam)
{
#if defined(MTK_DT_SUPPORT)
   int Flag_SecondModemPhoneCall = 0;
   Flag_SecondModemPhoneCall = pCCCI->mHw->GetFlag_SecondModemPhoneCall();
   if(Flag_SecondModemPhoneCall != 0){
      ALOGD("LAD_SetDualMicParameters: not support! Flag_SecondModemPhoneCall(%d)", Flag_SecondModemPhoneCall);
      return true;
   }
#endif
// NVRAM always contain (44+76), for WB we send (44+76), for NB we send (44)
// define the data length that actually sent to modem base on if WB is supported
#if defined(MTK_WB_SPEECH_SUPPORT)
   int32 sendDataLen = (NUM_ABF_PARAM+NUM_ABFWB_PARAM) * sizeof(unsigned short);
#else
   int32 sendDataLen = NUM_ABF_PARAM * sizeof(unsigned short);
#endif

   int32 offset = (pCCCI->GetM2AShareBufLen()>=4096)? SPH_PAR_A2M_SHARED_BUFFER_OFFSET : 0;;
   int32 len = LAD_SHARE_HEADER_LEN + sendDataLen;
   int8 *pShareBuf = (int8 *)pCCCI->GetA2MShareBufAddress() + offset;
   if(pCCCI->A2MBufLock() == true){
       // write header
       WriteShareBufHeader(pShareBuf, (int16)LADBUFID_CCCI_EM_PARAM, (int16)sendDataLen, (int16)A2M_BUF_HEADER);

       // increment for the header
       pShareBuf += LAD_SHARE_HEADER_LEN;

       // write parameters
       memcpy((void*)pShareBuf, (void*)pParam, sendDataLen);

       {
          unsigned short *ptr;
          int i = 0;
          ptr = (unsigned short*)pShareBuf;
          i = 0;
          ALOGD(AUDMSG"%d, %d, %d, %d, %d, %d, %d, %d, %d, %d", ptr[i+0], ptr[i+1], ptr[i+2], ptr[i+3], ptr[i+4], ptr[i+5], ptr[i+6], ptr[i+7], ptr[i+8], ptr[i+9]);
          i = 10;
          ALOGD(AUDMSG"%d, %d, %d, %d, %d, %d, %d, %d, %d, %d", ptr[i+0], ptr[i+1], ptr[i+2], ptr[i+3], ptr[i+4], ptr[i+5], ptr[i+6], ptr[i+7], ptr[i+8], ptr[i+9]);
          i = 20;
          ALOGD(AUDMSG"%d, %d, %d, %d, %d, %d, %d, %d, %d, %d", ptr[i+0], ptr[i+1], ptr[i+2], ptr[i+3], ptr[i+4], ptr[i+5], ptr[i+6], ptr[i+7], ptr[i+8], ptr[i+9]);
          i = 30;
          ALOGD(AUDMSG"%d, %d, %d, %d, %d, %d, %d, %d, %d, %d", ptr[i+0], ptr[i+1], ptr[i+2], ptr[i+3], ptr[i+4], ptr[i+5], ptr[i+6], ptr[i+7], ptr[i+8], ptr[i+9]);
          i = 40;
          ALOGD(AUDMSG"%d, %d, %d, %d, %d, %d, %d, %d, %d, %d", ptr[i+0], ptr[i+1], ptr[i+2], ptr[i+3], ptr[i+4], ptr[i+5], ptr[i+6], ptr[i+7], ptr[i+8], ptr[i+9]);
          i = 50;
          ALOGD(AUDMSG"%d, %d, %d, %d, %d, %d, %d, %d, %d, %d", ptr[i+0], ptr[i+1], ptr[i+2], ptr[i+3], ptr[i+4], ptr[i+5], ptr[i+6], ptr[i+7], ptr[i+8], ptr[i+9]);
          i = 60;
          ALOGD(AUDMSG"%d, %d, %d, %d, %d, %d, %d, %d, %d, %d", ptr[i+0], ptr[i+1], ptr[i+2], ptr[i+3], ptr[i+4], ptr[i+5], ptr[i+6], ptr[i+7], ptr[i+8], ptr[i+9]);
          i = 70;
          ALOGD(AUDMSG"%d, %d, %d, %d, %d, %d, %d, %d, %d, %d", ptr[i+0], ptr[i+1], ptr[i+2], ptr[i+3], ptr[i+4], ptr[i+5], ptr[i+6], ptr[i+7], ptr[i+8], ptr[i+9]);
          i = 80;
          ALOGD(AUDMSG"%d, %d, %d, %d, %d, %d, %d, %d, %d, %d", ptr[i+0], ptr[i+1], ptr[i+2], ptr[i+3], ptr[i+4], ptr[i+5], ptr[i+6], ptr[i+7], ptr[i+8], ptr[i+9]);
          i = 90;
          ALOGD(AUDMSG"%d, %d, %d, %d, %d, %d, %d, %d, %d, %d", ptr[i+0], ptr[i+1], ptr[i+2], ptr[i+3], ptr[i+4], ptr[i+5], ptr[i+6], ptr[i+7], ptr[i+8], ptr[i+9]);
          i = 100;
          ALOGD(AUDMSG"%d, %d, %d, %d, %d, %d, %d, %d, %d, %d", ptr[i+0], ptr[i+1], ptr[i+2], ptr[i+3], ptr[i+4], ptr[i+5], ptr[i+6], ptr[i+7], ptr[i+8], ptr[i+9]);
          i = 110;
          ALOGD(AUDMSG"%d, %d, %d, %d, %d, %d, %d, %d, %d, %d", ptr[i+0], ptr[i+1], ptr[i+2], ptr[i+3], ptr[i+4], ptr[i+5], ptr[i+6], ptr[i+7], ptr[i+8], ptr[i+9]);
       }


       // send message to modem
       ALOGD(AUDMSG"LAD_SetDualMicParameters (0x%x), number of para(%d)",AUDIO_DUAL_MIC_CUSTOM_PARAMETER, sendDataLen);
       int32 ret = pCCCI->SendMessageInQueue(CCCI_MSG2(AUDIO_DUAL_MIC_CUSTOM_PARAMETER, offset, len));
       if(ret<=0)
           ALOGE(AUDMSG"LAD_SetDualMicParameters, Send CCCI Msg failed!\n");
       return ret;
   }
   else{
       ALOGE(AUDMSG"LAD_SetDualMicParameters fail due to unalbe get A2M Buffer\n");
       return false;
   }
}
#endif

#if defined(MTK_WB_SPEECH_SUPPORT)
/*-----------------Set WB Speech Parameters-----------------------*/
int LAD::LAD_SetWBSpeechParameters(AUDIO_CUSTOM_WB_PARAM_STRUCT* pWBParam)
{
#if defined(MTK_DT_SUPPORT)
   int Flag_SecondModemPhoneCall = 0;
   Flag_SecondModemPhoneCall = pCCCI->mHw->GetFlag_SecondModemPhoneCall();
   if(Flag_SecondModemPhoneCall != 0){
      ALOGD("LAD_SetWBSpeechParameters: not support! Flag_SecondModemPhoneCall(%d)", Flag_SecondModemPhoneCall);
      return true;
   }
#endif
   int   ret = true;
   int32 offset = (pCCCI->GetM2AShareBufLen()>=4096)? SPH_PAR_A2M_SHARED_BUFFER_OFFSET : 0;;
   int32 len = LAD_SHARE_HEADER_LEN + sizeof(AUDIO_CUSTOM_WB_A2M_PARAM_STRUCT);
   int8 *pShareBuf = (int8 *)pCCCI->GetA2MShareBufAddress() + offset;
   AUDIO_CUSTOM_WB_A2M_PARAM_STRUCT *pA2M_Cust_Wb_Msg;
   //get buffer lock to prevent overwrite other's data
   if(pCCCI->A2MBufLock() == true){
       // write header
       WriteShareBufHeader(pShareBuf, (int16)LADBUFID_CCCI_EM_PARAM, (int16)sizeof(AUDIO_CUSTOM_WB_A2M_PARAM_STRUCT), (int16)A2M_BUF_HEADER);

       // increment for the header
       pShareBuf += LAD_SHARE_HEADER_LEN;

       pA2M_Cust_Wb_Msg = (AUDIO_CUSTOM_WB_A2M_PARAM_STRUCT *)pShareBuf;

       // write input fir parameters
       memcpy((void*)pA2M_Cust_Wb_Msg->speech_mode_wb_para, (void*)pWBParam->speech_mode_wb_para, sizeof(pWBParam->speech_mode_wb_para));
       memcpy((void*)pA2M_Cust_Wb_Msg->sph_wb_fir, (void*)pWBParam->sph_wb_in_fir, sizeof(pWBParam->sph_wb_in_fir));
       pA2M_Cust_Wb_Msg->input_out_fir_flag= 0;

       // send message to modem
       ALOGD(AUDMSG"LAD_SetWBSpeechParameters(0x%x) + input FIR",AUDIO_WB_CUSTOM_PARAMETER);
       ret = pCCCI->SendMessageInQueue(CCCI_MSG2(AUDIO_WB_CUSTOM_PARAMETER, offset, len));
       if(ret<=0)
           ALOGE(AUDMSG"LAD_SetWBSpeechParameters, Send CCCI Msg failed!\n");

       //get buffer lock to prevent overwrite other's data
       if(pCCCI->A2MBufLock() == true){
           // (ALPS00274550) update header info to avoid other messages overwrite it after WB input FIR is done
           pShareBuf = (int8 *)pCCCI->GetA2MShareBufAddress() + offset;
           WriteShareBufHeader(pShareBuf, (int16)LADBUFID_CCCI_EM_PARAM, (int16)sizeof(AUDIO_CUSTOM_WB_A2M_PARAM_STRUCT), (int16)A2M_BUF_HEADER);

           // write output fir parameters
           pA2M_Cust_Wb_Msg = (AUDIO_CUSTOM_WB_A2M_PARAM_STRUCT *)(pShareBuf + LAD_SHARE_HEADER_LEN);
           memcpy((void*)pA2M_Cust_Wb_Msg->speech_mode_wb_para, (void*)pWBParam->speech_mode_wb_para, sizeof(pWBParam->speech_mode_wb_para));
           memcpy((void*)pA2M_Cust_Wb_Msg->sph_wb_fir, (void*)pWBParam->sph_wb_out_fir, sizeof(pWBParam->sph_wb_out_fir));
           pA2M_Cust_Wb_Msg->input_out_fir_flag= 1;
           ALOGD(AUDMSG"LAD_SetWBSpeechParameters(0x%x) + output FIR",AUDIO_WB_CUSTOM_PARAMETER);
           ret &= pCCCI->SendMessageInQueue(CCCI_MSG2(AUDIO_WB_CUSTOM_PARAMETER, offset, len));
           if(ret<=0)
               ALOGE(AUDMSG"LAD_SetWBSpeechParameters, Send CCCI Msg failed!\n");
       }
       else{
           ALOGE(AUDMSG"LAD_SetWBSpeechParameters fail due to unalbe get A2M Buffer #2\n");
           ret = false;
       }
   }
   else{
       ALOGE(AUDMSG"LAD_SetWBSpeechParameters fail due to unalbe get A2M Buffer #1\n");
       ret = false;
   }

   return ret;
}
#endif

/*-----------------Set HD Record Parameters-----------------------*/
#if defined(MTK_AUDIO_HD_REC_SUPPORT)
int LAD::LAD_SetHdRecordParameters(AUDIO_HD_RECORD_PARAM_STRUCT* pParam)
{
    const int32 len_hd_record_param = CFG_FILE_AUDIO_HD_REC_PAR_SIZE;

    const int32 offset = (pCCCI->GetM2AShareBufLen() >= 4096) ? SPH_PAR_A2M_SHARED_BUFFER_OFFSET : 0;
    const int32 len = LAD_SHARE_HEADER_LEN + len_hd_record_param;

    int8 *pShareBuf = (int8 *)pCCCI->GetA2MShareBufAddress() + offset;

    // get buffer lock to prevent overwrite other's data
    if(pCCCI->A2MBufLock() == false){
       ALOGE(AUDMSG"LAD_SetHdRecordParameters fail due to unalbe get A2M Buffer\n");
       return false;
    }
    else{
        // write header
        WriteShareBufHeader(pShareBuf, LADBUFID_CCCI_HD_RECORD, (int16)len_hd_record_param, (int16)A2M_BUF_HEADER);

        // increment for the header
        pShareBuf += LAD_SHARE_HEADER_LEN;

        // write hd record parameters
        memcpy((void*)pShareBuf, (void*)pParam, len_hd_record_param);

#if 0 // Debug Info
        LOG_LAD("-LAD_SetHdRecordParameters, hd_rec_mode_num=%u", pParam->hd_rec_mode_num);

        LOG_LAD("-LAD_SetHdRecordParameters, hd_rec_fir_num=%u", pParam->hd_rec_fir_num);

        for(int i=0;i<pParam->hd_rec_mode_num;i++)
            for(int j=0;j<16;j++)
                LOG_LAD("-LAD_SetHdRecordParameters, hd_rec_speech_mode_para[%d][%d]=%u", i,j,pParam->hd_rec_speech_mode_para[i][j]);

        for(int i=0;i<pParam->hd_rec_fir_num;i++)
            for(int j=0;j<90;j++)
                LOG_LAD("-LAD_SetHdRecordParameters, hd_rec_fir[%d][%d]=%u", i,j,pParam->hd_rec_fir[i][j]);

        for(int i=0;i<pParam->hd_rec_mode_num;i++)
            LOG_LAD("-LAD_SetHdRecordParameters, hd_rec_map_to_fir_for_ch1[%d]=%u", i,pParam->hd_rec_map_to_fir_for_ch1[i]);

        for(int i=0;i<pParam->hd_rec_mode_num;i++)
            LOG_LAD("-LAD_SetHdRecordParameters, hd_rec_map_to_fir_for_ch2[%d]=%u", i,pParam->hd_rec_map_to_fir_for_ch2[i]);

        for(int i=0;i<pParam->hd_rec_mode_num;i++)
            LOG_LAD("-LAD_SetHdRecordParameters, hd_rec_map_to_dev_mode[%d]=%u", i,pParam->hd_rec_map_to_dev_mode[i]);

        for(int i=0;i<pParam->hd_rec_mode_num;i++)
            LOG_LAD("-LAD_SetHdRecordParameters, hd_rec_map_to_input_src[%d]=%u", i,pParam->hd_rec_map_to_input_src[i]);

        for(int i=0;i<pParam->hd_rec_mode_num;i++)
            LOG_LAD("-LAD_SetHdRecordParameters, hd_rec_map_to_stereo_flag[%d]=%u", i,pParam->hd_rec_map_to_stereo_flag[i]);

        uint16 *dbg_ptr = (uint16 *) pShareBuf;
        LOG_LAD("-LAD_SetHdRecordParameters, [pShareBuf]hd_rec_mode_num=%u", dbg_ptr[0]);
        LOG_LAD("-LAD_SetHdRecordParameters, [pShareBuf]hd_rec_fir_num=%u", dbg_ptr[1]);
        LOG_LAD("-LAD_SetHdRecordParameters, [pShareBuf]hd_rec_speech_mode_para[0][0] = %u", dbg_ptr[2]);
#endif

        // send message to modem
        ALOGD(AUDMSG"LAD_SetHdRecordParameters(0x%x), size of param: %d", AUDIO_HD_RECORD_PARAMETER, len_hd_record_param);
        return pCCCI->SendMessageInQueue(CCCI_MSG2(AUDIO_HD_RECORD_PARAMETER, offset, len));
    }
}
#endif

int LAD::LAD_GetMDCapability(void)
{
#if defined(MTK_DT_SUPPORT)
    int Flag_SecondModemPhoneCall = 0;
    Flag_SecondModemPhoneCall = pCCCI->mHw->GetFlag_SecondModemPhoneCall();
    if(Flag_SecondModemPhoneCall != 0){
       ALOGD("LAD_GetMDCapability: not support! Flag_SecondModemPhoneCall(%d)", Flag_SecondModemPhoneCall);
       return true;
    }
#endif
    int ret = true;
    ret = pCCCI->SendMessageInQueue(CCCI_MSG1(A2M_GetMDCapability, 0));
    return ret;
}

int LAD::LAD_SetMDCapability(int32 value)
{
    int ret = true;
    ret = pCCCI->SendMessageInQueue(CCCI_MSG1(A2M_SetMDCapability, value));
    return ret;
}

int LAD::LAD_LogMDSphCoeff(void)
{
    int vflag = 0;
    int ret   = true;
    char value[PROPERTY_VALUE_MAX];
    //get this property to dump speech log
    property_get("af.md.speechcoef.dump", value, "0");
    vflag=atoi(value);
    if (vflag){
        ALOGD(AUDMSG"LAD_LogMDSphCoeff(0x%x)\n", A2M_LogMDSphCoeff);
        ret = pCCCI->SendMessageInQueue(CCCI_MSG1(A2M_LogMDSphCoeff, 0));
    }
    return ret;
}

int LAD::LAD_RecordSR()
{
#if defined(MTK_DT_SUPPORT)
    int Flag_SecondModemPhoneCall = 0;
    Flag_SecondModemPhoneCall = pCCCI->mHw->GetFlag_SecondModemPhoneCall();
    if(Flag_SecondModemPhoneCall != 0){
        ALOGD("2nd MD LAD_RecordSR\n");
        return pATCmd->Spc_GetParameters(SPC_GET_PARAMS_SR);
    }
#endif
    return 8000;
}
int32 WriteShareBufHeader(int8 *ptr, int16 type, int16 len, int16 sync )
{
   int16 *ptr16 = (int16*)ptr;
   ptr16[0] = (int16)sync;
   ptr16[1] = (int16)type;
   ptr16[2] = (int16)len;
   return LAD_SHARE_HEADER_LEN;
}


/*----------------- Set TTY-CTM -----------------------*/
bool LAD::LAD_TtyCtmOn(int tty_mode)
{
#if defined(MTK_DT_SUPPORT)
   int Flag_SecondModemPhoneCall = 0;
   Flag_SecondModemPhoneCall = pCCCI->mHw->GetFlag_SecondModemPhoneCall();
   if(Flag_SecondModemPhoneCall != 0){
      ALOGD("LAD_TtyCtmOn: not support! Flag_SecondModemPhoneCall(%d)", Flag_SecondModemPhoneCall);
      return true;
   }
#endif
   ALOGD(AUDMSG"LAD_TtyCtmOn, tty_mode:%d",tty_mode);

//   typedef enum{
//      DIRECT_MODE,
//      BAUDOT_MODE
//   } L1Ctm_Interface;

   bool ret = pCCCI->SendMessageInQueue(CCCI_MSG1(A2M_CTM_Open, 1));  // 0: Direct Mode. 1: Baudot Mode
   mTty_used_in_phone_call = true;
   return ret;
}

bool LAD::LAD_TtyCtmOff()
{
#if defined(MTK_DT_SUPPORT)
   int Flag_SecondModemPhoneCall = 0;
   Flag_SecondModemPhoneCall = pCCCI->mHw->GetFlag_SecondModemPhoneCall();
   if(Flag_SecondModemPhoneCall != 0){
      ALOGD("LAD_TtyCtmOff: not support! Flag_SecondModemPhoneCall(%d)", Flag_SecondModemPhoneCall);
      return true;
   }
#endif
   ALOGD(AUDMSG"LAD_TtyCtmOff");
   bool ret = pCCCI->SendMessageInQueue(CCCI_MSG1(A2M_CTM_Close, 0));
   mTty_used_in_phone_call = false;
   return ret;
}

}; // namespace android
