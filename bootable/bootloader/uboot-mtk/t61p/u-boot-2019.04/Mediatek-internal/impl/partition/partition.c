// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#include <common.h>
#include <command.h>
#include <console.h>
#include <malloc.h>
#include <debug_impl.h>
#include <MsTypes.h>
#include <mmc.h>
#include <partition.h>
#include <sparse_format.h>
#include <image-sparse.h>
#include <utility.h>
#include <mtk_ab.h>
#include <romtblo_impl.h>
#include <blk.h>
#include <part.h>
#include <amzn_tv_secure_boot.h>

//-------------------------------------------------------------------------------------------------
// One-PKG definition
//-------------------------------------------------------------------------------------------------
/* one-pkg must have magic ID at first 4 bytes in header, MOPF means Mtk One Packet Format */
static const char ONE_PKG_MAGIC_ID[5] = "MOPF";

#if (CONFIG_AB_SIDELOAD == 1)
static int _convert_ab_name(char *target_name)
{
	char a_char[3] = "_a";
	char b_char[3] = "_b";

	/* If the system is boot from a */
	if (get_ab_selection_result() == PARTITION_SELECT_A)
	{
		UBOOT_DEBUG("ab selection result is 'a'\n");
		if (strnlen(a_char,sizeof(a_char)-1) + strnlen(target_name,PART_NAME_LEN) >= PART_NAME_LEN)  // to avoid the partition exceed max length (32)
		{
			UBOOT_ERROR("[ERROR] After add suffix '%s' partition %s length exceed %d\n",a_char,target_name,PART_NAME_LEN);
			return CMD_RET_FAILURE;
		}
		else
		{
			strncat(target_name, a_char, PART_NAME_LEN-1);      // put the suffix(_a) after the original name
			UBOOT_DEBUG("After converted, new partition name:%s\n", target_name);
			return CMD_RET_SUCCESS;
		}
	}

	/* If the system is boot from b */
	else if (get_ab_selection_result() == PARTITION_SELECT_B)
	{
		UBOOT_DEBUG("ab selection result is 'b'\n");
		if (strnlen(b_char,sizeof(a_char)-1) + strnlen(target_name,PART_NAME_LEN) >= PART_NAME_LEN)  // to avoid the partition exceed max length (32)
		{
			UBOOT_ERROR("[ERROR] After add suffix '%s' partition %s length exceed %d\n",b_char,target_name,PART_NAME_LEN);
			return CMD_RET_FAILURE;
		}
		else
		{
			strncat(target_name, b_char, PART_NAME_LEN-1);      // put the suffix(_b) after the original name
			UBOOT_DEBUG("After converted, new partition name:%s\n", target_name);
			return CMD_RET_SUCCESS;
		}
	}

	/* If the system is not boot from a or b */
	else
	{
		UBOOT_ERROR("[ERROR] Get ab selection fail, get_ab_selection_result return %d\n",get_ab_selection_result());
		return CMD_RET_FAILURE;
	}

	UBOOT_ERROR("[ERROR] Convert ab name occured unknown error\n");
	return CMD_RET_FAILURE;
}
#endif

static int _get_aisc_complete_name(char *asic_complete_name, unsigned int asic_complete_name_len)
{
	int snprintf_len;
	unsigned int chip_id = 0;
	unsigned int chip_revision = 0;

	/* Clean input array */
	memset(asic_complete_name, 0, asic_complete_name_len);

	/* Get ASIC chip id from ROM table */
	chip_id = romtbl_get_chip_id_info_dec();
	if(chip_id == 0)
	{
		UBOOT_ERROR("ASIC chip id get failure, Please check %x address\n",CONFIG_ROM_TABLE_ADDRESS);
		return ERR_INVALID_ASIC_ID;
	}
	else
		UBOOT_DEBUG("ASIC chip id: MT%d\n",chip_id);

	/* Get ASIC chip revision from ROM table */
	chip_revision = romtbl_get_chip_revision_info();
	if(chip_revision == 0)
	{
		UBOOT_ERROR("ASIC chip revision get failure, Please check %x address\n",CONFIG_ROM_TABLE_ADDRESS);
		return ERR_INVALID_ASIC_REVISION;
	}
	else
		UBOOT_DEBUG("ASIC chip revision: E%d\n",chip_revision);

	/* Check special case or not */
	if (chip_id == ASIC_MT5896_CHIP_ID && chip_revision >= ASIC_CHIP_REVISION_3)
	{
		/*  Special case 1 : MT5896 after E3 will use special ASIC complete name MT5896E3 */
		snprintf_len = snprintf(asic_complete_name, asic_complete_name_len, "%s%d%s%d", CHIP_ID_CHAR, chip_id, CHIP_REVISION_CHAR, ASIC_CHIP_REVISION_3);
		if (snprintf_len >= asic_complete_name_len)
		{
			UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", (int)asic_complete_name_len, asic_complete_name);
			return ERR_INVALID_ASIC_COMPLETE_NAME;
		}

		UBOOT_DEBUG("Special case! ASIC complete name = %s\n", asic_complete_name);
		return CMD_RET_SUCCESS;
	}
	else
	{
		/* Other cases : Use normal ASIC complete name MTxxxx, xxxx is chip id */
		snprintf_len = snprintf(asic_complete_name, asic_complete_name_len, "%s%d", CHIP_ID_CHAR, chip_id);
		if (snprintf_len >= asic_complete_name_len)
		{
			UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", (int)asic_complete_name_len, asic_complete_name);
			return ERR_INVALID_ASIC_COMPLETE_NAME;
		}

		UBOOT_DEBUG("ASIC complete name = %s\n", asic_complete_name);
		return CMD_RET_SUCCESS;
	}

}

static int _check_is_one_pkg(unsigned long *address, unsigned long *size)
{
	int i;
	int ret = 0;
	one_pkg_main_header* main_header = NULL;
	one_pkg_sub_header* sub_header = NULL;
	char asic_complete_name[SUB_HEADER_NAME_LEN + 1];
	char sub_header_complete_name[SUB_HEADER_NAME_LEN + 1];


	UBOOT_DUMP(*address, 0x50);

	main_header = (one_pkg_main_header*)*address;

	if(memcmp(main_header->magic_id, ONE_PKG_MAGIC_ID, 4) == 0)  /* Find magic ID in header, this PKG is one-PKG */
	{
		UBOOT_DEBUG("This PKG is one-PKG, start to find corresponding PKG.\n");

		UBOOT_DEBUG("main_header->magic_id = %s\n", main_header->magic_id);
		UBOOT_DEBUG("main_header->version  = 0x%x\n", main_header->version);
		UBOOT_DEBUG("main_header->count    = 0x%x\n", main_header->count);

		if(main_header->count > 0)  /* There must be more than 1 pkg in one-pkg */
		{
			/* Get ASIC complete name from ROM table & decide add chip revision or not */
			ret = _get_aisc_complete_name(asic_complete_name, sizeof(asic_complete_name));
			if(ret != CMD_RET_SUCCESS)
			{
				UBOOT_ERROR("[ERROR] Get ASIC complete name fail (%d)\n", ret);
				UBOOT_ERROR("Set write size = 0\n");
				*size = 0;
				return ERR_INVALID_ASIC_COMPLETE_NAME;
			}

			for (i = 0; i < main_header->count; i++)
			{
				/* Get sub_header in one-pkg */
				sub_header = (one_pkg_sub_header*) (*address + sizeof(one_pkg_main_header) + (sizeof(one_pkg_sub_header) * i));

				memset(sub_header_complete_name, 0, sizeof(sub_header_complete_name));
				strncpy((char *)sub_header_complete_name, (const char *)sub_header->name, SUB_HEADER_NAME_LEN);
				sub_header_complete_name[SUB_HEADER_NAME_LEN] = '\0';

				UBOOT_DEBUG("[%d] sub_header->name   = %s\n", i, sub_header_complete_name);
				UBOOT_DEBUG("[%d] sub_header->offset = 0x%x\n", i, sub_header->offset);
				UBOOT_DEBUG("[%d] sub_header->size   = 0x%x\n", i, sub_header->size);
				UBOOT_DEBUG("[%d] sub_header->id     = 0x%x\n", i, sub_header->id);

				/* check if ASIC complete name & sub_header are the same */
				if (strncmp(asic_complete_name, sub_header_complete_name, SUB_HEADER_NAME_LEN) == 0)  /* Find the target pkg */
				{
					UBOOT_DEBUG("ASIC complete name & sub_header name match, choose this PKG.\n");
					*address = *address + sub_header->offset;
					*size = sub_header->size;
					return PASS_IS_ONE_PKG;
				}
				else
					UBOOT_DEBUG("ASIC complete name & sub_header name NOT match, find next sub_header\n");
			}

			/* Can not find mtached sub_header id in sub_header, set write size to 0 */
			UBOOT_ERROR("Not find corresponding pkg in sub_header\n");
			UBOOT_ERROR("Set write size = 0\n");
			*size = 0;
			return ERR_NO_CORR_PKG_IN_SUB_HEAD;
		}
		else  /* There is no pkg in one-pkg */
		{
			UBOOT_ERROR("Invalid main_header->count = 0x%x\n", main_header->count);
			UBOOT_ERROR("Set write size = 0\n");
			*size = 0;
			return ERR_INVALID_PKG_CNT;
		}
	}
	else  /* Not find magic ID in header, this PKG is NOT one-PKG */
	{
		UBOOT_DEBUG("Not find one-pkg magic ID\n");
		return PASS_NOT_ONE_PKG;
	}
}

int get_boot_partition_info(const char *target_name, disk_partition_t *target_info, unsigned int *target_num)
{
	char device_name[STORAGE_DEVICE_BUF_SIZE] = {0};
	unsigned int device_number = 0;        // only considerating about device number is 0. If device number is not 0, please use another API to get partition info.
	struct blk_desc *blk_dev_desc = NULL;


	/* get boot device */
	if(sys_get_boot_device(device_name, sizeof(device_name))==-1)
	{
		UBOOT_ERROR("[ERROR] Get boot device fail !! \n");
		return CMD_RET_FAILURE;
	}


	/* Get information about boot interface */
	/* Set interface device */
	if ((strcmp(device_name, "mmc") == 0) || (strcmp(device_name, "usb") == 0))
	{
		/* Get information about boot interface */
		blk_dev_desc = blk_get_dev(device_name, device_number);                    // parameter of blk_get_dev should input mmc device number
		if (!blk_dev_desc) {
			UBOOT_ERROR("[ERROR] %s dev %u NOT available\n",device_name, device_number);
			return CMD_RET_FAILURE;
		}

		/* Set interface device */  // mark this part first, we only support device number is 1 case
/*		if (device_number != 0)
		{
			int snprintf_len;
			memset(cmd, 0, 128);
			snprintf_len = snprintf(cmd, sizeof(cmd), "%s dev %u",device_name,device_number);
			if (snprintf_len >= sizeof(cmd))
			{
				UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", (int)(sizeof(cmd)), cmd);
				return CMD_RET_FAILURE;
			}
			UBOOT_DEBUG("cmd=%s\n",cmd);
			if(run_command(cmd, 0) != 0)
			{
				UBOOT_ERROR("[ERROR] Set %s dev fail!!!\n",device_name);
				return CMD_RET_FAILURE;
			}
		}*/
	}
	else if (strcmp(device_name, "ufs") == 0)
	{
		/* Check interface device is 0, MTK only allow one UFS flash to exist */   // mark this part first, we only support device number is 1 case
/*		if (device_number != 0)      // UFS switch device # cmd is 'ufs target #', it's different from other flash type
		{
			UBOOT_ERROR("[ERROR] %s only support one target(0) now!!!\n",device_name);
			return CMD_RET_FAILURE;
		}*/

		/* Get information about boot interface */
		blk_dev_desc = blk_get_dev(device_name, 0);               // parameter of blk_get_dev should input UFS LUN(partition) number
		if (!blk_dev_desc) {                                      // MTK only allow one UFS flash to exist, so UFS user partition LUN hard code writes 0
			UBOOT_ERROR("[ERROR] %s LUN 0 NOT available\n",device_name);
			return CMD_RET_FAILURE;
		}
	}
	else
	{
		UBOOT_ERROR("[ERROR] Get blk_dev_desc fail!!!\n");
		return CMD_RET_FAILURE;
	}


	/* Get information about partition */
	if(get_partition_info(blk_dev_desc, target_name, target_info, target_num) != CMD_RET_SUCCESS)
	{
		UBOOT_ERROR("[ERROR] Get boot partition information fail!!!\n");
		return CMD_RET_FAILURE;
	}

	return CMD_RET_SUCCESS;
}

