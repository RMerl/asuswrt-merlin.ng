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

#include <typedefs.h>
#include <linuxver.h>
#include <osl.h>

#include <epivers.h>
#include <bcmutils.h>
#include <bcmendian.h>
#include <bcmdevs.h>

#include <ethernet.h>
#include <bcmevent.h>
#include <dngl_stats.h>
#include <dhd.h>
#include <dhd_linux.h>
#include <dhd_bus.h>
#include <dhd_proto.h>
#include <dhd_dbg.h>
#include <dhd_aggr.h>
#if defined(BCM_DHD_RUNNER)
#include <dhd_runner.h>
#endif /* BCM_DHD_RUNNER */

#ifdef PROP_TXSTATUS
#error "PROP_TXSTATUS and DHD_LBR_AGGR_BCM_ROUTER are not compatible"
#endif

#define LBR_AGGR_DEFAULT_LEN               32
#define LBR_AGGR_DEFAULT_RELEASE_TIMEOUT   10

#if defined(BCM_DHD_RUNNER) && defined(DHD_RNR_LBR_AGGR)
#define DHD_RNR_LBR_AGGR_CONFIG(dhdp)                      \
do {                                                       \
	dhd_runner_aggr_config_t aggr_cfg;                     \
	                                                       \
	aggr_cfg.en_mask = dhdp->lbr_aggr_en_mask;             \
	aggr_cfg.len = dhdp->lbr_aggr_len;                     \
	aggr_cfg.timeout = dhdp->lbr_aggr_release_timeout;     \
	                                                       \
	dhd_runner_notify(dhdp->runner_hlp,                    \
	    H2R_AGGR_CONFIG_NOTIF,                             \
	    (unsigned long)(&aggr_cfg), 0);                    \
} while (0)
#else /* !BCM_DHD_RUNNER || !DHD_RNR_LBR_AGGR */
#define DHD_RNR_LBR_AGGR_CONFIG(dhdp)     do {} while (0)
#endif /* !BCM_DHD_RUNNER || !DHD_RNR_LBR_AGGR */

/**
 * Resume transmit of previously intercept packets
 */

int BCMFASTPATH
dhd_sendpkt_resume_intercept(dhd_pub_t *dhdp, int data, void *pktbuf)
{
	int ret = BCME_OK;
	int ifidx = data;

	/* Reject if down */
	if (!dhdp->up || (dhdp->busstate == DHD_BUS_DOWN)) {
		/* free the packet here since the caller won't */
		PKTCFREE(dhdp->osh, pktbuf, TRUE);
		return -ENODEV;
	}
	if (dhdp->busstate == DHD_BUS_SUSPENDING || dhdp->busstate == DHD_BUS_SUSPENDED) {
		PKTFREE(dhdp->osh, pktbuf, TRUE);
		return -EBUSY;
	}

	/* If the protocol uses a data header, apply it */
	dhd_prot_hdrpush(dhdp, ifidx, pktbuf);
#ifdef BCMPCIE
	ret = dhd_bus_txdata(dhdp->bus, pktbuf, (uint8)ifidx);
#else
	ret = dhd_bus_txdata(dhdp->bus, pktbuf);
#endif /* BCMPCIE */
	return ret;

}

/**
 * check if packets to this flowring should be serviced later for better aggregation
 * FALSE - do not aggregate
 * TRUE  - aggregate
 */
bool
dhd_sendpkt_lbr_aggr_intercept(dhd_pub_t *dhdp, int data, void *pktbuf)
{
	dhd_aggregator_t *aggr;
	uint16 flowid;
	uint8  prio;

	if (!dhdp->lbr_aggr_en_mask) {
		return FALSE;
	}

	prio = dhdp->flow_prio_map[(PKTPRIO(pktbuf))];
	/* check for if aggregation is set for this priority */
	if  (!(1<<prio & dhdp->lbr_aggr_en_mask))
		return FALSE;

	flowid = DHD_PKT_GET_FLOWID(pktbuf);

#if defined(BCM_DHD_RUNNER) && defined(DHD_RNR_LBR_AGGR)
	/* Skip aggregation for runner offloaded rings (aggregation done in runner) */
	if (DHD_FLOWRING_RNR_OFFL(DHD_FLOW_RING(dhdp, flowid))) {
		return FALSE;
	}
#endif /* BCM_DHD_RUNNER && DHD_RNR_LBR_AGGR */

	if ((aggr = dhd_aggr_find_aggregator(dhdp, DHD_AGGR_TYPE_LBR, flowid)) == NULL) {
	    aggr = dhd_aggr_add_aggregator(dhdp,
	        DHD_AGGR_TYPE_LBR,
	        flowid,
	        dhd_sendpkt_resume_intercept,
	        dhdp->lbr_aggr_release_timeout,
	        dhdp->lbr_aggr_len,
	        data);
	}
	return dhd_aggr_intercept(dhdp, aggr, pktbuf);
}

/*
 * Configuration/support function
 */
int
dhd_lbr_aggr_init(dhd_pub_t *dhdp)
{
	dhdp->lbr_aggr_len = LBR_AGGR_DEFAULT_LEN;
	dhdp->lbr_aggr_release_timeout = LBR_AGGR_DEFAULT_RELEASE_TIMEOUT;
	return BCME_OK;
}

int
dhd_lbr_aggr_deinit(dhd_pub_t *dhdp)
{
	return BCME_OK;
}
uint32
dhd_lbr_aggr_en_mask(dhd_pub_t *dhdp, bool set, uint32 val)
{

	if (set) {
		dhdp->lbr_aggr_en_mask = val;
		DHD_RNR_LBR_AGGR_CONFIG(dhdp);
	}
	val = dhdp->lbr_aggr_en_mask;
	return val;
}

uint32
dhd_lbr_aggr_release_timeout(dhd_pub_t *dhdp, bool set, uint32 val)
{
	if (set) {
		dhdp->lbr_aggr_release_timeout = val;
		DHD_RNR_LBR_AGGR_CONFIG(dhdp);
	}
	val = dhdp->lbr_aggr_release_timeout;
	return val;
}

uint32
dhd_lbr_aggr_len(dhd_pub_t *dhdp, bool set, uint32 val)
{
	if (set) {
		dhdp->lbr_aggr_len = val;
		DHD_RNR_LBR_AGGR_CONFIG(dhdp);
	}
	val = dhdp->lbr_aggr_len;
	return val;
}
