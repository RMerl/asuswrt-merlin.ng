/*
 * activity.c: Define system activities available for sar/sadc.
 * (C) 1999-2011 by Sebastien GODARD (sysstat <at> orange.fr)
 *
 ***************************************************************************
 * This program is free software; you can redistribute it and/or modify it *
 * under the terms of the GNU General Public License as published  by  the *
 * Free Software Foundation; either version 2 of the License, or (at  your *
 * option) any later version.                                              *
 *                                                                         *
 * This program is distributed in the hope that it  will  be  useful,  but *
 * WITHOUT ANY WARRANTY; without the implied warranty  of  MERCHANTABILITY *
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License *
 * for more details.                                                       *
 *                                                                         *
 * You should have received a copy of the GNU General Public License along *
 * with this program; if not, write to the Free Software Foundation, Inc., *
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA                   *
 ***************************************************************************
 */

#include "sa.h"

#ifdef SOURCE_SADC
#include "rd_stats.h"
#include "rd_sensors.h"
#endif

#ifdef SOURCE_SAR
#include "pr_stats.h"
#endif

#ifdef SOURCE_SADF
#include "rndr_stats.h"
#include "xml_stats.h"
#include "json_stats.h"
#endif

/*
 ***************************************************************************
 * Definitions of system activities.
 * See sa.h file for activity structure definition.
 * Activity structure doesn't matter for daily data files.
 ***************************************************************************
 */

/*
 * Bitmaps needed by activities.
 * Remember to allocate them before use!
 */

/* CPU bitmap */
struct act_bitmap cpu_bitmap = {
	.b_array	= NULL,
	.b_size		= NR_CPUS
};

/* Interrupts bitmap */
struct act_bitmap irq_bitmap = {
	.b_array	= NULL,
	.b_size		= NR_IRQS
};


/*
 * CPU statistics.
 * This is the only activity which *must* be collected by sadc
 * so that uptime can be filled.
 */
struct activity cpu_act = {
	.id		= A_CPU,
	.options	= AO_COLLECTED + AO_REMANENT + AO_GLOBAL_ITV + AO_MULTIPLE_OUTPUTS,
	.magic		= ACTIVITY_MAGIC_BASE,
	.group		= G_DEFAULT,
#ifdef SOURCE_SADC
	.f_count	= wrap_get_cpu_nr,
	.f_count2	= NULL,
	.f_read		= wrap_read_stat_cpu,
#endif
#ifdef SOURCE_SAR
	.f_print	= print_cpu_stats,
	.f_print_avg	= print_cpu_stats,
#endif
#ifdef SOURCE_SADF
	.f_render	= render_cpu_stats,
	.f_xml_print	= xml_print_cpu_stats,
	.f_json_print	= json_print_cpu_stats,
	.hdr_line	= "CPU;%user;%nice;%system;%iowait;%steal;%idle|"
		          "CPU;%usr;%nice;%sys;%iowait;%steal;%irq;%soft;%guest;%idle",
	.name		= "A_CPU",
#endif
	.nr		= -1,
	.nr2		= 1,
	.fsize		= STATS_CPU_SIZE,
	.msize		= STATS_CPU_SIZE,
	.opt_flags	= AO_F_CPU_DEF,
	.buf		= {NULL, NULL, NULL},
	.bitmap		= &cpu_bitmap
};

/* Process (task) creation and context switch activity */
struct activity pcsw_act = {
	.id		= A_PCSW,
	.options	= AO_COLLECTED,
	.magic		= ACTIVITY_MAGIC_BASE,
	.group		= G_DEFAULT,
#ifdef SOURCE_SADC
	.f_count	= NULL,
	.f_count2	= NULL,
	.f_read		= wrap_read_stat_pcsw,
#endif
#ifdef SOURCE_SAR
	.f_print	= print_pcsw_stats,
	.f_print_avg	= print_pcsw_stats,
#endif
#ifdef SOURCE_SADF
	.f_render	= render_pcsw_stats,
	.f_xml_print	= xml_print_pcsw_stats,
	.f_json_print	= json_print_pcsw_stats,
	.hdr_line	= "proc/s;cswch/s",
	.name		= "A_PCSW",
#endif
	.nr		= 1,
	.nr2		= 1,
	.fsize		= STATS_PCSW_SIZE,
	.msize		= STATS_PCSW_SIZE,
	.opt_flags	= 0,
	.buf		= {NULL, NULL, NULL},
	.bitmap		= NULL
};

/* Interrupts statistics */
struct activity irq_act = {
	.id		= A_IRQ,
	.options	= AO_NULL,
	.magic		= ACTIVITY_MAGIC_BASE,
	.group		= G_INT,
#ifdef SOURCE_SADC
	.f_count	= wrap_get_irq_nr,
	.f_count2	= NULL,
	.f_read		= wrap_read_stat_irq,
#endif
#ifdef SOURCE_SAR
	.f_print	= print_irq_stats,
	.f_print_avg	= print_irq_stats,
#endif
#ifdef SOURCE_SADF
	.f_render	= render_irq_stats,
	.f_xml_print	= xml_print_irq_stats,
	.f_json_print	= json_print_irq_stats,
	.hdr_line	= "INTR;intr/s",
	.name		= "A_IRQ",
#endif
	.nr		= -1,
	.nr2		= 1,
	.fsize		= STATS_IRQ_SIZE,
	.msize		= STATS_IRQ_SIZE,
	.opt_flags	= 0,
	.buf		= {NULL, NULL, NULL},
	.bitmap		= &irq_bitmap
};

