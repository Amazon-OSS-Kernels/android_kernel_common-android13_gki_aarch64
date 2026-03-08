/* SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause) */
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#ifndef _PARTITION_H_
#define _PARTITION_H_

#define DEFAULT_PAR_STR_LEN 500

#if (CONFIG_AB_SIDELOAD == 1)
char default_par_info[]      =  "name=mboot_a,size=10M;" \
                                "name=mboot_b,size=10M;" \
                                "name=uenv,size=16M";
#else // reserve space even no AB
char default_par_info[]      =  "name=mboot_a,size=10M;" \
                                "name=mboot_dummy,size=10M;" \
                                "name=uenv,size=16M";
#endif
char usb_default_par_info[]  =  "name=sboot,size=1M;" \
                                "name=mboot,size=10M;" \
                                "name=uenv,size=16M";

char part_mboot_a[]          =  "mboot_a";
char part_mboot_b[]          =  "mboot_b";
char part_uenv[]             =  "uenv";

#endif
