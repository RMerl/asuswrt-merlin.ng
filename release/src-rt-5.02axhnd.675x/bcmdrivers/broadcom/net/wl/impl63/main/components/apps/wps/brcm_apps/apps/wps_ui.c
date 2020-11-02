/*
 * WPS ui
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
 * $Id: wps_ui.c 770390 2018-12-13 19:07:04Z $
 */

#include <stdio.h>
#include <errno.h>
#include <tutrace.h>
#include <time.h>
#include <wps_wl.h>
#include <wps_ui.h>
#include <shutils.h>
#include <wlif_utils.h>
#include <wps_ap.h>
#include <wps_enr.h>
#include <security_ipc.h>
#include <wps_wps.h>
#include <wps_pb.h>
#include <wps_led.h>
#include <wps_upnp.h>
#include <wps_ie.h>
#include <ap_ssr.h>
#include <wps_apputils.h>
#ifdef WPS_NFC_DEVICE
#include <wps_nfc.h>
#endif // endif

extern void wps_setProcessStates(int state);

typedef struct {
	uint32 value;
	const char *string;
} wps_ui_str_t;

/* wps_config_command */
static wps_ui_str_t _wps_ui_cmd_str[] = {
	{WPS_UI_CMD_NONE,	"None"},
	{WPS_UI_CMD_START,	"Start"},
	{WPS_UI_CMD_STOP,	"Stop"},
	{WPS_UI_CMD_NFC_WR_CFG,	"NFC Write Configuration"},
	{WPS_UI_CMD_NFC_RD_CFG,	"NFC Read Configuration"},
	{WPS_UI_CMD_NFC_WR_PW,	"NFC Write Password"},
	{WPS_UI_CMD_NFC_RD_PW,	"NFC Read Password"},
	{WPS_UI_CMD_NFC_HO_S,	"NFC Hand Over Selector"},
	{WPS_UI_CMD_NFC_HO_R,	"NFC Hand Over Rqquester"},
	{WPS_UI_CMD_NFC_FM,	"NFC Format"},
	{WPS_UI_CMD_MSGLEVEL,	"WPS msglevel"},
	{0,		NULL}
};

/* wps_action */
static wps_ui_str_t _wps_ui_action_str[] = {
	{WPS_UI_ACT_NONE,		"None"},
	{WPS_UI_ACT_ENROLL,		"Enroll"},
	{WPS_UI_ACT_CONFIGAP,		"Config AP"},
	{WPS_UI_ACT_ADDENROLLEE,	"Add Enrollee"},
	{WPS_UI_ACT_STA_CONFIGAP,	"STA Config AP"},
	{WPS_UI_ACT_STA_GETAPCONFIG,	"STA Get AP Config"},
	{0,				NULL}
};

/* wps_method */
static wps_ui_str_t _wps_ui_method_str[] = {
	{WPS_UI_METHOD_NONE,	"NONE"},
	{WPS_UI_METHOD_PIN,	"PIN"},
	{WPS_UI_METHOD_PBC,	"PBC"},
	{WPS_UI_METHOD_NFC_PW,	"NFC_PW"},
	{WPS_UI_METHOD_NFC_CHO,	"NFC_CHO"},
	{0,			NULL}
};

/* wps_pbc_method */
static wps_ui_str_t _wps_ui_pbc_str[] = {
	{WPS_UI_PBC_NONE,	"NONE"},
	{WPS_UI_PBC_HW,		"HW"},
	{WPS_UI_PBC_SW,		"SW"},
	{0,			NULL}
};

typedef struct {
	int wps_config_command;	/* Config */
	int wps_method;
	int wps_action;
	int wps_pbc_method;
	char wps_ifname[16];
	char wps_sta_pin[SIZE_64_BYTES];
	char wps_ssid[SIZE_32_BYTES+1];
	char wps_akm[SIZE_32_BYTES];
	char wps_crypto[SIZE_32_BYTES];
	char wps_psk[SIZE_64_BYTES+1];
	int wps_aplockdown;
	int wps_proc_status;	/* Status report */
	int wps_pinfail;
	char wps_pinfail_state[32];
	char wps_sta_mac[sizeof("00:00:00:00:00:00")];
	char wps_autho_sta_mac[sizeof("00:00:00:00:00:00")]; /* WSC 2.0 */
	char wps_sta_devname[32];
	char wps_enr_wsec[SIZE_32_BYTES];
	char wps_enr_ssid[SIZE_32_BYTES+1];
	char wps_enr_bssid[SIZE_32_BYTES];
	int wps_enr_scan;
#ifdef __CONFIG_WFI__
	char wps_device_pin[12]; /* For WiFi-Invite session PIN */
#endif /* __CONFIG_WFI__ */
	char wps_stareg_ap_pin[SIZE_64_BYTES];
	char wps_scstate[32];
#ifdef WPS_NFC_DEVICE
	int wps_nfc_dm_status;
	int wps_nfc_err_code;
#endif // endif
	unsigned int wps_msglevel;

#ifdef MULTIAP
	int map_pbc_method;
#endif // endif
} wps_ui_t;

static wps_hndl_t ui_hndl;
static int pending_flag = 0;
static unsigned long pending_time = 0;

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
static unsigned long addclient_window_start = 0;
static bool wps_wer_override_active = FALSE;
#endif /* WPS_ADDCLIENT_WWTP */

static wps_ui_t s_wps_ui;
static wps_ui_t *wps_ui = &s_wps_ui;

char *
wps_ui_get_env(char *name)
{
	static char buf[32];

	if (!strcmp(name, "wps_config_command")) {
		sprintf(buf, "%d", wps_ui->wps_config_command);
		return buf;
	}

	if (!strcmp(name, "wps_action")) {
		sprintf(buf, "%d", wps_ui->wps_action);
		return buf;
	}

	if (!strcmp(name, "wps_method")) {
		sprintf(buf, "%d", wps_ui->wps_method);
		return buf;
	}

	if (!strcmp(name, "wps_pbc_method")) {
		sprintf(buf, "%d", wps_ui->wps_pbc_method);
		return buf;
	}

	if (!strcmp(name, "wps_ssid"))
		return wps_ui->wps_ssid;

	if (!strcmp(name, "wps_akm"))
		return wps_ui->wps_akm;

	if (!strcmp(name, "wps_crypto"))
		return wps_ui->wps_crypto;

	if (!strcmp(name, "wps_psk"))
		return wps_ui->wps_psk;

	if (!strcmp(name, "wps_sta_pin"))
		return wps_ui->wps_sta_pin;

	if (!strcmp(name, "wps_proc_status")) {
		sprintf(buf, "%d", wps_ui->wps_proc_status);
		return buf;
	}

	if (!strcmp(name, "wps_enr_wsec")) {
		return wps_ui->wps_enr_wsec;
	}

	if (!strcmp(name, "wps_enr_ssid")) {
		return wps_ui->wps_enr_ssid;
	}

	if (!strcmp(name, "wps_enr_bssid")) {
		return wps_ui->wps_enr_bssid;
	}

	if (!strcmp(name, "wps_enr_scan")) {
		sprintf(buf, "%d", wps_ui->wps_enr_scan);
		return buf;
	}

	if (!strcmp(name, "wps_stareg_ap_pin"))
		return wps_ui->wps_stareg_ap_pin;

	if (!strcmp(name, "wps_scstate"))
		return wps_ui->wps_scstate;

#ifdef __CONFIG_WFI__
	/* For WiFi-Invite session PIN */
	if (!strcmp(name, "wps_device_pin")) {
		return wps_ui->wps_device_pin;
	}
#endif /* __CONFIG_WFI__ */

	if (!strcmp(name, "wps_autho_sta_mac")) /* WSC 2.0 */
		return wps_ui->wps_autho_sta_mac;

	if (!strcmp(name, "wps_sta_devname"))
		return wps_ui->wps_sta_devname;

	if (!strcmp(name, "wps_pinfail_state"))
		return wps_ui->wps_pinfail_state;

#ifdef WPS_NFC_DEVICE
	if (!strcmp(name, "wps_nfc_dm_status")) {
		sprintf(buf, "%d", wps_ui->wps_nfc_dm_status);
		return buf;
	}

	if (!strcmp(name, "wps_nfc_err_code")) {
		sprintf(buf, "%d", wps_ui->wps_nfc_err_code);
		return buf;
	}
#endif // endif

#ifdef MULTIAP
	if (!strcmp(name, "map_pbc_method")) {
		sprintf(buf, "%d", wps_ui->map_pbc_method);
		return buf;
	}
#endif // endif
	return "";
}

