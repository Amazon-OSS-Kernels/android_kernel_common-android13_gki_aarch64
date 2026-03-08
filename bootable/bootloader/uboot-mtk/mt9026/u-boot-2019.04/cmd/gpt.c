// SPDX-License-Identifier: GPL-2.0+
/*
 * cmd_gpt.c -- GPT (GUID Partition Table) handling command
 *
 * Copyright (C) 2015
 * Lukasz Majewski <l.majewski@majess.pl>
 *
 * Copyright (C) 2012 Samsung Electronics
 * author: Lukasz Majewski <l.majewski@samsung.com>
 * author: Piotr Wilczek <p.wilczek@samsung.com>
 */

#include <common.h>
#include <malloc.h>
#include <command.h>
#include <part_efi.h>
#include <exports.h>
#include <linux/ctype.h>
#include <div64.h>
#include <memalign.h>
#include <linux/compat.h>
#include <linux/sizes.h>
#include <stdlib.h>
#include <environment.h>
#include <partition/include/partition.h>
#include <utility.h>

#define MAX_PAR_STR_LEN     3000  // 64 partitions max=7665, but one partition uses 60 byte in average (name 40,size 20)
#define ONE_PAR_STR_LEN     168   // 1 partition max=168, calculated by API calc_parts_list_len, uuid_disk(49) + other(119)
#define NAME_CHAR_LEN       6
#define SIZE_CHAR_LEN       6
#define COMMA_CHAR_LEN      2
#define SEPARATE_CHAR_LEN   2
//#define DEBUG               1     // If you want to  print debug log, please remove "//"

static LIST_HEAD(disk_partitions);

/**
 * extract_env(): Expand env name from string format '&{env_name}'
 *                and return pointer to the env (if the env is set)
 *
 * @param str - pointer to string
 * @param env - pointer to pointer to extracted env
 *
 * @return - zero on successful expand and env is set
 */
static int extract_env(const char *str, char **env)
{
	int ret = -1;
	char *e, *s;
#ifdef CONFIG_RANDOM_UUID
	char uuid_str[UUID_STR_LEN + 1];
#endif

	if (!str || strlen(str) < 4)
		return -1;

	if (!((strncmp(str, "${", 2) == 0) && (str[strlen(str) - 1] == '}')))
		return -1;

	s = strdup(str);
	if (s == NULL)
		return -1;

	memset(s + strlen(s) - 1, '\0', 1);
	memmove(s, s + 2, strlen(s) - 1);

	e = env_get(s);
	if (e == NULL) {
#ifdef CONFIG_RANDOM_UUID
		debug("%s unset. ", str);
		gen_rand_uuid_str(uuid_str, UUID_STR_FORMAT_GUID);
		env_set(s, uuid_str);

		e = env_get(s);
		if (e) {
			debug("Set to random.\n");
			ret = 0;
		} else {
			debug("Can't get random UUID.\n");
		}
#else
		debug("%s unset.\n", str);
#endif
	} else {
		debug("%s get from environment.\n", str);
		ret = 0;
	}

	*env = e;
	free(s);

	return ret;
}

/**
 * extract_val(): Extract value from a key=value pair list (comma separated).
 *                Only value for the given key is returend.
 *                Function allocates memory for the value, remember to free!
 *
 * @param str - pointer to string with key=values pairs
 * @param key - pointer to the key to search for
 *
 * @return - pointer to allocated string with the value
 */
static char *extract_val(const char *str, const char *key)
{
	char *v, *k;
	char *s, *strcopy;
	char *new = NULL;

	strcopy = strdup(str);
	if (strcopy == NULL)
		return NULL;

	s = strcopy;
	while (s) {
		v = strsep(&s, ",");
		if (!v)
			break;
		k = strsep(&v, "=");
		if (!k)
			break;
		if  (strcmp(k, key) == 0) {
			new = strdup(v);
			break;
		}
	}

	free(strcopy);

	return new;
}

/**
 * found_key(): Found key without value in parameter list (comma separated).
 *
 * @param str - pointer to string with key
 * @param key - pointer to the key to search for
 *
 * @return - true on found key
 */
static bool found_key(const char *str, const char *key)
{
	char *k;
	char *s, *strcopy;
	bool result = false;

	strcopy = strdup(str);
	if (!strcopy)
		return NULL;

	s = strcopy;
	while (s) {
		k = strsep(&s, ",");
		if (!k)
			break;
		if  (strcmp(k, key) == 0) {
			result = true;
			break;
		}
	}

	free(strcopy);

	return result;
}

static int calc_parts_list_len(int numparts)
{
	int partlistlen = UUID_STR_LEN + 1 + strlen("uuid_disk=");
	/* for the comma */
	partlistlen++;

	/* per-partition additions; numparts starts at 1, so this should be correct */
	partlistlen += numparts * (strlen("name=,") + PART_NAME_LEN + 1);
	/* see part.h for definition of struct disk_partition */
	partlistlen += numparts * (strlen("start=MiB,") + sizeof(lbaint_t) + 1);
	partlistlen += numparts * (strlen("size=MiB,") + sizeof(lbaint_t) + 1);
	partlistlen += numparts * (strlen("uuid=;") + UUID_STR_LEN + 1);
	/* for the terminating null */
	partlistlen++;
	debug("Length of partitions_list is %d for %d partitions\n", partlistlen,
	      numparts);
	return partlistlen;
}

#ifdef CONFIG_CMD_GPT_RENAME
static void del_gpt_info(void)
{
	struct list_head *pos = &disk_partitions;
	struct disk_part *curr;
	while (!list_empty(pos)) {
		curr = list_entry(pos->next, struct disk_part, list);
		list_del(pos->next);
		free(curr);
	}
}

static struct disk_part *allocate_disk_part(disk_partition_t *info, int partnum)
{
	struct disk_part *newpart;
	newpart = calloc(1, sizeof(struct disk_part));
	if (!newpart)
		return ERR_PTR(-ENOMEM);

	newpart->gpt_part_info.start = info->start;
	newpart->gpt_part_info.size = info->size;
	newpart->gpt_part_info.blksz = info->blksz;
	strncpy((char *)newpart->gpt_part_info.name, (const char *)info->name,
		PART_NAME_LEN);
	newpart->gpt_part_info.name[PART_NAME_LEN - 1] = '\0';
	strncpy((char *)newpart->gpt_part_info.type, (const char *)info->type,
		PART_TYPE_LEN);
	newpart->gpt_part_info.type[PART_TYPE_LEN - 1] = '\0';
	newpart->gpt_part_info.bootable = info->bootable;
	newpart->gpt_part_info.system = info->system;
	newpart->gpt_part_info.hideefi = info->hideefi;
	newpart->gpt_part_info.ro = info->ro;
	newpart->gpt_part_info.hide = info->hide;
	newpart->gpt_part_info.nomount = info->nomount;
#ifdef CONFIG_PARTITION_UUIDS
	strncpy(newpart->gpt_part_info.uuid, (const char *)info->uuid,
		UUID_STR_LEN);
	/* UUID_STR_LEN is correct, as uuid[]'s length is UUID_STR_LEN+1 chars */
	newpart->gpt_part_info.uuid[UUID_STR_LEN] = '\0';
#endif
	newpart->partnum = partnum;

