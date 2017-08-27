/*
 * rd_stats.c: Read system statistics
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <dirent.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "common.h"
#include "rd_stats.h"
#include "ioconf.h"

#ifdef USE_NLS
#include <locale.h>
#include <libintl.h>
#define _(string) gettext(string)
#else
#define _(string) (string)
#endif

/*
 ***************************************************************************
 * Read CPU statistics and machine uptime.
 *
 * IN:
 * @st_cpu	Structure where stats will be saved.
 * @nbr		Total number of CPU (including cpu "all").
 *
 * OUT:
 * @st_cpu	Structure with statistics.
 * @uptime	Machine uptime multiplied by the number of processors.
 * @uptime0	Machine uptime. Filled only if previously set to zero.
 ***************************************************************************
 */
void read_stat_cpu(struct stats_cpu *st_cpu, int nbr,
		   unsigned long long *uptime, unsigned long long *uptime0)
{
	FILE *fp;
	struct stats_cpu *st_cpu_i;
	struct stats_cpu sc;
	char line[8192];
	int proc_nb;

	if ((fp = fopen(STAT, "r")) == NULL) {
		fprintf(stderr, _("Cannot open %s: %s\n"), STAT, strerror(errno));
		exit(2);
	}

	while (fgets(line, 8192, fp) != NULL) {

		if (!strncmp(line, "cpu ", 4)) {

			/*
			 * All the fields don't necessarily exist,
			 * depending on the kernel version used.
			 */
			memset(st_cpu, 0, STATS_CPU_SIZE);

			/*
			 * Read the number of jiffies spent in the different modes
			 * (user, nice, etc.) among all proc. CPU usage is not reduced
			 * to one processor to avoid rounding problems.
			 */
			sscanf(line + 5, "%llu %llu %llu %llu %llu %llu %llu %llu %llu",
			       &st_cpu->cpu_user,
			       &st_cpu->cpu_nice,
			       &st_cpu->cpu_sys,
			       &st_cpu->cpu_idle,
			       &st_cpu->cpu_iowait,
			       &st_cpu->cpu_hardirq,
			       &st_cpu->cpu_softirq,
			       &st_cpu->cpu_steal,
			       &st_cpu->cpu_guest);

			/*
			 * Compute the uptime of the system in jiffies (1/100ths of a second
			 * if HZ=100).
			 * Machine uptime is multiplied by the number of processors here.
			 *
			 * NB: Don't add cpu_guest because cpu_user already includes it.
			 */
			*uptime = st_cpu->cpu_user + st_cpu->cpu_nice    +
				st_cpu->cpu_sys    + st_cpu->cpu_idle    +
				st_cpu->cpu_iowait + st_cpu->cpu_hardirq +
				st_cpu->cpu_steal  + st_cpu->cpu_softirq;
		}

		else if (!strncmp(line, "cpu", 3)) {
			if (nbr > 1) {
				/* All the fields don't necessarily exist */
				memset(&sc, 0, STATS_CPU_SIZE);
				/*
				 * Read the number of jiffies spent in the different modes
				 * (user, nice, etc) for current proc.
				 * This is done only on SMP machines.
				 */
				sscanf(line + 3, "%d %llu %llu %llu %llu %llu %llu %llu %llu %llu",
				       &proc_nb,
				       &sc.cpu_user,
				       &sc.cpu_nice,
				       &sc.cpu_sys,
				       &sc.cpu_idle,
				       &sc.cpu_iowait,
				       &sc.cpu_hardirq,
				       &sc.cpu_softirq,
				       &sc.cpu_steal,
				       &sc.cpu_guest);

				if (proc_nb < (nbr - 1)) {
					st_cpu_i = st_cpu + proc_nb + 1;
					*st_cpu_i = sc;
				}
				/*
				 * else additional CPUs have been dynamically registered
				 * in /proc/stat.
				 */

				if (!proc_nb && !*uptime0) {
					/*
					 * Compute uptime reduced to one proc using proc#0.
					 * Done if /proc/uptime was unavailable.
					 *
					 * NB: Don't add cpu_guest because cpu_user already
					 * includes it.
					 */
					*uptime0 = sc.cpu_user + sc.cpu_nice  +
						sc.cpu_sys     + sc.cpu_idle  +
						sc.cpu_iowait  + sc.cpu_steal +
						sc.cpu_hardirq + sc.cpu_softirq;
				}
			}
		}
	}

	fclose(fp);
}

/*
 ***************************************************************************
 * Read processes (tasks) creation and context switches statistics
 * from /proc/stat.
 *
 * IN:
 * @st_pcsw	Structure where stats will be saved.
 *
 * OUT:
 * @st_pcsw	Structure with statistics.
 ***************************************************************************
 */
void read_stat_pcsw(struct stats_pcsw *st_pcsw)
{
	FILE *fp;
	char line[8192];

	if ((fp = fopen(STAT, "r")) == NULL)
		return;

	while (fgets(line, 8192, fp) != NULL) {

		if (!strncmp(line, "ctxt ", 5)) {
			/* Read number of context switches */
			sscanf(line + 5, "%llu", &st_pcsw->context_switch);
		}

		else if (!strncmp(line, "processes ", 10)) {
			/* Read number of processes created since system boot */
			sscanf(line + 10, "%lu", &st_pcsw->processes);
		}
	}

	fclose(fp);
}

/*
 ***************************************************************************
 * Read interrupts statistics from /proc/stat.
 *
 * IN:
 * @st_irq	Structure where stats will be saved.
 * @nbr		Number of interrupts to read, including the total number
 *		of interrupts.
 *
 * OUT:
 * @st_irq	Structure with statistics.
 ***************************************************************************
 */
void read_stat_irq(struct stats_irq *st_irq, int nbr)
{
	FILE *fp;
	struct stats_irq *st_irq_i;
	char line[8192];
	int i, pos;

	if ((fp = fopen(STAT, "r")) == NULL)
		return;
	
	while (fgets(line, 8192, fp) != NULL) {

		if (!strncmp(line, "intr ", 5)) {
			/* Read total number of interrupts received since system boot */
			sscanf(line + 5, "%llu", &st_irq->irq_nr);
			pos = strcspn(line + 5, " ") + 5;

			for (i = 1; i < nbr; i++) {
				st_irq_i = st_irq + i;
				sscanf(line + pos, " %llu", &st_irq_i->irq_nr);
				pos += strcspn(line + pos + 1, " ") + 1;
			}
		}
	}

	fclose(fp);
}

/*
 ***************************************************************************
 * Read queue and load statistics from /proc/loadavg and /proc/stat.
 *
 * IN:
 * @st_queue	Structure where stats will be saved.
 *
 * OUT:
 * @st_queue	Structure with statistics.
 ***************************************************************************
 */
void read_loadavg(struct stats_queue *st_queue)
{
	FILE *fp;
	char line[8192];
	int load_tmp[3];

	if ((fp = fopen(LOADAVG, "r")) == NULL)
		return;
	
	/* Read load averages and queue length */
	fscanf(fp, "%d.%d %d.%d %d.%d %ld/%d %*d\n",
	       &load_tmp[0], &st_queue->load_avg_1,
	       &load_tmp[1], &st_queue->load_avg_5,
	       &load_tmp[2], &st_queue->load_avg_15,
	       &st_queue->nr_running,
	       &st_queue->nr_threads);

	fclose(fp);

	st_queue->load_avg_1  += load_tmp[0] * 100;
	st_queue->load_avg_5  += load_tmp[1] * 100;
	st_queue->load_avg_15 += load_tmp[2] * 100;

	if (st_queue->nr_running) {
		/* Do not take current process into account */
		st_queue->nr_running--;
	}

	/* Read nr of tasks blocked from /proc/stat */
	if ((fp = fopen(STAT, "r")) == NULL)
		return;

	while (fgets(line, 8192, fp) != NULL) {

		if (!strncmp(line, "procs_blocked ", 14)) {
			/* Read number of processes blocked */
			sscanf(line + 14, "%lu", &st_queue->procs_blocked);
			break;
		}
	}

	fclose(fp);
}

/*
 ***************************************************************************
 * Read memory statistics from /proc/meminfo.
 *
 * IN:
 * @st_memory	Structure where stats will be saved.
 *
 * OUT:
 * @st_memory	Structure with statistics.
 ***************************************************************************
 */
