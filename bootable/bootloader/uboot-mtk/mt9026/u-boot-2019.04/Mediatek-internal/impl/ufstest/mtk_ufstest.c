// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#include <common.h>
#include <debug_impl.h>
#include <hexdump.h>
#include <mtk_ufstest.h>

extern int ufs_curr_target; /* current target */

#define TEST_LUN						0
#define UFS_IPVERIFY_PATITION_SIZE  	0x100000
#define UFS_IPVERIFY_PATITION_START		0x100000

#define UFS_SPEED_TEST_SINGLE_BLKCNT_W		64
#define UFS_SPEED_TEST_MULTIPLE_BLKCNT_W	((u32)30*(1024*1024/4096)) // total 30MB
#define UFS_SPEED_TEST_R_MULTI				10
#define UFS_SPEED_TEST_SINGLE_BLKCNT_R		((u32)UFS_SPEED_TEST_SINGLE_BLKCNT_W*UFS_SPEED_TEST_R_MULTI)
#define UFS_SPEED_TEST_MULTIPLE_BLKCNT_R	((u32)UFS_SPEED_TEST_MULTIPLE_BLKCNT_W*UFS_SPEED_TEST_R_MULTI)
#define UFS_SECTOR_4KBYTE					4096
#define UFS_SECTOR_4KBYTE_BITS				12

#define UFS_PWRCUT_DATA0				0x55
#define UFS_PWRCUT_DATA1				0xAA
#define UFS_PWRCUT_DATA_CNT				2
#define UFS_PWRCUT_TEST_UNIT_CNT		64 //16
#define UFS_PWRCUT_TEST_UNIT_BYTECNT	0x1000000 // 16MB
#define UFS_PWRCUT_TEST_SPACE_BYTECNT	(UFS_PWRCUT_TEST_UNIT_CNT*UFS_PWRCUT_TEST_UNIT_BYTECNT) // 16x16 = 256MB

typedef struct _UFS_PWRCUT_CTRLBLK {

	u32 u32_CheckSum;
	u8  u8_TargetUnit;
	u8  au8_TaegetData[UFS_PWRCUT_TEST_UNIT_CNT];

} UFS_PWRCUT_CTRLBLK_t;

void ufstest_downcount(u32 u32_Sec) // verify the HW Timer
{
	u32 u32_i; 
	ulong u32_t0, u32_t1;

	for (u32_i=0; u32_i<u32_Sec; u32_i++) {
		printf("%u \n", u32_Sec-u32_i);
        u32_t0 = timer_get_boot_us();
		do {
			u32_t1 = timer_get_boot_us();
		}while( (u32_t1-u32_t0) < 1000000);
	}
	printf(".\n");
}

