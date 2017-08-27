/*
 * cifsiostat: Report I/O statistics for CIFS filesystems.
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
#include <ctype.h>

#include "version.h"
#include "cifsiostat.h"
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
struct cifs_stats *st_cifs[2];
struct io_hdr_stats *st_hdr_cifs;

int cifs_nr = 0;	/* Nb of CIFS mounted directories found */
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
 * Find number of CIFS-mounted points that are registered in
 * /proc/fs/cifs/Stats.
 *
 * RETURNS:
 * Number of CIFS-mounted points.
 ***************************************************************************
 */
int get_cifs_nr(void)
{
	FILE *fp;
	char line[128];
	int cifs = 0;

	if ((fp = fopen(CIFSSTATS, "r")) == NULL)
		/* File non-existent */
		return 0;

	while (fgets(line, 128, fp) != NULL) {
		
		if (!strncmp(line, "Share (unique mount targets): ", 30)) {
			sscanf(line + 30, "%d", &cifs);
			break;
		}
	}

	/* Close file */
	fclose(fp);

	return cifs;
}

/*
 ***************************************************************************
 * Set every cifs_io entry to inactive state (unregistered).
 ***************************************************************************
 */
void set_entries_inactive(void)
{
	int i;
	struct io_hdr_stats *shi = st_hdr_cifs;

	for (i = 0; i < cifs_nr; i++, shi++) {
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
	struct io_hdr_stats *shi = st_hdr_cifs;

	for (i = 0; i < cifs_nr; i++, shi++) {
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

	/* Get number of CIFS directories in /proc/fs/cifs/Stats */
	if ((cifs_nr = get_cifs_nr()) > 0) {
		cifs_nr += NR_CIFS_PREALLOC;
	}
	if ((st_hdr_cifs = (struct io_hdr_stats *) calloc(cifs_nr, IO_HDR_STATS_SIZE)) == NULL) {
		perror("malloc");
		exit(4);
	}
	
	/* Allocate structures for number of CIFS directories found */
	for (i = 0; i < 2; i++) {
		if ((st_cifs[i] =
		    (struct cifs_stats *) calloc(cifs_nr, CIFS_STATS_SIZE)) == NULL) {
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

	/* Free CIFS directories structures */
	for (i = 0; i < 2; i++) {

		if (st_cifs[i]) {
			free(st_cifs[i]);
		}
	}
	
	if (st_hdr_cifs) {
		free(st_hdr_cifs);
	}
}

/*
 ***************************************************************************
 * Save stats for current CIFS filesystem.
 *
 * IN:
 * @name		Name of CIFS filesystem.
 * @curr		Index in array for current sample statistics.
 * @st_io		Structure with CIFS statistics to save.
 ***************************************************************************
 */
void save_stats(char *name, int curr, struct cifs_stats *st_io)
{
	int i, j;
	struct io_hdr_stats *st_hdr_cifs_i;
	struct cifs_stats *st_cifs_i;

	/* Look for CIFS directory in data table */
	for (i = 0; i < cifs_nr; i++) {
		st_hdr_cifs_i = st_hdr_cifs + i;
		if ((st_hdr_cifs_i->used == TRUE) &&
		    (!strcmp(st_hdr_cifs_i->name, name))) {
			break;
		}
	}

	if (i == cifs_nr) {
		/*
		 * This is a new filesystem: Look for an unused entry to store it.
		 */
		for (i = 0; i < cifs_nr; i++) {
			st_hdr_cifs_i = st_hdr_cifs + i;
			if (!st_hdr_cifs_i->used) {
				/* Unused entry found... */
				st_hdr_cifs_i->used = TRUE; /* Indicate it is now used */
				st_hdr_cifs_i->active = TRUE;
				strcpy(st_hdr_cifs_i->name, name);
				st_cifs_i = st_cifs[curr] + i;
				*st_cifs_i = *((struct cifs_stats *) st_io);
				break;
			}
		}
		if (i == cifs_nr) {
			/*
			 * It is a new CIFS directory
			 * but there is no free structure to store it.
			 */

			/* All entries are used: The number has to be increased */
			cifs_nr = cifs_nr + 5;

			/* Increase the size of st_hdr_ionfs buffer */
			if ((st_hdr_cifs = (struct io_hdr_stats *)
				realloc(st_hdr_cifs, cifs_nr * IO_HDR_STATS_SIZE)) == NULL) {
				perror("malloc");
				exit(4);
			}

			/* Set the new entries inactive */
			for (j = 0; j < 5; j++) {
				st_hdr_cifs_i = st_hdr_cifs + i + j;
				st_hdr_cifs_i->used = FALSE;
				st_hdr_cifs_i->active = FALSE;
			}

			/* Increase the size of st_hdr_ionfs buffer */
			for (j = 0; j < 2; j++) {
				if ((st_cifs[j] = (struct cifs_stats *)
					realloc(st_cifs[j], cifs_nr * CIFS_STATS_SIZE)) == NULL) {
					perror("malloc");
					exit(4);
				}
				memset(st_cifs[j] + i, 0, 5 * CIFS_STATS_SIZE);
			}
			/* Now i shows the first unused entry of the new block */
			st_hdr_cifs_i = st_hdr_cifs + i;
			st_hdr_cifs_i->used = TRUE; /* Indicate it is now used */
			st_hdr_cifs_i->active = TRUE;
			strcpy(st_hdr_cifs_i->name, name);
			st_cifs_i = st_cifs[curr] + i;
			*st_cifs_i = *st_io;
		}
	} else {
		st_hdr_cifs_i = st_hdr_cifs + i;
		st_hdr_cifs_i->active = TRUE;
		st_hdr_cifs_i->used = TRUE;
		st_cifs_i = st_cifs[curr] + i;
		*st_cifs_i = *st_io;
	}
	/*
	 * else it was a new CIFS directory
	 * but there was no free structure to store it.
	 */
}

/*
 ***************************************************************************
 * Read CIFS-mount directories stats from /proc/fs/cifs/Stats.
 *
 * IN:
 * @curr	Index in array for current sample statistics.
 ***************************************************************************
 */
void read_cifs_stat(int curr)
{
	FILE *fp;
	char line[256];
	char aux[32];
	int start = 0;
	long long unsigned aux_open;
	long long unsigned all_open = 0;
	char cifs_name[MAX_NAME_LEN];
	char name_tmp[MAX_NAME_LEN];
	struct cifs_stats scifs;

	/* Every CIFS entry is potentially unregistered */
	set_entries_inactive();

	if ((fp = fopen(CIFSSTATS, "r")) == NULL)
		return;

	sprintf(aux, "%%*d) %%%ds",
		MAX_NAME_LEN < 200 ? MAX_NAME_LEN - 1 : 200);

	while (fgets(line, 256, fp) != NULL) {

		/* Read CIFS directory name */
		if (isdigit((unsigned char) line[0]) && sscanf(line, aux , name_tmp) == 1) {
			if (start) {
				scifs.fopens = all_open;
				save_stats(cifs_name, curr, &scifs);
				all_open = 0;
			}
			else {
				start = 1;
			}
			strcpy(cifs_name, name_tmp);
		}
		else {
			if (!strncmp(line, "Reads:", 6)) {
				sscanf(line, "Reads: %llu Bytes: %llu", &scifs.rd_ops, &scifs.rd_bytes);
			}
			if (!strncmp(line, "Writes:", 7)) {
				sscanf(line, "Writes: %llu Bytes: %llu", &scifs.wr_ops, &scifs.wr_bytes);
			}
			if (!strncmp(line, "Opens:", 6)) {
				sscanf(line, "Opens: %llu Closes:%llu Deletes: %llu",
				       &aux_open, &scifs.fcloses, &scifs.fdeletes);
				all_open += aux_open;
			}
			if (!strncmp(line, "Posix Opens:", 12)) {
				sscanf(line, "Posix Opens: %llu", &aux_open);
				all_open += aux_open;
			}
		}
	}
	
	if (start) {
		scifs.fopens = all_open;
		save_stats(cifs_name, curr, &scifs);
	}

	fclose(fp);

	/* Free structures corresponding to unregistered filesystems */
	free_inactive_entries();
}

/*
 ***************************************************************************
 * Display CIFS stats header.
 *
 * OUT:
 * @fctr	Conversion factor.
 ***************************************************************************
 */
void write_cifs_stat_header(int *fctr)
{
	printf("Filesystem:           ");
	if (DISPLAY_KILOBYTES(flags)) {
		printf("        rkB/s        wkB/s");
		*fctr = 1024;
	}
	else if (DISPLAY_MEGABYTES(flags)) {
		printf("        rMB/s        wMB/s");
		*fctr = 1024 * 1024;
	}
	else {
		printf("         rB/s         wB/s");
		*fctr = 1;
	}
	printf("    rops/s    wops/s         fo/s         fc/s         fd/s\n");
}

/*
 ***************************************************************************
 * Write CIFS stats read from /proc/fs/cifs/Stats.
 *
 * IN:
 * @curr	Index in array for current sample statistics.
 * @itv		Interval of time.
 * @fctr	Conversion factor.
 * @shi		Structures describing the CIFS filesystems.
 * @ioi		Current sample statistics.
 * @ioj		Previous sample statistics.
 ***************************************************************************
 */
void write_cifs_stat(int curr, unsigned long long itv, int fctr,
		     struct io_hdr_stats *shi, struct cifs_stats *ioni,
		     struct cifs_stats *ionj)
{
	if (DISPLAY_HUMAN_READ(flags)) {
		printf("%-22s\n%23s", shi->name, "");
	}
	else {
		printf("%-22s ", shi->name);
	}

	/*       rB/s   wB/s   fo/s   fc/s   fd/s*/
	printf("%12.2f %12.2f %9.2f %9.2f %12.2f %12.2f %12.2f \n",
	       S_VALUE(ionj->rd_bytes, ioni->rd_bytes, itv) / fctr,
	       S_VALUE(ionj->wr_bytes, ioni->wr_bytes, itv) / fctr,
	       S_VALUE(ionj->rd_ops, ioni->rd_ops, itv),
	       S_VALUE(ionj->wr_ops, ioni->wr_ops, itv),
	       S_VALUE(ionj->fopens, ioni->fopens, itv),
	       S_VALUE(ionj->fcloses, ioni->fcloses, itv),
	       S_VALUE(ionj->fdeletes, ioni->fdeletes, itv));
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
	struct cifs_stats *ioni, *ionj;

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

	shi = st_hdr_cifs;

	/* Display CIFS stats header */
	write_cifs_stat_header(&fctr);

	for (i = 0; i < cifs_nr; i++, shi++) {
		if (shi->used) {
			ioni = st_cifs[curr]  + i;
			ionj = st_cifs[!curr] + i;
#ifdef DEBUG
			if (DISPLAY_DEBUG(flags)) {
				/* Debug output */
				fprintf(stderr, "name=%s itv=%llu fctr=%d ioni{ rd_bytes=%llu "
						"wr_bytes=%llu rd_ops=%llu wr_ops=%llu fopens=%llu "
						"fcloses=%llu fdeletes=%llu}\n",
					shi->name, itv, fctr,
					ioni->rd_bytes, ioni->wr_bytes,
					ioni->rd_ops,   ioni->wr_ops,
					ioni->fopens,   ioni->fcloses,
					ioni->fdeletes);
			}
#endif
			write_cifs_stat(curr, itv, fctr, shi, ioni, ionj);
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

		/* Read CIFS stats */
		read_cifs_stat(curr);

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
 * Main entry to the cifsiostat program.
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
					/* Display an easy-to-read CIFS report */
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
