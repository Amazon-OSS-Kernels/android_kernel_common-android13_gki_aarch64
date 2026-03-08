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

#define FIRMWARE_DIRECTORY "/vendor/firmware/"
#define BT_CONFIG_DIRECTORY "/vendor/firmware/"
#define AMAZON_CONFIG_IR_DIR "/vendor/firmware/"
#define AMAZON_CONFIG_IR_NAME "woble_config.txt"
#define AMAZON_PAIRED_DEVICES "misc/bluedroid/wake_on_ble.conf"

#define rtk_le16_to_cpu(x)        (x)
#define rtk_le32_to_cpu(x)        (x)

static const uint8_t EXTENSION_SECTION_SIGNATURE[4]={0x51,0x04,0xFD,0x77};
static const uint8_t RTK_EPATCH_SIGNATURE[8]={0x52,0x65,0x61,0x6C,0x74,0x65,0x63,0x68};
static const uint8_t RTK_EPATCH_SIGNATURE_V2[8]={0x52,0x54,0x42,0x54,0x43,0x6F,0x72,0x65};
uint8_t vnd_local_bd_addr[6]={0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

enum {
    FW_PATCH_SNIPPET = 1,
    FW_DUMMY_HEADER,
    FW_SECURITY_SIGNATURE,
    FW_OTA_FLAG
};

uint16_t usb_project_id[] = {
    ROM_LMP_8723a,
    ROM_LMP_8723b,
    ROM_LMP_8821a,
    ROM_LMP_8761a,
    ROM_LMP_8703a,
    ROM_LMP_8763a,
    ROM_LMP_8703b,
    ROM_LMP_8723c,
    ROM_LMP_8822b,
    ROM_LMP_8723d,
    ROM_LMP_8821c,
    ROM_LMP_NONE,
    ROM_LMP_NONE,
    ROM_LMP_8822c,
    ROM_LMP_8761b,
    ROM_LMP_NONE,
    ROM_LMP_NONE,   //0x10
    ROM_LMP_NONE,
    ROM_LMP_8852a,  //0x12
    ROM_LMP_8723f,
    ROM_LMP_8852b,
    ROM_LMP_8763c,  //bbpro2
    ROM_LMP_8773b,  //bblite
    ROM_LMP_8762a,  //bee
    ROM_LMP_8762b,  //bee2
    ROM_LMP_8852c,  //25
    ROM_LMP_NONE,
    ROM_LMP_NONE,
    ROM_LMP_NONE,
    ROM_LMP_NONE,
    ROM_LMP_NONE,
    ROM_LMP_NONE,
    ROM_LMP_NONE,
    ROM_LMP_8822e,
    ROM_LMP_8852bp, //34 8852bp
    ROM_LMP_8851a,
    ROM_LMP_8851b
};

char *rtk_trim(char *str)
{
    while (isspace(*str))
        ++str;

    if (!*str)
        return str;

    char *end_str = str + strlen(str) - 1;
    while (end_str > str && isspace(*end_str))
        --end_str;

    end_str[1] = '\0';
    return str;
}

int str_to_byte(char *str)
{
    int val;
    char d[2];

    if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X')) {
        d[0] = str[2];
        d[1] = str[3];
    } else {
        d[0] = str[0];
        d[1] = str[1];
    }

    if (d[0] >= '0' && d[0] <= '9') {
        val = (d[0] - '0') << 4;
    } else if (d[0] >= 'a' && d[0] <= 'f') {
        val = (d[0] - 'a' + 10) << 4;
    } else if (d[0] >= 'A' && d[0] <= 'F') {
        val = (d[0] - 'A' + 10) << 4;
    } else {
        return 0;
    }

    if (d[1] >= '0' && d[1] <= '9') {
        val += (d[1] - '0');
    } else if (d[1] >= 'a' && d[1] <= 'f') {
        val += (d[1] - 'a' + 10);
    } else if (d[1] >= 'A' && d[1] <= 'F') {
        val += (d[1] - 'A' + 10);
    } else {
        return 0;
    }

    return val;
}

