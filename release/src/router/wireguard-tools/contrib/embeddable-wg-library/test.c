// SPDX-License-Identifier: LGPL-2.1+
/*
 * Copyright (C) 2015-2020 Jason A. Donenfeld <Jason@zx2c4.com>. All Rights Reserved.
 */

#include "wireguard.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void list_devices(void)
{
	char *device_names, *device_name;
	size_t len;

	device_names = wg_list_device_names();
	if (!device_names) {
		perror("Unable to get device names");
		exit(1);
	}
	wg_for_each_device_name(device_names, device_name, len) {
		wg_device *device;
		wg_peer *peer;
		wg_key_b64_string key;

		if (wg_get_device(&device, device_name) < 0) {
			perror("Unable to get device");
			continue;
		}
		if (device->flags & WGDEVICE_HAS_PUBLIC_KEY) {
			wg_key_to_base64(key, device->public_key);
			printf("%s has public key %s\n", device_name, key);
		} else
			printf("%s has no public key\n", device_name);
		wg_for_each_peer(device, peer) {
			wg_key_to_base64(key, peer->public_key);
			printf(" - peer %s\n", key);
		}
		wg_free_device(device);
	}
	free(device_names);
}

int main(int argc, char *argv[])
{
	wg_peer new_peer = {
		.flags = WGPEER_HAS_PUBLIC_KEY | WGPEER_REPLACE_ALLOWEDIPS
	};
	wg_device new_device = {
		.name = "wgtest0",
		.listen_port = 1234,
		.flags = WGDEVICE_HAS_PRIVATE_KEY | WGDEVICE_HAS_LISTEN_PORT,
		.first_peer = &new_peer,
		.last_peer = &new_peer
	};
	wg_key temp_private_key;

	wg_generate_private_key(temp_private_key);
	wg_generate_public_key(new_peer.public_key, temp_private_key);
	wg_generate_private_key(new_device.private_key);

	if (wg_add_device(new_device.name) < 0) {
		perror("Unable to add device");
		exit(1);
	}

	if (wg_set_device(&new_device) < 0) {
		perror("Unable to set device");
		exit(1);
	}

	list_devices();

	if (wg_del_device(new_device.name) < 0) {
		perror("Unable to delete device");
		exit(1);
	}

	return 0;
}
