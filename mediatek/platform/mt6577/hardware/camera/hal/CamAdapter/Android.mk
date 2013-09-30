################################################################################
#
################################################################################

LOCAL_PATH := $(call my-dir)

################################################################################
#
################################################################################
include $(CLEAR_VARS)

#-----------------------------------------------------------
LOCAL_SRC_FILES += BaseCamAdapter.cpp
LOCAL_SRC_FILES += mHal/mHalBaseAdapter.cpp
LOCAL_SRC_FILES += mHal/mHalCamAdapter.cpp
LOCAL_SRC_FILES += mHal/mHalCamCmdQueThread.cpp
LOCAL_SRC_FILES += mHal/mHalCamImage.cpp
LOCAL_SRC_FILES += mHal/mHalCamUtils.cpp
LOCAL_SRC_FILES += mHal/mHalCamParameters.cpp
LOCAL_SRC_FILES += mHal/mHalCamEngParam.cpp

#-----------------------------------------------------------
LOCAL_C_INCLUDES += $(LOCAL_PATH)/..
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/kernel/include/
LOCAL_C_INCLUDES += $(TOP)/$(MTK_ROOT)/external
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/frameworks-ext/av/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM)/hal/inc/camera_feature
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM)/kernel/imgsensor/inc
#
LOCAL_C_INCLUDES += $(TOP)/external/skia/include/core
LOCAL_C_INCLUDES += $(TOP)/external/skia/include/images

#-----------------------------------------------------------
LOCAL_MODULE := camera.cam.adapter

#-----------------------------------------------------------
include $(BUILD_STATIC_LIBRARY)

