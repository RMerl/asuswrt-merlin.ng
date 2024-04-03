// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2015-2020 Jason A. Donenfeld <Jason@zx2c4.com>. All Rights Reserved.
 */

#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/sockio.h>
#include <sys/types.h>
#include <net/if.h>
#include <net/if_wg.h>
#include <netinet/in.h>
#include "containers.h"

#define IPC_SUPPORTS_KERNEL_INTERFACE

static int get_dgram_socket(void)
{
	static int sock = -1;
	if (sock < 0)
		sock = socket(AF_INET, SOCK_DGRAM, 0);
	return sock;
}

static int kernel_get_wireguard_interfaces(struct string_list *list)
{
	struct ifgroupreq ifgr = { .ifgr_name = "wg" };
	struct ifg_req *ifg;
	int s = get_dgram_socket(), ret = 0;

	if (s < 0)
		return -errno;

	if (ioctl(s, SIOCGIFGMEMB, (caddr_t)&ifgr) < 0)
		return errno == ENOENT ? 0 : -errno;

	ifgr.ifgr_groups = calloc(1, ifgr.ifgr_len);
	if (!ifgr.ifgr_groups)
		return -errno;
	if (ioctl(s, SIOCGIFGMEMB, (caddr_t)&ifgr) < 0) {
		ret = -errno;
		goto out;
	}

	for (ifg = ifgr.ifgr_groups; ifg && ifgr.ifgr_len > 0; ++ifg) {
		if ((ret = string_list_add(list, ifg->ifgrq_member)) < 0)
			goto out;
		ifgr.ifgr_len -= sizeof(struct ifg_req);
	}

out:
	free(ifgr.ifgr_groups);
	return ret;
}

static int kernel_get_device(struct wgdevice **device, const char *iface)
{
	struct wg_data_io wgdata = { .wgd_size = 0 };
	struct wg_interface_io *wg_iface;
	struct wg_peer_io *wg_peer;
	struct wg_aip_io *wg_aip;
	struct wgdevice *dev;
	struct wgpeer *peer;
	struct wgallowedip *aip;
	int s = get_dgram_socket(), ret;

	if (s < 0)
		return -errno;

	*device = NULL;
	strlcpy(wgdata.wgd_name, iface, sizeof(wgdata.wgd_name));
	for (size_t last_size = wgdata.wgd_size;; last_size = wgdata.wgd_size) {
		if (ioctl(s, SIOCGWG, (caddr_t)&wgdata) < 0)
			goto out;
		if (last_size >= wgdata.wgd_size)
			break;
		wgdata.wgd_interface = realloc(wgdata.wgd_interface, wgdata.wgd_size);
		if (!wgdata.wgd_interface)
			goto out;
	}

	wg_iface = wgdata.wgd_interface;
	dev = calloc(1, sizeof(*dev));
	if (!dev)
		goto out;
	strlcpy(dev->name, iface, sizeof(dev->name));

	if (wg_iface->i_flags & WG_INTERFACE_HAS_RTABLE) {
		dev->fwmark = wg_iface->i_rtable;
		dev->flags |= WGDEVICE_HAS_FWMARK;
	}

	if (wg_iface->i_flags & WG_INTERFACE_HAS_PORT) {
		dev->listen_port = wg_iface->i_port;
		dev->flags |= WGDEVICE_HAS_LISTEN_PORT;
	}

	if (wg_iface->i_flags & WG_INTERFACE_HAS_PUBLIC) {
		memcpy(dev->public_key, wg_iface->i_public, sizeof(dev->public_key));
		dev->flags |= WGDEVICE_HAS_PUBLIC_KEY;
	}

	if (wg_iface->i_flags & WG_INTERFACE_HAS_PRIVATE) {
		memcpy(dev->private_key, wg_iface->i_private, sizeof(dev->private_key));
		dev->flags |= WGDEVICE_HAS_PRIVATE_KEY;
	}

	wg_peer = &wg_iface->i_peers[0];
	for (size_t i = 0; i < wg_iface->i_peers_count; ++i) {
		peer = calloc(1, sizeof(*peer));
		if (!peer)
			goto out;

		if (dev->first_peer == NULL)
			dev->first_peer = peer;
		else
			dev->last_peer->next_peer = peer;
		dev->last_peer = peer;

		if (wg_peer->p_flags & WG_PEER_HAS_PUBLIC) {
			memcpy(peer->public_key, wg_peer->p_public, sizeof(peer->public_key));
			peer->flags |= WGPEER_HAS_PUBLIC_KEY;
		}

		if (wg_peer->p_flags & WG_PEER_HAS_PSK) {
			memcpy(peer->preshared_key, wg_peer->p_psk, sizeof(peer->preshared_key));
			if (!key_is_zero(peer->preshared_key))
				peer->flags |= WGPEER_HAS_PRESHARED_KEY;
		}

		if (wg_peer->p_flags & WG_PEER_HAS_PKA) {
			peer->persistent_keepalive_interval = wg_peer->p_pka;
			peer->flags |= WGPEER_HAS_PERSISTENT_KEEPALIVE_INTERVAL;
		}

		if (wg_peer->p_flags & WG_PEER_HAS_ENDPOINT && wg_peer->p_sa.sa_len <= sizeof(peer->endpoint.addr))
			memcpy(&peer->endpoint.addr, &wg_peer->p_sa, wg_peer->p_sa.sa_len);

		peer->rx_bytes = wg_peer->p_rxbytes;
		peer->tx_bytes = wg_peer->p_txbytes;

		peer->last_handshake_time.tv_sec = wg_peer->p_last_handshake.tv_sec;
		peer->last_handshake_time.tv_nsec = wg_peer->p_last_handshake.tv_nsec;

		wg_aip = &wg_peer->p_aips[0];
		for (size_t j = 0; j < wg_peer->p_aips_count; ++j) {
			aip = calloc(1, sizeof(*aip));
			if (!aip)
				goto out;

			if (peer->first_allowedip == NULL)
				peer->first_allowedip = aip;
			else
				peer->last_allowedip->next_allowedip = aip;
			peer->last_allowedip = aip;

			aip->family = wg_aip->a_af;
			if (wg_aip->a_af == AF_INET) {
				memcpy(&aip->ip4, &wg_aip->a_ipv4, sizeof(aip->ip4));
				aip->cidr = wg_aip->a_cidr;
			} else if (wg_aip->a_af == AF_INET6) {
				memcpy(&aip->ip6, &wg_aip->a_ipv6, sizeof(aip->ip6));
				aip->cidr = wg_aip->a_cidr;
			}
			++wg_aip;
		}
		wg_peer = (struct wg_peer_io *)wg_aip;
	}
	*device = dev;
	errno = 0;
out:
	ret = -errno;
	free(wgdata.wgd_interface);
	return ret;
}

