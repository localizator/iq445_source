/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2008
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

/*****************************************************************************
 *
 * Filename:
 * ---------
 *   AudioMeta.cpp
 *
 * Project:
 * --------
 *   Android
 *
 * Description:
 * ------------
 *   Audio HW Testing Tool for META
 *
 * Author:
 * -------
 *   Stan Huang
 *
 *============================================================================
 *             HISTORY
 * Below this line, this part is controlled by CC/CQ. DO NOT MODIFY!!
 *------------------------------------------------------------------------------
 * $Revision:$
 * $Modtime:$
 * $Log:$
 * 
 * 08 26 2012 weiguo.li
 * [ALPS00347285] [Need Patch] [Volunteer Patch]LGE AudioGainTable modification
 * .
 * 
 * 08 23 2012 weiguo.li
 * [ALPS00345541] [Need Patch] [Volunteer Patch]advanced metal NE for LGE project
 * .
 *
 * 07 29 2012 weiguo.li
 * [ALPS00319405] ALPS.JB.BSP.PRA check in CR for Jades
 * .
 *
 * 11 09 2011 weiguo.li
 * [ALPS00090825] [Need Patch] [Volunteer Patch]add dcl code to gb2 and alps
 * .
 *
 * 09 18 2011 donglei.ji
 * [ALPS00074126] [MP Feature Patch Back]WB DMNR patch back
 * [GB2]WB DMNR patch back..
 *
 * 08 12 2011 donglei.ji
 * [ALPS00066660] [Need Patch] [Volunteer Patch]Porting Advanced Meta Mode code
 * Porting meta tool dual mic support - MM handle framework.
 *
 *
 *------------------------------------------------------------------------------
 * Upper this line, this part is controlled by CC/CQ. DO NOT MODIFY!!
 *============================================================================
 ****************************************************************************/


/*****************************************************************************
*                E X T E R N A L   R E F E R E N C E S
******************************************************************************
*/

#include <AudioYusuLad.h>
#include <AudioYusuCcci.h>
#include <AudioYusuVolumeController.h>
#include <AudioYusuDef.h>
#include <AudioYusuHeadsetMessage.h>
#include <AudioCustParam.h>
#include <AudioSystem.h>

#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <signal.h>
#include <utils/threads.h>

#include <AudioAfe.h>
#include <AudioAnalogAfe.h>
#include <AudioFtm.h>
#include <AudioIoctl.h>
#include <AudioMeta.h>

#include <AudioAMPControlInterface.h>

//ACF
#include "AudioCompensationFilter.h"
#include "AudioCompFltCustParam.h"

//HCF
#include "HeadphoneCompensationFilter.h"
#include "HeadphoneCompFltCustParam.h"
//for log
#include "WM2Linux.h"
using namespace android;

/*****************************************************************************
*                     C O M P I L E R   F L A G S
******************************************************************************
*/

#define TEMP_FOR_DUALMIC
#define  AUD_DL1_USE_SLAVE

/*****************************************************************************
*                          C O N S T A N T S
******************************************************************************
*/
#define AUDIO_APPLY_MAX_GAIN (0xffff)
#define AUDIO_APPLY_BIG_GAIN (0xcccc)

#define MIC1_OFF  0
#define MIC1_ON   1
#define MIC2_OFF  2
#define MIC2_ON   3

#define PEER_BUF_SIZE 2*1024
#define ADVANCED_META_MODE 5
/*****************************************************************************
*                         D A T A   T Y P E S
******************************************************************************
*/
enum audio_devices
{
    // output devices
    OUT_EARPIECE = 0,
    OUT_SPEAKER = 1,
    OUT_WIRED_HEADSET = 2,
    DEVICE_OUT_WIRED_HEADPHONE = 3,
    DEVICE_OUT_BLUETOOTH_SCO = 4
};

#ifndef GENERIC_AUDIO
/*****************************************************************************
*                   G L O B A L      V A R I A B L E
******************************************************************************
*/
static android::AudioAfe *mAsmReg = NULL;
static android::AudioAnalog *mAnaReg = NULL;
static android::AudioFtm *mAudFtm = NULL;
static int mFd = 0;
static bool bMetaAudioInited = false;
static bool bMetaAudioRecording = false;
static android::LAD *mLad = NULL;
static android::AudioYusuVolumeController *mVolumeController = NULL;
static android::AudioAMPControlInterface *mMetaAuioDevice = NULL;

Mutex mLock;
Mutex mLockStop;
Condition mWaitWorkCV;
Condition mWaitStopCV;

static int META_SetEMParameter( void *audio_par );
static int META_GetEMParameter( void *audio_par );
static void Audio_Set_Speaker_Vol(int level);
static void Audio_Set_Speaker_On(int Channel);
static void Audio_Set_Speaker_Off(int Channel);
static void Audio_Set_HeadPhone_On(int Channel);
static void Audio_Set_HeadPhone_Off(int Channel);
static void Audio_Set_Earpiece_On();
static void Audio_Set_Earpiece_Off();

/*****************************************************************************
*                        F U N C T I O N   D E F I N I T I O N
******************************************************************************
*/

static void *AudioRecordControlLoop(void *arg)
{
    FT_L4AUD_REQ *pArg = (FT_L4AUD_REQ *)arg;
    FT_L4AUD_CNF audio_cnf;

    memset(&audio_cnf, 0, sizeof(FT_L4AUD_CNF));
    audio_cnf.header.id = FT_L4AUD_CNF_ID;
    audio_cnf.header.token = pArg->header.token;
    audio_cnf.op = pArg->op;
    audio_cnf.status = META_SUCCESS;
    ALOGD("AudioRecordControlLoop in");

    switch (pArg->op)
    {
    case FT_L4AUD_OP_DUALMIC_RECORD:
    {
        ALOGD("AudioRecordControlLoop begine to wait");
        mWaitWorkCV.waitRelative(mLock, milliseconds(pArg->req.dualmic_record.duration+500));
        AudioSystem::setParameters(0, String8("META_DUAL_MIC_RECORD=0"));
        AudioSystem::setParameters(0, String8("META_SET_DUAL_MIC_FLAG=0"));
        break;
    }
    case FT_L4AUD_OP_PLAYBACK_DUALMICRECORD:
    {
        ALOGD("AudioRecordControlLoop begine to wait");
        mWaitWorkCV.waitRelative(mLock, milliseconds(pArg->req.playback_dualmic_record.recordDuration+500));
        AudioSystem::setParameters(0, String8("META_DUAL_MIC_REC_PLAY=0"));
        AudioSystem::setParameters(0, String8("META_SET_DUAL_MIC_FLAG=0"));
        break;
    }
    case FT_L4AUD_OP_PLAYBACK_DUALMICRECORD_HS:
    {
        ALOGD("AudioRecordControlLoop begine to wait");
        mWaitWorkCV.waitRelative(mLock, milliseconds(pArg->req.playback_dualmic_record_hs.recordDuration+500));
        AudioSystem::setParameters(0, String8("META_DUAL_MIC_REC_PLAY_HS=0"));
        AudioSystem::setParameters(0, String8("META_SET_DUAL_MIC_FLAG=0"));
        break;
    }
    default:
        break;
    }

//   usleep(1000000);
    bMetaAudioRecording = false;
    audio_cnf.cnf.dualmic_record_cnf.state = RECORD_END;

    META_LOG("Record end, audio_cnf.status = %d \r", audio_cnf.status);
    WriteDataToPC(&audio_cnf, sizeof(FT_L4AUD_CNF), NULL, 0);
    mWaitStopCV.signal();

    return NULL;
}
static void Audio_Set_Speaker_Vol(int level)
{
    mAudFtm->Audio_Set_Speaker_Vol(level);
}

