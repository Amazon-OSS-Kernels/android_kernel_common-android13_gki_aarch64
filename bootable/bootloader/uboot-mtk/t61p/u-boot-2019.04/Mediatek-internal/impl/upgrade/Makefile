#
# (C) Copyright 2000-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# See file CREDITS for list of people who contributed to this
# project.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston,
# MA 02111-1307 USA
#

UBOOTINCLUDE += -I$(srctree)/Mediatek-internal/drivers/include/ \
                -I$(srctree)/Mediatek-internal/drivers/include/chip/ \
                -I$(srctree)/Mediatek-internal/impl/upgrade/include/ \
                -I$(srctree)/Mediatek-internal/impl/upgrade/include/secure/ \
                -I$(srctree)/Mediatek-internal/impl/panel/ \
                -I$(srctree)/Mediatek-internal/impl/gop/ \
                -I$(srctree)/Mediatek-internal/impl/lvgl_v6.1/ \
                -I$(srctree)/Mediatek-internal/impl/jpd/ \
                -I$(srctree)/Mediatek-internal/impl/system/include/

obj-y += ./usb_upgrade.o
obj-y += ./upgrade_osd.o
obj-y += ./upgrade_utility.o
obj-y += ./oad_upgrade.o
obj-y += ./secure/crypto_auth.o
obj-y += ./secure/crypto_func.o
obj-y += ./secure/crypto_rsa.o
obj-y += ./secure/crypto_sha.o
obj-y += ./secure/crypto_aes.o
obj-y += ./secure/secure_upgrade.o
obj-$(CONFIG_LVGL_UPGRADE_BAR) += ./picture_bar.o
