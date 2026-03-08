# SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
#
# Copyright (c) 2023 MediaTek Inc.
#

KBUILD_CFLAGS += -D'DONT_USE_CMA' -D'MSOS_TYPE_NOS' -D'MCU_ARM_CA7' -D'UTOPIA_V2'

UBOOTINCLUDE += -I$(srctree)/Mediatek-internal/drivers/driver/xc/api/xc/ \
				-I$(srctree)/Mediatek-internal/drivers/driver/xc/drv/xc/include/ \
				-I$(srctree)/Mediatek-internal/drivers/driver/xc/drv/xc/include/uapi/ \
				-I$(srctree)/Mediatek-internal/drivers/driver/xc/hal/$(BOARD)/xc/include/ \
				-I$(srctree)/Mediatek-internal/drivers/driver/graphic/hal/$(BOARD)/ge/ \
                -I$(srctree)/Mediatek-internal/drivers/driver/graphic/drv/ge/ \
                -I$(srctree)/Mediatek-internal/drivers/driver/graphic/hal/$(BOARD)/gop/ \
                -I$(srctree)/Mediatek-internal/drivers/driver/graphic/drv/gop/ \
                -I$(srctree)/Mediatek-internal/impl/system/include/ \
                -I$(srctree)/Mediatek-internal/drivers/driver/efuse/drv/

COBJS += ./hal/$(BOARD)/ge/halGE.o
COBJS += ./drv/ge/drvGE.o
COBJS += ./drv/ge/osalGE.o
COBJS += ./api/gfx/mapiGFX.o
COBJS += ./api/gfx/apiGFX.o

COBJS += ./hal/$(BOARD)/gop/halGOP.o
COBJS += ./drv/gop/drvGOP.o
COBJS += ./drv/gop/drvGFLIP.o
COBJS += ./api/gop/mapiGOP.o
COBJS += ./api/gop/apiGOP.o

OBJS   := $(addprefix $(obj),$(AOBJS) $(COBJS))

obj-y += $(COBJS)

all:	$(obj).depend $(OBJS)

#########################################################################
# defines $(obj).depend target
include $(srctree)/rules.mk

sinclude $(obj).depend

#########################################################################
