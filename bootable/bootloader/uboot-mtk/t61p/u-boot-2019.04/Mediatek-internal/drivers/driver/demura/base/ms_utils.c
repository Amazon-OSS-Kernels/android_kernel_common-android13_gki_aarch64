// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#include <stdio.h>
#include "common.h"
#include "command.h"
#include "fs.h"
#include "malloc.h"
#include "environment.h"
#include "debug_impl.h"
#include <linux/types.h>
#include <asm/byteorder.h>
#include "drvMIU.h"
#include "drvBDMA.h"
#include "spi.h"
#include "demura_config.h"
#include "ms_utils.h"
#include "crc_libs.h"
#include "utility.h"
#ifdef CONFIG_MULTICORES_PLATFORM
#include <smp/thread_info.h>
extern smp_spin_lock_t fs_spin_lock;
#endif

#include "panel_impl.h"
#include "dts_parser.h"

#ifdef ALIGN
#undef ALIGN
#define ALIGN(x,a)        __ALIGN_MASK((x),(typeof(x))(a)-1)
#define __ALIGN_MASK(x,mask)    (((x)+(mask))&~(mask))
#endif

#define ALIGN_1MB                (1 * 1024 * 1024)
#define DEMURA_SF_MAGIC_STRING   "Mstar CRC :\0"

#define STORAGE_DEVICE_BUF_SIZE 8
#define STORAGE_INDEX_BUF_SIZE 8
#define DEMEURA_BIN_FILE_PATH DEMURA_BIN_FILE_PATH
#define DEMEURA_BIN_FILE_PATH_DLG DEMURA_BIN_FILE_PATH_DLG

#define MSPI_DRIVER_NOT_READY

#if defined(CONFIG_DEMURA_VENDOR_BACKLIGHT)
#define MAX_CUST_PATH_LEN   (128)
#endif

struct mstar_crc_hdr {
	char magic[12];
	MS_U32  dat_offset;
	MS_U32  dat_length;
	MS_U32 crc_32;
};

static MS_U8 gu8_demurabin_type = E_MS_UTIL_BIN_TYPE_MAIN;
static MS_U8 gu8_demurabin_act = E_MS_UTIL_BIN_ACT_OFF;
static MS_U8 gu8_demurabin_file = E_MS_UTIL_BIN_FILE_NORMAL;
static MS_U8 gu8_demura_block_size = E_MS_UTIL_BLOCK_SIZE_H8V8;

static void backlight_demura_file_sys_read(const char *file_name, char **cfg_partition)
{
    UBOOT_TRACE("IN\n");

    if ((E_MS_UTIL_BIN_FILE_NORMAL == get_demura_file()) && (env_get(DEMURA_ENV_PARTITION) != NULL))
    {
        *cfg_partition = env_get(DEMURA_ENV_PARTITION);
    }
    else if ((E_MS_UTIL_BIN_FILE_BACKLIGHT == get_demura_file()) && (env_get(DEMURA_ENV_PARTITION_BACKLIGHT) != NULL) && \
        (strcmp(DEMEURA_BIN_FILE_PATH, file_name) != 0) && (strcmp(DEMEURA_BIN_FILE_PATH_DLG, file_name) != 0))
    {
        *cfg_partition = env_get(DEMURA_ENV_PARTITION_BACKLIGHT);
    }
    else if (((E_MS_UTIL_BIN_FILE_BACKLIGHT == get_demura_file()) || (E_MS_UTIL_BIN_FILE_BACKLIGHT_ONLY == get_demura_file())) && \
        (strcmp(DEMEURA_BIN_FILE_PATH, file_name) != 0) && (strcmp(DEMEURA_BIN_FILE_PATH_DLG, file_name) != 0))
    {
#if defined(CONFIG_DEMURA_VENDOR_BACKLIGHT)
        st_cust_dmc_info st_cust_dmc_info;
        memset(&st_cust_dmc_info, 0x00, sizeof(st_cust_dmc_info));
        parse_dt("/video_out", cus_demura_dt_parser, (void*)&st_cust_dmc_info, NULL);
        UBOOT_TRACE("backlight demura is %s\n", st_cust_dmc_info.bl_dmc_enable ? "enable" : "disable");
        UBOOT_TRACE("backlight demura partiton %s.\n", st_cust_dmc_info.bl_dmc_partiton);
        if (((E_MS_UTIL_BIN_FILE_BACKLIGHT == get_demura_file()) || (E_MS_UTIL_BIN_FILE_BACKLIGHT_ONLY == get_demura_file())) && \
            (st_cust_dmc_info.bl_dmc_enable))
        {
            *cfg_partition = st_cust_dmc_info.bl_dmc_partiton;
        }
#else
        UBOOT_TRACE("dmura partition is %s\n", DEMURA_PARTITION_NAME);
#endif
    }
    else
    {
        UBOOT_TRACE("dmura partition is %s\n", DEMURA_PARTITION_NAME);
    }
}

