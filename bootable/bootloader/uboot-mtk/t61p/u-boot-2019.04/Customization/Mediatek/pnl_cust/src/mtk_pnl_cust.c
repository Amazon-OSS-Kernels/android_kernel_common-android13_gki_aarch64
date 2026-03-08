// SPDX-License-Identifier: (GPL-2.0 OR BSD-3-Clause)
/*
 * MediaTek Panel cust driver
 *
 * Copyright (c) 2020 MediaTek Inc.
 */

#include <common.h>
#include <utility.h>
#include <environment.h>
#include "mtk_pnl_cust.h"
#include <i2c.h>
#include <asm/gpio.h>
#include <dm/uclass.h>
#include <dm/uclass-id.h>
#include <debug_impl.h>
#include <iniutility.h>
#include "mtk_tcon_common.h"
#include <display.h>
#include <mtk_panel.h>
#include <ms_utils.h>
#include <mtk-pm.h>

#ifndef TRUE
#define TRUE                        1
#endif
#ifndef FALSE
#define FALSE                       0
#endif

#define FILE_CUS_PARTITION          "CusFilePart"
#define FILE_DEFAULT_PARTITION_1    "tvconfig"
#define FILE_DEFAULT_PARTITION_2    "bootdata"
#define FILE_FIXED_FOLDER           ""
#define FILE_PATH_LENGTH            (128)
#define PANEL_DLG_PARTITION_PATH    "persist"
#define PANEL_DLG_INI_PATH          "panel_mode_cfg.ini"

#define CUST_IC_PRINT_LEN           (16)

#define H_K_C_FORMAT_TYPE_LEN       3
#define H_K_C_FORMAT_TYPE2_LEN       31
#define H_K_C_FORMAT_TYPE3_LEN       31
#define COST_FORMAT_TYPE1_LEN       42  //use CS602
#define COST_FORMAT_TYPE2_LEN       48
#define COST_FORMAT_TYPE3_LEN       48
#define CHOT_FORMAT_TYPE_LEN       25

#define COST_GAMMA_DATA_ADDR1       0xFF000
#define COST_GAMMA_DATA_ADDR2       0xFE000
#define COST_GAMMA_DATA_ADDR3       0x0F00

#define HKC_GAMMA_DATA_ADDR         0x00
#define CSOT_AUTOPGAMMA_EEPROM_ADDR 0xA0

#define AUTOPGAMMA_BUFFER_SIZE      512

#define AUTO_PANEL_GAMMA_DATA_ADDR         (0xEB000)
#define AUTO_PANEL_GAMMA_DATA_SIZE         (1024*2*3)  // 1024 Entry, 2 Byte, RGB
#define AUTO_PANEL_GAMMA_CRC_SIZE          (2)
#define AUTO_PANEL_GAMMA_TOTAL_SIZE        (AUTO_PANEL_GAMMA_DATA_SIZE + AUTO_PANEL_GAMMA_CRC_SIZE)
#define PANEL_GAMMA_MAINHEADER_LEN         (32)
#define PANEL_GAMMA_PRE_CHANNEL_LEN        (1024*2)

//auto update p2p cmd
#define AUTO_P2P_CMD_BUFFER_SIZE     512
#define ISP_HKC_GAMMA_ADDR                 (0x1000)
#define ISP_HKC_GAMMA_DATA_SIZE            (14)
#define ISP_HKC_GAMMA_TOTAL_SIZE           (16)

#define CSPI_GAMMA_ADDR                    (0xFF000 + 0x30)
#define CSPI_GAMMA_DATA_SIZE               (14)
#define CSPI_GAMMA_TOTAL_SIZE              (16)

#define USIT_GAMMA_ADDR                   (0xFF000 + 0x30)
#define USIT_GAMMA_DATA_SIZE              (14)
#define USIT_GAMMA_TOTAL_SIZE             (16)

#define CS602_GLD0_ADDR                    (0x0A) /* [4:0] */
#define CS602_GAMMA_ADDR_START             (0x10)
#define CS602_GAMMA_ADDR_END               (0x24)
#define CS602_VCOM1_ADDR_START             (0x25)
#define CS602_VCOM1_ADDR_END               (0x26)
#define CS602_VCOM2_ADDR_START             (0x26)
#define CS602_VCOM2_ADDR_END               (0x27)

#define CHOT_GAMMA_ADDR_START             (0x00)
#define CHOT_GAMMA_ADDR_END               (0x14)
#define CHOT_VCOM1_ADDR_START             (0x15)
#define CHOT_VCOM1_ADDR_END               (0x16)

#define HKC_GAMMA_ADDR_START             (0x03)
#define HKC_GAMMA_ADDR_END               (0x1E)
#define HKC_VCOM_ADDR             (0x02)
#define HKC_VCOM_MAX    7500 //7.80V
#define HKC_VCOM_MIN    5000 //5.00V

#define HKC_TYPE3_REG_GAMMA_START			(0x13)
#define HKC_TYPE3_REG_GAMMA_END				(0x27)

// Defined by the register map in the TC901 data sheet. These are the
// GAMMA and VCOM locations in the PMIC. These are used for the overrides
// of the auto pgamma that is stored in the panel memory's EERPOM.
#define TC901_AVDD_REG     0x09 //[5:0]
#define TC901_VCOM_MAX_REG 0x0E //[6:0]
#define TC901_VCOM_MIN_REG 0x0F //[6:0]
#define TC901_VCOM1_REG    0x10 //[6:0]
#define TC901_VCOM2_REG    0x11 //[6:0]
#define TC901_GAMMA_REGS   0x13 //from 0x13 to 0x27, 21 bytes for 14 GAMMAs
#define TC901_GAMMA_SIZE   21

static unsigned char *pmic_bin_buf          = NULL;
static unsigned char *pmic_sub_bin_buf      = NULL;
static unsigned char *pgamma_bin_buf        = NULL;
static unsigned char *pgamma_sub_bin_buf    = NULL;
static unsigned char *ls_bin_buf            = NULL;
static unsigned char *ls_sub_bin_buf        = NULL;
static unsigned char *vcomic_bin_buf        = NULL;
static unsigned char *vcomic_sub_bin_buf    = NULL;

static unsigned short pmic_size             = 0;
static unsigned short pmic_sub_size         = 0;
static unsigned short pgamma_size           = 0;
static unsigned short pgamma_sub_size       = 0;
static unsigned short ls_size               = 0;
static unsigned short ls_sub_size           = 0;
static unsigned short vcomic_size           = 0;
static unsigned short vcomic_sub_size       = 0;

static unsigned int iic_bus = 0;
static struct udevice *i2c_cur_dev = NULL;

unsigned int g_TCONLESS_FORCE_RESET_FLAG = 0;

#define UBOOT_DUMP_FORCE(addr, size)\
    do{\
        printf("\033[0;34m[DUMP] %s:%d:\033[0m\n",__FUNCTION__,__LINE__);\
        _dump(addr,size);\
    }while(0)

//G(x) = X16 + X15 + X2 + 1
//CRC16 G(x): 0x8005
const MS_U16 CSOT_CRC16_TABLE[256] = {
    0x0000, 0x8005, 0x800F, 0x000A, 0x801B, 0x001E, 0x0014, 0x8011, 0x8033, 0x0036, 0x003C, 0x8039, 0x0028, 0x802D, 0x8027, 0x0022,
    0x8063, 0x0066, 0x006C, 0x8069, 0x0078, 0x807D, 0x8077, 0x0072, 0x0050, 0x8055, 0x805F, 0x005A, 0x804B, 0x004E, 0x0044, 0x8041,
    0x80C3, 0x00C6, 0x00CC, 0x80C9, 0x00D8, 0x80DD, 0x80D7, 0x00D2, 0x00F0, 0x80F5, 0x80FF, 0x00FA, 0x80EB, 0x00EE, 0x00E4, 0x80E1,
    0x00A0, 0x80A5, 0x80AF, 0x00AA, 0x80BB, 0x00BE, 0x00B4, 0x80B1, 0x8093, 0x0096, 0x009C, 0x8099, 0x0088, 0x808D, 0x8087, 0x0082,
    0x8183, 0x0186, 0x018C, 0x8189, 0x0198, 0x819D, 0x8197, 0x0192, 0x01B0, 0x81B5, 0x81BF, 0x01BA, 0x81AB, 0x01AE, 0x01A4, 0x81A1,
    0x01E0, 0x81E5, 0x81EF, 0x01EA, 0x81FB, 0x01FE, 0x01F4, 0x81F1, 0x81D3, 0x01D6, 0x01DC, 0x81D9, 0x01C8, 0x81CD, 0x81C7, 0x01C2,
    0x0140, 0x8145, 0x814F, 0x014A, 0x815B, 0x015E, 0x0154, 0x8151, 0x8173, 0x0176, 0x017C, 0x8179, 0x0168, 0x816D, 0x8167, 0x0162,
    0x8123, 0x0126, 0x012C, 0x8129, 0x0138, 0x813D, 0x8137, 0x0132, 0x0110, 0x8115, 0x811F, 0x011A, 0x810B, 0x010E, 0x0104, 0x8101,
    0x8303, 0x0306, 0x030C, 0x8309, 0x0318, 0x831D, 0x8317, 0x0312, 0x0330, 0x8335, 0x833F, 0x033A, 0x832B, 0x032E, 0x0324, 0x8321,
    0x0360, 0x8365, 0x836F, 0x036A, 0x837B, 0x037E, 0x0374, 0x8371, 0x8353, 0x0356, 0x035C, 0x8359, 0x0348, 0x834D, 0x8347, 0x0342,
    0x03C0, 0x83C5, 0x83CF, 0x03CA, 0x83DB, 0x03DE, 0x03D4, 0x83D1, 0x83F3, 0x03F6, 0x03FC, 0x83F9, 0x03E8, 0x83ED, 0x83E7, 0x03E2,
    0x83A3, 0x03A6, 0x03AC, 0x83A9, 0x03B8, 0x83BD, 0x83B7, 0x03B2, 0x0390, 0x8395, 0x839F, 0x039A, 0x838B, 0x038E, 0x0384, 0x8381,
    0x0280, 0x8285, 0x828F, 0x028A, 0x829B, 0x029E, 0x0294, 0x8291, 0x82B3, 0x02B6, 0x02BC, 0x82B9, 0x02A8, 0x82AD, 0x82A7, 0x02A2,
    0x82E3, 0x02E6, 0x02EC, 0x82E9, 0x02F8, 0x82FD, 0x82F7, 0x02F2, 0x02D0, 0x82D5, 0x82DF, 0x02DA, 0x82CB, 0x02CE, 0x02C4, 0x82C1,
    0x8243, 0x0246, 0x024C, 0x8249, 0x0258, 0x825D, 0x8257, 0x0252, 0x0270, 0x8275, 0x827F, 0x027A, 0x826B, 0x026E, 0x0264, 0x8261,
    0x0220, 0x8225, 0x822F, 0x022A, 0x823B, 0x023E, 0x0234, 0x8231, 0x8213, 0x0216, 0x021C, 0x8219, 0x0208, 0x820D, 0x8207, 0x0202
};

static MS_U16 CSOT_Cal_CRC16(MS_U8 *pData,MS_U32 len)
{
    MS_U8 dat;
    MS_U16 crcData = 0x0000;
    while(len--){
        dat = (MS_U8)(crcData>>8);
        crcData <<= 8;
        crcData ^= CSOT_CRC16_TABLE[dat^*pData++];
    }

    return crcData;
}

static MS_U16 HKC_Cal_CRC16(MS_U8 *pData,MS_U32 len)
{
    unsigned int i=0, j=0;
    unsigned short crc = 0x0000;
    unsigned short crc_temp_1=0,crc_temp_2=0,crc_temp_3=0;
    unsigned char data_buffer=0;
    unsigned int checksum = 0;

    while(len--){
        data_buffer = pData[i++];
        checksum += data_buffer;
        crc_temp_1 = 0;
        crc_temp_2 = 0;
        crc_temp_3 = 0;

        for(j=0;j<8;j++){
            crc_temp_1 = ((crc>>15)^data_buffer)&0x0001; // result 1 CRC MSB[15] ^ DATA BIT0
            crc_temp_2 = ((crc>>1)^crc_temp_1)&0x0001; // result 2 --> CRC[1] ^ result 1
            crc_temp_3 = ((crc>>14)^crc_temp_1)&0x0001; // result 3 --> CRC[14] ^ result 1
            crc = crc <<1;
            crc &= 0x7FFA; // Clear CRC[0]/[2]/[15]
            crc |= crc_temp_1; // Load result 1 to CRC[0]
            crc |= (crc_temp_2<<2); // Load result 2 to CRC[2]
            crc |= (crc_temp_3<<15); // Load result 3 to CRC[15]
            data_buffer >>= 1;
        }
    }
    UBOOT_DEBUG("crc=0x%x\n", crc&0xFFFF);
    UBOOT_DEBUG("checksum=0x%x\n", checksum&0xFFFF);
    return (crc&0xFFFF);
}

static int _mtk_pnl_gpio_set_high(unsigned int gpio)
{
    int ret = 0;
    int value = 1;
    char name[4]={0,};
    if(snprintf(name,4,"%d",gpio) < 0){
        UBOOT_DEBUG("snprintf error! continue\n");
    }
    UBOOT_TRACE("%s,%d,gpio=%d,name=%s\n",__FUNCTION__,__LINE__,gpio,name);
    ret += gpio_lookup_name(name, NULL, NULL, &gpio);
    if (ret) {
        UBOOT_DEBUG("GPIO: '%s' not found\n", name);
    }
    ret += gpio_request(gpio, "cmd_gpio");
    if (ret && ret != -EBUSY) {
        UBOOT_DEBUG("gpio: requesting pin %u failed\n", gpio);
        return -1;
    }
    ret += gpio_direction_output(gpio, value);
    return ret;
}

static int _mtk_pnl_gpio_set_low(unsigned int gpio)
{
    int ret = 0;
    int value = 0;
    char name[4]={0,};
    if(snprintf(name,4,"%d",gpio) < 0){
        UBOOT_DEBUG("snprintf error! continue\n");
    }
    UBOOT_TRACE("%s,%d,gpio=%d,name=%s\n",__FUNCTION__,__LINE__,gpio,name);
    ret += gpio_lookup_name(name, NULL, NULL, &gpio);
    if (ret) {
        UBOOT_DEBUG("GPIO: '%s' not found\n", name);
    }
    ret += gpio_request(gpio, "cmd_gpio");
    if (ret && ret != -EBUSY) {
        UBOOT_DEBUG("gpio: requesting pin %u failed\n", gpio);
        return -1;
    }
    ret += gpio_direction_output(gpio, value);
    return ret;
}

static int _mtk_pnl_gpio_set_input(unsigned int gpio)
{
    int ret = 0;
    char name[4]={0,};
    if(snprintf(name,4,"%d",gpio) < 0){
        UBOOT_DEBUG("snprintf error! continue\n");
    }
    UBOOT_TRACE("%s,%d,gpio=%d,name=%s\n",__FUNCTION__,__LINE__,gpio,name);
    ret += gpio_lookup_name(name, NULL, NULL, &gpio);
    if (ret) {
        UBOOT_DEBUG("GPIO: '%s' not found\n", name);
    }
    ret += gpio_request(gpio, "cmd_gpio");
    if (ret && ret != -EBUSY) {
        UBOOT_DEBUG("gpio: requesting pin %u failed\n", gpio);
        return -1;
    }
    ret += gpio_direction_input(gpio);
    return ret;
}

static int _mtk_pnl_cust_iic_init(int busid)
{
    int ret = 0;

    if ((ret = uclass_get_device_by_seq(UCLASS_I2C, busid, &i2c_cur_dev)) != 0) {
        UBOOT_DEBUG("uclass_get_device_by_seq(UCLASS_I2C,%d) fail=%d.\n", busid, ret);
        return ret;
    }
    else
        iic_bus = busid;
    return 0;
}

static int _mtk_pnl_cust_iic_set_bus_num(unsigned int busnum)
{
    struct udevice *bus;
    int ret;

    ret = uclass_get_device_by_seq(UCLASS_I2C, busnum, &bus);
    if (ret) {
       UBOOT_TRACE("%s: No bus %d\n", __func__, busnum);
       return ret;
    }
    i2c_cur_dev = bus;

    return 0;
}

static int _mtk_pnl_cust_iic_get_cur_bus_chip(uint chip_addr, struct udevice **devp)
{
    struct udevice *bus;
    int ret = 0;

    if (!i2c_cur_dev) {
        UBOOT_DEBUG("No I2C bus selected\n");
        return -ENODEV;
    }
    bus = i2c_cur_dev;

    ret = i2c_get_chip(bus, chip_addr, 1, devp);
    return ret;
}

static int _mtk_pnl_cust_iic_read(unsigned char  u8SlaveCfg, unsigned long uAddrCnt, unsigned char  *pRegAddr, unsigned long uSize, unsigned char  *pData)
{
    unsigned int addr;
    int ret = 0;
    struct udevice *dev;

    if(uAddrCnt > 1)
    {
        addr = (pRegAddr[0]<<8) + (pRegAddr[1]&0xff);
    }
    else
    {
        addr = (pRegAddr[0]);
    }

    if(_mtk_pnl_cust_iic_set_bus_num(iic_bus) != 0)
    {
        UBOOT_ERROR("i2c_get_cur_bus_chip fail\n");
        return -EPERM;
    }

    if(_mtk_pnl_cust_iic_get_cur_bus_chip(u8SlaveCfg, &dev) != 0)
    {
        UBOOT_ERROR("i2c_get_cur_bus_chip fail\n");
        return -EPERM;
    }

    if(i2c_set_chip_offset_len(dev, uAddrCnt) != 0)
    {
        UBOOT_ERROR("i2c_set_chip_offset_len fail\n");
        return -EPERM;
    }

    ret=dm_i2c_read(dev, addr, pData, uSize);
    if(ret != 0)
    {
        UBOOT_ERROR("dm_i2c_read fail,%d\n",ret);
        return -EPERM;
    }

    return ret;
}

static int _mtk_pnl_cust_iic_write(unsigned char u8SlaveCfg, unsigned long uAddrCnt, unsigned char *pRegAddr, unsigned long uSize, unsigned char *pData)
{
    unsigned int addr = 0;
    int ret = 0;
    struct udevice *dev;

    if(uAddrCnt > 1)
    {
         addr = (pRegAddr[0]<<8) + (pRegAddr[1]&0xff);
    }
    else
    {
        addr = (pRegAddr[0]);
    }

    if(_mtk_pnl_cust_iic_set_bus_num(iic_bus) != 0)
    {
        UBOOT_DEBUG("i2c_get_cur_bus_chip fail\n");
        return -EPERM;
    }

    if(_mtk_pnl_cust_iic_get_cur_bus_chip(u8SlaveCfg, &dev) != 0)
    {
        UBOOT_DEBUG("i2c_get_cur_bus_chip fail\n");
        return -EPERM;
    }

    if(i2c_set_chip_offset_len(dev, uAddrCnt) != 0)
    {
        UBOOT_DEBUG("i2c_set_chip_offset_len fail\n");
        return -EPERM;
    }

    ret = dm_i2c_write(dev, addr, pData, uSize);
    if(ret != 0)
    {
       UBOOT_DEBUG("dm_i2c_write fail,%d\n", ret);
       return -EPERM;
    }

    return ret;
}

MS_BOOL _mtk_pnl_cust_eeprom_pageRead(MS_U16 offset, MS_U8 *pBuf, MS_U16 u16BufLen, MS_U8 BusID)
{
     ST_EEPROM_PARA stEtype;
     MS_U16 u16writeSize=0;

     if(pBuf == NULL)
     {
        UBOOT_ERROR("input parameter 'pBuf' is a null pointer\n");
        return -EPERM;
     }

     while(u16BufLen > 0)
     {
        //EEPROM_24C32 or EEPROM_24C64
        stEtype.u8PageSize=32;
        stEtype.u16SlaveIDAddr = CSOT_AUTOPGAMMA_EEPROM_ADDR;
        stEtype.u8AddrCnt = 2;
        stEtype.pu8Offset[0] = (offset >> 8);
        stEtype.pu8Offset[1] = offset;

        if(u16BufLen >= stEtype.u8PageSize)
        {
            u16writeSize = stEtype.u8PageSize;
            u16BufLen = u16BufLen - stEtype.u8PageSize;
        }
        else
        {
            u16writeSize = u16BufLen;
            u16BufLen = 0;
        }
        if(_mtk_pnl_cust_iic_read(stEtype.u16SlaveIDAddr |((MS_U16) BusID <<8), stEtype.u8AddrCnt, stEtype.pu8Offset, u16writeSize, (MS_U8*)(pBuf)) != TRUE)
        {
            UBOOT_ERROR("MApi_SWI2C_ReadBytes FAIL!!\n");
            return -EPERM;
        }

        pBuf += u16writeSize;
        offset += u16writeSize;
    }
    return 0;
}

