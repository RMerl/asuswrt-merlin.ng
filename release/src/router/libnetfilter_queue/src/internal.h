#ifndef INTERNAL_H
#define INTERNAL_H 1

#include "config.h"
#include <stdint.h>
#include <stdbool.h>
#ifdef HAVE_VISIBILITY_HIDDEN
#	define EXPORT_SYMBOL __attribute__((visibility("default")))
#else
#	define EXPORT_SYMBOL
#endif

struct iphdr;
struct ip6_hdr;

uint16_t nfq_checksum(uint32_t sum, uint16_t *buf, int size);
uint16_t nfq_checksum_tcpudp_ipv4(struct iphdr *iph, uint16_t protonum);
uint16_t nfq_checksum_tcpudp_ipv6(struct ip6_hdr *ip6h, void *transport_hdr,
				  uint16_t protonum);

struct pkt_buff {
	uint8_t *mac_header;
	uint8_t *network_header;
	uint8_t *transport_header;

	uint8_t *data;

	uint32_t len;
	uint32_t data_len;

	bool	mangled;
};

static inline uint8_t *pktb_tail(struct pkt_buff *pktb)
{
	return pktb->data + pktb->len;
}
#endif
