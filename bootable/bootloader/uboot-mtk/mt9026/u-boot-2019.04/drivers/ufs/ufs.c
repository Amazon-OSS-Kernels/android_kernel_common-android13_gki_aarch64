
#include <common.h>
#include <dm.h>
#include "ufshcd.h"
#include <ufs_scsi.h>
#include <dm/device-internal.h>
#include <dm/uclass-internal.h>
#include <asm/unaligned.h>

static int ufs_has_init = 0;
static int ufs_target_cnt = 0;	/* number of ufs device */
static int ufs_dev_cnt = 0;		/* number of ufs LUN */

extern int ufs_curr_target; /* current target */
static struct scsi_cmnd tempccb;	/* temporary scsi command buffer */

/* almost the maximum amount of the scsi_ext command.. */
#define SCSI_MAX_RW10_BLK (SG_ALL*64)	//SG_ALL*(256K/4K)

int get_ufs_num(void)
{
	return max((blk_find_max_devnum(IF_TYPE_UFS) + 1), 0);
}

struct ufs_hba *find_ufs_device(int dev_num)
{
	struct udevice *dev;
	struct ufs_hba *hba;
	int ret;

	ret = uclass_get_device_by_seq(UCLASS_UFS, dev_num, &dev);
	if (ret) {
		return NULL;
	}

	hba = dev_get_uclass_platdata(dev);

	return hba;
}

#ifdef CONFIG_SYS_64BIT_LBA
void ufs_scsi_setup_rw16(struct scsi_cmnd *pccb, lbaint_t start, unsigned long blocks, bool write)
{
	pccb->cmnd[0] = write ? WRITE_16 : READ_16;
	pccb->cmnd[1] = 0x08;
	pccb->cmnd[2] = (unsigned char)(start >> 56) & 0xff;
	pccb->cmnd[3] = (unsigned char)(start >> 48) & 0xff;
	pccb->cmnd[4] = (unsigned char)(start >> 40) & 0xff;
	pccb->cmnd[5] = (unsigned char)(start >> 32) & 0xff;
	pccb->cmnd[6] = (unsigned char)(start >> 24) & 0xff;
	pccb->cmnd[7] = (unsigned char)(start >> 16) & 0xff;
	pccb->cmnd[8] = (unsigned char)(start >> 8) & 0xff;
	pccb->cmnd[9] = (unsigned char)start & 0xff;
	pccb->cmnd[10] = (unsigned char)(blocks >> 24) & 0xff;
	pccb->cmnd[11] = (unsigned char)(blocks >> 16) & 0xff;
	pccb->cmnd[12] = (unsigned char)(blocks >> 8) & 0xff;
	pccb->cmnd[13] = (unsigned char)blocks & 0xff;
	pccb->cmnd[14] = 0;
	pccb->cmnd[15] = 0;
	pccb->cmd_len = 16;
	pccb->sc_data_direction = write ? DMA_TO_DEVICE : DMA_FROM_DEVICE;;

	debug("%s: cmd: %02X %02X startblk %02X%02X%02X%02X%02X%02X%02X%02X blccnt %02X%02X%02X%02X\n",
	      __func__,
	      pccb->cmnd[0], pccb->cmnd[1],
	      pccb->cmnd[2], pccb->cmnd[3], pccb->cmnd[4], pccb->cmnd[5],
	      pccb->cmnd[6], pccb->cmnd[7], pccb->cmnd[8], pccb->cmnd[9],
	      pccb->cmnd[10], pccb->cmnd[11], pccb->cmnd[12], pccb->cmnd[13]);
}
#endif

