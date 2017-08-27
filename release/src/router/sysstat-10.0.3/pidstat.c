/*
 * pidstat: Report statistics for Linux tasks
 * (C) 2007-2011 by Sebastien GODARD (sysstat <at> orange.fr)
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
#include <dirent.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <regex.h>

#include "version.h"
#include "pidstat.h"
#include "common.h"
#include "rd_stats.h"

#ifdef USE_NLS
#include <locale.h>
#include <libintl.h>
#define _(string) gettext(string)
#else
#define _(string) (string)
#endif

#define SCCSID "@(#)sysstat-" VERSION ": " __FILE__ " compiled " __DATE__ " " __TIME__
char *sccsid(void) { return (SCCSID); }

unsigned long long uptime[3] = {0, 0, 0};
unsigned long long uptime0[3] = {0, 0, 0};
struct pid_stats *st_pid_list[3] = {NULL, NULL, NULL};
unsigned int *pid_array = NULL;
struct pid_stats st_pid_null;
struct tm ps_tstamp[3];
char commstr[MAX_COMM_LEN];

unsigned int pid_nr = 0;	/* Nb of PID to display */
unsigned int pid_array_nr = 0;
int cpu_nr = 0;			/* Nb of processors on the machine */
unsigned long tlmkb;		/* Total memory in kB */
long interval = -1;
long count = 0;
unsigned int pidflag = 0;	/* General flags */
unsigned int tskflag = 0;	/* TASK/CHILD stats */
unsigned int actflag = 0;	/* Activity flag */


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
	fprintf(stderr, _("Usage: %s [ options ] [ <interval> [ <count> ] ]\n"),
		progname);

	fprintf(stderr, _("Options are:\n"
			  "[ -C <command> ] [ -d ] [ -h ] [ -I ] [ -l ] [ -r ] [ -s ]\n"
			  "[ -t ] [ -u ] [ -V ] [ -w ]\n"
			  "[ -p { <pid> [,...] | SELF | ALL } ] [ -T { TASK | CHILD | ALL } ]\n"));
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
 * Initialize uptime variables.
 ***************************************************************************
 */
void init_stats(void)
{
	memset(&st_pid_null, 0, PID_STATS_SIZE);
}

/*
 ***************************************************************************
 * Allocate structures for PIDs entered on command line.
 *
 * IN:
 * @len	Number of PIDs entered on the command line.
 ***************************************************************************
 */
void salloc_pid_array(unsigned int len)
{
	if ((pid_array = (unsigned int *) malloc(sizeof(int) * len)) == NULL) {
		perror("malloc");
		exit(4);
	}
	memset(pid_array, 0, sizeof(int) * len);
}

/*
 ***************************************************************************
 * Allocate structures for PIDs to read.
 *
 * IN:
 * @len	Number of PIDs (and TIDs) on the system.
 ***************************************************************************
 */
void salloc_pid(unsigned int len)
{
	int i;

	for (i = 0; i < 3; i++) {
		if ((st_pid_list[i] = (struct pid_stats *) malloc(PID_STATS_SIZE * len)) == NULL) {
			perror("malloc");
			exit(4);
		}
		memset(st_pid_list[i], 0, PID_STATS_SIZE * len);
	}
}

/*
 ***************************************************************************
 * Free PID list structures.
 ***************************************************************************
 */
void sfree_pid(void)
{
	int i;
	
	for (i = 0; i < 3; i++) {
		if (st_pid_list[i]) {
			free(st_pid_list[i]);
		}
	}
}

/*
 ***************************************************************************
 * Check flags and set default values.
 ***************************************************************************
 */
void check_flags(void)
{
	unsigned int act = 0;

	/* Display CPU usage for active tasks by default */
	if (!actflag) {
		actflag |= P_A_CPU;
	}

	if (!DISPLAY_PID(pidflag)) {
		pidflag |= P_D_ACTIVE_PID + P_D_PID + P_D_ALL_PID;
	}

	if (!tskflag) {
		tskflag |= P_TASK;
	}

	/* Check that requested activities are available */
	if (DISPLAY_TASK_STATS(tskflag)) {
		act |= P_A_CPU + P_A_MEM + P_A_IO + P_A_CTXSW + P_A_STACK;
	}
	if (DISPLAY_CHILD_STATS(tskflag)) {
		act |= P_A_CPU + P_A_MEM;
	}

	actflag &= act;

	if (!actflag) {
		fprintf(stderr, _("Requested activities not available\n"));
		exit(1);
	}
}

/*
 ***************************************************************************
 * Look for the PID in the list of PIDs entered on the command line, and
 * store it if necessary.
 *
 * IN:
 * @pid_array_nr	Length of the PID list.
 * @pid			PID to search.
 *
 * OUT:
 * @pid_array_nr	New length of the PID list.
 *
 * RETURNS:
 * Returns the position of the PID in the list.
 ***************************************************************************
 */
int update_pid_array(unsigned int *pid_array_nr, unsigned int pid)
{
	unsigned int i;

	for (i = 0; i < *pid_array_nr; i++) {
		if (pid_array[i] == pid)
			break;
	}

	if (i == *pid_array_nr) {
		/* PID not found: Store it */
		(*pid_array_nr)++;
		pid_array[i] = pid;
	}

	return i;
}

/*
 ***************************************************************************
 * Display process command name or command line.
 *
 * IN:
 * @pst		Pointer on structure with process stats and command line.
 ***************************************************************************
 */
void print_comm(struct pid_stats *pst)
{
	char *p;
	
	if (DISPLAY_CMDLINE(pidflag) && strlen(pst->cmdline)) {
		p = pst->cmdline;
	}
	else {
		p = pst->comm;
	}

	printf("  %s%s\n", pst->tgid ? "|__" : "", p);
}

/*
 ***************************************************************************
 * Read /proc/meminfo.
 ***************************************************************************
 */
void read_proc_meminfo(void)
{
	struct stats_memory st_mem;

	memset(&st_mem, 0, STATS_MEMORY_SIZE);
	read_meminfo(&st_mem);
	tlmkb = st_mem.tlmkb;
}

/*
 ***************************************************************************
 * Read stats from /proc/#[/task/##]/stat.
 *
 * IN:
 * @pid		Process whose stats are to be read.
 * @pst		Pointer on structure where stats will be saved.
 * @tgid	If !=0, thread whose stats are to be read.
 *
 * OUT:
 * @pst		Pointer on structure where stats have been saved.
 * @thread_nr	Number of threads of the process.
 *
 * RETURNS:
 * 0 if stats have been successfully read, and 1 otherwise.
 ***************************************************************************
 */
int read_proc_pid_stat(unsigned int pid, struct pid_stats *pst,
		       unsigned int *thread_nr, unsigned int tgid)
{
	FILE *fp;
	char filename[128], format[256], comm[MAX_COMM_LEN + 1];
	size_t len;

	if (tgid) {
		sprintf(filename, TASK_STAT, tgid, pid);
	}
	else {
		sprintf(filename, PID_STAT, pid);
	}

	if ((fp = fopen(filename, "r")) == NULL)
		/* No such process */
		return 1;

	sprintf(format, "%%*d (%%%ds %%*s %%*d %%*d %%*d %%*d %%*d %%*u %%lu %%lu"
		" %%lu %%lu %%lu %%lu %%lu %%lu %%*d %%*d %%u %%*u %%*d %%lu %%lu"
		" %%*u %%*u %%*u %%*u %%*u %%*u %%*u %%*u %%*u %%*u %%*u %%*u %%*u"
		" %%*u %%u %%*u %%*u %%*u %%lu %%lu\\n", MAX_COMM_LEN);

	fscanf(fp, format, comm,
	       &pst->minflt, &pst->cminflt, &pst->majflt, &pst->cmajflt,
	       &pst->utime,  &pst->stime, &pst->cutime, &pst->cstime,
	       thread_nr, &pst->vsz, &pst->rss, &pst->processor,
	       &pst->gtime, &pst->cgtime);

	fclose(fp);

	/* Convert to kB */
	pst->vsz >>= 10;
	pst->rss = PG_TO_KB(pst->rss);

	strncpy(pst->comm, comm, MAX_COMM_LEN);
	pst->comm[MAX_COMM_LEN - 1] = '\0';

	/* Remove trailing ')' */
	len = strlen(pst->comm);
	if (len && (pst->comm[len - 1] == ')')) {
		pst->comm[len - 1] = '\0';
	}

	pst->pid = pid;
	pst->tgid = tgid;
	return 0;
}

