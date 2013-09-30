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
#include <platform/mt_typedefs.h>
#include <platform/i2c.h>
#include <debug.h>

/*-----------------------------------------------------------------------
 * Set I2C Speend interface:    Set internal I2C speed, 
 *                              Goal is that get sample_cnt_div and step_cnt_div
 *   clock: Depends on the current MCU/AHB/APB clock frequency   
 *   mode:  ST_MODE. (fixed setting for stable I2C transaction)
 *   khz:   MAX_ST_MODE_SPEED. (fixed setting for stable I2C transaction) 
 *
 *   Returns: ERROR_CODE
 */
u32 i2c_v1_set_speed (u8 channel,unsigned long clock, I2C_SPD_MODE mode, unsigned long khz)
{
    u32 ret_code = I2C_OK;
    
    CHANNEL_BASE;
    unsigned short sample_cnt_div, step_cnt_div;
    unsigned short max_step_cnt_div = (mode == HS_MODE) ? MAX_HS_STEP_CNT_DIV : MAX_STEP_CNT_DIV;
    unsigned long tmp, sclk;
    
    {
        unsigned long diff, min_diff = I2C_CLK_RATE;
        unsigned short sample_div = MAX_SAMPLE_CNT_DIV;
        unsigned short step_div = max_step_cnt_div;
        for (sample_cnt_div = 1; sample_cnt_div <= MAX_SAMPLE_CNT_DIV; sample_cnt_div++) {
        
            for (step_cnt_div = 1; step_cnt_div <= max_step_cnt_div; step_cnt_div++) {
                sclk = (clock >> 1) / (sample_cnt_div * step_cnt_div);
                if (sclk > khz) 
                    continue;
                diff = khz - sclk;
                
                if (diff < min_diff) {
                    min_diff = diff;
                    sample_div = sample_cnt_div;
                    step_div   = step_cnt_div;
                }
            }
        }
        sample_cnt_div = sample_div;
        step_cnt_div   = step_div;
    }

    sclk = clock / (2 * sample_cnt_div * step_cnt_div);
    if (sclk > khz) {
	  ret_code = I2C_SET_SPEED_FAIL_OVER_SPEED;
        return ret_code;
    }

    step_cnt_div--;
    sample_cnt_div--;

    if (mode == HS_MODE) {
        tmp  = DRV_Reg16(MT_I2C_HS) & ((0x7 << 12) | (0x7 << 8));
        tmp  = (sample_cnt_div & 0x7) << 12 | (step_cnt_div & 0x7) << 8 | tmp;
        DRV_WriteReg16(MT_I2C_HS,tmp);
		dprintf(INFO,"HS_MODE\n");
        I2C_SET_HS_MODE(1);
    }
    else {
        tmp  = DRV_Reg16(MT_I2C_TIMING) & ~((0x7 << 8) | (0x3f << 0));
        tmp  = (0/*sample_cnt_div*/ & 0x7) << 8 | (5/*step_cnt_div*/ & 0x3f) << 0 | tmp;
        DRV_WriteReg16(MT_I2C_TIMING,tmp);
		dprintf(INFO,"not HS_MODE\n");
        I2C_SET_HS_MODE(0);
    }
    
    dprintf(INFO,"[i2c_set_speed] Set sclk to %ld khz (orig: %ld khz)\n", sclk, khz);
    dprintf(INFO,"[i2c_set_speed] I2C Timing parameter sample_cnt_div(%d),  step_cnt_div(%d)\n", sample_cnt_div, step_cnt_div);
    
    return ret_code;
}

static u32 i2c_channel_init(u8 channel)
{
	u32 ret_code = I2C_OK;
    CHANNEL_BASE;
	
    /* Power On I2C Duel */
    //PDN_Power_CONA_DOWN(PDN_PERI_I2C, KAL_FALSE); // wait PLL API release
    dprintf(INFO,"\n[i2c%d_init] Start...................\n",channel);

    /* Reset the HW I2C module */
    I2C_SOFTRESET;

    /* Set I2C control register */
    I2C_SET_TRANS_CTRL(ACK_ERR_DET_EN | CLK_EXT);

    /* Sset I2C speed mode */
    ret_code = i2c_v1_set_speed(channel,I2C_CLK_RATE, ST_MODE, MAX_ST_MODE_SPEED);
    if( ret_code !=  I2C_OK)
    {
        dprintf(INFO,"[i2c%d_init] i2c_v1_set_speed error (%d)\n", channel,ret_code);
        return ret_code;
    }

    /* Clear Interrupt status */ 
    I2C_CLR_INTR_STATUS(I2C_TRANSAC_COMP | I2C_ACKERR | I2C_HS_NACKERR);

    /* Double Reset the I2C START bit*/
    DRV_WriteReg32(MT_I2C_START,0);

    dprintf(INFO,"[i2c%d_init] Done\n",channel);
    
    return ret_code;
	
}

