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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <pthread.h>
#include <dlfcn.h>

#include <stdbool.h>
#include "meta_bt.h"

#include "cutils/misc.h"
#include "cust_bt.h"


typedef unsigned long DWORD;
typedef unsigned long* PDWORD;
typedef unsigned long* LPDWORD;
typedef unsigned short USHORT;
typedef unsigned char UCHAR;
typedef unsigned long HANDLE;
typedef void VOID;
typedef void* LPCVOID;
typedef void* LPVOID;
typedef void* LPOVERLAPPED;
typedef unsigned char* PUCHAR;
typedef unsigned char* PBYTE;
typedef unsigned char* LPBYTE;


#define LOG_TAG         "BT_META "
#include <cutils/log.h>

#define BT_META_DEBUG   1
#define ERR(f, ...)     ALOGE("%s: " f, __func__, ##__VA_ARGS__)
#define WAN(f, ...)     ALOGW("%s: " f, __func__, ##__VA_ARGS__)
#if BT_META_DEBUG
#define DBG(f, ...)     ALOGD("%s: " f, __func__, ##__VA_ARGS__)
#define TRC(f)          ALOGW("%s #%d", __func__, __LINE__)
#else
#define DBG(...)        ((void)0)
#define TRC(f)          ((void)0)
#endif

#ifndef BT_DRV_MOD_NAME
#define BT_DRV_MOD_NAME     "bluetooth"
#endif

#define EVTTYPE_NONE        0
#define EVTTYPE_EVENT       1
#define EVTTYPE_SCO         2
#define EVTTYPE_ACL         3


struct uart_t {
    char *type;
    int  m_id;
    int  p_id;
    int  proto;
    int  init_speed;
    int  speed;
    int  flags;
    int  pm;
    char *bdaddr;
    int  (*init) (int fd, struct uart_t *u, struct termios *ti);
    int  (*post) (int fd, struct uart_t *u, struct termios *ti);
};

/* define uart_t.flags */
#define FLOW_CTL_HW     0x0001
#define FLOW_CTL_SW     0x0002
#define FLOW_CTL_NONE   0x0000
#define FLOW_CTL_MASK   0x0003


//===============        Global Variables         =======================

static int   bt_init = 0;
static int   bt_fd = -1;
static int   bt_rfkill_id = -1;
static char *bt_rfkill_state_path = NULL;
static BT_CNF_CB cnf_cb = NULL;
static BT_CNF bt_cnf;

/* Used to read serial port */
static pthread_t rxThread;
static BOOL bKillThread = FALSE;

// mtk bt library
static void *glib_handle = NULL;
typedef int (*INIT)(int fd, struct uart_t *u, struct termios *ti);
typedef int (*UNINIT)(int fd);
typedef int (*WRITE)(int fd, unsigned char *buffer, unsigned long len);
typedef int (*READ)(int fd, unsigned char *buffer, unsigned long len);

INIT    mtk = NULL;
UNINIT  bt_restore = NULL;
WRITE   bt_send_data = NULL;
READ    bt_receive_data = NULL;


/**************************************************************************
  *                         F U N C T I O N S                             *
***************************************************************************/

static BOOL BT_DisableSleepMode(void);
static BOOL BT_Send_HciCmd(BT_HCI_CMD *hci_cmd);
static BOOL BT_Recv_HciEvent(BT_HCI_EVENT *hci_event);
static BOOL BT_Send_AclData(BT_HCI_BUFFER *pAclData);
static BOOL BT_Recv_AclData(BT_HCI_BUFFER *pRevAclData);


static void* BT_MetaThread(void* pContext);

static void bt_send_resp(BT_CNF *cnf, size_t len, void *buf, unsigned int size)
{
    if (cnf_cb)
        cnf_cb(cnf, buf, size);
    else
        WriteDataToPC(cnf, len, buf, size);

}

