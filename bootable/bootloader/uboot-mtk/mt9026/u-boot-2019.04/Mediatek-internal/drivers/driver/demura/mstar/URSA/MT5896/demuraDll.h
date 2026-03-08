// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#ifndef _DEMURA_DLL_H_
#define _DEMURA_DLL_H_

MS_U32 StrToHex(const char *psHex, int startIndex);
MS_BOOL mstar_demura_interface (interface_info *Info1,  BinOutputInfo *bin_info);

int get_demura_bin_size(void);

void demura_set_version(int version);
void demura_set_panelinfo(MS_U16 width, MS_U16 height);
void demura_get_panelinfo(MS_U16 *width, MS_U16 *height);
int demura_get_h_nodes(int panel_width, int block_width_shift, int plane_num, int mode);
#endif
