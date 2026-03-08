// SPDX-License-Identifier: (GPL-2.0 OR BSD-3-Clause)
/*
 * Copyright (c) 2021 MediaTek Inc.
 */

#include <common.h>
#include <audio_codec.h>
#include <dm.h>
#include <i2s.h>
#include <sound.h>
#include <asm/gpio.h>
#include <linux/io.h>
#include <utility.h>
#include <debug_impl.h>
#include <system_impl.h>
#include <fs.h>
#include <dts_parser.h>
#include <utility.h>
#include <iniutility.h>

#include "mtk_sound.h"
#ifdef CONFIG_MULTICORES_PLATFORM
#include <smp/thread_info.h>
extern smp_spin_lock_t fs_spin_lock;
#endif

#define	BYTES_IN_MIU_LINE_LOG2	4
#define ES_MAX_SIZE		0x20000 - 0x20
#define AUDIO_SPEAKER_I2S	0
#define AUDIO_SPEAKER_DAC0	1
#define AUDIO_SPEAKER_DAC1	2

#ifdef CONFIG_DATA_SEPARATION
#include <mtk_dataindex.h>
#define MUSIC_SECTION		"BootLogoMusic"
#define MUSIC_INI_KEY		"m_pBootLogoMusicCfg_File"
#define MUSIC_FILE_KEY		"m_pBootLogoMusic_Path"
#endif

static struct mtk_sound_priv *priv;
static struct decoder_register REG;

struct decoder_register {
	u32 DEC_PLAY_CMD;
	u32 DEC_ES_WPTR;
	u32 CH_6_CFG;
	u32 VOLUME;
};

struct mtk_sound_priv {
	unsigned int	riu_base;
	unsigned int	bank_num;
	unsigned int	*reg_bank;
	phys_addr_t 	*reg_base;
	const char	*pre_init_name;
	const char	*init_name;
	const char 	*music_name;;
	unsigned int 	music_offset;
	phys_addr_t	bus_addr;
	phys_size_t	buffer_size;
	unsigned int	speaker_out;
	struct bootmusic_info	info;
};

/*
     change user power on music vol (range from 0~100) to Abs audio vol(range from 0x7F00 ~ 0x0D01)
*/
const u16 music_vol_map[MUSIC_VOL_MAP_NUM] =             // UI:  Mute
//           Volume  Table     (High Byte : Integer part  ; Low Byte : Fraction part)
{
    //   1       2       3       4       5       6       7       8       9       10
    0x7F00, //  00
    0x4200, //  10
    0x3000, //  20
    0x2500, //  30
    0x1D00, //  40
    0x1800, //  50
    0x1402, //  60
    0x1106, //  70
    0x0F05, //  80
    0x0E03, //  90
    0x0D01, //  100
};

u16 mtk_volume_table(u8 vol)
{
    u16 ret = 0;
    u8  step = vol / VOL_STEP;

    if (vol > MAX_VOLUME) {
        vol = DEFAULT_VOL;
        step = vol / VOL_STEP;
        ret = music_vol_map[step] - (double)(vol%VOL_STEP)/VOL_STEP*(music_vol_map[step] - music_vol_map[step+1]);
    }
    else if (!(vol % VOL_STEP))
        ret =  music_vol_map[step];
    else
        ret = music_vol_map[step] - (double)(vol%VOL_STEP)/VOL_STEP*(music_vol_map[step] - music_vol_map[step+1]);

    return (u16)ret;
}

phys_addr_t mtk_alsa_get_bank_base(unsigned int base)
{
	phys_addr_t reg = 0;
	int i;

	for (i = 0; i < priv->bank_num; i++) {
		if (base == priv->reg_bank[i]) {
			reg = priv->reg_base[i];
			break;
		}
	}

	if (reg == 0)
		debug("[SND SOC]%s get bank base fail\n", __func__);

	return reg;
}

bool mtk_alsa_read_reg_polling(unsigned int addr,
		unsigned short mask, unsigned short val, int counter)
{
	int i = 0;
	phys_addr_t reg;
	unsigned int base;
	unsigned short offset;

	base = addr & 0xFFFFFF00;
	offset = addr & 0xFF;
	offset = offset << 1;

	reg = mtk_alsa_get_bank_base(base);

	while (i < counter) {
		if ((readw(reg + offset) & mask) == val)
			return 0;
		i++;
	}
	return 1;
}

