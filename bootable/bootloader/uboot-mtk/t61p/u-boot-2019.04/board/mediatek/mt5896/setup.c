#include <common.h>
#include <command.h>
#include <fdt.h>
#include <libfdt.h>
#include <idme.h>

#include <MsTypes.h>
#include <MsDebug.h>
#include <fat.h>
#include <string.h>
#include <amzn_tv_common.h>
#include <amzn_tv_secure_boot.h>

#define AMZN_MT9025_CID 0x40
#define BASE_REG_ADDR   0x1C000000
#define CID_REG_ADDR	0x00020306 /* 8bits mode */
#define EFUST_REG_ADDR	0x00012908 /* 8bits mode */
#define RIU_REG(addr) \
	((void __iomem *)(BASE_REG_ADDR) + (((addr) << 1) - ((addr) & 1)))
#define readb(addr) \
	({ unsigned char __v = (*(volatile unsigned char *)(addr)); __v; })

#if defined(CONFIG_OF_BOARD_SETUP)
int ft_board_setup(void *blob, bd_t *bd)
{
	struct fdt_header *fdt_ptr = (struct fdt_header *)blob;

	unsigned int newsize = fdt_totalsize(fdt_ptr)
				+ CONFIG_IDME_SIZE;
	fdt_open_into(fdt_ptr, fdt_ptr, newsize);
	idme_device_tree_initialize(fdt_ptr);
	printf("IDME inserted into FDT\n");
	return 0;
}
#endif

MS_BOOL read_eFuse_authenciation(void)
{
	#define EFUSE_AUTHENTICATION_BIT 0x02
	/* Bank 0x129 - 0x04[1] (16bit mode) eFuse authenciation enable */
	unsigned char val = readb(RIU_REG(EFUST_REG_ADDR));
	if (val & EFUSE_AUTHENTICATION_BIT)
		return TRUE;

	return FALSE;
}


unsigned char read_board_cid(void)
{
	/* read Customer ID */
	unsigned char val = readb(RIU_REG(CID_REG_ADDR));
	printf("%s: CID:0x%X\n", __func__, val);
	return val;
}

MS_BOOL is_secure_cpu(void)
{
	if (read_eFuse_authenciation()
		&& (read_board_cid() == AMZN_MT9025_CID))
			return TRUE;

	return FALSE;
}

#define SBOOT_VER_MARK "YAMA"
#define MAX_SBOOT_SIZE 536576 /* 524K */
#define VER_MARK_LEN   4
#define VER_LEN        2
#define PROD_LEN       4

