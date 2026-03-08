// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#include <common.h>
#include <command.h>
#include <environment.h>
#include <debug_impl.h>
#include <mmc.h>
#include <part.h>
#include "eMMC.h"



// =======================================================================
// quickly CMD for front lines
// =======================================================================

int do_eMMCops(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    u32 u32_err = EMMC_ST_SUCCESS;
    #if defined(IF_IP_VERIFY) && IF_IP_VERIFY
    u32 u32_blk_start_addr;
    unsigned long buf;
    u32 pwr_cut_start_blk = 0;
    #endif

    // ----------------------------------------
    if (argc>=1 && strcmp(argv[1], "info") == 0)
    {
        emmc_dump_emmc_status();
        emmc_dump_driver_status();
        emmc_print_device_info();

        if(argc>2 && strcmp(argv[2], "more")==0)
        {
            printf("\n\n");
            emmc_dump_pad_clk();
            emmc_fcie_dump_registers();
            emmc_fcie_dump_debug_bus();
            printf("\n\n");
        }
        return 1;
    }
    else if (argc>=1 && strcmp(argv[1], "reg") == 0)
    {
        emmc_fcie_dump_registers();
        return 1;
    }
    else if (argc>=1 && strcmp(argv[1], "CID") == 0)
    {
        unsigned char au8_cid[16];
        if (emmc_get_cid(au8_cid) == 0)
            emmc_dump_mem(au8_cid, 15);
        else
            printf("Get CID fail\n");
        return 1;
    }
    else if (argc>=1 && strcmp(argv[1], "ecsd") == 0)
    {
        if(argc <= 2) {
            int i;
            emmc_cmd8(GLOBAL_EMMC_SECTORBUF);
            for (i = 0; i < 512; i++)
                printf("ecsd[%d]:0x%x\r\n", i, GLOBAL_EMMC_SECTORBUF[i]);
            return 1;
        }
        else if(argc == 5) {
            u8 access_mode, byte_idx, value;
            access_mode = simple_strtoul(argv[2], NULL, 16);
            byte_idx = simple_strtoul(argv[3], NULL, 16);
            value = simple_strtoul(argv[4], NULL, 16);
            u32_err = emmc_modify_ext_csd(access_mode, byte_idx, value);
            if (u32_err != EMMC_ST_SUCCESS)
                printf("eMMC set ecsd mode %Xh, byte %Xh, value %Xh fail\n",
                    access_mode, byte_idx, value);
            else
                printf("eMMC set ecsd done\n");
            return 0;
        }
    }
    else if (argc>=1 && strcmp(argv[1], "reset") == 0)
    {
        if(simple_strtoul(argv[2], NULL, 16) )
        {
            printf("eMMC reset pin high\n");
            EMMC_RST_H();
        }
        else
        {   printf("eMMC reset pin low\n");
            EMMC_RST_L();
        }

        return 0;
    }
    else if (argc>=1 && strcmp(argv[1], "sanitize") == 0)
    {
        U32 u32_err;
		ulong start, t;

        printf("eMMC: please wait...\n");

       	start = get_timer(0);
        u32_err = emmc_sanitize(0xAA);
        t = get_timer(start);

        if(EMMC_ST_SUCCESS!=u32_err)
        {
            printf("eMMC Err: sanitize fail: %Xh\n", u32_err);
            return 0;
        }

		printf("sanitize time: %lu ms\n", t);

        return 0;
    }
    else if (argc>=1 && strcmp(argv[1], "init") == 0)
    {
        u32 u32_LoopCnt, u32_i;
		ulong start, t;

        u32_LoopCnt = simple_strtoul(argv[2], NULL, 16);
        for(u32_i=0; u32_i<u32_LoopCnt; u32_i++)
        {
            emmc_reset_ready_flag();
            start = get_timer(0);
            EMMC_DISABLE_LOG(1);
            if (EMMC_ST_SUCCESS != emmc_init())
                printf("emmc_init failed \n");

            EMMC_DISABLE_LOG(0);
            t = get_timer(start);
            printf("init time: %lu ms\n", t);
        }
    }
    else if (argc>=3 && strcmp(argv[1], "cmdlog") == 0)
    {
        if (strcmp(argv[2], "1") == 0)
            emmc_drv.u8_cmd_log_en = 1;
        else
            emmc_drv.u8_cmd_log_en = 0;
    }
    else if (argc>=1 && strcmp(argv[1], "fde") == 0)
    {
        if (argc == 3 && (0 == strcmp(argv[2], "test"))) {
            if (emmc_drv.host->fde != 0) {
                emmc_test_aes_simple_test(0xFF0000FF);
                emmc_test_aes_simple_test(0x00FFFF00);
                emmc_test_aes_simple_test(0xA55AA55A);
                emmc_test_aes_simple_test(0x55AA55AA);
                emmc_test_aes_simple_test(0x5AA55AA5);
                emmc_test_aes_simple_test(0xAA55AA55);
            }
            else
                printf("warning current chip not define fde base\n");
        }
        else if (argc == 3 && (0 == strcmp(argv[2], "speed"))) {
            if (emmc_drv.host->fde != 0)
                emmc_ipverify_performance_fde();
            else
                printf("warning current chip not define fde base\n");
        }
        else if (argc == 3 && (0 == strcmp(argv[2], "active"))) {
            if (emmc_drv.host->fde != 0)
                emmc_drv.host->no_fde = 0;
        }
        else if (argc == 3 && (0 == strcmp(argv[2], "inactive")))
            emmc_drv.host->no_fde = 1;
        else {
            if (emmc_drv.host->fde != 0 && fde_aes_switch_state()
                && emmc_drv.host->no_fde == 0)
                printf("active\n");
            else
                printf("inactive\n");
        }
    }

    #if defined(CONFIG_SECURITY_STORE_IN_EMMC_FLASH) && CONFIG_SECURITY_STORE_IN_EMMC_FLASH
    // ----------------------------------------
    else if (argc>=1 && strcmp(argv[1], "rpmb") == 0)
    {
        u32  u32_data_length,u32_err,u32_wcnt;
        u16  u16_blk_addr;
        void *addr_data;
        if(strcmp(argv[2], "key")==0)
        {
            printf("RPMB write ayth key\n");
            u32_err = emmc_rpmb_program_auth_key();
            if(u32_err==0)
               printf("RPMB write ayth key ok\n");
            else
               printf("RPMB write ayth key fail\n");
        }
        else if(strcmp(argv[2], "write")==0)
        {
            printf("RPMB write\n");
            addr_data= (void *)simple_strtoul(argv[3], NULL, 16);
            u32_data_length = (u32)simple_strtoul(argv[4], NULL, 16);
            u16_blk_addr = (u16)simple_strtoul(argv[5], NULL, 16);
            u32_err = emmc_rpmb_write_data(addr_data,u32_data_length,u16_blk_addr);
            if(u32_err==0)
               printf("RPMB write ok\n");
            else
               printf("RPMB write fail\n");
        }
        else if(strcmp(argv[2], "read")==0)
        {
            printf("RPMB read\n");
            addr_data= (void *)simple_strtoul(argv[3], NULL, 16);
            u32_data_length = (u32)simple_strtoul(argv[4], NULL, 16);
            u16_blk_addr = (u16)simple_strtoul(argv[5], NULL, 16);
            u32_err = emmc_rpmb_read_data_no_verify(addr_data,u32_data_length,u16_blk_addr);
            if(u32_err==0)
               printf("RPMB read ok\n");
            else
               printf("RPMB read fail\n");
        }
        else if(strcmp(argv[2], "test")==0)
        {
            u32_err = emmc_rpmb_test();

            if(u32_err==0)
              printf("RPMB verify ok\n");
            else
              printf("RPMB verify fail\n");
        }
        else if(strcmp(argv[2], "getcnt")==0)
        {
            emmc_rpmb_get_counter_for_cmd(&u32_wcnt);
            printf("RPMB write counter=%8Xh\n",u32_wcnt);
        }
        else if(strcmp(argv[2], "chkkey")==0)
        {
            u32_err = emmc_rpmb_if_key_written();
            printf("RPMB key is written or not:%d\n",u32_err);
        }
        return 0;
    }
    #endif
    #if defined(IF_IP_VERIFY) && IF_IP_VERIFY
    else if (argc>=1 && strcmp(argv[1], "test") == 0)
    {
        u32 u32_LoopCnt =0, u32_i = 0, u32_byte_cnt = 0;
        void *addr_dma;
        U16 u16_pattern_test[16],u16_i;
        U16 u16_pattern_cnt;
        // --------------------
        if(strcmp(argv[2], "rom")==0)
        {
            addr_dma = (void *)simple_strtoul(argv[3], NULL, 16);
            u32_byte_cnt = simple_strtoul(argv[4], NULL, 16);
            emmc_boot_mode(addr_dma, u32_byte_cnt);
            return 0;
        }
		/*
        else if(strcmp(argv[2], "wp")==0 && strcmp(argv[3], "boot")==0)
        {
            eMMC_verify_BootPartition_WP();
            return 0;
        }
        else if(strcmp(argv[2], "wp")==0 && strcmp(argv[3], "user")==0 && strcmp(argv[4], "st")==0)
        {
            eMMC_get_UserWPStatus(simple_strtoul(argv[5], NULL, 16), GLOBAL_EMMC_SECTORBUF);
            return 0;
        }
        else if(strcmp(argv[2], "wp")==0 && strcmp(argv[3], "user")==0)
        {
            printf("WPGrpSize: %u MB, total: %Xh sectors \n", eMMC_get_WPGropuSize(), emmc_drv.u32_SEC_COUNT);
            eMMC_verify_UserWP_WRC(simple_strtoul(argv[4], NULL, 16));
            return 0;
        }
		*/
        else if(argc>=1 && strcmp(argv[2], "rd")==0)
        {
            printf("Test eMMC read disturbance \n");
            #if defined(DMA_MBOOT_ADDR)
            emmc_test_read_disturbance();
            #endif
            return 0;
        }
        // --------------------
        if(argc>=1 && strcmp(argv[2], "hilowtemp") == 0)
        {
            emmc_ipverify_sdr_ddr_all_clk_temp();
            return 1;
        }

        // --------------------
        u32_LoopCnt = simple_strtoul(argv[2], NULL, 16);


        if(argc >= 4 && (strcmp(argv[3], "big") == 0)) {
                puts("Warnning:\n"
                   "big test option will damage all of your data\n"
                   "including the kernel, file system\n"
                   "and environments in eMMC.\n\n"
                   "Do you really want to do it? (y/n)\n");
                if (!(getc() == 'y' && getc() == '\r'))
                    return 0;
                else
                    return 1;
        }
        

        printf("\n====== eMMC test start ======\n");

        if (u32_LoopCnt == UNLIMIT_LOOP_CNT || u32_LoopCnt == MAX_LOOP_CNT) {
            // 0x3697 or 0xFFFFFFFF => unlimited test
            u32_LoopCnt = MAX_LOOP_CNT;
            printf("[Test times setting] = (Unlimited)\n\n");          
            
        } else {
           printf("[Test times setting] = (%Xh)\n\n", u32_LoopCnt);
        }
        while (u32_i < u32_LoopCnt) {
            printf(">> Test Loop #%u : ", u32_i);

            if (argc == 4 && (strcmp(argv[3], "ro") == 0))
            {
                printf("[RO]");
                u32_err = emmc_ipverify_read_only();
            }
            else if (argc == 4 && (strcmp(argv[3], "wo") == 0))
            {
                printf("[WO]");
                U16 u16_pattern = simple_strtoul(argv[4], NULL, 16);
                u32_err = emmc_ipverify_write_only(u16_pattern);
            }
            else if (argc == 4 && (strcmp(argv[3], "big") == 0))
            {
                printf("[BIG]");
                u32_err = emmc_ipverify_main_big();
            }
            else if (argc >= 6 && (strcmp(argv[3], "crazywo") == 0))
            {
                printf("[CRZ_WO]");
                u16_pattern_cnt = simple_strtoul(argv[4], NULL, 16);
                for(u16_i=0;u16_i<u16_pattern_cnt;u16_i++)
                    u16_pattern_test[u16_i]=simple_strtoul(argv[5+u16_i], NULL, 16);

                u32_err = emmc_crazy_write_only(u16_pattern_test,u16_pattern_cnt);
            }
            else if (argc >= 6 && (strcmp(argv[3], "crazyswo") == 0))
            {
                printf("[CRZ_SWO]");
                u16_pattern_cnt = simple_strtoul(argv[4], NULL, 16);
                for(u16_i=0;u16_i<u16_pattern_cnt;u16_i++)
                    u16_pattern_test[u16_i]=simple_strtoul(argv[5+u16_i], NULL, 16);

                u32_err = emmc_crazy_single_write_only(u16_pattern_test,u16_pattern_cnt);
            } 
            else if (argc == 3)
            {
                printf("[IPV]\n");
                u32_err = emmc_ipverify_main();
            }

            if(u32_err != EMMC_ST_SUCCESS)
                return 0 ;

            printf(" --> (PASS)\n");

            //While count ++
            u32_i++;

            //Restart count
            if ( (u32_LoopCnt==MAX_LOOP_CNT) && (u32_i==MAX_LOOP_CNT) ) 
                u32_i = 0;
         
        }

        printf("\n====== eMMC test complete ====== \n");
    }
    // ----------------------------------------
      #if defined ENABLE_EMMC_ATOP && ENABLE_EMMC_ATOP
    else if (strcmp(argv[1], "pll") == 0)
    {
        static U32 u32_emmc_pll_clock = 0;

        if(!u32_emmc_pll_clock) {
            u32_emmc_pll_clock = EMMC_PLL_CLK_FAST;
            emmc_clock_setting(u32_emmc_pll_clock);
            REG_FCIE_SETBIT(FCIE_SD_MODE, BIT_SD_CLK_EN);
        }
        else {
            u32_emmc_pll_clock--;
            if(u32_emmc_pll_clock == (EMMC_PLL_CLK_SLOW-1))
                u32_emmc_pll_clock = EMMC_PLL_CLK_FAST;
            emmc_clock_setting(u32_emmc_pll_clock);
            REG_FCIE_SETBIT(FCIE_SD_MODE, BIT_SD_CLK_EN);
        }
    }
      #endif
    // ----------------------------------------
    else if (strcmp(argv[1], "speed") == 0)
    {
        emmc_ipverify_performance();
    }
    else if (strcmp(argv[1], "life") == 0)
    {
        if(0 == strcmp(argv[2], "fixed"))
            emmc_test_life_time(EMMC_LIFETIME_TEST_FIXED);
        if(0 == strcmp(argv[2], "filled"))
            emmc_test_life_time(EMMC_LIFETIME_TEST_FIXED);
        if(0 == strcmp(argv[2], "random"))
            emmc_test_life_time(EMMC_LIFETIME_TEST_RANDOM);

        return 0;
    }
    else if(strcmp(argv[1], "pwr_cut") == 0)
    {
        if(strcmp(argv[2], "init") == 0)
        {
            buf = simple_strtoul(argv[3], NULL, 16);
            u32_blk_start_addr = simple_strtoul(argv[4], NULL, 16);

            if (EMMC_ST_SUCCESS == emmc_test_pwr_cut_init_data((U8*)buf,u32_blk_start_addr))
                printf("eMMC Info: eMMC Power Cut Init OK\n");
            else
            {
                printf("eMMC Err: eMMC Power Cut Init Fail\n");
                return 1;
            }
        }
        else if(strcmp(argv[2], "test") == 0)
        {
            buf = simple_strtoul(argv[3], NULL, 16);
            u32_blk_start_addr = simple_strtoul(argv[4], NULL, 16);

            if(EMMC_ST_SUCCESS == emmc_test_pwr_cut_test((U8*)buf,u32_blk_start_addr))
                printf("eMMC Info: eMMC Power Cut Test OK\n");
            else
            {
                printf("eMMC Err: eMMC Power Cut Test Fail\n");
                return 1;
            }
        }
        else if (strcmp(argv[2], "auto_init") == 0)
        {
            buf = (unsigned long)UPGRADE_BUFFER_ADDR;
            pwr_cut_start_blk = PWR_CUT_TEST_START_BLK;

            if (EMMC_ST_SUCCESS == emmc_test_pwr_cut_init_data(
                (U8*)(unsigned long)buf, pwr_cut_start_blk)) {
                printf("eMMC Info: Power Cut Init (Auto Run: ADDR:0x%x, StartBLK:0x%x) OK\n",
                    (U32)buf,
                    pwr_cut_start_blk);
            }
            else
            {
                printf("eMMC Info: Power Cut Init (Auto Run: ADDR:0x%x, StartBLK:0x%x) Fail\n",
                    (U32)buf,
                    pwr_cut_start_blk);

                return 1;
            }
        }
    }

    #endif // IF_IP_VERIFY

    // ----------------------------------------
    else if (strcmp(argv[1], "t_table") == 0)
    {
        // --------------------------------
        if(strcmp(argv[2], "erase") == 0)
        {
            U8 u8_i;

            emmc_drv.t_table.u32_chksum = ~emmc_drv.t_table.u32_chksum;

            memcpy(GLOBAL_EMMC_SECTORBUF, (U8*)&emmc_drv.t_table, sizeof(emmc_drv.t_table));

            for(u8_i=0; u8_i<4; u8_i++)
            {   // erase DDR, HS200, HS400, HS400 ext table
                u32_err = emmc_cmd24(EMMC_DDRTABLE_BLK_0 + (u8_i << 1), GLOBAL_EMMC_SECTORBUF);
                if(u32_err != EMMC_ST_SUCCESS)
                {
                    printf("eMMC Err: erase t_table.1 fail: %Xh\n", u32_err);
                    return 1;
                }
                u32_err = emmc_cmd24(EMMC_DDRTABLE_BLK_1 + (u8_i << 1), GLOBAL_EMMC_SECTORBUF);
                if(u32_err != EMMC_ST_SUCCESS)
                {
                    printf("eMMC Err: erase t_table.2 fail: %Xh\n", u32_err);
                    return 1;
                }
            }

            printf(" ok \n");
        }
        // --------------------------------
        else if (strcmp(argv[2], "hs400") == 0)
        {
            if(argc >= 4 && strcmp(argv[3], "5.1") == 0)
            {
                #if !(defined(ENABLE_EMMC_HS400_5_1) && ENABLE_EMMC_HS400_5_1)
                printf("\n host does not support enhanced strobe\n");
                return 0;
                #else
				if(!(emmc_drv.host->mmc->cfg->host_caps & MMC_CAP(MMC_HS_400_ES))){
	                printf("\n host does not support enhanced strobe\n");
    	            return 0;
				}

                if (emmc_drv.u8_ecsd184_stroe_support) {

                    if (emmc_drv.chip_id == MT5896_CHIP_ID &&
                        emmc_drv.revision == MT5896_CHIP_VER) {
                        u32_err = emmc_fcie_detect_hs400_5_1_skew4_timing();
                    } else {                    
                        // HS400 5.1 use DLL setting for CMD and DAT[0-7]
                        u32_err = emmc_fcie_detect_hs400_5_1_timing();
                    }
                    if(u32_err != EMMC_ST_SUCCESS)
                    {
                        emmc_debug(EMMC_DEBUG_LEVEL_ERROR,1,
                            "eMMC Err: eMMC_FCIE_DetectHS400 5.1 Timing fail: %Xh\n", u32_err);
                        emmc_fcie_err_handler_stop();
                    }

                    printf("\n ok \n");
                }
                else{
                    printf("\n eMMC device does not support enhanced strobe\n");
                }
                return 0;
                #endif
            }
            else
            {
                #if !(defined(ENABLE_EMMC_HS400) && ENABLE_EMMC_HS400)

                printf("\n host does not support \n");
                return 0;

                #else
				if (!(emmc_drv.host->mmc->cfg->host_caps & MMC_CAP(MMC_HS_400))) {
					printf("\n host does not support HS400 \n");
					return 0;
				}

                emmc_drv.u32_drvflag |= DRV_FLAG_SPEED_HS400_DETECT;

                // DS mode uses HS200 timing table for CMD
                if(EMMC_ST_SUCCESS == emmc_fcie_build_hS200_timing_table())
                {

                    if (emmc_drv.chip_id == MT5896_CHIP_ID &&
                        emmc_drv.revision == MT5896_CHIP_VER) {
                        u32_err = emmc_fcie_detect_hs400_skew4_timing();
                    } else {
                        // HS400 use DLL setting for DAT[0-7]
                        u32_err = emmc_fcie_detect_hs400_timing();
                    }
                    if(u32_err != EMMC_ST_SUCCESS)
                    {
                        emmc_debug(EMMC_DEBUG_LEVEL_ERROR,1,
                                   "eMMC Err: emmc_fcie_detect_hs400_timing fail: %Xh\n", u32_err);
                        emmc_fcie_err_handler_stop();
                    }

                    printf("\n ok \n");
                }
                else
                    printf("\n fail \n");

                #endif
            }
        }
        // --------------------------------
        else if (strcmp(argv[2], "hs200") == 0)
        {
            #if !(defined(ENABLE_EMMC_HS200) && ENABLE_EMMC_HS200)
            printf("\n host does not support \n");
            return 0;
            #else
			if(!(emmc_drv.host->mmc->cfg->host_caps & MMC_CAP(MMC_HS_200))) {
				printf("\n host does not support HS200 \n");
				return 0;
			}

            if(EMMC_ST_SUCCESS == emmc_fcie_build_hS200_timing_table())
                printf("\n ok \n");
            else
                printf("\n fail \n");
            #endif
        }
        // --------------------------------

        else if(strcmp(argv[2], "ddr") == 0)
        {
            #if !(defined(ENABLE_EMMC_ATOP) && ENABLE_EMMC_ATOP)
            printf("\n host does not support \n");
            return 0;
            #else
            if (EMMC_ST_SUCCESS == emmc_fcie_build_ddr_timing_table())
                printf("\n ok \n");
            else
                printf("\n fail \n");
            #endif
        }

        else if (strcmp(argv[2], "temp") == 0)
        {
            #if (defined(ENABLE_EMMC_HS400_5_1) && ENABLE_EMMC_HS400_5_1) ||\
                (defined(ENABLE_EMMC_HS400) && ENABLE_EMMC_HS400)
            U8 u8_i,u8_enable_error_log=0;

            EMMC_DISABLE_PAD_CHECK(1);

            u8_enable_error_log = simple_strtoul(argv[3], NULL, 16);


            for(u8_i=0;u8_i<10;u8_i++)
            {
                emmc_fcie_detect_hs200_timing_skew1_skew4(u8_enable_error_log);
            }
            #if (defined(ENABLE_EMMC_HS400_5_1) && ENABLE_EMMC_HS400_5_1)
            if (emmc_drv.u8_ecsd184_stroe_support){


                if (emmc_drv.chip_id == MT5896_CHIP_ID &&
                    emmc_drv.revision == MT5896_CHIP_VER) {
                    for(u8_i=0;u8_i<10;u8_i++)
                    {
                        emmc_fcie_m6e3_detect_hs400_5_1_timing_skew4_skew1(u8_enable_error_log);
                    }

                    for(u8_i=0;u8_i<10;u8_i++)
                    {
                        emmc_fcie_m6e3_detect_hs400_5_1_timing_skew1_skew2(u8_enable_error_log);
                    }
                } else {

                    for(u8_i=0;u8_i<10;u8_i++)
                    {
                        emmc_fcie_detect_hs400_5_1_timing_rx(u8_enable_error_log);
                    }

                    for(u8_i=0;u8_i<10;u8_i++)
                    {
                        emmc_fcie_detect_hs400_5_1_timing_skew1_skew2(u8_enable_error_log);
                    }

                    for(u8_i=0;u8_i<10;u8_i++)
                    {
                        emmc_fcie_detect_hs400_5_1_timing_trigerlevel_rx(u8_enable_error_log);
                    }
                }
            }
            else
            #endif
            {

                if (emmc_drv.chip_id == MT5896_CHIP_ID &&
                    emmc_drv.revision == MT5896_CHIP_VER) {

                    for(u8_i=0;u8_i<10;u8_i++)
                    {
                        emmc_fcie_m6e3_detect_hs400_timing_skew4_skew1(u8_enable_error_log);
                    }

                    for(u8_i=0;u8_i<10;u8_i++)
                    {
                        emmc_fcie_m6e3_detect_hs400_timing_skew1_skew2(u8_enable_error_log);
                    }

                } else {
                
                    for(u8_i=0;u8_i<10;u8_i++)
                    {
                        emmc_fcie_detect_hs400_timing_skew4_rx(u8_enable_error_log);
                    }

                    for(u8_i=0;u8_i<10;u8_i++)
                    {
                        emmc_fcie_detect_hs400_timing_skew4_skew1(u8_enable_error_log);
                    }

                    for(u8_i=0;u8_i<10;u8_i++)
                    {
                        emmc_fcie_detect_hs400_timing_skew1_skew2(u8_enable_error_log);
                    }

                    for(u8_i=0;u8_i<10;u8_i++)
                    {
                        emmc_fcie_detect_hs400_timing_trigger_level_rx(u8_enable_error_log);
                    }
                }
            }
            EMMC_DISABLE_PAD_CHECK(0);

            #endif
            printf("\n done \n");

        }
        else if(strcmp(argv[2], "valid") == 0)
        {
            extern U32 gu32_skew4_result;
            extern U32 gu32_skew4_result_lv0;
            extern U32 gu32_rx_dll_result;
            extern U32 gu32_rx_dll_result_lv0;

            extern U32 emmc_fcie_detect_timing_ex(U32);

            U16 u16_start_bit = 0;
            U16 u16_end_bit = 0;
            U16 u16_i = 0;
            U16 u16_cnt = 0;

            EMMC_DISABLE_PAD_CHECK(1);

            #if (defined(ENABLE_EMMC_HS200) && ENABLE_EMMC_HS200)

            emmc_fcie_detect_hs200_timing_triger_level_skew4(1);

            // check skew4
            if((gu32_skew4_result_lv0 & ((1<<PLL_SKEW4_CNT*2)-1)) == ((1<<PLL_SKEW4_CNT*2)-1) )
            {
                //emmc_debug(0, 1, "Skew4 all pass\n");
                emmc_debug(0, 0, "\nThis board has enough skew4\n\n");
            }
            else if( 0==(gu32_skew4_result_lv0&BIT0) || 0==(gu32_skew4_result_lv0&(1<<(PLL_SKEW4_CNT*2-1))) )
            {
                //emmc_debug(0, 1, "not wrapped case\n");

                for(u16_i=0; u16_i<PLL_SKEW4_CNT*2; u16_i++)
                {
                    if ((gu32_skew4_result_lv0 & (1<<u16_i)) && START_BIT_DEFAULT == u16_start_bit)
                        u16_start_bit = u16_i;
                    else if (START_BIT_DEFAULT != u16_start_bit && 0 == (gu32_skew4_result_lv0 & (1 << u16_i)))
                        u16_end_bit = u16_i - 1;

                    if (END_BIT_DEFAULT != u16_end_bit)
                    {
                        if (u16_end_bit - u16_start_bit+1<10)
                        {
                            // to ignore "discontinous case"
                            u16_start_bit = START_BIT_DEFAULT;
                            u16_end_bit = END_BIT_DEFAULT;
                        }
                        else
                            break;
                    }
                }

                if (END_BIT_DEFAULT == u16_end_bit)
                {
                    if ( gu32_skew4_result_lv0 & 1 <<(u16_i-1) )
                        u16_end_bit = u16_i-1;
                    else
                        u16_end_bit = u16_start_bit;
                }

                if (u16_end_bit - u16_start_bit + 1 < 10)
                {
                    emmc_debug(0, 0, "\n\033[7;31mThis board doesn't have enough skew4\033[m\n\n");
                }
                else
                {
                    if( emmc_fcie_detect_timing_ex(gu32_skew4_result) )
                        emmc_debug(0, 0, "\nThis board has enough skew4\n\n");
                    else
                        emmc_debug(0, 0, "\n\033[7;31mThis board doesn't have enough skew4\033[m\n\n");
                }
            }
            else
            {
                //emmc_debug(0, 1, "wrapped case\n");
                u16_cnt = 0;

                for (u16_i = 0; u16_i < TOTAL_PLL_SKEW4_CNT; u16_i++)
                {
                    if (gu32_skew4_result_lv0 & ( 1 << u16_i))
                        u16_cnt++;
                    else
                    {
                        u16_start_bit = u16_i-1;
                        break;
                    }
                }

                for (u16_i = TOTAL_PLL_SKEW4_CNT-1; u16_i>0; u16_i--)
                {
                    if (gu32_skew4_result_lv0 & (1 << u16_i))
                        u16_cnt++;
                    else
                    {
                        u16_end_bit = u16_i+1;
                        break;
                    }
                }

                if(u16_cnt<10)
                {
                    emmc_debug(0, 0, "\n\033[7;31mThis board doesn't have enough skew4\033[m\n\n");
                }
                else
                {
                    emmc_debug(0, 0, "\nThis board has enough skew4\n\n");
                }
            }
            #endif

            #if defined(ENABLE_EMMC_HS400) && ENABLE_EMMC_HS400

            emmc_fcie_detect_hs400_timing_trigger_level_rx(1);

            u16_cnt = 0;

            for (u16_i = 0; u16_i< 16; u16_i++)
            {
                if (gu32_rx_dll_result_lv0 & (1<<u16_i))
                    u16_cnt++;
            }

            if(u16_cnt<7)
            {
                emmc_debug(0, 0, "\n\033[7;31mThis board doesn't have enough RXDLL\033[m\n\n");
            }

            u16_cnt = 0;

            for(u16_i=0; u16_i<16; u16_i++)
            {
                if (gu32_rx_dll_result & (1 << u16_i) )
                    u16_cnt++;
            }

            if(u16_cnt<5)
            {
                emmc_debug(0, 0, "\n\033[7;31mThis board doesn't have enough RXDLL\033[m\n\n");
            }
            else
            {
                emmc_debug(0, 0, "\nThis board has enough RXDLL\n\n");
            }

            #endif

            EMMC_DISABLE_PAD_CHECK(0);
        }
        else if(strcmp(argv[2], "dump") == 0)
        {
            if (EMMC_ST_SUCCESS != emmc_fcie_choose_speed_mode())
            {
                printf("\neMMC Err: emmc_fcie_choose_speed_mode fail \n");
            }
            emmc_dump_timing_table();
        }
        else if(strcmp(argv[2], "cust_drv") == 0)
        {
			u32 driving;
			if(argv[3])
				driving = simple_strtoul(argv[3], NULL, 16);
			else {
	            printf("please enter customer driving value from 0 to 4 \n");
				return 0;
			}

			if(driving > 4) {
	            printf("please enter customer driving value from 0 to 4 \n");
				return 0;
			}

			emmc_drv.host->id_matched_driving = driving;
            printf("customer driving is set to %d\n", driving);
        }
        return 0;
    }
    // ----------------------------------------
    else if (strcmp(argv[1], "mode") == 0)
    {
        if (strcmp(argv[2], "sdr") == 0)
        {
            if (EMMC_ST_SUCCESS == emmc_fcie_enable_sdr_mode()) {
                printf("\neMMC Info: SDR mode ok \n");
                emmc_clock_setting(FCIE_DEFAULT_CLK);
            } else {
                printf("\neMMC Err: SDR mode fail \n");
            }
        }
        else if (strcmp(argv[2], "best") == 0)
        {
            #if defined(IF_IP_VERIFY) && IF_IP_VERIFY
            if(EMMC_ST_SUCCESS == emmc_fcie_choose_speed_mode())
                printf("\neMMC Info: best speed mode ok \n");
            else
                printf("\neMMC Err: best speed mode fail \n");
            #endif
        }
        else
            return 1;
    }
    // ----------------------------------------
    else if (strcmp(argv[1], "clk") == 0)
    {
        u8 u8_clk = simple_strtoul(argv[2], NULL, 16);

        emmc_clock_setting(u8_clk);
        emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "clk:%u KHz\n", (unsigned int)emmc_drv.u32_clk_khz);
    }
    // ----------------------------------------
    #if defined(ENABLE_EMMC_HS400) && ENABLE_EMMC_HS400
    else if (strcmp(argv[1], "settrig") == 0) // set trigger level
    {
        U8 u8_trig_level;
        extern void emmc_set_triger_level(U8 u8_level);

        u8_trig_level = simple_strtoul(argv[2], NULL, 10);
        emmc_set_triger_level(u8_trig_level);
    }
    else if (strcmp(argv[1], "setwp") == 0) // set write phase
    {
        U8 u8_write_phase;
        extern void emmc_set_write_phase(U8 u8_cmd_data, U8 u8_phase);

        if(strcmp(argv[2], "cmd") == 0)
        {
            u8_write_phase = simple_strtoul(argv[3], NULL, 10);
            emmc_set_write_phase(3, u8_write_phase);
        }
        else if(strcmp(argv[2], "data") == 0)
        {
            u8_write_phase = simple_strtoul(argv[3], NULL, 10);
            emmc_set_write_phase(2, u8_write_phase);
        }
        else if(strcmp(argv[2], "clk") == 0)
        {
            u8_write_phase = simple_strtoul(argv[3], NULL, 10);
            emmc_set_write_phase(1, u8_write_phase);
        }
        else if(strcmp(argv[2], "skew4") == 0)
        {
            u8_write_phase = simple_strtoul(argv[3], NULL, 10);
            emmc_set_write_phase(4, u8_write_phase);
        }
    }
    #endif

    else if(strcmp(argv[1], "phase")==0)
    {
        U32 u32_i;
        U32 u32_skew2_result = 0;

        EMMC_DISABLE_PAD_CHECK(1);

        #if (defined(ENABLE_EMMC_HS400_5_1) && ENABLE_EMMC_HS400_5_1) || \
            (defined(ENABLE_EMMC_HS400) && ENABLE_EMMC_HS400) || \
            (defined(ENABLE_EMMC_HS200) && ENABLE_EMMC_HS200)

        extern U32 emmc_fcie_detect_skew4(void);
		#if (defined(ENABLE_EMMC_HS400_5_1) && ENABLE_EMMC_HS400_5_1) || \
			(defined(ENABLE_EMMC_HS400) && ENABLE_EMMC_HS400)
		U16 u16_skew2_cnt = 0;
		#endif

        #if (defined(ENABLE_EMMC_HS400_5_1) && ENABLE_EMMC_HS400_5_1)
        extern U32 emmc_fcie_detect_hs400_5_1_timing_rx(U8 u8_enable_error_log);

        if (emmc_drv.u8_ecsd184_stroe_support) {
            EMMC_DISABLE_LOG(1);

            emmc_drv.u16_mmc_mode = BIT_SD_DEFAULT_MODE_REG;

            emmc_platform_init();

            u32_err = emmc_fcie_init();
            if (u32_err != EMMC_ST_SUCCESS)
            {
                emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "FCIE Reset Fail!!\n");
                emmc_fcie_err_handler_stop();
            }

            u32_err = emmc_init_device_ex();
            if (u32_err != EMMC_ST_SUCCESS)
            {
                emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Identify Error!!\n");
                emmc_fcie_err_handler_stop();
            }

            u32_err = emmc_fcie_enable_fast_mode(FCIE_EMMC_HS400_5_1);
            if (u32_err != EMMC_ST_SUCCESS)
            {
                emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC 5.1 switch to HS400 fail!!\n");
                emmc_fcie_err_handler_stop();
            }

            emmc_fcie_apply_reg(emmc_drv.t_table_g.u8_cur_set_idx);

            for (u32_i = 0; u32_i < 9; u32_i++) {
                emmc_drv.u32_drvflag |= DRV_FLAG_TUNING_TTABLE; // turn on tunning flag
                REG_FCIE_CLRBIT(reg_emmcpll_0x03, BIT_SKEW2_MASK);
                REG_FCIE_SETBIT(reg_emmcpll_0x03, u32_i<<4);

                if (EMMC_ST_SUCCESS == emmc_test_blk_wrc_probe_timing(EMMC_TEST_BLK_0))
                {
                    u32_skew2_result |= (1 << u32_i);
                }
                else
                {
                    //emmc_debug(0, 1, "Skew2: %d fail\n", u32_i);
                    u32_err = emmc_fcie_reset_to_hs400_5_1((U8)emmc_drv.u16_clk_reg_val);

                    emmc_fcie_apply_reg(emmc_drv.t_table_g.u8_cur_set_idx);

                    if (u32_err != EMMC_ST_SUCCESS)
                    {
                        break;
                    }
                 }
            }

            emmc_drv.u32_drvflag &= ~DRV_FLAG_TUNING_TTABLE;//turn off tunning flag

            u16_skew2_cnt = emmc_fcie_detect_hs400_timing_skew2_ex(u32_skew2_result);
            emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "Skew2Cnt:%x\n", u16_skew2_cnt);

            EMMC_DISABLE_LOG(0);

            emmc_debug(0, 0, "TX:\n");

            for (u32_i = 0; u32_i < 9; u32_i++)
                printf("%02d ", u32_i);
            printf("\n");

            for (u32_i = 0; u32_i < 9; u32_i++) {
                if ((u32_skew2_result & (1 << u32_i)) == 0)
                {
                    printf(" X ");
                }
                else
                {
                    printf(" O ");
                }
            }
            printf("\n");


            emmc_fcie_detect_hs400_5_1_timing_rx(0);

            EMMC_DISABLE_PAD_CHECK(0);

        }
        else
        #endif
        {
            #if (defined(ENABLE_EMMC_HS400) && ENABLE_EMMC_HS400)
            EMMC_DISABLE_LOG(1);

            emmc_drv.u16_mmc_mode = BIT_SD_DEFAULT_MODE_REG;

            emmc_platform_init();

            u32_err = emmc_fcie_init();
            if (u32_err != EMMC_ST_SUCCESS)
            {
                emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "FCIE Reset Fail!!\n");
                emmc_fcie_err_handler_stop();
            }

            u32_err = emmc_init_device_ex();
            if (u32_err != EMMC_ST_SUCCESS)
            {
                emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Identify Error!!\n");
                emmc_fcie_err_handler_stop();
            }

            u32_err = emmc_fcie_enable_fast_mode(FCIE_EMMC_HS400);
            if (u32_err != EMMC_ST_SUCCESS)
            {
                emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC switch to HS400 fail!!\n");
                emmc_fcie_err_handler_stop();
            }

            emmc_fcie_apply_reg(emmc_drv.t_table_g.u8_cur_set_idx);

            for (u32_i = 0; u32_i < 9; u32_i++) {
                emmc_drv.u32_drvflag |= DRV_FLAG_TUNING_TTABLE; // turn on tunning flag
                REG_FCIE_CLRBIT(reg_emmcpll_0x03, BIT_SKEW2_MASK);
                REG_FCIE_SETBIT(reg_emmcpll_0x03, u32_i<<4);

                if (EMMC_ST_SUCCESS == emmc_test_blk_wrc_probe_timing(EMMC_TEST_BLK_0))
                {
                    u32_skew2_result |= (1 << u32_i);
                }
                else
                {
                    //emmc_debug(0, 1, "Skew2: %d fail\n", u32_i);
                    u32_err = emmc_fcie_reset_to_hs400((U8)emmc_drv.u16_clk_reg_val);

                    emmc_fcie_apply_reg(emmc_drv.t_table_g.u8_cur_set_idx);

                    if (u32_err != EMMC_ST_SUCCESS)
                    {
                        break;
                    }
                }
            }

            emmc_drv.u32_drvflag &= ~DRV_FLAG_TUNING_TTABLE; // turn off tunning flag

            u16_skew2_cnt = emmc_fcie_detect_hs400_timing_skew2_ex(u32_skew2_result);
            emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "Skew2Cnt:%x\n", u16_skew2_cnt);

            EMMC_DISABLE_LOG(0);

            emmc_debug(0, 0, "TX:\n");

            for (u32_i = 0; u32_i < 9; u32_i++)
                printf("%02d ", u32_i);
            printf("\n");

            for (u32_i = 0; u32_i < 9; u32_i++) {
                if ((u32_skew2_result & (1 << u32_i)) == 0)
                {
                    printf(" X ");
                }
                else
                {
                    printf(" O ");
                }
            }
            printf("\n");

            #endif

            #if (defined(ENABLE_EMMC_HS400) && ENABLE_EMMC_HS400) || \
            (defined(ENABLE_EMMC_HS200) && ENABLE_EMMC_HS200)

            emmc_fcie_detect_skew4();

            #endif

            #if (defined(ENABLE_EMMC_HS400) && ENABLE_EMMC_HS400)

            extern U32 emmc_fcie_detect_rx_dll(void);

            emmc_fcie_detect_rx_dll();
            #endif

            EMMC_DISABLE_PAD_CHECK(0);

        }
        #endif
    }
    #if defined(EMMC_SKIP_IDENTIFY) && EMMC_SKIP_IDENTIFY
    else if(strcmp(argv[1], "skipinit")==0)
    {
        if(strcmp(argv[2], "clr")==0)
        {
            // make wrong checksum
            GLOBAL_EMMC_SECTORBUF[0] = ~GLOBAL_EMMC_SECTORBUF[0];
            emmc_cmd25(EMMC_UB_DRV_CONTX, GLOBAL_EMMC_SECTORBUF, 1);
            emmc_cmd25(EMMC_KL_DRV_CONTX, GLOBAL_EMMC_SECTORBUF, 1);
            printf("done\n");
        }
        else if(strcmp(argv[2], "dump")==0)
        {
            extern U8 au8_ext_csd[];
            u16 i;
            for (i = 0; i < 512; i++)
                printf("ecsd[%d]:0x%x\r\n", i, au8_ext_csd[i]);
        }
    }
    #endif

    return 0;
}