int get_partition_info(struct blk_desc *blk_dev_desc, const char *target_name, disk_partition_t *target_info, unsigned int *target_num)
{
	disk_partition_t info;
	char partition_name[PART_NAME_LEN];
	int part_num;

	memset(partition_name, 0, PART_NAME_LEN*sizeof(uchar));
	strncat(partition_name, target_name, (PART_NAME_LEN-1));     // copy target_name into a array, to aviod memory error when add suffixed after target_name
	if (!blk_dev_desc) {
		UBOOT_ERROR("[ERROR] block description NULL!!!!!!!!!!!!!!!!!\n");
		return CMD_RET_FAILURE;
	}

	/* search original input partition_name info. */
    if (blk_dev_desc->if_type == IF_TYPE_USB) {
        part_num = part_get_info_by_name(blk_dev_desc, partition_name, &info);
    } else {
        part_num = part_info_search(blk_dev_desc, partition_name, &info);
    }

	/* If original input partition_name is not found, search once with the suffixed name */
	if (part_num <= 0)
	{
#if (CONFIG_AB_SIDELOAD == 1)
		UBOOT_DEBUG("Can not find original partition name:%s !!!\n", partition_name);

		/* add suffixed after partition_name */
		if(_convert_ab_name(partition_name) == CMD_RET_SUCCESS)
		{
			UBOOT_DEBUG("Search partition name:%s again\n", partition_name);
		}
		else
		{
			UBOOT_ERROR("[ERROR] Convert ab name fail\n");
			return CMD_RET_FAILURE;
		}

		/* search new partition_name (add suffixed after partition_name) info. */
        if (blk_dev_desc->if_type == IF_TYPE_USB) {
            part_num = part_get_info_by_name(blk_dev_desc, partition_name, &info);
        } else {
            part_num = part_info_search(blk_dev_desc, partition_name, &info);
        }

		if (part_num <= 0)
		{
			UBOOT_ERROR("Can not find corresponding suffixed partition name:%s !!!\n", partition_name);
			return CMD_RET_FAILURE;
		}
#else
		UBOOT_ERROR("Can not find partition name:%s !!!\n", partition_name);
		return CMD_RET_FAILURE;
#endif
	}

	/* check target partition is found or not. */
	if (part_num > 0)
	{
		UBOOT_DEBUG("Find partition name:%s, number:%d, start LBA:0x%lx, size LBA:0x%lx\n", partition_name, part_num, info.start, info.size);
	}
	else
	{
		UBOOT_ERROR("[ERROR] Get partition info. something wrong!!\n");
		return CMD_RET_FAILURE;
	}

	if(target_info)
		*target_info = info;
	if(target_num)
		*target_num = part_num;

	return CMD_RET_SUCCESS;
}

int do_partition_read(const char *str_interface, const char *str_device, const char *str_partition_name, const char *str_address, const char *str_size, const char *str_start_byte_pos)
{
	char cmd[128];
	int snprintf_len;
	unsigned int device = 0;
	unsigned long address = 0;
	unsigned long size = 0;
	unsigned long start_byte_pos = 0;
	unsigned long start_shift_lba_number = 0;
	unsigned long start_shift_byte = 0;
	unsigned long size_lba_number = 0;
	unsigned long size_byte_align = 0;
	struct blk_desc *blk_dev_desc = NULL;
	disk_partition_t partition_info;


	/* Check interface type */
	if (strcmp(str_interface, "mmc") == 0)
		UBOOT_DEBUG("Interface is '%s'\n",str_interface);
	else if (strcmp(str_interface, "usb") == 0)
		UBOOT_DEBUG("Interface is '%s', user should run 'usb start' first.\n",str_interface);
	else if (strcmp(str_interface, "ufs") == 0)
		UBOOT_DEBUG("Interface is '%s'\n",str_interface);
	else
	{
		UBOOT_ERROR("[ERROR] Unrecognized interface, only support 'mmc' & 'usb' & 'ufs' now.\n");
		return CMD_RET_FAILURE;
	}


	/* Convert device number */
	device = simple_strtoul(str_device, NULL, 10);


	/* Check length of partition name not exceed PART_NAME_LEN(32) */
	if(strnlen(str_partition_name,PART_NAME_LEN) >= PART_NAME_LEN)
	{
		UBOOT_ERROR("[ERROR] Partition_name can not exceed max value %d\n",PART_NAME_LEN);
		return CMD_RET_FAILURE;
	}


	/* Convert DRAM address */
	address = simple_strtoul(str_address, NULL, 16);


	/* Convert reading size by hexadecimal */
	size = simple_strtoul(str_size, NULL, 16);


	/* Convert starting reading position by hexadecimal */
	if(str_start_byte_pos != NULL)
		start_byte_pos = simple_strtoul(str_start_byte_pos, NULL, 16);
	else
		start_byte_pos = 0;


	/* Get information about interface, including size of 1 LBA on interface */
	/* Set interface device */
	if ((strcmp(str_interface, "mmc") == 0) || (strcmp(str_interface, "usb") == 0))
	{
		/* Get size of 1 LBA (unit in bytes) */
		blk_dev_desc = blk_get_dev(str_interface, device);                    // parameter of blk_get_dev should input mmc device number
		if (!blk_dev_desc) {
			UBOOT_ERROR("[ERROR] %s dev %u NOT available\n",str_interface, device);
			return CMD_RET_FAILURE;
		}
		UBOOT_DEBUG("Flash type:%s, 1 LBA = 0x%lx bytes\n",str_interface,blk_dev_desc->blksz);

		/* Set interface device */
		if (device != 0)
		{
			memset(cmd, 0, 128);
			snprintf_len = snprintf(cmd, sizeof(cmd), "%s dev %u",str_interface,device);
			if (snprintf_len >= sizeof(cmd))
			{
				UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", (int)(sizeof(cmd)), cmd);
				return CMD_RET_FAILURE;
			}
			UBOOT_DEBUG("cmd=%s\n",cmd);
			if(run_command(cmd, 0) != 0)
			{
				UBOOT_ERROR("[ERROR] Set %s dev fail!!!\n",str_interface);
				return CMD_RET_FAILURE;
			}
		}
	}
	else if (strcmp(str_interface, "ufs") == 0)
	{
		/* Check interface device is 0, MTK only allow one UFS flash to exist */
		if (device != 0)      // UFS switch device # cmd is 'ufs target #', it's different from other flash type
		{
			UBOOT_ERROR("[ERROR] %s only support one target(0) now!!!\n",str_interface);
			return CMD_RET_FAILURE;
		}

		/* Get size of 1 LBA (unit in bytes) */
		blk_dev_desc = blk_get_dev(str_interface, 0);               // parameter of blk_get_dev should input UFS LUN(partition) number
		if (!blk_dev_desc) {                                        // MTK only allow one UFS flash to exist, so UFS user partition LUN hard code writes 0
			UBOOT_ERROR("[ERROR] %s LUN 0 NOT available\n",str_interface);
			return CMD_RET_FAILURE;
		}
		UBOOT_DEBUG("Flash type:%s, 1 LBA = 0x%lx bytes\n",str_interface,blk_dev_desc->blksz);
	}
	else
	{
		UBOOT_ERROR("[ERROR] Get %s size of 1 LBA fail!!!\n",str_interface);
		return CMD_RET_FAILURE;
	}


	/* Get the partition information, including start LBA & size LBA */
	if(get_partition_info(blk_dev_desc, str_partition_name, &partition_info, NULL) != CMD_RET_SUCCESS)
		return CMD_RET_FAILURE;


	/* Confirm that the read size does not exceed end of this partition */
	if(size + start_byte_pos > (partition_info.size*blk_dev_desc->blksz))
	{
		UBOOT_ERROR("[ERROR] Reading size can not exceed end of partition LBA (0x%lx LBA)!!!\n",(partition_info.start + partition_info.size));
		return CMD_RET_FAILURE;
	}


	/* Check whether the starting reading position is aligned 1 LBA (ex: emmc is 512 bytes)  */
	start_shift_lba_number = start_byte_pos / blk_dev_desc->blksz;
	start_shift_byte = start_byte_pos % blk_dev_desc->blksz;
	UBOOT_DEBUG("Start reading position will shift 0x%lx bytes (0x%lx LBA + 0x%lx bytes)\n", start_byte_pos, start_shift_lba_number, start_shift_byte);


	/* Start reading */
	char byte_align_buffer[blk_dev_desc->blksz];

	memset(byte_align_buffer, 0, blk_dev_desc->blksz);
	UBOOT_DEBUG("byte_align_buffer address 0x%p\n",byte_align_buffer);

	if(start_shift_byte == 0)     /* If reading position only needs to shift integrated LBA */
	{
		/* Check if read size is aligned 1 LBA (ex: emmc is 512 bytes)  */
		size_lba_number = size / blk_dev_desc->blksz;
		size_byte_align = size % blk_dev_desc->blksz;
		UBOOT_DEBUG("Read size = 0x%lx (0x%lx LBA + 0x%lx bytes)\n", size, size_lba_number, size_byte_align);


		/* Read integer LBA partition data to DRAM first */
		if (size_lba_number != 0)
		{
			memset(cmd, 0, 128);
			snprintf_len = snprintf(cmd, sizeof(cmd), "%s read 0x%lx 0x%lx 0x%lx",str_interface, address, (partition_info.start + start_shift_lba_number), size_lba_number);
			if (snprintf_len >= sizeof(cmd))
			{
				UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", (int)(sizeof(cmd)), cmd);
				return CMD_RET_FAILURE;
			}
			UBOOT_DEBUG("cmd=%s\n",cmd);
			if(run_command(cmd, 0) != 0)
			{
				UBOOT_ERROR("[ERROR] Read integer LBA partition data fail!!!\n");
				return CMD_RET_FAILURE;
			}
		}


		/* Read the remaining less than 1 LBA data from interface to DRAM */
		if (size_byte_align != 0)
		{
			/* Read the data in last incomplete LBA from interface to byte_align_buffer */
			memset(cmd, 0, 128);
			snprintf_len = snprintf(cmd, sizeof(cmd), "%s read 0x%p 0x%lx 0x1",str_interface, byte_align_buffer, (partition_info.start + start_shift_lba_number + size_lba_number));
			if (snprintf_len >= sizeof(cmd))
			{
				UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", (int)(sizeof(cmd)), cmd);
				return CMD_RET_FAILURE;
			}
			UBOOT_DEBUG("cmd=%s\n",cmd);
			if(run_command(cmd, 0) != 0)
			{
				UBOOT_ERROR("[ERROR] Read last align bytes partition data fail!!!\n");
				return CMD_RET_FAILURE;
			}

			/* Only copy the remaining bytes of data from byte_align_buffer to str_address */
			memcpy((void *)(address + size_lba_number * blk_dev_desc->blksz), byte_align_buffer, size_byte_align);
			UBOOT_DEBUG("Copy 0x%lx bytes from 0x%p to 0x%lx\n",size_byte_align, byte_align_buffer, (address + size_lba_number * blk_dev_desc->blksz));
		}
	}

	else                          /* If reading position needs to shift integrated LBA + aligned bytes */
	{
		unsigned long first_lba_read_byte = blk_dev_desc->blksz - start_shift_byte;

		if(size <= first_lba_read_byte)     // it means that all data you want to read is included in first_lba_read_byte
			first_lba_read_byte = size;     // so, update first_lba_read_byte to size

		/* Read the data in first incomplete LBA from interface to DRAM */
		memset(cmd, 0, 128);
		snprintf_len = snprintf(cmd, sizeof(cmd), "%s read 0x%p 0x%lx 0x1",str_interface, byte_align_buffer, (partition_info.start + start_shift_lba_number));
		if (snprintf_len >= sizeof(cmd))
		{
			UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", (int)(sizeof(cmd)), cmd);
			return CMD_RET_FAILURE;
		}
		UBOOT_DEBUG("cmd=%s\n",cmd);
		if(run_command(cmd, 0) != 0)
		{
			UBOOT_ERROR("[ERROR] Read first align bytes partition data fail!!!\n");
			return CMD_RET_FAILURE;
		}


		/* Only copy first_lba_read_byte bytes of data from byte_align_buffer[start_shift_byte] to str_address */
		memcpy((void *)address, (byte_align_buffer + start_shift_byte), first_lba_read_byte);
		UBOOT_DEBUG("Copy 0x%lx bytes from 0x%lx to 0x%lx\n", first_lba_read_byte, ((unsigned long)byte_align_buffer + start_shift_byte), address);


		/* Update remaining reading bytes */
		size = size - first_lba_read_byte;


		/* Check if remaining read size is aligned 1 LBA (ex: emmc is 512 bytes)  */
		size_lba_number = size / blk_dev_desc->blksz;
		size_byte_align = size % blk_dev_desc->blksz;
		UBOOT_DEBUG("Remaining read size = 0x%lx (0x%lx LBA + 0x%lx bytes)\n", size, size_lba_number, size_byte_align);


		/* Read integer LBA partition data to DRAM first */
		if (size_lba_number != 0)
		{
			memset(cmd, 0, 128);
			snprintf_len = snprintf(cmd, sizeof(cmd), "%s read 0x%lx 0x%lx 0x%lx",str_interface, (address + first_lba_read_byte), (partition_info.start + start_shift_lba_number + 1), size_lba_number);
			if (snprintf_len >= sizeof(cmd))
			{
				UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", (int)(sizeof(cmd)), cmd);
				return CMD_RET_FAILURE;
			}
			UBOOT_DEBUG("cmd=%s\n",cmd);
			if(run_command(cmd, 0) != 0)
			{
				UBOOT_ERROR("[ERROR] Read integer LBA partition data fail!!!\n");
				return CMD_RET_FAILURE;
			}
		}


		/* Read the remaining less than 1 LBA data from interface to DRAM */
		if (size_byte_align != 0)
		{
			/* Read the data in last incomplete LBA from interface to byte_align_buffer */
			memset(byte_align_buffer, 0, blk_dev_desc->blksz);
			memset(cmd, 0, 128);
			snprintf_len = snprintf(cmd, sizeof(cmd), "%s read 0x%p 0x%lx 0x1",str_interface, byte_align_buffer, (partition_info.start + start_shift_lba_number + 1 + size_lba_number));
			if (snprintf_len >= sizeof(cmd))
			{
				UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", (int)(sizeof(cmd)), cmd);
				return CMD_RET_FAILURE;
			}
			UBOOT_DEBUG("cmd=%s\n",cmd);
			if(run_command(cmd, 0) != 0)
			{
				UBOOT_ERROR("[ERROR] Read last align bytes partition data fail!!!\n");
				return CMD_RET_FAILURE;
			}

			/* Only copy the remaining bytes of data from byte_align_buffer to (str_address+first_lba_read_byte+size_lba_number*blk_dev_desc->blksz) */
			memcpy((void *)(address + first_lba_read_byte + size_lba_number * blk_dev_desc->blksz), byte_align_buffer, size_byte_align);
			UBOOT_DEBUG("Copy 0x%lx bytes from 0x%p to 0x%lx\n",size_byte_align, byte_align_buffer, (address + first_lba_read_byte + size_lba_number * blk_dev_desc->blksz));
		}
	}

	return CMD_RET_SUCCESS;
}

