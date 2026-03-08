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

#include <common.h>
#include <command.h>
#include <exports.h>
#include <environment.h>
#include <MsTypes.h>
#include <system_impl.h>
#include <debug_impl.h>
#include <usb_upgrade.h>
#include <upgrade_osd.h>
#include <upgrade_utility.h>
#include <MsTypes.h>
#include <utility.h>
#include <iniutility.h>
#include <oad_upgrade.h>
#include <secure/secure_upgrade.h>
#include <wdt.h>
#include <dm.h>
#include <vsprintf.h>
#include <string.h>
#include <linux/ctype.h>

#if (CONFIG_USB_UPGRADE == 1)
#include <usb.h>
#endif

#if (CONFIG_USB_UPGRADE == 1)
#if (CONFIG_MMC == 1)
#include <partition.h>
#include <mmc.h>
#endif
#endif

#include <amzn_tv_secure_boot.h>

#define MAX_DEVICE 3
#define MAX_PARTITION 5
#define DEFAULT_SCRIPT_SIZE_FOR_UPGRADE_IMAGE 0x10000
#define MAX_DONTWRITE_COUNT 20
#define MAX_FORCEWRIE_COUNT MAX_DONTWRITE_COUNT

#define FULL_AP_BIN_PATH        "/MTKUpgrade_full.bin" // always fo full upgrade

#define SCRIPT_FILE_COMMENT         '#'                 // symbol for comment
#define SCRIPT_FILE_END             '%'                 // symbol for file end
#define SCRIPT_FILE_ZERO            0
#define IS_COMMENT(x)       (SCRIPT_FILE_COMMENT == (x))
#define IS_FILE_END(x)      (SCRIPT_FILE_END == (x))
#define IS_ZERO_END(x)      (SCRIPT_FILE_ZERO == (x))
#define IS_LINE_END(x)      ('\r' == (x)|| '\n' == (x))
#define IS_IGNORED_CHAR(x)  ('\r' == (x)|| '\n' == (x) || '\t' == (x) || ' ' == (x))


st_image_offset_info image_offset_info;
struct upgrade_info gupgradeinfo={0};


static char ginterface[STORAGE_DEVICE_BUF_SIZE] = {0};
static int gdevice_number = 0;
static int gpartition_number = 0;
static en_update_mode gupdate_mode = EN_UPDATE_MODE_NONE;
static unsigned int gscript_size_of_upgrade_image = 0;
static unsigned char gfile_size_ready = FALSE;
static unsigned int gfile_size = 0;


extern char console_buffer[CONFIG_SYS_CBSIZE + 1];	/* console I/O buffer	*/
extern int sscanf(const char * buf, const char * fmt, ...);
extern int readline (const char *const prompt);
extern int parse_line (char *line, char *argv[]);


void jump_to_console(void)
{

    static char lastcommand[CONFIG_SYS_CBSIZE +1] = { 0, };//Fix Coverity CID:148928
    int len;
    int rc = 1;
    int aflag;

    // for case : close console log
//    open_console_log();                             // uboot-2011 in uboot/include/common.h
    printf("jump_to_console start!!\n");
    for (;;)
    {
        len = readline (CONFIG_SYS_PROMPT);

        aflag = 0;   /* assume no special flags for now */
        if (len > 0)
            strncpy (lastcommand, console_buffer, CONFIG_SYS_CBSIZE);
        else if (len == 0)
            aflag |= CMD_FLAG_REPEAT;

        if (len == -1)
            puts ("<INTERRUPT>\n");
        else if (!chk_cmd_lockdown(lastcommand)) {
            puts ("command not supported in lockdown\n");
            rc = 0;
        } else
            rc = run_command_repeatable(lastcommand, aflag);

        if (rc <= 0) {
            /* invalid command or not repeatable, forget it */
            lastcommand[0] = 0;
        }
    }
     printf("jump_to_console end!!\n");
}


char *get_script_next_line(char **line_buf_ptr)
{
    UBOOT_TRACE("IN\n");
    char *line_buf;
    char *next_line;
    int i;

    line_buf = (*line_buf_ptr);

    // strip '\r', '\n' and comment
    while (1)
    {
        // strip '\r' & '\n' & ' ' & '\t'
        if (IS_IGNORED_CHAR(line_buf[0]))
        {
			line_buf++;
        }
        // strip comment
        else if (IS_COMMENT(line_buf[0]))
        {
            for (i = 0; !IS_LINE_END(line_buf[0]) && i <= CONFIG_SYS_CBSIZE; i++)
            {
                line_buf++;
            }

            if (i > CONFIG_SYS_CBSIZE)
            {
                line_buf[0] = SCRIPT_FILE_END;

                UBOOT_ERROR("Error: the max size of one line is %d!!!\n", CONFIG_SYS_CBSIZE); // <-@@@

                break;
            }
        }
        else
        {
            break;
        }
    }

    // get next line
    if (IS_FILE_END(line_buf[0]) || IS_ZERO_END(line_buf[0]))
    {
        next_line = NULL;
    }
    else
    {
		next_line = line_buf;

        for (i = 0; !IS_LINE_END(line_buf[0]) && i <= CONFIG_SYS_CBSIZE; i++)
        {
            line_buf++;
        }

        if (i > CONFIG_SYS_CBSIZE)
        {
			next_line = NULL;
            UBOOT_ERROR("Error: the max size of one line is %d!!!\n", CONFIG_SYS_CBSIZE); // <-@@@
        }
        else
        {
            line_buf[0] = '\0';
            *line_buf_ptr = line_buf + 1;
        }
    }
    UBOOT_TRACE("OK\n");

    return next_line;
}

int runscript_linebyline(char *script_buf)
{
    char *script = NULL;
    char *next_line = NULL;
    int ret = 0;

    UBOOT_TRACE("IN\n");
    script = script_buf;
    if(script == NULL)
    {
        UBOOT_ERROR("Address is a null pointer\n");
        return -1;
    }
    else
    {
        UBOOT_INFO("\n########## Start to execute script ##########\n");
        while((next_line = get_script_next_line(&script)) != NULL)
        {
            UBOOT_INFO("\n>> %s \n", next_line);
            if(check_skip_cmd(next_line) == FALSE)
            {
                ret = run_command(next_line, 0);
                if(ret != 0)
                {
                    UBOOT_ERROR("Error: '%s' fail\n", next_line);
                    return -1;
                }
            }
        }
        UBOOT_INFO("\n########## Execute script succeed ##########\n\n");
    }
    UBOOT_TRACE("OK\n");

    return 0;
}

void reset_system(void)
{
    // clean up the upgrade releated env
    env_set(ENV_UPGRADE_MODE, NULL);
    env_set(ENV_UPGRADE_STATUS, NULL);
    env_save();
    run_command("reset", 0);
}


unsigned char check_skip_cmd(char *next_line)
{
    UBOOT_TRACE("IN\n");
    char *pLine = NULL;

    if (next_line == NULL)
    {
        UBOOT_ERROR("The input parameter 'next_line' is a null pointer\n");
        return FALSE;
    }

    //reset command will be excute in the end of upgrade
    if (strncmp(next_line,"reset",strlen("reset")) == 0)
    {
        UBOOT_INFO("Reset command is skipped\n");
        UBOOT_TRACE("OK\n");
        return TRUE;
    }

    //This part is for filter command.
    pLine=strstr(next_line, "#");
    if (pLine != NULL){
        *pLine='\0';
    }
    UBOOT_TRACE("OK\n");
    return FALSE;
}


