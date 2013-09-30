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

#ifndef __MT6575MDPREG_h__
#define __MT6575MDPREG_h__

/////////////////////////Mux and MOUT
#define MT6575_MMSYS1_BASE    0xC2080000
//
#define MT6575_MMSYS_CG_CON1(i)        *((volatile unsigned long*)(i + 0x004))
//bit[0] : 0-R_DMA0_MOUT, 1-BRZ_MOUT ; bit[15] : 0 - no Bypass, 1 - Bypass
#define MT6575_MMSYS_CAM_ISEL(i)       *((volatile unsigned long*)(i + 0x100))
//bit[0] : 0-OVL_DMA_MIMO, 1-CRZ
#define MT6575_MMSYS_IPP_ISEL(i)       *((volatile unsigned long*)(i + 0x104))
//bit[0] : 0-R_DMA0_MOUT, 1-CRZ
#define MT6575_MMSYS_OVL_DMA_ISEL(i)   *((volatile unsigned long*)(i + 0x108))
//bit[0] : 0-R_DMA0_MOUT, 1-OVL_DMA_MIMO
#define MT6575_MMSYS_MOUT_ISEL(i)      *((volatile unsigned long*)(i + 0x10C))
//bit[0] : 0-PRZ0_MOUT, 1-IPP_MOUT
#define MT6575_MMSYS_VDO_ROT0_ISEL(i)  *((volatile unsigned long*)(i + 0x110))
//bit[0:1] : 0-MOUT,1-IPP_MOUT,2-CAM,3-BRZ_MOUT
#define MT6575_MMSYS_PRZ0_ISEL(i)      *((volatile unsigned long*)(i + 0x114))
//bit[0:2] : 0-MOUT,1-BRZ_MOUT,2-IPP_MOUT,3-OVL_DMA_MIMO,4-PRZ0_MOUT
#define MT6575_MMSYS_VRZ0_ISEL(i)       *((volatile unsigned long*)(i + 0x118))
//bit[0] : 0-MOUT, 1-VRZ
#define MT6575_MMSYS_VDO_ROT1_ISEL(i)  *((volatile unsigned long*)(i + 0x11C))
//bit[0] : 0-R_DMA1, 1-BRZ_MOUT
#define MT6575_MMSYS_VRZ1_ISEL(i)      *((volatile unsigned long*)(i + 0x120))

//bit[0] : 0-IPP_MOUT, 1-MOUT
#define MT6575_MMSYS_JPEG_DMA_ISEL(i)      *((volatile unsigned long*)(i + 0x124))
//bit[0] : 0-PRZ0_MOUT, 1-IPP_MOUT
#define MT6575_MMSYS_RGB_ROT0_ISEL(i)      *((volatile unsigned long*)(i + 0x128))

#define MT6575_MMSYS_R_DMA0_MOUT_EN(i)         *((volatile unsigned long*)(i + 0x130))
#define MT6575_MMSYS_R_DMA0_MOUT_CLR(i)        *((volatile unsigned long*)(i + 0x134))
//bit[0] : CAM, bit[1] : OVL_DMA, bit[2] : MOUT
#define MT6575_MMSYS_R_DMA0_MOUT_SEL(i)        *((volatile unsigned long*)(i + 0x138))
#define MT6575_MMSYS_R_DMA0_MOUT_CON(i)        *((volatile unsigned long*)(i + 0x13C))

#define MT6575_MMSYS_IPP_MOUT_EN(i)            *((volatile unsigned long*)(i + 0x140))
#define MT6575_MMSYS_IPP_MOUT_CLR(i)           *((volatile unsigned long*)(i + 0x144))
//bit[0] : JPEG_DMA, bit[1] : VDO_ROT0, bit[2] : PRZ0,bit[3] : VRZ
#define MT6575_MMSYS_IPP_MOUT_SEL(i)           *((volatile unsigned long*)(i + 0x148))
#define MT6575_MMSYS_IPP_MOUT_CON(i)           *((volatile unsigned long*)(i + 0x14C))

#define MT6575_MMSYS_MOUT_EN(i)                *((volatile unsigned long*)(i + 0x150))
#define MT6575_MMSYS_MOUT_CLR(i)               *((volatile unsigned long*)(i + 0x154))
//bit[0] : JPEG_DMA, bit[1] : PRZ0, bit[2] : VRZ,bit[3] : VDO_ROT1
#define MT6575_MMSYS_MOUT_SEL(i)               *((volatile unsigned long*)(i + 0x158))
#define MT6575_MMSYS_MOUT_CON(i)               *((volatile unsigned long*)(i + 0x15C))

#define MT6575_MMSYS_PRZ0_MOUT_EN(i)           *((volatile unsigned long*)(i + 0x160))
#define MT6575_MMSYS_PRZ0_MOUT_CLR(i)          *((volatile unsigned long*)(i + 0x164))
//bit[0] : VDO_ROT0, bit[1] : RGB_ROT0, bit[2] : VRZ
#define MT6575_MMSYS_PRZ0_MOUT_SEL(i)          *((volatile unsigned long*)(i + 0x168))
#define MT6575_MMSYS_PRZ0_MOUT_CON(i)          *((volatile unsigned long*)(i + 0x16C))

