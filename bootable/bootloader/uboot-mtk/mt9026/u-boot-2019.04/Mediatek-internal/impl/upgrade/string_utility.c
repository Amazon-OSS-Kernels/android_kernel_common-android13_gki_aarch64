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

#include <stdio.h>
#include <common.h>
#include <command.h>
#include <MsTypes.h>
#include "language.h"
#include "loader_charset.h"
#include "loader_charset_eng.h"

#define STRBUFF_LENGTH       256
#define OSD_LANGUAGE         "osd_language"
#define LANGSTR(ID,OFFSET)   p_language_array[(ID)*6+(OFFSET)]
#define LANGSTRLEN(ID)       (LANGSTR(ID,0)<<8)|(LANGSTR(ID,1))
#define LANGSTRADDR(ID)      (LANGSTR(ID,2)<<24)|(LANGSTR(ID,3)<<16)|(LANGSTR(ID,4)<<8)|LANGSTR(ID,5)
#define ENGLISH              0
#define CHINESE              1
#define RUSSIAN              2


typedef struct{
  char *language;
  u8 languagenum;
}language_id;

language_id language_id_list[]={
    {"English",ENGLISH},
    {"Chinese",CHINESE},
    {"Russian",RUSSIAN},
};

static u8 char_buffer[STRBUFF_LENGTH];

const u8* p_loader_character_set = NULL;
const u8* p_loader_font = NULL;
const u8* p_loader_width_data=NULL;

u8 get_language_id(void)
{
    int  i,size;
    u8* language=NULL;
    unsigned char env=FALSE;
    language=(u8*)env_get(OSD_LANGUAGE);
    if(language == NULL)
    {
        printf("Failed to get the language from the environment,please check");
        return 0;
    }

    size=sizeof(language_id_list)/sizeof(language_id);
    for(i=0;i<size;i++)
    {
        if(strcmp((const char *)language,(const char *)language_id_list[i].language) == 0)
        {
            env= TRUE;
            break;
        }
        else
        {
            env=FALSE;
        }
    }
    if(env == TRUE)
    {
        return language_id_list[i].languagenum;
    }
    else
    {
        return language_id_list[0].languagenum;
    }
}

u8* osd_get_string(u32 id)
{
    const u8* p_language_array=NULL;
    u8 language_id;
    u16 strlength;
    u32 straddr;
    u16 i=0;
    memset(char_buffer,0,STRBUFF_LENGTH);
    id=id&0xffff;
    language_id=get_language_id();
    switch(language_id)
    {
        case 0:
            p_language_array=language0;
            break;
        case 1:
            p_language_array=language1;
            p_loader_character_set=loader_character_set1;
            p_loader_font=loader_font1;
            p_loader_width_data=loader_width_data1;
            break;
        case 2:
            p_language_array=language2;
            p_loader_character_set=loader_character_set2;
            p_loader_font=loader_font2;
            p_loader_width_data=loader_width_data2;
            break;
        case 3:
            p_language_array=language3;
            p_loader_character_set=loader_character_set3;
            p_loader_font=loader_font3;
            p_loader_width_data=loader_width_data3;
            break;
        case 4:
            p_language_array=language4;
            p_loader_character_set=loader_character_set4;
            p_loader_font=loader_font4;
            p_loader_width_data=loader_width_data4;
            break;
        case 5:
            p_language_array=language5;
            p_loader_character_set=loader_character_set5;
            p_loader_font=loader_font5;
            p_loader_width_data=loader_width_data5;
            break;
        case 6:
            p_language_array=language6;
            p_loader_character_set=loader_character_set6;
            p_loader_font=loader_font6;
            p_loader_width_data=loader_width_data6;
            break;
        case 7:
            p_language_array=language7;
            p_loader_character_set=loader_character_set7;
            p_loader_font=loader_font7;
            p_loader_width_data=loader_width_data7;
            break;
        default:
            printf("no more language");
            break;
    }

    if(p_language_array != NULL)
    {
        strlength=LANGSTRLEN(id);
        straddr=LANGSTRADDR(id);
        for(i=0;i<strlength;i++)
        {
            char_buffer[i]=p_language_array[straddr+i];
        }
        char_buffer[i]=' '; //space key
    }

    return char_buffer;

}

