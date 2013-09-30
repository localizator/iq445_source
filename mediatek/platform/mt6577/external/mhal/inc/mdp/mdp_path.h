/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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

/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2011. All rights reserved.
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
#ifndef __MDP_PATH_H__
#define __MDP_PATH_H__

#include "mdp_element.h"    //For MT6575 MDP elements

#if defined(MDP_FLAG_1_SUPPORT_JPEGCODEC)
#include "jpeg_enc_hal.h"    //For JPEG_ENC_HAL_IN in MdpPathJpgEncodeScale
#endif


/*/////////////////////////////////////////////////////////////////////////////
    MdpPath_I
  /////////////////////////////////////////////////////////////////////////////*/
class MDPELEMENT_I; //pre declaration

class MdpPath_I
{
    
protected:
    typedef int (*CALLBACKFUNC_BEFORE_START)( void* parameter);
    CALLBACKFUNC_BEFORE_START callbackfunc_before_start_;

    
public:
    MdpPath_I():
        callbackfunc_before_start_(0),
        b_is_mdp_begin_(0)
        {};
    virtual ~MdpPath_I(){};
        

public:
    void            Config_CallBackFunction_BeforeStart( CALLBACKFUNC_BEFORE_START pFunc ) {    callbackfunc_before_start_ = pFunc; }

    
    int             Start( void* pParam );
    
    unsigned long   ResourceIdMaskGet( void );

    int             WaitBusy( unsigned long rsc_to_wait_mask ); /*If rsc_to_wait_mask == NULL , wait all element*/
    int             QueueGetFreeListOrWaitBusy( unsigned long mdp_id,unsigned long *p_StartIndex, unsigned long *p_Count  );
    int             QueueGetFreeList( unsigned long mdp_id,unsigned long *p_StartIndex, unsigned long *p_Count  );
    virtual int     QueueRefill( unsigned long resource_to_refill_mask );/*virtualized for zero shutter path, it has special queue refill methodology*/

    int             DumpRegister( void* pRaram );

    int             End( void* pParam );

    
protected:
    virtual MDPELEMENT_I**  mdp_element_list() = 0;     //Descendent is responsible to return "MDP Elemment List Array"
    virtual int             mdp_element_count() = 0;    //Descendent is responsible to return count of "MDP Elemment List Array"

public:
    /*virtual int Config( void* pParam ) = 0; */ /*Due to config parameters various path to path*/
    virtual const char* name_str(){     return  "MdpPath";  }

protected:/*Overwrite when needed*/
    virtual int _StartPre( void* pParam ){  return 0;    };      //Invoke before MDP Start()
    virtual int _Start( void* pParam );                         //Standard MDP Start, overwrite only in special case
    virtual int _StartPost( void* pParam ){  return 0;    };     //Invoke after MDP Start()

    virtual int _WaitBusyPre( unsigned long rsc_to_wait_mask ){  return 0;    };   //Invoke before MDP WaitBusy()
    virtual int _WaitBusy( unsigned long rsc_to_wait_mask );                      //Standard MDP WaitBusy, overwrite only in special case
    virtual int _WaitBusyPost( unsigned long rsc_to_wait_mask ){  return 0;    };  //Invoke after MDP WaitBusy()

    virtual int _EndPre( void* pParam ){  return 0;    };        //Invoke before MDP End()
    virtual int _End( void* pParam );                           //Standard MDP End, overwrite only in special case
    virtual int _EndPost( void* pParam ){  return 0;    };       //Invoke after MDP End()

private:
    int b_is_mdp_begin_;    //flag to indicate if MdpBegin success
    
};


/*-----------------------------------------------------------------------------
    Lagecy C version
  -----------------------------------------------------------------------------*/
typedef int (*MDPCB_CUST_TRIGGER_HW)( void* p_custdata);
class MdpCustTriggerHw_t
{
public:
    MDPCB_CUST_TRIGGER_HW   cb_ConfigPre;
    MDPCB_CUST_TRIGGER_HW   cb_ConfigPost;

    MDPCB_CUST_TRIGGER_HW   cb_EnablePre;
    MDPCB_CUST_TRIGGER_HW   cb_EnablePost;
    
    MDPCB_CUST_TRIGGER_HW   cb_WaitPre;
    MDPCB_CUST_TRIGGER_HW   cb_WaitPost;

    MDPCB_CUST_TRIGGER_HW   cb_DisablePre;
    MDPCB_CUST_TRIGGER_HW   cb_DisablePost;

