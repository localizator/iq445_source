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

#ifndef __MDP_DRV_H__
#define __MDP_DRV_H__

#include "mdp_datatypes.h"

#if defined(MDP_FLAG_PROFILING)  
#include <sys/time.h> //gettimeofday      
#endif


/*/////////////////////////////////////////////////////////////////////////////
    Definition
  /////////////////////////////////////////////////////////////////////////////*/
typedef enum
{
    MLM_CLIENT_SFTEX,        //Surface flinger Texture
    MLM_CLIENT_MTKOVERLAY,  
    MLM_CLIENT_PV2ND,        //Preview 2nd path
    MLM_CLIENT_ELEMENT,      //MDP Element
    MLM_CLIENT_PVCPY,        //Camera preview path extra-copy
    MLM_CLIENT_GRALLOC,      //Graphic Buffer Allocator

    MLM_CLIENT_MAX = 19,    //MAX (for test)
    
} MVALOOPMEM_CLIENT;



/*/////////////////////////////////////////////////////////////////////////////
    Data Structure
  /////////////////////////////////////////////////////////////////////////////*/
typedef struct 
{
    unsigned long   byte_per_pixel; //When generic yuv,this value only means Y plane byte per pixel
    int             b_is_generic_yuv;
    int             b_is_uv_swap;
    int             b_is_uv_interleave;

    //Stride Align: 0:None 1:2 align 2:4 align 3:8 align 4:16 align
    unsigned long   y_stride_align;
    unsigned long   uv_stride_align;
    

    /*Register setting*/
    unsigned long   in_format:4;    //For RDMA used
    unsigned long   swap:2;         //For RDMA used: 01:Byte Swap 10:RGB Swap 3:Byte&RGB Swap
    
    unsigned long   out_format:8;   //For ROTDMA used

    //Sampling Period. b00:0 b01:1 b10:2 b11:4
    unsigned long   yh:2;   
    unsigned long   yv:2;
    unsigned long   uvh:2;
    unsigned long   uvv:2;

} MdpDrvColorInfo_t;




/*/////////////////////////////////////////////////////////////////////////////
    MDP Platform Driver Functions
        - General
  /////////////////////////////////////////////////////////////////////////////*/

int MdpDrvInit( void );      //Open mdp device driver node
int MdpDrvRelease( void );   //Close mdp device driver node
int MdpDrvFd( void );

int MdpDrvIsPmem( unsigned long addr, unsigned long size, unsigned long *p_phy_addr ); //Return 1 if addr is pmem
int MdpDrvLockResource( unsigned long mdp_resource_mask, int b_time_shared, unsigned long time_out_ms , const char* path_name_str );
int MdpDrvUnLockResource( unsigned long mdp_resource_mask );
int MdpDrv_DumpRegister( void );

int MdpDrvWaitIntDone( unsigned long mdp_rsc_id_mask , unsigned long time_out_ms );
int MdpDrvClearIntDone( unsigned long mdp_rsc_id_mask );


int MdpDrvColorFormatInfoGet( MdpColorFormat color, MdpDrvColorInfo_t* p_ci );




/*/////////////////////////////////////////////////////////////////////////////
    MDP Platform Driver Functions
        - M4U Support functions
  /////////////////////////////////////////////////////////////////////////////*/

int MdpDrv_AdaptMdpYuvAddrArray(   unsigned long mdp_id, 
                                    MdpYuvAddr* in_addr_list, int addr_count,   //Original user input address  
                                    MdpYuvAddr* out_adapt_addr_list,            //Output adapted address
                                    unsigned long *p_adapt_m4u_flag_bit,        //If address has been adapted with m4u mva, corresponding bit will set to 1
                                    unsigned long *p_alloc_mva_flag_bit_ );     //Corresponding bit will set to 1 if the mva is new allocated
                                    


int MdpDrv_UnAdaptMdpYuvAddrArray( unsigned long mdp_id, 
                                    MdpYuvAddr* in_addr_list, int addr_count,   //Original user input address  
                                    MdpYuvAddr* in_adapt_addr_list,             //Adapted address
                                    unsigned long adapt_m4u_flag_bit,           //Corresponding bit will set to 1 if address had been adapted with m4u mva
                                    unsigned long alloc_mva_flag_bit_ );        //Corresponding bit will set to 1 if the mva is new allocated

int MdpDrv_AdaptM4uForZSD(unsigned long va_addr, unsigned long va_size,
                    unsigned long* p_mva_addr    );

int MdpDrv_UnAdaptM4uForZSD(unsigned long va_addr, unsigned long va_size, unsigned long mva_addr );