static void Audio_Set_Speaker_On(int Channel)
{
    mAudFtm->Audio_Set_Speaker_On(Channel);
}

static void Audio_Set_Speaker_Off(int Channel)
{
    mAudFtm->Audio_Set_Speaker_Off(Channel);
}

void Audio_Set_HeadPhone_On(int Channel)
{
    mAudFtm->Audio_Set_HeadPhone_On(Channel);
}

void Audio_Set_HeadPhone_Off(int Channel)
{
    mAudFtm->Audio_Set_HeadPhone_Off(Channel);
}

void Audio_Set_Earpiece_On()
{
    mAudFtm->Audio_Set_Earpiece_On();
}

void Audio_Set_Earpiece_Off()
{
    mAudFtm->Audio_Set_Earpiece_Off();
}

bool META_Audio_init(void)
{
    META_LOG("+META_Audio_init");

    int bootMode = getBootMode();
    if(bootMode==ADVANCED_META_MODE)
    {
        META_LOG("META_Audio_init, the mode is advanced meta mode");
        bMetaAudioInited = true;
        return true;
    }
    if(bMetaAudioInited == true)
    {
        META_LOG("META_Audio_init, Already init");
        return true;
    }
    META_LOG("META_Audio_init bMetaAudioInited=%d",bMetaAudioInited);

    mFd = ::open("/dev/eac", O_RDWR);

    int err =0;
    /*  Init asm */
    mAsmReg = new android::AudioAfe(NULL);
    err = mAsmReg->Afe_Init(mFd);
    if(err == false)
    {
        ALOGD("Afe_Init error");
        return false;
    }

    /* init analog  */
    mAnaReg = new android::AudioAnalog(NULL);
    err = mAnaReg->AnalogAFE_Init(mFd);

    if(err == false)
    {
        ALOGD("AnalogAFE_Init error");
        return false;
    }

#ifdef  AUD_DL1_USE_SLAVE
    ioctl(mFd,SET_DL1_SLAVE_MODE, 1);  //Set Audio DL1 slave mode
    ioctl(mFd,INIT_DL1_STREAM,0x2700);    // init AFE
#else
    ioctl(mFd,SET_DL1_SLAVE_MODE, 0);  //Set Audio DL1 master mode
    ioctl(mFd,INIT_DL1_STREAM,0x3000);    // init AFE
#endif

    ioctl(mFd,START_DL1_STREAM,0);        // init memory

    /* create LAD */
    // lad will only be create once
    if(mLad == NULL)
    {
        mLad = new android::LAD(NULL);
        if( !mLad->LAD_Initial() )
        {
            ALOGD("LAD_Initial error!");
            return false;
        }
        if(mLad->mHeadSetMessager->SetHeadInit() == false)
            ALOGE("Common_Audio_init SetHeadInit error");

//      mLad->LAD_SwitchVCM(true);
        fcntl(mLad->pCCCI->GetRxFd(), F_SETFD, FD_CLOEXEC);
        fcntl(mLad->pCCCI->GetTxFd(), F_SETFD, FD_CLOEXEC);

        ALOGD("LAD create success!");
    }


    mAudFtm = new android::AudioFtm(NULL,mAsmReg,mAnaReg);
    err = mAudFtm->AudFtm_Init(mFd, mLad);

    //Request Analog clock before access analog hw
    mAnaReg->AnalogAFE_Request_ANA_CLK();

    /* new an volume controller */
    mVolumeController = new android::AudioYusuVolumeController(mFd,NULL,mAsmReg,mAnaReg);
    mVolumeController->InitVolumeController ();
    mVolumeController->ApplyGain(AUDIO_APPLY_BIG_GAIN,OUT_SPEAKER);  //apply volume

    mMetaAuioDevice = AudioDeviceManger::createInstance(mFd);
    bMetaAudioInited = true;

    Audio_Set_Speaker_Vol(12);
    Audio_Set_Speaker_Off(Channel_Stereo);
    mAnaReg->SetAnaReg(AUDIO_NCP0,0x102B,0xffff);
    mAnaReg->SetAnaReg(AUDIO_NCP1,0x0600,0x0E00);
    mAnaReg->SetAnaReg(AUDIO_LDO0,0x1030,0x1fff);
    mAnaReg->SetAnaReg(AUDIO_LDO1,0x3010,0xffff);
    mAnaReg->SetAnaReg(AUDIO_LDO2,0x0013,0x0013);
    mAnaReg->SetAnaReg(AUDIO_GLB0,0x2920,0xffff);
    mAnaReg->SetAnaReg(AUDIO_GLB1,0x0000,0xffff);
    mAnaReg->SetAnaReg(AUDIO_REG1,0x0001,0x0001);

    //Release Analog clock after access analog hw
    mAnaReg->AnalogAFE_Release_ANA_CLK();

    mLad->LAD_SetInputSource (LADIN_Microphone1);
    META_LOG("-META_Audio_init");
    return true;
}


bool META_Audio_deinit()
{
    META_LOG("META_Audio_deinit bMetaAudioInited = %d",bMetaAudioInited);
    int bootMode = getBootMode();
    if(bootMode==ADVANCED_META_MODE)
    {
        META_LOG("META_Audio_deinit, the mode is advanced meta mode");
        bMetaAudioInited = false;
        return true;
    }

    if(bMetaAudioInited == true)
    {
        bMetaAudioInited = false;
        ioctl(mFd,STANDBY_DL1_STREAM,0);
    }
    else
    {
        return true;
    }
    if(mVolumeController)
    {
        delete mVolumeController;
        mVolumeController = NULL;
        ALOGD("delete mVolumeController");
    }
    if(mAsmReg)
    {
        mAsmReg->Afe_Deinit();
        delete mAsmReg;
        mAsmReg = NULL;
        ALOGD("delete mAsmReg");
    }
    if(mAnaReg)
    {
        mAnaReg->AnalogAFE_Deinit();
        delete mAnaReg;
        mAnaReg = NULL;
        ALOGD("delete mAnaReg");
    }
    if(mLad)
    {
        mLad->LAD_Deinitial();
        delete mLad;
        mLad = NULL;
        ALOGD("delete mLad");
    }
    return true;
}

bool RecieverLoopbackTest(char echoflag)
{
    META_LOG("RecieverLoopbackTest echoflag=%d",echoflag);
    int ret =0;
    usleep(200*1000);
    //Request Analog clock before access analog hw
    mAnaReg->AnalogAFE_Request_ANA_CLK();
    if ( echoflag)
    {
        mAudFtm->FTM_AFE_UL_Loopback(true);
        mAudFtm->FTM_Ana_Loopback(true);
        mAnaReg->SetAnaReg(AUDIO_CON21,0x1100,0xffff); //PGA on/select
        mAnaReg->SetAnaReg(AUDIO_CON1,0x0c0c,0xffff);//Set HS and HP buffer.
        mAnaReg->SetAnaReg(AUDIO_CON2,0x002c,0x00ff);//mute HP R buffer.
        Audio_Set_Earpiece_On();
    }
    else
    {
        Audio_Set_Earpiece_Off();
        mAudFtm->FTM_AFE_UL_Loopback(false);
        mAudFtm->FTM_Ana_Loopback(false);
    }
    //Release Analog clock after access analog hw
    mAnaReg->AnalogAFE_Release_ANA_CLK();
    return true;
}

