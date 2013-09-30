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

#ifndef __MDP_DATATYPES_H__
#define __MDP_DATATYPES_H__


//Kernel Includes
#include "mt_mdp.h" //For MDP Element ID,and Kernel Flag


// add by mingchen
//#define MDP_DRIVER_DVT_SUPPORT
/*-----------------------------------------------------------------------------
    MDP Compile Time Option
    ( MDP_FLAG_# ==> #:means flag level)
  -----------------------------------------------------------------------------*/
#define MDP_FLAG_0_PLATFORM_LINUX

#if defined(MDP_KERNEL_FLAG_1_SUPPORT_INT)
#define MDP_FLAG_1_SUPPORT_INT      //ON: MDP use Interrupt. OFF: MDP use polling
#endif


#if defined(MTK_M4U_SUPPORT)
#define MDP_FLAG_1_SUPPORT_M4U      //ON: Use M4U. OFF: Do not use M4U
#endif
#define MDP_FLAG_2_M4U_SUPPORT_QUERY      //M4U support query mechinism
#define MDP_FLAG_2_M4U_SUPPORT_PREALLOC     //M4U support prealloc

//#if defined(MT6577)
#define MDP_FLAG_2_CACHE_USE_FLUSH_ALL  //Use M4U flush all api instead of flush range api
//#endif



#define MDP_FLAG_1_SUPPORT_SYSRAM       //ON: Use sysram driver. OFF: Use temp sysram allocator

#define MDP_FLAG_1_SUPPORT_DESCRIPTOR   //ON: Use descriptor mode driver. OFF: Use register mode

#define MDP_FLAG_1_SUPPORT_JPEGCODEC      //For Porting

#define MDP_FLAG_1_SUPPORT_MHALJPEG       //For Porting

//JBPORT #define MDP_FLAG_1_SUPPORT_PQ_ISP          //For Porting


#define MDP_FLAG_USE_RDMA_TO_CROP       //ON: Use rdma0 to do zoom when preview. OFF: Use CRZ to crop and upscale

//#define MDP_FLAG_USE_XLOG             //For Porting //ON: Use XLOG.  OFF: Use Android Log

#define MDP_FLAG_PROFILING



/*-----------------------------------------------------------------------------
    MDP ERROR CODE
  -----------------------------------------------------------------------------*/
  
#define MDP_ERROR_CODE_OK                      (0)
#define MDP_ERROR_CODE_FAIL                    (-1)

