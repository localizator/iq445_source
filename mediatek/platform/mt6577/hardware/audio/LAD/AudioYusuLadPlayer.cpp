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

#include <sys/time.h>
#include "AudioYusuDef.h"
#include "AudioYusuLadPlayer.h"
#include "AudioYusuLad.h"
#include "AudioYusuCcci.h"
#include <utils/Log.h>
#include <hardware_legacy/AudioSystemLegacy.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "LADPlayBuffer"

#ifdef ENABLE_LOG_LADPlay
    #define LOG_LADPlay ALOGD
#else
    #define LOG_LADPlay ALOGV
#endif

namespace android {

#ifdef BGS_USE_SINE_WAVE
static short LAD_Tone[] = {
    0x0   ,     0x0,          0xa02     ,0xa02,
    0x13a4   ,0x13a3,    0x1c83   ,0x1c83,
    0x2449   ,0x2449,    0x2aac   ,0x2aac,
    0x2f6a   ,0x2f69,    0x3254   ,0x3255,
    0x3352   ,0x3351,    0x3255   ,0x3255,
    0x2f6a   ,0x2f69,    0x2aac   ,0x2aac,
    0x2449   ,0x244a,    0x1c83   ,0x1c83,
    0x13a4   ,0x13a3,    0xa03     ,0xa02,
    0x0         ,0x0     ,    0xf5fd   ,0xf5fd,
    0xec5d   ,0xec5c,    0xe37e   ,0xe37e,
    0xdbb7   ,0xdbb7,    0xd555   ,0xd555,
    0xd097   ,0xd096,    0xcdab   ,0xcdac,
    0xccaf   ,0xccae,    0xcdab   ,0xcdab,
    0xd096   ,0xd097,    0xd554   ,0xd554,
    0xdbb7   ,0xdbb7,    0xe37e   ,0xe37d,
    0xec5d   ,0xec5d,    0xf5fd   ,0xf5fe
};
#define SIZE_LAD_Tone 128
#endif

void BLI_ASSERT( int expression )
{
}

LADPlayBuffer::LADPlayBuffer()
{
#ifdef DUMP_BGS_BLI_BUF
   p_OutFile1 = NULL;

   struct tm *timeinfo;
   time_t rawtime;
   time(&rawtime);
   timeinfo=localtime(&rawtime);
   char path[80];
   memset((void*)path,0,80);
   strftime (path,80,"/sdcard/%a_%b_%Y_%H_%M_%S_BGSBefore_BLI.pcm",timeinfo);
   ALOGD( "fopen path is : %s", path);
   p_OutFile1 = fopen(path,"w");
//   p_OutFile1 = fopen("/data/Record_before_BLI.pcm","wb");

   if( p_OutFile1 == NULL)
   {
      ALOGD("Fail to Open Record_before_BLI File %s ",p_OutFile1);
   }


/*
   p_OutFile2 = NULL;
   p_OutFile2 = fopen("/data/Record_after_BLI.pcm","wb");
   if( p_OutFile2 == NULL)
   {
      ALOGD("Fail to Open Record_after_BLI File %s ",p_OutFile2);
   }
*/
#endif
}

LADPlayBuffer::~LADPlayBuffer()
{
   pthread_mutex_lock( &(pLadPlayer->mLadPlayerWriteMutex) );

   // clean the buffer i use
   delete [] mBuf.pBufBase;
   delete [] pSrcBuf;

   // clean the idx of buffer table in LADPlayer
   pLadPlayer->mBufTable[mIdx] = NULL;
   pthread_mutex_unlock( &(pLadPlayer->mLadPlayerWriteMutex) );

#ifdef DUMP_BGS_BLI_BUF
   if(p_OutFile1!=NULL)
       fclose(p_OutFile1);
   //if(p_OutFile2!=NULL)
   //    fclose(p_OutFile2);
   p_OutFile1 = NULL;
   p_OutFile2 = NULL;
#endif

}

static void ConvertPcm8ToPcm16(int8* inBuf, int8* outBuf, int32 num)
{
   int16 *pBuf16 = (int16*)outBuf;
   int8 *pBuf8 = inBuf;
   do {
      *pBuf16++ = *pBuf8++;
   }while( --num > 0);
}

//*****************************************************************************************
//--------------------------for normal playbuffer------------------------------------------
//*****************************************************************************************

bool LADPlayBuffer::LADPlayBufferInit(LADPlayer *playPointer,
                                      uint32 idx,
                                      uint32 sampleRate,
                                      uint32 chNum,
                                      int32 format,
                                      uint32 targetSR)
{
   //keep the format
   mFormat = format;

   //keep LADPlayer pointer and idx to buffer table
   pLadPlayer = playPointer;
   mIdx = idx;

   ALOGD("LADPlayBufferInit sampleRate=%d ,ch=%d, format=%d",sampleRate,chNum,format);

   //set blisrc
   uint32 srcBufLen;
   BLI_GetMemSize( sampleRate, chNum, targetSR, LAD_CHANNEL_NUM, &srcBufLen);
   pSrcBuf = new int8[srcBufLen];
   pSrcHdl = BLI_Open( sampleRate, chNum, targetSR, LAD_CHANNEL_NUM, pSrcBuf);
   if ( !pSrcHdl ) return false;

   //buffer pointer setting
   mBuf.pBufBase = new int8[LAD_PLAY_BUFFER_LEN];
   mBuf.pRead = mBuf.pBufBase;
   mBuf.pWrite = mBuf.pBufBase;
   mBuf.bufLen = LAD_PLAY_BUFFER_LEN;

   ALOGD("pBufBase: %p, pRead: %p, pWrite: %p, bufLen:%d \n",mBuf.pBufBase,mBuf.pRead,mBuf.pWrite,mBuf.bufLen);

   //initial mutex for buffer access
#if defined(PC_EMULATION)
   InitializeCriticalSection(&bMutex);
#else
	int ret;
   ret = pthread_mutex_init(&bMutex, NULL);
   if ( ret != 0 ) { return -1; }
#endif

   return true;
}

void LADPlayBuffer::BuffLock()
{
#if defined(PC_EMULATION)
   EnterCriticalSection(&bMutex);
#else
   pthread_mutex_lock( &bMutex );
#endif
}

void LADPlayBuffer::BuffUnLock()
{
#if defined(PC_EMULATION)
   LeaveCriticalSection(&bMutex);
#else
   pthread_mutex_unlock( &bMutex );
#endif
}


//static int glbcnt = 0;

int32 LADPlayBuffer::LADPlayWrite(int8* iBuf, int32 num)
{
   int32 tryCount = 0;
   uint32 inCount, outCount, consumed;
   int8* buf;
   int dataCount = 0;

   LOG_LADPlay("+LADPlayWrite num=%d",num);

   //if VM case call VM write function
   if ( mFormat == android_audio_legacy::AudioSystem::VM_FMT ){
      return LADPlayWriteVM(iBuf, num);
   }

   //num not consumed
   int32 leftCount = num;

   //do 8bit to 16bit conversion if necessary
   if( mFormat == android_audio_legacy::AudioSystem::PCM_8_BIT ) {
      buf = new int8[num * 2];
      ConvertPcm8ToPcm16(iBuf, buf, num);
   }
   else{
      buf = iBuf;
   }
   //keep on trying until leftCount = 0
   do {

      // lock
      BuffLock();

      // if LadPlayer is closed, return this function to revent block
      if(pLadPlayer->runFlag == 0) {
         LOG_LADPlay("pLadPlayer=%x, runFlag=%d",pLadPlayer,pLadPlayer->runFlag );
      	BuffUnLock();
      	return num;
      }
/*
      //get data count in buffer
      dataCount = rb_getDataCount(&mBuf);
      LOG_LADPlay("!!!LADPlayWrite dataCount:%d\n",dataCount);
*/
      if( mBuf.pRead <= mBuf.pWrite ) {
         //printf("glbcnt: %d\n", glbcnt);
         //if ( glbcnt == 3 )
         //   glbcnt = glbcnt;
         //glbcnt++;

         //If write pointer is larger than read, we output from write til end
         inCount = leftCount;
         if ( mBuf.pRead == mBuf.pBufBase )
            outCount = mBuf.pBufBase + mBuf.bufLen - mBuf.pWrite - 2;
         else
            outCount = mBuf.pBufBase + mBuf.bufLen - mBuf.pWrite;

//         LOG_LADPlay("1LADPlayWrite~ inCount=%d, outCount=%d",inCount,outCount);

#ifdef DUMP_BGS_BLI_BUF
         fwrite(buf, sizeof(char), inCount, p_OutFile1);
#endif
         //Do Conversion
         consumed = BLI_Convert(pSrcHdl, (int16*)buf, &inCount, (int16*)mBuf.pWrite, &outCount);
         buf += consumed;


         LOG_LADPlay("1LADPlayWrite consumed=%d, inCount=%d, outCount=%d",consumed,inCount,outCount);

         //Judge if we need to try consume again
         if ( inCount == 0 ) {         // all input buffer is consumed, means no data to copy to rb
            mBuf.pWrite += outCount;   // update write pointer with outcount
            BuffUnLock();
            break;
         }
         else {                           // input buffer is not completely consumed, but ring buffer hits its end
            mBuf.pWrite += outCount;
            if ( mBuf.pWrite == mBuf.pBufBase + mBuf.bufLen )
               mBuf.pWrite = mBuf.pBufBase;  // ring buffer write pointer start from base
            leftCount -= consumed;        // update leftCount try to consume again
         }

         //get data count in buffer
         dataCount = rb_getDataCount(&mBuf);

         LOG_LADPlay("1LADPlayWrite pRead:%d, pWrite:%d, dataCount:%d\n",
            mBuf.pRead - mBuf.pBufBase, mBuf.pWrite - mBuf.pBufBase, dataCount);
      }
      else {
         //If read pointer is larger than write, we output from write til read pointer
         inCount = leftCount;
         outCount = mBuf.pRead - mBuf.pWrite - 2;

//         LOG_LADPlay("2LADPlayWrite~ inCount=%d, outCount=%d",inCount,outCount);

#ifdef DUMP_BGS_BLI_BUF
         fwrite(buf, sizeof(char), inCount, p_OutFile1);
#endif

         //Do Conversion
         consumed = BLI_Convert(pSrcHdl, (int16*)buf, &inCount, (int16*)mBuf.pWrite, &outCount);
         buf += consumed;

         LOG_LADPlay("2LADPlayWrite consumed=%d, inCount=%d, outCount=%d",consumed,inCount,outCount);

         //Judge if we need to try consume again
         if ( inCount == 0 ) {         // all input buffer is consumed, means no data to copy to rb
            mBuf.pWrite += outCount;   // update write pointer with outcount
            BuffUnLock();
            break;
         }
         else {                           // input buffer is not completely consumed, but ring buffer hits read pointer
            mBuf.pWrite += outCount;      // update write pointer
            leftCount -= consumed;        // update leftCount try to consume again
         }

         //get data count in buffer
         dataCount = rb_getDataCount(&mBuf);

         LOG_LADPlay("2LADPlayWrite pRead:%d, pWrite:%d, dataCount:%d\n",
            mBuf.pRead - mBuf.pBufBase, mBuf.pWrite - mBuf.pBufBase, dataCount);
      }

      //unlock
      BuffUnLock();

      AAD_Sleep_1ms(3);      //wait some time and try again

   } while ( tryCount++ < 200 );

      //   printf("exit:-- pRead:%d, pWrite:%d, pBase:0x%x, pEnd:0x%x\n",
      //      mBuf.pRead - mBuf.pBufBase, mBuf.pWrite - mBuf.pBufBase, mBuf.pBufBase, mBuf.pBufBase+ mBuf.bufLen);

   if(tryCount >= 20) ASSERT(0);

   //LOG_LADPlay("-LADPlayWrite tryCount:%d",tryCount);

   // always return num
   return num;
}


/* Output Stream call this to write to linear buffer without sampling rate conversion */
/* debug use only */
/*
int32 LADPlayBuffer::LADPlayWriteNoSR(int8* buf, int32 num)
{
   int8 *pBuf;
   int32 tryCount = 0;

   //num not consumed
   int32 leftCount = num;

   //free space in ring buffer
   int32 freeNumInBuf;

   //keep on trying until leftCount = 0
   do {

      freeNumInBuf = mBuf.bufLen - rb_getDataCount(&mBuf) -1;
      if( freeNumInBuf > leftCount ) {
         rb_copyFromLinear(&mBuf, pBuf, leftCount);
         break;
      }
      rb_copyFromLinear(&mBuf, pBuf, freeNumInBuf);
      leftCount -= freeNumInBuf;
      pBuf += freeNumInBuf;

      AAD_Sleep_1ms(20);
   } while ( tryCount++ >=20 );


   if(tryCount >= 20) ASSERT(0);


   // always return num
   return num;
}
*/

//*****************************************************************************************
//--------------------------for VM playbuffer------------------------------------------
//*****************************************************************************************

int32 LADPlayBuffer::LADPlayWriteVM(int8* iBuf, int32 num)
{
   int8 *pBuf=iBuf;
   int32 tryCount = 0;

   //num not consumed
   int32 leftCount = num;

   //free space in ring buffer
   int32 freeNumInBuf;

   //keep on trying until leftCount = 0
   do {

      freeNumInBuf = mBuf.bufLen - rb_getDataCount(&mBuf) -1;
      if( freeNumInBuf > leftCount ) {
         rb_copyFromLinear(&mBuf, pBuf, leftCount);
         break;
      }
      rb_copyFromLinear(&mBuf, pBuf, freeNumInBuf);
      leftCount -= freeNumInBuf;
      pBuf += freeNumInBuf;

      AAD_Sleep_1ms(20);
   } while ( tryCount++ >=20 );


   if(tryCount >= 20) ASSERT(0);


   // always return num
   return num;
}


//*****************************************************************************************
//--------------------------for LAD Player------------------------------------------
//*****************************************************************************************
#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "LADPlayer"
#include <utils/Log.h>

LADPlayer::~LADPlayer()
{

}

LADPlayer::LADPlayer(AudioYusuHardware *hw)
{
   int32 i,ret=0;

   //initial runFlag
   runFlag = 0;

   // hardware pointer
   mHw = hw;

   // set pointer to LAD
   pLad = mHw->pLad ;

   // get share buffer base pointer
   pShareBuf = (int8 *)pLad->pCCCI->GetA2MShareBufAddress();

   // add offset to make it point to the position we write data
   pShareBuf += LAD_PLAY_DATA_OFFSET;

   // initial all table entry to zero, means non of them are occupied
   for ( i = 0 ; i < MAX_LAD_PLAY_BUFFER_NUM ; i++ )
      mBufTable[i] = 0;

   ret |= pthread_mutex_init(&mLadPlayerWriteMutex, NULL);
   if ( ret != 0 )
       ALOGD("Failed to initialize mLadPlayerWriteMutex components for LADPlayer!");

#ifdef DUMP_BGS_DATA
   pOutFile = NULL;
#endif

    mBGSUplinkGain = MINUS_90_DB;
    mBGSDownlinkGain = -6;
}

void LADPlayer::LADPlayer_SetBGSoundGain(int32 uplinkGain, int32 downlinkGain)
{
   mBGSUplinkGain = uplinkGain;
   mBGSDownlinkGain = downlinkGain;
}

bool LADPlayer::LADPlayer_Open(int32 audioMode)
{
   mAudioMode = audioMode;
   bool ret = true;

   //set runFlag
   runFlag = 1;
   LOG_LADPlay("LADPlayer_Open runFlag=true");

   //search in buffers to see the format we have
   //bcs VM and PCM format are exclusive, we can check only one of the bufer
   int32 i;
   for ( i = 0 ; i < MAX_LAD_PLAY_BUFFER_NUM ; i++ ) {
      if ( mBufTable[i] != NULL )
         break;
   }
   if ( i >= MAX_LAD_PLAY_BUFFER_NUM )    //no stream is inside player, should return false
      return false;

   mFormat = mBufTable[i]->mFormat;

   //VM format------------------------------------------
   if ( mFormat == android_audio_legacy::AudioSystem::VM_FMT )
      return pLad->LAD_PlaybackTurnOn(LADDATA_VM);

#ifdef DUMP_BGS_DATA
   struct tm *timeinfo;
   time_t rawtime;
   time(&rawtime);
   timeinfo=localtime(&rawtime);
   char path[80];
   memset((void*)path,0,80);
   strftime (path,80,"/sdcard/%a_%b_%Y_%H_%M_%S_RecBGS.pcm",timeinfo);
   ALOGD( "fopen path is : %s", path);
   pOutFile = fopen(path,"w");
//   pOutFile = fopen("/data/Record_BGS.pcm","wb");
   if( pOutFile == NULL)
   {
      ALOGD("Fail to Open RecBGS File %s ",pOutFile);
   }
#endif

   //PCM format--------------------------------------
   if(mAudioMode == android_audio_legacy::AudioSystem::MODE_IN_CALL) {    // phone call mode

      //turn on background sound
      ret &= pLad->LAD_BGSoundOn();
      //recover the UL gain
      //ret &= pLad->LAD_BGSoundConfig(mBGSUplinkGain, mBGSDownlinkGain);

      ret &= pLad->LAD_BGSoundConfig(MINUS_90_DB, mBGSDownlinkGain);

   }
   else {                              // normal mode, ring tone mode
      //turn on background sound
      ret &= pLad->LAD_BGSoundOn();

      //set the UL gain to 0
      ret &= pLad->LAD_BGSoundConfig(MINUS_90_DB, mBGSDownlinkGain);
   }

   return ret;

}

bool LADPlayer::LADPlayer_Close()
{
   bool ret =0;
   pthread_mutex_lock( &mLadPlayerWriteMutex );

   //set runFlag
   runFlag = 0;
   LOG_LADPlay("LADPlayer_Close runFlag=false");

   //if empty return false
   //if( !LADPlayer_PlayerIsNotEmpty() ) return false;

   //VM format------------------------------------------
   if ( mFormat == android_audio_legacy::AudioSystem::VM_FMT ){
       ret = pLad->LAD_PlaybackTurnOff();
       pthread_mutex_unlock( &mLadPlayerWriteMutex );
       return ret;
   }

   //PCM format------------------------------------------
   ret =  pLad->LAD_BGSoundOff();

#ifdef DUMP_BGS_DATA
   fclose(pOutFile);
#endif

//   mHw->Set_Recovery_Bgs(false);
   pthread_mutex_unlock( &mLadPlayerWriteMutex );
   return ret;
}

bool LADPlayer::LADPlayer_PlayerIsNotEmpty()
{
   int32 i;

   // search for valid buffer
   for ( i = 0 ; i < MAX_LAD_PLAY_BUFFER_NUM ; i++ ) {
      if ( mBufTable[i] != NULL )
         break;
   }

   //there is some valid buffer
   if ( i < MAX_LAD_PLAY_BUFFER_NUM )
      return true;
   else
      return false;
}

LADPlayBuffer* LADPlayer::LADPlayer_CreateBuffer(uint32 sampleRate, uint32 chNum, int32 format, uint32 targetSR)
{
   LADPlayBuffer* pBuf;

   ALOGD("LADPlayer_CreateBuffer sampleRate=%d ,ch=%d, format=%d, targetSR:%d",sampleRate,chNum,format, targetSR);

   //protection
   ASSERT(format == android_audio_legacy::AudioSystem::VM_FMT || format == android_audio_legacy::AudioSystem::PCM_16_BIT || format == android_audio_legacy::AudioSystem::PCM_8_BIT);

   //VM format------------------------------------------
   if ( format == android_audio_legacy::AudioSystem::VM_FMT ) {

      // if there is some valid buffer, return NULL bcs we do not support mutiple stream for VM
      if ( LADPlayer_PlayerIsNotEmpty() )
         return NULL;
      // if the player is empty, we create a buffer for VM
      else {
         mFormat = format;
         pBuf = new LADPlayBuffer();
         pBuf->LADPlayBufferInit(this, 0, sampleRate, chNum, mFormat, targetSR);
         mBufTable[0] = pBuf;    //start from 0
         return pBuf;
      }
   }
   //PCM format------------------------------------------
   else {

      // if original format = VM, and player is not empty
      if ( mFormat==android_audio_legacy::AudioSystem::VM_FMT  &&  LADPlayer_PlayerIsNotEmpty() )
         return NULL;

      // original format = PCM or player is empty
      else {

         //update format
         mFormat = format;

         //check if free ID
         int32 i;
         for ( i = 0 ; i < MAX_LAD_PLAY_BUFFER_NUM ; i++ ) {
            if ( mBufTable[i] == NULL )
               break;
         }

         //meaning there is free space
         if ( i < MAX_LAD_PLAY_BUFFER_NUM ) {
            pBuf = new LADPlayBuffer();
            pBuf->LADPlayBufferInit(this, i, sampleRate, chNum, mFormat, targetSR);
            mBufTable[i] = pBuf;
            return pBuf;
         }

         return NULL;
      }
   }
}

int32 LADPlayer::LADPlayer_WaitPlayRequestEvent()
{
   int32 ret;

#if defined(PC_EMULATION)
   ret = WaitForSingleObject(mHw->LadPlayRequestEvent,INFINITE);
   ResetEvent(mHw->LadPlayRequestEvent);
#else
   pthread_mutex_lock( &mHw->lprMutex );
   ret = pthread_cond_wait( &mHw->LadPlayRequestEvent, &mHw->lprMutex );
   pthread_mutex_unlock( &mHw->lprMutex );
#endif
   return ret;
}

void LADPlayer::memmix16(int8* tBuf, int8* sBuf, int32 size, bool mix)
{
   // for pcm 16 bit mix
   int32 i = size >> 1;          // num of byte to num of short
   int16* inPtr = (int16*)sBuf;
   int16* outPtr = (int16*)tBuf;

   // if mix mode, do mix to buffer
   // if not mix mode, overwrite the buffer, (first write use)
   if ( mix ) {
      while ( i-- > 0 ) {
         *outPtr += *inPtr;
         outPtr++;
         inPtr++;
      }
   }
   else {
      while ( i-- > 0 ) {
         *outPtr++ = *inPtr++;
      }
   }

}

void LADPlayer::RingBufMixToLinear(int8* targetBuf, LADPlayBuffer* sourceBuf, int32 size, bool mix)
{
   rb* buf1 = &sourceBuf->mBuf;
   int8* pEnd = buf1->pBufBase + buf1->bufLen;

   if ( buf1->pRead < buf1->pWrite ) {
      memmix16(targetBuf, buf1->pRead, size, mix);
      buf1->pRead += size;
   }
   else {
      if ( buf1->pRead + size < pEnd ) {
         memmix16(targetBuf, buf1->pRead, size, mix);
         buf1->pRead += size;
      }
      else {
         int32 size1 = (int32)pEnd - (int32)buf1->pRead;
         int32 size2 = size - size1;
         memmix16(targetBuf, buf1->pRead, size1, mix);
         memmix16(targetBuf + size1, buf1->pBufBase, size2, mix);
         buf1->pRead = buf1->pBufBase + size2;
      }
   }
}

// return the count it mix
int32 LADPlayer::LADPlayer_DoMix(int32 requiredDataCount)
{
   int32 i;
   int32 dataCountInBuf = 0;
   int32 activeBufCount = 0;

   int8* pBuf1 = pShareBuf;

   // search for usaful active buffers
   // if they do not have enough data, fill with zeros
   for ( i = 0 ; i < MAX_LAD_PLAY_BUFFER_NUM ; i++ )
   {
      if ( mBufTable[i] != NULL)
      {
         rb* rb1 = &mBufTable[i]->mBuf;

         //lock
         mBufTable[i]->BuffLock();
         //increment active buffer count
         activeBufCount++;
         //get data count in buffer
         dataCountInBuf = rb_getDataCount(rb1);

         LOG_LADPlay("LADPlayer_DoMix dataCountInBuf=%d",dataCountInBuf );

         //if not enough, fill with zero
         if ( dataCountInBuf < requiredDataCount )
         {
//            rb_writeDataValue(rb1, 0, requiredDataCount - dataCountInBuf);
//            ALOGD("LADPlayer_DoMix buf underflow dataCountInBuf:%d ",dataCountInBuf);
         }
         //unlock
         mBufTable[i]->BuffUnLock();
      }
   }

   //write the share buffer header
   int32 type;
   if(mFormat == android_audio_legacy::AudioSystem::VM_FMT)
   	type = LADBUFID_CCCI_VM_TYPE;
   else
   	type = LADBUFID_CCCI_BGS_TYPE;

   if(dataCountInBuf > requiredDataCount){
      dataCountInBuf = requiredDataCount;
   }

   // if no buffer or no data in buffer just return 0
   if ( activeBufCount  == 0 ) {
      ALOGD("-LADPlayer_DoMix activeBufCount:%d, dataCountInBuf:%d ",activeBufCount,dataCountInBuf);
      dataCountInBuf = 0;
   }

//   int32 inc = WriteShareBufHeader(pBuf1, (int16)type, (int16)requiredDataCount, (int16)A2M_BUF_HEADER);
   int32 inc = WriteShareBufHeader(pBuf1, (int16)type, (int16)dataCountInBuf, (int16)A2M_BUF_HEADER);
   //increment pointer for header
   pBuf1 += inc;

   if(dataCountInBuf == 0){
      ALOGD("--LADPlayer_DoMix dataCountInBuf(0) ");
      return dataCountInBuf;
   }

   // do mix and write to share buffer, assuming share buffer is linear
   bool mix = 0;
   for ( i = 0 ; i < MAX_LAD_PLAY_BUFFER_NUM ;  i++ ) {
      if ( (mBufTable[i] != NULL) ) {
         //lock
         mBufTable[i]->BuffLock();

         //mix
//         RingBufMixToLinear(pBuf1, mBufTable[i], requiredDataCount, mix);
         RingBufMixToLinear(pBuf1, mBufTable[i], dataCountInBuf, mix);
         mix = 1;

         //unlock
         mBufTable[i]->BuffUnLock();
      }
   }

//   return requiredDataCount;
   return dataCountInBuf;
}


int32 LADPlayer::LADPlayer_MixAndPlay(uint32 numDataRequired)
{
   int32 mixCount;
   int32 maxBufLen;
#ifdef BGS_USE_SINE_WAVE
   static int32 i4Count = 0;
   int32 current_count = 0, remain_count = 0;
   int8* tmp_ptr = NULL;
#endif
   //wait for the data request event
   // not using this right now, we do mix and play in CCCI read thread
   //LADPlayer_WaitPlayRequestEvent();

   maxBufLen = A2M_SHARED_BUFFER_OFFSET - LAD_SHARE_HEADER_LEN;//[ALPS00072151]Prevent BGS buffer exceeds 2-way buffer boundary

   //Limit the required length, bcs we have limited linear buffer
   if ( numDataRequired > maxBufLen )
      numDataRequired = maxBufLen;

   //Mix all the play buffer in use into the share buffer
   mixCount = LADPlayer_DoMix(numDataRequired);
   //YAD_LOGW("mixCount : %d\n", mixCount);

   //pthread_mutex_lock( &mLadPlayerWriteMutex );

#ifdef DUMP_BGS_DATA
   fwrite((pShareBuf+LAD_SHARE_HEADER_LEN), sizeof(char), mixCount, pOutFile);
#endif

#ifdef BGS_USE_SINE_WAVE
   remain_count = mixCount;
   tmp_ptr = (pShareBuf+LAD_SHARE_HEADER_LEN);
   if (remain_count > (SIZE_LAD_Tone-i4Count)){
      memcpy(tmp_ptr,LAD_Tone+(i4Count>>1),SIZE_LAD_Tone-i4Count);
      tmp_ptr+=(SIZE_LAD_Tone-i4Count);
      remain_count -= (SIZE_LAD_Tone-i4Count);
      i4Count = 0;
   }
   while(remain_count > SIZE_LAD_Tone){
      memcpy(tmp_ptr,LAD_Tone,SIZE_LAD_Tone);
      tmp_ptr+=SIZE_LAD_Tone;
      remain_count -= SIZE_LAD_Tone;
   }
   if(remain_count > 0){
      memcpy(tmp_ptr,LAD_Tone,remain_count);
      i4Count = remain_count;
   }
#endif

   // send Data Notify to modem side if there is data
   // Even if the mixCount=0, AP side should set CCCI msg to modem side.
   if ( /*mixCount &&*/ runFlag) {
      if (mFormat == android_audio_legacy::AudioSystem::VM_FMT)
         pLad->LAD_BGSoundDataNotify(LAD_PLAY_DATA_OFFSET, mixCount + LAD_SHARE_HEADER_LEN);
      else
         pLad->LAD_BGSoundDataNotify(LAD_PLAY_DATA_OFFSET, mixCount + LAD_SHARE_HEADER_LEN);
   }
   //pthread_mutex_unlock( &mLadPlayerWriteMutex );

   return mixCount;

}

bool LADPlayer::LADPlayer_CheckBufferEmpty()
{
    bool ret = true ;
    for(int i=0; i< MAX_LAD_PLAY_BUFFER_NUM;i++){
    	if(mBufTable[i] != null){
	    ret = false;
	    break;
   	}
   }
    ALOGV("LADPlayer_CheckBufferEmpty return ret = %d",ret);
    return ret;
}



}; // namespace android


