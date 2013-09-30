#
# libcameracustom
#
LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

# force libcameracustom.so to re-build, due to change sensor will not 
# change any source code. 
#$(shell touch $(TOP)/$(MTK_PATH_SOURCE)/external/mhal/src/custom/common/hal/imgsensor/$(MTK_PROJECT)/sensorlist.cpp)

$(call config-custom-folder,common: inc:hal/inc)

LOCAL_SRC_FILES += $(call all-c-cpp-files-under, common/hal/camera/camera/$(call lc,$(MTK_PLATFORM)))
LOCAL_SRC_FILES += $(call all-c-cpp-files-under, \
   common/hal/camera  \
   common/hal/flashlight  \
   common/hal/imgsensor  \
   common/hal/lens  \
   common/hal/display/display_isp_tuning \
   common/hal/eeprom)

LOCAL_C_INCLUDES += \
    $(MTK_PATH_PLATFORM)/hardware/camera/custom/common/hal/camera \
    $(MTK_PATH_PLATFORM)/hardware/camera/custom/common/hal/flashlight \
    $(MTK_PATH_PLATFORM)/hardware/camera/custom/common/hal/imgsensor \
    $(MTK_PATH_PLATFORM)/hardware/camera/custom/common/hal/lens \
    $(MTK_PATH_PLATFORM)/hardware/camera/custom/common/hal/eeprom \
    $(MTK_PATH_PLATFORM)/hardware/camera/custom/inc \
    $(MTK_ROOT)/external/mhal \
    $(MTK_ROOT)/external/mhal/inc \
    $(MTK_PATH_CUSTOM)/kernel/sensor/inc \
    $(MTK_PATH_CUSTOM)/kernel/imgsensor/inc \
    $(MTK_PATH_CUSTOM)/kernel/eeprom/inc \
    $(MTK_PATH_CUSTOM)/hal/inc \
    $(MTK_PATH_CUSTOM)/hal/inc/camera_feature \
    $(MTK_PATH_CUSTOM)/hal/inc/isp_tuning \
    $(MTK_PATH_CUSTOM)/hal/inc/aaa \
    $(MTK_PATH_CUSTOM)/hal/inc/display_isp_tuning \
    $(MTK_PATH_CUSTOM)/cgen/cfgfileinc \

LOCAL_SHARED_LIBRARIES:= liblog libcutils

LOCAL_MODULE:= libcameracustom

LOCAL_MODULE_TAGS := user 

include $(BUILD_SHARED_LIBRARY)

