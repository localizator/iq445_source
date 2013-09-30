/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2008
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/
/*****************************************************************************
 *
 * Filename:
 * ---------
 *   ccci.h
 *
 * Project:
 * --------
 *   YuSu
 *
 * Description:
 * ------------
 *   MT6516 CCCI header file
 *
 * Author:
 * -------
 *   CC Hwang (mtk00702)
 *
 ****************************************************************************/

#ifndef __CCCI_H__
#define __CCCI_H__

/*
 * define constants
 */

#define CCCI_DEV_NAME "ccci"
#define CCCI_SYSFS_INFO "info"

/*
 * define data structures
 */

/* CCCI API return value */
typedef enum
{
    CCCI_SUCCESS = 0,
    CCCI_FAIL = -1001,
    CCCI_IN_USE = -1002,
    CCCI_NOT_OWNER = -1003,
    CCCI_INVALID_PARAM = -1004,
    CCCI_NO_PHY_CHANNEL = -1005,
    CCCI_IN_INTERRUPT = -1006,
    CCCI_IN_IRQ = -1007
}CCCI_RETURNVAL_T;

typedef enum
{
#define X_DEF_CH
#include "ccci_ch.h"
#undef X_DEF_CH
    CCCI_MAX_CHANNEL,
    CCCI_FORCE_RESET_MODEM_CHANNEL = 20090215,
} CCCI_CHANNEL_T;


/* CCCI mailbox channel structure */
typedef struct
{
    unsigned int magic;   /* 0xFFFFFFFF */
    unsigned int id;
} CCCI_MAILBOX_T;

/* CCCI stream channel structure */
typedef struct
{
    unsigned int addr;
    unsigned int len;
} CCCI_STREAM_T;

/* CCCI channel buffer structure */
typedef struct
{
    unsigned int data[2];
    unsigned int channel;
    unsigned int reserved;
} CCCI_BUFF_T;

/* CCCI callback function prototype */
typedef void (*CCCI_CALLBACK)(CCCI_BUFF_T *buff, void *private_data);

/* CCCI status */
typedef enum
{
    CCCI_IDLE = 0x00,
    CCCI_ACTIVE_READ = 0x01,
    CCCI_ACTIVE_WRITE = 0x02,
    CCCI_ACTIVE_ISR = 0x04
} CCCI_STATE_T;

/* CCCI control structure */
/*
typedef struct _CCCI_CTRL_T
{
    struct attribute attr;
    ssize_t (*show)(char *);
    ssize_t (*store)(const char *, size_t count);
    struct cdev cdev;
    CCCI_STATE_T state;
    int owner;
    CCCI_BUFF_T buff;
    void *private_data;
    CCCI_CALLBACK callback;
    wait_queue_head_t waitq;
    struct fasync_struct *fa;
} CCCI_CTRL_T;
*/

/*
 * define macros
 */

/* initialize a CCCI mailbox buffer */
#define CCCI_INIT_MAILBOX(buff, mailbox_id) \
        do {    \
            ((CCCI_MAILBOX_T *)((buff)->data))->magic = 0xFFFFFFFF; \
            ((CCCI_MAILBOX_T *)((buff)->data))->id = (mailbox_id);  \
            (buff)->channel = CCCI_MAX_CHANNEL;  \
            (buff)->reserved = 0;    \
        } while (0)

/* initialize a CCCI stream buffer */
#define CCCI_INIT_STREAM(buff, stream_addr, stream_len) \
        do {    \
            ((CCCI_STREAM_T *)((buff)->data))->addr = (stream_addr); \
            ((CCCI_STREAM_T *)((buff)->data))->len = (stream_len);  \
            (buff)->channel = CCCI_MAX_CHANNEL;  \
            (buff)->reserved = 0;    \
        } while (0)

/* check the CCCI buffer type */
#define CCCI_IS_MAILBOX(buff)   ((((CCCI_MAILBOX_T *)((buff)->data))->magic == 0xFFFFFFFF)? 1: 0)

/* get the id of the CCCI mailbox buffer */
#define CCCI_MAILBOX_ID(buff)   (((CCCI_MAILBOX_T *)((buff)->data))->id)

/* get the addr of the CCCI stream buffer */
#define CCCI_STREAM_ADDR(buff)   (((CCCI_STREAM_T *)((buff)->data))->addr)

/* get the len of the CCCI stream buffer */
#define CCCI_STREAM_LEN(buff)   (((CCCI_STREAM_T *)((buff)->data))->len)

/*
 * define function prototpye
 */
/*
extern int ccci_register(CCCI_CHANNEL_T channel, CCCI_CALLBACK funp, void *private_data);
extern int ccci_unregister(CCCI_CHANNEL_T channel);
extern int ccci_write(CCCI_CHANNEL_T channel, CCCI_BUFF_T *buff);
extern int ccci_read(CCCI_CHANNEL_T channel, CCCI_BUFF_T *buff);
extern int ccci_write_mailbox(CCCI_CHANNEL_T channel, int id);
extern int ccci_read_mailbox(CCCI_CHANNEL_T channel, int *id);
extern int ccci_write_stream(CCCI_CHANNEL_T channel, unsigned int addr, unsigned int len);
extern int ccci_read_stream(CCCI_CHANNEL_T channel, unsigned int *addr, unsigned int *len);
extern int ccci_pcm_base_req(void *addr, int *len);
extern int ccci_uart_setup(void *addr, int len);
extern int ccci_uart_base_req(void *addr, int *len);
extern int ccci_fs_setup(void *addr, int len);
extern int ccci_fs_base_req(void *addr, int *len);
extern int ccci_pmic_setup(void *addr, int len);
extern int ccci_pmic_base_req(void *addr, int *len);
*/
/*
 * define IOCTL commands
 */

#define CCCI_IOC_MAGIC 'C'

#define CCCI_IOC_MD_RESET _IO(CCCI_IOC_MAGIC, 0)

#define CCCI_IOC_PCM_BASE_ADDR _IOR(CCCI_IOC_MAGIC, 2, unsigned int)

#define CCCI_IOC_PCM_LEN _IOR(CCCI_IOC_MAGIC, 3, unsigned int)

#endif  /* !__CCCI_H__ */

