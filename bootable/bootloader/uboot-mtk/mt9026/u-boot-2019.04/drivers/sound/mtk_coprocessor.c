// SPDX-License-Identifier: (GPL-2.0 OR BSD-3-Clause)
/*
 * Copyright (c) 2021 MediaTek Inc.
 */

#include <common.h>
#include <dm.h>
#include <sound.h>
#include <asm/gpio.h>
#include <linux/io.h>
#include <utility.h>
#include <debug_impl.h>
#include <system_impl.h>
#include <fs.h>
#include <dts_parser.h>

#include "mtk_sound.h"

#define	BYTES_IN_MIU_LINE_LOG2	4
#define	ICACHE_MEMOFFSET_LOG2	12

static struct copr_register REG;

enum {
	AU_SE_SYSTEM = 0,
	AU_DVB2_NONE,
	AU_SIF,
	AU_CODE_SEGMENT_MAX,
};

#define MST_CODEC_PM1_FLASH_ADDR                  0x00000000
#define MST_CODEC_PM2_FLASH_ADDR                  0x00000c90
#define MST_CODEC_PM3_FLASH_ADDR                  0x00000de0
#define MST_CODEC_PM4_FLASH_ADDR                  0x00004d58
#define MST_CODEC_DEC_PM1_FLASH_ADDR              0x00004d70
#define MST_CODEC_DEC_PM2_FLASH_ADDR              0x00004dd0
#define MST_CODEC_DEC_PM3_FLASH_ADDR              0x00004e00
#define MST_CODEC_DEC_PM4_FLASH_ADDR              0x00004e18
#define MST_CODEC_SIF_PM1_FLASH_ADDR              0x00004e30
#define MST_CODEC_SIF_PM2_FLASH_ADDR              0x00004e90
#define MST_CODEC_SIF_PM3_FLASH_ADDR              0x00004ec0
#define MST_CODEC_MP3_PM1_FLASH_ADDR              0x00004ed8
#define MST_CODEC_MP3_PM2_FLASH_ADDR              0x00004ef0
#define MST_CODEC_MP3_PM3_FLASH_ADDR              0x00008718
#define MST_CODEC_MP3_PM4_FLASH_ADDR              0x0000b0a0
#define MST_CODEC_PM1_ADDR                        0x00000000
#define MST_CODEC_PM1_SIZE                        0x00000c8a
#define MST_CODEC_PM2_ADDR                        0x00001200
#define MST_CODEC_PM2_SIZE                        0x0000013b
#define MST_CODEC_PM3_ADDR                        0x00008000
#define MST_CODEC_PM3_SIZE                        0x00003f72
#define MST_CODEC_PM4_ADDR                        0x00012c00
#define MST_CODEC_PM4_SIZE                        0x00000003
#define MST_CODEC_DEC_PM1_ADDR                    0x00001100
#define MST_CODEC_DEC_PM1_SIZE                    0x00000057
#define MST_CODEC_DEC_PM2_ADDR                    0x00001500
#define MST_CODEC_DEC_PM2_SIZE                    0x00000021
#define MST_CODEC_DEC_PM3_ADDR                    0x0000a000
#define MST_CODEC_DEC_PM3_SIZE                    0x00000003
#define MST_CODEC_DEC_PM4_ADDR                    0x00013800
#define MST_CODEC_DEC_PM4_SIZE                    0x00000012
#define MST_CODEC_SIF_PM1_ADDR                    0x00001100
#define MST_CODEC_SIF_PM1_SIZE                    0x00000057
#define MST_CODEC_SIF_PM2_ADDR                    0x00001500
#define MST_CODEC_SIF_PM2_SIZE                    0x00000021
#define MST_CODEC_SIF_PM3_ADDR                    0x0000a000
#define MST_CODEC_SIF_PM3_SIZE                    0x00000003
#define MST_CODEC_MP3_PM1_ADDR                    0x00001100
#define MST_CODEC_MP3_PM1_SIZE                    0x00000009
#define MST_CODEC_MP3_PM2_ADDR                    0x00001500
#define MST_CODEC_MP3_PM2_SIZE                    0x00003819
#define MST_CODEC_MP3_PM3_ADDR                    0x0000a000
#define MST_CODEC_MP3_PM3_SIZE                    0x00002982
#define MST_CODEC_MP3_PM4_ADDR                    0x00013800
#define MST_CODEC_MP3_PM4_SIZE                    0x00005070

