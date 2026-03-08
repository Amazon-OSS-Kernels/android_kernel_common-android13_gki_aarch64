// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#include <common.h>
#include <command.h>
#include <exports.h>
#include <system_impl.h>
#include <MsTypes.h>
#include <debug_impl.h>
#include <usb_upgrade.h>
#include <upgrade_utility.h>
#include <secure/secure_upgrade.h>
#include <secure/secure_common.h>
#include <secure/crypto_auth.h>
#include <secure/crypto_aes.h>
#include <secure/crypto_sha.h>
#include <secure/crypto_rsa.h>
#include <secure_boot.h>
#include <iniutility.h>

//#include <MsVfs.h>                   // [TODO] - wait porting


#if defined(CONFIG_SECURE_UPGRADE_V2)
//-------------------------------------------------------------------------------------------------
//  Global Variables
//-------------------------------------------------------------------------------------------------
extern st_image_offset_info image_offset_info;
extern int uboot_boot_device;
extern struct upgrade_info gupgradeinfo;

//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------
#define SECURE_UPGRADE_TMP_BUF_ADDR  (CONFIG_SYS_MIU0_CACHE|0x10200000)
#define UPGRADE_SCRIPT_OFFSET 0x1000
#define HW_SHA_ALIGN_SIZE 64
#define DOWNLOAD_LEN_ALIGN_SIZE 0x200
//-------------------------------------------------------------------------------------------------
//  Local Functions
//-------------------------------------------------------------------------------------------------
static int get_public_key_n(EN_SECURE_KEY_TYPE en_key, unsigned char *buffer, unsigned int len);
static int get_public_key_e(EN_SECURE_KEY_TYPE en_key, unsigned char *buffer, unsigned int len);
static int get_aes_key(EN_SECURE_KEY_TYPE en_key, unsigned char *buffer, unsigned int len);

//-------------------------------------------------------------------------------------------------
//  Local Variables
//-------------------------------------------------------------------------------------------------
static unsigned char secure_init_ok = FALSE;

void secure_halt_cpu(void)
{
    #ifdef U_OTP_SBOOT_CONTROL
    if (MDrv_Get_U_OTP_SBOOT() == FALSE)            // [TODO] - wait porting MsSystem.h to uboot
    {
        UBOOT_ERROR("U_OTP_SBOOT is OFF, Skip it\n");
        return;
    }
    #endif

    //show error UI
    run_command("destroy_logo",0);
    run_command("bootlogo 0 0 1 1 boot_error.jpg", 0);

    run_command("panel_post_init",0);

#if (CONFIG_LOCAL_DIMMING)
    run_command("local_dimming", 0);
#endif
    //halt cpu
    while(1){;}
}

static int set_update_mode_by_interface(const char *interface, const char *type)
{
    UBOOT_TRACE("IN\n");
    int ret=0;
    if(strncmp(interface, "usb", strlen("usb")) == 0)
    {
        if(strncmp(type, "NON_DECRYPTED", strlen("NON_DECRYPTED")) == 0)
        {
            set_upgrade_mode(EN_UPDATE_MODE_USB);
        }
        else if(strncmp(type, "DECRYPTED", strlen("DECRYPTED")) == 0)
        {
            set_upgrade_mode(EN_UPDATE_MODE_USB_WITH_SEG_DECRYPTED);
        }
        else
        {
            UBOOT_ERROR("No type: %s\n", type);
            ret=-1;
        }
    }
    else if((strncmp(interface, "mmc", strlen("mmc")) == 0) || (strncmp(interface, "ufs", strlen("ufs")) == 0))
    {
        if(strncmp(type, "NON_DECRYPTED", strlen("NON_DECRYPTED")) == 0)
        {
            set_upgrade_mode(EN_UPDATE_MODE_OAD);
        }
        else if(strncmp(type, "DECRYPTED", strlen("DECRYPTED")) == 0)
        {
            set_upgrade_mode(EN_UPDATE_MODE_OAD_WITH_SEG_DECRYPTED);
        }
        else
        {
            UBOOT_ERROR("No type: %s\n", type);
            ret=-1;
        }
    }
    else
    {
        UBOOT_ERROR("No update mode: %s\n", interface);
    }
    UBOOT_TRACE("OK\n");
    return ret;
}


