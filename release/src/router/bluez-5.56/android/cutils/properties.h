/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2013  Intel Corporation. All rights reserved.
 *
 *
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>

#define PROPERTY_VALUE_MAX 32
#define PROPERTY_KEY_MAX 32

#define BLUETOOTH_MODE_PROPERTY_NAME "persist.sys.bluetooth.mode"
#define BLUETOOTH_MODE_PROPERTY_HANDSFREE "persist.sys.bluetooth.handsfree"

static inline int property_get(const char *key, char *value,
						const char *default_value)
{
	const char *prop = NULL;

	if (!strcmp(key, BLUETOOTH_MODE_PROPERTY_NAME))
		prop = getenv("BLUETOOTH_MODE");

	if (!strcmp(key, BLUETOOTH_MODE_PROPERTY_HANDSFREE))
		prop = getenv("BLUETOOTH_HANDSFREE_MODE");

	if (!prop)
		prop = default_value;

	if (prop) {
		strncpy(value, prop, PROPERTY_VALUE_MAX);

		value[PROPERTY_VALUE_MAX - 1] = '\0';

		return strlen(value);
	}

	return 0;
}

/* property_set: returns 0 on success, < 0 on failure
*/
static inline int property_set(const char *key, const char *value)
{
	static const char SYSTEM_SOCKET_PATH[] = "\0android_system";

	struct sockaddr_un addr;
	char msg[256];
	int fd, len;

	fd = socket(PF_LOCAL, SOCK_DGRAM, 0);
	if (fd < 0)
		return -1;

	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	memcpy(addr.sun_path, SYSTEM_SOCKET_PATH, sizeof(SYSTEM_SOCKET_PATH));

	if (connect(fd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		close(fd);
		return 0;
	}

	len = snprintf(msg, sizeof(msg), "%s=%s", key, value);

	if (send(fd, msg, len + 1, 0) < 0) {
		close(fd);
		return -1;
	}

	close(fd);

	return 0;
}
