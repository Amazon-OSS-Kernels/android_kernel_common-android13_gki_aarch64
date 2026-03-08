// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#include <common.h>
#include <linux/mtd/mtd.h>
#include <command.h>
#include <console.h>
#include <malloc.h>
#include <jffs2/jffs2.h>
#include <nand.h>
#include <hexdump.h>
#include <ubi_uboot.h>
#include <linux/mtd/ubi.h>

#define ERASED_VAL      0xFF

#define HASH0           0x80
#define SBOOT           0x81
#define UBOOT           0x82
#define DRAMC           0xDC

#define HASH_START_BLK  0x0A
#define DRAMC_BLK       1

#define NNI_SIZE        0x200
#define PNI_SIZE        0x200
#define PPM_SIZE        0x800

extern u32 drvNAND_FindMatchCIS(u8 *nniaddr, u8 *pniaddr,
        u16 *nniidx, u16 *pniidx, u16 *ppmidx);
extern u32 drvNAND_WriteCIS(u8 *nniaddr, u8 *pniaddr, u8 *ppmaddr);

static int nand_write_hash(struct mtd_info *mtd, loff_t offset, size_t length, u_char *buffer,
        u_char hash, loff_t *next_offset)
{
        int ret = 0;
        size_t left_to_write = length;
        size_t write_size;
        struct erase_info erase;
        mtd_oob_ops_t ops;
        u_char *p_buffer = buffer;
        u_char *p_oobbuf;
        int blocksize = mtd->erasesize;
        bool hashid_written = false;
        int i;

        p_oobbuf = malloc(mtd->oobsize);
        if (!p_oobbuf)
        {
                printf("Error: malloc oob buffer failed!\n");
                return -ENOMEM;
        }
        memset((void *)p_oobbuf, ERASED_VAL, mtd->oobsize);

        while (left_to_write > 0)
        {
                if (offset >= mtd->size)
                {
                        printf("No available block!\n");
                        ret = -ENOTBLK;
                        goto out;
                }

                if (nand_block_isbad(mtd, offset))
                {
                        printf("Skip bad block 0x%08llx\n", offset);
                        offset += mtd->erasesize;
                        continue;
                }

                memset(&erase, 0, sizeof(erase));
                erase.mtd = mtd;
                erase.len = mtd->erasesize;
                erase.addr = offset;
                ret = mtd_erase(mtd, &erase);
                if (ret != 0)
                {
                        printf("NAND erase to 0x%llx failed %d\n", offset, ret);
                        ret = mtd_block_markbad(mtd, offset);
                        if (ret != 0)
                        {
                                printf("NAND markbad to 0x%llx failed %d\n", offset, ret);
                                goto out;
                        }

                        offset += mtd->erasesize;
                        continue;
                }

                if (left_to_write < blocksize)
                        write_size = left_to_write;
                else
                        write_size = blocksize;

                if (hashid_written == false)
                {
                        p_oobbuf[1] = hash;
                        hashid_written = true;
                }
                else
                        p_oobbuf[1] = ERASED_VAL;

                memset(&ops, 0, sizeof(ops));
                ops.mode = MTD_OPS_PLACE_OOB;
                ops.len = write_size;
                ops.ooblen = mtd->oobsize;
                ops.ooboffs = 0;
                ops.datbuf = p_buffer;
                ops.oobbuf = p_oobbuf;

                ret = mtd_write_oob(mtd, offset, &ops);
                if (ret != 0)
                {
                        printf("NAND write to offset %llx failed %d\n", offset, ret);
                        goto out;
                }
                offset += write_size;
                p_buffer += write_size;
                left_to_write -= write_size;
        }

        if (offset & (mtd->erasesize - 1))
                offset = (offset & ~(mtd->erasesize - 1)) + mtd->erasesize;

        if (hash == SBOOT)
        {
                i = 0;
                while (i < DRAMC_BLK )
                {
                        if (offset >= mtd->size)
                        {
                                printf("No available block!\n");
                                ret = -ENOTBLK;
                                goto out;
                        }

                        if (nand_block_isbad(mtd, offset))
                        {
                                printf("Skip bad block 0x%08llx\n", offset);
                                offset += mtd->erasesize;
                                continue;
                        }

                        memset(&erase, 0, sizeof(erase));
                        erase.mtd = mtd;
                        erase.len = mtd->erasesize;
                        erase.addr = offset;
                        ret = mtd_erase(mtd, &erase);
                        if (ret != 0)
                        {
                                printf("NAND erase to 0x%llx failed %d\n", offset, ret);
                                ret = mtd_block_markbad(mtd, offset);
                                if (ret != 0)
                                {
                                        printf("NAND markbad to 0x%llx failed %d\n", offset, ret);
                                        goto out;
                                }

                                offset += mtd->erasesize;
                                continue;
                        }

                        offset += mtd->erasesize;
                        i++;
                }
        }
out:
        *next_offset = offset;
        free(p_oobbuf);
        return ret;
}

