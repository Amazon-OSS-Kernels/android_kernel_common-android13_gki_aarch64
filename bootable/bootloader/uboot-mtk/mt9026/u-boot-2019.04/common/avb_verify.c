/*
 * (C) Copyright 2018, Linaro Limited
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <avb_verify.h>
#include <blk.h>
#include <fastboot.h>
#include <image.h>
#include <malloc.h>
#include <part.h>
#include <tee.h>
#include <utility.h>
#include <tee/optee_ta_avb.h>
#include <debug_impl.h>
#include <boot_impl.h>
#include <partition.h>
#include <environment.h>
#include <idme.h>
//MTK code starts
#if defined(CONFIG_ROLLBACK_INDEX_IN_EFUSE)
#include <program_efuse_rollback_index.h>
#endif

#ifdef AMAZON_AVB_SIGNING
#include <amzn_tv_secure_boot.h>
#include <amzn_security_avb_key.h>
static uint8_t avb_vbmeta_pubkey_dev[] = { AVB_VBMETA_PUBKEY_DEV };
static uint8_t avb_vbmeta_pubkey_prod[] = { AVB_VBMETA_PUBKEY_PROD };

#if !defined(CONFIG_ANDROID_AB)
static uint8_t avb_recovery_pubkey_prod[] = { AVB_RECOVERY_PUBKEY_PROD };
#endif
#else
#define COMMAND_BUF_SIZE           128

#if !defined(UB_A_AVB_KEY_BIN_H) && !defined(GOOGLE_UB_A_AVB_KEY_BIN_H)
//MTK code ends
static const unsigned char avb_root_pub[1032] = {
	0x0, 0x0, 0x10, 0x0, 0x55, 0xd9, 0x4, 0xad, 0xd8, 0x4,
	0xaf, 0xe3, 0xd3, 0x84, 0x6c, 0x7e, 0xd, 0x89, 0x3d, 0xc2,
	0x8c, 0xd3, 0x12, 0x55, 0xe9, 0x62, 0xc9, 0xf1, 0xf, 0x5e,
	0xcc, 0x16, 0x72, 0xab, 0x44, 0x7c, 0x2c, 0x65, 0x4a, 0x94,
	0xb5, 0x16, 0x2b, 0x0, 0xbb, 0x6, 0xef, 0x13, 0x7, 0x53,
	0x4c, 0xf9, 0x64, 0xb9, 0x28, 0x7a, 0x1b, 0x84, 0x98, 0x88,
	0xd8, 0x67, 0xa4, 0x23, 0xf9, 0xa7, 0x4b, 0xdc, 0x4a, 0xf,
	0xf7, 0x3a, 0x18, 0xae, 0x54, 0xa8, 0x15, 0xfe, 0xb0, 0xad,
	0xac, 0x35, 0xda, 0x3b, 0xad, 0x27, 0xbc, 0xaf, 0xe8, 0xd3,
	0x2f, 0x37, 0x34, 0xd6, 0x51, 0x2b, 0x6c, 0x5a, 0x27, 0xd7,
	0x96, 0x6, 0xaf, 0x6b, 0xb8, 0x80, 0xca, 0xfa, 0x30, 0xb4,
	0xb1, 0x85, 0xb3, 0x4d, 0xaa, 0xaa, 0xc3, 0x16, 0x34, 0x1a,
	0xb8, 0xe7, 0xc7, 0xfa, 0xf9, 0x9, 0x77, 0xab, 0x97, 0x93,
	0xeb, 0x44, 0xae, 0xcf, 0x20, 0xbc, 0xf0, 0x80, 0x11, 0xdb,
	0x23, 0xc, 0x47, 0x71, 0xb9, 0x6d, 0xd6, 0x7b, 0x60, 0x47,
	0x87, 0x16, 0x56, 0x93, 0xb7, 0xc2, 0x2a, 0x9a, 0xb0, 0x4c,
	0x1, 0xc, 0x30, 0xd8, 0x93, 0x87, 0xf0, 0xed, 0x6e, 0x8b,
	0xbe, 0x30, 0x5b, 0xf6, 0xa6, 0xaf, 0xdd, 0x80, 0x7c, 0x45,
	0x5e, 0x8f, 0x91, 0x93, 0x5e, 0x44, 0xfe, 0xb8, 0x82, 0x7,
	0xee, 0x79, 0xca, 0xbf, 0x31, 0x73, 0x62, 0x58, 0xe3, 0xcd,
	0xc4, 0xbc, 0xc2, 0x11, 0x1d, 0xa1, 0x4a, 0xbf, 0xfe, 0x27,
	0x7d, 0xa1, 0xf6, 0x35, 0xa3, 0x5e, 0xca, 0xdc, 0x57, 0x2f,
	0x3e, 0xf0, 0xc9, 0x5d, 0x86, 0x6a, 0xf8, 0xaf, 0x66, 0xa7,
	0xed, 0xcd, 0xb8, 0xed, 0xa1, 0x5f, 0xba, 0x9b, 0x85, 0x1a,
	0xd5, 0x9, 0xae, 0x94, 0x4e, 0x3b, 0xcf, 0xcb, 0x5c, 0xc9,
	0x79, 0x80, 0xf7, 0xcc, 0xa6, 0x4a, 0xa8, 0x6a, 0xd8, 0xd3,
	0x31, 0x11, 0xf9, 0xf6, 0x2, 0x63, 0x2a, 0x1a, 0x2d, 0xd1,
	0x1a, 0x66, 0x1b, 0x16, 0x41, 0xbd, 0xbd, 0xf7, 0x4d, 0xc0,
	0x4a, 0xe5, 0x27, 0x49, 0x5f, 0x7f, 0x58, 0xe3, 0x27, 0x2d,
	0xe5, 0xc9, 0x66, 0xe, 0x52, 0x38, 0x16, 0x38, 0xfb, 0x16,
	0xeb, 0x53, 0x3f, 0xe6, 0xfd, 0xe9, 0xa2, 0x5e, 0x25, 0x59,
	0xd8, 0x79, 0x45, 0xff, 0x3, 0x4c, 0x26, 0xa2, 0x0, 0x5a,
	0x8e, 0xc2, 0x51, 0xa1, 0x15, 0xf9, 0x7b, 0xf4, 0x5c, 0x81,
	0x9b, 0x18, 0x47, 0x35, 0xd8, 0x2d, 0x5, 0xe9, 0xad, 0xf,
	0x35, 0x74, 0x15, 0xa3, 0x8e, 0x8b, 0xcc, 0x27, 0xda, 0x7c,
	0x5d, 0xe4, 0xfa, 0x4, 0xd3, 0x5, 0xb, 0xba, 0x3a, 0xb2,
	0x49, 0x45, 0x2f, 0x47, 0xc7, 0xd, 0x41, 0x3f, 0x97, 0x80,
	0x4d, 0x3f, 0xc1, 0xb5, 0xbb, 0x70, 0x5f, 0xa7, 0x37, 0xaf,
	0x48, 0x22, 0x12, 0x45, 0x2e, 0xf5, 0xf, 0x87, 0x92, 0xe2,
	0x84, 0x1, 0xf9, 0x12, 0xf, 0x14, 0x15, 0x24, 0xce, 0x89,
	0x99, 0xee, 0xb9, 0xc4, 0x17, 0x70, 0x70, 0x15, 0xea, 0xbe,
	0xc6, 0x6c, 0x1f, 0x62, 0xb3, 0xf4, 0x2d, 0x16, 0x87, 0xfb,
	0x56, 0x1e, 0x45, 0xab, 0xae, 0x32, 0xe4, 0x5e, 0x91, 0xed,
	0x53, 0x66, 0x5e, 0xbd, 0xed, 0xad, 0xe6, 0x12, 0x39, 0xd,
	0x83, 0xc9, 0xe8, 0x6b, 0x6c, 0x2d, 0xa5, 0xee, 0xc4, 0x5a,
	0x66, 0xae, 0x8c, 0x97, 0xd7, 0xd, 0x6c, 0x49, 0xc7, 0xf5,
	0xc4, 0x92, 0x31, 0x8b, 0x9, 0xee, 0x33, 0xda, 0xa9, 0x37,
	0xb6, 0x49, 0x18, 0xf8, 0xe, 0x60, 0x45, 0xc8, 0x33, 0x91,
	0xef, 0x20, 0x57, 0x10, 0xbe, 0x78, 0x2d, 0x83, 0x26, 0xd6,
	0xca, 0x61, 0xf9, 0x2f, 0xe0, 0xbf, 0x5, 0x30, 0x52, 0x5a,
	0x12, 0x1c, 0x0, 0xa7, 0x5d, 0xcc, 0x7c, 0x2e, 0xc5, 0x95,
	0x8b, 0xa3, 0x3b, 0xf0, 0x43, 0x2e, 0x5e, 0xdd, 0x0, 0xdb,
	0xd, 0xb3, 0x37, 0x99, 0xa9, 0xcd, 0x9c, 0xb7, 0x43, 0xf7,
	0x35, 0x44, 0x21, 0xc2, 0x82, 0x71, 0xab, 0x8d, 0xaa, 0xb4,
	0x41, 0x11, 0xec, 0x1e, 0x8d, 0xfc, 0x14, 0x82, 0x92, 0x4e,
	0x83, 0x6a, 0xa, 0x6b, 0x35, 0x5e, 0x5d, 0xe9, 0x5c, 0xcc,
	0x8c, 0xde, 0x39, 0xd1, 0x4a, 0x5b, 0x5f, 0x63, 0xa9, 0x64,
	0xe0, 0xa, 0xcb, 0xb, 0xb8, 0x5a, 0x7c, 0xc3, 0xb, 0xe6,
	0xbe, 0xfe, 0x8b, 0xf, 0x7d, 0x34, 0x8e, 0x2, 0x66, 0x74,
	0x1, 0x6c, 0xca, 0x76, 0xac, 0x7c, 0x67, 0x8, 0x2f, 0x3f,
	0x1a, 0xa6, 0x2c, 0x60, 0xb3, 0xff, 0xda, 0x8d, 0xb8, 0x12,
	0xc, 0x0, 0x7f, 0xcc, 0x50, 0xa1, 0x5c, 0x64, 0xa1, 0xe2,
	0x5f, 0x32, 0x65, 0xc9, 0x9c, 0xbe, 0xd6, 0xa, 0x13, 0x87,
	0x3c, 0x2a, 0x45, 0x47, 0xc, 0xca, 0x42, 0x82, 0xfa, 0x89,
	0x65, 0xe7, 0x89, 0xb4, 0x8f, 0xf7, 0x1e, 0xe6, 0x23, 0xa5,
	0xd0, 0x59, 0x37, 0x79, 0x92, 0xd7, 0xce, 0x3d, 0xfd, 0xe3,
	0xa1, 0xb, 0xcf, 0x6c, 0x85, 0xa0, 0x65, 0xf3, 0x5c, 0xc6,
	0x4a, 0x63, 0x5f, 0x6e, 0x3a, 0x3a, 0x2a, 0x8b, 0x6a, 0xb6,
	0x2f, 0xbb, 0xf8, 0xb2, 0x4b, 0x62, 0xbc, 0x1a, 0x91, 0x25,
	0x66, 0xe3, 0x69, 0xca, 0x60, 0x49, 0xb, 0xf6, 0x8a, 0xbe,
	0x3e, 0x76, 0x53, 0xc2, 0x7a, 0xa8, 0x4, 0x17, 0x75, 0xf1,
	0xf3, 0x3, 0x62, 0x1b, 0x85, 0xb2, 0xb0, 0xef, 0x80, 0x15,
	0xb6, 0xd4, 0x4e, 0xdf, 0x71, 0xac, 0xdb, 0x2a, 0x4, 0xd4,
	0xb4, 0x21, 0xba, 0x65, 0x56, 0x57, 0xe8, 0xfa, 0x84, 0xa2,
	0x7d, 0x13, 0xe, 0xaf, 0xd7, 0x9a, 0x58, 0x2a, 0xa3, 0x81,
	0x84, 0x8d, 0x9, 0xa0, 0x6a, 0xc1, 0xbb, 0xd9, 0xf5, 0x86,
	0xac, 0xbd, 0x75, 0x61, 0x9, 0xe6, 0x8c, 0x3d, 0x77, 0xb2,
	0xed, 0x30, 0x20, 0xe4, 0x0, 0x1d, 0x97, 0xe8, 0xbf, 0xc7,
	0x0, 0x1b, 0x21, 0xb1, 0x16, 0xe7, 0x41, 0x67, 0x2e, 0xec,
	0x38, 0xbc, 0xe5, 0x1b, 0xb4, 0x6, 0x23, 0x31, 0x71, 0x1c,
	0x49, 0xcd, 0x76, 0x4a, 0x76, 0x36, 0x8d, 0xa3, 0x89, 0x8b,
	0x4a, 0x7a, 0xf4, 0x87, 0xc8, 0x15, 0xf, 0x37, 0x39, 0xf6,
	0x6d, 0x80, 0x19, 0xef, 0x5c, 0xa8, 0x66, 0xce, 0x1b, 0x16,
	0x79, 0x21, 0xdf, 0xd7, 0x31, 0x30, 0xc4, 0x21, 0xdd, 0x34,
	0x5b, 0xd2, 0x1a, 0x2b, 0x3e, 0x5d, 0xf7, 0xea, 0xca, 0x5,
	0x8e, 0xb7, 0xcb, 0x49, 0x2e, 0xa0, 0xe3, 0xf4, 0xa7, 0x48,
	0x19, 0x10, 0x9c, 0x4, 0xa7, 0xf4, 0x28, 0x74, 0xc8, 0x6f,
	0x63, 0x20, 0x2b, 0x46, 0x24, 0x26, 0x19, 0x1d, 0xd1, 0x2c,
	0x31, 0x6d, 0x5a, 0x29, 0xa2, 0x6, 0xa6, 0xb2, 0x41, 0xcc,
	0xa, 0x27, 0x96, 0x9, 0x96, 0xac, 0x47, 0x65, 0x78, 0x68,
	0x51, 0x98, 0xd6, 0xd8, 0xa6, 0x2d, 0xa0, 0xcf, 0xec, 0xe2,
	0x74, 0xf2, 0x82, 0xe3, 0x97, 0xd9, 0x7e, 0xd4, 0xf8, 0xb,
	0x70, 0x43, 0x3d, 0xb1, 0x7b, 0x97, 0x80, 0xd6, 0xcb, 0xd7,
	0x19, 0xbc, 0x63, 0xb, 0xfd, 0x4d, 0x88, 0xfe, 0x67, 0xac,
	0xb8, 0xcc, 0x50, 0xb7, 0x68, 0xb3, 0x5b, 0xd6, 0x1e, 0x25,
	0xfc, 0x5f, 0x3c, 0x8d, 0xb1, 0x33, 0x7c, 0xb3, 0x49, 0x1,
	0x3f, 0x71, 0x55, 0xe, 0x51, 0xba, 0x61, 0x26, 0xfa, 0xea,
	0xe5, 0xb5, 0xe8, 0xaa, 0xcf, 0xcd, 0x96, 0x9f, 0xd6, 0xc1,
	0x5f, 0x53, 0x91, 0xad, 0x5, 0xde, 0x20, 0xe7, 0x51, 0xda,
	0x5b, 0x95, 0x67, 0xed, 0xf4, 0xee, 0x42, 0x65, 0x70, 0x13,
	0xb, 0x70, 0x14, 0x1c, 0xc9, 0xe0, 0x19, 0xca, 0x5f, 0xf5,
	0x1d, 0x70, 0x4b, 0x6c, 0x6, 0x74, 0xec, 0xb5, 0x2e, 0x77,
	0xe1, 0x74, 0xa1, 0xa3, 0x99, 0xa0, 0x85, 0x9e, 0xf1, 0xac,
	0xd8, 0x7e,
};
#else
#include <key.h>

	uint8_t avb_root_pub[] = {CONFIG_ANDROID_SIGNATURE_PUBKEY};
#endif

#endif /* AMAZON_AVB_SIGNING */

