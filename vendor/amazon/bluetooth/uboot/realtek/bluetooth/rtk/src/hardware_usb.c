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

static bt_hw_cfg_cb_t hw_cfg_cb;

typedef struct {
    uint16_t    vid;
    uint16_t    pid;
    uint16_t    lmp_sub_default;
    uint16_t    lmp_sub;
    uint16_t    eversion;
    char        *mp_patch_name;
    char        *patch_name;
    char        *config_name;
    uint8_t     *fw_cache;
    int         fw_len;
    uint16_t    mac_offset;
    uint32_t    max_patch_size;
} usb_patch_info;

static usb_patch_info usb_fw_patch_table[] = {
/* { vid, pid, lmp_sub_default, lmp_sub, eversion, mp_fw_name, fw_name, config_name, fw_cache, fw_len, mac_offset, max_patch_size } */
{ 0x0BDA, 0xA822, 0x8822, 0, 0, "mp_rtl8822e_fw", "rtl8822e_8822c_fw", "rtl8822e_8822c_config", NULL, 0, CONFIG_MAC_OFFSET_GEN_4PLUS, MAX_PATCH_SIZE_145K}, /* RTL8822EU */
{ 0x0BDA, 0xA82A, 0x8822, 0, 0, "mp_rtl8822e_fw", "rtl8822e_8822c_fw", "rtl8822e_8822c_config", NULL, 0, CONFIG_MAC_OFFSET_GEN_4PLUS, MAX_PATCH_SIZE_145K}, /* RTL8822EU */
{ 0x0BDA, 0xA82B, 0x8822, 0, 0, "mp_rtl8822e_fw", "rtl8822e_8822c_fw", "rtl8822e_8822c_config", NULL, 0, CONFIG_MAC_OFFSET_GEN_4PLUS, MAX_PATCH_SIZE_145K}, /* RTL8822EU */
{ 0x0BDA, 0xE822, 0x8822, 0, 0, "mp_rtl8822e_fw", "rtl8822e_8822c_fw", "rtl8822e_8822c_config", NULL, 0, CONFIG_MAC_OFFSET_GEN_4PLUS, MAX_PATCH_SIZE_145K}, /* RTL8822EU */

/* NOTE: must append patch entries above the null entry */
{ 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, 0, 0, 0 }
};

firmware_info *firmware_info_init(bt_ctlr_t udev)
{
    firmware_info *fw_info;

    RTK_DEBUG("%s: start\n", __func__);
    // osi_buff_init();

    fw_info = calloc(1, sizeof(*fw_info));
    if (fw_info == NULL)
        return NULL;

    // Notes: use malloc() for total_buf instead of fw_data
    // fw_info->fw_data = calloc(1, RTK_PATCH_LENGTH_MAX);
    // if (fw_info->fw_data == NULL) {
    //    free(fw_info);
    //    return NULL;
    // }

    fw_info->send_pkt = calloc(1, PKT_LEN);
    if (fw_info->send_pkt == NULL) {
        // free(fw_info->fw_data);
        free(fw_info);
        return NULL;
    }

    fw_info->rcv_pkt = calloc(1, PKT_LEN);
    if (fw_info->rcv_pkt == NULL) {
        free(fw_info->send_pkt);
        // free(fw_info->fw_data);
        free(fw_info);
        return NULL;
    }

    RTK_DEBUG("%s: calloc done\n", __func__);

    // fw_info->intf = NULL;
    fw_info->udev = udev;
    fw_info->pipe_in = usb_rcvintpipe(udev, INTR_EP);
    fw_info->pipe_out = usb_sndctrlpipe(udev, CTRL_EP);
    fw_info->cmd_hdr = (struct hci_command_hdr *)(fw_info->send_pkt);
    fw_info->evt_hdr = (struct hci_event_hdr *)(fw_info->rcv_pkt);
    fw_info->cmd_cmp = (struct hci_ev_cmd_complete *)(fw_info->rcv_pkt + EVT_HDR_LEN);
    fw_info->req_para = fw_info->send_pkt + CMD_HDR_LEN;
    fw_info->rsp_para = fw_info->rcv_pkt + EVT_HDR_LEN + CMD_CMP_LEN;

    RTK_DEBUG("%s: end\n", __func__);

    return fw_info;
}