#define MDP_ERROR_CODE_FAIL_00                 (-1 & ~(0x1<<0))  /*-2 , (0xFFFFFFFE)*/          
#define MDP_ERROR_CODE_FAIL_01                 (-1 & ~(0x1<<1))  /*-3 , (0xFFFFFFFD)*/          
#define MDP_ERROR_CODE_FAIL_02                 (-1 & ~(0x1<<2))  /*-5 , (0xFFFFFFFB)*/          
#define MDP_ERROR_CODE_FAIL_03                 (-1 & ~(0x1<<3))  /*-9 , (0xFFFFFFF7)*/          
#define MDP_ERROR_CODE_FAIL_04                 (-1 & ~(0x1<<4))  /*-17 , (0xFFFFFFEF)*/         
#define MDP_ERROR_CODE_LOCK_RESOURCE_FAIL      (-1 & ~(0x1<<5))  /*-33 , (0xFFFFFFDF)*/         
#define MDP_ERROR_CODE_FAIL_06                 (-1 & ~(0x1<<6))  /*-65 , (0xFFFFFFBF)*/         
#define MDP_ERROR_CODE_FAIL_07                 (-1 & ~(0x1<<7))  /*-129 , (0xFFFFFF7F)*/        
#define MDP_ERROR_CODE_FAIL_08                 (-1 & ~(0x1<<8))  /*-257 , (0xFFFFFEFF)*/        
#define MDP_ERROR_CODE_FAIL_09                 (-1 & ~(0x1<<9))  /*-513 , (0xFFFFFDFF)*/        
#define MDP_ERROR_CODE_FAIL_10                 (-1 & ~(0x1<<10))  /*-1025 , (0xFFFFFBFF)*/      
#define MDP_ERROR_CODE_FAIL_11                 (-1 & ~(0x1<<11))  /*-2049 , (0xFFFFF7FF)*/      
#define MDP_ERROR_CODE_FAIL_12                 (-1 & ~(0x1<<12))  /*-4097 , (0xFFFFEFFF)*/      
#define MDP_ERROR_CODE_FAIL_13                 (-1 & ~(0x1<<13))  /*-8193 , (0xFFFFDFFF)*/      
#define MDP_ERROR_CODE_FAIL_14                 (-1 & ~(0x1<<14))  /*-16385 , (0xFFFFBFFF)*/     
#define MDP_ERROR_CODE_FAIL_15                 (-1 & ~(0x1<<15))  /*-32769 , (0xFFFF7FFF)*/     
#define MDP_ERROR_CODE_FAIL_16                 (-1 & ~(0x1<<16))  /*-65537 , (0xFFFEFFFF)*/     
#define MDP_ERROR_CODE_FAIL_17                 (-1 & ~(0x1<<17))  /*-131073 , (0xFFFDFFFF)*/    
#define MDP_ERROR_CODE_FAIL_18                 (-1 & ~(0x1<<18))  /*-262145 , (0xFFFBFFFF)*/    
#define MDP_ERROR_CODE_FAIL_19                 (-1 & ~(0x1<<19))  /*-524289 , (0xFFF7FFFF)*/    
#define MDP_ERROR_CODE_FAIL_20                 (-1 & ~(0x1<<20))  /*-1048577 , (0xFFEFFFFF)*/   
#define MDP_ERROR_CODE_FAIL_21                 (-1 & ~(0x1<<21))  /*-2097153 , (0xFFDFFFFF)*/   
#define MDP_ERROR_CODE_FAIL_22                 (-1 & ~(0x1<<22))  /*-4194305 , (0xFFBFFFFF)*/   
#define MDP_ERROR_CODE_FAIL_23                 (-1 & ~(0x1<<23))  /*-8388609 , (0xFF7FFFFF)*/   
#define MDP_ERROR_CODE_FAIL_24                 (-1 & ~(0x1<<24))  /*-16777217 , (0xFEFFFFFF)*/  
#define MDP_ERROR_CODE_FAIL_25                 (-1 & ~(0x1<<25))  /*-33554433 , (0xFDFFFFFF)*/  
#define MDP_ERROR_CODE_FAIL_26                 (-1 & ~(0x1<<26))  /*-67108865 , (0xFBFFFFFF)*/  
#define MDP_ERROR_CODE_FAIL_27                 (-1 & ~(0x1<<27))  /*-134217729 , (0xF7FFFFFF)*/ 
#define MDP_ERROR_CODE_FAIL_28                 (-1 & ~(0x1<<28))  /*-268435457 , (0xEFFFFFFF)*/ 
#define MDP_ERROR_CODE_FAIL_29                 (-1 & ~(0x1<<29))  /*-536870913 , (0xDFFFFFFF)*/ 
#define MDP_ERROR_CODE_FAIL_30                 (-1 & ~(0x1<<30))  /*-1073741825 , (0xBFFFFFFF)*/


#define MDP_IS_ERROR_CODE( _retval_ , _errorcode_ ) \
            (   _retval_ >= 0    ?    0    :    ( (( _retval_|_errorcode_)==_errorcode_) ? 1 : 0 )     )



/*-----------------------------------------------------------------------------
    DATA STRUCTURE
  -----------------------------------------------------------------------------*/


