/*
 * WPS push button
 *
 * Copyright 2020 Broadcom
 *
 * This program is the proprietary software of Broadcom and/or
 * its licensors, and may only be used, duplicated, modified or distributed
 * pursuant to the terms and conditions of a separate, written license
 * agreement executed between you and Broadcom (an "Authorized License").
 * Except as set forth in an Authorized License, Broadcom grants no license
 * (express or implied), right to use, or waiver of any kind with respect to
 * the Software, and Broadcom expressly reserves all rights in and to the
 * Software and all intellectual property rights therein.  IF YOU HAVE NO
 * AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
 * WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF
 * THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use
 * all reasonable efforts to protect the confidentiality thereof, and to
 * use this information only in connection with your use of Broadcom
 * integrated circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
 * REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR
 * OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
 * DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
 * NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
 * ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
 * OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
 * BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL,
 * SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR
 * IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
 * IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii)
 * ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF
 * OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY
 * NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 * $Id: wps_pb.c 771948 2019-02-12 09:40:01Z $
 */

#include <stdio.h>
#include <time.h>
#include <tutrace.h>
#include <wps_pb.h>
#include <wpscommon.h>
#include <bcmparams.h>
#include <wps_led.h>
#include <security_ipc.h>
#include <wps_wl.h>
#include <bcmutils.h>
#include <wlif_utils.h>
#include <shutils.h>
#include <wps_wps.h>
#include <wps_ui.h>

#ifdef BCMWPSAPSTA
#include <wps_enrapi.h>
#include <wps_sta.h>
#include <wps_enr_osl.h>
#include <wps_enrapi.h>
#endif /* BCMWPSAPSTA */
#include <bcmnvram.h>

extern void wps_setProcessStates(int state);

static time_t wps_pb_selecting_time = 0;
static PBC_STA_INFO pbc_info[PBC_OVERLAP_CNT];
static int wps_pb_state = WPS_PB_STATE_INIT;
static wps_hndl_t pb_hndl;
static int pb_last_led_id = -1;
static char pb_ifname[IFNAMSIZ] = {0};

#ifdef BCMWPSAPSTA
/* for apsta mode concurrent wps */
#define WPS_FINDPBC_STATE_INIT		0
#define WPS_FINDPBC_STATE_SCANNING	1
#define WPS_FINDPBC_STATE_FINDINGAP	2
#define WPS_PBC_PREFER_INTF_COUNT	15

wps_pbc_apsta_intf_t wps_pbc_ap_ifnames[WPS_MAX_PBC_APSTA] = {
	{"wps_pbc_ap_ifname", ""},
	{"wps_pbc_ap2_ifname", ""},
	{"wps_pbc_ap3_ifname", ""}
};
wps_pbc_apsta_intf_t wps_pbc_sta_ifnames[WPS_MAX_PBC_APSTA] = {
	{"wps_pbc_sta_ifname", ""},
	{"wps_pbc_sta2_ifname", ""},
	{"wps_pbc_sta3_ifname", ""}
};

static unsigned long wps_enr_scan_state_time[WPS_MAX_PBC_APSTA];
static char wps_enr_scan_state[WPS_MAX_PBC_APSTA];
static int wps_virtual_pressed = WPS_NO_BTNPRESS;
static bool wps_pbc_apsta_upstream_pushed[WPS_MAX_PBC_APSTA];
static int wps_pbc_prefer_intf_delay_count;
static int wps_pbc_prefer_intf_count_tmp;
#endif /* BCMWPSAPSTA */