int do_partition_read_boot(const char *str_interface, const char *str_device, const char *str_partition_number, const char *str_address, const char *str_size, const char *str_start_byte_pos)
{
	char cmd[128];
	int snprintf_len;
	unsigned int device = 0;
	unsigned int partition = 0;
	unsigned long address = 0;
	unsigned long size = 0;
	unsigned long boot_partition_size_lba = 0;
	unsigned long start_byte_pos = 0;
	unsigned long start_shift_lba_number = 0;
	unsigned long start_shift_byte = 0;
	unsigned long size_lba_number = 0;
	unsigned long size_byte_align = 0;
	struct blk_desc *blk_dev_desc = NULL;


	/* Check interface type */
	if (strcmp(str_interface, "mmc") == 0)
		UBOOT_DEBUG("Interface is '%s'\n",str_interface);
	else if (strcmp(str_interface, "ufs") == 0)
		UBOOT_DEBUG("Interface is '%s'\n",str_interface);
	else
	{
		UBOOT_ERROR("[ERROR] Unrecognized interface, only support 'mmc' & 'ufs' now.\n");
		return CMD_RET_FAILURE;
	}


	/* Convert device number */
	device = simple_strtoul(str_device, NULL, 10);


	/* Convert partition number */
	partition = simple_strtoul(str_partition_number, NULL, 10);


	/* Convert DRAM address */
	address = simple_strtoul(str_address, NULL, 16);


	/* Convert reading size by hexadecimal */
	size = simple_strtoul(str_size, NULL, 16);


	/* Convert starting reading position by hexadecimal */
	if(str_start_byte_pos != NULL)
		start_byte_pos = simple_strtoul(str_start_byte_pos, NULL, 16);
	else
		start_byte_pos = 0;


	/* Get information about interface, including size of 1 LBA & boot partition on interface */
	if (strcmp(str_interface, "mmc") == 0)
	{
#ifdef CONFIG_MMC
		/* Get size of 1 LBA (unit in bytes) */
		blk_dev_desc = blk_get_dev(str_interface, device);                    // parameter of blk_get_dev should input mmc device number
		if (!blk_dev_desc) {
			UBOOT_ERROR("[ERROR] %s dev %u NOT available\n",str_interface, device);
			return CMD_RET_FAILURE;
		}
		UBOOT_DEBUG("Flash type:%s, 1 LBA = 0x%lx bytes\n",str_interface,blk_dev_desc->blksz);

		/* Get boot partition capacity (unit in bytes) */
		struct mmc *mmc;
		mmc = find_mmc_device(device);
		if (!mmc) {
			UBOOT_ERROR("[ERROR] no mmc device at slot %x\n", device);
			return CMD_RET_FAILURE;
		}
		boot_partition_size_lba = mmc->capacity_boot / blk_dev_desc->blksz;   // transfer unit of boot partition capacity from bytes to LBA

		/* Set interface device & partition */
		/* blk_get_dev will set interface-partition to default 0, so change interface-partition after blk_get_dev  */
		memset(cmd, 0, 128);
		snprintf_len = snprintf(cmd, sizeof(cmd), "%s dev %u %u",str_interface,device,partition);
		if (snprintf_len >= sizeof(cmd))
		{
			UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", (int)(sizeof(cmd)), cmd);
			return CMD_RET_FAILURE;
		}
		UBOOT_DEBUG("cmd=%s\n",cmd);
		if(run_command(cmd, 0) != 0)
		{
			UBOOT_ERROR("[ERROR] Set %s dev fail!!!\n",str_interface);
			return CMD_RET_FAILURE;
		}
#else
        UBOOT_ERROR("[ERROR] mmc config is not enable\n");
#endif
	}
	else if (strcmp(str_interface, "ufs") == 0)
	{
		/* Check interface device is 0 */
		if (device != 0)      // UFS switch device # cmd is 'ufs target #', it's different from other flash type
		{
			UBOOT_ERROR("[ERROR] %s only support one target(0) now!!!\n",str_interface);
			return CMD_RET_FAILURE;
		}

		/* Get size of 1 LBA (unit in bytes) */
		blk_dev_desc = blk_get_dev(str_interface, partition);               // parameter of blk_get_dev should input UFS LUN(partition) number
		if (!blk_dev_desc) {
			UBOOT_ERROR("[ERROR] %s LUN %u NOT available\n",str_interface, partition);
			return CMD_RET_FAILURE;
		}
		UBOOT_DEBUG("Flash type:%s, 1 LBA = 0x%lx bytes\n",str_interface,blk_dev_desc->blksz);

		/* Set interface partition */
		memset(cmd, 0, 128);
		snprintf_len = snprintf(cmd, sizeof(cmd), "%s device %u",str_interface,partition);   // UFS switch partition # cmd is 'ufs device #', it's different from other flash type
		if (snprintf_len >= sizeof(cmd))
		{
			UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", (int)(sizeof(cmd)), cmd);
			return CMD_RET_FAILURE;
		}
		UBOOT_DEBUG("cmd=%s\n",cmd);
		if(run_command(cmd, 0) != 0)
		{
			UBOOT_ERROR("[ERROR] Set %s dev fail!!!\n",str_interface);
			return CMD_RET_FAILURE;
		}

		/* Get boot partition capacity (unit in LBA) */
		memset(cmd, 0, 128);
		snprintf_len = snprintf(cmd, sizeof(cmd), "ufs read.capacity 0x%p",&boot_partition_size_lba);
		if (snprintf_len >= sizeof(cmd))
		{
			UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", (int)(sizeof(cmd)), cmd);
			return CMD_RET_FAILURE;
		}
		UBOOT_DEBUG("cmd=%s\n",cmd);
		if(run_command(cmd, 0) != 0)
		{
			UBOOT_ERROR("[ERROR] ufs read.capacity fail!!!\n");
			return CMD_RET_FAILURE;
		}
	}
	else
	{
		UBOOT_ERROR("[ERROR] Get %s size of 1 LBA fail!!!\n",str_interface);
		return CMD_RET_FAILURE;
	}

	if(boot_partition_size_lba != 0)
		UBOOT_DEBUG("dev # %u, partition # %u, total LBA # %u\n",device, partition, (unsigned int)boot_partition_size_lba);
	else
	{
		UBOOT_ERROR("[ERROR] Get %s boot partition capacity fail!!!\n",str_interface);
		return CMD_RET_FAILURE;
	}


	/* Confirm that the read size does not exceed end of this boot partition */
	if(size + start_byte_pos > boot_partition_size_lba * blk_dev_desc->blksz)
	{
		UBOOT_ERROR("[ERROR] Reading size can not exceed end of boot partition LBA (0x%lx LBA)!!!\n",boot_partition_size_lba);
		return CMD_RET_FAILURE;
	}


	/* Check whether the starting reading position is aligned 1 LBA (ex: emmc is 512 bytes)  */
	start_shift_lba_number = start_byte_pos / blk_dev_desc->blksz;
	start_shift_byte = start_byte_pos % blk_dev_desc->blksz;
	UBOOT_DEBUG("Start reading position will shift 0x%lx bytes (0x%lx LBA + 0x%lx bytes)\n", start_byte_pos, start_shift_lba_number, start_shift_byte);


	/* Start reading */
	char byte_align_buffer[blk_dev_desc->blksz];

	memset(byte_align_buffer, 0, blk_dev_desc->blksz);
	UBOOT_DEBUG("byte_align_buffer address 0x%p\n",byte_align_buffer);

	if(start_shift_byte == 0)     /* If reading position only needs to shift integrated LBA */
	{
		/* Check if read size is aligned 1 LBA (ex: emmc is 512 bytes)  */
		size_lba_number = size / blk_dev_desc->blksz;
		size_byte_align = size % blk_dev_desc->blksz;
		UBOOT_DEBUG("Read size = 0x%lx (0x%lx LBA + 0x%lx bytes)\n", size, size_lba_number, size_byte_align);


		/* Read integer LBA partition data to DRAM first */
		if (size_lba_number != 0)
		{
			memset(cmd, 0, 128);
			snprintf_len = snprintf(cmd, sizeof(cmd), "%s read 0x%lx 0x%lx 0x%lx",str_interface, address, start_shift_lba_number, size_lba_number);
			if (snprintf_len >= sizeof(cmd))
			{
				UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", (int)(sizeof(cmd)), cmd);
				return CMD_RET_FAILURE;
			}
			UBOOT_DEBUG("cmd=%s\n",cmd);
			if(run_command(cmd, 0) != 0)
			{
				UBOOT_ERROR("[ERROR] Read integer LBA partition data fail!!!\n");
				return CMD_RET_FAILURE;
			}
		}


		/* Read the remaining less than 1 LBA data from interface to DRAM */
		if (size_byte_align != 0)
		{
			/* Read the data in last incomplete LBA from interface to byte_align_buffer */
			memset(cmd, 0, 128);
			snprintf_len = snprintf(cmd, sizeof(cmd), "%s read 0x%p 0x%lx 0x1",str_interface, byte_align_buffer, (start_shift_lba_number + size_lba_number));
			if (snprintf_len >= sizeof(cmd))
			{
				UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", (int)(sizeof(cmd)), cmd);
				return CMD_RET_FAILURE;
			}
			UBOOT_DEBUG("cmd=%s\n",cmd);
			if(run_command(cmd, 0) != 0)
			{
				UBOOT_ERROR("[ERROR] Read last align bytes partition data fail!!!\n");
				return CMD_RET_FAILURE;
			}

			/* Only copy the remaining bytes of data from byte_align_buffer to str_address */
			memcpy((void *)(address + size_lba_number * blk_dev_desc->blksz), byte_align_buffer, size_byte_align);
			UBOOT_DEBUG("Copy 0x%lx bytes from 0x%p to 0x%lx\n",size_byte_align, byte_align_buffer, (address + size_lba_number * blk_dev_desc->blksz));
		}
	}

	else                          /* If reading position needs to shift integrated LBA + aligned bytes */
	{
		unsigned long first_lba_read_byte = blk_dev_desc->blksz - start_shift_byte;

		if(size <= first_lba_read_byte)     // it means that all data you want to read is included in first_lba_read_byte
			first_lba_read_byte = size;     // so, update first_lba_read_byte to size

		/* Read the data in first incomplete LBA from interface to DRAM */
		memset(cmd, 0, 128);
		snprintf_len = snprintf(cmd, sizeof(cmd), "%s read 0x%p 0x%lx 0x1",str_interface, byte_align_buffer, start_shift_lba_number);
		if (snprintf_len >= sizeof(cmd))
		{
			UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", (int)(sizeof(cmd)), cmd);
			return CMD_RET_FAILURE;
		}
		UBOOT_DEBUG("cmd=%s\n",cmd);
		if(run_command(cmd, 0) != 0)
		{
			UBOOT_ERROR("[ERROR] Read first align bytes partition data fail!!!\n");
			return CMD_RET_FAILURE;
		}


		/* Only copy first_lba_read_byte bytes of data from byte_align_buffer[start_shift_byte] to str_address */
		memcpy((void *)address, (byte_align_buffer + start_shift_byte), first_lba_read_byte);
		UBOOT_DEBUG("Copy 0x%lx bytes from 0x%lx to 0x%lx\n", first_lba_read_byte, ((unsigned long)byte_align_buffer + start_shift_byte), address);


		/* Update remaining reading bytes */
		size = size - first_lba_read_byte;


		/* Check if remaining read size is aligned 1 LBA (ex: emmc is 512 bytes)  */
		size_lba_number = size / blk_dev_desc->blksz;
		size_byte_align = size % blk_dev_desc->blksz;
		UBOOT_DEBUG("Remaining read size = 0x%lx (0x%lx LBA + 0x%lx bytes)\n", size, size_lba_number, size_byte_align);


		/* Read integer LBA partition data to DRAM first */
		if (size_lba_number != 0)
		{
			memset(cmd, 0, 128);
			snprintf_len = snprintf(cmd, sizeof(cmd), "%s read 0x%lx 0x%lx 0x%lx",str_interface, (address + first_lba_read_byte), (start_shift_lba_number + 1), size_lba_number);
			if (snprintf_len >= sizeof(cmd))
			{
				UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", (int)(sizeof(cmd)), cmd);
				return CMD_RET_FAILURE;
			}
			UBOOT_DEBUG("cmd=%s\n",cmd);
			if(run_command(cmd, 0) != 0)
			{
				UBOOT_ERROR("[ERROR] Read integer LBA partition data fail!!!\n");
				return CMD_RET_FAILURE;
			}
		}


		/* Read the remaining less than 1 LBA data from interface to DRAM */
		if (size_byte_align != 0)
		{
			/* Read the data in last incomplete LBA from interface to byte_align_buffer */
			memset(byte_align_buffer, 0, blk_dev_desc->blksz);
			memset(cmd, 0, 128);
			snprintf_len = snprintf(cmd, sizeof(cmd), "%s read 0x%p 0x%lx 0x1",str_interface, byte_align_buffer, (start_shift_lba_number + 1 + size_lba_number));
			if (snprintf_len >= sizeof(cmd))
			{
				UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", (int)(sizeof(cmd)), cmd);
				return CMD_RET_FAILURE;
			}
			UBOOT_DEBUG("cmd=%s\n",cmd);
			if(run_command(cmd, 0) != 0)
			{
				UBOOT_ERROR("[ERROR] Read last align bytes partition data fail!!!\n");
				return CMD_RET_FAILURE;
			}

			/* Only copy the remaining bytes of data from byte_align_buffer to (str_address+first_lba_read_byte+size_lba_number*blk_dev_desc->blksz) */
			memcpy((void *)(address + first_lba_read_byte + size_lba_number * blk_dev_desc->blksz), byte_align_buffer, size_byte_align);
			UBOOT_DEBUG("Copy 0x%lx bytes from 0x%p to 0x%lx\n",size_byte_align, byte_align_buffer, (address + first_lba_read_byte + size_lba_number * blk_dev_desc->blksz));
		}
	}

	/* Set interface device & partition to default */
	if (strcmp(str_interface, "mmc") == 0)
	{
		memset(cmd, 0, 128);
		snprintf_len = snprintf(cmd, sizeof(cmd), "%s dev 0",str_interface);
		if (snprintf_len >= sizeof(cmd))
		{
			UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", (int)(sizeof(cmd)), cmd);
			return CMD_RET_FAILURE;
		}
		UBOOT_DEBUG("cmd=%s\n",cmd);
		if(run_command(cmd, 0) != 0)
		{
			UBOOT_ERROR("[ERROR] Set %s dev fail!!!\n",str_interface);
				return CMD_RET_FAILURE;
		}
	}
	else if (strcmp(str_interface, "ufs") == 0)
	{
		memset(cmd, 0, 128);
		snprintf_len = snprintf(cmd, sizeof(cmd), "%s device 0",str_interface);
		if (snprintf_len >= sizeof(cmd))
		{
			UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", (int)(sizeof(cmd)), cmd);
			return CMD_RET_FAILURE;
		}
		UBOOT_DEBUG("cmd=%s\n",cmd);
		if(run_command(cmd, 0) != 0)
		{
			UBOOT_ERROR("[ERROR] Set %s dev fail!!!\n",str_interface);
			return CMD_RET_FAILURE;
		}
	}

	return CMD_RET_SUCCESS;
}