class MdpSize
{
public:
    unsigned long w;
    unsigned long h;

public:
    MdpSize():
        w(0),h(0)
        {};

   
    MdpSize(unsigned long _w, unsigned long _h )
        {
            w = _w; h = _h;
        };

    
    MdpSize& operator=( const MdpSize& rhs )
    {
        w = rhs.w; h = rhs.h;
        return *this;
    }

    bool operator==(const MdpSize& rhs) const
    {
        return (  ( w == rhs.w ) && ( h == rhs.h ) );
    }

    bool operator!=(const MdpSize& rhs) const
    {
        return !( *this == rhs );
    }

};

class MdpPoint
{
public:
    long    x;
    long    y;

public:
    MdpPoint():
        x(0),y(0)
        {};

   MdpPoint(unsigned long _x, unsigned long _y )
       {
           x = _x; y = _y;
       };
};

class MdpRect
{
public:
    long            x;
    long            y;
    unsigned long   w;
    unsigned long   h;
    
public:
    MdpRect():
        x(0),y(0),w(0),h(0)
        {};

   MdpRect(long _x, long _y, unsigned long _w, unsigned long _h )
        {
            x = _x; y = _y; w = _w; h = _h;
        };
    
    MdpRect(const MdpSize& size )
        {
            x = 0; y = 0; w = size.w; h = size.h;
        };

    MdpRect& operator=( const MdpRect& rhs )
    {
        x = rhs.x; y = rhs.y; w = rhs.w; h = rhs.h;
        return *this;
    }

    bool operator==(const MdpRect& rhs) const
    {
        return (  (x == rhs.x) && ( y == rhs.y) && ( w == rhs.w ) && ( h == rhs.h ) );
    }

    bool operator!=(const MdpRect& rhs) const
    {
        return !( *this == rhs );
    }
    
    
};

class MdpYuvAddr
{
public:
    unsigned long   y;
    unsigned long   u;
    unsigned long   v;
    
    /*User need not fill in the data below!-----------------------------------------------------------*/
    /*Below data is auto fill by MDP driver calculation!----------------------------------------------*/
    
    unsigned long   y_buffer_size; 
    unsigned long   u_buffer_size; 
    unsigned long   v_buffer_size;

    unsigned long   m4u_handle;

public:
    MdpYuvAddr():
        y(0), u(0), v(0),
        y_buffer_size(0), u_buffer_size(0), v_buffer_size(0),
        m4u_handle(0)
        {};
};


class MdpMemBuffer
{
public:
    unsigned long address;
    unsigned long size;

public:
    MdpMemBuffer():
        address(0), size(0)
        {};
};



typedef enum MdpColorFormat
{
    /*-----------------------------------------------------------------------------
        RGB
      -----------------------------------------------------------------------------*/
    RGB888 = 0,
    BGR888,
    RGB565,
    BGR565,
    ABGR8888,
    ARGB8888,
    BGRA8888,
    RGBA8888,

    /*-----------------------------------------------------------------------------
        YUV Packed
      -----------------------------------------------------------------------------*/
    //UYVY = YUV 4:2:2 (Y sample at every pixel, U and V sampled at every second pixel horizontally on each line). A macropixel contains 2 pixels in 1 u_int32    
    UYVY_Pack,
    YUV422_Pack     = UYVY_Pack,
    
    //YUYV = YUY2 = YUV 4:2:2 as for UYVY but with different component ordering within the u_int32 macropixel.
    YUYV_Pack,
    YUY2_Pack       = YUYV_Pack,
    YUV422_2_Pack   = YUYV_Pack,

    //YUV 4:1:1 with a packed, 6 byte/4 pixel macroblock structure.
    Y411_Pack,


    /*-----------------------------------------------------------------------------
        YUV Plane
      -----------------------------------------------------------------------------*/
    //YUV422, 2x1 subsampled U/V planes
    YV16_Planar,
    YUV422_Planar   = YV16_Planar,

    //YUV420, 2x2 subsampled U/V planes
    YV12_Planar,    //This is WRONG!!.YV12 should be YVU, if YUV should be named i420
    YUV420_Planar   = YV12_Planar,

    //Android defined YV12 (YVU420 Planar, 2x2 subsampled, with 16x aligned Y,V,U stride 
    ANDROID_YV12,

    //Imagination defined YV12 (YVU420 Planar, 2x2 subsampled, with 32x aligned Y,V,U stride 
    IMG_YV12,

    

    //Y plan only,only ROTDMA0
    Y800,
    Y8  = Y800,
    GREY= Y800,




    /*-----------------------------------------------------------------------------
        YUV Interleave
      -----------------------------------------------------------------------------*/
    //YUV420, 2x2 subsampled , interleaved U/V plane,only ROTDMA0
    NV12,
    YUV420_Inter    = NV12,

    //YUV420, 2x2 subsampled , interleaved V/U plane,only ROTDMA0
    NV21,
    YVU420_Inter    = NV21,

    
    /*-----------------------------------------------------------------------------
        Misc
      -----------------------------------------------------------------------------*/
    //For encoder use,only ROTDMA0 (So called "MTK YUV")
    YUV420_4x4BLK,
    MTK_YUV420  = YUV420_4x4BLK,


    //Output the same data with input
    RAW,    


} MdpColorFormat;




