/*
 * idme_platform.c
 *
 * Copyright 2011 - 2023 Amazon.com, Inc. or its Affiliates. All rights reserved.
 */
#include <common.h>
#include <config.h>
#include <mmc.h>
#include <asm/byteorder.h>
#include <asm/unaligned.h>
#include <idme.h>
#include "eMMC.h"
#include <mmc.h>
#include "ufbl_debug.h"
//#include "part_emmc.h"
#include <malloc.h>
/* module debug */
//#define MODULE_DEBUG

#define CFG_FASTBOOT_MMC_NO 0

/* read entire idme */
int idme_platform_read(unsigned char *pbuf)
{
	U8 *readBuf = NULL;
	U32 u32_err = EMMC_ST_SUCCESS;
	struct mmc *mmc;
	u64 block_offset =0;

	mmc = find_mmc_device(CFG_FASTBOOT_MMC_NO);
	if (!mmc) {
		dprintf(CRITICAL, "no mmc devices available\n");
		return -1;
	}

	if (mmc_switch_part(mmc, CONFIG_IDME_PARTITION_NUM) != 0) {
		dprintf(CRITICAL, "ERROR: couldn't switch to boot partition\n");
		return -1;
	}

	block_offset = mmc->capacity/CONFIG_MMC_BLOCK_SIZE;
	block_offset = block_offset > IDME_BLOCK_OFFSET_FROM_END ? block_offset - IDME_BLOCK_OFFSET_FROM_END : 0;

	readBuf = (U8*)memalign(EMMC_CACHE_LINE, CONFIG_IDME_SIZE);
	u32_err = emmc_init();
	if(EMMC_ST_SUCCESS != u32_err)
	{
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: init fail: %Xh \n", u32_err);
		goto exit_idme_platform_read;
	}
        /* read from boot partition*/
	u32_err = emmc_read_boot_part(readBuf, CONFIG_IDME_SIZE, block_offset, CONFIG_IDME_PARTITION_NUM);
	if(EMMC_ST_SUCCESS != u32_err)
        {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: read BootPart 2 fail: %Xh \n", u32_err);
		goto exit_idme_platform_read;
        }
        memcpy(pbuf,readBuf,CONFIG_IDME_SIZE);
#if defined(MODULE_DEBUG)
	hexdump8(pbuf, CONFIG_IDME_SIZE);
#endif
	exit_idme_platform_read:
		if(readBuf){
			free(readBuf);
			readBuf = NULL;
		}
	return 0;
}

/* write idme */
int idme_platform_write(const unsigned char *pbuf)
{
	U8 *writeBuf = NULL;
	U32 u32_err = EMMC_ST_SUCCESS;
        struct mmc *mmc;
        u64 block_offset =0;

        mmc = find_mmc_device(CFG_FASTBOOT_MMC_NO);
        if (!mmc) {
                dprintf(CRITICAL, "no mmc devices available\n");
                return -1;
        }

	unsigned char part_num = mmc_get_blk_desc(mmc)->hwpart;
        if (mmc_switch_part(mmc, CONFIG_IDME_PARTITION_NUM) != 0) {
                dprintf(CRITICAL, "ERROR: couldn't switch to boot partition\n");
                return -1;
        }

	block_offset = mmc->capacity/CONFIG_MMC_BLOCK_SIZE;
	block_offset = block_offset > IDME_BLOCK_OFFSET_FROM_END ? block_offset - IDME_BLOCK_OFFSET_FROM_END : 0;

	writeBuf = (U8*)memalign(EMMC_CACHE_LINE, CONFIG_IDME_SIZE);
	u32_err = emmc_init();
	if(EMMC_ST_SUCCESS != u32_err)
	{
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: init fail: %Xh \n", u32_err);
		goto exit_idme_platform_write;
	}
	/* write to boot partition */
	memcpy(writeBuf,pbuf,CONFIG_IDME_SIZE);
	u32_err = emmc_write_boot_part(writeBuf, CONFIG_IDME_SIZE, block_offset, CONFIG_IDME_PARTITION_NUM);
	if(EMMC_ST_SUCCESS != u32_err)
	{
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: read BootPart 2 fail: %Xh \n", u32_err);
		goto exit_idme_platform_write;
	}

#if defined(MODULE_DEBUG)
	hexdump8(pbuf, CONFIG_IDME_SIZE);
#endif
	exit_idme_platform_write:
		if(writeBuf){
			free(writeBuf);
			writeBuf = NULL;
		}

		/* switch back to previous partition */
		if (mmc_switch_part(mmc, part_num) != 0) {
			dprintf(CRITICAL, "ERROR: couldn't switch to partition %u\n", part_num);
			return -1;
		}

	return 0;
}

