// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2014, Staubli Faverges
 * Pierre Aubert
 *
 * UFS- Replay Protected Memory Block
 * According to JEDEC Standard No. 84-A441
 */

#include <config.h>
#include <common.h>
#include <memalign.h>
#include <ufs_scsi.h>
#include <u-boot/sha256.h>
#include "ufshcd.h"
#include <hexdump.h>

/* Request codes */
#define RPMB_REQ_KEY			1
#define RPMB_REQ_WCOUNTER		2
#define RPMB_REQ_WRITE_DATA		3
#define RPMB_REQ_READ_DATA		4
#define RPMB_REQ_STATUS			5

/* Response code */
#define RPMB_RESP_KEY			0x0100
#define RPMB_RESP_WCOUNTER		0x0200
#define RPMB_RESP_WRITE_DATA	0x0300
#define RPMB_RESP_READ_DATA		0x0400

/* Error codes */
#define RPMB_OK					0
#define RPMB_ERR_GENERAL		1
#define RPMB_ERR_AUTH			2
#define RPMB_ERR_COUNTER		3
#define RPMB_ERR_ADDRESS		4
#define RPMB_ERR_WRITE			5
#define RPMB_ERR_READ			6
#define RPMB_ERR_KEY			7
#define RPMB_ERR_CNT_EXPIRED	0x80
#define RPMB_ERR_MSK			0x7

/* Sizes of RPMB data frame */
#define RPMB_SZ_STUFF			196
#define RPMB_SZ_MAC				32
#define RPMB_SZ_DATA			256
#define RPMB_SZ_NONCE			16

#define SHA256_BLOCK_SIZE		64

/* Error messages */
static const char * const rpmb_err_msg[] = {
	"",
	"General failure",
	"Authentication failure",
	"Counter failure",
	"Address failure",
	"Write failure",
	"Read failure",
	"Authentication key not yet programmed",
};


/* Structure of RPMB data frame. */
struct s_rpmb {
	unsigned char stuff[RPMB_SZ_STUFF];
	unsigned char mac[RPMB_SZ_MAC];
	unsigned char data[RPMB_SZ_DATA];
	unsigned char nonce[RPMB_SZ_NONCE];
	unsigned int write_counter;
	unsigned short address;
	unsigned short block_count;
	unsigned short result;
	unsigned short request;
};

extern int ufs_curr_target; /* current target */
static struct scsi_cmnd tempccb;	/* temporary scsi command buffer */

static void scsi_setup_security_protocol_in(struct scsi_cmnd *pccb, u32 tag, u32 blk_cnt)
{
	memset(pccb->cmnd, 0, UFS_CDB_SIZE);

	pccb->lun = SCSI_W_LUN_RPMB;
	pccb->tag = tag;
	pccb->cmnd[0] = SECURITY_PROTOCOL_IN;
	pccb->cmnd[1] = SECURITY_PROTOCOL;
	pccb->cmnd[2] = (SECURITY_PROTOCOL_SPECIFIC >> 8) & 0xFF;
	pccb->cmnd[3] = SECURITY_PROTOCOL_SPECIFIC & 0xFF;
	pccb->cmnd[4] = 0x0;
	pccb->cmnd[5] = 0x0;
	pccb->cmnd[6] = ((blk_cnt  << 9) >> 24) & 0xFF;
	pccb->cmnd[7] = ((blk_cnt  << 9) >> 16) & 0xFF;;
	pccb->cmnd[8] = ((blk_cnt  << 9) >> 8) & 0xFF;
	pccb->cmnd[9] = (blk_cnt  << 9) & 0xFF;
	pccb->cmnd[10] = 0x0;
	pccb->cmnd[11] = 0x0;
	pccb->cmd_len = 12;
	pccb->sc_data_direction = DMA_FROM_DEVICE;
}

