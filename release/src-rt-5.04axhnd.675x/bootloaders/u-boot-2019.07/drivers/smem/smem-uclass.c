// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2018 Ramon Fried <ramon.fried@gmail.com>
 */

#include <common.h>
#include <dm.h>
#include <smem.h>

int smem_alloc(struct udevice *dev, unsigned int host,
		unsigned int item, size_t size)
{
	struct smem_ops *ops = smem_get_ops(dev);

	if (!ops->alloc)
		return -ENOSYS;

	return ops->alloc(host, item, size);
}

void *smem_get(struct udevice *dev, unsigned int host,
		unsigned int item, size_t *size)
{
	struct smem_ops *ops = smem_get_ops(dev);

	if (!ops->get)
		return NULL;

	return ops->get(host, item, size);
}

int smem_get_free_space(struct udevice *dev, unsigned int host)
{
	struct smem_ops *ops = smem_get_ops(dev);

	if (!ops->get_free_space)
		return -ENOSYS;

	return ops->get_free_space(host);
}

UCLASS_DRIVER(smem) = {
	.id     = UCLASS_SMEM,
	.name       = "smem",
};
