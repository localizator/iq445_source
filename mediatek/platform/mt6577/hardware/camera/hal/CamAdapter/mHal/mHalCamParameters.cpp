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
#include "Utils/inc/CamUtils.h"
#include "inc/ICamAdapter.h"
#include "inc/BaseCamAdapter.h"
#include "inc/mHalBaseAdapter.h"
#include "inc/mHalCamAdapter.h"

//
#include <camera_feature.h>
using namespace NSFeature;
//
#include <mhal/inc/camera.h>
#include <mhal/inc/camera/ioctl.h>
using namespace NSCamera;
//
using namespace android;


/******************************************************************************
*
*******************************************************************************/
#define MY_LOGV(fmt, arg...)    CAM_LOGV("[mHalCamAdapter::%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)    CAM_LOGD("[mHalCamAdapter::%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)    CAM_LOGI("[mHalCamAdapter::%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)    CAM_LOGW("[mHalCamAdapter::%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)    CAM_LOGE("[mHalCamAdapter::%s] "fmt, __FUNCTION__, ##arg)


/******************************************************************************
*
*******************************************************************************/
//NOTE: zoomRatio can be customized. BUT
//Ratio should be no less than 100. The first one must be 100.
//The upper bound is limited to hardware capability.
//Value must be ordered in ascedent
static const uint32_t gZoomRatio[] = {
    100, 114, 132, 151, 174, 
    200, 229, 263, 303, 348, 
    400
};


/******************************************************************************
*
*******************************************************************************/
uint32_t 
MTKCameraHardware::
getZoomValue(int index)
{
    return gZoomRatio[index];
}


/******************************************************************************
*
*******************************************************************************/
void 
MTKCameraHardware::
initZoomParam()
{
    mParameters.set(CameraParameters::KEY_ZOOM_SUPPORTED, "true");
    mParameters.set(CameraParameters::KEY_SMOOTH_ZOOM_SUPPORTED, "false");
    mParameters.set(CameraParameters::KEY_ZOOM, mZoomValue);
	
    mZoom_max_value = sizeof(gZoomRatio)/sizeof(gZoomRatio[0])-1 ;  // because max_value can be zero 
    mParameters.set(CameraParameters::KEY_MAX_ZOOM, mZoom_max_value);

    char buffer[1024];
	sprintf(buffer, "%d", gZoomRatio[0]);
    for (int i = 1; i <= mZoom_max_value; i++)
    {
        sprintf(buffer, "%s,%d", buffer, gZoomRatio[i]);
    }
    CAM_LOGD("zoom parm: %s", buffer);

    if (gZoomRatio[0] < 100) {
        CAM_LOGE("WRONG zoomRatio: no less than 100, no larger than 800");
    }
	
    mParameters.set(CameraParameters::KEY_ZOOM_RATIOS, buffer);  //FIXME: Not link with driver now
}


/******************************************************************************
*
*******************************************************************************/
char* 
MTKCameraHardware::
getFeatureIDToStr(int featureID)
{
    char *pstrResult = NULL;
    //
    switch (featureID) {
    case FID_AE_FLICKER:
        pstrResult = (char *) CameraParameters::KEY_ANTIBANDING;
        break;
    case FID_AE_METERING:
        pstrResult = (char *) MtkCameraParameters::KEY_EXPOSURE_METER;
        break;
    case FID_AE_ISO:
        pstrResult = (char *) MtkCameraParameters::KEY_ISO_SPEED;
        break;
    case FID_AE_EV:
        pstrResult = (char *) MtkCameraParameters::KEY_EXPOSURE;
        break;
    case FID_AE_STROBE:
        pstrResult = (char *) CameraParameters::KEY_FLASH_MODE;
        break;
    case FID_AWB_MODE:
        pstrResult = (char *) CameraParameters::KEY_WHITE_BALANCE;
        break;
    case FID_AF_MODE:
        pstrResult = (char *) CameraParameters::KEY_FOCUS_MODE;
        break;
    case FID_AF_METERING:
        pstrResult = (char *) MtkCameraParameters::KEY_FOCUS_METER;
        break;
    case FID_AE_SCENE_MODE:
        pstrResult = (char *) MtkCameraParameters::KEY_AE_MODE;
        break;
    case FID_SCENE_MODE:
        pstrResult = (char *) CameraParameters::KEY_SCENE_MODE;
        break;
    case FID_COLOR_EFFECT:
        pstrResult = (char *) CameraParameters::KEY_EFFECT;
        break;
    case FID_ISP_EDGE:
        pstrResult = (char *) MtkCameraParameters::KEY_EDGE;
        break;
    case FID_ISP_HUE:
        pstrResult = (char *) MtkCameraParameters::KEY_HUE;
        break;
    case FID_ISP_SAT:
        pstrResult = (char *) MtkCameraParameters::KEY_SATURATION;
        break;
    case FID_ISP_BRIGHT:
        pstrResult = (char *) MtkCameraParameters::KEY_BRIGHTNESS;
        break;
    case FID_ISP_CONTRAST:
        pstrResult = (char *) MtkCameraParameters::KEY_CONTRAST;
        break;
    case FID_FD_ON_OFF:
        //pstrResult = (char *) MtkCameraParameters::KEY_FD_MODE;
        break;
    case FID_CAP_SIZE:
        pstrResult = (char *) CameraParameters::KEY_PICTURE_SIZE;
        break;
    case FID_PREVIEW_SIZE:
        pstrResult = (char *) CameraParameters::KEY_PREVIEW_SIZE;
        break;
    case FID_FRAME_RATE:
        pstrResult = (char *) CameraParameters::KEY_PREVIEW_FRAME_RATE;
        break;
    case FID_EIS:
        pstrResult = (char *) CameraParameters::KEY_VIDEO_STABILIZATION; 
        break;
    case FID_ZSD:
        pstrResult = (char *) MtkCameraParameters::KEY_ZSD_MODE; 
        break;                
    case FID_AF_LAMP:
        pstrResult = (char *) MtkCameraParameters::KEY_AF_LAMP_MODE; 
        break;     
    case FID_STEREO_3D_CAP_SIZE:
        pstrResult = (char *) MtkCameraParameters::KEY_STEREO_3D_PICTURE_SIZE;
        break;
    case FID_STEREO_3D_PREVIEW_SIZE:
        pstrResult = (char *) MtkCameraParameters::KEY_STEREO_3D_PREVIEW_SIZE;
        break;                
    case FID_STEREO_3D_TYPE:
        pstrResult = (char *) MtkCameraParameters::KEY_STEREO_3D_TYPE; 
        break;        
    case FID_STEREO_3D_MODE:
        pstrResult = (char *) MtkCameraParameters::KEY_STEREO_3D_MODE; 
        break;
    case FID_STEREO_3D_IMAGE_FORMAT:
        pstrResult = (char *) MtkCameraParameters::KEY_STEREO_3D_IMAGE_FORMAT; 
        break;                
    case FID_CAPTURE_MODE:
        pstrResult = (char *) MtkCameraParameters::KEY_CAPTURE_MODE;
        break;
    case FID_FRAME_RATE_RANGE:
        pstrResult = (char *) CameraParameters::KEY_PREVIEW_FPS_RANGE;
        break;
    case FID_FAST_CONTINUOUS_SHOT:
        pstrResult = (char *) MtkCameraParameters::KEY_FAST_CONTINUOUS_SHOT; 
        break;
    case FID_AWB2PASS:
        pstrResult = (char *) MtkCameraParameters::KEY_AWB2PASS; 
        break;
    //case FID_FOCUS_DIST_NORMAL:
    //  break;
    default:
        CAM_LOGD("Undefined, T.B.D, 0x%x \n", featureID);
        return NULL;
        break;
    }
    //
    return pstrResult;
}

