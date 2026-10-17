// SPDX-License-Identifier: GPL-2.0+
/*
 * Bootmethod for extlinux boot from a block device
 *
 * Copyright 2021 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#define LOG_CATEGORY UCLASS_BOOTSTD

#include <common.h>
#include <bootdev.h>
#include <bootflow.h>
#include <bootmeth.h>
#include <bootstd.h>
#include <command.h>
#include <dm.h>
#include <extlinux.h>
#include <fs.h>
#include <malloc.h>
#include <mapmem.h>
#include <mmc.h>
#include <pxe_utils.h>

static int extlinux_get_state_desc(struct udevice *dev, char *buf, int maxsize)
{
	if (IS_ENABLED(CONFIG_SANDBOX)) {
		int len;

		len = snprintf(buf, maxsize, "OK");

		return len + 1 < maxsize ? 0 : -ENOSPC;
	}

	return 0;
}

static int extlinux_getfile(struct pxe_context *ctx, const char *file_path,
			    char *file_addr, ulong *sizep)
{
	struct extlinux_info *info = ctx->userdata;
	ulong addr;
	int ret;

	addr = simple_strtoul(file_addr, NULL, 16);

	/* Allow up to 1GB */
	*sizep = 1 << 30;
	ret = bootmeth_read_file(info->dev, info->bflow, file_path, addr,
				 sizep);
	if (ret)
		return log_msg_ret("read", ret);

	return 0;
}

static int extlinux_check(struct udevice *dev, struct bootflow_iter *iter)
{
	int ret;

	/* This only works on block devices */
	ret = bootflow_iter_check_blk(iter);
	if (ret)
		return log_msg_ret("blk", ret);

	return 0;
}

/**
 * extlinux_fill_info() - Decode the extlinux file to find out its info
 *
 * @bflow: Bootflow to process
 * @return 0 if OK, -ve on error
 */
static int extlinux_fill_info(struct bootflow *bflow)
{
	struct membuff mb;
	char line[200];
	char *data;
	int len;

	log_debug("parsing bflow file size %x\n", bflow->size);
	membuff_init(&mb, bflow->buf, bflow->size);
	membuff_putraw(&mb, bflow->size, true, &data);
	while (len = membuff_readline(&mb, line, sizeof(line) - 1, ' ', true), len) {
		char *tok, *p = line;

		tok = strsep(&p, " ");
		if (p) {
			if (!strcmp("label", tok)) {
				bflow->os_name = strdup(p);
				if (!bflow->os_name)
					return log_msg_ret("os", -ENOMEM);
			}
		}
	}

	return 0;
}

static int extlinux_read_bootflow(struct udevice *dev, struct bootflow *bflow)
{
	struct blk_desc *desc;
	const char *const *prefixes;
	struct udevice *bootstd;
	const char *prefix;
	loff_t size;
	int ret, i;

	ret = uclass_first_device_err(UCLASS_BOOTSTD, &bootstd);
	if (ret)
		return log_msg_ret("std", ret);

	/* If a block device, we require a partition table */
	if (bflow->blk && !bflow->part)
		return -ENOENT;

	prefixes = bootstd_get_prefixes(bootstd);
	i = 0;
	desc = bflow->blk ? dev_get_uclass_plat(bflow->blk) : NULL;
	do {
		prefix = prefixes ? prefixes[i] : NULL;

		ret = bootmeth_try_file(bflow, desc, prefix, EXTLINUX_FNAME);
	} while (ret && prefixes && prefixes[++i]);
	if (ret)
		return log_msg_ret("try", ret);
	size = bflow->size;

	ret = bootmeth_alloc_file(bflow, 0x10000, 1);
	if (ret)
		return log_msg_ret("read", ret);

	ret = extlinux_fill_info(bflow);
	if (ret)
		return log_msg_ret("inf", ret);

	return 0;
}

static int extlinux_boot(struct udevice *dev, struct bootflow *bflow)
{
	struct cmd_tbl cmdtp = {};	/* dummy */
	struct pxe_context ctx;
	struct extlinux_info info;
	ulong addr;
	int ret;

	addr = map_to_sysmem(bflow->buf);
	info.dev = dev;
	info.bflow = bflow;
	ret = pxe_setup_ctx(&ctx, &cmdtp, extlinux_getfile, &info, true,
			    bflow->fname, false);
	if (ret)
		return log_msg_ret("ctx", -EINVAL);

	ret = pxe_process(&ctx, addr, false);
	if (ret)
		return log_msg_ret("bread", -EINVAL);

	return 0;
}

static int extlinux_bootmeth_bind(struct udevice *dev)
{
	struct bootmeth_uc_plat *plat = dev_get_uclass_plat(dev);

	plat->desc = IS_ENABLED(CONFIG_BOOTSTD_FULL) ?
		"Extlinux boot from a block device" : "extlinux";

	return 0;
}

static struct bootmeth_ops extlinux_bootmeth_ops = {
	.get_state_desc	= extlinux_get_state_desc,
	.check		= extlinux_check,
	.read_bootflow	= extlinux_read_bootflow,
	.read_file	= bootmeth_common_read_file,
	.boot		= extlinux_boot,
};

static const struct udevice_id extlinux_bootmeth_ids[] = {
	{ .compatible = "u-boot,extlinux" },
	{ }
};

/* Put an number before 'extlinux' to provide a default ordering */
U_BOOT_DRIVER(bootmeth_1extlinux) = {
	.name		= "bootmeth_extlinux",
	.id		= UCLASS_BOOTMETH,
	.of_match	= extlinux_bootmeth_ids,
	.ops		= &extlinux_bootmeth_ops,
	.bind		= extlinux_bootmeth_bind,
};
