/*
 * sar, sadc, sadf, mpstat and iostat common routines.
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
#include <time.h>
#include <errno.h>
#include <unistd.h>	/* For STDOUT_FILENO, among others */
#include <sys/ioctl.h>
#include <sys/types.h>
#include <dirent.h>
#include <limits.h>		//PATH_MAX, LONG_MIN, LONG_MAX

#include "version.h"
#include "common.h"
#include "ioconf.h"
#include "rd_stats.h"

#ifdef USE_NLS
#include <locale.h>
#include <libintl.h>
#define _(string) gettext(string)
#else
#define _(string) (string)
#endif

/* Number of ticks per second */
unsigned int hz;
/* Number of bit shifts to convert pages to kB */
unsigned int kb_shift;

/*
 ***************************************************************************
 * Print sysstat version number and exit.
 ***************************************************************************
 */
void print_version(void)
{
	printf(_("sysstat version %s\n"), VERSION);
	printf("(C) Sebastien Godard (sysstat <at> orange.fr)\n");
	exit(0);
}

/*
 ***************************************************************************
 * Get local date and time.
 *
 * OUT:
 * @rectime	Current local date and time.
 *
 * RETURNS:
 * Value of time in seconds since the Epoch.
 ***************************************************************************
 */
time_t get_localtime(struct tm *rectime)
{
	time_t timer;
	struct tm *ltm;

	time(&timer);
	ltm = localtime(&timer);

	*rectime = *ltm;
	return timer;
}

/*
 ***************************************************************************
 * Get date and time expressed in UTC.
 *
 * OUT:
 * @rectime	Current date and time expressed in UTC.
 *
 * RETURNS:
 * Value of time in seconds since the Epoch.
 ***************************************************************************
 */
time_t get_gmtime(struct tm *rectime)
{
	time_t timer;
	struct tm *ltm;

	time(&timer);
	ltm = gmtime(&timer);

	*rectime = *ltm;
	return timer;
}

/*
 ***************************************************************************
 * Get date and time and take into account <ENV_TIME_DEFTM> variable.
 *
 * OUT:
 * @rectime	Current date and time.
 *
 * RETURNS:
 * Value of time in seconds since the Epoch.
 ***************************************************************************
 */
time_t get_time(struct tm *rectime)
{
	static int utc = 0;
	char *e;

	if (!utc) {
		/* Read environment variable value once */
		if ((e = getenv(ENV_TIME_DEFTM)) != NULL) {
			utc = !strcmp(e, K_UTC);
		}
		utc++;
	}
	
	if (utc == 2)
		return get_gmtime(rectime);
	else
		return get_localtime(rectime);
}

/*
 ***************************************************************************
 * Count number of comma-separated values in arguments list. For example,
 * the number will be 3 for the list "foobar -p 1 -p 2,3,4 2 5".
 *
 * IN:
 * @arg_c	Number of arguments in the list.
 * @arg_v	Arguments list.
 *
 * RETURNS:
 * Number of comma-separated values in the list.
 ***************************************************************************
 */
int count_csvalues(int arg_c, char **arg_v)
{
	int opt = 1;
	int nr = 0;
	char *t;
	
	while (opt < arg_c) {
		if (strchr(arg_v[opt], ',')) {
			for (t = arg_v[opt]; t; t = strchr(t + 1, ',')) {
				nr++;
			}
		}
		opt++;
	}
	
	return nr;
}

/*
 ***************************************************************************
 * Look for partitions of a given block device in /sys filesystem.
 *
 * IN:
 * @dev_name	Name of the block device.
 *
 * RETURNS:
 * Number of partitions for the given block device.
 ***************************************************************************
 */
int get_dev_part_nr(char *dev_name)
{
	DIR *dir;
	struct dirent *drd;
	char dfile[MAX_PF_NAME], line[MAX_PF_NAME];
	int part = 0;

	snprintf(dfile, MAX_PF_NAME, "%s/%s", SYSFS_BLOCK, dev_name);
	dfile[MAX_PF_NAME - 1] = '\0';

	/* Open current device directory in /sys/block */
	if ((dir = opendir(dfile)) == NULL)
		return 0;

	/* Get current file entry */
	while ((drd = readdir(dir)) != NULL) {
		if (!strcmp(drd->d_name, ".") || !strcmp(drd->d_name, ".."))
			continue;
		snprintf(line, MAX_PF_NAME, "%s/%s/%s", dfile, drd->d_name, S_STAT);
		line[MAX_PF_NAME - 1] = '\0';

		/* Try to guess if current entry is a directory containing a stat file */
		if (!access(line, R_OK)) {
			/* Yep... */
			part++;
		}
	}
	
	/* Close directory */
	closedir(dir);

	return part;
}

