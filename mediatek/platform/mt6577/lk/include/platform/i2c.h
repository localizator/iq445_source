/*
 * Copyright (c) 2012, Code Aurora Forum. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *  * Neither the name of Google, Inc. nor the names of its contributors
 *    may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef __MT6575_I2C_H__
#define __MT6575_I2C_H__

#include <platform/mt_typedefs.h>

enum{
	I2C0 = 0,
	I2C1 = 1,
	I2C2 = 2
};

#define CHANNEL_BASE	u32 base;\
	if(channel == I2C0) \
		base = 0xC1012000;\
	else if(channel == I2C1)\
		base = 0xC1013000;\
	else \
		base = 0xC1014000;\

//==============================================================================
// I2C Register
//==============================================================================
#define MT_I2C_DATA_PORT                       ((base) + 0x0000)
#define MT_I2C_SLAVE_ADDR                      ((base) + 0x0004)
#define MT_I2C_INTR_MASK                       ((base) + 0x0008)
#define MT_I2C_INTR_STAT                       ((base) + 0x000c)
#define MT_I2C_CONTROL                         ((base) + 0x0010)
#define MT_I2C_TRANSFER_LEN                    ((base) + 0x0014)
#define MT_I2C_TRANSAC_LEN                     ((base) + 0x0018)
#define MT_I2C_DELAY_LEN                       ((base) + 0x001c)
#define MT_I2C_TIMING                          ((base) + 0x0020)
#define MT_I2C_START                           ((base) + 0x0024)
#define MT_I2C_FIFO_STAT                       ((base) + 0x0030)
#define MT_I2C_FIFO_THRESH                     ((base) + 0x0034)
#define MT_I2C_FIFO_ADDR_CLR                   ((base) + 0x0038)
#define MT_I2C_IO_CONFIG                       ((base) + 0x0040)
#define MT_I2C_DEBUG                           ((base) + 0x0044)
#define MT_I2C_HS                              ((base) + 0x0048)
#define MT_I2C_SOFTRESET                       ((base) + 0x0050)
#define MT_I2C_DEBUGSTAT                       ((base) + 0x0064)
#define MT_I2C_DEBUGCTRL                       ((base) + 0x0068)

#define I2C_TRANS_LEN_MASK                  (0xff)
#define I2C_TRANS_AUX_LEN_MASK              (0x1f << 8)
#define I2C_CONTROL_MASK                    (0x3f << 1)

//----------- Register mask -------------------//
#define I2C_3_BIT_MASK                      0x07
#define I2C_4_BIT_MASK                      0x0f
#define I2C_6_BIT_MASK                      0x3f
#define I2C_8_BIT_MASK                      0xff
#define I2C_FIFO_THRESH_MASK                0x07
#define I2C_AUX_LEN_MASK                    0x1f00
#define I2C_MASTER_READ                     0x01
#define I2C_MASTER_WRITE                    0x00
#define I2C_CTL_RS_STOP_BIT                 0x02
#define I2C_CTL_DMA_EN_BIT                  0x04
#define I2C_CTL_CLK_EXT_EN_BIT              0x08
#define I2C_CTL_DIR_CHANGE_BIT              0x10
#define I2C_CTL_ACK_ERR_DET_BIT             0x20 
#define I2C_CTL_TRANSFER_LEN_CHG_BIT        0x40
#define I2C_DATA_READ_ADJ_BIT               0x8000
#define I2C_SCL_MODE_BIT                    0x01
#define I2C_SDA_MODE_BIT                    0x02
#define I2C_BUS_DETECT_EN_BIT               0x04
#define I2C_ARBITRATION_BIT                 0x01
#define I2C_CLOCK_SYNC_BIT                  0x02
#define I2C_BUS_BUSY_DET_BIT                0x04
#define I2C_HS_EN_BIT                       0x01
#define I2C_HS_NACK_ERR_DET_EN_BIT          0x02
#define I2C_HS_MASTER_CODE_MASK             0x70
#define I2C_HS_STEP_CNT_DIV_MASK            0x700
#define I2C_HS_SAMPLE_CNT_DIV_MASK          0x7000
#define I2C_FIFO_FULL_STATUS                0x01
#define I2C_FIFO_EMPTY_STATUS               0x02

#define I2C_DEBUG                           (1 << 3)
#define I2C_HS_NACKERR                      (1 << 2)
#define I2C_ACKERR                          (1 << 1)
#define I2C_TRANSAC_COMP                    (1 << 0)

#define I2C_TX_THR_OFFSET                   8
#define I2C_RX_THR_OFFSET                   0

/* i2c control bits */
#define TRANS_LEN_CHG                       (1 << 6)
#define ACK_ERR_DET_EN                      (1 << 5)
#define DIR_CHG                             (1 << 4)
#define CLK_EXT                             (1 << 3)
#define DMA_EN                              (1 << 2)
#define REPEATED_START_FLAG                 (1 << 1)
#define STOP_FLAG                           (0 << 1)

//==============================================================================
// I2C Configuration
//==============================================================================

#if (CFG_FPGA_PLATFORM)
#define I2C_CLK_RATE                        13000   
#else
#define I2C_CLK_RATE                        12350   /* EMI/16 (khz) */
#endif
#define I2C_FIFO_SIZE                       8

#define MAX_ST_MODE_SPEED                   100     /* khz */
#define MAX_FS_MODE_SPEED                   400     /* khz */
#define MAX_HS_MODE_SPEED                   3400    /* khz */

#define MAX_DMA_TRANS_SIZE                  252     /* Max(255) aligned to 4 bytes = 252 */
#define MAX_DMA_TRANS_NUM                   256