int check_file_exist(const char *interface, char *upgrade_file)
{
    UBOOT_TRACE("IN\n");

    if (check_usb_file_partition(interface, NULL, NULL, upgrade_file) == 0)     // read 1 byte in file upgrade_file to check upgrade_file existing
    {
        UBOOT_DEBUG("Find upgrade file '%s' in %s\n", upgrade_file, interface);
        return 0;
    }

    UBOOT_ERROR("Can NOT find '%s' in %s!!\n", upgrade_file, interface);
    return -1;
}

unsigned long get_storage_size(void)
{
#if defined(CONFIG_MMC)
    struct mmc *mmc;

    mmc = find_mmc_device(0);
    if (!mmc) {
        printf("no mmc device at slot 0\n");
        return 0;
    }

    if (!mmc_getcd(mmc))
        return 0;

    if (mmc_init(mmc))
        return 0;

    return (unsigned long)mmc->capacity;
#elif defined(CONFIG_UFS)
    struct blk_desc *dev_desc = NULL;

    dev_desc = blk_get_dev("ufs", 0);
    if (dev_desc == NULL) {
        printf("Block device %s %d not supported\n", "ufs", 0);
        return 0;
    }

    return (unsigned long)(dev_desc->lba * dev_desc->blksz);
#endif
}

int check_usb_upgrade_file_available(const char *interface, char *upgrade_file)
{
    UBOOT_TRACE("IN\n");
    int partition = 0;
    int device = 0;
    char device_partition[10] = {0};
    int snprintf_len;
    unsigned int upgrade_file_size = 0;

    if (check_usb_file_partition(interface, &device, &partition, upgrade_file) != 0)     // read 1 byte in upgrade_file to check upgrade_file exiting
    {
        UBOOT_ERROR("fail : no file in %s\n", interface);
        return -1;
    }

    gdevice_number = device;           // set USB device number which store the upgrade_file to global variable
    gpartition_number = partition;     // set USB partition number which store the upgrade_file to global variable

    memset(device_partition, 0, 10);
    snprintf_len = snprintf(device_partition, sizeof(device_partition), "%d:%d", gdevice_number, gpartition_number);
    if (snprintf_len >= sizeof(device_partition))
    {
        UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", (int)(sizeof(device_partition)), device_partition);
        return -1;
    }
    upgrade_file_size = get_file_size_for_upgrade(interface, device_partition, upgrade_file);     // get size of the upgrade file
    if (upgrade_file_size == 0 || (unsigned long)upgrade_file_size >= (unsigned long)get_storage_size())
    {
        UBOOT_ERROR("file size of upgrade image not valid\n");
        return -1;
    }

    UBOOT_TRACE("OK\n");
    return 0;
}


int check_usb_file_partition(const char *interface, int *device, int *partition, char *upgrade_file)
{
    UBOOT_TRACE("IN\n");
    char buffer[CMD_BUF] = "\0";
    int snprintf_len;
    int g_device_e = MAX_DEVICE;
    int g_partition_e = MAX_PARTITION;

    for(gdevice_number = 0 ; gdevice_number < g_device_e ;gdevice_number++)
    {
        for(gpartition_number = 0 ; gpartition_number < g_partition_e ; gpartition_number++)
        {
            snprintf_len = snprintf(buffer, CMD_BUF, "fatload %s %d:%d 0x%x %s 1", interface, gdevice_number, gpartition_number, UPGRADE_CHECK_BIN_BUFFER_ADDR, upgrade_file);
            if (snprintf_len >= CMD_BUF)
            {
                UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", CMD_BUF, buffer);
                return -1;
            }
            UBOOT_DEBUG("cmd: %s \n",buffer);
            if (run_command(buffer, 0) == 0)
            {
               UBOOT_DEBUG("Find '%s' on %s device %d partition %d\n", upgrade_file, interface, gdevice_number, gpartition_number);
               if ((device != NULL) && (partition != NULL))
               {
                   *device=gdevice_number;
                   *partition=gpartition_number;
               }
               UBOOT_TRACE("OK\n");
               return 0;
            }
        }
    }
    UBOOT_TRACE("OK\n");
    return -1;
}

#define EXT4_COMMAND_BUFFER 0x30

int check_ext4_upgrade_file_available(const char *interface)
{

    UBOOT_TRACE("IN\n");
    char fxt4_command[EXT4_COMMAND_BUFFER] = {0};
    int snprintf_len;
    unsigned long upgrade_file_size = 0;
    memset(fxt4_command, 0, EXT4_COMMAND_BUFFER);
    snprintf_len = snprintf(fxt4_command, sizeof(fxt4_command), "ext4size %s %s %s",
        interface, gupgradeinfo.upgrade_storage_info, gupgradeinfo.upgrade_filename);

    if (snprintf_len >= sizeof(fxt4_command))
    {
        UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", (int)(sizeof(fxt4_command)), fxt4_command);
        return -1;
    }
    UBOOT_DEBUG(" command:%s \n",fxt4_command);
    if(0!=run_command(fxt4_command,0))// get size of the upgrade file
    {
        UBOOT_ERROR("run command fail :%s\n",fxt4_command);
        return -1;
    }

    upgrade_file_size =(unsigned long) env_get_ulong("filesize",16,0);     // get size of the upgrade file to uboot's env. variable

    if (upgrade_file_size > 0)
    {
        printf("Upgrade infor=%s partition=%s file name=%s size=0x%lx \n",
            gupgradeinfo.upgrade_storage_info,gupgradeinfo.upgrade_partition, gupgradeinfo.upgrade_filename,upgrade_file_size);
    }
    else
    {
        UBOOT_ERROR("Can not get file size in environment\n");
        UBOOT_ERROR("file size of upgrade image not valid\n");
        return -1;
    }

    UBOOT_TRACE("OK\n");
    return 0;
}

int set_scrit_size_of_upgrade_image(unsigned int size)
{
    UBOOT_TRACE("IN\n");
    gscript_size_of_upgrade_image=size;
    UBOOT_TRACE("OK\n");
    return 0;
}

int set_upgrade_file_name(char *str)
{
    UBOOT_TRACE("IN\n");
    char *buffer = NULL;
    int snprintf_len;
    char *pname = NULL;
    int ret = 0;

    if (str == NULL)
    {
       UBOOT_ERROR("The input parameter 'str' is a null pointer\n");
       return -1;
    }

    buffer=malloc(CMD_BUF);
    if (buffer == NULL)
    {
        UBOOT_ERROR("malloc fail \n");
        return -1;
    }
    memset(buffer, 0, CMD_BUF);

    pname=str;

    snprintf_len = snprintf(buffer,CMD_BUF, "setenv %s %s", ENV_UPGRADE_IMAGE, pname);
    if (snprintf_len >= CMD_BUF)
    {
        UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", CMD_BUF, buffer);
        free(buffer);
        return -1;
    }

    UBOOT_DEBUG("cmd: %s \n", buffer);
    ret = run_command(buffer, 0);
    if (ret == -1)
    {
        free(buffer);
        UBOOT_ERROR("setenv ENV_UPGRADE_IMAGE fail\n");
    }
    else
    {
        free(buffer);
        UBOOT_TRACE("OK\n");
    }
    return ret;
}


int set_upgrade_mode(en_update_mode enMode)
{
    UBOOT_TRACE("IN\n");
    gupdate_mode=enMode;
    UBOOT_TRACE("OK\n");
    return 0;
}


