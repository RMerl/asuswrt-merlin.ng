// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2020 Linaro Limited
 */

#include <common.h>
#include <dm.h>
#include <mtd.h>

#include <linux/string.h>

#define MTDPARTS_LEN		256
#define MTDIDS_LEN		128

static void board_get_mtdparts(const char *dev, const char *partition,
			       char *mtdids, char *mtdparts)
{
	/* mtdids: "<dev>=<dev>, ...." */
	if (mtdids[0] != '\0')
		strcat(mtdids, ",");
	strcat(mtdids, dev);
	strcat(mtdids, "=");
	strcat(mtdids, dev);

	/* mtdparts: "mtdparts=<dev>:<mtdparts_<dev>>;..." */
	if (mtdparts[0] != '\0')
		strncat(mtdparts, ";", MTDPARTS_LEN);
	else
		strcat(mtdparts, "mtdparts=");

	strncat(mtdparts, dev, MTDPARTS_LEN);
	strncat(mtdparts, ":", MTDPARTS_LEN);
	strncat(mtdparts, partition, MTDPARTS_LEN);
}

void board_mtdparts_default(const char **mtdids, const char **mtdparts)
{
	struct mtd_info *mtd;
	struct udevice *dev;
	const char *mtd_partition;
	static char parts[3 * MTDPARTS_LEN + 1];
	static char ids[MTDIDS_LEN + 1];
	static bool mtd_initialized;

	if (mtd_initialized) {
		*mtdids = ids;
		*mtdparts = parts;
		return;
	}

	memset(parts, 0, sizeof(parts));
	memset(ids, 0, sizeof(ids));

	/* Currently mtdparts is needed on Qemu ARM64 for capsule updates */
	if (IS_ENABLED(CONFIG_EFI_CAPSULE_FIRMWARE_MANAGEMENT) &&
	    IS_ENABLED(CONFIG_TARGET_QEMU_ARM_64BIT)) {
		/* probe all MTD devices */
		for (uclass_first_device(UCLASS_MTD, &dev); dev;
		     uclass_next_device(&dev)) {
			debug("mtd device = %s\n", dev->name);
		}

		mtd = get_mtd_device_nm("nor0");
		if (!IS_ERR_OR_NULL(mtd)) {
			mtd_partition = CONFIG_MTDPARTS_NOR0;
			board_get_mtdparts("nor0", mtd_partition, ids, parts);
			put_mtd_device(mtd);
		}

		mtd = get_mtd_device_nm("nor1");
		if (!IS_ERR_OR_NULL(mtd)) {
			mtd_partition = CONFIG_MTDPARTS_NOR1;
			board_get_mtdparts("nor1", mtd_partition, ids, parts);
			put_mtd_device(mtd);
		}
	}

	mtd_initialized = true;
	*mtdids = ids;
	*mtdparts = parts;
	debug("%s:mtdids=%s & mtdparts=%s\n", __func__, ids, parts);
}
