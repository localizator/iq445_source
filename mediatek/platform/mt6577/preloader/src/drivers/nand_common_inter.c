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
#include "blkdev.h"
#include "nand.h"
#include "nand_common_inter.h"
#include "part.h"
#include "partition_define.h"


/**************************************************************************
*  DEBUG CONTROL
**************************************************************************/
#define NAND_INTER_DBG_LOG    0

/**************************************************************************
*  Common Nand Interface STRing
**************************************************************************/
#define NAND_INTER_STR        "<NAND_INTER>"    /* Module Log Prefix */

/**************************************************************************
*  MACRO DEFINITION
**************************************************************************/
#if NAND_INTER_DBG_LOG
#define LOG                   print
#else
#define LOG
#endif

/**************************************************************************
*  EXTERNAL DECLARATION
**************************************************************************/
extern struct nand_chip g_nand_chip;
extern u8 g_nand_spare[];
extern struct nand_ecclayout *nand_oob;

static blkdev_t g_nand_bdev;

u32 PAGE_SIZE;
u32 BLOCK_SIZE;

U8 Bad_Block_Table[8192] = {0};

/**************************************************************************
*  MACRO LIKE FUNCTION
**************************************************************************/
static inline u32 PAGE_NUM (u32 logical_size)
{
    return ((unsigned long) (logical_size) / PAGE_SIZE);
}
static inline u32 LOGICAL_ADDR (u32 page_addr)
{
    return ((unsigned long) (page_addr) * PAGE_SIZE);
} 
static inline u32 BLOCK_ALIGN (u32 logical_addr)
{
    return (((u32) (logical_addr / BLOCK_SIZE)) * BLOCK_SIZE);
}

static int nand_bread(blkdev_t *bdev, u32 blknr, u32 blks, u8 *buf)
{
    u32 i;
    u32 offset = blknr * bdev->blksz;

    for (i = 0; i < blks; i++) {
        offset = nand_read_data(buf, offset);
        offset += bdev->blksz;
        buf += bdev->blksz;
    }
    return 0;
}

static int nand_bwrite(blkdev_t *bdev, u32 blknr, u32 blks, u8 *buf)
{
    u32 i;
    u32 offset = blknr * bdev->blksz;

    for (i = 0; i < blks; i++) {
        offset = nand_write_data(buf, offset);
        offset += bdev->blksz;
        buf += bdev->blksz;
    }
    return 0;
}

// ==========================================================
// NAND Common Interface - Init
// ==========================================================

u32 nand_init_device(void)
{
    if (!blkdev_get(BOOTDEV_NAND)) {
        mtk_nand_reset_descriptor ();
        mtk_nand_init ();

        PAGE_SIZE = (u32) g_nand_chip.page_size;
        BLOCK_SIZE = (u32) g_nand_chip.erasesize;

        memset(&g_nand_bdev, 0, sizeof(blkdev_t));
        g_nand_bdev.blksz   = g_nand_chip.page_size;
        g_nand_bdev.erasesz = g_nand_chip.erasesize;
        g_nand_bdev.blks    = g_nand_chip.chipsize;
        g_nand_bdev.bread   = nand_bread;
        g_nand_bdev.bwrite  = nand_bwrite;
        g_nand_bdev.blkbuf  = (u8*)STORAGE_BUFFER_ADDR;
        g_nand_bdev.type    = BOOTDEV_NAND;
        blkdev_register(&g_nand_bdev);
    }
    
    return 0;
}

