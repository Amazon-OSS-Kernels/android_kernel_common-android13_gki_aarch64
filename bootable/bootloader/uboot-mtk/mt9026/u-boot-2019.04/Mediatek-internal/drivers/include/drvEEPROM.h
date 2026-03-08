/* SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause) */
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#ifndef EEPROM_IF_H
#define EEPROM_IF_H

//---------------------------------------------------------------------------
// Include files
//---------------------------------------------------------------------------
//#include "wrap.h"
#include <MsTypes.h>
#include <command.h>

//---------------------------------------------------------------------------
#define ASSERT(cond) do { if (!(cond)) { printf("%s:%s[%d] ASSERT(" #cond ") FAIL!!\n", __FILE__, __func__, __LINE__); } } while (0)
#define pr_warn(fmt, ...)      do { printf("[%s:%d]: " fmt, __func__, __LINE__, ##__VA_ARGS__); } while (0)
//-----------------------------------------------------------------------------
// Type definitions
//-----------------------------------------------------------------------------
#if 1
#ifndef CC_LDR_ENV_OFFSET
//#define CC_LDR_ENV_OFFSET       (DRAM_START + 0xf000) // use 60k~64k to save loader env.
#define CC_LDR_ENV_OFFSET       (&g_LdrEnv)
#endif

typedef struct
{
    char    szCustomer[16];
    char    szMagic[16];
    char    szModel[32];
    char    szVersion[64];
} LDR_ENV_HDR_T;

typedef union
{
    unsigned char           au1Raw[128];
    LDR_ENV_HDR_T   rHdr;
} LDR_ENV_HDR_UNION_T;

typedef struct
{
    unsigned int u4LoaderPlainSize;
    unsigned int u4LoaderLzhsSize;
    unsigned int u4LoaderLzhsAddress;
    unsigned int u4CfgFlag;
    //0x010
    LDR_ENV_HDR_UNION_T u;      //128byte
    //0x090
    unsigned char au1DtvCfg[16];        // DTVCFG_T use 9bytes
    //0x0a0
    unsigned int au4CustKey[64];      // 2048bit key
    //0x0e0
    unsigned char aes_key2[16];         // preloader -> loader
    unsigned char aes_iv2[16];          // preloader -> loader
} LDR_ENV_T;

extern LDR_ENV_T g_LdrEnv;

#endif

typedef enum __EEPDTV_Offset_T{
    EEPDTV_DTVCFG_MAGIC1 = 0,
} EEPDTV_Offset_T;

// DTVCFG_T Offset 0.
typedef struct __DtvCfg {
//    MS_U8 MsOffset[16];
    MS_U8 u1Magic1;         //< MTK fix value = 0x53.
    MS_U8 u1Magic2;         //< MTK fix value = 0x81.
    MS_U8 u1Flags;          //< MTK internal setting value.
    MS_U8 u1Upgrade;
    MS_U8 u1Upgrade_Ext;
    MS_U8 u1Flags2;
    MS_U8 u1Flags3;
    MS_U8 u1Flags4;
#ifdef CC_S_PLATFORM
    MS_U8 u1Flags5;
    MS_U8 u1Flags6;
#endif
    MS_U8 u1Flags7;
    MS_U8 u1Checksum;
} DTVCFG_T;

// DTVCFG parameter macro.
#define DTVCFG_MAGIC_CHAR1                  (0x53)
#define DTVCFG_MAGIC_CHAR2                  (0x81)

