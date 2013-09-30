/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#ifndef _MDP_HAL_H_
#define _MDP_HAL_H_

#include "mdp_datatypes.h"  //For MdpColorFormat
#include "mdp_pipe.h"       //For struct MdpPipeCameraCaptureParameter

/*******************************************************************************
*
********************************************************************************/
typedef unsigned int MUINT32;
typedef int MINT32;

/*******************************************************************************
*
********************************************************************************/
#define MDP_MODE_PRV_YUV    0
#define MDP_MODE_CAP_JPG    1

/*******************************************************************************
*
********************************************************************************/

#if 0 /*Old Style halIDPParam_s*/

typedef struct halIDPParam_s {
    MUINT32 u4SrcW;
    MUINT32 u4SrcH;
    MUINT32 u4MTKYuvW;
    MUINT32 u4MTKYuvH;
    MUINT32 u4YuvW;
    MUINT32 u4YuvH;
    MUINT32 u4RgbW;             // RGB 888/565
    MUINT32 u4RgbH;
    MUINT32 u4MTKYuvVirAddr;    // For MTKYUV
    MUINT32 u4MTKYuvBufCnt;
    MUINT32 u4YuvVirAddr;       // For YUV 420
    MUINT32 u4YuvBufCnt;
    MUINT32 u4RgbVirAddr;
    MUINT32 u4RgbBufCnt;
    MUINT32 u4Mode;
    MUINT32 u4Rotate;           // 0, 90, 180, 270
    MUINT32 u4UpCoef;           // Up sample coeff
    MUINT32 u4DownCoef;         // Down sample coeff
    MUINT32 u4ZoomRatio;
    MUINT32 u4Flip;             // 0: disable, 1: enable
} halIDPParam_t;

#else /*New Style halIDPParam_s*/

struct mHalWorkingBufferPort
{
    unsigned long   working_buffer_count; 
    unsigned long   working_buffer_addr; 
    MdpColorFormat  working_color_format;
    MdpSize         working_img_size;
    MdpRect         working_img_roi; 
    unsigned int    working_rotate;
    unsigned int    working_flip;
};


struct mHalOutPort
{

    MdpColorFormat          color_format; 
    unsigned int            size_w;
    unsigned int            size_h;
    unsigned int            buffer_addr;
    unsigned int            buffer_count;
    unsigned int            rotate;
    unsigned int            flip;
    unsigned int            up_coef;
    unsigned int            down_coef;
    unsigned int            zoom_ratio;

} ;


typedef struct halIDPParam_s 
{
    
    int mode; //0:MDP_MODE_PRV_YUV:Preview Mode 1:MDP_MODE_CAP_JPG:Capture Mode

    /*-----------------------------------------------------------------------------
        mode = 0
            Camera Preview Parameters
      -----------------------------------------------------------------------------*/

    /*These extra three parameters is designed for pueudo source test only*/
    int             test_pseudo_src_enable;
        MdpColorFormat  test_pseudo_src_color_format;   /*[valid when test_pseudo_src_enable=1]*/
        unsigned int    test_pseudo_src_yuv_img_addr;   /*[valid when test_pseudo_src_enable=1]*/
        
    int             debug_preview_single_frame_enable;   /*Debug Purpose: enable to disable continous frame when preview*/
    /* End of test parameter */

    
    
    unsigned int    src_size_w; //Input: sensor output after crop & filter
    unsigned int    src_size_h;

    #if defined (MDP_FLAG_USE_RDMA_TO_CROP)
    MdpRect         src_roi; 
    #endif
   

    unsigned int    src_tg2_size_w; /*valid when en_n3d_preview_path=1*/
    unsigned int    src_tg2_size_h; /*valid when en_n3d_preview_path=1*/

    #if defined (MDP_FLAG_USE_RDMA_TO_CROP)
    MdpRect         src_tg2_roi;    /*valid when en_n3d_preview_path=1*/
    #endif

    /*.............................................................................
        Normal & ZSD Preview & 1080p Preview
      .............................................................................*/
    int     en_one_pass_preview_path;   //0:normal preview                      1:one pass preview path for 1080p support
    int    en_zero_shutter_path;   //0:2 stage preview path(normal)       1:zero shutter path
        int             en_sw_trigger;              /*[valid when en_zero_shutter_path=0]*/  //0:2 stage preview path hw trigger    1:2 stage preview path sw trigger
        unsigned long   working_buffer_count;       /*[valid when en_zero_shutter_path=0]*/  
        unsigned long   working_buffer_addr;        /*[valid when en_zero_shutter_path=0]*/
            MdpColorFormat  working_color_format;   /*[valid when en_zero_shutter_path=0 && en_sw_trigger=1]*/
            MdpSize         working_img_size;       /*[valid when en_zero_shutter_path=0 && en_sw_trigger=1]*//*After rotate dimension*/
            MdpRect         working_img_roi;        /*[valid when en_zero_shutter_path=0 && en_sw_trigger=1]*//*This is the EIS croping window/After rotate dimension*/
            unsigned int    working_rotate;         /*[valid when en_zero_shutter_path=0 && en_sw_trigger=1]*/
            unsigned int    working_flip;           /*[valid when en_zero_shutter_path=0 && en_sw_trigger=1]*/
            CALLBACKFUNC_EIS_INFO_GET   CB_EIS_INFO_GET_FUNC;/*[valid when en_zero_shutter_path=0 && en_sw_trigger=1]*/ //Callback function for EIS coordinate
        unsigned long   working_buffer_addr_last_preview_;        /*[valid when en_zero_shutter_path=0]*/

        unsigned long   working_buffer_count_zsd;   /*[valid when en_zero_shutter_path=2]*/

    
    /*.............................................................................
        Native 3D Preview
      .............................................................................*/
    int     en_n3d_preview_path;        //0:normal preview path 1:native 3d preview path
        int    n3d_working_buff_layout; //0:LR 1:RL (L:tg1, R:tg2)
        struct mHalWorkingBufferPort    work_tg1;   /*valid when en_n3d_preview_path=1*/
        struct mHalWorkingBufferPort    work_tg2;   /*valid when en_n3d_preview_path=1*/    


    /*.............................................................................
        Output Buffer
      .............................................................................*/
    int    en_dst_port0;    //enable dst port0
    struct mHalOutPort dst_port0;   /*[valid when en_dst_port0=1]*/ //Output for display (default:NV21)
    int    en_dst_port1;    //enable dst port1
    struct mHalOutPort dst_port1;   /*[valid when en_dst_port1=1]*/ //Output for encode (default:NV21)
    int    en_dst_port2;    //enable dst port2
    struct mHalOutPort dst_port2;   /*[valid when en_dst_port2=1]*/ //Output for FD (default:RGB)



    /*.............................................................................
        Misc
      .............................................................................*/
    /*resizer coeff*/
    struct MdpReszCoeff prv_resz_coeff; //resz coeff for preview

    /*-----------------------------------------------------------------------------
        mode = 1
            Camera Capture Parameters
      -----------------------------------------------------------------------------*/
    struct MdpPipeCameraCaptureParameter Capture; /*[valid when mode=1]*/

    
} halIDPParam_t;


