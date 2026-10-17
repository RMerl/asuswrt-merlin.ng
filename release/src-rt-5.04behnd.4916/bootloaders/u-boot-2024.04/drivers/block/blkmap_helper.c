// SPDX-License-Identifier: GPL-2.0+
/*
 * blkmap helper function
 *
 * Copyright (c) 2023, Linaro Limited
 */

#include <blk.h>
#include <blkmap.h>
#include <dm/device.h>
#include <dm/device-internal.h>

int blkmap_create_ramdisk(const char *label, ulong image_addr, ulong image_size,
			  struct udevice **devp)
{
	int ret;
	lbaint_t blknum;
	struct blkmap *bm;
	struct blk_desc *desc;
	struct udevice *bm_dev;

	ret = blkmap_create(label, &bm_dev);
	if (ret) {
		log_err("failed to create blkmap\n");
		return ret;
	}

	bm = dev_get_plat(bm_dev);
	desc = dev_get_uclass_plat(bm->blk);
	blknum = image_size >> desc->log2blksz;
	ret = blkmap_map_pmem(bm_dev, 0, blknum, image_addr);
	if (ret) {
		log_err("Unable to map %#llx at block %d : %d\n",
			(unsigned long long)image_addr, 0, ret);
		goto err;
	}
	log_info("Block %d+0x" LBAF " mapped to %#llx\n", 0, blknum,
		 (unsigned long long)image_addr);

	ret = device_probe(bm->blk);
	if (ret)
		goto err;

	if (devp)
		*devp = bm_dev;

	return 0;

err:
	blkmap_destroy(bm_dev);

	return ret;
}