struct dsp_alg_info {
	unsigned int	cm_addr;
	unsigned int	cm_len;
	unsigned char	*cm_buf;
	unsigned int	pm_addr;
	unsigned int	pm_len;
	unsigned char	*pm_buf;
	unsigned int	cache_addr;
	unsigned int	cache_len;
	unsigned char	*cache_buf;
	unsigned int	prefetch_addr;
	unsigned int	prefetch_len;
	unsigned char	*prefetch_buf;
};

struct copr_register {
	u32 DEC_AUD_CTRL;
	u32 DEC_MAD_OFFSET_BASE_L;
	u32 DEC_MAD_OFFSET_BASE_H;
	u32 DEC_MAD_OFFSET_BASE_EXT;
	u32 SE_AUD_CTRL;
	u32 SE_MAD_OFFSET_BASE_L;
	u32 SE_MAD_OFFSET_BASE_H;
	u32 SE_MAD_OFFSET_BASE_EXT;
	u32 SE_IDMA_CTRL0;
	u32 SE_DSP_ICACHE_BASE_L;
	u32 SE_BDMA_CFG;
	u32 FD230_SEL;
	u32 SE_DSP_BRG_DATA_L;
	u32 SE_DSP_BRG_DATA_H;
	u32 SE_IDMA_WRBASE_ADDR_L;
	u32 RIU_MAIL_00;
	u32 DSP_POWER_DOWN_H;
	u32 DSP_DBG_CMD1;
	u32 DSP_DBG_RESULT1;
	u32 SE_MBASE_H;
	u32 SE_MSIZE_H;
	u32 SE_MCFG;
	u32 SE_MBASE_EXT;
};

struct mtk_copr_priv {
	unsigned int	cpu_base;
	phys_addr_t	bus_addr;
	phys_size_t	buffer_size;
	const char	*fw_name;
	unsigned int	fw_size;
	ofnode		subnode;
};

struct dsp_alg_info dsp_info[AU_CODE_SEGMENT_MAX];

