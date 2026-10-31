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
#define LARGE_BUF_SIZE 4096
#define FLD_NUM		8

/* RSSI cache: only SSIDs present in profile_list are stored */
#define RSSI_CACHE_MAX	PROFILE_MAX
#define RSSI_INVALID	INT_MIN
#define RSSI_MIN_VALID	(-150)	/* sanity lower bound for valid RSSI readings (dBm) */
/* Score offsets for profile priority */
#define SCORE_5G_BONUS	1000

typedef struct {
	char ssid[32];
	int  rssi;
} rssi_cache_entry_t;

static rssi_cache_entry_t rssi_cache[RSSI_CACHE_MAX];
static int rssi_cache_count = 0;

char wlc_profile_list[LARGE_BUF_SIZE];
static int wlcmon_retry = 0;
int wlcmon_retry_max;

/*
 * Fallback/restore debounce state machine.
 *
 * A WAN-band switch (fallback 5G→2G or restore 2G→5G) is only performed
 * after the trigger condition holds for WLCMON_CONFIRM_COUNT consecutive
 * wlcmon ticks.  This prevents transient link glitches (e.g. the brief
 * radio down/up caused by wpa_supplicant restart) from inducing rapid
 * fallback→restore→fallback oscillation.
 *
 * Additionally, MIN_SWITCH_INTERVAL_SECS enforces a minimum quiet period
 * between any two switches so that a freshly restarted wpa_supplicant
 * cannot immediately trigger the opposite switch before it has had time
 * to stabilise.
 *
 * Both confirm counters are cross-reset whenever one direction fires so
 * stale counts from before the switch cannot carry over.
 */
#define WLCMON_CONFIRM_COUNT     3    /* consecutive ticks required before switching */
#define MIN_SWITCH_INTERVAL_SECS 60   /* minimum seconds between any two WAN-band switches */
/* Grace period after (re)selecting a 5G-primary dual-band profile during which
 * we do NOT fall back to the 2G companion, even if 5G has not associated yet.
 * A dual-band profile brings up wpa_supplicant on BOTH radios; 2G (low channel)
 * usually associates faster than 5G (scan + higher channel), so without this
 * grace wlcmon would prematurely fall back to 2G and then restore to 5G once it
 * came up — a wasteful 2G-first-then-5G ping-pong (two restart_wisp_serv).
 * Give the fresh 5G association this long to complete first.  nvram-tunable via
 * "wlcmon_5g_grace" (seconds); 0/unset -> default. */
#define WLCMON_5G_GRACE_SECS     30

static int    fallback_confirm = 0;  /* ticks where (5G WAN, 5G down, 2G up) holds */
static int    restore_confirm  = 0;  /* ticks where (2G WAN, 5G up) holds */
static long   last_switch_ts   = 0;  /* monotonic uptime (seconds) of the most recent switch */
static int    profiles_tried   = 0;  /* profile switches since last successful connect */

/* Return monotonic uptime in seconds (immune to wall-clock adjustments). */
static long uptime_secs(void)
{
	struct timespec ts;
	if (clock_gettime(CLOCK_MONOTONIC, &ts) < 0)
		return 0;
	return (long)ts.tv_sec;
}

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

			profile_num++;
		}
		entry = next_entry;
	}
}

/*
 * Return 1 if the wl unit (wl<unit>_nband) is a 5 GHz radio, 0 otherwise.
 * nband "1" = 5 GHz, nband "2" = 2.4 GHz (Broadcom convention).
 */
static int is_5g_unit(int unit)
{
	char tmp[16], prefix[16];
	snprintf(prefix, sizeof(prefix), "wl%d_", unit);
	return nvram_pf_match(prefix, "nband", "1");
}

/*
 * Parse scan-result text output (from wl escanresults or acs_cli2 dump
 * scanresults) from file pointer fp into rssi_cache[].
 * Only SSIDs that appear in profile_list are recorded; all others are
 * ignored so that a crowded scan-result list cannot push profile SSIDs
 * out of the fixed-size cache.
 * Returns the number of new SSID entries added to the cache.
 * Format expected per BSS entry:
 *   SSID: "network_name"
 *   ...
 *   RSSI: -70 dBm  [Channel: N]
 */
static int ssid_in_profile_list(const char *ssid)
{
	int i;
	for (i = 0; i < profile_num; i++) {
		if (!strcmp(profile_list[i].ssid, ssid))
			return 1;
	}
	return 0;
}

static int parse_scan_output(FILE *fp)
{
	char line[256];
	char cur_ssid[32] = { 0 };
	int  i, added = 0;

	while (fgets(line, sizeof(line), fp)) {
		char *p;

		/* Parse SSID line: SSID: "network_name" */
		if (strncmp(line, "SSID:", 5) == 0) {
			char *start = strchr(line, '"');
			char *end   = start ? strchr(start + 1, '"') : NULL;
			if (start && end && (end - start - 1) <= (int)(sizeof(cur_ssid) - 1)) {
				memset(cur_ssid, 0, sizeof(cur_ssid));
				memcpy(cur_ssid, start + 1, end - start - 1);
			} else {
				cur_ssid[0] = '\0';
			}
			continue;
		}

		/* Parse RSSI line: RSSI: -70 dBm ... */
		p = strstr(line, "RSSI:");
		if (p && cur_ssid[0]) {
			int rssi = RSSI_INVALID;
			if (sscanf(p, "RSSI: %d dBm", &rssi) == 1) {
				/* Skip SSIDs not in the profile list */
				if (!ssid_in_profile_list(cur_ssid)) {
					cur_ssid[0] = '\0';
					continue;
				}
				/* Update or insert into cache (keep strongest signal) */
				int found = 0;
				for (i = 0; i < rssi_cache_count; i++) {
					if (!strcmp(rssi_cache[i].ssid, cur_ssid)) {
						if (rssi > rssi_cache[i].rssi)
							rssi_cache[i].rssi = rssi;
						found = 1;
						break;
					}
				}
				if (!found && rssi_cache_count < RSSI_CACHE_MAX) {
					strncpy(rssi_cache[rssi_cache_count].ssid, cur_ssid,
					        sizeof(rssi_cache[rssi_cache_count].ssid) - 1);
					rssi_cache[rssi_cache_count].ssid[sizeof(rssi_cache[rssi_cache_count].ssid) - 1] = '\0';
					rssi_cache[rssi_cache_count].rssi = rssi;
					rssi_cache_count++;
					added++;
				}
			}
			cur_ssid[0] = '\0';
		}
	}
	return added;
}

