# SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
#
# Copyright (c) 2023 MediaTek Inc.
#

UBOOTINCLUDE += -I$(srctree)/include \
                -I$(srctree)/Mediatek-internal/include \
                -I$(srctree)/Mediatek-internal/drivers/include \
				-I$(srctree)/Mediatek-internal/drivers/driver/eeprom/drv/eeprom/

COBJS += ./drv/eeprom/drvEEPROM.o

OBJS   := $(addprefix $(obj),$(AOBJS) $(COBJS))

obj-y += $(COBJS)

all:	$(obj).depend $(OBJS)

#########################################################################
# defines $(obj).depend target
include $(srctree)/rules.mk

sinclude $(obj).depend

#########################################################################