void mtk_copr_dsp_setmeminfo(bool miu_bank, phys_addr_t dsp_base,
				phys_addr_t icache_base)
{
	phys_addr_t dsp_line = dsp_base >> BYTES_IN_MIU_LINE_LOG2;
	phys_addr_t icache_offset = icache_base >> ICACHE_MEMOFFSET_LOG2;

	mtk_alsa_write_reg_mask_byte(REG.RIU_MAIL_00 + 1, 0x01, miu_bank);
	mtk_alsa_write_reg_mask_byte(REG.DEC_AUD_CTRL, 0x01, 0x00);
	mtk_alsa_write_reg_mask(REG.SE_AUD_CTRL, 0x04FF, 0x0400);

	/* COPR BASE */
	mtk_alsa_write_reg_mask_byte(REG.DEC_MAD_OFFSET_BASE_L + 1, 0xFF,
					(unsigned char)(dsp_line & 0xFF));
	mtk_alsa_write_reg_mask_byte(REG.SE_MAD_OFFSET_BASE_L + 1, 0xFF,
					(unsigned char)(dsp_line & 0xFF));
	mtk_alsa_write_reg(REG.DEC_MAD_OFFSET_BASE_H, (unsigned short)(dsp_line >> 8));
	mtk_alsa_write_reg(REG.SE_MAD_OFFSET_BASE_H, (unsigned short)(dsp_line >> 8));
	mtk_alsa_write_reg_mask_byte(REG.DEC_MAD_OFFSET_BASE_EXT, 0x0F,
					(unsigned char)(dsp_line >> 24));
	mtk_alsa_write_reg_mask_byte(REG.SE_MAD_OFFSET_BASE_EXT, 0x0F,
					(unsigned char)(dsp_line >> 24));

	mtk_alsa_write_reg_mask_byte(REG.SE_MCFG, 0xFF, 0x02);
	mtk_alsa_write_reg(REG.SE_MBASE_H,
				(0x0054000 >> (BYTES_IN_MIU_LINE_LOG2 + 8))); // es addr
	mtk_alsa_write_reg_mask_byte(REG.SE_MBASE_EXT, 0xFF,
				((0x0054000 >> (BYTES_IN_MIU_LINE_LOG2 + 24)) & 0xFF));
	mtk_alsa_write_reg_mask(REG.SE_MSIZE_H, 0xFF00,
				((0x20000 >> (BYTES_IN_MIU_LINE_LOG2)) - 1)); // es size
	mtk_alsa_write_reg_mask_byte(REG.SE_MCFG, 0xFF, 0x04);
	mtk_alsa_write_reg(REG.SE_MBASE_H,
				(0x0074000 >> (BYTES_IN_MIU_LINE_LOG2 + 8))); // pcm addr
	mtk_alsa_write_reg_mask_byte(REG.SE_MBASE_EXT, 0xFF,
				((0x074000 >> (BYTES_IN_MIU_LINE_LOG2 + 24)) & 0xFF));
	mtk_alsa_write_reg_mask(REG.SE_MSIZE_H, 0xFF00,
				((0x66000 >> (BYTES_IN_MIU_LINE_LOG2)) - 1)); // pcm size

	mtk_alsa_write_reg_mask_byte(REG.DEC_AUD_CTRL, 0x40, 0x40);
	mtk_alsa_write_reg_mask_byte(REG.SE_AUD_CTRL, 0x40, 0x40);
	mtk_alsa_write_reg_mask_byte(REG.DEC_AUD_CTRL, 0x80, 0x80);
	mtk_alsa_write_reg_mask_byte(REG.SE_AUD_CTRL, 0x80, 0x80);

	/* ICACHE BASE */
	mtk_alsa_write_reg(REG.SE_DSP_ICACHE_BASE_L, (unsigned short)(icache_offset & 0xFFFF));
	mtk_alsa_write_reg_mask_byte(REG.SE_BDMA_CFG, 0x0F,
					(unsigned char)(icache_offset >> 16));

	mtk_alsa_write_reg_mask_byte(REG.DEC_AUD_CTRL, 0x80, 0x00);
	mtk_alsa_write_reg_mask_byte(REG.SE_AUD_CTRL, 0x80, 0x00);
	mtk_alsa_write_reg_mask_byte(REG.DEC_AUD_CTRL, 0x40, 0x00);
	mtk_alsa_write_reg_mask_byte(REG.SE_AUD_CTRL, 0x40, 0x00);
}

