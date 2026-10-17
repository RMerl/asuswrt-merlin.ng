// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2022, Linaro Limited
 */

#define LOG_CATEGORY UCLASS_FWU_MDATA

#include <blk.h>
#include <dm.h>
#include <efi_loader.h>
#include <fwu.h>
#include <fwu_mdata.h>
#include <log.h>
#include <memalign.h>
#include <part.h>
#include <part_efi.h>

#include <dm/device-internal.h>
#include <linux/errno.h>
#include <linux/types.h>

enum {
	MDATA_READ = 1,
	MDATA_WRITE,
};

static uint g_mdata_part[2]; /* = {0, 0} to check against uninit parts */

static int gpt_get_mdata_partitions(struct blk_desc *desc)
{
	int i;
	u32 nparts;
	efi_guid_t part_type_guid;
	struct disk_partition info;
	const efi_guid_t fwu_mdata_guid = FWU_MDATA_GUID;

	/* if primary and secondary partitions already found */
	if (g_mdata_part[0] && g_mdata_part[1])
		return 0;

	nparts = 0;
	for (i = 1; i < MAX_SEARCH_PARTITIONS && nparts < 2; i++) {
		if (part_get_info(desc, i, &info))
			continue;
		uuid_str_to_bin(info.type_guid, part_type_guid.b,
				UUID_STR_FORMAT_GUID);

		if (!guidcmp(&fwu_mdata_guid, &part_type_guid))
			g_mdata_part[nparts++] = i;
	}

	if (nparts != 2) {
		log_debug("Expect two copies of the FWU metadata instead of %d\n",
			  nparts);
		g_mdata_part[0] = 0;
		g_mdata_part[1] = 0;
		return -EINVAL;
	}

	return 0;
}

static int gpt_get_mdata_disk_part(struct blk_desc *desc,
				   struct disk_partition *info,
				   u32 part_num)
{
	int ret;
	char *mdata_guid_str = "8a7a84a0-8387-40f6-ab41-a8b9a5a60d23";

	ret = part_get_info(desc, part_num, info);
	if (ret < 0) {
		log_debug("Unable to get the partition info for the FWU metadata part %d\n",
			  part_num);
		return -ENOENT;
	}

	/* Check that it is indeed the FWU metadata partition */
	if (!strncmp(info->type_guid, mdata_guid_str, UUID_STR_LEN))
		return 0;

	return -ENOENT;
}

static int gpt_read_write_mdata(struct blk_desc *desc,
				struct fwu_mdata *mdata,
				u8 access, u32 part_num)
{
	int ret;
	u32 len, blk_start, blkcnt;
	struct disk_partition info;

	ALLOC_CACHE_ALIGN_BUFFER_PAD(struct fwu_mdata, mdata_aligned, 1,
				     desc->blksz);

	if (!mdata)
		return -ENOMEM;

	ret = gpt_get_mdata_disk_part(desc, &info, part_num);
	if (ret < 0) {
		printf("Unable to get the FWU metadata partition\n");
		return -ENOENT;
	}

	len = sizeof(*mdata);
	blkcnt = BLOCK_CNT(len, desc);
	if (blkcnt > info.size) {
		log_debug("Block count exceeds FWU metadata partition size\n");
		return -ERANGE;
	}

	blk_start = info.start;
	if (access == MDATA_READ) {
		if (blk_dread(desc, blk_start, blkcnt, mdata_aligned) != blkcnt) {
			log_debug("Error reading FWU metadata from the device\n");
			return -EIO;
		}
		memcpy(mdata, mdata_aligned, sizeof(struct fwu_mdata));
	} else {
		if (blk_dwrite(desc, blk_start, blkcnt, mdata) != blkcnt) {
			log_debug("Error writing FWU metadata to the device\n");
			return -EIO;
		}
	}

	return 0;
}

static int fwu_get_mdata_device(struct udevice *dev, struct udevice **mdata_dev)
{
	u32 phandle;
	int ret, size;
	struct udevice *parent;
	const fdt32_t *phandle_p = NULL;

	phandle_p = dev_read_prop(dev, "fwu-mdata-store", &size);
	if (!phandle_p) {
		log_debug("fwu-mdata-store property not found\n");
		return -ENOENT;
	}

	phandle = fdt32_to_cpu(*phandle_p);

	ret = device_get_global_by_ofnode(ofnode_get_by_phandle(phandle),
					  &parent);
	if (ret)
		return ret;

	return blk_get_from_parent(parent, mdata_dev);
}

static int fwu_mdata_gpt_blk_probe(struct udevice *dev)
{
	int ret;
	struct udevice *mdata_dev = NULL;
	struct fwu_mdata_gpt_blk_priv *priv = dev_get_priv(dev);

	ret = fwu_get_mdata_device(dev, &mdata_dev);
	if (ret)
		return ret;

	priv->blk_dev = mdata_dev;

	return 0;
}

static int fwu_gpt_read_mdata(struct udevice *dev, struct fwu_mdata *mdata,
			      bool primary)
{
	struct fwu_mdata_gpt_blk_priv *priv = dev_get_priv(dev);
	struct blk_desc *desc = dev_get_uclass_plat(priv->blk_dev);
	int ret;

	ret = gpt_get_mdata_partitions(desc);
	if (ret < 0) {
		log_debug("Error getting the FWU metadata partitions\n");
		return -ENOENT;
	}

	return gpt_read_write_mdata(desc, mdata, MDATA_READ,
				    primary ? g_mdata_part[0] : g_mdata_part[1]);
}

static int fwu_gpt_write_mdata(struct udevice *dev, struct fwu_mdata *mdata,
			       bool primary)
{
	struct fwu_mdata_gpt_blk_priv *priv = dev_get_priv(dev);
	struct blk_desc *desc = dev_get_uclass_plat(priv->blk_dev);
	int ret;

	ret = gpt_get_mdata_partitions(desc);
	if (ret < 0) {
		log_debug("Error getting the FWU metadata partitions\n");
		return -ENOENT;
	}

	return gpt_read_write_mdata(desc, mdata, MDATA_WRITE,
				    primary ? g_mdata_part[0] : g_mdata_part[1]);
}

static const struct fwu_mdata_ops fwu_gpt_blk_ops = {
	.read_mdata = fwu_gpt_read_mdata,
	.write_mdata = fwu_gpt_write_mdata,
};

static const struct udevice_id fwu_mdata_ids[] = {
	{ .compatible = "u-boot,fwu-mdata-gpt" },
	{ }
};

U_BOOT_DRIVER(fwu_mdata_gpt_blk) = {
	.name		= "fwu-mdata-gpt-blk",
	.id		= UCLASS_FWU_MDATA,
	.of_match	= fwu_mdata_ids,
	.ops		= &fwu_gpt_blk_ops,
	.probe		= fwu_mdata_gpt_blk_probe,
	.priv_auto	= sizeof(struct fwu_mdata_gpt_blk_priv),
};
