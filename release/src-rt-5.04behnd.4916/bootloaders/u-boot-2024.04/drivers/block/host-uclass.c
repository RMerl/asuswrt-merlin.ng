// SPDX-License-Identifier: GPL-2.0+
/*
 * Uclass for sandbox host interface, used to access files on the host which
 * contain partitions and filesystem
 *
 * Copyright 2022 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#define LOG_CATEGORY UCLASS_HOST

#include <common.h>
#include <blk.h>
#include <dm.h>
#include <malloc.h>
#include <part.h>
#include <sandbox_host.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <dm/uclass-internal.h>

DECLARE_GLOBAL_DATA_PTR;

/**
 * struct host_priv - information kept by the host uclass
 *
 * @cur_dev: Currently selected host device, or NULL if none
 */
struct host_priv {
	struct udevice *cur_dev;
};

int host_create_device(const char *label, bool removable, unsigned long blksz,
		       struct udevice **devp)
{
	char dev_name[30], *str, *label_new;
	struct host_sb_plat *plat;
	struct udevice *dev, *blk;
	int ret;

	/* unbind any existing device with this label */
	dev = host_find_by_label(label);
	if (dev) {
		ret = host_detach_file(dev);
		if (ret)
			return log_msg_ret("det", ret);

		ret = device_unbind(dev);
		if (ret)
			return log_msg_ret("unb", ret);
	}

	snprintf(dev_name, sizeof(dev_name), "host-%s", label);
	str = strdup(dev_name);
	if (!str)
		return -ENOMEM;

	label_new = strdup(label);
	if (!label_new) {
		ret = -ENOMEM;
		goto no_label;
	}

	ret = device_bind_driver(gd->dm_root, "host_sb_drv", str, &dev);
	if (ret)
		goto no_dev;
	device_set_name_alloced(dev);

	if (!blk_find_from_parent(dev, &blk)) {
		struct blk_desc *desc = dev_get_uclass_plat(blk);

		desc->removable = removable;

		/* update blk device's block size with the provided one */
		if (blksz != desc->blksz) {
			desc->blksz = blksz;
			desc->log2blksz = LOG2(desc->blksz);
		}
	}

	plat = dev_get_plat(dev);
	plat->label = label_new;
	*devp = dev;

	return 0;

no_dev:
	free(label_new);
no_label:
	free(str);

	return ret;
}

int host_attach_file(struct udevice *dev, const char *filename)
{
	struct host_ops *ops = host_get_ops(dev);

	if (!ops->attach_file)
		return -ENOSYS;

	return ops->attach_file(dev, filename);
}

int host_create_attach_file(const char *label, const char *filename,
			    bool removable, unsigned long blksz,
			    struct udevice **devp)
{
	struct udevice *dev;
	int ret;

	ret = host_create_device(label, removable, blksz, &dev);
	if (ret)
		return log_msg_ret("cre", ret);

	ret = host_attach_file(dev, filename);
	if (ret) {
		device_unbind(dev);
		return log_msg_ret("att", ret);
	}
	*devp = dev;

	return 0;
}

int host_detach_file(struct udevice *dev)
{
	struct host_ops *ops = host_get_ops(dev);

	if (!ops->detach_file)
		return -ENOSYS;

	if (dev == host_get_cur_dev())
		host_set_cur_dev(NULL);

	return ops->detach_file(dev);
}

struct udevice *host_find_by_label(const char *label)
{
	struct udevice *dev;
	struct uclass *uc;

	uclass_id_foreach_dev(UCLASS_HOST, dev, uc) {
		struct host_sb_plat *plat = dev_get_plat(dev);

		if (plat->label && !strcmp(label, plat->label))
			return dev;
	}

	return NULL;
}

struct udevice *host_get_cur_dev(void)
{
	struct uclass *uc = uclass_find(UCLASS_HOST);

	if (uc) {
		struct host_priv *priv = uclass_get_priv(uc);

		return priv->cur_dev;
	}

	return NULL;
}

void host_set_cur_dev(struct udevice *dev)
{
	struct uclass *uc = uclass_find(UCLASS_HOST);

	if (uc) {
		struct host_priv *priv = uclass_get_priv(uc);

		priv->cur_dev = dev;
	}
}

UCLASS_DRIVER(host) = {
	.id		= UCLASS_HOST,
	.name		= "host",
#if CONFIG_IS_ENABLED(OF_REAL)
	.post_bind	= dm_scan_fdt_dev,
#endif
	.priv_auto	= sizeof(struct host_priv),
};
