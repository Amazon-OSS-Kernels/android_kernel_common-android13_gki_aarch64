#ifndef SCSI_UFS_H
#define SCSI_UFS_H

#include <linux/types.h>
#include <linux/dma-direction.h>

#define MAX_CID_SIZE	16

/*
 * This file intended to be included by both kernel and user space
 */

#define UFS_CDB_SIZE	16

/**
 * struct utp_upiu_header - UPIU header structure
 * @dword_0: UPIU header DW-0
 * @dword_1: UPIU header DW-1
 * @dword_2: UPIU header DW-2
 */
struct utp_upiu_header {
	__be32 dword_0;
	__be32 dword_1;
	__be32 dword_2;
};

/**
 * struct utp_upiu_query - upiu request buffer structure for
 * query request.
 * @opcode: command to perform B-0
 * @idn: a value that indicates the particular type of data B-1
 * @index: Index to further identify data B-2
 * @selector: Index to further identify data B-3
 * @reserved_osf: spec reserved field B-4,5
 * @length: number of descriptor bytes to read/write B-6,7
 * @value: Attribute value to be written DW-5
 * @reserved: spec reserved DW-6,7
 */
struct utp_upiu_query {
	__u8 opcode;
	__u8 idn;
	__u8 index;
	__u8 selector;
	__be16 reserved_osf;
	__be16 length;
	__be32 value;
	__be32 reserved[2];
};

/**
 * struct utp_upiu_cmd - Command UPIU structure
 * @data_transfer_len: Data Transfer Length DW-3
 * @cdb: Command Descriptor Block CDB DW-4 to DW-7
 */
struct utp_upiu_cmd {
	__be32 exp_data_transfer_len;
	__u8 cdb[UFS_CDB_SIZE];
};

/**
 * struct utp_upiu_req - general upiu request structure
 * @header:UPIU header structure DW-0 to DW-2
 * @sc: fields structure for scsi command DW-3 to DW-7
 * @qr: fields structure for query request DW-3 to DW-7
 */
struct utp_upiu_req {
	struct utp_upiu_header header;
	union {
		struct utp_upiu_cmd		sc;
		struct utp_upiu_query		qr;
		struct utp_upiu_query		tr;
		/* use utp_upiu_query to host the 4 dwords of uic command */
		struct utp_upiu_query		uc;
	};
};

/*
 * The Well Known LUNS (SAM-3) in our int representation of a LUN
 */
#define SCSI_W_LUN_BASE 0xc100
#define SCSI_W_LUN_REPORT_LUNS (SCSI_W_LUN_BASE + 1)
#define SCSI_W_LUN_ACCESS_CONTROL (SCSI_W_LUN_BASE + 2)
#define SCSI_W_LUN_TARGET_LOG_PAGE (SCSI_W_LUN_BASE + 3)
#define SCSI_W_LUN_RPMB (SCSI_W_LUN_BASE + 0x44)

static inline int scsi_is_wlun(u64 lun)
{
	return (lun & 0xff00) == SCSI_W_LUN_BASE;
}

/*
 *  MESSAGE CODES
 */

#define COMMAND_COMPLETE    0x00
#define EXTENDED_MESSAGE    0x01
#define     EXTENDED_MODIFY_DATA_POINTER    0x00
#define     EXTENDED_SDTR                   0x01
#define     EXTENDED_EXTENDED_IDENTIFY      0x02    /* SCSI-I only */
#define     EXTENDED_WDTR                   0x03
#define     EXTENDED_PPR                    0x04
#define     EXTENDED_MODIFY_BIDI_DATA_PTR   0x05
#define SAVE_POINTERS       0x02
#define RESTORE_POINTERS    0x03
#define DISCONNECT          0x04
#define INITIATOR_ERROR     0x05
#define ABORT_TASK_SET      0x06
#define MESSAGE_REJECT      0x07
#define NOP                 0x08
#define MSG_PARITY_ERROR    0x09
#define LINKED_CMD_COMPLETE 0x0a
#define LINKED_FLG_CMD_COMPLETE 0x0b
#define TARGET_RESET        0x0c
#define ABORT_TASK          0x0d
#define CLEAR_TASK_SET      0x0e
#define INITIATE_RECOVERY   0x0f            /* SCSI-II only */
#define RELEASE_RECOVERY    0x10            /* SCSI-II only */
#define CLEAR_ACA           0x16
#define LOGICAL_UNIT_RESET  0x17
#define SIMPLE_QUEUE_TAG    0x20
#define HEAD_OF_QUEUE_TAG   0x21
#define ORDERED_QUEUE_TAG   0x22
#define IGNORE_WIDE_RESIDUE 0x23
#define ACA                 0x24
#define QAS_REQUEST         0x55

