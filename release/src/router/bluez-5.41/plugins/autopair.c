/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2012 Google Inc.
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdbool.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include <glib.h>

#include "lib/bluetooth.h"
#include "lib/sdp.h"

#include "src/plugin.h"
#include "src/adapter.h"
#include "src/device.h"
#include "src/log.h"
#include "src/storage.h"

/*
 * Plugin to handle automatic pairing of devices with reduced user
 * interaction, including implementing the recommendation of the HID spec
 * for keyboard devices.
 *
 * The plugin works by intercepting the PIN request for devices; if the
 * device is a keyboard a random six-digit numeric PIN is generated and
 * returned, flagged for displaying using DisplayPinCode.
 *
 */

static ssize_t autopair_pincb(struct btd_adapter *adapter,
						struct btd_device *device,
						char *pinbuf, bool *display,
						unsigned int attempt)
{
	char addr[18];
	char pinstr[7];
	uint32_t class;

	ba2str(device_get_address(device), addr);

	class = btd_device_get_class(device);

	DBG("device %s 0x%x", addr, class);

	/* This is a class-based pincode guesser. Ignore devices with an
	 * unknown class.
	 */
	if (class == 0)
		return 0;

	switch ((class & 0x1f00) >> 8) {
	case 0x04:		/* Audio/Video */
		switch ((class & 0xfc) >> 2) {
		case 0x01:		/* Wearable Headset Device */
		case 0x02:		/* Hands-free Device */
		case 0x06:		/* Headphones */
		case 0x07:		/* Portable Audio */
		case 0x0a:		/* HiFi Audio Device */
			if (attempt > 1)
				return 0;
			memcpy(pinbuf, "0000", 4);
			return 4;
		}
		break;

	case 0x05:		/* Peripheral */
		switch ((class & 0xc0) >> 6) {
		case 0x01:		/* Keyboard */
		case 0x03:		/* Combo keyboard/pointing device */
			/* For keyboards rejecting the first random code
			 * in less than 500ms, try a fixed code. */
			if (attempt > 1 &&
				device_bonding_last_duration(device) < 500) {
				/* Don't try more than one dumb code */
				if (attempt > 2)
					return 0;
				/* Try "0000" as the code for the second
				 * attempt. */
				memcpy(pinbuf, "0000", 4);
				return 4;
			}

			/* Never try more than 3 random pincodes. */
			if (attempt >= 4)
				return 0;

			snprintf(pinstr, sizeof(pinstr), "%06d",
						rand() % 1000000);
			*display = true;
			memcpy(pinbuf, pinstr, 6);
			return 6;

		case 0x02: /* Pointing device */
			if (attempt > 1)
				return 0;
			memcpy(pinbuf, "0000", 4);
			return 4;
		}

		break;
	case 0x06:		/* Imaging */
		if (class & 0x80) {	/* Printer */
			if (attempt > 1)
				return 0;
			memcpy(pinbuf, "0000", 4);
			return 4;
		}
		break;
	}

	return 0;
}


static int autopair_probe(struct btd_adapter *adapter)
{
	btd_adapter_register_pin_cb(adapter, autopair_pincb);

	return 0;
}

static void autopair_remove(struct btd_adapter *adapter)
{
	btd_adapter_unregister_pin_cb(adapter, autopair_pincb);
}

static struct btd_adapter_driver autopair_driver = {
	.name = "autopair",
	.probe = autopair_probe,
	.remove = autopair_remove,
};

static int autopair_init(void)
{
	/* Initialize the random seed from /dev/urandom */
	unsigned int seed;
	int fd, err;
	ssize_t n;

	fd = open("/dev/urandom", O_RDONLY);
	if (fd < 0) {
		err = -errno;
		error("Failed to open /dev/urandom: %s (%d)", strerror(-err),
									-err);
		return err;
	}

	n = read(fd, &seed, sizeof(seed));
	if (n < (ssize_t) sizeof(seed)) {
		err = (n == -1) ? -errno : -EIO;
		error("Failed to read %zu bytes from /dev/urandom: %s (%d)",
					sizeof(seed), strerror(-err), -err);
		close(fd);
		return err;
	}

	close(fd);

	srand(seed);

	return btd_register_adapter_driver(&autopair_driver);
}

static void autopair_exit(void)
{
	btd_unregister_adapter_driver(&autopair_driver);
}

BLUETOOTH_PLUGIN_DEFINE(autopair, VERSION, BLUETOOTH_PLUGIN_PRIORITY_DEFAULT,
						autopair_init, autopair_exit)
