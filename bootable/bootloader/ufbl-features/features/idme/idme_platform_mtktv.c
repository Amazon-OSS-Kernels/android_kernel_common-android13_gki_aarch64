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

extern u32 mtk_fcie_read_boot_part(void *data_buf, u32 data_byte_cnt, u32 blk_addr, u8 part_no);
extern u32 mtk_fcie_write_boot_part(const void *data_buf, u32 data_byte_cnt, u32 blk_addr, u8 part_no);

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

	block_offset = mmc->capacity_boot/CONFIG_MMC_BLOCK_SIZE;
	block_offset = block_offset > IDME_BLOCK_OFFSET_FROM_END ? block_offset - IDME_BLOCK_OFFSET_FROM_END : 0;

	readBuf = (U8*)memalign(EMMC_CACHE_LINE, CONFIG_IDME_SIZE);

    /* read from boot partition*/
	u32_err = mtk_fcie_read_boot_part(readBuf, CONFIG_IDME_SIZE, block_offset, CONFIG_IDME_PARTITION_NUM);
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

	block_offset = mmc->capacity_boot/CONFIG_MMC_BLOCK_SIZE;
	block_offset = block_offset > IDME_BLOCK_OFFSET_FROM_END ? block_offset - IDME_BLOCK_OFFSET_FROM_END : 0;

	writeBuf = (U8*)memalign(EMMC_CACHE_LINE, CONFIG_IDME_SIZE);

	/* write to boot partition */
	memcpy(writeBuf,pbuf,CONFIG_IDME_SIZE);
	u32_err = mtk_fcie_write_boot_part(writeBuf, CONFIG_IDME_SIZE, block_offset, CONFIG_IDME_PARTITION_NUM);
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

	return 0;
}