/* PBC Overlapped detection */
int
wps_pb_check_pushtime(unsigned long time)
{
	int i;
	int PBC_sta = PBC_OVERLAP_CNT;
	for (i = 0; i < PBC_OVERLAP_CNT; i++) {
		/*
		 * 120 seconds is too sensitive, it have a chance that we receive
		 * a last ProbeReq with WPS_DEVICEPWDID_PUSH_BTN after clearing
		 * this station.  So plus 2 seconds
		 */
		if ((time < pbc_info[i].last_time) ||
		    ((time - pbc_info[i].last_time) > (120 + 2))) {
			memset(&pbc_info[i], 0, sizeof(PBC_STA_INFO));
		}

		if (pbc_info[i].last_time == 0)
			PBC_sta--;
	}

	TUTRACE((TUTRACE_INFO, "There are %d sta in PBC mode!\n", PBC_sta));
	TUTRACE((TUTRACE_INFO, "sta1: %02x:%02x:%02x:%02x:%02x:%02x, LT:%d/CT:%d\n",
		pbc_info[0].mac[0], pbc_info[0].mac[1], pbc_info[0].mac[2], pbc_info[0].mac[3],
		pbc_info[0].mac[4], pbc_info[0].mac[5], pbc_info[0].last_time, time));
	if (PBC_sta > 1)
		TUTRACE((TUTRACE_INFO, "sta2: %02x:%02x:%02x:%02x:%02x:%02x, LT:%d/CT:%d\n",
			pbc_info[1].mac[0], pbc_info[1].mac[1], pbc_info[1].mac[2],
			pbc_info[1].mac[3], pbc_info[1].mac[4], pbc_info[1].mac[5],
			pbc_info[1].last_time, time));
	return PBC_sta;
}

void
wps_pb_update_pushtime(unsigned char *mac, uint8 *uuid)
{
	int i;
	unsigned long now;

	(void) time((time_t*)&now);

	wps_pb_check_pushtime(now);

	for (i = 0; i < PBC_OVERLAP_CNT; i++) {
		if (memcmp(mac, pbc_info[i].mac, 6) == 0) {
			/* Confirmed in PF #3 */
			/* Do update new time, see test case 4.2.13 */
			pbc_info[i].last_time = now;
			return;
		}
	}

	if (pbc_info[0].last_time <= pbc_info[1].last_time)
		i = 0;
	else
		i = 1;

	memcpy(pbc_info[i].mac, mac, 6);
	if (uuid)
		memcpy(pbc_info[i].uuid, uuid, SIZE_16_BYTES);
	else
		memset(pbc_info[i].uuid, 0, SIZE_16_BYTES);
	pbc_info[i].last_time = now;

	return;
}

void
wps_pb_get_uuids(uint8 *buf, int len)
{
	int i;

	if (buf == NULL)
		return;

	memset(buf, 0, len);
	for (i = 0; i < PBC_OVERLAP_CNT; i++) {
		if (len >= SIZE_16_BYTES) {
			memcpy(buf, pbc_info[i].uuid, SIZE_16_BYTES);
			buf += SIZE_16_BYTES;
			len -= SIZE_16_BYTES;
		}
	}
}

void
wps_pb_clear_sta(unsigned char *mac)
{
	int i;

	for (i = 0; i < PBC_OVERLAP_CNT; i++) {
		if (memcmp(mac, pbc_info[i].mac, 6) == 0) {
			memset(&pbc_info[i], 0, sizeof(PBC_STA_INFO));
			return;
		}
	}

	return;
}

static int
wps_pb_find_next()
{
	char *value, *next;
	int next_id;
	int max_id = wps_hal_led_wl_max();
	char tmp[100];
	char ifname[IFNAMSIZ];
	char *wlnames;

	int target_id = -1;
	int target_instance = 0;
	int i, imax;

	imax = wps_get_ess_num();

refind:
	for (i = 0; i < imax; i++) {
		sprintf(tmp, "ess%d_led_id", i);
		value = wps_get_conf(tmp);
		if (value == NULL)
			continue;

		next_id = atoi(value);
		if ((next_id > pb_last_led_id) && (next_id <= max_id)) {
			if ((target_id == -1) || (next_id < target_id)) {
				/* Save the candidate */
				target_id = next_id;
				target_instance = i;
			}
		}
	}

	/* A candidate found ? */
	if (target_id == -1) {
		pb_last_led_id = -1;
		goto refind;
	}

	pb_last_led_id = target_id;

	/* Take the first wl interface */
	sprintf(tmp, "ess%d_wlnames", target_instance);

	wlnames = wps_safe_get_conf(tmp);
	foreach(ifname, wlnames, next) {
		wps_strncpy(pb_ifname, ifname, sizeof(pb_ifname));
		break;
	}

	return target_id;
}