int delete_upgrade_file_name(void)
{
    UBOOT_TRACE("IN\n");
    char *buffer = NULL;
    int snprintf_len;
    int ret = 0;

    buffer=malloc(CMD_BUF);
    if (buffer == NULL)
    {
        UBOOT_ERROR("malloc fail \n");
        return -1;
    }
    memset(buffer, 0, CMD_BUF);

    snprintf_len = snprintf(buffer,CMD_BUF, "setenv %s ", ENV_UPGRADE_IMAGE);
    if (snprintf_len >= CMD_BUF)
    {
        UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", CMD_BUF, buffer);
        free(buffer);
        return -1;
    }
    UBOOT_DEBUG("cmd: %s \n", buffer);
    ret = run_command(buffer, 0);
    if (ret == -1)
    {
        free(buffer);
        UBOOT_ERROR("delete ENV_UPGRADE_IMAGE fail\n");
    }
    else
    {
        free(buffer);
        UBOOT_TRACE("OK\n");
    }
    return ret;

}


unsigned int get_scrit_size_of_upgrade_image(void)
{
    UBOOT_TRACE("IN\n");
    if (gscript_size_of_upgrade_image == 0)
    {
        UBOOT_TRACE("OK\n");
        return DEFAULT_SCRIPT_SIZE_FOR_UPGRADE_IMAGE;
    }
    else
    {
        UBOOT_TRACE("OK\n");
        return gscript_size_of_upgrade_image;
    }
}


unsigned int get_file_size_for_upgrade(const char *interface, const char *device, char *file)
{
    UBOOT_TRACE("IN\n");
    char *buffer = NULL;
    int snprintf_len;
    int ret = 0;

    if (gfile_size_ready == TRUE)
    {
        UBOOT_TRACE("OK\n");
        return gfile_size;
    }

    buffer=(char *)malloc(CMD_BUF);
    if (buffer == NULL)
    {
       UBOOT_ERROR("Error: out of memory\n");
       return 0;
    }
    //Get total file size
    memset(buffer, 0, CMD_BUF);
    if (get_upgrade_mode()==EN_UPDATE_MODE_OAD_WITH_SEG_DECRYPTED || get_upgrade_mode()==EN_UPDATE_MODE_OAD)
    {
        snprintf_len = snprintf(buffer, CMD_BUF, "ext4size %s %s %s", interface, device, file);    // set size of the upgrade file to uboot's env. variable
        if (snprintf_len >= CMD_BUF)
        {
            UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", CMD_BUF, buffer);
            free(buffer);
            return 0;
        }
    }
    else if (get_upgrade_mode()==EN_UPDATE_MODE_USB || get_upgrade_mode()==EN_UPDATE_MODE_USB_WITH_SEG_DECRYPTED)
    {
        snprintf_len = snprintf(buffer, CMD_BUF, "fatsize %s %s %s", interface, device, file);     // set size of the upgrade file to uboot's env. variable
        if (snprintf_len >= CMD_BUF)
        {
            UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", CMD_BUF, buffer);
            free(buffer);
            return 0;
        }
    }
    else
    {
        UBOOT_ERROR("Wrong Upgrade Mode!!\n");
        free(buffer);
        return 0;
    }
    UBOOT_DEBUG("cmd:%s\n",buffer);
    ret = run_command(buffer, 0);
    if (ret < 0)
    {
        free(buffer);
        return 0;
    }
    char *p_size = env_get("filesize");     // get size of the upgrade file to uboot's env. variable
    if (p_size != NULL)
    {
        gfile_size = (int)simple_strtol(p_size, NULL, 16);
        printf("Upgrade file name=%s\n", file);
        printf("Upgrade file size=%u\n", gfile_size);
    }
    else
        UBOOT_ERROR("Can not get file size in environment\n");

    free(buffer);
    UBOOT_TRACE("OK\n");
    gfile_size_ready = TRUE;
    return gfile_size;
}


en_update_mode get_upgrade_mode(void)
{
    UBOOT_TRACE("IN\n");
    UBOOT_TRACE("OK\n");
    return gupdate_mode;
}


unsigned int check_upgrade_script_cmd_count(char *buffer)
{
    UBOOT_TRACE("IN\n");
    unsigned int script_file_size = 0;
    unsigned int amount = 0;
    char *temp_buf = NULL;

    script_file_size=get_scrit_size_of_upgrade_image();
    if (script_file_size == 0)
    {
        UBOOT_ERROR("The size of script file is zero \n");
        return 0;
    }

    temp_buf=malloc(script_file_size);
    if (temp_buf == NULL)
    {
        UBOOT_ERROR("malloc fail \n");
        return 0;
    }
    memcpy(temp_buf, buffer, script_file_size);
    amount=calculate_script_cmd_count(temp_buf);     // calculate the total amount of commands in the script file
    free(temp_buf);

    UBOOT_TRACE("OK\n");
    return amount;
}


unsigned int calculate_script_cmd_count(char *script_buf)
{
    char *_argv[CONFIG_SYS_MAXARGS + 1];
    char *next_line = NULL;
    unsigned int line = 0;

    memset(_argv, 0, sizeof(char*)*(CONFIG_SYS_MAXARGS + 1));

	next_line = get_script_next_line(&script_buf);
    UBOOT_DEBUG("The address of script_buf is 0x%p\n", script_buf);
    while (next_line != NULL)
    {
		if ((strlen(next_line) > CONFIG_SYS_CBSIZE))
        {
            UBOOT_DEBUG("The size of %uth line is over %d bytes. \n", line, CONFIG_SYS_CBSIZE);
            return line;
        }
        else
        {      /* parse command to argv */
			if (parse_line (next_line, _argv) == 0)
            {
                  UBOOT_DEBUG("%uth line doesn't find any argument \n", line);
                  return line;
            }
            else
            {   /* Look up command in command table */
				if (find_cmd(_argv[0]) == NULL)
                {
                      UBOOT_DEBUG("%uth line doesn't find any cmd, _argv[0]=%s \n", line,_argv[0]);
                      return line;
                }
            }
        }
        line += 1;
        next_line = get_script_next_line(&script_buf);
    }
    return line;
}