/******************************************************************************
*
*******************************************************************************/
char* 
MTKCameraHardware::
getFeatureEnumToStr(int featureID, int featureVal)
{
    // MtkCameraParameters::KEY_ANTIBANDING
    int flickerMode[] = {
        AE_FLICKER_MODE_AUTO, AE_FLICKER_MODE_50HZ, AE_FLICKER_MODE_60HZ,
        AE_FLICKER_MODE_OFF};
    const char *pflickerMode[] = {
        CameraParameters::ANTIBANDING_AUTO, CameraParameters::ANTIBANDING_50HZ, CameraParameters::ANTIBANDING_60HZ,
        CameraParameters::ANTIBANDING_OFF};
    //
    // exposure-meter
    int aeMeterMode[] = {
        AE_METERING_MODE_SOPT, AE_METERING_MODE_CENTER_WEIGHT,
        AE_METERING_MODE_AVERAGE};
    const char *paeMeterMode[] = {
        MtkCameraParameters::EXPOSURE_METER_SPOT, MtkCameraParameters::EXPOSURE_METER_CENTER,
        MtkCameraParameters::EXPOSURE_METER_AVERAGE};
    //
    // iso-speed
    int isospeedMode[] = {
        AE_ISO_AUTO, AE_ISO_100,
        AE_ISO_200, AE_ISO_400,
        AE_ISO_800, AE_ISO_1600
    };
    const char *pisospeedMode[] = {
        MtkCameraParameters::ISO_SPEED_AUTO, MtkCameraParameters::ISO_SPEED_100,
        MtkCameraParameters::ISO_SPEED_200, MtkCameraParameters::ISO_SPEED_400,
        MtkCameraParameters::ISO_SPEED_800,MtkCameraParameters::ISO_SPEED_1600};
    //
    // exposure
    int aeExpMode[] = {
        AE_EV_COMP_n30, AE_EV_COMP_n27,
        AE_EV_COMP_n25, AE_EV_COMP_n23,
        AE_EV_COMP_n20, AE_EV_COMP_n17,
        AE_EV_COMP_n15, AE_EV_COMP_n13,
        AE_EV_COMP_n10, AE_EV_COMP_n07,
        AE_EV_COMP_n05, AE_EV_COMP_n03,
        AE_EV_COMP_00, AE_EV_COMP_03,
        AE_EV_COMP_05, AE_EV_COMP_07,
        AE_EV_COMP_10, AE_EV_COMP_13,
        AE_EV_COMP_15, AE_EV_COMP_17,
        AE_EV_COMP_20, AE_EV_COMP_23,
        AE_EV_COMP_25, AE_EV_COMP_27,
        AE_EV_COMP_30};
    const char *paeExpMode[] = {
        "-3", "-2.7", "-2.5", "-2.3",
        "-2", "-1.7", "-1.5", "-1.3",
        "-1", "-0.7", "-0.5", "-0.3",
        "0", "0.3", "0.5", "0.7",
        "1", "1.3", "1.5", "1.7",
        "2", "2.3", "2.5", "2.7",
        "3"};
    //
    // MtkCameraParameters::KEY_FLASH_MODE, need to take care of red-eye mode
    int strobeMode[] = {
        FLASHLIGHT_AUTO, FLASHLIGHT_FORCE_ON,
        FLASHLIGHT_FORCE_OFF, FLASHLIGHT_REDEYE, FLASHLIGHT_TORCH};
    const char *pstrobeMode[] = {
        CameraParameters::FLASH_MODE_AUTO, CameraParameters::FLASH_MODE_ON,
        CameraParameters::FLASH_MODE_OFF, CameraParameters::FLASH_MODE_RED_EYE, CameraParameters::FLASH_MODE_TORCH};
    //
    // MtkCameraParameters::KEY_WHITE_BALANCE
    int awbMode[] = {
        AWB_MODE_AUTO, AWB_MODE_INCANDESCENT,
        AWB_MODE_FLUORESCENT, AWB_MODE_WARM_FLUORESCENT,
        AWB_MODE_DAYLIGHT, AWB_MODE_CLOUDY_DAYLIGHT,
        AWB_MODE_TWILIGHT, AWB_MODE_SHADE,
        AWB_MODE_TUNGSTEN};
    const char *pawbMode[] = {
        CameraParameters::WHITE_BALANCE_AUTO, CameraParameters::WHITE_BALANCE_INCANDESCENT,
        CameraParameters::WHITE_BALANCE_FLUORESCENT, CameraParameters::WHITE_BALANCE_WARM_FLUORESCENT,
        CameraParameters::WHITE_BALANCE_DAYLIGHT, CameraParameters::WHITE_BALANCE_CLOUDY_DAYLIGHT,
        CameraParameters::WHITE_BALANCE_TWILIGHT, CameraParameters::WHITE_BALANCE_SHADE,
        MtkCameraParameters::WHITE_BALANCE_TUNGSTEN};
    //
    // MtkCameraParameters::KEY_FOCUS_MODE
    int afMode[] = {
        AF_MODE_AFS, AF_MODE_MACRO,
        AF_MODE_INFINITY, AF_MODE_AFC, AF_MODE_AFC_VIDEO, 
        AF_MODE_MF, AF_MODE_FULLSCAN};
    const char *pafMode[] = {
        CameraParameters::FOCUS_MODE_AUTO, CameraParameters::FOCUS_MODE_MACRO,
        CameraParameters::FOCUS_MODE_INFINITY, CameraParameters::FOCUS_MODE_CONTINUOUS_PICTURE,
        CameraParameters::FOCUS_MODE_CONTINUOUS_VIDEO,
        "manual", "fullscan"};
    // focus-meter
    int afMeterMode[] = {
        AF_METER_SPOT, AF_METER_MATRIX};
    const char *pafMeterMode[] = {
        MtkCameraParameters::FOCUS_METER_SPOT, MtkCameraParameters::FOCUS_METER_MULTI};
    //
    // ae-mode
    int aeMode[] = {
        AE_MODE_AUTO, AE_MODE_PROGRAM, AE_MODE_TV, AE_MODE_AV,
        AE_MODE_SV, AE_MODE_VIDEO, AE_MODE_NIGHT, AE_MODE_ACTION,
        AE_MODE_BEACH, AE_MODE_CANDLELIGHT, AE_MODE_FIREWORKS, AE_MODE_LANDSCAPE,
        AE_MODE_PORTRAIT, AE_MODE_NIGHT_PORTRAIT, AE_MODE_PARTY, AE_MODE_SNOW,
        AE_MODE_SPORTS, AE_MODE_STEADYPHOTO, AE_MODE_SUNSET, AE_MODE_THEATRE,
        AE_MODE_ISO_ANTI_SHAKE, AE_MODE_BRACKET_AE};
    const char *paeMode[] = {
        "1", "2", "3", "4",
        "5", "6", "7", "8",
        "9", "10", "11", "12",
        "13", "14", "15", "16",
        "17", "18", "19", "20",
        "21", "22"};
    //
    // MtkCameraParameters::KEY_SUPPORTED_SCENE_MODES
    int sceneMode[] = {
        SCENE_MODE_OFF, SCENE_MODE_NORMAL, SCENE_MODE_PORTRAIT, SCENE_MODE_LANDSCAPE,
        SCENE_MODE_NIGHTSCENE, SCENE_MODE_NIGHTPORTRAIT, SCENE_MODE_THEATRE, SCENE_MODE_BEACH,
        SCENE_MODE_SNOW, SCENE_MODE_SUNSET, SCENE_MODE_STEADYPHOTO, SCENE_MODE_FIREWORKS,
        SCENE_MODE_SPORTS, SCENE_MODE_PARTY, SCENE_MODE_CANDLELIGHT/*, EF_SCENE_ISO_ANTI_SHAKE*/};
    const char *psceneMode[] = {
        CameraParameters::SCENE_MODE_AUTO, CameraParameters::SCENE_MODE_NORMAL,
        CameraParameters::SCENE_MODE_PORTRAIT, CameraParameters::SCENE_MODE_LANDSCAPE,
        CameraParameters::SCENE_MODE_NIGHT, CameraParameters::SCENE_MODE_NIGHT_PORTRAIT,
        CameraParameters::SCENE_MODE_THEATRE, CameraParameters::SCENE_MODE_BEACH,
        CameraParameters::SCENE_MODE_SNOW, CameraParameters::SCENE_MODE_SUNSET,
        CameraParameters::SCENE_MODE_STEADYPHOTO, CameraParameters::SCENE_MODE_FIREWORKS,
        CameraParameters::SCENE_MODE_SPORTS, CameraParameters::SCENE_MODE_PARTY,
        CameraParameters::SCENE_MODE_CANDLELIGHT};
    //
    // MtkCameraParameters::KEY_EFFECT
    int effectMode[] = {
        MEFFECT_OFF, MEFFECT_MONO, MEFFECT_SEPIA, MEFFECT_NEGATIVE,
        MEFFECT_SOLARIZE, MEFFECT_AQUA, MEFFECT_BLACKBOARD, MEFFECT_POSTERIZE,
        MEFFECT_WHITEBOARD, MEFFECT_SEPIABLUE, MEFFECT_SEPIAGREEN};
    const char *peffectMode[] = {
        CameraParameters::EFFECT_NONE, CameraParameters::EFFECT_MONO,
        CameraParameters::EFFECT_SEPIA, CameraParameters::EFFECT_NEGATIVE,
        CameraParameters::EFFECT_SOLARIZE, CameraParameters::EFFECT_AQUA,
        CameraParameters::EFFECT_BLACKBOARD, CameraParameters::EFFECT_POSTERIZE,
        CameraParameters::EFFECT_WHITEBOARD, MtkCameraParameters::EFFECT_SEPIA_BLUE,
        MtkCameraParameters::EFFECT_SEPIA_GREEN};
    //
    //
    int levelMode[] = {
        ISP_EDGE_HIGH, ISP_EDGE_MIDDLE, ISP_EDGE_LOW};
    const char *plevelMode[] = {
        MtkCameraParameters::HIGH, MtkCameraParameters::MIDDLE, MtkCameraParameters::LOW};

    //
    int eisMode[] = {
        EIS_OFF, EIS_ON}; 
    const char *peisMode[] = {
        CameraParameters::FALSE, CameraParameters::TRUE};
    //
    int zsdMode[] = {
        ZSD_OFF, ZSD_ON}; 
    const char *pzsdMode[] = {
        MtkCameraParameters::OFF, MtkCameraParameters::ON}; 
    //
    int fastContinuousMode[] = {
        FCS_OFF, FCS_ON}; 
    const char *pfastContinuousMode[] = {
        MtkCameraParameters::OFF, MtkCameraParameters::ON}; 
    //
    int awb2pass[] = {
        AWB2PASS_OFF, AWB2PASS_ON}; 
    const char *pawb2pass[] = {
        MtkCameraParameters::OFF, MtkCameraParameters::ON}; 
    //    
    int aflampMode[] = {
        AF_LAMP_OFF, AF_LAMP_ON, AF_LAMP_AUTO, AF_LAMP_FLASH}; 
    const char *paflampMode[] = {
        "off", "on", "auto", "flash"}; 
    //
    int Stereo3DType[] = {
        STEREO_3D_NOT_SUPPORT, STEREO_3D_FRAME_SEQ, STEREO_3D_SIDE_BY_SIDE, STEREO_3D_TOP_BOTTOM}; 
    const char *pStereo3DType[] = {
        "off", "frame_seq", "sidebyside", "topbottom"};         

    //
    int Stereo3DMode[] = {
        STEREO_3D_OFF, STEREO_3D_ON}; 
    const char *pStereo3DMode[] = {
        "off", "on"};         
    //
    int Stereo3DImageFormat[] = {
        STEREO_3D_JPS, STEREO_3D_MPO}; 
    const char *pStereo3DImageFormat[] = {
        "jps", "mpo"};
    //    
    int picSizes[] = {
        CAPTURE_SIZE_3264_2448, CAPTURE_SIZE_2560_1920, CAPTURE_SIZE_2048_1536, CAPTURE_SIZE_1600_1200, CAPTURE_SIZE_1280_960,
        CAPTURE_SIZE_1024_768, CAPTURE_SIZE_640_480, CAPTURE_SIZE_320_240};
    const char *ppicSizes[] = {
        "3264x2448", "2560x1920", "2048x1536", "1600x1200", "1280x960", "1024x768",
        "640x480", "320x240"};
    //
    //
    int prvSizes[] = {
        PREVIEW_SIZE_1920_1080, PREVIEW_SIZE_1280_720, PREVIEW_SIZE_800_600, PREVIEW_SIZE_864_480, PREVIEW_SIZE_800_480, PREVIEW_SIZE_720_480,
        PREVIEW_SIZE_640_480, PREVIEW_SIZE_480_368, PREVIEW_SIZE_480_320, PREVIEW_SIZE_352_288, PREVIEW_SIZE_320_240, PREVIEW_SIZE_176_144,
        PREVIEW_SIZE_160_120};
    const char *pprvSizes[] = {
        "1920x1088", "1280x720", "800x600", "864x480", "800x480", "720x480",
        "640x480", "480x368", "480x320", "352x288", "320x240", "176x144",
        "160x120"};
    //
    //    
    int Stereo3D_picSizes[] = {
        STEREO_3D_CAPTURE_SIZE_2560_720, STEREO_3D_CAPTURE_SIZE_3840_1080};
    const char *Stereo3D_ppicSizes[] = {
        "2560x720", "3840x1080"};
    //
    //
    int Stereo3D_prvSizes[] = {
        STEREO_3D_PREVIEW_SIZE_1280_720, STEREO_3D_PREVIEW_SIZE_960_540, STEREO_3D_PREVIEW_SIZE_854_480, STEREO_3D_PREVIEW_SIZE_640_360};
    const char *Stereo3D_pprvSizes[] = {
        "1280x720", "960x540", "854x480", "640x360"};
    
    //
//    int prvFpss[] = {
//        FRAME_RATE_300FPS, FRAME_RATE_250FPS, FRAME_RATE_150FPS, FRAME_RATE_125FPS,
//        FRAME_RATE_075FPS};
    const char *pprvFpss[] = {
         "0",  "1",  "2",  "3",  "4",  "5",  "6",  "7",  "8",  "9", 
        "10", "11", "12", "13", "14", "15", "16", "17", "18", "19", 
        "20", "21", "22", "23", "24", "25", "26", "27", "28", "29", 
        "30"};
    //
    //
    int pcapModes[] = {
        CAPTURE_MODE_NORMAL, CAPTURE_MODE_BURST_SHOT, CAPTURE_MODE_SMILE_SHOT, CAPTURE_MODE_PANORAMA, CAPTURE_MODE_AUTORAMA,
        CAPTURE_MODE_BEST_SHOT, CAPTURE_MODE_EV_BRACKET, CAPTURE_MODE_MAV, CAPTURE_MODE_HDR, CAPTURE_MODE_ASD, CAPTURE_MODE_PANO_3D, CAPTURE_MODE_SINGLE_3D, CAPTURE_MODE_FACE_BEAUTY, CAPTURE_MODE_CONTINUOUS_SHOT};
    const char *ppcapModes[] = {
        MtkCameraParameters::CAPTURE_MODE_NORMAL, MtkCameraParameters::CAPTURE_MODE_BURST_SHOT,
        MtkCameraParameters::CAPTURE_MODE_SMILE_SHOT, MtkCameraParameters::CAPTURE_MODE_PANORAMA_SHOT, 
        MtkCameraParameters::CAPTURE_MODE_AUTO_PANORAMA_SHOT, MtkCameraParameters::CAPTURE_MODE_BEST_SHOT, 
        MtkCameraParameters::CAPTURE_MODE_EV_BRACKET_SHOT, MtkCameraParameters::CAPTURE_MODE_MAV_SHOT, 
        MtkCameraParameters::CAPTURE_MODE_HDR_SHOT, MtkCameraParameters::CAPTURE_MODE_ASD_SHOT, 
        MtkCameraParameters::CAPTURE_MODE_PANO_3D, MtkCameraParameters::CAPTURE_MODE_SINGLE_3D, MtkCameraParameters::CAPTURE_MODE_FACE_BEAUTY, 
        MtkCameraParameters::CAPTURE_MODE_CONTINUOUS_SHOT
        };
    //
    int prvFpsRanges[] = {
        FRAME_RATE_RANGE_5_30_FPS};
    const char *pprvFpsRanges[] = {
        "5000,30000"};
    //
    int *pvalAry = NULL;
    const char **ppstrAry = NULL;
    char *pstrResult = NULL;
    int count = 0;
    int i;

    switch (featureID) {
    case FID_AE_FLICKER:
        pvalAry = flickerMode;
        ppstrAry = pflickerMode;
        count = sizeof(flickerMode) / 4;
        break;
    case FID_AE_METERING:
        pvalAry = aeMeterMode;
        ppstrAry = paeMeterMode;
        count = sizeof(aeMeterMode) / 4;
        break;
    case FID_AE_ISO:
        pvalAry = isospeedMode;
        ppstrAry = pisospeedMode;
        count = sizeof(isospeedMode) / 4;
        break;
    case FID_AE_EV:
        pvalAry = aeExpMode;
        ppstrAry = paeExpMode;
        count = sizeof(aeExpMode) / 4;
        break;
    case FID_AE_STROBE:
        pvalAry = strobeMode;
        ppstrAry = pstrobeMode;
        count = sizeof(strobeMode) / 4;
        break;
    case FID_AWB_MODE:
        pvalAry = awbMode;
        ppstrAry = pawbMode;
        count = sizeof(awbMode) / 4;
        break;
    case FID_AF_MODE:
        pvalAry = afMode;
        ppstrAry = pafMode;
        count = sizeof(afMode) / 4;
        break;
    case FID_AF_METERING:
        pvalAry = afMeterMode;
        ppstrAry = pafMeterMode;
        count = sizeof(afMeterMode) / 4;
        break;
    case FID_AE_SCENE_MODE:
        // do nothing, because UI doesn't need to set this
        pvalAry = aeMode;
        ppstrAry = paeMode;
        count = sizeof(aeMode) / 4;
        break;
    case FID_SCENE_MODE:
        pvalAry = sceneMode;
        ppstrAry = psceneMode;
        count = sizeof(sceneMode) / 4;
        break;
    case FID_COLOR_EFFECT:
        pvalAry = effectMode;
        ppstrAry = peffectMode;
        count = sizeof(effectMode) / 4;
        break;
    case FID_ISP_EDGE:
    case FID_ISP_HUE:
    case FID_ISP_SAT:
    case FID_ISP_BRIGHT:
    case FID_ISP_CONTRAST:
        pvalAry = levelMode;
        ppstrAry = plevelMode;
        count = sizeof(levelMode) / 4;
        break;
    case FID_CAP_SIZE:
        pvalAry = picSizes;
        ppstrAry = ppicSizes;
        count = sizeof(picSizes) / 4;
        break;
    case FID_PREVIEW_SIZE:
        pvalAry = prvSizes;
        ppstrAry = pprvSizes;
        count = sizeof(prvSizes) / 4;
        break;
    case FID_FRAME_RATE:
//        pvalAry = prvFpss;
        ppstrAry = pprvFpss;
//        count = sizeof(prvFpss) / 4;
        pstrResult = (char *) ppstrAry[featureVal];
        return  pstrResult;
//        break;
    case FID_EIS:
        pvalAry = eisMode;
        ppstrAry = peisMode;
        count = sizeof(eisMode) / 4;
        break;
    case FID_ZSD:
        pvalAry = zsdMode;
        ppstrAry = pzsdMode;
        count = sizeof(zsdMode) / 4;
        break;        
    case FID_AWB2PASS:
        pvalAry = awb2pass;
        ppstrAry = pawb2pass;
        count = sizeof(awb2pass) / 4;
        break;        
    case FID_AF_LAMP:
        pvalAry = aflampMode;
        ppstrAry = paflampMode;
        count = sizeof(aflampMode) / 4;
        break;     
    case FID_STEREO_3D_CAP_SIZE:
        pvalAry = Stereo3D_picSizes;
        ppstrAry = Stereo3D_ppicSizes;
        count = sizeof(Stereo3D_picSizes) / 4;
        break;
    case FID_STEREO_3D_PREVIEW_SIZE:
        pvalAry = Stereo3D_prvSizes;
        ppstrAry = Stereo3D_pprvSizes;
        count = sizeof(Stereo3D_prvSizes) / 4;
        break;                    
    case FID_STEREO_3D_TYPE:
        pvalAry = Stereo3DType;
        ppstrAry = pStereo3DType;
        count = sizeof(Stereo3DType) / 4;
        break;
    case FID_STEREO_3D_MODE:
        pvalAry = Stereo3DMode;
        ppstrAry = pStereo3DMode;
        count = sizeof(Stereo3DMode) / 4;
        break;
    case FID_STEREO_3D_IMAGE_FORMAT:
        pvalAry = Stereo3DImageFormat;
        ppstrAry = pStereo3DImageFormat;
        count = sizeof(Stereo3DImageFormat) / 4;
        break;        
    case FID_CAPTURE_MODE:
        pvalAry = pcapModes;
        ppstrAry = ppcapModes;
        count = sizeof(pcapModes) / 4;
        break;
    case FID_FRAME_RATE_RANGE:
        pvalAry = prvFpsRanges;
        ppstrAry = pprvFpsRanges;
        count = sizeof(prvFpsRanges) / 4;
        break;
    case FID_FAST_CONTINUOUS_SHOT:
        pvalAry = fastContinuousMode;
        ppstrAry = pfastContinuousMode;
        count = sizeof(fastContinuousMode) / 4;
        break;        
    //case FID_FOCUS_DIST_NORMAL:
    //  break;
    default:
        CAM_LOGD("Undefined, T.B.D \n");
        return NULL;
        break;
    }
    //
    for (int i = 0; i < count; i++) {
        if (pvalAry[i] == featureVal) {
            pstrResult = (char *) ppstrAry[i];
            break;
        }
    }
    //
    //CAM_LOGD("[getFeatureEnumToStr] %d, %s \n", featureVal, pstrResult);
    //
    return pstrResult;
}