bool RecieverLoopbackTest_Mic2(char echoflag)
{
    ALOGD("RecieverLoopbackTest_Mic2 echoflag=%d",echoflag);
    int ret =0;
    usleep(200*1000);
    //Request Analog clock before access analog hw
    mAnaReg->AnalogAFE_Request_ANA_CLK();
    if ( echoflag )
    {
        mAudFtm->FTM_AFE_UL_Loopback(true);
        mAudFtm->FTM_Ana_Loopback(true);
        mAnaReg->SetAnaReg(AUDIO_CON21,0x1200,0xffff); //PGA on/select
        mAnaReg->SetAnaReg(AUDIO_CON1,0x0c0c,0xffff);//Set HS and HP buffer.
        mAnaReg->SetAnaReg(AUDIO_CON2,0x002c,0x00ff);//mute HP L/R buffer.
        Audio_Set_Earpiece_On();
    }
    else
    {
        Audio_Set_Earpiece_Off();
        mAudFtm->FTM_AFE_UL_Loopback(false);
        mAudFtm->FTM_Ana_Loopback(false);
    }
    //Release Analog clock after access analog hw
    mAnaReg->AnalogAFE_Release_ANA_CLK();
    return true;
}


bool RecieverTest(char receiver_test)
{
    META_LOG("RecieverTest receiver_test=%d",receiver_test);
    //Request Analog clock before access analog hw
    mAnaReg->AnalogAFE_Request_ANA_CLK();
    if(receiver_test)
    {
        mAudFtm->FTM_AnaLpk_on();
        mAnaReg->SetAnaReg(AUDIO_CON3,0x01e0,0xffff);
        mAnaReg->SetAnaReg(AUDIO_CON5,0x4400,0xffff); //Enable HS + HPL buffer
        mAnaReg->SetAnaReg(AUDIO_CON1,0x0404,0xffff); //adjust HS volume

        mAsmReg->Afe_DL_Unmute(AFE_MODE_DAC);
        mAudFtm->Afe_Enable_SineWave(true);
        mAsmReg->SetAfeReg(AFE_UL_SRC_1,0x08e28e28,0xffffffff);//adjust to full swing
        Audio_Set_Earpiece_On();
    }
    else
    {
        Audio_Set_Earpiece_Off();
        mAudFtm->Afe_Enable_SineWave(false);
        mAudFtm->FTM_AnaLpk_off();
    }
    //Release Analog clock after access analog hw
    mAnaReg->AnalogAFE_Release_ANA_CLK();
    return true;
}

bool LouderSPKTest(char left_channel, char right_channel)
{
    META_LOG("LouderSPKTest left_channel=%d, right_channel=%d",left_channel,right_channel);

    int Speaker_Channel =0;
    //Request Analog clock before access analog hw
    mAnaReg->AnalogAFE_Request_ANA_CLK();
    if( left_channel == 0 && right_channel == 0)
    {
        mAudFtm->Afe_Enable_SineWave(false);
        mAudFtm->FTM_AnaLpk_off();
        Audio_Set_Speaker_Off(Channel_Stereo);
    }
    else
    {
        mAudFtm->FTM_AnaLpk_on();
        mVolumeController->ApplyGain(AUDIO_APPLY_BIG_GAIN,OUT_SPEAKER);  //apply volume
        mAsmReg->Afe_DL_Unmute(AFE_MODE_DAC);
        mAudFtm->Afe_Enable_SineWave(true);
        if(left_channel ==1 && right_channel == 1)
        {
            Audio_Set_Speaker_On(Channel_Stereo);
        }
        else if(right_channel ==1)
        {
            Audio_Set_Speaker_On(Channel_Right);
        }
        else if(left_channel == 1)
        {
            Audio_Set_Speaker_On(Channel_Left);
        }
    }
    //Release Analog clock after access analog hw
    mAnaReg->AnalogAFE_Release_ANA_CLK();
    return true;
}

bool EarphoneLoopbackTest(char bEnable)
{
    META_LOG("EarphoneLoopbackTest bEnable=%d",bEnable);
    usleep(200*1000);
    //Request Analog clock before access analog hw
    mAnaReg->AnalogAFE_Request_ANA_CLK();
    if ( (bEnable==MIC1_ON) || (bEnable==MIC2_ON) )
    {
        mAudFtm->FTM_AFE_UL_Loopback(true);
        mAudFtm->FTM_Ana_Loopback(true);
        mAnaReg->SetAnaReg(AUDIO_CON21,0x1100,0xffff); //PGA on/select
        mAnaReg->SetAnaReg(AUDIO_CON1,0x2c0c,0xffff);//mute headset buffer.
        mAnaReg->SetAnaReg(AUDIO_CON2,0x000c,0x00ff);//mute headset buffer.
        Audio_Set_HeadPhone_On(Channel_Stereo);
    }
    else
    {
        Audio_Set_HeadPhone_Off(Channel_Stereo);
        mAudFtm->FTM_AFE_UL_Loopback(false);
        mAudFtm->FTM_Ana_Loopback(false);
    }
    //Release Analog clock after access analog hw
    mAnaReg->AnalogAFE_Release_ANA_CLK();
    return true;
}

bool EarphoneTest(char bEnable)
{
    META_LOG("EarphoneTest bEnable=%d",bEnable);
    //Request Analog clock before access analog hw
    mAnaReg->AnalogAFE_Request_ANA_CLK();
    Audio_Set_Speaker_Off(Channel_Stereo);
    if(bEnable)
    {
        mLad->LAD_SetInputSource(android::LADIN_Microphone2);
        mAudFtm->FTM_AnaLpk_on();
        mVolumeController->ApplyGain(AUDIO_APPLY_BIG_GAIN,OUT_SPEAKER);  //apply volume
        mAsmReg->Afe_DL_Unmute(AFE_MODE_DAC);
        mAudFtm->Afe_Enable_SineWave(true);
        Audio_Set_HeadPhone_On(Channel_Stereo);
    }
    else
    {
        Audio_Set_HeadPhone_Off(Channel_Stereo);
        mAudFtm->Afe_Enable_SineWave(false);
        mAudFtm->FTM_AnaLpk_off();
        Audio_Set_Speaker_Off(Channel_Stereo);
    }
    //Release Analog clock after access analog hw
    mAnaReg->AnalogAFE_Release_ANA_CLK();
    return true;
}