static unsigned char * read_firmware_image_header(const char *interface, char *upgrade_file){
    UBOOT_TRACE("IN\n");
    unsigned char *ptr_header = (unsigned char*)UPGRADE_HEADER_BUFFER_ADDR;
    char *buffer = NULL;
    int snprintf_len =0;

    buffer=malloc(CMD_BUF);
    if (buffer == NULL)
    {
        UBOOT_ERROR("malloc fail\n");
        return NULL;
    }
    memset(buffer, 0, CMD_BUF);

    UBOOT_DEBUG("Before use fatload to read %s header info. to DRAM(0x%x)\n", upgrade_file, UPGRADE_HEADER_BUFFER_ADDR);
    UBOOT_DUMP(UPGRADE_HEADER_BUFFER_ADDR, 0x50);

    if (PACK_HEADER_SIZE > gfile_size)
    {
        UBOOT_ERROR("PACK_HEADER_SIZE is bigger than upgrade_file size\n");
        free(buffer);
        return NULL;
    }

    //load header in upgrade_file to DRAM(UPGRADE_HEADER_BUFFER_ADDR),start from offset 0 with size PACK_HEADER_SIZE
    if (strncmp(interface, "usb", strlen("usb")) == 0)
        snprintf_len = snprintf(buffer, CMD_BUF, "fatload %s %d:%d 0x%p %s 0x%x", interface, gdevice_number, gpartition_number, ptr_header, upgrade_file, PACK_HEADER_SIZE);
    else if ((strncmp(interface, "mmc", strlen("mmc")) == 0) || (strncmp(interface, "ufs", strlen("ufs")) == 0))
        snprintf_len = snprintf(buffer, CMD_BUF, "ext4load %s %s 0x%p %s 0x%x", interface, gupgradeinfo.upgrade_storage_info, ptr_header, upgrade_file, PACK_HEADER_SIZE);
    else
    {
        UBOOT_ERROR("Not support interface type '%s' (only support usb, mmc, ufs case)\n", interface);
        free(buffer);
        return NULL;
    }

    if (snprintf_len >= CMD_BUF)
    {
        UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", CMD_BUF, buffer);
        free(buffer);
        return NULL;
    }
    UBOOT_DEBUG("cmd=%s\n", buffer);
    if (run_command(buffer, 0) == -1)
    {
        UBOOT_ERROR("Error: cmd: %s\n", buffer);
        free(buffer);
        return NULL;
    }
    free(buffer);
    UBOOT_DEBUG("After use fatload to read %s header info. to DRAM(0x%x)\n", upgrade_file, UPGRADE_HEADER_BUFFER_ADDR);
    UBOOT_DUMP(UPGRADE_HEADER_BUFFER_ADDR, 0x50);
    UBOOT_TRACE("OK\n");
    return ptr_header;
}

static int isNumber(char s[])
{
    UBOOT_TRACE("IN\n");
    int i;
    for (i = 0; s[i]!= '\0'; i++){
        if (! isdigit(s[i])) {
            return 0;
        }
    }
    UBOOT_TRACE("OK\n");
    return 1;
}

static void print_version_info(st_image_version_info *version_info){
    UBOOT_DEBUG("full_version = %s\n", version_info->full_version);
    UBOOT_DEBUG("prefix = %s\n", version_info->prefix);
    UBOOT_DEBUG("major_num = %d\n", version_info->major_num);
    UBOOT_DEBUG("minor_num = %d\n", version_info->minor_num);
    UBOOT_DEBUG("subminor_num = %d\n\n", version_info->subminor_num);
}

static int parse_version_format(st_image_version_info *version_info, char version_buffer[HEADER_VERSION_USED_SIZE]){
    const int max_version_number = 999, num_base = 10;
    char *default_version = "V_0.0.0";
    char key_delim[HEADER_VERSION_DELIM_SIZE] = "V_";
    char version_num[HEADER_VERSION_NUM_SIZE] = {0};
    char *major_num, *minor_num, *subminor_num;
    int delim_start_idx = 0, i, ret = 0;

    if (version_buffer[0] == 0 && memcmp(version_buffer, version_buffer+1, HEADER_VERSION_USED_SIZE-1) == 0){
        /* Some pkgs without version info, such as the pkgs built by the server in the past, still need to be upgrade successfully.
        ** Hence, set the default version numbers for these pkgs.*/
        UBOOT_DEBUG("Without version info case. Use default value.\n");
        strncpy(version_buffer, default_version, HEADER_VERSION_USED_SIZE-1);
    }

    // 1. parse prefix and versioin number
    for(i = 0; i < HEADER_VERSION_USED_SIZE-1; i++) {
        if (version_buffer[i] == key_delim[0] && version_buffer[i+1] == key_delim[1]){
            delim_start_idx = i;
            break;
        }
    }
    if(delim_start_idx >= HEADER_VERSION_PREFIX_SIZE){
        UBOOT_ERROR("Prefix > 16 characters\n");
        ret = -1;
        goto end;
    }

    strncpy(version_info->prefix, version_buffer, delim_start_idx);
    strncpy(version_num, version_buffer + delim_start_idx + sizeof(key_delim), HEADER_VERSION_NUM_SIZE-1);
    version_num[HEADER_VERSION_NUM_SIZE-1] = '\0';

    // 2. Deconstruct version number
    major_num = strtok(version_num, ".");
    if (major_num == NULL) {
        UBOOT_ERROR("Major number is NULL.\n");
        ret = -1;
        goto end;
    }
    minor_num = strtok(NULL, ".");
    if (minor_num == NULL) {
        UBOOT_ERROR("Minor number is NULL.\n");
        ret = -1;
        goto end;
    }
    subminor_num = strtok(NULL, ".");
    if (subminor_num == NULL) {
        UBOOT_ERROR("Subminor number is NULL.\n");
        ret = -1;
        goto end;
    }

    if (!isNumber(major_num) || !isNumber(minor_num) || !isNumber(subminor_num)){
        UBOOT_ERROR("Versioin number is not a number.\n");
        UBOOT_DEBUG("major_num = %s\n", major_num);
        UBOOT_DEBUG("minor_num = %s\n", minor_num);
        UBOOT_DEBUG("subminor_num = %s\n", subminor_num);
        ret = -1;
        goto end;
    }

    version_info->major_num = simple_strtoul(major_num, NULL, num_base);
    version_info->minor_num = simple_strtoul(minor_num, NULL, num_base);
    version_info->subminor_num = simple_strtoul(subminor_num, NULL, num_base);
    if (version_info->major_num < 0 || version_info->major_num > max_version_number){
        UBOOT_ERROR("Versioin number < 0 or > %d.\n", max_version_number);
        ret = -1;
        goto end;
    }
    if (version_info->minor_num < 0 || version_info->minor_num > max_version_number){
        UBOOT_ERROR("Versioin number < 0 or > %d.\n", max_version_number);
        ret = -1;
        goto end;
    }
    if (version_info->subminor_num < 0 || version_info->subminor_num > max_version_number){
        UBOOT_ERROR("Versioin number < 0 or > %d.\n", max_version_number);
        ret = -1;
        goto end;
    }

end:
    if (ret != 0)
        UBOOT_ERROR("Version_buffer = %s\n", version_buffer);
    else
        strncpy(version_info->full_version, version_buffer, HEADER_VERSION_USED_SIZE-1);

    return ret;
}

static int anti_rollback_checker(st_image_version_info *cur_version_info, st_image_version_info *new_version_info){
    if (strncmp(cur_version_info->prefix, new_version_info->prefix, HEADER_VERSION_PREFIX_SIZE) == 0) {
        if (cur_version_info->major_num < new_version_info->major_num) return IMAGE_VERSIOIN_CHECKER_STATE_UPDATE;
        if (cur_version_info->major_num == new_version_info->major_num) {
            if (cur_version_info->minor_num < new_version_info->minor_num) return IMAGE_VERSIOIN_CHECKER_STATE_UPDATE;
            if (cur_version_info->minor_num == new_version_info->minor_num) {
                if (cur_version_info->subminor_num < new_version_info->subminor_num) return IMAGE_VERSIOIN_CHECKER_STATE_UPDATE;
                if (cur_version_info->subminor_num == new_version_info->subminor_num) return IMAGE_VERSIOIN_CHECKER_STATE_SAME;
            }
        }
        UBOOT_ERROR("This upgrade_image.pkg version is older than current version.\n");
    } else {
        UBOOT_ERROR("Version prefix is not equal\n");
        UBOOT_ERROR("[New prefix]: %s\n", new_version_info->prefix);
        UBOOT_ERROR("[Current prefix]: %s\n", cur_version_info->prefix);
    }
    return IMAGE_VERSIOIN_CHECKER_STATE_FAIL;
}

