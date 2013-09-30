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

#include "typedefs.h"
#include "platform.h"
#include "boot_device.h"
#include "nand_common_inter.h"
#include "mmc_common_inter.h"

#include "uart.h"
#include "nand.h"
#include "pll.h"
#include "nand.h"
#include "mt_i2c.h"
#include "mt_rtc.h"
#include "mt_emi.h"
#include "mt_pmic6329.h"
#include "mt_cpu_power.h"
#include "es_reg.h" 
#include "mtk_wdt.h"
#include "ram_console.h"
#include "cust_sec_ctrl.h"
#include "gpio.h"

/*============================================================================*/
/* CONSTAND DEFINITIONS                                                       */
/*============================================================================*/
#define MOD "[PLFM]"

/*============================================================================*/
/* GLOBAL VARIABLES                                                           */
/*============================================================================*/
unsigned int sys_stack[CFG_SYS_STACK_SZ >> 2];
const unsigned int sys_stack_sz = CFG_SYS_STACK_SZ;
boot_mode_t g_boot_mode;
boot_dev_t g_boot_dev;
meta_com_t g_meta_com_type = META_UNKNOWN_COM;
u32 g_meta_com_id = 0;
boot_reason_t g_boot_reason;
ulong g_boot_time; 

/*============================================================================*/
/* EXTERNAL FUNCTIONS                                                         */
/*============================================================================*/
static u32 boot_device_init(void)
{
#if CFG_LEGACY_USB_DOWNLOAD
    #if (CFG_BOOT_DEV == BOOTDEV_SDMMC)
    print("[%s] boot device: SDMMC\n", MOD);

    // basic device function
    g_boot_dev.dev_init = mmc_init_device;
    g_boot_dev.dev_read_data = mmc_read_data;
    g_boot_dev.dev_write_data = mmc_write_data;
    g_boot_dev.dev_erase_data = mmc_erase_data;    
    g_boot_dev.dev_wait_ready = mmc_wait_ready;     
    g_boot_dev.dev_find_safe_block = mmc_find_safe_block;             

    // for checksum calculation
    g_boot_dev.dev_chksum_per_file = mmc_chksum_per_file;             
    g_boot_dev.dev_cal_chksum_body = mmc_chksum_body;

    #else
    print("[%s] boot device: NAND\n", MOD);

    // basic device function
    g_boot_dev.dev_init = nand_init_device;
    g_boot_dev.dev_read_data = nand_read_data;
    g_boot_dev.dev_write_data = nand_write_data;
    g_boot_dev.dev_erase_data = nand_erase_data;    
    g_boot_dev.dev_wait_ready = nand_wait_ready;     
    g_boot_dev.dev_find_safe_block = nand_find_safe_block;             

    // for checksum calculation
    g_boot_dev.dev_chksum_per_file = nand_chksum_per_file;             
    g_boot_dev.dev_cal_chksum_body = nand_chksum_body;
    #endif

    /* perform device init */
    return (u32)g_boot_dev.dev_init();
#else
    #if (CFG_BOOT_DEV == BOOTDEV_SDMMC)
    return (u32)mmc_init_device();
    #else
    return (u32)nand_init_device();
    #endif
#endif
}

