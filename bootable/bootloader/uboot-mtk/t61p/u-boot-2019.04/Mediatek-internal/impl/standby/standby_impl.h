/* SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause) */
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#ifndef _STANDBY_IMPL_HEADER_
#define _STANDBY_IMPL_HEADER_

#define PMU_BASE_ADDR                       0x1C000000UL
#define PMU_DUMMY_ADDR                      0x100UL
#define PMU_DUMMY_OFFSET_IR_KEYPAD          0x0      // IR or keypad key code
#define PMU_DUMMY_OFFSET_BOOT_STATUS        0x1      // boot status
#define PMU_DUMMY_OFFSET_IR_WAKEUP          0x2      // IR wakeup key
#define PMU_REG_ACCESS(addr,idx)            *(volatile unsigned short*)(PMU_BASE_ADDR + ((addr<<9) + (idx<<2)))
/* Remote boot state . */
#define REMOTE_STATE_UBOOT		(0x00) /*  */
#define REMOTE_STATE_SECOND_STANDBY		(0x01) /*  */
#define REMOTE_STATE_KERNEL		(0x02) /*  */

int mtk_standby_mode_framework(void);
#endif /* _STANDBY_IMPL_HEADER_ */

