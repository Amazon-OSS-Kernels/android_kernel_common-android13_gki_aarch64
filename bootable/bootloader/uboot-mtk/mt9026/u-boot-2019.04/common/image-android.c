// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2011 Sebastian Andrzej Siewior <bigeasy@linutronix.de>
 */

#include <common.h>
#include <image.h>
#include <image-android-dt.h>
#include <android_image.h>
#include <malloc.h>
#include <errno.h>
#include <asm/unaligned.h>
#include <mapmem.h>
#if (CONFIG_MTK_ANDROID_HEADER_VERSION > 3)
#include <xbc.h>
#endif
#include <dm/ofnode.h>
#include <debug_impl.h>

#define MMAP_ENTRY_NEMBER	(4)
#define SHIFT_32_BIT	(32)
static ulong get_kernel_addr_from_mmap(void)
{
	ofnode node;
	u32 addr[MMAP_ENTRY_NEMBER];
	int ret;

	node = ofnode_path("/mmap_info/MI_KERNEL_POOL1");
	if (!ofnode_valid(node)) {
		UBOOT_ERROR("MI_KERNEL_POOL1 not found\n");
		return -ENXIO;
	}

	ret = ofnode_read_u32_array(node, "reg", addr, MMAP_ENTRY_NEMBER);
	if (ret) {
		UBOOT_ERROR("fail to parse MI_KERNEL_POOL1\n");
		return ret;
	}

	UBOOT_DEBUG("Kernel Loaded Address = %lx\n", (((ulong)addr[0] << SHIFT_32_BIT) | addr[1]));

	if(sizeof(ulong) > sizeof(uint))
		return (ulong)addr[1];

	return (((ulong)addr[0] << SHIFT_32_BIT) | addr[1]);
}

#define ANDROID_IMAGE_DEFAULT_KERNEL_ADDR	0x10008000

static char andr_tmp_str[ANDR_BOOT_ARGS_SIZE + 1];

#if (CONFIG_MTK_ANDROID_HEADER_VERSION > 2)
static ulong android_vendor_image_get_kernel_addr(const struct vendor_boot_img_hdr *vhdr)
{
	/*
	 * All the Android tools that generate a boot.img use this
	 * address as the default.
	 *
	 * Even though it doesn't really make a lot of sense, and it
	 * might be valid on some platforms, we treat that adress as
	 * the default value for this field, and try to execute the
	 * kernel in place in such a case.
	 *
	 * Otherwise, we will return the actual value set by the user.
	 */
	if (vhdr->kernel_addr == ANDROID_IMAGE_DEFAULT_KERNEL_ADDR)
		return (ulong)vhdr + vhdr->page_size;

	if (vhdr->kernel_addr <= CONFIG_BUSADDR_START)
		return (ulong)get_kernel_addr_from_mmap() + (ulong)vhdr->kernel_addr;

	return vhdr->kernel_addr;
}

ulong mtk_android_vendor_image_get_kload(void *vendor)
{
	return android_vendor_image_get_kernel_addr(vendor);
}