    unsigned long           mdp_module_after_cam;   //The MDP element which is right after CAM module.(CRZ or PRZ0)
    MDPCB_CUST_TRIGGER_HW   cb_CamConfig;
    MDPCB_CUST_TRIGGER_HW   cb_CamEnable;
    MDPCB_CUST_TRIGGER_HW   cb_CamWait;
    MDPCB_CUST_TRIGGER_HW   cb_CamDisable;

public:
    MdpCustTriggerHw_t():
        cb_ConfigPre(NULL),        cb_ConfigPost(NULL),
        cb_EnablePre(NULL),        cb_EnablePost(NULL),
        cb_WaitPre(NULL),          cb_WaitPost(NULL),
        cb_DisablePre(NULL),       cb_DisablePost(NULL),
        mdp_module_after_cam(MID_CRZ),
        cb_CamConfig(NULL),        cb_CamEnable(NULL),        cb_CamWait(NULL),        cb_CamDisable(NULL)
        {};
    
} ;


class MDPELEMENT_I;
int _MdpPathTriggerHw( MDPELEMENT_I* mdp_element_list[] , int mdp_element_count,  
                       MdpCustTriggerHw_t* p_cust_func = NULL, void* p_custdata = NULL );


int _MdpPathDumpRegister( MDPELEMENT_I* mdp_element_list[] , int mdp_element_count  );

/*-----------------------------------------------------------------------------
    MDP   EE (down scale only ) and Resize Coefficient
  -----------------------------------------------------------------------------*/
struct MdpReszCoeff
{
    unsigned char   crz_up_scale_coeff;     //Preview & Capture/0:linear interpolation 1:most blur 19:sharpest 8:recommeneded >12:undesirable
    unsigned char   crz_dn_scale_coeff;     //Preview & Capture/0:linear interpolation 1:most blur 19:sharpest 15:recommeneded 

    unsigned char   prz0_up_scale_coeff;    //Preview dsiplay & Capture QV/0:linear interpolation 1:most blur 19:sharpest 8:recommeneded >12:undesirable
    unsigned char   prz0_dn_scale_coeff;    //Preview dsiplay & Capture QV/0:linear interpolation 1:most blur 19:sharpest 15:recommeneded 
    unsigned char   prz0_ee_h_str;          //Preview dsiplay & Capture QV/down scale only/0~15
    unsigned char   prz0_ee_v_str;          //Preview dsiplay & Capture QV/down scale only/0~15

    unsigned char   prz1_up_scale_coeff;    //Preview encode/0:linear interpolation 1:most blur 19:sharpest 8:recommeneded >12:undesirable 
    unsigned char   prz1_dn_scale_coeff;    //Preview encode/0:linear interpolation 1:most blur 19:sharpest 15:recommeneded 
    unsigned char   prz1_ee_h_str;          //Preview encode/down scale only/0~15
    unsigned char   prz1_ee_v_str;          //Preview encode/down scale only/0~15
};


/*/////////////////////////////////////////////////////////////////////////////
    MdpPathCameraPreviewToMemory
  /////////////////////////////////////////////////////////////////////////////*/
struct MdpPathCameraPreviewToMemoryParameter
{
    /*pseduo source*/
    int             pseudo_src_enable;      //Use RDMA0 to take place of camera sensor as pseudo source          
        unsigned long   pseudo_src_buffer_count;
        MdpColorFormat  pseudo_src_color_format;
        MdpYuvAddr      pseudo_src_yuv_img_addr;
        

    /*Debug*/
    int             debug_preview_single_frame_enable;   /*Debug Purpose: enable to disable continous frame when preview*/

    /*HW Trigger*/
    int             b_hw_trigger_out;   /*hw trigger RDMA0*/

    /*//last preview frame for zsd */
    int             b_en_zsd_path; /*0: not enable 1,2: enable*/

    /*SRC*/
    MdpSize         src_img_size;
    MdpRect         src_img_roi;
    
    /*DST*/
    unsigned long   dst_buffer_count;  
    MdpColorFormat  dst_color_format;
    MdpSize         dst_img_size;       /*Before rotate dimension*//*Currently, "dst roi"  should be equal to "dst size"*/
    MdpRect         dst_img_roi;        /*Before rotate dimension*//*dst_img_roi is not used*/
    MdpYuvAddr      dst_yuv_img_addr;
    unsigned long   dst_rotate_angle;
    /*ZSD*/
    unsigned long   zsd_dst_buffer_count_;
    MdpYuvAddr      dst_yuv_img_addr_last_preview_;


    /*resizer coeff*/
    struct MdpReszCoeff resz_coeff;

    
};


class MdpPathCameraPreviewToMemory:public MdpPath_I
{
private:
    //MDP Drv operation
    int             m_mdp_element_count;
    MDPELEMENT_I*   m_mdp_element_list[MDP_ELEMENT_MAX_NUM];
    
    //MDP Elements 
    RDMA0       me_rdma0;   //Pseudo src used
    
    CRZ         me_crz;
    IPP         me_ipp;
    RGBROT0     me_rgbrot0; //option1: normal output ( normal 2-phase camera preview)
    VDOROT0     me_vdorot0; //option2: use when need to output planar color format (1080p video record)

