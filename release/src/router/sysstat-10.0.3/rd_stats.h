/*
 * rd_stats.h: Include file used to read system statistics
 * (C) 1999-2011 by Sebastien Godard (sysstat <at> orange.fr)
 */

#ifndef _RD_STATS_H
#define _RD_STATS_H

#include "common.h"


/*
 ***************************************************************************
 * Miscellaneous constants
 ***************************************************************************
 */

/* Get IFNAMSIZ */
#include <net/if.h>
#ifndef IFNAMSIZ
#define IFNAMSIZ	16
#endif

/* Maximum length of network interface name */
#define MAX_IFACE_LEN	IFNAMSIZ
/* Maximum length of USB manufacturer string */
#define MAX_MANUF_LEN	24
/* Maximum length of USB product string */
#define MAX_PROD_LEN	48

#define CNT_DEV		0
#define CNT_PART	1
#define CNT_ALL_DEV	0
#define CNT_USED_DEV	1

#define READ_PROC_STAT		0
#define READ_DISKSTATS		1
#define READ_PPARTITIONS	2

/*
 ***************************************************************************
 * System files containing statistics
 ***************************************************************************
 */

/* Files */
#define PROC		"/proc"
#define SERIAL		"/proc/tty/driver/serial"
#define FDENTRY_STATE	"/proc/sys/fs/dentry-state"
#define FFILE_NR	"/proc/sys/fs/file-nr"
#define FINODE_STATE	"/proc/sys/fs/inode-state"
#define PTY_NR		"/proc/sys/kernel/pty/nr"
#define NET_DEV		"/proc/net/dev"
#define NET_SOCKSTAT	"/proc/net/sockstat"
#define NET_SOCKSTAT6	"/proc/net/sockstat6"
#define NET_RPC_NFS	"/proc/net/rpc/nfs"
#define NET_RPC_NFSD	"/proc/net/rpc/nfsd"
#define LOADAVG		"/proc/loadavg"
#define VMSTAT		"/proc/vmstat"
#define NET_SNMP	"/proc/net/snmp"
#define NET_SNMP6	"/proc/net/snmp6"
#define CPUINFO		"/proc/cpuinfo"


/*
 ***************************************************************************
 * Definitions of structures for system statistics
 ***************************************************************************
 */

/*
 * Structure for CPU statistics.
 * In activity buffer: First structure is for global CPU utilisation ("all").
 * Following structures are for each individual CPU (0, 1, etc.)
 */
struct stats_cpu {
	unsigned long long cpu_user	__attribute__ ((aligned (16)));
	unsigned long long cpu_nice	__attribute__ ((aligned (16)));
	unsigned long long cpu_sys	__attribute__ ((aligned (16)));
	unsigned long long cpu_idle	__attribute__ ((aligned (16)));
	unsigned long long cpu_iowait	__attribute__ ((aligned (16)));
	unsigned long long cpu_steal	__attribute__ ((aligned (16)));
	unsigned long long cpu_hardirq	__attribute__ ((aligned (16)));
	unsigned long long cpu_softirq	__attribute__ ((aligned (16)));
	unsigned long long cpu_guest	__attribute__ ((aligned (16)));
};

#define STATS_CPU_SIZE	(sizeof(struct stats_cpu))

/*
 * Structure for task creation and context switch statistics.
 * The attribute (aligned(16)) is necessary so that sizeof(structure) has
 * the same value on 32 and 64-bit architectures.
 */
struct stats_pcsw {
	unsigned long long context_switch	__attribute__ ((aligned (16)));
	unsigned long processes			__attribute__ ((aligned (16)));
};

#define STATS_PCSW_SIZE	(sizeof(struct stats_pcsw))

/*
 * Structure for interrupts statistics.
 * In activity buffer: First structure is for total number of interrupts ("SUM").
 * Following structures are for each individual interrupt (0, 1, etc.)
 *
 * NOTE: The total number of interrupts is saved as a %llu by the kernel,
 * whereas individual interrupts are saved as %u.
 */