#ifdef BCMWPSAPSTA
static int
wps_pb_is_virtual_pressed()
{
	if (wps_virtual_pressed == WPS_LONG_BTNPRESS) {
		wps_virtual_pressed = WPS_NO_BTNPRESS;
		return WPS_LONG_BTNPRESS;
	}

	return WPS_NO_BTNPRESS;
}

static void
wps_pb_virtual_btn_pressed()
{
	wps_virtual_pressed = WPS_LONG_BTNPRESS;
}
#endif /* BCMWPSAPSTA */

static int
wps_pb_retrieve_event(int unit, char *wps_ifname, int wps_ifname_sz)
{
	int press;
	int event = WPSBTN_EVTI_NULL;
	int imax = wps_get_ess_num();
	char *custom_ifnames = NULL;

#ifdef BCMWPSAPSTA
	bool wps_pbc_apsta_enabled = FALSE;

	if (strcmp(wps_safe_get_conf("wps_pbc_apsta"), "enabled") == 0)
		wps_pbc_apsta_enabled = TRUE;
#endif // endif
	/*
	 * According to WPS Usability and Protocol Best Practices spec
	 * section 3.1 WPS Push Button Enrollee Addition sub-section 2.2.
	 * Skip using LAN led to indicate which ESS is going
	 * to run WPS when WPS ESS only have one
	 */
	press = wps_hal_btn_pressed();
#ifdef MULTIAP
	if (!strcmp(wps_ui_get_env("map_pbc_method"), "1")) {
		press = WPS_SHORT_BTNPRESS;
	}
#endif	/* MULTIAP */

#ifdef MULTIAP
	custom_ifnames = nvram_safe_get("wps_custom_ifnames");
#else
	custom_ifnames = wps_safe_get_conf("wps_custom_ifnames");
#endif	/* MULTIAP */

	/* When custom_ifnames are defined only short press is used for both selection
	 * as well as starting the wps on particular bss. Long press event is being ignored
	 * it can be used in future for some other purpose.
	 */
	if (custom_ifnames[0] != '\0') {
		if (press == WPS_SHORT_BTNPRESS) {
			find_next_in_list(custom_ifnames, pb_ifname, wps_ifname, wps_ifname_sz);
			wps_strncpy(pb_ifname, wps_ifname, sizeof(pb_ifname));

			return WPSBTN_EVTI_PUSH;
		} else {
			return WPSBTN_EVTI_NULL;
		}
	}

#ifdef BCMWPSAPSTA
	if (wps_pbc_apsta_enabled == TRUE && press == WPS_NO_BTNPRESS)
		press = wps_pb_is_virtual_pressed();
#endif // endif

	if ((imax == 1 ||
#ifdef BCMWPSAPSTA
	    wps_pbc_apsta_enabled == TRUE ||
#endif // endif
	    FALSE) &&
	    (press == WPS_LONG_BTNPRESS || press == WPS_SHORT_BTNPRESS)) {
		wps_pb_find_next();
		wps_pb_state = WPS_PB_STATE_CONFIRM;

#ifdef BCMWPSAPSTA
		if (wps_pbc_apsta_enabled == TRUE) {
			/*
			 * For apsta mode (wps_pbc_apsta enabled),
			 * wps_monitor decide which interface to run WPS when HW PBC pressed
			 */
			int i;
			for (i = 0; i < WPS_MAX_PBC_APSTA; i++) {
				if (wps_pbc_apsta_upstream_pushed[i]) {
					strncpy(wps_ifname,
						wps_safe_get_conf(wps_pbc_sta_ifnames[i].name),
						IFNAMSIZ);
					break;
				}
			}
			if (i == WPS_MAX_PBC_APSTA)
				strncpy(wps_ifname,
					wps_safe_get_conf(wps_pbc_ap_ifnames[0].name), IFNAMSIZ);

			TUTRACE((TUTRACE_INFO, "wps_pbc_apsta mode enabled, wps_ifname = %s \n",
				wps_ifname));
		}
		else
#endif /* BCMWPSAPSTA */
			strcpy(wps_ifname, pb_ifname);

		return WPSBTN_EVTI_PUSH;
	}

	switch (press) {
	case WPS_LONG_BTNPRESS:
		TUTRACE((TUTRACE_INFO, "Detected a wps LONG button-push\n"));

		if (pb_last_led_id == -1)
			wps_pb_find_next();

		/* clear LAN all leds first */
		if (wps_pb_state == WPS_PB_STATE_INIT)
			wps_led_wl_select_on();

		wps_led_wl_confirmed(pb_last_led_id);

		wps_pb_state = WPS_PB_STATE_CONFIRM;
		strcpy(wps_ifname, pb_ifname);

		event = WPSBTN_EVTI_PUSH;
		break;

	/* selecting */
	case WPS_SHORT_BTNPRESS:
		TUTRACE((TUTRACE_INFO, "Detected a wps SHORT button-push, "
			"wps_pb_state = %d\n", wps_pb_state));

		if (wps_pb_state == WPS_PB_STATE_INIT ||
			wps_pb_state == WPS_PB_STATE_SELECTING) {

			/* start bssid selecting, find next enabled bssid */
			/*
			 * NOTE: currently only support wireless unit 0
			 */
			wps_pb_find_next();

			/* clear LAN all leds when first time enter */
			if (wps_pb_state == WPS_PB_STATE_INIT)
				wps_led_wl_select_on();

			wps_led_wl_selecting(pb_last_led_id);

			/* get selecting time */
			wps_pb_state = WPS_PB_STATE_SELECTING;
			wps_pb_selecting_time = time(0);
		}
		break;

	default:
		break;
	}

	return event;
}

