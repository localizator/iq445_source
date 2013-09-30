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
 *   AudioFtm.cpp
 *
 * Project:
 * --------
 *   Android Audio Driver
 *
 * Description:
 * ------------
 *   Factory mode Control
 *
 * Author:
 * -------
 *   Stan Huang (mtk01728)
 *
 *------------------------------------------------------------------------------
 * $Revision: #5 $
 * $Modtime:$
 * $Log:$
 *
 * 04 10 2012 weiguo.li
 * [ALPS00266592] [Need Patch] [Volunteer Patch] ICS_MP patchback to ALPS.ICS of Audio
 * .
 *
 * 01 11 2012 donglei.ji
 * [ALPS00106007] [Need Patch] [Volunteer Patch]DMNR acoustic loopback feature
 * .
 *
 *
 *******************************************************************************/

/*****************************************************************************
*                     C O M P I L E R   F L A G S
******************************************************************************
*/


/*****************************************************************************
*                E X T E R N A L   R E F E R E N C E S
******************************************************************************
*/

#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sched.h>
#include <fcntl.h>

#include <linux/fm.h>

#include <sys/ioctl.h>
#include <utils/Log.h>
#include <utils/String8.h>
#include <hardware_legacy/power.h>

#include "AudioYusuHardware.h"
#include <hardware_legacy/AudioHardwareInterface.h>
#include "AudioAfe.h"
#include "AudioAnalogAfe.h"
#include "AudioFtm.h"
#include "AudioIoctl.h"
#include "AudioYusuDef.h"
#include "audio_custom_exp.h"
#include "AudioYusuStreamOut.h"
#include "AudioYusuCcci.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG  "AudioFtm"

#define I2S0_OUTPUT_USE_FLEXL2

#define BLUETOOTH_LOOPBACK_WAKELOCK_NAME "BLUETOOTH_LOOPBACK_WAKELOCK_NAME"

#define MIC1_OFF  0
#define MIC1_ON   1
#define MIC2_OFF  2
#define MIC2_ON   3

/*****************************************************************************
*                          C O N S T A N T S
******************************************************************************
*/

unsigned char stone1k_48kHz[192]=
    {0x00, 0x00, 0xFF, 0xFF, 0xB5, 0x10, 0xB5, 0x10, 0x21, 0x21, 0x21, 0x21,
     0xFC, 0x30, 0xFC, 0x30, 0x00, 0x40, 0x00, 0x40, 0xEB, 0x4D, 0xEB, 0x4D,
     0x81, 0x5A, 0x82, 0x5A, 0x8C, 0x65, 0x8C, 0x65, 0xD9, 0x6E, 0xD9, 0x6E,
     0x40, 0x76, 0x41, 0x76, 0xA3, 0x7B, 0xA2, 0x7B, 0xE7, 0x7E, 0xE6, 0x7E,
     0xFF, 0x7F, 0xFF, 0x7F, 0xE6, 0x7E, 0xE7, 0x7E, 0xA2, 0x7B, 0xA2, 0x7B,
     0x41, 0x76, 0x41, 0x76, 0xD9, 0x6E, 0xD8, 0x6E, 0x8D, 0x65, 0x8C, 0x65,
     0x82, 0x5A, 0x81, 0x5A, 0xEB, 0x4D, 0xEB, 0x4D, 0xFF, 0x3F, 0xFF, 0x3F,
     0xFB, 0x30, 0xFB, 0x30, 0x20, 0x21, 0x21, 0x21, 0xB5, 0x10, 0xB5, 0x10,
     0x00, 0x00, 0xFF, 0xFF, 0x4B, 0xEF, 0x4B, 0xEF, 0xE0, 0xDE, 0xE0, 0xDE,
     0x05, 0xCF, 0x05, 0xCF, 0x00, 0xC0, 0x00, 0xC0, 0x14, 0xB2, 0x14, 0xB2,
     0x7D, 0xA5, 0x7F, 0xA5, 0x74, 0x9A, 0x75, 0x9A, 0x27, 0x91, 0x26, 0x91,
     0xC0, 0x89, 0xBF, 0x89, 0x5E, 0x84, 0x5D, 0x84, 0x19, 0x81, 0x19, 0x81,
     0x02, 0x80, 0x02, 0x80, 0x19, 0x81, 0x19, 0x81, 0x5E, 0x84, 0x5E, 0x84,
     0xBF, 0x89, 0xBF, 0x89, 0x27, 0x91, 0x28, 0x91, 0x75, 0x9A, 0x74, 0x9A,
     0x7E, 0xA5, 0x7E, 0xA5, 0x15, 0xB2, 0x15, 0xB2, 0x00, 0xC0, 0x00, 0xC0,
     0x04, 0xCF, 0x04, 0xCF, 0xDF, 0xDE, 0xDF, 0xDE, 0x4B, 0xEF, 0x4B, 0xEF};

unsigned char stone1k_m4dB_48kHz[192]=
    {0x00, 0x00, 0x00, 0x00, 0x8A, 0x0A, 0x8B, 0x0A, 0xE6, 0x14, 0xE7, 0x14,
     0xE7, 0x1E, 0xE7, 0x1E, 0x61, 0x28, 0x62, 0x28, 0x29, 0x31, 0x2B, 0x31,
     0x1A, 0x39, 0x1B, 0x39, 0x12, 0x40, 0x12, 0x40, 0xF1, 0x45, 0xF1, 0x45,
     0x9D, 0x4A, 0x9D, 0x4A, 0x02, 0x4E, 0x02, 0x4E, 0x11, 0x50, 0x12, 0x50,
     0xC3, 0x50, 0xC3, 0x50, 0x12, 0x50, 0x11, 0x50, 0x03, 0x4E, 0x02, 0x4E,
     0x9D, 0x4A, 0x9D, 0x4A, 0xF1, 0x45, 0xF1, 0x45, 0x13, 0x40, 0x12, 0x40,
     0x1B, 0x39, 0x1B, 0x39, 0x29, 0x31, 0x2A, 0x31, 0x62, 0x28, 0x62, 0x28,
     0xE8, 0x1E, 0xE7, 0x1E, 0xE7, 0x14, 0xE6, 0x14, 0x8B, 0x0A, 0x8B, 0x0A,
     0x00, 0x00, 0x00, 0x00, 0x76, 0xF5, 0x76, 0xF5, 0x19, 0xEB, 0x19, 0xEB,
     0x18, 0xE1, 0x19, 0xE1, 0x9E, 0xD7, 0x9E, 0xD7, 0xD6, 0xCE, 0xD6, 0xCE,
     0xE6, 0xC6, 0xE5, 0xC6, 0xEE, 0xBF, 0xEE, 0xBF, 0x0F, 0xBA, 0x0F, 0xBA,
     0x64, 0xB5, 0x64, 0xB5, 0xFE, 0xB1, 0xFE, 0xB1, 0xEE, 0xAF, 0xEF, 0xAF,
     0x3D, 0xAF, 0x3D, 0xAF, 0xEF, 0xAF, 0xEE, 0xAF, 0xFE, 0xB1, 0xFE, 0xB1,
     0x63, 0xB5, 0x63, 0xB5, 0x0F, 0xBA, 0x10, 0xBA, 0xED, 0xBF, 0xEE, 0xBF,
     0xE4, 0xC6, 0xE6, 0xC6, 0xD6, 0xCE, 0xD6, 0xCE, 0x9F, 0xD7, 0x9F, 0xD7,
     0x19, 0xE1, 0x18, 0xE1, 0x1A, 0xEB, 0x19, 0xEB, 0x75, 0xF5, 0x76, 0xF5};

unsigned char stone1k_m8dB_48kHz[192]=
    {0x01, 0x00, 0xFF, 0xFF, 0xA7, 0x06, 0xA6, 0x06, 0x31, 0x0D, 0x31, 0x0D,
     0x80, 0x13, 0x80, 0x13, 0x7A, 0x19, 0x7A, 0x19, 0x05, 0x1F, 0x05, 0x1F,
     0x08, 0x24, 0x08, 0x24, 0x6D, 0x28, 0x6D, 0x28, 0x21, 0x2C, 0x22, 0x2C,
     0x14, 0x2F, 0x14, 0x2F, 0x38, 0x31, 0x39, 0x31, 0x85, 0x32, 0x86, 0x32,
     0xF6, 0x32, 0xF5, 0x32, 0x85, 0x32, 0x85, 0x32, 0x37, 0x31, 0x39, 0x31,
     0x14, 0x2F, 0x14, 0x2F, 0x21, 0x2C, 0x21, 0x2C, 0x6D, 0x28, 0x6D, 0x28,
     0x07, 0x24, 0x08, 0x24, 0x05, 0x1F, 0x05, 0x1F, 0x7A, 0x19, 0x7B, 0x19,
     0x81, 0x13, 0x81, 0x13, 0x30, 0x0D, 0x31, 0x0D, 0xA6, 0x06, 0xA7, 0x06,
     0xFF, 0xFF, 0x00, 0x00, 0x5A, 0xF9, 0x59, 0xF9, 0xD0, 0xF2, 0xD0, 0xF2,
     0x80, 0xEC, 0x80, 0xEC, 0x86, 0xE6, 0x86, 0xE6, 0xFB, 0xE0, 0xFB, 0xE0,
     0xF7, 0xDB, 0xF8, 0xDB, 0x93, 0xD7, 0x93, 0xD7, 0xDF, 0xD3, 0xDF, 0xD3,
     0xEC, 0xD0, 0xEC, 0xD0, 0xC7, 0xCE, 0xC8, 0xCE, 0x7A, 0xCD, 0x7B, 0xCD,
     0x0B, 0xCD, 0x0B, 0xCD, 0x7B, 0xCD, 0x7A, 0xCD, 0xC8, 0xCE, 0xC8, 0xCE,
     0xED, 0xD0, 0xEC, 0xD0, 0xDF, 0xD3, 0xDE, 0xD3, 0x93, 0xD7, 0x92, 0xD7,
     0xF8, 0xDB, 0xF8, 0xDB, 0xFB, 0xE0, 0xFB, 0xE0, 0x86, 0xE6, 0x86, 0xE6,
     0x80, 0xEC, 0x81, 0xEC, 0xD0, 0xF2, 0xD0, 0xF2, 0x5A, 0xF9, 0x59, 0xF9};

unsigned char stone2k_48kHz[96]=
    {0x00, 0x00, 0x00, 0x00, 0x20, 0x21, 0x21, 0x21, 0x00, 0x40, 0xFF, 0x3F,
     0x82, 0x5A, 0x83, 0x5A, 0xD9, 0x6E, 0xD9, 0x6E, 0xA2, 0x7B, 0xA3, 0x7B,
     0xFE, 0x7F, 0xFE, 0x7F, 0xA2, 0x7B, 0xA2, 0x7B, 0xD9, 0x6E, 0xD9, 0x6E,
     0x82, 0x5A, 0x82, 0x5A, 0x00, 0x40, 0x00, 0x40, 0x21, 0x21, 0x20, 0x21,
     0x00, 0x00, 0x00, 0x00, 0xE0, 0xDE, 0xDF, 0xDE, 0x00, 0xC0, 0x01, 0xC0,
     0x7E, 0xA5, 0x7E, 0xA5, 0x27, 0x91, 0x27, 0x91, 0x5E, 0x84, 0x5E, 0x84,
     0x01, 0x80, 0x01, 0x80, 0x5E, 0x84, 0x5D, 0x84, 0x27, 0x91, 0x26, 0x91,
     0x7E, 0xA5, 0x7E, 0xA5, 0x00, 0xC0, 0x01, 0xC0, 0xE0, 0xDE, 0xDF, 0xDE};

unsigned char stone2k_m4dB_48kHz[96]=
    {0x00, 0x00, 0x00, 0x00, 0xE8, 0x14, 0xE8, 0x14, 0x62, 0x28, 0x61, 0x28,
     0x1C, 0x39, 0x1B, 0x39, 0xF0, 0x45, 0xF1, 0x45, 0x03, 0x4E, 0x01, 0x4E,
     0xC2, 0x50, 0xC3, 0x50, 0x02, 0x4E, 0x02, 0x4E, 0xF1, 0x45, 0xF1, 0x45,
     0x1B, 0x39, 0x1B, 0x39, 0x61, 0x28, 0x61, 0x28, 0xE6, 0x14, 0xE6, 0x14,
     0xFF, 0xFF, 0x00, 0x00, 0x1A, 0xEB, 0x1A, 0xEB, 0x9E, 0xD7, 0x9F, 0xD7,
     0xE5, 0xC6, 0xE5, 0xC6, 0x0F, 0xBA, 0x0F, 0xBA, 0xFE, 0xB1, 0xFE, 0xB1,
     0x3E, 0xAF, 0x3D, 0xAF, 0xFD, 0xB1, 0xFE, 0xB1, 0x10, 0xBA, 0x0F, 0xBA,
     0xE5, 0xC6, 0xE5, 0xC6, 0x9F, 0xD7, 0x9E, 0xD7, 0x19, 0xEB, 0x19, 0xEB};

unsigned char stone3k_48kHz[64]=
    {0x00, 0x00, 0x00, 0x00, 0xFB, 0x30, 0xFB, 0x30, 0x82, 0x5A, 0x82, 0x5A,
     0x41, 0x76, 0x41, 0x76, 0xFF, 0x7F, 0xFF, 0x7F, 0x40, 0x76, 0x41, 0x76,
     0x82, 0x5A, 0x82, 0x5A, 0xFB, 0x30, 0xFC, 0x30, 0x00, 0x00, 0x00, 0x00,
     0x05, 0xCF, 0x05, 0xCF, 0x7E, 0xA5, 0x7E, 0xA5, 0xBF, 0x89, 0xBF, 0x89,
     0x01, 0x80, 0x01, 0x80, 0xC0, 0x89, 0xC0, 0x89, 0x7D, 0xA5, 0x7E, 0xA5,
     0x05, 0xCF, 0x04, 0xCF};

unsigned char stone3k_m4dB_48kHz[64]=
    {0x00, 0x00, 0xFF, 0xFF, 0xE8, 0x1E, 0xE8, 0x1E, 0x1B, 0x39, 0x1B, 0x39,
     0x9D, 0x4A, 0x9D, 0x4A, 0xC3, 0x50, 0xC2, 0x50, 0x9D, 0x4A, 0x9D, 0x4A,
     0x1A, 0x39, 0x1B, 0x39, 0xE8, 0x1E, 0xE7, 0x1E, 0x00, 0x00, 0x01, 0x00,
     0x18, 0xE1, 0x18, 0xE1, 0xE6, 0xC6, 0xE5, 0xC6, 0x63, 0xB5, 0x63, 0xB5,
     0x3E, 0xAF, 0x3D, 0xAF, 0x63, 0xB5, 0x63, 0xB5, 0xE5, 0xC6, 0xE5, 0xC6,
     0x18, 0xE1, 0x19, 0xE1};

unsigned char stone4k_48kHz[48]=
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x40, 0xD9, 0x6E, 0xD9, 0x6E,
     0xFF, 0x7F, 0xFF, 0x7F, 0xD9, 0x6E, 0xD9, 0x6E, 0xFF, 0x3F, 0xFF, 0x3F,
     0x01, 0x00, 0xFF, 0xFF, 0x01, 0xC0, 0x01, 0xC0, 0x27, 0x91, 0x27, 0x91,
     0x00, 0x80, 0x01, 0x80, 0x27, 0x91, 0x27, 0x91, 0x00, 0xC0, 0x00, 0xC0};