static int read_one_line(char *buffer, int buffer_size, unsigned char *file, int length_to_process)
{
    int i = 0;  // number of line endings processed
    int j = 0;  // number of char bytes processed

    // skip empty lines
    while (length_to_process > 0) {
        if ((file[i] == '\r') || (file[i] == '\n')) {
            length_to_process--;
            i++;
        } else {
            break;
        }
    }

    for (j = 0; j < length_to_process; j ++) {
        if (j >= buffer_size - 1) {
            break;
        }

        buffer[j] = file[i + j];

        if ((file[i + j] == '\r') || (file[i + j] == '\n')) {
            i++;
            break;
        }
    }

    buffer[j] = '\0';

    return i + j;   // total bytes processed
}

int load_parse_woble_config_to_raw(unsigned char *file, int32_t length, char *cmd_label, uint8_t *in)
{
    int param_bytes = 0;
    char line[1024];
    int actual_line_size;
    int length_remaining = length;
    int length_processed = 0;
    char *p;
    char *token;

    while (length_remaining > 0) {
        actual_line_size = read_one_line(line, sizeof(line), file + length_processed, length_remaining);
        length_processed += actual_line_size;
        length_remaining -= actual_line_size;

        // process the line from line[1024]
        p = &line[0];

        // remove leading and trailing space
        p = rtk_trim(p);

        // skip the line if it is a comment line
        if (*p == '#') {
            continue;
        }

        // search command label from the line
        p = strstr(p, cmd_label);
        if (p == NULL) {
            continue;
        }

        // there is a ':' following the command label
        p = strchr(p, ':');
        if (p == NULL) {
            continue;
        }
        p++;

        // there must be something after the ':', otherwise just return
        if (*p == '\0') {
            break;
        }

        while (p != NULL) {
            token = strsep(&p, ",");    // get string until the next ','
            token = rtk_trim(token);

            if (*token != '\0') {
                *in = (uint8_t)str_to_byte(token);
                in++;
                param_bytes++;
            }
        }

        break;
    }

    return param_bytes;
}

uint8_t rtk_get_fw_project_id(uint8_t *p_buf)
{
    uint8_t opcode;
    uint8_t len;
    uint8_t data = 0;

    do {
        opcode = *p_buf;
        len = *(p_buf - 1);
        if (opcode == 0x00) {
            if (len == 1) {
                data = *(p_buf - 2);
                RTK_DEBUG("bt_hw_parse_project_id: opcode %d, len %d, data %d", opcode, len, data);
                break;
            } else {
                RTK_DEBUG("bt_hw_parse_project_id: invalid len %d", len);
            }
        }
        p_buf -= len + 2;
    } while (*p_buf != 0xFF);

    return data;
}

