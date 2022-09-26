#ifndef __IPADDR_H__
#define __IPADDR_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "list.h"
#include "ipaddr.h"

#define bool  int
#define false 0
#define true  1

typedef struct ip_info_s {
    struct in_addr floor;
    uint32_t cidr;
    struct in_addr ceil; //<-- broadcast
    struct in_addr netmask;
    unsigned short isPriv;

    struct list_head list;
} ip_info_t;

struct private_ip_list {
    char addr[INET6_ADDRSTRLEN];
    uint32_t cidr;
    char mask[INET6_ADDRSTRLEN];
    unsigned short isPriv;
};

struct private_ip_list priv_ip_net[] = {
    {"0.0.0.0",          8, "0x00FFFFFF", 1}, // Current network (only valid as source address)
    {"10.0.0.0",         8, "0x00FFFFFF", 1}, // Used for local communication within a private network
    {"100.64.0.0",      10, "0x003FFFFF", 1}, // Shared address space, aka carrier-grade NAT
    {"127.0.0.0",        8, "0x00FFFFFF", 1}, // Used for lookback addresses to the local host
    {"169.254.0.0",     16, "0x0000FFFF", 1}, // Used for link-local addresses between two hosts on a single link when no IP address is specified
    {"172.16.0.0",      12, "0x000FFFFF", 1}, // Used for local communications with a private network
    {"192.0.0.0",       24, "0x000000FF", 1}, // IETF Protocol Assignments
    {"192.0.2.0",       24, "0x000000FF", 1}, // Assigned as TEST-NET-1, documentation and examples
    {"192.88.99.0",     24, "0x000000FF", 1}, // Reserved. formerly used for IPv6 to IPv4 relay (included IPv6 address block 2002::/16)
    {"192.168.0.0",     16, "0x0000FFFF", 1}, // Used for local communications with a private network
    {"198.18.0.0",      15, "0x0001FFFF", 1}, // Used for benchmark testing of inter-network communications between to separate subnets
    {"198.51.100.0",    24, "0x000000FF", 1}, // Assigned as TEST-NET-2, documentation and examples
    {"203.0.113.0",     24, "0x000000FF", 1}, // Assigned as TEST-NET-3, documentation and examples
    {"224.0.0.0",        4, "0x0FFFFFFF", 1}, // In use for IP multicast
    {"223.255.255.0",   24, "0x000000FF", 1}, // Assigned as MCAST-TEST-NET, documentation and examples
    {"240.0.0.0",        4, "0x0FFFFFFF", 1}  // Reserved for future use
#if 0
    {"255.255.255.255", 32, "0x00000000", 1}  // Reserved for the "limited broadcast" destination address
#endif
};

#define NUM(a)  (sizeof (a) / sizeof (a[0]))

extern int privLanList_init(struct list_head *privlist);
extern int privLanList_free(struct list_head *privlist);
extern bool quick_privLan_chk(struct in_addr *addr);
extern bool in_privLan(struct in_addr *addr, struct list_head *privlist);

#endif //__IPADDR_H__
