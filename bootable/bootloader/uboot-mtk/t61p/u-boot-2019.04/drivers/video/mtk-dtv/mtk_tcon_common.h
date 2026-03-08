// SPDX-License-Identifier: (GPL-2.0 OR BSD-3-Clause)
/*
 * MediaTek tcon common data tye and function
 *
 * Copyright (c) 2022 MediaTek Inc.
 */

#ifndef _MTK_TCON_COMMON_H_
#define _MTK_TCON_COMMON_H_

#if !defined(TRUE) && !defined(FALSE)
#define TRUE                        (1)
#define FALSE                       (0)
#endif
#define VERSION0                    (0x000)
#define VERSION1                    (0x001)
#define VERSION2                    (0x002)
#define VERSION3                    (0x003)
#define VERSION4                    (0x004)
#define VERSION5                    (0x005)
#define VERSION6                    (0x006)

//#define for TCON.BIN
#define TCON20_VERSION              (2)
#define SKIP_TCON_SUBNUM            (34)
#define SKIP_TCON_MAIMHEADER        (32)
#define SKIP_TCON_SUBHEADER         (8)
#define SKIP_TCON20_SUBHEADER       (40)
#define SKIP_TCON20_PANETYPE        (2)

#define TCON_BIN_VERSION_RGBW       (1)
#define TCON_BIN_HEADER_BYTE_NUBMER(byte_n) (byte_n)

#define MAX_DELAY_TIME              (10)
#define ALIGN_8(x)                  (((x) + 0x7) & (~0x7))

#define TCON_LOG_TAG                "<TCON>"
#define TCON_LOG_LEVEL              "TconLogLevel"

typedef enum
{
    EN_TCON_LOG_LEVEL_DISABLE = 0,
    EN_TCON_LOG_LEVEL_ERROR = 0x01,
    EN_TCON_LOG_LEVEL_INFO = 0x02,
    EN_TCON_LOG_LEVEL_DEBUG = 0x04,
    EN_TCON_LOG_LEVEL_INVALID = 0x1000,
    EN_TCON_LOG_LEVEL_MAX = EN_TCON_LOG_LEVEL_INVALID
}EN_TCON_LOG_LEVEL;

//define a marco with the tcon type of tcon tabs
#define LIST_OF_TCON_TABS \
    DEF_X(E_TCON_TAB_TYPE_GENERAL, 0, "General") \
    DEF_X(E_TCON_TAB_TYPE_GPIO, 1, "Gpio") \
    DEF_X(E_TCON_TAB_TYPE_SCALER, 2, "Scaler") \
    DEF_X(E_TCON_TAB_TYPE_MOD, 3, "Mod") \
    DEF_X(E_TCON_TAB_TYPE_GAMMA, 4, "Gamma") \
    DEF_X(E_TCON_TAB_TYPE_POWER_SEQUENCE_ON, 5, "Power Seq on") \
    DEF_X(E_TCON_TAB_TYPE_POWER_SEQUENCE_OFF, 6, "Power Seq off") \
    DEF_X(E_TCON_TAB_TYPE_PANEL_INFO, 7, "Panel info") \
    DEF_X(E_TCON_TAB_TYPE_OVERDRIVER, 8, "Over Driver") \
    DEF_X(E_TCON_TAB_TYPE_PCID, 9, "Pixel OD") \
    DEF_X(E_TCON_TAB_TYPE_PATCH, 10, "Patch") \
    DEF_X(E_TCON_TAB_TYPE_LINE_OD_TABLE, 11, "Line od table") \
    DEF_X(E_TCON_TAB_TYPE_LINE_OD_REG, 12, "Line od reg") \
    DEF_X(E_TCON_TAB_TYPE_VAC_REG, 13, "VAC reg") \
    DEF_X(E_TCON_TAB_TYPE_PMIC, 14, "PMIC") \
    DEF_X(E_TCON_TAB_TYPE_VAC_TABLE, 15, "VAC table") \
    DEF_X(E_TCON_TAB_TYPE_COUNT, 16, "Count") \
    DEF_X(E_TCON_TAB_TYPE_NULL, 17, "Null")

