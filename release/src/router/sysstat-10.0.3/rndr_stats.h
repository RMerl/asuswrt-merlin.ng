/*
 * rndr_stats.h: Include file used to display system statistics in selected format.
 * (C) 1999-2011 by Sebastien Godard (sysstat <at> orange.fr)
 */

#ifndef _RNDR_STATS_H
#define _RNDR_STATS_H

#include "common.h"

/*
 ***************************************************************************
 * Definitions for functions used by sadf.
 ***************************************************************************
 */

#define PT_NOFLAG  0x0000	/* Prevent undescribed '0' in render calls */
#define PT_USEINT  0x0001	/* Use the integer arg, not double nor string */
#define PT_NEWLIN  0x0002	/* Terminate the current output line */
#define PT_USESTR  0x0004	/* Use the string arg */

#define NOVAL      0		/* For placeholder zeros */
#define DNOVAL     0.0		/* Wilma!  */

/*
 * Conses are used to type independent passing
 * of variable optional data into our rendering routine.
 */

typedef enum e_tcons {iv, sv} tcons; /* Types of conses */

typedef struct {
	tcons t;		/* Type in {iv,sv} */
	union {
		unsigned long int i;
		char *s;
	} a, b;			/* Value pair, either ints or char *s */
} Cons;

/*
 ***************************************************************************
 * Prototypes for functions used to display system statistics in selected
 * format.
 ***************************************************************************
 */

/* Functions used to display statistics in the format selected by sadf */
extern __print_funct_t render_pcsw_stats
	(struct activity *, int, char *, int, unsigned long long);
extern __print_funct_t render_cpu_stats
	(struct activity *, int, char *, int, unsigned long long);
extern __print_funct_t render_irq_stats
	(struct activity *, int, char *, int, unsigned long long);
extern __print_funct_t render_swap_stats
	(struct activity *, int, char *, int, unsigned long long);
extern __print_funct_t render_paging_stats
	(struct activity *, int, char *, int, unsigned long long);
extern __print_funct_t render_io_stats
	(struct activity *, int, char *, int, unsigned long long);
extern __print_funct_t render_memory_stats
	(struct activity *, int, char *, int, unsigned long long);
extern __print_funct_t render_ktables_stats
	(struct activity *, int, char *, int, unsigned long long);
extern __print_funct_t render_queue_stats
	(struct activity *, int, char *, int, unsigned long long);
extern __print_funct_t render_serial_stats
	(struct activity *, int, char *, int, unsigned long long);
extern __print_funct_t render_disk_stats
	(struct activity *, int, char *, int, unsigned long long);
extern __print_funct_t render_net_dev_stats
	(struct activity *, int, char *, int, unsigned long long);
extern __print_funct_t render_net_edev_stats
	(struct activity *, int, char *, int, unsigned long long);
extern __print_funct_t render_net_nfs_stats
	(struct activity *, int, char *, int, unsigned long long);
extern __print_funct_t render_net_nfsd_stats
	(struct activity *, int, char *, int, unsigned long long);
extern __print_funct_t render_net_sock_stats
	(struct activity *, int, char *, int, unsigned long long);
extern __print_funct_t render_net_ip_stats
	(struct activity *, int, char *, int, unsigned long long);
extern __print_funct_t render_net_eip_stats
	(struct activity *, int, char *, int, unsigned long long);
extern __print_funct_t render_net_icmp_stats
	(struct activity *, int, char *, int, unsigned long long);
extern __print_funct_t render_net_eicmp_stats
	(struct activity *, int, char *, int, unsigned long long);
extern __print_funct_t render_net_tcp_stats
	(struct activity *, int, char *, int, unsigned long long);
extern __print_funct_t render_net_etcp_stats
	(struct activity *, int, char *, int, unsigned long long);
extern __print_funct_t render_net_udp_stats
	(struct activity *, int, char *, int, unsigned long long);
extern __print_funct_t render_net_sock6_stats
	(struct activity *, int, char *, int, unsigned long long);
extern __print_funct_t render_net_ip6_stats
	(struct activity *, int, char *, int, unsigned long long);
extern __print_funct_t render_net_eip6_stats
	(struct activity *, int, char *, int, unsigned long long);
extern __print_funct_t render_net_icmp6_stats
	(struct activity *, int, char *, int, unsigned long long);
extern __print_funct_t render_net_eicmp6_stats
	(struct activity *, int, char *, int, unsigned long long);
extern __print_funct_t render_net_udp6_stats
	(struct activity *, int, char *, int, unsigned long long);
extern __print_funct_t render_pwr_cpufreq_stats
	(struct activity *, int, char *, int, unsigned long long);
extern __print_funct_t render_pwr_fan_stats
	(struct activity *, int, char *, int, unsigned long long);
extern __print_funct_t render_pwr_temp_stats
	(struct activity *, int, char *, int, unsigned long long);
extern __print_funct_t render_pwr_in_stats
	(struct activity *, int, char *, int, unsigned long long);
extern __print_funct_t render_huge_stats
	(struct activity *, int, char *, int, unsigned long long);
extern __print_funct_t render_pwr_wghfreq_stats
	(struct activity *, int, char *, int, unsigned long long);
extern __print_funct_t render_pwr_usb_stats
	(struct activity *, int, char *, int, unsigned long long);

#endif /* _RNDR_STATS_H */