static int check_firmware_image_version_info(unsigned char *header_buf, st_image_version_info *new_image_version_info){
    UBOOT_TRACE("IN\n");
    UBOOT_INFO("Check pkg (upgrade image) version...\n");
    static char new_image_version_buffer[HEADER_VERSION_USED_SIZE] = {0};
    static char cur_image_version_buffer[HEADER_VERSION_USED_SIZE] = {0};
    st_image_version_info cur_image_version_info = {0};
    char *version_val_in_uboot;
    image_version_checker_state ret = IMAGE_VERSIOIN_CHECKER_STATE_UPDATE;

    /* 1. Read new version info from firmware image */
    strncpy(new_image_version_buffer, (char *)(header_buf+SIGNATURE_SIZE), HEADER_VERSION_USED_SIZE-1);

    /* 2. parse new version format */
    if (parse_version_format(new_image_version_info, new_image_version_buffer) != 0){
        UBOOT_ERROR("Parse new PKG versioin format fail\n");
        return IMAGE_VERSIOIN_CHECKER_STATE_FAIL;
    };
    UBOOT_DEBUG("new PKG version info:\n");
    print_version_info(new_image_version_info);

    /* 3. Read cur version info from uboot env */
    version_val_in_uboot = env_get(ENV_UPGRADE_PKG_VERSION);
    if (version_val_in_uboot == NULL){
        UBOOT_DEBUG("First upgrade. Current PKG version is not exist.\n");
        /* continue upgrading process */
    } else {
        if (strlen(version_val_in_uboot) >= HEADER_VERSION_USED_SIZE){
            UBOOT_ERROR("Size of current PKG versioin fail.\n");
            return IMAGE_VERSIOIN_CHECKER_STATE_FAIL;
        }

        /* 4. parse cur version format */
        strncpy(cur_image_version_buffer, version_val_in_uboot, HEADER_VERSION_USED_SIZE-1);
        if (parse_version_format(&cur_image_version_info, cur_image_version_buffer) != 0){
            UBOOT_ERROR("Parse current PKG version format fail.\n");
            return IMAGE_VERSIOIN_CHECKER_STATE_FAIL;
        };
        UBOOT_DEBUG("Current PKG version info:\n");
        print_version_info(&cur_image_version_info);

        /* 5. Anti-rollback checker */
        ret = anti_rollback_checker(&cur_image_version_info, new_image_version_info);
    }

    if (ret == IMAGE_VERSIOIN_CHECKER_STATE_FAIL) {
        UBOOT_ERROR("PKG Anti-rollback check failed.\n");
    } else {
        UBOOT_INFO("PKG version is valid\n");
        UBOOT_INFO("PKG version: %s\n", new_image_version_info->full_version);
    }
    UBOOT_TRACE("OK\n");
    return ret;
}

static int update_firmware_image_version_info(st_image_version_info *version_info){
    UBOOT_TRACE("IN\n");
    UBOOT_INFO("Update pkg version info...\n");
    if (env_set(ENV_UPGRADE_PKG_VERSION, version_info->full_version) != 0){
        UBOOT_ERROR("Set new pkg version failed.\n");
        return -1;
    }
    if (env_save() != 0){
        UBOOT_ERROR("Save new pkg version failed.\n");
        return -1;
    }
    UBOOT_TRACE("OK\n");
    return 0;
}

int read_firmware_image_offset_info(unsigned char *ptr_header, char *upgrade_file)
{
    UBOOT_TRACE("IN\n");
    int sscanf_num = 0;
    unsigned int u32ree_image_offset_start = 0;
    unsigned int u32ree_image_offset_len = 0;

    // shift offset to image offset info part
    ptr_header += SIGNATURE_SIZE;
    ptr_header += HEADER_VERSION_SIZE;
    UBOOT_DEBUG("SIGNATURE_SIZE is 0x%x, HEADER_VERSION_SIZE is 0x%x\n", SIGNATURE_SIZE,HEADER_VERSION_SIZE);
    UBOOT_DEBUG("%s REE offset info. at DRAM(0x%p)\n", upgrade_file, ptr_header);
    UBOOT_DUMP(ptr_header, 0x200);

    // get REE offset start address and length in upgrade_file
    sscanf_num = sscanf((const char *)ptr_header, "# REE_OFFSET_START = 0x%x #\n"\
                      "# REE_OFFSET_LEN = 0x%x #\n", \
           &u32ree_image_offset_start, &u32ree_image_offset_len);

    if (sscanf_num != 2)
    {
        UBOOT_ERROR("Can NOT scan variabe from firmware offset info (sscanf_num=%d)\n", sscanf_num);
        UBOOT_ERROR("Set REE_OFFSET_START & REE_OFFSET_LEN = 0\n");
        u32ree_image_offset_start = 0;
        u32ree_image_offset_len = 0;
    }

    // check REE offset info overflow
    if (u32ree_image_offset_start > (u32ree_image_offset_start + u32ree_image_offset_len))
    {
        UBOOT_ERROR("Check REE offset info overflow\n");
        return -1;
    }
    // check REE offset info is invalid
    if ((u32ree_image_offset_start + u32ree_image_offset_len) > gfile_size)
    {
        UBOOT_ERROR("Check REE offset info is invalid\n");
        return -1;
    }

    image_offset_info.u32ree_image_offset_start = u32ree_image_offset_start;
    image_offset_info.u32ree_image_offset_len = u32ree_image_offset_len;

    UBOOT_DEBUG("# REE_OFFSET_START = 0x%x #\n", image_offset_info.u32ree_image_offset_start);
    UBOOT_DEBUG("# REE_OFFSET_LEN = 0x%x #\n", image_offset_info.u32ree_image_offset_len);

    // pre-define offset for firmware image header
    image_offset_info.sign_of_header_size = SIGNATURE_SIZE;
    image_offset_info.version_size = HEADER_VERSION_SIZE;
    image_offset_info.image_offset_size = HEADER_IMAGE_OFFSET_SIZE;
    image_offset_info.chunk_info_size = HEADER_CHUNKINFO_SIZE;
    image_offset_info.sign_of_hash_set_size = SIGNATURE_SIZE;

    UBOOT_TRACE("OK\n");
    return 0;
}


