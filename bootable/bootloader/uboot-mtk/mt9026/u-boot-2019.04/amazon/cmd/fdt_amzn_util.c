/*
 * Copyright (c) 2023 Amazon.com, Inc. or its affiliates.  All rights reserved.
 */

#include <common.h>
#include <amzn_console.h>
#include <fdt.h>
#include <fdt_amzn_util.h>

void ft_uboot_log_setup(void *blob)
{
	struct fdt_header *fdt_ptr = (struct fdt_header *)blob;
	int ret=0, offset=0, err=0;

	char *temp = NULL;
	temp = (char*)(malloc(AMZN_LOG_SIZE+1));
	if (!temp) {
		printf("%s: memory allocation error\n", __FUNCTION__);
		return;
	}
	temp[0]=0;
	amzn_get_log(temp);
	int logsize = strlen(temp);
	if (logsize <= 0)  /* buffer is empty */
		goto done;

        unsigned int newsize = fdt_totalsize(fdt_ptr)
                                + logsize + 32;

	err = fdt_open_into(fdt_ptr, fdt_ptr, newsize);
	if (err != 0) {
		printf ("libfdt fdt_open_into(): %s\n",
			fdt_strerror(err));
	}

	ret = fdt_path_offset(fdt_ptr, "/ubootlog");
	if (ret & FDT_ERR_NOTFOUND) {
		/* Create the ubootlog root node in FDT */
		if ((ret = fdt_path_offset(fdt_ptr, "/")) < 0) {
			printf("%s: unable to find root offset\n", __FUNCTION__);
			goto done;
		}

		if((ret = fdt_add_subnode(fdt_ptr, ret, "ubootlog")) < 0) {
			printf("%s: unable to add ubootlog root node\n", __FUNCTION__);
			goto done;
		}

		/* Get the offset of the new ubootlog root node */
		if ((ret = fdt_path_offset(fdt_ptr, "/ubootlog")) < 0) {
			printf("%s: unable to find ubootlog root node offset\n", __FUNCTION__);
			goto done;
		}
		offset = ret;

		ret = fdt_setprop_string(fdt_ptr, offset, "value", temp);
		if (ret < 0) {
			printf("%s: unable to set ubootlog (value)\n", __FUNCTION__);
			goto done;
		}
	}

done:
	if (temp) {
		free(temp);
	}
	return;
}
