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
 * AudioATCommand.cpp
 *
 * Project:
 * --------
 *   Android + MT6573
 *
 * Description:
 * ------------
 *   This file implements ATCommand driver for audio.
 *
 * Author:
 * -------
 *   Tina Tsai (mtk01981)
 *
 *------------------------------------------------------------------------------
 * $Revision: #1 $
 * $Modtime:$
 * $Log:$
 *
 *******************************************************************************/
#include <utils/Log.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <AudioATCommand.h>

#if defined(EVDO_DT_SUPPORT)
#include <dlfcn.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>
#include "AudioYusuLad.h"
#endif

#define LOG_TAG                                    "AudioATCommand"
#define BUFFER_SIZE                                512

/* MT6252 */
#define DEVICE_NAME                                "/dev/ptty3audio"
#define AT_COMMAND_SPEECH_ON                       "AT+ESPEECH=1, "
#define AT_COMMAND_SPEECH_OFF                      "AT+ESPEECH=0"
#define AT_COMMAND_SIDETONE_VOLUME                 "AT+ESSTV="
#define AT_COMMAND_DEFAULT_TONE_PLAY               "AT+EDTP="
#define AT_COMMAND_DEFAULT_TONE_STOP               "AT+EDTS"
#define AT_COMMAND_INPUT_SOURCE                    "AT+ESETDEV=0, "
#define AT_COMMAND_OUTPUT_DEVICE                   "AT+ESETDEV=1, "
#define AT_COMMAND_OUTPUT_VOLUME                   "AT+ESOV="
#define AT_COMMAND_MICROPHONE_VOLUME               "AT+ESMV="
#define AT_COMMAND_SPEECH_MODE_ADAPTATION          "AT+ESSMA="

#define EVDO_REC_SR 8000 //8kHz sampling rate
#define EVDO_REC_CH 1    //mono

/*
   int main(int argc, char **argv)
   {
   ALOGD("%s\n", __FUNCTION__);
  creg();
  L1SP_Speech_On();
  L1SP_SetSpeechMode(1);
  L1SP_SetOutputDevice(2);
  L1SP_SetInputSource(3);
  AFE_SetOutputVolume(32767, 2);
  AFE_SetMicrophoneVolume(10);
  Default_TONE_Play(1);
  L1SP_Speech_Off();
  return 0;
   }
 */