int do_partition_write(const char *str_interface, const char *str_device, const char *str_partition_name, const char *str_address, const char *str_size, const char *str_start_byte_pos)
{
	char cmd[128];
	int snprintf_len;
	int check_one_pkg_ret = 0;
	unsigned int device = 0;
	unsigned long address = 0;
	unsigned long size = 0;
	unsigned long start_byte_pos = 0;
	unsigned long start_shift_lba_number = 0;
	unsigned long start_shift_byte = 0;
	unsigned long size_lba_number = 0;
	unsigned long size_byte_align = 0;
	struct blk_desc *blk_dev_desc = NULL;
	disk_partition_t partition_info;


	/* Check interface type */
	if (strcmp(str_interface, "mmc") == 0)
		UBOOT_DEBUG("Interface is '%s'\n",str_interface);
	else if (strcmp(str_interface, "usb") == 0)
		UBOOT_DEBUG("Interface is '%s', user should run 'usb start' first.\n",str_interface);
	else if (strcmp(str_interface, "ufs") == 0)
		UBOOT_DEBUG("Interface is '%s'\n",str_interface);
	else
	{
		UBOOT_ERROR("[ERROR] Unrecognized interface, only support 'mmc' & 'usb' & 'ufs' now.\n");
		return CMD_RET_FAILURE;
	}


	/* Convert device number */
	device = simple_strtoul(str_device, NULL, 10);


	/* Check length of partition name not exceed PART_NAME_LEN(32) */
	if(strnlen(str_partition_name,PART_NAME_LEN) >= PART_NAME_LEN)
	{
		UBOOT_ERROR("[ERROR] Partition_name can not exceed max value %d\n",PART_NAME_LEN);
		return CMD_RET_FAILURE;
	}


	/* Convert DRAM address */
	address = simple_strtoul(str_address, NULL, 16);


	/* Convert writing size by hexadecimal */
	size = simple_strtoul(str_size, NULL, 16);


	/* Convert starting reading position by hexadecimal */
	if(str_start_byte_pos != NULL)
		start_byte_pos = simple_strtoul(str_start_byte_pos, NULL, 16);
	else
		start_byte_pos = 0;


	/* Check this PKG is one-PKG or not */
	check_one_pkg_ret = _check_is_one_pkg(&address, &size);
	if (check_one_pkg_ret == PASS_IS_ONE_PKG)
	{
		UBOOT_DEBUG("This PKG is one-PKG, get new addr. & size.\n");

		UBOOT_DEBUG("source addr %s --> 0x%lx\n", str_address, address);
		UBOOT_DEBUG("write size %s  --> 0x%lx\n", str_size, size);
	}
	else if (check_one_pkg_ret == PASS_NOT_ONE_PKG)
	{
		UBOOT_DEBUG("This PKG is NOT one-PKG, use original addr. & size.\n");
	}
	else
	{
		UBOOT_ERROR("[ERROR] check one-pkg fail! (%d)\n", check_one_pkg_ret);
		return CMD_RET_FAILURE;
	}


	/* Get information about interface, including size of 1 LBA on interface */
	/* Set interface device */
	if ((strcmp(str_interface, "mmc") == 0) || (strcmp(str_interface, "usb") == 0))
	{
		/* Get size of 1 LBA (unit in bytes) */
		blk_dev_desc = blk_get_dev(str_interface, device);                    // parameter of blk_get_dev should input mmc device number
		if (!blk_dev_desc) {
			UBOOT_ERROR("[ERROR] %s dev %u NOT available\n",str_interface, device);
			return CMD_RET_FAILURE;
		}
		UBOOT_DEBUG("Flash type:%s, 1 LBA = 0x%lx bytes\n",str_interface,blk_dev_desc->blksz);

		/* Set interface device */
		if (device != 0)
		{
			memset(cmd, 0, 128);
			snprintf_len = snprintf(cmd, sizeof(cmd), "%s dev %u",str_interface,device);
			if (snprintf_len >= sizeof(cmd))
			{
				UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", (int)(sizeof(cmd)), cmd);
				return CMD_RET_FAILURE;
			}
			UBOOT_DEBUG("cmd=%s\n",cmd);
			if(run_command(cmd, 0) != 0)
			{
				UBOOT_ERROR("[ERROR] Set %s dev fail!!!\n",str_interface);
				return CMD_RET_FAILURE;
			}
		}
	}
	else if (strcmp(str_interface, "ufs") == 0)
	{
		/* Check interface device is 0, MTK only allow one UFS flash to exist */
		if (device != 0)      // UFS switch device # cmd is 'ufs target #', it's different from other flash type
		{
			UBOOT_ERROR("[ERROR] %s only support one target(0) now!!!\n",str_interface);
			return CMD_RET_FAILURE;
		}

		/* Get size of 1 LBA (unit in bytes) */
		blk_dev_desc = blk_get_dev(str_interface, 0);               // parameter of blk_get_dev should input UFS LUN(partition) number
		if (!blk_dev_desc) {                                        // MTK only allow one UFS flash to exist, so UFS user partition LUN hard code writes 0
			UBOOT_ERROR("[ERROR] %s LUN 0 NOT available\n",str_interface);
			return CMD_RET_FAILURE;
		}
		UBOOT_DEBUG("Flash type:%s, 1 LBA = 0x%lx bytes\n",str_interface,blk_dev_desc->blksz);
	}
	else
	{
		UBOOT_ERROR("[ERROR] Get %s size of 1 LBA fail!!!\n",str_interface);
		return CMD_RET_FAILURE;
	}


	/* Get the partition information, including start LBA & size LBA */
	if(get_partition_info(blk_dev_desc, str_partition_name, &partition_info, NULL) != CMD_RET_SUCCESS)
		return CMD_RET_FAILURE;


	/* Confirm that the write size does not exceed end of this partition */
	if(size + start_byte_pos > (partition_info.size*blk_dev_desc->blksz))
	{
		UBOOT_ERROR("[ERROR] Writing size can not exceed end of partition LBA (0x%lx LBA)!!!\n",(partition_info.start + partition_info.size));
		return CMD_RET_FAILURE;
	}


	/* Check whether the starting writing position is aligned 1 LBA (ex: emmc is 512 bytes)  */
	start_shift_lba_number = start_byte_pos / blk_dev_desc->blksz;
	start_shift_byte = start_byte_pos % blk_dev_desc->blksz;
	UBOOT_DEBUG("Start writing position will shift 0x%lx bytes (0x%lx LBA + 0x%lx bytes)\n", start_byte_pos, start_shift_lba_number, start_shift_byte);


	/* Start writing */
	char byte_align_buffer[blk_dev_desc->blksz];

	memset(byte_align_buffer, 0, blk_dev_desc->blksz);
	UBOOT_DEBUG("byte_align_buffer address 0x%p\n",byte_align_buffer);

	if(start_shift_byte == 0)     /* If writing position only needs to shift integrated LBA */
	{
		/* Check if write size is aligned 1 LBA (ex: emmc is 512 bytes)  */
		size_lba_number = size / blk_dev_desc->blksz;
		size_byte_align = size % blk_dev_desc->blksz;
		UBOOT_DEBUG("Write size = 0x%lx (0x%lx LBA + 0x%lx bytes)\n", size, size_lba_number, size_byte_align);


		/* Write integer LBA partition data to interface first */
		if (size_lba_number != 0)
		{
			memset(cmd, 0, 128);
			snprintf_len = snprintf(cmd, sizeof(cmd), "%s write 0x%lx 0x%lx 0x%lx",str_interface, address, (partition_info.start + start_shift_lba_number), size_lba_number);
			if (snprintf_len >= sizeof(cmd))
			{
				UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", (int)(sizeof(cmd)), cmd);
				return CMD_RET_FAILURE;
			}
			UBOOT_DEBUG("cmd=%s\n",cmd);
			if(run_command(cmd, 0) != 0)
			{
				UBOOT_ERROR("[ERROR] Write integer LBA partition data fail!!!\n");
				return CMD_RET_FAILURE;
			}
		}


		/* Write the remaining less than 1 LBA data from DRAM to interface */
		if (size_byte_align != 0)
		{
			/* Read the data in last incomplete LBA from interface to byte_align_buffer */
			memset(cmd, 0, 128);
			snprintf_len = snprintf(cmd, sizeof(cmd), "%s read 0x%p 0x%lx 0x1",str_interface, byte_align_buffer, (partition_info.start + start_shift_lba_number + size_lba_number));
			if (snprintf_len >= sizeof(cmd))
			{
				UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", (int)(sizeof(cmd)), cmd);
				return CMD_RET_FAILURE;
			}
			UBOOT_DEBUG("cmd=%s\n",cmd);
			if(run_command(cmd, 0) != 0)
			{
				UBOOT_ERROR("[ERROR] Read last align bytes partition data fail!!!\n");
				return CMD_RET_FAILURE;
			}

			/* Only copy the remaining bytes of data from str_address to byte_align_buffer */
			memcpy(byte_align_buffer, (void *)(address + size_lba_number * blk_dev_desc->blksz), size_byte_align);
			UBOOT_DEBUG("Copy 0x%lx bytes from 0x%lx to 0x%p\n",size_byte_align, (address + size_lba_number * blk_dev_desc->blksz), byte_align_buffer);

			/* Write the data in last incomplete LBA from byte_align_buffer to interface */
			memset(cmd, 0, 128);
			snprintf_len = snprintf(cmd, sizeof(cmd), "%s write 0x%p 0x%lx 0x1",str_interface, byte_align_buffer, (partition_info.start + start_shift_lba_number + size_lba_number));
			if (snprintf_len >= sizeof(cmd))
			{
				UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", (int)(sizeof(cmd)), cmd);
				return CMD_RET_FAILURE;
			}
			UBOOT_DEBUG("cmd=%s\n",cmd);
			if(run_command(cmd, 0) != 0)
			{
				UBOOT_ERROR("[ERROR] Write last align bytes partition data fail!!!\n");
				return CMD_RET_FAILURE;
			}
		}
	}

	else                          /* If writing position needs to shift integrated LBA + aligned bytes */
	{
		unsigned long first_lba_write_byte = blk_dev_desc->blksz - start_shift_byte;

		if(size <= first_lba_write_byte)     // it means that all data you want to write is included in first_lba_write_byte
			first_lba_write_byte = size;     // so, update first_lba_write_byte to size

		/* Read the data in first incomplete LBA from interface to DRAM */
		memset(cmd, 0, 128);
		snprintf_len = snprintf(cmd, sizeof(cmd), "%s read 0x%p 0x%lx 0x1",str_interface, byte_align_buffer, (partition_info.start + start_shift_lba_number));
		if (snprintf_len >= sizeof(cmd))
		{
			UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", (int)(sizeof(cmd)), cmd);
			return CMD_RET_FAILURE;
		}
		UBOOT_DEBUG("cmd=%s\n",cmd);
		if(run_command(cmd, 0) != 0)
		{
			UBOOT_ERROR("[ERROR] Read first align bytes partition data fail!!!\n");
			return CMD_RET_FAILURE;
		}


		/* Only copy first_lba_write_byte bytes of data from str_address to byte_align_buffer[start_shift_byte] */
		memcpy((byte_align_buffer + start_shift_byte), (void *)address, first_lba_write_byte);
		UBOOT_DEBUG("Copy 0x%lx bytes from 0x%lx to 0x%lx\n", first_lba_write_byte, address, ((unsigned long)byte_align_buffer + start_shift_byte));


		/* Write the data in first incomplete LBA from byte_align_buffer to interface */
		memset(cmd, 0, 128);
		snprintf_len = snprintf(cmd, sizeof(cmd), "%s write 0x%p 0x%lx 0x1",str_interface, byte_align_buffer, (partition_info.start + start_shift_lba_number));
		if (snprintf_len >= sizeof(cmd))
		{
			UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", (int)(sizeof(cmd)), cmd);
			return CMD_RET_FAILURE;
		}
		UBOOT_DEBUG("cmd=%s\n",cmd);
		if(run_command(cmd, 0) != 0)
		{
			UBOOT_ERROR("[ERROR] Write first align bytes partition data fail!!!\n");
			return CMD_RET_FAILURE;
		}


		/* Update remaining wirting bytes */
		size = size - first_lba_write_byte;


		/* Check if remaining write size is aligned 1 LBA (ex: emmc is 512 bytes)  */
		size_lba_number = size / blk_dev_desc->blksz;
		size_byte_align = size % blk_dev_desc->blksz;
		UBOOT_DEBUG("Remaining write size = 0x%lx (0x%lx LBA + 0x%lx bytes)\n", size, size_lba_number, size_byte_align);


		/* Write integer LBA partition data to DRAM first */
		if (size_lba_number != 0)
		{
			memset(cmd, 0, 128);
			snprintf_len = snprintf(cmd, sizeof(cmd), "%s write 0x%lx 0x%lx 0x%lx",str_interface, (address + first_lba_write_byte), (partition_info.start + start_shift_lba_number + 1), size_lba_number);
			if (snprintf_len >= sizeof(cmd))
			{
				UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", (int)(sizeof(cmd)), cmd);
				return CMD_RET_FAILURE;
			}
			UBOOT_DEBUG("cmd=%s\n",cmd);
			if(run_command(cmd, 0) != 0)
			{
				UBOOT_ERROR("[ERROR] Write integer LBA partition data fail!!!\n");
				return CMD_RET_FAILURE;
			}
		}


		/* Write the remaining less than 1 LBA data from interface to DRAM */
		if (size_byte_align != 0)
		{
			/* Read the data in last incomplete LBA from interface to byte_align_buffer */
			memset(byte_align_buffer, 0, blk_dev_desc->blksz);
			memset(cmd, 0, 128);
			snprintf_len = snprintf(cmd, sizeof(cmd), "%s read 0x%p 0x%lx 0x1",str_interface, byte_align_buffer, (partition_info.start + start_shift_lba_number + 1 + size_lba_number));
			if (snprintf_len >= sizeof(cmd))
			{
				UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", (int)(sizeof(cmd)), cmd);
				return CMD_RET_FAILURE;
			}
			UBOOT_DEBUG("cmd=%s\n",cmd);
			if(run_command(cmd, 0) != 0)
			{
				UBOOT_ERROR("[ERROR] Read last align bytes partition data fail!!!\n");
				return CMD_RET_FAILURE;
			}

			/* Only copy the remaining bytes of data from byte_align_buffer to (str_address+first_lba_write_byte+size_lba_number*blk_dev_desc->blksz) */
			memcpy(byte_align_buffer, (void *)(address + first_lba_write_byte + size_lba_number * blk_dev_desc->blksz), size_byte_align);
			UBOOT_DEBUG("Copy 0x%lx bytes from 0x%lx to 0x%p\n",size_byte_align, (address + first_lba_write_byte + size_lba_number * blk_dev_desc->blksz), byte_align_buffer);

			/* Write the data in last incomplete LBA from byte_align_buffer to interface */
			memset(cmd, 0, 128);
			snprintf_len = snprintf(cmd, sizeof(cmd), "%s write 0x%p 0x%lx 0x1",str_interface, byte_align_buffer, (partition_info.start + start_shift_lba_number + 1 + size_lba_number));
			if (snprintf_len >= sizeof(cmd))
			{
				UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", (int)(sizeof(cmd)), cmd);
				return CMD_RET_FAILURE;
			}
			UBOOT_DEBUG("cmd=%s\n",cmd);
			if(run_command(cmd, 0) != 0)
			{
				UBOOT_ERROR("[ERROR] Write last align bytes partition data fail!!!\n");
				return CMD_RET_FAILURE;
			}
		}
	}

	return CMD_RET_SUCCESS;
}