#define MT6575_MMSYS_OVL_DMA_MIMO_EN(i)        *((volatile unsigned long*)(i + 0x170))
#define MT6575_MMSYS_OVL_DMA_MIMO_CLR(i)       *((volatile unsigned long*)(i + 0x174))
#define MT6575_MMSYS_OVL_DMA_MIMO_CON(i)       *((volatile unsigned long*)(i + 0x178))
//bit[0] : IPP, bit[1] : VRZ, bit[2] : MOUT
#define MT6575_MMSYS_OVL_DMA_MIMO_OUTEN(i)     *((volatile unsigned long*)(i + 0x17C))
//bit[0:1] : IPP, bit[2:3] : VRZ, bit[4:5] : MOUT
#define MT6575_MMSYS_OVL_DMA_MIMO_OUTSEL(i)    *((volatile unsigned long*)(i + 0x180))
//bit[0:1] : 0-VDO_ROT0,1-RGB_ROT0,2-RGB_ROT1,3-VDO_ROT1
#define MT6575_MMSYS_R_DMA0_TRIG_SEL(i)    *((volatile unsigned long*)(i + 0x190))

#define MT6575_MMSYS_MDP_REQ(i)                 *((volatile unsigned long*)(i + 0x200))
#define MT6575_MMSYS_MDP_REQ0(i)                *((volatile unsigned long*)(i + 0x200))
#define MT6575_MMSYS_MDP_REQ1(i)                *((volatile unsigned long*)(i + 0x204))



#define MT6575_MMSYS_BRZ_MOUT_EN(i)           *((volatile unsigned long*)(i + 0x220))
#define MT6575_MMSYS_BRZ_MOUT_CLR(i)          *((volatile unsigned long*)(i + 0x224))
//bit[0] : PRZ1 , bit[1] : PRZ0 , bit[2] : VRZ, bit[3] : CAM
#define MT6575_MMSYS_BRZ_MOUT_SEL(i)          *((volatile unsigned long*)(i + 0x228))
#define MT6575_MMSYS_BRZ_MOUT_CON(i)          *((volatile unsigned long*)(i + 0x22C))

#define MT6575_MMSYS1_REG_RANGE    0x230


//BRZ
#define MT6575_BRZ_BASE 0xC208D000
#define MT6575_BRZ_CON(i)      *((volatile unsigned long*)i)
#define MT6575_BRZ_STA(i)      *((volatile unsigned long*)(i + 0x4))
#define MT6575_BRZ_INT(i)       *((volatile unsigned long*)(i + 0x8))
#define MT6575_BRZ_SRCSZ(i)   *((volatile unsigned long*)(i + 0xc))
#define MT6575_BRZ_BLKSCSFG(i)   *((volatile unsigned long*)(i + 0x10))
#define MT6575_BRZ_YLMBASE(i)     *((volatile unsigned long*)(i + 0x14))
#define MT6575_BRZ_ULMBASE(i)     *((volatile unsigned long*)(i + 0x18))
#define MT6575_BRZ_VLMBASE(i)     *((volatile unsigned long*)(i + 0x1c))
#define MT6575_BRZ_YLBSIZE(i)     *((volatile unsigned long*)(i + 0x20))
#define MT6575_BRZ_INFO0(i)        *((volatile unsigned long*)(i + 0x2c))
#define MT6575_BRZ_INFO1(i)        *((volatile unsigned long*)(i + 0x30))
#define MT6575_BRZ_REG_RANGE    0x34




