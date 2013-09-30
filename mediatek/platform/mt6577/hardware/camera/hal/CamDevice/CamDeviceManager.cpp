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

#define LOG_TAG "MtkCam/CamDevice"
//
#include <mhal/inc/camera.h>
#include <mhal/inc/camera/ioctl.h>
//
#include "Utils/inc/CamUtils.h"
//
#include "inc/ICamDevice.h"
#include "inc/CamDeviceManager.h"
//
#define MAX_SIMUL_CAMERAS_SUPPORTED (1)


/******************************************************************************
*
*******************************************************************************/
#define MY_LOGD(fmt, arg...)    CAM_LOGD("[CamDeviceManager::%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)    CAM_LOGI("[CamDeviceManager::%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)    CAM_LOGW("[CamDeviceManager::%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)    CAM_LOGE("[CamDeviceManager::%s] "fmt, __FUNCTION__, ##arg)


/******************************************************************************
*
*******************************************************************************/
namespace android {


/******************************************************************************
*
*******************************************************************************/
namespace {
    #warning "[FIXME] remove magic numbers for g_CamDevInfo[8]"
    CamDevInfo          g_CamDevInfo[8];

}   //namespace


/******************************************************************************
*
*******************************************************************************/
CamDeviceManager&
CamDeviceManager::
getInstance()
{
    static  CamDeviceManager singleton;
    return  singleton;
}


/******************************************************************************
*
*******************************************************************************/
CamDeviceManager::
CamDeviceManager()
    : mMtxOpenLock()
    , mi4DeviceNum(0)
    , mi4OpenNum(0)
{
}


/******************************************************************************
*
*******************************************************************************/
CamDeviceManager::
~CamDeviceManager()
{
}


/******************************************************************************
*
*******************************************************************************/
int32_t
CamDeviceManager::
getNumberOfCameras()
{
    mhalCamSensorInfo_t mhalcamsensorinfo[sizeof(g_CamDevInfo)/sizeof(CamDevInfo)];
    mhalCamSensorInfo_t*pinfo = &mhalcamsensorinfo[0];
    uint32_t            u4RetSize = 0;
    //
    android::Mutex::Autolock lock(mMtxOpenLock);
    //
    mi4DeviceNum = 0;
    //
    void* mhalFd = ::mHalOpen();
    if  ( ! mhalFd )
    {
        MY_LOGE("mHalOpen() return NULL");
        return  0;
    }
    //
    ::mHalIoctl(mhalFd, MHAL_IOCTL_SEARCH_CAMERA, NULL, 0, pinfo, sizeof(mhalCamSensorInfo_t), &u4RetSize);
    ::mHalClose(mhalFd);
    //
    mi4DeviceNum = u4RetSize / sizeof(mhalCamSensorInfo_t);
    MY_LOGI("mi4DeviceNum(%d)", mi4DeviceNum);
    //
    for (int i = 0; i < mi4DeviceNum; i++)
    {
        MY_LOGI("id(%d) (facing,orientation)=(%d,%d)", i, pinfo[i].facing, pinfo[i].orientation);
        CamDevInfo& rDevInfo = g_CamDevInfo[i];
        rDevInfo.facing     = pinfo[i].facing;
        rDevInfo.orientation= pinfo[i].orientation;
        switch  (pinfo[i].devType)
        {
        case MHAL_CAM_SENSOR_DEV_ATV:
            rDevInfo.eDevID = eDevId_AtvSensor;
            break;
        default:
            rDevInfo.eDevID = eDevId_ImgSensor;
            break;
        }
    }
    //
    return  mi4DeviceNum;
}


/******************************************************************************
*
*******************************************************************************/
int
CamDeviceManager::
getCameraInfo(int const cameraId, camera_info& rInfo)
{
    rInfo = g_CamDevInfo[cameraId];
    MY_LOGI("id(%d) (facing,orientation)=(%d,%d)", cameraId, rInfo.facing, rInfo.orientation);
    return  0;
}


/******************************************************************************
*
*******************************************************************************/
int
CamDeviceManager::
openDevice(const hw_module_t* module, const char* name, hw_device_t** device)
{
    int err = 0;
    //
    ICamDevice* pdev = NULL;
    int32_t     i4OpenId = 0;
    //
    Mutex::Autolock lock(mMtxOpenLock);
    //
    MY_LOGI("+ mi4OpenNum(%d)", mi4OpenNum);

    if (name != NULL)
    {
        i4OpenId = ::atoi(name);
        //
        if  ( mi4DeviceNum < i4OpenId )
        {
            err = -EINVAL;
            goto lbExit;
        }
        //
        if  ( MAX_SIMUL_CAMERAS_SUPPORTED <= mi4OpenNum )
        {
            MY_LOGW("open number(%d) >= maximum number(%d)", mi4OpenNum, MAX_SIMUL_CAMERAS_SUPPORTED);
//            err = -ENOMEM;
//            goto lbExit;
        }
        //
        pdev = ICamDevice::createInstance(
            i4OpenId, 
            g_CamDevInfo[i4OpenId], 
            *get_hw_device(), 
            module
        );
        //
        if  ( ! pdev )
        {
            MY_LOGE("camera device allocation fail: pdev(%p)", pdev);
            err = -ENOMEM;
            goto lbExit;
        }

        *device = pdev->get_hw_device();
        //
        mi4OpenNum++;
    }

lbExit:
    if  ( err )
    {
        if  ( pdev )
        {
            pdev->destroyInstance();
            pdev = NULL;
        }
        //
        *device = NULL;
    }
    MY_LOGI("- mi4OpenNum(%d)", mi4OpenNum);
    return  err;
}


/******************************************************************************
*
*******************************************************************************/
int
CamDeviceManager::
closeDevice(hw_device_t* device)
{
    int err = 0;
    //
    ICamDevice* pdev = ICamDevice::getIDev(reinterpret_cast<camera_device*>(device));
    //
    MY_LOGI("+ device(%p), ICamDevice(%p)", device, pdev);
    //
    Mutex::Autolock lock(mMtxOpenLock);
    //
    if  ( pdev )
    {
        mi4OpenNum--;
        pdev->destroyInstance();
        pdev = NULL;
        err = 0;
    }
    else
    {
        err = -EINVAL;
    }
    //
    MY_LOGI("- status(%d)", err);
    return  err;
}


////////////////////////////////////////////////////////////////////////////////
//  Implementation of hw_device_t
////////////////////////////////////////////////////////////////////////////////
int
CamDeviceManager::
close_device(hw_device_t* device)
{
    return  CamDeviceManager::getInstance().closeDevice(device);
}


hw_device_t const*
CamDeviceManager::
get_hw_device()
{
    static
    hw_device_t const
    _device =
    {
        /** tag must be initialized to HARDWARE_DEVICE_TAG */
        tag:        HARDWARE_DEVICE_TAG, 
        /** version number for hw_device_t */
        version:    0, 
        /** reference to the module this device belongs to */
        module:     NULL, 
        /** padding reserved for future use */
        reserved:   {0}, 
        /** Close this device */
        close:      CamDeviceManager::close_device, 
    };

    return  &_device;
}


////////////////////////////////////////////////////////////////////////////////
//  Implementation of hw_module_methods_t
////////////////////////////////////////////////////////////////////////////////
int
CamDeviceManager::
open_device(const hw_module_t* module, const char* name, hw_device_t** device)
{
    return  CamDeviceManager::getInstance().openDevice(module, name, device);
}


hw_module_methods_t*
CamDeviceManager::
get_module_methods()
{
    static
    hw_module_methods_t
    _methods =
    {
        open:   CamDeviceManager::open_device
    };

    return  &_methods;
}


////////////////////////////////////////////////////////////////////////////////
//  Implementation of camera_module_t
////////////////////////////////////////////////////////////////////////////////
int
CamDeviceManager::
get_number_of_cameras(void)
{
    return  CamDeviceManager::getInstance().getNumberOfCameras();
}


int
CamDeviceManager::
get_camera_info(int cameraId, camera_info *info)
{
    return  CamDeviceManager::getInstance().getCameraInfo(cameraId, *info);
}


}; // namespace android