int mtk_android_image_get_kernel(const struct andr_img_hdr *hdr, struct vendor_boot_img_hdr *vhdr,
				int verify, ulong *os_data, ulong *os_len)
{
	u32 kernel_addr;
	const struct image_header *ihdr;
	struct boot_img_hdr *bhdr;
	bhdr = (struct boot_img_hdr *)hdr;

	kernel_addr = android_vendor_image_get_kernel_addr(vhdr);
	ihdr = (const struct image_header *)((uintptr_t)bhdr + ANDR_BOOT_IMG_PAGE_SIZE);
	strncpy(andr_tmp_str, (const char *)vhdr->name, VENDOR_BOOT_NAME_SIZE);

	/*
	 * Not all Android tools use the id field for signing the image with
	 * sha1 (or anything) so we don't check it. It is not obvious that the
	 * string is null terminated so we take care of this.
	 */
	andr_tmp_str[ANDR_BOOT_NAME_SIZE] = '\0';
	if (strlen(andr_tmp_str))
		printf("Android's image name: %s\n", andr_tmp_str);

	printf("Kernel load addr 0x%08x size %u KiB\n",
	       kernel_addr, DIV_ROUND_UP(hdr->kernel_size, 1024));

	int len = 0;

	if (*bhdr->cmdline) {
		debug("Kernel command line: %s\n", bhdr->cmdline);
		len += strlen((const char *)bhdr->cmdline);
	}

	if (*vhdr->cmdline) {
		debug("vendor command line: %s\n", vhdr->cmdline);
		len += strlen((const char *)vhdr->cmdline);
	}

	char *bootargs = env_get("bootargs");
	if (bootargs)
		len += strlen(bootargs);

	char *newbootargs = malloc(len + 4);

	if (!newbootargs) {
		puts("Error: malloc in android_image_get_kernel failed!\n");
		return -ENOMEM;
	}
	*newbootargs = '\0';

	if (bootargs) {
		strncat(newbootargs, bootargs, strlen(bootargs));
		strncat(newbootargs, " ", 1);
	}

	if (*bhdr->cmdline){
		strncat(newbootargs, (const char *)bhdr->cmdline, strlen((const char *)bhdr->cmdline));
		strncat(newbootargs, " ", 1);
	}

	if (*vhdr->cmdline)
		strncat(newbootargs, (const char *)vhdr->cmdline, strlen((const char *)vhdr->cmdline));

	env_set("bootargs", newbootargs);

	if (os_data) {
		if (image_get_magic(ihdr) == IH_MAGIC) {
			*os_data = image_get_data(ihdr);
		} else {
			*os_data = (ulong)hdr;
			*os_data += ANDR_BOOT_IMG_PAGE_SIZE;
		}
	}
	if (os_len) {
		if (image_get_magic(ihdr) == IH_MAGIC)
			*os_len = image_get_data_size(ihdr);
		else
			*os_len = hdr->kernel_size;
	}

	return 0;
}

int android_image_check_vendor_header(const struct vendor_boot_img_hdr *vhdr)
{
	return memcmp(VENDOR_BOOT_MAGIC, vhdr->magic, VENDOR_BOOT_MAGIC_SIZE);
}

int mtk_android_image_get_dtb(const struct vendor_boot_img_hdr *vhdr,
			      ulong *dtb_data, ulong *dtb_len)
{
	if (!vhdr->dtb_size) {
		*dtb_data = *dtb_len = 0;
		return -1;
	}

	*dtb_data = (unsigned long)vhdr;
	*dtb_data += ALIGN(VENDOR_BOOT_HDR_SIZE, vhdr->page_size);
	*dtb_data += ALIGN(vhdr->vendor_ramdisk_size, vhdr->page_size);

	debug("dtb address is 0x%lx\n",*dtb_data);

	*dtb_len = vhdr->dtb_size;

	return 0;
}

int mtk_android_image_get_ramdisk(const struct andr_img_hdr *hdr, void *vendor_addr,
			      ulong *rd_data, ulong *rd_len)
{
	struct boot_img_hdr *bhdr;
	struct vendor_boot_img_hdr *vhdr;

	bhdr = (struct boot_img_hdr *)hdr;
	if (!bhdr->ramdisk_size) {
		vhdr = vendor_addr;
		if(memcmp(VENDOR_BOOT_MAGIC, vhdr->magic, VENDOR_BOOT_MAGIC_SIZE) == 0){
			*rd_data = *rd_len = 0;
			return -1;
		}
#if (CONFIG_MTK_ANDROID_HEADER_VERSION > 3)
#define KILOBYTE_NUM_SIZE (1024)
		struct boot_img_hdr *ihdr;
		ihdr = vendor_addr;
		if(memcmp(ANDR_BOOT_MAGIC, ihdr->magic, ANDR_BOOT_MAGIC_SIZE) == 0){
			*rd_data = (unsigned long)vendor_addr + ANDR_BOOT_IMG_PAGE_SIZE;
			*rd_len = ihdr->ramdisk_size;
			printf("RAM disk load addr 0x%08lx size %u KiB\n",
					*rd_data, DIV_ROUND_UP(ihdr->ramdisk_size, KILOBYTE_NUM_SIZE));
			return 0;
		}
#endif
	}
	vhdr = vendor_addr;
	printf("RAM disk load addr 0x%08x size %u KiB\n",
			vhdr->ramdisk_addr, DIV_ROUND_UP(bhdr->ramdisk_size, 1024));

	*rd_data = (unsigned long)bhdr;
	*rd_data += ANDR_BOOT_IMG_PAGE_SIZE;
	*rd_data += ALIGN(bhdr->kernel_size, ANDR_BOOT_IMG_PAGE_SIZE);

	*rd_len = bhdr->ramdisk_size;

	return 0;
}

