// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2002 ELTEC Elektronik AG
 * Frank Gottschling <fgottschling@eltec.de>
 */

/* i8042.c - Intel 8042 keyboard driver routines */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <i8042.h>
#include <input.h>
#include <keyboard.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

/* defines */
#define in8(p)		inb(p)
#define out8(p, v)	outb(v, p)

enum {
	QUIRK_DUP_POR	= 1 << 0,
};

/* locals */
struct i8042_kbd_priv {
	bool extended;	/* true if an extended keycode is expected next */
	int quirks;	/* quirks that we support */
};

static unsigned char ext_key_map[] = {
	0x1c, /* keypad enter */
	0x1d, /* right control */
	0x35, /* keypad slash */
	0x37, /* print screen */
	0x38, /* right alt */
	0x46, /* break */
	0x47, /* editpad home */
	0x48, /* editpad up */
	0x49, /* editpad pgup */
	0x4b, /* editpad left */
	0x4d, /* editpad right */
	0x4f, /* editpad end */
	0x50, /* editpad dn */
	0x51, /* editpad pgdn */
	0x52, /* editpad ins */
	0x53, /* editpad del */
	0x00  /* map end */
	};

static int kbd_input_empty(void)
{
	int kbd_timeout = KBD_TIMEOUT * 1000;

	while ((in8(I8042_STS_REG) & STATUS_IBF) && kbd_timeout--)
		udelay(1);

	return kbd_timeout != -1;
}

static int kbd_output_full(void)
{
	int kbd_timeout = KBD_TIMEOUT * 1000;

	while (((in8(I8042_STS_REG) & STATUS_OBF) == 0) && kbd_timeout--)
		udelay(1);

	return kbd_timeout != -1;
}

/**
 * check_leds() - Check the keyboard LEDs and update them it needed
 *
 * @ret:	Value to return
 * @return value of @ret
 */
static int i8042_kbd_update_leds(struct udevice *dev, int leds)
{
	kbd_input_empty();
	out8(I8042_DATA_REG, CMD_SET_KBD_LED);
	kbd_input_empty();
	out8(I8042_DATA_REG, leds & 0x7);

	return 0;
}

static int kbd_write(int reg, int value)
{
	if (!kbd_input_empty())
		return -1;
	out8(reg, value);

	return 0;
}

static int kbd_read(int reg)
{
	if (!kbd_output_full())
		return -1;

	return in8(reg);
}

static int kbd_cmd_read(int cmd)
{
	if (kbd_write(I8042_CMD_REG, cmd))
		return -1;

	return kbd_read(I8042_DATA_REG);
}

static int kbd_cmd_write(int cmd, int data)
{
	if (kbd_write(I8042_CMD_REG, cmd))
		return -1;

	return kbd_write(I8042_DATA_REG, data);
}

static int kbd_reset(int quirk)
{
	int config;

	/* controller self test */
	if (kbd_cmd_read(CMD_SELF_TEST) != KBC_TEST_OK)
		goto err;

	/* keyboard reset */
	if (kbd_write(I8042_DATA_REG, CMD_RESET_KBD) ||
	    kbd_read(I8042_DATA_REG) != KBD_ACK ||
	    kbd_read(I8042_DATA_REG) != KBD_POR)
		goto err;

	if (kbd_write(I8042_DATA_REG, CMD_DRAIN_OUTPUT) ||
	    kbd_read(I8042_DATA_REG) != KBD_ACK)
		goto err;

	/* set AT translation and disable irq */
	config = kbd_cmd_read(CMD_RD_CONFIG);
	if (config == -1)
		goto err;

	/* Sometimes get a second byte */
	else if ((quirk & QUIRK_DUP_POR) && config == KBD_POR)
		config = kbd_cmd_read(CMD_RD_CONFIG);

	config |= CONFIG_AT_TRANS;
	config &= ~(CONFIG_KIRQ_EN | CONFIG_MIRQ_EN);
	if (kbd_cmd_write(CMD_WR_CONFIG, config))
		goto err;

	/* enable keyboard */
	if (kbd_write(I8042_CMD_REG, CMD_KBD_EN) ||
	    !kbd_input_empty())
		goto err;

	return 0;
err:
	debug("%s: Keyboard failure\n", __func__);
	return -1;
}

static int kbd_controller_present(void)
{
	return in8(I8042_STS_REG) != 0xff;
}

/** Flush all buffer from keyboard controller to host*/
static void i8042_flush(void)
{
	int timeout;

	/*
	 * The delay is to give the keyboard controller some time
	 * to fill the next byte.
	 */
	while (1) {
		timeout = 100;	/* wait for no longer than 100us */
		while (timeout > 0 && !(in8(I8042_STS_REG) & STATUS_OBF)) {
			udelay(1);
			timeout--;
		}

		/* Try to pull next byte if not timeout */
		if (in8(I8042_STS_REG) & STATUS_OBF)
			in8(I8042_DATA_REG);
		else
			break;
	}
}

