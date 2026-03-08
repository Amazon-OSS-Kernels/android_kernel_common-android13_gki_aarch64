/*
 * Copyright (c) 2024 Amazon.com, Inc. or its affiliates.  All rights reserved.
 *
 */

#include <ld_bt.h>

#define TAG "[LD_BT]"

typedef struct {
    unsigned short vid;
    unsigned short pid;
    char name[15];
    char vendor;
    void (*init)(ld_bt_func_t *ptr);
} usb_vid_pid;

#define VENDOR_MTK  1
#define VENDOR_RTK  2

usb_vid_pid vid_pid_array[] = {
    {0x0BDA, 0xA822, "RTL8822", VENDOR_RTK, ld_bt_func_init_rtk},
    {0x0BDA, 0xA82A, "RTL8822", VENDOR_RTK, ld_bt_func_init_rtk},
    {0x0BDA, 0xA82B, "RTL8822", VENDOR_RTK, ld_bt_func_init_rtk},
    {0x0BDA, 0xE822, "RTL8822", VENDOR_RTK, ld_bt_func_init_rtk},
};
int vid_pid_array_size = sizeof(vid_pid_array) / sizeof(vid_pid_array[0]);


static unsigned char *ld_bt_load(char *partition, char *name, int *length)
{
    unsigned char *file_ptr;
    int result;
    int device;
    char device_name[8];
    char storage_info[8];
    loff_t size;
    int malloc_len;
#ifdef CONFIG_MULTICORES_PLATFORM
    unsigned long irq_flag;
#endif

    UBOOT_DEBUG("Partition: %s\n", partition);

    memset(&device_name[0], 0, sizeof(device_name));
    memset(&storage_info[0], 0, sizeof(storage_info));
    device = 0;

    result = sys_get_boot_device(&device_name[0], sizeof(device_name));
    if (result < 0) {
        UBOOT_ERROR("Failed to get boot device (%d)\n", result);
        return NULL;
    }

    result = sys_get_storage_info(device, partition, &storage_info[0]);
    if (result < 0) {
        UBOOT_ERROR("Failed to get storage info (%d)\n", result);
        return NULL;
    }

#ifdef CONFIG_MULTICORES_PLATFORM
    smp_spin_lock_save(&fs_spin_lock, irq_flag);
#endif
    result = fs_set_blk_dev(device_name, storage_info, FS_TYPE_EXT);
    if (result != 0) {
        UBOOT_ERROR("Failed to set bulk device (%d)\n", result);
#ifdef CONFIG_MULTICORES_PLATFORM
        smp_spin_unlock_restore(&fs_spin_lock, irq_flag);
#endif
        return NULL;
    }

    result = fs_size(name, &size);
    if (result < 0) {
        UBOOT_ERROR("Failed to get file size (%d)\n", result);
#ifdef CONFIG_MULTICORES_PLATFORM
        smp_spin_unlock_restore(&fs_spin_lock, irq_flag);
#endif
        return NULL;
    }
    UBOOT_DEBUG("File Size: %d\n", (int)size);

    malloc_len = ((int)size / 4096 + 1) * 4096;
    file_ptr = malloc(malloc_len);
    if (file_ptr == NULL) {
        UBOOT_ERROR("Failed to malloc (%d)\n", malloc_len);
#ifdef CONFIG_MULTICORES_PLATFORM
        smp_spin_unlock_restore(&fs_spin_lock, irq_flag);
#endif
        return NULL;
    }

    result = sys_get_storage_info(device, partition, &storage_info[0]);
    if (result < 0) {
        UBOOT_ERROR("Failed to get storage info (%d)\n", result);
#ifdef CONFIG_MULTICORES_PLATFORM
        smp_spin_unlock_restore(&fs_spin_lock, irq_flag);
#endif
        free(file_ptr);
        return NULL;
    }

    result = fs_set_blk_dev(device_name, storage_info, FS_TYPE_EXT);
    if (result != 0) {
        UBOOT_ERROR("Failed to set bulk device (%d)\n", result);
#ifdef CONFIG_MULTICORES_PLATFORM
        smp_spin_unlock_restore(&fs_spin_lock, irq_flag);
#endif
        free(file_ptr);
        return NULL;
    }

    result = fs_read(name, (unsigned long)file_ptr, 0, 0, &size);
    if (result < 0) {
        UBOOT_ERROR("Failed to read file (%d, %d)\n", result, errno);
#ifdef CONFIG_MULTICORES_PLATFORM
        smp_spin_unlock_restore(&fs_spin_lock, irq_flag);
#endif
        free(file_ptr);
        return NULL;
    }
#ifdef CONFIG_MULTICORES_PLATFORM
    smp_spin_unlock_restore(&fs_spin_lock, irq_flag);
#endif

    if (size < 0) {
        UBOOT_ERROR("Wrong file size (%d)\n", (int)size);
        free(file_ptr);
        return NULL;
    }

    UBOOT_INFO("%s File read: %d, %d\n", TAG, (int)size, malloc_len);
    *length = size;
    return file_ptr;
}

