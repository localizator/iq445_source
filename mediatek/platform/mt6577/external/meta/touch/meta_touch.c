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
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include "meta_touch.h"
#include "WM2Linux.h"

#define TOUCH_LOG META_LOG
const char *FILENAME = "/sys/module/tpd_setting/parameters/tpd_load_status";

void Meta_Touch_OP(Touch_REQ *req, char *peer_buff, unsigned short peer_len)
{
	Touch_CNF TouchMetaReturn;
	int ret, fd = -1;
	int status = 0;

	memset(&TouchMetaReturn, 0, sizeof(TouchMetaReturn));

	TouchMetaReturn.header.id=req->header.id+1;
	TouchMetaReturn.header.token = req->header.token;
	TouchMetaReturn.status=META_FAILED;
	
	/* open file */
	fd = open(FILENAME,O_RDWR, 0);
	if (fd < 0) {
		TOUCH_LOG("Open %s : ERROR \n", FILENAME);
		TOUCH_LOG("Open %s : ERROR \n", FILENAME);
		goto Touch_Finish;
	}
	
	ret = read(fd, &status, sizeof(int));
	if((status & 0X0ff) == '1')
		TouchMetaReturn.status=META_SUCCESS; 
		
	TOUCH_LOG("Cap touch status:%d\n", status);
	close(fd);

Touch_Finish:
	if (false == WriteDataToPC(&TouchMetaReturn,sizeof(Touch_CNF),NULL,0)) {
		TOUCH_LOG("%s : WriteDataToPC() fail 2\n", __FUNCTION__);
    }
	TOUCH_LOG("%s : Finish !\n", __FUNCTION__);
	
}

BOOL Meta_Touch_Init(void)
{
	return true;
}

BOOL Meta_Touch_Deinit(void)
{
	return true;
}
