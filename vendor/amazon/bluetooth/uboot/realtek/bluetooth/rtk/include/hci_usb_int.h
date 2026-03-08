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

#ifndef __RTLFW_H__
#define __RTLFW_H__

/*******************************
**    Reasil patch code
********************************/
#define PKT_LEN         300
#define MSG_TO          1000
#define PATCH_SEG_MAX   252
#define DATA_END        0x80
#define CMD_HDR_LEN     sizeof(struct hci_command_hdr)
#define EVT_HDR_LEN     sizeof(struct hci_event_hdr)
#define CMD_CMP_LEN     sizeof(struct hci_ev_cmd_complete)

enum rtk_endpoit {
    CTRL_EP = 0,
    INTR_EP = 1,
    BULK_EP = 2,
    ISOC_EP = 3
};

struct hci_command_hdr {
    u16    opcode;        /* OCF & OGF */
    unsigned char    plen;
} __attribute__((packed));

struct hci_event_hdr {
    unsigned char    evt;
    unsigned char    plen;
} __attribute__((packed));

struct hci_ev_cmd_complete {
    unsigned char     ncmd;
    unsigned char     opcode[2];
} __attribute__((packed));

struct rtk_reset_evt {
    uint8_t status;
} __attribute__ ((packed));

struct rtk_keycode_size_evt {
    unsigned char status;
} __attribute__ ((packed));

struct rtk_keymap_evt {
    unsigned char status;
} __attribute__ ((packed));

struct rtk_iradv_evt {
    unsigned char status;
} __attribute__ ((packed));

struct rtk_woble_en_evt {
    unsigned char status;
} __attribute__ ((packed));

struct rtk_eversion_evt {
    unsigned char status;
    unsigned char version;
    u16 revision;
    u8 lmp_ver;
    u16 comp_id;
    u16 lmp_sub;
} __attribute__ ((packed));

struct rtk_romvrsion_evt {
    unsigned char status;
    unsigned char version;
} __attribute__ ((packed));

struct rtk_readkeyid_evt {
    unsigned char status;
    unsigned char keyid;
    unsigned char resv;
} __attribute__ ((packed));

struct rtk_readkeyid_evt_1 {
    unsigned char status;
    unsigned short lmp_v;
} __attribute__ ((packed));

struct rtk_readkeyid_evt_2 {
    unsigned char status;
    unsigned short hci_rev;
} __attribute__ ((packed));

struct rtk_apcf_evt {
    unsigned char status;
    unsigned char r1;
    unsigned char r2;
} __attribute__ ((packed));

struct rtk_ex_scan_para_evt {
    unsigned char status;
} __attribute__ ((packed));

struct rtk_ex_scan_en_dis_evt {
    unsigned char status;
} __attribute__ ((packed));

struct rtk_vend_evt {
    unsigned char status;
} __attribute__ ((packed));

struct rtk_extension_entry {
    unsigned char opcode;
    unsigned char length;
    unsigned char *data;
} __attribute__ ((packed));

struct download_cp {
    unsigned char index;
    unsigned char data[PATCH_SEG_MAX];
} __attribute__((packed)) ;

struct download_rp {
    unsigned char status;
    unsigned char index;
} __attribute__((packed)) ;

typedef struct {
    // struct osi_usb_interface    *intf;
    struct usb_device        *udev;
    int                        pipe_in, pipe_out;
    unsigned char        *send_pkt;
    unsigned char        *rcv_pkt;
    struct hci_command_hdr        *cmd_hdr;
    struct hci_event_hdr        *evt_hdr;
    struct hci_ev_cmd_complete    *cmd_cmp;
    unsigned char        *req_para,    *rsp_para;
    unsigned char        *fw_data;
    int            pkt_len;
    int            fw_len;
} firmware_info;

#endif
