/*
 * Cevent Shared utils
 *
 *
 * Copyright (C) 2022, Broadcom. All Rights Reserved.
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
 *
 * $Id $
 */

#ifndef __CE_SHARED_H__
#define __CE_SHARED_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <typedefs.h>
#include <security_ipc.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <time.h>
#include <ethernet.h>
#include <bcmevent.h>
#include <802.11.h>

/* send given 'data' of 'data_len' to the loopback interface over the mentioned UDP 'port' */
extern int ce_send_to_port(uint32 port, unsigned char *data, int data_len);

/* builds bcm_event_t and sends to the loopback interface over the mentioned UDP 'port' */
extern void ce_send_event_to_port(uint32 port, char *ifname,
		struct ether_addr *lan_ea, struct ether_addr *sta_ea,
		uint32 status, uint32 reason, uint32 auth_type, uint32 ev_type,
		void *ev_payload, uint32 ev_payload_len);

#ifdef BCM_CEVENT

/* This funcitons builds wl_cevent_t and sends it as event payload to ceventd using
 * ce_send_event_to_port()
 */
extern void ce_send_cevent(char *ifname, struct ether_addr *lan_ea, struct ether_addr *sta_ea,
		uint32 status, uint32 reason, uint32 auth_type, uint32 ce_type,
		uint32 ce_subtype, uint32 ce_msgtype, uint32 ce_flags, void *ce_data,
		size_t ce_data_len);

/* Macro for A2C that uses null/0 for lan_ea, auth_type in send_cevent() */
#define CE_SEND_CEVENT_A2C_EXT(ifname, sta_ea, status, reason, ce_subtype, ce_msgtype, \
		ce_flags, ce_data, ce_data_len) \
	ce_send_cevent(ifname, NULL, sta_ea, status, reason, 0, CEVENT_TYPE_A2C, \
			ce_subtype, ce_msgtype, ce_flags, ce_data, ce_data_len)

/* Macro that uses zero for status, reason in SEND_CEVENT_A2C_EXT() */
#define CE_SEND_CEVENT_A2C(ifname, ce_subtype, ce_msgtype, ce_data, ce_data_len) \
	CE_SEND_CEVENT_A2C_EXT(ifname, NULL, 0, 0, ce_subtype, ce_msgtype, 0, ce_data, ce_data_len)

#endif /* BCM_CEVENT */

#endif /* __CE_SHARED_H__ */