	return newpart;
}

static void prettyprint_part_size(char *sizestr, lbaint_t partsize,
				  lbaint_t blksize)
{
	unsigned long long partbytes, partkilobytes;

	partbytes = partsize * blksize;
	partkilobytes = lldiv(partbytes, SZ_1K);
	snprintf(sizestr, 16, "%lluKB", partkilobytes);
}

static void print_gpt_info(void)
{
	struct list_head *pos;
	struct disk_part *curr;
	char partstartstr[16];
	char partsizestr[16];

	list_for_each(pos, &disk_partitions) {
		curr = list_entry(pos, struct disk_part, list);
		prettyprint_part_size(partstartstr, curr->gpt_part_info.start,
				      curr->gpt_part_info.blksz);
		prettyprint_part_size(partsizestr, curr->gpt_part_info.size,
				      curr->gpt_part_info.blksz);

		printf("Partition %d:\n", curr->partnum);
		printf("Start %s, size %s\n", partstartstr, partsizestr);
		printf("Block size %lu, name %s\n", curr->gpt_part_info.blksz,
		       curr->gpt_part_info.name);
		printf("Type %s, Attribute :", curr->gpt_part_info.type);

		if (curr->gpt_part_info.system)
			printf(" system=%d ",curr->gpt_part_info.system);

		if (curr->gpt_part_info.hideefi)
			printf(" hideefi=%d ",curr->gpt_part_info.hideefi);

		if (curr->gpt_part_info.bootable)
			printf(" bootable=%d ",curr->gpt_part_info.bootable);

		if (curr->gpt_part_info.ro)
			printf(" ro=%d ",curr->gpt_part_info.ro);

		if (curr->gpt_part_info.hide)
			printf(" hide=%d ",curr->gpt_part_info.hide);

		if (curr->gpt_part_info.nomount)
			printf(" nomount=%d",curr->gpt_part_info.nomount);

		printf("\n");

#ifdef CONFIG_PARTITION_UUIDS
		printf("UUID %s\n", curr->gpt_part_info.uuid);
#endif
		printf("\n");
	}
}

static int translate_gpt_info(char *par_info, int max_par_str_len, int one_par_str_len)        // TODO : this part will move to Mediatek-internal folder
{
	char name_char[NAME_CHAR_LEN] = "name=";
	char size_char[SIZE_CHAR_LEN] = "size=";
	char comma_char[COMMA_CHAR_LEN] = ",";
	char separate_char[SEPARATE_CHAR_LEN] = ";";
	struct list_head *pos;
	struct disk_part *curr;
	char partsizestr[16];
	unsigned int curr_str_par_len = 0;

	list_for_each(pos, &disk_partitions) {
		curr = list_entry(pos, struct disk_part, list);
		prettyprint_part_size(partsizestr, curr->gpt_part_info.size,
				      curr->gpt_part_info.blksz);

		debug("Partition %d: name=%s,size=%s\n", curr->partnum,
			   curr->gpt_part_info.name,partsizestr);

		curr_str_par_len = strnlen(name_char,sizeof(name_char)-1) + strnlen((const char *)curr->gpt_part_info.name,one_par_str_len)
							+strnlen(comma_char,sizeof(comma_char)-1)
							+strnlen(size_char,sizeof(size_char)-1) + strnlen(partsizestr,one_par_str_len)
							+strnlen(separate_char,sizeof(separate_char)-1);

		if (curr_str_par_len >= one_par_str_len){
			printf("[ERROR] Partition %d: name=%s,size=%s length exceeds %d\n",
							curr->partnum,curr->gpt_part_info.name,partsizestr,one_par_str_len);
			return -1;
		}
		else{
			if (curr_str_par_len + strnlen(par_info,max_par_str_len) >= max_par_str_len){
				printf("[ERROR] After add partition %d: name=%s,size=%s, all partitions length exceed %d\n",
								curr->partnum,curr->gpt_part_info.name,partsizestr,max_par_str_len);
				return -1;
			}
			else{
				strncat(par_info, name_char, sizeof(name_char)-1);
				strncat(par_info, (const char *)curr->gpt_part_info.name, one_par_str_len);
				strncat(par_info, comma_char, sizeof(comma_char)-1);
				strncat(par_info, size_char, sizeof(size_char)-1);
				strncat(par_info, partsizestr, one_par_str_len);
				strncat(par_info, separate_char, sizeof(separate_char)-1);
			}
		}
		curr_str_par_len = 0;
	}

	return 0;
}


/*
 * create the string that upstream 'gpt write' command will accept as an
 * argument
 *
 * From doc/README.gpt, Format of partitions layout:
 *    "uuid_disk=...;name=u-boot,size=60MiB,uuid=...;
 *	name=kernel,size=60MiB,uuid=...;"
 * The fields 'name' and 'size' are mandatory for every partition.
 * The field 'start' is optional. The fields 'uuid' and 'uuid_disk'
 * are optional if CONFIG_RANDOM_UUID is enabled.
 */
static int create_gpt_partitions_list(int numparts, const char *guid,
				      char *partitions_list)
{
	struct list_head *pos;
	struct disk_part *curr;
	char partstr[PART_NAME_LEN + 1];

	if (!partitions_list)
		return -EINVAL;

	strcpy(partitions_list, "uuid_disk=");
	strncat(partitions_list, guid, UUID_STR_LEN + 1);
	strcat(partitions_list, ";");

	list_for_each(pos, &disk_partitions) {
		curr = list_entry(pos, struct disk_part, list);
		strcat(partitions_list, "name=");
		strncat(partitions_list, (const char *)curr->gpt_part_info.name,
			PART_NAME_LEN + 1);
		sprintf(partstr, ",start=0x%llx",
			(unsigned long long)curr->gpt_part_info.start *
					    curr->gpt_part_info.blksz);
		/* one extra byte for NULL */
		strncat(partitions_list, partstr, PART_NAME_LEN + 1);
		sprintf(partstr, ",size=0x%llx",
			(unsigned long long)curr->gpt_part_info.size *
					    curr->gpt_part_info.blksz);
		strncat(partitions_list, partstr, PART_NAME_LEN + 1);

		strcat(partitions_list, ",uuid=");
		strncat(partitions_list, curr->gpt_part_info.uuid,
			UUID_STR_LEN + 1);
		strcat(partitions_list, ";");
	}
	return 0;
}