void ufs_scsi_setup_rw10(struct scsi_cmnd *pccb, lbaint_t start, unsigned short blocks, bool write)
{
	pccb->cmnd[0] = write ? WRITE_10 : READ_10;
	pccb->cmnd[1] = 0x08;
	pccb->cmnd[2] = (unsigned char)(start >> 24) & 0xff;
	pccb->cmnd[3] = (unsigned char)(start >> 16) & 0xff;
	pccb->cmnd[4] = (unsigned char)(start >> 8) & 0xff;
	pccb->cmnd[5] = (unsigned char)start & 0xff;
	pccb->cmnd[6] = 0;
	pccb->cmnd[7] = (unsigned char)(blocks >> 8) & 0xff;
	pccb->cmnd[8] = (unsigned char)blocks & 0xff;
	pccb->cmnd[9] = 0;
	pccb->cmd_len = 10;
	pccb->sc_data_direction = write ? DMA_TO_DEVICE : DMA_FROM_DEVICE;

	debug("%s: cmd: %02X %02X startblk %02X%02X%02X%02X blccnt %02X%02X\n",
	      __func__,
	      pccb->cmnd[0], pccb->cmnd[1],
	      pccb->cmnd[2], pccb->cmnd[3], pccb->cmnd[4], pccb->cmnd[5],
	      pccb->cmnd[7], pccb->cmnd[8]);
}

static ulong ufs_read(struct udevice *dev, lbaint_t blknr, lbaint_t blkcnt, void *buffer)
{
	struct blk_desc *block_dev = dev_get_uclass_platdata(dev);
	struct udevice *bdev = dev->parent;
	lbaint_t start, blks;
	uintptr_t buf_addr;
	unsigned long smallblks = 0;
	struct scsi_cmnd *pccb = (struct scsi_cmnd *)&tempccb;
	struct ufs_hba *hba;

	hba = (struct ufs_hba *)dev_get_uclass_platdata(bdev);

	/* Setup device */
	pccb->tag = 0;
	pccb->lun = block_dev->lun;
	buf_addr = (unsigned long)buffer;
	start = blknr;
	blks = blkcnt;
	debug("\n%s: dev %d startblk " LBAF ", blccnt " LBAF " buffer %lX\n",
	      __func__, block_dev->devnum, start, blks, (unsigned long)buffer);
	do {
		pccb->pdata = (unsigned char *)buf_addr;

		smallblks = min_t(lbaint_t, blks, SCSI_MAX_RW10_BLK);
		pccb->datalen = block_dev->blksz * smallblks;
		ufs_scsi_setup_rw10(pccb, start, smallblks, false);
		debug("%s: startblk " LBAF ", blccnt " LBAF " buffer %lX\n",
		      __func__, start, smallblks, buf_addr);
		if(ufshcd_queuecommand(hba, pccb)) {
			blkcnt -= blks;
			break;
		}

		start += smallblks;
		blks -= smallblks;
		buf_addr += pccb->datalen;
	} while (blks != 0);
	debug("%s: end startblk " LBAF ", blccnt " LBAF " buffer %lX\n",
	      __func__, start, smallblks, buf_addr);
	return blkcnt;
}

/*******************************************************************************
 * scsi_write
 */
static ulong ufs_write(struct udevice *dev, lbaint_t blknr, lbaint_t blkcnt, const void *buffer)
{
	struct blk_desc *block_dev = dev_get_uclass_platdata(dev);
	struct udevice *bdev = dev->parent;
	lbaint_t start, blks;
	uintptr_t buf_addr;
	unsigned long smallblks = 0;
	struct scsi_cmnd *pccb = (struct scsi_cmnd *)&tempccb;
	struct ufs_hba *hba;

	hba = (struct ufs_hba *)dev_get_uclass_platdata(bdev);

	/* Setup device */
	pccb->tag = 0;
	pccb->lun = block_dev->lun;
	buf_addr = (unsigned long)buffer;
	start = blknr;
	blks = blkcnt;
	debug("\n%s: dev %d startblk " LBAF ", blccnt " LBAF " buffer %lX\n",
	      __func__, block_dev->devnum, start, blks, (unsigned long)buffer);
	do {
		pccb->pdata = (unsigned char *)buf_addr;

		smallblks = min_t(lbaint_t, blks, SCSI_MAX_RW10_BLK);
		pccb->datalen = block_dev->blksz * smallblks;
		ufs_scsi_setup_rw10(pccb, start, smallblks, true);
		debug("%s: startblk " LBAF ", blccnt " LBAF " buffer %lX\n",
		      __func__, start, smallblks, buf_addr);
		if(ufshcd_queuecommand(hba, pccb)) {
			blkcnt -= blks;
			break;
		}

		start += smallblks;
		blks -= smallblks;
		buf_addr += pccb->datalen;
	} while (blks != 0);
	debug("%s: end startblk " LBAF ", blccnt " LBAF " buffer %lX\n",
	      __func__, start, smallblks, buf_addr);
	return blkcnt;
}

