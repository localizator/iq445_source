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


#ifndef _MTK_DEVICE_APC_H
#define _MTK_DEVICE_APC_H

#include "typedefs.h"

#define AP_DEVAPC0_BASE             0xC101C000
#define AP_DEVAPC1_BASE             0xC101D000
#define MM1_DEVAPC0_BASE            0xC101E000
#define MM1_DEVAPC1_BASE            0xC101F000
#define MD_DEVAPC0_BASE             0xD01A0000
#define MD_DEVAPC1_BASE             0xD1160000
#define DSP_DEVAPC0_BASE            0xD9070000

/*******************************************************************************
 * REGISTER ADDRESS DEFINATION
 ******************************************************************************/
#define DEVAPC0_D0_APC_0		    ((volatile unsigned int*)(DEVAPC0_BASE+0x0000))
#define DEVAPC0_D0_APC_1            ((volatile unsigned int*)(DEVAPC0_BASE+0x0004))
#define DEVAPC0_D1_APC_0            ((volatile unsigned int*)(DEVAPC0_BASE+0x0008))
#define DEVAPC0_D1_APC_1            ((volatile unsigned int*)(DEVAPC0_BASE+0x000C))
#define DEVAPC0_D2_APC_0            ((volatile unsigned int*)(DEVAPC0_BASE+0x0010))
#define DEVAPC0_D2_APC_1            ((volatile unsigned int*)(DEVAPC0_BASE+0x0014))
#define DEVAPC0_D3_APC_0            ((volatile unsigned int*)(DEVAPC0_BASE+0x0018))
#define DEVAPC0_D3_APC_1            ((volatile unsigned int*)(DEVAPC0_BASE+0x001C))
#define DEVAPC0_D0_VIO_MASK         ((volatile unsigned int*)(DEVAPC0_BASE+0x0020))
#define DEVAPC0_D1_VIO_MASK         ((volatile unsigned int*)(DEVAPC0_BASE+0x0024))
#define DEVAPC0_D2_VIO_MASK         ((volatile unsigned int*)(DEVAPC0_BASE+0x0028))
#define DEVAPC0_D3_VIO_MASK         ((volatile unsigned int*)(DEVAPC0_BASE+0x002C))
#define DEVAPC0_D0_VIO_STA          ((volatile unsigned int*)(DEVAPC0_BASE+0x0030))
#define DEVAPC0_D1_VIO_STA          ((volatile unsigned int*)(DEVAPC0_BASE+0x0034))
#define DEVAPC0_D2_VIO_STA          ((volatile unsigned int*)(DEVAPC0_BASE+0x0038))
#define DEVAPC0_D3_VIO_STA          ((volatile unsigned int*)(DEVAPC0_BASE+0x003C))
#define DEVAPC0_VIO_DBG0            ((volatile unsigned int*)(DEVAPC0_BASE+0x0040))
#define DEVAPC0_VIO_DBG1            ((volatile unsigned int*)(DEVAPC0_BASE+0x0044))
#define DEVAPC0_DXS_VIO_MASK        ((volatile unsigned int*)(DEVAPC0_BASE+0x0080))
#define DEVAPC0_DXS_VIO_STA         ((volatile unsigned int*)(DEVAPC0_BASE+0x0084))
#define DEVAPC0_APC_CON             ((volatile unsigned int*)(DEVAPC0_BASE+0x0090))
#define DEVAPC0_APC_LOCK            ((volatile unsigned int*)(DEVAPC0_BASE+0x0094))
                                                                      
