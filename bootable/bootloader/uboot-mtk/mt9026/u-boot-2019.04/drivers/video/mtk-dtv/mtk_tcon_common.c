// SPDX-License-Identifier: (GPL-2.0 OR BSD-3-Clause)
/*
 * MediaTek tcon common data tye and function
 *
 * Copyright (c) 2022 MediaTek Inc.
 */
#include <common.h>
#include <dm.h>
#include <utility.h>
#include <debug_impl.h>
#include <system_impl.h>
#include <dts_parser.h>
#include <asm/io.h>
#include <linux/string.h>

#include "mtk_tcon_common.h"
#include "mtk_tv_pnl.h"

//default tcon flow is sti or mix mode
#define DEFAULT_STI_FLOW            (TRUE)

//log level setting
EN_TCON_LOG_LEVEL g_enTconLogLevel = DEFAULT_TCON_LOG_LEVEL;

#define TCON_LOG_ERR                "ERROR"
#define TCON_LOG_INFO               "INFO"
#define TCON_LOG_DEBUG              "DEBUG"

//use BOE HV500QUB-F20 panel as the default tcon related file
#define FILE_CUS_PARTITION          "CusFilePart"
#define FILE_DEFAULT_PARTITION_1    "tvconfig"
#define FILE_DEFAULT_PARTITION_2    "bootdata"
#define FILE_FIXED_FOLDER           ""
#define TCON_FILE_PATH              "4k60/TCON20.bin"

#define PANELGAMMA_FILE_PATH        "4k60/gamma.bin"
#define TCON_HFR_FILE_PATH          "4k120/TCON20_HFR.bin"
#define TCON_HPC_FILE_PATH          "4k120/TCON20_HPC.bin"
#define TCON_GAME_FILE_PATH         "4k120/TCON20_GAME.bin"
#define TCON_POWER_ON_FILE_PATH     "4k120/TCON20_POST.bin"
#define FILE_PATH_LENGTH            (128)

//#define for TCON.BIN
#define TCON20_VERSION              (2)
#define SKIP_TCON_SUBNUM            (34)
#define SKIP_TCON_MAIMHEADER        (32)
#define SKIP_TCON_SUBHEADER         (8)
#define SKIP_TCON20_SUBHEADER       (40)
#define SKIP_TCON20_PANETYPE        (2)

#define TCON_REGISTERTYPE_1_BYTES   1
#define TCON_REGISTERTYPE_6_BYTES   6
#define TCON_REGISTERTYPE_4_BYTES   4
#define TCON_REGISTERTYPE_POWERSEQENCE_9_BYTES   9
#define TCON_REGISTERTYPE_POWERSEQENCE_7_BYTES   7
#define TCON_REGISTERTYPE_POWERSEQENCE_WITHSUB_15_BYTES   15
#define TCON_REGISTERTYPE_POWERSEQENCE_WITHSUB_11_BYTES   11
#define TCON_REGISTERTYPE_POWERSEQENCE_WITHSUB_14_BYTES   14

//store tcon related bin data to memory
static loff_t g_tcon_data_size = 0 ;
static unsigned char *g_tcon_data_buf = NULL;
static loff_t g_pnlgamma_data_size = 0 ;
static unsigned char *g_pnlgamma_data_buf = NULL;

/* pointer to the tcon power seq. on/off bin data */
static loff_t g_tcon_power_on_data_size = 0;
static unsigned char *g_tcon_power_on_data_buf = NULL;

typedef enum {
    /* input vfreq <= default max refresh rate */
    EN_TCON_MODE_DEFAULT = 0,
    /* high frame rate, ex: default max refresh rate < input vfreq <= HFR max refresh rate */
    EN_TCON_MODE_HFR,
    /* high pixel colck, ex: input freq = 144hz */
    EN_TCON_MODE_HPC,
    /* game, ex: direct 60hz */
    EN_TCON_MODE_GAME,
    EN_TCON_MODE_MAX
} EN_TCON_MODE;

