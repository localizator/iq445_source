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

#ifndef _MTK_UTILS_INC__CAM_BUFFER_H_
#define _MTK_UTILS_INC__CAM_BUFFER_H_
//
#include <utils/String8.h>
#include <utils/Vector.h>


/******************************************************************************
*
*******************************************************************************/


namespace android {
namespace MtkCamUtils {
/******************************************************************************
*
*******************************************************************************/
class IMemoryBufferPool;


/******************************************************************************
*   Camera Memory Buffer Pool
*******************************************************************************/
class CamMemBufPool : public IMemoryBufferPool
{
public:     ////    Index Reference Base.
    virtual int32_t         getRefCount(uint_t const index) const;
    virtual int32_t         incRefCount(uint_t const index);
    virtual int32_t         decRefCount(uint_t const index);
    //
public:     ////    Attributes.
    virtual const char*     getPoolName() const { return ms8PoolName.string(); }
    virtual size_t          getPoolSize() const { return mCamMem.size; }
    virtual size_t          getBufSize()  const { return mBufSize; }
    virtual uint_t          getBufCount() const { return mBufCount; }
    //
    virtual void*           getVirAddr(uint_t const index /*= 0*/) const
                            {
                                return (index >= mBufCount) ? 0 : (void*)(mu4VirBase + index * mBufSize);
                            }
    virtual void*           getPhyAddr(uint_t const index /*= 0*/) const
                            {
                                return (index >= mBufCount) ? 0 : (void*)(mu4PhyBase + index * mBufSize);
                            }
    //
    virtual camera_memory_t*    get_camera_memory() { return &mCamMem; }
    //
public:     ////    Instantiation.
    //
                            CamMemBufPool(
                                camera_memory_t const mem, 
                                size_t const bufSize, 
                                uint32_t const bufCount, 
                                char const*const poolName = ""
                            );
    virtual                 ~CamMemBufPool();
    //
protected:  ////    Data Members.
    Vector<int32_t>         mvRefCount;
    //
    String8                 ms8PoolName;
    size_t                  mBufSize;
    uint_t                  mBufCount;
    uint32_t                mu4VirBase;
    uint32_t                mu4PhyBase;
    camera_memory_t         mCamMem;
};


/******************************************************************************
*   Camera Image Buffer Pool
*******************************************************************************/
class CamImgBufPool : public IImageBufferPool
{
public:     ////    Index Reference Base.
    virtual int32_t         getRefCount(uint_t const index) const;
    virtual int32_t         incRefCount(uint_t const index);
    virtual int32_t         decRefCount(uint_t const index);
    //
public:     ////    Attributes.
    virtual const char*     getPoolName() const { return ms8PoolName.string(); }
    virtual size_t          getPoolSize() const { return mCamMem.size; }
    virtual size_t          getBufSize()  const { return mBufSize; }
    virtual uint_t          getBufCount() const { return mBufCount; }
    //
    virtual void*           getVirAddr(uint_t const index /*= 0*/) const
                            {
                                return (index >= mBufCount) ? 0 : (void*)(mu4VirBase + index * mBufSize);
                            }
    virtual void*           getPhyAddr(uint_t const index /*= 0*/) const
                            {
                                return (index >= mBufCount) ? 0 : (void*)(mu4PhyBase + index * mBufSize);
                            }
    //
    virtual camera_memory_t*    get_camera_memory() { return &mCamMem; }
    //
public:     ////    Image Attributes.
    virtual String8 const&  getImgFormat()      const   { return ms8ImgFormat; }
    virtual uint32_t        getImgWidth()       const   { return mu4ImgWidth; }
    virtual uint32_t        getImgHeight()      const   { return mu4ImgHeight; }
    virtual uint32_t        getBitsPerPixel()   const   { return mu4BitsPerPixel; }
    //
public:     ////    Instantiation.
    //
                            CamImgBufPool(
                                camera_memory_t const mem, 
                                size_t const bufSize, 
                                uint32_t const bufCount, 
                                uint32_t const u4ImgWidth, 
                                uint32_t const u4ImgHeight, 
                                char const*const ImgFormat, 
                                char const*const poolName = ""
                            );
    virtual                 ~CamImgBufPool();
    //
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data Members.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////    Reference.
    Vector<int32_t>         mvRefCount;
    //
protected:  ////    Memory.
    String8                 ms8PoolName;
    size_t                  mBufSize;
    uint_t                  mBufCount;
    uint32_t                mu4VirBase;
    uint32_t                mu4PhyBase;
    camera_memory_t         mCamMem;
    //
protected:  ////    Image.
    String8                 ms8ImgFormat;
    uint32_t                mu4ImgWidth;
    uint32_t                mu4ImgHeight;
    uint32_t                mu4BitsPerPixel;
};


/******************************************************************************
*
*******************************************************************************/
};  // namespace MtkCamUtils
};  // namespace android
#endif  //_MTK_UTILS_INC__CAM_BUFFER_H_