/* Swapping activity */
struct activity swap_act = {
	.id		= A_SWAP,
	.options	= AO_COLLECTED,
	.magic		= ACTIVITY_MAGIC_BASE,
	.group		= G_DEFAULT,
#ifdef SOURCE_SADC
	.f_count	= NULL,
	.f_count2	= NULL,
	.f_read		= wrap_read_swap,
#endif
#ifdef SOURCE_SAR
	.f_print	= print_swap_stats,
	.f_print_avg	= print_swap_stats,
#endif
#ifdef SOURCE_SADF
	.f_render	= render_swap_stats,
	.f_xml_print	= xml_print_swap_stats,
	.f_json_print	= json_print_swap_stats,
	.hdr_line	= "pswpin/s;pswpout/s",
	.name		= "A_SWAP",
#endif
	.nr		= 1,
	.nr2		= 1,
	.fsize		= STATS_SWAP_SIZE,
	.msize		= STATS_SWAP_SIZE,
	.opt_flags	= 0,
	.buf		= {NULL, NULL, NULL},
	.bitmap		= NULL
};

/* Paging activity */
struct activity paging_act = {
	.id		= A_PAGE,
	.options	= AO_COLLECTED,
	.magic		= ACTIVITY_MAGIC_BASE,
	.group		= G_DEFAULT,
#ifdef SOURCE_SADC
	.f_count	= NULL,
	.f_count2	= NULL,
	.f_read		= wrap_read_paging,
#endif
#ifdef SOURCE_SAR
	.f_print	= print_paging_stats,
	.f_print_avg	= print_paging_stats,
#endif
#ifdef SOURCE_SADF
	.f_render	= render_paging_stats,
	.f_xml_print	= xml_print_paging_stats,
	.f_json_print	= json_print_paging_stats,
	.hdr_line	= "pgpgin/s;pgpgout/s;fault/s;majflt/s;"
		          "pgfree/s;pgscank/s;pgscand/s;pgsteal/s;%vmeff",
	.name		= "A_PAGE",
#endif
	.nr		= 1,
	.nr2		= 1,
	.fsize		= STATS_PAGING_SIZE,
	.msize		= STATS_PAGING_SIZE,
	.opt_flags	= 0,
	.buf		= {NULL, NULL, NULL},
	.bitmap		= NULL
};

/* I/O and transfer rate activity */
struct activity io_act = {
	.id		= A_IO,
	.options	= AO_COLLECTED,
	.magic		= ACTIVITY_MAGIC_BASE,
	.group		= G_DEFAULT,
#ifdef SOURCE_SADC
	.f_count	= NULL,
	.f_count2	= NULL,
	.f_read		= wrap_read_io,
#endif
#ifdef SOURCE_SAR
	.f_print	= print_io_stats,
	.f_print_avg	= print_io_stats,
#endif
#ifdef SOURCE_SADF
	.f_render	= render_io_stats,
	.f_xml_print	= xml_print_io_stats,
	.f_json_print	= json_print_io_stats,
	.hdr_line	= "tps;rtps;wtps;bread/s;bwrtn/s",
	.name		= "A_IO",
#endif
	.nr		= 1,
	.nr2		= 1,
	.fsize		= STATS_IO_SIZE,
	.msize		= STATS_IO_SIZE,
	.opt_flags	= 0,
	.buf		= {NULL, NULL, NULL},
	.bitmap		= NULL
};

/* Memory and swap space utilization activity */
struct activity memory_act = {
	.id		= A_MEMORY,
	.options	= AO_COLLECTED + AO_MULTIPLE_OUTPUTS,
	.magic		= ACTIVITY_MAGIC_BASE,
	.group		= G_DEFAULT,
#ifdef SOURCE_SADC
	.f_count	= NULL,
	.f_count2	= NULL,
	.f_read		= wrap_read_meminfo,
#endif
#ifdef SOURCE_SAR
	.f_print	= print_memory_stats,
	.f_print_avg	= print_avg_memory_stats,
#endif
#ifdef SOURCE_SADF
	.f_render	= render_memory_stats,
	.f_xml_print	= xml_print_memory_stats,
	.f_json_print	= json_print_memory_stats,
	.hdr_line	= "frmpg/s;bufpg/s;campg/s|"
		          "kbmemfree;kbmemused;%memused;kbbuffers;kbcached;kbcommit;%commit;kbactive;kbinact|"
		          "kbswpfree;kbswpused;%swpused;kbswpcad;%swpcad",
	.name		= "A_MEMORY",
#endif
	.nr		= 1,
	.nr2		= 1,
	.fsize		= STATS_MEMORY_SIZE,
	.msize		= STATS_MEMORY_SIZE,
	.opt_flags	= 0,
	.buf		= {NULL, NULL, NULL},
	.bitmap		= NULL
};

/* Kernel tables activity */
struct activity ktables_act = {
	.id		= A_KTABLES,
	.options	= AO_COLLECTED,
	.magic		= ACTIVITY_MAGIC_BASE,
	.group		= G_DEFAULT,
#ifdef SOURCE_SADC
	.f_count	= NULL,
	.f_count2	= NULL,
	.f_read		= wrap_read_kernel_tables,
#endif
#ifdef SOURCE_SAR
	.f_print	= print_ktables_stats,
	.f_print_avg	= print_avg_ktables_stats,
#endif
#ifdef SOURCE_SADF
	.f_render	= render_ktables_stats,
	.f_xml_print	= xml_print_ktables_stats,
	.f_json_print	= json_print_ktables_stats,
	.hdr_line	= "dentunusd;file-nr;inode-nr;pty-nr",
	.name		= "A_KTABLES",
#endif
	.nr		= 1,
	.nr2		= 1,
	.fsize		= STATS_KTABLES_SIZE,
	.msize		= STATS_KTABLES_SIZE,
	.opt_flags	= 0,
	.buf		= {NULL, NULL, NULL},
	.bitmap		= NULL
};

