// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
*/

#include <common.h>
#include <malloc.h>
#include <debug_impl.h>
#include <romtblo_impl.h>
#include <mtk_romtblo.h>
#include <linux/libfdt.h>

DECLARE_GLOBAL_DATA_PTR;
static char *romtbl_base = NULL;
static struct list_head gic_base_list;
romtbl_header header = {0};

void show(romtbl_info rom_table_info)
{
    uint32_t i = 0;

    UBOOT_DEBUG("==============================\n");
    UBOOT_DEBUG("UniqueID: 0x%X\n", rom_table_info.unique_id);
    UBOOT_DEBUG("HwIpMajorVersion: 0x%X\n", rom_table_info.hw_ip_major_ver);
    UBOOT_DEBUG("HwIpMinorVersion: 0x%X\n", rom_table_info.hw_ip_minor_ver);
    UBOOT_DEBUG("RegisterBaseAddress: 0x%X\n", rom_table_info.register_base_addr);
    UBOOT_DEBUG("RegisterSize: 0x%x\n", rom_table_info.register_size);
    UBOOT_DEBUG("GicInterruptNumber: 0x%X\n", rom_table_info.gic_interrupt_num);
    for(i=0;i<ROMTBL_CAPABILITY_SIZE;i++)
        UBOOT_DEBUG("Capability[%d]: 0x%X\n", i, rom_table_info.capability[i]);
    for(i=0;i<ROMTBL_REGISTER_SIZE;i++)
    {
        UBOOT_DEBUG("Register Size[%d]: 0x%X\n", i, rom_table_info.register_size_array[i]);
        UBOOT_DEBUG("Register Address[%d]: 0x%X\n", i, rom_table_info.register_base_add_array[i]);
    }
}

unsigned int rom_table_hex_to_dec(unsigned int hex)
{
    unsigned int result = 0;
    unsigned char temp[CHIP_ID_BYTE_COUNT];
    int i = CHIP_ID_BYTE_COUNT;

    while(i != 0)
    {
        i--;
        temp[i] = (unsigned char)(hex & (HEX_DIG_SIZE-1));
        hex = hex >> BINARY_PRE_BYTE;
    }
    i = 0;
    while(i < CHIP_ID_BYTE_COUNT)
    {
        result = ((result*DECIMAL_BASE)+((unsigned int)temp[i]));
        i++;
    }
    return result;
}

ROMTBL_RETURN init_rom_table(char *romtbl)
{
    if(NULL == romtbl)
        return ROMTBL_FAILED;
    romtbl_base = romtbl;

    return ROMTBL_OK;
}

ROMTBL_RETURN check_rom_table_header(void)
{
    char *romtbl;
    int base = 0;

    if(NULL == romtbl_base)
        return ROMTBL_NOT_INITED;

    romtbl = romtbl_base;
    memset(&header, 0, sizeof(romtbl_header));
    header.major_ver = ((romtbl[base]<<BIT_PRE_BYTE)<<BIT_PRE_BYTE);
    base++;
    header.major_ver |= (romtbl[base]<<BIT_PRE_BYTE);
    base++;
    header.major_ver |= romtbl[base];
    base++;
    header.minor_ver = romtbl[base];
    base = CHIP_ID_BASE;
    header.chip = rom_table_hex_to_dec((unsigned int)(romtbl[base]|(romtbl[base+1]<<BIT_PRE_BYTE)));
    base = REVISION_BASE;
    header.revision = (unsigned int)(romtbl[base]|(romtbl[base+1]<<BIT_PRE_BYTE));
    UBOOT_INFO("[ROM table] version:%d.%d chip:%d revision:%d\n",header.major_ver,header.minor_ver,header.chip,header.revision);
    switch(header.chip)
    {
        case MEDIATEK_TV_CHIP_ID_MACHLI:
        case MEDIATEK_TV_CHIP_ID_MANKS:
        case MEDIATEK_TV_CHIP_ID_MOKONA:
        case MEDIATEK_TV_CHIP_ID_MIFFY:
            break;
        case MEDIATEK_TV_CHIP_ID_MOKA:
            return ROMTBL_NO_NEEDED;
        default:
            return ROMTBL_FAILED;
    }

    return ROMTBL_OK;
}

