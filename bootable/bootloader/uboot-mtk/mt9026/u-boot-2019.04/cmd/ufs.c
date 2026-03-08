/*
 * UFS support.
 */
#include <common.h>
#include <command.h>
#include <blk.h>
#include <dm.h>
#include <hexdump.h>
#include "../drivers/ufs/ufs.h"
#include <ufs_scsi.h>

int ufs_curr_target = -1; /* current target */
static int ufs_curr_dev = -1; /* current lun */
static int CapAdjFactor[8];

static int do_ufs_get_cid(int flag, int argc, char * const argv[])
{
	u8 *addr;
	int err;
	struct ufs_hba *hba = find_ufs_device(ufs_curr_target);

	if (argc < 2)
		return CMD_RET_USAGE;

	addr = (u8 *)simple_strtoul(argv[1], NULL, 16);
	err = ufs_get_cid(hba, addr);
	if (err)
		return CMD_RET_FAILURE;

	return CMD_RET_SUCCESS;
}

#if CONFIG_IS_ENABLED(CMD_UFS_RPMB)
static int confirm_key_prog(void)
{
	puts("Warning: Programming authentication key can be done only once !\n"
	     "         Use this command only if you are sure of what you are doing,\n"
	     "Really perform the key programming? <y/N> ");
	//if (confirm_yesno())
		return 1;

	puts("Authentication key programming aborted\n");
	return 0;
}
static int do_ufs_rpmb_key(int flag, int argc, char * const argv[])
{
	void *key_addr;
	struct ufs_hba *hba = find_ufs_device(ufs_curr_target);

	if (argc != 2)
		return CMD_RET_USAGE;

	key_addr = (void *)simple_strtoul(argv[1], NULL, 16);
	if (!confirm_key_prog())
		return CMD_RET_FAILURE;
	if (ufs_rpmb_set_key(hba, key_addr)) {
		printf("ERROR - Key already programmed ?\n");
		return CMD_RET_FAILURE;
	}
	return CMD_RET_SUCCESS;
}
static int do_ufs_rpmb_read(int flag, int argc, char * const argv[])
{
	u16 blk, cnt;
	void *addr;
	int n;
	void *key_addr = NULL;
	struct ufs_hba *hba = find_ufs_device(ufs_curr_target);

	if (argc < 4)
		return CMD_RET_USAGE;

	if(!hba)
		return CMD_RET_FAILURE;

	addr = (void *)simple_strtoul(argv[1], NULL, 16);
	blk = simple_strtoul(argv[2], NULL, 16);
	cnt = simple_strtoul(argv[3], NULL, 16);

	if (argc == 5)
		key_addr = (void *)simple_strtoul(argv[4], NULL, 16);

	printf("\nUFS RPMB read: dev # %d, block # %d, count %d ... ",
	       ufs_curr_target, blk, cnt);
	n =  ufs_rpmb_read(hba, addr, blk, cnt, key_addr);

	printf("%d RPMB blocks read: %s\n", n, (n == cnt) ? "OK" : "ERROR");
	if (n != cnt)
		return CMD_RET_FAILURE;
	return CMD_RET_SUCCESS;
}
static int do_ufs_rpmb_write(int flag, int argc, char * const argv[])
{
	u16 blk, cnt;
	void *addr;
	int n;
	void *key_addr;
	struct ufs_hba *hba = find_ufs_device(ufs_curr_target);

	if (argc != 5)
		return CMD_RET_USAGE;

	if(!hba)
		return CMD_RET_FAILURE;

	addr = (void *)simple_strtoul(argv[1], NULL, 16);
	blk = simple_strtoul(argv[2], NULL, 16);
	cnt = simple_strtoul(argv[3], NULL, 16);
	key_addr = (void *)simple_strtoul(argv[4], NULL, 16);

	printf("\nUFS RPMB write: dev # %d, block # %d, count %d ... ",
	       ufs_curr_target, blk, cnt);
	n =  ufs_rpmb_write(hba, addr, blk, cnt, key_addr);

	printf("%d RPMB blocks written: %s\n", n, (n == cnt) ? "OK" : "ERROR");
	if (n != cnt)
		return CMD_RET_FAILURE;
	return CMD_RET_SUCCESS;
}
static int do_ufs_rpmb_counter(int flag, int argc, char * const argv[])
{
	u32 counter;
	struct ufs_hba *hba = find_ufs_device(ufs_curr_target);

	if(!hba)
		return CMD_RET_FAILURE;

	if (ufs_rpmb_get_counter(hba, &counter))
		return CMD_RET_FAILURE;
	printf("RPMB Write counter = %x\n", counter);
	return CMD_RET_SUCCESS;
}
#endif