int MdpDrv_CalImgBufferSizeAndFillIn( 
                MdpColorFormat  color_format, 
                MdpYuvAddr*     p_yuv_addr,
                MdpSize         img_size,
                MdpRect         img_roi,
                int             rotate );

int MdpDrv_CalImgBufferArraySizeAndFillIn( 
                MdpColorFormat  color_format, 
                MdpYuvAddr*     yuv_addr_array,
                int             count,
                MdpSize         img_size,
                MdpRect         img_roi,
                int             rotate,
                unsigned long*  p_total_size );


/*/////////////////////////////////////////////////////////////////////////////
    MDP Platform Driver Functions
        - Cache Sync Function (Implement in M4U module)
  /////////////////////////////////////////////////////////////////////////////*/
typedef enum
{
    MDPDRV_CACHE_SYNC_CLEAN_BEFORE_HW_READ_MEM,
    MDPDRV_CACHE_SYNC_INVALID_BEFORE_HW_WRITE_MEM
} MDPDRV_CACHE_SYNC_OP;


int MdpDrv_CacheSync( unsigned long mdp_id, MDPDRV_CACHE_SYNC_OP op, unsigned long start_addr, unsigned long size);
int MdpDrv_CacheFlushAll( void );



/*/////////////////////////////////////////////////////////////////////////////
    MDP Platform Driver Functions
        - Camera ZOOM Support function
  /////////////////////////////////////////////////////////////////////////////*/
typedef struct {
    unsigned long mdp_id;
    unsigned long linked_mdp_id_set;   /*associated mdps that need to be update in one single cycle*/
    unsigned long reg_CFG;
    unsigned long reg_SRCSZ;
    unsigned long reg_CROPLR;
    unsigned long reg_CROPTB;
    unsigned long reg_HRATIO;
    unsigned long reg_VRATIO;
} MdpDrvConfigZoom_Param;


int MdpDrv_ConfigZoom( MdpDrvConfigZoom_Param* pParam );

/*/////////////////////////////////////////////////////////////////////////////
    MDP Platform Driver Functions
        - Camera ZSD ZOOM Support function
  /////////////////////////////////////////////////////////////////////////////*/
typedef struct {
//CRZ
    unsigned long u4CRZLBMAX;//+0x0 CRZ_CFG
    unsigned long u4CRZSrcSZ;//+0x10 CRZ_SRCSZ
    unsigned long u4CRZTarSZ;//+0x14 CRZ_TARSZ
    unsigned long u4CRZHRatio;//+0x18 CRZ_HRATIO
    unsigned long u4CRZVRatio;//+0x1C CRZ_VRATIO
    unsigned long u4CRZCropLR;//+0xF4 CRZ_CropLR
    unsigned long u4CRZCropTB;//+0xF8 CRZ_CropTB
//PRZ0
    unsigned long u4PRZ0LBMAX;//+0x0 PRZ0_CFG
    unsigned long u4PRZ0SrcSZ;//+0x10 PRZ0_SRCSZ
    unsigned long u4PRZ0TarSZ;//+0x14 PRZ0_TARSZ
    unsigned long u4PRZ0HRatio;//+0x18 PRZ0_HRATIO
    unsigned long u4PRZ0VRatio;//+0x1C PRZ0_VRATIO
//VRZ
    unsigned long u4VRZSrcSZ;//+0x10 VRZ_SRCSZ
    unsigned long u4VRZTarSZ;//+0x14 VRZ_TARSZ
    unsigned long u4VRZHRatio;//+0x18 VRZ_HRATIO
    unsigned long u4VRZVRatio;//+0x1C VRZ_VRATIO
    unsigned long u4VRZHRes;//+0x20 VRZ_HRES
    unsigned long u4VRZVRes;//+0x24 VRZ_VRES
//VDOROT1 descriptor
    unsigned long u4VDO1Seg4;//SrcW+SrcH
    unsigned long u4VDO1Seg5;//ClipW+ClipH
    unsigned long u4VDO1Seg6;//ClipX+ClipY
    unsigned long u4VDO1Seg7;//DstW in Bytes
} MdpDrvConfigZSDZoom_Param;

int MdpDrv_ConfigZSDZoom( MdpDrvConfigZSDZoom_Param* pParam );

/*/////////////////////////////////////////////////////////////////////////////
    MDP Platform Driver Functions
        - Camera ZSD last preview frame for ZSD Support function
  /////////////////////////////////////////////////////////////////////////////*/
