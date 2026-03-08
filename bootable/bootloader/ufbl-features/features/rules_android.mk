#
# Copyright (c) 2016 - 2023 Amazon.com, Inc. or its affiliates.  All rights reserved.
#

# Include rules for each feature
FEATURE_DIR := $(call my-dir)

UFBL_SOURCE_FILES :=
UFBL_INCLUDE_FILES := include
UFBL_CFLAGS :=

ifeq (true,$(strip $(FEATURE_SECURE_BOOT)))
include $(FEATURE_DIR)/secure_boot/rules_android.mk
endif

ifeq (true,$(strip $(FEATURE_UNLOCK)))
include $(FEATURE_DIR)/unlock/rules_android.mk
endif

ifeq (true,$(strip $(FEATURE_SECURE_FLASHING)))
include $(FEATURE_DIR)/secure_flashing/rules_android.mk
endif

ifeq (true,$(strip $(FEATURE_TEMP_UNLOCK)))
include $(FEATURE_DIR)/temp_unlock/rules_android.mk
endif

ifeq (true,$(strip $(FEATURE_REPLAY_PROTECTED_UNLOCK)))
include $(FEATURE_DIR)/replay_protected_unlock/rules_android.mk
endif

ifeq (true,$(strip $(FEATURE_ONETIME_UNLOCK)))
include $(FEATURE_DIR)/onetime_unlock/rules_android.mk
endif

ifeq (true,$(strip $(FEATURE_USB_UPDATE_VERIFIER)))
include $(FEATURE_DIR)/usb_update_verifier/rules_android.mk
endif

ifeq (true,$(strip $(FEATURE_YUBIKEY_UNLOCK_PUB_KEY_TAG)))
include $(FEATURE_DIR)/yubikey_unlock_public_key_tag/rules_android.mk
endif
