/* SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause) */
/*
 * Copyright (c) 2023 MediaTek Inc.
*/

#ifndef _INIT_UTILITY_H_
#define _INIT_UTILITY_H_

#define STORAGE_DEVICE_BUF_SIZE     8
#define STORAGE_INDEX_BUF_SIZE      8
#define COMMAND_BUF_SIZE            128
#define BOOTARGS_KEY_LEN            32
#define BOOTARGS_CFG_LEN            64

#define BOOTCMD "bootcmd"
#define RECOVERYCMD "recoverycmd"
#define DTBO_ADDR_ENV "dtboaddr"
#define DTBO_PARTNAME "dtbo"

#define SIZE_8G       0x200000000ULL
#define SIZE_6G       0x180000000ULL
#define SIZE_4G       0x100000000ULL
#define SIZE_3G       0x0C0000000ULL
#define SIZE_2G       0x080000000ULL
#define SIZE_1p5G     0x060000000ULL
#define SIZE_1G       0x040000000ULL

#define SIZE_1M       0x000100000ULL

typedef enum
{
    EN_BOOT_DEVICE_UNKNOWN,
    EN_BOOT_DEVICE_USB,
    EN_BOOT_DEVICE_EMMC,
    EN_BOOT_DEVICE_UFS,
    EN_BOOT_DEVICE_MAX =0xFF,
}EN_BOOT_DEVICE;

//-------------------------------------------------------------------------------------------------
/// Delet the member that exist in bootargs
/// For example: del_bootargs_cfg("autotest",0)
/// @param key                           \b IN: the member that exist in the env 'bootargs'
////@param save                      \b IN: 1, saveenv , 0, don't saveenv
/// @return  int                              \b OUT: 0, delete sucessfully , -1, delete fail
//-------------------------------------------------------------------------------------------------
int del_bootargs(char *key,bool save);
//-------------------------------------------------------------------------------------------------
/// Edit/Add a new member to bootargs
/// For example: add_bootargs("autotest","autotest=ture",0);
/// @param key                           \b IN: the new member's prefix
/// @param set_Cfg                               \b IN: the new member's content
////@param save                           \b IN: 1, saveenv , 0, don't saveenv
/// @return  int                                   \b OUT: 0, edit sucessfully , -1, edit fail
//-------------------------------------------------------------------------------------------------
int add_bootargs(char * key, char *set_cfg, bool save);
//-------------------------------------------------------------------------------------------------
/// Parameters Replace for upgrade script using
/// For example: para_replace("tftp 0x20000000 rom_emmc_boot.bin" ,0 ,"./[[mboot)")
/// @param str                           \b IN: string buffer
/// @param argc                               \b IN: string length
////@param argv                           \b IN: replace string
/// @return  int                                   \b OUT: new string
//-------------------------------------------------------------------------------------------------
char* para_replace(char *str,int argc,char * const argv[]);
//-------------------------------------------------------------------------------------------------
/// Find a member in current bootargs
/// For example: find_bootargs("autotest");
/// @param key                           \b IN: the member's prefix
/// @return  int                                   \b OUT: 0, not exist , non-Zero , exist
//-------------------------------------------------------------------------------------------------
int find_bootargs(char *key);

//-------------------------------------------------------------------------------------------------
/// Parameters N/A
/// @return  int                                   \b OUT: 0, normal boot mode, 1, QHB boot mode
//-------------------------------------------------------------------------------------------------
int is_qhb_boot_mode(void);

//-------------------------------------------------------------------------------------------------
/// get boot divice usb/mmc
//
/// @return  int                                   \b OUT: EN_BOOT_DEVICE_UNKNOWN, Fail ,EN_BOOT_DEVICE_EMMC

int get_boot_device(void);
//-------------------------------------------------------------------------------------------------
/// get boot divice usb/mmc
//  @param boot_device                           \b IN: string buffer
/// @return  int                                 \b OUT: -1, Fail ,1 sucessfully

int sys_get_boot_device(char * boot_device, unsigned int boot_device_len);

/// set retry count
void sys_add_retry_count(void);
void sys_show_retry_count(void);
void sys_set_retry_count(unsigned int retry_count);

int sys_get_storage_info(unsigned int device, const char *partition_name, char *storage_info);


//-------------------------------------------------------------------------------------------------
/// Convert GPT uenv partition to EXT4 format
/// Partition name must be 'uenv'
/// Size of uenv partition must be 8MB
/// Real size of uboot environment data can be store : about 3.8 MB
/// @return  int                               \b OUT: 0 Success, 1 Failure
//-------------------------------------------------------------------------------------------------
int format_uenv_to_ext4(const char *str_interface_type, const char *str_device_number);
// TODO: add compile flag to disable this API, when uboot env will not apply by mtk turnkey.

