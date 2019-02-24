/*
 * Copyright (C) 2012-2014 Tobias Brunner
 * HSR Hochschule fuer Technik Rapperswil
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */


#include "ip_packet.h"

#include <library.h>
#include <utils/debug.h>

#include <sys/types.h>

#ifndef WIN32
#include <netinet/in.h>
#include <netinet/ip.h>
#ifdef HAVE_NETINET_IP6_H
#include <netinet/ip6.h>
#endif
#else
struct ip {
#if BYTE_ORDER == LITTLE_ENDIAN
	uint8_t ip_hl: 4;
	uint8_t ip_v: 4;
#elif BYTE_ORDER == BIG_ENDIAN
	uint8_t ip_v: 4;
	uint8_t ip_hl: 4;
#endif
	uint8_t ip_tos;
	uint16_t ip_len;
	uint16_t ip_id;
	uint16_t ip_off;
	uint8_t ip_ttl;
	uint8_t ip_p;
	uint16_t ip_sum;
	struct in_addr ip_src, ip_dst;
} __attribute__((packed));
struct ip6_hdr {
	uint32_t ip6_flow; /* 4 bit version, 8 bit TC, 20 bit flow label */
	uint16_t ip6_plen;
	uint8_t ip6_nxt;
	uint8_t ip6_hlim;
	struct in6_addr ip6_src, ip6_dst;
} __attribute__((packed));
struct ip6_ext {
	uint8_t ip6e_nxt;
	uint8_t ip6e_len;
} __attribute__((packed));
#define HAVE_NETINET_IP6_H /* not really, but we only need the structs above */
#endif

#ifndef IP_OFFMASK
#define IP_OFFMASK 0x1fff
#endif

/**
 * TCP header, defined here because platforms disagree regarding member names
 * and unfortunately Android does not define a variant with BSD names.
 */
struct tcphdr {
	uint16_t source;
	uint16_t dest;
	uint32_t seq;
	uint32_t ack_seq;
	uint16_t flags;
	uint16_t window;
	uint16_t check;
	uint16_t urg_ptr;
} __attribute__((packed));

/**
 * UDP header, similar to the TCP header the system headers disagree on member
 * names.  Linux uses a union and on Android we could define __FAVOR_BSD to get
 * the BSD member names, but this is simpler and more consistent with the above.
 */
struct udphdr {
	uint16_t source;
	uint16_t dest;
	uint16_t len;
	uint16_t check;
} __attribute__((packed));

typedef struct private_ip_packet_t private_ip_packet_t;

/**
 * Private additions to ip_packet_t.
 */
struct private_ip_packet_t {

	/**
	 * Public members
	 */
	ip_packet_t public;

	/**
	 * Source address
	 */
	host_t *src;

	/**
	 * Destination address
	 */
	host_t *dst;

	/**
	 * IP packet
	 */
	chunk_t packet;

	/**
	 * IP payload (points into packet)
	 */
	chunk_t payload;

	/**
	 * IP version
	 */
	uint8_t version;

	/**
	 * Protocol|Next Header field
	 */
	uint8_t next_header;

};

METHOD(ip_packet_t, get_version, uint8_t,
	private_ip_packet_t *this)
{
	return this->version;
}

METHOD(ip_packet_t, get_source, host_t*,
	private_ip_packet_t *this)
{
	return this->src;
}

METHOD(ip_packet_t, get_destination, host_t*,
	private_ip_packet_t *this)
{
	return this->dst;
}

METHOD(ip_packet_t, get_encoding, chunk_t,
	private_ip_packet_t *this)
{
	return this->packet;
}

METHOD(ip_packet_t, get_payload, chunk_t,
	private_ip_packet_t *this)
{
	return this->payload;
}

METHOD(ip_packet_t, get_next_header, uint8_t,
	private_ip_packet_t *this)
{
	return this->next_header;
}

METHOD(ip_packet_t, clone_, ip_packet_t*,
	private_ip_packet_t *this)
{
	return ip_packet_create(chunk_clone(this->packet));
}

METHOD(ip_packet_t, destroy, void,
	private_ip_packet_t *this)
{
	this->src->destroy(this->src);
	this->dst->destroy(this->dst);
	chunk_free(&this->packet);
	free(this);
}

/**
 * Parse transport protocol header
 */
static bool parse_transport_header(chunk_t packet, uint8_t proto,
								   uint16_t *sport, uint16_t *dport)
{
	switch (proto)
	{
		case IPPROTO_UDP:
		{
			struct udphdr *udp;

			if (packet.len < sizeof(*udp))
			{
				DBG1(DBG_ESP, "UDP packet too short");
				return FALSE;
			}
			udp = (struct udphdr*)packet.ptr;
			*sport = ntohs(udp->source);
			*dport = ntohs(udp->dest);
			break;
		}
		case IPPROTO_TCP:
		{
			struct tcphdr *tcp;

			if (packet.len < sizeof(*tcp))
			{
				DBG1(DBG_ESP, "TCP packet too short");
				return FALSE;
			}
			tcp = (struct tcphdr*)packet.ptr;
			*sport = ntohs(tcp->source);
			*dport = ntohs(tcp->dest);
			break;
		}
		default:
			break;
	}
	return TRUE;
}

