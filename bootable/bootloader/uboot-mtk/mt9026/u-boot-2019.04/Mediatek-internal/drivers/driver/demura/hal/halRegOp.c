// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#include <linux/io.h>
#include <MsTypes.h>
#include <stdio.h>
#include <command.h>
#include <debug_impl.h>
#include <halRegOp.h>

#ifdef CONFIG_DEMURA_MT5896

//#ifdef REG_BASE
//#undef REG_BASE
//#define REG_BASE 0
//#endif
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


MS_U32 __u4IO32DemuraAccessFld(
    MS_U8 write, MS_U32 tmp32, MS_U32 val32, MS_U32 fld)
{
    MS_U32 t = 0;

    switch (Fld_ac(fld)) {
    case AC_FULLB0:
    case AC_FULLB1:
    case AC_FULLB2:
    case AC_FULLB3:
        if (write == 1)
            t = (tmp32&(~((MS_U32)0xFF<<
                (8*(Fld_ac(fld)-AC_FULLB0))))) |
                ((val32&0xFF)<<(8*(Fld_ac(fld)-AC_FULLB0)));
        else
            t = (tmp32&((MS_U32)0xFF<<
                (8*(Fld_ac(fld)-AC_FULLB0)))) >>
                (8*(Fld_ac(fld)-AC_FULLB0));
        break;
    case AC_FULLW10:
    case AC_FULLW21:
    case AC_FULLW32:
        if (write == 1)
            t = (tmp32&(~((MS_U32)0xFFFF<<
                (8*(Fld_ac(fld)-AC_FULLW10))))) |
                ((val32&0xFFFF)<<(8*(Fld_ac(fld)-AC_FULLW10)));
        else
            t = (tmp32&(((MS_U32)0xFFFF<<
                (8*(Fld_ac(fld)-AC_FULLW10))))) >>
                (8*(Fld_ac(fld)-AC_FULLW10));
        break;
    case AC_FULLDW:
        t = val32;
        break;
    case AC_MSKB0:
    case AC_MSKB1:
    case AC_MSKB2:
    case AC_MSKB3:
        if (write == 1)
            t = (tmp32&(~(((((MS_U32)1<<Fld_wid(fld))-1)<<
                Fld_shift(fld))))) |
                (((val32&(((MS_U32)1<<Fld_wid(fld))-1))<<
                Fld_shift(fld)));
        else
            t = (tmp32&(((((MS_U32)1<<Fld_wid(fld))-1)<<
                Fld_shift(fld)))) >>
                Fld_shift(fld);
        break;
    case AC_MSKW10:
    case AC_MSKW21:
    case AC_MSKW32:
        if (write == 1)
            t = (tmp32&(~(((((MS_U32)1<<Fld_wid(fld))-1)<<
                Fld_shift(fld))))) |
                (((val32&(((MS_U32)1<<Fld_wid(fld))-1))<<
                Fld_shift(fld)));
        else
            t = (tmp32&(((((MS_U32)1<<Fld_wid(fld))-1)<<
                Fld_shift(fld)))) >>
                Fld_shift(fld);
        break;
    case AC_MSKDW:
        if (write == 1)
            t = (tmp32&(~(((((MS_U32)1<<Fld_wid(fld))-1)<<
                Fld_shift(fld))))) |
                (((val32&(((MS_U32)1<<Fld_wid(fld))-1))<<
                Fld_shift(fld)));
        else
            t = (tmp32&(((((MS_U32)1<<Fld_wid(fld))-1)<<
                Fld_shift(fld)))) >>
                Fld_shift(fld);
        break;
    default:
        break;
    }
    return t;
}


MS_U32 HAL_DEMURA_Fld2Mask (MS_U32 fld)
{
    return __u4IO32DemuraAccessFld(1, 0x0, 0xFFFFFFFF, fld);
}

MS_U32 HAL_DEMURA_Val2FldMask (MS_U32 u32Value, MS_U32 fld)
{
    return __u4IO32DemuraAccessFld(1, 0x0, u32Value, fld);
}

void HAL_DEMURA_WriteByteMask(MS_PHY phy64RegAddr, MS_U8 u8Val, MS_U8 u8mask)
{
    SC_W1BYTEMSK(phy64RegAddr, u8Val, u8mask);
}