/*
 *****************************************************************************
 * Read stats from /proc/#[/task/##]/status.
 *
 * IN:
 * @pid		Process whose stats are to be read.
 * @pst		Pointer on structure where stats will be saved.
 * @tgid	If !=0, thread whose stats are to be read.
 *
 * OUT:
 * @pst		Pointer on structure where stats have been saved.
 *
 * RETURNS:
 * 0 if stats have been successfully read, and 1 otherwise.
 *****************************************************************************
 */
int read_proc_pid_status(unsigned int pid, struct pid_stats *pst,
			 unsigned int tgid)
{
	FILE *fp;
	char filename[128], line[256];

	if (tgid) {
		sprintf(filename, TASK_STATUS, tgid, pid);
	}
	else {
		sprintf(filename, PID_STATUS, pid);
	}

	if ((fp = fopen(filename, "r")) == NULL)
		/* No such process */
		return 1;

	while (fgets(line, 256, fp) != NULL) {

		if (!strncmp(line, "voluntary_ctxt_switches:", 24)) {
			sscanf(line + 25, "%lu", &pst->nvcsw);
		}
		else if (!strncmp(line, "nonvoluntary_ctxt_switches:", 27)) {
			sscanf(line + 28, "%lu", &pst->nivcsw);
		}
	}

	fclose(fp);

	pst->pid = pid;
	pst->tgid = tgid;
	return 0;
}

/*
  *****************************************************************************
  * Read information from /proc/#[/task/##}/smaps.
  *
  * @pid		Process whose stats are to be read.
  * @pst		Pointer on structure where stats will be saved.
  * @tgid		If !=0, thread whose stats are to be read.
  *
  * OUT:
  * @pst		Pointer on structure where stats have been saved.
  *
  * RETURNS:
  * 0 if stats have been successfully read, and 1 otherwise.
  *****************************************************************************
  */
int read_proc_pid_smap(unsigned int pid, struct pid_stats *pst, unsigned int tgid)
{
	FILE *fp;
	char filename[128], line[256];
	int state = 0;

	if (tgid) {
		sprintf(filename, TASK_SMAP, tgid, pid);
	}
	else {
		sprintf(filename, PID_SMAP, pid);
	}

	if ((fp = fopen(filename, "rt")) == NULL)
		/* No such process */
		return 1;

	while ((state < 3) && (fgets(line, sizeof(line), fp) != NULL)) {
		switch (state) {
			case 0:
				if (strstr(line, "[stack]")) {
					state = 1;
				}
				break;
			case 1:
				if (strstr(line, "Size:")) {
					sscanf(line + sizeof("Size:"), "%lu", &pst->stack_size);
					state = 2;
				}
				break;
			case 2:
				if (strstr(line, "Referenced:")) {
					sscanf(line + sizeof("Referenced:"), "%lu", &pst->stack_ref);
					state = 3;
				}
				break;
		}
	}

	fclose(fp);

	pst->pid = pid;
	pst->tgid = tgid;
	return 0;
}

/*
 *****************************************************************************
 * Read process command line from /proc/#[/task/##]/cmdline.
 *
 * IN:
 * @pid		Process whose command line is to be read.
 * @pst		Pointer on structure where command line will be saved.
 * @tgid	If !=0, thread whose command line is to be read.
 *
 * OUT:
 * @pst		Pointer on structure where command line has been saved.
 *
 * RETURNS:
 * 0 if command line has been successfully read, and 1 otherwise (the process
 * has terminated or its /proc/.../cmdline file is just empty).
 *****************************************************************************
 */
int read_proc_pid_cmdline(unsigned int pid, struct pid_stats *pst,
			  unsigned int tgid)
{
	FILE *fp;
	char filename[128], line[MAX_CMDLINE_LEN];
	size_t len;
	int i;

	if (tgid) {
		sprintf(filename, TASK_CMDLINE, tgid, pid);
	}
	else {
		sprintf(filename, PID_CMDLINE, pid);
	}

	if ((fp = fopen(filename, "r")) == NULL)
		/* No such process */
		return 1;

	memset(line, 0, MAX_CMDLINE_LEN);
	
	if ((len = fread(line, 1, MAX_CMDLINE_LEN - 1, fp)) < 0)
		/* Nothing to read doesn't mean that process no longer exists */
		return 1;
	
	for (i = 0; i < len; i++) {
		if (line[i] == '\0') {
			line[i] = ' ';
		}
	}

	fclose(fp);
	strncpy(pst->cmdline, line, MAX_CMDLINE_LEN);
	return 0;
}

/*
 ***************************************************************************
 * Read stats from /proc/#[/task/##]/io.
 *
 * IN:
 * @pid		Process whose stats are to be read.
 * @pst		Pointer on structure where stats will be saved.
 * @tgid	If !=0, thread whose stats are to be read.
 *
 * OUT:
 * @pst		Pointer on structure where stats have been saved.
 *
 * RETURNS:
 * 0 if stats have been successfully read.
 * Also returns 0 if current process has terminated or if its io file
 * doesn't exist, but in this case, set process' F_NO_PID_IO flag to
 * indicate that I/O stats should no longer be read for it.
 ***************************************************************************
 */
int read_proc_pid_io(unsigned int pid, struct pid_stats *pst,
		     unsigned int tgid)
{
	FILE *fp;
	char filename[128], line[256];

	if (tgid) {
		sprintf(filename, TASK_IO, tgid, pid);
	}
	else {
		sprintf(filename, PID_IO, pid);
	}

	if ((fp = fopen(filename, "r")) == NULL) {
		/* No such process... or file non existent! */
		pst->flags |= F_NO_PID_IO;
		/*
		 * Also returns 0 since io stats file doesn't necessarily exist,
		 * depending on the kernel version used.
		 */
		return 0;
	}

	while (fgets(line, 256, fp) != NULL) {

		if (!strncmp(line, "read_bytes:", 11)) {
			sscanf(line + 12, "%llu", &pst->read_bytes);
		}
		else if (!strncmp(line, "write_bytes:", 12)) {
			sscanf(line + 13, "%llu", &pst->write_bytes);
		}
		else if (!strncmp(line, "cancelled_write_bytes:", 22)) {
			sscanf(line + 23, "%llu", &pst->cancelled_write_bytes);
		}
	}

	fclose(fp);

	pst->pid = pid;
	pst->tgid = tgid;
	pst->flags &= ~F_NO_PID_IO;
	return 0;
}

/*
 ***************************************************************************
 * Read various stats for given PID.
 *
 * IN:
 * @pid		Process whose stats are to be read.
 * @pst		Pointer on structure where stats will be saved.
 * @tgid	If !=0, thread whose stats are to be read.
 *
 * OUT:
 * @pst		Pointer on structure where stats have been saved.
 * @thread_nr	Number of threads of the process.
 *
 * RETURNS:
 * 0 if stats have been successfully read, and 1 otherwise.
 ***************************************************************************
 */
int read_pid_stats(unsigned int pid, struct pid_stats *pst,
		   unsigned int *thread_nr, unsigned int tgid)
{
	if (read_proc_pid_stat(pid, pst, thread_nr, tgid))
		return 1;

	if (DISPLAY_CMDLINE(pidflag)) {
		if (read_proc_pid_cmdline(pid, pst, tgid))
			return 1;
	}
	
	if (read_proc_pid_status(pid, pst, tgid))
		return 1;

	if (DISPLAY_STACK(actflag)) {
		if (read_proc_pid_smap(pid, pst, tgid))
			return 1;
	}

	if (DISPLAY_IO(actflag))
		/* Assume that /proc/#/task/#/io exists! */
		return (read_proc_pid_io(pid, pst, tgid));

	return 0;
}

