// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2018 Ramon Fried <ramon.fried@gmail.com>
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <smem.h>
#include <asm/test.h>

static int sandbox_smem_alloc(unsigned int host,
		unsigned int item, size_t size)
{
	return 0;
}

static void *sandbox_smem_get(unsigned int host,
		unsigned int item, size_t *size)
{
	return NULL;
}

static int sandbox_smem_get_free_space(unsigned int host)
{
	return 0;
}

static const struct smem_ops sandbox_smem_ops = {
	.alloc	= sandbox_smem_alloc,
	.get	= sandbox_smem_get,
	.get_free_space	= sandbox_smem_get_free_space,
};

static const struct udevice_id sandbox_smem_ids[] = {
	{ .compatible = "sandbox,smem" },
	{ }
};

U_BOOT_DRIVER(smem_sandbox) = {
	.name		= "smem_sandbox",
	.id		= UCLASS_SMEM,
	.of_match	= sandbox_smem_ids,
	.ops		= &sandbox_smem_ops,
};
