/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * wol - Wake-on-LAN
 *
 * Supports both Wake-on-LAN packet types:
 * - EtherType 0x0842 packets
 * - UDP packets on ports 0, 7 and 9.
 *
 * Copyright 2018 Lothar Felten, lothar.felten@gmail.com
 */

#if defined(CONFIG_CMD_WOL)

#ifndef __WOL_H__
#define __WOL_H__

#include <net.h>

/**********************************************************************/

#define WOL_SYNC_BYTE			0xFF
#define WOL_SYNC_COUNT			6
#define WOL_MAC_REPETITIONS		16
#define WOL_DEFAULT_TIMEOUT		5000
#define WOL_PASSWORD_4B			4
#define WOL_PASSWORD_6B			6

/*
 * Wake-on-LAN header
 */
struct wol_hdr {
	u8	wol_sync[WOL_SYNC_COUNT];			/* sync bytes */
	u8	wol_dest[WOL_MAC_REPETITIONS * ARP_HLEN];	/* 16x MAC */
	u8	wol_passwd[0];					/* optional */
};

/*
 * Initialize wol (beginning of netloop)
 */
void wol_start(void);

/*
 * Check incoming Wake-on-LAN packet for:
 * - sync bytes
 * - sixteen copies of the target MAC address
 *
 * Optionally store the four or six byte password in the environment
 * variable "wolpassword"
 *
 * @param ip IP header in the packet
 * @param len Packet length
 */
void wol_receive(struct ip_udp_hdr *ip, unsigned int len);

/*
 * Set the timeout for the reception of a Wake-on-LAN packet
 *
 * @param timeout in milliseconds
 */
void wol_set_timeout(ulong timeout);

/**********************************************************************/

#endif /* __WOL_H__ */
#endif