void firmware_info_destroy(firmware_info *fw_info)
{
    // free(fw_info->fw_data);
    free(fw_info->rcv_pkt);
    free(fw_info->send_pkt);
    free(fw_info);
}

int send_hci_cmd(firmware_info *fw_info)
{
    int ret_val;

    ret_val = usb_control_msg(
        fw_info->udev, fw_info->pipe_out,
        0, USB_TYPE_CLASS, 0, 0,
        (void *)(fw_info->send_pkt),
        fw_info->pkt_len, MSG_TO);

    return ret_val;
}

int rcv_hci_evt(firmware_info *fw_info)
{
    int ret_val = 0;
    int i;
    u16 opcode;
    while (1) {
        for (i = 0; i < 5; i++) {
            ret_val = usb_submit_int_msg(
                fw_info->udev, fw_info->pipe_in,
                (void *)(fw_info->rcv_pkt), PKT_LEN,
                MSG_TO);
                if (ret_val >= 0)
                    break;
        }
        RTK_DEBUG("fw_info->rcv_pkt: 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n",
                   fw_info->rcv_pkt[0],
                   fw_info->rcv_pkt[1],
                   fw_info->rcv_pkt[2],
                   fw_info->rcv_pkt[3],
                   fw_info->rcv_pkt[4],
                   fw_info->rcv_pkt[5],
                   fw_info->rcv_pkt[6]);
        if (ret_val < 0)
            return ret_val;

        if (HCI_COMMAND_COMPLETE_EVT == fw_info->evt_hdr->evt) {
            opcode = fw_info->cmd_cmp->opcode[0] | (fw_info->cmd_cmp->opcode[1] << 8);
            RTK_DEBUG("opcode: 0x%02x\n", opcode);
            if (fw_info->cmd_hdr->opcode == opcode)
                return 0;
        }
    }
}

static usb_patch_info *rtk_usb_get_fw_table_entry(uint16_t vid, uint16_t pid)
{
    usb_patch_info *patch_entry = usb_fw_patch_table;

    uint32_t entry_size = sizeof(usb_fw_patch_table) / sizeof(usb_fw_patch_table[0]);
    uint32_t i;

    for (i = 0; i < entry_size; i++, patch_entry++) {
        if ((vid == patch_entry->vid)&&(pid == patch_entry->pid))
            break;
    }

    if (i == entry_size) {
        RTK_DEBUG("%s: No fw table entry found\n", __func__);
        return NULL;
    }

    return patch_entry;
}

int vendor_read_rom_version(firmware_info *fw_info)
{
    int ret_val;
    struct rtk_romvrsion_evt *evt;
    send_cmd(HCI_VSC_READ_ROM_VERSION, 0, NULL);
    recv_evt(rtk_romvrsion_evt);
    hw_cfg_cb.eversion = evt->version;
    RTK_DEBUG("%s: status %d, version %d\n", __func__, evt->status, evt->version);
    return ret_val;
}

int vendor_read_chip_type(firmware_info *fw_info)
{
    int ret_val;
    struct rtk_readkeyid_evt *evt;
    u8 d[]={0x10,0xA4,0xAD,0x00,0xB0};
    send_cmd(HCI_VSC_READ_CHIP_TYPE,5,d);
    recv_evt(rtk_readkeyid_evt);
    hw_cfg_cb.keyid = evt->keyid;
    RTK_DEBUG("%s: status %d, keyid %d\n", __func__, evt->status, evt->keyid);
    return ret_val;
}