#if (CONFIG_MTK_ANDROID_HEADER_VERSION > 3)
#define BOOTCONFIG_PADDING_BYTE_SIZE	(4)
#define NEWLINE_ASSCII	(0xA)
#define HEXADECIMAL	(16)

int mtk_android_image_set_bootconfig(unsigned int version, ulong vendor_addr,
					ulong *initrd_end)
{
	char *s = NULL, *vendor_boot_config = NULL, *vendor_boot_config_ptr = NULL, *ptr = NULL, *ret_ptr = NULL;
	struct vendor_boot_img_hdr *vhdr;
	ulong initrd_high;
	ulong *bootconfig;
	int initrd_copy_to_ram = 1, bootconfig_env_size = 0, bootconfig_env_len = 0, total_len = 0, ret, len;

	s = env_get("initrd_high");
	if (s) {
		/* a value of "no" or a similar string will act like 0,
		 * turning the "load high" feature off. This is intentional.
		 */
		initrd_high = simple_strtoul(s, NULL, HEXADECIMAL);
		if (initrd_high == ~0)
			initrd_copy_to_ram = 0;
	}

	if (initrd_copy_to_ram) {
		if(version == ANDROID_HEADER_VERSION_V4) {
			vhdr = (struct vendor_boot_img_hdr *)vendor_addr;
			s = env_get("bootconfig");
			if(s != NULL) {
				bootconfig_env_size = strlen(s);
				vendor_boot_config = (char *)calloc(vhdr->bootconfig_size + bootconfig_env_size + BOOTCONFIG_PADDING_BYTE_SIZE, sizeof(char));
				if(vendor_boot_config != NULL) {
					vendor_boot_config_ptr = vendor_boot_config;
					ptr = strtok(s, " ");
					while(ptr != NULL) {
						len = strlen(ptr);
						ret_ptr = strncpy(vendor_boot_config_ptr, ptr, len);
						if(ret_ptr == NULL)
							printf("vendor_boot_config_ptr copy failure\n");
						vendor_boot_config_ptr[len] = NEWLINE_ASSCII;
						vendor_boot_config_ptr = vendor_boot_config_ptr + len + 1;
						bootconfig_env_len = bootconfig_env_len + len + 1;
						ptr = strtok(NULL, " ");
					}
				}
			} else 
				vendor_boot_config = (char *)calloc(vhdr->bootconfig_size + BOOTCONFIG_PADDING_BYTE_SIZE, sizeof(char));

			if(vendor_boot_config != NULL) {
				bootconfig = (unsigned long *)(vendor_addr + (DIV_ROUND_UP(VENDOR_BOOT_HDR_SIZE,vhdr->page_size)+
							DIV_ROUND_UP(vhdr->vendor_ramdisk_size,vhdr->page_size)+
							DIV_ROUND_UP(vhdr->dtb_size,vhdr->page_size)+
							DIV_ROUND_UP(vhdr->vendor_ramdisk_table_size,vhdr->page_size))*vhdr->page_size);

				ret_ptr = strncat(vendor_boot_config, (void *)bootconfig, vhdr->bootconfig_size);
				if(ret_ptr == NULL)
					printf("vendor_boot_config copy failure\n");

				if((vhdr->bootconfig_size + bootconfig_env_len + 1)%BOOTCONFIG_PADDING_BYTE_SIZE == 0)
					total_len = vhdr->bootconfig_size + bootconfig_env_len + 1;
				else
					total_len = vhdr->bootconfig_size + bootconfig_env_len + 1 + (BOOTCONFIG_PADDING_BYTE_SIZE - (vhdr->bootconfig_size + 1 + bootconfig_env_len)%BOOTCONFIG_PADDING_BYTE_SIZE);

				memcpy((void *)*initrd_end, (void *)vendor_boot_config, total_len);
				ret = addBootConfigTrailer((ulong)*initrd_end, total_len);
				if(ret <= 0)
					printf("Produce bootconfig data failure, Try to boot continue.\n");

				*initrd_end = *initrd_end + total_len + ret;
				free(vendor_boot_config);
			}
		}
	}

	return 0;
}
#endif
#endif

