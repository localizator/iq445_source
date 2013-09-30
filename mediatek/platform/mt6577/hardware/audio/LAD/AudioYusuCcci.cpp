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
 * AudioYusuCcci.c
 *
 * Project:
 * --------
 *   Android Speech Driver
 *
 * Description:
 * ------------
 *   This file implements CCCI driver for audio.
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
 * 04 11 2012 weiguo.li
 * [ALPS00264069] A52ΪػٶȵŻ
 * .
 *
 * 04 10 2012 weiguo.li
 * [ALPS00266592] [Need Patch] [Volunteer Patch] ICS_MP patchback to ALPS.ICS of Audio
 * .
 *
 * 02 10 2012 donglei.ji
 * [ALPS00101149] [Need Patch] [Volunteer Patch]AudioPlayer, AMR/AWB Playback ICS migration
 * .
 *
 * 01 18 2012 weiguo.li
 * [ALPS00113586] [Need Patch] [Volunteer Patch] modify cccithread name
 * .
 *
 * 01 11 2012 donglei.ji
 * [ALPS00106007] [Need Patch] [Volunteer Patch]DMNR acoustic loopback feature
 * .
 *
 *
 *******************************************************************************/

#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <time.h>
#include <sched.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include <linux/rtpm_prio.h>

#define LOG_TAG "AudioYusuccci"
#include <utils/Log.h>
#include <utils/String8.h>
#include <cutils/properties.h>

#include "AudioYusuCcci.h"
#include "AudioYusuHardware.h"
#include "AudioYusuLad.h"
#include "AudioYusuLadPlayer.h"
#include "AudioPcm2way.h"
#include "AudioCustParam.h"
#include "audio_custom_exp.h"
#include <sys/prctl.h>

#if defined(MTK_DUAL_MIC_SUPPORT)||defined(MTK_AUDIO_HD_REC_SUPPORT)
#define USE_DUALMIC_SDBUF				//to use sdbuffer for dual mic meta support of play+rec
#endif

#if defined(PC_EMULATION)
#include "stdio.h"
#include "stdlib.h"
#include "windows.h"
#endif

/* static allocation for real CCCI nodes */
static const char *CCCI_PCM_RX_DEV = "/dev/ccci_pcm_rx";
static const char *CCCI_PCM_TX_DEV = "/dev/ccci_pcm_tx";
static const char *MD_INIT_FILE = "/sys/devices/virtual/BOOT/BOOT/boot/md";

// read : "/dev/ccci_pcm_rx"
// write: "/dev/ccci_pcm_tx"

