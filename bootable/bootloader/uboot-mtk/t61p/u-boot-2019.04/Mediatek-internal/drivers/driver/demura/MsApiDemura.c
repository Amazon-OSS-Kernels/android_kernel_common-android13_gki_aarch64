// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#include <common.h>
#include <MsTypes.h>
#include <fs.h>
//#include <apiPNL.h>
#include <panel_impl.h>
#include "ms_utils.h"
////#include <ShareType.h>
//#include <MsOS.h>
//#include <MsBoot.h>
//#include <MsSystem.h>
////#include <MsStr.h>
#include <debug_impl.h>
//#include <MsSystem.h>
//#include <MsMmap.h>
#include "dts_parser.h"
//#include <drvMIU.h>
#include "demura_config.h"
#include "apiDemura.h"
#include "mstar/URSA/CommonDataType.h"
#include "utility.h"
#include <linux/delay.h>
#include "demura_version.h"
#ifdef CONFIG_MULTICORES_PLATFORM
#include <smp/thread_info.h>
extern smp_spin_lock_t fs_spin_lock;
#endif

#ifdef MSOS_TYPE_LINUX_KERNEL
#define mst_atoi(str) simple_strtoul(((str != NULL) ? str : ""), NULL, 0);
#else
#define mst_atoi(str) strtoul(((str != NULL) ? str : ""), NULL, 0);
#endif

#undef ALIGN
#define ALIGN(x,a)        __ALIGN_MASK((x),(typeof(x))(a)-1)
#define __ALIGN_MASK(x,mask)    (((x)+(mask))&~(mask))

MS_BOOL MApi_SDCtiny_Init(MS_PHYADDR phy_addr,MS_U32 lut_virtaddr);
MS_BOOL MApi_SDCtiny_Enable(void);
MS_BOOL MApi_FCIC_Init(void);
MS_BOOL MApi_FCIC_Enable(MS_BOOL bEnable);
MS_BOOL MApi_SIW_Enable(MS_PHYADDR phy_addr,MS_BOOL bEnable);
MS_BOOL MApi_SIW_Init(MS_PHYADDR pHdr);

#if defined(CONFIG_DEMURA_VENDOR_MULTI)
MS_BOOL MApi_MsDemura_Init(Demura_Panel_Data panel_data, EN_DEMURA_MULTI_VENDOR multi_vendor);
#else
MS_BOOL MApi_MsDemura_Init(Demura_Panel_Data panel_data);
#endif
// Global value;
static demura_buf_desc *pbuf_desc = NULL;

static void init_dbuf_desc(MS_U64 mmap_start, MS_U32 mmap_len)
{
    MS_U64 u32_mmap_addr  = mmap_start;
    UBOOT_TRACE("IN\n");
    pbuf_desc  = (demura_buf_desc *)u32_mmap_addr;

    if (check_str_resume() == 0)
    {
        // AC ON
        memset(pbuf_desc, 0, DEMURA_BUFFER_DESC_SIZE);
        pbuf_desc->mmap_virt_addr =  mmap_start;
        pbuf_desc->mmap_phy_addr  =  BA2PA(mmap_start);
        pbuf_desc->mmap_length    =  mmap_len;
        pbuf_desc->next_bin_addr  =  pbuf_desc->mmap_phy_addr + DEMURA_BUFFER_DESC_SIZE;
        pbuf_desc->left_space     =  mmap_len   - DEMURA_BUFFER_DESC_SIZE;
    }
    UBOOT_TRACE("mmap_virt_addr = 0x%lx  \n", pbuf_desc->mmap_virt_addr);
    UBOOT_TRACE("mmap_virt_addr = 0x%lx  \n", pbuf_desc->mmap_virt_addr);
    UBOOT_TRACE("mmap_phy_addr = 0x%lx\n", pbuf_desc->mmap_phy_addr);
    UBOOT_TRACE("mmap_length   = 0x%x\n",  pbuf_desc->mmap_length);
    UBOOT_TRACE("next_bin_addr = 0x%x\n",  pbuf_desc->next_bin_addr);
    UBOOT_TRACE("left_space    = 0x%x\n",  pbuf_desc->left_space);

}


static void enable_dbuf_desc(void)
{
    memcpy(pbuf_desc->magic_str, DEMURA_BDM_STRING, strlen(DEMURA_BDM_STRING));
}