static ulong android_image_get_kernel_addr(const struct andr_img_hdr *hdr)
{
	/*
	 * All the Android tools that generate a boot.img use this
	 * address as the default.
	 *
	 * Even though it doesn't really make a lot of sense, and it
	 * might be valid on some platforms, we treat that adress as
	 * the default value for this field, and try to execute the
	 * kernel in place in such a case.
	 *
	 * Otherwise, we will return the actual value set by the user.
	 */
	if (hdr->kernel_addr == ANDROID_IMAGE_DEFAULT_KERNEL_ADDR)
		return (ulong)hdr + hdr->page_size;

	if (hdr->kernel_addr <= CONFIG_BUSADDR_START)
		return (ulong)get_kernel_addr_from_mmap() + (ulong)hdr->kernel_addr;

	return hdr->kernel_addr;
}

/**
 * android_image_get_kernel() - processes kernel part of Android boot images
 * @hdr:	Pointer to image header, which is at the start
 *			of the image.
 * @verify:	Checksum verification flag. Currently unimplemented.
 * @os_data:	Pointer to a ulong variable, will hold os data start
 *			address.
 * @os_len:	Pointer to a ulong variable, will hold os data length.
 *
 * This function returns the os image's start address and length. Also,
 * it appends the kernel command line to the bootargs env variable.
 *
 * Return: Zero, os start address and length on success,
 *		otherwise on failure.
 */
int android_image_get_kernel(const struct andr_img_hdr *hdr, int verify,
			     ulong *os_data, ulong *os_len)
{
	u32 kernel_addr = android_image_get_kernel_addr(hdr);
	const struct image_header *ihdr = (const struct image_header *)
		((uintptr_t)hdr + hdr->page_size);

	/*
	 * Not all Android tools use the id field for signing the image with
	 * sha1 (or anything) so we don't check it. It is not obvious that the
	 * string is null terminated so we take care of this.
	 */
	strncpy(andr_tmp_str, hdr->name, ANDR_BOOT_NAME_SIZE);
	andr_tmp_str[ANDR_BOOT_NAME_SIZE] = '\0';
	if (strlen(andr_tmp_str))
		printf("Android's image name: %s\n", andr_tmp_str);

	UBOOT_INFO("Kernel load addr 0x%08x size %u KiB\n",
	       kernel_addr, DIV_ROUND_UP(hdr->kernel_size, 1024));

	int len = 0;
	if (*hdr->cmdline) {
		printf("Kernel command line: %s\n", hdr->cmdline);
		len += strlen(hdr->cmdline);
	}

	char *bootargs = env_get("bootargs");
	if (bootargs)
		len += strlen(bootargs);

	char *newbootargs = malloc(len + 2);
	if (!newbootargs) {
		puts("Error: malloc in android_image_get_kernel failed!\n");
		return -ENOMEM;
	}
	*newbootargs = '\0';

	if (bootargs) {
		strcpy(newbootargs, bootargs);
		strcat(newbootargs, " ");
	}
	if (*hdr->cmdline)
		strcat(newbootargs, hdr->cmdline);

	env_set("bootargs", newbootargs);

	if (os_data) {
		if (image_get_magic(ihdr) == IH_MAGIC) {
			*os_data = image_get_data(ihdr);
		} else {
			*os_data = (ulong)hdr;
			*os_data += hdr->page_size;
		}
	}
	if (os_len) {
		if (image_get_magic(ihdr) == IH_MAGIC)
			*os_len = image_get_data_size(ihdr);
		else
			*os_len = hdr->kernel_size;
	}
	return 0;
}

int android_image_check_header(const struct andr_img_hdr *hdr)
{
	return memcmp(ANDR_BOOT_MAGIC, hdr->magic, ANDR_BOOT_MAGIC_SIZE);
}