static void apply_analog_configuration(void)
{
    u32 tmp, mipi_data, vdac_data;
    
    print("%s Apply analog configuration!\n", MOD);
    
    /* Thermal coefficient compensation */

    /* AUXADC calibration */

    /* VDAC output level */
    if (1 == (seclib_get_param(E_HW_RES1, MASK_R_HW_RES1_VDAC_EN) & 0x1)) {
        vdac_data = seclib_get_param(E_HW_RES1, MASK_R_HW_RES1_VDAC_TRIM_VAL) & 0x1F;
        //RG_TRIM_VAL C0007600 [8:12]   
        tmp = DRV_Reg32(0xC0007600);
        DRV_WriteReg32(0xC0007600, (tmp & 0xFFFFE0FF) | (vdac_data << 8));
    }

    /* MIPI resistor and output swing */
    if (1 == ((seclib_get_param(E_HW_RES1, MASK_R_HW_RES1_MIPI_DAT3)&0x800)>>11)) {
        //group 0
        mipi_data = seclib_get_param(E_HW_RES1, MASK_R_HW_RES1_MIPI_DAT0) & 0xF;
        tmp = DRV_Reg32(0xC20A3820);
        DRV_WriteReg32(0xC20A3820, (tmp&0xFFFFF000)|(mipi_data|(mipi_data<<4)|(mipi_data<<8)));
        //group 1
        mipi_data = seclib_get_param(E_HW_RES1, MASK_R_HW_RES1_MIPI_DAT1) & 0xF;
        tmp = DRV_Reg32(0xC20A3844);
        DRV_WriteReg32(0xC20A3844, (tmp&0xFFFFF000)|(mipi_data|(mipi_data<<4)|(mipi_data<<8)));
        //group 2
        mipi_data = seclib_get_param(E_HW_RES1, MASK_R_HW_RES1_MIPI_DAT2) & 0xF;
        tmp = DRV_Reg32(0xC20A3820);
        DRV_WriteReg32(0xC20A3820, (tmp&0xFFFF0FFF)|(mipi_data<<12));        
    }

    /* VBG DC level */
    if (1 == seclib_get_param(E_HW_RES2, MASK_R_HW_RES2_BGR_CTRL_EN)) {
        tmp = DRV_Reg32(0xC0007800);
        tmp &= 0x00000F00;
        tmp |= (seclib_get_param(E_HW_RES2, MASK_R_HW_RES2_BGR_CTRL)<<8);
        DRV_WriteReg32(0xC0007800, tmp);
    }
    if (1 == seclib_get_param(E_HW_RES2, MASK_R_HW_RES2_BGR_RSEL_EN)) {
        tmp = DRV_Reg32(0xC000780C);
        tmp &= 0x0000001F;
        tmp |= seclib_get_param(E_HW_RES2, MASK_R_HW_RES2_BGR_RSEL);
        DRV_WriteReg32(0xC000780C, tmp);
    }
}

#if CFG_FPGA_PLATFORM
int usb_accessory_in(void)
{
    return 1;
}

int usb_cable_in(void)
{
    mt_usb_phy_poweron();
    mt_usb_phy_savecurrent();
    return 1;
}

void platform_vusb_on(void)
{
    return;
}

#else
int usb_accessory_in(void)
{
    int exist = 0;

    if (PMIC_CHRDET_EXIST == pmic_IsUsbCableIn()) {
        exist = 1;
        #if !CFG_USBIF_COMPLIANCE
        /* enable charging current as early as possible to avoid can't enter 
         * following battery charging flow when low battery
         */
        platform_set_chrg_cur(450);
        #endif
    }
    return exist;
}

int usb_cable_in(void)
{
    int exist = 0;
    CHARGER_TYPE ret;

    if ((g_boot_reason == BR_USB) || usb_accessory_in()) {
        ret = mt_charger_type_detection();
        if (ret == PMIC_STANDARD_HOST || ret == PMIC_CHARGING_HOST) {
            print("\n%s USB cable in\n", MOD);
            mt_usb_phy_poweron();
            mt_usb_phy_savecurrent();

            /* enable pmic hw charger detection */
            #if CFG_BATTERY_DETECT
            if (hw_check_battery())
                pl_hw_ulc_det();
            #endif

            exist = 1;
        } else if (ret == PMIC_NONSTANDARD_CHARGER || ret == PMIC_STANDARD_CHARGER) {
            #if CFG_USBIF_COMPLIANCE
            platform_set_chrg_cur(450);
            #endif
        }
    }
    return exist;
}

void platform_vusb_on(void)
{
    #define RG_VUSB_EN 0x01
    U8 cmd, dat;
    U32 ret;

    cmd = BANK0_DIGLDO_CON19;
    ret = pmic6329_i2c_read(0xc0, &cmd, 1, &dat, 1);
    if (I2C_OK != ret) {
        print("%s read pmic(%x) fail(%d)\n", MOD, cmd, ret);
    } else {
        print("%s read pmic(%x) = %x\n", MOD, cmd, dat);
    }
    if (0 == (dat & RG_VUSB_EN)) { /*force turn on USB LDO*/
        dat |= RG_VUSB_EN;
        ret = pmic6329_i2c_write(0xc0, &cmd, 1, &dat, 1);
        if (I2C_OK != ret) 
            print("%s write pmic(%x) fail(%d)\n", MOD, cmd, ret);
        dat = 0;    
        ret = pmic6329_i2c_read(0xc0, &cmd, 1, &dat, 1);    
        if (I2C_OK != ret)
            print("%s read pmic(%x) fail(%d)\n", MOD, cmd, ret);
        print("%s read pmic(%x) = %x\n", MOD, cmd, dat);
    }

    return;
}
#endif

