// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (C) 2017 The Android Open Source Project
 */

#include <android_ab.h>
#include <android_bl_msg.h>
#include <common.h>
#include <memalign.h>
#include <u-boot/crc.h>
#include <asm/io.h>
#include <utility.h>
#include <debug_impl.h>
#include <mtk-pm.h>

#if defined(CONFIG_AB_SIDELOAD)
//-------------------------------------------------------------------------------------------------
//  Local Variables
//-------------------------------------------------------------------------------------------------
static unsigned char st_select_success_ful_boot = 0;
/*TODO: confirm base address (0x1C040400)(0x1C040480) is fixed in M6 family in the future*/
#define ROMCODE_AB_RETRY_COUNT	0x1C040400
#define ROMCODE_AB_SLOT_NUM	0x1C040480
#define AB_PRIORITY_MAX	15
static uint32_t ab_get_retry_count(void)
{
    return (AB_RETRY_COUNT_MAXIMUM - (readb(ROMCODE_AB_RETRY_COUNT) & 0x3)); // only 3-bit
}

static void ab_reset_retry_count(void)
{
    unsigned char reg=readb(ROMCODE_AB_RETRY_COUNT);
    printf("!!!!! retry_count: %d reset to 0  !!!!! \n",reg);
    writeb(reg&0xC,ROMCODE_AB_RETRY_COUNT);
}

static uint32_t ab_slot_num(void)
{
#ifndef CONFIG_AB_FROM_ROM
	return CONFIG_AB_FIXED_SLOT;
#else
	return (readb(ROMCODE_AB_SLOT_NUM) & 0x1); // if 0, it's slot A; if 1, it's slot B
#endif
}

int ab_pre_select_slot(void)
{
	return ab_slot_num();
}

static void ab_set_select_successful_boot(unsigned char is_success)
{
	st_select_success_ful_boot = is_success;
}

unsigned char ab_get_select_successful_boot(void)
{
	return st_select_success_ful_boot;
}
#endif

/**
 * Compute the CRC-32 of the bootloader control struct.
 *
 * Only the bytes up to the crc32_le field are considered for the CRC-32
 * calculation.
 */
static uint32_t ab_control_compute_crc(struct andr_bl_control *abc)
{
	return crc32(0, (void *)abc, offsetof(typeof(*abc), crc32_le));
}

/**
 * Initialize andr_bl_control to the default value.
 *
 * It allows us to boot all slots in order from the first one. This value
 * should be used when the bootloader message is corrupted, but not when
 * a valid message indicates that all slots are unbootable.
 */
static int ab_control_default(struct andr_bl_control *abc)
{
	int i;
	const struct andr_slot_metadata metadata = {
		.priority = 15,
#if defined(CONFIG_AB_SIDELOAD) && defined(AB_RETRY_COUNT_MAXIMUM)
		.tries_remaining = AB_RETRY_COUNT_MAXIMUM,
#else
		.tries_remaining = 7,
#endif
#ifdef ENABLE_VTS_TEST
		.successful_boot = 1,
#else
		.successful_boot = 0,
#endif
		.verity_corrupted = 0,
		.reserved = 0
	};

	if (!abc)
		return -EINVAL;

	memcpy(abc->slot_suffix, "a\0\0\0", 4);
	abc->magic = ANDROID_BOOT_CTRL_MAGIC;
	abc->version = ANDROID_BOOT_CTRL_VERSION;
	abc->nb_slot = ANDROID_NUM_SLOTS;
	memset(abc->reserved0, 0, sizeof(abc->reserved0));
	for (i = 0; i < abc->nb_slot; ++i)
		abc->slot_info[i] = metadata;
	//fix for Trial Boot write hdcp key issue
	abc->slot_info[0].successful_boot = 1;
	memset(abc->reserved1, 0, sizeof(abc->reserved1));
	abc->crc32_le = ab_control_compute_crc(abc);

	return 0;
}

/**
 * Load the boot_control struct from disk into newly allocated memory.
 *
 * This function allocates and returns an integer number of disk blocks,
 * based on the block size of the passed device to help performing a
 * read-modify-write operation on the boot_control struct.
 * The boot_control struct offset (2 KiB) must be a multiple of the device
 * block size, for simplicity.
 *
 * @param[in] dev_desc Device where to read the boot_control struct from
 * @param[in] part_info Partition in 'dev_desc' where to read from, normally
 *			the "misc" partition should be used
 * @param[out] pointer to pointer to andr_bl_control data
 * @return 0 on success and a negative on error
 */
