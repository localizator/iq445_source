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

#ifndef __MHAL_INTERFACE_H__
#define __MHAL_INTERFACE_H__


#include "mdp_hal.h"        //For halIDPParam_t datastructure (Camera)
#include "MediaHal.h"       //For MHAL color format


/*******************************************************************************
    Interface to upper client
 *******************************************************************************/
/**
    mHal should call mdp_pipe function call, which is a set of platform independent function call,
    thus, mHal implementation should be platform independent,
    no need to add mt6575 prefix.

    the mt6575 prefix is due to 73 mhal is platform dependent,
    so I add this prefix for temporary
    **/

/*-----------------------------------------------------------------------------
    Data Type
  -----------------------------------------------------------------------------*/
typedef enum
{
    MEM_TYPE_INPUT,
    MEM_TYPE_OUTPUT
    
} REGLOOPMEM_TYPE;

class RegisterLoopMemory_t
{
public:
    REGLOOPMEM_TYPE             mem_type;
    unsigned long               addr;
    unsigned long               buffer_size;
    MHAL_BITBLT_FORMAT_ENUM     mhal_color;
    MdpSize                     img_size;
    MdpRect                     img_roi;
    unsigned long               rotate; //0:0 1:90 2:180 3:270.rotate always 0 when used by RDMA(input memory)

public:
    RegisterLoopMemory_t():
        mem_type(MEM_TYPE_INPUT),
        addr(0),
        buffer_size(0),
        mhal_color(MHAL_FORMAT_RGB_565),
        rotate(0) //0:0 1:90 2:180 3:270.rotate always 0 when used by RDMA(input memory)
        {};
        
};

class RegisterLoopMemoryObj_t
{
public:
    unsigned long       mdp_id;
    MdpYuvAddr          calc_addr[1];
    MdpYuvAddr          adapt_addr[1];
    unsigned long       adapt_m4u_flag_bit;
    unsigned long       alloc_mva_flag_bit;

public:
    RegisterLoopMemoryObj_t():
        mdp_id(0),
        adapt_m4u_flag_bit(0),
        alloc_mva_flag_bit(0)
        {};
};

/*-----------------------------------------------------------------------------
    Function Pototype
  -----------------------------------------------------------------------------*/
MdpColorFormat          MhalColorFormatToMdp( MHAL_BITBLT_FORMAT_ENUM mhal_bitblt_color_format );
MHAL_BITBLT_FORMAT_ENUM MdpColorFormatToMhal( MdpColorFormat mdp_color );


int Mt6575_mHalBitblt(void *a_pInBuffer); /*mt6575 prefix should not add, this should be platform independent implement*/
int Mt6575_mHalCameraPreview( halIDPParam_t *phalIDPParam );

int Mt6575_mHalCrzLbLock( void );
int Mt6575_mHalCrzLbUnLock( void );


int Mdp_RegisterLoopMemory(   MVALOOPMEM_CLIENT client_id, RegisterLoopMemory_t* p_param, RegisterLoopMemoryObj_t* p_out_obj );
int Mdp_UnRegisterLoopMemory( MVALOOPMEM_CLIENT client_id, RegisterLoopMemoryObj_t* p_obj );

int Mdp_BitBltEx( mHalBltParamEx_t* p_param  );
int Mdp_BitbltSlice(void *a_pInBuffer);







#endif /*__MT6575_MHAL_INTERFACE_H__*/