void
wps_ui_set_env(char *name, char *value)
{
	if (!strcmp(name, "wps_ifname"))
		wps_strncpy(wps_ui->wps_ifname, value, sizeof(wps_ui->wps_ifname));
	else if (!strcmp(name, "wps_config_command"))
		wps_ui->wps_config_command = atoi(value);
	else if (!strcmp(name, "wps_action"))
		wps_ui->wps_action = atoi(value);
	else if (!strcmp(name, "wps_aplockdown"))
		wps_ui->wps_aplockdown = atoi(value);
	else if (!strcmp(name, "wps_proc_status"))
		wps_ui->wps_proc_status = atoi(value);
	else if (!strcmp(name, "wps_pinfail"))
		wps_ui->wps_pinfail = atoi(value);
	else if (!strcmp(name, "wps_pinfail_state"))
		wps_strncpy(wps_ui->wps_pinfail_state, value, sizeof(wps_ui->wps_pinfail_state));
	else if (!strcmp(name, "wps_sta_mac"))
		wps_strncpy(wps_ui->wps_sta_mac, value, sizeof(wps_ui->wps_sta_mac));
	else if (!strcmp(name, "wps_sta_devname"))
		wps_strncpy(wps_ui->wps_sta_devname, value, sizeof(wps_ui->wps_sta_devname));
	else if (!strcmp(name, "wps_sta_pin"))
		wps_strncpy(wps_ui->wps_sta_pin, value, sizeof(wps_ui->wps_sta_pin));
	else if (!strcmp(name, "wps_pbc_method"))
		wps_ui->wps_pbc_method = atoi(value);
#ifdef WPS_NFC_DEVICE
	else if (!strcmp(name, "wps_nfc_dm_status"))
		wps_ui->wps_nfc_dm_status = atoi(value);
	else if (!strcmp(name, "wps_nfc_err_code"))
		wps_ui->wps_nfc_err_code = atoi(value);
	else if (!strcmp(name, "wps_enr_wsec"))
		wps_strncpy(wps_ui->wps_enr_wsec, value, sizeof(wps_ui->wps_enr_wsec));
	else if (!strcmp(name, "wps_enr_ssid"))
		wps_strncpy(wps_ui->wps_enr_ssid, value, sizeof(wps_ui->wps_enr_ssid));
	else if (!strcmp(name, "wps_enr_bssid"))
		wps_strncpy(wps_ui->wps_enr_bssid, value, sizeof(wps_ui->wps_enr_bssid));
#endif // endif

#ifdef MULTIAP
	else if (!strcmp(name, "map_pbc_method"))
		wps_ui->map_pbc_method = atoi(value);
#endif // endif

	return;
}

void
wps_ui_reset_env()
{
	/* Reset partial wps_ui environment variables */
	wps_ui->wps_config_command = WPS_UI_CMD_NONE;
	wps_ui->wps_action = WPS_UI_ACT_NONE;
	wps_ui->wps_pbc_method = WPS_UI_PBC_NONE;
}

int
wps_ui_is_pending()
{
	return pending_flag;
}

