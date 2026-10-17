// SPDX-License-Identifier: GPL-2.0+
/*
 * Bootmethod for booting via a U-Boot script
 *
 * Copyright 2021 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#define LOG_CATEGORY UCLASS_BOOTSTD

#include <common.h>
#include <blk.h>
#include <bootflow.h>
#include <bootmeth.h>
#include <bootstd.h>
#include <dm.h>
#include <env.h>
#include <fs.h>
#include <image.h>
#include <malloc.h>
#include <mapmem.h>
#include <net.h>

#define SCRIPT_FNAME1	"boot.scr.uimg"
#define SCRIPT_FNAME2	"boot.scr"

static int script_check(struct udevice *dev, struct bootflow_iter *iter)
{
	/* This works on block devices, network devices and SPI Flash */
	if (iter->method_flags & BOOTFLOW_METHF_PXE_ONLY)
		return log_msg_ret("pxe", -ENOTSUPP);

	return 0;
}

/**
 * script_fill_info() - Decode the U-Boot script to find out distro info
 *
 * @bflow: Bootflow to process
 * @return 0 if OK, -ve on error
 */
static int script_fill_info(struct bootflow *bflow)
{
	char *name = NULL;
	char *data;
	uint len;
	int ret;

	log_debug("parsing bflow file size %x\n", bflow->size);

	ret = image_locate_script(bflow->buf, bflow->size, NULL, NULL, &data, &len);
	if (!ret) {
		if (strstr(data, "armbianEnv"))
			name = "Armbian";
	}

	if (name) {
		bflow->os_name = strdup(name);
		if (!bflow->os_name)
			return log_msg_ret("os", -ENOMEM);
	}

	return 0;
}

static int script_read_bootflow_file(struct udevice *bootstd,
				     struct bootflow *bflow)
{
	struct blk_desc *desc = NULL;
	const char *const *prefixes;
	const char *prefix;
	int ret, i;

	ret = uclass_first_device_err(UCLASS_BOOTSTD, &bootstd);
	if (ret)
		return log_msg_ret("std", ret);

	if (bflow->blk) {
		/* We require a partition table */
		if (!bflow->part)
			return -ENOENT;
		 desc = dev_get_uclass_plat(bflow->blk);
	}

	prefixes = bootstd_get_prefixes(bootstd);
	i = 0;
	do {
		prefix = prefixes ? prefixes[i] : NULL;

		ret = bootmeth_try_file(bflow, desc, prefix, SCRIPT_FNAME1);
		if (ret)
			ret = bootmeth_try_file(bflow, desc, prefix,
						SCRIPT_FNAME2);
	} while (ret && prefixes && prefixes[++i]);
	if (ret)
		return log_msg_ret("try", ret);

	bflow->subdir = strdup(prefix ? prefix : "");
	if (!bflow->subdir)
		return log_msg_ret("prefix", -ENOMEM);

	ret = bootmeth_alloc_file(bflow, 0x10000, ARCH_DMA_MINALIGN);
	if (ret)
		return log_msg_ret("read", ret);

	ret = script_fill_info(bflow);
	if (ret)
		return log_msg_ret("inf", ret);

	ret = bootmeth_alloc_other(bflow, "boot.bmp", &bflow->logo,
				   &bflow->logo_size);
	/* ignore error */

	return 0;
}

static int script_read_bootflow_net(struct bootflow *bflow)
{
	const char *addr_str;
	int size, ret;
	char *fname;
	ulong addr;

	/* figure out the load address */
	addr_str = env_get("scriptaddr");
	addr = addr_str ? hextoul(addr_str, NULL) : image_load_addr;

	fname = env_get("boot_script_dhcp");
	if (!fname)
		return log_msg_ret("dhc", -EINVAL);

	ret = dhcp_run(addr, fname, true);
	if (ret)
		return log_msg_ret("dhc", ret);

	size = env_get_hex("filesize", 0);
	if (!size)
		return log_msg_ret("sz", -EINVAL);

	bflow->buf = malloc(size + 1);
	if (!bflow->buf)
		return log_msg_ret("buf", -ENOMEM);
	memcpy(bflow->buf, map_sysmem(addr, size), size);
	bflow->buf[size] = '\0';
	bflow->size = size;
	bflow->state = BOOTFLOWST_READY;

	return 0;
}