#ifdef HAVE_NETINET_IP6_H
/**
 * Skip to the actual payload and parse the transport header.
 */
static bool parse_transport_header_v6(struct ip6_hdr *ip, chunk_t packet,
									  chunk_t *payload, uint8_t *proto,
									  uint16_t *sport, uint16_t *dport)
{
	struct ip6_ext *ext;
	bool fragment = FALSE;

	*proto = ip->ip6_nxt;
	*payload = chunk_skip(packet, 40);
	while (payload->len >= sizeof(struct ip6_ext))
	{
		switch (*proto)
		{
			case 44:  /* Fragment Header */
				fragment = TRUE;
				/* skip the header */
			case 0:   /* Hop-by-Hop Options Header */
			case 43:  /* Routing Header */
			case 60:  /* Destination Options Header */
			case 135: /* Mobility Header */
			case 139: /* HIP */
			case 140: /* Shim6 */
				/* simply skip over these headers for now */
				ext = (struct ip6_ext*)payload->ptr;
				*proto = ext->ip6e_nxt;
				*payload = chunk_skip(*payload, 8 * (ext->ip6e_len + 1));
				continue;
			default:
				/* assume anything else is an upper layer protocol but only
				 * attempt to parse the transport header for non-fragmented
				 * packets as there is no guarantee that initial fragments
				 * contain the transport header, depending on the number and
				 * type of extension headers */
				if (!fragment &&
					!parse_transport_header(*payload, *proto, sport, dport))
				{
					return FALSE;
				}
				break;
		}
		break;
	}
	return TRUE;
}
#endif /* HAVE_NETINET_IP6_H */

/**
 * Described in header.
 */
ip_packet_t *ip_packet_create(chunk_t packet)
{
	private_ip_packet_t *this;
	uint8_t version, next_header;
	uint16_t sport = 0, dport = 0;
	host_t *src, *dst;
	chunk_t payload;

	if (packet.len < 1)
	{
		DBG1(DBG_ESP, "IP packet too short");
		goto failed;
	}

	version = (packet.ptr[0] & 0xf0) >> 4;

	switch (version)
	{
		case 4:
		{
			struct ip *ip;

			if (packet.len < sizeof(struct ip))
			{
				DBG1(DBG_ESP, "IPv4 packet too short");
				goto failed;
			}
			ip = (struct ip*)packet.ptr;
			/* remove any RFC 4303 TFC extra padding */
			packet.len = min(packet.len, untoh16(&ip->ip_len));
			payload = chunk_skip(packet, ip->ip_hl * 4);
			if ((ip->ip_off & htons(IP_OFFMASK)) == 0 &&
				!parse_transport_header(payload, ip->ip_p, &sport, &dport))
			{
				goto failed;
			}
			src = host_create_from_chunk(AF_INET,
										 chunk_from_thing(ip->ip_src), sport);
			dst = host_create_from_chunk(AF_INET,
										 chunk_from_thing(ip->ip_dst), dport);
			next_header = ip->ip_p;
			break;
		}
#ifdef HAVE_NETINET_IP6_H
		case 6:
		{
			struct ip6_hdr *ip;

			if (packet.len < sizeof(*ip))
			{
				DBG1(DBG_ESP, "IPv6 packet too short");
				goto failed;
			}
			ip = (struct ip6_hdr*)packet.ptr;
			/* remove any RFC 4303 TFC extra padding */
			packet.len = min(packet.len, 40 + untoh16(&ip->ip6_plen));
			if (!parse_transport_header_v6(ip, packet, &payload, &next_header,
										   &sport, &dport))
			{
				goto failed;
			}
			src = host_create_from_chunk(AF_INET6,
										 chunk_from_thing(ip->ip6_src), sport);
			dst = host_create_from_chunk(AF_INET6,
										 chunk_from_thing(ip->ip6_dst), dport);
			break;
		}
#endif /* HAVE_NETINET_IP6_H */
		default:
			DBG1(DBG_ESP, "unsupported IP version");
			goto failed;
	}

	INIT(this,
		.public = {
			.get_version = _get_version,
			.get_source = _get_source,
			.get_destination = _get_destination,
			.get_next_header = _get_next_header,
			.get_encoding = _get_encoding,
			.get_payload = _get_payload,
			.clone = _clone_,
			.destroy = _destroy,
		},
		.src = src,
		.dst = dst,
		.packet = packet,
		.payload = payload,
		.version = version,
		.next_header = next_header,
	);
	return &this->public;

failed:
	chunk_free(&packet);
	return NULL;
}

/**
 * Calculate the checksum for the pseudo IP header
 */