u32 ufstest_speed(u8* u8_DataBuf, u32 u32_BlkAddr)
{
	int err = 0;
#if 0
	u32 u32_total_blk;
#endif
	u32 dLba;
	u32 u32_i, u32_SectorAddr=0, u32_SectorCnt=0, u32_t0, u32_t1, u32_t;
	ulong n;

	ufstest_downcount(3);

#if 0
	u32_total_blk = UFS_IPVERIFY_PATITION_SIZE/UFS_SECTOR_4KBYTE;
#endif
	dLba = u32_BlkAddr;

	for (u32_i=0; u32_i<UFS_IPVERIFY_PATITION_SIZE; u32_i++)
		u8_DataBuf[u32_i] = (u8)u32_i;

	printf("Single Write, total: %u KB, %u blocks, burst size: %Xh\n",
			(UFS_SPEED_TEST_SINGLE_BLKCNT_W<<UFS_SECTOR_4KBYTE_BITS)>>10,
			UFS_SPEED_TEST_SINGLE_BLKCNT_W, 1<<UFS_SECTOR_4KBYTE_BITS);
	u32_t0 = timer_get_boot_us();
	for (u32_i=0; u32_i<UFS_SPEED_TEST_SINGLE_BLKCNT_W; u32_i++) {
		#if 0
		u32_SectorAddr = timer_get_boot_us()%u32_total_blk;
		u32_SectorAddr += dLba;
		#else
		u32_SectorAddr = (dLba+u32_i);
		#endif

		n = blk_write_devnum(IF_TYPE_UFS, TEST_LUN, u32_SectorAddr, 1, (ulong *)u8_DataBuf);
		if(n != 1) {
			UBOOT_ERROR("UFS Err: write Fail, Err:%lx\n", n);
			return CMD_RET_FAILURE;
		}
	}
	u32_t1 = timer_get_boot_us();
	u32_t = u32_t1 > u32_t0 ? u32_t1-u32_t0 : 0xFFFFFFFF-u32_t0+u32_t1;
	printf(" %u MB/s \n", (UFS_SPEED_TEST_SINGLE_BLKCNT_W<<UFS_SECTOR_4KBYTE_BITS)/u32_t);

	// -----------------------------------------
	printf("Single Read, total: %u KB, %u blocks, burst size: %Xh\n",
			((UFS_SPEED_TEST_SINGLE_BLKCNT_R)<<UFS_SECTOR_4KBYTE_BITS)>>10,
			UFS_SPEED_TEST_SINGLE_BLKCNT_R, 1<<UFS_SECTOR_4KBYTE_BITS);
	u32_t0 = timer_get_boot_us();
	for (u32_i=0; u32_i<UFS_SPEED_TEST_SINGLE_BLKCNT_R; u32_i++) {
		#if 0
		u32_SectorAddr = timer_get_boot_us()%u32_total_blk;
		u32_SectorAddr += dLba;
		#else
		u32_SectorAddr = (dLba+u32_i);
		#endif

		n = blk_read_devnum(IF_TYPE_UFS, TEST_LUN, u32_SectorAddr, 1, (ulong *)u8_DataBuf);
		if(n != 1) {
			UBOOT_ERROR("UFS Err: read Fail, Err:%lx\n", n);
			return CMD_RET_FAILURE;
		}
	}
	u32_t1 = timer_get_boot_us();
	u32_t = u32_t1 > u32_t0 ? u32_t1-u32_t0 : 0xFFFFFFFF-u32_t0+u32_t1;
	printf(" %u MB/s \n", (UFS_SPEED_TEST_SINGLE_BLKCNT_R<<UFS_SECTOR_4KBYTE_BITS)/u32_t);

	// ==================================
	u32_SectorCnt = 0;
	LABEL_MULTI:
	printf("------------------------------\n");
	switch (u32_SectorCnt) {
		case 0:
			u32_SectorCnt = 0x400*4 >> UFS_SECTOR_4KBYTE_BITS; // 4KB
		    break;
		case 0x400*4 >> UFS_SECTOR_4KBYTE_BITS:
			u32_SectorCnt = 0x400*16 >> UFS_SECTOR_4KBYTE_BITS; // 16KB
		    break;
		case 0x400*16 >> UFS_SECTOR_4KBYTE_BITS:
			u32_SectorCnt = 0x400*64 >> UFS_SECTOR_4KBYTE_BITS; // 64KB
		    break;
		case 0x400*64 >> UFS_SECTOR_4KBYTE_BITS:
            u32_SectorCnt = 0x400*128 >> UFS_SECTOR_4KBYTE_BITS; // 128KB
            break;
        case 0x400*128 >> UFS_SECTOR_4KBYTE_BITS:
            u32_SectorCnt = 0x400*256 >> UFS_SECTOR_4KBYTE_BITS; // 256KB
            break;
        case 0x400*256 >> UFS_SECTOR_4KBYTE_BITS:
			u32_SectorCnt = 0x400*1024 >> UFS_SECTOR_4KBYTE_BITS; // 1024KB
		    break;
	}
	printf("Multi Write, total: %u MB, burst size: %u KB\n",
		(UFS_SPEED_TEST_MULTIPLE_BLKCNT_W<<UFS_SECTOR_4KBYTE_BITS)>>20, u32_SectorCnt<<(UFS_SECTOR_4KBYTE_BITS-10));

	u32_SectorAddr = dLba;
	u32_t0 = timer_get_boot_us();
	for (u32_i=0; u32_i<UFS_SPEED_TEST_MULTIPLE_BLKCNT_W; u32_i += u32_SectorCnt) {
		#if 0
		n = blk_write_devnum(IF_TYPE_UFS, TEST_LUN, u32_SectorAddr, u32_SectorCnt, (ulong *)u8_DataBuf);
		#else
		n = blk_write_devnum(IF_TYPE_UFS, TEST_LUN, u32_SectorAddr+u32_i, u32_SectorCnt, (ulong *)u8_DataBuf);
		#endif
		if(n != u32_SectorCnt) {
			UBOOT_ERROR("UFS Err: write Fail, Err:%lx\n", n);
			return CMD_RET_FAILURE;
		}
	}
	u32_t1 = timer_get_boot_us();
	u32_t = u32_t1 > u32_t0 ? u32_t1-u32_t0 : 0xFFFFFFFF-u32_t0+u32_t1;
	printf(" %u MB/s \n", (UFS_SPEED_TEST_MULTIPLE_BLKCNT_W<<UFS_SECTOR_4KBYTE_BITS)/u32_t);

	// -----------------------------------------
	printf("Multi Read, total: %uMB, burst size: %u KB \n",
		(UFS_SPEED_TEST_MULTIPLE_BLKCNT_R<<UFS_SECTOR_4KBYTE_BITS)>>20, u32_SectorCnt<<(UFS_SECTOR_4KBYTE_BITS-10));

	u32_SectorAddr = dLba;
	u32_t0 = timer_get_boot_us();
	for (u32_i=0; u32_i<UFS_SPEED_TEST_MULTIPLE_BLKCNT_R; u32_i += u32_SectorCnt) {
		#if 0
		n = blk_read_devnum(IF_TYPE_UFS, TEST_LUN, u32_SectorAddr, u32_SectorCnt, (ulong *)u8_DataBuf);
		#else
		n = blk_read_devnum(IF_TYPE_UFS, TEST_LUN, u32_SectorAddr+u32_i, u32_SectorCnt, (ulong *)u8_DataBuf);
		#endif
		if(n != u32_SectorCnt) {
			UBOOT_ERROR("UFS Err: read Fail, Err:%lx\n", n);
			return CMD_RET_FAILURE;
		}
	}
	u32_t1 = timer_get_boot_us();
	u32_t = u32_t1 > u32_t0 ? u32_t1-u32_t0 : 0xFFFFFFFF-u32_t0+u32_t1;
	printf(" %u MB/s \n", (UFS_SPEED_TEST_MULTIPLE_BLKCNT_R<<UFS_SECTOR_4KBYTE_BITS)/u32_t);

	if (u32_SectorCnt != 0x400*1024 >> UFS_SECTOR_4KBYTE_BITS)
		goto LABEL_MULTI;
	


	return err;
}

