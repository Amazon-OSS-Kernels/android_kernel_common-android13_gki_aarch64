/* SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause) */
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#ifndef _API_UPGRADE_UTILITY_H_
#define _API_UPGRADE_UTILITY_H_
#include <common.h>
#include <command.h>
#include <exports.h>


#define ENV_UPGRADE_IMAGE   "UpgradeImage"
#define ENV_UPGRADE_PKG_VERSION   "upgrade_pkg_version"
#define ENV_UPGRADE_STATUS  "upgrade_status"
#define ENV_UPGRADE_MODE    "upgrade_mode"
#define ENV_UPGRADE_COMPLETE    "upgrade_complete"


#define CONFIG_SYS_MIU0_CACHE 0x20000000
#define CONFIG_UPGRADE_OFFSET 0xF000000
#define CONFIG_UPGRADE_BUFFER_ADDR       (CONFIG_SYS_MIU0_CACHE|CONFIG_UPGRADE_OFFSET|0x200000) //0x2F200000
#define UPGRADE_CHECK_BIN_BUFFER_ADDR     CONFIG_UPGRADE_BUFFER_ADDR //0x2F200000
#define UPGRADE_HEADER_BUFFER_ADDR  (CONFIG_SYS_MIU0_CACHE|CONFIG_UPGRADE_OFFSET|0x100000) //0x2F100000
#define UPGRADE_LOAD_SCRIPT_BUFFER_ADDR  (CONFIG_SYS_MIU0_CACHE|CONFIG_UPGRADE_OFFSET) //0x2F000000


#define HEADER_VERSION_SIZE 0x200
/* version format: {prefix}V_{x.x.x}
** 0 <= prefix <= 16 characters
** 0 <= x < 1000 */
#define HEADER_VERSION_PREFIX_SIZE 17 // include null terminator
#define HEADER_VERSION_DELIM_SIZE 2
#define HEADER_VERSION_NUM_SIZE 12 // include null terminator
#define HEADER_VERSION_USED_SIZE 30 // include null terminator
#define HEADER_IMAGE_OFFSET_SIZE 0x200
#define HEADER_CHUNKINFO_SIZE 0x80
#define SIGNATURE_SIZE 0x100
// the auth part of header is VERSION, IMAGE_OFFSET, REE_CHUNK_INFO,REE's SIGN_OF_HASH_SET
#define HEADER_SIZE (HEADER_VERSION_SIZE + HEADER_IMAGE_OFFSET_SIZE + HEADER_CHUNKINFO_SIZE + SIGNATURE_SIZE)
#define PACK_HEADER_SIZE 0x1000
#define REE_SCRIPT_SIZE 0x10000
#define SEGMENT_RSA_CHUNK_FILE_SIZE 128

#define CHUNK_ID  "MTK....." // 8 bytes
#define CHUNK_END ".....mtk" // 8 bytes
// chunk info size of elements (bytes)
#define CHUNK_ID_HEAD_SIZE (sizeof(CHUNK_ID)-1)
#define SEGMENT_SIZE (sizeof(int))
#define FILE_DATA_OFFSET (sizeof(int))
#define FILE_DATA_SIZE (sizeof(int))
#define FILE_HASH_OFFSET (sizeof(int))
#define FILE_HASH_SIZE (sizeof(int))
#define DOWNLOAD_BUF (sizeof(int))
#define CHUNK_ID_END_SIZE (sizeof(CHUNK_END)-1)
#define RESERVED_SIZE (HEADER_CHUNKINFO_SIZE - (CHUNK_ID_HEAD_SIZE + SEGMENT_SIZE + FILE_DATA_OFFSET + FILE_DATA_SIZE + FILE_HASH_OFFSET + FILE_HASH_SIZE + DOWNLOAD_BUF + CHUNK_ID_END_SIZE))

#ifdef CONFIG_RELOAD_DEFAULT_ENV
#define SET_ENV_PARTITOIN  "tvconfig"
#define SET_ENV_FILE       "/config/set_env"
#endif

typedef struct image_offset_info
{
    // offset info of TEE image and REE image
    unsigned int u32ree_image_offset_start;
    unsigned int u32ree_image_offset_len;

    // offset for firmware image header
    int sign_of_hash_set_size;
    int chunk_info_size;
    int image_offset_size;
    int version_size;
    int sign_of_header_size;
}st_image_offset_info;

typedef struct image_version_info{
    char full_version[HEADER_VERSION_USED_SIZE];
    char prefix[HEADER_VERSION_PREFIX_SIZE];
    int major_num;
    int minor_num;
    int subminor_num;
}st_image_version_info;