/*
 ***************************************************************************
 * Count number of threads in /proc/#/task directory, including the leader
 * one.
 *
 * IN:
 * @pid	Process number for which the number of threads are to be counted.
 *
 * RETURNS:
 * Number of threads for the given process (min value is 1).
 * A value of 0 indicates that the process has terminated.
 ***************************************************************************
 */
unsigned int count_tid(unsigned int pid)
{
	struct pid_stats pst;
	unsigned int thread_nr;

	if (read_proc_pid_stat(pid, &pst, &thread_nr, 0) != 0)
		/* Task no longer exists */
		return 0;

	return thread_nr;
}

/*
 ***************************************************************************
 * Count number of processes (and threads).
 *
 * RETURNS:
 * Number of processes (and threads if requested).
 ***************************************************************************
 */
unsigned int count_pid(void)
{
	DIR *dir;
	struct dirent *drp;
	unsigned int pid = 0;

	/* Open /proc directory */
	if ((dir = opendir(PROC)) == NULL) {
		perror("opendir");
		exit(4);
	}

	/* Get directory entries */
	while ((drp = readdir(dir)) != NULL) {
		if (isdigit(drp->d_name[0])) {
			/* There is at least the TGID */
			pid++;
			if (DISPLAY_TID(pidflag)) {
				pid += count_tid(atoi(drp->d_name));
			}
		}
	}

	/* Close /proc directory */
	closedir(dir);

	return pid;
}

/*
 ***************************************************************************
 * Count number of threads associated with the tasks entered on the command
 * line.
 *
 * RETURNS:
 * Number of threads (including the leading one) associated with every task
 * entered on the command line.
 ***************************************************************************
 */
unsigned int count_tid_in_list(void)
{
	unsigned int p, tid, pid = 0;

	for (p = 0; p < pid_array_nr; p++) {

		tid = count_tid(pid_array[p]);

		if (!tid) {
			/* PID no longer exists */
			pid_array[p] = 0;
		}
		else {
			/* <tid_value> TIDs + 1 TGID */
			pid += tid + 1;
		}
	}

	return pid;
}

/*
 ***************************************************************************
 * Allocate and init structures according to system state.
 ***************************************************************************
 */
void pid_sys_init(void)
{
	/* Init stat common counters */
	init_stats();

	/* Count nb of proc */
	cpu_nr = get_cpu_nr(~0);

	if (DISPLAY_ALL_PID(pidflag)) {
		/* Count PIDs and allocate structures */
		pid_nr = count_pid() + NR_PID_PREALLOC;
		salloc_pid(pid_nr);
	}
	else if (DISPLAY_TID(pidflag)) {
		/* Count total number of threads associated with tasks in list */
		pid_nr = count_tid_in_list() + NR_PID_PREALLOC;
		salloc_pid(pid_nr);
	}
	else {
		pid_nr = pid_array_nr;
		salloc_pid(pid_nr);
	}
}

/*
 ***************************************************************************
 * Read stats for threads in /proc/#/task directory.
 *
 * IN:
 * @curr	Index in array for current sample statistics.
 * @pid		Process number whose threads stats are to be read.
 * @index	Index in process list where stats will be saved.
 *
 * OUT:
 * @index	Index in process list where next stats will be saved.
 ***************************************************************************
 */
void read_task_stats(int curr, unsigned int pid, unsigned int *index)
{
	DIR *dir;
	struct dirent *drp;
	char filename[128];
	struct pid_stats *pst;
	unsigned int thr_nr;

	/* Open /proc/#/task directory */
	sprintf(filename, PROC_TASK, pid);
	if ((dir = opendir(filename)) == NULL)
		return;

	while (*index < pid_nr) {

		while ((drp = readdir(dir)) != NULL) {
			if (isdigit(drp->d_name[0]))
				break;
		}

		if (drp) {
			pst = st_pid_list[curr] + (*index)++;
			if (read_pid_stats(atoi(drp->d_name), pst, &thr_nr, pid)) {
				/* Thread no longer exists */
				pst->pid = 0;
			}
		}
		else
			break;
	}
	closedir(dir);
}

/*
 ***************************************************************************
 * Read various stats.
 *
 * IN:
 * @curr	Index in array for current sample statistics.
 ***************************************************************************
 */
void read_stats(int curr)
{
	DIR *dir;
	struct dirent *drp;
	unsigned int p = 0, q, pid, thr_nr;
	struct pid_stats *pst;
	struct stats_cpu *st_cpu;

	/*
	 * Allocate two structures for CPU statistics.
	 * No need to init them (done by read_stat_cpu() function).
	 */
	if ((st_cpu = (struct stats_cpu *) malloc(STATS_CPU_SIZE * 2)) == NULL) {
		perror("malloc");
		exit(4);
	}
	/* Read statistics for CPUs "all" and 0 */
	read_stat_cpu(st_cpu, 2, &uptime[curr], &uptime0[curr]);
	free(st_cpu);

	if (DISPLAY_ALL_PID(pidflag)) {

		/* Open /proc directory */
		if ((dir = opendir(PROC)) == NULL) {
			perror("opendir");
			exit(4);
		}

		while (p < pid_nr) {

			/* Get directory entries */
			while ((drp = readdir(dir)) != NULL) {
				if (isdigit(drp->d_name[0]))
					break;
			}
			if (drp) {
				pst = st_pid_list[curr] + p++;
				pid = atoi(drp->d_name);
	
				if (read_pid_stats(pid, pst, &thr_nr, 0)) {
					/* Process has terminated */
					pst->pid = 0;
				}
	
				else if (DISPLAY_TID(pidflag)) {
					/* Read stats for threads in task subdirectory */
					read_task_stats(curr, pid, &p);
				}
			}
			else {
				for (q = p; q < pid_nr; q++) {
					pst = st_pid_list[curr] + q;
					pst->pid = 0;
				}
				break;
			}
		}

		/* Close /proc directory */
		closedir(dir);
	}

	else if (DISPLAY_PID(pidflag)) {
		unsigned int op;

		/* Read stats for each PID in the list */
		for (op = 0; op < pid_array_nr; op++) {

			if (p >= pid_nr)
				break;
			pst = st_pid_list[curr] + p++;
	
			if (pid_array[op]) {
				/* PID should still exist. So read its stats */
				if (read_pid_stats(pid_array[op], pst, &thr_nr, 0)) {
					/* PID has terminated */
					pst->pid = 0;
					pid_array[op] = 0;
				}
				else if (DISPLAY_TID(pidflag)) {
					read_task_stats(curr, pid_array[op], &p);
				}
			}
		}
		/* Reset remaining structures */
		for (q = p; q < pid_nr; q++) {
			pst = st_pid_list[curr] + q;
			pst->pid = 0;
		}
		
	}
	/* else unknown command */
}

/*
 ***************************************************************************
 * Get current PID to display.
 * First, check that PID exists. *Then* check that it's an active process
 * and/or that the string (entered on the command line with option -C)
 * is found in command name.
 *
 * IN:
 * @prev	Index in array where stats used as reference are.
 * @curr	Index in array for current sample statistics.
 * @p		Index in process list.
 * @activity	Current activity to display (CPU, memory...).
 * 		Can be more than one if stats are displayed on one line.
 * @pflag	Flag indicating whether stats are to be displayed for
 * 		individual tasks or for all their children.
 *
 * OUT:
 * @pstc	Structure with PID statistics for current sample.
 * @pstp	Structure with PID statistics for previous sample.
 *
 * RETURNS:
 *  0 if PID no longer exists.
 * -1 if PID exists but should not be displayed.
 *  1 if PID can be displayed.
 ***************************************************************************
 */