#if 0
static int do_ufs_rw_part(int flag, int argc, char * const argv[])
{
	struct blk_desc *dev_desc = NULL;
	disk_partition_t part_info;
	ulong offset = 0u;
	ulong limit = 0u;
	void *addr;
	uint blk;
	uint cnt;
	int ret;
	ulong n;

	dev_desc = blk_get_dev("ufs", ufs_curr_dev);
	if (dev_desc == NULL) {
		printf("Block device %s %d not supported\n", "ufs", ufs_curr_dev);
		return CMD_RET_FAILURE;
	}

	ret = part_get_info_by_name(dev_desc, argv[2], &part_info);
	if (ret == -1) {
		printf("Can't find partition '%s'\n", argv[2]);
		return CMD_RET_FAILURE;
	}
	offset = part_info.start;
	limit = part_info.size;

	addr = (void *)simple_strtoul(argv[1], NULL, 16);

	if (strcmp(argv[0], "read.p") == 0 || strcmp(argv[0], "write.p") == 0) {

		blk = 0;
		cnt = simple_strtoul(argv[3], NULL, 16);
	}
	else {
		blk = simple_strtoul(argv[3], NULL, 16);
		cnt = simple_strtoul(argv[4], NULL, 16);
	}

	if (cnt + blk > limit) {
		printf("Read out of range\n");
		return CMD_RET_FAILURE;
	}

	if (strncmp(argv[0], "read", 4) == 0)
		n = blk_dread(dev_desc, offset + blk, cnt, addr);
	else
		n = blk_dwrite(dev_desc, offset + blk, cnt, addr);

	printf("%ld blocks written: %s\n", n,
	       n == cnt ? "OK" : "ERROR");
	return n == cnt ? CMD_RET_SUCCESS : CMD_RET_FAILURE;
}
#endif

static int do_ufs_erase(int flag, int argc, char * const argv[])
{
	struct blk_desc *dev_desc = NULL;
	//disk_partition_t part_info;
	lbaint_t blk;
	ulong cnt;
	//int ret;
	ulong n;
			
	dev_desc = blk_get_dev("ufs", ufs_curr_dev);
	if (dev_desc == NULL) {
		printf("Block device %s %d not supported\n", "ufs", ufs_curr_dev);
		return CMD_RET_FAILURE;
	}

	#if 0
	if(strcmp(argv[0], "erase") == 0) { /* erase */
		blk = simple_strtoul(argv[1], NULL, 16);
		cnt = simple_strtoul(argv[2], NULL, 16);
	}
	else { /* erase.p */
		ret = part_get_info_by_name(dev_desc, argv[1], &part_info);
		if (ret == -1) {
			printf("Can't find partition '%s'\n", argv[2]);
			return CMD_RET_FAILURE;
		}
		blk = part_info.start;
		cnt = part_info.size;
	}
	#else
	blk = simple_strtoul(argv[1], NULL, 16);
	cnt = simple_strtoul(argv[2], NULL, 16);
	#endif

	n = blk_derase(dev_desc, blk, cnt);

	printf("\nufs erase: device %d block # "LBAFU", count %lu ... ",
	       ufs_curr_dev, blk, cnt);

	printf("%ld blocks erased: %s\n", n,
	       n == cnt ? "OK" : "ERROR");
	return n == cnt ? CMD_RET_SUCCESS : CMD_RET_FAILURE;
}