static void scsi_setup_security_protocol_out(struct scsi_cmnd *pccb, u32 tag, u32 blk_cnt)
{
	memset(pccb->cmnd, 0, UFS_CDB_SIZE);

	pccb->lun = SCSI_W_LUN_RPMB;
	pccb->tag = tag;
	pccb->cmnd[0] = SECURITY_PROTOCOL_OUT;
	pccb->cmnd[1] = SECURITY_PROTOCOL;
	pccb->cmnd[2] = (SECURITY_PROTOCOL_SPECIFIC >> 8) & 0xFF;
	pccb->cmnd[3] = SECURITY_PROTOCOL_SPECIFIC & 0xFF;
	pccb->cmnd[4] = 0x0;
	pccb->cmnd[5] = 0x0;
	pccb->cmnd[6] = ((blk_cnt  << 9) >> 24) & 0xFF;
	pccb->cmnd[7] = ((blk_cnt  << 9) >> 16) & 0xFF;;
	pccb->cmnd[8] = ((blk_cnt  << 9) >> 8) & 0xFF;
	pccb->cmnd[9] = (blk_cnt  << 9) & 0xFF;
	pccb->cmnd[10] = 0x0;
	pccb->cmnd[11] = 0x0;
	pccb->cmd_len = 12;
	pccb->sc_data_direction = DMA_TO_DEVICE;
}

static int ufs_rpmb_response(struct s_rpmb *s, unsigned short expected)
{
	int ret;

	/* Check the response and the status */
	if (be16_to_cpu(s->request) != expected) {
		printf("%s:response= %x\n", __func__,
		       be16_to_cpu(s->request));
		return -1;
	}
	ret = be16_to_cpu(s->result);
	if (ret) {
		printf("%s %s\n", rpmb_err_msg[ret & RPMB_ERR_MSK],
		       (ret & RPMB_ERR_CNT_EXPIRED) ?
		       "Write counter has expired" : "");
	}

	/* Return the status of the command */
	return ret;
}

static void rpmb_hmac(struct ufs_hba *hba, unsigned char *key, unsigned char *buff,
		      int len, unsigned char *output)
{
	sha256_context ctx;
	int i;
	unsigned char k_ipad[SHA256_BLOCK_SIZE];
	unsigned char k_opad[SHA256_BLOCK_SIZE];

	if (hba->vops && hba->vops->rpmb_hmac)
		return hba->vops->rpmb_hmac(hba, key, buff, len, output);

	sha256_starts(&ctx);

	/* According to RFC 4634, the HMAC transform looks like:
	   SHA(K XOR opad, SHA(K XOR ipad, text))

	   where K is an n byte key.
	   ipad is the byte 0x36 repeated blocksize times
	   opad is the byte 0x5c repeated blocksize times
	   and text is the data being protected.
	*/

	for (i = 0; i < RPMB_SZ_MAC; i++) {
		k_ipad[i] = key[i] ^ 0x36;
		k_opad[i] = key[i] ^ 0x5c;
	}
	/* remaining pad bytes are '\0' XOR'd with ipad and opad values */
	for ( ; i < SHA256_BLOCK_SIZE; i++) {
		k_ipad[i] = 0x36;
		k_opad[i] = 0x5c;
	}
	sha256_update(&ctx, k_ipad, SHA256_BLOCK_SIZE);
	sha256_update(&ctx, buff, len);
	sha256_finish(&ctx, output);

	/* Init context for second pass */
	sha256_starts(&ctx);

	/* start with outer pad */
	sha256_update(&ctx, k_opad, SHA256_BLOCK_SIZE);

	/* then results of 1st hash */
	sha256_update(&ctx, output, RPMB_SZ_MAC);

	/* finish up 2nd pass */
	sha256_finish(&ctx, output);
}