/* Queue and load activity */
struct activity queue_act = {
	.id		= A_QUEUE,
	.options	= AO_COLLECTED,
	.magic		= ACTIVITY_MAGIC_BASE + 1,
	.group		= G_DEFAULT,
#ifdef SOURCE_SADC
	.f_count	= NULL,
	.f_count2	= NULL,
	.f_read		= wrap_read_loadavg,
#endif
#ifdef SOURCE_SAR
	.f_print	= print_queue_stats,
	.f_print_avg	= print_avg_queue_stats,
#endif
#ifdef SOURCE_SADF
	.f_render	= render_queue_stats,
	.f_xml_print	= xml_print_queue_stats,
	.f_json_print	= json_print_queue_stats,
	.hdr_line	= "runq-sz;plist-sz;ldavg-1;ldavg-5;ldavg-15;blocked",
	.name		= "A_QUEUE",
#endif
	.nr		= 1,
	.nr2		= 1,
	.fsize		= STATS_QUEUE_SIZE,
	.msize		= STATS_QUEUE_SIZE,
	.opt_flags	= 0,
	.buf		= {NULL, NULL, NULL},
	.bitmap		= NULL
};

/* Serial lines activity */
struct activity serial_act = {
	.id		= A_SERIAL,
	.options	= AO_COLLECTED,
	.magic		= ACTIVITY_MAGIC_BASE,
	.group		= G_DEFAULT,
#ifdef SOURCE_SADC
	.f_count	= wrap_get_serial_nr,
	.f_count2	= NULL,
	.f_read		= wrap_read_tty_driver_serial,
#endif
#ifdef SOURCE_SAR
	.f_print	= print_serial_stats,
	.f_print_avg	= print_serial_stats,
#endif
#ifdef SOURCE_SADF
	.f_render	= render_serial_stats,
	.f_xml_print	= xml_print_serial_stats,
	.f_json_print	= json_print_serial_stats,
	.hdr_line	= "TTY;rcvin/s;txmtin/s;framerr/s;prtyerr/s;brk/s;ovrun/s",
	.name		= "A_SERIAL",
#endif
	.nr		= -1,
	.nr2		= 1,
	.fsize		= STATS_SERIAL_SIZE,
	.msize		= STATS_SERIAL_SIZE,
	.opt_flags	= 0,
	.buf		= {NULL, NULL, NULL},
	.bitmap		= NULL
};

/* Block devices activity */
struct activity disk_act = {
	.id		= A_DISK,
	.options	= AO_NULL,
	.magic		= ACTIVITY_MAGIC_BASE,
	.group		= G_DISK,
#ifdef SOURCE_SADC
	.f_count	= wrap_get_disk_nr,
	.f_count2	= NULL,
	.f_read		= wrap_read_disk,
#endif
#ifdef SOURCE_SAR
	.f_print	= print_disk_stats,
	.f_print_avg	= print_disk_stats,
#endif
#ifdef SOURCE_SADF
	.f_render	= render_disk_stats,
	.f_xml_print	= xml_print_disk_stats,
	.f_json_print	= json_print_disk_stats,
	.hdr_line	= "DEV;tps;rd_sec/s;wr_sec/s;avgrq-sz;avgqu-sz;await;svctm;%util",
	.name		= "A_DISK",
#endif
	.nr		= -1,
	.nr2		= 1,
	.fsize		= STATS_DISK_SIZE,
	.msize		= STATS_DISK_SIZE,
	.opt_flags	= 0,
	.buf		= {NULL, NULL, NULL},
	.bitmap		= NULL
};

/* Network interfaces activity */
struct activity net_dev_act = {
	.id		= A_NET_DEV,
	.options	= AO_COLLECTED,
	.magic		= ACTIVITY_MAGIC_BASE,
	.group		= G_DEFAULT,
#ifdef SOURCE_SADC
	.f_count	= wrap_get_iface_nr,
	.f_count2	= NULL,
	.f_read		= wrap_read_net_dev,
#endif
#ifdef SOURCE_SAR
	.f_print	= print_net_dev_stats,
	.f_print_avg	= print_net_dev_stats,
#endif
#ifdef SOURCE_SADF
	.f_render	= render_net_dev_stats,
	.f_xml_print	= xml_print_net_dev_stats,
	.f_json_print	= json_print_net_dev_stats,
	.hdr_line	= "IFACE;rxpck/s;txpck/s;rxkB/s;txkB/s;rxcmp/s;txcmp/s;rxmcst/s",
	.name		= "A_NET_DEV",
#endif
	.nr		= -1,
	.nr2		= 1,
	.fsize		= STATS_NET_DEV_SIZE,
	.msize		= STATS_NET_DEV_SIZE,
	.opt_flags	= 0,
	.buf		= {NULL, NULL, NULL},
	.bitmap		= NULL
};

/* Network interfaces activity */
struct activity net_edev_act = {
	.id		= A_NET_EDEV,
	.options	= AO_COLLECTED,
	.magic		= ACTIVITY_MAGIC_BASE,
	.group		= G_DEFAULT,
#ifdef SOURCE_SADC
	.f_count	= wrap_get_iface_nr,
	.f_count2	= NULL,
	.f_read		= wrap_read_net_edev,
#endif
#ifdef SOURCE_SAR
	.f_print	= print_net_edev_stats,
	.f_print_avg	= print_net_edev_stats,
#endif
#ifdef SOURCE_SADF
	.f_render	= render_net_edev_stats,
	.f_xml_print	= xml_print_net_edev_stats,
	.f_json_print	= json_print_net_edev_stats,
	.hdr_line	= "IFACE;rxerr/s;txerr/s;coll/s;rxdrop/s;txdrop/s;"
		          "txcarr/s;rxfram/s;rxfifo/s;txfifo/s",
	.name		= "A_NET_EDEV",
#endif
	.nr		= -1,
	.nr2		= 1,
	.fsize		= STATS_NET_EDEV_SIZE,
	.msize		= STATS_NET_EDEV_SIZE,
	.opt_flags	= 0,
	.buf		= {NULL, NULL, NULL},
	.bitmap		= NULL
};

