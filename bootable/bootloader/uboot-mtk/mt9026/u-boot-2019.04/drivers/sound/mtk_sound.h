// SPDX-License-Identifier: (GPL-2.0 OR BSD-3-Clause)
/*
 * Copyright (c) 2021 MediaTek Inc.
 */

#ifndef _MTK_SOUND_HEADER
#define _MTK_SOUND_HEADER

#define	ALSA_WRITE_BYTE(reg, val)	(writeb(val, reg))
#define	ALSA_WRITE_WORD(reg, val)	(writew(val, reg))

#define	ALSA_WRITE_MASK_BYTE(reg, mask, val)			\
	ALSA_WRITE_BYTE(reg, (readb(reg) & ~(mask)) | (val & mask))

#define	ALSA_WRITE_MASK_2BYTE(reg, mask, val)			\
	ALSA_WRITE_WORD(reg, (readw(reg) & ~(mask)) | (val & mask))

#define DEFAULT_PART		"bootdata"
#define DEFAULT_NAME		"boot0.mp3"
#define DEFAULT_VOL		20
#define VOL_STEP		10
#define MUSIC_VOL_MAP_NUM	11
#define MAX_VOLUME		100

unsigned char mtk_alsa_read_reg_byte(unsigned int addr);
unsigned short mtk_alsa_read_reg(unsigned int addr);
void mtk_alsa_write_reg_mask_byte(unsigned int addr, unsigned char mask, unsigned char val);
void mtk_alsa_write_reg_byte(unsigned int addr, unsigned char val);
void mtk_alsa_write_reg_mask(unsigned int addr,	unsigned short mask, unsigned short val);
void mtk_alsa_write_reg(unsigned int addr, unsigned short val);
bool mtk_alsa_read_reg_byte_polling(unsigned int addr, unsigned char mask, unsigned char val, int counter);
char *mtk_sound_data_load_to_memory(char *part, const char *filename, loff_t *filesize);

#endif