namespace android
{

#if defined(EVDO_DT_SUPPORT)

/**
 * queue add/get
 */
#define ATCMDQ_MAXSIZE 32

typedef enum e_atcmd_req_type {
	REQ_CLEAR=0,
	REQ_AUDIO_MODE=168, /* RIL_REQUEST_SET_AUDIO_PATH=168 */
	REQ_MUTE=53, /* RIL_REQUEST_SET_MUTE=53 */
	REQ_AUDIO_VOLUME=184, /* RIL_REQUEST_SET_VOICE_VOLUME=140->184, new add */
	REQ_PLAY_DTMF=185, /* #define RIL_REQUEST_PLAY_DTMF_TONE  185 */
	REQ_PLAY_TONE_SEQ=186  /* #define RIL_REQUEST_PLAY_TONE_SEQ 186 */
} atcmd_req_type_e;

typedef struct s_atcmd_req {
	int type;
	int arg1;
	int arg2;
} atcmd_req_t;

typedef struct s_atcmdq {
	atcmd_req_t cmd[ATCMDQ_MAXSIZE];
	unsigned long head;
	unsigned long tail;
	int full;
	pthread_mutex_t mutex;
	sem_t sem;
} atcmdq_t;

static atcmdq_t atcmdq;
int atcmd_init();

int atcmdq_add(atcmd_req_t *cmd)
{
	atcmdq_t *q=&atcmdq;
	unsigned long i, found=0;

	atcmd_init();

	pthread_mutex_lock(&atcmdq.mutex);

	ALOGD("+atcmdq_add: index=%lu type:%d args1=%d arg2=%d\n", q->tail, cmd->type, cmd->arg1, cmd->arg2);
	if(q->full==1){
		ALOGD("atcmdq_add: q is full");
		sem_post(&atcmdq.sem);
		pthread_mutex_unlock(&atcmdq.mutex);
		return -1;
	}

	/* take out the same type comment before: simple, assumption qlen is short
	   this method will impact the command order (not good), marked REQ_CLEAR and then add, avoid the stress test
	   order, does not mater, replace so far. at least, 3 case
	*/
	found=0;
	for(i=q->head;i<q->tail;i++) {
		if(q->cmd[i].type==cmd->type) {
			found=1;
			q->cmd[i].type=cmd->type;
			q->cmd[i].arg1=cmd->arg1;
			q->cmd[q->tail].arg2=cmd->arg2;
		}
	}
	if(found!=1) {
		q->cmd[q->tail].type=cmd->type;
		q->cmd[q->tail].arg1=cmd->arg1;
		q->cmd[q->tail].arg2=cmd->arg2;
		q->tail++;
		sem_post(&atcmdq.sem);
	}

	if(q->tail==ATCMDQ_MAXSIZE){
		q->tail=0;
	}
	if(q->tail==q->head){
		q->full=1;
	}
	pthread_mutex_unlock(&atcmdq.mutex);

	ALOGD("-atcmdq_add\n");
	return 0;
}

int atcmdq_get(atcmd_req_t *cmd)
{
	atcmdq_t *q=&atcmdq;

	sem_wait(&atcmdq.sem);
	pthread_mutex_lock(&atcmdq.mutex);
	if(q->head==q->tail) {
		ALOGD("atcmdq_get: q is empty");
		pthread_mutex_unlock(&atcmdq.mutex);
		return -1;
	}

	cmd->type=q->cmd[q->head].type;
	cmd->arg1=q->cmd[q->head].arg1;
	cmd->arg2=q->cmd[q->head].arg2;
	ALOGD("+atcmdq_get: index=%lu type:%d args1=%d arg2=%d)\n", q->head, cmd->type, cmd->arg1, cmd->arg2);
	q->head++;
	if(q->head==ATCMDQ_MAXSIZE){
		q->head=0;
	}
	q->full=0;
	pthread_mutex_unlock(&atcmdq.mutex);

	return 0;
}

/**
 * @brief this function reference CDMA alsa example code
 */
static pthread_mutex_t cpaudiopath_Mutex = PTHREAD_MUTEX_INITIALIZER;
#define RPC_RIL_PATH "/system/lib/librpcril.so"
#define RIL_REQUEST_SET_AUDIO_PATH  168
#define RIL_REQUEST_SET_MUTE  53
#define RIL_REQUEST_SET_VOICE_VOLUME 184

int setCPAudioPath(int voice_type)
{
	int ret;
	int s_fdListen=-1;
	int *dlHandle;
	int (*rpcrequest)(int, int);
	const char *rpcrilLibPath;
	pthread_mutex_lock(&cpaudiopath_Mutex);
	rpcrilLibPath=RPC_RIL_PATH;
	dlHandle=(int*)dlopen(rpcrilLibPath, RTLD_NOW);
	if(dlHandle==NULL) {
		ALOGD("dlopen fail: %s\n", dlerror());
		pthread_mutex_unlock(&cpaudiopath_Mutex);
		return -1;
	}
	rpcrequest=(int (*)(int, int))dlsym(dlHandle, "sendRpcRequest");
	if(NULL==rpcrequest) {
		ALOGE("can not find function sendRpcRequest");
		dlclose(dlHandle);
		pthread_mutex_unlock(&cpaudiopath_Mutex);
		return -1;
	}
	ret=rpcrequest(RIL_REQUEST_SET_AUDIO_PATH, voice_type);
	if(dlHandle) {
		dlclose(dlHandle);
	}
	pthread_mutex_unlock(&cpaudiopath_Mutex);

	return ret;
}

int setCPAudioMute(int enable)
{
	int ret;
	int s_fdListen=-1;
	int *dlHandle;
	int (*rpcrequest)(int, int);
	const char *rpcrilLibPath;
	pthread_mutex_lock(&cpaudiopath_Mutex);
	rpcrilLibPath=RPC_RIL_PATH;
	dlHandle=(int*)dlopen(rpcrilLibPath, RTLD_NOW);
	if(dlHandle==NULL) {
		ALOGD("dlopen fail: %s\n", dlerror());
		pthread_mutex_unlock(&cpaudiopath_Mutex);
		return -1;
	}
	rpcrequest=(int (*)(int, int))dlsym(dlHandle, "sendRpcRequest");
	if(NULL==rpcrequest) {
		ALOGE("can not find function sendRpcRequest");
		dlclose(dlHandle);
		pthread_mutex_unlock(&cpaudiopath_Mutex);
		return -1;
	}
	ret=rpcrequest(RIL_REQUEST_SET_MUTE, enable);
	if(dlHandle) {
		dlclose(dlHandle);
	}
	pthread_mutex_unlock(&cpaudiopath_Mutex);

	return ret;
}

int setCPAudioVolume(int level)
{
	int ret;
	int s_fdListen=-1;
	int *dlHandle;
	int (*rpcrequest)(int, int);
	const char *rpcrilLibPath;
	pthread_mutex_lock(&cpaudiopath_Mutex);
	rpcrilLibPath=RPC_RIL_PATH;
	dlHandle=(int*)dlopen(rpcrilLibPath, RTLD_NOW);
	if(dlHandle==NULL) {
		ALOGD("dlopen fail: %s\n", dlerror());
		pthread_mutex_unlock(&cpaudiopath_Mutex);
		return -1;
	}
	rpcrequest=(int (*)(int, int))dlsym(dlHandle, "sendRpcRequest");
	if(NULL==rpcrequest) {
		ALOGE("can not find function sendRpcRequest");
		dlclose(dlHandle);
		pthread_mutex_unlock(&cpaudiopath_Mutex);
		return -1;
	}
	ret=rpcrequest(RIL_REQUEST_SET_VOICE_VOLUME, level);
	if(dlHandle) {
		dlclose(dlHandle);
	}
	pthread_mutex_unlock(&cpaudiopath_Mutex);

	return ret;
}

int setCPAudioPlayDtmf(int mode, int dtmf)
{
		int ret;
		int s_fdListen=-1;
		int *dlHandle;
		int (*rpcrequest)(int, int, int, void *, int, int *, void ***);
		const char *rpcrilLibPath;
		int outLen = 0;
		void **outValue = NULL;

		int dtmfData[4] ={ 0 };

		pthread_mutex_lock(&cpaudiopath_Mutex);
		rpcrilLibPath=RPC_RIL_PATH;
		dlHandle=(int*)dlopen(rpcrilLibPath, RTLD_NOW);
		if(dlHandle==NULL) {
		ALOGD("dlopen fail: %s\n", dlerror());
				pthread_mutex_unlock(&cpaudiopath_Mutex);
				return -1;
		}
		rpcrequest=(int (*)(int, int, int, void *, int, int *, void ***))dlsym(dlHandle, "sendRpcRequestComm");
		if(NULL==rpcrequest) {
		ALOGE("can not find function sendRpcRequestComm");
				dlclose(dlHandle);
				pthread_mutex_unlock(&cpaudiopath_Mutex);
				return -1;
		}

		/*
		   typedef enum {
		   RIL_RPC_PARA_INTS = 0,
		   RIL_RPC_PARA_STRING = 1,
		   RIL_RPC_PARA_STRINGS  = 2,
		   RIL_RPC_PARA_RAW = 3,
		   RIL_RPC_PARA_NULL = 4,
		   RIL_RPC_PARA_RESERVE,
		   } RIL_RPC_ParaTypes;
		   int sendRpcRequestComm(int request_num, RIL_RPC_ParaTypes in_types, int in_len, void *in_value, RIL_RPC_ParaTypes out_types, int *out_len, void ***out_value);
		 */
		dtmfData[0] = mode;	// mode:1 play, mode:0 stop
		dtmfData[1] = dtmf; // dtmf
		dtmfData[2] = 5;	// volume
		dtmfData[3] = 0;	// duration
		ret=rpcrequest(REQ_PLAY_DTMF, 0, sizeof(dtmfData)/sizeof(int), dtmfData, 4, &outLen, &outValue);
		if(dlHandle) {
				dlclose(dlHandle);
		}
		pthread_mutex_unlock(&cpaudiopath_Mutex);

		return ret;
}

enum e_tone_spec {
	TONE_MMS_NOTIFICATION=12,
	TONE_LOW_BATTERY_NOTIFICATION=13,
	TONE_CALL_WAITING_INDICATION=14,
	TONE_CALL_REMINDER_INDICATION=15
};

typedef struct s_tone_freq {
	int duration; /* # of 20 msec frames  */
	int freq1; /* Dual Tone Frequencies in Hz; for single tone, set 2nd freq to zero */
	int freq2;
} tone_freq_t;

typedef struct s_tone_spec {
	int id;
	int num;
	int iteration;
	int volume;
	tone_freq_t tone1;
	tone_freq_t tone2;
	tone_freq_t tone3;
	tone_freq_t tone4;
} TONE_SPEC_T;

TONE_SPEC_T tone_spec[] = {
	{TONE_MMS_NOTIFICATION, 1, 0, 5, {10, 852, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
	{TONE_LOW_BATTERY_NOTIFICATION, 3, 0, 5, {15, 1100, 0}, {5, 0, 0}, {15, 900, 0}, {0, 0, 0}},
	{TONE_CALL_WAITING_INDICATION, 4, 0, 5, {10, 440, 0}, {5, 0, 0}, {10, 400, 0}, {175, 0, 0}},
	{TONE_CALL_REMINDER_INDICATION, 4, 1, 5, {10, 440, 0}, {5, 0, 0}, {10, 400, 0}, {175, 0, 0}}
};

int setCPAudioPlayTone(int toneId)
{
		int ret;
		int s_fdListen=-1;
		int *dlHandle;
		int (*rpcrequest)(int, int, int, void *, int, int *, void ***);
		const char *rpcrilLibPath;
		int outLen = 0;
		void **outValue = NULL;
	unsigned int i;
		TONE_SPEC_T *tone;

		pthread_mutex_lock(&cpaudiopath_Mutex);

		tone=0;
		for(i=0;i<(sizeof(tone_spec)/sizeof(TONE_SPEC_T));i++) {
			if(tone_spec[i].id==toneId) {
				tone=&tone_spec[i];
			}
		}
		if(tone==0) {
		ALOGD("setCPAudioPlayTone: tone:%d is not support", toneId);
			pthread_mutex_unlock(&cpaudiopath_Mutex);
			return -1;
		}

		rpcrilLibPath=RPC_RIL_PATH;
		dlHandle=(int*)dlopen(rpcrilLibPath, RTLD_NOW);
		if(dlHandle==NULL) {
		ALOGD("dlopen fail: %s\n", dlerror());
				pthread_mutex_unlock(&cpaudiopath_Mutex);
				return -1;
		}
		rpcrequest=(int (*)(int, int, int, void *, int, int *, void ***))dlsym(dlHandle, "sendRpcRequestComm");
		if(NULL==rpcrequest) {
		ALOGE("can not find function sendRpcRequestComm");
				dlclose(dlHandle);
				pthread_mutex_unlock(&cpaudiopath_Mutex);
				return -1;
		}

		/*
		   typedef enum {
		   RIL_RPC_PARA_INTS = 0,
		   RIL_RPC_PARA_STRING = 1,
		   RIL_RPC_PARA_STRINGS  = 2,
		   RIL_RPC_PARA_RAW = 3,
		   RIL_RPC_PARA_NULL = 4,
		   RIL_RPC_PARA_RESERVE,
		   } RIL_RPC_ParaTypes;
		   int sendRpcRequestComm(int request_num, RIL_RPC_ParaTypes in_types, int in_len, void *in_value, RIL_RPC_ParaTypes out_types, int *out_len, void ***out_value);
		 */
		ret=rpcrequest(REQ_PLAY_TONE_SEQ, 0, (sizeof(TONE_SPEC_T)/sizeof(int))-1, &(tone->num), 4, &outLen, &outValue);
		if(dlHandle) {
				dlclose(dlHandle);
		}
		pthread_mutex_unlock(&cpaudiopath_Mutex);

		return ret;
}

static void* atcmd_main(void *arg)
{
	atcmd_req_t cmd;
	int result;

	while(1) {
		//ALOGD("atcmd_main: wait\n");
		result=atcmdq_get(&cmd);
		ALOGD("atcmd_main: got it, result=%d cmd(%d, %d, %d)\n", result, cmd.type, cmd.arg1, cmd.arg2);
		if(result >= 0) {
			switch(cmd.type) {
				case REQ_AUDIO_MODE:
				{
					/* AT command to MD firmware */
					result=setCPAudioPath(cmd.arg1);
					if(result<0) {
							ALOGD("setCPAudioPath: REQ_AUDIO_MODE mode=%d\n", cmd.arg1);
					}
					break;
				}
				case REQ_MUTE:
				{
					result=setCPAudioMute(cmd.arg1);
					if(result<0) {
							ALOGD("setCPAudioMute: REQ_AUDIO_MODE enable=%d\n", cmd.arg1);
					}
					break;
				}
				case REQ_AUDIO_VOLUME:
				{
					result=setCPAudioVolume(cmd.arg1);
					if(result<0) {
							ALOGD("setCPAudioVolume: error REQ_AUDIO_VOLUME vol=%d result=%d\n", cmd.arg1, result);
					}
					break;
				}
				case REQ_PLAY_DTMF:
				{
					result=setCPAudioPlayDtmf(cmd.arg1, cmd.arg2);
					if(result<0) {
							ALOGD("setCPAudioPlayDtmf: error REQ_PLAY_DTMF mode=%d toneId=%d result=%d\n", cmd.arg1, cmd.arg2, result);
					}
					break;
				}
				case REQ_PLAY_TONE_SEQ:
				{
					result=setCPAudioPlayTone(cmd.arg1);
					if(result<0) {
							ALOGD("setCPAudioPlayTone: error REQ_PLAY_TONE_SEQ toneId=%d result=%d\n", cmd.arg1, result);
					}
					break;
				}
				default:
					ALOGD("atcmd_main: cmd:%d not support\r\n", cmd.type);
					break;
			}
		}
	}
	return 0;
}

int atcmd_init()
{
	static int init=0;
	pthread_attr_t attr;
	pthread_t atcmd_task;

	if(init==1) {
		return 0;
	}

	ALOGD("+atcmd: init\n");
	/* atcmdq init */
	atcmdq.tail=0;
	atcmdq.head=0;
	atcmdq.full=0;
	pthread_mutex_init(&atcmdq.mutex, NULL);
	sem_init(&atcmdq.sem, 0, 0);

	/* atcmd thread */
	pthread_attr_init(&attr);
	if(pthread_create(&atcmd_task, &attr, atcmd_main, 0)){
		ALOGD("atcmd_task: thread create fail, err=%d:%s\r\n", errno, strerror(errno));
		return -1;
	}
	init=1;
	ALOGD("-atcmd: init\n");
	return 0;
}
#endif

#if defined(EVDO_DT_SUPPORT)
static unsigned char MappingCdmaHwMode(uint8 HW_Mode)
{
	unsigned char CDMA_HW_Mode=9;
	/* mapping
	   LADIN_FM_Radio = 0,       // 0: FM analog line in Mono
	   LADIN_Microphone1,        // 1: Phone Mic (CDMA:0)
	   LADIN_Microphone2,        // 2: Earphone Mic (CDMA:1)
	   LADIN_BTIn,               // 3: BT Mic (CDMA:6)
	   LADIN_SingleDigitalMic,   // 4: Single digital Microphone
	   LADIN_DualAnalogMic,      // 5: Dual analog Microphone
	   LADIN_DualDigitalMic,     // 6: Dual digital Microphone
	   LADIN_FM_AnalogIn_Stereo  // 7: FM analog line in Stereo
	 */
	if(HW_Mode==LADIN_Microphone1) {
		return 0;
	} else if(HW_Mode==LADIN_Microphone2) {
		return 1;
	} else if(HW_Mode==LADIN_BTIn) {
		return 6;
	} else {
		/* un-support */
		return 9;
	}
}

static unsigned char MappingSpeechMode2Cdma(uint8 HW_Mode)
{
	unsigned char CDMA_HW_Mode=9;
	/* mapping
	   SPH_MODE_NORMAL=0,	// 0: CDMA(0)
	   SPH_MODE_EARPHONE=1,	// 1: CDMA(1)
	   SPH_MODE_LOUDSPK=2,	// 2: CDMA(2)
	   SPH_MODE_BT_EARPHONE=3,	// 3: CDMA(6)
	   SPH_MODE_BT_CORDLESS=4,	// 4: CDMA(6)
	   SPH_MODE_BT_CARKIT=5,	// 4: CDMA(6)
	   SPH_MODE_PRESERVED_1=6,
	   SPH_MODE_PRESERVED_2=7,
	   SPH_MODE_NO_CONNECT=8,
	 */
	if(HW_Mode==SPH_MODE_NORMAL) {
	return 0;
	} else if(HW_Mode==SPH_MODE_EARPHONE) {
		return 10;
	} else if(HW_Mode==SPH_MODE_LOUDSPK) {
		return 11;
	} else if(HW_Mode==SPH_MODE_BT_EARPHONE || HW_Mode==SPH_MODE_BT_CORDLESS || HW_Mode==SPH_MODE_BT_CARKIT) {
		return 6;
	} else {
		/* un-support */
		return 9;
	}
}
#endif

AudioATCommand::AudioATCommand(AudioYusuHardware *hw)
{
    mHw = hw;

    /* file handle is created to be 0 in constructor */
    /* it will become non-zero when initialized */
    fHdl = 0;
    bATCmdDeviceOpen = false;
    mMicGain = 0;
}


AudioATCommand::~AudioATCommand()
{
	ALOGD("AudioATCommand De-Constructor bATCmdDeviceOpen(%d) \n", bATCmdDeviceOpen);
    closeDevice(fHdl);
}

int AudioATCommand::openDevice(char *deviceName)
{
    char buf[BUFFER_SIZE];
    const int HALT = 200000;
    int i;
    if(!bATCmdDeviceOpen)
    {
		ALOGD("%s - %s\n", __FUNCTION__, deviceName);
        fHdl = open(deviceName, O_RDWR | O_NONBLOCK);

        if(fHdl < 0)
        {
			ALOGD("Fail to open %s, fHdl=%d\n", deviceName, fHdl);
            return -1;
        }
        else
        {
            bATCmdDeviceOpen = true;
			ALOGD("open %s Success!!!, fHdl=%d\n", deviceName, fHdl);
        }
        // +EIND will always feedback +EIND when open device,
        // so move this to openDevice.
        // +EIND
        memset(buf, 0, BUFFER_SIZE);
        for(i=0; i<5; i++)
        {
            usleep(HALT);
            read(fHdl, buf, BUFFER_SIZE);
            if(strcmp(buf, "+EIND")==0)
            {
                break;
            }
        }
		ALOGD("Open feedback:'%s', i=%d\n", buf, i);
    }
    return fHdl;
}

void AudioATCommand::closeDevice(int fd)
{
	ALOGD("%s - %d\n", __FUNCTION__, fd);
    if(bATCmdDeviceOpen)
    {
        close(fd);
        bATCmdDeviceOpen = false;
    }
}

int AudioATCommand::sendAtCommand(int fd, char* command)
{
    char buf[BUFFER_SIZE], buf_RX[BUFFER_SIZE];
    const int HALT = 20000;	/*20ms */
    int i;
	ALOGD("%s - %s", __FUNCTION__, command);
    sprintf(buf, "%s\r\n", command);
    memset(buf_RX, 0, BUFFER_SIZE);
    write(fd, buf, strlen(buf));
    for(i=0; i<5; i++)
    {
        usleep(HALT);
        read(fd, buf_RX, BUFFER_SIZE);
        if(strcmp(buf_RX, "OK")==0)//0: the same
        {
            break;
        }
    }
	ALOGD("Result: %s, i=%d\n", buf_RX, i);
    return strcmp(buf, "OK");
}

bool AudioATCommand::Spc_Speech_On(uint8 RAT_Mode)
{
    char command[BUFFER_SIZE];
    int fd;
    bool ret;
	fd = openDevice((char*)DEVICE_NAME);
	sprintf(command, "%s%d\r\n", (char*)AT_COMMAND_SPEECH_ON, RAT_Mode);
	ret = sendAtCommand(fd, (char*)command);
    return ret;
}

bool AudioATCommand::Spc_Speech_Off(void)
{
    int fd;
    bool ret;
	ALOGV("%s\n", __FUNCTION__);
	fd = openDevice((char*)DEVICE_NAME);
	ret = sendAtCommand(fd, (char*)AT_COMMAND_SPEECH_OFF);
    return ret;
}

bool AudioATCommand::Spc_SetSpeechMode_Adaptation( uint8 mode )
{
#if defined(EVDO_DT_SUPPORT)
	int result;
	atcmd_req_t cmd;
	unsigned char cHW_Mode=0;
	cHW_Mode= MappingSpeechMode2Cdma(mode);
	if(cHW_Mode==9) {
		ALOGD(AUDMSG"2nd MD Spc_SetSpeechMode_Adaptation, HW_Mode:%x unknow map, keep input source(CDMA:%x)", mode, cHW_Mode);
		return strcmp("OK", "OK");
	} else {
		ALOGD(AUDMSG"2nd MD Spc_SetSpeechMode_Adaptation, HW_Mode:%x (CDMA:%x)", mode, cHW_Mode);

		/* add request command in queue */
		memset(&cmd, 0, sizeof(atcmd_req_t));
		cmd.type=REQ_AUDIO_MODE;
		cmd.arg1=cHW_Mode;
		result=atcmdq_add(&cmd);
		if(result<0) {
			return strcmp("OK", "Fail");
		} else {
			return strcmp("OK", "OK");
		}
	}
#else
    char command[BUFFER_SIZE];
    int fd;
    bool ret;
    fd = openDevice(DEVICE_NAME);
	ALOGV("%s - %d\n", __FUNCTION__, mode);
    sprintf(command, "%s%d\r\n", AT_COMMAND_SPEECH_MODE_ADAPTATION, mode);
    ret = sendAtCommand(fd, command);
    return ret;
#endif
}

bool AudioATCommand::Spc_SetOutputDevice( uint8 device )
{
#if defined(EVDO_DT_SUPPORT)
	int ret=0;
	ALOGV("%s - %d, for CDMA Spc_SetOutputDevice did\n", __FUNCTION__, device);
    return ret;
#else
    char command[BUFFER_SIZE];
    int fd;
    bool ret;
	ALOGV("%s - %d\n", __FUNCTION__, device);
    fd = openDevice(DEVICE_NAME);
    sprintf(command, "%s%d\r\n", AT_COMMAND_OUTPUT_DEVICE, device);
    ret = sendAtCommand(fd, command);
    return ret;
#endif
}

bool AudioATCommand::Spc_SetInputSource( uint8 src )
{
#if defined(EVDO_DT_SUPPORT)
	ALOGD(AUDMSG"2nd MD LAD_SetInputSource, HW_Mode:%x ", src);
		return strcmp("OK", "OK");
#else
    char command[BUFFER_SIZE];
    int fd;
    bool ret;
	ALOGV("%s - %d\n", __FUNCTION__, src);
    fd = openDevice(DEVICE_NAME);
    sprintf(command, "%s%d\r\n", AT_COMMAND_INPUT_SOURCE, src);
    ret = sendAtCommand(fd, command);
    return ret;
#endif
}

bool AudioATCommand::Spc_SetOutputVolume( uint8 volume, int8 digital_gain_index )
{
    char command[BUFFER_SIZE];
    int fd;
    bool ret;
	ALOGD("%s - %d, %d\n", __FUNCTION__, volume, digital_gain_index);
	fd = openDevice((char*)DEVICE_NAME);
    sprintf(command, "%s%d, %d\r\n", AT_COMMAND_OUTPUT_VOLUME, volume, digital_gain_index);
    ret = sendAtCommand(fd, command);
    return ret;
}

bool AudioATCommand::Spc_SetOutputVolume( uint32 Gain )
{
    //[ToDo]Add output volume settings here.
#if defined(EVDO_DT_SUPPORT)
    /* config the VGR for speaker */
	int result;
	atcmd_req_t cmd;
	int gain[] 	= {-63, -57, -51, -45, -39, -33, -27 };
	int cdma_vol[]	= {  5,   5,   6,   7,   8,   9,  10 };
	unsigned int i;
	int vol;

	vol=0;
	for(i=0;i<sizeof(gain)/sizeof(int);i++) {
		if(gain[i]==(int)Gain) {
			vol=cdma_vol[i];
			break;
		}
	}

	ALOGD("EVDO>>> +Spc_SetOutputVolume");
	if(vol!=0) {
		ALOGD("EVDO>>> Spc_SetOutputVolume:%s Gain:%d vol:%d\n", __FUNCTION__, Gain, vol);
		memset(&cmd, 0, sizeof(atcmd_req_t));
		cmd.type=REQ_AUDIO_VOLUME;
		cmd.arg1=vol;
		result=atcmdq_add(&cmd);
		if(result<0) {
			return strcmp("OK", "Fail");
		} else {
			return strcmp("OK", "OK");
		}
	} else {
		ALOGD("EVDO>>> Spc_SetOutputVolume:%s Gain:%d no mapping\n", __FUNCTION__, Gain);
		return strcmp("OK", "OK");
	}
#else
    return false;
#endif
}

bool AudioATCommand::Spc_SetMicrophoneVolume( uint8 mic_volume )
{
#if defined(EVDO_DT_SUPPORT)
    /* config the VGT for mic */
	bool ret=0;
	ALOGD("Spc_SetMicrophoneVolume: %s - %d\n", __FUNCTION__, mic_volume);
    return ret;
#else
    char command[BUFFER_SIZE];
    int fd;
    bool ret;
	ALOGV("%s - %d\n", __FUNCTION__, mic_volume);
    mMicGain = mic_volume;
    fd = openDevice(DEVICE_NAME);
    sprintf(command, "%s%d\r\n", AT_COMMAND_MICROPHONE_VOLUME, mic_volume);
    ret = sendAtCommand(fd, command);
    return ret;
#endif
}

bool AudioATCommand::Spc_SetSidetoneVolume( uint8 sidetone_volume )
{
    char command[BUFFER_SIZE];
    int fd;
    bool ret;
	ALOGV("%s - %d\n", __FUNCTION__, sidetone_volume);
	fd = openDevice((char*)DEVICE_NAME);
    sprintf(command, "%s%d\r\n", AT_COMMAND_SIDETONE_VOLUME, sidetone_volume);
    ret = sendAtCommand(fd, command);
    return ret;
}

bool AudioATCommand::Spc_Default_Tone_Play(uint8 toneIdx)
{
#if defined(EVDO_DT_SUPPORT)
	int result;
	atcmd_req_t cmd;
	if(toneIdx<=11) {
		ALOGD("%s CDMA-DTMF, tone:%d\n", __FUNCTION__, toneIdx);
		/* add request command in queue */
		memset(&cmd, 0, sizeof(atcmd_req_t));
		cmd.type=REQ_PLAY_DTMF;
		cmd.arg1=1;
		cmd.arg2=toneIdx;
		result=atcmdq_add(&cmd);
		if(result<0) {
				return strcmp("OK", "Fail");
		} else {
				return strcmp("OK", "OK");
		}
	} else if(toneIdx>=12 && toneIdx<=14) {
		ALOGD("%s CDMA-TONE, tone:%d\n", __FUNCTION__, toneIdx);
		/* add request command in queue */
		memset(&cmd, 0, sizeof(atcmd_req_t));
		cmd.type=REQ_PLAY_TONE_SEQ;
		cmd.arg1=toneIdx;
		result=atcmdq_add(&cmd);
		if(result<0) {
				return strcmp("OK", "Fail");
		} else {
				return strcmp("OK", "OK");
		}
	} else {
		ALOGD("%s CDMA-toneIdx:%d not support\n", __FUNCTION__, toneIdx);
		return false;
	}
#else
    char command[BUFFER_SIZE];
    int fd;
    bool ret;
	ALOGV("%s - %d\n", __FUNCTION__, toneIdx);
    fd = openDevice(DEVICE_NAME);
    sprintf(command, "%s%d\r\n", AT_COMMAND_DEFAULT_TONE_PLAY, toneIdx);
    ret = sendAtCommand(fd, command);
    return ret;
#endif
}

bool AudioATCommand::Spc_Default_Tone_Stop(void)
{
#if defined(EVDO_DT_SUPPORT)
	int result;
	atcmd_req_t cmd;
	ALOGD("%s CDMA-Tone Stop\n", __FUNCTION__);
	/* add request command in queue */
	memset(&cmd, 0, sizeof(atcmd_req_t));
	cmd.type=REQ_PLAY_DTMF;
	cmd.arg1=0;
	cmd.arg2=0;
	result=atcmdq_add(&cmd);
	if(result<0) {
			return strcmp("OK", "Fail");
	} else {
			return strcmp("OK", "OK");
	}
#else
    int fd;
    bool ret;
	ALOGV("%s\n", __FUNCTION__);
    fd = openDevice(DEVICE_NAME);
    ret = sendAtCommand(fd, AT_COMMAND_DEFAULT_TONE_STOP);
    return ret;
#endif
}

bool AudioATCommand::Spc_MuteMicrophone(uint8 enable)
{
#if defined(EVDO_DT_SUPPORT)
		int result;
		atcmd_req_t cmd;
	ALOGV("%s - %d\n", __FUNCTION__, enable);
		memset(&cmd, 0, sizeof(atcmd_req_t));
		cmd.type=REQ_MUTE;
		cmd.arg1=enable;
		result=atcmdq_add(&cmd);
		if(result<0) {
			return strcmp("OK", "Fail");
		} else {
			return strcmp("OK", "OK");
		}
#else
    char command[BUFFER_SIZE];
    int fd;
    bool ret;
    int32 gain = 0;
	ALOGV("%s - %d\n", __FUNCTION__, enable);
    fd = openDevice(DEVICE_NAME);
    gain = (enable > 0)? 0 : mMicGain;
    sprintf(command, "%s%d\r\n", AT_COMMAND_MICROPHONE_VOLUME, gain);
    ret = sendAtCommand(fd, command);
    return ret;
#endif
}

int AudioATCommand::openRecDevice(char *deviceName)
{
    //[ToDo]Add EVDO code here
    return -1;
}

void AudioATCommand::closeRecDevice(void)
{
    //[ToDo]Add EVDO code here
}

int AudioATCommand::Spc_OpenNormalRecPath(uint8 format, uint8 sampleRate)
{
    //[ToDo]Add EVDO code here
    return true;
}

int AudioATCommand::Spc_CloseNormalRecPath()
{
    //[ToDo]Add EVDO code here
    return true;
}

int  AudioATCommand::Spc_ReadRNormalRecData(uint16 *pBuf, uint16 u2Size)
{
    //[ToDo]Add EVDO code here
    return 0;
}

int  AudioATCommand::Spc_GetParameters(ENUM_SPC_GET_PARAMS enum_idx)
{
    int i4ret = 0;
    AutoMutex lock(mLock);
    switch(enum_idx){
        case SPC_GET_PARAMS_SR:
        {
            i4ret = EVDO_REC_SR;
            break;
        }
        case SPC_GET_PARAMS_CH:
        {
            i4ret = EVDO_REC_CH;
            break;
        }
        case SPC_GET_PARAMS_REC_STATE:
        {
            i4ret = bRecordState;
            break;
        }
        default:
        {
            LOGE("%s - Invalid Input Par(%d)\n", __FUNCTION__,enum_idx);
            i4ret = -1;
            break;
        }
    }
    return i4ret;
}


}; // namespace android