    //Misc
    int     b_camera_in;
    int     b_continuous;
    
    
public:
    MdpPathCameraPreviewToMemory() :
        m_mdp_element_count(0),
        b_camera_in(1),
        b_continuous(1)
        {};
        
    virtual ~MdpPathCameraPreviewToMemory(){};

protected:/*[MdpPath_I]*/
    virtual MDPELEMENT_I**  mdp_element_list()  {   return m_mdp_element_list; }
    virtual int             mdp_element_count() {   return m_mdp_element_count; }  

public:
    virtual const char* name_str(){     return  "MdpPathCameraPreviewToMemory";  }

public:
    int     Config( struct MdpPathCameraPreviewToMemoryParameter* p_parameter ); 
    int     ConfigZoom( MdpRect crop_size );
    // last preview frame for zsd
    int     ConfigZSDPreviewFrame(unsigned long DstW, unsigned long DstH, unsigned long* index, bool bStopAfterZSD);
    int     AdpateLastPreviewBuffer(void);
    int     UnAdpateLastPreviewBuffer(void);


    
};



/*/////////////////////////////////////////////////////////////////////////////
    MdpPathCamera2PreviewToMemory
  /////////////////////////////////////////////////////////////////////////////*/
struct MdpPathCamera2PreviewToMemoryParameter
{
    /*pseduo source*/
    int             pseudo_src_enable;      //Use RDMA0 to take place of camera sensor as pseudo source          
        unsigned long   pseudo_src_buffer_count;
        MdpColorFormat  pseudo_src_color_format;
        MdpYuvAddr      pseudo_src_yuv_img_addr;

    /*Debug*/
    int             debug_preview_single_frame_enable;   /*Debug Purpose: enable to disable continous frame when preview*/

    /*HW Trigger*/
    int             b_hw_trigger_out;   /*hw trigger RDMA0*/

    /*SRC*/
    MdpSize         src_img_size;
    MdpRect         src_img_roi;
    
    /*DST*/
    unsigned long   dst_buffer_count;  
    MdpColorFormat  dst_color_format;
    MdpSize         dst_img_size;       /*Currently, "dst roi"  should be equal to "dst size"*/
    MdpRect         dst_img_roi;        /*dst_img_roi is not used*/
    MdpYuvAddr      dst_yuv_img_addr;
    unsigned long   dst_rotate_angle;

    
    /*resizer coeff*/
    struct MdpReszCoeff resz_coeff;
};


class MdpPathCamera2PreviewToMemory:public MdpPath_I
{
private:
    //MDP Drv operation
    int             m_mdp_element_count;
    MDPELEMENT_I*   m_mdp_element_list[MDP_ELEMENT_MAX_NUM];
    
    //MDP Elements 
    RDMA0       me_rdma0;   //Pseudo src used
    
    PRZ0        me_prz0;        
    VDOROT0     me_vdorot0;

    //Misc
    int     b_camera_in;
    int     b_continuous;
    
    
public:
    MdpPathCamera2PreviewToMemory() :
        m_mdp_element_count(0),
        b_camera_in(1),
        b_continuous(1)
        {};
        
    virtual ~MdpPathCamera2PreviewToMemory(){};

protected:/*[MdpPath_I]*/
    virtual MDPELEMENT_I**  mdp_element_list()  {   return m_mdp_element_list; }
    virtual int             mdp_element_count() {   return m_mdp_element_count; }  

public:
    virtual const char* name_str(){     return  "MdpPathCamera2PreviewToMemory";  }

public:
    int Config( struct MdpPathCamera2PreviewToMemoryParameter* p_parameter ); 

    
};




/*/////////////////////////////////////////////////////////////////////////////
    MdpPathDisplayFromMemory
  /////////////////////////////////////////////////////////////////////////////*/
struct MdpPathDisplayFromMemoryParameter
{
    /*HW Trigger Setting*/
    int             b_hw_trigger_in;    /*Enaable hw trigger input*/
    unsigned long   hw_trigger_src;     /*0:VDO_ROT0 1:RGB_ROT0 2:RGB_ROT1 3:VDO_ROT1*/    /*trigger source*/
    int             b_camera_in;
    int             b_continuous;
    int             b_eis;              /*enable EIS*/
        unsigned long   eis_float_x;
        unsigned long   eis_float_y;

    /*Source*/
    unsigned long   src_buffer_count;
    MdpColorFormat  src_color_format;
    MdpSize         src_img_size;
    MdpRect         src_img_roi;
    MdpYuvAddr      src_yuv_img_addr;
    

