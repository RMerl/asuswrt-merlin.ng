// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020 Philippe Reynes <philippe.reynes@softathome.com>
 *
 * Based on led-uclass.c
 */

#define LOG_CATEGORY UCLASS_BUTTON

#include <common.h>
#include <button.h>
#include <dm.h>
#include <dm/uclass-internal.h>

int button_get_by_label(const char *label, struct udevice **devp)
{
	struct udevice *dev;
	struct uclass *uc;

	uclass_id_foreach_dev(UCLASS_BUTTON, dev, uc) {
		struct button_uc_plat *uc_plat = dev_get_uclass_plat(dev);

		/* Ignore the top-level button node */
		if (uc_plat->label && !strcmp(label, uc_plat->label))
			return uclass_get_device_tail(dev, 0, devp);
	}

	return -ENODEV;
}

enum button_state_t button_get_state(struct udevice *dev)
{
	struct button_ops *ops = button_get_ops(dev);

	if (!ops->get_state)
		return -ENOSYS;

	return ops->get_state(dev);
}

int button_get_code(struct udevice *dev)
{
	struct button_ops *ops = button_get_ops(dev);

	if (!ops->get_code)
		return -ENOSYS;

	return ops->get_code(dev);
}

UCLASS_DRIVER(button) = {
	.id		= UCLASS_BUTTON,
	.name		= "button",
	.per_device_plat_auto	= sizeof(struct button_uc_plat),
};