int get_pid_to_display(int prev, int curr, int p, unsigned int activity,
		       unsigned int pflag,
		       struct pid_stats **pstc, struct pid_stats **pstp)
{
	int q, rc;
	regex_t regex;

	*pstc = st_pid_list[curr] + p;

	if (!(*pstc)->pid)
		/* PID no longer exists */
		return 0;

	if (DISPLAY_ALL_PID(pidflag) || DISPLAY_TID(pidflag)) {

		/* Look for previous stats for same PID */
		q = p;
	
		do {
			*pstp = st_pid_list[prev] + q;
			if (((*pstp)->pid == (*pstc)->pid) &&
			    ((*pstp)->tgid == (*pstc)->tgid))
				break;
			q++;
			if (q >= pid_nr) {
				q = 0;
			}
		}
		while (q != p);

		if (((*pstp)->pid != (*pstc)->pid) ||
		    ((*pstp)->tgid != (*pstc)->tgid)) {
			/* PID not found (no data previously read) */
			*pstp = &st_pid_null;
		}

		if (DISPLAY_ACTIVE_PID(pidflag)) {
			int isActive = FALSE;
			
			/* Check that it's an "active" process */
			if (DISPLAY_CPU(activity)) {
				/* User time already includes guest time */
				if (((*pstc)->utime != (*pstp)->utime) ||
				    ((*pstc)->stime != (*pstp)->stime)) {
					isActive = TRUE;
				}
				else {
					/*
					 * Process is not active but if we are showing
					 * child stats then we need to look there.
					 */
					if (DISPLAY_CHILD_STATS(pflag)) {
						/* User time already includes guest time */
						if (((*pstc)->cutime != (*pstp)->cutime) ||
						    ((*pstc)->cstime != (*pstp)->cstime)) {
							isActive = TRUE;
						}
					}
				}
			}
			
			if (DISPLAY_MEM(activity) && (!isActive)) {
				if (((*pstc)->minflt != (*pstp)->minflt) ||
				    ((*pstc)->majflt != (*pstp)->majflt)) {
					isActive = TRUE;
				}
				else {
					if (DISPLAY_TASK_STATS(pflag)) {
						if (((*pstc)->vsz != (*pstp)->vsz) ||
						    ((*pstc)->rss != (*pstp)->rss)) {
							isActive = TRUE;
						}
					}
					else if (DISPLAY_CHILD_STATS(pflag)) {
						if (((*pstc)->cminflt != (*pstp)->cminflt) ||
						    ((*pstc)->cmajflt != (*pstp)->cmajflt)) {
							isActive = TRUE;
						}
					}
				}
			}

			if (DISPLAY_IO(activity) && (!isActive) &&
				 /* /proc/#/io file should exist to check I/O stats */
				 !(NO_PID_IO((*pstc)->flags))) {
				if (((*pstc)->read_bytes  != (*pstp)->read_bytes)  ||
				    ((*pstc)->write_bytes != (*pstp)->write_bytes) ||
				    ((*pstc)->cancelled_write_bytes !=
				     (*pstp)->cancelled_write_bytes)) {
					isActive = TRUE;
				}
			}
			
			if (DISPLAY_CTXSW(activity) && (!isActive)) {
				if (((*pstc)->nvcsw  != (*pstp)->nvcsw) ||
				    ((*pstc)->nivcsw != (*pstp)->nivcsw)) {
					isActive = TRUE;
				}
			}
			
			/* If PID isn't active for any of the activities then return */
			if (!isActive)
				return -1;
		}
	}
	
	else if (DISPLAY_PID(pidflag)) {

		*pstp = st_pid_list[prev] + p;

		if (!(*pstp)->pid)
			/* PID no longer exists */
			return 0;
	}

	if (COMMAND_STRING(pidflag)) {

		if (regcomp(&regex, commstr, REG_EXTENDED | REG_NOSUB) != 0)
			/* Error in preparing regex structure */
			return -1;
		
		rc = regexec(&regex, (*pstc)->comm, 0, NULL, 0);
		regfree(&regex);

		if (rc)
			/* regex pattern not found in command name */
			return -1;
	}

	return 1;
}

/*
 ***************************************************************************
 * Display PID and TID.
 *
 * IN:
 * @pst		Current process statistics.
 * @c		No-op character.
 ***************************************************************************
 */
void __print_line_id(struct pid_stats *pst, char c)
{
	char format[32];

	if (DISPLAY_TID(pidflag)) {
		
		if (pst->tgid) {
			/* This is a TID */
			sprintf(format, "         %c %%9u", c);
		}
		else {
			/* This is a PID (TGID) */
			sprintf(format, " %%9u         %c", c);
		}
	}
	else {
		strcpy(format, " %9u");
	}

	printf(format, pst->pid);
}

/*
 ***************************************************************************
 * Display timestamp, PID and TID.
 *
 * IN:
 * @timestamp	Current timestamp.
 * @pst		Current process statistics.
 ***************************************************************************
 */
void print_line_id(char *timestamp, struct pid_stats *pst)
{
	printf("%-11s", timestamp);
	
	__print_line_id(pst, '-');
}

/*
 ***************************************************************************
 * Display all statistics for tasks in one line format.
 *
 * IN:
 * @prev	Index in array where stats used as reference are.
 * @curr	Index in array for current sample statistics.
 * @dis		TRUE if a header line must be printed.
 * @itv		Interval of time in jiffies.
 * @g_itv	Interval of time in jiffies multiplied by the number of
 * 		processors.
 *
 * RETURNS:
 * 0 if all the processes to display have terminated.
 * <> 0 if there are still some processes left to display.
 ***************************************************************************
 */
int write_pid_task_all_stats(int prev, int curr, int dis,
			     unsigned long long itv,
			     unsigned long long g_itv)
{
	struct pid_stats *pstc, *pstp;
	unsigned int p;
	int again = 0;

	if (dis) {
		PRINT_ID_HDR("#      Time", pidflag);
		if (DISPLAY_CPU(actflag)) {
			printf("    %%usr %%system  %%guest    %%CPU   CPU");
		}
		if (DISPLAY_MEM(actflag)) {
			printf("  minflt/s  majflt/s     VSZ    RSS   %%MEM");
		}
		if (DISPLAY_STACK(actflag)) {
			printf(" StkSize  StkRef");
		}
		if (DISPLAY_IO(actflag)) {
			printf("   kB_rd/s   kB_wr/s kB_ccwr/s");
		}
		if (DISPLAY_CTXSW(actflag)) {
			printf("   cswch/s nvcswch/s");
		}
		printf("  Command\n");
	}

	for (p = 0; p < pid_nr; p++) {

		if (get_pid_to_display(prev, curr, p, actflag, P_TASK,
				       &pstc, &pstp) <= 0)
			continue;

		printf("%11ld", (long) time(NULL));
		__print_line_id(pstc, '0');

		if (DISPLAY_CPU(actflag)) {
			printf(" %7.2f %7.2f %7.2f %7.2f",
			       (pstc->utime - pstc->gtime) < (pstp->utime - pstp->gtime) ?
			       0.0 :
			       SP_VALUE(pstp->utime - pstp->gtime,
					pstc->utime - pstc->gtime, itv),
			       SP_VALUE(pstp->stime,  pstc->stime, itv),
			       SP_VALUE(pstp->gtime,  pstc->gtime, itv),
			       /* User time already includes guest time */
			       IRIX_MODE_OFF(pidflag) ?
			       SP_VALUE(pstp->utime + pstp->stime,
					pstc->utime + pstc->stime, g_itv) :
			       SP_VALUE(pstp->utime + pstp->stime,
					pstc->utime + pstc->stime, itv));

			printf("   %3d", pstc->processor);
		}


		if (DISPLAY_MEM(actflag)) {
			printf(" %9.2f %9.2f %7lu %6lu %6.2f",
			       S_VALUE(pstp->minflt, pstc->minflt, itv),
			       S_VALUE(pstp->majflt, pstc->majflt, itv),
			       pstc->vsz,
			       pstc->rss,
			       tlmkb ? SP_VALUE(0, pstc->rss, tlmkb) : 0.0);
		}

		if (DISPLAY_STACK(actflag)) {
			printf("  %6lu  %6lu",
			       pstc->stack_size,
			       pstc->stack_ref);
		}

		if (DISPLAY_IO(actflag)) {
			if (!NO_PID_IO(pstc->flags))
			{
				printf(" %9.2f %9.2f %9.2f",
				       S_VALUE(pstp->read_bytes,  pstc->read_bytes, itv)  / 1024,
				       S_VALUE(pstp->write_bytes, pstc->write_bytes, itv) / 1024,
				       S_VALUE(pstp->cancelled_write_bytes,
					       pstc->cancelled_write_bytes, itv) / 1024);
			}
			else {
				/*
				 * Keep the layout even though this task has no I/O
				 * typically threads with no I/O measurements.
				 */
				printf(" %9.2f %9.2f %9.2f", -1.0, -1.0, -1.0);
			}
		}

		if (DISPLAY_CTXSW(actflag)) {
			printf(" %9.2f %9.2f",
			       S_VALUE(pstp->nvcsw, pstc->nvcsw, itv),
			       S_VALUE(pstp->nivcsw, pstc->nivcsw, itv));
		}
		
		print_comm(pstc);
		again = 1;
	}

	return again;
}