    /*For Display : NV21*/
    int             en_dst_port0;    //enable dst port0
    MdpRect         dst0_src_img_roi;
    unsigned long   dst0_buffer_count;
    MdpColorFormat  dst0_color_format;
    MdpSize         dst0_img_size;       /*Currently, "dst roi"  should be equal to "dst size"*/
    MdpRect         dst0_img_roi;        /*dst_img_roi is not used*/
    MdpYuvAddr      dst0_yuv_img_addr;
    unsigned long   dst0_rotate_angle;

    /*For Encode : YV12*/
    int             en_dst_port1;    //enable dst port1
    MdpRect         dst1_src_img_roi;
    unsigned long   dst1_buffer_count;
    MdpColorFormat  dst1_color_format;
    MdpSize         dst1_img_size;       /*Currently, "dst roi"  should be equal to "dst size"*/
    MdpRect         dst1_img_roi;        /*dst_img_roi is not used*/
    MdpYuvAddr      dst1_yuv_img_addr;
    unsigned long   dst1_rotate_angle;

    /*For FD : RGB*/
    int             en_dst_port2;    //enable dst port2
    unsigned long   dst2_buffer_count;
    MdpColorFormat  dst2_color_format;
    MdpSize         dst2_img_size;       /*Currently, "dst roi"  should be equal to "dst size"*/
    MdpRect         dst2_img_roi;        /*dst_img_roi is not used*/
    MdpYuvAddr      dst2_yuv_img_addr;
    unsigned long   dst2_rotate_angle;

    
    /*resizer coeff*/
    struct MdpReszCoeff resz_coeff;
};

void Printout_MdpPathDisplayFromMemoryParameter( MdpPathDisplayFromMemoryParameter* p_param );

class MdpPathDisplayFromMemory:public MdpPath_I
{
private:
    //MDP Drv operation
    int             m_mdp_element_count;
    MDPELEMENT_I*   m_mdp_element_list[MDP_ELEMENT_MAX_NUM];
    
    //MDP Elements 
    RDMA0       me_rdma0;
    MOUT        me_mout;

    PRZ0        me_prz0;
    PRZ1        me_prz1;
    VRZ0        me_vrz0;

    VDOROT0     me_vdo0;
    VDOROT1     me_vdo1;
    RGBROT1     me_rgb1;

    //Misc
    int     b_camera_in;
    int     b_continuous;
    
    
public:
    MdpPathDisplayFromMemory():
        m_mdp_element_count(0),
        b_camera_in(1),
        b_continuous(1)
        {};
        
    virtual ~MdpPathDisplayFromMemory(){};

protected:/*[MdpPath_I]*/
    virtual MDPELEMENT_I**  mdp_element_list()  {   return m_mdp_element_list; }
    virtual int             mdp_element_count() {   return m_mdp_element_count; }  

public:
    virtual const char* name_str(){     return  "MdpPathDisplayFromMemory";  }

public:
    int Config( struct MdpPathDisplayFromMemoryParameter* p_parameter ); 

    
};

/*-----------------------------------------------------------------------------
    Lagecy Functions
  -----------------------------------------------------------------------------*/
int MdpPathDisplayFromMemory_Func( struct MdpPathDisplayFromMemoryParameter* p_parameter );


/*/////////////////////////////////////////////////////////////////////////////
    MdpPathDisplayFromMemoryEx
  /////////////////////////////////////////////////////////////////////////////*/
struct MdpPathDisplayFromMemoryHdmiParameter
{
    /*Source*/
    MdpColorFormat  src_color_format;
    MdpSize         src_img_size;
    MdpRect         src_img_roi;
    MdpYuvAddr      src_yuv_img_addr;
    

    /*For HDMI : RGB*/
    int             en_hdmi_port;           //enable hdmi port
        MdpColorFormat  hdmi_color_format;
        MdpSize         hdmi_img_size;      //Stride       
        MdpRect         hdmi_img_roi;       //image size to be scaled
        MdpYuvAddr      hdmi_yuv_img_addr;
        unsigned long   hdmi_rotate_angle;

    /*For Display : 422 Pack*/
    int             en_disp_port;           //enable disp port
        MdpColorFormat  disp_color_format;
        MdpSize         disp_img_size;       /*Currently, "dst roi"  should be equal to "dst size"*/
        MdpRect         disp_img_roi;        /*dst_img_roi is not used*/
        MdpYuvAddr      disp_yuv_img_addr;
        unsigned long   disp_rotate_angle;
    
    /*resizer coeff*/
    struct MdpReszCoeff resz_coeff;
};

void Printout_MdpPathDisplayFromMemoryHdmiParameter( MdpPathDisplayFromMemoryHdmiParameter* p_param );

class MdpPathDisplayFromMemoryHdmi:public MdpPath_I
{
private:
    //MDP Drv operation
    int             m_mdp_element_count;
    MDPELEMENT_I*   m_mdp_element_list[MDP_ELEMENT_MAX_NUM];
    
