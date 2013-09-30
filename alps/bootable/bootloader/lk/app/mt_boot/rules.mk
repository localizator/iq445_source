LOCAL_DIR := $(GET_LOCAL_DIR)

MT_BOOT_OBJ_DIR := $(LK_TOP_DIR)/build-$(PROJECT)/app/mt_boot

INCLUDES += -I$(LK_TOP_DIR)/platform/mt6577/include

ifeq ($(CUSTOM_SEC_AUTH_SUPPORT), yes)
	LIBSEC   := -L$(LK_TOP_DIR)/app/mt_boot/lib -lsec 
	OBJS += \
		$(TO_ROOT)../../../mediatek/custom/common/security/fastboot/cust_auth.o
else
	LIBSEC   := -L$(LK_TOP_DIR)/app/mt_boot/lib -lsec -lauth
endif	



OBJS += \
	$(LOCAL_DIR)/mt_boot.o \
	$(LOCAL_DIR)/fastboot.o \
	$(LOCAL_DIR)/sys_commands.o\
	$(LOCAL_DIR)/download_commands.o\
	$(LOCAL_DIR)/sparse_state_machine.o\
	$(LOCAL_DIR)/sec_wrapper.o\



ifeq ($(BUILD_SEC_LIB),yes)
AUTH_DIR  := $(LOCAL_DIR)/auth
SEC_DIR   := $(LOCAL_DIR)/sec
INCLUDES += -I$(AUTH_DIR)/inc -I$(SEC_DIR)/inc
AUTH_LOCAL_OBJS := $(patsubst $(AUTH_DIR)/%.c,$(AUTH_DIR)/%.o,$(wildcard $(AUTH_DIR)/*.c))
SEC_LOCAL_OBJS  := $(patsubst $(SEC_DIR)/%.c,$(SEC_DIR)/%.o,$(wildcard $(SEC_DIR)/*.c))
AUTH_OBJS := $(patsubst $(AUTH_DIR)/%.c,$(MT_BOOT_OBJ_DIR)/auth/%.o,$(wildcard $(AUTH_DIR)/*.c))
SEC_OBJS  := $(patsubst $(SEC_DIR)/%.c,$(MT_BOOT_OBJ_DIR)/sec/%.o,$(wildcard $(SEC_DIR)/*.c))
OBJS += $(AUTH_LOCAL_OBJS) $(SEC_LOCAL_OBJS)
endif