int load_ree_script_to_dram(const char *interface, char *upgrade_file, char *download_buf)
{
    UBOOT_TRACE("IN\n");
    int load_offset = 0;
    char device_partition[10] = {0};
    char buffer[CMD_BUF] = {0};
    int snprintf_len;

    set_scrit_size_of_upgrade_image(REE_SCRIPT_SIZE);

    UBOOT_DEBUG("Before use fatload to read %s script to DRAM(0x%p)\n",upgrade_file, download_buf);
    UBOOT_DUMP(download_buf, 0x100);

    snprintf_len = snprintf(device_partition, sizeof(device_partition)-1, "%d:%d", gdevice_number, gpartition_number);
    if (snprintf_len >= CMD_BUF)
    {
        UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", CMD_BUF, buffer);
        return -1;
    }

    /* check script file is ciphertext or not(start) */
    // read REE Image Start Offset
    load_offset = image_offset_info.u32ree_image_offset_start;

    if (load_offset%0x200 != 0)
    {
        UBOOT_ERROR("Upgrade Image format is not vaild\n");
        return -1;
    }

    /* load REE script to DRAM */
#ifdef CONFIG_SECURE_UPGRADE_V2
    if(strncmp(interface, "usb", strlen("usb")) == 0)
    {
        set_upgrade_mode(EN_UPDATE_MODE_USB_WITH_SEG_DECRYPTED);
        snprintf_len = snprintf(buffer, CMD_BUF, "filepartloadSegAES %s %s 0x%p %s 0x%x 0x%x", ginterface, device_partition, download_buf, upgrade_file, REE_SCRIPT_SIZE, load_offset);
    }
    else if((strncmp(interface, "mmc", strlen("mmc")) == 0) || (strncmp(interface, "ufs", strlen("ufs")) == 0))
    {
        set_upgrade_mode(EN_UPDATE_MODE_OAD_WITH_SEG_DECRYPTED);
        snprintf_len = snprintf(buffer, CMD_BUF, "filepartloadSegAES %s %s 0x%p %s 0x%x 0x%x", ginterface, gupgradeinfo.upgrade_storage_info, download_buf, upgrade_file, REE_SCRIPT_SIZE, load_offset);
    }
    if (snprintf_len >= CMD_BUF)
    {
        UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", CMD_BUF, buffer);
        return -1;
    }
#else
    if(strncmp(interface, "usb", strlen("usb")) == 0)
    {

        set_upgrade_mode(EN_UPDATE_MODE_USB);
        snprintf_len = snprintf(buffer, CMD_BUF, "fatload %s %s 0x%p %s 0x%x 0x%x",ginterface, device_partition, download_buf, upgrade_file, REE_SCRIPT_SIZE , load_offset);
        if (snprintf_len >= CMD_BUF)
        {
            UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", CMD_BUF, buffer);
            return -1;
        }
    }
    else if((strncmp(interface, "mmc", strlen("mmc")) == 0) || (strncmp(interface, "ufs", strlen("ufs")) == 0))
    {
        set_upgrade_mode(EN_UPDATE_MODE_OAD);
        snprintf_len = snprintf(buffer, CMD_BUF, "ext4load %s %s 0x%p %s 0x%x 0x%x",ginterface, gupgradeinfo.upgrade_storage_info, download_buf, upgrade_file, REE_SCRIPT_SIZE , load_offset);
        if (snprintf_len >= CMD_BUF)
        {
            UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", CMD_BUF, buffer);
            return -1;
        }

    }
#endif

    UBOOT_DEBUG("cmd=%s\n", buffer);
    if (run_command(buffer, 0) == -1)
    {
        UBOOT_ERROR("Error: cmd: %s\n", buffer);
        reset_system();
    }
    UBOOT_DEBUG("After use fatload to read %s script to DRAM(0x%p)\n", upgrade_file, download_buf);
    UBOOT_DUMP(download_buf, 0x100);

     UBOOT_TRACE("OK\n");
     return 0;
}


int do_file_part_load (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    UBOOT_TRACE("IN\n");
    char buffer[CMD_BUF] = "\0";
    int snprintf_len;
    int ret = 0;
    char boot_device_name[STORAGE_DEVICE_BUF_SIZE] = {0};

    if (argc < 5)
    {
        cmd_usage(cmdtp);
        return -1;
    }
    ret = sys_get_boot_device(boot_device_name, sizeof(boot_device_name));
    if(ret < 0)
    {
        UBOOT_ERROR("Error: Get booting device name failure, Unknown storage device.\n");
        return ret;
    }

    switch(get_upgrade_mode())
    {
        case EN_UPDATE_MODE_USB:
            memset(buffer, 0 , CMD_BUF);
            snprintf_len = snprintf(buffer, CMD_BUF, "fatload usb %d:%d %s %s %s %s", gdevice_number, gpartition_number, argv[1], argv[2], argv[3], argv[4]);
            if (snprintf_len >= CMD_BUF)
            {
                UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", CMD_BUF, buffer);
                return -1;
            }
            UBOOT_DEBUG("cmd=%s\n", buffer);
            ret = run_command(buffer, 0);
            break;

        case EN_UPDATE_MODE_USB_WITH_SEG_DECRYPTED:
            snprintf_len = snprintf(buffer, CMD_BUF, "filepartloadSegAES usb %d:%d %s %s %s %s", gdevice_number, gpartition_number, argv[1], argv[2], argv[3], argv[4]);
            if (snprintf_len >= CMD_BUF)
            {
                UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", CMD_BUF, buffer);
                return -1;
            }
            UBOOT_DEBUG("cmd=%s\n", buffer);
            ret = run_command(buffer, 0);
            break;
        case EN_UPDATE_MODE_OAD:
            snprintf_len = snprintf(buffer, CMD_BUF, "ext4load %s %s %s %s %s %s",boot_device_name, gupgradeinfo.upgrade_storage_info, argv[1], argv[2], argv[3], argv[4]);
            if (snprintf_len >= CMD_BUF)
            {
                UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", CMD_BUF, buffer);
                return -1;
            }
            UBOOT_DEBUG("cmd=%s\n", buffer);
            ret = run_command(buffer, 0);
            break;
        case EN_UPDATE_MODE_OAD_WITH_SEG_DECRYPTED:
            snprintf_len = snprintf(buffer, CMD_BUF, "filepartloadSegAES %s %s %s %s %s %s",boot_device_name, gupgradeinfo.upgrade_storage_info, argv[1], argv[2], argv[3], argv[4]);
            if (snprintf_len >= CMD_BUF)
            {
                UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", CMD_BUF, buffer);
                return -1;
            }
            UBOOT_DEBUG("cmd=%s\n", buffer);
            ret = run_command(buffer, 0);
            break;

        default:
            UBOOT_ERROR("filepartload only support USB upgrade mode now!!!\n");
            break;
    }
    if (ret != -1)
    {
      ret = 0;
    }
    UBOOT_TRACE("OK\n");
    return ret;
}