static int demura_get_mmap(MS_PHY *addr, MS_U32 *u32_len)
{
    int ret = 0;
    char *str_mem = env_get(DEMURA_ENV_MEM);
    struct dts_mmap demura_mmap = {0};
    UBOOT_TRACE("IN\n");
    ret = parse_dt(DEMURA_MMAP_NODE, mmap_dt_parser, (void*)&demura_mmap, DEMURA_MMAP_KEYWORD);
    if(ret < 0)
    {
        UBOOT_ERROR("Error: MI_DISPOUT_DEMURA info parse error in DTS\n");
        return -1;
    }
    else
    {
        *addr = demura_mmap.address;
        *u32_len  = demura_mmap.size;
        UBOOT_TRACE("addr = 0x%llx, size = 0x%llx\n", demura_mmap.address, demura_mmap.size);
    }
    if (str_mem != NULL)
    {
        *addr = mst_atoi(str_mem);
        UBOOT_TRACE("mem addr = 0x%x, size = 0x%x\n", (MS_U32)*addr, *u32_len);
    }

    if (demura_mmap.address == 0 || demura_mmap.size == 0)
    {
        UBOOT_TRACE("mmap addr or size get fail\n");
        return 0;
    }

    UBOOT_TRACE("OK\n");
    return 1;
}

MS_U32 load_file_to_mmap(char *path, MS_U64 *dram_addr)
{
    unsigned int file_size=0;
    loff_t read_len = 0;
    loff_t size =0;
    char device_name[8/*STORAGE_DEVICE_BUF_SIZE*/] = {0};
    unsigned int device = 0;
    char storage_info[8/*STORAGE_INDEX_BUF_SIZE*/] = {0};
    int ret;
    MS_PHY mmap_addr = 0;
    MS_U32 mmap_len = 0;
#ifdef CONFIG_MULTICORES_PLATFORM
    unsigned long irq_flag = 0;
#endif

    UBOOT_TRACE("IN\n");

    if(dram_addr==NULL)
    {
        UBOOT_ERROR("Error: Invalid dram_addr\n");
        return 0;
    }

    ret = sys_get_boot_device(device_name, sizeof(device_name));
    if(ret < 0)
    {
        UBOOT_ERROR("Error: Get booting device name failure, Unknown storage device.\n");
        return 0;
    }

    ret = sys_get_storage_info(device, DEMURA_PARTITION_NAME,storage_info);
    if(ret < 0)
    {
        UBOOT_ERROR("Error: sys_get_storage_info failure\n");
        return 0;
    }

#if defined(CONFIG_MULTICORES_PLATFORM)
    smp_spin_lock_save(&fs_spin_lock, irq_flag);
#endif
    if (fs_set_blk_dev(device_name, storage_info, FS_TYPE_EXT))
    {
        UBOOT_ERROR("Error: partition select failure\n");
        goto LOAD_FILE_MMAP_FAIL;
    }

    UBOOT_INFO("read file from path:%s\n", path);
    if (fs_size(path, &size) < 0)
    {
        UBOOT_ERROR("Error: Get demura.bin file size failure\n");
        goto LOAD_FILE_MMAP_FAIL;
    }

    if (fs_set_blk_dev(device_name, storage_info, FS_TYPE_EXT))
    {
        UBOOT_ERROR("Error: partition select failure\n");
        goto LOAD_FILE_MMAP_FAIL;
    }

    if(demura_get_mmap(&mmap_addr, &mmap_len) != 1)
    {
        UBOOT_ERROR("Get Demura MMAP Fail");
        goto LOAD_FILE_MMAP_FAIL;
    }

    init_dbuf_desc(mmap_addr, mmap_len);

    //*dram_addr = (unsigned int *)malloc((unsigned long)size);
    *dram_addr = mmap_addr;

    ret = fs_read(path, *dram_addr, 0, 0, &read_len);
    if (ret < 0)
    {
        UBOOT_ERROR("Error: Read demura bin file failure\n");
        //free(*dram_addr);
        goto LOAD_FILE_MMAP_FAIL;
    }

#if defined(CONFIG_MULTICORES_PLATFORM)
    smp_spin_unlock_restore(&fs_spin_lock, irq_flag);
#endif
    printf("[%s:%d]%s file, put on 0x%X, size=0x%X\n",__FUNCTION__, __LINE__,
        path,(unsigned int)*dram_addr,(unsigned int)read_len);
    file_size = (unsigned long)read_len;
    UBOOT_TRACE("OK\n");
    return file_size;

LOAD_FILE_MMAP_FAIL:
#if defined(CONFIG_MULTICORES_PLATFORM)
    smp_spin_unlock_restore(&fs_spin_lock, irq_flag);
#endif
    return 0;
}