/*
 ***************************************************************************
 * Display all statistics for tasks' children in one line format.
 *
 * IN:
 * @prev	Index in array where stats used as reference are.
 * @curr	Index in array for current sample statistics.
 * @dis		TRUE if a header line must be printed.
 * @itv		Interval of time in jiffies.
 *
 * RETURNS:
 * 0 if all the processes to display have terminated.
 * <> 0 if there are still some processes left to display.
 ***************************************************************************
 */
int write_pid_child_all_stats(int prev, int curr, int dis,
			      unsigned long long itv)
{
	struct pid_stats *pstc, *pstp;
	unsigned int p;
	int again = 0;

	if (dis) {
		PRINT_ID_HDR("#      Time", pidflag);
		if (DISPLAY_CPU(actflag))
			printf("    usr-ms system-ms  guest-ms");
		if (DISPLAY_MEM(actflag))
			printf(" minflt-nr majflt-nr");
		printf("  Command\n");
	}

	for (p = 0; p < pid_nr; p++) {

		if (get_pid_to_display(prev, curr, p, actflag, P_CHILD,
				       &pstc, &pstp) <= 0)
			continue;

		printf("%11ld", (long) time(NULL));
		__print_line_id(pstc, '0');

		if (DISPLAY_CPU(actflag)) {
			printf(" %9.0f %9.0f %9.0f",
			       (pstc->utime + pstc->cutime - pstc->gtime - pstc->cgtime) <
			       (pstp->utime + pstp->cutime - pstp->gtime - pstp->cgtime) ?
			       0.0 :
			       (double) ((pstc->utime + pstc->cutime - pstc->gtime - pstc->cgtime) -
					 (pstp->utime + pstp->cutime - pstp->gtime - pstp->cgtime)) /
			       HZ * 1000,
			       (double) ((pstc->stime + pstc->cstime) -
					 (pstp->stime + pstp->cstime)) / HZ * 1000,
			       (double) ((pstc->gtime + pstc->cgtime) -
					 (pstp->gtime + pstp->cgtime)) / HZ * 1000);
		}


		if (DISPLAY_MEM(actflag)) {
			printf(" %9lu %9lu",
			       (pstc->minflt + pstc->cminflt) - (pstp->minflt + pstp->cminflt),
			       (pstc->majflt + pstc->cmajflt) - (pstp->majflt + pstp->cmajflt));
		}

		print_comm(pstc);
		again = 1;
	}

	return again;
}

/*
 ***************************************************************************
 * Display CPU statistics for tasks.
 *
 * IN:
 * @prev	Index in array where stats used as reference are.
 * @curr	Index in array for current sample statistics.
 * @dis		TRUE if a header line must be printed.
 * @disp_avg	TRUE if average stats are displayed.
 * @prev_string	String displayed at the beginning of a header line. This is
 * 		the timestamp of the previous sample, or "Average" when
 * 		displaying average stats.
 * @curr_string	String displayed at the beginning of current sample stats.
 * 		This is the timestamp of the current sample, or "Average"
 * 		when displaying average stats.
 * @itv		Interval of time in jiffies.
 * @g_itv	Interval of time in jiffies multiplied by the number of
 * 		processors.
 *
 * RETURNS:
 * 0 if all the processes to display have terminated.
 * <> 0 if there are still some processes left to display.
 ***************************************************************************
 */
int write_pid_task_cpu_stats(int prev, int curr, int dis, int disp_avg,
			     char *prev_string, char *curr_string,
			     unsigned long long itv,
			     unsigned long long g_itv)
{
	struct pid_stats *pstc, *pstp;
	unsigned int p;
	int again = 0;

	if (dis) {
		PRINT_ID_HDR(prev_string, pidflag);
		printf("    %%usr %%system  %%guest    %%CPU   CPU  Command\n");
	}

	for (p = 0; p < pid_nr; p++) {
	
		if (get_pid_to_display(prev, curr, p, P_A_CPU, P_TASK,
				       &pstc, &pstp) <= 0)
			continue;
	
		print_line_id(curr_string, pstc);
		printf(" %7.2f %7.2f %7.2f %7.2f",
		       (pstc->utime - pstc->gtime) < (pstp->utime - pstp->gtime) ?
		       0.0 :
		       SP_VALUE(pstp->utime - pstp->gtime,
				pstc->utime - pstc->gtime, itv),
		       SP_VALUE(pstp->stime,  pstc->stime, itv),
		       SP_VALUE(pstp->gtime,  pstc->gtime, itv),
		       /* User time already includes guest time */
		       IRIX_MODE_OFF(pidflag) ?
		       SP_VALUE(pstp->utime + pstp->stime,
				pstc->utime + pstc->stime, g_itv) :
		       SP_VALUE(pstp->utime + pstp->stime,
				pstc->utime + pstc->stime, itv));

		if (!disp_avg) {
			printf("   %3d", pstc->processor);
		}
		else {
			printf("     -");
		}
		print_comm(pstc);
		again = 1;
	}

	return again;
}

/*
 ***************************************************************************
 * Display CPU statistics for tasks' children.
 *
 * IN:
 * @prev	Index in array where stats used as reference are.
 * @curr	Index in array for current sample statistics.
 * @dis		TRUE if a header line must be printed.
 * @disp_avg	TRUE if average stats are displayed.
 * @prev_string	String displayed at the beginning of a header line. This is
 * 		the timestamp of the previous sample, or "Average" when
 * 		displaying average stats.
 * @curr_string	String displayed at the beginning of current sample stats.
 * 		This is the timestamp of the current sample, or "Average"
 * 		when displaying average stats.
 *
 * RETURNS:
 * 0 if all the processes to display have terminated.
 * <> 0 if there are still some processes left to display.
 ***************************************************************************
 */