#ifdef BCMWPSAP
/* Start pending for any MAC  */
static void
wps_ui_start_pending(char *wps_ifname)
{
	unsigned long now;
	CTlvSsrIE ssrmsg;

	int i, imax, scb_num;
	char temp[32];
	char ifname[IFNAMSIZ];
	unsigned int wps_uuid[16];
	unsigned char uuid_R[16];
	unsigned char authorizedMacs[SIZE_MAC_ADDR] = {0};
	int authorizedMacs_len = 0;
	char authorizedMacs_buf[SIZE_MAC_ADDR * SIZE_AUTHORIZEDMACS_NUM] = {0};
	int authorizedMacs_buf_len = 0;
	BufferObj *authorizedMacs_bufObj = NULL, *uuid_R_bufObj = NULL;
	char *wlnames, *next, *ipaddr = NULL, *value;
	bool found = FALSE;
	bool b_wps_version2 = FALSE;
	bool b_do_pbc = FALSE;
	uint8 scState = WPS_SCSTATE_UNCONFIGURED;

	(void) time((time_t*)&now);

	if (strcmp(wps_ui->wps_sta_pin, "00000000") == 0)
		b_do_pbc = TRUE;

	/* Check push time only when we use PBC method */
	if (b_do_pbc && wps_pb_check_pushtime(now) == PBC_OVERLAP_CNT) {
		TUTRACE((TUTRACE_INFO, "%d PBC station found, ignored PBC!\n", PBC_OVERLAP_CNT));
		wps_ui->wps_config_command = WPS_UI_CMD_NONE;
		wps_setProcessStates(WPS_PBCOVERLAP);

		return;
	}

	/* WSC 2.0,  find ipaddr according to wps_ifname */
	imax = wps_get_ess_num();
	for (i = 0; i < imax; i++) {
		sprintf(temp, "ess%d_wlnames", i);
		wlnames = wps_safe_get_conf(temp);

		foreach(ifname, wlnames, next) {
			if (!strcmp(ifname, wps_ifname)) {
				found = TRUE;
				break;
			}
		}

		if (found) {
			/* Get ipaddr */
			sprintf(temp, "ess%d_ipaddr", i);
			ipaddr = wps_safe_get_conf(temp);

			/* According to WPS 2.0 section "Wi-Fi Simple Configuration State"
			 * Note: The Internal Registrar waits until successful completion of the
			 * protocol before applying the automatically generated credentials to
			 * avoid an accidental transition from "Not Configured" to "Configured"
			 * in the case that a neighboring device tries to run WSC
			 */
			/* Get builtin register scState */
			sprintf(temp, "ess%d_wps_oob", i);
			if (!strcmp(wps_safe_get_conf(temp), "disabled") ||
			    !strcmp(wps_safe_get_conf("wps_oob_configured"), "1"))
				scState = WPS_SCSTATE_CONFIGURED;
			else
				scState = WPS_SCSTATE_UNCONFIGURED;

			/* Get UUID, convert string to hex */
			value = wps_safe_get_conf("wps_uuid");
			sscanf(value,
				"%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
				&wps_uuid[0], &wps_uuid[1], &wps_uuid[2], &wps_uuid[3],
				&wps_uuid[4], &wps_uuid[5], &wps_uuid[6], &wps_uuid[7],
				&wps_uuid[8], &wps_uuid[9], &wps_uuid[10], &wps_uuid[11],
				&wps_uuid[12], &wps_uuid[13], &wps_uuid[14], &wps_uuid[15]);
			for (i = 0; i < 16; i++)
				uuid_R[i] = (wps_uuid[i] & 0xff);

			break;
		}
	}

	if (!found || !ipaddr) {
		TUTRACE((TUTRACE_ERR, "Can't find ipaddr according to %s\n", wps_ifname));
		return;
	}

	/* WSC 2.0,  support WPS V2 or not */
	if (strcmp(wps_safe_get_conf("wps_version2"), "enabled") == 0)
		b_wps_version2 = TRUE;

	/* Set built-in registrar to be selected registrar */
	if (b_wps_version2) {
		if (strlen(wps_ui->wps_autho_sta_mac)) {
			/* For now, only support one authorized mac */
			authorizedMacs_len = SIZE_MAC_ADDR;
			ether_atoe(wps_ui->wps_autho_sta_mac, authorizedMacs);
		}
		else {
			/* WSC 2.0 r44, add wildcard MAC when authorized mac not specified */
			authorizedMacs_len = SIZE_MAC_ADDR;
			memcpy(authorizedMacs, wildcard_authorizedMacs, SIZE_MAC_ADDR);
		}

		/* Prepare authorizedMacs_Obj and uuid_R_Obj */
		authorizedMacs_bufObj = buffobj_new();
		if (!authorizedMacs_bufObj) {
			TUTRACE((TUTRACE_ERR, "Can't allocate authorizedMacs_bufObj\n"));
			return;
		}
		uuid_R_bufObj = buffobj_new();
		if (!uuid_R_bufObj) {
			TUTRACE((TUTRACE_ERR, "Can't allocate uuid_R_bufObj\n"));
			buffobj_del(authorizedMacs_bufObj);
			return;
		}
	}

	wps_ie_default_ssr_info(&ssrmsg, authorizedMacs, authorizedMacs_len,
		authorizedMacs_bufObj, uuid_R, uuid_R_bufObj, scState);

	/* WSC 2.0, add authorizedMACs when runs built-in registrar */
	ap_ssr_set_scb(ipaddr, &ssrmsg, NULL, now);

	if (b_wps_version2) {
		/*
		 * Get union (WSC 2.0 page 79, 8.4.1) of information received from all Registrars,
		 * use recently authorizedMACs list and update union attriabutes to ssrmsg
		 */
		scb_num = ap_ssr_get_union_attributes(&ssrmsg, authorizedMacs_buf,
			&authorizedMacs_buf_len);

		/*
		 * No any SSR exist, no any selreg is TRUE, set UPnP SSR time expired
		 * if it is activing
		 */
		if (scb_num == 0) {
			TUTRACE((TUTRACE_ERR, "No any SSR exist\n"));
			if (authorizedMacs_bufObj)
				buffobj_del(authorizedMacs_bufObj);
			if (uuid_R_bufObj)
				buffobj_del(uuid_R_bufObj);
			return;
		}

		/* Reset ssrmsg.authorizedMacs */
		ssrmsg.authorizedMacs.m_data = NULL;
		ssrmsg.authorizedMacs.subtlvbase.m_len = 0;

		/* Construct ssrmsg.authorizedMacs from authorizedMacs_buf */
		if (authorizedMacs_buf_len) {
			/* Re-use authorizedMacs_bufObj */
			buffobj_Rewind(authorizedMacs_bufObj);
			/* De-serialize the authorizedMacs data to get the TLVs */
			subtlv_serialize(WPS_WFA_SUBID_AUTHORIZED_MACS, authorizedMacs_bufObj,
				authorizedMacs_buf, authorizedMacs_buf_len);
			buffobj_Rewind(authorizedMacs_bufObj);
			subtlv_dserialize(&ssrmsg.authorizedMacs, WPS_WFA_SUBID_AUTHORIZED_MACS,
				authorizedMacs_bufObj, 0, 0);
		}
	}

	wps_ie_set(wps_ifname, &ssrmsg);

	if (authorizedMacs_bufObj)
		buffobj_del(authorizedMacs_bufObj);

	if (uuid_R_bufObj)
		buffobj_del(uuid_R_bufObj);

	pending_flag = 1;
	pending_time = now;

#ifdef WPS_ADDCLIENT_WWTP
	/* Change in PF #3 */
	if (addclient_window_start && wps_wer_override_active) {
		pending_time = addclient_window_start;
	}
	else {
		pending_time = now;
		addclient_window_start = now;
	}
#endif /* WPS_ADDCLIENT_WWTP */

	return;
}
#endif /* ifdef BCMWPSAP */

#ifdef WPS_ADDCLIENT_WWTP
int
wps_ui_is_SET_cmd(char *buf, int buflen)
{
	if (strncmp(buf, "SET ", 4) == 0)
		return 1;

	return 0;
}

void
wps_ui_wer_override_active(bool active)
{
	wps_wer_override_active = active;
}

void
wps_ui_close_addclient_window()
{
	addclient_window_start = 0;
	wps_ui_wer_override_active(FALSE);
}
#endif /* WPS_ADDCLIENT_WWTP */

void
wps_ui_clear_pending()
{
	pending_flag = 0;
	pending_time = 0;
}

int
wps_ui_pending_expire()
{
	unsigned long now;

	if (pending_flag && pending_time) {
		(void) time((time_t*)&now);

		if ((now < pending_time) || ((now - pending_time) > WPS_MAX_TIMEOUT)) {
#ifdef WPS_ADDCLIENT_WWTP
			wps_ui_close_addclient_window();
#endif // endif
			return 1;
		}
	}

	return 0;
}

static const char *
wps_ui_get_wps_str(wps_ui_str_t *wps_ui_str, uint32 value)
{
	int i;

	if (wps_ui_str == NULL)
		return "Null Argument";

	for (i = 0; wps_ui_str[i].string != NULL; i++) {
		if (wps_ui_str[i].value == value)
			return wps_ui_str[i].string;
	}

	return "Uknown";
}

static void
wps_ui_dump(wps_ui_t *ui)
{
	if (ui->wps_config_command == WPS_UI_CMD_MSGLEVEL) {
		printf("WPS_UI: set msglevel: 0x%x\n", ui->wps_msglevel);
		return;
	}

	printf("============ wps_ui_dump ============\n");
	printf("[%s]::\n", wps_ui_get_wps_str(_wps_ui_cmd_str, ui->wps_config_command));
	printf("    Method:     %s\n", wps_ui_get_wps_str(_wps_ui_method_str, ui->wps_method));
	printf("    Action:     %s\n", wps_ui_get_wps_str(_wps_ui_action_str, ui->wps_action));
	printf("    PBC Method: %s\n", wps_ui_get_wps_str(_wps_ui_pbc_str, ui->wps_pbc_method));
	printf("    Interface:  %s\n", ui->wps_ifname);
	printf("    STA PIN:    %s\n", ui->wps_sta_pin);
	printf("    SSID:       %s\n", ui->wps_ssid);
	printf("    AKM:        %s\n", ui->wps_akm);
	printf("    Crypto:     %s\n", ui->wps_crypto);
	printf("    PSK:        %s\n", ui->wps_psk);
	printf("    ENR WSEC:   %s\n", ui->wps_enr_wsec);
	printf("    ENR SSID:   %s\n", ui->wps_enr_ssid);
	printf("    ENR BSSID:  %s\n", ui->wps_enr_bssid);
	printf("    ENR SCAN:   %d\n", ui->wps_enr_scan);
#ifdef __CONFIG_WFI__
	printf("    WFI DEV PIN: %s\n", ui->wps_device_pin);
#endif /* __CONFIG_WFI__ */
	printf("    Autho MAC: %s\n", ui->wps_autho_sta_mac);
	printf("    STA REG AP PIN: %s\n", ui->wps_stareg_ap_pin);
	printf("    WPS State: %s\n", ui->wps_scstate);
	printf("=====================================\n");

	return;
}