// TCon register type
typedef enum
{
    // 32-bit address, 4+1+1, address+mask+value
    EN_TCON20_REGISTERTYPE_6_BYTES = 0,
    // 16-bit address, 2+1+1, address+mask+value
    EN_TCON20_REGISTERTYPE_4_BYTES,
    // 32-bit address, 4+1+1+1+1+1, address+mask+value+delayReady+delayTime+signalType
    EN_TCON20_REGISTERTYPE_POWERSEQENCE_9_BYTES,
    // 16-bit address, 2+1+1+1+1+1, address+mask+value+delayReady+delayTime+signalType
    EN_TCON20_REGISTERTYPE_POWERSEQENCE_7_BYTES,
    // 32-bit address, 4+1+1+4+1+1+1+1+1, subAddress+subMask+subValue+address+mask+value+delayReady+delayTime+signalType
    EN_TCON20_REGISTERTYPE_POWERSEQENCE_WITHSUB_15_BYTES,
    // 16-bit address, 2+1+1+2+1+1+1+1+1, subAddress+subMask+subValue+address+mask+value+delayReady+delayTime+signalType
    EN_TCON20_REGISTERTYPE_POWERSEQENCE_WITHSUB_11_BYTES,
    // Register type 6: P-Gamma (Power Gamma) 4-byte register format (16-bit address size and 16-bit value size)
    EN_TCON20_REGISTERTYPE_POWERGAMMA_4_BYTES,
    // Register type 7: TCON 3-byte register format (12-bit address size)
    EN_TCON20_REGISTERTYPE_3_BYTES,
    // Register type 8: 1-byte only data.
    EN_TCON20_REGISTERTYPE_PANEL_1_BYTES,
    //Register tyep 9: 2+4, panelinfo ID+value
    EN_TCON20_REGISTERTYPE_PANEL_6_BYTES,
    // Register type A : 3+1+1+3+2+2+1+1 subbankaddress + mask+subbank+adress+mask+value+delaytime+prioriry
    EN_TCON20_REGISTERTYPE_POWERSEQUENCE_14BYTES,
    //Register tyep 9: 2+4, panelinfo ID+value, for new SSC calculation
    EN_TCON20_REGISTERTYPE_PANEL_6_BYTES_NEWSSC,
    EN_TCON20_REGISTERTYPE_END,
} E_TCON20_REGISTERTYPE;

int _my_atoi(const char *src)
{
    int s = 0;
    bool isMinus = FALSE;

    while (*src == ' ')
        src++;

    if (*src == '+' || *src == '-')
    {
        if(*src == '-')
            isMinus = TRUE;
        src++;
    }
    else if (*src < '0' || *src > '9')
    {
        s = 111111111;
        return s;
    }

    while (*src != '\0' && *src >= '0' && *src <= '9')
    {
        s = s*10 + *src - '0';
        src++;
    }

    return s * (isMinus ? -1 : 1);
}

int _get_tcon_mode(struct mtk_panel_priv *priv)
{
    EN_TCON_MODE enMode = EN_TCON_MODE_DEFAULT;
    char *pEnv = env_get("TconMode");
    if (pEnv) {
        enMode = (EN_TCON_MODE)_my_atoi(pEnv);
    } else {

        if (!priv) {
            TCON_ERROR("input parmeter is null\n");
            return enMode;
        }

        if (priv->dlg_on)
            enMode = EN_TCON_MODE_HFR;
        else if (priv->hpc_mode_en)
            enMode = EN_TCON_MODE_HPC;

        TCON_DEBUG("dlg_on=%d, hpc_mode_en=%d, game_direct_fr_group=%d\n",
                    priv->dlg_on,
                    priv->hpc_mode_en,
                    priv->game_direct_fr_group);
    }

    return enMode;
}

uint16_t _regTypeToSize(uint8_t u8Registertype)
{
    E_TCON20_REGISTERTYPE en = (E_TCON20_REGISTERTYPE)u8Registertype;
    switch (en)
    {
        case EN_TCON20_REGISTERTYPE_6_BYTES:
            return (uint16_t)TCON_REGISTERTYPE_6_BYTES;
        case EN_TCON20_REGISTERTYPE_4_BYTES:
            return (uint16_t)TCON_REGISTERTYPE_4_BYTES;
        case EN_TCON20_REGISTERTYPE_POWERSEQENCE_9_BYTES:
            return (uint16_t)TCON_REGISTERTYPE_POWERSEQENCE_9_BYTES;
        case EN_TCON20_REGISTERTYPE_POWERSEQENCE_7_BYTES:
            return (uint16_t)TCON_REGISTERTYPE_POWERSEQENCE_7_BYTES;
        case EN_TCON20_REGISTERTYPE_POWERSEQENCE_WITHSUB_15_BYTES:
            return (uint16_t)TCON_REGISTERTYPE_POWERSEQENCE_WITHSUB_15_BYTES;
        case EN_TCON20_REGISTERTYPE_POWERSEQENCE_WITHSUB_11_BYTES:
            return (uint16_t)TCON_REGISTERTYPE_POWERSEQENCE_WITHSUB_11_BYTES;
        case EN_TCON20_REGISTERTYPE_PANEL_1_BYTES:
            return(uint16_t) TCON_REGISTERTYPE_1_BYTES;
        case EN_TCON20_REGISTERTYPE_PANEL_6_BYTES:
        case EN_TCON20_REGISTERTYPE_PANEL_6_BYTES_NEWSSC:
            return (uint16_t)TCON_REGISTERTYPE_6_BYTES;
        case EN_TCON20_REGISTERTYPE_POWERSEQUENCE_14BYTES:
            return (uint16_t)TCON_REGISTERTYPE_POWERSEQENCE_WITHSUB_14_BYTES;
        default:
            TCON_ERROR("register type out of range: %d", en);
            return 0;
    }
}