/**
 * ============================================================================
 * Boot states support (GREEN, YELLOW, ORANGE, RED) and dm_verity
 * ============================================================================
 */
char *avb_set_state(AvbOps *ops, enum avb_boot_state boot_state)
{
	struct AvbOpsData *data;
	char *cmdline = NULL;

	if (!ops)
		return NULL;

	data = (struct AvbOpsData *)ops->user_data;
	if (!data)
		return NULL;

	data->boot_state = boot_state;
	switch (boot_state) {
	case AVB_GREEN:
		cmdline = "androidboot.verifiedbootstate=green";
		break;
	case AVB_YELLOW:
		cmdline = "androidboot.verifiedbootstate=yellow";
		break;
	case AVB_ORANGE:
		cmdline = "androidboot.verifiedbootstate=orange";
	case AVB_RED:
		break;
	}

	return cmdline;
}

char *append_cmd_line(char *cmdline_orig, char *cmdline_new)
{
	char *cmd_line;

	if (!cmdline_new)
		return cmdline_orig;

	if (cmdline_orig)
		cmd_line = cmdline_orig;
	else
		cmd_line = " ";

	cmd_line = avb_strdupv(cmd_line, " ", cmdline_new, NULL);

	return cmd_line;
}

static int avb_find_dm_args(char **args, char *str)
{
	int i;

	if (!str)
		return -1;

	for (i = 0; i < AVB_MAX_ARGS && args[i]; ++i) {
		if (strstr(args[i], str))
			return i;
	}

	return -1;
}