#define DEVAPC1_D0_APC_0		    ((volatile unsigned int*)(DEVAPC1_BASE+0x0000))
#define DEVAPC1_D0_APC_1            ((volatile unsigned int*)(DEVAPC1_BASE+0x0004))
#define DEVAPC1_D1_APC_0            ((volatile unsigned int*)(DEVAPC1_BASE+0x0008))
#define DEVAPC1_D1_APC_1            ((volatile unsigned int*)(DEVAPC1_BASE+0x000C))
#define DEVAPC1_D2_APC_0            ((volatile unsigned int*)(DEVAPC1_BASE+0x0010))
#define DEVAPC1_D2_APC_1            ((volatile unsigned int*)(DEVAPC1_BASE+0x0014))
#define DEVAPC1_D3_APC_0            ((volatile unsigned int*)(DEVAPC1_BASE+0x0018))
#define DEVAPC1_D3_APC_1            ((volatile unsigned int*)(DEVAPC1_BASE+0x001C))
#define DEVAPC1_D0_VIO_MASK         ((volatile unsigned int*)(DEVAPC1_BASE+0x0020))
#define DEVAPC1_D1_VIO_MASK         ((volatile unsigned int*)(DEVAPC1_BASE+0x0024))
#define DEVAPC1_D2_VIO_MASK         ((volatile unsigned int*)(DEVAPC1_BASE+0x0028))
#define DEVAPC1_D3_VIO_MASK         ((volatile unsigned int*)(DEVAPC1_BASE+0x002C))
#define DEVAPC1_D0_VIO_STA          ((volatile unsigned int*)(DEVAPC1_BASE+0x0030))
#define DEVAPC1_D1_VIO_STA          ((volatile unsigned int*)(DEVAPC1_BASE+0x0034))
#define DEVAPC1_D2_VIO_STA          ((volatile unsigned int*)(DEVAPC1_BASE+0x0038))
#define DEVAPC1_D3_VIO_STA          ((volatile unsigned int*)(DEVAPC1_BASE+0x003C))
#define DEVAPC1_VIO_DBG0            ((volatile unsigned int*)(DEVAPC1_BASE+0x0040))
#define DEVAPC1_VIO_DBG1            ((volatile unsigned int*)(DEVAPC1_BASE+0x0044))
#define DEVAPC1_APC_CON             ((volatile unsigned int*)(DEVAPC1_BASE+0x0090))
#define DEVAPC1_APC_LOCK            ((volatile unsigned int*)(DEVAPC1_BASE+0x0094))
#define DEVAPC1_MAS_DOM             ((volatile unsigned int*)(DEVAPC1_BASE+0x00A0))
#define DEVAPC1_MAS_SEC             ((volatile unsigned int*)(DEVAPC1_BASE+0x00A4))
#define DEVAPC1_DEC_ERR_CON         ((volatile unsigned int*)(DEVAPC1_BASE+0x00B4))
#define DEVAPC1_DEC_ERR_ADDR        ((volatile unsigned int*)(DEVAPC1_BASE+0x00B8))
#define DEVAPC1_DEC_ERR_ID          ((volatile unsigned int*)(DEVAPC1_BASE+0x00BC))

#define MM1_DEVAPC0_D0_APC_0 	    ((volatile unsigned int*)(MM1_DEVAPC0_BASE+0x0000))
#define MM1_DEVAPC0_D0_APC_1        ((volatile unsigned int*)(MM1_DEVAPC0_BASE+0x0004))
#define MM1_DEVAPC0_D1_APC_0        ((volatile unsigned int*)(MM1_DEVAPC0_BASE+0x0008))
#define MM1_DEVAPC0_D1_APC_1        ((volatile unsigned int*)(MM1_DEVAPC0_BASE+0x000C))
#define MM1_DEVAPC0_D2_APC_0        ((volatile unsigned int*)(MM1_DEVAPC0_BASE+0x0010))
#define MM1_DEVAPC0_D2_APC_1        ((volatile unsigned int*)(MM1_DEVAPC0_BASE+0x0014))
#define MM1_DEVAPC0_D3_APC_0        ((volatile unsigned int*)(MM1_DEVAPC0_BASE+0x0018))
#define MM1_DEVAPC0_D3_APC_1        ((volatile unsigned int*)(MM1_DEVAPC0_BASE+0x001C))
#define MM1_DEVAPC0_D0_VIO_MASK     ((volatile unsigned int*)(MM1_DEVAPC0_BASE+0x0020))
#define MM1_DEVAPC0_D1_VIO_MASK     ((volatile unsigned int*)(MM1_DEVAPC0_BASE+0x0024))
#define MM1_DEVAPC0_D2_VIO_MASK     ((volatile unsigned int*)(MM1_DEVAPC0_BASE+0x0028))
#define MM1_DEVAPC0_D3_VIO_MASK     ((volatile unsigned int*)(MM1_DEVAPC0_BASE+0x002C))
#define MM1_DEVAPC0_D0_VIO_STA      ((volatile unsigned int*)(MM1_DEVAPC0_BASE+0x0030))
#define MM1_DEVAPC0_D1_VIO_STA      ((volatile unsigned int*)(MM1_DEVAPC0_BASE+0x0034))
#define MM1_DEVAPC0_D2_VIO_STA      ((volatile unsigned int*)(MM1_DEVAPC0_BASE+0x0038))
#define MM1_DEVAPC0_D3_VIO_STA      ((volatile unsigned int*)(MM1_DEVAPC0_BASE+0x003C))
#define MM1_DEVAPC0_VIO_DBG0        ((volatile unsigned int*)(MM1_DEVAPC0_BASE+0x0040))
#define MM1_DEVAPC0_VIO_DBG1        ((volatile unsigned int*)(MM1_DEVAPC0_BASE+0x0044))
#define MM1_DEVAPC0_APC_CON         ((volatile unsigned int*)(MM1_DEVAPC0_BASE+0x0090))
#define MM1_DEVAPC0_APC_LOCK        ((volatile unsigned int*)(MM1_DEVAPC0_BASE+0x0094))

