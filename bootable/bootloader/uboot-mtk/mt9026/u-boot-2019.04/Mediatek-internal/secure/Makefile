# SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
#
# Copyright (c) 2023 MediaTek Inc.
#

UBOOTINCLUDE += -I$(srctree)/Mediatek-internal/drivers/include/ \
                -I$(srctree)/Mediatek-internal/drivers/include/chip/ \
                -I$(srctree)/Mediatek-internal/impl/jpd/ \
                -I$(srctree)/Mediatek-internal/impl/gop/ \
                -I$(srctree)/Mediatek-internal/impl/panel/ \
                -I$(srctree)/Mediatek-internal/impl/raw/ \
                -I$(srctree)/Mediatek-internal/impl/system/ \
                -I$(srctree)/Mediatek-internal/include/ \
				-I$(srctree)/Mediatek-internal/secure/

obj-y += ./secure_boot.o

#android verify boot
obj-$(CONFIG_LIBAVB) += ./avb/avbctl_uboot.o
obj-$(CONFIG_LIBAVB) += ./avb/avb_uboot_verity.o
obj-$(CONFIG_LIBAVB) += ./avb/avb_uboot_verification.o
obj-$(CONFIG_AVB_VERIFY) += ./avb/image-android-sig.o
obj-$(CONFIG_ROLLBACK_INDEX_IN_EFUSE) += ./avb/program_efuse_rollback_index.o
obj-$(CONFIG_ROLLBACK_INDEX_IN_RPMB) += ./avb/program_rpmb_rollback_index.o