#ifdef WPS_NFC_DEVICE
static DevInfo *
wps_ui_nfc_get_devinfo(DevInfo *ap_devinfo, bool cho)
{
	bool b_wps_version2 = FALSE;
	char *dev_key = NULL;
	char *value, *next;
	int auth = 0;
	char ssid[SIZE_32_BYTES + 1] = {0};
	char psk[SIZE_64_BYTES + 1] = {0};
	char akmstr[32];
	char key[8];
	unsigned int akm = 0;
	unsigned int wsec = 0;
	int wep_index = 0;
	char *wep_key = NULL;
	char dev_akm[64] = {0};
	char dev_crypto[64] = {0};
	bool oob_addenr = 0;
	char prefix[] = "wlXXXXXXXXXX_";
	char tmp[256];
	char *wlnames;
	char ifname[IFNAMSIZ];
	int ess_id, imax;

	if (!ap_devinfo)
		return NULL;

	memset(ap_devinfo, 0, sizeof(DevInfo));

	/* Prefix */
	snprintf(prefix, sizeof(prefix), "%s_", wps_ui->wps_ifname);

	/* WSC 2.0,  support WPS V2 or not */
	if (strcmp(wps_safe_get_conf("wps_version2"), "enabled") == 0) {
		b_wps_version2 = TRUE;
		value = wps_get_conf("wps_version2_num");
		ap_devinfo->version2 = (uint8)(strtoul(value, NULL, 16));
	}

	/* raw OOB config state (per-ESS) */
	imax = wps_get_ess_num();
	for (ess_id = 0; ess_id < imax; ess_id++) {
		sprintf(tmp, "ess%d_wlnames", ess_id);
		wlnames = wps_safe_get_conf(tmp);

		foreach(ifname, wlnames, next) {
			if (!strcmp(ifname, wps_ui->wps_ifname)) {
				goto found;
			}
		}
	}

found:
	if (ess_id == imax)
		return NULL;

	/* Both CHO/Configuration Tag need RFband/APChannel and BSSID */
	/* RFband */
	sprintf(tmp, "%s_band", wps_ui->wps_ifname);
	value = wps_safe_get_conf(tmp);
	ap_devinfo->rfBand = atoi(value);

	/* APChannel */
	ap_devinfo->apchannel = wps_wl_channel(wps_ui->wps_ifname);

	/* BSSID */
	value = wps_get_conf(strcat_r(prefix, "hwaddr", tmp));
	if (value) {
		unsigned char apmac[SIZE_6_BYTES];

		ether_atoe(value, apmac);
		memcpy(ap_devinfo->macAddr, apmac, SIZE_6_BYTES);
	}

	if (cho) {
		/* AP SSID */
		value = wps_safe_get_conf(strcat_r(prefix, "ssid", tmp));
		wps_strncpy(ap_devinfo->ssid, value, sizeof(ap_devinfo->ssid));

		/* UUID */
		wps_upnp_device_uuid(ap_devinfo->uuid);

		return ap_devinfo;
	}

	sprintf(tmp, "ess%d_wps_oob", ess_id);
	if (!strcmp(wps_safe_get_conf(tmp), "disabled") ||
	    !strcmp(wps_safe_get_conf("wps_oob_configured"), "1"))
		ap_devinfo->scState = WPS_SCSTATE_CONFIGURED;
	else
		ap_devinfo->scState = WPS_SCSTATE_UNCONFIGURED;

	/* Auth */
	value = wps_safe_get_conf(strcat_r(prefix, "auth", tmp));
	if (!strcmp(value, "1"))
		auth = 1;

	ap_devinfo->auth = auth;

	/* If caller has specified credentials using them */
	if ((value = wps_ui_get_env("wps_ssid")) && strcmp(value, "") != 0) {

		/* SSID */
		value = wps_ui_get_env("wps_ssid");
		wps_strncpy(ssid, value, sizeof(ssid));

		/* AKM */
		value = wps_ui_get_env("wps_akm");
		foreach(akmstr, value, next) {
			if (!strcmp(akmstr, "psk"))
				akm |= WPA_AUTH_PSK;
			else if (!strcmp(akmstr, "psk2"))
				akm |= WPA2_AUTH_PSK;
			else {
				TUTRACE((TUTRACE_INFO, "Error in AKM\n"));
				return NULL;
			}
		}

		/* Crypto */
		if (akm) {
			value = wps_ui_get_env("wps_crypto");
			if (!strcmp(value, "aes"))
				wsec = AES_ENABLED;
			else if (!strcmp(value, "tkip+aes"))
				wsec = TKIP_ENABLED|AES_ENABLED;
			else {
				TUTRACE((TUTRACE_INFO, "Error in crypto\n"));
				return NULL;
			}

			/* Set PSK key */
			value = wps_ui_get_env("wps_psk");
			if (strlen(value) < 8 || strlen(value) > SIZE_64_BYTES) {
				TUTRACE((TUTRACE_INFO, "Error in crypto\n"));
				return NULL;
			}
			wps_strncpy(psk, value, sizeof(psk));
			dev_key = psk;
		}
	}
	else {
		/*
		 * Before check oob mode, we have to
		 * get ssid, akm, wep, crypto and mgmt key from config.
		 * because oob mode might change the settings.
		 */
		value = wps_safe_get_conf(strcat_r(prefix, "ssid", tmp));
		wps_strncpy(ssid, value, sizeof(ssid));

		/* AKM */
		value = wps_safe_get_conf(strcat_r(prefix, "akm", tmp));
		foreach(akmstr, value, next) {
			if (!strcmp(akmstr, "psk"))
				akm |= WPA_AUTH_PSK;
			else if (!strcmp(akmstr, "psk2"))
				akm |= WPA2_AUTH_PSK;
			else {
				TUTRACE((TUTRACE_INFO, "Unknown AKM value\n"));
				return NULL;
			}
		}

		value = wps_safe_get_conf(strcat_r(prefix, "wep", tmp));
		wsec = !strcmp(value, "enabled") ? WEP_ENABLED : 0;

		/* Crypto */
		value = wps_safe_get_conf(strcat_r(prefix, "crypto", tmp));
		if (WPS_WLAKM_PSK(akm) || WPS_WLAKM_PSK2(akm)) {
			if (!strcmp(value, "tkip"))
				wsec |= TKIP_ENABLED;
			else if (!strcmp(value, "aes"))
				wsec |= AES_ENABLED;
			else if (!strcmp(value, "tkip+aes"))
				wsec |= TKIP_ENABLED|AES_ENABLED;
			else {
				TUTRACE((TUTRACE_INFO, "Unknown Crypto value\n"));
				return NULL;
			}

			/* Set PSK key */
			value = wps_safe_get_conf(strcat_r(prefix, "wpa_psk", tmp));
			wps_strncpy(psk, value, sizeof(psk));
		}

		if (wsec & WEP_ENABLED) {
			/* Key index */
			value = wps_safe_get_conf(strcat_r(prefix, "key", tmp));
			wep_index = (int)strtoul(value, NULL, 0);

			/* Key */
			sprintf(key, "key%s", value);
			wep_key = wps_safe_get_conf(strcat_r(prefix, key, tmp));
		}

		if (ap_devinfo->scState == WPS_SCSTATE_UNCONFIGURED)
			oob_addenr = 1;

		/* Caution: oob_addenr will over-write akm and wsec */
		if (oob_addenr) {
			/* Generate random ssid and key */
			if (wps_gen_ssid(ssid, sizeof(ssid),
				wps_get_conf("wps_random_ssid_prefix"),
				wps_safe_get_conf("eth1_hwaddr")) == FALSE ||
			    wps_gen_key(psk, sizeof(psk)) == FALSE)
				return NULL;

			/* Open */
			auth = 0;

			/* PSK, PSK2 */
			akm = WPA_AUTH_PSK | WPA2_AUTH_PSK;
			wsec = AES_ENABLED;
		}

		/*
		 * Let the user have a chance to override the credential.
		 */
		if (WPS_WLAKM_BOTH(akm))
			strcpy(dev_akm, "WPA-PSK WPA2-PSK");
		else if (WPS_WLAKM_PSK(akm))
			strcpy(dev_akm, "WPA-PSK");
		else if (WPS_WLAKM_PSK2(akm))
			strcpy(dev_akm, "WPA2-PSK");
		else
			dev_akm[0] = 0;

		/* Encryption algorithm */
		if (WPS_WLENCR_BOTH(wsec))
			strcpy(dev_crypto, "AES+TKIP");
		else if (WPS_WLENCR_TKIP(wsec))
			strcpy(dev_crypto, "TKIP");
		else if (WPS_WLENCR_AES(wsec))
			strcpy(dev_crypto, "AES");
		else
			dev_crypto[0] = 0;

		/* Do customization, and check credentials again */
		wpsapp_utils_update_custom_cred(ssid, psk, dev_akm, dev_crypto, oob_addenr,
			b_wps_version2);

		/* Parsing return akm and crypto */
		if (strlen(dev_akm)) {
			if (!strcmp(dev_akm, "WPA-PSK WPA2-PSK"))
				akm = WPA_AUTH_PSK | WPA2_AUTH_PSK;
			else if (!strcmp(dev_akm, "WPA-PSK"))
				akm = WPA_AUTH_PSK;
			else if (!strcmp(dev_akm, "WPA2-PSK"))
				akm = WPA2_AUTH_PSK;
		}
		if (strlen(dev_crypto)) {
			if (!strcmp(dev_crypto, "AES+TKIP"))
				wsec = AES_ENABLED | TKIP_ENABLED;
			else if (!strcmp(dev_crypto, "AES"))
				wsec = AES_ENABLED;
			else if (!strcmp(dev_crypto, "TKIP"))
				wsec = TKIP_ENABLED;
		}
	}

	/*
	 * After doing customized credentials modification,
	 * fill ssid, psk, akm and crypto to ap_deviceinfo
	 */
	wps_strncpy(ap_devinfo->ssid, ssid, sizeof(ap_devinfo->ssid));

	/* KEY MGMT */
	/* WSC 2.0, deprecated SHARED */
	if (auth) {
		strcpy(ap_devinfo->keyMgmt, "SHARED");
		if (b_wps_version2 && ap_devinfo->scState == WPS_SCSTATE_CONFIGURED) {
			TUTRACE((TUTRACE_INFO, "Write NFC Configuration: "
				"Authentication type is Shared, violate WSC 2.0\n"));
			return NULL;
		}
	}
	else {
		if (WPS_WLAKM_BOTH(akm))
			strcpy(ap_devinfo->keyMgmt, "WPA-PSK WPA2-PSK");
		else if (WPS_WLAKM_PSK(akm))
			strcpy(ap_devinfo->keyMgmt, "WPA-PSK");
		else if (WPS_WLAKM_PSK2(akm))
			strcpy(ap_devinfo->keyMgmt, "WPA2-PSK");
		else
			ap_devinfo->keyMgmt[0] = 0;
	}

	/* WEP index */
	ap_devinfo->wep = (wsec & WEP_ENABLED) ? 1 : 0;

	/* AKM has higher priority than WEP */
	if (WPS_WLAKM_NONE(akm) && ap_devinfo->wep) {
		/* Get wps key index */
		ap_devinfo->wepKeyIdx = wep_index;

		/* Get wps key content */
		dev_key = wep_key;

		if (b_wps_version2 && ap_devinfo->scState == WPS_SCSTATE_CONFIGURED) {
			TUTRACE((TUTRACE_INFO, "Write NFC Configuration: "
				"WEP is enabled violate WSC 2.0\n"));
			return NULL;
		}
	}
	else if (!WPS_WLAKM_NONE(akm)) {
		dev_key = psk;

		if ((wsec & (AES_ENABLED | TKIP_ENABLED)) == 0) {
			TUTRACE((TUTRACE_INFO, "Error in crypto\n"));
			return NULL;
		}
		if (b_wps_version2 && ap_devinfo->scState == WPS_SCSTATE_CONFIGURED) {
			/* WSC 2.0, WPS-PSK is allowed only in mix mode */
			if ((akm & WPA_AUTH_PSK) && !(akm & WPA2_AUTH_PSK)) {
				TUTRACE((TUTRACE_ERR, "Write NFC Configuration: "
					" WPA-PSK enabled without WPA2-PSK,"
					" violate WSC 2.0\n"));
				return NULL;
			}
		}
	}

	/* Network key,  it may WEP key or Passphrase key */
	memset(ap_devinfo->nwKey, 0, SIZE_64_BYTES);
	if (dev_key)
		wps_strncpy(ap_devinfo->nwKey, dev_key, sizeof(ap_devinfo->nwKey));

	/* Set crypto algorithm */
	ap_devinfo->crypto = 0;
	if (wsec & TKIP_ENABLED)
		ap_devinfo->crypto |= WPS_ENCRTYPE_TKIP;
	if (wsec & AES_ENABLED)
		ap_devinfo->crypto |= WPS_ENCRTYPE_AES;

	if (ap_devinfo->crypto == 0)
		ap_devinfo->crypto = WPS_ENCRTYPE_NONE;

	/* The MAC Address attribute contains the Enrollee's MAC address.
	 * To set this attribute, the Registrar needs prior knowledge of the Enrollee's MAC
	 * address, either through a partial run of the registration protocol
	 * (until M2D message) or through prior out-of-band communication as with the
	 * Connection Handover protocol. If a Registrar does not know the Enrollee's MAC
	 * address or does not want to link the Credential to a specific MAC address,
	 * the broadcast MAC address (all octets set to 255) can be used. The MAC Address
	 * attribute is encoded as a Wi-Fi Simple Configuration TLV with Type 0x1020 and
	 * Length 0x0006
	 */
	if (strlen(wps_ui->wps_sta_mac))
		ether_atoe(wps_ui->wps_sta_mac, ap_devinfo->peerMacAddr);
	else
		ether_atoe("FF:FF:FF:FF:FF:FF", ap_devinfo->peerMacAddr);

	return ap_devinfo;
}