struct stats_irq {
	unsigned long long irq_nr	__attribute__ ((aligned (16)));
};

#define STATS_IRQ_SIZE	(sizeof(struct stats_irq))

/* Structure for swapping statistics */
struct stats_swap {
	unsigned long pswpin	__attribute__ ((aligned (8)));
	unsigned long pswpout	__attribute__ ((aligned (8)));
};

#define STATS_SWAP_SIZE	(sizeof(struct stats_swap))

/* Structure for paging statistics */
struct stats_paging {
	unsigned long pgpgin		__attribute__ ((aligned (8)));
	unsigned long pgpgout		__attribute__ ((aligned (8)));
	unsigned long pgfault		__attribute__ ((aligned (8)));
	unsigned long pgmajfault	__attribute__ ((aligned (8)));
	unsigned long pgfree		__attribute__ ((aligned (8)));
	unsigned long pgscan_kswapd	__attribute__ ((aligned (8)));
	unsigned long pgscan_direct	__attribute__ ((aligned (8)));
	unsigned long pgsteal		__attribute__ ((aligned (8)));
};

#define STATS_PAGING_SIZE	(sizeof(struct stats_paging))

/* Structure for I/O and transfer rate statistics */
struct stats_io {
	unsigned int  dk_drive			__attribute__ ((aligned (4)));
	unsigned int  dk_drive_rio		__attribute__ ((packed));
	unsigned int  dk_drive_wio		__attribute__ ((packed));
	unsigned int  dk_drive_rblk		__attribute__ ((packed));
	unsigned int  dk_drive_wblk		__attribute__ ((packed));
};

#define STATS_IO_SIZE	(sizeof(struct stats_io))

/* Structure for memory and swap space utilization statistics */
struct stats_memory {
	unsigned long frmkb	__attribute__ ((aligned (8)));
	unsigned long bufkb	__attribute__ ((aligned (8)));
	unsigned long camkb	__attribute__ ((aligned (8)));
	unsigned long tlmkb	__attribute__ ((aligned (8)));
	unsigned long frskb	__attribute__ ((aligned (8)));
	unsigned long tlskb	__attribute__ ((aligned (8)));
	unsigned long caskb	__attribute__ ((aligned (8)));
	unsigned long comkb	__attribute__ ((aligned (8)));
	unsigned long activekb	__attribute__ ((aligned (8)));
	unsigned long inactkb	__attribute__ ((aligned (8)));
};

#define STATS_MEMORY_SIZE	(sizeof(struct stats_memory))

/* Structure for kernel tables statistics */
struct stats_ktables {
	unsigned int  file_used		__attribute__ ((aligned (4)));
	unsigned int  inode_used	__attribute__ ((packed));
	unsigned int  dentry_stat	__attribute__ ((packed));
	unsigned int  pty_nr		__attribute__ ((packed));
};

#define STATS_KTABLES_SIZE	(sizeof(struct stats_ktables))

/* Structure for queue and load statistics */
struct stats_queue {
	unsigned long nr_running	__attribute__ ((aligned (8)));
	unsigned long procs_blocked	__attribute__ ((aligned (8)));
	unsigned int  load_avg_1	__attribute__ ((aligned (8)));
	unsigned int  load_avg_5	__attribute__ ((packed));
	unsigned int  load_avg_15	__attribute__ ((packed));
	unsigned int  nr_threads	__attribute__ ((packed));
};

#define STATS_QUEUE_SIZE	(sizeof(struct stats_queue))

/* Structure for serial statistics */
struct stats_serial {
	unsigned int rx		__attribute__ ((aligned (4)));
	unsigned int tx		__attribute__ ((packed));
	unsigned int frame	__attribute__ ((packed));
	unsigned int parity	__attribute__ ((packed));
	unsigned int brk	__attribute__ ((packed));
	unsigned int overrun	__attribute__ ((packed));
	/*
	 * A value of 0 means that the structure is unused.
	 * To avoid the confusion, the line number is saved as (line# + 1)
	 */
	unsigned int line	__attribute__ ((packed));
};