static int do_ncishash(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
        int ret = 0;
        int dev = nand_curr_device;
        struct mtd_info *mtd;
        loff_t offset, next_offset;
        ulong nniaddr, ppmaddr, pniaddr;
        ulong hash0addr, sbootaddr, ubootaddr;
        size_t hash0size, sbootsize, ubootsize;
        u16 nniidx, pniidx, ppmidx;
        //bool clean = 0;

        mtd = get_nand_dev_by_index(dev);
        if (!mtd)
        {
                printf("no devices available\n");
                return 1;
        }

        #if 0
        if (strcmp(argv[1], "dramc") == 0)
        {
                if (strcmp(argv[2], "clean") == 0 || strcmp(argv[2], "erase") == 0)
                {
                        // TODO : find dramc blk and erase
                }
                if (strcmp(argv[2], "clean") == 0)
                {
                        // TODO : write all 0xFF to dramc blk
                }

                return 0;
        }
        #endif

        nniaddr = (ulong)simple_strtoul(argv[1], NULL, 16);
        ppmaddr = (ulong)simple_strtoul(argv[2], NULL, 16);
        pniaddr = (ulong)simple_strtoul(argv[3], NULL, 16);
        hash0addr = (ulong)simple_strtoul(argv[4], NULL, 16);
        hash0size = (size_t)simple_strtoull(argv[5], NULL, 16);
        sbootaddr = (ulong)simple_strtoul(argv[6], NULL, 16);
        sbootsize = (size_t)simple_strtoull(argv[7], NULL, 16);
        ubootaddr = (ulong)simple_strtoul(argv[8], NULL, 16);
        ubootsize = (size_t)simple_strtoull(argv[9], NULL, 16);

        ret = drvNAND_FindMatchCIS((u8 *)nniaddr, (u8 *)pniaddr, &nniidx, &pniidx, &ppmidx);
        if (ret != 0)
        {
                printf("NAND find CIS failed %X\n", (u32)ret);
                return 1;
        }
        nniaddr += (nniidx * NNI_SIZE);
        pniaddr += (pniidx * PNI_SIZE);
        ppmaddr += (ppmidx * PPM_SIZE);

        offset = HASH_START_BLK * mtd->erasesize;
        /* write hash0 */
        ret = nand_write_hash(mtd, offset, hash0size, (u_char *)hash0addr, HASH0, &next_offset);
        if (ret != 0)
        {
                printf("NAND write hash0 failed %d\n", ret);
                return 1;
        }
        offset = next_offset;

        /* write hash0 bak*/
        ret = nand_write_hash(mtd, offset, hash0size, (u_char *)hash0addr, HASH0, &next_offset);
        if (ret != 0)
        {
                printf("NAND write hash0 bak failed %d\n", ret);
                return 1;
        }
        offset = next_offset;

        /* write sboot */
        ret = nand_write_hash(mtd, offset, sbootsize, (u_char *)sbootaddr, SBOOT, &next_offset);
        if (ret != 0)
        {
                printf("NAND write sboot failed %d\n", ret);
                return 1;
        }
        offset = next_offset;

        /* write sboot bak*/
        ret = nand_write_hash(mtd, offset, sbootsize, (u_char *)sbootaddr, SBOOT, &next_offset);
        if (ret != 0)
        {
                printf("NAND write sboot bak failed %d\n", ret);
                return 1;
        }
        offset = next_offset;

        /* write uboot */
        ret = nand_write_hash(mtd, offset, ubootsize, (u_char *)ubootaddr, UBOOT, &next_offset);
        if (ret != 0)
        {
                printf("NAND write uboot failed %d\n", ret);
                return 1;
        }
        offset = next_offset;

        /* write uboot bak*/
        ret = nand_write_hash(mtd, offset, ubootsize, (u_char *)ubootaddr, UBOOT, &next_offset);
        if (ret != 0)
        {
                printf("NAND write uboot bak failed %d\n", ret);
                return 1;
        }

        ret = drvNAND_WriteCIS((u8 *)nniaddr, (u8 *)pniaddr, (u8 *)ppmaddr);
        if (ret != 0)
        {
                printf("NAND write CIS failed %X\n", (u32)ret);
                return 1;
        }

        return 0;
}

