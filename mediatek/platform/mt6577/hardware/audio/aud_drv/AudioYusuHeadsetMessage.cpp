/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2009
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
*                E X T E R N A L   R E F E R E N C E S
******************************************************************************
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <fcntl.h>
#include <sys/types.h>
#include <errno.h>
#include <pthread.h>
#include "AudioYusuLad.h"
#include <utils/Log.h>
#include "AudioYusuLad.h"
#include "AudioYusuHeadsetMessage.h"
#include "AudioYusuHardware.h"

/*****************************************************************************
*                          C O N S T A N T S
******************************************************************************
*/
#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "AudioHeadSetMessager"

#ifdef ENABLE_LOG_AUDIOHEADSETMESSAGER
    #define LOG_AudioHeadSetMessager ALOGD
#else
    #define LOG_AudioHeadSetMessager ALOGV
#endif

namespace android {

/*****************************************************************************
*                   G L O B A L      V A R I A B L E
******************************************************************************
*/
static int HeadsetFd =0;
static int headstatusFd =0;
#define BUF_LEN 1
static char rbuf[BUF_LEN] = {'\0'};
static char wbuf[BUF_LEN] = {'1'};
static char wbuf1[BUF_LEN] = {'2'};

/*****************************************************************************
*                        F U N C T I O N   D E F I N I T I O N
******************************************************************************
*/
bool AudioYusuHeadSetMessager::SetHeadInit()
{
    LOG_AudioHeadSetMessager("SetHeadInit");
    int ret =0;
    if(HeadsetFd <=0){
        // open headset device
        HeadsetFd = open(HEADSET_PATH, O_RDONLY);
        if(HeadsetFd <0){
            ALOGE("open %s error fd = %d",HEADSET_PATH,HeadsetFd);
            return false;
        }
    }
    ret = ::ioctl(HeadsetFd,ACCDET_INIT,0);
    return true;
}

AudioYusuHeadSetMessager::AudioYusuHeadSetMessager(LAD *lad)
{
    LOG_AudioHeadSetMessager("AudioHeadSetMessager Contructor");
    int ret =0;
    if(lad != NULL){
        mLad = lad;
    }
    else{
        ALOGD("AudioHeadSetMessager Contructor with no Lad");
    }
    // create thread
//    ret = pthread_create(&hHeadsetReadThread,NULL,HeadsetIoctl_function,(void*)mLad);
}

void AudioYusuHeadSetMessager::SetHeadSetState(int state)
{
    LOG_AudioHeadSetMessager("SetHeadSetState state = %d");
    int ret =0;
    if(HeadsetFd <=0){
        // open headset device
        HeadsetFd = open(HEADSET_PATH, O_RDONLY);
        if(HeadsetFd <0){
            ALOGE("open %s error fd = %d",HEADSET_PATH,HeadsetFd);
            return;
        }
    }
    ret = ::ioctl(HeadsetFd,SET_CALL_STATE,state);
}

bool AudioYusuHeadSetMessager::Get_headset_info(void)
{
    headstatusFd = -1;
    headstatusFd = open(YUSUHEADSET_STAUTS_PATH, O_RDONLY,0);

    if(headstatusFd <0){
        ALOGE("open %s error fd = %d",YUSUHEADSET_STAUTS_PATH,headstatusFd);
        return false;
    }

    if (read(headstatusFd, rbuf, BUF_LEN) == -1) {
       ALOGD("Get_headset_info Can't read headset");
       close(headstatusFd);
       return false;
   }

    if (!strncmp(wbuf, rbuf, BUF_LEN)) {
       ALOGD( "Get_headset_info Get_headset_info state  == 1" );
       close(headstatusFd);
       return  true;
   }

    if (!strncmp(wbuf, rbuf, BUF_LEN)) {
       ALOGD( "Get_headset_info state  == 2" );
       close(headstatusFd);
       return true;
   }
   else {
       ALOGD("Get_headset_info return  false" );
       close(headstatusFd);
       return  false;
   }
}

/*
static void *HeadsetIoctl_function(void *ptr)
{
   //assigned pointer
    LAD *Lad = NULL;
    if(ptr != NULL){
        Lad = (LAD*)ptr;
    }

    // open headset device
    HeadsetFd = open(HEADSET_PATH, O_RDONLY);
    if(HeadsetFd <0){
        ALOGE("open %s error fd = %d",HEADSET_PATH,HeadsetFd);
        return 0;
    }

    int ret =0;
    int AudioMode;
    ALOGD("HeadsetIoctl_function into whileloop");
    // use ioctl for read heaset message
    while(1){
        ret = ::ioctl(HeadsetFd,HOOK_SWITCH_CHECK,0);
        if(ret == 0){
            if( Lad->mHw != NULL){
                Lad->mHw->getMode (&AudioMode);
            }
            else{
                AudioMode = 0;
            }
            ALOGD("HeadsetIoctl_function ret == 0 AudioMode = %d",AudioMode);
            //only mode == normal and no headset is available can close mic bias.
            if(AudioYusuHeadSetMessager::Get_headset_info() == false && AudioMode == AudioSystem::MODE_NORMAL)
                Lad->LAD_SwitchMicBias(0);
        }
        else if(ret ==1){
            ALOGD("HeadsetIoctl_function ret == 0");
            Lad->LAD_SwitchMicBias(1);
            //delay for suer micbias is open by modem side
            usleep(50*1000);
        }
        else{
            ALOGE("HeadsetFd HOOK_SWITCH_CHECK error");
        }
    }
    return 0;
}
*/

}


