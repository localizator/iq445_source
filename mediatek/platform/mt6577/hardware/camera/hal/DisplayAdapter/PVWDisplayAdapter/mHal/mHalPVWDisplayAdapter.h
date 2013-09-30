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

#ifndef _MTK_DISPLAY_ADAPTER_MHAL_PREVIEW_WINDOW_DISPLAY_ADAPTER_H_
#define _MTK_DISPLAY_ADAPTER_MHAL_PREVIEW_WINDOW_DISPLAY_ADAPTER_H_
//
#include <utils/Errors.h>
#include <utils/threads.h>
#include <utils/String8.h>
#include <utils/KeyedVector.h>
//
#include <hardware/camera.h>
#include <system/camera.h>
//
#include "inc/IDisplayAdapter.h"
#include "PVWDisplayAdapter/PVWDisplayAdapter.h"


/******************************************************************************
*
*******************************************************************************/
#define USE_HW_MEMCPY               (1)
#define USE_HW_MEMCPY_MAP_BUF_DST   (1)
#define USE_HW_MEMCPY_MAP_BUF_SRC   (1)
//----------------------------------------------------------
#if ( USE_HW_MEMCPY == 0 )
  #define USE_IMG_BUF_MAPPER        (0)
  #undef  USE_HW_MEMCPY_MAP_BUF_DST
  #define USE_HW_MEMCPY_MAP_BUF_DST (0)
  #undef  USE_HW_MEMCPY_MAP_BUF_SRC
  #define USE_HW_MEMCPY_MAP_BUF_SRC (0)
#elif   ( USE_HW_MEMCPY_MAP_BUF_DST == 0 && USE_HW_MEMCPY_MAP_BUF_SRC == 0 )
  #define USE_IMG_BUF_MAPPER        (0)
#else
  #define USE_IMG_BUF_MAPPER        (1)
#endif


/******************************************************************************
*
*******************************************************************************/
namespace android {
/******************************************************************************
*
*******************************************************************************/


/******************************************************************************
* Image Buffer Mapper
*******************************************************************************/
#if (USE_IMG_BUF_MAPPER)
class ImgBufMapper
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////    Instantiation.
                    ImgBufMapper(char const* name = "");
    virtual         ~ImgBufMapper();

public:     ////    Attributes.
    virtual size_t  size() const                    { return mMap.size(); }
    void*           valueFor(void*const key) const  { return mMap.valueFor(key); }

public:     ////    Operations.
    virtual void    clear();
    virtual bool    add_IfNotFound(
                        void*const      pBuf, 
                        bool const      bIsOut, 
                        int32_t const   i4Width, 
                        int32_t const   i4Height, 
                        int32_t const   i4HalPixelFormat
                    );

protected:  ////    Operations.
    virtual bool    add(
                        void*const      pBuf, 
                        bool const      bIsOut, 
                        int32_t const   i4Width, 
                        int32_t const   i4Height, 
                        int32_t const   i4HalPixelFormat
                    );

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////    
    Mutex           mMutex;
    String8 const   ms8Name;
    typedef DefaultKeyedVector<void*, void*> Map_t;
    Map_t           mMap;
};
#endif  //USE_IMG_BUF_MAPPER


/******************************************************************************
* Preview Window Display Adapter
*******************************************************************************/
class mHalPVWDisplayAdapter : public PVWDisplayAdapter
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Inherited Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////
                        mHalPVWDisplayAdapter();
    virtual             ~mHalPVWDisplayAdapter();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////    
    virtual bool        copyFrame(int const iDstStride, void* pDstBuf, DispFrame const& rDispFrame);
    virtual bool        onDestroy();

protected:  ////    Image Buffer Mapper
    //
#if (USE_HW_MEMCPY_MAP_BUF_DST)
    ImgBufMapper        mDstImgBufMap;  //  Map for Destination Image Buffers.
#endif
    //
#if (USE_HW_MEMCPY_MAP_BUF_SRC)
    ImgBufMapper        mSrcImgBufMap;  //  Map for Source Image Buffers.
#endif
};


}; // namespace android
#endif  //_MTK_DISPLAY_ADAPTER_MHAL_PREVIEW_WINDOW_DISPLAY_ADAPTER_H_

