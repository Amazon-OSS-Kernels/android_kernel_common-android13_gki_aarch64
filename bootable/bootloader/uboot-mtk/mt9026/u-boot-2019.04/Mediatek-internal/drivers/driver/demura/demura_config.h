// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#ifndef _DEMURA_CONFIG_H_
#define _DEMURA_CONFIG_H_

#define DEMURA_PARTITION_NAME       CONFIG_DEMURA_BIN_PARTITION
#define DEMURA_FILE_FOLDER          "/demura"
#define DEMURA_VENDOR_BIN_PATH      "/demura/demura_panel.bin"
#define DEMURA_BIN_FILE_PATH        CONFIG_DEMURA_BIN_PATH
#define DEMURA_BIN_FILE_PATH_DLG    CONFIG_DEMURA_BIN_PATH_DLG
#define DEMURA_HEADER_FILE_PATH     "/demura/demura_header.bin"
#define DEMURA_MMAP_NODE            "/mmap_info/MI_DISPOUT_DEMURA"
#define DEMURA_MMAP_KEYWORD         "reg"
#define DEMURA_VIDEO_OUT_NODE       "/video_out"
#define DEMURA_PNL_KEYWORD          "pnl_lib_version"

#define DEMURA_ENV_ENABLE           "dmc_enable"
#define DEMURA_ENV_PARTITION        "dmc_partition"
#define DEMURA_ENV_VENDORID         "dmc_vendorid"
#define DEMURA_ENV_PARTITION_BACKLIGHT         "dmc_partition_backlight"
#define DEMURA_ENV_VENDORID_BACKLIGHT          "dmc_vendorid_backlight"
#define DEMURA_ENV_STORE_BIN        "dmc_store_bin"
#define DEMURA_ENV_BYPASS_SPI       "dmc_bypass_spi"
#define DEMURA_ENV_DITHER           "dmc_dither"
#define DEMURA_ENV_MTK_BIN          "dmc_file"
#define DEMURA_ENV_VENDOR_BIN       "dmc_vendor_bin"
#define DEMURA_ENV_VENDOR_BACKLIGHT_BIN       "dmc_vendor_backlight_bin"
#define DEMURA_ENV_MEM              "dmc_mem"
#define DEMURA_ENV_VERSION          "dmc_ver"
#define DEMURA_ENV_BYPASS_WRITE_FS  "dmc_bypass_wfs"
#define DEMURA_ENV_PNL_H            "dmc_pnlh"
#define DEMURA_ENV_PNL_V            "dmc_pnlv"

#define DEMURA_ENV_ENABLE_INVALID       (0xFF)
#define DEMURA_ENV_ENABLE_TRUE          (0x1)
#define SPI_CLOCLK                      (5 * 1000 * 1000)   // 5MHz
#define SPI_MODE                         0
#define SPI_CH				             1

#define ENV_DEMURA_IP                    "demura_ip"
#define ENV_DEMURA_SIG                   "demura_sig"
#define ENV_DEMURA_SIG_BL                "demura_sig_bl"

#define ENV_DEMURA_MSTAR_CRC             "demura_ms_crc"
#define ENV_DEMURA_MSTAR_CRC_POS         "demura_ms_crc_pos"

#define ENV_DEMURA_IN_MBOOT              "demura_in_mboot"

#define DEMURA_MMAP_ID                   "E_MMAP_ID_DEMURA"
#define DEMURA_MMAP_ID_MI                "MI_DISPOUT_DEMURA"

#ifdef CONFIG_DEMURA_URSA11
    #define CONVERT_BUFFER_SIZE           (72 * 1024 * 1024)   // 72MB
#else
    #define CONVERT_BUFFER_SIZE           (100 * 1024 * 1024)   // 100MB
#endif

#define HW_CRC32_04C11DB7                  0

#ifdef CONFIG_DEMURA_VENDOR_LGD
    #define CONFIG_DEMURA_LGD_DEMO       1
#endif

#ifdef CONFIG_DEMURA_VENDOR_MULTI
    #define CONFIG_DEMURA_VENDOR_MULTI_EN   1
    #define CONFIG_DEMURA_LGD_DEMO          1
    #define NOVA_Shift                      0x00
    #define H_K_C_NOVA_Shift                  0x1000
    #define H_K_C_NOVA_Shift_120HZ            0x2000
#endif

#define CONFIG_DEMURA_WITHOUT_MMC_PART    0


#define CONFIG_DEMURA_ENV_SAVEENV 1

#define CONFIG_DEMURA_VENDOR_BACKLIGHT

enum en_backlight_levelcount
{
    BL_LEVEL_CNT_0,
    BL_LEVEL_CNT_1,
    BL_LEVEL_CNT_2,
    BL_LEVEL_CNT_3,
    BL_LEVEL_CNT_4,
    BL_LEVEL_CNT_5,
    BL_LEVEL_CNT_6,
    BL_LEVEL_CNT_7,
};

#endif
