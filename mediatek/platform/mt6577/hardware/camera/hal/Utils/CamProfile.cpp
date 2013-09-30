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

#define LOG_TAG "MtkCam/MtkCamUtils"
//
#include <utils/String8.h>
//
#include "inc/_CamLog.h"
#include "inc/_CamProfile.h"
using namespace android;
using namespace MtkCamUtils;
//


/******************************************************************************
*
*******************************************************************************/
#define MY_LOGV(fmt, arg...)    CAM_LOGV("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)    CAM_LOGD("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)    CAM_LOGI("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)    CAM_LOGW("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)    CAM_LOGE("[%s] "fmt, __FUNCTION__, ##arg)


/******************************************************************************
*
*******************************************************************************/
namespace android {
namespace MtkCamUtils {


/******************************************************************************
*
*******************************************************************************/
bool
CamProfile::
print(char const*const fmt /*= ""*/, ...) const
{
    if  ( ! mfgIsProfile )
    {
        return  false;
    }

    int32_t const i4EndUs = getUs();
    if  (0==mIdx)
    {
        va_list args;
        va_start(args, fmt);
        String8 result(String8::formatV(fmt, args));
        va_end(args);
        //
        CAM_LOGI(
            "{CamProfile}[%s::%s] %s: "
            "(%d-th) ===> [start-->now: %d ms]", 
            mpszClassName, mpszFuncName, result.string(), mIdx++, 
            (i4EndUs-mi4StartUs)/1000
        );
    }
    else
    {
        va_list args;
        va_start(args, fmt);
        String8 result(String8::formatV(fmt, args));
        va_end(args);
        //
        CAM_LOGI(
            "{CamProfile}[%s::%s] %s: "
            "(%d-th) ===> [start-->now: %d ms] [last-->now: %d ms]", 
            mpszClassName, mpszFuncName, result.string(), mIdx++, 
            (i4EndUs-mi4StartUs)/1000, (i4EndUs-mi4LastUs)/1000
        );
    }
    mi4LastUs = i4EndUs;
    return  true;
}


/******************************************************************************
*
*******************************************************************************/
bool
CamProfile::
print_overtime(
    int32_t const msTimeInterval, 
    char const*const fmt /*= ""*/, 
    ...
) const
{
    if  ( ! mfgIsProfile )
    {
        return  false;
    }
    //
    bool ret = false;
    //
    int32_t const i4EndUs = getUs();
    if  (0==mIdx)
    {
        int32_t const msElapsed = (i4EndUs-mi4StartUs)/1000;
        ret = ( msTimeInterval < msElapsed );
        if  ( ret )
        {
            va_list args;
            va_start(args, fmt);
            String8 result(String8::formatV(fmt, args));
            va_end(args);
            //
            CAM_LOGI(
                "{CamProfile}[%s::%s] %s: "
                "(%d-th) ===> (overtime > %d ms) [start-->now: %d ms]", 
                mpszClassName, mpszFuncName, result.string(), mIdx, 
                msTimeInterval, msElapsed
            );
            //
            goto lbExit;
        }
    }
    else
    {
        int32_t const msElapsed = (i4EndUs-mi4LastUs)/1000;
        ret = ( msTimeInterval < msElapsed );
        if  ( ret )
        {
            va_list args;
            va_start(args, fmt);
            String8 result(String8::formatV(fmt, args));
            va_end(args);
            //
            CAM_LOGI(
                "{CamProfile}[%s::%s] %s: "
                "(%d-th) ===> ( overtime > %d ms) [start-->now: %d ms] [last-->now: %d ms]", 
                mpszClassName, mpszFuncName, result.string(), mIdx, 
                msTimeInterval, (i4EndUs-mi4StartUs)/1000, msElapsed
            );
            //
            goto lbExit;
        }
    }
    //
lbExit:
    mIdx++;
    mi4LastUs = i4EndUs;
    return  ret;
}


};  // namespace MtkCamUtils
};  // namespace android

