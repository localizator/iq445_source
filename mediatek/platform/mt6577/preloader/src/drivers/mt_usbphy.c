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

#include "platform.h"
#include "mt_usbd.h"

#if CFG_FPGA_PLATFORM
#include "mt_i2c.h"
#endif

#define USB20_PHY_BASE	(USBSIF_BASE + 0x0800)
#define USB11_PHY_BASE	(USBSIF_BASE + 0x0900)

#if CFG_FPGA_PLATFORM
U32 usb_i2c_read8 (U8 addr, U8 *dataBuffer)
{
    U32 ret_code = I2C_OK;
    U8 write_data = addr;

    ret_code = i2c_v1_write(0xc0, &write_data, 1);    // set register command

    if (ret_code != I2C_OK)
        return ret_code;

    ret_code = i2c_v1_read(0xc0, dataBuffer, 1);

    return ret_code;
}

U32 usb_i2c_write8 (U8 addr, U8 value)
{
    U32 ret_code = I2C_OK;
    U8 write_data[2];

    write_data[0]= addr;
    write_data[1] = value;
 
    ret_code = i2c_v1_write(0xc0, write_data, 2);
    
    return ret_code;
}

#define USBPHY_I2C_READ8(addr, buffer)     usb_i2c_read8(addr, buffer)
#define USBPHY_I2C_WRITE8(addr, value)     usb_i2c_write8(addr, value)
#endif

#define USBPHY_READ8(offset)          __raw_readb(USB20_PHY_BASE+offset)
#define USBPHY_WRITE8(offset, value)  __raw_writeb(value, USB20_PHY_BASE+offset)
#define USBPHY_SET8(offset, mask)     USBPHY_WRITE8(offset, USBPHY_READ8(offset) | mask)
#define USBPHY_CLR8(offset, mask)     USBPHY_WRITE8(offset, USBPHY_READ8(offset) & ~mask)

#define USB11PHY_READ8(offset)          __raw_readb(USB11_PHY_BASE+offset)
#define USB11PHY_WRITE8(offset, value)  __raw_writeb(value, USB11_PHY_BASE+offset)
#define USB11PHY_SET8(offset, mask)     USB11PHY_WRITE8(offset, USB11PHY_READ8(offset) | mask)
#define USB11PHY_CLR8(offset, mask)     USB11PHY_WRITE8(offset, USB11PHY_READ8(offset) & ~mask)