static int parse_hwpart_unit(struct ufs_unit_descr *punit,
								u64 qAllocationUnitSize, u64 *qAvailNumAllocUnits,
								int argc, char * const argv[])
{
	int i;
	u64 val;

	i = 1;
	while (i < argc) {
		if (!strcmp(argv[i], "BootLunID")) {
			if (i + 1 >= argc)
				return -1;
			punit->bBootLunID = simple_strtoul(argv[i+1], NULL, 16);
			i += 2;
		} else if (!strcmp(argv[i], "LUWriteProtect")) {
			if (i + 1 >= argc)
				return -1;
			punit->bLUWriteProtect = simple_strtoul(argv[i+1], NULL, 16);
			i += 2;
		} else if (!strcmp(argv[i], "MemoryType")) {
			if (i + 1 >= argc)
				return -1;
			punit->bMemoryType = simple_strtoul(argv[i+1], NULL, 16);
			i += 2;
		} else if (!strcmp(argv[i], "DataReliability")) {
			if (i + 1 >= argc)
				return -1;
			punit->bDataReliability = simple_strtoul(argv[i+1], NULL, 16);
			i += 2;
		} else if (!strcmp(argv[i], "LogicalBlockSize")) {
			if (i + 1 >= argc)
				return -1;
			punit->bLogicalBlockSize = simple_strtoul(argv[i+1], NULL, 16);
			i += 2;
		} else if (!strcmp(argv[i], "ProvisioningType")) {
			if (i + 1 >= argc)
				return -1;
			punit->bProvisioningType = simple_strtoul(argv[i+1], NULL, 16);
			i += 2;
		} else if (!strcmp(argv[i], "ContextCapabilities")) {
			if (i + 1 >= argc)
				return -1;
			punit->wContextCapabilities = cpu_to_be16(simple_strtoul(argv[i+1], NULL, 16));
			i += 2;
		} else {
			break;
		}

		printf("%s %s\n", argv[i-2], argv[i-1]);
	}

	printf("size %s\n", argv[0]);
	if (!strcmp(argv[0], "-")) {
		punit->dNumAllocUnits = cpu_to_be32(*qAvailNumAllocUnits);
		*qAvailNumAllocUnits = 0;
	}
	else {
		val = simple_strtoul(argv[0], NULL, 16);
		punit->dNumAllocUnits
			= cpu_to_be32((val*CapAdjFactor[punit->bMemoryType]+qAllocationUnitSize-1)/qAllocationUnitSize);
		*qAvailNumAllocUnits -= ((val*CapAdjFactor[punit->bMemoryType]+qAllocationUnitSize-1)/qAllocationUnitSize);
	}

	punit->bLUEnable = 1;

	return i;
}