u32 ufstest_ChkSum(u8 *pu8_Data, u32 u32_ByteCnt)
{
	volatile u32 u32_Sum=0, u32_Tmp;
	for (u32_Tmp=0; u32_Tmp < u32_ByteCnt; u32_Tmp++)
		u32_Sum += pu8_Data[u32_Tmp];
	return (u32_Sum);
}

void ufstest_DumpsPwrCutCtrlBlk(UFS_PWRCUT_CTRLBLK_t *pCtrlBlk_t)
{
	u32 u32_i;

	printf("UFS: CheckSum: %Xh\n", pCtrlBlk_t->u32_CheckSum);
	printf("UFS: TargetUnit: %Xh\n", pCtrlBlk_t->u8_TargetUnit);

	printf("  Target Data: ");
	for (u32_i=0; u32_i<UFS_PWRCUT_TEST_UNIT_CNT; u32_i++) {
		if (0 == (u32_i & 0xF))
			printf("\n");

		printf(" %Xh", pCtrlBlk_t->au8_TaegetData[u32_i]);
	}
	printf("\n");
}

u32 ufstest_CheckPwrCutCtrlBlk(u8* u8_DataBuf, u32 u32_BlkAddr)
{
	UFS_PWRCUT_CTRLBLK_t *pCtrlBlk_t;
	ulong n;

	n = blk_read_devnum(IF_TYPE_UFS, TEST_LUN, u32_BlkAddr, 1, (ulong *)u8_DataBuf);
	if(n != 1) {
		UBOOT_ERROR("UFS Err: read CtrlBlk, Err:%lx\n", n);
		return CMD_RET_FAILURE;
	}

	pCtrlBlk_t = (UFS_PWRCUT_CTRLBLK_t*)u8_DataBuf;

	if (pCtrlBlk_t->u32_CheckSum !=
		ufstest_ChkSum(u8_DataBuf+4, sizeof(UFS_PWRCUT_CTRLBLK_t)-4)) {
		UBOOT_ERROR("UFS Err: read CtrlBlk, CheckSum fail\n");
	    return CMD_RET_FAILURE;
	}

	return CMD_RET_SUCCESS;
}


