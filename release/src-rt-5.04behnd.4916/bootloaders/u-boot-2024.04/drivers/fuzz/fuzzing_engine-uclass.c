/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2022 Google, Inc.
 * Written by Andrew Scull <ascull@google.com>
 */

#define LOG_CATEGORY UCLASS_FUZZING_ENGINE

#include <common.h>
#include <dm.h>
#include <fuzzing_engine.h>

int dm_fuzzing_engine_get_input(struct udevice *dev,
				const uint8_t **data,
				size_t *size)
{
	const struct dm_fuzzing_engine_ops *ops = device_get_ops(dev);

	if (!ops->get_input)
		return -ENOSYS;

	return ops->get_input(dev, data, size);
}

UCLASS_DRIVER(fuzzing_engine) = {
	.name = "fuzzing_engine",
	.id = UCLASS_FUZZING_ENGINE,
};
