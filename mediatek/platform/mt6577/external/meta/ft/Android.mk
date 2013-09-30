LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_ARM_MODE := arm
LOCAL_PRELINK_MODULE := false
PLATFORM_PATH := $(MTK_PATH_PLATFORM)/external/meta
LOCAL_SHARED_LIBRARIES := libc libnvram libcutils libaudiocustparam libaudiocompensationfilter libhwm libacdk libheadphonecompensationfilter libnetutils libmedia libhardware_legacy libfile_op libdl  libutils
                          
ifeq ($(strip $(BOARD_USES_YUSU_AUDIO)),true)
LOCAL_SHARED_LIBRARIES += libaudio.primary.default 
endif

ifeq ($(HAVE_MATV_FEATURE),yes)
LOCAL_SHARED_LIBRARIES += libmatv_cust
endif

LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/external/nvram/libfile_op \
                    $(MTK_PATH_SOURCE)/external/meta/common/inc \
                    $(PLATFORM_PATH)/include \
                    $(PLATFORM_PATH)/ft \
                    $(PLATFORM_PATH)/meta_lock \
                    $(PLATFORM_PATH)/Meta_APEditor \
                    $(PLATFORM_PATH)/lcd \
                    $(PLATFORM_PATH)/LCDBK \
                    $(PLATFORM_PATH)/cpu \
                    $(PLATFORM_PATH)/gpio \
                    $(PLATFORM_PATH)/keypadbk \
                    $(PLATFORM_PATH)/ADC \
                    $(PLATFORM_PATH)/vibrator \
                    $(PLATFORM_PATH)/msensor \
                    $(PLATFORM_PATH)/alsps \
                    $(PLATFORM_PATH)/gsensor \
                    $(PLATFORM_PATH)/gyroscope \
                    $(PLATFORM_PATH)/touch \
                    $(PLATFORM_PATH)/Audio \
                    $(PLATFORM_PATH)/sdcard \
                    $(PLATFORM_PATH)/cameratool/CCAP \
                    $(MTK_PATH_SOURCE)/platform/mt6575/hardware/camera/core/cameradebug/inc \
                    $(MTK_PATH_SOURCE)/external/mhal/src/custom/inc \
                    $(MTK_PATH_SOURCE)/external/mhal/inc \
                    $(TOP)/$(MTK_PATH_CUSTOM)/kernel/imgsensor/inc \
                    $(MTK_PATH_CUSTOM)/hal/inc 

ifeq ($(MTK_WLAN_SUPPORT),yes)
LOCAL_C_INCLUDES += $(PLATFORM_PATH)/wifi
endif

ifeq ($(MTK_GPS_SUPPORT),yes)
LOCAL_C_INCLUDES += $(PLATFORM_PATH)/gps
endif

ifeq ($(MTK_FM_SUPPORT),yes)
LOCAL_C_INCLUDES += $(PLATFORM_PATH)/fm
endif

ifeq ($(MTK_BT_SUPPORT),yes)
LOCAL_C_INCLUDES += $(PLATFORM_PATH)/bluetooth
endif

ifeq ($(HAVE_CMMB_FEATURE),yes)
LOCAL_C_INCLUDES += $(PLATFORM_PATH)/cmmb/include
endif

ifeq ($(HAVE_MATV_FEATURE),yes)
LOCAL_C_INCLUDES += $(PLATFORM_PATH)/matv \
                    $(MTK_PATH_SOURCE)/external/matvctrl
endif

ifeq ($(MTK_EMMC_SUPPORT),yes)
LOCAL_C_INCLUDES +=$(PLATFORM_PATH)/emmc
LOCAL_C_INCLUDES +=$(PLATFORM_PATH)/cryptfs
endif

LOCAL_SRC_FILES := ft_main.cpp \
                   ft_fnc.cpp

LOCAL_STATIC_LIBRARIES := libmeta_apeditor \
                          libmeta_lcd \
                          libmeta_lcdbk \
                          libmeta_cpu \
                          libmeta_lock \
                          libmeta_gpio \
                          libmeta_adc_old \
                          libmeta_vibrator \
                          libmeta_msensor \
                          libmeta_alsps \
                          libmeta_gsensor \
                          libmeta_gyroscope\
                          libmeta_touch \
                          libmeta_audio \
                          libmeta_sdcard\
                          libccap \

ifeq ($(HAVE_CMMB_FEATURE),yes)
LOCAL_STATIC_LIBRARIES += libmeta_cmmb
LOCAL_CFLAGS += \
    -DFT_CMMB_FEATURE
endif

ifeq ($(MTK_WLAN_SUPPORT),yes)
LOCAL_STATIC_LIBRARIES +=libmeta_wifi
LOCAL_CFLAGS += \
    -DFT_WIFI_FEATURE
endif

ifeq ($(MTK_GPS_SUPPORT),yes)
LOCAL_STATIC_LIBRARIES += libmeta_gps 
LOCAL_CFLAGS += \
    -DFT_GPS_FEATURE
endif

ifeq ($(MTK_FM_SUPPORT),yes)
LOCAL_STATIC_LIBRARIES += libmeta_fm   
LOCAL_CFLAGS += \
    -DFT_FM_FEATURE
endif

ifeq ($(MTK_EMMC_SUPPORT),yes)
LOCAL_STATIC_LIBRARIES +=  libmeta_clr_emmc \
                          libext4_utils \
                          libz \
                          libmtdutil \
                          libmeta_cryptfs
LOCAL_CFLAGS += \
    -DFT_EMMC_FEATURE
endif

ifeq ($(MTK_BT_SUPPORT),yes)
LOCAL_STATIC_LIBRARIES += libmeta_bluetooth 
LOCAL_CFLAGS += \
    -DFT_BT_FEATURE
endif                         
                         
ifeq ($(HAVE_MATV_FEATURE),yes)
LOCAL_STATIC_LIBRARIES += libmeta_matv
LOCAL_CFLAGS += \
    -DFT_MATV_FEATURE
endif       

LOCAL_MODULE_TAGS := user

LOCAL_MODULE := libft
include $(BUILD_SHARED_LIBRARY)

