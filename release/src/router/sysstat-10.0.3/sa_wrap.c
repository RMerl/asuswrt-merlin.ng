/*
 * sysstat - sa_wrap.c: Functions used in activity.c
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
#include "rd_stats.h"
#include "rd_sensors.h"

extern unsigned int flags;
extern struct record_header record_hdr;

/*
 ***************************************************************************
 * Read CPU statistics.
 *
 * IN:
 * @a	Activity structure.
 *
 * OUT:
 * @a	Activity structure with statistics.
 ***************************************************************************
 */
__read_funct_t wrap_read_stat_cpu(struct activity *a)
{
	struct stats_cpu *st_cpu
		= (struct stats_cpu *) a->_buf0;
	
	/* Read CPU statistics */
	read_stat_cpu(st_cpu, a->nr, &record_hdr.uptime, &record_hdr.uptime0);
	
	return;
}

/*
 ***************************************************************************
 * Read process (task) creation and context switch statistics.
 *
 * IN:
 * @a	Activity structure.
 *
 * OUT:
 * @a	Activity structure with statistics.
 ***************************************************************************
 */
__read_funct_t wrap_read_stat_pcsw(struct activity *a)
{
	struct stats_pcsw *st_pcsw
		= (struct stats_pcsw *) a->_buf0;

	/* Read process and context switch stats */
	read_stat_pcsw(st_pcsw);
	
	return;
}

/*
 ***************************************************************************
 * Read interrupt statistics.
 *
 * IN:
 * @a	Activity structure.
 *
 * OUT:
 * @a	Activity structure with statistics.
 ***************************************************************************
 */
__read_funct_t wrap_read_stat_irq(struct activity *a)
{
	struct stats_irq *st_irq
		= (struct stats_irq *) a->_buf0;

	/* Read interrupts stats */
	read_stat_irq(st_irq, a->nr);
	
	return;
}

/*
 ***************************************************************************
 * Read queue and load statistics.
 *
 * IN:
 * @a	Activity structure.
 *
 * OUT:
 * @a	Activity structure with statistics.
 ***************************************************************************
 */
__read_funct_t wrap_read_loadavg(struct activity *a)
{
	struct stats_queue *st_queue
		= (struct stats_queue *) a->_buf0;

	/* Read queue and load stats */
	read_loadavg(st_queue);
	
	return;
}

/*
 ***************************************************************************
 * Read memory statistics.
 *
 * IN:
 * @a	Activity structure.
 *
 * OUT:
 * @a	Activity structure with statistics.
 ***************************************************************************
 */
__read_funct_t wrap_read_meminfo(struct activity *a)
{
	struct stats_memory *st_memory
		= (struct stats_memory *) a->_buf0;

	/* Read memory stats */
	read_meminfo(st_memory);
	
	return;
}

/*
 ***************************************************************************
 * Read swapping statistics.
 *
 * IN:
 * @a	Activity structure.
 *
 * OUT:
 * @a	Activity structure with statistics.
 ***************************************************************************
 */
__read_funct_t wrap_read_swap(struct activity *a)
{
	struct stats_swap *st_swap
		= (struct stats_swap *) a->_buf0;

	/* Read stats from /proc/vmstat */
	read_vmstat_swap(st_swap);
	
	return;
}

/*
 ***************************************************************************
 * Read paging statistics.
 *
 * IN:
 * @a	Activity structure.
 *
 * OUT:
 * @a	Activity structure with statistics.
 ***************************************************************************
 */
__read_funct_t wrap_read_paging(struct activity *a)
{
	struct stats_paging *st_paging
		= (struct stats_paging *) a->_buf0;

	/* Read stats from /proc/vmstat */
	read_vmstat_paging(st_paging);
	
	return;
}

/*
 ***************************************************************************
 * Read I/O and transfer rates statistics.
 *
 * IN:
 * @a	Activity structure.
 *
 * OUT:
 * @a	Activity structure with statistics.
 ***************************************************************************
 */
__read_funct_t wrap_read_io(struct activity *a)
{
	struct stats_io *st_io
		= (struct stats_io *) a->_buf0;

	/* Read stats from /proc/diskstats */
	read_diskstats_io(st_io);

	return;
}