bool FMLoopbackTest(char bEnable)
{
    META_LOG("FMLoopbackTest bEnable = %d",bEnable);

    ALOGD("FMLoopbackTest for 6575.\n");
    //Request Analog clock before access analog hw
    mAnaReg->AnalogAFE_Request_ANA_CLK();
    if(bEnable)
    {
        ::ioctl(mFd,START_DL1_STREAM,0);        // init DL1 Stream
        // enable Digital AFE
        // ...
        // enable Analog AFE
        mAnaReg->SetAnaReg(AUDIO_CON0,(0x4<<12),0xf000);
        mAnaReg->SetAnaReg(AUDIO_CON1,0x0C0C,MASK_ALL);
        mAnaReg->SetAnaReg(AUDIO_CON2,0x000C,MASK_ALL);
        mAnaReg->SetAnaReg(AUDIO_CON3,0x0070,MASK_ALL);  // enable voice buffer, audio left/right buffer
//      SetAnaReg(AUDIO_CON5,0x0220,MASK_ALL);  // FM mono playback (analog line in)
        mAnaReg->SetAnaReg(AUDIO_CON5,0x0110,MASK_ALL);  // FM stereo playback (analog line in)
#if 0
        //mAnaReg->AnalogAFE_Depop(FM_PATH_STEREO,true);
#else
        mAnaReg->SetAnaReg(AUDIO_NCP0,0x102B,0xffff);
        mAnaReg->SetAnaReg(AUDIO_NCP1,0x0600,0x0E00);
        mAnaReg->SetAnaReg(AUDIO_LDO0,0x1030,0x1fff);
        mAnaReg->SetAnaReg(AUDIO_LDO1,0x3010,0xffff);
        mAnaReg->SetAnaReg(AUDIO_LDO2,0x0013,0x0013);
        mAnaReg->SetAnaReg(AUDIO_GLB0,0x2920,0xffff);
        mAnaReg->SetAnaReg(AUDIO_GLB1,0x0000,0xffff);
        mAnaReg->SetAnaReg(AUDIO_REG1,0x0001,0x0001);

        mAnaReg->SetAnaReg(AUDIO_CON0,0x5000,0xffff);
        mAnaReg->SetAnaReg(AUDIO_CON1,0x3f0C,0x3F3F);
        mAnaReg->SetAnaReg(AUDIO_CON2,0x000C,0x003F);
        mAnaReg->SetAnaReg(AUDIO_CON3,0x0030,0x01FF);
        mAnaReg->SetAnaReg(AUDIO_CON5,0x0110,MASK_ALL);

        mAnaReg->SetAnaReg(AUDIO_CON6,0x0533,0x0FFF);
        mAnaReg->SetAnaReg(AUDIO_CON7,0x003F,0x003F);
        mAnaReg->SetAnaReg(AUDIO_CON7,0x0000,0xC000);
        mAnaReg->SetAnaReg(AUDIO_CON8,0x0000,0xffff);
        mAnaReg->SetAnaReg(AUDIO_CON9,0x0058,0xffff);
        mAnaReg->SetAnaReg(AUDIO_CON14,0x0000,0xffff);
        mAnaReg->SetAnaReg(AUDIO_CON17,0x0018,0xffff);
#endif
        mAsmReg->Afe_DL_Unmute(AFE_MODE_DAC);
        Audio_Set_Speaker_On(Channel_Stereo);
    }
    else
    {
        mAsmReg->Afe_DL_Mute(AFE_MODE_DAC);
        //mAsmReg->ForceDisableSpeaker();
        Audio_Set_Speaker_Off(Channel_Stereo);
        // disable Digital AFE
        // ...
        // disable Analog AFE
        mAnaReg->SetAnaReg(AUDIO_CON0,0,0xf000);
        mAnaReg->SetAnaReg(AUDIO_CON3,0x0000,0x00f0);  // disable voice buffer, audio left/right buffer
        mAnaReg->SetAnaReg(AUDIO_CON5,0x0440,0x0ff0);  // disable FM mono playback (analog line in)

        mAnaReg->AnalogAFE_Depop(FM_PATH_STEREO,false);

        ::ioctl(mFd,STANDBY_DL1_STREAM,0);
    }
    //Release Analog clock after access analog hw
    mAnaReg->AnalogAFE_Release_ANA_CLK();
    return true;
}

int Audio_I2S_Play(int enable_flag)
{
    ALOGD("[META] Audio_I2S_Play");
    //Request Analog clock before access analog hw
    mAnaReg->AnalogAFE_Request_ANA_CLK();
    if(enable_flag == true)
    {
        ALOGD("+Audio_I2S_Play true");
        //mAudFtm->Meta_Open_Analog(AUDIO_PATH);
        //Temp solution
        mAnaReg->SetAnaReg(AUDIO_NCP0,0x102b,0xffff);
        mAnaReg->SetAnaReg(AUDIO_NCP1,0x0600,0xffff);
        //mAnaReg->SetAnaReg(AUDIO_NCP1,0x0000,0xffff);
        mAnaReg->SetAnaReg(AUDIO_LDO0,0x1030,0xffff);
        mAnaReg->SetAnaReg(AUDIO_LDO1,0x3010,0xffff);
        mAnaReg->SetAnaReg(AUDIO_LDO2,0x0013,0xffff);
        mAnaReg->SetAnaReg(AUDIO_GLB0,0x2920,0xffff);
        mAnaReg->SetAnaReg(AUDIO_GLB1,0x0000,0xffff);
        mAnaReg->SetAnaReg(AUDIO_REG1,0x0001,0xffff);

        mAnaReg->SetAnaReg(AUDIO_CON0,0x4000,0xffff); //Set Line-in gain
        mAnaReg->SetAnaReg(AUDIO_CON1,0x0c0c,0xffff); //Set HS/HPL gain
        mAnaReg->SetAnaReg(AUDIO_CON2,0x000c,0xffff); //Set HPR gain
        mAnaReg->SetAnaReg(AUDIO_CON3,0x01f0,0xffff); //Turn-on DAC and HP buffer
        mAnaReg->SetAnaReg(AUDIO_CON5,0x4440,0xffff); //Set HS/HPL/HPR MUX
        mAnaReg->SetAnaReg(AUDIO_CON6,0x0a44,0xffff); //Set buffer Ib/Iq current(thd=-91/-88dBc@16R/32R)
        mAnaReg->SetAnaReg(AUDIO_CON7,0x003f,0xffff); //Set ZCD bias current
        mAnaReg->SetAnaReg(AUDIO_CON8,0x0000,0xffff);
        mAnaReg->SetAnaReg(AUDIO_CON14,0x00c0,0xffff); //set DAC ref. gen.
        mAnaReg->SetAnaReg(AUDIO_CON17,0x0008,0xffff); //Enable HS pull-low

        mAnaReg->SetAnaReg(AUDIO_CON10,0x01a1,0xffff); //scrambler enable
        mAnaReg->SetAnaReg(AUDIO_CON9,0x0052,0xfffe);  //AFIFO enable
        usleep(1);
        mAnaReg->SetAnaReg(AUDIO_CON9,0x0001,0x0001);  //AFIFO enable

        //~Temp solution
        mAsmReg->Afe_DL_Start(AFE_MODE_FTM_I2S);
        //Temp solution
        mAsmReg->SetAfeReg(AFE_DAC_CON0,0x00000000,0x00000008);//Disable VUL
        mAsmReg->SetAfeReg(AFE_DAC_CON1,0x00000a00,0x00000f00);//48kHz
        mAsmReg->SetAfeReg(AFE_DL_SRC2_1,0x80000000,0xf0000000);//48kHz
        mAsmReg->SetAfeReg(AFE_CONN1,0x00200000,0xffffffff);//I00_O03_S
        mAsmReg->SetAfeReg(AFE_CONN2,0x00002000,0xffffffff);//I01_O04_S
        //~Temp solution
        mAsmReg->Afe_Set_Stream_Gain(0xffff); // hardcore AFE gain
        mAsmReg->Afe_DL_Unmute(AFE_MODE_DAC);
        Audio_Set_Speaker_On(Channel_Stereo);
        ::ioctl(mFd,AUDDRV_SET_FM_I2S_GPIO,0);// enable FM use I2S
        ALOGD("-Audio_I2S_Play true");
    }
    else
    {
        ALOGD("-Audio_I2S_Play false");
        Audio_Set_Speaker_Off(Channel_Stereo);
        mAsmReg->Afe_DL_Stop(AFE_MODE_FTM_I2S);
        mAudFtm->Meta_Close_Analog();
        ::ioctl(mFd,AUDDRV_RESET_BT_FM_GPIO,0);// Reset GPIO pin mux
        ALOGD("-Audio_I2S_Play false");
    }
    //Release Analog clock after access analog hw
    mAnaReg->AnalogAFE_Release_ANA_CLK();
    return true;
}

