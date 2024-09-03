/*
 * Wireless Application Event Service
 * shared header file
 *
 * Copyright (C) 2024, Broadcom. All Rights Reserved.
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
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: $
 */

#ifndef _appeventd_h_
#define _appeventd_h_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <typedefs.h>
#include <bcmnvram.h>
#include <bcmutils.h>
#include <bcmtimer.h>
#include <bcmendian.h>
#include <shutils.h>
#include <security_ipc.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <appevent_hdr.h>

extern int appeventd_debug_level;

#define APPEVENTD_DEBUG_ERROR	0x0001
#define APPEVENTD_DEBUG_WARNING	0x0002
#define APPEVENTD_DEBUG_INFO	0x0004
#define APPEVENTD_DEBUG_DETAIL	0x0008

#define APPEVENTD_ERROR(fmt, arg...) \
		do { if (appeventd_debug_level & APPEVENTD_DEBUG_ERROR) \
			printf("APPEVENTD >> "fmt, ##arg); } while (0)

#define APPEVENTD_WARNING(fmt, arg...) \
		do { if (appeventd_debug_level & APPEVENTD_DEBUG_WARNING) \
			printf("APPEVENTD >> "fmt, ##arg); } while (0)

#define APPEVENTD_INFO(fmt, arg...) \
		do { if (appeventd_debug_level & APPEVENTD_DEBUG_INFO) \
			printf("APPEVENTD >> "fmt, ##arg); } while (0)

#define APPEVENTD_DEBUG(fmt, arg...) \
		do { if (appeventd_debug_level & APPEVENTD_DEBUG_DETAIL) \
			printf("APPEVENTD >> "fmt, ##arg); } while (0)

#define APPEVENTD_BUFSIZE	2048

#define APPEVENTD_OK	0
#define APPEVENTD_FAIL -1

/* WiFi Application : Band Steering (BSD) Event IDs */
/*
 * BSD Indicates : a CLIENT Band Steering Started from Source BSS
 */
#define APP_E_BSD_STEER_START				1
/*
 * BSD Indicates : a CLIENT Band Steering Completed (Success/Failure)
 */
#define APP_E_BSD_STEER_END				2
/*
 * BSD Indicates : When a RADIO/STA Stats query is made
 */
#define APP_E_BSD_STATS_QUERY				3

/* WiFi Application : SmartMesh/Wi-Fi Blanket (WBD) Event IDs */
/*
 * Agent Indicates : a CLIENT Link has became Weak Link in SmartMesh Network
 */
#define APP_E_WBD_SLAVE_WEAK_CLIENT			4
/*
 * Agent Indicates : a CLIENT Steering Started from Source BSS via BTM Req or Brute Force
 */
#define APP_E_WBD_SLAVE_STEER_START			5
/*
 * Agent Indicates : a CLIENT has sent BTM Response for a BTM(Steer) Request to Source BSS
 */
#define APP_E_WBD_SLAVE_STEER_RESP			6
/*
 * Controller detected : a CLIENT Link has became Weak Link in SmartMesh Network
 */
#define APP_E_WBD_MASTER_WEAK_CLIENT			7
/*
 * Controller detected : a CLIENT Steering Started from Source BSS via BTM Req or Brute Force
 */
#define APP_E_WBD_MASTER_STEER_START			8
/*
 * Controller detected : a CLIENT has sent BTM Response for a BTM(Steer) Request to Source BSS
 */
#define APP_E_WBD_MASTER_STEER_RESP			9
/*
 * Controller detected : a CLIENT Steering Completed (Success/Failure)
 */
#define APP_E_WBD_MASTER_STEER_END			10
/*
 * Controller Indicates : SmartMesh Initialization Sequnce has Started with a Timestamp
 */
#define APP_E_WBD_MASTER_MAP_INIT_START			11
/*
 * Controller Indicates : SmartMesh Initialization Sequnce has Completed with a Timestamp
 */
#define APP_E_WBD_MASTER_MAP_INIT_END			12
/*
 * Agent Indicates : SmartMesh Initialization Sequnce has Completed with a Timestamp
 */
#define APP_E_WBD_SLAVE_MAP_INIT_END			13
/*
 * Controller detected : another Controller in 1905 AP-Autoconfiguration Search Resp Message
 */
#define APP_E_WBD_MASTER_ANOTHER_CONTROLLER_FOUND	14
/*
 * Agent Indicates : a CLIENT Link has became Strong Link in SmartMesh Network
 */
#define APP_E_WBD_SLAVE_STRONG_CLIENT			15
/*
 * Controller detected : a CLIENT Link has became Strong Link in SmartMesh Network
 */
#define APP_E_WBD_MASTER_STRONG_CLIENT			16
/*
 * Controller detected : an Agent has joined or left SmartMesh Network
 */
#define APP_E_WBD_MASTER_AGENT_STATUS_CHANGED		17
/*
 * Agent detected : the Controller has joined or left SmartMesh Network
 */
#define APP_E_WBD_SLAVE_CONTROLLER_STATUS_CHANGED	18
/*
 * Controller detected : a CLIENT has joined or left SmartMesh Network
 */
#define APP_E_WBD_MASTER_CLIENT_STATUS_CHANGED		19
/*
 * Agent detected : another Agent has joined or left SmartMesh Network
 */
#define APP_E_WBD_SLAVE_AGENT_STATUS_CHANGED		20
/*
 * Agent detected : a CLIENT has joined or left SmartMesh Network
 */
#define APP_E_WBD_SLAVE_CLIENT_STATUS_CHANGED		21

/* Last WiFi Application Event ID */
#define APP_E_LAST					22

/* AppEvent Dispatch Framework */
#define APPEVENTD_EVENT_MASK_LEN	(ROUNDUP(APP_E_LAST, NBBY)/NBBY)

typedef struct appeventd_app {
	int	port_no; /* Each app set port no to receive appevent data */
	uchar	bitvec[APPEVENTD_EVENT_MASK_LEN]; /* Each app choose appevents it needs */
} appeventd_app_t;

typedef struct appeventd_wksp {
	appeventd_app_t		qosmgmt; /* QosMgmt App Data */
} appeventd_wksp_t;

#endif /* _appeventd_h_ */