ROMTBL_RETURN rom_table_parser(unsigned short unique_id, romtbl_info *rom_table_info)
{
    int i = 0;
    int j = 0;
    int extended_code = 0;
    int cap_offset = 0;
    int cap_length = 0;
    const char *romtbl = romtbl_base;
    bool found_unique_id = false;

    if(NULL == romtbl)
        return ROMTBL_NOT_INITED;

    romtbl = romtbl+ROMTBL_HEADER_SIZE;
    for(i=0; i<ROMTBL_DEVENTRY_SIZE; i+=16)
    {
        if(unique_id == *(unsigned short *)(romtbl))
        {
            rom_table_info->unique_id = *(unsigned short *)(romtbl);
            rom_table_info->hw_ip_major_ver = *(char *)(romtbl+2);
            rom_table_info->hw_ip_minor_ver = *(char *)(romtbl+3);
            rom_table_info->gic_interrupt_num = *(unsigned short *)(romtbl+4);
            rom_table_info->register_size = *(unsigned short *)(romtbl+6);
            rom_table_info->register_base_addr = *(int *)(romtbl+8);
            extended_code = rom_table_info->register_base_addr;
            if(extended_code&0x80000000)
            {
                cap_offset = extended_code&0x0000FFFF;
                cap_length = (extended_code&0x7FFF0000)>>16;
                for(j=0; j<(cap_length/8); j++)
                {
                    rom_table_info->register_size_array[j] = *(int *)(romtbl_base+cap_offset+(j * 8));
                    rom_table_info->register_base_add_array[j] = *(int *)(romtbl_base+cap_offset+(j * 8)+4);
                }
            }
            rom_table_info->capability[0] = *(int *)(romtbl+12);
            extended_code = rom_table_info->capability[0];
            if(extended_code&0x80000000)
            {
                cap_offset = extended_code&0x0000FFFF;
                cap_length = (extended_code&0x7FFF0000)>>16;
                for(j=0; j<(cap_length/4); j++)
                {
                    rom_table_info->capability[j] = *(int *)(romtbl_base+cap_offset+(j * 4));
                }
            }
            found_unique_id = true;
            break;
        }
        romtbl = romtbl+16;
    }
    if(false == found_unique_id)
        return ROMTBL_ID_NOTFOUND;
    return ROMTBL_OK;
}

unsigned int gic_number_transfer(unsigned int rom_interrupt)
{
    struct list_head *ptr;
    ROM_GIC_LIST_T *rom_gic_ptr;
    int sum;

    sum = rom_interrupt + GIC_INTERRUPT_BASE;
    list_for_each(ptr, &gic_base_list)
    {
        rom_gic_ptr = list_entry(ptr, ROM_GIC_LIST_T, list);
        if(rom_gic_ptr->interrupt_base <= sum && (rom_gic_ptr->interrupt_base + rom_gic_ptr->size) > sum)
        {
            return (rom_interrupt+GIC_INTERRUPT_BASE-rom_gic_ptr->interrupt_base);
        }
    }

    return -1;
}

int romtbl_overlay_source_prepare(char *propname, romtbl_info *rom_table_info, fdt32_t *cells, __be32 *data, int *index)
{
    int i=0,j=0;
    int interrupt=0;

    if(strncmp(propname,"reg",3) == 0)
    {
        if((rom_table_info->register_base_addr & (0x1 << 31)) == 0)
        {
            cells[i++] = cpu_to_fdt32(ROMTBL_64BIT_HIGH_BYTE_VALUE);
            cells[i++] = cpu_to_fdt32(rom_table_info->register_base_addr);
            cells[i++] = cpu_to_fdt32(ROMTBL_64BIT_HIGH_BYTE_VALUE);
            cells[i++] = cpu_to_fdt32(rom_table_info->register_size);
        }
        else
        {
            while(rom_table_info->register_base_add_array[j] != 0)
            {
                cells[i++] = cpu_to_fdt32(ROMTBL_64BIT_HIGH_BYTE_VALUE);
                cells[i++] = cpu_to_fdt32(rom_table_info->register_base_add_array[j]);
                cells[i++] = cpu_to_fdt32(ROMTBL_64BIT_HIGH_BYTE_VALUE);
                cells[i++] = cpu_to_fdt32(rom_table_info->register_size_array[j]);
                j++;
            }
        }
    }
    else if(strncmp(propname,"cap",3) == 0)
    {
        while(rom_table_info->capability[i] != 0)
        {
            cells[i++] = cpu_to_fdt32(rom_table_info->capability[j]);
            j++;
        }
    }
    else if(strncmp(propname,"int",3) == 0)
    {
        interrupt = gic_number_transfer(rom_table_info->gic_interrupt_num);
        cells[i++] = data[0];
        if(interrupt == -1)
        {
            UBOOT_DEBUG("gic_base_list is empty or intrrupt number is bigger than maximun.\n");
            i++;
        }
        else
        {
            cells[i++] = cpu_to_fdt32(interrupt);
        }
        cells[i++] = data[2];
    }
    else if(strncmp(propname,"ip-",3) == 0)
    {
        cells[i++] = cpu_to_fdt32(((rom_table_info->hw_ip_major_ver << 8)| rom_table_info->hw_ip_minor_ver));
    }
    else
    {
        UBOOT_ERROR("Property is not support.\n");
    }

    *index = i;
    return 0;
}

