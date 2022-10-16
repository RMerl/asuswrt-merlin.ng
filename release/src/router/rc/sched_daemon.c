
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <signal.h>

#include <rc.h>
#include <shared.h>
#ifdef RTCONFIG_LIBASUSLOG
#include <libasuslog.h>
#endif
#ifdef RTCONFIG_WL_SCHED_V2
#include <sched_v2.h>
#endif
#ifdef RTCONFIG_TIME_QUOTA
#include <stdio.h>
#include <json.h>
#include "time_quota.h"
#endif

#define USE_TIMERUTIL 1
#ifdef USE_TIMERUTIL
#include <timer_utils.h>
#endif

#define PERIOD_NORMAL			5*TIMER_HZ      /* minisecond */
#define PERIOD_10_SEC			10*TIMER_HZ
#define PERIOD_30_SEC			30*TIMER_HZ
#define PERIOD_86400_SEC		86400*TIMER_HZ

#define SCHED_DAEMON_DEBUG		"/tmp/SCHED_DAEMON_DEBUG"

#ifdef RTCONFIG_LIBASUSLOG
#define SCHED_DAEMON_DBG_LOG	"sched_daemon.log"
extern char *__progname;
#define SCHED_DAEMON_DBG_SYSLOG(fmt,args...) \
	if (nvram_get_int("sched_daemon_syslog")) { \
		asusdebuglog(LOG_INFO, SCHED_DAEMON_DBG_LOG, LOG_CUSTOM, LOG_SHOWTIME, 0, "[%s][%d]][%s:(%d)] "fmt, __progname, getpid(), __FUNCTION__, __LINE__, ##args); \
	}
#else //RTCONFIG_LIBASUSLOG
#define SCHED_DAEMON_DBG_SYSLOG(fmt,args...) {}
#endif //RTCONFIG_LIBASUSLOG

#define SCHED_DAEMON_DBG(fmt,args...) do { \
	if(f_exists(SCHED_DAEMON_DEBUG) > 0) { \
		_dprintf("[SCHED_DAEMON][%s:(%d)]"fmt"\n", __FUNCTION__, __LINE__, ##args); \
	} \
	SCHED_DAEMON_DBG_SYSLOG(fmt,##args) \
} while(0)


#ifdef USE_TIMERUTIL
///////////////////////////////////// task prototype /////////////////////////////////////
static void task_wireless_schedule(struct timer_entry *timer, void *data);
#ifdef MEM_MON
static void task_ps_mem_monitor(struct timer_entry *timer, void *data);
static void task_sys_memfree_monitor(struct timer_entry *timer, void *data);
#endif
#ifdef RTCONFIG_PC_REWARD
static void task_pc_reward_cleaner(struct timer_entry *timer, void *data);
#endif
#ifdef RTCONFIG_TIME_QUOTA
static void task_pc_time_quota(struct timer_entry *timer, void *data);
#endif
///////////////////////////////////// task prototype /////////////////////////////////////

///////////////////////////////////// signal handler prototype /////////////////////////////////////
static void sched_daemon_sigusr1(struct timer_entry *timer, void *data);
static void sched_daemon_exit(struct timer_entry *timer, void *data);
static void sched_daemon_reap(struct timer_entry *timer, void *data);
///////////////////////////////////// signal handler prototype /////////////////////////////////////
static struct task_table sd_task_t[] =
{       /* sig, *timer, *func, *data, expires */
        {SIGALRM, 0, task_wireless_schedule, 0, PERIOD_30_SEC},
#ifdef MEM_MON
        {SIGALRM, 0, task_ps_mem_monitor, 0, PERIOD_86400_SEC},
        {SIGALRM, 0, task_sys_memfree_monitor, 0, PERIOD_86400_SEC},
#endif
#ifdef RTCONFIG_PC_REWARD
        {SIGALRM, 0, task_pc_reward_cleaner, 0, PERIOD_86400_SEC},
#endif
#ifdef RTCONFIG_TIME_QUOTA
        {SIGALRM, 0, task_pc_time_quota, 0, TIME_QUOTA_INTERVAL},
#endif
        {SIGUSR1, 0, sched_daemon_sigusr1, 0, 0},
        {SIGTERM, 0, sched_daemon_exit, 0, 0},
        {SIGCHLD, 0, sched_daemon_reap, 0, 0},
        {0, 0, 0, 0, 0}
};
static int sd_xnum = sizeof(sd_task_t)/sizeof(struct task_table);
#else
#define SCHED_PERIOD		1		/* second */

static unsigned int sigbones = 0;
static void *fn_acts[_NSIG];
static sigset_t sigs_to_catch;
static struct itimerval itv;
#ifdef RTCONFIG_TIME_QUOTA
static int period_time_quota_sec = 0;
#endif
static int period_30_sec = 0;
static int period_3600_sec = 0;
static int period_86400_sec = 0;
///////////////////////////////////// task prototype /////////////////////////////////////
static void task_wireless_schedule(void);
#ifdef MEM_MON
static void task_ps_mem_monitor(void);
static void task_sys_memfree_monitor(void);
#endif
#ifdef RTCONFIG_PC_REWARD
static void task_pc_reward_cleaner(void);
#endif
#ifdef RTCONFIG_TIME_QUOTA
static void task_pc_time_quota(void);
#endif
///////////////////////////////////// task prototype /////////////////////////////////////

///////////////////////////////////// signal handler prototype /////////////////////////////////////
static void sched_daemon_sigusr1(int sig);
static void sched_daemon_exit(int sig);
static void sched_daemon_reap(int sig);
static void sched_daemon(int sig);
///////////////////////////////////// signal handler prototype /////////////////////////////////////
#endif

#ifdef MEM_MON
//the format in nvarm : psname<mem_limit<max_exceed<action>...>...
typedef struct _ps_mem_list ps_mem_list_t;
struct _ps_mem_list {
	char psname[64];
	int mem_limit;
	int max_exceed;
	int action; //0: no aciton, 1: log it, 2 log and kill it
	int curr_exceed;
	ps_mem_list_t *next;
};
static ps_mem_list_t *pmem_list;
#endif

#ifdef RTCONFIG_TIME_QUOTA
static json_object *tq_profile_list = NULL;
static json_object *tq_client_list = NULL;
static json_object *rule_persist_list = NULL;
#endif

#ifndef USE_TIMERUTIL
void sched_daemon_sig(int signo) {
	sigbones |= 1<<signo;
}

typedef void (*sig_func)(int sig);
void put_all_sigs()
{
	int sig;
	sig_func dp=NULL;
	for (sig = 0; sig < (_NSIG - 1); sig++) {
		if(sigbones & 1<<sig  &&  (dp=fn_acts[sig])) {
			(*dp)(sig);
			sigbones &= ~(1<<sig);
		}
	}
}

/*static void catch_sig(int sig)
{
	return;
}*/

void install_sig() 
{
	int sig;
	for (sig = 0; sig < (_NSIG - 1); sig++) {
		if(sig == SIGCHLD 
		|| sig == SIGUSR1
		//|| sig == SIGUSR2
		//|| sig == SIGTSTP
		|| sig == SIGTERM
		|| sig == SIGALRM
		)
			signal(sig, sched_daemon_sig);
		fn_acts[sig] = NULL;
	}

	fn_acts[SIGCHLD] = sched_daemon_reap;
	fn_acts[SIGUSR1] = sched_daemon_sigusr1;
	//fn_acts[SIGUSR2] = catch_sig;
	//fn_acts[SIGTSTP] = catch_sig;
	fn_acts[SIGTERM] = sched_daemon_exit;
	fn_acts[SIGALRM] = sched_daemon;

	sigemptyset(&sigs_to_catch);
	//sigaddset(&sigs_to_catch, SIGCHLD);
	sigaddset(&sigs_to_catch, SIGUSR1);
	//sigaddset(&sigs_to_catch, SIGUSR2);
	//sigaddset(&sigs_to_catch, SIGTSTP);
	sigaddset(&sigs_to_catch, SIGTERM);
	sigaddset(&sigs_to_catch, SIGALRM);
}

static void
alarmtimer(unsigned long sec, unsigned long usec)
{
	itv.it_value.tv_sec = sec;
	itv.it_value.tv_usec = usec;
	itv.it_interval = itv.it_value;
	setitimer(ITIMER_REAL, &itv, NULL);
}

static void sched_daemon(int sig)
{
	//SCHED_DAEMON_DBG("sig=(%d)", sig);
	period_30_sec = (period_30_sec + 1) % 30;
	period_3600_sec = (period_3600_sec + 1) % 3600;
	period_86400_sec = (period_86400_sec + 1) % 86400;

#ifdef RTCONFIG_TIME_QUOTA
	period_time_quota_sec = (period_time_quota_sec + 1) % TIME_QUOTA_INTERVAL;
/*======== The following is for period of time quota ========*/
	if (period_time_quota_sec)  // If the TIME_QUOTA_INTERVAL is greater than 30, it must move the following related codes.
		return;

	task_pc_time_quota();
#endif

/*======== The following is for period 30 seconds ========*/
	if (period_30_sec)
		return;

	task_wireless_schedule();

/*======== The following is for period 1 hour ========*/
	if (period_3600_sec)
		return;

/*======== The following is for period 24 hour ========*/
	if (period_86400_sec)
		return;

#ifdef MEM_MON
	task_ps_mem_monitor();
	task_sys_memfree_monitor();
#endif
#ifdef RTCONFIG_PC_REWARD
	task_pc_reward_cleaner();
#endif
}
#endif

static int svcStatus[12] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
#ifndef RTCONFIG_WL_SCHED_V2
extern int timecheck_item(char *activeTime);
void wl_sched_v1(void)
{
	int activeNow;
	char schedTime[2048];
	char prefix[]="wlXXXXXX_";
	char tmp2[100];
	int unit, item;
	char word[256], *next, tmp[100];
#if defined(RTCONFIG_AMAS) && defined(RTCONFIG_QCA)
	char sctmp[20];
#endif

	SCHED_DAEMON_DBG(" start to timecheck()...");

	item = 0;
	unit = 0;

	if (nvram_match("svc_ready", "0") || nvram_match("wlready", "0"))
		return;

	if (nvram_match("reload_svc_radio", "1"))
	{
		nvram_set("reload_svc_radio", "0");

		foreach (word, nvram_safe_get("wl_ifnames"), next) {
			SKIP_ABSENT_BAND_AND_INC_UNIT(unit);
			svcStatus[item] = -1;
			item++;
			unit++;
		}

		item = 0;
		unit = 0;
	}

	// radio on/off
	foreach (word, nvram_safe_get("wl_ifnames"), next) {
		SKIP_ABSENT_BAND_AND_INC_UNIT(unit);
		snprintf(prefix, sizeof(prefix), "wl%d_", unit);
#if defined(RTCONFIG_AMAS) && defined(RTCONFIG_QCA)
		snprintf(sctmp, sizeof(sctmp), "wl%d_qca_sched", unit);
#endif
		//dbG("[watchdog] timecheck unit=%s radio=%s, timesched=%s\n", prefix, nvram_safe_get(strcat_r(prefix, "radio", tmp)), nvram_safe_get(strcat_r(prefix, "timesched", tmp2))); // radio toggle test
		if (nvram_match(strcat_r(prefix, "radio", tmp), "0") ||
			nvram_match(strcat_r(prefix, "timesched", tmp2), "0")) {
			item++;
			unit++;
			continue;
		}

		/*transfer wl_sched NULL value to 000000 value, because
		of old version firmware with wrong default value*/
		if (!strcmp(nvram_safe_get(strcat_r(prefix, "sched", tmp)), ""))
		{
			nvram_set(strcat_r(prefix, "sched", tmp),"000000");
			//nvram_set("wl_sched", "000000");
		}

		snprintf(schedTime, sizeof(schedTime), "%s", nvram_safe_get(strcat_r(prefix, "sched", tmp)));

		activeNow = timecheck_item(schedTime);

#if defined(RTCONFIG_LYRA_5G_SWAP)
		snprintf(tmp, sizeof(tmp), "%d", swap_5g_band(unit));
#else
		snprintf(tmp, sizeof(tmp), "%d", unit);
#endif
		SCHED_DAEMON_DBG(" unit=%d, activeNow=%d", unit, activeNow);

		if (svcStatus[item] != activeNow) {
#ifdef RTCONFIG_QCA
#if defined(RTCONFIG_AMAS)
			nvram_set_int(sctmp,activeNow);
#endif

#if defined(RTCONFIG_LYRA_5G_SWAP)
			if (match_radio_status(swap_5g_band(unit), activeNow)) {
#else
			if (match_radio_status(unit, activeNow)) {
#endif
				svcStatus[item] = activeNow;
				item++;
				unit++;
				continue;
			}
#else
			svcStatus[item] = activeNow;
#endif

			if (activeNow == 0) {
				eval("radio", "off", tmp);
				SCHED_DAEMON_DBG(" Turn radio [band_index=%s] off", tmp);
				logmessage("wifi scheduler", "Turn radio [band_index=%s] off.", tmp);
			} else {
				eval("radio", "on", tmp);
				SCHED_DAEMON_DBG(" Turn radio [band_index=%s] on", tmp);
				logmessage("wifi scheduler", "Turn radio [band_index=%s] on.", tmp);
			}
		}
		item++;
		unit++;

	}
}
#else
void wl_sched_v2(void)
{
	int activeNow;
	char schedTime[2048];
	char prefix[]="wlXXXXXX_", tmp[100], tmp2[100];
	char word[256], *next;
	int unit = 0, item = 0;
#if defined(RTCONFIG_AMAS) && defined(RTCONFIG_QCA)
	char sctmp[20];
#endif

	// Check whether conversion needed.
	convert_wl_sched_v1_to_sched_v2();

	SCHED_DAEMON_DBG("start to timecheck()...");

	item = 0;
	unit = 0;

	if (nvram_match("reload_svc_radio", "1"))
	{
		nvram_set("reload_svc_radio", "0");

		foreach (word, nvram_safe_get("wl_ifnames"), next) {
			SKIP_ABSENT_BAND_AND_INC_UNIT(unit);
			svcStatus[item] = -1;
			item++;
			unit++;
		}

		item = 0;
		unit = 0;
	}

	// radio on/off
	if (nvram_match("svc_ready", "1") && nvram_match("wlready", "1"))
	foreach (word, nvram_safe_get("wl_ifnames"), next) {
		SKIP_ABSENT_BAND_AND_INC_UNIT(unit);
		snprintf(prefix, sizeof(prefix), "wl%d_", unit);
#if defined(RTCONFIG_AMAS) && defined(RTCONFIG_QCA)
		snprintf(sctmp, sizeof(sctmp), "wl%d_qca_sched", unit);
#endif

		//dbG("[watchdog] timecheck unit=%s radio=%s, timesched=%s\n", prefix, nvram_safe_get(strcat_r(prefix, "radio", tmp)), nvram_safe_get(strcat_r(prefix, "timesched", tmp2))); // radio toggle test
		if (nvram_match(strcat_r(prefix, "radio", tmp), "0") ||
			nvram_match(strcat_r(prefix, "timesched", tmp2), "0")) {
			item++;
			unit++;
			continue;
		}

		/*transfer wl_sched NULL value to "" value, because
		of old version firmware with wrong default value*/
		if (!nvram_get(strcat_r(prefix, "sched_v2", tmp)))
		{
			nvram_set(strcat_r(prefix, "sched_v2", tmp), "");
		}

		snprintf(schedTime, sizeof(schedTime), "%s", nvram_safe_get(strcat_r(prefix, "sched_v2", tmp)));

		activeNow = check_sched_v2_on_off(schedTime);

#if defined(RTCONFIG_LYRA_5G_SWAP)
		snprintf(tmp, sizeof(tmp), "%d", swap_5g_band(unit));
#else
		snprintf(tmp, sizeof(tmp), "%d", unit);
#endif
		SCHED_DAEMON_DBG(" uschedTime=%s, unit=%d, activeNow=%d", schedTime, unit, activeNow);

		if (svcStatus[item] != activeNow) {
#ifdef RTCONFIG_QCA
#if defined(RTCONFIG_AMAS)
			nvram_set_int(sctmp,activeNow);
#endif

#if defined(RTCONFIG_LYRA_5G_SWAP)
			if (match_radio_status(swap_5g_band(unit), activeNow)) {
#else
			if (match_radio_status(unit, activeNow)) {
#endif
				svcStatus[item] = activeNow;
				item++;
				unit++;
				continue;
			}
#else
			svcStatus[item] = activeNow;
#endif

			if (activeNow == 0) {
				eval("radio", "off", tmp);
				SCHED_DAEMON_DBG(" Turn radio [band_index=%s] off", tmp);
				logmessage("wifi scheduler", "Turn radio [band_index=%s] off.", tmp);
			} else {
				eval("radio", "on", tmp);
				SCHED_DAEMON_DBG(" Turn radio [band_index=%s] on", tmp);
				logmessage("wifi scheduler", "Turn radio [band_index=%s] on.", tmp);
			}
		}
		item++;
		unit++;

	}
}
#endif //#ifndef RTCONFIG_WL_SCHED_V2

#ifdef MEM_MON
#define MAX_NVRAM_PS_MEM_LEN 512
//the format in nvarm : psname<mem_limit<max_exceed<action>...>...
void get_ps_mem_list(ps_mem_list_t **ps_mem_list)
{
	char ps_mem_limit_str[MAX_NVRAM_PS_MEM_LEN];
	char word1[MAX_NVRAM_PS_MEM_LEN], *next_word1;
	char word2[MAX_NVRAM_PS_MEM_LEN], *next_word2;
	int count1, count2;
	ps_mem_list_t **tmp_pmem = ps_mem_list;

	snprintf(ps_mem_limit_str, sizeof(ps_mem_limit_str), "%s", nvram_safe_get("mem_limit_setting"));
	foreach_62_keep_empty_string(count1, word1, ps_mem_limit_str, next_word1) {
		*tmp_pmem = (ps_mem_list_t *)malloc(sizeof(ps_mem_list_t));
		(*tmp_pmem)->curr_exceed = 0;
		foreach_60_keep_empty_string(count2, word2, word1, next_word2) {
			if (count2 == 3)
				snprintf((*tmp_pmem)->psname, sizeof((*tmp_pmem)->psname), "%s", word2);
			else if (count2 == 2)
				(*tmp_pmem)->mem_limit = (int)strtol(word2, NULL, 10);
			else if (count2 == 1)
				(*tmp_pmem)->max_exceed = (int)strtol(word2, NULL, 10);
			else if (count2 == 0)
				(*tmp_pmem)->action = (int)strtol(word2, NULL, 10);
		}
		while(*tmp_pmem != NULL)
			tmp_pmem = &((*tmp_pmem)->next);
	}
}

void free_ps_mem_list(ps_mem_list_t **ps_mem_list)
{
	ps_mem_list_t *tmp_pmem, *old_pmem;

	if(ps_mem_list == NULL)
		return;

	tmp_pmem = *ps_mem_list;
	while(tmp_pmem != NULL){
		old_pmem = tmp_pmem;
		tmp_pmem = tmp_pmem->next;
		free(old_pmem);
	}
}
#endif

#ifdef USE_TIMERUTIL
static void task_wireless_schedule(struct timer_entry *timer, void *data)
#else
static void task_wireless_schedule(void)
#endif
{
#ifndef RTCONFIG_WL_SCHED_V2
	wl_sched_v1();
#else
	wl_sched_v2();
#endif
#ifdef USE_TIMERUTIL
	mod_timer(timer, PERIOD_30_SEC);
#endif
}

/* 
   If the memory usage of a process higher than mem_limit_setting 3 times in row,
   log it to syslog or kill it according to the action value of mem_limit_setting.
 */
#ifdef MEM_MON
#ifdef USE_TIMERUTIL
static void task_ps_mem_monitor(struct timer_entry *timer, void *data)
#else
static void task_ps_mem_monitor(void)
#endif
{
	ps_mem_list_t *pmem;
	for (pmem = pmem_list; pmem != NULL; pmem = pmem->next) {
		if (process_mem_used(pidof(pmem->psname)) > pmem->mem_limit) {
			pmem->curr_exceed++;
			if (pmem->curr_exceed >= pmem->max_exceed) {
				if (pmem->action == 1) {  // log it
					logmessage("SCHED_DAEMON", "%s memory usage exceed %d.\n", pmem->psname, pmem->mem_limit);
					SCHED_DAEMON_DBG(" %s memory usage exceed %d.", pmem->psname, pmem->mem_limit);
				} else if (pmem->action == 2) { // kill it
					logmessage("SCHED_DAEMON", "%s memory usage exceed %d, kill it.\n", pmem->psname, pmem->mem_limit);
					SCHED_DAEMON_DBG(" %s memory usage exceed %d, kill it.", pmem->psname, pmem->mem_limit);
					killall_tk(pmem->psname);
				}
				pmem->curr_exceed = 0;
			}
		} else
			pmem->curr_exceed = 0;
	}
#ifdef USE_TIMERUTIL
	mod_timer(timer, PERIOD_86400_SEC);
#endif
}

/* 
   If the system free memory is lower than mem_limit_sys_memfree 3 times in row,
   log it to syslog.
 */
#ifdef USE_TIMERUTIL
static void task_sys_memfree_monitor(struct timer_entry *timer, void *data)
#else
static void task_sys_memfree_monitor(void)
#endif
{
	char cmd[64] = {0};
	FILE *fp = NULL;
	unsigned int memfree = 0;
	int memfree_threshold = nvram_get_int("mem_limit_sys_memfree");
	static int sys_memfree_exceed_cnt = 0;

	if (memfree_threshold <= 0)
		return;

	snprintf(cmd, sizeof(cmd), "/bin/cat /proc/meminfo | /bin/grep MemFree 2>/dev/null");
	if((fp = popen(cmd, "r")) == NULL){
		SCHED_DAEMON_DBG("\tCannot execute cat.");
		SCHED_DAEMON_DBG("... Failed");

		return;
	}

	fscanf(fp, "MemFree: %u %*s", &memfree);
	pclose(fp);

	if (memfree < memfree_threshold) {
		sys_memfree_exceed_cnt++;
		if (sys_memfree_exceed_cnt >= 3) {
			logmessage("SCHED_DAEMON", "System free memory is now %u lower than %u.\n", memfree, memfree_threshold);
			SCHED_DAEMON_DBG("System free memory is now %u, it is lower than %u.\n", memfree, memfree_threshold);
			sys_memfree_exceed_cnt = 0;
		}
	} else
		sys_memfree_exceed_cnt = 0;

#ifdef USE_TIMERUTIL
	mod_timer(timer, PERIOD_86400_SEC);
#endif
}
#endif

#ifdef RTCONFIG_PC_REWARD
#ifdef USE_TIMERUTIL
static void task_pc_reward_cleaner(struct timer_entry *timer, void *data)
#else
static void task_pc_reward_cleaner(void)
#endif
{
	system("pc_reward clean");
#ifdef USE_TIMERUTIL
	mod_timer(timer, PERIOD_86400_SEC);
#endif
}
#endif

#ifdef RTCONFIG_TIME_QUOTA
#ifdef USE_TIMERUTIL
static void task_pc_time_quota(struct timer_entry *timer, void *data)
#else
static void task_pc_time_quota(void)
#endif
{
	char now_s[32];
#ifdef USE_TIMERUTIL
	int next_expires;
#endif
	snprintf(now_s, sizeof(now_s), "%lu", time(NULL));
	if (!tq_profile_list)
		tq_profile_list = json_object_new_object();

	if (!tq_client_list)
		tq_client_list = json_object_new_object();

	if (!rule_persist_list)
		rule_persist_list = json_object_new_object();

	check_and_update_tq_profile_list(tq_profile_list, tq_client_list, rule_persist_list, now_s);
	if (nvram_match("time_quota_dbg", "1") || f_exists(SCHED_DAEMON_DEBUG)) {
		SCHED_DAEMON_DBG("tq_profile_list : %s", json_object_get_string(tq_profile_list));
		SCHED_DAEMON_DBG("tq_client_list : %s", json_object_get_string(tq_client_list));
		SCHED_DAEMON_DBG("=================================================================");
	}
#ifdef USE_TIMERUTIL
	next_expires = nvram_get_int("nf_conntrack_timer"); // Check nfcm interval dynamically.
	mod_timer(timer, next_expires > 0 ? next_expires*TIMER_HZ : TIME_QUOTA_INTERVAL*TIMER_HZ);
#endif
}
#endif

#ifdef USE_TIMERUTIL
static void sched_daemon_sigusr1(struct timer_entry *timer, void *data)
#else
static void sched_daemon_sigusr1(int sig)
#endif
{
	SCHED_DAEMON_DBG("SIGUSR1 got!!!");
#ifdef RTCONFIG_TIME_QUOTA
	apply_tq_rules_for_restart_firewall(rule_persist_list);
#endif
}

#ifdef USE_TIMERUTIL
static void sched_daemon_exit(struct timer_entry *timer, void *data)
#else
static void sched_daemon_exit(int sig)
#endif
{
#ifdef MEM_MON
	if (pmem_list) {
		free_ps_mem_list(&pmem_list);
		pmem_list = NULL;
	}
#endif
#ifdef RTCONFIG_TIME_QUOTA
	if (tq_profile_list)
		json_object_put(tq_profile_list);
	if (tq_client_list)
		json_object_put(tq_client_list);
	if (rule_persist_list)
		json_object_put(rule_persist_list);
#endif
	exit(0);
}

#ifdef USE_TIMERUTIL
static void sched_daemon_reap(struct timer_entry *timer, void *data)
#else
static void sched_daemon_reap(int sig)
#endif
{
	chld_reap(SIGCHLD);
}

int sched_daemon_main(int argc, char *argv[])
{
	setenv("TZ", nvram_safe_get("time_zone_x"), 1);

	_dprintf("TZ sched_daemon\n");

#ifdef MEM_MON
	get_ps_mem_list(&pmem_list);
#endif

#ifdef USE_TIMERUTIL
#ifdef RTCONFIG_TIME_QUOTA
	int i;
	int time_quota_period = nvram_get_int("nf_conntrack_timer"); // Check nfcm interval dynamically.
	if (time_quota_period > 0) {
		for (i=0; i<sd_xnum; i++) {
			if (sd_task_t[i].tfunc == task_pc_time_quota) {
				sd_task_t[i].fire_time = time_quota_period*TIMER_HZ;
				//_dprintf("sd_task_t[%d].tfunc=%p\n", i, sd_task_t[i].tfunc);
				break;
			}
		}
	}
#endif
	tasks_run(sd_task_t, sd_xnum, 0);
#else
	install_sig();
	/* set timer */
	alarmtimer(SCHED_PERIOD, 0);

	/* Most of time it goes to sleep */
	while(1) {
		while(sigbones)
			put_all_sigs();
		pause();
	}
#endif
	return 0;
}