static char *avb_set_enforce_option(const char *cmdline, const char *option)
{
	char *cmdarg[AVB_MAX_ARGS];
	char *newargs = NULL;
	int i = 0;
	int total_args;

	memset(cmdarg, 0, sizeof(cmdarg));
	cmdarg[i++] = strtok((char *)cmdline, " ");

	do {
		cmdarg[i] = strtok(NULL, " ");
		if (!cmdarg[i])
			break;

		if (++i >= AVB_MAX_ARGS) {
			printf("%s: Can't handle more then %d args\n",
			       __func__, i);
			return NULL;
		}
	} while (true);

	total_args = i;
	i = avb_find_dm_args(&cmdarg[0], VERITY_TABLE_OPT_LOGGING);
	if (i >= 0) {
		cmdarg[i] = (char *)option;
	} else {
		i = avb_find_dm_args(&cmdarg[0], VERITY_TABLE_OPT_RESTART);
		if (i < 0) {
			printf("%s: No verity options found\n", __func__);
			return NULL;
		}

		cmdarg[i] = (char *)option;
	}

	for (i = 0; i <= total_args; i++)
		newargs = append_cmd_line(newargs, cmdarg[i]);

	return newargs;
}

char *avb_set_ignore_corruption(const char *cmdline)
{
	char *newargs = NULL;

	newargs = avb_set_enforce_option(cmdline, VERITY_TABLE_OPT_LOGGING);
	if (newargs)
		newargs = append_cmd_line(newargs,
					  "androidboot.veritymode=eio");

	return newargs;
}

char *avb_set_enforce_verity(const char *cmdline)
{
	char *newargs;

	newargs = avb_set_enforce_option(cmdline, VERITY_TABLE_OPT_RESTART);
	if (newargs)
		newargs = append_cmd_line(newargs,
					  "androidboot.veritymode=enforcing");
	return newargs;
}

/**
 * ============================================================================
 * IO(mmc) auxiliary functions
 * ============================================================================
 */
#ifndef CONFIG_LEGACY_AVB_PARTITION
static unsigned long mmc_read_and_flush(struct mmc_part *part,
					lbaint_t start,
					lbaint_t sectors,
					void *buffer)
{
	unsigned long blks;
	void *tmp_buf;
	size_t buf_size;
	bool unaligned = is_buf_unaligned(buffer);

	if (start < part->info.start) {
		printf("%s: partition start out of bounds\n", __func__);
		return 0;
	}
	if ((start + sectors) > (part->info.start + part->info.size)) {
		sectors = part->info.start + part->info.size - start;
		printf("%s: read sector aligned to partition bounds (%ld)\n",
		       __func__, sectors);
	}

	/*
	 * Reading fails on unaligned buffers, so we have to
	 * use aligned temporary buffer and then copy to destination
	 */

	if (unaligned) {
		printf("Handling unaligned read buffer..\n");
		//tmp_buf = get_sector_buf();
		tmp_buf=avb_malloc(CONFIG_FASTBOOT_BUF_SIZE);
		if(NULL == tmp_buf)
			return AVB_IO_RESULT_ERROR_OOM;
		buf_size = CONFIG_FASTBOOT_BUF_SIZE;
		if (sectors > buf_size / part->info.blksz)
			sectors = buf_size / part->info.blksz;
	} else {
		tmp_buf = buffer;
	}

	blks = blk_dread(part->mmc_blk,
			 start, sectors, tmp_buf);
	/* flush cache after read */
	flush_cache((ulong)tmp_buf, sectors * part->info.blksz);

	if (unaligned)
	{
		memcpy(buffer, tmp_buf, sectors * part->info.blksz);
		if(tmp_buf) {avb_free(tmp_buf);}
	}

	return blks;
}

static unsigned long mmc_write(struct mmc_part *part, lbaint_t start,
			       lbaint_t sectors, void *buffer)
{
	void *tmp_buf;
	size_t buf_size;
	bool unaligned = is_buf_unaligned(buffer);

	if (start < part->info.start) {
		printf("%s: partition start out of bounds\n", __func__);
		return 0;
	}
	if ((start + sectors) > (part->info.start + part->info.size)) {
		sectors = part->info.start + part->info.size - start;
		printf("%s: sector aligned to partition bounds (%ld)\n",
		       __func__, sectors);
	}
	if (unaligned) {
		tmp_buf = get_sector_buf();
		buf_size = get_sector_buf_size();
		printf("Handling unaligned wrire buffer..\n");
		if (sectors > buf_size / part->info.blksz)
			sectors = buf_size / part->info.blksz;

		memcpy(tmp_buf, buffer, sectors * part->info.blksz);
	} else {
		tmp_buf = buffer;
	}

	return blk_dwrite(part->mmc_blk,
			  start, sectors, tmp_buf);
}

static struct mmc_part *get_partition(AvbOps *ops, const char *partition)
{
	int ret;
	u8 dev_num;
	int part_num = 0;
	struct mmc_part *part;
	struct blk_desc *mmc_blk;

	part = malloc(sizeof(struct mmc_part));
	if (!part)
		return NULL;

	dev_num = get_avb_boot_device(ops);
	part->mmc = find_mmc_device(dev_num);
	if (!part->mmc) {
		printf("No MMC device at slot %x\n", dev_num);
		goto err;
	}

	if (mmc_init(part->mmc)) {
		printf("MMC initialization failed\n");
		goto err;
	}

	ret = mmc_switch_part(part->mmc, part_num);
	if (ret)
		goto err;

	mmc_blk = mmc_get_blk_desc(part->mmc);
	if (!mmc_blk) {
		printf("Error - failed to obtain block descriptor\n");
		goto err;
	}

	ret = part_get_info_by_name(mmc_blk, partition, &part->info);
	if (!ret) {
		printf("Can't find partition '%s'\n", partition);
		goto err;
	}

	part->dev_num = dev_num;
	part->mmc_blk = mmc_blk;

	return part;
err:
	free(part);
	return NULL;
}

static AvbIOResult mmc_byte_io(AvbOps *ops,
			       const char *partition,
			       s64 offset,
			       size_t num_bytes,
			       void *buffer,
			       size_t *out_num_read,
			       enum mmc_io_type io_type)
{
	ulong ret;
	struct mmc_part *part;
	u64 start_offset, start_sector, sectors, residue;
	u8 *tmp_buf;
	size_t io_cnt = 0;

	if (!partition || !buffer || io_type > IO_WRITE)
		return AVB_IO_RESULT_ERROR_IO;