#define MM1_DEVAPC1_D0_APC_0 	    ((volatile unsigned int*)(MM1_DEVAPC1_BASE+0x0000))
#define MM1_DEVAPC1_D0_APC_1        ((volatile unsigned int*)(MM1_DEVAPC1_BASE+0x0004))
#define MM1_DEVAPC1_D1_APC_0        ((volatile unsigned int*)(MM1_DEVAPC1_BASE+0x0008))
#define MM1_DEVAPC1_D1_APC_1        ((volatile unsigned int*)(MM1_DEVAPC1_BASE+0x000C))
#define MM1_DEVAPC1_D2_APC_0        ((volatile unsigned int*)(MM1_DEVAPC1_BASE+0x0010))
#define MM1_DEVAPC1_D2_APC_1        ((volatile unsigned int*)(MM1_DEVAPC1_BASE+0x0014))
#define MM1_DEVAPC1_D3_APC_0        ((volatile unsigned int*)(MM1_DEVAPC1_BASE+0x0018))
#define MM1_DEVAPC1_D3_APC_1        ((volatile unsigned int*)(MM1_DEVAPC1_BASE+0x001C))
#define MM1_DEVAPC1_D0_VIO_MASK     ((volatile unsigned int*)(MM1_DEVAPC1_BASE+0x0020))
#define MM1_DEVAPC1_D1_VIO_MASK     ((volatile unsigned int*)(MM1_DEVAPC1_BASE+0x0024))
#define MM1_DEVAPC1_D2_VIO_MASK     ((volatile unsigned int*)(MM1_DEVAPC1_BASE+0x0028))
#define MM1_DEVAPC1_D3_VIO_MASK     ((volatile unsigned int*)(MM1_DEVAPC1_BASE+0x002C))
#define MM1_DEVAPC1_D0_VIO_STA      ((volatile unsigned int*)(MM1_DEVAPC1_BASE+0x0030))
#define MM1_DEVAPC1_D1_VIO_STA      ((volatile unsigned int*)(MM1_DEVAPC1_BASE+0x0034))
#define MM1_DEVAPC1_D2_VIO_STA      ((volatile unsigned int*)(MM1_DEVAPC1_BASE+0x0038))
#define MM1_DEVAPC1_D3_VIO_STA      ((volatile unsigned int*)(MM1_DEVAPC1_BASE+0x003C))
#define MM1_DEVAPC1_VIO_DBG0        ((volatile unsigned int*)(MM1_DEVAPC1_BASE+0x0040))
#define MM1_DEVAPC1_VIO_DBG1        ((volatile unsigned int*)(MM1_DEVAPC1_BASE+0x0044))
#define MM1_DEVAPC1_APC_CON         ((volatile unsigned int*)(MM1_DEVAPC1_BASE+0x0090))
#define MM1_DEVAPC1_APC_LOCK        ((volatile unsigned int*)(MM1_DEVAPC1_BASE+0x0094))
#define MM1_DEVAPC1_DEC_ERR_CON     ((volatile unsigned int*)(MM1_DEVAPC1_BASE+0x00B4))
#define MM1_DEVAPC1_DEC_ERR_ADDR    ((volatile unsigned int*)(MM1_DEVAPC1_BASE+0x00B8))
#define MM1_DEVAPC1_DEC_ERR_ID      ((volatile unsigned int*)(MM1_DEVAPC1_BASE+0x00BC))