//CRZ, PRZ
#define MT6575_CRZ_BASE 0xc2093000
#define MT6575_PRZ0_BASE 0xc2099000
#define MT6575_PRZ1_BASE 0xc209A000
#define MT6575_RZ_CFG(i)       *((volatile unsigned long*)(i + 0x0000))
#define MT6575_RZ_CON(i)       *((volatile unsigned long*)(i + 0x0004))
#define MT6575_RZ_STA(i)       *((volatile unsigned long*)(i + 0x0008))
#define MT6575_RZ_INT(i)       *((volatile unsigned long*)(i + 0x000C))
#define MT6575_RZ_SRCSZ(i)     *((volatile unsigned long*)(i + 0x0010))
#define MT6575_RZ_TARSZ(i)     *((volatile unsigned long*)(i + 0x0014))
#define MT6575_RZ_HRATIO(i)    *((volatile unsigned long*)(i + 0x0018))
#define MT6575_RZ_VRATIO(i)    *((volatile unsigned long*)(i + 0x001C))
#define MT6575_RZ_LOCK(i)      *((volatile unsigned long*)(i + 0x0020))
#define MT6575_RZ_LMU(i)       *((volatile unsigned long*)(i + 0x0024))
#define MT6575_RZ_COEFF(i)     *((volatile unsigned long*)(i + 0x0040))
#define MT6575_RZ_BUSYU(i)     *((volatile unsigned long*)(i + 0x0080)) // only CRZ
#define MT6575_RZ_BUSYD(i)     *((volatile unsigned long*)(i + 0x0084)) // only CRZ
#define MT6575_RZ_INFO0(i)     *((volatile unsigned long*)(i + 0x00B0)) // only CRZ
#define MT6575_RZ_INFO1(i)     *((volatile unsigned long*)(i + 0x00B4)) // only CRZ
#define MT6575_RZ_ORIGSZ(i)    *((volatile unsigned long*)(i + 0x00F0))
#define MT6575_RZ_CROPLR(i)    *((volatile unsigned long*)(i + 0x00F4))
#define MT6575_RZ_CROPTB(i)    *((volatile unsigned long*)(i + 0x00F8))
#define MT6575_CRZ_REG_RANGE    0xFC
#define MT6575_PRZ0_REG_RANGE   0xFC
#define MT6575_PRZ1_REG_RANGE   0xFC


//IPP
#define MT6575_IPP_BASE      0xC2095000
#define MT6575_IMGPROC_IPP_CFG(i)             *((volatile unsigned long*)(i + 0x0000))
#define MT6575_IMGPROC_R2Y_CFG(i)             *((volatile unsigned long*)(i + 0x0004))
#define MT6575_IMGPROC_HUE11(i)               *((volatile unsigned long*)(i + 0x0100))
#define MT6575_IMGPROC_HUE12(i)               *((volatile unsigned long*)(i + 0x0104))
#define MT6575_IMGPROC_HUE21(i)               *((volatile unsigned long*)(i + 0x0108))
#define MT6575_IMGPROC_HUE22(i)               *((volatile unsigned long*)(i + 0x010C))
#define MT6575_IMGPROC_SAT(i)                 *((volatile unsigned long*)(i + 0x0110))
#define MT6575_IMGPROC_BRIADJ1(i)             *((volatile unsigned long*)(i + 0x0120))
#define MT6575_IMGPROC_BRIADJ2(i)             *((volatile unsigned long*)(i + 0x0124))
#define MT6575_IMGPROC_CONADJ(i)              *((volatile unsigned long*)(i + 0x0128))
#define MT6575_IMGPROC_COLORIZEU(i)           *((volatile unsigned long*)(i + 0x0130))
#define MT6575_IMGPROC_COLORIZEV(i)           *((volatile unsigned long*)(i + 0x0134))
#define MT6575_IMGPROC_COLOR1R_OFFX(i)        *((volatile unsigned long*)(i + 0x0200))
#define MT6575_IMGPROC_COLOR2R_OFFX(i)        *((volatile unsigned long*)(i + 0x0204))
#define MT6575_IMGPROC_COLOR1G_OFFX(i)        *((volatile unsigned long*)(i + 0x0208))
#define MT6575_IMGPROC_COLOR2G_OFFX(i)        *((volatile unsigned long*)(i + 0x020C))
#define MT6575_IMGPROC_COLOR1B_OFFX(i)        *((volatile unsigned long*)(i + 0x0210))
#define MT6575_IMGPROC_COLOR2B_OFFX(i)        *((volatile unsigned long*)(i + 0x0214))
#define MT6575_IMGPROC_COLOR1R_OFFY(i)        *((volatile unsigned long*)(i + 0x0220))
#define MT6575_IMGPROC_COLOR2R_OFFY(i)        *((volatile unsigned long*)(i + 0x0224))
#define MT6575_IMGPROC_COLOR1G_OFFY(i)        *((volatile unsigned long*)(i + 0x0228))
#define MT6575_IMGPROC_COLOR2G_OFFY(i)        *((volatile unsigned long*)(i + 0x022C))
#define MT6575_IMGPROC_COLOR1B_OFFY(i)        *((volatile unsigned long*)(i + 0x0230))
#define MT6575_IMGPROC_COLOR2B_OFFY(i)        *((volatile unsigned long*)(i + 0x0234))
#define MT6575_IMGPROC_COLOR0R_SLP(i)         *((volatile unsigned long*)(i + 0x0240))
#define MT6575_IMGPROC_COLOR1R_SLP(i)         *((volatile unsigned long*)(i + 0x0244))
#define MT6575_IMGPROC_COLOR2R_SLP(i)         *((volatile unsigned long*)(i + 0x0248))
#define MT6575_IMGPROC_COLOR0G_SLP(i)         *((volatile unsigned long*)(i + 0x0250))
#define MT6575_IMGPROC_COLOR1G_SLP(i)         *((volatile unsigned long*)(i + 0x0254))
#define MT6575_IMGPROC_COLOR2G_SLP(i)         *((volatile unsigned long*)(i + 0x0258))
#define MT6575_IMGPROC_COLOR0B_SLP(i)         *((volatile unsigned long*)(i + 0x0260))
#define MT6575_IMGPROC_COLOR1B_SLP(i)         *((volatile unsigned long*)(i + 0x0264))
#define MT6575_IMGPROC_COLOR2B_SLP(i)         *((volatile unsigned long*)(i + 0x0268))
#define MT6575_IMGPROC_EN(i)                  *((volatile unsigned long*)(i + 0x0320))
#define MT6575_IMGPROC_IPP_RGB_DETECT(i)      *((volatile unsigned long*)(i + 0x0324))
#define MT6575_IMGPROC_IPP_RGB_REPLACE(i)     *((volatile unsigned long*)(i + 0x0328))
#define MT6575_IPP_REG_RANGE      0x32C