#define STATS_SERIAL_SIZE	(sizeof(struct stats_serial))

/* Structure for block devices statistics */
struct stats_disk {
	unsigned long long rd_sect	__attribute__ ((aligned (16)));
	unsigned long long wr_sect	__attribute__ ((aligned (16)));
	unsigned long rd_ticks		__attribute__ ((aligned (16)));
	unsigned long wr_ticks		__attribute__ ((aligned (8)));
	unsigned long tot_ticks		__attribute__ ((aligned (8)));
	unsigned long rq_ticks		__attribute__ ((aligned (8)));
	unsigned long nr_ios		__attribute__ ((aligned (8)));
	unsigned int  major		__attribute__ ((aligned (8)));
	unsigned int  minor		__attribute__ ((packed));
};

#define STATS_DISK_SIZE	(sizeof(struct stats_disk))

/* Structure for network interfaces statistics */
struct stats_net_dev {
	unsigned long rx_packets		__attribute__ ((aligned (8)));
	unsigned long tx_packets		__attribute__ ((aligned (8)));
	unsigned long rx_bytes			__attribute__ ((aligned (8)));
	unsigned long tx_bytes			__attribute__ ((aligned (8)));
	unsigned long rx_compressed		__attribute__ ((aligned (8)));
	unsigned long tx_compressed		__attribute__ ((aligned (8)));
	unsigned long multicast			__attribute__ ((aligned (8)));
	char	      interface[MAX_IFACE_LEN]	__attribute__ ((aligned (8)));
};

#define STATS_NET_DEV_SIZE	(sizeof(struct stats_net_dev))

/* Structure for network interface errors statistics */
struct stats_net_edev {
	unsigned long collisions		__attribute__ ((aligned (8)));
	unsigned long rx_errors			__attribute__ ((aligned (8)));
	unsigned long tx_errors			__attribute__ ((aligned (8)));
	unsigned long rx_dropped		__attribute__ ((aligned (8)));
	unsigned long tx_dropped		__attribute__ ((aligned (8)));
	unsigned long rx_fifo_errors		__attribute__ ((aligned (8)));
	unsigned long tx_fifo_errors		__attribute__ ((aligned (8)));
	unsigned long rx_frame_errors		__attribute__ ((aligned (8)));
	unsigned long tx_carrier_errors		__attribute__ ((aligned (8)));
	char	      interface[MAX_IFACE_LEN]	__attribute__ ((aligned (8)));
};

#define STATS_NET_EDEV_SIZE	(sizeof(struct stats_net_edev))

/* Structure for NFS client statistics */
struct stats_net_nfs {
	unsigned int  nfs_rpccnt	__attribute__ ((aligned (4)));
	unsigned int  nfs_rpcretrans	__attribute__ ((packed));
	unsigned int  nfs_readcnt	__attribute__ ((packed));
	unsigned int  nfs_writecnt	__attribute__ ((packed));
	unsigned int  nfs_accesscnt	__attribute__ ((packed));
	unsigned int  nfs_getattcnt	__attribute__ ((packed));
};

#define STATS_NET_NFS_SIZE	(sizeof(struct stats_net_nfs))

/* Structure for NFS server statistics */
struct stats_net_nfsd {
	unsigned int  nfsd_rpccnt	__attribute__ ((aligned (4)));
	unsigned int  nfsd_rpcbad	__attribute__ ((packed));
	unsigned int  nfsd_netcnt	__attribute__ ((packed));
	unsigned int  nfsd_netudpcnt	__attribute__ ((packed));
	unsigned int  nfsd_nettcpcnt	__attribute__ ((packed));
	unsigned int  nfsd_rchits	__attribute__ ((packed));
	unsigned int  nfsd_rcmisses	__attribute__ ((packed));
	unsigned int  nfsd_readcnt	__attribute__ ((packed));
	unsigned int  nfsd_writecnt	__attribute__ ((packed));
	unsigned int  nfsd_accesscnt	__attribute__ ((packed));
	unsigned int  nfsd_getattcnt	__attribute__ ((packed));
};

