# SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
#
# Copyright (c) 2023 MediaTek Inc.
#

ifeq ($(LINUX_ONLY),true)
-include $(BOOTLOADER_DIR)/u-boot-2019.04/Android.mk
endif
