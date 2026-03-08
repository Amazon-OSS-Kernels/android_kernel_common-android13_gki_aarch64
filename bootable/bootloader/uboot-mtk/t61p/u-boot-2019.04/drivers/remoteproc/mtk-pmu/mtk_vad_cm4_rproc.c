// SPDX-License-Identifier: GPL-2.0+
/*
 * Texas Instruments' K3 System Controller Driver
 *
 * Copyright (C) 2017-2018 Texas Instruments Incorporated - http://www.ti.com/
 *    Lokesh Vutla <lokeshvutla@ti.com>
 */

#include <common.h>
#include <dm.h>
#include <remoteproc.h>
#include <errno.h>
#include <mailbox.h>
#include "mdrv_pmu.h"
#include "platform.h"
#include "drvBDMA.h"
#include "halCM4.h"
#include <elf.h>
#include <fdt.h>
#include <linux/libfdt.h>
#include <linux/io.h>
#include <linux/ioport.h>

#define VAD_ADDR_NUM                    (4)
#define VAD_ADDR_SHIFT                  (32)
#define VAD_ADDR_IDX0                   (0)
#define VAD_ADDR_IDX1                   (1)
#define VAD_ADDR_IDX2                   (2)
#define VAD_ADDR_IDX3                   (3)
#define MASK_0XFFFFFFFF                 (0xFFFFFFFF)
#define MASK_0XF000                     (0xF000)
#define MASK_0X3                        (0x3)

#define PMU_OFS_00_15                   (((0x38 << 1) * 2) + 0)
#define PMU_OFS_16_31                   (((0x39 << 1) * 2) + 0)
#define PMU_OFS_32_33                   (((0x38 << 1) * 2) + 0)
#define ADDR_00_15                      (0)
#define ADDR_16_31                      (16)
#define ADDR_32_33                      (32)

/**
 * struct mtk_vad_privdata - Structure representing System Controller data.
 * @chan_tx:    Transmit mailbox channel
 * @chan_rx:    Receive mailbox channel
 * @desc:    SoC description for this instance
 * @seq_nr:    Counter for number of messages sent.
 */
struct mtk_vad_privdata {
	phys_addr_t cpu_base;
	phys_addr_t fw_addr;
	phys_size_t fw_size;
	struct resource shm;
	mtk_pmu_info reg;
};

/**
 * mtk_vad_init() - Initialize the remote processor
 * @dev:    rproc device pointer
 *
 * Return: 0 if all went ok, else return appropriate error
 */
static int mtk_vad_init(struct udevice *dev)
{
	mtk_pmu_info info;
	struct mtk_vad_privdata *priv = dev_get_priv(dev);
	/* Enable the module */

	memcpy(&info, &priv->reg, sizeof(mtk_pmu_info));
	mdrv_pmu_init_info(&info);

	mdrv_pmu_stop();
	mdrv_pm_imi_init(&info);
	mdrv_pmu_init(); // TODO: remove calling utopia by inline this operation
	mdrv_pmu_halt();
	PMU_log(PMU_DEBUG, "%s: rproc successfully initialized\n", __func__);
	return 0;
}

/**
 * mtk_vad_load() - Loadup the cm4 remote processor
 * @dev:    corresponding cm4 remote processor device
 * @addr:    Address in memory where image binary is stored
 * @size:    Size in bytes of the image binary
 *
 * Return: 0 if all goes good, else appropriate error message.
 */
