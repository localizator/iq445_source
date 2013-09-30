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

#ifndef PLL_H
#define PLL_H

#define ARMPLL_CON0     ((volatile UINT16 *)0xC0007100)
#define ARMPLL_CON2     ((volatile UINT16 *)0xC0007108)
#define MAINPLL_CON0    ((volatile UINT16 *)0xC0007120)
#define MAINPLL_CON1    ((volatile UINT16 *)0xC0007124)
#define MAINPLL_CON2    ((volatile UINT16 *)0xC0007128)
#define IPLL_CON0       ((volatile UINT16 *)0xC0007140)
#define IPLL_CON1       ((volatile UINT16 *)0xC0007144)
#define IPLL_CON2       ((volatile UINT16 *)0xC0007148)
#define UPLL_CON0       ((volatile UINT16 *)0xC0007160)
#define MDPLL_CON0      ((volatile UINT16 *)0xC0007180)
#define TVDDS_CON2      ((volatile UINT16 *)0xC00071A8)
#define WPLL_CON0       ((volatile UINT16 *)0xC00071C0)
#define AUDPLL_CON0     ((volatile UINT16 *)0xC00071E0)
#define MEMPLL_CON0     ((volatile UINT16 *)0xC0007200)
#define MEMPLL_CON1     ((volatile UINT16 *)0xC0007204)

#define PLL_CON1        ((volatile UINT16 *)0xC0007044)
#define PLL_CON2        ((volatile UINT16 *)0xC0007048)

#define PLL_CON9        ((volatile UINT16 *)0xC0007064)
#define PLL_CON10       ((volatile UINT16 *)0xC0007068)

#define TVDAC_CON0      ((volatile UINT16 *)0xC0007600)

#define TOPCKGEN_CON0   ((volatile UINT16 *)0xC0007B00)
#define TOPCKGEN_CON1   ((volatile UINT16 *)0xC0007B04)
#define TOPCKGEN_CON2   ((volatile UINT16 *)0xC0007B08)
#define TOPCKGEN_CON3   ((volatile UINT16 *)0xC0007B0C)

/* Top Clock Generator Register Definition */
#define TOP_CKMUXSEL    ((volatile UINT32 *)0xC0001000)
#define TOP_CKDIV0      ((volatile UINT32 *)0xC0001004)
#define TOP_CKDIV1      ((volatile UINT32 *)0xC0001008)
#define TOP_CKDIV23     ((volatile UINT32 *)0xC000100C)
#define TOP_DCMCTL      ((volatile UINT32 *)0xC0001010)
#define TOP_MISC        ((volatile UINT32 *)0xC0001014)
#define TOP_CA9DCMFSEL  ((volatile UINT32 *)0xC0001018)
#define TOP_CKCTL       ((volatile UINT32 *)0xC0001020)

#define CLK_CTL0        ((volatile UINT32 *)0xC0009000)

#define PERI_PDN0       ((volatile UINT32 *)0xC1000010)

#define HW_RESV         (0xC1019100)

#define ASYNC_MODE      0x0
#define SYNC_MODE       0x1

#endif
