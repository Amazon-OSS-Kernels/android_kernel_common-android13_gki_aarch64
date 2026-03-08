# SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
#
# Copyright (c) 2023 MediaTek Inc.
#

include $(srctree)/Mediatek-internal/drivers/driver/demura/board_check.mk

COBJS		:=
#UDV			:=
#VENDOR		:= Mstar
#LIB			:= $(obj)libdemura.a


ifeq ($(CONFIG_TARGET_MT5896), y)
    CHIP_NAME	:=MT5896
endif

LOCAL_PATH	:=
HAL_PATH	:= $(LOCAL_PATH)hal
BASE_PATH	:= $(LOCAL_PATH)base
MSTAR_PATH	:= $(LOCAL_PATH)mstar

###########  Include Path  ###########
UBOOTINCLUDE += -I$(srctree)/include \
                -I$(srctree)/Mediatek-internal/include \
                -I$(srctree)/Mediatek-internal/drivers/include \
				-I$(srctree)/Mediatek-internal/drivers/driver/demura/ \
                -I$(srctree)/Mediatek-internal/drivers/driver/demura/base/ \
				-I$(srctree)/Mediatek-internal/drivers/driver/demura/hal/ \
				-I$(srctree)/Mediatek-internal/drivers/driver/demura/mstar/ \
				-I$(srctree)/Mediatek-internal/impl/panel/ \
				-I$(srctree)/Mediatek-internal/impl/system/include/

###########  URSA Type  ###########
ifeq ($(URSA_TYPE),7622)
    URSA_DIR	:= M$(URSA_TYPE)
else ifeq ($(URSA_TYPE),7632)
    URSA_DIR	:= M$(URSA_TYPE)
else ifeq ($(URSA_TYPE), MT5896)
    URSA_DIR	:= $(URSA_TYPE)
else
    URSA_DIR	:= U$(URSA_TYPE)
endif

##########  Pre Define  ###########
ifeq ($(URSA_TYPE),11)
ccflags-y	+= -DCONFIG_DEMURA_URSA11
CPPFLAGS	+= -DCONFIG_DEMURA_URSA11
else ifeq ($(URSA_TYPE),13)
ccflags-y	+= -DCONFIG_DEMURA_URSA13
CPPFLAGS	+= -DCONFIG_DEMURA_URSA13
else ifeq ($(URSA_TYPE),7622)
ccflags-y	+= -DCONFIG_DEMURA_M7622
CPPFLAGS	+= -DCONFIG_DEMURA_M7622
else ifeq ($(URSA_TYPE),7632)
ccflags-y	+= -DCONFIG_DEMURA_M7632
CPPFLAGS	+= -DCONFIG_DEMURA_M7632
else ifeq ($(URSA_TYPE),MT5896)
ccflags-y	+= -DCONFIG_DEMURA_MT5896
CPPFLAGS	+= -DCONFIG_DEMURA_MT5896
endif
ccflags-y	+= -DCONFIG_DEMURA_URSA_STRING=\"$(URSA_TYPE)\"
ccflags-y	+= -DCONFIG_DEMURA_VENDOR_STRING=\"$(VENDOR)\"

###########  Base library  ###########
COBJS	+= $(BASE_PATH)/dmalloc.o
COBJS	+= $(BASE_PATH)/bits_stream_reader.o
COBJS	+= $(BASE_PATH)/read_text.o
COBJS	+= $(BASE_PATH)/ms_array.o
COBJS	+= $(BASE_PATH)/ms_vector.o
COBJS	+= $(BASE_PATH)/ms_utils.o
COBJS	+= $(BASE_PATH)/crc_libs.o
COBJS	+= $(LOCAL_PATH)MsApiDemura.o
COBJS	+= $(HAL_PATH)/halRegOp.o

ifeq ($(CONFIG_DEMURA_FCIC),y)
    DRV_PATH	:= $(LOCAL_PATH)fcic
    COBJS		+= $(DRV_PATH)/Init_IP.o
    COBJS		+= $(DRV_PATH)/MDrvFCIC.o
    COBJS		+= $(HAL_PATH)/halFCIC.o
    COBJS		:= $(COBJS)    # Unpack the macro right now
    ccflags-y	+= -I$(LOCAL_PATH)mstar
    CPPFLAGS	+= -I$(LOCAL_PATH)mstar
endif

ifeq ($(CONFIG_DEMURA_SIW),y)
    DRV_PATH	:= $(LOCAL_PATH)SIW
    COBJS		+= $(DRV_PATH)/Init_IP.o
    COBJS		+= $(DRV_PATH)/MDrvSIW.o
    COBJS		+= $(HAL_PATH)/halSIW.o
    COBJS		:= $(COBJS)    # Unpack the macro right now
    ccflags-y	+= -I$(LOCAL_PATH)mstar
    CPPFLAGS	+= -I$(LOCAL_PATH)mstar
endif

ifeq ($(CONFIG_DEMURA_SDCtiny),y)
    DRV_PATH	:= $(LOCAL_PATH)SDCtiny
    COBJS		+= $(DRV_PATH)/Init_IP.o
    COBJS		+= $(DRV_PATH)/MDrvSDCtiny.o
    COBJS		+= $(HAL_PATH)/halSDCtiny.o
    COBJS		:= $(COBJS)    # Unpack the macro right now
    ccflags-y	+= -I$(LOCAL_PATH)mstar
    CPPFLAGS	+= -I$(LOCAL_PATH)mstar
