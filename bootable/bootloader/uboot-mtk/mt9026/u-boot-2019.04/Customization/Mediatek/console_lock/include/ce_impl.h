/* SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause) */
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#ifndef __CE_IMPL_H__
#define __CE_IMPL_H__

#include <hexdump.h>
#include <debug_impl.h>
#include <vsprintf.h>

#ifdef CMD_CE_DEBUG
#define trace_dump_hex(b, l, info) do { \
	printf(info"\n");	\
	print_hex_dump("", DUMP_PREFIX_OFFSET, 16, 1, b, l, 1); \
} while(0)
#else
#define trace_dump_hex(b, l, info) do { } while(0)
#endif

int do_ce(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);
int do_check_ce(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[]);
bool ce_verify(uint8_t *in, size_t in_len, uint8_t *ans, size_t ans_len);
uint8_t *get_response_signed(size_t *len);

#ifdef CONFIG_CONSOLE_UNLOCK_USB
int do_ce_usb(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[]);
int sha256_crypto_prefix(uint8_t *in, size_t in_len, uint8_t *out, size_t out_len);
uint8_t *get_preset_prefix(size_t *len);
const char *get_device_ID(void);
const char *get_FW_version(void);
const char *get_serial_number(void);
const char *get_response_filename(void);
#else
int sha256_crypto_magic(uint8_t *in, size_t in_len, uint8_t *out, size_t out_len);
uint8_t *get_preset_nonce(size_t *len);
const char *get_build_type(void);
const char *get_build_project(void);
#endif

#endif