void HAL_DEMURA_Write2Byte (
    MS_U32 u32P_Addr,
    MS_U32 u32Value,
    MS_U32 fld)
{
    MS_U32 tmp = 0x00;
    tmp = __u4IO32DemuraAccessFld(
                1,
                readl((const volatile void *)((uintptr_t)(u32P_Addr + REG_BASE))),
                u32Value,
                fld);
    writel(tmp, (volatile void *)((uintptr_t)(u32P_Addr + REG_BASE)));
}

void HAL_DEMURA_Write2ByteMask(
    MS_U32 u32P_Addr,
    MS_U32 u32Value,
    MS_U32 fld)
{
    MS_U32 tmp = 0x00;
    tmp = __u4IO32DemuraAccessFld(
                1,
                readl((const volatile void *)((uintptr_t)(u32P_Addr + REG_BASE))),
                u32Value,
                fld);

    writel(tmp, (volatile void *)((uintptr_t)(u32P_Addr + REG_BASE)));
}

MS_U32 HAL_DEMURA_Read2Byte(
    MS_U32 u32P_Addr)
{
    return readl((const volatile void *)((uintptr_t)(u32P_Addr + REG_BASE)));
}

MS_U32 HAL_DEMURA_Read2bytemask(
    MS_U32 u32P_Addr,
    MS_U32 fld)
{
    MS_U32 tmp;

    tmp = __u4IO32DemuraAccessFld(
                0,
                readl((const volatile void *)((uintptr_t)(u32P_Addr + REG_BASE))),
                0,
                fld);
    return tmp;
}

#else
static MS_VIRT _gMIO_MapBase = 0;

void HAL_DEMURA_SetIOMapBase(MS_VIRT nonpm_base)
{
    if (_gMIO_MapBase == 0)
    {
        _gMIO_MapBase = nonpm_base;
        UBOOT_DEBUG("Mstar Demura IOMap base:%8x, Reg offset : %s\n",
            (unsigned int)_gMIO_MapBase, "None_PM_Bank");
    }
}


MS_U8 HAL_DEMURA_ReadByte(MS_PHY phy64RegAddr)
{
    return ((volatile MS_U8*)(_gMIO_MapBase))[(phy64RegAddr << 1) - (phy64RegAddr & 1)];
}

MS_U16 HAL_DEMURA_Read2Byte(MS_PHY phy64RegAddr)
{
    return ((volatile MS_U16*)(_gMIO_MapBase))[phy64RegAddr];
}

void HAL_DEMURA_WriteByte(MS_PHY phy64RegAddr, MS_U8 u8Val)
{
    ((volatile MS_U8*)(_gMIO_MapBase))[(phy64RegAddr << 1) - (phy64RegAddr & 1)] = u8Val;
}

void HAL_DEMURA_Write2Byte(MS_PHY phy64RegAddr, MS_U16 u16Val)
{
    ((volatile MS_U16*)(_gMIO_MapBase))[phy64RegAddr] = u16Val;
}


MS_U8 HAL_DEMURA_ReadByteMask(MS_PHY phy64RegAddr, MS_U8 u8mask)
{
    return  (HAL_DEMURA_ReadByte(phy64RegAddr) & u8mask);
}

MS_U16 HAL_DEMURA_Read2ByteMask(MS_PHY phy64RegAddr, MS_U16 u16mask)
{
    return  (HAL_DEMURA_Read2Byte(phy64RegAddr) & u16mask);
}

void HAL_DEMURA_WriteByteMask(MS_PHY phy64RegAddr, MS_U8 u8Val, MS_U8 u8mask)
{
    MS_U8 regVal = HAL_DEMURA_ReadByte(phy64RegAddr);

    UBOOT_TRACE("Write %llx %lx & %lx\n", phy64RegAddr, u8Val, u8mask);
    regVal = regVal & (~(u8mask));
    regVal = regVal | (u8Val & u8mask);
    HAL_DEMURA_WriteByte(phy64RegAddr, regVal);
}

void HAL_DEMURA_Write2ByteMask(MS_PHY phy64RegAddr, MS_U16 u16Val, MS_U16 u16mask)
{
    MS_U16 regVal = HAL_DEMURA_Read2Byte(phy64RegAddr);

    regVal = regVal & (~(u16mask));
    regVal = regVal | (u16Val & u16mask);
    HAL_DEMURA_Write2Byte(phy64RegAddr, regVal);
}
#endif /* #ifndef _HALREGOP_H_ */