ulong android_image_get_end(const struct andr_img_hdr *hdr)
{
	ulong end;
	/*
	 * The header takes a full page, the remaining components are aligned
	 * on page boundary
	 */
	end = (ulong)hdr;
#if (CONFIG_MTK_ANDROID_HEADER_VERSION > 2)
	if(hdr->header_version >= ANDROID_HEADER_VERSION_V3)
	{
		struct boot_img_hdr *bhdr = (struct boot_img_hdr *)hdr;
		end += ANDR_BOOT_IMG_PAGE_SIZE;
		end += ALIGN(bhdr->kernel_size, ANDR_BOOT_IMG_PAGE_SIZE);
		end += ALIGN(bhdr->ramdisk_size, ANDR_BOOT_IMG_PAGE_SIZE);
	}else{
#endif
		end += hdr->page_size;
		end += ALIGN(hdr->kernel_size, hdr->page_size);
		end += ALIGN(hdr->ramdisk_size, hdr->page_size);
		end += ALIGN(hdr->second_size, hdr->page_size);

		if (hdr->header_version >= 1)
			end += ALIGN(hdr->recovery_dtbo_size, hdr->page_size);

		if (hdr->header_version >= 2)
			end += ALIGN(hdr->dtb_size, hdr->page_size);
#if (CONFIG_MTK_ANDROID_HEADER_VERSION > 2)
	}
#endif

	return end;
}

ulong android_image_get_kload(const struct andr_img_hdr *hdr)
{
	return android_image_get_kernel_addr(hdr);
}

ulong android_image_get_kcomp(const struct andr_img_hdr *hdr)
{
	const void *p = (void *)((uintptr_t)hdr + hdr->page_size);

	if (get_unaligned_le32(p) == LZ4F_MAGIC)
		return IH_COMP_LZ4;
	else
		return IH_COMP_NONE;
}

int android_image_get_ramdisk(const struct andr_img_hdr *hdr,
			      ulong *rd_data, ulong *rd_len)
{
	if (!hdr->ramdisk_size) {
		*rd_data = *rd_len = 0;
		return -1;
	}

	UBOOT_INFO("RAM disk load addr 0x%08x size %u KiB\n",
	       hdr->ramdisk_addr, DIV_ROUND_UP(hdr->ramdisk_size, 1024));

	*rd_data = (unsigned long)hdr;
	*rd_data += hdr->page_size;
	*rd_data += ALIGN(hdr->kernel_size, hdr->page_size);

	*rd_len = hdr->ramdisk_size;
	return 0;
}

int android_image_get_second(const struct andr_img_hdr *hdr,
			      ulong *second_data, ulong *second_len)
{
	if (!hdr->second_size) {
		*second_data = *second_len = 0;
		return -1;
	}

	*second_data = (unsigned long)hdr;
	*second_data += hdr->page_size;
	*second_data += ALIGN(hdr->kernel_size, hdr->page_size);
	*second_data += ALIGN(hdr->ramdisk_size, hdr->page_size);

	printf("second address is 0x%lx\n",*second_data);

	*second_len = hdr->second_size;
	return 0;
}

/**
 * android_image_get_dtbo() - Get address and size of recovery DTBO image.
 * @hdr_addr: Boot image header address
 * @addr: If not NULL, will contain address of recovery DTBO image
 * @size: If not NULL, will contain size of recovery DTBO image
 *
 * Get the address and size of DTBO image in "Recovery DTBO" area of Android
 * Boot Image in RAM. The format of this image is Android DTBO (see
 * corresponding "DTB/DTBO Partitions" AOSP documentation for details). Once
 * the address is obtained from this function, one can use 'adtimg' U-Boot
 * command or android_dt_*() functions to extract desired DTBO blob.
 *
 * This DTBO (included in boot image) is only needed for non-A/B devices, and it
 * only can be found in recovery image. On A/B devices we can always rely on
 * "dtbo" partition. See "Including DTBO in Recovery for Non-A/B Devices" in
 * AOSP documentation for details.
 *
 * Return: true on success or false on error.
 */
