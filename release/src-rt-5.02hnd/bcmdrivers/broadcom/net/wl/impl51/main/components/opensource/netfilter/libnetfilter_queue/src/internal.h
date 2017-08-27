#ifndef INTERNAL_H
#define INTERNAL_H 1

#include "config.h"
#ifdef HAVE_VISIBILITY_HIDDEN
#	define __visible	__attribute__((visibility("default")))
#	define EXPORT_SYMBOL(x)	typeof(x) (x) __visible
#else
#	define EXPORT_SYMBOL
#endif

struct iphdr;
struct ip6_hdr;

uint16_t checksum(uint32_t sum, uint16_t *buf, int size);
uint16_t checksum_tcpudp_ipv4(struct iphdr *iph);
uint16_t checksum_tcpudp_ipv6(struct ip6_hdr *ip6h, void *transport_hdr);

struct pkt_buff {
	uint8_t *mac_header;
	uint8_t *network_header;
	uint8_t *transport_header;

	uint8_t *head;
	uint8_t *data;
	uint8_t *tail;

	uint32_t len;
	uint32_t data_len;

	bool	mangled;
};

#endif