static MS_BOOL file_sys_read(const char *file_name, MS_U8 *pdat, MS_U32 pos, MS_U32 dat_len)
{
//    loff_t read_len = 0;
    loff_t size =0;
    char device_name[STORAGE_DEVICE_BUF_SIZE] = {0};
    unsigned int device = 0;
    char storage_info[STORAGE_INDEX_BUF_SIZE] = {0};
//    int ret;
    loff_t act_read = 0;
//    int rc;
    char *cfg_partition = NULL;

    UBOOT_TRACE("IN\n");

    backlight_demura_file_sys_read(file_name, &cfg_partition);
#ifdef CONFIG_MULTICORES_PLATFORM
    unsigned long irq_flag = 0;
#endif
    UBOOT_TRACE("file: %s in %s, pos = %d, dat_len = %d\n", file_name, cfg_partition, pos, dat_len);

    if(pdat == NULL)
    {
        UBOOT_ERROR("Error: No valid memo addr\n");
        return FALSE;
    }

    if(sys_get_boot_device(device_name, sizeof(device_name)) < 0)
    {
        UBOOT_ERROR("Error: Get booting device name failure, Unknown storage device.\n");
        return FALSE;
    }
    UBOOT_TRACE("device_name = %s\n", device_name);


    if (cfg_partition)
    {
        UBOOT_TRACE("cfg_partition = %s\n", cfg_partition);
        if (sys_get_storage_info(device, cfg_partition, storage_info) < 0)
        {
            UBOOT_ERROR("Error: sys_get_storage_info failure\n");
            return FALSE;
        }
    }
    else if(sys_get_storage_info(device, DEMURA_PARTITION_NAME, storage_info) < 0)
    {
        UBOOT_ERROR("Error: sys_get_storage_info failure\n");
        return FALSE;
    }
    UBOOT_DEBUG("device = %d, storage_info = %s\n", device, storage_info);


#if defined(CONFIG_MULTICORES_PLATFORM)
    smp_spin_lock_save(&fs_spin_lock, irq_flag);
#endif
    if (fs_set_blk_dev(device_name, storage_info, FS_TYPE_EXT))
    {
        UBOOT_ERROR("Error: partition select failure\n");
        goto FILE_SYS_READ_FAIL;
    }

    if (fs_size(file_name, &size) < 0)
    {
        UBOOT_ERROR("Error: Get %s file size failure\n", file_name);
        goto FILE_SYS_READ_FAIL;
    }

    if(size < dat_len)
    {
        UBOOT_ERROR("Read length(%d) exceeds the file size(%d)\n", dat_len, (unsigned int)size);
        goto FILE_SYS_READ_FAIL;
    }

    if (fs_set_blk_dev(device_name, storage_info, FS_TYPE_EXT))
    {
        UBOOT_ERROR("Error: partition select failure\n");
        goto FILE_SYS_READ_FAIL;
    }

    if(0 > fs_read(file_name, (ulong)pdat, (loff_t)pos, dat_len, &act_read))
    {
        UBOOT_DEBUG("fail to read: %s\n", file_name);
        goto FILE_SYS_READ_FAIL;
    }

    if(act_read != (loff_t)dat_len)
    {
        UBOOT_ERROR("actual read length (%d) < dat_len (%d)\n", (MS_U32)act_read, (MS_U32)dat_len);
        goto FILE_SYS_READ_FAIL;
    }

#if defined(CONFIG_MULTICORES_PLATFORM)
    smp_spin_unlock_restore(&fs_spin_lock, irq_flag);
#endif
    UBOOT_TRACE("Read %s file success\n", file_name);
    UBOOT_DUMP(pdat, dat_len<=158 ? dat_len: 320);

    UBOOT_TRACE("OK\n");
    return TRUE;

FILE_SYS_READ_FAIL:
#if defined(CONFIG_MULTICORES_PLATFORM)
    smp_spin_unlock_restore(&fs_spin_lock, irq_flag);
#endif
    return FALSE;
}