/*
 * read partition info into disk_partitions list where
 * it can be printed or modified
 */
static int get_gpt_info(struct blk_desc *dev_desc)
{
	/* start partition numbering at 1, as U-Boot does */
	int valid_parts = 0, p, ret;
	disk_partition_t info;
	struct disk_part *new_disk_part;

	/*
	 * Always re-read partition info from device, in case
	 * it has changed
	 */
	INIT_LIST_HEAD(&disk_partitions);

	for (p = 1; p <= MAX_SEARCH_PARTITIONS; p++) {
		ret = part_get_info(dev_desc, p, &info);
		if (ret)
			continue;

		/* Add 1 here because counter is zero-based but p1 is
		   the first partition */
		new_disk_part = allocate_disk_part(&info, valid_parts+1);
		if (IS_ERR(new_disk_part))
			goto out;

		list_add_tail(&new_disk_part->list, &disk_partitions);
		valid_parts++;
	}
	if (valid_parts == 0) {
		printf("** No valid partitions found **\n");
		goto out;
	}
	return valid_parts;
 out:
	if (valid_parts >= 1)
		del_gpt_info();
	return -ENODEV;
}

/* a wrapper to test get_gpt_info */
static int do_get_gpt_info(struct blk_desc *dev_desc)
{
	int ret;

	ret = get_gpt_info(dev_desc);
	if (ret > 0) {
		print_gpt_info();
		del_gpt_info();
		return 0;
	}
	return ret;
}
#endif

/**
 * set_gpt_info(): Fill partition information from string
 *		function allocates memory, remember to free!
 *
 * @param dev_desc - pointer block device descriptor
 * @param str_part - pointer to string with partition information
 * @param str_disk_guid - pointer to pointer to allocated string with disk guid
 * @param partitions - pointer to pointer to allocated partitions array
 * @param parts_count - number of partitions
 *
 * @return - zero on success, otherwise error
 *
 */
static int set_gpt_info(struct blk_desc *dev_desc,
			const char *str_part,
			char **str_disk_guid,
			disk_partition_t **partitions,
			u8 *parts_count)
{
	char *tok, *str, *s;
	int i;
	char *val, *p;
	int p_count;
	disk_partition_t *parts;
	int errno = 0;
	uint64_t size_ll, start_ll;
	lbaint_t offset = 0;
	int max_str_part = calc_parts_list_len(MAX_SEARCH_PARTITIONS);

	debug("%s:  lba num: 0x%x %d\n", __func__,
	      (unsigned int)dev_desc->lba, (unsigned int)dev_desc->lba);

	if (str_part == NULL)
		return -1;

	str = strdup(str_part);
	if (str == NULL)
		return -ENOMEM;

	/* extract disk guid */
	s = str;
	val = extract_val(str, "uuid_disk");
	if (!val) {
#ifdef CONFIG_RANDOM_UUID
		*str_disk_guid = malloc(UUID_STR_LEN + 1);
		if (*str_disk_guid == NULL)
			return -ENOMEM;
		gen_rand_uuid_str(*str_disk_guid, UUID_STR_FORMAT_STD);
#else
		free(str);
		return -2;
#endif
	} else {
		val = strsep(&val, ";");
		if (extract_env(val, &p))
			p = val;
		*str_disk_guid = strdup(p);
		free(val);
		/* Move s to first partition */
		strsep(&s, ";");
	}
	if (s == NULL) {
		printf("Error: is the partitions string NULL-terminated?\n");
		return -EINVAL;
	}
	if (strnlen(s, max_str_part) == 0)
		return -3;

	i = strnlen(s, max_str_part) - 1;
	if (s[i] == ';')
		s[i] = '\0';

	/* calculate expected number of partitions */
	p_count = 1;
	p = s;
	while (*p) {
		if (*p++ == ';')
			p_count++;
	}

	/* allocate memory for partitions */
	parts = calloc(sizeof(disk_partition_t), p_count);
	if (parts == NULL)
		return -ENOMEM;

	/* retrieve partitions data from string */
	for (i = 0; i < p_count; i++) {
		tok = strsep(&s, ";");

		if (tok == NULL)
			break;

		/* uuid */
		val = extract_val(tok, "uuid");
		if (!val) {
			/* 'uuid' is optional if random uuid's are enabled */
#ifdef CONFIG_RANDOM_UUID
			gen_rand_uuid_str(parts[i].uuid, UUID_STR_FORMAT_STD);
#else
			errno = -4;
			goto err;
#endif
		} else {
			if (extract_env(val, &p))
				p = val;
			if (strnlen(p, max_str_part) >= sizeof(parts[i].uuid)) {
				printf("Wrong uuid format for partition %d\n", i);
				errno = -4;
				goto err;
			}
			strncpy((char *)parts[i].uuid, p, max_str_part);
			free(val);
		}
#ifdef CONFIG_PARTITION_TYPE_GUID
		/* guid */
		val = extract_val(tok, "type");
		if (val) {
			/* 'type' is optional */
			if (extract_env(val, &p))
				p = val;
			if (strnlen(p, max_str_part) >= sizeof(parts[i].type_guid)) {
				printf("Wrong type guid format for partition %d\n",
				       i);
				errno = -4;
				goto err;
			}
			strncpy((char *)parts[i].type_guid, p, max_str_part);
			free(val);
		}
#endif
		/* name */
		val = extract_val(tok, "name");
		if (!val) { /* name is mandatory */
			errno = -4;
			goto err;
		}
		if (extract_env(val, &p))
			p = val;
		if (strnlen(p, max_str_part) >= sizeof(parts[i].name)) {
			errno = -4;
			goto err;
		}
		strncpy((char *)parts[i].name, p, max_str_part);
		free(val);

		/* size */
		val = extract_val(tok, "size");
		if (!val) { /* 'size' is mandatory */
			errno = -4;
			goto err;
		}
		if (extract_env(val, &p))
			p = val;
		if ((strcmp(p, "-") == 0)) {
			/* Let part efi module to auto extend the size */
			parts[i].size = 0;
		} else {
			size_ll = ustrtoull(p, &p, 0);
			parts[i].size = lldiv(size_ll, dev_desc->blksz);
		}

		free(val);

		/* start address */
		val = extract_val(tok, "start");
		if (val) { /* start address is optional */
			if (extract_env(val, &p))
				p = val;
			start_ll = ustrtoull(p, &p, 0);
			parts[i].start = lldiv(start_ll, dev_desc->blksz);
			free(val);
		}

		offset += parts[i].size + parts[i].start;

		/* bootable */
		if (found_key(tok, "bootable"))
			parts[i].bootable = 1;

		/* system */
		if (found_key(tok, "system"))
			parts[i].system = 1;

		/* hideefi */
		if (found_key(tok, "hideefi"))
			parts[i].hideefi = 1;

		/* ro */
		if (found_key(tok, "ro"))
			parts[i].ro = 1;

		/* hide */
		if (found_key(tok, "hide"))
			parts[i].hide = 1;

		/* nomount */
		if (found_key(tok, "nomount"))
			parts[i].nomount = 1;
	}

	*parts_count = p_count;
	*partitions = parts;
	free(str);

	return 0;
err:
	free(str);
	free(*str_disk_guid);
	free(parts);

	return errno;
}