/*
 ***************************************************************************
 * Read block devices statistics.
 *
 * IN:
 * @a	Activity structure.
 *
 * OUT:
 * @a	Activity structure with statistics.
 ***************************************************************************
 */
__read_funct_t wrap_read_disk(struct activity *a)
{
	struct stats_disk *st_disk
		= (struct stats_disk *) a->_buf0;

	/* Read stats from /proc/diskstats */
	read_diskstats_disk(st_disk, a->nr, COLLECT_PARTITIONS(a->opt_flags));

	return;
}

/*
 ***************************************************************************
 * Read serial lines statistics.
 *
 * IN:
 * @a	Activity structure.
 *
 * OUT:
 * @a	Activity structure with statistics.
 ***************************************************************************
 */
__read_funct_t wrap_read_tty_driver_serial(struct activity *a)
{
	struct stats_serial *st_serial
		= (struct stats_serial *) a->_buf0;

	/* Read serial lines stats */
	read_tty_driver_serial(st_serial, a->nr);
	
	return;
}

/*
 ***************************************************************************
 * Read kernel tables statistics.
 *
 * IN:
 * @a	Activity structure.
 *
 * OUT:
 * @a	Activity structure with statistics.
 ***************************************************************************
 */
__read_funct_t wrap_read_kernel_tables(struct activity *a)
{
	struct stats_ktables *st_ktables
		= (struct stats_ktables *) a->_buf0;

	/* Read kernel tables stats */
	read_kernel_tables(st_ktables);
	
	return;
}

/*
 ***************************************************************************
 * Read network interfaces statistics.
 *
 * IN:
 * @a	Activity structure.
 *
 * OUT:
 * @a	Activity structure with statistics.
 ***************************************************************************
 */
__read_funct_t wrap_read_net_dev(struct activity *a)
{
	struct stats_net_dev *st_net_dev
		= (struct stats_net_dev *) a->_buf0;

	/* Read network interfaces stats */
	read_net_dev(st_net_dev, a->nr);
	
	return;
}

/*
 ***************************************************************************
 * Read network interfaces errors statistics.
 *
 * IN:
 * @a	Activity structure.
 *
 * OUT:
 * @a	Activity structure with statistics.
 ***************************************************************************
 */
__read_funct_t wrap_read_net_edev(struct activity *a)
{
	struct stats_net_edev *st_net_edev
		= (struct stats_net_edev *) a->_buf0;

	/* Read network interfaces errors stats */
	read_net_edev(st_net_edev, a->nr);
	
	return;
}

/*
 ***************************************************************************
 * Read NFS client statistics.
 *
 * IN:
 * @a	Activity structure.
 *
 * OUT:
 * @a	Activity structure with statistics.
 ***************************************************************************
 */
__read_funct_t wrap_read_net_nfs(struct activity *a)
{
	struct stats_net_nfs *st_net_nfs
		= (struct stats_net_nfs *) a->_buf0;

	/* Read NFS client stats */
	read_net_nfs(st_net_nfs);
	
	return;
}

/*
 ***************************************************************************
 * Read NFS server statistics.
 *
 * IN:
 * @a	Activity structure.
 *
 * OUT:
 * @a	Activity structure with statistics.
 ***************************************************************************
 */
__read_funct_t wrap_read_net_nfsd(struct activity *a)
{
	struct stats_net_nfsd *st_net_nfsd
		= (struct stats_net_nfsd *) a->_buf0;

	/* Read NFS server stats */
	read_net_nfsd(st_net_nfsd);
	
	return;
}

/*
 ***************************************************************************
 * Read network sockets statistics.
 *
 * IN:
 * @a	Activity structure.
 *
 * OUT:
 * @a	Activity structure with statistics.
 ***************************************************************************
 */
__read_funct_t wrap_read_net_sock(struct activity *a)
{
	struct stats_net_sock *st_net_sock
		= (struct stats_net_sock *) a->_buf0;

	/* Read network sockets stats */
	read_net_sock(st_net_sock);
	
	return;
}

