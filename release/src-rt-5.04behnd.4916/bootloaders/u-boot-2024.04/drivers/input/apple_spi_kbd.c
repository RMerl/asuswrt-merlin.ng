// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2021 Mark Kettenis <kettenis@openbsd.org>
 */

#include <common.h>
#include <dm.h>
#include <keyboard.h>
#include <spi.h>
#include <stdio_dev.h>
#include <asm-generic/gpio.h>
#include <linux/delay.h>
#include <linux/input.h>

/*
 * The Apple SPI keyboard controller implements a protocol that
 * closely resembles HID Keyboard Boot protocol.  The key codes are
 * mapped according to the HID Keyboard/Keypad Usage Table.
 */

/* Modifier key bits */
#define HID_MOD_LEFTCTRL	BIT(0)
#define HID_MOD_LEFTSHIFT	BIT(1)
#define HID_MOD_LEFTALT		BIT(2)
#define HID_MOD_LEFTGUI		BIT(3)
#define HID_MOD_RIGHTCTRL	BIT(4)
#define HID_MOD_RIGHTSHIFT	BIT(5)
#define HID_MOD_RIGHTALT	BIT(6)
#define HID_MOD_RIGHTGUI	BIT(7)

static const u8 hid_kbd_keymap[] = {
	KEY_RESERVED, 0xff, 0xff, 0xff,
	KEY_A, KEY_B, KEY_C, KEY_D,
	KEY_E, KEY_F, KEY_G, KEY_H,
	KEY_I, KEY_J, KEY_K, KEY_L,
	KEY_M, KEY_N, KEY_O, KEY_P,
	KEY_Q, KEY_R, KEY_S, KEY_T,
	KEY_U, KEY_V, KEY_W, KEY_X,
	KEY_Y, KEY_Z, KEY_1, KEY_2,
	KEY_3, KEY_4, KEY_5, KEY_6,
	KEY_7, KEY_8, KEY_9, KEY_0,
	KEY_ENTER, KEY_ESC, KEY_BACKSPACE, KEY_TAB,
	KEY_SPACE, KEY_MINUS, KEY_EQUAL, KEY_LEFTBRACE,
	KEY_RIGHTBRACE, KEY_BACKSLASH, 0xff, KEY_SEMICOLON,
	KEY_APOSTROPHE, KEY_GRAVE, KEY_COMMA, KEY_DOT,
	KEY_SLASH, KEY_CAPSLOCK, KEY_F1, KEY_F2,
	KEY_F3, KEY_F4, KEY_F5, KEY_F6,
	KEY_F7, KEY_F8, KEY_F9, KEY_F10,
	KEY_F11, KEY_F12, KEY_SYSRQ, KEY_SCROLLLOCK,
	KEY_PAUSE, KEY_INSERT, KEY_HOME, KEY_PAGEUP,
	KEY_DELETE, KEY_END, KEY_PAGEDOWN, KEY_RIGHT,
	KEY_LEFT, KEY_DOWN, KEY_UP, KEY_NUMLOCK,
	KEY_KPSLASH, KEY_KPASTERISK, KEY_KPMINUS, KEY_KPPLUS,
	KEY_KPENTER, KEY_KP1, KEY_KP2, KEY_KP3,
	KEY_KP4, KEY_KP5, KEY_KP6, KEY_KP7,
	KEY_KP8, KEY_KP9, KEY_KP0, KEY_KPDOT,
	KEY_BACKSLASH, KEY_COMPOSE, KEY_POWER, KEY_KPEQUAL,
};

/* Report ID used for keyboard input reports. */
#define KBD_REPORTID	0x01

struct apple_spi_kbd_report {
	u8 reportid;
	u8 modifiers;
	u8 reserved;
	u8 keycode[6];
	u8 fn;
};

struct apple_spi_kbd_priv {
	struct gpio_desc enable;
	struct apple_spi_kbd_report old; /* previous keyboard input report */
	struct apple_spi_kbd_report new; /* current keyboard input report */
};

/* Keyboard device. */
#define KBD_DEVICE	0x01

/* The controller sends us fixed-size packets of 256 bytes. */
struct apple_spi_kbd_packet {
	u8 flags;
#define PACKET_READ	0x20
	u8 device;
	u16 offset;
	u16 remaining;
	u16 len;
	u8 data[246];
	u16 crc;
};

/* Packets contain a single variable-sized message. */
struct apple_spi_kbd_msg {
	u8 type;
#define MSG_REPORT	0x10
	u8 device;
	u8 unknown;
	u8 msgid;
	u16 rsplen;
	u16 cmdlen;
	u8 data[0];
};

static void apple_spi_kbd_service_modifiers(struct input_config *input)
{
	struct apple_spi_kbd_priv *priv = dev_get_priv(input->dev);
	u8 new = priv->new.modifiers;
	u8 old = priv->old.modifiers;

	if ((new ^ old) & HID_MOD_LEFTCTRL)
		input_add_keycode(input, KEY_LEFTCTRL,
				  old & HID_MOD_LEFTCTRL);
	if ((new ^ old) & HID_MOD_RIGHTCTRL)
		input_add_keycode(input, KEY_RIGHTCTRL,
				  old & HID_MOD_RIGHTCTRL);
	if ((new ^ old) & HID_MOD_LEFTSHIFT)
		input_add_keycode(input, KEY_LEFTSHIFT,
				  old & HID_MOD_LEFTSHIFT);
	if ((new ^ old) & HID_MOD_RIGHTSHIFT)
		input_add_keycode(input, KEY_RIGHTSHIFT,
				  old & HID_MOD_RIGHTSHIFT);
	if ((new ^ old) & HID_MOD_LEFTALT)
		input_add_keycode(input, KEY_LEFTALT,
				  old & HID_MOD_LEFTALT);
	if ((new ^ old) & HID_MOD_RIGHTALT)
		input_add_keycode(input, KEY_RIGHTALT,
				  old & HID_MOD_RIGHTALT);
	if ((new ^ old) & HID_MOD_LEFTGUI)
		input_add_keycode(input, KEY_LEFTMETA,
				  old & HID_MOD_LEFTGUI);
	if ((new ^ old) & HID_MOD_RIGHTGUI)
		input_add_keycode(input, KEY_RIGHTMETA,
				  old & HID_MOD_RIGHTGUI);
}