#define A2MBUFLOCK_TIMEOUT  10000  //unit is ms.
#define SPHCOEFFLOCK_TIMEOUT 10000 //unit is ms.
namespace android {

bool AudioCCCI::Initial()
{
   //initial the message queue
   iQWrite = 0;
   iQRead = 0;
   qLen = AUDCCCI_MAX_QUEUE_LEN;
   CCCIEnable = true;
   b_modem_reset = false;
   m_last_write_ccci_msg = 0;
   m_last_read_ccci_msg = 0;
   m_MDReset_recover_record = 0;
   /* open CCCI and get handle */
#if defined(FAKE_CCCI)
   fHdl = 1;
   fHdlAck = 1;
   hPcmRx = 1;
#else
   fHdl = open(CCCI_PCM_RX_DEV, O_RDWR);
   fHdlAck = open(CCCI_PCM_TX_DEV, O_RDWR);
   hPcmRx = fHdl;
#endif
   YAD_LOGV("AudioCCCI Initial: fHdl:%d, fHdlAck:%d, hPcmRx:%d", fHdl, fHdlAck, hPcmRx);
   if ( fHdl < 0 || fHdlAck < 0 ) {
      YAD_LOGE("fail to open file %s or %s\n", CCCI_PCM_RX_DEV, CCCI_PCM_TX_DEV);
      return false;
   }

   //Get share buffer base and length
#if defined(FAKE_CCCI)
   mShareBufLen = CCCI_PCM_SHARE_BUF_LEN;
   mShareBufAdd = pFakeCCCI->GetBaseAddress();
#else
   ioctl(hPcmRx, CCCI_IOC_PCM_LEN, &mShareBufLen);
   mShareBufAdd = (uint32)mmap(NULL, mShareBufLen, PROT_READ | PROT_WRITE, MAP_SHARED, hPcmRx, 0);
#endif
   ALOGD("AudioCCCI Initial length:%d, mShareBufAdd:0x%x", mShareBufLen, mShareBufAdd);


   //Initialize Queue mutex
#if defined(PC_EMULATION)
   InitializeCriticalSection(&fQMutex);
#else
	int ret;
   ret = pthread_mutex_init(&fQMutex, NULL);
   ret = pthread_mutex_init(&mStreaminMutex, NULL);
   if ( ret != 0 ) { return -1; }
#endif

   ret = pthread_mutex_init(&m2WayRecMutex, NULL);
   ret |= pthread_mutex_init(&m2WayPlayMutex, NULL);
   ret |= pthread_mutex_init(&mA2MShareBufMutex, NULL);
   ret |= pthread_mutex_init(&mSetSpeechCoeffMutex, NULL);
   if ( ret != 0 )
   {
      ALOGD("Initial, m2WayReadMutex / m2WayWriteMutex / mA2MShareBufMutex / mSetSpeechCoeffMutex error");
      return -1;
   }

   /* create the CCCI event reading thread */
   return CreateReadingThread();
}

bool AudioCCCI::WaitUntilModemReady()
{
    char status = 0;
    int trycnt = 0;
    do {
        status = GetModemCurrentStatus();
        if (status == '2') {
            ALOGD("%s(): Modem Ready", __FUNCTION__);
            break;
        }
        else {
            ALOGW("Wait CCCI open #%d times, modem current status = %c", ++trycnt, status);
            usleep(100*1000);
            if (trycnt == 100) break;
        }
    } while (1);

    return (trycnt < 100) ? true : false;
}

bool AudioCCCI::Deinitial()
{
   int ret =0;
   /* open CCCI and get handle */
   CCCIEnable = false;

#if defined(FAKE_CCCI)
   fHdl = 1;
   fHdlAck = 1;
   hPcmRx = 1;
#else
   ret = close(fHdl);
   ret = close(fHdlAck);
#endif

   YAD_LOGV("AudioCCCI Deinit Success \n");
   return true;
}


char AudioCCCI::GetModemCurrentStatus()
{
    // '0': Boot stage 0 -> Means MD Does NOT run
    // '1': Boot stage 1 -> Means MD begin to run, but not ready
    // '2': Boot stage 2 -> Means MD is ready
    // '3': MD exception -> Means EE occur

    int fd = open(MD_INIT_FILE, O_RDONLY);
    if (fd < 0) {
        ALOGE("GetModemCurrentStatus: fail to open %s: errno: %d ", MD_INIT_FILE, errno);
        return '3';
    }

    char status = 0;
    read(fd, (void *)&status, sizeof(char));
    close(fd);

    return status;
}


int AudioCCCI::GetTxFd()
{
    return fHdl;
}
int AudioCCCI::GetRxFd()
{
    return fHdlAck;
}

uint32 AudioCCCI::GetM2AShareBufLen()
{
   int32 ret;

   // m2a buffer lengh should be half of mShareBufLen
   ret = mShareBufLen >> 1;
/*
   // Limit the length
   if ( ret > M2A_SHARE_BUF_LEN )
      ret = M2A_SHARE_BUF_LEN;
*/
   return ret;
}

uint32 AudioCCCI::GetA2MShareBufLen()
{
   int32 ret;

   // a2m buffer lengh should be half of mShareBufLen
   ret = mShareBufLen >> 1;
/*
   // Limit the length
   if ( ret > A2M_SHARE_BUF_LEN )
      ret = A2M_SHARE_BUF_LEN;
*/
   return ret;
}

uint32 AudioCCCI::GetM2AShareBufAddress()
{
   //Lower part
   return mShareBufAdd + (mShareBufLen >> 1);
}

uint32 AudioCCCI::GetA2MShareBufAddress()
{
   //higher part
   return mShareBufAdd;
}

//Write a message, the thread is pended until the message is received by MD
int AudioCCCI::SendMessage(uint32 MSG)
{
    int ret;
    CCCI_BUFF_T msgBuf;
    msgBuf.data[1] = MSG;

    /* check if already initialized */
    if (fHdl == 0 || fHdlAck == 0) {
        if(!Initial())
        return false;
    }

    // keep the last CCCI MSG that sent to modem side
    m_last_write_ccci_msg = MSG;

    /* send message */
#if defined(FAKE_CCCI)
    ret = pFakeCCCI->SendMessage(&msgBuf.data[0]);
#else
    //actuall send message
    CCCI_INIT_MAILBOX(&msgBuf, MSG);
    ret = write(fHdlAck, (void*)&msgBuf, sizeof(CCCI_BUFF_T));
#endif


    if (ret <= 0) {
        char status;
        if (errno == 3) {  //errno==3: md reset cause ccci message fail
            ALOGE("SendMessage MD RESET Skip Message (0x%x)(ret:%d)(errno:%d)", MSG, ret, errno);
        }
        else if ((status = GetModemCurrentStatus()) != '2') {  // modem reset
            ALOGE("SendMessage MD RESET Skip Message (0x%x)(ret:%d)(modem current status:%c)", MSG, ret, status);
        }
        else { // try for 150 times for every 2 ms
            int32 i = 150;
            while(i--)
            {
                ALOGE("SendMessage re-try (0x%x)(try:%d)(ret:%d)(errno:%d)", MSG, i, ret, errno);
                AAD_Sleep_1ms(2);
#if defined(FAKE_CCCI)
                ret = pFakeCCCI->SendMessage(&msgBuf.data[0]);
#else
                //actuall send message
                ret = write(fHdlAck, (void*)&msgBuf, sizeof(CCCI_BUFF_T));
#endif
                if (ret > 0 || errno == 3 || GetModemCurrentStatus() != '2') break;
            }
        }
    }

    return ret;
}



int AudioCCCI::ReadMessage(uint32 *msg)
{
   CCCI_BUFF_T msgBuf;
   int ret;
   uint32 read_ccci_MSG;
   int32 need_ack;
   ALOGV("+ReadMessage");

   /* check if already initialized */
   if( fHdl == 0 || fHdlAck == 0 )   {
      ALOGD("fHdl == 0 || fHdlAck == 0");
      if(!Initial())
         return false;
   }

   /* read message */
#if defined(FAKE_CCCI)
   ret = pFakeCCCI->ReadMessage(&msgBuf.data[0]);
#else
   //actuall read message
   ALOGV("+ReadMessage, read msgBuf=%x",&msgBuf);
   ret = read(fHdl, (void*)&msgBuf, sizeof(CCCI_BUFF_T));
   ALOGV("-ReadMessage, read");
#endif

   /* extract */
   *msg = CCCI_MAILBOX_ID(&msgBuf);
   read_ccci_MSG = *msg;
   if(ret < 0){
      ALOGD("ReadMessage, no ccci msg(0x%x),ret(%d)",read_ccci_MSG,ret);
      return ret;
   }

   // record the last Read CCCI message. (only record the need_ack CCCI message.)
   need_ack = JudgeAckOfMsg(read_ccci_MSG);
   if(need_ack ==1){
      m_last_read_ccci_msg = read_ccci_MSG;
   }

   ALOGV("-ReadMessage");
   return ret;
}

int32 AudioCCCI::GetQueueCount()
{
   int32 count = (iQWrite - iQRead);
   if( count < 0 ) count += qLen;
   return count;
}

bool AudioCCCI::CheckOffsetandLength(uint32 msg)
{
    //ALOGD("offset =%d length = %d msg = %x",CCCI_MESSAGE_OFFSET_PARSER(msg),CCCI_MESSAGE_LENGTH_PARSER(msg),msg);
    if((CCCI_MESSAGE_OFFSET_PARSER(msg) > GetM2AShareBufLen()) ||CCCI_MESSAGE_LENGTH_PARSER(msg) > GetM2AShareBufLen()){
        ALOGE("offset =%d length = %d msg = %x",CCCI_MESSAGE_OFFSET_PARSER(msg),CCCI_MESSAGE_LENGTH_PARSER(msg),msg);
        return false;
    }
    else{
        return true;
    }
}

int32 AudioCCCI::searchPrevAckQElement(int32 idx)
{
   // if the queue is empty
   if(idx == iQRead){
      YAD_LOGV("searchPrevAckQElement Queue empty, idx:%d, iQRead:%d \n",idx,iQRead);
      return idx;
   }

   //start from idx--, which means the previous index
   idx--;
   if( idx == -1 ) idx += qLen;

   // We can count the iQRead, which has already been sent
   int32 searchEnd = iQRead -1;
   if (searchEnd == -1) searchEnd += qLen;

   while(idx != searchEnd )
   {
      if ( pQueue[idx].ack == 1 )      // search valid ack message only, exclude 8 case
         break;
      idx--;
      if( idx == -1 ) idx += qLen;
   }
   return idx;
}

int32 AudioCCCI::JudgeAckOfMsg(int32 MSG)
{
// Todo:
// Need add
// AUDIO_CUSTOM_PARAMETER / AUDIO_WB_CUSTOM_PARAMETER / AUDIO_DUAL_MIC_CUSTOM_PARAMETER
// A2M_CTM_Open / A2M_CTM_Close
//

// for MT6516 / MT6573 modem side difference.
// MT6573, A2M_BGSND_Open/A2M_BGSND_Close/A2M_PCM_OPEN/A2M_PCM_Close, these msg need ack from modem.

   int32 function = CCCI_MESSAGE_FUNCTION_PARSER(MSG);
   if ( function == A2M_Record_turn_on  ||
        function == A2M_Record_turn_off ||
        function == A2M_Speech_On       ||
        function == A2M_Speech_Off      ||
//#if defined(MT6573)
// Enable this code when modem side check in.
        function == A2M_BGSND_Open ||
        function == A2M_BGSND_Close ||
        function == A2M_CTM_Open ||
        function == A2M_CTM_Close ||
        function == AUDIO_CUSTOM_PARAMETER ||
        function == AUDIO_WB_CUSTOM_PARAMETER ||
        function == AUDIO_DUAL_MIC_CUSTOM_PARAMETER ||
        function == A2M_SetSphEnhancementAll ||
        //function == AUDIO_HD_RECORD_PARAMETER || // AUDIO_HD_RECORD_PARAMETER == AUDIO_WB_CUSTOM_PARAMETER
//#endif
        function == A2M_SetAfeLoopback ||
        function == A2M_SetAcousticLoopback ||
        function == A2M_SetSpeechMode        ) {
//      ALOGD("JudgeAckOfMsg (%x)(1)\n",function);
      return 1;
   }
   else{
      //ALOGD("JudgeAckOfMsg (%x)(0)\n",function);
      return 0;
   }
}


int AudioCCCI::SendMessageInQueue(uint32 MSG)
{
   //Enter critical section for message queue, prevent multiple manipulation of queue
   MsgQueueLock();

   int32 count = GetQueueCount();
   int32 ack = JudgeAckOfMsg(MSG);
   int ret;

   // for the modem reset case.
   if(b_modem_reset == true)
   {
      ALOGD(AUDMSG"SendMessageInQueue,MD Reset(0x%x) \n",MSG);
      uint32 func = CCCI_MESSAGE_FUNCTION_PARSER(MSG);
      ALOGD(AUDMSG"SendMessageInQueue,MD Reset msg(0x%x) \n", func);
      if( (A2M_Speech_On == func)      ||
          (A2M_Speech_Off == func)     ||
          (A2M_Record_turn_on == func) ||
          (A2M_Record_turn_off == func)
        )
      {
         b_modem_reset = false;
      }
   }

   if ( count == (int32)(qLen - 1) ) {                 // if the queue is full
      ALOGE(AUDMSG"SendMessageInQueue Q full(%d), last Wmsg(0x%x),Rmsg(0x%x) \n",count,m_last_write_ccci_msg,m_last_read_ccci_msg);
      ret = false;
      goto SEND_MESSAGE_IN_QUEUE_END;
   }
   else if ( count == 0 )
   {
      if(ack == 0) {                  // no need ack, send directly
         ret = SendMessage(MSG);
         goto SEND_MESSAGE_IN_QUEUE_END;
      }
      else {                          // need ack, en-queue and send message
         pQueue[iQWrite].msg = MSG;
         pQueue[iQWrite].ack = 1;
         iQWrite++;
         if ( iQWrite == qLen )
            iQWrite -= qLen;

         ret = SendMessage(MSG);
         if(ret <=0)
         {// skip this fail CCCI message
            ALOGE(AUDMSG"SendMessageInQueue,1Warn, skip this fail CCCI message(0x%x)", MSG);
            iQRead++;
            if ( iQRead == qLen )
               iQRead -= qLen;
            //Do error handling here.
            SendMsgFailErrorHandling(MSG);
         }
         goto SEND_MESSAGE_IN_QUEUE_END;
      }
   }
   else {                                       //if the queue is not empty, must queue the element

      //Do cancellation of sreious of record_on / record_off
      //only for count > 2 we can do the cancellation
      //cancel if 'on - off - on'
      if ( count > 2 )
      {
         uint32 iPreAckMsg, iPrePreAckMsg;
         //YAD_LOGV("[0]:%x [1]:%x [2]:%x [3]:%x [4]:%x [5]:%x [6]:%x [7]:%x [8]:%x [9]:%x \n"
         //   , pQueue[0].msg, pQueue[1].msg, pQueue[2].msg, pQueue[3].msg, pQueue[4].msg, pQueue[5].msg, pQueue[6].msg, pQueue[7].msg , pQueue[8].msg, pQueue[9].msg);
         //YAD_LOGV(">2 SendMessageInQueue MSG:0x%x \n", MSG);
         if (CCCI_MESSAGE_FUNCTION_PARSER(MSG) == A2M_Record_turn_on) {

            iPreAckMsg = searchPrevAckQElement(iQWrite);
            YAD_LOGV(">2 SendMessageInQueue pQueue[%d].msg:0x%x \n",iPreAckMsg, pQueue[iPreAckMsg].msg);
            if (CCCI_MESSAGE_FUNCTION_PARSER(pQueue[iPreAckMsg].msg) == A2M_Record_turn_off) {

               iPrePreAckMsg = searchPrevAckQElement(iPreAckMsg);
               YAD_LOGV(">2 SendMessageInQueue pQueue[%d].msg:0x%x \n",iPrePreAckMsg, pQueue[iPrePreAckMsg].msg);
               if (CCCI_MESSAGE_FUNCTION_PARSER(pQueue[iPrePreAckMsg].msg) == A2M_Record_turn_on) {

                  pQueue[iPreAckMsg].ack = 8;  //cancel previous
                  pQueue[iPreAckMsg].msg = 0xff;  //cancel previous

                  // Record on-off-on, and the last off-on were be canceled.
                  // Because the last off will be canceled, the clock need to be turn off here.
                  mHw->SwitchAudioClock(false);

                  ret = true;
                  goto SEND_MESSAGE_IN_QUEUE_END;
               }
            }
         }
      }

      //En-queue the message
      pQueue[iQWrite].msg = MSG;
      pQueue[iQWrite].ack = ack;
      iQWrite++;
      if ( iQWrite == qLen ) iQWrite -= qLen;
      ret = true;
      count = GetQueueCount();
      YAD_LOGV(AUDMSG "Send_Msg to Q(0x%x), cnt(%d)\n", MSG,count);
   }

SEND_MESSAGE_IN_QUEUE_END:
   MsgQueueUnLock();

	return (ret>0) ? true : false;
}

bool AudioCCCI::ConsumeMessageInQueue()
{
   //Enter critical section for message queue, prevent multiple manipulation of queue
   MsgQueueLock();

   int32 count = GetQueueCount();
   if(count > 5)
      ALOGW(AUDMSG"ConsumeMsgInQ, Cnt=%d",count);

   bool ret;
   if ( count == 0 ) {
      YAD_LOGE("CCCI Q(%d): No Msg in Q \n",count);
      ret = false;
      goto CONSUME_MESSAGE_IN_QUEUE_END;
   }

   while ( 1 )
   {
      // when entering this function, the first message in queue must be a message waiting for ack
      // so we increment index, consuming the first message in queue
      iQRead++;
      if ( iQRead == qLen ) iQRead -= qLen;

      // check if empty
      if ( iQRead == iQWrite ) {
         ret = true;
         goto CONSUME_MESSAGE_IN_QUEUE_END;
      }

      // send message
      if (pQueue[iQRead].ack == 1) {            // the message need ack

         ALOGD(AUDMSG"ConsumeMsgInQ,ack (0x%x),cnt(%d) \n",pQueue[iQRead].msg,count);
         ret = SendMessage(pQueue[iQRead].msg);
         if(ret <= 0){
            // Send CCCI msg Fail
            ALOGE(AUDMSG"ConsumeMsgInQ,1:Warn, Send CCCI msg Fail(0x%x)", pQueue[iQRead].msg);

            //Do error handling here.
            SendMsgFailErrorHandling(pQueue[iQRead].msg);
         }
         else{
            // Send CCCI msg Sucess
            goto CONSUME_MESSAGE_IN_QUEUE_END;
         }
      }
      else if (pQueue[iQRead].ack == 0) {       // the message no need ack
         ret = SendMessage(pQueue[iQRead].msg);
         if ( ret <= 0 ){
            // Send CCCI msg Fail
            ALOGE(AUDMSG"ConsumeMsgInQ,2:Warn, Send CCCI msg Fail(0x%x)", pQueue[iQRead].msg);
            //goto CONSUME_MESSAGE_IN_QUEUE_END;
         }
      }
      else {                                    // the cancelled message, set ret to true
         ret = true;
      }
   }

CONSUME_MESSAGE_IN_QUEUE_END:
   MsgQueueUnLock();
   return ret;
}

bool AudioCCCI::MDReset_CheckMessageInQueue()
{
   MsgQueueLock();
   uint32 func;
   int32 count = GetQueueCount();
   ALOGD(AUDMSG"CheckMessageInQueue QueueCount=%d",count);
   bool ret=true;
   if ( count == 0 ) {
      ALOGD(AUDMSG"CheckMessageInQueue, 1No Msg \n");
      ret = false;
      goto CHECK_MESSAGE_IN_QUEUE_END;
   }
   // check if empty
   if ( iQRead == iQWrite ) {
      ALOGD(AUDMSG"CheckMessageInQueue, 2No Msg \n");
      ret = true;
      goto CHECK_MESSAGE_IN_QUEUE_END;
   }

   while(1)
   {
      // Modem already reset.
      // Check every CCCI message in queue.
      // These messages that are sent before modem rest, don't send to modem.
      // But AP side need to do related action to make AP side in the correct state.

      // check if empty
      if ( iQRead == iQWrite ) {
         ALOGD(AUDMSG"CheckMessageInQueue, check Msg done \n");
         ret = true;
         goto CHECK_MESSAGE_IN_QUEUE_END;
      }

      // Need ack message. But modem reset, So simulate that the modem send back ack msg.
      SendMsgFailErrorHandling(pQueue[iQRead].msg);

      iQRead++;
      if ( iQRead == qLen ) iQRead -= qLen;

   }

CHECK_MESSAGE_IN_QUEUE_END:
   MsgQueueUnLock();
   return ret;
}

bool AudioCCCI::MDReset_FlushMessageInQueue()
{
   //Enter critical section for message queue, prevent multiple manipulation of queue
   MsgQueueLock();
   int32 count = GetQueueCount();
   ALOGD(AUDMSG"FlushMessageInQueue QueueCount=%d",count);
   bool ret=true;
   if ( count == 0 ) {
      YAD_LOGE(AUDMSG"FlushMessageInQueue, No Msg. \n");
      ret = false;
      MsgQueueUnLock();
      return ret;
   }
   iQWrite = 0;
   iQRead = 0;

   MsgQueueUnLock();
   return ret;
}

#if defined(MTK_DUAL_MIC_SUPPORT)||defined(MTK_AUDIO_HD_REC_SUPPORT)
void AudioCCCI::DualMic_WriteDataToRecBuf(int32 msg)
{
    rb rBuffer;
    char* lBuffer;
    int8 *ptrW, *pEnd;
    int32 bufLen;
    rb* pRecBuf = &mHw->recDmsdBuf;


    // ring buffer set up
    int8* pBase = (int8*)this->GetM2AShareBufAddress();
    bufLen = this->GetM2AShareBufLen();
    lBuffer = new char[bufLen];                     //local linear buffer

    rBuffer.bufLen = bufLen;
    rBuffer.pBufBase = pBase;
    rBuffer.pRead = pBase + CCCI_MESSAGE_OFFSET_PARSER(msg);
    ptrW = rBuffer.pRead + CCCI_MESSAGE_LENGTH_PARSER(msg);
    pEnd = pBase + bufLen;
    if ( ptrW > pEnd ) {
        ptrW -= bufLen;
    }
    rBuffer.pWrite = ptrW;

    // copy to linear buffer
    int cntInBuf;
    memset(lBuffer, 0, bufLen);
    cntInBuf = rb_getDataCount(&rBuffer);


    //ALOGD("rb buf length: %d, ptrB:%p, ptrR:%p, prtW:%p, count:%d",
    //    rBuffer.bufLen, rBuffer.pBufBase, rBuffer.pRead, rBuffer.pWrite, cntInBuf);

    rb_copyToLinear(lBuffer, &rBuffer, cntInBuf);

    //eat header
    char *plBuf = lBuffer;
    plBuf += LAD_SHARE_HEADER_LEN;        //eat header
    cntInBuf -= LAD_SHARE_HEADER_LEN;

    //copy to record buffer
    pthread_mutex_lock(&mHw->mDmsdBufMutex);
    {
       int playBufFreeCnt = pRecBuf->bufLen - rb_getDataCount(pRecBuf) -1;
       if ( playBufFreeCnt >= cntInBuf ) {
           ALOGD("read from share buffer: %p, count: %d, bufFree: %d", plBuf, cntInBuf, playBufFreeCnt);
           if (pRecBuf->pBufBase)                     //make sure rec buffer exist
               rb_copyFromLinear(pRecBuf, plBuf, cntInBuf);
       }
       else {
           ALOGE(" record buffer overflow, do not copy");
       }
    }
    pthread_mutex_unlock(&mHw->mDmsdBufMutex);

    //send back
    mHw->pLad->LAD_PCM2WayDataSendBack();

    delete lBuffer;


}

void AudioCCCI::DualMic_ReadDataFromPlayBuf()
{
    int8* pShareBuf = (int8 *)this->GetA2MShareBufAddress();
    rb* pPlayBuf = &mHw->playDmsdBuf;
    int pcm2WayBufSize = (mHw->mDualMicMetaWB == TRUE)? PCM2WAY_BUF_SIZE*2 : PCM2WAY_BUF_SIZE;
    int dataCnt = pcm2WayBufSize;

    pShareBuf += A2M_SHARED_BUFFER_OFFSET;    //offset

    //writing header
    int16* pBuf16 = (int16*)pShareBuf;
    pBuf16[0] = (int16)(A2M_BUF_HEADER);
    pBuf16[1] = (int16)(LADBUFID_PCM_FillSpk);
    pBuf16[2] = (int16)(dataCnt);
    pShareBuf += LAD_SHARE_HEADER_LEN;

    //reading data from playBuf and WRITE TO share buffer directly
    pthread_mutex_lock(&mHw->mDmsdBufMutex);
    int playBufCnt = rb_getDataCount(pPlayBuf);
    if (playBufCnt >= pcm2WayBufSize) {
        dataCnt = pcm2WayBufSize;
    }
    else {
        dataCnt = 0;
        ALOGE("play buffer underflow, will fill zeros");
    }
    ALOGD(" Write to share buffer: buffer: %p, count: %d, bufAvailable: %d ",pShareBuf, dataCnt, playBufCnt);
    memset(pShareBuf, 0, pcm2WayBufSize);      // make sure buffer is zero if fp read to file end
    if (pPlayBuf->pBufBase)                                       //make sure play buffer exist
       rb_copyToLinear(pShareBuf, pPlayBuf, dataCnt);
    dataCnt = pcm2WayBufSize;                         // always put 320 or 640 to LAD
    pthread_mutex_unlock(&mHw->mDmsdBufMutex);

    //ALOGD("A2M data cnt:%d, [0x%x], [0x%x], [0x%x], [0x%x], [0x%x], [0x%x], [0x%x], [0x%x]", rcnt,
    //    pShareBuf[0], pShareBuf[1], pShareBuf[2], pShareBuf[3], pShareBuf[4], pShareBuf[5], pShareBuf[6], pShareBuf[7]);

    //send back
    int offset = A2M_SHARED_BUFFER_OFFSET;
    int len = dataCnt + LAD_SHARE_HEADER_LEN;
    mHw->pLad->LAD_PCM2WayDataNotify(offset, len);

}

void AudioCCCI::DualMic_WriteDataToFile(int32 msg)
{
    rb rBuffer;
    char* lBuffer;
    int8 *ptrW, *pEnd;
    int32 bufLen;
    FILE* fp = mHw->m_DualMic_pOutFile;  //file for output


    // ring buffer set up
    int8* pBase = (int8*)this->GetM2AShareBufAddress();
    bufLen = this->GetM2AShareBufLen();
    lBuffer = new char[bufLen];                     //local linear buffer

    rBuffer.bufLen = bufLen;
    rBuffer.pBufBase = pBase;
    rBuffer.pRead = pBase + CCCI_MESSAGE_OFFSET_PARSER(msg);
    ptrW = rBuffer.pRead + CCCI_MESSAGE_LENGTH_PARSER(msg);
    pEnd = pBase + bufLen;
    if ( ptrW > pEnd ) {
        ptrW -= bufLen;
    }
    rBuffer.pWrite = ptrW;

    // copy to linear buffer
    int cntInBuf;
    memset(lBuffer, 0, bufLen);
    cntInBuf = rb_getDataCount(&rBuffer);


    //ALOGD("rb buf length: %d, ptrB:%p, ptrR:%p, prtW:%p, count:%d",
    //    rBuffer.bufLen, rBuffer.pBufBase, rBuffer.pRead, rBuffer.pWrite, cntInBuf);


    rb_copyToLinear(lBuffer, &rBuffer, cntInBuf);

    //eat header
    char *plBuf = lBuffer;
    plBuf += LAD_SHARE_HEADER_LEN;        //eat header
    cntInBuf -= LAD_SHARE_HEADER_LEN;

    //write to file
    if (cntInBuf > 0 && fp) {
        ALOGD("Write to file: %p, count:%d", fp, cntInBuf);
        fwrite(plBuf, sizeof(char), cntInBuf, fp);
    }
    else {
        ALOGW("DualMic_WriteDataToFile: got no data to write to file");
    }

    //send back
    mHw->pLad->LAD_PCM2WayDataSendBack();

    delete lBuffer;


}

void AudioCCCI::DualMic_ReadDataFromFile()
{
    int8* pShareBuf = (int8 *)this->GetA2MShareBufAddress();
    int dataCnt = PCM2WAY_BUF_SIZE;
    FILE* fp = mHw->m_DualMic_pInFile;                 //file for input
    pShareBuf += A2M_SHARED_BUFFER_OFFSET;    //offset

    //writing header
    int16* pBuf16 = (int16*)pShareBuf;
    pBuf16[0] = (int16)(A2M_BUF_HEADER);
    pBuf16[1] = (int16)(LADBUFID_PCM_FillSpk);
    pBuf16[2] = (int16)(dataCnt);
    pShareBuf += LAD_SHARE_HEADER_LEN;

    //reading data from file and WRITE TO share buffer directly
    ALOGD("Write to share buffer: %p, count: %d, fp:%p", pShareBuf, dataCnt, fp);
    if (fp) {
        int rcnt;
        memset(pShareBuf, 0, dataCnt);      // make sure buffer is zero if fp read to file end
        rcnt = fread(pShareBuf, sizeof(char), dataCnt, fp);
        //ALOGD("A2M data cnt:%d, [0x%x], [0x%x], [0x%x], [0x%x], [0x%x], [0x%x], [0x%x], [0x%x]", rcnt,
        //    pShareBuf[0], pShareBuf[1], pShareBuf[2], pShareBuf[3], pShareBuf[4], pShareBuf[5], pShareBuf[6], pShareBuf[7]);
    }

    //send back
    int offset = A2M_SHARED_BUFFER_OFFSET;
    int len = dataCnt + LAD_SHARE_HEADER_LEN;
    mHw->pLad->LAD_PCM2WayDataNotify(offset, len);

}
#endif
// --> [Stan] Fix issue: ALPS00039668, MAUI_02917371
// this modification is co-work with modem side for background sound.
// Modem side would spend some time to wait DSP BGS off. So, "BGS off ack" message need to wait some time then back to AP side.
// This would cause some issue.

// If end the call and current routing path:Speaker, it would change device from Speaker to Earpiece.
// CCCI message in Queue: BGS off -- SPH off -- Set output/input device --
//                        SPH mode -- SPH on -- BGS on -- BGS notify -- Set output volume (BGS off ack come back) -- BGS notify

// the two "BGS notify" in message queue would lead the modem ASSERT. (MSG length and Header length is not match)
// <--
bool AudioCCCI::BGS_On_MSG_ACK_CheckMessageInQueue()
{
   MsgQueueLock();
//   ALOGD("BGS_On_MSG_ACK message count before consume:%d \n", GetQueueCount());
   int32 count = GetQueueCount();
   if(count > 8)
      ALOGW("BGS_MSG_ACK, cnt=%d",count);

   bool ret;
   if ( count == 0 ) {
      YAD_LOGE("BGS_MSG_ACK, CCCI Q(%d): No Msg in Q \n",count);
      ret = false;
      goto CONSUME_MESSAGE_IN_QUEUE_END;
   }

   while ( 1 )
   {
      // when entering this function, the first message in queue must be a message waiting for ack
      // so we increment index, consuming the first message in queue
      iQRead++;
      if ( iQRead == qLen ) iQRead -= qLen;

      // check if empty
      if ( iQRead == iQWrite ) {
         ret = true;
         goto CONSUME_MESSAGE_IN_QUEUE_END;
      }

      // send message
      if (pQueue[iQRead].ack == 1) {            // the message need ack
         ALOGD("BGS_MSG_ACK, ack MSG(0x%x), Ridx(%d) \n", pQueue[iQRead].msg,iQRead);
         ret = SendMessage(pQueue[iQRead].msg);
         if(ret <= 0){
            // Send CCCI msg Fail
            ALOGE("BGS_MSG_ACK,1:Warn, Send CCCI msg Fail(0x%x)", pQueue[iQRead].msg);

            //Do error handling here.
            SendMsgFailErrorHandling(pQueue[iQRead].msg);
         }
         else{
            // Send CCCI msg Success
            goto CONSUME_MESSAGE_IN_QUEUE_END;
         }
      }
      else if (pQueue[iQRead].ack == 0) {       // the message no need ack
         ALOGD("BGS_MSG_ACK, no ack MSG(0x%x), Ridx(%d) \n", pQueue[iQRead].msg,iQRead);
         if (CCCI_MESSAGE_FUNCTION_PARSER(pQueue[iQRead].msg) == A2M_BGSND_DataNotify) {
            ALOGD("BGS_MSG_ACK, [skip](0x%x) \n", A2M_BGSND_DataNotify);
            // if A2M message=A2M_BGSND_DataNotify, Skip this message.
            ret = true;
         }
         else{
            ret = SendMessage(pQueue[iQRead].msg);
         }
         if ( ret <= 0 ){
            // Send CCCI msg Fail
            ALOGE("BGS_On_MSG_ACK,2:Warn, Send CCCI msg Fail(0x%x)", pQueue[iQRead].msg);
            //goto CONSUME_MESSAGE_IN_QUEUE_END;
         }
      }
      else {                                    // the cancelled message, set ret to true
         ret = true;
      }
   }

CONSUME_MESSAGE_IN_QUEUE_END:
   MsgQueueUnLock();
   return ret;
}


void *CCCIReadThread(void *arg)
{
   prctl(PR_SET_NAME, (unsigned long)"CCCIReadThread", 0, 0, 0);
   uint32 msg, func, numDataRequired, bufLen;
   int8 *pBase, *ptrW, *pEnd, *pA2M_ptr;
   AudioCCCI *pCCCI = (AudioCCCI*)arg;
   LADPlayer *pLadPlayer;
   AudioStreamInInterface *inStream;
   LAD *pLad;

   Play2Way   *pPlay2Way;
   Record2Way *pRecord2Way;
   int offset, length, len;
   short sync_word,data_type,data_len;
   uint32 pcm_dataRequest;

   uint32 cnt_flag = 0;

#if 1
/*
    char thread_name[20]="AUDIOFLINGER";
    int result = 0;
    result = mtk_AdjustPrio(thread_name);	 sucess:return 0; fail:return -1
    ALOGW("AudioFlinger::MixerThread::threadLoop set priority = %d",result);
    */
    int result = -1;
    // if set prority false , force to set priority
    if(result == -1){
    struct sched_param sched_p;
    sched_getparam(0, &sched_p);
    sched_p.sched_priority = RTPM_PRIO_AUDIO_CCCI_THREAD;
        if(0 != sched_setscheduler(0, SCHED_RR, &sched_p)) {
            ALOGE("[%s] failed, errno: %d", __func__, errno);
        }
        else {
            sched_p.sched_priority = RTPM_PRIO_AUDIO_CCCI_THREAD;
            sched_getparam(0, &sched_p);
            ALOGD("sched_setscheduler ok, priority: %d", sched_p.sched_priority);
        }
    }
#endif

   YAD_LOGV( "the CCCI read thread tid = %d",gettid());

   while(pCCCI->CCCIEnable)
   {
      /* read message */

      int ret = pCCCI->ReadMessage(&msg);
      //ALOGD("-CCCI audio Got message(0x%x),ret(%d)\n", msg,ret);
      cnt_flag++;
      if(ret <0){
          ALOGE("msg(0x%x), ret(%d)",msg,ret);
          usleep(10*1000);
          continue;
      }

      /* handle the message */
      func = CCCI_MESSAGE_FUNCTION_PARSER(msg);

      switch (func)
      {
         case M2A_Record_DataNotify:

            // meaning that we are recording, modem have some data
            pCCCI->RecordLock();
            // update pCCCI->pShareW, pCCCI->pShareR
            bufLen = pCCCI->GetM2AShareBufLen();
            pBase = (char*)pCCCI->GetM2AShareBufAddress();
            pCCCI->pShareR = pBase + CCCI_MESSAGE_OFFSET_PARSER(msg);
            ptrW = pCCCI->pShareR + CCCI_MESSAGE_LENGTH_PARSER(msg);
            pCCCI->CheckOffsetandLength(msg);

            pEnd = pBase + bufLen;
            if ( ptrW > pEnd ) {
   	         ptrW -= bufLen;
            }
            pCCCI->pShareW = ptrW;

            // eat header length
            pCCCI->pShareR += LAD_SHARE_HEADER_LEN;
            if ( pCCCI->pShareR > pEnd ){
   	         pCCCI->pShareR -= bufLen;
            }

            //get pointer to input stream
            inStream = pCCCI->mHw->mStreamHandler->mInput[0];
            if( inStream != NULL)
            {
            	inStream->getDataFromModem();
            }
            else
            {
               ALOGD("M2A_Record_DataNotify inStream=NULL!!!");
               if(pCCCI->mHw != NULL){
                  pLad = pCCCI->mHw->pLad;
                  pLad->LAD_NormalRecDataSendBack();
               }
            }

            pCCCI->RecordUnLock();
            break;

         case M2A_PcmOpenCompleted_Notification:
         case M2A_PcmCloseCompleted_Notification:
         {
            if(M2A_PcmOpenCompleted_Notification == func){
               ALOGD(AUDMSG"--PCM2WayOn Ack done(0x%x)\n",M2A_PcmOpenCompleted_Notification);
            }
            if(M2A_PcmCloseCompleted_Notification == func){
               ALOGD(AUDMSG"--PCM2WayOff Ack done(0x%x)\n",M2A_PcmCloseCompleted_Notification);
            }
            break;
         }
         case M2A_GetMDCapability_Notification:
         {
             ALOGD(AUDMSG"!!Aud CCCI Ack Msg Notifiy:0x%x\n", func);
             ALOGD(AUDMSG"M2A_GetMDCapability_Notification(0x%x)\n",CCCI_MESSAGE_PARAM1_PARSER(msg));
             break;
         }
         case M2A_SetMDCapability_Notification:
         {
             ALOGD(AUDMSG"!!Aud CCCI Ack Msg Notifiy:0x%x\n", func);
             ALOGD(AUDMSG"M2A_SetMDCapability_Notification(0x%x)\n",CCCI_MESSAGE_PARAM1_PARSER(msg));
             break;
         }
         case M2A_SphEnhanceSpecificCompleted_Notification:
         {
            ALOGD(AUDMSG"--LAD_Set_Speech_Enhancement_Info(0x%x) Ack done. msg(0x%x)\n", M2A_SphEnhanceSpecificCompleted_Notification, msg);
            break;
         }
         case M2A_SpeechOnCompleted_Notification:
         case M2A_SpeechOffCompleted_Notification:
         case M2A_SpeechModeCompleted_Notification:
         case M2A_RecordOnCompleted_Notification:
         case M2A_RecordOffCompleted_Notification:
         case M2A_BgsndOpenCompleted_Notification:
         case M2A_BgsndCloseCompleted_Notification:
         case M2A_CTMOpenCompleted_Notification:
         case M2A_CTMCloseCompleted_Notification:
         case M2A_AudCusParaCompleted_Notification:
         case M2A_AudDualMicParaCompleted_Notification:
         case M2A_AudWBParaCompleted_Notification:
         //case M2A_AudHdRecordParaCompleted_Notification: // M2A_AudWBParaCompleted_Notification == M2A_AudHdRecordParaCompleted_Notification ...
         case M2A_DualRecPlay_turn_on_Notification:
         case M2A_DualRecPlay_turn_off_Notification:
         case M2A_SphEnhanceAllCompleted_Notification:
         case M2A_AfeLoopbackCompleted_Notification:
         case M2A_AcousticLoopbackCompleted_Notification:
         {
            YAD_LOGV(AUDMSG"!!Aud CCCI Ack Msg(0x%x)\n", func);

            if(M2A_SpeechOnCompleted_Notification == func)
            {
               pCCCI->b_modem_reset = false;
               if(pCCCI->mHw->mVT_Speech == true)
               {
                  pCCCI->mHw->Set_Recovery_VT(true);
               }
               else
               {
                  pCCCI->mHw->Set_Recovery_Speech(true);
               }
               //[ALPS0067166]Not to enable speaker before LAD_speech_on is acked.
               // Move to ccco read thread.
               pCCCI->mHw->EnableSpeaker();
               //~By
               pCCCI->mHw->pLad->LAD_LogMDSphCoeff();
               ALOGD(AUDMSG"--Speech_On Ack done(0x%x).  tid(%d),pid(%d)\n",M2A_SpeechOnCompleted_Notification,gettid(),getpid());
            }
            if(M2A_RecordOnCompleted_Notification == func){
               pCCCI->b_modem_reset = false;
               pCCCI->mHw->Set_Recovery_Record(true);
               pCCCI->mHw->pLad->LAD_LogMDSphCoeff();
               ALOGD(AUDMSG"--Record_On Ack done(0x%x)\n",M2A_RecordOnCompleted_Notification);
            }

            if(M2A_SpeechOffCompleted_Notification == func){
               pCCCI->b_modem_reset = false;
               if(pCCCI->mHw->Get_Recovery_VT()==true){
                   pCCCI->mHw->Set_Recovery_VT(false);
                   pCCCI->mHw->SwitchAudioClock(false);
               }
               else if(pCCCI->mHw->Get_Recovery_Speech()==true){
                   pCCCI->mHw->Set_Recovery_Speech(false);
                   pCCCI->mHw->SwitchAudioClock(false);
               }else{
                   ALOGE(AUDMSG"--Speech_Off Ack is not paired\n");
               }

               int mStreamMode;
               pCCCI->mHw->getMode(&mStreamMode);
               ALOGD(AUDMSG"--Speech_Off Ack done(%x)+N Routing(0x%d)\n",M2A_SpeechOffCompleted_Notification,mStreamMode);
               if((mStreamMode != android_audio_legacy::AudioSystem::MODE_IN_CALL)){
                  pCCCI->mHw->doOutputDeviceRouting();
               }

               // Modem side would turn off Digital/Analog AFE hardware, AP side should base on different condition
               // and enable the AFE hardware.

               // If the ATV is enabled during phone call, should set SetMatvAnalogEnable again here,
               // because middleware wouldn't aware this condition.
               if(pCCCI->mHw->m_AnalogIn_ATV_Enable_during_InCall == true){
                  ALOGD(AUDMSG"--Speech_Off Ack done(0x%x)\n",M2A_SpeechOffCompleted_Notification);
                  pCCCI->mHw->SetMatvAnalogEnable(true);
               }
               //FM RX Analogin is enable
               else if(pCCCI->mHw->GetAnalogLineinEnable() == true){
                  ALOGD(AUDMSG"Speech_Off FM RX Analog On: AnalogAFE_Depop (0x%x)\n",M2A_SpeechOffCompleted_Notification);
                  if(pCCCI->mHw->mStreamHandler->GetSrcBlockRunning() == false){
                     ALOGD(AUDMSG"+AnalogAFE_Depop(FM_PATH_STEREO) ");
                     pCCCI->mHw->mAnaReg->AnalogAFE_Depop(FM_PATH_STEREO,true);
                     ALOGD(AUDMSG"-AnalogAFE_Depop(FM_PATH_STEREO) ");
                  }
                  // stream is running
                  else  if(pCCCI->mHw->mStreamHandler->GetSrcBlockRunning() == true){
                     ALOGD(AUDMSG"+AnalogAFE_Depop(FM_STEREO_AUDIO)");
                     pCCCI->mHw->mAnaReg->AnalogAFE_Depop(FM_STEREO_AUDIO,true);
                     ALOGD(AUDMSG"-AnalogAFE_Depop(FM_STEREO_AUDIO)");
                  }
               }
               // No Stream is running. Close VAUDN / VAUDP
               // Now, modem side would not close VAUDN / VAUDP, AP side should turn off them.
               else if(pCCCI->mHw->mStreamHandler->GetSrcBlockRunning() == false){
                  ALOGD(AUDMSG"+Speech_Off AnalogAFE_Depop(0x%x)\n",M2A_SpeechOffCompleted_Notification);
                  pCCCI->mHw->mAnaReg->AnalogAFE_Depop(AUDIO_PATH, false);
                  ALOGD(AUDMSG"-Speech_Off AnalogAFE_Depop(0x%x)\n",M2A_SpeechOffCompleted_Notification);
               }
#if defined(MTK_DT_SUPPORT)
               pCCCI->mHw->SpeechHwCtrlUnLock();
               ALOGD("1st MD Speech_Off ACK SpeechHwCtrlUnLock");
#endif
            }
            if(M2A_RecordOffCompleted_Notification == func){
               pCCCI->b_modem_reset = false;
               if(pCCCI->mHw->Get_Recovery_Record() == true){
                   pCCCI->mHw->Set_Recovery_Record(false);
                   pCCCI->mHw->SwitchAudioClock(false);
               }else{
                   ALOGE(AUDMSG"CheckMessageInQueue, Record_turn_off Ack is not paired\n");
               }
               ALOGD(AUDMSG"--Record_Off Ack done(0x%x)\n",M2A_RecordOffCompleted_Notification);

               // No Stream is running. Close VAUDN / VAUDP
               // Now, modem side would not close VAUDN / VAUDP, AP side should turn off them.

               // for one case, speech on (vm record on-off-on), while vm record off, maybe apply de-pop flow to disable analog register.
               if(pCCCI->mHw->mStreamHandler->GetSrcBlockRunning() == false && pCCCI->mHw->GetVoiceStatus() == false && (pCCCI->mHw->GetAnalogLineinEnable() != true)){
                  ALOGD(AUDMSG"+Record Off AnalogAFE_Depop (0x%x)\n",M2A_RecordOffCompleted_Notification);
                  pCCCI->mHw->mAnaReg->AnalogAFE_Depop(AUDIO_PATH, false);
                  ALOGD(AUDMSG"-Record Off AnalogAFE_Depop (0x%x)\n",M2A_RecordOffCompleted_Notification);
               }
            }

            if(M2A_SpeechModeCompleted_Notification == func){
               ALOGD(AUDMSG"--SetSpeechMode Ack done(0x%x).  tid(%d),pid(%d)\n",M2A_SpeechModeCompleted_Notification,gettid(),getpid());
               if( pCCCI->m_MDReset_recover_record == 1)
               {
                   pCCCI->m_MDReset_recover_record = 0;
                   pCCCI->mHw->mStreamHandler->RecordOpen();
               }
            }

            if(M2A_SphEnhanceAllCompleted_Notification == func){
               ALOGD(AUDMSG"--LAD_Set_Speech_Enhancement(0x%x) Ack done. msg(0x%x)\n", M2A_SphEnhanceAllCompleted_Notification, msg);
            }

            if(M2A_BgsndOpenCompleted_Notification == func){
               pCCCI->mHw->Set_Recovery_Bgs(true);
               ALOGD(AUDMSG"--BGSoundOn Ack done(0x%x)\n",M2A_BgsndOpenCompleted_Notification);
               pCCCI->BGS_On_MSG_ACK_CheckMessageInQueue();
               break;
            }
            if(M2A_BgsndCloseCompleted_Notification == func){
               if (pCCCI->mHw->Get_Recovery_Bgs() == true)
               {
                   pCCCI->mHw->Set_Recovery_Bgs(false);
                   pCCCI->mHw->SwitchAudioClock(false);
                   ALOGD(AUDMSG"--BGSoundOff Ack done(0x%x)\n",M2A_BgsndCloseCompleted_Notification);
               }else{
                   ALOGE(AUDMSG"--BGSoundOff Ack done(0x%x) is not paired.\n",M2A_BgsndCloseCompleted_Notification);
               }
               pCCCI->BGS_On_MSG_ACK_CheckMessageInQueue();
               break;
            }

            // ToDo
            // modem side no ack CTM on/off now.
            if(M2A_CTMOpenCompleted_Notification == func){
               ALOGD(AUDMSG"--TtyCtmOn Ack done(0x%x)\n",M2A_CTMOpenCompleted_Notification);
            }
            if(M2A_CTMCloseCompleted_Notification == func){
               ALOGD(AUDMSG"--TtyCtmOff Ack done(0x%x)\n",M2A_CTMCloseCompleted_Notification);
            }

            if(M2A_AudCusParaCompleted_Notification == func){
               ALOGD(AUDMSG"--SetSpeechPara Ack done(0x%x)\n",M2A_AudCusParaCompleted_Notification);
               pCCCI->GetMDSideSphCoefArrival(NB_SPHCOEF);
               pCCCI->A2MBufUnLock();
            }
            if(M2A_AudWBParaCompleted_Notification == func){
#if !defined(MTK_AUDIO_HD_REC_SUPPORT)
               ALOGD(AUDMSG"--SetWBSpeechPara Ack done(0x%x)\n",M2A_AudWBParaCompleted_Notification);
               pCCCI->GetMDSideSphCoefArrival(WB_SPHCOEF);
               pCCCI->A2MBufUnLock();
#else
               /// M2A_AudWBParaCompleted_Notification == M2A_AudHdRecordParaCompleted_Notification ...
               const int32 offset = CCCI_MESSAGE_OFFSET_PARSER(msg);
               uint16 *pShareBuf = (uint16 *)(pCCCI->GetA2MShareBufAddress() + offset);
               //uint16 sync = pShareBuf[0];
               uint16 type = pShareBuf[1];
               //uint16 len  = pShareBuf[2];
               if(type == LADBUFID_CCCI_EM_PARAM) { /// WB
                   ALOGD(AUDMSG"--SetWBSpeechPara Ack done(0x%x)\n",M2A_AudWBParaCompleted_Notification);
                   pCCCI->GetMDSideSphCoefArrival(WB_SPHCOEF);
                   pCCCI->A2MBufUnLock();
               }
               else if(type == LADBUFID_CCCI_HD_RECORD) { /// HD Record
                   ALOGD(AUDMSG"--SetHdRecordPara Ack done(0x%x)\n", M2A_AudHdRecordParaCompleted_Notification);
                   pCCCI->GetMDSideSphCoefArrival(HDREC_SPHCOEF);
                   pCCCI->A2MBufUnLock();
               }
#endif
            }
            if(M2A_AudDualMicParaCompleted_Notification == func){
               ALOGD(AUDMSG"--SetDualMicPara Ack done(0x%x)\n",M2A_AudDualMicParaCompleted_Notification);
               pCCCI->GetMDSideSphCoefArrival(DMNR_SPHCOEF);
               pCCCI->A2MBufUnLock();
            }

            /// Loopback : turn off audio clock after md side turn off loopback
            if (func == M2A_AfeLoopbackCompleted_Notification ||
                func == M2A_AcousticLoopbackCompleted_Notification) {
                ALOGD(AUDMSG"--SetLoopback Complete (0x%x), ack = (0x%x) \n", M2A_AfeLoopbackCompleted_Notification, msg);
                bool b_loopback_on = msg & 1;
                if (b_loopback_on == false) {
                    usleep(50000); // avoid pop
                    if (pCCCI->mHw != NULL) { // for normal use
                        pCCCI->mHw->SwitchAudioClock(false);
                    }
                    else if (pCCCI->mFd > 0) { // for factory mode
                        ::ioctl(pCCCI->mFd, AUD_SET_ADC_CLOCK, 0);
                    }
                    else {
                        ALOGE(AUDMSG"Can not close audio clock!!");
                    }
                }
            }


            //got ack message, consume message queue
            pCCCI->ConsumeMessageInQueue();
            break;
         }
         case M2A_Playback_Request:
         case M2A_BGSND_Request:

            //ALOGD("M2A_BGSND_Request ||M2A_Playback_Request ");
            //make sure write should be complete and ladbuffer cannot be deleted from audiohardware
            pthread_mutex_lock( &pCCCI->mHw->LadBufferMutex );
            if(pCCCI->mHw->pLadPlayer != NULL){
               pLadPlayer=pCCCI->mHw->pLadPlayer;
            }
            else{
               ALOGD("M2A_BGSND_Request || M2A_Playback_Request without pLadPlaye, may be stop status");
               pthread_mutex_unlock( &pCCCI->mHw->LadBufferMutex);
               break;
            }
            //parse data required
            numDataRequired = CCCI_MESSAGE_PARAM1_PARSER(msg);
//            ALOGD("(%d)M2A_BGSND_Request (0x%x)  \n",numDataRequired,msg);

            //Do mix and play in CCCI read thread
            pLadPlayer->LADPlayer_MixAndPlay(numDataRequired);
            pthread_mutex_unlock( &pCCCI->mHw->LadBufferMutex);
            usleep(2);
            break;

         case M2A_PCM_DataNotify:  // Get Microphone data from Modem
         {
//            ALOGD("(%d)M2A_PCM_DataNotify (Mic) \n",cnt_flag);
#if defined(MTK_DUAL_MIC_SUPPORT)||defined(MTK_AUDIO_HD_REC_SUPPORT)
            if(pCCCI->mHw->mDualMic_Test) {
#ifdef USE_DUALMIC_SDBUF
                pCCCI->DualMic_WriteDataToRecBuf(msg);
#else
                pCCCI->DualMic_WriteDataToFile(msg);
#endif
                break;
            }
#ifdef MTK_DUAL_MIC_SUPPORT
            if(pCCCI->mHw->mAudParamTuning->isPlaying()) {
                pCCCI->mHw->mAudParamTuning->readDataFromModem(CCCI_MESSAGE_OFFSET_PARSER(msg),CCCI_MESSAGE_LENGTH_PARSER(msg));
                break;
            }
#endif
#endif
            pCCCI->Record2WayLock();

            pRecord2Way = pCCCI->mHw->pRecord2Way;
            if(pRecord2Way == NULL){
               ALOGD("M2A_PCM_DataNotify no pRecord2Way path [MediaServer Restart] \n");
               pLad = pCCCI->mHw->pLad;
               pLad->LAD_PCM2WayDataSendBack();
               break;
            }
            //PCM2WAY "GetFromMicrophone"
            bufLen = pCCCI->GetM2AShareBufLen();            // Ex: 2048 bytes
            pBase = (char*)pCCCI->GetM2AShareBufAddress();  // Ex: M2A data pointer
            pCCCI->pShareR = pBase + CCCI_MESSAGE_OFFSET_PARSER(msg);
            ptrW = pCCCI->pShareR + CCCI_MESSAGE_LENGTH_PARSER(msg); // len: 320 bytes
            pCCCI->CheckOffsetandLength(msg);
            pEnd = pBase + bufLen;
            if ( ptrW > pEnd ) {
   	         ptrW -= bufLen;
            }
            pCCCI->pShareW = ptrW;

            short *ptr_16;
            ptr_16 = (short *)pCCCI->pShareR;
            ALOGV("M2A_PCM_DataNotify, ptr_16:0x%x, (%x) \n",ptr_16,*ptr_16);
            sync_word = *ptr_16;
            data_type = *(ptr_16+1);
            data_len  = *(ptr_16+2);

            if(sync_word != 0x2A2A){
               ALOGD("sync_word error: 0x%x \n",sync_word);
            }
            ALOGV("sync_word:0x%x, data_type:%d, data_len:%d \n",sync_word,data_type,data_len);

            // consume header length (Sync work:2 byte, Data type:2 byte, Data length:2 byte)
            pCCCI->pShareR += LAD_SHARE_HEADER_LEN;
            if ( pCCCI->pShareR > pEnd ){
   	         pCCCI->pShareR -= bufLen;
            }
            ALOGV("M2A_PCM_DataNotify, pShareR:0x%x, len:%d \n",pCCCI->pShareR,data_len);

            if(pRecord2Way != NULL){
               pRecord2Way->Record2Way_GetDataFromMicrophone();
            }

            pLad = pCCCI->mHw->pLad;
            pLad->LAD_PCM2WayDataSendBack();

            pCCCI->Record2WayUnLock();
            break;
         }
         case M2A_PCM_Request: // Put Data to modem and play via Receive(or Speaker)
         {
//            ALOGD("(%d) M2A_PCM_Request (Spk) msg:0x%x ",cnt_flag,msg);
#if defined(MTK_DUAL_MIC_SUPPORT)||defined(MTK_AUDIO_HD_REC_SUPPORT)
            if(pCCCI->mHw->mDualMic_Test) {
#ifdef USE_DUALMIC_SDBUF
                pCCCI->DualMic_ReadDataFromPlayBuf();
#else
                pCCCI->DualMic_ReadDataFromFile();
#endif
                break;
            }
#endif

            if(pCCCI->mHw->mAudParamTuning->isPlaying()) {
                pCCCI->mHw->mAudParamTuning->putDataToModem();
                break;
            }
			
            pCCCI->Play2WayLock();

            pPlay2Way = pCCCI->mHw->pPlay2Way;
            if(pPlay2Way == NULL){
               ALOGD("M2A_PCM_DataNotify no pPlay2Way path [MediaServer Restart] \n");
               offset = A2M_SHARED_BUFFER_OFFSET;
               len = 320 + 6;  // PCMdataToModemSize=320. header_size=LAD_SHARE_HEADER_LEN(6)
               pLad = pCCCI->mHw->pLad;
               pLad->LAD_PCM2WayDataNotify(offset, len);
               break;
            }

            // parse size of pcm_dataRequest required
            pcm_dataRequest = CCCI_MESSAGE_PARAM1_PARSER(msg);
            pcm_dataRequest = 320;

            // Put Data to modem
            if(pPlay2Way != NULL){
               pPlay2Way->Play2Way_PutDataToSpeaker(pcm_dataRequest);
            }

            pCCCI->Play2WayUnLock();
            break;
         }
         // Modem side power or reset.
         // Modem would send this message to notify AP side. Need to send speech parameter to modem side again.
         case M2A_MD_Reset_Notify:
         {
            ALOGD("..[M2A_MD_Reset_Notify]..");
            pCCCI->ResetSphCoefFlag();
            if(pCCCI->mHw != NULL){
                // Recover Analog Register.
                pCCCI->mHw->mAnaReg->AnalogAFE_Recover();

                if(pCCCI->mHw->mStreamHandler->GetSrcBlockRunning() == false){
                    ALOGD(AUDMSG"Disable DL1 SRC\n");
                    pCCCI->mHw->mAfe_handle->SetAfeReg(AFE_DAC_CON0, 0x0, 0x2);    // Disable DL1
                    pCCCI->mHw->mAfe_handle->SetAfeReg(AFE_DL_SRC2_1, 0x0, 0x21);    // set audio mode, Disable DL1 SRC
                }

                // Create Send Sph Para Thread
                bool ret = pCCCI->CreateSendSphParaThread();
                if(ret == false){
                    ALOGD("M2A_MD_Reset_Notify fail");
                }
            }
            // update the md reset status to done (1)
            property_set("af.mdrst","1");
            break;
         }
         default:{
            ALOGD("No this CCCI msg(0x%x),tid(%d),pid(%d)",msg,gettid(),getpid());
            usleep(10*1000);
            break;
         }
      }
   }
   pthread_exit(NULL);
   return 0;
}


void *SendSphParaThread(void *arg)
{
    ALOGD("Send Sph Para thread tid=%d", gettid());

    AudioCCCI *pCCCI = (AudioCCCI*)arg;
    LAD *pLad = pCCCI->mHw->pLad;


    // Get SphCoeffLock for prevent been interruptted by md reset
    bool NeedUnlockSpeechCoeff = false;
    if (pCCCI->SpeechCoeffLock() == false) {
        ALOGE(AUDMSG"SendSphParaThread fail to get SpeechCoeffLock for ensuring share buffer control is completed");
    }
    else {
        NeedUnlockSpeechCoeff = true;
    }

    // Check the first message in queue. If need ack, take action.
    pCCCI->MDReset_CheckMessageInQueue();

    // Only set modem reset flag during phone call
    if (pCCCI->mHw != NULL) {
        int b_sph_on = pCCCI->mHw->Get_Recovery_Speech();
        int b_rec_on = pCCCI->mHw->Get_Recovery_Record();
        int b_bgs_on = pCCCI->mHw->Get_Recovery_Bgs();
        if (b_sph_on == true || b_rec_on == true || b_bgs_on == true) {
            pCCCI->b_modem_reset = true;
        }
        if (b_rec_on == true) {
            pCCCI->m_MDReset_recover_record = 1;
            pCCCI->mHw->mStreamHandler->RecordClose();
        }
    }

    // Modem reset, flush all CCCI queue first. Don't care for the CCCI queue.
    pCCCI->MDReset_FlushMessageInQueue();

    // Send speech parameters to modem side
    pLad->LAD_SendSphParaProcedure();


    // Release lock
    if (NeedUnlockSpeechCoeff == true) {
        pCCCI->SpeechCoeffUnLock();
    }

    pthread_exit(NULL);
    return 0;
}

bool AudioCCCI::CreateSendSphParaThread()
{
   int ret;
   ret = pthread_create( &hSendSphThread, NULL, SendSphParaThread, (void*)this);
   if ( ret != 0 )
      return false;

   return true;
}


bool AudioCCCI::CreateReadingThread()
{
#if defined(PC_EMULATION)
   hReadThread = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)CCCIReadThread,this,0,0);
   if(hReadThread == 0) return false;
   return true;
