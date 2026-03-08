// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
*/

/*****************************************************************************/
#include <common.h>
#include <command.h>
#include <asm/io.h>
#include <dm/device.h>
#include <dm/uclass.h>
#include <fdt_support.h>
#include <fs.h>
#include <linux/ioport.h>
#include <remoteproc.h>
#include <time.h>
#if defined(CONFIG_LIBUFDT_OVERLAY)
#include <ufdt_overlay.h>
#endif

#include "cm4_impl.h"
#include "debug_impl.h"
#include "utility.h"
#include "partition.h"
#include "secure_boot.h"
#include "secure/crypto_auth.h"
#include "secure/secure_common.h"
#ifdef CONFIG_MULTICORES_PLATFORM
#include <smp/thread_info.h>
extern smp_spin_lock_t fs_spin_lock;
#endif

/*****************************************************************************/
#define CM4_DEVNODE_CAPABILITY		"capability"
#define CM4_FIRMWARE_PART		"bootdata"
#define CM4_FIRMWARE_PREFIX		"firmware-prefix"
#define CM4_DTBO_PATH			"dtbo-path"
#define CM4_DTB_CUS_SIZE		"dtb-size"
#define CM4_BOOT_DEV_LEN		(8)
#define CM4_PART_DEV_LEN		(8)
#define CM4_FILE_NAME_LEN		(256)
#define CM4_DUMP_DATA_LEN		(64)
#define CM4_VER_LEN			(4)
#define CM4_FW_DIV_2			(2)

#define MT5896_MAX_IMI_SIZE		(2048)
#define MT5897_MAX_IMI_SIZE		(1696)

#define VERIFY_FW_TYPE			".bin"
#define VERIFY_DTB_TYPE			".dtb"

#define REG_NAME_PM_TOP			"pm_top"

/*****************************************************************************/
// FIXME: Remove the hard code define.
// DTB section, must sync with link.ld.S
// link.ld.S file path : vendor/mediatek/tv/misdk/rtos/freertos/kernel/arch/cm4/link.ld.S
// .dtb :
// {
//     . = ABSOLUTE(0x0400);  -->  means .dtb section start addr
//     *(dtb)
//     . = ABSOLUTE(0x6000);  -->  means .dtb section end addr
// } >RAM
//
// !! Notice !!
// .dtb start & end addr should add base offset (0x10000)
// you can use linux terminal cmd '$ readelf -a pmu.elf' to check the value of base offset
// Section Headers:
//   [Nr] Name              Type            Addr     Off    Size   ES Flg Lk Inf Al
//   [ 0]                   NULL            00000000 000000 000000 00      0   0  0
//   [ 1] .isr_vector       PROGBITS        00000000 010000 000400 00   A  0   0 16
//   [ 2] .dtb              PROGBITS        00000400 010400 006C00 00  WA  0   0  1
/*****************************************************************************/
#define CM4_SECTION_ISR_OFS		(0x10000)    /* Maybe is version.c ? */
#define CM4_SECTION_ISR_SIZE		(0x00400)
#define CM4_SECTION_DTB_OFS		(CM4_SECTION_ISR_OFS + CM4_SECTION_ISR_SIZE)
#define CM4_SECTION_DTB_SIZE		PMU_CM4_DTB_SIZE    /* TODO: Change name ? */

/*****************************************************************************/
struct cm4_fw_info {
    char prefix[CM4_FILE_NAME_LEN];
    void *temp_elf;
    void *temp_dtb;
    /* fw.ELF convert fw.BIN. */
    void *data_buff;
    size_t data_size;
    /* Device tree in fw.BIN. */
    void *dtb_buff;
    size_t dtb_size;
    ofnode node;
};

/*****************************************************************************/
bool mtk_cm4_support = true;

/*****************************************************************************/
static int cm4_get_file_prefix(struct cm4_fw_info *fw)
{
    const char *path_prefix = NULL;

    /* Already prefix. */
    if (strlen(fw->prefix))
    {
        goto cm4_get_file_prefix_end;
    }

    /* Get path_prefix. */
    if (!(path_prefix = ofnode_read_string(fw->node, CM4_FIRMWARE_PREFIX)))
    {
        UBOOT_ERROR("ofnode_read_string(..., %s) fail.\n", CM4_FIRMWARE_PREFIX);
        return -ENODEV;
    }
    UBOOT_INFO("%s=%s.\n", CM4_FIRMWARE_PREFIX, path_prefix);

    /* Combine total prefix name. */
    if (snprintf(fw->prefix, CM4_FILE_NAME_LEN - 1, "%s", path_prefix) < 0)
    {
        UBOOT_ERROR("snprintf(fw_prefix) fail.\n");
        return -ENOBUFS;
    }

cm4_get_file_prefix_end:
    UBOOT_INFO("file_prefix=%s.\n", fw->prefix);
    return 0;
}