/*
 * Refresh the RSSI cache for one interface.
 * Tries acs_cli2 -i <ifname> dump scanresults first (uses acsd's cached
 * scan data); falls back to wl -i <ifname> escanresults if acs_cli2
 * produces no SSID entries (e.g. when acsd info-level logging is off).
 */
static void refresh_rssi_cache_for_if(const char *ifname)
{
	FILE *fp;
	char cmd[64];
	int  added = 0;

	snprintf(cmd, sizeof(cmd), "acs_cli2 -i %s dump scanresults 2>/dev/null", ifname);
	fp = popen(cmd, "r");
	if (fp) {
		added = parse_scan_output(fp);
		pclose(fp);
	}

	if (!added) {
		/* Fall back to wl escanresults which always writes to stdout */
		snprintf(cmd, sizeof(cmd), "wl -i %s escanresults 2>/dev/null", ifname);
		fp = popen(cmd, "r");
		if (fp) {
			parse_scan_output(fp);
			pclose(fp);
		}
	}
}

/*
 * Refresh RSSI cache for all wl interfaces listed in wl_ifnames.
 */
static void refresh_rssi_cache(void)
{
	char wl_ifnames[64] = { 0 };
	char tmp[16], prefix[16], word[64], wlif[16], *next;
	int  unit = 0;

	rssi_cache_count = 0;
	strlcpy(wl_ifnames, nvram_safe_get("wl_ifnames"), sizeof(wl_ifnames));
	foreach (word, wl_ifnames, next) {
		SKIP_ABSENT_BAND_AND_INC_UNIT(unit);
		snprintf(prefix, sizeof(prefix), "wl%d_", unit);
		strlcpy(wlif, nvram_safe_get(strcat_r(prefix, "ifname", tmp)), sizeof(wlif));
		if (*wlif)
			refresh_rssi_cache_for_if(wlif);
		unit++;
	}
}

/*
 * Look up the cached RSSI for a given SSID.
 * Returns the RSSI value, or RSSI_INVALID if not found.
 */
static int get_rssi_for_ssid(const char *ssid)
{
	int i;
	for (i = 0; i < rssi_cache_count; i++) {
		if (!strcmp(rssi_cache[i].ssid, ssid))
			return rssi_cache[i].rssi;
	}
	return RSSI_INVALID;
}

/*
 * Compute a priority score for a profile entry.
 *   - 5 GHz radio gets a SCORE_5G_BONUS advantage over 2.4 GHz.
 *   - RSSI (if available) is added directly (e.g. -60 dBm contributes +40
 *     relative to -100 dBm baseline which contributes 0).
 */
static int score_profile(int idx)
{
	int score = 0;
	int unit  = atoi(profile_list[idx].band);
	int rssi;

	if (is_5g_unit(unit))
		score += SCORE_5G_BONUS;

	rssi = get_rssi_for_ssid(profile_list[idx].ssid);
	if (rssi != RSSI_INVALID && rssi > RSSI_MIN_VALID && rssi <= 0)
		score += (rssi + 100);	/* -100 dBm -> 0, -60 dBm -> 40 */

	return score;
}

/*
 * Choose the next profile to try based on RSSI score.
 * Skips profiles whose SSID matches the currently active SSID so that a
 * different AP is tried first.  If no other SSID exists, falls back to
 * the highest-scored profile overall.
 *   1. 5 GHz profiles are preferred over 2.4 GHz profiles (SCORE_5G_BONUS).
 *   2. Among profiles of the same band, higher RSSI wins.
 */
int choose_next_profile_idx(void)
{
	int i, best_idx = -1, best_score = INT_MIN;
	const char *cur_ssid = nvram_safe_get("wlc_ssid");

	/* Refresh RSSI cache once before scoring */
	refresh_rssi_cache();

	/* Prefer a profile with a different SSID than the current one */
	for (i = 0; i < profile_num; i++) {
		if (*cur_ssid && !strcmp(profile_list[i].ssid, cur_ssid))
			continue;
		int score = score_profile(i);
		if (best_idx < 0 || score > best_score) {
			best_idx  = i;
			best_score = score;
		}
	}

	if (best_idx < 0) {
		/* No other SSID available — pick the highest-scored overall */
		for (i = 0; i < profile_num; i++) {
			int score = score_profile(i);
			if (best_idx < 0 || score > best_score) {
				best_idx  = i;
				best_score = score;
			}
		}
		if (best_idx < 0)
			best_idx = 0;
	}

	/* Dump every profile so the selection logic is easy to audit in syslog */
	for (i = 0; i < profile_num; i++) {
		int s = score_profile(i);
		int rssi = get_rssi_for_ssid(profile_list[i].ssid);

		syslog(LOG_NOTICE,
		       "wlcmon: %s: [%d]%s band=%-2s ssid=%-32s rssi=%d score=%d\n",
		       __func__, i, (i == best_idx) ? "*" : " ",
		       profile_list[i].band, profile_list[i].ssid,
		       rssi, s);
	}

	syslog(LOG_NOTICE,"wlcmon: %s:: best_idx=%d ssid=%s score=%d\n", __func__,
	       best_idx, profile_list[best_idx].ssid, best_score);

	return best_idx;
}

/*
 * Find the index of a profile that matches the given SSID on the given
 * band unit.  Returns -1 if not found.
 */