int ufs_rpmb_get_counter(struct ufs_hba *hba, u32 *pcounter)
{
	int ret;
	struct scsi_cmnd *pccb = (struct scsi_cmnd *)&tempccb;
	ALLOC_CACHE_ALIGN_BUFFER(struct s_rpmb, rpmb_frame, 1);

	/* Write counter read request */
	memset(rpmb_frame, 0, sizeof(struct s_rpmb));
	rpmb_frame->request = cpu_to_be16(RPMB_REQ_WCOUNTER);
	scsi_setup_security_protocol_out(pccb, 0, 1);
	pccb->pdata = (unsigned char *)rpmb_frame;
	pccb->datalen = 512;
	#ifdef CONFIG_UFS_DEBUG
	print_hex_dump("RPMB_REQ_WCOUNTER", DUMP_PREFIX_ADDRESS, 16, 1,
					(unsigned char *)rpmb_frame, 512, false);
	#endif
	if(ufshcd_queuecommand(hba, pccb))
		return -1;

	/* Write counter read response */
	memset(rpmb_frame, 0, sizeof(struct s_rpmb));
	scsi_setup_security_protocol_in(pccb, 0, 1);
	pccb->pdata = (unsigned char *)rpmb_frame;
	pccb->datalen = 512;
	if(ufshcd_queuecommand(hba, pccb))
		return -1;
	#ifdef CONFIG_UFS_DEBUG
	print_hex_dump("RPMB_RESP_WCOUNTER", DUMP_PREFIX_ADDRESS, 16, 1,
					(unsigned char *)rpmb_frame, 512, false);
	#endif

	/* Check the result */
	ret = ufs_rpmb_response(rpmb_frame, RPMB_RESP_WCOUNTER);
	if(ret == 0)
		*pcounter = be32_to_cpu(rpmb_frame->write_counter);

	return ret;
}

int ufs_rpmb_set_key(struct ufs_hba *hba, void *key)
{
	struct scsi_cmnd *pccb = (struct scsi_cmnd *)&tempccb;
	ALLOC_CACHE_ALIGN_BUFFER(struct s_rpmb, rpmb_frame, 1);

	/* Authentication key programming request */
	memset(rpmb_frame, 0, sizeof(struct s_rpmb));
	rpmb_frame->request = cpu_to_be16(RPMB_REQ_KEY);
	memcpy(rpmb_frame->mac, key, RPMB_SZ_MAC);
	scsi_setup_security_protocol_out(pccb, 0, 1);
	pccb->pdata = (unsigned char *)rpmb_frame;
	pccb->datalen = 512;
	#ifdef CONFIG_UFS_DEBUG
	print_hex_dump("RPMB_REQ_KEY", DUMP_PREFIX_ADDRESS, 16, 1,
					(unsigned char *)rpmb_frame, 512, false);
	#endif
	if(ufshcd_queuecommand(hba, pccb))
		return -1;

	/* Result read request */
	memset(rpmb_frame, 0, sizeof(struct s_rpmb));
	rpmb_frame->request = cpu_to_be16(RPMB_REQ_STATUS);
	scsi_setup_security_protocol_out(pccb, 0, 1);
	pccb->pdata = (unsigned char *)rpmb_frame;
	pccb->datalen = 512;
	#ifdef CONFIG_UFS_DEBUG
	print_hex_dump("RPMB_REQ_STATUS", DUMP_PREFIX_ADDRESS, 16, 1,
					(unsigned char *)rpmb_frame, 512, false);
	#endif
	if(ufshcd_queuecommand(hba, pccb))
		return -1;

	/* Result read response */
	memset(rpmb_frame, 0, sizeof(struct s_rpmb));
	scsi_setup_security_protocol_in(pccb, 0, 1);
	pccb->pdata = (unsigned char *)rpmb_frame;
	pccb->datalen = 512;
	if(ufshcd_queuecommand(hba, pccb))
		return -1;
	#ifdef CONFIG_UFS_DEBUG
	print_hex_dump("RPMB_RESP_KEY", DUMP_PREFIX_ADDRESS, 16, 1,
					(unsigned char *)rpmb_frame, 512, false);
	#endif

	/* Check the result */
	return(ufs_rpmb_response(rpmb_frame, RPMB_RESP_KEY));
}

