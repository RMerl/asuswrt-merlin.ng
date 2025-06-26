
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
#ifdef RTCONFIG_MULTIWAN_PROFILE
#include "multi_wan.h"
#endif
#ifdef RTCONFIG_CFGSYNC
#include <cfg_event.h>
#endif

#define USE_TIMERUTIL 1
#ifdef USE_TIMERUTIL
#include <timer_utils.h>
#endif

#define PID_SCHED_DAEMON "/var/run/sched_daemon.pid"

#define PERIOD_NORMAL			5*TIMER_HZ      /* minisecond */
#define PERIOD_10_SEC			10*TIMER_HZ
#define PERIOD_30_SEC			30*TIMER_HZ
#define PERIOD_120_SEC			120*TIMER_HZ
#define PERIOD_86400_SEC		86400*TIMER_HZ

#define SCHED_DAEMON_DEBUG		"/tmp/SCHED_DAEMON_DEBUG"

extern char *__progname;
#ifdef RTCONFIG_LIBASUSLOG
#define SCHED_DAEMON_DBG_LOG	"sched_daemon.log"
#define SCHED_DAEMON_DBG_SYSLOG(fmt,args...) \
	if (nvram_get_int("sched_daemon_syslog")) { \
		asusdebuglog(LOG_INFO, SCHED_DAEMON_DBG_LOG, LOG_CUSTOM, LOG_SHOWTIME, 0, "[%s][%d]][%s:(%d)] "fmt, __progname, getpid(), __FUNCTION__, __LINE__, ##args); \
	}
#else //RTCONFIG_LIBASUSLOG
#define SCHED_DAEMON_DBG_SYSLOG(fmt,args...) {}
#endif //RTCONFIG_LIBASUSLOG

#define SCHED_DAEMON_DBG(fmt,args...) do { \
	if(f_exists(SCHED_DAEMON_DEBUG) > 0) { \
		_dprintf("[%s][%d][%s:(%d)]"fmt"\n", __progname, getpid(), __FUNCTION__, __LINE__, ##args); \
	} \
	SCHED_DAEMON_DBG_SYSLOG(fmt,##args) \
} while(0)

#ifndef RTCONFIG_AVOID_TZ_ENV
static char time_zone_t[32]={0};
#endif


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
#ifdef RTCONFIG_MULTIWAN_PROFILE
static void task_mtwan_schedule(struct timer_entry *timer, void *data);
#endif
#ifndef RTCONFIG_AVOID_TZ_ENV
static void task_timezone_checking(struct timer_entry *timer, void *data);
#endif
#ifdef RTCONFIG_REBOOT_SCHEDULE
static void task_reboot_schedule(struct timer_entry *timer, void *data);
#endif
#ifdef RTCONFIG_HNS
static void task_hns_history(struct timer_entry *timer, void *data);
static void task_hns_protection(struct timer_entry *timer, void *data);
static void task_hns_signature(struct timer_entry *timer, void *data);
static void task_hns_event_cc(struct timer_entry *timer, void *data);
static void task_hns_alive(struct timer_entry *timer, void *data);
#endif
///////////////////////////////////// task prototype /////////////////////////////////////

///////////////////////////////////// signal handler prototype /////////////////////////////////////
static void sched_daemon_sigusr1(struct timer_entry *timer, void *data);
static void sched_daemon_sigusr2(struct timer_entry *timer, void *data);
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
#ifdef RTCONFIG_MULTIWAN_PROFILE
        {SIGALRM, 0, task_mtwan_schedule, 0, PERIOD_30_SEC},
#endif
#ifndef RTCONFIG_AVOID_TZ_ENV
        {SIGALRM, 0, task_timezone_checking, 0, PERIOD_10_SEC},
#endif
#ifdef RTCONFIG_REBOOT_SCHEDULE
		{SIGALRM, 0, task_reboot_schedule, 0, PERIOD_30_SEC},
#endif
#ifdef RTCONFIG_HNS
        {SIGALRM, 0, task_hns_history, 0, PERIOD_120_SEC},
        {SIGALRM, 0, task_hns_protection, 0, PERIOD_120_SEC},
        {SIGALRM, 0, task_hns_signature, 0, PERIOD_86400_SEC},
        {SIGALRM, 0, task_hns_event_cc, 0, PERIOD_10_SEC},
        {SIGALRM, 0, task_hns_alive, 0, PERIOD_30_SEC},
#endif
        {SIGUSR1, 0, sched_daemon_sigusr1, 0, 0},
        {SIGUSR2, 0, sched_daemon_sigusr2, 0, 0},
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
static int period_120_sec = 0;
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
#ifdef RTCONFIG_MULTIWAN_PROFILE
static void task_mtwan_schedule(void);
#endif
#ifndef RTCONFIG_AVOID_TZ_ENV
static void task_timezone_checking(void);
#endif
#ifdef RTCONFIG_REBOOT_SCHEDULE
static void task_reboot_schedule(void);
#endif
#ifdef RTCONFIG_HNS
static void task_hns_history(void);
static void task_hns_protection(void);
static void task_hns_signature(void);
static void task_hns_event_cc(void);
static void task_hns_alive(void);
#endif
///////////////////////////////////// task prototype /////////////////////////////////////

///////////////////////////////////// signal handler prototype /////////////////////////////////////
static void sched_daemon_sigusr1(int sig);
static void sched_daemon_sigusr2(int sig);
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
	fn_acts[SIGUSR2] = sched_daemon_sigusr2;
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
	period_10_sec = (period_10_sec + 1) % 10;
	period_30_sec = (period_30_sec + 1) % 30;
	period_120_sec = (period_120_sec + 1) % 120;
	period_3600_sec = (period_3600_sec + 1) % 3600;
	period_86400_sec = (period_86400_sec + 1) % 86400;

#ifdef RTCONFIG_TIME_QUOTA
	period_time_quota_sec = (period_time_quota_sec + 1) % TIME_QUOTA_INTERVAL;
/*======== The following is for period of time quota ========*/
	if (period_time_quota_sec)  // If the TIME_QUOTA_INTERVAL is greater than 30, it must move the following related codes.
		return;

	task_pc_time_quota();
#endif

/*======== The following is for period 10 seconds ========*/
	if (period_10_sec)
		return;

#ifndef RTCONFIG_AVOID_TZ_ENV
	task_timezone_checking();
#endif
#ifdef RTCONFIG_HNS
	task_hns_event_cc();
#endif

#ifndef RTCONFIG_REBOOT_SCHEDULE
	task_reboot_schedule();
#endif

/*======== The following is for period 30 seconds ========*/
	if (period_30_sec)
		return;

	task_wireless_schedule();
#ifdef RTCONFIG_MULTIWAN_PROFILE
	task_mtwan_schedule();
#endif
#ifdef RTCONFIG_HNS
	task_hns_alive();
#endif

/*======== The following is for period 120 seconds ========*/
	if (period_120_sec)
		return;

#ifdef RTCONFIG_HNS
	task_hns_history();
	task_hns_protection();
#endif

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
#ifdef RTCONFIG_HNS
	task_hns_signature();
#endif
}
#endif

// the svcStatus and vif_svcstatus is used to save previous radio state.
static int svcStatus[12] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
static int vif_svcStatus[12][32];
static void reset_svc_status() {
	int unit = 0, subunit;
	char word[256], *next;
#if defined(RTCONFIG_MULTILAN_CFG)
	char word2[256], *next2;
	char nv[81];
#endif

	foreach (word, nvram_safe_get("wl_ifnames"), next) {
		SKIP_ABSENT_BAND_AND_INC_UNIT(unit);
		svcStatus[unit] = -1;

#if defined(RTCONFIG_MULTILAN_CFG)
		subunit = 1;
		memset(nv, 0, sizeof(nv));
		snprintf(nv, sizeof(nv), "wl%d_vifnames", unit);
		foreach (word2, nvram_safe_get(nv), next2) {
			vif_svcStatus[unit][subunit] = -1;
			subunit++;
		}
#endif

		unit++;
	}
}

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

		reset_svc_status();
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
				set_wlan_service_status(safe_atoi(tmp), -1, 0);
				SCHED_DAEMON_DBG(" Turn radio [band_index=%s] off", tmp);
				logmessage("wifi scheduler", "Turn radio [band_index=%s] off.", tmp);
			} else {
				set_wlan_service_status(safe_atoi(tmp), -1, 1);
				SCHED_DAEMON_DBG(" Turn radio [band_index=%s] on", tmp);
				logmessage("wifi scheduler", "Turn radio [band_index=%s] on.", tmp);
			}
		}
		item++;
		unit++;

	}
}
#else
#if defined(RTCONFIG_MULTILAN_CFG)
extern wifi_band_cap_st band_cap[MAX_BAND_CAP_LIST_SIZE];
extern int band_cap_total;
extern int is_reserved_if(int unit, int subunit, int filter_main);
extern int check_wlX_sched(int unit);
#endif

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

