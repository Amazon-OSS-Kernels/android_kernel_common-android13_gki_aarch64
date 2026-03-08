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

#ifndef RTK_HARDWARE_H__
#define RTK_HARDWARE_H__

#define PATCH_DATA_FIELD_MAX_SIZE       252
#define RTK_VENDOR_CONFIG_MAGIC         0x8723ab55


#define MAX_PATCH_SIZE_24K            (1024*24 + 529)   //24K
#define MAX_PATCH_SIZE_25K            (1024*25 + 529)   //25K for rtl8822b
#define MAX_PATCH_SIZE_40K            (1024*40 + 529)   //40K
#define MAX_PATCH_SIZE_49_2K          (0xC4Cf + 529)   //49.2K 8723f
#define MAX_PATCH_SIZE_69_2K          (0x114D0 + 529)  //69.2K 8852a
#define MAX_PATCH_SIZE_65_2K          (0x104D0 + 529)   //65.2K 8852b
#define MAX_PATCH_SIZE_145K           (0x24620)        //145K 8822E
#define RTK_PATCH_LENGTH_MAX          MAX_PATCH_SIZE_145K


/* h/w config control block */

typedef struct {
    u16     lmp_subversion;
    u32     hci_version_mask;
    u32     hci_revision_mask;
    u32     chip_type_mask;
    u32     project_id_mask;
    char    *patch_name;
    char    *config_name;
    u16     mac_offset;
    u32     max_patch_size;
} patch_info;


#define ROM_LMP_NONE                0x0000
#define ROM_LMP_8723a               0x1200
#define ROM_LMP_8723b               0x8723
#define ROM_LMP_8821a               0X8821
#define ROM_LMP_8761a               0X8761
#define ROM_LMP_8761b               0X8761
#define ROM_LMP_8703a               0x8723
#define ROM_LMP_8763a               0x8763
#define ROM_LMP_8703b               0x8703
#define ROM_LMP_8723c               0x8703
#define ROM_LMP_8822b               0x8822
#define ROM_LMP_8723d               0x8723
#define ROM_LMP_8821c               0x8821
#define ROM_LMP_8822c               0x8822
#define ROM_LMP_8852a               0x8852
#define ROM_LMP_8723f               0x8723
#define ROM_LMP_8852b               0x8852
#define ROM_LMP_8763c               0x8763
#define ROM_LMP_8773b               0x8773
#define ROM_LMP_8762a               0x8762
#define ROM_LMP_8762b               0x8762
#define ROM_LMP_8852c               0x8852
#define ROM_LMP_8851a               0x8852
#define ROM_LMP_8852bp              0x8852
#define ROM_LMP_8851b               0x8851
#define ROM_LMP_8822e               0x8822

#define CONFIG_MAC_OFFSET_GEN_1_2       (0x3C)      //MAC's OFFSET in config/efuse for realtek generation 1~2 bluetooth chip
#define CONFIG_MAC_OFFSET_GEN_3PLUS     (0x44)      //MAC's OFFSET in config/efuse for rtk generation 3+ bluetooth chip
#define CONFIG_MAC_OFFSET_GEN_4PLUS     (0x30)      //MAC's OFFSET in config/efuse for rtk generation 4+ bluetooth chip

struct rtk_epatch_ota{
    uint8_t chip_id;
    uint8_t enable;
    uint16_t reserve;
} __attribute__ ((packed));

struct rtk_epatch_fragment{
    uint8_t chip_id;
    uint8_t priority;
    uint8_t key_id;
    uint8_t reserve;
    uint32_t length;
    uint8_t  data[0];
} __attribute__ ((packed));

struct rtk_epatch_section{
    uint32_t opcode;
    uint32_t length;
    uint16_t number_of_fragment;
    uint16_t reserve;
    struct rtk_epatch_fragment fragment[0];
} __attribute__ ((packed));

struct rtk_epatch_v2{
    uint8_t signature[8];
    uint32_t fw_version;
    uint32_t fw_version_sub;
    uint16_t number_of_section;
    struct rtk_epatch_section section[0];
} __attribute__ ((packed));