void ufs_scsi_setup_unmap(struct scsi_cmnd *pccb, lbaint_t start, unsigned short blocks)
{
	pccb->cmnd[0] = UNMAP;
	pccb->cmnd[1] = 0;
	pccb->cmnd[2] = 0;
	pccb->cmnd[3] = 0;
	pccb->cmnd[4] = 0;
	pccb->cmnd[5] = 0;
	pccb->cmnd[6] = 0;
	pccb->cmnd[7] = 0;
	pccb->cmnd[8] = 24;
	pccb->cmnd[9] = 0;
	pccb->cmd_len = 10;
	pccb->sc_data_direction = DMA_TO_DEVICE;

	debug("%s: cmd: %02X %02X startblk %02X%02X%02X%02X blccnt %02X%02X\n",
	      __func__,
	      pccb->cmnd[0], pccb->cmnd[1],
	      pccb->cmnd[2], pccb->cmnd[3], pccb->cmnd[4], pccb->cmnd[5],
	      pccb->cmnd[7], pccb->cmnd[8]);
}

static ulong ufs_erase(struct udevice *dev, lbaint_t blknr, lbaint_t blkcnt)
{
	struct blk_desc *block_dev = dev_get_uclass_platdata(dev);
	struct udevice *bdev = dev->parent;
	struct scsi_cmnd *pccb = (struct scsi_cmnd *)&tempccb;
	struct ufs_hba *hba;
	u8 ParamList[128];

	hba = (struct ufs_hba *)dev_get_uclass_platdata(bdev);

	/* Setup device */
	pccb->tag = 0;
	pccb->lun = block_dev->lun;

	debug("\n%s: dev %d startblk " LBAF ", blccnt " LBAF "\n",
	      __func__, block_dev->devnum, blknr, blkcnt);

	ParamList[0] = ((23-1)>>8)&0xFF;
	ParamList[1] = (23-1)&0xFF;
	ParamList[2] = ((23-7)>>8)&0xFF;
	ParamList[3] = (23-7)&0xFF;
	ParamList[4] = 0;
	ParamList[5] = 0;
	ParamList[6] = 0;
	ParamList[7] = 0;
	ParamList[8] = (blknr>>56)&0xFF;
	ParamList[9] = (blknr>>48)&0xFF;
	ParamList[10] = (blknr>>40)&0xFF;
	ParamList[11] = (blknr>>32)&0xFF;
	ParamList[12] = (blknr>>24)&0xFF;
	ParamList[13] = (blknr>>16)&0xFF;
	ParamList[14] = (blknr>>8)&0xFF;
	ParamList[15] = (blknr)&0xFF;
	ParamList[16] = (blkcnt>>24)&0xFF;
	ParamList[17] = (blkcnt>>16)&0xFF;
	ParamList[18] = (blkcnt>>8)&0xFF;
	ParamList[19] = (blkcnt)&0xFF;
	ParamList[20] = 0;
	ParamList[21] = 0;
	ParamList[22] = 0;
	ParamList[23] = 0;
	pccb->pdata = (unsigned char *)ParamList;
	pccb->datalen = 24;

	ufs_scsi_setup_unmap(pccb, blknr, blkcnt);

	if(ufshcd_queuecommand(hba, pccb)) {
		blkcnt = 0;
	}

	return blkcnt;
}

static int ufs_test_unit_ready(struct udevice *dev, int lun)
{
	struct ufs_hba *hba;
	int ret = 0;
	struct scsi_cmnd *pccb = (struct scsi_cmnd *)&tempccb;

	/* Get controller platdata */
	hba = dev_get_uclass_platdata(dev);

	pccb->tag = 0;
	pccb->lun = lun;
	pccb->cmnd[0] = TEST_UNIT_READY;
	pccb->cmnd[1] = 0;
	pccb->cmnd[2] = 0;
	pccb->cmnd[3] = 0;
	pccb->cmnd[4] = 0;
	pccb->cmnd[5] = 0;
	pccb->cmd_len = 6;
	pccb->sc_data_direction = DMA_NONE;;
	pccb->pdata = NULL;
	pccb->datalen = 0;

	ret = ufshcd_queuecommand(hba, pccb);

	return ret;
}