//OVL
#define MT6575_OVL_DMA_BASE 0xC208F000
#define MT6575_OVL_DMA_IRQ_FLAG(i)              *((volatile unsigned long*)i)
#define MT6575_OVL_DMA_IRQ_FLAG_CLR(i)      *((volatile unsigned long*)(i + 0x8))
#define MT6575_OVL_DMA_CFG(i)                       *((volatile unsigned long*)(i + 0x18))
#define MT6575_OVL_DMA_IN_SEL(i)                  *((volatile unsigned long*)(i + 0x20))
#define MT6575_OVL_DMA_STOP(i)                     *((volatile unsigned long*)(i + 0x28))
#define MT6575_OVL_DMA_EN(i)                         *((volatile unsigned long*)(i + 0x30))
#define MT6575_OVL_DMA_RESET(i)                   *((volatile unsigned long*)(i + 0x38))
#define MT6575_OVL_DMA_LOCK(i)                     *((volatile unsigned long*)(i + 0x3C))
#define MT6575_OVL_DMA_QUEUE_RSTA(i)        *((volatile unsigned long*)(i + 0x40))
#define MT6575_OVL_DMA_QUEUE_WSTA(i)       *((volatile unsigned long*)(i + 0x58))
#define MT6575_OVL_DMA_QUEUE_DATA(i)        *((volatile unsigned long*)(i + 0x70))
#define MT6575_OVL_DMA_QUEUE_BASE(i)        *((volatile unsigned long*)(i + 0x78))
#define MT6575_OVL_DMA_EXEC_CNT(i)             *((volatile unsigned long*)(i + 0x80))
#define MT6575_OVL_DMA_REPLACE_COLOR(i)   *((volatile unsigned long*)(i + 0x90))
#define MT6575_OVL_DMA_HEADER_BASE(i)      *((volatile unsigned long*)(i + 0x100))
#define MT6575_OVL_DMA_MASK_BASE(i)          *((volatile unsigned long*)(i + 0x108))
#define MT6575_OVL_DMA_SRC_SIZE(i)             *((volatile unsigned long*)(i + 0x110))
#define MT6575_OVL_DMA_OFFSET(i)                *((volatile unsigned long*)(i + 0x118))
#define MT6575_OVL_DMA_MASK_SRC_SIZE(i)  *((volatile unsigned long*)(i + 0x120))
#define MT6575_OVL_DMA_MASK_DST_SIZE(i)  *((volatile unsigned long*)(i + 0x128))
#define MT6575_OVL_DMA_MASK_CFG(i)           *((volatile unsigned long*)(i + 0x130))
#define MT6575_OVL_DMA_CON(i)                     *((volatile unsigned long*)(i + 0x138))
#define MT6575_OVL_DMA_CUR_SRC_POSITION(i)                     *((volatile unsigned long*)(i + 0x140))
#define MT6575_OVL_DMA_CUR_MASK_POSITION(i)                  *((volatile unsigned long*)(i + 0x148))
#define MT6575_OVL_DMA_REG_RANGE 0x14C



//VRZ
#define MT6575_VRZ0_BASE      0xC2094000
#define MT6575_VRZ1_BASE      0xC20A4000
#define MT6575_VRZ_CFG(i)       *((volatile unsigned long*)(i + 0x0000))
#define MT6575_VRZ_CON(i)       *((volatile unsigned long*)(i + 0x0004))
#define MT6575_VRZ_STA(i)       *((volatile unsigned long*)(i+ 0x0008))
#define MT6575_VRZ_INT(i)       *((volatile unsigned long*)(i + 0x000C))
#define MT6575_VRZ_SRCSZ(i)     *((volatile unsigned long*)(i + 0x0010))
#define MT6575_VRZ_TARSZ(i)     *((volatile unsigned long*)(i + 0x0014))
#define MT6575_VRZ_HRATIO(i)    *((volatile unsigned long*)(i + 0x0018))
#define MT6575_VRZ_VRATIO(i)    *((volatile unsigned long*)(i + 0x001C))
#define MT6575_VRZ_HRES(i)      *((volatile unsigned long*)(i + 0x0020))
#define MT6575_VRZ_VRES(i)      *((volatile unsigned long*)(i + 0x0024))
#define MT6575_VRZ_FRFG(i)      *((volatile unsigned long*)(i + 0x0040))
#define MT6575_VRZ_PREMBASE(i)  *((volatile unsigned long*)(i + 0x005C))
#define MT6575_VRZ_INFO0(i)     *((volatile unsigned long*)(i + 0x00B0))
#define MT6575_VRZ_INFO1(i)     *((volatile unsigned long*)(i + 0x00B4))

