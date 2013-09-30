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
#include "cust_nand.h"
#include "nand.h"
//#include "mtk_nand_device.h"
#include "nand_device_list.h"
#include "bmt.h"
//#include "nand_customer.h"


#ifndef PART_SIZE_BMTPOOL
#define BMT_POOL_SIZE (80)
#else
#define BMT_POOL_SIZE (PART_SIZE_BMTPOOL)
#endif

#define PMT_POOL_SIZE (2)
/******************************************************************************
*
* Macro definition
*
*******************************************************************************/

#define NFI_SET_REG32(reg, value)   (DRV_WriteReg32(reg, DRV_Reg32(reg) | (value)))
#define NFI_SET_REG16(reg, value)   (DRV_WriteReg16(reg, DRV_Reg16(reg) | (value)))
#define NFI_CLN_REG32(reg, value)   (DRV_WriteReg32(reg, DRV_Reg32(reg) & (~(value))))
#define NFI_CLN_REG16(reg, value)   (DRV_WriteReg16(reg, DRV_Reg16(reg) & (~(value))))

#define FIFO_PIO_READY(x)  (0x1 & x)
#define WAIT_NFI_PIO_READY(timeout) \
    do {\
    while( (!FIFO_PIO_READY(DRV_Reg(NFI_PIO_DIRDY_REG16))) && (--timeout) );\
    if(timeout == 0)\
   {\
   MSG(ERR, "Error: FIFO_PIO_READY timeout at line=%d, file =%s\n", __LINE__, __FILE__);\
   }\
    } while(0);

#define TIMEOUT_1   0x1fff
#define TIMEOUT_2   0x8ff
#define TIMEOUT_3   0xffff
#define TIMEOUT_4   5000        //PIO

#define STATUS_READY			(0x40)
#define STATUS_FAIL				(0x01)
#define STATUS_WR_ALLOW			(0x80)

#define NFI_ISSUE_COMMAND(cmd, col_addr, row_addr, col_num, row_num) \
    do { \
    DRV_WriteReg(NFI_CMD_REG16,cmd);\
    while (DRV_Reg32(NFI_STA_REG32) & STA_CMD_STATE);\
    DRV_WriteReg32(NFI_COLADDR_REG32, col_addr);\
    DRV_WriteReg32(NFI_ROWADDR_REG32, row_addr);\
    DRV_WriteReg(NFI_ADDRNOB_REG16, col_num | (row_num<<ADDR_ROW_NOB_SHIFT));\
    while (DRV_Reg32(NFI_STA_REG32) & STA_ADDR_STATE);\
    }while(0);

extern u32 PAGE_SIZE;
extern u32 BLOCK_SIZE;

/**************************************************************************
*  MACRO LIKE FUNCTION
**************************************************************************/
#ifdef MTK_EMMC_SUPPORT
inline u32 PAGE_NUM (u64 logical_size)
{
        return ((u64) (logical_size) / PAGE_SIZE);
}

#else
inline u32 PAGE_NUM (u32 logical_size)
{
        return ((unsigned long) (logical_size) / PAGE_SIZE);
}
#endif

inline u32 LOGICAL_ADDR (u32 page_addr)
{
        return ((unsigned long) (page_addr) * PAGE_SIZE);
} 
inline u32 BLOCK_ALIGN (u32 logical_addr)
{
        return (((u32) (logical_addr / BLOCK_SIZE)) * BLOCK_SIZE);
}
//---------------------------------------------------------------------------

//-------------------------------------------------------------------------
typedef U32     (*STORGE_READ)(u8 *buf, u32 start, u32 img_size);

typedef struct {
	u32 page_size;
	u32 pktsz;
} device_info_t;
//-------------------------------------------------------------------------

device_info_t gdevice_info;
boot_dev_t g_dev_vfunc;

unsigned char g_nand_spare[128];
// unsigned char g_nand_buf[4096 + 128];

unsigned int nand_maf_id;
unsigned int nand_dev_id;
uint8 ext_id1, ext_id2, ext_id3;

//extern struct nand_chip g_nand_chip;

static u32 g_u4ChipVer;
static u32 g_i4ErrNum;
static BOOL g_bInitDone;
BOOL g_bHwEcc=TRUE;

/*
struct nand_oobinfo mtk_nand_oob = {
    .useecc = MTD_NANDECC_AUTOPLACE,
    .eccbytes = 32,
    .oobfree = {{1, 7}, {9, 7}, {17, 7}, {25, 7}, {0, 0}},
    .eccpos = {32, 33, 34, 35, 36, 37, 38, 39,
     40, 41, 42, 43, 44, 45, 46, 47,
     48, 49, 50, 51, 52, 53, 54, 55,
     56, 57, 58, 59, 60, 61, 62, 63}
};
*/
struct nand_chip g_nand_chip;
struct nand_ecclayout *nand_oob = NULL;

static struct nand_ecclayout nand_oob_16 = {
    .eccbytes = 8,
    .eccpos = {8, 9, 10, 11, 12, 13, 14, 15},
    .oobfree = {{1,6}, {0, 0}}
};

struct nand_ecclayout nand_oob_64 = {
    .eccbytes = 32,
    .eccpos = {32, 33, 34, 35, 36, 37, 38, 39,
     40, 41, 42, 43, 44, 45, 46, 47,
     48, 49, 50, 51, 52, 53, 54, 55,
     56, 57, 58, 59, 60, 61, 62, 63},
    .oobfree = {{1, 7}, {9, 7}, {17, 7}, {25, 6}, {0, 0}}
};

struct nand_ecclayout nand_oob_128 = {
    .eccbytes = 64,
    .eccpos = {
     64, 65, 66, 67, 68, 69, 70, 71,
     72, 73, 74, 75, 76, 77, 78, 79,
     80, 81, 82, 83, 84, 85, 86, 86,
     88, 89, 90, 91, 92, 93, 94, 95,
     96, 97, 98, 99, 100, 101, 102, 103,
     104, 105, 106, 107, 108, 109, 110, 111,
     112, 113, 114, 115, 116, 117, 118, 119,
     120, 121, 122, 123, 124, 125, 126, 127},
    .oobfree = {{1, 7}, {9, 7}, {17, 7}, {25, 7}, {33, 7}, {41, 7}, {49, 7}, {57, 6}}
};

struct NAND_CMD
{
    u32 u4ColAddr;
    u32 u4RowAddr;
    u32 u4OOBRowAddr;
    u8 au1OOB[64];
    u8 *pDataBuf;
};

static struct NAND_CMD g_kCMD;
static flashdev_info devinfo;
static char *nfi_buf;

/* Nand chip ID table */

struct nand_flash_device nand_flash_ids[] = {
#if 0
    {"NAND FLASH 1MB 5V 8b", 0x6e, 256, 1, 0x1000, 0},
    {"NAND FLASH 2MB 5V 8b", 0x64, 256, 2, 0x1000, 0},
    {"NAND FLASH 4MB 5V 8b", 0x6b, 512, 4, 0x2000, 0},
    {"NAND FLASH 1MB 3,3V 8b", 0xe8, 256, 1, 0x1000, 0},
    {"NAND FLASH 1MB 3,3V 8b", 0xec, 256, 1, 0x1000, 0},
    {"NAND FLASH 2MB 3,3V 8b", 0xea, 256, 2, 0x1000, 0},
    {"NAND FLASH 4MB 3,3V 8b", 0xd5, 512, 4, 0x2000, 0},
    {"NAND FLASH 4MB 3,3V 8b", 0xe3, 512, 4, 0x2000, 0},
    {"NAND FLASH 4MB 3,3V 8b", 0xe5, 512, 4, 0x2000, 0},
    {"NAND FLASH 8MB 3,3V 8b", 0xd6, 512, 8, 0x2000, 0},

    {"NAND FLASH 8MB 1,8V 8b", 0x39, 512, 8, 0x2000, 0},
    {"NAND FLASH 8MB 3,3V 8b", 0xe6, 512, 8, 0x2000, 0},
    {"NAND FLASH 8MB 1,8V 16b", 0x49, 512, 8, 0x2000, NAND_BUSW_16},
    {"NAND FLASH 8MB 3,3V 16b", 0x59, 512, 8, 0x2000, NAND_BUSW_16},

    {"NAND FLASH 16MB 1,8V 8b", 0x33, 512, 16, 0x4000, 0},
    {"NAND FLASH 16MB 3,3V 8b", 0x73, 512, 16, 0x4000, 0},
    {"NAND FLASH 16MB 1,8V 16b", 0x43, 512, 16, 0x4000, NAND_BUSW_16},
    {"NAND FLASH 16MB 3,3V 16b", 0x53, 512, 16, 0x4000, NAND_BUSW_16},

    {"NAND FLASH 32MB 1,8V 8b", 0x35, 512, 32, 0x4000, 0},
    {"NNAND FLASH AND 32MB 3,3V 8b", 0x75, 512, 32, 0x4000, 0},
    {"NAND FLASH 32MB 1,8V 16b", 0x45, 512, 32, 0x4000, NAND_BUSW_16},
    {"NAND FLASH 32MB 3,3V 16b", 0x55, 512, 32, 0x4000, NAND_BUSW_16},

