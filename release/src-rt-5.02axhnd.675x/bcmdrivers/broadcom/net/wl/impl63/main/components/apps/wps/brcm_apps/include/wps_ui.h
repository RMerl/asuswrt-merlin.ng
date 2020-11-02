/*
 * WPS environment variables
 *
 * Copyright (C) 2020, Broadcom. All Rights Reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: wps_ui.h 768362 2018-10-11 06:45:41Z $
 */

#ifndef __WPS_UI_H__
#define __WPS_UI_H__

/*
 * WPS module
 */
#define WPS_EAP_ADDR			"127.0.0.1"
#define WPS_UPNPDEV_ADDR		"127.0.0.1"
#define WPS_UI_ADDR				"127.0.0.1"

#define WPS_UPNPDEV_PORT		40000
#define WPS_UI_PORT			40500

/* WPS_UI definitions */
#define WPS_UI_CMD_NONE			0
#define WPS_UI_CMD_START		1
#define WPS_UI_CMD_STOP			2
#define WPS_UI_CMD_NFC_WR_CFG		3
#define WPS_UI_CMD_NFC_RD_CFG		4
#define WPS_UI_CMD_NFC_WR_PW		5
#define WPS_UI_CMD_NFC_RD_PW		6
#define WPS_UI_CMD_NFC_HO_S		7
#define WPS_UI_CMD_NFC_HO_R		8
#define WPS_UI_CMD_NFC_FM		9
#define WPS_UI_CMD_MSGLEVEL		99

#define IS_WPS_UI_CMD_NFC(cmd)	((cmd) == WPS_UI_CMD_NFC_WR_CFG || \
				 (cmd) == WPS_UI_CMD_NFC_RD_CFG || \
				 (cmd) == WPS_UI_CMD_NFC_WR_PW || \
				 (cmd) == WPS_UI_CMD_NFC_RD_PW || \
				 (cmd) == WPS_UI_CMD_NFC_HO_S || \
				 (cmd) == WPS_UI_CMD_NFC_HO_R || \
				 (cmd) == WPS_UI_CMD_NFC_FM)

#define WPS_UI_METHOD_NONE		0
#define WPS_UI_METHOD_PIN		1
#define WPS_UI_METHOD_PBC		2
#define WPS_UI_METHOD_NFC_PW		3
#define WPS_UI_METHOD_NFC_CHO		4

#define WPS_UI_ACT_NONE			0
#define WPS_UI_ACT_ENROLL		1
#define WPS_UI_ACT_CONFIGAP		2
#define WPS_UI_ACT_ADDENROLLEE		3
#define WPS_UI_ACT_STA_CONFIGAP		4
#define WPS_UI_ACT_STA_GETAPCONFIG	5

#define WPS_UI_PBC_NONE			0
#define WPS_UI_PBC_HW			1
#define WPS_UI_PBC_SW			2

/* For WPS module save in nvram and share with others, like GUI */
typedef enum {
	WPS_UI_INIT			= 0,	/* Idle and ready to be initiated */
	WPS_UI_ASSOCIATED		= 1,	/* Any request event was detected, example PBC */
	WPS_UI_OK			= 2,	/* WPS procedure (M1 ~ M8) was successfully done */
	WPS_UI_MSG_ERR			= 3,	/* Any error during WPS procedure */
	WPS_UI_TIMEOUT			= 4,	/* WPS procedure was incomplete within timeout */
	WPS_UI_SENDM2			= 5,	/* Send M2 msg */
	WPS_UI_SENDM7			= 6,	/* Send M7 msg */
	WPS_UI_MSGDONE			= 7,	/* WPS DONE msg was processed completely */
	WPS_UI_PBCOVERLAP		= 8,	/* PBC overlap detected */
	WPS_UI_FIND_PBC_AP		= 9,	/* Found AP with selregistar true in PBC method */
	WPS_UI_FIND_SEL_AP		= 10,	/* WPS sta associate to AP after finding PBC AP */
	WPS_UI_ASSOCIATING		= 11,	/* Found AP with selregistar true in PIN method */
	WPS_UI_NFC_WR_CFG		= 12,
	WPS_UI_NFC_WR_PW		= 13,
	WPS_UI_NFC_WR_CPLT		= 14,
	WPS_UI_NFC_RD_CFG		= 15,
	WPS_UI_NFC_RD_PW		= 16,
	WPS_UI_NFC_RD_CPLT		= 17,
	WPS_UI_NFC_HO_S			= 18,
	WPS_UI_NFC_HO_R			= 19,
	WPS_UI_NFC_HO_NDEF		= 20,
	WPS_UI_NFC_HO_CPLT		= 21,
	WPS_UI_NFC_OP_ERROR		= 22,
	WPS_UI_NFC_OP_STOP		= 23,
	WPS_UI_NFC_OP_TO		= 24,
	WPS_UI_NFC_FM			= 25,
	WPS_UI_NFC_FM_CPLT		= 26,
	WPS_UI_NFC_HO_DPI_MISMATCH	= 27,
	WPS_UI_NFC_HO_PKH_MISMATCH	= 28,
	WPS_UI_MAP_TIMEOUT		= 29	/* Multiap timeout occured in WPS sta */
} WPS_UI_SCSTATE;

typedef enum {
	WPS_UI_NFC_STATUS_INITING = -1,
	WPS_UI_NFC_STATUS_INITED = 0
} WPS_UI_NFC_STATUS;

int wps_ui_is_pending();
void wps_ui_clear_pending();
int wps_ui_pending_expire();
int wps_ui_init();
void wps_ui_deinit();
char *wps_ui_get_env(char *name);
void wps_ui_set_env(char *name, char *value);
void wps_ui_reset_env();
int wps_ui_process_msg(char *buf, int buflen);

#ifdef WPS_ADDCLIENT_WWTP
int wps_ui_is_SET_cmd(char *buf, int buflen);
void wps_ui_close_addclient_window();
void wps_ui_wer_override_active(bool active);
#endif // endif

#ifdef WPS_NFC_DEVICE
void wps_ui_nfc_open_session();
#endif // endif

#if defined(MULTIAP)
void wps_ui_map_try_pbc();
#endif	/* MULTIAP */
#endif	/* __WPS_UI_H__ */
