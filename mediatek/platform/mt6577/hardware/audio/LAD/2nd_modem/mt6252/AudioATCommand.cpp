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

#ifdef LOG_TAG
#undef LOG_TAG
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
#define AT_COMMAND_OPEN_NORMAL_STEREO_REC          "AT+EGCMD=25,2,\"0002\""
#define AT_COMMAND_OPEN_NORMAL_MONO_REC            "AT+EGCMD=25,2,\"0001\""
#define AT_COMMAND_CLOSE_NORMAL_REC                "AT+EGCMD=25,2,\"0000\""
#define REC_DEVICE_NAME                            "/dev/pttyvr"

#define MT6252_HW_INDEX 16
#define MT6252_REC_SR 8000 //8kHz sampling rate
#define MT6252_REC_CH 1    //mono

/* Enable to dump record data */
//#define DUMP_RecordData

int32  MT6252_HW_Gain[] = {8, 6, 4, 2, 0, -2, -4, -6, -8, -10, -12, -14, -16, -18, -20, -22};
int32  MT6252_volume1[] = {240, 224, 208, 192, 176, 160, 144, 128, 112, 96, 80, 64, 48, 32, 16, 0};

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
/*
void *ReadVoiceRecordingThread(void *arg)
{
  AudioATCommand *pATCommand = (AudioATCommand*)arg;
  uint16 vr_buf[640];
  while(1){
      pATCommand->Spc_ReadRNormalRecData(vr_buf,640);
      if(pATCommand->bRecordState == false){
          break;
      }else{
          usleep(10*1000);
      }
  }
  pthread_exit(NULL);
  return null;
}
*/

AudioATCommand::AudioATCommand(AudioYusuHardware *hw)
{
    mHw = hw;
    bRecordState = false;
    /* file handle is created to be 0 in constructor */
    /* it will become non-zero when initialized */
    fHdl = 0;
    fRecHdl = 0;
    bATCmdDeviceOpen = false;
    bRecDeviceOpen   = false;
    mMicGain = 0;
    pRecFile = null;
}


AudioATCommand::~AudioATCommand()
{
    ALOGD("AudioATCommand De-Constructor bATCmdDeviceOpen(%d) \n", bATCmdDeviceOpen);
    closeDevice(fHdl);
    if(fRecHdl){
        closeDevice(fRecHdl);
    }
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
    ret = sendAtCommand(fd, command);
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
    char command[BUFFER_SIZE];
    int fd;
    bool ret;
    fd = openDevice((char*)DEVICE_NAME);
    ALOGV("%s - %d\n", __FUNCTION__, mode);
    sprintf(command, "%s%d\r\n", (char*)AT_COMMAND_SPEECH_MODE_ADAPTATION, mode);
    ret = sendAtCommand(fd, command);
    return ret;
}

bool AudioATCommand::Spc_SetOutputDevice( uint8 device )
{
    char command[BUFFER_SIZE];
    int fd;
    bool ret;
    ALOGV("%s - %d\n", __FUNCTION__, device);
    fd = openDevice((char*)DEVICE_NAME);
    sprintf(command, "%s%d\r\n", (char*)AT_COMMAND_OUTPUT_DEVICE, device);
    ret = sendAtCommand(fd, command);
    return ret;
}

bool AudioATCommand::Spc_SetInputSource( uint8 src )
{
    char command[BUFFER_SIZE];
    int fd;
    bool ret;
    ALOGV("%s - %d\n", __FUNCTION__, src);
    fd = openDevice(DEVICE_NAME);
    sprintf(command, "%s%d\r\n", AT_COMMAND_INPUT_SOURCE, src);
    ret = sendAtCommand(fd, command);
    return ret;
}

bool AudioATCommand::Spc_SetOutputVolume( uint8 volume, int8 digital_gain_index )
{
    char command[BUFFER_SIZE];
    int fd;
    bool ret;
    ALOGV("%s - %d, %d\n", __FUNCTION__, volume, digital_gain_index);
    fd = openDevice(DEVICE_NAME);
    sprintf(command, "%s%d, %d\r\n", AT_COMMAND_OUTPUT_VOLUME, volume, digital_gain_index);
    ret = sendAtCommand(fd, command);
    return ret;
}

