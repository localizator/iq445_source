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
** Copyright 2008, The Android Open Source Project
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

#define LOG_TAG "MtkCam/MCamHw"
//
#include <sys/types.h>
#include <semaphore.h>
#include <linux/rtpm_prio.h>
//
#include "mHalCamUtils.h"
//
#include "Utils/inc/CamUtils.h"
using namespace android;
using namespace MtkCamUtils;


/******************************************************************************
*
*******************************************************************************/
#define MY_LOGV(fmt, arg...)    CAM_LOGV("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)    CAM_LOGD("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)    CAM_LOGI("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)    CAM_LOGW("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)    CAM_LOGE("[%s] "fmt, __FUNCTION__, ##arg)


/******************************************************************************
* save packed image file to unpacked image file 
*******************************************************************************/
bool saveUnpackRaw(char *packFileName, int width, int height, int bitDepth, int order, int rawSize, int isPacked )
{
    //
    char fname[128];
    int32_t len = strlen((char *) packFileName);
    // XXXXX__WxH_Bitdepth_colororder.raw
    memcpy(fname, packFileName, len - 4);
    sprintf(fname + len - 4, "__%dx%d_%d_%d.raw",
                 width, height, bitDepth, order);

    FILE *fp = fopen((char *)packFileName, "rb");
    if (NULL == fp) {
        MY_LOGE(" Can not open packed file =%s\n", packFileName);
        return false;
    }

    char *pBuf = (char *) malloc (width * height * 2);
    char *packedBuf = (char*)(((uint32_t)pBuf + width * height * 2 - rawSize) & (~0x1));

    if (NULL == pBuf) {
        MY_LOGE("Not enough memory for unpacking \n");
        return false;
    }

    fread(packedBuf, rawSize, 1, fp);
    fclose(fp);

    uint32_t *pu4SrcBuf = (uint32_t *) packedBuf;
    uint16_t *pu2DestBuf = (uint16_t *)pBuf;

    if (isPacked == 1) {
        while (pu2DestBuf < (uint16_t *)pBuf + width * height) {
            uint32_t u4Pixel = *(pu4SrcBuf++);
            *(pu2DestBuf++) = (uint16_t)(u4Pixel & 0x03FF);
            *(pu2DestBuf++) = (uint16_t)((u4Pixel >> 10) & 0x03FF);
            *(pu2DestBuf++) = (uint16_t)((u4Pixel >> 20) & 0x03FF);
        }
    }
    else {
        memcpy(pu2DestBuf, packedBuf, width * height  * 2);
    }

    FILE *pWp = fopen(fname, "wb");

    if (NULL == pWp ) {
        MY_LOGE("Can not open unpacked file =%s\n", fname);
        free(pBuf); 
        return false;
    }

    int32_t i4WriteCnt = fwrite(pBuf, 1, width * height * 2, pWp);

    MY_LOGD("Save unpacked image file name:%s\n", fname);

    fclose(pWp);

    free(pBuf);
    // delete packed file
    if( remove( packFileName ) != 0 ) {
        MY_LOGE("Error deleting file sorry");
    }
    return true; 
}


/******************************************************************************
* get the time by us 
*******************************************************************************/
long long getTimeUs()
{
    static struct timeval t1;
    static int isInit = 0;
    unsigned long us;
    struct timeval t2;

    if (isInit == 0) {
        gettimeofday(&t1, NULL);
        isInit = 1;
    }
    gettimeofday(&t2, NULL);

    us = (t2.tv_sec - t1.tv_sec) * 1000 * 1000;
    if (t2.tv_usec >= t1.tv_usec) {
        us += (t2.tv_usec - t1.tv_usec);
    }
    else {
        us -= (t1.tv_usec - t2.tv_usec);
    }

    return us;
} 

/******************************************************************************
* get the time by ms 
*******************************************************************************/
long long getTimeMs()
{
    struct timeval t1;
    long long ms;

    gettimeofday(&t1, NULL);
    ms = t1.tv_sec * 1000LL + t1.tv_usec / 1000;

    return ms;
}