static uint16_t pseudo_header_checksum(host_t *src, host_t *dst,
										uint8_t proto, chunk_t payload)
{
	switch (src->get_family(src))
	{
		case AF_INET:
		{
			struct __attribute__((packed)) {
				uint32_t src;
				uint32_t dst;
				u_char zero;
				u_char proto;
				uint16_t len;
			} pseudo = {
				.proto = proto,
				.len = htons(payload.len),
			};
			memcpy(&pseudo.src, src->get_address(src).ptr,
				   sizeof(pseudo.src));
			memcpy(&pseudo.dst, dst->get_address(dst).ptr,
				   sizeof(pseudo.dst));
			return chunk_internet_checksum(chunk_from_thing(pseudo));
		}
		case AF_INET6:
		{
			struct __attribute__((packed)) {
				u_char src[16];
				u_char dst[16];
				uint32_t len;
				u_char zero[3];
				u_char next_header;
			} pseudo = {
				.next_header = proto,
				.len = htons(payload.len),
			};
			memcpy(&pseudo.src, src->get_address(src).ptr,
				   sizeof(pseudo.src));
			memcpy(&pseudo.dst, dst->get_address(dst).ptr,
				   sizeof(pseudo.dst));
			return chunk_internet_checksum(chunk_from_thing(pseudo));
		}
	}
	return 0xffff;
}

/**
 * Apply transport ports and calculate header checksums
 */
static void fix_transport_header(host_t *src, host_t *dst, uint8_t proto,
								 chunk_t payload)
{
	uint16_t sum = 0, sport, dport;

	sport = src->get_port(src);
	dport = dst->get_port(dst);

	switch (proto)
	{
		case IPPROTO_UDP:
		{
			struct udphdr *udp;

			if (payload.len < sizeof(*udp))
			{
				return;
			}
			udp = (struct udphdr*)payload.ptr;
			if (sport != 0)
			{
				udp->source = htons(sport);
			}
			if (dport != 0)
			{
				udp->dest = htons(dport);
			}
			udp->check = 0;
			sum = pseudo_header_checksum(src, dst, proto, payload);
			udp->check = chunk_internet_checksum_inc(payload, sum);
			break;
		}
		case IPPROTO_TCP:
		{
			struct tcphdr *tcp;

			if (payload.len < sizeof(*tcp))
			{
				return;
			}
			tcp = (struct tcphdr*)payload.ptr;
			if (sport != 0)
			{
				tcp->source = htons(sport);
			}
			if (dport != 0)
			{
				tcp->dest = htons(dport);
			}
			tcp->check = 0;
			sum = pseudo_header_checksum(src, dst, proto, payload);
			tcp->check = chunk_internet_checksum_inc(payload, sum);
			break;
		}
		default:
			break;
	}
}

/**
 * Described in header.
 */
ip_packet_t *ip_packet_create_from_data(host_t *src, host_t *dst,
										uint8_t next_header, chunk_t data)
{
	chunk_t packet;
	int family;

	family = src->get_family(src);
	if (family != dst->get_family(dst))
	{
		DBG1(DBG_ESP, "address family does not match");
		return NULL;
	}

	switch (family)
	{
		case AF_INET:
		{
			struct ip ip = {
				.ip_v = 4,
				.ip_hl = 5,
				.ip_len = htons(20 + data.len),
				.ip_ttl = 0x80,
				.ip_p = next_header,
			};
			memcpy(&ip.ip_src, src->get_address(src).ptr, sizeof(ip.ip_src));
			memcpy(&ip.ip_dst, dst->get_address(dst).ptr, sizeof(ip.ip_dst));
			ip.ip_sum = chunk_internet_checksum(chunk_from_thing(ip));

			packet = chunk_cat("cc", chunk_from_thing(ip), data);
			fix_transport_header(src, dst, next_header, chunk_skip(packet, 20));
			return ip_packet_create(packet);
		}
#ifdef HAVE_NETINET_IP6_H
		case AF_INET6:
		{
			struct ip6_hdr ip = {
				.ip6_flow = htonl(6 << 28),
				.ip6_plen = htons(data.len),
				.ip6_nxt = next_header,
				.ip6_hlim = 0x80,
			};
			memcpy(&ip.ip6_src, src->get_address(src).ptr, sizeof(ip.ip6_src));
			memcpy(&ip.ip6_dst, dst->get_address(dst).ptr, sizeof(ip.ip6_dst));

			packet = chunk_cat("cc", chunk_from_thing(ip), data);
			fix_transport_header(src, dst, next_header, chunk_skip(packet, 40));
			return ip_packet_create(packet);
		}
#endif /* HAVE_NETINET_IP6_H */
		default:
			DBG1(DBG_ESP, "unsupported address family");
			return NULL;
	}
}

/**
 * Described in header.
 */
ip_packet_t *ip_packet_create_udp_from_data(host_t *src, host_t *dst,
											chunk_t data)
{
	struct udphdr udp = {
		.len = htons(8 + data.len),
		.check = 0,
	};
	ip_packet_t *packet;

	data = chunk_cat("cc", chunk_from_thing(udp), data);
	packet = ip_packet_create_from_data(src, dst, IPPROTO_UDP, data);
	chunk_free(&data);
	return packet;
}
