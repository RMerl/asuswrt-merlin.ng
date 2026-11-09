/*

	Copyright 2005, Broadcom Corporation
	All Rights Reserved.

	THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
	KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
	SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
	FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.

*/

#include "rc.h"
#if defined(RTCONFIG_RALINK)
#include "ate.h"
#endif

#include <sys/sysinfo.h>
#include <sys/ioctl.h>
#include <bcmutils.h>
#include <wlutils.h>

#ifdef RTCONFIG_QSR10G
#include "qcsapi_output.h"
#include "qcsapi_rpc_common/client/find_host_addr.h"

#include "qcsapi.h"
#include "qcsapi_rpc/client/qcsapi_rpc_client.h"
#include "qcsapi_rpc/generated/qcsapi_rpc.h"
#include <qcsapi_rpc_common/common/rpc_raw.h>
#include <qcsapi_rpc_common/common/rpc_pci.h>
#include "qcsapi_driver.h"
#include "call_qcsapi.h"
#endif

#if defined(RPAX58) || defined(RPBE58) || defined(RTBE58_GO)
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>
#endif

//	ref: http://wiki.openwrt.org/OpenWrtDocs/nas

//	#define DEBUG_TIMING

#ifdef REMOVE
static int security_on(int idx, int unit, int subunit, void *param)
{
	return nvram_get_int(wl_nvname("radio", unit, 0)) && (!nvram_match(wl_nvname("security_mode", unit, subunit), "disabled"));
}
static int is_wds(int idx, int unit, int subunit, void *param)
{
	return nvram_get_int(wl_nvname("radio", unit, 0)) && nvram_get_int(wl_nvname("wds_enable", unit, subunit));
}
#ifndef CONFIG_BCMWL5
static int is_sta(int idx, int unit, int subunit, void *param)
{
	return nvram_match(wl_nvname("mode", unit, subunit), "sta");
}
#endif
int wds_enable(void)
{
	return foreach_wif(1, NULL, is_wds);
}
#endif

#ifdef CONFIG_BCMWL5
extern int restore_defaults_g;

int
start_nas(void)
{
	char *nas_argv[] = { "nas", NULL };
	pid_t pid;

	stop_nas();

	if (!restore_defaults_g)
	{
#ifdef RTCONFIG_BRCM_HOSTAPD
		if (!nvram_match("hapd_enable", "0")) {
			start_hapd_wpasupp(0);
			start_wps_pbcd();
			return 0;
		} else
#endif
		return _eval(nas_argv, NULL, 0, &pid);
	}

	return 0;
}

void
stop_nas(void)
{
#ifdef RTCONFIG_BRCM_HOSTAPD
        if (!nvram_match("hapd_enable", "0")) {
		stop_hapd_wpasupp();
		stop_wps_pbcd();
        } else
#endif
	killall_tk("nas");
}

#ifdef REMOVE
void notify_nas(const char *ifname)
{
#ifdef DEBUG_TIMING
	struct sysinfo si;
	sysinfo(&si);
	_dprintf("%s: ifname=%s uptime=%ld\n", __FUNCTION__, ifname, si.uptime);
#else
	_dprintf("%s: ifname=%s\n", __FUNCTION__, ifname);
#endif

#ifdef CONFIG_BCMWL5

	/* Inform driver to send up new WDS link event */
	if (wl_iovar_setint((char *)ifname, "wds_enable", 1)) {
		_dprintf("%s: set wds_enable failed\n", ifname);
	}

#else	/* !CONFIG_BCMWL5 */

	if (!foreach_wif(1, NULL, security_on)) return;

	int i;
	int unit;

	i = 10;
	while (pidof("nas") == -1) {
		_dprintf("waiting for nas i=%d\n", i);
		if (--i < 0) {
			syslog(LOG_ERR, "Unable to find nas");
			break;
		}
		sleep(1);
	}
	sleep(5);

	/* the wireless interface must be configured to run NAS */
	wl_ioctl((char *)ifname, WLC_GET_INSTANCE, &unit, sizeof(unit));

	xstart("nas4not", "lan", ifname, "up", "auto",
		nvram_safe_get(wl_nvname("crypto", unit, 0)),	// aes, tkip (aes+tkip ok?)
		nvram_safe_get(wl_nvname("akm", unit, 0)),	// psk (only?)
		nvram_safe_get(wl_nvname("wpa_psk", unit, 0)),	// shared key
		nvram_safe_get(wl_nvname("ssid", unit, 0))	// ssid
	);

#endif /* CONFIG_BCMWL5 */
}
#endif
#endif