void read_meminfo(struct stats_memory *st_memory)
{
	FILE *fp;
	char line[128];
	
	if ((fp = fopen(MEMINFO, "r")) == NULL)
		return;

	while (fgets(line, 128, fp) != NULL) {

		if (!strncmp(line, "MemTotal:", 9)) {
			/* Read the total amount of memory in kB */
			sscanf(line + 9, "%lu", &st_memory->tlmkb);
		}
		else if (!strncmp(line, "MemFree:", 8)) {
			/* Read the amount of free memory in kB */
			sscanf(line + 8, "%lu", &st_memory->frmkb);
		}
		else if (!strncmp(line, "Buffers:", 8)) {
			/* Read the amount of buffered memory in kB */
			sscanf(line + 8, "%lu", &st_memory->bufkb);
		}
		else if (!strncmp(line, "Cached:", 7)) {
			/* Read the amount of cached memory in kB */
			sscanf(line + 7, "%lu", &st_memory->camkb);
		}
		else if (!strncmp(line, "SwapCached:", 11)) {
			/* Read the amount of cached swap in kB */
			sscanf(line + 11, "%lu", &st_memory->caskb);
		}
		else if (!strncmp(line, "Active:", 7)) {
			/* Read the amount of active memory in kB */
			sscanf(line + 7, "%lu", &st_memory->activekb);
		}
		else if (!strncmp(line, "Inactive:", 9)) {
			/* Read the amount of inactive memory in kB */
			sscanf(line + 9, "%lu", &st_memory->inactkb);
		}
		else if (!strncmp(line, "SwapTotal:", 10)) {
			/* Read the total amount of swap memory in kB */
			sscanf(line + 10, "%lu", &st_memory->tlskb);
		}
		else if (!strncmp(line, "SwapFree:", 9)) {
			/* Read the amount of free swap memory in kB */
			sscanf(line + 9, "%lu", &st_memory->frskb);
		}
		else if (!strncmp(line, "Committed_AS:", 13)) {
			/* Read the amount of commited memory in kB */
			sscanf(line + 13, "%lu", &st_memory->comkb);
		}
	}

	fclose(fp);
}

/*
 ***************************************************************************
 * Read swapping statistics from /proc/vmstat.
 *
 * IN:
 * @st_swap	Structure where stats will be saved.
 *
 * OUT:
 * @st_swap	Structure with statistics.
 ***************************************************************************
 */
void read_vmstat_swap(struct stats_swap *st_swap)
{
	FILE *fp;
	char line[128];

	if ((fp = fopen(VMSTAT, "r")) == NULL)
		return;

	while (fgets(line, 128, fp) != NULL) {

		if (!strncmp(line, "pswpin ", 7)) {
			/* Read number of swap pages brought in */
			sscanf(line + 7, "%lu", &st_swap->pswpin);
		}
		else if (!strncmp(line, "pswpout ", 8)) {
			/* Read number of swap pages brought out */
			sscanf(line + 8, "%lu", &st_swap->pswpout);
		}
	}
	
	fclose(fp);
}

/*
 ***************************************************************************
 * Read paging statistics from /proc/vmstat.
 *
 * IN:
 * @st_paging	Structure where stats will be saved.
 *
 * OUT:
 * @st_paging	Structure with statistics.
 ***************************************************************************
 */
void read_vmstat_paging(struct stats_paging *st_paging)
{
	FILE *fp;
	char line[128];
	unsigned long pgtmp;

	if ((fp = fopen(VMSTAT, "r")) == NULL)
		return;

	st_paging->pgsteal = 0;
	st_paging->pgscan_kswapd = st_paging->pgscan_direct = 0;

	while (fgets(line, 128, fp) != NULL) {

		if (!strncmp(line, "pgpgin ", 7)) {
			/* Read number of pages the system paged in */
			sscanf(line + 7, "%lu", &st_paging->pgpgin);
		}
		else if (!strncmp(line, "pgpgout ", 8)) {
			/* Read number of pages the system paged out */
			sscanf(line + 8, "%lu", &st_paging->pgpgout);
		}
		else if (!strncmp(line, "pgfault ", 8)) {
			/* Read number of faults (major+minor) made by the system */
			sscanf(line + 8, "%lu", &st_paging->pgfault);
		}
		else if (!strncmp(line, "pgmajfault ", 11)) {
			/* Read number of faults (major only) made by the system */
			sscanf(line + 11, "%lu", &st_paging->pgmajfault);
		}
		else if (!strncmp(line, "pgfree ", 7)) {
			/* Read number of pages freed by the system */
			sscanf(line + 7, "%lu", &st_paging->pgfree);
		}
		else if (!strncmp(line, "pgsteal_", 8)) {
			/* Read number of pages stolen by the system */
			sscanf(strchr(line, ' '), "%lu", &pgtmp);
			st_paging->pgsteal += pgtmp;
		}
		else if (!strncmp(line, "pgscan_kswapd_", 14)) {
			/* Read number of pages scanned by the kswapd daemon */
			sscanf(strchr(line, ' '), "%lu", &pgtmp);
			st_paging->pgscan_kswapd += pgtmp;
		}
		else if (!strncmp(line, "pgscan_direct_", 14)) {
			/* Read number of pages scanned directly */
			sscanf(strchr(line, ' '), "%lu", &pgtmp);
			st_paging->pgscan_direct += pgtmp;
		}
	}
	
	fclose(fp);
}

/*
 ***************************************************************************
 * Read I/O and transfer rates statistics from /proc/diskstats.
 *
 * IN:
 * @st_io	Structure where stats will be saved.
 *
 * OUT:
 * @st_io	Structure with statistics.
 ***************************************************************************
 */
void read_diskstats_io(struct stats_io *st_io)
{
	FILE *fp;
	char line[256];
	char dev_name[MAX_NAME_LEN];
	unsigned int major, minor;
	unsigned long rd_ios, wr_ios;
	unsigned long long rd_sec, wr_sec;

	if ((fp = fopen(DISKSTATS, "r")) == NULL)
		return;

	while (fgets(line, 256, fp) != NULL) {

		if (sscanf(line, "%u %u %s %lu %*u %llu %*u %lu %*u %llu",
			   &major, &minor, dev_name,
			   &rd_ios, &rd_sec, &wr_ios, &wr_sec) == 7) {
			
			if (is_device(dev_name)) {
				/*
				 * OK: It's a device and not a partition.
				 * Note: Structure should have been initialized first!
				 */
				st_io->dk_drive      += rd_ios + wr_ios;
				st_io->dk_drive_rio  += rd_ios;
				st_io->dk_drive_rblk += (unsigned int) rd_sec;
				st_io->dk_drive_wio  += wr_ios;
				st_io->dk_drive_wblk += (unsigned int) wr_sec;
			}
		}
	}
	
	fclose(fp);
}

/*
 ***************************************************************************
 * Read block devices statistics from /proc/diskstats.
 *
 * IN:
 * @st_disk	Structure where stats will be saved.
 * @nbr		Maximum number of block devices.
 * @read_part	True if disks *and* partitions should be read; False if only
 * 		disks are read.
 *
 * OUT:
 * @st_disk	Structure with statistics.
 ***************************************************************************
 */
void read_diskstats_disk(struct stats_disk *st_disk, int nbr, int read_part)
{
	FILE *fp;
	char line[256];
	char dev_name[MAX_NAME_LEN];
	int dsk = 0;
	struct stats_disk *st_disk_i;
	unsigned int major, minor;
	unsigned long rd_ios, wr_ios, rd_ticks, wr_ticks;
	unsigned long tot_ticks, rq_ticks;
	unsigned long long rd_sec, wr_sec;

	if ((fp = fopen(DISKSTATS, "r")) == NULL)
		return;

	while ((fgets(line, 256, fp) != NULL) && (dsk < nbr)) {

		if (sscanf(line, "%u %u %s %lu %*u %llu %lu %lu %*u %llu"
			   " %lu %*u %lu %lu",
			   &major, &minor, dev_name,
			   &rd_ios, &rd_sec, &rd_ticks, &wr_ios, &wr_sec, &wr_ticks,
			   &tot_ticks, &rq_ticks) == 11) {
			
			if (!rd_ios && !wr_ios)
				/* Unused device: Ignore it */
				continue;
			if (read_part || is_device(dev_name)) {
				st_disk_i = st_disk + dsk++;
				st_disk_i->major     = major;
				st_disk_i->minor     = minor;
				st_disk_i->nr_ios    = rd_ios + wr_ios;
				st_disk_i->rd_sect   = rd_sec;
				st_disk_i->wr_sect   = wr_sec;
				st_disk_i->rd_ticks  = rd_ticks;
				st_disk_i->wr_ticks  = wr_ticks;
				st_disk_i->tot_ticks = tot_ticks;
				st_disk_i->rq_ticks  = rq_ticks;
			}
		}
	}

	fclose(fp);
}

/*
 ***************************************************************************
 * Read serial lines statistics from /proc/tty/driver/serial.
 *
 * IN:
 * @st_serial	Structure where stats will be saved.
 * @nbr		Maximum number of serial lines.
 *
 * OUT:
 * @st_serial	Structure with statistics.
 ***************************************************************************
 */