struct rtk_epatch_fragment_linklist{
    struct rtk_epatch_fragment *fragment;
    struct rtk_epatch_fragment_linklist * next;
} __attribute__ ((packed));

typedef enum{
    AMAZON_SET_IR_TYPE=1,
    AMAZON_SET_IR_KEYCODE_SIZE,
    AMAZON_SET_IR_KEYMAP,
    AMAZON_SET_IR_ADTYPE,
    AMAZON_SET_WOBLE_EN
}AMAZON_AC;

typedef struct
{
    uint32_t    max_patch_size;
    uint32_t    baudrate;
    uint16_t    lmp_subversion;
    uint16_t    lmp_subversion_default;
    uint16_t    lmp_sub_current;
    uint8_t     state;          /* Hardware configuration state */
    uint8_t     eversion;
    uint64_t    project_id_mask;
    uint8_t     hci_version;
    uint8_t     hci_revision;
    uint8_t     chip_type;
    uint8_t     dl_fw_flag;
    int         fw_len;          /* FW patch file len */
    size_t      config_len;      /* Config patch file len */
    unsigned int         total_len;       /* FW & config extracted buf len */
    uint8_t     *fw_buf;         /* FW patch file buf */
    uint8_t     *config_buf;     /* Config patch file buf */
    uint8_t     *total_buf;      /* FW & config extracted buf */
    uint16_t     patch_frag_cnt;  /* Patch fragment count download */
    uint16_t     patch_frag_idx;  /* Current patch fragment index */
    uint8_t     patch_frag_len;  /* Patch fragment length */
    uint8_t     patch_frag_tail; /* Last patch fragment length */
    uint8_t     hw_flow_cntrl;   /* Uart flow control, bit7:set, bit0:enable */
    uint16_t    vid;   /* usb vendor id */
    uint16_t    pid;   /* usb product id */
    uint8_t     heartbeat; /*heartbeat*/
    uint8_t     parsing_rule; /* fw merge rule 1: v1, 2: v2 */
    uint8_t     keyid; /* fw merge rule 1: v1, 2: v2 */
    uint8_t     ir_type; /*amazon ir_type*/
    uint8_t     ir_wake_host_po; /*BT_WAKE_HOST_POLARITY*/
    uint8_t     ir_keycode_size;/*amazon ir_keycode_size*/
    uint32_t     (*ir_keymap)[2];/*amazon ir_keymap*/
} bt_hw_cfg_cb_t;

#define HCI_VERSION_5_3             0x000C
#define HCI_VERSION_5_2             0x000B
#define HCI_VERSION_5_1             0x000A
#define HCI_VERSION_5_0             0x0009
#define HCI_VERSION_4_2             0x0008
#define HCI_VERSION_4_1             0x0007
#define HCI_VERSION_4_0             0x0006
#define HCI_VERSION_2_1             0x0004

struct rtk_bt_vendor_config_entry{
    u16 offset;
    u8 entry_len;
    u8 entry_data[0];
} __attribute__ ((packed));

struct rtk_bt_vendor_config{
    u32 signature;
    u16 data_len;
    struct rtk_bt_vendor_config_entry entry[0];
} __attribute__ ((packed));

/******************************************************************************
**  Constants & Macros
******************************************************************************/

#define send_cmd(c,l,d)\
        RTK_INFO("%s \n",__func__);\
        if (!fw_info)\
            return -ENODEV;\
        fw_info->cmd_hdr->opcode = c;\
        fw_info->cmd_hdr->plen = l;\
        fw_info->pkt_len = CMD_HDR_LEN + l;\
        if(l != 0)\
            memcpy(fw_info->req_para,d,l);\
        ret_val = send_hci_cmd(fw_info);\
        if(ret_val < 0) {\
            RTK_ERROR("%s: Failed to send hci cmd 0x%04x, errno %d \n",__func__, fw_info->cmd_hdr->opcode, ret_val);\
            return ret_val;\
        }\
        
#define recv_evt(e)\
        ret_val = rcv_hci_evt(fw_info);\
        if (ret_val < 0){\
            RTK_ERROR("%s: Failed to receive hci event, errno %d\n",__func__, ret_val);\
            return ret_val;\
        }\
        evt = (struct e *)(fw_info->rsp_para);\

#endif