void rtk_usb_parse_config_file(bt_hw_cfg_cb_t *cfg_cb,unsigned char **config_buf, u32 *filelen, uint8_t bt_addr[6], uint16_t mac_offset)
{
    struct rtk_bt_vendor_config *config = (struct rtk_bt_vendor_config *)*config_buf;
    uint16_t config_len = rtk_le16_to_cpu(config->data_len), temp = 0;
    struct rtk_bt_vendor_config_entry *entry = config->entry;
    unsigned int i = 0;
    uint8_t heartbeat_buf = 0;
    uint8_t *p;

    RTK_DEBUG("bt_addr = %x\n", bt_addr[0]);
    if (rtk_le32_to_cpu(config->signature) != RTK_VENDOR_CONFIG_MAGIC) {
        RTK_DEBUG("config signature magic number(0x%x) is not set to RTK_VENDOR_CONFIG_MAGIC", config->signature);
        return;
    }

    if (config_len != *filelen - sizeof(struct rtk_bt_vendor_config)) {
        RTK_DEBUG("config len(0x%x) is not right(0x%zx)\n", config_len, *filelen-sizeof(struct rtk_bt_vendor_config));
        return;
    }

    cfg_cb->heartbeat = 0;
    for (i = 0; i < config_len;) {
        switch(rtk_le16_to_cpu(entry->offset)) {
        case 0x017a:
            if (mac_offset == CONFIG_MAC_OFFSET_GEN_1_2) {
                p = (uint8_t *)entry->entry_data;
                STREAM_TO_UINT8(heartbeat_buf, p);
                if ((heartbeat_buf & 0x02) && (heartbeat_buf & 0x10))
                    cfg_cb->heartbeat = 1;
                else
                    cfg_cb->heartbeat = 0;

                RTK_DEBUG("config 0x017a heartbeat = %d\n",cfg_cb->heartbeat);
            }
            break;
        case 0x01be:
            if (mac_offset == CONFIG_MAC_OFFSET_GEN_3PLUS || mac_offset == CONFIG_MAC_OFFSET_GEN_4PLUS) {
                p = (uint8_t *)entry->entry_data;
                STREAM_TO_UINT8(heartbeat_buf, p);
                if ((heartbeat_buf & 0x02) && (heartbeat_buf & 0x10))
                    cfg_cb->heartbeat = 1;
                else
                    cfg_cb->heartbeat = 0;

                RTK_DEBUG("config 0x01be heartbeat = %d\n",cfg_cb->heartbeat);
            }
            break;
        default:
            RTK_DEBUG("config offset(0x%x),length(0x%x)\n", entry->offset, entry->entry_len);
            break;
        }
        temp = entry->entry_len + sizeof(struct rtk_bt_vendor_config_entry);
        i += temp;
        entry = (struct rtk_bt_vendor_config_entry*)((uint8_t*)entry + temp);
    }

    return;
}

uint32_t rtk_usb_get_bt_config(bt_hw_cfg_cb_t *cfg, unsigned char **config_buf,
                               char *config_file_short_name, uint16_t mac_offset)
{
    u32 len;
    u8 *file;

    // file = ld_bt_load_file(config_file_short_name, BT_CONFIG_DIRECTORY, &len);
    file = ld_bt_load_file(config_file_short_name, NULL, (s32 *)&len);
    if (file == NULL) {
        return -1;
    }

    if (len > MAX_ORG_CONFIG_SIZE) {
        RTK_DEBUG("bt config file is too large(>0x%04x)\n", MAX_ORG_CONFIG_SIZE);
        return -1;
    }

    rtk_usb_parse_config_file(cfg, &file, &len, vnd_local_bd_addr, mac_offset);

    *config_buf = file;
    return len;
}

int rtk_get_bt_firmware(uint8_t **fw_buf, char *fw_short_name)
{
    u32 len;
    u8 *file;

    // file = ld_bt_load_file(fw_short_name, FIRMWARE_DIRECTORY, &len);
    file = ld_bt_load_file(fw_short_name, NULL, (s32 *)&len);
    if (file == NULL) {
        return -1;
    }
    *fw_buf = file;

    return len;
}

int rtk_get_woble_config(uint8_t **file_buf)
{
    u32 len;
    u8 *file;

    file = ld_bt_load_file(AMAZON_CONFIG_IR_NAME, NULL, (s32 *)&len);
    if (file == NULL) {
        return -1;
    }
    *file_buf = file;

    return len;
}

int rtk_get_paired_devices(uint8_t **file_buf)
{
    u32 len;
    u8 *file;

    file = ld_bt_load_file(AMAZON_PAIRED_DEVICES, NULL, (s32 *)&len);
    if (file == NULL) {
        return -1;
    }
    *file_buf = file;

    return len;
}

uint8_t rtk_get_fw_parsing_rule(uint8_t *p_buf)
{
    uint8_t opcode;
    uint8_t len;
    uint8_t data = 1;

    do {
        opcode = *p_buf;
        len = *(p_buf - 1);
        if (opcode == 0x01) {
            if (len == 1) {
                data = *(p_buf - 2);
                RTK_DEBUG("rtk_get_fw_parsing_rule: opcode %d, len %d, data %d", opcode, len, data);
                break;
            } else {
                RTK_DEBUG("rtk_get_fw_parsing_rule: invalid len %d", len);
            }
        }
        p_buf -= len + 2;
    } while (*p_buf != 0xFF);

    return data;
}