static void apple_spi_kbd_service_key(struct input_config *input, int i,
				      int released)
{
	struct apple_spi_kbd_priv *priv = dev_get_priv(input->dev);
	u8 *new;
	u8 *old;

	if (released) {
		new = priv->new.keycode;
		old = priv->old.keycode;
	} else {
		new = priv->old.keycode;
		old = priv->new.keycode;
	}

	if (memscan(new, old[i], sizeof(priv->new.keycode)) ==
	    new + sizeof(priv->new.keycode) &&
	    old[i] < ARRAY_SIZE(hid_kbd_keymap))
		input_add_keycode(input, hid_kbd_keymap[old[i]], released);
}

static int apple_spi_kbd_check(struct input_config *input)
{
	struct udevice *dev = input->dev;
	struct apple_spi_kbd_priv *priv = dev_get_priv(dev);
	struct apple_spi_kbd_packet packet;
	struct apple_spi_kbd_msg *msg;
	struct apple_spi_kbd_report *report;
	int i, ret;

	memset(&packet, 0, sizeof(packet));

	ret = dm_spi_claim_bus(dev);
	if (ret < 0)
		return ret;

	/*
	 * The keyboard controller needs delays after asserting CS#
	 * and before deasserting CS#.
	 */
	ret = dm_spi_xfer(dev, 0, NULL, NULL, SPI_XFER_BEGIN);
	if (ret < 0)
		goto fail;
	udelay(100);
	ret = dm_spi_xfer(dev, sizeof(packet) * 8, NULL, &packet, 0);
	if (ret < 0)
		goto fail;
	udelay(100);
	ret = dm_spi_xfer(dev, 0, NULL, NULL, SPI_XFER_END);
	if (ret < 0)
		goto fail;

	dm_spi_release_bus(dev);

	/*
	 * The keyboard controller needs a delay between subsequent
	 * SPI transfers.
	 */
	udelay(250);

	msg = (struct apple_spi_kbd_msg *)packet.data;
	report = (struct apple_spi_kbd_report *)msg->data;
	if (packet.flags == PACKET_READ && packet.device == KBD_DEVICE &&
	    msg->type == MSG_REPORT && msg->device == KBD_DEVICE &&
	    msg->cmdlen == sizeof(struct apple_spi_kbd_report) &&
	    report->reportid == KBD_REPORTID) {
		memcpy(&priv->new, report,
		       sizeof(struct apple_spi_kbd_report));
		apple_spi_kbd_service_modifiers(input);
		for (i = 0; i < sizeof(priv->new.keycode); i++) {
			apple_spi_kbd_service_key(input, i, 1);
			apple_spi_kbd_service_key(input, i, 0);
		}
		memcpy(&priv->old, &priv->new,
		       sizeof(struct apple_spi_kbd_report));
		return 1;
	}

	return 0;

fail:
	/*
	 * Make sure CS# is deasserted. If this fails there is nothing
	 * we can do, so ignore any errors.
	 */
	dm_spi_xfer(dev, 0, NULL, NULL, SPI_XFER_END);
	dm_spi_release_bus(dev);
	return ret;
}

static int apple_spi_kbd_probe(struct udevice *dev)
{
	struct apple_spi_kbd_priv *priv = dev_get_priv(dev);
	struct keyboard_priv *uc_priv = dev_get_uclass_priv(dev);
	struct stdio_dev *sdev = &uc_priv->sdev;
	struct input_config *input = &uc_priv->input;
	int ret;

	ret = gpio_request_by_name(dev, "spien-gpios", 0, &priv->enable,
				   GPIOD_IS_OUT);
	if (ret < 0)
		return ret;

	/* Reset the keyboard controller. */
	dm_gpio_set_value(&priv->enable, 1);
	udelay(5000);
	dm_gpio_set_value(&priv->enable, 0);
	udelay(5000);

	/* Enable the keyboard controller. */
	dm_gpio_set_value(&priv->enable, 1);

	input->dev = dev;
	input->read_keys = apple_spi_kbd_check;
	input_add_tables(input, false);
	strcpy(sdev->name, "spikbd");

	return input_stdio_register(sdev);
}

static const struct keyboard_ops apple_spi_kbd_ops = {
};

static const struct udevice_id apple_spi_kbd_of_match[] = {
	{ .compatible = "apple,spi-hid-transport" },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(apple_spi_kbd) = {
	.name = "apple_spi_kbd",
	.id = UCLASS_KEYBOARD,
	.of_match = apple_spi_kbd_of_match,
	.probe = apple_spi_kbd_probe,
	.priv_auto = sizeof(struct apple_spi_kbd_priv),
	.ops = &apple_spi_kbd_ops,
};
