// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2017 Google, Inc
 */

#include <common.h>
#include <dm.h>
#include <wdt.h>
#include <asm/state.h>

static int sandbox_wdt_start(struct udevice *dev, u64 timeout, ulong flags)
{
	struct sandbox_state *state = state_get_current();

	state->wdt.counter = timeout;
	state->wdt.running = true;

	return 0;
}

static int sandbox_wdt_stop(struct udevice *dev)
{
	struct sandbox_state *state = state_get_current();

	state->wdt.running = false;

	return 0;
}

static int sandbox_wdt_reset(struct udevice *dev)
{
	struct sandbox_state *state = state_get_current();

	state->wdt.reset_count++;

	return 0;
}

static int sandbox_wdt_expire_now(struct udevice *dev, ulong flags)
{
	sandbox_wdt_start(dev, 1, flags);

	return 0;
}

static const struct wdt_ops sandbox_wdt_ops = {
	.start = sandbox_wdt_start,
	.reset = sandbox_wdt_reset,
	.stop = sandbox_wdt_stop,
	.expire_now = sandbox_wdt_expire_now,
};

static const struct udevice_id sandbox_wdt_ids[] = {
	{ .compatible = "sandbox,wdt" },
	{}
};

U_BOOT_DRIVER(wdt_sandbox) = {
	.name = "wdt_sandbox",
	.id = UCLASS_WDT,
	.of_match = sandbox_wdt_ids,
	.ops = &sandbox_wdt_ops,
};
