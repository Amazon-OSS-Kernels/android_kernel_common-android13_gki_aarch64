// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
*/

#ifndef EFUSE_TEST_IN_PC
#define EFUSE_TEST_IN_PC    0
#endif


#include <stdio.h>
#include <stddef.h>


#if(EFUSE_TEST_IN_PC)
//#include <stdint.h>
#else
#include <common.h>
#endif

#include "efuse_common.h"

#include "hal_efuse.h"

//-----------------------------------------------------------------------------------------

#define EFUSE_ENABLE_SPINLOCK_SUBBANK   0
#define EFUSE_SPIN_LOCK_INIT_SUBBANK()
#define EFUSE_SPIN_LOCK_SUBBANK(ulFlag)
#define EFUSE_SPIN_UNLOCK_SUBBANK(ulFlag)

//--------------------------------------------------------------------------------------
#define _3M_EFUSE_BANK  0x20
#define _M6_EFUSE_BANK 0x129
#define _3M_CID_BANK    0x38
#define M6_CID_BANK    0x203

#define REG_EFUSE_00            (0x0000)//0x00
#define REG_EFUSE_02            (0x0008)//0x02
#define REG_EFUSE_28            (0x00A0)//0x28
#define REG_EFUSE_2C_RDATA_15_0 (0x00B0) //0x2C
#define REG_EFUSE_2D_RDATA_31_16 (0x00B4)//0x2D

#define REG_EFUSE_00_MASK__PROG_TRIG    0x0001
#define REG_EFUSE_02_MASK__IDLE         0x4000
#define REG_EFUSE_28_MASK__READ_TRIG    0x2000
#define REG_EFUSE_28_MASK__ADDR         0x03FC

//-----------------------------------------------------------------------------------------------

#define CONFIG_RIU_BASE_ADDRESS     0x1C000000

#ifndef RIU
#define RIU     ((unsigned short volatile *) CONFIG_RIU_BASE_ADDRESS)
#endif

#ifndef RIU8
#define RIU8    ((unsigned char volatile *) CONFIG_RIU_BASE_ADDRESS)
#endif

#define g_EFUSE_RIUBase (0x1C000000 + (_M6_EFUSE_BANK<<9))


#if(EFUSE_TEST_IN_PC)
#define EFUSE_RIU_MEM_SIZE  1024
EFUSE_U8 s_au8_efuse_bank_reg_val[EFUSE_RIU_MEM_SIZE];
#endif

//==========================================================================================================================
#define EFUSE_SHIFT_BIT_8   8
#define EFUSE_SHIFT_BIT_16  16

#define EFUSE_0xFFFFFFFF    0xFFFFFFFF
//==========================================================================================================================

#if (EFUSE_TEST_IN_PC)
#define udelay(us)
#endif

#define DEBUG_RW_BUS_ADDR(x)

static EFUSE_U16 readw(EFUSE_U32 u32Addr) // todo: need test
{
    EFUSE_U16* volatile pu16Data;

    DEBUG_RW_BUS_ADDR( EFUSE_PRINT(EFUSE_TAG "readw(u32Addr:0x%X) \n", u32Addr ); );

#if(EFUSE_TEST_IN_PC)
    pu16Data = (EFUSE_U16* volatile)(EFUSE_U64)u32Addr;

    EFUSE_DEBUG(EFUSE_TAG "{%s,%d} Read pu16Data:0x%llX ==> 0 \n", __FUNCTION__, __LINE__, (EFUSE_U64)pu16Data );

    return 0;

#else

    pu16Data = (EFUSE_U16* volatile)(EFUSE_U64)u32Addr;

    return *pu16Data;
#endif
}

static void writew( EFUSE_U16 u16Data, EFUSE_U32 u32Addr) // todo: need test
{
    EFUSE_U16* volatile pu16Data;

    DEBUG_RW_BUS_ADDR( EFUSE_PRINT(EFUSE_TAG "writew(u16Data:0x%X,u32Addr:0x%X) \n", u16Data, u32Addr ); );

#if(EFUSE_TEST_IN_PC)

    pu16Data = ((EFUSE_U16* volatile)(EFUSE_U64)(u32Addr));
    EFUSE_DEBUG(EFUSE_TAG "{%s,%d} Write pu16Data:0x%llX to 0x%X\n", __FUNCTION__, __LINE__, (EFUSE_U64)pu16Data, u16Data );

#else
    pu16Data = ((EFUSE_U16* volatile)(EFUSE_U64)(u32Addr));
    *pu16Data = u16Data;
#endif
}

