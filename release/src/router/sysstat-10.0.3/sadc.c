/*
 * sadc: system activity data collector
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
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>
#include <signal.h>
#include <dirent.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/utsname.h>

#include "version.h"
#include "sa.h"
#include "rd_stats.h"
#include "common.h"
#include "ioconf.h"

#ifdef USE_NLS
#include <locale.h>
#include <libintl.h>
#define _(string) gettext(string)
#else
#define _(string) (string)
#endif

#ifdef HAVE_SENSORS
#include "sensors/sensors.h"
#include "sensors/error.h"
#endif

#define SCCSID "@(#)sysstat-" VERSION ": " __FILE__ " compiled " __DATE__ " " __TIME__
char *sccsid(void) { return (SCCSID); }

long interval = 0;
unsigned int flags = 0;

int dis;
char timestamp[2][TIMESTAMP_LEN];

struct file_header file_hdr;
struct record_header record_hdr;
char comment[MAX_COMMENT_LEN];
unsigned int id_seq[NR_ACT];

extern struct activity *act[];

/*
 ***************************************************************************
 * Print usage and exit.
 *
 * IN:
 * @progname	Name of sysstat command
 ***************************************************************************
 */
void usage(char *progname)
{
	fprintf(stderr, _("Usage: %s [ options ] [ <interval> [ <count> ] ] [ <outfile> ]\n"),
		progname);

	fprintf(stderr, _("Options are:\n"
			  "[ -C <comment> ] [ -F ] [ -L ] [ -V ]\n"
			  "[ -S { INT | DISK | IPV6 | POWER | SNMP | XDISK | ALL | XALL } ]\n"));
	exit(1);
}

/*
 ***************************************************************************
 * Collect all activities belonging to a group.
 *
 * IN:
 * @group_id	Group identification number.
 * @opt_f	Optionnal flag to set.
 ***************************************************************************
 */
void collect_group_activities(unsigned int group_id, unsigned int opt_f)
{
	int i;

	for (i = 0; i < NR_ACT; i++) {
		if (act[i]->group & group_id) {
			act[i]->options |= AO_COLLECTED;
			if (opt_f) {
				act[i]->opt_flags |= opt_f;
			}
		}
	}
}

/*
 ***************************************************************************
 * Parse option -S, indicating which activities are to be collected.
 *
 * IN:
 * @argv	Arguments list.
 * @opt		Index in list of arguments.
 ***************************************************************************
 */