int Audio_FMTX_Play(bool Enable, unsigned int Freq)
{
    ALOGD("Audio_FMTX_Play : Enable =%d, Freq=%d ", Enable, Freq);
    //Request Analog clock before access analog hw
    mAnaReg->AnalogAFE_Request_ANA_CLK();
    if(Enable)
    {
        mAnaReg->SetAnaReg(AUDIO_CON1, 0x000C, 0xffff);
        mAnaReg->SetAnaReg(AUDIO_CON2, 0x000C, 0xffff);
        mAnaReg->SetAnaReg(AUDIO_CON3, 0x01B0, 0xffff);
        mAsmReg->Afe_DL_Unmute(AFE_MODE_DAC);
    }
    else
    {
        mAnaReg->SetAnaReg(AUDIO_CON1, 0, 0xffff);
        mAnaReg->SetAnaReg(AUDIO_CON2, 0, 0xffff);
        mAnaReg->SetAnaReg(AUDIO_CON3, 0, 0xffff);
        mAsmReg->Afe_DL_Mute(AFE_MODE_DAC);
    }
    //Release Analog clock after access analog hw
    mAnaReg->AnalogAFE_Release_ANA_CLK();
    return mAudFtm->WavGen_SW_SineWave(Enable, Freq, 0); // 0: FM-Tx, 1: HDMI
}

bool EarphoneMicbiasEnable(bool bMicEnable)
{
    META_LOG("EarphoneMicbiasEnable bEnable = %d",bMicEnable);

    mLad->LAD_SwitchMicBias((/*int32*/signed long)bMicEnable);
    return true;
}

static int META_SetEMParameter( void *audio_par )
{
    int WriteCount = 0;
    android::SetCustParamToNV( (AUDIO_CUSTOM_PARAM_STRUCT *)audio_par);
    return WriteCount;
}

static int META_GetEMParameter( void *audio_par )
{
    int ReadConut = 0;
    android::GetCustParamFromNV( (AUDIO_CUSTOM_PARAM_STRUCT *)audio_par);

    return ReadConut;
}

static int META_SetACFParameter( void *audio_par )
{
    int WriteCount = 0;
    android::SetAudioCompFltCustParamToNV( (AUDIO_ACF_CUSTOM_PARAM_STRUCT *)audio_par);
    return WriteCount;
}

static int META_GetACFParameter( void *audio_par )
{
    int ReadConut = 0;
    android::GetAudioCompFltCustParamFromNV( (AUDIO_ACF_CUSTOM_PARAM_STRUCT *)audio_par);

    return ReadConut;
}

static int META_SetACFPreviewParameter( void *audio_par )
{
    int WriteCount = 0;
    //set to working buffer

    android::AudioSystem::SetACFPreviewParameter( (AUDIO_ACF_CUSTOM_PARAM_STRUCT *)audio_par, sizeof(AUDIO_ACF_CUSTOM_PARAM_STRUCT));
    return WriteCount;
}

static int META_SetHCFParameter( void *audio_par )
{
    int WriteCount = 0;
    android::SetHeadphoneCompFltCustParamToNV( (AUDIO_ACF_CUSTOM_PARAM_STRUCT *)audio_par);
    return WriteCount;
}

static int META_GetHCFParameter( void *audio_par )
{
    int ReadConut = 0;
    android::GetHeadphoneCompFltCustParamFromNV( (AUDIO_ACF_CUSTOM_PARAM_STRUCT *)audio_par);

    return ReadConut;
}

static int META_SetHCFPreviewParameter( void *audio_par )
{
    int WriteCount = 0;
    //set to working buffer

    android::AudioSystem::SetHCFPreviewParameter( (AUDIO_ACF_CUSTOM_PARAM_STRUCT *)audio_par, sizeof(AUDIO_ACF_CUSTOM_PARAM_STRUCT));
    return WriteCount;
}

static void META_Load_Volume(int var)
{
    android::AudioSystem::SetAudioCommand(0x50,0x0);
    return;
}


//<--- add for dual mic support on advanced meta mode
static META_BOOL SetPlaybackFile(const char *fileName)
{
    FILE *fp;
    ALOGD("SetPlaybackFile() file name %s", fileName);
    fp = fopen(fileName, "wb+");
    if (fp==NULL)
    {
        ALOGE("SetPlaybackFile() open file failed");
        return false;
    }

    fclose(fp);
    return true;
}

static META_BOOL DownloadDataToFile(const char * fileName, char *data, unsigned short size)
{
    FILE *fp;
    ALOGV("DownloadDataToFile() file name %s, data 0x%x, size %d", fileName, data, size);
    if (NULL==data || 0==size)
        return false;

    fp = fopen(fileName, "ab+");
    if (fp==NULL)
    {
        ALOGE("DownloadDataToFile() open file failed");
        return false;
    }

    if (1!=fwrite(data, size, 1, fp))
    {
        ALOGE("DownloadDataToFile failed, fwrite failed, file name:%s", fileName);
        fclose(fp);
        return false;
    }

    fclose(fp);
    return true;
}

static META_BOOL DualMICRecorder(FT_L4AUD_REQ *req, FT_L4AUD_CNF *audio_par)
{
    int ret = 0;
    char mPlaybackFileNameParam[256];
    char mRecordFileNameParam[256];
    pthread_t s_tid_audio;
    META_BOOL playbackPlusRecord = false;

    AudioSystem::setParameters(0, String8("META_SET_DUAL_MIC_FLAG=1"));

    switch (req->op)
    {
    case FT_L4AUD_OP_DUALMIC_RECORD:
    {
        sprintf(mRecordFileNameParam, "META_DUAL_MIC_OUT_FILE_NAME=%s", req->req.dualmic_record.filename);
        ALOGV("DualMICRecorder(FT_L4AUD_OP_DUALMIC_RECORD): file name parameters : %s", mRecordFileNameParam);
        AudioSystem::setParameters(0, String8(mRecordFileNameParam));
        AudioSystem::setParameters(0, String8("HQA_RDMIC_P1=3"));
        AudioSystem::setParameters(0, String8("META_DUAL_MIC_RECORD=1"));
        playbackPlusRecord = false;
        break;
    }
    case FT_L4AUD_OP_PLAYBACK_DUALMICRECORD:
    {
        sprintf(mRecordFileNameParam, "META_DUAL_MIC_OUT_FILE_NAME=%s", req->req.playback_dualmic_record.recordingFilename);
        sprintf(mPlaybackFileNameParam, "META_DUAL_MIC_IN_FILE_NAME=%s", req->req.playback_dualmic_record.playbackFilename);
        ALOGV("DualMICRecorder(FT_L4AUD_OP_PLAYBACK_DUALMICRECORD): recording file name: %s, playback file name: %s", mRecordFileNameParam, mPlaybackFileNameParam);
        AudioSystem::setParameters(0, String8(mRecordFileNameParam));
        AudioSystem::setParameters(0, String8(mPlaybackFileNameParam));
        AudioSystem::setParameters(0, String8("HQA_RDMIC_P1=3"));
        AudioSystem::setParameters(0, String8("META_DUAL_MIC_REC_PLAY=1"));
        playbackPlusRecord = true;
        break;
    }
    case FT_L4AUD_OP_PLAYBACK_DUALMICRECORD_HS:
    {
        sprintf(mRecordFileNameParam, "META_DUAL_MIC_OUT_FILE_NAME=%s", req->req.playback_dualmic_record_hs.recordingFilename);
        sprintf(mPlaybackFileNameParam, "META_DUAL_MIC_IN_FILE_NAME=%s", req->req.playback_dualmic_record_hs.playbackFilename);
        ALOGV("DualMICRecorder(FT_L4AUD_OP_PLAYBACK_DUALMICRECORD_HS): recording file name: %s, playback file name: %s", mRecordFileNameParam, mPlaybackFileNameParam);
        AudioSystem::setParameters(0, String8(mRecordFileNameParam));
        AudioSystem::setParameters(0, String8(mPlaybackFileNameParam));
        AudioSystem::setParameters(0, String8("HQA_RDMIC_P1=3"));
        AudioSystem::setParameters(0, String8("META_DUAL_MIC_REC_PLAY_HS=1"));
        playbackPlusRecord = true;
        break;
    }
    default:
        break;
    }

    ret = pthread_create(&s_tid_audio, NULL, AudioRecordControlLoop, (void *)req);
    if (ret != 0)
    {
        ALOGE("Fail to create record control thread ");
        if (playbackPlusRecord)
            AudioSystem::setParameters(0, String8("META_DUAL_MIC_REC_PLAY=0"));
        else
            AudioSystem::setParameters(0, String8("META_DUAL_MIC_RECORD=0"));

        AudioSystem::setParameters(0, String8("META_SET_DUAL_MIC_FLAG=0"));
        return false;
    }

    audio_par->cnf.dualmic_record_cnf.state = RECORD_START;
    bMetaAudioRecording = true;

    return true;

}

