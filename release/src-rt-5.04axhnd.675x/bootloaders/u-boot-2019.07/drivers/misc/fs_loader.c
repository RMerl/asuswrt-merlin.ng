// SPDX-License-Identifier: GPL-2.0
 /*
 * Copyright (C) 2018-2019 Intel Corporation <www.intel.com>
 *
 */
#include <common.h>
#include <dm.h>
#include <errno.h>
#include <blk.h>
#include <fs.h>
#include <fs_loader.h>
#include <linux/string.h>
#include <mapmem.h>
#include <malloc.h>
#include <spl.h>

DECLARE_GLOBAL_DATA_PTR;

/**
 * struct firmware - A place for storing firmware and its attribute data.
 *
 * This holds information about a firmware and its content.
 *
 * @size: Size of a file
 * @data: Buffer for file
 * @priv: Firmware loader private fields
 * @name: Filename
 * @offset: Offset of reading a file
 */
struct firmware {
	size_t size;
	const u8 *data;
	const char *name;
	u32 offset;
};

#ifdef CONFIG_CMD_UBIFS
static int mount_ubifs(char *mtdpart, char *ubivol)
{
	int ret = ubi_part(mtdpart, NULL);

	if (ret) {
		debug("Cannot find mtd partition %s\n", mtdpart);
		return ret;
	}

	return cmd_ubifs_mount(ubivol);
}

static int umount_ubifs(void)
{
	return cmd_ubifs_umount();
}
#else
static int mount_ubifs(char *mtdpart, char *ubivol)
{
	debug("Error: Cannot load image: no UBIFS support\n");
	return -ENOSYS;
}
#endif

static int select_fs_dev(struct device_platdata *plat)
{
	int ret;

	if (plat->phandlepart.phandle) {
		ofnode node;

		node = ofnode_get_by_phandle(plat->phandlepart.phandle);

		struct udevice *dev;

		ret = device_get_global_by_ofnode(node, &dev);
		if (!ret) {
			struct blk_desc *desc = blk_get_by_device(dev);
			if (desc) {
				ret = fs_set_blk_dev_with_part(desc,
					plat->phandlepart.partition);
			} else {
				debug("%s: No device found\n", __func__);
				return -ENODEV;
			}
		}
	} else if (plat->mtdpart && plat->ubivol) {
		ret = mount_ubifs(plat->mtdpart, plat->ubivol);
		if (ret)
			return ret;

		ret = fs_set_blk_dev("ubi", NULL, FS_TYPE_UBIFS);
	} else {
		debug("Error: unsupported storage device.\n");
		return -ENODEV;
	}

	if (ret)
		debug("Error: could not access storage.\n");

	return ret;
}

/**
 * _request_firmware_prepare - Prepare firmware struct.
 *
 * @dev: An instance of a driver.
 * @name: Name of firmware file.
 * @dbuf: Address of buffer to load firmware into.
 * @size: Size of buffer.
 * @offset: Offset of a file for start reading into buffer.
 *
 * Return: Negative value if fail, 0 for successful.
 */
static int _request_firmware_prepare(struct udevice *dev,
				    const char *name, void *dbuf,
				    size_t size, u32 offset)
{
	if (!name || name[0] == '\0')
		return -EINVAL;

	struct firmware *firmwarep = dev_get_priv(dev);

	if (!firmwarep)
		return -ENOMEM;

	firmwarep->name = name;
	firmwarep->offset = offset;
	firmwarep->data = dbuf;
	firmwarep->size = size;

	return 0;
}

/**
 * fw_get_filesystem_firmware - load firmware into an allocated buffer.
 * @dev: An instance of a driver.
 *
 * Return: Size of total read, negative value when error.
 */
