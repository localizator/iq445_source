/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */
#include<sys/types.h>
#include<sys/stat.h>
#include<stdio.h>
#include<fcntl.h>
#include<string.h>
#include<sys/mount.h>
#include "Meta_Lock.h"
#include "FT_Public.h"
#include "WM2Linux.h"
#include "../../../../kernel/include/mtd/mtd-abi.h"
//#include "../../../../../source/external/mtd_util/mtd_utilc.h"



static int META_Lock_WriteFlash(unsigned int iMagicNum)
{
      int fd;
      int iWriteSize,iRealWriteSize;
      int result;
      unsigned int iReadValue=0;
      int *tempBuf=NULL;
      struct mtd_info_user info;
      struct erase_info_user erase_info;
	  META_LOG("[META_LOCK]:enter write flash\r\n");

      META_LOG("[META_LOCK]:open logo partition: '/dev/logo'\n");
	  fd=open("/dev/logo",O_RDWR);
      if(fd<0)
	   {
	    META_LOG("[META_LOCK]:mtd open error\r\n");
	    return 0;
	   }
	  
      result=ioctl(fd,MEMGETINFO,&info);
      if(result<0)
         {
          META_LOG("[META_LOCK]:mtd get info error\r\n");
          goto end;
         }
      iWriteSize=info.writesize;
      
      erase_info.start=END_BLOCK;
      erase_info.length=BLOCK_SIZE;
      result=ioctl(fd, MEMERASE, &erase_info);
      if(result<0)
       {
        META_LOG("[META_LOCK]:mtd erase error\r\n");
        goto end;
       }
	  
      tempBuf=(int*)malloc(iWriteSize);
      
      if(tempBuf==NULL)
      	{
      	META_LOG("[META_LOCK]:malloc error\r\n");
	      goto end;
      	}
      	memset(tempBuf,0xFF,iWriteSize);
      iRealWriteSize=sizeof(unsigned int);
      memcpy(tempBuf,&iMagicNum,iRealWriteSize);
	  
      result=lseek(fd,END_BLOCK,SEEK_SET);
      if(result!=(END_BLOCK))
	   {
          META_LOG("[META_LOCK]:mtd first lseek error\r\n");
          free(tempBuf);
          goto end;
	   }
      result=write(fd,tempBuf,iWriteSize);
      if(result!=iWriteSize)
	    {
          META_LOG("[META_LOCK]:mtd write error,iWriteSize:%d\r\n",iWriteSize);
          free(tempBuf);
          goto end;
      	}
      memset(tempBuf,0,iWriteSize);
      result=lseek(fd,END_BLOCK,SEEK_SET);
      if(result!=(END_BLOCK))
	    {
          META_LOG("[META_LOCK]:mtd second lseek error\r\n");
          free(tempBuf);
          goto end;
	    }
      result=read(fd,tempBuf,iRealWriteSize);
      if(result!=iRealWriteSize)
          {
           META_LOG("[META_LOCK]:mtd read error\r\n");
           free(tempBuf);
           goto end;
          }
      memcpy(&iReadValue,tempBuf,iRealWriteSize);
      free(tempBuf);
      close(fd);
	  
      if(iReadValue!=iMagicNum)
	    {   
	     META_LOG("[META_LOCK]:mtd readed value error,iReadValue:%d,iMagicNum:%d\r\n",iReadValue,iMagicNum);
	     goto end;
	    }
	  
      return 1;
  end:
  	  close(fd);
  	  return 0;
}


unsigned char META_Lock_OP()
{
   unsigned int iMagicNum=META_LOCK_MAGIC_NUM;
   int result=0;
   META_LOG("[META_LOCK]:enter meta_lock_op\r\n");
   result=META_Lock_WriteFlash(iMagicNum);
   if(result==0)
     return META_FAILED;
   return META_SUCCESS;
   
}
