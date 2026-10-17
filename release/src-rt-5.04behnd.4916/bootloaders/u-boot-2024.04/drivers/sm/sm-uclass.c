// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2023 SberDevices, Inc.
 *
 * Author: Alexey Romanov <avromanov@salutedevices.com>
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <sm-uclass.h>

static const struct sm_ops *get_sm_ops(struct udevice *dev)
{
	return (const struct sm_ops *)dev->driver->ops;
}

int sm_call(struct udevice *dev, u32 cmd, s32 *ret, struct pt_regs *args)
{
	const struct sm_ops *ops = get_sm_ops(dev);

	if (ops->sm_call)
		return ops->sm_call(dev, cmd, ret, args);

	return -ENOSYS;
}

int sm_call_read(struct udevice *dev, void *buffer, size_t size,
		 u32 cmd, struct pt_regs *args)
{
	const struct sm_ops *ops = get_sm_ops(dev);

	if (ops->sm_call_read)
		return ops->sm_call_read(dev, buffer, size, cmd,
					 args);

	return -ENOSYS;
}

int sm_call_write(struct udevice *dev, void *buffer, size_t size,
		   u32 cmd, struct pt_regs *args)
{
	const struct sm_ops *ops = get_sm_ops(dev);

	if (ops->sm_call_write)
		return ops->sm_call_write(dev, buffer, size, cmd,
					  args);

	return -ENOSYS;
}

UCLASS_DRIVER(sm) = {
	.name           = "sm",
	.id             = UCLASS_SM,
};
