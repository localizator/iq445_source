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
 /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
     General MDP Pipe:
         1.Platform independent MDP Pipe interface
         2.Just a thin interface to call platform-specific MDP Pipe implementation
   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __MDP_PIPE_H__
#define __MDP_PIPE_H__

#include "mdp_datatypes.h"
#include "mdp_path.h"



#include <semaphore.h>  //For buffer sync mechanism


/*
 * Ideally, mdp_pipe is platform-independent,while mdp_path is platform-dependent.
 * We should not include "mt6575_mdp_path.h" here"
 * To achive this:
 *      1. Dynamic instance mdp_path object in .cpps => performance
 *      2. mdp_path abtract layer
 * 
 */


/*/////////////////////////////////////////////////////////////////////////////
    MdpPipeImageTransform
  /////////////////////////////////////////////////////////////////////////////*/
/*-----------------------------------------------------------------------------
    Image Transform Favor Flags
  -----------------------------------------------------------------------------*/
#define ITFF_USE_CRZ        ( 0x1 << 0 )


struct MdpPipeImageTransformParameter /*Same with MdpPathImageTransformParameter*/
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
    unsigned long   do_image_process;// 0 : disable , 1: do all process, 2: color only
    unsigned long   favor_flags;        /*Use ITFF_**** flags , this use OR principle to decide image transform path.
                                          If favor_flags = 0, means don't care
                                         */

    /*Resizer coeff if can apply*/
    unsigned long resz_up_scale_coeff;    //0:linear interpolation 1:most blur 19:sharpest 8:recommeneded >12:undesirable
    unsigned long resz_dn_scale_coeff;    //0:linear interpolation 1:most blur 19:sharpest 15:recommeneded 
    unsigned long resz_ee_h_str;          //down scale only/0~15
    unsigned long resz_ee_v_str;          //down scale only/0~15
};


/*=============================================================================
    MdpPipeImageTransform C Version
  =============================================================================*/
int MdpPipeImageTransform_Func( struct MdpPipeImageTransformParameter* p_parameter );



/*/////////////////////////////////////////////////////////////////////////////
    MdpPipeCameraPreview
  /////////////////////////////////////////////////////////////////////////////*/