    //MDP Elements 
    RDMA0       me_rdma0;
    MOUT        me_mout;

    PRZ0        me_prz0;
    VRZ0        me_vrz0;

    RGBROT0     me_rgb0;
    VDOROT1     me_vdo1;

    //Misc
    int     b_camera_in;
    int     b_continuous;
    
    
public:
    MdpPathDisplayFromMemoryHdmi():
        m_mdp_element_count(0),
        b_camera_in(1),
        b_continuous(1)
        {};
        
    virtual ~MdpPathDisplayFromMemoryHdmi(){};

protected:/*[MdpPath_I]*/
    virtual MDPELEMENT_I**  mdp_element_list()  {   return m_mdp_element_list; }
    virtual int             mdp_element_count() {   return m_mdp_element_count; }  

public:
    virtual const char* name_str(){     return  "MdpPathDisplayFromMemoryHdmi";  }

public:
    int Config( struct MdpPathDisplayFromMemoryHdmiParameter* p_parameter ); 

    
};

/*/////////////////////////////////////////////////////////////////////////////
    MdpPathN3d2ndPass
  /////////////////////////////////////////////////////////////////////////////*/
struct MdpPathN3d2ndPassParameter
{
    /*Source*/
    unsigned long   src_buffer_count;
    MdpColorFormat  src_color_format;
    MdpSize         src_img_size;
    MdpRect         src_img_roi;
    MdpYuvAddr      src_yuv_img_addr;
    

    /*Output port*/
    unsigned long   dst_buffer_count;
    MdpYuvAddr      dst_yuv_img_addr;    
    MdpRect         dst_src_img_roi;    //Use when resizer in path has cropping ability
    MdpSize         dst_img_size;        //image stride
    MdpRect         dst_img_roi;         //if roi=0, default value is (0,0,size.w, size.h)
    MdpColorFormat  dst_color_format;    
    int             dst_flip;       
    int             dst_rotate; 
    
    /*resizer coeff*/
    struct MdpReszCoeff resz_coeff;
};

void Printout_MdpPathN3d2ndPassParameter( MdpPathN3d2ndPassParameter* p_param );

class MdpPathN3d2ndPass:public MdpPath_I
{
private:
    //MDP Drv operation
    int             m_mdp_element_count;
    MDPELEMENT_I*   m_mdp_element_list[MDP_ELEMENT_MAX_NUM];
    
    //MDP Elements 
    RDMA0       me_rdma0;
    MOUT        me_mout;

    VRZ0        me_vrz0;    //select 1 from 2 resizer
    PRZ1        me_prz1;    //select 1 from 2 resizer

    VDOROT1     me_vdo1;
    
public:
    MdpPathN3d2ndPass():
        m_mdp_element_count(0)
        {};
        
    virtual ~MdpPathN3d2ndPass(){};

protected:/*[MdpPath_I]*/
    virtual MDPELEMENT_I**  mdp_element_list()  {   return m_mdp_element_list; }
    virtual int             mdp_element_count() {   return m_mdp_element_count; }  

public:
    virtual const char* name_str(){     return  "MdpPathN3d2ndPass";  }

public:
    int Config( struct MdpPathN3d2ndPassParameter* p_parameter ); 

    
};

/*/////////////////////////////////////////////////////////////////////////////
    MdpPathCameraPreviewZeroShutter
  /////////////////////////////////////////////////////////////////////////////*/
struct MdpPathCameraPreviewZeroShutterParameter
{
    /*pseduo source*/
    int             pseudo_src_enable;      //Use RDMA0 to take place of camera sensor as pseudo source          
        MdpColorFormat  pseudo_src_color_format;
        MdpYuvAddr      pseudo_src_yuv_img_addr;

    /*Debug*/
    int             debug_preview_single_frame_enable;   /*Debug Purpose: enable to disable continous frame when preview*/


    /*SRC*/
    MdpSize         src_img_size;
    MdpRect         src_img_roi;
    
    /*For Display : NV21 (VDO0)*/
    int             en_dst_port0;    //enable dst port0(VDO0)
    unsigned long   dst0_buffer_count;
    MdpColorFormat  dst0_color_format;
    MdpSize         dst0_img_size;       /*Currently, "dst roi"  should be equal to "dst size"*/
    MdpRect         dst0_img_roi;        /*dst_img_roi is not used*/
    MdpYuvAddr      dst0_yuv_img_addr;
    unsigned long   dst0_rotate_angle;

    
    /*For Encode : YV12 (VDO1)*/
    int             en_dst_port1;    //enable dst port1(RGB1)
    unsigned long   dst1_buffer_count;
    MdpColorFormat  dst1_color_format;
    MdpSize         dst1_img_size;       /*Currently, "dst roi"  should be equal to "dst size"*/
    MdpRect         dst1_img_roi;        /*dst_img_roi is not used*/
    MdpYuvAddr      dst1_yuv_img_addr;
    unsigned long   dst1_rotate_angle;

   
    /*For FD : RGB (RGB1)*/
    int             en_dst_port2;    //enable dst port2(VDO1)
    unsigned long   dst2_buffer_count;
    MdpColorFormat  dst2_color_format;
    MdpSize         dst2_img_size;       /*Currently, "dst roi"  should be equal to "dst size"*/
    MdpRect         dst2_img_roi;        /*dst_img_roi is not used*/
    MdpYuvAddr      dst2_yuv_img_addr;
    unsigned long   dst2_rotate_angle;

    
    /*resizer coeff*/
    struct MdpReszCoeff resz_coeff;
};