U_BOOT_CMD(
        ncishash, CONFIG_SYS_MAXARGS, 0, do_ncishash,
        "NAND proprietary sub system",
        "ncishash nniaddr ppmaddr pniaddr\n"
        "         hash0addr hash0size\n"
        "         sbootaddr sbootsize\n"
        "         ubootaddr ubootsize\n"
        "\n"
        "ncishash dramc erase|clean\n"
        "         erase : erase dramc blk\n"
        "         clean : write 0xFF to dramc blk\n"
);

/**************************************************************************************************/
#define TEST_VOLUME             "test"
#define ROWSIZE                 16
#define GROUPSIZE               1
#define PWRCUT_DATA0            0x55
#define PWRCUT_DATA1            0xAA
#define PWRCUT_TEST_UNIT_CNT    32
#define PERCENTAGE              100

struct PWRCUT_CTRLBLK
{
        u32 u32_check_sum;
        u8 u8_target_unit;
        u8 au8_target_data[PWRCUT_TEST_UNIT_CNT];
};

static void down_count(int sec)
{
        #define DELAY_1s_in_us  1000000

        int i;
        ulong t0, t1;

        printf("\n");

        for (i = 0 ; i < sec; i++)
        {
                printf("%d ", sec-i);

                t0 = timer_get_boot_us();
                do
                {
                        t1 = timer_get_boot_us();
                } while ((t1 - t0) < DELAY_1s_in_us);
        }

        printf("\n");
}

static u32 check_sum(char *buffer, size_t length)
{
        u32 u32_check_sum = 0;
        int i;

        for (i = 0 ; i < length ; i++)
                u32_check_sum += buffer[i];

        return u32_check_sum;
}

static void dump_ctrl_blk(struct PWRCUT_CTRLBLK *p_ctrl_blk)
{
        printf("NAND: CheckSum: %Xh\n", p_ctrl_blk->u32_check_sum);
        printf("NAND: TargetUnit: %Xh\n", p_ctrl_blk->u8_target_unit);

        print_hex_dump("Target Data: ", DUMP_PREFIX_OFFSET, ROWSIZE, GROUPSIZE,
                (const void *)p_ctrl_blk->au8_target_data, PWRCUT_TEST_UNIT_CNT, 0);
}

