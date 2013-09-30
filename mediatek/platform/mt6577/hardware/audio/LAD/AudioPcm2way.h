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
 *   AudioPcm2way.h
 *
 * Project:
 * --------
 *   Android
 *
 * Description:
 * ------------
 *   PCM2WAY implement (user space)
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
 *
 *******************************************************************************/

#ifndef ANDROID_AUDIO_PCM2WAY_H
#define ANDROID_AUDIO_PCM2WAY_H

/*****************************************************************************
*                     C O M P I L E R   F L A G S
******************************************************************************
*/

//#define DUMP_MODEM_VT_UL_DATA

/*****************************************************************************
*                E X T E R N A L   R E F E R E N C E S
******************************************************************************
*/

#include <stdint.h>
#include <sys/types.h>


/*****************************************************************************
*                          C O N S T A N T S
******************************************************************************
*/


namespace android {

class AudioCCCI;

/*****************************************************************************
*                         D A T A   T Y P E S
******************************************************************************
*/

/*****************************************************************************
*                        C L A S S   D E F I N I T I O N
******************************************************************************
*/

/***********************************************************
*   PCM2WAY Interface -  Play2Way
***********************************************************/
class Play2Way
{
public:
   Play2Way(AudioYusuHardware *hw);
   ~Play2Way();

   void Play2Way_BufLock();
   void Play2Way_BufUnlock();

   int Play2Way_Start(int sample_rate);
   int Play2Way_Stop();
   int Play2Way_Write(void *buffer, int size_bytes);
   int Play2Way_GetFreeBufferCount(void);
   int Play2Way_PutDataToSpeaker(uint32 pcm_dataRequest);

   AudioYusuHardware *mHw;
   AudioCCCI         *pCCCI;
   LAD               *pLad;

   pthread_mutex_t pPlay2Way_Mutex;     // Mutex to protect internal buffer
   char            *pPlay2Way_Buf;
   char            *pA2M_SharedBufPtr;  // A2M Share Buffer
   rb              m_OutputBuf;         // Internal Output Buffer for Put Data to Modem via Receive(Speaker)
   bool            m_Play2Way_Started;
};

/***********************************************************
*   PCM2WAY Interface -  Record2Way
***********************************************************/
class Record2Way
{
public:
   Record2Way(AudioYusuHardware *hw);
   ~Record2Way();

   void Record2Way_BufLock();
   void Record2Way_BufUnlock();
   void Get_M2A_ShareBufferPtr();

   int Record2Way_Start(int sample_rate);
   int Record2Way_Stop();
   int Record2Way_Read(void *buffer, int size_bytes);
   void Record2Way_GetDataFromMicrophone();

   AudioYusuHardware *mHw;
   AudioCCCI         *pCCCI;
   LAD               *pLad;

   pthread_mutex_t pRec2Way_Mutex;    // Mutex to protect internal buffer
   char            *pRecord2Way_Buf;
   rb              m_M2A_ShareBuf;      // M2A Share Buffer
   rb              m_InputBuf;          // Internal Input Buffer for Get From Microphone Data
   bool            m_Rec2Way_Started;
#ifdef DUMP_MODEM_VT_UL_DATA
   FILE *pVTMicFile;
#endif
};



}; // namespace android

#endif