static int gpt_default(struct blk_desc *blk_dev_desc, const char *str_part)
{
	int ret, part_ret;
	char *str_disk_guid;
	u8 part_count = 0;
	disk_partition_t *partitions = NULL;

	/* fill partitions */
	ret = set_gpt_info(blk_dev_desc, str_part,
			&str_disk_guid, &partitions, &part_count);
	if (ret) {
		if (ret == -1)
			printf("No partition list provided\n");
		if (ret == -2)
			printf("Missing disk guid\n");
		if ((ret == -3) || (ret == -4)){
			printf("Partition list incomplete\n");
			printf("Command format should be : gpt create mmc 0 name=tvconfig,size=32M\n");
		}
		return -1;
	}

	/* save partitions layout to disk */
	ret = gpt_restore(blk_dev_desc, str_disk_guid, partitions, part_count);
	if (blk_dev_desc->if_type != IF_TYPE_USB) {
		part_ret = part_info_del(blk_dev_desc);
		if (part_ret == 0) {
			part_ret = part_info_mapping_list_scan(blk_dev_desc);
			if (part_ret == -1)
				printf("Part list re-scan failure\n");
		} else {
			printf("Part list delete failure\n");
		}
	}
	free(str_disk_guid);
	free(partitions);

	return ret;
}

static int gpt_create_default_partitions(struct blk_desc *blk_dev_desc, const char *str_interface_type, const char *str_device_number)        // TODO : this part will move to Mediatek-internal folder
{
	char par_info[DEFAULT_PAR_STR_LEN];
	int ret_default = 1;

	memset(par_info, 0, DEFAULT_PAR_STR_LEN*sizeof(char));

	/* Cat default partition string to par_info */
	if(blk_dev_desc->if_type == IF_TYPE_USB)
	{
		if (strlen(usb_default_par_info) > (DEFAULT_PAR_STR_LEN - 1)) {
			printf("[ERROR] usb_default_par_info length exceed %d\n", DEFAULT_PAR_STR_LEN);
			return -1;
		}
		strncat(par_info, usb_default_par_info, (DEFAULT_PAR_STR_LEN-1));
	}
	else
	{
		if (strlen(default_par_info) > (DEFAULT_PAR_STR_LEN - 1)) {
			printf("[ERROR] usb_default_par_info length exceed %d\n", DEFAULT_PAR_STR_LEN);
			return -1;
		}
		strncat(par_info, default_par_info, (DEFAULT_PAR_STR_LEN - 1));
	}

	/* Generate default partitions */
	ret_default = gpt_default(blk_dev_desc, par_info);
	if (ret_default != 0){
		printf("[ERROR] Default partitions create fail!\n");
		return -1;
	}
	else
		printf("Default partitions generated success!\n");


	/* Convert 8MB uenv partition to EXT4 formate (partition size&name can't be revised) */
	if(format_uenv_to_ext4(str_interface_type, str_device_number) != CMD_RET_SUCCESS)
	{
		printf("[ERROR] Convert 8MB uenv partition to EXT4 format fail!\n");
		return -1;
	}

	return 0;
}

static int gpt_create_partition(struct blk_desc *blk_dev_desc, const char *str_interface_type, const char *str_device_number, const char *str_part)        // TODO : this part will move to Mediatek-internal folder
{
	int part_num = -1;
	int ret_default = 1;
	char par_info[MAX_PAR_STR_LEN];
	char par_info_add[ONE_PAR_STR_LEN];

	memset(par_info, 0, MAX_PAR_STR_LEN*sizeof(char));
	memset(par_info_add, 0, ONE_PAR_STR_LEN*sizeof(char));

	/* Check GPT had been generated originally? */
	part_num = get_gpt_info(blk_dev_desc);

	/* If no GPT originally */
	if (part_num < 0){

		printf("There are no GPT originally!\n");

		/* Create default partitions */
		if (gpt_create_default_partitions(blk_dev_desc, str_interface_type, str_device_number) != 0)
			return -1;

		/* Becuause you generated default partitions, you should get partitions info. again */
		part_num = get_gpt_info(blk_dev_desc);
		if (part_num < 0){
			printf("[ERROR] Create default partitions fail!\n");
			return -1;
		}
	}

	/* Get original partitions information */
	if (translate_gpt_info(par_info,sizeof(par_info),sizeof(par_info_add)) != 0){
		printf("[ERROR] Get original partitions information fail!!\n");
		return -1;
	}
	else{
		debug("Original partitions : %s\n",par_info);
	}

	/* Add new partition information - make sure new partition length is not too long */
	if (strnlen(str_part,ONE_PAR_STR_LEN) == ONE_PAR_STR_LEN){
		printf("[ERROR] New partition length exceeds %d\n",ONE_PAR_STR_LEN);
		return -1;
	}
	else{
		strncat(par_info_add,str_part,ONE_PAR_STR_LEN);
		debug("Create new partition : %s\n",par_info_add);
	}

	/* Add new partition information - make sure all partitions length are not too long */
	if (strnlen(par_info,MAX_PAR_STR_LEN) + strnlen(par_info_add,ONE_PAR_STR_LEN) >= MAX_PAR_STR_LEN){
		printf("[ERROR] All partitions length exceed %d\n",MAX_PAR_STR_LEN);
		return -1;
	}
	else{
		strncat(par_info,par_info_add,ONE_PAR_STR_LEN);
		debug("All partitions : %s\n",par_info);
	}

	/* Generate GPT */
	ret_default = gpt_default(blk_dev_desc, par_info);
	if (ret_default != 0)
		return -1;
	else
		printf("GPT generated success!\n");

	del_gpt_info();

	return 0;
}