static void mtk_copr_get_dsp_info(struct mtk_copr_priv *priv, char *fw)
{
	u8 *fw_data;

	fw_data = (u8 *)fw;

	/* AU_SE_SYSTEM */
	dsp_info[AU_SE_SYSTEM].cm_addr = 0x0008;
	dsp_info[AU_SE_SYSTEM].cm_len = MST_CODEC_PM1_SIZE - 24;
	dsp_info[AU_SE_SYSTEM].cm_buf = fw_data + MST_CODEC_PM1_FLASH_ADDR + 24;

	dsp_info[AU_SE_SYSTEM].pm_addr = MST_CODEC_PM2_ADDR;
	dsp_info[AU_SE_SYSTEM].pm_len = MST_CODEC_PM2_SIZE;
	dsp_info[AU_SE_SYSTEM].pm_buf = fw_data + MST_CODEC_PM2_FLASH_ADDR;

	dsp_info[AU_SE_SYSTEM].cache_addr = MST_CODEC_PM3_ADDR;
	dsp_info[AU_SE_SYSTEM].cache_len = MST_CODEC_PM3_SIZE;
	dsp_info[AU_SE_SYSTEM].cache_buf = fw_data + MST_CODEC_PM3_FLASH_ADDR;

	dsp_info[AU_SE_SYSTEM].prefetch_addr = MST_CODEC_PM4_ADDR;
	dsp_info[AU_SE_SYSTEM].prefetch_len = MST_CODEC_PM4_SIZE;
	dsp_info[AU_SE_SYSTEM].prefetch_buf = fw_data +
						MST_CODEC_PM4_FLASH_ADDR;

	/* AU_DVB2_NONE */
	dsp_info[AU_DVB2_NONE].cm_addr = MST_CODEC_DEC_PM1_ADDR;
	dsp_info[AU_DVB2_NONE].cm_len = MST_CODEC_DEC_PM1_SIZE;
	dsp_info[AU_DVB2_NONE].cm_buf = fw_data + MST_CODEC_DEC_PM1_FLASH_ADDR;

	dsp_info[AU_DVB2_NONE].pm_addr = MST_CODEC_DEC_PM2_ADDR;
	dsp_info[AU_DVB2_NONE].pm_len = MST_CODEC_DEC_PM2_SIZE;
	dsp_info[AU_DVB2_NONE].pm_buf = fw_data + MST_CODEC_DEC_PM2_FLASH_ADDR;

	dsp_info[AU_DVB2_NONE].cache_addr = MST_CODEC_DEC_PM3_ADDR;
	dsp_info[AU_DVB2_NONE].cache_len = MST_CODEC_DEC_PM3_SIZE;
	dsp_info[AU_DVB2_NONE].cache_buf = fw_data +
						MST_CODEC_DEC_PM3_FLASH_ADDR;

	dsp_info[AU_DVB2_NONE].prefetch_addr = MST_CODEC_DEC_PM4_ADDR;
	dsp_info[AU_DVB2_NONE].prefetch_len = MST_CODEC_DEC_PM4_SIZE;
	dsp_info[AU_DVB2_NONE].prefetch_buf = fw_data +
						MST_CODEC_DEC_PM4_FLASH_ADDR;

	dsp_info[AU_SIF].cm_addr = MST_CODEC_MP3_PM1_ADDR;
	dsp_info[AU_SIF].cm_len = MST_CODEC_MP3_PM1_SIZE;
	dsp_info[AU_SIF].cm_buf = fw_data + MST_CODEC_MP3_PM1_FLASH_ADDR;

	dsp_info[AU_SIF].pm_addr = MST_CODEC_MP3_PM2_ADDR;
	dsp_info[AU_SIF].pm_len = MST_CODEC_MP3_PM2_SIZE;
	dsp_info[AU_SIF].pm_buf = fw_data + MST_CODEC_MP3_PM2_FLASH_ADDR;

	dsp_info[AU_SIF].cache_addr = MST_CODEC_MP3_PM3_ADDR;
	dsp_info[AU_SIF].cache_len = MST_CODEC_MP3_PM3_SIZE;
	dsp_info[AU_SIF].cache_buf = fw_data + MST_CODEC_MP3_PM3_FLASH_ADDR;

	dsp_info[AU_SIF].prefetch_addr = MST_CODEC_MP3_PM4_ADDR;
	dsp_info[AU_SIF].prefetch_len = MST_CODEC_MP3_PM4_SIZE;
	dsp_info[AU_SIF].prefetch_buf = fw_data + MST_CODEC_MP3_PM4_FLASH_ADDR;
}

bool mtk_alsa_write_sram_segment(unsigned int sram_address,
	unsigned char *sram_seg_buf, unsigned int sram_seg_len, bool sram_type)
{
	bool ret;
	int i;

	mtk_alsa_write_reg_mask(REG.SE_BDMA_CFG, 0x8080, 0x0000);
	mtk_alsa_write_reg_mask_byte(REG.FD230_SEL, 0x01, 0x01);

	ret = mtk_alsa_read_reg_byte_polling(REG.SE_IDMA_CTRL0, 0x18, 0x00, 200);
	if (ret)
		return 1;

	mtk_alsa_write_reg(REG.SE_IDMA_WRBASE_ADDR_L, sram_address);

	for (i = 0; i < (sram_seg_len / 3); i++) {
		mtk_alsa_write_reg_byte(REG.SE_DSP_BRG_DATA_L, *(sram_seg_buf + (3 * i) + 1));
		mtk_alsa_write_reg_byte(REG.SE_DSP_BRG_DATA_H, *(sram_seg_buf + (3 * i) + 2));
		ret = mtk_alsa_read_reg_byte_polling(REG.SE_IDMA_CTRL0, 0x04, 0x04, 200);
		if (ret)
			return 1;

		mtk_alsa_write_reg_byte(REG.SE_DSP_BRG_DATA_L, *(sram_seg_buf + (3 * i)));
		mtk_alsa_write_reg_byte(REG.SE_DSP_BRG_DATA_H, 0x00);
		ret = mtk_alsa_read_reg_byte_polling(REG.SE_IDMA_CTRL0, 0x18, 0x00, 200);
		if (ret)
			return 1;
	}

	return 0;
}