    {"NAND FLASH 64MB 1,8V 8b", 0x36, 512, 64, 0x4000, 0},
    {"NAND FLASH 64MB 3,3V 8b", 0x76, 512, 64, 0x4000, 0},
    {"NAND FLASH 64MB 1,8V 16b", 0x46, 512, 64, 0x4000, NAND_BUSW_16},
    {"NAND FLASH 64MB 3,3V 16b", 0x56, 512, 64, 0x4000, NAND_BUSW_16},

    {"NAND FLASH 128MB 1,8V 8b", 0x78, 512, 128, 0x4000, 0},
    {"NAND FLASH 128MB 3,3V 8b", 0x79, 512, 128, 0x4000, 0},
    {"NAND FLASH 128MB 1,8V 16b", 0x72, 512, 128, 0x4000, NAND_BUSW_16},
    {"NAND FLASH 128MB 3,3V 16b", 0x74, 512, 128, 0x4000, NAND_BUSW_16},

    {"NAND FLASH 256MB 3,3V 8b", 0x71, 512, 256, 0x4000, 0},

    {"NAND FLASH 512MB 3,3V 8b", 0xDC, 512, 512, 0x4000, 0},
#endif

    /* 1 G bit */
    {"NAND FLASH 128MB 1.8V 8b", 0xA1, 0, 128, 0,
    NAND_SS_LOWPOWER_OPTIONS | NAND_NO_AUTOINCR},
    {"NAND FLASH 128MB 3.3V 8b", 0xF1, 0, 128, 0,
    NAND_SS_LOWPOWER_OPTIONS | NAND_NO_AUTOINCR},
    {"NAND FLASH 128MB 1.8V 16b", 0xB1, 0, 128, 0,
    NAND_SS_LOWPOWER_OPTIONS | NAND_BUSW_16 | NAND_NO_AUTOINCR},
    {"NAND FLASH 128MB 3.3V 16b", 0xC1, 0, 128, 0,
    NAND_SS_LOWPOWER_OPTIONS | NAND_BUSW_16 | NAND_NO_AUTOINCR},
    /* 2 G bit */
    {"NAND FLASH 256MB 1.8V 8b", 0xAA, 0, 256, 0,
    NAND_SS_LOWPOWER_OPTIONS | NAND_NO_AUTOINCR},
    {"NAND FLASH 256MB 3.3V 8b", 0xDA, 0, 256, 0,
    NAND_SS_LOWPOWER_OPTIONS | NAND_NO_AUTOINCR},
    {"NAND FLASH 256MB 1.8V 16b", 0xBA, 0, 256, 0,
    NAND_SS_LOWPOWER_OPTIONS | NAND_BUSW_16 | NAND_NO_AUTOINCR},
    {"NAND FLASH 256MB 3.3V 16b", 0xCA, 0, 256, 0,
    NAND_SS_LOWPOWER_OPTIONS | NAND_BUSW_16 | NAND_NO_AUTOINCR},

    /* 4 G bit */
    {"NAND FLASH 512MB 1.8V 8b", 0xAC, 0, 512, 0,
    NAND_SS_LOWPOWER_OPTIONS | NAND_NO_AUTOINCR},
    {"NAND FLASH 512MB 3.3V 8b", 0xDC, 0, 512, 0,
    NAND_SS_LOWPOWER_OPTIONS | NAND_NO_AUTOINCR},
    {"NAND 512MB 1.8V 16b", 0xBC, 0, 512, 0,
    NAND_SS_LOWPOWER_OPTIONS | NAND_BUSW_16 | NAND_NO_AUTOINCR},
    {"NAND FLASH 512MB 3.3V 16b", 0xCC, 0, 512, 0,
    NAND_SS_LOWPOWER_OPTIONS | NAND_BUSW_16 | NAND_NO_AUTOINCR},

    /* 8 G bit */
    {"NAND FLASH 1GB 1.8V 8b", 0xA3, 0, 1024, 0,
    NAND_SS_LOWPOWER_OPTIONS | NAND_NO_AUTOINCR},
    {"NAND FLASH 1GB 3.3V 8b", 0xD3, 0, 1024, 0,
    NAND_SS_LOWPOWER_OPTIONS | NAND_NO_AUTOINCR},
    {"NAND FLASH 1GB 1.8V 16b", 0xB3, 0, 1024, 0,
    NAND_SS_LOWPOWER_OPTIONS | NAND_BUSW_16 | NAND_NO_AUTOINCR},
    {"NAND FLASH 1GB 3.3V 16b", 0xC3, 0, 1024, 0,
    NAND_SS_LOWPOWER_OPTIONS | NAND_BUSW_16 | NAND_NO_AUTOINCR},

    /* 16 G bit */
    {"NAND FLASH 2GB 1.8V 8b", 0xA5, 0, 2048, 0,
    NAND_SS_LOWPOWER_OPTIONS | NAND_NO_AUTOINCR},
    {"NAND FLASH 2GB 3.3V 8b", 0xD5, 0, 2048, 0,
    NAND_SS_LOWPOWER_OPTIONS | NAND_NO_AUTOINCR},
    {"NAND FLASH 2GB 1.8V 16b", 0xB5, 0, 2048, 0,
    NAND_SS_LOWPOWER_OPTIONS | NAND_BUSW_16 | NAND_NO_AUTOINCR},
    {"NAND FLASH 2GB 3.3V 16b", 0xC5, 0, 2048, 0,
    NAND_SS_LOWPOWER_OPTIONS | NAND_BUSW_16 | NAND_NO_AUTOINCR},
    {"NAND FLASH 128MB 3.3V 8b", 0x01, 2048, 128, 0x4000,
    NAND_IS_AND | NAND_NO_AUTOINCR | NAND_4PAGE_ARRAY},

    {NULL,}
};

bool get_device_info(u16 id, u32 ext_id, flashdev_info *devinfo);
/*
*   Manufacturer ID list
*/
struct nand_manufacturers nand_manuf_ids[] = {
    {NAND_MANFR_TOSHIBA, "Toshiba"},
    {NAND_MANFR_SAMSUNG, "Samsung"},
    {NAND_MANFR_FUJITSU, "Fujitsu"},
    {NAND_MANFR_NATIONAL, "National"},
    {NAND_MANFR_RENESAS, "Renesas"},
    {NAND_MANFR_STMICRO, "ST Micro"},
    {NAND_MANFR_HYNIX, "Hynix"},
    {NAND_MANFR_MICRON, "Micron"},
    {NAND_MANFR_AMD, "AMD"},
    {0x0, "Unknown"}
};

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

#define NAND_SECTOR_SIZE 512

/**************************************************************************
*  reset descriptor
**************************************************************************/
void mtk_nand_reset_descriptor (void)
{

    g_nand_chip.page_shift = 0;
    g_nand_chip.page_size = 0;
    g_nand_chip.ChipID = 0;     /* Type of DiskOnChip */
    g_nand_chip.chips_name = 0;
    g_nand_chip.chipsize = 0;
    g_nand_chip.erasesize = 0;
    g_nand_chip.mfr = 0;        /* Flash IDs - only one type of flash per device */
    g_nand_chip.id = 0;
    g_nand_chip.name = 0;
    g_nand_chip.numchips = 0;
    g_nand_chip.oobblock = 0;   /* Size of OOB blocks (e.g. 512) */
    g_nand_chip.oobsize = 0;    /* Amount of OOB data per block (e.g. 16) */
    g_nand_chip.eccsize = 0;
    g_nand_chip.bus16 = 0;
    g_nand_chip.nand_ecc_mode = 0;

}

bool get_device_info(u16 id, u32 ext_id, flashdev_info *devinfo)
{
    u32 index;
    for (index = 0; gen_FlashTable[index].id != 0; index++)
    {
        if (id == gen_FlashTable[index].id && ext_id == gen_FlashTable[index].ext_id)
        {
            devinfo->id = gen_FlashTable[index].id;
            devinfo->ext_id = gen_FlashTable[index].ext_id;
            devinfo->blocksize = gen_FlashTable[index].blocksize;
            devinfo->addr_cycle = gen_FlashTable[index].addr_cycle;
            devinfo->iowidth = gen_FlashTable[index].iowidth;
            devinfo->timmingsetting = gen_FlashTable[index].timmingsetting;
            devinfo->advancedmode = gen_FlashTable[index].advancedmode;
            devinfo->pagesize = gen_FlashTable[index].pagesize;
 	    devinfo->sparesize = gen_FlashTable[index].sparesize;
            devinfo->totalsize = gen_FlashTable[index].totalsize;
            memcpy(devinfo->devciename, gen_FlashTable[index].devciename, sizeof(devinfo->devciename));
            MSG(INIT, "MTK Table, ID: %x, EXT_ID: %x\n", id, ext_id);

            goto find;
        }
    }
#if 0
    for (index = 0; cust_FlashTable[index].id != 0; index++)
    {
        if (id == cust_FlashTable[index].id)
        {
            devinfo->id = cust_FlashTable[index].id;
            devinfo->blocksize = cust_FlashTable[index].blocksize;
            devinfo->addr_cycle = cust_FlashTable[index].addr_cycle;
            devinfo->iowidth = cust_FlashTable[index].iowidth;
            devinfo->timmingsetting = cust_FlashTable[index].timmingsetting;
            devinfo->advancedmode = cust_FlashTable[index].advancedmode;
            devinfo->pagesize = cust_FlashTable[index].pagesize;
            devinfo->totalsize = cust_FlashTable[index].totalsize;
            memcpy(devinfo->devciename, cust_FlashTable[index].devciename, sizeof(devinfo->devciename));
            MSG(INIT, "Cust Table, ID: %x\n", id);

            goto find;
        }
    }

#endif
find:
    if (0 == devinfo->id)
    {
        MSG(INIT, "ID: %x not found\n", id);
        return false;
    }
    else
        return true;
}

