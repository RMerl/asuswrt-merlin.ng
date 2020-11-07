/*
 * WPS monitor
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
 * $Id: wps_monitor.c 772297 2019-02-20 06:50:21Z $
 */
#include <stdio.h>
#include <unistd.h>
#include <errno.h>

#include <wpsheaders.h>
#include <wpscommon.h>

#include <tutrace.h>
#include <shutils.h>
#include <wps_ap.h>
#include <wps_enr.h>
#include <wps_wps.h>
#include <wps_ie.h>
#include <wps_ui.h>
#ifdef WPS_UPNP_DEVICE
#include <ap_upnp_sm.h>
#include <wps_upnp.h>
#endif // endif
#ifdef WPS_NFC_DEVICE
#include <wps_nfc.h>
#endif // endif
#include <wps_eap.h>
#include <wps_led.h>
#include <wps_pb.h>
#include <wps_wl.h>
#include <wps_aplockdown.h>
#include <wps_enrapi.h>
#ifdef BCMWPSAPSTA
#include <wps_sta.h>
#endif // endif

#ifdef __CONFIG_WFI__
#include <wps_wfi.h>
#endif /* __CONFIG_WFI__ */

#if defined(MULTIAP)
#include <wps_1905.h>
#endif // endif

extern void wps_setProcessStates(int state);
extern int wps_getProcessStates();

static int wps_flags;
static char wps_readbuf[WPS_EAPD_READ_MAX_LEN];
static int wps_buflen;
static unsigned char wps_uuid[16];
static int wps_ess_num = 0;

static wps_app_t s_wps_app = {0};
static wps_app_t *wps_app = &s_wps_app;

static int wps_conf_num;
static char **wps_conf_list;

static wps_hndl_t *wps_hndl_head = NULL;

#ifdef WPS_ADDCLIENT_WWTP
/* WSC 2.0,
 * One way to define the use of a particular PIN is that it should be valid for the whole
 * Walking Time period.  In other words, as long as the registration is not successful
 * and the process hasn't timed out and no new PIN has been entered, the AP should
 * accept to engage in a new registration process with the same PIN.
 * On some implementation, this is what happens on the enrollee side when it fails
 * registration with one of several AP and keep trying with all of them (actually,
 * I think we should implement this as well in our enrollee code).
 *
 * In our case, that would mean that :
 * - the AP should detect the NACK from the enrollee after failure on M4 (which I am
 * not sure it does) and close the session.
 * - as long as the PIN is valid, the AP should restart its state machine after failure,
 * keep the same PIN,  and answer an M1 with M2 instead of M2D.
 */
static char wps_SET_buf[WPS_EAPD_READ_MAX_LEN];
static int wps_SET_buflen;
static wps_hndl_t *wps_ui_hndl = NULL;
static int wps_addclient_2min = 0;
static int wps_addclient_states = WPS_INIT;

#define ADDCLINET_WINDOW_ENABLED()	(wps_addclient_2min && wps_ui_hndl)
#define ADDCLIENT_DONE_SUCCESS()	(wps_ui_hndl = NULL)
#endif /* WPS_ADDCLIENT_WWTP */

unsigned char *wps_get_uuid()
{
	return wps_uuid;
}

int wps_get_ess_num()
{
	return wps_ess_num;
}

wps_app_t *
get_wps_app()
{
	return wps_app;
}

/* WPS packet handle functions */
void
wps_hndl_add(wps_hndl_t *hndl)
{
	wps_hndl_t *node = wps_hndl_head;

	/* Check duplicate */
	while (node) {
		if (node == hndl)
			return;

		node = node->next;
	}

	/* Do prepend */
	hndl->next = wps_hndl_head;
	wps_hndl_head = hndl;

	return;
}

void
wps_hndl_del(wps_hndl_t *hndl)
{
	wps_hndl_t *temp, *prev;

	/* Search match */
	prev = NULL;
	temp = wps_hndl_head;
	while (temp) {
		if (temp == hndl) {
			/* Dequeue */
			if (prev == NULL)
				wps_hndl_head = wps_hndl_head->next;
			else
				prev->next = temp->next;

			return;

		}

		prev = temp;
		temp = temp->next;
	}

	return;
}

#ifdef WPS_ADDCLIENT_WWTP
void
wps_close_addclient_window()
{
	wps_addclient_2min = 0;
}

void
wps_enable_addclient_window()
{
	wps_addclient_2min = 1;
	wps_ui_wer_override_active(TRUE);
}