#else
   //create CCCI read thread here
   int ret;
   ret = pthread_create( &hReadThread, NULL, CCCIReadThread, (void*)this);
   if ( ret != 0 ) return false;

   ALOGD("AudioCCCI -CreateReadingThread \n");
   return true;
#endif

}


AudioCCCI::AudioCCCI(AudioYusuHardware *hw)
{
   /* set hw pointer */
   mHw = hw;

   /* file handle is created to be 0 in constructor */
   /* it will become non-zero when initialized */
   fHdl = 0;
   fHdlAck = 0;
   hPcmRx = 0;
   mFd = -1;

   mShareBufAdd = 0;
   mShareBufLen = 0;
   m_last_write_ccci_msg = 0;
   m_last_read_ccci_msg = 0;

   ResetSphCoefFlag();

   /* create the fake CCCI */
#if defined(FAKE_CCCI)
   pFakeCCCI = new FakeCCCI();
#endif

}

AudioCCCI::~AudioCCCI()
{
   CCCIEnable = false;
   ALOGD("AudioCCCI De-Constructor CCCIEnable(%d) \n",CCCIEnable);

#if defined(FAKE_CCCI)
   delete(pFakeCCCI);
#else
   close(fHdl);
   close(fHdlAck);
   //close(hPcmRx);
#endif

}