/******************************************************************************
*
*******************************************************************************/
char* 
MTKCameraHardware::
getSupportedValuesSuffix(char *pname)
{
    static char data[64];
    char *pdata = data;

    sprintf(pdata, "%s-values", pname);

    return pdata;
}

/******************************************************************************
*
*******************************************************************************/
int 
MTKCameraHardware::
getSupportedValueToEnum(const char *pstrVal, const char *pstrValList, const char *pstrEnumList)
{
    char *pch;
    int index = 0;
    int found = 0;
    char strValList[256];
    int enumIdx = 0;
    int retEnum = -1;

    strcpy(strValList, pstrValList);
    pch = strtok(strValList, " ,");
    while (pch != NULL) {
        if (strcmp(pstrVal, pch) == 0) {
            found = 1;
            break;
        }
        index++;
        pch = strtok(NULL, " ,");
    }
    if (found == 1) {
        strcpy(strValList, pstrEnumList);
        pch = strtok(strValList, " ,");
        enumIdx = 0;
        while (pch != NULL) {
            if (enumIdx == index) {
                retEnum = strtol(pch, 0, 0);
            }
            enumIdx++;
            pch = strtok(NULL, " ,");
        }
    }

    //CAM_LOGD("[getSupportedValueToEnum] (%s, %d), (%s), (%s) \n", pstrVal, retEnum, pstrValList, pstrEnumList);

    return retEnum;
}

/******************************************************************************
*
*******************************************************************************/
void 
MTKCameraHardware::
setCamFeatureMode(int sceneMode, int effectMode)
{
    status_t status;
    mhalCam3AParam_t cam3aParam;

    CAM_LOGD("[setCamFeatureMode] \n");

    memset(&cam3aParam, 0, sizeof(mhalCam3AParam_t));
    cam3aParam.sceneMode = sceneMode;
    cam3aParam.effectMode = effectMode;
    status = ::mHalIoctl(mmHalFd,MHAL_IOCTL_SET_CAM_FEATURE_MODE, &cam3aParam, sizeof(mhalCam3AParam_t), NULL, 0, NULL);
    if (status != NO_ERROR) {
        CAM_LOGD("[setCamFeatureMode], MHAL_IOCTL_SET_CAM_FEATURE_MODE err \n");
    }
}

