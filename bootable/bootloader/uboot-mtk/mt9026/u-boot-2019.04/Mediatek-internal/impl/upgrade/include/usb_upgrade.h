/* SPDX-License-Identifier: GPL-2.0-only OR BSD-3-Clause */
/******************************************************************************
 *
 * This file is provided under a dual license.  When you use or
 * distribute this software, you may choose to be licensed under
 * version 2 of the GNU General Public License ("GPLv2 License")
 * or BSD License.
 *
 * GPLv2 License
 *
 * Copyright(C) 2019 MediaTek Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See http://www.gnu.org/licenses/gpl-2.0.html for more details.
 *
 * BSD LICENSE
 *
 * Copyright(C) 2019 MediaTek Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *  * Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *****************************************************************************/

#ifndef _API_USB_UPGRADE_H_
#define _API_USB_UPGRADE_H_
#include <common.h>
#include <MsTypes.h>


#define BIT0   0x01
#define BIT1   0x02
#define BIT2   0x04
#define BIT3   0x08
#define BIT4   0x10
#define BIT5   0x20
#define BIT6   0x40
#define BIT7   0x80

#define BIT8   0x0100
#define BIT9   0x0200
#define BIT10  0x0400
#define BIT11  0x0800
#define BIT12  0x1000
#define BIT13  0x2000
#define BIT14  0x4000
#define BIT15  0x8000 

#define BIT16  0x00010000
#define BIT17  0x00020000
#define BIT18  0x00040000
#define BIT19  0x00080000
#define BIT20  0x00100000
#define BIT21  0x00200000
#define BIT22  0x00400000
#define BIT23  0x00800000

#define BIT24  0x01000000
#define BIT25  0x02000000
#define BIT26  0x04000000
#define BIT27  0x08000000
#define BIT28  0x10000000
#define BIT29  0x20000000
#define BIT30  0x40000000
#define BIT31  0x80000000

#define DEFAULT_BIN_PATH     "/upgrade_image_no_tvcertificate.pkg"
#define DEFAULT_SCRIPT_PATH  "/usb_auto_update.txt"
#define AP_BIN_PATH          env_get("ForceUpgradePath")==NULL?DEFAULT_BIN_PATH:env_get("ForceUpgradePath")
#define SCRIPT_PATH          env_get("ForceUpgradePath")==NULL?DEFAULT_SCRIPT_PATH:env_get("ForceUpgradePath")

#define ENV_USB_AUTO_UPGRADE "usb_auto_upgrade"


#if (CONFIG_USB_UPGRADE == 1)
int usb_upgrade_pkg(void);
#if defined(CONFIG_MTK_INTERNAL_USB_UPGRADE_SCRIPPT)
int usb_upgrade_script(void);
#endif

#if (CONFIG_USB_AUTO_UPGRADE == 1)
//--------------------------------------------------------------------------------------------------
/// get auto-upgrade flag in uboot's env. variable, and decide whether to do usb auto-upgrade or not
/// if usb_auto_upgrade=disable_once, not need to do usb upgrade next time
/// if usb_auto_upgrade=disable, not need to do usb upgrade forever
/// if usb_auto_upgrade=NULL or enable or others, do usb upgrade this time
///
/// @return : 0 = Success , 1 = Failure
//-------------------------------------------------------------------------------------------------
int do_check_usb_auto_upgrade(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);
#endif

#endif

#endif

