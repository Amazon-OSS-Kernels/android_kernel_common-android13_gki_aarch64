/*
 * Copyright (c) 2024 Amazon.com, Inc. or its affiliates.  All rights reserved.
 *
 */

#include <ld_bt.h>

#if !(CONFIG_MINIUBOOT)
U_BOOT_CMD(
    setRtkBT, CONFIG_SYS_MAXARGS, 1, do_setRtkBT,
    "load rtk bt patch, and set woble\n",
    NULL
);
#endif