	part = get_partition(ops, partition);
	if (!part)
		return AVB_IO_RESULT_ERROR_NO_SUCH_PARTITION;

	if (!part->info.blksz)
		return AVB_IO_RESULT_ERROR_IO;

	start_offset = calc_offset(part, offset);
	while (num_bytes) {
		start_sector = start_offset / part->info.blksz;
		sectors = num_bytes / part->info.blksz;
		/* handle non block-aligned reads */
		if (start_offset % part->info.blksz ||
		    num_bytes < part->info.blksz) {
			//tmp_buf = get_sector_buf();
			tmp_buf=avb_malloc(CONFIG_FASTBOOT_BUF_SIZE);
			if(NULL == tmp_buf)
				return AVB_IO_RESULT_ERROR_OOM;
			if (start_offset % part->info.blksz) {
				residue = part->info.blksz -
					(start_offset % part->info.blksz);
				if (residue > num_bytes)
					residue = num_bytes;
			} else {
				residue = num_bytes;
			}

			if (io_type == IO_READ) {
				ret = mmc_read_and_flush(part,
							 part->info.start +
							 start_sector,
							 1, tmp_buf);

				if (ret != 1) {
					printf("%s: read error (%ld, %lld)\n",
					       __func__, ret, start_sector);
					if(tmp_buf) {avb_free(tmp_buf);}
					return AVB_IO_RESULT_ERROR_IO;
				}
				/*
				 * if this is not aligned at sector start,
				 * we have to adjust the tmp buffer
				 */
				tmp_buf += (start_offset % part->info.blksz);
				memcpy(buffer, (void *)tmp_buf, residue);
			} else {
				ret = mmc_read_and_flush(part,
							 part->info.start +
							 start_sector,
							 1, tmp_buf);

				if (ret != 1) {
					printf("%s: read error (%ld, %lld)\n",
					       __func__, ret, start_sector);
					if(tmp_buf) {avb_free(tmp_buf);}
					return AVB_IO_RESULT_ERROR_IO;
				}
				memcpy((void *)tmp_buf +
					start_offset % part->info.blksz,
					buffer, residue);

				ret = mmc_write(part, part->info.start +
						start_sector, 1, tmp_buf);
				if (ret != 1) {
					printf("%s: write error (%ld, %lld)\n",
					       __func__, ret, start_sector);
					if(tmp_buf) {avb_free(tmp_buf);}
					return AVB_IO_RESULT_ERROR_IO;
				}
			}

			io_cnt += residue;
			buffer += residue;
			start_offset += residue;
			num_bytes -= residue;
			if(tmp_buf) {avb_free(tmp_buf);}
			continue;
		}

		if (sectors) {
			if (io_type == IO_READ) {
				ret = mmc_read_and_flush(part,
							 part->info.start +
							 start_sector,
							 sectors, buffer);
			} else {
				ret = mmc_write(part,
						part->info.start +
						start_sector,
						sectors, buffer);
			}

			if (!ret) {
				printf("%s: sector read error\n", __func__);
				return AVB_IO_RESULT_ERROR_IO;
			}

			io_cnt += ret * part->info.blksz;
			buffer += ret * part->info.blksz;
			start_offset += ret * part->info.blksz;
			num_bytes -= ret * part->info.blksz;
		}
	}

	/* Set counter for read operation */
	if (io_type == IO_READ && out_num_read)
		*out_num_read = io_cnt;

	return AVB_IO_RESULT_OK;
}

#else
/**
 * ============================================================================
 * IO(partition) auxiliary functions
 * ============================================================================
 */
static AvbIOResult partition_byte_io(AvbOps *ops,
			       const char *partition,
			       s64 offset,
			       size_t num_bytes,
			       void *buffer,
			       size_t *out_num_read,
			       enum mmc_io_type io_type)
{
	int ret = 0;
	char str_buffer[COMMAND_BUF_SIZE];
	char str_offset[COMMAND_BUF_SIZE];
	char str_num_bytes[COMMAND_BUF_SIZE];
	u64 start = 0, end = 0;

	start = get_timer(0);
	if (!partition || !buffer || io_type > IO_WRITE)
		return AVB_IO_RESULT_ERROR_IO;

	memset(str_buffer, 0, COMMAND_BUF_SIZE);
	memset(str_offset, 0, COMMAND_BUF_SIZE);
	memset(str_num_bytes, 0, COMMAND_BUF_SIZE);

	if (offset < 0) {
		disk_partition_t partition_info;
		if(get_boot_partition_info(partition, &partition_info, NULL) != 0)
			return AVB_IO_RESULT_ERROR_NO_SUCH_PARTITION;
		offset = partition_info.size * partition_info.blksz + offset;
	}

	ret = snprintf(str_buffer, sizeof(str_buffer)-1, "%p", buffer);
	if (ret < 0) {
		UBOOT_ERROR("fail to write formatted data to str_buffer\n");
		return AVB_IO_RESULT_ERROR_IO;
	}
	ret = snprintf(str_offset, sizeof(str_offset)-1, "%p", (void*)offset);
	if (ret < 0) {
		UBOOT_ERROR("fail to write formatted data to str_offset\n");
		return AVB_IO_RESULT_ERROR_IO;
        }

	ret = snprintf(str_num_bytes, sizeof(str_num_bytes)-1, "0x%zx", num_bytes);
	if (ret < 0) {
		UBOOT_ERROR("fail to write formatted data to str_num_bytes\n");
		return AVB_IO_RESULT_ERROR_IO;
	}

	if (io_type == IO_READ) {
		ret = do_boot_partition_read(partition, str_buffer, str_num_bytes, str_offset);
		end = get_timer(0);
		UBOOT_BOOTTIME("[Read %s partition][start:%llu][end:%llu][total time:%llu]\n", partition, start, end, end - start);
		if(ret != 0) {
			UBOOT_ERROR("fail to read partition !! \n");
			return AVB_IO_RESULT_ERROR_RANGE_OUTSIDE_PARTITION;
		}
	}
	else {
		ret = do_boot_partition_write(partition, str_buffer, str_num_bytes, str_offset);
		end = get_timer(0);
		UBOOT_BOOTTIME("[Write %s partition][start:%llu][end:%llu][total time:%llu]\n", partition, start, end, end - start);
		if(ret != 0) {
			UBOOT_ERROR("fail to write partition !! \n");
			return AVB_IO_RESULT_ERROR_RANGE_OUTSIDE_PARTITION;
		}
	}

	/* Set counter for read operation */
	if (io_type == IO_READ && out_num_read) {
		*out_num_read = num_bytes;
	}

	return AVB_IO_RESULT_OK;
}
#endif

/**
 * ============================================================================
 * AVB 2.0 operations
 * ============================================================================
 */

/**
 * read_from_partition() - reads @num_bytes from  @offset from partition
 * identified by a string name
 *
 * @ops: contains AVB ops handlers
 * @partition_name: partition name, NUL-terminated UTF-8 string
 * @offset: offset from the beginning of partition
 * @num_bytes: amount of bytes to read
 * @buffer: destination buffer to store data
 * @out_num_read:
 *
 * @return:
 *      AVB_IO_RESULT_OK, if partition was found and read operation succeed
 *      AVB_IO_RESULT_ERROR_IO, if i/o error occurred from the underlying i/o
 *            subsystem
 *      AVB_IO_RESULT_ERROR_NO_SUCH_PARTITION, if there is no partition with
 *      the given name
 */
static AvbIOResult read_from_partition(AvbOps *ops,
				       const char *partition_name,
				       s64 offset_from_partition,
				       size_t num_bytes,
				       void *buffer,
				       size_t *out_num_read)
{

#ifndef CONFIG_LEGACY_AVB_PARTITION
	return mmc_byte_io(ops, partition_name, offset_from_partition,
			   num_bytes, buffer, out_num_read, IO_READ);
#else
	return partition_byte_io(ops, partition_name, offset_from_partition,
			   num_bytes, buffer, out_num_read, IO_READ);
#endif

}