#define MT6575_VRZ0_REG_RANGE      0xB8
#define MT6575_VRZ1_REG_RANGE      0xB8



//RDMA
#define MT6575_RDMA0_BASE 0xC2086000
#define MT6575_RDMA1_BASE 0xC2087000
#define MT6575_RDMA_IRQ_FLAG(i)           *((volatile unsigned long*)i)
#define MT6575_RDMA_IRQ_FLAG_CLR(i)   *((volatile unsigned long*)(i + 0x0008))
#define MT6575_RDMA_CFG(i)                *((volatile unsigned long*)(i + 0x0018))
#define MT6575_RDMA_IN_SEL(i)	          *((volatile unsigned long*)(i + 0x0020))
#define MT6575_RDMA_STOP(i)               *((volatile unsigned long*)(i + 0x0028))
#define MT6575_RDMA_EN(i)	                *((volatile unsigned long*)(i + 0x0030))
#define MT6575_RDMA_RESET(i)	            *((volatile unsigned long*)(i + 0x0038))
#define MT6575_RDMA_QUEUE_RSTA(i)	        *((volatile unsigned long*)(i + 0x0040))
#define MT6575_RDMA_RD_BASE(i)	          *((volatile unsigned long*)(i + 0x0048))
#define MT6575_RDMA_RPT_ADV(i)	          *((volatile unsigned long*)(i + 0x0050))
#define MT6575_RDMA_QUEUE_WSTA(i)	        *((volatile unsigned long*)(i + 0x0058))
#define MT6575_RDMA_WR_BASE(i)	          *((volatile unsigned long*)(i + 0x0060))
#define MT6575_RDMA_WPT_ADV(i)	          *((volatile unsigned long*)(i + 0x0068))
#define MT6575_RDMA_QUEUE_DATA(i)	        *((volatile unsigned long*)(i + 0x0070))
#define MT6575_RDMA_QUEUE_BASE(i)	        *((volatile unsigned long*)(i + 0x0078))
#define MT6575_RDMA_EXEC_CNT(i)           *((volatile unsigned long*)(i + 0x0080))
#define MT6575_RDMA_Y_SRC_STR_ADDR(i)	    *((volatile unsigned long*)(i + 0x0340))// #descriptor
#define MT6575_RDMA_U_SRC_STR_ADDR(i)	    *((volatile unsigned long*)(i + 0x0344))// #descriptor
#define MT6575_RDMA_V_SRC_STR_ADDR(i)	    *((volatile unsigned long*)(i + 0x0348))// #descriptor
#define MT6575_RDMA_SRC_SIZE(i)           *((volatile unsigned long*)(i + 0x034c))// #descriptor
#define MT6575_RDMA_SRC_SIZE_IN_BYTE(i)   *((volatile unsigned long*)(i + 0x0354))// #descriptor
#define MT6575_RDMA_CLIP_SIZE(i)	        *((volatile unsigned long*)(i + 0x0358))// #descriptor
#define MT6575_RDMA_CLIP_SIZE_IN_BYTE(i)	*((volatile unsigned long*)(i + 0x035c))// #descriptor
#define MT6575_RDMA_CON(i)                *((volatile unsigned long*)(i + 0x0360))// #descriptor
#define MT6575_RDMA_SLOW_DOWN(i)	        *((volatile unsigned long*)(i + 0x0370))
#define MT6575_RDMA_EIS_STR(i)	          *((volatile unsigned long*)(i + 0x0384))
#define MT6575_RDMA_EIS_CON(i)	          *((volatile unsigned long*)(i + 0x0388))
#define MT6575_RDMA_DEBUG_STATUS0(i)	    *((volatile unsigned long*)(i + 0x0390))
#define MT6575_RDMA_DEBUG_STATUS1(i)	    *((volatile unsigned long*)(i + 0x0394))
#define MT6575_RDMA_DEBUG_STATUS2(i)	    *((volatile unsigned long*)(i + 0x0398))
#define MT6575_RDMA_DEBUG_STATUS3(i)	    *((volatile unsigned long*)(i + 0x039c))
#define MT6575_RDMA_DEBUG_STATUS4(i)	    *((volatile unsigned long*)(i + 0x03a0))
#define MT6575_RDMA_DEBUG_STATUS5(i)	    *((volatile unsigned long*)(i + 0x03a4))
#define MT6575_RDMA_DEBUG_STATUS6(i)	    *((volatile unsigned long*)(i + 0x03a8))
#define MT6575_RDMA_DEBUG_STATUS7(i)	    *((volatile unsigned long*)(i + 0x03ac))
#define MT6575_RDMA_DEBUG_STATUS8(i)	    *((volatile unsigned long*)(i + 0x03b0))
#define MT6575_RDMA_DEBUG_STATUS9(i)	    *((volatile unsigned long*)(i + 0x03b4))
#define MT6575_RDMA_DEBUG_STATUS10(i)    *((volatile unsigned long*)(i + 0x03b8))
#define MT6575_RDMA_DEBUG_STATUS11(i)    *((volatile unsigned long*)(i + 0x03bc))
#define MT6575_RDMA_DEBUG_STATUS12(i)    *((volatile unsigned long*)(i + 0x03c0))
#define MT6575_RDMA_DEBUG_STATUS13(i)    *((volatile unsigned long*)(i + 0x03c4))
#define MT6575_RDMA_DEBUG_STATUS14(i)    *((volatile unsigned long*)(i + 0x03c8))
#define MT6575_RDMA_DEBUG_STATUS15(i)    *((volatile unsigned long*)(i + 0x03cc))
#define MT6575_RDMA0_REG_RANGE 0x3D0
#define MT6575_RDMA1_REG_RANGE 0x3D0