bool _get_file_data(
                const char *default_path, const char *path,
                unsigned char **ppData, loff_t *pSize)
{
    // search order of binary file partition:
    //   1. uboot env: ${FILE_CUS_PARTITION}
    //   2. ${FILE_DEFAULT_PARTITION_1}
    //   3. ${FILE_DEFAULT_PARTITION_2}
    static char *chPartList[] = {
        NULL,
        FILE_DEFAULT_PARTITION_1,
        FILE_DEFAULT_PARTITION_2
    };
    bool bRet = FALSE;
    int i;
    size_t len;
    char chFilePath[FILE_PATH_LENGTH];
    const char *pchPath;

    TCON_CHECK_PARAMETER_NULL(ppData);
    TCON_CHECK_PARAMETER_NULL(default_path);
    TCON_CHECK_PARAMETER_NULL(path);
    TCON_CHECK_PARAMETER_NULL(pSize);

    memset(chFilePath, '\0', sizeof(chFilePath));
    *pSize = 0;

    //load bin
    pchPath = (path[0] != '\0') ? path : default_path;

    len = strlen(FILE_FIXED_FOLDER) + strlen(pchPath) + 1;
    if (len >= sizeof(chFilePath)) {
        TCON_ERROR(
            "Warning: The length of the path is %d dytes\n"
            "more than the maximum file length supported\n"
            "by the system is %d bytes.\n",
            (int)len, (int)sizeof(chFilePath));
        return FALSE;
    }

    len = sizeof(chFilePath) - 1;
    strncat(chFilePath, FILE_FIXED_FOLDER, len);
    len = len - strlen(FILE_FIXED_FOLDER);
    strncat(chFilePath, pchPath, len);
    TCON_INFO("Use %s path '%s'\n",
                (path[0] != '\0') ? "customized" : "default",
                chFilePath);

    //select partition
    chPartList[0] = env_get(FILE_CUS_PARTITION);
    for (i = 0; i < ARRAY_SIZE(chPartList); i++)
    {
        if (!chPartList[i])
        {
            TCON_DEBUG("Partition list [%d] is null\n", i);
            continue;
        }

        *ppData = read_storage_file_to_memory(chPartList[i], chFilePath, pSize);

        if (*ppData && pSize > 0)
        {
            TCON_DEBUG("File found. Path='%s/%s'\n", chPartList[i], chFilePath);
            bRet = TRUE;
            break;
        }
        else
        {
            TCON_DEBUG("File not found. Path='%s/%s'\n", chPartList[i], chFilePath);
        }
    }

    if (!bRet)
        TCON_DEBUG("cannot find '%s'\n", chFilePath);

    return bRet;
}

bool init_log_level(void)
{
    char *pEnv = env_get(TCON_LOG_LEVEL);
    g_enTconLogLevel = DEFAULT_TCON_LOG_LEVEL;

    if (pEnv)
    {
        if (strcmp(pEnv, TCON_LOG_INFO) == 0)
        {
            g_enTconLogLevel |= (EN_TCON_LOG_LEVEL_INFO | EN_TCON_LOG_LEVEL_DEBUG);
        }
        else if (strcmp(pEnv, TCON_LOG_DEBUG) == 0)
        {
            g_enTconLogLevel |= EN_TCON_LOG_LEVEL_DEBUG;
        }
    }

    TCON_INFO("g_enTconLogLevel=0x%X\n", g_enTconLogLevel);

    return TRUE;
}