static MS_BOOL file_sys_write(const char *file_name, MS_U8 *pdat, MS_U32 partition_oft, MS_U32 dat_len)
{
    loff_t actwrite = 0;
    char device_name[STORAGE_DEVICE_BUF_SIZE] = {0};
    unsigned int device = 0;
    char storage_info[STORAGE_INDEX_BUF_SIZE] = {0};
    char *cfg_partition = env_get(DEMURA_ENV_PARTITION);
#ifdef CONFIG_MULTICORES_PLATFORM
    unsigned long irq_flag = 0;
#endif

    UBOOT_TRACE("IN\n");

    if(pdat == NULL)
    {
        UBOOT_ERROR("Error: Invalid dram_addr\n");
        return FALSE;
    }

    if(sys_get_boot_device(device_name, sizeof(device_name)) < 0)
    {
        UBOOT_ERROR("Error: Get booting device name failure.\n");
        return FALSE;
    }

    if (cfg_partition)
    {
        UBOOT_TRACE("cfg_partition = %s\n", cfg_partition);
        if (sys_get_storage_info(device, cfg_partition, storage_info) < 0)
        {
            UBOOT_ERROR("Error: sys_get_storage_info failure\n");
            return FALSE;
        }
    }
    else if(sys_get_storage_info(device, DEMURA_PARTITION_NAME,storage_info) < 0)
    {
        UBOOT_ERROR("Error: sys_get_storage_info failure\n");
        return FALSE;
    }

#if defined(CONFIG_MULTICORES_PLATFORM)
    smp_spin_lock_save(&fs_spin_lock, irq_flag);
#endif
    if (fs_set_blk_dev(device_name, storage_info, FS_TYPE_EXT))
    {
#if defined(CONFIG_MULTICORES_PLATFORM)
        smp_spin_unlock_restore(&fs_spin_lock, irq_flag);
#endif
        UBOOT_ERROR("Error: partition select failure\n");
        return FALSE;
    }

    if(fs_write(file_name, (ulong)pdat, (loff_t)partition_oft,
                (loff_t)dat_len, &actwrite) < 0)
    {
#if defined(CONFIG_MULTICORES_PLATFORM)
        smp_spin_unlock_restore(&fs_spin_lock, irq_flag);
#endif
        UBOOT_ERROR("Error: Write file %s failure, actwrite=0x%x, dat_len=0x%x\n",
            file_name, (unsigned int)actwrite, dat_len);
        return FALSE;
    }

#if defined(CONFIG_MULTICORES_PLATFORM)
    smp_spin_unlock_restore(&fs_spin_lock, irq_flag);
#endif
    UBOOT_TRACE("OK\n");
    return TRUE;
}

#if CONFIG_SPI_FLASH

#if 1 //ndef MSPI_DRIVER_NOT_READY
static MS_U32 sf_init_flag = FALSE;

MS_BOOL init_spi_flash(void)
{
    char cmd[256];
    char *cfg_bypass_spi = env_get(DEMURA_ENV_BYPASS_SPI);
    st_panel_spi_info panel_spi_bus = {0};

    if (cfg_bypass_spi)
    {
        return TRUE;
    }
    if ((get_demura_file() == E_MS_UTIL_BIN_FILE_BACKLIGHT) || (get_demura_file() == E_MS_UTIL_BIN_FILE_BACKLIGHT_ONLY))
    {
        return TRUE;
    }

    if (sf_init_flag == FALSE)
    {
        memset(cmd, 0, sizeof(cmd));
        if (!parse_dt("/video_out", panel_spi_bus_info_dt_parser, (void*)&panel_spi_bus, NULL))
        {
            if (snprintf(cmd, sizeof(cmd), "sf probe %s %d %d", panel_spi_bus.panel_spi_bus, panel_spi_bus.panel_spi_speed, panel_spi_bus.panel_spi_mode) < 0)
                return FALSE;
            else
                printf("[customized]sf probe %s %d %d \n", panel_spi_bus.panel_spi_bus, panel_spi_bus.panel_spi_speed, panel_spi_bus.panel_spi_mode);
        }
        else
        {
            if (snprintf(cmd, sizeof(cmd), "sf probe %s %d %d", CONFIG_DEMURA_SPI_BUS, SPI_CLOCLK, SPI_MODE) < 0)
                return FALSE;
            else
                printf("[default]sf probe %s %d %d \n", CONFIG_DEMURA_SPI_BUS, SPI_CLOCLK, SPI_MODE);
        }
        if (run_command(cmd, 0) != 0)
        {
            printf("command(%s) error!\n", cmd);
            return FALSE;
        }
        sf_init_flag = TRUE;
    }
    return TRUE;
}