void
wps_disable_addclient_window()
{
	wps_addclient_2min = 0;
}

static void
wps_set_addclient_states(int states)
{
	wps_addclient_states = states;
}

static int
wps_get_addclient_states()
{
	return wps_addclient_states;
}
#endif /* WPS_ADDCLIENT_WWTP */

void
wps_close_session()
{
	TUTRACE((TUTRACE_INFO, "Session closed!!\n"));

	wps_ui_reset_env();
	wps_ui_clear_pending();
	wps_pb_reset();
#ifdef WPS_UPNP_DEVICE
	wps_upnp_clear_ssr();
#endif // endif

	if (wps_app->wksp) {
		(*wps_app->close)(wps_app->wksp);

		wps_app->wksp = 0;
		wps_app->close = 0;
		wps_app->process = 0;
		wps_app->check_timeout = 0;
		wps_app->sc_mode = 0;
#if defined(MULTIAP)
		wps_app->map_timeout = 0;
#endif	/* MULTIAP */
	}

	wps_ie_set(NULL, NULL);
}

int
wps_deauthenticate(unsigned char *bssid, unsigned char *sta_mac, int reason)
{
	int i, imax;
	char ifname[16];
	char tmp[256], prefix[] = "wlXXXXXXXXXX_";
	char *wlnames;
	char *next = NULL;
	char wl_mac[SIZE_6_BYTES];
	char *wl_hwaddr = NULL, *wl_mode = NULL;

	/* Check mac */
	imax = wps_get_ess_num();
	for (i = 0; i < imax; i++) {
		sprintf(tmp, "ess%d_wlnames", i);
		wlnames = wps_safe_get_conf(tmp);
		if (strlen(wlnames) == 0)
			continue;

		foreach(ifname, wlnames, next) {
			snprintf(prefix, sizeof(prefix), "%s_", ifname);
			wl_mode = wps_get_conf(strcat_r(prefix, "mode", tmp));
			if (!wl_mode || strcmp(wl_mode, "ap") != 0)
				continue; /* Only for AP mode */

			/* Get configured ethernet MAC address */
			snprintf(prefix, sizeof(prefix), "%s_", ifname);
			wl_hwaddr = wps_get_conf(strcat_r(prefix, "hwaddr", tmp));
			if (wl_hwaddr == NULL)
				continue;

			ether_atoe(wl_hwaddr, (unsigned char*)wl_mac);
			if (memcmp(wl_mac, bssid, 6) == 0)
				goto found;
			else
				wl_hwaddr = NULL;
		}
	}

found:
	if (wl_hwaddr == NULL)
		return -1;

	TUTRACE((TUTRACE_INFO, "%s\n", ifname));

	/* Issue wl driver deauth */
	return wps_wl_deauthenticate(ifname, sta_mac, reason);
}

#ifdef BCMWPSAPSTA
int wps_escan_timeout_handler(unsigned int timout_state)
{
	static struct timeval start_time;
	struct timeval end_time;
	int time_diff = 0;
	int ret = WPS_ESCAN_NOT_STARTED;

	if (get_wps_escan_state() != WPS_ESCAN_INPROGRESS) {
		return ret;
	}

	switch (timout_state) {
		case WPS_ESCAN_STARTED:
			/* Start time in usec */
			gettimeofday(&start_time, NULL);
			ret = WPS_ESCAN_INPROGRESS;
			break;
		case WPS_ESCAN_CHECK_TIMEOUT:
			/* End time in usec */
			gettimeofday(&end_time, NULL);
			time_diff = (end_time.tv_sec - start_time.tv_sec);

			if (time_diff > ESCAN_TIMER_INTERVAL_S) {
				wps_eap_reset_scan_result();
				ret = WPS_ESCAN_TIMEDOUT;
			} else {
				ret = WPS_ESCAN_INPROGRESS;
			}
			break;
		default:
			TUTRACE((TUTRACE_ERR, "unknown Timeout State\n"));
		}
		return ret;
}

