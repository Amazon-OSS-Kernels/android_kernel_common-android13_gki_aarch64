/******************************************************************************
 *
 *  Copyright (C) 2009-2012 Realtek Corporation
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at:
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 ******************************************************************************/

#ifndef __RTK_FUNC_H_
#define __RTK_FUNC_H_

#include "hardware.h"
#define RTK_VERSION "5.0.0_r1"

int load_parse_woble_config_to_raw(unsigned char *file, int32_t length, char *cmd_label, uint8_t *in);
uint32_t rtk_usb_get_bt_config(bt_hw_cfg_cb_t *cfg, unsigned char** config_buf, char *config_file_short_name, uint16_t mac_offset);
int rtk_get_bt_firmware(uint8_t **fw_buf, char *fw_short_name);
int rtk_get_woble_config(uint8_t **file_buf);
int rtk_get_paired_devices(uint8_t **file_buf);
uint8_t rtk_get_fw_parsing_rule(uint8_t *p_buf);
void rtk_get_bt_usb_final_patch(bt_hw_cfg_cb_t *cfg_cb);

firmware_info *firmware_info_init(bt_ctlr_t udev);
int get_firmware(firmware_info *fw_info);
int reset_controller(firmware_info *fw_info);
int download_data(firmware_info *fw_info);
int reset_channel(firmware_info *fw_info);

int build_send_vendor_cmd(firmware_info *fw_info, u16 opcode, uint8_t *in);
int add_woble_accept_list(firmware_info *fw_info);
int set_ex_scan_dis_enable(firmware_info *fw_info, u8 en);
int set_ex_scan_para(firmware_info *fw_info);
int set_vnd_bt_off(firmware_info *fw_info);

void firmware_info_destroy(firmware_info *fw_info);
int rtk_clean_up(bt_ctlr_t dev);

#endif
