// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
*/

#include <command.h>
#include <common.h>
#include <vsprintf.h>
#include <debug_impl.h>
#include <romtblo_impl.h>
#include <utility.h>
#include <mtk_romtblo.h>

#define FLAG_LENGTH     (1)
#define COMMAND_OPTION_MAX_NUM     (2)

int do_romtbl_overlay(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    unsigned int address;
    unsigned long *romtbl;
    int ret;

    if(argc > 2 || argc == 1)
    {
        return CMD_RET_USAGE;
    }

    romtbl = (unsigned long *)CONFIG_ROM_TABLE_ADDRESS;
    init_rom_table((char *)romtbl);
    ret = check_rom_table_header();
    if( ret == ROMTBL_FAILED || ret == ROMTBL_NOT_INITED )
    {
        UBOOT_ERROR("ROM table header is not match. Check memory address 0x%X\n",CONFIG_ROM_TABLE_ADDRESS);
        return CMD_RET_FAILURE;
    }
    else if( ret == ROMTBL_NO_NEEDED )
    {
        UBOOT_DEBUG("No need to do rom table overlay\n");
        return CMD_RET_SUCCESS;
    }

    if(strncmp(argv[1], "K", FLAG_LENGTH) == 0 || strncmp(argv[1], "k", FLAG_LENGTH) == 0)
    {
        address = simple_strtoul(argv[2], NULL, 16);
        ret = romtbl_overlay(address);
    }
    else if(strncmp(argv[1], "U", FLAG_LENGTH) == 0 || strncmp(argv[1], "u", FLAG_LENGTH) == 0)
    {
        ret = romtbl_overlay_to_uboot_dtb();
    }
    else
    {
        UBOOT_ERROR("Command do_romtbl_overlay input argument wrong, please check command usage.\n");
        return CMD_RET_FAILURE;
    }

    if(ret != 0)
    {
        UBOOT_ERROR("Rom table overlay failure.\n");
        return CMD_RET_FAILURE;
    }
    return CMD_RET_SUCCESS;
}

U_BOOT_CMD(
    romtbl, CONFIG_SYS_MAXARGS, 1,    do_romtbl_overlay,
    "romtbl   - Do rom table overlay for u-boot dtb or kernel dtb\n",
    "command: romtbl <flag>\n"
    "<flag> : u/U for uboot dtb, k/K for kernel dtb\n"
);

int do_chip_id_add_to_bootargs(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    int ret;
    unsigned int chip = 0;
    char buffer[BUFFER_SIZE] = { 0 };
    int snprintf_len;
    unsigned long *romtbl;

    romtbl = (unsigned long *)CONFIG_ROM_TABLE_ADDRESS;
    init_rom_table((char *)romtbl);
    ret = check_rom_table_header();
    if( ret == ROMTBL_FAILED || ret == ROMTBL_NOT_INITED )
    {
        UBOOT_ERROR("ROM table header is not match. Check memory address 0x%X\n",CONFIG_ROM_TABLE_ADDRESS);
        return CMD_RET_FAILURE;
    }

    if(argc == 1)
    {
        chip = romtbl_get_chip_id_info_dec();
        if(chip != 0)
        {
            snprintf_len = snprintf(buffer, BUFFER_SIZE, "%s=mt%d", CHIP_HARDWARE_KEY, chip);
            if (snprintf_len >= BUFFER_SIZE)
            {
                UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", BUFFER_SIZE, buffer);
                return CMD_RET_SUCCESS;
            }
        }
        else
        {
            UBOOT_ERROR("Get CHIP ID failure, rom table does not exist\n");
        }

        ret = add_bootargs(CHIP_HARDWARE_KEY,buffer,0);
        if(ret == -1)
            UBOOT_ERROR("Add %s to bootargs failure\n",CHIP_HARDWARE_KEY);
    }
    else if(argc == COMMAND_OPTION_MAX_NUM)
    {
        if(strstr(argv[1], "c") != 0)
        {
            chip = romtbl_get_chip_id_info_dec();
            if(chip == 0)
                printf("Chip id get failure, Please check %x address\n",CONFIG_ROM_TABLE_ADDRESS);
            else
                printf("Chip id: MT%d\n",chip);
        }
        else
            printf("Command option is not support\n");
    }
    else
        printf("Command option is not support\n");

    return CMD_RET_SUCCESS;
}

U_BOOT_CMD(
    chipid, CONFIG_SYS_MAXARGS, 1,    do_chip_id_add_to_bootargs,
    "chipid   - Do add chip id to bootargs\n",
    "command: chipid\n"
);

int do_chip_revision_add_to_bootargs(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    int ret;
    unsigned int revision = 0;
    char buffer[BUFFER_SIZE] = { 0 };
    int snprintf_len;
    unsigned long *romtbl;

    romtbl = (unsigned long *)CONFIG_ROM_TABLE_ADDRESS;
    init_rom_table((char *)romtbl);
    ret = check_rom_table_header();
    if( ret == ROMTBL_FAILED || ret == ROMTBL_NOT_INITED )
    {
        UBOOT_ERROR("ROM table header is not match. Check memory address 0x%X\n",CONFIG_ROM_TABLE_ADDRESS);
        return CMD_RET_FAILURE;
    }

    if(argc == 1)
    {
        revision = romtbl_get_chip_revision_info();
        if(revision != 0)
        {
            snprintf_len = snprintf(buffer, BUFFER_SIZE, "%s=%s%d", CHIP_HARDWAREREV_KEY, CHIP_REVISION_CHAR, revision);
            if (snprintf_len >= BUFFER_SIZE)
            {
                UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", BUFFER_SIZE, buffer);
                return CMD_RET_SUCCESS;
            }
        }
        else
        {
            UBOOT_ERROR("Get CHIP REVISION failure, rom table does not exist\n");
        }

        ret = add_bootargs(CHIP_HARDWAREREV_KEY,buffer,0);
        if(ret == -1)
            UBOOT_ERROR("Add %s to bootargs failure\n",CHIP_HARDWAREREV_KEY);
    }
    else if(argc == COMMAND_OPTION_MAX_NUM)
    {
        if(strstr(argv[1], "r") != 0)
        {
            revision = romtbl_get_chip_revision_info();
            if(revision == 0)
                printf("Chip revision get failure, Please check %x address\n",CONFIG_ROM_TABLE_ADDRESS);
            else
                printf("Chip revision: E%d\n",revision);
        }
        else
            printf("Command option is not support\n");
    }
    else
        printf("Command option is not support\n");

    return CMD_RET_SUCCESS;
}

U_BOOT_CMD(
    revisionid, CONFIG_SYS_MAXARGS, 1,    do_chip_revision_add_to_bootargs,
    "revisionid   - Do add revision id to bootargs\n",
    "command: revisionid\n"
);