/******************************************************************************
*
*******************************************************************************/
void 
MTKCameraHardware::
getCamFeatureEnums()
{
    status_t status;
    uint32_t const u4OutBufNum = FID_NUM;
    uint32_t u4NumReturned = FID_NUM;
    CamFeatureStruct featureAry[u4OutBufNum];
    char *pstrFName, *pstrFNameVals;
    char *pstrFVal, *pstr;
    char listVals[128];
    char *plistVals;
    char listEnums[128];
    char *plistEnums;
    char *pstrList;
    int len;

    CAM_LOGD("[getCamFeatureEnums] \n");
    //
    status = ::mHalIoctl(mmHalFd,MHAL_IOCTL_GET_CAM_FEATURE_ENUM, NULL, 0, &featureAry, u4OutBufNum, &u4NumReturned);
    if (status != NO_ERROR) {
        CAM_LOGD("[getCamFeatureEnums], MHAL_IOCTL_SET_CAM_FEATURE_MODE err \n");
        //+++++  android parameter begin +++++
        mParameters.set(CameraParameters::KEY_ANTIBANDING, CameraParameters::ANTIBANDING_OFF);
        mParameters.set(CameraParameters::KEY_SUPPORTED_ANTIBANDING, CameraParameters::ANTIBANDING_OFF);
        mParameters.set(CameraParameters::KEY_FLASH_MODE, CameraParameters::FLASH_MODE_AUTO);
        mParameters.set(CameraParameters::KEY_SUPPORTED_FLASH_MODES, CameraParameters::FLASH_MODE_AUTO);
        mParameters.set(CameraParameters::KEY_WHITE_BALANCE, CameraParameters::WHITE_BALANCE_AUTO);
        mParameters.set(CameraParameters::KEY_SUPPORTED_WHITE_BALANCE, CameraParameters::WHITE_BALANCE_AUTO);
        mParameters.set(CameraParameters::KEY_FOCUS_MODE, CameraParameters::FOCUS_MODE_INFINITY);
        mParameters.set(CameraParameters::KEY_SUPPORTED_FOCUS_MODES, CameraParameters::FOCUS_MODE_INFINITY);
        mParameters.set(CameraParameters::KEY_PICTURE_SIZE, "2560x1920");
        mParameters.set(CameraParameters::KEY_SUPPORTED_PICTURE_SIZES, "2560x1920");
        
        if (getDevId() == eDevId_AtvSensor)
        {
             mParameters.set(CameraParameters::KEY_PREVIEW_SIZE, "320x240");
             mParameters.set(CameraParameters::KEY_SUPPORTED_PREVIEW_SIZES, "320x240");
        }
        else
        {
             mParameters.set(CameraParameters::KEY_PREVIEW_SIZE, "640x480");
             mParameters.set(CameraParameters::KEY_SUPPORTED_PREVIEW_SIZES, "640x480");
        }

        mParameters.set(CameraParameters::KEY_PREVIEW_FRAME_RATE, "30");
        mParameters.set(CameraParameters::KEY_SUPPORTED_PREVIEW_FRAME_RATES, "30");
        mParameters.set(CameraParameters::KEY_EXPOSURE_COMPENSATION, 0);
        mParameters.set(CameraParameters::KEY_MAX_EXPOSURE_COMPENSATION, 0);
        mParameters.set(CameraParameters::KEY_MIN_EXPOSURE_COMPENSATION, 0);
        mParameters.set(CameraParameters::KEY_EXPOSURE_COMPENSATION_STEP, "1.0");
        mParameters.set(CameraParameters::KEY_SCENE_MODE, CameraParameters::SCENE_MODE_AUTO);
        mParameters.set(CameraParameters::KEY_SUPPORTED_SCENE_MODES, CameraParameters::SCENE_MODE_AUTO);
        mParameters.set(CameraParameters::KEY_EFFECT, CameraParameters::EFFECT_NONE);
        mParameters.set(getSupportedValuesSuffix((char *) CameraParameters::KEY_EFFECT), CameraParameters::EFFECT_NONE);
        mParameters.set(CameraParameters::KEY_PREVIEW_FPS_RANGE, "5000,30000");
        mParameters.set(CameraParameters::KEY_SUPPORTED_PREVIEW_FPS_RANGE, "(5000,30000)");
        mParameters.set(CameraParameters::KEY_VIDEO_STABILIZATION_SUPPORTED, CameraParameters::FALSE);      //Android 4 .0 
        mParameters.set(CameraParameters::KEY_AUTO_EXPOSURE_LOCK_SUPPORTED, CameraParameters::FALSE);       //Android 4.0 
        mParameters.set(CameraParameters::KEY_AUTO_WHITEBALANCE_LOCK_SUPPORTED, CameraParameters::FALSE);   //Android 4.0 
        //----- android parameter end -----
        //+++++ MTK proprietary parameter begin +++++
        // exposure-meter
        mParameters.set(MtkCameraParameters::KEY_EXPOSURE_METER, MtkCameraParameters::EXPOSURE_METER_CENTER);
        mParameters.set(getSupportedValuesSuffix((char *) MtkCameraParameters::KEY_EXPOSURE_METER), MtkCameraParameters::EXPOSURE_METER_CENTER);
        // iso-speed 
        mParameters.set(MtkCameraParameters::KEY_ISO_SPEED, MtkCameraParameters::ISO_SPEED_AUTO);
        mParameters.set(getSupportedValuesSuffix((char *) MtkCameraParameters::KEY_ISO_SPEED), MtkCameraParameters::ISO_SPEED_AUTO);
        // exposure  
        mParameters.set(MtkCameraParameters::KEY_EXPOSURE, "0");
        mParameters.set(getSupportedValuesSuffix((char *) MtkCameraParameters::KEY_EXPOSURE), 0);
        // focus-meter
        mParameters.set(MtkCameraParameters::KEY_FOCUS_METER, MtkCameraParameters::FOCUS_METER_SPOT);
        mParameters.set(getSupportedValuesSuffix((char *) MtkCameraParameters::KEY_FOCUS_METER), MtkCameraParameters::FOCUS_METER_SPOT);
        // AE mode 
        mParameters.set(MtkCameraParameters::KEY_AE_MODE, "1");
        mParameters.set(getSupportedValuesSuffix((char *) MtkCameraParameters::KEY_AE_MODE), "1");
        // Edge 
        mParameters.set(MtkCameraParameters::KEY_EDGE, MtkCameraParameters::MIDDLE);
        mParameters.set(getSupportedValuesSuffix((char *) MtkCameraParameters::KEY_EDGE), MtkCameraParameters::MIDDLE);
        // Hue 
        mParameters.set(MtkCameraParameters::KEY_HUE, MtkCameraParameters::MIDDLE);
        mParameters.set(getSupportedValuesSuffix((char *) MtkCameraParameters::KEY_HUE), MtkCameraParameters::MIDDLE);
        // Saturation 
        mParameters.set(MtkCameraParameters::KEY_SATURATION, MtkCameraParameters::MIDDLE);
        mParameters.set(getSupportedValuesSuffix((char *) MtkCameraParameters::KEY_SATURATION), MtkCameraParameters::MIDDLE);
        // Brightness 
        mParameters.set(MtkCameraParameters::KEY_BRIGHTNESS, MtkCameraParameters::MIDDLE);
        mParameters.set(getSupportedValuesSuffix((char *) MtkCameraParameters::KEY_BRIGHTNESS), MtkCameraParameters::MIDDLE);
        // constrast 
        mParameters.set(MtkCameraParameters::KEY_CONTRAST, MtkCameraParameters::MIDDLE);
        mParameters.set(getSupportedValuesSuffix((char *) MtkCameraParameters::KEY_CONTRAST), MtkCameraParameters::MIDDLE);
        // ZSD-mode 
        mParameters.set(MtkCameraParameters::KEY_ZSD_MODE, MtkCameraParameters::OFF); 
        mParameters.set(getSupportedValuesSuffix((char *) MtkCameraParameters::KEY_ZSD_MODE), MtkCameraParameters::OFF);        

        // Continuous Shot speed 
        mParameters.set(MtkCameraParameters::KEY_FAST_CONTINUOUS_SHOT, MtkCameraParameters::OFF); 
        mParameters.set(getSupportedValuesSuffix((char *) MtkCameraParameters::KEY_FAST_CONTINUOUS_SHOT), MtkCameraParameters::OFF); 
        // AWB2PASS        
        mParameters.set(MtkCameraParameters::KEY_AWB2PASS, MtkCameraParameters::OFF); 
        mParameters.set(getSupportedValuesSuffix((char *) MtkCameraParameters::KEY_AWB2PASS), MtkCameraParameters::OFF);        
        // AF Lamp mode 
        mParameters.set(MtkCameraParameters::KEY_AF_LAMP_MODE, MtkCameraParameters::OFF);         
        mParameters.set(getSupportedValuesSuffix((char *) MtkCameraParameters::KEY_AF_LAMP_MODE), MtkCameraParameters::OFF);
        // Stereo 3D-picture-size
        mParameters.set(MtkCameraParameters::KEY_STEREO_3D_PICTURE_SIZE, "2560x720");
        mParameters.set(getSupportedValuesSuffix((char *) MtkCameraParameters::KEY_STEREO_3D_PICTURE_SIZE), "1280x720");        
        // Stereo 3D-preview-size
        mParameters.set(MtkCameraParameters::KEY_STEREO_3D_PREVIEW_SIZE, "640x360");
        mParameters.set(getSupportedValuesSuffix((char *) MtkCameraParameters::KEY_STEREO_3D_PREVIEW_SIZE), "640x360");        
        // Stereo 3D-type 
        mParameters.set(MtkCameraParameters::KEY_STEREO_3D_TYPE, MtkCameraParameters::OFF); 
        mParameters.set(getSupportedValuesSuffix((char *) MtkCameraParameters::KEY_STEREO_3D_TYPE), MtkCameraParameters::OFF);        
        // Stereo 3D-mode 
        mParameters.set(MtkCameraParameters::KEY_STEREO_3D_MODE, MtkCameraParameters::OFF); 
        mParameters.set(getSupportedValuesSuffix((char *) MtkCameraParameters::KEY_STEREO_3D_MODE), MtkCameraParameters::OFF);        
        // Stereo 3D-Image-Format 
        mParameters.set(MtkCameraParameters::KEY_STEREO_3D_IMAGE_FORMAT, "jps"); 
        mParameters.set(getSupportedValuesSuffix((char *) MtkCameraParameters::KEY_STEREO_3D_IMAGE_FORMAT), "jps");
        //-----------------------------------------------------------------
        //
        mFeatureParam.set(CameraParameters::KEY_SUPPORTED_ANTIBANDING, AE_FLICKER_MODE_OFF);
        mFeatureParam.set(getSupportedValuesSuffix((char *) MtkCameraParameters::KEY_EXPOSURE_METER), AE_METERING_MODE_CENTER_WEIGHT);
        mFeatureParam.set(getSupportedValuesSuffix((char *) MtkCameraParameters::KEY_ISO_SPEED), AE_ISO_AUTO);
        mFeatureParam.set(getSupportedValuesSuffix((char *) MtkCameraParameters::KEY_EXPOSURE), AE_EV_COMP_00);
        mFeatureParam.set(CameraParameters::KEY_SUPPORTED_FLASH_MODES, FLASHLIGHT_AUTO);
        mFeatureParam.set(CameraParameters::KEY_SUPPORTED_WHITE_BALANCE, AWB_MODE_AUTO);
        mFeatureParam.set(CameraParameters::KEY_SUPPORTED_FOCUS_MODES, AF_MODE_INFINITY);
        mFeatureParam.set(getSupportedValuesSuffix((char *) MtkCameraParameters::KEY_FOCUS_METER), AF_METER_SPOT);
        mFeatureParam.set(getSupportedValuesSuffix((char *) MtkCameraParameters::KEY_AE_MODE), AE_MODE_AUTO);
        mFeatureParam.set(CameraParameters::KEY_SUPPORTED_SCENE_MODES, SCENE_MODE_OFF);
        mFeatureParam.set(getSupportedValuesSuffix((char *) CameraParameters::KEY_EFFECT), MEFFECT_OFF);
        mFeatureParam.set(getSupportedValuesSuffix((char *) MtkCameraParameters::KEY_EDGE), ISP_EDGE_MIDDLE);
        mFeatureParam.set(getSupportedValuesSuffix((char *) MtkCameraParameters::KEY_HUE), ISP_EDGE_MIDDLE);
        mFeatureParam.set(getSupportedValuesSuffix((char *) MtkCameraParameters::KEY_SATURATION), ISP_EDGE_MIDDLE);
        mFeatureParam.set(getSupportedValuesSuffix((char *) MtkCameraParameters::KEY_BRIGHTNESS), ISP_EDGE_MIDDLE);
        mFeatureParam.set(getSupportedValuesSuffix((char *) MtkCameraParameters::KEY_CONTRAST), ISP_EDGE_MIDDLE);
        mFeatureParam.set(getSupportedValuesSuffix((char *) MtkCameraParameters::KEY_ZSD_MODE), ZSD_OFF);         
        mFeatureParam.set(getSupportedValuesSuffix((char *) MtkCameraParameters::KEY_FAST_CONTINUOUS_SHOT), FCS_OFF);  
        mFeatureParam.set(getSupportedValuesSuffix((char *) MtkCameraParameters::KEY_AF_LAMP_MODE), AF_LAMP_OFF);         
        mFeatureParam.set(getSupportedValuesSuffix((char *) MtkCameraParameters::KEY_STEREO_3D_TYPE), STEREO_3D_NOT_SUPPORT);         
        mFeatureParam.set(getSupportedValuesSuffix((char *) MtkCameraParameters::KEY_STEREO_3D_MODE), STEREO_3D_OFF);                 
        mFeatureParam.set(getSupportedValuesSuffix((char *) MtkCameraParameters::KEY_STEREO_3D_IMAGE_FORMAT), STEREO_3D_JPS); 
        mFeatureParam.set(CameraParameters::KEY_SUPPORTED_PICTURE_SIZES, CAPTURE_SIZE_2560_1920);
        if (getDevId() == eDevId_AtvSensor)
        {
            mFeatureParam.set(CameraParameters::KEY_SUPPORTED_PREVIEW_SIZES, PREVIEW_SIZE_320_240);
        }
        else
        {
            mFeatureParam.set(CameraParameters::KEY_SUPPORTED_PREVIEW_SIZES, PREVIEW_SIZE_640_480);
        }
        mFeatureParam.set(CameraParameters::KEY_SUPPORTED_PREVIEW_FRAME_RATES, FRAME_RATE_300FPS);
        mParameters.set(MtkCameraParameters::KEY_CAPTURE_MODE, MtkCameraParameters::CAPTURE_MODE_NORMAL);
        mParameters.set(MtkCameraParameters::KEY_SUPPORTED_CAPTURE_MODES, MtkCameraParameters::CAPTURE_MODE_NORMAL);
        return;
    }
    //
    CamFeatureStruct* pFeature_AE_Ev = NULL;
    CamFeatureStruct* pfsPrvFpsRanges = NULL;
    for (uint32_t i = 0; i < u4NumReturned; i++) {
        #if 0
        CAM_LOGD("  id: %d, flag: %d, total: %d, current: %d \n",
            featureAry[i].u4FeatureID,
            featureAry[i].u4FIDSupportFlag,
            featureAry[i].u4SubItemTotalNum,
            featureAry[i].u4DefaultSelection );
        #endif
        //
        if (featureAry[i].u4FIDSupportFlag == 1 && featureAry[i].u4SubItemTotalNum > 0) {
            switch (featureAry[i].u4FeatureID) {
            case FID_AE_EV:
                pFeature_AE_Ev = &featureAry[i];
                break;
            case FID_FRAME_RATE_RANGE:
                pfsPrvFpsRanges = &featureAry[i]; 
                // process fps range in the end of this function
                continue;
                break;
            }
            pstrFName = getFeatureIDToStr(featureAry[i].u4FeatureID);
            if  ( ! pstrFName ) {
                continue;
            }
            pstrFNameVals = getSupportedValuesSuffix(pstrFName);
            pstrFVal = getFeatureEnumToStr(featureAry[i].u4FeatureID, featureAry[i].u4DefaultSelection);
            if (pstrFVal == NULL) {
                //CAM_LOGD("  Can't find: %s, %d \n", pstrKey, featureAry[i].i32DefaultSelection);
                continue;
            }
            plistVals = listVals;
            plistEnums = listEnums;
            memset(plistVals, 0, 128);
            memset(plistEnums, 0, 128);
            for (int j = 0; j < (int) featureAry[i].u4SubItemTotalNum; j++) {
                pstr = getFeatureEnumToStr(featureAry[i].u4FeatureID, featureAry[i].pu4SubItemAllSupport[j]);
                if (pstr == NULL) {
                    //CAM_LOGD("  Can't find: %s, %d \n", pstrKey, featureAry[i].i32SubItemAllSupport[j]);
                    continue;
                }
                // connect string
                strcpy(plistVals, pstr);
                plistVals += strlen(pstr);
                *plistVals++ = ',';
                *plistVals = 0;
                // connect sub items into string
                sprintf(plistEnums, "%d,", featureAry[i].pu4SubItemAllSupport[j]);
                plistEnums += strlen(plistEnums);
            }
            if ((len = strlen(listVals)) != 0) {
                listVals[len - 1] = 0; // Remove last ','
            }
            if ((len = strlen(listEnums)) != 0) {
                listEnums[len - 1] = 0; // Remove last ','
            }
            //CAM_LOGD("  (%s, %s), (%s, %s), (%s) \n", pstrFName, pstrFVal, pstrFNameVals, listVals, listEnums);
            mParameters.set(pstrFName, pstrFVal);
            mParameters.set(pstrFNameVals, listVals);
            mFeatureParam.set(pstrFNameVals, listEnums);
        }
    }
    // expsoure related for 2.2
    int ev, minEV = AE_EV_COMP_00, maxEV = AE_EV_COMP_00;
    for (int j = 0; j < (int) pFeature_AE_Ev->u4SubItemTotalNum; j++) {
        ev = pFeature_AE_Ev->pu4SubItemAllSupport[j];
        if (ev <= AE_EV_COMP_30) {
            if (ev > maxEV) {
                maxEV = ev;
            }
        }
        else {
            if (ev > minEV) {
                minEV = ev;
            }
        }
    }
    if (maxEV >= AE_EV_COMP_30) {
        maxEV = 3;
    }
    else if (maxEV >= AE_EV_COMP_20) {
        maxEV = 2;
    }    
    else if (maxEV >= AE_EV_COMP_10) {
        maxEV = 1;
    }
    else {
        maxEV = 0;
    }
    
    if (minEV >= AE_EV_COMP_n30) {
        minEV = -3;
    }    
    else if (minEV >= AE_EV_COMP_n20) {
        minEV = -2;
    }
    else if (minEV >= AE_EV_COMP_n10) {
        minEV = -1;
    }
    else {
        minEV = 0;
    }
    //@Sean, should restore the EV value accroding to the scene mode setting
    pstrFVal = getFeatureEnumToStr(pFeature_AE_Ev->u4FeatureID, pFeature_AE_Ev->u4DefaultSelection);
    //
    mParameters.set(CameraParameters::KEY_EXPOSURE_COMPENSATION, pstrFVal);
    mParameters.set(CameraParameters::KEY_MAX_EXPOSURE_COMPENSATION, maxEV);
    mParameters.set(CameraParameters::KEY_MIN_EXPOSURE_COMPENSATION, minEV);
    mParameters.set(CameraParameters::KEY_EXPOSURE_COMPENSATION_STEP, "1.0");
    //
    // Preview fps ramge
    if (pfsPrvFpsRanges) {
        // Assume we only support one range, if not, need to modify this
        if (pfsPrvFpsRanges->u4DefaultSelection == FRAME_RATE_RANGE_5_30_FPS) {
            mParameters.set(CameraParameters::KEY_PREVIEW_FPS_RANGE, "5000,30000");
            mParameters.set(CameraParameters::KEY_SUPPORTED_PREVIEW_FPS_RANGE, "(5000,30000)");
        }
    }
    // Video Stabilization (EIS) 
    if((strcmp(mParameters.get(getSupportedValuesSuffix((char *) CameraParameters::KEY_VIDEO_STABILIZATION)), "false,true") == 0)
           ||(strcmp(mParameters.get(getSupportedValuesSuffix((char *) CameraParameters::KEY_VIDEO_STABILIZATION)), "true,false") == 0))
    {
        mParameters.set(CameraParameters::KEY_VIDEO_STABILIZATION_SUPPORTED, CameraParameters::TRUE); 
    }
    else {
        mParameters.set(CameraParameters::KEY_VIDEO_STABILIZATION_SUPPORTED, CameraParameters::FALSE);
    }
    // AE, AWB lock 
    mHalCam3ASupportFeature_t r3AFeature; 
    memset(&r3AFeature, 0, sizeof(r3AFeature)); 
    status = ::mHalIoctl(mmHalFd,MHAL_IOCTL_GET_3A_SUPPORT_FEATURE, NULL, 0, &r3AFeature, sizeof(mHalCam3ASupportFeature_t), &u4NumReturned);    
    //AE, AWB Lock 
#if 0 
#warning "[FIXME] should query from HAL, not fix value <Sean>"
    mParameters.set(CameraParameters::KEY_AUTO_EXPOSURE_LOCK_SUPPORTED, CameraParameters::FALSE);        //Android 4.0 
    mParameters.set(CameraParameters::KEY_AUTO_WHITEBALANCE_LOCK_SUPPORTED, CameraParameters::FALSE);    //Android 4.0 
#else 
    mParameters.set(CameraParameters::KEY_AUTO_EXPOSURE_LOCK_SUPPORTED, r3AFeature.u4ExposureLock ? 
                                    CameraParameters::TRUE : CameraParameters::FALSE);        //Android 4.0 
    mParameters.set(CameraParameters::KEY_AUTO_WHITEBALANCE_LOCK_SUPPORTED, r3AFeature.u4AutoWhiteBalanceLock ? 
                                    CameraParameters::TRUE : CameraParameters::FALSE);    //Android 4.0 
#endif 
    // AE, AF areas
    mParameters.set(CameraParameters::KEY_MAX_NUM_FOCUS_AREAS, r3AFeature.u4FocusAreaNum); 
    mParameters.set(CameraParameters::KEY_MAX_NUM_METERING_AREAS, r3AFeature.u4MeterAreaNum); 
  
    // For video test use 
    //mParameters.set("preview-size-values", "176x144,320x240,352x288,480x320,480x368,640x480,720x480,864x480,960x540"); 
    //mParameters.set("preview-size-values", "320x240,352x288,704x576,640x480,720x480,960x720,960x540");

}