#define STATS_NET_NFSD_SIZE	(sizeof(struct stats_net_nfsd))

/* Structure for IPv4 sockets statistics */
struct stats_net_sock {
	unsigned int  sock_inuse	__attribute__ ((aligned (4)));
	unsigned int  tcp_inuse		__attribute__ ((packed));
	unsigned int  tcp_tw		__attribute__ ((packed));
	unsigned int  udp_inuse		__attribute__ ((packed));
	unsigned int  raw_inuse		__attribute__ ((packed));
	unsigned int  frag_inuse	__attribute__ ((packed));
};

#define STATS_NET_SOCK_SIZE	(sizeof(struct stats_net_sock))

/* Structure for IP statistics */
struct stats_net_ip {
	unsigned long InReceives	__attribute__ ((aligned (8)));
	unsigned long ForwDatagrams	__attribute__ ((aligned (8)));
	unsigned long InDelivers	__attribute__ ((aligned (8)));
	unsigned long OutRequests	__attribute__ ((aligned (8)));
	unsigned long ReasmReqds	__attribute__ ((aligned (8)));
	unsigned long ReasmOKs		__attribute__ ((aligned (8)));
	unsigned long FragOKs		__attribute__ ((aligned (8)));
	unsigned long FragCreates	__attribute__ ((aligned (8)));
};

#define STATS_NET_IP_SIZE	(sizeof(struct stats_net_ip))

/* Structure for IP errors statistics */
struct stats_net_eip {
	unsigned long InHdrErrors	__attribute__ ((aligned (8)));
	unsigned long InAddrErrors	__attribute__ ((aligned (8)));
	unsigned long InUnknownProtos	__attribute__ ((aligned (8)));
	unsigned long InDiscards	__attribute__ ((aligned (8)));
	unsigned long OutDiscards	__attribute__ ((aligned (8)));
	unsigned long OutNoRoutes	__attribute__ ((aligned (8)));
	unsigned long ReasmFails	__attribute__ ((aligned (8)));
	unsigned long FragFails		__attribute__ ((aligned (8)));
};

#define STATS_NET_EIP_SIZE	(sizeof(struct stats_net_eip))

/* Structure for ICMP statistics */
struct stats_net_icmp {
	unsigned long InMsgs		__attribute__ ((aligned (8)));
	unsigned long OutMsgs		__attribute__ ((aligned (8)));
	unsigned long InEchos		__attribute__ ((aligned (8)));
	unsigned long InEchoReps	__attribute__ ((aligned (8)));
	unsigned long OutEchos		__attribute__ ((aligned (8)));
	unsigned long OutEchoReps	__attribute__ ((aligned (8)));
	unsigned long InTimestamps	__attribute__ ((aligned (8)));
	unsigned long InTimestampReps	__attribute__ ((aligned (8)));
	unsigned long OutTimestamps	__attribute__ ((aligned (8)));
	unsigned long OutTimestampReps	__attribute__ ((aligned (8)));
	unsigned long InAddrMasks	__attribute__ ((aligned (8)));
	unsigned long InAddrMaskReps	__attribute__ ((aligned (8)));
	unsigned long OutAddrMasks	__attribute__ ((aligned (8)));
	unsigned long OutAddrMaskReps	__attribute__ ((aligned (8)));
};

#define STATS_NET_ICMP_SIZE	(sizeof(struct stats_net_icmp))

/* Structure for ICMP error message statistics */
struct stats_net_eicmp {
	unsigned long InErrors		__attribute__ ((aligned (8)));
	unsigned long OutErrors		__attribute__ ((aligned (8)));
	unsigned long InDestUnreachs	__attribute__ ((aligned (8)));
	unsigned long OutDestUnreachs	__attribute__ ((aligned (8)));
	unsigned long InTimeExcds	__attribute__ ((aligned (8)));
	unsigned long OutTimeExcds	__attribute__ ((aligned (8)));
	unsigned long InParmProbs	__attribute__ ((aligned (8)));
	unsigned long OutParmProbs	__attribute__ ((aligned (8)));
	unsigned long InSrcQuenchs	__attribute__ ((aligned (8)));
	unsigned long OutSrcQuenchs	__attribute__ ((aligned (8)));
	unsigned long InRedirects	__attribute__ ((aligned (8)));
	unsigned long OutRedirects	__attribute__ ((aligned (8)));
};