bool mtk_alsa_read_reg_byte_polling(unsigned int addr,
		unsigned char mask, unsigned char val, int counter)
{
	int i = 0;
	phys_addr_t reg;
	unsigned int base;
	unsigned short offset;

	base = addr & 0xFFFFFF00;
	offset = addr & 0xFF;
	offset = (offset << 1) - (offset & 1);

	reg = mtk_alsa_get_bank_base(base);

	while (i < counter) {
		if ((readb(reg + offset) & mask) == val)
			return 0;
		i++;
	}
	return 1;
}

unsigned char mtk_alsa_read_reg_byte(unsigned int addr)
{
	phys_addr_t reg;
	unsigned int base;
	unsigned short offset;

	base = addr & 0xFFFFFF00;
	offset = addr & 0xFF;
	offset = (offset << 1) - (offset & 1);

	reg = mtk_alsa_get_bank_base(base);

	return readb(reg + offset);
}

unsigned short mtk_alsa_read_reg(unsigned int addr)
{
	phys_addr_t reg;
	unsigned int base;
	unsigned short offset;

	base = addr & 0xFFFFFF00;
	offset = addr & 0xFF;
	offset = offset << 1;

	reg = mtk_alsa_get_bank_base(base);

	return readw(reg + offset);
}

void mtk_alsa_write_reg_mask_byte(unsigned int addr,
			unsigned char mask, unsigned char val)
{
	phys_addr_t reg;
	unsigned int base;
	unsigned short offset;

	base = addr & 0xFFFFFF00;
	offset = addr & 0xFF;
	offset = (offset << 1) - (offset & 1);

	reg = mtk_alsa_get_bank_base(base);
	if (reg == 0)
		return;
	ALSA_WRITE_MASK_BYTE(reg + offset, mask, val);
}

void mtk_alsa_write_reg_byte(unsigned int addr, unsigned char val)
{
	phys_addr_t reg;
	unsigned int base;
	unsigned short offset;

	base = addr & 0xFFFFFF00;
	offset = addr & 0xFF;
	offset = (offset << 1) - (offset & 1);

	reg = mtk_alsa_get_bank_base(base);

	writeb(val, reg + offset);
}

void mtk_alsa_write_reg_mask(unsigned int addr,
			unsigned short mask, unsigned short val)
{
	phys_addr_t reg;
	unsigned int base;
	unsigned short offset;

	base = addr & 0xFFFFFF00;
	offset = addr & 0xFF;
	offset = offset << 1;

	reg = mtk_alsa_get_bank_base(base);

	ALSA_WRITE_MASK_2BYTE(reg + offset, mask, val);
}

void mtk_alsa_write_reg(unsigned int addr, unsigned short val)
{
	phys_addr_t reg;
	unsigned int base;
	unsigned short offset;

	base = addr & 0xFFFFFF00;
	offset = addr & 0xFF;
	offset = offset << 1;

	reg = mtk_alsa_get_bank_base(base);

	writew(val, reg + offset);
}

static ofnode mtk_sound_get_mmap_ofnode(void)
{
	ofnode node;

	node = ofnode_path("/mmap_info/MI_MAD_ADV_BUF");

	return node;
}

