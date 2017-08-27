#if defined(CONFIG_BCM_KF_PROTO_ESP)
#ifndef _CONNTRACK_PROTO_ESP_H
#define _CONNTRACK_PROTO_ESP_H
#include <asm/byteorder.h>

/* ESP PROTOCOL HEADER */

struct esphdr {
	__u32	spi;
};

struct nf_ct_esp {
	unsigned int stream_timeout;
	unsigned int timeout;
};

#ifdef __KERNEL__
#include <net/netfilter/nf_conntrack_tuple.h>

#endif /* __KERNEL__ */
#endif /* _CONNTRACK_PROTO_ESP_H */
#endif