#endif /* BCNWPSAPSTA */
static int
wps_process_msg(char *buf, int buflen, wps_hndl_t *hndl)
{
	int ret = WPS_CONT;

	if (!hndl)
		return ret;

	/* check buffer type */
	switch (hndl->type) {
	case WPS_RECEIVE_PKT_UI:
	case WPS_RECEIVE_PKT_PB:
#ifdef WPS_ADDCLIENT_WWTP
		/* backup SET command */
		if (wps_ui_is_SET_cmd(buf, buflen)) {
			wps_SET_buflen = buflen;
			memcpy(wps_SET_buf, buf, wps_SET_buflen);
			wps_ui_hndl = hndl;
		}
#endif /* WPS_ADDCLIENT_WWTP */
		/* from ui */
		ret = wps_ui_process_msg(buf, buflen);
		break;

	case WPS_RECEIVE_PKT_EAP:
		/* from eapd */
		ret = wps_eap_process_msg(buf, buflen);
		break;

#ifdef WPS_UPNP_DEVICE
	case WPS_RECEIVE_PKT_UPNP:
		/* from upnp */
		ret = wps_upnp_process_msg(buf, buflen);
		break;
#endif // endif

#ifdef WPS_NFC_DEVICE
	case WPS_RECEIVE_PKT_NFC:
		/* from upnp */
		ret = wps_nfc_process_msg(buf, buflen);
		break;
#endif // endif

#if defined(MULTIAP)
	case WPS_RECEIVE_PKT_1905:
		ret = wps_1905_process_msg(buf, buflen);
		break;
#endif // endif
	default:
		break;
	}

#ifdef BCMWPSAPSTA
	if (get_wps_escan_state() == WPS_ESCAN_INPROGRESS)
		wps_escan_timeout_handler(WPS_ESCAN_CHECK_TIMEOUT);
#endif // endif
	return ret;
}

static wps_hndl_t *
wps_check_fd(char *buf, int *buflen)
{
	wps_hndl_t *hndl;

#ifdef WPS_ADDCLIENT_WWTP
	if (ADDCLINET_WINDOW_ENABLED()) {
		*buflen = wps_SET_buflen;
		memcpy(buf, wps_SET_buf, wps_SET_buflen);

		wps_disable_addclient_window();
		return wps_ui_hndl;
	}
#endif /* WPS_ADDCLIENT_WWTP */

	/* Forge ui_hndl for PBC */
	hndl = wps_pb_check(buf, buflen);
#ifdef WPS_NFC_DEVICE
	if (!hndl)
		hndl = wps_nfc_check(buf, buflen);
#endif // endif
	if (!hndl)
		hndl = wps_osl_wait_for_all_packets(buf, buflen, wps_hndl_head);

	return hndl;
}

bool
wps_is_wps_sta(char *wps_ifname)
{
	char tmp[100];
	char *wl_mode;

	if (wps_ifname == NULL)
		return false;

	sprintf(tmp, "%s_mode", wps_ifname);
	wl_mode = wps_safe_get_conf(tmp);
	if (strcmp(wl_mode, "ap") != 0)
		return true;
	else
		return false;
}

static int
wps_open()
{
	char *value;

	/* clear variables */
	wps_flags = 0;

#ifdef WPS_UPNP_DEVICE
	wps_upnp_device_uuid(wps_uuid);
#endif // endif

	value = wps_get_conf("wps_ess_num");
	if (!value || (wps_ess_num = atoi(value)) == 0) {
		TUTRACE((TUTRACE_INFO, "No ESS found\n"));
		return -1;
	}

	/* Init push button */
	if (wps_pb_init() == -1) {
		TUTRACE((TUTRACE_INFO, "No wireless interfaces found\n"));
		return -1;
	}

	wps_custom_init();

	/* Init wps led */
	wps_led_init();

	/* Init ap lock down log function */
	wps_aplockdown_init();

	/* Init packet handler */
	wps_hndl_head = 0;

	wps_eap_init();

	wps_ui_init();

#if defined(MULTIAP)
	wps_1905_init();
#endif // endif

#if defined(BCMWPSAP) && defined(__CONFIG_WFI__)
	wps_wfi_init();
#endif /* BCMWPSAP && __CONFIG_WFI__ */

	wps_ie_set(NULL, NULL);

#ifdef WPS_UPNP_DEVICE
	/* init upnp */
	wps_upnp_init();
#endif // endif

#ifdef WPS_NFC_DEVICE
	/* init nfc */
	wps_nfc_init();
#endif // endif

#ifdef BCMWPSAPSTA
	wps_wl_bss_config(wps_safe_get_conf("wps_pbc_sta_ifname"), 0);
#endif // endif

	return 0;
}