static int do_ufs_hwpartition(int flag, int argc, char * const argv[])
{
	struct ufs_config_descr *pconf;
	struct ufs_unit_descr *punit;
	int i, r, pidx;
	u8 desc_buf[QUERY_DESC_MAX_SIZE];
	u32 dBootLunEn;
	int err;
	u64 qTotalRawDeviceCap = 0;
	u32 dSegmentSize = 0;
	u8 bAllocationUnitSize = 0;
	u64 qAllocUnitSize;
	u64 qAvailNumAllocUnits;
	struct ufs_hba *hba = find_ufs_device(ufs_curr_target);
	u8 bUD0BaseOffset;// = desc_buf[DEVICE_DESC_PARAM_UD_OFFSET];
	u8 bUDConfigPLength;// = desc_buf[DEVICE_DESC_PARAM_UD_LEN];
	int buff_len;;

	if (argc < 2)
		return CMD_RET_USAGE;

	err = ufs_read_desc(hba, QUERY_DESC_IDN_DEVICE, 0, desc_buf, QUERY_DESC_DEVICE_DEF_SIZE);
	if (err < 0) {
		printf("Failed reading Device Descr. err = %d\n", err);
		return CMD_RET_FAILURE;
	}
	bUD0BaseOffset = desc_buf[DEVICE_DESC_PARAM_UD_OFFSET];
	bUDConfigPLength = desc_buf[DEVICE_DESC_PARAM_UD_LEN];

	err = ufshcd_read_geometry_desc(hba, desc_buf, QUERY_DESC_GEOMETRY_DEF_SIZE);
	if (err)  {
		printf("Failed reading geometry Desc. err = %d\n", err);
		return CMD_RET_FAILURE;
	}
	#if 1
	print_hex_dump("geometry desc ", DUMP_PREFIX_OFFSET, 16, 1, desc_buf,
		QUERY_DESC_GEOMETRY_DEF_SIZE, false);
	#endif
	qTotalRawDeviceCap =
		(((u64) desc_buf[GEOMETRY_DESC_PARAM_DEV_CAP]) << 56) |
		(((u64) desc_buf[GEOMETRY_DESC_PARAM_DEV_CAP+1]) << 48) |
		(((u64) desc_buf[GEOMETRY_DESC_PARAM_DEV_CAP+2]) << 40) |
		(((u64) desc_buf[GEOMETRY_DESC_PARAM_DEV_CAP+3]) << 32) |
		(((u64) desc_buf[GEOMETRY_DESC_PARAM_DEV_CAP+4]) << 24) |
		(((u64) desc_buf[GEOMETRY_DESC_PARAM_DEV_CAP+5]) << 16) |
		(((u64) desc_buf[GEOMETRY_DESC_PARAM_DEV_CAP+6]) << 8) |
		(((u64) desc_buf[GEOMETRY_DESC_PARAM_DEV_CAP+7]) << 0);
	dSegmentSize =
		(((u64) desc_buf[GEOMETRY_DESC_PARAM_SEG_SIZE]) << 24) |
		(((u64) desc_buf[GEOMETRY_DESC_PARAM_SEG_SIZE+1]) << 16) |
		(((u64) desc_buf[GEOMETRY_DESC_PARAM_SEG_SIZE+2]) << 8) |
		(((u64) desc_buf[GEOMETRY_DESC_PARAM_SEG_SIZE+3]) << 0);
	bAllocationUnitSize = desc_buf[GEOMETRY_DESC_PARAM_ALLOC_UNIT_SIZE];
	qAllocUnitSize = (u64)dSegmentSize*(u64)512*(u64)bAllocationUnitSize;
	qAvailNumAllocUnits = qTotalRawDeviceCap*(u64)512/qAllocUnitSize;
	printf("qTotalRawDeviceCap : %llXh\n", qTotalRawDeviceCap);
	printf("dSegmentSize : %Xh\n", dSegmentSize);
	printf("bAllocationUnitSize : %Xh\n", bAllocationUnitSize);
	printf("qAllocUnitSize : %llXh\n", qAllocUnitSize);
	printf("qAvailNumAllocUnits : %llXh\n", qAvailNumAllocUnits);

	CapAdjFactor[0] = 1;
	CapAdjFactor[1] = ((desc_buf[GEOMETRY_DESC_PARAM_SCM_CAP_ADJ_FCTR]<<8)+desc_buf[GEOMETRY_DESC_PARAM_SCM_CAP_ADJ_FCTR+1])/256;
	CapAdjFactor[2] = ((desc_buf[GEOMETRY_DESC_PARAM_NPM_CAP_ADJ_FCTR]<<8)+desc_buf[GEOMETRY_DESC_PARAM_NPM_CAP_ADJ_FCTR+1])/256;
	CapAdjFactor[3] = ((desc_buf[GEOMETRY_DESC_PARAM_ENM1_CAP_ADJ_FCTR]<<8)+desc_buf[GEOMETRY_DESC_PARAM_ENM1_CAP_ADJ_FCTR+1])/256;
	CapAdjFactor[4] = ((desc_buf[GEOMETRY_DESC_PARAM_ENM2_CAP_ADJ_FCTR]<<8)+desc_buf[GEOMETRY_DESC_PARAM_ENM2_CAP_ADJ_FCTR+1])/256;
	CapAdjFactor[5] = ((desc_buf[GEOMETRY_DESC_PARAM_ENM3_CAP_ADJ_FCTR]<<8)+desc_buf[GEOMETRY_DESC_PARAM_ENM3_CAP_ADJ_FCTR+1])/256;
	CapAdjFactor[6] = ((desc_buf[GEOMETRY_DESC_PARAM_ENM4_CAP_ADJ_FCTR]<<8)+desc_buf[GEOMETRY_DESC_PARAM_ENM4_CAP_ADJ_FCTR+1])/256;
	CapAdjFactor[7] = 0;

	memset((void *)desc_buf, 0, QUERY_DESC_MAX_SIZE);
	pconf = (struct ufs_config_descr *)&desc_buf[0];

	/* set default value */
	pconf->bBootEnable = 1;
	pconf->bDescrAccessEn = 1;
	pconf->bInitPowerMode = 1;
	pconf->bHighPriorityLUN = 0x7F;
	pconf->bRPMBRegionEnable = 1;

	i = 2;
	while (i < argc) {
		if (!strcmp(argv[i], "BootEn")) {
			pconf->bBootEnable = simple_strtoul(argv[i+1], NULL, 16);
			i += 2;
		} else if (!strcmp(argv[i], "DescrAccessEn")) {
			pconf->bDescrAccessEn = simple_strtoul(argv[i+1], NULL, 16);
			i += 2;
		} else if (!strcmp(argv[i], "InitPowerMode")) {
			pconf->bInitPowerMode = simple_strtoul(argv[i+1], NULL, 16);
			i += 2;
		} else if (!strcmp(argv[i], "HighPriorityLUN")) {
			pconf->bHighPriorityLUN = simple_strtoul(argv[i+1], NULL, 16);
			i += 2;
		} else if (!strcmp(argv[i], "SecureRemovalType")) {
			pconf->bSecureRemovalType = simple_strtoul(argv[i+1], NULL, 16);
			i += 2;
		} else if (!strcmp(argv[i], "InitActiveICCLevel")) {
			pconf->bInitActiveICCLevel = simple_strtoul(argv[i+1], NULL, 16);
			i += 2;
		} else if (!strcmp(argv[i], "PeriodicRTCUpdate")) {
			pconf->wPeriodicRTCUpdate =  cpu_to_be16(simple_strtoul(argv[i+1], NULL, 16));
			i += 2;
		} else if (!strncmp(argv[i], "LU", 2) &&
			   strlen(argv[i]) == 3 &&
			   argv[i][2] >= '0' && argv[i][2] <= '7') {
			printf("\n%s\n", argv[i]);
			pidx = argv[i][2] - '0';
			i++;
			punit = (struct ufs_unit_descr *)&desc_buf[bUD0BaseOffset + bUDConfigPLength*pidx];
			/* set default value */
			punit->bDataReliability = 1;
			punit->bLogicalBlockSize = 0xC;
			punit->bProvisioningType = 3;
			r = parse_hwpart_unit(punit, qAllocUnitSize, &qAvailNumAllocUnits, argc-i, &argv[i]);
			if (r < 0)
				return CMD_RET_USAGE;
			i += r;
		} else {
			return CMD_RET_USAGE;
		}

		printf("%s %s\n", argv[i-2],  argv[i-1]);
	}

	if(ufshcd_map_desc_id_to_length(hba, QUERY_DESC_IDN_CONFIGURATION, &buff_len)) {
		printf("Failed to get full descriptor length");
		return CMD_RET_FAILURE;
	}

	pconf->bLength = buff_len;
	pconf->bDescriptorDN = QUERY_DESC_IDN_CONFIGURATION;
	pconf->bConfDescContinue = 0;

	#if 1
	print_hex_dump("config desc ", DUMP_PREFIX_OFFSET, 16, 1, desc_buf,
		buff_len, false);
	#endif

	if(!ufshcd_write_configuration_desc(hba, 0, (u8 *)desc_buf, buff_len)) {
		dBootLunEn = 1;
		err = ufshcd_query_attr(hba, UPIU_QUERY_OPCODE_WRITE_ATTR, QUERY_ATTR_IDN_BOOT_LU_EN, 0, 0, &dBootLunEn);
		if (err) {
			printf("Failed configuring bBootLunEn = %d err = %d\n", dBootLunEn , err);
			puts("Failed!\n");
			return CMD_RET_FAILURE;
		}

		puts("Partitioning successful, "
		     "power-cycle to make effective\n");
		return CMD_RET_SUCCESS;
	} else {
		puts("Failed!\n");
		return CMD_RET_FAILURE;
	}
}