void Invert_Bits(u8* buff_ptr, u32 bit_pos)
{
    u32 byte_pos   = 0;
    u8  byte_val   = 0;
   u8  temp_val   = 0;
    u32 invert_bit = 0;

    byte_pos = bit_pos >> 3;
    invert_bit = bit_pos & ((1<<3)-1);
    byte_val = buff_ptr[byte_pos];    
    temp_val = byte_val & (1 << invert_bit);
    
    if(temp_val > 0)
        byte_val &= ~temp_val;
    else
        byte_val |= (1 << invert_bit);
    buff_ptr[byte_pos] = byte_val;    
}
void compare_page(u8 * testbuff, u8 * sourcebuff,u32 length,char *s){
	u32 errnum=0;
	u32 ii=0;
	u32 index;
	printf("%s",s);
	for(index=0;index<length;index++){				
		if(testbuff[index]!=sourcebuff[index]){
			u8 t=sourcebuff[index]^testbuff[index];
			for(ii=0;ii<8;ii++){
				if((t>>ii)&0x1==1){
					errnum++;
				}
			}
			printf(" ([%d]=%x) != ([%d]=%x )",index,sourcebuff[index],index,testbuff[index]);
		}

	}
	if(errnum>0){
		printf(": page have %d mismatch bits\n",errnum);
	}else{
		printf(" :the two buffers are same!\n");
	}
}

u8 empty_page(u8 * sourcebuff,u32 length)
{
	u32 index=0;
	for(index=0;index<length;index++){
		if(sourcebuff[index]!=0xFF){
			return 0;
		}
	}
	return 1;
}
u32 __nand_ecc_test(u32 offset, u32 max_ecc_capable){

	int ecc_level=max_ecc_capable;
	int sec_num=g_nand_chip.page_size>>9;
	u32 sec_size=g_nand_chip.page_size/sec_num;
	u32 NAND_MAX_PAGE_LENGTH=g_nand_chip.page_size+8*sec_num;	
	u32 chk_bit_len= 64*4;
	u32 page_per_blk=g_nand_chip.erasesize/g_nand_chip.page_size;
	u32 sec_index,curr_error_bit,err_bits_per_sec,page_idx,errbits,err;

	u8 *testbuff=malloc(NAND_MAX_PAGE_LENGTH);
	u8 *sourcebuff=malloc(NAND_MAX_PAGE_LENGTH);
	u8 empty;
	
	for(err_bits_per_sec=1;err_bits_per_sec<=ecc_level;err_bits_per_sec++){		
		printf("~~~start test ecc correct in ");
		#if USE_AHB_MODE
		printf(" AHB mode");
		#else
		printf(" MCU mode");
		#endif
		printf(", every sector have %d bit error~~~\n",err_bits_per_sec);		
		for(curr_error_bit=0; curr_error_bit < chk_bit_len && offset<g_nand_chip.chipsize; offset+=g_nand_chip.page_size){
			memset(testbuff,0x0a,NAND_MAX_PAGE_LENGTH);
			memset(sourcebuff,0x0b,NAND_MAX_PAGE_LENGTH);
			g_bHwEcc=TRUE;
			nand_read_data(sourcebuff,offset);
			empty=empty_page(sourcebuff,g_nand_chip.page_size) ;
			if(empty){
				printf("page %d is empty\n",offset/g_nand_chip.page_size);
				memset(sourcebuff,0x0c,NAND_MAX_PAGE_LENGTH);
				nand_write_data(sourcebuff,offset);
				nand_read_data(sourcebuff,offset);
			}
			err=(DRV_Reg32 (ECC_DECENUM_REG32) &  0xFFFF);
			if(err){
				printf("skip the page %d, because it is empty ( %d )or already have error bits (%x)!\n",offset/g_nand_chip.page_size,empty,err);
			}else{
				printf("~~~start test ecc correct in Page 0x%x ~~~\n",offset/g_nand_chip.page_size);
				memcpy(testbuff, sourcebuff, NAND_MAX_PAGE_LENGTH);
				for(sec_index = 0 ; sec_index < sec_num ; sec_index++){
					//printf("insert err bit @ page %d:sector %d : bit ",page_idx+offset/g_nand_chip.page_size,sec_index);
					for(errbits=0;errbits<err_bits_per_sec;errbits++){
						Invert_Bits(((u8 *)testbuff)+sec_index*sec_size, curr_error_bit);	
						//printf("%d, ",curr_error_bit);
						curr_error_bit++;
					}
					//printf("\n");
				}
				g_bHwEcc=FALSE;
				nand_write_data (testbuff,offset);
				compare_page(testbuff, sourcebuff,NAND_MAX_PAGE_LENGTH,"source and test buff check ");
				g_bHwEcc=TRUE;
				nand_read_data(testbuff,offset);
				compare_page(testbuff, sourcebuff,NAND_MAX_PAGE_LENGTH,"read back check ");
			}
		}
	}

	free(testbuff);
	free(sourcebuff);
	
}
u32 nand_ecc_test(void)
{
	part_t *part = part_get(PART_UBOOT);
	u32 offset = (part->startblk) * g_nand_chip.page_size;
	__nand_ecc_test(offset,4);
	

	part_t *part2 = part_get(PART_BOOTIMG);
	offset = (part2->startblk) * g_nand_chip.page_size;
	__nand_ecc_test(offset,4);
	return 0;
}