static int mtk_vad_load(struct udevice *dev, ulong addr, ulong size)
{
    struct mtk_vad_privdata *priv = dev_get_priv(dev);
    int ret = 0, i = 0;
    Elf32_Word total_size = 0;
    Elf32_Ehdr *ehdr = (Elf32_Ehdr *)addr;
    Elf32_Phdr *phdr = (Elf32_Phdr *)(addr + ehdr->e_phoff);

	PMU_log(PMU_DEBUG, "%s: Parsing ELF from 0x%016lX to 0x%016llX, size 0x%016lX\n",
		__func__, addr, (unsigned long long)(priv->fw_addr), size);

    // Verify valid ELF format
    ret = rproc_elf32_sanity_check(addr, size);
    if (ret != 0)
    {
        PMU_log(PMU_ERROR, "rproc_elf32_sanity_check fail addr:%016lX size:%016lX bytes\n", addr,size);
        return ret;
    }

    // Parsing ELF to phdr->p_paddr
    ret = rproc_elf32_load_image(dev, addr);
    if (ret != 0)
    {
        PMU_log(PMU_ERROR, "rproc_elf32_load_image fail addr:%016lX\n", addr);
        return ret;
    }

    /* Accumulate the size of each program  */
    for (i = 0; i < ehdr->e_phnum; ++i) {
        if (phdr->p_type != PT_LOAD)
            continue;
        total_size += phdr->p_memsz;
        // memory alignment
        if ((phdr->p_memsz % phdr->p_align) != 0)
            total_size += (phdr->p_align - (phdr->p_memsz % phdr->p_align));
        ++phdr;
    }

	if (total_size > priv->fw_size) {
		PMU_log(PMU_ERROR, "Overflow, total_size(%x) > priv->fwSize(%016llX)\n",
			total_size, (unsigned long long)(priv->fw_size));
		return -1;
	}

	//Copy to SRAM
	PMU_log(PMU_DEBUG, "Copy 0x%016llX to SRAM, size:0x%x\n",
		(unsigned long long)(priv->fw_addr), total_size);

	mdrv_bdma_copy(priv->fw_addr, 0x0, total_size, E_BDMA_HK_TO_CP);

    PMU_log(PMU_DEBUG,"%s: Firmware Loaded successfully on dev %s\n",
          __func__, dev->name);

    return ret;
}

/**
 * mtk_vad_start() - Start the remote processor
 *        Note that while technically the cm4 system controller starts up
 *        automatically after its firmware got loaded we still want to
 *        utilize the rproc start operation for other startup-related
 *        tasks.
 * @dev:    device to operate upon
 *
 * Return: 0 if all went ok, else return appropriate error
 */
static int mtk_vad_start(struct udevice *dev)
{
	int ret = 0;
	mtk_pmu_info info;
	struct mtk_vad_privdata *priv = dev_get_priv(dev);

	memcpy(&info, &priv->reg, sizeof(mtk_pmu_info));
	OUTREG16(((info.coin_cpu) | PMU_OFS_00_15),
		 (((priv->shm.start - priv->cpu_base) >> ADDR_00_15) & MASK_0XF000) |
		 ((priv->shm.start >> ADDR_32_33) & MASK_0X3));
	OUTREG16(((info.coin_cpu) | PMU_OFS_16_31),
		 ((priv->shm.start - priv->cpu_base) >> ADDR_16_31));

	mdrv_pmu_enable_uart(1);
	mhal_cm4_run();

	PMU_log(PMU_DEBUG, "%s: Boot notification received successfully on dev %s\n",
		__func__, dev->name);

	return ret;
}

/**
 * mtk_vad_stop() - Stop the remote processor
 *        Note that while technically the cm4 system controller starts up
 *        automatically after its firmware got loaded we still want to
 *        utilize the rproc start operation for other startup-related
 *        tasks.
 * @dev:    device to operate upon
 *
 * Return: 0 if all went ok, else return appropriate error
 */
static int mtk_vad_stop(struct udevice *dev)
{
	int ret = 0;
	mtk_pmu_info info;
	struct mtk_vad_privdata *priv = dev_get_priv(dev);

	memcpy(&info, &priv->reg, sizeof(mtk_pmu_info));

	mdrv_pmu_stop();
	mdrv_pm_imi_sleep(&info, E_PMIMI_MAX);

	PMU_log(PMU_DEBUG, "%s: Stop notification received successfully on dev %s\n",
		__func__, dev->name);

	return ret;
}

void * mtk_vad_device_to_virt(struct udevice *dev, ulong da)
{
	struct mtk_vad_privdata *priv = dev_get_priv(dev);
	ulong paddr = 0;

	/* Use a simple offset conversion */
	paddr = da + priv->fw_addr + priv->cpu_base;
	PMU_log(PMU_DEBUG, "%s:%s convert address 0x%016lX-> 0x%016lX\n",
		__func__, dev->name, da, paddr);

	return (void *)paddr;
}

/**
 * Check the remote processor status
 * Return: 0 if running, 1 if not running
 *         ,othres for error-handling by caller.
 */
static int mtk_vad_is_running(struct udevice *dev)
{
    return 0;
}

static const struct dm_rproc_ops mtk_vad_ops = {
	.init = mtk_vad_init,
	.load = mtk_vad_load,
	.start = mtk_vad_start,
	.stop = mtk_vad_stop,
	.device_to_virt = mtk_vad_device_to_virt,
	.is_running = mtk_vad_is_running,
};

