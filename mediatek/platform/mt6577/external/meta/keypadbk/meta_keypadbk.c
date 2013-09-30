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

#include <WM2Linux.h>
#include "meta_keypadbk_para.h"

#define KPD_BKL_PATH		"/sys/class/leds/button-backlight/brightness"

struct kpd_ledctl
{
	__u8 onoff;
	__u8 div;
	__u8 duty;
};

#define SET_KPD_BACKLIGHT	_IOW('k', 29, struct kpd_ledctl)

static int kpd_fd = -1;

BOOL Meta_KeypadBK_Init(void)
{
	kpd_fd = open(KPD_BKL_PATH, O_WRONLY);
	if (kpd_fd < 0) {
		META_LOG("kpd: open %s failed\n", KPD_BKL_PATH);
		return false;
	}

	return true;
}

KeypadBK_CNF Meta_KeypadBK_OP(KeypadBK_REQ req)
{
	int r;
	struct kpd_ledctl ledctl;
	KeypadBK_CNF cnf = { .status = false, };

	if (kpd_fd < 0) {
		META_LOG("kpd: uninitialized kpd_fd\n");
		return cnf;
	}
	
	if (req.onoff)
	{
			if (write(kpd_fd,"1",1) == -1) {
			LOGE("Can't write %d\n", req.onoff);
		}			
	}else if (write(kpd_fd, "0",1) == -1) {
			LOGE("Can't write %d\n", req.onoff);	
	}

	cnf.status = true;
	return cnf;
}

BOOL Meta_KeypadBK_Deinit(void)
{
	if (kpd_fd < 0) {
		META_LOG("kpd: uninitialized kpd_fd\n");
		return false;
	}

	close(kpd_fd);
	return true;
}