/******************************************************************************
*
*******************************************************************************/
void
MTKCameraHardware::
updateShotMode(String8 const& rs8ShotMode)
{
    struct Map : public DefaultKeyedVector<String8, uint32_t>
    {
        Map()
            : DefaultKeyedVector<String8, uint32_t>(MHAL_CAM_CAP_MODE_NORMAL)
        {
            clear();
            //
            add(String8(MtkCameraParameters::CAPTURE_MODE_NORMAL),              MHAL_CAM_CAP_MODE_NORMAL);
            add(String8(MtkCameraParameters::CAPTURE_MODE_BEST_SHOT),           MHAL_CAM_CAP_MODE_BEST_SHOT);
            add(String8(MtkCameraParameters::CAPTURE_MODE_EV_BRACKET_SHOT),     MHAL_CAM_CAP_MODE_EV_BRACKET);
            add(String8(MtkCameraParameters::CAPTURE_MODE_BURST_SHOT),          MHAL_CAM_CAP_MODE_BURST_SHOT);
            add(String8(MtkCameraParameters::CAPTURE_MODE_SMILE_SHOT),          MHAL_CAM_CAP_MODE_SMILE_SHOT);
            add(String8(MtkCameraParameters::CAPTURE_MODE_AUTO_PANORAMA_SHOT),  MHAL_CAM_CAP_MODE_AUTORAMA);
            add(String8(MtkCameraParameters::CAPTURE_MODE_MAV_SHOT),            MHAL_CAM_CAP_MODE_MAV);
            add(String8(MtkCameraParameters::CAPTURE_MODE_HDR_SHOT),            MHAL_CAM_CAP_MODE_HDR);
            add(String8(MtkCameraParameters::CAPTURE_MODE_ASD_SHOT),            MHAL_CAM_CAP_MODE_ASD);
            add(String8(MtkCameraParameters::CAPTURE_MODE_PANO_3D),             MHAL_CAM_CAP_MODE_PANO_3D);
            add(String8(MtkCameraParameters::CAPTURE_MODE_SINGLE_3D),           MHAL_CAM_CAP_MODE_SINGLE_3D);
            add(String8(MtkCameraParameters::CAPTURE_MODE_FACE_BEAUTY),         MHAL_CAM_CAP_MODE_FACE_BEAUTY);
            add(String8(MtkCameraParameters::CAPTURE_MODE_CONTINUOUS_SHOT),     MHAL_CAM_CAP_MODE_CONTINUOUS_SHOT);
            
        }
        //
        static Map& inst()
        {
            static Map singleton;
            return singleton;
        }
    };
    //
    //
    //  [1] map the new shot mode.
    uint32_t const u4NewShotMode = Map::inst().valueFor(rs8ShotMode);
    bool const isShotModeChanged = (u4NewShotMode != mu4ShotMode);
    MY_LOGI(
        "<shot mode> mode changed:%d, old:%d, new:%d(%s)", 
        isShotModeChanged, mu4ShotMode, u4NewShotMode, rs8ShotMode.string()
    );
    //
    //  [2] update data members.
    mu4ShotMode             = u4NewShotMode;
    mmHalCamParam.u4ShotMode= u4NewShotMode;
    
    if(u4NewShotMode==MHAL_CAM_CAP_MODE_BURST_SHOT)
    {
        mmHalCamParam.cam3AParam.isBurstShotMode=1;
    }
    else
    {		
        mmHalCamParam.cam3AParam.isBurstShotMode=0;
    }
    //
    //  [3] check to see if shot mode is changed.
    if  ( isShotModeChanged )
    {
        //  set shot modt to mhal
        status_t status = ::mHalIoctl(
                                mmHalFd, MHAL_IOCTL_SET_SHOT_MODE, 
                                (void*)u4NewShotMode, 0, NULL, 0, NULL
                            );
        if  (NO_ERROR != status) {
            MY_LOGE("MHAL_IOCTL_SET_SHOT_MODE fail - status(%d)", status);
        }
    }
}