bool android_image_get_dtbo(ulong hdr_addr, ulong *addr, unsigned int *size)
{
	const struct andr_img_hdr *hdr;
	ulong dtbo_img_addr;
	bool ret = true;

	hdr = map_sysmem(hdr_addr, sizeof(*hdr));
	if (android_image_check_header(hdr)) {
		printf("Error: Boot Image header is incorrect\n");
		ret = false;
		goto exit;
	}

	if (hdr->header_version < 1
#if (CONFIG_MTK_ANDROID_HEADER_VERSION > 2)
		|| hdr->header_version > 2
#endif
		) {
		printf("Error: header_version must be >= 1 to get dtbo\n");
		ret = false;
		goto exit;
	}

	if (hdr->recovery_dtbo_size == 0) {
		printf("Error: recovery_dtbo_size is 0\n");
		ret = false;
		goto exit;
	}

	/* Calculate the address of DTB area in boot image */
	dtbo_img_addr = hdr_addr;
	dtbo_img_addr += hdr->page_size;
	dtbo_img_addr += ALIGN(hdr->kernel_size, hdr->page_size);
	dtbo_img_addr += ALIGN(hdr->ramdisk_size, hdr->page_size);
	dtbo_img_addr += ALIGN(hdr->second_size, hdr->page_size);

	if (addr)
		*addr = dtbo_img_addr;
	if (size)
		*size = hdr->recovery_dtbo_size;

exit:
	unmap_sysmem(hdr);
	return ret;
}

/**
 * android_image_get_dtb_img_addr() - Get the address of DTB area in boot image.
 * @hdr_addr: Boot image header address
 * @addr: Will contain the address of DTB area in boot image
 *
 * Return: true on success or false on fail.
 */
static bool android_image_get_dtb_img_addr(ulong hdr_addr, ulong *addr)
{
	const struct andr_img_hdr *hdr;
	ulong dtb_img_addr;
	bool ret = true;

	hdr = map_sysmem(hdr_addr, sizeof(*hdr));
#if (CONFIG_MTK_ANDROID_HEADER_VERSION > 2)
	const struct vendor_boot_img_hdr *vhdr;
	vhdr = (const struct vendor_boot_img_hdr *)hdr;
#endif
	if (android_image_check_header(hdr) != 0
#if (CONFIG_MTK_ANDROID_HEADER_VERSION > 2)
	 && android_image_check_vendor_header(vhdr) != 0
#endif
	) {
		printf("Error: Boot Image header is incorrect\n");
		ret = false;
		goto exit;
	}

#if (CONFIG_MTK_ANDROID_HEADER_VERSION > 2)
	if(!android_image_check_vendor_header(vhdr)){
		if (vhdr->header_version < ANDROID_HEADER_VERSION_V2) {
			printf("Error: header_version must be >= 2 to get dtb\n");
			ret = false;
			goto exit;
		}

		if (vhdr->dtb_size == 0) {
			printf("Error: dtb_size is 0\n");
			ret = false;
			goto exit;
		}

		/* Calculate the address of DTB area in boot image */
		dtb_img_addr = hdr_addr;
		dtb_img_addr += ALIGN(vhdr->header_size, vhdr->page_size);
		dtb_img_addr += ALIGN(vhdr->vendor_ramdisk_size, vhdr->page_size);

		*addr = dtb_img_addr;
	}else if (!android_image_check_header(hdr)) {
#endif
		if (hdr->header_version < 2) {
			printf("Error: header_version must be >= 2 to get dtb\n");
			ret = false;
			goto exit;
		}

		if (hdr->dtb_size == 0) {
			printf("Error: dtb_size is 0\n");
			ret = false;
			goto exit;
		}

		/* Calculate the address of DTB area in boot image */
		dtb_img_addr = hdr_addr;
		dtb_img_addr += hdr->page_size;
		dtb_img_addr += ALIGN(hdr->kernel_size, hdr->page_size);
		dtb_img_addr += ALIGN(hdr->ramdisk_size, hdr->page_size);
		dtb_img_addr += ALIGN(hdr->second_size, hdr->page_size);
		dtb_img_addr += ALIGN(hdr->recovery_dtbo_size, hdr->page_size);
		*addr = dtb_img_addr;
#if (CONFIG_MTK_ANDROID_HEADER_VERSION > 2)
	}
#endif

exit:
	unmap_sysmem(hdr);
	return ret;
}