int firmware_image_header_authentication(unsigned char *header_buf)
{
    UBOOT_TRACE("IN\n");
    int ret=0;
    static unsigned char header[PACK_HEADER_SIZE] __attribute__((aligned(HW_SHA_ALIGN_SIZE)))= {0x00};
    unsigned char header_signature[SIGNATURE_SIZE]={0};
    unsigned char rsa_public_key_n[RSA_PUBLIC_KEY_N_LEN]="\0";
    unsigned char rsa_public_key_e[RSA_PUBLIC_KEY_E_LEN]="\0";

    memcpy(header_signature, header_buf, SIGNATURE_SIZE);
    memcpy(header, (header_buf+SIGNATURE_SIZE), HEADER_SIZE);

    //Get rsa upgrade public key
    ret = get_public_key_n(KEY_TYPE_CKB_A_UPG_KEY,rsa_public_key_n,RSA_PUBLIC_KEY_N_LEN);
    if(ret!=0)
    {
        UBOOT_ERROR("Get public key N fail\n");
        return -1;
    }
    ret = get_public_key_e(KEY_TYPE_CKB_A_UPG_KEY,rsa_public_key_e,RSA_PUBLIC_KEY_E_LEN);
    if(ret!=0)
    {
        UBOOT_ERROR("Get public key E fail\n");
        return -1;
    }

    // Verify PKG Header
    UBOOT_INFO("Image Header Authentication ....\n");
    ret = secure_do_authentication(rsa_public_key_n, rsa_public_key_e, header_signature, header, HEADER_SIZE);
    if(ret!=0)
    {
        UBOOT_ERROR("\033[0;31m RSA upgrade public-N key=\033[0m\n");
        UBOOT_ERRDUMP(rsa_public_key_n, RSA_PUBLIC_KEY_N_LEN);

        UBOOT_ERROR("\033[0;31m RSA upgrade public-E key=\033[0m\n");
        UBOOT_ERRDUMP(rsa_public_key_e, RSA_PUBLIC_KEY_E_LEN);

        UBOOT_ERROR("\033[0;31m signature=\033[0m\n");
        UBOOT_ERRDUMP(header_signature, SIGNATURE_LEN);

        UBOOT_ERROR("\033[0;31m clear image=\033[0m\n");
        UBOOT_ERRDUMP(header, HEADER_SIZE);
        secure_halt_cpu();
    }
    else
    {
        #if defined(CONFIG_SECURE_DEBUG_MESSAGE)
        UBOOT_INFO("\033[0;34m******************************************\033[0m\n");
        UBOOT_INFO("\033[0;34m* Image Header Authentication SUCCESS !! *\033[0m\n");
        UBOOT_INFO("\033[0;34m******************************************\033[0m\n");
        #endif
    }

    UBOOT_INFO("Image Header Authentication Finished....\n");

    UBOOT_TRACE("OK\n");
    return ret;
}


int firmware_image_authentication(const char *interface,char *upgrade_file, int device, int partition)

{
    char buffer[CMD_BUF]={0};
    int ret =0;
    UBOOT_TRACE("IN\n");

    memset(buffer, 0 , CMD_BUF);
    if (strncmp(interface, "usb", strlen("usb")) == 0)
    {
        ret = snprintf(buffer, CMD_BUF, "fileSegRSA %s %d:%d %s",interface, device, partition, upgrade_file);
    }else if((strncmp(interface, "mmc", strlen("mmc")) == 0)||(strncmp(interface, "ufs", strlen("ufs")) == 0))
    {
        ret = snprintf(buffer, CMD_BUF, "fileSegRSA %s %s %s",interface,gupgradeinfo.upgrade_storage_info, upgrade_file);
    }
    if(ret > CMD_BUF)
    {
        UBOOT_ERROR("Error: cmd buffer is larger than CMD_BUF(%d)\n",CMD_BUF);
        return -1;
    }

    UBOOT_DEBUG("cmd=%s\n",buffer);
    if(0 != run_command(buffer, 0))
    {
        UBOOT_ERROR("Error: cmd: %s\n",buffer);
        reset_system();
    }
    UBOOT_TRACE("OK\n");
    return 0;
}

