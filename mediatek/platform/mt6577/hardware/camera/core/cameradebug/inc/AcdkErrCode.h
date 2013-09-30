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

///////////////////////////////////////////////////////////////////////////////
// No Warranty
// Except as may be otherwise agreed to in writing, no warranties of any
// kind, whether express or implied, are given by MTK with respect to any MTK
// Deliverables or any use thereof, and MTK Deliverables are provided on an
// "AS IS" basis.  MTK hereby expressly disclaims all such warranties,
// including any implied warranties of merchantability, non-infringement and
// fitness for a particular purpose and any warranties arising out of course
// of performance, course of dealing or usage of trade.  Parties further
// acknowledge that Company may, either presently and/or in the future,
// instruct MTK to assist it in the development and the implementation, in
// accordance with Company's designs, of certain softwares relating to
// Company's product(s) (the "Services").  Except as may be otherwise agreed
// to in writing, no warranties of any kind, whether express or implied, are
// given by MTK with respect to the Services provided, and the Services are
// provided on an "AS IS" basis.  Company further acknowledges that the
// Services may contain errors, that testing is important and Company is
// solely responsible for fully testing the Services and/or derivatives
// thereof before they are used, sublicensed or distributed.  Should there be
// any third party action brought against MTK, arising out of or relating to
// the Services, Company agree to fully indemnify and hold MTK harmless.
// If the parties mutually agree to enter into or continue a business
// relationship or other arrangement, the terms and conditions set forth
// hereunder shall remain effective and, unless explicitly stated otherwise,
// shall prevail in the event of a conflict in the terms in any agreements
// entered into between the parties.
////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2008, MediaTek Inc.
// All rights reserved.
//
// Unauthorized use, practice, perform, copy, distribution, reproduction,
// or disclosure of this information in whole or in part is prohibited.
////////////////////////////////////////////////////////////////////////////////
// AcdkErrCode.h  $Revision$
////////////////////////////////////////////////////////////////////////////////

//! \file  AcdkErrCode.h
//! \brief

#ifndef _ACDKERRCODE_H_
#define _ACDKERRCODE_H_

#include "AcdkTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

///////////////////////////////////////////////////////////////////////////
//! Helper macros to define error code
///////////////////////////////////////////////////////////////////////////
#define ERRCODE(modid, errid)           \
  ((MINT32)                              \
    ((MUINT32)(0x80000000) |             \
     (MUINT32)((modid & 0x7f) << 24) |   \
     (MUINT32)(errid & 0xffff))          \
  )

#define OKCODE(modid, okid)             \
  ((MINT32)                              \
    ((MUINT32)(0x00000000) |             \
     (MUINT32)((modid & 0x7f) << 24) |   \
     (MUINT32)(okid & 0xffff))           \
  )

///////////////////////////////////////////////////////////////////////////
//! Helper macros to check error code
///////////////////////////////////////////////////////////////////////////
#define SUCCEEDED(Status)   ((MRESULT)(Status) >= 0)
#define FAILED(Status)      ((MRESULT)(Status) < 0)

enum  // Use enum to check duplicated error ID
{  
// Tool\manufacture
    MODULE_ACDK_BASE                        = 0x90, 
    MODULE_ACDK_CALIBRATION           = 0x91,
    MODULE_ACDK_ISPTUNING              = 0x92,
    MOUDLE_ACDK_IMGTOOL                 = 0x93,
    MODULE_ACDK_CAMCTRL                 = 0x94,
    MODULE_ACDK_SENSOR		     = 0x95, 
    MODULE_ACDK_3ACTRL			     = 0x96,
    MODULE_ACDK_IF                            = 0x97,
    MODULE_ACDK_CCAP                       = 0x98, 
};  


///////////////////////////////////////////////////////////////////////////////
//! Error Code for ACDKBase 
///////////////////////////////////////////////////////////////////////////////
#define ACDK_BASE_OKCODE(errid)          OKCODE(MODULE_ACDK_BASE, errid)
#define ACDK_BASE_ERRCODE(errid)         ERRCODE(MODULE_ACDK_BASE, errid)
enum 
{
    S_ACDK_BASE_OK            = ACDK_BASE_OKCODE(0),  //!<: OK
    E_ACDK_BASE_BAD_ARG       = ACDK_BASE_ERRCODE(0x0001),    //!< bad arguments
    E_ACDK_BASE_API_FAIL      = ACDK_BASE_ERRCODE(0x0002),    //!< API Fail
    E_ACDK_BASE_NULL_OBJ      = ACDK_BASE_ERRCODE(0x0003),    //!< Null Obj 
    E_ACDK_BASE_MEMORY_MAX    = ACDK_BASE_ERRCODE(0x0080)    //!< Max error code 
};