/*-----------------------------------------------------------------------------
    DEBUG LOG UTILITY
  -----------------------------------------------------------------------------*/
#define DEBUG_STR_BEGIN     ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>"
#define DEBUG_STR_END       "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<"


/*
     V    Verbose
     D    Debug
     I    Info
     W    Warn
     E    Error
     F    Fatal
     S    Silent
 */
extern int g_b_show_info_resource;  /*1*/
extern int g_b_show_info_fps;       /*2*/
extern int g_b_show_info_queue;     /*3*/
extern int g_b_show_info_mem;       /*4*/
extern int g_b_show_info_path;      /*5*/
extern int g_b_show_info_mdpd;      /*6*/
extern int g_b_show_info;           /*7*//*verbose*/
extern int g_b_profile;             /*8*/
extern int g_profile_level;         /*9*/



#if defined( MDP_FLAG_USE_XLOG  )
#include    <cutils/xlog.h>

#define MDPLOGE XLOGE
#define MDPLOGD XLOGD
#define MDPLOGI XLOGI
#define MDPLOGW XLOGW
#else
#include    <cutils/log.h>
    #if 0 /*GB/ICS version*/
    #define MDPLOGE LOGE
    #define MDPLOGD LOGD
    #define MDPLOGI LOGI
    #define MDPLOGW LOGW
    #else /*JB version*/
    #define MDPLOGE ALOGE
    #define MDPLOGD ALOGD
    #define MDPLOGI ALOGI
    #define MDPLOGW ALOGW
    #endif
#endif


#define MDP_PRINTF                          MDPLOGE
#define MDP_SHOWFUNCTION()                  if( g_b_show_info ) { MDPLOGD("[MDP Func Call](%lu): %s()",(unsigned long)gettid(), __FUNCTION__ ); }


#define MDP_INFO_RESOURCE(fmt, arg...)      if( g_b_show_info_resource )    { MDPLOGI("[MDP RESOURCE](%lu): "fmt,(unsigned long)gettid(), ##arg);   }
#define MDP_INFO_FPS(fmt, arg...)           if( g_b_show_info_fps )         { MDPLOGI("[MDP FPS](%lu): "fmt,(unsigned long)gettid(), ##arg);  }
#define MDP_INFO_QUEUE(fmt, arg...)         if( g_b_show_info_queue )       { MDPLOGI("[MDP QUEUE](%lu): "fmt,(unsigned long)gettid(), ##arg);   }
#define MDP_INFO_MEM(fmt, arg...)           if( g_b_show_info_mem )         { MDPLOGI("[MDP MEM](%lu): "fmt,(unsigned long)gettid(), ##arg);   }
#define MDP_INFO_PATH(fmt, arg...)          if( g_b_show_info_path )        { MDPLOGI("[MDP PATH](%lu): "fmt,(unsigned long)gettid(), ##arg);   }
#define MDP_INFO_MDPD(fmt, arg...)          if( g_b_show_info_mdpd )        { MDPLOGI("[MDP MDPD](%lu): "fmt,(unsigned long)gettid(), ##arg);   }
#define MDP_INFO(fmt, arg...)               if( g_b_show_info )             { MDPLOGI("[MDP INFO](%lu): "fmt,(unsigned long)gettid(), ##arg);   }