//---------------------------------------------------------------------------
static bool mtk_nand_check_RW_count (u16 u2WriteSize)
{
    u32 timeout = 0xFFFF;
    u16 u2SecNum = u2WriteSize >> 9;
    while (ADDRCNTR_CNTR (DRV_Reg16 (NFI_ADDRCNTR_REG16)) < u2SecNum)
    {
        timeout--;
        if (0 == timeout)
        {
            return FALSE;
        }
    }
    return TRUE;
}


//---------------------------------------------------------------------------
static bool mtk_nand_status_ready (u32 u4Status)
{
    u32 timeout = 0xFFFF;
    while ((DRV_Reg32 (NFI_STA_REG32) & u4Status) != 0)
    {
        timeout--;
        if (0 == timeout)
        {
            return FALSE;
        }
    }
    return TRUE;
}

//---------------------------------------------------------------------------
static void mtk_nand_set_mode (u16 u2OpMode)
{
    u16 u2Mode = DRV_Reg16 (NFI_CNFG_REG16);
    u2Mode &= ~CNFG_OP_MODE_MASK;
    u2Mode |= u2OpMode;
    DRV_WriteReg16 (NFI_CNFG_REG16, u2Mode);
}

//---------------------------------------------------------------------------
static bool mtk_nand_set_command (u16 command)
{
    /* Write command to device */
    DRV_WriteReg16 (NFI_CMD_REG16, command);
    return mtk_nand_status_ready (STA_CMD_STATE);
}

//---------------------------------------------------------------------------
static bool mtk_nand_set_address (u32 u4ColAddr, u32 u4RowAddr, u16 u2ColNOB,
                         u16 u2RowNOB)
{
    /* fill cycle addr */
    DRV_WriteReg32 (NFI_COLADDR_REG32, u4ColAddr);
    DRV_WriteReg32 (NFI_ROWADDR_REG32, u4RowAddr);
    DRV_WriteReg16 (NFI_ADDRNOB_REG16,
        u2ColNOB | (u2RowNOB << ADDR_ROW_NOB_SHIFT));
    return mtk_nand_status_ready (STA_ADDR_STATE);
}

//---------------------------------------------------------------------------
static void ECC_Decode_Start (void)
{
    /* wait for device returning idle */
    while (!(DRV_Reg16 (ECC_DECIDLE_REG16) & DEC_IDLE));
    DRV_WriteReg16 (ECC_DECCON_REG16, DEC_EN);
}

//---------------------------------------------------------------------------
static void ECC_Decode_End (void)
{
    /* wait for device returning idle */
    while (!(DRV_Reg16 (ECC_DECIDLE_REG16) & DEC_IDLE));
    DRV_WriteReg16 (ECC_DECCON_REG16, DEC_DE);
}

//---------------------------------------------------------------------------
static void ECC_Encode_Start (void)
{
    /* wait for device returning idle */
    while (!(DRV_Reg32 (ECC_ENCIDLE_REG32) & ENC_IDLE));
    DRV_WriteReg16 (ECC_ENCCON_REG16, ENC_EN);
}

//---------------------------------------------------------------------------
static void ECC_Encode_End (void)
{
    /* wait for device returning idle */
    while (!(DRV_Reg32 (ECC_ENCIDLE_REG32) & ENC_IDLE));
    DRV_WriteReg16 (ECC_ENCCON_REG16, ENC_DE);
}

//---------------------------------------------------------------------------
static void ECC_Config (u32 ecc_bit)
{
    u32 u4ENCODESize;
    u32 u4DECODESize;

    u32 ecc_bit_cfg = ECC_CNFG_ECC4;

    switch(ecc_bit)
    {
	case 4:
		ecc_bit_cfg = ECC_CNFG_ECC4;
		break;
	case 8:
		ecc_bit_cfg = ECC_CNFG_ECC8;
		break;
	case 10:
		ecc_bit_cfg = ECC_CNFG_ECC10;
		break;
	case 12:
		ecc_bit_cfg = ECC_CNFG_ECC12;
		break;
	default:
		break;
    }

    DRV_WriteReg16 (ECC_DECCON_REG16, DEC_DE);
    do
    {;
    }
    while (!DRV_Reg16 (ECC_DECIDLE_REG16));

    DRV_WriteReg16 (ECC_ENCCON_REG16, ENC_DE);
    do
    {;
    }
    while (!DRV_Reg32 (ECC_ENCIDLE_REG32));

    /* setup FDM register base */
    DRV_WriteReg32 (ECC_FDMADDR_REG32, NFI_FDM0L_REG32);

    u4ENCODESize = (NAND_SECTOR_SIZE + 8) << 3;
    u4DECODESize = ((NAND_SECTOR_SIZE + 8) << 3) +  ecc_bit * 13;

    /* configure ECC decoder && encoder */
    DRV_WriteReg32 (ECC_DECCNFG_REG32,
        ecc_bit_cfg | DEC_CNFG_NFI | DEC_CNFG_EMPTY_EN |
        (u4DECODESize << DEC_CNFG_CODE_SHIFT));

    DRV_WriteReg32 (ECC_ENCCNFG_REG32,
        ecc_bit_cfg | ENC_CNFG_NFI |
        (u4ENCODESize << ENC_CNFG_MSG_SHIFT));

#ifndef MANUAL_CORRECT
    NFI_SET_REG32 (ECC_DECCNFG_REG32, DEC_CNFG_CORRECT);
#else
    NFI_SET_REG32 (ECC_DECCNFG_REG32, DEC_CNFG_EL);
#endif

}

/******************************************************************************
* mtk_nand_check_bch_error
*
* DESCRIPTION:
*   Check BCH error or not !
*
* PARAMETERS:
*   struct mtd_info *mtd
*    u8* pDataBuf
*    u32 u4SecIndex
*    u32 u4PageAddr
*
* RETURNS:
*   None
*
* NOTES:
*   None
*
******************************************************************************/
static bool mtk_nand_check_bch_error (u8 * pDataBuf, u32 u4SecIndex, u32 u4PageAddr)
{
    bool bRet = TRUE;
    u16 u2SectorDoneMask = 1 << u4SecIndex;
    u32 u4ErrorNumDebug, i, u4ErrNum;
    u32 timeout = 0xFFFF;

#ifdef MANUAL_CORRECT
    u32 au4ErrBitLoc[6];
    u32 u4ErrByteLoc, u4BitOffset;
    u32 u4ErrBitLoc1th, u4ErrBitLoc2nd;
#endif

    while (0 == (u2SectorDoneMask & DRV_Reg16 (ECC_DECDONE_REG16)))
    {
        timeout--;
        if (0 == timeout)
        {
            return FALSE;
        }
    }
#ifndef MANUAL_CORRECT
    u4ErrorNumDebug = DRV_Reg32 (ECC_DECENUM_REG32);
    if (0 != (u4ErrorNumDebug & 0xFFFF))
    {
        for (i = 0; i <= u4SecIndex; ++i)
        {
            u4ErrNum = DRV_Reg32 (ECC_DECENUM_REG32) >> (i << 2);
            u4ErrNum &= 0xF;
            if (0xF == u4ErrNum)
            {
                bRet = FALSE;
                printf("UnCorrectable at PageAddr=%d, Sector=%d with ECC_DECENUM=%x\n", u4PageAddr, i,u4ErrNum);
            }
            else
            {
                printf( "Correct %d bit error at PageAddr=%d, Sector=%d\n",u4ErrNum, u4PageAddr, i);
            }
        }
    }
#else
/* We will manually correct the error bits in the last sector, not all the sectors of the page!*/
    //memset(au4ErrBitLoc, 0x0, sizeof(au4ErrBitLoc));
    u4ErrorNumDebug = DRV_Reg32 (ECC_DECENUM_REG32);
    u4ErrNum = DRV_Reg32 (ECC_DECENUM_REG32) >> (u4SecIndex << 2);
    u4ErrNum &= 0xF;
    if (u4ErrNum)
    {
        if (0xF == u4ErrNum)
        {
            //mtd->ecc_stats.failed++;
            bRet = FALSE;
            //printk(KERN_ERR"UnCorrectable at PageAddr=%d\n", u4PageAddr);
        }
        else
        {
            for (i = 0; i < ((u4ErrNum + 1) >> 1); ++i)
            {
                au4ErrBitLoc[i] = DRV_Reg32 (ECC_DECEL0_REG32 + i);
                u4ErrBitLoc1th = au4ErrBitLoc[i] & 0x1FFF;
                if (u4ErrBitLoc1th < 0x1000)
                {
                    u4ErrByteLoc = u4ErrBitLoc1th / 8;
                    u4BitOffset = u4ErrBitLoc1th % 8;
                    pDataBuf[u4ErrByteLoc] =
                        pDataBuf[u4ErrByteLoc] ^ (1 << u4BitOffset);
                    //mtd->ecc_stats.corrected++;
                }
                else
                {
                    //mtd->ecc_stats.failed++;
                    MSG (INIT, "UnCorrectable ErrLoc=%d\n",
                        au4ErrBitLoc[i]);
                }
                u4ErrBitLoc2nd = (au4ErrBitLoc[i] >> 16) & 0x1FFF;
                if (0 != u4ErrBitLoc2nd)
                {
                    if (u4ErrBitLoc2nd < 0x1000)
                    {
                        u4ErrByteLoc = u4ErrBitLoc2nd / 8;
                        u4BitOffset = u4ErrBitLoc2nd % 8;
                        pDataBuf[u4ErrByteLoc] =
                            pDataBuf[u4ErrByteLoc] ^ (1 <<
                            u4BitOffset);
                        //mtd->ecc_stats.corrected++;
                    }
                    else
                    {
                        //mtd->ecc_stats.failed++;
                        MSG (INIT, "UnCorrectable High ErrLoc=%d\n",
                            au4ErrBitLoc[i]);
                    }
                }
            }
        }
        if (0 == (DRV_Reg16 (ECC_DECFER_REG16) & (1 << u4SecIndex)))
        {
            bRet = FALSE;
        }
    }
#endif
    return bRet;
}


