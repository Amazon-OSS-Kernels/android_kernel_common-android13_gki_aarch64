// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
*/

#ifndef _INIPARSER_H
#define _INIPARSER_H
#include <stddef.h>
#include <linux/list.h>
#include <stdbool.h>

struct section {
	char *section;				// pointer to section name string
	struct list_head keyval_node;
	struct list_head section_node;
};

struct keyval_pair {
	char *key;					// pointer to key string
	char *value;				// pointer to value string
	bool update_key;			// 1: update, 0: not update
	bool update_value;			// 1: update, 0: not update
	struct list_head node;
};

struct iniparser_info {
	struct list_head index;
	char *rawdata;
};
typedef struct iniparser_info*  iniparser_handle_t;

/**
 * iniparser_create - create ini parser to parse ini
 *
 * @rawdata: Raw data of the ini file
 * @out_handle: Pointer to store the output ini parser handle
 *
 * Returns 0 if index is created with no error and negative otherwise.
 *         This function will create iniparser_info structure for parsing ini.
 *         Whether pass or fail, user should use iniparser_destroy() to
 *         release iniparser_info structure
 */
int iniparser_create(const unsigned char *rawdata, const unsigned int length,
					 iniparser_handle_t *out_handle);

/** 
 * iniparser_destroy - Destroy ini parser
 * @handle: Hanle to the ini parser to be destroyed
 *
 * Returns 0 if destroy ini info with no error
 *         negative otherwise. 
 */
int iniparser_destroy(const iniparser_handle_t handle);

/**
 * iniparser_for_each_section - Iterate each section in the iniparser_handle
 * @iniparser_handle: Hanle to the ini parser
 * @section_entry: Ini section to be operated
 *
 */
#define iniparser_for_each_section(iniparser_handle, section_entry)	\
				list_for_each_entry(section_entry, &iniparser_handle->index, section_node)

/**
 * iniparser_for_each_keyval - Iterate each key value entry in the section
 * @section_entry: Ini section to be operated
 * @keyval_entry: Keyval entry to be operated
 *
 * Usage:
 *
 * unsigned char *filebuf;
 * int filesize, filesize;
 * iniparser_handle_t ini_handle = NULL;
 * struct section *section_entry;
 * struct keyval_pair *keyval_entry;
 *
 * readfile("xxx.ini", &filebuf, &filesize);
 *
 * iniparser_create(filebuf, filesize, INIPARSER_RO, &ini_handle);
 *
 * iniparser_for_each_section(ini_handle, section_entry) {
 * 	   printf("[%s]\n", section_entry->section);
 * 	   iniparser_for_each_keyval(section_entry, keyval_entry) {
 * 	       printf("[%s][%ld]\n", keyval_entry->key, simple_strtoul(keyval_entry->value, NULL, 10));
 * 	   }
 * }
 * iniparser_destroy(ini_handle);
 *
 */
#define iniparser_for_each_keyval(section_entry, keyval_entry)	\
			list_for_each_entry(keyval_entry, &section_entry->keyval_node, node)

/**
 * iniparser_get_section - Get section according to section name
 * @iniparser_handle: Hanle to the ini parser
 * @section_name: Name for the section to be searched
 * @section_entry_out: Pointer to output of found section
 *
 * Returns 1 if found
 *         0 if not found
 *         negative if an error occurs
 */
int iniparser_get_section(const iniparser_handle_t handle, const char *section_name, struct section **section_entry_out);

/**
 * iniparser_get_keyval - Get keyval pair according to key
 * @section_entry: Pointer to section to be searched
 * @key: Key for the section to be searched
 * @keyval_entry_out: Pointer to output of found key value pair
 *
 * Returns 1 if found
 *         0 if not found
 *         negative if an error occurs
 */
int iniparser_get_keyval(const struct section *section_entry, const char *key, struct keyval_pair **keyval_entry_out);