unsigned char stone4k_m4dB_48kHz[48]=
    {0x00, 0x00, 0x01, 0x00, 0x61, 0x28, 0x61, 0x28, 0xF0, 0x45, 0xF1, 0x45,
     0xC2, 0x50, 0xC2, 0x50, 0xF1, 0x45, 0xF1, 0x45, 0x62, 0x28, 0x62, 0x28,
     0x00, 0x00, 0xFF, 0xFF, 0x9F, 0xD7, 0x9F, 0xD7, 0x0F, 0xBA, 0x0F, 0xBA,
     0x3E, 0xAF, 0x3E, 0xAF, 0x0F, 0xBA, 0x10, 0xBA, 0x9F, 0xD7, 0x9F, 0xD7};

unsigned char stone5k_48kHz[192]=
    {0x00, 0x00, 0x00, 0x00, 0xEC, 0x4D, 0xEB, 0x4D, 0xA3, 0x7B, 0xA2, 0x7B,
     0x41, 0x76, 0x41, 0x76, 0xFF, 0x3F, 0xFF, 0x3F, 0x4C, 0xEF, 0x4B, 0xEF,
     0x7F, 0xA5, 0x7E, 0xA5, 0x19, 0x81, 0x19, 0x81, 0x27, 0x91, 0x27, 0x91,
     0x05, 0xCF, 0x05, 0xCF, 0x21, 0x21, 0x20, 0x21, 0x8C, 0x65, 0x8D, 0x65,
     0xFF, 0x7F, 0xFF, 0x7F, 0x8C, 0x65, 0x8C, 0x65, 0x21, 0x21, 0x21, 0x21,
     0x05, 0xCF, 0x04, 0xCF, 0x27, 0x91, 0x27, 0x91, 0x1A, 0x81, 0x19, 0x81,
     0x7E, 0xA5, 0x7E, 0xA5, 0x4B, 0xEF, 0x4B, 0xEF, 0xFF, 0x3F, 0xFF, 0x3F,
     0x41, 0x76, 0x41, 0x76, 0xA2, 0x7B, 0xA2, 0x7B, 0xEB, 0x4D, 0xEC, 0x4D,
     0x00, 0x00, 0x01, 0x00, 0x15, 0xB2, 0x14, 0xB2, 0x5E, 0x84, 0x5E, 0x84,
     0xBF, 0x89, 0xBF, 0x89, 0x01, 0xC0, 0x00, 0xC0, 0xB5, 0x10, 0xB6, 0x10,
     0x81, 0x5A, 0x82, 0x5A, 0xE6, 0x7E, 0xE7, 0x7E, 0xD9, 0x6E, 0xDA, 0x6E,
     0xFC, 0x30, 0xFA, 0x30, 0xDF, 0xDE, 0xE0, 0xDE, 0x75, 0x9A, 0x74, 0x9A,
     0x01, 0x80, 0x01, 0x80, 0x73, 0x9A, 0x75, 0x9A, 0xE0, 0xDE, 0xDF, 0xDE,
     0xFB, 0x30, 0xFC, 0x30, 0xD9, 0x6E, 0xD9, 0x6E, 0xE7, 0x7E, 0xE6, 0x7E,
     0x81, 0x5A, 0x82, 0x5A, 0xB5, 0x10, 0xB5, 0x10, 0x00, 0xC0, 0x01, 0xC0,
     0xC0, 0x89, 0xC0, 0x89, 0x5D, 0x84, 0x5E, 0x84, 0x15, 0xB2, 0x15, 0xB2};

unsigned char stone5k_m4dB_48kHz[192]=
    {0x00, 0x00, 0xFF, 0xFF, 0x2A, 0x31, 0x2A, 0x31, 0x02, 0x4E, 0x02, 0x4E,
     0x9E, 0x4A, 0x9D, 0x4A, 0x62, 0x28, 0x61, 0x28, 0x75, 0xF5, 0x76, 0xF5,
     0xE5, 0xC6, 0xE5, 0xC6, 0xEE, 0xAF, 0xEE, 0xAF, 0x10, 0xBA, 0x0F, 0xBA,
     0x18, 0xE1, 0x19, 0xE1, 0xE7, 0x14, 0xE7, 0x14, 0x13, 0x40, 0x12, 0x40,
     0xC3, 0x50, 0xC2, 0x50, 0x12, 0x40, 0x12, 0x40, 0xE7, 0x14, 0xE6, 0x14,
     0x18, 0xE1, 0x19, 0xE1, 0x10, 0xBA, 0x10, 0xBA, 0xEE, 0xAF, 0xEE, 0xAF,
     0xE5, 0xC6, 0xE5, 0xC6, 0x76, 0xF5, 0x75, 0xF5, 0x61, 0x28, 0x61, 0x28,
     0x9D, 0x4A, 0x9D, 0x4A, 0x02, 0x4E, 0x02, 0x4E, 0x2A, 0x31, 0x29, 0x31,
     0x00, 0x00, 0x00, 0x00, 0xD6, 0xCE, 0xD7, 0xCE, 0xFE, 0xB1, 0xFE, 0xB1,
     0x64, 0xB5, 0x63, 0xB5, 0xA0, 0xD7, 0x9F, 0xD7, 0x8A, 0x0A, 0x8A, 0x0A,
     0x1B, 0x39, 0x1B, 0x39, 0x11, 0x50, 0x12, 0x50, 0xF1, 0x45, 0xF0, 0x45,
     0xE8, 0x1E, 0xE8, 0x1E, 0x19, 0xEB, 0x19, 0xEB, 0xEE, 0xBF, 0xEE, 0xBF,
     0x3D, 0xAF, 0x3D, 0xAF, 0xED, 0xBF, 0xED, 0xBF, 0x19, 0xEB, 0x1A, 0xEB,
     0xE7, 0x1E, 0xE8, 0x1E, 0xF1, 0x45, 0xF0, 0x45, 0x12, 0x50, 0x12, 0x50,
     0x1B, 0x39, 0x1A, 0x39, 0x8B, 0x0A, 0x8A, 0x0A, 0x9E, 0xD7, 0x9F, 0xD7,
     0x63, 0xB5, 0x63, 0xB5, 0xFF, 0xB1, 0xFE, 0xB1, 0xD6, 0xCE, 0xD7, 0xCE};

unsigned char stone6k_48kHz[32]=
    {0x00, 0x00, 0x00, 0x00, 0x82, 0x5A, 0x82, 0x5A, 0xFF, 0x7F, 0xFF, 0x7F,
     0x82, 0x5A, 0x82, 0x5A, 0x00, 0x00, 0x00, 0x00, 0x7E, 0xA5, 0x7E, 0xA5,
     0x02, 0x80, 0x01, 0x80, 0x7F, 0xA5, 0x7F, 0xA5};

unsigned char stone6k_m4dB_48kHz[32]=
    {0xFF, 0xFF, 0x00, 0x00, 0x1B, 0x39, 0x1B, 0x39, 0xC3, 0x50, 0xC3, 0x50,
     0x1B, 0x39, 0x1C, 0x39, 0x00, 0x00, 0x00, 0x00, 0xE4, 0xC6, 0xE5, 0xC6,
     0x3D, 0xAF, 0x3E, 0xAF, 0xE4, 0xC6, 0xE4, 0xC6};

unsigned char stone7k_48kHz[192]=
    {0x00, 0x00, 0xFF, 0xFF, 0x8B, 0x65, 0x8B, 0x65, 0xA2, 0x7B, 0xA3, 0x7B,
     0xFB, 0x30, 0xFB, 0x30, 0x00, 0xC0, 0x00, 0xC0, 0x19, 0x81, 0x19, 0x81,
     0x7F, 0xA5, 0x7E, 0xA5, 0xB5, 0x10, 0xB5, 0x10, 0xD8, 0x6E, 0xD9, 0x6E,
     0x41, 0x76, 0x41, 0x76, 0x20, 0x21, 0x21, 0x21, 0x14, 0xB2, 0x15, 0xB2,
     0x01, 0x80, 0x01, 0x80, 0x15, 0xB2, 0x14, 0xB2, 0x21, 0x21, 0x21, 0x21,
     0x40, 0x76, 0x41, 0x76, 0xD9, 0x6E, 0xD9, 0x6E, 0xB5, 0x10, 0xB5, 0x10,
     0x7D, 0xA5, 0x7E, 0xA5, 0x19, 0x81, 0x1A, 0x81, 0x01, 0xC0, 0x00, 0xC0,
     0xFB, 0x30, 0xFC, 0x30, 0xA3, 0x7B, 0xA3, 0x7B, 0x8C, 0x65, 0x8B, 0x65,
     0x01, 0x00, 0xFF, 0xFF, 0x75, 0x9A, 0x73, 0x9A, 0x5D, 0x84, 0x5E, 0x84,
     0x05, 0xCF, 0x05, 0xCF, 0xFF, 0x3F, 0xFF, 0x3F, 0xE6, 0x7E, 0xE7, 0x7E,
     0x82, 0x5A, 0x81, 0x5A, 0x4C, 0xEF, 0x4B, 0xEF, 0x27, 0x91, 0x27, 0x91,
     0xBE, 0x89, 0xBF, 0x89, 0xE0, 0xDE, 0xDF, 0xDE, 0xEC, 0x4D, 0xEB, 0x4D,
     0xFF, 0x7F, 0xFF, 0x7F, 0xEB, 0x4D, 0xEC, 0x4D, 0xDF, 0xDE, 0xE0, 0xDE,
     0xC0, 0x89, 0xBF, 0x89, 0x27, 0x91, 0x28, 0x91, 0x4B, 0xEF, 0x4B, 0xEF,
     0x82, 0x5A, 0x82, 0x5A, 0xE7, 0x7E, 0xE7, 0x7E, 0xFF, 0x3F, 0x00, 0x40,
     0x05, 0xCF, 0x04, 0xCF, 0x5E, 0x84, 0x5D, 0x84, 0x75, 0x9A, 0x74, 0x9A};

unsigned char stone7k_m4dB_48kHz[192]=
    {0x00, 0x00, 0x00, 0x00, 0x13, 0x40, 0x12, 0x40, 0x02, 0x4E, 0x02, 0x4E,
     0xE9, 0x1E, 0xE8, 0x1E, 0x9E, 0xD7, 0x9F, 0xD7, 0xEE, 0xAF, 0xEF, 0xAF,
     0xE5, 0xC6, 0xE5, 0xC6, 0x8B, 0x0A, 0x8B, 0x0A, 0xF1, 0x45, 0xF1, 0x45,
     0x9C, 0x4A, 0x9D, 0x4A, 0xE7, 0x14, 0xE7, 0x14, 0xD6, 0xCE, 0xD7, 0xCE,
     0x3E, 0xAF, 0x3D, 0xAF, 0xD6, 0xCE, 0xD7, 0xCE, 0xE6, 0x14, 0xE7, 0x14,
     0x9D, 0x4A, 0x9D, 0x4A, 0xF1, 0x45, 0xF1, 0x45, 0x8A, 0x0A, 0x8A, 0x0A,
     0xE6, 0xC6, 0xE5, 0xC6, 0xEF, 0xAF, 0xEE, 0xAF, 0x9F, 0xD7, 0x9F, 0xD7,
     0xE7, 0x1E, 0xE9, 0x1E, 0x03, 0x4E, 0x02, 0x4E, 0x12, 0x40, 0x12, 0x40,
     0x00, 0x00, 0x00, 0x00, 0xEF, 0xBF, 0xEE, 0xBF, 0xFE, 0xB1, 0xFE, 0xB1,
     0x18, 0xE1, 0x19, 0xE1, 0x61, 0x28, 0x61, 0x28, 0x11, 0x50, 0x11, 0x50,
     0x1B, 0x39, 0x1B, 0x39, 0x76, 0xF5, 0x75, 0xF5, 0x10, 0xBA, 0x0F, 0xBA,
     0x63, 0xB5, 0x63, 0xB5, 0x19, 0xEB, 0x19, 0xEB, 0x2A, 0x31, 0x29, 0x31,
     0xC3, 0x50, 0xC3, 0x50, 0x2A, 0x31, 0x2B, 0x31, 0x19, 0xEB, 0x19, 0xEB,
     0x64, 0xB5, 0x63, 0xB5, 0x10, 0xBA, 0x0F, 0xBA, 0x75, 0xF5, 0x76, 0xF5,
     0x1B, 0x39, 0x1B, 0x39, 0x12, 0x50, 0x11, 0x50, 0x61, 0x28, 0x61, 0x28,
     0x18, 0xE1, 0x19, 0xE1, 0xFE, 0xB1, 0xFD, 0xB1, 0xEE, 0xBF, 0xED, 0xBF};

unsigned char stone8k_48kHz[24]=
    {0x00, 0x00, 0x01, 0x00, 0xDA, 0x6E, 0xD8, 0x6E, 0xD9, 0x6E, 0xDA, 0x6E,
     0x00, 0x00, 0x00, 0x00, 0x27, 0x91, 0x26, 0x91, 0x27, 0x91, 0x27, 0x91};

unsigned char stone8k_m4dB_48kHz[24]=
    {0x00, 0x00, 0x01, 0x00, 0xF1, 0x45, 0xF1, 0x45, 0xF1, 0x45, 0xF1, 0x45,
     0xFF, 0xFF, 0x01, 0x00, 0x10, 0xBA, 0x0F, 0xBA, 0x0F, 0xBA, 0x0F, 0xBA};

unsigned char stone9k_48kHz[64]=
    {0x00, 0x00, 0x00, 0x00, 0x40, 0x76, 0x41, 0x76, 0x82, 0x5A, 0x82, 0x5A,
     0x04, 0xCF, 0x04, 0xCF, 0x00, 0x80, 0x01, 0x80, 0x04, 0xCF, 0x04, 0xCF,
     0x82, 0x5A, 0x82, 0x5A, 0x40, 0x76, 0x41, 0x76, 0x00, 0x00, 0xFF, 0xFF,
     0xC0, 0x89, 0xBF, 0x89, 0x7D, 0xA5, 0x7E, 0xA5, 0xFB, 0x30, 0xFB, 0x30,
     0xFF, 0x7F, 0xFF, 0x7F, 0xFC, 0x30, 0xFB, 0x30, 0x7E, 0xA5, 0x7E, 0xA5,
     0xC0, 0x89, 0xBF, 0x89};

unsigned char stone9k_m4dB_48kHz[64]=
    {0x00, 0x00, 0x01, 0x00, 0x9D, 0x4A, 0x9D, 0x4A, 0x1B, 0x39, 0x1B, 0x39,
     0x18, 0xE1, 0x18, 0xE1, 0x3D, 0xAF, 0x3D, 0xAF, 0x19, 0xE1, 0x18, 0xE1,
     0x1B, 0x39, 0x1B, 0x39, 0x9C, 0x4A, 0x9D, 0x4A, 0x00, 0x00, 0x00, 0x00,
     0x63, 0xB5, 0x63, 0xB5, 0xE4, 0xC6, 0xE5, 0xC6, 0xE8, 0x1E, 0xE8, 0x1E,
     0xC3, 0x50, 0xC3, 0x50, 0xE8, 0x1E, 0xE8, 0x1E, 0xE5, 0xC6, 0xE5, 0xC6,
     0x63, 0xB5, 0x63, 0xB5};

unsigned char stone10k_48kHz[96]=
    {0x00, 0x00, 0x00, 0x00, 0xA2, 0x7B, 0xA3, 0x7B, 0x00, 0x40, 0xFF, 0x3F,
     0x7E, 0xA5, 0x7E, 0xA5, 0x27, 0x91, 0x26, 0x91, 0x21, 0x21, 0x21, 0x21,
     0xFF, 0x7F, 0xFF, 0x7F, 0x21, 0x21, 0x20, 0x21, 0x27, 0x91, 0x27, 0x91,
     0x7F, 0xA5, 0x7D, 0xA5, 0x00, 0x40, 0x00, 0x40, 0xA3, 0x7B, 0xA3, 0x7B,
     0x00, 0x00, 0x00, 0x00, 0x5E, 0x84, 0x5E, 0x84, 0x00, 0xC0, 0x01, 0xC0,
     0x82, 0x5A, 0x82, 0x5A, 0xD9, 0x6E, 0xD9, 0x6E, 0xDF, 0xDE, 0xE0, 0xDE,
     0x01, 0x80, 0x00, 0x80, 0xDF, 0xDE, 0xDF, 0xDE, 0xD9, 0x6E, 0xD9, 0x6E,
     0x82, 0x5A, 0x82, 0x5A, 0x01, 0xC0, 0x01, 0xC0, 0x5E, 0x84, 0x5D, 0x84};