#ifdef BCMWPSAPSTA
/*
 * find an AP with PBC active or timeout.
 * Returns SSID and BSSID.
 * Note : when we join the SSID, the bssid of the AP might be different
 * than this bssid, in case of multiple AP in the ESS ...
 * Don't know what to do in that case if roaming is enabled ...
 */
int
wps_pb_find_pbc_ap(char * bssid, char *ssid, uint8 *wsec)
{
	int pbc_ret = PBC_NOT_FOUND;
	wps_ap_list_info_t *wpsaplist;

	/* caller has to add wps ie and rem wps ie by itself */
	wpsaplist = create_aplist_escan();
	if (wpsaplist) {
		wps_get_aplist(wpsaplist, wpsaplist);
		/* pbc timeout handled by caller */
		pbc_ret = wps_get_pbc_ap(wpsaplist, bssid, ssid,
			wsec, get_current_time(), true);
	}

	return pbc_ret;
}

static bool
wps_pb_apsta_HW_pressed()
{
	int pbc = WPS_UI_PBC_NONE;
	char *val;

	val = wps_ui_get_env("wps_pbc_method");
	if (val)
		pbc = atoi(val);

	/* Retrieve ENV */
	if (pbc ==  WPS_UI_PBC_HW)
		return TRUE;
	else
		return FALSE;
}