MS_BOOL push_demura_bin(demura_bin_desc *pbin)
{
    MS_U32 cur_num     =  pbuf_desc->bin_cur_num;
    MS_U32 start_addr  =  pbuf_desc->next_bin_addr;
    MS_U32 len_align   =  ALIGN(pbin->length, DEMURA_BIN_ALIGN_UNIT);

    if (cur_num >= DEMURA_BIN_MAX_NUM)
    {
        UBOOT_ERROR("cur_num(%d) >= DEMURA_BIN_MAX_NUM(%d)", (int)cur_num, DEMURA_BIN_MAX_NUM);
        return FALSE;
    }

    if (pbin->length == 0)
    {
        UBOOT_ERROR("pbin->length = 0\n");
        return FALSE;
    }
    if (len_align > pbuf_desc->left_space)
    {
        UBOOT_ERROR("len_align(0x%x) > pbuf_desc->left_space(0x%x)", \
                    (uint)len_align, (uint)pbuf_desc->left_space);
        return FALSE;
    }

    pbuf_desc->next_bin_addr +=  len_align;
    pbuf_desc->left_space    -=  len_align;
    pbin->phy_addr   =  start_addr;
    pbin->virt_addr  =  PA2BA(start_addr);
    pbin->len_align  =  len_align;
    UBOOT_TRACE("pbin->phy_addr = 0x%x\n", (uint)start_addr);
    UBOOT_TRACE("pbin->virt_addr = 0x%x\n", (uint)PA2BA(start_addr));
    memcpy(&(pbuf_desc->bdesc[cur_num]), pbin, sizeof(*pbin));
    pbuf_desc->bin_cur_num ++;

    return TRUE;
}

void pop_demura_bin()
{
    MS_U32 cur_num, len_align;
    demura_bin_desc *pbin;

    cur_num = pbuf_desc->bin_cur_num;
    if (cur_num <= 0)
    {
        return;
    }
    pbin      = &(pbuf_desc->bdesc[cur_num-1]);
    len_align = pbin->len_align;

    pbuf_desc->next_bin_addr -=  len_align;
    pbuf_desc->left_space    +=  len_align;

    memset(pbin, 0, sizeof(*pbin));
    pbuf_desc->bin_cur_num --;
}


MS_U64 add_dbin_buf(const char *ip_string, MS_U32 filesize, MS_U32 id_num)
{
    MS_U64  bus_addr;
    MS_U32  final_size;
    demura_bin_desc bin_desc;

    final_size = ALIGN(filesize, DEMURA_BIN_ALIGN_UNIT);
    if (strlen(ip_string) >= sizeof(bin_desc.ip_string))
    {
        UBOOT_ERROR("length of ip_string(%s) > %d", ip_string, (int)sizeof(bin_desc.ip_string));
        return 0x00;
    }

    memset(&bin_desc, 0, sizeof(bin_desc));
    memcpy(bin_desc.ip_string, ip_string, strlen(ip_string));
    bin_desc.length  = final_size;
    bin_desc.id_num  = id_num;

    if (push_demura_bin(&bin_desc) != TRUE)
    {
        UBOOT_ERROR("push_demura_bin Error!\n");
        return 0x00;
    }

    bus_addr = bin_desc.virt_addr;
    UBOOT_TRACE("virt_addr = 0x%x\n", (uint)bin_desc.virt_addr);
    UBOOT_TRACE("phy_addr = 0x%x\n", (uint)bin_desc.phy_addr);
    return bus_addr;
}



demura_bin_desc *get_demura_bin(char *ip_string, MS_U32 id_num)
{
    MS_U32 i = 0;
    MS_U32 num = pbuf_desc->bin_cur_num;
    demura_bin_desc *pbin = NULL;

    for (i = 0; i < num; i++)
    {
        pbin = &(pbuf_desc->bdesc[i]);
        if (strcmp(pbin->ip_string, ip_string) == 0)
        {
            if (pbin->id_num == id_num)
            {
                break;
            }
        }
    }
    if (i == num)
    {
        pbin = NULL;
    }
    return pbin;
}