u32 nand_get_device_id(u8 *id, u32 len)
{
    u8 buf[16];
    
    if (TRUE != getflashid(buf, len))
        return -1;

    len = len > 16 ? 16 : len;

    memcpy (id, buf, len);

    return 0;
}

/* LEGACY - TO BE REMOVED { */
// ==========================================================
// NAND Common Interface - Correct R/W Address
// ==========================================================
u32 nand_find_safe_block (u32 offset)
{

    u32 original_offset = offset;
    u32 new_offset = 0;
	unsigned int blk_index = 0;
	static BOOL Bad_Block_Table_init = FALSE;

	if(Bad_Block_Table_init==FALSE)
	{
		Bad_Block_Table_init = TRUE;
		memset(Bad_Block_Table,0,sizeof(Bad_Block_Table));
		print("Bad_Block_Table init, sizeof(Bad_Block_Table)= %d \n",sizeof(Bad_Block_Table));
	}

	blk_index = BLOCK_ALIGN (offset)/BLOCK_SIZE;
	if(Bad_Block_Table[blk_index] == 1)
	{
		return offset;
	}
    // new_offset is block alignment
    new_offset = nand_block_bad (BLOCK_ALIGN (offset));

    // find next block until the block is good
    while (new_offset != BLOCK_ALIGN (offset))
    {     
        offset = new_offset;
        new_offset = nand_block_bad (BLOCK_ALIGN (offset));
    }

    if (original_offset != offset)
    {     
    	Bad_Block_Table[(original_offset/BLOCK_SIZE)] = 2;
        LOG ("offset (0x%x) is bad block. next safe block is (0x%x)\n",original_offset, offset);
    }
	
	Bad_Block_Table[(BLOCK_ALIGN (offset)/BLOCK_SIZE)] = 1;

    return offset;
}
/* LEGACY - TO BE REMOVED } */

// ==========================================================
// NAND Common Interface - Read Function
// ==========================================================
u32 nand_read_data (u8 * buf, u32 offset)
{

    // make sure the block is safe to flash
    offset = nand_find_safe_block (offset);

    if (mtk_nand_read_page_hwecc (offset, buf) == FALSE)
    {     
        print ("nand_read_data fail\n");
        return -1;
    }

    return offset;
}

// ==========================================================
// NAND Common Interface - Write Function
// ==========================================================
u32 nand_write_data (u8 * buf, u32 offset)
{
    // make sure the block is safe to flash
    offset = nand_find_safe_block (offset);

    if (mtk_nand_write_page_hwecc (offset, buf) == FALSE)
    {
        print ("nand_write_data fail\n");
        ASSERT (0);
    }

    return offset;
}


