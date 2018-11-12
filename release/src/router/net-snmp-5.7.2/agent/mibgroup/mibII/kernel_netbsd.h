#ifndef _MIBGROUP_KERNEL_NETBSD_H
#define _MIBGROUP_KERNEL_NETBSD_H

#if defined(NETBSD_STATS_VIA_SYSCTL)

#include "kernel_mib.h"

int      netbsd_read_icmp_stat(struct icmp_mib *);
int      netbsd_read_icmp6_stat(struct icmp6_mib *);
int      netbsd_read_icmp_msg_stat(struct icmp_mib *,
                                   struct icmp4_msg_mib *,
		 		  int *flag);
int      netbsd_read_icmp6_msg_stat(struct icmp6_mib *,
                                    struct icmp6_msg_mib *,
				    int *support);
int      netbsd_read_ip_stat(struct ip_mib *);
int      netbsd_read_tcp_stat(struct tcp_mib *);
int      netbsd_read_udp_stat(struct udp_mib *);

#endif /* NETBSD_STATS_VIA_SYSCTL */

#endif /* _MIBGROUP_KERNEL_NETBSD_H */
