/* SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause) */
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

///////////////////////////////////////////////////////////////////////////////////////////////////
/// @file   apiGFX.h
/// @brief  MStar Graphics API
/// @author MStar Semiconductor Inc.
/// @attention
/// <b>(OBSOLETED) <em>legacy interface is only used by MStar proprietary OSD\n
/// It's API level for backward compatible and will be remove in the next version.\n
/// Please refer @ref drvGE.h for future compatibility.</em></b>
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _API_GFX_TYPE_H_
#define _API_GFX_TYPE_H_

//#include "drvGE.h"

//#define MAX_FONT                    20  //5
//#define MAX_BITMAP                  127
typedef MS_S8                       FONTHANDLE;      ///< Font handle
typedef MS_S16                  BMPHANDLE;       ///< Bitmap handle
typedef MS_S8                       DBHANDLE;



#if !defined(BIT0) && !defined(BIT1)
#define BIT0                        0x00000001
#define BIT1                        0x00000002
#define BIT2                        0x00000004
#define BIT3                        0x00000008
#define BIT4                        0x00000010
#define BIT5                        0x00000020
#define BIT6                        0x00000040
#define BIT7                        0x00000080
#define BIT8                        0x00000100
#define BIT9                        0x00000200
#define BIT10                       0x00000400
#define BIT11                       0x00000800
#define BIT12                       0x00001000
#define BIT13                       0x00002000
#define BIT14                       0x00004000
#define BIT15                       0x00008000
#define BIT16                       0x00010000
#define BIT17                       0x00020000
#define BIT18                       0x00040000
#define BIT19                       0x00080000
#define BIT20                       0x00100000
#define BIT21                       0x00200000
#define BIT22                       0x00400000
#define BIT23                       0x00800000
#define BIT24                       0x01000000
#define BIT25                       0x02000000
#define BIT26                       0x04000000
#define BIT27                       0x08000000
#define BIT28                       0x10000000
#define BIT29                       0x20000000
#define BIT30                       0x40000000
#define BIT31                       0x80000000
#endif



#endif //_API_GFX_TYPE_H_

