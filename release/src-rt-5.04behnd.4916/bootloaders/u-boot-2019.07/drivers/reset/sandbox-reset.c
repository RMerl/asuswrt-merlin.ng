// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2016, NVIDIA CORPORATION.
 */

#include <common.h>
#include <dm.h>
#include <reset-uclass.h>
#include <asm/io.h>
#include <asm/reset.h>

#define SANDBOX_RESET_SIGNALS 101

struct sandbox_reset_signal {
	bool asserted;
};

struct sandbox_reset {
	struct sandbox_reset_signal signals[SANDBOX_RESET_SIGNALS];
};

static int sandbox_reset_request(struct reset_ctl *reset_ctl)
{
	debug("%s(reset_ctl=%p)\n", __func__, reset_ctl);

	if (reset_ctl->id >= SANDBOX_RESET_SIGNALS)
		return -EINVAL;

	return 0;
}

static int sandbox_reset_free(struct reset_ctl *reset_ctl)
{
	debug("%s(reset_ctl=%p)\n", __func__, reset_ctl);

	return 0;
}

static int sandbox_reset_assert(struct reset_ctl *reset_ctl)
{
	struct sandbox_reset *sbr = dev_get_priv(reset_ctl->dev);

	debug("%s(reset_ctl=%p)\n", __func__, reset_ctl);

	sbr->signals[reset_ctl->id].asserted = true;

	return 0;
}

static int sandbox_reset_deassert(struct reset_ctl *reset_ctl)
{
	struct sandbox_reset *sbr = dev_get_priv(reset_ctl->dev);

	debug("%s(reset_ctl=%p)\n", __func__, reset_ctl);

	sbr->signals[reset_ctl->id].asserted = false;

	return 0;
}

static int sandbox_reset_bind(struct udevice *dev)
{
	debug("%s(dev=%p)\n", __func__, dev);

	return 0;
}

static int sandbox_reset_probe(struct udevice *dev)
{
	debug("%s(dev=%p)\n", __func__, dev);

	return 0;
}

static const struct udevice_id sandbox_reset_ids[] = {
	{ .compatible = "sandbox,reset-ctl" },
	{ }
};

struct reset_ops sandbox_reset_reset_ops = {
	.request = sandbox_reset_request,
	.free = sandbox_reset_free,
	.rst_assert = sandbox_reset_assert,
	.rst_deassert = sandbox_reset_deassert,
};

U_BOOT_DRIVER(sandbox_reset) = {
	.name = "sandbox_reset",
	.id = UCLASS_RESET,
	.of_match = sandbox_reset_ids,
	.bind = sandbox_reset_bind,
	.probe = sandbox_reset_probe,
	.priv_auto_alloc_size = sizeof(struct sandbox_reset),
	.ops = &sandbox_reset_reset_ops,
};

int sandbox_reset_query(struct udevice *dev, unsigned long id)
{
	struct sandbox_reset *sbr = dev_get_priv(dev);

	debug("%s(dev=%p, id=%ld)\n", __func__, dev, id);

	if (id >= SANDBOX_RESET_SIGNALS)
		return -EINVAL;

	return sbr->signals[id].asserted;
}