///////////////////////////////////////////////////////////////////////////////
//! \brief Error Code generated to the the caller as compliant to the error code format
//! and passed as MResult
//! \note
//!     - This makes use of the TuningTool manager error Indexes to specify success or
//!        a specific type of error code as an example only. For other CLI clients,
//!        please use the appropriate error header file and module ID
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//! Error Code for ISP Tuning
///////////////////////////////////////////////////////////////////////////////
#define ACDK_ISPTUNING_OKCODE(errid)          OKCODE(MODULE_ACDK_ISPTUNING, errid)
#define ACDK_ISPTUNING_ERRCODE(errid)         ERRCODE(MODULE_ACDK_ISPTUNING, errid)

enum 
{
    S_ACDK_ISPTUNING_OK            = ACDK_ISPTUNING_OKCODE(0),  //!<: OK
    E_ACDK_ISPTUNING_BAD_ARG       = ACDK_ISPTUNING_ERRCODE(0x0001),    //!< bad arguments
    E_ACDK_ISPTUNING_API_FAIL      = ACDK_ISPTUNING_ERRCODE(0x0002),    //!< API Fail
    E_ACDK_ISPTUNING_NULL_OBJ      = ACDK_ISPTUNING_ERRCODE(0x0003),    //!< Null Obj 
    E_ACDK_ISPTUNING_TIMEOUT       = ACDK_ISPTUNING_ERRCODE(0x0004),    //!< Time out 
    E_ACDK_ISPTUNING_MEMORY_MAX    = ACDK_ISPTUNING_ERRCODE(0x0080)    //!< Max error code 
};

///////////////////////////////////////////////////////////////////////////////
//! Error Code for Image Tool 
///////////////////////////////////////////////////////////////////////////////
#define ACDK_IMGTOOL_OKCODE(errid)          OKCODE(MOUDLE_ACDK_IMGTOOL, errid)
#define ACDK_IMGTOOL_ERRCODE(errid)         ERRCODE(MOUDLE_ACDK_IMGTOOL, errid)

enum 
{
    S_ACDK_IMGTOOL_OK               = ACDK_IMGTOOL_OKCODE(0),
    E_ACDK_IMGTOOL_BAD_ARG          = ACDK_IMGTOOL_ERRCODE(0x0001),    //!< bad arguments
    E_ACDK_IMGTOOL_API_FAIL         = ACDK_IMGTOOL_ERRCODE(0x0002),    //!< API Fail
    E_ACDK_IMGTOOL_NULL_OBJ         = ACDK_IMGTOOL_ERRCODE(0x0003),    //!< Null Obj 
    E_ACDK_IMGTOOL_TIMEOUT          = ACDK_IMGTOOL_ERRCODE(0x0004),    //!< Time out 
    E_ACDK_IMGTOOL_FILE_OPEN_FAIL   = ACDK_IMGTOOL_ERRCODE(0x0005), 
    E_ACDK_IMGTOOL_MEMORY_MAX       = ACDK_IMGTOOL_ERRCODE(0x0080)    //!< Max error code     
};

///////////////////////////////////////////////////////////////////////////////
//! Error Code for Calibration Tool 
///////////////////////////////////////////////////////////////////////////////
#define ACDK_CALIBRATION_OKCODE(errid)          OKCODE(MODULE_ACDK_CALIBRATION, errid)
#define ACDK_CALIBRATION_ERRCODE(errid)         ERRCODE(MODULE_ACDK_CALIBRATION, errid)