#ifndef MTK_COMBO_SUPPORT
static int bt_init_rfkill(void)
{
    char path[128];
    char buf[32];
    int fd, id;
    ssize_t sz;
    
    TRC();
    
    for (id = 0; id < 10 ; id++) {
        snprintf(path, sizeof(path), "/sys/class/rfkill/rfkill%d/type", id);
        fd = open(path, O_RDONLY);
        if (fd < 0) {
            ERR("Open %s fails: %s(%d)\n", path, strerror(errno), errno);
            return -1;
        }
        sz = read(fd, &buf, sizeof(buf));
        close(fd);
        if (sz >= (ssize_t)strlen(BT_DRV_MOD_NAME) && 
            memcmp(buf, BT_DRV_MOD_NAME, strlen(BT_DRV_MOD_NAME)) == 0) {
            bt_rfkill_id = id;
            break;
        }
    }
    
    if (id == 10)
        return -1;
    
    asprintf(&bt_rfkill_state_path, "/sys/class/rfkill/rfkill%d/state", 
        bt_rfkill_id);
    
    return 0;
}

static int bt_set_power(int on)
{
    int sz;
    int fd = -1;
    int ret = -1;
    const char buf = (on ? '1' : '0');
    
    TRC();
    
    if (bt_rfkill_id == -1) {
        if (bt_init_rfkill()) goto out;
    }
    
    fd = open(bt_rfkill_state_path, O_WRONLY);
    if (fd < 0) {
        ERR("Open %s to set BT power fails: %s(%d)", bt_rfkill_state_path,
             strerror(errno), errno);
        goto out;
    }
    sz = write(fd, &buf, 1);
    if (sz < 0) {
        ERR("Write %s fails: %s(%d)", bt_rfkill_state_path, 
             strerror(errno), errno);
        goto out;
    }
    ret = 0;

out:
    if (fd >= 0) close(fd);
    return ret;
}
#endif

void META_BT_Register(BT_CNF_CB callback)
{
    cnf_cb = callback;
}

BOOL META_BT_init(void)
{
    struct uart_t u;

    TRC();

#ifndef MTK_COMBO_SUPPORT
    /* in case BT is powered on before test */
    bt_set_power(0);
    
    if(bt_set_power(1) < 0) {
        ERR("BT power on fails\n");
        return -1;
    }
#endif

    glib_handle = dlopen("libbluetooth_mtk.so", RTLD_LAZY);
    if (!glib_handle){
        ERR("%s\n", dlerror());
        goto error;
    }
    
    mtk = dlsym(glib_handle, "mtk");
    bt_restore = dlsym(glib_handle, "bt_restore");
    bt_send_data = dlsym(glib_handle, "bt_send_data");
    bt_receive_data = dlsym(glib_handle, "bt_receive_data");
    
    if (!mtk || !bt_restore || !bt_send_data || !bt_receive_data){
        ERR("Can't find function symbols %s\n", dlerror());
        goto error;
    }

#ifndef MTK_COMBO_SUPPORT
    u.flags &= ~FLOW_CTL_MASK;
    u.flags |= FLOW_CTL_SW;
    u.speed = CUST_BT_BAUD_RATE;
#endif

    bt_fd = mtk(-1, &u, NULL);
    if (bt_fd < 0)
        goto error;
    
    DBG("BT is enabled success\n");
    
#ifndef MTK_COMBO_SUPPORT
    /* 
     BT META driver DONOT handle sleep mode and EINT,
     so disable Host and Controller sleep in META 
     on standalone chip;
     on combo chip, THIS IS NO NEED
     */
    BT_DisableSleepMode();
#endif

    /* Create RX thread */
    pthread_create(&rxThread, NULL, BT_MetaThread, (void*)&bt_cnf);
    
    bt_init = 1;
    sched_yield();
    
    return TRUE;

error:
    if (glib_handle){
        dlclose(glib_handle);
        glib_handle = NULL;
    }

#ifndef MTK_COMBO_SUPPORT
    bt_set_power(0);
#endif

    return FALSE;
}

void META_BT_deinit(void)
{
    TRC();
    
    /* Stop RX thread */
    bKillThread = TRUE;
    
    /* Wait until thread exist */
    pthread_join(rxThread, NULL);
    
    
    if (!glib_handle || !bt_restore){
        ERR("mtk bt library is unloaded!\n");
    }
    else{
        if (bt_fd < 0){
            ERR("bt driver fd is invalid!\n");
        }
        else{
            bt_restore(bt_fd);
            bt_fd = -1;
        }
        dlclose(glib_handle);
        glib_handle = NULL;
    }

#ifndef MTK_COMBO_SUPPORT    
    bt_set_power(0); /* shutdown BT */
#endif

    bt_init = 0;
    return;
}