#endif

//
typedef struct rect_s {
    MUINT32 x;
    MUINT32 y;
    MUINT32 w;
    MUINT32 h;
} rect_t;
//
typedef enum halMdpCmd_s {
    CMD_SET_ZOOM_RATIO          = 0x1001,
    CMD_GET_JPEG_FILE_SIZE,
    CMD_SET_ZSD_LAST_PREVIEW_FRAME, // last preview frame for zsd
    CMD_TRIGGER_LAST_FRAME_DISPLAY,
    CMD_SET_ZSD_PREVIEW_FRAME,
    CMD_GET_BUFFER_ADDRESS,
    CMD_N3D_MANUAL_TRIGGER_2ND_PASS,
    CMD_HAL_MDP_MAX             = 0xFFFF
} halMdpCmd_e;
//
#define MDPHAL_MAX_BUFFER_COUNT 16
typedef struct halMdpTimeStampInfo_s {
    MUINT32 timeStampS[MDPHAL_MAX_BUFFER_COUNT];     // Time stamp
    MUINT32 timeStampUs[MDPHAL_MAX_BUFFER_COUNT];
} halMdpTimeStampInfo_t;
//
typedef struct halIDPBufInfo_s {
    MUINT32 hwIndex;            // Current hardware index
    MUINT32 fillCnt;            // Current hardware fill count
    halMdpTimeStampInfo_t timeStamp;
} halMdpBufInfo_t;

typedef enum halMdpOutputPort{
    DISP_PORT = 0x1,
    VDOENC_PORT,
    FD_PORT,
    QUICKVIEW_PORT,
    JPEG_PORT,
    THUMBNAIL_PORT,
    PREVIEW_WORKING_BUFFER,
    ZSD_PORT,
    /*N3D Buffer Management*/
    N3D_PREVIEW_WORKING_BUFFER_TG1, //For native 3d camera support
    N3D_PREVIEW_WORKING_BUFFER_TG2, //For native 3d camera support
    N3D_OUTPUT_PORT                 //For native 3d, output L-R side-by-side image
    
    
} halMdpOutputPort_e;

/*******************************************************************************
*
********************************************************************************/
class MdpHal {
public:
    //
    static MdpHal* createInstance();
    virtual void destroyInstance() = 0;

protected:
    virtual ~MdpHal() {};

public:
    virtual MINT32 init() = 0;
    //
    virtual MINT32 uninit() = 0;
    //
    virtual MINT32 start() = 0;
    //
    virtual MINT32 stop() = 0;
    //
    virtual MINT32 setPrv(halIDPParam_t *phalIDPParam) = 0;
    //
    virtual MINT32 setCapJpg(halIDPParam_t *phalIDPParam) = 0;
    //
    virtual MINT32 setConf(halIDPParam_t *phalIDPParam) = 0;
    //
    virtual MINT32 getConf(halIDPParam_t *phalIDPParam) = 0;
    //
    virtual MINT32 calCropRect(rect_t rSrc, rect_t rDst, rect_t *prCrop, MUINT32 zoomRatio) = 0;
    //
    virtual MINT32 waitDone(MINT32 mode) = 0;
    //
    virtual MINT32 dumpReg() = 0;
    //
    virtual MINT32 sendCommand(int cmd, int parg1 = NULL, int parg2 = NULL, int parg3 = NULL) = 0;
    //
    virtual MINT32 dequeueBuff(halMdpOutputPort_e e_Port, halMdpBufInfo_t * a_pstBuffInfo) = 0;
    //
    virtual MINT32 enqueueBuff(halMdpOutputPort_e e_Port) = 0;
};

#endif // _ISP_DRV_H_