/* Old SCSI2 names, don't use in new code */
#define BUS_DEVICE_RESET    TARGET_RESET
#define ABORT               ABORT_TASK_SET

/*
 * Host byte codes
 */
#define host_byte(result)   (((result) >> 16) & 0xff)

#define DID_OK          0x00	/* NO error                                */
#define DID_NO_CONNECT  0x01	/* Couldn't connect before timeout period  */
#define DID_BUS_BUSY    0x02	/* BUS stayed busy through time out period */
#define DID_TIME_OUT    0x03	/* TIMED OUT for other reason              */
#define DID_BAD_TARGET  0x04	/* BAD target.                             */
#define DID_ABORT       0x05	/* Told to abort for some other reason     */
#define DID_PARITY      0x06	/* Parity error                            */
#define DID_ERROR       0x07	/* Internal error                          */
#define DID_RESET       0x08	/* Reset by somebody.                      */
#define DID_BAD_INTR    0x09	/* Got an interrupt we weren't expecting.  */
#define DID_PASSTHROUGH 0x0a	/* Force command past mid-layer            */
#define DID_SOFT_ERROR  0x0b	/* The low level driver just wish a retry  */
#define DID_IMM_RETRY   0x0c	/* Retry without decrementing retry count  */
#define DID_REQUEUE	0x0d	/* Requeue command (no immediate retry) also
				 * without decrementing the retry count	   */
#define DID_TRANSPORT_DISRUPTED 0x0e /* Transport error disrupted execution
				      * and the driver blocked the port to
				      * recover the link. Transport class will
				      * retry or fail IO */
#define DID_TRANSPORT_FAILFAST	0x0f /* Transport class fastfailed the io */
#define DID_TARGET_FAILURE 0x10 /* Permanent target failure, do not retry on
				 * other paths */
#define DID_NEXUS_FAILURE 0x11  /* Permanent nexus failure, retry on other
				 * paths might yield different results */
#define DID_ALLOC_FAILURE 0x12  /* Space allocation on the device failed */
#define DID_MEDIUM_ERROR  0x13  /* Medium error */
#define DRIVER_OK       0x00	/* Driver status                           */

/*
 *      SCSI opcodes
 */
#define TEST_UNIT_READY       0x00
#define REQUEST_SENSE         0x03
#define FORMAT_UNIT           0x04
#define READ_6                0x08
#define WRITE_6               0x0a
#define INQUIRY               0x12
#define MODE_SELECT           0x15
#define MODE_SENSE            0x1a
#define START_STOP            0x1b
#define SEND_DIAGNOSTIC       0x1d
#define READ_CAPACITY         0x25
#define READ_10               0x28
#define WRITE_10              0x2a
#define VERIFY                0x2f
#define PRE_FETCH             0x34
#define SYNCHRONIZE_CACHE     0x35
#define WRITE_BUFFER          0x3b
#define READ_BUFFER           0x3c
#define UNMAP		          0x42
#define MODE_SELECT_10        0x55
#define MODE_SENSE_10         0x5a
#define REPORT_LUNS           0xa0
#define SECURITY_PROTOCOL_IN  0xa2
#define SECURITY_PROTOCOL_OUT 0xb5
#define READ_16               0x88
#define WRITE_16              0x8a
#define SYNCHRONIZE_CACHE_16  0x91

#define SECURITY_PROTOCOL             0xEC
#define SECURITY_PROTOCOL_SPECIFIC    0x1

/*
 *  SCSI Architecture Model (SAM) Status codes. Taken from SAM-3 draft
 *  T10/1561-D Revision 4 Draft dated 7th November 2002.
 */
#define SAM_STAT_GOOD            0x00
#define SAM_STAT_CHECK_CONDITION 0x02
#define SAM_STAT_CONDITION_MET   0x04
#define SAM_STAT_BUSY            0x08
#define SAM_STAT_INTERMEDIATE    0x10
#define SAM_STAT_INTERMEDIATE_CONDITION_MET 0x14
#define SAM_STAT_RESERVATION_CONFLICT 0x18
#define SAM_STAT_COMMAND_TERMINATED 0x22	/* obsolete in SAM-3 */
#define SAM_STAT_TASK_SET_FULL   0x28
#define SAM_STAT_ACA_ACTIVE      0x30
#define SAM_STAT_TASK_ABORTED    0x40

/*
 *  SENSE KEYS
 */

