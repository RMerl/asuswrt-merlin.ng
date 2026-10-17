// SPDX-License-Identifier: GPL-2.0+
/*
 * Bootmethod for EFI boot manager
 *
 * Copyright 2021 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#define LOG_CATEGORY UCLASS_BOOTSTD

#include <common.h>
#include <bootdev.h>
#include <bootflow.h>
#include <bootmeth.h>
#include <command.h>
#include <dm.h>
#include <efi_loader.h>
#include <efi_variable.h>
#include <malloc.h>

/**
 * struct efi_mgr_priv - private info for the efi-mgr driver
 *
 * @fake_bootflow: Fake a valid bootflow for testing
 */
struct efi_mgr_priv {
	bool fake_dev;
};

void sandbox_set_fake_efi_mgr_dev(struct udevice *dev, bool fake_dev)
{
	struct efi_mgr_priv *priv = dev_get_priv(dev);

	priv->fake_dev = fake_dev;
}

static int efi_mgr_check(struct udevice *dev, struct bootflow_iter *iter)
{
	int ret;

	/* Must be an bootstd device */
	ret = bootflow_iter_check_system(iter);
	if (ret)
		return log_msg_ret("net", ret);

	return 0;
}

static int efi_mgr_read_bootflow(struct udevice *dev, struct bootflow *bflow)
{
	struct efi_mgr_priv *priv = dev_get_priv(dev);
	efi_status_t ret;
	efi_uintn_t size;
	u16 *bootorder;

	if (priv->fake_dev) {
		bflow->state = BOOTFLOWST_READY;
		return 0;
	}

	ret = efi_init_obj_list();
	if (ret)
		return log_msg_ret("init", ret);

	/* Enable this method if the "BootOrder" UEFI exists. */
	bootorder = efi_get_var(u"BootOrder", &efi_global_variable_guid,
				&size);
	if (bootorder) {
		free(bootorder);
		bflow->state = BOOTFLOWST_READY;
		return 0;
	}

	return -EINVAL;
}

static int efi_mgr_read_file(struct udevice *dev, struct bootflow *bflow,
				const char *file_path, ulong addr, ulong *sizep)
{
	/* Files are loaded by the 'bootefi bootmgr' command */

	return -ENOSYS;
}

static int efi_mgr_boot(struct udevice *dev, struct bootflow *bflow)
{
	int ret;

	/* Booting is handled by the 'bootefi bootmgr' command */
	ret = efi_bootmgr_run(EFI_FDT_USE_INTERNAL);

	return 0;
}

static int bootmeth_efi_mgr_bind(struct udevice *dev)
{
	struct bootmeth_uc_plat *plat = dev_get_uclass_plat(dev);

	plat->desc = "EFI bootmgr flow";
	plat->flags = BOOTMETHF_GLOBAL;

	return 0;
}

static struct bootmeth_ops efi_mgr_bootmeth_ops = {
	.check		= efi_mgr_check,
	.read_bootflow	= efi_mgr_read_bootflow,
	.read_file	= efi_mgr_read_file,
	.boot		= efi_mgr_boot,
};

static const struct udevice_id efi_mgr_bootmeth_ids[] = {
	{ .compatible = "u-boot,efi-bootmgr" },
	{ }
};

U_BOOT_DRIVER(bootmeth_efi_mgr) = {
	.name		= "bootmeth_efi_mgr",
	.id		= UCLASS_BOOTMETH,
	.of_match	= efi_mgr_bootmeth_ids,
	.ops		= &efi_mgr_bootmeth_ops,
	.bind		= bootmeth_efi_mgr_bind,
	.priv_auto	= sizeof(struct efi_mgr_priv),
};