#define MD_DEVAPC0_D0_APC_0 	   ((volatile unsigned int*)(MD_DEVAPC0_BASE+0x0000))
#define MD_DEVAPC0_D0_APC_1        ((volatile unsigned int*)(MD_DEVAPC0_BASE+0x0004))
#define MD_DEVAPC0_D1_APC_0        ((volatile unsigned int*)(MD_DEVAPC0_BASE+0x0008))
#define MD_DEVAPC0_D1_APC_1        ((volatile unsigned int*)(MD_DEVAPC0_BASE+0x000C))
#define MD_DEVAPC0_D2_APC_0        ((volatile unsigned int*)(MD_DEVAPC0_BASE+0x0010))
#define MD_DEVAPC0_D2_APC_1        ((volatile unsigned int*)(MD_DEVAPC0_BASE+0x0014))
#define MD_DEVAPC0_D3_APC_0        ((volatile unsigned int*)(MD_DEVAPC0_BASE+0x0018))
#define MD_DEVAPC0_D3_APC_1        ((volatile unsigned int*)(MD_DEVAPC0_BASE+0x001C))
#define MD_DEVAPC0_D0_VIO_MASK     ((volatile unsigned int*)(MD_DEVAPC0_BASE+0x0020))
#define MD_DEVAPC0_D1_VIO_MASK     ((volatile unsigned int*)(MD_DEVAPC0_BASE+0x0024))
#define MD_DEVAPC0_D2_VIO_MASK     ((volatile unsigned int*)(MD_DEVAPC0_BASE+0x0028))
#define MD_DEVAPC0_D3_VIO_MASK     ((volatile unsigned int*)(MD_DEVAPC0_BASE+0x002C))
#define MD_DEVAPC0_D0_VIO_STA      ((volatile unsigned int*)(MD_DEVAPC0_BASE+0x0030))
#define MD_DEVAPC0_D1_VIO_STA      ((volatile unsigned int*)(MD_DEVAPC0_BASE+0x0034))
#define MD_DEVAPC0_D2_VIO_STA      ((volatile unsigned int*)(MD_DEVAPC0_BASE+0x0038))
#define MD_DEVAPC0_D3_VIO_STA      ((volatile unsigned int*)(MD_DEVAPC0_BASE+0x003C))
#define MD_DEVAPC0_VIO_DBG0        ((volatile unsigned int*)(MD_DEVAPC0_BASE+0x0040))
#define MD_DEVAPC0_VIO_DBG1        ((volatile unsigned int*)(MD_DEVAPC0_BASE+0x0044))
#define MD_DEVAPC0_DXS_VIO_MASK    ((volatile unsigned int*)(MD_DEVAPC0_BASE+0x0080))
#define MD_DEVAPC0_DXS_VIO_STA     ((volatile unsigned int*)(MD_DEVAPC0_BASE+0x0084))
#define MD_DEVAPC0_APC_CON         ((volatile unsigned int*)(MD_DEVAPC0_BASE+0x0090))
#define MD_DEVAPC0_APC_LOCK        ((volatile unsigned int*)(MD_DEVAPC0_BASE+0x0094))
#define MD_DEVAPC0_MAS_DOM         ((volatile unsigned int*)(MD_DEVAPC0_BASE+0x00A0))
#define MD_DEVAPC0_MAS_SEC         ((volatile unsigned int*)(MD_DEVAPC0_BASE+0x00A4))

