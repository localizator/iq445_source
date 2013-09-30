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

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/rtc.h>
#include <sys/mman.h>

#include "meta_cpu_para.h"
#include "meta_common.h"
#include "FT_Public.h"
#include "WM2Linux.h"

#define TAG "[CPU] "


#define RTC_DEV_PATH	"/dev/rtc0"
#define MEM_DEV_PATH	"/dev/mem"
#define READ_SIZE		4

#if 0
static int dbg = 1;
#define dprintf(x...)	do { if (dbg) printf(x); } while (0)
#else
#define dprintf(x...)	do {} while (0)
#endif

WatchDog_CNF META_RTCRead_OP(WatchDog_REQ ft_req)
{
	int r, fd;
	struct rtc_time tm;
	WatchDog_CNF cnf = { .status = false, };

	fd = open(RTC_DEV_PATH, O_RDONLY);
	if (fd < 0) {
		META_LOG("rtc: open %s failed\n", RTC_DEV_PATH);
		return cnf;
	}

	r = ioctl(fd, RTC_RD_TIME, &tm);
	if (r < 0) {
		META_LOG("rtc: read rtc time failed\n");
		return cnf;
	}

	close(fd);

	/*META_LOG("rtc: %d/%02d/%02d (%d) (%d) %02d:%02d:%02d\n",
	         tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_wday, tm.tm_yday,
	         tm.tm_hour, tm.tm_min, tm.tm_sec);*/

	cnf.rtc_sec = tm.tm_sec;
	cnf.rtc_min = tm.tm_min;
	cnf.rtc_hour = tm.tm_hour;
	cnf.rtc_day = tm.tm_mday;
	cnf.rtc_mon = tm.tm_mon + 1;
	cnf.rtc_wday = tm.tm_wday;
	cnf.rtc_year = tm.tm_year + 1900;
	cnf.status = true;
	return cnf;
}

FT_REG_READ_CNF META_CPURegR_OP (FT_REG_READ_REQ *ft_reg)
{
	int fd;
	unsigned char *base;
	unsigned int Addr_base;
	unsigned int Addr_offset;
    FT_REG_READ_CNF CpuReadCnf;
    unsigned int Value;

	memset(&CpuReadCnf,0,sizeof(FT_REG_READ_CNF));

	fd = open(MEM_DEV_PATH, O_RDWR);
	if (fd < 0) {
		META_LOG("[META CPU]: open %s failed\n", MEM_DEV_PATH);
		CpuReadCnf.cnf.status = false;
		goto ErrExit;
	}

	Addr_base = (ft_reg->req.addr & 0xFFFFF000) ;
	Addr_offset = (ft_reg->req.addr & 0x00000FFF) ;

	META_LOG("[META CPU] ft_reg->req.bytenum = %d\n" ,ft_reg->req.bytenum);
	META_LOG("[META CPU] ft_reg->req.addr = 0x%x\n" ,ft_reg->req.addr);
	META_LOG("[META CPU] Addr_base = 0x%x\n" ,Addr_base);


	base = mmap(NULL, READ_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, Addr_base);
	if (base == MAP_FAILED) {
		META_LOG("[META CPU]: mmap failed\n");
		CpuReadCnf.cnf.status = false;
		goto ErrExit;
	}

	if(ft_reg->req.bytenum == 4)
	{
    	CpuReadCnf.cnf.value = *(unsigned int *)(base + Addr_offset);
	}
	else if(ft_reg->req.bytenum == 2)
	{
    	CpuReadCnf.cnf.value = *(unsigned int *)(base + Addr_offset);
	}
	else
	{
    	CpuReadCnf.cnf.value = *(unsigned int *)(base + Addr_offset);
	}
	
    CpuReadCnf.cnf.status = true;
	META_LOG("[META CPU]: READ CpuReadCnf.cnf.value = 0x%x\n", CpuReadCnf.cnf.value);
ErrExit:
	munmap(base, READ_SIZE);
	close(fd);

    return CpuReadCnf;
}

FT_REG_WRITE_CNF META_CPURegW_OP (FT_REG_WRITE_REQ *ft_reg)
{
	int fd;
	unsigned char *base;
	unsigned int Addr_base;
	unsigned int Addr_offset;
	FT_REG_WRITE_CNF CpuWriteCnf;
	unsigned int Value;
	
	memset(&CpuWriteCnf,0,sizeof(FT_REG_WRITE_CNF));

	META_LOG("[META CPU] ft_reg->req.bytenum = %d\n" ,ft_reg->req.bytenum);
	META_LOG("[META CPU] ft_reg->req.addr = 0x%x\n" ,ft_reg->req.addr);
	META_LOG("[META CPU]: ft_reg->req.value = 0x%x\n", ft_reg->req.value);

	
	fd = open(MEM_DEV_PATH, O_RDWR);
	if (fd < 0) {
		META_LOG("[META CPU]: open %s failed\n", MEM_DEV_PATH);
		CpuWriteCnf.cnf.status = false;
		goto ErrExit;
	}
	
	Addr_base = (ft_reg->req.addr & 0xFFFFF000) ;
	Addr_offset = (ft_reg->req.addr & 0x00000FFF) ;
	
	base = mmap(NULL, READ_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, Addr_base);
	if (base == MAP_FAILED) {
		META_LOG("[META CPU]: mmap failed\n");
		CpuWriteCnf.cnf.status = false;
		goto ErrExit;
	}
	
	
	//if(ft_reg->req.bytenum == 4)
	{
		*(unsigned int *)(base + Addr_offset) = ft_reg->req.value;
		if (ft_reg->req.value == ((unsigned int *)(base + Addr_offset)) )			
			CpuWriteCnf.cnf.status = true;
		else
			CpuWriteCnf.cnf.status = false;			
	}
#if 0	
	else if(ft_reg->req.bytenum == 2)
	{
		CpuWriteCnf.cnf.value = (unsigned short *)(base + Addr_offset);
	}
	else
	{
		CpuWriteCnf.cnf.value = (unsigned char *)(base + Addr_offset);
	}
#endif

	CpuWriteCnf.cnf.status = true;
ErrExit:
	munmap(base, READ_SIZE);
	close(fd);
	
	return CpuWriteCnf;

}

