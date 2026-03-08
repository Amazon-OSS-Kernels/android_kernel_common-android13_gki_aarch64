/* SPDX-License-Identifier: GPL-2.0-only OR BSD-3-Clause */
/******************************************************************************
 *
 * This file is provided under a dual license.  When you use or
 * distribute this software, you may choose to be licensed under
 * version 2 of the GNU General Public License ("GPLv2 License")
 * or BSD License.
 *
 * GPLv2 License
 *
 * Copyright(C) 2019 MediaTek Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See http://www.gnu.org/licenses/gpl-2.0.html for more details.
 *
 * BSD LICENSE
 *
 * Copyright(C) 2019 MediaTek Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *  * Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *****************************************************************************/

#ifndef _API_USB_UPGRADE_OSD_H_
#define _API_USB_UPGRADE_OSD_H_


typedef int (*show_load_data_cb)(unsigned int);
typedef int (*show_start_upgrading_cb)(unsigned int);
typedef int (*show_finish_cb)(unsigned int);
typedef int (*show_error_cb)(unsigned int);
typedef int (*show_upgrading_cb)(unsigned int var1, unsigned int var2);

#if (CONFIG_UPGRADE_OSD == 1)
//-------------------------------------------------------------------------------------------------    
/// Register the display function in Error phase
/// @param  cb                                  \b IN: call back function for handling in finish phase
//-------------------------------------------------------------------------------------------------
void register_cb_show_error(show_error_cb cb);

//-------------------------------------------------------------------------------------------------    
/// Register the display function in finish phase
/// @param  cb                                  \b IN: call back function for handling in finish phase
//-------------------------------------------------------------------------------------------------
void register_cb_show_finish(show_finish_cb cb);

//-------------------------------------------------------------------------------------------------    
/// Register the display function in load data phase
/// @param  cb                                  \b IN: call back function for handling in load data phase
//-------------------------------------------------------------------------------------------------
void register_cb_show_load_data(show_load_data_cb cb);

//-------------------------------------------------------------------------------------------------    
/// Register the display function in the beginning phase 
/// @param  cb                                  \b IN: call back function for handling in beginning phase
//-------------------------------------------------------------------------------------------------
void register_cb_show_start_upgrading(show_start_upgrading_cb cb);

//-------------------------------------------------------------------------------------------------    
/// Register the display of upgrading phase on software upgradeing
/// @param  cb                                  \b IN: call back function for handling in upgrading phase
//-------------------------------------------------------------------------------------------------
void register_cb_show_upgrading(show_upgrading_cb cb);

//-------------------------------------------------------------------------------------------------    
/// Unregister all call back function on software upgradeing
//-------------------------------------------------------------------------------------------------
void un_register_show_cb(void);


#if (CONFIG_LVGL_UPGRADE_BAR == 1)
#include <picture_bar.h>

int show_lvgl_LoadData(int var);
int show_lvgl_StartUpgrading(int var);
int show_lvgl_Upgrading(int current_cnt, int total_cnt);
int show_lvgl_Error(int var);
int show_lvgl_Finish(int var);
#endif
int show_load_data(int var);
int show_start_upgrading(int var);
int show_upgrading(int current_cnt, int total_cnt);
int show_error(int var);
int show_finish(int var);

//---------------------------------------------------------------- ---------------------------------
/// Check whether OSD execution or not.
/// @return BOOLEAN   \b OUT: TRUE: This OSD should be run. FALSE:This OSD should be disable
//-------------------------------------------------------------------------------------------------
unsigned char check_osd_active(void);

int do_create_osdlayer(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);
int do_draw_rect(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);
int do_draw_string(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);
int do_draw_progress(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);
int do_flush2screen_osdlayer(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);
#endif


int show_error_osd(unsigned int var);
int show_finish_osd(unsigned int var);
int show_load_data_osd(unsigned int var);
int show_start_upgrading_osd(unsigned int var);
int show_upgrading_osd(unsigned int var1, unsigned int var2);


#endif