enum 
{
    S_ACDK_CALIBRATION_OK               = ACDK_CALIBRATION_OKCODE(0),
    E_ACDK_CALIBRATION_BAD_ARG          = ACDK_CALIBRATION_ERRCODE(0x0001),    //!< bad arguments
    E_ACDK_CALIBRATION_API_FAIL         = ACDK_CALIBRATION_ERRCODE(0x0002),    //!< API Fail
    E_ACDK_CALIBRATION_NULL_OBJ         = ACDK_CALIBRATION_ERRCODE(0x0003),    //!< Null Obj 
    E_ACDK_CALIBRATION_TIMEOUT          = ACDK_CALIBRATION_ERRCODE(0x0004),    //!< Time out 
    E_ACDK_CALIBRATION_GET_FAIL         = ACDK_CALIBRATION_ERRCODE(0x0005),    //!< Get calibration result fail
    E_ACDK_CALIBRATION_FILE_OPEN_FAIL   = ACDK_CALIBRATION_ERRCODE(0x0006),    //!< Open File Fail 
    E_ACDK_CALIBRATION_DISABLE                = ACDK_CALIBRATION_ERRCODE(0x0007),
    E_ACDK_CALIBRATION_MEMORY_MAX       = ACDK_CALIBRATION_ERRCODE(0x0080)     //!< Max error code     
};

///////////////////////////////////////////////////////////////////////////////
//! Error Code for Camera Control 
///////////////////////////////////////////////////////////////////////////////
#define ACDK_CAMCTRL_OKCODE(errid)                 OKCODE(MODULE_ACDK_CAMCTRL, errid) 
#define ACDK_CAMCTRL_ERRCODE(errid)                ERRCODE(MODULE_ACDK_CAMCTRL, errid)

enum 
{
    S_ACDK_CAMCTRL_OK                  = ACDK_CAMCTRL_OKCODE(0),
    E_ACDK_CAMCTRL_OPEN_FAIL           = ACDK_CAMCTRL_ERRCODE(0x0001), 
    E_ACDK_CAMCTRL_NO_MEMORY              = ACDK_CAMCTRL_ERRCODE(0x0002),
    E_ACDK_CAMCTRL_API_FAIL            = ACDK_CAMCTRL_ERRCODE(0x0003),
    E_ACDK_CAMCTRL_BAD_ARG             = ACDK_CAMCTRL_ERRCODE(0x0004), 
    E_ACDK_CAMCTRL_NULL_OBJ            = ACDK_CAMCTRL_ERRCODE(0x0005), 
   
};

///////////////////////////////////////////////////////////////////////////////
//! Error Code for Sensor 
///////////////////////////////////////////////////////////////////////////////
#define ACDK_SENSOR_OKCODE(errid)                       OKCODE(MODULE_ACDK_SENSOR, errid) 
#define ACDK_SENSOR_ERRCODE(errid)                      ERRCODE(MODULE_ACDK_SENSOR, errid) 

enum 
{
    S_ACDK_SENSOR_OK                     = ACDK_SENSOR_OKCODE(0), 
    E_ACDK_SENSOR_API_FAIL               = ACDK_SENSOR_ERRCODE(0x0001), 
    E_ACDK_SENSOR_OPEN_FAIL                = ACDK_SENSOR_ERRCODE(0x0002), 
    E_ACDK_SENSOR_DISABLE			= ACDK_SENSOR_ERRCODE(0x0003), 
    E_ACDK_SENSOR_NULL_OBJ            = ACDK_SENSOR_ERRCODE(0x0004), 
    E_ACDK_SENSOR_BAD_ARG             = ACDK_SENSOR_ERRCODE(0x0005),
};

///////////////////////////////////////////////////////////////////////////////
//! Error Code for 3A 
///////////////////////////////////////////////////////////////////////////////
#define ACDK_3ACTRL_OKCODE(errid)                       OKCODE(MODULE_ACDK_3ACTRL, errid) 
#define ACDK_3ACTRL_ERRCODE(errid)                      ERRCODE(MODULE_ACDK_3ACTRL, errid) 

enum 
{
    S_ACDK_3ACTRL_OK                      = ACDK_3ACTRL_OKCODE(0), 
    E_ACDK_3ACTRL_API_FAIL               = ACDK_3ACTRL_ERRCODE(0x0001), 
    E_ACDK_3ACTRL_NULL_OBJ               = ACDK_3ACTRL_ERRCODE(0x0002), 
    E_ACDK_3ACTRL_BAD_ARG                = ACDK_3ACTRL_ERRCODE(0x0003), 
};