static int fw_get_filesystem_firmware(struct udevice *dev)
{
	loff_t actread;
	char *storage_interface, *dev_part, *ubi_mtdpart, *ubi_volume;
	int ret;

	storage_interface = env_get("storage_interface");
	dev_part = env_get("fw_dev_part");
	ubi_mtdpart = env_get("fw_ubi_mtdpart");
	ubi_volume = env_get("fw_ubi_volume");

	if (storage_interface && dev_part) {
		ret = fs_set_blk_dev(storage_interface, dev_part, FS_TYPE_ANY);
	} else if (storage_interface && ubi_mtdpart && ubi_volume) {
		ret = mount_ubifs(ubi_mtdpart, ubi_volume);
		if (ret)
			return ret;

		if (!strcmp("ubi", storage_interface))
			ret = fs_set_blk_dev(storage_interface, NULL,
				FS_TYPE_UBIFS);
		else
			ret = -ENODEV;
	} else {
		ret = select_fs_dev(dev->platdata);
	}

	if (ret)
		goto out;

	struct firmware *firmwarep = dev_get_priv(dev);

	if (!firmwarep)
		return -ENOMEM;

	ret = fs_read(firmwarep->name, (ulong)map_to_sysmem(firmwarep->data),
			firmwarep->offset, firmwarep->size, &actread);

	if (ret) {
		debug("Error: %d Failed to read %s from flash %lld != %zu.\n",
		      ret, firmwarep->name, actread, firmwarep->size);
	} else {
		ret = actread;
	}

out:
#ifdef CONFIG_CMD_UBIFS
	umount_ubifs();
#endif
	return ret;
}

/**
 * request_firmware_into_buf - Load firmware into a previously allocated buffer.
 * @dev: An instance of a driver.
 * @name: Name of firmware file.
 * @buf: Address of buffer to load firmware into.
 * @size: Size of buffer.
 * @offset: Offset of a file for start reading into buffer.
 *
 * The firmware is loaded directly into the buffer pointed to by @buf.
 *
 * Return: Size of total read, negative value when error.
 */
int request_firmware_into_buf(struct udevice *dev,
			      const char *name,
			      void *buf, size_t size, u32 offset)
{
	int ret;

	if (!dev)
		return -EINVAL;

	ret = _request_firmware_prepare(dev, name, buf, size, offset);
	if (ret < 0) /* error */
		return ret;

	ret = fw_get_filesystem_firmware(dev);

	return ret;
}

static int fs_loader_ofdata_to_platdata(struct udevice *dev)
{
	u32 phandlepart[2];

	ofnode fs_loader_node = dev_ofnode(dev);

	if (ofnode_valid(fs_loader_node)) {
		struct device_platdata *plat;

		plat = dev->platdata;
		if (!ofnode_read_u32_array(fs_loader_node,
					  "phandlepart",
					  phandlepart, 2)) {
			plat->phandlepart.phandle = phandlepart[0];
			plat->phandlepart.partition = phandlepart[1];
		}

		plat->mtdpart = (char *)ofnode_read_string(
				 fs_loader_node, "mtdpart");

		plat->ubivol = (char *)ofnode_read_string(
				 fs_loader_node, "ubivol");
	}

	return 0;
}

static int fs_loader_probe(struct udevice *dev)
{
#if CONFIG_IS_ENABLED(DM) && CONFIG_IS_ENABLED(BLK)
	int ret;
	struct device_platdata *plat = dev->platdata;

	if (plat->phandlepart.phandle) {
		ofnode node = ofnode_get_by_phandle(plat->phandlepart.phandle);
		struct udevice *parent_dev = NULL;

		ret = device_get_global_by_ofnode(node, &parent_dev);
		if (!ret) {
			struct udevice *dev;

			ret = blk_get_from_parent(parent_dev, &dev);
			if (ret) {
				debug("fs_loader: No block device: %d\n",
					ret);

				return ret;
			}
		}
	}
#endif

	return 0;
};

static const struct udevice_id fs_loader_ids[] = {
	{ .compatible = "u-boot,fs-loader"},
	{ }
};

U_BOOT_DRIVER(fs_loader) = {
	.name			= "fs-loader",
	.id			= UCLASS_FS_FIRMWARE_LOADER,
	.of_match		= fs_loader_ids,
	.probe			= fs_loader_probe,
	.ofdata_to_platdata	= fs_loader_ofdata_to_platdata,
	.platdata_auto_alloc_size	= sizeof(struct device_platdata),
	.priv_auto_alloc_size	= sizeof(struct firmware),
};

UCLASS_DRIVER(fs_loader) = {
	.id		= UCLASS_FS_FIRMWARE_LOADER,
	.name		= "fs-loader",
};
