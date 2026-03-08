// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (C) 2023 MediaTek Inc.
 */

#include "eMMC.h"

void fde_aes_sw_reset(void)
{
	REG_FCIE_SETBIT(FDE_AES_SW_RESET, BIT0);
	REG_FCIE_CLRBIT(FDE_AES_SW_RESET, BIT0);
}

void fde_aes_set_key(U8 *pu8_key, U32 u32_key_size)
{
	int i;
	for (i = 0; i < 8; i++)
		REG_FCIE_W(GET_REG_ADDR(FDE_AES_KEY, i), (pu8_key[i * 2 + 1] << 8) | pu8_key[i * 2]);

	REG_FCIE_CLRBIT(FDE_AES_CONTROL0, BIT_KEY_128BIT_SIZE);	//key size is 128 bit
	if (u32_key_size >= 24) {
		for (i = 8; i < 12; i++)
			REG_FCIE_W(GET_REG_ADDR(FDE_AES_KEY, i), (pu8_key[i * 2 + 1] << 8) | pu8_key[i * 2]);
		REG_FCIE_SETBIT(FDE_AES_CONTROL0, BIT_KEY_192BIT_SIZE); //key size is 192 bit
	}
	if (u32_key_size >= 32) {
		for (i = 12; i < 16; i++)
			REG_FCIE_W(GET_REG_ADDR(FDE_AES_KEY, i), (pu8_key[i * 2 + 1] << 8) | pu8_key[i * 2]);
		REG_FCIE_CLRBIT(FDE_AES_CONTROL0, BIT_MASK_KEY_SIZE);
		REG_FCIE_SETBIT(FDE_AES_CONTROL0, BIT_KEY_256BIT_SIZE); //key size is 256 bit
	}
}

void fde_set_set_iv(U8 *pu8_iv)
{
	int i;
	for (i = 0; i < 8; i++)
		REG_FCIE_W(GET_REG_ADDR(FDE_AES_IV, i), (pu8_iv[i * 2 + 1] << 8) | pu8_iv[i * 2]);
}

void fde_aes_set_ctr(U8 *pu8_ctr)
{
	int i;
	for (i = 0; i < 8; i++)
		REG_FCIE_W(GET_REG_ADDR(FDE_AES_CTR, i), (pu8_ctr[i * 2 + 1] << 8) | pu8_ctr[i * 2]);
}

void fde_aes_set_tcalc_key(U8 *pu8_tkey)
{
	int i;
	for (i = 0; i < 16; i++)
		REG_FCIE_W(GET_REG_ADDR(FDE_AES_XEX_HW_T_CALC_KEY, i), (pu8_tkey[i * 2 + 1] << 8) | pu8_tkey[i * 2]);
}

int fde_aes_set_mode(U8 u8_mode)
{
	if (u8_mode != ECB_MODE && u8_mode != CBC_MODE &&
	    u8_mode != CTR_MODE &&
	    u8_mode != OFB_MODE &&
	    u8_mode != CFB_MODE &&
	    u8_mode != XTS_MODE) {
		return -1;
	}
	REG_FCIE_CLRBIT(FDE_AES_CONTROL0, BIT_MASK_AES_MODE);
	REG_FCIE_SETBIT(FDE_AES_CONTROL0, u8_mode << BIT_AES_MODE_SHIFT);
	return 0;
}

int fde_aes_set_codec_dir(U8 u8_dir)
{
	if (u8_dir == ENABLE_ENC)
		REG_FCIE_CLRBIT(FDE_AES_CONTROL0, BIT_ENC_OR_DEC);
	else if (u8_dir == ENABLE_DEC)
		REG_FCIE_SETBIT(FDE_AES_CONTROL0, BIT_ENC_OR_DEC);
	else
		return -1;

	return 0;
}

void fde_aes_set_xts_lba(U32 u32_lba)
{
	U8 u8_ctr[16];
	U16 u16_mode;
	int i;
	REG_FCIE_R(FDE_AES_CONTROL0, u16_mode);
	u16_mode &= BIT_MASK_AES_MODE;
	u16_mode >>= BIT_AES_MODE_SHIFT;
	if (u16_mode != XTS_MODE)
		return ;
	//emmc_debug(0, 1, "LBA %Xh\n", u32_lba);
	for (i = 0; i < 16; i++) {
		if (i < sizeof(U32)) {
			u8_ctr[i] = u32_lba & 0xFF;
			u32_lba >>= 8;
		} else {
			u8_ctr[i] = 0;
		}
	}
	fde_aes_set_ctr(u8_ctr);
}