void read_tty_driver_serial(struct stats_serial *st_serial, int nbr)
{
	FILE *fp;
	struct stats_serial *st_serial_i;
	int sl = 0;
	char line[256];
	char *p;

	if ((fp = fopen(SERIAL, "r")) == NULL)
		return;

	while ((fgets(line, 256, fp) != NULL) && (sl < nbr)) {

		if ((p = strstr(line, "tx:")) != NULL) {
			st_serial_i = st_serial + sl;
			sscanf(line, "%u", &st_serial_i->line);
			/*
			 * A value of 0 means an unused structure.
			 * So increment it to make sure it is not null.
			 */
			(st_serial_i->line)++;
			/*
			 * Read the number of chars transmitted and received by
			 * current serial line.
			 */
			sscanf(p + 3, "%u", &st_serial_i->tx);
			if ((p = strstr(line, "rx:")) != NULL) {
				sscanf(p + 3, "%u", &st_serial_i->rx);
			}
			if ((p = strstr(line, "fe:")) != NULL) {
				sscanf(p + 3, "%u", &st_serial_i->frame);
			}
			if ((p = strstr(line, "pe:")) != NULL) {
				sscanf(p + 3, "%u", &st_serial_i->parity);
			}
			if ((p = strstr(line, "brk:")) != NULL) {
				sscanf(p + 4, "%u", &st_serial_i->brk);
			}
			if ((p = strstr(line, "oe:")) != NULL) {
				sscanf(p + 3, "%u", &st_serial_i->overrun);
			}
			
			sl++;
		}
	}

	fclose(fp);
}

/*
 ***************************************************************************
 * Read kernel tables statistics from various system files.
 *
 * IN:
 * @st_ktables	Structure where stats will be saved.
 *
 * OUT:
 * @st_ktables	Structure with statistics.
 ***************************************************************************
 */
void read_kernel_tables(struct stats_ktables *st_ktables)
{
	FILE *fp;
	unsigned int parm;
	
	/* Open /proc/sys/fs/dentry-state file */
	if ((fp = fopen(FDENTRY_STATE, "r")) != NULL) {
		fscanf(fp, "%*d %u",
		       &st_ktables->dentry_stat);
		fclose(fp);
	}

	/* Open /proc/sys/fs/file-nr file */
	if ((fp = fopen(FFILE_NR, "r")) != NULL) {
		fscanf(fp, "%u %u",
		       &st_ktables->file_used, &parm);
		fclose(fp);
		/*
		 * The number of used handles is the number of allocated ones
		 * minus the number of free ones.
		 */
		st_ktables->file_used -= parm;
	}

	/* Open /proc/sys/fs/inode-state file */
	if ((fp = fopen(FINODE_STATE, "r")) != NULL) {
		fscanf(fp, "%u %u",
		       &st_ktables->inode_used, &parm);
		fclose(fp);
		/*
		 * The number of inuse inodes is the number of allocated ones
		 * minus the number of free ones.
		 */
		st_ktables->inode_used -= parm;
	}

	/* Open /proc/sys/kernel/pty/nr file */
	if ((fp = fopen(PTY_NR, "r")) != NULL) {
		fscanf(fp, "%u",
		       &st_ktables->pty_nr);
		fclose(fp);
	}
}

/*
 ***************************************************************************
 * Read network interfaces statistics from /proc/net/dev.
 *
 * IN:
 * @st_net_dev	Structure where stats will be saved.
 * @nbr		Maximum number of network interfaces.
 *
 * OUT:
 * @st_net_dev	Structure with statistics.
 ***************************************************************************
 */
void read_net_dev(struct stats_net_dev *st_net_dev, int nbr)
{
	FILE *fp;
	struct stats_net_dev *st_net_dev_i;
	char line[256];
	char iface[MAX_IFACE_LEN];
	int dev = 0;
	int pos;

	if ((fp = fopen(NET_DEV, "r")) == NULL)
		return;
	
	while ((fgets(line, 256, fp) != NULL) && (dev < nbr)) {

		pos = strcspn(line, ":");
		if (pos < strlen(line)) {
			st_net_dev_i = st_net_dev + dev;
			strncpy(iface, line, MINIMUM(pos, MAX_IFACE_LEN - 1));
			iface[MINIMUM(pos, MAX_IFACE_LEN - 1)] = '\0';
			sscanf(iface, "%s", st_net_dev_i->interface); /* Skip heading spaces */
			sscanf(line + pos + 1, "%lu %lu %*u %*u %*u %*u %lu %lu %lu %lu "
			       "%*u %*u %*u %*u %*u %lu",
			       &st_net_dev_i->rx_bytes,
			       &st_net_dev_i->rx_packets,
			       &st_net_dev_i->rx_compressed,
			       &st_net_dev_i->multicast,
			       &st_net_dev_i->tx_bytes,
			       &st_net_dev_i->tx_packets,
			       &st_net_dev_i->tx_compressed);
			dev++;
		}
	}

	fclose(fp);
}

/*
 ***************************************************************************
 * Read network interfaces errors statistics from /proc/net/dev.
 *
 * IN:
 * @st_net_edev	Structure where stats will be saved.
 * @nbr		Maximum number of network interfaces.
 *
 * OUT:
 * @st_net_edev	Structure with statistics.
 ***************************************************************************
 */
void read_net_edev(struct stats_net_edev *st_net_edev, int nbr)
{
	FILE *fp;
	struct stats_net_edev *st_net_edev_i;
	static char line[256];
	char iface[MAX_IFACE_LEN];
	int dev = 0;
	int pos;

	if ((fp = fopen(NET_DEV, "r")) == NULL)
		return;

	while ((fgets(line, 256, fp) != NULL) && (dev < nbr)) {

		pos = strcspn(line, ":");
		if (pos < strlen(line)) {
			st_net_edev_i = st_net_edev + dev;
			strncpy(iface, line, MINIMUM(pos, MAX_IFACE_LEN - 1));
			iface[MINIMUM(pos, MAX_IFACE_LEN - 1)] = '\0';
			sscanf(iface, "%s", st_net_edev_i->interface); /* Skip heading spaces */
			sscanf(line + pos + 1, "%*u %*u %lu %lu %lu %lu %*u %*u %*u %*u "
			       "%lu %lu %lu %lu %lu",
			       &st_net_edev_i->rx_errors,
			       &st_net_edev_i->rx_dropped,
			       &st_net_edev_i->rx_fifo_errors,
			       &st_net_edev_i->rx_frame_errors,
			       &st_net_edev_i->tx_errors,
			       &st_net_edev_i->tx_dropped,
			       &st_net_edev_i->tx_fifo_errors,
			       &st_net_edev_i->collisions,
			       &st_net_edev_i->tx_carrier_errors);
			dev++;
		}
	}

	fclose(fp);
}

/*
 ***************************************************************************
 * Read NFS client statistics from /proc/net/rpc/nfs.
 *
 * IN:
 * @st_net_nfs	Structure where stats will be saved.
 *
 * OUT:
 * @st_net_nfs	Structure with statistics.
 ***************************************************************************
 */
void read_net_nfs(struct stats_net_nfs *st_net_nfs)
{
	FILE *fp;
	char line[256];
	unsigned int getattcnt = 0, accesscnt = 0, readcnt = 0, writecnt = 0;

	if ((fp = fopen(NET_RPC_NFS, "r")) == NULL)
		return;

	memset(st_net_nfs, 0, STATS_NET_NFS_SIZE);
	
	while (fgets(line, 256, fp) != NULL) {

		if (!strncmp(line, "rpc ", 4)) {
			sscanf(line + 4, "%u %u",
			       &st_net_nfs->nfs_rpccnt, &st_net_nfs->nfs_rpcretrans);
		}
		else if (!strncmp(line, "proc3 ", 6)) {
			sscanf(line + 6, "%*u %*u %u %*u %*u %u %*u %u %u",
			       &getattcnt, &accesscnt, &readcnt, &writecnt);
			
			st_net_nfs->nfs_getattcnt += getattcnt;
			st_net_nfs->nfs_accesscnt += accesscnt;
			st_net_nfs->nfs_readcnt   += readcnt;
			st_net_nfs->nfs_writecnt  += writecnt;
		}
		else if (!strncmp(line, "proc4 ", 6)) {
			sscanf(line + 6, "%*u %*u %u %u "
			       "%*u %*u %*u %*u %*u %*u %*u %*u %*u %*u %*u %*u %*u %*u %u %u",
			       &readcnt, &writecnt, &accesscnt, &getattcnt);
			
			st_net_nfs->nfs_getattcnt += getattcnt;
			st_net_nfs->nfs_accesscnt += accesscnt;
			st_net_nfs->nfs_readcnt   += readcnt;
			st_net_nfs->nfs_writecnt  += writecnt;
		}
	}

	fclose(fp);
}

/*
 ***************************************************************************
 * Read NFS server statistics from /proc/net/rpc/nfsd.
 *
 * IN:
 * @st_net_nfsd	Structure where stats will be saved.
 *
 * OUT:
 * @st_net_nfsd	Structure with statistics.
 ***************************************************************************
 */