void romtbl_integer_overlay(void *fdt, unsigned int offset, char *name, romtbl_info *rom_table_info)
{
    fdt32_t cells[ROMTBL_REGISTER_SIZE*ROMTBL_REGISTER_PAIR];
    const struct fdt_property *fdt_prop;
    __be32 *data;
    char propname[16];
    int index=0;
    int ret;

    memset(cells, 0, (sizeof(fdt32_t)*ROMTBL_REGISTER_SIZE*ROMTBL_REGISTER_PAIR));
    memset(propname, 0, sizeof(propname));
    strncpy(propname,name,sizeof(propname)-1);
    propname[(sizeof(propname)-1)] = '\0';
    fdt_prop = fdt_get_property(fdt, offset, propname, NULL);
    if(fdt_prop != NULL)
    {
        data = (__be32 *)fdt_prop->data;
        romtbl_overlay_source_prepare(propname, rom_table_info, cells, data, &index);
        ret = fdt_setprop(fdt, offset, propname, cells, (sizeof(cells[0])*index));
        if(ret < 0)
        {
            ret = fdt_shrink_to_minimum(fdt,(sizeof(fdt32_t)*index));
            if(ret <= 0)
            {
                UBOOT_ERROR("fdt_shrink_to_minimum failure, memory is not enough for overlay.\n");
            }
            else
            {
                ret = fdt_setprop(fdt, offset, propname, cells, (sizeof(cells[0])*index));
                if(ret < 0)
                    UBOOT_ERROR("Do ROM table overlay failure after fdt_shrink_to_minimum\n");
                else
                    UBOOT_INFO("Property %s overlay success.\n",name);
            }
        }
        else
        {
            UBOOT_INFO("Property %s overlay success.\n",name);
        }
    }
}

int romtbl_overlay(unsigned long dtb_addr)
{
    romtbl_info rom_table_info = {0};
    unsigned long *romtbl;
    const void *propvalue;
    char propname[16]={0};
    void *fdt;
    unsigned int offset = 0, offset_old = 0,ret;
    unsigned int unique_id;
    int len;
    __be32 *data;

    fdt = (void *)dtb_addr;
    romtbl = (unsigned long *)CONFIG_ROM_TABLE_ADDRESS;
    init_rom_table((char *)romtbl);
    strncpy(propname, "unique-id",sizeof(propname)-1);
    ret = fdt_check_header(fdt);
    if (ret != 0)
    {
        UBOOT_ERROR("DTB data address %p is wrong, please check again.\n",fdt);
        return ret;
    }

    for (offset = fdt_next_node(fdt, -1, NULL);
         offset >= 0;
         offset = fdt_next_node(fdt, offset, NULL))
    {
        if(offset_old > offset)
            break;
        propvalue = fdt_getprop(fdt, offset, propname, &len);
        if(propvalue != NULL)
        {
            data = (__be32 *)propvalue;
            unique_id = fdt32_to_cpu(data[0]);
            UBOOT_DEBUG("DTB offset:0x%X %s:0x%X\n",offset,propname,unique_id);
            memset(&rom_table_info,0,sizeof(romtbl_info));
            ret = rom_table_parser(unique_id, &rom_table_info);
            if(ret == ROMTBL_OK)
            {
                show(rom_table_info);
                if((rom_table_info.hw_ip_major_ver << 8| rom_table_info.hw_ip_minor_ver) != 0)
                {
                    romtbl_integer_overlay(fdt, offset, "ip-version", &rom_table_info);
                }
                if(rom_table_info.capability[0] != 0)
                {
                    romtbl_integer_overlay(fdt, offset, "capability", &rom_table_info);
                }
                if(rom_table_info.register_base_addr != 0)
                {
                    romtbl_integer_overlay(fdt, offset, "reg", &rom_table_info);
                }
                if(rom_table_info.gic_interrupt_num != 0)
                {
                    romtbl_integer_overlay(fdt, offset, "interrupts", &rom_table_info);
                }
            }
        }
        offset_old = offset;
    }

    UBOOT_INFO("ROM table overlay finish.\n");
    return 0;
}

