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
#include <cutils/atomic.h>
#include <cutils/properties.h>
//
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/prctl.h>
#include <fcntl.h>
#include <pthread.h>
#include <linux/rtpm_prio.h>
#include <sched.h>
#include <unistd.h>
#include <sys/resource.h>
//
#include "Utils/inc/CamUtils.h"
using namespace android;
using namespace MtkCamUtils;
//
#include "inc/ICamAdapter.h"
#include "inc/BaseCamAdapter.h"
#include "inc/mHalBaseAdapter.h"
#include "inc/mHalCamAdapter.h"
#include "mHalCamCmdQueThread.h"
#include "./mHalCamImage.h" 
#include "./mHalCamUtils.h"
//
#include "ClientCallback/inc/EventCallback.h"
//
#include <camera_feature.h>
using namespace NSFeature;
//
#include <mhal/inc/MediaHal.h>
#include <mhal/inc/camera.h>
#include <mhal/inc/camera/ioctl.h>
using namespace NSCamera;


#define mHalIoCtrl "[Warning] Please replace mHalIoCtrl with mHalIoctl"
#define MDP_NEW_PATH_FOR_ZSD

/******************************************************************************
*
*******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("(%s)[mHalCamAdapter::%s] "fmt, getName(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%s)[mHalCamAdapter::%s] "fmt, getName(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("(%s)[mHalCamAdapter::%s] "fmt, getName(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%s)[mHalCamAdapter::%s] "fmt, getName(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%s)[mHalCamAdapter::%s] "fmt, getName(), __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, arg...)    if (cond) { MY_LOGV(arg); }
#define MY_LOGD_IF(cond, arg...)    if (cond) { MY_LOGD(arg); }
#define MY_LOGI_IF(cond, arg...)    if (cond) { MY_LOGI(arg); }
#define MY_LOGW_IF(cond, arg...)    if (cond) { MY_LOGW(arg); }
#define MY_LOGE_IF(cond, arg...)    if (cond) { MY_LOGE(arg); }


/*******************************************************************************
*
********************************************************************************/
#define MTKCAM_CAP_PROFILE_OPT      1
#define MTKCAM_VDO_PROFILE_OPT      0
#define FILE_SAVE_WAIT              (1)

namespace NSCamCustom
{
extern MUINT32 isSupportJpegOrientation();
}