static int ab_control_create_from_disk(struct blk_desc *dev_desc,
				       const disk_partition_t *part_info,
				       struct andr_bl_control **abc)
{
	ulong abc_offset, abc_blocks;
    if(part_info->blksz<=0x800)
    {
        abc_offset = offsetof(struct andr_bl_msg_ab, slot_suffix);
        if (abc_offset % part_info->blksz) {
            printf("ANDROID: Boot control block not block aligned.\n");
            return -EINVAL;
        }
        abc_offset /= part_info->blksz;

        abc_blocks = DIV_ROUND_UP(sizeof(struct andr_bl_control),
                    part_info->blksz);
        if (abc_offset + abc_blocks > part_info->size) {
            printf("ANDROID: boot control partition too small. Need at");
            printf(" least %lu blocks but have %lu blocks.\n",
                   abc_offset + abc_blocks, part_info->size);
            return -EINVAL;
        }
        *abc = malloc_cache_aligned(abc_blocks * part_info->blksz);
        if (!*abc)
            return -ENOMEM;

        if (blk_dread(dev_desc, part_info->start + abc_offset, abc_blocks,
            *abc) != abc_blocks) {
            printf("ANDROID: Could not read from boot control partition\n");
            free(*abc);
            return -EIO;
        }
    }
    else
    {
        struct andr_bl_msg_ab *abma = NULL;
        abc_blocks=1;
        abma = malloc_cache_aligned(sizeof(struct andr_bl_msg_ab));
        if (!abma)
            return -ENOMEM;

        if (blk_dread(dev_desc, part_info->start , abc_blocks,abma) != abc_blocks)
        {
            printf("ANDROID: Could not read from boot control partition\n");
            free(abma);
            return -EIO;
        }
        *abc = malloc_cache_aligned(sizeof(struct andr_bl_control));
        if (!*abc)
        {
            free(abma);
            return -ENOMEM;
        }
        memcpy(*abc, abma->slot_suffix, sizeof(struct andr_bl_control));
        free(abma);
    }
	log_debug("ANDROID: Loaded ABC, %lu blocks\n", abc_blocks);

	return 0;
}

/**
 * Store the loaded boot_control block.
 *
 * Store back to the same location it was read from with
 * ab_control_create_from_misc().
 *
 * @param[in] dev_desc Device where we should write the boot_control struct
 * @param[in] part_info Partition on the 'dev_desc' where to write
 * @param[in] abc Pointer to the boot control struct and the extra bytes after
 *                it up to the nearest block boundary
 * @return 0 on success and a negative on error
 */
static int ab_control_store(struct blk_desc *dev_desc,
			    const disk_partition_t *part_info,
			    struct andr_bl_control *abc)
{
    ulong abc_offset, abc_blocks;
    if(part_info->blksz<=0x800)
    {
        abc_offset = offsetof(struct andr_bl_msg_ab, slot_suffix) /
                part_info->blksz;
        abc_blocks = DIV_ROUND_UP(sizeof(struct andr_bl_control),
                part_info->blksz);
        if (blk_dwrite(dev_desc, part_info->start + abc_offset, abc_blocks,
              abc) != abc_blocks) {
            printf("ANDROID: Could not write back the misc partition\n");
            return -EIO;
        }
    }
    else
    {
        struct andr_bl_msg_ab *abma = NULL;
        abc_blocks=1;
        abc_offset=0;
        abma = malloc_cache_aligned(sizeof(struct andr_bl_msg_ab));
        if (!abma)
            return -ENOMEM;

        if (blk_dread(dev_desc, part_info->start , abc_blocks,abma) != abc_blocks)
        {
            printf("ANDROID: Could not read from boot control partition\n");
            free(abma);
            return -EIO;
        }
        memcpy(abma->slot_suffix,abc, sizeof(struct andr_bl_control));
        if (blk_dwrite(dev_desc, part_info->start + abc_offset, abc_blocks,
            abma) != abc_blocks) {
            free(abma);
            printf("ANDROID: Could not write back the misc partition\n");
            return -EIO;
        }
        free(abma);
    }

	return 0;
}

#if (CONFIG_AB_SIDELOAD != 1)
/**
 * Compare two slots.
 *
 * The function determines slot which is should we boot from among the two.
 *
 * @param[in] a The first bootable slot metadata
 * @param[in] b The second bootable slot metadata
 * @return Negative if the slot "a" is better, positive of the slot "b" is
 *         better or 0 if they are equally good.
 */
static int ab_compare_slots(const struct andr_slot_metadata *a,
			    const struct andr_slot_metadata *b)
{
	/* Higher priority is better */
	if (a->priority != b->priority)
		return b->priority - a->priority;

	/* Higher successful_boot value is better, in case of same priority */
	if (a->successful_boot != b->successful_boot)
		return b->successful_boot - a->successful_boot;

	/* Higher tries_remaining is better to ensure round-robin */
	if (a->tries_remaining != b->tries_remaining)
		return b->tries_remaining - a->tries_remaining;

	return 0;
}
#endif

