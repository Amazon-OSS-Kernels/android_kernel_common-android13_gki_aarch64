/* SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause) */
/*
 * Copyright (c) 2023 MediaTek Inc.
*/

#ifndef _MTK_DEBUG_HEADER_
#define _MTK_DEBUG_HEADER_

#define EBUSY 16
#define STR_ERROR "ERROR"
#define STR_INFO "INFO"
#define STR_TRACE "TRACE"
#define STR_DEBUG "DEBUG"
#define STR_BOOTTIME "BOOTTIME"
#define STR_UDTBDEBUG "UDTB"
#define STR_KDTBDEBUG "KDTB"
#define STR_DISABLE "DISABLE"

typedef enum
{
    EN_DEBUG_LEVEL_DISABLE=0,
    EN_DEBUG_LEVEL_ERROR=0x01,
    EN_DEBUG_LEVEL_INFO=0x02,
    EN_DEBUG_LEVEL_TRACE=0x04,
    EN_DEBUG_LEVEL_DEBUG=0x08,
    EN_DEBUG_LEVEL_BOOTTIME=0x10,
    EN_DEBUG_LEVEL_UDTBDEBUG=0x20,
    EN_DEBUG_LEVEL_KDTBDEBUG=0x40,
    EN_DEBUG_LEVEL_INVALID=0x1000,
    EN_DEBUG_LEVEL_MAX=EN_DEBUG_LEVEL_INVALID
}EN_DEBUG_LEVEL;

typedef enum
{
    EN_DEBUG_MODULE_DISABLE=0,
    EN_DEBUG_MODULE_UPGRADE=(0x01<<0),
    EN_DEBUG_MODULE_FAT=(0x01<<1),
    EN_DEBUG_MODULE_ALL=0xFFFFFFFF
}EN_DEBUG_MODULE;

#define ENV_DEBUG_LEVLE "dbgLevel"

extern EN_DEBUG_LEVEL dbgLevel;
extern EN_DEBUG_MODULE dbgModule;

#define DEFAULT_DEBUG_LEVEL (EN_DEBUG_LEVEL_ERROR)
#define DEFAULT_DEBUG_MODULE (EN_DEBUG_MODULE_ALL)

#define UBOOT_ERROR(msg...)\
    do{\
        if(dbgLevel&EN_DEBUG_LEVEL_ERROR) \
        { \
            printf("\033[0;31m[ERROR] %s:%d: \033[0m",__FUNCTION__,__LINE__);\
            printf(msg);\
        } \
    }while(0)

#define UBOOT_INFO(msg...)\
    do{\
        if(dbgLevel&EN_DEBUG_LEVEL_INFO) \
        { \
                printf(msg);\
        } \
    }while(0)

#define UBOOT_BOOTTIME(msg...)\
    do{\
        if(dbgLevel&EN_DEBUG_LEVEL_BOOTTIME) \
        { \
                printf(msg);\
        } \
    }while(0)

#define UBOOT_TRACE(msg...)\
    do{\
        if(dbgLevel&EN_DEBUG_LEVEL_TRACE) \
        { \
                printf("\033[0;32m[TRACE] %s \033[0m",__FUNCTION__);\
                printf(msg);\
        } \
    }while(0)

#define UBOOT_DEBUG(msg...)\
    do{\
        if(dbgLevel&EN_DEBUG_LEVEL_DEBUG) \
        { \
                printf("\033[0;34m[DEBUG] %s:%d: \033[0m",__FUNCTION__,__LINE__);\
                printf(msg);\
        } \
    }while(0)

#define UBOOT_DUMP(addr, size)\
    do{\
        if(dbgLevel&EN_DEBUG_LEVEL_DEBUG) \
        { \
                printf("\033[0;34m[DUMP] %s:%d:\033[0m\n",__FUNCTION__,__LINE__);\
                _dump((void *)addr,size);\
        } \
    }while(0)

#define UBOOT_ERRDUMP(addr, size)\
    do{\
        if(dbgLevel&EN_DEBUG_LEVEL_ERROR) \
        { \
                printf("\033[0;31m[DUMP] %s:%d:\033[0m\n",__FUNCTION__,__LINE__);\
                _dump((void *)addr,size);\
        } \
    }while(0)


int _init_debug_level(void);
void debug_dtb(unsigned long address, char *option);
void _dump(void *addr, unsigned int size);
#endif