int ufs_rpmb_read(struct ufs_hba *hba, void *addr, unsigned short blk,
		  unsigned short cnt, unsigned char *key)
{
	struct scsi_cmnd *pccb = (struct scsi_cmnd *)&tempccb;
	ALLOC_CACHE_ALIGN_BUFFER(struct s_rpmb, rpmb_frame, 1);
	int i;

	for (i = 0; i < cnt; i++) {
		/* Authentication data read request */
		memset(rpmb_frame, 0, sizeof(struct s_rpmb));
		rpmb_frame->address = cpu_to_be16(blk + i);
		rpmb_frame->block_count = cpu_to_be16(1);
		rpmb_frame->request = cpu_to_be16(RPMB_REQ_READ_DATA);
		scsi_setup_security_protocol_out(pccb, 0, 1);
		pccb->pdata = (unsigned char *)rpmb_frame;
		pccb->datalen = 512;
		#ifdef CONFIG_UFS_DEBUG
		print_hex_dump("RPMB_REQ_READ_DATA", DUMP_PREFIX_ADDRESS, 16, 1,
						(unsigned char *)rpmb_frame, 512, false);
		#endif
		if(ufshcd_queuecommand(hba, pccb))
			return -1;

		/* Authentication data read response */
		memset(rpmb_frame, 0, sizeof(struct s_rpmb));
		scsi_setup_security_protocol_in(pccb, 0, 1);
		pccb->pdata = (unsigned char *)rpmb_frame;
		pccb->datalen = 512;
		if(ufshcd_queuecommand(hba, pccb))
			return -1;
		#ifdef CONFIG_UFS_DEBUG
		print_hex_dump("RPMB_RESP_READ_DATA", DUMP_PREFIX_ADDRESS, 16, 1,
						(unsigned char *)rpmb_frame, 512, false);
		#endif

		/* Check the result */
		if (ufs_rpmb_response(rpmb_frame, RPMB_RESP_READ_DATA))
			break;

		/* Check the HMAC if key is provided */
		if (key) {
			unsigned char ret_hmac[RPMB_SZ_MAC];

			rpmb_hmac(hba, key, rpmb_frame->data, 284, ret_hmac);
			if (memcmp(ret_hmac, rpmb_frame->mac, RPMB_SZ_MAC)) {
				printf("MAC error on block #%d\n", i);
				break;
			}
		}
		/* Copy data */
		memcpy(addr + i * RPMB_SZ_DATA, rpmb_frame->data, RPMB_SZ_DATA);
	}
	return i;
}

