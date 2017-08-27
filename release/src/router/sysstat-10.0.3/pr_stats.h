/*
 * pr_stats.h: Include file used to display system statistics
 * (C) 1999-2011 by Sebastien Godard (sysstat <at> orange.fr)
 */

#ifndef _PR_STATS_H
#define _PR_STATS_H

#include "common.h"


/*
 ***************************************************************************
 * Prototypes for functions used to display system statistics
 ***************************************************************************
 */

/* Functions used to display instantaneous statistics */
extern __print_funct_t print_cpu_stats
	(struct activity *, int, int, unsigned long long);
extern __print_funct_t print_pcsw_stats
	(struct activity *, int, int, unsigned long long);
extern __print_funct_t print_irq_stats
	(struct activity *, int, int, unsigned long long);
extern __print_funct_t print_swap_stats
	(struct activity *, int, int, unsigned long long);
extern __print_funct_t print_paging_stats
	(struct activity *, int, int, unsigned long long);
extern __print_funct_t print_io_stats
	(struct activity *, int, int, unsigned long long);
extern __print_funct_t print_memory_stats
	(struct activity *, int, int, unsigned long long);
extern __print_funct_t print_ktables_stats
	(struct activity *, int, int, unsigned long long);
extern __print_funct_t print_queue_stats
	(struct activity *, int, int, unsigned long long);
extern __print_funct_t print_serial_stats
	(struct activity *, int, int, unsigned long long);
extern __print_funct_t print_disk_stats
	(struct activity *, int, int, unsigned long long);
extern __print_funct_t print_net_dev_stats
	(struct activity *, int, int, unsigned long long);
extern __print_funct_t print_net_edev_stats
	(struct activity *, int, int, unsigned long long);
extern __print_funct_t print_net_nfs_stats
	(struct activity *, int, int, unsigned long long);
extern __print_funct_t print_net_nfsd_stats
	(struct activity *, int, int, unsigned long long);
extern __print_funct_t print_net_sock_stats
	(struct activity *, int, int, unsigned long long);
extern __print_funct_t print_net_ip_stats
	(struct activity *, int, int, unsigned long long);
extern __print_funct_t print_net_eip_stats
	(struct activity *, int, int, unsigned long long);
extern __print_funct_t print_net_icmp_stats
	(struct activity *, int, int, unsigned long long);
extern __print_funct_t print_net_eicmp_stats
	(struct activity *, int, int, unsigned long long);
extern __print_funct_t print_net_tcp_stats
	(struct activity *, int, int, unsigned long long);
extern __print_funct_t print_net_etcp_stats
	(struct activity *, int, int, unsigned long long);
extern __print_funct_t print_net_udp_stats
	(struct activity *, int, int, unsigned long long);
extern __print_funct_t print_net_sock6_stats
	(struct activity *, int, int, unsigned long long);
extern __print_funct_t print_net_ip6_stats
	(struct activity *, int, int, unsigned long long);
extern __print_funct_t print_net_eip6_stats
	(struct activity *, int, int, unsigned long long);
extern __print_funct_t print_net_icmp6_stats
	(struct activity *, int, int, unsigned long long);
extern __print_funct_t print_net_eicmp6_stats
	(struct activity *, int, int, unsigned long long);
extern __print_funct_t print_net_udp6_stats
	(struct activity *, int, int, unsigned long long);
extern __print_funct_t print_pwr_cpufreq_stats
	(struct activity *, int, int, unsigned long long);
extern __print_funct_t print_pwr_fan_stats
	(struct activity *, int, int, unsigned long long);
extern __print_funct_t print_pwr_temp_stats
	(struct activity *, int, int, unsigned long long);
extern __print_funct_t print_pwr_in_stats
	(struct activity *, int, int, unsigned long long);
extern __print_funct_t print_huge_stats
	(struct activity *, int, int, unsigned long long);
extern __print_funct_t print_pwr_wghfreq_stats
	(struct activity *, int, int, unsigned long long);
extern __print_funct_t print_pwr_usb_stats
	(struct activity *, int, int, unsigned long long);

/* Functions used to display average statistics */
extern __print_funct_t print_avg_memory_stats
	(struct activity *, int, int, unsigned long long);
extern __print_funct_t print_avg_ktables_stats
	(struct activity *, int, int, unsigned long long);
extern __print_funct_t print_avg_queue_stats
	(struct activity *, int, int, unsigned long long);
extern __print_funct_t print_avg_net_sock_stats
	(struct activity *, int, int, unsigned long long);
extern __print_funct_t print_avg_net_sock6_stats
	(struct activity *, int, int, unsigned long long);
extern __print_funct_t print_avg_pwr_cpufreq_stats
	(struct activity *, int, int, unsigned long long);
extern __print_funct_t print_avg_pwr_fan_stats
	(struct activity *, int, int, unsigned long long);
extern __print_funct_t print_avg_pwr_temp_stats
	(struct activity *, int, int, unsigned long long);
extern __print_funct_t print_avg_pwr_in_stats
	(struct activity *, int, int, unsigned long long);
extern __print_funct_t print_avg_huge_stats
	(struct activity *, int, int, unsigned long long);
extern __print_funct_t print_avg_pwr_usb_stats
	(struct activity *, int, int, unsigned long long);

#endif /* _PR_STATS_H */
