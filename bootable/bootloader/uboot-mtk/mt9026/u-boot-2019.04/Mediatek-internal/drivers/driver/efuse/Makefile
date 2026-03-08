# SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
#
# Copyright (c) 2023 MediaTek Inc.
#

UBOOTINCLUDE += -I$(srctree)/Mediatek-internal/drivers/driver/efuse/
UBOOTINCLUDE += -I$(srctree)/Mediatek-internal/drivers/driver/efuse/drv/
UBOOTINCLUDE += -I$(srctree)/Mediatek-internal/drivers/driver/efuse/hal/

#UBOOTINCLUDE += -I$(srctree)/Mediatek-internal/drivers/include


COBJS += ./drv/drv_efuse.o
COBJS += ./hal/hal_efuse.o

OBJS   := $(addprefix $(obj),$(AOBJS) $(COBJS))

obj-y += $(COBJS)

all:	$(obj).depend $(OBJS)

#########################################################################
# defines $(obj).depend target
include $(srctree)/rules.mk

sinclude $(obj).depend

#########################################################################