// one message at a time, used to make sure all messages are sent
void AudioCCCI::MsgQueueLock()
{
#if defined(PC_EMULATION)
   EnterCriticalSection(&fQMutex);
#else
   pthread_mutex_lock( &fQMutex );
#endif
}

void AudioCCCI::MsgQueueUnLock()
{
#if defined(PC_EMULATION)
   LeaveCriticalSection(&fQMutex);
#else
   pthread_mutex_unlock( &fQMutex );
#endif
}

void AudioCCCI::RecordLock()
{
#if defined(PC_EMULATION)
   EnterCriticalSection(&mStreaminMutex);
#else
   pthread_mutex_lock( &mStreaminMutex );
#endif
}

void AudioCCCI::RecordUnLock()
{
#if defined(PC_EMULATION)
   LeaveCriticalSection(&mStreaminMutex);
#else
   pthread_mutex_unlock(&mStreaminMutex );
#endif
}

void AudioCCCI::Record2WayLock()
{
   pthread_mutex_lock( &m2WayRecMutex );
}

void AudioCCCI::Record2WayUnLock()
{
   pthread_mutex_unlock(&m2WayRecMutex );
}

void AudioCCCI::Play2WayLock()
{
   pthread_mutex_lock( &m2WayPlayMutex );
}

void AudioCCCI::Play2WayUnLock()
{
   pthread_mutex_unlock(&m2WayPlayMutex );
}