void read_backlight_vendor_bin(char **cfg_vendorbin, char *backlight_vendorbin)
{
#if defined(CONFIG_DEMURA_VENDOR_BACKLIGHT)
    st_cust_dmc_info st_cust_dmc_info;

    UBOOT_TRACE("IN\n");

    memset(&st_cust_dmc_info, 0x00, sizeof(st_cust_dmc_info));
    parse_dt("/video_out", cus_demura_dt_parser, (void*)&st_cust_dmc_info, NULL);

    if (st_cust_dmc_info.bl_dmc_enable)
    {
        memcpy(backlight_vendorbin, st_cust_dmc_info.bl_dmc_vendor_bin, \
            ((strlen(st_cust_dmc_info.bl_dmc_vendor_bin) - strlen(".bin")) > MAX_CUST_PATH_LEN) \
            ? MAX_CUST_PATH_LEN : \
            (strlen(st_cust_dmc_info.bl_dmc_vendor_bin) - strlen(".bin")));

        if (E_MS_UTIL_BLOCK_SIZE_H4V4 == get_demura_block_size())
        {
            strncat(backlight_vendorbin, "_H4V4.bin", sizeof("_H4V4.bin"));
            *cfg_vendorbin = backlight_vendorbin;
        }
        else if (E_MS_UTIL_BLOCK_SIZE_H4V8 == get_demura_block_size())
        {
            strncat(backlight_vendorbin, "_H4V8.bin", sizeof("_H4V8.bin"));
            *cfg_vendorbin = backlight_vendorbin;
        }
        else if (E_MS_UTIL_BLOCK_SIZE_H4V16 == get_demura_block_size())
        {
            strncat(backlight_vendorbin, "_H4V16.bin", sizeof("_H4V16.bin"));
            *cfg_vendorbin = backlight_vendorbin;
        }
        else if (E_MS_UTIL_BLOCK_SIZE_H8V4 == get_demura_block_size())
        {
            strncat(backlight_vendorbin, "_H8V4.bin", sizeof("_H8V4.bin"));
            *cfg_vendorbin = backlight_vendorbin;
        }
/*
        else if (E_MS_UTIL_BLOCK_SIZE_H8V8 == get_demura_block_size())
        {
            strncat(backlight_vendorbin, "_H8V8.bin", sizeof("_H8V8.bin"));
            *cfg_vendorbin = backlight_vendorbin;
        }
*/
        else if (E_MS_UTIL_BLOCK_SIZE_H8V16 == get_demura_block_size())
        {
            strncat(backlight_vendorbin, "_H8V16.bin", sizeof("_H8V16.bin"));
            *cfg_vendorbin = backlight_vendorbin;
        }
        else if (E_MS_UTIL_BLOCK_SIZE_H16V4 == get_demura_block_size())
        {
            strncat(backlight_vendorbin, "_H16V4.bin", sizeof("_H16V4.bin"));
            *cfg_vendorbin = backlight_vendorbin;
        }
        else if (E_MS_UTIL_BLOCK_SIZE_H16V8 == get_demura_block_size())
        {
            strncat(backlight_vendorbin, "_H16V8.bin", sizeof("_H16V8.bin"));
            *cfg_vendorbin = backlight_vendorbin;
        }
        else if (E_MS_UTIL_BLOCK_SIZE_H16V16 == get_demura_block_size())
        {
            strncat(backlight_vendorbin, "_H16V16.bin", sizeof("_H16V16.bin"));
            *cfg_vendorbin = backlight_vendorbin;
        }
        else
        {
            *cfg_vendorbin = st_cust_dmc_info.bl_dmc_vendor_bin;
        }
    }
#else
    UBOOT_TRACE("dmura partition is %s\n", DEMURA_PARTITION_NAME);
#endif
}