/*-----------------------------------------------------------------------
* Initializa the HW I2C module
*    Returns: ERROR_CODE 
*/
u32 i2c_v1_init () 
{
    i2c_channel_init(I2C0);
    i2c_channel_init(I2C1);
//    i2c_channel_init(I2C2);
    return I2C_OK;
}

/*-----------------------------------------------------------------------
* De-Initializa the HW I2C module
*    Returns: ERROR_CODE 
*/
u32 i2c_v1_deinit (u8 channel)
{
    u32 ret_code = I2C_OK;
 	CHANNEL_BASE;

    /* Reset the HW I2C module */
    I2C_SOFTRESET;

    dprintf(INFO,"[i2c_deinit] Done\n");
    
    return ret_code;
}

/*-----------------------------------------------------------------------
 * Read interface: Read bytes
 *   chip:    I2C chip address, range 0..127
 *              e.g. Smart Battery chip number is 0xAA
 *   buffer:  Where to read/write the data (device address is regarded as data)
 *   len:     How many bytes to read/write
 *
 *   Returns: ERROR_CODE
 */
u32 i2c_v1_read(u8 channel,u8 chip, u8 *buffer, int len)
{
    u32 ret_code = I2C_OK;
    u8 *ptr = buffer;
    unsigned short status;    
    int ret = len;
    //long tmo;
    //u32 timeout_ms = I2C_TIMEOUT_TH;
    //u32 timeout_ms = 4000; // 4s
    //u32 start_tick=0, timeout_tick=0;
	unsigned int time_out_val=0;
	CHANNEL_BASE;
    
    /* CHECKME. mt65xx doesn't support len = 0. */
    if (!len) {
        dprintf(INFO,"[i2c_read] I2C doesn't support len = 0.\n");
        return I2C_READ_FAIL_ZERO_LENGTH;
    }

    /* for read, bit 0 is to indicate read REQ or write REQ */
    chip = (chip | 0x1);

    /* control registers */
    I2C_SET_SLAVE_ADDR(chip);
    I2C_SET_TRANS_LEN(len);
    I2C_SET_TRANSAC_LEN(1);
    I2C_SET_INTR_MASK(I2C_HS_NACKERR | I2C_ACKERR | I2C_TRANSAC_COMP);
    I2C_FIFO_CLR_ADDR;

    I2C_SET_TRANS_CTRL(ACK_ERR_DET_EN | CLK_EXT | STOP_FLAG);

    /* start trnasfer transaction */
    I2C_START_TRANSAC;

    /* set timer to calculate time avoid timeout without any reaction */
    //tmo = get_timer(0);    
#if 0    
    timeout_tick = gpt4_time2tick_ms(timeout_ms);
    start_tick = gpt4_get_current_tick();
#endif	

    /* polling mode : see if transaction complete */
    while (1) 
    {
        status = I2C_INTR_STATUS;

        if ( status & I2C_TRANSAC_COMP && (!I2C_FIFO_IS_EMPTY) )
        {
            ret = 0;
            ret_code = I2C_OK; // 0
            break;
        }
        else if ( status & I2C_HS_NACKERR) 
        {
            ret = 1;
            ret_code = I2C_READ_FAIL_HS_NACKERR;
            dprintf(INFO,"[i2c_read] transaction NACK error (%x)\n", status);
            break;
        }
        else if ( status & I2C_ACKERR) 
        {
            ret = 2;
            ret_code = I2C_READ_FAIL_ACKERR;
            dprintf(INFO,"[i2c_read] transaction ACK error (%x)\n", status);
            break;
        }
#if 1
        //else if (get_timer(tmo) > I2C_TIMEOUT_TH /* ms */) {           
        else if (time_out_val > 100000) { 
            ret = 3;
            ret_code = I2C_READ_FAIL_TIMEOUT;
            dprintf(INFO,"[i2c_read] transaction timeout:%d\n", time_out_val);
            break;
        }
		time_out_val++;
#endif
    }

    I2C_CLR_INTR_STATUS(I2C_TRANSAC_COMP | I2C_ACKERR | I2C_HS_NACKERR);
    
    if (!ret) 
    {
        while (len--) 
        {   
            I2C_READ_BYTE(*ptr);
            dprintf(INFO,"[i2c_read] read byte = 0x%x\n", *ptr);
            ptr++;
        }
    }

    /* clear bit mask */
    I2C_CLR_INTR_MASK(I2C_HS_NACKERR | I2C_ACKERR | I2C_TRANSAC_COMP);

    dprintf(INFO,"[i2c_read] Done\n");
    
    return ret_code;
}

