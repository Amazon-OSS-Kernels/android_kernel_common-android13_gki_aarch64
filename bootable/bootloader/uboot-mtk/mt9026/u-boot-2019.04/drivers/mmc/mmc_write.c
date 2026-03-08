// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2008, Freescale Semiconductor, Inc
 * Andy Fleming
 *
 * Based vaguely on the Linux code
 */

#include <config.h>
#include <common.h>
#include <dm.h>
#include <part.h>
#include <div64.h>
#include <linux/math64.h>
#include "mmc_private.h"
#ifdef CONFIG_MULTICORES_PLATFORM
#include <smp/thread_info.h>
extern smp_spin_lock_t emmc_spin_lock;
#endif

static ulong mmc_erase_t(struct mmc *mmc, ulong start, lbaint_t blkcnt)
{
	struct mmc_cmd cmd;
	ulong end;
	int err, start_cmd, end_cmd;

	if (mmc->high_capacity) {
		end = start + blkcnt - 1;
	} else {
		end = (start + blkcnt - 1) * mmc->write_bl_len;
		start *= mmc->write_bl_len;
	}

	if (IS_SD(mmc)) {
		start_cmd = SD_CMD_ERASE_WR_BLK_START;
		end_cmd = SD_CMD_ERASE_WR_BLK_END;
	} else {
		start_cmd = MMC_CMD_ERASE_GROUP_START;
		end_cmd = MMC_CMD_ERASE_GROUP_END;
	}

	cmd.cmdidx = start_cmd;
	cmd.cmdarg = start;
	cmd.resp_type = MMC_RSP_R1;

	err = mmc_send_cmd(mmc, &cmd, NULL);
	if (err)
		goto err_out;

	cmd.cmdidx = end_cmd;
	cmd.cmdarg = end;

	err = mmc_send_cmd(mmc, &cmd, NULL);
	if (err)
		goto err_out;

	cmd.cmdidx = MMC_CMD_ERASE;
	if (mmc->erase_feature_support & SEC_GB_CL_EN)
		cmd.cmdarg = MMC_TRIM_ARG;
	else
		cmd.cmdarg = MMC_ERASE_ARG;

	cmd.resp_type = MMC_RSP_R1b;

	err = mmc_send_cmd(mmc, &cmd, NULL);
	if (err)
		goto err_out;

	return 0;

err_out:
	puts("mmc erase failed\n");
	return err;
}