//---------------------------------------------------------------------------
static bool mtk_nand_RFIFOValidSize (u16 u2Size)
{
    u32 timeout = 0xFFFF;
    while (FIFO_RD_REMAIN (DRV_Reg16 (NFI_FIFOSTA_REG16)) < u2Size)
    {
        timeout--;
        if (0 == timeout)
        {
            return FALSE;
        }
    }
	if(u2Size==0)
	{
		while (FIFO_RD_REMAIN(DRV_Reg16(NFI_FIFOSTA_REG16)))
		{
			timeout--;
			if (0 == timeout){
				return FALSE;
			}
		}
	}
    return TRUE;
}

//---------------------------------------------------------------------------
static bool mtk_nand_WFIFOValidSize (u16 u2Size)
{
    u32 timeout = 0xFFFF;
    while (FIFO_WR_REMAIN (DRV_Reg16 (NFI_FIFOSTA_REG16)) > u2Size)
    {
        timeout--;
        if (0 == timeout)
        {
            return FALSE;
        }
    }
	if(u2Size==0)
	{
		while (FIFO_WR_REMAIN(DRV_Reg16(NFI_FIFOSTA_REG16)))
		{
			timeout--;
			if (0 == timeout){
				return FALSE;
			}
		}
	}
    return TRUE;
}

//---------------------------------------------------------------------------
bool mtk_nand_reset (void)
{
    int timeout = 0xFFFF;
    if (DRV_Reg16(NFI_MASTERSTA_REG16)) // master is busy
    {
    DRV_WriteReg16 (NFI_CON_REG16, CON_FIFO_FLUSH | CON_NFI_RST);
        while (DRV_Reg16(NFI_MASTERSTA_REG16))
        {
            timeout--;
            if (!timeout)
            {
                MSG(INIT, "MASTERSTA timeout\n");
            }
        }
    }
	/* issue reset operation */
	DRV_WriteReg16(NFI_CON_REG16, CON_FIFO_FLUSH | CON_NFI_RST);

    return mtk_nand_status_ready (STA_NFI_FSM_MASK | STA_NAND_BUSY) &&
		   mtk_nand_RFIFOValidSize(0) &&
		   mtk_nand_WFIFOValidSize(0);
}

static bool mtk_nand_read_status(void)
{
    int status, i;
    mtk_nand_reset();
    unsigned int timeout;

    mtk_nand_reset();

    /* Disable HW ECC */
    NFI_CLN_REG16 (NFI_CNFG_REG16, CNFG_HW_ECC_EN);

    /* Disable 16-bit I/O */
    NFI_CLN_REG16 (NFI_PAGEFMT_REG16, PAGEFMT_DBYTE_EN);
    NFI_SET_REG16 (NFI_CNFG_REG16, CNFG_OP_SRD | CNFG_READ_EN | CNFG_BYTE_RW);

    DRV_WriteReg16(NFI_CON_REG16, CON_NFI_SRD | (1 << CON_NOB_SHIFT));

    DRV_WriteReg16(NFI_CON_REG16, 0x3);
    mtk_nand_set_mode(CNFG_OP_SRD);
    DRV_WriteReg16(NFI_CNFG_REG16, 0x2042);
    mtk_nand_set_command(NAND_CMD_STATUS);
    DRV_WriteReg16(NFI_CON_REG16, 0x90);

    timeout = TIMEOUT_4;
    WAIT_NFI_PIO_READY(timeout);

    if(timeout)
    {
        status = (DRV_Reg16(NFI_DATAR_REG32));
    }

    //~  clear NOB
    DRV_WriteReg16(NFI_CON_REG16, 0);

    if (g_nand_chip.bus16 == NAND_BUS_WIDTH_16)
    {
        NFI_SET_REG16 (NFI_PAGEFMT_REG16, PAGEFMT_DBYTE_EN);
        NFI_CLN_REG16 (NFI_CNFG_REG16, CNFG_BYTE_RW);
    }
    
    // check READY/BUSY status first 
    if( !(STATUS_READY&status) ) {
        MSG(ERR, "status is not ready\n");  
    }

    // flash is ready now, check status code
    if( STATUS_FAIL & status ) {
        if( !(STATUS_WR_ALLOW&status) ) {
            MSG(INIT, "status locked\n");
            return FALSE;
        }
	else {
            MSG(INIT, "status unknown\n");
            return FALSE;
        }
    }
    else {
        return TRUE;
    }
}

//---------------------------------------------------------------------------


static void mtk_nand_configure_lock (void)
{
    u32 u4WriteColNOB = 2;
    u32 u4WriteRowNOB = 3;
    u32 u4EraseColNOB = 0;
    u32 u4EraseRowNOB = 3;
    DRV_WriteReg16 (NFI_LOCKANOB_REG16,
        (u4WriteColNOB << PROG_CADD_NOB_SHIFT) |
        (u4WriteRowNOB << PROG_RADD_NOB_SHIFT) |
        (u4EraseColNOB << ERASE_CADD_NOB_SHIFT) |
        (u4EraseRowNOB << ERASE_RADD_NOB_SHIFT));

    // Workaround method for ECO1 mt6577
    if (CHIPVER_ECO_1 == g_u4ChipVer)
    {
        int i;
        for (i = 0; i < 16; ++i)
        {
            DRV_WriteReg32 (NFI_LOCK00ADD_REG32 + (i << 1), 0xFFFFFFFF);
            DRV_WriteReg32 (NFI_LOCK00FMT_REG32 + (i << 1), 0xFFFFFFFF);
        }
        //DRV_WriteReg16(NFI_LOCKANOB_REG16, 0x0);
        DRV_WriteReg32 (NFI_LOCKCON_REG32, 0xFFFFFFFF);
        DRV_WriteReg16 (NFI_LOCK_REG16, NFI_LOCK_ON);
    }
}

//---------------------------------------------------------------------------

static void mtk_nand_configure_fdm (u16 u2FDMSize)
{
    NFI_CLN_REG16 (NFI_PAGEFMT_REG16,
        PAGEFMT_FDM_MASK | PAGEFMT_FDM_ECC_MASK);
    NFI_SET_REG16 (NFI_PAGEFMT_REG16, u2FDMSize << PAGEFMT_FDM_SHIFT);
    NFI_SET_REG16 (NFI_PAGEFMT_REG16, u2FDMSize << PAGEFMT_FDM_ECC_SHIFT);
}

//---------------------------------------------------------------------------
static void mtk_nand_set_autoformat (bool bEnable)
{
    if (bEnable)
    {
        NFI_SET_REG16 (NFI_CNFG_REG16, CNFG_AUTO_FMT_EN);
    }
    else
    {
        NFI_CLN_REG16 (NFI_CNFG_REG16, CNFG_AUTO_FMT_EN);
    }
}

//---------------------------------------------------------------------------
static void mtk_nand_command_bp (unsigned command)
{
    u32 timeout;

    switch (command)
    {
    case NAND_CMD_READID:
        /* Issue NAND chip reset command */
        NFI_ISSUE_COMMAND (NAND_CMD_RESET, 0, 0, 0, 0);

        timeout = TIMEOUT_4;

        while (timeout)
            timeout--;

        mtk_nand_reset ();

        /* Disable HW ECC */
        NFI_CLN_REG16 (NFI_CNFG_REG16, CNFG_HW_ECC_EN);

        /* Disable 16-bit I/O */
        NFI_CLN_REG16 (NFI_PAGEFMT_REG16, PAGEFMT_DBYTE_EN);
        NFI_SET_REG16 (NFI_CNFG_REG16, CNFG_READ_EN | CNFG_BYTE_RW);
        mtk_nand_reset ();
        mtk_nand_set_mode (CNFG_OP_SRD);
        mtk_nand_set_command (NAND_CMD_READID);
        mtk_nand_set_address (0, 0, 1, 0);
        DRV_WriteReg16 (NFI_CON_REG16, CON_NFI_SRD);
        while (DRV_Reg32 (NFI_STA_REG32) & STA_DATAR_STATE);
        break;

    default:
        break;
    }
}

