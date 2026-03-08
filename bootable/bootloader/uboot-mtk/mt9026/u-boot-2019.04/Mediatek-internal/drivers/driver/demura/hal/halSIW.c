// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#include <linux/io.h>
#include <command.h>
#include <stdio.h>
#include <common.h>
#include <debug_impl.h>
#include <MsTypes.h>
#include <halRegOp.h>
#include "halSIW.h"
#include "LXS_DEMURA_BKA3CD.h"
#include "TCON_ADL_CRC_BKA369.h"
#include "../SIW/MDrvSIW.h"
#define READ_BYTE(_reg)             ( readb((const volatile void *)((uintptr_t)(_reg))))
#define WRITE_BYTE(_reg, _val)      { writeb(_val, (volatile void *)((uintptr_t)(_reg))); }
#define READ_WORD(_reg)             ( readl((const volatile void *)((uintptr_t)(_reg))))
#define WRITE_WORD(_reg, _val)      { writel(_val, (volatile void *)((uintptr_t)(_reg))); }
#define RIU_READ_BYTE(addr)         ( READ_BYTE( REG_BASE + (addr) ) )
#define RIU_WRITE_BYTE(addr, val)   { WRITE_BYTE( REG_BASE + (addr), val) }
#define RIU_READ_2BYTE(addr)        ( READ_WORD( REG_BASE + (addr) ) )
#define RIU_WRITE_2BYTE(addr, val)  { WRITE_WORD( REG_BASE + (addr), val) }

#define SC_W1BYTEMSK( u32Addr, u8Val, u8mask) \
    ({RIU_WRITE_BYTE((u32Addr), ((RIU_READ_BYTE(u32Addr)&~(u8mask)) | ((u8Val)&(u8mask))));})
#define SC_R2BYTE( u32Id, u32Reg ) RIU_READ_2BYTE( u32Reg )
#define SC_R2BYTEMSK( u32Id, u32Reg, u16mask) (RIU_READ_2BYTE(u32Reg) & (u16mask))
#define SC_W2BYTEMSK( u32Id, u32Reg, u16Val, u16Mask) \
    ({RIU_WRITE_2BYTE((u32Reg), (RIU_READ_2BYTE((u32Reg))&~(u16Mask)) | ((u16Val)&(u16Mask)));})
#define SC_W2BYTE( u32Id, u32Reg, u16Val) RIU_WRITE_2BYTE( u32Reg, u16Val)

void HAL_SIW_Demura_Big_Endian(MS_BOOL bBigEndian)
{
    SC_W2BYTEMSK(0,
        (MS_U32)REG_0008_LXS_DEMURA_BKA3CD,
        (MS_U16)(bBigEndian << 4),
        (MS_U16)REG_0008_LXS_DEMURA_BKA3CD_REG_BIG_ENDIAN_SEL_0008);
}

void HAL_SIW_Demura_Enable_DL(MS_BOOL bEnable)
{
    // BKA369 h58[0][1]reg_clinetn_en & reg_clientn_trig_mode
    SC_W2BYTEMSK(0,
        (MS_U32)REG_0160_TCON_ADL_CRC_BKA369,
        (MS_U16)bEnable,
        (MS_U16)REG_0160_TCON_ADL_CRC_BKA369_REG_CLIENTN_EN_0160);

    SC_W2BYTEMSK(0,
        (MS_U32)REG_0160_TCON_ADL_CRC_BKA369,
        (MS_U16)bEnable << 1,
        (MS_U16)REG_0160_TCON_ADL_CRC_BKA369_REG_CLIENTN_TRIG_MODE_0160);
}

void HAL_SIW_Demura_SetDL_Depth(void)
{
    // dont need to set
    SC_W2BYTEMSK(0,
        (MS_U32)REG_016C_TCON_ADL_CRC_BKA369,
        0,
        (MS_U16)REG_016C_TCON_ADL_CRC_BKA369_REG_CLIENTN_DEPTH_016C);
}