MS_U32 get_dbuf_length(void)
{
    return pbuf_desc->mmap_length;
}

MS_BOOL MApi_Demura_Bypass(MS_BOOL bOnOff)
{
    MApi_MsDemura_BYPASS(bOnOff);
    return TRUE;
}

#if defined(CONFIG_DEMURA_VENDOR_MULTI)
MS_BOOL demura_check_vendor(EN_DEMURA_MULTI_VENDOR *multi_vendor)
{
#ifdef CONFIG_DEMURA_MSTAR
    char *vendid;

    if (get_demura_file() == E_MS_UTIL_BIN_FILE_BACKLIGHT)
    {
        vendid = env_get(DEMURA_ENV_VENDORID_BACKLIGHT);
    }
    else
    {
        vendid = env_get(DEMURA_ENV_VENDORID);
    }

    if (vendid)
    {
        *multi_vendor = mst_atoi(vendid);
        UBOOT_INFO("multi_vendor %s: %d\n", vendid, *multi_vendor);
        return TRUE;
    }
    if((*multi_vendor == EN_DEMURA_MULTI_NOT) || (*multi_vendor >= EN_DEMURA_MULTI_MAX))
    {
        return FALSE;
    }
    else if((*multi_vendor == EN_DEMURA_MULTI_LGD_START) || (*multi_vendor == EN_DEMURA_MULTI_LGD_END))
    {
        *multi_vendor = EN_DEMURA_MULTI_LGD_V18;
        UBOOT_INFO("multi_vendor LGD : use default V18\n");
    }
#endif
    return TRUE;
}

MS_BOOL demura_check_fcic(void)
{
#ifdef CONFIG_DEMURA_FCIC
    UBOOT_DEBUG("Init FCIC-Demura IP\n");
    if (FALSE == MApi_FCIC_Init())
    {
        UBOOT_ERROR("MApi_FCIC_Init Failed\n");
        return FALSE;
    }
#endif
    return TRUE;
}

