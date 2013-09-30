/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
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

/* 
 *
 * (C) Copyright 2009 
 * MediaTek <www.mediatek.com>
 * James Lo <james.lo@mediatek.com>
 *
 * I2C Slave Device Driver (bq24158)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
 
#include <platform/mt_typedefs.h>
#include <platform/mt_reg_base.h>
#include <platform/i2c.h>     
#include <platform/bq24158.h>
#include <printf.h>

/**********************************************************
  *
  *   [I2C Slave Setting] 
  *
  *********************************************************/
#define bq24158_SLAVE_ADDR_WRITE 0xD4
#define bq24158_SLAVE_ADDR_Read	0xD5

/**********************************************************
  *
  *   [Global Variable] 
  *
  *********************************************************/
#define bq24158_REG_NUM 7  
kal_uint8 bq24158_reg[bq24158_REG_NUM] = {0};

/**********************************************************
  *
  *   [I2C Function For Read/Write bq24158] 
  *
  *********************************************************/
U32 bq24158_i2c_read (U8 chip, U8 *cmdBuffer, int cmdBufferLen, U8 *dataBuffer, int dataBufferLen)
{
    U32 ret_code = I2C_OK;

    ret_code = i2c_v1_write(I2C0, chip, cmdBuffer, cmdBufferLen);    // set register command
    if (ret_code != I2C_OK)
        return ret_code;

    ret_code = i2c_v1_read(I2C0, chip, dataBuffer, dataBufferLen);

    //printf("[bq24158_i2c_read] Done\n");

    return ret_code;
}

U32 bq24158_i2c_write (U8 chip, U8 *cmdBuffer, int cmdBufferLen, U8 *dataBuffer, int dataBufferLen)
{
    U32 ret_code = I2C_OK;
    U8 write_data[I2C_FIFO_SIZE];
    int transfer_len = cmdBufferLen + dataBufferLen;
    int i=0, cmdIndex=0, dataIndex=0;

    if(I2C_FIFO_SIZE < (cmdBufferLen + dataBufferLen))
    {
        printf("[bq24158_i2c_write] exceed I2C FIFO length!! \n");
        return 0;
    }

    //write_data[0] = cmd;
    //write_data[1] = writeData;

    while(cmdIndex < cmdBufferLen)
    {
        write_data[i] = cmdBuffer[cmdIndex];
        cmdIndex++;
        i++;
    }

    while(dataIndex < dataBufferLen)
    {
        write_data[i] = dataBuffer[dataIndex];
        dataIndex++;
        i++;
    }

    /* dump write_data for check */
    for( i=0 ; i < transfer_len ; i++ )
    {
        //printf("[bq24158_i2c_write] write_data[%d]=%x\n", i, write_data[i]);
    }

    ret_code = i2c_v1_write(I2C0, chip, write_data, transfer_len);

    //printf("[bq24158_i2c_write] Done\n");

    return ret_code;
}

/**********************************************************
  *
  *   [Read / Write Function] 
  *
  *********************************************************/
kal_uint32 bq24158_read_interface (kal_uint8 RegNum, kal_uint8 *val, kal_uint8 MASK, kal_uint8 SHIFT)
{
    U8 chip_slave_address = bq24158_SLAVE_ADDR_WRITE;
    U8 cmd = 0x0;
    int cmd_len = 1;
    U8 data = 0xFF;
    int data_len = 1;
    U32 result_tmp;

	printf("--------------------------------------------------\n");

    cmd = RegNum;
    result_tmp = bq24158_i2c_read(chip_slave_address, &cmd, cmd_len, &data, data_len);
    
	printf("[bq24158_read_interface] Reg[%x]=0x%x\n", RegNum, data);
	
	data &= (MASK << SHIFT);
	*val = (data >> SHIFT);
	
	printf("[bq24158_read_interface] val=0x%x\n", *val);

	return 1;
}