#define STATS_NET_EICMP_SIZE	(sizeof(struct stats_net_eicmp))

/* Structure for TCP statistics */
struct stats_net_tcp {
	unsigned long ActiveOpens	__attribute__ ((aligned (8)));
	unsigned long PassiveOpens	__attribute__ ((aligned (8)));
	unsigned long InSegs		__attribute__ ((aligned (8)));
	unsigned long OutSegs		__attribute__ ((aligned (8)));
};

#define STATS_NET_TCP_SIZE	(sizeof(struct stats_net_tcp))

/* Structure for TCP errors statistics */
struct stats_net_etcp {
	unsigned long AttemptFails	__attribute__ ((aligned (8)));
	unsigned long EstabResets	__attribute__ ((aligned (8)));
	unsigned long RetransSegs	__attribute__ ((aligned (8)));
	unsigned long InErrs		__attribute__ ((aligned (8)));
	unsigned long OutRsts		__attribute__ ((aligned (8)));
};

#define STATS_NET_ETCP_SIZE	(sizeof(struct stats_net_etcp))

/* Structure for UDP statistics */
struct stats_net_udp {
	unsigned long InDatagrams	__attribute__ ((aligned (8)));
	unsigned long OutDatagrams	__attribute__ ((aligned (8)));
	unsigned long NoPorts		__attribute__ ((aligned (8)));
	unsigned long InErrors		__attribute__ ((aligned (8)));
};

#define STATS_NET_UDP_SIZE	(sizeof(struct stats_net_udp))

/* Structure for IPv6 statistics */
struct stats_net_ip6 {
	unsigned long InReceives6	__attribute__ ((aligned (8)));
	unsigned long OutForwDatagrams6	__attribute__ ((aligned (8)));
	unsigned long InDelivers6	__attribute__ ((aligned (8)));
	unsigned long OutRequests6	__attribute__ ((aligned (8)));
	unsigned long ReasmReqds6	__attribute__ ((aligned (8)));
	unsigned long ReasmOKs6		__attribute__ ((aligned (8)));
	unsigned long InMcastPkts6	__attribute__ ((aligned (8)));
	unsigned long OutMcastPkts6	__attribute__ ((aligned (8)));
	unsigned long FragOKs6		__attribute__ ((aligned (8)));
	unsigned long FragCreates6	__attribute__ ((aligned (8)));
};

#define STATS_NET_IP6_SIZE	(sizeof(struct stats_net_ip6))

/* Structure for IPv6 errors statistics */
struct stats_net_eip6 {
	unsigned long InHdrErrors6	__attribute__ ((aligned (8)));
	unsigned long InAddrErrors6	__attribute__ ((aligned (8)));
	unsigned long InUnknownProtos6	__attribute__ ((aligned (8)));
	unsigned long InTooBigErrors6	__attribute__ ((aligned (8)));
	unsigned long InDiscards6	__attribute__ ((aligned (8)));
	unsigned long OutDiscards6	__attribute__ ((aligned (8)));
	unsigned long InNoRoutes6	__attribute__ ((aligned (8)));
	unsigned long OutNoRoutes6	__attribute__ ((aligned (8)));
	unsigned long ReasmFails6	__attribute__ ((aligned (8)));
	unsigned long FragFails6	__attribute__ ((aligned (8)));
	unsigned long InTruncatedPkts6	__attribute__ ((aligned (8)));
};

#define STATS_NET_EIP6_SIZE	(sizeof(struct stats_net_eip6))