void META_BT_OP(BT_REQ   *req, 
                char     *peer_buf, 
                unsigned short peer_len)
{
    TRC();
    
    /* need check COM port handle */
    if (NULL == req || !bt_init) {
        ERR("Invalid arguments or operation!\n");
        return;
    }
    
    memset(&bt_cnf, 0, sizeof(BT_CNF));
    bt_cnf.header.id = FT_BT_CNF_ID;
    bt_cnf.header.token = req->header.token;
    bt_cnf.op = req->op;
    
    /* purse com port? */
    
    switch(req->op){
    	case BT_OP_GET_CHIP_ID:
    	    DBG("BT_OP_GET_CHIP_ID\n");
    	#ifdef MTK_MT6611
    	    bt_cnf.bt_result.dummy = BT_CHIP_ID_MT6611;
    	#elif defined MTK_MT6612
    	    bt_cnf.bt_result.dummy = BT_CHIP_ID_MT6612;
    	#elif defined MTK_MT6616
    	    bt_cnf.bt_result.dummy = BT_CHIP_ID_MT6616;
    	#elif defined MTK_MT6620
    	    bt_cnf.bt_result.dummy = BT_CHIP_ID_MT6620;
    	#elif defined MTK_MT6622
    	    bt_cnf.bt_result.dummy = BT_CHIP_ID_MT6622;
    	#elif defined MTK_MT6626
    	    bt_cnf.bt_result.dummy = BT_CHIP_ID_MT6626;
    	#elif defined MTK_MT6628
    	    bt_cnf.bt_result.dummy = BT_CHIP_ID_MT6628;
    	#endif
    	    bt_cnf.bt_status = TRUE;
    	    bt_cnf.status = META_SUCCESS;
    	    bt_send_resp(&bt_cnf, sizeof(BT_CNF), NULL, 0);
    	    break;
    	    
    	case BT_OP_HCI_SEND_COMMAND:
    	    DBG("BT_OP_HCI_SEND_COMMAND\n");
    	    BT_Send_HciCmd(&req->cmd.hci);
    	    break;
    	    
    	case BT_OP_HCI_CLEAN_COMMAND:
    	    DBG("BT_OP_HCI_CLEAN_COMMAND\n");
    	#ifndef MTK_COMBO_SUPPORT
    	    if (bt_fd >= 0)
    	        tcflush(bt_fd, TCIOFLUSH);
    	#endif
    	    bt_cnf.status = META_SUCCESS;
    	    break;
    	    
    	case BT_OP_HCI_SEND_DATA:
    	    DBG("BT_OP_HCI_SEND_DATA\n");
    	    BT_Send_AclData(&req->cmd.buf);
    	    break;
    	    
    	case BT_OP_HCI_TX_PURE_TEST_V2:
    	case BT_OP_HCI_TX_PURE_TEST:
    	case BT_OP_HCI_RX_TEST_START_V2:
    	case BT_OP_HCI_RX_TEST_START:
    	case BT_OP_HCI_RX_TEST_END:
    	case BT_OP_ENABLE_NVRAM_ONLINE_UPDATE:
    	case BT_OP_DISABLE_NVRAM_ONLINE_UPDATE:
    	    
    	case BT_OP_ENABLE_PCM_CLK_SYNC_SIGNAL:
    	case BT_OP_DISABLE_PCM_CLK_SYNC_SIGNAL:
    	    /* need to confirm with CCCI driver buddy */
    	    DBG("Not implemented command %d\n", req->op);
    	    bt_cnf.status = META_FAILED;
    	    bt_send_resp(&bt_cnf, sizeof(BT_CNF), NULL, 0);
    	    break;
    	    
    	default:
    	    DBG("Unknown command %d\n", req->op);
    	    bt_cnf.status = META_FAILED;
    	    bt_send_resp(&bt_cnf, sizeof(BT_CNF), NULL, 0);
    	    break;
    }
    
    return;
}