uint8_t rtk_check_epatch_signature(bt_hw_cfg_cb_t *cfg_cb, uint8_t parsing_rule)
{
    if (parsing_rule == 1) {
        RTK_DEBUG("using legacy parsing rule(V1) ");
        if (cfg_cb->lmp_subversion == 0x1200) { //LMPSUBVERSION_8723a
            if (memcmp(cfg_cb->fw_buf, RTK_EPATCH_SIGNATURE, 8) == 0) {
                RTK_DEBUG("8723as check signature error!");
                cfg_cb->dl_fw_flag = 0;
                return -1;
            } else {
                cfg_cb->total_len = cfg_cb->fw_len + cfg_cb->config_len;
                if (!(cfg_cb->total_buf = malloc(cfg_cb->total_len))) {
                    RTK_DEBUG("can't alloc memory for fw&config, errno:%d", errno);
                    cfg_cb->dl_fw_flag = 0;
                    return -1;
                } else {
                    RTK_DEBUG("8723as, fw copy direct");
                    memcpy(cfg_cb->total_buf, cfg_cb->fw_buf, cfg_cb->fw_len);
                    memcpy(cfg_cb->total_buf+cfg_cb->fw_len, cfg_cb->config_buf, cfg_cb->config_len);
                    //cfg_cb->lmp_sub_current = *(uint16_t *)(cfg_cb->total_buf + cfg_cb->total_len - cfg_cb->config_len - 4);
                    cfg_cb->dl_fw_flag = 1;
                    return -1;
                }
            }
        }

        if (memcmp(cfg_cb->fw_buf, RTK_EPATCH_SIGNATURE, 8)) {
            RTK_DEBUG("check signature error");
            cfg_cb->dl_fw_flag = 0;
            return -1;
        }

    } else if (parsing_rule == 2) {
        RTK_DEBUG("using new parsing rule(V2) ");
        if (memcmp(cfg_cb->fw_buf, RTK_EPATCH_SIGNATURE_V2, 8)) {
            RTK_DEBUG("check signature error");
            cfg_cb->dl_fw_flag = 0;
            return -1;
        }

    } else {
        RTK_DEBUG(" error parsing rule ");
        return -1;
    }

    return 0;
}

struct rtk_epatch_entry *rtk_get_patch_entry(bt_hw_cfg_cb_t *cfg_cb)
{
    u16 i;
    struct rtk_epatch *patch;
    struct rtk_epatch_entry *entry;
    u8 *p;
    u16 chip_id;

    patch = (struct rtk_epatch *)cfg_cb->fw_buf;
    entry = (struct rtk_epatch_entry *)malloc(sizeof(*entry));
    if(!entry) {
        RTK_ERROR("rtk_get_patch_entry: failed to allocate mem for patch entry \n");
        return NULL;
    }

    patch->number_of_total_patch = rtk_le16_to_cpu(patch->number_of_total_patch);

    RTK_INFO("rtk_get_patch_entry: fw_ver 0x%08x, patch_num %d \n",
              rtk_le32_to_cpu(patch->fw_version), patch->number_of_total_patch);

    for (i = 0; i < patch->number_of_total_patch; i++) {
        p = cfg_cb->fw_buf + 14 + 2 * i;
        STREAM_TO_UINT16(chip_id, p);
        if (chip_id == cfg_cb->eversion + 1) {
            entry->chip_id = chip_id;
            p = cfg_cb->fw_buf + 14 + 2 * patch->number_of_total_patch + 2 * i;
            STREAM_TO_UINT16(entry->patch_length, p);
            p = cfg_cb->fw_buf + 14 + 4 * patch->number_of_total_patch + 4 * i;
            STREAM_TO_UINT32(entry->start_offset, p);
            RTK_INFO("rtk_get_patch_entry: chip_id %d, patch_len 0x%x, patch_offset 0x%x \n",
                    entry->chip_id, entry->patch_length, entry->start_offset);
            break;
        }
    }

