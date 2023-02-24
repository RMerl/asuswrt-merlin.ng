
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include <stdint.h>
#include <limits.h>
#include <dirent.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <timer_utils.h>
#include <shared.h>
#include "pc.h"

#define NORMAL_PERIOD           5*TIMER_HZ      /* minisecond */

static int g_prev_block_all = 0;
static int g_prev_multifilter_all = 0;

static void pctime_loop(struct timer_entry *timer, void *data);
static void pctime_config(struct timer_entry *timer, void *data);	
static void pctime_flush(struct timer_entry *timer, void *data);	
static void pctime_free(struct timer_entry *timer, void *data);	
static void pctime_reap(struct timer_entry *timer, void *data);	

static struct task_table pctime_task_t[] =
{       /* sig, *timer, *func, *data, expires */
        {SIGALRM, 0, pctime_loop, 0, NORMAL_PERIOD},
        {SIGUSR1, 0, pctime_config, 0, 0},
        {SIGUSR2, 0, pctime_flush, 0, 0},
        {SIGTERM, 0, pctime_free, 0, 0},
        {SIGCHLD, 0, pctime_reap, 0, 0},
        {0, 0, 0, 0, 0}
};
static int pctime_xnum = sizeof(pctime_task_t)/sizeof(struct task_table);
static int next_expires = NORMAL_PERIOD;

static pc_s *mfpc_list = NULL, *tmp_list = NULL;
static int mfpc_count = -1;
static int pcdbg=0;
#ifdef RTCONFIG_ISP_OPTUS
static pc_s *oppc_list = NULL, *optmp_list = NULL;
static int oppc_count = -1;
#endif /* RTCONFIG_ISP_OPTUS */

static void 
pctime_free(struct timer_entry *timer, void *data)
{
	if (pcdbg)
		_dprintf("enter pctime_free\n");
	if(mfpc_list)
		free_pc_list(&mfpc_list);
	printf("bye\n");
	exit(1);
}

static void 
pctime_reap(struct timer_entry *timer, void *data)
{
	chld_reap(SIGCHLD);
}

static void 
pctime_config(struct timer_entry *timer, void *data)
{
	printf("config pc-list\n");
	if (pcdbg)
		_dprintf("enter pctime_config\n");

#ifdef RTCONFIG_ISP_OPTUS
	if (nvram_get_int("OPTUS_MULTIFILTER_ALL") != 0 && oppc_list) {
		op_get_all_pc_list(&optmp_list);
		if (is_same_pc_list(oppc_list, optmp_list)) {
			printf("op-pc-list is not changed.\n");
			free_pc_list(&optmp_list);
			optmp_list = NULL;
			goto SKIP_OP;
		}
		else {
			printf("op-pc-list is changed.\n");
			free_pc_list(&oppc_list);
			oppc_list = NULL;
			/*pc_s *follow_pc;
			pc_s **target_pc = &oppc_list;
			for(follow_pc = optmp_list; follow_pc != NULL; follow_pc = follow_pc->next){
				cp_pc(target_pc, follow_pc);

				while(*target_pc != NULL)
					target_pc = &((*target_pc)->next);
			}*/
			free_pc_list(&optmp_list);
			optmp_list = NULL;
		}
	}
	op_get_all_pc_list(&oppc_list);
	oppc_count = count_pc_rules(oppc_list, 2);

SKIP_OP:
#endif /*RTCONFIG_ISP_OPTUS */

	if(mfpc_list) {
		get_all_pc_list(&tmp_list);
		if (is_same_pc_list(mfpc_list, tmp_list)) {
			printf("pc-list is not changed.\n");
			free_pc_list(&tmp_list);
			tmp_list = NULL;
			return;
		}
		else {
			printf("pc-list is changed.\n");
			free_pc_list(&mfpc_list);
			mfpc_list = NULL;
			pc_s *follow_pc;
			pc_s **target_pc = &mfpc_list;
			for(follow_pc = tmp_list; follow_pc != NULL; follow_pc = follow_pc->next){
				cp_pc(target_pc, follow_pc);

				while(*target_pc != NULL)
					target_pc = &((*target_pc)->next);
			}
			free_pc_list(&tmp_list);
			tmp_list = NULL;
		}
	}
	get_all_pc_list(&mfpc_list);
	mfpc_count = count_pc_rules(mfpc_list, -1); // count those enabled=1 or 2.
}