static int
wps_ui_nfc_write_cfg()
{
	DevInfo ap_devinfo;

	if (wps_ui_nfc_get_devinfo(&ap_devinfo, FALSE) == NULL)
		return NFC_ERROR;

	/* Write NFC Configuration */
	return wps_nfc_write_cfg(wps_ui->wps_ifname, &ap_devinfo);
}

static int
wps_ui_nfc_write_pw()
{
	uint8 hex_pin[SIZE_32_BYTES];
	DevInfo ap_devinfo = {0};

	/* WSC 2.0,  support WPS V2 or not */
	if (strcmp(wps_safe_get_conf("wps_version2"), "enabled") == 0)
		ap_devinfo.version2 = (uint8)(strtoul(wps_get_conf("wps_version2_num"), NULL, 16));

	wps_gen_oob_dev_pw(ap_devinfo.pre_privkey, ap_devinfo.pub_key_hash,
		&ap_devinfo.devPwdId, hex_pin, sizeof(hex_pin));

	/* Do Hex to String translation */
	if (!wps_hex2str(ap_devinfo.pin, sizeof(ap_devinfo.pin), hex_pin, sizeof(hex_pin))) {
		TUTRACE((TUTRACE_NFC, "Invalid parameters\n"));
		return WPS_ERR_INVALID_PARAMETERS;
	}

	/* Write NFC Password */
	return wps_nfc_write_pw(wps_ui->wps_ifname, &ap_devinfo);
}