//create an enum of tcon tabs by marco expansion
typedef enum  {
    #define DEF_X(a, b, c) a,
    LIST_OF_TCON_TABS
    #undef DEF_X
}EN_TCON_TAB_TYPE;

typedef struct {
    uint8_t u8TconType;             //IN: tcon tab type
    uint8_t *pu8Table;              //IN & OUT: the pointer to tcon tab table
    uint8_t u8Version;              //OUT:
    uint16_t u16RegCount;           //OUT:
    uint8_t  u8RegType;             //OUT:
    uint32_t u32ReglistOffset;      //OUT:
    uint32_t u32ReglistSize;        //OUT:
    uint8_t u8PanelInterface;       //OUT:
}st_tcon_tab_info;

#define DEFAULT_TCON_LOG_LEVEL      (EN_TCON_LOG_LEVEL_ERROR)

extern EN_TCON_LOG_LEVEL g_enTconLogLevel;

#define TCON_ERROR(msg...)\
        do {\
            if(g_enTconLogLevel&EN_TCON_LOG_LEVEL_ERROR) \
            { \
                printf("\033[0;31m[ERROR]%s %s:%d: \033[0m", TCON_LOG_TAG, __FUNCTION__,__LINE__);\
                printf(msg);\
            } \
        } while(0)

#define TCON_INFO(msg...)\
        do {\
            if(g_enTconLogLevel&EN_TCON_LOG_LEVEL_INFO) \
            { \
                printf(msg);\
            } \
        } while(0)

#define TCON_DEBUG(msg...)\
        do {\
            if(g_enTconLogLevel&EN_TCON_LOG_LEVEL_DEBUG) \
            { \
                printf("\033[0;34m[DEBUG]%s %s:%d: \033[0m", TCON_LOG_TAG, __FUNCTION__, __LINE__);\
                printf(msg);\
            } \
        } while(0)


#define TCON_FUNC_ENTER() \
        do { \
            TCON_DEBUG(" >>>\n");\
        } while (0)

#define TCON_FUNC_EXIT(ret) \
        do { \
            TCON_DEBUG(" <<< ....'%s'\n", ret ? "OK" : "Fail");\
        } while (0)

#define TCON_FUNC_EXIT_ERR(fmt, args...) \
        do { \
            TCON_ERROR(" <<< "fmt, ##args);\
        } while (0)

#define TCON_CHECK_PARAMETER_NULL(p) \
        if (!p) { \
            TCON_ERROR(""#p" is NULL parameter\n"); \
            return FALSE; \
        }

#define TCON_CHECK_EQUAL_AND_ASSIGN(a, b)\
        if ((a) != (b)) {\
            TCON_DEBUG("%u -> %u:\n", (uint32_t)(a), (uint32_t)(b));\
            (a) = (b);\
        } else {\
            TCON_DEBUG("%u -> the same\n", (uint32_t)(a));\
        }

#define PNL_MALLOC_MEM(pu8Addr, size, bRet)       \
        pu8Addr = (void *) malloc(size); \
        if (pu8Addr == NULL) \
            { TCON_ERROR("malloc fail.\n"); \
               bRet &= FALSE;} \
        else  \
            { memset(pu8Addr, 0, size); \
               bRet &= TRUE;}

#define PNL_FREE_MEM(pu8Addr) \
        if (pu8Addr != NULL) \
            { free(pu8Addr); pu8Addr = NULL;}

bool init_log_level(void);
bool load_tcon_files(struct udevice *dev);
bool is_tcon_data_exist(unsigned char **ppdata, loff_t *plen);
bool is_tcon_power_seq_data_exist(unsigned char **ppdata, loff_t *plen);
bool is_pnlgamma_data_exist(unsigned char **ppdata, loff_t *plen);
bool get_tcon_dump_table(st_tcon_tab_info *pstInfo);
bool get_tcon_version(uint8_t *pu8Table, uint8_t *pu8Version);
bool free_resource(void);
bool is_tcon_sti_flow(void);
bool is_tcon_force_disable(void);
#endif