static int gpt_create_adaptive_partition(struct blk_desc *blk_dev_desc, const char *str_interface_type, const char *str_device_number, const char *str_part_name, const char *str_align_size)        // TODO : this part will move to Mediatek-internal folder
{
	char par_info_add[ONE_PAR_STR_LEN];
	int part_num = -1;
	disk_partition_t last_par_info;
	lbaint_t last_usable_lba, adap_par_start_lba, adap_par_size_lba;
	uint64_t adap_par_size, align_size;
	int snprintf_len;

	/* Check GPT had been generated originally? */
	part_num = get_gpt_info(blk_dev_desc);

	/* If no GPT originally */
	if (part_num < 0)
	{
		printf("[ERROR] Please create default partition first!\n");
		return -1;
	}

	/* If there are GPT originally, check default partition is existed or not */
	else
	{
		debug("There are %d partitions originally\n", part_num);

		if (part_get_info(blk_dev_desc, part_num, &last_par_info) != 0)
		{
			printf("[ERROR] Get last partition(%d) info fail!\n", part_num);
			return -1;
		}

		debug("Last partition (%d: %s) start:0x%lx LBA, size:0x%lx LBA, end:0x%lx LBA\n",
			part_num, last_par_info.name, last_par_info.start, last_par_info.size, (last_par_info.start + last_par_info.size - 1));

		last_usable_lba = (blk_dev_desc->lba - 34);
		debug("GPT last usable LBA is 0x%lx\n", last_usable_lba);

		adap_par_start_lba = last_par_info.start + last_par_info.size;
		adap_par_size_lba =  last_usable_lba - adap_par_start_lba + 1;
		adap_par_size = adap_par_size_lba * blk_dev_desc->blksz;
		debug("Adaptive partition (%d: %s) start:0x%lx LBA, max size:0x%lx LBA, end:0x%lx LBA\n",
			part_num+1, str_part_name, adap_par_start_lba, adap_par_size_lba, (adap_par_start_lba + adap_par_size_lba - 1));

		if ((strcmp(str_align_size, "-") == 0))
		{
			/* Let part efi module to auto extend the size */
			debug("'%s' means no need to align size, use max size:0x%lx LBA (0x%llx bytes)\n",
				str_align_size, adap_par_size_lba, adap_par_size);

			adap_par_size = 0;
		}
		else
		{
			align_size = ustrtoull(str_align_size, (char **)&str_align_size, 0);
			adap_par_size = adap_par_size - (adap_par_size % align_size);

			if (adap_par_size == 0)
			{
				printf("[ERROR] No enough space to create adaptive partition! (remaining size 0x%lx bytes, align size 0x%llx bytes)\n",
					(adap_par_size_lba * blk_dev_desc->blksz), align_size);
				return -1;
			}

			debug("Align 0x%llx, adaptive partition re-size to 0x%llx bytes\n", align_size, adap_par_size);
		}

		memset(par_info_add, 0, ONE_PAR_STR_LEN*sizeof(char));
		snprintf_len = snprintf(par_info_add, sizeof(par_info_add)-1, "name=%s,size=0x%llx", str_part_name, adap_par_size);
		if (snprintf_len >= sizeof(par_info_add)-1)
		{
			printf("[ERROR] The array size is too small(%d), snprintf fail '%s'\n", (int)(sizeof(par_info_add)-1), par_info_add);
			return -1;
		}
		debug("Adaptive partition info '%s'\n", par_info_add);

		if (gpt_create_partition(blk_dev_desc, str_interface_type, str_device_number, par_info_add) != 0)
		{
			printf("[ERROR] Create adaptive partition fail!\n");
			return -1;
		}

		return 0;
    }
}

static int gpt_clean(struct blk_desc *blk_dev_desc, const char *str_interface_type, const char *str_device_number)        // TODO : this part will move to Mediatek-internal folder
{
	int i;
	ALLOC_ALIGN_BUFFER(char, buf, blk_dev_desc->blksz, blk_dev_desc->blksz);

	memset(buf, 0, blk_dev_desc->blksz);
	//write block 0~33
	for(i = 0; i < 34; i ++)
		if (blk_dwrite(blk_dev_desc, i, 1, buf) != 1)
			return 1;
	//write block last one to last 33
	for(i = blk_dev_desc->lba - 33; i < blk_dev_desc->lba; i++)
		if (blk_dwrite(blk_dev_desc, i, 1, buf) != 1)
			return 1;

	printf("Clean all partitions success!\n");

	/* Create default partitions */
	if (gpt_create_default_partitions(blk_dev_desc, str_interface_type, str_device_number) != 0)
		return -1;

	return 0;
}

static int gpt_remove_partition(struct blk_desc *blk_dev_desc, const char *str_interface_type, const char *str_device_number, const char *str_part_name)        // TODO : this part will move to Mediatek-internal folder
{
	disk_partition_t last_part_info;
	char par_info[MAX_PAR_STR_LEN];
	char par_info_rm[ONE_PAR_STR_LEN];
	char *rm_par_pos = NULL;
	int part_num = -1;
	int ret = 0;
	int i = 0;

	memset(par_info, 0, MAX_PAR_STR_LEN * sizeof(char));
	memset(par_info_rm, 0, ONE_PAR_STR_LEN * sizeof(char));

	/* Check GPT had been generated originally */
	part_num = get_gpt_info(blk_dev_desc);
	if(part_num < 0)
	{
		printf("[ERROR] There is no GPT originally!\n");
		return -1;
	}

	/* Check remove partition is the last partition */
	for(i = 1; i < MAX_SEARCH_PARTITIONS; i++)
	{
		ret = part_get_info(blk_dev_desc, i, &last_part_info);
		if(ret != 0)
			break;
	}
	ret = part_get_info(blk_dev_desc, i - 1, &last_part_info);
	if(ret != 0)
	{
		printf("[ERROR] Get partition info fail!\n");
		return -1;
	}
	if(strncmp((char *)last_part_info.name, str_part_name, strlen((char *)last_part_info.name)) != 0)
	{
		printf("[ERROR] '%s' is the last partition, please check first!\n", last_part_info.name);
		return -1;
	}

	/* Check the last partition is not default partition */
	if((strncmp((char *)last_part_info.name, part_mboot_a, strlen(part_mboot_a)) == 0) || \
		(strncmp((char *)last_part_info.name, part_mboot_b, strlen(part_mboot_b)) == 0) || \
		(strncmp((char *)last_part_info.name, part_uenv, strlen(part_uenv)) == 0))
	{
		printf("[ERROR] Can not remove default partition of '%s'!\n", last_part_info.name);
		return -1;
	}

	/* Get original partitions information */
	if(translate_gpt_info(par_info, sizeof(par_info), sizeof(par_info_rm)) != 0)
	{
		printf("[ERROR] Get original partitions information fail!\n");
		return -1;
	}
	else
		debug("Original partitions: %s\n", par_info);


	/* Locate partition and Remove partition */
	ret = snprintf(par_info_rm, ONE_PAR_STR_LEN, "name=%s,size=%luKB", last_part_info.name, last_part_info.size * last_part_info.blksz / SZ_1K);
	if(ret < 0)
	{
		printf("[ERROR] Fail to snprintf '%s'\n", par_info_rm);
		return -1;
	}
	if(ret >= ONE_PAR_STR_LEN)
	{
		printf("[ERROR] The array size is too small(%u), snprintf fail '%s'\n", ONE_PAR_STR_LEN, par_info_rm);
		return -1;
	}
	debug("Remove partition: %s\n", par_info_rm);
	rm_par_pos = strstr(par_info, par_info_rm);
	if(rm_par_pos == NULL)
	{
		printf("[ERROR] Partition '%s' is not exist!\n", par_info_rm);
		printf("All partitions: %s\n", par_info);
		return -1;
	}
	else
	{
		*rm_par_pos = '\0';
		debug("New partitions: %s\n", par_info);
	}

	/* Regenerate GPT */
	ret = gpt_default(blk_dev_desc, par_info);
	if(ret != 0)
	{
		printf("[ERROR] Regenerate GPT fail!\n");
		return -1;
	}
	else
		printf("Remove partition '%s' success!\n", str_part_name);

	del_gpt_info();

	return 0;
}