EFUSE_U16 _efuse_hal_EFUSE_read_reg_U16(EFUSE_U16 u16RegAddr)
{
    EFUSE_U16 u16RegVal;

    EFUSE_DEBUG(EFUSE_TAG "_efuse_hal_EFUSE_read_reg_U16(0x%X) \n", u16RegAddr );

    //EFUSE_PRINT(EFUSE_TAG "{%s,%d} _efuse_hal_EFUSE_read_reg_U16(u16RegAddr:0x%X) \n", __FUNCTION__, __LINE__, u16RegAddr );
    /*
    if( g_EFUSE_RIUBase == NULL )
    {
        EFUSE_ERR(EFUSE_TAG "{%s,%d}Error: g_EFUSE_RIUBase is NULL! => Not inited!\n",
            __FUNCTION__, __LINE__ );
        return 0;
    }*/

    u16RegVal = readw(g_EFUSE_RIUBase + u16RegAddr);

#if(EFUSE_TEST_IN_PC)
    u16RegVal = s_au8_efuse_bank_reg_val[u16RegAddr] + (s_au8_efuse_bank_reg_val[u16RegAddr+1]<<EFUSE_SHIFT_BIT_8);
#endif

    return u16RegVal;
}

void _efuse_hal_EFUSE_wrtie_reg_U16(EFUSE_U16 u16RegAddr, EFUSE_U16 u16Data)
{
    EFUSE_DEBUG(EFUSE_TAG "{%s,%d} _efuse_hal_EFUSE_wrtie_reg_U16(u16RegAddr:0x%X, u16Data:0x%X) \n", __FUNCTION__, __LINE__, u16RegAddr, u16Data );

    /*
    if( g_EFUSE_RIUBase == NULL )
    {
        EFUSE_ERR(EFUSE_TAG "{%s,%d}Error: g_EFUSE_RIUBase is NULL! => Not inited!\n",
            __FUNCTION__, __LINE__ );
        return ;
    }*/

    writew( u16Data, g_EFUSE_RIUBase + u16RegAddr );

  #if(EFUSE_TEST_IN_PC)
    s_au8_efuse_bank_reg_val[u16RegAddr] = u16Data;
    s_au8_efuse_bank_reg_val[u16RegAddr+1] = u16Data>>EFUSE_SHIFT_BIT_8;
  #endif

}

//==========================================================================================================================

#define EFUSE_READ_SUB_BANK_TIMEOUT_ENABLE  1

#define EFUSE_READ_SUB_BANK_DELAY_US        5 // us
#define EFUSE_READ_SUB_BANK_TIMEOUT_COUNT   20
EFUSE_BOOL _efuse_hal_EFUSE_CheckEfuseReadDone(void)
{
#if(EFUSE_READ_SUB_BANK_TIMEOUT_ENABLE)
    EFUSE_U32 u32timeout_count = 0;
#endif
    EFUSE_BOOL b_read_trig;

    EFUSE_DEBUG(EFUSE_TAG "{%s,%d} Wait REG_EFUSE_28[13]=0 \n", __FUNCTION__, __LINE__ );

    while ( 1 )
    {
        if( _efuse_hal_EFUSE_read_reg_U16(REG_EFUSE_28) & REG_EFUSE_28_MASK__READ_TRIG )
            b_read_trig = 1;
        else
            b_read_trig = 0;

    #if(EFUSE_TEST_IN_PC)
        b_read_trig = 0;
    #endif

        if ( b_read_trig == 0 )
        {
            // ==> Read done
            break;
        }

    #if(EFUSE_READ_SUB_BANK_TIMEOUT_ENABLE)
        if ( (++u32timeout_count) > EFUSE_READ_SUB_BANK_TIMEOUT_COUNT)
        {
            EFUSE_ERR(EFUSE_TAG "{%s,%d}Error: Read EFUSE sub bank timeout!!\n",
                __FUNCTION__, __LINE__ );
            return FALSE;
        }

        EFUSE_PRINT(EFUSE_TAG "{%s,%d} Wait REG_EFUSE_28[13]=0 \n", __FUNCTION__, __LINE__ );

        //MsOS_DelayTaskUs(10);
        udelay(EFUSE_READ_SUB_BANK_DELAY_US);
    #endif
    }

    EFUSE_DEBUG(EFUSE_TAG "{%s,%d} Wait REG_EFUSE_28[13]=0 done\n", __FUNCTION__, __LINE__ );

    return TRUE;
}