#define MD_DEVAPC1_D0_APC_0 	   ((volatile unsigned int*)(MD_DEVAPC1_BASE+0x0000))
#define MD_DEVAPC1_D0_APC_1        ((volatile unsigned int*)(MD_DEVAPC1_BASE+0x0004))
#define MD_DEVAPC1_D1_APC_0        ((volatile unsigned int*)(MD_DEVAPC1_BASE+0x0008))
#define MD_DEVAPC1_D1_APC_1        ((volatile unsigned int*)(MD_DEVAPC1_BASE+0x000C))
#define MD_DEVAPC1_D2_APC_0        ((volatile unsigned int*)(MD_DEVAPC1_BASE+0x0010))
#define MD_DEVAPC1_D2_APC_1        ((volatile unsigned int*)(MD_DEVAPC1_BASE+0x0014))
#define MD_DEVAPC1_D3_APC_0        ((volatile unsigned int*)(MD_DEVAPC1_BASE+0x0018))
#define MD_DEVAPC1_D3_APC_1        ((volatile unsigned int*)(MD_DEVAPC1_BASE+0x001C))
#define MD_DEVAPC1_D0_VIO_MASK     ((volatile unsigned int*)(MD_DEVAPC1_BASE+0x0020))
#define MD_DEVAPC1_D1_VIO_MASK     ((volatile unsigned int*)(MD_DEVAPC1_BASE+0x0024))
#define MD_DEVAPC1_D2_VIO_MASK     ((volatile unsigned int*)(MD_DEVAPC1_BASE+0x0028))
#define MD_DEVAPC1_D3_VIO_MASK     ((volatile unsigned int*)(MD_DEVAPC1_BASE+0x002C))
#define MD_DEVAPC1_D0_VIO_STA      ((volatile unsigned int*)(MD_DEVAPC1_BASE+0x0030))
#define MD_DEVAPC1_D1_VIO_STA      ((volatile unsigned int*)(MD_DEVAPC1_BASE+0x0034))
#define MD_DEVAPC1_D2_VIO_STA      ((volatile unsigned int*)(MD_DEVAPC1_BASE+0x0038))
#define MD_DEVAPC1_D3_VIO_STA      ((volatile unsigned int*)(MD_DEVAPC1_BASE+0x003C))
#define MD_DEVAPC1_VIO_DBG0        ((volatile unsigned int*)(MD_DEVAPC1_BASE+0x0040))
#define MD_DEVAPC1_VIO_DBG1        ((volatile unsigned int*)(MD_DEVAPC1_BASE+0x0044))
#define MD_DEVAPC1_APC_CON         ((volatile unsigned int*)(MD_DEVAPC1_BASE+0x0090))
#define MD_DEVAPC1_APC_LOCK        ((volatile unsigned int*)(MD_DEVAPC1_BASE+0x0094))
#define MD_DEVAPC1_MAS_DOM         ((volatile unsigned int*)(MD_DEVAPC1_BASE+0x00A0))
#define MD_DEVAPC1_MAS_SEC         ((volatile unsigned int*)(MD_DEVAPC1_BASE+0x00A4))
#define MD_DEVAPC1_DEC_ERR_CON     ((volatile unsigned int*)(MD_DEVAPC1_BASE+0x00B4))
#define MD_DEVAPC1_DEC_ERR_ADDR    ((volatile unsigned int*)(MD_DEVAPC1_BASE+0x00B8))
#define MD_DEVAPC1_DEC_ERR_ID      ((volatile unsigned int*)(MD_DEVAPC1_BASE+0x00BC))

#define DSP_DEVAPC0_D0_APC_0 	    ((volatile unsigned int*)(DSP_DEVAPC0_BASE+0x0000))
#define DSP_DEVAPC0_D0_APC_1        ((volatile unsigned int*)(DSP_DEVAPC0_BASE+0x0004))
#define DSP_DEVAPC0_D1_APC_0        ((volatile unsigned int*)(DSP_DEVAPC0_BASE+0x0008))
#define DSP_DEVAPC0_D1_APC_1        ((volatile unsigned int*)(DSP_DEVAPC0_BASE+0x000C))
#define DSP_DEVAPC0_D2_APC_0        ((volatile unsigned int*)(DSP_DEVAPC0_BASE+0x0010))
#define DSP_DEVAPC0_D2_APC_1        ((volatile unsigned int*)(DSP_DEVAPC0_BASE+0x0014))
#define DSP_DEVAPC0_D3_APC_0        ((volatile unsigned int*)(DSP_DEVAPC0_BASE+0x0018))
#define DSP_DEVAPC0_D3_APC_1        ((volatile unsigned int*)(DSP_DEVAPC0_BASE+0x001C))
#define DSP_DEVAPC0_D0_VIO_MASK     ((volatile unsigned int*)(DSP_DEVAPC0_BASE+0x0020))
#define DSP_DEVAPC0_D1_VIO_MASK     ((volatile unsigned int*)(DSP_DEVAPC0_BASE+0x0024))
#define DSP_DEVAPC0_D2_VIO_MASK     ((volatile unsigned int*)(DSP_DEVAPC0_BASE+0x0028))
#define DSP_DEVAPC0_D3_VIO_MASK     ((volatile unsigned int*)(DSP_DEVAPC0_BASE+0x002C))
#define DSP_DEVAPC0_D0_VIO_STA      ((volatile unsigned int*)(DSP_DEVAPC0_BASE+0x0030))
#define DSP_DEVAPC0_D1_VIO_STA      ((volatile unsigned int*)(DSP_DEVAPC0_BASE+0x0034))
#define DSP_DEVAPC0_D2_VIO_STA      ((volatile unsigned int*)(DSP_DEVAPC0_BASE+0x0038))
#define DSP_DEVAPC0_D3_VIO_STA      ((volatile unsigned int*)(DSP_DEVAPC0_BASE+0x003C))
#define DSP_DEVAPC0_VIO_DBG0        ((volatile unsigned int*)(DSP_DEVAPC0_BASE+0x0040))
#define DSP_DEVAPC0_VIO_DBG1        ((volatile unsigned int*)(DSP_DEVAPC0_BASE+0x0044))
#define DSP_DEVAPC0_DXS_VIO_MASK    ((volatile unsigned int*)(DSP_DEVAPC0_BASE+0x0080))
#define DSP_DEVAPC0_DXS_VIO_STA     ((volatile unsigned int*)(DSP_DEVAPC0_BASE+0x0084))
#define DSP_DEVAPC0_APC_CON         ((volatile unsigned int*)(DSP_DEVAPC0_BASE+0x0090))
#define DSP_DEVAPC0_APC_LOCK        ((volatile unsigned int*)(DSP_DEVAPC0_BASE+0x0094))