static int mtk_copr_load_dsp_image(struct mtk_copr_priv *priv, char *fw,
				unsigned char dsp_select)
{
	struct dsp_alg_info *pau_info = NULL;
	phys_addr_t dsp_va;
	phys_addr_t MIU_addr;
	int ret;

	/* get dsp base */
	dsp_va = priv->bus_addr;

	pau_info = &dsp_info[dsp_select];
	/* Download PM of Algorithm */
	ret = mtk_alsa_write_sram_segment(pau_info->pm_addr,
			pau_info->pm_buf, pau_info->pm_len, 0);
	if (ret) {
		debug("[SND COPR]load dsp segememt %d pm fail\n", ret);
		return ret;
	}
	/* Download CM */
	ret = mtk_alsa_write_sram_segment(pau_info->cm_addr,
			pau_info->cm_buf, pau_info->cm_len, 0);
	if (ret) {
		debug("[SND COPR]load dsp segememt %d cm fail\n", ret);
		return ret;
	}
	/* Download PM of PreFetch */
	if (pau_info->prefetch_len != 0) {
		MIU_addr = pau_info->prefetch_addr * 3 + dsp_va;
		memcpy((void *)MIU_addr, (void *)pau_info->prefetch_buf,
						pau_info->prefetch_len);

		flush_cache((unsigned long)MIU_addr, pau_info->prefetch_len);

	}
	/* Download PM of Cache */
	if (pau_info->cache_len != 0) {
		MIU_addr = pau_info->cache_addr * 3 + dsp_va;
		memcpy((void *)MIU_addr, (void *)pau_info->cache_buf,
							pau_info->cache_len);

		flush_cache((unsigned long)MIU_addr, pau_info->cache_len);
	}

	if (dsp_select == AU_SE_SYSTEM) {
		pau_info->cm_buf = pau_info->cm_buf - 24;
		ret = mtk_alsa_write_sram_segment(0x0001,
					pau_info->cm_buf + 3, 21, 0);
		if (ret) {
			debug("[SND COPR]load dsp seg %d pm 6 fail\n", ret);
			return ret;
		}
		ret = mtk_alsa_write_sram_segment(0x0000,
					pau_info->cm_buf, 3, 0);
		if (ret) {
			debug("[SND COPR]load dsp seg %d pm 0 fail\n", ret);
			return ret;
		}
	}

	return 0;
}

void mtk_copr_load_image_to_dsp_and_run(char *buf, struct mtk_copr_priv *priv)
{
	int ret;
	int time_out = 0;

	/* toggle to initialize DAC DATA SRAM. */
	mtk_alsa_write_reg_mask_byte(REG.RIU_MAIL_00, 0x02, 0x02);
	udelay(1000);
	mtk_alsa_write_reg_mask_byte(REG.RIU_MAIL_00, 0x02, 0x00);
	/* reset dsp */
	mtk_alsa_write_reg_byte(REG.SE_IDMA_CTRL0, 0x02);
	udelay(1000);
	mtk_alsa_write_reg_byte(REG.SE_IDMA_CTRL0, 0x03);

	/* assign dsp info */
	mtk_copr_get_dsp_info(priv, buf);

	mtk_alsa_write_reg_mask(REG.DSP_DBG_CMD1, 0xFFFF, 0x0000);
	/* load dsp binary */
	/* load null */
	ret = mtk_copr_load_dsp_image(priv, buf, AU_SIF);
	if (ret)
		debug("[SND COPR]load dsp code AU_DVB2_NONE fail\n");
	/* load aucode_s */
	ret = mtk_copr_load_dsp_image(priv, buf, AU_SE_SYSTEM);
	if (ret)
		debug("[SND COPR]load dsp code AU_SE_SYSTEM fail\n");

	/* Wait Dsp init finished Ack */
	while (time_out++ < 200) {
		if (mtk_alsa_read_reg_byte(REG.DSP_DBG_RESULT1) == 0xE3)
			break;
		udelay(1000);
	}

	if (time_out >= 200)
		debug("[SND COPR]DSP2 Re-Active\n");
	else
		debug("[SND COPR]audio DSP_SE LoadCode success..\n");

	/* inform DSP to start to run */
	mtk_alsa_write_reg_mask(REG.DSP_DBG_CMD1, 0xFF00, 0xF300);
}