/*******************************************************************************
*
********************************************************************************/
void 
MTKCameraHardware::
mapCam3AParameter(mhalCam3AParam_t& cam3aParam, const MtkCameraParameters& params, const MtkCameraParameters& pfeature)
{
    char *pstr, *pstrVals;
    int mode;
    int featureIDAry[] = {FID_AE_STROBE, FID_SCENE_MODE, FID_AE_FLICKER, FID_AF_MODE,
                          FID_AF_METERING, FID_AE_METERING, FID_AE_EV, FID_AWB_MODE,
                          FID_AE_ISO, FID_COLOR_EFFECT, FID_ISP_BRIGHT, FID_ISP_HUE,
                          FID_ISP_SAT, FID_ISP_EDGE, FID_ISP_CONTRAST, FID_AE_SCENE_MODE};
    int featureCnt = sizeof(featureIDAry) / 4;
    int i;
    int *pmode = &cam3aParam.strobeMode;
    //
    //default off for strobe mode
    cam3aParam.strobeMode = FLASHLIGHT_FORCE_OFF;
    //
    for (i = 0; i < featureCnt; i++, pmode++) {
        pstr = getFeatureIDToStr(featureIDAry[i]);
        pstrVals = getSupportedValuesSuffix(pstr);
        if (params.get(pstr) == NULL) {
            continue;
        }
        if (params.get(pstrVals) == NULL) {
            continue;
        }
        mode = getSupportedValueToEnum(params.get(pstr), params.get(pstrVals), pfeature.get(pstrVals));
        *pmode = mode;
    }
    //
    if (params.get(MtkCameraParameters::KEY_VIDEO_STABILIZATION) != NULL ) {
        if (strcmp(params.get(MtkCameraParameters::KEY_VIDEO_STABILIZATION), CameraParameters::TRUE) == 0) {
            cam3aParam.eisMode = 1;   
        }
        else {
            cam3aParam.eisMode = 0; 
        }
    }
    CAM_LOGD(" EIS mode: %d\n", cam3aParam.eisMode); 
    //
    if (params.get(MtkCameraParameters::KEY_AF_LAMP_MODE) != NULL ) {
        if (strcmp(params.get(MtkCameraParameters::KEY_AF_LAMP_MODE), "flash") == 0) {
            cam3aParam.afLampMode = 3;   
        }       
        else if (strcmp(params.get(MtkCameraParameters::KEY_AF_LAMP_MODE), "on") == 0) {
            cam3aParam.afLampMode = 1;   
        }
        else if (strcmp(params.get(MtkCameraParameters::KEY_AF_LAMP_MODE), "auto") == 0) {
            cam3aParam.afLampMode = 2; 
        }
        else {
            cam3aParam.afLampMode = 0; 
        }
    }

    cam3aParam.afLampMode = 3;    // Force to use strobe menu now

    CAM_LOGD(" AF Lamp mode: %d\n", cam3aParam.afLampMode);     
    //
    // exposure for 2.2
    pstr = getFeatureIDToStr(FID_AE_EV);
    pstrVals = getSupportedValuesSuffix(pstr);
    if ((params.get(CameraParameters::KEY_EXPOSURE_COMPENSATION) != NULL) && (params.get(pstrVals) != NULL)) {
        mode = getSupportedValueToEnum(params.get(CameraParameters::KEY_EXPOSURE_COMPENSATION), params.get(pstrVals), pfeature.get(pstrVals));
        cam3aParam.aeExpMode = mode;
    }
    //
    // Prv frame rate
    cam3aParam.prvFps = params.getPreviewFrameRate();
    //
    // Capture mode
#warning "[TODO] it is better to set AE mode in mHalCam than CameraHal"
    switch (mu4ShotMode)
    {
    case MHAL_CAM_CAP_MODE_EV_BRACKET:
        cam3aParam.aeMode = AE_MODE_BRACKET_AE;
        CAM_LOGD("[mapCam3AParameter] AE mode: AE_MODE_BRACKET_AE");
        break;
    case MHAL_CAM_CAP_MODE_HDR:
        cam3aParam.aeMode = AE_MODE_HDR;
        CAM_LOGD("[mapCam3AParameter] AE mode: AE_MODE_HDR");
        break;
    case MHAL_CAM_CAP_MODE_AUTORAMA:
        cam3aParam.aeMode = AE_MODE_AUTO_PANORAMA;
        CAM_LOGD("[mapCam3AParameter] AE mode: AE_MODE_AUTO_PANORAMA");
        break;        
    default:
        CAM_LOGD("[mapCam3AParameter] AE mode: %d", cam3aParam.aeMode);
        break;
    }
    // if the PanoIdx is 2nd, 3rd, the AF should not draw
    cam3aParam.afIndicator = 0;
    if ( (cam3aParam.afMode == AF_MODE_AFS) || (cam3aParam.afMode == AF_MODE_MACRO) || (cam3aParam.afMode == AF_MODE_AFC)) {
        if (params.getInt(MtkCameraParameters::KEY_PANORAMA_IDX) == 0) {
            cam3aParam.afIndicator = params.getInt(MtkCameraParameters::KEY_FOCUS_DRAW);
        }
    }
    //
    cam3aParam.afX = mParameters.getInt(MtkCameraParameters::KEY_AF_X);
    cam3aParam.afY = mParameters.getInt(MtkCameraParameters::KEY_AF_Y);
    // EIS target w/h 
    mParameters.getPreviewSize(&cam3aParam.eisW, &cam3aParam.eisH);
    // AE lock 
    if (params.get(CameraParameters::KEY_AUTO_EXPOSURE_LOCK) != NULL) {
        cam3aParam.isAELock = strcmp(params.get(CameraParameters::KEY_AUTO_EXPOSURE_LOCK), CameraParameters::TRUE) == 0 ? 1 : 0; 
    }
    // AWB lock 
    if (params.get(CameraParameters::KEY_AUTO_WHITEBALANCE_LOCK) != NULL) {
        cam3aParam.isAWBLock = strcmp(params.get(CameraParameters::KEY_AUTO_WHITEBALANCE_LOCK), CameraParameters::TRUE) == 0 ? 1 :0; 
    }
    // AF Focus Areas 
    if (params.get(CameraParameters::KEY_FOCUS_AREAS) != NULL) {
        CAM_LOGD("[%s] AF Focus Areas:%s \n", __FUNCTION__, params.get(CameraParameters::KEY_FOCUS_AREAS)); 
        //const char pAreaStr [] = "(-100, -120, 300, 400, 1), (-10, -20, 30, 40, 2), (0, 0, 121, 123, 3)"; 
        const char *pAreaStr  = params.get(CameraParameters::KEY_FOCUS_AREAS); 
        List <camera_area_t> focusAreas; 
        const int maxNumFocusAreas = params.getInt(CameraParameters::KEY_MAX_NUM_FOCUS_AREAS);
        parseCamAreas(pAreaStr, strlen(pAreaStr) + 1, focusAreas, maxNumFocusAreas); 
        int index = 0; 
        for (List<camera_area_t>::const_iterator it = (List<camera_area_t>::const_iterator)focusAreas.begin(); it != focusAreas.end(); it++) {
            cam3aParam.focusAreas.areas[index].left = it->left; 
            cam3aParam.focusAreas.areas[index].top = it->top; 
            cam3aParam.focusAreas.areas[index].right = it->right; 
            cam3aParam.focusAreas.areas[index].bottom = it->bottom; 
            cam3aParam.focusAreas.areas[index].weight = it->weight; 
            index++; 
        }        
        cam3aParam.focusAreas.count = focusAreas.size(); 
    }
    // AE Metering Areas 
    if (params.get(CameraParameters::KEY_METERING_AREAS) != NULL) {
        CAM_LOGD("[%s] AE Metering Areas:%s \n", __FUNCTION__, params.get(CameraParameters::KEY_METERING_AREAS)); 
        const char *pAreaStr = params.get(CameraParameters::KEY_METERING_AREAS); 
        List <camera_area_t> meterAreas; 
        const int maxNumMeteringAreas = params.getInt(CameraParameters::KEY_MAX_NUM_METERING_AREAS);
        parseCamAreas(pAreaStr, strlen(pAreaStr) + 1, meterAreas, maxNumMeteringAreas); 
        int index = 0; 
        for (List<camera_area_t>::const_iterator it = (List<camera_area_t>::const_iterator)meterAreas.begin(); it != meterAreas.end(); it++) {
            cam3aParam.meteringAreas.areas[index].left = it->left; 
            cam3aParam.meteringAreas.areas[index].top = it->top; 
            cam3aParam.meteringAreas.areas[index].right = it->right; 
            cam3aParam.meteringAreas.areas[index].bottom = it->bottom; 
            cam3aParam.meteringAreas.areas[index].weight = it->weight; 
            index++;
        }        
        cam3aParam.meteringAreas.count = meterAreas.size(); 
    }

    // Engineer mode
    mapEng3AParameter(cam3aParam, params);	
    

    cam3aParam.isBurstShotMode = mmHalCamParam.cam3AParam.isBurstShotMode;    
}