int do_partition_write_boot(const char *str_interface, const char *str_device, const char *str_partition_number, const char *str_address, const char *str_size, const char *str_start_byte_pos)
{
	char cmd[128];
	int snprintf_len;
	int check_one_pkg_ret = 0;
	unsigned int device = 0;
	unsigned int partition = 0;
	unsigned long address = 0;
	unsigned long size = 0;
	unsigned long boot_partition_size_lba = 0;
	unsigned long start_byte_pos = 0;
	unsigned long start_shift_lba_number = 0;
	unsigned long start_shift_byte = 0;
	unsigned long size_lba_number = 0;
	unsigned long size_byte_align = 0;
	struct blk_desc *blk_dev_desc = NULL;

	sbvc_result sbvc_val =
		sboot_version_check((char *)simple_strtoul(str_address, NULL, 16),
			(int)simple_strtoul(str_size, NULL, 16),
			(int)simple_strtoul(str_partition_number, NULL, 10));
	printf("\nsboot_version_check: %d\n", sbvc_val);
	if (sbvc_val == SBVC_MARK_NOT_FOUND) {
		printf("ERROR: invalid sboot image.\nHALTING...\n");

		while (1);

	} else if (sbvc_val == SBVC_ROLLBACK) {  /* Anti-rollback flash control */
		if (target_is_production()) {
			printf("\n\n ERROR: sboot version rollback detected\n\n HALTING...\n\n");
			while (1);
		}
	} else if ((sbvc_val != SBVC_MATCH) && (sbvc_val != SBVC_DEV_MARK_NOT_FOUND)) {
		printf("Warning: sboot is not updated\n");
		return CMD_RET_SUCCESS;  // return success though
	}

	/* Check interface type */
	if (strcmp(str_interface, "mmc") == 0)
		UBOOT_DEBUG("Interface is '%s'\n",str_interface);
	else if (strcmp(str_interface, "ufs") == 0)
		UBOOT_DEBUG("Interface is '%s'\n",str_interface);
	else
	{
		UBOOT_ERROR("[ERROR] Unrecognized interface, only support 'mmc' & 'ufs' now.\n");
		return CMD_RET_FAILURE;
	}


	/* Convert device number */
	device = simple_strtoul(str_device, NULL, 10);


	/* Convert partition number */
	partition = simple_strtoul(str_partition_number, NULL, 10);


	/* Convert DRAM address */
	address = simple_strtoul(str_address, NULL, 16);


	/* Convert writing size by hexadecimal */
	size = simple_strtoul(str_size, NULL, 16);


	/* Convert starting reading position by hexadecimal */
	if(str_start_byte_pos != NULL)
		start_byte_pos = simple_strtoul(str_start_byte_pos, NULL, 16);
	else
		start_byte_pos = 0;


	/* Check this PKG is one-PKG or not */
	check_one_pkg_ret = _check_is_one_pkg(&address, &size);
	if (check_one_pkg_ret == PASS_IS_ONE_PKG)
	{
		UBOOT_DEBUG("This PKG is one-PKG, get new addr. & size.\n");

		UBOOT_DEBUG("source addr %s --> 0x%lx\n", str_address, address);
		UBOOT_DEBUG("write size %s  --> 0x%lx\n", str_size, size);
	}
	else if (check_one_pkg_ret == PASS_NOT_ONE_PKG)
	{
		UBOOT_DEBUG("This PKG is NOT one-PKG, use original addr. & size.\n");
	}
	else
	{
		UBOOT_ERROR("[ERROR] check one pkg fail! (%d)\n", check_one_pkg_ret);
		return CMD_RET_FAILURE;
	}

	/* Get information about interface, including size of 1 LBA & boot partition on interface */
	if (strcmp(str_interface, "mmc") == 0)
	{
#ifdef CONFIG_MMC
		/* Get size of 1 LBA (unit in bytes) */
		blk_dev_desc = blk_get_dev(str_interface, device);                    // parameter of blk_get_dev should input mmc device number
		if (!blk_dev_desc) {
			UBOOT_ERROR("[ERROR] %s dev %u NOT available\n",str_interface, device);
			return CMD_RET_FAILURE;
		}
		UBOOT_DEBUG("Flash type:%s, 1 LBA = 0x%lx bytes\n",str_interface,blk_dev_desc->blksz);

		/* Get boot partition capacity (unit in bytes) */
		struct mmc *mmc;
		mmc = find_mmc_device(device);
		if (!mmc) {
			UBOOT_ERROR("[ERROR] no mmc device at slot %x\n", device);
			return CMD_RET_FAILURE;
		}
		boot_partition_size_lba = mmc->capacity_boot / blk_dev_desc->blksz;   // transfer unit of boot partition capacity from bytes to LBA

		/* Set interface device & partition */
		/* blk_get_dev will set interface-partition to default 0, so change interface-partition after blk_get_dev  */
		memset(cmd, 0, 128);
		snprintf_len = snprintf(cmd, sizeof(cmd), "%s dev %u %u",str_interface,device,partition);
		if (snprintf_len >= sizeof(cmd))
		{
			UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", (int)(sizeof(cmd)), cmd);
			return CMD_RET_FAILURE;
		}
		UBOOT_DEBUG("cmd=%s\n",cmd);
		if(run_command(cmd, 0) != 0)
		{
			UBOOT_ERROR("[ERROR] Set %s dev fail!!!\n",str_interface);
			return CMD_RET_FAILURE;
		}
#else
        UBOOT_ERROR("[ERROR] mmc config is not enable\n");
#endif
	}
	else if (strcmp(str_interface, "ufs") == 0)
	{
		/* Check interface device is 0 */
		if (device != 0)      // UFS switch device # cmd is 'ufs target #', it's different from other flash type
		{
			UBOOT_ERROR("[ERROR] %s only support one target(0) now!!!\n",str_interface);
			return CMD_RET_FAILURE;
		}

		/* Get size of 1 LBA (unit in bytes) */
		blk_dev_desc = blk_get_dev(str_interface, partition);               // parameter of blk_get_dev should input UFS LUN(partition) number
		if (!blk_dev_desc) {
			UBOOT_ERROR("[ERROR] %s LUN %u NOT available\n",str_interface, partition);
			return CMD_RET_FAILURE;
		}
		UBOOT_DEBUG("Flash type:%s, 1 LBA = 0x%lx bytes\n",str_interface,blk_dev_desc->blksz);

		/* Set interface partition */
		memset(cmd, 0, 128);
		snprintf_len = snprintf(cmd, sizeof(cmd), "%s device %u",str_interface,partition);   // UFS switch partition # cmd is 'ufs device #', it's different from other flash type
		if (snprintf_len >= sizeof(cmd))
		{
			UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", (int)(sizeof(cmd)), cmd);
			return CMD_RET_FAILURE;
		}
		UBOOT_DEBUG("cmd=%s\n",cmd);
		if(run_command(cmd, 0) != 0)
		{
			UBOOT_ERROR("[ERROR] Set %s dev fail!!!\n",str_interface);
			return CMD_RET_FAILURE;
		}

		/* Get boot partition capacity (unit in LBA) */
		memset(cmd, 0, 128);
		snprintf_len = snprintf(cmd, sizeof(cmd), "ufs read.capacity 0x%p",&boot_partition_size_lba);
		if (snprintf_len >= sizeof(cmd))
		{
			UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", (int)(sizeof(cmd)), cmd);
			return CMD_RET_FAILURE;
		}
		UBOOT_DEBUG("cmd=%s\n",cmd);
		if(run_command(cmd, 0) != 0)
		{
			UBOOT_ERROR("[ERROR] ufs read.capacity fail!!!\n");
			return CMD_RET_FAILURE;
		}
	}
	else
	{
		UBOOT_ERROR("[ERROR] Get %s size of 1 LBA fail!!!\n",str_interface);
		return CMD_RET_FAILURE;
	}

	if(boot_partition_size_lba != 0)
		UBOOT_DEBUG("dev # %u, partition # %u, total LBA # %u\n",device, partition, (unsigned int)boot_partition_size_lba);
	else
	{
		UBOOT_ERROR("[ERROR] Get %s boot partition capacity fail!!!\n",str_interface);
		return CMD_RET_FAILURE;
	}


	/* Confirm that the write size does not exceed end of this boot partition */
	if(size + start_byte_pos > boot_partition_size_lba * blk_dev_desc->blksz)
	{
		UBOOT_ERROR("[ERROR] Writing size can not exceed end of boot partition LBA (0x%lx LBA)!!!\n",boot_partition_size_lba);
		return CMD_RET_FAILURE;
	}


	/* Check whether the starting writing position is aligned 1 LBA (ex: emmc is 512 bytes)  */
	start_shift_lba_number = start_byte_pos / blk_dev_desc->blksz;
	start_shift_byte = start_byte_pos % blk_dev_desc->blksz;
	UBOOT_DEBUG("Start writing position will shift 0x%lx bytes (0x%lx LBA + 0x%lx bytes)\n", start_byte_pos, start_shift_lba_number, start_shift_byte);


	/* Start writing */
	char byte_align_buffer[blk_dev_desc->blksz];

	memset(byte_align_buffer, 0, blk_dev_desc->blksz);
	UBOOT_DEBUG("byte_align_buffer address 0x%p\n",byte_align_buffer);

	if(start_shift_byte == 0)     /* If writing position only needs to shift integrated LBA */
	{
		/* Check if write size is aligned 1 LBA (ex: emmc is 512 bytes)  */
		size_lba_number = size / blk_dev_desc->blksz;
		size_byte_align = size % blk_dev_desc->blksz;
		UBOOT_DEBUG("Write size = 0x%lx (0x%lx LBA + 0x%lx bytes)\n", size, size_lba_number, size_byte_align);


		/* Write integer LBA partition data to interface first */
		if (size_lba_number != 0)
		{
			memset(cmd, 0, 128);
			snprintf_len = snprintf(cmd, sizeof(cmd), "%s write 0x%lx 0x%lx 0x%lx",str_interface, address, start_shift_lba_number, size_lba_number);
			if (snprintf_len >= sizeof(cmd))
			{
				UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", (int)(sizeof(cmd)), cmd);
				return CMD_RET_FAILURE;
			}
			UBOOT_DEBUG("cmd=%s\n",cmd);
			if(run_command(cmd, 0) != 0)
			{
				UBOOT_ERROR("[ERROR] Write integer LBA partition data fail!!!\n");
				return CMD_RET_FAILURE;
			}
		}


		/* Write the remaining less than 1 LBA data from DRAM to interface */
		if (size_byte_align != 0)
		{
			/* Read the data in last incomplete LBA from interface to byte_align_buffer */
			memset(cmd, 0, 128);
			snprintf_len = snprintf(cmd, sizeof(cmd), "%s read 0x%p 0x%lx 0x1",str_interface, byte_align_buffer, (start_shift_lba_number + size_lba_number));
			if (snprintf_len >= sizeof(cmd))
			{
				UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", (int)(sizeof(cmd)), cmd);
				return CMD_RET_FAILURE;
			}
			UBOOT_DEBUG("cmd=%s\n",cmd);
			if(run_command(cmd, 0) != 0)
			{
				UBOOT_ERROR("[ERROR] Read last align bytes partition data fail!!!\n");
				return CMD_RET_FAILURE;
			}

			/* Only copy the remaining bytes of data from str_address to byte_align_buffer */
			memcpy(byte_align_buffer, (void *)(address + size_lba_number * blk_dev_desc->blksz), size_byte_align);
			UBOOT_DEBUG("Copy 0x%lx bytes from 0x%lx to 0x%p\n",size_byte_align, (address + size_lba_number * blk_dev_desc->blksz), byte_align_buffer);

			/* Write the data in last incomplete LBA from byte_align_buffer to interface */
			memset(cmd, 0, 128);
			snprintf_len = snprintf(cmd, sizeof(cmd), "%s write 0x%p 0x%lx 0x1",str_interface, byte_align_buffer, (start_shift_lba_number + size_lba_number));
			if (snprintf_len >= sizeof(cmd))
			{
				UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", (int)(sizeof(cmd)), cmd);
				return CMD_RET_FAILURE;
			}
			UBOOT_DEBUG("cmd=%s\n",cmd);
			if(run_command(cmd, 0) != 0)
			{
				UBOOT_ERROR("[ERROR] Write last align bytes partition data fail!!!\n");
				return CMD_RET_FAILURE;
			}
		}
	}

	else                          /* If writing position needs to shift integrated LBA + aligned bytes */
	{
		unsigned long first_lba_write_byte = blk_dev_desc->blksz - start_shift_byte;

		if(size <= first_lba_write_byte)     // it means that all data you want to write is included in first_lba_write_byte
			first_lba_write_byte = size;     // so, update first_lba_write_byte to size

		/* Read the data in first incomplete LBA from interface to DRAM */
		memset(cmd, 0, 128);
		snprintf_len = snprintf(cmd, sizeof(cmd), "%s read 0x%p 0x%lx 0x1",str_interface, byte_align_buffer, start_shift_lba_number);
		if (snprintf_len >= sizeof(cmd))
		{
			UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", (int)(sizeof(cmd)), cmd);
			return CMD_RET_FAILURE;
		}
		UBOOT_DEBUG("cmd=%s\n",cmd);
		if(run_command(cmd, 0) != 0)
		{
			UBOOT_ERROR("[ERROR] Read first align bytes partition data fail!!!\n");
			return CMD_RET_FAILURE;
		}


		/* Only copy first_lba_write_byte bytes of data from str_address to byte_align_buffer[start_shift_byte] */
		memcpy((byte_align_buffer + start_shift_byte), (void *)address, first_lba_write_byte);
		UBOOT_DEBUG("Copy 0x%lx bytes from 0x%lx to 0x%lx\n", first_lba_write_byte, address, ((unsigned long)byte_align_buffer + start_shift_byte));


		/* Write the data in first incomplete LBA from byte_align_buffer to interface */
		memset(cmd, 0, 128);
		snprintf_len = snprintf(cmd, sizeof(cmd), "%s write 0x%p 0x%lx 0x1",str_interface, byte_align_buffer, start_shift_lba_number);
		if (snprintf_len >= sizeof(cmd))
		{
			UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", (int)(sizeof(cmd)), cmd);
			return CMD_RET_FAILURE;
		}
		UBOOT_DEBUG("cmd=%s\n",cmd);
		if(run_command(cmd, 0) != 0)
		{
			UBOOT_ERROR("[ERROR] Write first align bytes partition data fail!!!\n");
			return CMD_RET_FAILURE;
		}


		/* Update remaining wirting bytes */
		size = size - first_lba_write_byte;


		/* Check if remaining write size is aligned 1 LBA (ex: emmc is 512 bytes)  */
		size_lba_number = size / blk_dev_desc->blksz;
		size_byte_align = size % blk_dev_desc->blksz;
		UBOOT_DEBUG("Remaining write size = 0x%lx (0x%lx LBA + 0x%lx bytes)\n", size, size_lba_number, size_byte_align);


		/* Write integer LBA partition data to DRAM first */
		if (size_lba_number != 0)
		{
			memset(cmd, 0, 128);
			snprintf_len = snprintf(cmd, sizeof(cmd), "%s write 0x%lx 0x%lx 0x%lx",str_interface, (address + first_lba_write_byte), (start_shift_lba_number + 1), size_lba_number);
			if (snprintf_len >= sizeof(cmd))
			{
				UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", (int)(sizeof(cmd)), cmd);
				return CMD_RET_FAILURE;
			}
			UBOOT_DEBUG("cmd=%s\n",cmd);
			if(run_command(cmd, 0) != 0)
			{
				UBOOT_ERROR("[ERROR] Write integer LBA partition data fail!!!\n");
				return CMD_RET_FAILURE;
			}
		}


		/* Write the remaining less than 1 LBA data from interface to DRAM */
		if (size_byte_align != 0)
		{
			/* Read the data in last incomplete LBA from interface to byte_align_buffer */
			memset(byte_align_buffer, 0, blk_dev_desc->blksz);
			memset(cmd, 0, 128);
			snprintf_len = snprintf(cmd, sizeof(cmd), "%s read 0x%p 0x%lx 0x1",str_interface, byte_align_buffer, (start_shift_lba_number + 1 + size_lba_number));
			if (snprintf_len >= sizeof(cmd))
			{
				UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", (int)(sizeof(cmd)), cmd);
				return CMD_RET_FAILURE;
			}
			UBOOT_DEBUG("cmd=%s\n",cmd);
			if(run_command(cmd, 0) != 0)
			{
				UBOOT_ERROR("[ERROR] Read last align bytes partition data fail!!!\n");
				return CMD_RET_FAILURE;
			}

			/* Only copy the remaining bytes of data from byte_align_buffer to (str_address+first_lba_write_byte+size_lba_number*blk_dev_desc->blksz) */
			memcpy(byte_align_buffer, (void *)(address + first_lba_write_byte + size_lba_number * blk_dev_desc->blksz), size_byte_align);
			UBOOT_DEBUG("Copy 0x%lx bytes from 0x%lx to 0x%p\n",size_byte_align, (address + first_lba_write_byte + size_lba_number * blk_dev_desc->blksz), byte_align_buffer);

			/* Write the data in last incomplete LBA from byte_align_buffer to interface */
			memset(cmd, 0, 128);
			snprintf_len = snprintf(cmd, sizeof(cmd), "%s write 0x%p 0x%lx 0x1",str_interface, byte_align_buffer, (start_shift_lba_number + 1 + size_lba_number));
			if (snprintf_len >= sizeof(cmd))
			{
				UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", (int)(sizeof(cmd)), cmd);
				return CMD_RET_FAILURE;
			}
			UBOOT_DEBUG("cmd=%s\n",cmd);
			if(run_command(cmd, 0) != 0)
			{
				UBOOT_ERROR("[ERROR] Write last align bytes partition data fail!!!\n");
				return CMD_RET_FAILURE;
			}
		}
	}

	/* Set interface device & partition to default */
	if (strcmp(str_interface, "mmc") == 0)
	{
		memset(cmd, 0, 128);
		snprintf_len = snprintf(cmd, sizeof(cmd), "%s dev 0",str_interface);
		if (snprintf_len >= sizeof(cmd))
		{
			UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", (int)(sizeof(cmd)), cmd);
			return CMD_RET_FAILURE;
		}
		UBOOT_DEBUG("cmd=%s\n",cmd);
		if(run_command(cmd, 0) != 0)
		{
			UBOOT_ERROR("[ERROR] Set %s dev fail!!!\n",str_interface);
				return CMD_RET_FAILURE;
		}
	}
	else if (strcmp(str_interface, "ufs") == 0)
	{
		memset(cmd, 0, 128);
		snprintf_len = snprintf(cmd, sizeof(cmd), "%s device 0",str_interface);
		if (snprintf_len >= sizeof(cmd))
		{
			UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", (int)(sizeof(cmd)), cmd);
			return CMD_RET_FAILURE;
		}
		UBOOT_DEBUG("cmd=%s\n",cmd);
		if(run_command(cmd, 0) != 0)
		{
			UBOOT_ERROR("[ERROR] Set %s dev fail!!!\n",str_interface);
			return CMD_RET_FAILURE;
		}
	}

	return CMD_RET_SUCCESS;
}