    if (i == patch->number_of_total_patch) {
        RTK_ERROR("rtk_get_patch_entry: failed to get entry\n");
        free(entry);
        entry = NULL;
    }

    return entry;
}

uint16_t rtk_get_v1_final_fw(bt_hw_cfg_cb_t *cfg_cb)
{
    uint16_t fw_patch_len = -1;
    struct rtk_epatch_entry* entry = NULL;
    struct rtk_epatch *patch = (struct rtk_epatch *)cfg_cb->fw_buf;
    entry = rtk_get_patch_entry(cfg_cb);
    if (entry) {
        cfg_cb->total_len = entry->patch_length + cfg_cb->config_len;
    } else {
        cfg_cb->dl_fw_flag = 0;
    }

    RTK_DEBUG("total_len = 0x%x", cfg_cb->total_len);

    if (!(cfg_cb->total_buf = malloc(cfg_cb->total_len))) {
        RTK_DEBUG("Can't alloc memory for multi fw&config, errno:%d", errno);
        cfg_cb->dl_fw_flag = 0;
    } else {
        memcpy(cfg_cb->total_buf, cfg_cb->fw_buf + entry->start_offset, entry->patch_length);
        memcpy(cfg_cb->total_buf + entry->patch_length - 4, &patch->fw_version, 4);
        memcpy(&entry->svn_version, cfg_cb->total_buf + entry->patch_length - 8, 4);
        memcpy(&entry->coex_version, cfg_cb->total_buf + entry->patch_length - 12, 4);
        fw_patch_len = entry->patch_length;

        RTK_DEBUG("BTCOEX:20%06d-%04x svn_version:%u lmp_subversion:0x%x hci_version:0x%x hci_revision:0x%x chip_type:%d Cut:%d libbt-vendor version:%s, patch->fw_version = %x\n",
        ((entry->coex_version >> 16) & 0x7ff) + ((entry->coex_version >> 27) * 10000),
        (entry->coex_version & 0xffff), entry->svn_version, cfg_cb->lmp_subversion, cfg_cb->hci_version, cfg_cb->hci_revision, cfg_cb->chip_type, cfg_cb->eversion+1, RTK_VERSION, patch->fw_version);
    }

    if (cfg_cb->lmp_subversion == 0x1200) { //LMPSUBVERSION_8723a
        cfg_cb->lmp_sub_current = 0;
    } else {
        cfg_cb->lmp_sub_current = (uint16_t)patch->fw_version;
    }

    if (entry) {
        free(entry);
    }

    return fw_patch_len;
}

uint8_t rtk_insert_fw_patch_fragment_to_linklist(struct rtk_epatch_fragment *fragment,
        struct rtk_epatch_fragment_linklist **header)
{
    struct rtk_epatch_fragment_linklist *p = *header;
    struct rtk_epatch_fragment_linklist *q ;
    struct rtk_epatch_fragment_linklist *tmp;
    tmp = (struct rtk_epatch_fragment_linklist *)malloc(sizeof(*p));
    //ALOGI("rtk_insert_fw_patch_fragment_to_linklist ");
    if (!tmp) {
        RTK_DEBUG("rtk_insert_fw_patch_fragment_to_linklist: failed to allocate mem for patch entry");
        return -1;
    }

    tmp->fragment = fragment;
    tmp->next = NULL;
    if (*header == NULL) {
        *header = tmp;
        return 0;
    }

    if (tmp->fragment->priority < (*header)->fragment->priority) {
        tmp->next = *header;
        *header = tmp;
        return 0;
    }

    q = p ->next;
    while (p) {
         if (q) {
             if (tmp->fragment->priority < q->fragment->priority) {
                p->next = tmp;
                tmp->next = q;
                break;
             }
             p = q;
             q = p ->next;
         } else {
             p->next = tmp;
             break;
         }
    }

