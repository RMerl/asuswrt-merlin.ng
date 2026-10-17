// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2023, Linaro Limited
 */

#include <efi_loader.h>
#include <fwu.h>
#include <fwu_mdata.h>
#include <memalign.h>
#include <mtd.h>

#define DFU_ALT_BUF_LEN 256

/* Generate dfu_alt_info from partitions */
void set_dfu_alt_info(char *interface, char *devstr)
{
	ALLOC_CACHE_ALIGN_BUFFER(char, buf, DFU_ALT_BUF_LEN);
	struct mtd_info *mtd;
	int ret;

	memset(buf, 0, sizeof(buf));

	mtd_probe_devices();

	mtd = get_mtd_device_nm("nor1");
	if (IS_ERR_OR_NULL(mtd))
		return;

	ret = fwu_gen_alt_info_from_mtd(buf, DFU_ALT_BUF_LEN, mtd);
	if (ret < 0) {
		log_err("Error: Failed to generate dfu_alt_info. (%d)\n", ret);
		return;
	}
	log_debug("Make dfu_alt_info: '%s'\n", buf);

	env_set("dfu_alt_info", buf);
}