bool AudioCCCI::A2MBufLock()
{
   int rc = 0;
   rc = pthread_mutex_lock_timeout_np(&mA2MShareBufMutex, A2MBUFLOCK_TIMEOUT);
   if (rc != 0) {
      YAD_LOGE("Can not get A2MBufLock!\n");
      return false;
   }
   return true;
}

void AudioCCCI::A2MBufUnLock()
{
   pthread_mutex_unlock(&mA2MShareBufMutex );
}

bool AudioCCCI::SpeechCoeffLock()
{
   int rc = 0;
   rc = pthread_mutex_lock_timeout_np(&mSetSpeechCoeffMutex, SPHCOEFFLOCK_TIMEOUT);
   if (rc != 0) {
      YAD_LOGE("Can not get SpeechCoeffLock!\n");
      return false;
   }
   return true;
}
void AudioCCCI::SpeechCoeffUnLock()
{
   pthread_mutex_unlock(&mSetSpeechCoeffMutex );
}

void AudioCCCI::ResetSphCoefFlag()
{
    memset(&iSPHFlag,0,sizeof(iSPHFlag));
    ALOGD(AUDMSG"ResetSphCoefFlag, NB(%d)/WB(%d)/DMNR(%d)/HDREC(%d)\n", iSPHFlag[NB_SPHCOEF],iSPHFlag[WB_SPHCOEF], iSPHFlag[DMNR_SPHCOEF], iSPHFlag[HDREC_SPHCOEF]);
}