static int script_read_bootflow(struct udevice *dev, struct bootflow *bflow)
{
	const struct udevice *media = dev_get_parent(bflow->dev);
	struct udevice *bootstd;
	int ret;

	ret = uclass_first_device_err(UCLASS_BOOTSTD, &bootstd);
	if (ret)
		return log_msg_ret("std", ret);

	if (IS_ENABLED(CONFIG_CMD_DHCP) &&
	    device_get_uclass_id(media) == UCLASS_ETH) {
		/* we only support reading from one device, so ignore 'dev' */
		ret = script_read_bootflow_net(bflow);
		if (ret)
			return log_msg_ret("net", ret);
	} else {
		ret = script_read_bootflow_file(bootstd, bflow);
		if (ret)
			return log_msg_ret("blk", ret);
	}

	return 0;
}

static int script_set_bootflow(struct udevice *dev, struct bootflow *bflow,
			       char *buf, int size)
{
	buf[size] = '\0';
	bflow->buf = buf;
	bflow->size = size;
	bflow->state = BOOTFLOWST_READY;

	return 0;
}

static int script_boot(struct udevice *dev, struct bootflow *bflow)
{
	struct blk_desc *desc = dev_get_uclass_plat(bflow->blk);
	ulong addr;
	int ret = 0;

	if (desc->uclass_id == UCLASS_USB) {
		ret = env_set("devtype", "usb");
	} else {
		/* If the uclass is AHCI, but the driver is ATA
		 * (not scsi), set devtype to sata
		 */
		if (IS_ENABLED(CONFIG_SATA) &&
		    desc->uclass_id == UCLASS_AHCI)
			ret = env_set("devtype", "sata");
		else
			ret = env_set("devtype", blk_get_devtype(bflow->blk));
	}
	if (!ret)
		ret = env_set_hex("devnum", desc->devnum);
	if (!ret)
		ret = env_set_hex("distro_bootpart", bflow->part);
	if (!ret)
		ret = env_set("prefix", bflow->subdir);
	if (!ret && IS_ENABLED(CONFIG_ARCH_SUNXI) &&
	    !strcmp("mmc", blk_get_devtype(bflow->blk)))
		ret = env_set_hex("mmc_bootdev", desc->devnum);
	if (ret)
		return log_msg_ret("env", ret);

	log_debug("devtype: %s\n", env_get("devtype"));
	log_debug("devnum: %s\n", env_get("devnum"));
	log_debug("distro_bootpart: %s\n", env_get("distro_bootpart"));
	log_debug("prefix: %s\n", env_get("prefix"));
	log_debug("mmc_bootdev: %s\n", env_get("mmc_bootdev"));

	addr = map_to_sysmem(bflow->buf);
	ret = cmd_source_script(addr, NULL, NULL);
	if (ret)
		return log_msg_ret("boot", ret);

	return 0;
}

static int script_bootmeth_bind(struct udevice *dev)
{
	struct bootmeth_uc_plat *plat = dev_get_uclass_plat(dev);

	plat->desc = IS_ENABLED(CONFIG_BOOTSTD_FULL) ?
		"Script boot from a block device" : "script";

	return 0;
}

static struct bootmeth_ops script_bootmeth_ops = {
	.check		= script_check,
	.read_bootflow	= script_read_bootflow,
	.set_bootflow	= script_set_bootflow,
	.read_file	= bootmeth_common_read_file,
	.boot		= script_boot,
};

static const struct udevice_id script_bootmeth_ids[] = {
	{ .compatible = "u-boot,script" },
	{ }
};

/* Put an number before 'script' to provide a default ordering */
U_BOOT_DRIVER(bootmeth_2script) = {
	.name		= "bootmeth_script",
	.id		= UCLASS_BOOTMETH,
	.of_match	= script_bootmeth_ids,
	.ops		= &script_bootmeth_ops,
	.bind		= script_bootmeth_bind,
};
