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
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "src/shared/mainloop.h"
#include "src/shared/hfp.h"

static void hfp_debug(const char *str, void *user_data)
{
	const char *prefix = user_data;

	printf("%s%s\n", prefix, str);
}

static void command_handler(const char *command, void *user_data)
{
	struct hfp_gw *hfp = user_data;

	printf("Command: %s\n", command);

	hfp_gw_send_result(hfp, HFP_RESULT_ERROR);
}

static bool open_connection(void)
{
	static const char SOCKET_PATH[] = "\0hfp-headset";
	struct hfp_gw *hfp;
	struct sockaddr_un addr;
	int fd;

	fd = socket(PF_LOCAL, SOCK_STREAM | SOCK_CLOEXEC, 0);
	if (fd < 0) {
		perror("Failed to create Unix socket");
		return false;
	}

	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	memcpy(addr.sun_path, SOCKET_PATH, sizeof(SOCKET_PATH));

	if (connect(fd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		perror("Failed to connect Unix socket");
		close(fd);
		return false;
	}

	hfp = hfp_gw_new(fd);
	if (!hfp) {
		close(fd);
		return false;
	}

	hfp_gw_set_close_on_unref(hfp, true);

	hfp_gw_set_debug(hfp, hfp_debug, "HFP: ", NULL);

	hfp_gw_set_command_handler(hfp, command_handler, hfp, NULL);

	return true;
}

static void signal_callback(int signum, void *user_data)
{
	switch (signum) {
	case SIGINT:
	case SIGTERM:
		mainloop_quit();
		break;
	}
}

int main(int argc, char *argv[])
{
	mainloop_init();

	if (!open_connection())
		return EXIT_FAILURE;

	return mainloop_run_with_signal(signal_callback, NULL);
}
