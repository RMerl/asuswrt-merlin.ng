/*
 *  MIB statistics gathering routines
 *      for Linux architecture
 */

#ifndef _MIBGROUP_KERNEL_LINUX_H
#define _MIBGROUP_KERNEL_LINUX_H

#include "kernel_mib.h"

int             linux_read_ip_stat(struct ip_mib *);
int             linux_read_ip6_stat(struct ip6_mib *);
int             linux_read_icmp_stat(struct icmp_mib *);
int             linux_read_icmp6_stat(struct icmp6_mib *);
int             linux_read_udp_stat(struct udp_mib *);
int             linux_read_udp6_stat(struct udp6_mib *);
int             linux_read_tcp_stat(struct tcp_mib *);
int             linux_read_icmp_msg_stat(struct icmp_mib *,
                                         struct icmp4_msg_mib *,
                                         int *flag);
int             linux_read_icmp6_msg_stat(struct icmp6_mib *,
                                          struct icmp6_msg_mib *,
                                          int *support);

#endif                          /* _MIBGROUP_KERNEL_LINUX_H */