bool AudioATCommand::Spc_SetOutputVolume( uint32 Gain )
{
    uint32 hw_index         = 0;
    int32  digital_gain_index=0;
    uint32 volume;
    int32  i4Gain;
    ALOGD("Spc_SetOutputVolume gain:%x",Gain);
    i4Gain = (int32)Gain;
    volume = MT6252_volume1[MT6252_HW_INDEX-1];
    //Gain mapping for MT6252
    for(hw_index = 0; hw_index < MT6252_HW_INDEX; hw_index++)
    {
        //ALOGV("No.%d, Gain(%d),MT6252_HW_Gain[%d]=%d\n",hw_index,Gain,hw_index,MT6252_HW_Gain[hw_index]);
        if( i4Gain == MT6252_HW_Gain[hw_index])
        {
            volume = MT6252_volume1[hw_index];
            digital_gain_index = 0;
            break;
        }
        else if( i4Gain > MT6252_HW_Gain[hw_index])
        {
            volume = MT6252_volume1[hw_index];
            digital_gain_index = 2;
            break;
        }
     }
     return Spc_SetOutputVolume(volume, digital_gain_index);
}
bool AudioATCommand::Spc_SetMicrophoneVolume( uint8 mic_volume )
{
    char command[BUFFER_SIZE];
    int fd;
    bool ret;
    ALOGV("%s - %d\n", __FUNCTION__, mic_volume);
    mMicGain = mic_volume;
    fd = openDevice((char*)DEVICE_NAME);
    sprintf(command, "%s%d\r\n", (char*)AT_COMMAND_MICROPHONE_VOLUME, mic_volume);
    ret = sendAtCommand(fd, command);
    return ret;
}

bool AudioATCommand::Spc_SetSidetoneVolume( uint8 sidetone_volume )
{
    char command[BUFFER_SIZE];
    int fd;
    bool ret;
    ALOGV("%s - %d\n", __FUNCTION__, sidetone_volume);
    fd = openDevice(DEVICE_NAME);
    sprintf(command, "%s%d\r\n", AT_COMMAND_SIDETONE_VOLUME, sidetone_volume);
    ret = sendAtCommand(fd, command);
    return ret;
}

bool AudioATCommand::Spc_Default_Tone_Play(uint8 toneIdx)
{
    char command[BUFFER_SIZE];
    int fd;
    bool ret;
    ALOGV("%s - %d\n", __FUNCTION__, toneIdx);
    fd = openDevice((char*)DEVICE_NAME);
    sprintf(command, "%s%d\r\n", (char*)AT_COMMAND_DEFAULT_TONE_PLAY, toneIdx);
    ret = sendAtCommand(fd, command);
    return ret;
}

bool AudioATCommand::Spc_Default_Tone_Stop(void)
{
    int fd;
    bool ret;
    ALOGV("%s\n", __FUNCTION__);
    fd = openDevice((char*)DEVICE_NAME);
    ret = sendAtCommand(fd, (char*)AT_COMMAND_DEFAULT_TONE_STOP);
    return ret;
}

bool AudioATCommand::Spc_MuteMicrophone(uint8 enable)
{
    char command[BUFFER_SIZE];
    int fd;
    bool ret;
    int32 gain = 0;
    ALOGV("%s - %d\n", __FUNCTION__, enable);
    fd = openDevice((char*)DEVICE_NAME);
    gain = (enable > 0)? 0 : mMicGain;
    sprintf(command, "%s%d\r\n", (char*)AT_COMMAND_MICROPHONE_VOLUME, gain);
    ret = sendAtCommand(fd, command);
    return ret;
}

int AudioATCommand::openRecDevice(char *deviceName)
{
    char buf[BUFFER_SIZE];
    const int HALT = 200000;
    int i;
    if(!bRecDeviceOpen)
    {
        LOGD("%s - %s\n", __FUNCTION__, deviceName);
        fRecHdl = open(deviceName, O_RDWR | O_NONBLOCK);

        if(fRecHdl < 0)
        {
            LOGD("Fail to open %s, fHdl=%d\n", deviceName, fHdl);
            return -1;
        }
        else
        {
            bRecDeviceOpen = true;
            LOGD("open %s Success!!!, fHdl=%d\n", deviceName, fHdl);
        }
    }
    return fRecHdl;

}

