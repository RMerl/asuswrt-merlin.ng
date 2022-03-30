// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018 Lothar Felten, lothar.felten@gmail.com
 */

#include <common.h>
#include <command.h>
#include <net.h>
#include <environment.h>
#include "wol.h"

static ulong wol_timeout = WOL_DEFAULT_TIMEOUT;

/*
 * Check incoming Wake-on-LAN packet for:
 * - sync bytes
 * - sixteen copies of the target MAC address
 *
 * @param wol Wake-on-LAN packet
 * @param len Packet length
 */
static int wol_check_magic(struct wol_hdr *wol, unsigned int len)
{
	int i;

	if (len < sizeof(struct wol_hdr))
		return 0;

	for (i = 0; i < WOL_SYNC_COUNT; i++)
		if (wol->wol_sync[i] != WOL_SYNC_BYTE)
			return 0;

	for (i = 0; i < WOL_MAC_REPETITIONS; i++)
		if (memcmp(&wol->wol_dest[i * ARP_HLEN],
			   net_ethaddr, ARP_HLEN) != 0)
			return 0;

	return 1;
}

void wol_receive(struct ip_udp_hdr *ip, unsigned int len)
{
	struct wol_hdr *wol;

	wol = (struct wol_hdr *)ip;

	if (!wol_check_magic(wol, len))
		return;

	/* save the optional password using the ether-wake formats */
	/* don't check for exact length, the packet might have padding */
	if (len >= (sizeof(struct wol_hdr) + WOL_PASSWORD_6B)) {
		eth_env_set_enetaddr("wolpassword", wol->wol_passwd);
	} else if (len >= (sizeof(struct wol_hdr) + WOL_PASSWORD_4B)) {
		char buffer[16];
		struct in_addr *ip = (struct in_addr *)(wol->wol_passwd);

		ip_to_string(*ip, buffer);
		env_set("wolpassword", buffer);
	}
	net_set_state(NETLOOP_SUCCESS);
}

static void wol_udp_handler(uchar *pkt, unsigned int dest, struct in_addr sip,
			    unsigned int src, unsigned int len)
{
	struct wol_hdr *wol;

	wol = (struct wol_hdr *)pkt;

	/* UDP destination port must be 0, 7 or 9 */
	if (dest != 0 && dest != 7 && dest != 9)
		return;

	if (!wol_check_magic(wol, len))
		return;

	net_set_state(NETLOOP_SUCCESS);
}

void wol_set_timeout(ulong timeout)
{
	wol_timeout = timeout;
}

static void wol_timeout_handler(void)
{
	eth_halt();
	net_set_state(NETLOOP_FAIL);
}

void wol_start(void)
{
	net_set_timeout_handler(wol_timeout, wol_timeout_handler);
	net_set_udp_handler(wol_udp_handler);
}