bool _mtk_pnl_cust_get_file_data(unsigned char **ppData, const char *pchPath, unsigned short *pSize)
{
    // search order of binary file partition:
    //   1. uboot env: ${FILE_CUS_PARTITION}
    //   2. ${FILE_DEFAULT_PARTITION_1}
    //   3. ${FILE_DEFAULT_PARTITION_2}
    static char *chPartList[] = {
        NULL,
        FILE_DEFAULT_PARTITION_1,
        FILE_DEFAULT_PARTITION_2
    };
    bool bRet = FALSE;
    int i;
    size_t len;
    char chFilePath[FILE_PATH_LENGTH];
    //const char *pchPath;

    if (!ppData)
    {
        UBOOT_TRACE("ppData parameter is null\n");
        return FALSE;
    }

    if (!pchPath)
    {
        UBOOT_TRACE("pchPath parameter is null\n");
        return FALSE;
    }

    if (!pSize)
    {
        UBOOT_TRACE("pSize parameter is null\n");
        return FALSE;
    }

    memset(chFilePath, '\0', sizeof(chFilePath));

    len = strlen(FILE_FIXED_FOLDER) + strlen(pchPath) + 1;
    if (len >= sizeof(chFilePath)) {
        UBOOT_ERROR(
            "Warning: The length of the path is %d dytes\n"
            "more than the maximum file length supported\n"
            "by the system is %d bytes.\n",
            (int)len, (int)sizeof(chFilePath));
        return FALSE;
    }

    len = sizeof(chFilePath) - 1;
    strncat(chFilePath, FILE_FIXED_FOLDER, len);
    len = len - strlen(FILE_FIXED_FOLDER);
    strncat(chFilePath, pchPath, len);

    //select partition
    chPartList[0] = env_get(FILE_CUS_PARTITION);
    for (i = 0; i < ARRAY_SIZE(chPartList); i++)
    {
        if (!chPartList[i])
        {
            UBOOT_TRACE("Partition list [%d] is null\n", i);
            continue;
        }

        if(i==1)
        {
            /* STI flow for number 1 partition */
            if(!is_tcon_sti_flow())
            {
                UBOOT_BOOTTIME("NOT STI flow [%d]\n", i);
                continue;
            }
        }

        *ppData = read_storage_file_to_memory(chPartList[i], chFilePath, (loff_t *)pSize);

        if (*ppData && pSize > 0)
        {
            UBOOT_TRACE("File found. Path='%s/%s'\n", chPartList[i], chFilePath);
            bRet = TRUE;
            break;
        }
        else
        {
            UBOOT_TRACE("File not found. Path='%s/%s'\n", chPartList[i], pchPath);
        }
    }

    if (!bRet)
        UBOOT_TRACE("cannot find '%s'\n", chFilePath);

    return bRet;
}

static bool _mtk_pnl_cust_set_auto_gamma_addr_length(EN_AUTOPGAMMA_TYPE etype,
        MS_U64 *gamma_address, MS_U16 *auto_gamma_data_len)
{
    if (etype == EN_AUTOPGAMMA_H_K_C_TYPE)
    {
        *gamma_address = HKC_GAMMA_DATA_ADDR;
        *auto_gamma_data_len = H_K_C_FORMAT_TYPE_LEN;
    }
    else if (etype == EN_AUTOPGAMMA_CSOT_TYPE1)
    {
        *gamma_address = COST_GAMMA_DATA_ADDR1;
        *auto_gamma_data_len = COST_FORMAT_TYPE1_LEN;
    }
    else if (etype == EN_AUTOPGAMMA_CSOT_TYPE2)
    {
        *gamma_address = COST_GAMMA_DATA_ADDR2;
        *auto_gamma_data_len = COST_FORMAT_TYPE1_LEN;
    }
    else if (etype == EN_AUTOPGAMMA_CSOT_TYPE3)
    {
        *gamma_address = COST_GAMMA_DATA_ADDR3;
        *auto_gamma_data_len = COST_FORMAT_TYPE3_LEN;
    }
    else if (etype == EN_AUTOPGAMMA_CSOT_TYPE4)
    {
        *gamma_address = COST_GAMMA_DATA_ADDR2;
        *auto_gamma_data_len = COST_FORMAT_TYPE1_LEN;
    }
    else if (etype == EN_AUTOPGAMMA_CSOT_TYPE5)
    {
        *gamma_address = COST_GAMMA_DATA_ADDR2;
        *auto_gamma_data_len = COST_FORMAT_TYPE1_LEN;
    }
    else if (etype == EN_AUTOPGAMMA_CSOT_TYPE6)
    {
        *gamma_address = COST_GAMMA_DATA_ADDR2;
        *auto_gamma_data_len = COST_FORMAT_TYPE1_LEN;
    }
    else if (etype == EN_AUTOPGAMMA_CSOT_TYPE7)
    {
        *gamma_address = COST_GAMMA_DATA_ADDR2;
        *auto_gamma_data_len = COST_FORMAT_TYPE1_LEN;
    }
    else if (etype == EN_AUTOPGAMMA_CHOT_TYPE)
    {
        *gamma_address = COST_GAMMA_DATA_ADDR2;
        *auto_gamma_data_len = CHOT_FORMAT_TYPE_LEN;
    }
    else if (etype == EN_AUTOPGAMMA_H_K_C_TYPE2)
    {
        *gamma_address = HKC_GAMMA_DATA_ADDR;
        *auto_gamma_data_len = H_K_C_FORMAT_TYPE2_LEN;
    }
    else if (etype == EN_AUTOPGAMMA_H_K_C_TYPE3)
    {
        *gamma_address = HKC_GAMMA_DATA_ADDR;
        *auto_gamma_data_len = H_K_C_FORMAT_TYPE3_LEN;
    }
    else
    {
        UBOOT_DEBUG("Not support Auto-Pgamma [%d]\n",  etype);
        return false;
    }
	return true;
}

static int _mtk_pnl_cust_ic_auto_p_gamma(st_cust_ic_info *cust_ic, unsigned char *bin_buf, unsigned short size, unsigned char *r_data)
{
    EN_AUTOPGAMMA_TYPE etype    = EN_AUTOPGAMMA_TYPE_NULL;
    MS_U16  auto_gamma_data_len = COST_FORMAT_TYPE1_LEN;  //default length is 42, full data
    MS_U64  gamma_address       = COST_GAMMA_DATA_ADDR1;
    MS_U16  crc_high_address    = 0;
    MS_U16  crc_low_address     = 0;
    MS_U8   rd_buffer[AUTOPGAMMA_BUFFER_SIZE]    = {0};
    MS_U8   wr_buffer[AUTOPGAMMA_BUFFER_SIZE]    = {0};
    MS_U32  sample_crc16 = 0, cal_crc16 = 0;
    MS_U32  u32TempSum1 = 0, u32TempSum2 = 0;
    MS_U8   bRetE = 0;
    MS_U16  u16BusID = 0;
    MS_U8   u8CheckSum = 0;
    MS_U8   u8VcomCode = 0;
    MS_U16  u16VcomVolt = 0;

    MS_U16 i = 0;

    // step 1: mapping and getting valid data from external storage
    if ((bin_buf == NULL) || (cust_ic == NULL) || (r_data == NULL))
    {
        return -EINVAL;
    }

    etype = cust_ic->auto_update_from_flash;

    if (!_mtk_pnl_cust_set_auto_gamma_addr_length(etype, &gamma_address, &auto_gamma_data_len))
    {
        UBOOT_ERROR("Not support Auto-Pgamma [%d]\n",  etype);
        return -EINVAL;
    }

    if (size < auto_gamma_data_len)
    {
        UBOOT_ERROR("Auto Pgamma fail. bin size=%d read size=%d!!! \n", size, auto_gamma_data_len);
        return -EINVAL;
    }
    UBOOT_DEBUG("[Get]Auto P-gamma Offset=0x%lx data_len=%d type=%d \n", gamma_address, auto_gamma_data_len, etype);
    switch(etype)
    {
        case EN_AUTOPGAMMA_H_K_C_TYPE:
        case EN_AUTOPGAMMA_CSOT_TYPE1:
        case EN_AUTOPGAMMA_CSOT_TYPE2:
        case EN_AUTOPGAMMA_CSOT_TYPE4:
        case EN_AUTOPGAMMA_CSOT_TYPE5:
        case EN_AUTOPGAMMA_CSOT_TYPE6:
        case EN_AUTOPGAMMA_CSOT_TYPE7:
        case EN_AUTOPGAMMA_CHOT_TYPE:
        case EN_AUTOPGAMMA_H_K_C_TYPE2:
        case EN_AUTOPGAMMA_H_K_C_TYPE3:
        {
            #if CONFIG_SPI_FLASH
            if (init_spi_flash() != TRUE)
            {
                UBOOT_ERROR("init_spi_flash error!\n");
                return -ENXIO;
            }
            read_spi_flash(rd_buffer, gamma_address, auto_gamma_data_len);
            UBOOT_DUMP(rd_buffer, auto_gamma_data_len);
            #else
            UBOOT_ERROR("need CONFIG_SPI_FLASH!\n");
                return -ENXIO;
            #endif
            UBOOT_DEBUG("Auto P-gamma data read from SPI flash!!!\n");
        }
        break;
        case EN_AUTOPGAMMA_CSOT_TYPE3:
        {
            u16BusID = cust_ic->i2c_bus;
            UBOOT_DEBUG("read MultiByte BusID %d, SlaveID 0x%02x, Offset=0x%lx, Size=%d\n ", u16BusID, CSOT_AUTOPGAMMA_EEPROM_ADDR, gamma_address, auto_gamma_data_len);
            bRetE = _mtk_pnl_cust_eeprom_pageRead(gamma_address, rd_buffer, auto_gamma_data_len, (MS_U8)u16BusID);
            if (bRetE)
                UBOOT_ERROR("Auto P-gamma data read from eeprom fail!!! \n");
            else
                UBOOT_DEBUG("Auto P-gamma data read from eeprom success!! \n");
        }
        break;
        default:
              UBOOT_ERROR("Not support Auto-Pgamma=%d\n", etype);
        break;
    }

    // step 2: check data is valid or not
    for (i = 0; i < auto_gamma_data_len; i++)  //check for all 0x00 or 0xFF case
    {
        u32TempSum1 += rd_buffer[i];
        u32TempSum2 += rd_buffer[i];
    }
    if ((u32TempSum1 == 0) || (u32TempSum2 == auto_gamma_data_len * 0xFF))
    {
        UBOOT_ERROR("Read invalid values !!! \n");
        return -EINVAL;
    }

    switch (etype)
    {
        case EN_AUTOPGAMMA_H_K_C_TYPE:
        {
            u8CheckSum = (rd_buffer[1] + rd_buffer[2]) & 0xFF;
            if(u8CheckSum == rd_buffer[0])
            {
                UBOOT_DEBUG("Auto P-gamma checksum is correct!!! \n");
                memcpy(wr_buffer, rd_buffer, auto_gamma_data_len);
            }
            else
            {
                UBOOT_ERROR("Auto P-gamma checksum data is invalid!!! \n");
                return -EINVAL;
            }
        }
        break;
        case EN_AUTOPGAMMA_CSOT_TYPE1:
        case EN_AUTOPGAMMA_CSOT_TYPE2:
        case EN_AUTOPGAMMA_CSOT_TYPE3:
        case EN_AUTOPGAMMA_CSOT_TYPE4:
        case EN_AUTOPGAMMA_CSOT_TYPE5:
        case EN_AUTOPGAMMA_CSOT_TYPE6:
        case EN_AUTOPGAMMA_CSOT_TYPE7:
        case EN_AUTOPGAMMA_CHOT_TYPE:
        {
            crc_high_address = auto_gamma_data_len - 2;
            crc_low_address  = auto_gamma_data_len - 1;
            if ((crc_high_address >= AUTOPGAMMA_BUFFER_SIZE) || (crc_low_address >= AUTOPGAMMA_BUFFER_SIZE))
            {
                UBOOT_ERROR("Auto Pgamma crc address is invalid!!! \n");
                return -EINVAL;
            }
            sample_crc16 = (((MS_U16)rd_buffer[crc_high_address]) << 8) + rd_buffer[crc_low_address];
            cal_crc16 = CSOT_Cal_CRC16(rd_buffer, (auto_gamma_data_len - 2));
            UBOOT_DEBUG("sample_crc16=0x%x   cal_crc16=0x%x \n",sample_crc16, cal_crc16);
            if (sample_crc16 == cal_crc16)
            {
                UBOOT_DEBUG("Auto P-gamma CRC is correct!!! \n");
                memcpy(wr_buffer, rd_buffer, auto_gamma_data_len);
            }
            else
            {
                UBOOT_ERROR("Auto P-gamma CRC is invalid!!! \n");
                return -EINVAL;
            }
        }
        break;
        case EN_AUTOPGAMMA_H_K_C_TYPE2:
        {
            crc_high_address = 0;
            crc_low_address  = 1;
            sample_crc16 = (((MS_U16)rd_buffer[crc_high_address]) << 8) + rd_buffer[crc_low_address];
            cal_crc16 = HKC_Cal_CRC16(&rd_buffer[2], (HKC_GAMMA_ADDR_END-HKC_VCOM_ADDR+1));
            if (sample_crc16 == cal_crc16)
            {
                UBOOT_DEBUG("Auto P-gamma CRC is correct!!! \n");
                memcpy(wr_buffer, rd_buffer, auto_gamma_data_len);
            }
            else
            {
                UBOOT_ERROR("Auto P-gamma CRC is invalid!!! \n");
                return -EINVAL;
            }
        }
        break;
        case EN_AUTOPGAMMA_H_K_C_TYPE3:
        {
            UBOOT_DEBUG("Auto P-gamma no need to calculate CRC, get auto gamma data success!!! \n");
            memcpy(wr_buffer, rd_buffer, auto_gamma_data_len);
        }
        break;
        default:
            UBOOT_ERROR("Not support Auto-Pgamma=%d\n", etype);
        break;
    }

    // step 3: overwrite data to default buffer
    switch(etype)
    {
        case EN_AUTOPGAMMA_H_K_C_TYPE:
        {
            //Convert to CS602 format
            #define VCOM_CODE_MAX   127  //according to spec
            #define VCOM_VOLT_MAX   7541  //Max x1000
            #define VCOM_VOLT_MIN   5873  //Min x1000
            #define VCOM_VOLT_DIFF  (VCOM_VOLT_MAX - VCOM_VOLT_MIN)  //(Max - Min)

            MS_U8  u8CS602_GLDO_CODE = 0;
            MS_U16 u16CS602_GLDO_Volt = 0;
            MS_U16 u16CS602_VCOM_CODE = 0;

            u8VcomCode = wr_buffer[2];
            u8VcomCode = u8VcomCode >> 1; //bit0 is invalid
            u16VcomVolt = (VCOM_VOLT_DIFF * u8VcomCode) / VCOM_CODE_MAX + VCOM_VOLT_MIN;
            UBOOT_DEBUG("u8VcomCode = 0x%02x, u16VcomVolt = %d.\n", u8VcomCode << 1, u16VcomVolt);

            u8CS602_GLDO_CODE = bin_buf[0x0A] & 0x1F;
            u16CS602_GLDO_Volt = 13000 + (bin_buf[0x0A] & 0x1F) * 200;
            UBOOT_DEBUG("u8CS602_GLDO_CODE = 0x%02x, u16CS602_GLDO_Volt = %d.\n",
                u8CS602_GLDO_CODE, u16CS602_GLDO_Volt);

            u16CS602_VCOM_CODE = ((u16VcomVolt * 1024) / u16CS602_GLDO_Volt) - 1;
            UBOOT_DEBUG("u16CS602_VCOM_CODE = 0x%x.\n", u16CS602_VCOM_CODE);

            bin_buf[CS602_VCOM1_ADDR_START] = (bin_buf[CS602_VCOM1_ADDR_START] & 0xC0) +
                ((u16CS602_VCOM_CODE >> 4) & 0x3F);
            bin_buf[CS602_VCOM1_ADDR_END]   = (bin_buf[CS602_VCOM1_ADDR_END] & 0x0F) +
                ((u16CS602_VCOM_CODE & 0x0F) << 4);
            UBOOT_DEBUG("bin_buf[0x%02x] = 0x%02x.\n",CS602_VCOM1_ADDR_START,
                bin_buf[CS602_VCOM1_ADDR_START]);
            UBOOT_DEBUG("bin_buf[0x%02x] = 0x%02x.\n",CS602_VCOM1_ADDR_END,
                bin_buf[CS602_VCOM1_ADDR_END]);
        }
        break;
        case EN_AUTOPGAMMA_CSOT_TYPE1:
        case EN_AUTOPGAMMA_CSOT_TYPE2:
        case EN_AUTOPGAMMA_CSOT_TYPE3:
        {
            for (i = 0; i < auto_gamma_data_len; i++)
            {
                bin_buf[i] = wr_buffer[i];
            }
        }
        break;
        case EN_AUTOPGAMMA_CSOT_TYPE4:
        {
            bin_buf[CS602_VCOM1_ADDR_START] = wr_buffer[CS602_VCOM1_ADDR_START];
            bin_buf[CS602_VCOM1_ADDR_END]   = (wr_buffer[CS602_VCOM1_ADDR_END]&0xF0)|(bin_buf[CS602_VCOM1_ADDR_END]&0x0F); //Mask VCOM2 Value
            UBOOT_DEBUG("Replace result:bin_buf[0x%02x] = 0x%02x.\n",CS602_VCOM1_ADDR_START,bin_buf[CS602_VCOM1_ADDR_START]);
            UBOOT_DEBUG("Replace result:bin_buf[0x%02x] = 0x%02x.\n",CS602_VCOM1_ADDR_END,bin_buf[CS602_VCOM1_ADDR_END]);
        }
        break;
        case EN_AUTOPGAMMA_CSOT_TYPE5:
        {
            for (i = CS602_GAMMA_ADDR_START; i <= CS602_VCOM2_ADDR_END; i++)
            {
                bin_buf[i] = wr_buffer[i];
            }
            //only need to apply GLD0[4:0], the rest use original value.
            bin_buf[CS602_GLD0_ADDR] = (bin_buf[CS602_GLD0_ADDR]&0xE0) | (wr_buffer[CS602_GLD0_ADDR]&0x1F);
        }
        break;
        case EN_AUTOPGAMMA_CSOT_TYPE6:
        {
            bin_buf[0x0A] = (wr_buffer[0x0A]&0x1F) | (bin_buf[0x0A]&0xE0);
            bin_buf[0x10] = wr_buffer[0x10];
            bin_buf[0x11] = (wr_buffer[0x11]&0xF0) | (bin_buf[0x11]&0x0F);
            bin_buf[0x17] = ((wr_buffer[0x19]&0x30)>>4) | (bin_buf[0x17]&0xF0);
            bin_buf[0x18] = ((wr_buffer[0x19]&0x0F)<<4) | ((wr_buffer[0x1A]&0xF0)>>4);
            bin_buf[0x1C] = ((wr_buffer[0x1A]&0x03)<<4) | (wr_buffer[0x1B]>>4);
            bin_buf[0x1D] = ((wr_buffer[0x1B]&0x0F)<<4) | (bin_buf[0x1D]&0x0F);
            bin_buf[0x23] = (wr_buffer[0x23]&0x03) | (bin_buf[0x23]&0xFC);
            bin_buf[0x24] = wr_buffer[0x24];
            bin_buf[0x25] = wr_buffer[0x25];
            bin_buf[0x26] = wr_buffer[0x26];
            bin_buf[0x27] = wr_buffer[0x27];
            bin_buf[0x1D] = (wr_buffer[0x11]&0x03) | (bin_buf[0x1D]&0xFC);
            bin_buf[0x1E] = wr_buffer[0x12];
        }
        break;
        case EN_AUTOPGAMMA_CSOT_TYPE7:
        case EN_AUTOPGAMMA_CHOT_TYPE:
        {
            unsigned int GLDO_Vx1000 = 0;
            unsigned int AVDD_Vx1000 = 0;
            unsigned i;
            unsigned int gamma_n1;
            unsigned int gamma_n2;

            unsigned char vcom1 = 0;
            unsigned int vcom_max_Vx1000 = 0;
            unsigned int vcom_min_Vx1000 = 0;
            unsigned short eeprom_vcom1 = 0;
            unsigned int vcom1_Vx1000 = 0;

            unsigned char gamma_start = 0;
            unsigned char gamma_size = 0;
            unsigned char vcom_start = 0;
            unsigned char vcom_end = 0;
            int ret = -1;

            // Transform GAMMA/VCOM value from spi EEPROM to TC901
            // All voltage below are multiply by 1000 to avoid losing precise during division

            // EEPROM value for gamma/vcom voltage need to be calculate by gldo store on EEPROM
            // gldo voltage = 13.0V + (0.2 * gldo)
            if (etype == EN_AUTOPGAMMA_CSOT_TYPE7){
                GLDO_Vx1000 = 13000 + ((wr_buffer[CS602_GLD0_ADDR] & 0x1F) * 200);
                gamma_start = CS602_GAMMA_ADDR_START;
                gamma_size = ((CS602_GAMMA_ADDR_END-CS602_GAMMA_ADDR_START)+1);
                vcom_start = CS602_VCOM1_ADDR_START;
                vcom_end = CS602_VCOM1_ADDR_END;
            }else if (etype == EN_AUTOPGAMMA_CHOT_TYPE){
                GLDO_Vx1000 = 15000;
                gamma_start = CHOT_GAMMA_ADDR_START;
                gamma_size = ((CHOT_GAMMA_ADDR_END-CHOT_GAMMA_ADDR_START)+1);
                vcom_start = CHOT_VCOM1_ADDR_START;
                vcom_end = CHOT_VCOM1_ADDR_END;
            }

            // gamma/vcom voltage set to TC901 need to be calculate by avdd voltage on TC901
            // avdd voltage = 13.5V + (0.1 * avdd)
            AVDD_Vx1000 = 13500 + ((bin_buf[TC901_AVDD_REG] & 0x3F) * 100);

            UBOOT_DEBUG("avdd=%d.%03dV(0x%x) gldo=%d.%03dV(0x%x)\n",
            (AVDD_Vx1000 / 1000), (AVDD_Vx1000 % 1000), (bin_buf[TC901_AVDD_REG] & 0x3F),
            (GLDO_Vx1000 / 1000), (GLDO_Vx1000 % 1000), (wr_buffer[CS602_GLD0_ADDR] & 0x1F));

            for (i = 0; i < (gamma_size / 3); i++) {
                // For every 3 bytes, contains 2 gamma value by below allocation
                // EEPROM     bits    data types      bits
                // gamma[x+0] [5:0] | eeprom_gamma_n1 [9:4]
                // gamma[x+1] [4:7] | eeprom_gamma_n1 [3:0]
                //            [1:0] | eeprom_gamma_n2 [9:8]
                // gamma[x+2] [7:0] | eeprom_gamma_n2 [7:0]
                unsigned int offset = gamma_start + (i * 3);
                unsigned short eeprom_gamma_n1 = ((wr_buffer[offset] & 0x3F) << 4) + ((wr_buffer[(offset + 1)] & 0xF0) >> 4);
                unsigned short eeprom_gamma_n2 = ((wr_buffer[(offset + 1)] & 0x03) << 8) + wr_buffer[(offset + 2)];

                // gamma voltage = (eeprom_gamma + 1) * avdd_V / 1024
                unsigned int gamma_n1_Vx1000 = ((eeprom_gamma_n1 + 1) * GLDO_Vx1000) / 1024;
                unsigned int gamma_n2_Vx1000 = ((eeprom_gamma_n2 + 1) * GLDO_Vx1000) / 1024;

                // res4_V = AVDD_V / 1024
                // GAMMAn1 = GAMMA_n1_V / res4_V = (GAMMA_n1_V) * 1024 / AVDD_V
                gamma_n1 = (gamma_n1_Vx1000 * 1024 / AVDD_Vx1000);
                gamma_n2 = (gamma_n2_Vx1000 * 1024 / AVDD_Vx1000);

                UBOOT_DEBUG("GAMMA%02d=%02d.%03dV(eeprom:0x%03x tc901:0x%03x)\n",
                ((i * 2) + 1), (gamma_n1_Vx1000 / 1000), (gamma_n1_Vx1000 % 1000) , eeprom_gamma_n1, gamma_n1);
                UBOOT_DEBUG("GAMMA%02d=%02d.%03dV(eeprom:0x%03x tc901:0x%03x)\n",
                ((i * 2) + 2), (gamma_n2_Vx1000 / 1000), (gamma_n2_Vx1000 % 1000) , eeprom_gamma_n2, gamma_n2);

                // remap 2 gamma setting back to 3 bytes
                wr_buffer[offset]       = ((gamma_n1 & 0x3F0) >> 4);
                wr_buffer[(offset + 1)] = ((gamma_n1 & 0x00F) << 4) + ((gamma_n2 & 0x300) >> 8);
                wr_buffer[(offset + 2)] =  (gamma_n2 & 0x0FF);
            }

            //compare TC901 i2c eeprom and SPI flash transfer format data
            ret = memcmp(&r_data[TC901_GAMMA_REGS], &wr_buffer[gamma_start], sizeof(unsigned char)*gamma_size);
            memcpy(&bin_buf[TC901_GAMMA_REGS], &wr_buffer[gamma_start], sizeof(unsigned char)*gamma_size);
            UBOOT_DEBUG("TC901 gamma data is : %s\n", (ret==0) ? "same": "different");

            // vcom_max voltage = VCOM_MAX * avdd_V / 128
            vcom_max_Vx1000 = ((bin_buf[TC901_VCOM_MAX_REG] & 0x7F) + 1) * AVDD_Vx1000 / 128;
            // vcom_min voltage = VCOM_MIN * avdd_V / 128
            vcom_min_Vx1000 = (bin_buf[TC901_VCOM_MIN_REG] & 0x7F) * AVDD_Vx1000 / 128;

            // EEPROM   bits    data types    bits
            // vcom[0]  [5:0] | eeprom_vcom1  [9:4]
            // vcom[1]  [4:7] | eeprom_vcom1  [3:0]
            eeprom_vcom1 = ((wr_buffer[vcom_start] & 0x3F) << 4) + ((wr_buffer[vcom_end] & 0xF0) >> 4);
            // vcom voltage = (eeprom_vcom + 1) * avdd_V / 1024
            vcom1_Vx1000 = ((eeprom_vcom1 + 1) * GLDO_Vx1000) / 1024;

            // res1_V = (vcom_max_V - vcom_min_V) / 127
            // vcom1 = (vcom1_Vx1000 - vcom_min_V) / res1_V = (vcom1_Vx1000 - vcom_min_V) * 127 / (vcom_max_V - vcom_min_V)
            vcom1 = (unsigned char)((vcom1_Vx1000 - vcom_min_Vx1000) * 127 / (vcom_max_Vx1000 - vcom_min_Vx1000));

            UBOOT_DEBUG("vcom_max=%d.%03dV(0x%x) vcom_min=%d.%03dV(0x%x) vcom1=%d.%03dV(0x%x) eeprom_vcom1=0x%x\n",
            (vcom_max_Vx1000 / 1000), (vcom_max_Vx1000 % 1000), (bin_buf[TC901_VCOM_MAX_REG] & 0x7F),
            (vcom_min_Vx1000 / 1000), (vcom_min_Vx1000 % 1000), (bin_buf[TC901_VCOM_MIN_REG] & 0x7F),
            (vcom1_Vx1000    / 1000), (vcom1_Vx1000    % 1000), vcom1, eeprom_vcom1);

            //compare TC901 i2c eeprom and SPI flash transfer format data
            bin_buf[TC901_VCOM1_REG] = vcom1;
            UBOOT_DEBUG("TC901 vcom data is : %s\n", (vcom1 == r_data[TC901_VCOM1_REG]) ? "same": "different");
        }
    	break;
        case EN_AUTOPGAMMA_H_K_C_TYPE2:
        {
            unsigned int GLDO_Vx1000 = 15580;
            unsigned int AVDD_Vx1000 = 0;
            unsigned i;
            unsigned int gamma_n1;
            unsigned int gamma_n2;

            unsigned char vcom1 = 0;
            unsigned int vcom_max_Vx1000 = 0;
            unsigned int vcom_min_Vx1000 = 0;
            unsigned short eeprom_vcom1 = 0;
            unsigned int vcom1_Vx1000 = 0;

            unsigned char gamma_start = HKC_GAMMA_ADDR_START;
            unsigned char gamma_size = HKC_GAMMA_ADDR_END-HKC_GAMMA_ADDR_START+1;
            int ret = -1;

            // Transform GAMMA/VCOM value from spi EEPROM to TC901
            // All voltage below are multiply by 1000 to avoid losing precise during division

            // EEPROM value for gamma/vcom voltage need to be calculate by gldo store on EEPROM
            // gldo voltage = 15.58V

            // gamma/vcom voltage set to TC901 need to be calculate by avdd voltage on TC901
            // avdd voltage = 13.5V + (0.1 * avdd)
            AVDD_Vx1000 = 13500 + ((bin_buf[TC901_AVDD_REG] & 0x3F) * 100);

            UBOOT_DEBUG("avdd=%d.%03dV(0x%x) gldo=%d.%03dV\n",
            (AVDD_Vx1000 / 1000), (AVDD_Vx1000 % 1000), (bin_buf[TC901_AVDD_REG] & 0x3F),
            (GLDO_Vx1000 / 1000), (GLDO_Vx1000 % 1000));

            for (i = 0; i < (gamma_size / 4); i++) {
                // For every 3 bytes, contains 2 gamma value by below allocation
                // EEPROM     bits    data types      bits
                // gamma[x+0] [5:0] | eeprom_gamma_n1 [15:8]
                // gamma[x+1] [4:7] | eeprom_gamma_n1 [7:0]
                //            [1:0] | eeprom_gamma_n2 [15:8]
                // gamma[x+2] [7:0] | eeprom_gamma_n2 [7:0]
                unsigned int offset = gamma_start + (i * 4);
                unsigned short eeprom_gamma_n1 = (wr_buffer[offset] << 8) + wr_buffer[(offset + 1)];
                unsigned short eeprom_gamma_n2 = (wr_buffer[(offset + 2)] << 8) + wr_buffer[(offset + 3)];

                // gamma voltage = VREF / 1023 * eeprom_gamma
                unsigned int gamma_n1_Vx1000 =  ((GLDO_Vx1000*1000) / 1023 * eeprom_gamma_n1)/1000;
                unsigned int gamma_n2_Vx1000 = ((GLDO_Vx1000*1000) / 1023 * eeprom_gamma_n2)/1000;

                // res4_V = AVDD_V / 1024
                // GAMMAn1 = GAMMA_n1_V / res4_V = (GAMMA_n1_V) * 1024 / AVDD_V
                gamma_n1 = (gamma_n1_Vx1000 * 1024 / AVDD_Vx1000);
                gamma_n2 = (gamma_n2_Vx1000 * 1024 / AVDD_Vx1000);

                UBOOT_DEBUG("GAMMA%02d=%02d.%03dV(eeprom:0x%03x tc901:0x%03x)\n",
                ((i * 2) + 1), (gamma_n1_Vx1000 / 1000), (gamma_n1_Vx1000 % 1000) , eeprom_gamma_n1, gamma_n1);
                UBOOT_DEBUG("GAMMA%02d=%02d.%03dV(eeprom:0x%03x tc901:0x%03x)\n",
                ((i * 2) + 2), (gamma_n2_Vx1000 / 1000), (gamma_n2_Vx1000 % 1000) , eeprom_gamma_n2, gamma_n2);

                // remap 2 gamma setting back to 3 bytes
                offset = gamma_start + (i * 3);
                wr_buffer[offset]       = ((gamma_n1 & 0x3F0) >> 4);
                wr_buffer[(offset + 1)] = ((gamma_n1 & 0x00F) << 4) + ((gamma_n2 & 0x300) >> 8);
                wr_buffer[(offset + 2)] =  (gamma_n2 & 0x0FF);
            }

            //compare TC901 i2c eeprom and SPI flash transfer format data
            ret = memcmp(&r_data[TC901_GAMMA_REGS], &wr_buffer[gamma_start], sizeof(unsigned char)*TC901_GAMMA_SIZE);
            memcpy(&bin_buf[TC901_GAMMA_REGS], &wr_buffer[gamma_start], sizeof(unsigned char)*TC901_GAMMA_SIZE);
            UBOOT_DEBUG("TC901 gamma data is : %s\n", (ret==0) ? "same": "different");

            // vcom_max voltage = VCOM_MAX * avdd_V / 128
            vcom_max_Vx1000 = ((bin_buf[TC901_VCOM_MAX_REG] & 0x7F) + 1) * AVDD_Vx1000 / 128;
            // vcom_min voltage = VCOM_MIN * avdd_V / 128
            vcom_min_Vx1000 = (bin_buf[TC901_VCOM_MIN_REG] & 0x7F) * AVDD_Vx1000 / 128;

            eeprom_vcom1 = wr_buffer[HKC_VCOM_ADDR];
            // vcom voltage = (VCOM_MAX-VCOM_MIN)/127 * eeprom_vcom1 + VCOM_MIN = (7.8-5.0)/127 * eeprom_vcom1 + 5.0
            vcom1_Vx1000 = (HKC_VCOM_MAX-HKC_VCOM_MIN)/127 * eeprom_vcom1 + HKC_VCOM_MIN;

            // res1_V = (vcom_max_V - vcom_min_V) / 127
            // vcom1 = (vcom1_Vx1000 - vcom_min_V) / res1_V = (vcom1_Vx1000 - vcom_min_V) * 127 / (vcom_max_V - vcom_min_V)
            vcom1 = (unsigned char)((vcom1_Vx1000 - vcom_min_Vx1000) * 127 / (vcom_max_Vx1000 - vcom_min_Vx1000));

            UBOOT_DEBUG("vcom_max=%d.%03dV(0x%x) vcom_min=%d.%03dV(0x%x) vcom1=%d.%03dV(0x%x) eeprom_vcom1=0x%x\n",
            (vcom_max_Vx1000 / 1000), (vcom_max_Vx1000 % 1000), (bin_buf[TC901_VCOM_MAX_REG] & 0x7F),
            (vcom_min_Vx1000 / 1000), (vcom_min_Vx1000 % 1000), (bin_buf[TC901_VCOM_MIN_REG] & 0x7F),
            (vcom1_Vx1000    / 1000), (vcom1_Vx1000    % 1000), vcom1, eeprom_vcom1);

            //compare TC901 i2c eeprom and SPI flash transfer format data
            bin_buf[TC901_VCOM1_REG] = vcom1;
            UBOOT_DEBUG("TC901 vcom data is : %s\n", (vcom1 == r_data[TC901_VCOM1_REG]) ? "same": "different");
        }
        break;
        case EN_AUTOPGAMMA_H_K_C_TYPE3:
        {
            MS_U16 j = 0;
            for (i = HKC_TYPE3_REG_GAMMA_START, j=HKC_GAMMA_ADDR_START; i < HKC_TYPE3_REG_GAMMA_END;)
            {
                bin_buf[i] = ((wr_buffer[j]&0x0F)<<4) | ((wr_buffer[j+1]&0xF0)>>4);
                bin_buf[i+1] = ((wr_buffer[j+1]&0x0F)<<4) | ((wr_buffer[j+2]&0x0F));
                bin_buf[i+2] = wr_buffer[j+3];
                i+=3;
                j+=4;
            }
        }
        break;
        default:
            UBOOT_ERROR("Not support Auto-Pgamma=%d\n", etype);
        break;
    }
    return 0;
}

