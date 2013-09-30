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

#ifndef _MTK_CAM_ADAPTER_MHAL_CAM_ADAPTER_H_
#define _MTK_CAM_ADAPTER_MHAL_CAM_ADAPTER_H_


#include <semaphore.h>
#include <linux/android_pmem.h>
//
#include <utils/RefBase.h>
#include <utils/StrongPointer.h>
//
#include <utils/List.h>
//
#include <binder/MemoryBase.h>
#include <camera/MtkCamera.h>
#include <camera/MtkCameraParameters.h>
//
#include <mhal/inc/camera.h>
//

namespace android {

class ICamAdapter;
class BaseCamAdapter;
class mHalBaseAdapter;
class CmdQueThread;
class EventCallback;

#define mHalCamAdapter  MTKCameraHardware

class MTKCameraHardware : public mHalBaseAdapter
{
    friend  class ICamAdapter;
    friend  class CmdQueThread;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////    
    //
    virtual bool                    init();
    virtual bool                    uninit();
    //
    virtual status_t                startPreview();
    virtual void                    stopPreview();
    virtual bool                    previewEnabled();
    //
    virtual status_t                startRecording();
    virtual void                    stopRecording();
    virtual bool                    recordingEnabled();
    virtual void                    releaseRecordingFrame(const void *opaque);
    //
    virtual status_t                autoFocus();
    virtual status_t                cancelAutoFocus();
    //
    virtual status_t                takePicture();
    virtual status_t                cancelPicture();
    //
    virtual bool                    isParameterValid(const char *param, char *supportedParams);
    virtual bool                    isAreaValid(const char *param, const char *maxNum);
    virtual status_t                setParameters(const CameraParameters& params);
    virtual CameraParameters&       getParameters();
    //
    virtual status_t                sendCommand(int32_t cmd, int32_t arg1, int32_t arg2);
    virtual status_t 				queryQvFmt(int32_t &width, int32_t &height, String8 &s8FrmFormat);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  FrameNotifier Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////    
    virtual void    returnFrame(int32_t const frameType, void*const frameBuf);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Proprietary.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
    MTKCameraHardware(String8 const& rName, int32_t const i4OpenId, CamDevInfo const& rDevInfo);
    virtual ~MTKCameraHardware();
    virtual bool            onInitParameters();

    //+++++ parameter related begin +++++
    char* getFeatureIDToStr(int featureID);
    char* getFeatureEnumToStr(int featureID, int featureVal);
    void setCamFeatureMode(int sceneMode, int effectMode);
    void getCamFeatureEnums();
    char* getSupportedValuesSuffix(char *pname);
    int getSupportedValueToEnum(const char *pstrVal, const char *pstrValList, const char *pstrEnumList);
    void initDefaultParameters();
    void initZoomParam();
    uint32_t getZoomValue(int index);
    void getCamPreviewSupportFmt();
    void mapCam3AParameter(mhalCam3AParam_t& cam3aParam, const MtkCameraParameters& params, const MtkCameraParameters& pfeature);
    void setCam3AParameter(const MtkCameraParameters& params);
    void setCamFlashlightConf(const MtkCameraParameters& params);
    void getFocusWindowParam(MtkCameraParameters& params) const;
    //
    void                updateShotMode(String8 const& rs8ShotMode);
    uint32_t volatile   mu4ShotMode;
    // 
    status_t parseCamAreas(const char *areaStr, int areaLength, List<camera_area_t> &areas, const int maxnumArea);
    status_t parsePreviewSize(const char *sizeStr, int sizeLength);
    bool checkCamArea(camera_area_t const &area);
	
    // Engineer mode 
    void mapEng3AParameter(mhalCam3AParam_t& cam3aParam, const MtkCameraParameters& params);
    void defaultEngParameter(MtkCameraParameters& params);
	
    //----- Parameter related end -----

    /**
     * camera state
     */
    enum ECamState {
        eCS_Init, 
        eCS_TakingPicture,  //  taking picture...
    };
    ECamState volatile  meCamState;
    Mutex               mStateLock;
    Condition           mStateCond;

    static char const*  getCamStateStr(ECamState const eState);
    ECamState           changeState(ECamState const eNewState, bool isLock = true);

    mhalCamParam_t  mmHalCamParam;
    int32_t volatile    mIsPreviewEnabled;
    bool mIsVideoEnabled;
    //#define ZOOM_MAX_VALUE  10  
    int mZoom_max_value; 	    
    int mZoomValue;
    int mZoomStopValue;
    bool mIsSmoothZoom;
    //
    bool mIsSmoothZoomCBEnabled;    //  Smooth zoom callback: enabled or not.
    bool mIsDisplay;