/*
 ***************************************************************************
 * Read IP statistics.
 *
 * IN:
 * @a	Activity structure.
 *
 * OUT:
 * @a	Activity structure with statistics.
 ***************************************************************************
 */
__read_funct_t wrap_read_net_ip(struct activity *a)
{
	struct stats_net_ip *st_net_ip
		= (struct stats_net_ip *) a->_buf0;

	/* Read IP stats */
	read_net_ip(st_net_ip);
	
	return;
}

/*
 ***************************************************************************
 * Read IP error statistics.
 *
 * IN:
 * @a	Activity structure.
 *
 * OUT:
 * @a	Activity structure with statistics.
 ***************************************************************************
 */
__read_funct_t wrap_read_net_eip(struct activity *a)
{
	struct stats_net_eip *st_net_eip
		= (struct stats_net_eip *) a->_buf0;

	/* Read IP error stats */
	read_net_eip(st_net_eip);
	
	return;
}

/*
 ***************************************************************************
 * Read ICMP statistics.
 *
 * IN:
 * @a	Activity structure.
 *
 * OUT:
 * @a	Activity structure with statistics.
 ***************************************************************************
 */
__read_funct_t wrap_read_net_icmp(struct activity *a)
{
	struct stats_net_icmp *st_net_icmp
		= (struct stats_net_icmp *) a->_buf0;

	/* Read ICMP stats */
	read_net_icmp(st_net_icmp);
	
	return;
}

/*
 ***************************************************************************
 * Read ICMP error statistics.
 *
 * IN:
 * @a	Activity structure.
 *
 * OUT:
 * @a	Activity structure with statistics.
 ***************************************************************************
 */
__read_funct_t wrap_read_net_eicmp(struct activity *a)
{
	struct stats_net_eicmp *st_net_eicmp
		= (struct stats_net_eicmp *) a->_buf0;

	/* Read ICMP error stats */
	read_net_eicmp(st_net_eicmp);
	
	return;
}

/*
 ***************************************************************************
 * Read TCP statistics.
 *
 * IN:
 * @a	Activity structure.
 *
 * OUT:
 * @a	Activity structure with statistics.
 ***************************************************************************
 */
__read_funct_t wrap_read_net_tcp(struct activity *a)
{
	struct stats_net_tcp *st_net_tcp
		= (struct stats_net_tcp *) a->_buf0;

	/* Read TCP stats */
	read_net_tcp(st_net_tcp);
	
	return;
}

/*
 ***************************************************************************
 * Read TCP error statistics.
 *
 * IN:
 * @a	Activity structure.
 *
 * OUT:
 * @a	Activity structure with statistics.
 ***************************************************************************
 */
__read_funct_t wrap_read_net_etcp(struct activity *a)
{
	struct stats_net_etcp *st_net_etcp
		= (struct stats_net_etcp *) a->_buf0;

	/* Read TCP error stats */
	read_net_etcp(st_net_etcp);
	
	return;
}

/*
 ***************************************************************************
 * Read UDP statistics.
 *
 * IN:
 * @a	Activity structure.
 *
 * OUT:
 * @a	Activity structure with statistics.
 ***************************************************************************
 */
__read_funct_t wrap_read_net_udp(struct activity *a)
{
	struct stats_net_udp *st_net_udp
		= (struct stats_net_udp *) a->_buf0;

	/* Read UDP stats */
	read_net_udp(st_net_udp);
	
	return;
}

/*
 ***************************************************************************
 * Read IPv6 network sockets statistics.
 *
 * IN:
 * @a	Activity structure.
 *
 * OUT:
 * @a	Activity structure with statistics.
 ***************************************************************************
 */
__read_funct_t wrap_read_net_sock6(struct activity *a)
{
	struct stats_net_sock6 *st_net_sock6
		= (struct stats_net_sock6 *) a->_buf0;

	/* Read IPv6 network sockets stats */
	read_net_sock6(st_net_sock6);
	
	return;
}

/*
 ***************************************************************************
 * Read IPv6 statistics.
 *
 * IN:
 * @a	Activity structure.
 *
 * OUT:
 * @a	Activity structure with statistics.
 ***************************************************************************
 */
