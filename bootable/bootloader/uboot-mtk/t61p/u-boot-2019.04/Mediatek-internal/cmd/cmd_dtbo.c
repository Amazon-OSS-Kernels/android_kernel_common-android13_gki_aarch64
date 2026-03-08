// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#include <common.h>
#include <command.h>
#include <vsprintf.h>
#include <debug_impl.h>
#include <dtbo.h>
#include <mtk_dtbo.h>
#include <utility.h>
#if (CONFIG_AB_SIDELOAD == 1)
#include <mtk_ab.h>
#endif

int do_dtb_overlay (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int ret = -1;
	unsigned long dtbo_img_addr,uboot_dtb_addr,kernel_dtb_addr;

	if(argc > 3 || argc < 2)
	{
		UBOOT_ERROR("Overlay argument number is wrong, please check it\n");
		return CMD_RET_USAGE;
	}

	init_dtbo_selection();
	dtbo_img_addr = simple_strtoul(env_get("dtboaddr"), NULL, 16);
	set_dtbo_image_address(dtbo_img_addr);
	if(strncmp(argv[1],"u", sizeof(char)) == 0 || strncmp(argv[1],"U", sizeof(char)) == 0)
	{
		get_uboot_dtb_addr(&uboot_dtb_addr);
		set_dtb_image_address(uboot_dtb_addr,true);
	}
	else if(strncmp(argv[1],"k", sizeof(char)) == 0 || strncmp(argv[1],"K", sizeof(char)) == 0)
	{
		kernel_dtb_addr = simple_strtoul(argv[2], NULL, 16);
		set_dtb_image_address(kernel_dtb_addr,false);
	}
	else
	{
		UBOOT_ERROR("DTB Overlay mode is not support.\n");
		return CMD_RET_USAGE;
	}

	ret = dtbo_selection_and_overlay();
	if(ret != 0)
	{
		UBOOT_ERROR("DTB Overlay execution failure, please check dtbo.img or device type.\n");
		return CMD_RET_FAILURE;
	}
	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(
	dtbo, CONFIG_SYS_MAXARGS, 1,    do_dtb_overlay,
	"dtbo - do device tree binary overlay\n",
	"command: dtbo [address]\n"
	"address : hex represent, DRAM address for dtbo.img\n"
);

int do_get_uboot_dtb_addr (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	unsigned long dtb_addr = 0;
	get_uboot_dtb_addr(&dtb_addr);
	printf("uboot dtb address:0x%lX\n",dtb_addr);

	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(
	dtbaddr, CONFIG_SYS_MAXARGS, 1,    do_get_uboot_dtb_addr,
	"dtbaddr - get u-boot device tree binary address\n",
	NULL
);

int do_dtbo_verify (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
/* If AVB is enabled, DTBO partition is protected by AVB through AOSP design */
/* So we don't sign and verify DTBO with Amazon signing */
#ifndef BOARD_AVB_ENABLE
	unsigned char *sign_data = NULL, *auth_data = NULL;
	unsigned long auth_size = 0;
	unsigned long dtbo_img_addr, dtbo_img_size = 0;
	int ret = 0;

	if(argc != 2)
	{
		UBOOT_ERROR("DTBO Verify argument number is wrong, please check it\n");
		return CMD_RET_USAGE;
	}

	dtbo_img_addr = simple_strtoul(argv[1], NULL, 16);
	ret = get_dtbo_img_size(dtbo_img_addr, &dtbo_img_size);
	if(ret != 0) {
		UBOOT_ERROR("get_dtbo_img_size fail.\n");
		return CMD_RET_FAILURE;
	}

	sign_data = (unsigned char *)(dtbo_img_addr + dtbo_img_size);
	auth_data = (unsigned char *)dtbo_img_addr;
	auth_size = dtbo_img_size;
	UBOOT_DEBUG("auth_data=%p\n", auth_data);
	UBOOT_DEBUG("auth_size=%lx\n", auth_size);
	UBOOT_DEBUG("sign_data=%p\n", sign_data);

	/* Verify dtbo.img */
	ret = verify_dtbo_img(auth_data, sign_data, auth_size);
	if(ret != 0) {
		UBOOT_ERROR("dtbo img verify fail\n");
		return CMD_RET_FAILURE;
	}
#endif
	UBOOT_DEBUG("dtbo img verify success\n");
	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(
	dtboverify, CONFIG_SYS_MAXARGS, 1, do_dtbo_verify,
	"dtboverify - verify dtbo.img in dtboaddr before uboot dtboverlay\n",
	"command: dtboverify [address]\n"
	"address : hex represent, DRAM address for dtbo.img\n"
);

#ifdef CONFIG_MULTIPLE_DTB_SELECTION
int do_dtb_cfg_operation (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int ret,len;
	char *ptr;
	char setting[128],partition[STRING_BUFFER_SIZE],cfg_file[64];

	if(argc < 2)
		return CMD_RET_FAILURE;

	memset(partition, 0, sizeof(partition));
	ptr = env_get("dtbo_cfg_part");
	if(ptr == NULL)
	{
		printf("cfg partition not select, use default /bootdata\n");
		if(strlen(DEFAULT_DTB_PART) < STRING_BUFFER_SIZE){
			strncpy(partition, DEFAULT_DTB_PART, strlen(DEFAULT_DTB_PART));
			partition[strlen(DEFAULT_DTB_PART)] = '\0';
		}else{
			strncpy(partition, DEFAULT_DTB_PART, STRING_BUFFER_SIZE-1);
			partition[STRING_BUFFER_SIZE-1] = '\0';
		}
	}
	else
	{
#if (CONFIG_AB_SIDELOAD == 1)
		if(sizeof(partition) <= (strlen(ptr) + strlen(PART_SUFFIX_A)))
			len = sizeof(partition)- strlen(PART_SUFFIX_A) - 1;
#else
		if(sizeof(partition) <= strlen(ptr))
			len = sizeof(partition)-1;
#endif
		else
			len = strlen(ptr);
		strncat(partition, ptr, len);
	}

	if(strncmp(argv[1],"get",3) == 0)
	{
		if(argc == 2)
		{
			ptr = env_get("dtbo_cfg_sel");
			if(ptr == NULL)
				printf("dtbo cfg is not selected, use default dtb/mt5896_h2v1.txt\n");
			else
				printf("%s\n",ptr);
		}
		else if(argc == 3)
		{
			memset(cfg_file, 0, sizeof(cfg_file));
			ptr = env_get("dtbo_cfg_sel");
			if(ptr == NULL)
			{
				printf("cfg file is not assignment, use default dtb/mt5896_h2v1.txt\n");
				strncat(cfg_file, "dtb/mt5896_h2v1.txt", sizeof(cfg_file)-1);
			}
			else
			{
				if(sizeof(cfg_file) <= strlen(ptr))
					len = sizeof(cfg_file)-1;
				else
					len = strlen(ptr);
				strncat(cfg_file, ptr, len);
			}

			memset(setting, 0, sizeof(setting));
			ret = get_dtbo_cfg_selection(partition, cfg_file, argv[2], setting);
			if(ret < 0)
				return CMD_RET_FAILURE;
			printf("%s\n",setting);
		}
		else
			return CMD_RET_USAGE;
	}
	else if(strncmp(argv[1],"select",6) == 0)
	{
		if(argc != 3)
			return CMD_RET_USAGE;

		ret = set_dtbo_cfg_selection(partition,argv[2]);
		if(ret < 0)
			return CMD_RET_FAILURE;
	}
	else if(strncmp(argv[1],"ls",2) == 0)
	{
		if(argc != 2)
			return CMD_RET_USAGE;

		ret = ls_dtbo_cfg(partition);
		if(ret < 0)
			return CMD_RET_FAILURE;
	}
	else
	{
		return CMD_RET_USAGE;
	}

	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(
	dtbcfg, CONFIG_SYS_MAXARGS, 1,    do_dtb_cfg_operation,
	"dtbcfg - do dtb cfg select/get/ls operation\n",
	"dtbcfg get\n"
	"	- Print current dtbo cfg file name.\n"
	"dtbcfg get <TYPE>\n"
	"	- <TYPE>: dtb classification.\n"
	"	- ex: dtbcfg get pcb\n"
	"dtbcfg select <PATH>\n"
	"	- <PATH>: dtbo cfg file path under tvconfig partition.\n"
	"	- ex: dtbcfg select dtb/xxx.cfg\n"
	"dtbcfg ls\n"
	"	- Print file under tvconfig partition dtb folder.\n"
);
#endif