bool load_tcon_files(struct udevice *dev)
{
    struct mtk_panel_priv *priv = dev_get_priv(dev);
    EN_TCON_MODE enMode = EN_TCON_MODE_DEFAULT;
    const char *bin_path;

    TCON_CHECK_PARAMETER_NULL(priv);

    enMode = _get_tcon_mode(priv);
    TCON_DEBUG("tcon mode=%d\n", enMode);

    //load tcon bin
    if (!g_tcon_data_buf)
    {
        if (enMode == EN_TCON_MODE_HFR)
            bin_path = priv->tcon_info.higt_frame_rate_bin_path;
        else if (enMode == EN_TCON_MODE_HPC)
            bin_path = priv->tcon_info.higt_pixel_clock_bin_path;
        else if (enMode == EN_TCON_MODE_GAME)
            bin_path = priv->tcon_info.game_mode_bin_path;
        else
            bin_path = priv->tcon_info.tcon_bin_path;

        _get_file_data(
                priv->tcon_info.tcon_bin_path, bin_path,
                &g_tcon_data_buf, &g_tcon_data_size);

        TCON_DEBUG("tcon buffer size=%lld\n", g_tcon_data_size);
        if (g_tcon_data_buf && g_tcon_data_size > 1)
        {
            TCON_DEBUG("tcon buffer[0]=0x%X [1]=0x%X\n", g_tcon_data_buf[0], g_tcon_data_buf[1]);
        }
        else
        {
            TCON_DEBUG("File(%s) not found. Load default tcon bin.\n", bin_path);
            enMode = EN_TCON_MODE_DEFAULT;

             _get_file_data(
                TCON_FILE_PATH, priv->tcon_info.tcon_bin_path,
                &g_tcon_data_buf, &g_tcon_data_size);

            if (g_tcon_data_buf && g_tcon_data_size > 1)
            {
                TCON_DEBUG("tcon buffer[0]=0x%X [1]=0x%X\n",
                            g_tcon_data_buf[0], g_tcon_data_buf[1]);
            }
        }
    }

    //load power on bin
    if (!g_tcon_power_on_data_buf)
    {
        _get_file_data(
                TCON_POWER_ON_FILE_PATH, priv->tcon_info.power_seq_on_bin_path,
                &g_tcon_power_on_data_buf, &g_tcon_power_on_data_size);

        TCON_DEBUG("power on buffer size=%lld\n", g_tcon_power_on_data_size);
        if (g_tcon_power_on_data_buf && g_tcon_power_on_data_size > 1)
        {
            TCON_DEBUG("power on buffer[0]=0x%X [1]=0x%X\n",
                    g_tcon_power_on_data_buf[0], g_tcon_power_on_data_buf[1]);
        }
    }

    //load panel gamma bin
    if (priv->tcon_info.bPanelGammaEn)
    {
        if (enMode == EN_TCON_MODE_HFR)
            bin_path = priv->tcon_info.higt_frame_rate_gamma_bin_path;
        else if (enMode == EN_TCON_MODE_HPC)
            bin_path = priv->tcon_info.higt_pixel_clock_gamma_bin_path;
        else if (enMode == EN_TCON_MODE_GAME)
            bin_path = priv->tcon_info.game_mode_gamma_bin_path;
        else
            bin_path = priv->tcon_info.panelgamma_bin_path;

        _get_file_data(
                    priv->tcon_info.panelgamma_bin_path, bin_path,
                    &g_pnlgamma_data_buf, &g_pnlgamma_data_size);

        TCON_DEBUG("pnlgamma data buffer size=%lld\n", g_pnlgamma_data_size);
    }
    else
    {
        TCON_DEBUG("Not support panel gamma function.\n");
    }
    return TRUE;
}

bool load_tcon_pq_files(struct udevice *dev, struct st_tcon_pq_force_en force_en)
{
	if (force_en.force_enable && force_en.tcon_pq_bin_path) {
		TCON_DEBUG("Load default tcon pq = %s.\n", force_en.tcon_pq_bin_path);
		_get_file_data(force_en.tcon_pq_bin_path, force_en.tcon_pq_bin_path, &g_tcon_data_buf, &g_tcon_data_size);
		TCON_DEBUG("tcon pq buffer size=%lld\n", g_tcon_data_size);
	}
	if (force_en.force_enable && force_en.pga_bin_path) {
		_get_file_data(force_en.pga_bin_path, force_en.pga_bin_path, &g_pnlgamma_data_buf, &g_pnlgamma_data_size);
		TCON_DEBUG("panel gamma data buffer size=%lld\n", g_pnlgamma_data_size);
	}
	return true;
}

