/*
 * nfsiostat: Report NFS I/O statistics
 * Copyright (C) 2010 Red Hat, Inc. All Rights Reserved
 * Written by Ivana Varekova <varekova@redhat.com>
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
#include <unistd.h>
#include <signal.h>
#include <sys/utsname.h>

#include "version.h"
#include "nfsiostat.h"
#include "common.h"

#ifdef USE_NLS
#include <locale.h>
#include <libintl.h>
#define _(string) gettext(string)
#else
#define _(string) (string)
#endif

#define SCCSID "@(#)sysstat-" VERSION ": " __FILE__ " compiled " __DATE__ " " __TIME__
char *sccsid(void) { return (SCCSID); }

unsigned long long uptime[2]  = {0, 0};
unsigned long long uptime0[2] = {0, 0};
struct io_nfs_stats *st_ionfs[2];
struct io_hdr_stats *st_hdr_ionfs;

int ionfs_nr = 0;	/* Nb of NFS mounted directories found */
int cpu_nr = 0;		/* Nb of processors on the machine */
int flags = 0;		/* Flag for common options and system state */

long interval = 0;
char timestamp[64];


/*
 ***************************************************************************
 * Print usage and exit.
 *
 * IN:
 * @progname	Name of sysstat command.
 ***************************************************************************
 */
void usage(char *progname)
{
	fprintf(stderr, _("Usage: %s [ options ] [ <interval> [ <count> ] ]\n"),
		progname);

#ifdef DEBUG
	fprintf(stderr, _("Options are:\n"
			  "[ --debuginfo ] [ -h ] [ -k | -m ] [ -t ] [ -V ]\n"));
#else
	fprintf(stderr, _("Options are:\n"
			  "[ -h ] [ -k | -m ] [ -t ] [ -V ]\n"));
#endif
	exit(1);
}

/*
 ***************************************************************************
 * Set output unit. Unit will be kB/s unless POSIXLY_CORRECT
 * environment variable has been set, in which case the output will be
 * expressed in blocks/s.
 ***************************************************************************
 */
void set_output_unit(void)
{
	char *e;

	if (DISPLAY_KILOBYTES(flags) || DISPLAY_MEGABYTES(flags))
		return;

	/* Check POSIXLY_CORRECT environment variable */
	if ((e = getenv(ENV_POSIXLY_CORRECT)) == NULL) {
		/* Variable not set: Unit is kB/s and not blocks/s */
		flags |= I_D_KILOBYTES;
	}
}

/*
 ***************************************************************************
 * SIGALRM signal handler.
 *
 * IN:
 * @sig	Signal number. Set to 0 for the first time, then to SIGALRM.
 ***************************************************************************
 */
void alarm_handler(int sig)
{
	signal(SIGALRM, alarm_handler);
	alarm(interval);
}

/*
 ***************************************************************************
 * Find number of NFS-mounted points that are registered in
 * /proc/self/mountstats.
 *
 * RETURNS:
 * Number of NFS-mounted points.
 ***************************************************************************
 */
int get_nfs_mount_nr(void)
{
	FILE *fp;
	char line[8192];
	char type_name[10];
	unsigned int nfs = 0;

	if ((fp = fopen(NFSMOUNTSTATS, "r")) == NULL)
		/* File non-existent */
		return 0;

	while (fgets(line, 8192, fp) != NULL) {

		if ((strstr(line, "mounted")) && (strstr(line, "on")) &&
		    (strstr(line, "with")) && (strstr(line, "fstype"))) {

			sscanf(strstr(line, "fstype") + 6, "%9s", type_name);
			if ((!strncmp(type_name, "nfs", 3)) && (strncmp(type_name, "nfsd", 4))) {
				nfs ++;
			}
		}
	}

	fclose(fp);

	return nfs;
}

/*
 ***************************************************************************
 * Set every nfs_io entry to inactive state (unregistered).
 ***************************************************************************
 */
void set_entries_inactive(void)
{
	int i;
	struct io_hdr_stats *shi = st_hdr_ionfs;

	for (i = 0; i < ionfs_nr; i++, shi++) {
		shi->active = FALSE;
	}
}

/*
 ***************************************************************************
 * Free inactive entries (mark them as unused).
 ***************************************************************************
 */
void free_inactive_entries(void)
{
	int i;
	struct io_hdr_stats *shi = st_hdr_ionfs;

	for (i = 0; i < ionfs_nr; i++, shi++) {
		if (!shi->active) {
			shi->used = FALSE;
		}
	}
}

/*
 ***************************************************************************
 * Allocate and init structures, according to system state.
 ***************************************************************************
 */