static void
set_pc_list_state(pc_s *pc_list, int state, int verb)
{
	pc_s *follow_pc;
	if(pc_list == NULL)
		return;
	for(follow_pc = pc_list; follow_pc != NULL; follow_pc = follow_pc->next){
		follow_pc->state = state;
	}
}

static void 
pctime_flush(struct timer_entry *timer, void *data)
{
	if (pcdbg)
		_dprintf("enter pctime_flush\n");
#ifdef RTCONFIG_ISP_OPTUS
	/* Optus Pause : Optus customization */
	if (nvram_get_int("OPTUS_MULTIFILTER_ALL") != 0) {
		op_cleantrack_pc_list(oppc_list, pcdbg);
	}
#endif /* RTCONFIG_ISP_OPTUS */
	int curr_block_all = nvram_get_int("MULTIFILTER_BLOCK_ALL");
	int curr_multifilter_all = nvram_get_int("MULTIFILTER_ALL");
	// Block all devices enabled clean all conntracks
	if (g_prev_block_all == 0 && curr_block_all == 1) {
		eval("conntrack", "-F");
#ifdef HND_ROUTER
		eval("fc", "flush");
#elif defined(RTCONFIG_BCMARM)
		/* TBD. ctf ipct entries cleanup. */
#endif
		g_prev_block_all = curr_block_all;
		fprintf(stderr, "%s\n", "flush conntrack");
        return;
	}
	g_prev_block_all = curr_block_all;

	if (g_prev_multifilter_all == 1 && curr_multifilter_all == 0) {
		if (pcdbg)
			_dprintf("\n[pc] reset all pc list to NONBLOCK.");
		set_pc_list_state(mfpc_list, NONBLOCK, pcdbg);
	}
	g_prev_multifilter_all = curr_multifilter_all;

	if(curr_multifilter_all==0 || mfpc_count==0)
		return;
    if ((nvram_get_int("ntp_ready") != 1) && (nvram_get_int("qtn_ntp_ready") != 1))
        return;

    time_t t = time(NULL);
    struct tm *pnow = localtime(&t);

#ifdef RTCONFIG_PC_SCHED_V3
    cleantrack_daytime_pc_list(mfpc_list, pnow->tm_wday, pnow->tm_hour, pnow->tm_min, pcdbg);
#else
    cleantrack_daytime_pc_list(mfpc_list, pnow->tm_wday, pnow->tm_hour, pcdbg);
#endif
}

static void
pctime_loop(struct timer_entry *timer, void *data)
{
	if (pcdbg)
		_dprintf("enter pctime_loop\n");
#if 0
#ifdef RTCONFIG_ISP_OPTUS
	/* Optus Pause : Optus customization */
	if (nvram_get_int("OPTUS_MULTIFILTER_ALL") != 0) {
		op_cleantrack_pc_list(oppc_list, pcdbg);
	}
#endif /* RTCONFIG_ISP_OPTUS */
	int curr_block_all = nvram_get_int("MULTIFILTER_BLOCK_ALL");
	// Block all devices enabled clean all conntracks
	if (g_prev_block_all == 0 && curr_block_all == 1) {
		eval("conntrack", "-F");
#ifdef HND_ROUTER
		eval("fc", "flush");
#elif defined(RTCONFIG_BCMARM)
		/* TBD. ctf ipct entries cleanup. */
#endif
		g_prev_block_all = curr_block_all;
		fprintf(stderr, "%s\n", "flush conntrack");
        goto pctimer;
	}
	g_prev_block_all = curr_block_all;

    if(nvram_get_int("MULTIFILTER_ALL")==0 || mfpc_count==0)
        goto pctimer;
    if ((nvram_get_int("ntp_ready") != 1) && (nvram_get_int("qtn_ntp_ready") != 1))
        goto pctimer;

    time_t t = time(NULL);
    struct tm *pnow = localtime(&t);

#ifdef RTCONFIG_PC_SCHED_V3
    cleantrack_daytime_pc_list(mfpc_list, pnow->tm_wday, pnow->tm_hour, pnow->tm_min, pcdbg);
#else
    cleantrack_daytime_pc_list(mfpc_list, pnow->tm_wday, pnow->tm_hour, pcdbg);
#endif
#else
	pctime_flush(timer, data);
#endif
//pctimer:
	mod_timer(timer, next_expires);
}

int
pctime_main(int argc, char *argvs[])
{
	pcdbg = nvram_get_int("pcdbg")?1:0;
        next_expires = NORMAL_PERIOD;

	pctime_config(NULL, NULL);
        tasks_run(pctime_task_t, pctime_xnum, NORMAL_PERIOD);

	return 0;
}
