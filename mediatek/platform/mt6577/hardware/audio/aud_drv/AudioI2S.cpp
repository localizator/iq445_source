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
 *   AudioI2S.cpp
 *
 * Project:
 * --------
 *   Android
 *
 * Description:
 * ------------
 *   I2S input driver (user space)
 *
 * Author:
 * -------
 *   Stan Huang (mtk01728)
 *
 *------------------------------------------------------------------------------
 * $Revision$
 * $Modtime:$
 * $Log:$
 *
 * 04 13 2012 donglei.ji
 * [ALPS00267675] [Need Patch] [Volunteer Patch] modify FM and MATV sample rate setting
 * get FM and MATV I2S sample rate from driver.
 *
 * 02 03 2012 donglei.ji
 * [ALPS00103003] [Need Patch] [Volunteer Patch]AudioFlinger record, I2S, A2dp dynamic dump data
 * dynamic dump data.
 *
 * 12 15 2011 weiguo.li
 * [ALPS00103413] [Need Patch] [Volunteer Patch]
 * .
 *
 * 12 15 2011 donglei.ji
 * [ALPS00103003] [Need Patch] [Volunteer Patch]AudioFlinger record, I2S, A2dp dynamic dump data
 * remove compile option.
 *
 * 12 14 2011 donglei.ji
 * [ALPS00103003] [Need Patch] [Volunteer Patch]AudioFlinger record, I2S, A2dp dynamic dump data
 * dynamic dump data.
 *
 *
 *******************************************************************************/

/*****************************************************************************
*                     C O M P I L E R   F L A G S
******************************************************************************
*/

#define LOG_TAG  "AudioI2S"

//#define ENABLE_LOG_I2S
#ifdef ENABLE_LOG_I2S
#undef ALOGV
#define ALOGV(...) ALOGD(__VA_ARGS__)
#endif

//#define DUMP_I2S_REC_DATA
//#define HQA_I2S_SLAVE_IN

/*****************************************************************************
*                E X T E R N A L   R E F E R E N C E S
******************************************************************************
*/
#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sched.h>
#include <fcntl.h>
#include <time.h>
#include <sys/ioctl.h>
#include <utils/Log.h>
#include <utils/String8.h>
#include <utils/threads.h>

#ifndef ANDROID_DEFAULT_CODE
#include <cutils/properties.h>
#endif

#include "AudioYusuHardware.h"
#include "AudioYusuDef.h"
#include "audio_custom_exp.h"

#include "AudioAfe.h"
#include "AudioIoctl.h"
#include "AudioI2S.h"

#include <linux/rtpm_prio.h>
#include <linux/fm.h> 

#define MATV_DEVICE_NAME "/dev/MATV"
/*****************************************************************************
*                         D A T A   T Y P E S
******************************************************************************
*/

/*****************************************************************************
*                          C O N S T A N T S
******************************************************************************
*/

/*****************************************************************************
*                        C L A S S   D E F I N I T I O N
******************************************************************************
*/
static long long getTimeMs()
{
   struct timeval t1;
   long long ms;

   gettimeofday(&t1, NULL);
   ms = t1.tv_sec * 1000LL + t1.tv_usec / 1000;

   return ms;
}

#if !defined(ANDROID_DEFAULT_CODE) && defined(DEBUG_AUDIO_PCM)
static int checkAndCreateDirectory(const char * pC)
{
	char tmp[PATH_MAX];
	int i = 0;

	while(*pC)
	{
		tmp[i] = *pC;

		if(*pC == '/' && i)
		{
			tmp[i] = '\0';
			if(access(tmp, F_OK) != 0)
			{
				if(mkdir(tmp, 0770) == -1)
				{
					ALOGE("AudioDumpPCM: mkdir error! %s\n",(char*)strerror(errno));
					return -1;
				}
			}
			tmp[i] = '/';
		}
		i++;
		pC++;
	}
	return 0;
}