class MdpPathCameraPreviewZeroShutter:public MdpPath_I
{
private:
    //MDP Drv operation
    int             m_mdp_element_count;
    MDPELEMENT_I*   m_mdp_element_list[MDP_ELEMENT_MAX_NUM];
    
    //MDP Elements 
    RDMA0       me_rdma0;   //Pseudo src used
    
    CRZ         me_crz;
    OVL         me_ovl;
    
    IPP         me_ipp;

    PRZ0        me_prz0;
    VDOROT0     me_vdorot0;

    VRZ0        me_vrz0;
    RGBROT1     me_rgbrot1;

    MOUT        me_mout;
    PRZ1        me_prz1;
    VDOROT1     me_vdorot1;
        

    //Misc
    int     b_camera_in;
    int     b_continuous;


    /*ZSD Zoom*/
    int b_zsd_zoom_regdata_dirty_;
    RESZ_I::ReszZsdZoomParam zsd_reg_crz_;
    RESZ_I::ReszZsdZoomParam zsd_reg_prz0_;
    RESZ_I::ReszZsdZoomParam zsd_reg_prz1_;
    VRZ_I::VrzZsdZoomParam  zsd_reg_vrz0_;
    ROTDMA_I::RotdmaZsdZoomParam  zsd_reg_vdo1_;

    /*ZSD port*/
    int       b_zsd_dst_port0;
    int       b_zsd_dst_port1;
    int       b_zsd_dst_port2;


public:
    MdpPathCameraPreviewZeroShutter() :
        m_mdp_element_count(0),
        b_camera_in(1),
        b_continuous(1),
        b_zsd_zoom_regdata_dirty_(0)
        {};
        
    virtual ~MdpPathCameraPreviewZeroShutter(){};

protected:/*[MdpPath_I]*/
    virtual MDPELEMENT_I**  mdp_element_list()  {   return m_mdp_element_list; }
    virtual int             mdp_element_count() {   return m_mdp_element_count; }  

public:
    virtual const char* name_str(){     return  "MdpPathCameraPreviewZeroShutter";  }

public:
    int         Config( struct MdpPathCameraPreviewZeroShutterParameter* p_parameter ); 
    int         ConfigZoom( MdpRect crop_size , MdpRect* p_real_crop_size );
    virtual int QueueRefill( unsigned long resource_to_refill_mask );

protected:
    virtual int _EndPost( void* pParam );  
    
};


/*/////////////////////////////////////////////////////////////////////////////
    MdpPathImageTransform
  /////////////////////////////////////////////////////////////////////////////*/
struct MdpPathImageTransformParameter
{
    MdpColorFormat  src_color_format;
    MdpSize         src_img_size;
    MdpRect         src_img_roi;
    MdpYuvAddr      src_yuv_img_addr;

    MdpColorFormat  dst_color_format;
    MdpSize         dst_img_size;       /*Currently, "dst roi"  should be equal to "dst size"*/
    MdpRect         dst_img_roi;        /*dst_img_roi is not used*/
    MdpYuvAddr      dst_yuv_img_addr;
    unsigned long   dst_rotate_angle;
    unsigned long   dst_dither_en;

    /*Resizer coeff if can apply*/
    unsigned long resz_up_scale_coeff;    //0:linear interpolation 1:most blur 19:sharpest 8:recommeneded >12:undesirable
    unsigned long resz_dn_scale_coeff;    //0:linear interpolation 1:most blur 19:sharpest 15:recommeneded 
    unsigned long resz_ee_h_str;          //down scale only/0~15
    unsigned long resz_ee_v_str;          //down scale only/0~15
};



int MdpPathImageTransformYuv_1( struct MdpPathImageTransformParameter* p_parameter );
int MdpPathImageTransformYuv_2( struct MdpPathImageTransformParameter* p_parameter );
int MdpPathImageTransformYuv_3( struct MdpPathImageTransformParameter* p_parameter );
int MdpPathImageTransformRgb_1( struct MdpPathImageTransformParameter * p_parameter);
int MdpPathImageTransformRgb_3( struct MdpPathImageTransformParameter* p_parameter );
int MdpPathImageTransformRgb_4( struct MdpPathImageTransformParameter* p_parameter );