#if defined(CONFIG_BCMWL5) \
		|| (defined(RTCONFIG_RALINK) && defined(RTCONFIG_WIRELESSREPEATER)) \
		|| defined(RTCONFIG_QCA) || defined(RTCONFIG_REALTEK) \
		|| defined(RTCONFIG_QSR10G) || defined(RTCONFIG_LANTIQ)
#define APSCAN_INFO "/tmp/apscan_info.txt"

static int lock = -1;

static void wlcscan_safeleave(int signo) {
	signal(SIGTERM, SIG_IGN);

	nvram_set_int("wlc_scan_state", WLCSCAN_STATE_STOPPED);

	file_unlock(lock);
	exit(0);
}

#if defined(RPAX58) || defined(RPBE58) || defined(RTBE58_GO)

#define MAX_LINE_LENGTH 1024
#define MAX_AP_LIST	250
#if defined(RTBE58_GO)
#define MAX_FRESH_TIME	60
#else
#define MAX_FRESH_TIME	300
#endif

long parse_suffix(const char *filepath) {
	const char *last_dot = strrchr(filepath, '.');
	if (!last_dot) {
		fprintf(stderr, "Error: No '.' found in filepath: %s\n", filepath);
		return -1;
	}

	char *endptr;
	long suffix = strtol(last_dot + 1, &endptr, 10);

	if (*endptr != '\0' && *endptr != 'e') {
		fprintf(stderr, "Error: Invalid numeric suffix in filepath: %s\n", filepath);
		return -1;
	}

	return suffix;
}

int is_file_old(const char *filename) {
	long max_fresh_time = nvram_get_int("wlcscan_fresh_time")?:MAX_FRESH_TIME;
	long current_time = uptime();
	long file_time = parse_suffix(filename);

	dbg("%s, cur[%ld] - ft[%ld] = %ld\n", __func__, current_time, file_time, (current_time - file_time));
	return (current_time - file_time) > max_fresh_time;
}

void merge_file(const char *filename, FILE *output_file, char **line_set, size_t *line_count, int max_list) {
	int is_duplicate = 0;
	FILE *input_file = fopen(filename, "r");
	if (!input_file) {
		perror("fopen");
		return;
	}

	char line[MAX_LINE_LENGTH];
	while (fgets(line, sizeof(line), input_file)) {
		// Remove newline character at the end
		line[strcspn(line, "\n")] = 0;

		// Check for duplicates
		is_duplicate = 0;
		for (size_t i = 0; i < *line_count; i++) {
			if (i<max_list && strcmp(line_set[i], line) == 0) {
				is_duplicate = 1;
				break;
			}
		}

		// If not a duplicate, add it to the output file and set
		if (!is_duplicate) {
			fprintf(output_file, "%s\n", line);
			line_set[*line_count] = strdup(line);
			(*line_count)++;
		}
		if (*line_count >= max_list-1) {
			dbg("[wlc] : apscan_info file is full.\n");
			break;
		}
	}

	fclose(input_file);
}

int apscan_merge() {
	const char *pattern = "apscan_info.txt.";
	char apinfo_file[32];
	int max_ap_list = nvram_get_int("wlcscan_max_aplist")?:MAX_AP_LIST;
	DIR *dir = opendir("/tmp");
	if (!dir) {
		perror("opendir");
		return 1;
	}

	char **line_set = malloc(MAX_AP_LIST * sizeof(char *));
	size_t line_count = 0;

	FILE *output_file = fopen(APSCAN_INFO, "w");
	if (!output_file) {
		perror("fopen");
		closedir(dir);
		free(line_set);
		return 1;
	}

	struct dirent *entry;
	while ((entry = readdir(dir)) != NULL) {
		if (strncmp(entry->d_name, pattern, strlen(pattern)) == 0) {
			snprintf(apinfo_file, sizeof(apinfo_file), "/tmp/%s", entry->d_name);
			if (is_file_old(apinfo_file)) {
				dbg("Skipping old file: %s\n", apinfo_file);
				remove(apinfo_file);
			} else {
				if (line_count < max_ap_list - 1) {
					dbg("Merging file: %s\n", apinfo_file);
					merge_file(apinfo_file, output_file, line_set, &line_count, max_ap_list);
				} else {
					dbg("Skip merging file: %s due full.\n", apinfo_file);
					break;
				}
			}
		}
	}

	closedir(dir);
	fclose(output_file);

	for (size_t i = 0; i < line_count; i++) {
		free(line_set[i]);
	}
	free(line_set);

	dbg("[wlc] Merge complete. Output written to %s\n", APSCAN_INFO);
	return 0;
}


