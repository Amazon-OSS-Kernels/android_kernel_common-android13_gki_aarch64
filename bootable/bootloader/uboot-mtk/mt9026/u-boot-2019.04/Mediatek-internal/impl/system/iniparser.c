// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <vsprintf.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <debug_impl.h>
#include "iniparser.h"

#ifdef CONFIG_AMZ_ODMTVCONFIG_DTBO_OVERLAY
#define MAX_LINE_LENGTH        256
#else
#define MAX_LINE_LENGTH        128
#endif

int iniparser_create(const unsigned char *rawdata, const unsigned int length,
					 iniparser_handle_t *out_handle)
{
	char *line, *ptr;
	struct section *section_entry = NULL;
	struct keyval_pair keyval_entry,*new_keyval_entry;
	struct iniparser_info *info;

	UBOOT_TRACE("IN\n");
	if (!rawdata || !length || !out_handle)
		return -EINVAL;

	ptr = malloc(sizeof(struct iniparser_info)+length+1);
	if (!ptr) {
		UBOOT_ERROR("Error: malloc failure\n");
		*out_handle = NULL;
		return -ENOMEM;
	}
	*out_handle = info = (struct iniparser_info*)ptr;

	info->index.prev = info->index.next = &info->index;
	info->rawdata = ptr + sizeof(struct iniparser_info);
	memcpy(info->rawdata, rawdata, length);
	info->rawdata[length] = '\0';

	line = strtok(info->rawdata, "\r\n");
	while (line != NULL){
		if (strnlen(line, MAX_LINE_LENGTH+1) == MAX_LINE_LENGTH+1){
			UBOOT_ERROR("Error: line length is over %d\n",MAX_LINE_LENGTH);
			return -EINVAL;
		}
		if (line[0] == '\r' || line[0] == '\n') {	// skip empty line
			line = strtok(NULL, "\n");
            continue;
		}
        if (line[0] == '#' || line[0] == ';') {
			line = strtok(NULL, "\n");
            continue;
        }

		if (line[0] == '[') {
			line++;
			ptr = line;
			while(*ptr != ']' && *ptr !='\0')
				ptr++;
			if (*ptr == ']')
				*ptr = '\0';

			section_entry = (struct section *)malloc(sizeof(struct section));
			if (!section_entry){
				UBOOT_ERROR("Error: allocate section_entry failure.\n");
				return -ENOMEM;
			}

			section_entry->section = line;		// keep xxx of [xxx] into section
			INIT_LIST_HEAD(&section_entry->section_node);
			INIT_LIST_HEAD(&section_entry->keyval_node);
			list_add_tail(&section_entry->section_node, &info->index);
			UBOOT_DEBUG("section_entry(%p):%s\n",section_entry,section_entry->section);
		} else {
			keyval_entry.key = line;
			keyval_entry.update_key = false;
			keyval_entry.update_value = false;
			INIT_LIST_HEAD(&keyval_entry.node);

			// find separator '=' or end of line
			while(*line != '=' && *line !='\0')
				line++;
			if (*line == '=') {		// if found '='
				ptr = line;
				// replace '=' or ' ' or '\t' to '\0'
				do {
					*ptr-- = '\0';
				} while (*ptr == ' ' || *ptr == '\t');

				line++;	// go to dtbo_id

				while(*line == ' ' || *line =='\t')	// skip ' ' and '\t'
					line++;

				if (*line == '\n')
					return -EINVAL;
				else
					keyval_entry.value = line;

				while(*line != '\0') {	// remove '\r' '\n' ';' '#' at the end of value
					if (*line == '\r' || *line == '\n' || *line == ';' || *line == '#') {
						*line = '\0';
						break;
					} else
						line++;
				}

				UBOOT_DEBUG("keyval_entry:%p name:%s value:%s\n",&keyval_entry,keyval_entry.key,keyval_entry.value);
			} else {
				UBOOT_DEBUG("parsing error at line: '%s'\n", line);
				return -EINVAL;
			}

			if(section_entry == NULL)
			{
				UBOOT_ERROR("section_entry is NULL\n");
				return -EINVAL;
			}
			new_keyval_entry = (struct keyval_pair *)malloc(sizeof(struct keyval_pair));
			if (!new_keyval_entry){
				UBOOT_ERROR("Error: allocate keyval_entry failure.\n");
				return -ENOMEM;
			}
			memcpy(new_keyval_entry, &keyval_entry, sizeof(keyval_entry));

			INIT_LIST_HEAD(&new_keyval_entry->node);
			list_add_tail(&new_keyval_entry->node, &section_entry->keyval_node);
		}
		line = strtok(NULL, "\n");
	}
	UBOOT_TRACE("OUT\n");
	return 0;
}