#define NO_SENSE            0x00
#define RECOVERED_ERROR     0x01
#define NOT_READY           0x02
#define MEDIUM_ERROR        0x03
#define HARDWARE_ERROR      0x04
#define ILLEGAL_REQUEST     0x05
#define UNIT_ATTENTION      0x06
#define DATA_PROTECT        0x07
#define BLANK_CHECK         0x08
#define VENDOR_SPECIFIC     0x09
#define COPY_ABORTED        0x0a
#define ABORTED_COMMAND     0x0b
#define VOLUME_OVERFLOW     0x0d
#define MISCOMPARE          0x0e

/*
 * ScsiLun: 8 byte LUN.
 */
struct scsi_lun {
	__u8 scsi_lun[8];
};

#define SCSI_SENSE_BUFFERSIZE 	 18

struct scsi_cmnd {
	unsigned char			sense_buffer[SCSI_SENSE_BUFFERSIZE];
	unsigned long			sensedatalen;
	int						tag;
	int 					result;
	unsigned int			lun;
	unsigned char			cmnd[UFS_CDB_SIZE];
	unsigned char			cmd_len;
	unsigned char			*pdata;
	unsigned long			datalen;
	enum dma_data_direction	sc_data_direction;
};

struct ufs_config_descr {
	u8 bLength;
	u8 bDescriptorDN;
	u8 bConfDescContinue;
	u8 bBootEnable;
	u8 bDescrAccessEn;
	u8 bInitPowerMode;
	u8 bHighPriorityLUN;
	u8 bSecureRemovalType;
	u8 bInitActiveICCLevel;
	u16 wPeriodicRTCUpdate;
	u8 Reserved;
	u8 bRPMBRegionEnable;
	u8 bRPMBRegion1Size;
	u8 bRPMBRegion2Size;
	u8 bRPMBRegion3Size;
	u8 bWBBufPreserveUserSpaceEn;
	u8 bWBBufType;
	u32 dNumSharedWBBufAllocUnits;
} __attribute__((__packed__));

struct ufs_unit_descr{
	u8 bLUEnable;
	u8 bBootLunID;
	u8 bLUWriteProtect;
	u8 bMemoryType;
	u32 dNumAllocUnits;
	u8 bDataReliability;
	u8 bLogicalBlockSize;
	u8 bProvisioningType;
	u16 wContextCapabilities;
	u8 Reserved[];
} __attribute__((__packed__));

struct ufs_hba;
enum query_opcode;
enum attr_idn;
enum desc_idn;

int ufs_scan(bool verbose);
int get_ufs_num(void);
struct ufs_hba *find_ufs_device(int dev_num);

int ufs_get_cid(struct ufs_hba *hba, u8 *cid);
void ufs_crypto_enable(struct ufs_hba *hba);
void ufs_crypto_disable(struct ufs_hba *hba);
int ufs_rpmb_set_key(struct ufs_hba *hba, void *key);
int ufs_rpmb_get_counter(struct ufs_hba *hba, u32 *counter);
int ufs_rpmb_read(struct ufs_hba *hba, void *addr, unsigned short blk,
		  unsigned short cnt, unsigned char *key);
int ufs_rpmb_write(struct ufs_hba *hba, void *addr, unsigned short blk,
		   unsigned short cnt, unsigned char *key);
int ufs_rpmb_read_blk(void *databuf, unsigned char *nonce, unsigned short blk);
int ufs_rpmb_write_blk(void *databuf);
int ufs_rpmb_if_key_written(void);
int ufs_rpmb_get_counter_for_cmd(u32 *pcounter);
int ufs_rpmb_route_frames(struct ufs_hba *hba, void *req, unsigned long reqlen,
			  void *rsp, unsigned long rsplen);

int ufs_read_attr(struct ufs_hba *hba, int idn, u32 *attr_val);
int ufs_write_attr(struct ufs_hba *hba, int idn, u32 attr_val);
int ufs_read_flag(struct ufs_hba *hba, int idn, u32 *flag_res);
int ufs_write_flag(struct ufs_hba *hba, int idn, u32 flag_res);
int ufs_read_desc(struct ufs_hba *hba, int desc_id, int desc_index,
					u8 *param_read_buf, u8 param_size);
int ufs_write_desc(struct ufs_hba *hba, int desc_id, int desc_index,
					u8 *param_write_buf, u8 param_size);
int ufshcd_write_configuration_desc(struct ufs_hba *hba, int desc_index, u8 *buf, u32 size);
int ufshcd_read_geometry_desc(struct ufs_hba *hba, u8 *buf, u32 size);
int ufshcd_query_attr(struct ufs_hba *hba, enum query_opcode opcode,
		      enum attr_idn idn, u8 index, u8 selector, u32 *attr_val);
int ufshcd_map_desc_id_to_length(struct ufs_hba *hba,
	enum desc_idn desc_id, int *desc_len);

#endif /* SCSI_UFS_H */
