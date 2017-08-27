/*
 * mpstat: per-processor statistics
 * (C) 2000-2011 by Sebastien GODARD (sysstat <at> orange.fr)
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
#include <errno.h>
#include <ctype.h>
#include <sys/utsname.h>

#include "version.h"
#include "mpstat.h"
#include "common.h"
#include "rd_stats.h"

#ifdef USE_NLS
#include <locale.h>
#include <libintl.h>
#define _(string) gettext(string)
#else
#define _(string) (string)
#endif

#define SCCSID "@(#)sysstat-" VERSION ": "  __FILE__ " compiled " __DATE__ " " __TIME__
char *sccsid(void) { return (SCCSID); }

unsigned long long uptime[3] = {0, 0, 0};
unsigned long long uptime0[3] = {0, 0, 0};

/* NOTE: Use array of _char_ for bitmaps to avoid endianness problems...*/
unsigned char *cpu_bitmap;	/* Bit 0: Global; Bit 1: 1st proc; etc. */

/* Structures used to store stats */
struct stats_cpu *st_cpu[3];
struct stats_irq *st_irq[3];
struct stats_irqcpu *st_irqcpu[3];
struct stats_irqcpu *st_softirqcpu[3];

struct tm mp_tstamp[3];

/* Activity flag */
unsigned int actflags = 0;

unsigned int flags = 0;

/* Interval and count parameters */
long interval = -1, count = 0;

/* Nb of processors on the machine */
int cpu_nr = 0;
/* Nb of interrupts per processor */
int irqcpu_nr = 0;
/* Nb of soft interrupts per processor */
int softirqcpu_nr = 0;

/*
 ***************************************************************************
 * Print usage and exit
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
			  "[ -A ] [ -I { SUM | CPU | SCPU | ALL } ] [ -u ]\n"
			  "[ -P { <cpu> [,...] | ON | ALL } ] [ -V ]\n"));
	exit(1);
}

/*
 ***************************************************************************
 * SIGALRM signal handler
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
 * Allocate stats structures and cpu bitmap.
 *
 * IN:
 * @nr_cpus	Number of CPUs. This is the real number of available CPUs + 1
 * 		because we also have to allocate a structure for CPU 'all'.
 ***************************************************************************
 */
void salloc_mp_struct(int nr_cpus)
{
	int i;

	for (i = 0; i < 3; i++) {

		if ((st_cpu[i] = (struct stats_cpu *) malloc(STATS_CPU_SIZE * nr_cpus))
		    == NULL) {
			perror("malloc");
			exit(4);
		}
		memset(st_cpu[i], 0, STATS_CPU_SIZE * nr_cpus);

		if ((st_irq[i] = (struct stats_irq *) malloc(STATS_IRQ_SIZE * nr_cpus))
		    == NULL) {
			perror("malloc");
			exit(4);
		}
		memset(st_irq[i], 0, STATS_IRQ_SIZE * nr_cpus);

		if ((st_irqcpu[i] = (struct stats_irqcpu *) malloc(STATS_IRQCPU_SIZE * nr_cpus * irqcpu_nr))
		    == NULL) {
			perror("malloc");
			exit(4);
		}
		memset(st_irqcpu[i], 0, STATS_IRQCPU_SIZE * nr_cpus * irqcpu_nr);

		if ((st_softirqcpu[i] = (struct stats_irqcpu *) malloc(STATS_IRQCPU_SIZE * nr_cpus * softirqcpu_nr))
		     == NULL) {
			perror("malloc");
			exit(4);
		}
		memset(st_softirqcpu[i], 0, STATS_IRQCPU_SIZE * nr_cpus * softirqcpu_nr);
	}

	if ((cpu_bitmap = (unsigned char *) malloc((nr_cpus >> 3) + 1)) == NULL) {
		perror("malloc");
		exit(4);
	}
	memset(cpu_bitmap, 0, (nr_cpus >> 3) + 1);
}

/*
 ***************************************************************************
 * Free structures and bitmap.
 ***************************************************************************
 */