void platform_set_boot_args(void)
{
#if CFG_BOOT_ARGUMENT
    boot_arg_t *bootarg = (boot_arg_t*)BOOT_ARGUMENT_ADDR;

    print("\n%s boot reason: %d\n", MOD, g_boot_reason);
    print("%s boot mode: %d\n", MOD, g_boot_mode);
    print("%s META COM%d: %d\n", MOD, g_meta_com_id, g_meta_com_type);

    bootarg->magic = BOOT_ARGUMENT_MAGIC;
    bootarg->mode  = g_boot_mode;
    bootarg->e_flag = sp_check_platform();
#if UART_LOG_CHANGE
    if((g_boot_mode==META_BOOT)||(g_boot_mode==ADVMETA_BOOT)||(g_boot_mode==ATE_FACTORY_BOOT))
    		bootarg->log_port = CFG_UART_LOG ;
    else
		bootarg->log_port = CFG_UART_META;
#else 
    bootarg->log_port = CFG_UART_LOG ;
#endif
    bootarg->log_baudrate = CFG_LOG_BAUDRATE;
    bootarg->log_enable = (u8)log_status();
    bootarg->dram_rank_num = get_dram_rank_nr();
    get_dram_rank_size(bootarg->dram_rank_size);
    bootarg->boot_reason = g_boot_reason;
    bootarg->meta_com_type = (u32)g_meta_com_type;
    bootarg->meta_com_id = g_meta_com_id;
    bootarg->boot_time = get_timer(g_boot_time);

    print("%s boot time: %dms\n", MOD, bootarg->boot_time);
#endif
}

void platform_wdt_kick(void)
{
    /* kick hardware watchdog */
    mtk_wdt_restart();
}

void platform_wdt_all_kick(void)
{
    /* kick watchdog to avoid cpu reset */
    mtk_wdt_restart();

    /* kick PMIC watchdog to keep charging */
    pl_kick_chr_wdt();
}

#if CFG_DT_MD_DOWNLOAD
void platform_modem_download(void)
{
    print("%s modem download...\n", MOD);

    /* Switch to MT6252 USB: 
     * GPIO_52_USB_SW1(USB_SW1)=1
     * GPIO_52_USB_SW2(USB_SW2)=0
     */
    mt_set_gpio_mode(GPIO_52_USB_SW1, GPIO_52_USB_SW1_M_GPIO);
    mt_set_gpio_mode(GPIO_52_USB_SW2, GPIO_52_USB_SW2_M_GPIO);
    mt_set_gpio_dir(GPIO_52_USB_SW1, GPIO_DIR_OUT);
    mt_set_gpio_dir(GPIO_52_USB_SW2, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO_52_USB_SW1, GPIO_OUT_ONE);
    mt_set_gpio_out(GPIO_52_USB_SW2, GPIO_OUT_ZERO);

    /* Press MT6252 DL key to enter download mode
     * GPIO_52_KCOL0(GPIO_KCOL0)=0 
     */        
    mt_set_gpio_mode(GPIO_52_KCOL0, GPIO_52_KCOL0_M_GPIO);
    mt_set_gpio_dir(GPIO_52_KCOL0, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO_52_KCOL0, GPIO_OUT_ZERO);

    /* Bring-up MT6252:
     * GPIO_OTG_DRVVBUS_PIN(GPIO_USB_DRVVBUS)=0,
     * GPIO_52_RST(GPIO_RESETB)=INPUT/NOPULL, 
     * GPIO_52_PWR_KEY(GPIO_PWRKEY)=0->1->0 
     */
    mt_set_gpio_mode(GPIO_OTG_DRVVBUS_PIN, GPIO_OTG_DRVVBUS_PIN_M_GPIO);
    mt_set_gpio_mode(GPIO_52_RST, GPIO_52_RST_M_GPIO);

    /* MD DRVVBUS to low */
    mt_set_gpio_dir(GPIO_OTG_DRVVBUS_PIN, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO_OTG_DRVVBUS_PIN, GPIO_OUT_ZERO);

    /* MD reset pin: hold to zero */
    mt_set_gpio_pull_enable(GPIO_52_RST, GPIO_PULL_DISABLE); 
    mt_set_gpio_dir(GPIO_52_RST, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO_52_RST, GPIO_OUT_ZERO); /* default @ reset state */
    mdelay(200);

    /* MD reset pin: released */
    mt_set_gpio_out(GPIO_52_RST, GPIO_OUT_ONE);
    mt_set_gpio_dir(GPIO_52_RST, GPIO_DIR_IN);
    mdelay(200);

    while(1) {
        platform_wdt_all_kick();
    }
}
#endif

