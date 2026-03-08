// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#include <command.h>
#include <common.h>
#include <usb_upgrade.h>
#include <upgrade_osd.h>
#include <upgrade_utility.h>
#include <secure/secure_upgrade.h>
#include <oad_upgrade.h>


#if (CONFIG_MTK_UPGRADE == 1)
U_BOOT_CMD(
    mtkupgrade ,    CONFIG_SYS_MAXARGS,    1,     do_mtkupgrade,
    "do upgrade by different upgrade source & specified file\n",
    "<upgrade_source> [-script] [file_name]\n"
    "    - upgrade_source: usb, oad, net\n"
    "    - script   : Input file_name is script format.(NOT PKG format)\n"
    "                 script format only includes upgrade cmd\n"
    "                 PKG format includes header, upgrade cmd, image\n"
    "                 -script only supported by upgrade_source is usb\n"
    "    - file_name: User can specify the file name to be upgraded.\n"
    "                 If 'file_name' is omitted, it will check 'ForceUpgradePath' in environment first.\n"
    "                 If 'ForceUpgradePath' is not defined in environment, use below default file name.\n"
    "                 PKG format default file name is /MTKUpgrade.bin\n"
    "                 Script format default file name is /usb_auto_update.txt\n"
    "    - ex: mtkupgrade usb\n"
    "    - ex: mtkupgrade usb CusUpgrade.bin\n"
    "    - ex: mtkupgrade usb -script\n"
    "    - ex: mtkupgrade usb -script [[mboot\n"
);
#endif


#if (CONFIG_OAD_UPGRADE == 1)
U_BOOT_CMD(
    check_upgrade_mode ,    CONFIG_SYS_MAXARGS,    1,     do_check_upgrade_mode,
    "this command is used in bootflow only\n",
    "    - when power on, this command will check upgrade_mode.ini first.\n"
    "    - upgrade_mode oad or updatelogic\n"
    "    - upgrade_filename and upgrade_partition\n"
    "    - ex: check_upgrade_mode\n"
);
#endif


#if (CONFIG_USB_UPGRADE == 1)
#if (CONFIG_USB_AUTO_UPGRADE == 1)
U_BOOT_CMD(
    check_usb_auto_upgrade ,    CONFIG_SYS_MAXARGS,    1,     do_check_usb_auto_upgrade,
    "this command is used in bootflow only\n",
    "    - when power on, this command will check usb_auto_upgrade in env first.\n"
    "    - if usb_auto_upgrade=disable_once, not need to do usb upgrade next time.\n"
    "    - if usb_auto_upgrade=disable, not need to do usb upgrade forever.\n"
    "    - if usb_auto_upgrade=NULL or enable or others, do usb upgrade this time.\n"
    "    - ex: check_usb_auto_upgrade\n"
);
#endif

#if (CONFIG_MMC ==1)
U_BOOT_CMD(
    usb_partial_upgrade_to_emmc,    CONFIG_SYS_MAXARGS,    1,    do_usb_partial_upgrade_to_emmc,
    "partial upgrade from USB to eMMC\n",
    "<file_name> <partition_name>\n"
    "ex: usb_partial_upgrade_to_emmc mboot.bin mboot_a\n"
);
#endif
#endif

#if (CONFIG_UPGRADE_OSD == 1)
U_BOOT_CMD(
    osd_create,    3,   1,  do_create_osdlayer,
    "osd_create - create osd layer \n",
    "create osd layer \n"
    "    - w: in Dec\n"
    "    - h: in Dec\n"
    "    - ex: osd_create 720 576\n"
);

U_BOOT_CMD(
    osd_flush,    1,   1,  do_flush2screen_osdlayer,
    "osd_flush - flush canvas to screen \n",
    "flush canvas to screen \n"
    "    - ex: osd_flush\n"
);

U_BOOT_CMD(
    draw_rect,    6,   1,  do_draw_rect,
    "draw_rect - draw rect with color \n",
    "draw rect \n"
    "    - x: in Dec\n"
    "    - y: in Dec\n"
    "    - w: in Dec\n"
    "    - h: in Dec\n"
    "    - color: in Hex\n"
    "    - ex: draw_rect 60 88 600 400 0x800000ff\n"
);

U_BOOT_CMD(
    draw_string,    100,   1,  do_draw_string,
    "draw_string - draw string with color \n",
    "draw string \n"
    "    - string: in char\n"
    "    - x: in Dec\n"
    "    - y: in Dec\n"
    "    - color: in Hex\n"
    "    - attrib: align attrib, 0: left, 1: middle, 2: right\n"
    "    - ex: draw_string 60 188 0x3fffffff 1 3554E\n"
);

U_BOOT_CMD(
    draw_progress,    5,   1,  do_draw_progress,
    "draw_string - draw string with color \n",
    "draw string \n"
    "    - x: in Dec\n"
    "    - y: in Dec\n"
    "    - color: in Hex\n"
    "    - per: percent in Dec\n"
    "    - ex: draw_progress 110 338 0x3fffffff 1\n"
);
#endif

#if defined(CONFIG_SECURE_UPGRADE_V2)
U_BOOT_CMD(
    fileSegRSA, 4, 0, do_file_segment_rsa_authendication,
    "do file segment rsa authendication",
    "fileSegRSA <interface> <dev[:part> <filename>\n"
    "interface: usb, FS\n"
);

U_BOOT_CMD(
    filepartloadSegAES, 7, 0, do_file_part_load_with_segment_aes_decrypted,
    "load a part of file to RAM and do descrypted",
    "filepartloadSegAES interface(hex) device(hex) addr(hex) filename(hex) bytes(hex) offset(hex)\n"
);
#endif


U_BOOT_CMD(
    filepartload,    5,   0,  do_file_part_load,
    "load part of a file to DRAM\n",
    "<addr> <filename> <bytes> <pos> \n"
    "    - load part of a file 'filename' to address 'addr'.\n"
    "    - 'pos' gives the file position to start loading from.\n"
    "    - 'bytes' gives the size to load.\n"
    "    - ex: filepartload 0x25000000 MTKUpgrade.bin 0x200 0x300\n"
);