/**
 * Disables the keyboard so that key strokes no longer generate scancodes to
 * the host.
 *
 * @return 0 if ok, -1 if keyboard input was found while disabling
 */
static int i8042_disable(void)
{
	if (kbd_input_empty() == 0)
		return -1;

	/* Disable keyboard */
	out8(I8042_CMD_REG, CMD_KBD_DIS);

	if (kbd_input_empty() == 0)
		return -1;

	return 0;
}

static int i8042_kbd_check(struct input_config *input)
{
	struct i8042_kbd_priv *priv = dev_get_priv(input->dev);

	if ((in8(I8042_STS_REG) & STATUS_OBF) == 0) {
		return 0;
	} else {
		bool release = false;
		int scan_code;
		int i;

		scan_code = in8(I8042_DATA_REG);
		if (scan_code == 0xfa) {
			return 0;
		} else if (scan_code == 0xe0) {
			priv->extended = true;
			return 0;
		}
		if (scan_code & 0x80) {
			scan_code &= 0x7f;
			release = true;
		}
		if (priv->extended) {
			priv->extended = false;
			for (i = 0; ext_key_map[i]; i++) {
				if (ext_key_map[i] == scan_code) {
					scan_code = 0x60 + i;
					break;
				}
			}
			/* not found ? */
			if (!ext_key_map[i])
				return 0;
		}

		input_add_keycode(input, scan_code, release);
		return 1;
	}
}

/* i8042_kbd_init - reset keyboard and init state flags */
static int i8042_start(struct udevice *dev)
{
	struct keyboard_priv *uc_priv = dev_get_uclass_priv(dev);
	struct i8042_kbd_priv *priv = dev_get_priv(dev);
	struct input_config *input = &uc_priv->input;
	int keymap, try;
	char *penv;
	int ret;

	if (!kbd_controller_present()) {
		debug("i8042 keyboard controller is not present\n");
		return -ENOENT;
	}

	/* Init keyboard device (default US layout) */
	keymap = KBD_US;
	penv = env_get("keymap");
	if (penv != NULL) {
		if (strncmp(penv, "de", 3) == 0)
			keymap = KBD_GER;
	}

	for (try = 0; kbd_reset(priv->quirks) != 0; try++) {
		if (try >= KBD_RESET_TRIES)
			return -1;
	}

	ret = input_add_tables(input, keymap == KBD_GER);
	if (ret)
		return ret;

	i8042_kbd_update_leds(dev, NORMAL);
	debug("%s: started\n", __func__);

	return 0;
}

static int i8042_kbd_remove(struct udevice *dev)
{
	if (i8042_disable())
		log_debug("i8042_disable() failed. fine, continue.\n");
	i8042_flush();

	return 0;
}

/**
 * Set up the i8042 keyboard. This is called by the stdio device handler
 *
 * We want to do this init when the keyboard is actually used rather than
 * at start-up, since keyboard input may not currently be selected.
 *
 * Once the keyboard starts there will be a period during which we must
 * wait for the keyboard to init. We do this only when a key is first
 * read - see kbd_wait_for_fifo_init().
 *
 * @return 0 if ok, -ve on error
 */
static int i8042_kbd_probe(struct udevice *dev)
{
	struct keyboard_priv *uc_priv = dev_get_uclass_priv(dev);
	struct i8042_kbd_priv *priv = dev_get_priv(dev);
	struct stdio_dev *sdev = &uc_priv->sdev;
	struct input_config *input = &uc_priv->input;
	int ret;

	if (fdtdec_get_bool(gd->fdt_blob, dev_of_offset(dev),
			    "intel,duplicate-por"))
		priv->quirks |= QUIRK_DUP_POR;

	/* Register the device. i8042_start() will be called soon */
	input->dev = dev;
	input->read_keys = i8042_kbd_check;
	input_allow_repeats(input, true);
	strcpy(sdev->name, "i8042-kbd");
	ret = input_stdio_register(sdev);
	if (ret) {
		debug("%s: input_stdio_register() failed\n", __func__);
		return ret;
	}
	debug("%s: ready\n", __func__);

	return 0;
}

static const struct keyboard_ops i8042_kbd_ops = {
	.start	= i8042_start,
	.update_leds	= i8042_kbd_update_leds,
};

static const struct udevice_id i8042_kbd_ids[] = {
	{ .compatible = "intel,i8042-keyboard" },
	{ }
};

U_BOOT_DRIVER(i8042_kbd) = {
	.name	= "i8042_kbd",
	.id	= UCLASS_KEYBOARD,
	.of_match = i8042_kbd_ids,
	.probe = i8042_kbd_probe,
	.remove = i8042_kbd_remove,
	.ops	= &i8042_kbd_ops,
	.priv_auto_alloc_size = sizeof(struct i8042_kbd_priv),
};