    return 0;
}

uint32_t rtk_get_fw_patch_link_list(bt_hw_cfg_cb_t *cfg_cb,
                  struct rtk_epatch_fragment_linklist **linklist, uint16_t chip_id)
{
    uint8_t res;
    uint16_t i,j;
    struct rtk_epatch_v2 * patch;
    struct rtk_epatch_section * section;
    struct rtk_epatch_fragment * fragment = NULL;
    struct rtk_epatch_fragment_linklist * link_header = NULL;
    struct rtk_epatch_fragment_linklist * tmp;

    uint8_t *p, *q, *data;
    uint32_t fw_patch_len = 0;
    bool to_add = 0;
    uint32_t sec_sig_cnt = 0;
    uint8_t key_id = cfg_cb->keyid;

    patch = (struct rtk_epatch_v2 *)cfg_cb->fw_buf;

    patch->number_of_section = rtk_le16_to_cpu(patch->number_of_section);

    RTK_DEBUG("rtk_get_fw_patch_link_list: fw_ver 0x%08x,  fw_ver_sub 0x%08x, number_of_section %d",
        rtk_le32_to_cpu(patch->fw_version), rtk_le32_to_cpu(patch->fw_version_sub), patch->number_of_section);

    p = cfg_cb->fw_buf + 20;
   //Traversal section
    for (i = 0; i < patch->number_of_section; i++) {
        section = (struct rtk_epatch_section *) p;
        section->opcode = rtk_le32_to_cpu(section->opcode);
        section->length = rtk_le32_to_cpu(section->length);

        //q point to rtk_epatch_fragment
        q = p + 12;
        // p point to next rtk_epatch_section
        p = p + 8 + section->length;
        if (section->length == 0)
            continue;

        section->number_of_fragment = rtk_le16_to_cpu(section->number_of_fragment);
        RTK_DEBUG("rtk_get_fw_patch_link_list: opcode: %d,  length:%d, number_of_fragment: %d",
           section->opcode, section->length, section->number_of_fragment);

        fragment = (struct rtk_epatch_fragment *) q;

        //Traversal patch fragment
        for (j = 0; j < section->number_of_fragment; j++) {
            if (section->opcode != FW_OTA_FLAG) {
                fragment->length = rtk_le32_to_cpu(fragment->length);
                RTK_DEBUG("rtk_get_fw_patch_link_list: chip_id: %d,  priority:%d, length:  0x%x",
                   fragment->chip_id, fragment->priority, fragment->length);
            }
            switch (section->opcode) {
                case FW_PATCH_SNIPPET:
                    if(fragment->chip_id == chip_id)
                        to_add = true;
                    q = q + 8 + fragment->length;
                    break;
                case FW_DUMMY_HEADER:
                    if((fragment->chip_id == chip_id) && (key_id == 0x00 || key_id == 0xff))
                        to_add = true;
                    q = q + 8 + fragment->length;
                    break;
                case FW_SECURITY_SIGNATURE:
                    if((fragment->chip_id == chip_id) && (fragment->key_id == key_id) ){
                        to_add = true;
                        sec_sig_cnt++;
                    }
                    q = q + 8 + fragment->length;
                    break;
                case FW_OTA_FLAG:
                    q = q + sizeof(struct rtk_epatch_ota);
                    break;
            }
            if (to_add) {
                res = rtk_insert_fw_patch_fragment_to_linklist(fragment, &link_header);
                if(res)
                   goto free_linklist;
                to_add = false;
                fw_patch_len += fragment->length;
                {
                    data = fragment->data;
                    RTK_DEBUG("fragment->data  %02x %02x %02x %02x %02x %02x %02x %02x",
                       *(data), *(data+1), *(data+2), *(data+3), *(data+4), *(data+5), *(data+6), *(data+7));
                    data = data; // bypass warnings
                }
            }
        }
    }