static int cm4_get_file_buff(const char *fp, void **buf, size_t *sz)
{
    static char dev_boot[CM4_BOOT_DEV_LEN] = {0};
    static char dev_part[CM4_PART_DEV_LEN] = {0};
    struct blk_desc *blk_dev_desc = NULL;
    int idx_dev = -1;
    unsigned idx_part = -1;
#ifdef CONFIG_MULTICORES_PLATFORM
    unsigned long irq_flag = 0;
#endif

    /* Cache. */
    if (strlen(dev_boot) && strlen(dev_part))
    {
        goto cm4_get_file_buff_open;
    }

    /* Get ext4ls mmc 0:7 ./pmu about "mmc". */
    if (sys_get_boot_device(dev_boot, sizeof(dev_boot)) == -1)
    {
        UBOOT_ERROR("sys_get_boot_device(...) fail.\n");
        return -ENODEV;
    }

    /* Get ext4ls mmc 0:7 ./pmu about "0". */
    idx_dev = 0; /* TODO: Change device number(0) hardcode by mtk api. */
    if (!(blk_dev_desc = blk_get_dev(dev_boot, idx_dev)))
    {
        UBOOT_ERROR("blk_get_dev(%s, %d) fail.\n", dev_boot, idx_dev);
        return -ENODEV;
    }

    /* Get ext4ls mmc 0:7 ./pmu about "7". */
    if (get_partition_info(blk_dev_desc, CM4_FIRMWARE_PART, NULL, &idx_part) != CMD_RET_SUCCESS)
    {
        UBOOT_ERROR("get_partition_info(%s, ...) fail.\n", CM4_FIRMWARE_PART);
        return -ENODEV;
    }

    /* Combine ext4ls mmc 0:7 ./pmu about "0:7". */
    if (snprintf(dev_part, sizeof(dev_part) - 1, "%X:%X", idx_dev, idx_part) < 0)
    {
        UBOOT_ERROR("snprintf() fail.\n");
        return EINVAL;
    }

cm4_get_file_buff_open:
    /* Seek ext4ls mmc 0:7 ./pmu about "ext4ls mmc 0:7". */
#if defined(CONFIG_MULTICORES_PLATFORM)
    smp_spin_lock_save(&fs_spin_lock, irq_flag);
#endif
    if (fs_set_blk_dev(dev_boot, dev_part, FS_TYPE_EXT))
    {
        UBOOT_ERROR("fs_set_blk_dev(%s, %s, FS_TYPE_EXT) fail.\n", dev_boot, dev_part);
#if defined(CONFIG_MULTICORES_PLATFORM)
        smp_spin_unlock_restore(&fs_spin_lock, irq_flag);
#endif
        return -EBUSY;
    }

    /* Get buffer address & size. */
    if (fs_size(fp, (loff_t *)sz) < 0)
    {
        UBOOT_ERROR("fs_size(%s) fail.\n", fp);
#if defined(CONFIG_MULTICORES_PLATFORM)
        smp_spin_unlock_restore(&fs_spin_lock, irq_flag);
#endif
        return -ENOENT;
    }
    *buf = malloc(*sz);
    if (!*buf)
    {
        UBOOT_ERROR("malloc(0x%tX) fail.\n", *sz);
#if defined(CONFIG_MULTICORES_PLATFORM)
        smp_spin_unlock_restore(&fs_spin_lock, irq_flag);
#endif
        return -ENOMEM;
    }

    /* Seek ext4ls mmc 0:7 ./pmu about "ext4ls mmc 0:7". */
    if (fs_set_blk_dev(dev_boot, dev_part, FS_TYPE_EXT))
    {
        UBOOT_ERROR("fs_set_blk_dev(%s, %s, FS_TYPE_EXT) fail.\n", dev_boot, dev_part);
#if defined(CONFIG_MULTICORES_PLATFORM)
        smp_spin_unlock_restore(&fs_spin_lock, irq_flag);
#endif
        return -EBUSY;
    }

    /* Get firmware from partition to dram. */
    if (fs_read(fp, (ulong)*buf, 0, 0, (loff_t *)sz) < 0)
    {
        UBOOT_ERROR("fs_read(%s) fail.\n", fp);
#if defined(CONFIG_MULTICORES_PLATFORM)
        smp_spin_unlock_restore(&fs_spin_lock, irq_flag);
#endif
        return -EIO;
    }
    flush_cache((ulong)*buf, *sz);
#if defined(CONFIG_MULTICORES_PLATFORM)
    smp_spin_unlock_restore(&fs_spin_lock, irq_flag);
#endif
    return 0;
}

