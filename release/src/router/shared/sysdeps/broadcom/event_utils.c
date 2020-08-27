/*
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
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * Connectivity Debugging Framework - Shared utility function(s)
 *
 * $Id: event_utils.c 788123 2020-06-22 10:44:39Z $
 */

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

#ifdef BCMDBG
#define EVENT_UTILS_DBG(fmt, arg...) printf("%s: "fmt, __FUNCTION__ , ## arg)
#else
#define EVENT_UTILS_DBG(fmt, arg...)
#endif /* BCMDBG */

#define ETHER_TYPE_BRCM 0x886c

/* send given 'data' of 'data_len' to the loopback interface over the mentioned UDP 'port' */
int
send_to_port(uint32 port, unsigned char *data, int data_len)
{
	int reuse = 1;
	struct sockaddr_in addr;
	int sock;

	/* open socket */
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0) {
		EVENT_UTILS_DBG("Error: UDP Open failed.\n");
		return -1;
	}

	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse,
		sizeof(reuse)) < 0) {
		EVENT_UTILS_DBG("Error: UDP setsockopt failed.\n");
		close(sock);
		return -1;
	}

	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(port);
	if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		EVENT_UTILS_DBG("Error: UDP Bind failed, close eventd appSocket %d\n", sock);
		close(sock);
		return -1;
	}

	int sentBytes = 0;
	struct sockaddr_in to;

	to.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	to.sin_family = AF_INET;
	to.sin_port = htons(port);

	sentBytes = sendto(sock, (char*)data, data_len, 0,
			(struct sockaddr *)&to, sizeof(struct sockaddr_in));

	if (sentBytes != data_len) {
		EVENT_UTILS_DBG("Error: UDP send failed; sentBytes = %d\n", sentBytes);
	}

	close(sock);

	return 0;
}

/* send bcm event
 * Encapculation(s):
 * bcm_event_t
 *	struct ether_header, bcmeth_hdr_t
 *	wl_event_msg_t
 *		version, flags, event_type, status, reason, auth_type, data_len,
 *		addr, ifname, ifidx, bsscfgidx
 *	event payload (optional)
 */
void
send_event_to_port(uint32 port, char *ifname,
		struct ether_addr *lan_ea, struct ether_addr *sta_ea,
		uint32 status, uint32 reason, uint32 auth_type, uint32 ev_type,
		void *ev_payload, uint32 ev_payload_len)
{
	bcm_event_t *be;
	wl_event_msg_t *ev;
	unsigned char *buf;
	size_t buf_len;

	if ((!ev_payload) != (!ev_payload_len)) {
		EVENT_UTILS_DBG("Error: Mismatching ev_payload ptr, ev_payload_len %d.\n",
				ev_payload_len);
		return;
	}

	buf_len = sizeof(*be) + ev_payload_len;

	if (!(buf = (unsigned char *) calloc(buf_len, 1))) {
		EVENT_UTILS_DBG("Error: memory allocation for bcm_event_t failed\n");
		return;
	}
	be = (bcm_event_t *) buf;
	ev = &be->event;

	if (ev_payload) {
		memcpy((be + 1), ev_payload, ev_payload_len);
	}

	/* Update wl_event_msg_t  */
	ev->version = hton16(BCM_EVENT_MSG_VERSION);
	ev->flags = 0;
	ev->event_type = hton32(ev_type);
	ev->status = hton32(status);
	ev->reason = hton32(reason);
	ev->auth_type = hton32(auth_type);
	ev->datalen = hton32(ev_payload_len);
	if (sta_ea) {
		bcopy(sta_ea->octet, &(ev->addr), ETHER_ADDR_LEN);
	}
	if (ifname && ifname[0]) {
		memcpy(&(ev->ifname), ifname, BCM_MSG_IFNAME_MAX);
	}
	ev->ifidx = 0;
	ev->flags |= WLC_EVENT_MSG_UNKIF;
	ev->bsscfgidx = 0;
	ev->flags |= WLC_EVENT_MSG_UNKBSS;

	/* Update bcmeth_hdr_t - BCM Vendor specific header... */
	be->bcm_hdr.subtype = hton16(BCMILCP_SUBTYPE_VENDOR_LONG);
	/* Incorrect assignement of bcm_hdr.length! Picked it from driver code */
	be->bcm_hdr.length = hton16(BCMILCP_BCM_SUBTYPEHDR_MINLENGTH + BCM_MSG_LEN);
	be->bcm_hdr.version = hton16(BCMILCP_BCM_SUBTYPEHDR_VERSION);
	bcopy(BRCM_OUI, &(be->bcm_hdr.oui[0]), DOT11_OUI_LEN);

	/* Update bcmevent ethernet hdr. For now, src and dst of eth hdr are same */
	if (lan_ea) {
		memcpy(&(be->eth.ether_dhost), lan_ea->octet, ETHER_ADDR_LEN);
		memcpy(&(be->eth.ether_shost), lan_ea->octet, ETHER_ADDR_LEN);
	}
	be->eth.ether_type = hton16(ETHER_TYPE_BRCM);

	send_to_port(port, buf, buf_len);

	free(buf);
}

#ifdef BCM_CEVENT
/*
 * cevent in bcm event
 * Encapculation(s):
 * bcm_event_t
 *	struct ether_header, bcmeth_hdr_t
 *	wl_event_msg_t
 *		version, flags, event_type, status, reason, auth_type, data_len,
 *		addr, ifname, ifidx, bsscfgidx
 *	wl_cevent_t (if event_type is WLC_E_CEVENT)
 *		version, length, type, data_offset, subtype, flags, timestamp, ...,
 *		data (cevent type dependent payload)
 *
 * This funcitons builds wl_cevent_t and sends it as event payload to ceventd using
 * send_event_to_port()
 */
void
send_cevent(char *ifname, struct ether_addr *lan_ea, struct ether_addr *sta_ea,
		uint32 status, uint32 reason, uint32 auth_type, uint32 ce_type,
		uint32 ce_subtype, uint32 ce_flags, void *ce_data, size_t ce_data_len)
{
	struct timespec tp;
	uint64 time_in_ms = 0;
	wl_cevent_t *ce;
	unsigned char *buf;
	size_t buf_len;

	if ((!ce_data) != (!ce_data_len)) {
		EVENT_UTILS_DBG("Error: Mismatching ce_data ptr, ce_data_len %d.\n",
				ce_data_len);
		return;
	}

	buf_len = sizeof(*ce) + ce_data_len;

	if (clock_gettime(CLOCK_REALTIME, &tp) < 0) {
		return;
	}
	time_in_ms = (uint64)tp.tv_sec*1000LL + (uint64)tp.tv_nsec/1000000LL;

	if (!(buf = (unsigned char *) calloc(buf_len, 1))) {
		EVENT_UTILS_DBG("Error: memory allocation for wl_cevent_t failed\n");
		return;
	}
	ce = (wl_cevent_t *) buf;

	/* Update wl_cevent_t */
	ce->version = WLC_E_CEVENT_VER;
	ce->length = sizeof(*ce) + ce_data_len;
	ce->type = ce_type;
	ce->data_offset = sizeof(*ce);
	ce->subtype = ce_subtype;
	ce->flags = ce_flags;
	ce->timestamp = time_in_ms;
	if (ce_data) {
		memcpy(CEVENT_DATA(ce), ce_data, ce_data_len);
	}

	send_event_to_port(EAPD_WKSP_CEVENT_UDP_SPORT, ifname, lan_ea, sta_ea,
			status, reason, auth_type, WLC_E_CEVENT, buf, buf_len);

	free(buf);
}

#endif /* BCM_CEVENT */