/* DOMAIN_SETUP */
#define DOMAIN_AP						0
#define DOMAIN_FCORE					1
#define DOMAIN_MD						2

/* Masks for Domain Control for AP DEVAPC1 */
#define APPER_SDXC_DOM_CTRL             (0x3 << 18)   /* R/W */
#define APPER_MSDC0_DOM_CTRL            (0x3 << 16)   /* R/W */
#define APPER_MSDC1_DOM_CTRL            (0x3 << 14)   /* R/W */
#define APPER_MSDC2_DOM_CTRL            (0x3 << 12)   /* R/W */
#define APPER_MSDC3_DOM_CTRL            (0x3 << 10)   /* R/W */
#define APPER_THERMAL_DOM_CTRL          (0x3 << 8)    /* R/W */
#define APPER_PWM_DOM_CTRL              (0x3 << 6)    /* R/W */
#define APPER_USB_DMA1_DOM_CTRL         (0x3 << 4)    /* R/W */
#define APPER_USB_DMA0_DOM_CTRL         (0x3 << 2)    /* R/W */
#define APPER_NFI_DOM_CTRL              (0x3 << 0)    /* R/W */

/* Masks for Domain Control for MD DEVAPC0 */
#define MDPER_PFC_DOM_CTRL              (0x3 << 12)   /* R/W */
#define MDPER_LOGACC_M1_DOM_CTRL        (0x3 << 8)    /* R/W */
#define MDPER_LOGACC_M0_DOM_CTRL        (0x3 << 6)    /* R/W */

/* Masks for Domain Control for MD DEVAPC1 */
#define MD_DBG_DOM_CTRL                 (0x3 << 6)    /* R/W */
#define MD_3G_DOM_CTRL                  (0x3 << 2)    /* R/W */
#define MD_2G_DOM_CTRL                  (0x3 << 0)    /* R/W */

/* Masks for Domain Control for DSP DEVAPC */
#define FCORE_D1_PORT_DOM_CTRL          (0x3 << 4)    /* R/W */
#define FCORE_D0_PORT_DOM_CTRL          (0x3 << 2)    /* R/W */
#define FCORE_I_PORT_DOM_CTRL           (0x3 << 0)    /* R/W */

static inline unsigned int uffs(unsigned int x)
{
    unsigned int r = 1;

    if (!x)
        return 0;
    if (!(x & 0xffff)) {
        x >>= 16;
        r += 16;
    }
    if (!(x & 0xff)) {
        x >>= 8;
        r += 8;
    }
    if (!(x & 0xf)) {
        x >>= 4;
        r += 4;
    }
    if (!(x & 3)) {
        x >>= 2;
        r += 2;
    }
    if (!(x & 1)) {
        x >>= 1;
        r += 1;
    }
    return r;
}

#define reg_read16(reg)          __raw_readw(reg)
#define reg_read32(reg)          __raw_readl(reg)
#define reg_write16(reg,val)     __raw_writew(val,reg)
#define reg_write32(reg,val)     __raw_writel(val,reg)
 
#define reg_set_bits(reg,bs)     ((*(volatile u32*)(reg)) |= (u32)(bs))
#define reg_clr_bits(reg,bs)     ((*(volatile u32*)(reg)) &= ~((u32)(bs)))
 
#define reg_set_field(reg,field,val) \
     do {    \
         volatile unsigned int tv = reg_read32(reg); \
         tv &= ~(field); \
         tv |= ((val) << (uffs((unsigned int)field) - 1)); \
         reg_write32(reg,tv); \
     } while(0)
     
#define reg_get_field(reg,field,val) \
     do {    \
         volatile unsigned int tv = reg_read32(reg); \
         val = ((tv & (field)) >> (uffs((unsigned int)field) - 1)); \
     } while(0)

#endif