static int find_profile_idx_by_ssid_and_unit(const char *ssid, int unit)
{
	int i;
	char unit_str[4];

	snprintf(unit_str, sizeof(unit_str), "%d", unit);
	for (i = 0; i < profile_num; i++) {
		if (!strcmp(profile_list[i].ssid, ssid) &&
		    !strcmp(profile_list[i].band, unit_str))
			return i;
	}
	return -1;
}

/*
 * Find the 2G unit number paired with a 5G profile's SSID, i.e. locate
 * a 2G profile entry whose SSID equals the given SSID.
 * Returns the 2G unit number, or -1 if no such profile exists.
 */
static int find_2g_unit_for_ssid(const char *ssid)
{
	int i, unit;

	for (i = 0; i < profile_num; i++) {
		if (strcmp(profile_list[i].ssid, ssid))
			continue;
		unit = atoi(profile_list[i].band);
		if (!is_5g_unit(unit))
			return unit;
	}
	return -1;
}

/*
 * Find the 5G unit number paired with the given SSID, i.e. locate
 * a 5G profile entry whose SSID equals the given SSID.
 * Returns the 5G unit number, or -1 if no such profile exists.
 */
static int find_5g_unit_for_ssid(const char *ssid)
{
	int i, unit;

	for (i = 0; i < profile_num; i++) {
		if (strcmp(profile_list[i].ssid, ssid))
			continue;
		unit = atoi(profile_list[i].band);
		if (is_5g_unit(unit))
			return unit;
	}
	return -1;
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

	WLCDBG("entry[%d]: band=%s, ssid=%s, key=%s, auth=%s, crypto=%s, pin=%s, desc=%s, ts=(%s)%s\n", idx, p->band, p->ssid, p->key, p->auth, p->crypto, p->pin, p->desc, p->ts, buf);
}

#if defined(RTCONFIG_WISP) && defined(RTCONFIG_DUALWAN)
/* Return 1 when dualwan is active and the WiFi WAN unit is NOT the primary
 * (i.e. wanduck has failed over to wan1/eth0/USB).  Returns 0 in single-WAN
 * WISP or when the WiFi unit is still the active primary. */
static int wisp_wan1_active(void)
{
	int u;
	for (u = WAN_UNIT_FIRST; u < WAN_UNIT_MAX; u++) {
		int dt = get_dualwan_by_unit(u);
		/* The WiFi-STA WAN unit is the radio-hijacked 'wan' token (IF_WAN,
		 * reconfig_manual_wan_ifnames sets wan_ifname=get_wisp_ifname()) or an
		 * explicit 2g/5g token.  With the "wan wan2" dual-WAN layout the WiFi
		 * slot is IF_WAN, so it must be recognised here or wlcmon would never
		 * see eth1 as the active primary and would keep firing
		 * restart_wisp_serv against wanduck's routing decision. */
		if (dt == WANS_DUALWAN_IF_2G || dt == WANS_DUALWAN_IF_5G
#if defined(RTCONFIG_HND_WISP_DUALWAN)
		    || dt == WANS_DUALWAN_IF_WAN
#endif
		   )
			return wan_primary_ifunit() != u;
	}
	return 0;
}
#endif