void read_net_nfsd(struct stats_net_nfsd *st_net_nfsd)
{
	FILE *fp;
	char line[256];
	unsigned int getattcnt = 0, accesscnt = 0, readcnt = 0, writecnt = 0;

	if ((fp = fopen(NET_RPC_NFSD, "r")) == NULL)
		return;
	
	memset(st_net_nfsd, 0, STATS_NET_NFSD_SIZE);

	while (fgets(line, 256, fp) != NULL) {

		if (!strncmp(line, "rc ", 3)) {
			sscanf(line + 3, "%u %u",
			       &st_net_nfsd->nfsd_rchits, &st_net_nfsd->nfsd_rcmisses);
		}
		else if (!strncmp(line, "net ", 4)) {
			sscanf(line + 4, "%u %u %u",
			       &st_net_nfsd->nfsd_netcnt, &st_net_nfsd->nfsd_netudpcnt,
			       &st_net_nfsd->nfsd_nettcpcnt);
		}
		else if (!strncmp(line, "rpc ", 4)) {
			sscanf(line + 4, "%u %u",
			       &st_net_nfsd->nfsd_rpccnt, &st_net_nfsd->nfsd_rpcbad);
		}
		else if (!strncmp(line, "proc3 ", 6)) {
			sscanf(line + 6, "%*u %*u %u %*u %*u %u %*u %u %u",
			       &getattcnt, &accesscnt, &readcnt, &writecnt);

			st_net_nfsd->nfsd_getattcnt += getattcnt;
			st_net_nfsd->nfsd_accesscnt += accesscnt;
			st_net_nfsd->nfsd_readcnt   += readcnt;
			st_net_nfsd->nfsd_writecnt  += writecnt;
			
		}
		else if (!strncmp(line, "proc4ops ", 9)) {
			sscanf(line + 9, "%*u %*u %*u %*u %u "
			       "%*u %*u %*u %*u %*u %u "
			       "%*u %*u %*u %*u %*u %*u %*u %*u %*u %*u %*u %*u %*u %*u %*u %u "
			       "%*u %*u %*u %*u %*u %*u %*u %*u %*u %*u %*u %*u %u",
			       &accesscnt, &getattcnt, &readcnt, &writecnt);
			
			st_net_nfsd->nfsd_getattcnt += getattcnt;
			st_net_nfsd->nfsd_accesscnt += accesscnt;
			st_net_nfsd->nfsd_readcnt   += readcnt;
			st_net_nfsd->nfsd_writecnt  += writecnt;
		}
	}

	fclose(fp);
}

/*
 ***************************************************************************
 * Read network sockets statistics from /proc/net/sockstat.
 *
 * IN:
 * @st_net_sock	Structure where stats will be saved.
 *
 * OUT:
 * @st_net_sock	Structure with statistics.
 ***************************************************************************
 */
void read_net_sock(struct stats_net_sock *st_net_sock)
{
	FILE *fp;
	char line[96];
	char *p;

	if ((fp = fopen(NET_SOCKSTAT, "r")) == NULL)
		return;
	
	while (fgets(line, 96, fp) != NULL) {

		if (!strncmp(line, "sockets:", 8)) {
			/* Sockets */
			sscanf(line + 14, "%u", &st_net_sock->sock_inuse);
		}
		else if (!strncmp(line, "TCP:", 4)) {
			/* TCP sockets */
			sscanf(line + 11, "%u", &st_net_sock->tcp_inuse);
			if ((p = strstr(line, "tw")) != NULL) {
				sscanf(p + 2, "%u", &st_net_sock->tcp_tw);
			}
		}
		else if (!strncmp(line, "UDP:", 4)) {
			/* UDP sockets */
			sscanf(line + 11, "%u", &st_net_sock->udp_inuse);
		}
		else if (!strncmp(line, "RAW:", 4)) {
			/* RAW sockets */
			sscanf(line + 11, "%u", &st_net_sock->raw_inuse);
		}
		else if (!strncmp(line, "FRAG:", 5)) {
			/* FRAGments */
			sscanf(line + 12, "%u", &st_net_sock->frag_inuse);
		}
	}

	fclose(fp);
}

/*
 ***************************************************************************
 * Read IP network traffic statistics from /proc/net/snmp.
 *
 * IN:
 * @st_net_ip	Structure where stats will be saved.
 *
 * OUT:
 * @st_net_ip	Structure with statistics.
 ***************************************************************************
 */
void read_net_ip(struct stats_net_ip *st_net_ip)
{
	FILE *fp;
	char line[1024];
	int sw = FALSE;

	if ((fp = fopen(NET_SNMP, "r")) == NULL)
		return;
	
	while (fgets(line, 1024, fp) != NULL) {

		if (!strncmp(line, "Ip:", 3)) {
			if (sw) {
				sscanf(line + 3, "%*u %*u %lu %*u %*u %lu %*u %*u "
				       "%lu %lu %*u %*u %*u %lu %lu %*u %lu %*u %lu",
				       &st_net_ip->InReceives,
				       &st_net_ip->ForwDatagrams,
				       &st_net_ip->InDelivers,
				       &st_net_ip->OutRequests,
				       &st_net_ip->ReasmReqds,
				       &st_net_ip->ReasmOKs,
				       &st_net_ip->FragOKs,
				       &st_net_ip->FragCreates);
				
				break;
			}
			else {
				sw = TRUE;
			}
		}
	}

	fclose(fp);
}

/*
 ***************************************************************************
 * Read IP network error statistics from /proc/net/snmp.
 *
 * IN:
 * @st_net_eip	Structure where stats will be saved.
 *
 * OUT:
 * @st_net_eip	Structure with statistics.
 ***************************************************************************
 */
void read_net_eip(struct stats_net_eip *st_net_eip)
{
	FILE *fp;
	char line[1024];
	int sw = FALSE;

	if ((fp = fopen(NET_SNMP, "r")) == NULL)
		return;
	
	while (fgets(line, 1024, fp) != NULL) {

		if (!strncmp(line, "Ip:", 3)) {
			if (sw) {
				sscanf(line + 3, "%*u %*u %*u %lu %lu %*u %lu %lu "
				       "%*u %*u %lu %lu %*u %*u %*u %lu %*u %lu",
				       &st_net_eip->InHdrErrors,
				       &st_net_eip->InAddrErrors,
				       &st_net_eip->InUnknownProtos,
				       &st_net_eip->InDiscards,
				       &st_net_eip->OutDiscards,
				       &st_net_eip->OutNoRoutes,
				       &st_net_eip->ReasmFails,
				       &st_net_eip->FragFails);
				
				break;
			}
			else {
				sw = TRUE;
			}
		}
	}

	fclose(fp);
}

/*
 ***************************************************************************
 * Read ICMP network traffic statistics from /proc/net/snmp.
 *
 * IN:
 * @st_net_icmp	Structure where stats will be saved.
 *
 * OUT:
 * @st_net_icmp	Structure with statistics.
 ***************************************************************************
 */
void read_net_icmp(struct stats_net_icmp *st_net_icmp)
{
	FILE *fp;
	char line[1024];
	int sw = FALSE;

	if ((fp = fopen(NET_SNMP, "r")) == NULL)
		return;
	
	while (fgets(line, 1024, fp) != NULL) {

		if (!strncmp(line, "Icmp:", 5)) {
			if (sw) {
				sscanf(line + 5, "%lu %*u %*u %*u %*u %*u %*u "
				       "%lu %lu %lu %lu %lu %lu %lu %*u %*u %*u %*u "
				       "%*u %*u %lu %lu %lu %lu %lu %lu",
				       &st_net_icmp->InMsgs,
				       &st_net_icmp->InEchos,
				       &st_net_icmp->InEchoReps,
				       &st_net_icmp->InTimestamps,
				       &st_net_icmp->InTimestampReps,
				       &st_net_icmp->InAddrMasks,
				       &st_net_icmp->InAddrMaskReps,
				       &st_net_icmp->OutMsgs,
				       &st_net_icmp->OutEchos,
				       &st_net_icmp->OutEchoReps,
				       &st_net_icmp->OutTimestamps,
				       &st_net_icmp->OutTimestampReps,
				       &st_net_icmp->OutAddrMasks,
				       &st_net_icmp->OutAddrMaskReps);

				break;
			}
			else {
				sw = TRUE;
			}
		}
	}

	fclose(fp);
}

/*
 ***************************************************************************
 * Read ICMP network error statistics from /proc/net/snmp.
 *
 * IN:
 * @st_net_eicmp	Structure where stats will be saved.
 *
 * OUT:
 * @st_net_eicmp	Structure with statistics.
 ***************************************************************************
 */
void read_net_eicmp(struct stats_net_eicmp *st_net_eicmp)
{
	FILE *fp;
	char line[1024];
	int sw = FALSE;

	if ((fp = fopen(NET_SNMP, "r")) == NULL)
		return;
	
	while (fgets(line, 1024, fp) != NULL) {

		if (!strncmp(line, "Icmp:", 5)) {
			if (sw) {
				sscanf(line + 5, "%*u %lu %lu %lu %lu %lu %lu %*u %*u "
				       "%*u %*u %*u %*u %*u %lu %lu %lu %lu %lu %lu",
				       &st_net_eicmp->InErrors,
				       &st_net_eicmp->InDestUnreachs,
				       &st_net_eicmp->InTimeExcds,
				       &st_net_eicmp->InParmProbs,
				       &st_net_eicmp->InSrcQuenchs,
				       &st_net_eicmp->InRedirects,
				       &st_net_eicmp->OutErrors,
				       &st_net_eicmp->OutDestUnreachs,
				       &st_net_eicmp->OutTimeExcds,
				       &st_net_eicmp->OutParmProbs,
				       &st_net_eicmp->OutSrcQuenchs,
				       &st_net_eicmp->OutRedirects);

				break;
			}
			else {
				sw = TRUE;
			}
		}
	}

	fclose(fp);
}

