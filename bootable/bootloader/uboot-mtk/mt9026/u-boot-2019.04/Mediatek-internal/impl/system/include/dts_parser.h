/* SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause) */
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#ifndef _MTK_DTS_PARSER_HEADER_
#define _MTK_DTS_PARSER_HEADER_

#define DTS_SHIFT_32_BIT  32
#define PWM_MAX_CH   8

struct fdt_content
{
    char *field;
    int len;
    __be32 *attribute;
};

struct dts_mmap
{
    unsigned long long address;
    unsigned long long size;
};

void integer_dt_parser(struct fdt_content *fdt_data, void *out, const char *field_target);
void string_dt_parser(struct fdt_content *fdt_data, void *out, const char *field_target);
void mmap_dt_parser(struct fdt_content *fdt_data, void *out, const char *field_target);
void jpd_dt_parser(struct fdt_content *fdt_data, void *out, const char *field_target);
void gop_dt_parser(struct fdt_content *fdt_data, void *out, const char *field_target);
void pwm_dt_parser(struct fdt_content *fdt_data, void *out, const char *field_target);
void panel_dt_parser(struct fdt_content *fdt_data, void *out, const char *field_target);
void cus_panel_dt_parser(struct fdt_content *fdt_data, void *out, const char *field_target);
void cus_demura_dt_parser(struct fdt_content *fdt_data, void *out, const char *field_target);
#ifdef CONFIG_ENABLE_CUST_IC_UPDATE
void cust_pmic_dt_parser(struct fdt_content *fdt_data, void *out, const char *field_target);
void cust_pgamma_dt_parser(struct fdt_content *fdt_data, void *out, const char *field_target);
void cust_levelshift_dt_parser(struct fdt_content *fdt_data, void *out, const char *field_target);
void cust_vcomic_dt_parser(struct fdt_content *fdt_data, void *out, const char *field_target);

void cust_pmic_sub_dt_parser(struct fdt_content *fdt_data, void *out, const char *field_target);
void cust_pgamma_sub_dt_parser(struct fdt_content *fdt_data, void *out, const char *field_target);
void cust_levelshift_sub_dt_parser(struct fdt_content *fdt_data, void *out, const char *field_target);
void cust_vcomic_sub_dt_parser(struct fdt_content *fdt_data, void *out, const char *field_target);

void cust_tcon_dt_parser(struct fdt_content *fdt_data, void *out, const char *field_target);
#endif
void panel_spi_bus_info_dt_parser(struct fdt_content *fdt_data, void *out, const char *field_target);
void ldm_dt_parser(struct fdt_content *fdt_data, void *out, const char *field_target);

void board_dt_parser(struct fdt_content *fdt_data, void *out, const char *field_target);
void misc_dt_parser(struct fdt_content *fdt_data, void *out, const char *field_target);
int parse_dt(const char *tag_target, void (*parser_cb)(struct fdt_content *, void*, const char*), void *out, const char *field_target);

#endif
