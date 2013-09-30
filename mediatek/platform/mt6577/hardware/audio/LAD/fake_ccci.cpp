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
 * fake_ccci.c
 *
 * Project:
 * --------
 *   Android + MT6516
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
 * $Revision: #8 $
 * $Modtime:$
 * $Log:$
 *
 * 08 12 2010 chipeng.chang
 * [ALPS00003297] [Need Patch] [Volunteer Patch] android 2.2 migration
 * update for audio driver for playback.
 *
 *******************************************************************************/


#include "AudioYusuCcci.h"
#include "fake_ccci.h"
#include "AudioYusuHardware.h"
#include "AudioYusuLad.h"


#if defined(PC_EMULATION)
#include "stdio.h"
#include "stdlib.h"
#include "windows.h"
#endif

namespace android {

FakeCCCI::FakeCCCI()
{
   /* allocate fake share memory */
   pBufBase = new char[CCCI_PCM_SHARE_BUF_LEN];
   pM2ABufBase = pBufBase + M2A_SHARE_BUF_LEN;
   pA2MBufBase = pBufBase;

   /* running flag should be set to '0' when initial */
   mRunFlag = 0;

   /* setting fake values */
   memset(pBufBase, 0x77, CCCI_PCM_SHARE_BUF_LEN);

   /* initial events */
#if defined(PC_EMULATION)
   InitializeCriticalSection(&fMsgMutex);
   hEventDataOK = CreateEvent(NULL,TRUE,FALSE,"EventDataOK");
   hEventSendBack = CreateEvent(NULL,TRUE,FALSE,"EventSendBack");
   hEventPlayDataNotify = CreateEvent(NULL,TRUE,FALSE,"EventPlayDataNotify");
#else
   // event initialize
    int ret;

    ret = pthread_mutex_init(&fMsgMutex, NULL);

    ret = pthread_mutex_init(&fdataMutex, NULL);
    ret = pthread_mutex_init(&fbkMutex, NULL);
    ret = pthread_mutex_init(&fpdnMutex, NULL);

    ret = pthread_cond_init(&hEventDataOK, NULL);
    ret = pthread_cond_init(&hEventSendBack, NULL);
    ret = pthread_cond_init(&hEventPlayDataNotify, NULL);
    //if ( res != 0 ) { return -1 }

#endif

}

FakeCCCI::~FakeCCCI()
{

   /* free allocated memory for pc simulation */
    delete [] pBufBase;

}


uint32 FakeCCCI::GetBaseAddress()
{
   return (uint32)pBufBase;   //simply return the buf ptr allocated in constructor
}


int32 FakeCCCI::FakeCCCI_RecSendbackEvent_Set()
{
   int32 ret;
#if defined(PC_EMULATION)
   ret = SetEvent(hEventSendBack);
#else
   pthread_mutex_lock( &fbkMutex );
   fbkMsgFlag++;
   ret = pthread_cond_signal( &hEventSendBack );
   pthread_mutex_unlock( &fbkMutex );
#endif
   return ret;
}

int32 FakeCCCI::FakeCCCI_RecSendbackEvent_Wait()
{
   int32 ret = -1;
#if defined(PC_EMULATION)
   ret = WaitForSingleObject(hEventSendBack,INFINITE);
   ResetEvent(hEventSendBack);
   //printf("Modem got send back message\n");
#else
   pthread_mutex_lock( &fbkMutex );
   if ( fbkMsgFlag == 0 )
      ret = pthread_cond_wait( &hEventSendBack, &fbkMutex );
   fbkMsgFlag --;
   pthread_mutex_unlock( &fbkMutex );
   ALOGV("Modem: got send back message\n");
#endif
   return ret;
}

int32 FakeCCCI::FakeCCCI_PlayDataNotifyEvent_Set()
{
   int32 ret;
#if defined(PC_EMULATION)
   ret = SetEvent(hEventPlayDataNotify);
#else
   pthread_mutex_lock( &fpdnMutex );
   fpdnMsgFlag++;
   ret = pthread_cond_signal( &hEventPlayDataNotify );
   pthread_mutex_unlock( &fpdnMutex );
#endif
   return ret;
}

int32 FakeCCCI::FakeCCCI_PlayDataNotifyEvent_Wait()
{
   int32 ret = -1;
#if defined(PC_EMULATION)
   ret = WaitForSingleObject(hEventPlayDataNotify,INFINITE);
   ResetEvent(hEventPlayDataNotify);
   //printf("Modem got send back message\n");
#else
   pthread_mutex_lock( &fpdnMutex );
   if ( fbkMsgFlag == 0 )
      ret = pthread_cond_wait( &hEventPlayDataNotify, &fpdnMutex );
   fbkMsgFlag --;
   pthread_mutex_unlock( &fpdnMutex );
   ALOGV("Modem: got send back message\n");
#endif
   return ret;
}

int32 FakeCCCI::FakeCCCI_ModemSideEvent_Set()
{
   int32 ret;
#if defined(PC_EMULATION)
   ret = SetEvent(hEventDataOK);
#else
   pthread_mutex_lock( &fdataMutex );
   ret = pthread_cond_signal( &hEventDataOK );
   fdataMsgFlag++;
   pthread_mutex_unlock( &fdataMutex );
#endif
   return ret;
}

int32 FakeCCCI::FakeCCCI_ModemSideEvent_Wait()
{
   int32 ret = -1;
#if defined(PC_EMULATION)
   ret = WaitForSingleObject(hEventDataOK,INFINITE);
   ResetEvent(hEventDataOK);
   //printf("Read Message Rrturn : 0x%x \n", mSendMsg);

#else

   pthread_mutex_lock( &fdataMutex );
   if( fdataMsgFlag == 0 )
      ret = pthread_cond_wait( &hEventDataOK, &fdataMutex );
   fdataMsgFlag--;
   pthread_mutex_unlock( &fdataMutex );
   ALOGV("modem: Read Message Return \n");

#endif
   return ret;
}


bool FakeCCCI::SendMessage(uint32 *pMsg)
{
    uint32 msg = pMsg[1];
    uint32 param;
    uint32 func = CCCI_MESSAGE_FUNCTION_PARSER(msg);

    switch(func) {

    //-------------------PCM Group----------------------------

    case A2M_PCM_OPEN:

       // 2 way pcm open for normal mode recording and playback
       param = CCCI_MESSAGE_PARAM1_PARSER(msg);
       if (param == 2) {          // playback
          /* open the data writing thread */
          mpRunFlag = 1;
          CreateCCCIFakePlayThread();
       }
       else if (param == 4) {     // recording
          /* open the data generation thread */
          mRunFlag = 1;
          CreateCCCIFakeRecThread();
       }
       else if (param == 6) {          // playback + recording

          /* open the data writing thread */
          mpRunFlag = 1;
          CreateCCCIFakePlayThread();

          /* open the data generation thread */
          mRunFlag = 1;
          CreateCCCIFakeRecThread();
       }
       else {
          ASSERT(0);
       }

       break;

    case A2M_PCM_Close:

       if(mRunFlag) {      // if record running
          /* set to zero to make the while loop of FakeCCCIThread out */
          mRunFlag = 0;
          /* generate sendback message to make FakeCCCIThread go to end */
          /* or it will wait forever without end */
          FakeCCCI_RecSendbackEvent_Set();

       }

       if(mpRunFlag) {     // if playback running
          /* set to zero to make the while loop of FakeCCCIThread out */
          mpRunFlag = 0;
          /* generate data notify message to make FakeCCCIThread go to end */
          /* or it will wait forever without end */
          playDataOffset = CCCI_MESSAGE_OFFSET_PARSER(0);   // give 0 values, bcs this is for ending the thread
          playDataLen = CCCI_MESSAGE_LENGTH_PARSER(0);
          FakeCCCI_PlayDataNotifyEvent_Set();
       }

       break;

    case A2M_PCM_DataNotify:
       /* extract offset and length info */
       playDataOffset = CCCI_MESSAGE_OFFSET_PARSER(msg);
       playDataLen = CCCI_MESSAGE_LENGTH_PARSER(msg);

       /* send message to notify play thread */
       FakeCCCI_PlayDataNotifyEvent_Set();
       break;

    case A2M_PCM_DataSendBack :
       FakeCCCI_RecSendbackEvent_Set();
       break;

    //-------------------BGSound Group----------------------------
    case A2M_BGSND_Open :

       /* open the data writing thread */
       mpRunFlag = 1;
       CreateCCCIFakePlayThread();
       break;

    case A2M_BGSND_Close :

       /* set to zero to make the while loop of FakeCCCIThread out */
       mpRunFlag = 0;
       /* generate data notify message to make FakeCCCIThread go to end */
       /* or it will wait forever without end */
       playDataOffset = CCCI_MESSAGE_OFFSET_PARSER(0);   // give 0 values, bcs this is for ending the thread
       playDataLen = CCCI_MESSAGE_LENGTH_PARSER(0);
       FakeCCCI_PlayDataNotifyEvent_Set();
       break;

    case A2M_BGSND_DataNotify :
       /* extract offset and length info */
       playDataOffset = CCCI_MESSAGE_OFFSET_PARSER(msg);
       playDataLen = CCCI_MESSAGE_LENGTH_PARSER(msg);

       /* send message to notify play thread */
       FakeCCCI_PlayDataNotifyEvent_Set();
       break;


    //-------------------Record Group----------------------------
    case A2M_Record_turn_on :

       /* open the data generatio thread */
       mRunFlag = 1;
       CreateCCCIFakeRecThread();
       break;

    case A2M_Record_turn_off :

       /* set to zero to make the while loop of FakeCCCIThread out */
       mRunFlag = 0;
       /* generate sendback message to make FakeCCCIThread go to end */
       /* or it will wait forever without end */
       FakeCCCI_RecSendbackEvent_Set();
       break;


    case A2M_Record_DataSendBack :
       FakeCCCI_RecSendbackEvent_Set();
       break;

    //-------------------Playback Group(not used)----------------------------
    case A2M_playback_turn_on :

       /* open the data writing thread */
       mpRunFlag = 1;
       CreateCCCIFakePlayThread();
       break;

    case A2M_playback_turn_off :

       /* set to zero to make the while loop of FakeCCCIThread out */
       mpRunFlag = 0;
       /* generate data notify message to make FakeCCCIThread go to end */
       /* or it will wait forever without end */
       playDataOffset = 0;   // give 0 values, bcs this is for ending the thread
       playDataLen = 0;
       FakeCCCI_PlayDataNotifyEvent_Set();
       break;

    case A2M_Playback_DataNotify :
       /* extract offset and length info */
       playDataOffset = CCCI_MESSAGE_OFFSET_PARSER(msg);
       playDataLen = CCCI_MESSAGE_LENGTH_PARSER(msg);

       /* send message to notify play thread */
       FakeCCCI_PlayDataNotifyEvent_Set();
       break;

    default:
       /* sleep 5ms for simulation the time processs of other commands */
       AAD_Sleep_1ms(5);

       break;
    }
    return true;
}





bool FakeCCCI::ReadMessage(uint32 *pMsg)
{
   /* wait for a data message */
   FakeCCCI_ModemSideEvent_Wait();

   /* generate a simulated message */
   uint32 msg = mSendMsg;
   pMsg[1] = mSendMsg;

   MessageUnLock();

   //pMsg[1] = (type << 28) | (offset << 13) | validDataLen;


   return true;
}




void *FakeCCCIRecThread(void *arg)
{
   FakeCCCI *pCCCI = (FakeCCCI*)arg;

   /* sleep for 50 ms for simulation of turn on time */
   AAD_Sleep_1ms(50);

   YAD_LOGW("--Fake CCCI Rec thread start---\n");

   // prepare Record on Ack message
   pCCCI->MessageLock();
   pCCCI->mSendMsg = CCCI_MSG1(M2A_RecordOnCompleted_Notification, 0);

   // send the Record On Ack message by set event, this makes ReadMessage to return
   pCCCI->FakeCCCI_ModemSideEvent_Set();


   while(pCCCI->mRunFlag == 1) {

      /* wait for 20 ms */
      AAD_Sleep_1ms(15);

      /* prepare some fake data */
      int32 i;
      pCCCI->offset = 2000;
      pCCCI->validDataLen = 320;
      int16 *base = (int16*)(pCCCI->pM2ABufBase);     //base is on M2A buffer
      int16 *ptr = (int16*)(pCCCI->pM2ABufBase + pCCCI->offset);
      int16 *end = (int16*)(pCCCI->pM2ABufBase + M2A_SHARE_BUF_LEN);

      // add header
      *ptr++ = (int16)M2A_BUF_HEADER;
      if ( ptr == end ) ptr = base;

      *ptr++ = (int16)LADBUFID_PCM_GetFromMic;
      if ( ptr == end ) ptr = base;

      *ptr++ = (int16)pCCCI->validDataLen;
      if ( ptr == end ) ptr = base;


      if ( ptr + pCCCI->validDataLen < end ) {
         for ( i = 0 ; i < (int32)(pCCCI->validDataLen >> 1) ; i++ )
            //*ptr++ = i*400;
            *ptr++ = sineTable[i];
      }
      else {
         int32 cnt2end = ((int32)end - (int32)ptr) >> 1 ;
         for ( i = 0 ; i < cnt2end ; i++ )
            //*ptr++ = i*400;
            *ptr++ = sineTable[i];

         ptr = base;

         for (       ; i < (int32)(pCCCI->validDataLen >> 1) ; i++ )
            //*ptr++ = i *400;
            *ptr++ = sineTable[i];
      }

      // prepare Record Data Notify message
      pCCCI->MessageLock();
      pCCCI->mSendMsg = CCCI_MSG2(M2A_Record_DataNotify, pCCCI->offset, pCCCI->validDataLen + LAD_SHARE_HEADER_LEN);

      // send the Record Data Notify message by set event, this make ReadMessage return
      pCCCI->FakeCCCI_ModemSideEvent_Set();

      /* wait for 'send back' message */
      pCCCI->FakeCCCI_RecSendbackEvent_Wait();


   }

   /* sleep for 50 ms for simulation of turn off time */
   AAD_Sleep_1ms(50);

   // prepare Record off Ack message, to tell AP that Record is off
   pCCCI->MessageLock();
   pCCCI->mSendMsg = CCCI_MSG1(M2A_RecordOffCompleted_Notification, 0);

   // send the Record On Ack message by set event, this makes ReadMessage to return
   pCCCI->FakeCCCI_ModemSideEvent_Set();

   YAD_LOGW("--Fake CCCI Rec thread end---\n");
   return 0;
}


void *FakeCCCIPlayThread(void *arg)
{
   FakeCCCI *pCCCI = (FakeCCCI*)arg;
   int writeNum = 0;

   /* sleep for 10 ms for simulation of turn on time */
   AAD_Sleep_1ms(10);

   FILE* fp;
   fp = fopen("mixout.pcm","wb");

   YAD_LOGW("--Fake CCCI Playback thread start---\n");

   while(pCCCI->mpRunFlag == 1) {

      // wait for 20 ms
      AAD_Sleep_1ms(20);

      // set data request message
      pCCCI->MessageLock();
      pCCCI->mSendMsg = CCCI_MSG1(M2A_Playback_Request, FAKECCCI_PLAY_FRAME_LEN);
      pCCCI->FakeCCCI_ModemSideEvent_Set();

      /* wait for DataNotify message from AP side */
      pCCCI->FakeCCCI_PlayDataNotifyEvent_Wait();

      /* write to file */
      int8* pData = pCCCI->pA2MBufBase + pCCCI->playDataOffset + LAD_SHARE_HEADER_LEN;
      //prevent wirte minus number of data
      writeNum = pCCCI->playDataLen - LAD_SHARE_HEADER_LEN;
      if ( writeNum < 0 ) writeNum = 0;
      fwrite(pData, writeNum, sizeof(int8), fp);


      YAD_LOGW(" play len :%d \n", pCCCI->playDataLen);


   }

   fclose(fp);

   /* sleep for 50 ms for simulation of turn off time */
   AAD_Sleep_1ms(50);

   YAD_LOGW("--Fake CCCI Playback thread end---\n");
   return 0;
}

bool FakeCCCI::CreateCCCIFakeRecThread()
{
 /* create the CCCI event reading thread */
#if defined(PC_EMULATION)
   hCCCIRecThread = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)FakeCCCIRecThread,this,0,0);
   if(hCCCIRecThread == 0) return false;
   return true;