static u32 mtk_sound_get_cpu_base(void)
{
	u32 base = 0;
	ofnode node;
	node = ofnode_path("/memory_info");
	if (!ofnode_valid(node))
		return base;

	if (ofnode_read_u32(node, "cpu_emi0_base", &base) < 0)
		return 0;
	else
		return base;
}

static u32 copr_get_dts_u32(struct mtk_copr_priv *priv, const char *name)
{
	u32 value = 0;

	if (ofnode_read_u32(priv->subnode, name, &value) < 0)
		debug("[SND COPR]get %s fail\n", name);

	return value;
}

static u32 mtk_sound_get_copr_reg(struct mtk_copr_priv *priv)
{
	ofnode node;
	ofnode subnode;

	node = ofnode_path("/mediatek-sound-register");
	if (!ofnode_valid(node))
		debug("[SND COPR]get sound reg dts fail\n");

	subnode = ofnode_find_subnode(node, "coprocessor-register");
	if (!ofnode_valid(subnode))
		debug("[SND COPR]get coprocessor reg fail\n");

	priv->subnode = subnode;

	REG.DEC_AUD_CTRL = copr_get_dts_u32(priv, "reg_dec_aud_ctrl");
	REG.DEC_MAD_OFFSET_BASE_L = copr_get_dts_u32(priv, "reg_dec_mad_offset_base_l");
	REG.DEC_MAD_OFFSET_BASE_H = copr_get_dts_u32(priv, "reg_dec_mad_offset_base_h");
	REG.DEC_MAD_OFFSET_BASE_EXT = copr_get_dts_u32(priv, "reg_dec_mad_offset_base_ext");
	REG.SE_AUD_CTRL = copr_get_dts_u32(priv, "reg_se_aud_ctrl");
	REG.SE_MAD_OFFSET_BASE_L = copr_get_dts_u32(priv, "reg_se_mad_offset_base_l");
	REG.SE_MAD_OFFSET_BASE_H = copr_get_dts_u32(priv, "reg_se_mad_offset_base_h");
	REG.SE_MAD_OFFSET_BASE_EXT = copr_get_dts_u32(priv, "reg_se_mad_offset_base_ext");
	REG.SE_IDMA_CTRL0 = copr_get_dts_u32(priv, "reg_se_idma_ctrl0");
	REG.SE_DSP_ICACHE_BASE_L = copr_get_dts_u32(priv, "reg_se_dsp_icache_base_l");
	REG.SE_BDMA_CFG = copr_get_dts_u32(priv, "reg_se_bdma_cfg");
	REG.FD230_SEL = copr_get_dts_u32(priv, "reg_fd230_sel");
	REG.SE_DSP_BRG_DATA_L = copr_get_dts_u32(priv, "reg_se_dsp_brg_data_l");
	REG.SE_DSP_BRG_DATA_H = copr_get_dts_u32(priv, "reg_se_dsp_brg_data_h");
	REG.SE_IDMA_WRBASE_ADDR_L = copr_get_dts_u32(priv, "reg_se_idma_wrbase_addr_l");
	REG.RIU_MAIL_00 = copr_get_dts_u32(priv, "reg_riu_mail_00");
	REG.DSP_POWER_DOWN_H = copr_get_dts_u32(priv, "reg_dsp_power_down_h");
	REG.DSP_DBG_CMD1 = copr_get_dts_u32(priv, "reg_dsp_dbg_cmd1");
	REG.DSP_DBG_RESULT1 = copr_get_dts_u32(priv, "reg_dsp_dbg_result1");
	REG.SE_MBASE_H = copr_get_dts_u32(priv, "reg_se_mbase_h");
	REG.SE_MSIZE_H = copr_get_dts_u32(priv, "reg_se_msize_h");
	REG.SE_MCFG = copr_get_dts_u32(priv, "reg_se_mcfg");
	REG.SE_MBASE_EXT = copr_get_dts_u32(priv, "reg_se_mbase_ext");

	return 0;
}