static void
wps_pb_apsta_scan(int session_opened)
{
	int find_pbc;
	unsigned long now;
	uint8 bssid[6];
	char ssid[SIZE_SSID_LENGTH] = "";
	uint8 wsec = 1;
	uint8 i, j, wps_sta_scan_no = 0;
	char prefer_sta_ifname[IFNAMSIZ];
	int prefer_sta_idx = 0;

	if (strcmp(wps_safe_get_conf("wps_pbc_apsta"), "enabled") == 0 &&
	    wps_pb_apsta_HW_pressed() == TRUE && session_opened == 0) {

		for (i = 0; i < WPS_MAX_PBC_APSTA; i++) {
			if (strlen(wps_pbc_sta_ifnames[i].ifname))
				wps_sta_scan_no++;
		}

		strncpy(prefer_sta_ifname, wps_safe_get_conf("wps_pbc_prefer_intf"), IFNAMSIZ);
		for (i = 0; i < WPS_MAX_PBC_APSTA; i++) {
			if (!strcmp(prefer_sta_ifname, wps_pbc_sta_ifnames[i].ifname)) {
				prefer_sta_idx = i;
				break;
			}
		}

		now = get_current_time();
		for (i = 0; i < wps_sta_scan_no; i++) {
			/* continue to find pbc ap or check associating status */
			wps_osl_set_ifname(wps_safe_get_conf(wps_pbc_sta_ifnames[i].name));

			switch (wps_enr_scan_state[i]) {
			/* continue to find pbc ap or check associating status */
			case WPS_FINDPBC_STATE_INIT:
				wps_wl_bss_config(wps_safe_get_conf(
					wps_safe_get_conf(wps_pbc_sta_ifnames[i].name)), 1);
				do_wps_escan();
				wps_enr_scan_state[i] = WPS_FINDPBC_STATE_SCANNING;
				break;

			case WPS_FINDPBC_STATE_SCANNING:
				/* keep checking scan results for 10 second and issue scan again */
				if ((now - wps_enr_scan_state_time[i]) > 10) {
					TUTRACE((TUTRACE_INFO,  "%s: do_wps_scan\n", __FUNCTION__));
					do_wps_escan();
					wps_enr_scan_state_time[i] = get_current_time();
				}
				else if (get_wps_escan_results() != NULL) {
					/* got scan results, check to find pbc ap state */
					wps_enr_scan_state[i] = WPS_FINDPBC_STATE_FINDINGAP;
				}
				break;

			case WPS_FINDPBC_STATE_FINDINGAP:
				/* find pbc ap */
				find_pbc = wps_pb_find_pbc_ap((char *)bssid, (char *)ssid, &wsec);
				if (find_pbc == PBC_OVERLAP) {
					TUTRACE((TUTRACE_INFO,  "%s: PBC_OVERLAP\n", __FUNCTION__));
					wps_enr_scan_state[i] = WPS_FINDPBC_STATE_INIT;
					wps_pbc_prefer_intf_delay_count = wps_pbc_prefer_intf_count_tmp;
					break;
				}
				if (find_pbc == PBC_FOUND_OK) {
					if ((i != prefer_sta_idx) &&
						(wps_pbc_prefer_intf_delay_count > 0))
					{
						TUTRACE((TUTRACE_INFO,  "%s: PBC_FOUND_OK, but not"
						"prefered intf. delay count %d...\n", __FUNCTION__,
						wps_pbc_prefer_intf_delay_count));
						wps_pbc_prefer_intf_delay_count--;
						break;
					}
					TUTRACE((TUTRACE_INFO,  "%s: PBC_FOUND_OK, i=%d\n",
						__FUNCTION__, i));

					/* 1. Switch wps_pbc_apsta_role to WPS_PBC_APSTA_ENR */
					wps_pbc_apsta_upstream_pushed[i] = TRUE;

					/* 2. Generate a virtual PBC pressed event */
					wps_pb_virtual_btn_pressed();

					for (j = 0; j < WPS_MAX_PBC_APSTA; j++) {
						wps_enr_scan_state[j] = WPS_FINDPBC_STATE_INIT;
					}
					wps_pbc_prefer_intf_delay_count = wps_pbc_prefer_intf_count_tmp;
					wps_enr_scan_state_time[i] = get_current_time();
					goto force_end;
				}
				else {
					wps_enr_scan_state[i] = WPS_FINDPBC_STATE_SCANNING;
				}
				break;
			}
		}
	}
force_end:
	return;
}
#endif /* BCMWPSAPSTA */

void
wps_pb_timeout(int session_opened)
{
	time_t curr_time;

	/* check timeout when in WPS_PB_STATE_SELECTING */
	if (wps_pb_state == WPS_PB_STATE_SELECTING) {
		curr_time = time(0);
		if (curr_time > (wps_pb_selecting_time+WPS_PB_SELECTING_MAX_TIMEOUT)) {
			/* Reset pb state to init because of timed-out */
			wps_pb_state_reset();
		}
	}

#ifdef BCMWPSAPSTA
	/* for apsta mode concurrent wps */
	wps_pb_apsta_scan(session_opened);
#endif /* BCMWPSAPSTA */
}

int
wps_pb_state_reset()
{
	/* reset wps_pb_state to INIT */
	pb_last_led_id = -1;

	wps_pb_state = WPS_PB_STATE_INIT;

	/* Back to NORMAL */
	wps_hal_led_wl_select_off();

	return 0;
}

