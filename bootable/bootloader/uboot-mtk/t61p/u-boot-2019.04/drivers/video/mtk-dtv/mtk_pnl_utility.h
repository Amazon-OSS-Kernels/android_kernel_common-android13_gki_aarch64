// SPDX-License-Identifier: (GPL-2.0 OR BSD-3-Clause)
/*
 * MediaTek Panel driver
 *
 * Copyright (c) 2020 MediaTek Inc.
 * Author Yu-Jen.Huang <yu-jen.huang@mediatek.com>
 */

#ifndef _MTK_PNL_UTILITY_H
#define _MTK_PNL_UTILITY_H

#define REG_BASE	(0x1C000000)
#define READ_BYTE(_reg)             (readb((const volatile void *)_reg))
#define WRITE_BYTE(_reg, _val)      { writeb(_val, (volatile void *)_reg); }
#define READ_WORD(_reg)             (readw((const volatile void *)_reg))
#define WRITE_WORD(_reg, _val)      { writew(_val, (volatile void *)_reg); }
#define RIU_READ_BYTE(addr)         ( READ_BYTE( REG_BASE + (addr) ) )
#define RIU_WRITE_BYTE(addr, val)   { WRITE_BYTE( REG_BASE + (addr), val) }
#define RIU_READ_2BYTE(addr)        ( READ_WORD( REG_BASE + (addr) ) )
#define RIU_WRITE_2BYTE(addr, val)  { WRITE_WORD( REG_BASE + (addr), val) }
#define TCON_W1BYTEMSK( u32Addr, u8Val, u8mask)     \
            ( { RIU_WRITE_BYTE( (((u32Addr) <<1) - ((u32Addr) & 1)), (RIU_READ_BYTE((((u32Addr) <<1) - ((u32Addr) & 1))) & ~(u8mask)) | ((u8Val) & (u8mask))); })
#define TCON_W2BYTEMSK( u32Reg, u16Val, u16Mask)\
            ( { RIU_WRITE_2BYTE( (u32Reg)<< 1 , (RIU_READ_2BYTE((u32Reg) << 1) & ~(u16Mask)) | ((u16Val) & (u16Mask)) ) ; } )

#define W2BYTEMSK(addr, value, fld) MTK_W2BYTEMSK(addr, value, fld)
#define R2BYTEMSK(addr, fld) MTK_R2BYTEMSK(addr, fld)

#ifndef _BIT
#define _BIT(_bit_)	(1<<(_bit_))
#endif

void MTK_W2BYTEMSK(uint32_t addr, uint32_t value, uint32_t fld);
uint32_t MTK_R2BYTEMSK(uint32_t addr, uint32_t fld);
#endif