bool is_tcon_data_exist(unsigned char **ppdata, loff_t *plen)
{
    bool bExist = (g_tcon_data_buf && g_tcon_data_size > 0) ? TRUE : FALSE;
    if (ppdata)
    {
        *ppdata = g_tcon_data_buf;
    }
    if (plen)
    {
        *plen =  g_tcon_data_size;
    }
    return bExist;
}

bool is_tcon_power_seq_data_exist(unsigned char **ppdata, loff_t *plen)
{
    bool bExist = (g_tcon_power_on_data_buf && g_tcon_power_on_data_size > 0) ? TRUE : FALSE;
    if (ppdata)
        *ppdata = g_tcon_power_on_data_buf;

    if (plen)
        *plen = g_tcon_power_on_data_size;

    return bExist;
}

bool is_pnlgamma_data_exist(unsigned char **ppdata, loff_t *plen)
{
    bool bExist = (g_pnlgamma_data_buf && g_pnlgamma_data_size > 0) ? TRUE : FALSE;
    if (ppdata)
    {
        *ppdata = g_pnlgamma_data_buf;
    }
    if (plen)
    {
        *plen =  g_pnlgamma_data_size;
    }
    return bExist;
}

const char* _tabidx_to_string(uint8_t u8Idx)
{
    uint8_t u8TableNum = E_TCON_TAB_TYPE_COUNT;

    const char *pTable[] =
    {
        #define DEF_X(a, b, c) c,
        LIST_OF_TCON_TABS
        #undef DEF_X
    };

    TCON_INFO("Table Num=%d Idx=%d\n", u8TableNum, u8Idx);

    return (u8Idx < u8TableNum) ? pTable[u8Idx] : "Null";
}

bool get_tcon_version(uint8_t *pu8Table, uint8_t *pu8Version)
{
    TCON_CHECK_PARAMETER_NULL(pu8Table);
    TCON_CHECK_PARAMETER_NULL(pu8Version);
    *pu8Version = *((uint8_t*)(pu8Table));

    return TRUE;
}

bool get_tcon_dump_table(st_tcon_tab_info *pstInfo)
{
    uint8_t *pu8Table = NULL;
    uint8_t u8TableType = 0;
    uint16_t u16RegisterCount = 0;
    uint8_t  u8RegisterType = 0;
    uint32_t u32RegisterlistOffset = 0;
    uint32_t u32RegisterlistSize = 0;
    uint8_t u8PanelInterface = 0;
    uint8_t u8TconType = 0;
    uint8_t u8version = 0;
    bool bRet = TRUE;

    TCON_CHECK_PARAMETER_NULL(pstInfo);

    pu8Table = pstInfo->pu8Table;
    u8TconType = pstInfo->u8TconType;

    TCON_CHECK_PARAMETER_NULL(pu8Table);

    u8version = *((uint8_t*)(pu8Table));

    pu8Table += SKIP_TCON_MAIMHEADER;  // skip 32 bytes main header
    pu8Table += SKIP_TCON_SUBHEADER;   // skip 8 byte sub header

    if (u8version > TCON20_VERSION)
    {
        pu8Table += SKIP_TCON20_SUBHEADER; // skip 8byte TCON2.0 subheader
        u8PanelInterface = *((uint8_t*)(pu8Table + SKIP_TCON20_PANETYPE)); //get panel Interface
    }
    // sn MAY NEED version , so there needs a way to pass this info.
    u8TableType           = *((uint8_t*)(pu8Table + (u8TconType * 8) + 0));
    u16RegisterCount      = ((*((uint8_t*)(pu8Table + (u8TconType * 8) + 2)))<<8) |
                               *((uint8_t*)(pu8Table + (u8TconType * 8) + 1));
    u8RegisterType        = *((uint8_t*)(pu8Table + (u8TconType * 8) + 3));
    u32RegisterlistOffset = ((*((uint8_t*)(pu8Table + (u8TconType * 8) + 7)))<<24) |
                            ((*((uint8_t*)(pu8Table + (u8TconType * 8) + 6)))<<16) |
                            ((*((uint8_t*)(pu8Table + (u8TconType * 8) + 5)))<<8) |
                               *((uint8_t*)(pu8Table + (u8TconType * 8) + 4));
    u32RegisterlistSize   = u16RegisterCount * _regTypeToSize(u8RegisterType);

    //error handling
    if ((u8TableType != u8TconType) || (u16RegisterCount == 0))
    {
        pu8Table -= (SKIP_TCON_MAIMHEADER + SKIP_TCON_SUBHEADER);
        if (u8version > TCON20_VERSION)
        {
            pu8Table -= SKIP_TCON20_SUBHEADER;
        }
        bRet = FALSE;
    }
    else
    {
        if (u8version > TCON20_VERSION)
        {
            pu8Table += (u32RegisterlistOffset - (SKIP_TCON_MAIMHEADER + SKIP_TCON_SUBHEADER + SKIP_TCON20_SUBHEADER));
        }
        else
        {
            pu8Table += (u32RegisterlistOffset - (SKIP_TCON_MAIMHEADER + SKIP_TCON_SUBHEADER));
        }
    }

    pstInfo->pu8Table = pu8Table;
    pstInfo->u8Version = u8version;
    pstInfo->u16RegCount = u16RegisterCount;
    pstInfo->u8RegType = u8RegisterType;
    pstInfo->u32ReglistOffset = u32RegisterlistOffset;
    pstInfo->u32ReglistSize = u32RegisterlistSize;
    pstInfo->u8PanelInterface = u8PanelInterface;

    TCON_DEBUG("\033[1;32mFind '%s' table [%s]\033[0m\n",
            _tabidx_to_string(u8TconType), bRet ? "OK" : "Not Found");
    TCON_DEBUG("Current Type='%s' Version=%d PanelInterface=%d "\
            "RegisterCount=%d RegisterType=%d RegisterlistOffset=%d RegisterlistSize=%d\n", \
            _tabidx_to_string(u8TableType), \
            pstInfo->u8Version, pstInfo->u8PanelInterface, \
            pstInfo->u16RegCount, pstInfo->u8RegType, pstInfo->u32ReglistOffset, \
            pstInfo->u32ReglistSize);

    return bRet;
}