/*
 ***************************************************************************
 * Read TCP network traffic statistics from /proc/net/snmp.
 *
 * IN:
 * @st_net_tcp	Structure where stats will be saved.
 *
 * OUT:
 * @st_net_tcp	Structure with statistics.
 ***************************************************************************
 */
void read_net_tcp(struct stats_net_tcp *st_net_tcp)
{
	FILE *fp;
	char line[1024];
	int sw = FALSE;

	if ((fp = fopen(NET_SNMP, "r")) == NULL)
		return;
	
	while (fgets(line, 1024, fp) != NULL) {

		if (!strncmp(line, "Tcp:", 4)) {
			if (sw) {
				sscanf(line + 4, "%*u %*u %*u %*d %lu %lu "
				       "%*u %*u %*u %lu %lu",
				       &st_net_tcp->ActiveOpens,
				       &st_net_tcp->PassiveOpens,
				       &st_net_tcp->InSegs,
				       &st_net_tcp->OutSegs);

				break;
			}
			else {
				sw = TRUE;
			}
		}
	}

	fclose(fp);
}

/*
 ***************************************************************************
 * Read TCP network error statistics from /proc/net/snmp.
 *
 * IN:
 * @st_net_etcp	Structure where stats will be saved.
 *
 * OUT:
 * @st_net_etcp	Structure with statistics.
 ***************************************************************************
 */
void read_net_etcp(struct stats_net_etcp *st_net_etcp)
{
	FILE *fp;
	char line[1024];
	int sw = FALSE;

	if ((fp = fopen(NET_SNMP, "r")) == NULL)
		return;
	
	while (fgets(line, 1024, fp) != NULL) {

		if (!strncmp(line, "Tcp:", 4)) {
			if (sw) {
				sscanf(line + 4, "%*u %*u %*u %*d %*u %*u "
				       "%lu %lu %*u %*u %*u %lu %lu %lu",
				       &st_net_etcp->AttemptFails,
				       &st_net_etcp->EstabResets,
				       &st_net_etcp->RetransSegs,
				       &st_net_etcp->InErrs,
				       &st_net_etcp->OutRsts);

				break;
			}
			else {
				sw = TRUE;
			}
		}
	}

	fclose(fp);
}

/*
 ***************************************************************************
 * Read UDP network traffic statistics from /proc/net/snmp.
 *
 * IN:
 * @st_net_udp	Structure where stats will be saved.
 *
 * OUT:
 * @st_net_udp	Structure with statistics.
 ***************************************************************************
 */
void read_net_udp(struct stats_net_udp *st_net_udp)
{
	FILE *fp;
	char line[1024];
	int sw = FALSE;

	if ((fp = fopen(NET_SNMP, "r")) == NULL)
		return;
	
	while (fgets(line, 1024, fp) != NULL) {

		if (!strncmp(line, "Udp:", 4)) {
			if (sw) {
				sscanf(line + 4, "%lu %lu %lu %lu",
				       &st_net_udp->InDatagrams,
				       &st_net_udp->NoPorts,
				       &st_net_udp->InErrors,
				       &st_net_udp->OutDatagrams);

				break;
			}
			else {
				sw = TRUE;
			}
		}
	}

	fclose(fp);
}

/*
 ***************************************************************************
 * Read IPv6 network sockets statistics from /proc/net/sockstat6.
 *
 * IN:
 * @st_net_sock6	Structure where stats will be saved.
 *
 * OUT:
 * @st_net_sock6	Structure with statistics.
 ***************************************************************************
 */
void read_net_sock6(struct stats_net_sock6 *st_net_sock6)
{
	FILE *fp;
	char line[96];

	if ((fp = fopen(NET_SOCKSTAT6, "r")) == NULL)
		return;
	
	while (fgets(line, 96, fp) != NULL) {

		if (!strncmp(line, "TCP6:", 5)) {
			/* TCPv6 sockets */
			sscanf(line + 12, "%u", &st_net_sock6->tcp6_inuse);
		}
		else if (!strncmp(line, "UDP6:", 5)) {
			/* UDPv6 sockets */
			sscanf(line + 12, "%u", &st_net_sock6->udp6_inuse);
		}
		else if (!strncmp(line, "RAW6:", 5)) {
			/* IPv6 RAW sockets */
			sscanf(line + 12, "%u", &st_net_sock6->raw6_inuse);
		}
		else if (!strncmp(line, "FRAG6:", 6)) {
			/* IPv6 FRAGments */
			sscanf(line + 13, "%u", &st_net_sock6->frag6_inuse);
		}
	}

	fclose(fp);
}

/*
 ***************************************************************************
 * Read IPv6 network traffic statistics from /proc/net/snmp6.
 *
 * IN:
 * @st_net_ip6	Structure where stats will be saved.
 *
 * OUT:
 * @st_net_ip6	Structure with statistics.
 ***************************************************************************
 */
void read_net_ip6(struct stats_net_ip6 *st_net_ip6)
{
	FILE *fp;
	char line[128];

	if ((fp = fopen(NET_SNMP6, "r")) == NULL)
		return;

	while (fgets(line, 128, fp) != NULL) {

		if (!strncmp(line, "Ip6InReceives ", 14)) {
			sscanf(line + 14, "%lu", &st_net_ip6->InReceives6);
		}
		else if (!strncmp(line, "Ip6OutForwDatagrams ", 20)) {
			sscanf(line + 20, "%lu", &st_net_ip6->OutForwDatagrams6);
		}
		else if (!strncmp(line, "Ip6InDelivers ", 14)) {
			sscanf(line + 14, "%lu", &st_net_ip6->InDelivers6);
		}
		else if (!strncmp(line, "Ip6OutRequests ", 15)) {
			sscanf(line + 15, "%lu", &st_net_ip6->OutRequests6);
		}
		else if (!strncmp(line, "Ip6ReasmReqds ", 14)) {
			sscanf(line + 14, "%lu", &st_net_ip6->ReasmReqds6);
		}
		else if (!strncmp(line, "Ip6ReasmOKs ", 12)) {
			sscanf(line + 12, "%lu", &st_net_ip6->ReasmOKs6);
		}
		else if (!strncmp(line, "Ip6InMcastPkts ", 15)) {
			sscanf(line + 15, "%lu", &st_net_ip6->InMcastPkts6);
		}
		else if (!strncmp(line, "Ip6OutMcastPkts ", 16)) {
			sscanf(line + 16, "%lu", &st_net_ip6->OutMcastPkts6);
		}
		else if (!strncmp(line, "Ip6FragOKs ", 11)) {
			sscanf(line + 11, "%lu", &st_net_ip6->FragOKs6);
		}
		else if (!strncmp(line, "Ip6FragCreates ", 15)) {
			sscanf(line + 15, "%lu", &st_net_ip6->FragCreates6);
		}
	}
	
	fclose(fp);
}

/*
 ***************************************************************************
 * Read IPv6 network error statistics from /proc/net/snmp6.
 *
 * IN:
 * @st_net_eip6	Structure where stats will be saved.
 *
 * OUT:
 * @st_net_eip6	Structure with statistics.
 ***************************************************************************
 */
void read_net_eip6(struct stats_net_eip6 *st_net_eip6)
{
	FILE *fp;
	char line[128];

	if ((fp = fopen(NET_SNMP6, "r")) == NULL)
		return;

	while (fgets(line, 128, fp) != NULL) {

		if (!strncmp(line, "Ip6InHdrErrors ", 15)) {
			sscanf(line + 15, "%lu", &st_net_eip6->InHdrErrors6);
		}
		else if (!strncmp(line, "Ip6InAddrErrors ", 16)) {
			sscanf(line + 16, "%lu", &st_net_eip6->InAddrErrors6);
		}
		else if (!strncmp(line, "Ip6InUnknownProtos ", 19)) {
			sscanf(line + 19, "%lu", &st_net_eip6->InUnknownProtos6);
		}
		else if (!strncmp(line, "Ip6InTooBigErrors ", 18)) {
			sscanf(line + 18, "%lu", &st_net_eip6->InTooBigErrors6);
		}
		else if (!strncmp(line, "Ip6InDiscards ", 14)) {
			sscanf(line + 14, "%lu", &st_net_eip6->InDiscards6);
		}
		else if (!strncmp(line, "Ip6OutDiscards ", 15)) {
			sscanf(line + 15, "%lu", &st_net_eip6->OutDiscards6);
		}
		else if (!strncmp(line, "Ip6InNoRoutes ", 14)) {
			sscanf(line + 14, "%lu", &st_net_eip6->InNoRoutes6);
		}
		else if (!strncmp(line, "Ip6OutNoRoutes ", 15)) {
			sscanf(line + 15, "%lu", &st_net_eip6->OutNoRoutes6);
		}
		else if (!strncmp(line, "Ip6ReasmFails ", 14)) {
			sscanf(line + 14, "%lu", &st_net_eip6->ReasmFails6);
		}
		else if (!strncmp(line, "Ip6FragFails ", 13)) {
			sscanf(line + 13, "%lu", &st_net_eip6->FragFails6);
		}
		else if (!strncmp(line, "Ip6InTruncatedPkts ", 19)) {
			sscanf(line + 19, "%lu", &st_net_eip6->InTruncatedPkts6);
		}
	}
	
	fclose(fp);
}

