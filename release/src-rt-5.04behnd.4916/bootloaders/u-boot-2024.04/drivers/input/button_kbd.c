// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2023 Dzmitry Sankouski <dsankouski@gmail.com>
 */

#include <stdlib.h>
#include <common.h>
#include <dm.h>
#include <fdtdec.h>
#include <input.h>
#include <keyboard.h>
#include <button.h>
#include <dm/device-internal.h>
#include <log.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <linux/delay.h>
#include <linux/input.h>

/**
 * struct button_kbd_priv - driver private data
 *
 * @input: input configuration
 * @button_size: number of buttons found
 * @old_state: a pointer to old button states array. Used to determine button state change.
 */
struct button_kbd_priv {
	struct input_config *input;
	u32 button_size;
	u32 *old_state;
};

static int button_kbd_start(struct udevice *dev)
{
	struct button_kbd_priv *priv = dev_get_priv(dev);
	int i = 0;
	struct udevice *button_gpio_devp;

	uclass_foreach_dev_probe(UCLASS_BUTTON, button_gpio_devp) {
		struct button_uc_plat *uc_plat = dev_get_uclass_plat(button_gpio_devp);
		/* Ignore the top-level button node */
		if (!uc_plat->label)
			continue;
		debug("Found button %s #%d - %s, probing...\n",
		      uc_plat->label, i, button_gpio_devp->name);
		i++;
	}

	priv->button_size = i;
	priv->old_state = calloc(i, sizeof(int));

	return 0;
}

int button_read_keys(struct input_config *input)
{
	struct button_kbd_priv *priv = dev_get_priv(input->dev);
	struct udevice *button_gpio_devp;
	struct uclass *uc;
	int i = 0;
	u32 code, state, state_changed = 0;

	uclass_id_foreach_dev(UCLASS_BUTTON, button_gpio_devp, uc) {
		struct button_uc_plat *uc_plat = dev_get_uclass_plat(button_gpio_devp);
		/* Ignore the top-level button node */
		if (!uc_plat->label)
			continue;
		code = button_get_code(button_gpio_devp);
		if (!code)
			continue;

		state = button_get_state(button_gpio_devp);
		state_changed = state != priv->old_state[i];

		if (state_changed) {
			debug("%s: %d\n", uc_plat->label, code);
			priv->old_state[i] = state;
			input_add_keycode(input, code, state);
		}
		i++;
	}
	return 0;
}

static const struct keyboard_ops button_kbd_ops = {
	.start	= button_kbd_start,
};

static int button_kbd_probe(struct udevice *dev)
{
	struct button_kbd_priv *priv = dev_get_priv(dev);
	struct keyboard_priv *uc_priv = dev_get_uclass_priv(dev);
	struct stdio_dev *sdev = &uc_priv->sdev;
	struct input_config *input = &uc_priv->input;
	int ret = 0;

	input_init(input, false);
	input_add_tables(input, false);

	/* Register the device. */
	priv->input = input;
	input->dev = dev;
	input->read_keys = button_read_keys;
	strcpy(sdev->name, "button-kbd");
	ret = input_stdio_register(sdev);
	if (ret) {
		debug("%s: input_stdio_register() failed\n", __func__);
		return ret;
	}

	return 0;
}

U_BOOT_DRIVER(button_kbd) = {
	.name		= "button_kbd",
	.id		= UCLASS_KEYBOARD,
	.ops		= &button_kbd_ops,
	.priv_auto	= sizeof(struct button_kbd_priv),
	.probe		= button_kbd_probe,
};

U_BOOT_DRVINFO(button_kbd) = {
	.name = "button_kbd"
};