void io_sys_init(void)
{
	int i;
	
	/* How many processors on this machine? */
	cpu_nr = get_cpu_nr(~0);

	/* Get number of NFS directories in /proc/self/mountstats */
	if ((ionfs_nr = get_nfs_mount_nr()) > 0) {
		ionfs_nr += NR_NFS_PREALLOC;
	}
	if ((st_hdr_ionfs = (struct io_hdr_stats *) calloc(ionfs_nr, IO_HDR_STATS_SIZE)) == NULL) {
		perror("malloc");
		exit(4);
	}
	
	/* Allocate structures for number of NFS directories found */
	for (i = 0; i < 2; i++) {
		if ((st_ionfs[i] =
		    (struct io_nfs_stats *) calloc(ionfs_nr, IO_NFS_STATS_SIZE)) == NULL) {
			perror("malloc");
			exit(4);
		}
	}
}

/*
 ***************************************************************************
 * Free various structures.
 ***************************************************************************
*/
void io_sys_free(void)
{
	int i;

	/* Free I/O NFS directories structures */
	for (i = 0; i < 2; i++) {

		if (st_ionfs[i]) {
			free(st_ionfs[i]);
		}
	}
	
	if (st_hdr_ionfs) {
		free(st_hdr_ionfs);
	}
}

/*
 ***************************************************************************
 * Save stats for current NFS filesystem.
 *
 * IN:
 * @name		Name of NFS filesystem.
 * @curr		Index in array for current sample statistics.
 * @st_io		Structure with NFS statistics to save.
 * @ionfs_nr		Number of NFS filesystems.
 * @st_hdr_ionfs	Pointer on structures describing an NFS filesystem.
 *
 * OUT:
 * @st_hdr_ionfs	Pointer on structures describing an NFS filesystem.
 ***************************************************************************
 */
void save_stats(char *name, int curr, void *st_io)
{
	int i, j;
	struct io_hdr_stats *st_hdr_ionfs_i;
	struct io_nfs_stats *st_ionfs_i;

	/* Look for NFS directory in data table */
	for (i = 0; i < ionfs_nr; i++) {
		st_hdr_ionfs_i = st_hdr_ionfs + i;
		if ((st_hdr_ionfs_i->used) &&
		    (!strcmp(st_hdr_ionfs_i->name, name))) {
			break;
		}
	}

	if (i == ionfs_nr) {
		/*
		 * This is a new filesystem: Look for an unused entry to store it.
		 */
		for (i = 0; i < ionfs_nr; i++) {
			st_hdr_ionfs_i = st_hdr_ionfs + i;
			if (!st_hdr_ionfs_i->used) {
				/* Unused entry found... */
				st_hdr_ionfs_i->used = TRUE; /* Indicate it is now used */
				st_hdr_ionfs_i->active = TRUE;

				strcpy(st_hdr_ionfs_i->name, name);
				st_ionfs_i = st_ionfs[curr] + i;
				memset(st_ionfs_i, 0, IO_NFS_STATS_SIZE);
				*st_ionfs_i = *((struct io_nfs_stats *) st_io);
				break;
			}
		}
		if (i == ionfs_nr) {
			/* All entries are used: The number has to be increased */
			ionfs_nr = ionfs_nr + 5;

			/* Increase the size of st_hdr_ionfs buffer */
			if ((st_hdr_ionfs = (struct io_hdr_stats *)
				realloc(st_hdr_ionfs, ionfs_nr * IO_HDR_STATS_SIZE)) == NULL) {
				perror("malloc");
				exit(4);
			}

			/* Set the new entries inactive */
			for (j = 0; j < 5; j++) {
				st_hdr_ionfs_i = st_hdr_ionfs + i + j;
				st_hdr_ionfs_i->used = FALSE;
				st_hdr_ionfs_i->active = FALSE;
			}

			/* Increase the size of st_hdr_ionfs buffer */
			for (j = 0; j < 2; j++) {
				if ((st_ionfs[j] = (struct io_nfs_stats *)
					realloc(st_ionfs[j], ionfs_nr * IO_NFS_STATS_SIZE)) == NULL) {
					perror("malloc");
					exit(4);
				}
				memset(st_ionfs[j] + i, 0, 5 * IO_NFS_STATS_SIZE);
			}

			/* Now i shows the first unused entry of the new block */
			st_hdr_ionfs_i = st_hdr_ionfs + i;
			st_hdr_ionfs_i->used = TRUE; /* Indicate it is now used */
			strcpy(st_hdr_ionfs_i->name, name);
			st_ionfs_i = st_ionfs[curr] + i;
			memset(st_ionfs_i, 0, IO_NFS_STATS_SIZE);
		}
	} else  {
		st_hdr_ionfs_i = st_hdr_ionfs + i;
		st_hdr_ionfs_i->used = TRUE;
		st_hdr_ionfs_i->active = TRUE;
		st_ionfs_i = st_ionfs[curr] + i;
		*st_ionfs_i = *((struct io_nfs_stats *) st_io);
	}
	/*
	 * else it was a new NFS directory
	 * but there was no free structure to store it.
	 */
}