static int do_ufs_scan_one(struct udevice *dev, int lun, bool verbose)
{
	int ret = 0;
	struct ufs_hba *hba;
	struct udevice *bdev;
	struct blk_desc *bdesc;
	char str[10];
	u8 *desc_buf;
	int blksz;
	lbaint_t lba;
	int retry = 10;

	do {
		ret = ufs_test_unit_ready(dev, lun);
	} while(ret && --retry);

	if(ret) {
		dev_err(hba->dev, "Failed requesting test_unit_ready lun %d err = %d\n", lun, ret);
		return ret;
	}

	desc_buf = kmalloc(QUERY_DESC_MAX_SIZE, GFP_KERNEL);
	if (!desc_buf) {
		return -ENOMEM;
	}

	hba = dev_get_uclass_platdata(dev);

	ret = ufshcd_read_unit_desc_param(hba, lun, 0, desc_buf, QUERY_DESC_MAX_SIZE);
	if (ret)
	{
		dev_err(hba->dev, "Failed reading unit Desc.%d err = %d\n", lun, ret);
		goto out;
	}

	blksz = 1<<desc_buf[UNIT_DESC_PARAM_LOGICAL_BLK_SIZE];
	#ifdef CONFIG_SYS_64BIT_LBA
	lba = (((lbaint_t) desc_buf[UNIT_DESC_PARAM_LOGICAL_BLK_COUNT]) << 56) |
		(((lbaint_t) desc_buf[UNIT_DESC_PARAM_LOGICAL_BLK_COUNT+1]) << 48) |
		(((lbaint_t) desc_buf[UNIT_DESC_PARAM_LOGICAL_BLK_COUNT+2]) << 40) |
		(((lbaint_t) desc_buf[UNIT_DESC_PARAM_LOGICAL_BLK_COUNT+3]) << 32) |
		(((lbaint_t) desc_buf[UNIT_DESC_PARAM_LOGICAL_BLK_COUNT+4]) << 24) |
		(((lbaint_t) desc_buf[UNIT_DESC_PARAM_LOGICAL_BLK_COUNT+5]) << 16) |
		(((lbaint_t) desc_buf[UNIT_DESC_PARAM_LOGICAL_BLK_COUNT+6]) << 8) |
		(((lbaint_t) desc_buf[UNIT_DESC_PARAM_LOGICAL_BLK_COUNT+7]) << 0);
	#else
	lba = (((lbaint_t) desc_buf[UNIT_DESC_PARAM_LOGICAL_BLK_COUNT+4]) << 24) |
		(((lbaint_t) desc_buf[UNIT_DESC_PARAM_LOGICAL_BLK_COUNT+5]) << 16) |
		(((lbaint_t) desc_buf[UNIT_DESC_PARAM_LOGICAL_BLK_COUNT+6]) << 8) |
		(((lbaint_t) desc_buf[UNIT_DESC_PARAM_LOGICAL_BLK_COUNT+7]) << 0);
	#endif
	/*
	* Create only one block device and do detection
	* to make sure that there won't be a lot of
	* block devices created
	*/
	snprintf(str, sizeof(str), "lun%d", lun);
	ret = blk_create_devicef(dev, "ufs_blk", str, IF_TYPE_UFS, -1,
			blksz, lba, &bdev);
	if (ret) {
		dev_err(hba->dev, "Can't create ufs device\n");
		goto out;
	}
	ufs_dev_cnt++;

	bdesc = dev_get_uclass_platdata(bdev);
	bdesc->target = 0;
	bdesc->lun = lun;
	bdesc->removable = false;
	bdesc->type = DEV_TYPE_HARDDISK;
	bdesc->log2blksz = LOG2(bdesc->blksz);

	memcpy(&bdesc->vendor, (const char *)hba->sManufacturerName,
		min_t(u8, strlen((const char *)hba->sManufacturerName), sizeof(bdesc->vendor)));

	memcpy(&bdesc->product, (const char *)hba->sProductName,
		min_t(u8, strlen((const char *)hba->sProductName), sizeof(bdesc->product)));

	memcpy(&bdesc->revision, (const char *)hba->sProductRevisionLevel,
		min_t(u8, strlen((const char *)hba->sProductRevisionLevel), sizeof(bdesc->revision)));

	if (verbose) {
		printf("  Device %d: ", bdesc->devnum);
		dev_print(bdesc);
	}

out:
	kfree(desc_buf);
	return ret;
}