//-----------------------------------------------------------------------------
static u8 mtk_nand_read_byte (void)
{
    /* Check the PIO bit is ready or not */
    u32 timeout = TIMEOUT_4;
    WAIT_NFI_PIO_READY (timeout);
    return DRV_Reg8 (NFI_DATAR_REG32);
}

bool getflashid(u8 *nand_id, int longest_id_number)
{
	u8 maf_id=0;
	u8 dev_id=0;
        int i=0;
        u8 *id=nand_id;
	//PDN_Power_CONA_DOWN (PDN_PERI_NFI, FALSE);


	DRV_WriteReg32 (NFI_ACCCON_REG32, NFI_DEFAULT_ACCESS_TIMING);

	DRV_WriteReg16 (NFI_CNFG_REG16, 0);
	DRV_WriteReg16 (NFI_PAGEFMT_REG16, 0);


	mtk_nand_command_bp (NAND_CMD_READID);

	maf_id = mtk_nand_read_byte ();
	dev_id = mtk_nand_read_byte ();

	if(maf_id==0||dev_id==0)
	{
		return FALSE;
	}
	//*id= (dev_id<<8)|maf_id;
    //    *id= (maf_id<<8)|dev_id;
        id[0] = maf_id;
	id[1] = dev_id;
        
        for(i = 2; i < longest_id_number; i++)
            id[i] = mtk_nand_read_byte ();

	return TRUE;
}

int mtk_nand_init (void)
{
    int i, j, busw;
    u16 id;
    u32 ext_id;
    u16 spare_bit = 0;

    u16 spare_per_sector = 16;
    u32 ecc_bit = 4;
    nfi_buf = (unsigned char *)NAND_NFI_BUFFER;

    /* Power on NFI HW component. */
    //PDN_Power_CONA_DOWN (PDN_PERI_NFI, FALSE);
    memset(&devinfo, 0, sizeof(devinfo));

    /* Dynamic Control */
    g_bInitDone = FALSE;
    g_u4ChipVer = DRV_Reg32 (CONFIG_BASE);      /*HW_VER */
    g_kCMD.u4OOBRowAddr = (u32) - 1;

  
    /* Jun Shen, 2011.04.13  */
    /* Note: MT6577 EVB NAND  is mounted on CS0, but FPGA is CS1 */
    DRV_WriteReg16 (NFI_CSEL_REG16, NFI_DEFAULT_CS);
    /* Jun Shen, 2011.04.13  */


    /* Set default NFI access timing control */
    DRV_WriteReg32 (NFI_ACCCON_REG32, NFI_DEFAULT_ACCESS_TIMING);

    DRV_WriteReg16 (NFI_CNFG_REG16, 0);
    DRV_WriteReg16 (NFI_PAGEFMT_REG16, 0);

    /* Reset NFI HW internal state machine and flush NFI in/out FIFO */
    mtk_nand_reset ();

    /* Read the first 4 byte to identify the NAND device */

    g_nand_chip.page_shift = NAND_LARGE_PAGE;
    g_nand_chip.page_size = 1 << g_nand_chip.page_shift;
    g_nand_chip.oobblock = NAND_PAGE_SIZE;
    g_nand_chip.oobsize = NAND_BLOCK_BLKS;

    g_nand_chip.nand_ecc_mode = NAND_ECC_HW;

    mtk_nand_command_bp (NAND_CMD_READID);

    nand_maf_id = mtk_nand_read_byte ();
    nand_dev_id = mtk_nand_read_byte ();
    id = nand_dev_id | (nand_maf_id << 8);

    ext_id1 = mtk_nand_read_byte ();
    ext_id2 = mtk_nand_read_byte ();
    ext_id3 = mtk_nand_read_byte ();

    ext_id = ext_id1 << 16 | ext_id2 << 8 | ext_id3;
    memset(&devinfo, 0, sizeof(devinfo));

    if (!get_device_info(id, ext_id, &devinfo))
    {
        MSG (INIT, "NAND unsupport\n");
        ASSERT(0);
    }

    g_nand_chip.name = devinfo.devciename;
    g_nand_chip.chipsize = devinfo.totalsize << 20;
    g_nand_chip.page_size = devinfo.pagesize; 
    g_nand_chip.page_shift = uffs(g_nand_chip.page_size) - 1;
    g_nand_chip.oobblock = g_nand_chip.page_size;
    g_nand_chip.erasesize = devinfo.blocksize << 10;

    g_nand_chip.bus16 = devinfo.iowidth;
    DRV_WriteReg32(NFI_ACCCON_REG32, devinfo.timmingsetting);
    
    if(!devinfo.sparesize)
    	g_nand_chip.oobsize = (8 << ((ext_id2 >> 2) & 0x01)) * (g_nand_chip.oobblock / 512);
    else
    	g_nand_chip.oobsize = devinfo.sparesize;
    spare_per_sector = g_nand_chip.oobsize/(g_nand_chip.page_size/NAND_SECTOR_SIZE);
  
  	if(spare_per_sector>=28){
  		spare_bit = PAGEFMT_SPARE_28;
    		ecc_bit = 12;
    		spare_per_sector = 28;
  	}else if(spare_per_sector>=27){
  		spare_bit = PAGEFMT_SPARE_27;
    		ecc_bit = 8;
 		spare_per_sector = 27;
  	}else if(spare_per_sector>=26){
  		spare_bit = PAGEFMT_SPARE_26;
    		ecc_bit = 8;
		spare_per_sector = 26;
  	}else if(spare_per_sector>=16){
  		spare_bit = PAGEFMT_SPARE_16;
    		ecc_bit = 4;
		spare_per_sector = 16;
  	}else{
  		MSG(INIT, "[NAND]: NFI not support oobsize: %x\n", spare_per_sector);
    		ASSERT(0);
  	}
  	
  	g_nand_chip.oobsize = spare_per_sector*(g_nand_chip.page_size/NAND_SECTOR_SIZE);
    MSG(INIT, "[NAND]: oobsize: %x\n", g_nand_chip.oobsize);

#if 0
    for (i = 0; nand_flash_ids[i].name != NULL; i++)
    {
        if (nand_dev_id != nand_flash_ids[i].nand_id)
            continue;

        if (!g_nand_chip.name)
            g_nand_chip.name = nand_flash_ids[i].name;

        g_nand_chip.chipsize = nand_flash_ids[i].chip_size << 20;

        /* New devices have all the information in additional id bytes */
        if (!nand_flash_ids[i].page_size)
        {
            int extid;
            /* The 3rd id byte contains non relevant data ATM */
            extid = mtk_nand_read_byte ();
            /* The 4th id byte is the important one */
            extid = mtk_nand_read_byte ();
            /* Calc pagesize */
            g_nand_chip.page_size = 1024 << (extid & 0x3);
            g_nand_chip.page_shift = uffs(g_nand_chip.page_size) - 1;
            g_nand_chip.oobblock = g_nand_chip.page_size;
            extid >>= 2;
            /* Calc oobsize */
            g_nand_chip.oobsize =
                (8 << (extid & 0x03)) * (g_nand_chip.oobblock / 512);
            extid >>= 2;
            /* Calc blocksize. Blocksize is multiples of 64KiB */
            g_nand_chip.erasesize = (64 * 1024) << (extid & 0x03);
            extid >>= 2;
            /* Get buswidth information */
            busw = (extid & 0x01) ? NAND_BUSW_16 : 0;
        }
        else
        {
            /* Old devices have this data hardcoded in the
            * device id table */
            g_nand_chip.erasesize = nand_flash_ids[i].erase_size;
            g_nand_chip.oobblock = nand_flash_ids[i].page_size;
            g_nand_chip.oobsize = g_nand_chip.oobblock / 32;
            busw = nand_flash_ids[i].options & NAND_BUSW_16;
        }

        // substract pool size from chipsize
        g_nand_chip.chipsize -= g_nand_chip.erasesize * (BMT_POOL_SIZE);

        g_nand_chip.bus16 = busw & NAND_BUSW_16 ? NAND_BUS_WIDTH_16 : NAND_BUS_WIDTH_8;

        /* Try to identify manufacturer */
        for (j = 0; nand_manuf_ids[j].id != 0x0; j++)
        {
            if (nand_manuf_ids[j].id == nand_maf_id)
                break;
        }

        /*MSG (INIT,
            "NAND: Manu ID: 0x%x, Chip ID: 0x%x (%s %s)\n",
            nand_maf_id, nand_dev_id, nand_manuf_ids[j].name,
            g_nand_chip.name);
*/
        break;
    }
#endif
    
    // substract pool size from chipsize
    g_nand_chip.chipsize -= g_nand_chip.erasesize * (BMT_POOL_SIZE);
    // g_nand_chip.bus16 = busw & NAND_BUSW_16 ? NAND_BUS_WIDTH_16 : NAND_BUS_WIDTH_8;
/*
    if (!nand_flash_ids[i].name)
    {
        MSG (INIT, "No NAND found!!!\n");
    }
*/
    if (g_nand_chip.bus16 == NAND_BUS_WIDTH_16)
    {
#ifdef  DBG_PRELOADER        
        MSG (INIT, "USE 16 IO\n");
#endif      
        NFI_SET_REG16 (NFI_PAGEFMT_REG16, PAGEFMT_DBYTE_EN);
    }

    //MSG(INIT, "+ mtk_nand_init : Finish device query +\n");

    //if (g_nand_chip.page_shift == NAND_LARGE_PAGE)
    if (g_nand_chip.oobblock == 4096)
    {
        NFI_SET_REG16 (NFI_PAGEFMT_REG16,
            (spare_bit << PAGEFMT_SPARE_SHIFT) |
            PAGEFMT_4K);
        nand_oob = &nand_oob_128;
    }
    else if (g_nand_chip.oobblock == 2048)
    {
        NFI_SET_REG16 (NFI_PAGEFMT_REG16,
            (spare_bit<< PAGEFMT_SPARE_SHIFT) |
            PAGEFMT_2K);
        nand_oob = &nand_oob_64;
    }
    //else if(g_nand_chip.page_shift == NAND_SMALL_PAGE)
    else if (g_nand_chip.oobblock == 512)
    {
        NFI_SET_REG16 (NFI_PAGEFMT_REG16,
            (spare_bit << PAGEFMT_SPARE_SHIFT) |
            PAGEFMT_512);
        nand_oob = &nand_oob_16;
    }

#if 0
    // FIX ME : add nand type detection
    {
        /*
        *  set NAND page format for 2K page
        */
        DRV_WriteReg16 (NFI_PAGEFMT_REG16,
            (8 << PAGEFMT_FDM_ECC_NUM_SHIFT) |
            (8 << PAGEFMT_FDM_NUM_SHIFT) |
            PAGEFMT_SPARE_SIZE_16 | PAGEFMT_PAGE_SIZE_2K);
    }

    DRV_WriteReg16 (NFI_CNFG_REG16, CNFG_HW_ECC_EN);
#endif
    if (g_nand_chip.nand_ecc_mode == NAND_ECC_HW)
    {
        // MSG (INIT, "Use HW ECC\n");
        NFI_SET_REG32 (NFI_CNFG_REG16, CNFG_HW_ECC_EN);
        ECC_Config (ecc_bit);
        mtk_nand_configure_fdm (8);
        mtk_nand_configure_lock ();
    }

    /* Initilize interrupt. Clear interrupt, read clear. */
    DRV_Reg16 (NFI_INTR_REG16);

    /* Interrupt arise when read data or program data to/from AHB is done. */
    DRV_WriteReg16 (NFI_INTR_EN_REG16, 0);

    if ( !(init_bmt(&g_nand_chip, BMT_POOL_SIZE)) )
    {
        // MSG(INIT, "Error: init bmt failed, quit!\n");
        ASSERT(0);
        return 0;
    }

	//u8 idbuff[8];
	//getflashid(idbuff);
    g_nand_chip.chipsize -= g_nand_chip.erasesize * (PMT_POOL_SIZE);
    return 0;
}