/*
 ***************************************************************************
 * Read NFS-mount directories stats from /proc/self/mountstats.
 *
 * IN:
 * @curr	Index in array for current sample statistics.
 ***************************************************************************
 */
void read_nfs_stat(int curr)
{
	FILE *fp;
	int sw = 0;
	char line[256];
	char *xprt_line;
	char *mount_part;
	char nfs_name[MAX_NAME_LEN];
	char mount[10], on[10], prefix[10], aux[32];
	char operation[16];
	struct io_nfs_stats snfs;
	long int v1;

	/* Every I/O NFS entry is potentially unregistered */
	set_entries_inactive();

	if ((fp = fopen(NFSMOUNTSTATS, "r")) == NULL)
		return;

	sprintf(aux, "%%%ds",
		MAX_NAME_LEN < 200 ? MAX_NAME_LEN-1 : 200);

	while (fgets(line, 256, fp) != NULL) {
		/* Read NFS directory name */
		if (!strncmp(line, "device", 6)) {
			sw = 0;
			sscanf(line + 6, aux, nfs_name);
			mount_part = strchr(line + 7, ' ');
			if (mount_part != NULL) {
				sscanf(mount_part, "%9s %9s", mount, on);
				if ((!strncmp(mount, "mounted", 7)) && (!strncmp(on, "on", 2))) {
					sw = 1;
				}
			}
		}

		sscanf(line, "%9s", prefix);
		if (sw && (!strncmp(prefix, "bytes:", 6))) {
			/* Read the stats for the last NFS-mounted directory */
			sscanf(strstr(line, "bytes:") + 6, "%llu %llu %llu %llu %llu %llu",
			       &snfs.rd_normal_bytes, &snfs.wr_normal_bytes,
			       &snfs.rd_direct_bytes, &snfs.wr_direct_bytes,
			       &snfs.rd_server_bytes, &snfs.wr_server_bytes);
			sw = 2;
		}

		if ((sw == 2) && (!strncmp(prefix, "xprt:", 5))) {
			/*
			 * Read extended statistic for the last NFS-mounted directory
			 * - number of sent rpc requests.
			 */
			xprt_line = (strstr(line, "xprt:") + 6);
			/* udp, tcp or rdma data */
			if (!strncmp(xprt_line, "udp", 3)) {
				/* port bind_count sends recvs (bad_xids req_u bklog_u) */
				sscanf(strstr(xprt_line, "udp") + 4, "%*u %*u %lu",
				       &snfs.rpc_sends);
			}
			if (!strncmp(xprt_line, "tcp", 3)) {
				/*
				 * port bind_counter connect_count connect_time idle_time
				 * sends recvs (bad_xids req_u bklog_u)
				 */
				sscanf(strstr(xprt_line, "tcp") + 4,
				       "%*u %*u %*u %*u %*d %lu",
				       &snfs.rpc_sends);
			}
			if (!strncmp(xprt_line,"rdma", 4)) {
				/*
				 * 0(port) bind_count connect_count connect_time idle_time
				 * sends recvs (bad_xids req_u bklog_u...)
				 */
				sscanf(strstr(xprt_line, "rdma") + 5,
				       "%*u %*u %*u %*u %*d %lu",
				       &snfs.rpc_sends);
			}
			sw = 3;
		}

		if ((sw == 3) && (!strncmp(prefix, "per-op", 6))) {
			sw = 4;
			while (sw == 4) {
				fgets(line, 256, fp);
				sscanf(line, "%15s %lu", operation, &v1);
				if (!strncmp(operation, "READ:", 5)) {
					snfs.nfs_rops = v1;
				}
				else if (!strncmp(operation, "WRITE:", 6)) {
					snfs.nfs_wops = v1;

					save_stats(nfs_name, curr, &snfs);
					sw = 0;
				}
			}
		}
	}

	fclose(fp);

	/* Free structures corresponding to unregistered filesystems */
	free_inactive_entries();
}

/*
 ***************************************************************************
 * Display NFS stats header.
 *
 * OUT:
 * @fctr	Conversion factor.
 ***************************************************************************
 */