static int cm4_get_firmware(struct cm4_fw_info *fw)
{
    char file[CM4_FILE_NAME_LEN] = {0};
    const char *suffix = VERIFY_FW_TYPE;
    int ret = 0;

    if ((ret = cm4_get_file_prefix(fw)) != 0)
        return ret;

    if (snprintf(file, sizeof(file) - 1, "%s%s", fw->prefix, suffix) < 0)
    {
        UBOOT_ERROR("snprintf(..., %s, %s) fail.\n", fw->prefix, suffix);
        return -ENOBUFS;
    }
    UBOOT_DEBUG("file=%s.\n", file);

    if ((ret = cm4_get_file_buff(file, &fw->data_buff, &fw->data_size)) != 0)
        return ret;

    fw->temp_elf = fw->data_buff;
    UBOOT_DEBUG("firmware load at buff=0x%p size=0x%tX.\n", fw->data_buff, fw->data_size);
    return 0;
}

static int cm4_verify_firmware(struct cm4_fw_info *fw)
{
#if 1
	unsigned int sign_size = SIGNATURE_LEN;
	unsigned char *auth_data = (unsigned char *)(fw->data_buff + sign_size);
	unsigned int auth_size = sign_size + (sign_size / 2);

	/* Bypass CM4 firmware verification during development period at temporary */
    /* PROTON-1097 to track enabling CM4 firmware signing and signature verification */
	printf("\nBypassing %s!!!\n", __func__);
	auth_data = (unsigned char *)(fw->data_buff + sign_size + auth_size);
	auth_size = *((unsigned int *)(fw->data_buff + sign_size + auth_size - 4));
	fw->data_buff = auth_data;
	fw->data_size = auth_size;

#else
    int ret = 0;
    st_secure_key key = {0};
    /* As follow reference signature flow in build time. */
    unsigned char *sign_data = (unsigned char *)fw->data_buff;
    unsigned int sign_size = SIGNATURE_LEN;
    unsigned char *auth_data = (unsigned char *)(fw->data_buff + sign_size);
    unsigned int auth_size = sign_size + (sign_size / CM4_FW_DIV_2);

    /* Get public key. */
    if (secure_get_key(KEY_TYPE_CKB_A_RI_KEY, &key) == -1)
    {
        UBOOT_ERROR("secure_get_key() fail.\n");
        return -ENOSYS;
    }

    /* Verify header. */
    if ((ret = secure_do_authentication(key.data,
                                        key.data + RSA_PUBLIC_KEY_N_LEN,
                                        sign_data,
                                        auth_data,
                                        auth_size)) != 0)
    {
        UBOOT_ERROR("secure_do_authentication(HEADER) fail.\n");
        UBOOT_DEBUG("RSA_PUBLIC_KEY_N_LEN:\n");
        UBOOT_DUMP((void *)key.data, RSA_PUBLIC_KEY_N_LEN);
        UBOOT_DEBUG("RSA_PUBLIC_KEY_E_LEN:\n");
        UBOOT_DUMP((void *)key.data + RSA_PUBLIC_KEY_N_LEN, RSA_PUBLIC_KEY_E_LEN);
        UBOOT_DEBUG("signature:\n");
        UBOOT_DUMP((void *)sign_data, sign_size);
        UBOOT_DEBUG("header:\n");
        UBOOT_DUMP((void *)auth_data, CM4_DUMP_DATA_LEN);
        return -EPERM;
    }
    UBOOT_DEBUG("\033[0;34m* Header Authentication SUCCESS !! *\033[0m\n");

    /* Verify ELF. */
    sign_data = auth_data;
    auth_data = (unsigned char *)(fw->data_buff + sign_size + auth_size);
    auth_size = *((unsigned int *)(fw->data_buff + sign_size + auth_size - CM4_VER_LEN));
    if ((ret = secure_do_authentication(key.data,
                                        key.data + RSA_PUBLIC_KEY_N_LEN,
                                        sign_data,
                                        auth_data,
                                        auth_size)) != 0)
    {
        UBOOT_ERROR("secure_do_authentication(ELF) fail.\n");
        UBOOT_DEBUG("RSA_PUBLIC_KEY_N_LEN:\n");
        UBOOT_DUMP((void *)key.data, RSA_PUBLIC_KEY_N_LEN);
        UBOOT_DEBUG("RSA_PUBLIC_KEY_E_LEN:\n");
        UBOOT_DUMP((void *)key.data + RSA_PUBLIC_KEY_N_LEN, RSA_PUBLIC_KEY_E_LEN);
        UBOOT_DEBUG("signature:\n");
        UBOOT_DUMP((void *)sign_data, sign_size);
        UBOOT_DEBUG("firmware:\n");
        UBOOT_DUMP((void *)auth_data, CM4_DUMP_DATA_LEN);
        return -EPERM;
    }
    UBOOT_DEBUG("\033[0;34m* Firmware Authentication SUCCESS !! *\033[0m\n");

    fw->data_buff = auth_data;
    fw->data_size = auth_size;
    UBOOT_DEBUG("firmware adjust at buff=0x%p size=0x%tX.\n", fw->data_buff, fw->data_size);
#endif
    return 0;
}

