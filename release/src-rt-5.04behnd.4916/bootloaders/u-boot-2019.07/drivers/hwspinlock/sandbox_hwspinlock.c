// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright (C) 2018, STMicroelectronics - All Rights Reserved
 */

#include <common.h>
#include <dm.h>
#include <hwspinlock.h>
#include <asm/state.h>

static int sandbox_lock(struct udevice *dev, int index)
{
	struct sandbox_state *state = state_get_current();

	if (index != 0)
		return -1;

	if (state->hwspinlock)
		return -1;

	state->hwspinlock = true;

	return 0;
}

static int sandbox_unlock(struct udevice *dev, int index)
{
	struct sandbox_state *state = state_get_current();

	if (index != 0)
		return -1;

	if (!state->hwspinlock)
		return -1;

	state->hwspinlock = false;

	return 0;
}

static const struct hwspinlock_ops sandbox_hwspinlock_ops = {
	.lock = sandbox_lock,
	.unlock = sandbox_unlock,
};

static const struct udevice_id sandbox_hwspinlock_ids[] = {
	{ .compatible = "sandbox,hwspinlock" },
	{}
};

U_BOOT_DRIVER(hwspinlock_sandbox) = {
	.name = "hwspinlock_sandbox",
	.id = UCLASS_HWSPINLOCK,
	.of_match = sandbox_hwspinlock_ids,
	.ops = &sandbox_hwspinlock_ops,
};