/*-----------------------------------------------------------------------
 * Read interface: Write bytes
 *   chip:    I2C chip address, range 0..127
 *              e.g. Smart Battery chip number is 0xAA
 *   buffer:  Where to read/write the data (device address is regarded as data)
 *   len:     How many bytes to read/write
 *
 *   Returns: ERROR_CODE
 */
u32 i2c_v1_write (u8 channel,u8 chip, u8 *buffer, int len)
{
    u32 ret_code = I2C_OK;
    u8 *ptr = buffer;
    int ret = len;
    //long tmo;
    unsigned short status;
    //u32 timeout_ms = I2C_TIMEOUT_TH;
    //u32 start_tick=0, timeout_tick=0;
	unsigned int time_out_val=0;
    CHANNEL_BASE;
    /* CHECKME. mt65xx doesn't support len = 0. */
    if (!len)
    { 
		dprintf(INFO,"[i2c_write] I2C doesn't support len = 0.\n");
        return I2C_WRITE_FAIL_ZERO_LENGTH;
    }

    /* bit 0 is to indicate read REQ or write REQ */
    chip = (chip & ~0x1);

    /* control registers */
    I2C_SET_SLAVE_ADDR(chip);
    I2C_SET_TRANS_LEN(len);
    I2C_SET_TRANSAC_LEN(1);
    I2C_SET_INTR_MASK(I2C_HS_NACKERR | I2C_ACKERR | I2C_TRANSAC_COMP);
    I2C_FIFO_CLR_ADDR;

    I2C_SET_TRANS_CTRL(ACK_ERR_DET_EN | CLK_EXT | STOP_FLAG);

    /* start to write data */
    while (len--) {
        I2C_WRITE_BYTE(*ptr);
        //dbg_print("[i2c_write] write byte = 0x%x\n", *ptr);
        ptr++;
    }

    /* start trnasfer transaction */
    I2C_START_TRANSAC;

    /* set timer to calculate time avoid timeout without any reaction */
    //tmo = get_timer(0);
#if 0    
    timeout_tick = gpt4_time2tick_ms(timeout_ms);
    start_tick = gpt4_get_current_tick();
#endif	

    /* polling mode : see if transaction complete */
    while (1) {
        status = I2C_INTR_STATUS;
		//dprintf(INFO,"new status=%d\n",status);
/*
        if ( status & I2C_TRANSAC_COMP) {
            ret = 0;
            ret_code = I2C_OK;
			dprintf(INFO,"[i2c_write] i2c transaction complate\n");
            break;
        }
        else */if ( status & I2C_HS_NACKERR) {
            ret = 1;
            ret_code = I2C_WRITE_FAIL_HS_NACKERR;			
            dprintf(INFO,"[i2c_write] transaction NACK error\n");
            break;
        }
        else if ( status & I2C_ACKERR) {
            ret = 2;
            ret_code = I2C_WRITE_FAIL_ACKERR;
            dprintf(INFO, "[i2c0_write] transaction ACK error\n");
            break;
        }
	else if ( status & I2C_TRANSAC_COMP) {
            ret = 0;
            ret_code = I2C_OK;
			dprintf(INFO,"[i2c_write] i2c transaction complate\n");
            break;
        }
#if 1
        //else if (get_timer(tmo) > I2C_TIMEOUT_TH /* ms */ ) {
        else if (time_out_val > 100000) {
            ret = 3;
            ret_code	= I2C_WRITE_FAIL_TIMEOUT;		
            dprintf(INFO,"[i2c_write] transaction timeout:%d\n", time_out_val);
            break;
        }
		time_out_val++;
#endif	
    }

    I2C_CLR_INTR_STATUS(I2C_TRANSAC_COMP | I2C_ACKERR | I2C_HS_NACKERR);

    /* clear bit mask */
    I2C_CLR_INTR_MASK(I2C_HS_NACKERR | I2C_ACKERR | I2C_TRANSAC_COMP);

    return ret_code;
}