static int ufs_scan_dev(struct udevice *dev, bool verbose)
{
	struct ufs_hba *hba;
	int ret = 0;
	int lun;
	struct scsi_cmnd *pccb = (struct scsi_cmnd *)&tempccb;
	unsigned int length;
	unsigned int num_luns;
	unsigned int retries;
	struct scsi_lun *lunp, *lun_data;
	u8 desc_buf[QUERY_DESC_DEVICE_DEF_SIZE];

	/* probe UFS controller driver */
	ret = device_probe(dev);
	if (ret)
		return ret;

	/* Get controller platdata */
	hba = dev_get_uclass_platdata(dev);

	/*
	 * Allocate enough to hold the header (the same size as one scsi_lun)
	 * plus the number of luns we are requesting.  511 was the default
	 * value of the now removed max_report_luns parameter.
	 */
	length = (511 + 1) * sizeof(struct scsi_lun);
	lun_data = kmalloc(length, GFP_KERNEL);
	if (!lun_data) {
		ret = -ENOMEM;
		goto out;
	}
	pccb->tag = 0;
	pccb->lun = 0;
	pccb->cmnd[0] = REPORT_LUNS;
	pccb->cmnd[1] = 0;
	pccb->cmnd[2] = 0;
	pccb->cmnd[3] = 0;
	pccb->cmnd[4] = 0;
	pccb->cmnd[5] = 0;
	pccb->cmnd[6] = (unsigned char)(length >> 24) & 0xff;
	pccb->cmnd[7] = (unsigned char)(length >> 16) & 0xff;
	pccb->cmnd[8] = (unsigned char)(length >> 8) & 0xff;
	pccb->cmnd[9] = (unsigned char)length & 0xff;
	pccb->cmnd[10] = 0;
	pccb->cmnd[11] = 0;
	pccb->cmd_len = 12;
	pccb->sc_data_direction = DMA_FROM_DEVICE;;
	pccb->pdata = (unsigned char *)lun_data;
	pccb->datalen = length;
	/*
	 * We can get a UNIT ATTENTION, for example a power on/reset, so
	 * retry a few times (like sd.c does for TEST UNIT READY).
	 * Experience shows some combinations of adapter/devices get at
	 * least two power on/resets.
	 *
	 * Illegal requests (for devices that do not support REPORT LUNS)
	 * should come through as a check condition, and will not generate
	 * a retry.
	 */
	for (retries = 0; retries < 3; retries++) {
		ret = ufshcd_queuecommand(hba, pccb);
		if (ret == 0)
			break;
		else if ((pccb->sense_buffer[0] & 0x70) == 0x70) {	/* valid sense data */
			if (pccb->sense_buffer[2] && 0xF != UNIT_ATTENTION) /* check sense key */
				break;
		}
	}
	if (ret) {
		/*
		 * The device probably does not support a REPORT LUN command
		 */
		ret = 0;
		goto out;
	}

	ret = ufs_read_desc(hba, QUERY_DESC_IDN_DEVICE, 0, desc_buf, QUERY_DESC_DEVICE_DEF_SIZE);
	if (ret < 0) {
		dev_err(hba->dev, "%s: Failed reading Device Descr. err = %d\n",
			__func__, ret);
		goto out;
	}
	hba->bNumberLU = desc_buf[DEVICE_DESC_PARAM_NUM_LU];
	hba->wManufacturerID = (desc_buf[DEVICE_DESC_PARAM_MANF_ID]<<8)
						  |(desc_buf[DEVICE_DESC_PARAM_MANF_ID+1]);
	hba->wManufactureDate = (desc_buf[DEVICE_DESC_PARAM_MANF_DATE]<<8)
						  |(desc_buf[DEVICE_DESC_PARAM_MANF_DATE+1]);
	hba->iManufacturerName = desc_buf[DEVICE_DESC_PARAM_MANF_NAME];
	hba->iProductName = desc_buf[DEVICE_DESC_PARAM_PRDCT_NAME];
	hba->iSerialNumber = desc_buf[DEVICE_DESC_PARAM_SN];
	hba->iProductRevisionLevel = desc_buf[DEVICE_DESC_PARAM_PRDCT_REV];

	ret = ufshcd_read_string_desc(hba, hba->iManufacturerName, &hba->sManufacturerName, true/*ASCII*/);
	if (ret < 0) {
		dev_err(hba->dev, "%s: Failed reading Manufacturer Name. err = %d\n",
			__func__, ret);
		hba->sManufacturerName = (u8 *)"NA";
	}

	ret = ufshcd_read_string_desc(hba, hba->iProductName, &hba->sProductName, true/*ASCII*/);
	if (ret < 0) {
		dev_err(hba->dev, "%s: Failed reading Product Name. err = %d\n",
			__func__, ret);
		hba->sProductName = (u8 *)"NA";
	}

	ret = ufshcd_read_string_desc(hba, hba->iSerialNumber, &hba->sSerialNumber, true/*ASCII*/);
	if (ret < 0) {
		dev_err(hba->dev, "%s: Failed reading Product Name. err = %d\n",
			__func__, ret);
		hba->sSerialNumber = (u8 *)"NA";
	}

	ret = ufshcd_read_string_desc(hba, hba->iProductRevisionLevel, &hba->sProductRevisionLevel, true/*ASCII*/);
	if (ret < 0) {
		dev_err(hba->dev, "%s: Failed reading ProductRevisionLevel. err = %d\n",
			__func__, ret);
		hba->sProductRevisionLevel = (u8 *)"NA";
	}

	ret = 0;
	length = get_unaligned_be32(lun_data->scsi_lun);
	num_luns = (length / sizeof(struct scsi_lun));

	for (lunp = &lun_data[1]; lunp <= &lun_data[num_luns]; lunp++) {
		lun = lunp->scsi_lun[1];
		do_ufs_scan_one(dev, lun, verbose);
	}

	retries = 10;
	do {
		ret = ufs_test_unit_ready(dev, SCSI_W_LUN_RPMB);
	} while(ret && --retries);

	if (verbose) {
		ret = ufs_read_desc(hba, QUERY_DESC_IDN_HEALTH, 0, desc_buf, QUERY_DESC_HEALTH_DEF_SIZE);
		if (ret < 0) {
			//dev_err(hba->dev, "%s: Failed reading Health Descr. err = %d\n", __func__, ret);
		} else {
			u8 index;
			#define MAX_PRE_EOL_INFO 3
			const char * StrPreEOLInfo[MAX_PRE_EOL_INFO+1] = {
				"Not defined",
				"Normal",
				"Warning : Consumed 80% of reserved blocks",
				"Critical : Consumed 90% of reserved blocks",
			};
			#define MAX_DEVICE_LIFE_TIME_EST 0x0B
			const char * StrLifeTimeEst[MAX_DEVICE_LIFE_TIME_EST+1] = {
				"Information not available",
				"0% - 10% device life time used",
				"10% - 20% device life time used",
				"20% - 30% device life time used",
				"30% - 40% device life time used",
				"40% - 50% device life time used",
				"50% - 60% device life time used",
				"60% - 70% device life time used",
				"70% - 80% device life time used",
				"80% - 90% device life time used",
				"90% - 100% device life time used",
				"Exceeded its maximum estimated device life time",
			};

			index = desc_buf[QUERY_DESC_DESC_PRE_EOL_INFO];
			if (index > MAX_PRE_EOL_INFO)
				index = 0;
			printf("  Pre End of Life Information : %s\n", StrPreEOLInfo[index]);

			index = desc_buf[QUERY_DESC_DESC_DEVICE_LIFE_TIME_ESTA];
			if (index > MAX_DEVICE_LIFE_TIME_EST)
				index = 0;
			printf("  Device life time A : %s\n", StrLifeTimeEst[index]);

			index = desc_buf[QUERY_DESC_DESC_DEVICE_LIFE_TIME_ESTB];
			if (index > MAX_DEVICE_LIFE_TIME_EST)
				index = 0;
			printf("  Device life time B : %s\n", StrLifeTimeEst[index]);
		}
	}
out:
	kfree(lun_data);
	return ret;
}

