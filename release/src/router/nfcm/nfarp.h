#ifndef __NFARP_H__
#define __NFARP_H__

#include "log.h"

enum arp_node_attr {
	ARP_ATTR_HOSTNAME = 0,
	ARP_ATTR_IP,
	ARP_ATTR_AT,
	ARP_ATTR_MAC,
	ARP_ATTR_PHY_TYPE,
	ARP_ATTR_ON,
	ARP_ATTR_IFACE,

	ARP_ATTR_MAX
};

typedef struct arp_node_s {
    bool isv4;
    bool iswl;
    struct in_addr srcv4;
    struct in6_addr srcv6;
    int8_t port;
	char mac[ETHER_ADDR_LENGTH];

	struct list_head list;
} arp_node_t;

extern void arp_list_free(struct list_head *list);
extern void arp_list_parse(char *fname, struct list_head *arlist, struct list_head *list);

#endif // __NFARP_H__