    /**
     * takePicture() resets it to false.
     * cancelPicture() sets it to true.
     */
    bool volatile mIsCancelPicture;

    //bool mIsZoomReset;
    bool mIsATV;
    bool mIsTakePrv;
    bool mIsATVDelay;
    int mATVDispBufNo;
    long long mATVStartTime;
    //add by tengfei, mtk80343 for AV sync
    int mATVbuffers[12];
    unsigned int mAtvDispDelay;
    int pushindex;
    int popindex;
    int mPanoW;
    int mPanoH;
    int mPanoNum;
    int mstrobeMode;
    int mCurFramerate;
    //
    bool mIsZSD;
    //
    int mDispBufNo;
    //
    mutable MtkCameraParameters mParameters;
    MtkCameraParameters mFeatureParam;
    //
    sp<IMemoryBufferPool>   allocMem(size_t const bufsize, uint_t const numbufs, char const*const szName) const;
    sp<IMemoryBufferPool>   allocCamMem(int poolID, char const*const szName, int frmW, int frmH) const;
    sp<IImageBufferPool>    mPreviewMemPool;
    sp<IImageBufferPool>    mQvMemPool;
    sp<IMemoryBufferPool>   mJpegMemPool;
    sp<IMemoryBufferPool>   mVideoMemPool;
    sp<IMemoryBufferPool>   mPrvRgbMemPool;
    sp<IMemoryBufferPool>   mPrvJpegMemPool;
    sp<IMemoryBufferPool>   mPanoMemPool;
    //
    status_t doZoom(int currZoom, int nextZoom);
    //
    int32_t         mi4PreviewWidth;
    int32_t         mi4PreviewHeight;
    //
    sem_t           mSemPrv;
    bool volatile   mbPreviewThreadAlive;
    pthread_t       mPreviewThreadHandle;
    static void*    _previewThread(void* arg);
    void*           mAtvPreviewThread();
    void*           cameraPreviewThread();
    //
	int volatile 	    maVdoCbBufNo[8];
    long long volatile  maVdoTimStampNs[8];
    int 			    miVdoInIndex;
    int 			    miVdoOutIndex;
    sem_t               mSemVdo;
    int32_t volatile    mbVideoThreadAlive;
    pthread_t           mVideoThreadHandle;
    static void*        _videoThread(void* arg);
    void*               cameraVideoThread();
	
    bool            postPreviewFrame(int32_t i4Index = 0, int64_t i8Timestamp = 0);
    status_t        startPreviewInternal();
    status_t        stopPreviewInternal();
    //
    sp<CmdQueThread>mpMainThread;
    virtual bool    handleMainThreadCommand(void const*const pCmd);
    //
    sp<EventCallback> mEventCallback;	   
    //
    sem_t           mSemTakePicBack;
    status_t        takePictureProc();
    status_t        takePictureInternal();
    //
    static void     mhalCallback(void* param);
    static void     shotCallback(void* param);
    void            dispatchCallback(void*const param);
    void previewCallback(void *param);
    void videoRecordCallback(void *param); 
    //add by tengfei mtk80343
    void mATVbufferFIFOin(int pcbInfo_dispBufNo);
    void mATVbufferFIFOout();
    //
    status_t allocCamCaptureMem();
    status_t freeCamCaptureMem(); 
    //
    status_t takePreviewPictureProc();
    status_t takePreviewPictureInternal();
    status_t decidePanoMaxDim();
    status_t startSmoothZoom(int currZoom, int finalZoom);
    status_t stopSmoothZoom();
    status_t startMAV(int num);
    status_t stopMAV(int isMerge);
    status_t startAUTORAMA(int num);
    status_t stopAUTORAMA(int isMerge);
    status_t start3Dshot(int num);
    status_t stop3Dshot(int isMerge);
    //
    bool mIsAFMoveCallback; 
    //
    long long mFocusStartTime;
    long long mCaptureStartTime;
    long long mFocusCallbackTime;
    long long mShutterCallbackTime;
    long long mRawCallbackTime;
    long long mJpegCallbackTime;

    struct  DispQueNode
    {
        int32_t     mi4Index;
        int64_t     mi8Timestamp;
                    DispQueNode(int32_t i4Index = 0, int64_t i8Timestamp = 0)
                        : mi4Index(i4Index)
                        , mi8Timestamp(i8Timestamp)
                    {}
    };
    Vector<DispQueNode>     mDispQue;
    Condition               mDispQueCond;
    Mutex                   mDispQueLock;
};


}; // namespace android
#endif  //_MTK_CAM_ADAPTER_MHAL_CAM_ADAPTER_H_