#if defined(CONFIG_USERDATA_PARTITION_SELF_ADAPTION)
#if defined(CONFIG_SAMESIZE_EMMC_SELF_ADAPTION)
#define REMAIN_BLOCK_THRESHOLD  (1024 * 1024 * 1024 / 512)  //whether same size emmc(4/8/16/32G)
#else
#define REMAIN_BLOCK_THRESHOLD  7                           //reserve aligned size, align to 0x1000 bytes
#endif
#define PARTITION_MAX_NUM       CONFIG_EFI_PARTITION_ENTRIES_NUMBERS
#define SECONDARY_GPT_SIZE      33                          //LBA-0x21 ~ LBA-0x1
int do_emmc_self_adaption(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    struct mmc *mmc = NULL;
    struct blk_desc *mmc_blk_desc = NULL;
    disk_partition_t part_info;
    int remaining_block = 0;
    int curr_device = 0;
    char *env_first_setup = NULL;
    int ret = 0;
    int i = 0;

    UBOOT_TRACE("IN\n");
    env_first_setup = env_get("first_setup");
    if(env_first_setup != NULL && strncmp(env_first_setup, "1", strlen(env_first_setup)) == 0)
    {
        UBOOT_INFO("do not first setup (first_setup=%s), skip self adaption.\n", env_first_setup);
        UBOOT_TRACE("OK\n");
        return 0;
    }

    //mmc init
    mmc = find_mmc_device(curr_device);
    if(mmc == NULL)
    {
        UBOOT_ERROR("no mmc device at slot %d!\n", curr_device);
        return CMD_RET_FAILURE;
    }
    mmc_blk_desc = mmc_get_blk_desc(mmc);
    if(mmc_blk_desc == NULL)
    {
        UBOOT_ERROR("no mmc device found!\n");
        return CMD_RET_FAILURE;
    }
    else
    {
        UBOOT_DEBUG("eMMC: capacity LBA is 0x%llX\n", mmc->capacity / mmc_blk_desc->blksz);
        UBOOT_DEBUG("eMMC: capacity_user LBA is 0x%llX\n", mmc->capacity_user / mmc_blk_desc->blksz);
        UBOOT_DEBUG("eMMC: capacity_boot LBA is 0x%llX\n", mmc->capacity_boot / mmc_blk_desc->blksz);
        UBOOT_DEBUG("eMMC: capacity_rpmb LBA is 0x%llX\n", mmc->capacity_rpmb / mmc_blk_desc->blksz);
        UBOOT_DEBUG("eMMC: number of blocks is 0x%lX\n", mmc_blk_desc->lba);
        UBOOT_DEBUG("eMMC: block size is 0x%lX\n", mmc_blk_desc->blksz);
    }

    //check whether userdata is the last partition
    for(i = 1; i < PARTITION_MAX_NUM; i++)
    {
        ret = part_get_info(mmc_blk_desc, i, &part_info);
        if(ret != 0)
            break;
        UBOOT_DEBUG("number: %-5d, name: %-15s, start LBA: 0x%-8lX, end LBA: 0x%-8lX, size LBA: 0x%-8lX\n",
                    i, part_info.name, part_info.start, part_info.start + part_info.size - 1, part_info.size);
    }
    ret = part_get_info(mmc_blk_desc, i - 1, &part_info);
    if(ret != 0)
    {
        UBOOT_ERROR("get partition info fail!\n");
        return CMD_RET_FAILURE;
    }
    if(strncmp((char *)part_info.name, "userdata", strlen((char *)part_info.name)) != 0)
    {
        UBOOT_ERROR("userdata is not the last partition, emmc self adaption fail!\n");
        UBOOT_ERROR("the last partition is %s.\n", part_info.name);
        return CMD_RET_FAILURE;
    }

    //get remain space: emmc size - current part size include secondary gpt table size, and do adaption
    remaining_block = mmc->capacity / mmc_blk_desc->blksz - (part_info.start + part_info.size) - SECONDARY_GPT_SIZE;
    if(remaining_block > REMAIN_BLOCK_THRESHOLD)
    {
        UBOOT_DEBUG("name: %s, number: %d, start LBA: 0x%lX, size LBA: 0x%lX\n", part_info.name, i - 1, part_info.start, part_info.size);
        UBOOT_DEBUG("remaining LBA: 0x%X\n", remaining_block);
        ret = run_command("gpt remove mmc 0 userdata", 0);
        if(ret != 0)
        {
            UBOOT_ERROR("remove userdata fail!\n");
            return CMD_RET_FAILURE;
        }
        ret = run_command("gpt create_adaptive mmc 0 userdata 0x1000", 0);
        if(ret != 0)
        {
            UBOOT_ERROR("create userdata fail!\n");
            return CMD_RET_FAILURE;
        }
        ret = run_command("wipeuserdata", 0);          //when first_setup!=1 && userdata size isn't max, must into recovery
        if(ret != 0)
        {
            UBOOT_ERROR("wipe userdata fail!\n");
            return CMD_RET_FAILURE;
        }
        UBOOT_DEBUG("userdata self adaption success!\n");
    }
    else
    {
        UBOOT_DEBUG("userdata size is already the MAX!\n");
    }

    env_set("first_setup", "1");
    env_save();

    UBOOT_TRACE("OK\n");
    return 0;
}
#endif