int do_partition_erase(const char *str_interface, const char *str_device, const char *str_partition_name)
{
	char cmd[128];
	int snprintf_len;
	unsigned int device = 0;
	unsigned int n;
	struct blk_desc *blk_dev_desc = NULL;
	disk_partition_t partition_info;


	/* Check interface type */
	if (strcmp(str_interface, "mmc") == 0)
		UBOOT_DEBUG("Interface is '%s'\n",str_interface);
	else if (strcmp(str_interface, "usb") == 0)
	{
		UBOOT_DEBUG("Usb not support erase cmd, still return SUCCESS\n");
		return CMD_RET_SUCCESS;
	}
	else if (strcmp(str_interface, "ufs") == 0)
		UBOOT_DEBUG("Interface is '%s'\n",str_interface);
	else
	{
		UBOOT_ERROR("[ERROR] Unrecognized interface, only support 'mmc' & 'ufs' now.\n");
		return CMD_RET_FAILURE;
	}


	/* Convert device number */
	device = simple_strtoul(str_device, NULL, 10);


	/* Check length of partition name not exceed PART_NAME_LEN(32) */
	if(strnlen(str_partition_name,PART_NAME_LEN) >= PART_NAME_LEN)
	{
		UBOOT_ERROR("[ERROR] Partition_name can not exceed max value %d\n",PART_NAME_LEN);
		return CMD_RET_FAILURE;
	}


	/* Get information about interface, including size of 1 LBA on interface */
	/* Set interface device */
	if (strcmp(str_interface, "mmc") == 0)
	{
		/* Get size of 1 LBA (unit in bytes) */
		blk_dev_desc = blk_get_dev(str_interface, device);                    // parameter of blk_get_dev should input mmc device number
		if (!blk_dev_desc) {
			UBOOT_ERROR("[ERROR] %s dev %u NOT available\n",str_interface, device);
			return CMD_RET_FAILURE;
		}
		UBOOT_DEBUG("Flash type:%s, 1 LBA = 0x%lx bytes\n",str_interface,blk_dev_desc->blksz);

		/* Set interface device */
		if (device != 0)
		{
			memset(cmd, 0, 128);
			snprintf_len = snprintf(cmd, sizeof(cmd), "%s dev %u",str_interface,device);
			if (snprintf_len >= sizeof(cmd))
			{
				UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", (int)(sizeof(cmd)), cmd);
				return CMD_RET_FAILURE;
			}
			UBOOT_DEBUG("cmd=%s\n",cmd);
			if(run_command(cmd, 0) != 0)
			{
				UBOOT_ERROR("[ERROR] Set %s dev fail!!!\n",str_interface);
				return CMD_RET_FAILURE;
			}
		}
	}
	else if (strcmp(str_interface, "ufs") == 0)
	{
		/* Check interface device is 0, MTK only allow one UFS flash to exist */
		if (device != 0)      // UFS switch device # cmd is 'ufs target #', it's different from other flash type
		{
			UBOOT_ERROR("[ERROR] %s only support one target(0) now!!!\n",str_interface);
			return CMD_RET_FAILURE;
		}

		/* Get size of 1 LBA (unit in bytes) */
		blk_dev_desc = blk_get_dev(str_interface, 0);               // parameter of blk_get_dev should input UFS LUN(partition) number
		if (!blk_dev_desc) {                                        // MTK only allow one UFS flash to exist, so UFS user partition LUN hard code writes 0
			UBOOT_ERROR("[ERROR] %s LUN 0 NOT available\n",str_interface);
			return CMD_RET_FAILURE;
		}
		UBOOT_DEBUG("Flash type:%s, 1 LBA = 0x%lx bytes\n",str_interface,blk_dev_desc->blksz);
	}
	else
	{
		UBOOT_ERROR("[ERROR] Get %s size of 1 LBA fail!!!\n",str_interface);
		return CMD_RET_FAILURE;
	}


	/* Get the partition information, including start LBA & size LBA */
	if(get_partition_info(blk_dev_desc, str_partition_name, &partition_info, NULL) != CMD_RET_SUCCESS)
		return CMD_RET_FAILURE;


	/* Erase data in interface */
	UBOOT_DEBUG("\n%s erase: dev # %u, block # %u, count %u ... ",
	       str_interface, device, (unsigned int)partition_info.start, (unsigned int)partition_info.size);

	n = blk_derase(blk_dev_desc, partition_info.start, partition_info.size);
	UBOOT_DEBUG("%u blocks erased: %s\n", n, (n == partition_info.size) ? "OK" : "ERROR");

	return (n == partition_info.size) ? CMD_RET_SUCCESS : CMD_RET_FAILURE;
}