//-----------------------------------------------------------------------------
static void mtk_nand_stop_read (void)
{
    NFI_CLN_REG16 (NFI_CON_REG16, CON_NFI_BRD);
    if(g_bHwEcc)
    {
        ECC_Decode_End ();
    }
}

//-----------------------------------------------------------------------------
static void mtk_nand_stop_write (void)
{
    NFI_CLN_REG16 (NFI_CON_REG16, CON_NFI_BWR);
    if(g_bHwEcc)
    {
        ECC_Encode_End ();
    }
}

//-----------------------------------------------------------------------------
static bool mtk_nand_check_dececc_done (u32 u4SecNum)
{
    u32 timeout, dec_mask;
    timeout = 0xffff;
    dec_mask = (1 << u4SecNum) - 1;
    while ((dec_mask != DRV_Reg (ECC_DECDONE_REG16)) && timeout > 0)
        timeout--;
    if (timeout == 0)
    {
        MSG (ERR, "ECC_DECDONE: timeout\n");
        return false;
    }
    return true;
}

//-----------------------------------------------------------------------------
static bool mtk_nand_read_page_data (u32 * buf)
{
    u32 timeout = 0xFFFF;
    u32 u4Size = g_nand_chip.oobblock;
    u32 i;
    u32 *pBuf32;

#if (USE_AHB_MODE)
    pBuf32 = (u32 *) buf;
    NFI_CLN_REG16 (NFI_CNFG_REG16, CNFG_BYTE_RW);

    DRV_Reg16 (NFI_INTR_REG16);
    DRV_WriteReg16 (NFI_INTR_EN_REG16, INTR_AHB_DONE_EN);
    NFI_SET_REG16 (NFI_CON_REG16, CON_NFI_BRD);

    while ( !(DRV_Reg16(NFI_INTR_REG16) & INTR_AHB_DONE) )
    {
        timeout--;
        if (0 == timeout)
        {
            return FALSE;     //4  // AHB Mode Time Out!
        }
    }
    
    timeout = 0xFFFF;
    while ( (u4Size >> 9) > ((DRV_Reg16(NFI_BYTELEN_REG16) & 0xf000) >> 12) )
    {
        timeout--;
        if (0 == timeout)
        {
            return FALSE;     //4  // AHB Mode Time Out!
        }
    }

#else
    NFI_CLN_REG16 (NFI_CNFG_REG16, CNFG_BYTE_RW);
    //DRV_WriteReg32 (NFI_STRADDR_REG32, 0);
    NFI_SET_REG16 (NFI_CON_REG16, CON_NFI_BRD);

    //pBuf32 = (u32*)g_nand_buf;
    pBuf32 = (u32 *) buf;

    for (i = 0; (i < (u4Size >> 2)) && (timeout > 0);)
    {
	if (DRV_Reg16(NFI_PIO_DIRDY_REG16) & 1)
        {
            *pBuf32++ = DRV_Reg32 (NFI_DATAR_REG32);
            i++;
        }
        else
        {
            timeout--;
        }
        if (0 == timeout)
        {
            return FALSE;   //4 // MCU  Mode Time Out!
        }
    }
#endif
    return TRUE;
}

//-----------------------------------------------------------------------------
static bool mtk_nand_write_page_data (u32 * buf)
{
    u32 timeout = 0xFFFF;
    u32 u4Size = g_nand_chip.oobblock;

#if (USE_AHB_MODE)
    u32 *pBuf32;
    pBuf32 = (u32 *) buf;

    NFI_CLN_REG16 (NFI_CNFG_REG16, CNFG_BYTE_RW);

    DRV_Reg16 (NFI_INTR_REG16);
    DRV_WriteReg16 (NFI_INTR_EN_REG16, INTR_AHB_DONE_EN);
    NFI_SET_REG16 (NFI_CON_REG16, CON_NFI_BWR);
    while ( !(DRV_Reg16(NFI_INTR_REG16) & INTR_AHB_DONE) )
    {
        timeout--;
        if (0 == timeout)
        {
            return FALSE;     //4  // AHB Mode Time Out!
        }
    }

#else
    u32 i;
    u32 *pBuf32;
    pBuf32 = (u32 *) buf;

    NFI_CLN_REG16 (NFI_CNFG_REG16, CNFG_BYTE_RW);
    NFI_SET_REG16 (NFI_CON_REG16, CON_NFI_BWR);

    for (i = 0; (i < (u4Size >> 2)) && (timeout > 0);)
    {
        //if (FIFO_WR_REMAIN (DRV_Reg16 (NFI_FIFOSTA_REG16)) <= 12)
			if (DRV_Reg16(NFI_PIO_DIRDY_REG16) & 1)
        {
            DRV_WriteReg32 (NFI_DATAW_REG32, *pBuf32++);
            i++;
        }
        else
        {
            timeout--;
        }

        if (0 == timeout)
        {
            return FALSE;   //4 // MCU Mode Time Out!
        }
    }
#endif
    return TRUE;
}


//-----------------------------------------------------------------------------
static void mtk_nand_read_fdm_data (u32 u4SecNum, u8 * spare_buf)
{
    u32 i;
    //u32* pBuf32 = (u32*)g_nand_spare;
    u32 *pBuf32 = (u32 *) spare_buf;

    for (i = 0; i < u4SecNum; ++i)
    {
        *pBuf32++ = DRV_Reg32 (NFI_FDM0L_REG32 + (i << 3));
        *pBuf32++ = DRV_Reg32 (NFI_FDM0M_REG32 + (i << 3));
        }
}

//-----------------------------------------------------------------------------
static void
mtk_nand_write_fdm_data (u32 u4SecNum, u8 *oob)
{
    u32 i;
    u32 *pBuf32 = (u32 *)oob;

    for (i = 0; i < u4SecNum; ++i)
    {
        DRV_WriteReg32 (NFI_FDM0L_REG32 + (i << 3), *pBuf32++);
        DRV_WriteReg32 (NFI_FDM0M_REG32 + (i << 3), *pBuf32++);
    }
}

