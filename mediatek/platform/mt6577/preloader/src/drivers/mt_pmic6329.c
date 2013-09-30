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

#include <typedefs.h>
#include <mt6577.h>
#include <mt_i2c.h>
#include <mt_pmic6329.h>

#define HW_RESV (0xC1019100)

//==============================================================================
// PMIC6329 Chip Access APIs
//==============================================================================
U32 pmic6329_i2c_read (U8 chip, U8 *cmdBuffer, int cmdBufferLen, U8 *dataBuffer, int dataBufferLen)
{
    U32 ret_code = I2C_OK;

    ret_code = i2c_v1_write(chip, cmdBuffer, cmdBufferLen);    // set register command
    if (ret_code != I2C_OK)
        return ret_code;

    ret_code = i2c_v1_read(chip, dataBuffer, dataBufferLen);

    //print("[pmic6329_i2c_read] Done\n");

    return ret_code;
}

U32 pmic6329_i2c_write (U8 chip, U8 *cmdBuffer, int cmdBufferLen, U8 *dataBuffer, int dataBufferLen)
{
    U32 ret_code = I2C_OK;
    U8 write_data[I2C_FIFO_SIZE];
    int transfer_len = cmdBufferLen + dataBufferLen;
    int i=0, cmdIndex=0, dataIndex=0;

    if(I2C_FIFO_SIZE < (cmdBufferLen + dataBufferLen))
    {   return PMIC_EXCEED_I2C_FIFO_LENGTH;
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
        //print("[pmic6329_i2c_write] write_data[%d]=%x\n", i, write_data[i]);
    }

    ret_code = i2c_v1_write(chip, write_data, transfer_len);

    //print("[pmic6329_i2c_write] Done\n");
    
    return ret_code;
}

U32 pmic6329_int_ctrl_enable (U8 chip, U8 ctrl_type, U8 sel, kal_bool enable)
{
    U32 ret_code = PMIC_TEST_PASS;
    U32 delay_time_us = 50000;
    //U8 chip_slave_address = PMIC_SLAVE_ADDR;
	U8 chip_slave_address = chip;
    U8 cmd = 0x0;
    int cmd_len = 1;
    U8 data = 0x00;
    int data_len = 1;	
    U32 result_tmp;
    int i=0;

    cmd = ctrl_type;	
    if (enable){
        data |= (kal_uint8)sel;
    }else{
        data &= ~((kal_uint8)sel);
    }
    result_tmp = pmic6329_i2c_write(chip_slave_address, &cmd, cmd_len, &data, data_len);
    if(result_tmp != I2C_OK)
    {
        print("[pmic6329_int_ctrl_enable] Reg[%x]=0x%x\n", cmd, data);
        print("[pmic6329_int_ctrl_enable] -------------------------\n");
        return result_tmp;
    }

    /* check */
    result_tmp = pmic6329_i2c_read(chip_slave_address, &cmd, cmd_len, &data, data_len);
    if(result_tmp != I2C_OK)
    {
        print("[pmic6329_int_ctrl_enable] Reg[%x]=0x%x\n", cmd, data);
        print("[pmic6329_int_ctrl_enable] -------------------------\n");
        return result_tmp;
    }

    return result_tmp;
}

//==============================================================================
// PMIC6329 Usage APIs
//==============================================================================
U32 pmic6329_get_version (void)
{
    U32 ret_code = PMIC_TEST_PASS;
    U32 delay_time_us = 50000;
    U8 chip_slave_address = mt6329_BANK0_SLAVE_ADDR_WRITE;
    U8 cmd = 0x0;
    int cmd_len = 1;
    U8 data = 0xFF;
    int data_len = 1;	
    kal_uint16 eco_version = 0;
    U32 result_tmp;

#if 0
    gpt4_busy_wait_us(delay_time_us);
#endif

    /* Low part of CID */
    cmd = BANK0_CID0;	
    result_tmp = pmic6329_i2c_read(chip_slave_address, &cmd, cmd_len, &data, data_len);
    eco_version |= data;

    //print("[pmic6329_get_version] Reg(%x) = %x\n", cmd, data);

    /* High part of CID */
    cmd = BANK0_CID1;	
    result_tmp = pmic6329_i2c_read(chip_slave_address, &cmd, cmd_len, &data, data_len);	
    eco_version |= (data << 8);

    //print("[pmic6329_get_version] Reg(%x) = %x\n", cmd, data);

    if (eco_version == PMIC6329_E1_CID_CODE)
    {
        print("[pmic6329_get_version] PMIC6329_E1_CID_CODE !!\n");
    }
	else if (eco_version == PMIC6329_E2_CID_CODE)
    {
        printf("[pmic6329_get_version] PMIC6329_E2_CID_CODE !!\n");
    }
    else
    {
        print("[pmic6329_get_version] UNKNOWN VERSION !!\n");
        ret_code = PMIC_TEST_FAIL;
    }

    return ret_code;
}

U32 get_pmic6329_chip_version (void)
{
    U32 delay_time_us = 50000;
    U8 chip_slave_address = mt6329_BANK0_SLAVE_ADDR_WRITE;
    U8 cmd = 0x0;
    int cmd_len = 1;
    U8 data = 0xFF;
    int data_len = 1;	
    U32 eco_version = 0;
    U32 result_tmp;

    /* Low part of CID */
    cmd = BANK0_CID0;	
    result_tmp = pmic6329_i2c_read(chip_slave_address, &cmd, cmd_len, &data, data_len);
    eco_version |= data;

    /* High part of CID */
    cmd = BANK0_CID1;	
    result_tmp = pmic6329_i2c_read(chip_slave_address, &cmd, cmd_len, &data, data_len);	
    eco_version |= (data << 8);    

    return eco_version;
}


U32 pmic_IsUsbCableIn (void) 
{
    U32 ret_code = I2C_OK;
    U32 delay_time_us = 50000;
    U8 chip_slave_address = mt6329_BANK0_SLAVE_ADDR_WRITE;
    //U8 cmd = PMIC_STATUS_REG;
    U8 cmd = BANK0_CHRSTATUS;
    int cmd_len = 1;
    U8 data = 0x00;
    int data_len = 1;	
    
    gpt_busy_wait_us(delay_time_us);

    ret_code = pmic6329_i2c_read(chip_slave_address, &cmd, cmd_len, &data, data_len);
    //if (data & (CHRDET_MASK << CHRDET_SHIFT)){
    if (data & (BANK_0_PCHR_CHRDET_MASK << BANK_0_PCHR_CHRDET_SHIFT)){
        //print("[PMIC_IsUsbCableIn] Get register %x : data = %x\n", cmd, data);
        print("[PMIC_IsUsbCableIn] PMIC_CHRDET_EXIST\n");
        ret_code = PMIC_CHRDET_EXIST;
    }else{
        //print("[PMIC_IsUsbCableIn] Get register %x : data = %x\n", cmd, data);
        print("[PMIC_IsUsbCableIn] PMIC_CHRDET_NOT_EXIST\n");
        //ret_code = PMIC_CHRDET_NOT_EXIST;
    }

    return ret_code;
}