MS_BOOL MApi_Demura_Init(Demura_Panel_Data panel_data, EN_DEMURA_MULTI_VENDOR multi_vendor)
{
#ifdef CONFIG_DEMURA_SIW
    MS_BOOL siw_ret = FALSE;
#endif

    char *str_ip = NULL;
    MS_BOOL sdctiny_ret = FALSE;
    MS_BOOL ms_ret = FALSE;
    MS_BOOL fc_ret = FALSE;
    //MS_U32 buf_phy_len  = 0;
    MS_PHYADDR mmap_addr = 0;
    //MS_U64 u64_phy_addr = 0;
    MS_U32 mmap_len = 0;
    unsigned long init_start_time = 0;
    //MS_U32 BIN_SIZE = 0;
    //MS_PHYADDR* BIN_ADDR = 0;

    //struct device_node *target_memory_np = NULL;
    //__be32 *p = NULL;
    //MS_U64 *addr = NULL;


#ifdef CONFIG_MTK_DEMURA_UFC_BIN
    UBOOT_TRACE("MULTI UFC IN VERSION:%u.%u\n", DEMURA_MAJOR_NUMBER, DEMURA_MINOR_NUMBER);
#else
    UBOOT_TRACE("MULTI DMC IN VERSION:%u.%u\n", DEMURA_MAJOR_NUMBER, DEMURA_MINOR_NUMBER);
#endif
    str_ip = env_get(ENV_DEMURA_IN_MBOOT);

    if(str_ip == NULL)
    {
        UBOOT_DEBUG("ENV_DEMURA_IN_MBOOT NULL\n");
    }
    else
    {
        UBOOT_DEBUG("ENV_DEMURA_IN_MBOOT = %s\n", str_ip);
    }
    // Protect multi vendor list
    set_demura_file(E_MS_UTIL_BIN_FILE_NORMAL);
    if(!demura_check_vendor(&multi_vendor))
    {
        UBOOT_ERROR("multi_vendor input is not valid, init fail\n");
        UBOOT_TRACE("OK\n");
        return FALSE;
    }


    //init_start_time = MsSystemGetBootTime();
    init_start_time = timer_get_us();

    str_ip = env_get(ENV_DEMURA_IP);
    if(str_ip == NULL)
    {
        UBOOT_DEBUG("ENV_DEMURA_IP NULL\n");
    }
    else
    {
        UBOOT_DEBUG("ENV_DEMURA_IP = %s\n", str_ip);
    }


    if(demura_get_mmap(&mmap_addr, &mmap_len) != 1)
    {
        UBOOT_ERROR("Get Demura MMAP Fail");
        return FALSE;
    }


    //init_dbuf_desc(PA2BA(phy_addr), buf_phy_len);
    if (get_demura_bin_type() == E_MS_UTIL_BIN_TYPE_DLG)
        init_dbuf_desc(mmap_addr + (mmap_len>>1), (mmap_len>>1));
    else
        init_dbuf_desc(mmap_addr, (mmap_len>>1));

    #ifdef CONFIG_DEMURA_SDCtiny
    UBOOT_DEBUG("Init SDC_tiny-Demura IP\n");
    sdctiny_ret = MApi_SDCtiny_Init(phy_addr,PA2BA(phy_addr));
    if (sdctiny_ret == FALSE)
    {
        UBOOT_ERROR("MApi_SDC_tiny_Init Failed\n");
    }
    #endif

    // Using api(push_demura_bin) to get load buffer
    #ifdef CONFIG_DEMURA_MSTAR
    UBOOT_TRACE("Init Mstar-Demura IP\n");
    ms_ret = MApi_MsDemura_Init(panel_data, multi_vendor);
    if (ms_ret == FALSE)
    {
        UBOOT_ERROR("MApi_MstarDemura_Init Failed\n");
    }
    #endif
    #ifdef CONFIG_DEMURA_SIW
        UBOOT_DEBUG("Init SIW-Demura IP\n");
        siw_ret = MApi_SIW_Init(mmap_addr);
        if (siw_ret == FALSE)
        {
            UBOOT_ERROR("MApi_SIW_Init Failed\n");
        }
    #endif


    //Check FCIC is needed
    fc_ret = demura_check_fcic();


    /**************************************/
    // After All IP have be inited !
    if ((ms_ret == TRUE) || (fc_ret == TRUE))
    {
        enable_dbuf_desc();
    }

    #ifdef CONFIG_DEMURA_SDCtiny
    if ((sdctiny_ret == TRUE))
    {
        UBOOT_DEBUG("Enable SDCtiny-Demura IP\n");
        MApi_SDCtiny_Enable();
    }
    #endif

    if (((str_ip == NULL) || (strncmp(str_ip, "mstar", strlen("mstar")) == 0)) && (ms_ret == TRUE))
    {
        if (panel_data.bOn == TRUE)
        {
    #ifdef CONFIG_DEMURA_MSTAR
            UBOOT_TRACE("Enable Mstar-Demura IP\n");
            MApi_MsDemura_Enable(TRUE);
    #endif
        }
    }
    else if (fc_ret == TRUE)
    {
    #ifdef CONFIG_DEMURA_FCIC
        MApi_FCIC_Enable(TRUE);
    #endif
    }

    #ifdef CONFIG_DEMURA_SIW
    UBOOT_DEBUG("Enable SIW Demura IP\n");
    MApi_SIW_Enable(mmap_addr, siw_ret);
    #endif
    if ((ms_ret == TRUE) || (fc_ret == TRUE) || (sdctiny_ret == TRUE))
    {
        UBOOT_INFO("Demura_Init Cost : [%lu] us\n", timer_get_us() - init_start_time);
    }
    UBOOT_TRACE("OK\n");
    return TRUE;
}
#else
MS_BOOL demura_check_fcic()
{
#ifdef CONFIG_DEMURA_FCIC
    UBOOT_DEBUG("Init FCIC-Demura IP\n");
    if (FALSE == MApi_FCIC_Init())
    {
        UBOOT_ERROR("MApi_FCIC_Init Failed\n");
        return FALSE;
    }
#endif
    return TRUE;
}

void MApi_MsDemura_Enable_CCN()
{
#ifdef CONFIG_DEMURA_MSTAR
    UBOOT_DEBUG("Enable Mstar-Demura IP\n");
    MApi_MsDemura_Enable(TRUE);
#endif
}

void MApi_FCIC_Enable_CCN()
{
#ifdef CONFIG_DEMURA_FCIC
    UBOOT_DEBUG("Enable FCIC-Demura IP\n");
    MApi_FCIC_Enable(TRUE);
#endif
}