unsigned char stone10k_m4dB_48kHz[96]=
    {0x00, 0x00, 0xFF, 0xFF, 0x02, 0x4E, 0x01, 0x4E, 0x61, 0x28, 0x61, 0x28,
     0xE5, 0xC6, 0xE5, 0xC6, 0x10, 0xBA, 0x0E, 0xBA, 0xE7, 0x14, 0xE7, 0x14,
     0xC2, 0x50, 0xC3, 0x50, 0xE7, 0x14, 0xE6, 0x14, 0x10, 0xBA, 0x0F, 0xBA,
     0xE5, 0xC6, 0xE4, 0xC6, 0x61, 0x28, 0x61, 0x28, 0x02, 0x4E, 0x02, 0x4E,
     0x00, 0x00, 0xFF, 0xFF, 0xFE, 0xB1, 0xFE, 0xB1, 0x9F, 0xD7, 0x9F, 0xD7,
     0x1C, 0x39, 0x1C, 0x39, 0xF1, 0x45, 0xF1, 0x45, 0x18, 0xEB, 0x19, 0xEB,
     0x3E, 0xAF, 0x3D, 0xAF, 0x19, 0xEB, 0x1A, 0xEB, 0xF0, 0x45, 0xF1, 0x45,
     0x1C, 0x39, 0x1A, 0x39, 0x9F, 0xD7, 0x9F, 0xD7, 0xFD, 0xB1, 0xFE, 0xB1};

unsigned char stone11k_48kHz[192]=
    {0x00, 0x00, 0xFF, 0xFF, 0xE7, 0x7E, 0xE6, 0x7E, 0x20, 0x21, 0x21, 0x21,
     0xBE, 0x89, 0xBF, 0x89, 0x01, 0xC0, 0x01, 0xC0, 0x8C, 0x65, 0x8C, 0x65,
     0x82, 0x5A, 0x81, 0x5A, 0x14, 0xB2, 0x14, 0xB2, 0x27, 0x91, 0x27, 0x91,
     0xFB, 0x30, 0xFB, 0x30, 0xA3, 0x7B, 0xA2, 0x7B, 0x4B, 0xEF, 0x4B, 0xEF,
     0x01, 0x80, 0x01, 0x80, 0x4A, 0xEF, 0x4C, 0xEF, 0xA3, 0x7B, 0xA2, 0x7B,
     0xFB, 0x30, 0xFB, 0x30, 0x26, 0x91, 0x27, 0x91, 0x14, 0xB2, 0x15, 0xB2,
     0x82, 0x5A, 0x82, 0x5A, 0x8B, 0x65, 0x8C, 0x65, 0x00, 0xC0, 0x01, 0xC0,
     0xBF, 0x89, 0xBF, 0x89, 0x21, 0x21, 0x20, 0x21, 0xE7, 0x7E, 0xE7, 0x7E,
     0x01, 0x00, 0xFF, 0xFF, 0x19, 0x81, 0x1A, 0x81, 0xDF, 0xDE, 0xDF, 0xDE,
     0x41, 0x76, 0x41, 0x76, 0x00, 0x40, 0x00, 0x40, 0x74, 0x9A, 0x74, 0x9A,
     0x7E, 0xA5, 0x7E, 0xA5, 0xEB, 0x4D, 0xEB, 0x4D, 0xD9, 0x6E, 0xD9, 0x6E,
     0x05, 0xCF, 0x05, 0xCF, 0x5E, 0x84, 0x5D, 0x84, 0xB5, 0x10, 0xB4, 0x10,
     0xFE, 0x7F, 0xFF, 0x7F, 0xB4, 0x10, 0xB6, 0x10, 0x5E, 0x84, 0x5D, 0x84,
     0x04, 0xCF, 0x04, 0xCF, 0xDA, 0x6E, 0xD9, 0x6E, 0xEB, 0x4D, 0xEB, 0x4D,
     0x7E, 0xA5, 0x7E, 0xA5, 0x74, 0x9A, 0x74, 0x9A, 0x00, 0x40, 0x00, 0x40,
     0x40, 0x76, 0x41, 0x76, 0xDF, 0xDE, 0xDF, 0xDE, 0x19, 0x81, 0x19, 0x81};

unsigned char stone11k_m4dB_48kHz[192]=
    {0x01, 0x00, 0x00, 0x00, 0x12, 0x50, 0x11, 0x50, 0xE7, 0x14, 0xE6, 0x14,
     0x63, 0xB5, 0x63, 0xB5, 0x9F, 0xD7, 0x9F, 0xD7, 0x12, 0x40, 0x12, 0x40,
     0x1B, 0x39, 0x1B, 0x39, 0xD6, 0xCE, 0xD6, 0xCE, 0x0F, 0xBA, 0x0F, 0xBA,
     0xE8, 0x1E, 0xE8, 0x1E, 0x02, 0x4E, 0x02, 0x4E, 0x75, 0xF5, 0x76, 0xF5,
     0x3D, 0xAF, 0x3D, 0xAF, 0x75, 0xF5, 0x75, 0xF5, 0x02, 0x4E, 0x03, 0x4E,
     0xE7, 0x1E, 0xE8, 0x1E, 0x0F, 0xBA, 0x0F, 0xBA, 0xD6, 0xCE, 0xD6, 0xCE,
     0x1B, 0x39, 0x1C, 0x39, 0x12, 0x40, 0x12, 0x40, 0x9F, 0xD7, 0x9F, 0xD7,
     0x63, 0xB5, 0x64, 0xB5, 0xE7, 0x14, 0xE6, 0x14, 0x12, 0x50, 0x12, 0x50,
     0x01, 0x00, 0x00, 0x00, 0xEE, 0xAF, 0xEE, 0xAF, 0x19, 0xEB, 0x19, 0xEB,
     0x9D, 0x4A, 0x9D, 0x4A, 0x61, 0x28, 0x61, 0x28, 0xED, 0xBF, 0xEE, 0xBF,
     0xE5, 0xC6, 0xE5, 0xC6, 0x2A, 0x31, 0x29, 0x31, 0xF0, 0x45, 0xF1, 0x45,
     0x18, 0xE1, 0x19, 0xE1, 0xFD, 0xB1, 0xFE, 0xB1, 0x8B, 0x0A, 0x8B, 0x0A,
     0xC2, 0x50, 0xC3, 0x50, 0x8A, 0x0A, 0x8A, 0x0A, 0xFE, 0xB1, 0xFF, 0xB1,
     0x18, 0xE1, 0x18, 0xE1, 0xF1, 0x45, 0xF1, 0x45, 0x2A, 0x31, 0x29, 0x31,
     0xE5, 0xC6, 0xE5, 0xC6, 0xED, 0xBF, 0xEE, 0xBF, 0x61, 0x28, 0x62, 0x28,
     0x9D, 0x4A, 0x9D, 0x4A, 0x18, 0xEB, 0x1A, 0xEB, 0xEE, 0xAF, 0xEE, 0xAF};

unsigned char stone12k_48kHz[64]=
    {0x00, 0x00, 0x00, 0x00, 0xFF, 0x7F, 0xFF, 0x7F, 0xFF, 0xFF, 0x00, 0x00,
     0x01, 0x80, 0x01, 0x80, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0x7F, 0xFE, 0x7F,
     0x01, 0x00, 0x00, 0x00, 0x00, 0x80, 0x01, 0x80, 0xFF, 0xFF, 0x00, 0x00,
     0xFF, 0x7F, 0xFF, 0x7F, 0x00, 0x00, 0x00, 0x00, 0x01, 0x80, 0x01, 0x80,
     0x00, 0x00, 0x00, 0x00, 0xFF, 0x7F, 0xFF, 0x7F, 0x00, 0x00, 0x01, 0x00,
     0x00, 0x80, 0x01, 0x80};

unsigned char stone12k_m4dB_48kHz[64]=
    {0x00, 0x00, 0x00, 0x00, 0xC3, 0x50, 0xC2, 0x50, 0xFF, 0xFF, 0x00, 0x00,
     0x3D, 0xAF, 0x3D, 0xAF, 0x00, 0x00, 0x00, 0x00, 0xC3, 0x50, 0xC2, 0x50,
     0x01, 0x00, 0x00, 0x00, 0x3E, 0xAF, 0x3D, 0xAF, 0x00, 0x00, 0x00, 0x00,
     0xC2, 0x50, 0xC3, 0x50, 0x00, 0x00, 0x00, 0x00, 0x3E, 0xAF, 0x3E, 0xAF,
     0x00, 0x00, 0x01, 0x00, 0xC3, 0x50, 0xC3, 0x50, 0x00, 0x00, 0x00, 0x00,
     0x3E, 0xAF, 0x3D, 0xAF};

unsigned char stone13k_48kHz[192]=
    {0x01, 0x00, 0x01, 0x00, 0xE6, 0x7E, 0xE7, 0x7E, 0xDF, 0xDE, 0xE0, 0xDE,
     0xBF, 0x89, 0xBF, 0x89, 0xFF, 0x3F, 0x00, 0x40, 0x8C, 0x65, 0x8C, 0x65,
     0x7E, 0xA5, 0x7E, 0xA5, 0x15, 0xB2, 0x14, 0xB2, 0xD8, 0x6E, 0xD9, 0x6E,
     0xFB, 0x30, 0xFC, 0x30, 0x5E, 0x84, 0x5E, 0x84, 0x4B, 0xEF, 0x4B, 0xEF,
     0xFF, 0x7F, 0xFF, 0x7F, 0x4B, 0xEF, 0x4B, 0xEF, 0x5E, 0x84, 0x5E, 0x84,
     0xFC, 0x30, 0xFB, 0x30, 0xD9, 0x6E, 0xD9, 0x6E, 0x15, 0xB2, 0x15, 0xB2,
     0x7E, 0xA5, 0x7F, 0xA5, 0x8C, 0x65, 0x8B, 0x65, 0x00, 0x40, 0xFF, 0x3F,
     0xC0, 0x89, 0xBF, 0x89, 0xDF, 0xDE, 0xDF, 0xDE, 0xE7, 0x7E, 0xE7, 0x7E,
     0x00, 0x00, 0x00, 0x00, 0x19, 0x81, 0x19, 0x81, 0x20, 0x21, 0x20, 0x21,
     0x41, 0x76, 0x41, 0x76, 0x00, 0xC0, 0x01, 0xC0, 0x75, 0x9A, 0x75, 0x9A,
     0x81, 0x5A, 0x81, 0x5A, 0xEB, 0x4D, 0xEB, 0x4D, 0x27, 0x91, 0x27, 0x91,
     0x04, 0xCF, 0x05, 0xCF, 0xA3, 0x7B, 0xA3, 0x7B, 0xB5, 0x10, 0xB5, 0x10,
     0x02, 0x80, 0x01, 0x80, 0xB5, 0x10, 0xB5, 0x10, 0xA3, 0x7B, 0xA2, 0x7B,
     0x05, 0xCF, 0x05, 0xCF, 0x27, 0x91, 0x26, 0x91, 0xEC, 0x4D, 0xEC, 0x4D,
     0x82, 0x5A, 0x82, 0x5A, 0x74, 0x9A, 0x74, 0x9A, 0x01, 0xC0, 0x00, 0xC0,
     0x40, 0x76, 0x41, 0x76, 0x21, 0x21, 0x21, 0x21, 0x19, 0x81, 0x19, 0x81};

unsigned char stone13k_m4dB_48kHz[192]=
    {0x01, 0x00, 0x00, 0x00, 0x11, 0x50, 0x12, 0x50, 0x19, 0xEB, 0x19, 0xEB,
     0x64, 0xB5, 0x63, 0xB5, 0x62, 0x28, 0x61, 0x28, 0x12, 0x40, 0x12, 0x40,
     0xE5, 0xC6, 0xE5, 0xC6, 0xD5, 0xCE, 0xD6, 0xCE, 0xF1, 0x45, 0xF1, 0x45,
     0xE8, 0x1E, 0xE8, 0x1E, 0xFE, 0xB1, 0xFE, 0xB1, 0x75, 0xF5, 0x76, 0xF5,
     0xC3, 0x50, 0xC3, 0x50, 0x75, 0xF5, 0x76, 0xF5, 0xFE, 0xB1, 0xFD, 0xB1,
     0xE8, 0x1E, 0xE8, 0x1E, 0xF1, 0x45, 0xF0, 0x45, 0xD6, 0xCE, 0xD6, 0xCE,
     0xE5, 0xC6, 0xE4, 0xC6, 0x12, 0x40, 0x13, 0x40, 0x61, 0x28, 0x62, 0x28,
     0x63, 0xB5, 0x63, 0xB5, 0x1A, 0xEB, 0x19, 0xEB, 0x11, 0x50, 0x11, 0x50,
     0x00, 0x00, 0x00, 0x00, 0xEF, 0xAF, 0xEE, 0xAF, 0xE7, 0x14, 0xE6, 0x14,
     0x9D, 0x4A, 0x9D, 0x4A, 0x9E, 0xD7, 0x9E, 0xD7, 0xEE, 0xBF, 0xEE, 0xBF,
     0x1B, 0x39, 0x1B, 0x39, 0x29, 0x31, 0x2A, 0x31, 0x0F, 0xBA, 0x0F, 0xBA,
     0x18, 0xE1, 0x18, 0xE1, 0x03, 0x4E, 0x02, 0x4E, 0x8A, 0x0A, 0x8B, 0x0A,
     0x3D, 0xAF, 0x3E, 0xAF, 0x8B, 0x0A, 0x8A, 0x0A, 0x02, 0x4E, 0x02, 0x4E,
     0x18, 0xE1, 0x17, 0xE1, 0x0F, 0xBA, 0x10, 0xBA, 0x2A, 0x31, 0x2A, 0x31,
     0x1C, 0x39, 0x1B, 0x39, 0xEE, 0xBF, 0xED, 0xBF, 0x9F, 0xD7, 0x9E, 0xD7,
     0x9C, 0x4A, 0x9D, 0x4A, 0xE7, 0x14, 0xE7, 0x14, 0xEE, 0xAF, 0xEF, 0xAF};

unsigned char stone14k_48kHz[96]=
    {0x01, 0x00, 0x00, 0x00, 0xA3, 0x7B, 0xA3, 0x7B, 0x01, 0xC0, 0x01, 0xC0,
     0x7E, 0xA5, 0x7E, 0xA5, 0xD9, 0x6E, 0xDA, 0x6E, 0x21, 0x21, 0x21, 0x21,
     0x00, 0x80, 0x01, 0x80, 0x21, 0x21, 0x21, 0x21, 0xD9, 0x6E, 0xDA, 0x6E,
     0x7F, 0xA5, 0x7E, 0xA5, 0x00, 0xC0, 0x01, 0xC0, 0xA3, 0x7B, 0xA3, 0x7B,
     0x00, 0x00, 0x00, 0x00, 0x5D, 0x84, 0x5E, 0x84, 0x00, 0x40, 0x00, 0x40,
     0x81, 0x5A, 0x82, 0x5A, 0x27, 0x91, 0x26, 0x91, 0xE0, 0xDE, 0xE0, 0xDE,
     0xFF, 0x7F, 0xFF, 0x7F, 0xE0, 0xDE, 0xDE, 0xDE, 0x27, 0x91, 0x26, 0x91,
     0x81, 0x5A, 0x82, 0x5A, 0x00, 0x40, 0x00, 0x40, 0x5D, 0x84, 0x5D, 0x84};