static int cm4_get_dtb(struct cm4_fw_info *fw)
{
    char file[CM4_FILE_NAME_LEN] = {0};
    const char *suffix = VERIFY_DTB_TYPE;
    int ret = 0;

    if ((ret = cm4_get_file_prefix(fw)) != 0)
        return ret;

    if (snprintf(file, sizeof(file) - 1, "%s%s", fw->prefix, suffix) < 0) {
        UBOOT_ERROR("snprintf(..., %s, %s) fail.\n", fw->prefix, suffix);
        return -ENOBUFS;
    }
    UBOOT_DEBUG("file=%s.\n", file);

    if ((ret = cm4_get_file_buff(file, &fw->dtb_buff, &fw->dtb_size)) != 0)
        return ret;

    fw->temp_dtb = fw->dtb_buff;
    UBOOT_DEBUG("dtb load at buff=0x%p size=0x%tX.\n", fw->dtb_buff, fw->dtb_size);
    return 0;
}

static int cm4_overlay_dtb(struct cm4_fw_info *fw)
{
    int idx = 0, cnt = 0;
    size_t dtb_total = fw->dtb_size, dtb_limit = 0;
    unsigned int dtb_cus_size = 0;
    int ret = 0;
    const char *file = NULL;
    void *dtbo_buff = NULL;
    size_t dtbo_size = 0;
#if defined(CONFIG_LIBUFDT_OVERLAY)
    struct fdt_header *source = NULL;
#else
    void *temp_buff = NULL;
#endif

    /* Get number of dtbo. */
    if ((cnt = ofnode_read_string_count(fw->node, CM4_DTBO_PATH)) <= 0)
    {
        UBOOT_ERROR("ofnode_read_string_count(..., %s) fail.\n", CM4_DTBO_PATH);
        return -ENODEV;
    }
    UBOOT_INFO("dtbo-num=%d.\n", cnt);

    /* Get dtb_section limit. */
    dtb_limit = CM4_SECTION_DTB_SIZE;
    if (ofnode_read_u32(fw->node, CM4_DTB_CUS_SIZE, (u32 *)&dtb_cus_size) == 0)
        dtb_limit = (size_t)dtb_cus_size;
    UBOOT_DEBUG("dtb_limit=0x%tX.\n", dtb_limit);

    /* Oveylay for each path. */
    for (idx = 0; idx < cnt; idx++)
    {
        if (ofnode_read_string_index(fw->node, CM4_DTBO_PATH, idx, &file) < 0)
        {
            return -ENXIO;
        }
        /* Get dtbo buffer. */
        if ((ret = cm4_get_file_buff(file, &dtbo_buff, &dtbo_size)) != 0)
        {
            break;
        }

        /* Check dtb section size. */
        UBOOT_INFO("%s[%d/%d]=%s overlay size=0x%tX begin.\n", CM4_DTBO_PATH, idx, cnt, file, dtbo_size);
        dtb_total += dtbo_size;
        if (dtb_total > dtb_limit)
        {
            UBOOT_ERROR("Please adjust $(MEDIATEK_PMU_CM4_DTB_SIZE) by platform or confirm dts have %s,\
                         dtb_total=0x%tX > dtb_limit=0x%tX.\n", CM4_DTB_CUS_SIZE, dtb_total, dtb_limit);
            ret = -ENOSPC;
            break;
        }
        /* Overlay method. */
#if defined(CONFIG_LIBUFDT_OVERLAY)
        if (!(source = ufdt_apply_overlay(fw->dtb_buff, fw->dtb_size, dtbo_buff, dtbo_size)))
        {
            UBOOT_ERROR("ufdt_apply_overlay() fail.\n");
            ret = -EIO;
            break;
        }
        fw->dtb_buff = source;
        fw->dtb_size = fdt32_to_cpu(source->totalsize);
        if (fw->dtb_size == 0 || fw->dtb_size > dtb_limit)
        {
            UBOOT_ERROR("size fail dtb_size=0x%tX, dtb_limit=0x%tX.\n",
                         fw->dtb_size, dtb_limit);
            ret = -EIO;
            break;
        }
#else
        if (!(temp_buff = malloc(dtb_total)))
        {
            UBOOT_ERROR("malloc(0x%tX) fail.\n", dtb_total);
            ret = -ENOMEM;
            break;
        }
        if ((ret = fdt_open_into(fw->dtb_buff, temp_buff, dtb_total)) != 0)
        {
            UBOOT_ERROR("fdt_open_into() fail=%d.\n", ret);
            free(temp_buff);
            break;
        }
        if ((ret = fdt_overlay_apply(temp_buff, dtbo_buff)) != 0)
        {
            UBOOT_ERROR("fdt_overlay_apply() fail=%d.\n", ret);
            free(temp_buff);
            break;
        }
        free(fw->dtb_buff);
        fw->dtb_buff = temp_buff;
        temp_buff = NULL;
#endif
        UBOOT_DEBUG("dtb adjust at buff=0x%p size=0x%tX.\n", fw->dtb_buff, fw->dtb_size);
        free(dtbo_buff);
        dtbo_buff = NULL;
        UBOOT_INFO("%s[%d/%d]=%s overlay size=0x%tX done.\n", CM4_DTBO_PATH, idx + 1, cnt, file, dtbo_size);
    }

    /* Check result. */
    if (ret != 0)
        goto cm4_overlay_dtb_end;

    /* Overwrite dtb section at firmware buffer. */
    memset(fw->data_buff + CM4_SECTION_DTB_OFS, 0, dtb_limit);
    UBOOT_DEBUG("Clear dtb_section at buff=0x%p size=0x%tX.\n", fw->data_buff, dtb_limit);
    memcpy(fw->data_buff + CM4_SECTION_DTB_OFS, fw->dtb_buff , fw->dtb_size);
    UBOOT_DEBUG("Write dtb_section at buff=0x%p size=0x%tX.\n", fw->data_buff, fw->dtb_size);
    flush_cache((ulong)(fw->data_buff + CM4_SECTION_DTB_OFS), fw->dtb_size);

cm4_overlay_dtb_end:
    if (dtbo_buff)
    {
        free(dtbo_buff);
        dtbo_buff = NULL;
    }
    return ret;
}