wps_hndl_t *
wps_pb_check(char *buf, int *buflen)
{
	char wps_ifname[IFNAMSIZ] = {0};

	/* note: push button currently only support wireless unit 0 */
	/* user can use PBC to tigger wps_enr start */
	if (WPSBTN_EVTI_PUSH == wps_pb_retrieve_event(0, wps_ifname, sizeof(wps_ifname))) {
		int uilen = 0;

		uilen += sprintf(buf + uilen, "SET ");

		TUTRACE((TUTRACE_INFO, "%s :wps monitor: Button pressed!!\n", wps_ifname));

		wps_close_session();

		uilen += sprintf(buf + uilen, "wps_config_command=%d ", WPS_UI_CMD_START);
		uilen += sprintf(buf + uilen, "wps_ifname=%s ", wps_ifname);
		uilen += sprintf(buf + uilen, "wps_method=%d ", WPS_UI_METHOD_PBC);
		uilen += sprintf(buf + uilen, "wps_pbc_method=%d ", WPS_UI_PBC_HW);

		wps_setProcessStates(WPS_ASSOCIATED);

		/* for wps_enr application, start it right now */
		if (wps_is_wps_sta(wps_ifname)) {
			uilen += sprintf(buf + uilen, "wps_action=%d ", WPS_UI_ACT_ENROLL);
		}
		/* for wps_ap application */
		else {
			uilen += sprintf(buf + uilen, "wps_action=%d ", WPS_UI_ACT_ADDENROLLEE);
		}

		uilen += sprintf(buf + uilen, "wps_sta_pin=00000000 ");

		*buflen = uilen;
		return &pb_hndl;
	}

	return NULL;
}

void
wps_pb_reset()
{
#ifdef BCMWPSAPSTA
	int i;
#endif // endif
	memset(pbc_info, 0, sizeof(pbc_info));

#ifdef BCMWPSAPSTA
	for (i = 0; i < WPS_MAX_PBC_APSTA; i++) {
		wps_pbc_apsta_upstream_pushed[i] = FALSE;
		wps_enr_scan_state_time[i] = 0;
		wps_enr_scan_state[i] = WPS_FINDPBC_STATE_INIT;
		wps_wl_bss_config(wps_safe_get_conf(wps_pbc_sta_ifnames[i].name), 0);
	}
#endif // endif
}

int
wps_pb_init()
{
#ifdef BCMWPSAPSTA
	int i;
#endif // endif
	memset(pbc_info, 0, sizeof(pbc_info));

	memset(&pb_hndl, 0, sizeof(pb_hndl));
	pb_hndl.type = WPS_RECEIVE_PKT_PB;
	pb_hndl.handle = -1;
#ifdef BCMWPSAPSTA
	if (wps_safe_get_conf("wps_pbc_prefer_intf_count") != "")
	{
		wps_pbc_prefer_intf_count_tmp = atoi(wps_safe_get_conf("wps_pbc_prefer_intf_count"));
		TUTRACE((TUTRACE_INFO, "%s: nvram override delay count to %d...\n",
			__FUNCTION__, wps_pbc_prefer_intf_count_tmp));
	}
	else
		wps_pbc_prefer_intf_count_tmp = WPS_PBC_PREFER_INTF_COUNT; /* default */

	wps_pbc_prefer_intf_delay_count = wps_pbc_prefer_intf_count_tmp;

	for (i = 0; i < WPS_MAX_PBC_APSTA; i++) {
		wps_pbc_apsta_upstream_pushed[i] = FALSE;
		wps_enr_scan_state_time[i] = 0;
		wps_enr_scan_state[i] = WPS_FINDPBC_STATE_INIT;
	}
#endif // endif
	return (wps_hal_btn_init());
}

int
wps_pb_deinit()
{
	wps_pb_state_reset();

	wps_hal_btn_cleanup();

	return 0;
}

/* Interface names listed in nvram wps_custom_ifnames are used for selection as well as
 * starting the wps session on the selected interface.
 * The selected ifname gets copied into pb_ifname variable which is used for selcting
 * the next ifname from the ifnames list present in nvram in subsequent pbc press.
 * After successful wps session or when wps session times out pb_ifname needs to be cleared.
 * So that the next pbc press selects the first ifname listed in the nvram.
 */
void
wps_pb_ifname_reset()
{
	TUTRACE((TUTRACE_INFO, "Current pbc ifname[%s] cleared \n", pb_ifname));
	memset(pb_ifname, 0, sizeof(pb_ifname));
}