void write_nfs_stat_header(int *fctr)
{
	printf("Filesystem:           ");
	if (DISPLAY_KILOBYTES(flags)) {
		printf("    rkB_nor/s    wkB_nor/s    rkB_dir/s    wkB_dir/s"
		       "    rkB_svr/s    wkB_svr/s");
		*fctr = 1024;
	}
	else if (DISPLAY_MEGABYTES(flags)) {
		printf("    rMB_nor/s    wMB_nor/s    rMB_dir/s    wMB_dir/s"
		       "    rMB_svr/s    wMB_svr/s");
		*fctr = 1024 * 1024;
	}
	else {
		printf("   rBlk_nor/s   wBlk_nor/s   rBlk_dir/s   wBlk_dir/s"
		       "   rBlk_svr/s   wBlk_svr/s");
		*fctr = 512;
	}
	printf("     ops/s    rops/s    wops/s\n");
}

/*
 ***************************************************************************
 * Write NFS stats read from /proc/self/mountstats.
 *
 * IN:
 * @curr	Index in array for current sample statistics.
 * @itv		Interval of time.
 * @fctr	Conversion factor.
 * @shi		Structures describing the NFS filesystems.
 * @ioi		Current sample statistics.
 * @ioj		Previous sample statistics.
 ***************************************************************************
 */
void write_nfs_stat(int curr, unsigned long long itv, int fctr,
		    struct io_hdr_stats *shi, struct io_nfs_stats *ioni,
		    struct io_nfs_stats *ionj)
{
	if (DISPLAY_HUMAN_READ(flags)) {
		printf("%-22s\n%23s", shi->name, "");
	}
	else {
		printf("%-22s ", shi->name);
	}
	printf("%12.2f %12.2f %12.2f %12.2f %12.2f %12.2f %9.2f %9.2f %9.2f\n",
	       S_VALUE(ionj->rd_normal_bytes, ioni->rd_normal_bytes, itv) / fctr,
	       S_VALUE(ionj->wr_normal_bytes, ioni->wr_normal_bytes, itv) / fctr,
	       S_VALUE(ionj->rd_direct_bytes, ioni->rd_direct_bytes, itv) / fctr,
	       S_VALUE(ionj->wr_direct_bytes, ioni->wr_direct_bytes, itv) / fctr,
	       S_VALUE(ionj->rd_server_bytes, ioni->rd_server_bytes, itv) / fctr,
	       S_VALUE(ionj->wr_server_bytes, ioni->wr_server_bytes, itv) / fctr,
	       S_VALUE(ionj->rpc_sends, ioni->rpc_sends, itv),
	       S_VALUE(ionj->nfs_rops,  ioni->nfs_rops,  itv),
	       S_VALUE(ionj->nfs_wops,  ioni->nfs_wops,  itv));
}

/*
 ***************************************************************************
 * Print everything now (stats and uptime).
 *
 * IN:
 * @curr	Index in array for current sample statistics.
 * @rectime	Current date and time.
 ***************************************************************************
 */
void write_stats(int curr, struct tm *rectime)
{
	int i, fctr = 1;
	unsigned long long itv;
	struct io_hdr_stats *shi;
	struct io_nfs_stats *ioni, *ionj;

	/* Test stdout */
	TEST_STDOUT(STDOUT_FILENO);

	/* Print time stamp */
	if (DISPLAY_TIMESTAMP(flags)) {
		if (DISPLAY_ISO(flags)) {
			strftime(timestamp, sizeof(timestamp), "%FT%T%z", rectime);
		}
		else {
			strftime(timestamp, sizeof(timestamp), "%x %X", rectime);
		}
		printf("%s\n", timestamp);
#ifdef DEBUG
		if (DISPLAY_DEBUG(flags)) {
			fprintf(stderr, "%s\n", timestamp);
		}
#endif
	}

	/* Interval is multiplied by the number of processors */
	itv = get_interval(uptime[!curr], uptime[curr]);

	if (cpu_nr > 1) {
		/* On SMP machines, reduce itv to one processor (see note above) */
		itv = get_interval(uptime0[!curr], uptime0[curr]);
	}

	shi = st_hdr_ionfs;

	/* Display NFS stats header */
	write_nfs_stat_header(&fctr);

	for (i = 0; i < ionfs_nr; i++, shi++) {
		if (shi->used) {
			ioni = st_ionfs[curr]  + i;
			ionj = st_ionfs[!curr] + i;
#ifdef DEBUG
			if (DISPLAY_DEBUG(flags)) {
				/* Debug output */
				fprintf(stderr, "name=%s itv=%llu fctr=%d ioni{ rd_normal_bytes=%llu "
						"wr_normal_bytes=%llu rd_direct_bytes=%llu wr_direct_bytes=%llu rd_server_bytes=%llu "
						"wr_server_bytes=%llu rpc_sends=%lu nfs_rops=%lu nfs_wops=%lu }\n",
					shi->name, itv, fctr,
					ioni->rd_normal_bytes, ioni->wr_normal_bytes,
					ioni->rd_direct_bytes, ioni->wr_direct_bytes,
					ioni->rd_server_bytes, ioni->wr_server_bytes,
					ioni->rpc_sends,
					ioni->nfs_rops,        ioni->nfs_wops);
			}
#endif
			write_nfs_stat(curr, itv, fctr, shi, ioni, ionj);
		}
	}
	printf("\n");
}