void AudioATCommand::closeRecDevice(void)
{
    LOGD("%s - %d\n", __FUNCTION__, fRecHdl);
    if(bRecDeviceOpen)
    {
        close(fRecHdl);
        fRecHdl = -1;
        bRecDeviceOpen = false;
    }
}

int AudioATCommand::Spc_OpenNormalRecPath(uint8 format, uint8 sampleRate)
{
    char command[BUFFER_SIZE];
    int fd;
    int ret;
    AutoMutex lock(mLock);
    LOGV("%s - %d\n", __FUNCTION__, enable);
    fd = openDevice((char*)DEVICE_NAME);
    sprintf(command, "%s\r\n", (char*)AT_COMMAND_OPEN_NORMAL_MONO_REC);
    ret = sendAtCommand(fd, command);
    openRecDevice((char*)REC_DEVICE_NAME);
    bRecordState = true;
#if defined(DUMP_RecordData)
    {
        struct tm *timeinfo;
        time_t rawtime;
        char path[80];
        int32 i4format;
        time(&rawtime);
        timeinfo=localtime(&rawtime);
        memset((void*)path,0,80);
        strftime (path,80,"/sdcard/2ndStreamIn_%a_%b_%Y__%H_%M_%S.pcm", timeinfo);
        pRecFile = fopen(path,"w");
        if(pRecFile == NULL){
            LOGE("open %s file error\n",path);
        }else{
            LOGD("open %s file success\n",path);
        }
    }
#endif
    return true;
}

int AudioATCommand::Spc_CloseNormalRecPath()
{
    char command[BUFFER_SIZE];
    int fd;
    int ret;
    AutoMutex lock(mLock);
    LOGV("%s - %d\n", __FUNCTION__, enable);
    fd = openDevice((char*)DEVICE_NAME);
    sprintf(command, "%s\r\n", (char*)AT_COMMAND_CLOSE_NORMAL_REC);
    ret = sendAtCommand(fd, command);
    //Close Record Thread
    bRecordState = false;
    closeRecDevice();
#if defined(DUMP_RecordData)
    if(pRecFile!=NULL)
    {
        fclose(pRecFile);
    }
#endif
    return ret;
}

int  AudioATCommand::Spc_ReadRNormalRecData(uint16 *pBuf, uint16 u2Size)
{
    int fd = 0, count = 0, try_count = 5;
    uint16 u2RemainCount = u2Size;
    int u2idx_w=0;
    AutoMutex lock(mLock);

    if (fRecHdl<=0)
    {
        LOGD("Record device is not opened.");
        usleep(20*1000);
        return 0;
    }

    while(try_count > 0)
    {
        count = read(fRecHdl, (uint8*)pBuf+u2idx_w, u2RemainCount/*MAX_AT_RESPONSE*/);
        LOGV("Spc_ReadRNormalRecData Request %d, Get %d\n",u2RemainCount, count);
        if(count>0)
        {
#if defined(DUMP_RecordData)
            if(pRecFile!=NULL){
                fwrite((uint8*)pBuf+u2idx_w,count,1,pRecFile);
                LOGV("write size = %d\n",count);
            }
#endif
            u2RemainCount -= count;
            u2idx_w+=count;
        }
        else
        {
           try_count--; //Try again
           usleep(20*1000);
        }
        //Check if get all data
        if(u2RemainCount == 0){
            break;
        }
    }
    if(try_count<=0){
        LOGE("%s - [Error]Unable to read VR data from ExtMD\n", __FUNCTION__);
    }
    return (u2Size - u2RemainCount);//actually read size
}

int  AudioATCommand::Spc_GetParameters(ENUM_SPC_GET_PARAMS enum_idx)
{
    int i4ret = 0;
    AutoMutex lock(mLock);
    switch(enum_idx){
        case SPC_GET_PARAMS_SR:
        {
            i4ret = MT6252_REC_SR;
            break;
        }
        case SPC_GET_PARAMS_CH:
        {
            i4ret = MT6252_REC_CH;
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