sbvc_result sboot_version_check(char *sboot_buf, int sboot_len, int partition_num)
{
	int mark_loc = 0;
	int prod_match = 0;
	/* const version_bump: sboot version bumped can be different per device */
	/* It should be updated whenever there's version bump for anti-rollback */
#if defined(SBOOT_abc123_BOARD)
	const char *device_name = "abc123";
	const unsigned short version_bump = 0x13;
#elif defined(SBOOT_abc123_BOARD)
	const char *device_name = "abc123";
	const unsigned short version_bump = 0x13;
#elif defined(SBOOT_abc123_BOARD)
	const char *device_name = "abc123";
	const unsigned short version_bump = 0x13;
#elif defined(SBOOT_COYOTE_BOARD)
	const char *device_name = "COYOTE";
	const unsigned short version_bump = 0x19;
#elif defined(SBOOT_COYOTEEU_BOARD)
        const char *device_name = "COEU";
        const unsigned short version_bump = 0x17;
#elif defined(SBOOT_abc123_BOARD)
	const char *device_name = "abc123";
	const unsigned short version_bump = 0;
#elif defined(SBOOT_abc123_BOARD)
	const char *device_name = "abc123";
	const unsigned short version_bump = 0;
#elif defined(SBOOT_abc123_BOARD)
	const char *device_name = "abc123";
	const unsigned short version_bump = 0x11;
#elif defined(SBOOT_abc123EU_BOARD)
	const char *device_name = "GREU";
	const unsigned short version_bump = 0x12;
#else
	const char *device_name = "SANFORD";
	const unsigned short version_bump = 0;
#endif

	// don't check sboot for non-secure device and T61P based devices
#if (!defined(SBOOT_abc123_BOARD) && !defined(SBOOT_abc123_BOARD))
	if (is_secure_cpu() == FALSE)
#endif
		return SBVC_MATCH;

	if (sboot_buf == NULL)
		return SBVC_INVALID_ARG;

	// check sboot version mark
	while ( (mark_loc <= sboot_len - VER_MARK_LEN - VER_LEN - PROD_LEN) &&
		strncmp(sboot_buf+mark_loc, SBOOT_VER_MARK, VER_MARK_LEN) ) {
		mark_loc++;
	}

	if (mark_loc > sboot_len - VER_MARK_LEN - VER_LEN - PROD_LEN) {
		return SBVC_MARK_NOT_FOUND;
	}

	prod_match = !strncmp(sboot_buf+mark_loc+VER_MARK_LEN+VER_LEN, device_name, PROD_LEN);
	if (!prod_match) {
		printf("\n!!Device:%s, sboot:%.*s!!\n", device_name, PROD_LEN,
			sboot_buf+mark_loc+VER_MARK_LEN+VER_LEN);
		return SBVC_MARK_NOT_FOUND;
	}

	char *sboot_dev = NULL;
	// allocate memory for sboot image on the device
	sboot_dev = calloc(1, MAX_SBOOT_SIZE);
	if (!sboot_dev) {
		return SBVC_NO_MEM;
	}

	char mmc_command[64] = {0};
	int sboot_len_dev = (sboot_len < MAX_SBOOT_SIZE) ? sboot_len : MAX_SBOOT_SIZE;
	snprintf(mmc_command, 64, "partition read.boot mmc 0 %d 0x%p, 0x%08x",
		partition_num, sboot_dev, sboot_len_dev);
	printf("\n%s\n", mmc_command);
	if (run_command(mmc_command, 0) < 0) {
		free(sboot_dev);
		return SBVC_READ_ERR;
	}

	/* find the version mark */
	int mark_loc_dev = 0;
	while ( (mark_loc_dev <= sboot_len_dev - VER_MARK_LEN - VER_LEN - PROD_LEN) &&
		strncmp(sboot_dev+mark_loc_dev, SBOOT_VER_MARK, VER_MARK_LEN) ) {
		mark_loc_dev++;
	}

	if (mark_loc_dev > sboot_len_dev - VER_MARK_LEN - VER_LEN - PROD_LEN) {
		free(sboot_dev);
		return SBVC_DEV_MARK_NOT_FOUND;
	}

	unsigned short sboot_ver, sboot_ver_dev;
	sboot_ver = *(unsigned short*)(sboot_buf+mark_loc+VER_MARK_LEN);
	sboot_ver_dev = *(unsigned short*)(sboot_dev+mark_loc_dev+VER_MARK_LEN);

	printf("\n%s: sboot img version: %d, dev version: %d\n",
		__FUNCTION__, sboot_ver, sboot_ver_dev);
	if (sboot_ver == sboot_ver_dev) {
		free(sboot_dev);
		return SBVC_SAME_VER;
	} else {
		/* As sboot version in device and flash package are different, required to */
		/* check if it's rollback case since anti-rollback version bump per device */
		printf("\n%s: sboot version_bump on %s: %d\n", __FUNCTION__, device_name, version_bump);
		if ((version_bump != 0) && (sboot_ver_dev >= version_bump) && (sboot_ver < version_bump )) {
			free(sboot_dev);
			return SBVC_ROLLBACK;
		}
	}

	free(sboot_dev);

	return SBVC_MATCH;
}


#if defined(UFBL_FEATURE_IDME)
/*
	Function idme_get_oem_data_field
	This function finds individual item value from whole
	oem_string and return value found by reference.

	Input Parameters
	item: char string in the format of "item_name=" to get
		the value at right side of '=' separated by ':'
        or terminated by NULL
		ex) oem_data="item_a=value_a:item_b=value_b:item_c=value_c"
	buf: data buffer to copy the item value found as a string
		with NULL termination
	buf_len: the size of char* buf
*/
void idme_get_oem_data_field(const char *item, char *buf, unsigned buf_len)
{
	#define MAX_OEM_DATA 1024
	unsigned data_len = 0;
	char *i_begin = NULL, *i_end = NULL, oem_data[MAX_OEM_DATA] = { 0x00, };

	if ((item==NULL) || (buf==NULL)) {
		printf("no item or buf allocated to read oem_data");
		return;
	}
	idme_get_var_external("oem_data", oem_data, (sizeof(oem_data) - 1));

	i_begin = strstr(oem_data, item);
	if (i_begin == NULL) {
		/* printf("item(%s) not found in oem_data", item); */
		return;
	}

	i_end=strchr(i_begin, ':');
	if (i_end == NULL)
		/* this is the last item without separator, : */
		data_len = strlen(i_begin) - strlen(item);
	else
		data_len = i_end - i_begin - strlen(item);

	snprintf(buf, (data_len > buf_len-1) ? buf_len:data_len+1, "%s", i_begin + strlen(item) );
	return;
}
#endif
