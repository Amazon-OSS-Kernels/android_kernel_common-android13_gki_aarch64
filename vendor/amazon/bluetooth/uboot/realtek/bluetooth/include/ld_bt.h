/*
 * Copyright (c) 2024 Amazon.com, Inc. or its affiliates.  All rights reserved.
 *
 */

#ifndef __LD_BT_H
#define __LD_BT_H

#include <common.h>
#include <command.h>
#include <debug_impl.h>
#include <usb.h>
#include <dm.h>
#include <dm/uclass.h>
#include <utility.h>
#include <fs.h>
#ifdef CONFIG_MULTICORES_PLATFORM
#include <smp/thread_info.h>
extern smp_spin_lock_t fs_spin_lock;
#endif

#include <ld_bt_main.h>
#include <ld_bt_rtk.h>

#endif  /* #ifndef __LD_BT_H */