/*
 ***************************************************************************
 * Look for block devices present in /sys/ filesystem:
 * Check first that sysfs is mounted (done by trying to open /sys/block
 * directory), then find number of devices registered.
 *
 * IN:
 * @display_partitions	Set to TRUE if partitions must also be counted.
 *
 * RETURNS:
 * Total number of block devices (and partitions if @display_partitions was
 * set).
 ***************************************************************************
 */
int get_sysfs_dev_nr(int display_partitions)
{
	DIR *dir;
	struct dirent *drd;
	char line[MAX_PF_NAME];
	int dev = 0;

	/* Open /sys/block directory */
	if ((dir = opendir(SYSFS_BLOCK)) == NULL)
		/* sysfs not mounted, or perhaps this is an old kernel */
		return 0;

	/* Get current file entry in /sys/block directory */
	while ((drd = readdir(dir)) != NULL) {
		if (!strcmp(drd->d_name, ".") || !strcmp(drd->d_name, ".."))
			continue;
		snprintf(line, MAX_PF_NAME, "%s/%s/%s", SYSFS_BLOCK, drd->d_name, S_STAT);
		line[MAX_PF_NAME - 1] = '\0';

		/* Try to guess if current entry is a directory containing a stat file */
		if (!access(line, R_OK)) {
			/* Yep... */
			dev++;
	
			if (display_partitions) {
				/* We also want the number of partitions for this device */
				dev += get_dev_part_nr(drd->d_name);
			}
		}
	}

	/* Close /sys/block directory */
	closedir(dir);

	return dev;
}

/*
 ***************************************************************************
 * Read /proc/devices file and get device-mapper major number.
 * If device-mapper entry is not found in file, use DEFAULT_DEMAP_MAJOR
 * number.
 *
 * RETURNS:
 * Device-mapper major number.
 ***************************************************************************
 */
unsigned int get_devmap_major(void)
{
	FILE *fp;
	char line[128];
	unsigned int dm_major = DEFAULT_DEVMAP_MAJOR;

	if ((fp = fopen(DEVICES, "r")) == NULL)
		return dm_major;

	while (fgets(line, 128, fp) != NULL) {

		if (strstr(line, "device-mapper")) {
			/* Read device-mapper major number */
			sscanf(line, "%u", &dm_major);
		}
	}

	fclose(fp);

	return dm_major;
}

/*
 ***************************************************************************
 * Print banner.
 *
 * IN:
 * @rectime	Date and time to display.
 * @sysname	System name to display.
 * @release	System release number to display.
 * @nodename	Hostname to display.
 * @machine	Machine architecture to display.
 * @cpu_nr	Number of CPU.
 *
 * RETURNS:
 * TRUE if S_TIME_FORMAT is set to ISO, or FALSE otherwise.
 ***************************************************************************
 */
int print_gal_header(struct tm *rectime, char *sysname, char *release,
		     char *nodename, char *machine, int cpu_nr)
{
	char cur_date[64];
	char *e;
	int rc = 0;

	if (rectime == NULL) {
		strcpy(cur_date, "?/?/?");
	}
	else if (((e = getenv(ENV_TIME_FMT)) != NULL) && !strcmp(e, K_ISO)) {
		strftime(cur_date, sizeof(cur_date), "%Y-%m-%d", rectime);
		rc = 1;
	}
	else {
		strftime(cur_date, sizeof(cur_date), "%x", rectime);
	}

	printf("%s %s (%s) \t%s \t_%s_\t(%d CPU)\n", sysname, release, nodename,
	       cur_date, machine, cpu_nr);

	return rc;
}

#ifdef USE_NLS
/*
 ***************************************************************************
 * Init National Language Support.
 ***************************************************************************
 */
