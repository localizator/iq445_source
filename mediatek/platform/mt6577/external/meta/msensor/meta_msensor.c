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
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/mtgpio.h>
#include "meta_msensor.h"
#include <linux/sensors_io.h>

//#include "libhwm.h"
/*---------------------------------------------------------------------------*/
#define MS_PREFIX   "MS: "
#define MSENSOR_NAME	"/dev/msensor"

/*---------------------------------------------------------------------------*/
#define MMSLOGD(fmt, arg ...) META_LOG(MS_PREFIX fmt, ##arg)
//#define MMSLOGD(fmt, arg...) printf(MS_PREFIX fmt, ##arg)

/*---------------------------------------------------------------------------*/
static int fd = -1;
/*---------------------------------------------------------------------------*/
//static MS_CNF_CB meta_ms_cb = NULL;
/*---------------------------------------------------------------------------*/
/*
void Meta_MSensor_Register(MS_CNF_CB cb)
{
    meta_ms_cb = cb;
}
*/
/*---------------------------------------------------------------------------*/
bool Meta_MSensor_Open(void)
{
	if(fd < 0)
	{
		fd = open(MSENSOR_NAME, O_RDONLY);
		if(fd < 0)
		{
			MMSLOGD("Open msensor device error!\n");
			return false;
		}
		else
		{
			return true;
		}
	}
	else
	{
		MMSLOGD("msensor file handle is not right!\n");
		return false;
	}
}

int ms_exec_read_raw()
{
	int err = 0;
	int x, y, z;
	int i=0;
	static char buf[128];
	if(fd < 0)
	{
		MMSLOGD("null pointer: %d\n", fd);
		err= -1;
		return err;
	}
	
	err = ioctl(fd, MSENSOR_IOCTL_SET_MODE, 1);
	err = ioctl(fd, MSENSOR_IOCTL_INIT, 0);
	
	for(i=0; i< 20; i++ )
	{
	  err = ioctl(fd, MSENSOR_IOCTL_READ_SENSORDATA, buf);
	  if(err)
	  {
			MMSLOGD("read data fail\n");
	  }
	  else if(3 != sscanf(buf, "%x %x %x", &x, &y, &z))
	  {
		MMSLOGD("read format fail\n");
	  }
	  else
	  {
	    MMSLOGD("data x=%d , y=%d, z= %d\n",x,y,z);
	  }
	}
	
	return err;
}


int Meta_MSensor_OP()
{

	int err=0;
	err = ms_exec_read_raw();
	return err;
}
/*---------------------------------------------------------------------------*/
bool Meta_MSensor_Close(void)
{
    if(fd < 0)
    {
    	MMSLOGD("Msensor device handle is not valid\n");
		return false;
	}
	else
	{
		close(fd);
		return true;
	}   
}
/*---------------------------------------------------------------------------*/