static BOOL BT_DisableSleepMode(void)
{
    UCHAR   HCI_VS_SLEEP[] = 
                {0x01, 0x7A, 0xFC, 0x07, 0x00, 0x40, 0x1F, 0x00, 0x00, 0x00, 0x04};
    UCHAR   pAckEvent[7];
    UCHAR   ucEvent[] = {0x04, 0x0E, 0x04, 0x01, 0x7A, 0xFC, 0x00};
    
    TRC();
    
    if (!glib_handle || !bt_send_data || !bt_receive_data){
        ERR("mtk bt library is unloaded!\n");
        return FALSE;
    }
    if (bt_fd < 0){
        ERR("bt driver fd is invalid!\n");
        return FALSE;
    }
    
    if(bt_send_data(bt_fd, HCI_VS_SLEEP, sizeof(HCI_VS_SLEEP)) < 0){
        ERR("Send disable sleep mode command fails errno %d\n", errno);
        return FALSE;
    }
    
    if(bt_receive_data(bt_fd, pAckEvent, sizeof(pAckEvent)) < 0){
        ERR("Receive event fails errno %d\n", errno);
        return FALSE;
    }
    
    if(memcmp(pAckEvent, ucEvent, sizeof(ucEvent))){
        ERR("Receive unexpected event\n");
        return FALSE;
    }
    
    return TRUE;
}

static BOOL BT_Send_HciCmd(BT_HCI_CMD *hci_cmd)
{
    UCHAR   HCI_CMD[256+4];
    
    if (!glib_handle || !bt_send_data){
        ERR("mtk bt library is unloaded!\n");
        return FALSE;
    }
    if (bt_fd < 0){
        ERR("bt driver fd is invalid!\n");
        return FALSE;
    }
    
    HCI_CMD[0] = 0x01;
    HCI_CMD[1] = (hci_cmd->opcode)&0xff;
    HCI_CMD[2] = (hci_cmd->opcode>>8)&0xff;
    HCI_CMD[3] = hci_cmd->len;
    
    DBG("OpCode %x len %d\n", hci_cmd->opcode, (int)hci_cmd->len);
    
    memcpy(&HCI_CMD[4], hci_cmd->cmd, hci_cmd->len);
    
    if(bt_send_data(bt_fd, HCI_CMD, hci_cmd->len + 4) < 0){
        ERR("Write HCI command fails errno %d\r\n", errno);
        return FALSE;
    }
    
    return TRUE;
}

static BOOL BT_Recv_HciEvent(BT_HCI_EVENT *hci_event)
{
    hci_event->status = FALSE;
    
    if (!glib_handle || !bt_receive_data){
        ERR("mtk bt library is unloaded!\n");
        return FALSE;
    }
    if (bt_fd < 0){
        ERR("bt driver fd is invalid!\n");
        return FALSE;
    }
    
    if(bt_receive_data(bt_fd, &hci_event->event, 1) < 0){
        ERR("Read event code fails errno %d", errno);
        return FALSE;
    }
    
    DBG("Read event code: 0x%x\n", hci_event->event);
    
    if(bt_receive_data(bt_fd, &hci_event->len, 1) < 0){
        ERR("Read event length fails errno %d", errno);
        return FALSE;
    }
    
    DBG("Read event length: 0x%x\n", hci_event->len);
    
    if(hci_event->len){
        if(bt_receive_data(bt_fd, hci_event->parms, hci_event->len) < 0){
            ERR("Read event param fails errno %d", errno);
            return FALSE;
        }
    }
    
    hci_event->status = TRUE;
    return TRUE;
}

static BOOL BT_Send_AclData(BT_HCI_BUFFER *pAclData)
{
    UCHAR   AclData[1029];
    
    if (!glib_handle || !bt_send_data){
        ERR("mtk bt library is unloaded!\n");
        return FALSE;
    }
    if (bt_fd < 0){
        ERR("bt driver fd is invalid!\n");
        return FALSE;
    }
    
    AclData[0] = 0x02;
    AclData[1] = (pAclData->con_hdl)&0xff;
    AclData[2] = (pAclData->con_hdl>>8)&0xff;
    AclData[3] = (pAclData->len)&0xff;
    AclData[4] = (pAclData->len>>8)&0xff;
         
    memcpy(&AclData[5], pAclData->buffer, pAclData->len);
    
    if(bt_send_data(bt_fd, AclData, pAclData->len + 5) < 0){
        ERR("Write ACL data fails errno %d\r\n", errno);
        return FALSE;
    }
    
    return TRUE;
}

