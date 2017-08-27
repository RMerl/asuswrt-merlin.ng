/*
 * pr_stats.c: Functions used by sar to display statistics
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
#include <stdarg.h>
#include <stdlib.h>

#include "sa.h"
#include "ioconf.h"
#include "pr_stats.h"

#ifdef USE_NLS
#include <locale.h>
#include <libintl.h>
#define _(string) gettext(string)
#else
#define _(string) (string)
#endif

extern unsigned int flags;
extern unsigned int dm_major;
extern int  dis;
extern char timestamp[][TIMESTAMP_LEN];
extern unsigned long avg_count;

/*
 ***************************************************************************
 * Display CPU statistics.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @prev	Index in array where stats used as reference are.
 * @curr	Index in array for current sample statistics.
 * @g_itv	Interval of time in jiffies multiplied by the number
 *		of processors.
 ***************************************************************************
 */
__print_funct_t print_cpu_stats(struct activity *a, int prev, int curr,
				unsigned long long g_itv)
{
	int i;
	struct stats_cpu *scc, *scp;

	if (dis) {
		if (DISPLAY_CPU_DEF(a->opt_flags)) {
			printf("\n%-11s     CPU     %%user     %%nice   %%system"
			       "   %%iowait    %%steal     %%idle\n",
			       timestamp[!curr]);
		}
		else if (DISPLAY_CPU_ALL(a->opt_flags)) {
			printf("\n%-11s     CPU      %%usr     %%nice      %%sys"
			       "   %%iowait    %%steal      %%irq     %%soft"
			       "    %%guest     %%idle\n",
			       timestamp[!curr]);
		}
	}
	
	for (i = 0; (i < a->nr) && (i < a->bitmap->b_size + 1); i++) {
		
		/*
		 * The size of a->buf[...] CPU structure may be different from the default
		 * sizeof(struct stats_cpu) value if data have been read from a file!
		 * That's why we don't use a syntax like:
		 * scc = (struct stats_cpu *) a->buf[...] + i;
		 */
		scc = (struct stats_cpu *) ((char *) a->buf[curr] + i * a->msize);
		scp = (struct stats_cpu *) ((char *) a->buf[prev] + i * a->msize);

		/*
		 * Note: a->nr is in [1, NR_CPUS + 1].
		 * Bitmap size is provided for (NR_CPUS + 1) CPUs.
		 * Anyway, NR_CPUS may vary between the version of sysstat
		 * used by sadc to create a file, and the version of sysstat
		 * used by sar to read it...
		 */
		
		/* Should current CPU (including CPU "all") be displayed? */
		if (a->bitmap->b_array[i >> 3] & (1 << (i & 0x07))) {
			
			/* Yes: Display it */
			printf("%-11s", timestamp[curr]);
			
			if (!i) {
				/* This is CPU "all" */
				printf("     all");
			}
			else {
				printf("     %3d", i - 1);

				/*
				 * If the CPU is offline then it is omited from /proc/stat:
				 * All the fields couldn't have been read and the sum of them is zero.
				 * (Remember that guest time is already included in user mode.)
				 */
				if ((scc->cpu_user    + scc->cpu_nice + scc->cpu_sys   +
				     scc->cpu_iowait  + scc->cpu_idle + scc->cpu_steal +
				     scc->cpu_hardirq + scc->cpu_softirq) == 0) {
					/*
					 * Set current struct fields (which have been set to zero)
					 * to values from previous iteration. Hence their values won't
					 * jump from zero when the CPU comes back online.
					 */
					*scc = *scp;
					
					printf("    %6.2f    %6.2f    %6.2f"
					       "    %6.2f    %6.2f    %6.2f",
					       0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
					
					if (DISPLAY_CPU_ALL(a->opt_flags)) {
						printf("    %6.2f    %6.2f    %6.2f",
						       0.0, 0.0, 0.0);
					}
					printf("\n");
					continue;
				}
				
				/* Recalculate interval for current proc */
				g_itv = get_per_cpu_interval(scc, scp);
				
				if (!g_itv) {
					/*
					 * If the CPU is tickless then there is no change in CPU values
					 * but the sum of values is not zero.
					 */
					printf("    %6.2f    %6.2f    %6.2f"
					       "    %6.2f    %6.2f",
					       0.0, 0.0, 0.0, 0.0, 0.0);

					if (DISPLAY_CPU_DEF(a->opt_flags)) {
						printf("    %6.2f\n", 100.0);
					}
					else if (DISPLAY_CPU_ALL(a->opt_flags)) {
						printf("    %6.2f    %6.2f    %6.2f    %6.2f\n",
						       0.0, 0.0, 0.0, 100.0);
					}
					continue;
				}
			}
			
			if (DISPLAY_CPU_DEF(a->opt_flags)) {
				printf("    %6.2f    %6.2f    %6.2f    %6.2f    %6.2f    %6.2f\n",
				       ll_sp_value(scp->cpu_user,   scc->cpu_user,   g_itv),
				       ll_sp_value(scp->cpu_nice,   scc->cpu_nice,   g_itv),
				       ll_sp_value(scp->cpu_sys + scp->cpu_hardirq + scp->cpu_softirq,
						   scc->cpu_sys + scc->cpu_hardirq + scc->cpu_softirq,
						   g_itv),
				       ll_sp_value(scp->cpu_iowait, scc->cpu_iowait, g_itv),
				       ll_sp_value(scp->cpu_steal,  scc->cpu_steal,  g_itv),
				       scc->cpu_idle < scp->cpu_idle ?
				       0.0 :
				       ll_sp_value(scp->cpu_idle,   scc->cpu_idle,   g_itv));
			}
			else if (DISPLAY_CPU_ALL(a->opt_flags)) {
				printf("    %6.2f    %6.2f    %6.2f    %6.2f    %6.2f    %6.2f"
				       "    %6.2f    %6.2f    %6.2f\n",
				       (scc->cpu_user - scc->cpu_guest) < (scp->cpu_user - scp->cpu_guest) ?
				       0.0 :
				       ll_sp_value(scp->cpu_user - scp->cpu_guest,
						   scc->cpu_user - scc->cpu_guest,     g_itv),
				       ll_sp_value(scp->cpu_nice,    scc->cpu_nice,    g_itv),
				       ll_sp_value(scp->cpu_sys,     scc->cpu_sys,     g_itv),
				       ll_sp_value(scp->cpu_iowait,  scc->cpu_iowait,  g_itv),
				       ll_sp_value(scp->cpu_steal,   scc->cpu_steal,   g_itv),
				       ll_sp_value(scp->cpu_hardirq, scc->cpu_hardirq, g_itv),
				       ll_sp_value(scp->cpu_softirq, scc->cpu_softirq, g_itv),
				       ll_sp_value(scp->cpu_guest,   scc->cpu_guest,   g_itv),
				       scc->cpu_idle < scp->cpu_idle ?
				       0.0 :
				       ll_sp_value(scp->cpu_idle,    scc->cpu_idle,    g_itv));
			}
		}
	}
}

/*
 ***************************************************************************
 * Display tasks creation and context switches statistics.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @prev	Index in array where stats used as reference are.
 * @curr	Index in array for current sample statistics.
 * @itv		Interval of time in jiffies.
 ***************************************************************************
 */
__print_funct_t print_pcsw_stats(struct activity *a, int prev, int curr,
				 unsigned long long itv)
{
	struct stats_pcsw
		*spc = (struct stats_pcsw *) a->buf[curr],
		*spp = (struct stats_pcsw *) a->buf[prev];
	
	if (dis) {
		printf("\n%-11s    proc/s   cswch/s\n", timestamp[!curr]);
	}

	printf("%-11s %9.2f %9.2f\n", timestamp[curr],
	       S_VALUE   (spp->processes,      spc->processes,      itv),
	       ll_s_value(spp->context_switch, spc->context_switch, itv));
}

/*
 ***************************************************************************
 * Display interrupts statistics.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @prev	Index in array where stats used as reference are.
 * @curr	Index in array for current sample statistics.
 * @itv		Interval of time in jiffies.
 ***************************************************************************
 */
__print_funct_t print_irq_stats(struct activity *a, int prev, int curr,
				unsigned long long itv)
{
	int i;
	struct stats_irq *sic, *sip;
	
	if (dis) {
		printf("\n%-11s      INTR    intr/s\n", timestamp[!curr]);
	}
	
	for (i = 0; (i < a->nr) && (i < a->bitmap->b_size + 1); i++) {

		sic = (struct stats_irq *) ((char *) a->buf[curr] + i * a->msize);
		sip = (struct stats_irq *) ((char *) a->buf[prev] + i * a->msize);
		
		/*
		 * Note: a->nr is in [0, NR_IRQS + 1].
		 * Bitmap size is provided for (NR_IRQS + 1) interrupts.
		 * Anyway, NR_IRQS may vary between the version of sysstat
		 * used by sadc to create a file, and the version of sysstat
		 * used by sar to read it...
		 */
		
		/* Should current interrupt (including int "sum") be displayed? */
		if (a->bitmap->b_array[i >> 3] & (1 << (i & 0x07))) {
			
			/* Yes: Display it */
			printf("%-11s", timestamp[curr]);
			if (!i) {
				/* This is interrupt "sum" */
				printf("       sum");
			}
			else {
				printf("       %3d", i - 1);
			}

			printf(" %9.2f\n",
			       ll_s_value(sip->irq_nr, sic->irq_nr, itv));
		}
	}
}

/*
 ***************************************************************************
 * Display swapping statistics.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @prev	Index in array where stats used as reference are.
 * @curr	Index in array for current sample statistics.
 * @itv		Interval of time in jiffies.
 ***************************************************************************
 */
__print_funct_t print_swap_stats(struct activity *a, int prev, int curr,
				 unsigned long long itv)
{
	struct stats_swap
		*ssc = (struct stats_swap *) a->buf[curr],
		*ssp = (struct stats_swap *) a->buf[prev];
	
	if (dis) {
		printf("\n%-11s  pswpin/s pswpout/s\n", timestamp[!curr]);
	}

	printf("%-11s %9.2f %9.2f\n", timestamp[curr],
	       S_VALUE(ssp->pswpin,  ssc->pswpin,  itv),
	       S_VALUE(ssp->pswpout, ssc->pswpout, itv));
}

/*
 ***************************************************************************
 * Display paging statistics.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @prev	Index in array where stats used as reference are.
 * @curr	Index in array for current sample statistics.
 * @itv		Interval of time in jiffies.
 ***************************************************************************
 */
__print_funct_t print_paging_stats(struct activity *a, int prev, int curr,
				   unsigned long long itv)
{
	struct stats_paging
		*spc = (struct stats_paging *) a->buf[curr],
		*spp = (struct stats_paging *) a->buf[prev];

	if (dis) {
		printf("\n%-11s  pgpgin/s pgpgout/s   fault/s  majflt/s  pgfree/s"
		       " pgscank/s pgscand/s pgsteal/s    %%vmeff\n",
		       timestamp[!curr]);
	}

	printf("%-11s %9.2f %9.2f %9.2f %9.2f %9.2f %9.2f %9.2f %9.2f %9.2f\n",
	       timestamp[curr],
	       S_VALUE(spp->pgpgin,        spc->pgpgin,        itv),
	       S_VALUE(spp->pgpgout,       spc->pgpgout,       itv),
	       S_VALUE(spp->pgfault,       spc->pgfault,       itv),
	       S_VALUE(spp->pgmajfault,    spc->pgmajfault,    itv),
	       S_VALUE(spp->pgfree,        spc->pgfree,        itv),
	       S_VALUE(spp->pgscan_kswapd, spc->pgscan_kswapd, itv),
	       S_VALUE(spp->pgscan_direct, spc->pgscan_direct, itv),
	       S_VALUE(spp->pgsteal,       spc->pgsteal,       itv),
	       (spc->pgscan_kswapd + spc->pgscan_direct -
		spp->pgscan_kswapd - spp->pgscan_direct) ?
	       SP_VALUE(spp->pgsteal, spc->pgsteal,
			spc->pgscan_kswapd + spc->pgscan_direct -
			spp->pgscan_kswapd - spp->pgscan_direct) : 0.0);
}

/*
 ***************************************************************************
 * Display I/O and transfer rate statistics.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @prev	Index in array where stats used as reference are.
 * @curr	Index in array for current sample statistics.
 * @itv		Interval of time in jiffies.
 ***************************************************************************
 */
__print_funct_t print_io_stats(struct activity *a, int prev, int curr,
			       unsigned long long itv)
{
	struct stats_io
		*sic = (struct stats_io *) a->buf[curr],
		*sip = (struct stats_io *) a->buf[prev];

	if (dis) {
		printf("\n%-11s       tps      rtps      wtps   bread/s   bwrtn/s\n",
		       timestamp[!curr]);
	}

	printf("%-11s %9.2f %9.2f %9.2f %9.2f %9.2f\n", timestamp[curr],
	       S_VALUE(sip->dk_drive,      sic->dk_drive,      itv),
	       S_VALUE(sip->dk_drive_rio,  sic->dk_drive_rio,  itv),
	       S_VALUE(sip->dk_drive_wio,  sic->dk_drive_wio,  itv),
	       S_VALUE(sip->dk_drive_rblk, sic->dk_drive_rblk, itv),
	       S_VALUE(sip->dk_drive_wblk, sic->dk_drive_wblk, itv));
}

/*
 ***************************************************************************
 * Display memory and swap statistics. This function is used to
 * display instantaneous and average statistics.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @prev	Index in array where stats used as reference are.
 * @curr	Index in array for current sample statistics.
 * @itv		Interval of time in jiffies.
 * @dispavg	TRUE if displaying average statistics.
 ***************************************************************************
 */
void stub_print_memory_stats(struct activity *a, int prev, int curr,
			     unsigned long long itv, int dispavg)
{
	struct stats_memory
		*smc = (struct stats_memory *) a->buf[curr],
		*smp = (struct stats_memory *) a->buf[prev];
	static unsigned long long
		avg_frmkb    = 0,
		avg_bufkb    = 0,
		avg_camkb    = 0,
		avg_comkb    = 0,
		avg_activekb = 0,
		avg_inactkb  = 0;
	static unsigned long long
		avg_frskb = 0,
		avg_tlskb = 0,
		avg_caskb = 0;
	
	if (DISPLAY_MEMORY(a->opt_flags)) {
		if (dis) {
			printf("\n%-11s   frmpg/s   bufpg/s   campg/s\n",
			       timestamp[!curr]);
		}

		printf("%-11s %9.2f %9.2f %9.2f\n", timestamp[curr],
		       S_VALUE((double) KB_TO_PG(smp->frmkb), (double) KB_TO_PG(smc->frmkb), itv),
		       S_VALUE((double) KB_TO_PG(smp->bufkb), (double) KB_TO_PG(smc->bufkb), itv),
		       S_VALUE((double) KB_TO_PG(smp->camkb), (double) KB_TO_PG(smc->camkb), itv));
	}
	
	if (DISPLAY_MEM_AMT(a->opt_flags)) {
		if (dis) {
			printf("\n%-11s kbmemfree kbmemused  %%memused kbbuffers  kbcached"
			       "  kbcommit   %%commit  kbactive   kbinact\n", timestamp[!curr]);
		}

		if (!dispavg) {
			/* Display instantaneous values */
			printf("%-11s %9lu %9lu    %6.2f %9lu %9lu %9lu   %7.2f %9lu %9lu\n",
			       timestamp[curr],
			       smc->frmkb,
			       smc->tlmkb - smc->frmkb,
			       smc->tlmkb ?
			       SP_VALUE(smc->frmkb, smc->tlmkb, smc->tlmkb) : 0.0,
			       smc->bufkb,
			       smc->camkb,
			       smc->comkb,
			       (smc->tlmkb + smc->tlskb) ?
			       SP_VALUE(0, smc->comkb, smc->tlmkb + smc->tlskb) : 0.0,
			       smc->activekb,
			       smc->inactkb);

			/*
			 * Will be used to compute the average.
			 * We assume that the total amount of memory installed can not vary
			 * during the interval given on the command line.
			 */
			avg_frmkb    += smc->frmkb;
			avg_bufkb    += smc->bufkb;
			avg_camkb    += smc->camkb;
			avg_comkb    += smc->comkb;
			avg_activekb += smc->activekb;
			avg_inactkb  += smc->inactkb;
		}
		else {
			/* Display average values */
			printf("%-11s %9.0f %9.0f    %6.2f %9.0f %9.0f %9.0f   %7.2f %9.0f %9.0f\n",
			       timestamp[curr],
			       (double) avg_frmkb / avg_count,
			       (double) smc->tlmkb - ((double) avg_frmkb / avg_count),
			       smc->tlmkb ?
			       SP_VALUE((double) (avg_frmkb / avg_count), smc->tlmkb,
					smc->tlmkb) :
			       0.0,
			       (double) avg_bufkb / avg_count,
			       (double) avg_camkb / avg_count,
			       (double) avg_comkb / avg_count,
			       (smc->tlmkb + smc->tlskb) ?
			       SP_VALUE(0.0, (double) (avg_comkb / avg_count),
					smc->tlmkb + smc->tlskb) : 0.0,
			       (double) avg_activekb / avg_count,
			       (double) avg_inactkb / avg_count);
			
			/* Reset average counters */
			avg_frmkb = avg_bufkb = avg_camkb = avg_comkb = 0;
			avg_activekb = avg_inactkb = 0;
		}
	}
	
	if (DISPLAY_SWAP(a->opt_flags)) {
		if (dis) {
			printf("\n%-11s kbswpfree kbswpused  %%swpused  kbswpcad   %%swpcad\n",
			       timestamp[!curr]);
		}

		if (!dispavg) {
			/* Display instantaneous values */
			printf("%-11s %9lu %9lu    %6.2f %9lu    %6.2f\n",
			       timestamp[curr],
			       smc->frskb,
			       smc->tlskb - smc->frskb,
			       smc->tlskb ?
			       SP_VALUE(smc->frskb, smc->tlskb, smc->tlskb) : 0.0,
			       smc->caskb,
			       (smc->tlskb - smc->frskb) ?
			       SP_VALUE(0, smc->caskb, smc->tlskb - smc->frskb) : 0.0);

			/*
			 * Will be used to compute the average.
			 * We assume that the total amount of swap space may vary.
			 */
			avg_frskb += smc->frskb;
			avg_tlskb += smc->tlskb;
			avg_caskb += smc->caskb;
		}
		else {
			/* Display average values */
			printf("%-11s %9.0f %9.0f    %6.2f %9.0f    %6.2f\n",
			       timestamp[curr],
			       (double) avg_frskb / avg_count,
			       ((double) avg_tlskb / avg_count) -
			       ((double) avg_frskb / avg_count),
			       ((double) (avg_tlskb / avg_count)) ?
			       SP_VALUE((double) (avg_frskb / avg_count),
					(double) (avg_tlskb / avg_count),
					(double) (avg_tlskb / avg_count)) :
			       0.0,
			       (double) (avg_caskb / avg_count),
			       (((double) avg_tlskb / avg_count) -
				((double) avg_frskb / avg_count)) ?
			       SP_VALUE(0.0, (double) (avg_caskb / avg_count),
					((double) avg_tlskb / avg_count) -
					((double) avg_frskb / avg_count)) :
			       0.0);
			
			/* Reset average counters */
			avg_frskb = avg_tlskb = avg_caskb = 0;
		}
	}
}

/*
 ***************************************************************************
 * Display memory and swap statistics.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @prev	Index in array where stats used as reference are.
 * @curr	Index in array for current sample statistics.
 * @itv		Interval of time in jiffies.
 ***************************************************************************
 */
__print_funct_t print_memory_stats(struct activity *a, int prev, int curr,
				   unsigned long long itv)
{
	stub_print_memory_stats(a, prev, curr, itv, FALSE);
}

/*
 ***************************************************************************
 * Display average memory statistics.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @prev	Index in array where stats used as reference are.
 * @curr	Index in array for current sample statistics.
 * @itv		Interval of time in jiffies.
 ***************************************************************************
 */
__print_funct_t print_avg_memory_stats(struct activity *a, int prev, int curr,
				       unsigned long long itv)
{
	stub_print_memory_stats(a, prev, curr, itv, TRUE);
}

/*
 ***************************************************************************
 * Display kernel tables statistics. This function is used to display
 * instantaneous and average statistics.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @curr	Index in array for current sample statistics.
 * @dispavg	True if displaying average statistics.
 ***************************************************************************
 */
void stub_print_ktables_stats(struct activity *a, int curr, int dispavg)
{
	struct stats_ktables
		*skc = (struct stats_ktables *) a->buf[curr];
	static unsigned long long
		avg_dentry_stat = 0,
		avg_file_used   = 0,
		avg_inode_used  = 0,
		avg_pty_nr      = 0;
		
	
	if (dis) {
		printf("\n%-11s dentunusd   file-nr  inode-nr    pty-nr\n",
		       timestamp[!curr]);
	}

	if (!dispavg) {
		/* Display instantaneous values */
		printf("%-11s %9u %9u %9u %9u\n", timestamp[curr],
		       skc->dentry_stat,
		       skc->file_used,
		       skc->inode_used,
		       skc->pty_nr);

		/*
		 * Will be used to compute the average.
		 * Note: Overflow unlikely to happen but not impossible...
		 */
		avg_dentry_stat += skc->dentry_stat;
		avg_file_used   += skc->file_used;
		avg_inode_used  += skc->inode_used;
		avg_pty_nr      += skc->pty_nr;
	}
	else {
		/* Display average values */
		printf("%-11s %9.0f %9.0f %9.0f %9.0f\n",
		       timestamp[curr],
		       (double) avg_dentry_stat / avg_count,
		       (double) avg_file_used   / avg_count,
		       (double) avg_inode_used  / avg_count,
		       (double) avg_pty_nr      / avg_count);

		/* Reset average counters */
		avg_dentry_stat = avg_file_used = avg_inode_used = avg_pty_nr = 0;
	}
}

/*
 ***************************************************************************
 * Display kernel tables statistics.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @prev	Index in array where stats used as reference are.
 * @curr	Index in array for current sample statistics.
 * @itv		Interval of time in jiffies.
 ***************************************************************************
 */
__print_funct_t print_ktables_stats(struct activity *a, int prev, int curr,
				    unsigned long long itv)
{
	stub_print_ktables_stats(a, curr, FALSE);
}

/*
 ***************************************************************************
 * Display average kernel tables statistics.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @prev	Index in array where stats used as reference are.
 * @curr	Index in array for current sample statistics.
 * @itv		Interval of time in jiffies.
 ***************************************************************************
 */
__print_funct_t print_avg_ktables_stats(struct activity *a, int prev, int curr,
					unsigned long long itv)
{
	stub_print_ktables_stats(a, curr, TRUE);
}

/*
 ***************************************************************************
 * Display queue and load statistics. This function is used to display
 * instantaneous and average statistics.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @curr	Index in array for current sample statistics.
 * @dispavg	TRUE if displaying average statistics.
 ***************************************************************************
 */
void stub_print_queue_stats(struct activity *a, int curr, int dispavg)
{
	struct stats_queue
		*sqc = (struct stats_queue *) a->buf[curr];
	static unsigned long long
		avg_nr_running    = 0,
		avg_nr_threads    = 0,
		avg_load_avg_1    = 0,
		avg_load_avg_5    = 0,
		avg_load_avg_15   = 0,
		avg_procs_blocked = 0;
	
	if (dis) {
		printf("\n%-11s   runq-sz  plist-sz   ldavg-1   ldavg-5  ldavg-15   blocked\n",
		       timestamp[!curr]);
	}

	if (!dispavg) {
		/* Display instantaneous values */
		printf("%-11s %9lu %9u %9.2f %9.2f %9.2f %9lu\n", timestamp[curr],
		       sqc->nr_running,
		       sqc->nr_threads,
		       (double) sqc->load_avg_1  / 100,
		       (double) sqc->load_avg_5  / 100,
		       (double) sqc->load_avg_15 / 100,
		       sqc->procs_blocked);

		/* Will be used to compute the average */
		avg_nr_running    += sqc->nr_running;
		avg_nr_threads    += sqc->nr_threads;
		avg_load_avg_1    += sqc->load_avg_1;
		avg_load_avg_5    += sqc->load_avg_5;
		avg_load_avg_15   += sqc->load_avg_15;
		avg_procs_blocked += sqc->procs_blocked;
	}
	else {
		/* Display average values */
		printf("%-11s %9.0f %9.0f %9.2f %9.2f %9.2f %9.0f\n", timestamp[curr],
		       (double) avg_nr_running    / avg_count,
		       (double) avg_nr_threads    / avg_count,
		       (double) avg_load_avg_1    / (avg_count * 100),
		       (double) avg_load_avg_5    / (avg_count * 100),
		       (double) avg_load_avg_15   / (avg_count * 100),
		       (double) avg_procs_blocked / avg_count);

		/* Reset average counters */
		avg_nr_running = avg_nr_threads = 0;
		avg_load_avg_1 = avg_load_avg_5 = avg_load_avg_15 = 0;
		avg_procs_blocked = 0;
	}
}

/*
 ***************************************************************************
 * Display queue and load statistics.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @prev	Index in array where stats used as reference are.
 * @curr	Index in array for current sample statistics.
 * @itv		Interval of time in jiffies.
 ***************************************************************************
 */
__print_funct_t print_queue_stats(struct activity *a, int prev, int curr,
				  unsigned long long itv)
{
	stub_print_queue_stats(a, curr, FALSE);
}

/*
 ***************************************************************************
 * Display average queue and load statistics.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @prev	Index in array where stats used as reference are.
 * @curr	Index in array for current sample statistics.
 * @itv		Interval of time in jiffies.
 ***************************************************************************
 */
__print_funct_t print_avg_queue_stats(struct activity *a, int prev, int curr,
				      unsigned long long itv)
{
	stub_print_queue_stats(a, curr, TRUE);
}

/*
 ***************************************************************************
 * Display serial lines statistics.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @prev	Index in array where stats used as reference are.
 * @curr	Index in array for current sample statistics.
 * @itv		Interval of time in jiffies.
 ***************************************************************************
 */
__print_funct_t print_serial_stats(struct activity *a, int prev, int curr,
				   unsigned long long itv)
{
	int i;
	struct stats_serial *ssc, *ssp;

	if (dis) {
		printf("\n%-11s       TTY   rcvin/s   xmtin/s framerr/s prtyerr/s"
		       "     brk/s   ovrun/s\n", timestamp[!curr]);
	}

	for (i = 0; i < a->nr; i++) {

		ssc = (struct stats_serial *) ((char *) a->buf[curr] + i * a->msize);
		ssp = (struct stats_serial *) ((char *) a->buf[prev] + i * a->msize);

		if (ssc->line == 0)
			continue;

		printf("%-11s       %3d", timestamp[curr], ssc->line - 1);

		if ((ssc->line == ssp->line) || WANT_SINCE_BOOT(flags)) {
			printf(" %9.2f %9.2f %9.2f %9.2f %9.2f %9.2f\n",
			       S_VALUE(ssp->rx,      ssc->rx,      itv),
			       S_VALUE(ssp->tx,      ssc->tx,      itv),
			       S_VALUE(ssp->frame,   ssc->frame,   itv),
			       S_VALUE(ssp->parity,  ssc->parity,  itv),
			       S_VALUE(ssp->brk,     ssc->brk,     itv),
			       S_VALUE(ssp->overrun, ssc->overrun, itv));
		}
		else {
			printf("       N/A       N/A       N/A       N/A"
			       "       N/A       N/A\n");
		}
	}
}

/*
 ***************************************************************************
 * Display disks statistics.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @prev	Index in array where stats used as reference are.
 * @curr	Index in array for current sample statistics.
 * @itv		Interval of time in jiffies.
 ***************************************************************************
 */
__print_funct_t print_disk_stats(struct activity *a, int prev, int curr,
				 unsigned long long itv)
{
	int i, j;
	struct stats_disk *sdc,	*sdp;
	struct ext_disk_stats xds;
	char *dev_name;

	if (dis) {
		printf("\n%-11s       DEV       tps  rd_sec/s  wr_sec/s  avgrq-sz"
		       "  avgqu-sz     await     svctm     %%util\n",
		       timestamp[!curr]);
	}

	for (i = 0; i < a->nr; i++) {

		sdc = (struct stats_disk *) ((char *) a->buf[curr] + i * a->msize);

		if (!(sdc->major + sdc->minor))
			continue;

		j = check_disk_reg(a, curr, prev, i);
		sdp = (struct stats_disk *) ((char *) a->buf[prev] + j * a->msize);

		/* Compute service time, etc. */
		compute_ext_disk_stats(sdc, sdp, itv, &xds);
		
		dev_name = NULL;

		if ((USE_PRETTY_OPTION(flags)) && (sdc->major == dm_major)) {
			dev_name = transform_devmapname(sdc->major, sdc->minor);
		}

		if (!dev_name) {
			dev_name = get_devname(sdc->major, sdc->minor,
					       USE_PRETTY_OPTION(flags));
		}
		
		printf("%-11s %9s %9.2f %9.2f %9.2f %9.2f %9.2f %9.2f %9.2f %9.2f\n",
		       timestamp[curr],
		       /* Confusion possible here between index and minor numbers */
		       dev_name,
		       S_VALUE(sdp->nr_ios, sdc->nr_ios,  itv),
		       ll_s_value(sdp->rd_sect, sdc->rd_sect, itv),
		       ll_s_value(sdp->wr_sect, sdc->wr_sect, itv),
		       /* See iostat for explanations */
		       xds.arqsz,
		       S_VALUE(sdp->rq_ticks, sdc->rq_ticks, itv) / 1000.0,
		       xds.await,
		       xds.svctm,
		       xds.util / 10.0);
	}
}

/*
 ***************************************************************************
 * Display network interfaces statistics.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @prev	Index in array where stats used as reference are.
 * @curr	Index in array for current sample statistics.
 * @itv		Interval of time in jiffies.
 ***************************************************************************
 */
__print_funct_t print_net_dev_stats(struct activity *a, int prev, int curr,
				    unsigned long long itv)
{
	int i, j;
	struct stats_net_dev *sndc, *sndp;

	if (dis) {
		printf("\n%-11s     IFACE   rxpck/s   txpck/s    rxkB/s    txkB/s"
		       "   rxcmp/s   txcmp/s  rxmcst/s\n", timestamp[!curr]);
	}

	for (i = 0; i < a->nr; i++) {

		sndc = (struct stats_net_dev *) ((char *) a->buf[curr] + i * a->msize);

		if (!strcmp(sndc->interface, ""))
			continue;
		
		j = check_net_dev_reg(a, curr, prev, i);
		sndp = (struct stats_net_dev *) ((char *) a->buf[prev] + j * a->msize);

		printf("%-11s %9s", timestamp[curr], sndc->interface);

		printf(" %9.2f %9.2f %9.2f %9.2f %9.2f %9.2f %9.2f\n",
		       S_VALUE(sndp->rx_packets,    sndc->rx_packets,    itv),
		       S_VALUE(sndp->tx_packets,    sndc->tx_packets,    itv),
		       S_VALUE(sndp->rx_bytes,      sndc->rx_bytes,      itv) / 1024,
		       S_VALUE(sndp->tx_bytes,      sndc->tx_bytes,      itv) / 1024,
		       S_VALUE(sndp->rx_compressed, sndc->rx_compressed, itv),
		       S_VALUE(sndp->tx_compressed, sndc->tx_compressed, itv),
		       S_VALUE(sndp->multicast,     sndc->multicast,     itv));
	}
}

/*
 ***************************************************************************
 * Display network interface errors statistics.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @prev	Index in array where stats used as reference are.
 * @curr	Index in array for current sample statistics.
 * @itv		Interval of time in jiffies.
 ***************************************************************************
 */
__print_funct_t print_net_edev_stats(struct activity *a, int prev, int curr,
				     unsigned long long itv)
{
	int i, j;
	struct stats_net_edev *snedc, *snedp;

	if (dis) {
		printf("\n%-11s     IFACE   rxerr/s   txerr/s    coll/s  rxdrop/s"
		       "  txdrop/s  txcarr/s  rxfram/s  rxfifo/s  txfifo/s\n",
		       timestamp[!curr]);
	}

	for (i = 0; i < a->nr; i++) {

		snedc = (struct stats_net_edev *) ((char *) a->buf[curr] + i * a->msize);

		if (!strcmp(snedc->interface, ""))
			continue;
		
		j = check_net_edev_reg(a, curr, prev, i);
		snedp = (struct stats_net_edev *) ((char *) a->buf[prev] + j * a->msize);

		printf("%-11s %9s", timestamp[curr], snedc->interface);

		printf(" %9.2f %9.2f %9.2f %9.2f %9.2f %9.2f %9.2f %9.2f %9.2f\n",
		       S_VALUE(snedp->rx_errors,         snedc->rx_errors,         itv),
		       S_VALUE(snedp->tx_errors,         snedc->tx_errors,         itv),
		       S_VALUE(snedp->collisions,        snedc->collisions,        itv),
		       S_VALUE(snedp->rx_dropped,        snedc->rx_dropped,        itv),
		       S_VALUE(snedp->tx_dropped,        snedc->tx_dropped,        itv),
		       S_VALUE(snedp->tx_carrier_errors, snedc->tx_carrier_errors, itv),
		       S_VALUE(snedp->rx_frame_errors,   snedc->rx_frame_errors,   itv),
		       S_VALUE(snedp->rx_fifo_errors,    snedc->rx_fifo_errors,    itv),
		       S_VALUE(snedp->tx_fifo_errors,    snedc->tx_fifo_errors,    itv));
	}
}

/*
 ***************************************************************************
 * Display NFS client statistics.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @prev	Index in array where stats used as reference are.
 * @curr	Index in array for current sample statistics.
 * @itv		Interval of time in jiffies.
 ***************************************************************************
 */
__print_funct_t print_net_nfs_stats(struct activity *a, int prev, int curr,
				    unsigned long long itv)
{
	struct stats_net_nfs
		*snnc = (struct stats_net_nfs *) a->buf[curr],
		*snnp = (struct stats_net_nfs *) a->buf[prev];
	
	if (dis) {
		printf("\n%-11s    call/s retrans/s    read/s   write/s  access/s"
		       "  getatt/s\n", timestamp[!curr]);
	}

	printf("%-11s %9.2f %9.2f %9.2f %9.2f %9.2f %9.2f\n", timestamp[curr],
	       S_VALUE(snnp->nfs_rpccnt,     snnc->nfs_rpccnt,     itv),
	       S_VALUE(snnp->nfs_rpcretrans, snnc->nfs_rpcretrans, itv),
	       S_VALUE(snnp->nfs_readcnt,    snnc->nfs_readcnt,    itv),
	       S_VALUE(snnp->nfs_writecnt,   snnc->nfs_writecnt,   itv),
	       S_VALUE(snnp->nfs_accesscnt,  snnc->nfs_accesscnt,  itv),
	       S_VALUE(snnp->nfs_getattcnt,  snnc->nfs_getattcnt,  itv));
}

/*
 ***************************************************************************
 * Display NFS server statistics.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @prev	Index in array where stats used as reference are.
 * @curr	Index in array for current sample statistics.
 * @itv		Interval of time in jiffies.
 ***************************************************************************
 */
__print_funct_t print_net_nfsd_stats(struct activity *a, int prev, int curr,
				     unsigned long long itv)
{
	struct stats_net_nfsd
		*snndc = (struct stats_net_nfsd *) a->buf[curr],
		*snndp = (struct stats_net_nfsd *) a->buf[prev];

	if (dis) {
		printf("\n%-11s   scall/s badcall/s  packet/s     udp/s     tcp/s     "
		       "hit/s    miss/s   sread/s  swrite/s saccess/s sgetatt/s\n",
		       timestamp[!curr]);
	}

	printf("%-11s %9.2f %9.2f %9.2f %9.2f %9.2f %9.2f %9.2f %9.2f %9.2f %9.2f %9.2f\n",
	       timestamp[curr],
	       S_VALUE(snndp->nfsd_rpccnt,    snndc->nfsd_rpccnt,    itv),
	       S_VALUE(snndp->nfsd_rpcbad,    snndc->nfsd_rpcbad,    itv),
	       S_VALUE(snndp->nfsd_netcnt,    snndc->nfsd_netcnt,    itv),
	       S_VALUE(snndp->nfsd_netudpcnt, snndc->nfsd_netudpcnt, itv),
	       S_VALUE(snndp->nfsd_nettcpcnt, snndc->nfsd_nettcpcnt, itv),
	       S_VALUE(snndp->nfsd_rchits,    snndc->nfsd_rchits,    itv),
	       S_VALUE(snndp->nfsd_rcmisses,  snndc->nfsd_rcmisses,  itv),
	       S_VALUE(snndp->nfsd_readcnt,   snndc->nfsd_readcnt,   itv),
	       S_VALUE(snndp->nfsd_writecnt,  snndc->nfsd_writecnt,  itv),
	       S_VALUE(snndp->nfsd_accesscnt, snndc->nfsd_accesscnt, itv),
	       S_VALUE(snndp->nfsd_getattcnt, snndc->nfsd_getattcnt, itv));
}

/*
 ***************************************************************************
 * Display network sockets statistics. This function is used to display
 * instantaneous and average statistics.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @curr	Index in array for current sample statistics.
 * @dispavg	TRUE if displaying average statistics.
 ***************************************************************************
 */
void stub_print_net_sock_stats(struct activity *a, int curr, int dispavg)
{
	struct stats_net_sock
		*snsc = (struct stats_net_sock *) a->buf[curr];
	static unsigned long long
		avg_sock_inuse = 0,
		avg_tcp_inuse  = 0,
		avg_udp_inuse  = 0,
		avg_raw_inuse  = 0,
		avg_frag_inuse = 0,
		avg_tcp_tw     = 0;
	
	if (dis) {
		printf("\n%-11s    totsck    tcpsck    udpsck    rawsck   ip-frag    tcp-tw\n",
		       timestamp[!curr]);
	}

	if (!dispavg) {
		/* Display instantaneous values */
		printf("%-11s %9u %9u %9u %9u %9u %9u\n", timestamp[curr],
		       snsc->sock_inuse,
		       snsc->tcp_inuse,
		       snsc->udp_inuse,
		       snsc->raw_inuse,
		       snsc->frag_inuse,
		       snsc->tcp_tw);

		/* Will be used to compute the average */
		avg_sock_inuse += snsc->sock_inuse;
		avg_tcp_inuse  += snsc->tcp_inuse;
		avg_udp_inuse  += snsc->udp_inuse;
		avg_raw_inuse  += snsc->raw_inuse;
		avg_frag_inuse += snsc->frag_inuse;
		avg_tcp_tw     += snsc->tcp_tw;
	}
	else {
		/* Display average values */
		printf("%-11s %9.0f %9.0f %9.0f %9.0f %9.0f %9.0f\n", timestamp[curr],
		       (double) avg_sock_inuse / avg_count,
		       (double) avg_tcp_inuse  / avg_count,
		       (double) avg_udp_inuse  / avg_count,
		       (double) avg_raw_inuse  / avg_count,
		       (double) avg_frag_inuse / avg_count,
		       (double) avg_tcp_tw     / avg_count);

		/* Reset average counters */
		avg_sock_inuse = avg_tcp_inuse = avg_udp_inuse = 0;
		avg_raw_inuse = avg_frag_inuse = avg_tcp_tw = 0;
	}
}

/*
 ***************************************************************************
 * Display network sockets statistics.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @prev	Index in array where stats used as reference are.
 * @curr	Index in array for current sample statistics.
 * @itv		Interval of time in jiffies.
 ***************************************************************************
 */
__print_funct_t print_net_sock_stats(struct activity *a, int prev, int curr,
				     unsigned long long itv)
{
	stub_print_net_sock_stats(a, curr, FALSE);
}

/*
 ***************************************************************************
 * Display average network sockets statistics.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @prev	Index in array where stats used as reference are.
 * @curr	Index in array for current sample statistics.
 * @itv		Interval of time in jiffies.
 ***************************************************************************
 */
__print_funct_t print_avg_net_sock_stats(struct activity *a, int prev, int curr,
					 unsigned long long itv)
{
	stub_print_net_sock_stats(a, curr, TRUE);
}

/*
 ***************************************************************************
 * Display IP network traffic statistics.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @prev	Index in array where stats used as reference are.
 * @curr	Index in array for current sample statistics.
 * @itv		Interval of time in jiffies.
 ***************************************************************************
 */
__print_funct_t print_net_ip_stats(struct activity *a, int prev, int curr,
				   unsigned long long itv)
{
	struct stats_net_ip
		*snic = (struct stats_net_ip *) a->buf[curr],
		*snip = (struct stats_net_ip *) a->buf[prev];

	if (dis) {
		printf("\n%-11s    irec/s  fwddgm/s    idel/s     orq/s   asmrq/s"
		       "   asmok/s  fragok/s fragcrt/s\n", timestamp[!curr]);
	}

	printf("%-11s %9.2f %9.2f %9.2f %9.2f %9.2f %9.2f %9.2f %9.2f\n",
	       timestamp[curr],
	       S_VALUE(snip->InReceives,    snic->InReceives,    itv),
	       S_VALUE(snip->ForwDatagrams, snic->ForwDatagrams, itv),
	       S_VALUE(snip->InDelivers,    snic->InDelivers,    itv),
	       S_VALUE(snip->OutRequests,   snic->OutRequests,   itv),
	       S_VALUE(snip->ReasmReqds,    snic->ReasmReqds,    itv),
	       S_VALUE(snip->ReasmOKs,      snic->ReasmOKs,      itv),
	       S_VALUE(snip->FragOKs,       snic->FragOKs,       itv),
	       S_VALUE(snip->FragCreates,   snic->FragCreates,   itv));
}

/*
 ***************************************************************************
 * Display IP network error statistics.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @prev	Index in array where stats used as reference are.
 * @curr	Index in array for current sample statistics.
 * @itv		Interval of time in jiffies.
 ***************************************************************************
 */
__print_funct_t print_net_eip_stats(struct activity *a, int prev, int curr,
				    unsigned long long itv)
{
	struct stats_net_eip
		*sneic = (struct stats_net_eip *) a->buf[curr],
		*sneip = (struct stats_net_eip *) a->buf[prev];

	if (dis) {
		printf("\n%-11s ihdrerr/s iadrerr/s iukwnpr/s   idisc/s   odisc/s"
		       "   onort/s    asmf/s   fragf/s\n", timestamp[!curr]);
	}

	printf("%-11s %9.2f %9.2f %9.2f %9.2f %9.2f %9.2f %9.2f %9.2f\n",
	       timestamp[curr],
	       S_VALUE(sneip->InHdrErrors,     sneic->InHdrErrors,     itv),
	       S_VALUE(sneip->InAddrErrors,    sneic->InAddrErrors,    itv),
	       S_VALUE(sneip->InUnknownProtos, sneic->InUnknownProtos, itv),
	       S_VALUE(sneip->InDiscards,      sneic->InDiscards,      itv),
	       S_VALUE(sneip->OutDiscards,     sneic->OutDiscards,     itv),
	       S_VALUE(sneip->OutNoRoutes,     sneic->OutNoRoutes,     itv),
	       S_VALUE(sneip->ReasmFails,      sneic->ReasmFails,      itv),
	       S_VALUE(sneip->FragFails,       sneic->FragFails,       itv));
}

/*
 ***************************************************************************
 * Display ICMP network traffic statistics.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @prev	Index in array where stats used as reference are.
 * @curr	Index in array for current sample statistics.
 * @itv		Interval of time in jiffies.
 ***************************************************************************
 */
__print_funct_t print_net_icmp_stats(struct activity *a, int prev, int curr,
				     unsigned long long itv)
{
	struct stats_net_icmp
		*snic = (struct stats_net_icmp *) a->buf[curr],
		*snip = (struct stats_net_icmp *) a->buf[prev];

	if (dis) {
		printf("\n%-11s    imsg/s    omsg/s    iech/s   iechr/s    oech/s"
		       "   oechr/s     itm/s    itmr/s     otm/s    otmr/s"
		       "  iadrmk/s iadrmkr/s  oadrmk/s oadrmkr/s\n", timestamp[!curr]);
	}

	printf("%-11s %9.2f %9.2f %9.2f %9.2f %9.2f %9.2f %9.2f %9.2f %9.2f %9.2f"
	       " %9.2f %9.2f %9.2f %9.2f\n", timestamp[curr],
	       S_VALUE(snip->InMsgs,           snic->InMsgs,           itv),
	       S_VALUE(snip->OutMsgs,          snic->OutMsgs,          itv),
	       S_VALUE(snip->InEchos,          snic->InEchos,          itv),
	       S_VALUE(snip->InEchoReps,       snic->InEchoReps,       itv),
	       S_VALUE(snip->OutEchos,         snic->OutEchos,         itv),
	       S_VALUE(snip->OutEchoReps,      snic->OutEchoReps,      itv),
	       S_VALUE(snip->InTimestamps,     snic->InTimestamps,     itv),
	       S_VALUE(snip->InTimestampReps,  snic->InTimestampReps,  itv),
	       S_VALUE(snip->OutTimestamps,    snic->OutTimestamps,    itv),
	       S_VALUE(snip->OutTimestampReps, snic->OutTimestampReps, itv),
	       S_VALUE(snip->InAddrMasks,      snic->InAddrMasks,      itv),
	       S_VALUE(snip->InAddrMaskReps,   snic->InAddrMaskReps,   itv),
	       S_VALUE(snip->OutAddrMasks,     snic->OutAddrMasks,     itv),
	       S_VALUE(snip->OutAddrMaskReps,  snic->OutAddrMaskReps,  itv));
}

/*
 ***************************************************************************
 * Display ICMP network error statistics.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @prev	Index in array where stats used as reference are.
 * @curr	Index in array for current sample statistics.
 * @itv		Interval of time in jiffies.
 ***************************************************************************
 */
__print_funct_t print_net_eicmp_stats(struct activity *a, int prev, int curr,
				      unsigned long long itv)
{
	struct stats_net_eicmp
		*sneic = (struct stats_net_eicmp *) a->buf[curr],
		*sneip = (struct stats_net_eicmp *) a->buf[prev];

	if (dis) {
		printf("\n%-11s    ierr/s    oerr/s idstunr/s odstunr/s   itmex/s"
		       "   otmex/s iparmpb/s oparmpb/s   isrcq/s   osrcq/s"
		       "  iredir/s  oredir/s\n", timestamp[!curr]);
	}

	printf("%-11s %9.2f %9.2f %9.2f %9.2f %9.2f %9.2f %9.2f %9.2f %9.2f %9.2f"
	       " %9.2f %9.2f\n", timestamp[curr],
	       S_VALUE(sneip->InErrors,        sneic->InErrors,        itv),
	       S_VALUE(sneip->OutErrors,       sneic->OutErrors,       itv),
	       S_VALUE(sneip->InDestUnreachs,  sneic->InDestUnreachs,  itv),
	       S_VALUE(sneip->OutDestUnreachs, sneic->OutDestUnreachs, itv),
	       S_VALUE(sneip->InTimeExcds,     sneic->InTimeExcds,     itv),
	       S_VALUE(sneip->OutTimeExcds,    sneic->OutTimeExcds,    itv),
	       S_VALUE(sneip->InParmProbs,     sneic->InParmProbs,     itv),
	       S_VALUE(sneip->OutParmProbs,    sneic->OutParmProbs,    itv),
	       S_VALUE(sneip->InSrcQuenchs,    sneic->InSrcQuenchs,    itv),
	       S_VALUE(sneip->OutSrcQuenchs,   sneic->OutSrcQuenchs,   itv),
	       S_VALUE(sneip->InRedirects,     sneic->InRedirects,     itv),
	       S_VALUE(sneip->OutRedirects,    sneic->OutRedirects,    itv));
}

/*
 ***************************************************************************
 * Display TCP network traffic statistics.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @prev	Index in array where stats used as reference are.
 * @curr	Index in array for current sample statistics.
 * @itv		Interval of time in jiffies.
 ***************************************************************************
 */
__print_funct_t print_net_tcp_stats(struct activity *a, int prev, int curr,
				    unsigned long long itv)
{
	struct stats_net_tcp
		*sntc = (struct stats_net_tcp *) a->buf[curr],
		*sntp = (struct stats_net_tcp *) a->buf[prev];

	if (dis) {
		printf("\n%-11s  active/s passive/s    iseg/s    oseg/s\n",
		       timestamp[!curr]);
	}

	printf("%-11s %9.2f %9.2f %9.2f %9.2f\n",
	       timestamp[curr],
	       S_VALUE(sntp->ActiveOpens,  sntc->ActiveOpens,  itv),
	       S_VALUE(sntp->PassiveOpens, sntc->PassiveOpens, itv),
	       S_VALUE(sntp->InSegs,       sntc->InSegs,       itv),
	       S_VALUE(sntp->OutSegs,      sntc->OutSegs,      itv));
}

/*
 ***************************************************************************
 * Display TCP network error statistics.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @prev	Index in array where stats used as reference are.
 * @curr	Index in array for current sample statistics.
 * @itv		Interval of time in jiffies.
 ***************************************************************************
 */
__print_funct_t print_net_etcp_stats(struct activity *a, int prev, int curr,
				     unsigned long long itv)
{
	struct stats_net_etcp
		*snetc = (struct stats_net_etcp *) a->buf[curr],
		*snetp = (struct stats_net_etcp *) a->buf[prev];

	if (dis) {
		printf("\n%-11s  atmptf/s  estres/s retrans/s isegerr/s   orsts/s\n",
		       timestamp[!curr]);
	}

	printf("%-11s %9.2f %9.2f %9.2f %9.2f %9.2f\n",
	       timestamp[curr],
	       S_VALUE(snetp->AttemptFails, snetc->AttemptFails, itv),
	       S_VALUE(snetp->EstabResets,  snetc->EstabResets,  itv),
	       S_VALUE(snetp->RetransSegs,  snetc->RetransSegs,  itv),
	       S_VALUE(snetp->InErrs,       snetc->InErrs,       itv),
	       S_VALUE(snetp->OutRsts,      snetc->OutRsts,      itv));
}

/*
 ***************************************************************************
 * Display UDP network traffic statistics.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @prev	Index in array where stats used as reference are.
 * @curr	Index in array for current sample statistics.
 * @itv		Interval of time in jiffies.
 ***************************************************************************
 */
__print_funct_t print_net_udp_stats(struct activity *a, int prev, int curr,
				    unsigned long long itv)
{
	struct stats_net_udp
		*snuc = (struct stats_net_udp *) a->buf[curr],
		*snup = (struct stats_net_udp *) a->buf[prev];

	if (dis) {
		printf("\n%-11s    idgm/s    odgm/s  noport/s idgmerr/s\n",
		       timestamp[!curr]);
	}

	printf("%-11s %9.2f %9.2f %9.2f %9.2f\n",
	       timestamp[curr],
	       S_VALUE(snup->InDatagrams,  snuc->InDatagrams,  itv),
	       S_VALUE(snup->OutDatagrams, snuc->OutDatagrams, itv),
	       S_VALUE(snup->NoPorts,      snuc->NoPorts,      itv),
	       S_VALUE(snup->InErrors,     snuc->InErrors,     itv));
}

/*
 ***************************************************************************
 * Display IPv6 sockets statistics. This function is used to display
 * instantaneous and average statistics.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @curr	Index in array for current sample statistics.
 * @dispavg	TRUE if displaying average statistics.
 ***************************************************************************
 */
void stub_print_net_sock6_stats(struct activity *a, int curr, int dispavg)
{
	struct stats_net_sock6
		*snsc = (struct stats_net_sock6 *) a->buf[curr];
	static unsigned long long
		avg_tcp6_inuse  = 0,
		avg_udp6_inuse  = 0,
		avg_raw6_inuse  = 0,
		avg_frag6_inuse = 0;
	
	if (dis) {
		printf("\n%-11s   tcp6sck   udp6sck   raw6sck  ip6-frag\n",
		       timestamp[!curr]);
	}

	if (!dispavg) {
		/* Display instantaneous values */
		printf("%-11s %9u %9u %9u %9u\n", timestamp[curr],
		       snsc->tcp6_inuse,
		       snsc->udp6_inuse,
		       snsc->raw6_inuse,
		       snsc->frag6_inuse);

		/* Will be used to compute the average */
		avg_tcp6_inuse  += snsc->tcp6_inuse;
		avg_udp6_inuse  += snsc->udp6_inuse;
		avg_raw6_inuse  += snsc->raw6_inuse;
		avg_frag6_inuse += snsc->frag6_inuse;
	}
	else {
		/* Display average values */
		printf("%-11s %9.0f %9.0f %9.0f %9.0f\n", timestamp[curr],
		       (double) avg_tcp6_inuse  / avg_count,
		       (double) avg_udp6_inuse  / avg_count,
		       (double) avg_raw6_inuse  / avg_count,
		       (double) avg_frag6_inuse / avg_count);

		/* Reset average counters */
		avg_tcp6_inuse = avg_udp6_inuse = avg_raw6_inuse = avg_frag6_inuse = 0;
	}
}

/*
 ***************************************************************************
 * Display IPv6 sockets statistics.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @prev	Index in array where stats used as reference are.
 * @curr	Index in array for current sample statistics.
 * @itv		Interval of time in jiffies.
 ***************************************************************************
 */
__print_funct_t print_net_sock6_stats(struct activity *a, int prev, int curr,
				      unsigned long long itv)
{
	stub_print_net_sock6_stats(a, curr, FALSE);
}

/*
 ***************************************************************************
 * Display average IPv6 sockets statistics.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @prev	Index in array where stats used as reference are.
 * @curr	Index in array for current sample statistics.
 * @itv		Interval of time in jiffies.
 ***************************************************************************
 */
__print_funct_t print_avg_net_sock6_stats(struct activity *a, int prev, int curr,
					  unsigned long long itv)
{
	stub_print_net_sock6_stats(a, curr, TRUE);
}

/*
 ***************************************************************************
 * Display IPv6 network traffic statistics.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @prev	Index in array where stats used as reference are.
 * @curr	Index in array for current sample statistics.
 * @itv		Interval of time in jiffies.
 ***************************************************************************
 */
__print_funct_t print_net_ip6_stats(struct activity *a, int prev, int curr,
				    unsigned long long itv)
{
	struct stats_net_ip6
		*snic = (struct stats_net_ip6 *) a->buf[curr],
		*snip = (struct stats_net_ip6 *) a->buf[prev];

	if (dis) {
		printf("\n%-11s   irec6/s fwddgm6/s   idel6/s    orq6/s  asmrq6/s"
		       "  asmok6/s imcpck6/s omcpck6/s fragok6/s fragcr6/s\n",
		       timestamp[!curr]);
	}

	printf("%-11s %9.2f %9.2f %9.2f %9.2f %9.2f %9.2f %9.2f %9.2f %9.2f %9.2f\n",
	       timestamp[curr],
	       S_VALUE(snip->InReceives6,       snic->InReceives6,       itv),
	       S_VALUE(snip->OutForwDatagrams6, snic->OutForwDatagrams6, itv),
	       S_VALUE(snip->InDelivers6,       snic->InDelivers6,       itv),
	       S_VALUE(snip->OutRequests6,      snic->OutRequests6,      itv),
	       S_VALUE(snip->ReasmReqds6,       snic->ReasmReqds6,       itv),
	       S_VALUE(snip->ReasmOKs6,         snic->ReasmOKs6,         itv),
	       S_VALUE(snip->InMcastPkts6,      snic->InMcastPkts6,      itv),
	       S_VALUE(snip->OutMcastPkts6,     snic->OutMcastPkts6,     itv),
	       S_VALUE(snip->FragOKs6,          snic->FragOKs6,          itv),
	       S_VALUE(snip->FragCreates6,      snic->FragCreates6,      itv));
}

/*
 ***************************************************************************
 * Display IPv6 network error statistics.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @prev	Index in array where stats used as reference are.
 * @curr	Index in array for current sample statistics.
 * @itv		Interval of time in jiffies.
 ***************************************************************************
 */
__print_funct_t print_net_eip6_stats(struct activity *a, int prev, int curr,
				     unsigned long long itv)
{
	struct stats_net_eip6
		*sneic = (struct stats_net_eip6 *) a->buf[curr],
		*sneip = (struct stats_net_eip6 *) a->buf[prev];

	if (dis) {
		printf("\n%-11s ihdrer6/s iadrer6/s iukwnp6/s  i2big6/s  idisc6/s  odisc6/s"
		       "  inort6/s  onort6/s   asmf6/s  fragf6/s itrpck6/s\n",
		       timestamp[!curr]);
	}

	printf("%-11s %9.2f %9.2f %9.2f %9.2f %9.2f %9.2f %9.2f %9.2f %9.2f %9.2f %9.2f\n",
	       timestamp[curr],
	       S_VALUE(sneip->InHdrErrors6,     sneic->InHdrErrors6,     itv),
	       S_VALUE(sneip->InAddrErrors6,    sneic->InAddrErrors6,    itv),
	       S_VALUE(sneip->InUnknownProtos6, sneic->InUnknownProtos6, itv),
	       S_VALUE(sneip->InTooBigErrors6,  sneic->InTooBigErrors6,  itv),
	       S_VALUE(sneip->InDiscards6,      sneic->InDiscards6,      itv),
	       S_VALUE(sneip->OutDiscards6,     sneic->OutDiscards6,     itv),
	       S_VALUE(sneip->InNoRoutes6,      sneic->InNoRoutes6,      itv),
	       S_VALUE(sneip->OutNoRoutes6,     sneic->OutNoRoutes6,     itv),
	       S_VALUE(sneip->ReasmFails6,      sneic->ReasmFails6,      itv),
	       S_VALUE(sneip->FragFails6,       sneic->FragFails6,       itv),
	       S_VALUE(sneip->InTruncatedPkts6, sneic->InTruncatedPkts6, itv));
}

/*
 ***************************************************************************
 * Display ICMPv6 network traffic statistics.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @prev	Index in array where stats used as reference are.
 * @curr	Index in array for current sample statistics.
 * @itv		Interval of time in jiffies.
 ***************************************************************************
 */
__print_funct_t print_net_icmp6_stats(struct activity *a, int prev, int curr,
				      unsigned long long itv)
{
	struct stats_net_icmp6
		*snic = (struct stats_net_icmp6 *) a->buf[curr],
		*snip = (struct stats_net_icmp6 *) a->buf[prev];

	if (dis) {
		printf("\n%-11s   imsg6/s   omsg6/s   iech6/s  iechr6/s  oechr6/s"
		       "  igmbq6/s  igmbr6/s  ogmbr6/s igmbrd6/s ogmbrd6/s irtsol6/s ortsol6/s"
		       "  irtad6/s inbsol6/s onbsol6/s  inbad6/s  onbad6/s\n",
		       timestamp[!curr]);
	}

	printf("%-11s %9.2f %9.2f %9.2f %9.2f %9.2f %9.2f %9.2f %9.2f %9.2f %9.2f"
	       " %9.2f %9.2f %9.2f %9.2f %9.2f %9.2f %9.2f\n", timestamp[curr],
	       S_VALUE(snip->InMsgs6,                    snic->InMsgs6,                    itv),
	       S_VALUE(snip->OutMsgs6,                   snic->OutMsgs6,                   itv),
	       S_VALUE(snip->InEchos6,                   snic->InEchos6,                   itv),
	       S_VALUE(snip->InEchoReplies6,             snic->InEchoReplies6,             itv),
	       S_VALUE(snip->OutEchoReplies6,            snic->OutEchoReplies6,            itv),
	       S_VALUE(snip->InGroupMembQueries6,        snic->InGroupMembQueries6,        itv),
	       S_VALUE(snip->InGroupMembResponses6,      snic->InGroupMembResponses6,      itv),
	       S_VALUE(snip->OutGroupMembResponses6,     snic->OutGroupMembResponses6,     itv),
	       S_VALUE(snip->InGroupMembReductions6,     snic->InGroupMembReductions6,     itv),
	       S_VALUE(snip->OutGroupMembReductions6,    snic->OutGroupMembReductions6,    itv),
	       S_VALUE(snip->InRouterSolicits6,          snic->InRouterSolicits6,          itv),
	       S_VALUE(snip->OutRouterSolicits6,         snic->OutRouterSolicits6,         itv),
	       S_VALUE(snip->InRouterAdvertisements6,    snic->InRouterAdvertisements6,    itv),
	       S_VALUE(snip->InNeighborSolicits6,        snic->InNeighborSolicits6,        itv),
	       S_VALUE(snip->OutNeighborSolicits6,       snic->OutNeighborSolicits6,       itv),
	       S_VALUE(snip->InNeighborAdvertisements6,  snic->InNeighborAdvertisements6,  itv),
	       S_VALUE(snip->OutNeighborAdvertisements6, snic->OutNeighborAdvertisements6, itv));
}

/*
 ***************************************************************************
 * Display ICMPv6 network error statistics.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @prev	Index in array where stats used as reference are.
 * @curr	Index in array for current sample statistics.
 * @itv		Interval of time in jiffies.
 ***************************************************************************
 */
__print_funct_t print_net_eicmp6_stats(struct activity *a, int prev, int curr,
				       unsigned long long itv)
{
	struct stats_net_eicmp6
		*sneic = (struct stats_net_eicmp6 *) a->buf[curr],
		*sneip = (struct stats_net_eicmp6 *) a->buf[prev];

	if (dis) {
		printf("\n%-11s   ierr6/s idtunr6/s odtunr6/s  itmex6/s  otmex6/s"
		       " iprmpb6/s oprmpb6/s iredir6/s oredir6/s ipck2b6/s opck2b6/s\n",
		       timestamp[!curr]);
	}

	printf("%-11s %9.2f %9.2f %9.2f %9.2f %9.2f %9.2f %9.2f %9.2f %9.2f %9.2f"
	       " %9.2f\n", timestamp[curr],
	       S_VALUE(sneip->InErrors6,        sneic->InErrors6,        itv),
	       S_VALUE(sneip->InDestUnreachs6,  sneic->InDestUnreachs6,  itv),
	       S_VALUE(sneip->OutDestUnreachs6, sneic->OutDestUnreachs6, itv),
	       S_VALUE(sneip->InTimeExcds6,     sneic->InTimeExcds6,     itv),
	       S_VALUE(sneip->OutTimeExcds6,    sneic->OutTimeExcds6,    itv),
	       S_VALUE(sneip->InParmProblems6,  sneic->InParmProblems6,  itv),
	       S_VALUE(sneip->OutParmProblems6, sneic->OutParmProblems6, itv),
	       S_VALUE(sneip->InRedirects6,     sneic->InRedirects6,     itv),
	       S_VALUE(sneip->OutRedirects6,    sneic->OutRedirects6,    itv),
	       S_VALUE(sneip->InPktTooBigs6,    sneic->InPktTooBigs6,    itv),
	       S_VALUE(sneip->OutPktTooBigs6,   sneic->OutPktTooBigs6,   itv));
}

/*
 ***************************************************************************
 * Display UDPv6 network traffic statistics.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @prev	Index in array where stats used as reference are.
 * @curr	Index in array for current sample statistics.
 * @itv		Interval of time in jiffies.
 ***************************************************************************
 */
__print_funct_t print_net_udp6_stats(struct activity *a, int prev, int curr,
				     unsigned long long itv)
{
	struct stats_net_udp6
		*snuc = (struct stats_net_udp6 *) a->buf[curr],
		*snup = (struct stats_net_udp6 *) a->buf[prev];

	if (dis) {
		printf("\n%-11s   idgm6/s   odgm6/s noport6/s idgmer6/s\n",
		       timestamp[!curr]);
	}

	printf("%-11s %9.2f %9.2f %9.2f %9.2f\n",
	       timestamp[curr],
	       S_VALUE(snup->InDatagrams6,  snuc->InDatagrams6,  itv),
	       S_VALUE(snup->OutDatagrams6, snuc->OutDatagrams6, itv),
	       S_VALUE(snup->NoPorts6,      snuc->NoPorts6,      itv),
	       S_VALUE(snup->InErrors6,     snuc->InErrors6,     itv));
}

/*
 ***************************************************************************
 * Display CPU frequency statistics. This function is used to display
 * instantaneous and average statistics.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @curr	Index in array for current sample statistics.
 * @dispavg	True if displaying average statistics.
 ***************************************************************************
 */
void stub_print_pwr_cpufreq_stats(struct activity *a, int curr, int dispavg)
{
	int i;
	struct stats_pwr_cpufreq *spc;
	static unsigned long long
		*avg_cpufreq = NULL;
	
	if (!avg_cpufreq) {
		/* Allocate array of CPU frequency */
		if ((avg_cpufreq = (unsigned long long *) malloc(sizeof(unsigned long long) * a->nr))
		    == NULL) {
			perror("malloc");
			exit(4);
		}
		memset(avg_cpufreq, 0, sizeof(unsigned long long) * a->nr);
	}
	
	if (dis) {
		printf("\n%-11s     CPU       MHz\n",
		       timestamp[!curr]);
	}

	for (i = 0; (i < a->nr) && (i < a->bitmap->b_size + 1); i++) {
		
		/*
		 * The size of a->buf[...] CPU structure may be different from the default
		 * sizeof(struct stats_pwr_cpufreq) value if data have been read from a file!
		 * That's why we don't use a syntax like:
		 * spc = (struct stats_pwr_cpufreq *) a->buf[...] + i;
		 */
		spc = (struct stats_pwr_cpufreq *) ((char *) a->buf[curr] + i * a->msize);

		/*
		 * Note: a->nr is in [1, NR_CPUS + 1].
		 * Bitmap size is provided for (NR_CPUS + 1) CPUs.
		 * Anyway, NR_CPUS may vary between the version of sysstat
		 * used by sadc to create a file, and the version of sysstat
		 * used by sar to read it...
		 */
		
		/* Should current CPU (including CPU "all") be displayed? */
		if (a->bitmap->b_array[i >> 3] & (1 << (i & 0x07))) {

			/* Yes: Display it */
			printf("%-11s", timestamp[curr]);
			
			if (!i) {
				/* This is CPU "all" */
				printf("     all");
			}
			else {
				printf("     %3d", i - 1);
			}
			
			if (!dispavg) {
				/* Display instantaneous values */
				printf(" %9.2f\n",
				       ((double) spc->cpufreq) / 100);
				/*
				 * Will be used to compute the average.
				 * Note: overflow unlikely to happen but not impossible...
				 */
				avg_cpufreq[i] += spc->cpufreq;
			}
			else {
				/* Display average values */
				printf(" %9.2f\n",
				       (double) avg_cpufreq[i] / (100 * avg_count));				
			}
		}
	}
	
	if (dispavg) {
		/* Array of CPU frequency no longer needed: Free it! */
		if (avg_cpufreq) {
			free(avg_cpufreq);
			avg_cpufreq = NULL;
		}
	}
}

/*
 ***************************************************************************
 * Display CPU frequency statistics.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @prev	Index in array where stats used as reference are.
 * @curr	Index in array for current sample statistics.
 * @itv		Interval of time in jiffies.
 ***************************************************************************
 */
__print_funct_t print_pwr_cpufreq_stats(struct activity *a, int prev, int curr,
					unsigned long long itv)
{
	stub_print_pwr_cpufreq_stats(a, curr, FALSE);
}

/*
 ***************************************************************************
 * Display average CPU frequency statistics.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @prev	Index in array where stats used as reference are.
 * @curr	Index in array for current sample statistics.
 * @itv		Interval of time in jiffies.
 ***************************************************************************
 */
__print_funct_t print_avg_pwr_cpufreq_stats(struct activity *a, int prev, int curr,
					    unsigned long long itv)
{
	stub_print_pwr_cpufreq_stats(a, curr, TRUE);
}

/*
 ***************************************************************************
 * Display fan statistics. This function is used to display
 * instantaneous and average statistics.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @curr	Index in array for current sample statistics.
 * @dispavg	True if displaying average statistics.
 ***************************************************************************
 */
void stub_print_pwr_fan_stats(struct activity *a, int curr, int dispavg)
{
	int i;
	struct stats_pwr_fan *spc;
	static double *avg_fan = NULL;
	static double *avg_fan_min = NULL;

	/* Allocate arrays of fan RPMs */
	if (!avg_fan) {
		if ((avg_fan = (double *) malloc(sizeof(double) * a->nr)) == NULL) {
			perror("malloc");
			exit(4);
		}
		memset(avg_fan, 0, sizeof(double) * a->nr);
	}
	if (!avg_fan_min) {
		if ((avg_fan_min = (double *) malloc(sizeof(double) * a->nr)) == NULL) {
			perror("malloc");
			exit(4);
		}
		memset(avg_fan_min, 0, sizeof(double) * a->nr);
	}

	if (dis) {
		printf("\n%-11s     FAN       rpm      drpm %*s\n",
		       timestamp[!curr], MAX_SENSORS_DEV_LEN, "DEVICE");
	}

	for (i = 0; i < a->nr; i++) {
		spc = (struct stats_pwr_fan *) ((char *) a->buf[curr] + i * a->msize);

		printf("%-11s     %3d", timestamp[curr], i + 1);

		if (dispavg) {
			/* Display average values */
			printf(" %9.2f %9.2f",
			       (double) avg_fan[i] / avg_count,
			       (double) (avg_fan[i] - avg_fan_min[i]) / avg_count);
		}
		else {
			/* Display instantaneous values */
			printf(" %9.2f %9.2f",
			       spc->rpm,
			       spc->rpm - spc->rpm_min);
			avg_fan[i]     += spc->rpm;
			avg_fan_min[i] += spc->rpm_min;
		}

		printf(" %*s\n", MAX_SENSORS_DEV_LEN, spc->device);
	}

	if (dispavg) {
		if (avg_fan) {
			free(avg_fan);
			avg_fan = NULL;
		}
		if (avg_fan_min) {
			free(avg_fan_min);
			avg_fan_min = NULL;
		}
	}
}

/*
 ***************************************************************************
 * Display fan statistics.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @prev	Index in array where stats used as reference are.
 * @curr	Index in array for current sample statistics.
 * @itv		Interval of time in jiffies.
 ***************************************************************************
 */
__print_funct_t print_pwr_fan_stats(struct activity *a, int prev, int curr,
				    unsigned long long itv)
{
	stub_print_pwr_fan_stats(a, curr, FALSE);
}

/*
 ***************************************************************************
 * Display average fan statistics.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @prev	Index in array where stats used as reference are.
 * @curr	Index in array for current sample statistics.
 * @itv		Interval of time in jiffies.
 ***************************************************************************
 */
__print_funct_t print_avg_pwr_fan_stats(struct activity *a, int prev, int curr,
					unsigned long long itv)
{
	stub_print_pwr_fan_stats(a, curr, TRUE);
}

/*
 ***************************************************************************
 * Display device temperature statistics. This function is used to display
 * instantaneous and average statistics.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @curr	Index in array for current sample statistics.
 * @dispavg	True if displaying average statistics.
 ***************************************************************************
 */
void stub_print_pwr_temp_stats(struct activity *a, int curr, int dispavg)
{
	int i;
	struct stats_pwr_temp *spc;
	static double *avg_temp = NULL;
	static double *avg_temp_min = NULL, *avg_temp_max = NULL;

	/* Allocate arrays of temperatures */
	if (!avg_temp) {
		if ((avg_temp = (double *) malloc(sizeof(double) * a->nr)) == NULL) {
			perror("malloc");
			exit(4);
		}
		memset(avg_temp, 0, sizeof(double) * a->nr);
	}
	if (!avg_temp_min) {
		if ((avg_temp_min = (double *) malloc(sizeof(double) * a->nr)) == NULL) {
			perror("malloc");
			exit(4);
		}
		memset(avg_temp_min, 0, sizeof(double) * a->nr);
	}
	if (!avg_temp_max) {
		if ((avg_temp_max = (double *) malloc(sizeof(double) * a->nr)) == NULL) {
			perror("malloc");
			exit(4);
		}
		memset(avg_temp_max, 0, sizeof(double) * a->nr);
	}

	if (dis) {
		printf("\n%-11s    TEMP      degC     %%temp %*s\n",
		       timestamp[!curr], MAX_SENSORS_DEV_LEN, "DEVICE");
	}

	for (i = 0; i < a->nr; i++) {
		spc = (struct stats_pwr_temp *) ((char *) a->buf[curr] + i * a->msize);

		printf("%-11s     %3d", timestamp[curr], i + 1);

		if (dispavg) {
			/* Display average values */
			printf(" %9.2f %9.2f",
			       (double) avg_temp[i] / avg_count,
			       (avg_temp_max[i] - avg_temp_min[i]) ?
			       ((double) (avg_temp[i] / avg_count) - avg_temp_min[i]) / (avg_temp_max[i] - avg_temp_min[i]) * 100 :
			       0.0);
		}
		else {
			/* Display instantaneous values */
			printf(" %9.2f %9.2f",
			       spc->temp,
			       (spc->temp_max - spc->temp_min) ?
			       (spc->temp - spc->temp_min) / (spc->temp_max - spc->temp_min) * 100 :
			       0.0);
			avg_temp[i] += spc->temp;
			/* Assume that min and max temperatures cannot vary */
			avg_temp_min[i] = spc->temp_min;
			avg_temp_max[i] = spc->temp_max;
		}
		
		printf(" %*s\n", MAX_SENSORS_DEV_LEN, spc->device);
	}

	if (dispavg) {
		if (avg_temp) {
			free(avg_temp);
			avg_temp = NULL;
		}
		if (avg_temp_min) {
			free(avg_temp_min);
			avg_temp_min = NULL;
		}
		if (avg_temp_max) {
			free(avg_temp_max);
			avg_temp_max = NULL;
		}
	}
}

/*
 ***************************************************************************
 * Display temperature statistics.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @prev	Index in array where stats used as reference are.
 * @curr	Index in array for current sample statistics.
 * @itv		Interval of time in jiffies.
 ***************************************************************************
 */
__print_funct_t print_pwr_temp_stats(struct activity *a, int prev, int curr,
				     unsigned long long itv)
{
	stub_print_pwr_temp_stats(a, curr, FALSE);
}

/*
 ***************************************************************************
 * Display average temperature statistics.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @prev	Index in array where stats used as reference are.
 * @curr	Index in array for current sample statistics.
 * @itv		Interval of time in jiffies.
 ***************************************************************************
 */
__print_funct_t print_avg_pwr_temp_stats(struct activity *a, int prev, int curr,
					 unsigned long long itv)
{
	stub_print_pwr_temp_stats(a, curr, TRUE);
}

/*
 ***************************************************************************
 * Display voltage inputs statistics. This function is used to display
 * instantaneous and average statistics.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @curr	Index in array for current sample statistics.
 * @dispavg	True if displaying average statistics.
 ***************************************************************************
 */
void stub_print_pwr_in_stats(struct activity *a, int curr, int dispavg)
{
	int i;
	struct stats_pwr_in *spc;
	static double *avg_in = NULL;
	static double *avg_in_min = NULL, *avg_in_max = NULL;

	/* Allocate arrays of voltage inputs */
	if (!avg_in) {
		if ((avg_in = (double *) malloc(sizeof(double) * a->nr)) == NULL) {
			perror("malloc");
			exit(4);
		}
		memset(avg_in, 0, sizeof(double) * a->nr);
	}
	if (!avg_in_min) {
		if ((avg_in_min = (double *) malloc(sizeof(double) * a->nr)) == NULL) {
			perror("malloc");
			exit(4);
		}
		memset(avg_in_min, 0, sizeof(double) * a->nr);
	}
	if (!avg_in_max) {
		if ((avg_in_max = (double *) malloc(sizeof(double) * a->nr)) == NULL) {
			perror("malloc");
			exit(4);
		}
		memset(avg_in_max, 0, sizeof(double) * a->nr);
	}

	if (dis) {
		printf("\n%-11s      IN       inV       %%in %*s\n",
		       timestamp[!curr], MAX_SENSORS_DEV_LEN, "DEVICE");
	}

	for (i = 0; i < a->nr; i++) {
		spc = (struct stats_pwr_in *) ((char *) a->buf[curr] + i * a->msize);

		printf("%-11s     %3d", timestamp[curr], i);

		if (dispavg) {
			/* Display average values */
			printf(" %9.2f %9.2f",
			       (double) avg_in[i] / avg_count,
			       (avg_in_max[i] - avg_in_min[i]) ?
			       ((double) (avg_in[i] / avg_count) - avg_in_min[i]) / (avg_in_max[i] - avg_in_min[i]) * 100 :
			       0.0);
		}
		else {
			/* Display instantaneous values */
			printf(" %9.2f %9.2f",
			       spc->in,
			       (spc->in_max - spc->in_min) ?
			       (spc->in - spc->in_min) / (spc->in_max - spc->in_min) * 100 :
			       0.0);
			avg_in[i] += spc->in;
			/* Assume that min and max voltage inputs cannot vary */
			avg_in_min[i] = spc->in_min;
			avg_in_max[i] = spc->in_max;
		}

		printf(" %*s\n", MAX_SENSORS_DEV_LEN, spc->device);
	}

	if (dispavg) {
		if (avg_in) {
			free(avg_in);
			avg_in = NULL;
		}
		if (avg_in_min) {
			free(avg_in_min);
			avg_in_min = NULL;
		}
		if (avg_in_max) {
			free(avg_in_max);
			avg_in_max = NULL;
		}
	}
}

/*
 ***************************************************************************
 * Display voltage inputs statistics.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @prev	Index in array where stats used as reference are.
 * @curr	Index in array for current sample statistics.
 * @itv		Interval of time in jiffies.
 ***************************************************************************
 */
__print_funct_t print_pwr_in_stats(struct activity *a, int prev, int curr,
				   unsigned long long itv)
{
	stub_print_pwr_in_stats(a, curr, FALSE);
}

/*
 ***************************************************************************
 * Display average voltage inputs statistics.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @prev	Index in array where stats used as reference are.
 * @curr	Index in array for current sample statistics.
 * @itv		Interval of time in jiffies.
 ***************************************************************************
 */
__print_funct_t print_avg_pwr_in_stats(struct activity *a, int prev, int curr,
				       unsigned long long itv)
{
	stub_print_pwr_in_stats(a, curr, TRUE);
}

/*
 ***************************************************************************
 * Display huge pages statistics. This function is used to
 * display instantaneous and average statistics.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @curr	Index in array for current sample statistics.
 * @dispavg	TRUE if displaying average statistics.
 ***************************************************************************
 */
void stub_print_huge_stats(struct activity *a, int curr, int dispavg)
{
	struct stats_huge
		*smc = (struct stats_huge *) a->buf[curr];
	static unsigned long long
		avg_frhkb = 0,
		avg_tlhkb = 0;

	if (dis) {
		printf("\n%-11s kbhugfree kbhugused  %%hugused\n",
		       timestamp[!curr]);
	}

	if (!dispavg) {
		/* Display instantaneous values */
		printf("%-11s %9lu %9lu    %6.2f\n",
		       timestamp[curr],
		       smc->frhkb,
		       smc->tlhkb - smc->frhkb,
		       smc->tlhkb ?
		       SP_VALUE(smc->frhkb, smc->tlhkb, smc->tlhkb) : 0.0);

		/* Will be used to compute the average */
		avg_frhkb += smc->frhkb;
		avg_tlhkb += smc->tlhkb;
	}
	else {
		/* Display average values */
		printf("%-11s %9.0f %9.0f    %6.2f\n",
		       timestamp[curr],
		       (double) avg_frhkb / avg_count,
		       ((double) avg_tlhkb / avg_count) -
		       ((double) avg_frhkb / avg_count),
		       ((double) (avg_tlhkb / avg_count)) ?
		       SP_VALUE((double) (avg_frhkb / avg_count),
				(double) (avg_tlhkb / avg_count),
				(double) (avg_tlhkb / avg_count)) :
		       0.0);

		/* Reset average counters */
		avg_frhkb = avg_tlhkb = 0;
	}
}

/*
 ***************************************************************************
 * Display huge pages statistics.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @prev	Index in array where stats used as reference are.
 * @curr	Index in array for current sample statistics.
 * @itv		Interval of time in jiffies.
 ***************************************************************************
 */
__print_funct_t print_huge_stats(struct activity *a, int prev, int curr,
				 unsigned long long itv)
{
	stub_print_huge_stats(a, curr, FALSE);
}

/*
 ***************************************************************************
 * Display huge pages statistics.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @prev	Index in array where stats used as reference are.
 * @curr	Index in array for current sample statistics.
 * @itv		Interval of time in jiffies.
 ***************************************************************************
 */
__print_funct_t print_avg_huge_stats(struct activity *a, int prev, int curr,
				     unsigned long long itv)
{
	stub_print_huge_stats(a, curr, TRUE);
}

/*
 ***************************************************************************
 * Display CPU weighted frequency statistics. This function is used to
 * display instantaneous and average statistics.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @prev	Index in array where stats used as reference are.
 * @curr	Index in array for current sample statistics.
 * @itv		Interval of time in jiffies.
 ***************************************************************************
 */
void print_pwr_wghfreq_stats(struct activity *a, int prev, int curr,
			     unsigned long long itv)
{
	int i, k;
	struct stats_pwr_wghfreq *spc, *spp, *spc_k, *spp_k;
	unsigned long long tis, tisfreq;

	if (dis) {
		printf("\n%-11s     CPU    wghMHz\n",
		       timestamp[!curr]);
	}

	for (i = 0; (i < a->nr) && (i < a->bitmap->b_size + 1); i++) {

		/*
		 * The size of a->buf[...] CPU structure may be different from the default
		 * sizeof(struct stats_pwr_wghfreq) value if data have been read from a file!
		 * That's why we don't use a syntax like:
		 * spc = (struct stats_pwr_wghfreq *) a->buf[...] + i;
		 */
		spc = (struct stats_pwr_wghfreq *) ((char *) a->buf[curr] + i * a->msize * a->nr2);
		spp = (struct stats_pwr_wghfreq *) ((char *) a->buf[prev] + i * a->msize * a->nr2);

		/*
		 * Note: a->nr is in [1, NR_CPUS + 1].
		 * Bitmap size is provided for (NR_CPUS + 1) CPUs.
		 * Anyway, NR_CPUS may vary between the version of sysstat
		 * used by sadc to create a file, and the version of sysstat
		 * used by sar to read it...
		 */

		/* Should current CPU (including CPU "all") be displayed? */
		if (a->bitmap->b_array[i >> 3] & (1 << (i & 0x07))) {

			/* Yes: Display it */
			printf("%-11s", timestamp[curr]);

			if (!i) {
				/* This is CPU "all" */
				printf("     all");
			}
			else {
				printf("     %3d", i - 1);
			}

			tisfreq = 0;
			tis = 0;

			for (k = 0; k < a->nr2; k++) {

				spc_k = (struct stats_pwr_wghfreq *) ((char *) spc + k * a->msize);
				if (!spc_k->freq)
					break;
				spp_k = (struct stats_pwr_wghfreq *) ((char *) spp + k * a->msize);

				tisfreq += (spc_k->freq / 1000) *
				           (spc_k->time_in_state - spp_k->time_in_state);
				tis     += (spc_k->time_in_state - spp_k->time_in_state);
			}

			/* Display weighted frequency for current CPU */
			printf(" %9.2f\n", tis ? ((double) tisfreq) / tis : 0.0);
		}
	}
}

/*
 ***************************************************************************
 * Display USB devices statistics. This function is used to
 * display instantaneous and summary statistics.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @curr	Index in array for current sample statistics.
 * @itv		Interval of time in jiffies.
 * @dispavg	TRUE if displaying average statistics.
 ***************************************************************************
 */
void stub_print_pwr_usb_stats(struct activity *a, int curr, int dispavg)
{
	int i, j;
	struct stats_pwr_usb *suc, *sum;

	if (dis) {
		printf("\n%-11s     BUS  idvendor    idprod  maxpower",
		       (dispavg ? _("Summary") : timestamp[!curr]));
		printf(" %*s", MAX_MANUF_LEN - 1, "manufact");
		printf(" %*s\n", MAX_PROD_LEN - 1, "product");
	}
	
	for (i = 0; i < a->nr; i++) {
		suc = (struct stats_pwr_usb *) ((char *) a->buf[curr] + i * a->msize);

		if (!suc->bus_nr)
			/* Bus#0 doesn't exist: We are at the end of the list */
			break;
		
		printf("%-11s  %6d %9x %9x %9u",
		       (dispavg ? _("Summary") : timestamp[curr]),
		       suc->bus_nr,
		       suc->vendor_id, suc->product_id,
		       /* bMaxPower is expressed in 2 mA units */
		       suc->bmaxpower << 1);

		printf(" %*s", MAX_MANUF_LEN - 1, suc->manufacturer);
		printf(" %*s\n", MAX_PROD_LEN - 1, suc->product);

		if (!dispavg) {
			/* Save current USB device in summary list */
			for (j = 0; j < a->nr; j++) {
				sum = (struct stats_pwr_usb *) ((char *) a->buf[2] + j * a->msize);

				if ((sum->bus_nr     == suc->bus_nr) &&
				    (sum->vendor_id  == suc->vendor_id) &&
				    (sum->product_id == suc->product_id))
					/* USB device found in summary list */
					break;
				if (!sum->bus_nr) {
					/*
					 * Current slot is free:
					 * Save USB device in summary list.
					 */
					*sum = *suc;
					break;
				}
				if (j == a->nr - 1) {
					/*
					 * This is the last slot and
					 * we still havent't found the device.
					 * So create a dummy device here.
					 */
					sum->bus_nr = ~0;
					sum->vendor_id = sum->product_id = 0;
					sum->bmaxpower = 0;
					sum->manufacturer[0] = '\0';
					snprintf(sum->product, MAX_PROD_LEN, "%s",
						 _("Other devices not listed here"));
					sum->product[MAX_PROD_LEN - 1] = '\0';
				}
			}
		}
	}
}	

/*
 ***************************************************************************
 * Display memory and swap statistics.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @prev	Index in array where stats used as reference are.
 * @curr	Index in array for current sample statistics.
 * @itv		Interval of time in jiffies.
 ***************************************************************************
 */
__print_funct_t print_pwr_usb_stats(struct activity *a, int prev, int curr,
				   unsigned long long itv)
{
	stub_print_pwr_usb_stats(a, curr, FALSE);
}

/*
 ***************************************************************************
 * Display average memory statistics.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @prev	Index in array where stats used as reference are.
 * @curr	Index in array for current sample statistics.
 * @itv		Interval of time in jiffies.
 ***************************************************************************
 */
__print_funct_t print_avg_pwr_usb_stats(struct activity *a, int prev, int curr,
					unsigned long long itv)
{
	stub_print_pwr_usb_stats(a, 2, TRUE);
}