/*
 ***************************************************************************
 * Read ICMPv6 network traffic statistics from /proc/net/snmp6.
 *
 * IN:
 * @st_net_icmp6	Structure where stats will be saved.
 *
 * OUT:
 * @st_net_icmp6	Structure with statistics.
 ***************************************************************************
 */
void read_net_icmp6(struct stats_net_icmp6 *st_net_icmp6)
{
	FILE *fp;
	char line[128];

	if ((fp = fopen(NET_SNMP6, "r")) == NULL)
		return;

	while (fgets(line, 128, fp) != NULL) {

		if (!strncmp(line, "Icmp6InMsgs ", 12)) {
			sscanf(line + 12, "%lu", &st_net_icmp6->InMsgs6);
		}
		else if (!strncmp(line, "Icmp6OutMsgs ", 13)) {
			sscanf(line + 13, "%lu", &st_net_icmp6->OutMsgs6);
		}
		else if (!strncmp(line, "Icmp6InEchos ", 13)) {
			sscanf(line + 13, "%lu", &st_net_icmp6->InEchos6);
		}
		else if (!strncmp(line, "Icmp6InEchoReplies ", 19)) {
			sscanf(line + 19, "%lu", &st_net_icmp6->InEchoReplies6);
		}
		else if (!strncmp(line, "Icmp6OutEchoReplies ", 20)) {
			sscanf(line + 20, "%lu", &st_net_icmp6->OutEchoReplies6);
		}
		else if (!strncmp(line, "Icmp6InGroupMembQueries ", 24)) {
			sscanf(line + 24, "%lu", &st_net_icmp6->InGroupMembQueries6);
		}
		else if (!strncmp(line, "Icmp6InGroupMembResponses ", 26)) {
			sscanf(line + 26, "%lu", &st_net_icmp6->InGroupMembResponses6);
		}
		else if (!strncmp(line, "Icmp6OutGroupMembResponses ", 27)) {
			sscanf(line + 27, "%lu", &st_net_icmp6->OutGroupMembResponses6);
		}
		else if (!strncmp(line, "Icmp6InGroupMembReductions ", 27)) {
			sscanf(line + 27, "%lu", &st_net_icmp6->InGroupMembReductions6);
		}
		else if (!strncmp(line, "Icmp6OutGroupMembReductions ", 28)) {
			sscanf(line + 28, "%lu", &st_net_icmp6->OutGroupMembReductions6);
		}
		else if (!strncmp(line, "Icmp6InRouterSolicits ", 22)) {
			sscanf(line + 22, "%lu", &st_net_icmp6->InRouterSolicits6);
		}
		else if (!strncmp(line, "Icmp6OutRouterSolicits ", 23)) {
			sscanf(line + 23, "%lu", &st_net_icmp6->OutRouterSolicits6);
		}
		else if (!strncmp(line, "Icmp6InRouterAdvertisements ", 28)) {
			sscanf(line + 28, "%lu", &st_net_icmp6->InRouterAdvertisements6);
		}
		else if (!strncmp(line, "Icmp6InNeighborSolicits ", 24)) {
			sscanf(line + 24, "%lu", &st_net_icmp6->InNeighborSolicits6);
		}
		else if (!strncmp(line, "Icmp6OutNeighborSolicits ", 25)) {
			sscanf(line + 25, "%lu", &st_net_icmp6->OutNeighborSolicits6);
		}
		else if (!strncmp(line, "Icmp6InNeighborAdvertisements ", 30)) {
			sscanf(line + 30, "%lu", &st_net_icmp6->InNeighborAdvertisements6);
		}
		else if (!strncmp(line, "Icmp6OutNeighborAdvertisements ", 31)) {
			sscanf(line + 31, "%lu", &st_net_icmp6->OutNeighborAdvertisements6);
		}
	}
	
	fclose(fp);
}

/*
 ***************************************************************************
 * Read ICMPv6 network error statistics from /proc/net/snmp6.
 *
 * IN:
 * @st_net_eicmp6	Structure where stats will be saved.
 *
 * OUT:
 * @st_net_eicmp6	Structure with statistics.
 ***************************************************************************
 */
void read_net_eicmp6(struct stats_net_eicmp6 *st_net_eicmp6)
{
	FILE *fp;
	char line[128];

	if ((fp = fopen(NET_SNMP6, "r")) == NULL)
		return;

	while (fgets(line, 128, fp) != NULL) {

		if (!strncmp(line, "Icmp6InErrors ", 14)) {
			sscanf(line + 14, "%lu", &st_net_eicmp6->InErrors6);
		}
		else if (!strncmp(line, "Icmp6InDestUnreachs ", 20)) {
			sscanf(line + 20, "%lu", &st_net_eicmp6->InDestUnreachs6);
		}
		else if (!strncmp(line, "Icmp6OutDestUnreachs ", 21)) {
			sscanf(line + 21, "%lu", &st_net_eicmp6->OutDestUnreachs6);
		}
		else if (!strncmp(line, "Icmp6InTimeExcds ", 17)) {
			sscanf(line + 17, "%lu", &st_net_eicmp6->InTimeExcds6);
		}
		else if (!strncmp(line, "Icmp6OutTimeExcds ", 18)) {
			sscanf(line + 18, "%lu", &st_net_eicmp6->OutTimeExcds6);
		}
		else if (!strncmp(line, "Icmp6InParmProblems ", 20)) {
			sscanf(line + 20, "%lu", &st_net_eicmp6->InParmProblems6);
		}
		else if (!strncmp(line, "Icmp6OutParmProblems ", 21)) {
			sscanf(line + 21, "%lu", &st_net_eicmp6->OutParmProblems6);
		}
		else if (!strncmp(line, "Icmp6InRedirects ", 17)) {
			sscanf(line + 17, "%lu", &st_net_eicmp6->InRedirects6);
		}
		else if (!strncmp(line, "Icmp6OutRedirects ", 18)) {
			sscanf(line + 18, "%lu", &st_net_eicmp6->OutRedirects6);
		}
		else if (!strncmp(line, "Icmp6InPktTooBigs ", 18)) {
			sscanf(line + 18, "%lu", &st_net_eicmp6->InPktTooBigs6);
		}
		else if (!strncmp(line, "Icmp6OutPktTooBigs ", 19)) {
			sscanf(line + 19, "%lu", &st_net_eicmp6->OutPktTooBigs6);
		}
	}
	
	fclose(fp);
}

/*
 ***************************************************************************
 * Read UDPv6 network traffic statistics from /proc/net/snmp6.
 *
 * IN:
 * @st_net_udp6	Structure where stats will be saved.
 *
 * OUT:
 * @st_net_udp6	Structure with statistics.
 ***************************************************************************
 */
void read_net_udp6(struct stats_net_udp6 *st_net_udp6)
{
	FILE *fp;
	char line[128];

	if ((fp = fopen(NET_SNMP6, "r")) == NULL)
		return;

	while (fgets(line, 128, fp) != NULL) {

		if (!strncmp(line, "Udp6InDatagrams ", 16)) {
			sscanf(line + 16, "%lu", &st_net_udp6->InDatagrams6);
		}
		else if (!strncmp(line, "Udp6OutDatagrams ", 17)) {
			sscanf(line + 17, "%lu", &st_net_udp6->OutDatagrams6);
		}
		else if (!strncmp(line, "Udp6NoPorts ", 12)) {
			sscanf(line + 12, "%lu", &st_net_udp6->NoPorts6);
		}
		else if (!strncmp(line, "Udp6InErrors ", 13)) {
			sscanf(line + 13, "%lu", &st_net_udp6->InErrors6);
		}
	}
	
	fclose(fp);
}

/*
 ***************************************************************************
 * Read CPU frequency statistics.
 *
 * IN:
 * @st_pwr_cpufreq	Structure where stats will be saved.
 * @nbr			Total number of CPU (including cpu "all").
 *
 * OUT:
 * @st_pwr_cpufreq	Structure with statistics.
 ***************************************************************************
 */