static int get_public_key_n(EN_SECURE_KEY_TYPE en_key, unsigned char *buffer, unsigned int len)
{
    UBOOT_TRACE("IN\n");
    st_secure_key secure_key = {0};

    if(buffer==NULL)
    {
        UBOOT_ERROR("The input parameter 'buffer' is a null pointer\n");
        return -1;
    }

    if(len<RSA_PUBLIC_KEY_N_LEN)
    {
        UBOOT_ERROR("The len is smaller than the lenght of public-N(%d bytes).\n",RSA_PUBLIC_KEY_N_LEN);
        return -1;
    }

    if(secure_get_key(en_key, &secure_key)!=-1)
    {
        //UBOOT_DEBUG("secure_key->len 0x%x\n",secure_key.len);
        //UBOOT_DEBUG("secure_key->data 0x%x\n",secure_key.data);
        memcpy(buffer,secure_key.data,len);
    }
    else
    {
        UBOOT_ERROR("Wrong EN_SECURE_KEY_TYPE Index !!\n");
        return -1;
    }

    UBOOT_TRACE("OK\n");
    return 0;
}


static int get_public_key_e(EN_SECURE_KEY_TYPE en_key, unsigned char *buffer, unsigned int len)
{
    UBOOT_TRACE("IN\n");
    st_secure_key secure_key = {0};

    if(buffer==NULL)
    {
        UBOOT_ERROR("The input parameter 'buffer' is a null pointer\n");
        return -1;
    }

    if(len<RSA_PUBLIC_KEY_E_LEN)
    {
        UBOOT_ERROR("The len is smaller than the lenght of public-E(%d bytes).\n",RSA_PUBLIC_KEY_E_LEN);
        return -1;
    }

    if(secure_get_key(en_key, &secure_key)!=-1)
    {
        //UBOOT_DEBUG("secure_key->len 0x%x\n",secure_key.len);
        //UBOOT_DEBUG("secure_key->data 0x%x\n",secure_key.data);
        memcpy(buffer,secure_key.data+RSA_PUBLIC_KEY_N_LEN,len);
    }
    else
    {
        UBOOT_ERROR("Wrong Key Index !!\n");
        return -1;
    }

    UBOOT_TRACE("OK\n");
    return 0;
}

static int get_aes_key(EN_SECURE_KEY_TYPE en_key, unsigned char *buffer, unsigned int len)
{
    UBOOT_TRACE("IN\n");
    st_secure_key secure_key = {0};

    if(buffer==NULL)
    {
        UBOOT_ERROR("The input parameter 'buffer' is a null pointer\n");
        return -1;
    }

    if(len!=AES_KEY_LEN)
    {
        UBOOT_ERROR("The len is smaller than the lenght of AES Key(%d bytes).\n", AES_KEY_LEN);
        return -1;
    }

    if(secure_get_key(en_key, &secure_key)!=-1)
    {
        memcpy(buffer,secure_key.data,len);
    }
    else
    {
        UBOOT_ERROR("Wrong Key Index !!\n");
        return -1;
    }

    UBOOT_TRACE("OK\n");
    return 0;
}


unsigned char get_secure_init_status(void)
{
    return secure_init_ok;
}

static int get_aes_decode_len(unsigned int download_offset, unsigned int download_len, unsigned int upgrade_file_size, unsigned int hashset_len, unsigned int *decode_len)
{
    unsigned int align_download_len = download_len;

    UBOOT_TRACE("IN\n");

    // align download len
    if ((download_len % DOWNLOAD_LEN_ALIGN_SIZE) != 0)
    {
        align_download_len = ((download_len/DOWNLOAD_LEN_ALIGN_SIZE)+1)*DOWNLOAD_LEN_ALIGN_SIZE;
        UBOOT_DEBUG("align_download_len=0x%x\n", align_download_len);
    }

    // check overflow
    if ((download_offset > (download_offset + align_download_len)) || (upgrade_file_size < (upgrade_file_size - hashset_len)))
    {
        UBOOT_ERROR("download_offset or upgrade_file_size overflow\n");
        return -1;
    }

    // check download_offset
    if ((download_offset + align_download_len) > (upgrade_file_size - hashset_len))
    {
        UBOOT_ERROR("download_offset or align_download_len is invalid\n");
        return -1;
    }

    UBOOT_DEBUG("aes_decode_len=0x%x\n", align_download_len);
    *decode_len = align_download_len;

    UBOOT_TRACE("OK\n");
    return 0;
}