/**
 * iniparser_getint - Get integer value for "key" in "section_entry"
 * @section_entry: Pointer to section to be searched
 * @key: Pointer to the key to be searched
 * @defval: Default value if the key is not found
 * @out: Pointer to the output integer value. If the key is not found, out is set to defval
 *
 * Returns 1 if found
 *         0 if not found
 *         negative if an error occurs
 */
int iniparser_getint(const struct section *section_entry, const char *key, const int defval, int *out);

/**
 * iniparser_getstring - Get string value for "key" in "section_entry"
 * @section_entry: Pointer to section to be searched
 * @key: Pointer to the key to be searched
 * @defval: Default value if the key is not found
 * @out: Pointer to the output string buffer. If the key is not found, out is set to defval
 * @size: Size of the out string buffer
 *
 * Returns 1 if found
 *         0 if not found
 *         negative if an error occurs
 */
int iniparser_getstring(const struct section *section_entry, const char *key, const char *defval, char *out, const int size);

/**
 * iniparser_getbool - Get boolean value for "key" in "section_entry"
 * @section_entry: Pointer to section to be searched
 * @key: Pointer to the key to be searched
 * @defval: Default value if the key is not found
 * @out: Pointer to the output boolean value. If the key is not found, out is set to 
 *       1 if defval is non-zero
 *       0 if defval is zero
 *
 * Returns 1 if found
 *         0 if not found
 *         negative if an error occurs
 */
int iniparser_getbool(const struct section *section_entry, const char *key, const bool defval, bool *out);


/**
 * iniparser_setint - Set integer value for "key" in "section_entry"
 * @section_entry: Pointer to section to be searched
 * @key: Pointer to the key to be searched
 * @val: Value to be set
 *
 * Returns 0 if successful
 *         negative if an error occurs
 */
int iniparser_setint(const struct section *section_entry, const char *key, const int val);

/**
 * iniparser_setstring - Set string value for "key" in "section_entry"
 * @section_entry: Pointer to section to be searched
 * @key: Pointer to the key to be searched
 * @val: Value to be set
 *
 * Returns 0 if successful
 *         negative if an error occurs
 */
int iniparser_setstring(const struct section *section_entry, const char *key, const char *val);

/**
 * iniparser_setbool - Set boolean value for "key" in "section_entry"
 * @section_entry: Pointer to section to be searched
 * @key: Pointer to the key to be searched
 * @val: Value to be set
 *
 * Returns 0 if successful
 *         negative if an error occurs
 */
int iniparser_setbool(const struct section *section_entry, const char *key, const bool val);

/**
 * iniparser_set_value - Set new value string to some keyval pair
 * @keyval_entry: Pointer to the key value pair to be set
 * @newkey: String to the new value. The key is not set if newkey is NULL
 * @newval: String to the new value. The value is not set if newval is NULL
 *
 * Returns 0 if set scuccessfully
 *         negative if an error occurs
 */
int iniparser_set_keyval(struct keyval_pair *keyval_entry, const char *newkey, const char *newval);

/**
 * iniparser_create_dumpdata - create ini raw data for dumping
 * @handle: Hanle to the ini parser to be packed
 * @rawdata_out: Pointer to store output of the raw data araray
 * @size_out: Pointer to store output of the length of rawdata
 *
 * Returns 0 if update scuccessfully
 *         negative if an error occurs
 *
 * This function is used to create ini raw data for saving it to the ini file.
 * If this function return successfully, one should free rawdata_out buffer
 * if it is not used anymore
 */
int iniparser_create_dumpdata(const iniparser_handle_t handle, unsigned char **rawdata_out, unsigned int *size_out);

/**
 * iniparser_unescape_string - unescape ini value string
 * @dest[out]: destination buffer
 * @src[src]: source value string
 * @size[in]: size of the destination buffer
 *
 * Returns positive copied chars if update scuccessfully
 *         zero or negative if an error occurs
 */
int iniparser_unescape_string(char *dest, const char *src, const int size);
#endif
