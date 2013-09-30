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

/*******************************************************************************
** Copyright (c) 2006 MediaTek Inc.
**
** All rights reserved. Copying, compilation, modification, distribution
** or any other use whatsoever of this material is strictly prohibited
** except in accordance with a Software License Agreement with
** MediaTek Inc.
********************************************************************************
*/

#include <sys/types.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <math.h>
#include <errno.h>
#include <fcntl.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
//#include <net/ethernet.h>
#include <sys/time.h>
#include <unistd.h>
#include <linux/socket.h>
#include <linux/if.h>
#include <net/if_arp.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <linux/types.h>
#ifndef __user
#define __user
#endif
#include "wireless.h"
#include "iwlibstub.h"

int openNetHandle(void){
   int sock;		/* generic raw socket desc.	*/


    /* Try to open the socket, if success returns it */
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock >= 0)
        return sock;
    else
        return -1;
}

void closeNetHandle(int skfd){
    /* Close the socket. */
    close(skfd);
}

int enumNetIf(int skfd, _enum_handler fn, void* argc){
    struct  ifconf ifc;
    char    buff[1024];
    struct  ifreq *ifr;
    int     i, num = 0;

    ifc.ifc_len = sizeof(buff);
    ifc.ifc_buf = buff;
    if(ioctl(skfd, SIOCGIFCONF, &ifc) < 0)
    {
        fprintf(stderr, "SIOCGIFCONF: %s\n", strerror(errno));
        return num;
    }
    ifr = ifc.ifc_req;

    for(i = ifc.ifc_len / sizeof(struct ifreq); --i >= 0; ifr++){
        (*fn)(skfd,ifr->ifr_name, argc);
        num ++;
    }
    return num;
}


int setIWreq(int skfd, char* if_name,
    unsigned long ndisOid,
    unsigned char* ndisData,
    unsigned long bufLen,
    unsigned long* outputBufLen)
{
    struct iwreq		wrq;
    unsigned char *	buffer = NULL;
    unsigned int    reqBufLen = 0;
    NDIS_TRANSPORT_STRUCT*   reqStruct_p = NULL;
    int     result;

    reqBufLen = bufLen + sizeof(*reqStruct_p) -
        sizeof(reqStruct_p->ndisOidContent);

    buffer = (unsigned char *)malloc(reqBufLen);

    if (buffer == NULL) {
        printf("malloc(%d) fail\n", reqBufLen);
        return -1;
    }

    reqStruct_p = (NDIS_TRANSPORT_STRUCT*)buffer;

    reqStruct_p->ndisOidCmd = ndisOid;
    reqStruct_p->inNdisOidlength = bufLen;

    if (bufLen != 0) {
        memcpy(reqStruct_p->ndisOidContent, ndisData, bufLen);
    }

    /* Try to read the results */
    wrq.u.data.pointer = buffer;
    wrq.u.data.flags = PRIV_CMD_OID;
    wrq.u.data.length = reqBufLen;

#if 0
    printf("%s: buffer(0x%p), flags(%d), length(%d)\n",
        __FUNCTION__,
        buffer, PRIV_CMD_OID, reqBufLen
        );
#endif

    /* Set device name */
    strncpy(wrq.ifr_name, if_name, IFNAMSIZ);
    /* Do the request */
    result = ioctl(skfd, IOCTL_SET_STRUCT, &wrq);

    if (result < 0) {
        fprintf(stderr, "result %d %s\n", result, strerror(errno));
    }
    *outputBufLen = reqStruct_p->outNdisOidLength;

    free(buffer);

    if(result == 0)
        return 0;
    return -1;
}

int getIWreq(int skfd, char* if_name,
    unsigned long ndisOid,
    unsigned char* ndisData,
    unsigned long bufLen,
    unsigned long* outputBufLen)
{
    struct iwreq		wrq;
    unsigned char *	buffer = NULL;
    unsigned long   reqBufLen = 0;
    NDIS_TRANSPORT_STRUCT*   reqStruct_p = NULL;
    int     result;

    //printf("%s\n", __FUNCTION__);

    reqBufLen = bufLen + sizeof(*reqStruct_p) -
        sizeof(reqStruct_p->ndisOidContent);

    buffer = (unsigned char *)malloc(reqBufLen);

    if(buffer == NULL)
        return -1;

    reqStruct_p = (NDIS_TRANSPORT_STRUCT*)buffer;

    reqStruct_p->ndisOidCmd = ndisOid;
    reqStruct_p->inNdisOidlength = bufLen;

    /* IMPORTANT!! copy input data to buffer (ex. mcr index) */
    memcpy(reqStruct_p->ndisOidContent, ndisData, bufLen);

    /* Try to read the results */
    wrq.u.data.pointer = buffer;
    wrq.u.data.flags = PRIV_CMD_OID;
    wrq.u.data.length = reqBufLen;

    /* Set device name */
    strncpy(wrq.ifr_name, if_name, IFNAMSIZ);
    /* Do the request */
    result = ioctl(skfd, IOCTL_GET_STRUCT, &wrq);

    if(result < 0)
        fprintf(stderr, "getIWreq result %s %d %s\n", if_name, result, strerror(errno));

    if(result == 0){
        memcpy(ndisData, reqStruct_p->ndisOidContent, reqStruct_p->outNdisOidLength);
    }

    *outputBufLen = reqStruct_p->outNdisOidLength;

    free(buffer);

    if(result == 0)
        return 0;
    return -1;
}