unsigned char stone14k_m4dB_48kHz[96]=
    {0x00, 0x00, 0x00, 0x00, 0x02, 0x4E, 0x02, 0x4E, 0x9E, 0xD7, 0x9E, 0xD7,
     0xE5, 0xC6, 0xE5, 0xC6, 0xF0, 0x45, 0xF0, 0x45, 0xE7, 0x14, 0xE7, 0x14,
     0x3D, 0xAF, 0x3E, 0xAF, 0xE7, 0x14, 0xE6, 0x14, 0xF1, 0x45, 0xF1, 0x45,
     0xE5, 0xC6, 0xE5, 0xC6, 0x9F, 0xD7, 0x9F, 0xD7, 0x03, 0x4E, 0x02, 0x4E,
     0x00, 0x00, 0x00, 0x00, 0xFD, 0xB1, 0xFD, 0xB1, 0x61, 0x28, 0x62, 0x28,
     0x1B, 0x39, 0x1B, 0x39, 0x0F, 0xBA, 0x0F, 0xBA, 0x19, 0xEB, 0x1A, 0xEB,
     0xC3, 0x50, 0xC3, 0x50, 0x19, 0xEB, 0x1A, 0xEB, 0x0F, 0xBA, 0x0F, 0xBA,
     0x1B, 0x39, 0x1B, 0x39, 0x62, 0x28, 0x61, 0x28, 0xFE, 0xB1, 0xFD, 0xB1};

unsigned char stone15k_48kHz[64]=
    {0x00, 0x00, 0x00, 0x00, 0x41, 0x76, 0x41, 0x76, 0x7E, 0xA5, 0x7E, 0xA5,
     0x05, 0xCF, 0x05, 0xCF, 0xFF, 0x7F, 0xFF, 0x7F, 0x05, 0xCF, 0x04, 0xCF,
     0x7E, 0xA5, 0x7E, 0xA5, 0x41, 0x76, 0x41, 0x76, 0x00, 0x00, 0x00, 0x00,
     0xBF, 0x89, 0xBF, 0x89, 0x82, 0x5A, 0x82, 0x5A, 0xFB, 0x30, 0xFB, 0x30,
     0x01, 0x80, 0x02, 0x80, 0xFB, 0x30, 0xFB, 0x30, 0x82, 0x5A, 0x81, 0x5A,
     0xBF, 0x89, 0xBF, 0x89};

unsigned char stone15k_m4dB_48kHz[64]=
    {0xFF, 0xFF, 0x00, 0x00, 0x9D, 0x4A, 0x9D, 0x4A, 0xE4, 0xC6, 0xE4, 0xC6,
     0x18, 0xE1, 0x19, 0xE1, 0xC3, 0x50, 0xC3, 0x50, 0x18, 0xE1, 0x18, 0xE1,
     0xE6, 0xC6, 0xE4, 0xC6, 0x9D, 0x4A, 0x9C, 0x4A, 0x00, 0x00, 0x00, 0x00,
     0x64, 0xB5, 0x63, 0xB5, 0x1B, 0x39, 0x1B, 0x39, 0xE7, 0x1E, 0xE8, 0x1E,
     0x3D, 0xAF, 0x3E, 0xAF, 0xE8, 0x1E, 0xE8, 0x1E, 0x1B, 0x39, 0x1B, 0x39,
     0x62, 0xB5, 0x63, 0xB5};


/*****************************************************************************
*                          D A T A   T Y P E S
******************************************************************************
*/
static struct fm_i2s_setting Setting_FM;

#define FTM_Audio_FMTx   0
#define FTM_Audio_HDMI   1

/*****************************************************************************
*                        F U N C T I O N   D E F I N I T I O N
******************************************************************************
*/