int do_partition_erase_boot(const char *str_interface, const char *str_device, const char *str_partition_number)
{
	char cmd[128];
	int snprintf_len;
	unsigned int device = 0;
	unsigned int partition = 0;
	unsigned long boot_partition_size_lba = 0;
	unsigned int n;
	struct blk_desc *blk_dev_desc = NULL;


	/* Check interface type */
	if (strcmp(str_interface, "mmc") == 0)
		UBOOT_DEBUG("Interface is '%s'\n",str_interface);
	else if (strcmp(str_interface, "usb") == 0)
	{
		UBOOT_DEBUG("Usb not support erase cmd, still return SUCCESS\n");
		return CMD_RET_SUCCESS;
	}
	else if (strcmp(str_interface, "ufs") == 0)
		UBOOT_DEBUG("Interface is '%s'\n",str_interface);
	else
	{
		UBOOT_ERROR("[ERROR] Unrecognized interface, only support 'mmc' & 'ufs' now.\n");
		return CMD_RET_FAILURE;
	}


	/* Convert device number */
	device = simple_strtoul(str_device, NULL, 10);


	/* Convert partition number */
	partition = simple_strtoul(str_partition_number, NULL, 10);

	/* Get information about interface, including size of 1 LBA & boot partition on interface */
	if (strcmp(str_interface, "mmc") == 0)
	{
#ifdef CONFIG_MMC
		/* Get size of 1 LBA (unit in bytes) */
		blk_dev_desc = blk_get_dev(str_interface, device);                    // parameter of blk_get_dev should input mmc device number
		if (!blk_dev_desc) {
			UBOOT_ERROR("[ERROR] %s dev %u NOT available\n",str_interface, device);
			return CMD_RET_FAILURE;
		}
		UBOOT_DEBUG("Flash type:%s, 1 LBA = 0x%lx bytes\n",str_interface,blk_dev_desc->blksz);

		/* Get boot partition capacity (unit in bytes) */
		struct mmc *mmc;
		mmc = find_mmc_device(device);
		if (!mmc) {
			UBOOT_ERROR("[ERROR] no mmc device at slot %x\n", device);
			return CMD_RET_FAILURE;
		}
		boot_partition_size_lba = mmc->capacity_boot / blk_dev_desc->blksz;   // transfer unit of boot partition capacity from bytes to LBA

		/* Set interface device & partition */
		/* blk_get_dev will set interface-partition to default 0, so change interface-partition after blk_get_dev  */
		memset(cmd, 0, 128);
		snprintf_len = snprintf(cmd, sizeof(cmd), "%s dev %u %u",str_interface,device,partition);
		if (snprintf_len >= sizeof(cmd))
		{
			UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", (int)(sizeof(cmd)), cmd);
			return CMD_RET_FAILURE;
		}
		UBOOT_DEBUG("cmd=%s\n",cmd);
		if(run_command(cmd, 0) != 0)
		{
			UBOOT_ERROR("[ERROR] Set %s dev fail!!!\n",str_interface);
			return CMD_RET_FAILURE;
		}
#else
        UBOOT_ERROR("[ERROR] mmc config is not enable\n");
#endif
	}
	else if (strcmp(str_interface, "ufs") == 0)
	{
		/* Check interface device is 0 */
		if (device != 0)      // UFS switch device # cmd is 'ufs target #', it's different from other flash type
		{
			UBOOT_ERROR("[ERROR] %s only support one target(0) now!!!\n",str_interface);
			return CMD_RET_FAILURE;
		}

		/* Get size of 1 LBA (unit in bytes) */
		blk_dev_desc = blk_get_dev(str_interface, partition);               // parameter of blk_get_dev should input UFS LUN(partition) number
		if (!blk_dev_desc) {
			UBOOT_ERROR("[ERROR] %s LUN %u NOT available\n",str_interface, partition);
			return CMD_RET_FAILURE;
		}
		UBOOT_DEBUG("Flash type:%s, 1 LBA = 0x%lx bytes\n",str_interface,blk_dev_desc->blksz);

		/* Set interface partition */
		memset(cmd, 0, 128);
		snprintf_len = snprintf(cmd, sizeof(cmd), "%s device %u",str_interface,partition);   // UFS switch partition # cmd is 'ufs device #', it's different from other flash type
		if (snprintf_len >= sizeof(cmd))
		{
			UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", (int)(sizeof(cmd)), cmd);
			return CMD_RET_FAILURE;
		}
		UBOOT_DEBUG("cmd=%s\n",cmd);
		if(run_command(cmd, 0) != 0)
		{
			UBOOT_ERROR("[ERROR] Set %s dev fail!!!\n",str_interface);
			return CMD_RET_FAILURE;
		}

		/* Get boot partition capacity (unit in LBA) */
		memset(cmd, 0, 128);
		snprintf_len = snprintf(cmd, sizeof(cmd), "ufs read.capacity 0x%p",&boot_partition_size_lba);
		if (snprintf_len >= sizeof(cmd))
		{
			UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", (int)(sizeof(cmd)), cmd);
			return CMD_RET_FAILURE;
		}
		UBOOT_DEBUG("cmd=%s\n",cmd);
		if(run_command(cmd, 0) != 0)
		{
			UBOOT_ERROR("[ERROR] ufs read.capacity fail!!!\n");
			return CMD_RET_FAILURE;
		}
	}
	else
	{
		UBOOT_ERROR("[ERROR] Get %s size of 1 LBA fail!!!\n",str_interface);
		return CMD_RET_FAILURE;
	}

	if(boot_partition_size_lba != 0)
		UBOOT_DEBUG("dev # %u, partition # %u, total LBA # %u\n",device, partition, (unsigned int)boot_partition_size_lba);
	else
	{
		UBOOT_ERROR("[ERROR] Get %s boot partition capacity fail!!!\n",str_interface);
		return CMD_RET_FAILURE;
	}


	/* Erase data in interface */
	UBOOT_DEBUG("\n%s erase: dev # %u, partition # %u, block # 0, count %u ... ",
	       str_interface, device, partition, (unsigned int)boot_partition_size_lba);

	n = blk_derase(blk_dev_desc, 0, boot_partition_size_lba);
	UBOOT_DEBUG("%u blocks erased: %s\n", n, (n == boot_partition_size_lba) ? "OK" : "ERROR");


	/* Set interface device & partition to default */
	if (strcmp(str_interface, "mmc") == 0)
	{
		memset(cmd, 0, 128);
		snprintf_len = snprintf(cmd, sizeof(cmd), "%s dev 0",str_interface);
		if (snprintf_len >= sizeof(cmd))
		{
			UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", (int)(sizeof(cmd)), cmd);
			return CMD_RET_FAILURE;
		}
		UBOOT_DEBUG("cmd=%s\n",cmd);
		if(run_command(cmd, 0) != 0)
		{
			UBOOT_ERROR("[ERROR] Set %s dev fail!!!\n",str_interface);
				return CMD_RET_FAILURE;
		}
	}
	else if (strcmp(str_interface, "ufs") == 0)
	{
		memset(cmd, 0, 128);
		snprintf_len = snprintf(cmd, sizeof(cmd), "%s device 0",str_interface);
		if (snprintf_len >= sizeof(cmd))
		{
			UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", (int)(sizeof(cmd)), cmd);
			return CMD_RET_FAILURE;
		}
		UBOOT_DEBUG("cmd=%s\n",cmd);
		if(run_command(cmd, 0) != 0)
		{
			UBOOT_ERROR("[ERROR] Set %s dev fail!!!\n",str_interface);
			return CMD_RET_FAILURE;
		}
	}

	return (n == boot_partition_size_lba) ? CMD_RET_SUCCESS : CMD_RET_FAILURE;
}

#if (CONFIG_AB_SIDELOAD == 1)
int do_partition_clone(const char *str_interface, const char *str_device, const char *str_action)
{
    char cmd[128];
    int snprintf_len;
    unsigned int action = 0;
    unsigned int device = 0;
    unsigned int partition = 0;
    int p, ret;
    struct blk_desc *blk_dev_desc = NULL;
    disk_partition_t info;
    char name_a[PART_NAME_LEN];
    char name_b[PART_NAME_LEN];
    unsigned int len;
    unsigned long long size;
    char *buf = NULL;

    /* Check interface type */
    if (strcmp(str_interface, "mmc") == 0)
    {
        UBOOT_DEBUG("Interface is '%s'\n",str_interface);
    }
    else if (strcmp(str_interface, "ufs") == 0)
    {
        UBOOT_DEBUG("Interface is '%s'\n",str_interface);
    }
    else
    {
        UBOOT_ERROR("[ERROR] Unrecognized interface, only support 'mmc' & 'ufs' now.\n");
        return CMD_RET_FAILURE;
    }

    /* Convert device number */
    device = simple_strtoul(str_device, NULL, 10);

    /* Check action */
    action = simple_strtoul(str_action, NULL, 10);

    if (action == 0)
    { //self-adaption
        if (get_ab_selection_result() == PARTITION_SELECT_A)
        {
            action = 1; //Clone slot A to slot B
        }
        else if (get_ab_selection_result() == PARTITION_SELECT_B)
        {
            action = 2; //Clone slot B to slot A
        }
        else
        {
            UBOOT_ERROR("[ERROR] Unrecognized slot number.\n");
            return CMD_RET_FAILURE;
        }
    }
    else if (action!=1 && action!=2)
    {
        UBOOT_ERROR("[ERROR] Unrecognized command, only support 0, 1, and 2.\n");
        return CMD_RET_FAILURE;
    }

    if (action == 1)
    {
        UBOOT_DEBUG("Clone slot A to slot B\n");
    }
    else
    {
        UBOOT_DEBUG("Clone slot B to slot A\n");
    }

    /* Get information about interface, including size of 1 LBA & boot partition on interface */
    if (strcmp(str_interface, "mmc") == 0)
    {
        /* Get size of 1 LBA (unit in bytes) */
        blk_dev_desc = blk_get_dev(str_interface, device);                    // parameter of blk_get_dev should input mmc device number
        if (!blk_dev_desc) {
            UBOOT_ERROR("[ERROR] %s dev %u NOT available\n",str_interface, device);
            return CMD_RET_FAILURE;
        }
        UBOOT_DEBUG("Flash type:%s, 1 LBA = 0x%lx bytes\n",str_interface,blk_dev_desc->blksz);
    }
    else if (strcmp(str_interface, "ufs") == 0)
    {
        /* Check interface device is 0 */
        if (device != 0)      // UFS switch device # cmd is 'ufs target #', it's different from other flash type
        {
            UBOOT_ERROR("[ERROR] %s only support one target(0) now!!!\n",str_interface);
            return CMD_RET_FAILURE;
        }

        /* Get size of 1 LBA (unit in bytes) */
        blk_dev_desc = blk_get_dev(str_interface, partition);               // parameter of blk_get_dev should input UFS LUN(partition) number
        if (!blk_dev_desc)
        {
            UBOOT_ERROR("[ERROR] %s LUN %u NOT available\n",str_interface, partition);
            return CMD_RET_FAILURE;
        }
        UBOOT_DEBUG("Flash type:%s, 1 LBA = 0x%lx bytes\n",str_interface,blk_dev_desc->blksz);
    }


    // Clone user area partitions
    for (p = 1; p <= MAX_SEARCH_PARTITIONS; p++)
    {
        ret = part_get_info(blk_dev_desc, p, &info);
        if (ret)
            continue;

        memset(name_a, 0, PART_NAME_LEN);
        memcpy(name_a, info.name, PART_NAME_LEN);
        len = strnlen((const char*)name_a, PART_NAME_LEN-1);
        name_a[len] = '\0';

        if (len>=2 && name_a[len-2]=='_' && name_a[len-1]=='a')
        {
            memcpy(name_b, name_a, PART_NAME_LEN);
            name_b[len-1] = 'b';
            name_b[len] = '\0';
            size = info.blksz * info.size;

            /* Check overflow
                x = a * b;
                if (a != 0 && x / a != b) {
                        // overflow handling
                }
            */
            if ( (info.blksz !=0) && (size/info.blksz != info.size))
            {
                UBOOT_ERROR("[ERROR] partition size overflow\n");
                return CMD_RET_FAILURE;
            }

            buf = (char *)malloc(size);
            if (!buf)
            {
                UBOOT_ERROR("[ERROR] malloc failed\n");
                return CMD_RET_FAILURE;
            }

            if (action == 1) //clone a to b
            {
                memset(cmd, 0, 128);
                snprintf_len = snprintf(cmd, sizeof(cmd), "partition read %s %d %s %p 0x%llx", str_interface, device, name_a, buf, size);
                if (snprintf_len >= sizeof(cmd))
                {
                    UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", (int)(sizeof(cmd)), cmd);
                    free(buf);
                    return CMD_RET_FAILURE;
                }
                UBOOT_DEBUG("cmd=%s\n",cmd);
                if(run_command(cmd, 0) != 0)
                {
                    UBOOT_ERROR("[ERROR] cmd run error\n");
                    free(buf);
                    return CMD_RET_FAILURE;
                }

                memset(cmd, 0, 128);
                snprintf_len = snprintf(cmd, sizeof(cmd), "partition write %s %d %s %p 0x%llx", str_interface, device, name_b, buf, size);
                if (snprintf_len >= sizeof(cmd))
                {
                    UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", (int)(sizeof(cmd)), cmd);
                    free(buf);
                    return CMD_RET_FAILURE;
                }
                UBOOT_DEBUG("cmd=%s\n",cmd);
                if(run_command(cmd, 0) != 0)
                {
                    UBOOT_ERROR("[ERROR] cmd run error\n");
                    free(buf);
                    return CMD_RET_FAILURE;
                }
            }
            else //clone b to a
            {
                memset(cmd, 0, 128);
                snprintf_len = snprintf(cmd, sizeof(cmd), "partition read %s %d %s %p 0x%llx", str_interface, device, name_b, buf, size);
                if (snprintf_len >= sizeof(cmd))
                {
                    UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", (int)(sizeof(cmd)), cmd);
                    free(buf);
                    return CMD_RET_FAILURE;
                }
                UBOOT_DEBUG("cmd=%s\n",cmd);
                if(run_command(cmd, 0) != 0)
                {
                    UBOOT_ERROR("[ERROR] cmd run error\n");
                    free(buf);
                    return CMD_RET_FAILURE;
                }

                memset(cmd, 0, 128);
                snprintf_len = snprintf(cmd, sizeof(cmd), "partition write %s %d %s %p 0x%llx", str_interface, device, name_a, buf, size);
                if (snprintf_len >= sizeof(cmd))
                {
                    UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", (int)(sizeof(cmd)), cmd);
                    free(buf);
                    return CMD_RET_FAILURE;
                }
                UBOOT_DEBUG("cmd=%s\n",cmd);
                if(run_command(cmd, 0) != 0)
                {
                    UBOOT_ERROR("[ERROR] cmd run error\n");
                    free(buf);
                    return CMD_RET_FAILURE;
                }
            }

            free(buf);
        }
    }

    // Clone boot partition
    //TODO remove hardcode 2MB boot partition size

#define BOOT_PARTITION_IMAGE_SIZE (0x200000)

    buf = (char *)malloc(BOOT_PARTITION_IMAGE_SIZE);
    if (!buf)
    {
        UBOOT_ERROR("[ERROR] malloc failed\n");
        return CMD_RET_FAILURE;
    }

    if (action == 1) // clone boot1 to boot2
    {
        memset(cmd, 0, 128);
        snprintf_len = snprintf(cmd, sizeof(cmd), "partition read.boot %s %d 1 %p 0x%x", str_interface, device, buf, BOOT_PARTITION_IMAGE_SIZE);
        if (snprintf_len >= sizeof(cmd))
        {
            UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", (int)(sizeof(cmd)), cmd);
            free(buf);
            return CMD_RET_FAILURE;
        }
        UBOOT_DEBUG("cmd=%s\n",cmd);
        if(run_command(cmd, 0) != 0)
        {
            UBOOT_ERROR("[ERROR] cmd run error\n");
            free(buf);
            return CMD_RET_FAILURE;
        }

        memset(cmd, 0, 128);
        snprintf_len = snprintf(cmd, sizeof(cmd), "partition write.boot %s %d 2 %p 0x%x", str_interface, device, buf, BOOT_PARTITION_IMAGE_SIZE);
        if (snprintf_len >= sizeof(cmd))
        {
            UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", (int)(sizeof(cmd)), cmd);
            free(buf);
            return CMD_RET_FAILURE;
        }
        UBOOT_DEBUG("cmd=%s\n",cmd);
        if(run_command(cmd, 0) != 0)
        {
            UBOOT_ERROR("[ERROR] cmd run error\n");
            free(buf);
            return CMD_RET_FAILURE;
        }
    }
    else // clone boot2 to boot 1
    {
        memset(cmd, 0, 128);
        snprintf_len = snprintf(cmd, sizeof(cmd), "partition read.boot %s %d 2 %p 0x%x", str_interface, device, buf, BOOT_PARTITION_IMAGE_SIZE);
        if (snprintf_len >= sizeof(cmd))
        {
            UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", (int)(sizeof(cmd)), cmd);
            free(buf);
            return CMD_RET_FAILURE;
        }
        UBOOT_DEBUG("cmd=%s\n",cmd);
        if(run_command(cmd, 0) != 0)
        {
            UBOOT_ERROR("[ERROR] cmd run error\n");
            free(buf);
            return CMD_RET_FAILURE;
        }

        memset(cmd, 0, 128);
        snprintf_len = snprintf(cmd, sizeof(cmd), "partition write.boot %s %d 1 %p 0x%x", str_interface, device, buf, BOOT_PARTITION_IMAGE_SIZE);
        if (snprintf_len >= sizeof(cmd))
        {
            UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", (int)(sizeof(cmd)), cmd);
            free(buf);
            return CMD_RET_FAILURE;
        }
        UBOOT_DEBUG("cmd=%s\n",cmd);
        if(run_command(cmd, 0) != 0)
        {
            UBOOT_ERROR("[ERROR] cmd run error\n");
            free(buf);
            return CMD_RET_FAILURE;
        }
    }

    free(buf);

    return CMD_RET_SUCCESS;
}
#endif


