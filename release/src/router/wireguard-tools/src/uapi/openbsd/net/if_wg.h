/* SPDX-License-Identifier: ISC */
/*
 * Copyright (C) 2020 Jason A. Donenfeld <Jason@zx2c4.com>. All Rights Reserved.
 * Copyright (c) 2020 Matt Dunwoodie <ncon@noconroy.net>
 */

#ifndef __IF_WG_H__
#define __IF_WG_H__

#include <sys/limits.h>
#include <sys/errno.h>

#include <net/if.h>
#include <netinet/in.h>


/*
 * This is the public interface to the WireGuard network interface.
 *
 * It is designed to be used by tools such as ifconfig(8) and wg(8).
 */

#define WG_KEY_LEN 32

#define SIOCSWG _IOWR('i', 210, struct wg_data_io)
#define SIOCGWG _IOWR('i', 211, struct wg_data_io)

#define a_ipv4	a_addr.addr_ipv4
#define a_ipv6	a_addr.addr_ipv6

struct wg_aip_io {
	sa_family_t	 a_af;
	int		 a_cidr;
	union wg_aip_addr {
		struct in_addr		addr_ipv4;
		struct in6_addr		addr_ipv6;
	}		 a_addr;
};

#define WG_PEER_HAS_PUBLIC		(1 << 0)
#define WG_PEER_HAS_PSK			(1 << 1)
#define WG_PEER_HAS_PKA			(1 << 2)
#define WG_PEER_HAS_ENDPOINT		(1 << 3)
#define WG_PEER_REPLACE_AIPS		(1 << 4)
#define WG_PEER_REMOVE			(1 << 5)
#define WG_PEER_UPDATE			(1 << 6)

#define p_sa		p_endpoint.sa_sa
#define p_sin		p_endpoint.sa_sin
#define p_sin6		p_endpoint.sa_sin6

struct wg_peer_io {
	int			p_flags;
	int			p_protocol_version;
	uint8_t			p_public[WG_KEY_LEN];
	uint8_t			p_psk[WG_KEY_LEN];
	uint16_t		p_pka;
	union wg_peer_endpoint {
		struct sockaddr		sa_sa;
		struct sockaddr_in	sa_sin;
		struct sockaddr_in6	sa_sin6;
	}			p_endpoint;
	uint64_t		p_txbytes;
	uint64_t		p_rxbytes;
	struct timespec		p_last_handshake; /* nanotime */
	size_t			p_aips_count;
	struct wg_aip_io	p_aips[];
};

#define WG_INTERFACE_HAS_PUBLIC		(1 << 0)
#define WG_INTERFACE_HAS_PRIVATE	(1 << 1)
#define WG_INTERFACE_HAS_PORT		(1 << 2)
#define WG_INTERFACE_HAS_RTABLE		(1 << 3)
#define WG_INTERFACE_REPLACE_PEERS	(1 << 4)

struct wg_interface_io {
	uint8_t			i_flags;
	in_port_t		i_port;
	int			i_rtable;
	uint8_t			i_public[WG_KEY_LEN];
	uint8_t			i_private[WG_KEY_LEN];
	size_t			i_peers_count;
	struct wg_peer_io	i_peers[];
};

struct wg_data_io {
	char	 		 wgd_name[IFNAMSIZ];
	size_t			 wgd_size;	/* total size of the memory pointed to by wgd_interface */
	struct wg_interface_io	*wgd_interface;
};

#endif /* __IF_WG_H__ */