#define EFUSE_WAIT_IDLE__DELAY_US       10 // us
#define EFUSE_WAIT_IDLE__TIMEOUT_COUNT  100

EFUSE_BOOL _efuse_hal_EFUSE_Wait_Idle(void)
{
#if(EFUSE_READ_SUB_BANK_TIMEOUT_ENABLE)
    EFUSE_U32 u32timeout_count = 0;
#endif
    EFUSE_BOOL b_idle;
    EFUSE_BOOL b_prog_trig;
    EFUSE_BOOL b_read_trig;


    EFUSE_DEBUG(EFUSE_TAG "{%s,%d} _efuse_hal_EFUSE_Wait_Idle() \n", __FUNCTION__, __LINE__ );

    // Wait idle(EFUSE_02[14])=1 ...
    // Wait prog_trig(EFUSE_00[0])=0,
    // Wait read_trig(EFUSE_28[13])=0,
    while (1)
    {
        if( _efuse_hal_EFUSE_read_reg_U16(REG_EFUSE_02) & REG_EFUSE_02_MASK__IDLE )
            b_idle = 1;
        else
            b_idle = 0;

        if( _efuse_hal_EFUSE_read_reg_U16(REG_EFUSE_00) & REG_EFUSE_00_MASK__PROG_TRIG )
            b_prog_trig = 1;
        else
            b_prog_trig = 0;

        if( _efuse_hal_EFUSE_read_reg_U16(REG_EFUSE_28) & REG_EFUSE_28_MASK__READ_TRIG )
            b_read_trig = 1;
        else
            b_read_trig = 0;


    #if(EFUSE_TEST_IN_PC)
        b_idle = 1;
        b_read_trig = 0;
    #endif

        if( (b_idle == 1)
          &&(b_prog_trig == 0)
          &&(b_read_trig == 0)
          )
        {
            EFUSE_DEBUG(EFUSE_TAG "{%s,%d} _efuse_hal_EFUSE_Wait_Idle() => OK \n", __FUNCTION__, __LINE__ );
            return TRUE;
        }

        EFUSE_PRINT(EFUSE_TAG "{%s,%d} Wait EFUSE idle => (%d,%d,%d)\n",
            __FUNCTION__, __LINE__, b_idle, b_prog_trig, b_read_trig );

    #if(EFUSE_READ_SUB_BANK_TIMEOUT_ENABLE)
        if ( (++u32timeout_count) > EFUSE_WAIT_IDLE__TIMEOUT_COUNT)
        {
            EFUSE_ERR(EFUSE_TAG "{%s,%d}Error: Wait EFUSE idle timeout! => NG (%d,%d,%d)\n",
                __FUNCTION__, __LINE__, b_idle, b_prog_trig, b_read_trig );
            return FALSE;
        }

        //MsOS_DelayTaskUs(10);
        udelay(EFUSE_WAIT_IDLE__DELAY_US);
    #endif
    }

    EFUSE_PRINT(EFUSE_TAG "{%s,%d} _efuse_hal_EFUSE_Wait_Idle() => NG\n", __FUNCTION__, __LINE__ );
    return FALSE;
}


#define EFUSE_READ_SUBBANK_HW_ONETIME   0