//ROTDMA
#define MT6575_RGB_ROT0_BASE        0xC2089000
#define MT6575_RGB_ROT1_BASE        0xC208B000
#define MT6575_RGB_ROT2_BASE        0xC20A0000
#define MT6575_VDO_ROT0_BASE        0xC2088000
#define MT6575_VDO_ROT1_BASE        0xC208A000
#define MT6575_TV_ROT_BASE          0xC209F000
#define MT6575_LCD_BASE             0xC20A1000
#define MT6575_ROT_DMA_IRQ_FLAG(i)              *((volatile unsigned long*)i)
#define MT6575_ROT_DMA_IRQ_FLAG_CLR(i)      *((volatile unsigned long*)(i + 0x8))
#define MT6575_ROT_DMA_CFG(i)                *((volatile unsigned long*)(i + 0x18))
#define MT6575_ROT_DMA_IN_SEL(i)           *((volatile unsigned long*)(i + 0x20))
#define MT6575_ROT_DMA_STOP(i)              *((volatile unsigned long*)(i + 0x28))
#define MT6575_ROT_DMA_EN(i)                  *((volatile unsigned long*)(i + 0x30))
#define MT6575_ROT_DMA_RESET(i)    *((volatile unsigned long*)(i + 0x38))
#define MT6575_ROT_DMA_LOCK(i)      *((volatile unsigned long*)(i + 0x3C))
#define MT6575_ROT_DMA_QUEUE_RSTA(i)            *((volatile unsigned long*)(i + 0x40))
#define MT6575_ROT_DMA_RD_BASE(i)               *((volatile unsigned long*)(i + 0x48))
#define MT6575_ROT_DMA_RPT_ADVANCE(i)       *((volatile unsigned long*)(i + 0x50))
#define MT6575_ROT_DMA_QUEUE_WSTA(i)        *((volatile unsigned long*)(i + 0x58))
#define MT6575_ROT_DMA_WR_BASE(i)               *((volatile unsigned long*)(i + 0x60))
#define MT6575_ROT_DMA_WPT_ADVANCE(i)       *((volatile unsigned long*)(i + 0x68))
#define MT6575_ROT_DMA_QUEUE_DATA(i)         *((volatile unsigned long*)(i + 0x70))
#define MT6575_ROT_DMA_QUEUE_BASE(i)          *((volatile unsigned long*)(i + 0x78))
#define MT6575_ROT_DMA_EXEC_CNT(i)              *((volatile unsigned long*)(i + 0x80))
#define MT6575_ROT_DMA_DROPPED_FRAME_CNT(i)     *((volatile unsigned long*)(i + 0x88))
#define MT6575_ROT_DMA_LCD(i)                   *((volatile unsigned long*)(i + 0x200))
#define MT6575_ROT_DMA_LCD_STA(i)           *((volatile unsigned long*)(i + 0x208))
#define MT6575_ROT_DMA_LCD_RPT_ADVANCE(i)       *((volatile unsigned long*)(i + 0x210))
#define MT6575_ROT_DMA_SLOW_DOWN(i)                 *((volatile unsigned long*)(i + 0x300))
#define MT6575_ROT_DMA_BUF_ADDR0(i)             *((volatile unsigned long*)(i + 0x308))
#define MT6575_ROT_DMA_BUF_ADDR1(i)             *((volatile unsigned long*)(i + 0x310))
#define MT6575_ROT_DMA_BUF_ADDR2(i)             *((volatile unsigned long*)(i + 0x380))
#define MT6575_ROT_DMA_BUF_ADDR3(i)             *((volatile unsigned long*)(i + 0x388))
#define MT6575_ROT_DMA_LCD_STR_ADDR(i)        *((volatile unsigned long*)(i + 0x314)) // #descriptor
#define MT6575_ROT_DMA_Y_DST_STR_ADDR(i)        *((volatile unsigned long*)(i + 0x318))// #descriptor
#define MT6575_ROT_DMA_U_DST_STR_ADDR(i)        *((volatile unsigned long*)(i + 0x320))// #descriptor
#define MT6575_ROT_DMA_V_DST_STR_ADDR(i)        *((volatile unsigned long*)(i + 0x328))// #descriptor
#define MT6575_ROT_DMA_SRC_SIZE(i)              *((volatile unsigned long*)(i + 0x330)) // #descriptor
#define MT6575_ROT_DMA_CLIP_SIZE(i)             *((volatile unsigned long*)(i + 0x338)) //#descriptor
#define MT6575_ROT_DMA_CLIP_OFFSET(i)        *((volatile unsigned long*)(i + 0x340)) // #descriptor
#define MT6575_ROT_DMA_DST_SIZE(i)              *((volatile unsigned long*)(i + 0x348)) //#descriptor
#define MT6575_ROT_DMA_CLIP_W_IN_BYTE(i)  *((volatile unsigned long*)(i + 0x350)) //#descriptor
#define MT6575_ROT_DMA_CLIP_H_IN_BYTE(i)   *((volatile unsigned long*)(i + 0x358)) //#descriptor
#define MT6575_ROT_DMA_CON(i)                   *((volatile unsigned long*)(i + 0x368)) // #descriptor
#define MT6575_ROT_DMA_PERF(i)                  *((volatile unsigned long*)(i + 0x36c)) // #descriptor
#define MT6575_ROT_DMA_MAIN_BUFF_SIZE(i)        *((volatile unsigned long*)(i + 0x370)) // #descriptor
#define MT6575_ROT_DMA_SUB_BUFF_SIZE(i)         *((volatile unsigned long*)(i + 0x374)) // #descriptor
#define MT6575_ROT_DMA_DITHER(i)                *((volatile unsigned long*)(i + 0x378))
#define MT6575_ROT_DMA_DBG_ST50(i)                *((volatile unsigned long*)(i + 0x400))
#define MT6575_ROT_DMA_DBG_ST56(i)                *((volatile unsigned long*)(i + 0x418))
#define MT6575_ROT_DMA_DBG_ST0(i)                *((volatile unsigned long*)(i + 0x430))



