/*
 * Copyright 2023, ASUSTeK Inc.
 * All Rights Reserved.
 *
 */

#include <rc.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <shutils.h>
#include <stdarg.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <limits.h>
#include <shared.h>
#include <syslog.h>
#include <bcmnvram.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <sys/reboot.h>
#include <sys/sysinfo.h>
#include <time.h>
#include <rc.h>

#define NORMAL_PERIOD	20	/* sec */

#define wlcmondbg(fmt, args...) do { FILE *fp = fopen("/dev/console", "w"); if (fp) { fprintf(fp, fmt, ## args); fclose(fp); } else fprintf(stderr, fmt, ## args); } while (0)

#define WLCDBG(fmt, arg...) \
                do { \
                        if (nvram_match("wlcdbg", "1")) \
                                wlcmondbg("wlcmon: "fmt, ##arg); \
                } while (0)

typedef unsigned char u8_t;
typedef unsigned long u32_t;

#define PROFILE_MAX    16
#define PROFILE_FIELD_MAX 64
#define NVRAM_BUF_SIZE 1024
#define LARGE_BUF_SIZE 4096
#define FLD_NUM		8

char wlc_profile_list[LARGE_BUF_SIZE];
char wlc_profile_retired[NVRAM_BUF_SIZE];
static int wlcmon_retry = 0;
int wlcmon_retry_max;

static sigset_t sigs_to_catch;
static unsigned int signals_noticed = 0;
static struct itimerval itv;
static unsigned long long diff;

static unsigned int poll_t = 0;
static void _note_sig(int signo) {
        signals_noticed |= 1<<signo;
}

void wlcmon(int sig);

static void *fn_acts[_NSIG];

static void chld_reap_local(int sig)
{
        chld_reap(sig);
}

typedef struct {
	char band[4];
	char ssid[32];
	char key[PROFILE_FIELD_MAX];	// 64
	char auth[12];
	char crypto[12];
	char pin[2];
	char desc[32];
	char ts[20];
	int tried;
} wlc_profile_t;

wlc_profile_t profile_list[PROFILE_MAX];
int profile_num = 0;

static void
alarmtimer(unsigned long sec, unsigned long usec)
{
        itv.it_value.tv_sec = sec;
        itv.it_value.tv_usec = usec;
        itv.it_interval = itv.it_value;
        setitimer(ITIMER_REAL, &itv, NULL);
}

static void catch_sig(int sig)
{
        if (sig == SIGUSR1)
        {
                printf("[wlcmon] tbd.\n");

                alarmtimer(poll_t, 0);
        }
}

typedef void (*mon_func)(int sig);
static mon_func _ep=NULL;

void _watch_notices()
{
        int sig;
        for (sig = 0; sig < (_NSIG - 1); sig++) {
                if(signals_noticed & 1<<sig  &&  (_ep=fn_acts[sig])) {
                        (*_ep)(sig);
                        signals_noticed &= ~(1<<sig);
                }
        }
}

void wlcmon_init_sig()
{
        int sig;

        for (sig = 0; sig < (_NSIG - 1); sig++) {
                if(sig == SIGCHLD
                || sig == SIGUSR1
                || sig == SIGALRM
                )
                signal(sig, _note_sig);
                fn_acts[sig] = NULL;
        }

        fn_acts[SIGCHLD] = chld_reap_local;
        fn_acts[SIGUSR1] = catch_sig;
        fn_acts[SIGALRM] = wlcmon;

        sigemptyset(&sigs_to_catch);
        sigaddset(&sigs_to_catch, SIGCHLD);
        sigaddset(&sigs_to_catch, SIGUSR1);
        sigaddset(&sigs_to_catch, SIGALRM);
}

// wlc_profile_list: band<ssid<key<auth<crypto<pin<desc<timestamp>
void parse_wlc_profile_list(const char* src)
{
	char buf[LARGE_BUF_SIZE];
	char *entry_ptrs[PROFILE_MAX];
	char *fields[FLD_NUM];
	int i, entry_count, field_count;
	int field_len;
	char *p, *entry, *field, *next_entry, *next_field;

	strncpy(buf, src, LARGE_BUF_SIZE-1);
	buf[LARGE_BUF_SIZE-1] = 0;

	entry = buf;
	profile_num = 0;
	while (entry && (*entry) && profile_num < PROFILE_MAX) {
		next_entry = strchr(entry, '>');
		if (next_entry) {
			*next_entry = '\0';
			next_entry++;
		}

		p = entry;
		for (i = 0; i < FLD_NUM; i++) {
			next_field = strchr(p, '<');
			if (next_field && i < FLD_NUM-1) {
				field_len = next_field - p;
				if (field_len < 0)
					field_len = 0;
				fields[i] = p;
				p = next_field + 1;
				*next_field = '\0';
			} else {
				fields[i] = p;
				break;
			}
		}

		if (i == FLD_NUM-1 && fields[FLD_NUM-1]) {
			strncpy(profile_list[profile_num].band,   fields[0], sizeof(profile_list[profile_num].band)-1);
			profile_list[profile_num].band[sizeof(profile_list[profile_num].band)-1] = 0;
			strncpy(profile_list[profile_num].ssid,   fields[1], sizeof(profile_list[profile_num].ssid)-1);
			profile_list[profile_num].ssid[sizeof(profile_list[profile_num].ssid)-1] = 0;
			strncpy(profile_list[profile_num].key,    fields[2], sizeof(profile_list[profile_num].key)-1);
			profile_list[profile_num].key[sizeof(profile_list[profile_num].key)-1] = 0;
			strncpy(profile_list[profile_num].auth,   fields[3], sizeof(profile_list[profile_num].auth)-1);
			profile_list[profile_num].auth[sizeof(profile_list[profile_num].auth)-1] = 0;
			strncpy(profile_list[profile_num].crypto, fields[4], sizeof(profile_list[profile_num].crypto)-1);
			profile_list[profile_num].crypto[sizeof(profile_list[profile_num].crypto)-1] = 0;

			strncpy(profile_list[profile_num].pin,   fields[5], sizeof(profile_list[profile_num].pin)-1);
			profile_list[profile_num].pin[sizeof(profile_list[profile_num].pin)-1] = 0;
			strncpy(profile_list[profile_num].desc,   fields[6], sizeof(profile_list[profile_num].desc)-1);
			profile_list[profile_num].desc[sizeof(profile_list[profile_num].desc)-1] = 0;
			strncpy(profile_list[profile_num].ts,   fields[7], sizeof(profile_list[profile_num].ts)-1);
			profile_list[profile_num].ts[sizeof(profile_list[profile_num].ts)-1] = 0;

			profile_list[profile_num].tried = 0;
			profile_num++;
		}
		entry = next_entry;
	}
}

// wlc_profile_retired:  ssid_1<ssid_2<ssid_3...
void load_retried_status(char* retried_list)
{
	char buf[NVRAM_BUF_SIZE];
	int i;
	char *tok = NULL;

	for (i = 0; i < profile_num; i++) {
		profile_list[i].tried = 0;
	}

	strncpy(buf, retried_list, sizeof(buf)-1);
	buf[sizeof(buf)-1] = 0;

	tok = strtok(buf, "<");

	while(tok) {
		for (i = 0; i < profile_num; i++) {
			if (!strcmp(profile_list[i].ssid, tok))
				profile_list[i].tried = 1;
		}
		tok = strtok(NULL, "<");
	}
}

void update_retired_list_nvram()
{
	char retried_str[NVRAM_BUF_SIZE] = "";
	int first = 1;
	int i;

	for(i = 0; i < profile_num; i++) {
		if (profile_list[i].tried) {
			if (!first) 
				strcat(retried_str, "<");
			strncat(retried_str, profile_list[i].ssid, sizeof(retried_str)-strlen(retried_str)-1);
			first = 0;
		}
	}
	nvram_set("wlc_profile_retired", retried_str);
}

void reset_all_tried()
{
	int i;

	for(i = 0; i < profile_num; i++) 
		profile_list[i].tried = 0;

	nvram_set("wlc_profile_retired", "");
}

// choose next entry w/ tried = 0, maybe ref timestamp
int choose_next_profile_idx()
{
	int i;

	for(i = 0; i < profile_num; i++) {
		if( profile_list[i].tried == 0)
			return i;
	}

	// to reset retried var when all entry's tried = 1
	reset_all_tried();
	return 0;
}

void serialize_wlc_profile_list(char *dst, size_t dst_size) {
	char entry[256];
	int i;

	dst[0] = '\0';
	for (i = 0; i < profile_num; ++i) {
		snprintf(entry, sizeof(entry), "%s<%s<%s<%s<%s<%s<%s<%s>",
		profile_list[i].band,
		profile_list[i].ssid,
		profile_list[i].key,
		profile_list[i].auth,
		profile_list[i].crypto,
		profile_list[i].pin,
		profile_list[i].desc,
		profile_list[i].ts);

		if (i > 0)
			strncat(dst, ">", dst_size - strlen(dst) - 1);

		strncat(dst, entry, dst_size - strlen(dst) - 1);
	}
}

int record_profile_ts(time_t ts)
{
	int i;
	char nvram_buf[LARGE_BUF_SIZE];

	for(i = 0; i < profile_num; i++) {
		if(nvram_match("wlc_ssid", profile_list[i].ssid)) {
			snprintf(profile_list[i].ts, sizeof(profile_list[i].ts), "%lld", (long long)ts);	
			
			print_profile_entry(i);
			serialize_wlc_profile_list(nvram_buf, sizeof(nvram_buf));
			nvram_set("wlc_profile_list", nvram_buf);
			WLCDBG("%s:: update wlc_profile_list:%s)\n", __func__, nvram_buf);
		}
	}

	return 0;
}

// band<ssid<key<auth<crypto>
void print_profile_entry(int idx)
{
	char buf[26];
	time_t timestamp;
	struct tm *tm_info;

	wlc_profile_t *p = &profile_list[idx];

	timestamp = (time_t)atoll(p->ts);
	tm_info = localtime(&timestamp);
	strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", tm_info);

	WLCDBG("entry[%d]: band=%s, ssid=%s, key=%s, auth=%s, crypto=%s, pin=%s, desc=%s, ts=(%s)%s, tried=%d\n", idx, p->band, p->ssid, p->key, p->auth, p->crypto, p->pin, p->desc, p->ts, buf, p->tried);
}

void apply_new_wlc(int idx)
{
	char tmp[100], prefix[]="wlXXXXXXX_", word[64], *next;
	char wl_ifnames[32] = { 0 };
	char wlif[16] = { 0 };
	int i = 0;

	snprintf(prefix, sizeof(prefix), "wlc_");

	nvram_set("wlc_band_prev", nvram_safe_get("wlc_band"));
	nvram_set(strcat_r(prefix, "band", tmp), profile_list[idx].band);
	nvram_set(strcat_r(prefix, "ssid", tmp), profile_list[idx].ssid);
	nvram_set(strcat_r(prefix, "wpa_psk", tmp), profile_list[idx].key);
	nvram_set(strcat_r(prefix, "auth_mode", tmp), profile_list[idx].auth);
	nvram_set(strcat_r(prefix, "crypto", tmp), profile_list[idx].crypto);

	syslog(LOG_NOTICE, "%s:timeout:: wlc try next profile [%s] ...\n", __func__, profile_list[idx].ssid);

	if (nvram_match("wlcmon_restart_net", "1"))
		notify_rc("restart_net");
	else {
		strlcpy(wl_ifnames, nvram_safe_get("wl_ifnames"), sizeof(wl_ifnames));
		foreach (word, wl_ifnames, next)
		{
			SKIP_ABSENT_BAND_AND_INC_UNIT(i);
			snprintf(prefix, sizeof(prefix), "wl%d_", i);
			strlcpy(wlif, nvram_safe_get(strcat_r(prefix, "ifname", tmp)), sizeof(wlif));
			generate_wl_para(wlif, i, -1);
			i++;
		}

		notify_rc("restart_wisp_serv");
	}
}

void wlcmon(int sig)
{
	int wlc_state = 0;
	static int counter = 0;
	int idx = 0;
	time_t last_timestamp = 0;

	wlc_state = get_wpacli_status(nvram_get_int("wlc_band"));

	WLCDBG("%s:: wlc_state=%d, (retry:%d)(c-counter:%d)\n", __func__, wlc_state, wlcmon_retry, counter);

	if (!wlc_state) {
		wlcmon_retry++;
		if (wlcmon_retry == wlcmon_retry_max && profile_num > 1) {
			idx = choose_next_profile_idx();
			profile_list[idx].tried = 1;
			update_retired_list_nvram();
			wlcmon_retry = 0;

			// TODO: apply profile, restart daemon, record chosen
			WLCDBG("%s:: try conn w/ next profile...\n", __func__);
			print_profile_entry(idx);

			apply_new_wlc(idx);
		}
	} else {
		wlcmon_retry = 0;

		if (wlc_state == 2) {
			counter++;
			if (counter == 3) {
				last_timestamp = time(NULL);
				WLCDBG("%s:: to record ts(%lld)...\n", __func__, (long long)last_timestamp);

				record_profile_ts(last_timestamp);
				counter = 0;
			}
		}
	}
}

int
wlcmon_main(int argc, char *argv[])
{
        FILE *fp;
	int i;	

	strlcpy(wlc_profile_list, nvram_safe_get("wlc_profile_list"), sizeof(wlc_profile_list));
	strlcpy(wlc_profile_retired, nvram_safe_get("wlc_profile_retired"), sizeof(wlc_profile_retired));
	wlcmon_retry_max = nvram_get_int("wlcmon_retry_max") ? : 5;

	parse_wlc_profile_list(wlc_profile_list);
	load_retried_status(wlc_profile_retired);

	WLCDBG("wlcmon start...\nwlc_profile_list=%s, profile_num=%d.\nwlc_profile_retired=%s\nwlcmon retry-conn-max=%d\n", wlc_profile_list, profile_num, wlc_profile_retired, wlcmon_retry_max);

	for (i = 0; i < profile_num; i++) 
		print_profile_entry(i);

        if ((fp = fopen("/var/run/wlcmon.pid", "w")) != NULL)
        {
                fprintf(fp, "%d", getpid());
                fclose(fp);
        }

	wlcmon_init_sig();
	poll_t = nvram_get_int("wlcmon_poll_t") ? : NORMAL_PERIOD;
        alarmtimer(poll_t, 0);

        while (1)
        {
                while(signals_noticed)
                        _watch_notices();

                pause();
        }

	return 0;
}