/* Structure for ICMPv6 statistics */
struct stats_net_icmp6 {
	unsigned long InMsgs6				__attribute__ ((aligned (8)));
	unsigned long OutMsgs6				__attribute__ ((aligned (8)));
	unsigned long InEchos6				__attribute__ ((aligned (8)));
	unsigned long InEchoReplies6			__attribute__ ((aligned (8)));
	unsigned long OutEchoReplies6			__attribute__ ((aligned (8)));
	unsigned long InGroupMembQueries6		__attribute__ ((aligned (8)));
	unsigned long InGroupMembResponses6		__attribute__ ((aligned (8)));
	unsigned long OutGroupMembResponses6		__attribute__ ((aligned (8)));
	unsigned long InGroupMembReductions6		__attribute__ ((aligned (8)));
	unsigned long OutGroupMembReductions6		__attribute__ ((aligned (8)));
	unsigned long InRouterSolicits6			__attribute__ ((aligned (8)));
	unsigned long OutRouterSolicits6		__attribute__ ((aligned (8)));
	unsigned long InRouterAdvertisements6		__attribute__ ((aligned (8)));
	unsigned long InNeighborSolicits6		__attribute__ ((aligned (8)));
	unsigned long OutNeighborSolicits6		__attribute__ ((aligned (8)));
	unsigned long InNeighborAdvertisements6		__attribute__ ((aligned (8)));
	unsigned long OutNeighborAdvertisements6	__attribute__ ((aligned (8)));
};

#define STATS_NET_ICMP6_SIZE	(sizeof(struct stats_net_icmp6))

/* Structure for ICMPv6 error message statistics */
struct stats_net_eicmp6 {
	unsigned long InErrors6		__attribute__ ((aligned (8)));
	unsigned long InDestUnreachs6	__attribute__ ((aligned (8)));
	unsigned long OutDestUnreachs6	__attribute__ ((aligned (8)));
	unsigned long InTimeExcds6	__attribute__ ((aligned (8)));
	unsigned long OutTimeExcds6	__attribute__ ((aligned (8)));
	unsigned long InParmProblems6	__attribute__ ((aligned (8)));
	unsigned long OutParmProblems6	__attribute__ ((aligned (8)));
	unsigned long InRedirects6	__attribute__ ((aligned (8)));
	unsigned long OutRedirects6	__attribute__ ((aligned (8)));
	unsigned long InPktTooBigs6	__attribute__ ((aligned (8)));
	unsigned long OutPktTooBigs6	__attribute__ ((aligned (8)));
};

#define STATS_NET_EICMP6_SIZE	(sizeof(struct stats_net_eicmp6))

/* Structure for UDPv6 statistics */
struct stats_net_udp6 {
	unsigned long InDatagrams6	__attribute__ ((aligned (8)));
	unsigned long OutDatagrams6	__attribute__ ((aligned (8)));
	unsigned long NoPorts6		__attribute__ ((aligned (8)));
	unsigned long InErrors6		__attribute__ ((aligned (8)));
};

#define STATS_NET_UDP6_SIZE	(sizeof(struct stats_net_udp6))

/* Structure for IPv6 sockets statistics */
struct stats_net_sock6 {
	unsigned int  tcp6_inuse	__attribute__ ((aligned (4)));
	unsigned int  udp6_inuse	__attribute__ ((packed));
	unsigned int  raw6_inuse	__attribute__ ((packed));
	unsigned int  frag6_inuse	__attribute__ ((packed));
};

#define STATS_NET_SOCK6_SIZE	(sizeof(struct stats_net_sock6))

/*
 * Structure for CPU frequency statistics.
 * In activity buffer: First structure is for global CPU utilisation ("all").
 * Following structures are for each individual CPU (0, 1, etc.)
 */
struct stats_pwr_cpufreq {
	unsigned long cpufreq	__attribute__ ((aligned (8)));
};

#define STATS_PWR_CPUFREQ_SIZE	(sizeof(struct stats_pwr_cpufreq))

/* Structure for hugepages statistics */
struct stats_huge {
	unsigned long frhkb			__attribute__ ((aligned (8)));
	unsigned long tlhkb			__attribute__ ((aligned (8)));
};

#define STATS_HUGE_SIZE	(sizeof(struct stats_memory))

