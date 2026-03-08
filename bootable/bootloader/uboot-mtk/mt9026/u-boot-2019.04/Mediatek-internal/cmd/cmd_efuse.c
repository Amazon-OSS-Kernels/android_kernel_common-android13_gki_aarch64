// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#include <common.h>
#include <command.h>
#include <debug_impl.h>

//=====================================================================================================
//
#define EFUSE_2     2

int mdrv_efuse_read_subbank_reg_U32( unsigned char u8SubBank, unsigned int * pu32RegVal);

int do_efuse_test(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    int ret = 0;
    unsigned short u16SubBank;
    unsigned int u32RegVal = 0;
    int i_rtn;


    //UBOOT_TRACE("IN\n");

    printf("{%s,%d} efuse test ...\n", __func__, __LINE__ );

    for( u16SubBank = 0; u16SubBank <= 0xFF; u16SubBank += 1 )
    {
        printf("--------------------------- Read u8SubBank:0x%X --------------------------\n", u16SubBank );
        //printf("{%s,%d} call mdrv_efuse_read_subbank_reg_U32( u8SubBank:0x%X,)\n", __func__, __LINE__, u8SubBank );

        u32RegVal = 0;
        i_rtn = mdrv_efuse_read_subbank_reg_U32( u16SubBank, &u32RegVal);
        if( i_rtn != 0 )
        {
            printf("{%s,%d} mdrv_efuse_read_subbank_reg_U32( u8SubBank:0x%X,) => i_rtn:%d\n", __func__, __LINE__, u16SubBank, i_rtn );
        }
        else
        {
            printf("{%s,%d} mdrv_efuse_read_subbank_reg_U32( u8SubBank:0x%X,) => Val:0x%X\n", __func__, __LINE__, u16SubBank, u32RegVal );
        }
    }


    //UBOOT_TRACE("OK\n");

    return ret;
}

U_BOOT_CMD(
    efuse_test, EFUSE_2, 1,    do_efuse_test,
    "efuse_test - Test read efuse\n",
    NULL
);


