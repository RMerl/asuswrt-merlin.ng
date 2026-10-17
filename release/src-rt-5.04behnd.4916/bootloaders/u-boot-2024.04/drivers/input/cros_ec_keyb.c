// SPDX-License-Identifier: GPL-2.0+
/*
 * Chromium OS Matrix Keyboard
 *
 * Copyright (c) 2012 The Chromium OS Authors.
 */

#include <common.h>
#include <cros_ec.h>
#include <dm.h>
#include <errno.h>
#include <input.h>
#include <keyboard.h>
#include <key_matrix.h>
#include <log.h>
#include <stdio_dev.h>

enum {
	KBC_MAX_KEYS		= 8,	/* Maximum keys held down at once */
	KBC_REPEAT_RATE_MS	= 30,
	KBC_REPEAT_DELAY_MS	= 240,
};

struct cros_ec_keyb_priv {
	struct input_config *input;	/* The input layer */
	struct key_matrix matrix;	/* The key matrix layer */
	int key_rows;			/* Number of keyboard rows */
	int key_cols;			/* Number of keyboard columns */
	int ghost_filter;		/* 1 to enable ghost filter, else 0 */
};


/**
 * Check the keyboard controller and return a list of key matrix positions
 * for which a key is pressed
 *
 * @param dev		Keyboard device
 * @param keys		List of keys that we have detected
 * @param max_count	Maximum number of keys to return
 * @param samep		Set to true if this scan repeats the last, else false
 * Return: number of pressed keys, 0 for none, -EIO on error
 */
static int check_for_keys(struct udevice *dev, struct key_matrix_key *keys,
			  int max_count, bool *samep)
{
	struct cros_ec_keyb_priv *priv = dev_get_priv(dev);
	struct key_matrix_key *key;
	static struct mbkp_keyscan last_scan;
	static bool last_scan_valid;
	struct ec_response_get_next_event event;
	struct mbkp_keyscan *scan = (struct mbkp_keyscan *)
				    &event.data.key_matrix;
	unsigned int row, col, bit, data;
	int num_keys;
	int ret;

	/* Get pending MKBP event. It may not be a key matrix event. */
	do {
		ret = cros_ec_get_next_event(dev->parent, &event);
		/* The EC has no events for us at this time. */
		if (ret == -EC_RES_UNAVAILABLE)
			return -EIO;
		else if (ret)
			break;
	} while (event.event_type != EC_MKBP_EVENT_KEY_MATRIX);

	/* Try the old command if the EC doesn't support the above. */
	if (ret == -EC_RES_INVALID_COMMAND) {
		if (cros_ec_scan_keyboard(dev->parent, scan)) {
			debug("%s: keyboard scan failed\n", __func__);
			return -EIO;
		}
	} else if (ret) {
		debug("%s: Error getting next MKBP event. (%d)\n",
		      __func__, ret);
		return -EIO;
	}
	*samep = last_scan_valid && !memcmp(&last_scan, scan, sizeof(*scan));

	/*
	 * This is a bit odd. The EC has no way to tell us that it has run
	 * out of key scans. It just returns the same scan over and over
	 * again. So the only way to detect that we have run out is to detect
	 * that this scan is the same as the last.
	 */
	last_scan_valid = true;
	memcpy(&last_scan, scan, sizeof(last_scan));

	for (col = num_keys = bit = 0; col < priv->matrix.num_cols;
			col++) {
		for (row = 0; row < priv->matrix.num_rows; row++) {
			unsigned int mask = 1 << (bit & 7);

			data = scan->data[bit / 8];
			if ((data & mask) && num_keys < max_count) {
				key = keys + num_keys++;
				key->row = row;
				key->col = col;
				key->valid = 1;
			}
			bit++;
		}
	}

	return num_keys;
}

/**
 * Check the keyboard, and send any keys that are pressed.
 *
 * This is called by input_tstc() and input_getc() when they need more
 * characters
 *
 * @param input		Input configuration
 * Return: 1, to indicate that we have something to look at
 */