char *mtk_sound_data_load_to_memory(char *part, const char *filename, loff_t *filesize)
{
	char device_name[STORAGE_DEVICE_BUF_SIZE] = {0};
	char storage_info[STORAGE_INDEX_BUF_SIZE] = {0};
	unsigned int device = 0;
	unsigned char *file_buf;
	loff_t read_len = 0, size = 0;
	int ret;
#ifdef CONFIG_MULTICORES_PLATFORM
	unsigned long irq_flag = 0;
#endif

	ret = sys_get_boot_device(device_name, sizeof(device_name));
	if (ret < 0) {
		UBOOT_ERROR("Error: Get booting device failure, Unknown storage.\n");
		return 0;
	}

	ret = sys_get_storage_info(device, part, storage_info);
	if (ret < 0) {
		UBOOT_ERROR("Error: sys_get_storage_info failure\n");
		return 0;
	}

#if defined(CONFIG_MULTICORES_PLATFORM)
	smp_spin_lock_save(&fs_spin_lock, irq_flag);
#endif
	if (fs_set_blk_dev(device_name, storage_info, FS_TYPE_EXT)) {
		UBOOT_ERROR("Error: partition select failure\n");
		goto READ_MUSIC_FILE_FAIL;
	}

	if (fs_size(filename, &size) < 0) {
		UBOOT_ERROR("Error: Get boot music file %s size failure\n",
			    filename);
		goto READ_MUSIC_FILE_FAIL;
	}

	file_buf = malloc((unsigned int)size);
	if (!file_buf) {
		UBOOT_ERROR("Error: file_buf allocate failure\n");
		goto READ_MUSIC_FILE_FAIL;
	}

	if (fs_set_blk_dev(device_name, storage_info, FS_TYPE_EXT)) {
		UBOOT_ERROR("Error: partition select failure\n");
		free(file_buf);
		goto READ_MUSIC_FILE_FAIL;
	}

	ret = fs_read(filename, (ulong)file_buf, 0, 0, &read_len);
	if (ret < 0) {
		UBOOT_ERROR("Error: Read boot logo file failure\n");
		free(file_buf);
		goto READ_MUSIC_FILE_FAIL;
	}
#if defined(CONFIG_MULTICORES_PLATFORM)
	smp_spin_unlock_restore(&fs_spin_lock, irq_flag);
#endif
	*filesize = size;

	return (char *)file_buf;

READ_MUSIC_FILE_FAIL:
#if defined(CONFIG_MULTICORES_PLATFORM)
	smp_spin_unlock_restore(&fs_spin_lock, irq_flag);
#endif
	return NULL;
}

void mtk_sound_load_mp3_to_dram(char *buf, loff_t size)
{
	phys_addr_t dsp_va;
	phys_addr_t MIU_addr;

	/* get dsp base */
	dsp_va = priv->bus_addr;

	MIU_addr = dsp_va + priv->music_offset;

	memcpy((void *)MIU_addr, (void *)buf, size);

	flush_cache((unsigned long)MIU_addr, size);
}

static int mtk_alsa_init(char *buf, int index)
{
	char *endp;
	char str1[] = "{0x";
	char str2[] = "0x";
	unsigned int u32Value = 0;
	u16 u16Value = 0;
	unsigned int u32Addr;
	unsigned char u8Mask;
	unsigned char u8Value;
	char *loc = NULL;
	int pos = 0;
	int ret;
	char temp_str[7] = {0};
	int audio_init_flag = 0;

	while (1) {
		loc = strstr(buf, str1);
		if (loc) {
			pos = loc - buf;
			pos += strlen(str1);

			strncpy(temp_str, (buf + pos), sizeof(int));
			temp_str[4] = '\0';
			u32Value = ustrtoul(temp_str, &endp, 16);
			strncpy(temp_str, (buf + pos + 4), sizeof(u16));
			temp_str[2] = '\0';
			u16Value = ustrtoul(temp_str, &endp, 16);
			u32Addr = (u32Value << 8) | (u16Value);
			buf += pos;
		} else
			break;

		loc = strstr(buf, str2);
		if (loc) {
			pos = loc - buf;
			pos += strlen(str2);

			strncpy(temp_str, (buf + pos), sizeof(u16));
			temp_str[2] = '\0';
			u16Value = ustrtoul(temp_str, &endp, 16);
			u8Mask = u16Value;
			buf += pos;
		} else
			break;

		loc = strstr(buf, str2);
		if (loc) {
			pos = loc - buf;
			pos += strlen(str2);

			strncpy(temp_str, (buf + pos), sizeof(u16));
			temp_str[2] = '\0';
			u16Value = ustrtoul(temp_str, &endp, 16);
			u8Value = u16Value;
			buf += pos;
		} else
			break;

		/* {0xffffff, 0x00, 0x00} means end of audio init table */
		if (u32Addr == 0xffffff && u8Mask == 0x0 && u8Value == 0x0) {
			audio_init_flag = 1;
			break;
		}

		if ((u32Addr == 0xFFFFFF) && (u8Mask == 1) && (u8Value > 0))
			mdelay(u8Value);
		else
			mtk_alsa_write_reg_mask_byte(u32Addr, u8Mask, u8Value);
	}

	if (audio_init_flag == 1) {
		if (index == 0)
			debug("[SND SOC]do pre-init done\n");
		else
			debug("[SND SOC]do init done\n");
		ret = 0;
	} else {
		if (index == 0)
			debug("[SND SOC]do pre-init fail\n");
		else
			debug("[SND SOC]do init fail\n");
		ret = -EIO;
	}

	return ret;
}

