/* SPDX-License-Identifier: GPL-2.0 */
/*
 *	Copied from Linux Monitor (LiMon) - Networking.
 *
 *	Copyright 1994 - 2000 Neil Russell.
 *	(See License)
 *	Copyright 2000 Roland Borde
 *	Copyright 2000 Paolo Scaffardi
 *	Copyright 2000-2002 Wolfgang Denk, wd@denx.de
 */

#ifndef __PING_H__
#define __PING_H__

#include <common.h>
#include <net.h>

/*
 * Initialize ping (beginning of netloop)
 */
void ping_start(void);

/*
 * Deal with the receipt of a ping packet
 *
 * @param et Ethernet header in packet
 * @param ip IP header in the same packet
 * @param len Packet length
 */
void ping_receive(struct ethernet_hdr *et, struct ip_udp_hdr *ip, int len);

#endif /* __PING_H__ */