static int
wps_close()
{
	/* clean up app workspace */
	wps_close_session();

	wps_ie_clear();

	/* free allocated AP Lock down buffer */
	wps_aplockdown_cleanup();

	/* disconnect push button */
	wps_pb_deinit();

	/* disconnect led */
	wps_led_deinit();

	/* close bind socket */
	wps_eap_deinit();

	wps_ui_deinit();

#if defined(MULTIAP)
	wps_1905_deinit();
#endif // endif

#ifdef WPS_UPNP_DEVICE
	/* detach upnp */
	wps_upnp_deinit();
#endif // endif

#ifdef WPS_NFC_DEVICE
	/* detach nfc */
	wps_nfc_deinit();
#endif // endif

#if defined(BCMWPSAP) && defined(__CONFIG_WFI__)
	wps_wfi_cleanup();
#endif /* BCMWPSAP && __CONFIG_WFI__ */

	wps_custom_deinit();
	return 0;
}

/* Periodically timed-out check */
static void
wps_timeout()
{
	int states = WPS_INIT;

	if (wps_ui_pending_expire()) {

#ifdef WPS_ADDCLIENT_WWTP
		/* whole walking time period timeout */
		wps_close_addclient_window();

		/* use addclient states if it has been failed and timeout happen */
		states = wps_get_addclient_states();

		/* reset addclient states */
		wps_set_addclient_states(WPS_INIT);

		/* Clear the buffered UI handle */
		wps_ui_hndl = NULL;
#endif /* WPS_ADDCLIENT_WWTP */

		/* state maching in progress, check stop or push button here */
		wps_close_session();
		wps_setProcessStates(states);

		/*
		 * set wps LAN all leds to normal and wps_pb_state
		 * when session timeout or user stop
		 */
		wps_pb_state_reset();
		wps_pb_ifname_reset();
	}

#ifdef WPS_UPNP_DEVICE
	if (wps_upnp_ssr_expire()) {
		wps_upnp_clear_ssr();
		wps_ie_set(NULL, NULL);
	}
#endif /* WPS_UPNP_DEVICE */

	/* do led status update */
	wps_pb_timeout((int)wps_app->wksp);

	wps_led_update();

	/* do ap lock down ageout check */
	wps_aplockdown_check();

#if defined(BCMWPSAP) && defined(__CONFIG_WFI__)
	/* Check Wifi-Invite activity */
	wps_wfi_check();
#endif /* BCMWPSAP && __CONFIG_WFI__ */

#ifdef WPS_NFC_DEVICE
	wps_nfc_timeout();
#endif // endif
}