static int gpt_get_default_par_name(struct blk_desc *blk_dev_desc, char *default_par_name, int *default_par_cnt)        // TODO : this part will move to Mediatek-internal folder
{
	char comma_char[COMMA_CHAR_LEN] = ",";
	char par_info[DEFAULT_PAR_STR_LEN];
	char *s, *tok, *val, *p;
	int i, cnt_max, cnt;

	memset(par_info, 0, DEFAULT_PAR_STR_LEN*sizeof(char));


	/* Cat default partition string to par_info */
	if(blk_dev_desc->if_type == IF_TYPE_USB)
	{
		if (strlen(usb_default_par_info) > (DEFAULT_PAR_STR_LEN - 1)) {
			debug("[ERROR] usb_default_par_info length exceed %d\n", DEFAULT_PAR_STR_LEN);
			return -1;
		}
		strncat(par_info, usb_default_par_info, (DEFAULT_PAR_STR_LEN-1));
	}
	else
	{
		if (strlen(default_par_info) > (DEFAULT_PAR_STR_LEN - 1)) {
			debug("[ERROR] usb_default_par_info length exceed %d\n", DEFAULT_PAR_STR_LEN);
			return -1;
		}
		strncat(par_info, default_par_info, (DEFAULT_PAR_STR_LEN - 1));
	}

	/* Remove the last ';' in default partition, to avoid cnt_max calculating fail */
	i = strnlen(par_info, DEFAULT_PAR_STR_LEN) - 1;
	if ((i >= 0) && (par_info[i] == ';'))
		par_info[i] = '\0';


	/* Calculate expected number of partitions */
	cnt_max = 1;
	p = par_info;
	while (*p) {
		if (*p++ == ';')
			cnt_max++;
	}


	/* Start to get all partition name in default partition */
	s = par_info;
	for(cnt=1; cnt<=cnt_max ; cnt++)
	{
		/* Use ';' to get target string in default partition */
		tok = strsep(&s, ";");
		if (tok == NULL)
		{
			debug("target string can NOT find ';', end search!\n\n");
			break;
		}

		/* Extract partition name */
		val = extract_val(tok, "name");
		if (!val)
		{
			debug("target string can NOT find 'name', end search!\n\n");
			break;
		}

		*default_par_cnt = cnt;
		strncat(default_par_name, val, DEFAULT_PAR_STR_LEN-1);
		strncat(default_par_name, comma_char, sizeof(comma_char)-1);
		debug("[%d/%d] default partition name = '%s'\n", *default_par_cnt, cnt_max, default_par_name);

		free(val);
	}

	return 0;
}

static int gpt_get_current_par_name(struct blk_desc *blk_dev_desc, char *current_par_name, int default_par_cnt)        // TODO : this part will move to Mediatek-internal folder
{
	int current_par_cnt = 0;
	char comma_char[COMMA_CHAR_LEN] = ",";
	struct list_head *pos;
	struct disk_part *curr;


	/* Start to get all partition name in current partition */
	list_for_each(pos, &disk_partitions) {
		if (current_par_cnt >= default_par_cnt)
		{
			debug("End the process of getting current partition name.\n");
			return 0;
		}
		curr = list_entry(pos, struct disk_part, list);

//		debug("Current partition %d: name=%s\n", curr->partnum, curr->gpt_part_info.name);

		current_par_cnt = current_par_cnt + 1;
		strncat(current_par_name, (const char *)curr->gpt_part_info.name, PART_NAME_LEN);
		strncat(current_par_name, comma_char, sizeof(comma_char)-1);
		debug("[%d/%d] current partition name = '%s'\n", current_par_cnt, default_par_cnt, current_par_name);
	}

	if (current_par_cnt < default_par_cnt)
	{
		debug("There are only %d partitions on flash!\n\n", current_par_cnt);
	}

	return 0;
}

static int gpt_check_default_par_exist(struct blk_desc *blk_dev_desc)        // TODO : this part will move to Mediatek-internal folder
{
	int default_par_cnt = 0;
	char default_par_name[DEFAULT_PAR_STR_LEN];
	char current_par_name[DEFAULT_PAR_STR_LEN];

	memset(default_par_name, 0, DEFAULT_PAR_STR_LEN*sizeof(char));
	memset(current_par_name, 0, DEFAULT_PAR_STR_LEN*sizeof(char));

	/* get default partition name info. */
	if (gpt_get_default_par_name(blk_dev_desc, default_par_name, &default_par_cnt) != 0)
	{
		printf("[ERROR] Get default partitions name fail!\n");
		return -1;
	}

	/* get current partition name info. */
	if (gpt_get_current_par_name(blk_dev_desc, current_par_name, default_par_cnt) != 0)
	{
		printf("[ERROR] Get current partitions name fail!\n");
		return -1;
	}

	/* check if default partition & current partition are the same */
	if (strncmp(default_par_name, current_par_name, DEFAULT_PAR_STR_LEN) != 0)
	{
		printf("\n[ERROR] Current partition does NOT match default partition info!!\n");
		return -1;
	}
	else
	{
		debug("\nCurrent partition include default partition.\n");
		return 0;
	}
}

static int gpt_check(struct blk_desc *blk_dev_desc, const char *str_interface_type, const char *str_device_number)        // TODO : this part will move to Mediatek-internal folder
{
	int part_num = -1;

	/* Check GPT had been generated originally? */
	part_num = get_gpt_info(blk_dev_desc);

	/* If no GPT originally */
	if (part_num < 0){
		printf("There are no GPT originally!\n");

		/* Create default partitions */
		if (gpt_create_default_partitions(blk_dev_desc, str_interface_type, str_device_number) != 0)
			return -1;

		return 0;
	}

	/* If there are GPT originally, check default partition is existed or not */
	else{

		/* check if default partition & current partition are the same */
		if (gpt_check_default_par_exist(blk_dev_desc) != 0)
		{
			/* Create default partitions */
			if (gpt_create_default_partitions(blk_dev_desc, str_interface_type, str_device_number) != 0)
			{
				return -1;
			}
			else
			{
				return 0;
			}
		}

		debug("There are %d partitions originally, no need to create GPT again!\n",part_num);
		return 0;
    }
}