// DTVCFG flag for system configuration.
#define DTVCFG_FLAG_POWERON_BOOT            (1U << 0)   /* Map to PC_MISC_CTRL_NO_STANDBY, if 1, just boot, otherwise standby if ext_status on */
#define DTVCFG_FLAG_FACTORY_MODE            (1U << 1)   /* Map to PC_MISC_CTRL_FACTORY_MODE */
#define DTVCFG_FLAG_POWERON_LAST            (1U << 2)   /* Map to PC_MISC_CTRL_NO_STANDBY_2, if 1, read system_state_on to decide boot or standby */
#define DTVCFG_FLAG_8280_UPGRADE_ON         (1U << 3)   /* Map to PC_MISC_CTRL_COMPANION_CHIP_UPGRADE */
#define DTVCFG_FLAG_POWER_BTN_ON            (1U << 4)   /* Use to record power button count, 1 is on, 0 is off */
#define DTVCFG_FLAG_SYSTEM_STATE_ON         (1U << 5)   /* Use to record previous system state, 1 is on, 0 is off. */
#define DTVCFG_FLAG_POWER_LEDOFF            (1U << 6)   /* Map to PC_MISC_CTRL_POWER_LED_OFF */
#define DTVCFG_FLAG_USB_UPGRADE_ON          (1U << 7)   /* Map to PC_MISC_CTRL_FIRMWARE_UPGRADE */

// DTVCFG flags2 for system configuration.
#define DTVCFG_FLAG2_IMAGE_BANK             (1U << 0)
#define DTVCFG_FLAG2_8032UP_BANK            (1U << 1)
#define DTVCFG_FLAG2_QUIET_BOOT             (1U << 2)   /* Map to PC_MISC_CTRL_RTC_QUIET_BOOT, if 1, set quiet boot. */
#define DTVCFG_FLAG2_DISABLE_LOGO           (1U << 3)   /* Use to record logo display state, if 1, disable logo. otherwise show logo */
#define DTVCFG_FLAG2_FLIP_ON                (1U << 4)   /* FLIP_ENABLE */
#define DTVCFG_FLAG2_MIRROR_ON              (1U << 5)   /* MIRROR_ENABLE*/
#define DTVCFG_FLAG2_FP_LOCK                (1U << 6)   /* FP_LOCK, default 0. if 1 keypad cannot wakeup from standby. */
#define DTVCFG_FLAG2_UPGRADE_GO	            (1U << 7)   /* Use to record the usb upgrade status. */

/// DTVCFG flags3 for system configuration.
#define DTVCFG_FLAG3_KERNEL_BANK                    (1U << 0)
#define DTVCFG_FLAG3_ROOTFS_BANK                    (1U << 1)
#define DTVCFG_FLAG3_FAST_BOOT                      (1U << 2)
#define DTVCFG_FLAG3_NORMAL_BOOT_WITH_SUSPEND       (1U << 3)
#define DTVCFG_FLAG3_NORMAL_BOOT_WITHOUT_SUSPEND    (1U << 4)
#define DTVCFG_FLAG3_SUPPORT_CLI                    (1U << 5)
#define DTVCFG_FLAG3_WAKEUP_ON_LAN                  (1U << 6) /* Use to record WOL state, 1 is on, 0 is off */
#define DTVCFG_FLAG3_AMBER_LED_ON                   (1U << 7)

#if defined(CC_SALVIA_CUST_DRV)||defined(CC_SAKURA_CUST_DRV)||defined(CUST_SOEM_DRV)
#define DTVCFG_FLAG4_POWERON_CONFIGURED             (1U << 0)  /* PC_MISC_CTRL_NO_STANDBY or PC_MISC_CTRL_NO_STANDBY_2 is called */
#define DTVCFG_FLAG4_DISABLE_LOGO_MENU_SET          (1U << 1)  /* Save/Restore the LOGO DISABLE setting when ON/OFF hotel mode*/ 
#define DTVCFG_FLAG4_USB_UART_STATUS                (1U << 2)
#define DTVCFG_FLAG4_OAD_STANDBY                    (1U << 3)  /*after an OAD upgrade, before reboot set this flag to power on to standby*/
#define DTVCFG_FLAG4_HOTEL_MODE                     (1U << 4)  /*to record if the TV in or not in hotel mode*/
#endif
#ifndef CC_S_PLATFORM
#define DTVCFG_FLAG4_HOTEL_MODE                     (1U << 4) /*to record if the TV in or not in hotel mode*/
#endif
#define DTVCFG_FLAG4_WAKEUP_ON_WLAN                 (1U << 5)   /*Use to record WOWLan state, 1 is on, 0 is off */
#define DTVCFG_FLAG4_FAST_CLOCK                     (1U << 6)   /* Use to record fast clock state, 1 is on, 0 is off */
//joe_20140328
#define DTVCFG_FLAG4_QUIET_HOT_BOOT                 (1U << 7)