typedef struct Chunk_Info_Structure
{
    unsigned char chunk_id_head[CHUNK_ID_HEAD_SIZE];
    unsigned int segment_size;
    unsigned int file_data_offset;
    unsigned int file_data_len;
    unsigned int file_hash_offset;
    unsigned int file_hash_len;
    unsigned int download_buf;
    unsigned char reserved[RESERVED_SIZE];
    unsigned char chunk_id_end[CHUNK_ID_END_SIZE];
}st_chunk_info;

typedef enum
{
    EN_UPDATE_MODE_NONE,
    EN_UPDATE_MODE_TFTP,
    EN_UPDATE_MODE_USB,
    EN_UPDATE_MODE_OAD,
    EN_UPDATE_MODE_NET,
    EN_UPDATE_MODE_USB_WITH_SEG,
    EN_UPDATE_MODE_USB_WITH_SEG_DECRYPTED,
    EN_UPDATE_MODE_OAD_WITH_SEG_DECRYPTED,
}en_update_mode;

typedef enum
{
    IMAGE_VERSIOIN_CHECKER_STATE_FAIL = -1,
    IMAGE_VERSIOIN_CHECKER_STATE_SAME,
    IMAGE_VERSIOIN_CHECKER_STATE_UPDATE,
}image_version_checker_state;

//-------------------------------------------------------------------------------------------------
/// Jump to uboot's console from anywhere
//-------------------------------------------------------------------------------------------------
void jump_to_console(void);


//-------------------------------------------------------------------------------------------------
/// get next line in script, and the input data will be modified in this function
/// @return  char*                            \b OUT: a string in the script
//-------------------------------------------------------------------------------------------------
char *get_script_next_line(char **line_buf_ptr);


//-------------------------------------------------------------------------------------------------
/// load entire script to "get next line in script"
/// @return  0 = Success , -1 = Failure
//-------------------------------------------------------------------------------------------------
int runscript_linebyline(char *script_buf);


// temp for usb secure upgrade used
void reset_system(void);


//-------------------------------------------------------------------------------------------------
/// Check if the specified file is in USB or not
/// @param interface     \b IN: the interface which store the upgrade_file
/// @param upgrade_file  \b IN: the specified file you want to check
/// @return  0 = Success , -1 = Failure
//-------------------------------------------------------------------------------------------------
int check_file_exist(const char *interface, char *upgrade_file);

unsigned long get_storage_size(void);
//-------------------------------------------------------------------------------------------------
/// check the usb file exiting & get the device and partition number in the storage
/// @param interface     \b IN: the interface which store the upgrade_file
/// @param upgrade_file  \b IN: the specified file you want to check
/// @return  0 = Success , -1 = Failure
//-------------------------------------------------------------------------------------------------
int check_usb_upgrade_file_available(const char *interface, char *upgrade_file);


//-------------------------------------------------------------------------------------------------
/// init. each USB port on board, and read 1 byte in specified file to check the file exiting
/// @param interface     \b IN: the interface which store the upgrade_file
/// @param device        \b IN: the device number which store the upgrade_file
/// @param partition     \b IN: the partition number which store the upgrade_file
/// @param upgrade_file  \b IN: the specified file you want to check
/// @return  0 = Success , -1 = Failure
//-------------------------------------------------------------------------------------------------
int check_usb_file_partition(const char *interface, int *device, int *partition, char *upgrade_file);
//-------------------------------------------------------------------------------------------------
/// init. each USB port on board, and read 1 byte in specified file to check the file exiting
/// @param interface     \b IN: the interface which store the upgrade_file
/// @param device        \b IN: the device number which store the upgrade_file
/// @param upgrade_info  \b IN: the specified file information partition/name...
/// @return  0 = Success , -1 = Failure
//-------------------------------------------------------------------------------------------------

int check_ext4_upgrade_file_available(const char *interface);

//-------------------------------------------------------------------------------------------------
/// Check if this cmd should be executed or not.
/// @param next_line  \b IN: cmd that will be executed later.
/// @return unsigned char   \b OUT: TRUE: This cmd should be skipped. FALSE:This cmd should be executed.
//-------------------------------------------------------------------------------------------------
unsigned char check_skip_cmd(char *next_line);


//-------------------------------------------------------------------------------------------------
/// setup the script file's size, and this script file is stored in front of the upgrade image.
/// The default size is 0x10000
/// @param  size                            \b IN: The file size of upgrade script file
/// @return  int                             \b OUT: 0: setup size successfully. -1: setup size fail
//-------------------------------------------------------------------------------------------------
int set_scrit_size_of_upgrade_image(unsigned int size);


//-------------------------------------------------------------------------------------------------
/// Set the file name of upgrade image to uboot's environment variable
/// @param  str                             \b IN: The file name of upgrade image
/// @return  int                             \b OUT: 0: set file name successfully. -1: set file name fail
//-------------------------------------------------------------------------------------------------
int set_upgrade_file_name(char *str);


