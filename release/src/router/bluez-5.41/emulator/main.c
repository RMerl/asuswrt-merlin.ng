/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2011-2014  Intel Corporation
 *  Copyright (C) 2004-2010  Marcel Holtmann <marcel@holtmann.org>
 *
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <getopt.h>

#include "src/shared/mainloop.h"
#include "serial.h"
#include "server.h"
#include "vhci.h"
#include "amp.h"
#include "le.h"

static void signal_callback(int signum, void *user_data)
{
	switch (signum) {
	case SIGINT:
	case SIGTERM:
		mainloop_quit();
		break;
	}
}

static void usage(void)
{
	printf("btvirt - Bluetooth emulator\n"
		"Usage:\n");
	printf("\tbtvirt [options]\n");
	printf("options:\n"
		"\t-S                    Create local serial port\n"
		"\t-s                    Create local server sockets\n"
		"\t-l [num]              Number of local controllers\n"
		"\t-L                    Create LE only controller\n"
		"\t-B                    Create BR/EDR only controller\n"
		"\t-A                    Create AMP controller\n"
		"\t-h, --help            Show help options\n");
}

static const struct option main_options[] = {
	{ "serial",  no_argument,       NULL, 'S' },
	{ "server",  no_argument,       NULL, 's' },
	{ "local",   optional_argument, NULL, 'l' },
	{ "le",      no_argument,       NULL, 'L' },
	{ "bredr",   no_argument,       NULL, 'B' },
	{ "amp",     no_argument,       NULL, 'A' },
	{ "letest",  optional_argument, NULL, 'U' },
	{ "amptest", optional_argument, NULL, 'T' },
	{ "version", no_argument,	NULL, 'v' },
	{ "help",    no_argument,	NULL, 'h' },
	{ }
};

int main(int argc, char *argv[])
{
	struct server *server1;
	struct server *server2;
	struct server *server3;
	struct server *server4;
	struct server *server5;
	bool server_enabled = false;
	bool serial_enabled = false;
	int letest_count = 0;
	int amptest_count = 0;
	int vhci_count = 0;
	enum vhci_type vhci_type = VHCI_TYPE_BREDRLE;
	sigset_t mask;
	int i;

	mainloop_init();

	for (;;) {
		int opt;

		opt = getopt_long(argc, argv, "Ssl::LBAUTvh",
						main_options, NULL);
		if (opt < 0)
			break;

		switch (opt) {
		case 'S':
			serial_enabled = true;
			break;
		case 's':
			server_enabled = true;
			break;
		case 'l':
			if (optarg)
				vhci_count = atoi(optarg);
			else
				vhci_count = 1;
			break;
		case 'L':
			vhci_type = VHCI_TYPE_LE;
			break;
		case 'B':
			vhci_type = VHCI_TYPE_BREDR;
			break;
		case 'A':
			vhci_type = VHCI_TYPE_AMP;
			break;
		case 'U':
			if (optarg)
				letest_count = atoi(optarg);
			else
				letest_count = 1;
			break;
		case 'T':
			if (optarg)
				amptest_count = atoi(optarg);
			else
				amptest_count = 1;
			break;
		case 'v':
			printf("%s\n", VERSION);
			return EXIT_SUCCESS;
		case 'h':
			usage();
			return EXIT_SUCCESS;
		default:
			return EXIT_FAILURE;
		}
	}

	if (letest_count < 1 && amptest_count < 1 &&
			vhci_count < 1 && !server_enabled && !serial_enabled) {
		fprintf(stderr, "No emulator specified\n");
		return EXIT_FAILURE;
	}

	sigemptyset(&mask);
	sigaddset(&mask, SIGINT);
	sigaddset(&mask, SIGTERM);

	mainloop_set_signal(&mask, signal_callback, NULL, NULL);

	printf("Bluetooth emulator ver %s\n", VERSION);

	for (i = 0; i < letest_count; i++) {
		struct bt_le *le;

		le = bt_le_new();
		if (!le) {
			fprintf(stderr, "Failed to create LE controller\n");
			return EXIT_FAILURE;
		}
	}

	for (i = 0; i < amptest_count; i++) {
		struct bt_amp *amp;

		amp = bt_amp_new();
		if (!amp) {
			fprintf(stderr, "Failed to create AMP controller\n");
			return EXIT_FAILURE;
		}
	}

	for (i = 0; i < vhci_count; i++) {
		struct vhci *vhci;

		vhci = vhci_open(vhci_type);
		if (!vhci) {
			fprintf(stderr, "Failed to open Virtual HCI device\n");
			return EXIT_FAILURE;
		}
	}

	if (serial_enabled) {
		struct serial *serial;

		serial = serial_open(SERIAL_TYPE_BREDRLE);
		if (!serial)
			fprintf(stderr, "Failed to open serial emulation\n");
	}

	if (server_enabled) {
		server1 = server_open_unix(SERVER_TYPE_BREDRLE,
						"/tmp/bt-server-bredrle");
		if (!server1)
			fprintf(stderr, "Failed to open BR/EDR/LE server\n");

		server2 = server_open_unix(SERVER_TYPE_BREDR,
						"/tmp/bt-server-bredr");
		if (!server2)
			fprintf(stderr, "Failed to open BR/EDR server\n");

		server3 = server_open_unix(SERVER_TYPE_AMP,
						"/tmp/bt-server-amp");
		if (!server3)
			fprintf(stderr, "Failed to open AMP server\n");

		server4 = server_open_unix(SERVER_TYPE_LE,
						"/tmp/bt-server-le");
		if (!server4)
			fprintf(stderr, "Failed to open LE server\n");

		server5 = server_open_unix(SERVER_TYPE_MONITOR,
						"/tmp/bt-server-mon");
		if (!server5)
			fprintf(stderr, "Failed to open monitor server\n");
	}

	return mainloop_run();
}