u32 ufstest_pwrcut_initdata(u8* u8_DataBuf, u32 u32_BlkStartAddr)
{
	u32 u32_i, u32_j;
	UFS_PWRCUT_CTRLBLK_t CtrlBlk_t;
	ulong n;

	// ---------------------------------------
	CtrlBlk_t.u8_TargetUnit = 0xFF;

	for (u32_i=0; u32_i<UFS_PWRCUT_TEST_UNIT_CNT; u32_i++)
		CtrlBlk_t.au8_TaegetData[u32_i] = UFS_PWRCUT_DATA0;

	CtrlBlk_t.u32_CheckSum = ufstest_ChkSum((u8*)&CtrlBlk_t.u8_TargetUnit,
		sizeof(UFS_PWRCUT_CTRLBLK_t)-4);

	n = blk_write_devnum(IF_TYPE_UFS, TEST_LUN, u32_BlkStartAddr, 1, (ulong *)&CtrlBlk_t);
	if(n != 1) {
		UBOOT_ERROR("UFS Err: write CtrlBlk, Err:%lx\n", n);
		return CMD_RET_FAILURE;
	}

	// ---------------------------------------
	u32_BlkStartAddr+=1; // skip Ctrl Blk

    memset(u8_DataBuf, UFS_PWRCUT_DATA0, UFS_PWRCUT_TEST_UNIT_BYTECNT);
	printf("Reliable write ...\n");

	for (u32_i=0; u32_i<UFS_PWRCUT_TEST_UNIT_CNT; u32_i++) {
		n = blk_write_devnum(IF_TYPE_UFS, TEST_LUN, u32_BlkStartAddr + u32_i*(UFS_PWRCUT_TEST_UNIT_BYTECNT>>12),
			UFS_PWRCUT_TEST_UNIT_BYTECNT>>12, (ulong *)u8_DataBuf);
		if(n != UFS_PWRCUT_TEST_UNIT_BYTECNT>>12) {
			UBOOT_ERROR("UFS Err: Reliable write Fail @ %u unit, Err:%lx\n", u32_i, n);
			return CMD_RET_FAILURE;
		}

		printf("%03u%% \r", (u32_i+1)*100/UFS_PWRCUT_TEST_UNIT_CNT);
	}

	// ---------------------------------------
	printf("\ncheck ...\n");

	if (ufstest_CheckPwrCutCtrlBlk(u8_DataBuf, u32_BlkStartAddr-1) != 0) {
		UBOOT_ERROR("UFS Err: read CtrlBlk fail\n");
	    return CMD_RET_FAILURE;
	}

	for (u32_i=0; u32_i<UFS_PWRCUT_TEST_UNIT_CNT; u32_i++) {
		n = blk_read_devnum(IF_TYPE_UFS, TEST_LUN, u32_BlkStartAddr + u32_i*(UFS_PWRCUT_TEST_UNIT_BYTECNT>>12),
			UFS_PWRCUT_TEST_UNIT_BYTECNT>>12, (ulong *)u8_DataBuf);
		if(n != UFS_PWRCUT_TEST_UNIT_BYTECNT>>12) {
			UBOOT_ERROR("UFS Err: Reliable read Fail @ %u unit, Err:%lx\n", u32_i, n);
			return CMD_RET_FAILURE;
		}

		for (u32_j=0; u32_j < UFS_PWRCUT_TEST_UNIT_BYTECNT; u32_j++) {
		    if (u8_DataBuf[u32_j] != UFS_PWRCUT_DATA0) {
			    UBOOT_ERROR("UFS Err: Data Mismatch:  Blk:%Xh  ByteIdx:%Xh  ByteVal:%Xh \n",
					u32_BlkStartAddr+u32_i*(UFS_PWRCUT_TEST_UNIT_BYTECNT>>12)+(u32_j>>12),
					u32_j & 0x1FF, u8_DataBuf[u32_j]);
			    return CMD_RET_FAILURE;
		    }
	    }

		printf("%03u%% \r", (u32_i+1)*100/UFS_PWRCUT_TEST_UNIT_CNT);
	}

	printf("\n init success\n");
	return CMD_RET_SUCCESS;
}