static u32 mtk_sound_get_dec_reg(void)
{
	ofnode node;
	ofnode subnode, subnode1;

	node = ofnode_path("/mediatek-sound-register");
	if (!ofnode_valid(node)) {
		debug("[SND SOC]get sound reg fail\n");
		return -EINVAL;
	}

	subnode = ofnode_find_subnode(node, "decoder-register");
	if (!ofnode_valid(subnode)) {
		debug("[SND SOC]get decoder reg fail\n");
		return -EINVAL;
	}

	if (ofnode_read_u32(subnode, "reg_dec_play_cmd", &REG.DEC_PLAY_CMD) < 0) {
		debug("[SND SOC]get reg_dec_play_cmd fail\n");
		return -EINVAL;
	}

	if (ofnode_read_u32(subnode, "reg_dec_es_wptr", &REG.DEC_ES_WPTR) < 0) {
		debug("[SND SOC]get reg_dec_es_wptr fail\n");
		return -EINVAL;
	}

	if (ofnode_read_u32(subnode, "reg_volume", &REG.VOLUME) < 0) {
		debug("[SND SOC]get reg_volume fail\n");
		return -EINVAL;
	}

	subnode1 = ofnode_find_subnode(node, "channel-mux-register");
	if (!ofnode_valid(subnode1)) {
		debug("[SND SOC]get channel mux reg fail\n");
		return -EINVAL;
	}

	if (ofnode_read_u32(subnode1, "reg_ch_6_cfg", &REG.CH_6_CFG) < 0) {
		debug("[SND SOC]get reg_ch_6_cfg fail\n");
		return -EINVAL;
	}

	return 0;
}

void mtk_headphone_detect(void)
{
	ofnode node;
	struct gpio_desc hp_detect = {};
	int ret;

	node = ofnode_path("/extcon-headset");
	if (!ofnode_valid(node)) {
		debug("[SND SOC]get extcon headset node fail\n");
		return;
	}

	gpio_request_by_name_nodev(node, "extcon-gpios", 0, &hp_detect, GPIOD_IS_IN);

	if (dm_gpio_is_valid(&hp_detect)) {
		ret = dm_gpio_get_value(&hp_detect);
		if (ret < 0) {
			debug("[SND SOC] fail to get hp gpio value\n");
			return;
		}

		if (ret) {
			/* output to headphone, mute speaker */
			if (priv->speaker_out == AUDIO_SPEAKER_I2S)
				mtk_alsa_write_reg_mask_byte(REG.VOLUME, 0x07, 0x1);
			else if (priv->speaker_out == AUDIO_SPEAKER_DAC0)
				mtk_alsa_write_reg_mask_byte(REG.VOLUME, 0x07, 0x2);
			else
				mtk_alsa_write_reg_mask_byte(REG.VOLUME, 0x07, 0x4);
		} else {
			/* output to speaker, mute headphone */
			if (priv->speaker_out == AUDIO_SPEAKER_I2S)
				mtk_alsa_write_reg_mask_byte(REG.VOLUME, 0x07, 0x6);
			else if (priv->speaker_out == AUDIO_SPEAKER_DAC0)
				mtk_alsa_write_reg_mask_byte(REG.VOLUME, 0x07, 0x5);
			else
				mtk_alsa_write_reg_mask_byte(REG.VOLUME, 0x07, 0x3);
		}
	}
	else
		debug("[SND SOC] fail to get hp gpio\n");

	return;
}
#ifdef CONFIG_DATA_SEPARATION
static void mtk_sound_get_bootmusic_info(void)
{
	int ret;
	char filepath[FILE_PATH_SIZE], part[PART_NAME_SIZE];
	const char *relpath;

	memset(priv->info.part, 0, sizeof(priv->info.part));
	memset(priv->info.filename, 0, sizeof(priv->info.filename));
	/* get music on from ini */
	memset(part, 0, sizeof(part));
	memset(filepath, 0, sizeof(filepath));
	if (dataindex_get_key(filepath, FILE_PATH_SIZE, MUSIC_SECTION, MUSIC_INI_KEY, NULL) == 0) {
		if (dataindex_resolve_path(part, PART_NAME_SIZE, &relpath, filepath) == 0) {
			ret = get_bootmusic_info(part, relpath, &priv->info);
			if (ret < 0)
				debug("Load ini failure with data separation\n");
		}
		else
			debug("resolve path fail: %s",filepath);
	}

	memset(filepath, 0, sizeof(filepath));
	if (dataindex_get_key(filepath, FILE_PATH_SIZE, MUSIC_SECTION, MUSIC_FILE_KEY, NULL) == 0) {
		strncat(filepath, priv->info.filename, strlen(priv->info.filename));
		if (dataindex_resolve_path(priv->info.part, PART_NAME_SIZE, &relpath, filepath) != 0)
			debug("resolve path fail: %s",filepath);
		else
			strlcpy(priv->info.filename, relpath, sizeof(priv->info.filename));
	}
}
#endif
static int mtk_sound_play(struct udevice *dev, void *data, uint data_size)
{
	char *buf;
	loff_t size;
	u16 vol = 0;

	if (priv->info.enable == 0) {
		debug("[SND SOC]music off from ini, skip boot music !!!!\n");
		return 1;
	}

	/* dsp decoder 5 isr -> ch 6 */
	/* ch6_256fsi_mux1_int_ck [2:0] = 1 */
	/* ch6_256fsi_mux_int_ck [3] = 1 */
	writew(0x0C, 0x1C205060);

	//load mp3 to dram
#ifdef CONFIG_HAPS
	buf = mtk_sound_data_load_to_memory(DEFAULT_PART, DEFAULT_NAME, &size);
#else
	buf = mtk_sound_data_load_to_memory(priv->info.part, priv->info.filename, &size);
#endif
	if (buf == 0) {
		dev_err(dev, "[SND SOC]load mp3 fail\n");
		return -EINVAL;
	}

	mtk_sound_load_mp3_to_dram(buf, size);
	free(buf);

	mtk_headphone_detect();

	vol = mtk_volume_table(priv->info.volume);

	if (vol == music_vol_map[0])
		debug("bootmusic vol = 0, sound may mute!!!\n");
	mtk_alsa_write_reg_mask(REG.VOLUME, 0xFFF8, vol);

	dev_info(dev, "[SND SOC]do play mp3\n");
	mtk_alsa_write_reg(REG.DEC_ES_WPTR, (priv->music_offset >> BYTES_IN_MIU_LINE_LOG2));
	mtk_alsa_write_reg_byte(REG.CH_6_CFG, 0xC);
	mtk_alsa_write_reg(REG.DEC_PLAY_CMD, 0x2B);

	if (size >= ES_MAX_SIZE)
		size = ES_MAX_SIZE;

	mtk_alsa_write_reg(REG.DEC_ES_WPTR,
			((priv->music_offset + size) >> BYTES_IN_MIU_LINE_LOG2));

	return 0;
}