/******************************************************************************
* convert file name w/ index append at the end 
*******************************************************************************/
void convertFileName(char *pdest, const char *psrc, int index)
{
    char *ptr = pdest;

    if ((pdest == NULL) || (psrc == NULL)) {
        return;
    }
    strcpy(pdest, psrc);
    // start from last two bytes
    ptr += strlen(psrc) - 2;
    sprintf(ptr, "%02d", index);

    CAM_LOGD("[convertFileName] %s, %s, %d \n", pdest, psrc, index);
}

/******************************************************************************
  1. If wants to call SaveToFileWait to save file, must wait quit meg when capture done
  2. When need to limit buffer num, modify FILE_BUFF_MAX,
*******************************************************************************/
/******************************************************************************
*
*******************************************************************************/
static uint8_t *FileGetBuff(uint32_t size)
{
   if(size != 0){
      return (uint8_t *)malloc(size);
   }
   return 0;
}

/******************************************************************************
*
*******************************************************************************/
#define FILE_BUFF_MAX (16+1)
#define FILE_NAME_MAX (128+1)

typedef struct{
    unsigned int Front;
    unsigned int Rear;
    unsigned int cmd[FILE_BUFF_MAX]; //1: finish flag
    unsigned int size[FILE_BUFF_MAX];
    char *buff[FILE_BUFF_MAX];
    char filename[FILE_BUFF_MAX][FILE_NAME_MAX];
}tFileMsg;

static sem_t FileSemInt, FileSemDone;
static tFileMsg FileMsg;
static pthread_t FileSaveHandle;
static volatile int WaitThreadFlag = 0;

/******************************************************************************
 *
*******************************************************************************/
static int FileMsgInsQueue(tFileMsg *pCmd, unsigned int cmd, unsigned int size, char* buff, char *filename)
{
    int SemValue, Policy, OldPriority;
    
    CAM_LOGD("[FileMsgInsQueue]Put:Front=%d, Rear=%d\n",pCmd->Front,pCmd->Rear);
    sem_getvalue(&FileSemInt,&SemValue);
    if(SemValue >= FILE_BUFF_MAX){
        CAM_LOGD("[FileMsgInsQueue]Queue is full\n");
        return 0;
    }
    pCmd->Rear = (pCmd->Rear+1)%FILE_BUFF_MAX;
    pCmd->cmd[pCmd->Rear] = cmd;
    pCmd->size[pCmd->Rear] = size;
    pCmd->buff[pCmd->Rear] = buff;
    strncpy(pCmd->filename[pCmd->Rear], filename, FILE_NAME_MAX);
    sem_post(&FileSemInt);

    if(cmd)
    {
        //FIXME, replace it by setPriority() and getPriority() 
        CAM_LOGD("[FileMsgInsQueue]Burst shot is done, pull up save file thread priority.\n");
        setPriority(SCHED_RR, RTPM_PRIO_CAMERA_COMPRESS);
    }
    return 1;
}

/******************************************************************************
 *
*******************************************************************************/
static int FileMsgDelQueue(tFileMsg *pCmd, unsigned int *size, char **buff, char *filename, unsigned int *cmd)
{
    int SemValue;

    sem_getvalue(&FileSemInt,&SemValue);
    CAM_LOGD("[FileMsgDelQueue]Get:SemValue=%d\n",SemValue);
    if(SemValue == 0)
    {
        CAM_LOGD("[FileMsgDelQueue]Queue is empty\n");
    }
    sem_wait(&FileSemInt);
    CAM_LOGD("[FileMsgDelQueue]Get:Front=%d, Rear=%d\n",pCmd->Front,pCmd->Rear);
    pCmd->Front = (pCmd->Front+1)%FILE_BUFF_MAX;
    *cmd = pCmd->cmd[pCmd->Front];
    *size = pCmd->size[pCmd->Front];
    *buff = pCmd->buff[pCmd->Front];
    strncpy(filename, pCmd->filename[pCmd->Front], FILE_NAME_MAX);
    return 1;
}