int iniparser_destroy(const iniparser_handle_t handle)
{
	struct section *section_entry, *tmp_section_entry;
	struct keyval_pair *keyval_entry, *tmp_keyval_entry;
	struct iniparser_info *info = handle;

	UBOOT_TRACE("IN\n");
	if (!info){
		UBOOT_ERROR("Error: info pointer is NULL\n");
		UBOOT_TRACE("OUT\n");
		return -EINVAL;
	}

	list_for_each_entry_safe(section_entry, tmp_section_entry, &info->index, section_node) {
		list_for_each_entry_safe(keyval_entry, tmp_keyval_entry, &section_entry->keyval_node, node) {
			UBOOT_DEBUG("keyval_entry:%p name:%s\n",keyval_entry,keyval_entry->key);
			if (keyval_entry->update_key)
				free(keyval_entry->key);
			if (keyval_entry->update_value)
				free(keyval_entry->value);
			free(keyval_entry);
		}
		UBOOT_DEBUG("section_entry:%p name:%s\n",section_entry,section_entry->section);
		free(section_entry);
	}
	UBOOT_DEBUG("handle:%p\n",info);
	free(info);
	UBOOT_TRACE("OUT\n");
	return 0;
}

int iniparser_get_section(const iniparser_handle_t handle, const char *section_name, struct section **section_entry_out)
{
	struct section *section_entry;
	struct iniparser_info *info = handle;

	if (!handle || !section_name || !section_entry_out)
		return -EINVAL;

	list_for_each_entry(section_entry, &info->index, section_node) {
		if (strncmp(section_entry->section, section_name, strlen(section_name)) == 0 && strlen(section_entry->section) == strlen(section_name)) {
			*section_entry_out = section_entry;
			return 1;	// found
		}
	}
	*section_entry_out = NULL;
	return 0; // not found
}

int iniparser_get_keyval(const struct section *section_entry, const char *key, struct keyval_pair **keyval_entry_out)
{
	struct keyval_pair *keyval_entry;

	if (!section_entry || !key || !keyval_entry_out)
		return -EINVAL;

	list_for_each_entry(keyval_entry, &section_entry->keyval_node, node) {
		if (strncmp(keyval_entry->key, key, strlen(key)) == 0 && strlen(keyval_entry->key) == strlen(key)) {
			*keyval_entry_out = keyval_entry;
			return 1;	// found
		}
	}
	*keyval_entry_out = NULL;
	return 0; // not found
}

int iniparser_getint(const struct section *section_entry, const char *key, const int defval, int *out)
{
	struct keyval_pair *keyval_entry;
	int ret;
	if (!section_entry || !key || !out)
		return -EINVAL;
	ret = iniparser_get_keyval(section_entry, key, &keyval_entry);
	if (ret == 1 && keyval_entry) {
		*out = (int)simple_strtoul(keyval_entry->value, NULL, 0); // use 0 instead of 10, to support both D and HEX.
	} else {
		*out = defval;
	}
	return ret;
}

/*
 * can strip leading space and tailing space
 * can strip comment begins with '#' or ';'
 * can be quote by "
 * cannot unescape characters like \n , \" , \# , \;
 */
int iniparser_unescape_string(char *dest, const char *src, const int size)
{
	// strip strings
	char *sp, *ep;
	int instring = 0;
	int len = 0;

	UBOOT_DEBUG("get from strings '%s'\n", src);
	sp = (char *)src;
	// skip leading space
	while (*sp != 0 && (*sp == ' ' || *sp == '\t'))
		sp++;

	if (*sp == '"') {
		instring = 1;
		sp++;
	}
	ep = sp;
	while (*ep) {
		if ((*ep == '#' || *ep == ';') && instring == 0)
			break;
		if (*ep == '"')
			break;
		ep++;
	}
	ep--;
	// skip tailing space
	while (ep > sp && (*ep == ' ' || *ep == '\t'))
		ep--;
	if (ep > sp && *ep == '"' && instring == 1)
		ep--;
	len = (ep - sp) + 1;
	if (len > size)
		len = size - 1;
	memcpy(dest, sp, len);
	dest[len] = 0;
	UBOOT_DEBUG("get dest strings '%s'\n", dest);
	return len;
}

int iniparser_getstring(const struct section *section_entry, const char *key, const char *defval, char *out, const int size)
{
	struct keyval_pair *keyval_entry;
	int ret;
	if (!section_entry || !key || !out)
		return -EINVAL;
	ret = iniparser_get_keyval(section_entry, key, &keyval_entry);
	if (ret == 1 && keyval_entry) {
		iniparser_unescape_string(out, keyval_entry->value, size);
	} else {
		strncpy(out, defval, size);
	}
	return ret;
}

int iniparser_getbool(const struct section *section_entry, const char *key, const bool defval, bool *out)
{
	struct keyval_pair *keyval_entry;
	int ret;
	char val;
	if (!section_entry || !key || !out)
		return -EINVAL;
	ret = iniparser_get_keyval(section_entry, key, &keyval_entry);
	if (ret == 1 && keyval_entry) {
		val = keyval_entry->value[0];
		if(val == '1' || val == 't' || val == 'T' || val == 'y' || val == 'Y')
		{
			*out = 1;
		}
		else if(val == '0' || val == 'f' || val == 'F' || val == 'n' || val == 'N')
		{
			*out = 0;
		}
	} else {
		*out = defval;
	}
	return ret;
}