/*
 * Structure for weighted CPU frequency statistics.
 * In activity buffer: First structure is for global CPU utilisation ("all").
 * Following structures are for each individual CPU (0, 1, etc.)
 */
struct stats_pwr_wghfreq {
	unsigned long long 	time_in_state	__attribute__ ((aligned (16)));
	unsigned long 		freq		__attribute__ ((aligned (16)));
};

#define STATS_PWR_WGHFREQ_SIZE	(sizeof(struct stats_pwr_wghfreq))

/*
 * Structure for USB devices plugged into the system.
 */
struct stats_pwr_usb {
	unsigned int  bus_nr				__attribute__ ((aligned (4)));
	unsigned int  vendor_id				__attribute__ ((packed));
	unsigned int  product_id			__attribute__ ((packed));
	unsigned int  bmaxpower				__attribute__ ((packed));
	char	      manufacturer[MAX_MANUF_LEN];
	char	      product[MAX_PROD_LEN];
};

#define STATS_PWR_USB_SIZE	(sizeof(struct stats_pwr_usb))

/*
 ***************************************************************************
 * Prototypes for functions used to read system statistics
 ***************************************************************************
 */

extern void
	read_stat_cpu(struct stats_cpu *, int,
		      unsigned long long *, unsigned long long *);
extern void
	read_stat_pcsw(struct stats_pcsw *);
extern void
	read_stat_irq(struct stats_irq *, int);
extern void
	read_loadavg(struct stats_queue *);
extern void
	read_meminfo(struct stats_memory *);
extern void
	read_vmstat_swap(struct stats_swap *);
extern void
	read_vmstat_paging(struct stats_paging *);
extern void
	read_diskstats_io(struct stats_io *);
extern void
	read_diskstats_disk(struct stats_disk *, int, int);
extern void
	read_tty_driver_serial(struct stats_serial *, int);
extern void
	read_kernel_tables(struct stats_ktables *);
extern void
	read_net_dev(struct stats_net_dev *, int);
extern void
	read_net_edev(struct stats_net_edev *, int);
extern void
	read_net_nfs(struct stats_net_nfs *);
extern void
	read_net_nfsd(struct stats_net_nfsd *);
extern void
	read_net_sock(struct stats_net_sock *);
extern void
	read_net_ip(struct stats_net_ip *);
extern void
	read_net_eip(struct stats_net_eip *);
extern void
	read_net_icmp(struct stats_net_icmp *);
extern void
	read_net_eicmp(struct stats_net_eicmp *);
extern void
	read_net_tcp(struct stats_net_tcp *);
extern void
	read_net_etcp(struct stats_net_etcp *);
extern void
	read_net_udp(struct stats_net_udp *);
extern void
	read_uptime(unsigned long long *);
extern void
	read_net_sock6(struct stats_net_sock6 *);
extern void
	read_net_ip6(struct stats_net_ip6 *);
extern void
	read_net_eip6(struct stats_net_eip6 *);
extern void
	read_net_icmp6(struct stats_net_icmp6 *);
extern void
	read_net_eicmp6(struct stats_net_eicmp6 *);
extern void
	read_net_udp6(struct stats_net_udp6 *);
extern void
	read_cpuinfo(struct stats_pwr_cpufreq *, int);
extern void
	read_meminfo_huge(struct stats_huge *);
extern void
	read_time_in_state(struct stats_pwr_wghfreq *, int, int);
extern void
	read_bus_usb_dev(struct stats_pwr_usb *, int);

/*
 ***************************************************************************
 * Prototypes for functions used to count number of items
 ***************************************************************************
 */

extern int
	get_irq_nr(void);
extern int
	get_serial_nr(void);
extern int
	get_iface_nr(void);
extern int
	get_diskstats_dev_nr(int, int);
extern int
	get_disk_nr(unsigned int);
extern int
	get_cpu_nr(unsigned int);
extern int
	get_irqcpu_nr(char *, int, int);
extern int
	get_freq_nr(void);
extern int
	get_usb_nr(void);

#endif /* _RD_STATS_H */