static int check_chunk_info(st_chunk_info *ptr_chunk_info, unsigned int upgrade_file_size)
{
    unsigned int chehk_segment_num = 0;

    UBOOT_TRACE("IN\n");

    // Check whether chunk info is valid or not
    if(memcmp(ptr_chunk_info->chunk_id_head,CHUNK_ID,strlen(CHUNK_ID))!=0){
        UBOOT_ERROR("[ERROR] CHUNK_ID compare fail\n");
        return -1;
    }
    if(memcmp(ptr_chunk_info->chunk_id_end,CHUNK_END,strlen(CHUNK_END))!=0){
        UBOOT_ERROR("[ERROR] CHUNK_END compare fail\n");
        return -1;
    }

    if ( (ptr_chunk_info->segment_size > 0) && (ptr_chunk_info->file_data_len > 0) && (ptr_chunk_info->file_hash_len > 0) )
    {
        if((ptr_chunk_info->file_data_len%ptr_chunk_info->segment_size)==0)
            chehk_segment_num  = ptr_chunk_info->file_data_len/ptr_chunk_info->segment_size;
        else
            chehk_segment_num  = ptr_chunk_info->file_data_len/ptr_chunk_info->segment_size+1;

        // Calculate the fileHashLen, fileHashLen = chehk_segment_num *
        // sha256(256/8) or SHA384_DIGEST_SIZE ( 384 / 8)
        if((chehk_segment_num*SHA_DIGEST_SIZE) != ptr_chunk_info->file_hash_len)
        {
            UBOOT_ERROR("The parameter in chunk info is not valid \n");
            return -1;
        }
    }
    else
    {
        UBOOT_ERROR("The parameter in chunk info is not valid \n");
        return -1;
    }

    // check file_data_offset and file_len overflow
    if (ptr_chunk_info->file_data_offset > (ptr_chunk_info->file_data_offset + ptr_chunk_info->file_data_len))
    {
        UBOOT_ERROR("The parameter in chunk info overflow \n");
        return -1;
    }

    // check file_data_offset and file_len is valid
    if ((ptr_chunk_info->file_data_offset + ptr_chunk_info->file_data_len) != ptr_chunk_info->file_hash_offset)
    {
        UBOOT_ERROR("The parameter in chunk info is not valid \n");
        return -1;
    }

    // check hash_offset and hash_len overflow
    if (ptr_chunk_info->file_hash_offset > (ptr_chunk_info->file_hash_offset + ptr_chunk_info->file_hash_len))
    {
        UBOOT_ERROR("The parameter in chunk info overflow \n");
        return -1;
    }

    // check hash_offset and hash_len is valid
    if ((ptr_chunk_info->file_hash_offset + ptr_chunk_info->file_hash_len) != (upgrade_file_size))
    {
        UBOOT_ERROR("The parameter in chunk info is not valid \n");
        return -1;
    }

    UBOOT_DEBUG("REE segment size=0x%x\n",ptr_chunk_info->segment_size);
    UBOOT_DEBUG("REE file_data_offset=0x%x\n",ptr_chunk_info->file_data_offset);
    UBOOT_DEBUG("REE file_data_len=0x%x\n",ptr_chunk_info->file_data_len);
    UBOOT_DEBUG("REE file_hash_offset=0x%x\n",ptr_chunk_info->file_hash_offset);
    UBOOT_DEBUG("REE fileHashLen=0x%x\n",ptr_chunk_info->file_hash_len);
    UBOOT_DEBUG("REE upgrade_download_buf=0x%x\n",ptr_chunk_info->download_buf);
    UBOOT_TRACE("OK\n");
    return 0;
}