#ifdef CC_S_PLATFORM
/// DTVCFG flags5 for customization.
#ifndef DTVCFG_FLAG5_CUSTOM_FEATURE00_SUPPORT
#define DTVCFG_FLAG5_CUSTOM_FEATURE00_SUPPORT       (1U << 0)
#endif
#ifndef DTVCFG_FLAG5_CUSTOM_FEATURE01_SUPPORT
#define DTVCFG_FLAG5_CUSTOM_FEATURE01_SUPPORT       (1U << 1)
#endif
#ifndef DTVCFG_FLAG5_CUSTOM_FEATURE02_SUPPORT
#define DTVCFG_FLAG5_CUSTOM_FEATURE02_SUPPORT       (1U << 2)
#endif
#ifndef DTVCFG_FLAG5_CUSTOM_FEATURE03_SUPPORT
#define DTVCFG_FLAG5_CUSTOM_FEATURE03_SUPPORT       (1U << 3)
#endif
#ifndef DTVCFG_FLAG5_CUSTOM_FEATURE04_SUPPORT
#define DTVCFG_FLAG5_CUSTOM_FEATURE04_SUPPORT       (1U << 4)
#endif
#ifndef DTVCFG_FLAG5_CUSTOM_FEATURE05_SUPPORT
#define DTVCFG_FLAG5_CUSTOM_FEATURE05_SUPPORT       (1U << 5)
#endif
#ifndef DTVCFG_FLAG5_CUSTOM_FEATURE06_SUPPORT
#define DTVCFG_FLAG5_CUSTOM_FEATURE06_SUPPORT       (1U << 6)
#endif
#ifndef DTVCFG_FLAG5_CUSTOM_FEATURE07_SUPPORT
#define DTVCFG_FLAG5_CUSTOM_FEATURE07_SUPPORT       (1U << 7)
#endif
#endif

#if 1

// 00: normal(no QHB), 01: QHB, 02-03 reserved
#define DTVCFG_FLAG7_BOOT_MODE_BIT0 (1U << 6)
#define DTVCFG_FLAG7_BOOT_MODE_BIT1 (1U << 7)

#ifndef DRVCUST_EEPROM_DTV_DRIVER_OFFSET
#define DRVCUST_EEPROM_DTV_DRIVER_OFFSET    ((unsigned int)16)
#endif

#ifndef DRVCUST_EEPROM_DTV_DRIVER_SIZE
#define DRVCUST_EEPROM_DTV_DRIVER_SIZE      ((unsigned int)96)
#endif

#define EEPROM_DTV_DRIVER_SIZE              0x60 //DRVCUST_EEPROM_DTV_DRIVER_SIZE
#define EEPROM_DTV_DRIVER_OFFSET            0x10 //DRVCUST_EEPROM_DTV_DRIVER_OFFSET

//-----------------------------------------------------------------------------
// Public functions
//-----------------------------------------------------------------------------
extern int EEPROM_Read(unsigned long long u8Offset, unsigned long u4MemPtr, unsigned int u4MemLen);
extern int EEPROM_Write(unsigned long long u8Offset, unsigned long u4MemPtr, unsigned int u4MemLen);

extern int EEPDTV_Read(unsigned long long u8Offset, unsigned char * u4MemPtr, unsigned int u4MemLen);
extern int EEPDTV_Write(unsigned long long u8Offset, unsigned char * u4MemPtr, unsigned int u4MemLen);

extern int EEPDTV_GetCfg(DTVCFG_T* prDtvCfg);
extern int EEPDTV_SetCfg(DTVCFG_T* prDtvCfg);
#endif // if 0
MS_BOOL EEPRON_IsQuiteHotBootMode(void) ;
MS_BOOL EEPRON_IsMemoryStandbyMode(void) ;
MS_BOOL EEPRON_IsOADMode(void);
MS_BOOL EEPRON_ClearOADMode(void);

#endif