/* NFS client activity */
struct activity net_nfs_act = {
	.id		= A_NET_NFS,
	.options	= AO_COLLECTED,
	.magic		= ACTIVITY_MAGIC_BASE,
	.group		= G_DEFAULT,
#ifdef SOURCE_SADC
	.f_count	= NULL,
	.f_count2	= NULL,
	.f_read		= wrap_read_net_nfs,
#endif
#ifdef SOURCE_SAR
	.f_print	= print_net_nfs_stats,
	.f_print_avg	= print_net_nfs_stats,
#endif
#ifdef SOURCE_SADF
	.f_render	= render_net_nfs_stats,
	.f_xml_print	= xml_print_net_nfs_stats,
	.f_json_print	= json_print_net_nfs_stats,
	.hdr_line	= "call/s;retrans/s;read/s;write/s;access/s;getatt/s",
	.name		= "A_NET_NFS",
#endif
	.nr		= 1,
	.nr2		= 1,
	.fsize		= STATS_NET_NFS_SIZE,
	.msize		= STATS_NET_NFS_SIZE,
	.opt_flags	= 0,
	.buf		= {NULL, NULL, NULL},
	.bitmap		= NULL
};

/* NFS server activity */
struct activity net_nfsd_act = {
	.id		= A_NET_NFSD,
	.options	= AO_COLLECTED,
	.magic		= ACTIVITY_MAGIC_BASE,
	.group		= G_DEFAULT,
#ifdef SOURCE_SADC
	.f_count	= NULL,
	.f_count2	= NULL,
	.f_read		= wrap_read_net_nfsd,
#endif
#ifdef SOURCE_SAR
	.f_print	= print_net_nfsd_stats,
	.f_print_avg	= print_net_nfsd_stats,
#endif
#ifdef SOURCE_SADF
	.f_render	= render_net_nfsd_stats,
	.f_xml_print	= xml_print_net_nfsd_stats,
	.f_json_print	= json_print_net_nfsd_stats,
	.hdr_line	= "scall/s;badcall/s;packet/s;udp/s;tcp/s;hit/s;miss/s;"
		          "sread/s;swrite/s;saccess/s;sgetatt/s",
	.name		= "A_NET_NFSD",
#endif
	.nr		= 1,
	.nr2		= 1,
	.fsize		= STATS_NET_NFSD_SIZE,
	.msize		= STATS_NET_NFSD_SIZE,
	.opt_flags	= 0,
	.buf		= {NULL, NULL, NULL},
	.bitmap		= NULL
};

/* Network sockets activity */
struct activity net_sock_act = {
	.id		= A_NET_SOCK,
	.options	= AO_COLLECTED,
	.magic		= ACTIVITY_MAGIC_BASE,
	.group		= G_DEFAULT,
#ifdef SOURCE_SADC
	.f_count	= NULL,
	.f_count2	= NULL,
	.f_read		= wrap_read_net_sock,
#endif
#ifdef SOURCE_SAR
	.f_print	= print_net_sock_stats,
	.f_print_avg	= print_avg_net_sock_stats,
#endif
#ifdef SOURCE_SADF
	.f_render	= render_net_sock_stats,
	.f_xml_print	= xml_print_net_sock_stats,
	.f_json_print	= json_print_net_sock_stats,
	.hdr_line	= "totsck;tcpsck;udpsck;rawsck;ip-frag;tcp-tw",
	.name		= "A_NET_SOCK",
#endif
	.nr		= 1,
	.nr2		= 1,
	.fsize		= STATS_NET_SOCK_SIZE,
	.msize		= STATS_NET_SOCK_SIZE,
	.opt_flags	= 0,
	.buf		= {NULL, NULL, NULL},
	.bitmap		= NULL
};

/* IP network traffic activity */
struct activity net_ip_act = {
	.id		= A_NET_IP,
	.options	= AO_NULL,
	.magic		= ACTIVITY_MAGIC_BASE,
	.group		= G_SNMP,
#ifdef SOURCE_SADC
	.f_count	= NULL,
	.f_count2	= NULL,
	.f_read		= wrap_read_net_ip,
#endif
#ifdef SOURCE_SAR
	.f_print	= print_net_ip_stats,
	.f_print_avg	= print_net_ip_stats,
#endif
#ifdef SOURCE_SADF
	.f_render	= render_net_ip_stats,
	.f_xml_print	= xml_print_net_ip_stats,
	.f_json_print	= json_print_net_ip_stats,
	.hdr_line	= "irec/s;fwddgm/s;idel/s;orq/s;asmrq/s;asmok/s;fragok/s;fragcrt/s",
	.name		= "A_NET_IP",
#endif
	.nr		= 1,
	.nr2		= 1,
	.fsize		= STATS_NET_IP_SIZE,
	.msize		= STATS_NET_IP_SIZE,
	.opt_flags	= 0,
	.buf		= {NULL, NULL, NULL},
	.bitmap		= NULL
};

/* IP network traffic (errors) activity */
struct activity net_eip_act = {
	.id		= A_NET_EIP,
	.options	= AO_NULL,
	.magic		= ACTIVITY_MAGIC_BASE,
	.group		= G_SNMP,
#ifdef SOURCE_SADC
	.f_count	= NULL,
	.f_count2	= NULL,
	.f_read		= wrap_read_net_eip,
#endif
#ifdef SOURCE_SAR
	.f_print	= print_net_eip_stats,
	.f_print_avg	= print_net_eip_stats,
#endif
#ifdef SOURCE_SADF
	.f_render	= render_net_eip_stats,
	.f_xml_print	= xml_print_net_eip_stats,
	.f_json_print	= json_print_net_eip_stats,
	.hdr_line	= "ihdrerr/s;iadrerr/s;iukwnpr/s;idisc/s;odisc/s;onort/s;asmf/s;fragf/s",
	.name		= "A_NET_EIP",
#endif
	.nr		= 1,
	.nr2		= 1,
	.fsize		= STATS_NET_EIP_SIZE,
	.msize		= STATS_NET_EIP_SIZE,
	.opt_flags	= 0,
	.buf		= {NULL, NULL, NULL},
	.bitmap		= NULL
};

