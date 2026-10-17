// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright (C) 2020, STMicroelectronics - All Rights Reserved
 */

#include <common.h>
#include <blk.h>
#include <dm.h>
#include <dfu.h>
#include <env.h>
#include <log.h>
#include <memalign.h>
#include <misc.h>
#include <mtd.h>
#include <mtd_node.h>
#include <asm/arch/stm32prog.h>
#include <linux/printk.h>

#define DFU_ALT_BUF_LEN SZ_1K

static void board_get_alt_info_mmc(struct udevice *dev, char *buf)
{
	struct disk_partition info;
	int p, len, devnum;
	bool first = true;
	const char *name;
	struct mmc *mmc;
	struct blk_desc *desc;

	mmc = mmc_get_mmc_dev(dev);
	if (!mmc)
		return;

	if (mmc_init(mmc))
		return;

	desc = mmc_get_blk_desc(mmc);
	if (!desc)
		return;

	name = blk_get_uclass_name(desc->uclass_id);
	devnum = desc->devnum;
	len = strlen(buf);

	if (buf[0] != '\0')
		len += snprintf(buf + len,
				DFU_ALT_BUF_LEN - len, "&");
	len += snprintf(buf + len, DFU_ALT_BUF_LEN - len,
			 "%s %d=", name, devnum);

	if (IS_MMC(mmc) && mmc->capacity_boot) {
		len += snprintf(buf + len, DFU_ALT_BUF_LEN - len,
				"%s%d_boot1 raw 0x0 0x%llx mmcpart 1;",
				name, devnum, mmc->capacity_boot);
		len += snprintf(buf + len, DFU_ALT_BUF_LEN - len,
				"%s%d_boot2 raw 0x0 0x%llx mmcpart 2",
				name, devnum, mmc->capacity_boot);
		first = false;
	}

	for (p = 1; p <= MAX_SEARCH_PARTITIONS; p++) {
		if (part_get_info(desc, p, &info))
			continue;
		if (!first)
			len += snprintf(buf + len, DFU_ALT_BUF_LEN - len, ";");
		first = false;
		len += snprintf(buf + len, DFU_ALT_BUF_LEN - len,
				"%s%d_%s part %d %d",
				name, devnum, info.name, devnum, p);
	}
}

static void board_get_alt_info_mtd(struct mtd_info *mtd, char *buf)
{
	struct mtd_info *part;
	const char *name;
	int len, partnum = 0;

	name = mtd->name;
	len = strlen(buf);

	if (buf[0] != '\0')
		len += snprintf(buf + len, DFU_ALT_BUF_LEN - len, "&");
	len += snprintf(buf + len, DFU_ALT_BUF_LEN - len,
			"mtd %s=", name);

	len += snprintf(buf + len, DFU_ALT_BUF_LEN - len,
			"%s raw 0x0 0x%llx",
			name, mtd->size);

	list_for_each_entry(part, &mtd->partitions, node) {
		partnum++;
		len += snprintf(buf + len, DFU_ALT_BUF_LEN - len,
				";%s_%s part %d",
				name, part->name, partnum);
	}
}

void set_dfu_alt_info(char *interface, char *devstr)
{
	struct udevice *dev;
	struct mtd_info *mtd;

	ALLOC_CACHE_ALIGN_BUFFER(char, buf, DFU_ALT_BUF_LEN);

	if (env_get("dfu_alt_info"))
		return;

	memset(buf, 0, sizeof(buf));

	snprintf(buf, DFU_ALT_BUF_LEN,
		 "ram 0=%s", CONFIG_DFU_ALT_RAM0);

	if (CONFIG_IS_ENABLED(MMC)) {
		if (!uclass_get_device(UCLASS_MMC, 0, &dev))
			board_get_alt_info_mmc(dev, buf);

		if (!uclass_get_device(UCLASS_MMC, 1, &dev))
			board_get_alt_info_mmc(dev, buf);
	}

	if (IS_ENABLED(CONFIG_MTD)) {
		/* probe all MTD devices */
		mtd_probe_devices();

		mtd_for_each_device(mtd)
			if (!mtd_is_partition(mtd))
				board_get_alt_info_mtd(mtd, buf);
	}

	if (IS_ENABLED(CONFIG_DFU_VIRT)) {
		/* virtual device id 0 is aligned with stm32mp_dfu_virt.c */
		strlcat(buf, "&virt 0=OTP", DFU_ALT_BUF_LEN);

		if (IS_ENABLED(CONFIG_PMIC_STPMIC1))
			strlcat(buf, "&virt 1=PMIC", DFU_ALT_BUF_LEN);
	}

	env_set("dfu_alt_info", buf);
	puts("DFU alt info setting: done\n");
}