//-------------------------------------------------------------------------------------------------
/// setup current upgrade mode
/// @return  unsigned int                             \b OUT: 0: setup successfully, -1: setup fail
//-------------------------------------------------------------------------------------------------
int set_upgrade_mode(en_update_mode enMode);


//-------------------------------------------------------------------------------------------------
/// delete an environment variable, and this environmet recall the file name of upgrade image
/// @return  int                             \b OUT: 0: delete file name successfully. -1: delete file name fail
//-------------------------------------------------------------------------------------------------
int delete_upgrade_file_name(void);


//-------------------------------------------------------------------------------------------------
/// get the script file's size, and this script file is stored in front of the upgrade image.
/// If no setting, this function will return the default value. Default value is 0x4000 now.
/// @return  unsigned int                             \b OUT: script size of upgrade file
//-------------------------------------------------------------------------------------------------
unsigned int get_scrit_size_of_upgrade_image(void);


//--------------------------------------------------------------------------------------------------
/// get size of the upgrade file
///
/// @param interface  \b IN: the interface you want to upgrade, ex: usb
/// @param device     \b IN: the device number which store the upgrade file
/// @param file       \b IN: the specified file you want to upgrade
/// @return : size of the upgrade file = Success , -1 = Failure
//-------------------------------------------------------------------------------------------------
unsigned int get_file_size_for_upgrade(const char *interface, const char *device, char *file);


//-------------------------------------------------------------------------------------------------
/// get current upgrade mode
/// @return  en_update_mode                      \b OUT:
//-------------------------------------------------------------------------------------------------
en_update_mode get_upgrade_mode(void);


//-------------------------------------------------------------------------------------------------
/// calculate the total amount of commands in the script file
/// PS: This function will destory the datas that are stored in the buffer.
/// @param  buffer                                  \b IN: the buffer for the script file
/// @return  unsigned int                           \b OUT: the total amount of commands
//-------------------------------------------------------------------------------------------------
unsigned int check_upgrade_script_cmd_count(char *buffer);


//-------------------------------------------------------------------------------------------------
/// calculate the total amount of commands in the script file
/// PS: This function will destory the datas that are stored in the buffer.
/// @param  script_buf                                  \b IN: the buffer for the script file
/// @return  unsigned int                               \b OUT: the total amount of commands
//-------------------------------------------------------------------------------------------------
unsigned int calculate_script_cmd_count(char *script_buf);


//-------------------------------------------------------------------------------------------------
/// read upgrade file offset information
/// @param ptr_header     \b IN: the ptr_header which points the header buffer
/// @param upgrade_file  \b IN: the specified file you want to check
/// @return  0 = Success , -1 = Failure
//-------------------------------------------------------------------------------------------------
int read_firmware_image_offset_info(unsigned char *ptr_header, char *upgrade_file);


//-------------------------------------------------------------------------------------------------
/// load upgrade script
/// @param interface     \b IN: the interface which store the upgrade_file
/// @param upgrade_file  \b IN: the specified file you want to check
/// @param download_buf  \b IN: the buffer for the script file
/// @return  0 = Success , -1 = Failure
//-------------------------------------------------------------------------------------------------
int load_ree_script_to_dram(const char *interface, char *upgrade_file, char *download_buf);


//-------------------------------------------------------------------------------------------------
/// Load part of a file to DRAM
/// @return  int                              \b OUT: 0: successful. -1: fail
//-------------------------------------------------------------------------------------------------
int do_file_part_load (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);


//--------------------------------------------------------------------------------------------------
/// do upgrade by specified file
///
/// @param upgrade_file  \b IN: the specified file you want to upgrade
/// @param enMode        \b IN: upgrade mode, usb, oad, ...
/// @param if_type       \b IN: upgrade_file source come from, support usb & mmc & ufs now

/// @return : 0 = Success , 1 = Failure
//-------------------------------------------------------------------------------------------------
int do_upgrade(char *upgrade_file, en_update_mode enMode, enum if_type	if_type);


#if (CONFIG_MTK_UPGRADE == 1)
//--------------------------------------------------------------------------------------------------
/// Mediatek provides different ways to upgrade, including usb, oad, net.
/// If user wants to use these methods to upgrade, user must enable
/// CONFIG_MTK_UPGRADE & corresponding settings (ex: usb for CONFIG_USB_UPGRADE)
///
/// @return : 0 = Success , 1 = Failure
//-------------------------------------------------------------------------------------------------
int do_mtkupgrade(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);
#endif


#if (CONFIG_USB_UPGRADE == 1)
#if (CONFIG_MMC == 1)
//--------------------------------------------------------------------------------------------------
/// Upgrade partition data from usb file
///
/// @return : 0 = Success , 1 = Failure
//-------------------------------------------------------------------------------------------------
int do_usb_partial_upgrade_to_emmc(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);
#endif
#endif

#endif

