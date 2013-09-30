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
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <utils/Log.h>

#undef LOG_TAG 
#define LOG_TAG "META"

#include "meta_vibrator_para.h"

#define LOGD ALOGD
#define LOGE ALOGE
#define LOGI ALOGI
#define LOGW ALOGW



#define VIB_DEV_PATH		"/sys/class/timed_output/vibrator/vibr_on"
#define LED_BRIGHTNESS_PATH_R "/sys/class/leds/red/brightness"
#define LED_BRIGHTNESS_PATH_G "/sys/class/leds/green/brightness"
#define LED_BRIGHTNESS_PATH_B "/sys/class/leds/blue/brightness"

#define KPD_BKL_PATH		"/sys/class/leds/button-backlight/brightness"


#define ENABLE_VIBRATOR 	4
#define DISABLE_VIBRATOR 	5

#define __DEBUG				0

#if __DEBUG
static int dbg = 1;
#define dprintf(x...)	do { if (dbg) printf(x); } while (0)
#else
#define dprintf(x...)	do {} while (0)
#endif

enum mt65xx_led_color
{
	MT65XX_LED_COLOR_RED,
	MT65XX_LED_COLOR_GREEN,
	MT65XX_LED_COLOR_BLUE,
	MT65XX_LED_COLOR_NUM,
};

static int vib_fd = -1;


// level: 0 means OFF, others means ON
BOOL nled_set_brightness(enum mt65xx_led_color color, int level)
{
	int fd = -1;
	BOOL ret = false;

#define BUF_LEN 16
	char wbuf[BUF_LEN] = {'\0'};
	char rbuf[BUF_LEN] = {'\0'};

	char *led_brightness_path[MT65XX_LED_COLOR_NUM] = {
		LED_BRIGHTNESS_PATH_R, 
		LED_BRIGHTNESS_PATH_G,
		LED_BRIGHTNESS_PATH_B};

	if (color >= MT65XX_LED_COLOR_NUM)
		return false;

	fd = open(led_brightness_path[color], O_RDWR, 0);
	if (fd == -1) {
		LOGE("Can't open %s\n", led_brightness_path[color]);
		return false;
	}
	if (level < 0)
		level = 0;
	sprintf(wbuf, "%d\n", level);
	if (write(fd, wbuf, strlen(wbuf)) == -1) {
		LOGE("Can't write %s\n", led_brightness_path[color]);
		goto EXIT;
	}
	close(fd);

	fd = open(led_brightness_path[color], O_RDWR, 0);
	if (fd == -1) {
		LOGE("Can't open %s\n", led_brightness_path[color]);
		goto EXIT;
	}
	if (read(fd, rbuf, BUF_LEN) == -1) {
		LOGE("Can't read %s\n", led_brightness_path[color]);
		goto EXIT;
	}

	if (!strncmp(wbuf, rbuf, BUF_LEN))
		ret = true;

EXIT:
	if (fd != -1)
		close(fd);
	return ret;
}

BOOL nled_set_vibration(int level) // level 0:off, 1:on, 2:blink
{
	int r;
	
	if (vib_fd < 0) 
	{
		dprintf("meta_vib: uninitialized vib_fd\n");
		return false;
	}

	if (level)
	{
		r = write(vib_fd,"1",1);
		dprintf("meta_vib: enable vibrator\n");
	
		if (r < 0) 
		{
			dprintf("meta_vib: enable vibrator failed\n");
			return false;
		}
	}	
	else
	{
		r = write(vib_fd,"0",1);
		dprintf("meta_vib: disable vibrator\n");
		
		if (r < 0) 
		{
			dprintf("meta_vib: enable vibrator failed\n");
			return false;
		}
	}

	return true;
}

BOOL Meta_Vibrator_Init(void)
{
	vib_fd = open(VIB_DEV_PATH, O_WRONLY);
	
	if (vib_fd < 0) 
	{
		dprintf("meta_vib: open %s failed\n", VIB_DEV_PATH);
		return false;
	}

	return true;
}

BOOL Meta_KeypadBK(BOOL onoff)
{
	int r;
	int kpd_fd = -1;

	kpd_fd = open(KPD_BKL_PATH, O_WRONLY);
	if (kpd_fd < 0) {
		dprintf("kpd: open %s failed\n", KPD_BKL_PATH);
		dprintf("kpd: uninitialized kpd_fd\n");
		return false;
	}

	
	if (onoff)
	{
			if (write(kpd_fd,"1",1) == -1) {
			dprintf("Can't write %d\n",onoff);
		}			
	}else if (write(kpd_fd, "0",1) == -1) {
			dprintf("Can't write %d\n", onoff);	
	}

	return true;
}

NLED_CNF Meta_Vibrator_OP(NLED_REQ req)
{
	int r;
	NLED_CNF cnf = { .status = false, };
	int level;

	if (req.onoff)
		level = 255;
	else
		level = 0;

	switch (req.LedNum)
	{
		case LEDNUM_VIBRATOR:
			cnf.status = nled_set_vibration(req.onoff);
			break;
		case LEDNUM_RED:
			cnf.status = nled_set_brightness(MT65XX_LED_COLOR_RED, level);
			break;
		case LEDNUM_GREEN:
			cnf.status = nled_set_brightness(MT65XX_LED_COLOR_GREEN, level);
			break;
		case LEDNUM_BLUE:
			cnf.status = nled_set_brightness(MT65XX_LED_COLOR_BLUE, level);
			break;
		case LEDNUM_KEYPAD:
			cnf.status = Meta_KeypadBK(req.onoff);
			//cnf.status = true;
			break;
	}

	return cnf;
}

BOOL Meta_Vibrator_Deinit(void)
{
	if (vib_fd < 0) 
	{
		dprintf("meta_vib: uninitialized vib_fd\n");
		return false;
	}

	close(vib_fd);
	return true;
}
