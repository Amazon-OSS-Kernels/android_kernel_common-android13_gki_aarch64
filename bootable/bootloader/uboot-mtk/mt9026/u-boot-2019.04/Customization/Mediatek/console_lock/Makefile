#// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
#/*
# * Copyright (c) 2023 MediaTek Inc.
# */

UBOOTINCLUDE += -I$(srctree)/Mediatek-internal/impl/upgrade/include/secure/ \
		-I$(srctree)/Mediatek-internal/impl/system/include/ \
		-I$(srctree)/Customization/$(CUSTOMER)/console_lock/include/ \
		-I$(srctree)/Mediatek-internal/impl/upgrade/include/ \
		-I$(srctree)/Mediatek-internal/secure/

PLATFORM_CPPFLAGS += -DCONSOLE_LOCK_ENABLE

ifneq ($(UBOOT_CONSOLE_LOCK_PRJOECT_NAME),)
PLATFORM_CPPFLAGS += -DAN_BUILD_PROJ=$(UBOOT_CONSOLE_LOCK_PRJOECT_NAME)
endif

ccflags-y += -Werror -Wno-pointer-sign

PLATFORM_CPPFLAGS += -DMINI_GMP_UBOOT_PORTING
PLATFORM_CPPFLAGS += -DMINI_GMP_DONT_USE_FLOAT_H
#PLATFORM_CPPFLAGS += -DCMD_CE_DEBUG
PLATFORM_CPPFLAGS += -DCE_STORAGE_USE_OFFSET

obj-y += src/ce_impl.o
obj-y += src/console_impl.o
obj-y += src/ta_rpmb_impl.o
#obj-y += src/ce_test_impl.o
obj-y += src/mini-gmp.o
obj-y += src/bootenv_impl.o
obj-y += src/storage_offset_impl.o
obj-y += src/ce_ops_impl.o

obj-y += cmd/cmd_ce.o
obj-y += cmd/cmd_console_lock.o
#obj-y += cmd/cmd_ce_test.o