static int do_ufs_rw_attr(int flag, int argc, char * const argv[])
{
	int idn;
	u32 attr_val;
	struct ufs_hba *hba = find_ufs_device(ufs_curr_target);

	idn = simple_strtoul(argv[1], NULL, 16);

	if(strncmp(argv[0], "read", 4) == 0) {
		if (argc != 2)
			return CMD_RET_USAGE;

		if (ufs_read_attr(hba, idn, &attr_val))
			return CMD_RET_FAILURE;

		printf("attr %Xh:%Xh\n", idn, attr_val);
	}
	else {
		if (argc != 3)
			return CMD_RET_USAGE;

		attr_val = simple_strtoul(argv[2], NULL, 16);
		if (ufs_write_attr(hba, idn, attr_val))
			return CMD_RET_FAILURE;
	}

	return CMD_RET_SUCCESS;
}

static int do_ufs_rw_flag(int flag, int argc, char * const argv[])
{
	int idn;
	u32 flag_res;
	struct ufs_hba *hba = find_ufs_device(ufs_curr_target);

	idn = simple_strtoul(argv[1], NULL, 16);

	if(strncmp(argv[0], "read", 4) == 0) {
		if (argc != 2)
			return CMD_RET_USAGE;

		if (ufs_read_flag(hba, idn, &flag_res))
			return CMD_RET_FAILURE;

		printf("flag %Xh:%Xh\n", idn, flag_res);
	}
	else {
		if (argc != 3)
			return CMD_RET_USAGE;

		flag_res = simple_strtoul(argv[2], NULL, 16);
		if (ufs_write_flag(hba, idn, flag_res))
			return CMD_RET_FAILURE;
	}

	return CMD_RET_SUCCESS;
}