#define onetime_erase_sectorcnt  (1024 * 1024 * 1024) >> 9
#define max_erase_timeout 60000
#if CONFIG_IS_ENABLED(BLK)
ulong mmc_berase(struct udevice *dev, lbaint_t start, lbaint_t blkcnt)
#else
ulong mmc_berase(struct blk_desc *block_dev, lbaint_t start, lbaint_t blkcnt)
#endif
{
	#ifdef CONFIG_MULTICORES_PLATFORM
	unsigned long irq_flag = 0;
	#endif
#if CONFIG_IS_ENABLED(BLK)
	struct blk_desc *block_dev = dev_get_uclass_platdata(dev);
#endif
	int dev_num = block_dev->devnum;
	int err = 0, i;
	u32 start_rem, blkcnt_rem;
	u32 Onetimeerase_sectorcnt, erase_sectorcnt, erase_start, erase_end;
	struct mmc *mmc = find_mmc_device(dev_num);
	lbaint_t blk = 0, blk_r = 0;
	int timeout_ms = max_erase_timeout;
	lbaint_t end, hblkcnt = 0 ;
	char buf[MMC_MAX_BLOCK_LEN];

	if (!mmc)
		return -1;

	err = blk_select_hwpart_devnum(IF_TYPE_MMC, dev_num,
				       block_dev->hwpart);
	if (err < 0)
		return -1;


	if (mmc->erase_feature_support & SEC_GB_CL_EN) {
		Onetimeerase_sectorcnt = onetime_erase_sectorcnt;

		if (blkcnt <= Onetimeerase_sectorcnt) {
			#ifdef CONFIG_MULTICORES_PLATFORM
			smp_spin_lock_save(&emmc_spin_lock, irq_flag);
			#endif
			err = mmc_erase_t(mmc, start, blkcnt);
			if (err) {
				#ifdef CONFIG_MULTICORES_PLATFORM
				smp_spin_unlock_restore(&emmc_spin_lock, irq_flag);
				#endif
				return 0;
			}
			if (mmc_poll_for_busy(mmc, timeout_ms)) {
				#ifdef CONFIG_MULTICORES_PLATFORM
				smp_spin_unlock_restore(&emmc_spin_lock, irq_flag);
				#endif
				return 0;
			}
		} else {
			#ifdef CONFIG_MULTICORES_PLATFORM
			smp_spin_lock_save(&emmc_spin_lock, irq_flag);
			#endif
			erase_start = start;
			erase_end = start + (blkcnt / Onetimeerase_sectorcnt) * Onetimeerase_sectorcnt;
			for (i = 0; i < (blkcnt / Onetimeerase_sectorcnt); i++) {
				err = mmc_erase_t(mmc, erase_start, Onetimeerase_sectorcnt);
				if (err) {
					#ifdef CONFIG_MULTICORES_PLATFORM
					smp_spin_unlock_restore(&emmc_spin_lock, irq_flag);
					#endif
					return 0;
				}
				if (mmc_poll_for_busy(mmc, timeout_ms)) {
					#ifdef CONFIG_MULTICORES_PLATFORM
					smp_spin_unlock_restore(&emmc_spin_lock, irq_flag);
					#endif
					return 0;
				}
				erase_start += Onetimeerase_sectorcnt;
			}

			erase_sectorcnt =  blkcnt - (erase_end - start);
			if (erase_sectorcnt) {
				err = mmc_erase_t(mmc, erase_end, erase_sectorcnt);
				if (err) {
					#ifdef CONFIG_MULTICORES_PLATFORM
					smp_spin_unlock_restore(&emmc_spin_lock, irq_flag);
					#endif
					return 0;
				}
				/* Waiting for the ready status */
				if (mmc_poll_for_busy(mmc, timeout_ms)) {
					#ifdef CONFIG_MULTICORES_PLATFORM
					smp_spin_unlock_restore(&emmc_spin_lock, irq_flag);
					#endif
					return 0;
				}
			}
		}
		blk = blkcnt;
	} else {
		memset(buf, 0, MMC_MAX_BLOCK_LEN);
		err = div_u64_rem(start, mmc->erase_grp_size, &start_rem);
		if (start_rem) {
			start_rem = (blkcnt > mmc->erase_grp_size - start_rem) ? mmc->erase_grp_size - start_rem : blkcnt;
			for (blk = start; blk < start + start_rem; blk++)
				mmc_bwrite(dev, blk, 1, buf);
			start = start + start_rem;
			blkcnt -= start_rem;
			hblkcnt += start_rem;
		}

		end = blkcnt + start;
		err = div_u64_rem(blkcnt, mmc->erase_grp_size, &blkcnt_rem);
		if (blkcnt_rem) {
			blkcnt_rem = end - blkcnt_rem;
			for (blk = blkcnt_rem; blk < end; blk++)
				mmc_bwrite(dev, blk, 1, buf);
			blkcnt -= end - blkcnt_rem;
			hblkcnt += end - blkcnt_rem;
		}

		blk = 0;
		if (!blkcnt)
			return hblkcnt;
		/*
		 * We want to see if the requested start or total block count are
		 * unaligned.  We discard the whole numbers and only care about the
		 * remainder.
		 */
		err = div_u64_rem(start, mmc->erase_grp_size, &start_rem);
		err = div_u64_rem(blkcnt, mmc->erase_grp_size, &blkcnt_rem);
		if (start_rem || blkcnt_rem) {
			printf("\n\nCaution! Your devices Erase group is 0x%x\n", mmc->erase_grp_size);
			printf("The erase range would be change to ");
			printf("0x" LBAF, start & ~(mmc->erase_grp_size - 1));
			printf("~0x" LBAF "\n\n", ((start + blkcnt + mmc->erase_grp_size) & ~(mmc->erase_grp_size - 1)) - 1);
		}
		#ifdef CONFIG_MULTICORES_PLATFORM
		smp_spin_lock_save(&emmc_spin_lock, irq_flag);
		#endif
		while (blk < blkcnt) {
			if (IS_SD(mmc) && mmc->ssr.au) {
				blk_r = ((blkcnt - blk) > mmc->ssr.au) ?
					mmc->ssr.au : (blkcnt - blk);
			} else {
				blk_r = ((blkcnt - blk) > mmc->erase_grp_size) ?
					mmc->erase_grp_size : (blkcnt - blk);
			}
			err = mmc_erase_t(mmc, start + blk, blk_r);
			if (err)
				break;

			blk += blk_r;

			/* Waiting for the ready status */
			if (mmc_poll_for_busy(mmc, timeout_ms)) {
				#ifdef CONFIG_MULTICORES_PLATFORM
				smp_spin_unlock_restore(&emmc_spin_lock, irq_flag);
				#endif
				return 0;
			}
		}
		blk += hblkcnt;
	}
	#ifdef CONFIG_MULTICORES_PLATFORM
	smp_spin_unlock_restore(&emmc_spin_lock, irq_flag);
	#endif
	return blk;
}