static int gpt_verify(struct blk_desc *blk_dev_desc, const char *str_part)
{
	ALLOC_CACHE_ALIGN_BUFFER_PAD(gpt_header, gpt_head, 1,
				     blk_dev_desc->blksz);
	disk_partition_t *partitions = NULL;
	gpt_entry *gpt_pte = NULL;
	char *str_disk_guid;
	u8 part_count = 0;
	int ret = 0;

	/* fill partitions */
	ret = set_gpt_info(blk_dev_desc, str_part,
			&str_disk_guid, &partitions, &part_count);
	if (ret) {
		if (ret == -1) {
			printf("No partition list provided - only basic check\n");
			ret = gpt_verify_headers(blk_dev_desc, gpt_head,
						 &gpt_pte);
			goto out;
		}
		if (ret == -2)
			printf("Missing disk guid\n");
		if ((ret == -3) || (ret == -4))
			printf("Partition list incomplete\n");
		return -1;
	}

	/* Check partition layout with provided pattern */
	ret = gpt_verify_partitions(blk_dev_desc, partitions, part_count,
				    gpt_head, &gpt_pte);
	free(str_disk_guid);
	free(partitions);
	if(ret == -1)
		return ret;
 out:
	free(gpt_pte);
	return ret;
}

static int do_disk_guid(struct blk_desc *dev_desc, char * const namestr)
{
	int ret;
	char disk_guid[UUID_STR_LEN + 1];

	ret = get_disk_guid(dev_desc, disk_guid);
	if (ret < 0)
		return CMD_RET_FAILURE;

	if (namestr)
		env_set(namestr, disk_guid);
	else
		printf("%s\n", disk_guid);

	return ret;
}

#ifdef CONFIG_CMD_GPT_RENAME
static int do_rename_gpt_parts(struct blk_desc *dev_desc, char *subcomm,
			       char *name1, char *name2)
{
	struct list_head *pos;
	struct disk_part *curr;
	disk_partition_t *new_partitions = NULL;
	char disk_guid[UUID_STR_LEN + 1];
	char *partitions_list, *str_disk_guid = NULL;
	u8 part_count = 0;
	int partlistlen, ret, numparts = 0, partnum, i = 1, ctr1 = 0, ctr2 = 0;

	if ((subcomm == NULL) || (name1 == NULL) || (name2 == NULL) ||
	    (strcmp(subcomm, "swap") && (strcmp(subcomm, "rename"))))
		return -EINVAL;

	ret = get_disk_guid(dev_desc, disk_guid);
	if (ret < 0)
		return ret;
	/*
	 * Allocates disk_partitions, requiring matching call to del_gpt_info()
	 * if successful.
	 */
	numparts = get_gpt_info(dev_desc);
	if (numparts <=  0)
		return numparts ? numparts : -ENODEV;

	partlistlen = calc_parts_list_len(numparts);
	partitions_list = malloc(partlistlen);
	if (!partitions_list) {
		del_gpt_info();
		return -ENOMEM;
	}
	memset(partitions_list, '\0', partlistlen);

	ret = create_gpt_partitions_list(numparts, disk_guid, partitions_list);
	if (ret < 0) {
		free(partitions_list);
		return ret;
	}
	/*
	 * Uncomment the following line to print a string that 'gpt write'
	 * or 'gpt verify' will accept as input.
	 */
	debug("OLD partitions_list is %s with %u chars\n", partitions_list,
	      (unsigned)strlen(partitions_list));

	/* set_gpt_info allocates new_partitions and str_disk_guid */
	ret = set_gpt_info(dev_desc, partitions_list, &str_disk_guid,
			   &new_partitions, &part_count);
	if(ret < 0)
		goto out;

	if (!strcmp(subcomm, "swap")) {
		if ((strlen(name1) > PART_NAME_LEN) || (strlen(name2) > PART_NAME_LEN)) {
			printf("Names longer than %d characters are truncated.\n", PART_NAME_LEN);
			ret = -EINVAL;
			goto out;
		}
		list_for_each(pos, &disk_partitions) {
			curr = list_entry(pos, struct disk_part, list);
			if (!strcmp((char *)curr->gpt_part_info.name, name1)) {
				strcpy((char *)curr->gpt_part_info.name, name2);
				ctr1++;
			} else if (!strcmp((char *)curr->gpt_part_info.name, name2)) {
				strcpy((char *)curr->gpt_part_info.name, name1);
				ctr2++;
			}
		}
		if ((ctr1 + ctr2 < 2) || (ctr1 != ctr2)) {
			printf("Cannot swap partition names except in pairs.\n");
			ret = -EINVAL;
			goto out;
		}
	} else { /* rename */
		if (strlen(name2) > PART_NAME_LEN) {
			printf("Names longer than %d characters are truncated.\n", PART_NAME_LEN);
			ret = -EINVAL;
			goto out;
		}
		partnum = (int)simple_strtol(name1, NULL, 10);
		if ((partnum < 0) || (partnum > numparts)) {
			printf("Illegal partition number %s\n", name1);
			ret = -EINVAL;
			goto out;
		}
		ret = part_get_info(dev_desc, partnum, new_partitions);
		if (ret < 0)
			goto out;

		/* U-Boot partition numbering starts at 1 */
		list_for_each(pos, &disk_partitions) {
			curr = list_entry(pos, struct disk_part, list);
			if (i == partnum) {
				strcpy((char *)curr->gpt_part_info.name, name2);
				break;
			}
			i++;
		}
	}

	ret = create_gpt_partitions_list(numparts, disk_guid, partitions_list);
	if (ret < 0)
		goto out;
	debug("NEW partitions_list is %s with %u chars\n", partitions_list,
	      (unsigned)strlen(partitions_list));

	ret = set_gpt_info(dev_desc, partitions_list, &str_disk_guid,
			   &new_partitions, &part_count);
	/*
	 * Even though valid pointers are here passed into set_gpt_info(),
	 * it mallocs again, and there's no way to tell which failed.
	 */
	if (ret < 0)
		goto out;

	debug("Writing new partition table\n");
	ret = gpt_restore(dev_desc, disk_guid, new_partitions, numparts);
	if (ret < 0) {
		printf("Writing new partition table failed\n");
		goto out;
	}

	debug("Reading back new partition table\n");
	/*
	 * Empty the existing disk_partitions list, as otherwise the memory in
	 * the original list is unreachable.
	 */
	del_gpt_info();
	numparts = get_gpt_info(dev_desc);
	if (numparts <=  0) {
		ret = numparts ? numparts : -ENODEV;
		goto out;
	}
	printf("new partition table with %d partitions is:\n", numparts);
	print_gpt_info();
 out:
	del_gpt_info();
#ifdef CONFIG_RANDOM_UUID
	if (str_disk_guid)
		free(str_disk_guid);
#endif
	if (new_partitions)
		free(new_partitions);
	free(partitions_list);
	return ret;
}
#endif