__read_funct_t wrap_read_net_ip6(struct activity *a)
{
	struct stats_net_ip6 *st_net_ip6
		= (struct stats_net_ip6 *) a->_buf0;

	/* Read IPv6 stats */
	read_net_ip6(st_net_ip6);
	
	return;
}

/*
 ***************************************************************************
 * Read IPv6 error statistics.
 *
 * IN:
 * @a	Activity structure.
 *
 * OUT:
 * @a	Activity structure with statistics.
 ***************************************************************************
 */
__read_funct_t wrap_read_net_eip6(struct activity *a)
{
	struct stats_net_eip6 *st_net_eip6
		= (struct stats_net_eip6 *) a->_buf0;

	/* Read IPv6 error stats */
	read_net_eip6(st_net_eip6);
	
	return;
}

/*
 ***************************************************************************
 * Read ICMPv6 statistics.
 *
 * IN:
 * @a	Activity structure.
 *
 * OUT:
 * @a	Activity structure with statistics.
 ***************************************************************************
 */
__read_funct_t wrap_read_net_icmp6(struct activity *a)
{
	struct stats_net_icmp6 *st_net_icmp6
		= (struct stats_net_icmp6 *) a->_buf0;

	/* Read ICMPv6 stats */
	read_net_icmp6(st_net_icmp6);
	
	return;
}

/*
 ***************************************************************************
 * Read ICMPv6 error statistics.
 *
 * IN:
 * @a	Activity structure.
 *
 * OUT:
 * @a	Activity structure with statistics.
 ***************************************************************************
 */
__read_funct_t wrap_read_net_eicmp6(struct activity *a)
{
	struct stats_net_eicmp6 *st_net_eicmp6
		= (struct stats_net_eicmp6 *) a->_buf0;

	/* Read ICMPv6 error stats */
	read_net_eicmp6(st_net_eicmp6);
	
	return;
}

/*
 ***************************************************************************
 * Read UDPv6 statistics.
 *
 * IN:
 * @a	Activity structure.
 *
 * OUT:
 * @a	Activity structure with statistics.
 ***************************************************************************
 */
__read_funct_t wrap_read_net_udp6(struct activity *a)
{
	struct stats_net_udp6 *st_net_udp6
		= (struct stats_net_udp6 *) a->_buf0;

	/* Read UDPv6 stats */
	read_net_udp6(st_net_udp6);
	
	return;
}

/*
 ***************************************************************************
 * Read CPU frequency statistics.
 *
 * IN:
 * @a	Activity structure.
 *
 * OUT:
 * @a	Activity structure with statistics.
 ***************************************************************************
 */
__read_funct_t wrap_read_cpuinfo(struct activity *a)
{
	struct stats_pwr_cpufreq *st_pwr_cpufreq
		= (struct stats_pwr_cpufreq *) a->_buf0;

	/* Read CPU frequency stats */
	read_cpuinfo(st_pwr_cpufreq, a->nr);
	
	return;
}

/*
 ***************************************************************************
 * Read fan statistics.
 *
 * IN:
 * @a  Activity structure.
 *
 * OUT:
 * @a  Activity structure with statistics.
 ***************************************************************************
 */
__read_funct_t wrap_read_fan(struct activity *a)
{
	struct stats_pwr_fan *st_pwr_fan
		= (struct stats_pwr_fan *) a->_buf0;

	/* Read fan stats */
	read_fan(st_pwr_fan, a->nr);

	return;
}

/*
 ***************************************************************************
 * Read temperature statistics.
 *
 * IN:
 * @a  Activity structure.
 *
 * OUT:
 * @a  Activity structure with statistics.
 ***************************************************************************
 */
__read_funct_t wrap_read_temp(struct activity *a)
{
	struct stats_pwr_temp *st_pwr_temp
		= (struct stats_pwr_temp *) a->_buf0;

	/* Read temperature stats */
	read_temp(st_pwr_temp, a->nr);

	return;
}

/*
 ***************************************************************************
 * Read voltage input statistics.
 *
 * IN:
 * @a  Activity structure.
 *
 * OUT:
 * @a  Activity structure with statistics.
 ***************************************************************************
 */
