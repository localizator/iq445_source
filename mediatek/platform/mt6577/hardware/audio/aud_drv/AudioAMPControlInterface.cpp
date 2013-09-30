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

#define LOG_TAG "AudioAMPControl"
//#include <utils/Log.h>
#include <cutils/xlog.h>
#include <sys/ioctl.h>
#include "AudioIoctl.h"
#include <media/AudioSystem.h>
#include <utils/threads.h>
#include"AudioAMPControlInterface.h"

namespace {

	using  namespace android;

	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	// AMP Control

	class AudioAMPControl : public AudioAMPControlInterface
	{
	public:
		AudioAMPControl(int fd);
		~AudioAMPControl();
		bool setSpeaker( bool on);
		bool setHeadphone(bool on);
		bool setReceiver(bool on);
		void setMode(int mode);
		int setParameters(int command1 ,int command2 , unsigned int data);
		int getParameters(int command1 ,int command2 , void* data);
	private:
		int   mFd;
		bool  mSpeakerOn;
		bool  mReceiverOn;
		bool  mHeadphoneOn;
		int   mMode;
		Mutex mLock;
	};


	AudioAMPControl::AudioAMPControl(int fd)
		:mFd(fd),
		mSpeakerOn(false),
		mReceiverOn(false),
		mHeadphoneOn(false),
		mMode(0)
	{


	}

	AudioAMPControl::~AudioAMPControl()
	{

	}

	bool AudioAMPControl::setSpeaker(bool on)
	{
		SXLOGD("setSpeaker::mode=%d,prestate=%d,state=%d",mMode,mSpeakerOn,on);
		Mutex::Autolock autoLock(mLock);
		int ret = NO_ERROR;
		if(mSpeakerOn == on)
			return true;
		if(on)
		{
			ret = ::ioctl(mFd,SET_SPEAKER_ON,Channel_Stereo);// enable speaker
		}
		else
		{
			ret = ::ioctl(mFd,SET_SPEAKER_OFF,Channel_Stereo);// disable speaker
		}
        if(ret < 0)
        {
            SXLOGW(" setSpeaker(%d) error %s (%d)", on, ::strerror(errno), errno);
            return false;
        }
		mSpeakerOn =on;
		return true;
	}

	bool AudioAMPControl::setHeadphone( bool on)
	{
		SXLOGD("setHeadphone::mode=%d,prestate=%d,state=%d",mMode,mHeadphoneOn,on);
		Mutex::Autolock autoLock(mLock);
		int ret = NO_ERROR;
		if(mHeadphoneOn== on)
		    return true;
		if(on)
		{
		    ret = ::ioctl(mFd,SET_HEADPHONE_ON,Channel_Stereo);// enable headphone
		}
		else
		{
		    ret = ::ioctl(mFd,SET_HEADPHONE_OFF,Channel_Stereo);// disable headphone
		}
		if(ret < 0)
		{
		    SXLOGW(" setHeadphone(%d) error %s (%d)", on, ::strerror(errno), errno);
		    return false;
		}
		mHeadphoneOn =on;
		return true;
	}

	bool AudioAMPControl::setReceiver(bool on)
	{
		SXLOGD("setReceiver::mode=%d,prestate=%d,state=%d",mMode,mReceiverOn,on);
		Mutex::Autolock autoLock(mLock);
		int ret = NO_ERROR;
		if(mReceiverOn == on)
		    return true;
		if(on)
		{
		    ret = ::ioctl(mFd,SET_EARPIECE_ON,Channel_Stereo);// enable earpiece
		}
		else
		{
		    ret = ::ioctl(mFd,SET_EARPIECE_OFF,Channel_Stereo);// diable earpiece
		}
		if(ret < 0)
		{
		    SXLOGW(" setreceiver(%d) error %s (%d)", on, ::strerror(errno), errno);
		    return false;
		}
		mReceiverOn = on;
		return true;
	}

	void AudioAMPControl::setMode(int mode)
	{
		SXLOGD("setMode mode=%d",mode);
		// do not use lock here ,because setparameter will try to get lock.
		setParameters(AUD_AMP_SET_MODE,0,mode);
		return;
	}

	int AudioAMPControl::setParameters(int command1 ,int command2 , unsigned int data)
	{
		SXLOGD("setparameters commad1 = %d command2 = %d,data=%u",command1,command2,data);
		Mutex::Autolock autoLock(mLock);
		int ret = NO_ERROR;
		AMP_Control command;
		memset(&command,0,sizeof(AMP_Control));
		switch(command1)
		{
			case AUD_AMP_SET_REGISTER:
			{
				command.command = command1;
				command.param1  = command2;
				command.param2  = data;
				ret =::ioctl(mFd,SET_EAMP_PARAMETER,&command);
				break;
			}
			case AUD_AMP_SET_AMPGAIN:
            {
                command.command = command1;
                command.param1  = data;
                ret =::ioctl(mFd,SET_EAMP_PARAMETER,&command);
                break;
            }
			case AUD_AMP_SET_MODE:
			{
				command.command = command1;
				command.param1  = data;
                mMode = data;
				ret =::ioctl(mFd,SET_EAMP_PARAMETER,&command);
				break;
			}
			default:
			break;
		}
        if(ret < 0)
        {
            SXLOGW(" setParameters(%d) error %s (%d)",command1, ::strerror(errno), errno);
        }
		return ret;
	}

	int AudioAMPControl::getParameters(int command1 ,int command2 , void * data)
	{
		SXLOGD("getparameters command = %d command2 = %d,data=%p",command1,command2,data);
		Mutex::Autolock autoLock(mLock);
		int ret = NO_ERROR;
		AMP_Control command;
        memset(&command,0,sizeof(AMP_Control));
		switch(command1)
		{
		case AUD_AMP_GET_CTRP_NUM:
		case AUD_AMP_GET_CTRP_BITS:
		case AUD_AMP_GET_REGISTER:
		case AUD_AMP_GET_AMPGAIN:
			{
				command.command = command1;
				command.param1  = command2;
				ret =::ioctl(mFd,GET_EAMP_PARAMETER,&command);
				if(data)
				{
					int * p=(int *)data;
					*p = ret;
				}
				break;
			}
		case AUD_AMP_GET_CTRP_TABLE:
			{
				command.command = command1;
				command.param1  = command2;
				command.param2  = (unsigned long int )data;
				ret =::ioctl(mFd,GET_EAMP_PARAMETER,&command);
				break;
			}

		default:
			break;

		}
        if(ret < 0)
        {
            SXLOGW(" getParameters(%d) error %s (%d)", command1, ::strerror(errno), errno);
        }
		return ret;
	}

	//AMP Control end
	//---------------------------------------------------------------------------------
}

namespace android {

	AudioAMPControlInterface *AudioDeviceManger::createInstance(int fd)
	{
		return new AudioAMPControl(fd);
	}
	void AudioDeviceManger::releaseInstance(AudioAMPControlInterface *AudioDevice)
	{
		delete AudioDevice;
	}


}