#if CFG_EMERGENCY_DL_SUPPORT
void platform_download(int enable, u32 timeout)
{
    u32 tmp = enable ? (DEM_USBDL_BIT | timeout) : 0;
    
    DRV_WriteReg32(DEM_LOCK, DEM_UNLOCK_VAL);
    DRV_WriteReg32(DEM_USBDL, DEM_USBDL_BIT|timeout);
    tmp = DRV_Reg32(DEM_RST_SRC_CTRL);
    tmp &= ~DEM_RST_WDT;
    DRV_WriteReg32(DEM_RST_SRC_CTRL, tmp);
    DRV_WriteReg32(DEM_LOCK, 0);
}

void platform_emergency_download(u32 timeout)
{
    /* enter download mode */
    print("%s download mode(timeout: %ds).\n", MOD, timeout/1000);

    platform_download(1, timeout);
    mtk_arch_reset(0); /* don't bypass power key */
    while(1);
}
#else
#define platform_emergency_download(x)  do{}while(0)
#endif


int platform_get_mcp_id(u8 *id, u32 len)
{
    int ret = -1;

    memset(id, 0, len);
    
#if (CFG_BOOT_DEV == BOOTDEV_SDMMC)
    ret = mmc_get_device_id(id, len);
#else
    ret = nand_get_device_id(id, len);
#endif

    return ret;
}

void platform_set_chrg_cur(int ma)
{
    hw_set_cc(ma);
}

static boot_reason_t platform_boot_status(void)
{  
    if (rtc_boot_check()) {
        print("%s RTC boot!\n", MOD);
        return BR_RTC;
    }

    if (mtk_wdt_boot_check() == WDT_NORMAL_REBOOT) {
        print("%s WDT normal boot!\n", MOD);
        return BR_WDT;
    } else if (mtk_wdt_boot_check() == WDT_BY_PASS_PWK_REBOOT){
        print("%s WDT reboot bypass power key!\n", MOD);
        return BR_WDT_BY_PASS_PWK;
    }
#if !CFG_FPGA_PLATFORM
    /* check power key */
    if (mtk_detect_key(8)) {
        print("%s Power key boot!\n", MOD);
        rtc_mark_bypass_pwrkey();
        return BR_POWER_KEY;
    }
#endif

#ifndef EVB_PLATFORM
    if (usb_accessory_in()) {
        print("%s USB/charger boot!\n", MOD);
        return BR_USB;
    }

    print("%s Unknown boot!\n", MOD);
    pl_power_off();
    /* should nerver be reached */
#endif

    print("%s Power key boot!\n", MOD);

    return BR_POWER_KEY;
}

#if CFG_LOAD_DSP_ROM || CFG_LOAD_MD_ROM
int platform_is_three_g(void)
{
    u32 tmp = sp_check_platform();

    return (tmp & 0x1) ? 0 : 1;
}
#endif