void AudioCCCI::GetMDSideSphCoefArrival(int type)
{
    if(type >= MAX_SPHCOEF){
        ALOGE(AUDMSG"GetMDSideSphCoefArrival(%d) Error!",type);
    }else{
        if (iSPHFlag[type] < 127){//prevent overflow
            iSPHFlag[type]++;
        }
        ALOGD(AUDMSG"GetMDSideSphCoefArrival(%d), NB(%d)/WB(%d)/DMNR(%d)/HDREC(%d)\n",type, iSPHFlag[NB_SPHCOEF],iSPHFlag[WB_SPHCOEF], iSPHFlag[DMNR_SPHCOEF], iSPHFlag[HDREC_SPHCOEF]);
    }
}

bool AudioCCCI::CheckMDSideSphCoefArrival()
{
   bool ret = false;

   bool NeedUnlockSpeechCoeff = false;
   if (SpeechCoeffLock() == false) {
       ALOGE(AUDMSG"CheckMDSideSphCoefArrival fail to get SpeechCoeffLock for ensuring share buffer control is completed");
   }
   else {
       NeedUnlockSpeechCoeff = true;
   }

   if((iSPHFlag[NB_SPHCOEF] == 0)
#if defined(MTK_WB_SPEECH_SUPPORT)
     ||(iSPHFlag[WB_SPHCOEF] < 2)
#endif
#if defined(MTK_DUAL_MIC_SUPPORT)
     ||(iSPHFlag[DMNR_SPHCOEF] == 0)
#endif
#if defined(MTK_AUDIO_HD_REC_SUPPORT)
     ||(iSPHFlag[HDREC_SPHCOEF] == 0)
#endif
     ) {
        ALOGE(AUDMSG"CheckSphCoefArrivalInMDSide Fail, NB(%d)/WB(%d)/DMNR(%d)/HDREC(%d)", iSPHFlag[NB_SPHCOEF],iSPHFlag[WB_SPHCOEF], iSPHFlag[DMNR_SPHCOEF], iSPHFlag[HDREC_SPHCOEF]);
        // Send speech parameters to modem side
        if (mHw != NULL) ret = mHw->pLad->LAD_SendSphParaProcedure();
   } else {
        ALOGD(AUDMSG"CheckSphCoefArrivalInMDSide Pass, NB(%d)/WB(%d)/DMNR(%d)/HDREC(%d)", iSPHFlag[NB_SPHCOEF],iSPHFlag[WB_SPHCOEF], iSPHFlag[DMNR_SPHCOEF], iSPHFlag[HDREC_SPHCOEF]);
        ret = true;
   }

   if (NeedUnlockSpeechCoeff == true) {
       SpeechCoeffUnLock();
   }

   return ret;
}