int iniparser_set_keyval(struct keyval_pair *keyval_entry, const char *newkey, const char *newval)
{
	char *dupkey = NULL, *dupval = NULL;
	if (!keyval_entry || (!newkey && !newval))
		return -EINVAL;

	if (newkey) {
		dupkey = strdup(newkey);
		if (!dupkey)
			return -ENOMEM;
	}
	if (newval) {
		dupval = strdup(newval);
		if (!dupval && dupkey) {
			free(dupkey);
			return -ENOMEM;
		}
	}
	if (keyval_entry->update_key)
		free(keyval_entry->key);
	if (keyval_entry->update_value)
		free(keyval_entry->value);
	if (newkey) {
		keyval_entry->key = dupkey;
		keyval_entry->update_key = true;
	}
	if (newval) {
		keyval_entry->value = dupval;
		keyval_entry->update_value = true;
	}
	return 0;
}

int iniparser_setint(const struct section *section_entry, const char *key, const int val)
{
	struct keyval_pair *keyval_entry;
	int ret;
	char stringbuf[24];

	if (!section_entry || !key)
		return -EINVAL;
	ret = iniparser_get_keyval(section_entry, key, &keyval_entry);
	if (ret == 1 && keyval_entry) {
		ret = snprintf(stringbuf, sizeof(stringbuf), "%d", val);
		if(ret < 0)
			return -EINVAL;
		return iniparser_set_keyval(keyval_entry, NULL, stringbuf);
	}
	return -EINVAL;
}

int iniparser_setstring(const struct section *section_entry, const char *key, const char *val)
{
	struct keyval_pair *keyval_entry;
	int ret;

	if (!section_entry || !key || !val)
		return -EINVAL;
	ret = iniparser_get_keyval(section_entry, key, &keyval_entry);
	if (ret == 1 && keyval_entry) {
		return iniparser_set_keyval(keyval_entry, NULL, val);
	}
	return -EINVAL;
}

int iniparser_setbool(const struct section *section_entry, const char *key, const bool val)
{
	struct keyval_pair *keyval_entry;
	int ret;
	char stringbuf[24];

	if (!section_entry || !key)
		return -EINVAL;
	ret = iniparser_get_keyval(section_entry, key, &keyval_entry);
	if (ret == 1 && keyval_entry) {
		ret = snprintf(stringbuf, sizeof(stringbuf),"%d", (int)val);
		if(ret < 0)
			return -EINVAL;
		return iniparser_set_keyval(keyval_entry, NULL, stringbuf);
	}
	return -EINVAL;
}

int iniparser_create_dumpdata(const iniparser_handle_t handle, unsigned char **rawdata_out, unsigned int *size_out)
{
	struct section *section_entry;
	struct keyval_pair *keyval_entry;
	struct iniparser_info *info = handle;
	unsigned char *rawdata_new, *ptr;
	int size = 0, updated = 0, len;

	if (!handle || !rawdata_out || !size_out)
		return -EINVAL;

	list_for_each_entry(section_entry, &info->index, section_node) {
		size += strnlen(section_entry->section, MAX_LINE_LENGTH);
		size += 3;	// add '[' and ']' and '\n'
		list_for_each_entry(keyval_entry, &section_entry->keyval_node, node) {
			size += strnlen(keyval_entry->key, MAX_LINE_LENGTH);
			size += strnlen(keyval_entry->value, MAX_LINE_LENGTH);
			size += 2;	// add '=' and '\n'
			if (keyval_entry->update_key || keyval_entry->update_value)
				updated = 1;
		}
	}

	if (!updated) {
		*rawdata_out = (unsigned char *)info->rawdata;
		*size_out = size;
		return 0;
	}

	rawdata_new = (unsigned char *)malloc(size);
	if (!rawdata_new) {
		return -ENOMEM;
	}
	ptr = rawdata_new;

	list_for_each_entry(section_entry, &info->index, section_node) {
		*ptr++ = '[';
		len = strnlen(section_entry->section, MAX_LINE_LENGTH);
		strncpy((char *)ptr, section_entry->section, len);
		ptr += len;
		*ptr++ = ']';
		*ptr++ = '\n';
		list_for_each_entry(keyval_entry, &section_entry->keyval_node, node) {
			len = strnlen(keyval_entry->key, MAX_LINE_LENGTH);
			strncpy((char *)ptr, keyval_entry->key, len);
			ptr += len;
			*ptr++ = '=';
			len = strnlen(keyval_entry->value, MAX_LINE_LENGTH);
			strncpy((char *)ptr, keyval_entry->value, len);
			ptr += len;
			*ptr++ = '\n';
		}
	}
	*rawdata_out = rawdata_new;
	*size_out = size;
	return 0;
}
