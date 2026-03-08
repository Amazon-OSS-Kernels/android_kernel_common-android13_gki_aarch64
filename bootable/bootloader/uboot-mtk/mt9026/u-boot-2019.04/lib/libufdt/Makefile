# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2000-2007
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.

# Use upstream code.
obj-y += \
	ufdt_convert.o \
	ufdt_node.o \
	ufdt_node_pool.o \
	ufdt_prop_dict.o \
	sysdeps/libufdt_sysdeps_vendor.o

obj-$(CONFIG_LIBUFDT_OVERLAY) += ufdt_overlay.o

ccflags-y := -I$(srctree)/include/linux -I$(srctree)/lib/libufdt -I$(srctree)/lib/libufdt/include -I$(srctree)/lib/libufdt/sysdeps/include