void init_nls(void)
{
	setlocale(LC_MESSAGES, "");
	setlocale(LC_CTYPE, "");
	setlocale(LC_TIME, "");
	setlocale(LC_NUMERIC, "");

	bindtextdomain(PACKAGE, LOCALEDIR);
	textdomain(PACKAGE);
}
#endif

/*
 ***************************************************************************
 * Get number of rows for current window.
 *
 * RETURNS:
 * Number of rows.
 ***************************************************************************
 */
int get_win_height(void)
{
	struct winsize win;
	/*
	 * This default value will be used whenever STDOUT
	 * is redirected to a pipe or a file
	 */
	int rows = 3600 * 24;

	if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &win) != -1) {
		if (win.ws_row > 2) {
			rows = win.ws_row - 2;
		}
	}
	return rows;
}

/*
 ***************************************************************************
 * Remove /dev from path name.
 *
 * IN:
 * @name	Device name (may begins with "/dev/")
 *
 * RETURNS:
 * Device basename.
 ***************************************************************************
 */
char *device_name(char *name)
{
	if (!strncmp(name, "/dev/", 5))
		return name + 5;

	return name;
}

/*
 ***************************************************************************
 * Test whether given name is a device or a partition, using sysfs.
 * This is more straightforward that using ioc_iswhole() function from
 * ioconf.c which should be used only with kernels that don't have sysfs.
 *
 * IN:
 * @name	Device or partition name.
 *
 * RETURNS:
 * TRUE if @name is a (whole) device.
 ***************************************************************************
 */
int is_device(char *name)
{
	char syspath[PATH_MAX];
	char *slash;

	/* Some devices may have a slash in their name (eg. cciss/c0d0...) */
	while ((slash = strchr(name, '/'))) {
		*slash = '!';
	}
	snprintf(syspath, sizeof(syspath), "%s/%s", SYSFS_BLOCK, name);
	
	return !(access(syspath, F_OK));
}

/*
 ***************************************************************************
 * Get page shift in kB.
 ***************************************************************************
 */
void get_kb_shift(void)
{
	int shift = 0;
	long size;

	/* One can also use getpagesize() to get the size of a page */
	if ((size = sysconf(_SC_PAGESIZE)) == -1) {
		perror("sysconf");
	}

	size >>= 10;	/* Assume that a page has a minimum size of 1 kB */

	while (size > 1) {
		shift++;
		size >>= 1;
	}

	kb_shift = (unsigned int) shift;
}

/*
 ***************************************************************************
 * Get number of clock ticks per second.
 ***************************************************************************
 */
void get_HZ(void)
{
	long ticks;

	if ((ticks = sysconf(_SC_CLK_TCK)) == -1) {
		perror("sysconf");
	}

	hz = (unsigned int) ticks;
}

/*
 ***************************************************************************
 * Handle overflow conditions properly for counters which are read as
 * unsigned long long, but which can be unsigned long long or
 * unsigned long only depending on the kernel version used.
 * @value1 and @value2 being two values successively read for this
 * counter, if @value2 < @value1 and @value1 <= 0xffffffff, then we can
 * assume that the counter's type was unsigned long and has overflown, and
 * so the difference @value2 - @value1 must be casted to this type.
 * NOTE: These functions should no longer be necessary to handle a particular
 * stat counter when we can assume that everybody is using a recent kernel
 * (defining this counter as unsigned long long).
 ***************************************************************************
 */
double ll_sp_value(unsigned long long value1, unsigned long long value2,
		   unsigned long long itv)
{
	if ((value2 < value1) && (value1 <= 0xffffffff))
		/* Counter's type was unsigned long and has overflown */
		return ((double) ((value2 - value1) & 0xffffffff)) / itv * 100;
	else
		return SP_VALUE(value1, value2, itv);
}

double ll_s_value(unsigned long long value1, unsigned long long value2,
		  unsigned long long itv)
{
	if ((value2 < value1) && (value1 <= 0xffffffff))
		/* Counter's type was unsigned long and has overflown */
		return ((double) ((value2 - value1) & 0xffffffff)) / itv * HZ;
	else
		return S_VALUE(value1, value2, itv);
}