#ifdef RTCONFIG_MULTILAN_CFG
	int activeNow2;
	int expireNow;
	int timesched;
	char schedTime2[2048];
	char expireTime[2048];
	char prefix2[]="wlXXXXXX_";
	char word2[256], *next2;
	char nv[81];
	int subunit = 0;
#if defined(RTCONFIG_AMAS) && defined(RTCONFIG_QCA)
	char sctmp2[32];
#endif
	int wlX_activeNow = -1;
#endif
	int bss_status;
	int radio_status;

	// Check whether conversion needed.
	convert_wl_sched_v1_to_sched_v2();

	SCHED_DAEMON_DBG("start to timecheck()...");

	item = 0;
	unit = 0;

	if (nvram_match("reload_svc_radio", "1"))
	{
		nvram_set("reload_svc_radio", "0");

		reset_svc_status();
	}

#ifdef RTCONFIG_MULTILAN_CFG
	band_cap_total = 0;
	memset(band_cap, 0, (sizeof(wifi_band_cap_st)*MAX_BAND_CAP_LIST_SIZE));
	get_wifi_band_cap(band_cap, MAX_BAND_CAP_LIST_SIZE, &band_cap_total);
#endif

	// radio on/off
	if (nvram_match("svc_ready", "1") && nvram_match("wlready", "1"))
	foreach (word, nvram_safe_get("wl_ifnames"), next) {
		SKIP_ABSENT_BAND_AND_INC_UNIT(unit);

#if defined(RTCONFIG_MULTILAN_CFG)
		wlX_activeNow = check_wlX_sched(unit);
		subunit = 1;
		snprintf(nv, sizeof(nv), "wl%d_vifnames", unit);
		foreach (word2, nvram_safe_get(nv), next2) {
			memset(prefix2, 0, sizeof(prefix2));
			snprintf(prefix2, sizeof(prefix2), "wl%d.%d_", unit, subunit);
#if defined(RTCONFIG_AMAS) && defined(RTCONFIG_QCA)
			snprintf(sctmp2, sizeof(sctmp2), "wl%d.%d_qca_sched", unit, subunit);
#endif
			timesched = nvram_get_int(strcat_r(prefix2, "timesched", tmp2));
			if ((nvram_get_int(strcat_r(prefix2, "radio", tmp)) == 0) ||
				(nvram_get_int(strcat_r(prefix2, "bss_enabled", tmp2)) == 0)/* || 
				(timesched == 0)*/) {
				subunit++;
				continue;
			}

			// if wlX_activeNow is equal -1, its represents the scheuler of main wifi is not set. We shoudld not control main interface.
			if (is_reserved_if(unit, subunit, (wlX_activeNow == -1))) {
				SCHED_DAEMON_DBG("[wifi-scheduler] [wl%d.%d] is reserved interface. bypass it.\n", unit, subunit);
				subunit++;
				continue;
			}

			// On RE node, the amas_lanctl should be the radio controler if RE is disconnected from CAP.
			if (nvram_get_int("re_mode") == 1 && nvram_get_int("cfg_alive") ==0 ) {
				subunit++;
				continue;
			}

			/*transfer wl_sched NULL value to "" value, because
			of old version firmware with wrong default value*/
			if (!nvram_get(strcat_r(prefix2, "sched_v2", tmp)))
			{
				nvram_set(strcat_r(prefix2, "sched_v2", tmp), "");
			}

			memset(schedTime2, 0, sizeof(schedTime2));
			if (wlX_activeNow >= 0) {
				activeNow2 = wlX_activeNow;
			} else {
				if ((timesched & TIMESCHED_ACCESSTIME) > 0) {
					snprintf(expireTime, sizeof(expireTime), "%s", nvram_safe_get(strcat_r(prefix2, "expiretime", tmp)));
					expireNow = check_expire_on_off(expireTime);
					if (expireNow < 0) { // not in expire time period
						// if access time is enabled and not in expire time preiod, 
						// we need to check if schedule is enabled too (consider both of access time and schedule are enabled).
						if ((timesched & TIMESCHED_SCHEDULE) > 0) {
							snprintf(schedTime2, sizeof(schedTime2), "%s", nvram_safe_get(strcat_r(prefix2, "sched_v2", tmp)));
							activeNow2 = check_sched_v2_on_off(schedTime2);
						} else {
							activeNow2 = (expireNow == -2) ? 0 : 1;
						}
					} else { // in expire time period
						activeNow2 = expireNow;
					}
				} else if ((timesched & TIMESCHED_SCHEDULE) > 0) {
					snprintf(schedTime2, sizeof(schedTime2), "%s", nvram_safe_get(strcat_r(prefix2, "sched_v2", tmp)));
					activeNow2 = check_sched_v2_on_off(schedTime2);
				} else {
					activeNow2 = 1;  // both of access time and schedule are not enabled, default is radio on.
				}
			}

			memset(tmp2, 0, sizeof(tmp2));
			snprintf(tmp2, sizeof(tmp2), "%d", subunit);

			memset(tmp, 0, sizeof(tmp));
#if defined(RTCONFIG_LYRA_5G_SWAP)
			snprintf(tmp, sizeof(tmp), "%d", swap_5g_band(unit));
#else
			snprintf(tmp, sizeof(tmp), "%d", unit);
#endif
			SCHED_DAEMON_DBG("[wifi-scheduler] 3 uschedTime=%s, unit=%d, subunit=%d, activeNow2=%d, wlX_activeNow=%d\n", schedTime2, unit, subunit, activeNow2, wlX_activeNow);

			if (vif_svcStatus[item][subunit] != activeNow2) {
#ifdef RTCONFIG_QCA
#if defined(RTCONFIG_AMAS)
				nvram_set_int(sctmp,activeNow2);
#endif

#if defined(RTCONFIG_LYRA_5G_SWAP)
				if (match_radio_status(swap_5g_band(unit), activeNow2)) {
#else
				if (match_radio_status(unit, activeNow2)) {
#endif
					vif_svcStatus[item][subunit] = activeNow2;
					subunit++;
					continue;
				}
#else
				vif_svcStatus[item][subunit] = activeNow2;
#endif

				if (activeNow2 == 0) {
					bss_status = get_wlan_service_status(safe_atoi(tmp), safe_atoi(tmp2));
					if (bss_status == 1) { // radio is on
						set_wlan_service_status(safe_atoi(tmp), safe_atoi(tmp2), 0);
						SCHED_DAEMON_DBG("[wifi-scheduler] Turn radio [band_index=%s, subunit=%s] off\n", tmp, tmp2);
						logmessage("wifi scheduler", "Turn radio [band_index=%s, subunit=%s] off.", tmp, tmp2);
					} else {
						if (bss_status == 0) {
							SCHED_DAEMON_DBG("[wifi-scheduler] Turn radio [band_index=%s, subunit=%s] off. Already off, no need to set it again.\n", tmp, tmp2);
							//logmessage("wifi-scheduler", "Turn radio [band_index=%s, subunit=%s] off. Already off, no need to set it again.\n", tmp, tmp2);
						} else
							SCHED_DAEMON_DBG("[wifi-scheduler] Turn radio [band_index=%s, subunit=%s] off. Error occur(%d).\n", tmp, tmp2, bss_status);
					}
				} else {
					// We must check if radio is on, otherwsie the set set_wlan_service_status will failed.
					if (wlX_activeNow == 1) {
						radio_status = get_radio(safe_atoi(tmp), -1);
						if (radio_status == 0) {
							set_radio(1, safe_atoi(tmp), -1);
							SCHED_DAEMON_DBG(" Turn radio [band_index=%s] on", tmp);
							logmessage("wifi scheduler", "Turn radio [band_index=%s] on.", tmp);
						}
					}
					bss_status = get_wlan_service_status(safe_atoi(tmp), safe_atoi(tmp2));
					if (bss_status == 0) { // radio is off
						set_wlan_service_status(safe_atoi(tmp), safe_atoi(tmp2), 1);
						SCHED_DAEMON_DBG("[wifi-scheduler] Turn radio [band_index=%s, subunit=%s] on\n", tmp, tmp2);
						logmessage("wifi scheduler", "Turn radio [band_index=%s, subunit=%s] on.", tmp, tmp2);
					} else {
						if (bss_status == 1) {
							SCHED_DAEMON_DBG("[wifi-scheduler] Turn radio [band_index=%s, subunit=%s] on. Already on, no need to set it again.\n", tmp, tmp2);
							//logmessage("wifi scheduler", "Turn radio [band_index=%s, subunit=%s] on. Already on, no need to set it again.\n", tmp, tmp2);
						} else
							SCHED_DAEMON_DBG("[wifi-scheduler] Turn radio [band_index=%s, subunit=%s] on. Error occur(%d).\n", tmp, tmp2, bss_status);
					}
				}	
			}
		
			subunit++;
		}
#endif	// RTCONFIG_MULTILAN_CFG
		
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
				radio_status = get_radio(safe_atoi(tmp), -1);
				if (radio_status == 1) { // radio is on
					set_radio(0, safe_atoi(tmp), -1);
					SCHED_DAEMON_DBG(" Turn radio [band_index=%s] off", tmp);
					logmessage("wifi scheduler", "Turn radio [band_index=%s] off.", tmp);
#ifdef RTCONFIG_ENERGY_SAVE
					esr_set_wlunit(safe_atoi(tmp), ES_STS_OFF, "WL_SCHED");
#endif
				} else {
					if (radio_status == 0) {
						SCHED_DAEMON_DBG("[wifi-scheduler] Turn radio [band_index=%s] off. Already off, no need to set it again.\n", tmp);
						//logmessage("wifi scheduler", "Turn radio [band_index=%s, subunit=%s] off. Already off, no need to set it again.\n", tmp, tmp2);
					} else
						SCHED_DAEMON_DBG("[wifi-scheduler] Turn radio [band_index=%s] off. Error occur(%d).\n", tmp, radio_status);
				}
			} else {
				radio_status = get_radio(safe_atoi(tmp), -1);
				if (radio_status == 0) { // radio is off
					set_radio(1, safe_atoi(tmp), -1);
					SCHED_DAEMON_DBG(" Turn radio [band_index=%s] on", tmp);
					logmessage("wifi scheduler", "Turn radio [band_index=%s] on.", tmp);
#ifdef RTCONFIG_ENERGY_SAVE
					esr_set_wlunit(safe_atoi(tmp), ES_STS_ON, "WL_SCHED");
#endif
				} else {
					if (radio_status == 1) {
						SCHED_DAEMON_DBG("[wifi-scheduler] Turn radio [band_index=%s] on. Already on, no need to set it again.\n", tmp);
						//logmessage("wifi scheduler", "Turn radio [band_index=%s, subunit=%s] on. Already on, no need to set it again.\n", tmp, tmp2);
					} else
						SCHED_DAEMON_DBG("[wifi-scheduler] Turn radio [band_index=%s] on. Error occur(%d).\n", tmp, radio_status);
				}
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
		memset((void *)(*tmp_pmem), 0, sizeof(ps_mem_list_t));
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

#ifdef RTCONFIG_MULTIWAN_PROFILE
#ifdef USE_TIMERUTIL
static void task_mtwan_schedule(struct timer_entry *timer, void *data)
#else
static void task_mtwan_schedule(void)
#endif
{
	char mtwan_prefix[16] = {0};
	int mtwan_idx;
	int mtwan_group, to_group;

	SCHED_DAEMON_DBG("Start Multi-WAN time check...");

	if (!nvram_get_int("svc_ready"))
	{
#ifdef USE_TIMERUTIL
		mod_timer(timer, PERIOD_30_SEC);
#endif
		return;
	}

	for (mtwan_idx = MAX_MTWAN_PROFILE_NUM; mtwan_idx > 0; mtwan_idx--) {
		snprintf(mtwan_prefix, sizeof(mtwan_prefix), "mtwan%d_", mtwan_idx);
		if (!nvram_pf_get_int(mtwan_prefix, "enable"))
			continue;
		if (nvram_pf_get_int(mtwan_prefix, "mode") != MTWAN_MODE_TIME)
			continue;

		mtwan_group = nvram_pf_get_int(mtwan_prefix, "group");
		if (check_sched_v2_on_off(nvram_pf_safe_get(mtwan_prefix, "sched"))) {
			to_group = mtwan_get_first_group(mtwan_idx);
		}
		else {
			to_group = mtwan_get_second_group(mtwan_idx);
		}

		if (mtwan_group != to_group) {
			SCHED_DAEMON_DBG("[mtwan-scheduler] Change to group %d\n", to_group);
			mtwan_init_profile();
			mtwan_handle_group_change(mtwan_idx, to_group);
		}
	}

#ifdef USE_TIMERUTIL
	mod_timer(timer, PERIOD_30_SEC);
#endif
}
#endif //RTCONFIG_MULTIWAN_PROFILE

#ifndef RTCONFIG_AVOID_TZ_ENV
#ifdef USE_TIMERUTIL
static void task_timezone_checking(struct timer_entry *timer, void *data)
#else
static void task_timezone_checking(void)
#endif
{
	if(strcmp(nvram_safe_get("time_zone_x"), time_zone_t)){
		SCHED_DAEMON_DBG("update time zone from %s to %s\n", time_zone_t, nvram_safe_get("time_zone_x"));
		//logmessage("sched_daemon", "update time zone from %s to %s", time_zone_t, nvram_safe_get("time_zone_x"));
		strlcpy(time_zone_t, nvram_safe_get("time_zone_x"), sizeof(time_zone_t));
		setenv("TZ", nvram_safe_get("time_zone_x"), 1);
		tzset();
	}

#ifdef USE_TIMERUTIL
	mod_timer(timer, PERIOD_10_SEC);
#endif
}
#endif	/* RTCONFIG_AVOID_TZ_ENV */

#ifdef RTCONFIG_REBOOT_SCHEDULE
extern int timecheck_reboot(int sched_type, char *activeSchedule);
#ifdef USE_TIMERUTIL
static void task_reboot_schedule(struct timer_entry *timer, void *data)
#else
static void task_reboot_schedule(void)
#endif
{
	/* Reboot Schedule */
	char reboot_schedule[PATH_MAX];
	int reboot_schedule_type = 0;
	SCHED_DAEMON_DBG("[reboot-scheduler] checking...\n");
	// don't need to check reboot schedule when the system is booting up.
	if (uptime() > 300 && nvram_match("reboot_schedule_enable", "1"))
	{
		if (nvram_match("ntp_ready", "1"))
		{
			//SMTWTFSHHMM
			//XXXXXXXXXXX
#ifdef RTCONFIG_REBOOT_SCHEDULE_V2
			reboot_schedule_type = nvram_get_int("reboot_schedule_type");
#endif
			if (reboot_schedule_type == REBOOT_SCHED_TYPE_WEEKLY)
				snprintf(reboot_schedule, sizeof(reboot_schedule), "%s", nvram_safe_get("reboot_schedule"));
#ifdef RTCONFIG_REBOOT_SCHEDULE_V2
			else if (reboot_schedule_type == REBOOT_SCHED_TYPE_MONTHLY)
				snprintf(reboot_schedule, sizeof(reboot_schedule), "%s", nvram_safe_get("reboot_schedule_month"));
#endif
			SCHED_DAEMON_DBG("[reboot-scheduler] reboot_schedule_type=%d, reboot_schedule=%s\n", reboot_schedule_type, reboot_schedule);
			if (strlen(reboot_schedule) == 11 
#ifdef RTCONFIG_REBOOT_SCHEDULE_V2
					|| reboot_schedule_type == REBOOT_SCHED_TYPE_MONTHLY
#endif
				)
			{
				if (timecheck_reboot(reboot_schedule_type, reboot_schedule))
				{
					char reboot[sizeof("255")];
					char upgrade[sizeof("255")];

					memset(reboot, 0, sizeof("255"));
					memset(upgrade, 0, sizeof("255"));
					f_read_string("/tmp/reboot", reboot, sizeof(reboot));
					f_read_string("/tmp/upgrade", upgrade, sizeof(upgrade));

					if (atoi(reboot) || atoi(upgrade))
						return;

					nvram_set("sys_reboot_reason", "rbt_scheduler");
					logmessage("reboot scheduler", "[%s] The system is going down for reboot\n", __FUNCTION__);
					kill(1, SIGTERM);
				}
			}
		}
		else
			logmessage("reboot scheduler", "[%s] NTP sync error\n", __FUNCTION__);
	}

#ifdef USE_TIMERUTIL
	mod_timer(timer, PERIOD_30_SEC);
#endif
}
#endif // #ifdef RTCONFIG_REBOOT_SCHEDULE

#ifdef RTCONFIG_HNS
#ifdef USE_TIMERUTIL
static void task_hns_history(struct timer_entry *timer, void *data)
#else
static void task_hns_history(void)
#endif
{
	exe_hns_history();

#ifdef USE_TIMERUTIL
	mod_timer(timer, PERIOD_120_SEC);
#endif
}

#ifdef USE_TIMERUTIL
static void task_hns_protection(struct timer_entry *timer, void *data)
#else
static void task_hns_protection(void)
#endif
{
	exe_hns_protection();

#ifdef USE_TIMERUTIL
	mod_timer(timer, PERIOD_120_SEC);
#endif
}

#ifdef USE_TIMERUTIL
static void task_hns_signature(struct timer_entry *timer, void *data)
#else
static void task_hns_signature(void)
#endif
{
	hns_sig_update_flow();

#ifdef USE_TIMERUTIL
	mod_timer(timer, PERIOD_86400_SEC);
#endif
}

#ifdef USE_TIMERUTIL
static void task_hns_event_cc(struct timer_entry *timer, void *data)
#else
static void task_hns_event_cc(void)
#endif
{
#ifdef RTCONFIG_NOTIFICATION_CENTER
	hns_protect_event_cc();
#endif

#ifdef USE_TIMERUTIL
	mod_timer(timer, PERIOD_10_SEC);
#endif
}

#ifdef USE_TIMERUTIL
static void task_hns_alive(struct timer_entry *timer, void *data)
#else
static void task_hns_alive(void)
#endif
{
	check_hns_alive_service();

#ifdef USE_TIMERUTIL
	mod_timer(timer, PERIOD_30_SEC);
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
static void sched_daemon_sigusr2(struct timer_entry *timer, void *data)
#else
static void sched_daemon_sigusr2(int sig)
#endif
{
	SCHED_DAEMON_DBG("SIGUSR2 got!!!");
	reset_svc_status();
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
	FILE *fp;
	int pid = 0;

	/* write pid */
	if ((fp = fopen(PID_SCHED_DAEMON, "w")) != NULL)
	{
		pid = getpid();
		fprintf(fp, "%d", pid);
		fclose(fp);
	}

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