static ulong mmc_write_blocks(struct mmc *mmc, lbaint_t start,
		lbaint_t blkcnt, const void *src)
{
	struct mmc_cmd cmd;
	struct mmc_data data;
	int timeout_ms = 1000;

	if ((start + blkcnt) > mmc_get_blk_desc(mmc)->lba) {
		printf("MMC: block number 0x" LBAF " exceeds max(0x" LBAF ")\n",
		       start + blkcnt, mmc_get_blk_desc(mmc)->lba);
		return 0;
	}

	if (blkcnt == 0)
		return 0;

    #if 1

	if ( blkcnt > 1) {
		cmd.cmdidx = MMC_CMD_SET_BLOCK_COUNT;
		cmd.cmdarg = blkcnt;
        cmd.cmdarg |= MMC_RELIABLE_WRITE_ARG;
		cmd.resp_type = MMC_RSP_R1;
		if (mmc_send_cmd(mmc, &cmd, NULL)) {
			printf("mmc fail to send set block count cmd\n");
			return 0;
		}
	}
    #endif

    
	if (blkcnt == 1)
		cmd.cmdidx = MMC_CMD_WRITE_SINGLE_BLOCK;
	else
		cmd.cmdidx = MMC_CMD_WRITE_MULTIPLE_BLOCK;

	if (mmc->high_capacity)
		cmd.cmdarg = start;
	else
		cmd.cmdarg = start * mmc->write_bl_len;

	cmd.resp_type = MMC_RSP_R1;

	data.src = src;
	data.blocks = blkcnt;
	data.blocksize = mmc->write_bl_len;
	data.flags = MMC_DATA_WRITE;

	if (mmc_send_cmd(mmc, &cmd, &data)) {
		printf("mmc write failed\n");
		return 0;
	}

    #if 0
	/* SPI multiblock writes terminate using a special
	 * token, not a STOP_TRANSMISSION request.
	 */
	if (!mmc_host_is_spi(mmc) && blkcnt > 1) {
		cmd.cmdidx = MMC_CMD_STOP_TRANSMISSION;
		cmd.cmdarg = 0;
		cmd.resp_type = MMC_RSP_R1b;
		if (mmc_send_cmd(mmc, &cmd, NULL)) {
			printf("mmc fail to send stop cmd\n");
			return 0;
		}
	}
    #endif

	/* Waiting for the ready status */
	if (mmc_poll_for_busy(mmc, timeout_ms))
		return 0;

	return blkcnt;
}

#if CONFIG_IS_ENABLED(BLK)
ulong mmc_bwrite(struct udevice *dev, lbaint_t start, lbaint_t blkcnt,
		 const void *src)
#else
ulong mmc_bwrite(struct blk_desc *block_dev, lbaint_t start, lbaint_t blkcnt,
		 const void *src)
#endif
{
	#ifdef CONFIG_MULTICORES_PLATFORM
	unsigned long irq_flag = 0;
	smp_spin_lock_save(&emmc_spin_lock, irq_flag);
	#endif
#if CONFIG_IS_ENABLED(BLK)
	struct blk_desc *block_dev = dev_get_uclass_platdata(dev);
#endif
	int dev_num = block_dev->devnum;
	lbaint_t cur, blocks_todo = blkcnt;
	int err;

	struct mmc *mmc = find_mmc_device(dev_num);
	if (!mmc) {
		#ifdef CONFIG_MULTICORES_PLATFORM
		smp_spin_unlock_restore(&emmc_spin_lock, irq_flag);
		#endif
		return 0;
	}
	err = blk_select_hwpart_devnum(IF_TYPE_MMC, dev_num, block_dev->hwpart);
	if (err < 0) {
		#ifdef CONFIG_MULTICORES_PLATFORM
		smp_spin_unlock_restore(&emmc_spin_lock, irq_flag);
		#endif      
		return 0;
	}
	if (mmc_set_blocklen(mmc, mmc->write_bl_len)) {
		#ifdef CONFIG_MULTICORES_PLATFORM
		smp_spin_unlock_restore(&emmc_spin_lock, irq_flag);
		#endif      
		return 0;
	}
	do {
		cur = (blocks_todo > mmc->cfg->b_max) ?
			mmc->cfg->b_max : blocks_todo;
		if (mmc_write_blocks(mmc, start, cur, src) != cur) {
			#ifdef CONFIG_MULTICORES_PLATFORM
			smp_spin_unlock_restore(&emmc_spin_lock, irq_flag);
			#endif
			return 0;
		}
		blocks_todo -= cur;
		start += cur;
		src += cur * mmc->write_bl_len;
	} while (blocks_todo > 0);
	#ifdef CONFIG_MULTICORES_PLATFORM
	smp_spin_unlock_restore(&emmc_spin_lock, irq_flag);
	#endif

	return blkcnt;
}