#define MT6575_RGB_ROT0_REG_RANGE           0x434
#define MT6575_RGB_ROT1_REG_RANGE           0x434
#define MT6575_RGB_ROT2_REG_RANGE           0x434
#define MT6575_VDO_ROT0_REG_RANGE           0x434
#define MT6575_VDO_ROT1_REG_RANGE           0x434
#define MT6575_TV_ROT_REG_RANGE             0x434
#define MT6575_LCD_REG_RANGE                0x434



//JPGDMA

#define MT6575_JPEG_DMA_BASE      0xC208E000
#define MT6575_JPEG_DMA_STOP(i)            *((volatile unsigned long*)i)
#define MT6575_JPEG_DMA_EN(i)                *((volatile unsigned long*)(i + 0x4))
#define MT6575_JPEG_DMA_RESET(i)          *((volatile unsigned long*)(i + 0x8))
#define MT6575_JPEG_DMA_CON(i)             *((volatile unsigned long*)(i + 0xc))
#define MT6575_JPEG_DMA_BUF_BASE_ADDR0(i)  *((volatile unsigned long*)(i + 0x10))
#define MT6575_JPEG_DMA_BUF_BASE_ADDR1(i)  *((volatile unsigned long*)(i + 0x14))
#define MT6575_JPEG_DMA_SIZE(i)             *((volatile unsigned long*)(i + 0x18))
#define MT6575_JPEG_DMA_INTERRUPT(i)             *((volatile unsigned long*)(i + 0x20))
#define MT6575_JPEG_DMA_DEBUG_STATUS0(i)    *((volatile unsigned long*)(i + 0x90))
#define MT6575_JPEG_DMA_DEBUG_STATUS1(i)    *((volatile unsigned long*)(i + 0x94))
#define MT6575_JPEG_DMA_DEBUG_STATUS2(i)    *((volatile unsigned long*)(i + 0x98))
#define MT6575_JPEG_DMA_DEBUG_STATUS3(i)    *((volatile unsigned long*)(i + 0x9c))
#define MT6575_JPEG_DMA_DEBUG_STATUS4(i)    *((volatile unsigned long*)(i + 0xa0))
#define MT6575_JPEG_DMA_DEBUG_STATUS5(i)    *((volatile unsigned long*)(i + 0xa4))
#define MT6575_JPEG_DMA_DEBUG_STATUS6(i)    *((volatile unsigned long*)(i + 0xa8))
#define MT6575_JPEG_DMA_DEBUG_STATUS7(i)    *((volatile unsigned long*)(i + 0xac))
#define MT6575_JPEG_DMA_REG_RANGE      0xB0