static void _mtk_pnl_cust_ic_get_sub_bin_file_part1(st_multi_cust_ic_info *multi_cust_ic)
{
    if (multi_cust_ic == NULL) {
        UBOOT_TRACE("[%s][%d] invalid data!\n", __FUNCTION__,__LINE__);
        return;
    }

    char *ic_bin_path = multi_cust_ic->pmic_sub_info.ic_bin_file_path;
    if ((ic_bin_path != NULL) && (multi_cust_ic->pmic_sub_info.force_init == 1)) {
        _mtk_pnl_cust_get_file_data(&pmic_sub_bin_buf, ic_bin_path, &pmic_sub_size);
        UBOOT_TRACE("[%d] sub pmic bin path=%s, ls_size=%d\n", __LINE__, ic_bin_path, pmic_sub_size);
        if ((pmic_sub_bin_buf != NULL) && (pmic_sub_size > 0)) {
            UBOOT_DUMP(pmic_sub_bin_buf, pmic_sub_size);
        }
    }
    if (pmic_sub_bin_buf == NULL)
    {
        UBOOT_TRACE("Read sub pmic ic bin file failure!\n");
    }

    ic_bin_path = multi_cust_ic->pgamma_sub_info.ic_bin_file_path;
    if ((ic_bin_path != NULL) && (multi_cust_ic->pgamma_sub_info.force_init == 1)) {
        _mtk_pnl_cust_get_file_data(&pgamma_sub_bin_buf, ic_bin_path, &pgamma_sub_size);
        UBOOT_TRACE("[%d] sub pgamma bin path=%s, pgamma_sub_size=%d\n", __LINE__, ic_bin_path, pgamma_sub_size);
        if ((pgamma_sub_bin_buf != NULL) && (pgamma_sub_size > 0)) {
            UBOOT_DUMP(pgamma_sub_bin_buf, pgamma_sub_size);
        }
    }
    if (pgamma_sub_bin_buf == NULL)
    {
        UBOOT_TRACE("Read sub pgamma ic bin file failure!\n");
    }

    return;
}

static void _mtk_pnl_cust_ic_get_sub_bin_file_part2(st_multi_cust_ic_info *multi_cust_ic)
{
    if (multi_cust_ic == NULL) {
        UBOOT_TRACE("[%s][%d] invalid data!\n", __FUNCTION__,__LINE__);
        return;
    }

    char *ic_bin_path = multi_cust_ic->levelshift_sub_info.ic_bin_file_path;
    if ((ic_bin_path != NULL) && (multi_cust_ic->levelshift_sub_info.force_init == 1)) {
        _mtk_pnl_cust_get_file_data(&ls_sub_bin_buf, ic_bin_path, &ls_sub_size);
        UBOOT_TRACE("[%d] sub level shift bin path=%s, ls_size=%d\n", __LINE__, ic_bin_path, ls_sub_size);
        if ((ls_sub_bin_buf != NULL) && (ls_sub_size > 0)) {
            UBOOT_DUMP(ls_sub_bin_buf, ls_sub_size);
        }
    }
    if (ls_sub_bin_buf == NULL)
    {
        UBOOT_TRACE("Read sub level shifit ic bin file failure!\n");
    }

    ic_bin_path = multi_cust_ic->vcomic_sub_info.ic_bin_file_path;
    if ((ic_bin_path != NULL) && (multi_cust_ic->vcomic_sub_info.force_init == 1)) {
        _mtk_pnl_cust_get_file_data(&vcomic_sub_bin_buf, ic_bin_path, &vcomic_sub_size);
        UBOOT_TRACE("[%d] sub vcom ic bin path=%s, vcomic_sub_size=%d\n", __LINE__, ic_bin_path, vcomic_sub_size);
        if ((vcomic_sub_bin_buf != NULL) && (vcomic_sub_size > 0)) {
            UBOOT_DUMP(vcomic_sub_bin_buf, vcomic_sub_size);
        }
    }
    if (vcomic_sub_bin_buf == NULL)
    {
        UBOOT_TRACE("Read sub vcom ic bin file failure!\n");
    }

    return;
}

static void _mtk_pnl_cust_ic_get_bin_file_part1(st_multi_cust_ic_info *multi_cust_ic)
{
    char *ic_bin_path = NULL;

    if (multi_cust_ic == NULL) {
        UBOOT_TRACE("[%s][%d] invalid data!\n", __FUNCTION__, __LINE__);
        return;
    }

    ic_bin_path = multi_cust_ic->pmic_info.ic_bin_file_path;

    if ((ic_bin_path != NULL) && (multi_cust_ic->pmic_info.force_init == 1)) {
        _mtk_pnl_cust_get_file_data(&pmic_bin_buf, ic_bin_path, &pmic_size);
        UBOOT_TRACE("[%d] pmic bin path=%s, pmic_size=%d\n", __LINE__, ic_bin_path, pmic_size);
        if ((pmic_bin_buf != NULL) && (pmic_size > 0)) {
            UBOOT_DUMP(pmic_bin_buf, pmic_size);
        }
    }
    if (pmic_bin_buf == NULL)
    {
        UBOOT_TRACE("Read pmic bin file failure!\n");
    }

    ic_bin_path = multi_cust_ic->pgamma_info.ic_bin_file_path;
    if ((ic_bin_path != NULL) && (multi_cust_ic->pgamma_info.force_init == 1)) {
        _mtk_pnl_cust_get_file_data(&pgamma_bin_buf, ic_bin_path, &pgamma_size);
        UBOOT_TRACE("[%d] pgamma bin path=%s, pgamma_size=%d\n", __LINE__, ic_bin_path, pgamma_size);
        if ((pgamma_bin_buf != NULL) && (pgamma_size > 0)) {
            UBOOT_DUMP(pgamma_bin_buf, pgamma_size);
        }
    }
    if (pgamma_bin_buf == NULL)
    {
        UBOOT_TRACE("Read pgamma bin file failure!\n");
    }
}

static void _mtk_pnl_cust_ic_get_bin_file_part2(st_multi_cust_ic_info *multi_cust_ic)
{
    char *ic_bin_path = NULL;

    if (multi_cust_ic == NULL) {
        UBOOT_TRACE("[%s][%d] invalid data!\n", __FUNCTION__, __LINE__);
        return;
    }

    ic_bin_path = multi_cust_ic->levelshift_info.ic_bin_file_path;

    if ((ic_bin_path != NULL) && (multi_cust_ic->levelshift_info.force_init == 1)) {
        _mtk_pnl_cust_get_file_data(&ls_bin_buf, ic_bin_path, &ls_size);
        UBOOT_TRACE("[%d] level shift bin path=%s, ls_size=%d\n", __LINE__, ic_bin_path, ls_size);
        if ((ls_bin_buf != NULL) && (ls_size > 0)) {
            UBOOT_DUMP(ls_bin_buf, ls_size);
        }
    }
    if (ls_bin_buf == NULL)
    {
        UBOOT_TRACE("Read levelshifit bin file failure!\n");
    }

    ic_bin_path = multi_cust_ic->vcomic_info.ic_bin_file_path;
    if ((ic_bin_path != NULL) && (multi_cust_ic->vcomic_info.force_init == 1)) {
        _mtk_pnl_cust_get_file_data(&vcomic_bin_buf, ic_bin_path, &vcomic_size);
        UBOOT_TRACE("[%d] vcom ic bin path=%s, vcomic_size=%d\n", __LINE__, ic_bin_path, vcomic_size);
        if ((vcomic_bin_buf != NULL) && (vcomic_size > 0)) {
            UBOOT_DUMP(vcomic_bin_buf, vcomic_size);
        }
    }
    if (vcomic_bin_buf == NULL)
    {
        UBOOT_TRACE("Read vcom ic bin file failure!\n");
    }
}