void read_cpuinfo(struct stats_pwr_cpufreq *st_pwr_cpufreq, int nbr)
{
	FILE *fp;
	struct stats_pwr_cpufreq *st_pwr_cpufreq_i;
	char line[1024];
	int proc_nb = 0, nr = 0;
	unsigned int ifreq, dfreq;
	
	if ((fp = fopen(CPUINFO, "r")) == NULL)
		return;
	
	st_pwr_cpufreq->cpufreq = 0;
	
	while (fgets(line, 1024, fp) != NULL) {
		
		if (!strncmp(line, "processor\t", 10)) {
			sscanf(strchr(line, ':') + 1, "%d", &proc_nb);
		}
		
		else if (!strncmp(line, "cpu MHz\t", 8)) {
			sscanf(strchr(line, ':') + 1, "%u.%u", &ifreq, &dfreq);
			
			if (proc_nb < (nbr - 1)) {
				/* Save current CPU frequency */
				st_pwr_cpufreq_i = st_pwr_cpufreq + proc_nb + 1;
				st_pwr_cpufreq_i->cpufreq = ifreq * 100 + dfreq / 10;
				
				/* Also save it to compute an average CPU frequency */
				st_pwr_cpufreq->cpufreq += st_pwr_cpufreq_i->cpufreq;
				nr++;
			}
			else if (!proc_nb && (nbr == 1)) {
				/*
				 * We are reading freq for "Processor 0" and we have a machine
				 * with only one processor and not an SMP kernel, with /sys not mounted
				 * (the nr of proc has been counted using /proc/stat and there was
				 * only one line with global CPU stats here).
				 * This is a very specific case, I must admit...
				 */
				st_pwr_cpufreq->cpufreq = ifreq * 100 + dfreq / 10;
			}
		}
	}
	
	fclose(fp);
	
	if (nr) {
		/* Compute average CPU frequency for this machine */
		st_pwr_cpufreq->cpufreq /= nr;
	}
}

/*
 ***************************************************************************
 * Read hugepages statistics from /proc/meminfo.
 *
 * IN:
 * @st_huge	Structure where stats will be saved.
 *
 * OUT:
 * @st_huge	Structure with statistics.
 ***************************************************************************
 */
void read_meminfo_huge(struct stats_huge *st_huge)
{
	FILE *fp;
	char line[128];
	unsigned long szhkb = 0;

	if ((fp = fopen(MEMINFO, "r")) == NULL)
		return;

	while (fgets(line, 128, fp) != NULL) {

		if (!strncmp(line, "HugePages_Total:", 16)) {
			/* Read the total number of huge pages */
			sscanf(line + 16, "%lu", &st_huge->tlhkb);
		}
		else if (!strncmp(line, "HugePages_Free:", 15)) {
			/* Read the number of free huge pages */
			sscanf(line + 15, "%lu", &st_huge->frhkb);
		}
		else if (!strncmp(line, "Hugepagesize:", 13)) {
			/* Read the default size of a huge page in kB */
			sscanf(line + 13, "%lu", &szhkb);
		}
	}

	fclose(fp);

	/* We want huge pages stats in kB and not expressed in a number of pages */
	st_huge->tlhkb *= szhkb;
	st_huge->frhkb *= szhkb;
}

/*
 ***************************************************************************
 * Read CPU average frequencies statistics.
 *
 * IN:
 * @st_pwr_wghfreq	Structure where stats will be saved.
 * @cpu_nr		CPU number for which time_in_state date will be read.
 * @nbr			Total number of states (frequencies).
 *
 * OUT:
 * @st_pwr_wghfreq	Structure with statistics.
 ***************************************************************************
 */
void read_time_in_state(struct stats_pwr_wghfreq *st_pwr_wghfreq, int cpu_nr, int nbr)
{
	FILE *fp;
	struct stats_pwr_wghfreq *st_pwr_wghfreq_j;
	char filename[MAX_PF_NAME];
	char line[128];
	int j = 0;
	unsigned long freq;
	unsigned long long time_in_state;

	snprintf(filename, MAX_PF_NAME, "%s/cpu%d/%s",
		 SYSFS_DEVCPU, cpu_nr, SYSFS_TIME_IN_STATE);
	if ((fp = fopen(filename, "r")) == NULL)
		return;

	while (fgets(line, 128, fp) != NULL) {

		sscanf(line, "%lu %llu", &freq, &time_in_state);

		if (j < nbr) {
			/* Save current frequency and time */
			st_pwr_wghfreq_j = st_pwr_wghfreq + j;
			st_pwr_wghfreq_j->freq = freq;
			st_pwr_wghfreq_j->time_in_state = time_in_state;
			j++;
		}
	}

	fclose(fp);
}

/*
 ***************************************************************************
 * Read current USB device data.
 *
 * IN:
 * @st_pwr_usb		Structure where stats will be saved.
 * @usb_device		File name for current USB device.
 *
 * OUT:
 * @st_pwr_usb		Structure with statistics.
 ***************************************************************************
 */
void read_usb_stats(struct stats_pwr_usb *st_pwr_usb, char *usb_device)
{
	int l;
	FILE *fp;
	char filename[MAX_PF_NAME];

	/* Get USB device bus number */
	sscanf(usb_device, "%u", &st_pwr_usb->bus_nr);

	/* Read USB device vendor ID */
	snprintf(filename, MAX_PF_NAME, "%s/%s/%s",
		 SYSFS_USBDEV, usb_device, SYSFS_IDVENDOR);
	if ((fp = fopen(filename, "r")) != NULL) {
		fscanf(fp, "%x",
		       &st_pwr_usb->vendor_id);
		fclose(fp);
	}

	/* Read USB device product ID */
	snprintf(filename, MAX_PF_NAME, "%s/%s/%s",
		 SYSFS_USBDEV, usb_device, SYSFS_IDPRODUCT);
	if ((fp = fopen(filename, "r")) != NULL) {
		fscanf(fp, "%x",
		       &st_pwr_usb->product_id);
		fclose(fp);
	}
	
	/* Read USB device max power consumption */
	snprintf(filename, MAX_PF_NAME, "%s/%s/%s",
		 SYSFS_USBDEV, usb_device, SYSFS_BMAXPOWER);
	if ((fp = fopen(filename, "r")) != NULL) {
		fscanf(fp, "%u",
		       &st_pwr_usb->bmaxpower);
		fclose(fp);
	}

	/* Read USB device manufacturer */
	snprintf(filename, MAX_PF_NAME, "%s/%s/%s",
		 SYSFS_USBDEV, usb_device, SYSFS_MANUFACTURER);
	if ((fp = fopen(filename, "r")) != NULL) {
		fgets(st_pwr_usb->manufacturer, MAX_MANUF_LEN - 1, fp);
		fclose(fp);
		if ((l = strlen(st_pwr_usb->manufacturer)) > 0) {
			/* Remove trailing CR */
			st_pwr_usb->manufacturer[l - 1] = '\0';
		}
	}

	/* Read USB device product */
	snprintf(filename, MAX_PF_NAME, "%s/%s/%s",
		 SYSFS_USBDEV, usb_device, SYSFS_PRODUCT);
	if ((fp = fopen(filename, "r")) != NULL) {
		fgets(st_pwr_usb->product, MAX_PROD_LEN - 1, fp);
		fclose(fp);
		if ((l = strlen(st_pwr_usb->product)) > 0) {
			/* Remove trailing CR */
			st_pwr_usb->product[l - 1] = '\0';
		}
	}
}

/*
 ***************************************************************************
 * Read USB devices statistics.
 *
 * IN:
 * @st_pwr_usb		Structure where stats will be saved.
 * @nbr			Total number of USB devices.
 *
 * OUT:
 * @st_pwr_usb		Structure with statistics.
 ***************************************************************************
 */
void read_bus_usb_dev(struct stats_pwr_usb *st_pwr_usb, int nbr)
{
	DIR *dir;
	struct dirent *drd;
	struct stats_pwr_usb *st_pwr_usb_j;
	int j = 0;

	/* Open relevant /sys directory */
	if ((dir = opendir(SYSFS_USBDEV)) == NULL)
		return;

	/* Get current file entry */
	while ((drd = readdir(dir)) != NULL) {

		if (isdigit(drd->d_name[0]) && !strchr(drd->d_name, ':')) {
			if (j < nbr) {
				/* Read current USB device data */
				st_pwr_usb_j = st_pwr_usb + j;
				read_usb_stats(st_pwr_usb_j, drd->d_name);
				j++;
			}
			else
				break;
		}
	}

	/* Close directory */
	closedir(dir);
}

/*
 ***************************************************************************
 * Read machine uptime, independently of the number of processors.
 *
 * OUT:
 * @uptime	Uptime value in jiffies.
 ***************************************************************************
 */
void read_uptime(unsigned long long *uptime)
{
	FILE *fp;
	char line[128];
	unsigned long up_sec, up_cent;

	if ((fp = fopen(UPTIME, "r")) == NULL)
		return;

	if (fgets(line, 128, fp) == NULL) {
		fclose(fp);
		return;
	}

	sscanf(line, "%lu.%lu", &up_sec, &up_cent);
	*uptime = (unsigned long long) up_sec * HZ +
	          (unsigned long long) up_cent * HZ / 100;

	fclose(fp);

}

/*
 ***************************************************************************
 * Count number of interrupts that are in /proc/stat file.
 *
 * RETURNS:
 * Number of interrupts, including total number of interrupts.
 ***************************************************************************
 */
int get_irq_nr(void)
{
	FILE *fp;
	char line[8192];
	int in = 0;
	int pos = 4;

	if ((fp = fopen(STAT, "r")) == NULL)
		return 0;

	while (fgets(line, 8192, fp) != NULL) {

		if (!strncmp(line, "intr ", 5)) {

			while (pos < strlen(line)) {
				in++;
				pos += strcspn(line + pos + 1, " ") + 1;
			}
		}
	}

	fclose(fp);

	return in;
}

