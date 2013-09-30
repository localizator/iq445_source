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

#ifndef _MTK_UTILS_INC__ICAM_BUFFER_H_
#define _MTK_UTILS_INC__ICAM_BUFFER_H_
//
#include <utils/RefBase.h>
#include <utils/String8.h>
#include <utils/StrongPointer.h>


/******************************************************************************
*
*******************************************************************************/


namespace android {
namespace MtkCamUtils {
/******************************************************************************
*
*******************************************************************************/


/******************************************************************************
*   Memory Buffer Pool Interface.
*******************************************************************************/
class IMemoryBufferPool : public virtual IdxRefBase, public virtual RefBase
{
public:     ////    Attributes.
    virtual const char*     getPoolName() const = 0;
    virtual size_t          getPoolSize() const = 0;
    virtual size_t          getBufSize()  const = 0;
    virtual uint_t          getBufCount() const = 0;
    //
    virtual void*           getVirAddr(uint_t const index = 0) const    = 0;
    virtual void*           getPhyAddr(uint_t const index = 0) const    = 0;
    //
    virtual camera_memory_t*    get_camera_memory()                 = 0;
    //
public:     ////
    //
    virtual                 ~IMemoryBufferPool() {};
};


/******************************************************************************
*   Image Buffer Pool Interface.
*******************************************************************************/
class IImageBufferPool : public IMemoryBufferPool
{
public:     ////    Attributes.
    virtual String8 const&  getImgFormat()      const   = 0;
    virtual uint32_t        getImgWidth()       const   = 0;
    virtual uint32_t        getImgHeight()      const   = 0;
    virtual uint32_t        getBitsPerPixel()   const   = 0;
    //
public:     ////
    //
    virtual                 ~IImageBufferPool() {};
};


/******************************************************************************
*   Buffer Provider Interface.
*******************************************************************************/
class IBufferProvider
{
public:     ////    camera_memory_t.
    //
    virtual bool    allocBuffer(camera_memory_t& rBuf, size_t const bufSize, uint32_t const bufCount) const   = 0;
    virtual bool    freeBuffer(camera_memory_t& rBuf) const   = 0;
    //
public:     ////    IMemoryBufferPool
    //
    virtual bool    allocBuffer(
                        sp<IMemoryBufferPool>&  rpBuf, 
                        size_t const            bufSize, 
                        uint32_t const          bufCount, 
                        char const*const        szName = ""
                    ) const = 0;
    //
public:     ////    IImageBufferPool
    //
    virtual bool    allocBuffer(
                        sp<IImageBufferPool>&   rpBuf, 
                        char const*const        szImgFormat, 
                        uint32_t const          u4ImgWidth, 
                        uint32_t const          u4ImgHeight, 
                        uint32_t const          bufCount, 
                        char const*const        szName = ""
                    ) const = 0;
    //
public:     ////
    //
    virtual         ~IBufferProvider() {};
};


/******************************************************************************
*
*******************************************************************************/
};  // namespace MtkCamUtils
}; // namespace android
#endif  //_MTK_UTILS_INC__ICAM_BUFFER_H_

