
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

static pc_s *mfpc_list = NULL;
static int mfpc_count = -1;
static int pcdbg=0;

static void 
pctime_free(struct timer_entry *timer, void *data)
{
	int i;

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
	if(mfpc_list)
		free_pc_list(&mfpc_list);
	get_all_pc_list(&mfpc_list);
	mfpc_count = count_pc_rules(mfpc_list, 1);
}

static void 
pctime_flush(struct timer_entry *timer, void *data)
{
	printf("flush pclist conntracks\n");

	eval("pc", "flush");
}

static void
pctime_loop(struct timer_entry *timer, void *data)
{
        if(nvram_get_int("MULTIFILTER_ALL")==0 || mfpc_count==0)
                goto pctimer;
        if ((nvram_get_int("ntp_ready") != 1) && (nvram_get_int("qtn_ntp_ready") != 1))
                goto pctimer;

        time_t t = time(NULL);
        struct tm *pnow = localtime(&t);

        cleantrack_daytime_pc_list(mfpc_list, pnow->tm_wday, pnow->tm_hour, pcdbg);

pctimer:
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