//add by Donglei to dump pcm data
static void dumpPCMData(void * buffer, int count)
{
	const char * i2sstreamin = "/sdcard/mtklog/audio_dump/I2S_StreamIn_Dump.pcm";
    char value[PROPERTY_VALUE_MAX];
	int ret;
    property_get("I2S.streamin.pcm", value, "0");
	  int bflag=atoi(value);
	  if(bflag) 
	  {
	  	  ret = checkAndCreateDirectory(i2sstreamin);
		  if(ret<0)
		  {
			  ALOGE("AudioI2S Dump_StreamIn_PCM checkAndCreateDirectory() fail!!!");
		  }
		  else
		  {
	      	FILE * fp= fopen(i2sstreamin, "ab+");
	      	if(fp!=NULL) {
	          fwrite(buffer,count,1,fp);
	          fclose(fp);
	      	}else {
	          ALOGE("dump I2S.streamin.pcm fail");
	      	}
		  }
	  }
}
#endif
namespace android {

// get input I2S driver information
static int getDriverInfo(I2STYPE device, DriverInfo *pInfo)
{
    int fd = -1;
    int ret = -1;
		
    switch(device){
      case MATV:
          fd = open(MATV_DEVICE_NAME, O_RDWR);
          if(fd<0){
              ALOGE("Open 'dev/MATV' Fail !fd(%d)", fd);
          }else{
              ret = ioctl(fd, MATV_QUERY_I2S_INFO, pInfo);
              ALOGD("MATV I2S input infos status=%d, mode=%d, samplerate=%d", pInfo->status, pInfo->mode, pInfo->sampleRate);
              if(ret){
                  ALOGE("get ioctl MATV_QUERY_I2S_INFO Fail !ret(%d)", ret);
              }
          }
          break;
      case FMRX:
          fd = open(FM_DEVICE_NAME, O_RDWR);
          if(fd<0){
              ALOGE("Open 'dev/fm' Fail !fd(%d)", fd);
          }else{
              ret = ioctl(fd, FM_IOCTL_GET_I2S_INFO, pInfo);
              ALOGD("FM I2S input infos status=%d, mode=%d, samplerate=%d", pInfo->status, pInfo->mode, pInfo->sampleRate);
              if(ret){
                  ALOGE("get ioctl FM_IOCTL_GET_I2S_INFO Fail !ret(%d)", ret);
              }
          }
          break;
      default:
          ALOGE("do not support this device ! device(%d)", device);
          break;
    }

    if (fd>=0) {
        close(fd);
     }
    return ret;
}

AudioI2S* AudioI2S::UniqueI2SInstance = NULL;

static pthread_mutex_t mReadMutex;
static pthread_cond_t mRead_Cond;

static int SampleCount =0;
static int current_user=0;
static int SampleCount_Dump =0;

AudioI2S* AudioI2S::getInstance()
{
    if(UniqueI2SInstance == NULL){
        ALOGV("+UniqueI2SInstance");
        UniqueI2SInstance = new AudioI2S();
        ALOGV("-UniqueI2SInstance");
    }
    current_user++;
    ALOGV("getInstance(), current_user=%d ",current_user);
    return UniqueI2SInstance;
}

void AudioI2S::freeInstance()
{
    current_user--;
    ALOGV("freeInstance(), current_user=%d",current_user);

    /*
    // here I2S would be a manager
    if(current_user == 0){
        ALOGV("+delete UniqueI2SInstance");
        delete UniqueI2SInstance;
        UniqueI2SInstance = NULL;
    }
    */
    return;
}

int AudioI2S::GetI2SCapability(void)
{
    return I2S_CAPABILITY;
}

void *I2sReadRoutine(void *hdl)
{
   ALOGV("...[I2sReadRoutine]... tid:%d", gettid());
   uint32 readsize=0;
   int ret;
   I2SClient *pClient;
   int client_cnt;
   int i;
   int buf_index;
   int read_fail_count = 0;
   long long renderbrt;
   int ReadBufSize =I2S_INPUT_BUFFER_SIZE;
   AudioI2S *I2SInstance = AudioI2S::getInstance();
   int AudioFd = I2SInstance->GetAudioHandle();
   I2SInputBuffer *I2S_Buf = &(I2SInstance->I2SRead_Buf);

   // defaut setting of thread init
   pthread_mutex_lock(&mReadMutex);
   I2SInstance->ReadThreadExit = false;
   I2SInstance->ReadThreadAvtive = true;
   pthread_mutex_unlock(&mReadMutex);

   if(I2S_Buf == NULL)
   {
      ALOGD("[I2sReadRoutine] No Buffer fed to kernel, Exit");
      pthread_mutex_lock(&mReadMutex);
      I2SInstance->ReadThreadExit = true;
      I2SInstance->ReadThreadAvtive = false;
      pthread_mutex_unlock(&mReadMutex);
   }

   ret = I2SInstance->FindFreeClient(&client_cnt);

   // removed by 
   // this will prevent a case to start normally
   // 1. client1 open => 2. client2 open => 3. client2 start
   // when client 2 start, in this check it first check client1 if it has buffer, it should be NULL, so thread will exit.
   // Remove is ok because in while loop, it also check for NULL buffer pointer. If it is NULL, it will not copy data. (see below)
   /*
   for(i=0;i<client_cnt;i++)
   {
      pClient = I2SInstance->GetI2SClient(i);
      if( (pClient->InputBuffer[0].Buffer == NULL) || (pClient->InputBuffer[1].Buffer == NULL) )
      {
         ALOGD("[I2sReadRoutine] No Client Buffer, Exit ReadRoutine, index:%d", i);
         pthread_mutex_lock(&mReadMutex);
         I2SInstance->ReadThreadExit = true;
         I2SInstance->ReadThreadAvtive = false;
         ALOGD("I2SInstance->ReadThreadExit[%x]= %d", I2SInstance->ReadThreadExit, &(I2SInstance->ReadThreadExit));
         ALOGD("I2SInstance->ReadThreadAvtive[%x]= %d", I2SInstance->ReadThreadAvtive, &(I2SInstance->ReadThreadAvtive));
         pthread_mutex_unlock(&mReadMutex);
      }
   }*/

   int result = -1;
   // if set prority false , force to set priority
   if(result == -1){
      struct sched_param sched_p;
      sched_getparam(0, &sched_p);
      sched_p.sched_priority = RTPM_PRIO_AUDIO_I2S;
      if(0 != sched_setscheduler(0, SCHED_RR, &sched_p)) {
         ALOGE("[%s] failed, errno: %d", __func__, errno);
      }
      else {
         sched_p.sched_priority =RTPM_PRIO_AUDIO_I2S;
         sched_getparam(0, &sched_p);
         ALOGD("sched_setscheduler ok, priority: %d", sched_p.sched_priority);
      }
   }

   ALOGD("[I2sReadRoutine] client_cnt=%d, mState=%d, ReadThreadExit=%d, mFd=%d",client_cnt,I2SInstance->GetState(),I2SInstance->ReadThreadExit,AudioFd);
   while( (I2SInstance->StateInI2S0InputStart()) && (I2SInstance->ReadThreadExit == false) )
   {
      // read from I2S driver
      renderbrt = getTimeMs();
      readsize = ::read(AudioFd,I2S_Buf->Buffer,ReadBufSize);
      ALOGV("[%lld]~[%lld][I2sReadRoutine] read readsize=%d",renderbrt, getTimeMs(), readsize);

      // readsize = -1 --> mean No data Read from kernel.
      int i_readside = (int)readsize;
      if(i_readside == -1){
         read_fail_count++;
         usleep(10*1000);
         if(read_fail_count > 100){
            ALOGD("[I2sReadRoutine] Fail, No data from Kenrel");
            break;
         }
      }
#if !defined(ANDROID_DEFAULT_CODE) && defined(DEBUG_AUDIO_PCM)
      dumpPCMData((void *)I2S_Buf->Buffer, i_readside);
#endif

      pthread_mutex_lock(&mReadMutex);
      ret = I2SInstance->FindFreeClient(&client_cnt);

      for(i=0;i<client_cnt;i++)
      {
         ALOGV("[I2sReadRoutine] client[%d] buf_idx=%d",i,pClient->Buf_idx);

         pClient = I2SInstance->GetI2SClient(i);
         if( (pClient->IsOccupied==true) && (pClient->IsStart==true) )
         {
            buf_index = pClient->Buf_idx;

            if((pClient->InputBuffer[buf_index].Buffer != NULL) && (readsize <= EDI_BUFFER_SIZE)){
            	 //ALOGD("[Tid:%d] I2sReadRoutine Copy InputBuffer:%x, I2S_Buf:%x, size:%d",gettid(), (int)pClient->InputBuffer[buf_index].Buffer, (int)I2S_Buf->Buffer, readsize);
               memcpy((void*)pClient->InputBuffer[buf_index].Buffer,(void*)I2S_Buf->Buffer,readsize);
            }
            else{
               ALOGD("[I2sReadRoutine] error No Client[%d] Buffer",client_cnt);
               ALOGD("[I2sReadRoutine] or readsize abnormal (readsize is unsigned) : %d ",readsize);
            }
            pClient->InputBuffer[buf_index].Valid = true;
            pClient->Buf_idx = !(pClient->Buf_idx);
         }
      }

      SampleCount += readsize;
      ALOGV("[I2sReadRoutine] finish -- readsize=%u, buf_index=%d",readsize,pClient->Buf_idx);
      pthread_cond_broadcast(&mRead_Cond); // wake all thread
      pthread_mutex_unlock(&mReadMutex);
   }

   ALOGD("[I2sReadRoutine] exit ");
   pthread_mutex_lock(&mReadMutex);
   I2SInstance->ReadThreadExit= true;
   I2SInstance->ReadThreadAvtive = false;

   AudioI2S::freeInstance();
   ALOGD("[I2sReadRoutine] -freeInstance ");
   pthread_cond_broadcast(&mRead_Cond); // wakeu all thread
   ALOGD("[I2sReadRoutine] -pthread_cond_broadcast ");
   pthread_mutex_unlock(&mReadMutex);

   return 0;
}

AudioI2S::AudioI2S()
{
   ALOGV("+AudioI2S()");

   int ret=0;
   mFd = ::open(kAudioDeviceName, O_RDWR);
   if(mFd < 0){
     ALOGE("AudioI2S mFd init fail");
   }
   ret = pthread_mutex_init(&mI2SMutex, NULL);
   if ( ret != 0 ){
     ALOGE("Failed to initialize AudioI2S mMutex!");
   }
   //Init mutex for Read data , use for read thread.
   ret = pthread_mutex_init(&mReadMutex, NULL);
   if ( ret != 0 ){
      ALOGE("Failed to initialize AudioI2S mReadMutex!");
   }

   ret = pthread_cond_init(&mRead_Cond, NULL);
   if ( ret != 0 ){
      ALOGE("Failed to initialize AudioI2S mRead_Cond!");
   }

   mAfe = new AudioAfe(NULL);
   ret = mAfe->Afe_Init(mFd);
   if ( ret == false ){
      ALOGE("Failed to Afe_Init mAfe!");
   }
   memset((void*)&mClient,0,sizeof(I2SClient)*I2S_MAX_USER);

   mState = I2S_CREATED;
   ReadThreadExit = false;
   ReadThreadAvtive = false;
   mSamplerate =0;
   mSourceType =-1;
   mReg_Value = 0;
   mInterrupt_Timer = 0;
   I2S0OutputSampleRate =0;
   I2S1OutputSampleRate =0;
   srand ( time(NULL) );

   ALOGV("-AudioI2S()");
}

AudioI2S::~AudioI2S()
{
    if(mFd ){
         close(mFd);
         mFd = 0;
    }
}

unsigned int AudioI2S::samplerate()
{

    ALOGD("[AudioI2S] samplerate=%d",mSamplerate);
    return mSamplerate;
}

int AudioI2S::GetSourceType()
{
    ALOGD("[AudioI2S] GetSourceType, mSourceType=%d",mSourceType);
    return mSourceType;
}

unsigned int AudioI2S::GetReadBufferSize(void)
{
   return (uint32)I2S_INPUT_BUFFER_SIZE;
}

int AudioI2S::checkIdValid(uint32_t Identity)
{
   ALOGV("[checkIdValid] Identity = %d",Identity);
   for(int i=0; i <I2S_MAX_USER ; i++ ){
      if(Identity == mClient[i].Identity)
         return i;
   }
   return -1;
}

bool AudioI2S::CompareIdValid(uint32_t Identity)
{
   ALOGV("[CompareIdValid] Identity = %d",Identity);
   for(int i=0; i <I2S_MAX_USER ; i++ )
   {
      if(mClient[i].IsOccupied == true && mClient[i].Identity == Identity)
      {
         ALOGV("[CompareIdValid]false, Identity=%d",Identity);
         return false;
      }
   }
   return true;
}

bool AudioI2S::CheckCanStop(void)
{
   for(int i=0; i <I2S_MAX_USER ; i++ )
   {
      ALOGD("i = %d mClient[i].IsStart  = %d",i,mClient[i].IsStart );
      if(mClient[i].IsStart == true )
      {
         ALOGV("[CheckCanStop] false");
         return false;
      }
   }
   ALOGV("[CheckCanStop] true");
   return true;
}

bool AudioI2S::FindFreeClient(int* ClientIndex)
{
   //ALOGV("FindFreeClient ClientIndex=%u",*ClientIndex);
   for(int i=0;i< I2S_MAX_USER ; i++)
   {
      if(mClient[i].IsOccupied ==0){
         *ClientIndex = i;
         return true;
      }
   }
   return false;
}

bool AudioI2S::FindValidClient(uint32_t Identity)
{
   for(uint32_t i=0;i< I2S_MAX_USER ; i++)
   {
      if(mClient[i].Identity ==Identity){
         return true;
      }
   }
   return false;
}

bool AudioI2S::setEdiControl(uint32_t TYPE, int32_t samplerate)
{
   ALOGD("[setEdiControl] TYPE=%d",TYPE);
   uint32 Reg_Value        = 0;
   uint32 Input_SampleRate = 0;
   uint32 Interrupt_Timer  = 0;
   int chip_ver;
   DriverInfo inputI2SInfo;

   switch(TYPE)
   {
      case MATV:
      {
         Reg_Value &= (~(1<<1));  // bit1: (WLEN)  0 for 16 bits, 1 for 32 bits
         Reg_Value &= (~(1<<2));  // bit2: (SRC)   0 for master mode(clock from AFE), 1 for slave mode(clock from other device)
         Reg_Value |= (1<<3);     // bit3: (FMT)   1 for I2S, 0 for EIAJ
         Reg_Value |= (1<<4);     // bit4: (DIR)   0 for output mode, 1 for input mode

         mSourceType = MATV;
         
         if (getDriverInfo(MATV, &inputI2SInfo)==0) {
             Input_SampleRate = (uint32)inputI2SInfo.sampleRate;
             mSamplerate = Input_SampleRate;
         }else {
             Input_SampleRate = MATV_SAMPLERATE;
             mSamplerate = MATV_SAMPLERATE;
         }
         ALOGD("setEdiControl MATV sample rate is %d",mSamplerate);
         Interrupt_Timer  = (EDI_BUFFER_SIZE >> 4);// stereo, 16bit and 1/4 buffer szie inter
         break;
      }
      case FMRX:{
         Reg_Value &= (~(1<<1));  // bit1: (WLEN)  0 for 16 bits, 1 for 32 bits
         chip_ver = ::ioctl(mFd,AUDDRV_MT6573_CHIP_VER,0);  //ioctl to stanby mode

         if(chip_ver == 0x8a00){
            ALOGD("--[setEdiControl] MT6573 E1 Chip (slave mode) ");
            // For MT6620 HW bug. MT6573E1 Audio Sys must set as slave mode.
            Reg_Value |= (1<<2);     // bit2: (SRC)   0 for master mode(clock from AFE), 1 for slave mode(clock from other device)

         }
         else if(chip_ver == 0xca10){
            ALOGD("--[setEdiControl] MT6573 E2 Chip (master mode) ");
            Reg_Value &= (~(1<<2));  // bit2: (SRC)   0 for master mode(clock from AFE), 1 for slave mode(clock from other device)
         }
         else // default use master mode
         {
            ALOGD("--[setEdiControl] No Chip ver (master mode) ");
            Reg_Value &= (~(1<<2));  // bit2: (SRC)   0 for master mode(clock from AFE), 1 for slave mode(clock from other device)
         }

         Reg_Value |= (1<<3);     // bit3: (FMT)   1 for I2S, 0 for EIAJ
         Reg_Value |= (1<<4);     // bit4: (DIR)   0 for output mode, 1 for input mode

         mSourceType = FMRX;
         
         if (getDriverInfo(FMRX, &inputI2SInfo)==0) {
             Input_SampleRate = (uint32)inputI2SInfo.sampleRate;
             mSamplerate = Input_SampleRate;
         }else {
             Input_SampleRate = 48000;//FM_SAMPLERATE;
             mSamplerate = Input_SampleRate;
         }
         ALOGD("setEdiControl FM sample rate is %d",mSamplerate);
         Interrupt_Timer  = (EDI_BUFFER_SIZE >> 4);// stereo, 16bit and 1/4 buffer size inter
         ALOGD("--[setEdiControl] FMRX (%d) ",Input_SampleRate);
         break;
      }
      case I2S0OUTPUT:{
         Reg_Value &= (~(1<<1));  // bit1: (WLEN)  0 for 16 bits, 1 for 32 bits
         Reg_Value &= (~(1<<2));  // bit2: (SRC)   0 for master mode(clock from AFE), 1 for slave mode(clock from other device)
         Reg_Value |= (1<<3);  // bit3: (FMT)   1 for I2S, 0 for EIAJ
         Reg_Value &= (~(1<<4));     // bit4: (DIR)   0 for output mode, 1 for input mode
         mSourceType = I2S0OUTPUT;
         Input_SampleRate = 44100;
         Interrupt_Timer  = (EDI_BUFFER_SIZE >> 4);// stereo, 16bit and 1/4 buffer szie inter
         break;
      }

      case HOA_SAMPLERATE:   //use for HQA support
      {
          Reg_Value &= (~(1<<1));  // bit1: (WLEN)  0 for 16 bits, 1 for 32 bits
#ifdef HQA_I2S_SLAVE_IN
          Reg_Value |= (1<<2);     // bit2: (SRC)   0 for master mode(clock from AFE), 1 for slave mode(clock from other device)
#else
          Reg_Value &= (~(1<<2));  // bit2: (SRC)   0 for master mode(clock from AFE), 1 for slave mode(clock from other device)
#endif
          Reg_Value |= (1<<3);     // bit3: (FMT)   1 for I2S, 0 for EIAJ
          Reg_Value |= (1<<4);     // bit4: (DIR)   0 for output mode, 1 for input mode
          ALOGD("setEdiControl HOA_SAMPLERATE with samplerate = %d",samplerate);
          mSourceType = HOA_SAMPLERATE;
          Input_SampleRate = samplerate;
          mSamplerate = Input_SampleRate;
          Interrupt_Timer  = (EDI_BUFFER_SIZE >> 4);// stereo, 16bit and 1/4 buffer szie inter
          break;
      }
      // default use MATV setting
      default:
      {
         Reg_Value &= (~(1<<1));  // bit1: (WLEN)  0 for 16 bits, 1 for 32 bits
         Reg_Value &= (~(1<<2));  // bit2: (SRC)   0 for master mode(clock from AFE), 1 for slave mode(clock from other device)
         Reg_Value |= (1<<3);  // bit3: (FMT)   1 for I2S, 0 for EIAJ
         Reg_Value |= (1<<4);     // bit4: (DIR)   0 for output mode, 1 for input mode
         mSourceType = MATV;
         Input_SampleRate = MATV_SAMPLERATE;
         mSamplerate = MATV_SAMPLERATE;
         Interrupt_Timer  = (EDI_BUFFER_SIZE >> 4);// stereo, 16bit and 1/4 buffer szie inter
         break;
      }
   }

   mReg_Value = Reg_Value;
   mInterrupt_Timer = Interrupt_Timer;

   ALOGD("[setEdiControl] AFE_I2S_IN_CON, AFE_IRQ_CNT2");
   mAfe->Afe_Set_Stream_Attribute(android_audio_legacy::AudioSystem::PCM_16_BIT, android_audio_legacy::AudioSystem::CHANNEL_OUT_STEREO, Input_SampleRate, AFE_MODE_I2S_IN);
   mAfe->SetAfeReg(AFE_I2S_IN_CON, Reg_Value, 0xff);
   mAfe->SetAfeReg(AFE_IRQ_CNT2, Interrupt_Timer, 0xffff); // interrupt counter

   return true;
}

int AudioI2S::GetState()
{
   return mState;
}

// use let I2S read thread to control asm.
int AudioI2S::GetAudioHandle()
{
   return mFd;
}

I2SClient* AudioI2S::GetI2SClient(int index)
{
   return &mClient[index];
}

// when user want to use I2s , first it should request first.
unsigned int AudioI2S::open()
{
   ALOGD("...[open]...");

   int ret =0;
   uint32_t ClientNumber =0;
   int ClientIndex = 0;
   int buffersize = I2S_INPUT_BUFFER_SIZE;

   pthread_mutex_lock( &mI2SMutex);
   ret = FindFreeClient(&ClientIndex);
   if(ret == false){
      ALOGD("[open] fail, no free client");
      pthread_mutex_unlock( &mI2SMutex);
      return false;
   }
   else
   {
      do{
         ClientNumber = (uint32_t)(rand() % RAND_MAX +1);
         ALOGV("[open] new Identity=%d",ClientNumber);
      }while(CompareIdValid(ClientNumber)== false);

      // save information
      mClient[ClientIndex].Identity = ClientNumber;
      mClient[ClientIndex].IsOccupied= true;
   }
   pthread_mutex_unlock(&mI2SMutex);
   ALOGD("[open] ClientIndex=%d, Identity=%d",ClientIndex,ClientNumber);
   return ClientNumber;
}

// release for this ID
bool AudioI2S::close(uint32_t Identity)
{
   ALOGD("...[close]...");

   int ret =0;
   uint32_t ClientIndex = 0;
   pthread_mutex_lock( &mI2SMutex);
   ret = checkIdValid(Identity);
   if(ret == -1){
      ALOGD("[close] error, invalid identity");
      pthread_mutex_unlock( &mI2SMutex);
      return false;
   }

   ALOGD("[close] reset Client information");
   for(int i=0;i< I2S_MAX_USER ; i++)
   {
      if(mClient[i].Identity == Identity)
      {
         // clean information
         mClient[i].Identity   = 0;
         mClient[i].IsOccupied = false;

         ALOGV("[close] success");
         break;
      }
   }
   pthread_mutex_unlock( &mI2SMutex);
   return true;
}

unsigned int AudioI2S::read(uint32_t Identity,void* buffer, uint32 buffersize)
{
   int ret   = 0;
   int index = 0;
   int buf_index   = 0;
   uint32 dataRead = 0;
   uint32 datanumber =I2S_INPUT_BUFFER_SIZE;
   I2SClient *pClient;
   I2SInputBuffer *buf1, *buf2;
   long long rbrt;
   // check if this client is valid.
   index = checkIdValid(Identity);
   if(index == -1){
      ALOGE("[read %d] fail in checkIdValid ",index);
      return 0;
   }
   rbrt = getTimeMs();

   pthread_mutex_lock(&mReadMutex);

   pClient = &mClient[index];

   if( (false == ReadThreadExit) && (pClient->IsStart == true) )
   {
      ALOGV("[read %d] pthread_cond_wait buffersize=%u",index,buffersize);

      //wait only if no data exist
      if( (pClient->InputBuffer[0].Valid == false) && (pClient->InputBuffer[1].Valid == false) ){
         pthread_cond_wait(&mRead_Cond,&mReadMutex);
      }
      else{
         ALOGV("[read %d] have buffer ...",index);
      }

      dataRead = buffersize > datanumber ? datanumber : buffersize;

      if( (pClient->InputBuffer[0].Valid == true) && (pClient->InputBuffer[1].Valid == true) )
      {
         buf_index = !(pClient->Buf_idx);
         memcpy(buffer,(void*)pClient->InputBuffer[buf_index].Buffer,dataRead);
         pClient->InputBuffer[buf_index].Valid = false;
         ALOGV("[%lld]~[%lld] 0[read %d]- Identity:%d, dataRead:%d, buffersize:%d",rbrt,getTimeMs(),index,Identity,dataRead,buffersize);
      }
      else if(pClient->InputBuffer[0].Valid == true)
      {
         memcpy(buffer,(void*)pClient->InputBuffer[0].Buffer,dataRead);
         pClient->InputBuffer[0].Valid = false;
         ALOGV("[%lld]~[%lld] 1[read %d]- Identity:%d, dataRead:%d, buffersize:%d",rbrt,getTimeMs(),index,Identity,dataRead,buffersize);
      }
      else if(pClient->InputBuffer[1].Valid == true)
      {
         memcpy(buffer,(void*)pClient->InputBuffer[1].Buffer,dataRead);
         pClient->InputBuffer[1].Valid = false;
         ALOGV("[%lld]~[%lld] 2[read %d]- Identity:%d, dataRead:%d, buffersize:%d",rbrt,getTimeMs(),index,Identity,dataRead,buffersize);
      }
      else
      {
         ALOGD("[read %d] fail, Buffer Underflow, no buffer",index);
      }

#ifdef DUMP_I2S_REC_DATA
      if(pOutFile != NULL)
      {
#ifdef DUMP_Limited1M_Byte
          if(SampleCount_Dump>=1048576)//1M sample
          {
              fclose(pOutFile);
              pOutFile = fopen("/sdcard/I2SRecord.pcm","wb");
              if( pOutFile == NULL)
              {
                 ALOGD("Fail to Open File %s ",pOutFile);
              }
              SampleCount_Dump=0;
          }
          fwrite(buffer, sizeof(char), dataRead, pOutFile);
          SampleCount_Dump+=dataRead;
#else
          fwrite(buffer, sizeof(char), dataRead, pOutFile);
#endif


      }
#endif

   }

   pthread_mutex_unlock(&mReadMutex);
   return dataRead;
}

bool AudioI2S::set(uint32_t TYPE, int samplerate)
{
   int ret =0;
   ALOGD("...[set]... TYPE = %d samplerate = %d mState = %x",TYPE,samplerate,mState);
   if(StateInIdle()){
      setEdiControl(TYPE,samplerate);
   }
   return true;
}

void AudioI2S::SetI2SState(uint32 state)
{
    ALOGD("SetI2SState state = %d",state);
    mState|=state;
    ALOGD("SetI2SState mState = %d",mState);
}

void AudioI2S::ClearI2SState(uint32 state)
{
    ALOGD("SetI2SState state = %d",state);
    mState&=(~state);
    ALOGD("SetI2SState mState = %d",mState);
}

// when start base on type and valid Identity
bool AudioI2S::start(uint32_t Identity , I2STYPE Type,uint32_t SampleRate){
    ALOGD("AudioI2S Start with Type = %d SampleRate = %d",Type,SampleRate);
    pthread_mutex_lock( &mI2SMutex);
    int index = checkIdValid(Identity);
    if(index <0){
        ALOGD("[start] error, checkIdValid fail");
        pthread_mutex_unlock( &mI2SMutex);
        return false;
    }
    int ret =0;
    switch(Type){
        case MATV:
        case FMRX:
        case FMRX_32K:
        case FMRX_48K:
        {
            if(StateInI2S0OutputStart()){
                ALOGE("StateInI2S0OutputStart mState = %d",mState);
                pthread_mutex_unlock( &mI2SMutex);
                return false;
            }
            else{
                set(Type,SampleRate);
                ret =startInput(Identity,Type);
            }
            break;
        }
        case I2S0OUTPUT:
        {
            if(StateInI2S0InputStart()){
                ALOGE("StateInI2S0InputStart mState = %d",mState);
                pthread_mutex_unlock( &mI2SMutex);
                return false;
            }
            else{
                startI2S0Output(SampleRate);
            }
            break;
        }
        case I2S1OUTPUT:
        {
            if(StateInI2S1OutputStart()){
               ALOGE("StateInI2S1OutputStart state = %d",mState);
               pthread_mutex_unlock( &mI2SMutex);
               return false;
            }
            startI2S1Output(SampleRate);
            break;
        }
        case HOA_SAMPLERATE:
        {
            if(StateInI2S0OutputStart()){
                ALOGE("StateInI2S0OutputStart mState = %d",mState);
                pthread_mutex_unlock( &mI2SMutex);
                return false;
            }
            else{
                set(Type,SampleRate);
                ret =startInput(Identity,Type);
            }
            break;
        }
        default:
        {
             ALOGW("start with no default type");
             pthread_mutex_unlock( &mI2SMutex);
             return false;
        }
    }
    pthread_mutex_unlock( &mI2SMutex);
    return ret;
}

bool AudioI2S::startI2S0Output(uint32_t SampleRate)
{
    ALOGD("startI2S0Output SampleRate = %d",SampleRate);
    if(StateInI2S0OutputStart()||StateInI2S0InputStart()){
         ALOGE("startI2S0Output but state = %d",mState);
         return false;
    }
    else if(StateInI2S1OutputStart() == true){
        if(I2S1OutputSampleRate != SampleRate){
            ALOGE("startI2S0Output SampleRate = %dI2S1OutputSampleRate = %d State = %x"
                ,SampleRate,I2S1OutputSampleRate,mState);
        }
       return false;
    }
    uint32_t registerval =0;
    uint32_t SamplingRateConvert[9] = {0, 1, 2, 4, 5, 6, 8, 9, 10};
    uint32_t SR_index = mAfe->Afe_Get_DL_SR_Index(SampleRate);
    uint32_t SR_index_mode = SamplingRateConvert[SR_index];
    I2S0OutputSampleRate =SampleRate;
    ::ioctl(mFd, AUD_SET_I2S_CLOCK, 1);

    ::ioctl(mFd,SET_DL1_AFE_BUFFER,0);
#ifdef I2S0_OUTPUT_USE_FLEXL2
    ::ioctl(mFd, SET_I2S_Output_BUFFER, 0);
#else
    ::ioctl(mFd,OPEN_I2S_INPUT_STREAM,EDI_BUFFER_SIZE);
    ::ioctl(mFd,START_I2S_INPUT_STREAM,0);
#endif

    registerval = registerval |(SR_index_mode<<8);   // bit8~bit11: I2S_MODE
    mAfe->SetAfeReg(AFE_DAC_CON1,registerval,0x00000f00);
    mAfe->SetAfeReg(AFE_I2S_IN_CON,0x9,0x1f);        // bit0: 1:enable I2S0;bit3: 1:output

    registerval = 0x9|(SR_index_mode<<8);           // bit8~bit11: I2S_MODE
    if(StateInI2S1OutputStart()== false){
        mAfe->SetAfeReg(AFE_I2S_OUT_CON, registerval, 0xf0b);        // bit0: 1:enable I2S1, set sampling rate
    }

    SetI2SState (I2S0_OUTPUT_START);
    return true;
}

bool AudioI2S::stopI2S0Output(void)
{
    ALOGD("stopI2S0Output");
    // Turn off device . Disable I2S0 master output
    mAfe->SetAfeReg(AFE_I2S_IN_CON,0x0,0x1);        // bit0: 1:enable I2S0;bit3:1:output_format
    // if I2S1 is no need to output , stop I2S1 output
    if(StateInI2S1OutputStart() == false){
        mAfe->SetAfeReg(AFE_I2S_OUT_CON,0x0,0x1);
    }
    if(StateInI2S0OutputStart() == true){
        #ifndef I2S0_OUTPUT_USE_FLEXL2
        ::ioctl(mFd, STANDBY_I2S_INPUT_STREAM, 0);
        ::ioctl(mFd, CLOSE_I2S_INPUT_STREAM, 0);
        #endif
        ::ioctl(mFd, AUD_SET_I2S_CLOCK, 0);
        ClearI2SState (I2S0_OUTPUT_START);
    }
    return true;
}

bool AudioI2S::startI2S1Output(uint32_t SampleRate)
{
    ALOGD("startI2S1Output with SampleRate = %d",SampleRate);
    if(StateInI2S1OutputStart()){
        ALOGE("startI2S1Output but state = %d",mState);
        return false;
    }
    else if(StateInI2S0OutputStart()){
        if(I2S0OutputSampleRate != SampleRate){
            ALOGE("startI2S1Output but I2S0OutputSampleRate = %d SampleRate = %d",I2S0OutputSampleRate,SampleRate);
            return false;
        }
    }
    uint32_t registerval =0;
    uint32_t SamplingRateConvert[9] = {0, 1, 2, 4, 5, 6, 8, 9, 10};
    uint32_t SR_index = mAfe->Afe_Get_DL_SR_Index(SampleRate);
    uint32_t SR_index_mode = SamplingRateConvert[SR_index];
    registerval = 0x9 |(SR_index_mode<<8);
    mAfe->SetAfeReg(AFE_I2S_OUT_CON, registerval, 0xf0b);
    ::ioctl(mFd, AUD_SET_I2S_CLOCK, 1);
    I2S1OutputSampleRate =SampleRate;
    SetI2SState (I2S1_OUTPUT_START);
    return true;
}

bool AudioI2S::stopI2S1Output()
{
    ALOGD("stopI2S1Output");
    if(StateInI2S0OutputStart() == false){
        mAfe->SetAfeReg(AFE_I2S_OUT_CON, 0x0, 0x1);
    }
    if(StateInI2S1OutputStart () == true){
        ALOGD("stopI2S1Output AUD_SET_I2S_CLOCK");
        ::ioctl(mFd, AUD_SET_I2S_CLOCK, 0);
        ClearI2SState (I2S1_OUTPUT_START);
    }
    return true;
}

bool AudioI2S::startInput(uint32_t Identity,I2STYPE Type)
{
   ALOGD("...[startInput]...");

   int RegValue = 0;
   int ret =0;
   int index =0;
   int buffersize = I2S_INPUT_BUFFER_SIZE;
   index = checkIdValid(Identity);
   if(index <0){
      ALOGD("[start] error, checkIdValid fail");
      return false;
   }

   mClient[index].IsStart = true;
   ALOGD("index = %d mClient[index].IsStart = %d",index,mClient[index].IsStart);
   for(int i=0; i<I2S_Buffer_Count; i++)
   {
      mClient[index].InputBuffer[i].Buffer = new char[buffersize];
      memset((void*)mClient[index].InputBuffer[i].Buffer ,0,buffersize );
      mClient[index].InputBuffer[i].Valid = false;
   }

   // already Start ?
   if(StateInI2S0InputStart()){
      ALOGD("[start] already in EDI_START state!");
      mClient[index].IsStart = true;
      return true;
   }

   ::ioctl(mFd, AUD_SET_I2S_CLOCK, 1);// turn on audio clock

   I2SRead_Buf.Buffer = new char[buffersize];
   memset((void*)I2SRead_Buf.Buffer,0,buffersize );
   I2SRead_Buf.Valid  = true;

   // allocate input stream buffer.
   ret = ioctl(mFd,OPEN_I2S_INPUT_STREAM,EDI_BUFFER_SIZE);
   if(ret <0){
      ALOGD("[start] OPEN_I2S_INPUT_STREAM error ret = %d",ret);
   }

   ret = ioctl(mFd,START_I2S_INPUT_STREAM,0);
   SetI2SState(I2S0_INPUT_START);

   // start I2S
   mAfe->SetAfeReg(AFE_I2S_IN_CON, mReg_Value, 0xff);
   mAfe->SetAfeReg(AFE_IRQ_CNT2, mInterrupt_Timer, 0xffff); // interrupt counter
   mAfe->Afe_UL_Start(AFE_MODE_I2S_IN);


   ALOGD("[start] +create I2sReadRoutine thread");
   ret = pthread_create(&mReadThread, NULL,I2sReadRoutine,this);
   ALOGD("[start] -create I2sReadRoutine thread");


#ifdef DUMP_I2S_REC_DATA
   pOutFile = fopen("/sdcard/I2SRecord.pcm","wb");
   if( pOutFile == NULL)
   {
      ALOGD("Fail to Open File %s ",pOutFile);
   }
   SampleCount_Dump=0;
#endif
   return true;
}

bool AudioI2S::stop(uint32_t Identity ,I2STYPE Type)
{
    ALOGD("AudioI2S stop with Type = %d",Type);
    pthread_mutex_lock( &mI2SMutex);
    switch(Type){
        case MATV:
        case FMRX:
        case FMRX_32K:
        case FMRX_48K:
        {
            stopInput(Identity,Type);
            break;
        }
        case I2S0OUTPUT:
        {
            stopI2S0Output();
            break;
        }
        case I2S1OUTPUT:
        {
            stopI2S1Output();
            break;
        }
        case HOA_SAMPLERATE:
        {
            stopInput(Identity,Type);
            break;
        }
        default:
        {
            ALOGW("stop with no default type");
            pthread_mutex_unlock( &mI2SMutex);
            return false;
        }
    }
    pthread_mutex_unlock( &mI2SMutex);
    return true;
}

bool AudioI2S::stopInput(uint32_t Identity,I2STYPE Type)
{
   ALOGD("...[stop]...");
   int RegValue = 0;
   int ret =0;
   int index =0;
   index = checkIdValid(Identity);

   if(index < 0){
      ALOGD("[stop] checkIdValid fail");
      return false;
   }

   ALOGV("[stop] mState=%d index = %d",mState,index);
   if(!StateInI2S0InputStart()){
      ALOGD("[stop] mState != I2S0_INPUT_START mState = %d",mState);
      return false;
   }

   mClient[index].IsStart = false;

   for(int i=0; i< I2S_Buffer_Count ; i++)
   {
      if(mClient[index].InputBuffer[i].Buffer != NULL)
      {
         delete mClient[index].InputBuffer[i].Buffer;
         mClient[index].InputBuffer[i].Buffer = NULL;
         mClient[index].InputBuffer[i].Valid = false;
      }
      else{
         ALOGD("[stop] InputBuffer[i] is NULL");
      }
   }

   // can be stop ?
   if(StateInI2S0InputStart() && CheckCanStop() == false){
      ALOGD("[stop] Active Client exist, don't stop");
      return true;
   }

   ReadThreadExit = true;
   int cnt_val=50;

   while((ReadThreadAvtive == true) && (cnt_val>0)){
      ALOGD("[stop] wait thread to exit (%d) ", cnt_val);
      cnt_val--;
      usleep(1000*50);
   }

   if(cnt_val <=0)
   {
      ALOGD("[stop] ReadThreadAvtive:%d, cnt_val:%d ",ReadThreadAvtive,cnt_val);
   }

   pthread_mutex_lock( &mReadMutex);

   if(!StateInI2S0InputStart()){
      ALOGD("[stop] not in I2S0_INPUT_START state ");
      pthread_mutex_unlock(&mReadMutex);
      return false;
   }

#ifdef DUMP_I2S_REC_DATA
   fclose(pOutFile);
#endif

   // stop I2S
   mAfe->Afe_UL_Stop(AFE_MODE_I2S_IN);
   ret = ioctl(mFd,STANDBY_I2S_INPUT_STREAM,0);
   ret = ioctl(mFd,CLOSE_I2S_INPUT_STREAM,0);

   ClearI2SState (I2S0_INPUT_START);
   SampleCount =0;

   ALOGD("[stop] delete I2SRead_Buf (data from kernel)");
   if(I2SRead_Buf.Buffer != NULL)
   {
      delete I2SRead_Buf.Buffer;
      I2SRead_Buf.Buffer = NULL;
      I2SRead_Buf.Valid  = false;
   }

   ::ioctl(mFd, AUD_SET_I2S_CLOCK, 0);// turn off audio clock
   mReg_Value =0;

   pthread_mutex_unlock( &mReadMutex);
   return true;
}

// state of I2S
bool AudioI2S::StateInIdle(void)
{
    return !((mState&I2S0_INPUT_START)||(mState&I2S0_OUTPUT_START)||(mState&I2S1_OUTPUT_START));
}

bool AudioI2S::StateInI2S0InputStart(void)
{
    return mState&I2S0_INPUT_START;
}

bool AudioI2S::StateInI2S0OutputStart(void)
{
    return mState&I2S0_OUTPUT_START;
}

bool AudioI2S::StateInI2S1OutputStart()
{
    return mState&I2S1_OUTPUT_START;
}

bool AudioI2S::StateChecking(uint32_t state)
{
    return mState&state;
}

}