int ufs_rpmb_write(struct ufs_hba *hba, void *addr, unsigned short blk,
		  unsigned short cnt, unsigned char *key)
{
	struct scsi_cmnd *pccb = (struct scsi_cmnd *)&tempccb;
	ALLOC_CACHE_ALIGN_BUFFER(struct s_rpmb, rpmb_frame, 1);
	u32 wcount;
	int i;

	for (i = 0; i < cnt; i++) {
		if (ufs_rpmb_get_counter(hba, &wcount)) {
			printf("Cannot read RPMB write counter\n");
			break;
		}

		/* Authentication data write request */
		memset(rpmb_frame, 0, sizeof(struct s_rpmb));
		memcpy(rpmb_frame->data, addr + i * RPMB_SZ_DATA, RPMB_SZ_DATA);
		rpmb_frame->address = cpu_to_be16(blk + i);
		rpmb_frame->block_count = cpu_to_be16(1);
		rpmb_frame->write_counter = cpu_to_be32(wcount);
		rpmb_frame->request = cpu_to_be16(RPMB_REQ_WRITE_DATA);
		/* Computes HMAC */
		rpmb_hmac(hba, key, rpmb_frame->data, 284, rpmb_frame->mac);
		scsi_setup_security_protocol_out(pccb, 0, 1);
		pccb->pdata = (unsigned char *)rpmb_frame;
		pccb->datalen = 512;
		#ifdef CONFIG_UFS_DEBUG
		print_hex_dump("RPMB_REQ_WRITE_DATA", DUMP_PREFIX_ADDRESS, 16, 1,
						(unsigned char *)rpmb_frame, 512, false);
		#endif
		if(ufshcd_queuecommand(hba, pccb))
			return -1;

		/* Result read request */
		memset(rpmb_frame, 0, sizeof(struct s_rpmb));
		rpmb_frame->request = cpu_to_be16(RPMB_REQ_STATUS);
		scsi_setup_security_protocol_out(pccb, 0, 1);
		pccb->pdata = (unsigned char *)rpmb_frame;
		pccb->datalen = 512;
		#ifdef CONFIG_UFS_DEBUG
		print_hex_dump("RPMB_REQ_STATUS", DUMP_PREFIX_ADDRESS, 16, 1,
						(unsigned char *)rpmb_frame, 512, false);
		#endif
		if(ufshcd_queuecommand(hba, pccb))
			return -1;

		/* Result read response */
		memset(rpmb_frame, 0, sizeof(struct s_rpmb));
		scsi_setup_security_protocol_in(pccb, 0, 1);
		pccb->pdata = (unsigned char *)rpmb_frame;
		pccb->datalen = 512;
		if(ufshcd_queuecommand(hba, pccb))
			return -1;
		#ifdef CONFIG_UFS_DEBUG
		print_hex_dump("RPMB_RESP_WRITE_DATA", DUMP_PREFIX_ADDRESS, 16, 1,
						(unsigned char *)rpmb_frame, 512, false);
		#endif

		/* Check the result */
		if (ufs_rpmb_response(rpmb_frame, RPMB_RESP_WRITE_DATA))
			break;
	}
	return i;
}

int ufs_rpmb_read_blk(void *databuf, unsigned char *nonce, unsigned short blk)
{
    int err = 0;
	struct ufs_hba *hba = find_ufs_device(ufs_curr_target);
	struct scsi_cmnd *pccb = (struct scsi_cmnd *)&tempccb;
	struct s_rpmb *rpmb_frame = (struct s_rpmb *)databuf;

    if (rpmb_frame == NULL) {
		printf("[%s] NULL rpmb_frame\n", __func__);
		return -EINVAL;
	}

	/* Authentication data read request */
	rpmb_frame->address = cpu_to_be16(blk);
	rpmb_frame->block_count = cpu_to_be16(1);
	if (nonce)
		memcpy(rpmb_frame->nonce, nonce, RPMB_SZ_NONCE);
	rpmb_frame->request = cpu_to_be16(RPMB_REQ_READ_DATA);
	scsi_setup_security_protocol_out(pccb, 0, 1);
	pccb->pdata = (unsigned char *)rpmb_frame;
	pccb->datalen = 512;
	err = ufshcd_queuecommand(hba, pccb);
	if (err) {
		printf("[%s] Authentication data read request return %d\n", __func__, err);
		return err;
	}

	/* Authentication data read response */
	memset(rpmb_frame, 0, sizeof(struct s_rpmb));
	scsi_setup_security_protocol_in(pccb, 0, 1);
	pccb->pdata = (unsigned char *)rpmb_frame;
	pccb->datalen = 512;
	err = ufshcd_queuecommand(hba, pccb);
	if (err) {
		printf("[%s] Authentication data read response return %d\n", __func__, err);
		return err;
	}

	/* Check the result */
	err = ufs_rpmb_response(rpmb_frame, RPMB_RESP_READ_DATA);
	if (err) {
		printf("[%s] Check the result return %d\n", __func__, err);
		return err;
	}

	return err;
}