/******************************************************************************
*
*******************************************************************************/
namespace android {


/******************************************************************************
*
*******************************************************************************/
MTKCameraHardware::MTKCameraHardware(String8 const& rName, int32_t const i4OpenId, CamDevInfo const& rDevInfo)
    : mHalBaseAdapter(rName, i4OpenId, rDevInfo),
    //
    mu4ShotMode(MHAL_CAM_CAP_MODE_UNKNOWN),
    //
    meCamState(eCS_Init),
    mStateLock(),
    mStateCond(),
    //
    mmHalCamParam(),
    mIsPreviewEnabled(false),
    mIsVideoEnabled(false),
    //
    mZoomValue(0),
    mZoomStopValue(0),
    mIsSmoothZoom(false),
    mIsSmoothZoomCBEnabled(false),
    //
    mIsDisplay(false),
    //
    mIsCancelPicture(false),
    //
    mIsATV(false),
    mIsTakePrv(false),
    mIsATVDelay(false),
    mATVDispBufNo(0),
    mATVStartTime(0),
    mATVbuffers(),
    mAtvDispDelay(0),
    pushindex(0),
    popindex(0),
    //
    mPanoW(0),
    mPanoH(0),
    mPanoNum(0),
    mstrobeMode(-1),
    mCurFramerate(30),
    //
    mIsZSD(0),
    //
    mDispBufNo(0),
    //
    mParameters(),
    mFeatureParam(),
    //
    mPreviewMemPool(),
    mQvMemPool(),
    mJpegMemPool(),
    mVideoMemPool(),
    mPrvRgbMemPool(),
    mPrvJpegMemPool(),
    mPanoMemPool(),
    //
    mi4PreviewWidth(0),
    mi4PreviewHeight(0),
    mSemPrv(),
    mbPreviewThreadAlive(false), 
    mPreviewThreadHandle(0),
    //
    maVdoCbBufNo(),
    maVdoTimStampNs(),
    miVdoInIndex(0),
    miVdoOutIndex(0),
    mSemVdo(),
    mbVideoThreadAlive(0), 
    mVideoThreadHandle(0),
    //
    mpMainThread(NULL),
    //   
    mEventCallback(NULL),    
    //
    mSemTakePicBack(),
    //
    mIsAFMoveCallback(0), 
    //
    mFocusStartTime(0),
    mCaptureStartTime(0),
    mFocusCallbackTime(0),
    mShutterCallbackTime(0),
    mRawCallbackTime(0),
    mJpegCallbackTime(0)
{
    ::memset(&mmHalCamParam, 0, sizeof(mhalCamParam_t));
    //
    ::sem_init(&mSemPrv, 0, 0);
    ::sem_init(&mSemVdo, 0, 0);
    ::sem_init(&mSemTakePicBack, 0, 0);
}

/******************************************************************************
*
*******************************************************************************/
MTKCameraHardware::~MTKCameraHardware()
{
}


/******************************************************************************
*
*******************************************************************************/
void
MTKCameraHardware::
mhalCallback(void* param)
{
    mHalCamCBInfo*const pCBInfo = reinterpret_cast<mHalCamCBInfo*>(param);
    if  ( ! pCBInfo ) {
        CAM_LOGW("[mHalCamAdapter::mhalCallback] NULL pCBInfo");
        return;
    }
    //
    mHalCamAdapter*const pmHalCamAdapter = reinterpret_cast<mHalCamAdapter*>(pCBInfo->mCookie);
    if  ( ! pmHalCamAdapter ) {
        CAM_LOGW("[mHalCamAdapter::mhalCallback] NULL pmHalCamAdapter");
        return;
    }
    //
    pmHalCamAdapter->dispatchCallback(pCBInfo);
}


/******************************************************************************
*
*******************************************************************************/
void
MTKCameraHardware::
shotCallback(void* param)
{
    mHalCamCBInfo*const pCBInfo = reinterpret_cast<mHalCamCBInfo*>(param);
    if  ( ! pCBInfo ) {
        CAM_LOGW("[mHalCamAdapter::shotCallback] NULL pCBInfo");
        return;
    }
    //
    mHalCamAdapter*const pmHalCamAdapter = reinterpret_cast<mHalCamAdapter*>(pCBInfo->mCookie);
    if  ( ! pmHalCamAdapter ) {
        CAM_LOGW("[mHalCamAdapter::shotCallback] NULL pmHalCamAdapter");
        return;
    }
    //
    pmHalCamAdapter->dispatchCallback(pCBInfo);
}


/******************************************************************************
*
*******************************************************************************/
void
MTKCameraHardware::
dispatchCallback(void*const param)
{
    mHalCamCBInfo*const pinfo = reinterpret_cast<mHalCamCBInfo*>(param);
    //
    MUINT32 const   u4Type  = pinfo->mType;
    void*const      pvData  = pinfo->mpData;
    MUINT32 const   u4Size  = pinfo->mDataSize;
    //
    //
    switch (u4Type) {
    case MHAL_CAM_CB_ATV_DISP:
        if (mIsATV && !mIsATVDelay) {
            sem_post(&mSemPrv);
            //trick log mATVbuffers[pushindex-1], add (pushindex-1+12)%12
            CAM_LOGD("  mATVpreviewCallback : in %d , out %d\n", mATVbuffers[(pushindex+11)%12],mATVbuffers[popindex-1]);
            if (popindex >= (int)mPreviewMemPool->getBufCount()) {
                popindex -= (int)mPreviewMemPool->getBufCount();
            }
        }
        break;
    case MHAL_CAM_CB_PREVIEW:
        // preview callback
        previewCallback(pvData);
        break;
    case MHAL_CAM_CB_VIDEO_RECORD:
        // video callback 
        videoRecordCallback(pvData); 
        break; 
    case MHAL_CAM_CB_AF:
        if (mMsgEnabled & CAMERA_MSG_FOCUS) {
            CAM_LOGD("[dispatchCallback] MHAL_CAM_CB_AF \n");
            #if (MTKCAM_CAP_PROFILE_OPT)
            mFocusCallbackTime = getTimeMs();
            #endif
            // focus callback      
            MINT32 *pbuf = (MINT32 *) pvData;            
            MBOOL isFocused = (MBOOL)pbuf[0];
            uint32_t const u4AfDataSize = pbuf[1];
            uint32_t const u4DataSize = u4AfDataSize + sizeof(uint32_t)*(1);
    
            camera_memory_t mem;
            if  ( mpBufProvider->allocBuffer(mem, u4DataSize, 1) )
            {
                if  ( mem.data && u4DataSize <= mem.size )
                {
                    uint32_t*const pCBData = reinterpret_cast<uint32_t*>(mem.data);
                    pCBData[0] = MTK_CAMERA_MSG_EXT_DATA_AF;
                    for(uint_t i = 0; i < u4AfDataSize/4; ++i)
                    {
                        pCBData[i+1] = reinterpret_cast<uint32_t*>(pbuf)[i+2];
                    }
                        
                    mDataCb(MTK_CAMERA_MSG_EXT_DATA, &mem, 0, NULL, mCallbackCookie);
                }
                mpBufProvider->freeBuffer(mem);
            }
            mNotifyCb(CAMERA_MSG_FOCUS, isFocused, 0, mCallbackCookie);
        }
        break;
    case MHAL_CAM_CB_AF_MOVE:
        {
            // focus callback      
            MUINT32  isFocusMoving = *(MUINT32 *) pvData;            
            if (mIsAFMoveCallback) 
            {
                mNotifyCb(CAMERA_MSG_FOCUS_MOVE, isFocusMoving, 0, mCallbackCookie); 
            }       
        }
        break;
    case MHAL_CAM_CB_ZOOM:
        {
            mhalZsdParam_t* pzsd = (mhalZsdParam_t*)pvData;
            if ( (0 !=(mMsgEnabled & CAMERA_MSG_ZOOM)) && (mIsSmoothZoomCBEnabled) ) {
                CAM_LOGD("[dispatchCallback] MHAL_CAM_CB_ZOOM \n");
                mNotifyCb(CAMERA_MSG_ZOOM, mZoomValue, mZoomValue == mZoomStopValue, mCallbackCookie);

                //  The final zoom.
                if ( mZoomValue == mZoomStopValue ){
                    mIsSmoothZoomCBEnabled = false;
                    mIsSmoothZoom = false;
                }
            }
            if (pzsd->u4ZsdEnable == 0x1){
                mmHalCamParam.camZsdParam.u4ZsdZoomWidth = pzsd->u4ZsdZoomWidth;
                mmHalCamParam.camZsdParam.u4ZsdZoomHeigth = pzsd->u4ZsdZoomHeigth;
            }
        }
        break;
    //
    case MHAL_CAM_CB_SHUTTER:
        if (mMsgEnabled & CAMERA_MSG_SHUTTER) {
            CAM_LOGD("[dispatchCallback] MHAL_CAM_CB_SHUTTER");
            #if (MTKCAM_CAP_PROFILE_OPT)
            mShutterCallbackTime = getTimeMs();
            #endif
            //
            switch (mu4ShotMode) {
            case MHAL_CAM_CAP_MODE_EV_BRACKET:
            case MHAL_CAM_CAP_MODE_BURST_SHOT:
                mNotifyCb(
                    MTK_CAMERA_MSG_EXT_NOTIFY, MTK_CAMERA_MSG_EXT_NOTIFY_BURST_SHUTTER, 
                    ( mmHalCamParam.u4BusrtCnt - 1 - mmHalCamParam.u4BusrtNo ), 
                    mCallbackCookie
                );
                break;
            //
            case MHAL_CAM_CAP_MODE_CONTINUOUS_SHOT:
                mNotifyCb(
                    MTK_CAMERA_MSG_EXT_NOTIFY, MTK_CAMERA_MSG_EXT_NOTIFY_CONTINUOUS_SHUTTER, 
                    mmHalCamParam.u4BusrtNo+1, 
                    mCallbackCookie
                );
                break;
            default:
                mNotifyCb(CAMERA_MSG_SHUTTER, 0, 0, mCallbackCookie);
                break;
            }
        }
        break;
    //
    case MHAL_CAM_CB_RAW:
        CAM_LOGD("[dispatchCallback] MHAL_CAM_CB_RAW - (BusrtCnt,BusrtNo)=(%d,%d)", mmHalCamParam.u4BusrtCnt, mmHalCamParam.u4BusrtNo);
        #if (MTKCAM_CAP_PROFILE_OPT)
        mRawCallbackTime = getTimeMs();
        #endif
        //
        if  ( 1 + mmHalCamParam.u4BusrtNo == mmHalCamParam.u4BusrtCnt )
        {
            if  ( msgTypeEnabled(CAMERA_MSG_RAW_IMAGE_NOTIFY) ) {
                CAM_LOGD("CAMERA_MSG_RAW_IMAGE_NOTIFY");
                mNotifyCb(CAMERA_MSG_RAW_IMAGE_NOTIFY, 0, 0, mCallbackCookie);
            }
            else if  ( msgTypeEnabled(CAMERA_MSG_RAW_IMAGE) ) {
                CAM_LOGD("CAMERA_MSG_RAW_IMAGE");
                camera_memory_t dummyRaw;
                if  ( mpBufProvider->allocBuffer(dummyRaw, 1, 1) )
                {
                    mDataCb(CAMERA_MSG_RAW_IMAGE, &dummyRaw, 0, NULL, mCallbackCookie);
                    mpBufProvider->freeBuffer(dummyRaw);
                }
            }
        }
        break;
    //
    case MHAL_CAM_CB_POSTVIEW:
        CAM_LOGD("[dispatchCallback] MHAL_CAM_CB_POSTVIEW");
        postBuffer(mQvMemPool, 0, 0, CAMERA_MSG_POSTVIEW_FRAME);
        break;
    //
    case MHAL_CAM_CB_JPEG:
        //
        if (mmHalCamParam.u4IsDumpRaw) {
            mhalCamRawImageInfo_t rawInfo;
            memset(&rawInfo, 0, sizeof(mhalCamRawImageInfo_t));
            MINT32 mode = 0;
            if (mParameters.getInt(MtkCameraParameters::KEY_RAW_SAVE_MODE) == 1) {//preview
                mode = 1;
            }
            LOGD("[EM] mode: %d", mode);
            ::mHalIoctl(mmHalFd, MHAL_IOCTL_GET_RAW_IMAGE_INFO, &mode, sizeof(int), &rawInfo, sizeof(mhalCamRawImageInfo_t), NULL);
            saveUnpackRaw((char *)mmHalCamParam.u1FileName, 
                                   rawInfo.u4Width, 
                                   rawInfo.u4Height, 
                                   rawInfo.u4BitDepth, 
                                   rawInfo.u1Order, 
                                   rawInfo.u4Size, 
                                   rawInfo.u4IsPacked
                                 );
        }
        //
        if  ( msgTypeEnabled(CAMERA_MSG_COMPRESSED_IMAGE) ) {
            //
            uint32_t const jpegSize = u4Size;
            //
            CAM_LOGD("[dispatchCallback] MHAL_CAM_CB_JPEG - size(%d)", jpegSize);
            #if (MTKCAM_CAP_PROFILE_OPT)
            mJpegCallbackTime = getTimeMs();
            MY_LOGD("MCamHw-pn", "***Callback time, %8lld, %8lld, %8lld, %8lld, %8lld, %8lld",
                mFocusStartTime, mFocusCallbackTime, mCaptureStartTime, mShutterCallbackTime,
                mRawCallbackTime, mJpegCallbackTime );
            CAM_LOGD("***Callback time, %8lld, %8lld, %8lld, %8lld, %8lld ",
                mFocusCallbackTime - mFocusStartTime, mCaptureStartTime - mFocusCallbackTime,
                mShutterCallbackTime - mCaptureStartTime, mRawCallbackTime - mShutterCallbackTime,
                mJpegCallbackTime - mRawCallbackTime );
            #endif
            /*
             * Notes:
             *  jpegSize and the size of mJpegMemPool are different, so we cannot 
             *  directly invoke a data callback with mJpegMemPool.
             */
            switch  (mu4ShotMode)
            {
            case MHAL_CAM_CAP_MODE_EV_BRACKET:
            case MHAL_CAM_CAP_MODE_BURST_SHOT:
//            case MHAL_CAM_CAP_MODE_NORMAL:
                {
                    uint32_t const u4DataSize = jpegSize + sizeof(uint32_t)*(1+2);
                    camera_memory_t mem;
                    if  ( mpBufProvider->allocBuffer(mem, u4DataSize, 1) )
                    {
                        if  ( mem.data && u4DataSize == mem.size )
                        {
                            uint32_t*const pCBData = reinterpret_cast<uint32_t*>(mem.data);
                            pCBData[0] = MTK_CAMERA_MSG_EXT_DATA_BURST_SHOT;
                            pCBData[1] = mmHalCamParam.u4BusrtCnt;
                            pCBData[2] = ( mmHalCamParam.u4BusrtCnt - 1 - mmHalCamParam.u4BusrtNo );
                            ::memcpy((void*)(&pCBData[3]), (void*)mJpegMemPool->getVirAddr(), jpegSize);
                            //
                            mDataCb(MTK_CAMERA_MSG_EXT_DATA, &mem, 0, NULL, mCallbackCookie);
                        }
                        mpBufProvider->freeBuffer(mem);
                    }
                }
                break;
            case MHAL_CAM_CAP_MODE_CONTINUOUS_SHOT:
                 {
                    uint32_t const u4DataSize = jpegSize + sizeof(uint32_t)*(1+1);
                    camera_memory_t mem;
                    if  ( mpBufProvider->allocBuffer(mem, u4DataSize, 1) )
                    {
                        if  ( mem.data && u4DataSize == mem.size )
                        {
                            uint32_t*const pCBData = reinterpret_cast<uint32_t*>(mem.data);
                            pCBData[0] = MTK_CAMERA_MSG_EXT_DATA_CONTINUOUS_SHOT;
                            pCBData[1] = mmHalCamParam.u4BusrtNo+1;

                            ::memcpy((void*)(&pCBData[2]), (void*)mJpegMemPool->getVirAddr(), jpegSize);
                            //
                            mDataCb(MTK_CAMERA_MSG_EXT_DATA, &mem, 0, NULL, mCallbackCookie);
                        }
                        mpBufProvider->freeBuffer(mem);
                    }
                }
                break;
            default:
                {
                    sp<IMemoryBufferPool> jpegMemPool = allocMem(jpegSize, 1, "jpeg callback");
                    if  ( jpegMemPool != NULL )
                    {
                        ::memcpy((void*)jpegMemPool->getVirAddr(), (void*)mJpegMemPool->getVirAddr(), jpegSize);
                        mDataCb(CAMERA_MSG_COMPRESSED_IMAGE, jpegMemPool->get_camera_memory(), 0, NULL, mCallbackCookie);
                        jpegMemPool = NULL;
                    }
                }
                break;
            }
        }
        break;
        //
    case MHAL_CAM_CB_CAPTURE_DONE:
        CAM_LOGD("[dispatchCallback] MHAL_CAM_CB_CAPTURE_DONE");
        ::sem_post(&mSemTakePicBack);
        CAM_LOGD(" - post mSemTakePicBack\n");
        break;
        //
    case MHAL_CAM_CB_MAV:
        CAM_LOGD("[dispatchCallback] MTK_CAMERA_MSG_EXT_NOTIFY_MAV");
        mNotifyCb(MTK_CAMERA_MSG_EXT_NOTIFY, MTK_CAMERA_MSG_EXT_NOTIFY_MAV, 0, mCallbackCookie);
        break;

    case MHAL_CAM_CB_AUTORAMA:
        {
            CAM_LOGD("[dispatchCallback] MHAL_CAM_CB_AUTORAMA \n");
            //
            uint32_t const u4DataSize = sizeof(uint32_t)*(1+3);
            //
            camera_memory_t mem;
            if  ( mpBufProvider->allocBuffer(mem, u4DataSize, 1) )
            {
                if  ( mem.data && u4DataSize <= mem.size )
                {
                    uint32_t*const pCBData = reinterpret_cast<uint32_t*>(mem.data);
                    pCBData[0] = MTK_CAMERA_MSG_EXT_DATA_AUTORAMA;
                    pCBData[1] = 1;//(MHAL_CAM_CB_AUTORAMA == pcbParam->type);
                    pCBData[2] = reinterpret_cast<uint32_t*>(pvData)[0];
                    pCBData[3] = reinterpret_cast<uint32_t*>(pvData)[1];
                    //
                    if (pCBData[2])
                        mNotifyCb(CAMERA_MSG_SHUTTER, 0, 0, mCallbackCookie);
                    mDataCb(MTK_CAMERA_MSG_EXT_DATA, &mem, 0, NULL, mCallbackCookie);
                }
                mpBufProvider->freeBuffer(mem);
            }
        }
        break;

    case MHAL_CAM_CB_AUTORAMAMV:
        {
            CAM_LOGD("[dispatchCallback] MHAL_CAM_CB_AUTORAMAMV");
            //
            uint32_t const u4DataSize = sizeof(uint32_t)*(1+4);
            //
            camera_memory_t mem;
            if  ( mpBufProvider->allocBuffer(mem, u4DataSize, 1) )
            {
                if  ( mem.data && u4DataSize <= mem.size )
                {
                    uint32_t*const pData = reinterpret_cast<uint32_t*>(pvData);
                    uint32_t*const pCBData = reinterpret_cast<uint32_t*>(mem.data);
                    pCBData[0] = MTK_CAMERA_MSG_EXT_DATA_AUTORAMA;
                    pCBData[1] = 0;//(MHAL_CAM_CB_AUTORAMAMV == pcbParam->type);
                    pCBData[2] = pData[0];
                    pCBData[3] = pData[1];
                    pCBData[4] = pData[2];
                    //
                    mDataCb(MTK_CAMERA_MSG_EXT_DATA, &mem, 0, NULL, mCallbackCookie);
                }
                mpBufProvider->freeBuffer(mem);
            }
        }
        break;

    case MHAL_CAM_CB_SCALADO:{
        uint8_t * addr;
        int capW,capH;

        CAM_LOGD("[dispatchCallback] MHAL_CAM_CB_SCALADO start\n");
        mParameters.getPictureSize(&capW, &capH);
        addr = (uint8_t *)pvData;
        //saveBufToFile((char *) pcapFName, addr, capW*capH*2);
        }  
        CAM_LOGD("[dispatchCallback] MHAL_CAM_CB_SCALADO done\n");
        break;
    case MHAL_CAM_CB_ASD:
        {
            MUINT32 const u4Scene = *reinterpret_cast<MUINT32*>(pvData);
            CAM_LOGD("[dispatchCallback] MTK_CAMERA_MSG_EXT_NOTIFY_ASD:Scene = %d", u4Scene);
            mNotifyCb(MTK_CAMERA_MSG_EXT_NOTIFY, MTK_CAMERA_MSG_EXT_NOTIFY_ASD, u4Scene, mCallbackCookie);
        }
        break;
    case MHAL_CAM_CB_FD:
        {
            if (msgTypeEnabled(CAMERA_MSG_PREVIEW_METADATA)) {
                CAM_LOGD("[dispatchCallback]MHAL_CAM_CB_FD +");
                if (mEventCallback != 0) {
                    mEventCallback->setDataCallback(mDataCb);
                    mEventCallback->setBufferProvider(mpBufProvider);
                    mEventCallback->setDataBuf(
                         CAMERA_MSG_PREVIEW_METADATA,
                         NULL,
                         0,
                         pvData,
                         mCallbackCookie
                    );
                }
            }
        }
        break;
    case MHAL_CAM_CB_SMILE:
        if (msgTypeEnabled(CAMERA_MSG_PREVIEW_METADATA)) {
            CAM_LOGD("[dispatchCallback] MTK_CAMERA_MSG_EXT_NOTIFY_SMILE_DETECT +");
                if (mEventCallback != 0) {
                    mEventCallback->setNotifyCallback(mNotifyCb);
                    mEventCallback->setNotifyBuf(
                    MTK_CAMERA_MSG_EXT_NOTIFY, 
                    MTK_CAMERA_MSG_EXT_NOTIFY_SMILE_DETECT, 
                    0,
                    mCallbackCookie
                ); 
            }
        }
        break;
    case MHAL_CAM_CB_ERR:
        {
            MUINT32 const msg = *reinterpret_cast<MUINT32*>(pvData);
            mNotifyCb(CAMERA_MSG_ERROR, msg, 0, mCallbackCookie); // no memory callback to AP.   
        }
        break;
    case MHAL_CAM_CB_VSS_JPG_ENC:
    {
        CAM_LOGD("[dispatchCallback]VSS:MHAL_CAM_CB_VSS_JPG_ENC");
        if(msgTypeEnabled(CAMERA_MSG_COMPRESSED_IMAGE))
        {
            mhalVSSJpgEncParam_t* pVSSJpgEncParam = (mhalVSSJpgEncParam_t*)pvData;
            // use sw to compress rgb565 to jpeg
            CAM_LOGD("[dispatchCallback]VSS:JPG:RgbAddr(0x%08X),YuvAddr(0x%08X),JpgAddr(0x%08X),W(%d),H(%d)",
                pVSSJpgEncParam->u4ImgRgbVirAddr,
                pVSSJpgEncParam->u4ImgYuvVirAddr,
                pVSSJpgEncParam->u4ImgJpgVirAddr,
                pVSSJpgEncParam->u4ImgWidth,
                pVSSJpgEncParam->u4ImgHeight);
            if(pVSSJpgEncParam->u4ImgYuvVirAddr != 0)
            {
                CAM_LOGD("[dispatchCallback]VSS:YUV2RGB");
                ::hwConvertImageFormat(
                    (uint8_t*)pVSSJpgEncParam->u4ImgYuvVirAddr,
                    MHAL_FORMAT_YUV_420,
                    pVSSJpgEncParam->u4ImgWidth,
                    pVSSJpgEncParam->u4ImgHeight,
                    (uint8_t*)pVSSJpgEncParam->u4ImgRgbVirAddr,
                    MHAL_FORMAT_RGB_565);
            }
            CAM_LOGD("[dispatchCallback]VSS:RGB2JPG");
            ::rgb565toJpeg(
                (uint8_t*)(pVSSJpgEncParam->u4ImgRgbVirAddr),
                (uint8_t*)(pVSSJpgEncParam->u4ImgJpgVirAddr),
                pVSSJpgEncParam->u4ImgWidth,
                pVSSJpgEncParam->u4ImgHeight, 
                &(pVSSJpgEncParam->u4ImgJpgSize));
            CAM_LOGD("[dispatchCallback]VSS:JPG:Size(%d)",pVSSJpgEncParam->u4ImgJpgSize);
        }
        else
        {
            CAM_LOGE("[dispatchCallback]CAMERA_MSG_COMPRESSED_IMAGE is not enabled");
        }
        break;
    }
    case MHAL_CAM_CB_VSS_JPG:
        {
            CAM_LOGD("[dispatchCallback]VSS:MHAL_CAM_CB_VSS_JPG");
            if(msgTypeEnabled(CAMERA_MSG_COMPRESSED_IMAGE))
            {
                sp<IMemoryBufferPool> jpegMemPool = allocMem(
                                                        u4Size,
                                                        1,
                                                        "VSS jpeg callback");
                CAM_LOGD("[dispatchCallback]VSS:JPG:SrcAddr(0x%08X),DstAddr(0x%08X),Size(%d)",
                    (MUINT32)pvData,
                    jpegMemPool->getVirAddr(),
                    u4Size);
                ::memcpy(
                    (void*)jpegMemPool->getVirAddr(),
                    pvData,
                    u4Size);
                //
                CAM_LOGD("[dispatchCallback]VSS:Jpg callback E");
                mDataCb(CAMERA_MSG_COMPRESSED_IMAGE, jpegMemPool->get_camera_memory(), 0, NULL, mCallbackCookie);
                CAM_LOGD("[dispatchCallback]VSS:Jpg callback X");
            }
            else
            {
                CAM_LOGE("[dispatchCallback]CAMERA_MSG_COMPRESSED_IMAGE is not enabled");
            }
        }
        break;
    case MHAL_CAM_CB_FLASHON:
        {
            CAM_LOGD("[dispatchCallback]MHAL_CAM_CB_FLASHON\n");
            mIsZSD = 0;
            mmHalCamParam.u4strobeon = 1;
        }
        break;
    case MHAL_CAM_CB_ZSD_PREVIEW_DONE:
        {
            CAM_LOGD("[dispatchCallback]ZSD:preview done");
            mNotifyCb(MTK_CAMERA_MSG_EXT_NOTIFY, MTK_CAMERA_MSG_EXT_NOTIFY_ZSD_PREVIEW_DONE, 0, mCallbackCookie); // no memory callback to AP.
        }
            break;
    default:
        CAM_LOGD("[dispatchCallback] Unsupported \n");
        break;
    } // end of switch
}


/*******************************************************************************
*
********************************************************************************/
void* MTKCameraHardware::_previewThread(void* arg)
{
    ::prctl(PR_SET_NAME,"Camera@Preview",0,0,0);
    //
    //  thread policy & priority
    int policy = 0, priority = 0;
    getPriority(policy, priority);
    CAM_LOGD("[mHalCamAdapter::_previewThread] (tid, policy, priority)=(%d, %d, %d)", ::gettid(), policy, priority);

    //  detach thread
    ::pthread_detach(::pthread_self());

    //
    MTKCameraHardware*const pHardware = reinterpret_cast<MTKCameraHardware*>(arg);
    if  ( ! pHardware )
    {
        CAM_LOGE("[mHalCamAdapter::_previewThread] NULL arg");
        return  NULL;
    }

    return  ( eDevId_AtvSensor == pHardware->getDevId() )
        ?   pHardware->mAtvPreviewThread()
        :   pHardware->cameraPreviewThread()
        ;
}


/******************************************************************************
*
*******************************************************************************/
void*
MTKCameraHardware::
mAtvPreviewThread()
{
    MY_LOGD("+ tid(%d), mbPreviewThreadAlive(%d)", ::gettid(), mbPreviewThreadAlive);
    //
    while (mbPreviewThreadAlive)
    {
        ::sem_wait(&mSemPrv);
        if ( ! mbPreviewThreadAlive )
        {
            break;
        }
        //
        //  After sleep, CamAdapter object (& memory) could be deleted and freed, 
        //  so that de-reference bad object will cause exception.
        //  And hence, we firstly hold a strong pointer before sleep.
        sp<ICamAdapter> pThis = this;
        //
        //to resolve matv desense issue
        ::usleep(mAtvDispDelay);
        //
        if  ( pThis->getStrongCount() <= 1 ) {
            MY_LOGW("tid(%d): Camera Adapter was set to free during sleep %d us - getStrongCount(%d)", ::gettid(), mAtvDispDelay, pThis->getStrongCount());
            pThis.clear();
            MY_LOGW("break loop !!");
            break;
        }
        //
        postPreviewFrame(mDispBufNo);
    }
    //
    MY_LOGD("- tid(%d)", ::gettid());
    return  NULL;
}


/******************************************************************************
*
*******************************************************************************/
void*
MTKCameraHardware::
cameraPreviewThread()
{
    //  After wake-up from sem_wait, CamAdapter object (& memory) could be deleted and freed, 
    //  so that de-reference bad object will cause exception.
    //  And hence, we firstly hold a strong pointer during this thread is running.
    sp<ICamAdapter> pThis = this;

    DispQueNode dispQueNode;
    size_t      dispQueSize = 0;

    MY_LOGD("+ tid(%d), mbPreviewThreadAlive(%d)", ::gettid(), mbPreviewThreadAlive);
    //
    while (mbPreviewThreadAlive)
    {
#if 0
        mIsDisplay = false;
        ::sem_wait(&mSemPrv);
        if ( ! mbPreviewThreadAlive )
        {
            break;
        }
        //
        mIsDisplay = true;
        //
        postPreviewFrame();
#else
        {
            Mutex::Autolock _lock(mDispQueLock);
            while   ( mDispQue.empty() && mbPreviewThreadAlive )
            {
                status_t status = mDispQueCond.wait(mDispQueLock);
                if  ( OK != status )
                {
                    MY_LOGW("wait status(%d), DispQue.size(%d), mbPreviewThreadAlive(%d)", status, mDispQue.size(), mbPreviewThreadAlive);
                }
            }
            dispQueSize = mDispQue.size();
            if  ( dispQueSize > 0 )
            {
                dispQueNode = mDispQue[dispQueSize-1];
                if  ( mDispQue.size() > 1 )
                {
                    MY_LOGW("Maybe Display too long, DispQue.size(%d)>1: use the lastest", mDispQue.size());
                }
                mDispQue.clear();
            }
        }
        //
        if ( ! mbPreviewThreadAlive )
        {
            break;
        }
        //
        if  ( dispQueSize > 0 )
        {
            postPreviewFrame(dispQueNode.mi4Index, dispQueNode.mi8Timestamp);
        }
#endif
    }
    //
    MY_LOGD("- tid(%d)", ::gettid());
    return  NULL;
}


/******************************************************************************
*
*******************************************************************************/
bool
MTKCameraHardware::
postPreviewFrame(int32_t i4Index, int64_t i8Timestamp)
{
    //  [1] Don't post preview frame if preview has been disabled.
    if  ( ! previewEnabled() )
    {
        MY_LOGW("Preview not enabled - skip i4Index(%d)", i4Index);
        return  false;
    }
    //
    //  [2] Load a strong pointer to preview memory pool.
    int32_t const       i4DispBufNo = i4Index;
    sp<IImageBufferPool> pImagePool = mPreviewMemPool;
    if  (pImagePool == 0)
    {
        MY_LOGW("NULL pImagePool");
        return  false;
    }
    //
    //  [3] Post preview frame.
    CamProfile profile(__FUNCTION__, "MTKCameraHardware");
    //
    postBuffer(pImagePool, i4DispBufNo, i8Timestamp, CAMERA_MSG_PREVIEW_FRAME);
    //
    if  ( profile.print_overtime(12, "postBuffer for CAMERA_MSG_PREVIEW_FRAME") )
    {
        MY_LOGW(
            "i4Index(%d), ImagePool(%p/%d), image WxH=%dx%d", 
            i4Index, pImagePool->getVirAddr(i4DispBufNo), pImagePool->getBufSize(), 
            pImagePool->getImgWidth(), pImagePool->getImgHeight()
        );
    }
    //
    return  true;
}


/******************************************************************************
*
*******************************************************************************/
void MTKCameraHardware::mATVbufferFIFOin(int pcbInfo_dispBufNo)
{
    mATVbuffers[pushindex++] = pcbInfo_dispBufNo;
    //CAM_LOGD("  mATVbufferFIFOin : %d %d\n", pushindex-1, popindex);
    if (pushindex >= (int)mPreviewMemPool->getBufCount()) {
        pushindex -= (int)mPreviewMemPool->getBufCount();
    }
}

/******************************************************************************
*
*******************************************************************************/
void MTKCameraHardware::mATVbufferFIFOout()
{
    //if need delay not post buffer.
    if (mIsATVDelay) {
        if (mATVDispBufNo == -1) {
            mATVStartTime = getTimeMs();
            mATVDispBufNo = 0;
            //pushindex = 0;
            //popindex= 0;
        }
        int delta = getTimeMs() - mATVStartTime;
        if (delta >= mParameters.getInt("tv-delay")) {
            mIsATVDelay = false;
            CAM_LOGD("  Elapsed time: %d \n", delta);
            //get atv custom delay time
            ::mHalIoctl(mmHalFd, MHAL_IOCTL_GET_ATV_DISP_DELAY,NULL,0,&mAtvDispDelay, sizeof(unsigned int),NULL);
            //start wait VD to callback display
            ::mHalIoctl(mmHalFd, MHAL_IOCTL_SET_ATV_DISP,NULL,0,NULL, 0,NULL);            
        }
        else {
            return;
        }
    }
    mDispBufNo =  mATVbuffers[popindex++];

    //to resolve matv desense issue
    //sem_post(&mSemPrv); 

    //move to dispatchCallback() to resolve matv desense issue
    //trick log mATVbuffers[pushindex-1], add (pushindex-1+12)%12
    //CAM_LOGD("  mATVpreviewCallback : in %d , out %d\n", mATVbuffers[(pushindex+11)%12],mATVbuffers[popindex-1]);
    //if (popindex >= YUV_ATV_BUF_NUM) {
    //     	  popindex -= YUV_ATV_BUF_NUM;
    //}
    return;
}

/*******************************************************************************
*
********************************************************************************/
void* MTKCameraHardware::_videoThread(void* arg)
{
    ::prctl(PR_SET_NAME,"Camera@Video",0,0,0);
    //
    //  thread policy & priority
    int policy = 0, priority = 0;
    getPriority(policy, priority);
    CAM_LOGD("[mHalCamAdapter::_videoThread] (tid, policy, priority)=(%d, %d, %d)", ::gettid(), policy, priority);

    //  detach thread
    ::pthread_detach(::pthread_self());

    //
    MTKCameraHardware*const pHardware = reinterpret_cast<MTKCameraHardware*>(arg);
    if  ( ! pHardware )
    {
        CAM_LOGE("[mHalCamAdapter::_videoThread] NULL arg");
        return  NULL;
    }

    return  pHardware->cameraVideoThread();
}

/******************************************************************************
*
*******************************************************************************/
void*
MTKCameraHardware::
cameraVideoThread()
{
    MY_LOGD("+ tid(%d), mbVideoThreadAlive(%d)", ::gettid(), mbVideoThreadAlive);
    //
    while (mbVideoThreadAlive == 0x55555555)
    {
        ::sem_wait(&mSemVdo);  
        if ( mbVideoThreadAlive != 0x55555555)
        {
            break;
        }
        
        CamProfile profile(__FUNCTION__, "MTKCameraHardware");
        sp<IMemoryBufferPool>   pMemPool;
        if (mVideoMemPool.get() ) {
            pMemPool = mVideoMemPool; 
        }
        else {
            pMemPool = mPreviewMemPool; 
        }
        
        camera_memory_t*p_cam_mem = pMemPool->get_camera_memory();

        int iOutIndex = miVdoOutIndex;
        int iCbNo = maVdoCbBufNo[iOutIndex];
        
        miVdoOutIndex = (miVdoOutIndex+1) % 8;
        
        mDataCbTimestamp(maVdoTimStampNs[iOutIndex], CAMERA_MSG_VIDEO_FRAME, p_cam_mem, maVdoCbBufNo[iOutIndex], mCallbackCookie);
        
        profile.print_overtime(5, "mDataCbTimestamp(CAMERA_MSG_VIDEO_FRAME, %d)", iCbNo);
    }
    //
    MY_LOGD("- tid(%d)", ::gettid());
    return  NULL;
}


/******************************************************************************
*
*******************************************************************************/
static long long msRecordStart = 0;
static long long msRecordEnd = 0;
void MTKCameraHardware::videoRecordCallback(void *param)
{
    if (!mIsVideoEnabled || param == NULL) {
        return; 
    }
    //    
    mhalCamTimeStampBufCBInfo *pcbInfo = (mhalCamTimeStampBufCBInfo *) param;
    /*
    sp<IMemoryBufferPool>   pMemPool;
    if (mVideoMemPool.get() ) {
        pMemPool = mVideoMemPool; 
    }
    else {
        pMemPool = mPreviewMemPool; 
    }
    */
    if (mMsgEnabled & CAMERA_MSG_VIDEO_FRAME) {
        nsecs_t nsecCur;
        nsecCur = (nsecs_t) pcbInfo->u4TimStampS * (nsecs_t) 1000000000LL,
        nsecCur += (nsecs_t) pcbInfo->u4TimStampUs * (nsecs_t) 1000LL;    
        msRecordStart = getTimeMs();
    /*
        int32_t i4CbIndex = pcbInfo->u4BufIndex;
        camera_memory_t*p_cam_mem = pMemPool->get_camera_memory();
        //
        CamProfile profile(__FUNCTION__, "MTKCameraHardware");
        //
        mDataCbTimestamp(nsecCur, CAMERA_MSG_VIDEO_FRAME, p_cam_mem, i4CbIndex, mCallbackCookie);
        //
        profile.print_overtime(5, "mDataCbTimestamp(CAMERA_MSG_VIDEO_FRAME, %d)", i4CbIndex);
    */

        maVdoTimStampNs[miVdoInIndex] = nsecCur; 
        maVdoCbBufNo[miVdoInIndex] = pcbInfo->u4BufIndex;

        if(miVdoInIndex != miVdoOutIndex)
            CAM_LOGD("videoRecordCallback  in %d, out %d \n", maVdoCbBufNo[miVdoInIndex], maVdoCbBufNo[miVdoOutIndex]);
        
        miVdoInIndex = (miVdoInIndex+1) % 8;
        
        sem_post(&mSemVdo);
    }
}


/******************************************************************************
*
*******************************************************************************/
void MTKCameraHardware::previewCallback(void *param)
{
    mhalCamBufCBInfo *pcbInfo = (mhalCamBufCBInfo *) param;

    if (mIsTakePrv) {
        mIsTakePrv = false;
        uint8_t *pbufIn, *pbufOut;
        pbufIn = (uint8_t *) mmHalCamParam.frmYuv.virtAddr + mmHalCamParam.frmYuv.frmSize * pcbInfo->u4BufIndex;
        pbufOut = (uint8_t *) mPrvRgbMemPool->getVirAddr();
        //CAM_LOGD("  pbufIn: 0x%x, pbufOut: 0x%x \n", (int) pbufIn, (int) pbufOut);
        const char *prvFmt = mParameters.get(MtkCameraParameters::KEY_PREVIEW_INT_FORMAT);
        uint32_t mPrvFmt = MHAL_FORMAT_YUV_420_SP; 
        if (prvFmt != NULL) {
            if (strcmp(prvFmt, "mtkyuv") == 0) {
                mPrvFmt = MHAL_FORMAT_MTK_YUV;
            }
            else if (strcmp(prvFmt, "yuv420p") == 0) {
                mPrvFmt = MHAL_FORMAT_YUV_420;
            }
            else if (0 == strcmp(prvFmt, MtkCameraParameters::PIXEL_FORMAT_YUV420I)) {
                //  [FIXME:] not sure, mdp pipe may need modify.
                mPrvFmt = MHAL_FORMAT_YUV_420;
            }
            else {
                CAM_LOGD("No mPixelFmt, error \n");
            }
        }
        //use hw to convert preview frame to rgb565
        ::hwConvertImageFormat(pbufIn, mPrvFmt, mmHalCamParam.frmYuv.w, mmHalCamParam.frmYuv.h, pbufOut, MHAL_FORMAT_RGB_565);
        //saveBufToFile("/data/1.565", (uint8_t *) mPrvRgbPmemPool->mVirtAddr, mmHalCamParam.frmYuv.w * mmHalCamParam.frmYuv.h * 2);
        mpMainThread->postCommand(CmdQueThread::Command(CmdQueThread::Command::eID_TAKE_PRV_PIC));
    }

    if  (mIsATV) {
        if (!mIsDisplay) {
            //add by tengfei mtk80343 for AV sync
            //mATV FIFO in
            mATVbufferFIFOin(pcbInfo->u4BufIndex);
            //maTV FIFO out and trigger sem_post()
            mATVbufferFIFOout();
        }
        else {
            CAM_LOGD("  Display too long (>33ms), skip one \n\n");
        }
    }
    else {
#if 0
        if (!mIsDisplay) {
            mDispBufNo = pcbInfo->u4BufIndex;
            sem_post(&mSemPrv);
        }
        else {
            CAM_LOGD("  Display too long (>33ms), skip one \n\n");
        }
#else
        mhalCamTimeStampBufCBInfo *_pcbInfo = (mhalCamTimeStampBufCBInfo *) param;

        nsecs_t timestamp = (nsecs_t)_pcbInfo->u4TimStampS * (nsecs_t)1000000000LL + (nsecs_t)_pcbInfo->u4TimStampUs * (nsecs_t)1000LL;
        int32_t index     = _pcbInfo->u4BufIndex;

        Mutex::Autolock _lock(mDispQueLock);
/*
        if  ( mDispQue.size() >= 1 ) {
            CAM_LOGD("  Maybe Display too long: que.size(%d), skip one", mDispQue.size());
            return;
        }
//        if  ( mDispQue.size() >= 1 ) {
//            CAM_LOGD("  Maybe Display too long: que.size(%d)", mDispQue.size());
//        }
*/
        mDispQue.push_back(DispQueNode(index, timestamp));
        mDispQueCond.broadcast();
#endif
    }
}


/******************************************************************************
*
*******************************************************************************/
status_t MTKCameraHardware::doZoom(int currZoom, int nextZoom)
{
    status_t status = NO_ERROR;

    CAM_LOGD("[doZoom] zoom: %d, %d \n", currZoom, nextZoom);

    if (currZoom == nextZoom) {
        return NO_ERROR;
    }

    if ((nextZoom < 0) || (nextZoom > mZoom_max_value)) {
        // Restore to its current valid value
        mParameters.set(MtkCameraParameters::KEY_ZOOM, currZoom);
        return BAD_VALUE;
    }

    if (getZoomValue(nextZoom) < 100 ) { // in case of wrong customization
        mParameters.set(CameraParameters::KEY_ZOOM, currZoom);      
        return BAD_VALUE;
    }

    // Set zoom
    mZoomValue = nextZoom;

    //  Immediate zoom is set in setParameters(); but don't forget to set smooth zoom.
    mParameters.set(MtkCameraParameters::KEY_ZOOM, mZoomValue);
    int realZoom = getZoomValue(nextZoom);

    status = ::mHalIoctl(mmHalFd, MHAL_IOCTL_SET_ZOOM, &realZoom, sizeof(realZoom), NULL, 0, NULL);
    if (status != NO_ERROR) {
        // Should not happen
        CAM_LOGD("[doZoom] MHAL_IOCTL_SET_ZOOM fail \n");
    }

    return status;
}

/******************************************************************************
*
*******************************************************************************/
status_t MTKCameraHardware::startSmoothZoom(int currZoom, int finalZoom)
{
    // ICS. [Sean]
    // 1. Only support for Mediatek proprietary APK 
    // 2. Only support in Video mode for increase the zoom performance in video record
    CAM_LOGD("[startSmoothZoom] Zoom (Cur, Fianl) = (%d, %d)\n", currZoom, finalZoom); 
    status_t status = NO_ERROR; 
    if (mParameters.getInt(MtkCameraParameters::KEY_CAMERA_MODE) == 2) {
        mZoomStopValue = finalZoom; 
        status = doZoom(mZoomValue, mZoomStopValue);
        if (status != NO_ERROR) {
               return status;
        }
    }

#if 0 
    status_t status = NO_ERROR;
    int zoomStep, nextZoom;
    int priority, policy;
    int zoomTime;

    mIsSmoothZoomCBEnabled = true;

    // Get its default priority
    getPriority(policy, priority);
    // Set to RR
    setPriority(SCHED_RR, 20);

    //
    if (finalZoom > currZoom) {
        zoomStep = 1;
        zoomTime = finalZoom - currZoom;
    }
    else {
        zoomStep = -1;
        zoomTime = currZoom - finalZoom;
    }

    // Here we make sure currZoom != finalZoom.
    // Do zooming with the final one left.
    while ( 1 < zoomTime && (mIsSmoothZoom) ) {
        nextZoom = currZoom + zoomStep;
        status = doZoom(currZoom, nextZoom);
        if (status != NO_ERROR) {
            break;
        }
        currZoom = nextZoom;
        zoomTime--;
    }
    //  The final zoom.
    //  Force to callback with a stoped signal.
    if  ( 1 <= zoomTime ) {
        currZoom = mZoomValue;
        nextZoom = currZoom + zoomStep;
        mZoomStopValue = nextZoom;  //  Force to stop.
        status = doZoom(currZoom, nextZoom);
    }
    if (status != NO_ERROR) {
        mIsSmoothZoomCBEnabled = false;
        mIsSmoothZoom          = false;
    }

    // Restore to its default priority
    setPriority(policy, priority);
#endif 

    return status;
}

/******************************************************************************
*
*******************************************************************************/
status_t MTKCameraHardware::stopSmoothZoom()
{
    status_t status = NO_ERROR;

    mIsSmoothZoom = false;

    return status;
}

/******************************************************************************
*
*******************************************************************************/
status_t MTKCameraHardware::startPreviewInternal()
{
    status_t status = NO_ERROR;
    CAM_LOGD("[startPreviewInternal] Start\n");

    //
    mZoomValue = mParameters.getInt(MtkCameraParameters::KEY_ZOOM);
    //
    if ( mPreviewMemPool == NULL ) {
        CAM_LOGD("[startPreviewInternal] Err Not enough MEMORY mPreviewPmemPool \n\n");
        return NO_MEMORY;
    }
    //
    memset(&mmHalCamParam, 0, sizeof(mhalCamParam_t));
    mmHalCamParam.frmYuv.w = mi4PreviewWidth;
    mmHalCamParam.frmYuv.h = mi4PreviewHeight;
    mmHalCamParam.frmYuv.frmSize = mPreviewMemPool->getBufSize();       //yuvW * yuvH * 2;
    mmHalCamParam.frmYuv.frmCount = mPreviewMemPool->getBufCount();      // yuvBufCnt;
    mmHalCamParam.frmYuv.bufSize = mPreviewMemPool->getPoolSize();      //(yuvW * yuvH * 2) * yuvBufCnt;
    mmHalCamParam.frmYuv.virtAddr = (uint32_t)mPreviewMemPool->getVirAddr();
    mmHalCamParam.frmYuv.phyAddr = (uint32_t)mPreviewMemPool->getPhyAddr();
    //
    const char *prvFmt = mParameters.get(MtkCameraParameters::KEY_PREVIEW_INT_FORMAT);
    const char *defaultFmt = MtkCameraParameters::PIXEL_FORMAT_YUV420SP;
    int pixelFmt;
    if (prvFmt == NULL) {
        prvFmt = defaultFmt;
    }
    mmHalCamParam.frmYuv.frmFormat = mapPixelFormat_string_to_mHal(String8(prvFmt));

    // video frame parameter 
    mVideoMemPool = NULL;
    if (NULL != mParameters.get(CameraParameters::KEY_PREFERRED_PREVIEW_SIZE_FOR_VIDEO)) {
        int i4VideoWidth = 0, i4VideoHeight =0;    
        mParameters.getVideoSize(&i4VideoWidth, &i4VideoHeight); 
        mVideoMemPool = allocCamMem(MHAL_CAM_BUF_VIDEO, "video", i4VideoWidth, i4VideoHeight); 
        if  ( mVideoMemPool == NULL ) {
            CAM_LOGE("[startPreviewInternal] Err Not enough MEMORY mVideoPmemPool \n\n");
            return NO_MEMORY;
        }
        mmHalCamParam.frmVdo.w = i4VideoWidth; 
        mmHalCamParam.frmVdo.h = i4VideoHeight; 
        mmHalCamParam.frmVdo.frmSize = mVideoMemPool->getBufSize();
        mmHalCamParam.frmVdo.frmCount = mVideoMemPool->getBufCount();
        mmHalCamParam.frmVdo.bufSize = mVideoMemPool->getPoolSize();
        mmHalCamParam.frmVdo.virtAddr = (uint32_t)mVideoMemPool->getVirAddr();
        mmHalCamParam.frmVdo.phyAddr = (uint32_t)mVideoMemPool->getPhyAddr();
        mmHalCamParam.frmVdo.frmFormat = mapPixelFormat_string_to_mHal(String8(prvFmt));
    }
    //
    mmHalCamParam.u4CamMode = mParameters.getInt(MtkCameraParameters::KEY_CAMERA_MODE);
    if (mmHalCamParam.u4CamMode == 1){
        if (strcmp(mParameters.get(MtkCameraParameters::KEY_ZSD_MODE), MtkCameraParameters::ON) == 0) {
#ifdef MDP_NEW_PATH_FOR_ZSD
            CAM_LOGD("[ZSD.N]ZSD enable \n");
            mmHalCamParam.camZsdParam.u4ZsdEnable = 0x2;
#else
            CAM_LOGD("[ZSD]ZSD enable \n");
            mmHalCamParam.camZsdParam.u4ZsdEnable = 0x1;
#endif
            if (mIsZSD == 1){
                mmHalCamParam.camZsdParam.u4ZsdSkipPrev = 1;
            }
            mIsZSD = 1;
        }
        else{
            mIsZSD = 0;            
        }
    }
    else{
        mIsZSD = 0;
    }

    updateShotMode(String8(mParameters.get(MtkCameraParameters::KEY_CAPTURE_MODE)));
    mapCam3AParameter(mmHalCamParam.cam3AParam, mParameters, mFeatureParam);
    //
    mmHalCamParam.cam3AParam.eisW = mi4PreviewWidth; 
    mmHalCamParam.cam3AParam.eisH = mi4PreviewHeight; 
    //
    mmHalCamParam.u4ZoomVal = getZoomValue(mZoomValue);
    mmHalCamParam.mhalObserver = mHalCamObserver(mhalCallback, this);
    mmHalCamParam.shotObserver = mHalCamObserver(shotCallback, this);
    //
    mmHalCamParam.u4CamIspMode = mParameters.getInt(MtkCameraParameters::KEY_ISP_MODE);
    //
    strcpy((char *) mmHalCamParam.u1FileName, mParameters.get(MtkCameraParameters::KEY_RAW_PATH));
    //
    status = ::mHalIoctl(mmHalFd, MHAL_IOCTL_PREVIEW_START, &mmHalCamParam, sizeof(mhalCamParam_t), NULL, 0, NULL);
    if (status != NO_ERROR) {
        CAM_LOGD("[startPreviewInternal] MHAL_IOCTL_PREVIEW_START err  \n");
        return status;
    }
    //
    ::android_atomic_release_store(true, &mIsPreviewEnabled);
    //
    CAM_LOGD("[startPreviewInternal] End\n");

    return status;
}

/******************************************************************************
*
*******************************************************************************/
status_t MTKCameraHardware::stopPreviewInternal()
{
    status_t status = NO_ERROR;

    //CAM_LOGD("[stopPreviewInternal] Start\n");
    if (previewEnabled()) {
        status = ::mHalIoctl(mmHalFd, MHAL_IOCTL_PREVIEW_STOP, NULL, 0, NULL, 0, NULL);

        ::android_atomic_acquire_store(false, &mIsPreviewEnabled);

        mPreviewMemPool = NULL;
        mVideoMemPool = NULL;

        if (mIsATV) {
            // Disable fb immediate update
            int enable = 0;
            status = ::mHalIoctl(mmHalFd, MHAL_IOCTL_FB_CONFIG_IMEDIATE_UPDATE, &enable, sizeof(enable), NULL, 0, NULL);
            if (status != NO_ERROR) {
                CAM_LOGD("[stopPreviewInternal] MHAL_IOCTL_FB_CONFIG_IMEDIATE_UPDATE err \n");
                return status;
            }
        }
    }

    //CAM_LOGD("[stopPreviewInternal] End\n");

    return status;
}

/******************************************************************************
*
*******************************************************************************/
status_t MTKCameraHardware::startPreview()
{
    CAM_LOGD("[startPreview] + tid(%d)", gettid());

    status_t status = NO_ERROR;
    MUINT32 TimeoutCount;
    // Check if it is ATV preview
    mIsATV = false;
    mIsATVDelay = false;
    mAtvDispDelay = 0;
    mParameters.getPreviewSize(&mi4PreviewWidth, &mi4PreviewHeight);

    Vector<Size> sizes;
    int32_t   i4MaxPrvW = 0;
    int32_t   i4MaxPrvH = 0;
    mParameters.getSupportedPreviewSizes(sizes);
    
    for(Vector<Size>::iterator iter = sizes.begin(); iter!=sizes.end(); ++iter)
    {
        if(i4MaxPrvW<iter->width || i4MaxPrvH<iter->height);
        {
            i4MaxPrvW = iter->width;
            i4MaxPrvH = iter->height;
        }
    }

    if(mi4PreviewWidth>i4MaxPrvW || mi4PreviewHeight>i4MaxPrvH)
    {
        CAM_LOGE("The preview size is to large, not supported! Max supported preview size(%d/%d), required preview size(%d/%d)",
            i4MaxPrvW, i4MaxPrvH, mi4PreviewWidth, mi4PreviewHeight);
        return BAD_VALUE;     
    }

    if ( getDevId() == eDevId_AtvSensor ) 
    { 
        CAM_LOGD("[startPreview] ATV Preview \n"); 
        mIsATV = true;
        mIsATVDelay = true;
        mATVDispBufNo = -1;
        //add by tengfei mtk80343 for AV sync
        pushindex = 0;
        popindex = 0; 
        mPreviewMemPool = allocImagePool(MHAL_CAM_BUF_PREVIEW_ATV, mi4PreviewWidth, mi4PreviewHeight, "previewATV");   

        status = startPreviewInternal();
        if (status != NO_ERROR) 
        {
            CAM_LOGD("[startPreview] startPreviewInternal err  \n");
            return status;
        }
        // Enable fb immediate update
        int enable = 1;
        status = ::mHalIoctl(mmHalFd, MHAL_IOCTL_FB_CONFIG_IMEDIATE_UPDATE, &enable, sizeof(enable), NULL, 0, NULL);
        if (status != NO_ERROR) {
            CAM_LOGD("[startPreview] MHAL_IOCTL_FB_CONFIG_IMEDIATE_UPDATE err \n");
            return status;
        }
    }
    else 
    {
        TimeoutCount = 0;
        while(meCamState == eCS_TakingPicture)
        {
            CAM_LOGW("[startPreview]Wait capture end,count(%d)",TimeoutCount);
            ::usleep(20*1000);
            TimeoutCount++;
            if(TimeoutCount > 50)
            {
                CAM_LOGE("[startPreview]Wait capture end timeout,count(%d)",TimeoutCount);
                return INVALID_OPERATION;
            }
        }
        //
        CAM_LOGD("[startPreview] Camera Preview \n"); 
        mPreviewMemPool = allocImagePool(MHAL_CAM_BUF_PREVIEW, mi4PreviewWidth, mi4PreviewHeight, "preview");
        status = startPreviewInternal();
        if (status != NO_ERROR) 
        {
            CAM_LOGD("[startPreview] startPreviewInternal err  \n");
            return status;
        }
    }

    return status;
}

/******************************************************************************
*
*******************************************************************************/
void MTKCameraHardware::stopPreview()
{
    CAM_LOGD("[stopPreview] + tid(%d)", gettid());

    stopPreviewInternal();

    //mIsZoomReset = true;
    //CAM_LOGD("[stopPreview] End\n");
}

/******************************************************************************
*
*******************************************************************************/
bool MTKCameraHardware::previewEnabled() {

    return  (0 != ::android_atomic_acquire_load(&mIsPreviewEnabled));
}

/******************************************************************************
*
*******************************************************************************/
status_t MTKCameraHardware::startRecording()
{
    status_t status = NO_ERROR;
    int data[3];
    //
    CAM_LOGD("[startRecording] E \n");
    
    //
    miVdoInIndex= 0;
    miVdoOutIndex = 0; 
    //
    status = ::mHalIoctl(mmHalFd, MHAL_IOCTL_VIDEO_START_RECORD, data, sizeof(data), NULL, 0, NULL);
    if (status != NO_ERROR) {
        CAM_LOGD("[startRecording] MHAL_IOCTL_VIDEO_START_RECORD err  \n");
        return status;
    }
    //
    mIsVideoEnabled = true;
    return status;
}

/******************************************************************************
*
*******************************************************************************/
void MTKCameraHardware::stopRecording()
{
    status_t status = NO_ERROR;
    //
    CAM_LOGD("[stopRecording] \n");
    //
    mIsVideoEnabled = false;
    //
    status = ::mHalIoctl(mmHalFd, MHAL_IOCTL_VIDEO_STOP_RECORD, NULL, 0, NULL, 0, NULL);
    if (status != NO_ERROR) {
        CAM_LOGD("[stopRecording] MHAL_IOCTL_VIDEO_STOP_RECORD err  \n");
    }
}

/******************************************************************************
*
*******************************************************************************/
bool MTKCameraHardware::recordingEnabled()
{
    return mIsVideoEnabled;
}

/******************************************************************************
* Release a record frame previously returned by CAMERA_MSG_VIDEO_FRAME.
*
* It is camera hal client's responsibility to release video recording
* frames sent out by the camera hal before the camera hal receives
* a call to disableMsgType(CAMERA_MSG_VIDEO_FRAME). After it receives
* the call to disableMsgType(CAMERA_MSG_VIDEO_FRAME), it is camera hal's
* responsibility of managing the life-cycle of the video recording
* frames.
*******************************************************************************/
void
MTKCameraHardware::
releaseRecordingFrame(const void *opaque)
{
    if  ( ! msgTypeEnabled(CAMERA_MSG_VIDEO_FRAME) )
    {
        CAM_LOGI(
            "[releaseRecordingFrame] + CAMERA_MSG_VIDEO_FRAME has been disabled: tid(%d), opaque(%p)"
            , gettid(), opaque
        );
    }
    //
    sp<IMemoryBufferPool>   pMemPool;
    if (mVideoMemPool.get() ) {
        pMemPool = mVideoMemPool; 
    } 
    else {
        pMemPool = mPreviewMemPool; 
    }
    if (pMemPool == NULL) 
    {
        CAM_LOGI("[releaseRecordingFrame] video mem already release"); 
        return; 
    }

    void const*const pReleaseMem = opaque;
    msRecordEnd = getTimeMs();
    static uint32_t idx = 0;
    if  ( pReleaseMem != (void const*)pMemPool->getVirAddr(idx) ) {            
        CAM_LOGD("[releaseRecordingFrame] invalid frame - mVideoMemPool->getVirAddr(%d)=%08x", idx, pMemPool->getVirAddr(idx));
        for (idx = 0; idx < pMemPool->getBufCount(); idx++) {
            if  ( pReleaseMem == (void const*)pMemPool->getVirAddr(idx) ) {
                break;
            }
        }
    }
    if  (idx < pMemPool->getBufCount()) {
        CAM_LOGD("[releaseRecordingFrame] mpBuffer[%d]=%p - period: %lld ms", idx, pReleaseMem, msRecordEnd-msRecordStart);
            
        status_t status = NO_ERROR;            
        status = ::mHalIoctl(mmHalFd, MAHL_IOCTL_RELEASE_VDO_FRAME, &idx, sizeof(int), NULL, 0, NULL);            
        idx = (idx+1) % pMemPool->getBufCount();
    }
    else {
        CAM_LOGD("[releaseRecordingFrame] cannot find frame: %08x", pMemPool->getVirAddr(idx));
        idx = 0;
   }       
}

/******************************************************************************
*
*******************************************************************************/
status_t MTKCameraHardware::autoFocus()
{
    status_t status = NO_ERROR;

    if (!mIsPreviewEnabled) {
        CAM_LOGD("[autoFocus] not in preview state \n"); 
        return NO_ERROR;    //It is better to return INVALID_OPERATION; 
    }

    CAM_LOGD("[autoFocus] \n");
    #if (MTKCAM_CAP_PROFILE_OPT)
    mFocusStartTime = getTimeMs();
    #endif
    
    status = ::mHalIoctl(mmHalFd, MHAL_IOCTL_DO_FOCUS, NULL, 0, NULL, 0, NULL);
    if (status != NO_ERROR) {
        // can't lock resource
        CAM_LOGD("  autoFocus, set MHAL_IOCTL_DO_FOCUS fail \n");
        freeCamCaptureMem(); 
    }

    return status;
}

/******************************************************************************
*
*******************************************************************************/
status_t MTKCameraHardware::cancelAutoFocus()
{
    status_t status = NO_ERROR;

    CAM_LOGD("[cancelAutoFocus] \n");

    if (previewEnabled()) {
        status = ::mHalIoctl(mmHalFd, MHAL_IOCTL_CANCEL_FOCUS, NULL, 0, NULL, 0, NULL);
        if (status != NO_ERROR) {
            // can't lock resource
            CAM_LOGD("  cancelAutoFocus, set MHAL_IOCTL_CANCEL_FOCUS fail \n");
        }
    }

    return status;
}


/******************************************************************************
*
*******************************************************************************/
sp<IMemoryBufferPool>
MTKCameraHardware::
allocMem(size_t const bufsize, uint_t const numbufs, char const*const szName) const
{
    sp<IMemoryBufferPool> mem;
    int i = 0, retry = 10;
    int us = 200 * 1000;

    //
    do {
        mpBufProvider->allocBuffer(mem, bufsize, numbufs, szName);
        if  ( mem != NULL &&  0 != mem->getPoolSize() &&  0 != mem->getVirAddr() ) {
            break; 
        }
        ::usleep(us); 
        CAM_LOGD("[allocMem] retry: %d \n ", i++); 
    } while ((--retry) > 0);

    //
    if  ( mem == NULL || 0 == mem->getPoolSize() || 0 == mem->getVirAddr() ) {
        // Should not happen
        CAM_LOGD("[allocMem] alloc mem fail, should not happen \n");
        mem = NULL;
        //while (1);
    }

    return mem;
}


/******************************************************************************
*
*******************************************************************************/
sp<IMemoryBufferPool>
MTKCameraHardware::
allocCamMem(int poolID, char const*const szName, int frmW, int frmH) const
{
    status_t status = NO_ERROR; 
    CAM_LOGD("[allocCamMem] poolID = %d, poolName = %s, frmW = %d, frmH = %d \n", 
                          poolID, szName, frmW, frmH); 

    //
    mhalCamBufMemInfo_t camBufInfo; 
    memset(&camBufInfo, 0, sizeof(mhalCamBufMemInfo_t)); 
    camBufInfo.bufID = poolID; 
    camBufInfo.frmW = frmW; 
    camBufInfo.frmH = frmH; 
    //
    status = ::mHalIoctl(mmHalFd, MHAL_IOCTL_GET_CAM_BUF_MEM_INFO, NULL, 0, &camBufInfo, sizeof(mhalCamBufMemInfo_t), NULL); 
    if (status != NO_ERROR) {
        CAM_LOGD("[allocCamMem] MHAL_IOCTL_GET_CAM_BUF_INFO fail \n"); 
        return NULL; 
    }
/*
    int32_t i4Type = MemPool::eTYPE_UKNOWN;
    switch  (camBufInfo.camMemType)
    {
    case MHAL_CAM_ASHMEM_TYPE:
        i4Type = MemPool::eTYPE_ASHMEM;
        break;
    case MHAL_CAM_PMEM_TYPE:
        i4Type = MemPool::eTYPE_PMEM;
        break;
    default:
        i4Type = MemPool::eTYPE_UKNOWN;
        break;
    }
*/
    return  allocMem(camBufInfo.camBufSize, camBufInfo.camBufCount, szName);
}


/******************************************************************************
*
*******************************************************************************/
status_t MTKCameraHardware::freeCamCaptureMem() 
{
    mQvMemPool = NULL; 
    mJpegMemPool = NULL; 
    return NO_ERROR; 
}


/******************************************************************************
*
*******************************************************************************/
status_t MTKCameraHardware::allocCamCaptureMem()
{
    status_t status = NO_ERROR;

    // 
    if (mQvMemPool == NULL) {
        int qvW = 0, qvH = 0;
        //qv size is the same as preview size
        mParameters.getPreviewSize(&qvW, &qvH);
        if(1920 == qvW && 1088 == qvH)
        {
            CAM_LOGD("For 1080p preview size, QV size is set as 640*480 \n");
            qvW = 640;
            qvH = 480;
        }
        
        mQvMemPool = allocImagePool(MHAL_CAM_BUF_POSTVIEW, qvW, qvH, "qv");
        //
        if (mQvMemPool == NULL) {
            CAM_LOGD("[allocCamCaptureMem] Err Not enough MEMORY for mQvPmemPool \n\n");
            return NO_MEMORY;
        }
    }    
    //
    if (mJpegMemPool == NULL) {
        int capW = 0, capH = 0; 
        mParameters.getPictureSize(&capW, &capH);
        mJpegMemPool = allocCamMem(MHAL_CAM_BUF_CAPTURE, "jpeg", capW, capH); 
        //
        if (mJpegMemPool == NULL) {
            CAM_LOGD("[allocCamCaptureMem] Err Not enough MEMORY for mJpegPmemPool \n\n");
            return NO_MEMORY;
        }
    }
    return status;
}

/*******************************************************************************
*
********************************************************************************/
status_t MTKCameraHardware::takePictureProc()
{
    int rotation_angle;
    int current_sensor;
    int current_sensor_angle;
    status_t status = NO_ERROR;
    int burstCnt = mmHalCamParam.u4BusrtCnt;

    CAM_LOGD("[takePictureProc]: tid(%d) getStrongCount(%d) burstCnt(%d)", gettid(), getStrongCount(), burstCnt);

    status = ::mHalIoctl(mmHalFd, MHAL_IOCTL_CAPTURE_INIT, &mmHalCamParam, sizeof(mhalCamParam_t), NULL, 0, NULL);
    if (status != NO_ERROR) {
        CAM_LOGD("  takePictureProc, set MHAL_IOCTL_CAPTURE_INIT fail \n");
        return status;
    }

    {
        mStateLock.lock();
        CAM_LOGD("[takePictureProc][state transition] %s --> %s", getCamStateStr(meCamState), getCamStateStr(eCS_TakingPicture));
        //
        //  TODO: should check the current state.
        //
        //
        meCamState = eCS_TakingPicture;
        mStateLock.unlock();
    }

//    if (burstCnt > 1) {
//        // disable jpeg callback, enable it until last picture
//        disableMsgType(CAMERA_MSG_COMPRESSED_IMAGE);
//    }

    //
    while ( ! mIsCancelPicture && burstCnt > 0 ) {
        status = ::mHalIoctl(mmHalFd, MHAL_IOCTL_CAPTURE_START, &mmHalCamParam, sizeof(mhalCamParam_t), NULL, 0, NULL);
        if (status != NO_ERROR) {
            CAM_LOGD("  takePictureProc, set MHAL_IOCTL_CAPTURE_START fail \n");
            break;
        }
        if ( 0 != mmHalCamParam.camZsdParam.u4ZsdAddr ){
            mmHalCamParam.camZsdParam.u4ZsdAddr = 0;
            mmHalCamParam.u4ZoomVal = getZoomValue(mZoomValue);
//            mmHalCamParam.camZsdParam.u4ZsdEVShot = 0;
            mmHalCamParam.camZsdParam.u4ZsdEnable = 0;
        }
        //
        ::sem_wait(&mSemTakePicBack);
        CAM_LOGD("  got mSemTakePicBack \n");
        //
        mmHalCamParam.u4BusrtNo++;
        burstCnt--;
        //
        if (mIsCancelPicture) {
            CAM_LOGD("mIsCancelPicture=true");
            break;
        }
        //
        if (burstCnt > 0) {
            mIsZSD = 0;
            //freeCamCaptureMem(); 
            //allocCamCaptureMem();
            mmHalCamParam.frmQv.virtAddr = (uint32_t)mQvMemPool->getVirAddr();
            mmHalCamParam.frmQv.phyAddr = (uint32_t)mQvMemPool->getPhyAddr();
            mmHalCamParam.frmJpg.virtAddr = (uint32_t)mJpegMemPool->getVirAddr();
            mmHalCamParam.frmJpg.phyAddr = (uint32_t)mJpegMemPool->getPhyAddr();
//            if (burstCnt == 1) {
//                // enable last jpeg callback
//                enableMsgType(CAMERA_MSG_COMPRESSED_IMAGE);
//            }
        }
    }

    if(mu4ShotMode == MHAL_CAM_CAP_MODE_CONTINUOUS_SHOT)
    {
        LOGD("continuous shot end");
        mNotifyCb(
            MTK_CAMERA_MSG_EXT_NOTIFY, MTK_CAMERA_MSG_EXT_NOTIFY_CONTINUOUS_END, 
            mmHalCamParam.u4BusrtNo, 
            mCallbackCookie);
    }
    //
    if(burstCnt > 0)
    {
        CAM_LOGD("[takePictureProc]Force save all file");
        waitFileSaveDone();
        CAM_LOGD("[takePictureProc]All file saved done");
    }
    //
    status = ::mHalIoctl(mmHalFd, MHAL_IOCTL_CAPTURE_UNINIT, NULL, 0, NULL, 0, NULL);
    if (status != NO_ERROR) {
        CAM_LOGD("  takePictureProc, set MHAL_IOCTL_CAPTURE_UNINIT fail \n");
    }

    // Free memory
    mQvMemPool = NULL;
    mJpegMemPool = NULL;

    changeState(eCS_Init, true);


    CAM_LOGD("[takePictureProc] -");
    return status;
}

/******************************************************************************
*
*******************************************************************************/
status_t MTKCameraHardware::takePictureInternal()
{
    status_t status = NO_ERROR;
    int capW, capH, thumbW, thumbH;
    int jpqQVal, gpsAltitude;
    char *gpsLatitude, *gpsLongitude, *gpsTimestamp, *gpsProcessingMethod;
    char *pstr, *pstrVals;
    int zsdAddr=0,zsdW=0,zsdH=0;
    int strobeon;

    #if 0   // For Debug
    mParameters.set(MtkCameraParameters::KEY_JPEG_THUMBNAIL_WIDTH, 0);
    mParameters.set(MtkCameraParameters::KEY_JPEG_THUMBNAIL_HEIGHT, 0);
    mParameters.set(MtkCameraParameters::KEY_GPS_ALTITUDE, "21");
    mParameters.set(MtkCameraParameters::KEY_GPS_LATITUDE, "37.736071");
    mParameters.set(MtkCameraParameters::KEY_GPS_LONGITUDE, "-122.441983");
    mParameters.set(MtkCameraParameters::KEY_GPS_TIMESTAMP, "1199145600");
    mParameters.set(MtkCameraParameters::KEY_GPS_PROCESSING_METHOD, "GPS NETWORK HYBRID ARE ALL FINE.");
    #endif
    // Get capture size
    mParameters.getPictureSize(&capW, &capH);
    // Get Zoom dimension
    mZoomValue = mParameters.getInt(MtkCameraParameters::KEY_ZOOM);
    thumbW = mParameters.getInt(MtkCameraParameters::KEY_JPEG_THUMBNAIL_WIDTH);
    thumbH = mParameters.getInt(MtkCameraParameters::KEY_JPEG_THUMBNAIL_HEIGHT);
    jpqQVal = mParameters.getInt(MtkCameraParameters::KEY_JPEG_QUALITY);
    gpsAltitude = mParameters.getInt(MtkCameraParameters::KEY_GPS_ALTITUDE);
    gpsLatitude = (char *) mParameters.get(MtkCameraParameters::KEY_GPS_LATITUDE);
    gpsLongitude = (char *) mParameters.get(MtkCameraParameters::KEY_GPS_LONGITUDE);
    gpsTimestamp = (char *) mParameters.get(MtkCameraParameters::KEY_GPS_TIMESTAMP);
    gpsProcessingMethod = (char *) mParameters.get(MtkCameraParameters::KEY_GPS_PROCESSING_METHOD);
    if ((mIsZSD == 1)&&(0!=mmHalCamParam.camZsdParam.u4ZsdAddr)){
        if((mmHalCamParam.camZsdParam.u4ZsdZoomWidth != 0) && (mmHalCamParam.camZsdParam.u4ZsdZoomHeigth !=0)){
            zsdW = mmHalCamParam.camZsdParam.u4ZsdZoomWidth;
            zsdH = mmHalCamParam.camZsdParam.u4ZsdZoomHeigth;
        }
        else{
            zsdW = mmHalCamParam.camZsdParam.u4ZsdWidth;
            zsdH = mmHalCamParam.camZsdParam.u4ZsdHeight;
        }        
        zsdAddr = mmHalCamParam.camZsdParam.u4ZsdAddr;
        if (mmHalCamParam.camZsdParam.u4ZsdDump){
            CAM_LOGD("[dump to file][ZSD]: Addr:%x, W:%d,H:%d \n",zsdAddr,zsdW,zsdH); 
            saveBufToFile((char *) "/sdcard/zsd_cap.bin", (MUINT8 *) zsdAddr, zsdW*zsdH*2);
        }
        CAM_LOGD("[takePictureInternal][ZSD]: Addr:0x%x,W:%d,H:%d \n",zsdAddr,zsdW,zsdH); 
    }
    else{
        mIsZSD = 0;
        mmHalCamParam.camZsdParam.u4ZsdAddr = 0;
        mmHalCamParam.camZsdParam.u4ZsdEnable = 0;
        mmHalCamParam.u4ZoomVal = getZoomValue(mZoomValue);
        CAM_LOGD("[takePictureInternal][ZSD]: zsd off\n"); 
    }
    strobeon = mmHalCamParam.u4strobeon;
    status = allocCamCaptureMem();
    if (status != NO_ERROR) {
        CAM_LOGE("[takePictureInternal] Err Not enough MEMORY for capture \n\n");
        mNotifyCb(CAMERA_MSG_ERROR, CAMERA_ERROR_NO_MEMORY, 0, mCallbackCookie); // no memory callback to AP.
        return NO_ERROR; // to avoid java exception triggered in JNI        
    }
    //
    ::memset(&mmHalCamParam, 0, sizeof(mhalCamParam_t));
    if (mIsZSD == 1){
        mmHalCamParam.camZsdParam.u4ZsdEnable = 0x1;
        mmHalCamParam.camZsdParam.u4ZsdAddr = zsdAddr;
        mmHalCamParam.camZsdParam.u4ZsdWidth = zsdW;
        mmHalCamParam.camZsdParam.u4ZsdHeight = zsdH;
    }
    mmHalCamParam.u4strobeon = strobeon;
    //AWB2PASS
    if (strcmp(mParameters.get(MtkCameraParameters::KEY_AWB2PASS), MtkCameraParameters::ON) == 0) {
        mmHalCamParam.u4awb2pass = 0x1;
    }
    // Quickview
    mmHalCamParam.frmQv.w           = mQvMemPool->getImgWidth();
    mmHalCamParam.frmQv.h           = mQvMemPool->getImgHeight();
    mmHalCamParam.frmQv.frmSize     = mQvMemPool->getBufSize();
    mmHalCamParam.frmQv.frmCount    = mQvMemPool->getBufCount();
    mmHalCamParam.frmQv.bufSize     = mQvMemPool->getBufSize();
    mmHalCamParam.frmQv.virtAddr    = (uint32_t)mQvMemPool->getVirAddr();
    mmHalCamParam.frmQv.phyAddr     = (uint32_t)mQvMemPool->getPhyAddr();
    mmHalCamParam.frmQv.frmFormat   = mapPixelFormat_string_to_mHal(mQvMemPool->getImgFormat());
    //
    // Jpeg
    mmHalCamParam.frmJpg.w          = capW;
    mmHalCamParam.frmJpg.h          = capH;
    mmHalCamParam.frmJpg.frmSize    = mJpegMemPool->getBufSize();
    mmHalCamParam.frmJpg.frmCount   = mJpegMemPool->getBufCount();
    mmHalCamParam.frmJpg.bufSize    = mJpegMemPool->getBufSize();
    mmHalCamParam.frmJpg.virtAddr   = (uint32_t)mJpegMemPool->getVirAddr();
    mmHalCamParam.frmJpg.phyAddr    = (uint32_t)mJpegMemPool->getPhyAddr();
    //
    // Thumb
    mmHalCamParam.u4ThumbW = thumbW;
    mmHalCamParam.u4ThumbH = thumbH;
    if (mIsZSD == 1){
#ifdef MDP_NEW_PATH_FOR_ZSD    
        mmHalCamParam.u4ZoomVal = getZoomValue(mZoomValue);
#else
        mmHalCamParam.u4ZoomVal = 100;
#endif
    }
    else{
        mmHalCamParam.u4ZoomVal = getZoomValue(mZoomValue);
    }    
    mmHalCamParam.mhalObserver = mHalCamObserver(mhalCallback, this);
    mmHalCamParam.shotObserver = mHalCamObserver(shotCallback, this);
    // Other
    mmHalCamParam.u4JpgQValue = jpqQVal;
    mmHalCamParam.camExifParam.gpsIsOn = 0;
    if ((gpsLatitude != NULL) && (gpsLongitude != NULL) 
        && (gpsTimestamp != NULL) && (gpsProcessingMethod != NULL)) 
    {
        strcpy(mmHalCamParam.camExifParam.gpsLongitude, gpsLongitude);
        strcpy(mmHalCamParam.camExifParam.gpsLatitude, gpsLatitude);
        strcpy(mmHalCamParam.camExifParam.gpsTimeStamp, gpsTimestamp);
        strcpy(mmHalCamParam.camExifParam.gpsProcessingMethod, gpsProcessingMethod);
        mmHalCamParam.camExifParam.gpsAltitude = gpsAltitude;
        mmHalCamParam.camExifParam.gpsIsOn = 1;
    }
    else 
    {
        CAM_LOGW("[startAUTORAMA] GPS Information are not avaiable !"); 
    }
    // rawsave-mode
    if (mParameters.getInt(MtkCameraParameters::KEY_RAW_SAVE_MODE) > 0) {
        mmHalCamParam.u4IsDumpRaw = 1;
        if (mParameters.getInt(MtkCameraParameters::KEY_RAW_SAVE_MODE) == 1) {
            mmHalCamParam.u4CapPreFlag = 1;
        }
    }
    strcpy((char *) mmHalCamParam.u1FileName, mParameters.get(MtkCameraParameters::KEY_RAW_PATH));
    if  ( char const* pFileName = mParameters.get(MtkCameraParameters::KEY_CAPTURE_PATH) )
    {
        ::strncpy((char *)mmHalCamParam.uShotFileName, pFileName, sizeof(mmHalCamParam.uShotFileName));
        MY_LOGD("uShotFileName=\"%s\"", mmHalCamParam.uShotFileName);
    }
    // Restore ui settings for 3A for EXIF
    updateShotMode(String8(mParameters.get(MtkCameraParameters::KEY_CAPTURE_MODE)));
    mapCam3AParameter(mmHalCamParam.cam3AParam, mParameters, mFeatureParam);
    // capture mode
    mmHalCamParam.u4BusrtNo = 0;
    switch (mu4ShotMode) {
    case MHAL_CAM_CAP_MODE_EV_BRACKET:
    case MHAL_CAM_CAP_MODE_BEST_SHOT:
        mmHalCamParam.u4BusrtCnt = 3;
        break;
    case MHAL_CAM_CAP_MODE_BURST_SHOT:
        mmHalCamParam.u4BusrtCnt = mParameters.getInt(MtkCameraParameters::KEY_BURST_SHOT_NUM);
        break;
    case MHAL_CAM_CAP_MODE_CONTINUOUS_SHOT:
        mmHalCamParam.u4BusrtCnt = mParameters.getInt(MtkCameraParameters::KEY_BURST_SHOT_NUM);
        if (strcmp(mParameters.get(MtkCameraParameters::KEY_FAST_CONTINUOUS_SHOT), MtkCameraParameters::ON) == 0)
            mmHalCamParam.u4ContinuousShotSpeed = 1;
        else
            mmHalCamParam.u4ContinuousShotSpeed = 0;
                
        break;
    default:
        mmHalCamParam.u4BusrtCnt = 1;
        break;
    }
    if (mu4ShotMode != MHAL_CAM_CAP_MODE_NORMAL){
        if (mIsZSD == 1){
            mmHalCamParam.camZsdParam.u4ZsdEnable = 0;
            mmHalCamParam.camZsdParam.u4ZsdAddr = 0;
            mmHalCamParam.u4ZoomVal = getZoomValue(mZoomValue);
            mIsZSD = 0;
            CAM_LOGD("[takePictureInternal][ZSD]Off ZSD, mu4ShotMode:%d \n",mu4ShotMode); 
        }
    }
    //
    // for JPEG orienation, 
    // Due to contiuous shot performance, in this mode, do not do JPEG orientation
    /*if (mu4ShotMode == MHAL_CAM_CAP_MODE_CONTINUOUS_SHOT ||
        false == NSCamCustom::isSupportJpegOrientation()
        )
    {
        mmHalCamParam.camExifParam.orientation = mParameters.getInt(MtkCameraParameters::KEY_ROTATION);
        mmHalCamParam.u4JPEGOrientation = 0; 
    }
    else 
    {
        mmHalCamParam.camExifParam.orientation = 0; 
        mmHalCamParam.u4JPEGOrientation = mParameters.getInt(MtkCameraParameters::KEY_ROTATION) / 90;
        if (mmHalCamParam.u4JPEGOrientation != 0) 
        {
            mmHalCamParam.u4DumpYuvData = 1; 
        }
    }
    */
    mmHalCamParam.camExifParam.orientation = mParameters.getInt(MtkCameraParameters::KEY_ROTATION);
    mmHalCamParam.u4JPEGOrientation = 0;

    //
    mmHalCamParam.u4CamIspMode = mParameters.getInt(MtkCameraParameters::KEY_ISP_MODE);
    //
    mpMainThread->postCommand(CmdQueThread::Command(CmdQueThread::Command::eID_TAKE_PICTURE));
    //CAM_LOGD("[takePicture] End\n");

    return status;
}

/******************************************************************************
*
*******************************************************************************/
status_t MTKCameraHardware::takePicture()
{
    CAM_LOGD("[takePicture] + tid(%d)", gettid());

    if  ( ! previewEnabled() )
    {
        CAM_LOGE("[takePicture] not in preview state");
        return INVALID_OPERATION; 
    }

    status_t status = NO_ERROR;

    #if (MTKCAM_CAP_PROFILE_OPT)
    mCaptureStartTime = getTimeMs();
    #endif

    mIsCancelPicture = false;

    if (mIsATV) {
        takePreviewPictureInternal();
        return status;
    }
    //
    if(mIsVideoEnabled)
    {
        CAM_LOGD("[takePicture]MHAL_IOCTL_VIDEO_SNAPSHOT");
        status = ::mHalIoctl(mmHalFd, MHAL_IOCTL_VIDEO_SNAPSHOT, (void*)(mParameters.getInt(MtkCameraParameters::KEY_ROTATION)), 0, NULL, 0, NULL);
        if(status != NO_ERROR)
        {
            CAM_LOGD("[takePicture]MHAL_IOCTL_VIDEO_SNAPSHOT fail");
            return status;
        }
        return status;
    }
    //
    if (mParameters.getInt(MtkCameraParameters::KEY_FOCUS_ENG_MODE) == 0 || mParameters.getInt(MtkCameraParameters::KEY_FOCUS_ENG_MODE) == 5) {
        // Do pre capture before stop preview, for 3A
        CAM_LOGD("[takePicture] Do pre capture \n");
        status = ::mHalIoctl(mmHalFd, MHAL_IOCTL_PRE_CAPTURE, NULL, 0, NULL, 0, NULL);
        if (status != NO_ERROR) {
            CAM_LOGD("[takePicture] MHAL_IOCTL_PRE_CAPTURE fail \n");
            return status;
        }
    }
    //
    stopPreviewInternal();
    //
    status = takePictureInternal();
    //
    CAM_LOGD("[takePicture] - status(%d)", status);
    return status;
}

/*******************************************************************************
*
********************************************************************************/
status_t MTKCameraHardware::takePreviewPictureProc()
{
    status_t status = NO_ERROR;
    int memSize;

    CAM_LOGD("[takePreviewPictureProc] \n");
    //
    uint8_t *pbufIn, *pbufOut;
    pbufIn = (uint8_t *) mPrvRgbMemPool->getVirAddr();
    pbufOut = (uint8_t *) mPrvJpegMemPool->getVirAddr();
    // use sw to compress rgb565 to jpeg
    uint32_t jpegSize;
    uint32_t w = mmHalCamParam.frmYuv.w;
    uint32_t h = mmHalCamParam.frmYuv.h;
    ::rgb565toJpeg(pbufIn, pbufOut, w, h, &jpegSize);
    //FIX_ME
    //saveBufToFile((char *)"/data/1.jpg", pbufOut, jpegSize);
    //
    if (mMsgEnabled & CAMERA_MSG_COMPRESSED_IMAGE) {
        CAM_LOGD("[takePreviewPictureProc] MHAL_CAM_CB_JPEG \n");
        mDataCb(CAMERA_MSG_COMPRESSED_IMAGE, mPrvJpegMemPool->get_camera_memory(), 0, NULL, mCallbackCookie);
    }
    //
    mPrvRgbMemPool = NULL;
    mPrvJpegMemPool = NULL;

    CAM_LOGD("[takePreviewPictureProc] End \n");

    return status;
}

/******************************************************************************
*
*******************************************************************************/
status_t MTKCameraHardware::takePreviewPictureInternal()
{
    status_t status = NO_ERROR;

    int memSize = mmHalCamParam.frmYuv.w * mmHalCamParam.frmYuv.h * 2;
    mPrvRgbMemPool = allocMem(memSize, 1, "prvRgb");
    mPrvJpegMemPool = allocMem(memSize, 1, "prvJpeg");

    mIsTakePrv = true;

    return status;
}

/******************************************************************************
*
*******************************************************************************/
status_t MTKCameraHardware::cancelPicture()
{
    CAM_LOGD("[cancelPicture] + (pid,tid)=(%d,%d) mIsCancelPicture(%d)", getpid(), gettid(), mIsCancelPicture);
    mIsCancelPicture = true;
    //
    //  Callers cannot be callback threads to CameraService, such as mainThread and shutterThread.
    //  TODO: check shutterThread
    CAM_LOGD("[cancelPicture] tid:(current, main thread)=(%d, %d)", ::gettid(), mpMainThread->getTid());
    if  ( ::gettid() == mpMainThread->getTid() )
    {
        MY_LOGW("[cancelPicture] The caller is mainThread");
        return PERMISSION_DENIED;
    }
    //
    Mutex::Autolock stateLock(&mStateLock);
    while   ( eCS_TakingPicture == meCamState )
    {
        CAM_LOGD("[cancelPicture] waiting until taking picture done");
        ::mHalIoctl(mmHalFd, MHAL_IOCTL_CAPTURE_CANCEL, NULL, 0, NULL, 0, NULL);
        mStateCond.wait(mStateLock);
    }

    CAM_LOGD("[cancelPicture] -");
    return NO_ERROR;
}


/*******************************************************************************
*
*******************************************************************************/
bool
MTKCameraHardware::
handleMainThreadCommand(void const*const pCmd)
{
    if  ( ! pCmd )
    {
        CAM_LOGD("[handleMainThreadCommand] null cmd");
        return  NO_ERROR;
    }

    CmdQueThread::Command const& rCmd = *reinterpret_cast<CmdQueThread::Command const*>(pCmd);
    status_t status = NO_ERROR;

    switch  (rCmd.eId)
    {
    case CmdQueThread::Command::eID_TAKE_PICTURE:
        status = takePictureProc();
        if  ( NO_ERROR != status )
        {
            CAM_LOGD("  mainThread, takePictureProc err: %d", status);
            // Error callback T.B.D
        }
        break;
    //
    case CmdQueThread::Command::eID_TAKE_PRV_PIC:
        status = takePreviewPictureProc();
        if  ( NO_ERROR != status )
        {
            CAM_LOGD("  mainThread, takePreviewPictureProc err: %d", status);
            // Error callback T.B.D
        }
        break;
    //
    case CmdQueThread::Command::eID_SMOOTH_ZOOM:
        mZoomStopValue = rCmd.u4Ext1;
        mIsSmoothZoom = true;
        status = startSmoothZoom(mZoomValue, mZoomStopValue);
        if  ( NO_ERROR != status )
        {
            CAM_LOGD("  mainThread, takePreviewPictureProc err: %d", status);
            // Error callback T.B.D
        }
        break;
    //
    default:
        CAM_LOGA("mainThread cannot handle bad command::%s", rCmd.name());
        break;
    }

    return  (NO_ERROR==status);
}

/******************************************************************************
*
*******************************************************************************/
status_t MTKCameraHardware::decidePanoMaxDim()
{
    status_t status = NO_ERROR;

    if (mPanoW == 0) {
        #if 0
        // Try to allocate 16MB memory
        int memSize;
        sp<CamMemPool> memPool;
        memSize = 16 * 1024 * 1024;
        memPool = allocMem(memSize, "pano", 1, memSize);
        if (memPool->mMemHeapBase != NULL) {
            mPanoW = 1600;
            mPanoH = 1200;
        }
        else {
            mPanoW = 1280;
            mPanoH = 960;
        }
        memPool = NULL;
        #else
        // Sysram is not enough, use 1280x960 instead
        mPanoW = 1280;
        mPanoH = 960;
        #endif
        CAM_LOGD("[decidePanoMaxDim] W/H: %d/%d \n", mPanoW, mPanoH);
    }

    return status;
}


/******************************************************************************
*
*******************************************************************************/
status_t MTKCameraHardware::startMAV(int num)
{
    status_t status = NO_ERROR;
    CAM_LOGD("[startMAV] %d", num); 
    
    strcpy((char *) mmHalCamParam.u1FileName, mParameters.get(MtkCameraParameters::KEY_CAPTURE_PATH));
    mmHalCamParam.u4BusrtNo = num;
    mmHalCamParam.camExifParam.orientation = mParameters.getInt(MtkCameraParameters::KEY_ROTATION);
    status = ::mHalIoctl(mmHalFd, MHAL_IOCTL_START_MAV, &mmHalCamParam, sizeof(mhalCamParam_t), NULL, 0, NULL);
    if (status != NO_ERROR)
    {
        CAM_LOGD("error occurs in startMAV");
        mNotifyCb(CAMERA_MSG_ERROR, CAMERA_ERROR_RESET, 0, mCallbackCookie);
        status = NO_ERROR; // To avoid Java exception in JNI
    }

    return status;
}

/******************************************************************************
*
*******************************************************************************/
status_t MTKCameraHardware::stopMAV(int isMerge)
{
    status_t status = NO_ERROR;
    CAM_LOGD("[stopMAV] %d", isMerge);

    status = ::mHalIoctl(mmHalFd, MHAL_IOCTL_STOP_MAV, &isMerge, sizeof(int), NULL, 0, NULL);
    if (status != NO_ERROR)
    {
        CAM_LOGD("error occurs in startMAV");
        mNotifyCb(CAMERA_MSG_ERROR, CAMERA_ERROR_RESET, 0, mCallbackCookie);
        status = NO_ERROR; // To avoid Java exception in JNI
    }

    return status;
}

/******************************************************************************
*
*******************************************************************************/
status_t MTKCameraHardware::startAUTORAMA(int num)
{
    status_t status = NO_ERROR;
    CAM_LOGD("[startAUTORAMA] %d", num);
    
    strcpy((char *) mmHalCamParam.u1FileName, mParameters.get(MtkCameraParameters::KEY_CAPTURE_PATH));
    mmHalCamParam.u4BusrtNo = num;
    mmHalCamParam.camExifParam.orientation = mParameters.getInt(MtkCameraParameters::KEY_ROTATION);
    char *gpsLatitude, *gpsLongitude, *gpsTimestamp, *gpsProcessingMethod;
    int gpsAltitude;
    gpsAltitude = mParameters.getInt(MtkCameraParameters::KEY_GPS_ALTITUDE);
    gpsLatitude = (char *) mParameters.get(MtkCameraParameters::KEY_GPS_LATITUDE);
    gpsLongitude = (char *) mParameters.get(MtkCameraParameters::KEY_GPS_LONGITUDE);
    gpsTimestamp = (char *) mParameters.get(MtkCameraParameters::KEY_GPS_TIMESTAMP);
    gpsProcessingMethod = (char *) mParameters.get(MtkCameraParameters::KEY_GPS_PROCESSING_METHOD);
    mmHalCamParam.camExifParam.gpsIsOn = 0;
    if ((gpsLatitude != NULL) && (gpsLongitude != NULL) 
         &&  (gpsTimestamp != NULL) && (gpsProcessingMethod != NULL) ) 
    {
        CAM_LOGD("GPS is recorded");
        strcpy(mmHalCamParam.camExifParam.gpsLongitude, gpsLongitude);
        strcpy(mmHalCamParam.camExifParam.gpsLatitude, gpsLatitude);
        strcpy(mmHalCamParam.camExifParam.gpsTimeStamp, gpsTimestamp);
        strcpy(mmHalCamParam.camExifParam.gpsProcessingMethod, gpsProcessingMethod);
        mmHalCamParam.camExifParam.gpsAltitude = gpsAltitude;
        mmHalCamParam.camExifParam.gpsIsOn = 1;
    }
    else 
    {
        CAM_LOGW("[startAUTORAMA] GPS Information are not avaiable !"); 
    }
    status = ::mHalIoctl(mmHalFd, MHAL_IOCTL_START_AUTORAMA, &mmHalCamParam, sizeof(mhalCamParam_t), NULL, 0, NULL);
    if (status != NO_ERROR)
    {
        CAM_LOGD("error occurs in startAUTORAMA");
        mNotifyCb(CAMERA_MSG_ERROR, CAMERA_ERROR_RESET, 0, mCallbackCookie);
        status = NO_ERROR; // To avoid Java exception in JNI
    }

    return status;
}

/******************************************************************************
*
*******************************************************************************/
status_t MTKCameraHardware::stopAUTORAMA(int isMerge)
{
    status_t status = NO_ERROR;
    CAM_LOGD("[stopAUTORAMA] %d", isMerge);
    
    status = ::mHalIoctl(mmHalFd, MHAL_IOCTL_STOP_AUTORAMA, &isMerge, sizeof(int), NULL, 0, NULL);
    if (status != NO_ERROR)
    {
        CAM_LOGD("error occurs in stopAUTORAMA");
        mNotifyCb(CAMERA_MSG_ERROR, CAMERA_ERROR_RESET, 0, mCallbackCookie);
        status = NO_ERROR; // To avoid Java exception in JNI
    }

    return status;
}


/******************************************************************************
*
*******************************************************************************/
status_t MTKCameraHardware::start3Dshot(int num)
{
    status_t status = NO_ERROR;
    CAM_LOGD("[start3Dshot] %d", num);    
    
    strcpy((char *) mmHalCamParam.u1FileName, mParameters.get(MtkCameraParameters::KEY_CAPTURE_PATH));
    mmHalCamParam.u4BusrtNo = num; 
    mmHalCamParam.camExifParam.orientation = mParameters.getInt(MtkCameraParameters::KEY_ROTATION);
    status = ::mHalIoctl(mmHalFd, MHAL_IOCTL_START_3DSHOT, &mmHalCamParam, sizeof(mhalCamParam_t), NULL, 0, NULL);
    if (status != NO_ERROR)
    {
        CAM_LOGD("error occurs in start3Dshot");
        mNotifyCb(CAMERA_MSG_ERROR, CAMERA_ERROR_RESET, 0, mCallbackCookie);
        status = NO_ERROR; // To avoid Java exception in JNI
    }

    return status;
}

/******************************************************************************
*
*******************************************************************************/
status_t MTKCameraHardware::stop3Dshot(int isMerge)
{
    status_t status = NO_ERROR;
    CAM_LOGD("[stop3Dshot] %d", isMerge);

    status = ::mHalIoctl(mmHalFd, MHAL_IOCTL_STOP_3DSHOT, &isMerge, sizeof(int), NULL, 0, NULL);
    if (status != NO_ERROR)
    {
        CAM_LOGD("error occurs in stop3Dshot");
        mNotifyCb(CAMERA_MSG_ERROR, CAMERA_ERROR_RESET, 0, mCallbackCookie);
        status = NO_ERROR; // To avoid Java exception in JNI
    }

    return status;
}

/******************************************************************************
*
*******************************************************************************/
void
MTKCameraHardware::
returnFrame(int32_t const frameType, void*const frameBuf)
{
#warning "[TODO] frame reference counting by means of map"
}

/******************************************************************************
*
*******************************************************************************/
status_t MTKCameraHardware::sendCommand(int32_t command, int32_t arg1,
                                         int32_t arg2)
{
    status_t status = NO_ERROR;

    CAM_LOGD("[sendCommand] cmd: 0x%x, arg: 0x%x,0x%x \n", command, arg1, arg2);

    switch (command) {
    case CAMERA_CMD_START_SMOOTH_ZOOM:
        if  ( (0 > arg1) || (mZoom_max_value < arg1) ) {
            CAM_LOGD("[sendCommand] ZOOM_MAX_VALUE(%d) <= arg1(%d) \n", mZoom_max_value, arg1);
            status = BAD_VALUE;
            break;
        }
        if  (mZoomValue == arg1) {
            CAM_LOGD("[sendCommand] mZoomValue == arg1(%d) \n", arg1);
            status = NO_ERROR;
            break;
        }
        //mpMainThread->postCommand(CmdQueThread::Command(CmdQueThread::Command::eID_SMOOTH_ZOOM, arg1, true));
        // [Sean] for ICS zoom performance 
        // In video mode, the set zoom will use startSmoothZoom 
        // instead of setParameters()
        status = startSmoothZoom(mZoomValue, arg1); 
        break;
    case CAMERA_CMD_STOP_SMOOTH_ZOOM:
        status = stopSmoothZoom();
        break;
    case CAMERA_CMD_ENABLE_FOCUS_MOVE_MSG:
        CAM_LOGD("[sendCommand] CAMERA_CMD_ENABLE_FOCUS_MOVE_MSG (%d)\n", arg1); 
        mIsAFMoveCallback = (arg1 == 0) ? (0) : (1); 
        status = 0; 
        break; 
    case CAMERA_CMD_START_FACE_DETECTION:
        status = ::mHalIoctl(mmHalFd, MHAL_IOCTL_START_FACE_DETECTION, NULL, 0, NULL, 0, NULL);
        break;
    case CAMERA_CMD_STOP_FACE_DETECTION:
        status = ::mHalIoctl(mmHalFd, MHAL_IOCTL_STOP_FACE_DETECTION, NULL, 0, NULL, 0, NULL);
        break;        
    case CAMERA_CMD_START_SD_PREVIEW:
        status = ::mHalIoctl(mmHalFd, MHAL_IOCTL_START_SD_PREVIEW, NULL, 0, NULL, 0, NULL);
        break;
    case CAMERA_CMD_CANCEL_SD_PREVIEW:
        status = ::mHalIoctl(mmHalFd, MHAL_IOCTL_CANCEL_SD_PREVIEW, NULL, 0, NULL, 0, NULL);
        break;
    case CAMERA_CMD_START_MAV:
        status = startMAV(arg1);
        break;
    case CAMERA_CMD_STOP_MAV:
        status = stopMAV(arg1);
        break;
    case CAMERA_CMD_START_AUTORAMA:
        status = startAUTORAMA(arg1);
        break;
    case CAMERA_CMD_STOP_AUTORAMA:
        status = stopAUTORAMA(arg1);
        break;        
    case CAMERA_CMD_GET_MEM_INFO:
        {
            CameraMemInfo& rInfo = *reinterpret_cast<CameraMemInfo*>(arg1);
            sp<IMemoryBufferPool>   pMemPool;
            if (mVideoMemPool.get()) {
                pMemPool = mVideoMemPool; 
            }           
            else {
                pMemPool = mPreviewMemPool; 
            }

            if  ( sizeof(CameraMemInfo) != arg2 )
            {
                MY_LOGE("<CAMERA_CMD_GET_MEM_INFO> argument size(%d) != struct size(%d)", arg2, sizeof(CameraMemInfo));
                status = BAD_VALUE;
                break;
            }
            if  ( CameraMemInfo::eTYPE_PMEM != rInfo.u4Type )
            {
                MY_LOGE("<CAMERA_CMD_GET_MEM_INFO> type(%d) != PMEM", rInfo.u4Type);
                status = BAD_TYPE;
                break;
            }
            if  ( ! pMemPool.get() )
            {
                MY_LOGE("<CAMERA_CMD_GET_MEM_INFO> NULL mVideoMemPool");
                status = NO_MEMORY;
                break;
            }

            rInfo.u4VABase  =   (uint32_t)pMemPool->getVirAddr();
            rInfo.u4PABase  =   (uint32_t)pMemPool->getPhyAddr();
            rInfo.u4MemSize =   pMemPool->getBufSize();
            rInfo.u4MemCount=   pMemPool->getBufCount();

            MY_LOGI("<CAMERA_CMD_GET_MEM_INFO> type:%d (VA, PA)=(0x%08X, 0x%08X) (u4MemSize, u4MemCount)=(%d, %d)", rInfo.u4Type, rInfo.u4VABase, rInfo.u4PABase, rInfo.u4MemSize, rInfo.u4MemCount);
        }
        break;
    case CAMERA_CMD_CANCEL_CONTINUOUS_SHOT:
        status = cancelPicture();
        break;
    case CAMERA_CMD_SLOW_CONTINUOUS_SHOT:
        mmHalCamParam.u4ContinuousShotSpeed = 2;
        break;
    default:
        status = BaseCamAdapter::sendCommand(command, arg1, arg2);
        break;
    }

    return status;
}
/******************************************************************************
*
*******************************************************************************/
status_t MTKCameraHardware::queryQvFmt(int32_t &width, int32_t &height, String8 &s8FrmFormat)
{
    if (mQvMemPool != NULL) 
    {
        width         = mQvMemPool->getImgWidth();
        height        = mQvMemPool->getImgHeight();
        s8FrmFormat   = mQvMemPool->getImgFormat();

        return NO_ERROR;
    }
    else
    {
        return BAD_VALUE;
    }
}

/******************************************************************************
*
*******************************************************************************/
bool
mHalCamAdapter::
onInitParameters()
{
    setCamFeatureMode(SCENE_MODE_OFF, MEFFECT_OFF);
    initDefaultParameters();
    return  true;
}


/******************************************************************************
*
*******************************************************************************/
bool
MTKCameraHardware::
init()
{
    bool    ret = false;
    status_t status = NO_ERROR;
    //
    MY_LOGI("+ tid(%d), OpenId(%d), getStrongCount(%d)", ::gettid(), getOpenId(), getStrongCount());
    //
    //
    if  ( ! mHalBaseAdapter::init() || ! mmHalFd )
    {
        MY_LOGE("mHalBaseAdapter::init() fail");
        goto lbExit;
    }
    //
    //  preview thread
    {
        mbPreviewThreadAlive = true;
        pthread_attr_t const attr = {0, NULL, 1024 * 1024, 4096, SCHED_RR, RTPM_PRIO_CAMERA_PREVIEW};
        ::pthread_create(&mPreviewThreadHandle, &attr, _previewThread, this);
    }
    //
    //  video thread
    {
        mbVideoThreadAlive = 0x55555555;
        pthread_attr_t const attr = {0, NULL, 1024 * 1024, 4096, SCHED_RR, RTPM_PRIO_CAMERA_RECORD};
        ::pthread_create(&mVideoThreadHandle, &attr, _videoThread, this);
    }
    //
    //  main thread
    mpMainThread = new CmdQueThread(this, &MTKCameraHardware::handleMainThreadCommand);
    if  (
            mpMainThread == 0
        ||  NO_ERROR!=(status = mpMainThread->run())
        )
    {
        MY_LOGE("Fail to run CmdQueThread - (mpMainThread,status)=(%p,%d)", mpMainThread.get(), status);
        goto lbExit;
    }
    //
    // mEventCallback
    
    mEventCallback = new EventCallback();
	if  (
		    mEventCallback == 0
		||  NO_ERROR!=(status =  mEventCallback->run())
        )
    {
    	MY_LOGE("Fail to run mEventCallback - (mEventCallback,status)=(%p,%d)", mEventCallback.get(), status);
    	goto lbExit;
    } 

    ret = true;
lbExit:
    MY_LOGI("- ret(%d), getStrongCount(%d)", ret, getStrongCount());
    return  ret;
}


/******************************************************************************
*
*******************************************************************************/
bool
MTKCameraHardware::
uninit()
{
    MY_LOGI(
        "+ caller tid(%d), OpenId(%d), getStrongCount(%d), mPreviewThreadHandle(%lx), mVideoThreadHandle(%lx),mmHalFd(%p)", 
        ::gettid(), getOpenId(), getStrongCount(), mPreviewThreadHandle, mVideoThreadHandle, mmHalFd
    );

    if  ( mmHalFd )
    {
        //
        {
            Mutex::Autolock _lock(mDispQueLock);
            MY_LOGD("Clear DispQue");
            mDispQue.clear();
            mDispQueCond.broadcast();
            mbPreviewThreadAlive = false;
        }
        ::sem_post(&mSemPrv);
        //
        mbVideoThreadAlive = 0;
        ::sem_post(&mSemVdo);
        //
        if  ( mpMainThread != 0 )
        {
            MY_LOGD(
                "Main Thread: (tid, getStrongCount, mpMainThread)=(%d, %d, %p)", 
                mpMainThread->getTid(), mpMainThread->getStrongCount(), mpMainThread.get()
            );
            mpMainThread->requestExit();
            mpMainThread = NULL;
        }
        //
        //
        
        if  ( mEventCallback != 0 )
        {
            MY_LOGD(
                "FD Thread: (tid, getStrongCount, mEventCallback)=(%d, %d, %p)", 
                mEventCallback->getTid(), mEventCallback->getStrongCount(), mEventCallback.get()
            );
            mEventCallback->requestExit();
            mEventCallback = NULL;
        }
    }
    //
    mHalBaseAdapter::uninit();
    //
    //
    MY_LOGI("- caller tid(%d), getStrongCount(%d)", ::gettid(), getStrongCount());
    return  true;
}


/******************************************************************************
*
*******************************************************************************/
char const*
MTKCameraHardware::
getCamStateStr(ECamState const eState)
{
    static char const* states[] = {
#define STATE_STR(x) #x
        STATE_STR(eCS_Init),
        STATE_STR(eCS_TakingPicture),
#undef STATE_STR
    };
    return states[eState];
}

/******************************************************************************
*
*******************************************************************************/
MTKCameraHardware::ECamState
MTKCameraHardware::
changeState(ECamState const eNewState, bool isLock /*= true*/)
{
    if  (isLock)
    {
        mStateLock.lock();
    }
    //
    if  ( eNewState != meCamState )
    {
        CAM_LOGD("[state transition] %s --> %s", getCamStateStr(meCamState), getCamStateStr(eNewState));
        meCamState = eNewState;
        mStateCond.signal();
    }
    //
    if  (isLock)
    {
        mStateLock.unlock();
    }
    return eNewState;
}


/******************************************************************************
*
*******************************************************************************/
sp<ICamAdapter>
ICamAdapter::
createInstance(String8 const& rName, int32_t const i4OpenId, CamDevInfo const& rDevInfo)
{
    switch  ( rDevInfo.eDevID )
    {
    case eDevId_AtvSensor:
    case eDevId_ImgSensor:
    default:
        return  new mHalCamAdapter(rName, i4OpenId, rDevInfo);
    }
    return  NULL;
}

}; // namespace android

