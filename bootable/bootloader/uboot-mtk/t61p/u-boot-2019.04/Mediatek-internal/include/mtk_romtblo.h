/* SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause) */
/*
 * Copyright (c) 2023 MediaTek Inc.
*/

#ifndef _ROM_TABLE_HEADER_
#define _ROM_TABLE_HEADER_

typedef enum
{
    ROMTBL_FAILED = -1,
    ROMTBL_OK = 0x0,
    ROMTBL_NOT_INITED,
    ROMTBL_NO_NEEDED,
    ROMTBL_ID_NOTFOUND
} ROMTBL_RETURN;

ROMTBL_RETURN init_rom_table(char *romtbl);
ROMTBL_RETURN check_rom_table_header(void);
int romtbl_overlay(unsigned long dtb_addr);
int romtbl_overlay_to_uboot_dtb(void);
unsigned int romtbl_get_chip_id_info_dec(void);
unsigned int romtbl_get_chip_revision_info(void);
#endif /* _ROM_TABLE_HEADER_ */