struct MdpPipeCameraPreviewParameter /*Same with MdpPathCameraPreviewToMemoryParameter & 
                                                 MdpPathDisplayFromMemory              */
{
    /*MdpPathCameraPreviewToMemoryParameter*/
    int             pseudo_src_enable;       //Use RDMA0 to take place of camera sensor as pseudo source          
    MdpColorFormat  pseudo_src_color_format; /*pseduo source use the same buffer count as dst buffer*/
    MdpYuvAddr      pseudo_src_yuv_img_addr;

    int             debug_preview_single_frame_enable;  /*Debug Purpose: enable to disable continous frame when preview*/
    
    MdpSize         src_img_size;
    MdpRect         src_img_roi;

    MdpSize         src_tg2_img_size;   /*valid when en_n3d_preview_path=1*/
    MdpRect         src_tg2_img_roi;    /*valid when en_n3d_preview_path=1*/

    /*.............................................................................
        Normal & ZSD Preview
      .............................................................................*/
    int en_one_pass_preview_path;  //0:Normal preview                     1:one pass preview path for 1080p support

    int             en_sw_trigger;

    int             en_zero_shutter_path;
        /*  (2 Phase camera preview pipe)
            Output buffer from "Camera Preview Path", and also is 
            Input buffer  to "Display From Memory Path" */
        //if( en_zero_shutter_path == 0 ) 
        //{
        unsigned long   dst_buffer_count;
        MdpColorFormat  dst_color_format;
        MdpSize         dst_img_size;       /*Before rotate dimension*//*Currently, "dst roi"  should be equal to "dst size"*/
        MdpRect         dst_img_roi;        /*Before rotate dimension*//*dst_img_roi is not used*/
        MdpYuvAddr      dst_yuv_img_addr;
        MdpYuvAddr      dst_yuv_img_addr_last_preview_;
        unsigned long   dst_rotate_angle;
        unsigned long   dst_buffer_count_zsd;
        //}

    /*.............................................................................
        Native 3D Preview
      .............................................................................*/
    int     en_n3d_preview_path;    //0:normal preview path 1:native 3d preview path
        
        int    n3d_working_buff_layout; //0:LR 1:RL (L:tg1, R:tg2)
        //struct mHalWorkingBufferPort    work_tg1;   /*valid when en_n3d_preview_path=1*/
        unsigned long   working_tg1_buffer_count; 
        MdpYuvAddr      working_tg1_buffer_addr; 
        MdpColorFormat  working_tg1_color_format;
        MdpSize         working_tg1_img_size;
        MdpRect         working_tg1_img_roi; 
        unsigned int    working_tg1_rotate_angle;

        //struct mHalWorkingBufferPort    work_tg2;   /*valid when en_n3d_preview_path=1*/    
        unsigned long   working_tg2_buffer_count; 
        MdpYuvAddr      working_tg2_buffer_addr; 
        MdpColorFormat  working_tg2_color_format;
        MdpSize         working_tg2_img_size;
        MdpRect         working_tg2_img_roi; 
        unsigned int    working_tg2_rotate_angle;
    
    
    /*.............................................................................
        Output Buffer
      .............................................................................*/
    /*For Display : NV21 /In N3D Path as the only output buffer*/
    int             en_dst_port0;    //enable dst port0
        //if( en_dst_port0 )
        //{
        unsigned long   dst0_buffer_count;
        MdpColorFormat  dst0_color_format;
        MdpSize         dst0_img_size;       /*Currently, "dst roi"  should be equal to "dst size"*/
        MdpRect         dst0_img_roi;        /*dst_img_roi is not used*/
        MdpYuvAddr      dst0_yuv_img_addr;
        unsigned long   dst0_rotate_angle;
        //}

    /*For Encode : YV12*/
    int             en_dst_port1;    //enable dst port1
        //if( en_dst_port1 )
        //{
        unsigned long   dst1_buffer_count;
        MdpColorFormat  dst1_color_format;
        MdpSize         dst1_img_size;       /*Currently, "dst roi"  should be equal to "dst size"*/
        MdpRect         dst1_img_roi;        /*dst_img_roi is not used*/
        MdpYuvAddr      dst1_yuv_img_addr;
        unsigned long   dst1_rotate_angle;

    /*For FD : RGB*/
    int             en_dst_port2;    //enable dst port2
        //if( en_dst_port2 )
        //{
        unsigned long   dst2_buffer_count;
        MdpColorFormat  dst2_color_format;
        MdpSize         dst2_img_size;       /*Currently, "dst roi"  should be equal to "dst size"*/
        MdpRect         dst2_img_roi;        /*dst_img_roi is not used*/
        MdpYuvAddr      dst2_yuv_img_addr;
        unsigned long   dst2_rotate_angle;

    
    /*.............................................................................
        Misc
      .............................................................................*/
    /*resizer coeff*/
    struct MdpReszCoeff resz_coeff;

    
};


/*=============================================================================
    MdpPipeCameraPreview C++ Version
  =============================================================================*/
typedef int (*CALLBACKFUNC_EIS_INFO_GET)( unsigned long *p_eis_x, unsigned long *p_eis_y );
class RegisterLoopMemory_t;
class RegisterLoopMemoryObj_t;


class MdpPipeCameraPreview
{
private:
    /*Path Option 1:Normal Preview & ZSD Ver.2*/
    MdpPathCameraPreviewToMemory    m_MdpPathCameraPreviewToMemory;
    MdpPathDisplayFromMemory        m_MapPathDisplayFromMemory;

    /*Path Option 2:ZSD Ver.1*/
    MdpPathCameraPreviewZeroShutter m_MapPathCameraPreviewZeroShutter;

