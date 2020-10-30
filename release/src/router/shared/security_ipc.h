/*
 * Broadcom security module ipc ports file
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
 * $Id: security_ipc.h 789955 2020-08-12 10:21:09Z $
 */

#ifndef __SECURITY_IPC_H__
#define __SECURITY_IPC_H__

#ifdef RTCONFIG_HND_ROUTER_AX
#include <ethernet.h>
#include <802.11.h>
#include <bcmevent.h>
#endif

/*
 * WAI module
 */
#define WAI_UI_ADDR			"127.0.0.1"
#define WAP_UI_PORT			9002

/*
 * AS module
 */
#define AS_UI_ADDR			"127.0.0.1"
#define AS_UI_PORT			9001
#define AS_WAI_PORT			3810

/*
 * EAP module
 */
#define EAPD_WKSP_UDP_ADDR		"127.0.0.1"

/* get_ifname_unit() index is << 4 */
#define EAPD_WKSP_PORT_INDEX_SHIFT	4
#define EAPD_WKSP_SPORT_OFFSET		(1 << 5)
#define EAPD_WKSP_MPORT_OFFSET		(1 << 6)
#define EAPD_WKSP_VX_PORT_OFFSET	(1 << 7)

#define EAPD_WKSP_WPS_UDP_PORT		37000
#define EAPD_WKSP_WPS_UDP_RPORT		EAPD_WKSP_WPS_UDP_PORT
#define EAPD_WKSP_WPS_UDP_SPORT		EAPD_WKSP_WPS_UDP_PORT + EAPD_WKSP_SPORT_OFFSET
#define EAPD_WKSP_WPS_UDP_MPORT		EAPD_WKSP_WPS_UDP_PORT + EAPD_WKSP_MPORT_OFFSET

#define EAPD_WKSP_NAS_UDP_PORT		38000
#define EAPD_WKSP_NAS_UDP_RPORT		EAPD_WKSP_NAS_UDP_PORT
#define EAPD_WKSP_NAS_UDP_SPORT		EAPD_WKSP_NAS_UDP_PORT + EAPD_WKSP_SPORT_OFFSET

#define EAPD_WKSP_SES_UDP_PORT		39000
#define EAPD_WKSP_SES_UDP_RPORT		EAPD_WKSP_SES_UDP_PORT
#define EAPD_WKSP_SES_UDP_SPORT		EAPD_WKSP_SES_UDP_PORT + EAPD_WKSP_SPORT_OFFSET

#define EAPD_WKSP_WAI_UDP_PORT		41000
#define EAPD_WKSP_WAI_UDP_RPORT 	EAPD_WKSP_WAI_UDP_PORT
#define EAPD_WKSP_WAI_UDP_SPORT 	EAPD_WKSP_WAI_UDP_PORT + EAPD_WKSP_SPORT_OFFSET

#define EAPD_WKSP_DCS_UDP_PORT		42000
#define EAPD_WKSP_DCS_UDP_RPORT 	EAPD_WKSP_DCS_UDP_PORT
#define EAPD_WKSP_DCS_UDP_SPORT 	EAPD_WKSP_DCS_UDP_PORT + EAPD_WKSP_SPORT_OFFSET

#define EAPD_WKSP_DIF_UDP_PORT		43000

#define EAPD_WKSP_MEVENT_UDP_PORT	44000
#define EAPD_WKSP_MEVENT_UDP_RPORT 	EAPD_WKSP_MEVENT_UDP_PORT
#define EAPD_WKSP_MEVENT_UDP_SPORT 	EAPD_WKSP_MEVENT_UDP_PORT + EAPD_WKSP_SPORT_OFFSET

#define EAPD_WKSP_BSD_UDP_PORT		45000
#define EAPD_WKSP_BSD_UDP_RPORT 	EAPD_WKSP_BSD_UDP_PORT
#define EAPD_WKSP_BSD_UDP_SPORT 	EAPD_WKSP_BSD_UDP_PORT + EAPD_WKSP_SPORT_OFFSET
#define EAPD_WKSP_BSD_UDP_MPORT 	EAPD_WKSP_BSD_UDP_PORT + EAPD_WKSP_MPORT_OFFSET
#define EAPD_WKSP_BSD_CLI_PORT		EAPD_WKSP_BSD_UDP_PORT + 65

#define EAPD_WKSP_SSD_UDP_PORT		46000
#define EAPD_WKSP_SSD_UDP_RPORT 	EAPD_WKSP_SSD_UDP_PORT
#define EAPD_WKSP_SSD_UDP_SPORT 	EAPD_WKSP_SSD_UDP_PORT + EAPD_WKSP_SPORT_OFFSET

#define EAPD_WKSP_EVENTD_UDP_PORT	47000
#define EAPD_WKSP_EVENTD_UDP_RPORT 	EAPD_WKSP_EVENTD_UDP_PORT
#define EAPD_WKSP_EVENTD_UDP_SPORT 	EAPD_WKSP_EVENTD_UDP_PORT + EAPD_WKSP_SPORT_OFFSET