static ofnode mtk_sound_get_mmap_ofnode(void)
{
	ofnode node;
	node = ofnode_path("/mmap_info/MI_MAD_ADV_BUF");
	return node;
}

static int mtk_copr_setup(struct udevice *dev)
{
	struct mtk_copr_priv *priv = dev_get_priv(dev);
	phys_addr_t dsp_phy_addr;
	char *buf = NULL;
	loff_t size;

	buf = mtk_sound_data_load_to_memory(DEFAULT_PART, priv->fw_name, &size);
	if(buf == 0) {
		dev_err(dev, "[SND COPR]read firmware fail\n");
		return -EINVAL;
	}
	dsp_phy_addr = priv->bus_addr - priv->cpu_base;

	/* DSP power up command, DO NOT touch bit3 */
	mtk_alsa_write_reg_mask_byte(REG.DSP_POWER_DOWN_H, 0x02, 0x00);
	/* set mem info */
	mtk_copr_dsp_setmeminfo(0, dsp_phy_addr, dsp_phy_addr);
	/* load dsp binary */
	mtk_copr_load_image_to_dsp_and_run(buf, priv);

	free(buf);

	return 0;
}

static int mtk_copr_probe(struct udevice *dev)
{
	struct mtk_copr_priv *priv = dev_get_priv(dev);
	u32 cpu_base;
	ofnode node;
	u32 addr[4];
	int ret;

	/* parse dts */
	cpu_base = mtk_sound_get_cpu_base();
	if (!cpu_base) {
		dev_err(dev, "[SND COPR]not find cpu base\n");
		return -EINVAL;
	}
	priv->cpu_base = cpu_base;

	node = mtk_sound_get_mmap_ofnode();
	if (!ofnode_valid(node)) {
		dev_err(dev, "[SND COPR]not find mmap node\n");
		return -EINVAL;
	}
	if (ofnode_read_u32_array(node, "reg", addr, 4)) {
		dev_err(dev, "[SND COPR]not find address\n");
		return -EINVAL;
	}
	priv->bus_addr = addr[1];
	priv->buffer_size = addr[3];

	priv->fw_name = dev_read_string(dev, "fw_name");
	if (!priv->fw_name) {
		dev_err(dev, "[SND COPR]can't get fw name\n");
		return -EINVAL;
	}

	ret = dev_read_u32(dev, "fw_size", &priv->fw_size);
	if (ret) {
		dev_err(dev, "[SND COPR]can't get fw size\n");
		return -EINVAL;
	}

	ret  = mtk_sound_get_copr_reg(priv);
	if (ret) {
		dev_err(dev, "[SND COPR]can't get copr register\n");
		return -EINVAL;
	}

	return 0;
}

static const struct sound_ops mtk_copr_ops = {
	.setup	= mtk_copr_setup,
};

static const struct udevice_id mtk_copr_ids[] = {
	{ .compatible = "mediatek,mtk-snd-coprocessor" },
	{ }
};

U_BOOT_DRIVER(mediatek_coprocessor) = {
	.name		= "mediatek_coprocessor",
	.id		= UCLASS_SOUND,
	.of_match	= mtk_copr_ids,
	.probe		= mtk_copr_probe,
	.ops		= &mtk_copr_ops,
	.priv_auto_alloc_size = sizeof(struct mtk_copr_priv),
};