static BOOL BT_Recv_AclData(BT_HCI_BUFFER *pRevAclData)
{

    if (!glib_handle || !bt_receive_data){
        ERR("mtk bt library is unloaded!\n");
        return FALSE;
    }
    if (bt_fd < 0){
        ERR("bt driver fd is invalid!\n");
        return FALSE;
    }
    
    if(bt_receive_data(bt_fd, (UCHAR*)&pRevAclData->con_hdl, 2) < 0){
        ERR("Read connection handle fails errno %d", errno);
        return FALSE;
    }
    
    pRevAclData->con_hdl = ((pRevAclData->con_hdl&&0xff)<<8)|((pRevAclData->con_hdl>>8)&&0xff);
    
    if(bt_receive_data(bt_fd, (UCHAR*)&pRevAclData->len, 2) < 0){
        ERR("Read ACL data length fails errno %d", errno);
        return FALSE;
    }
    
    pRevAclData->len = ((pRevAclData->len&0xff)<<8)|((pRevAclData->len>>8)&0xff);
    
    if(pRevAclData->len){
        if(bt_receive_data(bt_fd, pRevAclData->buffer, pRevAclData->len) < 0){
            ERR("Read ACL data fails errno %d", errno);
            return FALSE;
        }
    }
    
    return TRUE;
}


static void *BT_MetaThread( void *ptr )
{
    BT_HCI_EVENT hci_event;
    BOOL     RetVal = TRUE;
    UCHAR    ucHeader = 0;
    BT_CNF  *pBt_CNF = (BT_CNF*)ptr;
    BT_HCI_BUFFER temp_acl_data;
    
    TRC();
    
    while(!bKillThread){
        
        if (!glib_handle || !bt_receive_data){
            ERR("mtk bt library is unloaded!\n");
            goto CleanUp;
        }
        if (bt_fd < 0){
            ERR("bt driver fd is invalid!\n");
            goto CleanUp;
        }
        
        if(bt_receive_data(bt_fd, &ucHeader, sizeof(ucHeader)) < 0){
            ERR("zero byte read\n");
            continue;
        }    
        
        switch (ucHeader)
        {
            case 0x04:
                DBG("Receive HCI event\n");
                if(BT_Recv_HciEvent(&hci_event))
                {
                    pBt_CNF->bt_status = TRUE;
                    pBt_CNF->eventtype = EVTTYPE_EVENT;
                    memcpy(&pBt_CNF->bt_result.hcievent, &hci_event, sizeof(hci_event));
                    pBt_CNF->status = META_SUCCESS;
                    bt_send_resp(&bt_cnf, sizeof(BT_CNF), NULL, 0);
                }
                else{
                    pBt_CNF->bt_status = FALSE;
                    pBt_CNF->eventtype = EVTTYPE_EVENT;
                    pBt_CNF->status = META_FAILED;
                    bt_send_resp(&bt_cnf, sizeof(BT_CNF), NULL, 0);
                }
                break;
                
            case 0x02:
                DBG("Receive ACL data\n");
                if(BT_Recv_AclData(&temp_acl_data))
                {
                    pBt_CNF->bt_status = TRUE;
                    pBt_CNF->eventtype = EVTTYPE_ACL;
                    memcpy(&pBt_CNF->bt_result.hcibuffer, &temp_acl_data, sizeof(temp_acl_data));
                    pBt_CNF->status = META_SUCCESS;
                    bt_send_resp(&bt_cnf, sizeof(BT_CNF), NULL, 0);
                }
                else{
                    pBt_CNF->bt_status = FALSE;
                    pBt_CNF->eventtype = EVTTYPE_ACL;
                    pBt_CNF->status = META_FAILED;
                    bt_send_resp(&bt_cnf, sizeof(BT_CNF), NULL, 0);
                }
                break;
                
            default:
                ERR("Unexpected BT packet header %02x\n", ucHeader);
                goto CleanUp;
        }
    }

CleanUp:
    return 0;
}