#if CFG_FPGA_PLATFORM
void mt_usb_phy_poweron (void)
{
    #define PHY_DRIVING   0x3

    UINT8 usbreg8;
    unsigned int i;

    /* force_suspendm = 0 */
    USBPHY_CLR8(0x6a, 0x04);

    USBPHY_I2C_WRITE8(0xff, 0x00);
    USBPHY_I2C_WRITE8(0x61, 0x04);
    USBPHY_I2C_WRITE8(0x68, 0x00);
    USBPHY_I2C_WRITE8(0x6a, 0x00);
    USBPHY_I2C_WRITE8(0x00, 0x6e);
    USBPHY_I2C_WRITE8(0x1b, 0x0c);
    USBPHY_I2C_WRITE8(0x08, 0x44);
    USBPHY_I2C_WRITE8(0x11, 0x55);
    USBPHY_I2C_WRITE8(0x1a, 0x68);

    #if defined(USB_PHY_DRIVING_TUNING)
    /* driving tuning */
    USBPHY_I2C_READ8(0xab, &usbreg8);
    usbreg8 &= ~0x3;
    usbreg8 |= PHY_DRIVING;
    USBPHY_I2C_WRITE8(0xab, usbreg8);
        
    for(i = 0; i < 16; i++)
    {
        USBPHY_I2C_READ8((0x92+i), &usbreg8)
        usbreg8 &= ~0x3;
        usbreg8 |= PHY_DRIVING;
        USBPHY_I2C_WRITE8((0x92+i), usbreg8);    
    }

    USBPHY_I2C_READ8(0xbc, &usbreg8);
    usbreg8 &= ~0x3;
    usbreg8 |= PHY_DRIVING;
    USBPHY_I2C_WRITE8(0xbc, usbreg8);
    
    USBPHY_I2C_READ8(0xbe, &usbreg8);
    usbreg8 &= ~0x3;
    usbreg8 |= PHY_DRIVING;
    USBPHY_I2C_WRITE8(0xbe, usbreg8);

    USBPHY_I2C_READ8(0xbf, &usbreg8);
    usbreg8 &= ~0x3;
    usbreg8 |= PHY_DRIVING;
    USBPHY_I2C_WRITE8(0xbf, usbreg8);

    USBPHY_I2C_READ8(0xcd, &usbreg8);
    usbreg8 &= ~0x3;
    usbreg8 |= PHY_DRIVING;
    USBPHY_I2C_WRITE8(0xcd, usbreg8);       

    USBPHY_I2C_READ8(0xf1, &usbreg8);
    usbreg8 &= ~0x3;
    usbreg8 |= PHY_DRIVING;
    USBPHY_I2C_WRITE8(0xf1, usbreg8);

    USBPHY_I2C_READ8(0xa7, &usbreg8);
    usbreg8 &= ~0x3;
    usbreg8 |= PHY_DRIVING;
    USBPHY_I2C_WRITE8(0xa7, usbreg8);
       
    USBPHY_I2C_READ8(0xa8, &usbreg8);
    usbreg8 &= ~0x3;
    usbreg8 |= PHY_DRIVING;
    USBPHY_I2C_WRITE8(0xa8, usbreg8);
    #endif  

    udelay(800);

}
void mt_usb_phy_savecurrent (void)
{
    /* no need */
}
void mt_usb_phy_recover (void)
{
    /* no need */
}
void mt_usb11_phy_savecurrent(void)
{
    /* no need */
}
#else
void mt_usb_phy_poweron (void)
{
    USBPHY_CLR8(0x6b, 0x04);
    USBPHY_CLR8(0x6e, 0x01);
    USBPHY_CLR8(0x1c, 0x80);
    USBPHY_CLR8(0x02, 0x7f);
    USBPHY_SET8(0x02, 0x09);
    USBPHY_CLR8(0X22, 0x03);
    USBPHY_CLR8(0x6a, 0x04);
    USBPHY_SET8(0x1b, 0x08);

    udelay(800);
 
    return;
}

void mt_usb_phy_savecurrent (void)
{
    USBPHY_CLR8(0x6b, 0x04);
    USBPHY_CLR8(0x6e, 0x01);
    USBPHY_CLR8(0x6a, 0x04);
    USBPHY_SET8(0x68, 0xc0);
    USBPHY_CLR8(0x68, 0x30);
    USBPHY_SET8(0x68, 0x10);
    USBPHY_SET8(0x68, 0x04);
    USBPHY_CLR8(0x69, 0x3c);
    USBPHY_SET8(0x6a, 0xba);
    USBPHY_CLR8(0x1c, 0x80);
    USBPHY_CLR8(0x1b, 0x08);
      
    udelay(800);

    USBPHY_SET8(0x63, 0x02);

    udelay(1);

    USBPHY_SET8(0x6a, 0x04);
  
    udelay(1);

    return;
}

void mt_usb_phy_recover (void)
{
    USBPHY_CLR8(0x6b, 0x04);
    USBPHY_CLR8(0x6e, 0x01);
    USBPHY_CLR8(0x6a, 0x04);
    USBPHY_CLR8(0x68, 0xf4);
    USBPHY_CLR8(0x69, 0x3c);
    USBPHY_CLR8(0x6a, 0xba);
    USBPHY_CLR8(0x1c, 0x80);
    USBPHY_SET8(0x1b, 0x08);

    udelay(800);

    return;
}

void mt_usb11_phy_savecurrent(void)
{
    USB11PHY_SET8(0xca, 0x10);
    USB11PHY_SET8(0xcb, 0x3c);
    USB11PHY_CLR8(0xc1, 0x08);
    USB11PHY_CLR8(0xc7, 0x06);
    USB11PHY_SET8(0xc6, 0x06);

    return;
}
#endif

