/*
	hour_monitor.c for do something at hours
*/

#include <rc.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#ifdef RTCONFIG_BWDPI
#include <bwdpi_common.h>
#endif

// define function bit, you can define more functions as below
#define TRAFFIC_LIMITER         0x01
#define TRAFFIC_ANALYZER        0x02

#define MON_30_PERIOD           30    /* second */
#define MON_120_PERIOD          120   /* second */
#define MON_3600_PERIOD         3600  /* second */
static int debug = 0;
static int value = 0;
static int is_first = 1;
static int hm_alarm_status = 0;
static struct itimerval itv;
#define ANALYZER_TIMEOUT       30

static void alarmtimer(unsigned long sec, unsigned long usec)
{
	itv.it_value.tv_sec = sec;
	itv.it_value.tv_usec = usec;
	itv.it_interval = itv.it_value;
	setitimer(ITIMER_REAL, &itv, NULL);
}

void hm_traffic_analyzer_save()
{
	char *analyzer_exe_argv[] = {"TrafficAnalyzer", "-e", NULL};
	char *analyzer_size_argv[] = {"TrafficAnalyzer", "-d", BWDPI_ANA_DB_SIZE, NULL};

	_eval(analyzer_size_argv, NULL, ANALYZER_TIMEOUT, NULL);

	if (!f_exists(DEVNODE) || !f_exists("/dev/idpfw")) {
		_dprintf("%s : dpi engine doesn't exist, not to save any database\n", __FUNCTION__);
		logmessage("hour monitor", "dpi engine doesn't exist");
		return;
	}

	_eval(analyzer_exe_argv, NULL, ANALYZER_TIMEOUT, NULL);
	//logmessage("hour monitor", "store traffic analyzer data"); // DEBUG
}

void hm_traffic_limiter_save()
{
	if (nvram_get_int("hour_monitor_debug"))
		debug = 1;
	else
		debug = 0;

	if (debug) dbg("%s : traffic_limiter is saving ... \n", __FUNCTION__);

	eval("traffic_limiter", "-w");
}

int hour_monitor_function_check()
{
	debug = nvram_get_int("hour_monitor_debug");

	// intial global variable
	value = 0;

	// traffic limiter
	if (nvram_get_int("tl_enable"))
		value |= TRAFFIC_LIMITER;

	// traffic analyzer
	if (nvram_get_int("bwdpi_db_enable"))
		value |= TRAFFIC_ANALYZER;

	if (debug)
	{
		logmessage("hour monitor", "value = %d(0x%x)", value, value);
	}

	return value;
}

static void hour_monitor_call_fucntion()
{
	// check function enable or not
	if (!hour_monitor_function_check())
		return;

	if ((value & TRAFFIC_LIMITER) != 0)
		hm_traffic_limiter_save();

	if ((value & TRAFFIC_ANALYZER) != 0)
		hm_traffic_analyzer_save();
}

static void hour_monitor_save_database()
{
	/* use for save database only */

	// check function enable or not
	if (!hour_monitor_function_check())
		return;

	if ((value & TRAFFIC_LIMITER) != 0)
		hm_traffic_limiter_save();

	if ((value & TRAFFIC_ANALYZER) != 0)
		hm_traffic_analyzer_save();
}

static void catch_sig_exit(int sig)
{
	alarmtimer(0, 0);
	hour_monitor_save_database(); /* use for save database only */
	remove("/var/run/hour_monitor.pid");
	exit(0);
}

int diff_timer()
{
	int diff_sec = MON_30_PERIOD;
	if (nvram_get_int("ntp_ready"))
	{
		struct tm local;
		time_t now;

		hm_alarm_status = 1;

		time(&now);
		localtime_r(&now, &local);
		if (debug) dbg("%s: %d-%d-%d, %d:%d:%d\n", __FUNCTION__,
			local.tm_year+1900, local.tm_mon+1, local.tm_mday, local.tm_hour, local.tm_min, local.tm_sec);
		if (debug) logmessage("hour_monitor", "%d-%d-%d, %d:%d:%d\n", 
			local.tm_year+1900, local.tm_mon+1, local.tm_mday, local.tm_hour, local.tm_min, local.tm_sec);

		/* every hour */
		if ((local.tm_min != 0) || (local.tm_sec != 0)) {
			diff_sec = 3600 - (local.tm_min * 60 + local.tm_sec);

			// delay 15 sec to wait for DST NTP sync done
			if (strstr(nvram_safe_get("time_zone_x"), "DST")) diff_sec += 15;
		}
		else {
			diff_sec = MON_3600_PERIOD;
		}

		if (is_first) {
			diff_sec = MON_30_PERIOD;
			is_first = 0;
		}
	}
	else
	{
		if (debug) _dprintf("%s: ntp is not syn ... \n", __FUNCTION__);
		hm_alarm_status = 0;
		diff_sec = MON_120_PERIOD;
	}

	if (debug) {
		_dprintf("%s: diff_sec=%d\n", __FUNCTION__, diff_sec);
		logmessage("hour monitor", "diff_sec=%d", diff_sec);
	}

	return diff_sec;
}

static void catch_sig_alrm(int sig)
{
	int timer_val = diff_timer();

	if (hm_alarm_status == 1) {
		hour_monitor_call_fucntion();
	}
	else if (hm_alarm_status == 0) {
		logmessage("hour monitor", "ntp sync fail, will retry after %d sec", timer_val);
	}

	alarmtimer(timer_val, 0);
	TstampToNvram("tstamp_hour_monitor");
}

int hour_monitor_main(int argc, char **argv)
{
	FILE *fp;
	sigset_t sigs_to_catch;

	debug = nvram_get_int("hour_monitor_debug");

	/* starting message */
	if (debug) _dprintf("%s: daemong is starting ... \n", __FUNCTION__);
	logmessage("hour monitor", "daemon is starting");

	/* check need to enable monitor fucntion or not */
	if (!hour_monitor_function_check()) {
		if (debug) _dprintf("%s: terminate ... \n", __FUNCTION__);
		logmessage("hour monitor", "daemon terminates");
		exit(0);
	}

	/* write pid */
	if ((fp = fopen("/var/run/hour_monitor.pid", "w")) != NULL)
	{
		fprintf(fp, "%d", getpid());
		fclose(fp);
	}

	/* set the signal handler */
	sigemptyset(&sigs_to_catch);
	sigaddset(&sigs_to_catch, SIGTERM);
	sigaddset(&sigs_to_catch, SIGALRM);
	sigprocmask(SIG_UNBLOCK, &sigs_to_catch, NULL);

	signal(SIGTERM, catch_sig_exit);
	signal(SIGALRM, catch_sig_alrm);

	if (debug) {
		_dprintf("%s: ntp_ready=%d, DST=%s\n", __FUNCTION__, nvram_get_int("ntp_ready"), nvram_safe_get("time_zone_x"));
		logmessage("hour monitor", "ntp_ready=%d, DST=%s", nvram_get_int("ntp_ready"), nvram_safe_get("time_zone_x"));
	}

	// alarm 30 secs
	alarmtimer(MON_30_PERIOD, 0);

	while (1)
	{
		pause();
	}

	return 0;
}