int ufs_scan(bool verbose)
{
	struct uclass *uc;
	struct udevice *dev;
	int ret;

	if (verbose)
		printf("\nscanning bus for ufs devices...\n");

	ufs_target_cnt = 0;
	ufs_dev_cnt = 0;

	blk_unbind_all(IF_TYPE_UFS);

	ret = uclass_get(UCLASS_UFS, &uc);
	if (ret)
		return ret;

	uclass_foreach_dev(dev, uc) {
		if (ufs_has_init)
			device_remove(dev, DM_REMOVE_NORMAL);

		ufs_target_cnt++;

		ret = ufs_scan_dev(dev, verbose);
		if (ret) {
			ufs_has_init = 0;
			return ret;
		}
	}

	ufs_has_init = 1;

	return 0;
}

#ifdef CONFIG_BLK
static const struct blk_ops ufs_blk_ops = {
	.read	= ufs_read,
	.write	= ufs_write,
	.erase	= ufs_erase,
	.select_hwpart = NULL,
};

U_BOOT_DRIVER(ufs_blk) = {
	.name	= "ufs_blk",
	.id		= UCLASS_BLK,
	.ops	= &ufs_blk_ops,
};
#endif

int ufs_read_attr(struct ufs_hba *hba, int idn, u32 *attr_val)
{
	return(ufshcd_query_attr(hba, UPIU_QUERY_OPCODE_READ_ATTR,
			idn, 0, 0, attr_val));
}