/*
 ***************************************************************************
 * Find number of serial lines that support tx/rx accounting
 * in /proc/tty/driver/serial file.
 *
 * RETURNS:
 * Number of serial lines supporting tx/rx accouting.
 ***************************************************************************
 */
int get_serial_nr(void)
{
	FILE *fp;
	char line[256];
	int sl = 0;

	if ((fp = fopen(SERIAL, "r")) == NULL)
		return 0;	/* No SERIAL file */

	while (fgets(line, 256, fp) != NULL) {
		/*
		 * tx/rx statistics are always present,
		 * except when serial line is unknown.
		 */
		if (strstr(line, "tx:") != NULL) {
			sl++;
		}
	}

	fclose(fp);

	return sl;
}

/*
 ***************************************************************************
 * Find number of interfaces (network devices) that are in /proc/net/dev
 * file.
 *
 * RETURNS:
 * Number of network interfaces.
 ***************************************************************************
 */
int get_iface_nr(void)
{
	FILE *fp;
	char line[128];
	int iface = 0;

	if ((fp = fopen(NET_DEV, "r")) == NULL)
		return 0;	/* No network device file */

	while (fgets(line, 128, fp) != NULL) {
		if (strchr(line, ':')) {
			iface++;
		}
	}

	fclose(fp);

	return iface;
}

/*
 ***************************************************************************
 * Find number of devices and partitions available in /proc/diskstats.
 *
 * IN:
 * @count_part		Set to TRUE if devices _and_ partitions are to be
 *			counted.
 * @only_used_dev	When counting devices, set to TRUE if only devices
 *			with non zero stats must be counted.
 *
 * RETURNS:
 * Number of devices (and partitions).
 ***************************************************************************
 */
int get_diskstats_dev_nr(int count_part, int only_used_dev)
{
	FILE *fp;
	char line[256];
	char dev_name[MAX_NAME_LEN];
	int dev = 0, i;
	unsigned long rd_ios, wr_ios;

	if ((fp = fopen(DISKSTATS, "r")) == NULL)
		/* File non-existent */
		return 0;

	/*
	 * Counting devices and partitions is simply a matter of counting
	 * the number of lines...
	 */
	while (fgets(line, 256, fp) != NULL) {
		if (!count_part) {
			i = sscanf(line, "%*d %*d %s %lu %*u %*u %*u %lu",
				   dev_name, &rd_ios, &wr_ios);
			if ((i == 2) || !is_device(dev_name))
				/* It was a partition and not a device */
				continue;
			if (only_used_dev && !rd_ios && !wr_ios)
				/* Unused device */
				continue;
		}
		dev++;
	}

	fclose(fp);

	return dev;
}

/*
 ***************************************************************************
 * Get number of devices in /proc/diskstats.
 *
 * IN:
 * @f	Non zero (true) if disks *and* partitions should be counted, and
 *	zero (false) if only disks must be counted.
 *
 * RETURNS:
 * Number of devices.
 ***************************************************************************
 */
int get_disk_nr(unsigned int f)
{
	int disk_nr;

	/*
	 * Partitions are taken into account by sar -d only with
	 * kernels 2.6.25 and later.
	 */
	disk_nr = get_diskstats_dev_nr(f, CNT_USED_DEV);

	return disk_nr;
}

/*
 ***************************************************************************
 * Count number of processors in /sys.
 *
 * RETURNS:
 * Number of processors (online and offline).
 * A value of 0 means that /sys was not mounted.
 * A value of N (!=0) means N processor(s) (cpu0 .. cpu(N-1)).
 ***************************************************************************
 */
int get_sys_cpu_nr(void)
{
	DIR *dir;
	struct dirent *drd;
	struct stat buf;
	char line[MAX_PF_NAME];
	int proc_nr = 0;

	/* Open relevant /sys directory */
	if ((dir = opendir(SYSFS_DEVCPU)) == NULL)
		return 0;

	/* Get current file entry */
	while ((drd = readdir(dir)) != NULL) {

		if (!strncmp(drd->d_name, "cpu", 3) && isdigit(drd->d_name[3])) {
			snprintf(line, MAX_PF_NAME, "%s/%s", SYSFS_DEVCPU, drd->d_name);
			line[MAX_PF_NAME - 1] = '\0';
			if (stat(line, &buf) < 0)
				continue;
			if (S_ISDIR(buf.st_mode)) {
				proc_nr++;
			}
		}
	}

	/* Close directory */
	closedir(dir);

	return proc_nr;
}

/*
 ***************************************************************************
 * Count number of processors in /proc/stat.
 *
 * RETURNS:
 * Number of processors. The returned value is greater than or equal to the
 * number of online processors.
 * A value of 0 means one processor and non SMP kernel.
 * A value of N (!=0) means N processor(s) (0 .. N-1) with SMP kernel.
 ***************************************************************************
 */
int get_proc_cpu_nr(void)
{
	FILE *fp;
	char line[16];
	int num_proc, proc_nr = -1;

	if ((fp = fopen(STAT, "r")) == NULL) {
		fprintf(stderr, _("Cannot open %s: %s\n"), STAT, strerror(errno));
		exit(1);
	}

	while (fgets(line, 16, fp) != NULL) {

		if (strncmp(line, "cpu ", 4) && !strncmp(line, "cpu", 3)) {
			sscanf(line + 3, "%d", &num_proc);
			if (num_proc > proc_nr) {
				proc_nr = num_proc;
			}
		}
	}

	fclose(fp);

	return (proc_nr + 1);
}

/*
 ***************************************************************************
 * Count the number of processors on the machine.
 * Try to use /sys for that, or /proc/stat if /sys doesn't exist.
 *
 * IN:
 * @max_nr_cpus	Maximum number of proc that sysstat can handle.
 *
 * RETURNS:
 * Number of processors.
 * 0: one proc and non SMP kernel.
 * 1: one proc and SMP kernel (NB: On SMP machines where all the CPUs but
 *    one have been disabled, we get the total number of proc since we use
 *    /sys to count them).
 * 2: two proc...
 ***************************************************************************
 */
int get_cpu_nr(unsigned int max_nr_cpus)
{
	int cpu_nr;

	if ((cpu_nr = get_sys_cpu_nr()) == 0) {
		/* /sys may be not mounted. Use /proc/stat instead */
		cpu_nr = get_proc_cpu_nr();
	}

	if (cpu_nr > max_nr_cpus) {
		fprintf(stderr, _("Cannot handle so many processors!\n"));
		exit(1);
	}

	return cpu_nr;
}

/*
 ***************************************************************************
 * Find number of interrupts available per processor (use
 * /proc/interrupts file or /proc/softirqs).
 *
 * IN:
 * @file		/proc file to read (interrupts or softirqs).
 * @max_nr_irqcpu       Maximum number of interrupts per processor that
 *                      sadc can handle.
 * @cpu_nr		Number of processors.
 *
 * RETURNS:
 * Number of interrupts per processor + a pre-allocation constant.
 ***************************************************************************
 */
int get_irqcpu_nr(char *file, int max_nr_irqcpu, int cpu_nr)
{
	FILE *fp;
	char *line = NULL;
	unsigned int irq = 0;
	int p;

	if ((fp = fopen(file, "r")) == NULL)
		return 0;       /* No interrupts file */

	SREALLOC(line, char, INTERRUPTS_LINE + 11 * cpu_nr);

	while ((fgets(line, INTERRUPTS_LINE + 11 * cpu_nr , fp) != NULL) &&
	       (irq < max_nr_irqcpu)) {
		p = strcspn(line, ":");
		if ((p > 0) && (p < 16)) {
			irq++;
		}
	}

	fclose(fp);

	if (line) {
		free(line);
	}

	return irq;
}

/*
 ***************************************************************************
 * Count number of possible frequencies for CPU#0.
 *
 * RETURNS:
 * Number of frequencies.
 ***************************************************************************
 */
int get_freq_nr(void)
{
	FILE *fp;
	char filename[MAX_PF_NAME];
	char line[128];
	int freq = 0;

	snprintf(filename, MAX_PF_NAME, "%s/cpu0/%s",
		 SYSFS_DEVCPU, SYSFS_TIME_IN_STATE);
	if ((fp = fopen(filename, "r")) == NULL)
		return 0;	/* No time_in_state file for CPU#0 */

	while (fgets(line, 128, fp) != NULL) {
		freq++;
	}

	fclose(fp);

	return freq;
}

/*
 ***************************************************************************
 * Count number of USB devices in /sys/bus/usb/devices.
 *
 * RETURNS:
 * Number of USB devices plugged into the system.
 * Don't count USB root hubs.
 * Return -1 if directory doesn't exist in sysfs.
 ***************************************************************************
 */
int get_usb_nr(void)
{
	DIR *dir;
	struct dirent *drd;
	int usb = 0;

	/* Open relevant /sys directory */
	if ((dir = opendir(SYSFS_USBDEV)) == NULL)
		return -1;

	/* Get current file entry */
	while ((drd = readdir(dir)) != NULL) {

		if (isdigit(drd->d_name[0]) && !strchr(drd->d_name, ':')) {
			usb++;
		}
	}

	/* Close directory */
	closedir(dir);

	return usb;
}