/*
 ***************************************************************************
 * Main loop: Read stats from the relevant sources and display them.
 *
 * IN:
 * @count	Number of lines of stats to print.
 * @rectime	Current date and time.
 ***************************************************************************
 */
void rw_io_stat_loop(long int count, struct tm *rectime)
{
	int curr = 1;

	/* Don't buffer data if redirected to a pipe */
	setbuf(stdout, NULL);
	
	do {
		if (cpu_nr > 1) {
			/*
			 * Read system uptime (only for SMP machines).
			 * Init uptime0. So if /proc/uptime cannot fill it,
			 * this will be done by /proc/stat.
			 */
			uptime0[curr] = 0;
			read_uptime(&(uptime0[curr]));
		}
		/* Read NFS directories stats */
		read_nfs_stat(curr);

		/* Get time */
		get_localtime(rectime);

		/* Print results */
		write_stats(curr, rectime);

		if (count > 0) {
			count--;
		}
		if (count) {
			curr ^= 1;
			pause();
		}
	}
	while (count);
}

/*
 ***************************************************************************
 * Main entry to the nfsiostat program.
 ***************************************************************************
 */
int main(int argc, char **argv)
{
	int it = 0;
	int opt = 1;
	int i;
	long count = 1;
	struct utsname header;
	struct tm rectime;

#ifdef USE_NLS
	/* Init National Language Support */
	init_nls();
#endif

	/* Get HZ */
	get_HZ();

	/* Process args... */
	while (opt < argc) {

#ifdef DEBUG
		if (!strcmp(argv[opt], "--debuginfo")) {
			flags |= I_D_DEBUG;
			opt++;
		} else
#endif
		if (!strncmp(argv[opt], "-", 1)) {
			for (i = 1; *(argv[opt] + i); i++) {

				switch (*(argv[opt] + i)) {

				case 'h':
					/* Display an easy-to-read NFS report */
					flags |= I_D_HUMAN_READ;
					break;
	
				case 'k':
					if (DISPLAY_MEGABYTES(flags)) {
						usage(argv[0]);
					}
					/* Display stats in kB/s */
					flags |= I_D_KILOBYTES;
					break;

				case 'm':
					if (DISPLAY_KILOBYTES(flags)) {
						usage(argv[0]);
					}
					/* Display stats in MB/s */
					flags |= I_D_MEGABYTES;
					break;

				case 't':
					/* Display timestamp */
					flags |= I_D_TIMESTAMP;
					break;

				case 'V':
					/* Print version number and exit */
					print_version();
					break;
	
				default:
					usage(argv[0]);
				}
			}
			opt++;
		}

		else if (!it) {
			interval = atol(argv[opt++]);
			if (interval < 0) {
				usage(argv[0]);
			}
			count = -1;
			it = 1;
		}

		else if (it > 0) {
			count = atol(argv[opt++]);
			if ((count < 1) || !interval) {
				usage(argv[0]);
			}
			it = -1;
		}
		else {
			usage(argv[0]);
		}
	}

	if (!interval) {
		count = 1;
	}

	/* Select output unit (kB/s or blocks/s) */
	set_output_unit();

	/* Init structures according to machine architecture */
	io_sys_init();

	get_localtime(&rectime);

	/* Get system name, release number and hostname */
	uname(&header);
	if (print_gal_header(&rectime, header.sysname, header.release,
			     header.nodename, header.machine, cpu_nr)) {
		flags |= I_D_ISO;
	}
	printf("\n");

	/* Set a handler for SIGALRM */
	alarm_handler(0);

	/* Main loop */
	rw_io_stat_loop(count, &rectime);

	/* Free structures */
	io_sys_free();

	return 0;
}