int write_pid_child_cpu_stats(int prev, int curr, int dis, int disp_avg,
			      char *prev_string, char *curr_string)
{
	struct pid_stats *pstc, *pstp;
	unsigned int p;
	int rc, again = 0;

	if (dis) {
		PRINT_ID_HDR(prev_string, pidflag);
		printf("    usr-ms system-ms  guest-ms  Command\n");
	}

	for (p = 0; p < pid_nr; p++) {
	
		if ((rc = get_pid_to_display(prev, curr, p, P_A_CPU, P_CHILD,
					     &pstc, &pstp)) == 0)
			/* PID no longer exists */
			continue;
	
		/* This will be used to compute average */
		if (!disp_avg) {
			pstc->uc_asum_count = pstp->uc_asum_count + 1;
		}
	
		if (rc < 0)
			/* PID should not be displayed */
			continue;

		print_line_id(curr_string, pstc);
		if (disp_avg) {
			printf(" %9.0f %9.0f %9.0f",
			       (pstc->utime + pstc->cutime - pstc->gtime - pstc->cgtime) <
			       (pstp->utime + pstp->cutime - pstp->gtime - pstp->cgtime) ?
			       0.0 :
			       (double) ((pstc->utime + pstc->cutime - pstc->gtime - pstc->cgtime) -
					 (pstp->utime + pstp->cutime - pstp->gtime - pstp->cgtime)) /
			       (HZ * pstc->uc_asum_count) * 1000,
			       (double) ((pstc->stime + pstc->cstime) -
					 (pstp->stime + pstp->cstime)) /
			       (HZ * pstc->uc_asum_count) * 1000,
			       (double) ((pstc->gtime + pstc->cgtime) -
					 (pstp->gtime + pstp->cgtime)) /
			       (HZ * pstc->uc_asum_count) * 1000);
		}
		else {
			printf(" %9.0f %9.0f %9.0f",
			       (pstc->utime + pstc->cutime - pstc->gtime - pstc->cgtime) <
			       (pstp->utime + pstp->cutime - pstp->gtime - pstp->cgtime) ?
			       0.0 :
			       (double) ((pstc->utime + pstc->cutime - pstc->gtime - pstc->cgtime) -
					 (pstp->utime + pstp->cutime - pstp->gtime - pstp->cgtime)) /
			       HZ * 1000,
			       (double) ((pstc->stime + pstc->cstime) -
					 (pstp->stime + pstp->cstime)) / HZ * 1000,
			       (double) ((pstc->gtime + pstc->cgtime) -
					 (pstp->gtime + pstp->cgtime)) / HZ * 1000);
		}
		print_comm(pstc);
		again = 1;
	}
	
	return again;
}

/*
 ***************************************************************************
 * Display memory and/or stack size statistics for tasks.
 *
 * IN:
 * @prev	Index in array where stats used as reference are.
 * @curr	Index in array for current sample statistics.
 * @dis		TRUE if a header line must be printed.
 * @disp_avg	TRUE if average stats are displayed.
 * @prev_string	String displayed at the beginning of a header line. This is
 * 		the timestamp of the previous sample, or "Average" when
 * 		displaying average stats.
 * @curr_string	String displayed at the beginning of current sample stats.
 * 		This is the timestamp of the current sample, or "Average"
 * 		when displaying average stats.
 * @itv		Interval of time in jiffies.
 *
 * RETURNS:
 * 0 if all the processes to display have terminated.
 * <> 0 if there are still some processes left to display.
 ***************************************************************************
 */
int write_pid_task_memory_stats(int prev, int curr, int dis, int disp_avg,
				char *prev_string, char *curr_string,
				unsigned long long itv)
{
	struct pid_stats *pstc, *pstp;
	unsigned int p;
	int rc, again = 0;

	if (dis) {
		PRINT_ID_HDR(prev_string, pidflag);
		if (DISPLAY_MEM(actflag)) {
			printf("  minflt/s  majflt/s     VSZ    RSS   %%MEM");
		}
		if (DISPLAY_STACK(actflag)) {
			printf(" StkSize  StkRef");
		}
		printf("  Command\n");
	}
	
	for (p = 0; p < pid_nr; p++) {
	
		if ((rc = get_pid_to_display(prev, curr, p, P_A_MEM, P_TASK,
					     &pstc, &pstp)) == 0)
			/* PID no longer exists */
			continue;
	
		/* This will be used to compute average */
		if (!disp_avg) {
			if (DISPLAY_MEM(actflag)) {
				pstc->total_vsz = pstp->total_vsz + pstc->vsz;
				pstc->total_rss = pstp->total_rss + pstc->rss;
			}
			if (DISPLAY_STACK(actflag)) {
				pstc->total_stack_size = pstp->total_stack_size + pstc->stack_size;
				pstc->total_stack_ref  = pstp->total_stack_ref  + pstc->stack_ref;
			}
			pstc->rt_asum_count = pstp->rt_asum_count + 1;
		}

		if (rc < 0)
			/* PID should not be displayed */
			continue;

		print_line_id(curr_string, pstc);
		
		if (DISPLAY_MEM(actflag)) {
			printf(" %9.2f %9.2f ",
			       S_VALUE(pstp->minflt, pstc->minflt, itv),
			       S_VALUE(pstp->majflt, pstc->majflt, itv));

			if (disp_avg) {
				printf("%7.0f %6.0f %6.2f",
				       (double) pstc->total_vsz / pstc->rt_asum_count,
				       (double) pstc->total_rss / pstc->rt_asum_count,
				       tlmkb ?
				       SP_VALUE(0, pstc->total_rss / pstc->rt_asum_count, tlmkb)
				       : 0.0);
			}
			else {
				printf("%7lu %6lu %6.2f",
				       pstc->vsz,
				       pstc->rss,
				       tlmkb ? SP_VALUE(0, pstc->rss, tlmkb) : 0.0);
			}
		}
		
		if (DISPLAY_STACK(actflag)) {
			if (disp_avg) {
				printf("%7.0f %7.0f",
				       (double) pstc->total_stack_size / pstc->rt_asum_count,
				       (double) pstc->total_stack_ref  / pstc->rt_asum_count);
			}
			else {
				printf("%7lu %7lu",
				       pstc->stack_size,
				       pstc->stack_ref);
			}
		}

		print_comm(pstc);
		again = 1;
	}

	return again;
}

/*
 ***************************************************************************
 * Display memory statistics for tasks' children.
 *
 * IN:
 * @prev	Index in array where stats used as reference are.
 * @curr	Index in array for current sample statistics.
 * @dis		TRUE if a header line must be printed.
 * @disp_avg	TRUE if average stats are displayed.
 * @prev_string	String displayed at the beginning of a header line. This is
 * 		the timestamp of the previous sample, or "Average" when
 * 		displaying average stats.
 * @curr_string	String displayed at the beginning of current sample stats.
 * 		This is the timestamp of the current sample, or "Average"
 * 		when displaying average stats.
 *
 * RETURNS:
 * 0 if all the processes to display have terminated.
 * <> 0 if there are still some processes left to display.
 ***************************************************************************
 */
int write_pid_child_memory_stats(int prev, int curr, int dis, int disp_avg,
				 char *prev_string, char *curr_string)
{
	struct pid_stats *pstc, *pstp;
	unsigned int p;
	int rc, again = 0;

	if (dis) {
		PRINT_ID_HDR(prev_string, pidflag);
		printf(" minflt-nr majflt-nr  Command\n");
	}
	
	for (p = 0; p < pid_nr; p++) {
	
		if ((rc = get_pid_to_display(prev, curr, p, P_A_MEM, P_CHILD,
					     &pstc, &pstp)) == 0)
			/* PID no longer exists */
			continue;

		/* This will be used to compute average */
		if (!disp_avg) {
			pstc->rc_asum_count = pstp->rc_asum_count + 1;
		}
	
		if (rc < 0)
			/* PID should not be displayed */
			continue;

		print_line_id(curr_string, pstc);
		if (disp_avg) {
			printf(" %9.0f %9.0f",
			       (double) ((pstc->minflt + pstc->cminflt) -
					 (pstp->minflt + pstp->cminflt)) / pstc->rc_asum_count,
			       (double) ((pstc->majflt + pstc->cmajflt) -
					 (pstp->majflt + pstp->cmajflt)) / pstc->rc_asum_count);
		}
		else {
			printf(" %9lu %9lu",
			       (pstc->minflt + pstc->cminflt) - (pstp->minflt + pstp->cminflt),
			       (pstc->majflt + pstc->cmajflt) - (pstp->majflt + pstp->cmajflt));
		}
		print_comm(pstc);
		again = 1;
	}

	return again;
}