//---------------------------------------------------------------------------
static bool mtk_nand_ready_for_read (u32 page_addr, u32 sec_num, u8 *buf)
{
    u32 u4RowAddr = page_addr;
    u32 colnob = 2;
    u32 rownob = devinfo.addr_cycle - colnob;
    bool bRet = FALSE;

    if (!mtk_nand_reset ())
    {
        goto cleanup;
    }

    /* Enable HW ECC */
    NFI_SET_REG16 (NFI_CNFG_REG16, CNFG_HW_ECC_EN);

    mtk_nand_set_mode (CNFG_OP_READ);
    NFI_SET_REG16 (NFI_CNFG_REG16, CNFG_READ_EN);
    DRV_WriteReg16 (NFI_CON_REG16, sec_num << CON_NFI_SEC_SHIFT);

#if USE_AHB_MODE
    NFI_SET_REG16 (NFI_CNFG_REG16, CNFG_AHB);
#else
    NFI_CLN_REG16 (NFI_CNFG_REG16, CNFG_AHB);
#endif
    DRV_WriteReg32(NFI_STRADDR_REG32, buf);
    if(g_bHwEcc)
    {
	NFI_SET_REG16 (NFI_CNFG_REG16, CNFG_HW_ECC_EN);
    }else{
	NFI_CLN_REG16 (NFI_CNFG_REG16, CNFG_HW_ECC_EN);
    }

    mtk_nand_set_autoformat (TRUE);
    if(g_bHwEcc)
    {
        ECC_Decode_Start ();
    }
    if (!mtk_nand_set_command (NAND_CMD_READ0))
    {
        goto cleanup;
    }

    //1 FIXED ME: For Any Kind of AddrCycle
    if (!mtk_nand_set_address (0, u4RowAddr, colnob, rownob))
    {
        goto cleanup;
    }

    if (!mtk_nand_set_command (NAND_CMD_READSTART))
    {
        goto cleanup;
    }

    if (!mtk_nand_status_ready (STA_NAND_BUSY))
    {
        goto cleanup;
    }

    bRet = TRUE;

cleanup:
    return bRet;
}

//-----------------------------------------------------------------------------
static bool mtk_nand_ready_for_write (u32 page_addr, u32 sec_num, u8 *buf)
{
    bool bRet = FALSE;
    u32 u4RowAddr = page_addr;
    u32 colnob = 2;
    u32 rownob = devinfo.addr_cycle - colnob;

    /* Reset NFI HW internal state machine and flush NFI in/out FIFO */
    if (!mtk_nand_reset ())
    {
        return FALSE;
    }

    mtk_nand_set_mode (CNFG_OP_PRGM);

    NFI_CLN_REG16 (NFI_CNFG_REG16, CNFG_READ_EN);

    DRV_WriteReg16 (NFI_CON_REG16, sec_num << CON_NFI_SEC_SHIFT);

#if USE_AHB_MODE
    NFI_SET_REG16 (NFI_CNFG_REG16, CNFG_AHB);
    DRV_WriteReg32(NFI_STRADDR_REG32, buf);
#else
    NFI_CLN_REG16 (NFI_CNFG_REG16, CNFG_AHB);
#endif

    if(g_bHwEcc)
    {
        NFI_SET_REG16 (NFI_CNFG_REG16, CNFG_HW_ECC_EN);
    }else{
	NFI_CLN_REG16 (NFI_CNFG_REG16, CNFG_HW_ECC_EN);
    }
    mtk_nand_set_autoformat (TRUE);
    if(g_bHwEcc)
    {
	ECC_Encode_Start ();
    }

    if (!mtk_nand_set_command (NAND_CMD_SEQIN))
    {
        goto cleanup;
    }

    //1 FIXED ME: For Any Kind of AddrCycle
    if (!mtk_nand_set_address (0, u4RowAddr, colnob, rownob))
    {
        goto cleanup;
    }

    if (!mtk_nand_status_ready (STA_NAND_BUSY))
    {
        goto cleanup;
    }

    bRet = TRUE;
cleanup:

    return bRet;
}


//#############################################################################
//# NAND Driver : Page Read
//#
//# NAND Page Format (Large Page 2KB)
//#  |------ Page:2048 Bytes ----->>||---- Spare:64 Bytes -->>|
//#
//# Parameter Description:
//#     page_addr               : specify the starting page in NAND flash
//#
//#############################################################################
int mtk_nand_read_page_hwecc (unsigned int logical_addr, char *buf)
{
    int i, start, len, offset = 0;
    int block = logical_addr / g_nand_chip.erasesize;
    int page_in_block = PAGE_NUM(logical_addr) % NAND_BLOCK_BLKS;
    int mapped_block;
    u8 *oob = buf + g_nand_chip.page_size;

    mapped_block = get_mapping_block_index(block);

    if (!mtk_nand_read_page_hw(page_in_block + mapped_block * NAND_BLOCK_BLKS, buf, g_nand_spare)) // g_nand_spare
        return FALSE;
    
    for (i = 0; i < MTD_MAX_OOBFREE_ENTRIES && nand_oob->oobfree[i].length; i++)
    {
        /* Set the reserved bytes to 0xff */
        start = nand_oob->oobfree[i].offset;
        len = nand_oob->oobfree[i].length;
        memcpy (oob + offset, g_nand_spare + start, len);
        offset += len;
    }

    return true;
}

int mtk_nand_read_page_hw (u32 page, u8 *dat, u8 *oob)
{
    bool bRet = TRUE;
    u8 *pPageBuf;
    u32 u4SecNum = g_nand_chip.oobblock >> NAND_PAGE_SHIFT;
    // u32 page_addr = PAGE_NUM (logical_addr);

    pPageBuf = (u8 *)dat;

    if (mtk_nand_ready_for_read (page, u4SecNum, pPageBuf))
    {
        if (!mtk_nand_read_page_data ((u32 *) pPageBuf))
        {
            bRet = FALSE;
        }

        if (!mtk_nand_status_ready (STA_NAND_BUSY))
        {
            bRet = FALSE;
        }
	if(g_bHwEcc)
        {
            if (!mtk_nand_check_dececc_done (u4SecNum) )
            {
                bRet = FALSE;
            }
	}
        mtk_nand_read_fdm_data (u4SecNum, oob);
	if(g_bHwEcc)
        {
            if (!mtk_nand_check_bch_error(pPageBuf, u4SecNum - 1, page))
            {
                MSG(ERASE, "check bch error !\n");
                bRet = FALSE;
            }
        }
        mtk_nand_stop_read ();
    }
#if 0
    for (i = 0; mtk_nand_oob.oobfree[i][1]; i++)
    {
        /* Set the reserved bytes to 0xff */
        start = mtk_nand_oob.oobfree[i][0];
        len = mtk_nand_oob.oobfree[i][1];
        memcpy ((buf + 2048 + i), (g_nand_spare + start), len);
        offset += len;
    }

    for (i = 0; i < 64; i++)
    {
        MSG (ERASE, "spare area is : [%d] = 0x%x\n", *(buf + 2048 + i));
    }
#endif
    return bRet;
}

//#############################################################################
//# NAND Driver : Page Write
//#
//# NAND Page Format (Large Page 2KB)
//#  |------ Page:2048 Bytes ----->>||---- Spare:64 Bytes -->>|
//#
//# Parameter Description:
//#     page_addr               : specify the starting page in NAND flash
//#
//#############################################################################

int mtk_nand_write_page_hwecc (unsigned int logical_addr, char *buf)
{
    u16 block = logical_addr / g_nand_chip.erasesize;
    u16 mapped_block = get_mapping_block_index(block);
    u16 page_in_block = PAGE_NUM(logical_addr) % NAND_BLOCK_BLKS;
    u8 *oob = buf + g_nand_chip.oobblock;
    int i;
    int start, len, offset;

    // oob position
    for (i = 0; i < sizeof(g_nand_spare); i++)
        *(g_nand_spare + i) = 0xFF;

    offset = 0;
    for (i = 0; i < MTD_MAX_OOBFREE_ENTRIES && nand_oob->oobfree[i].length; i++)
    {
        /* Set the reserved bytes to 0xff */
        start = nand_oob->oobfree[i].offset;
        len = nand_oob->oobfree[i].length;
        memcpy ((g_nand_spare + start), (oob + offset), len);
        offset += len;
    }

    // write bad index into oob
    if (mapped_block != block)
    {
        set_bad_index_to_oob(g_nand_spare, block);
    }
    else 
    {
        set_bad_index_to_oob(g_nand_spare, FAKE_INDEX);
    }

    if (!mtk_nand_write_page_hw(page_in_block + mapped_block * NAND_BLOCK_BLKS, 
            buf, g_nand_spare))
    {
        MSG(INIT, "write fail happened @ block 0x%x, page 0x%x\n", mapped_block, page_in_block);
        return update_bmt( (page_in_block + mapped_block * NAND_BLOCK_BLKS) * g_nand_chip.oobblock,
                UPDATE_WRITE_FAIL, buf, g_nand_spare);
    }

    return TRUE;
}

