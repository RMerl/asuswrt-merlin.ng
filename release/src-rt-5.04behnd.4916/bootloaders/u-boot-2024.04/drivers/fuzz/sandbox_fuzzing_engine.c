/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2022 Google, Inc.
 * Written by Andrew Scull <ascull@google.com>
 */

#include <common.h>
#include <dm.h>
#include <fuzzing_engine.h>
#include <asm/fuzzing_engine.h>

static int get_input(struct udevice *dev,
		     const uint8_t **data,
		     size_t *size)
{
	return sandbox_fuzzing_engine_get_input(data, size);
}

static const struct dm_fuzzing_engine_ops sandbox_fuzzing_engine_ops = {
	.get_input = get_input,
};

static const struct udevice_id sandbox_fuzzing_engine_match[] = {
	{
		.compatible = "sandbox,fuzzing-engine",
	},
	{},
};

U_BOOT_DRIVER(sandbox_fuzzing_engine) = {
	.name = "sandbox-fuzzing-engine",
	.id = UCLASS_FUZZING_ENGINE,
	.of_match = sandbox_fuzzing_engine_match,
	.ops = &sandbox_fuzzing_engine_ops,
};
