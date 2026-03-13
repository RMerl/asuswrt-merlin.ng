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

char wlc_profile_list[1024];
char wlc_profile_retired[512];
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

#define PROFILE_MAX    8
#define PROFILE_FIELD_MAX 64
#define NVRAM_BUF_SIZE 1024

typedef struct {
	char band[2];
	char ssid[32];
	char key[PROFILE_FIELD_MAX];	// 64
	char auth[12];
	char crypto[12];
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

// wlc_profile_list: band<ssid<key<auth<crypto>band<ssid<key<auth<crypto>>
void parse_wlc_profile_list(const char* src)
{
	char buf[NVRAM_BUF_SIZE];
	char *entry_ptrs[PROFILE_MAX];
	char *fields[5];
	int i, entry_count, field_count;
	int field_len;
	char *p, *entry, *field, *next_entry, *next_field;

	strncpy(buf, src, NVRAM_BUF_SIZE-1);
	buf[NVRAM_BUF_SIZE-1] = 0;

	entry = buf;
	profile_num = 0;
	while (entry && (*entry) && profile_num < PROFILE_MAX) {
		next_entry = strchr(entry, '>');
		if (next_entry) {
			*next_entry = '\0';
			next_entry++;
		}

		p = entry;
		for (i = 0; i < 5; i++) {
			next_field = strchr(p, '<');
			if (next_field && i < 4) {
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

		if (i == 4 && fields[4] && strlen(fields[4]) > 0) {
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

// choose next entry w/ tried = 0
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

// band<ssid<key<auth<crypto>
void print_profile_entry(int idx)
{
	wlc_profile_t* p = &profile_list[idx];

	WLCDBG("entry[%d]: band=%s, ssid=%s, key=%s, auth=%s, crypto=%s, tried=%d\n", idx, p->band, p->ssid, p->key, p->auth, p->crypto, p->tried);
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
	int idx = 0;

	wlc_state = get_wpacli_status(nvram_get_int("wlc_band"));

	WLCDBG("%s:: wlc_state=%d, (%d)\n", __func__, wlc_state, wlcmon_retry);

	if (!wlc_state) {
		wlcmon_retry++;
		if (wlcmon_retry == wlcmon_retry_max) {
			idx = choose_next_profile_idx();
			profile_list[idx].tried = 1;
			update_retired_list_nvram();
			wlcmon_retry = 0;

			// TODO: apply profile, restart daemon, record chosen
			WLCDBG("%s:: Try conn w/ next profile...\n", __func__);
			print_profile_entry(idx);

			apply_new_wlc(idx);
		}
	} else {
		wlcmon_retry = 0;
	}
}

int
wlcmon_main(int argc, char *argv[])
{
        FILE *fp;
	int i;	

	strlcpy(wlc_profile_list, nvram_safe_get("wlc_profile_list"), sizeof(wlc_profile_list));
	strlcpy(wlc_profile_retired, nvram_safe_get("wlc_profile_retired"), sizeof(wlc_profile_retired));
	wlcmon_retry_max = nvram_get_int("wlcmon_retry_max");
	if (!wlcmon_retry_max)
		wlcmon_retry_max = 5;

	WLCDBG("wlcmon start...\nwlc_profile_list=%s\nwlc_profile_retired=%s\nwlcmon retry-conn-max=%d\n", wlc_profile_list, wlc_profile_retired, wlcmon_retry_max);

	parse_wlc_profile_list(wlc_profile_list);
	load_retried_status(wlc_profile_retired);

	for (i = 0; i < profile_num; i++) 
		print_profile_entry(i);

        if ((fp = fopen("/var/run/wlcmon.pid", "w")) != NULL)
        {
                fprintf(fp, "%d", getpid());
                fclose(fp);
        }

	wlcmon_init_sig();
	poll_t = nvram_get_int("wlcmon_poll_t") ? nvram_get_int("wlcmon_poll_t") : NORMAL_PERIOD;
        alarmtimer(poll_t, 0);

        while (1)
        {
                while(signals_noticed)
                        _watch_notices();

                pause();
        }

	return 0;
}