    /*Path Option 3:Native 3D*/
    MdpPathCameraPreviewToMemory    m_MdpPathN3d1stTg1;
    MdpPathCamera2PreviewToMemory   m_MdpPathN3d1stTg2;
    MdpPathN3d2ndPass               m_MdpPathN3d2nd;
    
    

private:
    int en_one_pass_preview_path_;  //0:Normal preview                     1:one pass preview path for 1080p support
    int en_zero_shutter_path_;      //0:2 stage preview path(normal)       1:zero shutter path
    int en_sw_trigger_;             //0:hw trigger preview 2nd path(normal)       1:sw trigger preview 2nd path
    int en_n3d_preview_path_;       //Native 3D preview path
    CALLBACKFUNC_EIS_INFO_GET   CB_EIS_INFO_GET_;   //Callback function for get EIS info
    unsigned long path_1_rotate_angle_;    //remember MdpPathCameraPreviewToMemory rotate angle
    unsigned long fps_show_info_counter_;
    int b_is_generic_yuv_in_one_pass_preview_path_; //Remember working bufer color format

    
private: /*Internal buffer management structure*/
    class BufferInfo
    {
    public:
        char            name_str[255];
        int             enable;
        int             buffer_count;
        MdpYuvAddr      buffer[MT6575RDMA_MAX_RINGBUFFER_CNT];/*Virtual address*/
        int             index_w;    /*write index for hw to write to*/
        int             index_r;    /*read index for sw to read back*/
        int             b_empty;    /*1:no free buffer for hw to write,i.e. no one enque*/
        stTimeStamp     time_stamp; /*time stamp*/

    public:
        int             b_show_drop_frame;
        unsigned long   drop_frame_count;
       

    private:
        sem_t   sem_hw_finish_count_;    /*semaphore for hw finish & sw dequeue, used only in disp0,1,2 */

    private: /*Support for register loop memory*/
        int                         loop_mem_index_;
        RegisterLoopMemory_t*       p_loop_mem_param[MT6575RDMA_MAX_RINGBUFFER_CNT];
        RegisterLoopMemoryObj_t*    p_loop_mem_obj[MT6575RDMA_MAX_RINGBUFFER_CNT];
        

    public:
        void SetName( const char* name ){ strcpy(name_str,name);  } 
        void Reset( int loop_mem_index );                 /*Reset data structure*/
        int  IsEmptyForHwWrite( void );     /*1: there is no free buffer for hw to write*/
        int  IsEmptyForSwRead( void ){  return ( (index_w == index_r) && !b_empty ) ? 1:0;  }     /*1: there is no available buffer for sw to read*/
        int  GetAddrByIndex( int index, MdpYuvAddr *pAddr );
        MdpYuvAddr WriteYuvAddrGet( void ){ return buffer[index_w]; }
        MdpYuvAddr ReadYuvAddrGet( void ){ return buffer[index_r]; }    //return the next buffer output by hw and need to process
        int AdvanceWritePointer( unsigned long ts_sec, unsigned long ts_us );   //Call when everytime sw trigger done   
        int AdvanceReadPointer( void );                                         //Call when EnQueue buffer
        void QueryAvailableReadBuffer( unsigned long *p_startindex, unsigned long *p_count);

        int RegisterLoopMemory( int b_mem_type_input, 
                                MdpColorFormat color_format, 
                                MdpSize img_size, 
                                MdpRect img_roi, 
                                int rotate );
        
        int UnRegisterLoopMemory( void );
    };

    
private:/*Path Option 1 sw trigger local buffer management*/
    struct      MdpPathDisplayFromMemoryParameter        display_from_memory_param_;
    MdpRect     zoom_crop_region_2nd_path_;
    /*Last Preview for ZSD*/
    MdpRect     zoom_crop_region_2nd_path_last_frame_;
    MdpSize     display_From_Memory_Src_Size_Last_Preview;
    MdpYuvAddr  disp_src_buffer_last_preview_addr;
    