/*****************************************************************************/
int cm4_loader(const char *dev_node)
{
    struct udevice *dev = NULL;
    struct cm4_fw_info fw = {0};
    int ret = 0;
    const char *dev_name = strchr(dev_node, '/') + 1;

    UBOOT_TRACE("load cm4 via rproc.\n");

    /* Check device exist. */
    fw.node = ofnode_path(dev_node);
    if (!ofnode_valid(fw.node) || !ofnode_is_available(fw.node))
    {
        UBOOT_DEBUG("ignore %s loading.\n", dev_node);
        return 0;
    }

    /* Decide by dtbo result. */
    ret = uclass_get_device_by_name(UCLASS_REMOTEPROC, dev_name, &dev);
    if (ret == -ENODEV)
    {
        UBOOT_DEBUG("get_device(%s) pass=%d.\n", dev_name, ret);
        return 0;
    }
    else if (ret != 0)
    {
        UBOOT_ERROR("get_device(%s) fail=%d.\n", dev_name, ret);
        return ret;
    }

    /* Init framework. */
    ret = rproc_init();
    if ((ret != 0) && (ret != -EINVAL))
    {
        UBOOT_ERROR("rproc_init() fail=%d.\n", ret);
        return ret;
    }

    /* Check feature support via rproc layer. */
    if (mtk_cm4_support == false)
    {
        UBOOT_DEBUG("mtk_cm4_support=%d.\n", mtk_cm4_support);
        return 0;
    }

    /* Obtain firmware buffer. */
    ret = cm4_get_firmware(&fw);
    if (ret != 0)
    {
        UBOOT_ERROR("cm4_get_firmware() fail=%d.\n", ret);
        goto cm4_loader_err;
    }

    /* Verify firmware buffer. */
    ret = cm4_verify_firmware(&fw);
    if (ret != 0)
    {
        UBOOT_ERROR("cm4_verify_firmware() fail=%d.\n", ret);
        goto cm4_loader_err;
    }

    /* Obtain dtb buffer. */
    ret = cm4_get_dtb(&fw);
    if (ret != 0)
    {
        UBOOT_ERROR("cm4_get_dtb() fail=%d.\n", ret);
        goto cm4_loader_err;
    }

    /* Overlap device tree. */
    ret = cm4_overlay_dtb(&fw);
    if (ret != 0)
    {
        UBOOT_ERROR("cm4_overlay_dtb() fail=%d.\n", ret);
        goto cm4_loader_err;
    }

    /* Load firmware. */
    ret = rproc_load(dev->seq, (ulong)fw.data_buff, (ulong)fw.data_size);
    if (ret != 0)
    {
        UBOOT_ERROR("rproc_load() fail=%d.\n", ret);
        goto cm4_loader_err;
    }

    /* Release mcu. */
    ret = rproc_start(dev->seq);
    if (ret != 0)
        UBOOT_ERROR("rproc_start() fail=%d.\n", ret);

cm4_loader_err:
    if (fw.temp_elf) {
        free(fw.temp_elf);
        fw.temp_elf = NULL;
    }
    if (fw.temp_dtb) {
        free(fw.temp_dtb);
        fw.temp_dtb = NULL;
    }
    return ret;
}