static int kernel_set_device(struct wgdevice *dev)
{
	struct wg_data_io wgdata = { .wgd_size = sizeof(struct wg_interface_io) };
	struct wg_interface_io *wg_iface;
	struct wg_peer_io *wg_peer;
	struct wg_aip_io *wg_aip;
	struct wgpeer *peer;
	struct wgallowedip *aip;
	int s = get_dgram_socket(), ret;
	size_t peer_count, aip_count;

	if (s < 0)
		return -errno;

	for_each_wgpeer(dev, peer) {
		wgdata.wgd_size += sizeof(struct wg_peer_io);
		for_each_wgallowedip(peer, aip)
			wgdata.wgd_size += sizeof(struct wg_aip_io);
	}
	wg_iface = wgdata.wgd_interface = calloc(1, wgdata.wgd_size);
	if (!wgdata.wgd_interface)
		return -errno;
	strlcpy(wgdata.wgd_name, dev->name, sizeof(wgdata.wgd_name));

	if (dev->flags & WGDEVICE_HAS_PRIVATE_KEY) {
		memcpy(wg_iface->i_private, dev->private_key, sizeof(wg_iface->i_private));
		wg_iface->i_flags |= WG_INTERFACE_HAS_PRIVATE;
	}

	if (dev->flags & WGDEVICE_HAS_LISTEN_PORT) {
		wg_iface->i_port = dev->listen_port;
		wg_iface->i_flags |= WG_INTERFACE_HAS_PORT;
	}

	if (dev->flags & WGDEVICE_HAS_FWMARK) {
		wg_iface->i_rtable = dev->fwmark;
		wg_iface->i_flags |= WG_INTERFACE_HAS_RTABLE;
	}

	if (dev->flags & WGDEVICE_REPLACE_PEERS)
		wg_iface->i_flags |= WG_INTERFACE_REPLACE_PEERS;

	peer_count = 0;
	wg_peer = &wg_iface->i_peers[0];
	for_each_wgpeer(dev, peer) {
		wg_peer->p_flags = WG_PEER_HAS_PUBLIC;
		memcpy(wg_peer->p_public, peer->public_key, sizeof(wg_peer->p_public));

		if (peer->flags & WGPEER_HAS_PRESHARED_KEY) {
			memcpy(wg_peer->p_psk, peer->preshared_key, sizeof(wg_peer->p_psk));
			wg_peer->p_flags |= WG_PEER_HAS_PSK;
		}

		if (peer->flags & WGPEER_HAS_PERSISTENT_KEEPALIVE_INTERVAL) {
			wg_peer->p_pka = peer->persistent_keepalive_interval;
			wg_peer->p_flags |= WG_PEER_HAS_PKA;
		}

		if ((peer->endpoint.addr.sa_family == AF_INET || peer->endpoint.addr.sa_family == AF_INET6) &&
		    peer->endpoint.addr.sa_len <= sizeof(wg_peer->p_endpoint)) {
			memcpy(&wg_peer->p_endpoint, &peer->endpoint.addr, peer->endpoint.addr.sa_len);
			wg_peer->p_flags |= WG_PEER_HAS_ENDPOINT;
		}

		if (peer->flags & WGPEER_REPLACE_ALLOWEDIPS)
			wg_peer->p_flags |= WG_PEER_REPLACE_AIPS;

		if (peer->flags & WGPEER_REMOVE_ME)
			wg_peer->p_flags |= WG_PEER_REMOVE;

		aip_count = 0;
		wg_aip = &wg_peer->p_aips[0];
		for_each_wgallowedip(peer, aip) {
			wg_aip->a_af = aip->family;
			wg_aip->a_cidr = aip->cidr;

			if (aip->family == AF_INET)
				memcpy(&wg_aip->a_ipv4, &aip->ip4, sizeof(wg_aip->a_ipv4));
			else if (aip->family == AF_INET6)
				memcpy(&wg_aip->a_ipv6, &aip->ip6, sizeof(wg_aip->a_ipv6));
			else
				continue;
			++aip_count;
			++wg_aip;
		}
		wg_peer->p_aips_count = aip_count;
		++peer_count;
		wg_peer = (struct wg_peer_io *)wg_aip;
	}
	wg_iface->i_peers_count = peer_count;

	if (ioctl(s, SIOCSWG, (caddr_t)&wgdata) < 0)
		goto out;
	errno = 0;

out:
	ret = -errno;
	free(wgdata.wgd_interface);
	return ret;
}