//Always on info & debug usage
#define MDP_INFO_PERSIST(fmt, arg...)   { MDPLOGI("[MDP INFO](%lu): "fmt,(unsigned long)gettid(), ##arg);   }
#define MDP_ERROR(fmt, arg...)          { MDPLOGE("[MDP E](%lu): %s(): %s@%d: "fmt,(unsigned long)gettid(),__FUNCTION__, __FILE__,__LINE__, ##arg);MdpDrv_DumpCallStack(NULL);  }
#define MDP_WARNING(fmt, arg...)        MDPLOGW("[MDP W](%lu): %s(): "fmt,(unsigned long)gettid(),__FUNCTION__, ##arg)
#define MDP_DEBUG(fmt, arg...)          MDPLOGE("[MDP DEBUG](%lu): "fmt,(unsigned long)gettid(), ##arg)
#define MDP_PROFILE(fmt, arg...)        if( g_b_profile )     {   MDPLOGI("[MDP PROFILE](%lu): "fmt,(unsigned long)gettid() ,##arg);  }

#define MDP_DUMP_VAR_H(_x_)             MDP_PRINTF("\t\t"#_x_"=0x%08X\n",(unsigned int)_x_);
#define MDP_DUMP_VAR_D(_x_)             MDP_PRINTF("\t\t"#_x_"=%d\n",(int)_x_);
#define MDP_DUMP_VAR_S(_x_)             MDP_PRINTF("\t\t"#_x_"=%s\n",_x_);

#define MDP_SHOW_VAR_H(_x_)             MDP_INFO("\t\t"#_x_"=0x%08X\n",(unsigned int)_x_);
#define MDP_SHOW_VAR_D(_x_)             MDP_INFO("\t\t"#_x_"=%d\n",(int)_x_);
#define MDP_SHOW_VAR_S(_x_)             MDP_INFO("\t\t"#_x_"=%s\n",_x_);








/*-----------------------------------------------------------------------------
    UTILITY MACRO
  -----------------------------------------------------------------------------*/
#define MDP_MAX3(a, b, c) (((a) > (((b) > (c)) ? (b) : (c))) ? (a) : (((b) > (c)) ? (b) : (c)))
#define MDP_IS_ALIGN( _number_, _power_of_2_ )      ( ( (_number_) & ( (0x1<<(_power_of_2_))-1 ) ) ? 0 : 1 )
#define MDP_ROUND_UP( _number_, _power_of_2_ )      _number_ = ( ( ((_number_) + (( 0x1 << (_power_of_2_) )-1)) >> (_power_of_2_) ) << (_power_of_2_) )
#define MDP_ROUND_DOWN( _number_, _power_of_2_ )    _number_ =     ((_number_) & ~(( 0x1 << (_power_of_2_) )-1))

#define MDP_ROUND_UP_VALUE( _number_, _power_of_2_ )      ( ( ((_number_) + (( 0x1 << (_power_of_2_) )-1)) >> (_power_of_2_) ) << (_power_of_2_) )
#define MDP_ROUND_DOWN_VALUE( _number_, _power_of_2_ )        ((_number_) & ~(( 0x1 << (_power_of_2_) )-1))




#endif /*__MDP_DATATYPES_H__*/