#define MAX_SAMPLE_CNT_DIV                  8
#define MAX_STEP_CNT_DIV                    64
#define MAX_HS_STEP_CNT_DIV                 8

typedef enum {
    ST_MODE,
    FS_MODE,
    HS_MODE,
} I2C_SPD_MODE;

#define I2C_TIMEOUT_TH                      200     // i2c wait for response timeout value, 200ms


//==============================================================================
// I2C Macro
//==============================================================================
#define I2C_START_TRANSAC                   DRV_WriteReg32(MT_I2C_START,0x1)
#define I2C_FIFO_CLR_ADDR                   DRV_WriteReg32(MT_I2C_FIFO_ADDR_CLR,0x1)
#define I2C_FIFO_OFFSET                     (DRV_Reg32(MT_I2C_FIFO_STAT)>>4&0xf)
#define I2C_FIFO_IS_EMPTY                   (DRV_Reg16(MT_I2C_FIFO_STAT)>>0&0x1)

#define I2C_SOFTRESET                       DRV_WriteReg32(MT_I2C_SOFTRESET,0x1)
#define I2C_INTR_STATUS                     DRV_Reg16(MT_I2C_INTR_STAT)

#define I2C_SET_BITS(BS,REG)                ((*(volatile u32*)(REG)) |= (u32)(BS))
#define I2C_CLR_BITS(BS,REG)                ((*(volatile u32*)(REG)) &= ~((u32)(BS)))

#define I2C_SET_FIFO_THRESH(tx,rx) \
    do { u32 tmp = (((tx) & 0x7) << I2C_TX_THR_OFFSET) | \
                   (((rx) & 0x7) << I2C_RX_THR_OFFSET); \
         DRV_WriteReg32(MT_I2C_FIFO_THRESH,tmp); \
    } while(0)

#define I2C_SET_INTR_MASK(mask)             DRV_WriteReg32(MT_I2C_INTR_MASK,mask)

#define I2C_CLR_INTR_MASK(mask)\
    do { u32 tmp = DRV_Reg32(MT_I2C_INTR_MASK); \
         tmp &= ~(mask); \
         DRV_WriteReg32(MT_I2C_INTR_MASK,tmp); \
    } while(0)

#define I2C_SET_SLAVE_ADDR(addr)            DRV_WriteReg32(MT_I2C_SLAVE_ADDR,addr)

#define I2C_SET_TRANS_LEN(len) \
    do { u32 tmp = DRV_Reg32(MT_I2C_TRANSFER_LEN) & \
                              ~I2C_TRANS_LEN_MASK; \
         tmp |= ((len) & I2C_TRANS_LEN_MASK); \
         DRV_WriteReg32(MT_I2C_TRANSFER_LEN,tmp); \
    } while(0)

#define I2C_SET_TRANS_AUX_LEN(len) \
    do { u32 tmp = DRV_Reg32(MT_I2C_TRANSFER_LEN) & \
                             ~(I2C_TRANS_AUX_LEN_MASK); \
         tmp |= (((len) << 8) & I2C_TRANS_AUX_LEN_MASK); \
         DRV_WriteReg32(MT_I2C_TRANSFER_LEN,tmp); \
    } while(0)

#define I2C_SET_TRANSAC_LEN(len)            DRV_WriteReg32(MT_I2C_TRANSAC_LEN,len)
#define I2C_SET_TRANS_DELAY(delay)          DRV_WriteReg32(MT_I2C_DELAY_LEN,delay)

#define I2C_SET_TRANS_CTRL(ctrl)\
    do { u32 tmp = DRV_Reg32(MT_I2C_CONTROL) & ~I2C_CONTROL_MASK; \
        tmp |= ((ctrl) & I2C_CONTROL_MASK); \
        DRV_WriteReg32(MT_I2C_CONTROL,tmp); \
    } while(0)

#define I2C_SET_HS_MODE(on_off) \
    do { u32 tmp = DRV_Reg32(MT_I2C_HS) & ~0x1; \
    tmp |= (on_off & 0x1); \
    DRV_WriteReg32(MT_I2C_HS,tmp); \
    } while(0)

#define I2C_READ_BYTE(byte)     \
    do { byte = DRV_Reg8(MT_I2C_DATA_PORT); } while(0)
    
#define I2C_WRITE_BYTE(byte) \
    do { DRV_WriteReg8(MT_I2C_DATA_PORT,byte); } while(0)

#define I2C_CLR_INTR_STATUS(status) \
    do { DRV_WriteReg16(MT_I2C_INTR_STAT,status); } while(0)



//==============================================================================
// I2C Status Code
//==============================================================================
#define I2C_OK                              0x0000
#define I2C_SET_SPEED_FAIL_OVER_SPEED       0xA001
#define I2C_READ_FAIL_ZERO_LENGTH           0xA002
#define I2C_READ_FAIL_HS_NACKERR            0xA003
#define I2C_READ_FAIL_ACKERR                0xA004
#define I2C_READ_FAIL_TIMEOUT               0xA005
#define I2C_WRITE_FAIL_ZERO_LENGTH          0xA012
#define I2C_WRITE_FAIL_HS_NACKERR           0xA013
#define I2C_WRITE_FAIL_ACKERR               0xA014
#define I2C_WRITE_FAIL_TIMEOUT              0xA015

//==============================================================================
// I2C Exported Function
//==============================================================================
extern u32 i2c_v1_init ();
extern u32 i2c_v1_deinit (u8);
extern u32 i2c_v1_set_speed (u8,unsigned long clock, I2C_SPD_MODE mode, unsigned long khz);
extern u32 i2c_v1_read(u8,u8 chip, u8 *buffer, int len);
extern u32 i2c_v1_write (u8,u8 chip, u8 *buffer, int len);


#endif /* __MT6516_I2C_H__ */
