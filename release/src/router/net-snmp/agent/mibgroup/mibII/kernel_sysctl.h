#ifndef _MIBGROUP_KERNEL_SYSCTL_H
#define _MIBGROUP_KERNEL_SYSCTL_H

#if defined(NETSNMP_CAN_USE_SYSCTL)

#include "kernel_mib.h"

int      sysctl_read_icmp_stat(struct icmp_mib *);
int      sysctl_read_icmp6_stat(struct icmp6_mib *);
int      sysctl_read_icmp_msg_stat(struct icmp_mib *,
                                   struct icmp4_msg_mib *,
		 		  int *flag);
int      sysctl_read_icmp6_msg_stat(struct icmp6_mib *,
                                    struct icmp6_msg_mib *,
				    int *support);

#endif /* NETSNMP_CAN_USE_SYSCTL */

#endif /* _MIBGROUP_KERNEL_SYSCTL_H */