__read_funct_t wrap_read_in(struct activity *a)
{
	struct stats_pwr_in *st_pwr_in
		= (struct stats_pwr_in *) a->_buf0;

	/* Read voltage input stats */
	read_in(st_pwr_in, a->nr);

	return;
}

/*
 ***************************************************************************
 * Read hugepages statistics.
 *
 * IN:
 * @a	Activity structure.
 *
 * OUT:
 * @a	Activity structure with statistics.
 ***************************************************************************
 */
__read_funct_t wrap_read_meminfo_huge(struct activity *a)
{
	struct stats_huge *st_huge
		= (struct stats_huge *) a->_buf0;

	/* Read hugepages stats */
	read_meminfo_huge(st_huge);

	return;
}

/*
 ***************************************************************************
 * Read weighted CPU frequency statistics.
 *
 * IN:
 * @a	Activity structure.
 *
 * OUT:
 * @a	Activity structure with statistics.
 ***************************************************************************
 */
__read_funct_t wrap_read_time_in_state(struct activity *a)
{
	__nr_t	cpu = 0;
	int j;
	struct stats_pwr_wghfreq *st_pwr_wghfreq
		= (struct stats_pwr_wghfreq *) a->_buf0;
	struct stats_pwr_wghfreq *st_pwr_wghfreq_i, *st_pwr_wghfreq_j, *st_pwr_wghfreq_all_j;

	while (cpu < (a->nr - 1)) {
		/* Read current CPU time-in-state data */
		st_pwr_wghfreq_i = st_pwr_wghfreq + (cpu + 1) * a->nr2;
		read_time_in_state(st_pwr_wghfreq_i, cpu, a->nr2);

		/* Also save data for CPU 'all' */
		for (j = 0; j < a->nr2; j++) {
			st_pwr_wghfreq_j     = st_pwr_wghfreq_i + j;	/* CPU #cpu, state #j */
			st_pwr_wghfreq_all_j = st_pwr_wghfreq   + j;	/* CPU #all, state #j */
			if (!cpu) {
				/* Assume that possible frequencies are the same for all CPUs */
				st_pwr_wghfreq_all_j->freq = st_pwr_wghfreq_j->freq;
			}
			st_pwr_wghfreq_all_j->time_in_state += st_pwr_wghfreq_j->time_in_state;
		}
		cpu++;
	}

	/* Special processing for non SMP kernels: Only CPU 'all' is available */
	if (a->nr == 1) {
		read_time_in_state(st_pwr_wghfreq, 0, a->nr2);
	}
	else {
		for (j = 0; j < a->nr2; j++) {
			st_pwr_wghfreq_all_j = st_pwr_wghfreq + j;	/* CPU #all, state #j */
			st_pwr_wghfreq_all_j->time_in_state /= (a->nr - 1);
		}
	}

	return;
}

/*
 ***************************************************************************
 * Read USB devices statistics.
 *
 * IN:
 * @a  Activity structure.
 *
 * OUT:
 * @a  Activity structure with statistics.
 ***************************************************************************
 */
__read_funct_t wrap_read_bus_usb_dev(struct activity *a)
{
	struct stats_pwr_usb *st_pwr_usb
		= (struct stats_pwr_usb *) a->_buf0;

	/* Read USB devices stats */
	read_bus_usb_dev(st_pwr_usb, a->nr);

	return;
}

/*
 ***************************************************************************
 * Count number of interrupts that are in /proc/stat file.
 * Truncate the number of different individual interrupts to NR_IRQS.
 *
 * IN:
 * @a	Activity structure.
 *
 * RETURNS:
 * Number of interrupts, including total number of interrupts.
 * Value in [0, NR_IRQS + 1].
 ***************************************************************************
 */
__nr_t wrap_get_irq_nr(struct activity *a)
{
	__nr_t n;

	if ((n = get_irq_nr()) > (a->bitmap->b_size + 1)) {
		n = a->bitmap->b_size + 1;
	}

	return n;
}

