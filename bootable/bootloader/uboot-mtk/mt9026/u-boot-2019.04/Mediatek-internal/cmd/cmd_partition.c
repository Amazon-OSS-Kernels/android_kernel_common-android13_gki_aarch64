/* SPDX-License-Identifier: GPL-2.0-only OR BSD-3-Clause */
/******************************************************************************
 *
 * This file is provided under a dual license.  When you use or
 * distribute this software, you may choose to be licensed under
 * version 2 of the GNU General Public License ("GPLv2 License")
 * or BSD License.
 *
 * GPLv2 License
 *
 * Copyright(C) 2019 MediaTek Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See http://www.gnu.org/licenses/gpl-2.0.html for more details.
 *
 * BSD LICENSE
 *
 * Copyright(C) 2019 MediaTek Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *  * Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *****************************************************************************/

#include <command.h>
#include <common.h>
#include <mmc.h>
#include <partition.h>
#include <debug_impl.h>

/**
 * do_partition(): Perform partition operations
 *
 * @param cmdtp - command name
 * @param flag
 * @param argc
 * @param argv
 *
 * @return zero on success; otherwise error
 */
static int do_partition(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int ret = CMD_RET_SUCCESS;

	if (argc < 5 || argc > 8)
		return CMD_RET_USAGE;


	if ((strcmp(argv[1], "read") == 0) && ((argc == 7)||(argc == 8))) {
		ret = do_partition_read(argv[2],argv[3],argv[4],argv[5],argv[6],argv[7]);
	} else if ((strcmp(argv[1], "read.boot") == 0) && ((argc == 7)||(argc == 8))) {
		ret = do_partition_read_boot(argv[2],argv[3],argv[4],argv[5],argv[6],argv[7]);
	} else if ((strcmp(argv[1], "write") == 0) && ((argc == 7)||(argc == 8))) {
		ret = do_partition_write(argv[2],argv[3],argv[4],argv[5],argv[6],argv[7]);
	} else if ((strcmp(argv[1], "write.boot") == 0) && ((argc == 7)||(argc == 8))) {
		ret = do_partition_write_boot(argv[2],argv[3],argv[4],argv[5],argv[6],argv[7]);
#if CONFIG_IS_ENABLED(IMAGE_SPARSE)
	} else if ((strcmp(argv[1], "swrite") == 0) && (argc == 7)) {
		ret = do_partition_swrite(argv[2],argv[3],argv[4],argv[5],argv[6]);
#endif
	} else if ((strcmp(argv[1], "erase") == 0) && (argc == 5)) {
		ret = do_partition_erase(argv[2],argv[3],argv[4]);
	} else if ((strcmp(argv[1], "erase.boot") == 0) && (argc == 5)) {
		ret = do_partition_erase_boot(argv[2],argv[3],argv[4]);
#if (CONFIG_AB_SIDELOAD == 1)
	} else if ((strcmp(argv[1], "clone") == 0) && (argc == 5)) {
		ret = do_partition_clone(argv[2], argv[3], argv[4]);
#endif
	} else {
		return CMD_RET_USAGE;
	}

	if (ret) {
		UBOOT_ERROR("do_partition error!\n");
		return CMD_RET_FAILURE;
	}

	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(
	partition, 29, 1, do_partition,
	"Operating flash with partition as the unit.",
	"read <interface> <dev> <partition_name> <addr> <byte_count> [pos]\n"
	"    - Read 'byte_count' bytes from 'partition_name' to address 'addr'\n"
	"    - 'pos' gives the partition position to start reading from.\n"
	"    - The unit of 'byte_count' & 'pos' is byte in hexadecimal\n"
	"    - If 'pos' is omitted, 0 is used.\n"
	"    - ex : partition read mmc 0 mboot_a 0x28000000 0x200\n"
	"    - ex : partition read usb 0 mboot_b 0x29000000 0x400\n"
	"    - ex : partition read mmc 0 mboot_a 0x28000000 0x200 0x800\n"
	"partition read.boot <interface> <dev> <partition_number> <addr> <byte_count> [pos]\n"
	"    - Read 'byte_count' bytes bootdata from 'partition_number' of 'dev' on 'interface'\n"
	"      to address 'addr'\n"
	"    - 'pos' gives the partition position to start reading from.\n"
	"    - The unit of 'byte_count' & 'pos' is byte in hexadecimal\n"
	"    - If 'pos' is omitted, 0 is used.\n"
	"    - ex : partition read.boot mmc 0 1 0x28000000 0x200\n"
	"    - ex : partition read.boot mmc 0 1 0x28000000 0x200 0x800\n"
	"partition write <interface> <dev> <partition_name> <addr> <byte_count> [pos]\n"
	"    - Write 'byte_count' bytes to 'partition_name' from address 'addr'\n"
	"    - 'pos' gives the partition position to start writing from.\n"
	"    - The unit of 'byte_count' & 'pos' is byte in hexadecimal\n"
	"    - If 'pos' is omitted, 0 is used.\n"
	"    - ex : partition write mmc 0 mboot_a 0x28000000 0x200\n"
	"    - ex : partition write usb 0 mboot_b 0x29000000 0x400\n"
	"    - ex : partition write mmc 0 mboot_a 0x28000000 0x200 0x800\n"
	"partition write.boot <interface> <dev> <partition_number> <addr> <byte_count> [pos]\n"
	"    - Write 'byte_count' bytes bootdata from address 'addr'\n"
	"      to 'partition_number' of 'dev' on 'interface'\n"
	"    - 'pos' gives the partition position to start writing from.\n"
	"    - The unit of 'byte_count' & 'pos' is byte in hexadecimal\n"
	"    - If 'pos' is omitted, 0 is used.\n"
	"    - ex : partition write.boot mmc 0 1 0x28000000 0x200\n"
	"    - ex : partition write.boot mmc 0 1 0x28000000 0x200 0x800\n"
#if CONFIG_IS_ENABLED(IMAGE_SPARSE)
	"partition swrite <interface> <dev> <partition_name> <addr> <byte_count>\n"
	"    - If no sparse image in 'addr', this cmd acts as 'partition write' cmd\n"
	"      otherwise this cmd writes the whole partition and not references <byte_count>\n"
	"    - The unit of 'byte_count' is byte in hexadecimal\n"
	"    - ex : partition swrite mmc 0 super 0x28000000 0x200\n"
	"    - ex : partition swrite usb 0 super 0x29000000 0x400\n"
#endif
	"partition erase <interface> <dev> <partition_name>\n"
	"    - Erase all data on 'partition_name'\n"
	"    - ex : partition erase mmc 0 mboot_a\n"
	"partition erase.boot <interface> <dev> <partition_number>\n"
	"    - Erase all bootdata on 'partition_number' of 'dev' on 'interface'\n"
	"    - ex : partition erase.boot mmc 0 1\n"
#if (CONFIG_AB_SIDELOAD == 1)
	"partition clone <interface> <dev> <action>\n"
	"    - Clone partitions between A/B for ease of A/B boot test\n"
	"    - There are 3 actions:\n"
	"    -   0 : If running slot is A (or B), then clone all _a (or _b) partitions into _b (or _a).\n"
	"    -   1 : Clone all _a partitions into _b.\n"
	"    -   2 : Clone all _b partitions into _a.\n"
	"    - ex : partition clone mmc 0 0\n\n"
#endif
);

static int do_boot_partition(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int ret = CMD_RET_SUCCESS;

	if (argc < 5 || argc > 6)
		return CMD_RET_USAGE;


	if ((strcmp(argv[1], "read") == 0) && ((argc == 5)||(argc == 6))) {
		ret = do_boot_partition_read(argv[2],argv[3],argv[4],argv[5]);

	} else if ((strcmp(argv[1], "write") == 0) && ((argc == 5)||(argc == 6))) {
		ret = do_boot_partition_write(argv[2],argv[3],argv[4],argv[5]);

#if CONFIG_IS_ENABLED(IMAGE_SPARSE)
	} else if ((strcmp(argv[1], "swrite") == 0) && (argc == 5)) {
		ret = do_boot_partition_swrite(argv[2],argv[3],argv[4]);

#endif
	}
	else {
		return CMD_RET_USAGE;
	}

	if (ret) {
		UBOOT_ERROR("do_partition_raw error!\n");
		return CMD_RET_FAILURE;
	}

	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(
	bootpartition, 29, 1, do_boot_partition,
	"Auto-Detect boot flash (mmc/usb/ufs) type and "
	"operating flash with partition as the unit.\n"
	"This command is only allowed when there is only one boot device on chip.",
	"read <partition_name> <addr> <byte_count> [pos]\n"
	"    - Read 'byte_count' bytes from 'partition_name' to address 'addr'\n"
	"    - 'pos' gives the partition position to start reading from.\n"
	"    - The unit of 'byte_count' & 'pos' is byte in hexadecimal\n"
	"    - If 'pos' is omitted, 0 is used.\n"
	"    - ex : bootpartition read mboot_a 0x28000000 0x200\n"
	"    - ex : bootpartition read mboot_a 0x28000000 0x200 0x800\n"
	"bootpartition write <partition_name> <addr> <byte_count> [pos]\n"
	"    - Write 'byte_count' bytes to 'partition_name' from address 'addr'\n"
	"    - 'pos' gives the partition position to start writing from.\n"
	"    - The unit of 'byte_count' & 'pos' is byte in hexadecimal\n"
	"    - If 'pos' is omitted, 0 is used.\n"
	"    - ex : bootpartition write mboot_a 0x28000000 0x200\n"
	"    - ex : bootpartition write mboot_a 0x28000000 0x200 0x800\n"
#if CONFIG_IS_ENABLED(IMAGE_SPARSE)
	"bootpartition swrite <partition_name> <addr> <byte_count>\n"
	"    - If no sparse image in 'addr', this cmd acts as 'partition write' cmd\n"
	"      otherwise this cmd writes the whole partition and not references <byte_count>\n"
	"    - The unit of 'byte_count' is byte in hexadecimal\n"
	"    - ex : bootpartition swrite super 0x28000000 0x200\n"
#endif

);