///////////////////////////////////////////////////////////////////////////////
//! Error Code for ACDK IF  
///////////////////////////////////////////////////////////////////////////////
#define ACDK_IF_OKCODE(errid)                       OKCODE(MODULE_ACDK_IF, errid) 
#define ACDK_IF_ERRCODE(errid)                      ERRCODE(MODULE_ACDK_IF, errid) 

enum 
{
              S_ACDK_IF_OK                      = ACDK_IF_OKCODE(0), 
              E_ACDK_IF_API_FAIL               = ACDK_IF_ERRCODE(0x0001), 
              E_ACDK_IF_NO_SUPPORT_FOMAT = ACDK_IF_ERRCODE(0x0002),
              E_ACDK_IF_IS_ACTIVED                = ACDK_IF_ERRCODE(0x0003), 
              E_ACDK_IF_INVALID_DRIVER_MOD_ID  = ACDK_IF_ERRCODE(0x0004),
              E_ACDK_IF_INVALID_FEATURE_ID = ACDK_IF_ERRCODE(0x0005), 
              E_ACDK_IF_INVALID_SCENARIO_ID = ACDK_IF_ERRCODE(0x0006), 
              E_ACDK_IF_INVALID_CTRL_CODE = ACDK_IF_ERRCODE(0x0007), 
              E_ACDK_IF_VIDEO_ENCODER_BUSY = ACDK_IF_ERRCODE(0x0008),
              E_ACDK_IF_INVALID_PARA = ACDK_IF_ERRCODE(0x0009),
              E_ACDK_IF_OUT_OF_BUFFER_NUMBER = ACDK_IF_ERRCODE(0x000A),
              E_ACDK_IF_INVALID_ISP_STATE = ACDK_IF_ERRCODE(0x000B),
              E_ACDK_IF_INVALID_ACDK_STATE = ACDK_IF_ERRCODE(0x000C),
              E_ACDK_IF_PHY_VIR_MEM_MAP_FAIL = ACDK_IF_ERRCODE(0x000D),
              E_ACDK_IF_ENQUEUE_BUFFER_NOT_FOUND = ACDK_IF_ERRCODE(0x000E),
              E_ACDK_IF_BUFFER_ALREADY_INIT= ACDK_IF_ERRCODE(0x000F),
              E_ACDK_IF_BUFFER_OUT_OF_MEMORY = ACDK_IF_ERRCODE(0x0010),
              E_ACDK_IF_SENSOR_POWER_ON_FAIL = ACDK_IF_ERRCODE(0x0011),
              E_ACDK_IF_SENSOR_CONNECT_FAIL = ACDK_IF_ERRCODE(0x0012),
              E_ACDK_IF_IO_CONTROL_CODE = ACDK_IF_ERRCODE(0x0013),
              E_ACDK_IF_IO_CONTROL_MSG_QUEUE_OPEN_FAIL = ACDK_IF_ERRCODE(0x0014),
              E_ACDK_IF_DRIVER_INIT_FAIL = ACDK_IF_ERRCODE(0x0015),
              E_ACDK_IF_WRONG_NVRAM_CAMERA_VERSION = ACDK_IF_ERRCODE(0x0016),
              E_ACDK_IF_NVRAM_CAMERA_FILE_FAIL = ACDK_IF_ERRCODE(0x0017),
              E_ACDK_IF_IMAGE_DECODE_FAIL = ACDK_IF_ERRCODE(0x0018),
              E_ACDK_IF_IMAGE_ENCODE_FAIL = ACDK_IF_ERRCODE(0x0019),
              E_ACDK_IF_LED_FLASH_POWER_ON_FAIL =ACDK_IF_ERRCODE(0x001A),
};

///////////////////////////////////////////////////////////////////////////////
//! Error Code for 3A 
///////////////////////////////////////////////////////////////////////////////
#define ACDK_CCAP_OKCODE(errid)                       OKCODE(MODULE_ACDK_CCAP, errid) 
#define ACDK_CCAP_ERRCODE(errid)                      ERRCODE(MODULE_ACDK_CCAP, errid) 

enum 
{
    S_ACDK_CCAP_OK                      = ACDK_CCAP_OKCODE(0), 
    E_ACDK_CCAP_API_FAIL               = ACDK_CCAP_ERRCODE(0x0001), 
};



#ifdef __cplusplus
}
#endif

#endif //end _ACDKERRCODE_H_