int vendor_read_chip_type_1(firmware_info *fw_info)
{
    int ret_val;
    struct rtk_readkeyid_evt_1 *evt;
    u8 d[]={0x10,0x38,0x04,0x28,0x80};
    send_cmd(HCI_VSC_READ_CHIP_TYPE,5,d);
    recv_evt(rtk_readkeyid_evt_1);
    if (evt->lmp_v != 0x8822)
        return ENO8822;
    hw_cfg_cb.lmp_subversion = evt->lmp_v;
    RTK_DEBUG("%s: status %d, Read FC61 Lmp Sub Version 0x%x\n", __func__, evt->status, hw_cfg_cb.lmp_subversion);
    return ret_val;
}

int vendor_read_chip_type_2(firmware_info *fw_info)
{
    int ret_val;
    struct rtk_readkeyid_evt_2 *evt;
    u8 d[]={0x10,0x3A,0x04,0x28,0x80};
    send_cmd(HCI_VSC_READ_CHIP_TYPE,5,d);
    recv_evt(rtk_readkeyid_evt_2);
    if (evt->hci_rev != 0x000e)
        return ENO000E;
    hw_cfg_cb.hci_version = (uint8_t)HCI_VERSION_5_3;
    hw_cfg_cb.hci_revision = evt->hci_rev;
    RTK_DEBUG("%s: status %d, Read FC61 Hci Sub Version event 0x%d\n", __func__, evt->status, hw_cfg_cb.hci_revision);
    return ret_val;
}

int get_eversion(firmware_info *fw_info)
{
    int ret_val;
    struct rtk_eversion_evt *evt;
    send_cmd(HCI_READ_LMP_VERSION, 0, NULL);
    recv_evt(rtk_eversion_evt);
    hw_cfg_cb.hci_version = evt->version;
    hw_cfg_cb.hci_revision =(u8)evt->revision;
    hw_cfg_cb.lmp_subversion=evt->lmp_sub;
    RTK_DEBUG("%s: status %d, hw_cfg_cb.hci_version %d,hw_cfg_cb.hci_revision %d ,hw_cfg_cb.lmp_subversion %d\n", __func__,
        evt->status,hw_cfg_cb.hci_version,hw_cfg_cb.hci_revision,hw_cfg_cb.lmp_subversion);
    return ret_val;
}

int get_firmware(firmware_info *fw_info)
{
    usb_patch_info *prtk_usb_patch_file_info = NULL;

    memset(&hw_cfg_cb, 0, sizeof(bt_hw_cfg_cb_t));
    vendor_read_rom_version(fw_info);

    RTK_DEBUG("%s: start\n", __func__);

    prtk_usb_patch_file_info = rtk_usb_get_fw_table_entry(fw_info->udev->descriptor.idVendor, fw_info->udev->descriptor.idProduct);
    if((prtk_usb_patch_file_info == NULL) || (prtk_usb_patch_file_info->lmp_sub_default == 0)) {
        RTK_DEBUG("get patch entry error\n");
        return -1;
    }
    hw_cfg_cb.lmp_subversion_default = prtk_usb_patch_file_info->lmp_sub_default;

    RTK_DEBUG("%s: patch entry found\n", __func__);

    hw_cfg_cb.config_len = rtk_usb_get_bt_config(&hw_cfg_cb, &hw_cfg_cb.config_buf, prtk_usb_patch_file_info->config_name, prtk_usb_patch_file_info->mac_offset);
    if (hw_cfg_cb.config_len < 0) {
        RTK_DEBUG("get BT config error\n");
        hw_cfg_cb.config_len = 0;
        return -1;
    }

    hw_cfg_cb.fw_len = rtk_get_bt_firmware(&hw_cfg_cb.fw_buf, prtk_usb_patch_file_info->patch_name);
    if (hw_cfg_cb.fw_len < 0) {
        RTK_DEBUG("get BT firmware error\n");
        hw_cfg_cb.fw_len = 0;
        return -1;
    } else {
        hw_cfg_cb.parsing_rule = rtk_get_fw_parsing_rule(hw_cfg_cb.fw_buf + hw_cfg_cb.fw_len - 5);
        if (hw_cfg_cb.parsing_rule == 2) {
            vendor_read_chip_type(fw_info);
            if ((vendor_read_chip_type_1(fw_info) == 0) && (vendor_read_chip_type_2(fw_info) == 0)) {
                rtk_get_bt_usb_final_patch(&hw_cfg_cb); //8822e
            } else {
                get_eversion(fw_info);
                rtk_get_bt_usb_final_patch(&hw_cfg_cb);
            }
        } else {
            rtk_get_bt_usb_final_patch(&hw_cfg_cb);
        }
    }

    fw_info->fw_len = hw_cfg_cb.total_len;
    fw_info->fw_data = hw_cfg_cb.total_buf;

    RTK_DEBUG("%s: fw_len=%d\n", __func__, fw_info->fw_len);
    if (fw_info->fw_len <= 0)
        return -1;

    return 0;
}