static int mtk_sound_setup(struct udevice *dev)
{
	char *buf = NULL;
	loff_t size;
	int ret;

	if (priv->info.enable == 0) {
		debug("[SND SOC]music off from ini, skip boot music !!!!\n");
		return 1;
	}

	dev_info(dev, "[SND SOC]do init audio\n");

	buf = mtk_sound_data_load_to_memory(DEFAULT_PART, priv->pre_init_name, &size);
	if (buf == 0) {
		dev_err(dev, "[SND SOC]load pre init table fail\n");
		return -EINVAL;
	}

	ret = mtk_alsa_init(buf, 0);
	if (ret) {
		free(buf);
		return ret;
	}

	free(buf);

	buf = mtk_sound_data_load_to_memory(DEFAULT_PART, priv->init_name, &size);
	if (buf == 0) {
		dev_err(dev, "[SND SOC]load init table fail\n");
		return -EINVAL;
	}

	ret = mtk_alsa_init(buf, 1);
	if (ret) {
		free(buf);
		return ret;
	}

	free(buf);

	/* stop play */
	mtk_alsa_write_reg(REG.DEC_PLAY_CMD, 0x0);

	return 0;
}

static int mtk_sound_probe(struct udevice *dev)
{
	int clk_reg_count;
	u32 *clk_reg_setting;
	u16 reg_val;
	phys_addr_t bank_base;
	ofnode node;
	u32 addr[4];
	int i;
	int ret;

	priv = calloc(1, sizeof(struct mtk_sound_priv));
	if (!priv)
		return -ENOMEM;
	/* parse dts */
	node = mtk_sound_get_mmap_ofnode();
	if (!ofnode_valid(node)) {
		dev_err(dev, "[SND SOC]not find mmap node\n");
		return -EINVAL;
	}

	if (ofnode_read_u32_array(node, "reg", addr, 4)) {
		dev_err(dev, "[SND SOC]not find address\n");
		return -EINVAL;
	}
	priv->bus_addr = addr[1];
	priv->buffer_size = addr[3];

	ret = dev_read_u32(dev, "riu_base", &priv->riu_base);
	if (ret) {
		dev_err(dev, "[SND SOC]can't get riu base\n");
		return -EINVAL;
	}

	ret = dev_read_u32(dev, "audio_bank_num", &priv->bank_num);
	if (ret) {
		dev_err(dev, "[SND SOC]can't get bank num\n");
		return -EINVAL;
	}

	/* do audio clock init */
	/* mux clock */
	/* sw en clock */
	/* enable audio clock */
	clk_reg_count = dev_read_size(dev, "module-clocks");
	if (clk_reg_count > 0) {
		clk_reg_setting = malloc(clk_reg_count);
		if (!clk_reg_setting)
			return -ENOMEM;

		/* get property "module-clocks" data */
		clk_reg_count /= sizeof(u32);
		ret = dev_read_u32_array(dev, "module-clocks", clk_reg_setting,
					 clk_reg_count);
		if (ret) {
			free(clk_reg_setting);
			return ret;
		}

		/* traverse all "module-clocks" setting */
		/* "module-clocks" format: addr, mask, value */
		clk_reg_count /= 3;
		for (i = 0 ; i < clk_reg_count ; i++) {
			reg_val = readw((u64)(clk_reg_setting[i*3]));
			reg_val &= ~clk_reg_setting[i*3+1];
			reg_val |= clk_reg_setting[i*3+2];
			writew(reg_val, (u64)(clk_reg_setting[i*3]));
		}
		free(clk_reg_setting);
	}

	priv->reg_bank = calloc(priv->bank_num, sizeof(unsigned int));
	if (!priv->reg_bank)
		return -ENOMEM;

	priv->reg_base = calloc(priv->bank_num, sizeof(phys_addr_t));
	if (!priv->reg_base)
		return -ENOMEM;

	for (i = 0; i < priv->bank_num; i++) {
		priv->reg_base[i] = dev_read_addr_index(dev, i);
		if (priv->reg_base[i] == FDT_ADDR_T_NONE) {
			dev_err(dev, "[SND SOC]Cannot read address index %d\n", i);
			return -EINVAL;
		}
		bank_base = dev_read_addr_index(dev, i);
		priv->reg_bank[i] = (bank_base - priv->riu_base) >> 1;
	}

	priv->pre_init_name = dev_read_string(dev, "pre_init_name");
	if (!priv->pre_init_name) {
		dev_err(dev, "[SND SOC]can't get audio pre init name\n");
		return -EINVAL;
	}

	priv->init_name = dev_read_string(dev, "init_name");
	if (!priv->init_name) {
		dev_err(dev, "[SND SOC]can't get audio pre init name\n");
		return -EINVAL;
	}

	ret = dev_read_u32(dev, "music_offset", &priv->music_offset);
	if (ret) {
		dev_err(dev, "[SND SOC]can't get music offset\n");
		return -EINVAL;
	}

	ret = dev_read_u32(dev, "speaker_out", &priv->speaker_out);
	if (ret) {
		dev_err(dev, "[SND SOC]can't get speaker out\n");
		return -EINVAL;
	}

	ret = mtk_sound_get_dec_reg();
	if (ret) {
		dev_err(dev, "[SND SOC]can't get dec register\n");
		return -EINVAL;
	}

	strlcpy(priv->info.part, DEFAULT_PART, strlen(DEFAULT_PART) + 1);
	strlcpy(priv->info.filename, DEFAULT_NAME, strlen(DEFAULT_NAME) + 1);
	priv->info.volume = DEFAULT_VOL;
	priv->info.enable = 1;
#ifdef CONFIG_DATA_SEPARATION
	mtk_sound_get_bootmusic_info();
#endif
	return 0;
}

static const struct sound_ops mtk_sound_ops = {
	.setup	= mtk_sound_setup,
	.play	= mtk_sound_play,
};

static const struct udevice_id mtk_sound_ids[] = {
	{ .compatible = "mediatek,mtk-soc" },
	{ }
};

U_BOOT_DRIVER(mediatek_sound) = {
	.name		= "mediatek_sound",
	.id		= UCLASS_SOUND,
	.of_match	= mtk_sound_ids,
	.probe		= mtk_sound_probe,
	.ops		= &mtk_sound_ops,
};