void apply_new_wlc(int idx)
{
	char tmp[100], prefix[]="wlXXXXXXX_", word[64], *next;
	char wl_ifnames[32] = { 0 };
	char wlif[16] = { 0 };
	int  i = 0;
	int  sel_unit, dual_2g_unit, dual_5g_unit;

	sel_unit = atoi(profile_list[idx].band);

	/*
	 * Dual-band 5G-first: if the selected entry is 2G but the SAME SSID also
	 * exists on 5G, promote the 5G entry to primary so we connect 5G FIRST.
	 * This must live here (not only in score_profile) because several entry
	 * points bypass the scorer and would otherwise start on 2G:
	 *   - wlc_restart_profile -> apply_new_wlc(0) (resets to profile[0], which
	 *     is 2G when the WebUI lists the 2G band first — the 070807.1 case),
	 *   - first-boot apply_new_wlc(0).
	 * Starting on 2G then having the restore branch move to 5G is the wasteful
	 * 2G-first-then-5G ping-pong (two restart_wisp_serv).  Promoting here makes
	 * the primary 5G from the outset; the old 2G entry becomes the wlcp_band
	 * companion via the 5G branch below.  (The WLCMON_5G_GRACE_SECS fallback
	 * grace then protects 5G's slower initial association.)
	 */
	if (!is_5g_unit(sel_unit)) {
		int p5g_unit = find_5g_unit_for_ssid(profile_list[idx].ssid);
		if (p5g_unit >= 0) {
			int p5g = find_profile_idx_by_ssid_and_unit(profile_list[idx].ssid, p5g_unit);
			if (p5g >= 0) {
				syslog(LOG_NOTICE,
				       "wlcmon: %s: dual-band 5G-first: promote [%s] 2G unit%d (idx %d) -> 5G unit%d (idx %d)\n",
				       __func__, profile_list[idx].ssid, sel_unit, idx, p5g_unit, p5g);
				idx = p5g;
				sel_unit = p5g_unit;
			}
		}
	}

	snprintf(prefix, sizeof(prefix), "wlc_");

	nvram_set("wlc_band_prev", nvram_safe_get("wlc_band"));
	nvram_set(strcat_r(prefix, "band", tmp), profile_list[idx].band);
	nvram_set(strcat_r(prefix, "ssid", tmp), profile_list[idx].ssid);
	nvram_set(strcat_r(prefix, "wpa_psk", tmp), profile_list[idx].key);
	nvram_set(strcat_r(prefix, "auth_mode", tmp), profile_list[idx].auth);
	nvram_set(strcat_r(prefix, "crypto", tmp), profile_list[idx].crypto);

	/*
	 * Dual-band support: when the selected profile is 5 GHz and the same
	 * SSID also appears with a 2 GHz band entry in the profile list,
	 * configure the 2 GHz wlif with the same credentials so both radios
	 * associate with the upstream AP.  The 5 GHz interface is preferred as
	 * the designated WAN (wlc_band points to the 5 GHz unit); record the
	 * 2 GHz companion in wlcp_band so wisp_serv can start wpa_supplicant
	 * on it without relying on lan_ifnames.
	 */
	dual_2g_unit = -1;
	if (is_5g_unit(sel_unit)) {
		dual_2g_unit = find_2g_unit_for_ssid(profile_list[idx].ssid);
		if (dual_2g_unit >= 0) {
			int p2g = find_profile_idx_by_ssid_and_unit(profile_list[idx].ssid, dual_2g_unit);
			if (p2g >= 0) {
				char dual_prefix[16], wl2g_prefix[16], sel_prefix[16];
				char wl_if[8], lan_ifnames[512];

				/* Set wlcN_ credentials for the 2G unit */
				snprintf(dual_prefix, sizeof(dual_prefix), "wlc%d_", dual_2g_unit);
				nvram_set(strcat_r(dual_prefix, "ssid",      tmp), profile_list[p2g].ssid);
				nvram_set(strcat_r(dual_prefix, "wpa_psk",   tmp), profile_list[p2g].key);
				nvram_set(strcat_r(dual_prefix, "auth_mode", tmp), profile_list[p2g].auth);
				nvram_set(strcat_r(dual_prefix, "crypto",    tmp), profile_list[p2g].crypto);

				/*
				 * Also write wlN_ nvrams so wpa_supplicant config generation
				 * picks up the right credentials.  generate_wl_para() for the
				 * non-WISP unit does not propagate wlcN_ -> wlN_ in WISP
				 * router mode, so we must set them explicitly here.
				 */
				snprintf(wl2g_prefix, sizeof(wl2g_prefix), "wl%d_", dual_2g_unit);
				nvram_set(strcat_r(wl2g_prefix, "ssid",        tmp), profile_list[p2g].ssid);
				nvram_set(strcat_r(wl2g_prefix, "auth_mode_x", tmp), profile_list[p2g].auth);
				nvram_set(strcat_r(wl2g_prefix, "akm",         tmp), profile_list[p2g].auth);
				nvram_set(strcat_r(wl2g_prefix, "wpa_psk",     tmp), profile_list[p2g].key);
				nvram_set(strcat_r(wl2g_prefix, "crypto",      tmp), profile_list[p2g].crypto);

				/* Set wlc<sel_unit>_ for the 5G unit for consistency */
				snprintf(sel_prefix, sizeof(sel_prefix), "wlc%d_", sel_unit);
				nvram_set(strcat_r(sel_prefix, "ssid",      tmp), profile_list[idx].ssid);
				nvram_set(strcat_r(sel_prefix, "wpa_psk",   tmp), profile_list[idx].key);
				nvram_set(strcat_r(sel_prefix, "auth_mode", tmp), profile_list[idx].auth);
				nvram_set(strcat_r(sel_prefix, "crypto",    tmp), profile_list[idx].crypto);

				/* Record the companion 2G band for wisp_serv */
				nvram_set("wlcp_band", profile_list[p2g].band);

				/*
				 * Remove both the primary (5G) and companion (2G) interfaces
				 * from lan_ifnames — they are STA interfaces and must not be
				 * bridged into the LAN.  wisp_serv START will start
				 * wpa_supplicant on them using wlc_band + wlcp_band directly.
				 */
				strlcpy(lan_ifnames, nvram_safe_get("lan_ifnames"), sizeof(lan_ifnames));
				snprintf(wl_if, sizeof(wl_if), "wl%d", sel_unit);
				remove_from_list(wl_if, lan_ifnames, sizeof(lan_ifnames));
				snprintf(wl_if, sizeof(wl_if), "wl%d", dual_2g_unit);
				remove_from_list(wl_if, lan_ifnames, sizeof(lan_ifnames));
				nvram_set("lan_ifnames", lan_ifnames);

				syslog(LOG_NOTICE,
				       "wlcmon: %s: dual-band: 5G unit%d + 2G unit%d for [%s], wlcp_band=%d\n",
				       __func__, sel_unit, dual_2g_unit, profile_list[p2g].ssid, dual_2g_unit);
			}
		}
	}

	if (dual_2g_unit < 0 && is_5g_unit(sel_unit)) {
		/*
		 * 5G single-band: clear wlcp_band and remove wl<sel_unit>
		 * from lan_ifnames.
		 */
		char rm_if[8], lan_ifnames[512];
		nvram_set("wlcp_band", "");
		snprintf(rm_if, sizeof(rm_if), "wl%d", sel_unit);
		strlcpy(lan_ifnames, nvram_safe_get("lan_ifnames"), sizeof(lan_ifnames));
		if (remove_from_list(rm_if, lan_ifnames, sizeof(lan_ifnames)) == 0) {
			nvram_set("lan_ifnames", lan_ifnames);
			syslog(LOG_NOTICE,
			       "wlcmon: %s: single-band 5G: removed %s from lan_ifnames\n",
			       __func__, rm_if);
		}
	}

	/*
	 * Dual-band support (symmetric case): when the selected profile is
	 * 2 GHz and the same SSID also appears with a 5 GHz band entry,
	 * configure the 5 GHz wlif with the same credentials.  The 2 GHz
	 * interface acts as WAN (wlc_band points to the 2 GHz unit).
	 */
	dual_5g_unit = -1;
	if (!is_5g_unit(sel_unit)) {
		dual_5g_unit = find_5g_unit_for_ssid(profile_list[idx].ssid);
		if (dual_5g_unit >= 0) {
			int p5g = find_profile_idx_by_ssid_and_unit(profile_list[idx].ssid, dual_5g_unit);
			if (p5g >= 0) {
				char dual_prefix[16], wl5g_prefix[16], sel_prefix[16];
				char wl_if[8], lan_ifnames[512];

				/* Set wlcN_ credentials for the 5G unit */
				snprintf(dual_prefix, sizeof(dual_prefix), "wlc%d_", dual_5g_unit);
				nvram_set(strcat_r(dual_prefix, "ssid",      tmp), profile_list[p5g].ssid);
				nvram_set(strcat_r(dual_prefix, "wpa_psk",   tmp), profile_list[p5g].key);
				nvram_set(strcat_r(dual_prefix, "auth_mode", tmp), profile_list[p5g].auth);
				nvram_set(strcat_r(dual_prefix, "crypto",    tmp), profile_list[p5g].crypto);

				/*
				 * Also write wlN_ nvrams so wpa_supplicant config generation
				 * picks up the right credentials.
				 */
				snprintf(wl5g_prefix, sizeof(wl5g_prefix), "wl%d_", dual_5g_unit);
				nvram_set(strcat_r(wl5g_prefix, "ssid",        tmp), profile_list[p5g].ssid);
				nvram_set(strcat_r(wl5g_prefix, "auth_mode_x", tmp), profile_list[p5g].auth);
				nvram_set(strcat_r(wl5g_prefix, "akm",         tmp), profile_list[p5g].auth);
				nvram_set(strcat_r(wl5g_prefix, "wpa_psk",     tmp), profile_list[p5g].key);
				nvram_set(strcat_r(wl5g_prefix, "crypto",      tmp), profile_list[p5g].crypto);

				/* Set wlc<sel_unit>_ for the 2G unit for consistency */
				snprintf(sel_prefix, sizeof(sel_prefix), "wlc%d_", sel_unit);
				nvram_set(strcat_r(sel_prefix, "ssid",      tmp), profile_list[idx].ssid);
				nvram_set(strcat_r(sel_prefix, "wpa_psk",   tmp), profile_list[idx].key);
				nvram_set(strcat_r(sel_prefix, "auth_mode", tmp), profile_list[idx].auth);
				nvram_set(strcat_r(sel_prefix, "crypto",    tmp), profile_list[idx].crypto);

				/* Record the companion 5G band for wisp_serv */
				nvram_set("wlcp_band", profile_list[p5g].band);

				/*
				 * Remove both the primary (2G) and companion (5G) interfaces
				 * from lan_ifnames.
				 */
				strlcpy(lan_ifnames, nvram_safe_get("lan_ifnames"), sizeof(lan_ifnames));
				snprintf(wl_if, sizeof(wl_if), "wl%d", sel_unit);
				remove_from_list(wl_if, lan_ifnames, sizeof(lan_ifnames));
				snprintf(wl_if, sizeof(wl_if), "wl%d", dual_5g_unit);
				remove_from_list(wl_if, lan_ifnames, sizeof(lan_ifnames));
				nvram_set("lan_ifnames", lan_ifnames);

				syslog(LOG_NOTICE,
				       "wlcmon: %s: dual-band: 2G unit%d + 5G unit%d for [%s], wlcp_band=%d\n",
				       __func__, sel_unit, dual_5g_unit, profile_list[p5g].ssid, dual_5g_unit);
			}
		}
	}

	if (dual_5g_unit < 0 && !is_5g_unit(sel_unit)) {
		/*
		 * 2G single-band: clear wlcp_band and remove wl<sel_unit>
		 * from lan_ifnames.
		 */
		char rm_if[8], lan_ifnames[512];
		nvram_set("wlcp_band", "");
		snprintf(rm_if, sizeof(rm_if), "wl%d", sel_unit);
		strlcpy(lan_ifnames, nvram_safe_get("lan_ifnames"), sizeof(lan_ifnames));
		if (remove_from_list(rm_if, lan_ifnames, sizeof(lan_ifnames)) == 0) {
			nvram_set("lan_ifnames", lan_ifnames);
			syslog(LOG_NOTICE,
			       "wlcmon: %s: single-band 2G: removed %s from lan_ifnames\n",
			       __func__, rm_if);
		}
	}

	syslog(LOG_NOTICE, "wlcmon: %s:timeout:: wlc try next profile [%s] band=%s...\n",
	       __func__, profile_list[idx].ssid, profile_list[idx].band);

#if defined(RTCONFIG_WISP) && defined(RTCONFIG_DUALWAN)
	if (wisp_mode() && wisp_wan1_active()) {
		syslog(LOG_NOTICE,
		       "wlcmon: apply_new_wlc: wan1 active (wifi not primary), skip restart\n");
		return;
	}
#endif

	if (nvram_match("wlcmon_restart_net", "1")) {
		notify_rc("restart_net");
	}
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

	/* Stamp the switch time so the fallback grace (WLCMON_5G_GRACE_SECS) and
	 * the restore cooldown (MIN_SWITCH_INTERVAL_SECS) both start counting from
	 * THIS apply — otherwise a stale last_switch_ts lets the fallback branch
	 * drop a still-associating 5G primary straight to 2G. */
	last_switch_ts = uptime_secs();
	fallback_confirm = 0;
	restore_confirm  = 0;
}