chip_ver_t platform_chip_ver(void)
{
    unsigned int hw_subcode = DRV_Reg32(APHW_SUBCODE);
    unsigned int sw_ver = DRV_Reg32(APSW_VER);
    chip_ver_t ver = CHIP_6575_E2;

    if (0x8A00 == hw_subcode) {
        if (0xE100 == sw_ver) {
            ver = CHIP_6575_E1;
        } else if (0xE201 == sw_ver) {
            if (0x40000000 == (DRV_Reg32(0xC1019100) & 0x40000000)) {
                ver = CHIP_6575_E3;
            } else {
                ver = CHIP_6575_E2;
            }
        }    
    } else if (0x8B00 == hw_subcode) {
        ver = CHIP_6577_E1;
    }
    return ver;
}

// ------------------------------------------------ 
// detect download mode
// ------------------------------------------------ 

bool platform_com_wait_forever_check(void)
{
#ifdef USBDL_DETECT_VIA_KEY
    /* check download key */
    if (TRUE == mtk_detect_key(COM_WAIT_KEY)) {
        print("%s COM handshake timeout force disable: Key\n", MOD);
        return TRUE;
    }
#endif

#ifdef USBDL_DETECT_VIA_AT_COMMAND
    /* check WDT_IN_DIS bit */
    if(0 == (INREG32(MTK_WDT_MODE) & MTK_WDT_MODE_IN_DIS)) {
        print("%s COM handshake timeout force disable: AT Cmd\n", MOD);
        SETREG32(MTK_WDT_MODE, MTK_WDT_MODE_IN_DIS);
        return TRUE;
    }
#endif

    return FALSE;
}

void platform_pre_init(void)
{
    u32 i2c_ret, pmic_ret;

    /* init timer */
    mtk_timer_init();

    /* init boot time */
    g_boot_time = get_timer(0);

    if (platform_chip_ver() >= CHIP_6577_E1)
    {
        /*
         * NoteXXX: CPU 1 may not be reset clearly after power-ON.
         *          Need to apply a S/W workaround to manualy reset it first.
         */
        {
            U32 val;
            val = DRV_Reg32(0xC0009010);
            DRV_WriteReg32(0xC0009010, val | 0x2);
            gpt_busy_wait_us(10);
            DRV_WriteReg32(0xC0009010, val & ~0x2);
            gpt_busy_wait_us(10);
        }
#ifndef SLT_BOOTLOADER        
        /* power off cpu1 for power saving */
        power_off_cpu1();
#endif
    }

    /* init pll */
    mt_pll_init();

    /* init uart baudrate when pll on */
    mtk_uart_init(UART_SRC_CLK_FRQ, CFG_LOG_BAUDRATE);

    /* init pmic i2c interface and pmic */
    i2c_ret = i2c_v1_init();
#if !CFG_FPGA_PLATFORM
    pmic_ret = pmic6329_init();
    i2c_workaround();
#endif

    print("%s Init I2C: %s(%d)\n", MOD, i2c_ret ? "FAIL" : "OK", i2c_ret);    
    print("%s Init PMIC: %s(%d)\n", MOD, pmic_ret ? "FAIL" : "OK", pmic_ret);

    /* init pll according to ddr1 or ddr2 */
    if (mt_pll_init2()) {
        /* re-init uart baudrate when pll changes */
        mtk_uart_init(UART_SRC_CLK_FRQ, CFG_LOG_BAUDRATE);
#if !CFG_FPGA_PLATFORM
        /* re-init i2c when pll changes */
        i2c_workaround();
#endif
    }
    print("%s chip[%x]\n", MOD, platform_chip_ver());
}