static int
wps_ui_nfc_cho(bool cho_s)
{
	uint8 hex_pin[SIZE_32_BYTES];
	DevInfo ap_devinfo = {0};

	if (wps_ui_nfc_get_devinfo(&ap_devinfo, TRUE) == NULL)
		return NFC_ERROR;

	wps_gen_oob_dev_pw(ap_devinfo.pre_privkey, ap_devinfo.pub_key_hash,
		&ap_devinfo.devPwdId, hex_pin, sizeof(hex_pin));

	if (!wps_hex2str(ap_devinfo.pin, sizeof(ap_devinfo.pin), hex_pin, sizeof(hex_pin))) {
		TUTRACE((TUTRACE_NFC, "Invalid parameters\n"));
		return WPS_ERR_INVALID_PARAMETERS;
	}

	/* Start hand over selector with WPS configuration */
	if (cho_s) {
		return wps_nfc_ho_selector(wps_ui->wps_ifname, &ap_devinfo);
	}
	else
		return wps_nfc_ho_requester(wps_ui->wps_ifname, &ap_devinfo);
}

void
wps_ui_nfc_open_session()
{
	char uibuf[1024];
	struct strbuf b;

	/* AP Registration: Add Enrollee */
	/* AP Enrollment: Add Enrollee */
	/* STA Enrollment: Get AP Config */
	/* STA Registration: Get AP Config */
	/* CHO */
	str_binit(&b, uibuf, sizeof(uibuf));

	str_bprintf(&b, "%s", "SET ");
	str_bprintf(&b, "wps_config_command=%d ", WPS_UI_CMD_START);
	str_bprintf(&b, "wps_config_command=%d ", WPS_UI_CMD_START);
	str_bprintf(&b, "wps_action=%d ", wps_ui->wps_action);
	str_bprintf(&b, "wps_method=%d ", WPS_UI_METHOD_PIN);
	str_bprintf(&b, "wps_pbc_method=\"%d\" ", wps_ui->wps_pbc_method);
	str_bprintf(&b, "wps_sta_pin=\"%s\" ", wps_ui->wps_sta_pin);
	str_bprintf(&b, "wps_stareg_ap_pin=\"%s\" ", wps_ui->wps_stareg_ap_pin);
	str_bprintf(&b, "wps_autho_sta_mac=%s ", wps_ui->wps_autho_sta_mac);
	str_bprintf(&b, "wps_ifname=%s ", wps_ui->wps_ifname);
	str_bprintf(&b, "wps_ssid=\"%s\" ", wps_ui->wps_ssid);
	str_bprintf(&b, "wps_akm=\"%s\" ", wps_ui->wps_akm);
	str_bprintf(&b, "wps_crypto=\"%s\" ", wps_ui->wps_crypto);
	str_bprintf(&b, "wps_psk=\"%s\" ", wps_ui->wps_psk);
	str_bprintf(&b, "wps_enr_wsec=\"%s\" ", wps_ui->wps_enr_wsec);
	str_bprintf(&b, "wps_enr_ssid=\"%s\" ", wps_ui->wps_enr_ssid);
	str_bprintf(&b, "wps_enr_bssid=\"%s\" ", wps_ui->wps_enr_bssid);
	str_bprintf(&b, "wps_enr_scan=\"%d\" ", wps_ui->wps_enr_scan);

	if (wps_osl_nfc_set_wps_env(&ui_hndl, uibuf, strlen(uibuf)+1) < 0) {
		TUTRACE((TUTRACE_ERR, "wps_ui_nfc_open_session:sendto failed:%s\n",
			strerror(errno)));
	}

	return;
}
#endif /* WPS_NFC_DEVICE */

static void
wps_ui_do_get()
{
#ifdef WPS_UPNP_DEVICE
	unsigned char wps_uuid[16];
#endif // endif
	char buf[512];
	struct strbuf b;

	str_binit(&b, buf, sizeof(buf));

	str_bprintf(&b, "wps_config_command=%d ", wps_ui->wps_config_command);
	str_bprintf(&b, "wps_action=%d ", wps_ui->wps_action);
	/* Add in PF #3 */
	str_bprintf(&b, "wps_method=%d ", wps_ui->wps_method);
	str_bprintf(&b, "wps_autho_sta_mac=%s ", wps_ui->wps_autho_sta_mac);
	str_bprintf(&b, "wps_ifname=%s ", wps_ui->wps_ifname);
#ifdef WPS_UPNP_DEVICE
	wps_upnp_device_uuid(wps_uuid);
	str_bprintf(&b,
		"wps_uuid=%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x ",
		wps_uuid[0], wps_uuid[1], wps_uuid[2], wps_uuid[3],
		wps_uuid[4], wps_uuid[5], wps_uuid[6], wps_uuid[7],
		wps_uuid[8], wps_uuid[9], wps_uuid[10], wps_uuid[11],
		wps_uuid[12], wps_uuid[13], wps_uuid[14], wps_uuid[15]);
#endif /* WPS_UPNP_DEVICE */
	str_bprintf(&b, "wps_aplockdown=%d ", wps_ui->wps_aplockdown);
	str_bprintf(&b, "wps_proc_status=%d ", wps_ui->wps_proc_status);
	str_bprintf(&b, "wps_pinfail=%d ", wps_ui->wps_pinfail);
	str_bprintf(&b, "wps_sta_mac=%s ", wps_ui->wps_sta_mac);
	str_bprintf(&b, "wps_sta_devname=\"%s\" ", wps_ui->wps_sta_devname);
#ifdef WPS_NFC_DEVICE
	str_bprintf(&b, "wps_nfc_dm_status=%d ", wps_ui->wps_nfc_dm_status);
	str_bprintf(&b, "wps_nfc_err_code=%d ", wps_ui->wps_nfc_err_code);
#endif // endif
	str_bprintf(&b, "wps_msglevel=0x%x ", wps_tutrace_get_msglevel());

	if (wps_osl_send_uimsg(&ui_hndl, buf, strlen(buf)+1) < 0) {
		TUTRACE((TUTRACE_ERR, "wps_osl_send_uimsg:sendto failed:%s\n", strerror(errno)));
	}

	return;
}

#define MAX_UI_ARGS	32

/* Note, the ui parser has limitation on processing quote.
 * It can only process string within two quotes at most
 */