static u32 check_ctrl_blk(struct ubi_volume_desc *ubi_desc, char *buffer)
{
        struct PWRCUT_CTRLBLK *p_ctrl_blk;
        u32 u32_check_sum;
        int ret;

        ret = ubi_leb_read(ubi_desc, 0, buffer, 0, ubi_desc->vol->ubi->leb_size, 0);
        if (ret)
        {
                printf("UBI Err: read CtrlBlk, Err:%d\n", ret);
                return 1;
        }

        p_ctrl_blk = (struct PWRCUT_CTRLBLK *)buffer;
        u32_check_sum = check_sum(buffer + sizeof(u32),
                sizeof(struct PWRCUT_CTRLBLK) - sizeof(u32));
        if (p_ctrl_blk->u32_check_sum != u32_check_sum)
        {
                printf("UBI Err: read CtrlBlk, CheckSum fail\n");
                return 1;
        }

        return 0;
}

static int pwrcut_initdata(struct ubi_volume_desc *ubi_desc, char *buffer)
{
        int i, j;
        struct PWRCUT_CTRLBLK *p_ctrl_blk;
        int lnum;
        int leb_size;
        int ret;

        p_ctrl_blk = (struct PWRCUT_CTRLBLK *)buffer;
        leb_size = ubi_desc->vol->ubi->leb_size;

        // ---------------------------------------
        p_ctrl_blk->u8_target_unit = ERASED_VAL;

        for (i = 0 ; i < PWRCUT_TEST_UNIT_CNT ; i++)
                p_ctrl_blk->au8_target_data[i] = PWRCUT_DATA0;

        p_ctrl_blk->u32_check_sum = check_sum((char *)&p_ctrl_blk->u8_target_unit,
                sizeof(struct PWRCUT_CTRLBLK) - sizeof(u32));

        ret = ubi_leb_change(ubi_desc, 0, (const void *)buffer, leb_size);
        if (ret)
        {
                printf("UBI Err: write CtrlBlk, Err:%d\n", ret);
                return 1;
        }

        // ---------------------------------------
        lnum += 1; // skip Ctrl Blk

        memset(buffer, PWRCUT_DATA0, leb_size);
        printf("atomic write ...\n");

        for (i = 0 ; i < PWRCUT_TEST_UNIT_CNT ; i++)
        {
                ret = ubi_leb_change(ubi_desc, i + 1, (const void *)buffer, leb_size);
                if (ret)
                {
                        printf("UBI Err: atomic write fail @ %u unit, Err:%d\n", i, ret);
                        return 1;
                }

                printf("%03u%%\r", ((i + 1) * PERCENTAGE) / PWRCUT_TEST_UNIT_CNT);
        }

        // ---------------------------------------
        printf("\n check ...\n");

        if (check_ctrl_blk(ubi_desc, buffer) != 0)
        {
                printf("UBI Err: read CtrlBlk fail\n");
                return 1;
        }

        for (i = 0 ; i < PWRCUT_TEST_UNIT_CNT ; i++)
        {
                ret = ubi_leb_read(ubi_desc, i + 1 , buffer, 0, leb_size, 0);
                if (ret)
                {
                        printf("UBI Err: read fail @ %u unit, Err:%d\n", i, ret);
                        return 1;
                }

                for (j = 0 ; j < leb_size ; j++)
                {
                        if (buffer[j] != PWRCUT_DATA0)
                        {
                                printf("UBI Err: Data Mismatch: unit:%u ByteIdx:%Xh ByteVal:%Xh\n",
                                        i, j, buffer[j]);
                                return 1;
                        }
                }

                printf("%03u%%\r", ((i + 1) * PERCENTAGE) / PWRCUT_TEST_UNIT_CNT);
        }

        printf("\n init success\n");

        return 0;
}

