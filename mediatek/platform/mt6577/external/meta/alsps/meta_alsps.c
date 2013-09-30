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
#include <linux/sensors_io.h>

#include "meta_alsps.h"
#include "WM2Linux.h"

//#include "libhwm.h"
/*---------------------------------------------------------------------------*/
#define ALSPS_PREFIX   "ALSPS: "
#define ALSPS_NAME	"/dev/als_ps"

/*---------------------------------------------------------------------------*/
#define MALSPSLOGD(fmt, arg ...) META_LOG(ALSPS_PREFIX fmt, ##arg)
//#define MALSPSLOGD(fmt, arg...) printf(ALSPS_PREFIX fmt, ##arg)

/*---------------------------------------------------------------------------*/
static int fd = -1;
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
BOOL Meta_ALSPS_Open(void)
{
    int retry =0;
	int max_retry = 3;
	int retry_period = 100;
	int err =0;
	unsigned int flags = 1;
	if(fd < 0)
	{
		fd = open(ALSPS_NAME, O_RDONLY);
		if(fd < 0)
		{
			MALSPSLOGD("Open alsps device error!\n");
			return false;
		}
		else
		{
		    MALSPSLOGD("Open alsps OK!\n");
		}
	}

	//enable als ps
	 retry = 0;
     while ((err = ioctl(fd, ALSPS_SET_ALS_MODE, &flags)) && (retry ++ < max_retry)) 
        usleep(retry_period*1000);
     if (err) 
	 {
        MALSPSLOGD("enable als fail: %s", strerror(errno));
        return false;            
     }
	 retry = 0;
     while ((err = ioctl(fd, ALSPS_SET_PS_MODE, &flags)) && (retry ++ < max_retry))
        usleep(retry_period*1000);
     if (err) 
	 {
         MALSPSLOGD("enable ps fail: %s", strerror(errno));
         return false;            
     } 
	 return true;
   
}

int alsps_exec_read_raw()
{
	int err = -1;
    int als_dat, ps_dat=0;
    if (fd == -1) 
	{
        MALSPSLOGD("invalid fd\n");
        return -1;
    } 
	
	if ((err = ioctl(fd, ALSPS_GET_ALS_RAW_DATA, &als_dat))) 
	{
        MALSPSLOGD("read als raw error \n");
        return err;
    }
	MALSPSLOGD("als=%x\n",als_dat);

	if ((err = ioctl(fd, ALSPS_GET_PS_RAW_DATA, &ps_dat))) 
	{
        MALSPSLOGD("read ps  raw error \n");
        return err;
    }
	MALSPSLOGD("ps=%x\n",ps_dat);
	
	return err;
}


int Meta_ALSPS_OP()
{

	int err=0;
	err = alsps_exec_read_raw();
	return err;
}
/*---------------------------------------------------------------------------*/
BOOL Meta_ALSPS_Close(void)
{
    if(fd < 0)
    {
    	MALSPSLOGD("alsps device handle is not valid\n");
		return false;
	}
	else
	{
		close(fd);
		return true;
	}   
}
/*---------------------------------------------------------------------------*/