void AudioCCCI::SendMsgFailErrorHandling(uint32 MSG)
{
    //Do error handling here.
    uint32 func = CCCI_MESSAGE_FUNCTION_PARSER(MSG);
    ALOGD(AUDMSG"SendMsgFailErrorHandling(0x%x)\n", func);
    if(A2M_Speech_On == func){
        if(mHw->mVT_Speech == true)
        {
            mHw->Set_Recovery_VT(true);
        }
        else
        {
            mHw->Set_Recovery_Speech(true);
        }
    }
    else if(A2M_Speech_Off == func){
        if(mHw->Get_Recovery_VT()==true){
            mHw->Set_Recovery_VT(false);
            mHw->SwitchAudioClock(false);
        }
        else if(mHw->Get_Recovery_Speech()==true){
            mHw->Set_Recovery_Speech(false);
            mHw->SwitchAudioClock(false);
        }else{
            ALOGE(AUDMSG"CheckMessageInQueue, Speech_Off Ack is not paired\n");
        }
    }
    else if(A2M_Record_turn_on == func){
        mHw->Set_Recovery_Record(true);
    }
    else if(A2M_Record_turn_off == func){
        if(mHw->Get_Recovery_Record()==true){
            mHw->Set_Recovery_Record(false);
            mHw->SwitchAudioClock(false);

        }else{
            ALOGE(AUDMSG"CheckMessageInQueue, Record_turn_off Ack is not paired\n");
        }
    }
    else if(A2M_BGSND_Open == func){
        mHw->Set_Recovery_Bgs(true);
    }
    else if(A2M_BGSND_Close == func){
        if(mHw->Get_Recovery_Bgs()==true){
            mHw->Set_Recovery_Bgs(false);
            mHw->SwitchAudioClock(false);
        }else{
            ALOGE(AUDMSG"CheckMessageInQueue, BGSND_Close Ack is not paired\n");
        }
    }
    else if(AUDIO_CUSTOM_PARAMETER == func){
        A2MBufUnLock();
    }
    else if(AUDIO_WB_CUSTOM_PARAMETER == func){
        A2MBufUnLock();
    }
    //else if(AUDIO_HD_RECORD_PARAMETER == func){ // AUDIO_HD_RECORD_PARAMETER == AUDIO_WB_CUSTOM_PARAMETER ...
    //   A2MBufUnLock();
    //}
    else if(AUDIO_DUAL_MIC_CUSTOM_PARAMETER == func){
        A2MBufUnLock();
    }
    else{
        ALOGD(AUDMSG"SendMsgFailErrorHandling(0x%x). ack Don't Care \n", func);
    }
}
/*------------------------------------------------------------------------*/