int ufs_rpmb_write_blk(void *databuf)
{
    int err = 0;
	struct ufs_hba *hba = find_ufs_device(ufs_curr_target);
	struct scsi_cmnd *pccb = (struct scsi_cmnd *)&tempccb;
	struct s_rpmb *rpmb_frame = (struct s_rpmb *)databuf;

    if (rpmb_frame == NULL) {
		printf("[%s] NULL rpmb_frame\n", __func__);
		return -EINVAL;
	}

	/* Authentication data write request */
	scsi_setup_security_protocol_out(pccb, 0, 1);
	pccb->pdata = (unsigned char *)rpmb_frame;
	pccb->datalen = 512;
	err = ufshcd_queuecommand(hba, pccb);
	if (err) {
		printf("[%s] Authentication data write request return %d\n", __func__, err);
		return err;
	}

	/* Result read request */
	memset(rpmb_frame, 0, sizeof(struct s_rpmb));
	rpmb_frame->request = cpu_to_be16(RPMB_REQ_STATUS);
	scsi_setup_security_protocol_out(pccb, 0, 1);
	pccb->pdata = (unsigned char *)rpmb_frame;
	pccb->datalen = 512;
	err = ufshcd_queuecommand(hba, pccb);
	if (err) {
		printf("[%s] Result read request return %d\n", __func__, err);
		return err;
	}

	/* Result read response */
	memset(rpmb_frame, 0, sizeof(struct s_rpmb));
	scsi_setup_security_protocol_in(pccb, 0, 1);
	pccb->pdata = (unsigned char *)rpmb_frame;
	pccb->datalen = 512;
	err = ufshcd_queuecommand(hba, pccb);
	if (err) {
		printf("[%s] Result read response return %d\n", __func__, err);
		return err;
	}

	/* Check the result */
	err = ufs_rpmb_response(rpmb_frame, RPMB_RESP_READ_DATA);
	if (err) {
		printf("[%s] Check the result return %d\n", __func__, err);
		return err;
	}

	return err;
}

int ufs_rpmb_if_key_written(void)
{
    int err = 0;
	struct ufs_hba *hba = find_ufs_device(ufs_curr_target);
	u32 counter;

	err = ufs_rpmb_get_counter(hba, &counter);
	if (err == 0)
		return 1;
	else if (err == RPMB_ERR_KEY)
		return 0;
	else {
		printf("[%s] Get counter return %d\n", __func__, err);
		return -1;
	}
}

int ufs_rpmb_get_counter_for_cmd(u32 *pcounter)
{
    int err = 0;
	struct ufs_hba *hba = find_ufs_device(ufs_curr_target);

	err = ufs_rpmb_get_counter(hba, pcounter);
	if (err)
		printf("[%s] Get counter return %d\n", __func__, err);

	return err;
}

static int rpmb_route_write_req(struct ufs_hba *hba, struct s_rpmb *req,
				unsigned short req_cnt, struct s_rpmb *rsp,
				unsigned short rsp_cnt)
{
	struct scsi_cmnd *pccb = (struct scsi_cmnd *)&tempccb;

	/* Authentication data write request */
	scsi_setup_security_protocol_out(pccb, 0, req_cnt);
	pccb->pdata = (unsigned char *)req;
	pccb->datalen = 512*req_cnt;
	#ifdef CONFIG_UFS_DEBUG
	print_hex_dump("RPMB_REQ_WRITE_DATA", DUMP_PREFIX_ADDRESS, 16, 1,
					(unsigned char *)req, 512*req_cnt, false);
	#endif
	if(ufshcd_queuecommand(hba, pccb))
		return -1;

	/* Result read request */
	memset(rsp, 0, sizeof(*rsp));
	rsp->request = cpu_to_be16(RPMB_REQ_STATUS);
	scsi_setup_security_protocol_out(pccb, 0, 1);
	pccb->pdata = (unsigned char *)rsp;
	pccb->datalen = 512;
	#ifdef CONFIG_UFS_DEBUG
	print_hex_dump("RPMB_REQ_STATUS", DUMP_PREFIX_ADDRESS, 16, 1,
					(unsigned char *)rsp, 512, false);
	#endif
	if(ufshcd_queuecommand(hba, pccb))
		return -1;

	/* Result read response */
	memset(rsp, 0, sizeof(struct s_rpmb));
	scsi_setup_security_protocol_in(pccb, 0, 1);
	pccb->pdata = (unsigned char *)rsp;
	pccb->datalen = 512;
	if(ufshcd_queuecommand(hba, pccb))
		return -1;
	#ifdef CONFIG_UFS_DEBUG
	print_hex_dump("RPMB_RESP_WRITE_DATA", DUMP_PREFIX_ADDRESS, 16, 1,
					(unsigned char *)rsp, 512, false);
	#endif

	return 0;
}

