// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2020 Linaro Limited
 */

#include <common.h>
#include <dfu.h>
#include <env.h>
#include <memalign.h>
#include <mtd.h>

#define DFU_ALT_BUF_LEN		SZ_1K

static void board_get_alt_info(struct mtd_info *mtd, char *buf)
{
	struct mtd_info *part;
	bool first = true;
	const char *name;
	int len, partnum = 0;

	name = mtd->name;
	len = strlen(buf);

	if (buf[0] != '\0')
		len += snprintf(buf + len, DFU_ALT_BUF_LEN - len, "&");
	len += snprintf(buf + len, DFU_ALT_BUF_LEN - len,
			"mtd %s=", name);

	list_for_each_entry(part, &mtd->partitions, node) {
		partnum++;
		if (!first)
			len += snprintf(buf + len, DFU_ALT_BUF_LEN - len, ";");
		first = false;

		len += snprintf(buf + len, DFU_ALT_BUF_LEN - len,
				"%s part %d",
				part->name, partnum);
	}
}

void set_dfu_alt_info(char *interface, char *devstr)
{
	struct mtd_info *mtd;

	ALLOC_CACHE_ALIGN_BUFFER(char, buf, DFU_ALT_BUF_LEN);

	if (!IS_ENABLED(CONFIG_EFI_HAVE_CAPSULE_SUPPORT) &&
	    env_get("dfu_alt_info"))
		return;

	memset(buf, 0, DFU_ALT_BUF_LEN);

	/*
	 * Currently dfu_alt_info is needed on Qemu ARM64 for
	 * capsule updates
	*/
	if (IS_ENABLED(CONFIG_EFI_CAPSULE_FIRMWARE_MANAGEMENT) &&
	    IS_ENABLED(CONFIG_TARGET_QEMU_ARM_64BIT)) {
		/* probe all MTD devices */
		mtd_probe_devices();

		mtd = get_mtd_device_nm("nor0");
		if (!IS_ERR_OR_NULL(mtd))
			board_get_alt_info(mtd, buf);
	}

	env_set("dfu_alt_info", buf);
	printf("dfu_alt_info set\n");
}
