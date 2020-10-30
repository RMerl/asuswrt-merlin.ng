/*
 * Broadcom Router WPS definitions
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
 * $Id$
 */

#ifndef __WPSDEFS_H__
#define __WPSDEFS_H__

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

#define WPS_1905_ADDR		"127.0.0.1"
#define WPS_UI_PORT		40500
#define WPS_1905_PORT		40600

/*WPS CONFIG METHODS*/
#define WPS_CONFIG_USBA 0x0001
#define WPS_CONFIG_ETHERNET 0x0002
#define WPS_CONFIG_LABEL 0x0004
#define WPS_CONFIG_DISPLAY 0x0008
#define WPS_CONFIG_EXT_NFC_TOKEN 0x0010
#define WPS_CONFIG_INT_NFC_TOKEN 0x0020
#define WPS_CONFIG_NFC_INTERFACE 0x0040
#define WPS_CONFIG_PUSHBUTTON 0x0080
#define WPS_CONFIG_KEYPAD 0x0100
#define WPS_CONFIG_VIRT_PUSHBUTTON 0x0280
#define WPS_CONFIG_PHY_PUSHBUTTON 0x0480
#define WPS_CONFIG_P2PS 0x1000
#define WPS_CONFIG_VIRT_DISPLAY 0x2008
#define WPS_CONFIG_PHY_DISPLAY 0x4008

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

typedef enum wps_1905_ret_status {
	WPS_ERROR_1905_SENDFAILURE=200,		/* Failed to send response M1/M2 to 1905 */

	/* wscprocess,getm2 */
	WPS_1905_M1HANDLE_M2DATA,
	WPS_1905_M1HANDLE_NOTREGISRAR,

	/* handle startautoconfig */
	WPS_1905_RESHANDLE_SESSIONOVERLAPPING,
	WPS_1905_RESHANDLE_M1DATA,

	/* wscprocess, configureap */
	WPS_1905_M2SET_DONE,			/* 1905M2 SETUP DONE */
	WPS_1905_M2SET_NOMATCHING,
	WPS_1905_M2SET_NOSESSION,		/* NO session opened for actions */
	WPS_1905_M2SET_NOAUTHENTICATOR,		/* NO AUTHENTICATOR INFO in M2 */

	/* handle mtype */
	WPS_1905_UNKNOWNWSCMESSAGE,

	/* wscenabled */
	WPS_1905_WSCAP_ENABLED,
	WPS_1905_WSCAP_DISABLED,

	/* wscstatus */
	WPS_1905_WSCAP_CONFIGURED,
	WPS_1905_WSCAP_UNCONFIGURED,
	WPS_1905_WSCAP_SESSIONONGOING
} WPS_1905_RET_STATUS;

typedef enum wps_1905_ctl_cmd
{
	WPS_1905_CTL_WSCSTATUS = 0,
	WPS_1905_CTL_WSCCANCEL,
	WPS_1905_CTL_WSCENABLED,
	WPS_1905_CTL_STARTAUTOCONF,
	WPS_1905_CTL_GETM2,
	WPS_1905_CTL_CONFIGUREAP,
	WPS_1905_CTL_WSCPROCESS,
	WPS_1905_CTL_WLINSTANCE,
	WPS_1905_CTL_CLIENT_REGISTER,
	WPS_1905_CTL_GETMINFO,
	WPS_1905_NOTIFY_CLIENT_RESTART,
	WPS_1905_CTL_PBC
} WPS_1905_CTL_CMD;

typedef struct wps_1905_message {
	WPS_1905_CTL_CMD cmd;
	char             ifName[32];
	int              len;
	int              status;
} WPS_1905_MESSAGE;

typedef struct wps_1905_m_message_info {
	char             id[5];
	int              mtype;
	int              rfband;
} WPS_1905_M_MESSAGE_INFO;

typedef enum  {
	WPS_1905_CONF_NOCHANGE_UNCONFIGURED,
	WPS_1905_CONF_NOCHANGE_CONFIGURED,
	WPS_1905_CONF_TO_UNCONF,
	WPS_1905_UNCONF_TO_CONF
} wps_1905_config_status_change;

typedef struct wps_1905_notify_message {
	char          ifName[32];
	unsigned char credentialChanged;
	unsigned char confStatus;
} WPS_1905_NOTIFY_MESSAGE;

#define WPS1905MSG_DATAPTR(pMessage) ((char *)(((void *)pMessage)+sizeof(WPS_1905_MESSAGE)))
#define WPS1905CMD_PTR(pMessage) (&(pMessage->cmd))

#endif /* __WPSDEFS_H__ */
