// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#ifndef _VENDOR_H_
#define _VENDOR_H_

#define FCIC_CHECKSUM_SIZE       4
#define FCIC_CRC_SIZE            4
#define FCIC_EXTRA_SIZE          (FCIC_CHECKSUM_SIZE + FCIC_CRC_SIZE)

#define LGD_MURA_START_ADR       0x69000      // Just for filename test
#define LGD_MURA_DAT_OFFSET      0x00004
#define LGD_CHECKSUM_ADR         0x2662f4

#define ID_CUS_LGD               0x0800

#define SAMPLE_DAT_START         1024     // Byte
#define SAMPLE_DAT_LEN           4096     // Byte


#endif  /* _VENDOR_H_ */