/**
 * write_to_partition() - writes N bytes to a partition identified by a string
 * name
 *
 * @ops: AvbOps, contains AVB ops handlers
 * @partition_name: partition name
 * @offset_from_partition: offset from the beginning of partition
 * @num_bytes: amount of bytes to write
 * @buf: data to write
 * @out_num_read:
 *
 * @return:
 *      AVB_IO_RESULT_OK, if partition was found and read operation succeed
 *      AVB_IO_RESULT_ERROR_IO, if input/output error occurred
 *      AVB_IO_RESULT_ERROR_NO_SUCH_PARTITION, if partition, specified in
 *            @partition_name was not found
 */
static AvbIOResult write_to_partition(AvbOps *ops,
				      const char *partition_name,
				      s64 offset_from_partition,
				      size_t num_bytes,
				      const void *buffer)
{
#ifndef CONFIG_LEGACY_AVB_PARTITION
	return mmc_byte_io(ops, partition_name, offset_from_partition,
			   num_bytes, (void *)buffer, NULL, IO_WRITE);
#else
	return partition_byte_io(ops, partition_name, offset_from_partition,
			   num_bytes, (void *)buffer, NULL, IO_WRITE);
#endif

}

/**
 * validate_vmbeta_public_key() - checks if the given public key used to sign
 * the vbmeta partition is trusted
 *
 * @ops: AvbOps, contains AVB ops handlers
 * @public_key_data: public key for verifying vbmeta partition signature
 * @public_key_length: length of public key
 * @public_key_metadata:
 * @public_key_metadata_length:
 * @out_key_is_trusted:
 *
 * @return:
 *      AVB_IO_RESULT_OK, if partition was found and read operation succeed
 */
static AvbIOResult validate_vbmeta_public_key(AvbOps *ops,
					      const u8 *public_key_data,
					      size_t public_key_length,
					      const u8
					      *public_key_metadata,
					      size_t
					      public_key_metadata_length,
					      bool *out_key_is_trusted)
{
#ifdef AMAZON_AVB_SIGNING
	AvbRSAPublicKeyHeader *key_hdr;
	uint8_t *pubk;
	uint32_t pubk_sz = 0;

	if (out_key_is_trusted == NULL || public_key_data == NULL)
		return AVB_IO_RESULT_ERROR_IO;

	key_hdr = (AvbRSAPublicKeyHeader *)public_key_data;
	pubk = (uint8_t *)(public_key_data + sizeof(AvbRSAPublicKeyHeader));
	pubk_sz = avb_htobe32(key_hdr->key_num_bits) / 8;

	*out_key_is_trusted = false;
	if (pubk_sz != AVB_VBMETA_PUBKEY_SIZE) {
		printf("%s public key size(%d) is invalid\n", __func__, pubk_sz);
		return AVB_IO_RESULT_ERROR_IO;
	}

	if (memcmp((void *)avb_vbmeta_pubkey_prod, (void *)pubk, pubk_sz) == 0) {
		*out_key_is_trusted = true;
		printf("AVB prod key is found.\n");
	} else if (memcmp((void *)avb_vbmeta_pubkey_dev, (void *)pubk, pubk_sz) == 0) {
		printf("AVB dev key is found.\n");
		if (is_lockdown())
			printf("AVB dev key is not allowed on locked production device!\n");
		else
			*out_key_is_trusted = true;
	}
#else
	if (!public_key_length || !public_key_data || !out_key_is_trusted)
		return AVB_IO_RESULT_ERROR_IO;

	*out_key_is_trusted = false;
	if (public_key_length != sizeof(avb_root_pub))
		return AVB_IO_RESULT_ERROR_IO;

	if (memcmp(avb_root_pub, public_key_data, public_key_length) == 0)
		*out_key_is_trusted = true;
#endif
	return AVB_IO_RESULT_OK;
}

#ifdef CONFIG_OPTEE_TA_AVB
static int get_open_session(struct AvbOpsData *ops_data)
{
	struct udevice *tee = NULL;

	while (!ops_data->tee) {
		const struct tee_optee_ta_uuid uuid = TA_AVB_UUID;
		struct tee_open_session_arg arg;
		int rc;

		tee = tee_find_device(tee, NULL, NULL, NULL);
		if (!tee)
			return -ENODEV;

		memset(&arg, 0, sizeof(arg));
		tee_optee_ta_uuid_to_octets(arg.uuid, &uuid);
		rc = tee_open_session(tee, &arg, 0, NULL);
		if (!rc) {
			ops_data->tee = tee;
			ops_data->session = arg.session;
		}
	}

	return 0;
}

//MTK code starts
typedef enum
{
	AVB_RPMB_NOT_CHECKED=0x0,
	AVB_RPMB_OK=0x1,
	AVB_RPMB_RPMBKEYNOTEXIST=0x2,
}AVB_RPMBKEY_STATUS_ENUM;

static AVB_RPMBKEY_STATUS_ENUM check_is_RPMBKey_exist(void)
{
	static AVB_RPMBKEY_STATUS_ENUM RPMBKey_status = AVB_RPMB_NOT_CHECKED;
	if( RPMBKey_status == AVB_RPMB_NOT_CHECKED) {
		RPMBKey_status = (is_RPMBKey_exist() ? AVB_RPMB_OK : AVB_RPMB_RPMBKEYNOTEXIST);
	}
	return RPMBKey_status;
}
//MTK code ends

static AvbIOResult invoke_func(struct AvbOpsData *ops_data, u32 func,
			       ulong num_param, struct tee_param *param)
{
	struct tee_invoke_arg arg;
	//MTK code starts
	//Check RPMBKey status
	if( check_is_RPMBKey_exist() == AVB_RPMB_RPMBKEYNOTEXIST) {
		printf("%s AVB_RPMB_RPMBKEYNOTEXIST\n", __func__);
		return AVB_IO_RESULT_ERROR_RPMBKEYNOTEXISTS;
	}
	//MTK code ends

	if (get_open_session(ops_data))
		return AVB_IO_RESULT_ERROR_IO;

	memset(&arg, 0, sizeof(arg));
	arg.func = func;
	arg.session = ops_data->session;

	if (tee_invoke_func(ops_data->tee, &arg, num_param, param))
		return AVB_IO_RESULT_ERROR_IO;
	//MTK code starts
	//Printf result from TEEOS
	if(arg.ret != TEE_SUCCESS) {
		printf("tee_invoke result code=0x%x\n", arg.ret);
	}
	//MTK code ends
	switch (arg.ret) {
	case TEE_SUCCESS:
		return AVB_IO_RESULT_OK;
	case TEE_ERROR_OUT_OF_MEMORY:
		return AVB_IO_RESULT_ERROR_OOM;
	case TEE_ERROR_ITEM_NOT_FOUND:
		return TEE_ERROR_ITEM_NOT_FOUND;
	case TEE_ERROR_TARGET_DEAD:
		/*
		 * The TA has paniced, close the session to reload the TA
		 * for the next request.
		 */
		tee_close_session(ops_data->tee, ops_data->session);
		ops_data->tee = NULL;
		return AVB_IO_RESULT_ERROR_IO;
	default:
		return AVB_IO_RESULT_ERROR_IO;
	}
}
#endif

/**
 * read_rollback_index() - gets the rollback index corresponding to the
 * location of given by @out_rollback_index.
 *
 * @ops: contains AvbOps handlers
 * @rollback_index_slot:
 * @out_rollback_index: used to write a retrieved rollback index.
 *
 * @return
 *       AVB_IO_RESULT_OK, if the roolback index was retrieved
 */
