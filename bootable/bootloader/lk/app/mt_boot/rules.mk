LOCAL_DIR := $(GET_LOCAL_DIR)

MT_BOOT_OBJ_DIR := $(LK_TOP_DIR)/build-$(PROJECT)/app/mt_boot

ifeq ($(CUSTOM_SEC_AUTH_SUPPORT), yes)
	OBJS += \
		$(TO_ROOT)../../../mediatek/custom/common/security/fastboot/cust_auth.o
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
DEVINFO_DIR   := $(LOCAL_DIR)/devinfo
SEC_PLAT_DIR   := $(LOCAL_DIR)/platform
INCLUDES += -I$(AUTH_DIR)/inc -I$(SEC_DIR)/inc -I$(SEC_PLAT_DIR) -I$(DEVINFO_DIR)
AUTH_LOCAL_OBJS := $(patsubst $(AUTH_DIR)/%.c,$(AUTH_DIR)/%.o,$(wildcard $(AUTH_DIR)/*.c))
SEC_LOCAL_OBJS  := $(patsubst $(SEC_DIR)/%.c,$(SEC_DIR)/%.o,$(wildcard $(SEC_DIR)/*.c))
DEVINFO_LOCAL_OBJS  := $(patsubst $(DEVINFO_DIR)/%.c,$(DEVINFO_DIR)/%.o,$(wildcard $(DEVINFO_DIR)/*.c))
SEC_PLAT_LOCAL_OBJS  := $(patsubst $(SEC_PLAT_DIR)/%.c,$(SEC_PLAT_DIR)/%.o,$(wildcard $(SEC_PLAT_DIR)/*.c))
AUTH_OBJS := $(patsubst $(AUTH_DIR)/%.c,$(MT_BOOT_OBJ_DIR)/auth/%.o,$(wildcard $(AUTH_DIR)/*.c))
SEC_OBJS  := $(patsubst $(SEC_DIR)/%.c,$(MT_BOOT_OBJ_DIR)/sec/%.o,$(wildcard $(SEC_DIR)/*.c))
DEVINFO_OBJS  := $(patsubst $(DEVINFO_DIR)/%.c,$(MT_BOOT_OBJ_DIR)/devinfo/%.o,$(wildcard $(DEVINFO_DIR)/*.c))
SEC_PLAT_OBJS  := $(patsubst $(SEC_PLAT_DIR)/%.c,$(MT_BOOT_OBJ_DIR)/platform/%.o,$(wildcard $(SEC_PLAT_DIR)/*.c))
OBJS += $(AUTH_LOCAL_OBJS) $(SEC_LOCAL_OBJS) $(SEC_PLAT_LOCAL_OBJS) $(DEVINFO_LOCAL_OBJS)
endif