int cros_ec_kbc_check(struct input_config *input)
{
	struct udevice *dev = input->dev;
	struct cros_ec_keyb_priv *priv = dev_get_priv(dev);
	static struct key_matrix_key last_keys[KBC_MAX_KEYS];
	static int last_num_keys;
	struct key_matrix_key keys[KBC_MAX_KEYS];
	int keycodes[KBC_MAX_KEYS];
	int num_keys, num_keycodes;
	int irq_pending, sent;
	bool same = false;

	/*
	 * Loop until the EC has no more keyscan records, or we have
	 * received at least one character. This means we know that tstc()
	 * will always return non-zero if keys have been pressed.
	 *
	 * Without this loop, a key release (which generates no new ascii
	 * characters) will cause us to exit this function, and just tstc()
	 * may return 0 before all keys have been read from the EC.
	 */
	do {
		irq_pending = cros_ec_interrupt_pending(dev->parent);
		if (irq_pending) {
			num_keys = check_for_keys(dev, keys, KBC_MAX_KEYS,
						  &same);
			if (num_keys < 0)
				return 0;
			last_num_keys = num_keys;
			memcpy(last_keys, keys, sizeof(keys));
		} else {
			/*
			 * EC doesn't want to be asked, so use keys from last
			 * time.
			 */
			num_keys = last_num_keys;
			memcpy(keys, last_keys, sizeof(keys));
		}

		if (num_keys < 0)
			return -1;
		num_keycodes = key_matrix_decode(&priv->matrix, keys,
				num_keys, keycodes, KBC_MAX_KEYS);
		sent = input_send_keycodes(input, keycodes, num_keycodes);

		/*
		 * For those ECs without an interrupt, stop scanning when we
		 * see that the scan is the same as last time.
		 */
		if ((irq_pending < 0) && same)
			break;
	} while (irq_pending && !sent);

	return 1;
}

/**
 * Decode MBKP keyboard details from the device tree
 *
 * @param blob		Device tree blob
 * @param node		Node to decode from
 * @param config	Configuration data read from fdt
 * Return: 0 if ok, -1 on error
 */
static int cros_ec_keyb_decode_fdt(struct udevice *dev,
				   struct cros_ec_keyb_priv *config)
{
	/*
	 * Get keyboard rows and columns - at present we are limited to
	 * 8 columns by the protocol (one byte per row scan)
	 */
	config->key_rows = dev_read_u32_default(dev, "keypad,num-rows", 0);
	config->key_cols = dev_read_u32_default(dev, "keypad,num-columns", 0);
	if (!config->key_rows || !config->key_cols ||
			config->key_rows * config->key_cols / 8
				> CROS_EC_KEYSCAN_COLS) {
		debug("%s: Invalid key matrix size %d x %d\n", __func__,
		      config->key_rows, config->key_cols);
		return -1;
	}
	config->ghost_filter = dev_read_bool(dev, "google,needs-ghost-filter");

	return 0;
}

static int cros_ec_kbd_probe(struct udevice *dev)
{
	struct cros_ec_keyb_priv *priv = dev_get_priv(dev);
	struct keyboard_priv *uc_priv = dev_get_uclass_priv(dev);
	struct stdio_dev *sdev = &uc_priv->sdev;
	struct input_config *input = &uc_priv->input;
	int ret;

	ret = cros_ec_keyb_decode_fdt(dev, priv);
	if (ret) {
		debug("%s: Cannot decode node (ret=%d)\n", __func__, ret);
		return -EINVAL;
	}
	input_set_delays(input, KBC_REPEAT_DELAY_MS, KBC_REPEAT_RATE_MS);
	ret = key_matrix_init(&priv->matrix, priv->key_rows, priv->key_cols,
			      priv->ghost_filter);
	if (ret) {
		debug("%s: cannot init key matrix\n", __func__);
		return ret;
	}
	ret = key_matrix_decode_fdt(dev, &priv->matrix);
	if (ret) {
		debug("%s: Could not decode key matrix from fdt\n", __func__);
		return ret;
	}
	debug("%s: Matrix keyboard %dx%d ready\n", __func__, priv->key_rows,
	      priv->key_cols);

	priv->input = input;
	input->dev = dev;
	input_add_tables(input, false);
	input->read_keys = cros_ec_kbc_check;
	strcpy(sdev->name, "cros-ec-keyb");

	/* Register the device. cros_ec_init_keyboard() will be called soon */
	return input_stdio_register(sdev);
}

static const struct keyboard_ops cros_ec_kbd_ops = {
};

static const struct udevice_id cros_ec_kbd_ids[] = {
	{ .compatible = "google,cros-ec-keyb" },
	{ }
};

U_BOOT_DRIVER(google_cros_ec_keyb) = {
	.name	= "google_cros_ec_keyb",
	.id	= UCLASS_KEYBOARD,
	.of_match = cros_ec_kbd_ids,
	.probe = cros_ec_kbd_probe,
	.ops	= &cros_ec_kbd_ops,
	.priv_auto	= sizeof(struct cros_ec_keyb_priv),
};