static AvbIOResult read_rollback_index(AvbOps *ops,
				       size_t rollback_index_slot,
				       u64 *out_rollback_index)
{
#ifndef CONFIG_OPTEE_TA_AVB
	/* For now we always return 0 as the stored rollback index. */
	printf("%s not supported yet\n", __func__);

	if (out_rollback_index)
		*out_rollback_index = 0;

#if defined(CONFIG_HAPS)
	return AVB_IO_RESULT_OK;
#else
	return AVB_IO_RESULT_ERROR_IO;
#endif
#else
	//MTK code starts
#if defined(CONFIG_ROLLBACK_INDEX_IN_EFUSE)
	{
		//In case that rollback enabling bit is unset
		unsigned int enabling_bit = 1; //rollback feature is enabled by default

		if (get_efuse_rollback_enabling_bit(&enabling_bit) == AVB_IO_RESULT_OK) {
			if (!enabling_bit) {
				*out_rollback_index = 0;
				return AVB_IO_RESULT_OK;
			}
		} else {
			return AVB_IO_RESULT_ERROR_IO;
		}
		if (is_slot_in_efuse(rollback_index_slot))
			return read_rollback_index_by_efuse(rollback_index_slot, out_rollback_index);
	}
#endif
	//MTK code ends
	AvbIOResult rc;
	struct tee_param param[2];

	if (rollback_index_slot >= TA_AVB_MAX_ROLLBACK_LOCATIONS)
		return AVB_IO_RESULT_ERROR_NO_SUCH_VALUE;

	memset(param, 0, sizeof(param));
	param[0].attr = TEE_PARAM_ATTR_TYPE_VALUE_INPUT;
	param[0].u.value.a = rollback_index_slot;
	param[1].attr = TEE_PARAM_ATTR_TYPE_VALUE_OUTPUT;

	rc = invoke_func(ops->user_data, TA_AVB_CMD_READ_ROLLBACK_INDEX,
			 ARRAY_SIZE(param), param);
	//MTK code starts
	//In case of AVB_IO_RESULT_ERROR_RPMBKEYNOTEXISTS
	if( rc == AVB_IO_RESULT_ERROR_RPMBKEYNOTEXISTS) {
		*out_rollback_index = 0;
		return AVB_IO_RESULT_OK;
	}
	//MTK code ends
	if (rc)
		return rc;

	*out_rollback_index = (u64)param[1].u.value.a << 32 |
			      (u32)param[1].u.value.b;
	return AVB_IO_RESULT_OK;
#endif
}

/**
 * write_rollback_index() - sets the rollback index corresponding to the
 * location of given by @out_rollback_index.
 *
 * @ops: contains AvbOps handlers
 * @rollback_index_slot:
 * @rollback_index: rollback index to write.
 *
 * @return
 *       AVB_IO_RESULT_OK, if the roolback index was retrieved
 */
static AvbIOResult write_rollback_index(AvbOps *ops,
					size_t rollback_index_slot,
					u64 rollback_index)
{
#ifndef CONFIG_OPTEE_TA_AVB
	/* For now this is a no-op. */
	printf("%s not supported yet\n", __func__);

#if defined(CONFIG_HAPS)
	return AVB_IO_RESULT_OK;
#else
	return AVB_IO_RESULT_ERROR_IO;
#endif
#else
	struct tee_param param[2];

	if (rollback_index_slot >= TA_AVB_MAX_ROLLBACK_LOCATIONS)
		return AVB_IO_RESULT_ERROR_NO_SUCH_VALUE;

	memset(param, 0, sizeof(param));
	param[0].attr = TEE_PARAM_ATTR_TYPE_VALUE_INPUT;
	param[0].u.value.a = rollback_index_slot;
	param[1].attr = TEE_PARAM_ATTR_TYPE_VALUE_INPUT;
	param[1].u.value.a = (u32)(rollback_index >> 32);
	param[1].u.value.b = (u32)rollback_index;

	return invoke_func(ops->user_data, TA_AVB_CMD_WRITE_ROLLBACK_INDEX,
			   ARRAY_SIZE(param), param);
#endif
}

//MTK code starts
static AvbIOResult read_is_device_unlocked_from_ENV(bool *out_is_unlocked)
{
#if defined(CONFIG_ANDROID_AVB_PURELINUX_ENABLE)
	//force *out_is_unlocked=lock for MLR
	*out_is_unlocked = false;
	return AVB_IO_RESULT_OK;
#else
	char *dev_state = env_get("devicestate");
	if(dev_state != NULL && strncmp(dev_state, "unlock", strlen("unlock")) == 0) {
		*out_is_unlocked = true;
		return AVB_IO_RESULT_OK;

	}
	else if(dev_state != NULL && strncmp(dev_state, "lock", strlen("lock")) == 0) {
		*out_is_unlocked = false;
		return AVB_IO_RESULT_OK;
	}
	else {
		*out_is_unlocked = false;
		return AVB_IO_RESULT_ERROR_IO;
	}
#endif
}

static AvbIOResult write_device_status_TA_AVB(AvbOps *ops, uint16_t value)
{
#ifdef CONFIG_OPTEE_TA_AVB
	AvbIOResult rc;
	struct tee_param param;
	memset(&param, 0, sizeof(param));
	param.attr = TEE_PARAM_ATTR_TYPE_VALUE_INPUT;
	param.u.value.a = (size_t)value;

	rc = invoke_func(ops->user_data, TA_AVB_CMD_WRITE_LOCK_STATE, 1, &param);
	if (rc)
		return rc;
	//read data out and check data consistency again to ensure it's been written properly
	param.attr = TEE_PARAM_ATTR_TYPE_VALUE_OUTPUT;
	param.u.value.a = ~value;
	rc = invoke_func(ops->user_data, TA_AVB_CMD_READ_LOCK_STATE, 1, &param);
	if (rc)
		return rc;
	if (value != param.u.value.a)
		rc = AVB_IO_RESULT_ERROR_IO;
	return rc;
#elif defined(CONFIG_HAPS)
	return AVB_IO_RESULT_OK;
#else
	return AVB_IO_RESULT_ERROR_IO;
#endif
}

#define KM_DATA_NAME "kmdata"
static AvbIOResult read_vbmeta_data(AvbOps *ops,
				       void* buffer,
				       size_t size)
{
#ifdef CONFIG_OPTEE_TA_AVB
	AvbIOResult rc;
	struct tee_param param[2];
	void* NameBuffer = NULL;
	void* Invokebuffer = NULL;

	NameBuffer = malloc_TEE(sizeof(KM_DATA_NAME));
	if(!NameBuffer) {
		return AVB_IO_RESULT_ERROR_INSUFFICIENT_SPACE;
	}
	Invokebuffer = malloc_TEE(size);
	if(!Invokebuffer) {
		free_TEE(NameBuffer);
		return AVB_IO_RESULT_ERROR_INSUFFICIENT_SPACE;
	}
	memset(param, 0, sizeof(param));
	memset(Invokebuffer, 0, size);
	memcpy(NameBuffer, KM_DATA_NAME, sizeof(KM_DATA_NAME));
	param[0].attr = TEE_PARAM_ATTR_TYPE_MEMREF_INPUT;
	param[0].u.memref.shm_offs = (ulong)NameBuffer;
	param[0].u.memref.size = sizeof(KM_DATA_NAME);
	param[1].attr = TEE_PARAM_ATTR_TYPE_MEMREF_INOUT;
	param[1].u.memref.shm_offs = (ulong)Invokebuffer;
	param[1].u.memref.size = size;

	rc = invoke_func(ops->user_data, TA_AVB_CMD_READ_PERSIST_VALUE,
			 ARRAY_SIZE(param), param);

	if(rc == TEE_SUCCESS) {
		memcpy(buffer, Invokebuffer, size);
	}
	free_TEE(NameBuffer);
	free_TEE(Invokebuffer);
	return rc;
#elif defined(CONFIG_HAPS)
	return AVB_IO_RESULT_OK;
#else
	printf("%s not supported yet\n", __func__);
	return AVB_IO_RESULT_ERROR_IO;
#endif
}

