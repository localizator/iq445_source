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
 * Copyright (C) 2011 The Android Open Source Project
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

#ifndef ANDROID_AUDIO_HW_HAL_I2S_H
#define ANDROID_AUDIO_HW_HAL_I2S_H

//#include "audio_hw_I2S_hal.h"
#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sched.h>
#include <fcntl.h>
#include <time.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <utils/threads.h>
#include <AudioI2S.h>
#include <audio_hw_I2S_hal.h>

namespace android{


// wrapper for android::AudioI2S
AudioI2S* I2S_wp_getInstance()
{
    return AudioI2S::getInstance ();
}

void I2S_wp_Free_I2S_Instance()
{
    return AudioI2S::freeInstance ();
}

int32_t I2S_wp_Audio_GetI2SCapability()
{
    return AudioI2S::GetI2SCapability();
}

int I2S_wp_Audio_GetState(AudioI2S* handle)
{
    return handle->GetState ();
}

int32_t I2S_wp_Audio_GetAudioHandle(AudioI2S* handle)
{
    return handle->GetAudioHandle ();
}

uint32_t I2S_wp_Audio_samplerate(AudioI2S* handle)
{
    return handle->samplerate ();
}

int32_t I2S_wp_Audio_GetSourceType(AudioI2S* handle)
{
    return handle->GetSourceType ();
}

uint32_t I2S_wp_Audio_GetReadBufferSize(AudioI2S* handle)
{
    return handle->GetReadBufferSize ();
}

I2SClient* I2S_wp_Audio_GetI2SClient(AudioI2S* handle,int index)
{
    return handle->GetI2SClient (index);
}

bool I2S_wp_Audio_FindFreeClient(AudioI2S* handle,int32_t* ClientIndex)
{
    return handle->FindFreeClient (ClientIndex);
}

bool I2S_wp_Audio_FindValidClient(AudioI2S* handle,uint32_t Identity)
{
    return handle->FindValidClient (Identity);
}

uint32_t I2S_wp_Audio_open(AudioI2S* handle)
{
    return handle->open();
}

bool I2S_wp_Audio_close(AudioI2S* handle,uint32_t Identity)
{
    return handle->close (Identity);
}

// weh nstart with I2STYPE ,should call set and then start
bool I2S_wp_Audio_start(AudioI2S* handle,uint32_t Identity ,I2STYPE Type,uint32_t SampleRate)
{
     return handle->start(Identity,Type,SampleRate);
}

bool I2S_wp_Audio_stop(AudioI2S* handle,uint32_t Identity ,I2STYPE Type)
{
    return handle->stop(Identity,Type);
}

unsigned int I2S_wp_Audio_read(AudioI2S* handle,uint32_t Identity,void* buffer, uint32_t buffersize)
{
    return handle->read(Identity,buffer,buffersize);
}

bool I2S_wp_Audio_set(AudioI2S* handle,uint32_t TYPE, int32_t samplerate)
{
    return handle->set(TYPE, samplerate);
}

// state of I2S
bool I2S_wp_Audio_StateInIdle(AudioI2S* handle)
{
    return handle->StateInIdle();
}

bool I2S_wp_Audio_StateInI2S0InputStart(AudioI2S* handle)
{
    return handle->StateInI2S0InputStart();
}

bool I2S_wp_Audio_StateInI2S0OutputStart(AudioI2S* handle)
{
    return handle->StateInI2S0OutputStart();
}

bool I2S_wp_Audio_StateInI2S1OutputStart(AudioI2S* handle )
{
    return handle->StateInI2S1OutputStart();
}

bool I2S_wp_Audio_StateChecking(AudioI2S* handle,uint32_t state)
{
    return handle->StateChecking(state);
}

}

#endif