MS_BOOL read_spi_flash(MS_U8 *pBuf, MS_U32 pos, MS_U32 length)
{
    char cmd[256];
    char *cfg_bypass_spi = env_get(DEMURA_ENV_BYPASS_SPI);
    char *cfg_vendorbin = NULL;
#if defined(CONFIG_DEMURA_VENDOR_BACKLIGHT)
    char *backlight_vendorbin = NULL;
#endif

    UBOOT_TRACE("IN\n");

    backlight_vendorbin = malloc(MAX_CUST_PATH_LEN);
    memset(backlight_vendorbin, 0x00, MAX_CUST_PATH_LEN);

    if ((cfg_bypass_spi) && (get_demura_file() == E_MS_UTIL_BIN_FILE_NORMAL))
    {
        cfg_vendorbin = env_get(DEMURA_ENV_VENDOR_BIN);
    }
    else if ((get_demura_file() == E_MS_UTIL_BIN_FILE_BACKLIGHT) && (env_get(DEMURA_ENV_VENDOR_BACKLIGHT_BIN) != NULL))
    {
        cfg_vendorbin = env_get(DEMURA_ENV_VENDOR_BACKLIGHT_BIN);
    }
    else if ((get_demura_file() == E_MS_UTIL_BIN_FILE_BACKLIGHT) || (get_demura_file() == E_MS_UTIL_BIN_FILE_BACKLIGHT_ONLY))
    {
        read_backlight_vendor_bin(&cfg_vendorbin, backlight_vendorbin);
    }
        if (cfg_vendorbin)
        {
            UBOOT_TRACE("demura: read \"%s\"\n", cfg_vendorbin);
            if (file_sys_read(cfg_vendorbin, pBuf, pos, length))
            {
            UBOOT_TRACE("OK\n");
#if defined(CONFIG_DEMURA_VENDOR_BACKLIGHT)
            if (backlight_vendorbin != NULL)
            {
                free(backlight_vendorbin);
            }
#endif
                return TRUE;
            }
        else
        {
            if (backlight_vendorbin != NULL)
            {
                free(backlight_vendorbin);
            }
            return FALSE;
        }
    }

    memset(cmd, 0, sizeof(cmd));
    if (snprintf(cmd, sizeof(cmd), "sf read %p 0x%x 0x%x", pBuf, (unsigned int)pos, (unsigned int)length) < 0)
    {
#if defined(CONFIG_DEMURA_VENDOR_BACKLIGHT)
    if (backlight_vendorbin != NULL)
    {
        free(backlight_vendorbin);
    }
#endif
        return FALSE;
    }
    if (run_command(cmd, 0) != 0)
    {
        printf("command(%s) error\n", cmd);
#if defined(CONFIG_DEMURA_VENDOR_BACKLIGHT)
        if (backlight_vendorbin != NULL)
        {
            free(backlight_vendorbin);
        }
#endif
        return FALSE;
    }
    UBOOT_TRACE("OK\n");
#if defined(CONFIG_DEMURA_VENDOR_BACKLIGHT)
    if (backlight_vendorbin != NULL)
    {
        free(backlight_vendorbin);
    }
#endif
    return TRUE;
}
#else //Use file system read to replace spi_read
/* No need to init */

#define DEMEURA_FLASH_FILE_PATH "demura_in.bin"

MS_BOOL init_spi_flash(void)
{
    return TRUE;
}

MS_BOOL read_spi_flash(MS_U8 *pdat, MS_U32 pos, MS_U32 dat_len)
{    

    return file_sys_read(DEMEURA_FLASH_FILE_PATH, pdat, pos, dat_len);
}

#endif /* #ifndef MSPI_DRIVER_NOT_READY */
#endif /* #if CONFIG_SPI_FLASH */

