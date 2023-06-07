// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2011-2012  Intel Corporation
 *  Copyright (C) 2004-2010  Marcel Holtmann <marcel@holtmann.org>
 *
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <errno.h>
#include <getopt.h>
#include <stdbool.h>

#include "src/shared/mainloop.h"
#include "src/shared/util.h"
#include "src/shared/mgmt.h"

static struct mgmt *mgmt = NULL;

static void mgmt_debug(const char *str, void *user_data)
{
	const char *prefix = user_data;

	printf("%s%s\n", prefix, str);
}

static void signal_callback(int signum, void *user_data)
{
	static bool terminated = false;

	switch (signum) {
	case SIGINT:
	case SIGTERM:
		if (!terminated) {
			mainloop_quit();
			terminated = true;
		}
		break;
	}
}
static void usage(void)
{
	printf("btconfig - Bluetooth configuration utility\n"
		"Usage:\n");
	printf("\tbtconfig [options]\n");
	printf("options:\n"
		"\t-h, --help             Show help options\n");
}

static const struct option main_options[] = {
	{ "version",  no_argument,       NULL, 'v' },
	{ "help",     no_argument,       NULL, 'h' },
	{ }
};

int main(int argc, char *argv[])
{
	int exit_status;

	for (;;) {
		int opt;

		opt = getopt_long(argc, argv, "vh",
						main_options, NULL);
		if (opt < 0)
			break;

		switch (opt) {
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

	if (argc - optind > 0) {
		fprintf(stderr, "Invalid command line parameters\n");
		return EXIT_FAILURE;
	}

	mainloop_init();

	mgmt = mgmt_new_default();
	if (!mgmt) {
		fprintf(stderr, "Unable to open mgmt_socket\n");
		return EXIT_FAILURE;
	}

	if (getenv("MGMT_DEBUG"))
		mgmt_set_debug(mgmt, mgmt_debug, "mgmt: ", NULL);

	exit_status = mainloop_run_with_signal(signal_callback, NULL);

	mgmt_cancel_all(mgmt);
	mgmt_unregister_all(mgmt);

	mgmt_unref(mgmt);

	return exit_status;
}