static void _mtk_pnl_cust_ic_get_bin_file(st_multi_cust_ic_info *multi_cust_ic)
{
    if (multi_cust_ic == NULL) {
        UBOOT_TRACE("[%s][%d] invalid data!\n", __FUNCTION__,__LINE__);
        return;
    }

    _mtk_pnl_cust_ic_get_bin_file_part1(multi_cust_ic);
    _mtk_pnl_cust_ic_get_bin_file_part2(multi_cust_ic);

    _mtk_pnl_cust_ic_get_sub_bin_file_part1(multi_cust_ic);
    _mtk_pnl_cust_ic_get_sub_bin_file_part2(multi_cust_ic);

    return;
}

static void _mtk_pnl_cust_ic_write(unsigned char *w_data, int w_size, st_cust_ic_info *cust_info)
{
    uint16_t bus_id, slave_id;
    unsigned long addr_count;
    unsigned char reg_offset;
    unsigned long data_size;
    unsigned char *write_data;
    unsigned char write_mode;
    int ret = 0, i;
    /******************************************************************************************************/
    // PNL_VCC on -> VCC delay(ms) -> Pre GPIO Operation -> Pre delay(ms) -> IC write (from bin) ->
    // Post GPIO Operation -> Post delay(ms)
    /******************************************************************************************************/

    if(cust_info == NULL)
        return;

    if(cust_info->vcc_dly > 0) {
        mdelay(cust_info->vcc_dly);
    }

    if(cust_info->gpio_pre_num > 0) {
        switch(cust_info->gpio_pre_ops)
        {
            case 0:
            {
                _mtk_pnl_gpio_set_low(cust_info->gpio_pre_num);
                UBOOT_TRACE("GPIO[%d] Ouput Low\n",cust_info->gpio_pre_num);
                break;
            }
            case 1:
            {
                _mtk_pnl_gpio_set_high(cust_info->gpio_pre_num);
                UBOOT_TRACE("GPIO[%d] Ouput High\n",cust_info->gpio_pre_num);
                break;
            }
            case 0xFF:
            default:
            {
                _mtk_pnl_gpio_set_input(cust_info->gpio_pre_num);
                UBOOT_TRACE("GPIO[%d] Input Hi-Z\n",cust_info->gpio_pre_num);
                break;
            }
        }
    }

    if(cust_info->gpio_pre_dly > 0) {
        mdelay(cust_info->gpio_pre_dly);
    }
    bus_id = cust_info->i2c_bus;
    slave_id = cust_info->i2c_dev_addr;
    addr_count = 1;
    reg_offset = cust_info->i2c_reg_offset;
    data_size = w_size;
    write_data = w_data;
    write_mode = cust_info->write_mode;

    UBOOT_TRACE("I2C write MultiByte bus_id=%d, slave_id=0x%02x, reg_offset=%d, data_size=%ld\n ", bus_id, slave_id, reg_offset, data_size);
    _mtk_pnl_cust_iic_init(bus_id);
    if(write_mode == 1) //write a byte at a time
    {
        for(i=0; i<data_size; i++)
        {
            ret |= _mtk_pnl_cust_iic_write(slave_id, addr_count, &reg_offset, 1, write_data);
            reg_offset++;
            write_data++;
        }
    }
    else
    {
    ret = _mtk_pnl_cust_iic_write(slave_id, addr_count, &reg_offset, data_size, write_data);
    }
    if(ret < 0)
        UBOOT_ERROR("write iic error!! bus_id=%d slave_id=0x%02x \n",bus_id, slave_id);

    if(cust_info->gpio_post_dly > 0) {
        mdelay(cust_info->gpio_post_dly);
    }
    if(cust_info->gpio_post_num > 0) {
        switch(cust_info->gpio_post_ops)
        {
            case 0:
            {
                _mtk_pnl_gpio_set_low(cust_info->gpio_post_num);
                UBOOT_TRACE("GPIO[%d] Ouput Low\n",cust_info->gpio_post_num);
                break;
            }
            case 1:
            {
                _mtk_pnl_gpio_set_high(cust_info->gpio_post_num);
                UBOOT_TRACE("GPIO[%d] Ouput High\n",cust_info->gpio_post_num);
                break;
            }
            case 0xFF:
            default:
            {
                _mtk_pnl_gpio_set_input(cust_info->gpio_post_num);
                UBOOT_TRACE("GPIO[%d] Input Hi-Z\n",cust_info->gpio_post_num);
                break;
            }
        }
    }
    return;
}

static void _mtk_pnl_cust_ic_write_by_mask(unsigned char *w_data, int w_size, st_cust_ic_info *cust_info)
{
    return;
}

static int _mtk_pnl_cust_ic_read(unsigned char* r_data, int r_size, st_cust_ic_info *cust_info)
{
    uint16_t bus_id, slave_id;
    unsigned long addr_count;
    unsigned char reg_offset;
    unsigned char read_mode;
    unsigned long data_size;
    unsigned char read_control_data[2];
    int ret = 0;

    if ((r_data == NULL) && (r_size == 0)) {
        UBOOT_TRACE("invalid data !\n");
        return -ENOMEM;
    }

    bus_id = cust_info->i2c_bus;
    slave_id = cust_info->i2c_dev_addr;
    addr_count = 1;
    reg_offset = cust_info->i2c_reg_offset;
    data_size = r_size;
    read_mode = cust_info->read_mode;
    UBOOT_TRACE("I2C read MultiByte bus_id=%d, slave_id=0x%02x, reg_offset=%d, data_size=%ld read_mode=%d\n ", bus_id, slave_id, reg_offset, data_size, read_mode);
    _mtk_pnl_cust_iic_init(bus_id);
    if (read_mode) {
        if ((read_mode == 1) || (read_mode == 3)) {
            read_control_data[0] = cust_info->i2c_ctrl_reg_offset;
            read_control_data[1] = cust_info->i2c_ctrl_reg;
            UBOOT_TRACE("Control reg:%x Control data:%x\n", read_control_data[0], read_control_data[1]);
            _mtk_pnl_cust_iic_write(slave_id, 0, &reg_offset, 1, read_control_data);
        } else if (read_mode == 2) {
            _mtk_pnl_cust_iic_write(slave_id, 0, &reg_offset, 1, &reg_offset);
        }
        ret = _mtk_pnl_cust_iic_read(slave_id, addr_count, &reg_offset, data_size, r_data);
    } else {
        ret = _mtk_pnl_cust_iic_read(slave_id, addr_count, &reg_offset, data_size, r_data);
    }
    if(ret < 0) {
        UBOOT_ERROR("read iic error!! bus_id=%d slave_id=0x%02x \n",bus_id, slave_id);
        return -EINVAL;
    } else {
         UBOOT_TRACE("read iic OK: \n");
         UBOOT_DUMP(r_data, data_size);
    }
    return 0;
}

static void _mtk_pnl_cust_ic_read_by_mask(unsigned char* r_data, int r_size, st_cust_ic_info *cust_info)
{
    return;
}

static int _mtk_pnl_cust_ic_checksum(st_cust_ic_info *cust_info, unsigned char *r_data, unsigned char *bin_buf, unsigned short bin_size)
{
    int index = 0, bypass_index = 0;
    unsigned long bin_checksum = 0;
    unsigned long read_checksum = 0;
    int data_start_offset = 0;
    int data_end_offset = 0;
    bool bypass = FALSE;

    UBOOT_TRACE("IN\n");
    if ((cust_info == NULL) || (r_data == NULL) || (bin_buf == NULL) || (bin_size == 0)) {
        UBOOT_TRACE("[%d] Invalid data!\n", __LINE__);
        return FALSE;
    }
    /* skip status register and control register if needed */
    data_start_offset = cust_info->data_start;
    data_end_offset   = cust_info->data_end;

    if ((data_start_offset < 0) || (data_start_offset >= bin_size) || (data_end_offset < 0) || (data_end_offset >= bin_size)) {
        UBOOT_TRACE("[%d] Invalid data!\n", __LINE__);
        return FALSE;
    }
    for (index = data_start_offset; index < (bin_size - data_end_offset); index++) {
		if (cust_info->checksum_bypass_size>0 && cust_info->checksum_bypass_offset != NULL) {
			bypass = FALSE;
			for (bypass_index = 0; bypass_index < cust_info->checksum_bypass_size; bypass_index++) {
				if (index == cust_info->checksum_bypass_offset[bypass_index]) {
					UBOOT_INFO("checksum bypass index = %d\n", index);
					bypass = TRUE;
					break;
				}
			}
			if (bypass == TRUE)
				continue;
		}

        bin_checksum  += bin_buf[index];
        read_checksum += r_data[index];
    }
    if (bin_checksum != read_checksum) {
        UBOOT_TRACE("Checksum fail! bin_checksum=0x%lx, read_checksum=0x%lx\n", bin_checksum, read_checksum);
        return FALSE;
    }
    UBOOT_TRACE("EXIT\n");

    return TRUE;
}

static int _mtk_pnl_cust_ic_burn(st_cust_ic_info *cust_info)
{
    uint16_t bus_id, slave_id;
    unsigned long addr_count = 0;
    unsigned long data_size = 0;
    unsigned char burn_cmd = 0;
    unsigned char burn_offset = 0;
    int  ret;

    UBOOT_TRACE("IN\n");
    if (cust_info == NULL)
    {
        UBOOT_TRACE("[%d] Invalid data!\n", __LINE__);
        return FALSE;
    }

    if (cust_info->gpio_pre_num > 0)
    {
        switch (cust_info->gpio_pre_ops)
        {
            case 0:
            {
                _mtk_pnl_gpio_set_low(cust_info->gpio_pre_num);
                UBOOT_TRACE("GPIO[%d] Ouput Low\n", cust_info->gpio_pre_num);
                break;
            }
            case 1:
            {
                _mtk_pnl_gpio_set_high(cust_info->gpio_pre_num);
                UBOOT_TRACE("GPIO[%d] Ouput High\n", cust_info->gpio_pre_num);
                break;
            }
            case 0xFF:
            default:
            {
                _mtk_pnl_gpio_set_input(cust_info->gpio_pre_num);
                UBOOT_TRACE("GPIO[%d] Input Hi-Z\n", cust_info->gpio_pre_num);
                break;
            }
        }
    }

    if (cust_info->gpio_pre_dly > 0)
    {
        mdelay(cust_info->gpio_pre_dly);
    }

    bus_id      = cust_info->i2c_bus;
    slave_id    = cust_info->i2c_dev_addr;
    addr_count  = 1;
    burn_offset = cust_info->i2c_burn_offset;
    data_size   = 1;
    burn_cmd    = cust_info->i2c_burn_cmd;

    UBOOT_TRACE("I2C write MultiByte bus_id=%d, slave_id=0x%02x, burn_offset=%d, burn_cmd=ox%x\n ", bus_id, slave_id, burn_offset, burn_cmd);
    ret = _mtk_pnl_cust_iic_write(slave_id, addr_count, &burn_offset, data_size, &burn_cmd);
    if (ret < 0)
    {
        UBOOT_DEBUG("write iic error!! bus_id=%d slave_id=0x%02x \n", bus_id, slave_id);
        return FALSE;
    }

    if (cust_info->gpio_post_dly > 0)
    {
        mdelay(cust_info->gpio_post_dly);
    }

    if (cust_info->gpio_post_num > 0)
    {
        switch (cust_info->gpio_post_ops)
        {
            case 0:
            {
                _mtk_pnl_gpio_set_low(cust_info->gpio_post_num);
                UBOOT_TRACE("GPIO[%d] Ouput Low\n", cust_info->gpio_post_num);
                break;
            }
            case 1:
            {
                _mtk_pnl_gpio_set_high(cust_info->gpio_post_num);
                UBOOT_TRACE("GPIO[%d] Ouput High\n", cust_info->gpio_post_num);
                break;
            }
            case 0xFF:
            default:
            {
                _mtk_pnl_gpio_set_input(cust_info->gpio_post_num);
                UBOOT_TRACE("GPIO[%d] Input Hi-Z\n", cust_info->gpio_post_num);
                break;
            }
        }
    }

    UBOOT_TRACE("OK\n");
    return TRUE;
}

#define RETRY_MAX 3
int force_reset(void)
{
    udelay (50000);             /* wait 50 ms */

    disable_interrupts();

    reset_misc();
    reset_cpu(0);

    /*NOTREACHED*/
    return 0;
}