/*
MS_BOOL sf_write_data_safe(MS_U32 expect_addr, MS_U8 *pdat, MS_U32 dat_len, MS_U32 *extact_addr)
{
    MS_U32 i, flash_size, align_addr;
    MS_U32 wr_begin, wr_end;
#if 0 //csmao: mark up for temp
    MS_U32 sector_size = ONE_SECTOR_SIZE;
    char  cmd[256];
    MS_U8 buf[ONE_SECTOR_SIZE];
#else
    MS_U32 sector_size = 128;
    char  cmd[256];
    MS_U8 buf[128];
#endif
    

    *extact_addr = -1;
    flash_size = get_spi_flash_size();
    align_addr = ALIGN(expect_addr, sector_size);
    wr_begin = align_addr;
    wr_end   = wr_begin + sector_size;
    memset(buf, 0, sizeof(buf));

    if (wr_end > flash_size)
    {
        UBOOT_ERROR("can not find eraseable sector (0x%x < 0x%x)!\n", (uint)flash_size, (uint)wr_end);
        return FALSE;
    }
    if (dat_len > sector_size)
    {
        UBOOT_ERROR("only support little data to write (0x%x < 0x%x)!\n", (uint)sector_size, (uint)dat_len);
        return FALSE;
    }

    if (spi_flash_erase_one_sector(wr_begin) != 0)
    {
        UBOOT_ERROR("spi_flash_erase_one_sector error!\n");
        return FALSE;
    }

    // Check if erase successfully
    memset(cmd, 0, sizeof(cmd));
    if(snprintf(cmd, sizeof(cmd), "sf read %p 0x%x 0x%x", buf, (unsigned int)wr_begin, (unsigned int)sector_size) < 0)
        return FALSE;
    if (run_command(cmd, 0) != 0)
    {
        UBOOT_ERROR("command(%s) error\n", cmd);
        return FALSE;
    }
    for (i = 0; i < sector_size; i++)
    {
        if (buf[i] != 0xFF)
        {
            UBOOT_ERROR("erase sectors error !\n");
            return FALSE;
        }
    }

    memset(cmd, 0x00, sizeof(cmd));
    memcpy(buf, pdat, dat_len);
    if(snprintf(cmd, sizeof(cmd), "sf write %p 0x%x 0x%x", buf, (unsigned int)wr_begin, (unsigned int)dat_len) < 0)
        return FALSE;
    if (run_command(cmd, 0) != 0)
    {
        UBOOT_ERROR("command(%s) error\n", cmd);
        return FALSE;
    }
    *extact_addr = wr_begin;
    return TRUE;
}

*/
/*
MS_BOOL mark_sf_by_mstar(MS_U32 sf_dat_oft, MS_U8 *pdat, MS_U32 dat_len)
{
    char buf[64];
    MS_U32 extact_addr, begin, end;
    struct mstar_crc_hdr hdr;

    memset(&hdr, 0, sizeof(hdr));
    memcpy(hdr.magic, DEMURA_SF_MAGIC_STRING, sizeof(hdr.magic));
    hdr.dat_offset = sf_dat_oft;
    hdr.dat_length = dat_len;
    hdr.crc_32 = MDrv_CRC32_Cal_DeMura((MS_U8 *)pdat, dat_len);

    begin = sf_dat_oft;
    end   = begin + dat_len;
    if (sf_write_data_safe(end, (MS_U8 *)&hdr, sizeof(hdr), &extact_addr) != TRUE)
    {
        UBOOT_ERROR("sf_write_data_safe error\n");
        return FALSE;
    }

    sprintf(buf, "0x%X", (uint)hdr.crc_32);
    env_set(ENV_DEMURA_MSTAR_CRC, buf);
    sprintf(buf, "0x%X", (uint)extact_addr);
    env_set(ENV_DEMURA_MSTAR_CRC_POS, buf);
#if CONFIG_DEMURA_ENV_SAVEENV
    env_save();
#endif
    return TRUE;
}

static MS_U32 get_sf_crc(MS_U32 crc_pos)
{
    MS_U8 buf[256];

    memset(buf, 0, sizeof(buf));
    if (read_spi_flash(buf, crc_pos, 256) != TRUE)
    {
        UBOOT_ERROR("read_spi_flash error\n");
        return 0xFFFFffff;
    }

    struct mstar_crc_hdr *phdr = (struct mstar_crc_hdr *)buf;
    if (strncmp(phdr->magic, DEMURA_SF_MAGIC_STRING, sizeof(phdr->magic)) != 0)
    {
        UBOOT_ERROR("Invalid demura sf header !\n");
        return 0xFFFFffff;
    }
    return phdr->crc_32;
}

MS_BOOL sf_match_by_mstar(void)
{
    char *crc_str, *cpos_str;
    MS_U32 crc_board, crc_sf, crc_pos;

    if (init_spi_flash() != TRUE)
    {
        UBOOT_ERROR("init_spi_flash error, Assume that data is match !\n");
        return TRUE;
    }

    crc_str  = env_get(ENV_DEMURA_MSTAR_CRC);
    cpos_str = env_get(ENV_DEMURA_MSTAR_CRC_POS);
    if ((crc_str == NULL) || (cpos_str == NULL))
    {
        UBOOT_ERROR("Can not find env(%s and %s), Don't match !\n", ENV_DEMURA_MSTAR_CRC, ENV_DEMURA_MSTAR_CRC_POS);
        return FALSE;
    }
    crc_pos = simple_strtoul(cpos_str, NULL, 0);
    crc_board = simple_strtoul(crc_str, NULL, 0);

    crc_sf = get_sf_crc(crc_pos);
    if (crc_sf == 0xFFFFffff)
    {
        return FALSE;
    }
    if (crc_board != crc_sf)
    {
        UBOOT_ERROR("crc_board(0x%x) != crc_sf(0x%x)\n", (uint)crc_board, (uint)crc_sf);
        return FALSE;
    }
    return TRUE;
}
*/