static int do_ufs_rw_desc(int flag, int argc, char * const argv[])
{
	int desc_id, desc_index;
	u8 *param_buf;
	u8 param_size;
	struct ufs_hba *hba = find_ufs_device(ufs_curr_target);

	if (argc != 5)
		return CMD_RET_USAGE;

	desc_id = simple_strtoul(argv[1], NULL, 16);
	desc_index = simple_strtoul(argv[2], NULL, 16);
	param_buf = (u8 *)simple_strtoul(argv[3], NULL, 16);
	param_size = simple_strtoul(argv[4], NULL, 16);

	if(strncmp(argv[0], "read", 4) == 0) {
		if (ufs_read_desc(hba, desc_id, desc_index, param_buf, param_size))
			return CMD_RET_FAILURE;
	}
	else {
		if (ufs_write_desc(hba, desc_id, desc_index, param_buf, param_size))
			return CMD_RET_FAILURE;
	}

	return CMD_RET_SUCCESS;
}

static int do_ufs_capacity(int flag, int argc, char * const argv[])
{
	struct blk_desc *dev_desc = NULL;
	u32 *addr;

	addr = (u32 *)simple_strtoul(argv[1], NULL, 16);

	dev_desc = blk_get_dev("ufs", ufs_curr_dev);
	if (dev_desc == NULL) {
		printf("Block device %s %d not supported\n", "ufs", ufs_curr_dev);
		return CMD_RET_FAILURE;
	}

	*addr = (u32)dev_desc->lba;

	return CMD_RET_SUCCESS;
}

/*
 * ufs command intepreter
 */