/*
 ***************************************************************************
 * Find number of serial lines that support tx/rx accounting
 * in /proc/tty/driver/serial file.
 *
 * IN:
 * @a	Activity structure.
 *
 * RETURNS:
 * Number of serial lines supporting tx/rx accouting + a pre-allocation
 * constant.
 ***************************************************************************
 */
__nr_t wrap_get_serial_nr(struct activity *a)
{
	__nr_t n = 0;

	if ((n = get_serial_nr()) > 0)
		return n + NR_SERIAL_PREALLOC;

	return 0;
}

/*
 ***************************************************************************
 * Find number of interfaces (network devices) that are in /proc/net/dev
 * file.
 *
 * IN:
 * @a	Activity structure.
 *
 * RETURNS:
 * Number of network interfaces + a pre-allocation constant.
 ***************************************************************************
 */
__nr_t wrap_get_iface_nr(struct activity *a)
{
	__nr_t n = 0;

	if ((n = get_iface_nr()) > 0)
		return n + NR_IFACE_PREALLOC;

	return 0;
}

/*
 ***************************************************************************
 * Compute number of CPU structures to allocate.
 *
 * IN:
 * @a	Activity structure.
 *
 * RETURNS:
 * Number of structures (value in [1, NR_CPUS + 1]).
 * 1 means that there is only one proc and non SMP kernel.
 * 2 means one proc and SMP kernel.
 * Etc.
 ***************************************************************************
 */
__nr_t wrap_get_cpu_nr(struct activity *a)
{
	return (get_cpu_nr(a->bitmap->b_size) + 1);
}

/*
 ***************************************************************************
 * Get number of devices in /proc/diskstats.
 * Always done, since disk stats must be read at least for sar -b
 * if not for sar -d.
 *
 * IN:
 * @a	Activity structure.
 *
 * RETURNS:
 * Number of devices + a pre-allocation constant.
 ***************************************************************************
 */
__nr_t wrap_get_disk_nr(struct activity *a)
{
	__nr_t n = 0;
	unsigned int f = COLLECT_PARTITIONS(a->opt_flags);

	if ((n = get_disk_nr(f)) > 0)
		return n + NR_DISK_PREALLOC;

	return 0;
}

/*
 ***************************************************************************
 * Get number of fan structures to allocate.
 *
 * IN:
 * @a  Activity structure.
 *
 * RETURNS:
 * Number of structures.
 ***************************************************************************
 */
__nr_t wrap_get_fan_nr(struct activity *a)
{
	return (get_fan_nr());
}

/*
 ***************************************************************************
 * Get number of temp structures to allocate.
 *
 * IN:
 * @a  Activity structure.
 *
 * RETURNS:
 * Number of structures.
 ***************************************************************************
 */
__nr_t wrap_get_temp_nr(struct activity *a)
{
	return (get_temp_nr());
}

/*
 ***************************************************************************
 * Get number of voltage input structures to allocate.
 *
 * IN:
 * @a  Activity structure.
 *
 * RETURNS:
 * Number of structures.
 ***************************************************************************
 */
__nr_t wrap_get_in_nr(struct activity *a)
{
	return (get_in_nr());
}

/*
 ***************************************************************************
 * Count number of possible frequencies for CPU#0.
 *
 * IN:
 * @a   Activity structure.
 *
 * RETURNS:
 * Number of CPU frequencies + a pre-allocation constant.
 ***************************************************************************
 */
__nr_t wrap_get_freq_nr(struct activity *a)
{
	__nr_t n = 0;

	if ((n = get_freq_nr()) > 0)
		return n + NR_FREQ_PREALLOC;

	return 0;
}

/*
 ***************************************************************************
 * Count number of USB devices plugged into the system.
 *
 * IN:
 * @a	Activity structure.
 *
 * RETURNS:
 * Number of USB devices + a pre-allocation constant.
 ***************************************************************************
 */
__nr_t wrap_get_usb_nr(struct activity *a)
{
	__nr_t n = 0;

	if ((n = get_usb_nr()) >= 0)
		/* Return a positive number even if no USB devices have been found */
		return (n + NR_USB_PREALLOC);
	
	return 0;
}