//==============================================================================
// PMIC6329 Access Interface
//==============================================================================
kal_uint32 pmic_read_interface (kal_uint8 RegNum, kal_uint8 *val, kal_uint8 MASK, kal_uint8 SHIFT)
{
    U8 chip_slave_address = 0xC0;
    U8 cmd = 0x0;
    int cmd_len = 1;
    U8 data = 0xFF;
    int data_len = 1;	
    U32 result_tmp;

    cmd = RegNum;	
    result_tmp = pmic6329_i2c_read(chip_slave_address, &cmd, cmd_len, &data, data_len);

    data &= (MASK << SHIFT);
    *val = (data >> SHIFT);

	//printf("[pmic_read_interface] Reg[0x%x]=0x%x, val=%d\n", RegNum, data, *val);

	return 0;
}

kal_uint32 pmic_config_interface (kal_uint8 RegNum, kal_uint8 val, kal_uint8 MASK, kal_uint8 SHIFT)
{
    U8 chip_slave_address = 0xC0;
    U8 cmd = 0x0;
    int cmd_len = 1;
    U8 data = 0xFF;
    int data_len = 1;	
    U32 result_tmp;

    cmd = RegNum;	
    result_tmp = pmic6329_i2c_read(chip_slave_address, &cmd, cmd_len, &data, data_len);

    data &= ~(MASK << SHIFT);
    data |= (val << SHIFT);

    result_tmp = pmic6329_i2c_write(chip_slave_address, &cmd, cmd_len, &data, data_len);

    //check 
    //result_tmp = pmic6329_i2c_read(chip_slave_address, &cmd, cmd_len, &data, data_len);
	
	//printf("[pmic_config_interface] Reg[0x%x]=0x%x\n", RegNum, data);

	return 0;
}

kal_uint32 pmic_bank1_read_interface (kal_uint8 RegNum, kal_uint8 *val, kal_uint8 MASK, kal_uint8 SHIFT)
{
    U8 chip_slave_address = 0xC2;
    U8 cmd = 0x0;
    int cmd_len = 1;
    U8 data = 0xFF;
    int data_len = 1;	
    U32 result_tmp;

    cmd = RegNum;	
    result_tmp = pmic6329_i2c_read(chip_slave_address, &cmd, cmd_len, &data, data_len);

    data &= (MASK << SHIFT);
    *val = (data >> SHIFT);

	//printf("[pmic_bank1_read_interface] Reg[0x%x]=0x%x, val=%d\n", RegNum, data, *val);

	return 0;
}