static AvbIOResult write_vbmeta_data(AvbOps *ops,
				       const void* buffer,
				       size_t size)
{
#ifdef CONFIG_OPTEE_TA_AVB
	AvbIOResult rc;
	struct tee_param param[2];
	void* NameBuffer = NULL;
	void* Invokebuffer = NULL;

	NameBuffer = malloc_TEE(sizeof(KM_DATA_NAME));
	if(!NameBuffer) {
		return AVB_IO_RESULT_ERROR_INSUFFICIENT_SPACE;
	}
	Invokebuffer = malloc_TEE(size);
	if(!Invokebuffer) {
		free_TEE(NameBuffer);
		return AVB_IO_RESULT_ERROR_INSUFFICIENT_SPACE;
	}
	memset(param, 0, sizeof(param));
	memcpy(Invokebuffer, buffer, size);
	memcpy(NameBuffer, KM_DATA_NAME, sizeof(KM_DATA_NAME));
	param[0].attr = TEE_PARAM_ATTR_TYPE_MEMREF_INPUT;
	param[0].u.memref.shm_offs = (ulong)NameBuffer;
	param[0].u.memref.size = sizeof(KM_DATA_NAME);
	param[1].attr = TEE_PARAM_ATTR_TYPE_MEMREF_INPUT;
	param[1].u.memref.shm_offs = (ulong)Invokebuffer;
	param[1].u.memref.size = size;

	rc = invoke_func(ops->user_data, TA_AVB_CMD_WRITE_PERSIST_VALUE,
			 ARRAY_SIZE(param), param);
	if (rc)
		goto free_buffers;
	//read data out and check data consistency again to ensure it's been written properly
	rc = read_vbmeta_data(ops, Invokebuffer, size);
	if (rc)
		goto free_buffers;
	if( 0 != memcmp(Invokebuffer, buffer, size))
	{
		rc = AVB_IO_RESULT_ERROR_IO;
	}
free_buffers:
	free_TEE(NameBuffer);
	free_TEE(Invokebuffer);
	return rc;
#elif defined(CONFIG_HAPS)
	return AVB_IO_RESULT_OK;
#else
	printf("%s not supported yet\n", __func__);
	return AVB_IO_RESULT_ERROR_IO;
#endif
}

/**
 * validate_public_key_for_partition() - checks if the given public key used to sign
 * the partition is trusted
 *
 * @ops: AvbOps, contains AVB ops handlers
 * @partition: partition name
 * @public_key_data: public key for verifying vbmeta partition signature
 * @public_key_length: length of public key
 * @public_key_metadata:
 * @public_key_metadata_length:
 * @out_key_is_trusted:
 * @out_rollback_index_location: partition rollback index location
 *
 * @return:
 *      AVB_IO_RESULT_OK, if partition was found and read operation succeed
 */
#define RECOVERY_ROLLBACK_INDEX_LOCATION 1
static AvbIOResult validate_public_key_for_partition(AvbOps *ops,
					      const char* partition,
					      const u8 *public_key_data,
					      size_t public_key_length,
					      const u8
					      *public_key_metadata,
					      size_t
					      public_key_metadata_length,
					      bool *out_key_is_trusted,
					      uint32_t* out_rollback_index_location)
{
	//partition is recovery only
#ifdef AMAZON_AVB_SIGNING
	AvbRSAPublicKeyHeader *key_hdr;
	uint8_t *pubk;
	uint32_t pubk_sz = 0;

	if (!ops
		|| !partition
		|| !public_key_data
		|| !out_key_is_trusted
		|| !out_rollback_index_location)
		return AVB_IO_RESULT_ERROR_IO;

	if (0 != memcmp((void *)partition, (void *)"recovery", strlen("recovery"))) {
		printf("%s: error, %s partition is not supported\n", __func__, partition);
		return AVB_IO_RESULT_ERROR_NO_SUCH_PARTITION;
	}

	key_hdr = (AvbRSAPublicKeyHeader *)public_key_data;
	pubk = (uint8_t *)(public_key_data + sizeof(AvbRSAPublicKeyHeader));
	pubk_sz = avb_htobe32(key_hdr->key_num_bits) / 8;

	*out_key_is_trusted = false;
	if (pubk_sz != AVB_VBMETA_PUBKEY_SIZE) {
		printf("%s public key size(%d) is invalid\n", __func__, pubk_sz);
		return AVB_IO_RESULT_ERROR_IO;
	}

	if (memcmp((void *)avb_vbmeta_pubkey_prod, (void *)pubk, pubk_sz) == 0) {
		*out_key_is_trusted = true;
		printf("AVB prod key is found.\n");
#if !defined(CONFIG_ANDROID_AB)
	} else if (memcmp((void *)avb_recovery_pubkey_prod, (void *)pubk, pubk_sz) == 0) {
		*out_key_is_trusted = true;
		printf("AVB recovery prod key is found.\n");
#endif
	} else if (memcmp((void *)avb_vbmeta_pubkey_dev, (void *)pubk, pubk_sz) == 0) {
		printf("AVB dev key is found.\n");
		if (is_lockdown())
			printf("AVB dev key is not allowed on locked production device!\n");
		else
			*out_key_is_trusted = true;
	}
	*out_rollback_index_location = RECOVERY_ROLLBACK_INDEX_LOCATION;
#else
	if (!public_key_length || !public_key_data || !out_key_is_trusted)
		return AVB_IO_RESULT_ERROR_IO;

	*out_key_is_trusted = false;
	if (public_key_length != sizeof(avb_root_pub))
		return AVB_IO_RESULT_ERROR_IO;

	if (memcmp(avb_root_pub, public_key_data, public_key_length) == 0)
		*out_key_is_trusted = true;

	*out_rollback_index_location = RECOVERY_ROLLBACK_INDEX_LOCATION;
#endif
	return AVB_IO_RESULT_OK;
}
//MTK code ends

/**
 * read_is_device_unlocked() - gets whether the device is unlocked
 *
 * @ops: contains AVB ops handlers
 * @out_is_unlocked: device unlock state is stored here, true if unlocked,
 *       false otherwise
 *
 * @return:
 *       AVB_IO_RESULT_OK: state is retrieved successfully
 *       AVB_IO_RESULT_ERROR_IO: an error occurred
 */
static AvbIOResult read_is_device_unlocked(AvbOps *ops, bool *out_is_unlocked)
{
#ifndef CONFIG_OPTEE_TA_AVB
#if 0
	/* For now we always return that the device is unlocked. */

	printf("%s not supported yet\n", __func__);

	*out_is_unlocked = true;

	return AVB_IO_RESULT_OK;
#endif
	return read_is_device_unlocked_from_ENV(out_is_unlocked);

#else
	AvbIOResult rc;
	struct tee_param param = { .attr = TEE_PARAM_ATTR_TYPE_VALUE_OUTPUT };

	rc = invoke_func(ops->user_data, TA_AVB_CMD_READ_LOCK_STATE, 1, &param);
	//MTK code starts
	//In case of AVB_IO_RESULT_ERROR_RPMBKEYNOTEXISTS
	if( rc == AVB_IO_RESULT_ERROR_RPMBKEYNOTEXISTS) {
		return read_is_device_unlocked_from_ENV(out_is_unlocked);
	}
	//MTK code ends
	if (rc)
		return rc;
	*out_is_unlocked = !param.u.value.a;
	//MTK code starts
	//Get device lock status from ENV first
	bool out_is_unlocked_from_env;
	if(read_is_device_unlocked_from_ENV(&out_is_unlocked_from_env) != AVB_IO_RESULT_OK) {
		return AVB_IO_RESULT_ERROR_IO;
	}
	UBOOT_INFO("out_is_unlocked_from_env=%d, out_is_unlocked=%d\n", out_is_unlocked_from_env, *out_is_unlocked);
	/*
	In case of an uninitialised RPMB filesystem, TA_AVB_CMD_READ_LOCK_STATE returns UNLOCKED (out_is_unlocked=1) as default,
	device status in ENV is LOCKED (out_is_unlocked_from_env=0) as default,
	but we expect the default device status is LOCKED (out_is_unlocked=0),
	in such case we override device status to LOCKED (out_is_unlocked=0).
	For cases other than this, out_is_unlocked always sticks to the value from TA_AVB_CMD_READ_LOCK_STATE only.
	Here is the table for all cases
	------------------------------------------------------
	ENV      |   RPMB     |  status    |  out_is_unlocked
	------------------------------------------------------
	LOCKED   |   UNLOCKED |  LOCKED    |  0 (the overridden case)
	------------------------------------------------------
	UNLOCKED |   LOCKED   |  LOCKED    |  0
	------------------------------------------------------
	LOCKED   |   LOCKED   |  LOCKED    |  0
	------------------------------------------------------
	UNLOCKED |   UNLOCKED |  UNLOCKED  |  1
	------------------------------------------------------
	*/
	if(!out_is_unlocked_from_env && *out_is_unlocked) {
		*out_is_unlocked = 0; //override device status to LOCKED and store it into RPMB
		rc = write_device_status_TA_AVB(ops, true);
		if (rc) {
			printf("Can't write device lock state\n");
			return rc;
		}
	}
	//MTK code ends
	return AVB_IO_RESULT_OK;
#endif
}