// ==========================================================
// NAND Common Interface - Erase Function
// ==========================================================
bool nand_erase_data (u32 offset, u32 offset_limit, u32 size)
{

    u32 img_size = size;
    u32 tpgsz;
    u32 tblksz;
    u32 cur_offset;
    u32 i = 0;

    // do block alignment check
    if (offset % BLOCK_SIZE != 0)
    {
        print ("offset must be block alignment (0x%x)\n", BLOCK_SIZE);
        ASSERT (0);
    }

    // calculate block number of this image
    if ((img_size % BLOCK_SIZE) == 0)
    {
        tblksz = img_size / BLOCK_SIZE;
    }
    else
    {
        tblksz = (img_size / BLOCK_SIZE) + 1;
    }

    print ("[ERASE] image size = 0x%x\n", img_size);
    print ("[ERASE] the number of nand block of this image = %d\n", tblksz);

    // erase nand block
    cur_offset = offset;
    while (tblksz != 0)
    {
        if (mtk_nand_erase (cur_offset) == FALSE)
        {
            print ("[ERASE] erase fail\n");
            mark_block_bad (cur_offset);
            //ASSERT (0);
        }
        cur_offset += BLOCK_SIZE;

        tblksz--;

        if (tblksz != 0 && cur_offset >= offset_limit)
        {
            print ("[ERASE] cur offset (0x%x) exceeds erase limit address (0x%x)\n", cur_offset, offset_limit);
            return TRUE;
        }
    }


    return TRUE;
}
#if CFG_LEGACY_USB_DOWNLOAD

// ==========================================================
// NAND Common Interface - Check If Device Is Ready To Use
// ==========================================================
void nand_wait_ready (void)
{
    // wait for NAND flashing complete
    mtk_nand_wait_for_finish ();
}



// ==========================================================
// NAND Common Interface - Checksum Calculation Body (skip HW ECC area)
// ==========================================================
u32 nand_chksum_body (u32 chksm, char *buf, u32 pktsz)
{
    u32 i, spare_start, spare_offset = 0, spare_len;
    struct nand_oobfree *oobfree = &nand_oob->oobfree[0];

     for (i = 0; i < MTD_MAX_OOBFREE_ENTRIES && oobfree->length; i++, oobfree++)
    {
        /* Set the reserved bytes to 0xff */
        spare_start = oobfree->offset;
        spare_len = oobfree->length;
        memcpy ((buf + g_nand_chip.page_size + spare_offset), (g_nand_spare + spare_start), spare_len);
        spare_offset += spare_len;
    }

    /* checksum algorithm core, simply exclusive or */
    // skip spare because FAT format image doesn't have any spare region
    for (i = 0; i < pktsz - g_nand_chip.oobsize; i++)
    {    chksm ^= buf[i];
    }

    return chksm;
}


// ==========================================================
// NAND Common Interface - Checksum Calculation
// ==========================================================
u32 nand_chksum_per_file (u32 nand_offset, u32 img_size)
{
    u32 now = 0, i = 0, chksm = 0, start_block = 0, total = 0;
    INT32 cnt;
    bool ret = TRUE;

    u32 start = nand_offset;
    u32 pagesz = g_nand_chip.page_size;
    u32 pktsz = pagesz + g_nand_chip.oobsize;
    u8 *buf = (u8 *)STORAGE_BUFFER_ADDR;

    // clean the buffer
    memset (buf, 0x0, pktsz);

    // calculate the number of page
    total = img_size / pagesz;
    if (img_size % pagesz != 0)
    {     
        total++;
    }

    // check the starting block is safe
    start_block = nand_find_safe_block (start);
    if (start_block != start)
    {     
        start = start_block;
    }

    // copy data from NAND to MEM
    for (cnt = total, now = start; cnt >= 0; cnt--, now += pagesz)
    {

        // when the address is block alignment, check if this block is good
        if (now % BLOCK_SIZE == 0)
        {     now = nand_find_safe_block (now);
        }

        /* read a packet */
        nand_read_data (buf, now);

        /* cal chksm */
        chksm = nand_chksum_body (chksm, buf, pktsz);

    }

    return chksm;
}
#endif