static int
wps_ui_parse_env(char *buf, wps_ui_t *ui)
{
	char *argv[MAX_UI_ARGS] = {0};
	char *p;
	char *name, *value;
	int i;

	/* Seperate buf into argv[], we have to make sure at least one is empty */
	for (i = 0, p = buf; i < MAX_UI_ARGS-1; i++) {
		/* Eat white space */
		while (*p == ' ')
			p++;
		if (*p == 0)
			goto all_found;

		/* Save this item */
		argv[i] = p;

		 /* Search until space */
		while (*p != ' ' && *p) {
			/* Take care of doube quot */
			if (*p == '\"') {
				char *qs, *qe;

				qs = p;
				qe = strchr(p+1, '\"');
				if (qe == NULL) {
					printf("%s:%d, unbalanced quote string!",
						__func__, __LINE__);
					argv[i] = 0;
					goto all_found;
				}

				/* Null eneded quot string and do shift */
				*qe = '\0';
				memmove(qs, qs+1, (int)(qe-qs));

				p = qe+1;
				break;
			}

			p++;
		}

		if (*p)
			*p++ = '\0';
	}

	ui->wps_msglevel = wps_tutrace_get_msglevel();

all_found:
	for (i = 0; argv[i]; i++) {
		value = argv[i];
		name = strsep(&value, "=");
		if (name) {
			if (!strcmp(name, "wps_config_command"))
				ui->wps_config_command = atoi(value);
			else if (!strcmp(name, "wps_method"))
				ui->wps_method = atoi(value);
			else if (!strcmp(name, "wps_action"))
				ui->wps_action = atoi(value);
			else if (!strcmp(name, "wps_pbc_method"))
				ui->wps_pbc_method = atoi(value);
			else if (!strcmp(name, "wps_ifname"))
				wps_strncpy(ui->wps_ifname, value, sizeof(ui->wps_ifname));
			else if (!strcmp(name, "wps_sta_pin"))
				wps_strncpy(ui->wps_sta_pin, value, sizeof(ui->wps_sta_pin));
			else if (!strcmp(name, "wps_ssid"))
				wps_strncpy(ui->wps_ssid, value, sizeof(ui->wps_ssid));
			else if (!strcmp(name, "wps_akm"))
				wps_strncpy(ui->wps_akm, value, sizeof(ui->wps_akm));
			else if (!strcmp(name, "wps_crypto"))
				wps_strncpy(ui->wps_crypto, value, sizeof(ui->wps_crypto));
			else if (!strcmp(name, "wps_psk"))
				wps_strncpy(ui->wps_psk, value, sizeof(ui->wps_psk));
			else if (!strcmp(name, "wps_enr_wsec"))
				wps_strncpy(ui->wps_enr_wsec, value, sizeof(ui->wps_enr_wsec));
			else if (!strcmp(name, "wps_enr_ssid"))
				wps_strncpy(ui->wps_enr_ssid, value, sizeof(ui->wps_enr_ssid));
			else if (!strcmp(name, "wps_enr_bssid"))
				wps_strncpy(ui->wps_enr_bssid, value, sizeof(ui->wps_enr_bssid));
			else if (!strcmp(name, "wps_enr_scan"))
				ui->wps_enr_scan = atoi(value);
#ifdef __CONFIG_WFI__
			else if (!strcmp(name, "wps_device_pin"))
				wps_strncpy(ui->wps_device_pin, value, sizeof(ui->wps_device_pin));
#endif /* __CONFIG_WFI__ */
			else if (!strcmp(name, "wps_autho_sta_mac")) /* WSC 2.0 */
				wps_strncpy(ui->wps_autho_sta_mac, value,
					sizeof(ui->wps_autho_sta_mac));
			else if (!strcmp(name, "wps_stareg_ap_pin"))
				wps_strncpy(ui->wps_stareg_ap_pin, value,
					sizeof(ui->wps_stareg_ap_pin));
			else if (!strcmp(name, "wps_scstate"))
				wps_strncpy(ui->wps_scstate, value, sizeof(ui->wps_scstate));
			else if (!strcmp(name, "wps_msglevel"))
				ui->wps_msglevel = strtoul(value, NULL, 16);
		}
	}

#ifdef WPS_NFC_DEVICE
	/* Keep current device managment status and err code */
	ui->wps_nfc_dm_status = wps_ui->wps_nfc_dm_status;
	ui->wps_nfc_err_code = wps_ui->wps_nfc_err_code;
#endif // endif

	return 0;
}