void parse_sadc_S_option(char *argv[], int opt)
{
	char *p;
	int i;

	for (p = strtok(argv[opt], ","); p; p = strtok(NULL, ",")) {
		if (!strcmp(p, K_INT)) {
			/* Select group of interrupt activities */
			collect_group_activities(G_INT, AO_F_NULL);
		}
		else if (!strcmp(p, K_DISK)) {
			/* Select group of disk activities */
			collect_group_activities(G_DISK, AO_F_NULL);
		}
		else if (!strcmp(p, K_XDISK)) {
			/* Select group of disk and partition activities */
			collect_group_activities(G_DISK, AO_F_DISK_PART);
		}
		else if (!strcmp(p, K_SNMP)) {
			/* Select group of SNMP activities */
			collect_group_activities(G_SNMP, AO_F_NULL);
		}
		else if (!strcmp(p, K_IPV6)) {
			/* Select group of IPv6 activities */
			collect_group_activities(G_IPV6, AO_F_NULL);
		}
		else if (!strcmp(p, K_POWER)) {
			/* Select group of activities related to power management */
			collect_group_activities(G_POWER, AO_F_NULL);
		}
		else if (!strcmp(p, K_ALL) || !strcmp(p, K_XALL)) {
			/* Select all activities */
			for (i = 0; i < NR_ACT; i++) {
				act[i]->options |= AO_COLLECTED;
			}
			if (!strcmp(p, K_XALL)) {
				/* Tell sadc to also collect partition statistics */
				collect_group_activities(G_DISK, AO_F_DISK_PART);
			}
		}
		else if (strspn(argv[opt], DIGITS) == strlen(argv[opt])) {
			/*
			 * Although undocumented, option -S followed by a numerical value
			 * enables the user to select each activity that should be
			 * collected. "-S 0" unselects all activities but CPU.
			 * A value greater than 255 enables the user to select groups
			 * of activities.
			 */
			int act_id;

			act_id = atoi(argv[opt]);
			if (act_id > 255) {
				act_id >>= 8;
				for (i = 0; i < NR_ACT; i++) {
					if (act[i]->group & act_id) {
						act[i]->options |= AO_COLLECTED;
					}
				}
			}
			else if ((act_id < 0) || (act_id > NR_ACT)) {
				usage(argv[0]);
			}
			else if (!act_id) {
				/* Unselect all activities but CPU */
				for (i = 0; i < NR_ACT; i++) {
					act[i]->options &= ~AO_COLLECTED;
				}
				COLLECT_ACTIVITY(A_CPU);
			}
			else {
				/* Select chosen activity */
				COLLECT_ACTIVITY(act_id);
			}
		}
		else {
			usage(argv[0]);
		}
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
 * Display an error message.
 ***************************************************************************
 */
void p_write_error(void)
{
	fprintf(stderr, _("Cannot write data to system activity file: %s\n"),
		strerror(errno));
	exit(2);
}

/*
 ***************************************************************************
 * Init structures. All of them are init'ed first when they are allocated
 * (done by SREALLOC() macro in sa_sys_init() function).
 * Then, they are init'ed again each time before reading the various system
 * stats to make sure that no stats from a previous reading will remain (eg.
 * if some network interfaces or block devices have been unregistered).
 ***************************************************************************
 */
void reset_stats(void)
{
	int i;

	for (i = 0; i < NR_ACT; i++) {
		if ((act[i]->nr > 0) && act[i]->_buf0) {
			memset(act[i]->_buf0, 0, act[i]->msize * act[i]->nr * act[i]->nr2);
		}
	}
}

/*
 ***************************************************************************
 * Allocate and init structures, according to system state.
 ***************************************************************************
 */
void sa_sys_init(void)
{
	int i;

	for (i = 0; i < NR_ACT; i++) {

		if (act[i]->f_count) {
			/* Number of items is not a constant and should be calculated */
			act[i]->nr = (*act[i]->f_count)(act[i]);
		}

		if (act[i]->nr > 0) {
			if (act[i]->f_count2) {
				act[i]->nr2 = (*act[i]->f_count2)(act[i]);
			}
			/* else act[i]->nr2 is a constant and doesn't need to be calculated */
			
			if (!act[i]->nr2) {
				act[i]->nr = 0;
			}
		}

		if (act[i]->nr > 0) {
			/* Allocate structures for current activity */
			SREALLOC(act[i]->_buf0, void, act[i]->msize * act[i]->nr * act[i]->nr2);
		}
		else {
			/* No items found: Invalidate current activity */
			act[i]->options &= ~AO_COLLECTED;
		}

		/* Set default activity list */
		id_seq[i] = act[i]->id;
	}
}

/*
 ***************************************************************************
 * Free structures.
 ***************************************************************************
 */
void sa_sys_free(void)
{
	int i;

	for (i = 0; i < NR_ACT; i++) {

		if (act[i]->nr > 0) {
			if (act[i]->_buf0) {
				free(act[i]->_buf0);
				act[i]->_buf0 = NULL;
			}
		}
	}
}

/*
 ***************************************************************************
 * Write data to file. If the write() call was interrupted by a signal, try
 * again so that the whole buffer can be written.
 *
 * IN:
 * @fd		Output file descriptor.
 * @buf		Data buffer.
 * @nr_bytes	Number of bytes to write.
 *
 * RETURNS:
 * Number of bytes written to file, or -1 on error.
 ***************************************************************************
 */
int write_all(int fd, const void *buf, int nr_bytes)
{
	int block, offset = 0;
	char *buffer = (char *) buf;

	while (nr_bytes > 0) {
		block = write(fd, &buffer[offset], nr_bytes);

		if (block < 0) {
			if (errno == EINTR)
				continue;
			return block;
		}
		if (block == 0)
			return offset;

		offset += block;
		nr_bytes -= block;
	}

	return offset;
}

/*
 ***************************************************************************
 * If -L option used, request a non-blocking, exclusive lock on the file.
 * If lock would block, then another process (possibly sadc) has already
 * opened that file => exit.
 *
 * IN:
 * @fd		Output file descriptor.
 * @fatal	Indicate if failing to lock file should be fatal or not.
 * 		If it's not fatal then we'll wait for next iteration and
 * 		try again.
 *
 * RETURNS:
 * 0 on success, or 1 if file couldn't be locked.
 ***************************************************************************
 */
int ask_for_flock(int fd, int fatal)
{
	/* Option -L may be used only if an outfile was specified on the command line */
	if (LOCK_FILE(flags)) {
		/*
		 * Yes: Try to lock file. To make code portable, check for both EWOULDBLOCK
		 * and EAGAIN return codes, and treat them the same (glibc documentation).
		 * Indeed, some Linux ports (e.g. hppa-linux) do not equate EWOULDBLOCK and
		 * EAGAIN like every other Linux port.
		 */
		if (flock(fd, LOCK_EX | LOCK_NB) < 0) {
			if ((((errno == EWOULDBLOCK) || (errno == EAGAIN)) && (fatal == FATAL)) ||
			    ((errno != EWOULDBLOCK) && (errno != EAGAIN))) {
				perror("flock");
				exit(1);
			}
			/* Was unable to lock file: Lock would have blocked... */
			return 1;
		}
		else {
			/* File successfully locked */
			flags |= S_F_FILE_LOCKED;
		}
	}
	return 0;
}

/*
 ***************************************************************************
 * Fill system activity file magic header.
 *
 * IN:
 * @file_magic	System activity file magic header.
 ***************************************************************************
 */
void fill_magic_header(struct file_magic *file_magic)
{
	char *v;
	char version[16];

	memset(file_magic, 0, FILE_MAGIC_SIZE);

	file_magic->sysstat_magic = SYSSTAT_MAGIC;
	file_magic->format_magic  = FORMAT_MAGIC;
	file_magic->sysstat_extraversion = 0;

	strcpy(version, VERSION);

	/* Get version number */
	if ((v = strtok(version, ".")) == NULL)
		return;
	file_magic->sysstat_version = atoi(v) & 0xff;

	/* Get patchlevel number */
	if ((v = strtok(NULL, ".")) == NULL)
		return;
	file_magic->sysstat_patchlevel = atoi(v) & 0xff;

	/* Get sublevel number */
	if ((v = strtok(NULL, ".")) == NULL)
		return;
	file_magic->sysstat_sublevel = atoi(v) & 0xff;

	/* Get extraversion number. Don't necessarily exist */
	if ((v = strtok(NULL, ".")) == NULL)
		return;
	file_magic->sysstat_extraversion = atoi(v) & 0xff;
}

/*
 ***************************************************************************
 * Fill system activity file header, then write it (or print it if stdout).
 *
 * IN:
 * @fd	Output file descriptor. May be stdout.
 ***************************************************************************
 */
void setup_file_hdr(int fd)
{
	int n, i, p;
	struct tm rectime;
	struct utsname header;
	struct file_magic file_magic;
	struct file_activity file_act;

	/* Fill then write file magic header */
	fill_magic_header(&file_magic);

	if ((n = write_all(fd, &file_magic, FILE_MAGIC_SIZE)) != FILE_MAGIC_SIZE)
		goto write_error;

	/* First reset the structure */
	memset(&file_hdr, 0, FILE_HEADER_SIZE);

	/* Then get current date */
	file_hdr.sa_ust_time = get_time(&rectime);

	/* OK, now fill the header */
	file_hdr.sa_nr_act      = get_activity_nr(act, AO_COLLECTED, COUNT_ACTIVITIES);
	file_hdr.sa_day         = rectime.tm_mday;
	file_hdr.sa_month       = rectime.tm_mon;
	file_hdr.sa_year        = rectime.tm_year;
	file_hdr.sa_sizeof_long = sizeof(long);

	/* Get system name, release number, hostname and machine architecture */
	uname(&header);
	strncpy(file_hdr.sa_sysname, header.sysname, UTSNAME_LEN);
	file_hdr.sa_sysname[UTSNAME_LEN - 1]  = '\0';
	strncpy(file_hdr.sa_nodename, header.nodename, UTSNAME_LEN);
	file_hdr.sa_nodename[UTSNAME_LEN - 1] = '\0';
	strncpy(file_hdr.sa_release, header.release, UTSNAME_LEN);
	file_hdr.sa_release[UTSNAME_LEN - 1]  = '\0';
	strncpy(file_hdr.sa_machine, header.machine, UTSNAME_LEN);
	file_hdr.sa_machine[UTSNAME_LEN - 1]  = '\0';

	/* Write file header */
	if ((n = write_all(fd, &file_hdr, FILE_HEADER_SIZE)) != FILE_HEADER_SIZE)
		goto write_error;

	/* Write activity list */
	for (i = 0; i < NR_ACT; i++) {

		/*
		 * Activity sequence given by id_seq array.
		 * Sequence must be the same for stdout as for output file.
		 */
		if (!id_seq[i])
			continue;
		if ((p = get_activity_position(act, id_seq[i])) < 0)
			continue;

		if (IS_COLLECTED(act[p]->options)) {
			file_act.id    = act[p]->id;
			file_act.magic = act[p]->magic;
			file_act.nr    = act[p]->nr;
			file_act.nr2   = act[p]->nr2;
			file_act.size  = act[p]->fsize;

			if ((n = write_all(fd, &file_act, FILE_ACTIVITY_SIZE))
			    != FILE_ACTIVITY_SIZE)
				goto write_error;
		}
	}

	return;

write_error:

	fprintf(stderr, _("Cannot write system activity file header: %s\n"),
		strerror(errno));
	exit(2);
}

/*
 ***************************************************************************
 * sadc called with interval and count parameters not set:
 * Write a dummy record notifying a system restart, or insert a comment in
 * binary data file if option -C has been used.
 * Writing a dummy record should typically be done at boot time,
 * before the cron daemon is started to avoid conflict with sa1/sa2 scripts.
 *
 * IN:
 * @ofd		Output file descriptor.
 * @rtype	Record type to write (dummy or comment).
 ***************************************************************************
 */
void write_special_record(int ofd, int rtype)
{
	int n;
	struct tm rectime;

	/* Check if file is locked */
	if (!FILE_LOCKED(flags)) {
		ask_for_flock(ofd, FATAL);
	}

	/* Reset the structure (not compulsory, but a bit cleaner) */
	memset(&record_hdr, 0, RECORD_HEADER_SIZE);

	/* Set record type */
	record_hdr.record_type = rtype;

	/* Save time */
	record_hdr.ust_time = get_time(&rectime);

	record_hdr.hour   = rectime.tm_hour;
	record_hdr.minute = rectime.tm_min;
	record_hdr.second = rectime.tm_sec;

	/* Write record now */
	if ((n = write_all(ofd, &record_hdr, RECORD_HEADER_SIZE)) != RECORD_HEADER_SIZE) {
		p_write_error();
	}

	if (rtype == R_COMMENT) {
		/* Also write the comment */
		if ((n = write_all(ofd, comment, MAX_COMMENT_LEN)) != MAX_COMMENT_LEN) {
			p_write_error();
		}
	}
}

/*
 ***************************************************************************
 * Write stats (or print them if stdout).
 *
 * IN:
 * @ofd		Output file descriptor. May be stdout.
 ***************************************************************************
 */
void write_stats(int ofd)
{
	int i, n, p;

	/* Try to lock file */
	if (!FILE_LOCKED(flags)) {
		if (ask_for_flock(ofd, NON_FATAL))
			/*
			 * Unable to lock file:
			 * Wait for next iteration to try again to save data.
			 */
			return;
	}

	/* Write record header */
	if ((n = write_all(ofd, &record_hdr, RECORD_HEADER_SIZE)) != RECORD_HEADER_SIZE) {
		p_write_error();
	}

	/* Then write all statistics */
	for (i = 0; i < NR_ACT; i++) {

		if (!id_seq[i])
			continue;
		if ((p = get_activity_position(act, id_seq[i])) < 0)
			continue;

		if (IS_COLLECTED(act[p]->options)) {
			if ((n = write_all(ofd, act[p]->_buf0, act[p]->fsize * act[p]->nr * act[p]->nr2)) !=
			    (act[p]->fsize * act[p]->nr * act[p]->nr2)) {
				p_write_error();
			}
		}
	}
}

/*
 ***************************************************************************
 * Create a system activity daily data file.
 *
 * IN:
 * @ofile	Name of output file.
 *
 * OUT:
 * @ofd		Output file descriptor.
 ***************************************************************************
 */
void create_sa_file(int *ofd, char *ofile)
{
	if ((*ofd = open(ofile, O_CREAT | O_WRONLY,
			 S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) < 0) {
		fprintf(stderr, _("Cannot open %s: %s\n"), ofile, strerror(errno));
		exit(2);
	}

	/* Try to lock file */
	ask_for_flock(*ofd, FATAL);

	/* Truncate file */
	if (ftruncate(*ofd, 0) < 0) {
		fprintf(stderr, _("Cannot open %s: %s\n"), ofile, strerror(errno));
		exit(2);
	}

	/* Write file header */
	setup_file_hdr(*ofd);
}

/*
 ***************************************************************************
 * Get descriptor for stdout.
 *
 * IN:
 * @stdfd	A value >= 0 indicates that stats data should also
 *		be written to stdout.
 *
 * OUT:
 * @stdfd	Stdout file descriptor.
 ***************************************************************************
 */
void open_stdout(int *stdfd)
{
	if (*stdfd >= 0) {
		if ((*stdfd = dup(STDOUT_FILENO)) < 0) {
			perror("dup");
			exit(4);
		}
		/* Write file header on STDOUT */
		setup_file_hdr(*stdfd);
	}
}

/*
 ***************************************************************************
 * Get descriptor for output file and write its header.
 * We may enter this function several times (when we rotate a file).
 *
 * IN:
 * @ofile	Name of output file.
 *
 * OUT:
 * @ofd		Output file descriptor.
 ***************************************************************************
 */
void open_ofile(int *ofd, char ofile[])
{
	struct file_magic file_magic;
	struct file_activity file_act;
	ssize_t sz;
	int i, p;

	if (ofile[0]) {
		/* Does file exist? */
		if (access(ofile, F_OK) < 0) {
			/* NO: Create it */
			create_sa_file(ofd, ofile);
		}
		else {
			/* YES: Append data to it if possible */
			if ((*ofd = open(ofile, O_APPEND | O_RDWR)) < 0) {
				fprintf(stderr, _("Cannot open %s: %s\n"), ofile, strerror(errno));
				exit(2);
			}

			/* Read file magic header */
			sz = read(*ofd, &file_magic, FILE_MAGIC_SIZE);
			if (!sz) {
				close(*ofd);
				/* This is an empty file: Create it again */
				create_sa_file(ofd, ofile);
				return;
			}
			if ((sz != FILE_MAGIC_SIZE) ||
			    (file_magic.sysstat_magic != SYSSTAT_MAGIC) ||
			    (file_magic.format_magic != FORMAT_MAGIC)) {
				if (FORCE_FILE(flags)) {
					close(*ofd);
					/* -F option used: Truncate file */
					create_sa_file(ofd, ofile);
					return;
				}
				/* Display error message and exit */
				handle_invalid_sa_file(ofd, &file_magic, ofile, sz);
			}

			/* Read file standard header */
			if (read(*ofd, &file_hdr, FILE_HEADER_SIZE) != FILE_HEADER_SIZE) {
				/* Display error message and exit */
				handle_invalid_sa_file(ofd, &file_magic, ofile, 0);
			}

			/*
			 * OK: It's a true system activity file.
			 * List of activities from the file prevails over that of the user.
			 * So unselect all of them. And reset activity sequence.
			 */
			for (i = 0; i < NR_ACT; i++) {
				act[i]->options &= ~AO_COLLECTED;
				id_seq[i] = 0;
			}

			if (!file_hdr.sa_nr_act || (file_hdr.sa_nr_act > NR_ACT))
				/*
				 * No activities at all or at least one unknown activity:
				 * Cannot append data to such a file.
				 */
				goto append_error;

			for (i = 0; i < file_hdr.sa_nr_act; i++) {

				/* Read current activity in list */
				if (read(*ofd, &file_act, FILE_ACTIVITY_SIZE) != FILE_ACTIVITY_SIZE) {
					handle_invalid_sa_file(ofd, &file_magic, ofile, 0);
				}

				p = get_activity_position(act, file_act.id);

				if ((p < 0) || (act[p]->fsize != file_act.size) ||
				    (act[p]->magic != file_act.magic))
					/*
					 * Unknown activity in list or item size has changed or
					 * unknown activity format.
					 */
					goto append_error;

				if ((act[p]->nr != file_act.nr) || (act[p]->nr2 != file_act.nr2)) {
					if (IS_REMANENT(act[p]->options) || !file_act.nr || !file_act.nr2)
						/*
						 * Remanent structures cannot have a different number of items.
						 * Also number of items and subitems should never be null.
						 */
						goto append_error;
					else {
						/*
						 * Force number of items (serial lines, network interfaces...)
						 * and sub-items to that of the file, and reallocate structures.
						 */
						act[p]->nr  = file_act.nr;
						act[p]->nr2 = file_act.nr2;
						SREALLOC(act[p]->_buf0, void, act[p]->msize * act[p]->nr * act[p]->nr2);
					}
				}
				/* Save activity sequence */
				id_seq[i] = file_act.id;
				act[p]->options |= AO_COLLECTED;
			}
		}
	}

	return;

append_error:

	close(*ofd);
	if (FORCE_FILE(flags)) {
		/* Truncate file */
		create_sa_file(ofd, ofile);
	}
	else {
		fprintf(stderr, _("Cannot append data to that file (%s)\n"), ofile);
		exit(1);
	}
}

/*
 ***************************************************************************
 * Read statistics from various system files.
 ***************************************************************************
 */
void read_stats(void)
{
	int i;
	__nr_t cpu_nr = act[get_activity_position(act, A_CPU)]->nr;

	/*
	 * Init uptime0. So if /proc/uptime cannot fill it,
	 * this will be done by /proc/stat.
	 * If cpu_nr = 2, force /proc/stat to fill it.
	 * If cpu_nr = 1, uptime0 and uptime are equal.
	 * NB: uptime0 is always filled.
	 * Remember that cpu_nr = 1 means one CPU and no SMP kernel
	 * (one structure for CPU "all") and cpu_nr = 2 means one CPU
	 * and an SMP kernel (two structures for CPUs "all" and "0").
	 */
	record_hdr.uptime0 = 0;
	if (cpu_nr > 2) {
		read_uptime(&(record_hdr.uptime0));
	}

	for (i = 0; i < NR_ACT; i++) {
		if (IS_COLLECTED(act[i]->options)) {
			/* Read statistics for current activity */
			(*act[i]->f_read)(act[i]);
		}
	}

	if (cpu_nr == 1) {
		/*
		 * uptime has been filled by read_uptime()
		 * or when reading CPU stats from /proc/stat.
		 */
		record_hdr.uptime0 = record_hdr.uptime;
	}
}

/*
 ***************************************************************************
 * Main loop: Read stats from the relevant sources and display them.
 *
 * IN:
 * @count		Number of lines of stats to display.
 * @rectime		Current date and time.
 * @stdfd		Stdout file descriptor.
 * @ofd			Output file descriptor.
 * @ofile		Name of output file.
 ***************************************************************************
 */
void rw_sa_stat_loop(long count, struct tm *rectime, int stdfd, int ofd,
		     char ofile[])
{
	int do_sa_rotat = 0;
	unsigned int save_flags;
	char new_ofile[MAX_FILE_LEN];

	new_ofile[0] = '\0';

	/* Main loop */
	do {

		/*
		 * Init all structures.
		 * Exception for individual CPUs structures which must not be
		 * init'ed to keep values for CPU before they were disabled.
		 */
		reset_stats();

		/* Save time */
		record_hdr.ust_time = get_time(rectime);
		record_hdr.hour     = rectime->tm_hour;
		record_hdr.minute   = rectime->tm_min;
		record_hdr.second   = rectime->tm_sec;

		/* Set record type */
		if (do_sa_rotat) {
			record_hdr.record_type = R_LAST_STATS;
		}
		else {
			record_hdr.record_type = R_STATS;
		}

		/* Read then write stats */
		read_stats();

		if (stdfd >= 0) {
			save_flags = flags;
			flags &= ~S_F_LOCK_FILE;
			write_stats(stdfd);
			flags = save_flags;
		}

		/* If the record type was R_LAST_STATS, tag it R_STATS before writing it */
		record_hdr.record_type = R_STATS;
		if (ofile[0]) {
			write_stats(ofd);
		}

		if (do_sa_rotat) {
			/*
			 * Stats are written at the end of previous file *and* at the
			 * beginning of the new one (outfile must have been specified
			 * as '-' on the command line).
			 */
			do_sa_rotat = FALSE;

			if (fdatasync(ofd) < 0) {
				/* Flush previous file */
				perror("fdatasync");
				exit(4);
			}
			close(ofd);
			strcpy(ofile, new_ofile);

			/* Recalculate number of system items and reallocate structures */
			sa_sys_init();

			/*
			 * Open and init new file.
			 * This is also used to set activity sequence to that of the file
			 * if the file already exists.
			 */
			open_ofile(&ofd, ofile);

			/*
			 * Rewrite header and activity sequence to stdout since
			 * number of items may have changed.
			 */
			if (stdfd >= 0) {
				setup_file_hdr(stdfd);
			}

			/* Write stats to file again */
			write_stats(ofd);
		}

		/* Flush data */
		fflush(stdout);

		if (count > 0) {
			count--;
		}

		if (count) {
			pause();
		}

		/* Rotate activity file if necessary */
		if (WANT_SA_ROTAT(flags)) {
			/* The user specified '-' as the filename to use */
			set_default_file(rectime, new_ofile);

			if (strcmp(ofile, new_ofile)) {
				do_sa_rotat = TRUE;
			}
		}
	}
	while (count);

	/* Close file descriptors if they have actually been used */
	CLOSE(stdfd);
	CLOSE(ofd);
}

/*
 ***************************************************************************
 * Main entry to the program.
 ***************************************************************************
 */
int main(int argc, char **argv)
{
	int opt = 0, optz = 0;
	char ofile[MAX_FILE_LEN];
	struct tm rectime;
	int stdfd = 0, ofd = -1;
	long count = 0;

	/* Get HZ */
	get_HZ();
	
	/* Compute page shift in kB */
	get_kb_shift();

	ofile[0] = comment[0] = '\0';

#ifdef HAVE_SENSORS
	/* Initialize sensors, let it use the default cfg file */
	int err = sensors_init(NULL);
	if (err) {
		fprintf(stderr, "sensors_init: %s\n", sensors_strerror(err));
	}
#endif /* HAVE_SENSORS */
	
#ifdef USE_NLS
	/* Init National Language Support */
	init_nls();
#endif

	while (++opt < argc) {

		if (!strcmp(argv[opt], "-S")) {
			if (argv[++opt]) {
				parse_sadc_S_option(argv, opt);
			}
			else {
				usage(argv[0]);
			}
		}

		else if (!strcmp(argv[opt], "-F")) {
			flags |= S_F_FORCE_FILE;
		}

		else if (!strcmp(argv[opt], "-L")) {
			flags |= S_F_LOCK_FILE;
		}

		else if (!strcmp(argv[opt], "-V")) {
			print_version();
		}

		else if (!strcmp(argv[opt], "-z")) {
			/* Set by sar command */
			optz = 1;
		}

		else if (!strcmp(argv[opt], "-C")) {
			if (argv[++opt]) {
				strncpy(comment, argv[opt], MAX_COMMENT_LEN);
				comment[MAX_COMMENT_LEN - 1] = '\0';
				if (!strlen(comment)) {
					usage(argv[0]);
				}
			}
			else {
				usage(argv[0]);
			}
		}

		else if (strspn(argv[opt], DIGITS) != strlen(argv[opt])) {
			if (!ofile[0]) {
				stdfd = -1;	/* Don't write to STDOUT */
				if (!strcmp(argv[opt], "-")) {
					/* File name set to '-' */
					set_default_file(&rectime, ofile);
					flags |= S_F_SA_ROTAT;
				}
				else if (!strncmp(argv[opt], "-", 1)) {
					/* Bad option */
					usage(argv[0]);
				}
				else {
					/* Write data to file */
					strncpy(ofile, argv[opt], MAX_FILE_LEN);
					ofile[MAX_FILE_LEN - 1] = '\0';
				}
			}
			else {
				/* Outfile already specified */
				usage(argv[0]);
			}
		}

		else if (!interval) {
			/* Get interval */
			interval = atol(argv[opt]);
			if (interval < 1) {
				usage(argv[0]);
			}
			count = -1;
		}

		else if (count <= 0) {
			/* Get count value */
			count = atol(argv[opt]);
			if (count < 1) {
				usage(argv[0]);
			}
		}

		else {
			usage(argv[0]);
		}
	}

	/*
	 * If option -z used, write to STDOUT even if a filename
	 * has been entered on the command line.
	 */
	if (optz) {
		stdfd = 0;
	}

	if (!ofile[0]) {
		/* -L option ignored when writing to STDOUT */
		flags &= ~S_F_LOCK_FILE;
	}

	/* Init structures according to machine architecture */
	sa_sys_init();

	/*
	 * Open output file then STDOUT. Write header for each of them.
	 * NB: Output file must be opened first, because we may change
	 * the activities collected AND the activity sequence to that
	 * of the file, and the activities collected and activity sequence
	 * written on STDOUT must be consistent to those of the file.
	 */
	open_ofile(&ofd, ofile);
	open_stdout(&stdfd);

	if (!interval) {
		if (ofd >= 0) {
			/*
			 * Interval (and count) not set:
			 * Write a dummy record, or insert a comment, then exit.
			 * NB: Never write such a dummy record on stdout since
			 * sar never expects it.
			 */
			if (comment[0]) {
				write_special_record(ofd, R_COMMENT);
			}
			else {
				write_special_record(ofd, R_RESTART);
			}

			/* Close file descriptor */
			CLOSE(ofd);
		}

		/* Free structures */
		sa_sys_free();
		exit(0);
	}

	/* Set a handler for SIGALRM */
	alarm_handler(0);

	/* Main loop */
	rw_sa_stat_loop(count, &rectime, stdfd, ofd, ofile);

#ifdef HAVE_SENSORS
	/* Cleanup sensors */
	sensors_cleanup();
#endif /* HAVE_SENSORS */

	/* Free structures */
	sa_sys_free();

	return 0;
}
