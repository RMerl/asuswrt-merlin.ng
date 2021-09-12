// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2018-2020 Jason A. Donenfeld <Jason@zx2c4.com>. All Rights Reserved.
 */

#include <stdio.h>
#include <sys/stat.h>
static FILE *hacked_userspace_interface_file(const char *iface);
#define stat(a, b) ({ return hacked_userspace_interface_file(iface); 0; })
#define RUNSTATEDIR "/var/empty"
#include "../curve25519.c"
#undef __linux__
#include "../ipc.c"
#include "../encoding.c"

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

const char *__asan_default_options()
{
	return "verbosity=1";
}

union hackiface {
	char ifname[IFNAMSIZ];
	struct {
		const uint8_t *data;
		size_t len;
	};
};

static FILE *hacked_userspace_interface_file(const char *iface)
{
	union hackiface *hack = (union hackiface *)iface;
	FILE *f = fmemopen(NULL, hack->len + 7, "r+");
	fseek(f, 7, SEEK_SET);
	fwrite(hack->data, hack->len, 1, f);
	fseek(f, 0, SEEK_SET);
	memcpy(hack->ifname, "hack", 5);
	return f;
}

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t len)
{
	union hackiface hack = {
		.data = data,
		.len = len
	};
	struct wgdevice *dev = NULL;

	userspace_get_device(&dev, (const char *)&hack);
	free_wgdevice(dev);
	return 0;
}
