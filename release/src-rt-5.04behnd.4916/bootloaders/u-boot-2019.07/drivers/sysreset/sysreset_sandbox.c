// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <sysreset.h>
#include <asm/state.h>
#include <asm/test.h>

static int sandbox_warm_sysreset_request(struct udevice *dev,
					 enum sysreset_t type)
{
	struct sandbox_state *state = state_get_current();

	switch (type) {
	case SYSRESET_WARM:
		state->last_sysreset = type;
		break;
	default:
		return -ENOSYS;
	}
	if (!state->sysreset_allowed[type])
		return -EACCES;

	return -EINPROGRESS;
}

int sandbox_warm_sysreset_get_status(struct udevice *dev, char *buf, int size)
{
	strlcpy(buf, "Reset Status: WARM", size);

	return 0;
}

int sandbox_warm_sysreset_get_last(struct udevice *dev)
{
	return SYSRESET_WARM;
}

static int sandbox_sysreset_request(struct udevice *dev, enum sysreset_t type)
{
	struct sandbox_state *state = state_get_current();

	/*
	 * If we have a device tree, the device we created from platform data
	 * (see the U_BOOT_DEVICE() declaration below) should not do anything.
	 * If we are that device, return an error.
	 */
	if (state->fdt_fname && !dev_of_valid(dev))
		return -ENODEV;

	switch (type) {
	case SYSRESET_COLD:
		state->last_sysreset = type;
		break;
	case SYSRESET_POWER:
		state->last_sysreset = type;
		if (!state->sysreset_allowed[type])
			return -EACCES;
		sandbox_exit();
		break;
	case SYSRESET_POWER_OFF:
		if (!state->sysreset_allowed[type])
			return -EACCES;
	default:
		return -ENOSYS;
	}
	if (!state->sysreset_allowed[type])
		return -EACCES;

	return -EINPROGRESS;
}

int sandbox_sysreset_get_status(struct udevice *dev, char *buf, int size)
{
	strlcpy(buf, "Reset Status: COLD", size);

	return 0;
}

int sandbox_sysreset_get_last(struct udevice *dev)
{
	struct sandbox_state *state = state_get_current();

	/*
	 * The first phase is a power reset, after that we assume we don't
	 * know.
	 */
	return state->jumped_fname ? SYSRESET_WARM : SYSRESET_POWER;
}

static struct sysreset_ops sandbox_sysreset_ops = {
	.request	= sandbox_sysreset_request,
	.get_status	= sandbox_sysreset_get_status,
	.get_last	= sandbox_sysreset_get_last,
};

static const struct udevice_id sandbox_sysreset_ids[] = {
	{ .compatible = "sandbox,reset" },
	{ }
};

U_BOOT_DRIVER(sysreset_sandbox) = {
	.name		= "sysreset_sandbox",
	.id		= UCLASS_SYSRESET,
	.of_match	= sandbox_sysreset_ids,
	.ops		= &sandbox_sysreset_ops,
};

static struct sysreset_ops sandbox_warm_sysreset_ops = {
	.request	= sandbox_warm_sysreset_request,
	.get_status	= sandbox_warm_sysreset_get_status,
	.get_last	= sandbox_warm_sysreset_get_last,
};

static const struct udevice_id sandbox_warm_sysreset_ids[] = {
	{ .compatible = "sandbox,warm-reset" },
	{ }
};

U_BOOT_DRIVER(warm_sysreset_sandbox) = {
	.name		= "warm_sysreset_sandbox",
	.id		= UCLASS_SYSRESET,
	.of_match	= sandbox_warm_sysreset_ids,
	.ops		= &sandbox_warm_sysreset_ops,
};

/* This is here in case we don't have a device tree */
U_BOOT_DEVICE(sysreset_sandbox_non_fdt) = {
	.name = "sysreset_sandbox",
};