U_BOOT_CMD(
    emmc,    22,    1,     do_eMMCops,
	"eMMC sub system",
	"emmc info - lists CSD & ExtCSD on eMMC\n"
	"emmc init count - reset & init eMMC for count loops\n"
	"emmc test count - verify eMMC & board signals for count loops\n"
	"emmc speed - show eMMC speed sdr or ddr mode @ driver layer\n"
	"emmc t_table <ddr/hs200/hs400/erase/dump> - build timing table\n"
	"emmc mode <sdr/best> - SDR or choose best mode \n"
	"emmc clk - set ClkRegVal \n"
	"emmc cis - check or erase \n"
	"emmc pwr_cut init [addr][start block] - eMMC Power Cut Init\n"
	"emmc pwr_cut test [addr][start block] - eMMC Power Cut Test\n"
	"emmc reset [0/1] - toggle eMMC reset pin\n"
	"emmc rpmb key - program the authentication key\n"
    "emmc rpmb write [addr] [size] [start block]- authenticated data write\n"
    "emmc rpmb read  [addr] [size] [start block]- authenticated data read\n"
    "emmc rpmb test - verify eMMC rpmb\n"
    "emmc rpmb getcnt - get eMMC write counter\n"
);

#if defined(CONFIG_USERDATA_PARTITION_SELF_ADAPTION)
U_BOOT_CMD(
    emmc_self_adaption,    1,    0,     do_emmc_self_adaption,
    "emmc_self_adaption - \n",
    "emmc_self_adaption - allocate the reset of emmc space to userdata partition\n"
);
#endif