static int pwrcut_test(struct ubi_volume_desc *ubi_desc, char *wbuffer, char *rbuffer)
{
        int i, j;
        struct PWRCUT_CTRLBLK ctrl_blk;
        int lnum;
        int leb_size;
        int ret;

        // ---------------------------------------
        printf("\nUBI PwrCut Test, checking ...\n");

        // get Ctrl Blk
        if (check_ctrl_blk(ubi_desc, rbuffer) != 0)
        {
                printf("UBI Err: read CtrlBlk fail\n");
                return 1;
        }
        memcpy(&ctrl_blk, rbuffer, sizeof(struct PWRCUT_CTRLBLK));
        leb_size = ubi_desc->vol->ubi->leb_size;

        // check data
        for (i = 0 ; i < PWRCUT_TEST_UNIT_CNT ; i++)
        {
                ret = ubi_leb_read(ubi_desc, i + 1 , rbuffer, 0, leb_size, 0);
                if (ret)
                {
                        printf("UBI Err: read fail @ %u unit, Err:%d\n", i, ret);
                        return 1;
                }

                if (i == ctrl_blk.u8_target_unit)
                {
                        for (j = 0 ; j < leb_size ; j++)
                        {
                                if (rbuffer[j] != PWRCUT_DATA0 && rbuffer[j] != PWRCUT_DATA1)
                                {
                                        printf("UBI Err: Target Data Mismatch: Unit:%uh "
                                                "ByteIdx:%Xh ByteVal:%Xh\n",
                                                i + 1, j, rbuffer[j]);
                                        dump_ctrl_blk(&ctrl_blk);
                                        print_hex_dump("", DUMP_PREFIX_ADDRESS, ROWSIZE, GROUPSIZE,
                                                (const void *)rbuffer, leb_size, 0);

                                        return 1;
                                }
                        }
                        // recover TargetUnit
                        lnum = ctrl_blk.u8_target_unit + 1;
                        memset(wbuffer, ctrl_blk.au8_target_data[ctrl_blk.u8_target_unit],
                                leb_size);
                        ret = ubi_leb_change(ubi_desc, lnum, (const void *)wbuffer, leb_size);
                        if (ret)
                        {
                                printf("UBI Err: atomic write fail @ %u unit, Err:%d\n",
                                        ctrl_blk.u8_target_unit, ret);
                                return 1;
                        }
                }
                else
                {
                        for (j = 0 ; j < leb_size ; j++)
                        {
                                if (rbuffer[j] != ctrl_blk.au8_target_data[i])
                                {
                                        printf("UBI Err: Data Mismatch: Unit:%uh "
                                                "ByteIdx:%Xh ByteVal:%Xh\n",
                                                i + 1, j, rbuffer[j]);
                                        dump_ctrl_blk(&ctrl_blk);
                                        print_hex_dump("", DUMP_PREFIX_ADDRESS, ROWSIZE, GROUPSIZE,
                                                (const void *)rbuffer, leb_size, 0);

                                        return 1;
                                }
                        }
                }

                printf("%03u%%\r", ((i + 1) * PERCENTAGE) / PWRCUT_TEST_UNIT_CNT);
        }

        printf("\n   ok\n");

        // ---------------------------------------
        while (1)
        {
                ctrl_blk.u8_target_unit = timer_get_boot_us() % PWRCUT_TEST_UNIT_CNT;
                ctrl_blk.au8_target_data[ctrl_blk.u8_target_unit] ^= (~((u8)0));
                ctrl_blk.u32_check_sum = check_sum((char *)&ctrl_blk.u8_target_unit,
                        sizeof(struct PWRCUT_CTRLBLK) - sizeof(u32));
                memset((void *)wbuffer, 0, leb_size);
                memcpy((void *)wbuffer, (const void *)&ctrl_blk, sizeof(struct PWRCUT_CTRLBLK));
                ret = ubi_leb_change(ubi_desc, 0, (const void *)wbuffer, leb_size);
                if (ret)
                {
                        printf("UBI Err: write CtrlBlk, Err:%x\n", ret);
                        return 1;
                }

                memset(wbuffer, ctrl_blk.au8_target_data[ctrl_blk.u8_target_unit], leb_size);

                printf("\nWriting Unit %u, Data %02Xh ...\n",
                        ctrl_blk.u8_target_unit, ctrl_blk.au8_target_data[ctrl_blk.u8_target_unit]);

                ret = ubi_leb_change(ubi_desc, ctrl_blk.u8_target_unit + 1, (const void *)wbuffer,
                        leb_size);
                if (ret)
                {
                        printf("UBI Err: write Fail, Err:%u\n", ret);
                        return 1;
                }

                printf("reading back and compare...\n");
                ret = ubi_leb_read(ubi_desc, ctrl_blk.u8_target_unit + 1, rbuffer, 0, leb_size, 0);
                if (ret)
                {
                        printf("UBI Err: read Fail, Err:%d\n", ret);
                        return 1;
                }

                if (memcmp(wbuffer, rbuffer, leb_size))
                {
                        printf("UBI Err: cmp Fail\n");
                        return 1;
                }

                printf("ok\n");
        }

        return 0;
}

