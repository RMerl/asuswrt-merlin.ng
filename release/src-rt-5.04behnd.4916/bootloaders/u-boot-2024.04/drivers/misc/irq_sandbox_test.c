// SPDX-License-Identifier: GPL-2.0
/*
 * Sandbox driver for testing interrupts with of-platdata
 *
 * Copyright 2021 Google LLC
 */

#include <common.h>
#include <dm.h>
#include <irq.h>
#include <asm/irq.h>

static const struct udevice_id sandbox_irq_test_ids[] = {
	{ .compatible = "sandbox,irq-test" },
	{ }
};

U_BOOT_DRIVER(sandbox_irq_test) = {
	.name = "sandbox_irq_test",
	.id = UCLASS_MISC,
	.of_match = sandbox_irq_test_ids,
};
