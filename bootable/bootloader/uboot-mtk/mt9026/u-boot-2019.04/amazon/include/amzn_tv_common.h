/*
 * Copyright (c) 2023 Amazon.com, Inc. or its affiliates.  All rights reserved.
 *
 */

#ifndef __AMZN_TV_COMMON_H
#define __AMZN_TV_COMMON_H

#if defined(UFBL_FEATURE_IDME)
void idme_get_oem_data_field(const char *item, char *buf, unsigned buf_len);
#endif

#if defined(CONFIG_DIAG_TRANSITION_DIALOG)
int initr_diag_fos_trans_screen(void);
#endif

#endif