int check_image_segement_SHA(st_chunk_info *ptr_chunk_info, char *target_interface, char *target_device, char *target_file, unsigned char* HashSetBuf, int is_descrypted)
{
    int i = 0, n;
    unsigned char *pt=NULL;
    unsigned char *segment_buf = NULL;
    unsigned int read_len=0;
    unsigned int file_data_len=0;
    unsigned int file_data_offset=0;
    unsigned int file_hash_offset=0;
    unsigned char *upgrade_download_buf=NULL;
    unsigned int upgrade_segment_size=0;
    unsigned long sha_digest_size=SHA_DIGEST_SIZE;
    static unsigned char hash_out_buf_align[SHA_DIGEST_SIZE] __attribute__((aligned(HW_SHA_ALIGN_SIZE)))={0x0};

    char buffer[CMD_BUF]="\0";
    UBOOT_TRACE("IN\n");

    // segmentSize is global variable, set it for partload segment used.
    upgrade_segment_size = ptr_chunk_info->segment_size;
    file_data_len = ptr_chunk_info->file_data_len;
    file_data_offset = ptr_chunk_info->file_data_offset;
    file_hash_offset = ptr_chunk_info->file_hash_offset;
    upgrade_download_buf = (unsigned char*)((unsigned long)ptr_chunk_info->download_buf);

    UBOOT_DEBUG("upgrade_segment_size=0x%x\n", upgrade_segment_size);
    UBOOT_DEBUG("file_data_len=0x%x\n", file_data_len);
    UBOOT_DEBUG("file_data_offset=0x%x\n", file_data_offset);
    UBOOT_DEBUG("file_hash_offset=0x%x\n", file_hash_offset);
    UBOOT_DEBUG("upgrade_download_buf=%p\n", upgrade_download_buf);

    if (upgrade_download_buf == 0)//this means the gen_upgrade_image.py is still not updated yet.
    {
        upgrade_download_buf = (unsigned char*)SECURE_UPGRADE_TMP_BUF_ADDR;
        UBOOT_INFO("no upgrade_download_buf info in PKG, use default SECURE_UPGRADE_TMP_BUF_ADDR: %p\n",upgrade_download_buf);
    }

    segment_buf = upgrade_download_buf;
    pt = HashSetBuf;
    while(file_data_len)
    {
        read_len=(file_data_len>=upgrade_segment_size)?upgrade_segment_size:file_data_len;

        memset(buffer,0,CMD_BUF);
        n = snprintf(buffer, CMD_BUF, "filepartloadSegAES %s %s %p %s %x %x", target_interface, target_device, segment_buf, target_file, read_len, file_data_offset);
        if (n < 0 || n >= CMD_BUF)
        {
            UBOOT_ERROR("snprintf fail: %d\n", n);
            return -1;
        }
        UBOOT_DEBUG("cmd=%s\n", buffer);
        if(run_command(buffer, 0)!=0)
        {
            UBOOT_ERROR("Cmd:'%s' fail!!\n",buffer);
            return -1;
        }

        UBOOT_INFO("*");

        if(common_sha(segment_buf,hash_out_buf_align,(MS_U32)read_len)!=0)
        {
            UBOOT_ERROR("CommonSHA Fail!!\n");
            return -1;
        }

        for(i = 0; i<sha_digest_size; i++)
        {
            if(hash_out_buf_align[i] != pt[i])
            {
                UBOOT_ERROR("hash_out_buf_align[%d]=0x%x, *pt=0x%x \n",i,hash_out_buf_align[i],*pt);
                return -1;
            }
        }

        file_data_len-=read_len;
        file_data_offset+=read_len;
        pt+=sha_digest_size;

        UBOOT_DEBUG("read_len=0x%x\n",read_len);
        UBOOT_DEBUG("file_data_len=0x%x\n",file_data_len);
        UBOOT_DEBUG("file_data_offset=0x%x\n",file_data_offset);
        UBOOT_DEBUG("pHashOffset=%p\n", pt);
    }

    UBOOT_INFO("\n");
    UBOOT_TRACE("OK\n");
    return 0;
}


