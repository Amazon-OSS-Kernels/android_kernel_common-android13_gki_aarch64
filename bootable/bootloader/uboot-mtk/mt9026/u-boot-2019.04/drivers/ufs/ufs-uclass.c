#include <common.h>
#include <dm.h>
#include "ufshcd.h"
#include <ufs_scsi.h>

UCLASS_DRIVER(ufs) = {
	.id		= UCLASS_UFS,
	.name		= "ufs",
	.flags		= DM_UC_FLAG_SEQ_ALIAS,
	.per_device_platdata_auto_alloc_size = sizeof(struct ufs_hba),
};
