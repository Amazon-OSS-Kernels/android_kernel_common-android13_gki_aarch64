/* SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause) */
/*
 * Copyright (c) 2023 MediaTek Inc.
*/
#ifndef _EFUSE_COMMON_H_
#define _EFUSE_COMMON_H_

#ifndef EFUSE_TEST_IN_PC
#define EFUSE_TEST_IN_PC    0
#endif

#include "debug_impl.h"

//=====================================================================================================================
#ifndef EFUSE_U8
    #define EFUSE_U8    unsigned char
#endif
#ifndef EFUSE_U16
    #define EFUSE_U16   unsigned short
#endif
#ifndef EFUSE_U32
    #define EFUSE_U32   unsigned int
#endif
#ifndef EFUSE_U64
    #define EFUSE_U64   unsigned long long
#endif
#ifndef EFUSE_BOOL
    #define EFUSE_BOOL  unsigned char
#endif


#ifndef FALSE
#define FALSE   0
#endif
#ifndef TRUE
#define TRUE    1
#endif

/*
#ifndef NULL
#define NULL ((void *)0)  // 0
#endif
*/
//=====================================================================================================================
#define EFUSE_TAG "[U][EFUSE]"

extern unsigned int u32_EFUSE_debug_flag;

#if(EFUSE_TEST_IN_PC)
#define EFUSE_PRINT(fmt, args...)       do{ UBOOT_INFO( fmt, ##args); } while(0)
#define EFUSE_ERR(fmt, args...)         do{ printf( "[ERR]" fmt, ##args); } while(0)
#else
#define EFUSE_PRINT(fmt, args...)       do{ UBOOT_INFO( fmt, ##args); } while(0)
#define EFUSE_ERR(fmt, args...)         do{ printf( "[ERR]" fmt, ##args); } while(0)
#endif

#define EFUSE_DEBUG(fmt, args...)       do{ if( u32_EFUSE_debug_flag&1 ) { EFUSE_PRINT(fmt, ##args); } } while(0)


//========================================================================================================================

#endif