/*#############################################################################*/

#define MT6575_sysram_base 0xC2000000 //256k



#define MT6575_BANK0_SIZE 0x8000
#define MT6575_BANK0_ADDR (MT6575_sysram_base)//0xC2000000
#define MT6575_BANK1_SIZE 0x8000
#define MT6575_BANK1_ADDR (MT6575_BANK0_ADDR + MT6575_BANK0_SIZE)//0xC2008000
#define MT6575_BANK2_SIZE 0x8000
#define MT6575_BANK2_ADDR (MT6575_BANK1_ADDR + MT6575_BANK1_SIZE)//0xC2010000
#define MT6575_BANK3_SIZE 0x8000
#define MT6575_BANK3_ADDR (MT6575_BANK2_ADDR + MT6575_BANK2_SIZE)//0xC2018000
#define MT6575_BANK4_SIZE 0x8000
#define MT6575_BANK4_ADDR (MT6575_BANK3_ADDR + MT6575_BANK3_SIZE)//0xC2020000
#define MT6575_BANK5_SIZE 0x8000
#define MT6575_BANK5_ADDR (MT6575_BANK4_ADDR + MT6575_BANK4_SIZE)//0xC2028000
#define MT6575_BANK6_SIZE 0x8000
#define MT6575_BANK6_ADDR (MT6575_BANK5_ADDR + MT6575_BANK5_SIZE)//0xC2030000
#define MT6575_BANK7_SIZE 0x8000
#define MT6575_BANK7_ADDR (MT6575_BANK6_ADDR + MT6575_BANK5_SIZE)//0xC2038000~C2040000


/****************************************************************************************************
2kByte of Bank3 is reserved for ROTDMA/RDMA/OVL/TV_ROT descriptor for 128kByte platform
RGB_ROT0 : 0xC201F800 ~ ..+0xC0
RGB_ROT1 : 0xC201F8C0 ~ ..+0xC0
RGB_ROT2 : 0xC201F980 ~ ..+0xC0
VDO_ROT0 : 0xC201FA40 ~ ..+0xC0
VDO_ROT1 : 0xC201FB00 ~ ..+0xC0
RDMA0 : 0xC201FBC0~ +0xC0
RDMA1 : 0xC201FC80~ +0xC0
OVL : 0xC201FD40~ +0x200
TV_ROT : 0xC201FF40~ +0xC0

or

2kByte of Bank7 is reserved for ROTDMA/RDMA/OVL/TV_ROT descriptor for 256kByte platform
RGB_ROT0 : 0xC203F800 ~ ..+0xC0
RGB_ROT1 : 0xC203F8C0 ~ ..+0xC0
RGB_ROT2 : 0xC203F980 ~ ..+0xC0
VDO_ROT0 : 0xC203FA40 ~ ..+0xC0
VDO_ROT1 : 0xC203FB00 ~ ..+0xC0
RDMA0 : 0xC203FBC0 ~ +0xC0
RDMA1 : 0xC203FC80 ~ +0xC0
OVL : 0xC203FD40 ~ +0x200
TV_ROT : 0xC203FF40 ~ +0xC0
*****************************************************************************************************/
#define MT6575_DESCRIPTOR_BASE (MT6575_BANK2_ADDR + 0x7800)
#define MT6575_RGB_ROT0_DESCRIPT_BASE MT6575_DESCRIPTOR_BASE//0xC201F800
#define MT6575_RGB_ROT1_DESCRIPT_BASE (MT6575_RGB_ROT0_DESCRIPT_BASE + 0xC0)//0xC201F8C0
#define MT6575_RGB_ROT2_DESCRIPT_BASE (MT6575_RGB_ROT1_DESCRIPT_BASE + 0xC0)//0xC201F980
#define MT6575_VDO_ROT0_DESCRIPT_BASE (MT6575_RGB_ROT2_DESCRIPT_BASE + 0xC0)//0xC201FA40
#define MT6575_VDO_ROT1_DESCRIPT_BASE (MT6575_VDO_ROT0_DESCRIPT_BASE + 0xC0)//0xC201FB00
#define MT6575_RDMA0_DESCRIPT_BASE (MT6575_VDO_ROT1_DESCRIPT_BASE + 0xC0)//0xC201FBC0
#define MT6575_RDMA1_DESCRIPT_BASE (MT6575_RDMA0_DESCRIPT_BASE + 0xC0)//0xC201FC80
#define MT6575_OVL_DESCRIPT_BASE (MT6575_RDMA1_DESCRIPT_BASE + 0xC0)//0xC201FD40
#define MT6575_TV_ROT_DESCRIPT_BASE (MT6575_OVL_DESCRIPT_BASE + 0x200)//0xC201FF40



#endif