/**
 * do_gpt(): Perform GPT operations
 *
 * @param cmdtp - command name
 * @param flag
 * @param argc
 * @param argv
 *
 * @return zero on success; otherwise error
 */
static int do_gpt(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int ret = CMD_RET_SUCCESS;
	int dev = 0;
	char *ep;
	struct blk_desc *blk_dev_desc = NULL;

#ifndef CONFIG_CMD_GPT_RENAME
	if (argc < 4 || argc > 5)
#else
	if (argc < 4 || argc > 6)
#endif
		return CMD_RET_USAGE;

	dev = (int)simple_strtoul(argv[3], &ep, 10);
	if (!ep || ep[0] != '\0') {
		printf("'%s' is not a number\n", argv[3]);
		return CMD_RET_USAGE;
	}
	blk_dev_desc = blk_get_dev(argv[2], dev);
	if (!blk_dev_desc) {
		printf("%s: %s dev %d NOT available\n",
		       __func__, argv[2], dev);
		return CMD_RET_FAILURE;
	}

	if ((strcmp(argv[1], "write") == 0) && (argc == 5)) {
		printf("Writing GPT: ");
		ret = gpt_default(blk_dev_desc, argv[4]);
	} else if ((strcmp(argv[1], "create") == 0) && (argc == 5)) {
		printf("Create GPT: ");
		ret = gpt_create_partition(blk_dev_desc, argv[2], argv[3], argv[4]);
	} else if ((strcmp(argv[1], "create_adaptive") == 0) && (argc == 6)) {
		printf("Create adaptive GPT: ");
		ret = gpt_create_adaptive_partition(blk_dev_desc, argv[2], argv[3], argv[4], argv[5]);
	} else if ((strcmp(argv[1], "clean") == 0) && (argc == 4)) {
		ret = gpt_clean(blk_dev_desc, argv[2], argv[3]);
	} else if ((strcmp(argv[1], "remove") == 0) && (argc == 5)) {
		printf("Remove GPT: ");
		ret = gpt_remove_partition(blk_dev_desc, argv[2], argv[3], argv[4]);
	} else if ((strcmp(argv[1], "check") == 0) && (argc == 4)) {
		ret = gpt_check(blk_dev_desc, argv[2], argv[3]);
	} else if ((strcmp(argv[1], "verify") == 0)) {
		ret = gpt_verify(blk_dev_desc, argv[4]);
		printf("Verify GPT: ");
	} else if (strcmp(argv[1], "guid") == 0) {
		ret = do_disk_guid(blk_dev_desc, argv[4]);
#ifdef CONFIG_CMD_GPT_RENAME
	} else if (strcmp(argv[1], "read") == 0) {
		ret = do_get_gpt_info(blk_dev_desc);
	} else if ((strcmp(argv[1], "swap") == 0) ||
		   (strcmp(argv[1], "rename") == 0)) {
		ret = do_rename_gpt_parts(blk_dev_desc, argv[1], argv[4], argv[5]);
#endif
	} else {
		return CMD_RET_USAGE;
	}

	if (ret) {
		printf("error!\n");
		return CMD_RET_FAILURE;
	}

	debug("success!\n");
	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(gpt, CONFIG_SYS_MAXARGS, 1, do_gpt,
	"GUID Partition Table",
	"<command> <interface> <dev> <partitions_list>\n"
	" - GUID partition table restoration and validity check\n"
	" Restore or verify GPT information on a device connected\n"
	" to interface\n"
	" Example usage:\n"
	" gpt write <interface> <dev> $partitions\n"
	"       - only new partitions will be reserved\n"
	" gpt create <interface> <dev> $partition\n"
	"       - create a new partition after original partitions\n"
	"       - partition name & size are mandatory\n"
	"       - partition name can only be up to 31 characters\n"
	"       - unit of partition size : K, M, G (bytes)\n"
	"       - ex : gpt create mmc 0 name=tvconfig,size=32M\n"
	" gpt create_adaptive <interface> <dev> <name> <align_size>\n"
	"       - create an adaptive partition after original partitions\n"
	"       - partition size will auto extend to the end of GPT\n"
	"       - partition size will align to <align_size>\n"
	"       - partition name & align size are mandatory\n"
	"       - partition name can only be up to 31 characters\n"
	"       - unit of align size should be hexadecimal : 0x1000 (bytes)\n"
	"       - if <align_size> is '-', partition size will not do align\n"
	"       - ex : gpt create_adaptive mmc 0 userdata 0x1000\n"
	"       - ex : gpt create_adaptive mmc 0 userdata -\n"
	" gpt verify mmc 0 $partitions\n"
	" gpt read <interface> <dev>\n"
	"       - read GPT into a data structure for manipulation\n"
	"       - ex : gpt read mmc 0\n"
	" gpt guid <interface> <dev>\n"
	"       - print disk GUID\n"
	"       - ex : gpt guid mmc 0\n"
	" gpt guid <interface> <dev> <varname>\n"
	"       - set environment variable to disk GUID\n"
	"       - ex : gpt guid mmc 0 varname\n"
	" gpt clean <interface> <dev>\n"
	"       - clean GPT header & entries\n"
	"         and create default partitions at the same time.\n"
	"       - ex : gpt clean mmc 0\n"
	" gpt remove <interface> <dev> <partition>\n"
	"       - remove partition's GPT header & entry\n"
	"       - only the last partition can be remove\n"
	"       - ex : gpt remove mmc 0 userdata\n"
	" gpt check <interface> <dev>\n"
	"       - check default partitions are existing\n"
	"         if default partitions does NOT exist, create default partitions\n"
	"         if default partitions does exist, do nothing\n"
	"       - ex : gpt check mmc 0\n"
#ifdef CONFIG_CMD_GPT_RENAME
	" gpt partition renaming commands:\n"
	" gpt swap <interface> <dev> <name1> <name2>\n"
	"       - change all partitions named name1 to name2\n"
	"         and vice-versa\n"
	"       - ex : gpt swap mmc 0 foo bar\n"
	" gpt rename <interface> <dev> <part> <name>\n"
	"       - rename the specified partition\n"
	"       - ex : gpt rename mmc 0 3 foo\n"
#endif
);