static int nandtest_pwrcut_init(void)
{
        struct ubi_volume_desc *ubi_desc;
        char *buffer;
        int ret;

        if (run_command("ubi check test", 0) != 0)
        {
                if (run_command("ubi create test -", 0) != 0)
                {
                        printf("ubi create volume fail\n");
                        return 1;
                }
        }

        ubi_desc = ubi_open_volume_nm(0, "test", UBI_READWRITE);
        if (IS_ERR(ubi_desc))
        {
                printf("ubi open volume fail\n");
                return 1;
        }

        buffer = kmalloc(ubi_desc->vol->ubi->leb_size, GFP_KERNEL);
        if (!buffer)
        {
                printf("allocate memory fail\n");
                return 1;
        }

        ret = pwrcut_initdata(ubi_desc, buffer);

        ubi_close_volume(ubi_desc);

        kfree(buffer);

        return ret;
}

static int nandtest_pwrcut_test(void)
{
        struct ubi_volume_desc *ubi_desc;
        char *wbuffer, *rbuffer;
        int ret;

        ubi_desc = ubi_open_volume_nm(0, "test", UBI_READWRITE);
        if (IS_ERR(ubi_desc))
        {
                printf("ubi open volume fail\n");
                return 1;
        }

        wbuffer = kmalloc(ubi_desc->vol->ubi->leb_size, GFP_KERNEL);
        rbuffer = kmalloc(ubi_desc->vol->ubi->leb_size, GFP_KERNEL);
        if (!wbuffer || !rbuffer)
        {
                printf("allocate memory fail\n");
                return 1;
        }

        ret = pwrcut_test(ubi_desc, wbuffer, rbuffer);

        ubi_close_volume(ubi_desc);

        kfree(wbuffer);
        kfree(rbuffer);

        return ret;
}