#endif

// TODO: wlcscan_main
// 	- scan and save result into files for httpd hook
//	- handlling stop signal

int wlcscan_main(void)
{
	FILE *fp=NULL;
	char word[256]={0}, *next = NULL;
#if defined(RTCONFIG_CONCURRENTREPEATER) && defined(RTCONFIG_MTK_REP)		
	char wl_ifs[256]={0};
#else
	char wl_ifnames[32] = { 0 };
#endif
	int i = 0;
#ifdef RTCONFIG_QSR10G
	CLIENT *clnt;
	char host[18];
#endif
#ifdef __CONFIG_DHDAP__
	char tmp[100], prefix[]="wlXXXXXXX_";
	int is_dhd = 0;
#endif

#if defined(RPAX58) || defined(RPBE58) || defined(RTBE58_GO)
	int apscan_counts = nvram_get_int("apscan_counts");
	char apscan_file[32];
	int wlcscan_wait = nvram_get_int("pre_wlcscan_wait")?:1;

	if (nvram_match("wlescan", "1"))
		snprintf(apscan_file, sizeof(apscan_file), "%s.%d.%ld.e", APSCAN_INFO, apscan_counts, uptime());
	else 
		snprintf(apscan_file, sizeof(apscan_file), "%s.%d.%ld", APSCAN_INFO, apscan_counts, uptime());

	if (nvram_match("x_Setting", "0")) {
		if (pids("obd")) {
			_dprintf("%s, disable obd/amas_ssd_cd due ss, wait %d for wlcscan\n", __func__, wlcscan_wait);
			nvram_set("no_obd", "1");
			killall_tk("obd");
			stop_conn_diag_ss();
			sleep(wlcscan_wait);
		} else
			sleep(wlcscan_wait);
	}
#endif

	signal(SIGTERM, wlcscan_safeleave);

#ifdef RTCONFIG_QSR10G
	snprintf(host, sizeof(host), "localhost");
	clnt = clnt_pci_create(host, QCSAPI_PROG, QCSAPI_VERS, NULL);
	if (clnt == NULL) {
		_dprintf("[%s][%d] clnt_pci_create() error\n", __func__, __LINE__);
	}else{
		client_qcsapi_set_rpcclient(clnt);
	}
#endif

	/* clean APSCAN_INFO */
	lock = file_lock("sitesurvey");
#if defined(RPAX58) || defined(RPBE58) || defined(RTBE58_GO)
	if ((fp = fopen(apscan_file, "w")) != NULL) {
#else
	if ((fp = fopen(APSCAN_INFO, "w")) != NULL) {
#endif
		fclose(fp);
	}
	file_unlock(lock);

	nvram_set_int("wlc_scan_state", WLCSCAN_STATE_INITIALIZING);
#ifdef CONFIG_BCMWL5
	nvram_set_int("wlcscan", 1);
#endif

	/* Starting scanning */
	i = 0;
#if defined(RTCONFIG_CONCURRENTREPEATER) && defined(RTCONFIG_MTK_REP)
	if (nvram_match("wps_cli_state", "1")) {
		if (nvram_match("wlc_express", "1"))
			strncpy(wl_ifs,nvram_safe_get("wl0_ifname"), sizeof(wl_ifs));
		else if (nvram_match("wlc_express", "2"))
			strncpy(wl_ifs,nvram_safe_get("wl1_ifname"), sizeof(wl_ifs));		
		else  
			strncpy(wl_ifs,nvram_safe_get("wl_ifnames"), sizeof(wl_ifs));
	}
	else
		strncpy(wl_ifs,nvram_safe_get("wl_ifnames"), sizeof(wl_ifs));
	foreach (word, wl_ifs, next)
#else
	strlcpy(wl_ifnames, nvram_safe_get("wl_ifnames"), sizeof(wl_ifnames));
	foreach (word, wl_ifnames, next)
#endif
	{	
		SKIP_ABSENT_BAND_AND_INC_UNIT(i);
#ifdef __CONFIG_DHDAP__
		is_dhd = !dhd_probe(word);
		snprintf(prefix, sizeof(prefix), "wl%d_", i);
		if (is_dhd && !nvram_match(strcat_r(prefix, "mode", tmp), "wds"))
			wlcscan_core_escan(APSCAN_INFO, word);
		else
#endif
		{
#if defined(RPAX58) || defined(RPBE58) || defined(RTBE58_GO)
			wlcscan_core(apscan_file, word);
#else
			wlcscan_core(APSCAN_INFO, word);
#endif
		}
		// suppose only two or less interface handled
		nvram_set_int("wlc_scan_state", WLCSCAN_STATE_2G+i);

		i++;
	}

#if defined(RPAX58) || defined(RPBE58) || defined(RTBE58_GO)
	apscan_merge();

	nvram_set_int("apscan_counts", ++apscan_counts);
	nvram_set("wlescan", "0");
	dbg("[wlc] fin.(%d)\n", nvram_get_int("apscan_counts"));
#endif
#ifdef RTCONFIG_QTN
	wlcscan_core_qtn(APSCAN_INFO, "wifi0");
#endif
#ifdef RTCONFIG_QSR10GBAK
	if(clnt != NULL){
		clnt_destroy(clnt);
	}
#endif

#if defined(RPAX58) || defined(RPBE58) || defined(RTBE58_GO)
	struct stat filestat;

	if (!stat(APSCAN_INFO, &filestat) && filestat.st_size > 0) {
		nvram_set_int("wlc_scan_state", WLCSCAN_STATE_FINISHED);
		nvram_set_int("wlcscan", 0);
		dbg("[wlc] wlcscan fin.(%d)\n", filestat.st_size);
#if defined(RPAX58) || defined(RPBE58) || defined(RTBE58_GO)
		if (nvram_match("x_Setting", "0") && !pids("obd")) {
			_dprintf("%s, recover obd due ss fin.\n", __func__);
			start_obd();
			nvram_set("no_obd", "0");
		} else
			_dprintf("%s, obd?(%d)(%d).\n", __func__, nvram_get_int("x_Setting"), pids("obd"));
#endif
	} else {
		dbg("[wlc] invalid scan results, re-scan...\n\n");
		if (nvram_match("escan_ref", "1"))
			nvram_set("wlescan", "1");
		system("wlcscan");
	}
#else
	nvram_set_int("wlc_scan_state", WLCSCAN_STATE_FINISHED);
#ifdef CONFIG_BCMWL5
	nvram_set_int("wlcscan", 0);
#endif
#endif

	return 1;
}

#endif /* CONFIG_BCMWL5 || (RTCONFIG_RALINK && RTCONFIG_WIRELESSREPEATER) || defined(RTCONFIG_QCA) */

#ifdef RTCONFIG_WIRELESSREPEATER
#define NOTIFY_IDLE 0
#define NOTIFY_CONN 1
#define NOTIFY_DISCONN -1

static void wlcconnect_safeleave(int signo) {
	signal(SIGTERM, SIG_IGN);

	nvram_set_int("wlc_state", WLC_STATE_STOPPED);
	nvram_set_int("wlc_sbstate", WLC_STOPPED_REASON_MANUAL);

	exit(0);
}

#if defined(RTCONFIG_CONCURRENTREPEATER) && defined(RTCONFIG_REALTEK)
int get_wlc_rssi_status(char *interface)
{
	bss_info bss_buf;
	char bssid[8];
	memset(&bss_buf,0,sizeof(bss_info));
	if (wl_ioctl(interface, WLC_GET_BSSID, bssid, ETHER_ADDR_LEN) < 0 ||
	    wl_ioctl(interface, WLC_GET_BSS_INFO, &bss_buf, sizeof(bss_buf)) < 0)
		return 0;
	logmessage(__func__,"*******%s %d interface=%s state=%d channel=%d txRate=%d rssi=%d ssid=%s bssid=%x%x%x%x%x%x********\n",__FUNCTION__,__LINE__,interface,bss_buf.state,bss_buf.channel,bss_buf.txRate,bss_buf.rssi,bss_buf.ssid,bss_buf.bssid[0],bss_buf.bssid[1],bss_buf.bssid[2],bss_buf.bssid[3],bss_buf.bssid[4],bss_buf.bssid[5]);

	return (bss_buf.rssi);
}
#endif

// wlcconnect_main
//	wireless ap monitor to connect to ap
//	when wlc_list, then connect to it according to priority
int wlcconnect_main(void)
{
_dprintf("%s: Start to run...\n", __FUNCTION__);
	int ret, old_ret = -1, sleep_s = 0, sleep_us = 0;
	int link_setup = 0, wlc_count = 0;
	int wanduck_notify = NOTIFY_IDLE;
	int wlc_wait_time = nvram_get_int("wl_time") ? : 5;
#if defined(RTCONFIG_BLINK_LED)
	int unit = nvram_get_int("wlc_band");
	char *led_gpio = get_wl_led_gpio_nv(unit);
#endif

	_dprintf("%s: Start to run...\n", __FUNCTION__);
	signal(SIGTERM, wlcconnect_safeleave);
#if (defined(RTCONFIG_RALINK) || defined(RTCONFIG_QCA)) \
 && !defined(RTCONFIG_CONCURRENTREPEATER)
	signal(SIGTSTP, wlcconnect_sig_handle);
	signal(SIGCONT, wlcconnect_sig_handle);
#endif

	nvram_set_int("wlc_state", WLC_STATE_INITIALIZING);
	nvram_set_int("wlc_sbstate", WLC_STOPPED_REASON_NONE);
	nvram_set_int("wlc_state", WLC_STATE_CONNECTING);

#ifdef RTCONFIG_LANTIQ
	start_repeater();
#endif

#if defined(RPAC51)
	sleep_us = 500;
#elif defined(RTCONFIG_RALINK)
	sleep_s = 1;
#elif defined(RTCONFIG_QCA)
	if (mediabridge_mode())
		sleep_s = 20;
	else
		sleep_s = 5;
#endif

	while (1) {
		ret = wlcconnect_core();
		if (ret == WLC_STATE_CONNECTED) nvram_set_int("wlc_state", WLC_STATE_CONNECTED);
		else if (ret == WLC_STATE_CONNECTING) {
			nvram_set_int("wlc_state", WLC_STATE_STOPPED);
			nvram_set_int("wlc_sbstate", WLC_STOPPED_REASON_AUTH_FAIL);
		}
		else if (ret == WLC_STATE_INITIALIZING) {
			nvram_set_int("wlc_state", WLC_STATE_STOPPED);
			nvram_set_int("wlc_sbstate", WLC_STOPPED_REASON_NO_SIGNAL);
		}
		else if (ret == WLC_STATE_STOPPED) {
			nvram_set_int("wlc_state", WLC_STATE_STOPPED);
			nvram_set_int("wlc_sbstate", WLC_STOPPED_REASON_NO_SIGNAL);
		}

		// let ret be two value: connected, disconnected.
		if (ret != WLC_STATE_CONNECTED ||
			(nvram_match("lan_proto", "dhcp") && nvram_get_int("lan_state_t") != LAN_STATE_CONNECTED))
			ret = WLC_STATE_CONNECTING;

		if(link_setup == 1){
			if(ret != WLC_STATE_CONNECTED){
				if(wlc_count < 3){
					wlc_count++;
_dprintf("Ready to disconnect...%d.\n", wlc_count);
					if (sleep_s > 0)
						sleep(sleep_s);
					if (sleep_us > 0)
						usleep(sleep_us);
					continue;
				}
			}
			else
				wlc_count = 0;
		}

		if (ret != old_ret) {
			if (link_setup == 0 && ret == WLC_STATE_CONNECTED)
				link_setup = 1;

			if (link_setup) {
				if (ret == WLC_STATE_CONNECTED)
					wanduck_notify = NOTIFY_CONN;
				else
					wanduck_notify = NOTIFY_DISCONN;

#if defined(RTCONFIG_CONCURRENTREPEATER) && defined(RTCONFIG_REALTEK)
#if defined(RPAC55)
				if (!strcmp(nvram_safe_get("wlc_band"), "0"))
				{
					get_wlc_rssi_status("wl0-vxd");	
				}
				else if (!strcmp(nvram_safe_get("wlc_band"), "1"))
				{
					get_wlc_rssi_status("wl1-vxd");
				}
#endif
#endif
				// notify the change to init.
				if (ret == WLC_STATE_CONNECTED)
				{
#if defined(RTCONFIG_WISP)
					if (!wisp_mode())
#endif
					notify_rc_and_wait("restart_wlcmode 1");
				}
				else
				{
#if defined(RTCONFIG_WISP)
					if (!wisp_mode())
#endif
					notify_rc_and_wait("restart_wlcmode 0");
#if defined(RTCONFIG_CONCURRENTREPEATER) && defined(RTCONFIG_RALINK)
					nvram_set_int("lan_ready",0);
#endif
				}

#if defined(RTCONFIG_BLINK_LED)
#if defined(RTCONFIG_QCA) || defined(RTCONFIG_RALINK)
				if (wanduck_notify == NOTIFY_CONN)
					append_netdev_bled_if(led_gpio, get_staifname(unit));
				else
					remove_netdev_bled_if(led_gpio, get_staifname(unit));
				update_wifi_led_state_in_wlcmode();
#endif
#endif
			}

#ifdef WEB_REDIRECT
			if (wanduck_notify == NOTIFY_DISCONN) {
				wanduck_notify = NOTIFY_IDLE;

				logmessage("notify wanduck", "wlc_state change!");
				_dprintf("%s: notify wanduck: wlc_state=%d.\n", __FUNCTION__, ret);
				// notify the change to wanduck.
				kill_pidfile_s("/var/run/wanduck.pid", SIGUSR1);
			}
#endif

			old_ret = ret;
		}

		sleep(wlc_wait_time);
	}

	return 0;
}

void repeater_pap_disable(void)
{
	char word[256], *next;
	int wlc_band = nvram_get_int("wlc_band");
	int i;

	i = 0;

	foreach(word, nvram_safe_get("wl_ifnames"), next) {
		SKIP_ABSENT_BAND_AND_INC_UNIT(i);
#if defined(RTCONFIG_REPEATER_STAALLBAND)
		if (i != WL_60G_BAND) {
#else
		if (wlc_band == i) {
#endif
			eval("ebtables", "-t", "filter", "-I", "FORWARD", "-i", word, "-j", "DROP");
			break;
		}
		i++;
	}
}

#if defined(RTCONFIG_BLINK_LED)
void update_wifi_led_state_in_wlcmode(void)
{
	int band, wlc_state = nvram_get_int("wlc_state"), wlc_band = nvram_get_int("wlc_band");
	char *led_gpio;
	enum led_id id;

	if (!repeater_mode() && !mediabridge_mode())
		return;

	for (band = 0; band < MAX_NR_WL_IF; ++band) {
		SKIP_ABSENT_BAND(band);
		if (band >= 2) {
			/* 2-nd 5G LED and 11ad LED have not been supported! */
			dbg("%s: Unknown LED for wl%d\n", __func__, band);
			continue;
		}
		id = get_wl_led_id(band);
		led_gpio = get_wl_led_gpio_nv(band);
		if (band != wlc_band) {
			if (mediabridge_mode())
				led_control(id, LED_OFF);
			continue;
		}

		if (wlc_state == WLC_STATE_CONNECTED) {
			set_bled_normal_mode(led_gpio);
			led_control(id, inhibit_led_on()? LED_OFF : LED_ON);
		} else {
			set_bled_udef_pattern(led_gpio, 700, "0 1");
			set_bled_udef_pattern_mode(led_gpio);
			led_control(id, inhibit_led_on()? LED_OFF : LED_ON);
		}
	}
}
#endif
#endif	/* RTCONFIG_WIRELESSREPEATER */

#if defined(RTCONFIG_QCA) || defined(RTCONFIG_RALINK)
int dump_powertable(void)
{
	_dump_powertable();
	return 0;
}
#endif

#if defined(RTCONFIG_RALINK)
int dump_txbftable(void)
{
	_dump_txbftable();
	return 0;
}
#endif