void sfree_mp_struct(void)
{
	int i;

	for (i = 0; i < 3; i++) {

		if (st_cpu[i]) {
			free(st_cpu[i]);
		}
		if (st_irq[i]) {
			free(st_irq[i]);
		}
		if (st_irqcpu[i]) {
			free(st_irqcpu[i]);
		}
		if (st_softirqcpu[i]) {
			free(st_softirqcpu[i]);
		}
	}

	if (cpu_bitmap) {
		free(cpu_bitmap);
	}
}

/*
 ***************************************************************************
 * Display per CPU statistics.
 *
 * IN:
 * @st_ic	Array for per-CPU statistics.
 * @ic_nr	Number of interrupts (hard or soft) per CPU.
 * @dis		TRUE if a header line must be printed.
 * @itv		Interval value.
 * @prev	Position in array where statistics used	as reference are.
 *		Stats used as reference may be the previous ones read, or
 *		the very first ones when calculating the average.
 * @curr	Position in array where current statistics will be saved.
 * @prev_string	String displayed at the beginning of a header line. This is
 * 		the timestamp of the previous sample, or "Average" when
 * 		displaying average stats.
 * @curr_string	String displayed at the beginning of current sample stats.
 * 		This is the timestamp of the current sample, or "Average"
 * 		when displaying average stats.
 ***************************************************************************
 */
void write_irqcpu_stats(struct stats_irqcpu *st_ic[], int ic_nr, int dis,
			unsigned long long itv, int prev, int curr,
			char *prev_string, char *curr_string)
{
	struct stats_cpu *scc, *scp;
	int j = 0, offset, cpu;
	struct stats_irqcpu *p, *q, *p0, *q0;

	/*
	* Check if number of interrupts has changed.
	* NB: A null interval value indicates that we are
	 * displaying statistics since system startup.
	 */
	if (!dis && interval) {
		do {
			p0 = st_ic[curr] + j;
			if (p0->irq_name[0] != '\0') {
				q0 = st_ic[prev] + j;
				if (strcmp(p0->irq_name, q0->irq_name)) {
					/* These are two different irq */
					j = -2;
				}
			}
			j++;
		}
		while ((j > 0) && (j <= ic_nr));
	}

	if (dis || (j < 0)) {
		/* Print header */
		printf("\n%-11s  CPU", prev_string);
		for (j = 0; j < ic_nr; j++) {
			p0 = st_ic[curr] + j;
			if (p0->irq_name[0] != '\0') {	/* Nb of irq per proc may have varied... */
				printf(" %8s/s", p0->irq_name);
			}
		}
		printf("\n");
	}

	for (cpu = 1; cpu <= cpu_nr; cpu++) {

		scc = st_cpu[curr] + cpu;
		scp = st_cpu[prev] + cpu;

		/*
		 * Check if we want stats about this CPU.
		 * CPU must have been explicitly selected using option -P,
		 * else we display every CPU.
		 */
		if (!(*(cpu_bitmap + (cpu >> 3)) & (1 << (cpu & 0x07))) && USE_P_OPTION(flags))
			continue;

		if ((scc->cpu_user    + scc->cpu_nice + scc->cpu_sys   +
		     scc->cpu_iowait  + scc->cpu_idle + scc->cpu_steal +
		     scc->cpu_hardirq + scc->cpu_softirq) == 0) {

			/* Offline CPU found */

			if (DISPLAY_ONLINE_CPU(flags))
				continue;
		}

		printf("%-11s  %3d", curr_string, cpu - 1);

		for (j = 0; j < ic_nr; j++) {
			p0 = st_ic[curr] + j;	/* irq field set only for proc #0 */
			/*
			 * An empty string for irq name means it is a remaining interrupt
			 * which is no longer used, for example because the
			 * number of interrupts has decreased in /proc/interrupts.
			 */
			if (p0->irq_name[0] != '\0') {
				q0 = st_ic[prev] + j;
				offset = j;

				/*
				 * If we want stats for the time since system startup,
				 * we have p0->irq != q0->irq, since q0 structure is
				 * completely set to zero.
				 */
				if (strcmp(p0->irq_name, q0->irq_name) && interval) {
					if (j)
						offset = j - 1;
					q0 = st_ic[prev] + offset;
					if (strcmp(p0->irq_name, q0->irq_name) && (j + 1 < ic_nr))
						offset = j + 1;
					q0 = st_ic[prev] + offset;
				}

				if (!strcmp(p0->irq_name, q0->irq_name) || !interval) {
					p = st_ic[curr] + (cpu - 1) * ic_nr + j;
					q = st_ic[prev] + (cpu - 1) * ic_nr + offset;
					printf(" %10.2f",
					       S_VALUE(q->interrupt, p->interrupt, itv));
				}
				else
					printf("        N/A");
			}
		}
		printf("\n");
	}
}