static int nandtest_speed(struct mtd_info *mtd, loff_t offset, size_t length)
{
        #define TEST_DELAY      3

        int ret = 0;
        ulong t, t0, t1;
        int i;
        char *wbuffer, *rbuffer;
        size_t rwsize;
        nand_erase_options_t opts;

        down_count(TEST_DELAY);

        wbuffer = kmalloc(mtd->erasesize, GFP_KERNEL);
        rbuffer = kmalloc(mtd->erasesize, GFP_KERNEL);
        if (!wbuffer || !rbuffer)
        {
                printf("allocate memory fail\n");
                return 1;
        }

        for (i = 0 ; i < mtd->erasesize ; i++)
                wbuffer[i] = i;

        memset(&opts, 0, sizeof(opts));
        opts.offset = offset;
        opts.length = mtd->erasesize;
        opts.spread = 1;
        opts.lim = length;
        ret = nand_erase_opts(mtd, &opts);
        if (ret != 0)
        {
                printf("nand_erase_opts fail %d\n", ret);
                ret = 1;
                goto exit;
        }

        rwsize = mtd->erasesize;
        t0 = timer_get_boot_us();
        ret = nand_write_skip_bad(mtd, offset, &rwsize, NULL, length, (u_char *)wbuffer, 0);
        if (ret != 0)
        {
                printf("nand_write_skip_bad fail %d\n", ret);
                ret = 1;
                goto exit;
        }
        t1 = timer_get_boot_us();
        t = (t1 > t0) ? (t1 - t0) : (~((u32)0) - t0 + t1);
        printf("write : %lu MB/s\n", mtd->erasesize/t);

        memset((void *)rbuffer, 0, mtd->erasesize);
        rwsize = mtd->erasesize;
        t0 = timer_get_boot_us();
        ret = nand_read_skip_bad(mtd, offset, &rwsize, NULL, length, (u_char *)rbuffer);
        if (ret != 0)
        {
                printf("nand_read_skip_bad fail %d\n", ret);
                ret = 1;
                goto exit;
        }
        t1 = timer_get_boot_us();
        for (i = 0 ; i < mtd->erasesize ; i++)
        {
                if (wbuffer[i] != rbuffer[i])
                {
                        printf("data comparison fail\n");
                        ret = 1;
                        goto exit;
                }
        }
        t = (t1 > t0) ? (t1 - t0) : (~((u32)0) - t0 + t1);
        printf("read : %lu MB/s\n", mtd->erasesize/t);

exit:
        kfree(wbuffer);
        kfree(rbuffer);

        return 0;
}

static int nandtest_crazywr(struct mtd_info *mtd, loff_t offset, size_t length,
        int argc, char *const argv[])
{
        int ret = 0;
        int i;
        char *wbuffer, *rbuffer;
        unsigned short *p;
        size_t rwsize;
        struct erase_info erase;
        loff_t off;
        int cnt;

        cnt = simple_strtoul(argv[0], NULL, 16);
        if (cnt != (argc - 1))
        {
                printf("pattern count is not the same as argument count\n");
                return 1;
        }

        wbuffer = kmalloc(mtd->erasesize, GFP_KERNEL);
        rbuffer = kmalloc(mtd->erasesize, GFP_KERNEL);
        if (!wbuffer || !rbuffer)
        {
                printf("allocate memory fail\n");
                return 1;
        }

        p = (unsigned short *)wbuffer;
        for (i = 0 ; i < (mtd->erasesize >> 1) ; i++)
                p[i] = simple_strtoul(argv[(i % cnt) + 1], NULL, 16);
        print_hex_dump("crazy pattern ", DUMP_PREFIX_OFFSET, ROWSIZE, GROUPSIZE,
                (const void *)wbuffer, mtd->writesize, 0);

        off = offset;
        do
        {
                if (off == (offset + length))
                {
                        off = offset;
                }

                printf("crazy test at 0x%08llx\n", off);

                ret = mtd_block_isbad(mtd, off);
                if (ret != 0)
                {
                        printf("Skipping bad block at 0x%08llx\n", off);
                        off += mtd->erasesize;
                        continue;
                }

                memset(&erase, 0, sizeof(erase));
                erase.mtd = mtd;
                erase.len = mtd->erasesize;
                erase.addr = off;
                ret = mtd_erase(mtd, &erase);
                if (ret != 0)
                {
                        printf("mtd_erase fail %d at 0x%08llx\n", ret, off);
                        if (ret == -EIO)
                        {
                                ret = mtd_block_markbad(mtd, off);
                                if (ret != 0)
                                {
                                        printf("mtd_block_markbad fail %d at 0x%08llx\n", ret, off);
                                        goto exit;
                                }
                        }
                        off += mtd->erasesize;
                        continue;
                }

                rwsize = mtd->erasesize;
                ret = nand_write_skip_bad(mtd, off, &rwsize, NULL, rwsize, (u_char *)wbuffer, 0);
                if (ret != 0)
                {
                        printf("nand_write_skip_bad fail %d\n", ret);
                        ret = 1;
                        goto exit;
                }

                memset((void *)rbuffer, 0, mtd->erasesize);
                rwsize = mtd->erasesize;
                ret = nand_read_skip_bad(mtd, offset, &rwsize, NULL, rwsize, (u_char *)rbuffer);
                if (ret != 0)
                {
                        printf("nand_read_skip_bad fail %d\n", ret);
                        ret = 1;
                        goto exit;
                }

                for (i = 0 ; i < mtd->erasesize ; i++)
                {
                        if (wbuffer[i] != rbuffer[i])
                        {
                                printf("data comparison fail\n");
                                ret = 1;
                                goto exit;
                        }
                }

                off += mtd->erasesize;
        } while (ret == 0);
exit:
        kfree(wbuffer);
        kfree(rbuffer);

        return ret;
}

