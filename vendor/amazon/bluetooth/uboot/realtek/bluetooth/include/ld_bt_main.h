/*
 * Copyright (c) 2024 Amazon.com, Inc. or its affiliates.  All rights reserved.
 *
 */

#ifndef __LD_BT_MAIN_H
#define __LD_BT_MAIN_H

typedef struct usb_device *bt_ctlr_t;

// Each vendor needs to implement these functions
typedef struct {
    int (*init)(bt_ctlr_t udev);
    int (*set_woble)(bt_ctlr_t udev);
    int (*clean_up)(bt_ctlr_t udev);
} ld_bt_func_t;

// Each vendor needs to implement a function below to setup function pointers
extern void ld_bt_func_init_rtk(ld_bt_func_t *ptr);
extern void ld_bt_func_init_mtk(ld_bt_func_t *ptr);

unsigned char *ld_bt_load_file(char *name, char *path, int *length);

int do_setRtkBT(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[]);

#endif  /* #ifndef __LD_BT_MAIN_H */
