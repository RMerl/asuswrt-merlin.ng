/*
 * Broadcom WPS module (for libupnp), soap_x_wfawlanconfig.c
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
 * $Id: soap_x_wfawlanconfig.c 738257 2017-12-27 22:59:37Z $
 */
#include <upnp.h>
#include <WFADevice.h>

/*
 * WARNNING: PLEASE IMPLEMENT YOUR CODES AFTER
 *          "<< USER CODE START >>"
 * AND DON'T REMOVE TAG :
 *          "<< AUTO GENERATED FUNCTION: "
 *          ">> AUTO GENERATED FUNCTION"
 *          "<< USER CODE START >>"
 */

/* << AUTO GENERATED FUNCTION: statevar_WLANResponse() */
static int
statevar_WLANResponse
(
	UPNP_CONTEXT * context,
	UPNP_SERVICE * service,
	UPNP_TLV * tlv
)
{
	HINT(BIN::tlv);

	/* << USER CODE START >> */
	return OK;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: statevar_WLANEventType() */
static int
statevar_WLANEventType
(
	UPNP_CONTEXT * context,
	UPNP_SERVICE * service,
	UPNP_TLV * tlv
)
{
	HINT(UI1::tlv)

	/* << USER CODE START >> */
	/*
	 * Simon note:
	 * Should find a way to solve this situation.
	 */
	/* upnp_tlv_set(tlv, WSC_WLAN_EVENT_TYPE_EAP_FRAME); */
	return OK;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: statevar_InMessage() */
static int
statevar_InMessage
(
	UPNP_CONTEXT * context,
	UPNP_SERVICE * service,
	UPNP_TLV * tlv
)
{
	HINT(BIN::tlv)

	/* << USER CODE START >> */
	return OK;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: statevar_OutMessage() */
static int
statevar_OutMessage
(
	UPNP_CONTEXT * context,
	UPNP_SERVICE * service,
	UPNP_TLV * tlv
)
{
	HINT(BIN::tlv)

	/* << USER CODE START >> */
	return OK;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: statevar_APSettings() */
static int
statevar_APSettings
(
	UPNP_CONTEXT * context,
	UPNP_SERVICE * service,
	UPNP_TLV * tlv
)
{
	HINT(BIN::tlv)

	/* << USER CODE START >> */
	char *NewAPSettings = "Sample Binary";
	int len = strlen(NewAPSettings);

	upnp_tlv_set_bin(tlv, (uintptr_t)NewAPSettings, len);
	return OK;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: statevar_Message() */
static int
statevar_Message
(
	UPNP_CONTEXT * context,
	UPNP_SERVICE * service,
	UPNP_TLV * tlv
)
{
	HINT(BIN::tlv)

	/* << USER CODE START >> */
	return OK;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: statevar_STAStatus() */
static int
statevar_STAStatus
(
	UPNP_CONTEXT * context,
	UPNP_SERVICE * service,
	UPNP_TLV * tlv
)
{
	HINT(UI1::tlv)

	/* Simon note:
	 * Should find a way to solve this situation.
	 */
	upnp_tlv_set(tlv, 0x1);
	return OK;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: statevar_WLANEventMAC() */
static int
statevar_WLANEventMAC
(
	UPNP_CONTEXT * context,
	UPNP_SERVICE * service,
	UPNP_TLV * tlv
)
{
	HINT(STR::tlv)

	/* << USER CODE START >> */
	upnp_tlv_set(tlv, (int)"");
	return OK;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: statevar_DeviceInfo() */
static int
statevar_DeviceInfo
(
	UPNP_CONTEXT * context,
	UPNP_SERVICE * service,
	UPNP_TLV * tlv
)
{
	HINT(BIN::tlv)

	/* << USER CODE START >> */
	return OK;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: statevar_STASettings() */
static int
statevar_STASettings
(
	UPNP_CONTEXT * context,
	UPNP_SERVICE * service,
	UPNP_TLV * tlv
)
{
	HINT(BIN::tlv)

	/* << USER CODE START >> */
	char *NewSTASettings = "Sample Binary";
	int len = strlen(NewSTASettings);

	upnp_tlv_set_bin(tlv, (uintptr_t)NewSTASettings, len);
	return OK;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: statevar_APStatus() */
static int
statevar_APStatus
(
	UPNP_CONTEXT * context,
	UPNP_SERVICE * service,
	UPNP_TLV * tlv
)
{
	HINT(UI1::tlv)

	/* << USER CODE START >> */
	/*
	 * Simon note:
	 * Should find a way to solve this situation.
	 */
	upnp_tlv_set(tlv, 0x1);
	return OK;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: statevar_WLANEvent() */
static int
statevar_WLANEvent
(
	UPNP_CONTEXT * context,
	UPNP_SERVICE * service,
	UPNP_TLV * tlv
)
{
	HINT(BIN::tlv)

	/* << USER CODE START >> */
	UPNP_STATE_VAR *statevar;
	UPNP_EVENT *event;

	/* Find the statevar */
	statevar = find_event_var(context, service, "WLANEvent");
	event = get_event(context, statevar);

	if (event == NULL)
		return ERROR;

	/* Do nothing if it is the same */
	if (tlv == &event->tlv)
		return OK;

	upnp_tlv_set_bin(tlv, (uintptr_t)event->tlv.val.bin, event->tlv.len);
	return OK;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: action_DelAPSettings() */
static int
action_DelAPSettings
(
	UPNP_CONTEXT * context,
	UPNP_SERVICE * service,
	IN_ARGUMENT * in_argument,
	OUT_ARGUMENT * out_argument
)
{
	HINT( BIN::UPNP_TLV *in_NewAPSettings = UPNP_IN_TLV("NewAPSettings"); )

	/* << USER CODE START >> */
	return OK;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: action_DelSTASettings() */
static int
action_DelSTASettings
(
	UPNP_CONTEXT * context,
	UPNP_SERVICE * service,
	IN_ARGUMENT * in_argument,
	OUT_ARGUMENT * out_argument
)
{
	HINT( BIN::UPNP_TLV *in_NewSTASettings = UPNP_IN_TLV("NewSTASettings"); )

	/* << USER CODE START >> */
	return OK;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: action_GetAPSettings() */
static int
action_GetAPSettings
(
	UPNP_CONTEXT * context,
	UPNP_SERVICE * service,
	IN_ARGUMENT * in_argument,
	OUT_ARGUMENT * out_argument
)
{
	HINT( BIN::UPNP_TLV *in_NewMessage = UPNP_IN_TLV("NewMessage"); )
	HINT( BIN::UPNP_TLV *out_NewAPSettings = UPNP_OUT_TLV("NewAPSettings"); )

	/* << USER CODE START >> */
	UPNP_TLV *out_NewAPSettings = UPNP_OUT_TLV("NewAPSettings");

	return statevar_APSettings(context, service, out_NewAPSettings);
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: action_GetDeviceInfo() */
static int
action_GetDeviceInfo
(
	UPNP_CONTEXT * context,
	UPNP_SERVICE * service,
	IN_ARGUMENT * in_argument,
	OUT_ARGUMENT * out_argument
)
{
	HINT( BIN::UPNP_TLV *out_NewDeviceInfo = UPNP_OUT_TLV("NewDeviceInfo"); )

	/* << USER CODE START >> */
	UPNP_TLV *out_NewDeviceInfo = UPNP_OUT_TLV("NewDeviceInfo");

	return wfa_GetDeviceInfo(context, out_NewDeviceInfo);
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: action_GetSTASettings() */
static int
action_GetSTASettings
(
	UPNP_CONTEXT * context,
	UPNP_SERVICE * service,
	IN_ARGUMENT * in_argument,
	OUT_ARGUMENT * out_argument
)
{
	HINT( BIN::UPNP_TLV *in_NewMessage = UPNP_IN_TLV("NewMessage"); )
	HINT( BIN::UPNP_TLV *out_NewSTASettings = UPNP_OUT_TLV("NewSTASettings"); )

	/* << USER CODE START >> */
	UPNP_TLV *out_NewSTASettings = UPNP_OUT_TLV("NewSTASettings");

	return statevar_STASettings(context, service, out_NewSTASettings);
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: action_PutMessage() */
static int
action_PutMessage
(
	UPNP_CONTEXT * context,
	UPNP_SERVICE * service,
	IN_ARGUMENT * in_argument,
	OUT_ARGUMENT * out_argument
)
{
	HINT( BIN::UPNP_TLV *in_NewInMessage = UPNP_IN_TLV("NewInMessage"); )
	HINT( BIN::UPNP_TLV *out_NewOutMessage = UPNP_OUT_TLV("NewOutMessage"); )

	/* << USER CODE START >> */
	UPNP_TLV *in_NewInMessage = UPNP_IN_TLV("NewInMessage");
	UPNP_TLV *out_NewOutMessage = UPNP_OUT_TLV("NewOutMessage");

	return wfa_PutMessage(context, in_NewInMessage, out_NewOutMessage);
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: action_PutWLANResponse() */
static int
action_PutWLANResponse
(
	UPNP_CONTEXT * context,
	UPNP_SERVICE * service,
	IN_ARGUMENT * in_argument,
	OUT_ARGUMENT * out_argument
)
{
	HINT( BIN::UPNP_TLV *in_NewMessage = UPNP_IN_TLV("NewMessage"); )
	HINT( UI1::UPNP_TLV *in_NewWLANEventType = UPNP_IN_TLV("NewWLANEventType"); )
	HINT( STR::UPNP_TLV *in_NewWLANEventMAC = UPNP_IN_TLV("NewWLANEventMAC"); )

	/* << USER CODE START >> */
	UPNP_TLV *in_NewMessage = UPNP_IN_TLV("NewMessage");

	return wfa_PutWLANResponse(context, in_NewMessage);
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: action_RebootAP() */
static int
action_RebootAP
(
	UPNP_CONTEXT * context,
	UPNP_SERVICE * service,
	IN_ARGUMENT * in_argument,
	OUT_ARGUMENT * out_argument
)
{
 	HINT( BIN::UPNP_TLV *in_NewAPSettings = UPNP_IN_TLV("NewAPSettings"); )

	/* << USER CODE START >> */
	return OK;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: action_RebootSTA() */
static int
action_RebootSTA
(
	UPNP_CONTEXT * context,
	UPNP_SERVICE * service,
	IN_ARGUMENT * in_argument,
	OUT_ARGUMENT * out_argument
)
{
  	HINT( BIN::UPNP_TLV *in_NewSTASettings = UPNP_IN_TLV("NewSTASettings"); )

	/* << USER CODE START >> */
	return OK;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: action_ResetAP() */
static int
action_ResetAP
(
	UPNP_CONTEXT * context,
	UPNP_SERVICE * service,
	IN_ARGUMENT * in_argument,
	OUT_ARGUMENT * out_argument
)
{
   	HINT( BIN::UPNP_TLV *in_NewMessage = UPNP_IN_TLV("NewMessage"); )

	/* << USER CODE START >> */
	return OK;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: action_ResetSTA() */
static int
action_ResetSTA
(
	UPNP_CONTEXT * context,
	UPNP_SERVICE * service,
	IN_ARGUMENT * in_argument,
	OUT_ARGUMENT * out_argument
)
{
    	HINT( BIN::UPNP_TLV *in_NewMessage = UPNP_IN_TLV("NewMessage"); )

	/* << USER CODE START >> */
	return OK;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: action_SetAPSettings() */
static int
action_SetAPSettings
(
	UPNP_CONTEXT * context,
	UPNP_SERVICE * service,
	IN_ARGUMENT * in_argument,
	OUT_ARGUMENT * out_argument
)
{
    	HINT( BIN::UPNP_TLV *in_NewAPSettings = UPNP_IN_TLV("NewAPSettings"); )

	/* << USER CODE START >> */
	return OK;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: action_SetSelectedRegistrar() */
static int
action_SetSelectedRegistrar
(
	UPNP_CONTEXT * context,
	UPNP_SERVICE * service,
	IN_ARGUMENT * in_argument,
	OUT_ARGUMENT * out_argument
)
{
	HINT( BIN::UPNP_TLV *in_NewMessage = UPNP_IN_TLV("NewMessage"); )

	/* << USER CODE START >> */
	UPNP_TLV *in_NewMessage = UPNP_IN_TLV("NewMessage");

	return wfa_SetSelectedRegistrar(context, in_NewMessage);
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: action_SetSTASettings() */
static int
action_SetSTASettings
(
	UPNP_CONTEXT * context,
	UPNP_SERVICE * service,
	IN_ARGUMENT * in_argument,
	OUT_ARGUMENT * out_argument
)
{
     	HINT( BIN::UPNP_TLV *out_NewSTASettings = UPNP_OUT_TLV("NewSTASettings"); )

	/* << USER CODE START >> */
	UPNP_TLV *out_NewSTASettings = UPNP_OUT_TLV("NewSTASettings");

	return statevar_STASettings(context, service, out_NewSTASettings);
}
/* >> AUTO GENERATED FUNCTION */

/* << TABLE BEGIN */
/*
 * WARNNING: DON'T MODIFY THE FOLLOWING TABLES
 * AND DON'T REMOVE TAG :
 *          "<< TABLE BEGIN"
 *          ">> TABLE END"
 */

#define	STATEVAR_WLANRESPONSE   0
#define	STATEVAR_WLANEVENTTYPE  1
#define	STATEVAR_INMESSAGE      2
#define	STATEVAR_OUTMESSAGE     3
#define	STATEVAR_APSETTINGS     4
#define	STATEVAR_MESSAGE        5
#define	STATEVAR_STASTATUS      6
#define	STATEVAR_WLANEVENTMAC   7
#define	STATEVAR_DEVICEINFO     8
#define	STATEVAR_STASETTINGS    9
#define	STATEVAR_APSTATUS       10
#define	STATEVAR_WLANEVENT      11

/* State Variable Table */
UPNP_STATE_VAR statevar_x_wfawlanconfig[] =
{
	{0, "WLANResponse",     UPNP_TYPE_BIN_BASE64,   &statevar_WLANResponse,	    0},
	{0, "WLANEventType",    UPNP_TYPE_UI1,          &statevar_WLANEventType,    0},
	{0, "InMessage",        UPNP_TYPE_BIN_BASE64,   &statevar_InMessage,        0},
	{0, "OutMessage",       UPNP_TYPE_BIN_BASE64,   &statevar_OutMessage,       0},
	{0, "APSettings",       UPNP_TYPE_BIN_BASE64,   &statevar_APSettings,       0},
	{0, "Message",          UPNP_TYPE_BIN_BASE64,   &statevar_Message,          0},
	{0, "STAStatus",        UPNP_TYPE_UI1,          &statevar_STAStatus,        1},
	{0, "WLANEventMAC",     UPNP_TYPE_STR,          &statevar_WLANEventMAC,     0},
	{0, "DeviceInfo",       UPNP_TYPE_BIN_BASE64,   &statevar_DeviceInfo,       0},
	{0, "STASettings",      UPNP_TYPE_BIN_BASE64,   &statevar_STASettings,      0},
	{0, "APStatus",         UPNP_TYPE_UI1,          &statevar_APStatus,         1},
	{0, "WLANEvent",        UPNP_TYPE_BIN_BASE64,   &statevar_WLANEvent,        1},
	{0, 0,                  0,                      0,                          0}
};

/* Action Table */
static ACTION_ARGUMENT arg_in_DelAPSettings [] =
{
	{"NewAPSettings",       UPNP_TYPE_BIN_BASE64,    STATEVAR_APSETTINGS}
};

static ACTION_ARGUMENT arg_in_DelSTASettings [] =
{
	{"NewSTASettings",      UPNP_TYPE_BIN_BASE64,    STATEVAR_STASETTINGS}
};

static ACTION_ARGUMENT arg_in_GetAPSettings [] =
{
	{"NewMessage",          UPNP_TYPE_BIN_BASE64,    STATEVAR_MESSAGE}
};

static ACTION_ARGUMENT arg_out_GetAPSettings [] =
{
	{"NewAPSettings",       UPNP_TYPE_BIN_BASE64,    STATEVAR_APSETTINGS}
};

static ACTION_ARGUMENT arg_out_GetDeviceInfo [] =
{
	{"NewDeviceInfo",       UPNP_TYPE_BIN_BASE64,    STATEVAR_DEVICEINFO}
};

static ACTION_ARGUMENT arg_in_GetSTASettings [] =
{
	{"NewMessage",          UPNP_TYPE_BIN_BASE64,    STATEVAR_MESSAGE}
};

static ACTION_ARGUMENT arg_out_GetSTASettings [] =
{
	{"NewSTASettings",      UPNP_TYPE_BIN_BASE64,    STATEVAR_STASETTINGS}
};

static ACTION_ARGUMENT arg_in_PutMessage [] =
{
	{"NewInMessage",        UPNP_TYPE_BIN_BASE64,    STATEVAR_INMESSAGE}
};

static ACTION_ARGUMENT arg_out_PutMessage [] =
{
	{"NewOutMessage",       UPNP_TYPE_BIN_BASE64,    STATEVAR_OUTMESSAGE}
};

static ACTION_ARGUMENT arg_in_PutWLANResponse [] =
{
	{"NewMessage",          UPNP_TYPE_BIN_BASE64,    STATEVAR_MESSAGE},
	{"NewWLANEventType",    UPNP_TYPE_UI1,           STATEVAR_WLANEVENTTYPE},
	{"NewWLANEventMAC",     UPNP_TYPE_STR,           STATEVAR_WLANEVENTMAC}
};

static ACTION_ARGUMENT arg_in_RebootAP [] =
{
	{"NewAPSettings",       UPNP_TYPE_BIN_BASE64,    STATEVAR_APSETTINGS}
};

static ACTION_ARGUMENT arg_in_RebootSTA [] =
{
	{"NewSTASettings",      UPNP_TYPE_BIN_BASE64,    STATEVAR_STASETTINGS}
};

static ACTION_ARGUMENT arg_in_ResetAP [] =
{
	{"NewMessage",          UPNP_TYPE_BIN_BASE64,    STATEVAR_MESSAGE}
};

static ACTION_ARGUMENT arg_in_ResetSTA [] =
{
	{"NewMessage",          UPNP_TYPE_BIN_BASE64,    STATEVAR_MESSAGE}
};

static ACTION_ARGUMENT arg_in_SetAPSettings [] =
{
	{"NewAPSettings",       UPNP_TYPE_BIN_BASE64,    STATEVAR_APSETTINGS}
};

static ACTION_ARGUMENT arg_in_SetSelectedRegistrar [] =
{
	{"NewMessage",          UPNP_TYPE_BIN_BASE64,    STATEVAR_MESSAGE}
};

static ACTION_ARGUMENT arg_out_SetSTASettings [] =
{
	{"NewSTASettings",      UPNP_TYPE_BIN_BASE64,    STATEVAR_STASETTINGS}
};

UPNP_ACTION action_x_wfawlanconfig[] =
{
	{"DelAPSettings",          1,   arg_in_DelAPSettings,         0,   0,                       &action_DelAPSettings},
	{"DelSTASettings",         1,   arg_in_DelSTASettings,        0,   0,                       &action_DelSTASettings},
	{"GetAPSettings",          1,   arg_in_GetAPSettings,         1,   arg_out_GetAPSettings,   &action_GetAPSettings},
	{"GetDeviceInfo",          0,   0,                            1,   arg_out_GetDeviceInfo,   &action_GetDeviceInfo},
	{"GetSTASettings",         1,   arg_in_GetSTASettings,        1,   arg_out_GetSTASettings,  &action_GetSTASettings},
	{"PutMessage",             1,   arg_in_PutMessage,            1,   arg_out_PutMessage,      &action_PutMessage},
	{"PutWLANResponse",        3,   arg_in_PutWLANResponse,       0,   0,                       &action_PutWLANResponse},
	{"RebootAP",               1,   arg_in_RebootAP,              0,   0,                       &action_RebootAP},
	{"RebootSTA",              1,   arg_in_RebootSTA,             0,   0,                       &action_RebootSTA},
	{"ResetAP",                1,   arg_in_ResetAP,               0,   0,                       &action_ResetAP},
	{"ResetSTA",               1,   arg_in_ResetSTA,              0,   0,                       &action_ResetSTA},
	{"SetAPSettings",          1,   arg_in_SetAPSettings,         0,   0,                       &action_SetAPSettings},
	{"SetSTASettings",         0,   0,                            1,   arg_out_SetSTASettings,  &action_SetSTASettings},
	{"SetSelectedRegistrar",   1,   arg_in_SetSelectedRegistrar,  0,   0,                       &action_SetSelectedRegistrar},
	{0,                        0,   0,                            0,   0,                       0}
};
/* >> TABLE END */