typedef struct {
    bool          bStopAfterZSDDone;
//CRZ
    unsigned long u4CRZLBMAX;//+0x0 CRZ_CFG
    unsigned long u4CRZSrcSZ;//+0x10 CRZ_SRCSZ
    unsigned long u4CRZTarSZ;//+0x14 CRZ_TARSZ
    unsigned long u4CRZHRatio;//+0x18 CRZ_HRATIO
    unsigned long u4CRZVRatio;//+0x1C CRZ_VRATIO
    unsigned long u4CRZCropLR;//+0xF4 CRZ_CropLR
    unsigned long u4CRZCropTB;//+0xF8 CRZ_CropTB
//RGBROT0 descriptor
    unsigned long u4RGB0Seg1[16];//y dst addr
    unsigned long u4RGB0Seg4;//SrcW+SrcH
    unsigned long u4RGB0Seg5;//ClipW+ClipH
    unsigned long u4RGB0Seg6;//ClipX+ClipY
    unsigned long u4RGB0Seg7;//DstW in Bytes
    unsigned long u4RGB0Seg8;//DstW in Bytes
    unsigned long u4RGB0Seg9;//DstW in Bytes
    unsigned long u4DescUpdateZone; // 0: unsafe 1:safe
    unsigned long u4DescRemain;
    unsigned long u4DescCnt;
    unsigned long u4DescHwWriteIndex;
    unsigned long u4DescHwReadIndex;
} MdpDrvConfigZSDPreview_Param;
int MdpDrv_ConfigZSDPreviewFrame( MdpDrvConfigZSDPreview_Param* pParam);



/*/////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////*/
int             MdpDrv_GetTimeStamp( stTimeStamp * p_timestamp );
unsigned long   MdpDrv_RDMA_EIS_CON_Value( unsigned long eis_float_x, unsigned long eis_float_y );



/*/////////////////////////////////////////////////////////////////////////////
    MDP Kernel API
  /////////////////////////////////////////////////////////////////////////////*/
int             MdpDrv_Mdpk_BitbltWaitRequest( MDPIOCTL_MdpkBitbltConfig* pConfig );
int             MdpDrv_Mdpk_BitbltInformDone( MDPIOCTL_MdpkBitbltInformDone* pDone );
unsigned long   MdpDrv_Mdpk_MmapKernelMemory( unsigned long kernel_vmalloc_addr, unsigned long size );
int             MdpDrv_Mdpk_MunMapKernelMemory( unsigned long user_address, unsigned long size );

/*/////////////////////////////////////////////////////////////////////////////
    MDP Utility
  /////////////////////////////////////////////////////////////////////////////*/
#define MDPDRV_WATCH_MAX_LEVEL  10
#define MDPDRV_WATCH_LEVEL_IT     0
#define MDPDRV_WATCH_LEVEL_PIPE   1  
#define MDPDRV_WATCH_LEVEL_PATH   2
#define MDPDRV_WATCH_LEVEL_ELMNT  3
#define MDPDRV_WATCH_LEVEL_DRV    4




void MdpDrv_WatchStart( int level );
void MdpDrv_WatchStop(  int level,
                        const char* title_str,
                        unsigned long* p_total_time, 
                        unsigned long* p_frame_count,
                        unsigned long* p_avg_elapse_time,
                        unsigned long  reset_frame_count );

void MdpDrv_DumpCallStack( const char* prefix_str );


void MdpDrv_MvaLoopMemTrack( int client_id, MLMT_ACTION action, unsigned long size );
void MdpDrv_CpuClockGet( unsigned long *p_sec, unsigned long *p_usec );
unsigned long MdpDrv_GetTaskStruct( void );




/*-----------------------------------------------------------------------------
    C++ version mdp drv watch
  -----------------------------------------------------------------------------*/
#if defined(MDP_FLAG_PROFILING)  
class MdpDrv_Watch
{
private:
    struct timeval  tv0_;
    struct timezone tz0_;
    struct timeval  tv1_;
    struct timezone tz1_;

public:
    void Start( int level );
    void Stop(  int level, 
                const char* title_str,
                unsigned long* p_total_time, 
                unsigned long* p_frame_count,
                unsigned long* p_avg_elapse_time,
                unsigned long  reset_frame_count );
    
    
};

#endif




#if defined(MDP_FLAG_PROFILING)
//#define MDPDRV_WATCHSTART   MdpDrv_WatchStart
//#define MDPDRV_WATCHSTOP    MdpDrv_WatchStop
#define MDPDRV_WATCHSTART   _MdpDrvWatch.Start
#define MDPDRV_WATCHSTOP    _MdpDrvWatch.Stop
#else
#define MDPDRV_WATCHSTART(...)
#define MDPDRV_WATCHSTOP(...)
#endif

unsigned long MdpDrv_GetTimeUs( void );


#endif /*__MDP_DRV_H__*/