static int
wps_ui_do_set(char *buf)
{
	int ret = WPS_CONT;
	wps_ui_t a_ui;
	wps_ui_t *ui = &a_ui;
	wps_app_t *wps_app = get_wps_app();
#if defined(MULTIAP)
	unsigned long start_tm = 0;
#endif	/* MULTIAP */

	/* Process set command */
	memset(ui, 0, sizeof(wps_ui_t));
	if (wps_ui_parse_env(buf, ui) != 0)
		return WPS_CONT;

#ifdef _TUDEBUGTRACE
	wps_ui_dump(ui);
#endif // endif

	/* user click STOPWPS button or pending expire */
	if (ui->wps_config_command == WPS_UI_CMD_STOP) {
#ifdef WPS_NFC_DEVICE
		if (IS_WPS_UI_CMD_NFC(wps_ui->wps_config_command)) {
			wps_nfc_stop();
			wps_close_session();
		}
		else
#endif /* WPS_NFC_DEVICE */
		{
#ifdef WPS_ADDCLIENT_WWTP
			/* stop add client 2 min. window */
			wps_ui_close_addclient_window();
			wps_close_addclient_window();
#endif /* WPS_ADDCLIENT_WWTP */

			/* state maching in progress, check stop or push button here */
			wps_close_session();
			wps_setProcessStates(WPS_INIT);

			/*
			  * set wps LAN all leds to normal and wps_pb_state
			  * when session timeout or user stop
			  */
			wps_pb_state_reset();
		}
	}
#ifdef WPS_NFC_DEVICE
	else if (ui->wps_config_command == WPS_UI_CMD_NFC_WR_CFG ||
		ui->wps_config_command == WPS_UI_CMD_NFC_RD_CFG ||
		ui->wps_config_command == WPS_UI_CMD_NFC_FM ||
		ui->wps_config_command == WPS_UI_CMD_NFC_WR_PW ||
		ui->wps_config_command == WPS_UI_CMD_NFC_RD_PW ||
		ui->wps_config_command == WPS_UI_CMD_NFC_HO_S ||
		ui->wps_config_command == WPS_UI_CMD_NFC_HO_R) {

		/* Close current session if any */
		if (wps_app->wksp || !wps_nfc_is_idle()) {
			wps_nfc_stop();
			wps_close_session();
		}

		/* Fine to accept set command */
		memcpy(wps_ui, ui, sizeof(*ui));

		if (wps_ui->wps_config_command == WPS_UI_CMD_NFC_WR_CFG)
			ret = wps_ui_nfc_write_cfg();
		else if (wps_ui->wps_config_command == WPS_UI_CMD_NFC_RD_CFG)
			ret = wps_nfc_read_cfg(wps_ui->wps_ifname);
		else if (wps_ui->wps_config_command == WPS_UI_CMD_NFC_WR_PW)
			ret = wps_ui_nfc_write_pw();
		else if (wps_ui->wps_config_command == WPS_UI_CMD_NFC_RD_PW)
			ret = wps_nfc_read_pw(wps_ui->wps_ifname);
		else if (wps_ui->wps_config_command == WPS_UI_CMD_NFC_HO_S)
			ret = wps_ui_nfc_cho(TRUE);
		else if (wps_ui->wps_config_command == WPS_UI_CMD_NFC_HO_R)
			ret = wps_ui_nfc_cho(FALSE);
		else if (wps_ui->wps_config_command == WPS_UI_CMD_NFC_FM)
			ret = wps_nfc_format();
	}
#endif /* WPS_NFC_DEVICE */
	else if (ui->wps_config_command == WPS_UI_CMD_MSGLEVEL) {
		/* Change the wps_msglevel dynamicly */
		wps_tutrace_set_msglevel(ui->wps_msglevel);
	}
	else {
#if defined(MULTIAP)
		start_tm = wpssta_get_start_time();
#endif	/* MULTIAP */
		/* Add in PF #3 */
		if (ui->wps_config_command == WPS_UI_CMD_START &&
		    ui->wps_pbc_method == WPS_UI_PBC_SW &&
		    (wps_app->wksp || pending_flag)) {
			/* close session if session opend or in pending state */
			wps_close_session();
		}

		/* for WPS_AP_M2D */
		if ((!wps_app->wksp || WPS_IS_PROXY(wps_app->sc_mode)) &&
		    !pending_flag) {
			/* Fine to accept set command */
			memcpy(wps_ui, ui, sizeof(*ui));

			/* some button in UI is pushed */
			if (wps_ui->wps_config_command == WPS_UI_CMD_START) {
				wps_setProcessStates(WPS_INIT);

				/* if proxy in process, stop it */
				if (wps_app->wksp) {
					wps_close_session();
					wps_ui->wps_config_command = WPS_UI_CMD_START;
					wps_ui->wps_action = ui->wps_action;
				}

				if (wps_is_wps_sta(wps_ui->wps_ifname)) {
					/* set what interface you want */
#ifdef BCMWPSAPSTA
					/* set bss to up */
					wps_wl_bss_config(wps_safe_get_conf("wps_pbc_sta_ifname"),
						1);

					/* set the process status */
					wps_setProcessStates(WPS_ASSOCIATED);
					ret = wpssta_open_session(wps_app, wps_ui->wps_ifname);
#if defined(MULTIAP)
					/* If multiap onboarding is happening map time being
					 * updated in open_session and update the start time
					 * to the time when session was started, for the first
					 * session variable start_tm will be 0 hence it will get
					 * updated only after first iteration is done.
					 */
					if (wps_map_is_onboarding(wps_ui->wps_ifname)) {
						if (start_tm != 0 &&
							(get_current_time() - start_tm) < 120) {
							wpssta_set_start_time(start_tm);
						}
					}
#endif	/* MULTIAP */
					if (ret != WPS_CONT) {
						/* Normally it cause by associate time out */
						wps_setProcessStates(WPS_TIMEOUT);
					}
#endif /* BCMWPSAPSTA */

				}
#ifdef BCMWPSAP
				/* for wps_ap application */
				else {
					/* set the process status */
					wps_setProcessStates(WPS_ASSOCIATED);

					if (wps_ui->wps_action == WPS_UI_ACT_CONFIGAP) {
						/* TBD - Initial all relative nvram setting */
						char tmptr[] = "wlXXXXXXXXXX_hwaddr";
						char *macaddr;
						char *pre_privkey = NULL;

						sprintf(tmptr, "%s_hwaddr", wps_ui->wps_ifname);
						macaddr = wps_get_conf(tmptr);

						if (macaddr) {
						unsigned char ifmac[SIZE_6_BYTES];
						ether_atoe(macaddr, ifmac);
#ifdef WPS_NFC_DEVICE
						if (strcmp(wps_ui_get_env("wps_stareg_ap_pin"),
							"NFC_PW") == 0) {
							pre_privkey = (char *)wps_nfc_priv_key();
						}
#endif // endif
						/* WSC 2.0, Request to Enroll TRUE */
						ret = wpsap_open_session(wps_app,
							SCMODE_AP_ENROLLEE, ifmac, NULL,
							wps_ui->wps_ifname, NULL,
							pre_privkey, NULL, 0, 1, 0);
						}
						else {
							TUTRACE((TUTRACE_INFO, "can not find mac "
								"on %s\n", wps_ui->wps_ifname));
						}
					}
					else if (wps_ui->wps_action == WPS_UI_ACT_ADDENROLLEE) {
						/* Do nothing, wait for client connect */
						wps_ui_start_pending(wps_ui->wps_ifname);
					}
				}

#else
				TUTRACE((TUTRACE_INFO, "AP functionality not included.\n"));
#endif /* BCMWPSAP */
			}
		}
	}

	return ret;
}

int
wps_ui_process_msg(char *buf, int buflen)
{
	if (strncmp(buf, "GET", 3) == 0) {
		wps_ui_do_get();
	}

	if (strncmp(buf, "SET ", 4) == 0) {
		return wps_ui_do_set(buf+4);
	}

	return WPS_CONT;
}

int
wps_ui_init()
{
	char *value;

	/* Init eap_hndl */
	memset(&ui_hndl, 0, sizeof(ui_hndl));

	ui_hndl.type = WPS_RECEIVE_PKT_UI;
	ui_hndl.handle = wps_osl_ui_handle_init();
	if (ui_hndl.handle == -1)
		return -1;

	wps_hndl_add(&ui_hndl);

	/*
	 * Update the wps_proc_status from NVRAM to ui variable,
	 * we need it to reflect the wps process status after restart.
	 * In the case of using wps_cmd to get WPS status, the wps_cmd
	 * always read WPS status from the ui variable.
	*/
	value = wps_get_conf("wps_proc_status");
	if (value == NULL)
		value = "0"; /* Init */
	wps_ui_set_env("wps_proc_status", value);

	return 0;
}

void
wps_ui_deinit()
{
	wps_hndl_del(&ui_hndl);
	wps_osl_ui_handle_deinit(ui_hndl.handle);
	return;
}

#if defined(MULTIAP)
/* Try map onboarding in another wireless sta interface */
void
wps_ui_map_try_pbc()
{
	char buf[512] = {0}, ifname[15] = {0}, *next;
	int len = 0;
	char *ifnames = wps_map_get_ifnames();
	char *mode = NULL, tmp[64] = {0};

	TUTRACE((TUTRACE_INFO, "Multiap timeout happend trying wps in "
		"other than %s interface!\n", wps_ui->wps_ifname));

	foreach(ifname, ifnames, next) {
		if (!strcmp(ifname, wps_ui->wps_ifname)) {
			continue;
		}
		snprintf(tmp, sizeof(tmp), "%s_mode", ifname);
		mode = wps_safe_get_conf(tmp);
		if (strcmp(mode, "sta")) {
			continue;
		}

		break;
	}

	if (ifname[0] != '\0') {
		wps_map_update_ifnames(wps_ui->wps_ifname);

		TUTRACE((TUTRACE_INFO, "Current wps session was in %s now "
			"trying in interface %s !\n", wps_ui->wps_ifname, ifname));

		len += sprintf(buf + len, "wps_config_command=\"%d\" ", WPS_UI_CMD_START);
		len += sprintf(buf + len, "wps_action=\"%d\" ", WPS_UI_ACT_ENROLL);
		len += sprintf(buf + len, "wps_method=\"%d\" ", WPS_UI_METHOD_PBC);
		len += sprintf(buf + len, "wps_pbc_method=\"%d\" ", WPS_UI_PBC_SW);
		len += sprintf(buf + len, "wps_ifname=%s ", ifname);

		wps_ui_do_set(buf);
	}
}
#endif	/* MULTIAP */