kal_uint32 bq24158_config_interface (kal_uint8 RegNum, kal_uint8 val, kal_uint8 MASK, kal_uint8 SHIFT)
{
    U8 chip_slave_address = bq24158_SLAVE_ADDR_WRITE;
    U8 cmd = 0x0;
    int cmd_len = 1;
    U8 data = 0xFF;
    int data_len = 1;
    U32 result_tmp;

	printf("--------------------------------------------------\n");

    cmd = RegNum;
    result_tmp = bq24158_i2c_read(chip_slave_address, &cmd, cmd_len, &data, data_len);
    printf("[bq24158_config_interface] Reg[%x]=0x%x\n", RegNum, data);

    data &= ~(MASK << SHIFT);
    data |= (val << SHIFT);

    result_tmp = bq24158_i2c_write(chip_slave_address, &cmd, cmd_len, &data, data_len);
    printf("[bq24158_config_interface] write Reg[%x]=0x%x\n", RegNum, data);

    // Check
    result_tmp = bq24158_i2c_read(chip_slave_address, &cmd, cmd_len, &data, data_len);
    printf("[bq24158_config_interface] Check Reg[%x]=0x%x\n", RegNum, data);
    
	return 1;
}

/**********************************************************
  *
  *   [bq24158 Function] 
  *
  *********************************************************/
//CON0
void bq24158_set_tmr_rst(unsigned int val)
{
	kal_uint32 ret=0;	

	ret=bq24158_config_interface(	(kal_uint8)(bq24158_CON0), 
									(kal_uint8)(val),
									(kal_uint8)(CON0_TMR_RST_MASK),
									(kal_uint8)(CON0_TMR_RST_SHIFT)
									);
}

unsigned int bq24158_get_slrst_status(void)
{
	kal_uint32 ret=0;
	kal_uint8 val=0;

	ret=bq24158_read_interface( 	(kal_uint8)(bq24158_CON0), 
									(&val),
									(kal_uint8)(CON0_SLRST_MASK),
									(kal_uint8)(CON0_SLRST_SHIFT)
									);
	return val;
}

void bq24158_set_en_stat(unsigned int val)
{
	kal_uint32 ret=0;	

	ret=bq24158_config_interface(	(kal_uint8)(bq24158_CON0), 
									(kal_uint8)(val),
									(kal_uint8)(CON0_EN_STAT_MASK),
									(kal_uint8)(CON0_EN_STAT_SHIFT)
									);
}

unsigned int bq24158_get_chip_status(void)
{
	kal_uint32 ret=0;
	kal_uint8 val=0;

	ret=bq24158_read_interface( 	(kal_uint8)(bq24158_CON0), 
									(&val),
									(kal_uint8)(CON0_STAT_MASK),
									(kal_uint8)(CON0_STAT_SHIFT)
									);
	return val;
}

unsigned int bq24158_get_fault_reason(void)
{
	kal_uint32 ret=0;
	kal_uint8 val=0;

	ret=bq24158_read_interface( 	(kal_uint8)(bq24158_CON0), 
									(&val),
									(kal_uint8)(CON0_FAULT_MASK),
									(kal_uint8)(CON0_FAULT_SHIFT)
									);
	return val;
}

//CON1
void bq24158_set_lin_limit(unsigned int val)
{
	kal_uint32 ret=0;	

	ret=bq24158_config_interface(	(kal_uint8)(bq24158_CON1), 
									(kal_uint8)(val),
									(kal_uint8)(CON1_LIN_LIMIT_MASK),
									(kal_uint8)(CON1_LIN_LIMIT_SHIFT)
									);
}

void bq24158_set_lowv_2(unsigned int val)
{
	kal_uint32 ret=0;	

	ret=bq24158_config_interface(	(kal_uint8)(bq24158_CON1), 
									(kal_uint8)(val),
									(kal_uint8)(CON1_LOW_V_2_MASK),
									(kal_uint8)(CON1_LOW_V_2_SHIFT)
									);
}

void bq24158_set_lowv_1(unsigned int val)
{
	kal_uint32 ret=0;	

	ret=bq24158_config_interface(	(kal_uint8)(bq24158_CON1), 
									(kal_uint8)(val),
									(kal_uint8)(CON1_LOW_V_1_MASK),
									(kal_uint8)(CON1_LOW_V_1_SHIFT)
									);
}

void bq24158_set_te(unsigned int val)
{
	kal_uint32 ret=0;	

	ret=bq24158_config_interface(	(kal_uint8)(bq24158_CON1), 
									(kal_uint8)(val),
									(kal_uint8)(CON1_TE_MASK),
									(kal_uint8)(CON1_TE_SHIFT)
									);
}