// From int _efuse_hal_EFUSE_read_subbank_reg_U32( EFUSE_U8 u8SubBank, EFUSE_U32 * pu32RegVal)
int hal_efuse_read_subbank_reg_U32( unsigned char u8SubBank, unsigned int * pu32RegVal)
{
    EFUSE_U32 u32_reg_val = 0;
    EFUSE_U32 u32Val_bit15_0 = 0;
    EFUSE_U32 u32Val_bit31_16 = 0;
#if(EFUSE_READ_SUB_BANK_TIMEOUT_ENABLE)
    EFUSE_U8 u8RetryCount = 0;
#endif
    EFUSE_U16 u16Reg28Val_1;
    EFUSE_U16 u16Reg28Val_2;
    EFUSE_BOOL bReadSubbankOK = FALSE;
    int i_func_rtn;


#if ( EFUSE_ENABLE_SPINLOCK_SUBBANK )
    unsigned long ul_spinlock_flags_for_subbank = 0;
#endif


    EFUSE_PRINT(EFUSE_TAG "{%s,%d} (u8SubBank:0x%X) \n", __FUNCTION__, __LINE__, u8SubBank );


    // Check para ...
    if( pu32RegVal == NULL )
    {
        EFUSE_ERR(EFUSE_TAG "{%s,%d} Error: Invalid para pu32RegVal!\n", __FUNCTION__, __LINE__ );
        return K_EFUSE_R_ERR_INVALID_PARA;
    }


    // Check init done
    //EFUSE_CHECK_INIT_DONE__FAIL_RTN(K_EFUSE_R_ERR_INIT_NOT_DONE);

/*
    if( g_EFUSE_RIUBase == NULL )
    {
        EFUSE_ERR(EFUSE_TAG "{%s,%d}Error: g_EFUSE_RIUBase is NULL! => Not inited!\n",
            __FUNCTION__, __LINE__ );
        return K_EFUSE_R_ERR_INIT_NOT_DONE;
    }
*/

#define EFUSE_READ_SUBBANK_RETRY_MAX    5
#define EFUSE_SUB_BANK_SHIFT            2
#define EFUSE_READ_SUBBANK_RETRY_DELAY  10


    u16Reg28Val_1 = (u8SubBank<<EFUSE_SUB_BANK_SHIFT)&REG_EFUSE_28_MASK__ADDR;

    // lock subbank
    EFUSE_SPIN_LOCK_SUBBANK(ul_spinlock_flags_for_subbank);

#if (EFUSE_READ_SUBBANK_HW_ONETIME)
    if( s_ast_EfuseSubBankValue[u8SubBank].bValid )
    {
        *pu32RegVal = s_ast_EfuseSubBankValue[u8SubBank].u32Val;

        EFUSE_PRINT(EFUSE_TAG "{%s,%d} Use last subbank_%02X val:0x%X \n", __FUNCTION__, __LINE__, u8SubBank, s_ast_EfuseSubBankValue[u8SubBank].u32Val );

        // unlock subbank
        EFUSE_SPIN_UNLOCK_SUBBANK(ul_spinlock_flags_for_subbank);
        return K_EFUSE_R_OK;
    }
#endif


    while( 1 )
    {
        // Wait idle(EFUSE_02[14])=1 ...
        // Wait prog_trig(EFUSE_00[0])=0,
        // Wait read_trig(EFUSE_28[13])=0,
        if( _efuse_hal_EFUSE_Wait_Idle() == FALSE )
        {
            EFUSE_ERR(EFUSE_TAG "{%s,%d}Error: _efuse_hal_EFUSE_Wait_Idle FAIL!! \n",
                __FUNCTION__, __LINE__ );

            // unlock subbank
            EFUSE_SPIN_UNLOCK_SUBBANK(ul_spinlock_flags_for_subbank);

            return K_EFUSE_R_ERR_WAIT_HW_TIMEOUT;
        }

        // set efuse subbank and trigger ...
        EFUSE_DEBUG(EFUSE_TAG "{%s,%d} Write REG_EFUSE_28 = 0x%04X \n", __FUNCTION__, __LINE__, (unsigned int)(u16Reg28Val_1 | REG_EFUSE_28_MASK__READ_TRIG) );
        _efuse_hal_EFUSE_wrtie_reg_U16( REG_EFUSE_28, (u16Reg28Val_1 | REG_EFUSE_28_MASK__READ_TRIG) );

        if( _efuse_hal_EFUSE_CheckEfuseReadDone() == FALSE )
        {
            EFUSE_ERR(EFUSE_TAG "{%s,%d}Error: _efuse_hal_EFUSE_CheckEfuseReadDone FAIL!! \n",
                __FUNCTION__, __LINE__ );

            // unlock subbank
            EFUSE_SPIN_UNLOCK_SUBBANK(ul_spinlock_flags_for_subbank);

            return K_EFUSE_R_ERR_WAIT_HW_TIMEOUT;
        }

        // Read sub bank value ...
        //EFUSE_PRINT(EFUSE_TAG "{%s,%d} read REG_EFUSE_2C_RDATA_15_0  \n", __FUNCTION__, __LINE__ );
        u32Val_bit15_0 = _efuse_hal_EFUSE_read_reg_U16(REG_EFUSE_2C_RDATA_15_0);

        //EFUSE_PRINT(EFUSE_TAG "{%s,%d} read REG_EFUSE_2D_RDATA_31_16  \n", __FUNCTION__, __LINE__ );
        u32Val_bit31_16 = _efuse_hal_EFUSE_read_reg_U16(REG_EFUSE_2D_RDATA_31_16);

        // Read subbank again
        u16Reg28Val_2 = _efuse_hal_EFUSE_read_reg_U16(REG_EFUSE_28)&REG_EFUSE_28_MASK__ADDR;


        EFUSE_DEBUG(EFUSE_TAG "{%s,%d} read REG_EFUSE_2C_RDATA_15_0 => 0x%X \n", __FUNCTION__, __LINE__, u32Val_bit15_0 );
        EFUSE_DEBUG(EFUSE_TAG "{%s,%d} read REG_EFUSE_2D_RDATA_31_16 => 0x%X \n", __FUNCTION__, __LINE__, u32Val_bit31_16 );

        u32_reg_val = u32Val_bit15_0 | (u32Val_bit31_16 << EFUSE_SHIFT_BIT_16);

        // Check if subbank changed...
        EFUSE_DEBUG(EFUSE_TAG "{%s,%d} Check subbank: u16Reg28Val_1:0x%X, u16Reg28Val_2:0x%X\n", __FUNCTION__, __LINE__, u16Reg28Val_1, u16Reg28Val_2 );
        if( u16Reg28Val_2 == u16Reg28Val_1 ) // subbank is the same => Read OK
        {
            // Read success ...
            bReadSubbankOK = TRUE;
            break;
        }
        else // Read NG
        {
            EFUSE_ERR(EFUSE_TAG "{%s,%d} Error: subbank changed: u16Reg28Val_1:0x%X, u16Reg28Val_2:0x%X\n", __FUNCTION__, __LINE__, u16Reg28Val_1, u16Reg28Val_2 );
        }

    #if(EFUSE_READ_SUB_BANK_TIMEOUT_ENABLE)
        // Need retry ...
        u8RetryCount += 1;
        if( u8RetryCount >= EFUSE_READ_SUBBANK_RETRY_MAX )
        {
            EFUSE_ERR(EFUSE_TAG "{%s,%d}Error: Efuse read sub bank FAIL! retry over %d time\n",
                __FUNCTION__, __LINE__, EFUSE_READ_SUBBANK_RETRY_MAX );
            break;
        }
        else
        {
            EFUSE_PRINT(EFUSE_TAG "{%s,%d} Efuse read sub bank retry ... %d\n", __FUNCTION__, __LINE__, u8RetryCount );
        }

      #if(EFUSE_READ_SUBBANK_RETRY_DELAY)
        // Need sleep ??
        udelay(EFUSE_READ_SUBBANK_RETRY_DELAY);
      #endif
    #endif
    }

    if( bReadSubbankOK )
    {
        i_func_rtn = K_EFUSE_R_OK;

    #if(EFUSE_READ_SUBBANK_HW_ONETIME)
        s_ast_EfuseSubBankValue[u8SubBank].u32Val = u32_reg_val;
        s_ast_EfuseSubBankValue[u8SubBank].bValid = 1;
        EFUSE_PRINT(EFUSE_TAG "{%s,%d} Save subbank_%02X val:0x%X \n", __FUNCTION__, __LINE__, u8SubBank, s_ast_EfuseSubBankValue[u8SubBank].u32Val );
    #endif
        *pu32RegVal = u32_reg_val;
    }
    else
    {
        i_func_rtn = K_EFUSE_R_ERR_READ_SUBBANK_FAIL;
        *pu32RegVal = EFUSE_0xFFFFFFFF;
    }

    // unlock subbank
    EFUSE_SPIN_UNLOCK_SUBBANK(ul_spinlock_flags_for_subbank);

    return i_func_rtn;
}