/**
 * android_image_get_dtb_by_index() - Get address and size of blob in DTB area.
 * @hdr_addr: Boot image header address
 * @index: Index of desired DTB in DTB area (starting from 0)
 * @addr: If not NULL, will contain address to specified DTB
 * @size: If not NULL, will contain size of specified DTB
 *
 * Get the address and size of DTB blob by its index in DTB area of Android
 * Boot Image in RAM.
 *
 * Return: true on success or false on error.
 */
bool android_image_get_dtb_by_index(ulong hdr_addr, unsigned int index, ulong *addr,
				    unsigned int *size)
{
	const struct andr_img_hdr *hdr;
	bool res;
	ulong dtb_img_addr;	/* address of DTB part in boot image */
	u32 dtb_img_size;	/* size of DTB payload in boot image */
	ulong dtb_addr;		/* address of DTB blob with specified index  */
	u32 i;			/* index iterator */

	res = android_image_get_dtb_img_addr(hdr_addr, &dtb_img_addr);
	if (!res)
		return false;

	/* Check if DTB area of boot image is in DTBO format */
	if (android_dt_check_header(dtb_img_addr)) {
		return android_dt_get_fdt_by_index(dtb_img_addr, index, addr,
						   size);
	}

	/* Find out the address of DTB with specified index in concat blobs */
	hdr = map_sysmem(hdr_addr, sizeof(*hdr));
	dtb_img_size = hdr->dtb_size;
	unmap_sysmem(hdr);
	i = 0;
	dtb_addr = dtb_img_addr;
	while (dtb_addr < dtb_img_addr + dtb_img_size) {
		const struct fdt_header *fdt;
		u32 dtb_size;

		fdt = map_sysmem(dtb_addr, sizeof(*fdt));
		if (fdt_check_header(fdt) != 0) {
			unmap_sysmem(fdt);
			printf("Error: Invalid FDT header for index %u\n", i);
			return false;
		}

		dtb_size = fdt_totalsize(fdt);
		unmap_sysmem(fdt);

		if (i == index) {
			if (size)
				*size = dtb_size;
			if (addr)
				*addr = dtb_addr;
			return true;
		}

		dtb_addr += dtb_size;
		++i;
	}

	printf("Error: Index is out of bounds (%u/%u)\n", index, i);
	return false;
}

#if !defined(CONFIG_SPL_BUILD)
/**
 * android_print_contents - prints out the contents of the Android format image
 * @hdr: pointer to the Android format image header
 *
 * android_print_contents() formats a multi line Android image contents
 * description.
 * The routine prints out Android image properties
 *
 * returns:
 *     no returned results
 */
void android_print_contents(const struct andr_img_hdr *hdr)
{
	const char * const p = IMAGE_INDENT_STRING;
	/* os_version = ver << 11 | lvl */
	u32 os_ver = hdr->os_version >> 11;
	u32 os_lvl = hdr->os_version & ((1U << 11) - 1);

	printf("%skernel size:          %x\n", p, hdr->kernel_size);
	printf("%skernel address:       %x\n", p, hdr->kernel_addr);
	printf("%sramdisk size:         %x\n", p, hdr->ramdisk_size);
	printf("%sramdisk address:      %x\n", p, hdr->ramdisk_addr);
	printf("%ssecond size:          %x\n", p, hdr->second_size);
	printf("%ssecond address:       %x\n", p, hdr->second_addr);
	printf("%stags address:         %x\n", p, hdr->tags_addr);
	printf("%spage size:            %x\n", p, hdr->page_size);
	/* ver = A << 14 | B << 7 | C         (7 bits for each of A, B, C)
	 * lvl = ((Y - 2000) & 127) << 4 | M  (7 bits for Y, 4 bits for M) */
	printf("%sos_version:           %x (ver: %u.%u.%u, level: %u.%u)\n",
	       p, hdr->os_version,
	       (os_ver >> 7) & 0x7F, (os_ver >> 14) & 0x7F, os_ver & 0x7F,
	       (os_lvl >> 4) + 2000, os_lvl & 0x0F);
	printf("%sname:                 %s\n", p, hdr->name);
	printf("%scmdline:              %s\n", p, hdr->cmdline);
	printf("%sheader_version:       %d\n", p, hdr->header_version);

	if (hdr->header_version >= 1) {
		printf("%srecovery dtbo size:   %x\n", p,
		       hdr->recovery_dtbo_size);
		printf("%srecovery dtbo offset: %llx\n", p,
		       hdr->recovery_dtbo_offset);
		printf("%sheader size:          %x\n", p,
		       hdr->header_size);
	}

	if (hdr->header_version >= 2) {
		printf("%sdtb size:             %x\n", p, hdr->dtb_size);
		printf("%sdtb addr:             %llx\n", p, hdr->dtb_addr);
	}
}