static META_BOOL StopDualMICRecorder()
{
    mWaitWorkCV.signal(); // signal thread to stop record
    mWaitStopCV.waitRelative(mLockStop, milliseconds(2000)); //waiting thread exit
    ALOGV("StopDualMICRecorder():Stop dual mic recording ");
    return true;
}

static META_BOOL UplinkDataToPC(ft_l4aud_ul_data_package_req &uplike_par, void *audio_par, unsigned char *pBuff)
{
    FILE *fp;
    int uplinkdatasize = 0;
    static long mCurrFilePosition = 0;
    static char mLastFileName[256] = "hello";
    ft_l4aud_ul_data_package_cnf *rec_data_cnf = (ft_l4aud_ul_data_package_cnf *)audio_par;

    if (uplike_par.size>PEER_BUF_SIZE)
    {
        ALOGE("UplinkDataToPC():required data size more than limitation, ReadSize=%d, Limiation=%d", uplike_par.size, PEER_BUF_SIZE);
        return false;
    }

    fp = fopen(uplike_par.filename, "rb+");
    if (NULL==fp)
        return false;

    if (0!=strcmp(mLastFileName, uplike_par.filename))
    {
        mCurrFilePosition = 0;
        strcpy(mLastFileName, uplike_par.filename);
        ALOGD("UplinkDataToPC():read different file, from beginning of the file ");
    }

    if (1==(uplike_par.flag&0x01))
    {
        ALOGD("UplinkDataToPC():read data from beginnig of the file ");
        mCurrFilePosition = 0;
    }

    fseek(fp, mCurrFilePosition, SEEK_SET);
    uplinkdatasize = fread(pBuff, sizeof(char), uplike_par.size, fp);
    rec_data_cnf->size = uplinkdatasize;
    rec_data_cnf->flag = 0;

    if (uplinkdatasize<uplike_par.size && !feof(fp))
    {
        fclose(fp);
        mCurrFilePosition = 0;
        return false;
    }
    else if (feof(fp))
    {
        mCurrFilePosition = 0;
        rec_data_cnf->flag = 1;
        ALOGD("UplinkDataToPC():read data to end of the file ");
    }
    else
    {
        mCurrFilePosition = ftell(fp);
    }

    fclose(fp);
    return true;
}

static META_BOOL setParameters(ft_l4aud_dualmic_set_params_req &set_par)
{
    char mParams[128];
    if (0==strlen(set_par.param))
    {
        ALOGE("parameters name is null");
        return false;
    }

    sprintf(mParams, "%s=%d", set_par.param, set_par.value);
    AudioSystem::setParameters(0, String8(mParams));
    return true;
}

static META_BOOL getParameters(ft_l4aud_dualmic_get_params_req &get_par, void *audio_par)
{
    char mParams[128];
    char *pParamName;
    char *pParamValue;
    ft_l4aud_dualmic_get_param_cnf *get_param_cnf = (ft_l4aud_dualmic_get_param_cnf *)audio_par;

    if (0==strlen(get_par.param))
    {
        ALOGE("parameters name is null");
        return false;
    }

    String8 mValue = AudioSystem::getParameters(0,String8(get_par.param));
    ALOGV("getParameters:getParameters the parameters is %s", mValue.string());
    strcpy(mParams, mValue.string());
    pParamValue = mParams;
    pParamName = strsep(&pParamValue, "=");
    if (NULL!=pParamName && NULL!=pParamValue)
    {
        strcpy(get_param_cnf->param_name, pParamName);
        get_param_cnf->value = atoi(pParamValue);
    }
    ALOGD("getParameters: param name %s, param value %d", get_param_cnf->param_name, get_param_cnf->value);
    return true;
}
//---> add for dual mic support on advanced meta mode