int mtk_nand_write_page_hw(u32 page, u8 *dat, u8 *oob)
{
    bool bRet = TRUE;
    u32 pagesz = g_nand_chip.oobblock;
    u32 timeout, u4SecNum = pagesz >> NAND_PAGE_SHIFT;

    int i, j, start, len;
    bool empty = TRUE;
    u8 oob_checksum = 0;
 
    for (i = 0; i < MTD_MAX_OOBFREE_ENTRIES && nand_oob->oobfree[i].length; i++)
    {
        /* Set the reserved bytes to 0xff */
        start = nand_oob->oobfree[i].offset;
        len = nand_oob->oobfree[i].length;
        for (j = 0; j < len; j++)
        {
            oob_checksum ^= oob[start + j];
            if (oob[start + j] != 0xFF)
                empty = FALSE;
        }
    }

    if (!empty)
    {
        oob[nand_oob->oobfree[i-1].offset + nand_oob->oobfree[i-1].length] = oob_checksum;
    }

    while (DRV_Reg32 (NFI_STA_REG32) & STA_NAND_BUSY);

    if (mtk_nand_ready_for_write (page, u4SecNum, dat))
    {
        mtk_nand_write_fdm_data (u4SecNum, oob);
        if (!mtk_nand_write_page_data ((u32 *)dat))
        {
            bRet = FALSE;
        }
        if (!mtk_nand_check_RW_count (g_nand_chip.oobblock))
        {
            bRet = FALSE;
        }
        mtk_nand_stop_write ();
        mtk_nand_set_command (NAND_CMD_PAGEPROG);
        mtk_nand_status_ready(STA_NAND_BUSY);
        return mtk_nand_read_status();
    }
    else
    {
        return FALSE;
    }

#if 0
    for (i = 0; i < 64; i++)
    {
        *(g_nand_buf + 2048 + i) = 0xA;
    }

    mtk_nand_read_page_hwecc (logical_addr, g_nand_buf);

    for (i = 0; i < 32; i++)
    {
        if (*(g_nand_spare + i) != *(g_nand_buf + 2048 + i))
            MSG (ERASE,
            "g_nand_spare [%d] : 0x%x, g_nand_buf [%d] : 0x%x\n", i,
            *(g_nand_spare + i), i, *(g_nand_buf + 2048 + i));
    }
#endif

    return bRet;
}
// static char nfi_buf[NFI_BUF_MAX_SIZE];

unsigned int nand_block_bad (unsigned int logical_addr)
{
    int block = logical_addr / g_nand_chip.erasesize;
    int mapped_block = get_mapping_block_index(block);

    if (nand_block_bad_hw(mapped_block * g_nand_chip.erasesize))
    {
        if (update_bmt(mapped_block * g_nand_chip.erasesize, UPDATE_UNMAPPED_BLOCK, NULL, NULL))
        {
            return logical_addr;        // return logical address
        }
        return logical_addr + g_nand_chip.erasesize;
    }

    return logical_addr;
}

bool nand_block_bad_hw (u32 logical_addr)
{
    bool bRet = FALSE;
    u32 page = logical_addr / g_nand_chip.oobblock;

    int i, page_num = (g_nand_chip.erasesize / g_nand_chip.oobblock);
    // int iaCheckPageID[4];
    unsigned char *pspare;
    char *tmp = (char *) nfi_buf;
    memset (tmp, 0x0, g_nand_chip.oobblock + g_nand_chip.oobsize);

    u32 u4SecNum = g_nand_chip.oobblock >> NAND_PAGE_SHIFT;
    page &= ~(page_num - 1);

    if (mtk_nand_ready_for_read (page, u4SecNum, tmp))
    {
        if (!mtk_nand_read_page_data ((u32 *) tmp))
        {
            bRet = FALSE;
        }

        if (!mtk_nand_status_ready (STA_NAND_BUSY))
        {
            bRet = FALSE;
        }

        if (!mtk_nand_check_dececc_done (u4SecNum))
        {
            bRet = FALSE;
        }

        mtk_nand_read_fdm_data (u4SecNum, g_nand_spare);

        if (!mtk_nand_check_bch_error(tmp, u4SecNum - 1, page))
        {
            MSG(ERASE, "check bch error !\n");
            bRet = FALSE;
        }

        mtk_nand_stop_read ();
    }

        pspare = g_nand_spare;

    if (pspare[0] != 0xFF || pspare[8] != 0xFF || pspare[16] != 0xFF
        || pspare[24] != 0xFF)
    {
        bRet = TRUE;
        // break;
    }

    return bRet;
/*
    if (bRet)
    {
        //MSG(BAD, "BAD BLOCK detected at 0x%x\n", logical_addr);
        print ("BAD BLOCK detected at 0x%x\n", logical_addr);
        return LOGICAL_ADDR (page_addr + page_num);
    }
    else
    {
        //MSG(BAD, "NO BAD BLOCK at 0x%x\n", logical_addr);
        return logical_addr;
    }
*/
}

bool mark_block_bad (u32 logical_addr)
{
    int block = logical_addr / g_nand_chip.erasesize;
    int mapped_block = get_mapping_block_index(block);

    return mark_block_bad_hw(mapped_block * g_nand_chip.erasesize);
}

bool mark_block_bad_hw(u32 offset)
{
    bool bRet = FALSE;
    u32 index;
    u32 page_addr = offset / g_nand_chip.oobblock;
    u32 u4SecNum = g_nand_chip.oobblock >> NAND_PAGE_SHIFT;
    unsigned char *pspare;
    int i, page_num = (g_nand_chip.erasesize / g_nand_chip.oobblock);
    unsigned char buf[2048];

    //for(index=0; index<2048; index++)
    //    buf[index]=0xAA;

    //mtk_nand_erase(logical_addr);
    //delay(1000);

#if 0
    char *tmp = (char *) nfi_buf;
#endif

    for (index = 0; index < 64; index++)
        *(g_nand_spare + index) = 0xFF;

    pspare = g_nand_spare;

    for (index = 8, i = 0; i < 4; i++)
        pspare[i * index] = 0x0;

#if 0
    MSG (BAD, "==========Spare area=========\n");
    for (i = 0; i < 64; i++)
      {
          MSG (BAD, "0x%x ", pspare[i]);
          if ((i % 8) == 7)
              MSG (BAD, "\n");
      }
    MSG (BAD, "=============================\n");
#endif

    page_addr &= ~(page_num - 1);
    MSG (BAD, "Mark bad block at 0x%x\n", page_addr);
    while (DRV_Reg32 (NFI_STA_REG32) & STA_NAND_BUSY);

    if (mtk_nand_ready_for_write (page_addr, u4SecNum, buf))
    {
          mtk_nand_write_fdm_data (u4SecNum, g_nand_spare);
          if (!mtk_nand_write_page_data ((u32 *) & buf))
          {
                bRet = FALSE;
          }
          if (!mtk_nand_check_RW_count (g_nand_chip.oobblock))
          {
                bRet = FALSE;
          }
          mtk_nand_stop_write ();
          mtk_nand_set_command (NAND_CMD_PAGEPROG);
          mtk_nand_status_ready(STA_NAND_BUSY);
    }
    else
    {
        return FALSE;
    }

#if 0
    for (index = 0; index < 64; index++)
        *(g_nand_spare + index) = 0xFF;

    mtk_nand_read_page_hwecc (LOGICAL_ADDR (page_addr), tmp);

    MSG (BAD, "==========After Mark=========\n");
    for (i = 0; i < 64; i++)
      {
          MSG (BAD, "0x%x ", pspare[i]);
          if ((i % 8) == 7)
              MSG (BAD, "\n");
      }
    MSG (BAD, "=============================\n");
#endif

    for (index = 0; index < 64; index++)
        *(g_nand_spare + index) = 0xFF;
}

//#############################################################################
//# NAND Driver : Page Write
//#
//# NAND Page Format (Large Page 2KB)
//#  |------ Page:2048 Bytes ----->>||---- Spare:64 Bytes -->>|
//#
//# Parameter Description:
//#     page_addr               : specify the starting page in NAND flash
//#
//#############################################################################
int mtk_nand_erase (u32 logical_addr)
{
    int block = logical_addr / g_nand_chip.erasesize;
    int mapped_block = get_mapping_block_index(block);

    if (!mtk_nand_erase_hw(mapped_block * g_nand_chip.erasesize))
    {
        MSG(INIT, "erase block 0x%x failed\n", mapped_block);
        return update_bmt(mapped_block * g_nand_chip.erasesize, UPDATE_ERASE_FAIL, NULL, NULL);
    }

    return TRUE;
}

bool mtk_nand_erase_hw (u32 offset)
{
    bool bRet = TRUE;
    u32 timeout, u4SecNum = g_nand_chip.oobblock >> NAND_PAGE_SHIFT;
    u32 rownob = devinfo.addr_cycle - 2;
    u32 page_addr = offset / g_nand_chip.oobblock;

    //MSG(ERASE, "logical_addr addr is 0x%x\n", logical_addr);
    //MSG(ERASE, "erase page addr is 0x%x\n", page_addr);

    if (nand_block_bad_hw(offset))
    {
        return FALSE;
    }

    mtk_nand_reset ();
    mtk_nand_set_mode (CNFG_OP_ERASE);
    mtk_nand_set_command (NAND_CMD_ERASE1);
    mtk_nand_set_address (0, page_addr, 0, rownob);

    mtk_nand_set_command (NAND_CMD_ERASE2);
    if (!mtk_nand_status_ready(STA_NAND_BUSY))
    {
        return FALSE;
    }

    if (!mtk_nand_read_status())
    {
        return FALSE;
    }
    return bRet;
}

bool mtk_nand_wait_for_finish (void)
{
    while (DRV_Reg32 (NFI_STA_REG32) & STA_NAND_BUSY);
    return TRUE;
}