/* ICMP network traffic activity */
struct activity net_icmp_act = {
	.id		= A_NET_ICMP,
	.options	= AO_NULL,
	.magic		= ACTIVITY_MAGIC_BASE,
	.group		= G_SNMP,
#ifdef SOURCE_SADC
	.f_count	= NULL,
	.f_count2	= NULL,
	.f_read		= wrap_read_net_icmp,
#endif
#ifdef SOURCE_SAR
	.f_print	= print_net_icmp_stats,
	.f_print_avg	= print_net_icmp_stats,
#endif
#ifdef SOURCE_SADF
	.f_render	= render_net_icmp_stats,
	.f_xml_print	= xml_print_net_icmp_stats,
	.f_json_print	= json_print_net_icmp_stats,
	.hdr_line	= "imsg/s;omsg/s;iech/s;iechr/s;oech/s;oechr/s;itm/s;itmr/s;otm/s;"
		          "otmr/s;iadrmk/s;iadrmkr/s;oadrmk/s;oadrmkr/s",
	.name		= "A_NET_ICMP",
#endif
	.nr		= 1,
	.nr2		= 1,
	.fsize		= STATS_NET_ICMP_SIZE,
	.msize		= STATS_NET_ICMP_SIZE,
	.opt_flags	= 0,
	.buf		= {NULL, NULL, NULL},
	.bitmap		= NULL
};

/* ICMP network traffic (errors) activity */
struct activity net_eicmp_act = {
	.id		= A_NET_EICMP,
	.options	= AO_NULL,
	.magic		= ACTIVITY_MAGIC_BASE,
	.group		= G_SNMP,
#ifdef SOURCE_SADC
	.f_count	= NULL,
	.f_count2	= NULL,
	.f_read		= wrap_read_net_eicmp,
#endif
#ifdef SOURCE_SAR
	.f_print	= print_net_eicmp_stats,
	.f_print_avg	= print_net_eicmp_stats,
#endif
#ifdef SOURCE_SADF
	.f_render	= render_net_eicmp_stats,
	.f_xml_print	= xml_print_net_eicmp_stats,
	.f_json_print	= json_print_net_eicmp_stats,
	.hdr_line	= "ierr/s;oerr/s;idstunr/s;odstunr/s;itmex/s;otmex/s;"
		          "iparmpb/s;oparmpb/s;isrcq/s;osrcq/s;iredir/s;oredir/s",
	.name		= "A_NET_EICMP",
#endif
	.nr		= 1,
	.nr2		= 1,
	.fsize		= STATS_NET_EICMP_SIZE,
	.msize		= STATS_NET_EICMP_SIZE,
	.opt_flags	= 0,
	.buf		= {NULL, NULL, NULL},
	.bitmap		= NULL
};

/* TCP network traffic activity */
struct activity net_tcp_act = {
	.id		= A_NET_TCP,
	.options	= AO_NULL,
	.magic		= ACTIVITY_MAGIC_BASE,
	.group		= G_SNMP,
#ifdef SOURCE_SADC
	.f_count	= NULL,
	.f_count2	= NULL,
	.f_read		= wrap_read_net_tcp,
#endif
#ifdef SOURCE_SAR
	.f_print	= print_net_tcp_stats,
	.f_print_avg	= print_net_tcp_stats,
#endif
#ifdef SOURCE_SADF
	.f_render	= render_net_tcp_stats,
	.f_xml_print	= xml_print_net_tcp_stats,
	.f_json_print	= json_print_net_tcp_stats,
	.hdr_line	= "active/s;passive/s;iseg/s;oseg/s",
	.name		= "A_NET_TCP",
#endif
	.nr		= 1,
	.nr2		= 1,
	.fsize		= STATS_NET_TCP_SIZE,
	.msize		= STATS_NET_TCP_SIZE,
	.opt_flags	= 0,
	.buf		= {NULL, NULL, NULL},
	.bitmap		= NULL
};

/* TCP network traffic (errors) activity */
struct activity net_etcp_act = {
	.id		= A_NET_ETCP,
	.options	= AO_NULL,
	.magic		= ACTIVITY_MAGIC_BASE,
	.group		= G_SNMP,
#ifdef SOURCE_SADC
	.f_count	= NULL,
	.f_count2	= NULL,
	.f_read		= wrap_read_net_etcp,
#endif
#ifdef SOURCE_SAR
	.f_print	= print_net_etcp_stats,
	.f_print_avg	= print_net_etcp_stats,
#endif
#ifdef SOURCE_SADF
	.f_render	= render_net_etcp_stats,
	.f_xml_print	= xml_print_net_etcp_stats,
	.f_json_print	= json_print_net_etcp_stats,
	.hdr_line	= "atmptf/s;estres/s;retrans/s;isegerr/s;orsts/s",
	.name		= "A_NET_ETCP",
#endif
	.nr		= 1,
	.nr2		= 1,
	.fsize		= STATS_NET_ETCP_SIZE,
	.msize		= STATS_NET_ETCP_SIZE,
	.opt_flags	= 0,
	.buf		= {NULL, NULL, NULL},
	.bitmap		= NULL
};

/* UDP network traffic activity */
struct activity net_udp_act = {
	.id		= A_NET_UDP,
	.options	= AO_NULL,
	.magic		= ACTIVITY_MAGIC_BASE,
	.group		= G_SNMP,
#ifdef SOURCE_SADC
	.f_count	= NULL,
	.f_count2	= NULL,
	.f_read		= wrap_read_net_udp,
#endif
#ifdef SOURCE_SAR
	.f_print	= print_net_udp_stats,
	.f_print_avg	= print_net_udp_stats,
#endif
#ifdef SOURCE_SADF
	.f_render	= render_net_udp_stats,
	.f_xml_print	= xml_print_net_udp_stats,
	.f_json_print	= json_print_net_udp_stats,
	.hdr_line	= "idgm/s;odgm/s;noport/s;idgmerr/s",
	.name		= "A_NET_UDP",
#endif
	.nr		= 1,
	.nr2		= 1,
	.fsize		= STATS_NET_UDP_SIZE,
	.msize		= STATS_NET_UDP_SIZE,
	.opt_flags	= 0,
	.buf		= {NULL, NULL, NULL},
	.bitmap		= NULL
};