static int do_nandtest(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
        #define COMMAND_SIZE    32
        int ret = 0;
        loff_t off, size, maxsize;
        char command[COMMAND_SIZE];
        char *cmd, *subcmd;
        struct mtd_info *mtd;
        int dev = nand_curr_device;

        /* at least 3 arguments please */
        if (argc < 3)
        {
                goto usage;
        }

        cmd = argv[1];

        mtd = get_nand_dev_by_index(dev);
        if (!mtd)
        {
                puts("\nno devices available\n");
                return 1;
        }

        if (strcmp(cmd, "speed") == 0)
        {
                if (mtd_arg_off(argv[2], &dev, &off, &size, &maxsize,
                        MTD_DEV_TYPE_NAND,
                        mtd->size))
                {
                        return 1;
                }

                ret = nandtest_speed(mtd, off, size);

                return ret;
        }

        if (strcmp(cmd, "pwr_cut") == 0)
        {
                if (argc < 4)
                {
                        goto usage;
                }

                snprintf(command, COMMAND_SIZE, "ubi part %s", argv[3]);
                if (run_command(command, 0) != 0)
                {
                        printf("ubi part fail\n");
                        return 1;
                }

                subcmd = argv[2];

                if (strcmp(subcmd, "init") == 0)
                {
                        ret = nandtest_pwrcut_init();
                }
                else if (strcmp(subcmd, "test") == 0)
                {
                        ret = nandtest_pwrcut_test();
                }
                else
                {
                        printf("invalid command\n");
                        return 1;
                }

                return ret;
        }

        if (strcmp(cmd, "crazywr") == 0)
        {
                if (mtd_arg_off(argv[2], &dev, &off, &size, &maxsize,
                        MTD_DEV_TYPE_NAND,
                        mtd->size))
                {
                        return 1;
                }

                ret = nandtest_crazywr(mtd, off, size, argc - 3, argv + 3);

                return ret;
        }

usage:
        return CMD_RET_USAGE;
}

U_BOOT_CMD(
        nandtest, CONFIG_SYS_MAXARGS, 0, do_nandtest,
        "NAND proprietary test sub system",
        "add TEST partition for the following nandtest command"
        "       - mtdparts add nand0 0x1000000 TEST\n"
        "nandtest speed partition - nandtest speed test\n"
        "       - nandtest speed TEST\n"
        "nandtest pwr_cut init partition - nandtest power cut init\n"
        "       - nandtest pwr_cut init TEST\n"
        "nandtest pwr_cut test partition - nandtest power cut test\n"
        "       - nandtest pwr_cut test TEST\n"
        "nandtest crazywr partition cnt pattern1 pattern2 ... - nandtest crazywr test\n"
        "       - nandtest crazywr TEST 0x10 0x01fe 0xfe01 0x02fd 0xfd02 0x04fb 0xfb04 0x08f7"
        " 0xf708 0x10ef 0xef10 0x20df 0xdf20 0x40bf 0xbf40 0x807f 0x7f80\n"
);