kal_uint32 pmic_bank1_config_interface (kal_uint8 RegNum, kal_uint8 val, kal_uint8 MASK, kal_uint8 SHIFT)
{
    U8 chip_slave_address = 0xC2;
    U8 cmd = 0x0;
    int cmd_len = 1;
    U8 data = 0xFF;
    int data_len = 1;	
    U32 result_tmp;

    cmd = RegNum;	
    result_tmp = pmic6329_i2c_read(chip_slave_address, &cmd, cmd_len, &data, data_len);

    data &= ~(MASK << SHIFT);
    data |= (val << SHIFT);

    result_tmp = pmic6329_i2c_write(chip_slave_address, &cmd, cmd_len, &data, data_len);

    //check 
    //result_tmp = pmic6329_i2c_read(chip_slave_address, &cmd, cmd_len, &data, data_len);

	//printf("[pmic_bank1_config_interface] Reg[0x%x]=0x%x\n", RegNum, data);

	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////
// PMIC-Charger Type Detection
//////////////////////////////////////////////////////////////////////////////////////////
CHARGER_TYPE g_ret = PMIC_CHARGER_UNKNOWN;
int g_charger_in_flag = 0;
int g_first_check=0;

CHARGER_TYPE hw_charger_type_detection(void)
{
	CHARGER_TYPE ret 				= PMIC_CHARGER_UNKNOWN;
	unsigned int USB_U2PHYACR3_2 	= 0xC126081C;
	unsigned int USBPHYRegs			= 0xC1260800; //U2B20_Base+0x800
	unsigned short wChargerAvail	= 0;
	unsigned short bLineState_B		= 0;
	unsigned short bLineState_C 	= 0;
	int ret_val						= 0;
	unsigned short testReadValue	= 0;

	//msleep(400);
	//printf("mt_charger_type_detection : start!\r\n");

/********* Step 0.0 : enable USB memory and clock *********/
	//hwEnableClock(MT65XX_PDN_PERI_USB1, "pmu");
	//hwEnableClock(MT65XX_PDN_PERI_USB2, "pmu");
	//mdelay(1);

/********* Step 1.0 : PMU_BC11_Detect_Init ***************/		
	SETREG8(USB_U2PHYACR3_2,0x80); //USB_U2PHYACR3[7]=1		
	
	//BC11_RST=1
	ret_val=pmic_config_interface(0x33,0x1,BANK_0_BC11_RST_MASK,BANK_0_BC11_RST_SHIFT); 
	//BC11_RST=0	
	//ret_val=pmic_config_interface(0x33,0x0,BANK_0_BC11_RST_MASK,BANK_0_BC11_RST_SHIFT); 
	//BC11_BB_CTRL=1
	ret_val=pmic_config_interface(0x33,0x1,BANK_0_BC11_BB_CTRL_MASK,BANK_0_BC11_BB_CTRL_SHIFT);
	
	//RG_BC11_BIAS_EN=1	
	ret_val=pmic_config_interface(0x34,0x1,BANK_0_BC11_BIAS_EN_MASK,BANK_0_BC11_BIAS_EN_SHIFT); 
	//RG_BC11_VSRC_EN[1:0]=00
	ret_val=pmic_config_interface(0x33,0x0,BANK_0_RG_BC11_VSRC_EN_MASK,BANK_0_RG_BC11_VSRC_EN_SHIFT); 
	//RG_BC11_VREF_VTH = 0
	ret_val=pmic_config_interface(0x34,0x0,BANK_0_BC11_VREF_VTH_MASK,BANK_0_BC11_VREF_VTH_SHIFT); 
	//RG_BC11_CMP_EN[1.0] = 00
	ret_val=pmic_config_interface(0x34,0x0,BANK_0_BC11_CMP_EN_MASK,BANK_0_BC11_CMP_EN_SHIFT);
	//RG_BC11_IPU_EN[1.0] = 00
	ret_val=pmic_config_interface(0x34,0x0,BANK_0_BC11_IPU_EN_MASK,BANK_0_BC11_IPU_EN_SHIFT);
	//RG_BC11_IPD_EN[1.0] = 00
	ret_val=pmic_config_interface(0x34,0x0,BANK_0_BC11_IPD_EN_MASK,BANK_0_BC11_IPD_EN_SHIFT);

	//ret_val=pmic_read_interface(0x33,&testReadValue,0xFF,0);		
	//printf("Reg[0x33]=%x, ", testReadValue);
	//ret_val=pmic_read_interface(0x34,&testReadValue,0xFF,0);		
	//printf("Reg[0x34]=%x \r\n", testReadValue);

/********* Step A *************************************/
	//printf("mt_charger_type_detection : step A\r\n");
	//RG_BC11_IPU_EN[1.0] = 00
	ret_val=pmic_config_interface(0x34,0x0,BANK_0_BC11_IPU_EN_MASK,BANK_0_BC11_IPU_EN_SHIFT);
	
	SETREG8(USBPHYRegs+0x1C,0x1000);//RG_PUPD_BIST_EN = 1	
	CLRREG8(USBPHYRegs+0x1C,0x0400);//RG_EN_PD_DM=0
	
	//RG_BC11_VSRC_EN[1.0] = 10 
	ret_val=pmic_config_interface(0x33,0x2,BANK_0_RG_BC11_VSRC_EN_MASK,BANK_0_RG_BC11_VSRC_EN_SHIFT); 
	//RG_BC11_IPD_EN[1:0] = 01
	ret_val=pmic_config_interface(0x34,0x1,BANK_0_BC11_IPD_EN_MASK,BANK_0_BC11_IPD_EN_SHIFT);
	//RG_BC11_VREF_VTH = 0
	ret_val=pmic_config_interface(0x34,0x0,BANK_0_BC11_VREF_VTH_MASK,BANK_0_BC11_VREF_VTH_SHIFT);
  	//RG_BC11_CMP_EN[1.0] = 01
  	ret_val=pmic_config_interface(0x34,0x1,BANK_0_BC11_CMP_EN_MASK,BANK_0_BC11_CMP_EN_SHIFT);
	//mdelay(400);
	mdelay(100);
		
	ret_val=pmic_read_interface(0x33,&wChargerAvail,BANK_0_BC11_CMP_OUT_MASK,BANK_0_BC11_CMP_OUT_SHIFT); 
	//printf("mt_charger_type_detection : step A : wChargerAvail=%x\r\n", wChargerAvail);
	
	//RG_BC11_VSRC_EN[1:0]=00
	ret_val=pmic_config_interface(0x33,0x0,BANK_0_RG_BC11_VSRC_EN_MASK,BANK_0_RG_BC11_VSRC_EN_SHIFT); 
	//RG_BC11_IPD_EN[1.0] = 00
	ret_val=pmic_config_interface(0x34,0x0,BANK_0_BC11_IPD_EN_MASK,BANK_0_BC11_IPD_EN_SHIFT);
	//RG_BC11_CMP_EN[1.0] = 00
	ret_val=pmic_config_interface(0x34,0x0,BANK_0_BC11_CMP_EN_MASK,BANK_0_BC11_CMP_EN_SHIFT);
	mdelay(50);
	
	if(wChargerAvail==1)
	{
/********* Step B *************************************/
		//printk("mt_charger_type_detection : step B\r\n");

		//RG_BC11_IPU_EN[1:0]=10
		ret_val=pmic_config_interface(0x34,0x2,BANK_0_BC11_IPU_EN_MASK,BANK_0_BC11_IPU_EN_SHIFT);		
		mdelay(80);
		
		bLineState_B = INREG8(USBPHYRegs+0x76);
		//printk("mt_charger_type_detection : step B : bLineState_B=%x\r\n", bLineState_B);
		if(bLineState_B & 0x80)
		{
			ret = PMIC_STANDARD_CHARGER;
			printf("mt_charger_type_detection : step B : STANDARD CHARGER!\r\n");
		}
		else
		{
			ret = PMIC_CHARGING_HOST;
			printf("mt_charger_type_detection : step B : Charging Host!\r\n");
		}
	}
	else
	{
/********* Step C *************************************/
		//printk("mt_charger_type_detection : step C\r\n");

		//RG_BC11_IPU_EN[1:0]=01
		ret_val=pmic_config_interface(0x34,0x1,BANK_0_BC11_IPU_EN_MASK,BANK_0_BC11_IPU_EN_SHIFT);
		//RG_BC11_CMP_EN[1.0] = 01
		ret_val=pmic_config_interface(0x34,0x1,BANK_0_BC11_CMP_EN_MASK,BANK_0_BC11_CMP_EN_SHIFT);
		//ret_val=pmic_read_interface(0x34,&testReadValue,0xFF,0);		
		//printf("mt_charger_type_detection : step C : Reg[0x34]=%x\r\n", testReadValue);		
		mdelay(80);
		
		ret_val=pmic_read_interface(0x33,&bLineState_C,0xFF,0);
		//printf("mt_charger_type_detection : step C : bLineState_C=%x\r\n", bLineState_C);
		if(bLineState_C & 0x80)
		{
			ret = PMIC_NONSTANDARD_CHARGER;
			printf("mt_charger_type_detection : step C : UNSTANDARD CHARGER!!\r\n");
			//RG_BC11_IPU_EN[1:0]=10
			ret_val=pmic_config_interface(0x34,0x2,BANK_0_BC11_IPU_EN_MASK,BANK_0_BC11_IPU_EN_SHIFT);			
			mdelay(80);
		}
		else
		{
			ret = PMIC_STANDARD_HOST;
			printf("mt_charger_type_detection : step C : Standard USB Host!\r\n");
		}
	}
/********* Finally setting *******************************/
	//RG_BC11_VSRC_EN[1:0]=00
	ret_val=pmic_config_interface(0x33,0x0,BANK_0_RG_BC11_VSRC_EN_MASK,BANK_0_RG_BC11_VSRC_EN_SHIFT); 
	//RG_BC11_VREF_VTH = 0
	ret_val=pmic_config_interface(0x34,0x0,BANK_0_BC11_VREF_VTH_MASK,BANK_0_BC11_VREF_VTH_SHIFT);
	//RG_BC11_CMP_EN[1.0] = 00
	ret_val=pmic_config_interface(0x34,0x0,BANK_0_BC11_CMP_EN_MASK,BANK_0_BC11_CMP_EN_SHIFT);
	//RG_BC11_IPU_EN[1.0] = 00
	ret_val=pmic_config_interface(0x34,0x0,BANK_0_BC11_IPU_EN_MASK,BANK_0_BC11_IPU_EN_SHIFT);
	//RG_BC11_IPD_EN[1.0] = 00
	ret_val=pmic_config_interface(0x34,0x0,BANK_0_BC11_IPD_EN_MASK,BANK_0_BC11_IPD_EN_SHIFT);
	//RG_BC11_BIAS_EN=0
	ret_val=pmic_config_interface(0x34,0x0,BANK_0_BC11_BIAS_EN_MASK,BANK_0_BC11_BIAS_EN_SHIFT); 
	
	CLRREG8(USB_U2PHYACR3_2,0x80); //USB_U2PHYACR3[7]=0

	//hwDisableClock(MT65XX_PDN_PERI_USB1, "pmu");
	//hwDisableClock(MT65XX_PDN_PERI_USB2, "pmu");

	//step4:done, ret the type	
	return ret;
	
}

CHARGER_TYPE mt_charger_type_detection(void)
{
    if( g_first_check == 0 )
    {
		g_first_check = 1;
		g_ret = hw_charger_type_detection();
    }
    else
    {
		printf("[mt_charger_type_detection] Got data !!, %d, %d\r\n", g_charger_in_flag, g_first_check);
    }

    return g_ret;
}

int t_d3 = 119;
int t_d2_x = 0;
int t_d2 = 0;
int t_d1 = 30;
int read_times = 51;
int t_d4 = 7;

void spk_amp_offset_sw_trim(void)
{
	int ret_val = 0;
	int i=0;
	unsigned short ReadValue = 0;
	unsigned short ReadValue_sum = 0;
	int a_val=0;
	int b_val=0;
	int counter=0;

	t_d2 = t_d3 - t_d2_x;
		
	print("\n[spk_amp_offset_sw_trim] Start...................\n");

	//-------------------------------------------------------init
	ret_val=pmic_config_interface(0xc8,0x81,0xff,0x0);
	ret_val=pmic_bank1_config_interface(0x15,0x0,0x1,0x1);
	ret_val=pmic_bank1_config_interface(0x5a,0x01,0xff,0x0);	
	ret_val=pmic_bank1_config_interface(0x56,0x40,0xff,0x0);
	ret_val=pmic_bank1_config_interface(0x49,0x40,0xff,0x0);
	ret_val=pmic_bank1_config_interface(0x4f,0x40,0xff,0x0);
	ret_val=pmic_bank1_config_interface(0x5a,0xcd,0xff,0x0);
	ret_val=pmic_bank1_config_interface(0x5a,0xc1,0xff,0x0);
	ret_val=pmic_bank1_config_interface(0x5b,0x0d,0xff,0x0);
	mdelay(t_d1);
	ret_val=pmic_bank1_config_interface(0x5b,0x0c,0xff,0x0);
	mdelay(t_d2-t_d1);
	ret_val=pmic_bank1_config_interface(0x5b,0x3c,0xff,0x0);
	mdelay(t_d3-t_d2);
	//-------------------------------------------------------trim L
	ReadValue_sum = 0;
	for(i=0 ; i<read_times ; i++)
	{
		ret_val=pmic_bank1_read_interface(0x4a,&ReadValue,0x1,6);
		ReadValue_sum += ReadValue;
		print("[spk_amp_offset_sw_trim_L_1] ReadValue=%d\n", ReadValue);
	}
	if(ReadValue_sum > (read_times/2))	
		a_val = 0;
	else	
		a_val = 1;
		
	ret_val=pmic_bank1_config_interface(0x49,a_val,0x1,5);
	mdelay(1);
	ret_val=pmic_bank1_config_interface(0x5b,0xbc,0xff,0x0);
	for(counter=0 ; counter<32 ; counter++)
	{
		ret_val=pmic_bank1_config_interface(0x49,counter,0x1F,0);
	
		mdelay(t_d4);
		
		ReadValue_sum = 0;
		for(i=0 ; i<read_times ; i++)
		{
			ret_val=pmic_bank1_read_interface(0x4a,&ReadValue,0x1,6);
			ReadValue_sum += ReadValue;
			print("[spk_amp_offset_sw_trim_L_2] ReadValue=%d\n", ReadValue);
		}
		if(ReadValue_sum > (read_times/2))	
			b_val = 0;
		else	
			b_val = 1;

		print("[spk_amp_offset_sw_trim_L] counter=%d, a_val=%d, b_val=%d\n", counter, a_val, b_val);

		if(b_val != a_val)
		{
			break;
		}		
	}
	mdelay(3);
	ret_val=pmic_bank1_config_interface(0x5b,0x3c,0xff,0x0);
	ret_val=pmic_bank1_config_interface(0x5a,0x41,0xff,0x0);
	//-------------------------------------------------------trim R
	ReadValue_sum = 0;
	for(i=0 ; i<read_times ; i++)
	{
		ret_val=pmic_bank1_read_interface(0x50,&ReadValue,0x1,6);
		ReadValue_sum += ReadValue;
		print("[spk_amp_offset_sw_trim_R_1] ReadValue=%d\n", ReadValue);
	}
	if(ReadValue_sum > (read_times/2))	
		a_val = 0;
	else	
		a_val = 1;
	
	ret_val=pmic_bank1_config_interface(0x4F,a_val,0x1,5);
	mdelay(1);
	ret_val=pmic_bank1_config_interface(0x5b,0x7c,0xff,0x0);
	for(counter=0 ; counter<32 ; counter++)
	{
		ret_val=pmic_bank1_config_interface(0x4F,counter,0x1F,0);
	
		mdelay(t_d4);
		
		ReadValue_sum = 0;
		for(i=0 ; i<read_times ; i++)
		{
			ret_val=pmic_bank1_read_interface(0x50,&ReadValue,0x1,6);
			ReadValue_sum += ReadValue;
			print("[spk_amp_offset_sw_trim_R_2] ReadValue=%d\n", ReadValue);
		}
		if(ReadValue_sum > (read_times/2))	
			b_val = 0;
		else	
			b_val = 1;

		print("[spk_amp_offset_sw_trim_R] counter=%d, a_val=%d, b_val=%d\n", counter, a_val, b_val);

		if(b_val != a_val)
		{
			break;
		}		
	}
	mdelay(3);
	ret_val=pmic_bank1_config_interface(0x5b,0x3c,0xff,0x0);
	ret_val=pmic_bank1_config_interface(0x5a,0x00,0xff,0x0);
	//-------------------------------------------------------End
	ret_val=pmic_bank1_config_interface(0x56,0x00,0xff,0x0);

	ret_val=pmic_bank1_read_interface(0x49,&ReadValue,0xFF,0);
	print("[spk_amp_offset_sw_trim] Reg[0x49]=0x%x, ", ReadValue);
	ret_val=pmic_bank1_read_interface(0x4F,&ReadValue,0xFF,0);
	print("Reg[0x4F]=0x%x\n", ReadValue);
}
	
//==============================================================================
// PMIC6329 Init Code
//==============================================================================
U32 pmic6329_init (void)
{
    U32 ret_code = PMIC_TEST_PASS;
	int ret_val=0;
	int reg_val=0;
	int reg_val_1=0;
	int reg_val_2=0;
	int volt_bias=0;
	int volt=0;
	int volt_set=0;

	print("\n[pmic6329_init] Start...................\n");

	//Put init setting here

	//Use E1 default design
	print("\n[pmic6329_init] Use E1 default design ...................\n");
	ret_val=pmic_bank1_config_interface(0x3C,0x10,0xFF,0x0);
	//check
        ret_val=pmic_bank1_read_interface(0x3C,&reg_val,0xFF,0x0);
        print("\n[PreLoader_pmic6329_init] Bank1[0x3C]=0x%x\n", reg_val);

	print("\n[PreLoader_pmic6329_init] Set no I2C de-glich\n");
	ret_val=pmic_bank1_config_interface(BANK1_I2C_CON0,0x0,0xFF,0x0);
	ret_val=pmic_bank1_config_interface(BANK1_I2C_CON1,0x0,0xFF,0x0);

	//check
	ret_val=pmic_bank1_read_interface(BANK1_I2C_CON0,&reg_val,0xFF,0x0);
	print("\n[PreLoader_pmic6329_init] Bank1[0x%x]=0x%x\n", BANK1_I2C_CON0, reg_val);
	ret_val=pmic_bank1_read_interface(BANK1_I2C_CON1,&reg_val,0xFF,0x0);
	print("\n[PreLoader_pmic6329_init] Bank1[0x%x]=0x%x\n", BANK1_I2C_CON1, reg_val);

#ifdef PMIC_SW_TEST_MODE_6326
    ret_code = pmic6329_int_ctrl_enable(mt6329_BANK0_SLAVE_ADDR_WRITE, PMIC_INIT_CTRL_1, INT1_EN_ALL, KAL_FALSE);
    if(ret_code != I2C_OK)
    {
        return ret_code;
    }
    ret_code = pmic6329_int_ctrl_enable(mt6329_BANK0_SLAVE_ADDR_WRITE, PMIC_INIT_CTRL_2, INT2_EN_ALL, KAL_TRUE);
    if(ret_code != I2C_OK)
    {
        return ret_code;
    }
    ret_code = pmic6329_int_ctrl_enable(mt6329_BANK0_SLAVE_ADDR_WRITE, PMIC_INIT_CTRL_3, INT3_EN_ALL, KAL_FALSE);
    if(ret_code != I2C_OK)
    {
        return ret_code;
    }
    ret_code = pmic6329_int_ctrl_enable(mt6329_BANK0_SLAVE_ADDR_WRITE, PMIC_INIT_CTRL_3, INT_EN_CHRDET, KAL_TRUE);
    if(ret_code != I2C_OK)
    {
        return ret_code;
    }
#endif	

	//pmic6329_get_version();
	//pmic_IsUsbCableIn();
	//mt_detect_powerkey();
	//mt_charger_type_detection();
	//hw_set_cc(650);

	if( get_pmic6329_chip_version() == PMIC6329_E1_CID_CODE )
	{
		printf("[pmic6329_init] PMIC6329_E1_CID_CODE\n");
	}
	else if( get_pmic6329_chip_version() == PMIC6329_E2_CID_CODE )
	{
		printf("[pmic6329_init] PMIC6329_E2_CID_CODE\n");

		//print("\n[pmic6329_init] Set E2 design ...................\n");
		//ret_val=pmic_bank1_config_interface(0x3C,0x00,0xFF,0x0);
		ret_val=pmic_config_interface(0x4F,0x00,0xFF,0x0);
		//check
		//ret_val=pmic_bank1_read_interface(0x3C,&reg_val,0xFF,0x0);
		//print("\n[PreLoader_pmic6329_init] Bank1[0x3C]=0x%x\n", reg_val);
		ret_val=pmic_read_interface(0x4F,&reg_val,0xFF,0x0);
	        print("\n[PreLoader_pmic6329_init] Bank0[0x4F]=0x%x\n", reg_val);
	}
	else
	{
		printf("[pmic6329_init] Unknown Chip Version\n");
	}
#if 1
	//FG HW init	
	//(1)	i2c_write (0x60, 0xC8, 0x01); // Enable VA2
	ret_val=pmic_config_interface(0xC8, 0x1, 0xFF, 0x0);
	//(2)	i2c_write (0x61, 0x15, 0x00); // Enable FGADC clock for digital
	ret_val=pmic_bank1_config_interface(0x15, 0x0, 0xFF, 0x0);
	//(3)	i2c_write (0x61, 0x69, 0x28); // Set current mode, auto-calibration mode and 32KHz clock source
	ret_val=pmic_bank1_config_interface(0x69, 0x28, 0xFF, 0x0);
	//(4)	i2c_write (0x61, 0x69, 0x29); // Enable FGADC
	ret_val=pmic_bank1_config_interface(0x69, 0x29, 0xFF, 0x0);
	printf("[pmic6329_init] FG HW init\n");
#endif
	//spk_amp_offset_sw_trim();

if (platform_chip_ver() >= CHIP_6577_E1)
{	
    if ( (DRV_Reg32(HW_RESV) & (0x1 << 23)) && ((DRV_Reg32(HW_RESV) & (0x1 << 20)) == 0) )    
    {     
        printf("[pmic6329_init] 1-1 !\n");
        ret_val=pmic_config_interface(0x44,0x19,0xFF,0x0);
        ret_val=pmic_config_interface(0x45,0x19,0xFF,0x0);
        ret_val=pmic_config_interface(0x46,0x19,0xFF,0x0);
        ret_val=pmic_config_interface(0x47,0x19,0xFF,0x0);
        ret_val=pmic_config_interface(0x48,0x19,0xFF,0x0);
        ret_val=pmic_config_interface(0x49,0x19,0xFF,0x0);
        ret_val=pmic_config_interface(0x4A,0x19,0xFF,0x0);
        ret_val=pmic_config_interface(0x8A,0x19,0xFF,0x0);
        ret_val=pmic_config_interface(0x8B,0x19,0xFF,0x0);  
    }
    else
    {
    	  printf("[pmic6329_init] 1-2 !\n");
    	  
        if ((DRV_Reg32(HW_RESV) & (0x1 << 21))) 
            volt_bias = 2; 
        else if ((DRV_Reg32(HW_RESV) & (0x1 << 22)))
            volt_bias = 1; 
		else
            volt_bias = 0;
            
        volt = (DRV_Reg32(HW_RESV) & 0xE000) >> 13;
        
        if (volt == 0x0)            volt_set = 0x17+volt_bias;
        else if (volt == 0x1)       volt_set = 0x16+volt_bias;
        else if (volt == 0x2)       volt_set = 0x15+volt_bias;
        else if (volt == 0x3)       volt_set = 0x14+volt_bias;
        else if (volt == 0x4)       volt_set = 0x13+volt_bias;
        else if (volt == 0x5)       volt_set = 0x18+volt_bias;
        else if (volt == 0x6)       volt_set = 0x19+volt_bias;
        else if (volt == 0x7)       volt_set = 0x19+volt_bias;
        else                        volt_set = 0x13+volt_bias;
        
        ret_val=pmic_config_interface(0x44,volt_set,0xFF,0x0);
        ret_val=pmic_config_interface(0x45,volt_set,0xFF,0x0);
        ret_val=pmic_config_interface(0x46,volt_set,0xFF,0x0);
        ret_val=pmic_config_interface(0x47,volt_set,0xFF,0x0);
        ret_val=pmic_config_interface(0x48,volt_set,0xFF,0x0);
        ret_val=pmic_config_interface(0x49,volt_set,0xFF,0x0);
        ret_val=pmic_config_interface(0x4A,volt_set,0xFF,0x0);
        ret_val=pmic_config_interface(0x8A,volt_set,0xFF,0x0);
        ret_val=pmic_config_interface(0x8B,volt_set,0xFF,0x0);        
        
        printf("[pmic6329_init] volt_set=%d\n", volt_set);
    }
}
else
{
    //20110823-Juinn-Ting update
    ret_val =pmic_config_interface(0x44,0x16,0x1F,0x0);
    ret_val =pmic_config_interface(0x45,0x16,0x1F,0x0);
    ret_val =pmic_config_interface(0x46,0x16,0x1F,0x0);
    ret_val =pmic_config_interface(0x47,0x16,0x1F,0x0);
    ret_val =pmic_config_interface(0x48,0x16,0x1F,0x0);
    ret_val =pmic_config_interface(0x49,0x16,0x1F,0x0);
    ret_val =pmic_config_interface(0x4A,0x16,0x1F,0x0);
    ret_val =pmic_config_interface(0x8A,0x16,0x1F,0x0);
    ret_val =pmic_config_interface(0x8B,0x16,0x1F,0x0);
    printf("[pmic6329_init] All VPROC and VM12_INT => 1.25V\n");
}	

    hw_check_battery();
    printf("[pmic6329_init] hw_check_battery\n");

    return ret_code;
}

int mt_detect_powerkey(void)
{
#if CFG_FPGA_PLATFORM
    return 1;
#else
	U32 ret_code = I2C_OK;
	U8 chip_slave_address = mt6329_BANK0_SLAVE_ADDR_WRITE;
    U8 cmd = 0x09;
    int cmd_len = 1;
    U8 data = 0xFF;
    int data_len = 1;

	ret_code = pmic6329_i2c_read(chip_slave_address, &cmd, cmd_len, &data, data_len);

	if (data & (BANK_0_PWRKEY_DEB_MASK << BANK_0_PWRKEY_DEB_SHIFT)){     
        printf("[PreLoader_mt_detect_powerkey] Release\n");
        //return 1;
        return 0;
    }else{
        printf("[PreLoader_mt_detect_powerkey] Press\n");
        //return 0;
        return 1;
    }
#endif
}


void hw_set_cc(int cc_val)
{
#if !CFG_FPGA_PLATFORM
	int ret_val=0;
	int reg_val=0;	
	int i=0;
        int hw_charger_ov_flag=0;

	printf("hw_set_cc: %d\r\n", cc_val);
	//VCDT_HV_VTH, 7V,change to 6.5v
	ret_val=pmic_config_interface(0x22, 0x0A, 0x0F, 0x04); //add for charger ov setting 
	//VCDT_HV_EN=1
	ret_val=pmic_config_interface(0x21, 0x01, 0x01, 0x00); //add for charger ov setting
	//upmu_chr_baton_tdet_en(0);
	//ret_val=pmic_config_interface(0x38, 0x00, 0x01, 0x05);
	//CS_EN=1
	ret_val=pmic_config_interface(0x23, 0x01, 0x01, 0x03);
	//CSDAC_MODE=1
	ret_val=pmic_config_interface(0x38, 0x01, 0x01, 0x02);

        ret_val=pmic_read_interface(0x21,&hw_charger_ov_flag,0x01,0x07);
        if(hw_charger_ov_flag == 1)
        {
             ret_val=pmic_config_interface(0x21, 0x00, 0x01, 0x04);
             printf("[PreLoader_mt_charger_ov]turn off charging \n"); 
              return;
        }

	// CS_VTH
    switch(cc_val){
        case 1600:			
			ret_val=pmic_config_interface(0x25, 0x00, 0xFF, 0x0);
			break;
		case 1500:			
			ret_val=pmic_config_interface(0x25, 0x01, 0xFF, 0x0);
			break;	   
        case 1400:			
			ret_val=pmic_config_interface(0x25, 0x02, 0xFF, 0x0);
			break;
		case 1300:			
			ret_val=pmic_config_interface(0x25, 0x03, 0xFF, 0x0);
			break;
        case 1200:			
			ret_val=pmic_config_interface(0x25, 0x04, 0xFF, 0x0);
			break;
		case 1100:			
			ret_val=pmic_config_interface(0x25, 0x05, 0xFF, 0x0);
			break;
        case 1000:			
			ret_val=pmic_config_interface(0x25, 0x06, 0xFF, 0x0);
			break;
		case 900:			
			ret_val=pmic_config_interface(0x25, 0x07, 0xFF, 0x0);
			break;			
        case 800:			
			ret_val=pmic_config_interface(0x25, 0x08, 0xFF, 0x0);
			break;
		case 700:			
			ret_val=pmic_config_interface(0x25, 0x09, 0xFF, 0x0);
			break;	   
        case 650:			
			ret_val=pmic_config_interface(0x25, 0x0A, 0xFF, 0x0);
			break;
		case 550:			
			ret_val=pmic_config_interface(0x25, 0x0B, 0xFF, 0x0);
			break;
        case 450:			
			ret_val=pmic_config_interface(0x25, 0x0C, 0xFF, 0x0);
			break;
		case 400:			
			ret_val=pmic_config_interface(0x25, 0x0D, 0xFF, 0x0);
			break;
        case 200:			
			ret_val=pmic_config_interface(0x25, 0x0E, 0xFF, 0x0);
			break;
		case 70:			
			ret_val=pmic_config_interface(0x25, 0x0F, 0xFF, 0x0);
			break;			
		default:
			dbg_print("hw_set_cc: argument invalid!!\r\n");
			break;
	}

	//upmu_chr_csdac_dly(0x4);				// CSDAC_DLY
	ret_val=pmic_config_interface(0x36, 0x04, 0x07, 0x0);
	//upmu_chr_csdac_stp(0x1);				// CSDAC_STP
	ret_val=pmic_config_interface(0x36, 0x01, 0x07, 0x4);
	//upmu_chr_csdac_stp_inc(0x1);				// CSDAC_STP_INC
	ret_val=pmic_config_interface(0x35, 0x01, 0x07, 0x0);
	//upmu_chr_csdac_stp_dec(0x2);			// CSDAC_STP_DEC
	ret_val=pmic_config_interface(0x35, 0x02, 0x07, 0x4);
	//upmu_chr_chrwdt_td(0x0);				// CHRWDT_TD, 4s, check me
	ret_val=pmic_config_interface(0x2E, 0x00, 0x0F, 0x0);
	//upmu_chr_chrwdt_int_en(1);				// CHRWDT_INT_EN, check me
	ret_val=pmic_config_interface(0x30, 0x01, 0x01, 0x0);
	//upmu_chr_chrwdt_en(1); 					// CHRWDT_EN, check me
	ret_val=pmic_config_interface(0x2E, 0x01, 0x01, 0x4);
	//upmu_chr_chrwdt_flag_wr(1);				// CHRWDT_FLAG, check me
	ret_val=pmic_config_interface(0x30, 0x01, 0x01, 0x1);
	//upmu_chr_csdac_enable(1);				// CSDAC_EN
	ret_val=pmic_config_interface(0x21, 0x01, 0x01, 0x3);
	//upmu_chr_enable(1);						// CHR_EN
	ret_val=pmic_config_interface(0x21, 0x01, 0x01, 0x4);

	for(i=0x21 ; i<=0x3D ; i++)	
	{		
		ret_val=pmic_read_interface(i,&reg_val,0xFF,0x0);    	
		print("[PreLoader] Bank0[0x%x]=0x%x\n", i, reg_val);	
	}

	printf("hw_set_cc: done\r\n");

	// DE debug
#if 0  
	ret_val=pmic_config_interface(0x2B, 0x01, 0x01, 0x4);

	ret_val=pmic_config_interface(0x2C, 0x01, 0x3F, 0x0);
	ret_val=pmic_read_interface(0x2C,&reg_val,0xFF,0x0);    	
	print("[PreLoader] Bank0[0x2C]=0x%x\n", reg_val);
	ret_val=pmic_read_interface(0x2B,&reg_val,0xFF,0x0);    	
	print("[PreLoader] Bank0[0x2B]=0x%x\n", reg_val);
	printf("--------------------------------------\r\n");

	ret_val=pmic_config_interface(0x2C, 0x02, 0x3F, 0x0);
	ret_val=pmic_read_interface(0x2C,&reg_val,0xFF,0x0);    	
	print("[PreLoader] Bank0[0x2C]=0x%x\n", reg_val);
	ret_val=pmic_read_interface(0x2B,&reg_val,0xFF,0x0);    	
	print("[PreLoader] Bank0[0x2B]=0x%x\n", reg_val);
	printf("--------------------------------------\r\n");

	ret_val=pmic_config_interface(0x2C, 0x03, 0x3F, 0x0);
	ret_val=pmic_read_interface(0x2C,&reg_val,0xFF,0x0);    	
	print("[PreLoader] Bank0[0x2C]=0x%x\n", reg_val);
	ret_val=pmic_read_interface(0x2B,&reg_val,0xFF,0x0);    	
	print("[PreLoader] Bank0[0x2B]=0x%x\n", reg_val);
	printf("--------------------------------------\r\n");

	ret_val=pmic_config_interface(0x2C, 0x06, 0x3F, 0x0);
	ret_val=pmic_read_interface(0x2C,&reg_val,0xFF,0x0);    	
	print("[PreLoader] Bank0[0x2C]=0x%x\n", reg_val);
	ret_val=pmic_read_interface(0x2B,&reg_val,0xFF,0x0);    	
	print("[PreLoader] Bank0[0x2B]=0x%x\n", reg_val);
	printf("--------------------------------------\r\n");

	ret_val=pmic_config_interface(0x2C, 0x07, 0x3F, 0x0);
	ret_val=pmic_read_interface(0x2C,&reg_val,0xFF,0x0);    	
	print("[PreLoader] Bank0[0x2C]=0x%x\n", reg_val);
	ret_val=pmic_read_interface(0x2B,&reg_val,0xFF,0x0);    	
	print("[PreLoader] Bank0[0x2B]=0x%x\n", reg_val);
	printf("--------------------------------------\r\n");

	//while(1);
#endif	
#endif

}

int hw_check_battery(void)
{
	U32 ret_code = I2C_OK;
	U8 chip_slave_address = mt6329_BANK0_SLAVE_ADDR_WRITE;
    U8 cmd = 0x28;
    int cmd_len = 1;
    U8 data = 0xFF;
    int data_len = 1;
   
	int ret_val=0;
	int reg_val=0;
   
    if( get_pmic6329_chip_version() == PMIC6329_E1_CID_CODE )
    {
        return 1;
    }

	ret_val=pmic_config_interface(0x38, 0x01, 0x01, 0x5); //BATON_TDET_EN=1					
	ret_val=pmic_config_interface(0xE9, 0x01, 0x01, 0x1); //RG_BUF_PWD_B=1

    ret_code = pmic6329_i2c_read(chip_slave_address, &cmd, cmd_len, &data, data_len);

	if (data & (BANK_0_BATON_UNDET_MASK << BANK_0_BATON_UNDET_SHIFT)){     		

		ret_val=pmic_read_interface(0x38,&reg_val,0xFF,0x0); print("[hw_check_battery+] Bank0[0x38]=0x%x\n", reg_val);
		ret_val=pmic_read_interface(0xE9,&reg_val,0xFF,0x0); print("[hw_check_battery+] Bank0[0xE9]=0x%x\n", reg_val);
		
        printf("[hw_check_battery] No Battery\n");
		
		//ret_val=pmic_config_interface(0x38, 0x00, 0x01, 0x5); //BATON_TDET_EN=0						
		//ret_val=pmic_config_interface(0xE9, 0x00, 0x01, 0x1); //RG_BUF_PWD_B=0

		ret_val=pmic_read_interface(0x38,&reg_val,0xFF,0x0); print("[hw_check_battery-] Bank0[0x38]=0x%x\n", reg_val);
		ret_val=pmic_read_interface(0xE9,&reg_val,0xFF,0x0); print("[hw_check_battery-] Bank0[0xE9]=0x%x\n", reg_val);		
		
        return 0;
		
    }else{

		ret_val=pmic_read_interface(0x38,&reg_val,0xFF,0x0); print("[hw_check_battery+] Bank0[0x38]=0x%x\n", reg_val);
		ret_val=pmic_read_interface(0xE9,&reg_val,0xFF,0x0); print("[hw_check_battery+] Bank0[0xE9]=0x%x\n", reg_val);	
	
        printf("[hw_check_battery] Battery exist\n");
		
		//ret_val=pmic_config_interface(0x38, 0x00, 0x01, 0x5); //BATON_TDET_EN=0						
		//ret_val=pmic_config_interface(0xE9, 0x00, 0x01, 0x1); //RG_BUF_PWD_B=0

		ret_val=pmic_read_interface(0x38,&reg_val,0xFF,0x0); print("[hw_check_battery-] Bank0[0x38]=0x%x\n", reg_val);
		ret_val=pmic_read_interface(0xE9,&reg_val,0xFF,0x0); print("[hw_check_battery-] Bank0[0xE9]=0x%x\n", reg_val);
	
		pl_hw_ulc_det();
		
        return 1;
    }
	
}

void pl_charging(int en_chr)
{
	int ret_val=0;
	int reg_val=0;
	int i=0;
	
	if(en_chr == 1)
	{
		printf("[pl_charging] enable\n");
	
		hw_set_cc(450);

		//USBDL set 1
		ret_val=pmic_config_interface(0x31, 0x01, 0x01, 0x3);		
	}
	else
	{
		printf("[pl_charging] disable\n");
	
		//USBDL set 0
		ret_val=pmic_config_interface(0x31, 0x00, 0x01, 0x3);
	
		//upmu_chr_enable(0);						// CHR_EN
		ret_val=pmic_config_interface(0x21, 0x00, 0x01, 0x4);		
	}

	for(i=0x21 ; i<=0x3D ; i++)	
	{		
		ret_val=pmic_read_interface(i,&reg_val,0xFF,0x0);    	
		print("[pl_charging] Bank0[0x%x]=0x%x\n", i, reg_val);	
	}
	
}

void pl_kick_chr_wdt(void)
{
	int ret_val=0;
	
	//upmu_chr_chrwdt_td
	ret_val=pmic_config_interface(0x2E, 0x03, 0x0F, 0x0);
	//upmu_chr_chrwdt_int_en(1);				// CHRWDT_INT_EN, check me
	ret_val=pmic_config_interface(0x30, 0x01, 0x01, 0x0);
	//upmu_chr_chrwdt_en(1); 					// CHRWDT_EN, check me
	ret_val=pmic_config_interface(0x2E, 0x01, 0x01, 0x4);
	//upmu_chr_chrwdt_flag_wr(1);				// CHRWDT_FLAG, check me
	ret_val=pmic_config_interface(0x30, 0x01, 0x01, 0x1);

	//printf("[pl_kick_chr_wdt] done\n");
}

void pl_close_pre_chr_led(void)
{
	int ret_val=0;	
	ret_val=pmic_config_interface(0x37, 0x00, 0x01, 0x6);
	printf("[pmic6329_init] Close pre-chr LED\n");
}

void pl_hw_ulc_det(void)
{
	int ret_val=0;
	
	//upmu_chr_ulc_det_en(1);			// RG_ULC_DET_EN=1
	ret_val=pmic_config_interface(0x38, 0x01, 0x01, 0x07);
	//upmu_chr_low_ich_db(1);			// RG_LOW_ICH_DB=000001'b
	ret_val=pmic_config_interface(0x37, 0x01, 0x3F, 0x00);
}

U32 pmic6329_detect_homekey(void)
{
	U32 ret_code = I2C_OK;
	U8 chip_slave_address = mt6329_BANK0_SLAVE_ADDR_WRITE;
        U8 cmd = 0x0C;
        int cmd_len = 1;
        U8 data = 0xFF;
        int data_len = 1;

	ret_code = pmic6329_i2c_read(chip_slave_address, &cmd, cmd_len, &data, data_len);

	if (data & (BANK_0_HOMEKEY_DEB_MASK << BANK_0_HOMEKEY_DEB_SHIFT)){     
              print("[mt6577_detect_homekey] Release\n");
              return 0;
        }else{
              print("[mt6577_detect_homekey] Press\n");
              return 1;
        }
}
