################################################################################
#
################################################################################

LOCAL_PATH := $(call my-dir)

################################################################################
#
################################################################################
include $(CLEAR_VARS)

#-----------------------------------------------------------
LOCAL_SRC_FILES += CamProfile.cpp
LOCAL_SRC_FILES += CamThreads.cpp
LOCAL_SRC_FILES += CamFormat.cpp
LOCAL_SRC_FILES += CamBuffer.cpp
LOCAL_SRC_FILES += CamFile.cpp
LOCAL_SRC_FILES += FrameProvider.cpp

#-----------------------------------------------------------
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/frameworks-ext/av/include

#-----------------------------------------------------------
LOCAL_MODULE := camera.utils

#-----------------------------------------------------------
include $(BUILD_STATIC_LIBRARY)