/* WPS message dispatch */
static int
wps_dispatch()
{
	int ret;
	int sc_mode;
	wps_hndl_t *hndl;

	wps_buflen = sizeof(wps_readbuf);

	memset(&wps_readbuf, 0, sizeof(wps_readbuf));

	/* 2. check packet from Eapd, UPnP and Wpsap, and do proper action */
	hndl = wps_check_fd(wps_readbuf, &wps_buflen);

	/* 3. process received buffer */
	ret = wps_process_msg(wps_readbuf, wps_buflen, hndl);

	/* 4. if app exist, check app timeout */
	if (wps_app->wksp && ret == WPS_CONT)
		ret = (*wps_app->check_timeout)(wps_app->wksp);

#if defined(MULTIAP)
	/* Handle multiap wifi onboarding case */
	if (wps_app->wksp && ret == WPS_CONT) {
		int map_ret = WPS_CONT;
		if (wps_app->map_timeout) {
			map_ret = (*wps_app->map_timeout)(wps_app->wksp);
		}

		if (map_ret == MAP_TIMEOUT) {
			/* Multiap timeout has occured start WPS session in another interface */
			TUTRACE((TUTRACE_ERR, "MultiAP timeout occured retry  WPS session!\n"));
			wps_setProcessStates(WPS_MAP_TIMEOUT);
			wps_ui_map_try_pbc();
		}
	}
#endif	/* MULTIAP */

	/* 5. check wps application process result */
	if (wps_app->wksp) {
		if (ret != WPS_CONT) {
			TUTRACE((TUTRACE_INFO, "ret value = %d, wps_app ->sc_mode = %d \n",
				ret, wps_app->sc_mode));

#ifdef WPS_ADDCLIENT_WWTP
			/* Clear wps_ui_hndl */
			if (ret == WPS_RESULT_SUCCESS ||
			    ret == WPS_RESULT_SUCCESS_RESTART) {
				ADDCLIENT_DONE_SUCCESS();
			}
#endif /* WPS_ADDCLIENT_WWTP */

			/* wps_close_session will clean up 'mode' now */
			sc_mode = wps_app->sc_mode;

			/* save which mode is success */
			wps_setWPSSuccessMode(sc_mode);

			/* wps operation completed. close session */
			wps_close_session();

			/* wpsap finished, set LAN leds to NORMAL */
			wps_pb_state_reset();

			switch (sc_mode) {
			case SCMODE_STA_ENROLLEE:
			case SCMODE_STA_REGISTRAR:
				if (ret == WPS_RESULT_SUCCESS_RESTART) {
					/* get configuration successful */
					wps_flags = WPSM_WKSP_FLAG_SUCCESS_RESTART;
					return 0;
				}
				break;

			case SCMODE_AP_ENROLLEE:
				if (ret == WPS_RESULT_ENROLLMENT_PINFAIL) {
					wps_aplockdown_add();
				}
#ifdef WPS_ADDCLIENT_WWTP
				else if (ret == WPS_RESULT_ENROLLMENT_M2DFAIL &&
					!strcmp(wps_safe_get_conf("wps_wer_override"), "1")) {
					/* re-open previous session */
					TUTRACE((TUTRACE_ERR,
						"AP enrollment failed, continue "
						"adding enrollee until Whole Walking Time "
						"Period timeout!\n"));
					wps_enable_addclient_window();

					/* reset addclient states */
					wps_set_addclient_states(WPS_INIT);
				}
#endif /* WPS_ADDCLIENT_WWTP */
				else if (ret == WPS_RESULT_SUCCESS_RESTART) {
					wps_flags = WPSM_WKSP_FLAG_SUCCESS_RESTART;
					return 0;
				}
				/* Do nothing */
				break;

			case SCMODE_AP_REGISTRAR:
				if (WPS_IS_PROXY(sc_mode)) {
					/* Process M2D return case */
					break;
				}

				/* Process return code */
				if (ret == WPS_RESULT_SUCCESS_RESTART) {
					/*
					 * wps_flags must have WPSM_WKSP_FLAG_SET_RESTART
					 * flag to indicate the start_wps() skip to reset
					 * wps_proc_status.
					 * In the case of whent Add Enrolle in Unconfiged
					 * State.
					 */
					wps_flags = WPSM_WKSP_FLAG_SUCCESS_RESTART;
					return 0;
				}
#ifdef WPS_ADDCLIENT_WWTP
				else if ((ret == WPS_RESULT_REGISTRATION_PINFAIL ||
#ifdef WPS_NFC_DEVICE
					ret == WPS_RESULT_REGISTRATION_CHOFAIL ||
#endif // endif
					FALSE) &&
					strcmp(wps_ui_get_env("wps_pinfail_state"), "M6") != 0) {
					/* set add client window open again
					 * only when pinfail in first helf
					 */
					TUTRACE((TUTRACE_ERR,
						"Adding enrollee PIN failure, continue "
						"adding enrollee until Whole Walking Time "
						"Period timeout!\n"));
					wps_enable_addclient_window();

					/* remember the ping fail status */
#if defined(WPS_NFC_DEVICE)
					if (ret == WPS_RESULT_REGISTRATION_CHOFAIL)
						wps_set_addclient_states(wps_getProcessStates());
					else
#endif // endif
						wps_set_addclient_states(WPS_MSG_ERR);
				}
				else {
					/* don't keep opening the windows in other case */
					/* reset the start adding client time */
					wps_ui_close_addclient_window();

					/* reset addclient states */
					wps_set_addclient_states(WPS_INIT);
				}
#endif /* WPS_ADDCLIENT_WWTP */
				break;
			default:
				/* Do nothing */
				break;
			}
		}
	}
	else if (ret == WPS_ERR_OPEN_SESSION) {
		TUTRACE((TUTRACE_ERR, "Session open failed\n"));

		/* wps operation completed. close session */
		wps_close_session();

		/* wpsap finished, set LAN leds to NORMAL */
		wps_pb_state_reset();
	}
#ifdef WPS_NFC_DEVICE
	else if (ret == NFC_RD_CFG_SUCCESS_RESTART || ret == NFC_WR_CFG_SUCCESS_RESTART) {
		wps_flags = WPSM_WKSP_FLAG_SUCCESS_RESTART;
		wps_ui_reset_env();
	}
	else if (ret == NFC_WR_PW_SUCCESS || ret == NFC_RD_PW_SUCCESS ||
		ret == NFC_CHO_S_SUCCESS || ret == NFC_CHO_R_SUCCESS) {
		/* Lanuch a WPS session :
		 * NFC_WR_PW_SUCCESS
		 *    - AP do "NFC Config AP" / STA do NFC "Enroll"
		 * NFC_RD_PW_SUCCESS
		 *    - AP do "NFC Add Enrollee" / STA do NFC "Config AP" or "Get AP Config"
		 * NFC_CHO_S_SUCCESS/NFC_CHO_R_SUCCESS
		 *    - Do CHO Selector/Requester
		 */
		TUTRACE((TUTRACE_ERR, "NFC lanuch a WPS session!\n"));
		wps_ui_nfc_open_session();
	}
#ifdef WPS_ADDCLIENT_WWTP
	else if (ret == NFC_RW_RETRY) {
		/* PF#4: WAR for type 3 tag,
		 * continues to do NFC RW operation until timeout or success.
		 */
		WpsSleep(1);

		/* re-open previous session */
		wps_enable_addclient_window();

		/* reset addclient states */
		wps_set_addclient_states(WPS_INIT);
	}
#endif /* WPS_ADDCLIENT_WWTP */
	else if (ret == NFC_ERROR || ret == NFC_WR_CFG_SUCCESS || ret == NFC_FORMAT_SUCCESS) {
		wps_ui_reset_env();
	}
#endif /* WPS_NFC_DEVICE */

	return 0;
}