#if (CONFIG_AB_SIDELOAD == 1)
int ab_select_slot(struct blk_desc *dev_desc, disk_partition_t *part_info)
{
	struct andr_bl_control *abc = NULL;
	u32 crc32_le;
	int slot, ret;
	bool store_needed = false;
	char slot_suffix[4];

	ret = ab_control_create_from_disk(dev_desc, part_info, &abc);
	if (ret < 0) {
		/*
		 * This condition represents an actual problem with the code or
		 * the board setup, like an invalid partition information.
		 * Signal a repair mode and do not try to boot from either slot.
		 */
		return ret;
	}

	crc32_le = ab_control_compute_crc(abc);
	if (abc->crc32_le != crc32_le) {
		printf("ANDROID: Invalid CRC-32 (expected %.8x, found %.8x), ",
		       crc32_le, abc->crc32_le);
		printf("re-initializing A/B metadata.\n");
		ret = ab_control_default(abc);
		if (ret < 0) {
			free(abc);
			return -ENODATA;
		}
		store_needed = true;
	}

	if (abc->magic != ANDROID_BOOT_CTRL_MAGIC) {
		printf("ANDROID: Unknown A/B metadata: %.8x\n", abc->magic);
		free(abc);
		return -ENODATA;
	}

	if (abc->version > ANDROID_BOOT_CTRL_VERSION) {
		printf("ANDROID: Unsupported A/B metadata version: %.8x\n",
		       abc->version);
		free(abc);
		return -ENODATA;
	}

	/*
	 * At this point a valid boot control metadata is stored in abc,
	 * followed by other reserved data in the same block. We select a with
	 * the higher priority slot that
	 *  - is not marked as corrupted and
	 *  - either has tries_remaining > 0 or successful_boot is true.
	 * If the selected slot has a false successful_boot, we also decrement
	 * the tries_remaining until it eventually becomes unbootable because
	 * tries_remaining reaches 0. This mechanism produces a bootloader
	 * induced rollback, typically right after a failed update.
	 */

	/* Safety check: limit the number of slots. */
	if (abc->nb_slot > ARRAY_SIZE(abc->slot_info)) {
		abc->nb_slot = ARRAY_SIZE(abc->slot_info);
		store_needed = true;
	}

	slot = ab_slot_num();
	if (slot < 0) {
		free(abc);
		return -ENODATA;
	}

	// successful_boot reset retry count
	if (abc->slot_info[slot].successful_boot)
		ab_reset_retry_count();

	if (pm_get_boot_reason() != PM_BR_WATCHDOG) {
		/*default retry count +1 if jump to kernel dm-verity fail  */
		sys_add_retry_count();
		skip_add_retry_count();
	}

	if (abc->slot_info[slot].tries_remaining != ab_get_retry_count())
		store_needed = true;

	abc->slot_info[slot].tries_remaining = ab_get_retry_count();
	if (!abc->slot_info[slot].successful_boot) {
		printf("ANDROID: Attempting slot %c, tries remaining %d\n",
		       ANDROID_BOOT_SLOT_NAME(slot),
		       abc->slot_info[slot].tries_remaining);
	}

	log_debug("ANDROID: bootable slot %d pri: %d, tries: %d, ",
		  slot, abc->slot_info[slot].priority,
		  abc->slot_info[slot].tries_remaining);
	log_debug("corrupt: %d, successful: %d\n",
		  abc->slot_info[slot].verity_corrupted,
		  abc->slot_info[slot].successful_boot);

	/*
	 * Legacy user-space requires this field to be set in the BCB.
	 * Newer releases load this slot suffix from the command line
	 * or the device tree.
	 */
	if (slot == 0) {
		abc->slot_info[slot].priority = AB_PRIORITY_MAX;
		abc->slot_info[slot + 1].priority = AB_PRIORITY_MAX - 1;
	} else if (slot == 1) {
		abc->slot_info[slot].priority = AB_PRIORITY_MAX;
		abc->slot_info[slot - 1].priority = AB_PRIORITY_MAX - 1;
	}
	memset(slot_suffix, 0, sizeof(slot_suffix));
	slot_suffix[0] = ANDROID_BOOT_SLOT_NAME(slot);
	if (memcmp(abc->slot_suffix, slot_suffix, sizeof(slot_suffix))) {
		memcpy(abc->slot_suffix, slot_suffix,
		       sizeof(slot_suffix));
		store_needed = true;
	}
	ab_set_select_successful_boot(abc->slot_info[slot].successful_boot);

	if (store_needed) {
		abc->crc32_le = ab_control_compute_crc(abc);
		ab_control_store(dev_desc, part_info, abc);
	}
	free(abc);

	return slot;
}
#else
int ab_select_slot(struct blk_desc *dev_desc, disk_partition_t *part_info)
{
	struct andr_bl_control *abc = NULL;
	u32 crc32_le;
	int slot, ret;
	unsigned int i;
	bool store_needed = false;
	char slot_suffix[4];

	ret = ab_control_create_from_disk(dev_desc, part_info, &abc);
	if (ret < 0) {
		/*
		 * This condition represents an actual problem with the code or
		 * the board setup, like an invalid partition information.
		 * Signal a repair mode and do not try to boot from either slot.
		 */
		return ret;
	}

	crc32_le = ab_control_compute_crc(abc);
	if (abc->crc32_le != crc32_le) {
		printf("ANDROID: Invalid CRC-32 (expected %.8x, found %.8x), ",
		       crc32_le, abc->crc32_le);
		printf("re-initializing A/B metadata.\n");
		ret = ab_control_default(abc);
		if (ret < 0) {
			free(abc);
			return -ENODATA;
		}
		store_needed = true;
	}

	if (abc->magic != ANDROID_BOOT_CTRL_MAGIC) {
		printf("ANDROID: Unknown A/B metadata: %.8x\n", abc->magic);
		free(abc);
		return -ENODATA;
	}

	if (abc->version > ANDROID_BOOT_CTRL_VERSION) {
		printf("ANDROID: Unsupported A/B metadata version: %.8x\n",
		       abc->version);
		free(abc);
		return -ENODATA;
	}

	/*
	 * At this point a valid boot control metadata is stored in abc,
	 * followed by other reserved data in the same block. We select a with
	 * the higher priority slot that
	 *  - is not marked as corrupted and
	 *  - either has tries_remaining > 0 or successful_boot is true.
	 * If the selected slot has a false successful_boot, we also decrement
	 * the tries_remaining until it eventually becomes unbootable because
	 * tries_remaining reaches 0. This mechanism produces a bootloader
	 * induced rollback, typically right after a failed update.
	 */

	/* Safety check: limit the number of slots. */
	if (abc->nb_slot > ARRAY_SIZE(abc->slot_info)) {
		abc->nb_slot = ARRAY_SIZE(abc->slot_info);
		store_needed = true;
	}

	slot = -1;
	for (i = 0; i < abc->nb_slot; ++i) {
		if (abc->slot_info[i].verity_corrupted ||
		    !abc->slot_info[i].tries_remaining) {
			log_debug("ANDROID: unbootable slot %d tries: %d, ",
				  i, abc->slot_info[i].tries_remaining);
			log_debug("corrupt: %d\n",
				  abc->slot_info[i].verity_corrupted);
			continue;
		}
		log_debug("ANDROID: bootable slot %d pri: %d, tries: %d, ",
			  i, abc->slot_info[i].priority,
			  abc->slot_info[i].tries_remaining);
		log_debug("corrupt: %d, successful: %d\n",
			  abc->slot_info[i].verity_corrupted,
			  abc->slot_info[i].successful_boot);

		if (slot < 0 ||
		    ab_compare_slots(&abc->slot_info[i],
				     &abc->slot_info[slot]) < 0) {
			slot = i;
		}
	}

	if (slot >= 0 && !abc->slot_info[slot].successful_boot) {
		printf("ANDROID: Attempting slot %c, tries remaining %d\n",
		       ANDROID_BOOT_SLOT_NAME(slot),
		       abc->slot_info[slot].tries_remaining);
		abc->slot_info[slot].tries_remaining--;
		store_needed = true;
	}

	if (slot >= 0) {
		/*
		 * Legacy user-space requires this field to be set in the BCB.
		 * Newer releases load this slot suffix from the command line
		 * or the device tree.
		 */
        if(slot==0)
        {
            abc->slot_info[slot].priority=15;
            abc->slot_info[slot+1].priority=14;
        }
        else if(slot==1)
        {
            abc->slot_info[slot].priority=15;
            abc->slot_info[slot-1].priority=14;
        }
		memset(slot_suffix, 0, sizeof(slot_suffix));
		slot_suffix[0] = ANDROID_BOOT_SLOT_NAME(slot);
		if (memcmp(abc->slot_suffix, slot_suffix,
			   sizeof(slot_suffix))) {
			memcpy(abc->slot_suffix, slot_suffix,
			       sizeof(slot_suffix));
			store_needed = true;
		}
	}

	if (store_needed) {
		abc->crc32_le = ab_control_compute_crc(abc);
		ab_control_store(dev_desc, part_info, abc);
	}
	free(abc);

	if (slot < 0)
		return -EINVAL;

	return slot;
}
#endif