/**
 * mtk_of_to_priv() - generate private data from device tree
 * @dev:    corresponding mtk remote processor device
 * @priv:    pointer to driver specific private data
 *
 * Return: 0 if all goes good, else appropriate error message.
 */
static int mtk_of_to_priv(struct udevice *dev,
             struct mtk_vad_privdata *priv)
{
	u32 base = 0;
	u32 addr[VAD_ADDR_NUM] = {0};
	ofnode node;

	/* Get host cpu base. */
	node = ofnode_path("/memory_info");
	if ((!ofnode_valid(node)) ||
	    (ofnode_read_u32(node, "cpu_emi0_base", &base) < 0)) {
		PMU_log(PMU_ERROR, "[%s] node(/memory_info/cpu_emi0_base) not found.\n",
			__func__);
		return -EIO;
	}
	priv->cpu_base = (phys_addr_t)base & MASK_0XFFFFFFFF;

	/* Get mmap layout. */
	node = ofnode_path("/mmap_info/MI_CM4_VOICE");
	if ((!ofnode_valid(node)) ||
	    (ofnode_read_u32_array(node, "reg", addr, VAD_ADDR_NUM) < 0)) {
		PMU_log(PMU_ERROR, "[%s] node(/memory_info/MI_CM4_VOICE/reg) not found.\n",
			__func__);
		return -EIO;
	}

	priv->fw_addr = (((u64)addr[VAD_ADDR_IDX0] << VAD_ADDR_SHIFT) | addr[VAD_ADDR_IDX1])
			- priv->cpu_base;
	priv->fw_size = (((u64)addr[VAD_ADDR_IDX2] << VAD_ADDR_SHIFT) | addr[VAD_ADDR_IDX3]);

	/* Get dummy address and write share memory address. */
	if (ofnode_read_resource(node, 0, &priv->shm) < 0) {
		PMU_log(PMU_ERROR, "ofnode_read_resource() not found.\n");
		return -ENOENT;
	}

	/* Get device register. */
	priv->reg.paganini = dev_read_addr_name(dev, "paganini");
	priv->reg.pm_sleep = dev_read_addr_name(dev, "pm_sleep");
	priv->reg.pm_top = dev_read_addr_name(dev, "pm_top");
	priv->reg.pm_misc = dev_read_addr_name(dev, "pm_misc");
	priv->reg.cpu_int = dev_read_addr_name(dev, "cpu_int");
	priv->reg.inturrpt = dev_read_addr_name(dev, "inturrpt");
	priv->reg.viv_bdma = dev_read_addr_name(dev, "viv_bdma");
	priv->reg.MALIBOX = dev_read_addr_name(dev, "MALIBOX");
	priv->reg.vivaldi4 = dev_read_addr_name(dev, "vivaldi4");
	priv->reg.pm_imi = dev_read_addr_name(dev, "pm_imi");
	priv->reg.vivaldi2 = dev_read_addr_name(dev, "vivaldi2");
	priv->reg.ckgen00_pm = dev_read_addr_name(dev, "ckgen00_pm");
	priv->reg.ckgen01_pm = dev_read_addr_name(dev, "ckgen01_pm");
	priv->reg.vad_0 = dev_read_addr_name(dev, "vad_0");
	priv->reg.vrec = dev_read_addr_name(dev, "vrec");
	priv->reg.coin_cpu = dev_read_addr_name(dev, "coin_cpu");
	priv->reg.vad_1 = dev_read_addr_name(dev, "vad_1");
	priv->reg.efuse_0 = dev_read_addr_name(dev, "efuse_0");