/*
 ***************************************************************************
 * Display I/O statistics.
 *
 * IN:
 * @prev	Index in array where stats used as reference are.
 * @curr	Index in array for current sample statistics.
 * @dis		TRUE if a header line must be printed.
 * @prev_string	String displayed at the beginning of a header line. This is
 * 		the timestamp of the previous sample, or "Average" when
 * 		displaying average stats.
 * @curr_string	String displayed at the beginning of current sample stats.
 * 		This is the timestamp of the current sample, or "Average"
 * 		when displaying average stats.
 * @itv		Interval of time in jiffies.
 *
 * RETURNS:
 * 0 if all the processes to display have terminated.
 * <> 0 if there are still some processes left to display.
 ***************************************************************************
 */
int write_pid_io_stats(int prev, int curr, int dis,
		       char *prev_string, char *curr_string,
		       unsigned long long itv)
{
	struct pid_stats *pstc, *pstp;
	unsigned int p;
	int again = 0;

	if (dis) {
		PRINT_ID_HDR(prev_string, pidflag);
		printf("   kB_rd/s   kB_wr/s kB_ccwr/s  Command\n");
	}

	for (p = 0; p < pid_nr; p++) {

		if (get_pid_to_display(prev, curr, p, P_A_IO, P_NULL,
				       &pstc, &pstp) <= 0)
			continue;
	
		print_line_id(curr_string, pstc);
		printf(" %9.2f %9.2f %9.2f",
		       S_VALUE(pstp->read_bytes,  pstc->read_bytes, itv)  / 1024,
		       S_VALUE(pstp->write_bytes, pstc->write_bytes, itv) / 1024,
		       S_VALUE(pstp->cancelled_write_bytes,
			       pstc->cancelled_write_bytes, itv) / 1024);
		print_comm(pstc);
		again = 1;
	}

	return again;
}

/*
 ***************************************************************************
 * Display context switches statistics.
 *
 * IN:
 * @prev	Index in array where stats used as reference are.
 * @curr	Index in array for current sample statistics.
 * @dis		TRUE if a header line must be printed.
 * @prev_string	String displayed at the beginning of a header line. This is
 * 		the timestamp of the previous sample, or "Average" when
 * 		displaying average stats.
 * @curr_string	String displayed at the beginning of current sample stats.
 * 		This is the timestamp of the current sample, or "Average"
 * 		when displaying average stats.
 * @itv		Interval of time in jiffies.
 *
 * RETURNS:
 * 0 if all the processes to display have terminated.
 * <> 0 if there are still some processes left to display.
 ***************************************************************************
 */
int write_pid_ctxswitch_stats(int prev, int curr, int dis,
			      char *prev_string, char *curr_string,
			      unsigned long long itv)
{
	struct pid_stats *pstc, *pstp;
	unsigned int p;
	int again = 0;

	if (dis) {
		PRINT_ID_HDR(prev_string, pidflag);
		printf("   cswch/s nvcswch/s  Command\n");
	}

	for (p = 0; p < pid_nr; p++) {
	
		if (get_pid_to_display(prev, curr, p, P_A_CTXSW, P_NULL,
				       &pstc, &pstp) <= 0)
			continue;
	
		print_line_id(curr_string, pstc);
		printf(" %9.2f %9.2f",
		       S_VALUE(pstp->nvcsw, pstc->nvcsw, itv),
		       S_VALUE(pstp->nivcsw, pstc->nivcsw, itv));
		print_comm(pstc);
		again = 1;
	}

	return again;
}

/*
 ***************************************************************************
 * Display statistics.
 *
 * IN:
 * @prev	Index in array where stats used as reference are.
 * @curr	Index in array for current sample statistics.
 * @dis		TRUE if a header line must be printed.
 * @disp_avg	TRUE if average stats are displayed.
 * @prev_string	String displayed at the beginning of a header line. This is
 * 		the timestamp of the previous sample, or "Average" when
 * 		displaying average stats.
 * @curr_string	String displayed at the beginning of current sample stats.
 * 		This is the timestamp of the current sample, or "Average"
 * 		when displaying average stats.
 *
 * RETURNS:
 * 0 if all the processes to display have terminated.
 * <> 0 if there are still some processes left to display.
 ***************************************************************************
 */
int write_stats_core(int prev, int curr, int dis, int disp_avg,
		     char *prev_string, char *curr_string)
{
	unsigned long long itv, g_itv;
	int again = 0;

	/* Test stdout */
	TEST_STDOUT(STDOUT_FILENO);

	/* g_itv is multiplied by the number of processors */
	g_itv = get_interval(uptime[prev], uptime[curr]);

	if (cpu_nr > 1) {
		/* SMP machines */
		itv = get_interval(uptime0[prev], uptime0[curr]);
	}
	else {
		/* UP machines */
		itv = g_itv;
	}

	if (DISPLAY_ONELINE(pidflag)) {
		if (DISPLAY_TASK_STATS(tskflag)) {
			again += write_pid_task_all_stats(prev, curr, dis,
						  	 itv, g_itv);
		}
		if (DISPLAY_CHILD_STATS(tskflag)) {
			again += write_pid_child_all_stats(prev, curr, dis, itv);
		}
	}
	else {
		/* Display CPU stats */
		if (DISPLAY_CPU(actflag)) {

			if (DISPLAY_TASK_STATS(tskflag)) {
				again += write_pid_task_cpu_stats(prev, curr, dis, disp_avg,
								  prev_string, curr_string,
								  itv, g_itv);
			}
			if (DISPLAY_CHILD_STATS(tskflag)) {
				again += write_pid_child_cpu_stats(prev, curr, dis, disp_avg,
								   prev_string, curr_string);
			}
		}

		/* Display memory and/or stack stats */
		if (DISPLAY_MEM(actflag) || DISPLAY_STACK(actflag)) {

			if (DISPLAY_TASK_STATS(tskflag)) {
				again += write_pid_task_memory_stats(prev, curr, dis, disp_avg,
								     prev_string, curr_string, itv);
			}
			if (DISPLAY_CHILD_STATS(tskflag) && DISPLAY_MEM(actflag)) {
				again += write_pid_child_memory_stats(prev, curr, dis, disp_avg,
								      prev_string, curr_string);
			}
		}

		/* Display I/O stats */
		if (DISPLAY_IO(actflag)) {
			again += write_pid_io_stats(prev, curr, dis, prev_string,
						    curr_string, itv);
		}

		/* Display context switches stats */
		if (DISPLAY_CTXSW(actflag)) {
			again += write_pid_ctxswitch_stats(prev, curr, dis, prev_string,
							   curr_string, itv);
		}
	}

	if (DISPLAY_ALL_PID(pidflag)) {
		again = 1;
	}

	return again;
}

/*
 ***************************************************************************
 * Print statistics average.
 *
 * IN:
 * @curr	Index in array for current sample statistics.
 * @dis		TRUE if a header line must be printed.
 ***************************************************************************
 */
void write_stats_avg(int curr, int dis)
{
	char string[16];

	strncpy(string, _("Average:"), 16);
	string[15] = '\0';
	write_stats_core(2, curr, dis, TRUE, string, string);
}

/*
 ***************************************************************************
 * Get previous and current timestamps, then display statistics.
 *
 * IN:
 * @curr	Index in array for current sample statistics.
 * @dis		TRUE if a header line must be printed.
 *
 * RETURNS:
 * 0 if all the processes to display have terminated.
 * <> 0 if there are still some processes left to display.
 ***************************************************************************
 */
int write_stats(int curr, int dis)
{
	char cur_time[2][16];

	/* Get previous timestamp */
	strftime(cur_time[!curr], 16, "%X", &ps_tstamp[!curr]);

	/* Get current timestamp */
	strftime(cur_time[curr], 16, "%X", &ps_tstamp[curr]);

	return (write_stats_core(!curr, curr, dis, FALSE,
				 cur_time[!curr], cur_time[curr]));
}

