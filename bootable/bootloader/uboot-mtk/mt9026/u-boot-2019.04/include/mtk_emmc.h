#ifndef __MTK_EMMC_H__
#define __MTK_EMMC_H__

#ifndef U32
#define U32  unsigned int
#endif
#ifndef U16
#define U16  unsigned short
#endif
#ifndef U8
#define U8   unsigned char
#endif
#ifndef S32
#define S32  signed int
#endif
#ifndef S16
#define S16  signed short
#endif
#ifndef S8
#define S8   signed char
#endif

/*=============================================================*/
// Include files
/*=============================================================*/

/*=============================================================*/
// Extern definition
/*=============================================================*/

/*=============================================================*/
// Macro definition
/*=============================================================*/
#define RPMB_BLOCK_SIZE         512
#define RPMB_NONCE_SIZE         16
#define RPMB_UBOOT_MAX_SIZE     128
#define RPMB_DATA_SIZE          256
/*=============================================================*/
// Data type definition
/*=============================================================*/
struct EMMC_RPMB_DATA
{
	U16 u16_req_rsp;
	U16 u16_result;
	U16 u16_blk_cnt;
	U16 u16_addr;
	U32 u32_writecnt;
	U8  u8_nonce[16];
	U8  u8_data[256];
	U8  u8_auth_key[32];
	U8  u8_stuff[196];
} __attribute__((__packed__));

/* Make sure aligned with tee loader & sboot */
struct rpmb_fs_partition {
	uint32_t rpmb_fs_magic;
	uint32_t fs_version;
	uint32_t write_counter;
	uint32_t fat_start_address;
	/* Do not use reserved[] for other purpose than partition data. */
	uint8_t reserved[39];
	uint8_t  ce;	// the sony console lock & log enable bit
	uint8_t  unique_itvid[24];
	uint8_t  magicString[16];
	uint32_t uboot_version;
	uint32_t hash1_version;
	uint32_t teekeybank_version;
	uint32_t teeloader_version;
	uint32_t reeloader_version;
	uint32_t optee_version;
	uint32_t armfw_version;
	uint32_t anti_rollback_init_flag;
};

/*=============================================================*/
// Variable definition
/*=============================================================*/

/*=============================================================*/
// Global function definition
/*=============================================================*/

U32 emmc_rpmb_get_counter_for_cmd(U32 *pu32_wcnt);
U32 emmc_rpmb_if_key_written(void);
U32 emmc_rpmb_read_blk(U8 *pu8_data_buf, U8 *pu8_nonce, U16 u16_blk_addr);
U32 emmc_rpmb_write_blk(U8 *pu8_data_buf);
U32 emmc_get_cid(U8 *pu8_cid);

#endif //__MTK_MMC_H__

