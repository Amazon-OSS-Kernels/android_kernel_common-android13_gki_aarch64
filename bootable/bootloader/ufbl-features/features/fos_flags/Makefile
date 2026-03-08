# Copyright (c) 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved

#
# Common list of objects files we want to include
#
FOS_FLAGS_OBJECTS = \
    fastboot_fos_flags.o \
    fos_flags.o

# Common set of CFLAGS we need
FOS_FLAGS_CFLAGS = \
    -DSUPPORT_UBOOT \
    -DUFBL_FEATURE_FOS_FLAGS

FOS_FLAGS_HEADERS = \
    -I$(UFBL_PATH)/include

# Begin rules for kbuild U-Boot
ifeq (yes,$(strip $(UFBL_UBOOT_IS_KBUILD)))

ccflags-y    += $(FOS_FLAGS_CFLAGS) $(FOS_FLAGS_HEADERS)
obj-y        += $(FOS_FLAGS_OBJECTS)

else # default rules

include $(TOPDIR)/config.mk

FOS_FLAGS_LIB = $(obj)libfos_flags.o

# Default CFLAGS
CFLAGS += \
    $(FOS_FLAGS_CFLAGS) \
    $(FOS_FLAGS_HEADERS)

COBJS := $(FOS_FLAGS_OBJECTS)
SRCS  := $(COBJS:.o=.c)
OBJS  := $(addprefix $(obj),$(COBJS))

$(FOS_FLAGS_LIB): $(OBJS)
	$(call cmd_link_o_target, $(OBJS))

endif # (yes,$(strip $(UFBL_UBOOT_IS_KBUILD)))
