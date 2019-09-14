/*

	Copyright 2005, Broadcom Corporation
	All Rights Reserved.

	THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
	KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
	SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
	FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.

*/

#include "rc.h"

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
		return _eval(nas_argv, NULL, 0, &pid);

	return 0;
}

void
stop_nas(void)
{
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

// TODO: wlcscan_main
// 	- scan and save result into files for httpd hook
//	- handlling stop signal

int wlcscan_main(void)
{
	FILE *fp=NULL;
	char word[256]={0}, *next = NULL;
#if defined(RTCONFIG_CONCURRENTREPEATER) && defined(RTCONFIG_MTK_REP)		
	char wl_ifs[256]={0};
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
	if ((fp = fopen(APSCAN_INFO, "w")) != NULL) {
		fclose(fp);
	}
	file_unlock(lock);

	nvram_set_int("wlc_scan_state", WLCSCAN_STATE_INITIALIZING);

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
	foreach (word, nvram_safe_get("wl_ifnames"), next)
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
			wlcscan_core(APSCAN_INFO, word);

		// suppose only two or less interface handled
		nvram_set_int("wlc_scan_state", WLCSCAN_STATE_2G+i);

		i++;
	}

#ifdef RTCONFIG_QTN
	wlcscan_core_qtn(APSCAN_INFO, "wifi0");
#endif
#ifdef RTCONFIG_QSR10GBAK
	if(clnt != NULL){
		clnt_destroy(clnt);
	}
#endif

	nvram_set_int("wlc_scan_state", WLCSCAN_STATE_FINISHED);

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

				// notify the change to init.
				if (ret == WLC_STATE_CONNECTED)
				{
					notify_rc_and_wait("restart_wlcmode 1");
				}
				else
				{
					notify_rc_and_wait("restart_wlcmode 0");
#if defined(RTCONFIG_CONCURRENTREPEATER) && defined(RTCONFIG_RALINK)
					nvram_set_int("lan_ready",0);
#endif
				}

#if defined(RTCONFIG_BLINK_LED)
#if defined(RTCONFIG_QCA)
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
			led_control(id, LED_ON);
		} else {
			set_bled_udef_pattern(led_gpio, 700, "0 1");
			set_bled_udef_pattern_mode(led_gpio);
			led_control(id, LED_ON);
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