void MApi_SIW_Enable_CCN(MS_PHYADDR phy_addr)
{
#ifdef CONFIG_DEMURA_SIW
    UBOOT_DEBUG("Enable SIW-Demura IP\n");
    MApi_SIW_Enable(phy_addr,TRUE);
#endif
}

MS_BOOL MApi_Demura_Init(Demura_Panel_Data panel_data)
{
    char *str_ip = NULL;
    MS_BOOL sdctiny_ret = FALSE;
    MS_BOOL ms_ret = FALSE;
    MS_BOOL fc_ret = FALSE;
    MS_BOOL siw_ret = FALSE;
    MS_U32 buf_phy_len  = 0;
    MS_PHYADDR phy_addr = 0;
    long init_start_time = 0;

    UBOOT_TRACE("MULTI IN\n");
    init_start_time = MsSystemGetBootTime();
    str_ip = env_get(ENV_DEMURA_IP);
    UBOOT_DEBUG("ENV_DEMURA_IP = %s\n", str_ip);

    /*if (get_addr_from_mmap(DEMURA_MMAP_ID, &phy_addr) == -1)
    {
          if (get_addr_from_mmap(DEMURA_MMAP_ID_MI, &phy_addr) == -1)
          {
            UBOOT_ERROR("get addr from %s mmap fail\n",DEMURA_MMAP_ID);
            UBOOT_TRACE("OK\n");
            return FALSE;
        }
    }
    if (get_length_from_mmap(DEMURA_MMAP_ID, (U32 *)&buf_phy_len) == -1)
    {
          if (get_length_from_mmap(DEMURA_MMAP_ID_MI, (U32 *)&buf_phy_len) == -1)
          {
            UBOOT_ERROR("get length from %s mmap fail\n",DEMURA_MMAP_ID);
            UBOOT_TRACE("OK\n");
            return FALSE;
        }
    }*/

    buf_phy_len = 4765 * 1024;


    init_dbuf_desc(PA2BA(phy_addr), buf_phy_len);

    #ifdef CONFIG_DEMURA_SDCtiny
    UBOOT_DEBUG("Init SDC_tiny-Demura IP\n");
    sdctiny_ret = MApi_SDCtiny_Init(phy_addr,PA2BA(phy_addr));
    if (sdctiny_ret == FALSE)
    {
        UBOOT_ERROR("MApi_SDC_tiny_Init Failed\n");
    }
    #endif

    // Using api(push_demura_bin) to get load buffer
    #ifdef CONFIG_DEMURA_MSTAR
    UBOOT_DEBUG("Init Mstar-Demura IP\n");
    ms_ret = MApi_MsDemura_Init(panel_data);
    if (ms_ret == FALSE)
    {
        UBOOT_ERROR("MApi_MstarDemura_Init Failed\n");
    }
    #endif

    fc_ret = demura_check_fcic();

#ifdef CONFIG_DEMURA_SIW
    UBOOT_DEBUG("Init SIW-Demura IP\n");
    siw_ret = MApi_SIW_Init(PA2BA(phy_addr));
    if (siw_ret == FALSE)
    {
        UBOOT_ERROR("MApi_SIW_Init Failed\n");
    }
#endif


    /**************************************/
    // After All IP have be inited !
    if ((ms_ret == TRUE) || (fc_ret == TRUE))
    {
        enable_dbuf_desc();
    }

    #ifdef CONFIG_DEMURA_SDCtiny
    if ((sdctiny_ret == TRUE))
    {
        UBOOT_DEBUG("Enable SDCtiny-Demura IP\n");
        MApi_SDCtiny_Enable();
    }
    #endif

    if (((str_ip == NULL) || (strcmp(str_ip, "mstar") == 0)) && (ms_ret == TRUE))
    {
        MApi_MsDemura_Enable_CCN();
    }
    else if ((strcmp(str_ip, "fcic") == 0)&& (fc_ret == TRUE))
    {
        MApi_FCIC_Enable_CCN(TRUE);
    }

    MApi_SIW_Enable_CCN(phy_addr);

    if ((ms_ret == TRUE) || (fc_ret == TRUE) || (siw_ret == TRUE) || (sdctiny_ret == TRUE))
    {
        UBOOT_INFO("Demura_Init Cost : [%lu] ms\n", MsSystemGetBootTime() - init_start_time);
    }
    UBOOT_TRACE("OK\n");
    return TRUE;
}
#endif