bool free_resource(void)
{
    PNL_FREE_MEM(g_tcon_data_buf);
    PNL_FREE_MEM(g_tcon_power_on_data_buf);
    PNL_FREE_MEM(g_pnlgamma_data_buf);

    return TRUE;
}

bool is_tcon_sti_flow(void)
{
    bool bEnable = DEFAULT_STI_FLOW;
    char *pEnv = env_get("TconStiEnable");
    if (pEnv)
    {
        bEnable = (bool)_my_atoi(pEnv);
    }

    return bEnable;
}

bool is_tcon_force_disable(void)
{
    bool bDisable = FALSE;
    char *pEnv = env_get("TconForceDisable");
    if (pEnv)
    {
        bDisable = (bool)_my_atoi(pEnv);
    }

    return bDisable;
}

bool is_tcon_pq_force_enable(struct st_tcon_pq_force_en *tcon_pq_en)
{
	char *env_val = env_get("tcon_pq_en");

	if (env_val && tcon_pq_en) {
		tcon_pq_en->force_enable = (uint)_my_atoi(env_val);
		tcon_pq_en->tcon_pq_bin_path = env_get("tcon_pq_bin");
		tcon_pq_en->eva_bin_path = env_get("tcon_eva_bin");
		tcon_pq_en->pga_bin_path = env_get("tcon_pga_bin");
		TCON_DEBUG("force_enable=%d\n", tcon_pq_en->force_enable);
	} else {
		return false;
	}
	if (tcon_pq_en->force_enable &&
	    (tcon_pq_en->tcon_pq_bin_path ||
	     tcon_pq_en->eva_bin_path ||
	     tcon_pq_en->pga_bin_path))
		return true;
	else
		return false;
}

void print_look_up_table(int rows, int columns, u16 *table, bool is_byte)
{
	int i, j;
	u8 *table_is_byte = NULL;

	if (!table)
		return;
	if (is_byte)
		table_is_byte = (u8 *)table;

	TCON_INFO("LUT %d * %d start:\n", rows, columns);
	for (i = 0; i < rows; i++) {
		for (j = 0; j < columns; j++) {
		if (is_byte)
			TCON_INFO("%d\t", table_is_byte[i * columns + j]);
		else
			TCON_INFO("%d\t", table[i * columns + j]);
	}
		TCON_INFO("\n");
	}
	TCON_INFO("\n");
	TCON_INFO("LUT end.\n");
}