int do_upgrade(char *upgrade_file, en_update_mode enMode, enum if_type	if_type)
{
    UBOOT_TRACE("IN\n");
    unsigned char *ptr_header;
    unsigned char *ree_script_buf = (unsigned char*)UPGRADE_LOAD_SCRIPT_BUFFER_ADDR;
    char *next_line = NULL;
    unsigned int progress_total_cnt = 0;
    unsigned int progress_cnt = 0;
    int snprintf_len = 0;
    st_image_version_info new_image_version_info = {0};

    /* 1. set the Upgrade mode */
    set_upgrade_mode(enMode);

    /* 2. set interface type (where to store the upgrade_file) */
    memset(ginterface, 0, sizeof(ginterface));

    if (if_type == IF_TYPE_USB)
        snprintf_len = snprintf(ginterface, (sizeof(ginterface)-1), "%s","usb");
    else if (if_type == IF_TYPE_MMC)
        snprintf_len = snprintf(ginterface, (sizeof(ginterface)-1), "%s","mmc");
    else if (if_type == IF_TYPE_UFS)
        snprintf_len = snprintf(ginterface, (sizeof(ginterface)-1), "%s","ufs");
    else
    {
        UBOOT_ERROR("Not support interface type '%d' (only support usb, mmc, ufs case)\n", if_type);
        return -1;
    }

    if (snprintf_len >= (sizeof(ginterface)-1))
    {
        UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", (int)(sizeof(ginterface)-1), ginterface);
        return -1;
    }
    else
        UBOOT_DEBUG("Interface type is '%s'\n", ginterface);


    /* 3. show load data progress bar on screen */
    show_load_data_osd(0);


    /* 4. set upgrade file name to uboot's env. variable */

    if (set_upgrade_file_name(gupgradeinfo.upgrade_filename) == -1)
    {
        set_upgrade_mode(EN_UPDATE_MODE_NONE);
        UBOOT_ERROR("Set upgrade mode fail\n");
        show_error_osd(0);
        return -1;
    }


    /* 5. get device & partition number which store the upgrade_file, and get size of the upgrade file */
    if (if_type == IF_TYPE_USB)
    {
        if (check_usb_upgrade_file_available(ginterface, gupgradeinfo.upgrade_filename) != 0)
        {
            set_upgrade_mode(EN_UPDATE_MODE_NONE);
            UBOOT_ERROR("Check upgrade_file Available fail\n");
            show_error_osd(0);
            return -1;
        }
    }
    else if ((if_type == IF_TYPE_MMC) || (if_type == IF_TYPE_UFS))
    {
        if (check_ext4_upgrade_file_available(ginterface) != 0)
        {
            set_upgrade_mode(EN_UPDATE_MODE_NONE);
            UBOOT_ERROR("Check upgrade_file Available fail\n");
            show_error_osd(0);
            return -1;
        }
    }

    /* 6. Read firware image header to DRAM (UPGRADE_HEADER_BUFFER_ADDR) */
    ptr_header = read_firmware_image_header(ginterface, upgrade_file);
    if (! ptr_header)
    {
        set_upgrade_mode(EN_UPDATE_MODE_NONE);
        UBOOT_ERROR("Read Firmware Image header Info fail\n");
        show_error_osd(0);
        return -1;
    }

#if defined(CONFIG_SECURE_UPGRADE_V2)
    /* 7. do firmware image header authentication */
    if (firmware_image_header_authentication(ptr_header) != 0)
    {
        UBOOT_ERROR("Firmware image header authentication fail\n");
        reset_system();
        return -1;
    }
#endif

    /* 8 check upgrade_file version info and update uboot env var*/
    switch (check_firmware_image_version_info(ptr_header, &new_image_version_info)) {
        case IMAGE_VERSIOIN_CHECKER_STATE_SAME:
            UBOOT_DEBUG("PKG Version is the same.\n");
            break;
        case IMAGE_VERSIOIN_CHECKER_STATE_UPDATE:
            update_firmware_image_version_info(&new_image_version_info);
            break;
        case IMAGE_VERSIOIN_CHECKER_STATE_FAIL:
        default:
            set_upgrade_mode(EN_UPDATE_MODE_NONE);
            UBOOT_ERROR("Check PKG Version Info fail.\n");
            show_error_osd(0);
            return -1;
            break;
    }

    /* 9. set upgrade_file offset information to global variable */
    if (read_firmware_image_offset_info(ptr_header, upgrade_file) != 0)
    {
        set_upgrade_mode(EN_UPDATE_MODE_NONE);
        UBOOT_ERROR("Read Firmware Image Offset Info fail\n");
        show_error_osd(0);
        return -1;
    }

#if defined(CONFIG_SECURE_UPGRADE_V2)
    /* 10. do image authendication & check the integrity of image */
    if (firmware_image_authentication(ginterface,upgrade_file, gdevice_number, gpartition_number) != 0)
    {
        set_upgrade_mode(EN_UPDATE_MODE_NONE);
        UBOOT_ERROR("Firmware Image Authentication fail\n");
        show_error_osd(0);
        return -1;
    }
#endif

    /* 11. load REE script in upgrade_file to DRAM */
    if (load_ree_script_to_dram(ginterface, upgrade_file, (char *)ree_script_buf) != 0)
    {
        delete_upgrade_file_name();
        set_upgrade_mode(EN_UPDATE_MODE_NONE);
        show_error_osd(0);
        return -1;
    }

    /* 12. start to run REE script */
    set_scrit_size_of_upgrade_image(REE_SCRIPT_SIZE);                    // set the size of script in upgrade file to global variable, default size is DEFAULT_SCRIPT_SIZE_FOR_UPGRADE_IMAGE(0x10000)
    progress_total_cnt=check_upgrade_script_cmd_count((char *)ree_script_buf);   // calculate the total amount of commands in the script file
    if (progress_total_cnt == 0)
    {
        delete_upgrade_file_name();
        set_upgrade_mode(EN_UPDATE_MODE_NONE);
        UBOOT_ERROR("There are no any commands in the upgrade script file\n");
        show_error_osd(0);
        return -1;
    }
    UBOOT_DEBUG("progress_total_cnt=%u\n",progress_total_cnt);

    show_start_upgrading_osd(0);

    while ((next_line = get_script_next_line((char **)&ree_script_buf)) != NULL)  // start executing script in upgrade file
    {
        UBOOT_INFO("\n>> %s \n", next_line);
        progress_cnt++;
        UBOOT_DEBUG("progress_cnt=%u,progress_total_cnt=%u\n",progress_cnt,progress_total_cnt);
        show_upgrading_osd(progress_cnt,progress_total_cnt);
        if (check_skip_cmd(next_line) == FALSE)  //filterCmd
        {
            if (run_command(next_line, 0) != 0)
            {
                delete_upgrade_file_name();
                set_upgrade_mode(EN_UPDATE_MODE_NONE);
                UBOOT_ERROR("cmd: '%s' Fail !!\n", next_line);
                show_error_osd(0);
                return -1;
            }
        }
    }

    /* 13. after the update is complete, delete upgrade file name in uboot's env. variable */
    if (delete_upgrade_file_name() == -1)
    {
        UBOOT_ERROR("Delete upgrade file name fail\n");
    }

    show_finish_osd(0);
    UBOOT_TRACE("OK\n");
    return 0;
}


