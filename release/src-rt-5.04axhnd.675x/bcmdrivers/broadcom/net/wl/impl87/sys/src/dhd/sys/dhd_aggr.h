/*
 * Broadcom Dongle Host Driver (DHD) - Low Bit Rate Aggregation
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
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id$
 */

#ifndef _dhd_aggr_h_
#define _dhd_aggr_h_

#define DHD_AGGR_DEFAULT_TABLE_SIZE	256
#define DHD_AGGR_DEFAULT_POOL_SIZE	256

/*
 * dhd_aggr_type_t is used to define and to differentiate between different aggregators
 * that may be me active at multiple interception point on the same flow in a data path
 */

typedef enum {
	DHD_AGGR_TYPE_LBR, /* low bit rate aggregator */
} dhd_aggr_type_t;

typedef int (*dhd_aggr_cb_t)(dhd_pub_t *dhdp, int data, void * pkt);

/**
 * An aggregator is a data-path interceptor, used to intercept packets and to release
 * intercepted packest at some appropriate event e.g. aggregation reached, or a timer event
 */
typedef struct dhd_aggregator {
	dll_t list;                 /* manage active aggregator in a double linked list */
	dhd_aggr_type_t type;	    /* aggregate type */
	uint16 uid;		    /* user/unique key used to assigned this aggregate */
	void * pktqhead;            /* first packet in the queue */
	void * pktqtail;            /* last packet in the queue */
	int16  pktqlen;             /* number of packets in the queue */
	uint16 max_aggr;            /* maximum budget - for this pkt accumulator */
	uint32	release_time_msec;
	struct timer_list release_timer; /* timeout event to release */
	bool   release_timer_active;     /* flag to check timer status */
	dhd_aggr_cb_t  release_cb;       /* callback to release packet */
	unsigned long  cb_data;		 /* callback data for release function */
	bool	timeren;		 /* flag to inform if timer is requested */
	void * lock;			 /* to manage pktq sharing if timer is enabled */
	dhd_pub_t *dhdp;
} dhd_aggregator_t;

typedef struct dhd_aggrlist {
	dll_t list;                 /* manage aggregator in a double linked list */
} dhd_aggrlist_t;

typedef struct dhd_aggr_info {
	dhd_aggrlist_t *aggr_table;
	dhd_aggrlist_t free_pool;
	int   table_sz;
	void *lock;		    /* to protect access to aggr_table/free_pool */
} dhd_aggr_info_t;

int dhd_aggr_init(dhd_pub_t *dhdp);
void dhd_aggr_deinit(dhd_pub_t *dhdp);
dhd_aggregator_t *
dhd_aggr_add_aggregator(dhd_pub_t *dhdp, dhd_aggr_type_t type, uint16 key,
	dhd_aggr_cb_t release_cb, int release_timeout, int max_aggr, int cb_data);
void dhd_aggr_del_aggregator(dhd_pub_t *dhdp, dhd_aggr_type_t type, uint16 key);
bool dhd_aggr_intercept(dhd_pub_t *dhdp, dhd_aggregator_t *aggr, void *pktbuf);
dhd_aggregator_t *dhd_aggr_find_aggregator(dhd_pub_t *dhdp, dhd_aggr_type_t type, uint16 key);

#endif /* _dhd_aggr_h_ */