int MdpPathImageProcess( struct MdpPathImageTransformParameter* p_parameter , unsigned long u4ProcFlag);
/*/////////////////////////////////////////////////////////////////////////////
    MdpPathJpgEncodeScale
  /////////////////////////////////////////////////////////////////////////////*/
struct MdpPathJpgEncodeScaleParameter
{
    /*-----Input-----*/
    int b_sensor_input;         /*0:input from memory (rdma0) 1:input from sensor*/
        //if( b_sensor_input == 0 )
        //{
        MdpYuvAddr      src_yuv_img_addr;   //Omit if b_sensor_input = 1
        MdpColorFormat  src_color_format;   //Omit if b_sensor_input = 1
        //}

    MdpSize         src_img_size;
    MdpRect         src_img_roi;

    /*-----JPEG Output-----*/
    int b_jpg_path_disen;   /*1:DISABLE jpg encode path*/
        //if( b_jpg_path_disen == 0 )
        //{
        MdpSize         jpg_img_size;           //Omit if b_jpg_path_disen = 1
        unsigned long   jpg_yuv_color_format;   //integer : 411, 422, 444, 400, 410 etc... //Omit if b_jpg_path_disen = 1
        unsigned int    jpg_buffer_addr;        //Omit if b_jpg_path_disen = 1
        unsigned int    jpg_buffer_size;        //Omit if b_jpg_path_disen = 1
        unsigned int    jpg_quality;    //39~90 //Omit if b_jpg_path_disen = 1
        int             jpg_b_add_soi;  //1:Add EXIF 0:none //Omit if b_jpg_path_disen = 1
        //}
    
    /*-----Quick View (RGB) Output-----*/  
    int b_qv_path_en;   /*1:enable quick view path*/
        //if( b_qv_path_en == 1 )
        //{
        int qv_path_sel;    /*(Legacy.Don't care) 0:auto select quick view path 1:QV_path_1 2:QV_path_2*/
        MdpYuvAddr      qv_yuv_img_addr;    //Omit if b_qv_path_en = 0
        MdpSize         qv_img_size;        //Omit if b_qv_path_en = 0, image stride
        MdpRect         qv_img_roi;         //Omit if b_qv_path_en = 0, if roi=0, default value is (0,0,size.w, size.h)
        MdpColorFormat  qv_color_format;    //Omit if b_qv_path_en = 0
        int             qv_flip;            //Omit if b_qv_path_en = 0
        int             qv_rotate;          //Omit if b_qv_path_en = 0
        //}

    
    /*-----Full Frame (YUV) Output-----*/  
    int b_ff_path_en;   /*1:enable full frame path*/
        //if( b_ff_path_en == 1 )
        //{
        MdpYuvAddr      ff_yuv_img_addr;    //Omit if b_ff_path_en = 0
        MdpSize         ff_img_size;        //Omit if b_ff_path_en = 0, image stride
        MdpRect         ff_img_roi;         //Omit if b_ff_path_en = 0, if roi=0, default value is (0,0,size.w, size.h)
        MdpColorFormat  ff_color_format;    //Omit if b_ff_path_en = 0
        int             ff_flip;            //Omit if b_ff_path_en = 0
        int             ff_rotate;          //Omit if b_ff_path_en = 0
        //}

    
    /*resizer coeff*/
    struct MdpReszCoeff resz_coeff;
        
    
};


class MdpPathJpgEncodeScale:public MdpPath_I
{
private:
    //MDP Drv operation
    int             m_mdp_element_count;
    MDPELEMENT_I*   m_mdp_element_list[MDP_ELEMENT_MAX_NUM];
    
    //MDP Elements 
    RDMA0       me_rdma0;   /*option:input from sensor or memory*/
    CRZ         me_crz;
    IPP         me_ipp;
    JPEGDMA     me_jpegdma;

                            /*Quick View Path*/
    PRZ0        me_prz0; 
    RGBROT0     me_rgb0;
    VDOROT0     me_vdo0;
    
    VRZ0        me_vrz0;
    VDOROT1     me_vdo1;
    RGBROT1     me_rgb1;

    //JPEG Encode Object
    
#if defined(MDP_FLAG_1_SUPPORT_JPEGCODEC)
    JPEG_ENC_HAL_IN jpg_inJpgEncParam_;
#endif
    int             b_jpg_path_disen_;   /*1:DISABLE jpg encode path*/
    int             jpg_encID_;
    unsigned int    jpg_encode_size_;   //Output: encoded size of jpeg file
    

    
public:
    MdpPathJpgEncodeScale():
        m_mdp_element_count(0),
        jpg_encID_(0),
        jpg_encode_size_(0)
        {};
        