/* IPv6 sockets activity */
struct activity net_sock6_act = {
	.id		= A_NET_SOCK6,
	.options	= AO_NULL,
	.magic		= ACTIVITY_MAGIC_BASE,
	.group		= G_IPV6,
#ifdef SOURCE_SADC
	.f_count	= NULL,
	.f_count2	= NULL,
	.f_read		= wrap_read_net_sock6,
#endif
#ifdef SOURCE_SAR
	.f_print	= print_net_sock6_stats,
	.f_print_avg	= print_avg_net_sock6_stats,
#endif
#ifdef SOURCE_SADF
	.f_render	= render_net_sock6_stats,
	.f_xml_print	= xml_print_net_sock6_stats,
	.f_json_print	= json_print_net_sock6_stats,
	.hdr_line	= "tcp6sck;udp6sck;raw6sck;ip6-frag",
	.name		= "A_NET_SOCK6",
#endif
	.nr		= 1,
	.nr2		= 1,
	.fsize		= STATS_NET_SOCK6_SIZE,
	.msize		= STATS_NET_SOCK6_SIZE,
	.opt_flags	= 0,
	.buf		= {NULL, NULL, NULL},
	.bitmap		= NULL
};

/* IPv6 network traffic activity */
struct activity net_ip6_act = {
	.id		= A_NET_IP6,
	.options	= AO_NULL,
	.magic		= ACTIVITY_MAGIC_BASE,
	.group		= G_IPV6,
#ifdef SOURCE_SADC
	.f_count	= NULL,
	.f_count2	= NULL,
	.f_read		= wrap_read_net_ip6,
#endif
#ifdef SOURCE_SAR
	.f_print	= print_net_ip6_stats,
	.f_print_avg	= print_net_ip6_stats,
#endif
#ifdef SOURCE_SADF
	.f_render	= render_net_ip6_stats,
	.f_xml_print	= xml_print_net_ip6_stats,
	.f_json_print	= json_print_net_ip6_stats,
	.hdr_line	= "irec6/s;fwddgm6/s;idel6/s;orq6/s;asmrq6/s;asmok6/s;"
			  "imcpck6/s;omcpck6/s;fragok6/s;fragcr6/s",
	.name		= "A_NET_IP6",
#endif
	.nr		= 1,
	.nr2		= 1,
	.fsize		= STATS_NET_IP6_SIZE,
	.msize		= STATS_NET_IP6_SIZE,
	.opt_flags	= 0,
	.buf		= {NULL, NULL, NULL},
	.bitmap		= NULL
};

/* IPv6 network traffic (errors) activity */
struct activity net_eip6_act = {
	.id		= A_NET_EIP6,
	.options	= AO_NULL,
	.magic		= ACTIVITY_MAGIC_BASE,
	.group		= G_IPV6,
#ifdef SOURCE_SADC
	.f_count	= NULL,
	.f_count2	= NULL,
	.f_read		= wrap_read_net_eip6,
#endif
#ifdef SOURCE_SAR
	.f_print	= print_net_eip6_stats,
	.f_print_avg	= print_net_eip6_stats,
#endif
#ifdef SOURCE_SADF
	.f_render	= render_net_eip6_stats,
	.f_xml_print	= xml_print_net_eip6_stats,
	.f_json_print	= json_print_net_eip6_stats,
	.hdr_line	= "ihdrer6/s;iadrer6/s;iukwnp6/s;i2big6/s;idisc6/s;odisc6/s;"
			  "inort6/s;onort6/s;asmf6/s;fragf6/s;itrpck6/s",
	.name		= "A_NET_EIP6",
#endif
	.nr		= 1,
	.nr2		= 1,
	.fsize		= STATS_NET_EIP6_SIZE,
	.msize		= STATS_NET_EIP6_SIZE,
	.opt_flags	= 0,
	.buf		= {NULL, NULL, NULL},
	.bitmap		= NULL
};

/* ICMPv6 network traffic activity */
struct activity net_icmp6_act = {
	.id		= A_NET_ICMP6,
	.options	= AO_NULL,
	.magic		= ACTIVITY_MAGIC_BASE,
	.group		= G_IPV6,
#ifdef SOURCE_SADC
	.f_count	= NULL,
	.f_count2	= NULL,
	.f_read		= wrap_read_net_icmp6,
#endif
#ifdef SOURCE_SAR
	.f_print	= print_net_icmp6_stats,
	.f_print_avg	= print_net_icmp6_stats,
#endif
#ifdef SOURCE_SADF
	.f_render	= render_net_icmp6_stats,
	.f_xml_print	= xml_print_net_icmp6_stats,
	.f_json_print	= json_print_net_icmp6_stats,
	.hdr_line	= "imsg6/s;omsg6/s;iech6/s;iechr6/s;oechr6/s;igmbq6/s;igmbr6/s;ogmbr6/s;"
			  "igmbrd6/s;ogmbrd6/s;irtsol6/s;ortsol6/s;irtad6/s;inbsol6/s;onbsol6/s;"
			  "inbad6/s;onbad6/s",
	.name		= "A_NET_ICMP6",
#endif
	.nr		= 1,
	.nr2		= 1,
	.fsize		= STATS_NET_ICMP6_SIZE,
	.msize		= STATS_NET_ICMP6_SIZE,
	.opt_flags	= 0,
	.buf		= {NULL, NULL, NULL},
	.bitmap		= NULL
};