/**
 * android_image_print_dtb_info - Print info for one DTB blob in DTB area.
 * @fdt: DTB header
 * @index: Number of DTB blob in DTB area.
 *
 * Return: true on success or false on error.
 */
static bool android_image_print_dtb_info(const struct fdt_header *fdt,
					 u32 index)
{
	int root_node_off;
	u32 fdt_size;
	const char *model;
	const char *compatible;

	root_node_off = fdt_path_offset(fdt, "/");
	if (root_node_off < 0) {
		printf("Error: Root node not found\n");
		return false;
	}

	fdt_size = fdt_totalsize(fdt);
	compatible = fdt_getprop(fdt, root_node_off, "compatible",
				 NULL);
	model = fdt_getprop(fdt, root_node_off, "model", NULL);

	printf(" - DTB #%u:\n", index);
	printf("           (DTB)size = %d\n", fdt_size);
	printf("          (DTB)model = %s\n", model ? model : "(unknown)");
	printf("     (DTB)compatible = %s\n",
	       compatible ? compatible : "(unknown)");

	return true;
}

/**
 * android_image_print_dtb_contents() - Print info for DTB blobs in DTB area.
 * @hdr_addr: Boot image header address
 *
 * DTB payload in Android Boot Image v2+ can be in one of following formats:
 *   1. Concatenated DTB blobs
 *   2. Android DTBO format (see CONFIG_CMD_ADTIMG for details)
 *
 * This function does next:
 *   1. Prints out the format used in DTB area
 *   2. Iterates over all DTB blobs in DTB area and prints out the info for
 *      each blob.
 *
 * Return: true on success or false on error.
 */
bool android_image_print_dtb_contents(ulong hdr_addr)
{
	const struct andr_img_hdr *hdr;
	bool res;
	ulong dtb_img_addr;	/* address of DTB part in boot image */
	u32 dtb_img_size;	/* size of DTB payload in boot image */
	ulong dtb_addr;		/* address of DTB blob with specified index  */
	u32 i;			/* index iterator */

	res = android_image_get_dtb_img_addr(hdr_addr, &dtb_img_addr);
	if (!res)
		return false;

	/* Check if DTB area of boot image is in DTBO format */
	if (android_dt_check_header(dtb_img_addr)) {
		printf("## DTB area contents (DTBO format):\n");
		android_dt_print_contents(dtb_img_addr);
		return true;
	}

	printf("## DTB area contents (concat format):\n");

	/* Iterate over concatenated DTB blobs */
	hdr = map_sysmem(hdr_addr, sizeof(*hdr));
	dtb_img_size = hdr->dtb_size;
	unmap_sysmem(hdr);
	i = 0;
	dtb_addr = dtb_img_addr;
	while (dtb_addr < dtb_img_addr + dtb_img_size) {
		const struct fdt_header *fdt;
		u32 dtb_size;

		fdt = map_sysmem(dtb_addr, sizeof(*fdt));
		if (fdt_check_header(fdt) != 0) {
			unmap_sysmem(fdt);
			printf("Error: Invalid FDT header for index %u\n", i);
			return false;
		}

		res = android_image_print_dtb_info(fdt, i);
		if (!res) {
			unmap_sysmem(fdt);
			return false;
		}

		dtb_size = fdt_totalsize(fdt);
		unmap_sysmem(fdt);
		dtb_addr += dtb_size;
		++i;
	}

	return true;
}
#endif