static AvbIOResult write_device_status(AvbOps *ops, uint16_t value)
{
	const char *is_skip = env_get("skip_wipe_userdata_once");
	unsigned long bootmode = 0;

	//check value's validity
	if( value != 0 && value != 1 ) {
		printf("%s:lock = 1, unlock = 0: '%d' is not support.\n", __FUNCTION__, value);
		return AVB_IO_RESULT_ERROR_IO;
	}
	//check current devicestate
	bool unlocked = false;
	int devicestate = 1;
	if (read_is_device_unlocked(ops, &unlocked) != AVB_IO_RESULT_OK) {
		printf("Can't determine device lock state.\n");
		return AVB_IO_RESULT_ERROR_IO;
	}
	if (unlocked)
		devicestate= 0; //unlock=true, means devicestate is unlocked
	else
		devicestate= 1;//unlock=false, means devicestate is locked

	if(value == devicestate) {
		printf("%s : set device state unchange\n", __FUNCTION__);
		return AVB_IO_RESULT_ERROR_IO;
	}

	bootmode = simple_strtoul(env_get("bootmode"), NULL, 10);
	//not touch fos userdata partition in diagmode
	if (bootmode == IDME_BOOTMODE_DIAG) {
		printf("skip wipe_user_data_in_uboot in bootmode %ld\n", bootmode);
	//skip if skip_wipe_userdata_once is yes
	} else if ((is_skip && strncmp(is_skip, "yes", 3) == 0)) {
		printf("skip wipe_user_data_in_uboot once\n");
		env_set("skip_wipe_userdata_once", "no");
		env_save();
	} else {
		//wipe userdata when lock/unlock state change
		if(wipe_user_data_in_uboot() != 0)
			return AVB_IO_RESULT_ERROR_IO;
	}

	AvbIOResult rc = write_device_status_TA_AVB(ops, value);
	if (rc) {
		printf("Can't write device lock state\n");
		return rc;
	}
	if(value == 0) {
		env_set("devicestate", "unlock");
		env_save();
	}
	if(value == 1) {
		env_set("devicestate", "lock");
		env_save();
	}

	return AVB_IO_RESULT_OK;
}

/**
 * get_unique_guid_for_partition() - gets the GUID for a partition identified
 * by a string name
 *
 * @ops: contains AVB ops handlers
 * @partition: partition name (NUL-terminated UTF-8 string)
 * @guid_buf: buf, used to copy in GUID string. Example of value:
 *      527c1c6d-6361-4593-8842-3c78fcd39219
 * @guid_buf_size: @guid_buf buffer size
 *
 * @return:
 *      AVB_IO_RESULT_OK, on success (GUID found)
 *      AVB_IO_RESULT_ERROR_IO, if incorrect buffer size (@guid_buf_size) was
 *             provided
 *      AVB_IO_RESULT_ERROR_NO_SUCH_PARTITION, if partition was not found
 */
static AvbIOResult get_unique_guid_for_partition(AvbOps *ops,
						 const char *partition,
						 char *guid_buf,
						 size_t guid_buf_size)
{
#ifndef CONFIG_LEGACY_AVB_PARTITION
	struct mmc_part *part;
	size_t uuid_size;

	part = get_partition(ops, partition);
	if (!part)
		return AVB_IO_RESULT_ERROR_NO_SUCH_PARTITION;

	uuid_size = sizeof(part->info.uuid);
	if (uuid_size > guid_buf_size)
		return AVB_IO_RESULT_ERROR_IO;

	memcpy(guid_buf, part->info.uuid, uuid_size);
#else
	disk_partition_t partition_info;
	size_t uuid_size;

	if (!partition || !guid_buf)
		return AVB_IO_RESULT_ERROR_IO;

	uuid_size = sizeof(partition_info.uuid);
	if (uuid_size > guid_buf_size)
		return AVB_IO_RESULT_ERROR_IO;

	if(get_boot_partition_info(partition, &partition_info, NULL) != 0)
		memset(partition_info.uuid, 0, uuid_size);
	memcpy(guid_buf, partition_info.uuid, uuid_size);
#endif
	guid_buf[uuid_size - 1] = 0;

	return AVB_IO_RESULT_OK;
}

/**
 * get_size_of_partition() - gets the size of a partition identified
 * by a string name
 *
 * @ops: contains AVB ops handlers
 * @partition: partition name (NUL-terminated UTF-8 string)
 * @out_size_num_bytes: returns the value of a partition size
 *
 * @return:
 *      AVB_IO_RESULT_OK, on success (GUID found)
 *      AVB_IO_RESULT_ERROR_INSUFFICIENT_SPACE, out_size_num_bytes is NULL
 *      AVB_IO_RESULT_ERROR_NO_SUCH_PARTITION, if partition was not found
 */
static AvbIOResult get_size_of_partition(AvbOps *ops,
					 const char *partition,
					 u64 *out_size_num_bytes)
{
#ifndef CONFIG_LEGACY_AVB_PARTITION
	struct mmc_part *part;

	if (!out_size_num_bytes)
		return AVB_IO_RESULT_ERROR_INSUFFICIENT_SPACE;

	part = get_partition(ops, partition);
	if (!part)
		return AVB_IO_RESULT_ERROR_NO_SUCH_PARTITION;

	*out_size_num_bytes = part->info.blksz * part->info.size;

#else
	disk_partition_t partition_info;

	if (!out_size_num_bytes)
		return AVB_IO_RESULT_ERROR_INSUFFICIENT_SPACE;

	if(get_boot_partition_info(partition, &partition_info, NULL) != 0)
		return AVB_IO_RESULT_ERROR_NO_SUCH_PARTITION;

	*out_size_num_bytes = partition_info.size * partition_info.blksz;
#endif

	return AVB_IO_RESULT_OK;
}

/**
 * ============================================================================
 * AVB2.0 AvbOps alloc/initialisation/free
 * ============================================================================
 */
AvbOps *avb_ops_alloc(int boot_device)
{
	struct AvbOpsData *ops_data;

	ops_data = avb_calloc(sizeof(struct AvbOpsData));
	if (!ops_data)
		return NULL;

	ops_data->ops.user_data = ops_data;

	ops_data->ops.read_from_partition = read_from_partition;
	ops_data->ops.write_to_partition = write_to_partition;
	ops_data->ops.validate_vbmeta_public_key = validate_vbmeta_public_key;
	ops_data->ops.read_rollback_index = read_rollback_index;
	ops_data->ops.write_rollback_index = write_rollback_index;
	ops_data->ops.read_is_device_unlocked = read_is_device_unlocked;
	//MTK code starts
	ops_data->ops.write_device_status = write_device_status;
	ops_data->ops.write_vbmeta_data = write_vbmeta_data;
	ops_data->ops.read_vbmeta_data = read_vbmeta_data;
	ops_data->ops.validate_public_key_for_partition = validate_public_key_for_partition;
	//MTK code ends
	ops_data->ops.get_unique_guid_for_partition =
		get_unique_guid_for_partition;
	ops_data->ops.get_size_of_partition = get_size_of_partition;
	ops_data->mmc_dev = boot_device;

	return &ops_data->ops;
}

void avb_ops_free(AvbOps *ops)
{
	struct AvbOpsData *ops_data;

	if (!ops)
		return;

	ops_data = ops->user_data;

	if (ops_data) {
#ifdef CONFIG_OPTEE_TA_AVB
		if (ops_data->tee)
			tee_close_session(ops_data->tee, ops_data->session);
#endif
		avb_free(ops_data);
	}
}
