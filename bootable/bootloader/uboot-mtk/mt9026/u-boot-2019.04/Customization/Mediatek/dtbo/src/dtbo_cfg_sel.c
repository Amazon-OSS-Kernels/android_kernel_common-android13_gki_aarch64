// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
*/

#include <dtbo_cfg_sel.h>
#include <debug_impl.h>
#include <system_impl.h>
#include <linux/io.h>

#define BOND_SW_BANK					(0x100a)
#define BOND_SW_REG                                     (0x00)
#define BOND_SW_MASK					(0x000c)
#define BOND_SW_5873_1G                                 (0x0000)
#define BOND_SW_5873_1p5G                               (0x0004)
#define BOND_SW_5873_2G                                 (0x0008)
#define BOND_SW_2G                                      (0x0004)
#define BOND_SW_3G                                      (0x000c)
#define BOND_SW_CUSTOM_MASK				(0x0180)
#define BOND_SW_CUSTOM_0				(0x0000)
#define BOND_SW_CUSTOM_2				(0x0100)
//Define for PKGHEAT
#define BOND_SW_PKGHEAT_MASK                            (0x0010)
#define BOND_SW_PKGHEAT_NTC                             (0x0000)
#define BOND_SW_PKGHEAT_HTC                             (0x0010)

//Define for PKGTYPE
#define BOND_SW_PKGTYPE_MASK                            (0x0040)
#define BOND_SW_PKGTYPE_MCP                             (0x0000)
#define BOND_SW_PKGTYPE_EXT                             (0x0040)

//Define for PKGSIZE
#define BOND_SW_PKGSIZE_MASK                            (0x0080)
#define BOND_SW_PKGSIZE_SMALL                           (0x0000)
#define BOND_SW_PKGSIZE_BIG                             (0x0080)

//Define for CID check
#define CID_SW_BANK                                     (0x203)
#define CID_SW_REG                                      (0x03)
#define CID_SW_MASK                                     (0x00ff)
#define CID_SW_CUS1                                     (0x1)

#define DTBOCFG_REG(bank, reg)			(void*)(uintptr_t)(RIU_BUS_ADDR + (((bank)<<9) + ((reg)<<2)))

#define CHIPID_5896						(5896)
#define CHIPID_5897						(5897)
#define CHIPID_5876						(5876)
#define CHIPID_5873                                             (5873)
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
                case CHIPID_5873:
                        // always disable revision tag for the CHIPID
                        flag_tag_revision = false;
                        if (revision == REV_1)
                        {
                                /* check bonding ? */
                                u16 regval = ioread16(DTBOCFG_REG(BOND_SW_BANK, BOND_SW_REG));
                                u16 regbond = regval & BOND_SW_MASK;
                                //u16 regpkgheat = regval & BOND_SW_PKGHEAT_MASK;
                                u16 regpkgtype = regval & BOND_SW_PKGTYPE_MASK;
                                u16 regpkgsize = regval & BOND_SW_PKGSIZE_MASK;
                                u16 regval_cid = ioread16(DTBOCFG_REG(CID_SW_BANK, CID_SW_REG));
                                u16 reg_cid = regval_cid & CID_SW_MASK;
                                if (regbond == BOND_SW_5873_1G)
                                {
                                        if ( (regpkgtype == BOND_SW_PKGTYPE_MCP) && (regpkgsize == BOND_SW_PKGSIZE_SMALL) )
                                        {
                                                pcb = "h2v1";
                                        }
                                        else if ( (regpkgtype == BOND_SW_PKGTYPE_MCP) && (regpkgsize == BOND_SW_PKGSIZE_BIG) )
                                        {
                                                pcb = "h1v1";
                                        }
                                        else if ( (regpkgtype == BOND_SW_PKGTYPE_EXT) )
                                        {
                                                pcb = "h3v1";
                                        }
                                        else
                                        {
                                                UBOOT_ERROR("cannot match BOND_SW reg!\n");
                                                return false;
                                        }
                                }
                                else if (regbond == BOND_SW_5873_1p5G)
                                {
                                        if ( (regpkgtype == BOND_SW_PKGTYPE_MCP) )
                                        {
                                                pcb = "h1v1";
                                        }
                                        else
                                        {
                                                pcb = "h3v1";
                                        }

                                        if ( reg_cid == CID_SW_CUS1 )
                                        {
                                                pcb = "h4v1";
                                        }
                                }
                                else if (regbond == BOND_SW_5873_2G)
                                {
                                        if ( (regpkgtype == BOND_SW_PKGTYPE_MCP) )
                                        {
                                                pcb = "h1v1";
                                        }
                                        else
                                        {
                                                pcb = "h3v1";
                                        }

                                        if ( reg_cid == CID_SW_CUS1 )
                                        {
                                                pcb = "h4v1";
                                        }
                                }
                                else if (regpkgtype == BOND_SW_PKGTYPE_EXT)
                                {
                                        pcb = "h3v1";
                                }
                                else
                                {
                                        UBOOT_ERROR("cannot match BOND_SW reg!\n");
                                        return false;
                                }
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