/*
 ***************************************************************************
 * Main loop: Read and display PID stats.
 *
 * IN:
 * @dis_hdr	Set to TRUE if the header line must always be printed.
 * @rows	Number of rows of screen.
 ***************************************************************************
 */
void rw_pidstat_loop(int dis_hdr, int rows)
{
	int curr = 1, dis = 1;
	int again;
	unsigned long lines = rows;

	/* Don't buffer data if redirected to a pipe */
	setbuf(stdout, NULL);
	
	if (cpu_nr > 1) {
		/*
		 * Read system uptime (only for SMP machines).
		 * Init uptime0. So if /proc/uptime cannot fill it, this will be
		 * done by /proc/stat.
		 */
		uptime0[0] = 0;
		read_uptime(&uptime0[0]);
	}
	read_stats(0);

	if (DISPLAY_MEM(actflag)) {
		/* Get total memory */
		read_proc_meminfo();
	}

	if (!interval) {
		/* Display since boot time */
		ps_tstamp[1] = ps_tstamp[0];
		memset(st_pid_list[1], 0, PID_STATS_SIZE * pid_nr);
		write_stats(0, DISP_HDR);
		exit(0);
	}

	/* Set a handler for SIGALRM */
	alarm_handler(0);

	/* Save the first stats collected. Will be used to compute the average */
	ps_tstamp[2] = ps_tstamp[0];
	uptime[2] = uptime[0];
	uptime0[2] = uptime0[0];
	memcpy(st_pid_list[2], st_pid_list[0], PID_STATS_SIZE * pid_nr);

	pause();

	do {
		/* Get time */
		get_localtime(&ps_tstamp[curr]);

		if (cpu_nr > 1) {
			/*
			 * Read system uptime (only for SMP machines).
			 * Init uptime0. So if /proc/uptime cannot fill it, this will be
			 * done by /proc/stat.
			 */
			uptime0[curr] = 0;
			read_uptime(&(uptime0[curr]));
		}

		/* Read stats */
		read_stats(curr);

		if (!dis_hdr) {
			dis = lines / rows;
			if (dis) {
				lines %= rows;
			}
			lines++;
		}

		/* Print results */
		again = write_stats(curr, dis);

		if (!again)
			return;

		if (count > 0) {
			count--;
		}

		if (count) {
			curr ^= 1;
			pause();
		}
	}
	while (count);

	/*
	 * The one line format uses a raw time value rather than time strings
	 * so the average doesn't really fit.
	 */
	if (!DISPLAY_ONELINE(pidflag))
	{
		/* Write stats average */
		write_stats_avg(curr, dis_hdr);
	}
}

/*
 ***************************************************************************
 * Main entry to the pidstat program.
 ***************************************************************************
 */
int main(int argc, char **argv)
{
	int opt = 1, dis_hdr = -1;
	int i;
	unsigned int pid;
	struct utsname header;
	int rows = 23;
	char *t;

#ifdef USE_NLS
	/* Init National Language Support */
	init_nls();
#endif

	/* Get HZ */
	get_HZ();

	/* Compute page shift in kB */
	get_kb_shift();
	
	/* Allocate structures for device list */
	if (argc > 1) {
		salloc_pid_array((argc / 2) + count_csvalues(argc, argv));
	}

	/* Process args... */
	while (opt < argc) {

		if (!strcmp(argv[opt], "-p")) {
			pidflag |= P_D_PID;
			if (argv[++opt]) {

				for (t = strtok(argv[opt], ","); t; t = strtok(NULL, ",")) {
					if (!strcmp(t, K_ALL)) {
						pidflag |= P_D_ALL_PID;
					}
					else if (!strcmp(t, K_SELF)) {
						update_pid_array(&pid_array_nr, getpid());
					}
					else {
						if (strspn(t, DIGITS) != strlen(t)) {
							usage(argv[0]);
						}
						pid = atoi(t);
						if (pid < 1) {
							usage(argv[0]);
						}
						update_pid_array(&pid_array_nr, pid);
					}
				}
				opt++;
			}
			else {
				usage(argv[0]);
			}
		}

		else if (!strcmp(argv[opt], "-C")) {
			if (argv[++opt]) {
				strncpy(commstr, argv[opt++], MAX_COMM_LEN);
				commstr[MAX_COMM_LEN - 1] = '\0';
				pidflag |= P_F_COMMSTR;
				if (!strlen(commstr)) {
					usage(argv[0]);
				}
			}
			else {
				usage(argv[0]);
			}
		}

		else if (!strcmp(argv[opt], "-T")) {
			if (argv[++opt]) {
				if (tskflag) {
					dis_hdr++;
				}
				if (!strcmp(argv[opt], K_P_TASK)) {
					tskflag |= P_TASK;
				}
				else if (!strcmp(argv[opt], K_P_CHILD)) {
					tskflag |= P_CHILD;
				}
				else if (!strcmp(argv[opt], K_P_ALL)) {
					tskflag |= P_TASK + P_CHILD;
					dis_hdr++;
				}
				else {
					usage(argv[0]);
				}
				opt++;
			}
			else {
				usage(argv[0]);
			}
		}

		else if (!strncmp(argv[opt], "-", 1)) {
			for (i = 1; *(argv[opt] + i); i++) {

				switch (*(argv[opt] + i)) {

				case 'd':
					/* Display I/O usage */
					actflag |= P_A_IO;
					dis_hdr++;
					break;
					
				case 'h':
					/* Display stats on one line */
					pidflag |= P_D_ONELINE;
					break;

				case 'I':
					/* IRIX mode off */
					pidflag |= P_F_IRIX_MODE;
					break;
					
				case 'l':
					/* Display whole command line */
					pidflag |= P_D_CMDLINE;
					break;
	
				case 'r':
					/* Display memory usage */
					actflag |= P_A_MEM;
					dis_hdr++;
					break;
					
				case 's':
					/* Display stack sizes */
					actflag |= P_A_STACK;
					dis_hdr++;
					break;
	
				case 't':
					/* Display stats for threads */
					pidflag |= P_D_TID;
					break;

				case 'u':
					/* Display CPU usage */
					actflag |= P_A_CPU;
					dis_hdr++;
					break;

				case 'V':
					/* Print version number and exit */
					print_version();
					break;

				case 'w':
					/* Display context switches */
					actflag |= P_A_CTXSW;
					dis_hdr++;
					break;
	
				default:
					usage(argv[0]);
				}
			}
			opt++;
		}

		else if (interval < 0) {	/* Get interval */
			if (strspn(argv[opt], DIGITS) != strlen(argv[opt])) {
				usage(argv[0]);
			}
			interval = atol(argv[opt++]);
			if (interval < 0) {
				usage(argv[0]);
			}
			count = -1;
		}

		else if (count <= 0) {	/* Get count value */
			if ((strspn(argv[opt], DIGITS) != strlen(argv[opt])) ||
			    !interval) {
				usage(argv[0]);
			}
			count = atol(argv[opt++]);
			if (count < 1) {
				usage(argv[0]);
			}
		}
		else {
			usage(argv[0]);
		}
	}

	if (interval < 0) {
		/* Interval not set => display stats since boot time */
		interval = 0;
	}

	/* Check flags and set default values */
	check_flags();

	/* Init structures */
	pid_sys_init();

	if (dis_hdr < 0) {
		dis_hdr = 0;
	}
	if (!dis_hdr) {
		if (pid_nr > 1) {
			dis_hdr = 1;
		}
		else {
			rows = get_win_height();
		}
	}

	/* Get time */
	get_localtime(&(ps_tstamp[0]));

	/* Get system name, release number and hostname */
	uname(&header);
	print_gal_header(&(ps_tstamp[0]), header.sysname, header.release,
			 header.nodename, header.machine, cpu_nr);

	/* Main loop */
	rw_pidstat_loop(dis_hdr, rows);
	
	/* Free structures */
	if (pid_array) {
		free(pid_array);
	}
	sfree_pid();

	return 0;
}