char *
wps_get_conf(char *name)
{
	int i;
	char *p, **conf, *val = NULL;

	conf = wps_conf_list;

	for (i = 0; i < wps_conf_num; i++, conf++) {
		p = strstr(*conf, "=");
		if (p) {
			if (!strncmp(*conf, name, strlen(name))) {
				val = (p+1);
				break;
			}
		}
	}
	return val;
}

char *
wps_safe_get_conf(char *name)
{
	char *value = wps_get_conf(name);

	if (value == 0)
		value = "";

	return value;
}

int wps_set_conf(char *name, char *value)
{
	return wps_osl_set_conf(name, value);
}

/* WPS mainloop */
int
wps_mainloop(int num, char **list)
{
	wps_conf_list = list;
	wps_conf_num = num;

	/* init */
	if (wps_open() < 0) {
		TUTRACE((TUTRACE_ERR, "WPSM work space initial failed.\n"));
		return 0;
	}

	/* main loop to dispatch message */
	while (1) {
		/* do packets dispatch */
		wps_dispatch();

		/* check flag for shutdown */
		if (wps_flags & WPSM_WKSP_FLAG_SHUTDOWN) {
			wps_close();
			break;
		}

		/* check timeout */
		wps_timeout();
	}

	return wps_flags;
}

/* termination handler, call by osl main function */
void
wps_stophandler(int sig)
{
	wps_flags |= WPSM_WKSP_FLAG_SHUTDOWN;
#ifdef BCA_CPEROUTER
	/* prevent the file not killed in time for next restart */
	kill_wps_pid_file();
#endif // endif
	return;
}

/* restart handler, call by osl main function */
void
wps_restarthandler(int sig)
{
	wps_flags |= WPSM_WKSP_FLAG_SET_RESTART | WPSM_WKSP_FLAG_SHUTDOWN;
#ifdef BCA_CPEROUTER
	/* prevent the file not killed in time for next restart */
	kill_wps_pid_file();
#endif // endif
	return;
}

#if defined(MULTIAP)
/* Initialize loopback socket with eapd */
int
wps_map_eap_init()
{
	int ret = 0;

	if (wps_eap_init() < 0) {
		TUTRACE((TUTRACE_ERR, "WPS EAPD init failed \n"));
		ret = -1;
	}

	return ret;
}

/* Close loopback socket with eapd */
void
wps_map_eap_dinit()
{
	wps_eap_deinit();
}

/* Handles the escan event received from eapd */
int
wps_map_escan_handler()
{
	int ret;
	wps_hndl_t *hndl;

	wps_buflen = sizeof(wps_readbuf);

	memset(&wps_readbuf, 0, sizeof(wps_readbuf));

	hndl = wps_osl_wait_for_all_packets(wps_readbuf, &wps_buflen, wps_hndl_head);

	ret = wps_process_msg(wps_readbuf, wps_buflen, hndl);

	return ret;
}
#endif	/* MULTIAP */

void
wps_conf_upd(int num, char **list)
{
	TUTRACE((TUTRACE_ERR, "Update wps conf!\n"));
	wps_conf_list = list;
	wps_conf_num = num;
}
