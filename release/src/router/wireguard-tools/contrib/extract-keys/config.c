// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2015-2020 Jason A. Donenfeld <Jason@zx2c4.com>. All Rights Reserved.
 */

struct def {
	const char *name;
	long long value;
};
extern const struct def defs[];

#ifdef __KERNEL__
#include "../drivers/net/wireguard/device.h"
#include "../drivers/net/wireguard/peer.h"
#include "../drivers/net/wireguard/noise.h"
const struct def defs[] = {
	{ "SOCK_DEVICE_OFFSET", offsetof(struct sock, sk_user_data) },
	{ "DEVICE_NAME_OFFSET", -ALIGN(sizeof(struct net_device), NETDEV_ALIGN) + offsetof(struct net_device, name) },
	{ "IFNAMSIZ", IFNAMSIZ },
	{ "DEVICE_PEERS_OFFSET", offsetof(struct wg_device, peer_list) },
	{ "PEERS_PEER_OFFSET", -offsetof(struct wg_peer, peer_list) },
	{ "PEER_CURRENTKEY_OFFSET", offsetof(struct wg_peer, keypairs.current_keypair) },
	{ "PEER_PREVIOUSKEY_OFFSET", offsetof(struct wg_peer, keypairs.previous_keypair) },
	{ "PEER_NEXTKEY_OFFSET", offsetof(struct wg_peer, keypairs.next_keypair) },
	{ "KEY_LOCALID_OFFSET", offsetof(struct noise_keypair, entry.index) },
	{ "KEY_REMOTEID_OFFSET", offsetof(struct noise_keypair, remote_index) },
	{ "KEY_SENDING_OFFSET", offsetof(struct noise_keypair, sending.key) },
	{ "KEY_RECEIVING_OFFSET", offsetof(struct noise_keypair, receiving.key) },
	{ NULL, 0 }
};
#else
#include <stdio.h>
int main(int argc, char *argv[])
{
	for (const struct def *def = defs; def->name; ++def)
		printf("#define %s %lld\n", def->name, def->value);
	return 0;
}
#endif