static int rpmb_route_read_req(struct ufs_hba *hba, struct s_rpmb *req,
			       unsigned short req_cnt, struct s_rpmb *rsp,
			       unsigned short rsp_cnt)
{
	struct scsi_cmnd *pccb = (struct scsi_cmnd *)&tempccb;

	/* Authentication data read request */
	scsi_setup_security_protocol_out(pccb, 0, 1);
	pccb->pdata = (unsigned char *)req;
	pccb->datalen = 512;
	#ifdef CONFIG_UFS_DEBUG
	print_hex_dump("RPMB_REQ_READ_DATA", DUMP_PREFIX_ADDRESS, 16, 1,
					(unsigned char *)req, 512, false);
	#endif
	if(ufshcd_queuecommand(hba, pccb))
		return -1;

	/* Authentication data read response */
	scsi_setup_security_protocol_in(pccb, 0, rsp_cnt);
	pccb->pdata = (unsigned char *)rsp;
	pccb->datalen = 512*rsp_cnt;
	if(ufshcd_queuecommand(hba, pccb))
		return -1;
	#ifdef CONFIG_UFS_DEBUG
	print_hex_dump("RPMB_RESP_READ_DATA", DUMP_PREFIX_ADDRESS, 16, 1,
					(unsigned char *)rsp, 512*rsp_cnt, false);
	#endif

	return 0;
}

static int rpmb_route_frames(struct ufs_hba *hba, struct s_rpmb *req,
			     unsigned short req_cnt, struct s_rpmb *rsp,
			     unsigned short rsp_cnt)
{
	unsigned short n;

	/*
	 * If multiple request frames are provided, make sure that all are
	 * of the same type.
	 */
	for (n = 1; n < req_cnt; n++)
		if (req[n].request != req->request)
			return -EINVAL;

	switch (be16_to_cpu(req->request)) {
	case RPMB_REQ_KEY:
		if (req_cnt != 1 || rsp_cnt != 1)
			return -EINVAL;
		return rpmb_route_write_req(hba, req, req_cnt, rsp, rsp_cnt);

	case RPMB_REQ_WRITE_DATA:
		if (!req_cnt || rsp_cnt != 1)
			return -EINVAL;
		return rpmb_route_write_req(hba, req, req_cnt, rsp, rsp_cnt);

	case RPMB_REQ_WCOUNTER:
		if (req_cnt != 1 || rsp_cnt != 1)
			return -EINVAL;
		return rpmb_route_read_req(hba, req, req_cnt, rsp, rsp_cnt);

	case RPMB_REQ_READ_DATA:
		if (req_cnt != 1 || !req_cnt)
			return -EINVAL;
		return rpmb_route_read_req(hba, req, req_cnt, rsp, rsp_cnt);

	default:
		debug("Unsupported message type: %d\n",
		      be16_to_cpu(req->request));
		return -EINVAL;
	}
}

int ufs_rpmb_route_frames(struct ufs_hba *hba, void *req, unsigned long reqlen,
			  void *rsp, unsigned long rsplen)
{
	/*
	 * Whoever crafted the data supplied to this function knows how to
	 * format the PRMB frames and which response is expected. If
	 * there's some unexpected mismatch it's more helpful to report an
	 * error immediately than trying to guess what was the intention
	 * and possibly just delay an eventual error which will be harder
	 * to track down.
	 */

	if (reqlen % sizeof(struct s_rpmb) || rsplen % sizeof(struct s_rpmb))
		return -EINVAL;

	return rpmb_route_frames(hba, req, reqlen / sizeof(struct s_rpmb),
				 rsp, rsplen / sizeof(struct s_rpmb));
}

