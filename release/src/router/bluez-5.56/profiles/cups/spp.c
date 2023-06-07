// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2003-2010  Marcel Holtmann <marcel@holtmann.org>
 *
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>

#include "lib/bluetooth.h"
#include "lib/rfcomm.h"
#include "lib/sdp.h"
#include "lib/sdp_lib.h"

#include "cups.h"

int spp_print(bdaddr_t *src, bdaddr_t *dst, uint8_t channel, int fd, int copies, const char *cups_class)
{
	struct sockaddr_rc addr;
	unsigned char buf[2048];
	int i, sk, err, len;

	if ((sk = socket(PF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM)) < 0) {
		perror("ERROR: Can't create socket");
		if (cups_class)
			return CUPS_BACKEND_FAILED;
		else
			return CUPS_BACKEND_RETRY;
	}

	addr.rc_family = AF_BLUETOOTH;
	bacpy(&addr.rc_bdaddr, src);
	addr.rc_channel = 0;

	if (bind(sk, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		perror("ERROR: Can't bind socket");
		close(sk);
		if (cups_class)
			return CUPS_BACKEND_FAILED;
		else
			return CUPS_BACKEND_RETRY;
	}

	addr.rc_family = AF_BLUETOOTH;
	bacpy(&addr.rc_bdaddr, dst);
	addr.rc_channel = channel;

	if (connect(sk, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		perror("ERROR: Can't connect to device");
		close(sk);
		if (cups_class)
			return CUPS_BACKEND_FAILED;
		else
			return CUPS_BACKEND_RETRY;
	}

	fputs("STATE: -connecting-to-device\n", stderr);

	/* Ignore SIGTERM signals if printing from stdin */
	if (fd == 0) {
#ifdef HAVE_SIGSET
		sigset(SIGTERM, SIG_IGN);
#elif defined(HAVE_SIGACTION)
		memset(&action, 0, sizeof(action));
		sigemptyset(&action.sa_mask);
		action.sa_handler = SIG_IGN;
		sigaction(SIGTERM, &action, NULL);
#else
		signal(SIGTERM, SIG_IGN);
#endif /* HAVE_SIGSET */
	}

	for (i = 0; i < copies; i++) {

		if (fd != 0) {
			fprintf(stderr, "PAGE: 1 1\n");
			lseek(fd, 0, SEEK_SET);
		}

		while ((len = read(fd, buf, sizeof(buf))) > 0) {
			err = write(sk, buf, len);
			if (err < 0) {
				perror("ERROR: Error writing to device");
				close(sk);
				return CUPS_BACKEND_FAILED;
			}
		}

	}

	close(sk);

	return CUPS_BACKEND_OK;
}
