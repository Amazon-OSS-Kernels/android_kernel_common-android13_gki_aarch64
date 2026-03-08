# Copyright Statement:
#
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

local_path_full := $(shell pwd)/$(LOCAL_PATH)

ifeq ($(BOARD_MTK_ONE_IMAGE),true)
BT_DRIVER_MODULE_NAME_76x8 := $(BT_DRIVER_MODULE_NAME_76x8)
else
BT_DRIVER_MODULE_NAME_76x8 := $(BT_DRIVER_MODULE_NAME)
endif
LOCAL_MODULE := $(BT_DRIVER_MODULE_NAME_76x8).ko
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR)/lib/modules

include $(BUILD_SYSTEM)/base_rules.mk

ifeq ($(TARGET_CUSTOM_KERNEL_OUT),)
TARGET_CUSTOM_KERNEL_OUT := $(KERNEL_OUT)
endif

btusb_module_target := $(LOCAL_BUILT_MODULE)

$(btusb_module_target): PRIVATE_DRIVER_LOCAL_DIR := $(local_path_full)
$(btusb_module_target): PRIVATE_DRIVER_OUT := $(btusb_module_target)
$(btusb_module_target): $(PRODUCT_OUT)/kernel
	$(hide) rm -rf $(PRIVATE_DRIVER_OUT)
	$(PREBUILT_MAKE_PREFIX)$(MAKE) -C $(TARGET_CUSTOM_KERNEL_OUT) M=$(PRIVATE_DRIVER_LOCAL_DIR) $(TARGET_KERNEL_MAKE_OPTION) modules
	$(hide) cp -f $(PRIVATE_DRIVER_LOCAL_DIR)/$(BT_DRIVER_MODULE_NAME_76x8).ko $(PRIVATE_DRIVER_OUT)
	$(PREBUILT_MAKE_PREFIX)$(MAKE) -C $(TARGET_CUSTOM_KERNEL_OUT) M=$(PRIVATE_DRIVER_LOCAL_DIR) $(TARGET_KERNEL_MAKE_OPTION) clean

local_path_full :=
btusb_module_target :=