#if (CONFIG_MTK_UPGRADE == 1)
int do_mtkupgrade(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    int ret = CMD_RET_SUCCESS;


	if (argc < 2 || argc > 4)
		return CMD_RET_USAGE;
    //disable wdt befor upgrade
    struct udevice *dev;
    if(uclass_get_device(UCLASS_WDT, 0, &dev)==0)
    {
        if(dev==NULL)
        {
            printf(" WDT device Null !!! \n");
        }
        else
        {
            wdt_stop(dev);
            printf(" Stop WDT !!! \n");
        }
    }
    else
        printf(" WDT get device Error !!!!\n");

    // do upgrade by USB
    if (strcmp(argv[1], "usb") == 0)
    {
        if (is_lockdown()) {
            UBOOT_ERROR("do_mtkupgrade usb fail!!! device is LOCKDOWN state.\n");
            return CMD_RET_FAILURE;
        }
#if (CONFIG_USB_UPGRADE == 1)
        if (argc == 2)                                // mtkupgrade usb, upgrade file name from 'ForceUpgradePath' in environment or default /MTKUpgrade.bin
        {
            UBOOT_DEBUG("Use '%s' do PKG upgrade\n",AP_BIN_PATH);
            ret = usb_upgrade_pkg();
        }
        else if (argc == 3)
        {
#if defined(CONFIG_MTK_INTERNAL_USB_UPGRADE_SCRIPPT)
            if ((strcmp(argv[2], "-script") == 0))   // mtkupgrade usb -script, upgrade file name from 'ForceUpgradePath' in environment or default /usb_auto_update.txt
            {
                UBOOT_DEBUG("Use '%s' do script upgrade\n",SCRIPT_PATH);
                ret = usb_upgrade_script();
            }
            else
#endif
            {
                env_set("ForceUpgradePath", argv[2]);  // mtkupgrade usb file_name, upgrade file name from cmd input
                env_save();
                UBOOT_DEBUG("Use '%s' do PKG upgrade\n",AP_BIN_PATH);
                ret = usb_upgrade_pkg();
            }
        }
        else if (argc == 4)
        {
#if defined(CONFIG_MTK_INTERNAL_USB_UPGRADE_SCRIPPT)
            if ((strcmp(argv[2], "-script") == 0))   // mtkupgrade usb -script file_name, upgrade file name from cmd input
            {
                env_set("ForceUpgradePath", argv[3]);
                env_save();
                UBOOT_DEBUG("Use '%s' do script upgrade\n",SCRIPT_PATH);
                ret = usb_upgrade_script();
            }
            else
#endif
            {
                UBOOT_ERROR("Unrecognized mtkupgrade usb parameters!!!\n");
                ret = CMD_RET_FAILURE;
            }
        }
        else
        {
            UBOOT_ERROR("Invalid mtkupgrade usb argc (%d), should be 2~4.!!!\n", argc);
            ret = CMD_RET_FAILURE;
        }
#else
        UBOOT_ERROR("CONFIG_USB_UPGRADE is not set!!!\n");
        ret = CMD_RET_FAILURE;
#endif
    }

    // do upgrade by OAD
    else if (strcmp(argv[1], "oad") == 0)
    {
        UBOOT_ERROR("Not support OAD upgrade now!!!\n");
        ret = CMD_RET_FAILURE;
    }

    // do upgrade by NET
	else if (strcmp(argv[1], "net") == 0)
    {
        UBOOT_ERROR("Not support NET upgrade now!!!\n");
        ret = CMD_RET_FAILURE;
    }

    else
    {
        UBOOT_ERROR("Unrecognized upgrade_source!!!\n");
        ret = CMD_RET_FAILURE;
    }


    if (ret != 0)
    {
        UBOOT_ERROR("do_mtkupgrade fail!!!\n");
		return CMD_RET_FAILURE;
	}
    return CMD_RET_SUCCESS;
}
#endif

#if (CONFIG_USB_UPGRADE == 1)
#if (CONFIG_MMC == 1)
#define PARTIAL_UPGRADE_BUFFER_SIZE    0x3200000
int do_usb_partial_upgrade_to_emmc(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    struct blk_desc *blk_dev_desc = NULL;
    disk_partition_t partition_info;
    char command[COMMAND_BUF_SIZE] = {0};
    char device_name[STORAGE_DEVICE_BUF_SIZE] = {0};
    char *file_name = NULL;
    char *partition_name = NULL;
    char *env_filesize = NULL;
    int usb_device = 0;
    int usb_part = 0;
    int device = 0;
    unsigned int partition_num = 0;
    unsigned long file_size = 0;
    unsigned long remain_size = 0;
    unsigned long partial_size = 0;
    unsigned long file_offset = 0;
    int ret = 0;

    if (argc < 3)
        return CMD_RET_USAGE;

    file_name = argv[1];
    partition_name = argv[2];
    UBOOT_DEBUG("file_name='%s', partition_name='%s'\n", file_name, partition_name);

    ret = sys_get_boot_device(device_name, sizeof(device_name));
    if (ret < 0 || strncmp(device_name, "mmc", strlen(device_name)) != 0)
    {
        UBOOT_ERROR("Error: no mmc boot device found\n");
        return CMD_RET_FAILURE;
    }

    blk_dev_desc = blk_get_dev(device_name, device);
    ret = get_partition_info(blk_dev_desc, partition_name, &partition_info, &partition_num);
    if (ret != CMD_RET_SUCCESS)
    {
        UBOOT_ERROR("Error: get partition info of %s failure\n", partition_name);
        return CMD_RET_FAILURE;
    }

    if (check_usb_file_partition("usb", &usb_device, &usb_part, file_name) != 0)
    {
        UBOOT_ERROR("Error: please run 'usb start' first, and check %s in usb disk\n", file_name);
        return CMD_RET_FAILURE;
    }

    UBOOT_DEBUG("Get file size of %s in usb\n", file_name);
    ret = snprintf(command, COMMAND_BUF_SIZE, "fatsize usb %d:%d %s", usb_device, usb_part, file_name);
    if (ret < 0 || ret >= COMMAND_BUF_SIZE)
    {
        UBOOT_ERROR("The array size is too small(%u), snprintf fail '%s'\n", COMMAND_BUF_SIZE, command);
        return CMD_RET_FAILURE;
    }
    UBOOT_DEBUG("cmd=%s\n", command);
    ret = run_command(command, 0);
    if (ret != CMD_RET_SUCCESS)
    {
        UBOOT_ERROR("Error: get file size of %s fail\n", file_name);
        return CMD_RET_FAILURE;
    }

    env_filesize = env_get("filesize");
    if (env_filesize == NULL)
    {
        UBOOT_ERROR("Error: get env of 'filesize' fail\n");
        return CMD_RET_FAILURE;
    }
    file_size = simple_strtoul(env_filesize, NULL, 16);
    remain_size = file_size;
    printf("%s size : 0x%lX\n", file_name, file_size);
    if (file_size > (partition_info.size * partition_info.blksz))
    {
        UBOOT_ERROR("File size (0x%lX) of %s is bigger than partition size (0x%lX) of %s\n", file_size, file_name, (partition_info.size * partition_info.blksz), partition_name);
        return CMD_RET_FAILURE;
    }

    while (remain_size > 0)
    {
        if (remain_size > PARTIAL_UPGRADE_BUFFER_SIZE)
            partial_size = PARTIAL_UPGRADE_BUFFER_SIZE;
        else
            partial_size = remain_size;

        ret = snprintf(command, COMMAND_BUF_SIZE, "fatload usb %d:%d 0x%lX %s 0x%lX 0x%lX",
                       usb_device, usb_part, (unsigned long)CONFIG_UPGRADE_BUFFER_ADDR, file_name, partial_size, file_offset);
        if (ret < 0 || ret >= COMMAND_BUF_SIZE)
        {
            UBOOT_ERROR("The array size is too small(%u), snprintf fail '%s'\n", COMMAND_BUF_SIZE, command);
            return CMD_RET_FAILURE;
        }
        UBOOT_DEBUG("cmd=%s\n", command);
        ret = run_command(command, 0);
        if (ret != CMD_RET_SUCCESS)
        {
            UBOOT_ERROR("Load file of %s fail\n", file_name);
            return CMD_RET_FAILURE;
        }

        flush_cache((unsigned long)CONFIG_UPGRADE_BUFFER_ADDR, partial_size);
        ret = snprintf(command, COMMAND_BUF_SIZE, "partition write mmc %d %s 0x%lX 0x%lX 0x%lX",
                       device, partition_name, (unsigned long)CONFIG_UPGRADE_BUFFER_ADDR, partial_size, file_offset);
        if (ret < 0 || ret >= COMMAND_BUF_SIZE)
        {
            UBOOT_ERROR("The array size is too small(%u), snprintf fail '%s'\n", COMMAND_BUF_SIZE, command);
            return CMD_RET_FAILURE;
        }
        UBOOT_DEBUG("cmd=%s\n", command);
        ret = run_command(command, 0);
        if (ret != CMD_RET_SUCCESS)
        {
            UBOOT_ERROR("Write file to %s fail\n", partition_name);
            return CMD_RET_FAILURE;
        }

        remain_size -= partial_size;
        file_offset += partial_size;
    }

    return CMD_RET_SUCCESS;
}
#endif
#endif