static int _mtk_pnl_cust_pmic_auto_update_from_flash(st_cust_ic_info *pmic_info, unsigned char *r_data)
{
    int retval = -EINVAL;
    bool checkstatus = TRUE;
    bool checkautpgammastatus = TRUE;
    int i = 0;
    unsigned char *wr_data = NULL;
    bool check_write_status = FALSE;
    unsigned char* pmin_bin_backup = NULL;
    char wr_buf;
    int pm_boot_reason;
    unsigned char check_byte = 255;
    unsigned char check_byte_offset;

    if (pmic_info == NULL)
    {
        UBOOT_TRACE("malloc fail!\n");
        return -ENOMEM;
    }

    if (r_data == NULL)
    {
        UBOOT_TRACE("r_data is NULL!\n");
        return -ENOMEM;
    }

    wr_data = malloc(pmic_size);
    if (wr_data == NULL)
    {
        UBOOT_TRACE("wr_data is NULL!\n");
        return -ENOMEM;
    }

    if(pmic_info->nvm_chk_en == 1)
    {
        _mtk_pnl_cust_iic_init(pmic_info->i2c_bus);
        check_byte_offset = (unsigned char)pmic_info->nvm_chk_offset;
        retval = _mtk_pnl_cust_iic_read(pmic_info->i2c_dev_addr, 1, &check_byte_offset, 1, &check_byte);
        UBOOT_TRACE("I2C read bus_id=%d, slave_id=0x%02x, offset=0x%02x, check_byte=%d\n ", pmic_info->i2c_bus, pmic_info->i2c_dev_addr, check_byte_offset, check_byte);
        if (retval < 0)
        {
            check_byte = pmic_info->nvm_chk_val;
            UBOOT_ERROR("[%d][%d] _mtk_pnl_cust_iic_read 0xE0 fail, force update PMIC setting!\n", __LINE__, retval);
        }
    }

    if (pmic_info->auto_update_from_flash)
    {
        pmin_bin_backup = malloc(pmic_size);
        if (pmin_bin_backup == NULL)
        {
            UBOOT_TRACE("pmin_bin_backup is NULL!\n");
            return -ENOMEM;
        }
        //backup pmic_bin_buf before auto-p.
        memcpy(pmin_bin_backup, pmic_bin_buf, pmic_size);

        for (i=0; i<RETRY_MAX; i++)
        {
            retval = _mtk_pnl_cust_ic_auto_p_gamma(pmic_info, pmic_bin_buf, pmic_size, r_data);
            if (retval == 0)
                break;
            printf("Retry for _mtk_pnl_cust_ic_auto_p_gamma!!!! retval=%d, retries=%d!!!!!!!!!\n", retval, i+1);
        }
        if (retval < 0)
        {
            UBOOT_ERROR("[%d][%d] pmic auto update from flash fail, use default data!\n", __LINE__, retval);
        }
        if ((pmic_bin_buf != NULL) && (pmic_size > 0))
        {
            UBOOT_TRACE("[%d] pmic auto update from flash:\n", __LINE__);
            UBOOT_DUMP(pmic_bin_buf, pmic_size);

            checkautpgammastatus = _mtk_pnl_cust_ic_checksum(pmic_info, r_data, pmic_bin_buf, pmic_size);
            UBOOT_TRACE("pmic auto gamma checksum : %s\n", checkautpgammastatus ? "pass": "fail");
            if (!checkautpgammastatus || (check_byte == pmic_info->nvm_chk_val))
            {
                UBOOT_TRACE("checksum fail! need update PMIC setting!\n");
                for (i=0; i<RETRY_MAX; i++)
                {
                    memset(wr_data, 0 , pmic_size);
                    if (pmic_info->bin_format_type == 0)
                    {
                        _mtk_pnl_cust_ic_write(pmic_bin_buf, pmic_size, pmic_info);
                    }
                    else if (pmic_info->bin_format_type == 1)
                    {
                        _mtk_pnl_cust_ic_write_by_mask(pmic_bin_buf, pmic_size, pmic_info);
                    }
                    else
                    {
                        retval = -ERANGE;
                        UBOOT_TRACE("not support this format:%d \n", pmic_info->bin_format_type);
                    }
                    if (pmic_info->bin_format_type == 0)
                    {
                        _mtk_pnl_cust_ic_read(wr_data, pmic_size, pmic_info);
                    }
                    else if (pmic_info->bin_format_type == 1)
                    {
                        _mtk_pnl_cust_ic_read_by_mask(wr_data, pmic_size, pmic_info);
                    }
                    check_write_status = _mtk_pnl_cust_ic_checksum(pmic_info, wr_data, pmic_bin_buf, pmic_size);
                    if (check_write_status == TRUE)
                        break;

                    printf("Write => Read Checksum fail on %d tries !!!!!!\n", i+1);
                    pmic_info->gpio_pre_dly += 10;
                    pmic_info->gpio_post_dly += 10;
                }
                if (0)
                {
                    printf("Dump data from r_data:\n");
                    UBOOT_DUMP_FORCE(r_data, pmic_size);
                    printf("Dump data for I2C write out data:\n");
                    UBOOT_DUMP_FORCE(pmic_bin_buf, pmic_size);
                    printf("Dump data for I2C read back data:\n");
                    UBOOT_DUMP_FORCE(wr_data, pmic_size);
                    printf("Retry check failed, reset device!!!!\n");
                    if (!g_TCONLESS_FORCE_RESET_FLAG)
                    {
                        /* store current boot reason */
                        pm_boot_reason = pm_get_boot_reason();
                        UBOOT_INFO("Store current boot reason, 0x%x\n", pm_boot_reason);
                        wr_buf = (char)pm_boot_reason;
                        env_set("save_boot_reason", &wr_buf);
                        env_save();
                        pm_set_boot_reason(PM_BR_TCONLESS_FORCE_RESET);
                        mtk_panel_enable_vcc(FALSE);
                        mdelay(1000);
                        puts ("Force reset by PMIC flow failed!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
                        force_reset();
                    }
                    else
                    {
                        printf("Retry reset still failed, use default PMIC bin !!!!\n");
                        if (pmin_bin_backup != NULL)
                        {
                            if (pmic_info->bin_format_type == 0)
                            {
                                _mtk_pnl_cust_ic_write(pmin_bin_backup, pmic_size, pmic_info);
                            }
                            else if (pmic_info->bin_format_type == 1)
                            {
                                _mtk_pnl_cust_ic_write_by_mask(pmin_bin_backup, pmic_size, pmic_info);
                            }
                            else
                            {
                                retval = -ERANGE;
                                UBOOT_TRACE("not support this format:%d \n", pmic_info->bin_format_type);
                            }
                        }
                    }
                }
            }
        }
        UBOOT_TRACE("[%d] pmic auto update from flash success!\n", __LINE__);
    }
    else
    {
        //check bin
        checkstatus = _mtk_pnl_cust_ic_checksum(pmic_info, r_data, pmic_bin_buf, pmic_size);
        UBOOT_TRACE("pmic bin checksum : %s\n", checkstatus ? "pass": "fail");
        if (!checkstatus || (check_byte == pmic_info->nvm_chk_val))
        {
            UBOOT_TRACE("checksum fail! need update PMIC setting!\n");
            for (i=0; i<RETRY_MAX; i++)
            {
                memset(wr_data, 0 , pmic_size);
                if (pmic_info->bin_format_type == 0)
                {
                    _mtk_pnl_cust_ic_write(pmic_bin_buf, pmic_size, pmic_info);
                }
                else if (pmic_info->bin_format_type == 1)
                {
                    _mtk_pnl_cust_ic_write_by_mask(pmic_bin_buf, pmic_size, pmic_info);
                }
                else
                {
                    retval = -ERANGE;
                    UBOOT_TRACE("not support this format:%d \n", pmic_info->bin_format_type);
                }
                if (pmic_info->bin_format_type == 0)
                {
                    _mtk_pnl_cust_ic_read(wr_data, pmic_size, pmic_info);
                }
                else if (pmic_info->bin_format_type == 1)
                {
                    _mtk_pnl_cust_ic_read_by_mask(wr_data, pmic_size, pmic_info);
                }
                check_write_status = _mtk_pnl_cust_ic_checksum(pmic_info, wr_data, pmic_bin_buf, pmic_size);
                if (check_write_status == TRUE)
                    break;

                printf("Write => Read Checksum fail on %d tries !!!!!!\n", i+1);
                pmic_info->gpio_pre_dly += 10;
                pmic_info->gpio_post_dly += 10;
            }
            if (0)
            {
                printf("Dump data from r_data:\n");
                UBOOT_DUMP_FORCE(r_data, pmic_size);
                printf("Dump data for I2C write out data:\n");
                UBOOT_DUMP_FORCE(pmic_bin_buf, pmic_size);
                printf("Dump data for I2C read back data:\n");
                UBOOT_DUMP_FORCE(wr_data, pmic_size);
                printf("Retry check failed, reset device!!!!\n");
                if (!g_TCONLESS_FORCE_RESET_FLAG)
                {
                    /* store current boot reason */
                    pm_boot_reason = pm_get_boot_reason();
                    UBOOT_INFO("Store current boot reason, 0x%x\n", pm_boot_reason);
                    wr_buf = (char)pm_boot_reason;
                    env_set("save_boot_reason", &wr_buf);
                    env_save();
                    pm_set_boot_reason(PM_BR_TCONLESS_FORCE_RESET);
                    mtk_panel_enable_vcc(FALSE);
                    mdelay(1000);
                    puts ("Force reset by PMIC flow failed!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
                    force_reset();
                }
                else
                {
                    printf("Retry reset still failed, use default PMIC bin !!!!\n");
                }
            }
        }
    }

    if ((pmic_info->with_nvm == 1) && (!checkstatus || !checkautpgammastatus))
    {
        retval = _mtk_pnl_cust_ic_burn(pmic_info);
    }

    if(check_write_status == TRUE)
    {
        if((pmic_info->nvm_chk_i2c_post_dly != 0) || (pmic_info->nvm_chk_rst_dly != 0))
        {
            mdelay(pmic_info->nvm_chk_i2c_post_dly);
            mtk_panel_enable_vcc(FALSE);
            mdelay(pmic_info->nvm_chk_rst_dly);
            puts ("Force reset by PMIC update!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
            force_reset();
        }
    }

    if (wr_data != NULL)
    {
        free(wr_data);
        wr_data = NULL;
    }
    if (pmin_bin_backup != NULL)
    {
        free(pmin_bin_backup);
        pmin_bin_backup = NULL;
    }
    return retval;
}

void _mtk_pnl_cust_set_default_post_gpio(st_cust_ic_info *cust_ic_info)
{
    if (cust_ic_info->gpio_post_num > 0)
        {
            switch (cust_ic_info->gpio_post_ops)
            {
                case 0:
                {
                    _mtk_pnl_gpio_set_low(cust_ic_info->gpio_post_num);
                    UBOOT_TRACE("GPIO[%d] Ouput Low\n", cust_ic_info->gpio_post_num);
                    break;
                }
                case 1:
                {
                    _mtk_pnl_gpio_set_high(cust_ic_info->gpio_post_num);
                    UBOOT_TRACE("GPIO[%d] Ouput High\n", cust_ic_info->gpio_post_num);
                    break;
                }
                case 0xFF:
                default:
                {
                    _mtk_pnl_gpio_set_input(cust_ic_info->gpio_post_num);
                    UBOOT_TRACE("GPIO[%d] Input Hi-Z\n", cust_ic_info->gpio_post_num);
                    break;
                }
            }
        }
}

static int _mtk_pnl_cust_pmic_init(st_multi_cust_ic_info *multi_cust_ic)
{
    st_cust_ic_info *pmic_info = NULL;
    unsigned char *r_data = NULL;
    int retval = -EINVAL;
    int i = 0;

    if (multi_cust_ic == NULL)
    {
        UBOOT_TRACE("[%s][%d] invalid data!\n", __FUNCTION__, __LINE__);
        retval = -EINVAL;
        return retval;
    }
    if ((pmic_size > 0) && (pmic_bin_buf != NULL))
    {
        pmic_info = malloc(sizeof(st_cust_ic_info));
        if (pmic_info == NULL)
        {
            UBOOT_TRACE("malloc fail!\n");
            retval = -ENOMEM;
            goto pmic_ic_init_finish;
        }

        pmic_info->info_version           = multi_cust_ic->pmic_info.info_version;
        pmic_info->ic_type                = multi_cust_ic->pmic_info.ic_type;
        pmic_info->bin_format_type        = multi_cust_ic->pmic_info.bin_format_type;
        pmic_info->force_init             = multi_cust_ic->pmic_info.force_init;
        pmic_info->with_nvm               = multi_cust_ic->pmic_info.with_nvm;
        pmic_info->i2c_bus                = multi_cust_ic->pmic_info.i2c_bus;
        pmic_info->i2c_mode               = multi_cust_ic->pmic_info.i2c_mode;
        pmic_info->i2c_dev_addr           = multi_cust_ic->pmic_info.i2c_dev_addr;
        pmic_info->i2c_reg_offset         = multi_cust_ic->pmic_info.i2c_reg_offset;
        pmic_info->vcc_dly                = multi_cust_ic->pmic_info.vcc_dly;
        pmic_info->gpio_pre_num           = multi_cust_ic->pmic_info.gpio_pre_num;
        pmic_info->gpio_pre_ops           = multi_cust_ic->pmic_info.gpio_pre_ops;
        pmic_info->gpio_pre_dly           = multi_cust_ic->pmic_info.gpio_pre_dly;
        pmic_info->gpio_post_num          = multi_cust_ic->pmic_info.gpio_post_num;
        pmic_info->gpio_post_ops          = multi_cust_ic->pmic_info.gpio_post_ops;
        pmic_info->gpio_post_dly          = multi_cust_ic->pmic_info.gpio_post_dly;
        pmic_info->auto_update_from_flash = multi_cust_ic->pmic_info.auto_update_from_flash;
        pmic_info->read_mode              = multi_cust_ic->pmic_info.read_mode;
        pmic_info->write_mode             = multi_cust_ic->pmic_info.write_mode;
        pmic_info->i2c_burn_cmd           = multi_cust_ic->pmic_info.i2c_burn_cmd;
        pmic_info->i2c_burn_offset        = multi_cust_ic->pmic_info.i2c_burn_offset;
        pmic_info->i2c_ctrl_reg           = multi_cust_ic->pmic_info.i2c_ctrl_reg;
        pmic_info->i2c_ctrl_reg_offset    = multi_cust_ic->pmic_info.i2c_ctrl_reg_offset;
        pmic_info->data_start             = multi_cust_ic->pmic_info.data_start;
        pmic_info->data_end               = multi_cust_ic->pmic_info.data_end;
        pmic_info->nvm_chk_en             = multi_cust_ic->pmic_info.nvm_chk_en;
        pmic_info->nvm_chk_offset         = multi_cust_ic->pmic_info.nvm_chk_offset;
        pmic_info->nvm_chk_val            = multi_cust_ic->pmic_info.nvm_chk_val;
        pmic_info->nvm_chk_i2c_post_dly   = multi_cust_ic->pmic_info.nvm_chk_i2c_post_dly;
        pmic_info->nvm_chk_rst_dly        = multi_cust_ic->pmic_info.nvm_chk_rst_dly;
        pmic_info->checksum_bypass_offset = multi_cust_ic->pmic_info.checksum_bypass_offset;
        pmic_info->checksum_bypass_size   = multi_cust_ic->pmic_info.checksum_bypass_size;

        //set default post gpio
        _mtk_pnl_cust_set_default_post_gpio(pmic_info);

        if (pmic_info->with_nvm == 0)
        {
            if (pmic_info->bin_format_type == 0)
            {
                _mtk_pnl_cust_ic_write(pmic_bin_buf, pmic_size, pmic_info);
            }
            else if (pmic_info->bin_format_type == 1)
            {
                _mtk_pnl_cust_ic_write_by_mask(pmic_bin_buf, pmic_size, pmic_info);
            }
            else
            {
                retval = -ERANGE;
                UBOOT_TRACE("not support this format:%d \n", pmic_info->bin_format_type);
            }
        }

        r_data = malloc(pmic_size);
        if (r_data == NULL)
        {
            UBOOT_TRACE("malloc fail!\n");
            retval = -ENOMEM;
            goto pmic_ic_init_finish;
        }
        else
        {
            memset(r_data, 0 , pmic_size);
        }
        for (i=0; i<RETRY_MAX; i++)
        {
            if (pmic_info->bin_format_type == 0)
            {
                retval = _mtk_pnl_cust_ic_read(r_data, pmic_size, pmic_info);
            }
            else if (pmic_info->bin_format_type == 1)
            {
                _mtk_pnl_cust_ic_read_by_mask(r_data, pmic_size, pmic_info);
            }
            else
            {
                retval = -ERANGE;
                UBOOT_TRACE("not support this format:%d \n", pmic_info->bin_format_type);
            }
            if (retval == 0)
                break;
            printf("Retry for read PMIC value %d times\n", i+1);
            mdelay(100);
        }

        retval = _mtk_pnl_cust_pmic_auto_update_from_flash(pmic_info, r_data);

        UBOOT_TRACE("end of pmic initialization. \n");
        goto pmic_ic_init_finish;
    }

    return retval;

pmic_ic_init_finish:
    if (pmic_info != NULL)
    {
        free(pmic_info);
        pmic_info = NULL;
    }
    if (r_data != NULL)
    {
        free(r_data);
        r_data = NULL;
    }
    return retval;
}

static int _mtk_pnl_cust_pgamma_init_auto_update_from_flash(st_cust_ic_info *pgamma_info, unsigned char *r_data)
{
    int retval = -EINVAL;
    bool checkstatus = TRUE;
    bool checkautpgammastatus = TRUE;

    if (pgamma_info == NULL)
    {
        UBOOT_TRACE("malloc fail!\n");
        return -ENOMEM;
    }

    if (r_data == NULL)
    {
        UBOOT_TRACE("r_data is NULL!\n");
        return -ENOMEM;
    }

    if (pgamma_info->auto_update_from_flash)
    {
        retval = _mtk_pnl_cust_ic_auto_p_gamma(pgamma_info, pgamma_bin_buf, pgamma_size, r_data);
        if (retval)
        {
            UBOOT_TRACE("[%d][%d] pgamma auto update from flash fail, use default data!\n", __LINE__, retval);
        }
        else
        {
            if ((pgamma_bin_buf != NULL) && (pgamma_size > 0))
            {
                UBOOT_TRACE("[%d] pgamma after auto update from flash:\n", __LINE__);
                UBOOT_DUMP(pgamma_bin_buf, pgamma_size);
                checkautpgammastatus = _mtk_pnl_cust_ic_checksum(pgamma_info, r_data, pgamma_bin_buf, pgamma_size);
                UBOOT_TRACE("pgamma auto gamma checksum : %s\n", checkautpgammastatus ? "pass": "fail");
                if (!checkautpgammastatus)
                {
                    if (pgamma_info->bin_format_type == 0)
                    {
                        _mtk_pnl_cust_ic_write(pgamma_bin_buf, pgamma_size, pgamma_info);
                    }
                    else if (pgamma_info->bin_format_type == 1)
                    {
                        _mtk_pnl_cust_ic_write_by_mask(pgamma_bin_buf, pgamma_size, pgamma_info);
                    }
                    else
                    {
                        retval = -ERANGE;
                        UBOOT_TRACE("not support this format:%d \n", pgamma_info->bin_format_type);
                    }
                }
            }
            UBOOT_TRACE("[%d] pgamma auto update from flash success!\n",  __LINE__);
        }
    }
    else
    {
        //check bin
        checkstatus = _mtk_pnl_cust_ic_checksum(pgamma_info, r_data, pgamma_bin_buf, pgamma_size);
        UBOOT_TRACE("pgamma bin checksum : %s\n", checkstatus ? "pass": "fail");
        if (!checkstatus)
        {
            UBOOT_TRACE("checksum fail! need update PGAMMA setting!\n");
            if (pgamma_info->bin_format_type == 0)
            {
                _mtk_pnl_cust_ic_write(pgamma_bin_buf, pgamma_size, pgamma_info);
            }
            else if (pgamma_info->bin_format_type == 1)
            {
                _mtk_pnl_cust_ic_write_by_mask(pgamma_bin_buf, pgamma_size, pgamma_info);
            }
            else
            {
                retval = -ERANGE;
                UBOOT_TRACE("not support this format:%d \n", pgamma_info->bin_format_type);
            }
        }
    }

    if ((pgamma_info->with_nvm == 1) && (!checkstatus || !checkautpgammastatus))
    {
        retval = _mtk_pnl_cust_ic_burn(pgamma_info);
    }
    return retval;
}

static int _mtk_pnl_cust_pgamma_init(st_multi_cust_ic_info *multi_cust_ic)
{
    st_cust_ic_info *pgamma_info = NULL;
    unsigned char *r_data = NULL;
    int retval = -EINVAL;

    if (multi_cust_ic == NULL)
    {
        UBOOT_TRACE("[%s][%d] invalid data!\n", __FUNCTION__,__LINE__);
        retval = -EINVAL;
        return retval;
    }
    if ((pgamma_size > 0) && (pgamma_bin_buf != NULL))
    {
        pgamma_info = malloc(sizeof(st_cust_ic_info));
        if (pgamma_info == NULL)
        {
            UBOOT_TRACE("malloc fail!\n");
            retval = -ENOMEM;
            goto pgamma_ic_init_finish;
        }

        pgamma_info->info_version           = multi_cust_ic->pgamma_info.info_version;
        pgamma_info->ic_type                = multi_cust_ic->pgamma_info.ic_type;
        pgamma_info->bin_format_type        = multi_cust_ic->pgamma_info.bin_format_type;
        pgamma_info->force_init             = multi_cust_ic->pgamma_info.force_init;
        pgamma_info->with_nvm               = multi_cust_ic->pgamma_info.with_nvm;
        pgamma_info->i2c_bus                = multi_cust_ic->pgamma_info.i2c_bus;
        pgamma_info->i2c_mode               = multi_cust_ic->pgamma_info.i2c_mode;
        pgamma_info->i2c_dev_addr           = multi_cust_ic->pgamma_info.i2c_dev_addr;
        pgamma_info->i2c_reg_offset         = multi_cust_ic->pgamma_info.i2c_reg_offset;
        pgamma_info->vcc_dly                = multi_cust_ic->pgamma_info.vcc_dly;
        pgamma_info->gpio_pre_num           = multi_cust_ic->pgamma_info.gpio_pre_num;
        pgamma_info->gpio_pre_ops           = multi_cust_ic->pgamma_info.gpio_pre_ops;
        pgamma_info->gpio_pre_dly           = multi_cust_ic->pgamma_info.gpio_pre_dly;
        pgamma_info->gpio_post_num          = multi_cust_ic->pgamma_info.gpio_post_num;
        pgamma_info->gpio_post_ops          = multi_cust_ic->pgamma_info.gpio_post_ops;
        pgamma_info->gpio_post_dly          = multi_cust_ic->pgamma_info.gpio_post_dly;
        pgamma_info->auto_update_from_flash = multi_cust_ic->pgamma_info.auto_update_from_flash;
        pgamma_info->read_mode              = multi_cust_ic->pgamma_info.read_mode;
        pgamma_info->write_mode             = multi_cust_ic->pgamma_info.write_mode;
        pgamma_info->i2c_burn_cmd           = multi_cust_ic->pgamma_info.i2c_burn_cmd;
        pgamma_info->i2c_burn_offset        = multi_cust_ic->pgamma_info.i2c_burn_offset;
        pgamma_info->i2c_ctrl_reg           = multi_cust_ic->pgamma_info.i2c_ctrl_reg;
        pgamma_info->i2c_ctrl_reg_offset    = multi_cust_ic->pgamma_info.i2c_ctrl_reg_offset;
        pgamma_info->data_start             = multi_cust_ic->pgamma_info.data_start;
        pgamma_info->data_end               = multi_cust_ic->pgamma_info.data_end;
        pgamma_info->checksum_bypass_offset = multi_cust_ic->pgamma_info.checksum_bypass_offset;
        pgamma_info->checksum_bypass_size   = multi_cust_ic->pgamma_info.checksum_bypass_size;

        //set default post gpio
        _mtk_pnl_cust_set_default_post_gpio(pgamma_info);

        if (pgamma_info->with_nvm == 0)
        {
            if (pgamma_info->bin_format_type == 0)
            {
                _mtk_pnl_cust_ic_write(pgamma_bin_buf, pgamma_size, pgamma_info);
            }
            else if (pgamma_info->bin_format_type == 1)
            {
                _mtk_pnl_cust_ic_write_by_mask(pgamma_bin_buf, pgamma_size, pgamma_info);
            }
            else
            {
                retval = -ERANGE;
                UBOOT_TRACE("not support this format:%d \n", pgamma_info->bin_format_type);
            }
        }

        r_data = malloc(pgamma_size);
        if (r_data == NULL)
        {
            UBOOT_TRACE("malloc fail!\n");
            retval = -ENOMEM;
            goto pgamma_ic_init_finish;
        }
        else
        {
            memset(r_data, 0 , pgamma_size);
        }
        if (pgamma_info->bin_format_type == 0)
        {
            _mtk_pnl_cust_ic_read(r_data, pgamma_size, pgamma_info);
        }
        else if (pgamma_info->bin_format_type == 1)
        {
            _mtk_pnl_cust_ic_read_by_mask(r_data, pgamma_size, pgamma_info);
        }
        else
        {
            retval = -ERANGE;
            UBOOT_TRACE("not support this format:%d \n", pgamma_info->bin_format_type);
        }

        retval = _mtk_pnl_cust_pgamma_init_auto_update_from_flash(pgamma_info, r_data);

        UBOOT_TRACE("end of pgamma initialization. \n");
        goto pgamma_ic_init_finish;
    }

    return retval;

pgamma_ic_init_finish:
    if (pgamma_info != NULL)
    {
        free(pgamma_info);
        pgamma_info = NULL;
    }
    if (r_data != NULL)
    {
        free(r_data);
        r_data = NULL;
    }
    return retval;
}

static int _mtk_pnl_cust_levelshifit_init_auto_update_from_flash(st_cust_ic_info *ls_info, unsigned char *r_data)
{
    int retval = -EINVAL;
    bool checkstatus = TRUE;
    bool checkautpgammastatus = TRUE;

    if (ls_info == NULL)
    {
        UBOOT_TRACE("malloc fail!\n");
        return -ENOMEM;
    }

    if (r_data == NULL)
    {
        UBOOT_TRACE("r_data is NULL!\n");
        return -ENOMEM;
    }

    if (ls_info->auto_update_from_flash)
    {
        retval = _mtk_pnl_cust_ic_auto_p_gamma(ls_info, ls_bin_buf, ls_size, r_data);
        if (retval)
        {
            UBOOT_TRACE("[%d][%d] level shift auto update from flash fail, use default data!\n", __LINE__, retval);
        }
        else
        {
            if ((ls_bin_buf != NULL) && (ls_size > 0))
            {
                UBOOT_TRACE("[%d] level shift auto update from flash:\n", __LINE__);
                UBOOT_DUMP(ls_bin_buf, ls_size);
                checkautpgammastatus = _mtk_pnl_cust_ic_checksum(ls_info, r_data, ls_bin_buf, ls_size);
                UBOOT_TRACE("level shift auto gamma checksum : %s\n", checkautpgammastatus ? "pass": "fail");
                if (!checkautpgammastatus)
                {
                    if (ls_info->bin_format_type == 0)
                    {
                        _mtk_pnl_cust_ic_write(ls_bin_buf, ls_size, ls_info);
                    }
                    else if (ls_info->bin_format_type == 1)
                    {
                        _mtk_pnl_cust_ic_write_by_mask(ls_bin_buf, ls_size, ls_info);
                    }
                    else
                    {
                        retval = -ERANGE;
                        UBOOT_TRACE("not support this format:%d \n", ls_info->bin_format_type);
                    }
                }
            }
            UBOOT_TRACE("[%d] level shift auto update from flash success!\n",  __LINE__);
        }
    }
    else
    {
        //check bin
        checkstatus = _mtk_pnl_cust_ic_checksum(ls_info, r_data, ls_bin_buf, ls_size);
        UBOOT_TRACE("level shift bin checksum : %s\n", checkstatus ? "pass": "fail");
        if (!checkstatus)
        {
            UBOOT_TRACE("checksum fail! need update LEVEL SHIFT setting!\n");
            if (ls_info->bin_format_type == 0)
            {
                _mtk_pnl_cust_ic_write(ls_bin_buf, ls_size, ls_info);
            }
            else if (ls_info->bin_format_type == 1)
            {
                _mtk_pnl_cust_ic_write_by_mask(ls_bin_buf, ls_size, ls_info);
            }
            else
            {
                retval = -ERANGE;
                UBOOT_TRACE("not support this format:%d \n", ls_info->bin_format_type);
            }
        }
    }

    if ((ls_info->with_nvm == 1) && (!checkstatus || !checkautpgammastatus))
    {
        retval = _mtk_pnl_cust_ic_burn(ls_info);
    }
    return retval;
}

static int _mtk_pnl_cust_levelshifit_init(st_multi_cust_ic_info *multi_cust_ic)
{
    st_cust_ic_info *ls_info = NULL;
    unsigned char *r_data = NULL;
    int retval = -EINVAL;

    if (multi_cust_ic == NULL)
    {
        UBOOT_TRACE("[%s][%d] invalid data!\n", __FUNCTION__,__LINE__);
        retval = -EINVAL;
        return retval;
    }
    if ((ls_size > 0) && (ls_bin_buf != NULL))
    {
        ls_info = malloc(sizeof(st_cust_ic_info));
        if (ls_info == NULL)
        {
            UBOOT_TRACE("malloc fail!\n");
            retval = -ENOMEM;
            goto ls_ic_init_finish;
        }

        ls_info->info_version           = multi_cust_ic->levelshift_info.info_version;
        ls_info->ic_type                = multi_cust_ic->levelshift_info.ic_type;
        ls_info->bin_format_type        = multi_cust_ic->levelshift_info.bin_format_type;
        ls_info->force_init             = multi_cust_ic->levelshift_info.force_init;
        ls_info->with_nvm               = multi_cust_ic->levelshift_info.with_nvm;
        ls_info->i2c_bus                = multi_cust_ic->levelshift_info.i2c_bus;
        ls_info->i2c_mode               = multi_cust_ic->levelshift_info.i2c_mode;
        ls_info->i2c_dev_addr           = multi_cust_ic->levelshift_info.i2c_dev_addr;
        ls_info->i2c_reg_offset         = multi_cust_ic->levelshift_info.i2c_reg_offset;
        ls_info->vcc_dly                = multi_cust_ic->levelshift_info.vcc_dly;
        ls_info->gpio_pre_num           = multi_cust_ic->levelshift_info.gpio_pre_num;
        ls_info->gpio_pre_ops           = multi_cust_ic->levelshift_info.gpio_pre_ops;
        ls_info->gpio_pre_dly           = multi_cust_ic->levelshift_info.gpio_pre_dly;
        ls_info->gpio_post_num          = multi_cust_ic->levelshift_info.gpio_post_num;
        ls_info->gpio_post_ops          = multi_cust_ic->levelshift_info.gpio_post_ops;
        ls_info->gpio_post_dly          = multi_cust_ic->levelshift_info.gpio_post_dly;
        ls_info->auto_update_from_flash = multi_cust_ic->levelshift_info.auto_update_from_flash;
        ls_info->read_mode              = multi_cust_ic->levelshift_info.read_mode;
        ls_info->write_mode             = multi_cust_ic->levelshift_info.write_mode;
        ls_info->i2c_burn_cmd           = multi_cust_ic->levelshift_info.i2c_burn_cmd;
        ls_info->i2c_burn_offset        = multi_cust_ic->levelshift_info.i2c_burn_offset;
        ls_info->i2c_ctrl_reg           = multi_cust_ic->levelshift_info.i2c_ctrl_reg;
        ls_info->i2c_ctrl_reg_offset    = multi_cust_ic->levelshift_info.i2c_ctrl_reg_offset;
        ls_info->data_start             = multi_cust_ic->levelshift_info.data_start;
        ls_info->data_end               = multi_cust_ic->levelshift_info.data_end;
        ls_info->checksum_bypass_offset = multi_cust_ic->levelshift_info.checksum_bypass_offset;
        ls_info->checksum_bypass_size   = multi_cust_ic->levelshift_info.checksum_bypass_size;

        //set default post gpio
        _mtk_pnl_cust_set_default_post_gpio(ls_info);

        if (ls_info->with_nvm == 0)
        {
            if (ls_info->bin_format_type == 0)
            {
                _mtk_pnl_cust_ic_write(ls_bin_buf, ls_size, ls_info);
            }
            else if (ls_info->bin_format_type == 1)
            {
                _mtk_pnl_cust_ic_write_by_mask(ls_bin_buf, ls_size, ls_info);
            }
            else
            {
                retval = -ERANGE;
                UBOOT_TRACE("not support this format:%d \n", ls_info->bin_format_type);
            }
        }

        r_data = malloc(ls_size);
        if (r_data == NULL)
        {
            UBOOT_TRACE("malloc fail!\n");
            retval = -ENOMEM;
            goto ls_ic_init_finish;
        }
        else
        {
            memset(r_data, 0 , ls_size);
        }
        if (ls_info->bin_format_type == 0)
        {
            _mtk_pnl_cust_ic_read(r_data, ls_size, ls_info);
        }
        else if (ls_info->bin_format_type == 1)
        {
            _mtk_pnl_cust_ic_read_by_mask(r_data, ls_size, ls_info);
        }
        else
        {
            retval = -ERANGE;
            UBOOT_TRACE("not support this format:%d \n", ls_info->bin_format_type);
        }

        retval = _mtk_pnl_cust_levelshifit_init_auto_update_from_flash(ls_info, r_data);

        UBOOT_TRACE("end of levelshift initialization. \n");
        goto ls_ic_init_finish;
    }

    return retval;

ls_ic_init_finish:
    if (ls_info != NULL)
    {
        free(ls_info);
        ls_info = NULL;
    }
    if (r_data != NULL)
    {
        free(r_data);
        r_data = NULL;
    }
    return retval;
}

static int _mtk_pnl_cust_vcomic_init_auto_update_from_flash(st_cust_ic_info *vcomic_info, unsigned char *r_data)
{
    int retval = -EINVAL;
    bool checkstatus = TRUE;
    bool checkautpgammastatus = TRUE;

    if (vcomic_info == NULL)
    {
        UBOOT_TRACE("malloc fail!\n");
        return -ENOMEM;
    }

    if (r_data == NULL)
    {
        UBOOT_TRACE("r_data is NULL!\n");
        return -ENOMEM;
    }

    if (vcomic_info->auto_update_from_flash)
    {
        retval = _mtk_pnl_cust_ic_auto_p_gamma(vcomic_info, vcomic_bin_buf, vcomic_size, r_data);
        if (retval)
        {
            UBOOT_TRACE("[%d][%d] vcom ic auto update from flash fail, use default data!\n", __LINE__, retval);
        }
        else
        {
            if ((vcomic_bin_buf != NULL) && (vcomic_size > 0))
            {
                UBOOT_TRACE("[%d] vcom auto update from flash:\n", __LINE__);
                UBOOT_DUMP(vcomic_bin_buf, vcomic_size);
                checkautpgammastatus = _mtk_pnl_cust_ic_checksum(vcomic_info, r_data, vcomic_bin_buf, vcomic_size);
                UBOOT_TRACE("vcom auto gamma checksum : %s\n", checkautpgammastatus ? "pass": "fail");
                if (!checkautpgammastatus)
                {
                    if (vcomic_info->bin_format_type == 0)
                    {
                        _mtk_pnl_cust_ic_write(vcomic_bin_buf, vcomic_size, vcomic_info);
                    }
                    else if (vcomic_info->bin_format_type == 1)
                    {
                        _mtk_pnl_cust_ic_write_by_mask(vcomic_bin_buf, vcomic_size, vcomic_info);
                    }
                    else
                    {
                        retval = -ERANGE;
                        UBOOT_TRACE("not support this format:%d \n", vcomic_info->bin_format_type);
                    }
                }
            }
            UBOOT_TRACE("[%d] vcom ic auto update from flash success!\n",  __LINE__);
        }
    }
    else
    {
        //check bin
        checkstatus = _mtk_pnl_cust_ic_checksum(vcomic_info, r_data, vcomic_bin_buf, vcomic_size);
        UBOOT_TRACE("vcom bin checksum : %s\n", checkstatus ? "pass": "fail");
        if (!checkstatus)
        {
            UBOOT_TRACE("checksum fail! need update VCOM setting!\n");
            if (vcomic_info->bin_format_type == 0)
            {
                _mtk_pnl_cust_ic_write(vcomic_bin_buf, vcomic_size, vcomic_info);
            }
            else if (vcomic_info->bin_format_type == 1)
            {
                _mtk_pnl_cust_ic_write_by_mask(vcomic_bin_buf, vcomic_size, vcomic_info);
            }
            else
            {
                retval = -ERANGE;
                UBOOT_TRACE("not support this format:%d \n", vcomic_info->bin_format_type);
            }
        }
    }

    if ((vcomic_info->with_nvm == 1) && (!checkstatus || !checkautpgammastatus))
    {
        retval = _mtk_pnl_cust_ic_burn(vcomic_info);
    }
    return retval;
}

static int _mtk_pnl_cust_vcomic_init(st_multi_cust_ic_info *multi_cust_ic)
{
    st_cust_ic_info *vcomic_info = NULL;
    unsigned char *r_data = NULL;
    int retval = -EINVAL;

    if (multi_cust_ic == NULL)
    {
        UBOOT_TRACE("[%s][%d] invalid data!\n", __FUNCTION__, __LINE__);
        retval = -EINVAL;
        return retval;
    }
    if ((vcomic_size > 0) && (vcomic_bin_buf != NULL))
    {
        vcomic_info = malloc(sizeof(st_cust_ic_info));
        if (vcomic_info == NULL)
        {
            UBOOT_TRACE("malloc fail!\n");
            retval = -ENOMEM;
            goto vcomic_ic_init_finish;
        }

        vcomic_info->info_version           = multi_cust_ic->vcomic_info.info_version;
        vcomic_info->ic_type                = multi_cust_ic->vcomic_info.ic_type;
        vcomic_info->bin_format_type        = multi_cust_ic->vcomic_info.bin_format_type;
        vcomic_info->force_init             = multi_cust_ic->vcomic_info.force_init;
        vcomic_info->with_nvm               = multi_cust_ic->vcomic_info.with_nvm;
        vcomic_info->i2c_bus                = multi_cust_ic->vcomic_info.i2c_bus;
        vcomic_info->i2c_mode               = multi_cust_ic->vcomic_info.i2c_mode;
        vcomic_info->i2c_dev_addr           = multi_cust_ic->vcomic_info.i2c_dev_addr;
        vcomic_info->i2c_reg_offset         = multi_cust_ic->vcomic_info.i2c_reg_offset;
        vcomic_info->vcc_dly                = multi_cust_ic->vcomic_info.vcc_dly;
        vcomic_info->gpio_pre_num           = multi_cust_ic->vcomic_info.gpio_pre_num;
        vcomic_info->gpio_pre_ops           = multi_cust_ic->vcomic_info.gpio_pre_ops;
        vcomic_info->gpio_pre_dly           = multi_cust_ic->vcomic_info.gpio_pre_dly;
        vcomic_info->gpio_post_num          = multi_cust_ic->vcomic_info.gpio_post_num;
        vcomic_info->gpio_post_ops          = multi_cust_ic->vcomic_info.gpio_post_ops;
        vcomic_info->gpio_post_dly          = multi_cust_ic->vcomic_info.gpio_post_dly;
        vcomic_info->auto_update_from_flash = multi_cust_ic->vcomic_info.auto_update_from_flash;
        vcomic_info->read_mode              = multi_cust_ic->vcomic_info.read_mode;
        vcomic_info->write_mode             = multi_cust_ic->vcomic_info.write_mode;
        vcomic_info->i2c_burn_cmd           = multi_cust_ic->vcomic_info.i2c_burn_cmd;
        vcomic_info->i2c_burn_offset        = multi_cust_ic->vcomic_info.i2c_burn_offset;
        vcomic_info->i2c_ctrl_reg           = multi_cust_ic->vcomic_info.i2c_ctrl_reg;
        vcomic_info->i2c_ctrl_reg_offset    = multi_cust_ic->vcomic_info.i2c_ctrl_reg_offset;
        vcomic_info->data_start             = multi_cust_ic->vcomic_info.data_start;
        vcomic_info->data_end               = multi_cust_ic->vcomic_info.data_end;
        vcomic_info->checksum_bypass_offset = multi_cust_ic->vcomic_info.checksum_bypass_offset;
        vcomic_info->checksum_bypass_size   = multi_cust_ic->vcomic_info.checksum_bypass_size;

        //set default post gpio
        _mtk_pnl_cust_set_default_post_gpio(vcomic_info);

        if (vcomic_info->with_nvm == 0)
        {
            if (vcomic_info->bin_format_type == 0)
            {
                _mtk_pnl_cust_ic_write(vcomic_bin_buf, vcomic_size, vcomic_info);
            }
            else if (vcomic_info->bin_format_type == 1)
            {
                _mtk_pnl_cust_ic_write_by_mask(vcomic_bin_buf, vcomic_size, vcomic_info);
            }
            else
            {
                retval = -ERANGE;
                UBOOT_TRACE("not support this format:%d \n", vcomic_info->bin_format_type);
            }
        }

        r_data = malloc(vcomic_size);
        if (r_data == NULL)
        {
            UBOOT_TRACE("malloc fail!\n");
            retval = -ENOMEM;
            goto vcomic_ic_init_finish;
        }
        else
        {
            memset(r_data, 0 , vcomic_size);
        }
        if (vcomic_info->bin_format_type == 0)
        {
            _mtk_pnl_cust_ic_read(r_data, vcomic_size, vcomic_info);
        }
        else if (vcomic_info->bin_format_type == 1)
        {
            _mtk_pnl_cust_ic_read_by_mask(r_data, vcomic_size, vcomic_info);
        }
        else
        {
            retval = -ERANGE;
            UBOOT_TRACE("not support this format:%d \n", vcomic_info->bin_format_type);
        }

        retval = _mtk_pnl_cust_vcomic_init_auto_update_from_flash(vcomic_info, r_data);

        UBOOT_TRACE("end of vcom ic initialization. \n");
        goto vcomic_ic_init_finish;
    }

    return retval;

vcomic_ic_init_finish:
    if (vcomic_info != NULL)
    {
        free(vcomic_info);
        vcomic_info = NULL;
    }
    if (r_data != NULL)
    {
        free(r_data);
        r_data = NULL;
    }
    return retval;
}

static int _mtk_pnl_cust_ic_sub_init_auto_update_from_flash(st_cust_ic_info *cust_ic_info, unsigned char* sub_bin_buf, unsigned short sub_size, unsigned char *r_data)
{
    int retval = -EINVAL;
    bool checkstatus = TRUE;
    bool checkautpgammastatus = TRUE;

    if (cust_ic_info == NULL)
    {
        UBOOT_TRACE("malloc fail!\n");
        return -ENOMEM;
    }

    if (r_data == NULL)
    {
        UBOOT_TRACE("r_data is NULL!\n");
        return -ENOMEM;
    }

    if (cust_ic_info->auto_update_from_flash)
    {
        retval = _mtk_pnl_cust_ic_auto_p_gamma(cust_ic_info, sub_bin_buf, sub_size, r_data);
        if (retval)
        {
            UBOOT_TRACE("[%d][%d] sub ic=[%d] auto update from flash fail, use default data!\n", __LINE__, retval, cust_ic_info->ic_type);
        }
        else
        {
            if ((sub_bin_buf != NULL) && (sub_size > 0))
            {
                UBOOT_TRACE("[%d] sub ic=[%d] after auto update from flash:\n", __LINE__, cust_ic_info->ic_type);
                UBOOT_DUMP(sub_bin_buf, sub_size);
                checkautpgammastatus = _mtk_pnl_cust_ic_checksum(cust_ic_info, r_data, sub_bin_buf, sub_size);
                UBOOT_TRACE("[%d] sub ic auto gamma checksum : %s\n", cust_ic_info->bin_format_type, checkautpgammastatus ? "pass": "fail");
                if (!checkautpgammastatus)
                {
                    if (cust_ic_info->bin_format_type == 0)
                    {
                        _mtk_pnl_cust_ic_write(sub_bin_buf, sub_size, cust_ic_info);
                    }
                    else if (cust_ic_info->bin_format_type == 1)
                    {
                        _mtk_pnl_cust_ic_write_by_mask(sub_bin_buf, sub_size, cust_ic_info);
                    }
                    else
                    {
                         retval = -ERANGE;
                         UBOOT_TRACE("not support this format:%d \n", cust_ic_info->bin_format_type);
                    }
                }
            }
            UBOOT_TRACE("[%d] ic=[%d] auto update from flash success!\n",  __LINE__, cust_ic_info->ic_type);
        }
    }
    else
    {
        //check bin
        checkstatus = _mtk_pnl_cust_ic_checksum(cust_ic_info, r_data, sub_bin_buf, sub_size);
        UBOOT_TRACE("[%d] sub ic bin checksum : %s\n", cust_ic_info->bin_format_type, checkstatus ? "pass": "fail");
        if (!checkstatus)
        {
            UBOOT_TRACE("checksum fail! need update VCOM setting!\n");
            if (cust_ic_info->bin_format_type == 0)
            {
                _mtk_pnl_cust_ic_write(sub_bin_buf, sub_size, cust_ic_info);
            }
            else if (cust_ic_info->bin_format_type == 1)
            {
                _mtk_pnl_cust_ic_write_by_mask(sub_bin_buf, sub_size, cust_ic_info);
            }
            else
            {
                retval = -ERANGE;
                UBOOT_TRACE("not support this format:%d \n", cust_ic_info->bin_format_type);
            }
        }
    }

    if ((cust_ic_info->with_nvm == 1) && (!checkstatus || !checkautpgammastatus))
    {
        retval = _mtk_pnl_cust_ic_burn(cust_ic_info);
    }
    return retval;
}

static int _mtk_pnl_cust_ic_sub_set_info(st_multi_cust_ic_info *multi_cust_ic, en_pnl_cust_ic_type en_type,
            st_cust_ic_info *cust_ic_info, unsigned char* sub_bin_buf, unsigned short sub_size)
{
    int retval = -EINVAL;

    if (en_type == E_PNL_CUST_IC_SECOND_PMIC)
    {
        cust_ic_info->info_version           = multi_cust_ic->pmic_sub_info.info_version;
        cust_ic_info->ic_type                = multi_cust_ic->pmic_sub_info.ic_type;
        cust_ic_info->bin_format_type        = multi_cust_ic->pmic_sub_info.bin_format_type;
        cust_ic_info->force_init             = multi_cust_ic->pmic_sub_info.force_init;
        cust_ic_info->with_nvm               = multi_cust_ic->pmic_sub_info.with_nvm;
        cust_ic_info->i2c_bus                = multi_cust_ic->pmic_sub_info.i2c_bus;
        cust_ic_info->i2c_mode               = multi_cust_ic->pmic_sub_info.i2c_mode;
        cust_ic_info->i2c_dev_addr           = multi_cust_ic->pmic_sub_info.i2c_dev_addr;
        cust_ic_info->i2c_reg_offset         = multi_cust_ic->pmic_sub_info.i2c_reg_offset;
        cust_ic_info->vcc_dly                = multi_cust_ic->pmic_sub_info.vcc_dly;
        cust_ic_info->gpio_pre_num           = multi_cust_ic->pmic_sub_info.gpio_pre_num;
        cust_ic_info->gpio_pre_ops           = multi_cust_ic->pmic_sub_info.gpio_pre_ops;
        cust_ic_info->gpio_pre_dly           = multi_cust_ic->pmic_sub_info.gpio_pre_dly;
        cust_ic_info->gpio_post_num          = multi_cust_ic->pmic_sub_info.gpio_post_num;
        cust_ic_info->gpio_post_ops          = multi_cust_ic->pmic_sub_info.gpio_post_ops;
        cust_ic_info->gpio_post_dly          = multi_cust_ic->pmic_sub_info.gpio_post_dly;
        cust_ic_info->auto_update_from_flash = multi_cust_ic->pmic_sub_info.auto_update_from_flash;
        cust_ic_info->read_mode              = multi_cust_ic->pmic_sub_info.read_mode;
        cust_ic_info->write_mode             = multi_cust_ic->pmic_sub_info.write_mode;
        cust_ic_info->i2c_burn_cmd           = multi_cust_ic->pmic_sub_info.i2c_burn_cmd;
        cust_ic_info->i2c_burn_offset        = multi_cust_ic->pmic_sub_info.i2c_burn_offset;
        cust_ic_info->i2c_ctrl_reg           = multi_cust_ic->pmic_sub_info.i2c_ctrl_reg;
        cust_ic_info->i2c_ctrl_reg_offset    = multi_cust_ic->pmic_sub_info.i2c_ctrl_reg_offset;
        cust_ic_info->data_start             = multi_cust_ic->pmic_sub_info.data_start;
        cust_ic_info->data_end               = multi_cust_ic->pmic_sub_info.data_end;
        cust_ic_info->checksum_bypass_offset = multi_cust_ic->pmic_sub_info.checksum_bypass_offset;
        cust_ic_info->checksum_bypass_size   = multi_cust_ic->pmic_sub_info.checksum_bypass_size;
    }
    else if (en_type == E_PNL_CUST_IC_SECOND_PGAMMAIC)
    {
        cust_ic_info->info_version           = multi_cust_ic->pgamma_sub_info.info_version;
        cust_ic_info->ic_type                = multi_cust_ic->pgamma_sub_info.ic_type;
        cust_ic_info->bin_format_type        = multi_cust_ic->pgamma_sub_info.bin_format_type;
        cust_ic_info->force_init             = multi_cust_ic->pgamma_sub_info.force_init;
        cust_ic_info->with_nvm               = multi_cust_ic->pgamma_sub_info.with_nvm;
        cust_ic_info->i2c_bus                = multi_cust_ic->pgamma_sub_info.i2c_bus;
        cust_ic_info->i2c_mode               = multi_cust_ic->pgamma_sub_info.i2c_mode;
        cust_ic_info->i2c_dev_addr           = multi_cust_ic->pgamma_sub_info.i2c_dev_addr;
        cust_ic_info->i2c_reg_offset         = multi_cust_ic->pgamma_sub_info.i2c_reg_offset;
        cust_ic_info->vcc_dly                = multi_cust_ic->pgamma_sub_info.vcc_dly;
        cust_ic_info->gpio_pre_num           = multi_cust_ic->pgamma_sub_info.gpio_pre_num;
        cust_ic_info->gpio_pre_ops           = multi_cust_ic->pgamma_sub_info.gpio_pre_ops;
        cust_ic_info->gpio_pre_dly           = multi_cust_ic->pgamma_sub_info.gpio_pre_dly;
        cust_ic_info->gpio_post_num          = multi_cust_ic->pgamma_sub_info.gpio_post_num;
        cust_ic_info->gpio_post_ops          = multi_cust_ic->pgamma_sub_info.gpio_post_ops;
        cust_ic_info->gpio_post_dly          = multi_cust_ic->pgamma_sub_info.gpio_post_dly;
        cust_ic_info->auto_update_from_flash = multi_cust_ic->pgamma_sub_info.auto_update_from_flash;
        cust_ic_info->read_mode              = multi_cust_ic->pgamma_sub_info.read_mode;
        cust_ic_info->write_mode             = multi_cust_ic->pgamma_sub_info.write_mode;
        cust_ic_info->i2c_burn_cmd           = multi_cust_ic->pgamma_sub_info.i2c_burn_cmd;
        cust_ic_info->i2c_burn_offset        = multi_cust_ic->pgamma_sub_info.i2c_burn_offset;
        cust_ic_info->i2c_ctrl_reg           = multi_cust_ic->pgamma_sub_info.i2c_ctrl_reg;
        cust_ic_info->i2c_ctrl_reg_offset    = multi_cust_ic->pgamma_sub_info.i2c_ctrl_reg_offset;
        cust_ic_info->data_start             = multi_cust_ic->pgamma_sub_info.data_start;
        cust_ic_info->data_end               = multi_cust_ic->pgamma_sub_info.data_end;
        cust_ic_info->checksum_bypass_offset = multi_cust_ic->pgamma_sub_info.checksum_bypass_offset;
        cust_ic_info->checksum_bypass_size   = multi_cust_ic->pgamma_sub_info.checksum_bypass_size;
    }
    else if (en_type == E_PNL_CUST_IC_SECOND_LEVELSHIFTIC)
    {
        cust_ic_info->info_version           = multi_cust_ic->levelshift_sub_info.info_version;
        cust_ic_info->ic_type                = multi_cust_ic->levelshift_sub_info.ic_type;
        cust_ic_info->bin_format_type        = multi_cust_ic->levelshift_sub_info.bin_format_type;
        cust_ic_info->force_init             = multi_cust_ic->levelshift_sub_info.force_init;
        cust_ic_info->with_nvm               = multi_cust_ic->levelshift_sub_info.with_nvm;
        cust_ic_info->i2c_bus                = multi_cust_ic->levelshift_sub_info.i2c_bus;
        cust_ic_info->i2c_mode               = multi_cust_ic->levelshift_sub_info.i2c_mode;
        cust_ic_info->i2c_dev_addr           = multi_cust_ic->levelshift_sub_info.i2c_dev_addr;
        cust_ic_info->i2c_reg_offset         = multi_cust_ic->levelshift_sub_info.i2c_reg_offset;
        cust_ic_info->vcc_dly                = multi_cust_ic->levelshift_sub_info.vcc_dly;
        cust_ic_info->gpio_pre_num           = multi_cust_ic->levelshift_sub_info.gpio_pre_num;
        cust_ic_info->gpio_pre_ops           = multi_cust_ic->levelshift_sub_info.gpio_pre_ops;
        cust_ic_info->gpio_pre_dly           = multi_cust_ic->levelshift_sub_info.gpio_pre_dly;
        cust_ic_info->gpio_post_num          = multi_cust_ic->levelshift_sub_info.gpio_post_num;
        cust_ic_info->gpio_post_ops          = multi_cust_ic->levelshift_sub_info.gpio_post_ops;
        cust_ic_info->gpio_post_dly          = multi_cust_ic->levelshift_sub_info.gpio_post_dly;
        cust_ic_info->auto_update_from_flash = multi_cust_ic->levelshift_sub_info.auto_update_from_flash;
        cust_ic_info->read_mode              = multi_cust_ic->levelshift_sub_info.read_mode;
        cust_ic_info->write_mode             = multi_cust_ic->levelshift_sub_info.write_mode;
        cust_ic_info->i2c_burn_cmd           = multi_cust_ic->levelshift_sub_info.i2c_burn_cmd;
        cust_ic_info->i2c_burn_offset        = multi_cust_ic->levelshift_sub_info.i2c_burn_offset;
        cust_ic_info->i2c_ctrl_reg           = multi_cust_ic->levelshift_sub_info.i2c_ctrl_reg;
        cust_ic_info->i2c_ctrl_reg_offset    = multi_cust_ic->levelshift_sub_info.i2c_ctrl_reg_offset;
        cust_ic_info->data_start             = multi_cust_ic->levelshift_sub_info.data_start;
        cust_ic_info->data_end               = multi_cust_ic->levelshift_sub_info.data_end;
        cust_ic_info->checksum_bypass_offset = multi_cust_ic->levelshift_sub_info.checksum_bypass_offset;
        cust_ic_info->checksum_bypass_size   = multi_cust_ic->levelshift_sub_info.checksum_bypass_size;
    }
    else if (en_type == E_PNL_CUST_IC_SECOND_VCOMIC)
    {
        cust_ic_info->info_version           = multi_cust_ic->vcomic_sub_info.info_version;
        cust_ic_info->ic_type                = multi_cust_ic->vcomic_sub_info.ic_type;
        cust_ic_info->bin_format_type        = multi_cust_ic->vcomic_sub_info.bin_format_type;
        cust_ic_info->force_init             = multi_cust_ic->vcomic_sub_info.force_init;
        cust_ic_info->with_nvm               = multi_cust_ic->vcomic_sub_info.with_nvm;
        cust_ic_info->i2c_bus                = multi_cust_ic->vcomic_sub_info.i2c_bus;
        cust_ic_info->i2c_mode               = multi_cust_ic->vcomic_sub_info.i2c_mode;
        cust_ic_info->i2c_dev_addr           = multi_cust_ic->vcomic_sub_info.i2c_dev_addr;
        cust_ic_info->i2c_reg_offset         = multi_cust_ic->vcomic_sub_info.i2c_reg_offset;
        cust_ic_info->vcc_dly                = multi_cust_ic->vcomic_sub_info.vcc_dly;
        cust_ic_info->gpio_pre_num           = multi_cust_ic->vcomic_sub_info.gpio_pre_num;
        cust_ic_info->gpio_pre_ops           = multi_cust_ic->vcomic_sub_info.gpio_pre_ops;
        cust_ic_info->gpio_pre_dly           = multi_cust_ic->vcomic_sub_info.gpio_pre_dly;
        cust_ic_info->gpio_post_num          = multi_cust_ic->vcomic_sub_info.gpio_post_num;
        cust_ic_info->gpio_post_ops          = multi_cust_ic->vcomic_sub_info.gpio_post_ops;
        cust_ic_info->gpio_post_dly          = multi_cust_ic->vcomic_sub_info.gpio_post_dly;
        cust_ic_info->auto_update_from_flash = multi_cust_ic->vcomic_sub_info.auto_update_from_flash;
        cust_ic_info->read_mode              = multi_cust_ic->vcomic_sub_info.read_mode;
        cust_ic_info->write_mode             = multi_cust_ic->vcomic_sub_info.write_mode;
        cust_ic_info->i2c_burn_cmd           = multi_cust_ic->vcomic_sub_info.i2c_burn_cmd;
        cust_ic_info->i2c_burn_offset        = multi_cust_ic->vcomic_sub_info.i2c_burn_offset;
        cust_ic_info->i2c_ctrl_reg           = multi_cust_ic->vcomic_sub_info.i2c_ctrl_reg;
        cust_ic_info->i2c_ctrl_reg_offset    = multi_cust_ic->vcomic_sub_info.i2c_ctrl_reg_offset;
        cust_ic_info->data_start             = multi_cust_ic->vcomic_sub_info.data_start;
        cust_ic_info->data_end               = multi_cust_ic->vcomic_sub_info.data_end;
        cust_ic_info->checksum_bypass_offset = multi_cust_ic->vcomic_sub_info.checksum_bypass_offset;
        cust_ic_info->checksum_bypass_size   = multi_cust_ic->vcomic_sub_info.checksum_bypass_size;
    }
    else
    {
        retval = -ERANGE;
        return retval;
    }

    //set default post gpio
    _mtk_pnl_cust_set_default_post_gpio(cust_ic_info);

    if (cust_ic_info->with_nvm == 0)
    {
        if (cust_ic_info->bin_format_type == 0)
        {
            _mtk_pnl_cust_ic_write(sub_bin_buf, sub_size, cust_ic_info);
        }
        else if (cust_ic_info->bin_format_type == 1)
        {
            _mtk_pnl_cust_ic_write_by_mask(sub_bin_buf, sub_size, cust_ic_info);
        }
        else
        {
            retval = -ERANGE;
            UBOOT_TRACE("not support this format:%d \n", cust_ic_info->bin_format_type);
        }
    }
    return retval;
}

static int _mtk_pnl_cust_ic_sub_init_set_r_data(unsigned char *r_data, unsigned short sub_size, st_cust_ic_info *cust_ic_info)
{
    int retval = -EINVAL;

    if (r_data == NULL || cust_ic_info == NULL)
    {
        UBOOT_TRACE("malloc fail!\n");
        return -ENOMEM;
    }
    else
    {
        memset(r_data, 0 , sub_size);
    }
    if (cust_ic_info->bin_format_type == 0)
    {
        _mtk_pnl_cust_ic_read(r_data, sub_size, cust_ic_info);
    }
    else if (cust_ic_info->bin_format_type == 1)
    {
        _mtk_pnl_cust_ic_read_by_mask(r_data, sub_size, cust_ic_info);
    }
    else
    {
        retval = -ERANGE;
        UBOOT_TRACE("not support this format:%d \n", cust_ic_info->bin_format_type);
    }
    return retval;
}

static int _mtk_pnl_cust_ic_sub_init(st_multi_cust_ic_info *multi_cust_ic, en_pnl_cust_ic_type en_type, unsigned char* sub_bin_buf, unsigned short sub_size)
{
    st_cust_ic_info *cust_ic_info = NULL;
    unsigned char *r_data = NULL;
    int retval = -EINVAL;

    if ((multi_cust_ic == NULL) ||  (en_type < E_PNL_CUST_IC_NONE) || (en_type > E_PNL_CUST_IC_MAX))
    {
            UBOOT_TRACE("[%d] invalid data!\n", __LINE__);
            retval = -EINVAL;
            return retval;
    }

    if ((sub_size > 0) && (sub_bin_buf != NULL))
    {
        cust_ic_info = malloc(sizeof(st_cust_ic_info));
        if (cust_ic_info == NULL)
        {
            UBOOT_TRACE("malloc fail!\n");
            retval = -ENOMEM;
            goto sub_cust_ic_init_finish;
        }

        retval = _mtk_pnl_cust_ic_sub_set_info(multi_cust_ic, en_type, cust_ic_info, sub_bin_buf, sub_size);
        if (retval == -ERANGE)
        {
            goto sub_cust_ic_init_finish;
        }

        r_data = malloc(sub_size);
        retval = _mtk_pnl_cust_ic_sub_init_set_r_data(r_data, sub_size, cust_ic_info);
        if (retval == -ENOMEM)
        {
            goto sub_cust_ic_init_finish;
        }

        retval = _mtk_pnl_cust_ic_sub_init_auto_update_from_flash(cust_ic_info, sub_bin_buf, sub_size, r_data);

        UBOOT_TRACE("end of sub cust ic [%d] initialization. \n", cust_ic_info->ic_type);
        goto sub_cust_ic_init_finish;
    }

    UBOOT_TRACE("[%d] invalid data!\n", __LINE__);
    retval = -EINVAL;
    return retval;

sub_cust_ic_init_finish:
    if (cust_ic_info != NULL)
    {
        free(cust_ic_info);
        cust_ic_info = NULL;
    }
    if (r_data != NULL)
    {
        free(r_data);
        r_data = NULL;
    }
    return retval;
}

static int _mtk_pnl_cust_auto_panel_gamma(void)
{
    UBOOT_TRACE("IN\n");
    int ret;
    int nAutoPga = 0;
    struct udevice *dev;
    MS_U8 *u8AutoPga = NULL;
    MS_U16  u16SampleCrc = 0, u16CalCrc = 0;

    ret = uclass_get_device_by_name(UCLASS_DISPLAY, "video_out", &dev);
    if (!ret)
    {
        if (display_get_property(dev, E_PNL_PROP_AUTO_PGA_UPDATE, &nAutoPga) == 0)
        {
            UBOOT_TRACE("Auto PGA select=%d \n", nAutoPga);
        }
    }
    else
    {
        UBOOT_ERROR("Get video out device is fail\n");
        ret = -EINVAL;
        goto AUTOPANELGAMMAEXIT;
    }

    if (nAutoPga == 0)
    {
        UBOOT_TRACE("Auto PGA is disable.\n");
         ret = -EINVAL;
         goto AUTOPANELGAMMAEXIT;
    }

    /*read panel gamma curve from SPI flash*/
    u8AutoPga = (MS_U8*)malloc(AUTO_PANEL_GAMMA_TOTAL_SIZE);
    if (u8AutoPga == NULL)
    {
        UBOOT_ERROR("malloc fail!\n");
        ret = -ENOMEM;
        goto AUTOPANELGAMMAEXIT;
    }
    memset(u8AutoPga, 0x00, AUTO_PANEL_GAMMA_TOTAL_SIZE);

#if CONFIG_SPI_FLASH
    if (init_spi_flash() != TRUE)
    {
        UBOOT_TRACE("init_spi_flash error!\n");
        ret = -ENXIO;
        goto AUTOPANELGAMMAEXIT;
    }
    if (u8AutoPga != NULL)
    {
        ret = read_spi_flash(u8AutoPga, AUTO_PANEL_GAMMA_DATA_ADDR, AUTO_PANEL_GAMMA_TOTAL_SIZE);
        u16SampleCrc = (u8AutoPga[AUTO_PANEL_GAMMA_TOTAL_SIZE - 2] << 8) | u8AutoPga[AUTO_PANEL_GAMMA_TOTAL_SIZE - 1];
        UBOOT_DUMP(u8AutoPga, PANEL_GAMMA_MAINHEADER_LEN);
        UBOOT_TRACE("u16SampleCrc = 0x%x\n", u16SampleCrc);
    }
#else
    UBOOT_TRACE("need CONFIG_SPI_FLASH!\n");
    ret = -ENXIO;
    goto AUTOPANELGAMMAEXIT;
#endif

    if ((ret == TRUE) && (u16SampleCrc != 0))
    {
        u16CalCrc = CSOT_Cal_CRC16(u8AutoPga, AUTO_PANEL_GAMMA_DATA_SIZE);
        if (u16CalCrc != u16SampleCrc)
        {
            UBOOT_TRACE("CRC16 fail!!! u16CalCrc=0x%x, u16SampleCrc=0x%x\n", u16CalCrc, u16SampleCrc);
            ret = FALSE;
            goto AUTOPANELGAMMAEXIT;
        }
        else
        {
            UBOOT_DEBUG("CRC16 succeeded, u16SampleCrc=0x%x \n", u16SampleCrc);
        }
    }

    /*set panel gamma curve*/
    ret = display_set_property(dev, E_SET_PNL_PROP_AUTO_PGA_120Hz, u8AutoPga);
    if (ret)
    {
        UBOOT_DEBUG("set property [%d] fail = %d \n", E_SET_PNL_PROP_AUTO_PGA_120Hz, ret);
    }

AUTOPANELGAMMAEXIT:
    if (u8AutoPga != NULL)
    {
        free(u8AutoPga);
        u8AutoPga = NULL;
    }
    UBOOT_TRACE("OUT\n");
    return ret;
}

static int _mtk_pnl_cust_auto_update_driver_settings(void)
{
    struct udevice *dev;
    int ntype = 0;
    EN_AUTO_P2P_CMD_TYPE eType = 0;
    MS_U8 pu8ReadBuff[AUTO_P2P_CMD_BUFFER_SIZE] = {0};
    MS_U8 pu8SetBuff[AUTO_P2P_CMD_BUFFER_SIZE] = {0};
    MS_U16 u16SampleCrc = 0, u16CalCrc = 0;
    MS_U16 u16CheckSum = 0;
    MS_U8 u8Index = 0;
    int ret;

    UBOOT_TRACE("IN\n");
    ret = uclass_get_device_by_name(UCLASS_DISPLAY, "video_out", &dev);
    if (!ret)
    {
        if (display_get_property(dev, E_PNL_PROP_AUTO_P2P_CMD_UPDATE, &ntype) == 0)
        {
            UBOOT_TRACE("Auto P2P cmd update=%d \n", ntype);
        }
    }
    else
    {
        UBOOT_ERROR("Get video out device is fail\n");
        ret = -EINVAL;
        return ret;
    }

    if (ntype == 0)
    {
        UBOOT_TRACE("Auto P2P cmd update is disable.\n");
        ret = -EINVAL;
        return ret;
    }
    eType = (EN_AUTO_P2P_CMD_TYPE)ntype;

#if CONFIG_SPI_FLASH
    if (init_spi_flash() != TRUE)
    {
        UBOOT_TRACE("init_spi_flash error!\n");
        ret = -ENXIO;
        return ret;
    }
    //step 1: read data from flash
    switch (eType)
    {
        case EN_AUTO_P2P_CMD_TYPE_CSOT_GAMMA:
        {
            ret = read_spi_flash(pu8ReadBuff, CSPI_GAMMA_ADDR, CSPI_GAMMA_TOTAL_SIZE);
        }
        break;
        case EN_AUTO_P2P_CMD_TYPE_H_K_C_GAMMA:
        {
            ret = read_spi_flash(pu8ReadBuff, ISP_HKC_GAMMA_ADDR, ISP_HKC_GAMMA_TOTAL_SIZE);
        }
        break;
        case EN_AUTO_P2P_CMD_TYPE_USIT_GAMMA:
        {
            ret = read_spi_flash(pu8ReadBuff, USIT_GAMMA_ADDR, USIT_GAMMA_TOTAL_SIZE);
        }
        break;
        default:
        {
            UBOOT_DEBUG("Not support Auto P2P CMD Update [%d]\n", eType);
        }
        break;
    }
#else
    UBOOT_TRACE("need CONFIG_SPI_FLASH!\n");
    ret = -ENXIO;
    return ret;
#endif
    //step 2: Checksum
    switch (eType)
    {
        case EN_AUTO_P2P_CMD_TYPE_CSOT_GAMMA:
        {
            u16SampleCrc = (pu8ReadBuff[CSPI_GAMMA_TOTAL_SIZE - 2] << 8) |
                pu8ReadBuff[CSPI_GAMMA_TOTAL_SIZE - 1];
            UBOOT_DUMP(pu8ReadBuff, CSPI_GAMMA_TOTAL_SIZE);
            UBOOT_TRACE("u16SampleCrc = 0x%x\n", u16SampleCrc);
            if ((ret == TRUE) && (u16SampleCrc != 0))
            {
                u16CalCrc = CSOT_Cal_CRC16(pu8ReadBuff, CSPI_GAMMA_TOTAL_SIZE - 2);
                if (u16CalCrc != u16SampleCrc)
                {
                    UBOOT_TRACE("CRC16 fail!!! u16CalCrc=0x%x, u16SampleCrc=0x%x\n", u16CalCrc, u16SampleCrc);
                    ret = -EINVAL;
                    return ret;
                }
                else
                {
                    UBOOT_DEBUG("CRC16 succeeded, u16SampleCrc=0x%x \n", u16SampleCrc);
                }
            }
        }
        break;
        case EN_AUTO_P2P_CMD_TYPE_H_K_C_GAMMA:
        {
            UBOOT_DUMP(pu8ReadBuff, ISP_HKC_GAMMA_TOTAL_SIZE);
            for (u8Index = 0; u8Index < ISP_HKC_GAMMA_TOTAL_SIZE - 2; u8Index++)
            {
                u16CheckSum += pu8ReadBuff[u8Index];
            }
            if ((u16CheckSum & 0xFF) != pu8ReadBuff[ISP_HKC_GAMMA_TOTAL_SIZE - 1])
            {
                UBOOT_TRACE("Checksum fail!!! u16CheckSum=0x%x, ReadCheckSum=0x%x\n",
                    u16CheckSum, pu8ReadBuff[ISP_HKC_GAMMA_TOTAL_SIZE - 1]);
                ret = -EINVAL;
                return ret;
            }
            else
            {
                UBOOT_DEBUG("CheckSum succeeded, u16CheckSum=0x%x \n", u16CheckSum);
            }
        }
        break;
        case EN_AUTO_P2P_CMD_TYPE_USIT_GAMMA:
        {
            u16SampleCrc = (pu8ReadBuff[USIT_GAMMA_TOTAL_SIZE - 2] << 8) |
                pu8ReadBuff[USIT_GAMMA_TOTAL_SIZE - 1];
            UBOOT_DUMP(pu8ReadBuff, USIT_GAMMA_TOTAL_SIZE);
            UBOOT_TRACE("u16SampleCrc = 0x%x\n", u16SampleCrc);
            if ((ret == TRUE) && (u16SampleCrc != 0))
            {
                u16CalCrc = CSOT_Cal_CRC16(pu8ReadBuff, USIT_GAMMA_TOTAL_SIZE - 2);
                if (u16CalCrc != u16SampleCrc)
                {
                    UBOOT_TRACE("CRC16 fail!!! u16CalCrc=0x%x, u16SampleCrc=0x%x\n", u16CalCrc, u16SampleCrc);
                    ret = -EINVAL;
                    return ret;
                }
                else
                {
                    UBOOT_DEBUG("CRC16 succeeded, u16SampleCrc=0x%x \n", u16SampleCrc);
                }
            }
        }
        break;
        default:
        {
            UBOOT_DEBUG("Not support Auto P2P CMD Update [%d]\n", eType);
        }
        break;
    }

    //step 3: re-mapping data
    switch (eType)
    {
        case EN_AUTO_P2P_CMD_TYPE_CSOT_GAMMA:
        case EN_AUTO_P2P_CMD_TYPE_H_K_C_GAMMA:
        {
            //re-mapping data, reg 21-22 keep 0
            for (u8Index = 0; u8Index < CSPI_GAMMA_DATA_SIZE / 2; u8Index++)
            {
                pu8SetBuff[u8Index] = pu8ReadBuff[u8Index];
            }
            for (u8Index = CSPI_GAMMA_DATA_SIZE / 2; u8Index < CSPI_GAMMA_DATA_SIZE; u8Index++)
            {
                pu8SetBuff[u8Index + 2] = pu8ReadBuff[u8Index];
            }
        }
        break;
        case EN_AUTO_P2P_CMD_TYPE_USIT_GAMMA:
        {
            for (u8Index = 0; u8Index < USIT_GAMMA_DATA_SIZE; u8Index++)
            {
                pu8SetBuff[u8Index] = pu8ReadBuff[u8Index];
            }
        }
        break;
        default:
        {
            UBOOT_DEBUG("Not support Auto P2P CMD Update [%d]\n", eType);
        }
        break;
    }

    //step 4:set p2p cmd
    switch (eType)
    {
        case EN_AUTO_P2P_CMD_TYPE_CSOT_GAMMA:
        case EN_AUTO_P2P_CMD_TYPE_H_K_C_GAMMA:
        {
            UBOOT_TRACE("CSPI/ISP Gamma dump:\n");
            UBOOT_DUMP(pu8SetBuff, CSPI_GAMMA_TOTAL_SIZE);
            ret = display_set_property(dev, E_SET_PNL_PROP_CSPI_CMD_UPDATE, pu8SetBuff);
            if (ret)
            {
                UBOOT_DEBUG("set property [%d]fail \n", E_SET_PNL_PROP_CSPI_CMD_UPDATE);
                return ret;
            }
        }
        break;
        case EN_AUTO_P2P_CMD_TYPE_USIT_GAMMA:
        {
            UBOOT_TRACE("USIT Gamma dump:\n");
            UBOOT_DUMP(pu8SetBuff, USIT_GAMMA_TOTAL_SIZE);
            ret = display_set_property(dev, E_SET_PNL_PROP_USIT_CMD_UPDATE, pu8SetBuff);
            if (ret)
            {
                UBOOT_DEBUG("set property [%d]fail \n", E_SET_PNL_PROP_USIT_CMD_UPDATE);
                return ret;
            }
        }
        break;
        default:
        {
            UBOOT_DEBUG("Not support Auto P2P CMD Update [%d]\n", eType);
        }
        break;
    }

    UBOOT_TRACE("OK\n");
    return ret;
}

int _mtk_pnl_cust_pmic_override_vcom(st_multi_cust_ic_info *multi_cust_ic)
{
    st_cust_pmic_vcom_info *pInfo = NULL;
    uint32_t data_index = 0, i;

    if (!multi_cust_ic)
    {
        UBOOT_TRACE("[%s][%d] invalid data!\n", __FUNCTION__,__LINE__);
        return -ENXIO;
    }

    if (!pmic_bin_buf)
    {
        UBOOT_TRACE("[%s][%d] invalid pmic bin data!\n", __FUNCTION__,__LINE__);
        return -ENXIO;
    }

    pInfo = &(multi_cust_ic->pmic_vcom_info);

    //The VCOM data adjusted in emmc is implemented to override the VCOM default data in the PMIC.
    //pmic_bin_buf: the parameter is original pmic data
    //pmic_size: the parameter is original pmic data length
    if (pInfo && pInfo->override_enable)
    {
        UBOOT_TRACE("[%s][%d] addr_start=%d addr_lengh=%d\n",
                __FUNCTION__, __LINE__, pInfo->addr_start, pInfo->addr_length);

        for (i = 0; i < pInfo->addr_length; i++)
        {
            data_index = pInfo->addr_start + i;

            if (data_index < pmic_size)
            {
                UBOOT_TRACE("[%s][%d] pmic_bin_buf[%d]=0x%X\n",
                    __FUNCTION__, __LINE__, data_index, pmic_bin_buf[data_index]);

                //do override...

            }
            else
            {
                UBOOT_TRACE("[%s][%d] Override addr over the pmic length\n",
                    __FUNCTION__, __LINE__);
            }
        }
    }
    else
    {
        UBOOT_TRACE("[%s][%d] Override is disable\n", __FUNCTION__, __LINE__);
    }

    return 0;
}

int _mtk_pnl_cust_ic_override_data(st_multi_cust_ic_info *multi_cust_ic)
{
    if (!multi_cust_ic) {
        UBOOT_TRACE("[%s][%d] invalid data!\n", __FUNCTION__,__LINE__);
        return -ENXIO;
    }

    _mtk_pnl_cust_pmic_override_vcom(multi_cust_ic);

    return 0;
}

int _mtk_pnl_is_support_vrr_mode(st_cust_tcon_info *tcon_info)
{
    if(tcon_info->bVRR_HighFrameRateMode_Support ||
       tcon_info->bVRR_HighPixelClockMode_Support ||
       tcon_info->bGAME_Direct60HzMode_Support)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

int mtk_pnl_is_dlg_mode(void)
{
    int ret = 0;
    struct panel_dlg_info panel_dlg_info;

    memset(&panel_dlg_info, 0, sizeof(panel_dlg_info));

    ret = get_panel_dlg_info(PANEL_DLG_PARTITION_PATH, PANEL_DLG_INI_PATH, &panel_dlg_info);
    if((ret >= 0) && (panel_dlg_info.panel_dlg == 1))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }

}

int _mtk_pnl_is_game_direct_mode(void)
{
    int ret = 0;
    struct panel_dlg_info panel_dlg_info;

    memset(&panel_dlg_info, 0, sizeof(panel_dlg_info));

    ret = get_panel_dlg_info(PANEL_DLG_PARTITION_PATH, PANEL_DLG_INI_PATH, &panel_dlg_info);
    if((ret >= 0) && (panel_dlg_info.panel_dlg == 2))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }

}

/*
------------------------------------------------------------------
VCC --> onTiming1 delay --> Data --> onTiming2 delay --> Backlight
------------------------------------------------------------------
*/

int mtk_pnl_cust_settings_befor_vcc(st_multi_cust_ic_info *multi_cust_ic)
{
	mtk_panel_test_on_init();
	UBOOT_TRACE("cust settings befor vcc!\n");
	return FALSE;
}

int mtk_pnl_cust_settings_vcc_ontiming1(st_multi_cust_ic_info *multi_cust_ic)
{
	UBOOT_TRACE("cust settings vcc --> ontiming1!\n");

    // 1.get bin file
    if(multi_cust_ic == NULL){
        UBOOT_TRACE("[%s][%d] invalid data!\n", __FUNCTION__,__LINE__);
        return FALSE;
    }

    _mtk_pnl_cust_ic_get_bin_file(multi_cust_ic);

    //The new data that can override the default data of the bin file.
    _mtk_pnl_cust_ic_override_data(multi_cust_ic);

    if(multi_cust_ic->pmic_info.force_init == 1){
        _mtk_pnl_cust_pmic_init(multi_cust_ic);
    }
    if (multi_cust_ic->pmic_sub_info.force_init == 1)
    {
        _mtk_pnl_cust_ic_sub_init(multi_cust_ic, E_PNL_CUST_IC_SECOND_PMIC, pmic_sub_bin_buf, pmic_sub_size);
    }

    if(multi_cust_ic->pgamma_info.force_init == 1){
        _mtk_pnl_cust_pgamma_init(multi_cust_ic);
    }
    if (multi_cust_ic->pgamma_sub_info.force_init == 1)
    {
        _mtk_pnl_cust_ic_sub_init(multi_cust_ic, E_PNL_CUST_IC_SECOND_PGAMMAIC, pgamma_sub_bin_buf, pgamma_sub_size);
    }

    if(multi_cust_ic->levelshift_info.force_init == 1){
        _mtk_pnl_cust_levelshifit_init(multi_cust_ic);
    }
    if (multi_cust_ic->levelshift_sub_info.force_init == 1)
    {
        _mtk_pnl_cust_ic_sub_init(multi_cust_ic, E_PNL_CUST_IC_SECOND_LEVELSHIFTIC, ls_sub_bin_buf, ls_sub_size);
    }

    if(multi_cust_ic->vcomic_info.force_init == 1){
        _mtk_pnl_cust_vcomic_init(multi_cust_ic);
    }
    if (multi_cust_ic->vcomic_sub_info.force_init == 1)
    {
        _mtk_pnl_cust_ic_sub_init(multi_cust_ic, E_PNL_CUST_IC_SECOND_VCOMIC, vcomic_sub_bin_buf, vcomic_sub_size);
    }

    return TRUE;
}

int mtk_pnl_cust_settings_ontiming1_data(st_multi_cust_ic_info *multi_cust_ic)
{
	UBOOT_TRACE("cust settings ontiming1 --> data!\n");
    // do auto panel gamma
    _mtk_pnl_cust_auto_panel_gamma();
    // do auto update driver settings, such as CSPI-Gamma, CEDS SOE
    _mtk_pnl_cust_auto_update_driver_settings();
    return TRUE;
}

int mtk_pnl_cust_settings_data_ontiming2(st_multi_cust_ic_info *multi_cust_ic)
{
	UBOOT_TRACE("cust settings data --> ontiming2!\n");
	return FALSE;
}

int mtk_pnl_cust_settings_ontiming2_backlight(st_multi_cust_ic_info *multi_cust_ic)
{
	UBOOT_TRACE("cust settings ontiming2 --> backlight!\n");
	return FALSE;
}

int mtk_pnl_cust_settings_after_backlight(st_multi_cust_ic_info *multi_cust_ic)
{
	UBOOT_TRACE("cust settings after backlight!\n");
	return FALSE;
}

int mtk_pnl_cust_set_panel_mode(st_cust_tcon_info *tcon_info)
{
    uint16_t bus_id = 0;
    uint16_t slave_id = 0;
    unsigned long addr_count = 0;
    unsigned char reg_offset = 0;
    unsigned long data_size = 0;
    unsigned char *write_data;
    uint16_t bus_id_sec = 0;
    uint16_t slave_id_sec = 0;
    unsigned long addr_count_sec = 0;
    unsigned char reg_offset_sec = 0;
    unsigned long data_size_sec = 0;
    unsigned char *write_data_sec;
    int ret = 0;

    UBOOT_TRACE("mtk_pnl_cust_set_panel_mode!\n");
    if(_mtk_pnl_is_support_vrr_mode(tcon_info) == FALSE)
    {
        UBOOT_TRACE("This panel not support VRR mode. skip it\n");
        return FALSE;
    }

    // Mode Change GPIO setting
    if((mtk_pnl_is_dlg_mode() == TRUE) && (tcon_info->bSupportGpioModeChange) && (tcon_info->u16GPIO_NUM_MODE_CHG))
    {
        if(tcon_info->u16GPIO_NUM_MODE_CHG > 0)
        {
            switch(tcon_info->ucGPIO_CTRL_MODE_CHG_ON)
            {
                case 0:
                {
                    _mtk_pnl_gpio_set_low(tcon_info->u16GPIO_NUM_MODE_CHG);
                    UBOOT_TRACE("GPIO[%d] Ouput Low\n",tcon_info->u16GPIO_NUM_MODE_CHG);
                    break;
                }
                case 1:
                {
                    _mtk_pnl_gpio_set_high(tcon_info->u16GPIO_NUM_MODE_CHG);
                    UBOOT_TRACE("GPIO[%d] Ouput High\n",tcon_info->u16GPIO_NUM_MODE_CHG);
                    break;
                }
                case 0xFF:
                default:
                {
                    _mtk_pnl_gpio_set_input(tcon_info->u16GPIO_NUM_MODE_CHG);
                    UBOOT_TRACE("GPIO[%d] Input Hi-Z\n",tcon_info->u16GPIO_NUM_MODE_CHG);
                    break;
                }
            }
        }

        if(tcon_info->u16DELAY_MODE_CHG > 0)
        {
            mdelay(tcon_info->u16DELAY_MODE_CHG);
        }
    }
    else if((mtk_pnl_is_dlg_mode() == FALSE) && (tcon_info->bSupportGpioModeChange) && (tcon_info->u16GPIO_NUM_MODE_CHG))
    {
        if(tcon_info->u16GPIO_NUM_MODE_CHG > 0)
        {
            switch(tcon_info->ucGPIO_CTRL_MODE_CHG_OFF)
            {
                case 0:
                {
                    _mtk_pnl_gpio_set_low(tcon_info->u16GPIO_NUM_MODE_CHG);
                    UBOOT_TRACE("GPIO[%d] Ouput Low\n",tcon_info->u16GPIO_NUM_MODE_CHG);
                    break;
                }
                case 1:
                {
                    _mtk_pnl_gpio_set_high(tcon_info->u16GPIO_NUM_MODE_CHG);
                    UBOOT_TRACE("GPIO[%d] Ouput High\n",tcon_info->u16GPIO_NUM_MODE_CHG);
                    break;
                }
                case 0xFF:
                default:
                {
                    _mtk_pnl_gpio_set_input(tcon_info->u16GPIO_NUM_MODE_CHG);
                    UBOOT_TRACE("GPIO[%d] Input Hi-Z\n",tcon_info->u16GPIO_NUM_MODE_CHG);
                    break;
                }
            }
        }

        if(tcon_info->u16DELAY_MODE_CHG > 0)
        {
            mdelay(tcon_info->u16DELAY_MODE_CHG);
        }
    }

    // I2C WP gpio setting before i2c write (wp off, i2c on)
    if(((mtk_pnl_is_dlg_mode())||(_mtk_pnl_is_game_direct_mode()))&&
        (tcon_info->bSupportTconI2cWP)&&(tcon_info->u16GPIO_NUM_TCON_I2C_WP))
    {
        if(tcon_info->u16GPIO_NUM_TCON_I2C_WP > 0)
        {
            switch(tcon_info->ucGPIO_TCON_I2C_WP_OFF)
            {
                case 0:
                {
                    _mtk_pnl_gpio_set_low(tcon_info->u16GPIO_NUM_TCON_I2C_WP);
                    UBOOT_TRACE("GPIO[%d] Ouput Low\n",tcon_info->u16GPIO_NUM_TCON_I2C_WP);
                    break;
                }
                case 1:
                {
                    _mtk_pnl_gpio_set_high(tcon_info->u16GPIO_NUM_TCON_I2C_WP);
                    UBOOT_TRACE("GPIO[%d] Ouput High\n",tcon_info->u16GPIO_NUM_TCON_I2C_WP);
                    break;
                }
                case 0xFF:
                default:
                {
                    _mtk_pnl_gpio_set_input(tcon_info->u16GPIO_NUM_TCON_I2C_WP);
                    UBOOT_TRACE("GPIO[%d] Input Hi-Z\n",tcon_info->u16GPIO_NUM_TCON_I2C_WP);
                    break;
                }
            }
        }

        //TODO: delay
    }

    // I2C setting
    if (tcon_info->bVRR_HighFrameRateMode_Support == TRUE) // Turn on DLG mode
    {
        bus_id = tcon_info->ucVRR_HFR_ON_I2C_bus;
        slave_id = tcon_info->ucVRR_HFR_ON_I2C_dev_addr;
        addr_count = 1;
        reg_offset = tcon_info->ucVRR_HFR_ON_I2C_offst;
        if (mtk_pnl_is_dlg_mode())
        {
            data_size = tcon_info->ucVRR_HFR_ON_I2C_DATA_SIZE;
            write_data = tcon_info->ucVRR_HFR_ON_I2C_DATA;
        }
        else
        {
            data_size = tcon_info->ucVRR_HFR_OFF_I2C_DATA_SIZE;
            write_data = tcon_info->ucVRR_HFR_OFF_I2C_DATA;
        }
        bus_id_sec = tcon_info->ucVRR_HFR_ON_I2C_SEC_bus;
        slave_id_sec = tcon_info->ucVRR_HFR_ON_I2C_SEC_dev_addr;
        addr_count_sec = 1;
        reg_offset_sec = tcon_info->ucVRR_HFR_ON_I2C_SEC_offst;
        if (mtk_pnl_is_dlg_mode())
        {
            data_size_sec = tcon_info->ucVRR_HFR_ON_I2C_SEC_DATA_SIZE;
            write_data_sec = tcon_info->ucVRR_HFR_ON_I2C_SEC_DATA;
        }
        else
        {
            data_size_sec = tcon_info->ucVRR_HFR_OFF_I2C_SEC_DATA_SIZE;
            write_data_sec = tcon_info->ucVRR_HFR_OFF_I2C_SEC_DATA;
        }

        UBOOT_TRACE("I2C write MultiByte bus_id=%d, slave_id=0x%02x, reg_offset=%d, data_size=%ld\n ", bus_id,slave_id,reg_offset,data_size);
        _mtk_pnl_cust_iic_init(bus_id);
        ret = _mtk_pnl_cust_iic_write(slave_id, addr_count, &reg_offset, data_size, write_data);

        if(ret < 0)
            UBOOT_DEBUG("write iic error!! bus_id=%d slave_id=0x%02x \n",bus_id, slave_id);

        if (tcon_info->bVRR_HFR_ON_I2C_SEC_Support)
        {
            UBOOT_TRACE("I2C SEC bus_id=%d, slave_id=0x%02x, reg_offset=%d, data_size=%ld\n ",
                bus_id_sec, slave_id_sec, reg_offset_sec, data_size_sec);
            _mtk_pnl_cust_iic_init(bus_id_sec);
            ret = _mtk_pnl_cust_iic_write(slave_id_sec, addr_count_sec,
                &reg_offset_sec, data_size_sec, write_data_sec);

            if (ret < 0)
            {
                UBOOT_DEBUG("write iic sec error!! bus_id=%d slave_id=0x%02x \n",
                    bus_id_sec, slave_id_sec);
            }
        }
    }
    else if((_mtk_pnl_is_game_direct_mode() == TRUE)&&(tcon_info->bGAME_Direct60HzMode_Support == TRUE)) // Turn on GAME mode
    {
        //TODO
    }

    // I2C WP gpio setting after i2c write (wp on, i2c off)
    if(((mtk_pnl_is_dlg_mode())||(_mtk_pnl_is_game_direct_mode()))&&
        (tcon_info->bSupportTconI2cWP)&&(tcon_info->u16GPIO_NUM_TCON_I2C_WP))
    {
        if(tcon_info->u16GPIO_NUM_TCON_I2C_WP > 0)
        {
            switch(tcon_info->ucGPIO_TCON_I2C_WP_ON)
            {
                case 0:
                {
                    _mtk_pnl_gpio_set_low(tcon_info->u16GPIO_NUM_TCON_I2C_WP);
                    UBOOT_TRACE("GPIO[%d] Ouput Low\n",tcon_info->u16GPIO_NUM_TCON_I2C_WP);
                    break;
                }
                case 1:
                {
                    _mtk_pnl_gpio_set_high(tcon_info->u16GPIO_NUM_TCON_I2C_WP);
                    UBOOT_TRACE("GPIO[%d] Ouput High\n",tcon_info->u16GPIO_NUM_TCON_I2C_WP);
                    break;
                }
                case 0xFF:
                default:
                {
                    _mtk_pnl_gpio_set_input(tcon_info->u16GPIO_NUM_TCON_I2C_WP);
                    UBOOT_TRACE("GPIO[%d] Input Hi-Z\n",tcon_info->u16GPIO_NUM_TCON_I2C_WP);
                    break;
                }
            }
        }
    }

    return TRUE;
}