/*******************************************************************************
*
********************************************************************************/
void 
MTKCameraHardware::
setCam3AParameter(const MtkCameraParameters& params)
{
    status_t status = NO_ERROR;
    mhalCam3AParam_t cam3aParam;
    //
    memset(&cam3aParam, 0, sizeof(mhalCam3AParam_t));
    mapCam3AParameter(cam3aParam, params, mFeatureParam);
    mstrobeMode = cam3aParam.strobeMode;
    //
    // Set 3a parameter
    status = ::mHalIoctl(mmHalFd,MHAL_IOCTL_SET_CAM_3A_PARAMETER, &cam3aParam, sizeof(mhalCam3AParam_t), NULL, 0, NULL);
    if (status != NO_ERROR) {
        CAM_LOGD("[setCam3AParameter] set MHAL_IOCTL_SET_CAM_3A_PARAMETER fail \n");
    }
}
/******************************************************************************
*
*******************************************************************************/
bool MTKCameraHardware::isParameterValid(const char *param, char *supportedParams)
{
    bool ret = true;
    char *pos = NULL;

    if ( NULL == supportedParams )
    {
        CAM_LOGD("Invalid supported parameters string");
        ret = false;
        goto exit;
    }
	
    if ( NULL == param )
    {
        CAM_LOGD("Invalid parameter string");
        ret = false;
        goto exit;
    }

    pos = strstr(supportedParams, param);
    if ( NULL == pos )
    {
        ret = false;
    }
    else
    {
        ret = true;
    }
	
exit:

    return ret;
}
/******************************************************************************
*
*******************************************************************************/
bool MTKCameraHardware::isAreaValid(const char *param, const char *maxNum)
{
    bool ret = true;
    List <camera_area_t> Areas;
    const int maxNumAreas = mParameters.getInt(maxNum);

    if ( NULL == param )
    {
        CAM_LOGD("Invalid supported parameters string");
        ret = false;
        goto exit;
    }

    if (parseCamAreas(param, strlen(param) + 1, Areas, maxNumAreas))
    {
        ret = false;
    }
    else
    {
        ret = true;
    }

exit:

    return ret;	
}
/******************************************************************************
*
*******************************************************************************/
status_t
MTKCameraHardware::
setParameters(const CameraParameters& params)
{
    char *pstrScene;
    char *pstrMode;
    char *pstrPreSize;
    char *pstrArea;

    int changed = 0;

    CAM_LOGD("[setParameters]");
    showParameters(params.flatten());

//-------------------------------------------------------------
    // workaround: for 3rd party set invalid flash mode.
    //             only test 'invalid' parameter.
    char *psupportAfMode = (char *)mParameters.get(CameraParameters::KEY_SUPPORTED_FOCUS_MODES);
    char *psupportFlashMode = (char *)mParameters.get(CameraParameters::KEY_SUPPORTED_FLASH_MODES);
  	if(psupportFlashMode == NULL)// 
	    psupportFlashMode = "off";


    pstrMode = (char *) CameraParameters::KEY_FOCUS_MODE;
    if ((params.get(pstrMode)) != NULL) {
        //if (!isParameterValid(params.get(pstrMode), psupportAfMode)) {
        if ( strcmp(params.get(pstrMode), "invalid") == 0 ) {
            CAM_LOGE("[%s]ERROR: Invalid FOCUS mode = %s", __FUNCTION__, params.get(pstrMode));
            return -EINVAL;
        }
    }
    pstrMode = (char *) CameraParameters::KEY_FLASH_MODE;
    if ((params.get(pstrMode)) != NULL) {
        //if (!isParameterValid(params.get(pstrMode), psupportFlashMode)) {
        if ( strcmp(params.get(pstrMode), "invalid") == 0 ) {
            CAM_LOGE("[%s]ERROR: Invalid FLASH mode = %s", __FUNCTION__, params.get(pstrMode));
            return -EINVAL;
        }
    }   
    //
    pstrPreSize = (char *) CameraParameters::KEY_PREVIEW_SIZE;
    if ((params.get(pstrPreSize)) != NULL) {
        const char *pSizeStr  = params.get(pstrPreSize); 
        status_t ret = NO_ERROR;
        ret = parsePreviewSize(pSizeStr, strlen(pSizeStr) + 1);
     
        if (ret != NO_ERROR) {
            CAM_LOGE("[%s]ERROR: Invalid Preview Size = %s \n", __FUNCTION__, pSizeStr);
            return ret;
        }
    }

//-------------------------------------------------------------
    // Focus Areas 
    pstrArea = (char *) params.get(CameraParameters::KEY_FOCUS_AREAS);
    if (pstrArea != NULL) {
        if(!isAreaValid(pstrArea, CameraParameters::KEY_MAX_NUM_FOCUS_AREAS)) {
            CAM_LOGE("[%s]ERROR: focus areas = %s", __FUNCTION__, pstrArea);
            return -EINVAL;
        }
    }

    // Metering Areas 
    pstrArea = (char *) params.get(CameraParameters::KEY_METERING_AREAS);
    if (pstrArea != NULL) {
        if(!isAreaValid(pstrArea, CameraParameters::KEY_MAX_NUM_METERING_AREAS)) {
            CAM_LOGE("[%s]ERROR: metering areas = %s", __FUNCTION__, pstrArea);
            return -EINVAL;
        }
    }
//-------------------------------------------------------------
	

    //
    pstrScene = (char *) CameraParameters::KEY_SCENE_MODE;
    if (strcmp(params.get(pstrScene), mParameters.get(pstrScene)) != 0) {
        // scene mode changed
        changed = 1;
    }
    // Update new param
    mParameters = params;
    updateShotMode(String8(mParameters.get(MtkCameraParameters::KEY_CAPTURE_MODE)));

    // Update new 3A param
    if (changed == 1) {
        CAM_LOGD("  Scene or Effect changed \n\n");
        int scene, effect;
        char *pstrVals, *pstrEffect;
        //
        pstrVals = getSupportedValuesSuffix(pstrScene);
        scene = getSupportedValueToEnum(params.get(pstrScene), params.get(pstrVals), mFeatureParam.get(pstrVals));
        pstrEffect = (char *) CameraParameters::KEY_EFFECT;
        pstrVals = getSupportedValuesSuffix(pstrEffect);
        effect = getSupportedValueToEnum(params.get(pstrEffect), params.get(pstrVals), mFeatureParam.get(pstrVals));
        setCamFeatureMode(scene, effect);
        //backup frame rate for scene change
        mCurFramerate = params.getPreviewFrameRate();
        CAM_LOGD(" mCurFramerate: %d\n", mCurFramerate); 
        initDefaultParameters();
#warning "[FIXME] initDefaultParameters() will overwrite the parameters from setParameters"
#warning "[FIXME] when scene mode changes, only scene-dependent parameters must be updated; otherwise no change"

        //sync EIS Setting 
        if (NULL != params.get(CameraParameters::KEY_VIDEO_STABILIZATION)) {
            mParameters.set(CameraParameters::KEY_VIDEO_STABILIZATION, params.get(CameraParameters::KEY_VIDEO_STABILIZATION)); 
        }
        //sync PictureSize
        int picWidth = 0, picHeight;
        params.getPictureSize(&picWidth, &picHeight);
        mParameters.setPictureSize(picWidth, picHeight);
        //sync PreviewSize
        int prvWidth = 0, prvHeight;
        params.getPreviewSize(&prvWidth, &prvHeight);
        mParameters.setPreviewSize(prvWidth, prvHeight);
    }
    if (mIsPreviewEnabled) {
        // [Sean] 
        // For ICS Video Zoom peformance, in Video zoom mode, the 
        // zoom setting will replace to startSmootZoom API. 
        if (mParameters.getInt(MtkCameraParameters::KEY_CAMERA_MODE) != 2) {
            mZoomStopValue = mParameters.getInt(CameraParameters::KEY_ZOOM);
            status_t status = doZoom(mZoomValue, mZoomStopValue);
            if (status != NO_ERROR) {
                return status;
            }
        } 
        setCam3AParameter(mParameters);
        // Check pano's size
        if ( strcmp(params.get(MtkCameraParameters::KEY_CAPTURE_MODE), MtkCameraParameters::CAPTURE_MODE_PANORAMA_SHOT) == 0 ) {
            decidePanoMaxDim();
        }
    }
    // FIXME: For fps
    int minFps, maxFps;
    mParameters.getPreviewFpsRange(&minFps, &maxFps);
    if ((minFps < 5000) || (maxFps > 33000)) {
        return BAD_VALUE;
    }
    //
    //set flashlight parameter in both preview/IDLE state
    //ex. for "torch" flashlight control ,3rd party APP may set in IDLD mode
    //TODO:set 3A strobe mode in flashlight control module.
    setCamFlashlightConf(mParameters);
    
    return NO_ERROR;
}

/******************************************************************************
*
*******************************************************************************/
CameraParameters& 
MTKCameraHardware::
getParameters()
{
    return mParameters;
}

/******************************************************************************
*
*******************************************************************************/
void 
MTKCameraHardware::
initDefaultParameters()
{
    MtkCameraParameters mParam;

    //reset mParameters to prevent from previous remaining data
    mParameters = mParam;
    //
    getCamFeatureEnums();
    // preview related
    getCamPreviewSupportFmt(); 
    // Default is normal fps (determined by 3A), (normal,fix)
    //
    if (mCurFramerate != 30){
        mParameters.set(CameraParameters::KEY_PREVIEW_FRAME_RATE, mCurFramerate);
    }
    // FIXME, should get the setting from sensor real properity 
    mParameters.set(CameraParameters::KEY_ROTATION, 0);
    mParameters.set(CameraParameters::KEY_HORIZONTAL_VIEW_ANGLE, 360);
    mParameters.set(CameraParameters::KEY_VERTICAL_VIEW_ANGLE, 360);
    // zoom related
    initZoomParam();    
    // picture related
    mParameters.setPictureFormat(CameraParameters::PIXEL_FORMAT_JPEG);
    mParameters.set(CameraParameters::KEY_SUPPORTED_PICTURE_FORMATS, CameraParameters::PIXEL_FORMAT_JPEG);
    // jpeg related
    mParameters.set(CameraParameters::KEY_JPEG_QUALITY, 85);
    mParameters.set(CameraParameters::KEY_JPEG_THUMBNAIL_WIDTH, 160);
    mParameters.set(CameraParameters::KEY_JPEG_THUMBNAIL_HEIGHT, 120);
    mParameters.set(CameraParameters::KEY_SUPPORTED_JPEG_THUMBNAIL_SIZES, "0x0,160x120,320x240");
    mParameters.set(CameraParameters::KEY_JPEG_THUMBNAIL_QUALITY, 85);  //FIXME: Not link with driver now
    // 3A related
    mParameters.set(CameraParameters::KEY_FOCAL_LENGTH, "3.5"); // Should query from driver
    mParameters.set(CameraParameters::KEY_FOCUS_DISTANCES, "0.95,1.9,Infinity");
    //ICS FD    
    mParameters.set(CameraParameters::KEY_MAX_NUM_DETECTED_FACES_HW, 15);
    mParameters.set(CameraParameters::KEY_MAX_NUM_DETECTED_FACES_SW, 0);
    // ICS Video 
    //mParameters.set(CameraParameters::KEY_PREFERRED_PREVIEW_SIZE_FOR_VIDEO, "960x720"); 
    //mParameters.set(CameraParameters::KEY_SUPPORTED_VIDEO_SIZES, "1280x720,720x1280,640x480,480x640,480x368,368x480,176x144,144x176,320x240");
    //mParameters.set(CameraParameters::KEY_VIDEO_SIZE, "1280x720"); 
    
    //+++++ MTK proprietary parameters begin +++++
    // Camera Mode, default is normal (normal,mtk-prv,mtk-vdo)
    mParameters.set(MtkCameraParameters::KEY_CAMERA_MODE, MtkCameraParameters::CAMERA_MODE_NORMAL);
    // af related
    mParameters.set(MtkCameraParameters::KEY_FOCUS_DRAW, MtkCameraParameters::DISABLE);
    //
    mParameters.set(MtkCameraParameters::KEY_CAPTURE_PATH, "/sdcard/DCIM/cap00");
    mParameters.set(MtkCameraParameters::KEY_BURST_SHOT_NUM, 1);
    // mAtv
    mParameters.set(MtkCameraParameters::KEY_MATV_PREVIEW_DELAY, 240);
    // Panorama
    mParameters.set(MtkCameraParameters::KEY_PANORAMA_IDX, 0);
    mParameters.set(MtkCameraParameters::KEY_PANORAMA_DIR, MtkCameraParameters::PANORAMA_DIR_RIGHT);
    // isp mode
    mParameters.set(MtkCameraParameters::KEY_ISP_MODE, 0);     //0: normal
    // AF 
    mParameters.set(MtkCameraParameters::KEY_AF_X, 0);
    mParameters.set(MtkCameraParameters::KEY_AF_Y, 0);
    // ZSD AP
    if(strcmp(mParameters.get(getSupportedValuesSuffix((char *) MtkCameraParameters::KEY_ZSD_MODE)), "off,off") == 0) {
        mParameters.set(MtkCameraParameters::KEY_ZSD_MODE, MtkCameraParameters::OFF);
        mParameters.set(MtkCameraParameters::KEY_SUPPORTED_ZSD_MODE, CameraParameters::FALSE);
        //CAM_LOGD("[initDefaultParameters][ZSD]: Disable ZSD by custom config \n");
    }
    else if(strcmp(mParameters.get(getSupportedValuesSuffix((char *) MtkCameraParameters::KEY_ZSD_MODE)), "on,on") == 0){
        mParameters.set(MtkCameraParameters::KEY_ZSD_MODE, MtkCameraParameters::ON);
        mParameters.set(MtkCameraParameters::KEY_SUPPORTED_ZSD_MODE, CameraParameters::FALSE);
        CAM_LOGD("[initDefaultParameters][ZSD]: Always Enable ZSD by custom config\n");
    }
    else if((strcmp(mParameters.get(getSupportedValuesSuffix((char *) MtkCameraParameters::KEY_ZSD_MODE)), "on,off") == 0)
           ||(strcmp(mParameters.get(getSupportedValuesSuffix((char *) MtkCameraParameters::KEY_ZSD_MODE)), "off,on") == 0)){
        mParameters.set(MtkCameraParameters::KEY_SUPPORTED_ZSD_MODE, CameraParameters::TRUE);
        CAM_LOGD("[initDefaultParameters][ZSD]: Enable ZSD by AP config\n");
    }

    // ENGINEER MODE
    defaultEngParameter(mParameters);
	
    //----- MTK proprietary parameters end -----

    showParameters(mParameters.flatten());
}