/* reset_controller is aimed to reset_bt_fw before update FW patch */
int reset_controller(firmware_info *fw_info)
{
    int ret_val;
    send_cmd(HCI_VENDOR_FORCE_RESET_AND_PATCHABLE, 0, NULL);
    udelay(1000 * 200);
    RTK_DEBUG("%s: wait for FW reset for 200ms\n", __func__);
    return ret_val;
}

/* reset_channel to recover the communication between wifi 8192eu with 8761 bt controller in case of geteversion error */
int reset_channel(firmware_info *fw_info)
{
    int ret_val;
    struct rtk_reset_evt *evt;
    send_cmd(HCI_VENDOR_RESET, 0, NULL);
    recv_evt(rtk_reset_evt);
    RTK_DEBUG("%s: status %d\n", __func__, evt->status);
    evt = evt; // bypass warning
    udelay(1000 * 300);
    RTK_DEBUG("%s: wait for channel reset for 300ms\n", __func__);
    return ret_val;
}

int download_data(firmware_info *fw_info)
{
    struct download_cp cmd_para;
    struct download_rp *evt = NULL;
    int pkt_len, frag_num, frag_len, i, ret_val;
    unsigned char *pcur;
    frag_num = fw_info->fw_len / PATCH_SEG_MAX + 1;
    pkt_len = sizeof(struct download_cp);
    cmd_para.index = 0;
    pcur=fw_info->fw_data;
    frag_len = PATCH_SEG_MAX;
    RTK_DEBUG("%s: start\n", __func__);
    for (i = 0; i < frag_num; i++) {
        if (i == (frag_num - 1)) {
            cmd_para.index |= DATA_END;
            frag_len = fw_info->fw_len % PATCH_SEG_MAX;
            pkt_len -= (PATCH_SEG_MAX - frag_len);
        }
        memcpy(cmd_para.data, pcur, frag_len);
        send_cmd(HCI_VSC_DOWNLOAD_FW_PATCH, pkt_len, &cmd_para);
        recv_evt(download_rp);
        if (0 != evt->status) {
            RTK_ERROR("%s: Receive acked frag num %d, err status %d\n", __func__, ret_val, evt->status);
            return -1;
        }
        if ((evt->index & DATA_END) || (evt->index == frag_num - 1)) {
            RTK_DEBUG("%s: Receive last acked index %d\n", __func__, evt->index);
            goto end;
        }
        cmd_para.index ++;
        if (cmd_para.index == 0x80)
            cmd_para.index = 1;
        pcur += PATCH_SEG_MAX;
    }
end:
    RTK_DEBUG("%s: done, sent %d frag pkts, received %d frag events\n", __func__, cmd_para.index, evt->index);
    return fw_info->fw_len;
}


uint8_t *revermem(uint8_t *str, uint32_t len)
{
    uint8_t *start = str;
    uint8_t *end = str + len - 1;
    uint8_t ch;

    if (str != NULL) {
        while (start < end) {
            ch = *start;
            *start++ = *end;
            *end-- = ch;
        }
    }

    return str;
}

int set_add_dev_acept_list(firmware_info *fw_info, uint8_t *wakeup_bdaddr)
{
    struct rtk_vend_evt *evt;
    int ret_val = 0;
    uint8_t s[7] = {0};
    s[0] = 0;
    wakeup_bdaddr = revermem(wakeup_bdaddr,6);
    memcpy(s+1,wakeup_bdaddr,6);//APCF_Broadcaster_Address 
    send_cmd(HCI_LE_ADD_ACPT_LIST, 7, s);
    recv_evt(rtk_vend_evt);
    RTK_DEBUG("%s: status %d\n", __func__, evt->status);
    evt = evt; // bypass warning
    return ret_val;
}