unsigned char *ld_bt_load_file(char *name, char *path, int *length)
{
    unsigned char *ret;

    if (name == NULL) {
        UBOOT_ERROR("File name cannot be NULL\n");
        return NULL;
    }

    char file_path[128];
    memset(&file_path[0], 0, sizeof(file_path));

    if (path != NULL) {
        (void)snprintf(file_path, sizeof(file_path), "%s/%s", path, name);
    } else {
        (void)snprintf(file_path, sizeof(file_path), "%s", name);
    }
    UBOOT_INFO("%s Load file: %s\n", TAG, &file_path[0]);

    char *partition[] = {"uenv", "bootdata", "tvconfig", "vendor", "userdata", "system", "APP"};
    int i = 0;

    for (i = 0; i < sizeof(partition)/sizeof(char *); i++) {
        ret = ld_bt_load(partition[i], file_path, length);
        if (ret != NULL) {
            return ret;
        }
    }

    UBOOT_ERROR("Cannot load the file\n");
    return NULL;
}


typedef int (*usb_show_info_t)(bt_ctlr_t dstudev, bt_ctlr_t udev, int *index_found);
static int usb_show_info(bt_ctlr_t dstudev, bt_ctlr_t udev, int *index_found)
{
    int ret;
    int index;
    struct udevice *child;
    UBOOT_DEBUG("begin\n");

    for (index = 0; index < vid_pid_array_size; index++) {
        if ((udev->descriptor.idVendor == vid_pid_array[index].vid) &&
            (udev->descriptor.idProduct == vid_pid_array[index].pid)) {
            memcpy(dstudev, udev, sizeof(struct usb_device));
            *index_found = index;
            return 0;
        }
    }

    for (device_find_first_child(udev->dev, &child);
        child;
        device_find_next_child(&child)) {

        if (device_active(child) &&
            (device_get_uclass_id(child) != UCLASS_USB_EMUL) &&
            (device_get_uclass_id(child) != UCLASS_BLK)) {
            udev = dev_get_parent_priv(child);
            ret = usb_show_info(dstudev, udev, index_found);
            if (ret == 0) {
                return 0;
            }
        }
    }

    UBOOT_DEBUG("end\n");
    return -1;
}

static int usb_for_each_root_dev(bt_ctlr_t dstudev, usb_show_info_t func, int *index_found)
{
    int ret;
    struct udevice *bus;
    struct udevice *dev;
    bt_ctlr_t udev;
    UBOOT_DEBUG("begin\n");

    ret = usb_init();
    if (ret != 0) {
        UBOOT_ERROR("USB init failed (%d)\n", ret);
        return -1;
    }

    ret = -1;
    for (uclass_first_device(UCLASS_USB, &bus);
        bus;
        uclass_next_device(&bus)) {

        if (!device_active(bus)) {
            continue;
        }
        device_find_first_child(bus, &dev);
        if (dev && device_active(dev)) {
            udev = dev_get_parent_priv(dev);
            ret = func(dstudev, udev, index_found);
            if (ret == 0) {
                break;
            }
        }
    }

    UBOOT_DEBUG("end\n");
    return ret;
}

static int ld_bt_find_usb_device(bt_ctlr_t udev, int *index_found)
{
    int ret = -1;

    if (udev == NULL) {
        return -1;
    }

#ifdef CONFIG_DM_USB
    ret = usb_for_each_root_dev(udev, usb_show_info, index_found);
#endif

    if (ret != 0) {
        UBOOT_ERROR("Failed to find USB device (%d)\n", ret);
        return -1;
    }

    UBOOT_INFO("%s USB device found: 0x%04X, 0x%04X\n", TAG,
               vid_pid_array[*index_found].vid,
               vid_pid_array[*index_found].pid);
    return 0;
}


int do_setRtkBT(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
    int ret;

    // Change message print level locally
    // EN_DEBUG_LEVEL dbgLevelBackup = dbgLevel;
    // dbgLevel  = EN_DEBUG_LEVEL_DISABLE;
    // dbgLevel |= EN_DEBUG_LEVEL_ERROR;
    // dbgLevel |= EN_DEBUG_LEVEL_INFO;
    // dbgLevel |= EN_DEBUG_LEVEL_TRACE;
    // dbgLevel |= EN_DEBUG_LEVEL_DEBUG;

    UBOOT_INFO("%s Enter command %s\n", TAG, __func__);

    if (argc < 1) {
        cmd_usage(cmdtp);
        return -1;
    }

    struct usb_device udev;
    memset(&udev, 0, sizeof(struct usb_device));
    int index = -1;

    // Search USB devices and compare VID/PID
    ret = ld_bt_find_usb_device(&udev, &index);
    if (ret != 0) {
        return -1;
    }

    // Setup function pointers by the vendor
    ld_bt_func_t ld_bt_func;
    ld_bt_func.init = NULL;
    ld_bt_func.set_woble = NULL;
    ld_bt_func.clean_up = NULL;
    vid_pid_array[index].init(&ld_bt_func);

    ret = ld_bt_func.init(&udev);
    if (ret != 0) {
        UBOOT_ERROR("Vendor BT init failed (%d)\n", ret);
    } else {
        UBOOT_INFO("%s Vendor BT init succeeded\n", TAG);
    }

    if (ret == 0) {
        ret = ld_bt_func.set_woble(&udev);
        if (ret != 0) {
            UBOOT_ERROR("Vendor BT WoBLE failed (%d)\n", ret);
        } else {
            UBOOT_INFO("%s Vendor BT WoBLE succeeded\n", TAG);
        }
    }

    ret = ld_bt_func.clean_up(&udev);
    if (ret != 0) {
        UBOOT_ERROR("Vendor BT cleanup failed (%d)\n", ret);
    } else {
        UBOOT_INFO("%s Vendor BT cleanup succeeded\n", TAG);
    }

    UBOOT_TRACE("OK\n");

    // dbgLevel = dbgLevelBackup;
    return ret;
}