//-------------------------------------------------------------------------------------------------
/// Parameters N/A
/// @return  int                                   \b OUT: 0, normal boot mode, 1, recovery boot mode
//-------------------------------------------------------------------------------------------------
int is_recovery_mode(void);
//-------------------------------------------------------------------------------------------------
/// Parameters N/A
/// @return  int                                   \b OUT: 0, normal boot mode, 1, disable backlight and bootlogo
//-------------------------------------------------------------------------------------------------

int pm_check_back_ground_active(void);

//-------------------------------------------------------------------------------------------------
/// Store u-boot time stamp to dummy register
/// Parameters N/A
/// @return  N/A
//-------------------------------------------------------------------------------------------------
void time_stamp_store_to_register(uint64_t time, int count);

//-------------------------------------------------------------------------------------------------
/// read if is RPMBKey exist
/// Parameters N/A
/// @return  int                               \b OUT: 0 no, 1 yes
//-------------------------------------------------------------------------------------------------
unsigned int is_RPMBKey_exist(void);

//-------------------------------------------------------------------------------------------------
/// generate a random number form AESDMA
/// @Parameters target                         \b IN: the rnd stored
/// @Parameters size                           \b IN: the input buffer size
/// @return  int                               \b OUT: 0 on success, otherwise is error
//-------------------------------------------------------------------------------------------------
int generate_random_number(unsigned char *target, int size);

//-------------------------------------------------------------------------------------------------
/// get device id from efuse
/// @Parameters buf                            \b IN: the device id stored
/// @Parameters size                           \b IN: the input buffer size
/// @return  int                               \b OUT: 0 on success, otherwise is error
//-------------------------------------------------------------------------------------------------
int get_device_unique_id(unsigned char *buf, unsigned int size);

//-------------------------------------------------------------------------------------------------
/// Read file from storage to memory
/// @Parameters partition                            \b IN: the partition name which want to access
/// @Parameters path                           \b IN: the file path which want to access
/// @Parameters size                           \b IN: the size of file
/// @return  char *                               \b OUT: NULL is read file failure, otherwise is success
//-------------------------------------------------------------------------------------------------
unsigned char *read_storage_file_to_memory(const char *partition, const char *path, loff_t *size);

//-------------------------------------------------------------------------------------------------
/// write ini file to ext4 file system
/// @Parameters partition                            \b IN: the partition name which want to access
/// @Parameters path                           \b IN: the file path which want to access
/// @Parameters path                           \b IN: the  ini file addr
/// @Parameters size                           \b IN: the size of file
/// @return  char *                               \b OUT: NULL is read file failure, otherwise is success
//-------------------------------------------------------------------------------------------------

unsigned char *write_storage_file(const char *partition, const char *path, unsigned char *buffer, loff_t *size);

//-------------------------------------------------------------------------------------------------
/// Parsing loading address according to bootcmd
/// @Parameters part_str                          \b IN: the partition name to be searched
/// @Parameters bootcmd                           \b IN: buffer of the embedded bootcmd to be parsed
/// @return  long                                 \b OUT: 0 if it fails to parse, otherwise parsed address is returned
//-------------------------------------------------------------------------------------------------
unsigned long getImageAddr(const char* part_str, const char* bootcmd);

//-------------------------------------------------------------------------------------------------
/// Parsing loading address according to bootcmd
/// @Parameters part_strings                      \b IN: the partition name array to refer to
/// @Parameters num_of_parts                      \b IN: Number of partition name array
/// @Parameters filtered_bootcmd                  \b INOUT: The output filtered bootcmd
/// @Parameters outBuffSize                       \b IN: Size of the output buffer
/// @return  long                                 \b OUT: 0 if it fails to filter, normally 1 is returned
unsigned int filterBootcmd(const char* const *part_strings, size_t num_of_parts, char* filtered_bootcmd, size_t outBuffSize);

//-------------------------------------------------------------------------------------------------
/// Parsing loading address according to DTBO_ADDRESS
/// @return  long                                \b OUT: 0 if it fails to parse, otherwise parsed address is returned
//-------------------------------------------------------------------------------------------------
unsigned long getDtboAddr(void);

//-------------------------------------------------------------------------------------------------
/// Common system_fail_handler
//-------------------------------------------------------------------------------------------------
void system_fail_handler(void);

void skip_add_retry_count(void);

void setup_avb_data_for_dtb(char* data, unsigned int size);
int insert_avb_data_into_dtb(void *fdt);
int is_update_rollback_index_needed(bool* b_update_rollback_index_needed);
bool is_pass_rollback_index_info_needed(void);
int set_pass_rollback_indexes_needed(void);
void set_pmu_rollback_index(uint64_t rollback_index);
uint64_t get_pmu_rollback_index(void);

#endif