/*
 ***************************************************************************
 * Compute time interval.
 *
 * IN:
 * @prev_uptime	Previous uptime value in jiffies.
 * @curr_uptime	Current uptime value in jiffies.
 *
 * RETURNS:
 * Interval of time in jiffies.
 ***************************************************************************
 */
unsigned long long get_interval(unsigned long long prev_uptime,
				unsigned long long curr_uptime)
{
	unsigned long long itv;

	/* prev_time=0 when displaying stats since system startup */
	itv = curr_uptime - prev_uptime;
	
	if (!itv) {	/* Paranoia checking */
		itv = 1;
	}

	return itv;
}

/*
 ***************************************************************************
 * Since ticks may vary slightly from CPU to CPU, we'll want
 * to recalculate itv based on this CPU's tick count, rather
 * than that reported by the "cpu" line. Otherwise we
 * occasionally end up with slightly skewed figures, with
 * the skew being greater as the time interval grows shorter.
 *
 * IN:
 * @scc	Current sample statistics for current CPU.
 * @scp	Previous sample statistics for current CPU.
 *
 * RETURNS:
 * Interval of time based on current CPU.
 ***************************************************************************
 */
unsigned long long get_per_cpu_interval(struct stats_cpu *scc,
					struct stats_cpu *scp)
{
	/* Don't take cpu_guest into account because cpu_user already includes it */
	return ((scc->cpu_user    + scc->cpu_nice   +
		 scc->cpu_sys     + scc->cpu_iowait +
		 scc->cpu_idle    + scc->cpu_steal  +
		 scc->cpu_hardirq + scc->cpu_softirq) -
		(scp->cpu_user    + scp->cpu_nice   +
		 scp->cpu_sys     + scp->cpu_iowait +
		 scp->cpu_idle    + scp->cpu_steal  +
		 scp->cpu_hardirq + scp->cpu_softirq));
}

/*
 ***************************************************************************
 * Unhandled situation: Panic and exit.
 *
 * IN:
 * @function	Function name where situation occured.
 * @error_code	Error code.
 ***************************************************************************
 */
#ifdef DEBUG
void sysstat_panic(const char *function, int error_code)
{
	fprintf(stderr, "sysstat: %s[%d]: Last chance handler...\n",
		function, error_code);
	exit(1);
}
#endif

/*
 ***************************************************************************
 * Count number of bits set in an array.
 *
 * IN:
 * @ptr		Pointer to array.
 * @size	Size of array in bytes.
 *
 * RETURNS:
 * Number of bits set in the array.
 ***************************************************************************
*/
int count_bits(void *ptr, int size)
{
	int nr = 0, i, k;
	char *p;

	p = ptr;
	for (i = 0; i < size; i++, p++) {
		k = 0x80;
		while (k) {
			if (*p & k)
				nr++;
			k >>= 1;
		}
	}

	return nr;
}

/*
 ***************************************************************************
 * Compute "extended" device statistics (service time, etc.).
 *
 * IN:
 * @sdc		Structure with current device statistics.
 * @sdp		Structure with previous device statistics.
 * @itv		Interval of time in jiffies.
 *
 * OUT:
 * @xds		Structure with extended statistics.
 ***************************************************************************
*/
void compute_ext_disk_stats(struct stats_disk *sdc, struct stats_disk *sdp,
			    unsigned long long itv, struct ext_disk_stats *xds)
{
	double tput
		= ((double) (sdc->nr_ios - sdp->nr_ios)) * HZ / itv;
	
	xds->util  = S_VALUE(sdp->tot_ticks, sdc->tot_ticks, itv);
	xds->svctm = tput ? xds->util / tput : 0.0;
	/*
	 * Kernel gives ticks already in milliseconds for all platforms
	 * => no need for further scaling.
	 */
	xds->await = (sdc->nr_ios - sdp->nr_ios) ?
		((sdc->rd_ticks - sdp->rd_ticks) + (sdc->wr_ticks - sdp->wr_ticks)) /
		((double) (sdc->nr_ios - sdp->nr_ios)) : 0.0;
	xds->arqsz = (sdc->nr_ios - sdp->nr_ios) ?
		((sdc->rd_sect - sdp->rd_sect) + (sdc->wr_sect - sdp->wr_sect)) /
		((double) (sdc->nr_ios - sdp->nr_ios)) : 0.0;
}