int do_file_segment_rsa_authendication(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    #define TARGET_INTERFACE argv[1]
    #define TARGET_DEVICE  argv[2]
    #define TARGET_FILE  argv[3]
    #undef RSA_PUBLIC_KEY_LEN
    #define RSA_PUBLIC_KEY_LEN (RSA_PUBLIC_KEY_N_LEN+RSA_PUBLIC_KEY_E_LEN)

    char buffer[CMD_BUF]="\0";
    int ret = 0, n;
    unsigned char rsa_public_key_N[RSA_PUBLIC_KEY_N_LEN]={0x0};
    unsigned char rsa_public_key_E[RSA_PUBLIC_KEY_E_LEN]={0x0};
    unsigned char hashset_signature[SIGNATURE_SIZE]={0x0};
    unsigned char *hashset_buf_align = NULL;
    unsigned int load_offset = 0;
    unsigned int upgrade_file_size = 0;
    unsigned char *upgrade_tmp_buffer = (unsigned char*)CONFIG_UPGRADE_BUFFER_ADDR; //ToDo: remove hardcode after LMB problem.
    unsigned char *upgrade_header_buffer = (unsigned char*)UPGRADE_HEADER_BUFFER_ADDR;
    st_chunk_info ree_chunk_info={0};

    UBOOT_TRACE("IN\n");
    if(argc<4)
        return -1;

    //Get total file size
    upgrade_file_size = get_file_size_for_upgrade(TARGET_INTERFACE, TARGET_DEVICE, TARGET_FILE);
    if (upgrade_file_size == 0 || ((unsigned long)upgrade_file_size >= (unsigned long)get_storage_size()))
    {
        UBOOT_ERROR("upgrade_file_size is invalid\n");
        return -1;
    }

    /* Read REE chunk info from header, the offset is 0x100+0x200+0200 in header */
    load_offset = SIGNATURE_SIZE + HEADER_VERSION_SIZE + HEADER_IMAGE_OFFSET_SIZE;
    memcpy(&ree_chunk_info, (upgrade_header_buffer+load_offset), HEADER_CHUNKINFO_SIZE);
    UBOOT_DUMP(&ree_chunk_info, HEADER_CHUNKINFO_SIZE);

    /* Read signature of REE's hashset from header, the offset is 0x100+0x200+0x80+0x80+0x100 in headera */
    load_offset = SIGNATURE_SIZE + HEADER_VERSION_SIZE + HEADER_IMAGE_OFFSET_SIZE + HEADER_CHUNKINFO_SIZE;
    memcpy(hashset_signature, (upgrade_header_buffer+load_offset), SIGNATURE_SIZE);
    UBOOT_DUMP(hashset_signature, SIGNATURE_SIZE);

    // check chunk info
    if (check_chunk_info(&ree_chunk_info, upgrade_file_size) != 0)
    {
        UBOOT_ERROR("check chunk info fail\n");
        return -1;
    }

    //Get rsa upgrade public key
    ret = get_public_key_n(KEY_TYPE_CKB_A_UPG_KEY, rsa_public_key_N, RSA_PUBLIC_KEY_N_LEN);
    if (ret != 0)
    {
        UBOOT_ERROR("Get public key N fail\n");
        return -1;
    }
    UBOOT_DUMP(rsa_public_key_N, RSA_PUBLIC_KEY_N_LEN);

    ret = get_public_key_e(KEY_TYPE_CKB_A_UPG_KEY, rsa_public_key_E, RSA_PUBLIC_KEY_E_LEN);
    if (ret != 0)
    {
        UBOOT_ERROR("Get public key E fail\n");
        return -1;
    }
    UBOOT_DUMP(rsa_public_key_E, RSA_PUBLIC_KEY_E_LEN);

    memset(buffer,0,CMD_BUF);
    n = snprintf(buffer, CMD_BUF, "filepartload %p %s 0x%x 0x%x", upgrade_tmp_buffer, TARGET_FILE, ree_chunk_info.file_hash_len, ree_chunk_info.file_hash_offset);
    if (n < 0 || n >= CMD_BUF)
    {
        UBOOT_ERROR("snprintf fail: %d\n", n);
        return -1;
    }
    UBOOT_DEBUG("Cmd= '%s'\n", buffer);
    if(run_command(buffer, 0)!=0)
    {
        UBOOT_ERROR("Cmd:'%s' fail!!\n",buffer);
        return -1;
    }

    hashset_buf_align=(unsigned char*)memalign(HW_SHA_ALIGN_SIZE,ree_chunk_info.file_hash_len);
    if(hashset_buf_align==NULL)
    {
        UBOOT_ERROR("[ERROR] hashset_buf_align memroy allocate fail\n");
        return -1;
    }

    memcpy(hashset_buf_align, upgrade_tmp_buffer, ree_chunk_info.file_hash_len);
    UBOOT_INFO("Verify PKG HASHSET...\n");

    ret = secure_do_authentication(rsa_public_key_N, rsa_public_key_E, hashset_signature, hashset_buf_align, ree_chunk_info.file_hash_len);
    if(ret!=0)
    {
        UBOOT_ERROR("\033[0;31m RSA upgrade public-N key=\033[0m\n");
        UBOOT_ERRDUMP(rsa_public_key_N, RSA_PUBLIC_KEY_N_LEN);

        UBOOT_ERROR("\033[0;31m RSA upgrade public-E key=\033[0m\n");
        UBOOT_ERRDUMP(rsa_public_key_E, RSA_PUBLIC_KEY_E_LEN);

        UBOOT_ERROR("\033[0;31m signature=\033[0m\n");
        UBOOT_ERRDUMP(hashset_signature, SIGNATURE_LEN);

        UBOOT_ERROR("\033[0;31m clear image=\033[0m\n");
        UBOOT_ERRDUMP(hashset_buf_align, ree_chunk_info.file_hash_len);
        free(hashset_buf_align);
        secure_halt_cpu();
        return -1;
    }
    else
    {
        #if defined(CONFIG_SECURE_DEBUG_MESSAGE)
        UBOOT_INFO("\033[0;34m******************************************\033[0m\n");
        UBOOT_INFO("\033[0;34m* HASHSET Authentication SUCCESS !! *\033[0m\n");
        UBOOT_INFO("\033[0;34m******************************************\033[0m\n");
        #endif
    }

    // Do rsa authentication for ***.hash
    UBOOT_INFO("REE Image Segment SHA Authentication ....\n");
    set_update_mode_by_interface(TARGET_INTERFACE, "DECRYPTED");
    ret = check_image_segement_SHA(&ree_chunk_info, TARGET_INTERFACE, TARGET_DEVICE, TARGET_FILE,hashset_buf_align, 1);
    if( ret != 0 )
    {
        free(hashset_buf_align);
        return -1;
    }
    UBOOT_INFO("\n");

    free(hashset_buf_align);
    UBOOT_TRACE("OK\n");
    return 0;
}

