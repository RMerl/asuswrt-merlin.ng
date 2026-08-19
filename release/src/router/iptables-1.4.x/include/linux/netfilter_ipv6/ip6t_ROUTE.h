/* Header for the ROUTE IPv6 target (netfilter patch-o-matic heritage).
 * Restored: referenced by extensions/libip6t_ROUTE.c but absent from tree. */
#ifndef _IP6T_ROUTE_H_target
#define _IP6T_ROUTE_H_target

#include <linux/types.h>

#define IP6T_ROUTE_IFNAMSIZ 16

struct ip6t_route_target_info {
	char      oif[IP6T_ROUTE_IFNAMSIZ];
	char      iif[IP6T_ROUTE_IFNAMSIZ];
	__u32     gw[4];
	__u8      flags;
};

/* values for flags */
#define IP6T_ROUTE_CONTINUE  0x01
#define IP6T_ROUTE_TEE       0x02

#endif /* _IP6T_ROUTE_H_target */
