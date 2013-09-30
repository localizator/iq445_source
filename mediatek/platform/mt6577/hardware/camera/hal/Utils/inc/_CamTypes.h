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

#ifndef _MTK_UTILS_INC__CAM_TYPES_H_
#define _MTK_UTILS_INC__CAM_TYPES_H_
//
#include <utils/RefBase.h>


/******************************************************************************
*
*******************************************************************************/


namespace android {
namespace MtkCamUtils {
/******************************************************************************
*
*******************************************************************************/


/******************************************************************************
*   Index Reference Base.
*******************************************************************************/
class IdxRefBase
{
public:     ////    Operations.
    virtual int32_t     getRefCount(uint_t const index) const       = 0;
    virtual int32_t     incRefCount(uint_t const index)             = 0;
    virtual int32_t     decRefCount(uint_t const index)             = 0;
    //
public:     ////
    //
    virtual             ~IdxRefBase() {};
};


/******************************************************************************
*   
*******************************************************************************/
class CamFrame;
typedef void (*cam_frame_callback) (CamFrame*const pCamFrame);


/******************************************************************************
*   
*******************************************************************************/
class MessageNotifier : public virtual RefBase
{
public:     ////    Interfaces.
    virtual void    subscribeMsgType(int32_t const msgType, void* cookie, cam_frame_callback const frameCb) = 0;
    virtual void    unsubscribeMsgType(int32_t const msgType, void* cookie) = 0;
    //
    virtual         ~MessageNotifier() {};
};


/******************************************************************************
* Interace class abstraction for Camera Adapter to act as a frame provider
* This interface is fully implemented by Camera Adapter
*******************************************************************************/
class FrameNotifier : public MessageNotifier
{
public:     ////    Interfaces.
    virtual void    returnFrame(int32_t const frameType, void*const frameBuf)   = 0;
};


/******************************************************************************
* Wrapper class around Frame Notifier.
*******************************************************************************/
class FrameProvider : public virtual RefBase
{
protected:  ////    fields.
    sp<FrameNotifier>       mpFrameNotifier;
    void*                   mCookie;
    cam_frame_callback      mFrameCb;

public:     ////    Interfaces.
                    FrameProvider(FrameNotifier*const fn, void* cookie, cam_frame_callback const frameCb);
                    ~FrameProvider();
    //
    void            enableFrameNotification(int32_t frameTypes);
    void            disableFrameNotification(int32_t frameTypes);
    void            returnFrame(int32_t const frameType, void*const frameBuf);

};


/******************************************************************************
*
*******************************************************************************/
};  // namespace MtkCamUtils
};  // namespace android
#endif  //_MTK_UTILS_INC__CAM_TYPES_H_