MS_BOOL write_raw_data(MS_U8 *pdat, MS_U32 partition_oft, MS_U32 dat_len)
{
#if 0 //csmao: mark up, should write to file system
    MS_U32 result;
    raw_io_config_push();
#if 0 //csmao: mark up for temp
    #if defined (CONFIG_NAND_FLASH) || defined(CONFIG_SPI_NAND_FLASH)
    // dynpart edb64M-nand:0x800000(KL),0x700000(UBIRO),0x100000(RTPM),-(UBI)
    // dynpart edb64M-nand:0x800000(KL),0x700000(UBIRO),0x300000(demura),0x100000(RTPM),-(UBI)
    result = raw_io_config(E_RAW_DATA_IN_NAND, DEMURA_PARTITION_NAME, UNNECESSARY_PARTITION);
    #elif defined (CONFIG_MMC)
    result = raw_io_config(E_RAW_DATA_IN_MMC,  DEMURA_PARTITION_NAME, MMC_DEFAULT_VOLUME);
    #else
    UBOOT_ERROR("Unknow storage\n");
    result = -1;
    #endif
#else
    result = raw_io_config(22,  DEMURA_PARTITION_NAME, "MMC_DEFAULT_VOLUME");
#endif
    if (result != 0)
    {
        UBOOT_ERROR("raw_io_config Failed\n");
        raw_io_config_pop();
        return FALSE;
    }

    result = raw_write((MS_U32)pdat, partition_oft, dat_len);
    if (result != 0)
    {
        UBOOT_ERROR("raw write demura partition Failed\n");
        raw_io_config_pop();
        return FALSE;
    }
    raw_io_config_pop();
#else

    if (gu8_demurabin_type == E_MS_UTIL_BIN_TYPE_DLG)
        return file_sys_write(DEMEURA_BIN_FILE_PATH_DLG, pdat, partition_oft, dat_len);
    return file_sys_write(DEMEURA_BIN_FILE_PATH, pdat, partition_oft, dat_len);
#endif
}

MS_BOOL read_raw_data(MS_U8 *pdat, MS_U32 partition_oft, MS_U32 dat_len)
{
#if 0 //csmao: mark up, should read from file system

    MS_U32 result;
    raw_io_config_push();
#if 0
    #if defined (CONFIG_NAND_FLASH) || defined(CONFIG_SPI_NAND_FLASH)
    // dynpart edb64M-nand:0x800000(KL),0x700000(UBIRO),0x100000(RTPM),-(UBI)
    // dynpart edb64M-nand:0x800000(KL),0x700000(UBIRO),0x300000(demura),0x100000(RTPM),-(UBI)
    result = raw_io_config(E_RAW_DATA_IN_NAND, DEMURA_PARTITION_NAME, UNNECESSARY_PARTITION);
    #elif defined (CONFIG_MMC)
    result = raw_io_config(E_RAW_DATA_IN_MMC,  DEMURA_PARTITION_NAME, MMC_DEFAULT_VOLUME);
    #else
    UBOOT_ERROR("Unknow storage\n");
    result = -1;
    #endif
#else
    result = raw_io_config(2,  DEMURA_PARTITION_NAME, "MMC_DEFAULT_VOLUME");
#endif
    if (result != 0)
    {
        UBOOT_ERROR("raw_io_config Failed\n");
        raw_io_config_pop();
        return FALSE;
    }

    result = raw_read((MS_U32)pdat, partition_oft, dat_len);
    if (result != 0)
    {
        UBOOT_ERROR("raw read demura partition Failed\n");
        raw_io_config_pop();
        return FALSE;
    }
    raw_io_config_pop();
#else

    if (gu8_demurabin_type == E_MS_UTIL_BIN_TYPE_DLG)
        return file_sys_read(DEMEURA_BIN_FILE_PATH_DLG, pdat, partition_oft, dat_len);
    return file_sys_read(DEMEURA_BIN_FILE_PATH, pdat, partition_oft, dat_len);

#endif
}