void platform_init(void)
{
    u32 ret, tmp;
    boot_reason_t reason;

    /* init watch dog, will enable AP watch dog */
    mtk_wdt_init();

#if CFG_MDWDT_DISABLE
    /* disable MD watch dog. (JH) */
    DRV_WriteReg32(0xD10C0000, 0x2200); 

    /* disable DSP watch dog. (YT) */
    DRV_WriteReg32(0xD10C0020, 0x2200); 
#endif

    /* make usb11 phy enter savecurrent mode */
    mt_usb11_phy_savecurrent();

    g_boot_reason = reason = platform_boot_status();

    if (reason == BR_RTC || reason == BR_POWER_KEY || reason == BR_USB || reason == BR_WDT || reason == BR_WDT_BY_PASS_PWK)
        rtc_bbpu_power_on();

#if CFG_EMERGENCY_DL_SUPPORT
    /* check if to enter emergency download mode */
    if (mtk_detect_dl_keys()) {
        platform_emergency_download(CFG_EMERGENCY_DL_TIMEOUT_MS);
    }
#endif

    /* init memory */
    mt_mem_init();

    /* enable CA9 share bits for USB(30)/NFI(29)/MSDC(28) modules to access ISRAM */
    tmp = DRV_Reg32(0xC1000200);
    tmp |= ((1<<30)|(1<<29)|(1<<28));
    DRV_WriteReg32 (0xC1000200, tmp);

    /* init device storeage */
    ret = boot_device_init();
    print("%s Init Boot Device: %s(%d)\n", MOD, ret ? "FAIL" : "OK", ret);

#if CFG_REBOOT_TEST
    mtk_wdt_sw_reset();
    while(1);
#endif

}

void platform_post_init(void)
{
    struct ram_console_buffer *ram_console;

#if CFG_BATTERY_DETECT
    /* normal boot to check battery exists or not */
    if (g_boot_mode == NORMAL_BOOT && !hw_check_battery() && usb_accessory_in()) {
        print("%s Wait for battery inserted...\n", MOD);
        /* disable pmic pre-charging led */
        pl_close_pre_chr_led();
        /* enable force charging mode */
        pl_charging(1);
        do {
            mdelay(300);
            /* check battery exists or not */
            if (hw_check_battery())
                break;
            /* kick all watchdogs */
            platform_wdt_all_kick();
        } while(1);
        /* disable force charging mode */
        pl_charging(0);
    }
#endif

#if !CFG_FPGA_PLATFORM
    /* security check */
    sec_lib_read_secro();
    sec_boot_check();
    device_APC_dom_setup();

    /* apply analog setting */
    apply_analog_configuration();
#endif

#if CFG_MDJTAG_SWITCH
    /* ungate MD */
    DRV_WriteReg32(0xD10C0040, 0x00000037);
    /* switch to MD legacy JTAG */
    DRV_WriteReg32(0xC011AFB0, 0xC5ACCE55); /* unlock lock mechanism */
    DRV_WriteReg32(0xC011A034, 0x00000001); /* switch to MD Legacy JTAG */
#endif

#if CFG_MDMETA_DETECT
    if (g_boot_mode == META_BOOT || g_boot_mode == ADVMETA_BOOT) {
        DRV_WriteReg32(0xD10D0010, 0x1);
    } else {
        DRV_WriteReg32(0xD10D0010, 0x0);
    }
#endif
    /* disable USB(30)/NFI(29)/MSDC(28)/THERMAL(27) modules to access ISRAM */
    u32 tmp = DRV_Reg32(0xC1000200);
    tmp &= ~((1<<30)|(1<<29)|(1<<28)|(1<<27));
    DRV_WriteReg32(0xC1000200, tmp);

#if CFG_RAM_CONSOLE
    ram_console = (struct ram_console_buffer *)RAM_CONSOLE_ADDR;

    if (ram_console->sig == RAM_CONSOLE_SIG) {
        print("%s ram_console->start=0x%x\n", MOD, ram_console->start);
        if (ram_console->start > RAM_CONSOLE_MAX_SIZE)
            ram_console->start = 0;

        ram_console->hw_status = g_rgu_status;

        print("%s ram_console(0x%x)=0x%x (boot reason)\n", MOD, 
            ram_console->hw_status, g_rgu_status);
    }
#endif
}

void platform_error_handler(void)
{
    /* if log is disabled, re-init log port and enable it */
    if (log_status() == 0) {
        mtk_uart_init(UART_SRC_CLK_FRQ, CFG_LOG_BAUDRATE);
        log_ctrl(1);
    }
    print("%s preloader fatal error...\n", MOD);
    sec_util_brom_download_recovery_check(); 
    /* enter emergency download mode */
    platform_emergency_download(CFG_EMERGENCY_DL_MAX_TIMEOUT_MS);
    while(1);
}

void platform_assert(char *file, int line, char *expr)
{   
    print("<ASSERT> %s:line %d %s\n", file, line, expr); 
    platform_error_handler();
}