int ufs_write_attr(struct ufs_hba *hba, int idn, u32 attr_val)
{
	return(ufshcd_query_attr(hba, UPIU_QUERY_OPCODE_WRITE_ATTR,
			idn, 0, 0, &attr_val));
}

int ufs_read_flag(struct ufs_hba *hba, int idn, u32 *flag_res)
{
	return(ufshcd_query_flag(hba, UPIU_QUERY_OPCODE_READ_FLAG,
			(enum flag_idn)idn, (bool *)flag_res));
}

int ufs_write_flag(struct ufs_hba *hba, int idn, u32 flag_res)
{
	return(ufshcd_query_flag(hba, UPIU_QUERY_OPCODE_SET_FLAG,
			(enum flag_idn)idn, (bool *)&flag_res));
}

int ufs_read_desc(struct ufs_hba *hba, int desc_id, int desc_index,
					u8 *param_read_buf, u8 param_size)
{
	return(ufshcd_read_desc_param(hba, (enum desc_idn)desc_id, desc_index,
			   				0, param_read_buf, param_size));
}

int ufs_write_desc(struct ufs_hba *hba, int desc_id, int desc_index,
					u8 *param_write_buf, u8 param_size)
{
	return(ufshcd_write_desc_param(hba, (enum desc_idn)desc_id, desc_index,
			   				0, param_write_buf, param_size));

}

int ufs_get_cid(struct ufs_hba *hba, u8 *cid)
{
	/**
	 * CID Field:
	 * ManufacturerID	: 2 bytes
	 * ManufactureDate	: 2 bytes
	 * SerialNumber		: 12 bytes max
	 * ProductName		: if CID has free space
	 */

	int i, j;

	if (cid == NULL) {
		printf("[%s] NULL cid\n", __func__);
		return -EINVAL;
	}

	cid[0] = (hba->wManufacturerID>>8)&0xFF;
	cid[1] = (hba->wManufacturerID&0xFF);
	cid[2] = (hba->wManufactureDate>>8)&0xFF;
	cid[3] = (hba->wManufactureDate&0xFF);

	for (j=0, i=4 ; i<MAX_CID_SIZE ; i++, j++) {
		if (hba->sSerialNumber[j] == 0)
			break;

		cid[i] = hba->sSerialNumber[j];
	}

	for(j=0 ; i<MAX_CID_SIZE ; i++, j++) {
		if (hba->sProductName[j] == 0)
			break;

		cid[i] = hba->sProductName[j];
	}

	for( ; i<MAX_CID_SIZE ; i++) {
		cid[i] = '0';
	}

	return 0;
}

void ufs_crypto_enable(struct ufs_hba *hba)
{
	ufshcd_crypto_enable(hba);
}

void ufs_crypto_disable(struct ufs_hba *hba)
{
	ufshcd_crypto_disable(hba);
}
