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

#include "osi_include_ext.h"
#include "osi_include_int.h"

static firmware_info *fw_info;

int ld_bt_rtk_init(bt_ctlr_t udev)
{
    int ret;

    fw_info = firmware_info_init(udev);
    if (fw_info == NULL) {
        RTK_ERROR("firmware_info_init failed\n");
        return -1;
    }

    ret = get_firmware(fw_info);
    if (ret < 0) {
        RTK_ERROR("get_firmware failed\n");
        return -1;
    }
    RTK_DEBUG("Read firmware files done\n");

    ret = reset_controller(fw_info);
    if (ret < 0) {
        RTK_ERROR("reset_controller failed\n");
        return -1;
    }
    RTK_DEBUG("Reset controller done\n");

    ret = download_data(fw_info);
    if (ret <= 0) {
        RTK_ERROR("download_data failed\n");
        return -1;
    }
    RTK_DEBUG("Download firmware done\n");

    ret = reset_channel(fw_info);
    if (ret < 0) {
        RTK_ERROR("reset_channel failed\n");
        return -1;
    }

    return 0;
}

int ld_bt_rtk_set_woble(bt_ctlr_t udev)
{
    unsigned char *config_file = NULL;
    int length;
    unsigned char bytes[256];

    length = rtk_get_woble_config(&config_file);
    if (config_file == NULL) {
        RTK_ERROR("rtk_get_woble_config failed\n");
        return -1;
    }
    RTK_DEBUG("Read WOBLE config file done\n");

    load_parse_woble_config_to_raw(config_file, length, "APCF00", bytes);
    build_send_vendor_cmd(fw_info, HCI_VENDOR_APCF_SUB, bytes);

    load_parse_woble_config_to_raw(config_file, length, "APCF01", bytes);
    build_send_vendor_cmd(fw_info, HCI_VENDOR_APCF_SUB, bytes);

    load_parse_woble_config_to_raw(config_file, length, "APCF02", bytes);
    build_send_vendor_cmd(fw_info, HCI_VENDOR_APCF_SUB, bytes);

    load_parse_woble_config_to_raw(config_file, length, "WOBLE00", bytes);
    build_send_vendor_cmd(fw_info, 0xFDB4, bytes);

    free(config_file);
    RTK_DEBUG("WOBLE config file done\n");

    add_woble_accept_list(fw_info);     // woble01
    set_ex_scan_dis_enable(fw_info, 0);
    set_ex_scan_para(fw_info);
    set_ex_scan_dis_enable(fw_info, 1);
    set_vnd_bt_off(fw_info);            //enter suspend mode

    return 0;
}

int ld_bt_rtk_clean_up(bt_ctlr_t udev)
{
    firmware_info_destroy(fw_info);
    rtk_clean_up(udev);

    return 0;
}

void ld_bt_func_init_rtk(ld_bt_func_t *ptr)
{
    ptr->init = ld_bt_rtk_init;
    ptr->set_woble = ld_bt_rtk_set_woble;
    ptr->clean_up = ld_bt_rtk_clean_up;
}
