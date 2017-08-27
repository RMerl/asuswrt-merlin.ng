/*
 * json_stats.c: Funtions used by sadf to display statistics in JSON format.
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

#include "sa.h"
#include "sadf.h"
#include "ioconf.h"
#include "json_stats.h"

#ifdef USE_NLS
#include <locale.h>
#include <libintl.h>
#define _(string) gettext(string)
#else
#define _(string) (string)
#endif

extern unsigned int flags;
extern unsigned int dm_major;

/*
 ***************************************************************************
 * Open or close "network" markup.
 *
 * IN:
 * @tab		Number of tabulations.
 * @action	Open or close action.
 ***************************************************************************
 */
void json_markup_network(int tab, int action)
{
	static int markup_state = CLOSE_JSON_MARKUP;

	if (action == markup_state)
		return;
	markup_state = action;

	if (action == OPEN_JSON_MARKUP) {
		/* Open markup */
		xprintf(tab, "\"network\": {");
	}
	else {
		/* Close markup */
		printf("\n");
		xprintf0(tab, "}");
	}
}

/*
 ***************************************************************************
 * Open or close "power-management" markup.
 *
 * IN:
 * @tab		Number of tabulations.
 * @action	Open or close action.
 ***************************************************************************
 */
void json_markup_power_management(int tab, int action)
{
	static int markup_state = CLOSE_JSON_MARKUP;

	if (action == markup_state)
		return;
	markup_state = action;

	if (action == OPEN_JSON_MARKUP) {
		/* Open markup */
		xprintf(tab, "\"power-management\": {");
	}
	else {
		/* Close markup */
		printf("\n");
		xprintf0(tab, "}");
	}
}

/*
 ***************************************************************************
 * Display CPU statistics in JSON.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @curr	Index in array for current sample statistics.
 * @tab		Indentation in output.
 * @g_itv	Interval of time in jiffies mutliplied by the number of
 * 		processors.
 ***************************************************************************
 */
__print_funct_t json_print_cpu_stats(struct activity *a, int curr, int tab,
				     unsigned long long g_itv)
{
	int i, cpu_offline;
	int sep = FALSE;
	struct stats_cpu *scc, *scp;
	char cpuno[8];

	if (DISPLAY_CPU_DEF(a->opt_flags)) {
		xprintf(tab++, "\"cpu-load\": [");
	}
	else if (DISPLAY_CPU_ALL(a->opt_flags)) {
		xprintf(tab++, "\"cpu-load-all\": [");
	}

	for (i = 0; (i < a->nr) && (i < a->bitmap->b_size + 1); i++) {
		
		scc = (struct stats_cpu *) ((char *) a->buf[curr]  + i * a->msize);
		scp = (struct stats_cpu *) ((char *) a->buf[!curr] + i * a->msize);

		/* Should current CPU (including CPU "all") be displayed? */
		if (a->bitmap->b_array[i >> 3] & (1 << (i & 0x07))) {
			
			/* Yes: Display current CPU stats */

			if (sep) {
				printf(",\n");
			}
			sep = TRUE;
			
			if (!i) {
				/* This is CPU "all" */
				strcpy(cpuno, "all");
			}
			else {
				sprintf(cpuno, "%d", i - 1);

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

					g_itv = 0;
					cpu_offline = TRUE;
				}
				else {
					/*
					 * Recalculate interval for current proc.
					 * If result is 0 then current CPU is a tickless one.
					 */
					g_itv = get_per_cpu_interval(scc, scp);
					cpu_offline = FALSE;
				}
				
				if (!g_itv) {
					/* Current CPU is offline or tickless */
					if (DISPLAY_CPU_DEF(a->opt_flags)) {
						xprintf0(tab, "{\"cpu\": \"%d\", "
							 "\"user\": %.2f, "
							 "\"nice\": %.2f, "
							 "\"system\": %.2f, "
							 "\"iowait\": %.2f, "
							 "\"steal\": %.2f, "
							 "\"idle\": %.2f}",
							 i - 1, 0.0, 0.0, 0.0, 0.0, 0.0,
							 cpu_offline ? 0.0 : 100.0);
					}
					else if (DISPLAY_CPU_ALL(a->opt_flags)) {
						xprintf0(tab, "{\"cpu\": \"%d\", "
							 "\"usr\": %.2f, "
							 "\"nice\": %.2f, "
							 "\"sys\": %.2f, "
							 "\"iowait\": %.2f, "
							 "\"steal\": %.2f, "
							 "\"irq\": %.2f, "
							 "\"soft\": %.2f, "
							 "\"guest\": %.2f, "
							 "\"idle\": %.2f}",
							 i - 1, 0.0, 0.0, 0.0, 0.0,
							 0.0, 0.0, 0.0, 0.0,
							 cpu_offline ? 0.0 : 100.0);
					}
					continue;
				}
			}

			if (DISPLAY_CPU_DEF(a->opt_flags)) {
				xprintf0(tab, "{\"cpu\": \"%s\", "
					 "\"user\": %.2f, "
					 "\"nice\": %.2f, "
					 "\"system\": %.2f, "
					 "\"iowait\": %.2f, "
					 "\"steal\": %.2f, "
					 "\"idle\": %.2f}",
					 cpuno,
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
				xprintf0(tab, "{\"cpu\": \"%s\", "
					 "\"usr\": %.2f, "
					 "\"nice\": %.2f, "
					 "\"sys\": %.2f, "
					 "\"iowait\": %.2f, "
					 "\"steal\": %.2f, "
					 "\"irq\": %.2f, "
					 "\"soft\": %.2f, "
					 "\"guest\": %.2f, "
					 "\"idle\": %.2f}",
					 cpuno,
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
					 ll_sp_value(scp->cpu_idle,   scc->cpu_idle,   g_itv));
			}
		}
	}

	printf("\n");
	xprintf0(--tab, "]");
}

/*
 ***************************************************************************
 * Display task creation and context switch statistics in JSON.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @curr	Index in array for current sample statistics.
 * @tab		Indentation in output.
 * @itv		Interval of time in jiffies.
 ***************************************************************************
 */
__print_funct_t json_print_pcsw_stats(struct activity *a, int curr, int tab,
				      unsigned long long itv)
{
	struct stats_pcsw
		*spc = (struct stats_pcsw *) a->buf[curr],
		*spp = (struct stats_pcsw *) a->buf[!curr];

	/* proc/s and cswch/s */
	xprintf0(tab, "\"process-and-context-switch\": {"
		 "\"proc\": %.2f, "
		 "\"cswch\": %.2f}",
		 S_VALUE(spp->processes, spc->processes, itv),
		 ll_s_value(spp->context_switch, spc->context_switch, itv));
}

/*
 ***************************************************************************
 * Display interrupts statistics in JSON.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @curr	Index in array for current sample statistics.
 * @tab		Indentation in output.
 * @itv		Interval of time in jiffies.
 ***************************************************************************
 */
__print_funct_t json_print_irq_stats(struct activity *a, int curr, int tab,
				     unsigned long long itv)
{
	int i;
	struct stats_irq *sic, *sip;
	int sep = FALSE;
	char irqno[8];
	
	xprintf(tab++, "\"interrupts\": [");

	for (i = 0; (i < a->nr) && (i < a->bitmap->b_size + 1); i++) {

		sic = (struct stats_irq *) ((char *) a->buf[curr]  + i * a->msize);
		sip = (struct stats_irq *) ((char *) a->buf[!curr] + i * a->msize);
		
		/* Should current interrupt (including int "sum") be displayed? */
		if (a->bitmap->b_array[i >> 3] & (1 << (i & 0x07))) {
			
			/* Yes: Display it */

			if (sep) {
				printf(",\n");
			}
			sep = TRUE;

			if (!i) {
				/* This is interrupt "sum" */
				strcpy(irqno, "sum");
			}
			else {
				sprintf(irqno, "%d", i - 1);
			}

			xprintf0(tab, "{\"intr\": \"%s\", "
				 "\"value\": %.2f}",
				 irqno,
				 ll_s_value(sip->irq_nr, sic->irq_nr, itv));
		}
	}

	printf("\n");
	xprintf0(--tab, "]");
}