//Sending CCCI message with 1 parameter
uint32 CCCI_MSG1(uint32 Function,uint32 Param1)
{
   return ((uint32)Function << 24) | ((uint32)Param1 & 0xffffff);
}


//Sending CCCI message with 2 parameters
//for CCCI message with 2 parameters, Function actually has 6 bits but is extend to 8 bits by padding '0'
uint32 CCCI_MSG2(uint32 Function, uint32 Param1, uint32 Param2)
{
   return ((uint32)Function << 24) | (((uint32)Param1 & 0x1fff) << 13) | ((uint32)Param2 & 0x1fff);
}

//Sending CCCI message with 2 parameters, another edition
//for CCCI message with 2 parameters, Function actually has 8 bits, parameters has 12 bits
uint32 CCCI_MSG3(uint32 Function, uint32 Param1, uint32 Param2)
{
   return ((uint32)Function << 24) | (((uint32)Param1 & 0x0fff) << 12) | ((uint32)Param2 & 0x0fff);
}


//Parse CCCI message type
uint32 CCCI_MESSAGE_TYPE_PARSER(uint32 CCCI_MESSAGE)
{
   return ((uint32)CCCI_MESSAGE) >> 28;
}


//Parse CCCI message function
uint32 CCCI_MESSAGE_FUNCTION_PARSER(uint32 CCCI_MESSAGE)
{
   uint32 function = ((uint32)CCCI_MESSAGE) >> 24;
   uint32 type = CCCI_MESSAGE_TYPE_PARSER(CCCI_MESSAGE);

   // for 'M2A Data Notification'and 'A2M Data Notification' types
   // function has acutally 6 bits, last 2 bits are pedded with '0'
   if ( type == 5 || type == 9 )
      function = (function >> 2) << 2;

   return function;
}


//Parse CCCI message for data offset (Only for Data Notify)
uint32 CCCI_MESSAGE_OFFSET_PARSER(uint32 CCCI_MESSAGE)
{
   return (((uint32)CCCI_MESSAGE) << 6) >> 19;
}


//Parse CCCI message for data length (Only for Data Notify)
uint32 CCCI_MESSAGE_LENGTH_PARSER(uint32 CCCI_MESSAGE)
{
   return (((uint32)CCCI_MESSAGE) << 19) >> 19;
}

//Parse CCCI message for parameter 1
uint32 CCCI_MESSAGE_PARAM1_PARSER(uint32 CCCI_MESSAGE)
{
   return (((uint32)CCCI_MESSAGE) << 8) >> 8;
}

}; // namespace android