    virtual ~MdpPathJpgEncodeScale(){};

protected:/*[MdpPath_I]*/
    virtual MDPELEMENT_I**  mdp_element_list()  {   return m_mdp_element_list; }
    virtual int             mdp_element_count() {   return m_mdp_element_count; }  

public:
    virtual const char* name_str(){     return  "MdpPathJpgEncodeScale";  }


public:
    int Config( struct MdpPathJpgEncodeScaleParameter* p_parameter ); 
    unsigned int jpg_encode_size( void ){   return jpg_encode_size_;    }


protected:
    virtual int _StartPre( void* pParam );      //Invoke before MDP Start()
    //virtual int _Start( void* pParam );                         //Standard MDP Start, overwrite only in special case
    virtual int _StartPost( void* pParam );     //Invoke after MDP Start()

    virtual int _WaitBusyPre( unsigned long rsc_to_wait_mask );   //Invoke before MDP WaitBusy()
    //virtual int _WaitBusy( unsigned long rsc_to_wait_mask );                      //Standard MDP WaitBusy, overwrite only in special case
    virtual int _WaitBusyPost( unsigned long rsc_to_wait_mask );  //Invoke after MDP WaitBusy()

    virtual int _EndPre( void* pParam );        //Invoke before MDP End()
    //virtual int _End( void* pParam );                           //Standard MDP End, overwrite only in special case
    virtual int _EndPost( void* pParam );       //Invoke after MDP End()


    
};


/*/////////////////////////////////////////////////////////////////////////////
    MdpPathStnr
  /////////////////////////////////////////////////////////////////////////////*/
struct MdpPathStnrParameter
{
    int b_crz_use_line_buffer;
    int b_prz0_use_line_buffer;
    int b_prz1_use_line_buffer;
};


class MdpPathStnr:public MdpPath_I
{
private:
    //MDP Drv operation
    int             m_mdp_element_count;
    MDPELEMENT_I*   m_mdp_element_list[MDP_ELEMENT_MAX_NUM];
    
    //MDP Elements 
    CRZ         me_crz;
    PRZ0        me_prz0;
    PRZ1        me_prz1;


private: /*MdpPathStnr is a special mdp path, use customise start/end function*/
    int m_resource_lock_id; 
    int b_crz_use_line_buffer;
    int b_prz0_use_line_buffer;
    int b_prz1_use_line_buffer;
    
    
    
public:
    MdpPathStnr():
        m_mdp_element_count(0),
        m_resource_lock_id(0), 
        b_crz_use_line_buffer(1),
        b_prz0_use_line_buffer(1),
        b_prz1_use_line_buffer(1)
        {};
        
    virtual ~MdpPathStnr(){};

protected:/*[MdpPath_I]*/
    virtual MDPELEMENT_I**  mdp_element_list()  {   return m_mdp_element_list; }
    virtual int             mdp_element_count() {   return m_mdp_element_count; }  

public:
    virtual const char* name_str(){     return  "MdpPathStnr";  }

public:
    int Config( struct MdpPathStnrParameter* p_parameter ); 

public: /*[MdpPath_I]*//*MdpPathStnr is a special mdp path, use customise _start/_end function*/
    virtual int _Start( void* pParam );
    virtual int _WaitBusy( unsigned long rsc_to_wait_mask ){   return 0;   }
    virtual int _End( void* pParam );
    
};

/*/////////////////////////////////////////////////////////////////////////////
    MdpPathDummyBrz
  /////////////////////////////////////////////////////////////////////////////*/
struct MdpPathDummyBrzParameter
{
    int dummy;
};


class MdpPathDummyBrz:public MdpPath_I
{
private:
    //MDP Drv operation
    int             m_mdp_element_count;
    MDPELEMENT_I*   m_mdp_element_list[MDP_ELEMENT_MAX_NUM];
    
    //MDP Elements 
    BRZ         me_brz;

    
public:
    MdpPathDummyBrz():
        m_mdp_element_count(0)
        {};
        
    virtual ~MdpPathDummyBrz(){};

protected:/*[MdpPath_I]*/
    virtual MDPELEMENT_I**  mdp_element_list()  {   return m_mdp_element_list; }
    virtual int             mdp_element_count() {   return m_mdp_element_count; }  

public:
    virtual const char* name_str(){     return  "MdpPathDummyBrz";  }

public:
    int Config( struct MdpPathDummyBrzParameter* p_parameter ); 

protected: /*[MdpPath_I]*//*MdpPathDummyBrz is a special mdp path, use customise _start/_end function*/
    virtual int _Start( void* pParam );
    virtual int _WaitBusy( unsigned long rsc_to_wait_mask ){   return 0;   }
    virtual int _End( void* pParam );
    
};


#endif
