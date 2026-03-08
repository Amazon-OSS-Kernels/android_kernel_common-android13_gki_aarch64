# SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
#
# Copyright (c) 2023 MediaTek Inc.
#

KBUILD_CFLAGS += -D'DONT_USE_CMA' -D'MSOS_TYPE_NOS' -D'MCU_ARM_CA7' -D'UTOPIA_V2'

UBOOTINCLUDE += -I$(srctree)/Mediatek-internal/drivers/driver/msos/utopia_core/ \
				-I$(srctree)/Mediatek-internal/drivers/include/ \
                -I$(srctree)/Mediatek-internal/drivers/include/chip/ \
                -I$(srctree)/Mediatek-internal/drivers/include/tools/

ifeq ($(BOARD),mt5888)
obj-y += cpu/
obj-y += graphic/
obj-y += miu/
obj-y += msos/
obj-y += mvop/
obj-y += njpd/
obj-y += sem/
obj-y += sys/
obj-y += xc/
endif
obj-y += graphic/
obj-$(CONFIG_ENABLE_DEMURA) += demura/
obj-y += efuse/
obj-$(CONFIG_ENABLE_EEPROM) += eeprom/
