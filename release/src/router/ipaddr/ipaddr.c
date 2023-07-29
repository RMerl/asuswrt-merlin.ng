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

#define bool  int
#define false 0
#define true  1

LIST_HEAD(privLanList);

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

// the information is got from https://en.wikipedia.org/wiki/Reserved_IP_addresses
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
    {"240.0.0.0",        4, "0x0FFFFFFF", 1}, // Reserved for future use
#if 1
    {"255.255.255.255", 32, "0x00000000", 1}  // Reserved for the "limited broadcast" destination address
#endif
};

#define NUM(a)	(sizeof (a) / sizeof (a[0]))

ip_info_t *ip_info_new()
{
	ip_info_t *ii = (ip_info_t *)calloc(1, sizeof(ip_info_t));
	if (!ii)
		return NULL;

	INIT_LIST_HEAD(&ii->list);

	return ii;

}

void ip_info_free(ip_info_t *ii)
{
	if (ii) free(ii);

	return;
}

#ifdef PDBG
void ip_info_dump(struct list_head *privlist)
{
	ip_info_t *ii;
	char ipstr[INET6_ADDRSTRLEN];
	int i=0;

	printf("		Floor		  Val		Ceil		  Val	  Subnetmask		 Val   Pri\n");
	printf("== ================ ========= ================ ======== ================ ======== ===\n");

	list_for_each_entry(ii, privlist, list) {
		i++;
		inet_ntop(AF_INET, &ii->floor, ipstr, INET_ADDRSTRLEN);
		printf("%2d: %-16s %08X ", i, ipstr, ii->floor.s_addr);

		inet_ntop(AF_INET, &ii->ceil, ipstr, INET_ADDRSTRLEN);
		printf("%-16s %08X ", ipstr, ii->ceil.s_addr);

		inet_ntop(AF_INET, &ii->netmask, ipstr, INET_ADDRSTRLEN);
		printf("%-16s %08X ", ipstr, ii->netmask.s_addr);

		printf("Yes\n");
	}

	return;
}
#endif //PDBG

int priv_ip_info_add(struct list_head *privlist)
{
	int i;
	ip_info_t *ii;
	uint32_t t_ip;

	char ceilstr[INET6_ADDRSTRLEN];
	char floorstr[INET6_ADDRSTRLEN];
	char mskstr[INET6_ADDRSTRLEN];

	for(i=0; i<NUM(priv_ip_net); i++) {
		ii = ip_info_new();
		if(ii == NULL) 
			continue;

		list_add_tail(&ii->list, privlist);

		// cidr
		ii->cidr = priv_ip_net[i].cidr;

		// subnet floor address
		inet_pton(AF_INET, priv_ip_net[i].addr, &ii->floor);
#ifdef PDBG
		inet_ntop(AF_INET, &ii->floor, floorstr, INET_ADDRSTRLEN);
		printf("addr=%16s[0x%08x], addr=[%16s]\n", priv_ip_net[i].addr, ii->floor.s_addr, floorstr);
#endif

		// subnet mask	
		t_ip = ntohl(strtoul(priv_ip_net[i].mask, NULL, 16));
		ii->netmask.s_addr = ~t_ip; 

#ifdef PDBG
		inet_ntop(AF_INET, &ii->netmask, mskstr, INET_ADDRSTRLEN);
		printf("<<<<netmask=%16s[0x%08x]\n", mskstr, ii->netmask.s_addr);
#endif
	
		// subnet ceil address aka broadcast
		ii->ceil.s_addr = ii->floor.s_addr ^ ~ii->netmask.s_addr;

#ifdef PDBG
		inet_ntop(AF_INET, &ii->ceil, ceilstr, INET_ADDRSTRLEN);
		printf("ceil=s%16s[0x%08x]\n", ceilstr, ii->ceil.s_addr);
#endif
	
#ifdef PDBG
		inet_ntop(AF_INET, &ii->floor, floorstr, INET_ADDRSTRLEN);
		inet_ntop(AF_INET, &ii->ceil, ceilstr, INET_ADDRSTRLEN);
		inet_ntop(AF_INET, &ii->netmask, mskstr, INET_ADDRSTRLEN);
		printf("floor=[%s], ceil=[%s], mask=[%s]\n", floorstr, ceilstr, mskstr);
#endif
	}

	return true;
}

int privLanList_init(struct list_head *privlist)
{
	priv_ip_info_add(privlist);

#ifdef PDBG
	ip_info_dump(privlist);	
#endif

	return true;
}

int privLanList_free(struct list_head *privlist)
{
	ip_info_t *ii;
	ip_info_t *iit;

    list_for_each_entry_safe(ii, iit, privlist, list) {
        list_del(&ii->list);
		ip_info_free(ii);
    }

    return true;

#ifdef PDBG
	ip_info_dump(privlist);	
#endif

	return true;
}

bool quick_privLan_chk(struct in_addr *addr)
{
	uint8_t pre = addr->s_addr & 0x000000FF;

	switch(pre) {
	case 0:
	case 100:
	case 127:
	case 169:
	case 172:
	case 192:
	case 198:
	case 203:
	case 223:
	case 240:
	case 255:
		return true;

	default:
		return false;
	}
}

bool in_privLan(struct in_addr *addr, struct list_head *privlist)
{
	ip_info_t *ii;

	list_for_each_entry(ii, privlist, list) {
		if ((htonl(addr->s_addr) & htonl(ii->netmask.s_addr)) == 
			(htonl(ii->floor.s_addr) & htonl(ii->netmask.s_addr))) 
		{
			return true;
		}
	}
	return false;
}