#if defined(RTBE58_GO)
/*
 * A BCM6765 driver-level escan/scan-engine wedge ("Escan set error (-16)"
 * kernel spam, GET BSSID failed with -17, scan abort failed) can leave a
 * STA that is ALREADY associated unable to (re)associate at all once it
 * drops, no matter which profile wlcmon picks next -- wpa_supplicant's own
 * reconnect attempts fail at the driver level too. The only recovery
 * observed to clear it is a full radio down/up. This is unconditional on
 * which/how-many profiles are configured: it runs before profile
 * reselection so a single-PAP WISP setup gets the same recovery attempt as
 * a multi-profile one. Harmless when the STA is merely out of range -- a
 * clean down/up before the next association attempt costs nothing.
 */
static void wisp_wlcmon_reset_radio(int unit)
{
	char wlif[8];

	snprintf(wlif, sizeof(wlif), "wl%d", unit);
	syslog(LOG_NOTICE,
	       "wlcmon: %s: %s stuck disconnected -- resetting radio (down/up) "
	       "in case of a driver escan wedge\n", __func__, wlif);
	eval("wl", "-i", wlif, "down");
	sleep(1);
	eval("wl", "-i", wlif, "up");
}
#endif

void wlcmon(int sig)
{
	int wpacli_conn = 0;
	int idx = 0;
	int cur_band = nvram_get_int("wlc_band");
	const char *cur_ssid = nvram_safe_get("wlc_ssid");

	/*
	 * Pick up runtime changes to wlc_profile_list.
	 *
	 * The list is otherwise parsed exactly once, in wlcmon_main(), and
	 * nothing restarts this daemon on a WISP profile change: wlcmon is only
	 * (re)started by the net / net_and_phy / allnet / nas rc_services and by
	 * start_services(), NOT by wisp_serv.  The WebUI "Change Network" flow
	 * updates wlc_profile_list (update_wlc_profile_list(), in-place for an
	 * existing ssid+band, else appended), writes the new wlc_ssid/wlc_band
	 * and then fires notify_rc("restart_wisp_serv") itself -- so without
	 * re-reading here wlcmon keeps rotating its boot-time snapshot: it can
	 * never select an entry the user has just added, and its next
	 * retry-exhaustion cycle re-applies a stale profile, rewriting wlc_ssid
	 * back to the network the user navigated away from.
	 *
	 * Bounded by the buffer size on purpose: a list longer than
	 * LARGE_BUF_SIZE is stored truncated here, and comparing the full
	 * strings would then mismatch forever and re-parse on every tick.
	 */
	if (strncmp(wlc_profile_list, nvram_safe_get("wlc_profile_list"),
		    sizeof(wlc_profile_list) - 1) != 0) {
		/* Sized like the parsed field so the lookups below compare
		 * like-for-like with what parse_wlc_profile_list() stored. */
		char sel_ssid[sizeof(profile_list[0].ssid)];
		int i, comp_unit;

		strlcpy(wlc_profile_list, nvram_safe_get("wlc_profile_list"),
			sizeof(wlc_profile_list));
		parse_wlc_profile_list(wlc_profile_list);
		syslog(LOG_NOTICE,
		       "wlcmon: wlc_profile_list changed -- reloaded %d profile(s)\n",
		       profile_num);
		for (i = 0; i < profile_num; i++)
			print_profile_entry(i);

		strlcpy(sel_ssid, nvram_safe_get("wlc_ssid"), sizeof(sel_ssid));
		if (*sel_ssid) {
			/* Recompute the dual-band companion for the (possibly new)
			 * selection, same rule as wlcmon_main()'s startup pass. */
			comp_unit = is_5g_unit(cur_band) ? find_2g_unit_for_ssid(sel_ssid)
							 : find_5g_unit_for_ssid(sel_ssid);
			if (comp_unit >= 0) {
				char comp_str[4];
				snprintf(comp_str, sizeof(comp_str), "%d", comp_unit);
				nvram_set("wlcp_band", comp_str);
			}
			else
				nvram_set("wlcp_band", "");

			/*
			 * Re-anchor the rotation on the currently selected entry --
			 * for a "Change Network" that is the one the user just
			 * picked, already written to wlc_ssid/wlc_band by httpd.
			 * The lookup is a presence test only (ssid+band is unique
			 * in the list, the WebUI writer dedupes on exactly that
			 * pair); the credentials in use still come from wlc_*.
			 * Without this reset the freshly selected profile inherits
			 * the previous cycle's retry counters and could be
			 * abandoned almost immediately -- it must get a full retry
			 * budget before the normal "current one broken -> try the
			 * next entry" failover moves on from it.
			 */
			if (find_profile_idx_by_ssid_and_unit(sel_ssid, cur_band) >= 0) {
				wlcmon_retry     = 0;
				profiles_tried   = 0;
				nvram_set_int("wlc_tried_all", 0);
				last_switch_ts   = uptime_secs();
				fallback_confirm = 0;
				restore_confirm  = 0;
				syslog(LOG_NOTICE,
				       "wlcmon: re-anchored rotation on selected profile [%s] band=%d\n",
				       sel_ssid, cur_band);
			}
		}
	}

	/*
	 * wlc_restart_profile: when set to "1" by the user/admin, reset the
	 * WISP connection state so that wlcmon starts connecting from the
	 * first entry in wlc_profile_list on the next cycle.
	 *
	 * Actions taken:
	 *   - Reset wlcmon_retry counter.
	 *   - Apply profile[0] immediately via apply_new_wlc(0).
	 *   - Unset wlc_restart_profile so this runs only once.
	 */
	if (nvram_match("wlc_restart_profile", "1") && profile_num > 0) {
		syslog(LOG_NOTICE,
		       "wlcmon: wlc_restart_profile set — resetting to profile[0] [%s]\n",
		       profile_list[0].ssid);
		wlcmon_retry = 0;
		nvram_unset("wlc_restart_profile");
		apply_new_wlc(0);
		return;
	}

	wpacli_conn = get_wpacli_status(cur_band);
	/* Use wlcmon_conn (not the nvram "wlc_state") as our exclusive L2-ok signal.
	 * nvram "wlc_state" is owned by wlcconnect/amas_wlcconnect and writes STOPPED(3)
	 * even when the STA is associated; fighting it causes wanduck false negatives. */
	nvram_set_int("wlcmon_conn", wpacli_conn == 2 ? 1 : 0);

	/*
	 * Dual-band WAN selection:
	 *   - When 5G is primary WAN and 5G disconnects but 2G is up, fall back
	 *     to 2G as WAN by updating wlc_band / wlcp_band and restarting wisp_serv.
	 *   - When 2G is acting as fallback WAN and 5G recovers, restore 5G as
	 *     WAN by updating wlc_band / wlcp_band and restarting wisp_serv.
	 * Both wlc_band and wlcp_band are updated together so that wisp_serv
	 * START can launch wpa_supplicant on BOTH radios.
	 */
	{
		const char *wlcp_pre = nvram_safe_get("wlcp_band");
		int wlcp_unit_pre = (*wlcp_pre) ? atoi(wlcp_pre) : -1;
		int wlcp_state_pre = (wlcp_unit_pre >= 0) ? get_wpacli_status(wlcp_unit_pre) : 0;
		WLCDBG("%s:: [before dual-band-wan-select] wlc_band=%d wlcp_band=%s wpacli_conn=%d wlcp_state=%d\n",
		       __func__, cur_band, wlcp_pre, wpacli_conn, wlcp_state_pre);
	}

	if (*cur_ssid) {
		if (is_5g_unit(cur_band)) {
			/*
			 * Currently on 5G WAN.  If 5G is down but 2G is up, increment
			 * fallback_confirm.  Only switch when the condition has held for
			 * WLCMON_CONFIRM_COUNT consecutive ticks to avoid reacting to
			 * transient glitches.
			 */
			int dual_2g_unit = find_2g_unit_for_ssid(cur_ssid);
			if (dual_2g_unit >= 0 && wpacli_conn != 2 &&
			    get_wpacli_status(dual_2g_unit) == 2) {
				long now = uptime_secs();
				long grace = nvram_get_int("wlcmon_5g_grace");
				if (grace <= 0)
					grace = WLCMON_5G_GRACE_SECS;
				restore_confirm = 0;
				/*
				 * 5G is the freshly-selected primary but has not associated
				 * yet.  Hold the 2G fallback during the post-switch grace so a
				 * slow initial 5G association does not trigger the wasteful
				 * 2G-first-then-restore ping-pong.  Genuine 5G failure still
				 * falls back once the grace elapses.
				 */
				if ((now - last_switch_ts) < grace) {
					fallback_confirm = 0;
					WLCDBG("%s:: 5G still coming up, grace active (%lds remaining) -- hold 2G fallback\n",
					       __func__, (long)(grace - (now - last_switch_ts)));
					goto dual_band_done;
				}
				fallback_confirm++;
				WLCDBG("%s:: 5G down, 2G ready -- fallback_confirm=%d/%d\n",
				       __func__, fallback_confirm, WLCMON_CONFIRM_COUNT);
				if (fallback_confirm >= WLCMON_CONFIRM_COUNT) {
					char band_str[4], wlcp_str[4];

					snprintf(band_str, sizeof(band_str), "%d", dual_2g_unit);
					snprintf(wlcp_str, sizeof(wlcp_str), "%d", cur_band);
					syslog(LOG_NOTICE,
					       "wlcmon: 5G down (confirmed %d ticks), fallback WAN to 2G unit%d, wlcp_band->%d\n",
					       WLCMON_CONFIRM_COUNT, dual_2g_unit, cur_band);
					/* Swap: new primary = 2G, companion = old 5G primary */
					nvram_set("wlc_band",  band_str);
					nvram_set("wlcp_band", wlcp_str);
					WLCDBG("%s:: [after dual-band-wan-select/fallback] wlc_band=%d wlcp_band=%d wpacli_conn=%d wlcp_state=%d\n",
					       __func__, dual_2g_unit, cur_band, 2 /* 2G connected */, wpacli_conn);
					wlcmon_retry    = 0;
					fallback_confirm = 0;
					last_switch_ts  = uptime_secs();
#if defined(RTCONFIG_DUALWAN)
					if (!wisp_wan1_active())
#endif
						notify_rc("restart_wisp_serv");
					return;
				}
			} else {
				/* Condition not met — reset counter */
				fallback_confirm = 0;
			}
		} else {
			/*
			 * Currently on 2G (fallback) WAN.  If 5G has recovered, increment
			 * restore_confirm.  Only switch back to 5G when:
			 *   1. The condition has held for WLCMON_CONFIRM_COUNT consecutive ticks.
			 *   2. At least MIN_SWITCH_INTERVAL_SECS have elapsed since the last
			 *      switch (cooldown), so the freshly restarted wpa_supplicant on
			 *      5G has had time to stabilise before we switch back.
			 */
			int paired_5g_unit = find_5g_unit_for_ssid(cur_ssid);
			if (paired_5g_unit >= 0 &&
			    get_wpacli_status(paired_5g_unit) == 2) {
				long now = uptime_secs();
				if ((now - last_switch_ts) < MIN_SWITCH_INTERVAL_SECS) {
					WLCDBG("%s:: 5G recovered but post-switch cooldown active (%lds remaining)\n",
					       __func__,
					       (long)(MIN_SWITCH_INTERVAL_SECS - (now - last_switch_ts)));
					restore_confirm = 0;
				} else {
					fallback_confirm = 0;
					restore_confirm++;
					WLCDBG("%s:: 5G recovered -- restore_confirm=%d/%d\n",
					       __func__, restore_confirm, WLCMON_CONFIRM_COUNT);
					if (restore_confirm >= WLCMON_CONFIRM_COUNT) {
						char band_str[4], wlcp_str[4];

						snprintf(band_str, sizeof(band_str), "%d", paired_5g_unit);
						snprintf(wlcp_str, sizeof(wlcp_str), "%d", cur_band);
						syslog(LOG_NOTICE,
						       "wlcmon: 5G recovered (confirmed %d ticks), restoring WAN to unit%d, wlcp_band->%d\n",
						       WLCMON_CONFIRM_COUNT, paired_5g_unit, cur_band);
						/* Swap: new primary = 5G, companion = old 2G fallback */
						nvram_set("wlc_band",  band_str);
						nvram_set("wlcp_band", wlcp_str);
						WLCDBG("%s:: [after dual-band-wan-select/restore] wlc_band=%d wlcp_band=%d wpacli_conn=%d wlcp_state=%d\n",
						       __func__, paired_5g_unit, cur_band, 2 /* 5G connected */, wpacli_conn);
						wlcmon_retry   = 0;
						restore_confirm = 0;
						last_switch_ts = uptime_secs();
#if defined(RTCONFIG_DUALWAN)
						if (!wisp_wan1_active())
#endif
							notify_rc("restart_wisp_serv");
						return;
					}
				}
			} else {
				/* Condition not met — reset counter */
				restore_confirm = 0;
			}
		}
	}

dual_band_done:
	WLCDBG("%s:: [after dual-band-wan-select/no-change] wlc_band=%d wlcp_band=%s wpacli_conn=%d (retry:%d)\n",
	       __func__, cur_band, nvram_safe_get("wlcp_band"), wpacli_conn, wlcmon_retry);

	if (!wpacli_conn) {
		/*
		 * For a dual-band same-SSID profile, check the companion band as
		 * well.  Only advance the retry counter when BOTH bands are
		 * disconnected — if the companion radio is still associated the
		 * fallback/recover path above will handle the WAN switch on the
		 * next wlcmon tick, and we should not disturb it by switching
		 * profiles prematurely.
		 */
		const char *wlcp_str = nvram_safe_get("wlcp_band");
		int wlcp_unit = (*wlcp_str) ? atoi(wlcp_str) : -1;
		int wlcp_state = (wlcp_unit >= 0) ? get_wpacli_status(wlcp_unit) : 0;

		if (wlcp_state != 0) {
			/* Companion band still connected; hold retry counter */
		} else {
			int i;
			int has_other_ssid = 0;

#if defined(RTCONFIG_WISP) && defined(RTCONFIG_DUALWAN)
			/*
			 * secwan-first: when the WiFi WAN is NOT the active primary
			 * (wanduck has failed over to the wired/USB backup and it is
			 * serving), stay completely quiescent -- do NOT pause fc or cycle
			 * WISP profiles.  wisp_fc_hwaccel_pause() below disables the
			 * accelerator for the *working* backup and then trips
			 * wisp_fc_state_probe() into a restart_firewall storm every ~90s;
			 * and switching profiles is a no-op anyway because apply_new_wlc()
			 * skips restart_wisp_serv while wan1 is active, so the STA only ever
			 * re-tries the CURRENT profile.  Leave the WiFi STA's wpa_supplicant
			 * running in the background on its current profile so a genuine
			 * failback still works once the backup drops.
			 */
			if (wisp_mode() && wisp_wan1_active())
				return;
#endif
			wlcmon_retry++;
			if (wlcmon_retry >= wlcmon_retry_max) {
				wlcmon_retry = 0;

				/*
				 * Check whether there is at least one profile with a
				 * different SSID -- used only for the syslog wording
				 * below. choose_next_profile_idx() already falls back to
				 * re-selecting the current (best-scored) profile when no
				 * other SSID exists, so this must NOT gate whether we act:
				 * a single-profile WISP setup needs to recover too, not
				 * just a multi-profile one.
				 */
				for (i = 0; i < profile_num; i++) {
					if (strcmp(profile_list[i].ssid, cur_ssid) != 0) {
						has_other_ssid = 1;
						break;
					}
				}

#if defined(RTBE58_GO)
				wisp_fc_hwaccel_pause();
				wisp_wlcmon_reset_radio(cur_band);
#endif

				idx = choose_next_profile_idx();
				WLCDBG("%s:: try conn w/ %s profile...\n", __func__,
				       has_other_ssid ? "next" : "the only");
				print_profile_entry(idx);
				profiles_tried++;
				if (profiles_tried >= profile_num) {
					nvram_set_int("wlc_tried_all", 1);
					syslog(LOG_NOTICE,
					       "wlcmon: all %d profiles tried, no connection\n",
					       profile_num);
					profiles_tried = 0;
				}
				apply_new_wlc(idx);
			}
		}
	} else {
		wlcmon_retry = 0;
		if (profiles_tried != 0 || nvram_get_int("wlc_tried_all")) {
			profiles_tried = 0;
			nvram_set_int("wlc_tried_all", 0);
			syslog(LOG_NOTICE, "wlcmon: connected, cleared wlc_tried_all\n");
		}
	}
}