/* ICMPv6 network traffic (errors) activity */
struct activity net_eicmp6_act = {
	.id		= A_NET_EICMP6,
	.options	= AO_NULL,
	.magic		= ACTIVITY_MAGIC_BASE,
	.group		= G_IPV6,
#ifdef SOURCE_SADC
	.f_count	= NULL,
	.f_count2	= NULL,
	.f_read		= wrap_read_net_eicmp6,
#endif
#ifdef SOURCE_SAR
	.f_print	= print_net_eicmp6_stats,
	.f_print_avg	= print_net_eicmp6_stats,
#endif
#ifdef SOURCE_SADF
	.f_render	= render_net_eicmp6_stats,
	.f_xml_print	= xml_print_net_eicmp6_stats,
	.f_json_print	= json_print_net_eicmp6_stats,
	.hdr_line	= "ierr6/s;idtunr6/s;odtunr6/s;itmex6/s;otmex6/s;"
		          "iprmpb6/s;oprmpb6/s;iredir6/s;oredir6/s;ipck2b6/s;opck2b6/s",
	.name		= "A_NET_EICMP6",
#endif
	.nr		= 1,
	.nr2		= 1,
	.fsize		= STATS_NET_EICMP6_SIZE,
	.msize		= STATS_NET_EICMP6_SIZE,
	.opt_flags	= 0,
	.buf		= {NULL, NULL, NULL},
	.bitmap		= NULL
};

/* UDPv6 network traffic activity */
struct activity net_udp6_act = {
	.id		= A_NET_UDP6,
	.options	= AO_CLOSE_MARKUP,
	.magic		= ACTIVITY_MAGIC_BASE,
	.group		= G_IPV6,
#ifdef SOURCE_SADC
	.f_count	= NULL,
	.f_count2	= NULL,
	.f_read		= wrap_read_net_udp6,
#endif
#ifdef SOURCE_SAR
	.f_print	= print_net_udp6_stats,
	.f_print_avg	= print_net_udp6_stats,
#endif
#ifdef SOURCE_SADF
	.f_render	= render_net_udp6_stats,
	.f_xml_print	= xml_print_net_udp6_stats,
	.f_json_print	= json_print_net_udp6_stats,
	.hdr_line	= "idgm6/s;odgm6/s;noport6/s;idgmer6/s",
	.name		= "A_NET_UDP6",
#endif
	.nr		= 1,
	.nr2		= 1,
	.fsize		= STATS_NET_UDP6_SIZE,
	.msize		= STATS_NET_UDP6_SIZE,
	.opt_flags	= 0,
	.buf		= {NULL, NULL, NULL},
	.bitmap		= NULL
};

/* CPU frequency */
struct activity pwr_cpufreq_act = {
	.id		= A_PWR_CPUFREQ,
	.options	= AO_NULL,
	.magic		= ACTIVITY_MAGIC_BASE,
	.group		= G_POWER,
#ifdef SOURCE_SADC
	.f_count	= wrap_get_cpu_nr,
	.f_count2	= NULL,
	.f_read		= wrap_read_cpuinfo,
#endif
#ifdef SOURCE_SAR
	.f_print	= print_pwr_cpufreq_stats,
	.f_print_avg	= print_avg_pwr_cpufreq_stats,
#endif
#ifdef SOURCE_SADF
	.f_render	= render_pwr_cpufreq_stats,
	.f_xml_print	= xml_print_pwr_cpufreq_stats,
	.f_json_print	= json_print_pwr_cpufreq_stats,
	.hdr_line	= "CPU;MHz",
	.name		= "A_PWR_CPUFREQ",
#endif
	.nr		= -1,
	.nr2		= 1,
	.fsize		= STATS_PWR_CPUFREQ_SIZE,
	.msize		= STATS_PWR_CPUFREQ_SIZE,
	.opt_flags	= 0,
	.buf		= {NULL, NULL, NULL},
	.bitmap		= &cpu_bitmap
};

/* Fan */
struct activity pwr_fan_act = {
	.id		= A_PWR_FAN,
	.options	= AO_NULL,
	.magic		= ACTIVITY_MAGIC_BASE,
	.group		= G_POWER,
#ifdef SOURCE_SADC
	.f_count	= wrap_get_fan_nr,
	.f_count2	= NULL,
	.f_read		= wrap_read_fan,
#endif
#ifdef SOURCE_SAR
	.f_print	= print_pwr_fan_stats,
	.f_print_avg	= print_avg_pwr_fan_stats,
#endif
#ifdef SOURCE_SADF
	.f_render	= render_pwr_fan_stats,
	.f_xml_print	= xml_print_pwr_fan_stats,
	.f_json_print	= json_print_pwr_fan_stats,
	.hdr_line	= "FAN;DEVICE;rpm;drpm",
	.name		= "A_PWR_FAN",
#endif
	.nr		= -1,
	.nr2		= 1,
	.fsize		= STATS_PWR_FAN_SIZE,
	.msize		= STATS_PWR_FAN_SIZE,
	.opt_flags	= 0,
	.buf		= {NULL, NULL, NULL},
	.bitmap		= NULL
};

/* Temperature */
struct activity pwr_temp_act = {
	.id		= A_PWR_TEMP,
	.options	= AO_NULL,
	.magic		= ACTIVITY_MAGIC_BASE,
	.group		= G_POWER,
#ifdef SOURCE_SADC
	.f_count	= wrap_get_temp_nr,
	.f_count2	= NULL,
	.f_read		= wrap_read_temp,
#endif
#ifdef SOURCE_SAR
	.f_print	= print_pwr_temp_stats,
	.f_print_avg	= print_avg_pwr_temp_stats,
#endif
#ifdef SOURCE_SADF
	.f_render	= render_pwr_temp_stats,
	.f_xml_print	= xml_print_pwr_temp_stats,
	.f_json_print	= json_print_pwr_temp_stats,
	.hdr_line	= "TEMP;DEVICE;degC;%temp",
	.name		= "A_PWR_TEMP",
#endif
	.nr		= -1,
	.nr2		= 1,
	.fsize		= STATS_PWR_TEMP_SIZE,
	.msize		= STATS_PWR_TEMP_SIZE,
	.opt_flags	= 0,
	.buf		= {NULL, NULL, NULL},
	.bitmap		= NULL
};

