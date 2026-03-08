/*
 * Copyright (c) 2023-2024 Amazon.com, Inc. or its affiliates.  All rights reserved.
 *
 */

#ifndef __FDT_AMZN_UTIL_H
#define __FDT_AMZN_UTIL_H

/**
 * Setup fdt node used to pass uboot log to kernel
 *
 * @param blob		FDT blob to update
 */
void ft_uboot_log_setup(void *blob);

#endif