/*
 ***************************************************************************
 * Display swapping statistics in JSON.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @curr	Index in array for current sample statistics.
 * @tab		Indentation in output.
 * @itv		Interval of time in jiffies.
 ***************************************************************************
 */
__print_funct_t json_print_swap_stats(struct activity *a, int curr, int tab,
				      unsigned long long itv)
{
	struct stats_swap
		*ssc = (struct stats_swap *) a->buf[curr],
		*ssp = (struct stats_swap *) a->buf[!curr];
	
	xprintf0(tab, "\"swap-pages\": {"
		 "\"pswpin\": %.2f, "
		 "\"pswpout\": %.2f}",
		 S_VALUE(ssp->pswpin,  ssc->pswpin,  itv),
		 S_VALUE(ssp->pswpout, ssc->pswpout, itv));
}

/*
 ***************************************************************************
 * Display paging statistics in JSON.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @curr	Index in array for current sample statistics.
 * @tab		Indentation in output.
 * @itv		Interval of time in jiffies.
 ***************************************************************************
 */
__print_funct_t json_print_paging_stats(struct activity *a, int curr, int tab,
					unsigned long long itv)
{
	struct stats_paging
		*spc = (struct stats_paging *) a->buf[curr],
		*spp = (struct stats_paging *) a->buf[!curr];

	xprintf0(tab, "\"paging\": {"
		 "\"pgpgin\": %.2f, "
		 "\"pgpgout\": %.2f, "
		 "\"fault\": %.2f, "
		 "\"majflt\": %.2f, "
		 "\"pgfree\": %.2f, "
		 "\"pgscank\": %.2f, "
		 "\"pgscand\": %.2f, "
		 "\"pgsteal\": %.2f, "
		 "\"vmeff-percent\": %.2f}",
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
 * Display I/O and transfer rate statistics in JSON.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @curr	Index in array for current sample statistics.
 * @tab		Indentation in output.
 * @itv		Interval of time in jiffies.
 ***************************************************************************
 */
__print_funct_t json_print_io_stats(struct activity *a, int curr, int tab,
				    unsigned long long itv)
{
	struct stats_io
		*sic = (struct stats_io *) a->buf[curr],
		*sip = (struct stats_io *) a->buf[!curr];

	xprintf0(tab, "\"io\": {"
		 "\"tps\": %.2f, "
		 "\"io-reads\": {"
		 "\"rtps\": %.2f, "
		 "\"bread\": %.2f}, "
		 "\"io-writes\": {"
		 "\"wtps\": %.2f, "
		 "\"bwrtn\": %.2f}}",
		 S_VALUE(sip->dk_drive, sic->dk_drive, itv),
		 S_VALUE(sip->dk_drive_rio,  sic->dk_drive_rio,  itv),
		 S_VALUE(sip->dk_drive_rblk, sic->dk_drive_rblk, itv),
		 S_VALUE(sip->dk_drive_wio,  sic->dk_drive_wio,  itv),
		 S_VALUE(sip->dk_drive_wblk, sic->dk_drive_wblk, itv));
}

/*
 ***************************************************************************
 * Display memory statistics in JSON.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @curr	Index in array for current sample statistics.
 * @tab		Indentation in output.
 * @itv		Interval of time in jiffies.
 ***************************************************************************
 */
__print_funct_t json_print_memory_stats(struct activity *a, int curr, int tab,
					unsigned long long itv)
{
	struct stats_memory
		*smc = (struct stats_memory *) a->buf[curr],
		*smp = (struct stats_memory *) a->buf[!curr];
	int sep = FALSE;

	xprintf0(tab, "\"memory\": {");
	
	if (DISPLAY_MEM_AMT(a->opt_flags)) {

		sep = TRUE;

		printf("\"memfree\": %lu, "
		       "\"memused\": %lu, "
		       "\"memused-percent\": %.2f, "
		       "\"buffers\": %lu, "
		       "\"cached\": %lu, "
		       "\"commit\": %lu, "
		       "\"commit-percent\": %.2f, "
		       "\"active\": %lu, "
		       "\"inactive\": %lu",
		       smc->frmkb,
		       smc->tlmkb - smc->frmkb,
		       smc->tlmkb ?
		       SP_VALUE(smc->frmkb, smc->tlmkb, smc->tlmkb) :
		       0.0,
		       smc->bufkb,
		       smc->camkb,
		       smc->comkb,
		       (smc->tlmkb + smc->tlskb) ?
		       SP_VALUE(0, smc->comkb, smc->tlmkb + smc->tlskb) :
		       0.0,
		       smc->activekb,
		       smc->inactkb);
	}

	if (DISPLAY_SWAP(a->opt_flags)) {

		if (sep) {
			printf(", ");
		}
		sep = TRUE;

		printf("\"swpfree\": %lu, "
		       "\"swpused\": %lu, "
		       "\"swpused-percent\": %.2f, "
		       "\"swpcad\": %lu, "
		       "\"swpcad-percent\": %.2f",
		       smc->frskb,
		       smc->tlskb - smc->frskb,
		       smc->tlskb ?
		       SP_VALUE(smc->frskb, smc->tlskb, smc->tlskb) :
		       0.0,
		       smc->caskb,
		       (smc->tlskb - smc->frskb) ?
		       SP_VALUE(0, smc->caskb, smc->tlskb - smc->frskb) :
		       0.0);
	}

	if (DISPLAY_MEMORY(a->opt_flags)) {

		if (sep) {
			printf(", ");
		}
		
		printf("\"frmpg\": %.2f, "
		       "\"bufpg\": %.2f, "
		       "\"campg\": %.2f",
		       S_VALUE((double) KB_TO_PG(smp->frmkb),
			       (double) KB_TO_PG(smc->frmkb), itv),
		       S_VALUE((double) KB_TO_PG(smp->bufkb),
			       (double) KB_TO_PG(smc->bufkb), itv),
		       S_VALUE((double) KB_TO_PG(smp->camkb),
			       (double) KB_TO_PG(smc->camkb), itv));
	}

	printf("}");
}

/*
 ***************************************************************************
 * Display kernel tables statistics in JSON.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @curr	Index in array for current sample statistics.
 * @tab		Indentation in output.
 * @itv		Interval of time in jiffies.
 ***************************************************************************
 */
__print_funct_t json_print_ktables_stats(struct activity *a, int curr, int tab,
					 unsigned long long itv)
{
	struct stats_ktables
		*skc = (struct stats_ktables *) a->buf[curr];
	
	xprintf0(tab, "\"kernel\": {"
		 "\"dentunusd\": %u, "
		 "\"file-nr\": %u, "
		 "\"inode-nr\": %u, "
		 "\"pty-nr\": %u}",
		 skc->dentry_stat,
		 skc->file_used,
		 skc->inode_used,
		 skc->pty_nr);
}

/*
 ***************************************************************************
 * Display queue and load statistics in JSON.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @curr	Index in array for current sample statistics.
 * @tab		Indentation in output.
 * @itv		Interval of time in jiffies.
 ***************************************************************************
 */
__print_funct_t json_print_queue_stats(struct activity *a, int curr, int tab,
				       unsigned long long itv)
{
	struct stats_queue
		*sqc = (struct stats_queue *) a->buf[curr];
	
	xprintf0(tab, "\"queue\": {"
		 "\"runq-sz\": %lu, "
		 "\"plist-sz\": %u, "
		 "\"ldavg-1\": %.2f, "
		 "\"ldavg-5\": %.2f, "
		 "\"ldavg-15\": %.2f, "
		 "\"blocked\": %lu}",
		 sqc->nr_running,
		 sqc->nr_threads,
		 (double) sqc->load_avg_1 / 100,
		 (double) sqc->load_avg_5 / 100,
		 (double) sqc->load_avg_15 / 100,
		 sqc->procs_blocked);
}

/*
 ***************************************************************************
 * Display serial lines statistics in JSON.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @curr	Index in array for current sample statistics.
 * @tab		Indentation in output.
 * @itv		Interval of time in jiffies.
 ***************************************************************************
 */
__print_funct_t json_print_serial_stats(struct activity *a, int curr, int tab,
					unsigned long long itv)
{
	int i;
	struct stats_serial *ssc, *ssp;
	int sep = FALSE;

	xprintf(tab++, "\"serial\": [");

	for (i = 0; i < a->nr; i++) {

		ssc = (struct stats_serial *) ((char *) a->buf[curr]  + i * a->msize);
		ssp = (struct stats_serial *) ((char *) a->buf[!curr] + i * a->msize);

		if (ssc->line == 0)
			continue;

		if (ssc->line == ssp->line) {

			if (sep) {
				printf(",\n");
			}
			sep = TRUE;
			
			xprintf0(tab, "{\"line\": %d, "
				 "\"rcvin\": %.2f, "
				 "\"xmtin\": %.2f, "
				 "\"framerr\": %.2f, "
				 "\"prtyerr\": %.2f, "
				 "\"brk\": %.2f, "
				 "\"ovrun\": %.2f}",
				 ssc->line - 1,
				 S_VALUE(ssp->rx,      ssc->rx,      itv),
				 S_VALUE(ssp->tx,      ssc->tx,      itv),
				 S_VALUE(ssp->frame,   ssc->frame,   itv),
				 S_VALUE(ssp->parity,  ssc->parity,  itv),
				 S_VALUE(ssp->brk,     ssc->brk,     itv),
				 S_VALUE(ssp->overrun, ssc->overrun, itv));
		}
	}

	printf("\n");
	xprintf0(--tab, "]");
}

/*
 ***************************************************************************
 * Display disks statistics in JSON.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @curr	Index in array for current sample statistics.
 * @tab		Indentation in output.
 * @itv		Interval of time in jiffies.
 ***************************************************************************
 */
__print_funct_t json_print_disk_stats(struct activity *a, int curr, int tab,
				      unsigned long long itv)
{
	int i, j;
	struct stats_disk *sdc,	*sdp;
	struct ext_disk_stats xds;
	int sep = FALSE;
	char *dev_name;

	xprintf(tab++, "\"disk\": [");

	for (i = 0; i < a->nr; i++) {

		sdc = (struct stats_disk *) ((char *) a->buf[curr] + i * a->msize);

		if (!(sdc->major + sdc->minor))
			continue;

		j = check_disk_reg(a, curr, !curr, i);
		sdp = (struct stats_disk *) ((char *) a->buf[!curr] + j * a->msize);

		/* Compute extended statistics values */
		compute_ext_disk_stats(sdc, sdp, itv, &xds);
		
		dev_name = NULL;

		if ((USE_PRETTY_OPTION(flags)) && (sdc->major == dm_major)) {
			dev_name = transform_devmapname(sdc->major, sdc->minor);
		}

		if (!dev_name) {
			dev_name = get_devname(sdc->major, sdc->minor,
					       USE_PRETTY_OPTION(flags));
		}

		if (sep) {
			printf(",\n");
		}
		sep = TRUE;
		
		xprintf0(tab, "{\"disk-device\": \"%s\", "
			 "\"tps\": %.2f, "
			 "\"rd_sec\": %.2f, "
			 "\"wr_sec\": %.2f, "
			 "\"avgrq-sz\": %.2f, "
			 "\"avgqu-sz\": %.2f, "
			 "\"await\": %.2f, "
			 "\"svctm\": %.2f, "
			 "\"util-percent\": %.2f}",
			 /* Confusion possible here between index and minor numbers */
			 dev_name,
			 S_VALUE(sdp->nr_ios, sdc->nr_ios, itv),
			 ll_s_value(sdp->rd_sect, sdc->rd_sect, itv),
			 ll_s_value(sdp->wr_sect, sdc->wr_sect, itv),
			 /* See iostat for explanations */
			 xds.arqsz,
			 S_VALUE(sdp->rq_ticks, sdc->rq_ticks, itv) / 1000.0,
			 xds.await,
			 xds.svctm,
			 xds.util / 10.0);
	}

	printf("\n");
	xprintf0(--tab, "]");
}

/*
 ***************************************************************************
 * Display network interfaces statistics in JSON.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @curr	Index in array for current sample statistics.
 * @tab		Indentation in output.
 * @itv		Interval of time in jiffies.
 ***************************************************************************
 */
__print_funct_t json_print_net_dev_stats(struct activity *a, int curr, int tab,
					 unsigned long long itv)
{
	int i, j;
	struct stats_net_dev *sndc, *sndp;
	int sep = FALSE;

	if (!IS_SELECTED(a->options) || (a->nr <= 0))
		goto close_json_markup;

	json_markup_network(tab, OPEN_JSON_MARKUP);
	tab++;
	
	xprintf(tab++, "\"net-dev\": [");

	for (i = 0; i < a->nr; i++) {

		sndc = (struct stats_net_dev *) ((char *) a->buf[curr] + i * a->msize);

		if (!strcmp(sndc->interface, ""))
			continue;
		
		j = check_net_dev_reg(a, curr, !curr, i);
		sndp = (struct stats_net_dev *) ((char *) a->buf[!curr] + j * a->msize);

		if (sep) {
			printf(",\n");
		}
		sep = TRUE;
		
		xprintf0(tab, "{\"iface\": \"%s\", "
			 "\"rxpck\": %.2f, "
			 "\"txpck\": %.2f, "
			 "\"rxkB\": %.2f, "
			 "\"txkB\": %.2f, "
			 "\"rxcmp\": %.2f, "
			 "\"txcmp\": %.2f, "
			 "\"rxmcst\": %.2f}",
			 sndc->interface,
			 S_VALUE(sndp->rx_packets,    sndc->rx_packets,    itv),
			 S_VALUE(sndp->tx_packets,    sndc->tx_packets,    itv),
			 S_VALUE(sndp->rx_bytes,      sndc->rx_bytes,      itv) / 1024,
			 S_VALUE(sndp->tx_bytes,      sndc->tx_bytes,      itv) / 1024,
			 S_VALUE(sndp->rx_compressed, sndc->rx_compressed, itv),
			 S_VALUE(sndp->tx_compressed, sndc->tx_compressed, itv),
			 S_VALUE(sndp->multicast,     sndc->multicast,     itv));
	}

	printf("\n");
	xprintf0(--tab, "]");

	tab--;

close_json_markup:
	if (CLOSE_MARKUP(a->options)) {
		json_markup_network(tab, CLOSE_JSON_MARKUP);
	}
}

/*
 ***************************************************************************
 * Display network interfaces error statistics in JSON.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @curr	Index in array for current sample statistics.
 * @tab		Indentation in output.
 * @itv		Interval of time in jiffies.
 ***************************************************************************
 */
__print_funct_t json_print_net_edev_stats(struct activity *a, int curr, int tab,
					  unsigned long long itv)
{
	int i, j;
	struct stats_net_edev *snedc, *snedp;
	int sep = FALSE;

	if (!IS_SELECTED(a->options) || (a->nr <= 0))
		goto close_json_markup;

	json_markup_network(tab, OPEN_JSON_MARKUP);
	tab++;
	
	xprintf(tab++, "\"net-edev\": [");

	for (i = 0; i < a->nr; i++) {

		snedc = (struct stats_net_edev *) ((char *) a->buf[curr] + i * a->msize);

		if (!strcmp(snedc->interface, ""))
			continue;
		
		j = check_net_edev_reg(a, curr, !curr, i);
		snedp = (struct stats_net_edev *) ((char *) a->buf[!curr] + j * a->msize);
		
		if (sep) {
			printf(",\n");
		}
		sep = TRUE;

		xprintf0(tab, "{\"iface\": \"%s\", "
			 "\"rxerr\": %.2f, "
			 "\"txerr\": %.2f, "
			 "\"coll\": %.2f, "
			 "\"rxdrop\": %.2f, "
			 "\"txdrop\": %.2f, "
			 "\"txcarr\": %.2f, "
			 "\"rxfram\": %.2f, "
			 "\"rxfifo\": %.2f, "
			 "\"txfifo\": %.2f}",
			 snedc->interface,
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

	printf("\n");
	xprintf0(--tab, "]");

	tab--;

close_json_markup:
	if (CLOSE_MARKUP(a->options)) {
		json_markup_network(tab, CLOSE_JSON_MARKUP);
	}
}

/*
 ***************************************************************************
 * Display NFS client statistics in JSON.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @curr	Index in array for current sample statistics.
 * @tab		Indentation in output.
 * @itv		Interval of time in jiffies.
 ***************************************************************************
 */
__print_funct_t json_print_net_nfs_stats(struct activity *a, int curr, int tab,
					 unsigned long long itv)
{
	struct stats_net_nfs
		*snnc = (struct stats_net_nfs *) a->buf[curr],
		*snnp = (struct stats_net_nfs *) a->buf[!curr];
	
	if (!IS_SELECTED(a->options) || (a->nr <= 0))
		goto close_json_markup;

	json_markup_network(tab, OPEN_JSON_MARKUP);
	tab++;

	xprintf0(tab, "\"net-nfs\": {"
		 "\"call\": %.2f, "
		 "\"retrans\": %.2f, "
		 "\"read\": %.2f, "
		 "\"write\": %.2f, "
		 "\"access\": %.2f, "
		 "\"getatt\": %.2f}",
		 S_VALUE(snnp->nfs_rpccnt,     snnc->nfs_rpccnt,     itv),
		 S_VALUE(snnp->nfs_rpcretrans, snnc->nfs_rpcretrans, itv),
		 S_VALUE(snnp->nfs_readcnt,    snnc->nfs_readcnt,    itv),
		 S_VALUE(snnp->nfs_writecnt,   snnc->nfs_writecnt,   itv),
		 S_VALUE(snnp->nfs_accesscnt,  snnc->nfs_accesscnt,  itv),
		 S_VALUE(snnp->nfs_getattcnt,  snnc->nfs_getattcnt,  itv));
	tab--;

close_json_markup:
	if (CLOSE_MARKUP(a->options)) {
		json_markup_network(tab, CLOSE_JSON_MARKUP);
	}
}

/*
 ***************************************************************************
 * Display NFS server statistics in JSON.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @curr	Index in array for current sample statistics.
 * @tab		Indentation in output.
 * @itv		Interval of time in jiffies.
 ***************************************************************************
 */
__print_funct_t json_print_net_nfsd_stats(struct activity *a, int curr, int tab,
					  unsigned long long itv)
{
	struct stats_net_nfsd
		*snndc = (struct stats_net_nfsd *) a->buf[curr],
		*snndp = (struct stats_net_nfsd *) a->buf[!curr];

	if (!IS_SELECTED(a->options) || (a->nr <= 0))
		goto close_json_markup;

	json_markup_network(tab, OPEN_JSON_MARKUP);
	tab++;

	xprintf0(tab, "\"net-nfsd\": {"
		 "\"scall\": %.2f, "
		 "\"badcall\": %.2f, "
		 "\"packet\": %.2f, "
		 "\"udp\": %.2f, "
		 "\"tcp\": %.2f, "
		 "\"hit\": %.2f, "
		 "\"miss\": %.2f, "
		 "\"sread\": %.2f, "
		 "\"swrite\": %.2f, "
		 "\"saccess\": %.2f, "
		 "\"sgetatt\": %.2f}",
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
	tab--;

close_json_markup:
	if (CLOSE_MARKUP(a->options)) {
		json_markup_network(tab, CLOSE_JSON_MARKUP);
	}
}

/*
 ***************************************************************************
 * Display network socket statistics in JSON.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @curr	Index in array for current sample statistics.
 * @tab		Indentation in output.
 * @itv		Interval of time in jiffies.
 ***************************************************************************
 */
__print_funct_t json_print_net_sock_stats(struct activity *a, int curr, int tab,
					  unsigned long long itv)
{
	struct stats_net_sock
		*snsc = (struct stats_net_sock *) a->buf[curr];
	
	if (!IS_SELECTED(a->options) || (a->nr <= 0))
		goto close_json_markup;

	json_markup_network(tab, OPEN_JSON_MARKUP);
	tab++;

	xprintf0(tab, "\"net-sock\": {"
		 "\"totsck\": %u, "
		 "\"tcpsck\": %u, "
		 "\"udpsck\": %u, "
		 "\"rawsck\": %u, "
		 "\"ip-frag\": %u, "
		 "\"tcp-tw\": %u}",
		 snsc->sock_inuse,
		 snsc->tcp_inuse,
		 snsc->udp_inuse,
		 snsc->raw_inuse,
		 snsc->frag_inuse,
		 snsc->tcp_tw);
	tab--;

close_json_markup:
	if (CLOSE_MARKUP(a->options)) {
		json_markup_network(tab, CLOSE_JSON_MARKUP);
	}
}

/*
 ***************************************************************************
 * Display IP network statistics in JSON.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @curr	Index in array for current sample statistics.
 * @tab		Indentation in output.
 * @itv		Interval of time in jiffies.
 ***************************************************************************
 */
__print_funct_t json_print_net_ip_stats(struct activity *a, int curr, int tab,
					unsigned long long itv)
{
	struct stats_net_ip
		*snic = (struct stats_net_ip *) a->buf[curr],
		*snip = (struct stats_net_ip *) a->buf[!curr];
	
	if (!IS_SELECTED(a->options) || (a->nr <= 0))
		goto close_json_markup;

	json_markup_network(tab, OPEN_JSON_MARKUP);
	tab++;

	xprintf0(tab, "\"net-ip\": {"
		 "\"irec\": %.2f, "
		 "\"fwddgm\": %.2f, "
		 "\"idel\": %.2f, "
		 "\"orq\": %.2f, "
		 "\"asmrq\": %.2f, "
		 "\"asmok\": %.2f, "
		 "\"fragok\": %.2f, "
		 "\"fragcrt\": %.2f}",
		 S_VALUE(snip->InReceives,    snic->InReceives,    itv),
		 S_VALUE(snip->ForwDatagrams, snic->ForwDatagrams, itv),
		 S_VALUE(snip->InDelivers,    snic->InDelivers,    itv),
		 S_VALUE(snip->OutRequests,   snic->OutRequests,   itv),
		 S_VALUE(snip->ReasmReqds,    snic->ReasmReqds,    itv),
		 S_VALUE(snip->ReasmOKs,      snic->ReasmOKs,      itv),
		 S_VALUE(snip->FragOKs,       snic->FragOKs,       itv),
		 S_VALUE(snip->FragCreates,   snic->FragCreates,   itv));
	tab--;

close_json_markup:
	if (CLOSE_MARKUP(a->options)) {
		json_markup_network(tab, CLOSE_JSON_MARKUP);
	}
}

/*
 ***************************************************************************
 * Display IP network error statistics in JSON.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @curr	Index in array for current sample statistics.
 * @tab		Indentation in output.
 * @itv		Interval of time in jiffies.
 ***************************************************************************
 */
__print_funct_t json_print_net_eip_stats(struct activity *a, int curr, int tab,
					 unsigned long long itv)
{
	struct stats_net_eip
		*sneic = (struct stats_net_eip *) a->buf[curr],
		*sneip = (struct stats_net_eip *) a->buf[!curr];
	
	if (!IS_SELECTED(a->options) || (a->nr <= 0))
		goto close_json_markup;

	json_markup_network(tab, OPEN_JSON_MARKUP);
	tab++;

	xprintf0(tab, "\"net-eip\": {"
		 "\"ihdrerr\": %.2f, "
		 "\"iadrerr\": %.2f, "
		 "\"iukwnpr\": %.2f, "
		 "\"idisc\": %.2f, "
		 "\"odisc\": %.2f, "
		 "\"onort\": %.2f, "
		 "\"asmf\": %.2f, "
		 "\"fragf\": %.2f}",
		 S_VALUE(sneip->InHdrErrors,     sneic->InHdrErrors,     itv),
		 S_VALUE(sneip->InAddrErrors,    sneic->InAddrErrors,    itv),
		 S_VALUE(sneip->InUnknownProtos, sneic->InUnknownProtos, itv),
		 S_VALUE(sneip->InDiscards,      sneic->InDiscards,      itv),
		 S_VALUE(sneip->OutDiscards,     sneic->OutDiscards,     itv),
		 S_VALUE(sneip->OutNoRoutes,     sneic->OutNoRoutes,     itv),
		 S_VALUE(sneip->ReasmFails,      sneic->ReasmFails,      itv),
		 S_VALUE(sneip->FragFails,       sneic->FragFails,       itv));
	tab--;

close_json_markup:
	if (CLOSE_MARKUP(a->options)) {
		json_markup_network(tab, CLOSE_JSON_MARKUP);
	}
}

/*
 ***************************************************************************
 * Display ICMP network statistics in JSON.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @curr	Index in array for current sample statistics.
 * @tab		Indentation in output.
 * @itv		Interval of time in jiffies.
 ***************************************************************************
 */
__print_funct_t json_print_net_icmp_stats(struct activity *a, int curr, int tab,
					  unsigned long long itv)
{
	struct stats_net_icmp
		*snic = (struct stats_net_icmp *) a->buf[curr],
		*snip = (struct stats_net_icmp *) a->buf[!curr];
	
	if (!IS_SELECTED(a->options) || (a->nr <= 0))
		goto close_json_markup;

	json_markup_network(tab, OPEN_JSON_MARKUP);
	tab++;

	xprintf0(tab, "\"net-icmp\": {"
		 "\"imsg\": %.2f, "
		 "\"omsg\": %.2f, "
		 "\"iech\": %.2f, "
		 "\"iechr\": %.2f, "
		 "\"oech\": %.2f, "
		 "\"oechr\": %.2f, "
		 "\"itm\": %.2f, "
		 "\"itmr\": %.2f, "
		 "\"otm\": %.2f, "
		 "\"otmr\": %.2f, "
		 "\"iadrmk\": %.2f, "
		 "\"iadrmkr\": %.2f, "
		 "\"oadrmk\": %.2f, "
		 "\"oadrmkr\": %.2f}",
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
	tab--;

close_json_markup:
	if (CLOSE_MARKUP(a->options)) {
		json_markup_network(tab, CLOSE_JSON_MARKUP);
	}
}

/*
 ***************************************************************************
 * Display ICMP error message statistics in JSON.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @curr	Index in array for current sample statistics.
 * @tab		Indentation in output.
 * @itv		Interval of time in jiffies.
 ***************************************************************************
 */
__print_funct_t json_print_net_eicmp_stats(struct activity *a, int curr, int tab,
					   unsigned long long itv)
{
	struct stats_net_eicmp
		*sneic = (struct stats_net_eicmp *) a->buf[curr],
		*sneip = (struct stats_net_eicmp *) a->buf[!curr];
	
	if (!IS_SELECTED(a->options) || (a->nr <= 0))
		goto close_json_markup;

	json_markup_network(tab, OPEN_JSON_MARKUP);
	tab++;

	xprintf0(tab, "\"net-eicmp\": {"
		 "\"ierr\": %.2f, "
		 "\"oerr\": %.2f, "
		 "\"idstunr\": %.2f, "
		 "\"odstunr\": %.2f, "
		 "\"itmex\": %.2f, "
		 "\"otmex\": %.2f, "
		 "\"iparmpb\": %.2f, "
		 "\"oparmpb\": %.2f, "
		 "\"isrcq\": %.2f, "
		 "\"osrcq\": %.2f, "
		 "\"iredir\": %.2f, "
		 "\"oredir\": %.2f}",
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
	tab--;

close_json_markup:
	if (CLOSE_MARKUP(a->options)) {
		json_markup_network(tab, CLOSE_JSON_MARKUP);
	}
}

/*
 ***************************************************************************
 * Display TCP network statistics in JSON.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @curr	Index in array for current sample statistics.
 * @tab		Indentation in output.
 * @itv		Interval of time in jiffies.
 ***************************************************************************
 */
__print_funct_t json_print_net_tcp_stats(struct activity *a, int curr, int tab,
					 unsigned long long itv)
{
	struct stats_net_tcp
		*sntc = (struct stats_net_tcp *) a->buf[curr],
		*sntp = (struct stats_net_tcp *) a->buf[!curr];
	
	if (!IS_SELECTED(a->options) || (a->nr <= 0))
		goto close_json_markup;

	json_markup_network(tab, OPEN_JSON_MARKUP);
	tab++;

	xprintf0(tab, "\"net-tcp\": {"
		 "\"active\": %.2f, "
		 "\"passive\": %.2f, "
		 "\"iseg\": %.2f, "
		 "\"oseg\": %.2f}",
		 S_VALUE(sntp->ActiveOpens,  sntc->ActiveOpens,  itv),
		 S_VALUE(sntp->PassiveOpens, sntc->PassiveOpens, itv),
		 S_VALUE(sntp->InSegs,       sntc->InSegs,       itv),
		 S_VALUE(sntp->OutSegs,      sntc->OutSegs,      itv));
	tab--;

close_json_markup:
	if (CLOSE_MARKUP(a->options)) {
		json_markup_network(tab, CLOSE_JSON_MARKUP);
	}
}

/*
 ***************************************************************************
 * Display TCP network error statistics in JSON.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @curr	Index in array for current sample statistics.
 * @tab		Indentation in XML output.
 * @itv		Interval of time in jiffies.
 ***************************************************************************
 */
__print_funct_t json_print_net_etcp_stats(struct activity *a, int curr, int tab,
					  unsigned long long itv)
{
	struct stats_net_etcp
		*snetc = (struct stats_net_etcp *) a->buf[curr],
		*snetp = (struct stats_net_etcp *) a->buf[!curr];
	
	if (!IS_SELECTED(a->options) || (a->nr <= 0))
		goto close_json_markup;

	json_markup_network(tab, OPEN_JSON_MARKUP);
	tab++;

	xprintf0(tab, "\"net-etcp\": {"
		 "\"atmptf\": %.2f, "
		 "\"estres\": %.2f, "
		 "\"retrans\": %.2f, "
		 "\"isegerr\": %.2f, "
		 "\"orsts\": %.2f}",
		 S_VALUE(snetp->AttemptFails, snetc->AttemptFails,  itv),
		 S_VALUE(snetp->EstabResets,  snetc->EstabResets,  itv),
		 S_VALUE(snetp->RetransSegs,  snetc->RetransSegs,  itv),
		 S_VALUE(snetp->InErrs,       snetc->InErrs,  itv),
		 S_VALUE(snetp->OutRsts,      snetc->OutRsts,  itv));
	tab--;

close_json_markup:
	if (CLOSE_MARKUP(a->options)) {
		json_markup_network(tab, CLOSE_JSON_MARKUP);
	}
}

/*
 ***************************************************************************
 * Display UDP network statistics in JSON.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @curr	Index in array for current sample statistics.
 * @tab		Indentation in output.
 * @itv		Interval of time in jiffies.
 ***************************************************************************
 */
__print_funct_t json_print_net_udp_stats(struct activity *a, int curr, int tab,
					 unsigned long long itv)
{
	struct stats_net_udp
		*snuc = (struct stats_net_udp *) a->buf[curr],
		*snup = (struct stats_net_udp *) a->buf[!curr];
	
	if (!IS_SELECTED(a->options) || (a->nr <= 0))
		goto close_json_markup;

	json_markup_network(tab, OPEN_JSON_MARKUP);
	tab++;

	xprintf0(tab, "\"net-udp\": {"
		 "\"idgm\": %.2f, "
		 "\"odgm\": %.2f, "
		 "\"noport\": %.2f, "
		 "\"idgmerr\": %.2f}",
		 S_VALUE(snup->InDatagrams,  snuc->InDatagrams,  itv),
		 S_VALUE(snup->OutDatagrams, snuc->OutDatagrams, itv),
		 S_VALUE(snup->NoPorts,      snuc->NoPorts,      itv),
		 S_VALUE(snup->InErrors,     snuc->InErrors,     itv));
	tab--;

close_json_markup:
	if (CLOSE_MARKUP(a->options)) {
		json_markup_network(tab, CLOSE_JSON_MARKUP);
	}
}

/*
 ***************************************************************************
 * Display IPv6 network socket statistics in JSON.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @curr	Index in array for current sample statistics.
 * @tab		Indentation in output.
 * @itv		Interval of time in jiffies.
 ***************************************************************************
 */
__print_funct_t json_print_net_sock6_stats(struct activity *a, int curr, int tab,
					   unsigned long long itv)
{
	struct stats_net_sock6
		*snsc = (struct stats_net_sock6 *) a->buf[curr];
	
	if (!IS_SELECTED(a->options) || (a->nr <= 0))
		goto close_json_markup;

	json_markup_network(tab, OPEN_JSON_MARKUP);
	tab++;

	xprintf0(tab, "\"net-sock6\": {"
		 "\"tcp6sck\": %u, "
		 "\"udp6sck\": %u, "
		 "\"raw6sck\": %u, "
		 "\"ip6-frag\": %u}",
		 snsc->tcp6_inuse,
		 snsc->udp6_inuse,
		 snsc->raw6_inuse,
		 snsc->frag6_inuse);
	tab--;

close_json_markup:
	if (CLOSE_MARKUP(a->options)) {
		json_markup_network(tab, CLOSE_JSON_MARKUP);
	}
}

/*
 ***************************************************************************
 * Display IPv6 network statistics in JSON.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @curr	Index in array for current sample statistics.
 * @tab		Indentation in output.
 * @itv		Interval of time in jiffies.
 ***************************************************************************
 */
__print_funct_t json_print_net_ip6_stats(struct activity *a, int curr, int tab,
					 unsigned long long itv)
{
	struct stats_net_ip6
		*snic = (struct stats_net_ip6 *) a->buf[curr],
		*snip = (struct stats_net_ip6 *) a->buf[!curr];
	
	if (!IS_SELECTED(a->options) || (a->nr <= 0))
		goto close_json_markup;

	json_markup_network(tab, OPEN_JSON_MARKUP);
	tab++;

	xprintf0(tab, "\"net-ip6\": {"
		 "\"irec6\": %.2f, "
		 "\"fwddgm6\": %.2f, "
		 "\"idel6\": %.2f, "
		 "\"orq6\": %.2f, "
		 "\"asmrq6\": %.2f, "
		 "\"asmok6\": %.2f, "
		 "\"imcpck6\": %.2f, "
		 "\"omcpck6\": %.2f, "
		 "\"fragok6\": %.2f, "
		 "\"fragcr6\": %.2f}",
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
	tab--;

close_json_markup:
	if (CLOSE_MARKUP(a->options)) {
		json_markup_network(tab, CLOSE_JSON_MARKUP);
	}
}

/*
 ***************************************************************************
 * Display IPv6 network error statistics in JSON.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @curr	Index in array for current sample statistics.
 * @tab		Indentation in output.
 * @itv		Interval of time in jiffies.
 ***************************************************************************
 */
__print_funct_t json_print_net_eip6_stats(struct activity *a, int curr, int tab,
					  unsigned long long itv)
{
	struct stats_net_eip6
		*sneic = (struct stats_net_eip6 *) a->buf[curr],
		*sneip = (struct stats_net_eip6 *) a->buf[!curr];
	
	if (!IS_SELECTED(a->options) || (a->nr <= 0))
		goto close_json_markup;

	json_markup_network(tab, OPEN_JSON_MARKUP);
	tab++;

	xprintf0(tab, "\"net-eip6\": {"
		 "\"ihdrer6\": %.2f, "
		 "\"iadrer6\": %.2f, "
		 "\"iukwnp6\": %.2f, "
		 "\"i2big6\": %.2f, "
		 "\"idisc6\": %.2f, "
		 "\"odisc6\": %.2f, "
		 "\"inort6\": %.2f, "
		 "\"onort6\": %.2f, "
		 "\"asmf6\": %.2f, "
		 "\"fragf6\": %.2f, "
		 "\"itrpck6\": %.2f}",
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
	tab--;

close_json_markup:
	if (CLOSE_MARKUP(a->options)) {
		json_markup_network(tab, CLOSE_JSON_MARKUP);
	}
}

/*
 ***************************************************************************
 * Display ICMPv6 network statistics in JSON.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @curr	Index in array for current sample statistics.
 * @tab		Indentation in output.
 * @itv		Interval of time in jiffies.
 ***************************************************************************
 */
__print_funct_t json_print_net_icmp6_stats(struct activity *a, int curr, int tab,
					   unsigned long long itv)
{
	struct stats_net_icmp6
		*snic = (struct stats_net_icmp6 *) a->buf[curr],
		*snip = (struct stats_net_icmp6 *) a->buf[!curr];
	
	if (!IS_SELECTED(a->options) || (a->nr <= 0))
		goto close_json_markup;

	json_markup_network(tab, OPEN_JSON_MARKUP);
	tab++;

	xprintf0(tab, "\"net-icmp6\": {"
		 "\"imsg6\": %.2f, "
		 "\"omsg6\": %.2f, "
		 "\"iech6\": %.2f, "
		 "\"iechr6\": %.2f, "
		 "\"oechr6\": %.2f, "
		 "\"igmbq6\": %.2f, "
		 "\"igmbr6\": %.2f, "
		 "\"ogmbr6\": %.2f, "
		 "\"igmbrd6\": %.2f, "
		 "\"ogmbrd6\": %.2f, "
		 "\"irtsol6\": %.2f, "
		 "\"ortsol6\": %.2f, "
		 "\"irtad6\": %.2f, "
		 "\"inbsol6\": %.2f, "
		 "\"onbsol6\": %.2f, "
		 "\"inbad6\": %.2f, "
		 "\"onbad6\": %.2f}",
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
	tab--;

close_json_markup:
	if (CLOSE_MARKUP(a->options)) {
		json_markup_network(tab, CLOSE_JSON_MARKUP);
	}
}

/*
 ***************************************************************************
 * Display ICMPv6 error message statistics in JSON.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @curr	Index in array for current sample statistics.
 * @tab		Indentation in output.
 * @itv		Interval of time in jiffies.
 ***************************************************************************
 */
__print_funct_t json_print_net_eicmp6_stats(struct activity *a, int curr, int tab,
					    unsigned long long itv)
{
	struct stats_net_eicmp6
		*sneic = (struct stats_net_eicmp6 *) a->buf[curr],
		*sneip = (struct stats_net_eicmp6 *) a->buf[!curr];
	
	if (!IS_SELECTED(a->options) || (a->nr <= 0))
		goto close_json_markup;

	json_markup_network(tab, OPEN_JSON_MARKUP);
	tab++;

	xprintf0(tab, "\"net-eicmp6\": {"
		 "\"ierr6\": %.2f, "
		 "\"idtunr6\": %.2f, "
		 "\"odtunr6\": %.2f, "
		 "\"itmex6\": %.2f, "
		 "\"otmex6\": %.2f, "
		 "\"iprmpb6\": %.2f, "
		 "\"oprmpb6\": %.2f, "
		 "\"iredir6\": %.2f, "
		 "\"oredir6\": %.2f, "
		 "\"ipck2b6\": %.2f, "
		 "\"opck2b6\": %.2f}",
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
	tab--;

close_json_markup:
	if (CLOSE_MARKUP(a->options)) {
		json_markup_network(tab, CLOSE_JSON_MARKUP);
	}
}

/*
 ***************************************************************************
 * Display UDPv6 network statistics in JSON.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @curr	Index in array for current sample statistics.
 * @tab		Indentation in output.
 * @itv		Interval of time in jiffies.
 ***************************************************************************
 */
__print_funct_t json_print_net_udp6_stats(struct activity *a, int curr, int tab,
					  unsigned long long itv)
{
	struct stats_net_udp6
		*snuc = (struct stats_net_udp6 *) a->buf[curr],
		*snup = (struct stats_net_udp6 *) a->buf[!curr];
	
	if (!IS_SELECTED(a->options) || (a->nr <= 0))
		goto close_json_markup;

	json_markup_network(tab, OPEN_JSON_MARKUP);
	tab++;

	xprintf0(tab, "\"net-udp6\": {"
		 "\"idgm6\": %.2f, "
		 "\"odgm6\": %.2f, "
		 "\"noport6\": %.2f, "
		 "\"idgmer6\": %.2f}",
		 S_VALUE(snup->InDatagrams6,  snuc->InDatagrams6,  itv),
		 S_VALUE(snup->OutDatagrams6, snuc->OutDatagrams6, itv),
		 S_VALUE(snup->NoPorts6,      snuc->NoPorts6,      itv),
		 S_VALUE(snup->InErrors6,     snuc->InErrors6,     itv));
	tab--;

close_json_markup:
	if (CLOSE_MARKUP(a->options)) {
		json_markup_network(tab, CLOSE_JSON_MARKUP);
	}
}

/*
 ***************************************************************************
 * Display CPU frequency statistics in JSON.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @curr	Index in array for current sample statistics.
 * @tab		Indentation in output.
 * @itv		Interval of time in jiffies.
 ***************************************************************************
 */
__print_funct_t json_print_pwr_cpufreq_stats(struct activity *a, int curr, int tab,
					     unsigned long long itv)
{
	int i;
	struct stats_pwr_cpufreq *spc;
	int sep = FALSE;
	char cpuno[8];

	if (!IS_SELECTED(a->options) || (a->nr <= 0))
		goto close_json_markup;

	json_markup_power_management(tab, OPEN_JSON_MARKUP);
	tab++;

	xprintf(tab++, "\"cpu-frequency\": [");
	
	for (i = 0; (i < a->nr) && (i < a->bitmap->b_size + 1); i++) {
		
		spc = (struct stats_pwr_cpufreq *) ((char *) a->buf[curr]  + i * a->msize);
	
		/* Should current CPU (including CPU "all") be displayed? */
		if (a->bitmap->b_array[i >> 3] & (1 << (i & 0x07))) {

			/* Yes: Display it */
			if (!i) {
				/* This is CPU "all" */
				strcpy(cpuno, "all");
			}
			else {
				sprintf(cpuno, "%d", i - 1);
			}

			if (sep) {
				printf(",\n");
			}
			sep = TRUE;
			
			xprintf0(tab, "{\"number\": \"%s\", "
				 "\"frequency\": %.2f}",
				 cpuno,
				 ((double) spc->cpufreq) / 100);
		}
	}
	
	printf("\n");
	xprintf0(--tab, "]");
	tab--;

close_json_markup:
	if (CLOSE_MARKUP(a->options)) {
		json_markup_power_management(tab, CLOSE_JSON_MARKUP);
	}
}

/*
 ***************************************************************************
 * Display fan statistics in JSON.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @curr	Index in array for current sample statistics.
 * @tab		Indentation in output.
 * @itv		Interval of time in jiffies.
 ***************************************************************************
 */
__print_funct_t json_print_pwr_fan_stats(struct activity *a, int curr, int tab,
					 unsigned long long itv)
{
	int i;
	struct stats_pwr_fan *spc;
	int sep = FALSE;

	if (!IS_SELECTED(a->options) || (a->nr <= 0))
		goto close_json_markup;

	json_markup_power_management(tab, OPEN_JSON_MARKUP);
	tab++;

	xprintf(tab++, "\"fan-speed\": [");

	for (i = 0; i < a->nr; i++) {
		spc = (struct stats_pwr_fan *) ((char *) a->buf[curr]  + i * a->msize);

		if (sep) {
			printf(",\n");
		}
		sep = TRUE;
		
		xprintf0(tab, "{\"number\": %d, "
			 "\"rpm\": %llu, "
			 "\"drpm\": %llu, "
			 "\"device\": \"%s\"}",
			 i + 1,
			 (unsigned long long) spc->rpm,
			 (unsigned long long) (spc->rpm - spc->rpm_min),
			 spc->device);
	}

	printf("\n");
	xprintf0(--tab, "]");
	tab--;

close_json_markup:
	if (CLOSE_MARKUP(a->options)) {
		json_markup_power_management(tab, CLOSE_JSON_MARKUP);
	}
}

/*
 ***************************************************************************
 * Display temperature statistics in JSON.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @curr	Index in array for current sample statistics.
 * @tab		Indentation in output.
 * @itv		Interval of time in jiffies.
 ***************************************************************************
 */
__print_funct_t json_print_pwr_temp_stats(struct activity *a, int curr, int tab,
					  unsigned long long itv)
{
	int i;
	struct stats_pwr_temp *spc;
	int sep = FALSE;

	if (!IS_SELECTED(a->options) || (a->nr <= 0))
		goto close_json_markup;

	json_markup_power_management(tab, OPEN_JSON_MARKUP);
	tab++;

	xprintf(tab++, "\"temperature\": [");

	for (i = 0; i < a->nr; i++) {
		spc = (struct stats_pwr_temp *) ((char *) a->buf[curr]  + i * a->msize);

		if (sep) {
			printf(",\n");
		}
		sep = TRUE;
		
		xprintf0(tab, "{\"number\": %d, "
			 "\"degC\": %.2f, "
			 "\"percent-temp\": %.2f, "
			 "\"device\": \"%s\"}",
			 i + 1,
			 spc->temp,
			 (spc->temp_max - spc->temp_min) ?
			 (spc->temp - spc->temp_min) / (spc->temp_max - spc->temp_min) * 100 :
			 0.0,
			 spc->device);
	}

	printf("\n");
	xprintf0(--tab, "]");
	tab--;

close_json_markup:
	if (CLOSE_MARKUP(a->options)) {
		json_markup_power_management(tab, CLOSE_JSON_MARKUP);
	}
}

/*
 ***************************************************************************
 * Display voltage inputs statistics in JSON.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @curr	Index in array for current sample statistics.
 * @tab		Indentation in output.
 * @itv		Interval of time in jiffies.
 ***************************************************************************
 */
__print_funct_t json_print_pwr_in_stats(struct activity *a, int curr, int tab,
					unsigned long long itv)
{
	int i;
	struct stats_pwr_in *spc;
	int sep = FALSE;

	if (!IS_SELECTED(a->options) || (a->nr <= 0))
		goto close_json_markup;

	json_markup_power_management(tab, OPEN_JSON_MARKUP);
	tab++;

	xprintf(tab++, "\"voltage-input\": [");

	for (i = 0; i < a->nr; i++) {
		spc = (struct stats_pwr_in *) ((char *) a->buf[curr]  + i * a->msize);

		if (sep) {
			printf(",\n");
		}
		sep = TRUE;
		
		xprintf0(tab, "{\"number\": %d, "
			 "\"inV\": %.2f, "
			 "\"percent-in\": %.2f, "
			 "\"device\": \"%s\"}",
			 i,
			 spc->in,
			 (spc->in_max - spc->in_min) ?
			 (spc->in - spc->in_min) / (spc->in_max - spc->in_min) * 100 :
			 0.0,
			 spc->device);
	}

	printf("\n");
	xprintf0(--tab, "]");
	tab--;

close_json_markup:
	if (CLOSE_MARKUP(a->options)) {
		json_markup_power_management(tab, CLOSE_JSON_MARKUP);
	}
}

/*
 ***************************************************************************
 * Display huge pages statistics in JSON.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @curr	Index in array for current sample statistics.
 * @tab		Indentation in output.
 * @itv		Interval of time in jiffies.
 ***************************************************************************
 */
__print_funct_t json_print_huge_stats(struct activity *a, int curr, int tab,
				      unsigned long long itv)
{
	struct stats_huge
		*smc = (struct stats_huge *) a->buf[curr];

	xprintf0(tab, "\"hugepages\": {"
		 "\"hugfree\": %lu, "
		 "\"hugused\": %lu, "
		 "\"hugused-percent\": %.2f}",
		 smc->frhkb,
		 smc->tlhkb - smc->frhkb,
		 smc->tlhkb ?
		 SP_VALUE(smc->frhkb, smc->tlhkb, smc->tlhkb) :
		 0.0);
}

/*
 ***************************************************************************
 * Display weighted CPU frequency statistics in JSON.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @curr	Index in array for current sample statistics.
 * @tab		Indentation in output.
 * @itv		Interval of time in jiffies.
 ***************************************************************************
 */
__print_funct_t json_print_pwr_wghfreq_stats(struct activity *a, int curr, int tab,
					     unsigned long long itv)
{
	int i, k;
	struct stats_pwr_wghfreq *spc, *spp, *spc_k, *spp_k;
	unsigned long long tis, tisfreq;
	int sep = FALSE;
	char cpuno[8];

	if (!IS_SELECTED(a->options) || (a->nr <= 0))
		goto close_json_markup;

	json_markup_power_management(tab, OPEN_JSON_MARKUP);
	tab++;

	xprintf(tab++, "\"cpu-weighted-frequency\": [");

	for (i = 0; (i < a->nr) && (i < a->bitmap->b_size + 1); i++) {

		spc = (struct stats_pwr_wghfreq *) ((char *) a->buf[curr]  + i * a->msize * a->nr2);
		spp = (struct stats_pwr_wghfreq *) ((char *) a->buf[!curr] + i * a->msize * a->nr2);

		/* Should current CPU (including CPU "all") be displayed? */
		if (a->bitmap->b_array[i >> 3] & (1 << (i & 0x07))) {

			/* Yes... */
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

			if (!i) {
				/* This is CPU "all" */
				strcpy(cpuno, "all");
			}
			else {
				sprintf(cpuno, "%d", i - 1);
			}

			if (sep) {
				printf(",\n");
			}
			sep = TRUE;

			xprintf0(tab, "{\"number\": \"%s\", "
				 "\"weighted-frequency\": %.2f}",
				 cpuno,
				 tis ? ((double) tisfreq) / tis : 0.0);
		}
	}

	printf("\n");
	xprintf0(--tab, "]");
	tab--;

close_json_markup:
	if (CLOSE_MARKUP(a->options)) {
		json_markup_power_management(tab, CLOSE_JSON_MARKUP);
	}
}

/*
 ***************************************************************************
 * Display USB devices statistics in JSON.
 *
 * IN:
 * @a		Activity structure with statistics.
 * @curr	Index in array for current sample statistics.
 * @tab		Indentation in output.
 * @itv		Interval of time in jiffies.
 ***************************************************************************
 */
__print_funct_t json_print_pwr_usb_stats(struct activity *a, int curr, int tab,
					 unsigned long long itv)
{
	int i;
	struct stats_pwr_usb *suc;
	int sep = FALSE;

	if (!IS_SELECTED(a->options) || (a->nr <= 0))
		goto close_json_markup;

	json_markup_power_management(tab, OPEN_JSON_MARKUP);
	tab++;

	xprintf(tab++, "\"usb-devices\": [");

	for (i = 0; i < a->nr; i++) {
		suc = (struct stats_pwr_usb *) ((char *) a->buf[curr]  + i * a->msize);

		if (!suc->bus_nr)
			/* Bus#0 doesn't exist: We are at the end of the list */
			break;

		if (sep) {
			printf(",\n");
		}
		sep = TRUE;
		
		xprintf0(tab, "{\"bus_number\": %d, "
			 "\"idvendor\": \"%x\", "
			 "\"idprod\": \"%x\", "
			 "\"maxpower\": %u, "
			 "\"manufact\": \"%s\", "
			 "\"product\": \"%s\"}",
			 suc->bus_nr,
			 suc->vendor_id,
			 suc->product_id,
			 suc->bmaxpower << 1,
			 suc->manufacturer,
			 suc->product);
	}

	printf("\n");
	xprintf0(--tab, "]");
	tab--;

close_json_markup:
	if (CLOSE_MARKUP(a->options)) {
		json_markup_power_management(tab, CLOSE_JSON_MARKUP);
	}
}
