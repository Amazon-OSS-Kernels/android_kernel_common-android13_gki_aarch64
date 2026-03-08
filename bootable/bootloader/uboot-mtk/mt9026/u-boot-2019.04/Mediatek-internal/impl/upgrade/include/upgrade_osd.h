/* SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause) */
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

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