void META_Audio_OP(FT_L4AUD_REQ *req, char *peer_buff, unsigned short peer_len)
{
    META_BOOL ret = true;
    unsigned char pBuff[PEER_BUF_SIZE];
    unsigned short mReadSize = 0;
    FT_L4AUD_CNF audio_cnf;
    memset(&audio_cnf, 0, sizeof(FT_L4AUD_CNF));
    audio_cnf.header.id = FT_L4AUD_CNF_ID;
    audio_cnf.header.token = req->header.token;
    audio_cnf.op = req->op;
    audio_cnf.status = META_SUCCESS;

    if(bMetaAudioInited == FALSE)
    {
        META_LOG("META_Audio_OP not initialed \r");
        audio_cnf.status = META_FAILED;
        WriteDataToPC(&audio_cnf, sizeof(FT_L4AUD_CNF), NULL, 0);
    }

    META_LOG("META_Audio_OP req->op=%d \r",req->op);

    switch(req->op)
    {
    case FT_L4AUD_OP_SET_PARAM_SETTINGS_0809:
    {
        ft_l4aud_set_param_0809 *par;
        META_LOG("META_Audio_OP, Audio Set Param Req \r");

        META_SetEMParameter((void *)&req->req );
        par = (ft_l4aud_set_param_0809 *)&req->req;
        break;
    }

    case FT_L4AUD_OP_GET_PARAM_SETTINGS_0809:
    {
        ft_l4aud_set_param_0809 *par;
        META_LOG("META_Audio_OP, Audio Get Param Req\r\n");

        META_GetEMParameter( (void *)&audio_cnf.cnf );
        par = (ft_l4aud_set_param_0809 *)&audio_cnf.cnf;
        break;
    }
    case FT_L4AUD_OP_SET_ACF_COEFFS:
    {
        ft_l4aud_set_acf_param_req *par;
        META_LOG("META_Audio_OP, Audio Set ACF Param Req \r");

        META_SetACFParameter((void *)&req->req );
        par = (ft_l4aud_set_acf_param_req *)&req->req;
        break;
    }

    case FT_L4AUD_OP_GET_ACF_COEFFS:
    {
        ft_l4aud_get_acf_param_cnf *par;
        META_LOG("META_Audio_OP, Audio Get ACF Param Req\r\n");

        META_GetACFParameter( (void *)&audio_cnf.cnf );
        par = (ft_l4aud_get_acf_param_cnf *)&audio_cnf.cnf;
        break;
    }
    case FT_L4AUD_OP_SET_PREVIEW_ACF_COEFFS:
    {
        ft_l4aud_set_acf_param_req *par;
        META_LOG("META_Audio_OP, Audio Set ACF Preview Param Req\r\n");

        META_SetACFPreviewParameter( (void *)&req->req );
        par = (ft_l4aud_set_acf_param_req *)&req->req;
        break;
    }
    case FT_L4AUD_OP_SET_HCF_COEFFS:
    {
        ft_l4aud_set_acf_param_req *par;
        META_LOG("META_Audio_OP, Audio Set HCF Param Req \r");

        META_SetHCFParameter((void *)&req->req );
        par = (ft_l4aud_set_acf_param_req *)&req->req;
        break;
    }

    case FT_L4AUD_OP_GET_HCF_COEFFS:
    {
        ft_l4aud_get_acf_param_cnf *par;
        META_LOG("META_Audio_OP, Audio Get HCF Param Req\r\n");

        META_GetHCFParameter( (void *)&audio_cnf.cnf );
        par = (ft_l4aud_get_acf_param_cnf *)&audio_cnf.cnf;
        break;
    }
    case FT_L4AUD_OP_SET_PREVIEW_HCF_COEFFS:
    {
        ft_l4aud_set_acf_param_req *par;
        META_LOG("META_Audio_OP, Audio Set HCF Preview Param Req\r\n");

        META_SetHCFPreviewParameter( (void *)&req->req );
        par = (ft_l4aud_set_acf_param_req *)&req->req;
        break;
    }

    case FT_L4AUD_OP_SET_ECHO:
    {
        META_LOG("META_Audio_OP, Loopback test \r\n");
        ft_l4aud_set_echo *par;
        par = (ft_l4aud_set_echo *)&req->req;
        RecieverLoopbackTest(par->echoflag);
        break;
    }
    case FT_L4AUD_OP_MIC2_LOOPBACK:
    {
        ALOGD("META_Audio_OP, MIC2 Loopback test \r\n");
        ft_l4aud_set_echo *par;
        par = (ft_l4aud_set_echo *)&req->req;
        RecieverLoopbackTest_Mic2(par->echoflag);
        break;
    }
    case FT_L4AUD_OP_RECEIVER_TEST:
    {
        META_LOG("META_Audio_OP, Receiver test \r\n");
        ft_l4aud_receiver_test *par;
        par = (ft_l4aud_receiver_test *)&req->req;
        RecieverTest(par->receiver_test);
        break;
    }
    case FT_L4AUD_OP_LOUDSPK_TEST:
    {
        META_LOG("META_Audio_OP, LoudSpk test \r\n");
        ft_l4aud_loudspk *par;
        par = (ft_l4aud_loudspk *)&req->req;
        LouderSPKTest(par->left_channel, par->right_channel);
        break;
    }
    case FT_L4AUD_OP_EARPHONE_TEST:
    {
        META_LOG("META_Audio_OP, Earphone test \r\n");
        EarphoneTest(req->req.eaphone_test.bEnable);
        break;
    }

    case FT_L4AUD_OP_HEADSET_LOOPBACK_TEST:
    {
        META_LOG("META_Audio_OP, Headset loopback test \r\n");
        EarphoneLoopbackTest(req->req.headset_loopback_test.bEnable);
        break;
    }

    case FT_L4AUD_OP_FM_LOOPBACK_TEST:
    {
        META_LOG("META_Audio_OP, FM loopback test \r\n");
        // Need to check FM function is ready
//			 FMLoopbackTest(req->req.fm_loopback_test.bEnable);
        break;
    }

    case FT_L4AUD_OP_SET_PLAYBACK_FILE:
    {
        META_LOG("META_Audio_OP, set playback file \r\n");
        ret = SetPlaybackFile(req->req.dl_playback_file.filename);
        break;
    }

    case FT_L4AUD_OP_DL_DATA_PACKAGE:
    {
        META_LOG("META_Audio_OP, down link data pakage \r\n");
        ret = DownloadDataToFile(req->req.dl_data_package.filename, peer_buff, peer_len);
        break;
    }

    case FT_L4AUD_OP_DUALMIC_RECORD:
    {
        META_LOG("META_Audio_OP, dual mic recording \r\n");
        ret = false;
        if (!bMetaAudioRecording)
            ret = DualMICRecorder(req, &audio_cnf);
        break;
    }

    case FT_L4AUD_OP_PLAYBACK_DUALMICRECORD:
    {
        META_LOG("META_Audio_OP, playback and dual mic recording \r\n");
        ret = false;
        if (!bMetaAudioRecording)
            ret = DualMICRecorder(req, &audio_cnf);

        break;
    }

    case FT_L4AUD_OP_PLAYBACK_DUALMICRECORD_HS:
    {
        META_LOG("META_Audio_OP, headset playback and dual mic recording \r\n");
        ret = false;
        if (!bMetaAudioRecording)
            ret = DualMICRecorder(req, &audio_cnf);

        break;
    }

    case FT_L4AUD_OP_STOP_DUALMIC_RECORD:
    {
        META_LOG("META_Audio_OP, stop dual mic recording \r\n");
        ret = false;
        if (bMetaAudioRecording)
            ret = StopDualMICRecorder();

        break;
    }

    case FT_L4AUD_OP_UL_DATA_PACKAGE:
    {
        META_LOG("META_Audio_OP, uplink data package \r\n");
        ret = false;
        if (PEER_BUF_SIZE>=req->req.ul_data_package.size)
        {
            ret = UplinkDataToPC(req->req.ul_data_package, (void *)&audio_cnf.cnf, pBuff);
            mReadSize = ret==true?audio_cnf.cnf.ul_data_package_cnf.size:0;
            ALOGV("uplink data package  size = %d", mReadSize);
            ALOGV("uplink data package  flag = %d", audio_cnf.cnf.ul_data_package_cnf.flag);
        }

        break;
    }

    case FT_L4AUD_OP_DUALMIC_SET_PARAMS:
    {
        META_LOG("META_Audio_OP, set parameters \r\n");
        setParameters(req->req.dualmic_set_params);
        break;
    }

    case FT_L4AUD_OP_DUALMIC_GET_PARAMS:
    {
        META_LOG("META_Audio_OP, get parameters \r\n");
        getParameters(req->req.dualmic_get_params, (void *)&audio_cnf.cnf);
        break;
    }

    case FT_L4AUD_OP_LOAD_VOLUME:
    {
        META_Load_Volume(NULL);
        META_LOG("META_Audio_OP, FT_L4AUD_OP_LOAD_VOLUME\r\n");
        ret = true;
        break;
    }
      case FT_L4AUD_OP_GET_GAINTABLE_SUPPORT:{
         META_LOG("META_Audio_OP, FT_L4AUD_OP_GET_GAINTABLE_SUPPORT \r\n");
         ft_l4aud_gaintable_cnt *par;
         par = (ft_l4aud_gaintable_cnt *)&audio_cnf.cnf;
         #ifdef MTK_AUDIO_GAIN_TABLE
         par->bEnable = true;
         #else
         par->bEnable = false;
         #endif
         ret = true;
         break;
      }
      case FT_L4AUD_OP_GET_GAINTABLE_NUM:
      {
         META_LOG("META_Audio_OP, FT_L4AUD_OP_GET_GAINTABLE_NUM \r\n");
         ft_l4aud_gaintablenum_cnf *par;
         par = (ft_l4aud_gaintablenum_cnf *)&audio_cnf.cnf;
         par->gaintablenum =NUM_AUDIO_GAIN_TYPES ;
         ret = true;
         break;
      }
      case FT_L4AUD_OP_GET_GAINTABLE_LEVEL:
      {
         META_LOG("META_Audio_OP, FT_L4AUD_OP_GET_GAINTABLE_LEVEL \r\n");
         ft_l4aud_gaintablelevel_req *par;
         ft_l4aud_gaintablelevel_cnf  *cnf1;
         par = (ft_l4aud_gaintablelevel_req *)&req->req;
         cnf1 = (ft_l4aud_gaintablelevel_cnf *)&audio_cnf.cnf;
         cnf1->gaintabletype = par->gaintabletype;
         switch(par->gaintabletype){
             case AUDIO_GAIN_VOICE_CALL:
                 cnf1->gainttablelevel =VOICE_GAIN_TABLE_LEVEL;
                 break;
             case AUDIO_GAIN_SYSTEM:
                 cnf1->gainttablelevel =SYSTEM_GAIN_TABLE_LEVEL;
                 break;
             case AUDIO_GAIN_RING:
                 cnf1->gainttablelevel =RING_GAIN_TABLE_LEVEL;
                 break;
             case AUDIO_GAIN_MUSIC:
                 cnf1->gainttablelevel =MUSIC_GAIN_TABLE_LEVEL;
                 break;
             case AUDIO_GAIN_ALARM:
                 cnf1->gainttablelevel =ALARM_GAIN_TABLE_LEVEL;
                 break;
             case AUDIO_GAIN_NOTIFICATION:
                 cnf1->gainttablelevel =NOTIFICATION_GAIN_TABLE_LEVEL;
                 break;
             case AUDIO_GAIN_BLUETOOTH_SCO:
                 cnf1->gainttablelevel =BLUETOOTHSCO_GAIN_TABLE_LEVEL;
                 break;
             case AUDIO_GAIN_ENFORCED_AUDIBLE:
                 cnf1->gainttablelevel =ENFORCEAUDIBLE_GAIN_TABLE_LEVEL;
                 break;
             case AUDIO_GAIN_DTMF:
                 cnf1->gainttablelevel =DTMF_GAIN_TABLE_LEVEL;
                 break;
             case AUDIO_GAIN_TTS:
                 cnf1->gainttablelevel =TTS_GAIN_TABLE_LEVEL;
                 break;
             case AUDIO_GAIN_FM:
                 cnf1->gainttablelevel =FM_GAIN_TABLE_LEVEL;
                 break;
             case AUDIO_GAIN_SPEECH:
                 cnf1->gainttablelevel =SPEECH_GAIN_TABLE_LEVLE;
                 break;
             default:
                 break;
         }
         ret = true;
         break;
      }
      case FT_L4AUD_OP_GET_CTRPOINT_NUM:{
          META_LOG("META_Audio_OP, FT_L4AUD_OP_GET_CTRPOINT_NUM \r\n");
          ft_l4aud_ctrpointnum_cnt *par;
          par = (ft_l4aud_ctrpointnum_cnt *)&audio_cnf.cnf;
          if(mMetaAuioDevice != NULL)
          {
               par->i4ctrpointnum = mMetaAuioDevice->getParameters(AUD_AMP_GET_CTRP_NUM, 0,NULL);
          }
          else
          {
               int *p = &(par->i4ctrpointnum);
               android::AudioSystem::GetAudioData(GET_GAIN_TABLE_CTRPOINT_NUM,sizeof(p), p);
          }
          ret = true;
          break;
      }
      case FT_L4AUD_OP_GET_CTRPOINT_BITS:{
          META_LOG("META_Audio_OP, FT_L4AUD_OP_GET_CTRPOINT_BITS \r\n");
          ft_l4aud_ctrpointbits_req *par;
          ft_l4aud_ctrpointbits_cnt  *cnf1;
          par = (ft_l4aud_ctrpointbits_req *)&req->req;
          cnf1 = (ft_l4aud_ctrpointbits_cnt *)&audio_cnf.cnf;
          cnf1->i4ctrpoint = par->i4ctrpoint;
          if(mMetaAuioDevice != NULL)
          {
               cnf1->i4ctrpointbits = mMetaAuioDevice->getParameters(AUD_AMP_GET_CTRP_BITS, par->i4ctrpoint,NULL);
          }
          else
          {   
               int data[2] = {par->i4ctrpoint,cnf1->i4ctrpointbits};
               android::AudioSystem::GetAudioData(GET_GAIN_TABLE_CTRPOINT_BITS,sizeof(data),data);
               cnf1->i4ctrpointbits = data[1];
          }
          ret = true;
          break;
      }
      case FT_L4AUD_OP_GET_CTRPOINT_TABLE:{
          META_LOG("META_Audio_OP, FT_L4AUD_OP_GET_CTRPOINT_TABLE \r\n");
          ft_l4aud_ctrpointtable_req *par;
          ft_l4aud_ctrpointtable_cnt  *cnf1;
          par = (ft_l4aud_ctrpointtable_req *)&req->req;
          cnf1 = (ft_l4aud_ctrpointtable_cnt *)&audio_cnf.cnf;

          // first check if bits equal to buffer length , have buffer to copy.
          cnf1->i4ctrpoint = par->i4ctrpoint;
          int controlpointbits = 0;
          if(mMetaAuioDevice != NULL)
          {
               controlpointbits = mMetaAuioDevice->getParameters(AUD_AMP_GET_CTRP_BITS, par->i4ctrpoint,NULL);
          }
          else
          {   
              int data[2] = {par->i4ctrpoint,controlpointbits};
               android::AudioSystem::GetAudioData(GET_GAIN_TABLE_CTRPOINT_BITS,sizeof(data),data);
               controlpointbits = data[1];
          }
          if((1<<controlpointbits) >= cnf1->i4bufferlength){
              if(mMetaAuioDevice != NULL)
              {
                   mMetaAuioDevice->getParameters (AUD_AMP_GET_CTRP_TABLE, par->i4ctrpoint,cnf1->buffer);
              }
              else
              {   
                   char data[255] = {0};
                   data[0] = par->i4ctrpoint;
                   android::AudioSystem::GetAudioData(GET_GAIN_TABLE_CTRPOINT_TABLE,sizeof(data),data);
                   memcpy(cnf1->buffer,data,sizeof(cnf1->buffer));
              }
              cnf1->i4bufferlength = (1<<controlpointbits);
              ret = true;
          }
          break;
      }
      case FT_L4AUD_OP_GET_HDREC_INFO:{
          META_LOG("META_Audio_OP, FT_L4AUD_OP_GET_HDREC_INFO \r\n");
          ft_l4aud_hdrec_info_cnf *par;
          par = (ft_l4aud_hdrec_info_cnf *)&audio_cnf.cnf;

#if defined(MTK_AUDIO_HD_REC_SUPPORT)
          par->bHdRecSupport= true;
#else
          par->bHdRecSupport = false;
#endif

#if defined(MTK_DUAL_MIC_SUPPORT)
          par->bDualMicSupport = true;
#else
          par->bDualMicSupport = false;
#endif
          ret = true;

          break;
      }

    default:
        audio_cnf.status = META_FAILED;
        break;
    }

    if (!ret)
        audio_cnf.status = META_FAILED;

    META_LOG("META_Audio_OP, audio_cnf.status = %d \r", audio_cnf.status);
    WriteDataToPC(&audio_cnf, sizeof(FT_L4AUD_CNF), pBuff, mReadSize);

}

#else  // #ifndef GENERIC_AUDIO

void META_Audio_OP(FT_L4AUD_REQ *req, char *peer_buff, unsigned short peer_len)
{
    META_LOG("META_Audio_OP with generic Audio , no test");
}

bool META_Audio_init(void)
{

    META_LOG("META_Audio_init");
    return true;
}

bool META_Audio_deinit()
{
    META_LOG("META_Audio_deinit");
    return true;
}

bool RecieverLoopbackTest(char echoflag)
{
    return true;
}

bool RecieverTest(char receiver_test)
{
    return true;
}

bool LouderSPKTest(char left_channel, char right_channel)
{
    return true;
}

bool EarphoneLoopbackTest(char bEnable)
{
    return true;
}

bool EarphoneTest(char bEnable)
{
    return true;
}

bool FMLoopbackTest(char bEnable)
{
    return true;
}

bool EarphoneMicbiasEnable(bool bMicEnable)
{
    return true;
}

int Audio_I2S_Play(int enable_flag)
{
    return true;
}

int Audio_FMTX_Play(bool Enable, unsigned int Freq)
{
    return true;
}

#endif  // #ifndef GENERIC_AUDIO



