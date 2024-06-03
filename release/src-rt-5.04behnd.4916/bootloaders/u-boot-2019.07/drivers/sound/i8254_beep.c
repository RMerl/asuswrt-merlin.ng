// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018 Google LLC
 */

#include <common.h>
#include <dm.h>
#include <sound.h>
#include <asm/i8254.h>

int i8254_start_beep(struct udevice *dev, int frequency_hz)
{
	return i8254_enable_beep(frequency_hz);
}

int i8254_stop_beep(struct udevice *dev)
{
	i8254_disable_beep();

	return 0;
}

static const struct sound_ops i8254_ops = {
	.start_beep	= i8254_start_beep,
	.stop_beep	= i8254_stop_beep,
};

static const struct udevice_id i8254_ids[] = {
	{ .compatible = "i8254,beeper" },
	{ }
};

U_BOOT_DRIVER(i8254_drv) = {
	.name		= "i8254_drv",
	.id		= UCLASS_SOUND,
	.of_match	= i8254_ids,
	.ops		= &i8254_ops,
};