int do_file_part_load_with_segment_aes_decrypted(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    #define DOWNLOAD_INTERFACE argv[1]
    #define DOWNLOAD_DEVICE argv[2]
    #define DOWNLOAD_ADDR argv[3]
    #define DOWNLOAD_FILE argv[4]
    #define DOWNLOAD_LENGTH argv[5]
    #define DOWNLOAD_OFFSET argv[6]
    static unsigned char aes_key[AES_KEY_LEN]={0};
    unsigned int download_offset=0;
    unsigned int download_len=0;
    unsigned char *download_addr= NULL;
    unsigned int aes_decode_len=0;
    unsigned int upgrade_file_size=0;
    unsigned int load_offset = 0;
    int snprintf_len = 0;
    unsigned char *aes_iv_buffer = (unsigned char *)CONFIG_UPGRADE_BUFFER_ADDR;
    unsigned char *upgrade_header_buffer = (unsigned char*)UPGRADE_HEADER_BUFFER_ADDR;
    unsigned char IV[16]="\0";
    char buffer[CMD_BUF]="\0";
    st_chunk_info ree_chunk_info = {0};
    int aes_result=0;

    UBOOT_TRACE("IN\n");

    if(argc<7)
    {
        return -1;
    }

    download_addr = (unsigned char*)simple_strtoul(DOWNLOAD_ADDR, NULL, 16);
    download_offset = (unsigned int)simple_strtoul(DOWNLOAD_OFFSET, NULL, 16);
    download_len = (unsigned int)simple_strtoul(DOWNLOAD_LENGTH, NULL, 16);

    //Get total file size
    upgrade_file_size = get_file_size_for_upgrade(DOWNLOAD_INTERFACE, DOWNLOAD_DEVICE, DOWNLOAD_FILE);
    if (upgrade_file_size == 0 || ((unsigned long)upgrade_file_size >= (unsigned long)get_storage_size()))
    {
        UBOOT_ERROR("upgrade_file_size is invalid\n");
        return -1;
    }

    /* Read REE chunk info from header, the offset is 0x100+0x200+0200 in header */
    load_offset = SIGNATURE_SIZE + HEADER_VERSION_SIZE + HEADER_IMAGE_OFFSET_SIZE;
    memcpy(&ree_chunk_info, (upgrade_header_buffer+load_offset), HEADER_CHUNKINFO_SIZE);
    UBOOT_DUMP(&ree_chunk_info, HEADER_CHUNKINFO_SIZE);

    // check chunk info
    if (check_chunk_info(&ree_chunk_info, upgrade_file_size) != 0)
    {
        UBOOT_ERROR("check chunk info fail\n");
        return -1;
    }

    //Get AES decode key
    get_aes_key(KEY_TYPE_CKB_D_UPG_KEY,aes_key,AES_KEY_LEN);

    //DownloadLen Check
    if (get_aes_decode_len(download_offset, download_len, upgrade_file_size, ree_chunk_info.file_hash_len, &aes_decode_len) < 0)
    {
        UBOOT_ERROR("check download_len Error\n");
        return -1;
    }

    UBOOT_DEBUG("upgrade_file_size=0x%x\n",upgrade_file_size);
    UBOOT_DEBUG("download_offset=0x%x\n",download_offset);
    UBOOT_DEBUG("download_len=0x%x\n",download_len);
    UBOOT_DEBUG("download_addr=%p\n",download_addr);
    UBOOT_DEBUG("aes_decode_len=0x%x\n",aes_decode_len);

    // Load data to pdownloadaddr by different tunnel (USB or FS)
    if(get_upgrade_mode()==EN_UPDATE_MODE_OAD_WITH_SEG_DECRYPTED)
    {
        if(download_offset != UPGRADE_SCRIPT_OFFSET)//copy the last block for next IV input.
        {
            memset(buffer, 0, CMD_BUF);
            snprintf_len=snprintf(buffer, CMD_BUF, "ext4load %s %s %p %s 0x%x 0x%x", DOWNLOAD_INTERFACE, DOWNLOAD_DEVICE, aes_iv_buffer, DOWNLOAD_FILE, 0x200, download_offset-0x200);
            UBOOT_DEBUG("cmd=%s \n",buffer);
            if (snprintf_len < 0)
            {
                UBOOT_ERROR("snprintf Error !!!\n");
                return -1;
            }
            if (run_command(buffer, 0) != 0)
            {
                return -1;
            }

        }
        memset(buffer, 0, CMD_BUF);
        snprintf_len=snprintf(buffer, CMD_BUF, "ext4load %s %s %p %s 0x%x 0x%x", DOWNLOAD_INTERFACE,DOWNLOAD_DEVICE, download_addr, DOWNLOAD_FILE, aes_decode_len, download_offset);
        UBOOT_DEBUG("cmd=%s \n",buffer);
        if (snprintf_len < 0)
        {
            UBOOT_ERROR("snprintf Error !!!\n");
            return -1;
        }
        if (run_command(buffer, 0) != 0)
        {
            return -1;
        }
    }
    else if(get_upgrade_mode()==EN_UPDATE_MODE_USB_WITH_SEG_DECRYPTED)
    {
        if(download_offset != UPGRADE_SCRIPT_OFFSET)//copy the last block for next IV input.
        {
            memset(buffer, 0, CMD_BUF);
            snprintf_len=snprintf(buffer, CMD_BUF, "fatload %s %s %p %s 0x%x 0x%x", DOWNLOAD_INTERFACE, DOWNLOAD_DEVICE, aes_iv_buffer, DOWNLOAD_FILE, 0x200, download_offset-0x200);
            UBOOT_DEBUG("cmd=%s \n",buffer);
            if (snprintf_len < 0)
            {
                UBOOT_ERROR("snprintf Error !!!\n");
                return -1;
            }
            if (run_command(buffer, 0) != 0)
            {
                return -1;
            }

        }
        memset(buffer, 0, CMD_BUF);
        snprintf_len=snprintf(buffer, CMD_BUF, "fatload %s %s %p %s 0x%x 0x%x", DOWNLOAD_INTERFACE,DOWNLOAD_DEVICE, download_addr, DOWNLOAD_FILE, aes_decode_len, download_offset);
        UBOOT_DEBUG("cmd=%s \n", buffer);
        if (snprintf_len < 0)
        {
            UBOOT_ERROR("snprintf Error !!!\n");
            return -1;
        }
        if (run_command(buffer, 0) != 0)
        {
            return -1;
        }
    }
    else
    {
        UBOOT_ERROR("Wrong Upgrade Mode!!\n");
        return -1;
    }

    // Do AES Crypt, IV setting
    memset(IV, 0, AES_IV_LEN);
    if(download_offset != UPGRADE_SCRIPT_OFFSET)
    {
        memcpy(IV,&aes_iv_buffer[496],AES_IV_LEN);//offset 496 is IV value
    }

    aes_result = aes_main(download_addr, aes_decode_len, aes_key, IV, AES_KEY_LEN_BIT);

    if(aes_result != 0)
    {
        UBOOT_ERROR("AES Decrypt fail!\n");
        return -1;
    }
    UBOOT_DEBUG("Decrypted data (0):\n");
    UBOOT_DUMP(download_addr, 0x200);

    env_set("filesize",DOWNLOAD_LENGTH);

    UBOOT_TRACE("OK\n");
    return 0;
}

#endif