u32 ufstest_pwrcut_test(u8* u8_DataBuf, u32 u32_BlkStartAddr, u8* u8_ReadDataBuf)
{
	u32 u32_i, u32_j;
	u32 u32_err = 0;
	u32 u32_T0, u32_BlkAddr;
	UFS_PWRCUT_CTRLBLK_t CtrlBlk_t;
	ulong n;

	// ---------------------------------------
	printf("\n UFS PwrCut Test, checking ...\n");

	// get Ctrl Blk
	if (ufstest_CheckPwrCutCtrlBlk(u8_DataBuf, u32_BlkStartAddr) != 0) {
		UBOOT_ERROR("UFS Err: read CtrlBlk fail\n");
		return CMD_RET_FAILURE;
	}
	memcpy(&CtrlBlk_t, u8_DataBuf, sizeof(UFS_PWRCUT_CTRLBLK_t));

	// check data
	u32_BlkStartAddr++;

	for (u32_i=0; u32_i<UFS_PWRCUT_TEST_UNIT_CNT; u32_i++) {
		n = blk_read_devnum(IF_TYPE_UFS, TEST_LUN, u32_BlkStartAddr + u32_i*(UFS_PWRCUT_TEST_UNIT_BYTECNT>>12),
			UFS_PWRCUT_TEST_UNIT_BYTECNT>>12, (ulong *)u8_DataBuf);
		if(n != UFS_PWRCUT_TEST_UNIT_BYTECNT>>12) {
			UBOOT_ERROR("UFS Err: Reliable read Fail @ %u unit, Err:%lx\n", u32_i, n);
			return CMD_RET_FAILURE;
		}

		if (u32_i == CtrlBlk_t.u8_TargetUnit) {
			for (u32_j=0; u32_j < UFS_PWRCUT_TEST_UNIT_BYTECNT; u32_j++) {
			    if (u8_DataBuf[u32_j]!=UFS_PWRCUT_DATA0 &&
					u8_DataBuf[u32_j]!=UFS_PWRCUT_DATA1) {
				    UBOOT_ERROR("\nUFS Err: Target Data Mismatch:  Unit:%Xh  "
						"Blk: %Xh + %Xh x %Xh + %Xh = %Xh, ByteIdx:%Xh  ByteVal:%Xh \n",
						u32_i,
						u32_BlkStartAddr, u32_i, (UFS_PWRCUT_TEST_UNIT_BYTECNT>>12), (u32_j>>12),
						u32_BlkStartAddr+u32_i*(UFS_PWRCUT_TEST_UNIT_BYTECNT>>12)+(u32_j>>12),
						u32_j & 0xFFF, u8_DataBuf[u32_j]);
					ufstest_DumpsPwrCutCtrlBlk(&CtrlBlk_t);
					print_hex_dump("", DUMP_PREFIX_ADDRESS, 16, 4, &u8_DataBuf[u32_j]-0x10, 
								0x30, false);

				    return CMD_RET_FAILURE;
			    }
		    }
			// recover TargetUnit
			u32_BlkAddr = u32_BlkStartAddr +
				CtrlBlk_t.u8_TargetUnit *
				(UFS_PWRCUT_TEST_UNIT_BYTECNT>>UFS_SECTOR_4KBYTE_BITS);
			memset(u8_DataBuf, CtrlBlk_t.au8_TaegetData[CtrlBlk_t.u8_TargetUnit], UFS_PWRCUT_TEST_UNIT_BYTECNT);
			n = blk_write_devnum(IF_TYPE_UFS, TEST_LUN, u32_BlkAddr,
				UFS_PWRCUT_TEST_UNIT_BYTECNT>>12, (ulong *)u8_DataBuf);
			if(n != UFS_PWRCUT_TEST_UNIT_BYTECNT>>12) {
				UBOOT_ERROR("UFS Err: write TargetUnit fail, Err:%lx\n",n);
				return CMD_RET_FAILURE;
			}
		}
		else {
			for (u32_j=0; u32_j < UFS_PWRCUT_TEST_UNIT_BYTECNT; u32_j++) {
			    if (u8_DataBuf[u32_j]!=CtrlBlk_t.au8_TaegetData[u32_i]) {
				    UBOOT_ERROR("\nUFS Err: Data Mismatch:  Unit:%Xh  "
						"Blk: %Xh + %Xh x %Xh + %Xh = %Xh, ByteIdx:%Xh  ByteVal:%Xh \n",
						u32_i,
						u32_BlkStartAddr, u32_i, (UFS_PWRCUT_TEST_UNIT_BYTECNT>>12), (u32_j>>12),
						u32_BlkStartAddr+u32_i*(UFS_PWRCUT_TEST_UNIT_BYTECNT>>12)+(u32_j>>12),
						u32_j & 0xFFF, u8_DataBuf[u32_j]);
					ufstest_DumpsPwrCutCtrlBlk(&CtrlBlk_t);
					print_hex_dump("", DUMP_PREFIX_ADDRESS, 16, 4, &u8_DataBuf[u32_j]-0x10, 
								0x30, false);

					return CMD_RET_FAILURE;
			    }
		    }
		}

		printf("%03u%% \r", (u32_i+1)*100/UFS_PWRCUT_TEST_UNIT_CNT);
	}

	printf("   ok\n");

    // ---------------------------------------
	while(1) {
		u32_T0 = timer_get_boot_us();
		CtrlBlk_t.u8_TargetUnit = u32_T0 % UFS_PWRCUT_TEST_UNIT_CNT;
		CtrlBlk_t.au8_TaegetData[CtrlBlk_t.u8_TargetUnit] ^= 0xFF;
		CtrlBlk_t.u32_CheckSum = ufstest_ChkSum((u8*)&CtrlBlk_t.u8_TargetUnit,
		    sizeof(UFS_PWRCUT_CTRLBLK_t)-4);
		n = blk_write_devnum(IF_TYPE_UFS, TEST_LUN, u32_BlkStartAddr-1, 1, (ulong *)&CtrlBlk_t);
		if(n != 1) {
			UBOOT_ERROR("UFS Err: write CtrlBlk, Err:%x\n", u32_err);
			return CMD_RET_FAILURE;
		}

		u32_BlkAddr = u32_BlkStartAddr +
			CtrlBlk_t.u8_TargetUnit *
			(UFS_PWRCUT_TEST_UNIT_BYTECNT>>UFS_SECTOR_4KBYTE_BITS);
		memset(u8_DataBuf, CtrlBlk_t.au8_TaegetData[CtrlBlk_t.u8_TargetUnit], UFS_PWRCUT_TEST_UNIT_BYTECNT);

		printf("\nWriting Unit %u, Blk %u, Data %02Xh ...\n",
			CtrlBlk_t.u8_TargetUnit, u32_BlkAddr, CtrlBlk_t.au8_TaegetData[CtrlBlk_t.u8_TargetUnit]);

		n = blk_write_devnum(IF_TYPE_UFS, TEST_LUN, u32_BlkAddr,
			UFS_PWRCUT_TEST_UNIT_BYTECNT>>12, (ulong *)u8_DataBuf);
		if(n != UFS_PWRCUT_TEST_UNIT_BYTECNT>>12) {
			UBOOT_ERROR("UFS Err: write Fail, Err:%lx\n", n);
			return CMD_RET_FAILURE;
		}

		if (u8_ReadDataBuf) {
			printf("reading back and compare...\n");
			n = blk_read_devnum(IF_TYPE_UFS, TEST_LUN, u32_BlkAddr,
			UFS_PWRCUT_TEST_UNIT_BYTECNT>>12, (ulong *)u8_ReadDataBuf);
			if(n != UFS_PWRCUT_TEST_UNIT_BYTECNT>>12) {
				UBOOT_ERROR("UFS Err: read Fail, Err:%x\n",u32_err);
				return CMD_RET_FAILURE;
			}

			if (memcmp(u8_DataBuf, u8_ReadDataBuf, UFS_PWRCUT_TEST_UNIT_BYTECNT)) {
				UBOOT_ERROR("UFS Err: cmp Fail\n");
				return CMD_RET_FAILURE;
			}
		}
		
		printf("ok\n");
    }

	return CMD_RET_SUCCESS;
}

