// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#ifndef _HWREG_UTILITY_H_
#define _HWREG_UTILITY_H_

extern unsigned long MS_RIU_MAP;

#define RIU_READ_BYTE(addr)         ( READ_BYTE( MS_RIU_MAP + (addr) ) )
#define RIU_READ_2BYTE(addr)        ( READ_WORD( MS_RIU_MAP + (addr) ) )
#define RIU_WRITE_BYTE(addr, val)   { WRITE_BYTE( MS_RIU_MAP + (addr), val) }
#define RIU_WRITE_2BYTE(addr, val)  { WRITE_WORD( MS_RIU_MAP + (addr), val) }

#define W2BYTE( u32Reg, u16Val)     RIU_WRITE_2BYTE( (u32Reg) << 1 , u16Val )
#define R2BYTE( u32Reg )            RIU_READ_2BYTE( (u32Reg) << 1)

#define R1BYTEMSK( u32Reg, u16mask)  \
            ( { RIU_READ_BYTE(((u32Reg) << 1) - ((u32Reg) & 1)) & u16mask ; } )

#define W1BYTEMSK( u32Addr, u8Val, u8mask)     \
            ( { RIU_WRITE_BYTE( (((u32Addr) <<1) - ((u32Addr) & 1)), (RIU_READ_BYTE((((u32Addr) <<1) - ((u32Addr) & 1))) & ~(u8mask)) | ((u8Val) & (u8mask))); })

#define R2BYTEMSK( u32Reg, u16mask)  \
            ( { RIU_READ_2BYTE( (u32Reg)<< 1) & u16mask ; } )

#define W2BYTEMSK( u32Reg, u16Val, u16Mask)  \
            ( { RIU_WRITE_2BYTE( (u32Reg)<< 1 , (RIU_READ_2BYTE((u32Reg) << 1) & ~(u16Mask)) | ((u16Val) & (u16Mask)) ) ; } )


#define BIT0        0x0001
#define BIT1        0x0002
#define BIT2        0x0004
#define BIT3        0x0008
#define BIT4        0x0010
#define BIT5        0x0020
#define BIT6        0x0040
#define BIT7        0x0080
#define BIT8        0x0100
#define BIT9        0x0200
#define BIT10       0x0400
#define BIT11       0x0800
#define BIT12       0x1000
#define BIT13       0x2000
#define BIT14       0x4000
#define BIT15       0x8000

#endif