int cm4_consulter(const char *dev_node)
{
    struct udevice *dev = NULL;
    int ret = 0;
    const char *dev_name = strchr(dev_node, '/') + 1;

    UBOOT_TRACE("consult cm4 via rproc.\n");

    /* Check framework init. */
    if (rproc_is_initialized() == false)
        return -EINVAL;

    /* Decide by dtbo result. */
    ret = uclass_get_device_by_name(UCLASS_REMOTEPROC, dev_name, &dev);
    if (ret == -ENODEV)
    {
        UBOOT_DEBUG("get_device(%s) pass=%d.\n", dev_name, ret);
        return 0;
    }
    else if (ret != 0)
    {
        UBOOT_ERROR("get_device(%s) fail=%d.\n", dev_name, ret);
        return ret;
    }

    /* Check mcu ready. */
    return rproc_is_running(dev->seq);
}

int cm4_stop(const char *dev_node)
{
    struct udevice *dev = NULL;
    int ret = 0;
    const char *dev_name = strchr(dev_node, '/') + 1;

    UBOOT_TRACE("stop cm4 via rproc.\n");

    /* Check framework init. */
    if (rproc_is_initialized() == false)
    {
        return -EINVAL;
    }

    /* Decide by dtbo result. */
    ret = uclass_get_device_by_name(UCLASS_REMOTEPROC, dev_name, &dev);
    if (ret == -ENODEV)
    {
        UBOOT_DEBUG("get_device(%s) pass=%d.\n", dev_name, ret);
        return 0;
    }
    else if (ret != 0)
    {
        UBOOT_ERROR("get_device(%s) fail=%d.\n", dev_name, ret);
        return ret;
    }

    /* Check mcu ready. */
    return rproc_stop(dev->seq);
}