int
wlcmon_main(int argc, char *argv[])
{
        FILE *fp;
	int i;
	int wlc_band_init, wlcp_band_init;
	const char *cur_ssid;
	char wlcp_band_str[4];

	strlcpy(wlc_profile_list, nvram_safe_get("wlc_profile_list"), sizeof(wlc_profile_list));
	wlcmon_retry_max = nvram_get_int("wlcmon_retry_max") ? : 3;

	parse_wlc_profile_list(wlc_profile_list);

	/*
	 * At startup, determine the companion band (wlcp_band) from the
	 * current wlc_ssid / wlc_band.  If the active SSID has a matching
	 * profile entry on the other band, set wlcp_band so that wisp_serv
	 * can start wpa_supplicant on both radios immediately without waiting
	 * for the first apply_new_wlc() call.
	 */
	cur_ssid      = nvram_safe_get("wlc_ssid");
	wlc_band_init = nvram_get_int("wlc_band");
	wlcp_band_init = -1;

	if (*cur_ssid) {
		if (is_5g_unit(wlc_band_init))
			wlcp_band_init = find_2g_unit_for_ssid(cur_ssid);
		else
			wlcp_band_init = find_5g_unit_for_ssid(cur_ssid);

		if (wlcp_band_init >= 0) {
			snprintf(wlcp_band_str, sizeof(wlcp_band_str), "%d", wlcp_band_init);
			nvram_set("wlcp_band", wlcp_band_str);
		} else {
			nvram_set("wlcp_band", "");
		}
	}

	syslog(LOG_NOTICE,
	       "wlcmon start: wlc_ssid=%s wlc_band=%d wlcp_band=%d "
	       "profile_num=%d retry-max=%d\n",
	       cur_ssid, wlc_band_init, wlcp_band_init,
	       profile_num, wlcmon_retry_max);

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