void HAL_SIW_Demura_SetDL_ReqLenth(void)
{
    // BKA369 h5c[15:0]reg_clientn_req_len
    // dont need to set
    SC_W2BYTEMSK(0,
        (MS_U32)REG_0170_TCON_ADL_CRC_BKA369,
        0,
        (MS_U16)REG_0170_TCON_ADL_CRC_BKA369_REG_CLIENTN_REQ_LEN_0170);
}

void HAL_SIW_Demura_SetDL_BaseAddr(MS_PHYADDR u32Addr)
{
    MS_U32 base_addr = u32Addr / DEMURA_DMA_ADDR_UNIT;
    DEBUG_SIW_DEMURA("HAL_SIW_Demura_SetDL_BaseAddr Addr = 0x%X 0x%X\n", u32Addr, base_addr);
    // BKA369 h5a59[28:0]reg_clientn_baseaddr
    SC_W2BYTEMSK(0,
        REG_0164_TCON_ADL_CRC_BKA369,
        (MS_U16)(base_addr & 0xFFFF),
        (REG_0164_TCON_ADL_CRC_BKA369_REG_CLIENTN_BASEADR_0_0164));
    SC_W2BYTEMSK(0,
        REG_0168_TCON_ADL_CRC_BKA369,
        (base_addr >> 16),
        (REG_0168_TCON_ADL_CRC_BKA369_REG_CLIENTN_BASEADR_1_0168));
}

void HAL_SIW_Demura_RD_Done(MS_BOOL bDone)
{
    SC_W2BYTEMSK(0,
        (MS_U32)REG_0008_LXS_DEMURA_BKA3CD,
        (MS_U16)bDone,
        (MS_U16)REG_0008_LXS_DEMURA_BKA3CD_REG_I_RD_DONE_0008);

    DEBUG_SIW_DEMURA("HAL_SIW_Demura_RD_Done %d\n", bDone);
}

void HAL_SIW_Demura_SPOC_Reg(MS_U8 *pReg)
{
    MS_U16 u16_value = 0;
    MS_U8 i = 0;
    unsigned char *pLut = (unsigned char *)pReg;

    // BKA3CD h20[7:0]~h37[7:0]
    for(i = 0; i < SIW_SPOC_REG_NUM; i++)
    {
        u16_value = pLut[i];
        DEBUG_SIW_DEMURA("SIW_Demura_Reg[%02d] 0x%04X, addr 0x%08X\n", i, u16_value, &pLut[i]);
        // reg unit is 4 bytes, 2 bytes share 1 unit
        if((i % 2) == 0)
        {
                SC_W2BYTEMSK(0,
                (MS_U32)(REG_0080_LXS_DEMURA_BKA3CD + (((i & 0xFE) >> 1) * 4)),
                (MS_U16)u16_value,
                (MS_U16)0x00FF);
        }
        else
        {
                SC_W2BYTEMSK(0,
                (MS_U32)(REG_0080_LXS_DEMURA_BKA3CD + (((i & 0xFE) >> 1) * 4)),
                (MS_U16)((u16_value & 0xFF) << 8),
                (MS_U16)0xFF00);
        }
    }

    DEBUG_SIW_DEMURA("HAL_SIW_SPOC_Reg OK\n");
}

#if(TEST_SIW_DEMURA == 1)
// no need to set, tcon.bin will disable/enable bypass
void HAL_SIW_Demura_Bypass(MS_BOOL b_bypass)
{
    SC_W2BYTEMSK(0,
        (MS_U32)REG_0004_LXS_DEMURA_BKA3CD,
        (MS_U16)b_bypass,
        (MS_U16)REG_0004_LXS_DEMURA_BKA3CD_REG_SIW_DEMURA_BYPASS_EN_0004);

    DEBUG_SIW_DEMURA("b_bypass %d \n", b_bypass);
}
#endif

