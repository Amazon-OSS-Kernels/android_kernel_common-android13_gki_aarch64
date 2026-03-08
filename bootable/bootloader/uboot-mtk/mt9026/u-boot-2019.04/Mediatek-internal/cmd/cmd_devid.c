// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
*/

#include <command.h>
#include <linux/stddef.h>
#include <linux/sizes.h>
#include <memalign.h>
#include <utility.h>
#include <errno.h>

#define APMCU_RIUBASE	0x1C000000

#define DEV_ID_LENGTH (6)
#define ELEMENT_BUF_SIZE (5)
#define DID_BUF_SIZE (23)

#define IO_READ16(base, offset) \
	 *((volatile unsigned short*)(base + offset))

#define EFUSE_DID_BASE			(APMCU_RIUBASE + 0x40600)

#define REG_DEVICE_ID_0			(0x0000)    //0x00
#define REG_DEVICE_ID_1			(0x0004)    //0x01
#define REG_DEVICE_ID_2			(0x0008)    //0x02
#define REG_DEVICE_ID_3			(0x0010)    //0x04
#define REG_DEVICE_ID_4			(0x0014)    //0x05
#define REG_DEVICE_ID_5			(0x0018)    //0x06

int get_devid(char *des)
{
	unsigned short device_id[DEV_ID_LENGTH] = {0};
	unsigned char *ptr;
	char num[ELEMENT_BUF_SIZE];
	char did[DID_BUF_SIZE];

	int i;
	int snprintf_len;

	memset(did, 0, DID_BUF_SIZE);

	device_id[0] = IO_READ16(EFUSE_DID_BASE, REG_DEVICE_ID_0);
	device_id[1] = IO_READ16(EFUSE_DID_BASE, REG_DEVICE_ID_1);
	device_id[2] = IO_READ16(EFUSE_DID_BASE, REG_DEVICE_ID_2);
	device_id[3] = IO_READ16(EFUSE_DID_BASE, REG_DEVICE_ID_3);
	device_id[4] = IO_READ16(EFUSE_DID_BASE, REG_DEVICE_ID_4);
	device_id[5] = (IO_READ16(EFUSE_DID_BASE, REG_DEVICE_ID_5) & 0xFF);

	ptr = (unsigned char *)device_id;
	for(i = 10; i >= 0; i--) {
		memset(num, 0, sizeof(num));
		snprintf_len = snprintf(num, sizeof(num)-1, "%02X", ptr[i]);
		if (snprintf_len < 0 || (unsigned int)snprintf_len >= (sizeof(num)-1)) {
			printf("The array size is too small(%d), snprintf fail at num'%s'\n", (int)(sizeof(num)-1), num);
			return -1;
		}
		strncat(did, num, DID_BUF_SIZE-1);
	}
	strncpy(des, did, DID_BUF_SIZE-1);

	return 0;
}

int do_print_devid(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	char did[DID_BUF_SIZE];

	memset(did, 0, sizeof(did));
	if (get_devid(did) != 0){
		printf("{%s,%d} failed to get device id", __func__, __LINE__);
		return CMD_RET_FAILURE;
	}

	printf("%s\n",did);

	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(
	devid, 1, 0, do_print_devid,
	"Get Device Unique ID",
	NULL
);