#if CONFIG_IS_ENABLED(IMAGE_SPARSE)

#define BOUNCE_BUFFER_SIZE 1024*1024
static lbaint_t partition_blkdev_sparse_write(struct sparse_storage *info, lbaint_t blk,
				 lbaint_t blkcnt, const void *buffer)
{
	struct blk_desc *dev_desc = info->priv;
	static unsigned char* bounce_buf;
	lbaint_t ret_blk, blk_w, r_blk, start;

	//check buffer alignment
	if (((uintptr_t)buffer & 0xFF) != 0 && (dev_desc->if_type == IF_TYPE_MMC)) {
		//check bounce buffer
		if (!bounce_buf) {
			bounce_buf = (unsigned char*)malloc(BOUNCE_BUFFER_SIZE);
			if (!bounce_buf) {
				UBOOT_ERROR("\n\nallocate bounce buffer fail \n");
				return -1;
			}
			UBOOT_DEBUG("\n\nallocate bounce buffer is 0x%x\n", BOUNCE_BUFFER_SIZE);
		}
		r_blk = blkcnt;
		ret_blk = 0;
		start = blk;
		while (blkcnt) {
			blk_w = (blkcnt > (BOUNCE_BUFFER_SIZE / dev_desc->blksz)) ?
						(BOUNCE_BUFFER_SIZE / dev_desc->blksz) : blkcnt;
			memcpy(bounce_buf, (void*) ((uintptr_t)buffer + ret_blk*dev_desc->blksz), blk_w*dev_desc->blksz);
			r_blk = blk_dwrite(dev_desc, start, blk_w, bounce_buf);
			ret_blk += r_blk;
			if (r_blk != blk_w)
				return ret_blk;
			start += blk_w;
			blkcnt -= blk_w;
		}
		return ret_blk;
	}
	else
		return blk_dwrite(dev_desc, blk, blkcnt, buffer);
}

static lbaint_t partition_blkdev_sparse_reserve(struct sparse_storage *info,
				   lbaint_t blk, lbaint_t blkcnt)
{
	return blkcnt;
}

int do_partition_swrite(const char *str_interface, const char *str_device, const char *str_partition_name, const char *str_address, const char *str_size)
{
	char cmd[128];
	int snprintf_len;
	unsigned int device = 0;
	unsigned long address = 0;
	struct blk_desc *blk_dev_desc = NULL;
	disk_partition_t partition_info;
	struct sparse_storage sparse;


	/* Check interface type */
	if (strcmp(str_interface, "mmc") == 0)
		UBOOT_DEBUG("Interface is '%s'\n",str_interface);
	else if (strcmp(str_interface, "usb") == 0)
		UBOOT_DEBUG("Interface is '%s', user should run 'usb start' first.\n",str_interface);
	else if (strcmp(str_interface, "ufs") == 0)
		UBOOT_DEBUG("Interface is '%s'\n",str_interface);
	else
	{
		UBOOT_ERROR("[ERROR] Unrecognized interface, only support 'mmc' & 'usb' & 'ufs' now.\n");
		return CMD_RET_FAILURE;
	}


	/* Convert device number */
	device = simple_strtoul(str_device, NULL, 10);


	/* Check length of partition name not exceed PART_NAME_LEN(32) */
	if(strnlen(str_partition_name,PART_NAME_LEN) >= PART_NAME_LEN)
	{
		UBOOT_ERROR("[ERROR] Partition_name can not exceed max value %d\n",PART_NAME_LEN);
		return CMD_RET_FAILURE;
	}


	/* Convert DRAM address */
	address = simple_strtoul(str_address, NULL, 16);


	/* if Not Sparse image do raw write*/
	if (!is_sparse_image((void *)address)) {
		UBOOT_ERROR("Not a sparse image, use partition write.\n");
		return do_partition_write(str_interface, str_device, str_partition_name, str_address, str_size, NULL);
	}


	/* Get information about interface, including size of 1 LBA on interface */
	/* Set interface device */
	if ((strcmp(str_interface, "mmc") == 0) || (strcmp(str_interface, "usb") == 0))
	{
		/* Get size of 1 LBA (unit in bytes) */
		blk_dev_desc = blk_get_dev(str_interface, device);                    // parameter of blk_get_dev should input mmc device number
		if (!blk_dev_desc) {
			UBOOT_ERROR("[ERROR] %s dev %u NOT available\n",str_interface, device);
			return CMD_RET_FAILURE;
		}
		UBOOT_DEBUG("Flash type:%s, 1 LBA = 0x%lx bytes\n",str_interface,blk_dev_desc->blksz);

		/* Set interface device */
		if (device != 0)
		{
			memset(cmd, 0, 128);
			snprintf_len = snprintf(cmd, sizeof(cmd), "%s dev %u",str_interface,device);
			if (snprintf_len >= sizeof(cmd))
			{
				UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", (int)(sizeof(cmd)), cmd);
				return CMD_RET_FAILURE;
			}
			UBOOT_DEBUG("cmd=%s\n",cmd);
			if(run_command(cmd, 0) != 0)
			{
				UBOOT_ERROR("[ERROR] Set %s dev fail!!!\n",str_interface);
				return CMD_RET_FAILURE;
			}
		}
	}
	else if (strcmp(str_interface, "ufs") == 0)
	{
		/* Check interface device is 0, MTK only allow one UFS flash to exist */
		if (device != 0)      // UFS switch device # cmd is 'ufs target #', it's different from other flash type
		{
			UBOOT_ERROR("[ERROR] %s only support one target(0) now!!!\n",str_interface);
			return CMD_RET_FAILURE;
		}

		/* Get size of 1 LBA (unit in bytes) */
		blk_dev_desc = blk_get_dev(str_interface, 0);               // parameter of blk_get_dev should input UFS LUN(partition) number
		if (!blk_dev_desc) {                                        // MTK only allow one UFS flash to exist, so UFS user partition LUN hard code writes 0
			UBOOT_ERROR("[ERROR] %s LUN 0 NOT available\n",str_interface);
			return CMD_RET_FAILURE;
		}
		UBOOT_DEBUG("Flash type:%s, 1 LBA = 0x%lx bytes\n",str_interface,blk_dev_desc->blksz);
	}
	else
	{
		UBOOT_ERROR("[ERROR] Get %s size of 1 LBA fail!!!\n",str_interface);
		return CMD_RET_FAILURE;
	}


	/* Get the partition information, including start LBA & size LBA */
	if(get_partition_info(blk_dev_desc, str_partition_name, &partition_info, NULL) != CMD_RET_SUCCESS)
		return CMD_RET_FAILURE;


	/* Fill sparse structure */
	sparse.priv = blk_dev_desc;
	sparse.blksz = blk_dev_desc->blksz;
	sparse.start = partition_info.start;
	sparse.size = partition_info.size;
	sparse.write = partition_blkdev_sparse_write;
	sparse.reserve = partition_blkdev_sparse_reserve;
	sparse.mssg = NULL;

	if (write_sparse_image(&sparse, str_partition_name, (void *)address, NULL))
		return CMD_RET_FAILURE;
	else
		return CMD_RET_SUCCESS;
}
#endif

int do_boot_partition_write(const char *str_partition_name, const char *str_address, const char *str_size, const char *str_start_byte_pos)
{
	char device_name[STORAGE_DEVICE_BUF_SIZE] = {0};
	char device_number[5] = "0";        // only considerating about device number is 0. If device number is not 0, please use another API to get partition info.

	if(sys_get_boot_device(device_name, sizeof(device_name))==-1)
	{
		UBOOT_ERROR("[ERROR] Get boot device fail !! \n");
		return CMD_RET_FAILURE;
	}

	UBOOT_DEBUG("device name: %s , device number: %s\n",device_name,device_number);

	if(do_partition_write(device_name,device_number, str_partition_name, str_address, str_size, str_start_byte_pos)==CMD_RET_FAILURE)
	{
		UBOOT_ERROR("[ERROR] Do raw write fail !! \n");
		return CMD_RET_FAILURE;
	}
	return CMD_RET_SUCCESS;
}

int do_boot_partition_read(const char *str_partition_name, const char *str_address, const char *str_size, const char *str_start_byte_pos)
{
	char device_name[STORAGE_DEVICE_BUF_SIZE] = {0};
	char device_number[5] = "0";        // only considerating about device number is 0. If device number is not 0, please use another API to get partition info.

	if(sys_get_boot_device(device_name, sizeof(device_name))==-1)
	{
		UBOOT_ERROR("[ERROR] Get boot device fail !! \n");
		return CMD_RET_FAILURE;
	}

	UBOOT_DEBUG("device name: %s , device number: %s\n",device_name,device_number);

	if(do_partition_read(device_name,device_number,str_partition_name,str_address,str_size, str_start_byte_pos)==CMD_RET_FAILURE)
	{
		UBOOT_ERROR("[ERROR] Do raw read fail !! \n");
		return CMD_RET_FAILURE;
	}
	return CMD_RET_SUCCESS;
}
int do_boot_partition_swrite(const char *str_partition_name, const char *str_address, const char *str_size)
{
	char device_name[STORAGE_DEVICE_BUF_SIZE] = {0};
	char device_number[5] = "0";        // only considerating about device number is 0. If device number is not 0, please use another API to get partition info.

	if(sys_get_boot_device(device_name, sizeof(device_name))==-1)
	{
		UBOOT_ERROR("[ERROR] Get boot device fail !! \n");
		return CMD_RET_FAILURE;
	}

	UBOOT_DEBUG("device name: %s , device number: %s\n",device_name,device_number);

	if(do_partition_swrite(device_name,device_number, str_partition_name, str_address, str_size)==CMD_RET_FAILURE)
	{
		UBOOT_ERROR("[ERROR] Do sparse raw write fail !! \n");
		return CMD_RET_FAILURE;
	}
	return CMD_RET_SUCCESS;
}