#define EAPD_WKSP_ASPM_UDP_PORT		48000
#define EAPD_WKSP_ASPM_UDP_RPORT	EAPD_WKSP_ASPM_UDP_PORT
#define EAPD_WKSP_ASPM_UDP_SPORT	EAPD_WKSP_ASPM_UDP_PORT + EAPD_WKSP_SPORT_OFFSET
#define EAPD_WKSP_ASPM_UDP_MPORT	EAPD_WKSP_ASPM_UDP_PORT + EAPD_WKSP_MPORT_OFFSET

#define EAPD_WKSP_VISDCOLL_UDP_PORT	49000
#define EAPD_WKSP_VISDCOLL_UDP_RPORT	EAPD_WKSP_VISDCOLL_UDP_PORT
#define EAPD_WKSP_VISDCOLL_UDP_SPORT	EAPD_WKSP_VISDCOLL_UDP_PORT + EAPD_WKSP_SPORT_OFFSET

#define EAPD_WKSP_WBD_UDP_PORT		50000
#define EAPD_WKSP_WBD_UDP_RPORT		EAPD_WKSP_WBD_UDP_PORT
#define EAPD_WKSP_WBD_UDP_SPORT		EAPD_WKSP_WBD_UDP_PORT + EAPD_WKSP_SPORT_OFFSET
#define EAPD_WKSP_WBD_UDP_MPORT		EAPD_WKSP_WBD_UDP_PORT + EAPD_WKSP_MPORT_OFFSET
#define EAPD_WKSP_WBD_TCP_MASTERCLI_PORT	EAPD_WKSP_WBD_UDP_PORT + 256
#define EAPD_WKSP_WBD_TCP_SLAVECLI_PORT		EAPD_WKSP_WBD_UDP_PORT + 257
#define EAPD_WKSP_WBD_TCP_MASTER_PORT	EAPD_WKSP_WBD_UDP_PORT + 258
#define EAPD_WKSP_WBD_TCP_SLAVE_PORT	EAPD_WKSP_WBD_UDP_PORT + 259
#define EAPD_WKSP_WBD_AGENTCLI_PORT	EAPD_WKSP_WBD_UDP_PORT + 260
#define EAPD_WKSP_WBD_CTRLCLI_PORT	EAPD_WKSP_WBD_UDP_PORT + 261
#define EAPD_WKSP_WBD_EVENT_PORT	EAPD_WKSP_WBD_UDP_PORT + 262
#define ACSD_DEFAULT_CLI_PORT		5916
#define ESCAND_DEFAULT_CLI_PORT		5917

#define EAPD_WKSP_CEVENT_UDP_PORT      52000
#define EAPD_WKSP_CEVENT_UDP_RPORT     EAPD_WKSP_CEVENT_UDP_PORT
#define EAPD_WKSP_CEVENT_UDP_SPORT     EAPD_WKSP_CEVENT_UDP_PORT + EAPD_WKSP_SPORT_OFFSET

#define EAPD_WKSP_AIRIQ_UDP_PORT       53000
#define EAPD_WKSP_AIRIQ_UDP_RPORT      EAPD_WKSP_AIRIQ_UDP_PORT
#define EAPD_WKSP_AIRIQ_UDP_SPORT      EAPD_WKSP_AIRIQ_UDP_PORT + EAPD_WKSP_SPORT_OFFSET

#define EAPD_WKSP_LTE_U_UDP_PORT       54000
#define EAPD_WKSP_LTE_U_UDP_RPORT      EAPD_WKSP_LTE_U_UDP_PORT
#define EAPD_WKSP_LTE_U_UDP_SPORT      EAPD_WKSP_LTE_U_UDP_PORT + EAPD_WKSP_SPORT_OFFSET

#define EAPD_WKSP_WLEVENT_UDP_PORT	58000
#define EAPD_WKSP_WLEVENT_UDP_RPORT	EAPD_WKSP_WLEVENT_UDP_PORT
#define EAPD_WKSP_WLEVENT_UDP_SPORT	EAPD_WKSP_WLEVENT_UDP_PORT + EAPD_WKSP_SPORT_OFFSET

#define EAPD_WKSP_WLCEVENTD_UDP_PORT	59000
#define EAPD_WKSP_WLCEVENTD_UDP_RPORT 	EAPD_WKSP_WLCEVENTD_UDP_PORT
#define EAPD_WKSP_WLCEVENTD_UDP_SPORT 	EAPD_WKSP_WLCEVENTD_UDP_PORT + EAPD_WKSP_SPORT_OFFSET
#define EAPD_WKSP_WLCEVENTD_UDP_MPORT 	EAPD_WKSP_WLCEVENTD_UDP_PORT + EAPD_WKSP_MPORT_OFFSET