    if ((key_id != 0x00) && (key_id != 0xff) && (sec_sig_cnt == 0)) {
        p = cfg_cb->fw_buf + 20;
        for (i = 0; i < patch->number_of_section; i++) {
            section = (struct rtk_epatch_section *) p;
            section->opcode = rtk_le32_to_cpu(section->opcode);
            section->length = rtk_le32_to_cpu(section->length);
            section->number_of_fragment = rtk_le16_to_cpu(section->number_of_fragment);
            RTK_DEBUG("rtk_get_fw_patch_link_list 2: opcode: %d,  length:%d, number_of_fragment: %d",
                      section->opcode, section->length, section->number_of_fragment);

            q = p + 12;
            p = p + 12 + section->length;

            // Traversal patch fragment
            if (section->opcode == FW_DUMMY_HEADER) {
                for (j = 0; j < section->number_of_fragment; j++) {
                    fragment = (struct rtk_epatch_fragment *) q;
                    fragment->length = rtk_le32_to_cpu(fragment->length);
                    RTK_DEBUG("rtk_get_fw_patch_link_list: chip_id: %d, priority:%d, length:  0x%x",
                              fragment->chip_id, fragment->priority, fragment->length);

                    if (fragment->chip_id == chip_id) {
                        res = rtk_insert_fw_patch_fragment_to_linklist(fragment, &link_header);
                        if (res)
                            goto free_linklist;
                        to_add = false;
                        fw_patch_len += fragment->length;
                        {
                            data = fragment->data;
                            RTK_DEBUG("fragment->data  %02x %02x %02x %02x %02x %02x %02x %02x",
                                      *(data), *(data+1), *(data+2), *(data+3), *(data+4), *(data+5), *(data+6), *(data+7));
                        }
                    }
                    q = q + 8 + fragment->length;
                }
            }
        }
    }

    *linklist = link_header;
    return fw_patch_len;
free_linklist:
    while (link_header) {
        tmp = link_header;
        link_header = link_header->next;
        tmp->fragment = NULL;
        tmp->next = NULL;
        free(tmp);
    }

    return -1;
}

uint32_t rtk_get_v2_final_fw(bt_hw_cfg_cb_t *cfg_cb)
{
    uint8_t *p, *data;
    uint32_t fw_patch_len = -1;
    uint32_t fw_version, svn_version, coex_version;
    uint16_t chip_id = cfg_cb->eversion + 1;
    struct rtk_epatch_fragment_linklist *fw_patch_link = NULL;
    struct rtk_epatch_fragment_linklist * tmp;

    fw_patch_len = rtk_get_fw_patch_link_list(cfg_cb, &fw_patch_link, chip_id);
    if(fw_patch_len > 0) {
        cfg_cb->total_len = fw_patch_len + cfg_cb->config_len;
    } else {
        cfg_cb->dl_fw_flag = 0;
        while (fw_patch_link) {
            tmp = fw_patch_link;
            fw_patch_link = fw_patch_link->next;
            free(tmp);
        }
        return fw_patch_len;
    }

    RTK_DEBUG("fw_patch_len = 0x%x, total_len = 0x%x", fw_patch_len, cfg_cb->total_len);

    if (!(cfg_cb->total_buf = malloc(cfg_cb->total_len))) {
        RTK_DEBUG("Can't alloc memory for multi fw&config, errno:%d", errno);
        cfg_cb->dl_fw_flag = 0;
        fw_patch_len = -1;
    } else {
        p = cfg_cb->total_buf;
        tmp = fw_patch_link;
        //fw_patch_len = 0;
        RTK_DEBUG("copy patch fragment to total_buf");
        while (tmp) {
            if (tmp->fragment) {
                RTK_DEBUG("fragment->priority= %d, fragment->length = 0x%x",
                            tmp->fragment->priority, tmp->fragment->length);

                memcpy(p, tmp->fragment->data, tmp->fragment->length);
                p += fw_patch_link->fragment->length;
                //fw_patch_len += fw_patch_link->fragment->length;

                {
                    data = tmp->fragment->data;
                    RTK_DEBUG("fragment->data  %02x %02x %02x %02x %02x %02x %02x %02x",
                       *(data), *(data+1), *(data+2), *(data+3), *(data+4), *(data+5), *(data+6), *(data+7));
                    data = data; // bypass warnings
                }
            }
            tmp = tmp->next;
        }
    }

    memcpy(&fw_version, cfg_cb->total_buf + fw_patch_len - 4, 4);
    memcpy(&svn_version, cfg_cb->total_buf + fw_patch_len - 8, 4);
    memcpy(&coex_version, cfg_cb->total_buf + fw_patch_len - 12, 4);
    cfg_cb->lmp_sub_current = (uint16_t)fw_version;

    RTK_DEBUG("BTCOEX:20%06d-%04x svn_version:%u lmp_subversion:0x%x hci_version:0x%x hci_revision:0x%x chip_type:%d Cut:%d libbt-vendor version:%s, patch->fw_version = %x\n",
    ((coex_version >> 16) & 0x7ff) + ((coex_version >> 27) * 10000),
    (coex_version & 0xffff), svn_version, cfg_cb->lmp_subversion, cfg_cb->hci_version, cfg_cb->hci_revision, cfg_cb->chip_type, cfg_cb->eversion+1, RTK_VERSION, fw_version);

    while (fw_patch_link) {
        tmp = fw_patch_link;
        fw_patch_link = fw_patch_link->next;
        free(tmp);
    }

    return fw_patch_len;
}