static int do_ufs(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	int ret;
	int tar;
	char *cmd;

	cmd = argv[1];

	/* at least two arguments please */
	if (argc < 2)
		return CMD_RET_USAGE;

	if (strcmp(argv[1], "scan") == 0) {
		bool verbose = true;

		if (argc == 3)
			if (simple_strtoul(argv[2], NULL, 10) == 0)
				verbose = false;

		ret = ufs_scan(verbose);
		if (ret)
			return CMD_RET_FAILURE;

		ufs_curr_dev = 0;
		ufs_curr_target = 0;

		return ret;
	}
	else if (ufs_curr_dev < 0) {
		if (get_ufs_num() > 0) {
			ufs_curr_dev = 0;
			ufs_curr_target = 0;
		} else {
			puts("No UFS device available\n");
			return CMD_RET_FAILURE;
		}
	}

	if (strcmp(argv[1], "target") == 0) {
		if (argc < 3) {
			putc('\n');
			if (ufs_curr_target < 0)
				puts("no target available\n");
			else
				printf("Target %d\n", ufs_curr_target);
			return 0;
		}

		tar = (int)simple_strtoul(argv[2], NULL, 10);
		printf("Target %d:", tar);
		puts("... is now current target\n");
		ufs_curr_target = tar;

		return 0;
	}
	else if ((strncmp(argv[1], "read.", 5) == 0)) {
		if (!strcmp(&cmd[4], ".attr"))
			return do_ufs_rw_attr(flag, argc-1, argv+1);
		else if (!strcmp(&cmd[4], ".flag"))
			return do_ufs_rw_flag(flag, argc-1, argv+1);
		else if (!strcmp(&cmd[4], ".desc"))
			return do_ufs_rw_desc(flag, argc-1, argv+1);
		else if (!strcmp(&cmd[4], ".capacity"))
			return do_ufs_capacity(flag, argc-1, argv+1);
		#if 0
		if (!strcmp(&cmd[4], ".p") || !strcmp(&cmd[4], ".p.continue"))
			return do_ufs_rw_part(flag, argc-1, argv+1);
		#endif
		else
			return CMD_RET_USAGE;
	}
	else if ((strncmp(argv[1], "write.", 6) == 0)) {
		if (!strcmp(&cmd[5], ".attr"))
			return do_ufs_rw_attr(flag, argc-1, argv+1);
		else if (!strcmp(&cmd[5], ".flag"))
			return do_ufs_rw_flag(flag, argc-1, argv+1);
		else if (!strcmp(&cmd[5], ".desc"))
			return do_ufs_rw_desc(flag, argc-1, argv+1);
		#if 0
		if (!strcmp(&cmd[5], ".p") || !strcmp(&cmd[5], ".p.continue"))
			return do_ufs_rw_part(flag, argc-1, argv+1);
		#endif
		else
			return CMD_RET_USAGE;
	}
	else if ((strcmp(argv[1], "erase") == 0) /*|| (strcmp(argv[1], "erase.p") == 0)*/) {
		return do_ufs_erase(flag, argc-1, argv+1);
	}
	else if (strcmp(argv[1], "getcid") == 0) {
		return do_ufs_get_cid(flag, argc-1, argv+1);
	}
	#if CONFIG_IS_ENABLED(CMD_UFS_RPMB)
	else if (strcmp(argv[1], "rpmb") == 0) {
		if (argc < 3)
			return CMD_RET_USAGE;
		argc -= 2;
		argv += 2;
		if(strcmp(argv[0], "key") == 0) {
			return do_ufs_rpmb_key(flag, argc, argv);
		}
		else if(strcmp(argv[0], "read") == 0) {
			return do_ufs_rpmb_read(flag, argc, argv);
		}
		else if(strcmp(argv[0], "write") == 0) {
			return do_ufs_rpmb_write(flag, argc, argv);
		}
		else if(strcmp(argv[0], "counter") == 0) {
			return do_ufs_rpmb_counter(flag, argc, argv);
		}

		return CMD_RET_USAGE;
	}
	#endif
	else if (strcmp(argv[1], "hwpartition") == 0) {
		return do_ufs_hwpartition(flag, argc, argv);
	}
	else if (strcmp(argv[1], "crypto") == 0) {
		struct ufs_hba *hba = find_ufs_device(ufs_curr_target);

		ret = CMD_RET_SUCCESS;
		if (!strcmp(argv[2], "enable"))
			ufs_crypto_enable(hba);
		else if (!strcmp(argv[2], "disable"))
			ufs_crypto_disable(hba);
		else
			ret = CMD_RET_USAGE;

		if (ret == CMD_RET_SUCCESS)
			blkcache_invalidate(IF_TYPE_UFS, 0);

		return ret;
	}

	return blk_common_cmd(argc, argv, IF_TYPE_UFS, &ufs_curr_dev);
}

