// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2015  Intel Corporation. All rights reserved.
 *
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define _GNU_SOURCE
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include "peripheral/log.h"

static int kmsg_fd = -1;

void log_open(void)
{
	if (kmsg_fd >= 0)
		return;

	kmsg_fd = open("/dev/kmsg", O_WRONLY | O_NOCTTY | O_CLOEXEC);
	if (kmsg_fd < 0) {
		fprintf(stderr, "Failed to open kernel logging: %m\n");
		return;
	}
}

void log_close(void)
{
	if (kmsg_fd < 0)
		return;

	close(kmsg_fd);
	kmsg_fd = -1;
}