#define EAPD_WKSP_WBD_UDP_PORT		50000
#define EAPD_WKSP_WBD_UDP_RPORT		EAPD_WKSP_WBD_UDP_PORT
#define EAPD_WKSP_WBD_UDP_SPORT		EAPD_WKSP_WBD_UDP_PORT + EAPD_WKSP_SPORT_OFFSET

#define CEVENT_CLI_SERVER_UDP_PORT	55000

#define EAPD_WKSP_EVT_UDP_PORT		56000
#define EAPD_WKSP_EVT_UDP_RPORT		EAPD_WKSP_EVT_UDP_PORT
#define EAPD_WKSP_EVT_UDP_SPORT		EAPD_WKSP_EVT_UDP_PORT + EAPD_WKSP_SPORT_OFFSET

#define EAPD_WKSP_ECBD_UDP_PORT         57000
#define EAPD_WKSP_ECBD_UDP_RPORT        EAPD_WKSP_ECBD_UDP_PORT
#define EAPD_WKSP_ECBD_UDP_SPORT        EAPD_WKSP_ECBD_UDP_PORT + EAPD_WKSP_SPORT_OFFSET

#define EAPD_WKSP_RGD_UDP_PORT         58000
#define EAPD_WKSP_RGD_UDP_RPORT        EAPD_WKSP_RGD_UDP_PORT
#define EAPD_WKSP_RGD_UDP_SPORT        EAPD_WKSP_RGD_UDP_PORT + EAPD_WKSP_SPORT_OFFSET

/*
 * UPNP module
 */
#define	UPNP_IPC_ADDR			"127.0.0.1"
#define UPNP_WFA_ADDR			"127.0.0.1"

#define UPNP_IPC_PORT			40100
#define UPNP_WFA_PORT			40040		/* WFA wlan receive port */

/* WPS UPNP definitions */
#define UPNP_WPS_TYPE_SSR		1		/* Set Selected Registrar */
#define UPNP_WPS_TYPE_PMR		2		/* Wait For Put Message Resp */
#define UPNP_WPS_TYPE_GDIR		3		/* Wait For Get DevInfo Resp */
#define UPNP_WPS_TYPE_PWR		4		/* Put WLAN Response */
#define UPNP_WPS_TYPE_WE		5		/* WLAN Event */
#define UPNP_WPS_TYPE_QWFAS		6		/* Query WFAWLANConfig Subscribers */
#define UPNP_WPS_TYPE_DISCONNECT	7		/* Subscriber unreachable */
#define UPNP_WPS_TYPE_MAX		8

typedef struct {
	unsigned int type;
	unsigned char dst_addr[16];
	unsigned int length;
	unsigned char data[1];
} UPNP_WPS_CMD;

#define UPNP_WPS_CMD_SIZE		24

/*
 * WPS module
 */
#define WPS_EAP_ADDR			"127.0.0.1"
#define WPS_UPNPDEV_ADDR		"127.0.0.1"

#define WPS_UPNPDEV_PORT		40000

/* Wireless Application Event Service (appeventd) */
#define APPS_EVENT_UDP_PORT		40200

/* all application uses this shared function to send event to appeventd */
extern int app_event_sendup(int event_id, int status, unsigned char *data, int data_len);

/* send given 'data' of 'data_len' to the loopback interface over the mentioned UDP 'port' */
extern int send_to_port(uint32 port, unsigned char *data, int data_len);

/* builds bcm_event_t and sends to the loopback interface over the mentioned UDP 'port' */
extern void send_event_to_port(uint32 port, char *ifname,
		struct ether_addr *lan_ea, struct ether_addr *sta_ea,
		uint32 status, uint32 reason, uint32 auth_type, uint32 ev_type,
		void *ev_payload, uint32 ev_payload_len);

#ifdef BCM_CEVENT

/* This funcitons builds wl_cevent_t and sends it as event payload to ceventd using
 * send_event_to_port()
 */
extern void send_cevent(char *ifname, struct ether_addr *lan_ea, struct ether_addr *sta_ea,
		uint32 status, uint32 reason, uint32 auth_type, uint32 ce_type,
		uint32 ce_subtype, uint32 ce_flags, void *ce_data, size_t ce_data_len);

/* Macro for A2C that uses null/0 for lan_ea, sta_ea, auth_type in send_cevent() */
#define SEND_CEVENT_A2C_EXT(ifname, status, reason, ce_subtype, ce_flags, ce_data, ce_data_len) \
	send_cevent(ifname, NULL, NULL, status, reason, 0, CEVENT_TYPE_A2C, \
			ce_subtype, ce_flags, ce_data, ce_data_len)

/* Macro that uses zero for status, reason in SEND_CEVENT_A2C_EXT() */
#define SEND_CEVENT_A2C(ifname, ce_subtype, ce_data, ce_data_len) \
	SEND_CEVENT_A2C_EXT(ifname, 0, 0, ce_subtype, 0, ce_data, ce_data_len)

#endif /* BCM_CEVENT */

#endif	/* __SECURITY_IPC_H__ */