U_BOOT_CMD(
	ufs, 80, 1, do_ufs,
	"UFS sub-system",
	"ufs info  - show available UFS devices(LUN)\n"
	"ufs scan  - (re-)scan UFS bus\n"
	"ufs device [dev] - show or set current device(LUN)\n"
	"ufs target [tar] - show or set current target\n"
	"ufs part [dev] - print partition table of one or all UFS devices(LUN)\n"
	"ufs read addr blk# cnt - read `cnt' blocks starting at block `blk#'\n"
	"     to memory address `addr'\n"
	"ufs write addr blk# cnt - write `cnt' blocks starting at block\n"
	"     `blk#' from memory address `addr'"
	"ufs erase blk# cnt\n"
	"ufs read.capacity addr - read capacity(block cnt) to memory address `addr'\n"
	#if 0
	"ufs read.p addr partition_name cnt\n"
	"ufs write.p addr partition_name cnt\n"
	"ufs read.p.continue addr partition_name offset_blk# cnt\n"
	"ufs write.p.continue addr partition_name offset_blk# cnt\n"
	"ufs erase.p partition_name\n"
	#endif
	"ufs read.attr idn\n"
	"ufs write.attr idn val\n"
	"ufs read.flag idn\n"
	"ufs write.flag idn val\n"
	"ufs read.desc idn index addr size\n"
	"ufs write.desc idn index addr size\n"
	"ufs getcid addr\n"
	#if CONFIG_IS_ENABLED(CMD_UFS_RPMB)
	"ufs rpmb read addr blk# cnt [address of auth-key] - block size is 256 bytes\n"
	"ufs rpmb write addr blk# cnt <address of auth-key> - block size is 256 bytes\n"
	"ufs rpmb key <address of auth-key> - program the RPMB authentication key.\n"
	"ufs rpmb counter - read the value of the write counter\n"
	#endif
	"ufs hwpartition [args...] - does hardware partitioning\n"
	"  arguments (sizes in 4K-byte blocks):\n"
	"    [BootEn val] - Boot Enable\n"
	"    [DescrAccessEn val] - Descriptor Access Enable\n"
	"    [InitPowerMode val] - Initial Power Mode\n"
	"    [HighPriorityLUN val] - High Priority LUN\n"
	"    [SecureRemovalType val] - Secure Removal Type\n"
	"    [InitActiveICCLevel val] - Initial Active ICC Level\n"
	"    [PeriodicRTCUpdate val] - Initial Active ICC Level\n"
	"    [LU0|LU1|LU2|LU3|LU4|LU5|LU6|LU7 - Logical Unit partition\n"
	"        Size - in byte\n"
	"        [BootLunID val] - Boot LUN ID\n"
	"        [LUWriteProtect val] - Logical Unit Write Protect\n"
	"        [MemoryType val] - Memory Type\n"
	"        [DataReliability val] - Data Reliability\n"
	"        [LogicalBlockSize val] - Logical Block Size\n"
	"        [ProvisioningType val] - Provisioning Type\n"
	"        [ContextCapabilities val] - Context Capabilities]\n"
	"  WARNING:Power cycling is required to initialize partitions after set to complete.\n"
	"ufs crypto enable|disable - enable or disable crypto function\n"
);