void bq24158_set_ce(unsigned int val)
{
	kal_uint32 ret=0;	

	ret=bq24158_config_interface(	(kal_uint8)(bq24158_CON1), 
									(kal_uint8)(val),
									(kal_uint8)(CON1_CE_MASK),
									(kal_uint8)(CON1_CE_SHIFT)
									);
}

void bq24158_set_hz_mode(unsigned int val)
{
	kal_uint32 ret=0;	

	ret=bq24158_config_interface(	(kal_uint8)(bq24158_CON1), 
									(kal_uint8)(val),
									(kal_uint8)(CON1_HZ_MODE_MASK),
									(kal_uint8)(CON1_HZ_MODE_SHIFT)
									);
}

void bq24158_set_opa_mode(unsigned int val)
{
	kal_uint32 ret=0;	

	ret=bq24158_config_interface(	(kal_uint8)(bq24158_CON1), 
									(kal_uint8)(val),
									(kal_uint8)(CON1_OPA_MODE_MASK),
									(kal_uint8)(CON1_OPA_MODE_SHIFT)
									);
}

//CON2
void bq24158_set_cv_vth(unsigned int val)
{
	kal_uint32 ret=0;	

	ret=bq24158_config_interface(	(kal_uint8)(bq24158_CON2), 
									(kal_uint8)(val),
									(kal_uint8)(CON2_CV_VTH_MASK),
									(kal_uint8)(CON2_CV_VTH_SHIFT)
									);
}

//CON3
unsigned int bq24158_get_vender_code(void)
{
	kal_uint32 ret=0;
	kal_uint8 val=0;

	ret=bq24158_read_interface( 	(kal_uint8)(bq24158_CON3), 
									(&val),
									(kal_uint8)(CON3_VENDER_CODE_MASK),
									(kal_uint8)(CON3_VENDER_CODE_SHIFT)
									);
	return val;
}

unsigned int bq24158_get_pin(void)
{
	kal_uint32 ret=0;
	kal_uint8 val=0;

	ret=bq24158_read_interface( 	(kal_uint8)(bq24158_CON3), 
									(&val),
									(kal_uint8)(CON3_PIN_MASK),
									(kal_uint8)(CON3_PIN_SHIFT)
									);
	return val;
}

unsigned int bq24158_get_revision(void)
{
	kal_uint32 ret=0;
	kal_uint32 val=0;

	ret=bq24158_read_interface( 	(kal_uint8)(bq24158_CON3), 
									(&val),
									(kal_uint8)(CON3_REVISION_MASK),
									(kal_uint8)(CON3_REVISION_SHIFT)
									);
	return val;
}

//CON4
void bq24158_set_reset(unsigned int val)
{
	kal_uint32 ret=0;	

	ret=bq24158_config_interface(	(kal_uint8)(bq24158_CON4), 
									(kal_uint8)(val),
									(kal_uint8)(CON4_RESET_MASK),
									(kal_uint8)(CON4_RESET_SHIFT)
									);
}

void bq24158_set_ac_charging_current(unsigned int val)
{
	kal_uint32 ret=0;	

	ret=bq24158_config_interface(	(kal_uint8)(bq24158_CON4), 
									(kal_uint8)(val),
									(kal_uint8)(CON4_I_CHR_MASK),
									(kal_uint8)(CON4_I_CHR_SHIFT)
									);
}

void bq24158_set_termination_current(unsigned int val)
{
	kal_uint32 ret=0;	

	ret=bq24158_config_interface(	(kal_uint8)(bq24158_CON4), 
									(kal_uint8)(val),
									(kal_uint8)(CON4_I_TERM_MASK),
									(kal_uint8)(CON4_I_TERM_SHIFT)
									);
}

//CON5
void bq24158_set_low_chg(unsigned int val)
{
	kal_uint32 ret=0;	

	ret=bq24158_config_interface(	(kal_uint8)(bq24158_CON5), 
									(kal_uint8)(val),
									(kal_uint8)(CON5_LOW_CHG_MASK),
									(kal_uint8)(CON5_LOW_CHG_SHIFT)
									);
}

unsigned int bq24158_get_dpm_status(void)
{
	kal_uint32 ret=0;
	kal_uint8 val=0;

	ret=bq24158_read_interface( 	(kal_uint8)(bq24158_CON5), 
									(&val),
									(kal_uint8)(CON5_DPM_STATUS_MASK),
									(kal_uint8)(CON5_DPM_STATUS_SHIFT)
									);
	return val;
}

