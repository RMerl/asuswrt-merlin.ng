// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2002-2010  Marcel Holtmann <marcel@holtmann.org>
 *
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <getopt.h>
#include <sys/socket.h>

#include "lib/bluetooth.h"
#include "lib/hci.h"
#include "lib/hci_lib.h"

static struct option main_options[] = {
	{ "device",	1, 0, 'i' },
	{ 0, 0, 0, 0 }
};

int main(int argc, char *argv[])
{
	uint8_t events[8] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0x1f, 0x00, 0x00 };
	struct hci_dev_info di;
	struct hci_version ver;
	int dd, opt, dev = 0;

	while ((opt=getopt_long(argc, argv, "+i:", main_options, NULL)) != -1) {
		switch (opt) {
		case 'i':
			dev = hci_devid(optarg);
			if (dev < 0) {
				perror("Invalid device");
				exit(1);
			}
			break;
		}
	}

	dd = hci_open_dev(dev);
	if (dd < 0) {
		fprintf(stderr, "Can't open device hci%d: %s (%d)\n",
						dev, strerror(errno), errno);
		exit(1);
	}

	if (hci_devinfo(dev, &di) < 0) {
		fprintf(stderr, "Can't get device info for hci%d: %s (%d)\n",
						dev, strerror(errno), errno);
		hci_close_dev(dd);
		exit(1);
	}

	if (hci_read_local_version(dd, &ver, 1000) < 0) {
		fprintf(stderr, "Can't read version info for hci%d: %s (%d)\n",
						dev, strerror(errno), errno);
		hci_close_dev(dd);
		exit(1);
	}

	hci_close_dev(dd);

	if (ver.hci_ver > 1) {
		if (di.features[5] & LMP_SNIFF_SUBR)
			events[5] |= 0x20;

		if (di.features[5] & LMP_PAUSE_ENC)
			events[5] |= 0x80;

		if (di.features[6] & LMP_EXT_INQ)
			events[5] |= 0x40;

		if (di.features[6] & LMP_NFLUSH_PKTS)
			events[7] |= 0x01;

		if (di.features[7] & LMP_LSTO)
			events[6] |= 0x80;

		if (di.features[6] & LMP_SIMPLE_PAIR) {
			events[6] |= 0x01;	/* IO Capability Request */
			events[6] |= 0x02;	/* IO Capability Response */
			events[6] |= 0x04;	/* User Confirmation Request */
			events[6] |= 0x08;	/* User Passkey Request */
			events[6] |= 0x10;	/* Remote OOB Data Request */
			events[6] |= 0x20;	/* Simple Pairing Complete */
			events[7] |= 0x04;	/* User Passkey Notification */
			events[7] |= 0x08;	/* Keypress Notification */
			events[7] |= 0x10;	/* Remote Host Supported
						 * Features Notification */
		}

		if (di.features[4] & LMP_LE)
			events[7] |= 0x20;

		if (di.features[6] & LMP_LE_BREDR)
			events[7] |= 0x20;
	}

	printf("Setting event mask:\n");
	printf("\thcitool cmd 0x%02x 0x%04x  "
					"0x%02x 0x%02x 0x%02x 0x%02x "
					"0x%02x 0x%02x 0x%02x 0x%02x\n",
				OGF_HOST_CTL, OCF_SET_EVENT_MASK,
				events[0], events[1], events[2], events[3],
				events[4], events[5], events[6], events[7]);

	return 0;
}
