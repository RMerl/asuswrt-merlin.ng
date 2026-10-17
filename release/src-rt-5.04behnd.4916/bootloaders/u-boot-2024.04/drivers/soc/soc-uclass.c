// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2020 - Texas Instruments Incorporated - https://www.ti.com/
 *	Dave Gerlach <d-gerlach@ti.com>
 */

#define LOG_CATEGORY UCLASS_SOC

#include <common.h>
#include <soc.h>
#include <dm.h>
#include <errno.h>
#include <dm/lists.h>
#include <dm/root.h>

int soc_get(struct udevice **devp)
{
	return uclass_first_device_err(UCLASS_SOC, devp);
}

int soc_get_machine(struct udevice *dev, char *buf, int size)
{
	struct soc_ops *ops = soc_get_ops(dev);

	if (!ops->get_machine)
		return -ENOSYS;

	return ops->get_machine(dev, buf, size);
}

int soc_get_family(struct udevice *dev, char *buf, int size)
{
	struct soc_ops *ops = soc_get_ops(dev);

	if (!ops->get_family)
		return -ENOSYS;

	return ops->get_family(dev, buf, size);
}

int soc_get_revision(struct udevice *dev, char *buf, int size)
{
	struct soc_ops *ops = soc_get_ops(dev);

	if (!ops->get_revision)
		return -ENOSYS;

	return ops->get_revision(dev, buf, size);
}

const struct soc_attr *
soc_device_match(const struct soc_attr *matches)
{
	bool match;
	struct udevice *soc;
	char str[SOC_MAX_STR_SIZE];

	if (!matches)
		return NULL;

	if (soc_get(&soc))
		return NULL;

	while (1) {
		if (!(matches->machine || matches->family ||
		      matches->revision))
			break;

		match = true;

		if (matches->machine) {
			if (!soc_get_machine(soc, str, SOC_MAX_STR_SIZE)) {
				if (strcmp(matches->machine, str))
					match = false;
			}
		}

		if (matches->family) {
			if (!soc_get_family(soc, str, SOC_MAX_STR_SIZE)) {
				if (strcmp(matches->family, str))
					match = false;
			}
		}

		if (matches->revision) {
			if (!soc_get_revision(soc, str, SOC_MAX_STR_SIZE)) {
				if (strcmp(matches->revision, str))
					match = false;
			}
		}

		if (match)
			return matches;

		matches++;
	}

	return NULL;
}

UCLASS_DRIVER(soc) = {
	.id		= UCLASS_SOC,
	.name		= "soc",
};