int romtbl_gic_parser(unsigned long dtb_addr, unsigned long dtb_size)
{
    const struct fdt_property *fdt_prop;
    void *fdt;
    __be32 *data;
    ROM_GIC_LIST_T *rom_gic_entry;
    unsigned int offset = 0, offset_old = 0;
    int ret;

    fdt = (void *)dtb_addr;
    ret = fdt_check_header(fdt);
    if (ret != 0)
    {
        printf("DTB data address %p is wrong, please check again.\n",fdt);
        return ret;
    }

    for (offset = fdt_next_node(fdt, -1, NULL);
         offset >= 0;
         offset = fdt_next_node(fdt, offset, NULL))
    {
        if(offset_old > offset)
            break;
        fdt_prop = fdt_get_property(fdt, offset, "gic_spi", NULL);
        if(fdt_prop != NULL)
        {
            data = (__be32 *)fdt_prop->data;
            rom_gic_entry = (ROM_GIC_LIST_T *)(unsigned long)malloc(sizeof(ROM_GIC_LIST_T));
            if(rom_gic_entry == NULL)
            {
                printf("Can't allocate %lx size memory for rom_gic_entry. Finish romtbl_gic_parser.\n",(unsigned long)sizeof(ROM_GIC_LIST_T));
                break;
            }
            rom_gic_entry->interrupt_base = fdt32_to_cpu(data[0]);
            rom_gic_entry->size = fdt32_to_cpu(data[1]);
            UBOOT_DEBUG("[GIC] interrupt:0x%X size:0x%X\n",rom_gic_entry->interrupt_base,rom_gic_entry->size);
            list_add_tail(&rom_gic_entry->list,&gic_base_list);
        }
        offset_old = offset;
    }
    UBOOT_INFO("Get gic information from ROM table finish.\n");
    return 0;
}

int romtbl_overlay_to_uboot_dtb(void)
{
    int ret = 0;
    INIT_LIST_HEAD(&gic_base_list);

    ret = romtbl_gic_parser((unsigned long)(gd->fdt_blob),(unsigned long)(gd->fdt_size));
    if(ret != 0)
    {
        printf("Get gic information from ROM table failure\n");
    }

    ret = romtbl_overlay((unsigned long)(gd->fdt_blob));
    if(ret != 0)
    {
        printf("Rom table overlay failure in u-boot stage.\n");
        return -1;
    }

    return 0;
}

int romtbl_get_chip_id_info(void)
{
    switch(header.chip)
    {
        case MEDIATEK_TV_CHIP_ID_MACHLI:
            UBOOT_INFO("Chip id:%d\n",header.chip);
            return MT5896;
        case MEDIATEK_TV_CHIP_ID_MANKS:
            UBOOT_INFO("Chip id:%d\n",header.chip);
            return MT5897;
        case MEDIATEK_TV_CHIP_ID_MOKONA:
            UBOOT_INFO("Chip id:%d\n",header.chip);
            return MT5876;
        case MEDIATEK_TV_CHIP_ID_MIFFY:
            UBOOT_INFO("Chip id:%d\n",header.chip);
            return MT5879;
        case MEDIATEK_TV_CHIP_ID_MOKA:
            UBOOT_INFO("Chip id:%d\n",header.chip);
            return MT5873;
        default:
            UBOOT_ERROR("Chip id %d is not support\n",header.chip);
            return -1;
    }
}

unsigned int romtbl_get_chip_id_info_dec(void)
{
    UBOOT_INFO("Chip id:%d\n",header.chip);
    return header.chip;
}

unsigned int romtbl_get_chip_revision_info(void)
{
    UBOOT_INFO("Chip revision:%d\n",header.revision);
    return header.revision;
}