void rtk_get_bt_usb_final_patch(bt_hw_cfg_cb_t *cfg_cb)
{
    uint8_t proj_id = 0;
    uint8_t res = 0;
    uint8_t parsing_rule = cfg_cb->parsing_rule; // 1: Legacy format, 2: New format
    uint32_t fw_patch_len = 0;

    /* check the extension section signature */
    if (memcmp(cfg_cb->fw_buf + cfg_cb->fw_len - 4, EXTENSION_SECTION_SIGNATURE, 4)) {
        RTK_DEBUG("check extension section signature error");
        cfg_cb->dl_fw_flag = 0;
        goto free_buf;
    }

    res = rtk_check_epatch_signature(cfg_cb, parsing_rule);
    if (res) {
        goto free_buf;
    }

    proj_id = rtk_get_fw_project_id(cfg_cb->fw_buf + cfg_cb->fw_len - 5);
    if (usb_project_id[proj_id] != cfg_cb->lmp_subversion_default) {
        RTK_DEBUG("usb_project_id is 0x%02x, fw project_id is %02x, does not match!!!",
                   usb_project_id[proj_id], cfg_cb->lmp_subversion_default);
        cfg_cb->dl_fw_flag = 0;
        goto free_buf;
    }

    if (1 == parsing_rule) {
        fw_patch_len = rtk_get_v1_final_fw(cfg_cb);
    } else if (2 == parsing_rule) {
        fw_patch_len = rtk_get_v2_final_fw(cfg_cb);
    }

    if (fw_patch_len <= 0) {
        goto free_buf;
    }

    RTK_DEBUG(" fw_patch_len = 0x%x ", fw_patch_len);
    if (cfg_cb->config_len) {
        memcpy(cfg_cb->total_buf+fw_patch_len, cfg_cb->config_buf, cfg_cb->config_len);
    }

    cfg_cb->dl_fw_flag = 1;
    RTK_DEBUG("Fw:%s exists, config file:%s exists", (cfg_cb->fw_len>0)?"":"not", (cfg_cb->config_len>0)?"":"not");

free_buf:
    if (cfg_cb->fw_len > 0) {
        free(cfg_cb->fw_buf);
        cfg_cb->fw_len = 0;
        cfg_cb->fw_buf = NULL;
    }

    if (cfg_cb->config_len > 0) {
        free(cfg_cb->config_buf);
        cfg_cb->config_len = 0;
        cfg_cb->config_buf = NULL;
    }
}