unsigned int bq24158_get_cd_status(void)
{
	kal_uint32 ret=0;
	kal_uint8 val=0;

	ret=bq24158_read_interface( 	(kal_uint8)(bq24158_CON5), 
									(&val),
									(kal_uint8)(CON5_CD_STATUS_MASK),
									(kal_uint8)(CON5_CD_STATUS_SHIFT)
									);
	return val;
}

void bq24158_set_vsreg(unsigned int val)
{
	kal_uint32 ret=0;	

	ret=bq24158_config_interface(	(kal_uint8)(bq24158_CON5), 
									(kal_uint8)(val),
									(kal_uint8)(CON5_VSREG_MASK),
									(kal_uint8)(CON5_VSREG_SHIFT)
									);
}

//CON6
void bq24158_set_mchrg(unsigned int val)
{
	kal_uint32 ret=0;	

	ret=bq24158_config_interface(	(kal_uint8)(bq24158_CON6), 
									(kal_uint8)(val),
									(kal_uint8)(CON6_MCHRG_MASK),
									(kal_uint8)(CON6_MCHRG_SHIFT)
									);
}

void bq24158_set_mreg(unsigned int val)
{
	kal_uint32 ret=0;	

	ret=bq24158_config_interface(	(kal_uint8)(bq24158_CON6), 
									(kal_uint8)(val),
									(kal_uint8)(CON6_MREG_MASK),
									(kal_uint8)(CON6_MREG_SHIFT)
									);
}

/**********************************************************
  *
  *   [Internal Function] 
  *
  *********************************************************/
//debug 
unsigned int bq24158_config_interface_reg (unsigned char RegNum, unsigned char val)
{
    kal_uint32 ret=0;
	kal_uint8 ret_val=0;

    printf("--------------------------------------------------\n");
    
    //bq24158_read_byte(RegNum, &bq24158_reg);
	//ret=bq24158_read_interface(RegNum,&val,0xFF,0x0);
    //printk("[ bq24158_config_interface] Reg[%x]=0x%x\n", RegNum, val);
    
    //bq24158_write_byte(RegNum, val);
    ret=bq24158_config_interface(RegNum,val,0xFF,0x0);
    printf("[ bq24158_config_interface] write Reg[%x]=0x%x\n", RegNum, val);

    // Check
    //bq24158_read_byte(RegNum, &bq24158_reg);
    ret=bq24158_read_interface(RegNum,&ret_val,0xFF,0x0);
    printf("[ bq24158_config_interface] Check Reg[%x]=0x%x\n", RegNum, ret_val);

    return 1;
}

void bq24158_dump_register(void)
{
    U8 chip_slave_address = bq24158_SLAVE_ADDR_WRITE;
    U8 cmd = 0x0;
    int cmd_len = 1;
    U8 data = 0xFF;
    int data_len = 1;
    int i=0;
    for (i=0;i<bq24158_REG_NUM;i++)
	{
        cmd = i;
        bq24158_i2c_read(chip_slave_address, &cmd, cmd_len, &data, data_len);
        bq24158_reg[i] = data;
        printf("[bq24158_dump_register] Reg[0x%X]=0x%X\n", i, bq24158_reg[i]);
    }
}

void bq24158_read_register(int i)
{
    U8 chip_slave_address = bq24158_SLAVE_ADDR_WRITE;
    U8 cmd = 0x0;
    int cmd_len = 1;
    U8 data = 0xFF;
    int data_len = 1;

    cmd = i;
    bq24158_i2c_read(chip_slave_address, &cmd, cmd_len, &data, data_len);
    bq24158_reg[i] = data;
    printf("[bq24158_read_register] Reg[0x%X]=0x%X\n", i, bq24158_reg[i]);
}

void bq24158_hw_init(void)
{	
#if defined(HIGH_BATTERY_VOLTAGE_SUPPORT)
    printf("[bq24158_hw_init] (0x06,0x77)\n");
    bq24158_config_interface_reg(0x06,0x77); // set ISAFE and HW CV point (4.34)
#else
    printf("[bq24158_hw_init] (0x06,0x70)\n");
    bq24158_config_interface_reg(0x06,0x70); // set ISAFE
#endif
}