void fde_aes_set_xex_hw_t_calc_kick(int val)
{
	if (val == 1) {
		while ((REG_FCIE(FDE_AES_BUSY) & BIT_AES_BUSY))
			;

		REG_FCIE_SETBIT(FDE_AES_XEX_HW_T_CALC_KICK, BIT_AES_XEX_HW_T_CALC_KICK);
	} else if (val == 0)
		REG_FCIE_CLRBIT(FDE_AES_XEX_HW_T_CALC_KICK, BIT_AES_XEX_HW_T_CALC_KICK);
}

char mode_string[9][20] = {
	"ECB Mode",
	"CBC_Mode",
	"CTR_Mode",
	"",
	"",
	"OFB_Mode",
	"CFB_Mode",
	"",
	"XTS_Mode"
};

int fde_aes_init(void)//should be configured in TEE world
{
	U8 u8_mode;
	U8 u8_key[32], u8_iv[16], u8_ctr[16], u8_t_key[32];
	int i;
//	u8_mode = ECB_MODE;
	u8_mode = XTS_MODE;
//	u8_mode = CBC_MODE;
//	u8_mode = CTR_MODE;
//	u8_mode = OFB_MODE;
//	u8_mode = CFB_MODE;


	emmc_debug(0, 1, "Test %s\n", mode_string[u8_mode]);

	memset(u8_iv, 0, sizeof(u8_iv));
	memset(u8_ctr, 0, sizeof(u8_ctr));
	memset(u8_t_key, 0, sizeof(u8_t_key));
	for (i = 0 ; i < 32; i++) {
		u8_key[i] = i;
		if (u8_mode == XTS_MODE)
			u8_t_key[i] = 31 - i;//((0x3f - i*2 + 12) * 3) & 0xFF;

		#if 0
		if (u8_mode == CBC_MODE || u8_mode == OFB_MODE || u8_mode == CFB_MODE)
			if (i < 16)
				u8_IV[i] = 15 - i;//((0x3f - i * 2 + 12) * 3) & 0xFF;

		if (u8_mode == CTR_MODE)
			if (i < 16)
				u8_ctr[i] = 15 - i;//((0x3f - i * 2 + 12) * 3) & 0xFF;
		#endif
	}
//	if (u8_mode != XTS_MODE)
//		REG_FCIE_SETBIT(FDE_AES_FUN, BIT2);
	fde_aes_set_key(u8_key, 16);
	fde_set_set_iv(u8_iv);
	fde_aes_set_ctr(u8_ctr);
	fde_aes_set_tcalc_key(u8_t_key);
	fde_aes_set_mode(u8_mode);
	REG_FCIE_W(FDE_AES_DATA_UNIT0, 0x200);
	REG_FCIE_W(FDE_AES_DATA_UNIT1, 0x0);
	return 0;
}

void fde_aes_open(void)
{
	REG_FCIE_SETBIT(FDE_AES_FUN, BIT_AES_FUNC_EN);
}

void fde_aes_close(void)
{
	REG_FCIE_CLRBIT(FDE_AES_FUN, BIT_AES_FUNC_EN);
}

void FDE_AES_CQE_Open(void)
{
	REG_FCIE_SETBIT(FDE_AES_FUN, BIT_AES_AUTO_CONFIG_CMDQ);
}

void FDE_AES_CQE_Close(void)
{
	REG_FCIE_CLRBIT(FDE_AES_FUN, BIT_AES_AUTO_CONFIG_CMDQ);
}

int fde_aes_switch_state(void)
{
	return ((REG_FCIE(FDE_AES_FUN) & BIT_AES_SWITCH_ON)) ? 1 : 0;
}

void fde_aes_func_open(U8 u8_dir, U32 lba)
{
	if ((REG_FCIE(FDE_AES_FUN) & BIT_AES_SWITCH_ON)) {
		fde_aes_sw_reset();
		fde_aes_set_xts_lba(lba);
		fde_aes_set_codec_dir(u8_dir);
		fde_aes_set_xex_hw_t_calc_kick(1);
		fde_aes_open();
    }
}

void fde_aes_func_close(void)
{
	if ((REG_FCIE(FDE_AES_FUN) & (BIT_AES_SWITCH_ON | BIT_AES_FUNC_EN))
		== (BIT_AES_SWITCH_ON|BIT_AES_FUNC_EN)) {
		fde_aes_set_xex_hw_t_calc_kick(0);
		fde_aes_close();
	}
}

