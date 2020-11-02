/*
 * Broadcom WPS module (for libupnp), WFADevice_table.c
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
 * $Id: WFADevice_table.c 270398 2011-07-04 06:31:51Z $
 */
#include <upnp.h>
#include <WFADevice.h>

/* << TABLE BEGIN */
/*
 * WARNNING: DON'T MODIFY THE FOLLOWING TABLES
 * AND DON'T REMOVE TAG :
 *          "<< TABLE BEGIN"
 *          ">> TABLE END"
 */

extern UPNP_ACTION action_x_wfawlanconfig[];

extern UPNP_STATE_VAR statevar_x_wfawlanconfig[];

static UPNP_SERVICE WFADevice_service [] =
{
	{"/control?WFAWLANConfig",	"/event?WFAWLANConfig",	"urn:schemas-wifialliance-org:service:WFAWLANConfig",	"WFAWLANConfig1",	action_x_wfawlanconfig,	statevar_x_wfawlanconfig},
	{0,				0,			0,							0,			0,			0}
};
static UPNP_ADVERTISE WFADevice_advertise [] =
{
	{"urn:schemas-wifialliance-org:device:WFADevice",		"00010203-0405-0607-0809-0a0b0c0d0ebb",	ADVERTISE_ROOTDEVICE},
	{"urn:schemas-wifialliance-org:service:WFAWLANConfig",		"00010203-0405-0607-0809-0a0b0c0d0ebb",	ADVERTISE_SERVICE},
	{0,															""}
};

extern char xml_WFADevice[];
extern char xml_x_wfawlanconfig[];

static UPNP_DESCRIPTION WFADevice_description [] =
{
	{"/WFADevice.xml",          "text/xml",     0,      xml_WFADevice},
	{"/x_wfawlanconfig.xml",    "text/xml",     0,      xml_x_wfawlanconfig},
	{0,                         0,              0,      0}
};

UPNP_DEVICE WFADevice =
{
	"WFADevice.xml",
	WFADevice_service,
	WFADevice_advertise,
	WFADevice_description,
	WFADevice_open,
	WFADevice_close,
	WFADevice_timeout,
	WFADevice_notify,
	WFADevice_scbrchk
};
/* >> TABLE END */
