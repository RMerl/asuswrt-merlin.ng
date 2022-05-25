/*
 * Wireless Multicast Forwarding (WMF)
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
 * $Id: dhd_wmf_linux.h 802430 2021-08-26 09:44:04Z $
 */
#ifndef _dhd_wmf_h_
#define _dhd_wmf_h_

#include <linux/list.h>

#include <emf/emf/emf_cfg.h>
#include <emf/emf/emfc_export.h>
#include <emf/igs/igs_cfg.h>
#include <emf/igs/igsc_export.h>

#include <dhd.h>

/* IGMP message types */
#define IGMPV2_HOST_MEMBERSHIP_QUERY            0x11
#define IGMPV2_HOST_MEMBERSHIP_REPORT           0x12
#define IGMPV2_HOST_NEW_MEMBERSHIP_REPORT       0x16
#define IGMPV2_LEAVE_GROUP_MESSAGE              0x17

#define MCAST_ADDR_UPNP_SSDP(a) ((a) == 0xeffffffa)
#define MAXSCB 32

/* Packet handling decision code */
#define WMF_DROP 0
#define WMF_NOP 1
#define WMF_TAKEN 2

/* WMF instance specific information */
typedef struct dhd_wmf_instance {
	void *emfci;	/* Pointer to emfc instance */
	void *igsci;	/* Pointer to igsc instance */
} dhd_wmf_instance_t;

/* Wrapper structure to pass to emf */
typedef struct dhd_wmf_wrapper {
	dhd_pub_t *dhdp;
	int bssidx;
} dhd_wmf_wrapper_t;

typedef struct dhd_wmf {
	bool wmf_enable;
	dhd_wmf_instance_t *wmf_instance;
	dhd_wmf_wrapper_t  *wmfh;
	bool wmf_bss_enab_val;
} dhd_wmf_t;

int dhd_wmf_start(dhd_pub_t *pub, uint32 bssidx);
void dhd_wmf_stop(dhd_pub_t *pub, uint32 bssidx);

int32 dhd_wmf_instance_add(dhd_pub_t *pub, uint32 bssidx);
void dhd_wmf_instance_del(dhd_pub_t *pub, uint32 bssidx);

int dhd_wmf_prep_stalist(void *pub);
int dhd_wmf_del_stalist(void *pub);

int dhd_wmf_set_mcast_sendup_only(dhd_pub_t *pub, uint32 bssidx, uint32 mode);
int dhd_wmf_mcast_data_sendup(dhd_pub_t *pub, uint32 bssidx, bool set, bool enable);

int32 dhd_wmf_hooks_register(void *wrapper);
int32 dhd_wmf_hooks_unregister(void *wrapper);

int32 dhd_wmf_igs_broadcast(void *wrapper, uint8 *ip, uint32 length, uint32 mgrp_ip);
int32 dhd_wmf_forward(void *wrapper, void *p, uint32 mgrp_ip, void *txif, int rt_port);
void dhd_wmf_sendup(void *wrapper, void *p);
int dhd_wmf_packets_handle(void *pub, void *p, void *ifp, int ifidx, bool frombss);
void dhd_wmf_cleanup(dhd_pub_t *pub, uint32 bssidx);

extern int dhd_wmf_bss_enable(dhd_pub_t *dhd_pub, uint32 bssidx);

#if (defined(BCM_NBUFF_WLMCAST) && defined(DHD_WMF))
int dhd_wmf_sta_del(dhd_wmf_t *wmf, void *scb);
#endif

#endif /* _dhd_wmf_h_ */
