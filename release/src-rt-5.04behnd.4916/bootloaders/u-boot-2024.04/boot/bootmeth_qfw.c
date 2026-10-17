// SPDX-License-Identifier: GPL-2.0+
/*
 * Bootmethod for QEMU qfw
 *
 * Copyright 2023 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#define LOG_CATEGORY UCLASS_BOOTSTD

#include <common.h>
#include <command.h>
#include <bootdev.h>
#include <bootflow.h>
#include <bootmeth.h>
#include <env.h>
#include <qfw.h>
#include <dm.h>

static int qfw_check(struct udevice *dev, struct bootflow_iter *iter)
{
	const struct udevice *media = dev_get_parent(iter->dev);
	enum uclass_id id = device_get_uclass_id(media);

	log_debug("media=%s\n", media->name);
	if (id == UCLASS_QFW)
		return 0;

	return -ENOTSUPP;
}

static int qfw_read_bootflow(struct udevice *dev, struct bootflow *bflow)
{
	struct udevice *qfw_dev = dev_get_parent(bflow->dev);
	ulong load, initrd;
	int ret;

	load = env_get_hex("kernel_addr_r", 0);
	initrd = env_get_hex("ramdisk_addr_r", 0);
	log_debug("setup kernel %s %lx %lx\n", qfw_dev->name, load, initrd);
	bflow->name = strdup("qfw");
	if (!bflow->name)
		return log_msg_ret("name", -ENOMEM);

	ret = qemu_fwcfg_setup_kernel(qfw_dev, load, initrd);
	log_debug("setup kernel result %d\n", ret);
	if (ret)
		return log_msg_ret("cmd", -EIO);

	bflow->state = BOOTFLOWST_READY;

	return 0;
}

static int qfw_read_file(struct udevice *dev, struct bootflow *bflow,
			 const char *file_path, ulong addr, ulong *sizep)
{
	return -ENOSYS;
}

static int qfw_boot(struct udevice *dev, struct bootflow *bflow)
{
	int ret;

	ret = run_command("booti ${kernel_addr_r} ${ramdisk_addr_r}:${filesize} ${fdtcontroladdr}",
			  0);
	if (ret) {
		ret = run_command("bootz ${kernel_addr_r} ${ramdisk_addr_r}:${filesize} "
				  "${fdtcontroladdr}", 0);
	}

	return ret ? -EIO : 0;
}

static int qfw_bootmeth_bind(struct udevice *dev)
{
	struct bootmeth_uc_plat *plat = dev_get_uclass_plat(dev);

	plat->desc = "QEMU boot using firmware interface";

	return 0;
}

static struct bootmeth_ops qfw_bootmeth_ops = {
	.check		= qfw_check,
	.read_bootflow	= qfw_read_bootflow,
	.read_file	= qfw_read_file,
	.boot		= qfw_boot,
};

static const struct udevice_id qfw_bootmeth_ids[] = {
	{ .compatible = "u-boot,qfw-extlinux" },
	{ }
};

U_BOOT_DRIVER(bootmeth_qfw) = {
	.name		= "bootmeth_qfw",
	.id		= UCLASS_BOOTMETH,
	.of_match	= qfw_bootmeth_ids,
	.ops		= &qfw_bootmeth_ops,
	.bind		= qfw_bootmeth_bind,
};