    BufferInfo  disp_src_buffer;    /*loop_mem_index:0*/
    BufferInfo  disp_dst0_buffer;   /*loop_mem_index:1*/
    BufferInfo  disp_dst1_buffer;   /*loop_mem_index:2*/
    BufferInfo  disp_dst2_buffer;   /*loop_mem_index:3*/



private:/*Path Option : native 3d local buffer management*/
    struct  MdpPathN3d2ndPassParameter  n3d2ndpass_tg1_param_;
    struct  MdpPathN3d2ndPassParameter  n3d2ndpass_tg2_param_;

    BufferInfo  n3d_tg1_work_src_buffer;    /*loop_mem_index:0*/
    BufferInfo  n3d_tg2_work_src_buffer;    /*loop_mem_index:1*/
    BufferInfo  n3d_dst_buffer;             /*loop_mem_index:2*/


private:/*fps calculate*/
    class DmaFreqInfo
    {
    public:
        unsigned long   mdp_id_;
        unsigned long   last_timestamp_us_;
        unsigned long   total_time_elapse_us_;
        unsigned long   latest_time_elapse_us_;
        unsigned long   counter_;
        unsigned long   infini_counter_;
        unsigned long   fps_;
        unsigned long   avg_interval_us_;
    public:
        DmaFreqInfo():mdp_id_(0),
                      last_timestamp_us_(0),
                      total_time_elapse_us_(0),
                      latest_time_elapse_us_(0),
                      counter_(0),
                      infini_counter_(0),
                      fps_(0),
                      avg_interval_us_(0)
                      {}
        
        void SetID( unsigned long mdpid ){ mdp_id_ = mdpid;   }
        void TickReference( unsigned long mdp_resource_id_mask );
        
    };

    //Enqueue fps info
    DmaFreqInfo fps_eq_working_;
    DmaFreqInfo fps_eq_dst0_;
    DmaFreqInfo fps_eq_dst1_;
    DmaFreqInfo fps_eq_dst2_;

    //DeQueue fps info
    DmaFreqInfo fps_dq_working_;
    DmaFreqInfo fps_dq_dst0_;
    DmaFreqInfo fps_dq_dst1_;
    DmaFreqInfo fps_dq_dst2_;
    
    

public:
    MdpPipeCameraPreview():
        en_one_pass_preview_path_(0),
        en_zero_shutter_path_(0),
        en_sw_trigger_(0),
        en_n3d_preview_path_(0),
        CB_EIS_INFO_GET_(NULL),
        path_1_rotate_angle_(0),
        fps_show_info_counter_(0),
        b_is_generic_yuv_in_one_pass_preview_path_(0)
        {
            
            fps_eq_working_.SetID( MID_RGB_ROT0 );
            fps_eq_dst0_.SetID( MID_VDO_ROT0 );
            fps_eq_dst1_.SetID( MID_VDO_ROT1 );
            fps_eq_dst2_.SetID( MID_RGB_ROT1 );
            
            fps_dq_working_.SetID( MID_RGB_ROT0 );
            fps_dq_dst0_.SetID( MID_VDO_ROT0 );
            fps_dq_dst1_.SetID( MID_VDO_ROT1 );
            fps_dq_dst2_.SetID( MID_RGB_ROT1 );
        };
    virtual ~MdpPipeCameraPreview(){};
    
        
public:
    int  Config( struct MdpPipeCameraPreviewParameter* p_parameter );
    int  ConfigZoom( MdpRect crop_size, MdpRect* p_real_crop_size );
    void Config_CallBackFunction_EisInfoGet( CALLBACKFUNC_EIS_INFO_GET pFunc ) {    CB_EIS_INFO_GET_ = pFunc; }
    int Start( void* pParam );
    int WaitBusy( unsigned long rsc_to_wait_mask  );
    int QueueGetFreeListOrWaitBusy( unsigned long mdp_id,unsigned long *p_StartIndex, unsigned long *p_Count  );
    int QueueRefill( unsigned long resource_to_refill_mask );
    int GetTimeStamp( unsigned long mdp_id, stTimeStamp * p_timestamp );
    int GetBufferAddress( unsigned long mdp_id, int buffer_index, MdpYuvAddr* pAddr ); 
    int DumpRegister( void );
    int End( void* pParam );


