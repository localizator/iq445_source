################################################################################
#
# hw/<CAMERA_HARDWARE_MODULE_ID>.<ro.hardware>.so
# hw/<CAMERA_HARDWARE_MODULE_ID>.default.so
#
################################################################################

LOCAL_PATH := $(call my-dir)

################################################################################
#
################################################################################
include $(CLEAR_VARS)

#$(call make-private-dependency,\
#  $(BOARD_CONFIG_DIR)/configs/camera_config.mk \
#)


#-----------------------------------------------------------
LOCAL_SRC_FILES += CamModule.cpp

#-----------------------------------------------------------
LOCAL_C_INCLUDES += 

#-----------------------------------------------------------
LOCAL_WHOLE_STATIC_LIBRARIES += camera.utils
#
LOCAL_STATIC_LIBRARIES += camera.cam.device
LOCAL_STATIC_LIBRARIES += camera.cam.adapter
LOCAL_STATIC_LIBRARIES += camera.display.adapter
LOCAL_STATIC_LIBRARIES += camera.client.callback

#-----------------------------------------------------------
LOCAL_SHARED_LIBRARIES += liblog
LOCAL_SHARED_LIBRARIES += libutils
LOCAL_SHARED_LIBRARIES += libcutils
LOCAL_SHARED_LIBRARIES += libbinder
LOCAL_SHARED_LIBRARIES += libui
LOCAL_SHARED_LIBRARIES += libcamera_client
LOCAL_SHARED_LIBRARIES += libskia
LOCAL_SHARED_LIBRARIES += libmhal
LOCAL_SHARED_LIBRARIES += libmhalmdp
LOCAL_SHARED_LIBRARIES += libcameraprofile
LOCAL_SHARED_LIBRARIES += libcameracustom

#-----------------------------------------------------------
#LOCAL_MODULE_TAGS := optional 
LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)/hw
LOCAL_MODULE := camera.default

#-----------------------------------------------------------
include $(BUILD_SHARED_LIBRARY)


################################################################################
#
################################################################################
include $(CLEAR_VARS)
include $(call all-makefiles-under,$(LOCAL_PATH))