int set_ex_scan_dis_enable(firmware_info *fw_info, u8 en)
{
    struct rtk_ex_scan_en_dis_evt *evt;
    int ret_val = 0;
    uint8_t s[6] = {0};
    s[0] = en;//LE Scan Enable     1 Enabled 0 Disable
    s[1] = 0;//Filter Duplicates    Disabled
    memset(s + 2,0,2);//Scan Duration    0 s
    memset(s + 4,0,2);//Scan Period    0 s
    send_cmd(HCI_BLE_SET_EXT_SCAN_ENABLE,6,s);
    recv_evt(rtk_ex_scan_en_dis_evt);
    RTK_DEBUG("%s: status %d\n", __func__, evt->status);
    evt = evt; // bypass warning
    return ret_val;
}

int set_ex_scan_para(firmware_info *fw_info)
{
    struct rtk_ex_scan_para_evt *evt;
    int ret_val;
    uint8_t s[8] = {0};
    s[0] = 0;//Own Address Type    Public
    s[1] = 1;//Scanning Filter Policy  Basic filtered scanning filter policy The Link Layer shall process advertising and scan response PDUs only from devices in the Filter Accept List
    s[2] = 1;//Scanning PHYs    LE 1M
    s[3] = 1;//Scan Type    Active
    *((u16 *)(s+4))= 1056;//Scan Interval    660 ms 
    *((u16 *)(s+6))= 50;//Scan Window 31.25 ms 
    send_cmd(HCI_BLE_SET_EXT_SCAN_PARAM,8,s);
    recv_evt(rtk_ex_scan_para_evt);
    RTK_DEBUG("%s: status %d\n", __func__, evt->status);
    evt = evt; // bypass warning
    return ret_val;
}

int set_vnd_bt_off(firmware_info *fw_info)
{
    struct rtk_vend_evt *evt;
    int ret_val;
    uint8_t s[2] = {0};
    s[0] = 1;
    s[1] = 1;
    send_cmd(HCI_VSC_BT_OFF,2,s);
    recv_evt(rtk_vend_evt);
    RTK_DEBUG("%s: status %d\n", __func__, evt->status);
    evt = evt; // bypass warning
    return ret_val;
}

int build_send_vendor_cmd(firmware_info* fw_info,u16 opcode,uint8_t * in){
    struct rtk_vend_evt *evt;
    int ret_val;
    send_cmd(opcode, in[2], in + 3);
    recv_evt(rtk_vend_evt);
    RTK_DEBUG("%s: opcode:0x%x status %d\n", __func__, opcode ,evt->status);
    evt = evt; // bypass warning
    return ret_val;
}

int add_woble_accept_list(firmware_info *fw_info)
{
    int ret = 0, len;
    uint8_t *file;
    uint8_t num = 0, i;
    uint8_t *paired_wakeup_bdaddr;

    len = rtk_get_paired_devices(&file);
    if (len < 2) {
        return -1;
    }

    num = file[1];
    if (num == 0) {
        return -1;
    }

    paired_wakeup_bdaddr = &file[2];

    for (i = 0; i < num; i++) {
        ret = set_add_dev_acept_list(fw_info, paired_wakeup_bdaddr + i*6);
    }

    free(file);
    return ret;
}

int rtk_clean_up(bt_ctlr_t dev)
{
    if(hw_cfg_cb.total_buf)
        free(hw_cfg_cb.total_buf);
    if(hw_cfg_cb.config_buf)
        free(hw_cfg_cb.config_buf);
    if(hw_cfg_cb.fw_buf)
        free(hw_cfg_cb.fw_buf);
    if(hw_cfg_cb.ir_keymap)
        free(hw_cfg_cb.ir_keymap);
    return 0;
}