namespace android {

static void FM_I2S_Setting()
{
    int fd_FM = -1, ret_FM = 0;

    fd_FM = open(FM_DEVICE_NAME, O_RDWR);
    if(fd_FM < 0){
        ALOGE("Open 'dev/fm' Fail !fd_FM(%d)", fd_FM);
    }
    else
    {
        ret_FM = ioctl(fd_FM, FM_IOCTL_I2S_SETTING, &Setting_FM);
        if(ret_FM){
            ALOGE("set ioctl FM_IOCTL_I2S_SETTING Fail !ret_FM(%d)", ret_FM);
        }
    }
    close(fd_FM);
}

AudioFtm::AudioFtm(AudioYusuHardware *hw, AudioAfe *pAfe, AudioAnalog *pAnaAfe)
{
   mFd = -1;
   mSamplingRate = 0;
   mAudioHardware = hw;
   mAfe           = pAfe;
   mAnalog        = pAnaAfe;
   mAudioSinWave_thread = false;
   mAudioBuffer = NULL;
   mSineWaveStatus = 0;
   mLad = NULL;

   eCurrentLoopbackType = NO_LOOPBACK;
   bSwapADDA = bConnectAllADDA = bUseDMNR = false;
   mIsLoopbackDefaultUseLoudSpeaker = false; // default receiver
   return;
}

AudioFtm::~AudioFtm()
{
   return;
}

bool AudioFtm::AudFtm_Init(uint32 Fd, LAD *pLad)
{
   ALOGD("AudFtm_Init \n");
   mFd = Fd;
   if( mFd <0 )
   {
      ALOGD("AudFtm_Init Fail \n");
      return false;
   }

   mLad = pLad;
   pthread_mutex_init(&mLoopbackMutex, NULL);

   return true;
}

unsigned int AudioFtm::WavGen_AudioWrite(void* buffer, unsigned int bytes)
{
   uint32 consume_byte = 0, i;
   consume_byte = ::write(mFd, buffer, bytes);
   return consume_byte;
}

void AudioFtm::WavGen_AudioRead(char * pBuffer, unsigned int bytes)
{
   int copysize, IdxAudioPattern_Next, i, TotalSample;
   char *copybuffer = NULL;
   copybuffer =pBuffer;
   TotalSample = bytes>>1;

   copysize = SizeAudioPattern - IdxAudioPattern;
   ALOGV("WavGen_AudioRead IdxAudioPattern=%d, SizeAudioPattern=%d, copysize=%d, bytes=%d,", IdxAudioPattern, SizeAudioPattern, copysize, bytes);

   while(bytes){
      memcpy((void*)copybuffer, (void*)g_i2VDL_DATA+IdxAudioPattern, copysize);
      bytes -= copysize;
      copybuffer += copysize;
      if(bytes >=SizeAudioPattern){
         copysize = SizeAudioPattern;
         IdxAudioPattern = 0;
      }
      else{
         copysize = bytes;
         IdxAudioPattern_Next = bytes;
      }

      ALOGV("in loop IdxAudioPattern=%d, SizeAudioPattern=%d, copysize=%d, bytes=%d,", IdxAudioPattern, SizeAudioPattern, copysize, bytes);
   }
   IdxAudioPattern = IdxAudioPattern_Next;
}

void AudioFtm::FmTx_thread_digital_out(void)
{
   int SizeByte, numread = 0, i;

   mFlag_Aud_DL1_SlaveOn = ::ioctl(mFd, GET_DL1_SLAVE_MODE,0);
   if (mFlag_Aud_DL1_SlaveOn)
   {
      SizeByte = 9984;
   }
   else
   {
      SizeByte = 12288;
   }

   if (mAudioBuffer == NULL)
      mAudioBuffer = new char[SizeByte];

   //config register
   mSamplingRate = 48000;
   IdxAudioPattern = 0;
   memset(mAudioBuffer, 0, SizeByte);

   ALOGD("FmTx_thread_digital_out: SizeByte=%d, g_i2VDL_DATA addr=0x%x", SizeByte, g_i2VDL_DATA);
   ::ioctl(mFd, AUD_SET_CLOCK, 1);
   ::ioctl(mFd, START_DL1_STREAM, 0);
   mAfe->Afe_DL_Start(AFE_MODE_I2S0_OUT);
   mAfe->Afe_Set_Stereo();
   mAfe->SetAfeReg(AFE_DL_SRC2_2, 0xffff0000, 0xffff0000);
   mAfe->SetAfeReg(AFE_DL_SRC1_1, 0x1, 0xf);
   //set gpio 200,201,202
   ALOGD("SetFmDigitalOutEnable AUDDRV_SET_FM_I2S_GPIO");
   ::ioctl(mFd, AUDDRV_SET_FM_I2S_GPIO);// enable FM use I2S
   Setting_FM.onoff = 0;
   Setting_FM.mode = 1;
   Setting_FM.sample = 2;
   FM_I2S_Setting();
   mAfe->Afe_Set_Timer(IRQ1_MCU, SizeByte/12); // time = 1024 sample (1024*2*2 = 4096 bytes/interrupt)
   while (1)
   {
      if(!mAudioSinWave_thread)
      {
         mAfe->Afe_DL_Stop(AFE_MODE_I2S0_OUT);
         mAfe->SetAfeReg(AFE_DL_SRC2_2, 0xffff0000, 0xffff0000);
         mAfe->SetAfeReg(AFE_DL_SRC1_1, 0, 0xf);
         //reset gpio
         ALOGD("SetFmDigitalInEnable AUDDRV_RESET_BT_FM_GPIO");
         ::ioctl(mFd, AUDDRV_RESET_BT_FM_GPIO);// Reset GPIO pin mux
         ::ioctl(mFd, STANDBY_DL1_STREAM, 0);

         ::ioctl(mFd, AUD_SET_CLOCK, 0);
         ALOGD("break FmTx_thread_digital_out thread");
         break;
      }
      //Read pattern and write to asm buf
      WavGen_AudioRead(mAudioBuffer, SizeByte);
      WavGen_AudioWrite(mAudioBuffer, SizeByte);
   }

   if (mAudioBuffer) {
      delete [] mAudioBuffer;
      mAudioBuffer = NULL;
   }
}

void  AudioFtm::FmTx_thread_analog_out(void)
{
   int SizeByte, numread = 0, i;

   mFlag_Aud_DL1_SlaveOn = ::ioctl(mFd, GET_DL1_SLAVE_MODE,0);
   if (mFlag_Aud_DL1_SlaveOn)
   {
      SizeByte = 9984;
   }
   else
   {
      SizeByte = 12288;
   }

   if (mAudioBuffer == NULL)
      mAudioBuffer = new char[SizeByte];

   //config register
   mSamplingRate = 48000;
   IdxAudioPattern = 0;
   memset(mAudioBuffer, 0, SizeByte);

   ALOGD("FmTx_thread_analog_out: SizeByte=%d, g_i2VDL_DATA addr=0x%x", SizeByte, g_i2VDL_DATA);
   ::ioctl(mFd, AUD_SET_CLOCK, 1);
   ::ioctl(mFd, START_DL1_STREAM, 0);
   mAfe->Afe_DL_Start(AFE_MODE_DAC);
   mAfe->Afe_Set_Stereo();
   mAfe->SetAfeReg(AFE_DL_SRC2_2, 0xffff0000, 0xffff0000);
   mAfe->SetAfeReg(AFE_DL_SRC1_1, 0x1, 0xf);
   mAfe->Afe_Set_Timer(IRQ1_MCU, SizeByte/12); // time = 1024 sample (1024*2*2 = 4096 bytes/interrupt)
   while (1)
   {
      if(!mAudioSinWave_thread)
      {
         mAfe->Afe_DL_Stop(AFE_MODE_DAC);
         mAfe->SetAfeReg(AFE_DL_SRC1_1, 0, 0xf);
         //reset gpio
         ::ioctl(mFd, STANDBY_DL1_STREAM, 0);

         ::ioctl(mFd, AUD_SET_CLOCK, 0);
         ALOGD("break FmTx_thread_analog_out thread");
         break;
      }
      //Read pattern and write to asm buf
      WavGen_AudioRead(mAudioBuffer, SizeByte);
      WavGen_AudioWrite(mAudioBuffer, SizeByte);
   }

   if (mAudioBuffer) {
      delete [] mAudioBuffer;
      mAudioBuffer = NULL;
   }
}

void AudioFtm::HDMI_thread_I2SOutput(void)
{
   int SizeByte, numread = 0, i;

   mFlag_Aud_DL1_SlaveOn = ::ioctl(mFd, GET_DL1_SLAVE_MODE,0);
   if (mFlag_Aud_DL1_SlaveOn)
   {
      SizeByte = 9984;
   }
   else
   {
      SizeByte = 12288;
   }

   if (mAudioBuffer == NULL)
      mAudioBuffer = new char[SizeByte];

   //config register
   mSamplingRate = 44100;
   IdxAudioPattern = 0;
   memset(mAudioBuffer, 0, SizeByte);

   ALOGD("HDMI_thread_I2SOutput: SizeByte=%d, g_i2VDL_DATA addr=0x%x", SizeByte, g_i2VDL_DATA);
   ::ioctl(mFd, AUD_SET_HDMI_CLOCK, 1);
   ::ioctl(mFd, START_DL1_STREAM, 0);

   mAfe->Afe_DL_Start(AFE_MODE_I2S1_OUT_HDMI);
   mAfe->Afe_Set_Stereo();
   mAfe->SetAfeReg(AFE_DL_SRC2_2, 0xffff0000, 0xffff0000);
   mAfe->SetAfeReg(AFE_DL_SRC1_1, 0x1, 0xf);

   //set gpio 53/54/55
//   ALOGD("HDMI_thread_I2SOutput, AUD_SET_HDMI_GPIO");
//   ::ioctl(mFd, AUD_SET_HDMI_GPIO, 1);// enable FM use I2S

   // Set HDMI Tx driver sampling rate
   // to do


   mAfe->Afe_Set_Timer(IRQ1_MCU, SizeByte/12); // time = 1024 sample (1024*2*2 = 4096 bytes/interrupt)
   while (1)
   {
      if(!mAudioSinWave_thread)
      {
         mAfe->Afe_DL_Stop(AFE_MODE_I2S1_OUT_HDMI);
         mAfe->SetAfeReg(AFE_DL_SRC2_2, 0xffff0000, 0xffff0000);
         mAfe->SetAfeReg(AFE_DL_SRC1_1, 0, 0xf);
         //reset gpio
         ALOGD("HDMI_thread_I2SOutput, AUDDRV_RESET_BT_FM_GPIO");
//         ::ioctl(mFd, AUD_SET_HDMI_GPIO, 0);// Reset GPIO pin mux
         ::ioctl(mFd, STANDBY_DL1_STREAM, 0);

         ::ioctl(mFd, AUD_SET_HDMI_CLOCK, 0);
         ALOGD("exit HDMI_thread_I2SOutput thread");
         break;
      }
      //Read pattern and write to asm buf
      WavGen_AudioRead(mAudioBuffer, SizeByte);
      WavGen_AudioWrite(mAudioBuffer, SizeByte);
   }

   if (mAudioBuffer) {
      delete [] mAudioBuffer;
      mAudioBuffer = NULL;
   }
}

void *AudioFtm::FmTx_thread_create(void *arg)
{
   AudioFtm *p;
   p = (AudioFtm *)arg;
#ifdef FM_DIGITAL_OUT_SUPPORT
   ALOGD("FmTx_thread_create, I2S");
   p->FmTx_thread_digital_out();

#elif defined( FM_ANALOG_OUT_SUPPORT)
   ALOGD("FmTx_thread_create, Analog");
   p->FmTx_thread_analog_out();

#else
   ALOGD("not supported!!!");

#endif
   return NULL;
}

void *AudioFtm::HDMI_thread_create(void *arg)
{
   AudioFtm *p;
   p = (AudioFtm *)arg;

   ALOGD("HDMI_thread_I2SOutput");
   p->HDMI_thread_I2SOutput();

   return NULL;
}

bool AudioFtm::WavGen_SW_SineWave(bool Enable, uint32 Freq, int type)
{
   return WavGen_SWPattern(Enable, Freq, type);
}

bool AudioFtm::WavGen_SWPattern(bool Enable, uint32 Freq, int type)
{
   uint32 Reg_Freq, Reg;
   ALOGD("WavGen_SW_SineWave, Enable=%d,  Freq = %d\n", Enable, Freq);

   if(Enable)
   {
      if(!mAudioSinWave_thread)
      {
         if((Freq <= FREQ_NONE)||(Freq > FREQ_15K_HZ))
         {
            ALOGE("Invalid SineWave Frequency %d", Freq);
            return false;
         }
         else
         {
            switch(Freq)
            {
               case FREQ_1K_HZ:
                  SizeAudioPattern = 192;
                  g_i2VDL_DATA = &stone1k_m4dB_48kHz[0];
                  break;
               case FREQ_2K_HZ:
                  SizeAudioPattern = 96;
                  g_i2VDL_DATA = &stone2k_m4dB_48kHz[0];
                  break;
               case FREQ_3K_HZ:
                  SizeAudioPattern = 64;
                  g_i2VDL_DATA = &stone3k_m4dB_48kHz[0];
                  break;
               case FREQ_4K_HZ:
                  SizeAudioPattern = 48;
                  g_i2VDL_DATA = &stone4k_m4dB_48kHz[0];
                  break;
               case FREQ_5K_HZ:
                  SizeAudioPattern = 192;
                  g_i2VDL_DATA = &stone5k_m4dB_48kHz[0];
                  break;
               case FREQ_6K_HZ:
                  SizeAudioPattern = 32;
                  g_i2VDL_DATA = &stone6k_m4dB_48kHz[0];
                  break;
               case FREQ_7K_HZ:
                  SizeAudioPattern = 192;
                  g_i2VDL_DATA = &stone7k_m4dB_48kHz[0];
                  break;
               case FREQ_8K_HZ:
                  SizeAudioPattern = 24;
                  g_i2VDL_DATA = &stone8k_m4dB_48kHz[0];
                  break;
               case FREQ_9K_HZ:
                  SizeAudioPattern = 64;
                  g_i2VDL_DATA = &stone9k_m4dB_48kHz[0];
                  break;
               case FREQ_10K_HZ:
                  SizeAudioPattern = 96;
                  g_i2VDL_DATA = &stone10k_m4dB_48kHz[0];
                  break;
               case FREQ_11K_HZ:
                  SizeAudioPattern = 192;
                  g_i2VDL_DATA = &stone11k_m4dB_48kHz[0];
                  break;
               case FREQ_12K_HZ:
                  SizeAudioPattern = 64;
                  g_i2VDL_DATA = &stone12k_m4dB_48kHz[0];
                  break;
               case FREQ_13K_HZ:
                  SizeAudioPattern = 192;
                  g_i2VDL_DATA = &stone13k_m4dB_48kHz[0];
                  break;
               case FREQ_14K_HZ:
                  SizeAudioPattern = 96;
                  g_i2VDL_DATA = &stone14k_m4dB_48kHz[0];
                  break;
               case FREQ_15K_HZ:
                  SizeAudioPattern = 64;
                  g_i2VDL_DATA = &stone15k_m4dB_48kHz[0];
                  break;
               default:
                  SizeAudioPattern = 192;
                  g_i2VDL_DATA = &stone1k_m4dB_48kHz[0];
                  break;
             }
         }

         if(type == FTM_Audio_FMTx){
            pthread_create(&m_WaveThread, NULL, &AudioFtm::FmTx_thread_create, (void*)this);
         }
         else if( type== FTM_Audio_HDMI){
            pthread_create(&m_WaveThread, NULL, &AudioFtm::HDMI_thread_create, (void*)this);
         }
         else{
            ALOGD("WavGen_SWPattern fail");
         }

         mAudioSinWave_thread = true;
      }
      return true;
   }
   else
   {//WavGen_SWPattern
      if(mAudioSinWave_thread)
      {
         //leave thread
         ALOGD("WavGen_SWPattern, destroy m_WaveThread");
         mAudioSinWave_thread = false;
         pthread_join(m_WaveThread, NULL);
      }
      return true;
   }
}

// for test
void AudioFtm::Afe_Enable_SineWave(bool bEnable )
{
   ALOGD("Afe_Enable_SineWave, bEnable:%d \n",bEnable);
   uint32 value = 0x0;
   uint32 mask  = 0x0;
   if( bEnable )
   {
      mAfe->GetAfeReg(AUDIO_TOP_CON0, &ftm_rec_reg.rAUDIO_TOP_CON0);
      mAfe->GetAfeReg(AFE_DAC_CON0, &ftm_rec_reg.rAFE_DAC_CON0);
      mAfe->GetAfeReg(AFE_TOP_CONTROL_0, &ftm_rec_reg.rAFE_TOP_CONTROL_0);
      mAfe->GetAfeReg(AFE_DL_SRC1_1, &ftm_rec_reg.rAFE_DL_SRC1_1);
      mAfe->GetAfeReg(AFE_DL_SRC2_1, &ftm_rec_reg.rAFE_DL_SRC2_1);
      mAfe->GetAfeReg(AFE_DL_SRC2_2, &ftm_rec_reg.rAFE_DL_SRC2_2);
      mAfe->GetAfeReg(AFE_UL_SRC_1, &ftm_rec_reg.rAFE_UL_SRC_1);
      mAfe->GetAfeReg(AFE_SDM_GAIN_STAGE, &ftm_rec_reg.rAFE_SDM_GAIN_STAGE);
      mAfe->GetAfeReg(AFE_DL_SDM_CON0, &ftm_rec_reg.rAFE_DL_SDM_CON0);

//      SetAfeReg(AUDIO_TOP_CON0,0x0,0xffffffff);
      mAfe->SetAfeReg(AFE_DAC_CON0,0x1,0x1);

      // Add to set DL1_MODE (sampling rate)
      mAfe->SetAfeReg(AFE_DAC_CON1,0x0,0xfffff);
      mAfe->SetAfeReg(AFE_UL_SRC_0,0x0,0xf0000);

      mAfe->SetAfeReg(AFE_TOP_CONTROL_0,0x40000000,0xffffffff);
      mAfe->SetAfeReg(AFE_DL_SRC1_1,0x1,0x1);
      mAfe->SetAfeReg(AFE_DL_SRC2_1,0x63000603,0xf3000ff3); // sample rate: 32k Hz
      mAfe->SetAfeReg(AFE_DL_SRC2_2,0xf0000000,0xffff0000);  // gain setting twice  -66dBv:0x00400000 0dBv:0xf0000000 mute:0x00000000
      mAfe->SetAfeReg(AFE_UL_SRC_1,0x08e26e26,0xffffffff);   // freq setting 750Hz: 1.5KHz:0x08828828, 1kHz:0x08e26e26(full swing)
      mAfe->SetAfeReg(AFE_SDM_GAIN_STAGE,0x1e,0xffff00ff);  // SDM gain (1e->1c)
      mAfe->SetAfeReg(AFE_DL_SDM_CON0,0x08800000,0xffffffff);  // SDM choose (2-Order 9-Bit Scrambler and No Dithering)
      mSineWaveStatus = 1;
   }
   else
   {
//      SetAfeReg(AUDIO_TOP_CON0,0x4,0x4);   // bit2: power down AFE clock
      if (mSineWaveStatus == 1)
      {
         mAfe->SetAfeReg(AUDIO_TOP_CON0,ftm_rec_reg.rAUDIO_TOP_CON0,MASK_ALL);
         mAfe->SetAfeReg(AFE_DAC_CON0,ftm_rec_reg.rAFE_DAC_CON0,MASK_ALL);
         mAfe->SetAfeReg(AFE_TOP_CONTROL_0,ftm_rec_reg.rAFE_TOP_CONTROL_0,MASK_ALL);
         mAfe->SetAfeReg(AFE_DL_SRC1_1,ftm_rec_reg.rAFE_DL_SRC1_1,MASK_ALL);
         mAfe->SetAfeReg(AFE_DL_SRC2_1,ftm_rec_reg.rAFE_DL_SRC2_1,MASK_ALL);
         mAfe->SetAfeReg(AFE_DL_SRC2_2,ftm_rec_reg.rAFE_DL_SRC2_2,MASK_ALL);
         mAfe->SetAfeReg(AFE_UL_SRC_1,ftm_rec_reg.rAFE_UL_SRC_1,MASK_ALL);
         mAfe->SetAfeReg(AFE_SDM_GAIN_STAGE,ftm_rec_reg.rAFE_SDM_GAIN_STAGE,MASK_ALL);
         mAfe->SetAfeReg(AFE_DL_SDM_CON0,ftm_rec_reg.rAFE_DL_SDM_CON0,MASK_ALL);
         mSineWaveStatus = 0;
      }
   }
}

void AudioFtm::FTM_AFE_UL_Loopback(bool bEnable, int digital_mic)
{
    ALOGD("FTM_AFE_UL_Loopback, bEnable:%d \n",bEnable);

    if (bEnable)
    {
        //Store register values.
        mAfe->GetAfeReg(AUDIO_TOP_CON0, &ftm_rec_reg.rAUDIO_TOP_CON0);
        mAfe->GetAfeReg(AFE_DAC_CON0,&ftm_rec_reg.rAFE_DAC_CON0);
        mAfe->GetAfeReg(AFE_DL_SRC2_1,&ftm_rec_reg.rAFE_DL_SRC2_1);
        mAfe->GetAfeReg(AFE_DL_SRC2_2,&ftm_rec_reg.rAFE_DL_SRC2_2);
        mAfe->GetAfeReg(AFE_UL_AGC0,&ftm_rec_reg.rAFE_UL_AGC0);
        mAfe->GetAfeReg(AFE_SDM_GAIN_STAGE,&ftm_rec_reg.rAFE_SDM_GAIN_STAGE);
        mAfe->GetAfeReg(AFE_DL_SDM_CON0,&ftm_rec_reg.rAFE_DL_SDM_CON0);
        mAfe->GetAfeReg(AFE_CONN1,&ftm_rec_reg.rAFE_CONN1);
        mAfe->GetAfeReg(AFE_CONN2,&ftm_rec_reg.rAFE_CONN2);
        mAfe->GetAfeReg(AFE_CONN3,&ftm_rec_reg.rAFE_CONN3);
        mAfe->GetAfeReg(AFE_UL_SRC_0,&ftm_rec_reg.rAFE_UL_SRC_0);

        /*
         * Digital's control sequence should be follwed.
         * (Error control sequence will make continuous noise.)
         */

        // power
        mAfe->SetAfeReg(AUDIO_TOP_CON0, 0x0, 0xffffffff);

        //Route A/D's CH1 to DL's CH1 and CH2
        if(digital_mic&DIGITAL_MIC1){
            mAfe->SetAfeReg(AFE_CONN1,1<<25,1<<25); //ADC_2 -> DAC_L
            mAfe->SetAfeReg(AFE_CONN2,1<<16,1<<16); //ADC_2 -> DAC_R
        }
        else{
            mAfe->SetAfeReg(AFE_CONN1,1<<24,1<<24); //ADC_1 -> DAC_L
            mAfe->SetAfeReg(AFE_CONN2,1<<15,1<<15); //ADC_1 -> DAC_R
        }

        mAfe->SetAfeReg(AFE_DAC_CON0,0x00000001,0x00000001);
        usleep(150);

        mAfe->SetAfeReg(AFE_DL_SRC2_2,0xffffffff,0xffffffff);//hardcode DL2 gain.
        mAfe->SetAfeReg(AFE_SDM_GAIN_STAGE,0x00007fde,0xffffffff);
        mAfe->SetAfeReg(AFE_DL_SDM_CON0,0x00000000,0xffffffff);
        mAfe->SetAfeReg(AFE_UL_AGC0,0x71c671c6,0xffffffff);//Set UL PGA min gain.

        mAfe->SetAfeReg(WR_PATH0,0x8020,0x8020);

#if defined(MTK_DIGITAL_MIC_SUPPORT)
        mAfe->SetAfeReg(AFE_UL_SRC_0,0x1be00402,0x3fe003A2);
#endif
        mAfe->SetAfeReg(AFE_UL_SRC_0,0xA0001,0x1E0001);

        mAfe->SetAfeReg(AFE_DL_SRC2_1,0x33000623,0xF3000623);
    }
    else
    {
        //Restore register values.
        mAfe->SetAfeReg(AFE_DL_SRC2_1,ftm_rec_reg.rAFE_DL_SRC2_1,0xffffffff);
        mAfe->SetAfeReg(AFE_DL_SRC2_2,ftm_rec_reg.rAFE_DL_SRC2_2,0xffffffff);
        mAfe->SetAfeReg(AFE_UL_AGC0,ftm_rec_reg.rAFE_UL_AGC0,0xffffffff);
        mAfe->SetAfeReg(AFE_SDM_GAIN_STAGE,ftm_rec_reg.rAFE_SDM_GAIN_STAGE,0xffffffff);
        mAfe->SetAfeReg(AFE_DL_SDM_CON0,ftm_rec_reg.rAFE_DL_SDM_CON0,0xffffffff);
        mAfe->SetAfeReg(AFE_CONN1,ftm_rec_reg.rAFE_CONN1,0xffffffff);
        mAfe->SetAfeReg(AFE_CONN2,ftm_rec_reg.rAFE_CONN2,0xffffffff);
        mAfe->SetAfeReg(AFE_CONN3,ftm_rec_reg.rAFE_CONN3,0xffffffff);
        mAfe->SetAfeReg(AFE_UL_SRC_0,ftm_rec_reg.rAFE_UL_SRC_0,0xffffffff);
        usleep(150);
        mAfe->SetAfeReg(AFE_DAC_CON0,ftm_rec_reg.rAFE_DAC_CON0,0xffffffff);
        mAfe->SetAfeReg(AUDIO_TOP_CON0,ftm_rec_reg.rAUDIO_TOP_CON0,0xffffffff);
    }
}

void AudioFtm::FTM_AFE_Loopback(bool bEnable)
{
   ALOGD("FTM_AFE_Loopback, bEnable:%d \n",bEnable);
   if(bEnable == true)
   {
      // for facotry receive loopback. No need to set IRQ interrupt
      mAfe->SetAfeReg(AFE_CONN1,(0<<27),(1<<27)); // Connect -- DL1_R to DAC_L, bit27: I06-O03
      mAfe->SetAfeReg(AFE_CONN2,(0<<17),(1<<17));   // Connect -- DL1_L to DAC_R, bit17: I05-O04

      mAfe->SetAfeReg(AFE_IRQ_CON,0x0,0x1);
      mAfe->SetAfeReg(AFE_IR_CLR,0x1,0x1);

      mAfe->SetAfeReg(AFE_CONN1,(1<<24),(1<<24));  // Connect -- ADC_L to DAC_L, bit24: I03-O03
      mAfe->SetAfeReg(AFE_CONN2,(1<<16),(1<<16));  // Connect -- ADC_R to DAC_R, bit16: I04-O04
      mAfe->SetAfeReg(AFE_DL_SRC2_1,0x0020,0x0020);  // bit5: DL_2_VOICE_MODE-- 0 for audio, 1 for voice
      mAfe->SetAfeReg(AFE_UL_SRC_1,(1<<31),(1<<31)); // bit31: C_SDM_RESET  1:reset , 0:no reset
   }
   else
   {
      mAfe->SetAfeReg(AFE_CONN1,(0<<24),(1<<24));  // DisConnect -- ADC_L to DAC_L, bit24: I03-O03
      mAfe->SetAfeReg(AFE_CONN2,(0<<16),(1<<16));  // DisConnect -- ADC_R to DAC_R, bit16: I04-O04
      mAfe->SetAfeReg(AFE_DL_SRC2_1,0x0000,0x0020);  // bit5: DL_2_VOICE_MODE-- 0 for audio, 1 for voice
      mAfe->SetAfeReg(AFE_UL_SRC_1,(0<<31),(1<<31)); // bit31: C_SDM_RESET  1:reset , 0:no reset
   }
}




void AudioFtm::Meta_Open_Analog(AnalogAFE_Mux pinmux)
{
   ALOGD("Meta_Open_Analog pinmux=%d \n",pinmux);
   //Request Analog clock before access analog hw
   mAnalog->AnalogAFE_Request_ANA_CLK();
   if(pinmux == AUDIO_PATH)
   {
      // Pin Mux enable for audio/voice
      mAnalog->SetAnaReg(AUDIO_CON5,0x0440,0x0FF0); // Change mux to audio/voice
      // Enable Audio Bias (normal)
      mAnalog->SetAnaReg(AUDIO_CON3,0x0000,0x0002);
      // Enable Audio DAC (L/R) and Voice DAC
      mAnalog->SetAnaReg(AUDIO_CON3,0x01f0,0x01f0);
      mAnalog->SetAnaReg(AUDIO_CON10,0x0020,0xffff); // Zero-Padding off
   }
   else if(pinmux == VOICE_PATH)
   {
      // Enable Voice DAC (R)
      mAnalog->SetAnaReg(AUDIO_CON3,0x0180,0x01f0);
      // Enable Voice Buffer
      mAnalog->SetAnaReg(AUDIO_CON3,0x0040,0x0040);
      mAnalog->SetAnaReg(AUDIO_CON10,0x0020,0xffff); // Zero-Padding off

      // Enable Audio Bias (normal)
      mAnalog->SetAnaReg(AUDIO_CON3,0x0000,0x0002);

      // Pin Mux enable for audio/voice
      mAnalog->SetAnaReg(AUDIO_CON5,0x4000,0xF000); // Change mux to audio/voice
   }
   else
   {
      ALOGD("Meta_Open_Analog none \n");
   }
   mAnalog->EnableHeadset (true);
   //Release Analog clock after access analog hw
   mAnalog->AnalogAFE_Release_ANA_CLK();

#ifdef  ENABLE_EXT_DAC
   mAudioHardware->mAudioCustomFunction->EXT_DAC_TurnOnSpeaker (EXT_DAC_I2S,AUDIO_EXP_DAC_BUF0);
   mAudioHardware->mAudioCustomFunction->EXT_DAC_TurnOnSpeaker (EXT_DAC_I2S,AUDIO_EXP_DAC_BUF1);
   mAudioHardware->mAudioCustomFunction->EXT_DAC_TurnOnSpeaker (EXT_DAC_LINEIN,AUDIO_EXP_DAC_BUF0);
   mAudioHardware->mAudioCustomFunction->EXT_DAC_TurnOnSpeaker (EXT_DAC_LINEIN,AUDIO_EXP_DAC_BUF1);
#endif
}

void AudioFtm::Meta_Close_Analog()
{
   ALOGD("Meta_Close_Analog \n");
   //Request Analog clock before access analog hw
   mAnalog->AnalogAFE_Request_ANA_CLK();
   mAnalog->EnableHeadset(false);
   mAnalog->SetAnaReg(AUDIO_CON3,0x0000,0x0180);   // disable DAC
   mAnalog->SetAnaReg(AUDIO_CON3,0x0000,0x0030);  // disable Audio Buffer
   //Release Analog clock after access analog hw
   mAnalog->AnalogAFE_Release_ANA_CLK();

#ifdef  ENABLE_EXT_DAC
   mAudioHardware->mAudioCustomFunction->EXT_DAC_TurnOffSpeaker (EXT_DAC_I2S,AUDIO_EXP_DAC_BUF0);
   mAudioHardware->mAudioCustomFunction->EXT_DAC_TurnOffSpeaker (EXT_DAC_I2S,AUDIO_EXP_DAC_BUF1);
   mAudioHardware->mAudioCustomFunction->EXT_DAC_TurnOffSpeaker (EXT_DAC_LINEIN,AUDIO_EXP_DAC_BUF0);
   mAudioHardware->mAudioCustomFunction->EXT_DAC_TurnOffSpeaker (EXT_DAC_LINEIN,AUDIO_EXP_DAC_BUF1);
#endif
}

void AudioFtm::FTM_Ana_Loopback(int bEnable)
{
    ALOGD("FTM_Ana_Loopback, bEnable:%d \n",bEnable);
    //Request Analog clock before access analog hw
    mAnalog->AnalogAFE_Request_ANA_CLK();
    if(bEnable)
    {
        //store original register values.
        mAnalog->GetAnaReg(AUDIO_CON0,&ftm_ana.rAUDIO_CON0);
        mAnalog->GetAnaReg(AUDIO_CON1,&ftm_ana.rAUDIO_CON1);
        mAnalog->GetAnaReg(AUDIO_CON2,&ftm_ana.rAUDIO_CON2);
        mAnalog->GetAnaReg(AUDIO_CON3,&ftm_ana.rAUDIO_CON3);
        mAnalog->GetAnaReg(AUDIO_CON4,&ftm_ana.rAUDIO_CON4);
        mAnalog->GetAnaReg(AUDIO_CON5,&ftm_ana.rAUDIO_CON5);
        mAnalog->GetAnaReg(AUDIO_CON6,&ftm_ana.rAUDIO_CON6);
        mAnalog->GetAnaReg(AUDIO_CON7,&ftm_ana.rAUDIO_CON7);
        mAnalog->GetAnaReg(AUDIO_CON8,&ftm_ana.rAUDIO_CON8);
        mAnalog->GetAnaReg(AUDIO_CON9,&ftm_ana.rAUDIO_CON9);
        mAnalog->GetAnaReg(AUDIO_CON10,&ftm_ana.rAUDIO_CON10);
        mAnalog->GetAnaReg(AUDIO_CON14,&ftm_ana.rAUDIO_CON14);
        mAnalog->GetAnaReg(AUDIO_CON17,&ftm_ana.rAUDIO_CON17);
        mAnalog->GetAnaReg(AUDIO_CON20,&ftm_ana.rAUDIO_CON20);
        mAnalog->GetAnaReg(AUDIO_CON21,&ftm_ana.rAUDIO_CON21);
        mAnalog->GetAnaReg(AUDIO_CON22,&ftm_ana.rAUDIO_CON22);
        mAnalog->GetAnaReg(AUDIO_CON23,&ftm_ana.rAUDIO_CON23);
        mAnalog->GetAnaReg(AUDIO_CON24,&ftm_ana.rAUDIO_CON24);
        mAnalog->GetAnaReg(AUDIO_CON28,&ftm_ana.rAUDIO_CON28);
        mAnalog->GetAnaReg(AUDIO_CON29,&ftm_ana.rAUDIO_CON29);
        mAnalog->GetAnaReg(AUDIO_CON30,&ftm_ana.rAUDIO_CON30);
        mAnalog->GetAnaReg(AUDIO_CON31,&ftm_ana.rAUDIO_CON31);
        mAnalog->GetAnaReg(AUDIO_CON32,&ftm_ana.rAUDIO_CON32);
        mAnalog->GetAnaReg(AUDIO_CON33,&ftm_ana.rAUDIO_CON33);
        mAnalog->GetAnaReg(AUDIO_CON34,&ftm_ana.rAUDIO_CON34);
        mAnalog->GetAnaReg(AUDIO_NCP0,&ftm_ana.rAUDIO_NCP0);
        mAnalog->GetAnaReg(AUDIO_NCP1,&ftm_ana.rAUDIO_NCP1);
        mAnalog->GetAnaReg(AUDIO_LDO0,&ftm_ana.rAUDIO_LDO0);
        mAnalog->GetAnaReg(AUDIO_LDO1,&ftm_ana.rAUDIO_LDO1);
        mAnalog->GetAnaReg(AUDIO_LDO2,&ftm_ana.rAUDIO_LDO2);
        mAnalog->GetAnaReg(AUDIO_GLB0,&ftm_ana.rAUDIO_GLB0);
        mAnalog->GetAnaReg(AUDIO_GLB1,&ftm_ana.rAUDIO_GLB1);
        mAnalog->GetAnaReg(AUDIO_REG1,&ftm_ana.rAUDIO_REG1);

        //For Uplink ABB PMU
        mAnalog->SetAnaReg(AUDIO_NCP0,0x102b,0xffff);
        mAnalog->SetAnaReg(AUDIO_NCP1,0x0600,0xffff);

        //mAnalog->SetAnaReg(AUDIO_NCP1,0x0000,0xffff);
        mAnalog->SetAnaReg(AUDIO_LDO0,0x1030,0xffff);
        mAnalog->SetAnaReg(AUDIO_LDO1,0x3010,0xffff);
        mAnalog->SetAnaReg(AUDIO_LDO2,0x0013,0xffff);
        mAnalog->SetAnaReg(AUDIO_GLB0,0x2920,0xffff);
        mAnalog->SetAnaReg(AUDIO_GLB1,0x0000,0xffff);
        mAnalog->SetAnaReg(AUDIO_REG1,0x0001,0xffff);
        mAnalog->SetAnaReg(AUDIO_CON0,0x4000,0xffff); //Set Line-in gain
        mAnalog->SetAnaReg(AUDIO_CON1,0x0c0c,0xffff); //Set HS/HPL gain
        mAnalog->SetAnaReg(AUDIO_CON2,0x000c,0xffff); //Set HPR gain
        mAnalog->SetAnaReg(AUDIO_CON3,0x01f0,0xffff); //Turn-on DAC and HP buffer
        mAnalog->SetAnaReg(AUDIO_CON5,0x4440,0xffff); //Set HS/HPL/HPR MUX
        mAnalog->SetAnaReg(AUDIO_CON6,0x0a44,0xffff); //Set buffer Ib/Iq current(thd=-91/-88dBc@16R/32R)
        mAnalog->SetAnaReg(AUDIO_CON7,0x003f,0xffff); //Set ZCD bias current
        mAnalog->SetAnaReg(AUDIO_CON8,0x0000,0xffff);
        mAnalog->SetAnaReg(AUDIO_CON14,0x00c0,0xffff); //set DAC ref. gen.
        mAnalog->SetAnaReg(AUDIO_CON17,0x0008,0xffff); //Enable HS pull-low
        mAnalog->SetAnaReg(AUDIO_CON10,0x01a1,0xffff); //scrambler enable
        mAnalog->SetAnaReg(AUDIO_CON9,0x0052,0xfffe);  //AFIFO enable
        usleep(1);
        mAnalog->SetAnaReg(AUDIO_CON9,0x0001,0x0001);

        //For uplink ABB
        mAnalog->SetAnaReg(AUDIO_CON28,0x0000,0xffff); //Audio ADC max current
        mAnalog->SetAnaReg(AUDIO_CON29,0x1154,0xffff); //Set RC tuning
        mAnalog->SetAnaReg(AUDIO_CON30,0x0154,0xffff); //Set RC tuning
        mAnalog->SetAnaReg(AUDIO_CON20,0x2525,0xffff); //PGA gain setting
        mAnalog->SetAnaReg(AUDIO_CON21,0x1200,0xffff); //PGA on/select
        //mAnalog->SetAnaReg(AUDIO_CON22,0x1100,0xffff); //PGA on/select
        //mAnalog->SetAnaReg(AUDIO_CON23,0xCC00,0xffff); //ADC on/select
        mAnalog->SetAnaReg(AUDIO_CON23,0xC000,0xffff); //ADC on/select
        mAnalog->SetAnaReg(AUDIO_CON34,0x8000,0xffff); //VMIC on
        mAnalog->SetAnaReg(AUDIO_CON31,0x8000,0xffff); //Encoder clock enable

#ifdef ENABLE_EXT_DAC
        mAudioHardware->mAudioCustomFunction->EXT_DAC_TurnOnSpeaker (EXT_DAC_LINEIN,AUDIO_EXP_DAC_BUF0);
        mAudioHardware->mAudioCustomFunction->EXT_DAC_TurnOnSpeaker (EXT_DAC_LINEIN,AUDIO_EXP_DAC_BUF1);
#endif
    }
    else
    {
        //Restore register values.
        mAnalog->SetAnaReg(AUDIO_CON9,ftm_ana.rAUDIO_CON9,0xffffffff);
        mAnalog->SetAnaReg(AUDIO_CON0,ftm_ana.rAUDIO_CON0,0xffffffff);
        mAnalog->SetAnaReg(AUDIO_CON1,ftm_ana.rAUDIO_CON1,0xffffffff);
        mAnalog->SetAnaReg(AUDIO_CON2,ftm_ana.rAUDIO_CON2,0xffffffff);
        mAnalog->SetAnaReg(AUDIO_CON3,ftm_ana.rAUDIO_CON3,0xffffffff);
        mAnalog->SetAnaReg(AUDIO_CON4,ftm_ana.rAUDIO_CON4,0xffffffff);
        mAnalog->SetAnaReg(AUDIO_CON5,ftm_ana.rAUDIO_CON5,0xffffffff);
        mAnalog->SetAnaReg(AUDIO_CON6,ftm_ana.rAUDIO_CON6,0xffffffff);
        mAnalog->SetAnaReg(AUDIO_CON7,ftm_ana.rAUDIO_CON7,0xffffffff);
        mAnalog->SetAnaReg(AUDIO_CON8,ftm_ana.rAUDIO_CON8,0xffffffff);
        mAnalog->SetAnaReg(AUDIO_CON10,ftm_ana.rAUDIO_CON10,0xffffffff);
        mAnalog->SetAnaReg(AUDIO_CON14,ftm_ana.rAUDIO_CON14,0xffffffff);
        mAnalog->SetAnaReg(AUDIO_CON17,ftm_ana.rAUDIO_CON17,0xffffffff);
        mAnalog->SetAnaReg(AUDIO_CON20,ftm_ana.rAUDIO_CON20,0xffffffff);
        mAnalog->SetAnaReg(AUDIO_CON21,ftm_ana.rAUDIO_CON21,0xffffffff);
        mAnalog->SetAnaReg(AUDIO_CON22,ftm_ana.rAUDIO_CON22,0xffffffff);
        mAnalog->SetAnaReg(AUDIO_CON23,ftm_ana.rAUDIO_CON23,0xffffffff);
        mAnalog->SetAnaReg(AUDIO_CON24,ftm_ana.rAUDIO_CON24,0xffffffff);
        mAnalog->SetAnaReg(AUDIO_CON28,ftm_ana.rAUDIO_CON28,0xffffffff);
        mAnalog->SetAnaReg(AUDIO_CON29,ftm_ana.rAUDIO_CON29,0xffffffff);
        mAnalog->SetAnaReg(AUDIO_CON30,ftm_ana.rAUDIO_CON30,0xffffffff);
        mAnalog->SetAnaReg(AUDIO_CON31,ftm_ana.rAUDIO_CON31,0xffffffff);
        mAnalog->SetAnaReg(AUDIO_CON32,ftm_ana.rAUDIO_CON32,0xffffffff);
        mAnalog->SetAnaReg(AUDIO_CON33,ftm_ana.rAUDIO_CON33,0xffffffff);
        mAnalog->SetAnaReg(AUDIO_CON34,ftm_ana.rAUDIO_CON34,0xffffffff);

        mAnalog->SetAnaReg(AUDIO_NCP0,ftm_ana.rAUDIO_NCP0,0xffffffff);
        mAnalog->SetAnaReg(AUDIO_NCP1,ftm_ana.rAUDIO_NCP1,0xffffffff);
        mAnalog->SetAnaReg(AUDIO_LDO0,ftm_ana.rAUDIO_LDO0,0xffffffff);
        mAnalog->SetAnaReg(AUDIO_LDO1,ftm_ana.rAUDIO_LDO1,0xffffffff);
        mAnalog->SetAnaReg(AUDIO_LDO2,ftm_ana.rAUDIO_LDO2,0xffffffff);
        mAnalog->SetAnaReg(AUDIO_GLB0,ftm_ana.rAUDIO_GLB0,0xffffffff);
        mAnalog->SetAnaReg(AUDIO_GLB1,ftm_ana.rAUDIO_GLB1,0xffffffff);
        mAnalog->SetAnaReg(AUDIO_REG1,ftm_ana.rAUDIO_REG1,0xffffffff);
#ifdef ENABLE_EXT_DAC
        mAudioHardware->mAudioCustomFunction->EXT_DAC_TurnOffSpeaker (EXT_DAC_LINEIN,AUDIO_EXP_DAC_BUF0);
        mAudioHardware->mAudioCustomFunction->EXT_DAC_TurnOffSpeaker (EXT_DAC_LINEIN,AUDIO_EXP_DAC_BUF1);
#endif
    }
    //Release Analog clock after access analog hw
    mAnalog->AnalogAFE_Release_ANA_CLK();
}


void AudioFtm::FTM_AnaLpk_on()
{
   ALOGD("FTM_AnaLpk_on \n");

   //Request Analog clock before access analog hw
   mAnalog->AnalogAFE_Request_ANA_CLK();

   mAnalog->GetAnaReg(AUDIO_CON0, &ftm_ana.rAUDIO_CON0);
   mAnalog->GetAnaReg(AUDIO_CON1, &ftm_ana.rAUDIO_CON1);
   mAnalog->GetAnaReg(AUDIO_CON2, &ftm_ana.rAUDIO_CON2);
   mAnalog->GetAnaReg(AUDIO_CON3, &ftm_ana.rAUDIO_CON3);
   mAnalog->GetAnaReg(AUDIO_CON4, &ftm_ana.rAUDIO_CON4);
   mAnalog->GetAnaReg(AUDIO_CON5, &ftm_ana.rAUDIO_CON5);
   mAnalog->GetAnaReg(AUDIO_CON6, &ftm_ana.rAUDIO_CON6);
   mAnalog->GetAnaReg(AUDIO_CON7, &ftm_ana.rAUDIO_CON7);
   mAnalog->GetAnaReg(AUDIO_CON8, &ftm_ana.rAUDIO_CON8);
   mAnalog->GetAnaReg(AUDIO_CON9, &ftm_ana.rAUDIO_CON9);
   mAnalog->GetAnaReg(AUDIO_CON10, &ftm_ana.rAUDIO_CON10);
   mAnalog->GetAnaReg(WR_PATH0, &ftm_ana.rWR_PATH0); // FIFO Clock Edge Control

   mAnalog->SetAnaReg(AUDIO_CON0,0x0000,0xffffffff);
   mAnalog->SetAnaReg(AUDIO_CON1,0x0C0C,0xffffffff);
   mAnalog->SetAnaReg(AUDIO_CON2,0x000C,0xffffffff);
   mAnalog->SetAnaReg(AUDIO_CON3,0x01B0,0xffffffff);
   mAnalog->SetAnaReg(AUDIO_CON4,0x1818,0xffffffff);
   mAnalog->SetAnaReg(AUDIO_CON5,0x0440,0xffffffff);
   mAnalog->SetAnaReg(AUDIO_CON6,0x0533,0xffffffff);
   mAnalog->SetAnaReg(AUDIO_CON7,0x003f,0xffffffff);
   mAnalog->SetAnaReg(AUDIO_CON8,0x0000,0xffffffff);
   mAnalog->SetAnaReg(AUDIO_CON9,0x0052,0xfffffffe);
   mAnalog->SetAnaReg(AUDIO_CON14,0x0040,0xffffffff);
   mAnalog->SetAnaReg(AUDIO_CON17,0x0000,0xffffffff);
   mAnalog->SetAnaReg(AUDIO_CON9,0x0001,0x00000001);
   mAnalog->SetAnaReg(AUDIO_CON10,0x01a1,0xffffffff);
   mAnalog->SetAnaReg(WR_PATH0,0x8020,0x8020); // FIFO Clock Edge Control

   mAnalog->SetAnaReg(AUDIO_NCP0,0x102B,0xffff);
   mAnalog->SetAnaReg(AUDIO_NCP1,0x0600,0xffff);
   mAnalog->SetAnaReg(AUDIO_LDO0,0x1030,0xffff);
   mAnalog->SetAnaReg(AUDIO_LDO1,0x3010,0xffff);
   mAnalog->SetAnaReg(AUDIO_LDO2,0x0013,0xffff);
   mAnalog->SetAnaReg(AUDIO_GLB0,0x2920,0xffff);
   mAnalog->SetAnaReg(AUDIO_GLB1,0x0000,0xffff);
   mAnalog->SetAnaReg(AUDIO_REG1,0x0001,0xffff);

   //Release Analog clock after access analog hw
   mAnalog->AnalogAFE_Release_ANA_CLK();

#ifdef ENABLE_EXT_DAC
    mAudioHardware->mAudioCustomFunction->EXT_DAC_TurnOnSpeaker (EXT_DAC_I2S,AUDIO_EXP_DAC_BUF0);
    mAudioHardware->mAudioCustomFunction->EXT_DAC_TurnOnSpeaker (EXT_DAC_I2S,AUDIO_EXP_DAC_BUF1);
#endif

}

void AudioFtm::FTM_AnaLpk_off()
{
   ALOGD("FTM_AnaLpk_off \n");
/*
   SetAnaReg(AUDIO_CON0,ftm_ana.rAUDIO_CON0,MASK_ALL);
   SetAnaReg(AUDIO_CON1,ftm_ana.rAUDIO_CON1,MASK_ALL);
   SetAnaReg(AUDIO_CON2,ftm_ana.rAUDIO_CON2,MASK_ALL);
   SetAnaReg(AUDIO_CON3,ftm_ana.rAUDIO_CON3,MASK_ALL);
   SetAnaReg(AUDIO_CON4,ftm_ana.rAUDIO_CON4,MASK_ALL);
   SetAnaReg(AUDIO_CON5,ftm_ana.rAUDIO_CON5,MASK_ALL);
   SetAnaReg(AUDIO_CON6,ftm_ana.rAUDIO_CON6,MASK_ALL);
   SetAnaReg(AUDIO_CON7,ftm_ana.rAUDIO_CON7,MASK_ALL);
   SetAnaReg(AUDIO_CON8,ftm_ana.rAUDIO_CON8,MASK_ALL);
   SetAnaReg(AUDIO_CON9,ftm_ana.rAUDIO_CON9,MASK_ALL);
   SetAnaReg(AUDIO_CON10,ftm_ana.rAUDIO_CON10,MASK_ALL);
*/
   //Request Analog clock before access analog hw
   mAnalog->AnalogAFE_Request_ANA_CLK();

   // Init Analog Register
   mAnalog->SetAnaReg(AUDIO_CON0,0x0000,0xffff);
   mAnalog->SetAnaReg(AUDIO_CON1,0x000C,0xffff);
   mAnalog->SetAnaReg(AUDIO_CON2,0x000C,0xffff);
   mAnalog->SetAnaReg(AUDIO_CON4,0x1818,0xffff);
   mAnalog->SetAnaReg(AUDIO_CON5,0x0440,0xffff);  // select Audio path
   mAnalog->SetAnaReg(AUDIO_CON6,0x001B,0xffff);
   mAnalog->SetAnaReg(AUDIO_CON7,0x0400,0xffff);  //DRTZ 0; disable HP amp startup mode
   mAnalog->SetAnaReg(AUDIO_CON8,0x7000,0xffff);  //Disable short circuit protection
   mAnalog->SetAnaReg(AUDIO_CON9,0x0002,0xfffe);
   usleep(1);
   mAnalog->SetAnaReg(AUDIO_CON9,0x0001,0x0001);
   mAnalog->SetAnaReg(AUDIO_CON10,0x0020,0xffff); // Zero-Padding off

   //Release Analog clock after access analog hw
   mAnalog->AnalogAFE_Release_ANA_CLK();

#ifdef ENABLE_EXT_DAC
    mAudioHardware->mAudioCustomFunction->EXT_DAC_TurnOffSpeaker (EXT_DAC_I2S,AUDIO_EXP_DAC_BUF0);
    mAudioHardware->mAudioCustomFunction->EXT_DAC_TurnOffSpeaker (EXT_DAC_I2S,AUDIO_EXP_DAC_BUF1);
#endif
}

/// ----------------------------------------------------------------------------
/// ---> Loopbacks start
void AudioFtm::Audio_Set_Speaker_Vol(int level)
{
    ALOGD("Audio_Set_Speaker_Vol with level = %d",level);
    if(mFd>0)
    {
        ::ioctl(mFd,SET_SPEAKER_VOL,level);// init speaker volume level 0
    }
    else
    {
        ALOGD("Set_Speaker_Vol with mFd not init\n");
    }
}

void AudioFtm::Audio_Set_Speaker_On(int Channel)
{
    ALOGD("Audio_Set_Speaker_On Channel = %d\n",Channel);
    if (mFd>0)
    {
        ::ioctl(mFd,SET_SPEAKER_ON,Channel);// enable speaker
    }
    else
    {
        ALOGD("Set_Speaker_On with mFd not init\n");
    }
}

void AudioFtm::Audio_Set_Speaker_Off(int Channel)
{
    ALOGD("Audio_Set_Speaker_Off Channel = %d\n",Channel);
    if (mFd>0)
    {
        ::ioctl(mFd,SET_SPEAKER_OFF,Channel);// disable speaker
    }
    else
    {
        ALOGD("Set_Speaker_OFF with mFd not init\n");
    }
}

void AudioFtm::Audio_Set_HeadPhone_On(int Channel)
{
    ALOGD("Audio_Set_HeadPhone_On Channel = %d\n",Channel);
    if (mFd>0)
    {
        ::ioctl(mFd,SET_HEADPHONE_ON,Channel);// enable headphone
    }
    else
    {
        ALOGD("Audio_Set_HeadPhone_On with mFd not init\n");
    }
}

void AudioFtm::Audio_Set_HeadPhone_Off(int Channel)
{
    ALOGD("Audio_Set_HeadPhone_Off Channel = %d\n",Channel);
    if (mFd>0)
    {
        ::ioctl(mFd,SET_HEADPHONE_OFF,Channel);// disable headphone
    }
    else
    {
        ALOGD("Audio_Set_HeadPhone_Off with mFd not init\n");
    }
}

void AudioFtm::Audio_Set_Earpiece_On()
{
    ALOGD("Audio_Set_Earpiece_On Channel\n");
    if (mFd>0)
    {
        ::ioctl(mFd,SET_EARPIECE_ON,Channel_Stereo);// enable earpiece
    }
    else
    {
        ALOGD("Audio_Set_Earpiece_On with mFd not init\n");
    }
}

void AudioFtm::Audio_Set_Earpiece_Off()
{
    ALOGD("Audio_Set_Earpiece_Off Channel\n");
    if (mFd>0)
    {
        ::ioctl(mFd,SET_EARPIECE_OFF,Channel_Stereo);// disable earpiece
    }
    else
    {
        ALOGD("Audio_Set_Earpiece_Off with mFd not init\n");
    }
}

void AudioFtm::Aud_Loopback(char b_phone_mic_sel,
            char b_headset_mic_in,
            char b_spk_out,
            char b_receiver_out,
            char b_headset_out)
{
    ALOGD("!!! Aud_Loopback phone_mic=%d, headset_mic=%d, Spk=%d, Receiver=%d, headset=%d"
                ,b_phone_mic_sel,b_headset_mic_in,b_spk_out,b_receiver_out,b_headset_out);
    usleep(200*1000);
    //Request Analog clock before access analog hw
    mAnalog->AnalogAFE_Request_ANA_CLK();
    if ((b_phone_mic_sel==MIC1_ON) || (b_phone_mic_sel==MIC2_ON))
    {
#if defined(MTK_DIGITAL_MIC_SUPPORT)
        FTM_AFE_UL_Loopback(true, (b_phone_mic_sel==MIC1_ON) ? DIGITAL_MIC1 : DIGITAL_MIC2);
#else
        FTM_AFE_UL_Loopback(true, ANALOG_MIC);
#endif

        mAfe->SetAfeReg(AFE_DL_SRC2_1,0x18,0x18);
        usleep(50000);

        FTM_Ana_Loopback(true);

        if(b_receiver_out == true) //set HS Buffer
        {
            mAnalog->SetAnaReg(AUDIO_CON1,0x0700,0x3f00);//set HPL Gain
            mAnalog->SetAnaReg(AUDIO_CON1,0x000c,0x003f);//set HS Gain
            mAnalog->SetAnaReg(AUDIO_CON3,0x1e0,0x1e0);//turn on DAC, HS Buffer
        }
        else
        {
            mAnalog->SetAnaReg(AUDIO_CON3,0x0,0x40);//turn off HS Buffer
        }

        if(b_headset_out == true) //set HP Buffer
        {
            mAnalog->SetAnaReg(AUDIO_CON1,0x000c,0x003f);//set HPL Gain
            mAnalog->SetAnaReg(AUDIO_CON2,0x000c,0x003f);//set HPR Gain
            mAnalog->SetAnaReg(AUDIO_CON3,0x1b0,0x1b0);//turn on DAC, HP Buffer
        }
        else
        {
            if(b_receiver_out == true)
              mAnalog->SetAnaReg(AUDIO_CON3,0x0,0x10);//turn off HP-R Buffer
            else
              mAnalog->SetAnaReg(AUDIO_CON3,0x0,0x30);//turn off HP-R/L Buffer
        }

        if(b_spk_out == true) //set speaker
        {
            mAnalog->SetAnaReg(AUDIO_CON1,0x000c,0x003f);//set HPL Gain
            mAnalog->SetAnaReg(AUDIO_CON2,0x000c,0x003f);//set HPR Gain
            mAnalog->SetAnaReg(AUDIO_CON3,0x1b0,0x1b0);//turn on DAC, HP Buffer
           // Audio_Set_Speaker_On(Channel_Stereo);
        }
        else
        {
            if(b_receiver_out == true){
              mAnalog->SetAnaReg(AUDIO_CON3,0x0,0x10);//turn off HP-R Buffer
            }
            else if(b_headset_out != true) {
              mAnalog->SetAnaReg(AUDIO_CON3,0x0,0x30);//turn off HP-R/L Buffer
            }
            Audio_Set_Speaker_Off(Channel_Stereo);
        }

        if(b_receiver_out == true)//set receiver
        {
           // Audio_Set_Earpiece_On();
			 Audio_Set_Speaker_On(Channel_Stereo);
        }
        else
        {
            Audio_Set_Earpiece_Off();
        }

        if(b_headset_out == true)//set headset
        {
            Audio_Set_HeadPhone_On(Channel_Stereo);
        }
        else
        {
            Audio_Set_HeadPhone_Off(Channel_Stereo);
        }

        if(b_headset_mic_in == true) //set PreAMP
        {
            mAnalog->SetAnaReg(AUDIO_CON21,0x1300,0xffff); //PreAMP on/select AIN3
        }
        else if (b_phone_mic_sel == MIC1_ON)
        {
            mAnalog->SetAnaReg(AUDIO_CON21,0x1100,0xffff); //PreAMP on/select AIN1
#if defined(MTK_DIGITAL_MIC_SUPPORT)
            mAnalog->SetAnaReg(AUDIO_CON22,0x1100,0xffff); //PreAMP on /select AIN1
            mAnalog->SetAnaReg(AUDIO_CON23,0x4400,0xffff); //select preamp input
            mAnalog->SetAnaReg(AUDIO_CON33,0x0042,0xffff);
            mAnalog->SetAnaReg(AUDIO_CON34,0x8000,0xffff);
            mAnalog->SetAnaReg(AUDIO_CON24,0x0001,0xffff);
#endif
        }
        else if (b_phone_mic_sel == MIC2_ON)
        {
            mAnalog->SetAnaReg(AUDIO_CON21,0x1200,0xffff); //PreAMP on/select AIN2
#if defined(MTK_DIGITAL_MIC_SUPPORT)
            mAnalog->SetAnaReg(AUDIO_CON22,0x1200,0xffff); //PreAMP off/select AIN2
            mAnalog->SetAnaReg(AUDIO_CON23,0x4400,0xffff); //select preamp input
            mAnalog->SetAnaReg(AUDIO_CON33,0x0042,0xffff);
            mAnalog->SetAnaReg(AUDIO_CON34,0x8000,0xffff);
            mAnalog->SetAnaReg(AUDIO_CON24,0x0001,0xffff);
#endif
        }
        usleep(50000);
        mAfe->SetAfeReg(AFE_DL_SRC2_1,0x0,0x18);
    }
    else
    {
        mAfe->SetAfeReg(AFE_DL_SRC2_1,0x18,0x18);
        usleep(50000);
        FTM_Ana_Loopback(false);
        usleep(50000);
        mAfe->SetAfeReg(AFE_DL_SRC2_1,0x0,0x18);

        FTM_AFE_UL_Loopback(false);
        Audio_Set_Earpiece_Off();
        Audio_Set_HeadPhone_Off(Channel_Stereo);
        Audio_Set_Speaker_Off(Channel_Stereo);
    }
    //Release Analog clock after access analog hw
    mAnalog->AnalogAFE_Release_ANA_CLK();
}

int AudioFtm::PhoneMic_Receiver_Loopback(char echoflag)
{
    ALOGD("PhoneMic_Receiver_Loopback echoflag=%d",echoflag);

    //Path: Phone Mic -> Receiver
    char b_phone_mic_sel  = echoflag;
    char b_headset_mic_in = false;
    char b_spk_out        = true;//false;
    char b_receiver_out   = true;
    char b_headset_out    = false;

    Aud_Loopback(b_phone_mic_sel, b_headset_mic_in, b_spk_out, b_receiver_out, b_headset_out);

    return true;
}

int AudioFtm::PhoneMic_EarphoneLR_Loopback(char echoflag)
{
    ALOGD("PhoneMic_EarphoneLR_Loopback echoflag=%d",echoflag);

    //Path: Phone Mic -> Earphone
    char b_phone_mic_sel  = echoflag;
    char b_headset_mic_in = false;
    char b_spk_out        = false;
    char b_receiver_out   = false;
    char b_headset_out    = true;

    Aud_Loopback(b_phone_mic_sel, b_headset_mic_in, b_spk_out, b_receiver_out, b_headset_out);

    return true;
}


int AudioFtm::PhoneMic_SpkLR_Loopback(char echoflag)
{
    ALOGD("PhoneMic_SpkLR_Loopback echoflag=%d",echoflag);

    //Path: Phone Mic -> Speaker
    char b_phone_mic_sel  = echoflag;
    char b_headset_mic_in = false;
    char b_spk_out        = true;
    char b_receiver_out   = false;
    char b_headset_out    = false;

    Aud_Loopback(b_phone_mic_sel, b_headset_mic_in, b_spk_out, b_receiver_out, b_headset_out);

    return true;
}

int AudioFtm::HeadsetMic_EarphoneLR_Loopback(char bEnable, char bHeadsetMic)
{
    ALOGD("HeadsetMic_EarphoneLR_Loopback, bEnable=%d",bEnable);

    //Path: Headset Mic -> Headphone
    char b_phone_mic_sel  = bEnable;
    char b_headset_mic_in = bHeadsetMic;
    char b_spk_out        = true;//false;
    char b_receiver_out   = false;
    char b_headset_out    = true;

    Aud_Loopback(b_phone_mic_sel, b_headset_mic_in, b_spk_out, b_receiver_out, b_headset_out);

    return true;
}

int AudioFtm::HeadsetMic_SpkLR_Loopback(char echoflag)
{
    ALOGD("HeadsetMic_SpkLR_Loopback echoflag=%d",echoflag);

    //Path: Headset Mic -> Speaker
    char b_phone_mic_sel  = echoflag;
    char b_headset_mic_in = true;
    char b_spk_out        = true;
    char b_receiver_out   = false;
    char b_headset_out    = false;

    Aud_Loopback(b_phone_mic_sel, b_headset_mic_in, b_spk_out, b_receiver_out, b_headset_out);

    return true;
}


bool AudioFtm::isMdLoopback(ENUM_LOOPBACK_TYPES eLoopbackType)
{
    bool retval;

    switch(eLoopbackType)
    {
        case MD_MAIN_MIC_AFE_LOOPBACK:
        case MD_MAIN_MIC_ACOUSTIC_LOOPBACK:
        case MD_HEADSET_MIC_AFE_LOOPBACK:
        case MD_HEADSET_MIC_ACOUSTIC_LOOPBACK:
        case MD_DUAL_MIC_AFE_LOOPBACK:
        case MD_DUAL_MIC_ACOUSTIC_LOOPBACK_WITHOUT_DMNR:
        case MD_DUAL_MIC_ACOUSTIC_LOOPBACK_WITH_DMNR:
        case MD_BT_LOOPBACK:
            retval = true;
            break;
        default:
            retval = false;
            break;
    }
    return retval;
}

static bool bCurrentUseLoudSpeaker = false;

bool AudioFtm::SetLoopbackOn(ENUM_LOOPBACK_TYPES eLoopbackType, bool bEarphoneInserted)
{
    ALOGD("+SetLoopbackOn(%d, %d)", eLoopbackType, bEarphoneInserted);

    // check no loobpack function on
    pthread_mutex_lock_timeout_np(&mLoopbackMutex, 1000); // 1s
    if (eCurrentLoopbackType != NO_LOOPBACK)
    {
        ALOGW("Please Turn off Loopback Type %d First!!", eCurrentLoopbackType);
        pthread_mutex_unlock(&mLoopbackMutex);
        return false;
    }

    bool bMdLoopback = isMdLoopback(eLoopbackType);
    LadOutPutDevice_Line eOutputDevice;
    if (bMdLoopback == true)
    {
        // check mLad
        if (mLad == NULL)
        {
            ALOGW("mLad == NULL!! return");
            pthread_mutex_unlock(&mLoopbackMutex);
            return false;
        }

        // check EM data first
        mLad->pCCCI->CheckMDSideSphCoefArrival();

        // set output device by bEarphoneInserted
        if (bEarphoneInserted == true) {  // earphone
            eOutputDevice = LADOUT_SPEAKER2;
        }
        else if (mIsLoopbackDefaultUseLoudSpeaker == true) {  // loud speaker
            bCurrentUseLoudSpeaker = true;
#if defined(ENABLE_EXT_DAC)
            eOutputDevice = LADOUT_SPEAKER1;
#else
            eOutputDevice = LADOUT_LOUDSPEAKER;
#endif
        }
        else {  // receiver
            eOutputDevice = LADOUT_SPEAKER1;
        }

        // let both earphone & speaker L/R channel have sound
        bConnectAllADDA = (bEarphoneInserted || bCurrentUseLoudSpeaker) ? true : false;

        // mute
        mLad->LAD_MuteSpeaker(true);
    }

    // turn on audio clock
    ::ioctl(mFd, AUD_SET_ADC_CLOCK, true);

    // Enable loopback function
    switch(eLoopbackType)
    {
        case AP_MAIN_MIC_AFE_LOOPBACK:
            if (bEarphoneInserted == true) {
                PhoneMic_EarphoneLR_Loopback(MIC1_ON);
            }
            else if (mIsLoopbackDefaultUseLoudSpeaker == true) {
                PhoneMic_SpkLR_Loopback(MIC1_ON);
            }
            else {
                PhoneMic_Receiver_Loopback(MIC1_ON);
            }
            break;
        case AP_HEADSET_MIC_AFE_LOOPBACK:
            HeadsetMic_EarphoneLR_Loopback(true, true);
            break;
#if defined(MTK_DUAL_MIC_SUPPORT)
        case AP_REF_MIC_AFE_LOOPBACK:
            if (bEarphoneInserted == true) {
                PhoneMic_EarphoneLR_Loopback(MIC2_ON);
            }
            else if (mIsLoopbackDefaultUseLoudSpeaker == true) {
                PhoneMic_SpkLR_Loopback(MIC2_ON);
            }
            else {
                PhoneMic_Receiver_Loopback(MIC2_ON);
            }
            break;
#endif
        case MD_MAIN_MIC_AFE_LOOPBACK:
            mLad->LAD_ForceSetInputSource(LADIN_Microphone1);
            mLad->LAD_SetOutputDevice(eOutputDevice);
            mLad->LAD_SetAfeLoopback(true, bSwapADDA, bConnectAllADDA);
            break;
        case MD_MAIN_MIC_ACOUSTIC_LOOPBACK:
            mLad->LAD_ForceSetInputSource(LADIN_Microphone1);
            mLad->LAD_SetOutputDevice(eOutputDevice);
            mLad->LAD_SetSpeechMode((bCurrentUseLoudSpeaker == true) ? SPH_MODE_LOUDSPK : SPH_MODE_NORMAL);
            mLad->LAD_SetAcousticLoopback(true, bUseDMNR);
            break;
        case MD_HEADSET_MIC_AFE_LOOPBACK:
            mLad->LAD_ForceSetInputSource(LADIN_Microphone2);
            mLad->LAD_SetOutputDevice(eOutputDevice);
            mLad->LAD_SetAfeLoopback(true, bSwapADDA, bConnectAllADDA);
            break;
        case MD_HEADSET_MIC_ACOUSTIC_LOOPBACK:
            mLad->LAD_ForceSetInputSource(LADIN_Microphone2);
            mLad->LAD_SetOutputDevice(eOutputDevice);
            mLad->LAD_SetSpeechMode(SPH_MODE_EARPHONE);
            mLad->LAD_SetAcousticLoopback(true, bUseDMNR);
            break;
#if defined(MTK_DUAL_MIC_SUPPORT)
        case MD_DUAL_MIC_AFE_LOOPBACK:
            bConnectAllADDA = true;
            mLad->LAD_ForceSetInputSource(LADIN_DualAnalogMic);
            mLad->LAD_SetOutputDevice(eOutputDevice);
            mLad->LAD_SetAfeLoopback(true, bSwapADDA, bConnectAllADDA);
            break;
        case MD_DUAL_MIC_ACOUSTIC_LOOPBACK_WITHOUT_DMNR:
            bUseDMNR = false;
            mLad->LAD_ForceSetInputSource(LADIN_DualAnalogMic);
            mLad->LAD_SetOutputDevice(eOutputDevice);
            mLad->LAD_SetSpeechMode((bCurrentUseLoudSpeaker == true) ? SPH_MODE_LOUDSPK : SPH_MODE_NORMAL);
            mLad->LAD_SetAcousticLoopback(true, bUseDMNR);
            break;
        case MD_DUAL_MIC_ACOUSTIC_LOOPBACK_WITH_DMNR:
            bUseDMNR = true;
            mLad->LAD_ForceSetInputSource(LADIN_DualAnalogMic);
            mLad->LAD_SetOutputDevice(eOutputDevice);
            mLad->LAD_SetSpeechMode((bCurrentUseLoudSpeaker == true) ? SPH_MODE_LOUDSPK : SPH_MODE_NORMAL);
            mLad->LAD_SetAcousticLoopback(true, bUseDMNR);
            break;
#endif
        case MD_BT_LOOPBACK:
            acquire_wake_lock(PARTIAL_WAKE_LOCK, BLUETOOTH_LOOPBACK_WAKELOCK_NAME);
            mLad->LAD_SetSpeechMode(SPH_MODE_BT_EARPHONE);
            mLad->LAD_Set_Speech_Enhancement(false);
            mLad->LAD_SetAcousticLoopback(true, false);
            break;
        default:
            ALOGW("-SetLoopbackOn(%d) : No such Loopback type", eLoopbackType);
            ::ioctl(mFd, AUD_SET_ADC_CLOCK, false);
            pthread_mutex_unlock(&mLoopbackMutex);
            return false;
    }

    if (bMdLoopback == true) {
        // turn on amp for laud speaker
        if (bCurrentUseLoudSpeaker == true) {
            Audio_Set_Speaker_On(Channel_Stereo);
        }

        // set gain
        mLad->LAD_SetOutputVolume(0xFFFFE9);     // 1 db range(-32~12)
        mLad->LAD_SetMicrophoneVolume(0xFFFFCC); // the speech gain
        mLad->LAD_SetSidetoneVolume(0xFFF6AD);   // the lowest gain

        // unmute
        usleep(50000);
        mLad->LAD_MuteSpeaker(false);
    }

    eCurrentLoopbackType = eLoopbackType;
    pthread_mutex_unlock(&mLoopbackMutex);

    ALOGD("-SetLoopbackOn(%d, %d)", eLoopbackType, bEarphoneInserted);

    return true;
}

bool AudioFtm::SetLoopbackOff()
{
    ALOGD("+SetLoopbackOff(), eCurrentLoopbackType = %d", eCurrentLoopbackType);

    pthread_mutex_lock_timeout_np(&mLoopbackMutex, 1000); // 1000 ms

    if (eCurrentLoopbackType == NO_LOOPBACK)
    {
        ALOGD("-SetLoopbackOff() : No looback to be closed");
        pthread_mutex_unlock(&mLoopbackMutex);
        return true;
    }

    bool bMdLoopback = isMdLoopback(eCurrentLoopbackType);

    switch(eCurrentLoopbackType)
    {
        case AP_MAIN_MIC_AFE_LOOPBACK:
        case AP_HEADSET_MIC_AFE_LOOPBACK:
        case AP_REF_MIC_AFE_LOOPBACK:
            Aud_Loopback(false, false, false, false, false);
            ::ioctl(mFd, AUD_SET_ADC_CLOCK, false);
            break;
        case MD_MAIN_MIC_AFE_LOOPBACK:
        case MD_HEADSET_MIC_AFE_LOOPBACK:
        case MD_DUAL_MIC_AFE_LOOPBACK:
            mLad->LAD_SetAfeLoopback(false, bSwapADDA, bConnectAllADDA);
            break;
        case MD_MAIN_MIC_ACOUSTIC_LOOPBACK:
        case MD_HEADSET_MIC_ACOUSTIC_LOOPBACK:
        case MD_DUAL_MIC_ACOUSTIC_LOOPBACK_WITHOUT_DMNR:
        case MD_DUAL_MIC_ACOUSTIC_LOOPBACK_WITH_DMNR:
            mLad->LAD_SetAcousticLoopback(false, bUseDMNR);
            break;
        case MD_BT_LOOPBACK:
            mLad->LAD_SetAcousticLoopback(false, false);
            mLad->LAD_Set_Speech_Enhancement(true);
            mLad->LAD_SetSpeechMode(SPH_MODE_NORMAL);
            release_wake_lock(BLUETOOTH_LOOPBACK_WAKELOCK_NAME);
            break;
    }

    if (bMdLoopback == true) {
        // turn off amp for laud speaker
        if (bCurrentUseLoudSpeaker) {
            Audio_Set_Speaker_Off(Channel_Stereo);
            mLad->LAD_SetOutputDevice(LADOUT_SPEAKER1); 
            bCurrentUseLoudSpeaker = false;
        }
    }

    // clean
    eCurrentLoopbackType = NO_LOOPBACK;
    bSwapADDA = bConnectAllADDA = bUseDMNR = false;

    pthread_mutex_unlock(&mLoopbackMutex);

    ALOGD("-SetLoopbackOff()");

    return true;
}

/// <--- Loopbacks end
/// ----------------------------------------------------------------------------

}