/******************************************************************************
*
*******************************************************************************/
void 
MTKCameraHardware::
setCamFlashlightConf(const MtkCameraParameters& params)
{
    status_t status = NO_ERROR;
    MINT32 strobeMode = mstrobeMode; //get last strobeMode updated by mapCam3AParameter() in preview state
    //
    if (params.get(CameraParameters::KEY_FLASH_MODE)) {
        //TODO: handle all cases eventually
        if ( strcmp(params.get(CameraParameters::KEY_FLASH_MODE), CameraParameters::FLASH_MODE_TORCH) == 0 ) {
            CAM_LOGD("[setCamFlashlightConf]: torch flashlight AE_STROBE_MODE_FORCE_TORCH \n");
            //FIXME: create an official control path for torch light
            strobeMode = 555;//AE_STROBE_MODE_FORCE_TORCH;
        }
        //
        status = ::mHalIoctl(mmHalFd,MHAL_IOCTL_SET_FLASHLIGHT_PARAMETER, &strobeMode, sizeof(MINT32), NULL, 0, NULL);
        if (status != NO_ERROR) {
            CAM_LOGD("[setCamFlashlightConf] set MHAL_IOCTL_SET_FLASHLIGHT_PARAMETER fail \n");
        }
    }
}

/******************************************************************************
*
*******************************************************************************/
void 
MTKCameraHardware::
getFocusWindowParam(MtkCameraParameters& params) const
{
#if 0
#warning "[Remove Me] should be replaced with ICS method"
    status_t status = NO_ERROR;

    if (mIsPreviewEnabled) {
        int focusPos[9 * 5];  // Maximum 9 windows, each is x0,y0,x1,y1,state
        memset(focusPos, -1, sizeof(focusPos));
        status = ::mHalIoctl(mmHalFd,MHAL_IOCTL_GET_FOCUS_WINDOW, NULL, 0, &focusPos, sizeof(focusPos), NULL);
        if (status == NO_ERROR) {
            char data[128];
            char *pbuf = data;
            int *ppos = focusPos;
            int len;
            pbuf[0] = 0;
            for (int i = 0; i < 9; i++) {
                if (ppos[0] == -1) {
                    break;
                }
                sprintf(pbuf, "%dx%dx%dx%dx%d,", ppos[0], ppos[1], ppos[2], ppos[3], ppos[4]);
                pbuf += strlen(pbuf);
                ppos += 5;
            }
            if ((len = strlen(data)) != 0) {
                data[len - 1] = 0; // Remove last ','
            }
            params.set("focus-win", data);
            CAM_LOGD("[getFocusWindowParam] %s \n", data);
        }
        else {
            CAM_LOGD("[getFocusWindowParam], MHAL_IOCTL_GET_FOCUS_WINDOW err \n");
        }
    }
#endif
}

/******************************************************************************
*
*******************************************************************************/
void 
MTKCameraHardware::
getCamPreviewSupportFmt()
{
    status_t status;
    MUINT32 formats[2] = {0};   // formats[0], support format, formats[1] default formats 
    MUINT32 retLen = 0; 

    status = ::mHalIoctl(mmHalFd,MHAL_IOCTL_GET_BUF_SUPPORT_FORMAT, NULL, 0, &formats, 2 * sizeof(MINT32), &retLen);
    CAM_LOGD("[getCamPreviewSupportFmt] support format = 0x%x, default format = 0x%x\n", formats[0], formats[1]);
    //
    #warning "[FIXME] supported preview format I420 should be obtained from mHalCam"
    String8 supportedPreviewFormats = String8(CameraParameters::PIXEL_FORMAT_YUV420SP)
                                    + ","
                                    + String8(CameraParameters::PIXEL_FORMAT_YUV420P)
                                    + ","
                                    + String8(MtkCameraParameters::PIXEL_FORMAT_YUV420I);
    //
    mParameters.set(CameraParameters::KEY_SUPPORTED_PREVIEW_FORMATS, supportedPreviewFormats);
    mParameters.setPreviewFormat(CameraParameters::PIXEL_FORMAT_YUV420SP);  //@ Default preview support format NV21 (Yuv420sp)
    //
    switch (formats[1]) {
//        case MHAL_CAM_BUF_FMT_MTKYUV:
//            mParameters.set(MtkCameraParameters::KEY_PREVIEW_INT_FORMAT, "mtkyuv");       
//            break; 
        case ePIXEL_FORMAT_NV21:
            mParameters.set(MtkCameraParameters::KEY_PREVIEW_INT_FORMAT, CameraParameters::PIXEL_FORMAT_YUV420SP);
            break;
        case ePIXEL_FORMAT_YV12:
            mParameters.set(MtkCameraParameters::KEY_PREVIEW_INT_FORMAT, CameraParameters::PIXEL_FORMAT_YUV420P); 
            break;   
        case ePIXEL_FORMAT_I420:
            mParameters.set(MtkCameraParameters::KEY_PREVIEW_INT_FORMAT, MtkCameraParameters::PIXEL_FORMAT_YUV420I);
            break;        
        default:
            break;             
    }
}

/******************************************************************************
*
*******************************************************************************/
status_t 
MTKCameraHardware::
parsePreviewSize(const char *sizeStr, int sizeLength)
{
    status_t ret = NO_ERROR;
    char *ctx;
    char *pSize = NULL;
    char *pStart = NULL;
    char *pEnd = NULL;
    const char sep = 'x';
    MINT32 height = 0;
    MINT32 width = 0;

    //CAM_LOGV("[%s] E ", __FUNCTION__); 

    if ( ( NULL == sizeStr ) || ( 0 >= sizeLength ) ) {
        return -EINVAL;
    }

    pSize = ( char * ) malloc(sizeLength);
    if ( NULL == pSize ) {
        return -ENOMEM;
    }

    memcpy(pSize, sizeStr, sizeLength);

    pStart = pSize;
    // width 
    if ( NULL == pStart ) {
        CAM_LOGE("[%s] Parsing of the width failed!", __FUNCTION__);
        ret = -EINVAL;
    }
    else {
        width = static_cast<MINT32>(strtol(pStart, &pEnd, 10));
    }
    // height 
    if ( sep != *pEnd ) {
        CAM_LOGE("[%s] Parsing of the height failed!", __FUNCTION__);
        ret = -EINVAL;
    }
    else {
        height = static_cast<MINT32>(strtol(pEnd+1, &pEnd, 10));
    }

    if ((width < 0) || (height < 0)) {
        CAM_LOGE("preview size error!");
        ret = -EINVAL;
    }

    if ( NULL != pSize ) {
        free(pSize);
    }

    //CAM_LOGV("[%s] X ",  __FUNCTION__); 

    return ret;
}

/******************************************************************************
*
*******************************************************************************/
status_t 
MTKCameraHardware::
parseCamAreas(const char *areaStr, int areaLength, List<camera_area_t> &areas, const int maxnumArea)
{
    status_t ret = NO_ERROR;
    char *ctx;
    char *pArea = NULL;
    char *pStart = NULL;
    char *pEnd = NULL;
    const char *startToken = "(";
    const char endToken = ')';
    const char sep = ',';
    ssize_t top, left, bottom, right, weight;
    char *tmpBuffer = NULL;
    camera_area_t currentArea; 
    int numArea = 0;

    //CAM_LOGV("[%s] E ", __FUNCTION__); 

    if ( ( NULL == areaStr ) || ( 0 >= areaLength ) ) {
        return -EINVAL;
    }

    tmpBuffer = ( char * ) malloc(areaLength);
    if ( NULL == tmpBuffer ) {
        return -ENOMEM;
    }

    memcpy(tmpBuffer, areaStr, areaLength);

    pArea = strtok_r(tmpBuffer, startToken, &ctx);

    do {
        pStart = pArea;
        // left 
        if ( NULL == pStart ) {
            CAM_LOGE("Parsing of the left area coordinate failed!");
            ret = -EINVAL;
            break;
        }
        else {
            currentArea.left = static_cast<MINT32>(strtol(pStart, &pEnd, 10));
        }
        // top 
        if ( sep != *pEnd ) {
            CAM_LOGE("Parsing of the top area coordinate failed!");
            ret = -EINVAL;
            break;
        }
        else {
            currentArea.top = static_cast<MINT32>(strtol(pEnd+1, &pEnd, 10));
        }
        // right 
        if ( sep != *pEnd ) {
            CAM_LOGE("Parsing of the right area coordinate failed!");
            ret = -EINVAL;
            break;
        }
        else {
            currentArea.right = static_cast<MINT32>(strtol(pEnd+1, &pEnd, 10));
        }
        // bottom 
        if ( sep != *pEnd ) {
            CAM_LOGE("Parsing of the bottom area coordinate failed!");
            ret = -EINVAL;
            break;
        }
        else {
            currentArea.bottom = static_cast<MINT32>(strtol(pEnd+1, &pEnd, 10));
        }
        // weight 
        if ( sep != *pEnd ) {
            CAM_LOGE("Parsing of the weight area coordinate failed!");
            ret = -EINVAL;
            break;
        }
        else {
            currentArea.weight = static_cast<MINT32>(strtol(pEnd+1, &pEnd, 10));
        }

        if ( endToken != *pEnd ) {
            CAM_LOGE("Malformed area!");
            ret = -EINVAL;
            break;
        }

        if (!checkCamArea(currentArea) ) {
            CAM_LOGE("Error area!");
            ret = -EINVAL;
            break; 
        }

        numArea++;
        if (numArea > maxnumArea) {
            CAM_LOGE("Error count of area! ");
            ret = -EINVAL;
            break; 
        }
        areas.push_back(currentArea);
        pArea = strtok_r(NULL, startToken, &ctx);

    }
    while ( NULL != pArea );

    if ( NULL != tmpBuffer ) {
        free(tmpBuffer);
    }
    //CAM_LOGV("[%s] X ",  __FUNCTION__); 

    return ret;
}

/******************************************************************************
*
*******************************************************************************/
#define CAM_AREA_LEFT                    (-1000)
#define CAM_AREA_TOP                     (-1000)
#define CAM_AREA_RIGHT                   (1000)
#define CAM_AREA_BOTTOM                (1000)
#define CAM_AREA_WEIGHT_MIN         (1)
#define CAM_AREA_WEIGHT_MAX        (1000)
bool 
MTKCameraHardware::
checkCamArea(camera_area_t const &area)
{
    //Handles the invalid regin corner case.
    if ( ( 0 == area.top ) && ( 0 == area.left ) && ( 0 == area.bottom ) && ( 0 == area.right ) && ( 0 == area.weight ) ) {
        return true;
    }

    if ( ( CAM_AREA_WEIGHT_MIN > area.weight ) ||  ( CAM_AREA_WEIGHT_MAX < area.weight ) ) {
        CAM_LOGE("Camera area weight is invalid %d", area.weight);
        return false;
    }

    if ( ( CAM_AREA_TOP > area.top ) || ( CAM_AREA_BOTTOM < area.top ) ) {
        CAM_LOGE("Camera area top coordinate is invalid %d", area.top );
        return false;
    }

    if ( ( CAM_AREA_TOP > area.bottom ) || ( CAM_AREA_BOTTOM < area.bottom ) ) {
        CAM_LOGE("Camera area bottom coordinate is invalid %d", area.bottom );
        return false;
    }

    if ( ( CAM_AREA_LEFT > area.left ) || ( CAM_AREA_RIGHT < area.left ) ) {
        CAM_LOGE("Camera area left coordinate is invalid %d", area.left );
        return false;
    }

    if ( ( CAM_AREA_LEFT > area.right ) || ( CAM_AREA_RIGHT < area.right ) ) {
        CAM_LOGE("Camera area right coordinate is invalid %d", area.right );
        return false;
    }

    if ( area.left >= area.right ) {
        CAM_LOGE("Camera area left larger than right");
        return false;
    }

    if ( area.top >= area.bottom ) {
        CAM_LOGE("Camera area top larger than bottom");
        return false;
    }
    return true;
}