#else
   //create CCCI read thread here
   int ret;
   ret = pthread_create( &hCCCIRecThread, NULL, FakeCCCIRecThread, (void*)this);
   if ( ret != 0 ) return false;
   return true;
#endif

}

bool FakeCCCI::CreateCCCIFakePlayThread()
{
 /* create the CCCI event reading thread */
#if defined(PC_EMULATION)
   hCCCIPlayThread = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)FakeCCCIPlayThread,this,0,0);
   if(hCCCIPlayThread == 0) return false;
   return true;
#else
   //create CCCI read thread here
   int ret;
   ret = pthread_create( &hCCCIPlayThread, NULL, FakeCCCIPlayThread, (void*)this);
   if ( ret != 0 ) return false;
   return true;
#endif

}





// one message at a time, used to make sure all messages are sent
void FakeCCCI::MessageLock()
{
#if defined(PC_EMULATION)
   EnterCriticalSection(&fMsgMutex);
#else
   pthread_mutex_lock( &fMsgMutex );
#endif
}

void FakeCCCI::MessageUnLock()
{
#if defined(PC_EMULATION)
   LeaveCriticalSection(&fMsgMutex);
#else
   pthread_mutex_unlock( &fMsgMutex );
#endif
}


//SampleRate = 8000, Frequency = 200;
const uint16 sineTable[320] = {
0x0000, 0x0A03, 0x13C7, 0x1D0E, 0x259E, 0x2D41, 0x33C7, 0x3906,
0x3CDE, 0x3F36, 0x4000, 0x3F36, 0x3CDE, 0x3906, 0x33C7, 0x2D41,
0x259E, 0x1D0E, 0x13C7, 0x0A03, 0x0000, 0xF5FD, 0xEC39, 0xE2F2,
0xDA62, 0xD2BF, 0xCC39, 0xC6FA, 0xC322, 0xC0CA, 0xC000, 0xC0CA,
0xC322, 0xC6FA, 0xCC39, 0xD2BF, 0xDA62, 0xE2F2, 0xEC39, 0xF5FD,
0xFFFF, 0x0A03, 0x13C7, 0x1D0E, 0x259E, 0x2D41, 0x33C7, 0x3906,
0x3CDE, 0x3F36, 0x4000, 0x3F36, 0x3CDE, 0x3906, 0x33C7, 0x2D41,
0x259E, 0x1D0E, 0x13C7, 0x0A03, 0x0000, 0xF5FD, 0xEC39, 0xE2F2,
0xDA62, 0xD2BF, 0xCC39, 0xC6FA, 0xC322, 0xC0CA, 0xC000, 0xC0CA,
0xC322, 0xC6FA, 0xCC39, 0xD2BF, 0xDA62, 0xE2F2, 0xEC39, 0xF5FD,
0xFFFF, 0x0A03, 0x13C7, 0x1D0E, 0x259E, 0x2D41, 0x33C7, 0x3906,
0x3CDE, 0x3F36, 0x4000, 0x3F36, 0x3CDE, 0x3906, 0x33C7, 0x2D41,
0x259E, 0x1D0E, 0x13C7, 0x0A03, 0x0000, 0xF5FD, 0xEC39, 0xE2F2,
0xDA62, 0xD2BF, 0xCC39, 0xC6FA, 0xC322, 0xC0CA, 0xC000, 0xC0CA,
0xC322, 0xC6FA, 0xCC39, 0xD2BF, 0xDA62, 0xE2F2, 0xEC39, 0xF5FD,
0xFFFF, 0x0A03, 0x13C7, 0x1D0E, 0x259E, 0x2D41, 0x33C7, 0x3906,
0x3CDE, 0x3F36, 0x4000, 0x3F36, 0x3CDE, 0x3906, 0x33C7, 0x2D41,
0x259E, 0x1D0E, 0x13C7, 0x0A03, 0x0000, 0xF5FD, 0xEC39, 0xE2F2,
0xDA62, 0xD2BF, 0xCC39, 0xC6FA, 0xC322, 0xC0CA, 0xC000, 0xC0CA,
0xC322, 0xC6FA, 0xCC39, 0xD2BF, 0xDA62, 0xE2F2, 0xEC39, 0xF5FD,
0xFFFF, 0x0A03, 0x13C7, 0x1D0E, 0x259E, 0x2D41, 0x33C7, 0x3906,
0x3CDE, 0x3F36, 0x4000, 0x3F36, 0x3CDE, 0x3906, 0x33C7, 0x2D41,
0x259E, 0x1D0E, 0x13C7, 0x0A03, 0x0000, 0xF5FD, 0xEC39, 0xE2F2,
0xDA62, 0xD2BF, 0xCC39, 0xC6FA, 0xC322, 0xC0CA, 0xC000, 0xC0CA,
0xC322, 0xC6FA, 0xCC39, 0xD2BF, 0xDA62, 0xE2F2, 0xEC39, 0xF5FD,
0xFFFF, 0x0A03, 0x13C7, 0x1D0E, 0x259E, 0x2D41, 0x33C7, 0x3906,
0x3CDE, 0x3F36, 0x4000, 0x3F36, 0x3CDE, 0x3906, 0x33C7, 0x2D41,
0x259E, 0x1D0E, 0x13C7, 0x0A03, 0x0000, 0xF5FD, 0xEC39, 0xE2F2,
0xDA62, 0xD2BF, 0xCC39, 0xC6FA, 0xC322, 0xC0CA, 0xC000, 0xC0CA,
0xC322, 0xC6FA, 0xCC39, 0xD2BF, 0xDA62, 0xE2F2, 0xEC39, 0xF5FD,
0xFFFF, 0x0A03, 0x13C7, 0x1D0E, 0x259E, 0x2D41, 0x33C7, 0x3906,
0x3CDE, 0x3F36, 0x4000, 0x3F36, 0x3CDE, 0x3906, 0x33C7, 0x2D41,
0x259E, 0x1D0E, 0x13C7, 0x0A03, 0xFFFF, 0xF5FD, 0xEC39, 0xE2F2,
0xDA62, 0xD2BF, 0xCC39, 0xC6FA, 0xC322, 0xC0CA, 0xC000, 0xC0CA,
0xC322, 0xC6FA, 0xCC39, 0xD2BF, 0xDA62, 0xE2F2, 0xEC39, 0xF5FD,
0xFFFF, 0x0A03, 0x13C7, 0x1D0E, 0x259E, 0x2D41, 0x33C7, 0x3906,
0x3CDE, 0x3F36, 0x4000, 0x3F36, 0x3CDE, 0x3906, 0x33C7, 0x2D41,
0x259E, 0x1D0E, 0x13C7, 0x0A03, 0x0000, 0xF5FD, 0xEC39, 0xE2F2,
0xDA62, 0xD2BF, 0xCC39, 0xC6FA, 0xC322, 0xC0CA, 0xC000, 0xC0CA,
0xC322, 0xC6FA, 0xCC39, 0xD2BF, 0xDA62, 0xE2F2, 0xEC39, 0xF5FD,
};

}; // namespace android

