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

/*******************************************************************************
 *
 * Filename:
 * ---------
 *   AudioYusuDef.h
 *
 * Project:
 * --------
 *   Android
 *
 * Description:
 * ------------
 *   Audio Definition
 *
 * Author:
 * -------
 *   Chipeng    (mtk02308)
 *   Stan Huang (mtk01728)
 *
 *
 *------------------------------------------------------------------------------
 * $Revision$
 * $Modtime:$
 * $Log:$
 *
 *
 *******************************************************************************/

#ifndef _AUDIO_YUSU_DEF_H_
#define _AUDIO_YUSU_DEF_H_

/*****************************************************************************
*                E X T E R N A L   R E F E R E N C E S
******************************************************************************
*/

#ifndef ASSERT
#include <assert.h>
#define ASSERT(x)    assert(x)
#endif

/*****************************************************************************
*                     C O M P I L E R   F L A G S
******************************************************************************
*/

//#define PC_EMULATION
//#define FAKE_CCCI


//----------------log definition---------------------------

#define ENABLE_LOG_HARDWARE
#define ENABLE_LOG_STREAMOUT
#define ENABLE_LOG_STREAMIN
#define ENABLE_LOG_VOLUMECONTROLLER
#define ENABLE_LOG_STREAMHANDLER
//#define ENABLE_LOG_ANA
//#define ENABLE_LOG_AFE
#define ENABLE_LOG_AUDIOFLINGER
#define ENABLE_LOG_LAD

//#define ENABLE_LOG_AUDIOPOLICYSERVICE
//#define ENABLE_LOG_AUDIOPOLICYANAGER
//#define ENABLE_LOG_REAMPLERMTK
//#define ENABLE_LOG_A2DPINTERFACE
//#define ENABLE_LOG_AUDIO_MIXER
//#define ENABLE_LOG_AUDIOHEADSETMESSAGER
//#define ENABLE_LOG_I2S
//#define ENABLE_LOG_I2SSTREAMIN
#define TEMP_SOLUTION_VM

#if defined(PC_EMULATION)
#define YAD_LOGW   printf
#define YAD_LOGE   printf
#define YAD_LOGD   printf
#else
#ifdef  ENABLE_LOG_STREAMIN
   #define YAD_LOGV   ALOGD
#else
   #define YAD_LOGV   ALOGV
#endif
#define YAD_LOGW   ALOGW
#define YAD_LOGE   ALOGE
#define YAD_LOGD   ALOGD
#endif

/*****************************************************************************
*                          C O N S T A N T S
******************************************************************************
*/

/*****************************************************************************
*                         D A T A   T Y P E S
******************************************************************************
*/

//--------------- Type definition ---------------------

#ifndef int8
typedef char int8;
#endif
#ifndef uint8
typedef unsigned char  uint8;
#endif

#ifndef int16
typedef short  int16;
#endif

#ifndef uint16
typedef unsigned short uint16;
#endif

#ifndef int32
typedef int int32;
#endif

#ifndef uint32
typedef unsigned int   uint32;
#endif

#ifndef uint32
typedef unsigned int   UINT32;
#endif

#ifndef true
#define true  1
#endif

#ifndef false
#define false 0
#endif

#ifndef TRUE
#define TRUE  1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef null
#define null 0
#endif

namespace android {

#ifndef int8
    typedef char int8;
#endif
#ifndef uint8
    typedef unsigned char  uint8;
#endif

#ifndef int16
typedef short  int16;
#endif

#ifndef uint16
typedef unsigned short uint16;
#endif

#ifndef int32
typedef int int32;
#endif

#ifndef uint32
typedef unsigned int   uint32;
#endif

#ifndef true
#define true  1
#endif

#ifndef false
#define false 0
#endif

#ifndef null
    #define null 0
#endif

//---------------structure definition------------------------

struct rb {
   char *pBufBase;
   char *pRead;
   char *pWrite;
   int   bufLen;
};

#if defined(PC_EMULATION)

enum audio_mode {
   MODE_NORMAL = 0,
   MODE_RINGTONE = 1,
   MODE_IN_CALL = 2
};

class AudioSystem
{
public:

    enum audio_format {
        FORMAT_DEFAULT = 0,
        PCM_16_BIT,
        PCM_8_BIT,
        VM_FMT,
        INVALID_FORMAT
    };

    enum audio_mode {
        MODE_INVALID = -2,
        MODE_CURRENT = -1,
        MODE_NORMAL = 0,
        MODE_RINGTONE,
        MODE_IN_CALL,
        NUM_MODES  // not a valid entry, denotes end-of-list
    };
};
#endif


/*****************************************************************************
*                        F U N C T I O N   D E F I N I T I O N
******************************************************************************
*/

extern void AAD_Sleep_1ms(int ms);
extern int rb_getDataCount(rb *rb1);
extern void rb_copyToLinear(char *buf, rb *rb1, int count);
extern void rb_copyFromLinear(rb *rb1, char *buf, int count);
extern void rb_copyEmpty(rb *rbt, rb *rbs);
extern void rb_writeDataValue(rb *rb1, int value, int count);

extern void rb_copyFromLinearSRC(void *pSrcHdl, rb *rbt, int8 *buf, int32 num, int32 srt, int32 srs);
extern void rb_copyEmptySRC(void *pSrcHdl, rb *rbt, rb *rbs, int32 srt, int32 srs);
}; // namespace android

#endif   //_AUDIO_YUSU_DEF_H_

