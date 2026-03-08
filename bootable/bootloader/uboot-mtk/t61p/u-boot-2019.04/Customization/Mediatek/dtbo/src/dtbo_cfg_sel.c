// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
*/

#include <dtbo_cfg_sel.h>
#include <debug_impl.h>
#include <system_impl.h>
#include <linux/io.h>

#define BOND_SW_BANK					(0x100a)
#define BOND_SW_REG						(0x00)
#define BOND_SW_MASK					(0x000c)
#define BOND_SW_2G						(0x0004)
#define BOND_SW_3G						(0x000c)
#define BOND_SW_CUSTOM_MASK				(0x0180)
#define BOND_SW_CUSTOM_0				(0x0000)
#define BOND_SW_CUSTOM_2				(0x0100)

#define DTBOCFG_REG(bank, reg)			(void*)(uintptr_t)(RIU_BUS_ADDR + (((bank)<<9) + ((reg)<<2)))

#define CHIPID_5896						(5896)
#define CHIPID_5897						(5897)
#define CHIPID_5876						(5876)
#define REV_1							(1)
#define REV_2							(2)
#define REV_3							(3)

bool dtbo_cfg_sel_get_pcb(char *cfg_path, int size, u32 chipid, u32 revision)
{
	const char *pcb = "h1v1";
	int len;
	// flag to add revision tag to dtbo_cfg_sel
	bool flag_tag_revision = (revision > REV_1);

	UBOOT_TRACE("IN\n");
	switch (chipid) {
		case CHIPID_5896:
#if defined(CONFIG_ANDROID_CN_PLATFORM)
			if (revision == REV_3)
				pcb = "h1v4";
#else
			if (revision == REV_1)
				pcb = "h2v1";
			else if (revision == REV_3)
				pcb = "h2v3";
#endif
			break;
		case CHIPID_5897:
			pcb = "h1v1";
			break;
		case CHIPID_5876:
			// always disable revision tag for the CHIPID
			flag_tag_revision = false;
			if (revision == REV_1) {
				/* check bonding ? */
				u16 regval = ioread16(DTBOCFG_REG(BOND_SW_BANK, BOND_SW_REG));
				u16 regbond = regval & BOND_SW_MASK;
				if (regbond == BOND_SW_2G) {
					u16 regcustom = regval & BOND_SW_CUSTOM_MASK;
					if (regcustom == BOND_SW_CUSTOM_0)
						pcb = "h1v1";
					else if (regcustom == BOND_SW_CUSTOM_2)
						pcb = "h3v1";
					else {
						UBOOT_ERROR("cannot match BOND_SW reg!\n");
						return false;
					}
				}
				else if (regbond == BOND_SW_3G)
					pcb = "h2v1";
			}
			break;
		default:
			pcb = "h1v1";
			break;
	}

	if (flag_tag_revision) {
		len = snprintf(cfg_path, size,
				DTBOCFG_ROOT_PATH "/mt%d_e%d_%s.ini", chipid, revision, pcb);
	} else {
		len = snprintf(cfg_path, size,
				DTBOCFG_ROOT_PATH "/mt%d_%s.ini", chipid, pcb);
	}

	if (len >= size - 1) {
		UBOOT_ERROR("The size of default_dtbo_cfg buffer is too small!\n");
		return false;
	}

	UBOOT_INFO("auto select %s because dtbo_cfg_sel=default\n", cfg_path);
	UBOOT_TRACE("OUT\n");
	return true;
}