/*
 ***************************************************************************
 * Core function used to display statistics.
 *
 * IN:
 * @prev	Position in array where statistics used	as reference are.
 *		Stats used as reference may be the previous ones read, or
 *		the very first ones when calculating the average.
 * @curr	Position in array where statistics for current sample are.
 * @dis		TRUE if a header line must be printed.
 * @prev_string	String displayed at the beginning of a header line. This is
 * 		the timestamp of the previous sample, or "Average" when
 * 		displaying average stats.
 * @curr_string	String displayed at the beginning of current sample stats.
 * 		This is the timestamp of the current sample, or "Average"
 * 		when displaying average stats.
 ***************************************************************************
 */
void write_stats_core(int prev, int curr, int dis,
		      char *prev_string, char *curr_string)
{
	struct stats_cpu *scc, *scp;
	unsigned long long itv, pc_itv, g_itv;
	int cpu;

	/* Test stdout */
	TEST_STDOUT(STDOUT_FILENO);

	/* Compute time interval */
	g_itv = get_interval(uptime[prev], uptime[curr]);

	/* Reduce interval value to one processor */
	if (cpu_nr > 1) {
		itv = get_interval(uptime0[prev], uptime0[curr]);
	}
	else {
		itv = g_itv;
	}

	/* Print CPU stats */
	if (DISPLAY_CPU(actflags)) {
		if (dis) {
			printf("\n%-11s  CPU    %%usr   %%nice    %%sys %%iowait    %%irq   "
			       "%%soft  %%steal  %%guest   %%idle\n",
			       prev_string);
		}

		/* Check if we want global stats among all proc */
		if (*cpu_bitmap & 1) {

			printf("%-11s  all", curr_string);

			printf("  %6.2f  %6.2f  %6.2f  %6.2f  %6.2f  %6.2f  %6.2f  %6.2f  %6.2f\n",
			       (st_cpu[curr]->cpu_user - st_cpu[curr]->cpu_guest) <
			       (st_cpu[prev]->cpu_user - st_cpu[prev]->cpu_guest) ?
			       0.0 :
			       ll_sp_value(st_cpu[prev]->cpu_user - st_cpu[prev]->cpu_guest,
					   st_cpu[curr]->cpu_user - st_cpu[curr]->cpu_guest,
					   g_itv),
			       ll_sp_value(st_cpu[prev]->cpu_nice,
					   st_cpu[curr]->cpu_nice,
					   g_itv),
			       ll_sp_value(st_cpu[prev]->cpu_sys,
					   st_cpu[curr]->cpu_sys,
					   g_itv),
			       ll_sp_value(st_cpu[prev]->cpu_iowait,
					   st_cpu[curr]->cpu_iowait,
					   g_itv),
			       ll_sp_value(st_cpu[prev]->cpu_hardirq,
					   st_cpu[curr]->cpu_hardirq,
					   g_itv),
			       ll_sp_value(st_cpu[prev]->cpu_softirq,
					   st_cpu[curr]->cpu_softirq,
					   g_itv),
			       ll_sp_value(st_cpu[prev]->cpu_steal,
					   st_cpu[curr]->cpu_steal,
					   g_itv),
			       ll_sp_value(st_cpu[prev]->cpu_guest,
					   st_cpu[curr]->cpu_guest,
					   g_itv),
			       (st_cpu[curr]->cpu_idle < st_cpu[prev]->cpu_idle) ?
			       0.0 :
			       ll_sp_value(st_cpu[prev]->cpu_idle,
					   st_cpu[curr]->cpu_idle,
					   g_itv));
		}

		for (cpu = 1; cpu <= cpu_nr; cpu++) {

			scc = st_cpu[curr] + cpu;
			scp = st_cpu[prev] + cpu;

			/* Check if we want stats about this proc */
			if (!(*(cpu_bitmap + (cpu >> 3)) & (1 << (cpu & 0x07))))
				continue;
			
			/*
			 * If the CPU is offline then it is omited from /proc/stat
			 * and the sum of all values is zero.
			 * (Remember that guest time is already included in user mode.)
			 */
			if ((scc->cpu_user    + scc->cpu_nice + scc->cpu_sys   +
			     scc->cpu_iowait  + scc->cpu_idle + scc->cpu_steal +
			     scc->cpu_hardirq + scc->cpu_softirq) == 0) {

				if (!DISPLAY_ONLINE_CPU(flags)) {
					printf("%-11s %4d"
					       "  %6.2f  %6.2f  %6.2f  %6.2f  %6.2f  %6.2f"
					       "  %6.2f  %6.2f  %6.2f\n",
					       curr_string, cpu - 1,
					       0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
				}
				continue;
			}

			printf("%-11s %4d", curr_string, cpu - 1);

			/* Recalculate itv for current proc */
			pc_itv = get_per_cpu_interval(scc, scp);
			
			if (!pc_itv) {
				/*
				 * If the CPU is tickless then there is no change in CPU values
				 * but the sum of values is not zero.
				 */
				printf("  %6.2f  %6.2f  %6.2f  %6.2f  %6.2f  %6.2f"
				       "  %6.2f  %6.2f  %6.2f\n",
				       0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 100.0);
			}

			else {
				printf("  %6.2f  %6.2f  %6.2f  %6.2f  %6.2f  %6.2f"
				       "  %6.2f  %6.2f  %6.2f\n",
				       (scc->cpu_user - scc->cpu_guest) < (scp->cpu_user - scp->cpu_guest) ?
				       0.0 :
				       ll_sp_value(scp->cpu_user - scp->cpu_guest,
						   scc->cpu_user - scc->cpu_guest,
						   pc_itv),
				       ll_sp_value(scp->cpu_nice,
						   scc->cpu_nice,
						   pc_itv),
				       ll_sp_value(scp->cpu_sys,
						   scc->cpu_sys,
						   pc_itv),
				       ll_sp_value(scp->cpu_iowait,
						   scc->cpu_iowait,
						   pc_itv),
				       ll_sp_value(scp->cpu_hardirq,
						   scc->cpu_hardirq,
						   pc_itv),
				       ll_sp_value(scp->cpu_softirq,
						   scc->cpu_softirq,
						   pc_itv),
				       ll_sp_value(scp->cpu_steal,
						   scc->cpu_steal,
						   pc_itv),
				       ll_sp_value(scp->cpu_guest,
						   scc->cpu_guest,
						   pc_itv),
				       (scc->cpu_idle < scp->cpu_idle) ?
				       0.0 :
				       ll_sp_value(scp->cpu_idle,
						   scc->cpu_idle,
						   pc_itv));
			}
		}
	}

	/* Print total number of interrupts per processor */
	if (DISPLAY_IRQ_SUM(actflags)) {
		struct stats_irq *sic, *sip;

		if (dis) {
			printf("\n%-11s  CPU    intr/s\n", prev_string);
		}

		if (*cpu_bitmap & 1) {
			printf("%-11s  all %9.2f\n", curr_string,
			       ll_s_value(st_irq[prev]->irq_nr, st_irq[curr]->irq_nr, itv));
		}

		for (cpu = 1; cpu <= cpu_nr; cpu++) {

			sic = st_irq[curr] + cpu;
			sip = st_irq[prev] + cpu;

			scc = st_cpu[curr] + cpu;
			scp = st_cpu[prev] + cpu;

			/* Check if we want stats about this proc */
			if (!(*(cpu_bitmap + (cpu >> 3)) & (1 << (cpu & 0x07))))
				continue;

			if ((scc->cpu_user    + scc->cpu_nice + scc->cpu_sys   +
			     scc->cpu_iowait  + scc->cpu_idle + scc->cpu_steal +
			     scc->cpu_hardirq + scc->cpu_softirq) == 0) {

				/* This is an offline CPU */
				
				if (!DISPLAY_ONLINE_CPU(flags)) {
					printf("%-11s %4d"
					       "  %9.2f\n",
					       curr_string, cpu - 1,
					       0.0);
				}
				continue;
			}

			printf("%-11s %4d", curr_string, cpu - 1);

			/* Recalculate itv for current proc */
			pc_itv = get_per_cpu_interval(scc, scp);

			if (!pc_itv) {
				/* This is a tickless CPU */
				printf(" %9.2f\n", 0.0);
			}
			else {
				printf(" %9.2f\n",
				       ll_s_value(sip->irq_nr, sic->irq_nr, itv));
			}
		}
	}

	if (DISPLAY_IRQ_CPU(actflags)) {
		write_irqcpu_stats(st_irqcpu, irqcpu_nr, dis, itv, prev, curr,
				   prev_string, curr_string);
	}

	if (DISPLAY_SOFTIRQS(actflags)) {
		write_irqcpu_stats(st_softirqcpu, softirqcpu_nr, dis, itv, prev, curr,
				   prev_string, curr_string);
	}

	/* Fix CPU counter values for every offline CPU */
	for (cpu = 1; cpu <= cpu_nr; cpu++) {

		scc = st_cpu[curr] + cpu;
		scp = st_cpu[prev] + cpu;

		if ((scc->cpu_user    + scc->cpu_nice + scc->cpu_sys   +
		     scc->cpu_iowait  + scc->cpu_idle + scc->cpu_steal +
		     scc->cpu_hardirq + scc->cpu_softirq) == 0) {
			/*
			 * Offline CPU found.
			 * Set current struct fields (which have been set to zero)
			 * to values from previous iteration. Hence their values won't
			 * jump from zero when the CPU comes back online.
			 */
			*scc = *scp;
		}
	}
}

/*
 ***************************************************************************
 * Print statistics average.
 *
 * IN:
 * @curr	Position in array where statistics for current sample are.
 * @dis		TRUE if a header line must be printed.
 ***************************************************************************
 */
void write_stats_avg(int curr, int dis)
{
	char string[16];

	strncpy(string, _("Average:"), 16);
	string[15] = '\0';
	write_stats_core(2, curr, dis, string, string);
}

/*
 ***************************************************************************
 * Print statistics.
 *
 * IN:
 * @curr	Position in array where statistics for current sample are.
 * @dis		TRUE if a header line must be printed.
 ***************************************************************************
 */
void write_stats(int curr, int dis)
{
	char cur_time[2][16];

	/* Get previous timestamp */
	strftime(cur_time[!curr], 16, "%X", &(mp_tstamp[!curr]));

	/* Get current timestamp */
	strftime(cur_time[curr], 16, "%X", &(mp_tstamp[curr]));

	write_stats_core(!curr, curr, dis, cur_time[!curr], cur_time[curr]);
}

/*
 ***************************************************************************
 * Read stats from /proc/interrupts or /proc/softirqs.
 *
 * IN:
 * @file	/proc file to read (interrupts or softirqs).
 * @ic_nr	Number of interrupts (hard or soft) per CPU.
 * @curr	Position in array where current statistics will be saved.
 *
 * OUT:
 * @st_ic	Array for per-CPU statistics.
 ***************************************************************************
 */
void read_interrupts_stat(char *file, struct stats_irqcpu *st_ic[], int ic_nr, int curr)
{
	FILE *fp;
	struct stats_irq *st_irq_i;
	struct stats_irqcpu *p;
	char *line = NULL;
	unsigned long irq = 0;
	unsigned int cpu;
	int cpu_index[cpu_nr], index = 0, dgt, len;
	char *cp, *next;

	for (cpu = 0; cpu < cpu_nr; cpu++) {
		st_irq_i = st_irq[curr] + cpu + 1;
		st_irq_i->irq_nr = 0;
	}

	if ((fp = fopen(file, "r")) != NULL) {

		SREALLOC(line, char, INTERRUPTS_LINE + 11 * cpu_nr);

		/*
		 * Parse header line to see which CPUs are online
		 */
		while (fgets(line, INTERRUPTS_LINE + 11 * cpu_nr, fp) != NULL) {
			next = line;
			while (((cp = strstr(next, "CPU")) != NULL) && (index < cpu_nr)) {
				cpu = strtol(cp + 3, &next, 10);
				cpu_index[index++] = cpu;
			}
			if (index)
				/* Header line found */
				break;
		}

		while ((fgets(line, INTERRUPTS_LINE + 11 * cpu_nr, fp) != NULL) &&
		       (irq < ic_nr)) {

			/* Skip over "<irq>:" */
			if ((cp = strchr(line, ':')) == NULL)
				continue;
			cp++;

			p = st_ic[curr] + irq;
			len = strcspn(line, ":");
			if (len >= MAX_IRQ_LEN) {
				len = MAX_IRQ_LEN - 1;
			}
			strncpy(p->irq_name, line, len);
			p->irq_name[len] = '\0';
			dgt = isdigit(line[len - 1]);

			for (cpu = 0; cpu < index; cpu++) {
				p = st_ic[curr] + cpu_index[cpu] * ic_nr + irq;
				st_irq_i = st_irq[curr] + cpu_index[cpu] + 1;
				/*
				 * No need to set (st_irqcpu + cpu * irqcpu_nr)->irq:
				 * This is the same as st_irqcpu->irq.
				 */
				p->interrupt = strtoul(cp, &next, 10);
				if (dgt) {
					/* Sum only numerical irq (and not NMI, LOC, etc.) */
					st_irq_i->irq_nr += p->interrupt;
				}
				cp = next;
			}
			irq++;
		}

		fclose(fp);
		
		if (line) {
			free(line);
		}
	}

	while (irq < ic_nr) {
		/* Nb of interrupts per processor has changed */
		p = st_ic[curr] + irq;
		p->irq_name[0] = '\0';	/* This value means this is a dummy interrupt */
		irq++;
	}
}

/*
 ***************************************************************************
 * Main loop: Read stats from the relevant sources, and display them.
 *
 * IN:
 * @dis_hdr	Set to TRUE if the header line must always be printed.
 * @rows	Number of rows of screen.
 ***************************************************************************
 */
void rw_mpstat_loop(int dis_hdr, int rows)
{
	struct stats_cpu *scc;
	int cpu;
	int curr = 1, dis = 1;
	unsigned long lines = rows;

	/* Dont buffer data if redirected to a pipe */
	setbuf(stdout, NULL);

	/* Read stats */
	if (cpu_nr > 1) {
		/*
		 * Init uptime0. So if /proc/uptime cannot fill it,
		 * this will be done by /proc/stat.
		 */
		uptime0[0] = 0;
		read_uptime(&(uptime0[0]));
	}
	read_stat_cpu(st_cpu[0], cpu_nr + 1, &(uptime[0]), &(uptime0[0]));

	if (DISPLAY_IRQ_SUM(actflags)) {
		read_stat_irq(st_irq[0], 1);
	}

	if (DISPLAY_IRQ_SUM(actflags) || DISPLAY_IRQ_CPU(actflags)) {
		/* Read this file to display int per CPU or total nr of int per CPU */
		read_interrupts_stat(INTERRUPTS, st_irqcpu, irqcpu_nr, 0);
	}
	
	if (DISPLAY_SOFTIRQS(actflags)) {
		read_interrupts_stat(SOFTIRQS, st_softirqcpu, softirqcpu_nr, 0);
	}

	if (!interval) {
		/* Display since boot time */
		mp_tstamp[1] = mp_tstamp[0];
		memset(st_cpu[1], 0, STATS_CPU_SIZE * (cpu_nr + 1));
		memset(st_irq[1], 0, STATS_IRQ_SIZE * (cpu_nr + 1));
		memset(st_irqcpu[1], 0, STATS_IRQCPU_SIZE * (cpu_nr + 1) * irqcpu_nr);
		if (DISPLAY_SOFTIRQS(actflags)) {
			memset(st_softirqcpu[1], 0, STATS_IRQCPU_SIZE * (cpu_nr + 1) * softirqcpu_nr);
		}
		write_stats(0, DISP_HDR);
		exit(0);
	}

	/* Set a handler for SIGALRM */
	alarm_handler(0);

	/* Save the first stats collected. Will be used to compute the average */
	mp_tstamp[2] = mp_tstamp[0];
	uptime[2] = uptime[0];
	uptime0[2] = uptime0[0];
	memcpy(st_cpu[2], st_cpu[0], STATS_CPU_SIZE * (cpu_nr + 1));
	memcpy(st_irq[2], st_irq[0], STATS_IRQ_SIZE * (cpu_nr + 1));
	memcpy(st_irqcpu[2], st_irqcpu[0], STATS_IRQCPU_SIZE * (cpu_nr + 1) * irqcpu_nr);
	if (DISPLAY_SOFTIRQS(actflags)) {
		memcpy(st_softirqcpu[2], st_softirqcpu[0],
		       STATS_IRQCPU_SIZE * (cpu_nr + 1) * softirqcpu_nr);
	}

	pause();

	do {
		/*
		 * Resetting the structure not needed since every fields will be set.
		 * Exceptions are per-CPU structures: Some of them may not be filled
		 * if corresponding processor is disabled (offline). We set them to zero
		 * to be able to distinguish between offline and tickless CPUs.
		 */
		for (cpu = 1; cpu <= cpu_nr; cpu++) {
			scc = st_cpu[curr] + cpu;
			memset(scc, 0, STATS_CPU_SIZE);
		}

		/* Get time */
		get_localtime(&(mp_tstamp[curr]));

		/* Read stats */
		if (cpu_nr > 1) {
			uptime0[curr] = 0;
			read_uptime(&(uptime0[curr]));
		}
		read_stat_cpu(st_cpu[curr], cpu_nr + 1, &(uptime[curr]), &(uptime0[curr]));

		if (DISPLAY_IRQ_SUM(actflags)) {
			read_stat_irq(st_irq[curr], 1);
		}

		if (DISPLAY_IRQ_SUM(actflags) || DISPLAY_IRQ_CPU(actflags)) {
			read_interrupts_stat(INTERRUPTS, st_irqcpu, irqcpu_nr, curr);
		}

		if (DISPLAY_SOFTIRQS(actflags)) {
			read_interrupts_stat(SOFTIRQS, st_softirqcpu, softirqcpu_nr, curr);
		}

		/* Write stats */
		if (!dis_hdr) {
			dis = lines / rows;
			if (dis) {
				lines %= rows;
			}
			lines++;
		}
		write_stats(curr, dis);

		if (count > 0) {
			count--;
		}
		if (count) {
			curr ^= 1;
			pause();
		}
	}
	while (count);

	/* Write stats average */
	write_stats_avg(curr, dis_hdr);
}

/*
 ***************************************************************************
 * Main entry to the program
 ***************************************************************************
 */
int main(int argc, char **argv)
{
	int opt = 0, i, actset = FALSE;
	struct utsname header;
	int dis_hdr = -1;
	int rows = 23;
	char *t;

#ifdef USE_NLS
	/* Init National Language Support */
	init_nls();
#endif

	/* Get HZ */
	get_HZ();

	/* How many processors on this machine ? */
	cpu_nr = get_cpu_nr(~0);
	
	/* Calculate number of interrupts per processor */
	irqcpu_nr = get_irqcpu_nr(INTERRUPTS, NR_IRQS, cpu_nr) +
		    NR_IRQCPU_PREALLOC;
	/* Calculate number of soft interrupts per processor */
	softirqcpu_nr = get_irqcpu_nr(SOFTIRQS, NR_IRQS, cpu_nr) +
			NR_IRQCPU_PREALLOC;

	/*
	 * cpu_nr: a value of 2 means there are 2 processors (0 and 1).
	 * In this case, we have to allocate 3 structures: global, proc0 and proc1.
	 */
	salloc_mp_struct(cpu_nr + 1);

	while (++opt < argc) {

		if (!strcmp(argv[opt], "-I")) {
			if (argv[++opt]) {
				actset = TRUE;
				if (!strcmp(argv[opt], K_SUM)) {
					/* Display total number of interrupts per CPU */
					actflags |= M_D_IRQ_SUM;
				}
				else if (!strcmp(argv[opt], K_CPU)) {
					/* Display interrupts per CPU */
					actflags |= M_D_IRQ_CPU;
				}
				else if (!strcmp(argv[opt], K_SCPU)) {
					/* Display soft interrupts per CPU */
					actflags |= M_D_SOFTIRQS;
				}
				else if (!strcmp(argv[opt], K_ALL)) {
					actflags |= M_D_IRQ_SUM + M_D_IRQ_CPU + M_D_SOFTIRQS;
				}
				else {
					usage(argv[0]);
				}
			}
			else {
				usage(argv[0]);
			}
		}

		else if (!strcmp(argv[opt], "-P")) {
			/* '-P ALL' can be used on UP machines */
			if (argv[++opt]) {
				flags |= F_P_OPTION;
				dis_hdr++;
				
				for (t = strtok(argv[opt], ","); t; t = strtok(NULL, ",")) {
					if (!strcmp(t, K_ALL) || !strcmp(t, K_ON)) {
						if (cpu_nr) {
							dis_hdr = 9;
						}
						/*
						 * Set bit for every processor.
						 * Also indicate to display stats for CPU 'all'.
						 */
						memset(cpu_bitmap, 0xff, ((cpu_nr + 1) >> 3) + 1);
						if (!strcmp(t, K_ON)) {
							/* Display stats only for online CPU */
							flags |= F_P_ON;
						}
					}
					else {
						if (strspn(t, DIGITS) != strlen(t)) {
							usage(argv[0]);
						}
						i = atoi(t);	/* Get cpu number */
						if (i >= cpu_nr) {
							fprintf(stderr, _("Not that many processors!\n"));
							exit(1);
						}
						i++;
						*(cpu_bitmap + (i >> 3)) |= 1 << (i & 0x07);
					}
				}
			}
			else {
				usage(argv[0]);
			}
		}

		else if (!strncmp(argv[opt], "-", 1)) {
			for (i = 1; *(argv[opt] + i); i++) {

				switch (*(argv[opt] + i)) {

				case 'A':
					actflags |= M_D_CPU + M_D_IRQ_SUM + M_D_IRQ_CPU + M_D_SOFTIRQS;
					actset = TRUE;
					/* Select all processors */
					flags |= F_P_OPTION;
					memset(cpu_bitmap, 0xff, ((cpu_nr + 1) >> 3) + 1);
					break;
					
				case 'u':
					/* Display CPU */
					actflags |= M_D_CPU;
					break;

				case 'V':
					/* Print version number */
					print_version();
					break;

				default:
					usage(argv[0]);
				}
			}
		}

		else if (interval < 0) {
			/* Get interval */
			if (strspn(argv[opt], DIGITS) != strlen(argv[opt])) {
				usage(argv[0]);
			}
			interval = atol(argv[opt]);
			if (interval < 0) {
				usage(argv[0]);
			}
			count = -1;
		}

		else if (count <= 0) {
			/* Get count value */
			if ((strspn(argv[opt], DIGITS) != strlen(argv[opt])) ||
			    !interval) {
				usage(argv[0]);
			}
			count = atol(argv[opt]);
			if (count < 1) {
				usage(argv[0]);
			}
		}

		else {
			usage(argv[0]);
		}
	}

	/* Default: Display CPU */
	if (!actset) {
		actflags |= M_D_CPU;
	}

	if (count_bits(&actflags, sizeof(unsigned int)) > 1) {
		dis_hdr = 9;
	}
	
	if (!USE_P_OPTION(flags)) {
		/* Option -P not used: Set bit 0 (global stats among all proc) */
		*cpu_bitmap = 1;
	}
	if (dis_hdr < 0) {
		dis_hdr = 0;
	}
	if (!dis_hdr) {
		/* Get window size */
		rows = get_win_height();
	}
	if (interval < 0) {
		/* Interval not set => display stats since boot time */
		interval = 0;
	}

	/* Get time */
	get_localtime(&(mp_tstamp[0]));

	/* Get system name, release number and hostname */
	uname(&header);
	print_gal_header(&(mp_tstamp[0]), header.sysname, header.release,
			 header.nodename, header.machine, cpu_nr);

	/* Main loop */
	rw_mpstat_loop(dis_hdr, rows);

	/* Free structures */
	sfree_mp_struct();

	return 0;
}
