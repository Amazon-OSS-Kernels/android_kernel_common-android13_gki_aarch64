/* SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause) */
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#ifndef _OAD_UPGRADE_H_
#define _OAD_UPGRADE_H_
#include <common.h>
#include <iniutility.h>
#ifdef CONFIG_DATA_SEPARATION
#define UPGRADE_SECTION     "Upgrade"
#define UPGRADE_KEY         "m_pUpgradeMode_File"
#endif

//--------------------------------------------------------------------------------------------------
/// oad_upgrade
/// 
/// 
///
/// @return : 0 = Success , -1 = Failure
//-------------------------------------------------------------------------------------------------
int oad_upgrade(char *upgrade_file);
int do_check_upgrade_mode(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);
int write_upgrade_complete(char *device_name, struct upgrade_info *upgradeinfo, int complete_flag);

#endif