endif

ifeq ($(CONFIG_DEMURA_MSTAR),y)
    DRV_PATH	:= $(LOCAL_PATH)mstar
    COBJS		+= $(DRV_PATH)/Init_IP.o
    COBJS		+= $(HAL_PATH)/halDemura.o
    COBJS		+= $(DRV_PATH)/MDrvDemura.o
    COBJS		+= $(DRV_PATH)/convert_entry.o
    ccflags-y	+= -I$(DRV_PATH)
    CPPFLAGS	+= -I$(DRV_PATH)

    ##########  Panel Vendor  ###########
    ifneq ($(CONFIG_DEMURA_VENDOR_MSTAR),y)
        ifeq ($(CONFIG_DEMURA_VENDOR_CSOT),y)
            VENDOR	:= csot
        else ifeq ($(CONFIG_DEMURA_VENDOR_AUO),y)
            VENDOR	:= auo
            COBJS	+= $(DRV_PATH)/vendor/$(VENDOR)/parse_header.o
        else ifeq ($(CONFIG_DEMURA_VENDOR_INNOLUX),y)
            VENDOR	:= innolux
            COBJS	+= $(DRV_PATH)/vendor/$(VENDOR)/decompress_8206.o
            COBJS	+= $(DRV_PATH)/vendor/$(VENDOR)/parse_header.o
        else ifeq ($(CONFIG_DEMURA_VENDOR_LGD),y)
            VENDOR	:= lgd
            COBJS	+= $(DRV_PATH)/vendo:qr/$(VENDOR)/parse_fcic.o
        else ifeq ($(CONFIG_DEMURA_VENDOR_CHOT),y)
            VENDOR	:= chot
            COBJS	+= $(DRV_PATH)/vendor/$(VENDOR)/parse_header.o
        else ifeq ($(CONFIG_DEMURA_VENDOR_NOVA),y)
            VENDOR	:= nova
            COBJS	+= $(DRV_PATH)/vendor/$(VENDOR)/parse_header.o
        else ifeq ($(CONFIG_DEMURA_VENDOR_SDC),y)
            VENDOR	:= sdc
            COBJS	+= $(DRV_PATH)/vendor/$(VENDOR)/parse_header.o
        else ifeq ($(CONFIG_DEMURA_VENDOR_CSOT_HISILICON),y)
            VENDOR	:= csot_hisilicon
            COBJS	+= $(DRV_PATH)/vendor/$(VENDOR)/parse_header.o
        else ifeq ($(CONFIG_DEMURA_VENDOR_CSOT_HIMAX),y)
            VENDOR	:= csot_himax
            COBJS	+= $(DRV_PATH)/vendor/$(VENDOR)/parse_header.o
        else ifeq ($(CONFIG_DEMURA_VENDOR_CSOT_CSOT),y)
            VENDOR	:= csot_csot
            COBJS	+= $(DRV_PATH)/vendor/$(VENDOR)/parse_header.o
        else ifeq ($(CONFIG_DEMURA_VENDOR_MULTI),y)
            VENDOR	:= multi
            COBJS	+= $(DRV_PATH)/vendor/$(VENDOR)/decompress_8206.o
            COBJS	+= $(DRV_PATH)/vendor/$(VENDOR)/parse_header.o
            COBJS	+= $(DRV_PATH)/vendor/$(VENDOR)/parse_fcic.o
        endif
    endif

    COBJS	+= $(DRV_PATH)/URSA/$(URSA_DIR)/demura.o
    COBJS	+= $(DRV_PATH)/URSA/$(URSA_DIR)/demura_common.o
    COBJS	+= $(DRV_PATH)/URSA/$(URSA_DIR)/demuraDLL.o
    COBJS	+= $(DRV_PATH)/vendor/$(VENDOR)/decoder_input.o
    
    UBOOTINCLUDE += -I$(srctree)/Mediatek-internal/drivers/driver/demura/$(DRV_PATH)/URSA/
    UBOOTINCLUDE += -I$(srctree)/Mediatek-internal/drivers/driver/demura/$(DRV_PATH)/URSA/$(URSA_DIR)/
    UBOOTINCLUDE += -I$(srctree)/Mediatek-internal/drivers/driver/demura/$(DRV_PATH)/vendor/
    CPPFLAGS		+= -I$(DRV_PATH)/URSA/
    CPPFLAGS		+= -I$(DRV_PATH)/URSA/$(URSA_DIR)/
    CPPFLAGS		+= -I$(DRV_PATH)/vendor/
endif


##########  Check Rule  ###########
# ccflags-y	+= -Wextra
#Our function return type maybe int, double or float ..., While C default return type is "int" !
# ccflags-y	+= -Werror=implicit-function-declaration

###########  # Complie rule  ###########
#COBJS	:= $(COBJS)
#SRCS	:= $(COBJS:.o=.c)
#OBJS	:= $(addprefix $(obj),$(COBJS))

#$(LIB):	$(obj).depend $(OBJS)
#	$(call cmd_link_o_target, $(OBJS))

OBJS   := $(addprefix $(obj),$(AOBJS) $(COBJS))

obj-y += $(COBJS)

all:	$(obj).depend $(OBJS)
#########################################################################
# defines $(obj).depend target
include $(srctree)/rules.mk

sinclude $(obj).depend

#########################################################################