/******************************************************************************
* save file thread 
*******************************************************************************/
static void* CapWaitSaveThread(void *arg)
{
    unsigned int size;
    unsigned int cmd;
    char *buff;
    char FileName[FILE_NAME_MAX];

    //  thread policy & priority
    //  Notes:
    //      Even if pthread_create() with SCHED_OTHER policy, a newly-created thread 
    //      may inherit the non-SCHED_OTHER policy & priority of the thread creator.
    //      And thus, we must set the expected policy & priority after a thread creation.
    setPriority(SCHED_OTHER, 0); 
    CAM_LOGD("[CapWaitSaveThread]$ Thread start tid=%d \n", gettid());

    while (1)
    {
        int policy = 0, priority = 0;  
        getPriority(policy, priority); 
        CAM_LOGD("[CapWaitSaveThread]$ (tid, policy, priority)= (%d, %d, %d) \n", gettid(), policy, priority); 
        if (FileMsgDelQueue(&FileMsg, &size, &buff, FileName, &cmd)){
            if (cmd == 1) {
                CAM_LOGD("[CapWaitSaveThread]$ Send quit command \n");
                sem_post(&FileSemDone);
                WaitThreadFlag = 0;
                CAM_LOGD("[CapWaitSaveThread]$ kill capwaitSaveThread \n");
                pthread_exit(NULL);
                //CAM_LOGD("[CapWaitSaveThread][FileMsgDelQueue]$ killed \n");
                continue;
            }
            CAM_LOGD("[CapWaitSaveThread]$ Try to save to file. buffer=%x, filename=%d, size=%d \n",(unsigned int)buff,(unsigned int)FileName,size);
            saveBufToFile(FileName, (uint8_t *)buff, size);
            //CAM_LOGD("[CapWaitSaveThread][FileMsgDelQueue]$ Save file done \n");
            if (buff != NULL){
                free(buff);
                //CAM_LOGD("[CapWaitSaveThread][free]$ free the buffer done= %x \n", buff);
            }
        }
        //usleep(2000);
        //usleep(2000000); //for presure test, wait 2s per capture
    }
    return NULL;
}

/******************************************************************************
* create a thread to save the file at background 
*******************************************************************************/
void saveToFileWait(char *fname, uint8_t *buff, uint32_t size)
{
   uint8_t *FileBuff;

   CAM_LOGD("[SaveToFileWait]$ Start \n");
   if (!WaitThreadFlag){
      sem_init(&FileSemInt, 0, 0);
      sem_init(&FileSemDone, 0, 0);
      WaitThreadFlag = 1;
      memset(&FileMsg, 0x0, sizeof(tFileMsg));
      pthread_create(&FileSaveHandle, NULL, CapWaitSaveThread, NULL);
      CAM_LOGD("[SaveToFileWait]$ CapWaitSaveThread created done \n");
   }
   FileBuff = FileGetBuff(size);
   if (FileBuff != NULL){
       memcpy(FileBuff, buff, size);
       while(!FileMsgInsQueue(&FileMsg, 0, size, (char *)FileBuff, fname));
       CAM_LOGD("[SaveToFileWait]$ FileMsgInsQueue in queue, size=%d,buff=%x \n",size,(unsigned int)FileBuff);
  }
  else{
      CAM_LOGD("[SaveToFileWait]$!!! Alloc buff failed \n");
  }
}

/******************************************************************************
* wait file to save done 
* FIXME, should packed saveToFileWait & waitFileSaveDone
*******************************************************************************/
void waitFileSaveDone()
{
    if(WaitThreadFlag)
    {
        while(!FileMsgInsQueue(&FileMsg,1,0,NULL, (char*)"lastone"));
        sem_wait(&FileSemDone);
    }
    else
    {
        CAM_LOGW("[waitFileSaveDone]Thread is not created!");
    }
}