	PMU_log(PMU_INFO, "%s(priv->cpu_base=0x%016llX)\n", __func__, (unsigned long long)(priv->cpu_base));
	PMU_log(PMU_INFO, "%s(priv->fw_addr=0x%016llX)\n", __func__, (unsigned long long)(priv->fw_addr));
	PMU_log(PMU_INFO, "%s(priv->fw_size=0x%016llX)\n", __func__, (unsigned long long)(priv->fw_size));
	PMU_log(PMU_INFO, "%s(priv->shmAddr=0x%016tX)\n", __func__, (size_t)(priv->shm.start));
	PMU_log(PMU_INFO, "%s(priv->shmSize=0x%016tX)\n", __func__, (size_t)(priv->shm.end - priv->shm.start));
	PMU_log(PMU_INFO, "%s(priv->reg.paganini=0x%016llX)\n", __func__, (unsigned long long)(priv->reg.paganini));
	PMU_log(PMU_INFO, "%s(priv->reg.pm_sleep=0x%016llX)\n", __func__, (unsigned long long)(priv->reg.pm_sleep));
	PMU_log(PMU_INFO, "%s(priv->reg.pm_top=0x%016llX)\n", __func__, (unsigned long long)(priv->reg.pm_top));
	PMU_log(PMU_INFO, "%s(priv->reg.pm_misc=0x%016llX)\n", __func__, (unsigned long long)(priv->reg.pm_misc));
	PMU_log(PMU_INFO, "%s(priv->reg.cpu_int=0x%016llX)\n", __func__, (unsigned long long)(priv->reg.cpu_int));
	PMU_log(PMU_INFO, "%s(priv->reg.inturrpt=0x%016llX)\n", __func__, (unsigned long long)(priv->reg.inturrpt));
	PMU_log(PMU_INFO, "%s(priv->reg.viv_bdma=0x%016llX)\n", __func__, (unsigned long long)(priv->reg.viv_bdma));
	PMU_log(PMU_INFO, "%s(priv->reg.MALIBOX=0x%016llX)\n", __func__, (unsigned long long)(priv->reg.MALIBOX));
	PMU_log(PMU_INFO, "%s(priv->reg.vivaldi4=0x%016llX)\n", __func__, (unsigned long long)(priv->reg.vivaldi4));
	PMU_log(PMU_INFO, "%s(priv->reg.pm_imi=10x%016llX)\n", __func__, (unsigned long long)(priv->reg.pm_imi));
	PMU_log(PMU_INFO, "%s(priv->reg.vivaldi2=0x%016llX)\n", __func__, (unsigned long long)(priv->reg.vivaldi2));
	PMU_log(PMU_INFO, "%s(priv->reg.ckgen00_pm=0x%016llX)\n", __func__, (unsigned long long)(priv->reg.ckgen00_pm));
	PMU_log(PMU_INFO, "%s(priv->reg.ckgen01_pm=0x%016llX)\n", __func__, (unsigned long long)(priv->reg.ckgen01_pm));
	PMU_log(PMU_INFO, "%s(priv->reg.vad_0=0x%016llX)\n", __func__, (unsigned long long)(priv->reg.vad_0));
	PMU_log(PMU_INFO, "%s(priv->reg.vrec=0x%016llX)\n", __func__, (unsigned long long)(priv->reg.vrec));
	PMU_log(PMU_INFO, "%s(priv->reg.coin_cpu=0x%016llX)\n", __func__, (unsigned long long)(priv->reg.coin_cpu));
	PMU_log(PMU_INFO, "%s(priv->reg.vad_1=0x%016llX)\n", __func__, (unsigned long long)(priv->reg.vad_1));
	PMU_log(PMU_INFO, "%s(priv->reg.efuse_0=0x%016llX)\n", __func__, (unsigned long long)(priv->reg.efuse_0));
	return 0;
}

/**
 * mtk_vad_probe() - Basic probe
 * @dev:    corresponding mtk remote processor device
 *
 * Return: 0 if all goes good, else appropriate error message.
 */
static int mtk_vad_probe(struct udevice *dev)
{
    struct mtk_vad_privdata *priv = NULL;
    int ret = 0;

    PMU_log(PMU_DEBUG,"%s(dev=%p)\n", __func__, dev);

    priv = dev_get_priv(dev);

    ret = mtk_of_to_priv(dev, priv);
    if (ret) {
        dev_err(dev, "[%s] Probe failed with errno=%d.\n", __func__, ret);
        return ret;
    }

    return 0;
}

static const struct udevice_id mtk_vad_ids[] = {
    {.compatible = "mediatek,merak-vad-cm4-rproc"},
    {}
};

U_BOOT_DRIVER(mtk_vad) = {
    .name = "mtk_vad_cm4_rproc",
    .of_match = mtk_vad_ids,
    .id = UCLASS_REMOTEPROC,
    .ops = &mtk_vad_ops,
    .probe = mtk_vad_probe,
    .priv_auto_alloc_size = sizeof(struct mtk_vad_privdata),
};
