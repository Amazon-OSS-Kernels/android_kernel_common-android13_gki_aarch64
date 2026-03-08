/* SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause) */
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#ifndef _SECURE_UPGRADE_H_
#define _SECURE_UPGRADE_H_

#include <secure/secure_common.h>

#if defined(CONFIG_SECURE_UPGRADE_V2)
void secure_halt_cpu(void);
int firmware_image_header_authentication(unsigned char *header_buf);
int firmware_image_authentication(const char *interface,char *upgrade_file, int device, int partition);
int do_file_segment_rsa_authendication(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);
int do_file_part_load_with_segment_aes_decrypted(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);
unsigned char get_secure_init_status(void);
#endif
#endif