#if 0 //csmao: mark up, seems useless.
static MS_BOOL adjust_memory(MS_PHYADDR *pmem_start, MS_PHYADDR mem_len, MS_U32 min_distance)
{
    // Get MBoot self memory infomation
    static MS_U32 has_inited = FALSE;
    static MS_PHYADDR mboot_start = 0;
    static MS_PHYADDR mboot_end   = 0;
    static MS_PHYADDR mboot_len = 0;

    UBOOT_TRACE("IN\n");
    if (has_inited == FALSE)
    {
        if (get_map_size_from_env(/*E_MBOOT_MEM_USAGE*/27, /*NO_DEFAULT_MMAP_VALUE*/0xFFFFFFFF, &mboot_len) != 0)
            return FALSE;
        if (get_map_addr_from_env(/*E_MBOOT_MEM_USAGE*/27, /*NO_DEFAULT_MMAP_VALUE*/0xFFFFFFFF, &mboot_start) != 0)
            return FALSE;
        mboot_end  = mboot_start + mboot_len;
        has_inited = TRUE;
    }

    MS_PHYADDR mem_start = *pmem_start;
    MS_PHYADDR mem_end   = mem_start + mem_len;

    if (mem_start <= mboot_start)
    {
        if ((mboot_start - mem_start) >= min_distance)
        {
            return TRUE;
        }
        if ((mem_end > (mboot_end + (ALIGN_1MB*2))) && ((mem_end - mboot_end - (ALIGN_1MB*2)) >= min_distance))
        {
            *pmem_start = ALIGN((mboot_end + ALIGN_1MB), ALIGN_1MB);
            UBOOT_DEBUG("Change memory_addr from 0x%lx to 0x%lx\n", mem_start, *pmem_start);
            return TRUE;
        }
    }
    else
    {
        if (mem_start < mboot_end)
        {
            if ((mem_end > (mboot_end + (ALIGN_1MB*2))) && ((mem_end - mboot_end - (ALIGN_1MB*2)) >= min_distance))
            {
                *pmem_start = ALIGN((mboot_end + ALIGN_1MB), ALIGN_1MB);
                UBOOT_DEBUG("Change memory_addr from 0x%lx to 0x%lx\n", mem_start, *pmem_start);
                return TRUE;
            }
        }
        if (mem_len >= min_distance)
        {
            return TRUE;
        }
    }

    UBOOT_TRACE("OK\n");
    return FALSE;
}
#endif

MS_BOOL get_large_memory(MS_PHY *pmem_addr, MS_U32 req_len)
{
#if 0 //csmao: mark up, seems useless.
    unsigned int i;
    MS_PHYADDR len  = 0;
    MS_PHYADDR addr = 0;
    /*UBOOT_MMAP_ID*/ u8 mpool[] = {/*E_LX_MEM*/1, /*E_LX_MEM2*/2, /*E_LX_MEM3*/3, /*E_LX_MEM4*/ };

    UBOOT_TRACE("IN\n");
    for (i = 0; i < ARRAY_SIZE(mpool); i++)
    {
        if (get_map_size_from_env(mpool[i], /*NO_DEFAULT_MMAP_VALUE*/0xFFFFFFFF, &len) != 0)
        {
            // get_lenght of mpool[i] failed
            continue;
        }
        if (len >= req_len)
        {
            if (get_map_addr_from_env(mpool[i], /*NO_DEFAULT_MMAP_VALUE*/0xFFFFFFFF, &addr) != 0)
            {
                // get_addr of mpool[i] failed
                continue;
            }
            if ((addr < 0xFFF00000) && (adjust_memory(&addr, len, req_len) == TRUE))
            {
                UBOOT_DEBUG("Select E_LX_MEM_%d as Large memory Buffer, request length :0x%X\n", i, (uint)req_len);
                *pmem_addr = (MS_U32)PA2BA(addr);
                UBOOT_TRACE("OK\n");
                return TRUE;
            }
        }
    }
    *pmem_addr = NULL;
    UBOOT_TRACE("OK\n");
    return FALSE;
#else
    return TRUE;
#endif
}

MS_BOOL get_demura_env_bypass_write_fs(void)
{
    char *cfg_bypass_write = env_get(DEMURA_ENV_BYPASS_WRITE_FS);
    if (cfg_bypass_write)
    {
        return TRUE;
    }
    return FALSE;
}

void set_demura_bin_type(MS_U8 u8type)/* EN_MS_UTIL_BIN_TYPE */
{
    gu8_demurabin_type = u8type;
}

MS_U8 get_demura_bin_type(void)
{
    return gu8_demurabin_type;
}

void set_demura_act(MS_U8 act)
{
    gu8_demurabin_act = act;
}

MS_U8 get_demura_act(void)
{
    return gu8_demurabin_act;
}

void set_demura_file(MS_U8 file)
{
    gu8_demurabin_file = file;
}

MS_U8 get_demura_file(void)
{
    return gu8_demurabin_file;
}

MS_BOOL get_demura_dlg_is_enable(void)
{
    st_cust_dmc_info st_cust_dmc_info;
    memset(&st_cust_dmc_info, 0x00, sizeof(st_cust_dmc_info));
    parse_dt("/video_out", cus_demura_dt_parser, (void*)&st_cust_dmc_info, NULL);
    UBOOT_TRACE("backlight demura is %s\n", st_cust_dmc_info.bl_dmc_enable ? "enable" : "disable");

    return st_cust_dmc_info.bl_dmc_enable;
}

void set_demura_block_size(MS_U8 file)
{
    gu8_demura_block_size = file;
}

MS_U8 get_demura_block_size(void)
{
    return gu8_demura_block_size;
}