/* Voltage inputs */
struct activity pwr_in_act = {
	.id		= A_PWR_IN,
	.options	= AO_NULL,
	.magic		= ACTIVITY_MAGIC_BASE,
	.group		= G_POWER,
#ifdef SOURCE_SADC
	.f_count	= wrap_get_in_nr,
	.f_count2	= NULL,
	.f_read		= wrap_read_in,
#endif
#ifdef SOURCE_SAR
	.f_print	= print_pwr_in_stats,
	.f_print_avg	= print_avg_pwr_in_stats,
#endif
#ifdef SOURCE_SADF
	.f_render	= render_pwr_in_stats,
	.f_xml_print	= xml_print_pwr_in_stats,
	.f_json_print	= json_print_pwr_in_stats,
	.hdr_line	= "IN;DEVICE;inV;%in",
	.name		= "A_PWR_IN",
#endif
	.nr		= -1,
	.nr2		= 1,
	.fsize		= STATS_PWR_IN_SIZE,
	.msize		= STATS_PWR_IN_SIZE,
	.opt_flags	= 0,
	.buf		= {NULL, NULL, NULL},
	.bitmap		= NULL
};

/* Hugepages activity */
struct activity huge_act = {
	.id		= A_HUGE,
	.options	= AO_COLLECTED,
	.magic		= ACTIVITY_MAGIC_BASE,
	.group		= G_DEFAULT,
#ifdef SOURCE_SADC
	.f_count	= NULL,
	.f_count2	= NULL,
	.f_read		= wrap_read_meminfo_huge,
#endif
#ifdef SOURCE_SAR
	.f_print	= print_huge_stats,
	.f_print_avg	= print_avg_huge_stats,
#endif
#ifdef SOURCE_SADF
	.f_render	= render_huge_stats,
	.f_xml_print	= xml_print_huge_stats,
	.f_json_print	= json_print_huge_stats,
	.hdr_line	= "kbhugfree;kbhugused;%hugused",
	.name		= "A_HUGE",
#endif
	.nr		= 1,
	.nr2		= 1,
	.fsize		= STATS_HUGE_SIZE,
	.msize		= STATS_HUGE_SIZE,
	.opt_flags	= 0,
	.buf		= {NULL, NULL, NULL},
	.bitmap		= NULL
};

/* CPU weighted frequency */
struct activity pwr_wghfreq_act = {
	.id		= A_PWR_WGHFREQ,
	.options	= AO_NULL,
	.magic		= ACTIVITY_MAGIC_BASE,
	.group		= G_POWER,
#ifdef SOURCE_SADC
	.f_count	= wrap_get_cpu_nr,
	.f_count2	= wrap_get_freq_nr,
	.f_read		= wrap_read_time_in_state,
#endif
#ifdef SOURCE_SAR
	.f_print	= print_pwr_wghfreq_stats,
	.f_print_avg	= print_pwr_wghfreq_stats,
#endif
#ifdef SOURCE_SADF
	.f_render	= render_pwr_wghfreq_stats,
	.f_xml_print	= xml_print_pwr_wghfreq_stats,
	.f_json_print	= json_print_pwr_wghfreq_stats,
	.hdr_line	= "CPU;wghMHz",
	.name		= "A_PWR_WGHFREQ",
#endif
	.nr		= -1,
	.nr2		= 1,
	.fsize		= STATS_PWR_WGHFREQ_SIZE,
	.msize		= STATS_PWR_WGHFREQ_SIZE,
	.opt_flags	= 0,
	.buf		= {NULL, NULL, NULL},
	.bitmap		= &cpu_bitmap
};

/* USB devices plugged into the system */
struct activity pwr_usb_act = {
	.id		= A_PWR_USB,
	.options	= AO_CLOSE_MARKUP,
	.magic		= ACTIVITY_MAGIC_BASE,
	.group		= G_POWER,
#ifdef SOURCE_SADC
	.f_count	= wrap_get_usb_nr,
	.f_count2	= NULL,
	.f_read		= wrap_read_bus_usb_dev,
#endif
#ifdef SOURCE_SAR
	.f_print	= print_pwr_usb_stats,
	.f_print_avg	= print_avg_pwr_usb_stats,
#endif
#ifdef SOURCE_SADF
	.f_render	= render_pwr_usb_stats,
	.f_xml_print	= xml_print_pwr_usb_stats,
	.f_json_print	= json_print_pwr_usb_stats,
	.hdr_line	= "manufact;product;BUS;idvendor;idprod;maxpower",
	.name		= "A_PWR_USB",
#endif
	.nr		= -1,
	.nr2		= 1,
	.fsize		= STATS_PWR_USB_SIZE,
	.msize		= STATS_PWR_USB_SIZE,
	.opt_flags	= 0,
	.buf		= {NULL, NULL, NULL},
	.bitmap		= NULL
};


/*
 * Array of activities.
 * (Order of activities doesn't matter for daily data files).
 */
struct activity *act[NR_ACT] = {
	&cpu_act,
	&pcsw_act,
	&irq_act,
	&swap_act,
	&paging_act,
	&io_act,
	&memory_act,
	&huge_act,
	&ktables_act,
	&queue_act,
	&serial_act,
	&disk_act,
	/* <network> */
	&net_dev_act,
	&net_edev_act,
	&net_nfs_act,
	&net_nfsd_act,
	&net_sock_act,
	&net_ip_act,
	&net_eip_act,
	&net_icmp_act,
	&net_eicmp_act,
	&net_tcp_act,
	&net_etcp_act,
	&net_udp_act,
	&net_sock6_act,
	&net_ip6_act,
	&net_eip6_act,
	&net_icmp6_act,
	&net_eicmp6_act,
	&net_udp6_act,		/* AO_CLOSE_MARKUP */
	/* </network> */
	/* <power-management> */
	&pwr_cpufreq_act,
	&pwr_fan_act,
	&pwr_temp_act,
	&pwr_in_act,
	&pwr_wghfreq_act,
	&pwr_usb_act		/* AO_CLOSE_MARKUP */
	/* </power-management> */
};