    /*Specific for Native 3D camera*/
    int N3dManualTrigger2ndPass( MdpRect tg1_roi, MdpRect tg2_roi );

    

    /*Specific for last preview frame for zsd*/
    int ConfigLastPreviewFrameAndTriggerCurrentDisplay(MdpRect crop_size);
    void TiggerLastFrameDisplay(void);
    int ConfigZSDPreviewFrame(MdpRect crop_size);


private:
    int _ConfigNormalPathHwTrigger( struct MdpPipeCameraPreviewParameter* p_parameter );
    int _ConfigNormalPathSwTrigger( struct MdpPipeCameraPreviewParameter* p_parameter );
    int _ConfigZeroShutterPath( struct MdpPipeCameraPreviewParameter* p_parameter );
    int _ConfigN3dPath( struct MdpPipeCameraPreviewParameter* p_parameter );
    int _ConfigOnePassPath( struct MdpPipeCameraPreviewParameter* p_parameter );

private:
    /*For Preview SW Trigger path*/
    int _QueueGetFreeListOrWaitBusySwTrigger( unsigned long mdp_id,unsigned long *p_StartIndex, unsigned long *p_Count  );
    int _SwTriggerWorkingByIndex( unsigned long index  );
    int _QueueRefillSwTrigger( unsigned long resource_to_refill_mask );

    /*For N3D Path*/
    int _QueueGetFreeListOrWaitBusyN3dPath( unsigned long mdp_id,unsigned long *p_StartIndex, unsigned long *p_Count  );
    int _QueueRefillN3dPath( unsigned long resource_to_refill_mask );

private:
    void _TickEnQueueFpsInfo( unsigned long mdp_resource_mask );
    void _TickDeQueueFpsInfo( unsigned long mdp_resource_mask );
    
private:
    int _SaveLastPreviewZoomInfo(MdpRect crop_size);
    int _UpdateLastPreviewZoomInfo(void);

};

/*=============================================================================
    MdpPipeCameraPreview C Version
  =============================================================================*/
int MdpPipeCameraPreview_Func( struct MdpPipeCameraPreviewParameter* p_parameter );






/*/////////////////////////////////////////////////////////////////////////////
    MdpPipeCameraCapture
  /////////////////////////////////////////////////////////////////////////////*/

struct MdpPipeCameraCaptureParameter
{
    /*-----Input-----*/
    int camera_out_mode;    //0:BayerRaw 1:YUV 2:JPEG
    int camera_count;       //0:single camera 1:dual camera

    int             pseudo_src_enable;      //Use RDMA0 to take place of camera sensor as pseudo source          
        MdpYuvAddr      pseudo_src_yuv_img_addr;
        MdpColorFormat  pseudo_src_color_format;

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


/*=============================================================================
    MdpPipeCameraCapture C++ Version
  =============================================================================*/
class MdpPipeCameraCapture
{
private:
    MdpPathJpgEncodeScale   mpath_jpg_encode_scale_;

public:
    MdpPipeCameraCapture(){};
    virtual ~MdpPipeCameraCapture(){};
    
public:
    int Config( struct MdpPipeCameraCaptureParameter* p_parameter );
    int Start( void* pParam );
    int WaitBusy( unsigned long rsc_to_wait_mask  );
    int DumpRegister( void );
    int End( void* pParam );

public:
    unsigned int jpg_encode_size( void )    {   return mpath_jpg_encode_scale_.jpg_encode_size(); }
    
};








//Open device driver node
//int MdpPipe_Init( void );

//Close device driver node , and any allocated resource
//int MdpPipe_Release( void );




#endif
