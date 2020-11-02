/*
 * Broadcom 802.11abg Networking Device Driver
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
 * $Id: wl_rte.c 766305 2018-07-30 08:39:18Z $
 */

/**
 * @file
 * @brief
 * XXX Twiki: [HndRte]
 */

/* XXX: Define wlc_cfg.h to be the first header file included as some builds
 * get their feature flags thru this file.
 */
#include "wl_rte.h"
#include <osl.h>
#include <siutils.h>
#include <pcie_core.h>
#include <epivers.h>
#include <proto/ethernet.h>
#include <bcmdevs.h>
#include <wlioctl.h>

#include <proto/802.11.h>
#include <sbhndpio.h>
#include <sbhnddma.h>
#include <hnddma.h>
#include <d11.h>
#include <wlc_channel.h>
#include <wlc_pub.h>
#include <wlc.h>
#include <wlc_hw.h>
#include <wlc_hw_priv.h>
#include <wlc_bmac.h>
#ifdef HOST_HDR_FETCH
#include <wlc_tx.h>
#endif // endif
#include <bcmsrom_fmt.h>
#include <bcmsrom.h>
#ifdef BCM_OL_DEV
#include <bcm_ol_msg.h>
#endif /* BCM_OL_DEV */
#ifdef MSGTRACE
#include <msgtrace.h>
#endif // endif
#ifdef LOGTRACE
#include <logtrace.h>
#endif // endif

#include <wl_export.h>
#include <wlc_led.h>

#include <wl_toe.h>
#include <wl_arpoe.h>
#include <wl_keep_alive.h>
#include <wl_eventq.h>
#include <wl_gas.h>
#include <wl_p2po_disc.h>
#include <wl_p2po.h>
#include <wl_anqpo.h>
#include <wlc_pkt_filter.h>
#include <wlc_pcb.h>

#if defined(ATE_BUILD)
#include <../ate/include/wlu_ate.h>
#endif  /* ATE_BUILD */

#ifdef BCMDBG
#include <bcmutils.h>
#endif // endif

#include <dngl_bus.h>
#include <dngl_wlhdr.h>

#define WL_IFTYPE_BSS	1
#define WL_IFTYPE_WDS	2

#if defined(BCM_OL_DEV)
#include <wlc_pktfilterol.h>
#include <wlc_dngl_ol.h>
#else
#include <wlc_bsscfg.h>
#endif // endif

#if defined(BCMAUTH_PSK)
#include <proto/eapol.h>
#include <wlc_auth.h>
#endif // endif

#include <wlc_ampdu_rx.h>
#if defined(PROP_TXSTATUS)
#include <flring_fc.h>
#ifdef BCMPCIEDEV
#include <bcmmsgbuf.h>
#endif // endif
#endif /* PROP_TXSTATUS */
#include <wl_nfc.h>

#include <wlc_objregistry.h>

#ifdef WL_TBOW
#include <wlc_tbow.h>
#endif // endif

#include <rte_dev.h>
#include <rte_cons.h>
#include <rte_isr.h>
#include <rte.h>
#include <rte_mem.h>
#include <rte_ioctl.h>
#include <rte_timer.h>

#include <hnd_pktpool.h>
#include <hnd_pt.h>

#ifdef BCMPCIEDEV
#include <wlc_scb.h>
#include <wlc_key.h>
#include <wlc_tx.h>

#if defined(BCMWAPI_WPI) || defined(BCMWAPI_WAI)
#include <wlc_wapi.h>
#endif // endif
#include <rte_pktfetch.h>
#endif /* BCMPCIEDEV */

#ifdef ECOUNTERS
#include <ecounters.h>
#endif // endif

#ifdef WLWNM_AP
#include <wlc_wnm.h>
#endif // endif

#ifdef WL_MONITOR
#ifdef WL_STA_MONITOR_COMP
#include <wlc_stamon.h>
#endif /* WL_STA_MONITOR_COMP */
#ifdef WL_RADIOTAP
#include <wlc_ethereal.h>
#include <wl_radiotap.h>
#endif /* WL_RADIOTAP */
#endif /* WL_MONITOR */
#ifdef WL11K
#include <wlc_rrm.h>
#endif /* WL11K */

typedef struct wl_cmn_data {
	void *osh_cmn;
	void *objreg;
} wl_cmn_data_t;

#define WL_IF(wl, dev)	(((hnd_dev_t *)(dev) == ((wl_info_t *)(wl))->dev) ? \
			 NULL : \
			 *(wl_if_t **)((hnd_dev_t *)(dev) + 1))

#ifdef WLC_LOW_ONLY

/* Minimal memory requirement to do wlc_dpc. This is critical for BMAC as it cannot
 * lose frames
 * This value is based on perceived DPC need. Note that it accounts for possible
 * fragmentation  where sufficient memory does not mean getting contiguous allocation
 */

#define MIN_DPC_MEM	((RXBND + 6)* 2048)

#endif /* WLC_LOW_ONLY */

/* host wakeup filter flags */
#define HWFFLAG_UCAST	1		/* unicast */
#define HWFFLAG_BCAST	2		/* broadcast */

/* iovar table */
enum {
	IOV_HWFILTER,		/* host wakeup filter */
	IOV_DEEPSLEEP,		/* Deep sleep mode */
	IOV_DNGL_STATS,
#ifdef PROP_TXSTATUS
	IOV_WLFC_MODE,
	IOV_PROPTX_CRTHRESH,	/* PROPTX credit report level */
	IOV_PROPTX_DATATHRESH,	/* PROPTX pending data report level */
#endif /* PROP_TXSTATUS */
	IOV_RTETIMESYNC,
#ifdef ECOUNTERS
	IOV_ECOUNTERS,
#endif // endif
	IOV_LAST		/* In case of a need to check max ID number */
};

static const bcm_iovar_t wl_iovars[] = {
	{"hwfilter", IOV_HWFILTER, 0, IOVT_BUFFER, 0},
	{"deepsleep", IOV_DEEPSLEEP, 0, IOVT_BOOL, 0},
	{"dngl_stats", IOV_DNGL_STATS, 0, IOVT_BUFFER, 0},
#ifdef PROP_TXSTATUS
	{"wlfc_mode", IOV_WLFC_MODE, 0, IOVT_UINT8, 0},
	{"proptx_credit_thresh", IOV_PROPTX_CRTHRESH, 0, IOVT_UINT32, 0},
	{"proptx_data_thresh", IOV_PROPTX_DATATHRESH, 0, IOVT_UINT32, 0},
#endif /* PROP_TXSTATUS */
	{"rte_timesync", IOV_RTETIMESYNC, 0, IOVT_UINT32, 0},
#ifdef ECOUNTERS
	{"ecounters", IOV_ECOUNTERS, 0, IOVT_BUFFER, sizeof(ecounters_config_request_t)},
#endif // endif
	{NULL, 0, 0, 0, 0}
};

#ifdef PROP_TXSTATUS
static void wlfc_sendup_timer(void* arg);
static int wlfc_initialize(wl_info_t *wl, wlc_info_t *wlc);
#endif /* PROP_TXSTATUS */
#ifdef PROP_TXSTATUS_DEBUG
static void wlfc_hostpkt_callback(wlc_info_t *wlc, void *pkt, uint txs);
#endif // endif

#ifdef BCMDBG
static void do_wlmsg_cmd(void *arg, int argc, char *argv[]);
#endif // endif
#ifdef WLC_HIGH
static void wl_statsupd(wl_info_t *wl);
#endif // endif
static void wl_timer_main(hnd_timer_t *t);
#ifdef WLC_HIGH
static int wl_doiovar(void *hdl, const bcm_iovar_t *vi, uint32 actionid, const char *name,
                      void *p, uint plen, void *a, int alen, int vsize, struct wlc_if *wlcif);
#endif // endif

/* Driver entry points */
void *wl_probe(hnd_dev_t *dev, void *regs, uint bus,
	uint16 devid, uint coreid, uint unit);
static void wl_free(wl_info_t *wl, osl_t *osh);
static void wl_isr(void *cbdata);
#ifdef THREAD_SUPPORT
static void wl_dpc_thread(void *cbdata);
#endif	/* THREAD_SUPPORT */
static void wl_run(hnd_dev_t *dev);
static void _wl_dpc(hnd_timer_t *timer);
static void wl_dpc(wl_info_t *wl);
static int wl_open(hnd_dev_t *dev);
static int wl_send(hnd_dev_t *src, hnd_dev_t *dev, struct lbuf *lb);
static int wl_close(hnd_dev_t *dev);
#if defined(WLC_HIGH) && defined(BCMPCIEDEV)
static void wl_tx_pktfetch(wl_info_t *wl, struct lbuf *lb,
	hnd_dev_t *src, hnd_dev_t *dev);
static void wl_send_cb(void *lbuf, void *orig_lfrag, void *ctx, bool cancelled);
#endif /* WLC_HIGH && BCMPCIEDEV */
#ifdef BCMPCIEDEV
static uint32 wl_flowring_update(hnd_dev_t *dev, uint16 flowid, uint8 op, uint8 * sa,
	uint8 *da, uint8 tid);
#endif /* BCMPCIEDEV */
#if defined(PROP_TXSTATUS) && defined(BCMPCIEDEV)
static int wlfc_push_signal_bus_data(struct wl_info *wl, void* data, uint8 len);
#endif // endif

#ifndef WLC_LOW_ONLY
static int wl_ioctl(hnd_dev_t *dev, uint32 cmd, void *buf, int len, int *used, int *needed,
	int set);
#else
#ifdef BCM_OL_DEV
static int wl_ioctl(hnd_dev_t *dev, uint32 cmd, void *buf, int len, int *used, int *needed,
	int set);
#endif // endif

static void wl_rpc_down(void *wlh);
static void wl_rpc_resync(void *wlh);

static void wl_rpc_tp_txflowctl(hnd_dev_t *dev, bool state, int prio);
static void wl_rpc_txflowctl(void *wlh, bool on);

#if defined(HND_PT_GIANT) && defined(DMA_TX_FREE)
static void wl_lowmem_free(void *wlh);
#endif // endif
static void wl_rpc_bmac_dispatch(void *ctx, struct rpc_buf* buf);

static void do_wlhist_cmd(void *arg, int argc, char *argv[]);
static void do_wldpcdump_cmd(void *arg, int argc, char *argv[]);
#endif /* WLC_LOW_ONLY */

#ifdef WLC_HIGH
#ifdef TOE
static void _wl_toe_send_proc(wl_info_t *wl, void *p);
static int _wl_toe_recv_proc(wl_info_t *wl, void *p);
#endif /* TOE */
#endif /* WLC_HIGH */
#ifdef HOST_HDR_FETCH
static void wl_mac_dma_submit(hnd_dev_t *dev, void *p,  uint queue, bool commit, bool firstentry);
static void wl_mac_dma_commit(hnd_dev_t *dev, uint queue);
#endif /* HOST_HDR_FETCH */
#ifdef ECOUNTERS
static int wl_ecounters_entry_point(ecounters_get_stats fn, uint16 type,
	void *context);
#endif // endif
static hnd_dev_ops_t wl_funcs = {
#if defined(BCMROMSYMGEN_BUILD) || !defined(BCMROMBUILD)
	probe:		wl_probe,
#endif // endif
	open:		wl_open,
	close:		wl_close,
	xmit:		wl_send,
#ifdef WLC_LOW_ONLY
	txflowcontrol:	wl_rpc_tp_txflowctl,
#ifdef BCM_OL_DEV
	ioctl:		wl_ioctl,
#endif /* PCIDEV */
#else
	ioctl:		wl_ioctl,
#endif /* WLC_LOW_ONLY */
	poll:           wl_run,
#ifdef BCMPCIEDEV
	flowring_link_update: wl_flowring_update,
#endif /* BCMPCIEDEV */
#ifdef HOST_HDR_FETCH
	macdma_submit: wl_mac_dma_submit,
	macdma_commit: wl_mac_dma_commit
#endif // endif
};

#ifndef NUMD11CORES
#define  NUMD11CORES 1
#endif // endif

hnd_dev_t bcmwl[ /* NUMD11CORES */ ] = {
#if (NUMD11CORES == 2)
	{
		name:"wl",
		ops:&wl_funcs,
		softc:NULL,
		next:NULL,
		chained:NULL,
		stats:NULL,
		commondata:NULL,
		pdev:NULL
	},
#endif // endif
	{
		name:"wl",
		ops:&wl_funcs,
		softc:NULL,
		next:NULL,
		chained:NULL,
		stats:NULL,
		commondata:NULL,
		pdev:NULL
	}
};

#ifdef WLC_LOW_ONLY
#endif /* WLC_LOW_ONLY */

#ifdef WLC_HIGH

/* This includes the auto generated ROM IOCTL/IOVAR patch handler C source file (if auto patching is
 * enabled). It must be included after the prototypes and declarations above (since the generated
 * source file may reference private constants, types, variables, and functions).
 */
#include <wlc_patch.h>

static int
wl_doiovar(void *hdl, const bcm_iovar_t *vi, uint32 actionid, const char *name,
           void *p, uint plen, void *arg, int alen, int vsize, struct wlc_if *wlcif)
{
	wl_info_t *wl = (wl_info_t *)hdl;
	wlc_info_t *wlc = wl->wlc;
	int32 int_val = 0;
	int32 *ret_int_ptr;
	bool bool_val;
	int err = 0;
	int radio;

	/* convenience int and bool vals for first 8 bytes of buffer */
	if (plen >= (int)sizeof(int_val))
		bcopy(p, &int_val, sizeof(int_val));
	bool_val = (int_val != 0) ? TRUE : FALSE;

	/* convenience int ptr for 4-byte gets (requires int aligned arg) */
	ret_int_ptr = (int32 *)arg;

	switch (actionid) {
	case IOV_GVAL(IOV_HWFILTER):
		*ret_int_ptr = wl->hwfflags;
		break;

	case IOV_SVAL(IOV_HWFILTER):
		wl->hwfflags = (uint8)int_val;
		break;

	case IOV_GVAL(IOV_DEEPSLEEP):
		if ((err = wlc_get(wlc, WLC_GET_RADIO, &radio)))
			break;
		*ret_int_ptr = (radio & WL_RADIO_SW_DISABLE) ? TRUE : FALSE;
		break;

	case IOV_SVAL(IOV_DEEPSLEEP):
		wlc_set(wlc, WLC_SET_RADIO, (WL_RADIO_SW_DISABLE << 16)
		        | (bool_val ? WL_RADIO_SW_DISABLE : 0));
		/* suspend or resume timers */
		if (bool_val)
			hnd_suspend_timer();
		else
			hnd_resume_timer();
		break;

	case IOV_GVAL(IOV_DNGL_STATS):
		wl_statsupd(wl);
		bcopy(&wl->stats, arg, MIN(plen, sizeof(wl->stats)));
		break;

#ifdef PROP_TXSTATUS
	case IOV_GVAL(IOV_WLFC_MODE):
		if (PROP_TXSTATUS_ENAB(wlc->pub)) {
			uint32 caps = 0;
			WLFC_SET_AFQ(caps, 1);
			WLFC_SET_REUSESEQ(caps, 1);
			WLFC_SET_REORDERSUPP(caps, 1);
			*ret_int_ptr = caps;
		} else
			err = BCME_UNSUPPORTED;
		break;

	case IOV_SVAL(IOV_WLFC_MODE):
		if (PROP_TXSTATUS_ENAB(wlc->pub)) {
			if (WLFC_IS_OLD_DEF(int_val)) {
				wl->wlfc_mode = 0;
				if (int_val == WLFC_MODE_AFQ) {
					WLFC_SET_AFQ(wl->wlfc_mode, 1);
				} else if (int_val == WLFC_MODE_HANGER) {
					WLFC_SET_AFQ(wl->wlfc_mode, 0);
				} else {
					WL_ERROR(("%s: invalid wlfc mode value = 0x%x\n",
						__FUNCTION__, int_val));
				}
			} else {
				wl->wlfc_mode = int_val;
			}
		} else
			err = BCME_UNSUPPORTED;
		break;

	case IOV_SVAL(IOV_PROPTX_CRTHRESH):
		if (PROP_TXSTATUS_ENAB(wlc->pub)) {
			wlc_tunables_t *tunables = (((wlc_info_t *)(wl->wlc))->pub)->tunables;

			if (wl->wlfc_info == NULL)
				return BCME_NOTREADY;

			wl->wlfc_info->fifo_credit_threshold[TX_AC_BK_FIFO] = MIN(
				MAX((int_val >> 24) & 0xff, 1), tunables->wlfcfifocreditac0);
			wl->wlfc_info->fifo_credit_threshold[TX_AC_BE_FIFO] = MIN(
				MAX((int_val >> 16) & 0xff, 1), tunables->wlfcfifocreditac1);
			wl->wlfc_info->fifo_credit_threshold[TX_AC_VI_FIFO] = MIN(
				MAX((int_val >> 8) & 0xff, 1), tunables->wlfcfifocreditac2);
			wl->wlfc_info->fifo_credit_threshold[TX_AC_VO_FIFO] = MIN(
				MAX((int_val & 0xff), 1), tunables->wlfcfifocreditac3);
		} else
			err = BCME_UNSUPPORTED;
		break;

	case IOV_GVAL(IOV_PROPTX_CRTHRESH):
		if (PROP_TXSTATUS_ENAB(wlc->pub)) {
			if (wl->wlfc_info == NULL)
				return BCME_NOTREADY;

			*ret_int_ptr =
				(wl->wlfc_info->fifo_credit_threshold[TX_AC_BK_FIFO] << 24) |
				(wl->wlfc_info->fifo_credit_threshold[TX_AC_BE_FIFO] << 16) |
				(wl->wlfc_info->fifo_credit_threshold[TX_AC_VI_FIFO] << 8) |
				(wl->wlfc_info->fifo_credit_threshold[TX_AC_VO_FIFO]);
		} else
			err = BCME_UNSUPPORTED;
		break;

	case IOV_SVAL(IOV_PROPTX_DATATHRESH):
		if (PROP_TXSTATUS_ENAB(wlc->pub)) {
			if (wl->wlfc_info == NULL)
				return BCME_NOTREADY;

			wl->wlfc_info->pending_datathresh =
				MIN(MAX(int_val, 1), WLFC_MAX_PENDING_DATALEN);
		} else
			err = BCME_UNSUPPORTED;
		break;

	case IOV_GVAL(IOV_PROPTX_DATATHRESH):
		if (PROP_TXSTATUS_ENAB(wlc->pub)) {
			if (wl->wlfc_info == NULL)
				return BCME_NOTREADY;

			*ret_int_ptr = wl->wlfc_info->pending_datathresh;
		} else
			err = BCME_UNSUPPORTED;
		break;
#endif /* PROP_TXSTATUS */

#ifdef DONGLEBUILD
	case IOV_SVAL(IOV_RTETIMESYNC):
		hnd_set_reftime_ms(int_val);
		break;

	case IOV_GVAL(IOV_RTETIMESYNC):
		*ret_int_ptr = hnd_time();
		break;
#endif /* DONGLEBUILD */

#ifdef ECOUNTERS
	/* Ecounters set routine */
	case IOV_SVAL(IOV_ECOUNTERS):
		if (ECOUNTERS_ENAB())
		{
			/* params = parameters */
			/* p_len = parameter length in bytes? */
			/* arg = destination buffer where some info could be put. */
			/* len = length of destination buffer. */
			*ret_int_ptr = ecounters_config(p, plen);
		}
		else
		{
			err = BCME_UNSUPPORTED;
		}
		break;
#endif /* ECOUNTERS */
	default:
		err = BCME_UNSUPPORTED;
	}

	return err;
} /* wl_doiovar */

static void
BCMINITFN(_wl_init)(wl_info_t *wl)
{
	wl_reset(wl);

	wlc_init(wl->wlc);
}

void
wl_init(wl_info_t *wl)
{
	WL_TRACE(("wl%d: wl_init\n", wl->unit));

#if defined(DONGLEBUILD) && defined(BCMNODOWN)
	if (!bcmreclaimed)
#endif /* defined(DONGLEBUILD) && defined (BCMNODOWN) */
		_wl_init(wl);
}
#endif /* WLC_HIGH */

uint
BCMINITFN(wl_reset)(wl_info_t *wl)
{
	WL_TRACE(("wl%d: wl_reset\n", wl->unit));

	wlc_reset(wl->wlc);

	return 0;
}

bool
BCMINITFN(wl_alloc_dma_resources)(wl_info_t *wl, uint addrwidth)
{
	return TRUE;
}

/**
 * These are interrupt on/off enter points.
 * Since wl_run is serialized with other drive rentries using spinlock,
 * They are SMP safe, just call common routine directly,
 */
void
wl_intrson(wl_info_t *wl)
{
	wlc_intrson(wl->wlc);
}

uint32
wl_intrsoff(wl_info_t *wl)
{
	return wlc_intrsoff(wl->wlc);
}

void
wl_intrsrestore(wl_info_t *wl, uint32 macintmask)
{
	wlc_intrsrestore(wl->wlc, macintmask);
}

#ifdef PROP_TXSTATUS
static void wl_send_credit_map(wl_info_t *wl)
{
	if (PROP_TXSTATUS_ENAB(wl->pub) && HOST_PROPTXSTATUS_ACTIVATED((wlc_info_t*)(wl->wlc))) {
		int new_total_credit = 0;

		if (POOL_ENAB(wl->pub->pktpool)) {
			new_total_credit = pktpool_len(wl->pub->pktpool);
		}

		if ((wl->wlfc_info->total_credit > 0) && (new_total_credit > 0) &&
			(new_total_credit != wl->wlfc_info->total_credit)) {
			/* re-allocate new total credit among ACs */
			wl->wlfc_info->fifo_credit[TX_AC_BK_FIFO] =
				wl->wlfc_info->fifo_credit[TX_AC_BK_FIFO] * new_total_credit /
				wl->wlfc_info->total_credit;
			wl->wlfc_info->fifo_credit[TX_AC_VI_FIFO] =
				wl->wlfc_info->fifo_credit[TX_AC_VI_FIFO] * new_total_credit /
				wl->wlfc_info->total_credit;
			wl->wlfc_info->fifo_credit[TX_AC_VO_FIFO] =
				wl->wlfc_info->fifo_credit[TX_AC_VO_FIFO] * new_total_credit /
				wl->wlfc_info->total_credit;
			wl->wlfc_info->fifo_credit[TX_BCMC_FIFO] =
				wl->wlfc_info->fifo_credit[TX_BCMC_FIFO] * new_total_credit /
				wl->wlfc_info->total_credit;
			/* give all remainig credits to BE */
			wl->wlfc_info->fifo_credit[TX_AC_BE_FIFO] = new_total_credit -
				wl->wlfc_info->fifo_credit[TX_AC_BK_FIFO] -
				wl->wlfc_info->fifo_credit[TX_AC_VI_FIFO] -
				wl->wlfc_info->fifo_credit[TX_AC_VO_FIFO] -
				wl->wlfc_info->fifo_credit[TX_BCMC_FIFO];

			/* recaculate total credit from actual pool size */
			wl->wlfc_info->total_credit = new_total_credit;
		}

		if (wl->wlfc_info->totalcredittohost != wl->wlfc_info->total_credit) {
			wlc_mac_event(wl->wlc, WLC_E_FIFO_CREDIT_MAP, NULL, 0, 0, 0,
				wl->wlfc_info->fifo_credit, sizeof(wl->wlfc_info->fifo_credit));
			wlc_mac_event(wl->wlc, WLC_E_BCMC_CREDIT_SUPPORT, NULL, 0, 0, 0, NULL, 0);

			wl->wlfc_info->totalcredittohost = wl->wlfc_info->total_credit;
		}
	}
}

void wl_reset_credittohost(struct wl_info *wl)
{
	if (wl && wl->wlfc_info) {
		wl->wlfc_info->totalcredittohost = 0;
	}
}
#endif /* PROP_TXSTATUS */

#ifdef WLC_HIGH
/** BMAC driver has alternative up/down etc. */
int
wl_up(wl_info_t *wl)
{
	int ret;
	wlc_info_t *wlc = (wlc_info_t *) wl->wlc;

	WL_TRACE(("wl%d: wl_up\n", wl->unit));

	if (wl->pub->up)
		return 0;

#if defined(BCMNODOWN)
	if (bcmreclaimed) {
		wlc_minimal_up(wl->wlc);
		return BCME_OK; /* wlc_minimal_up() takes it from here */
	}
#endif /* defined (BCMNODOWN) */

	if (wl->pub->up)
		return 0;

	/* Reset the hw to known state */
	ret = wlc_up(wlc);

	if (ret == 0)
		ret = wl_keep_alive_up(wl->keep_alive_info);

#ifndef RSOCK
#ifdef DONGLEBUILD
	/* Don't attempt to recliam the ucode section here */
	ucodes_reclaimed = TRUE;
	hnd_reclaim();
#endif // endif

#ifdef BCMPKTPOOL
	if (POOL_ENAB(wl->pub->pktpool)) {
		hnd_pktpool_fill(wl->pub->pktpool, FALSE);
#ifdef PROP_TXSTATUS
		wl_send_credit_map(wl);
#endif /* PROP_TXSTATUS */
	}
#ifdef BCMFRAGPOOL
	if (POOL_ENAB(wl->pub->pktpool_lfrag))
		hnd_pktpool_fill(wl->pub->pktpool_lfrag, FALSE);
#endif /* BCMFRAGPOOL */
#ifdef BCMRXFRAGPOOL
	if (POOL_ENAB(wl->pub->pktpool_rxlfrag))
		hnd_pktpool_fill(wl->pub->pktpool_rxlfrag, FALSE);
#endif /* BCMRXFRAGPOOL */
#endif /* BCMPKTPOOL */

#endif /* RSOCK */
	return ret;
}

void
wl_down(wl_info_t *wl)
{
	WL_TRACE(("wl%d: wl_down\n", wl->unit));
	if (!wl->pub->up)
		return;

#ifdef BCMNODOWN
	wlc_minimal_down(wl->wlc);
#else
	wl_keep_alive_down(wl->keep_alive_info);
	wlc_down(wl->wlc);
	wl->pub->hw_up = FALSE;
#endif /* BCMNODOWN */
	wl_indicate_maccore_state(wl, LTR_SLEEP);
}

void
wl_dump_ver(wl_info_t *wl, struct bcmstrbuf *b)
{
	bcm_bprintf(b, "wl%d: %s %s version %s FWID 01-%x\n", wl->unit,
		__DATE__, __TIME__, EPI_VERSION_STR, gFWID);
}

#if defined(BCMDBG) || defined(WLDUMP)
static int
wl_dump(wl_info_t *wl, struct bcmstrbuf *b)
{
	wl_dump_ver(wl, b);

	return 0;
}
#endif /* BCMDBG || WLDUMP */
#endif /* WLC_HIGH */

void
wl_monitor(wl_info_t *wl, wl_rxsts_t *rxsts, void *p)
{
#ifdef WL_MONITOR
	wlc_info_t *wlc = wl->wlc;
	struct lbuf *mon_pkt;
	mon_pkt = (struct lbuf *)p;
#ifdef BCMPCIEDEV
	wlc_d11rxhdr_t *wrxh;
	wlc_pkttag_t * pkttag = WLPKTTAG(mon_pkt);
	PKTPUSH(wlc->osh, mon_pkt, wlc->hwrxoff);
	wrxh = (wlc_d11rxhdr_t *)PKTDATA(wlc->osh, mon_pkt);
	pkttag->pktinfo.misc.snr = wlc_phy_noise_avg((wlc_phy_t *)
			WLC_PI_BANDUNIT(wlc, CHSPEC_WLCBANDUNIT(wrxh->rxhdr.RxChan)));
	pkttag->pktinfo.misc.rssi = wrxh->rssi;
	if (RXFIFO_SPLIT() && !PKT_CLASSIFY_EN(wrxh->rxhdr.fifo)) {
		PKTPULL(wlc->osh, mon_pkt, wlc->hwrxoff);
	}

	PKTSET80211(mon_pkt);
	PKTSETMON(mon_pkt);
#else
	wlc_pkttag_t * pkttag = WLPKTTAG(p);
	wlc_pkttag_t * mon_pkttag;
	mon_pkttag = WLPKTTAG(mon_pkt);

	uint len = PKTLEN(wlc->osh, p) - D11_PHY_RXPLCP_LEN(wlc->pub->corerev);

	if ((mon_pkt = PKTGET(wlc->osh, len, FALSE)) == NULL)
		return;

	memcpy(PKTDATA(wlc->osh, mon_pkt),
			PKTDATA(wlc->osh, p) + D11_PHY_RXPLCP_LEN(wlc->pub->corerev), len);

	mon_pkttag->rxchannel = pkttag->rxchannel;
	mon_pkttag->pktinfo.misc.rssi = pkttag->pktinfo.misc.rssi;
	mon_pkttag->rspec = pkttag->rspec;

#endif /* BCMPCIEDEV */
	wl_sendup(wl, NULL, mon_pkt, 1);

#endif /* WL_MONITOR */

}

void
wl_set_monitor(wl_info_t *wl, int val)
{
#if defined(WL_MONITOR) && !defined(WL_MONITOR_DISABLED)
	wlc_info_t *wlc = wl->wlc;
	wlc_tunables_t *tune = wlc->pub->tunables;
	wl_set_monitor_mode(wl, val);
	/* wlc->hwrxoff is used by host to strip the rxhdr after forming the radiotap header */
	if (val) {
		wl_set_copycount_bytes(wl, tune->copycount, wlc->hwrxoff);
	}
	wl->monitor_type = (uint32)val;
#endif /* WL_MONITOR && WL_MONITOR_DISABLED */
}

char *
wl_ifname(wl_info_t *wl, struct wl_if *wlif)
{
	if (wlif == NULL)
		return wl->dev->name;
	else
		return wlif->dev->name;
}

#if defined(WLC_HIGH)
static hnd_dev_ops_t*
get_wl_funcs(void)
{
	return &wl_funcs;
}

/** Allocate wl_if_t, hnd_dev_t, and wl_if_t * all together */
static wl_if_t *
wl_alloc_if(wl_info_t *wl, int iftype, uint subunit, struct wlc_if *wlcif, bool rebind)
{
	hnd_dev_t *dev;
	wl_if_t *wlif;
	osl_t *osh = wl->pub->osh;
	hnd_dev_t *bus = wl->dev->chained;
	uint len;
	int ifindex;
	wl_if_t **priv;

	/* the primary device must be binded to the bus */
	if (bus == NULL) {
		WL_ERROR(("wl%d: %s: device not binded\n", wl->pub->unit, __FUNCTION__));
		return NULL;
	}

	/* allocate wlif struct + dev struct + priv pointer */
	len = sizeof(wl_if_t) + sizeof(hnd_dev_t) + sizeof(wl_if_t **);
	if ((wlif = MALLOC(osh, len)) == NULL) {
		WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
		          (wl->pub)?wl->pub->unit:subunit, __FUNCTION__, MALLOCED(wl->pub->osh)));
		goto err;
	}
	bzero(wlif, len);

	dev = (hnd_dev_t *)(wlif + 1);
	priv = (wl_if_t **)(dev + 1);

	wlif->dev = dev;
	wlif->wlcif = wlcif;

	dev->ops = get_wl_funcs();
	dev->softc = wl;
	if (iftype == WL_IFTYPE_WDS) {
		wlc_bsscfg_t *cfg = wlc_bsscfg_find_by_wlcif(wl->wlc, wlcif);
		ASSERT(cfg != NULL);
		snprintf(dev->name, HND_DEV_NAME_MAX, "wds%d.%d.%d", wl->pub->unit,
				WLC_BSSCFG_IDX(cfg), subunit);
	} else {
		snprintf(dev->name, HND_DEV_NAME_MAX, "wl%d.%d", wl->pub->unit, subunit);
	}

	*priv = wlif;

	/* use the return value as the i/f no. in the event to the host */
	if (!rebind) {
#ifdef DONGLEBUILD
		if ((ifindex = bus_ops->binddev(bus, dev, -1)) < 1) {
			WL_ERROR(("wl%d: %s: bus_binddev failed\n", wl->pub->unit, __FUNCTION__));
			goto err;
		}
#else
		ifindex = subunit;
#endif /* DONGLEBUILD */
		wlcif->index = (uint8)ifindex;
	}

	/* create and populate arpi for this IF */
	if (ARPOE_SUPPORT(wl->pub)) {
		wlif->arpi = wl_arp_alloc_ifarpi(wl->arpi, wlcif);
	}

#ifdef WLNDOE
	if (NDOE_ENAB(wl->pub))
		wlif->ndi = wl_nd_alloc_ifndi(wl->ndi, wlcif);
#endif // endif

#ifdef WLNFC
	if (NFC_ENAB(wl->pub)) {
		wlif->nfci = wl_nfc_alloc_ifnfci(wl->nfci, wlcif);
	}
#endif // endif

	return wlif;

err:
	if (wlif != NULL)
		MFREE(osh, wlif, len);
	return NULL;
} /* wl_alloc_if */

static void
wl_free_if(wl_info_t *wl, wl_if_t *wlif)
{
	/* free arpi for this IF */
	if (ARPOE_SUPPORT(wl->pub)) {
		wl_arp_free_ifarpi(wlif->arpi);
	}

#ifdef WLNDOE
	/* free ndi for this IF */
	if (NDOE_ENAB(wl->pub)) {
		wl_nd_free_ifndi(wlif->ndi);
	}
#endif // endif

#ifdef WLNFC
	/* free nfci for this IF */
	if (NFC_ENAB(wl->pub)) {
		wl_nfc_free_ifnfci(wlif->nfci);
	}
#endif // endif

	MFREE(wl->pub->osh, wlif, sizeof(wl_if_t) + sizeof(hnd_dev_t) + sizeof(wl_if_t *));
}

struct wl_if *
wl_add_if(wl_info_t *wl, struct wlc_if *wlcif, uint unit, struct ether_addr *remote)
{
	wl_if_t *wlif;

	wlif = wl_alloc_if(wl, remote != NULL ? WL_IFTYPE_WDS : WL_IFTYPE_BSS, unit, wlcif, FALSE);

	if (wlif == NULL) {
		WL_ERROR(("wl%d: %s: failed to create %s interface %d\n", wl->pub->unit,
			__FUNCTION__, (remote)?"WDS":"BSS", unit));
		return NULL;
	}

	return wlif;
}

#ifdef WLRSDB
/** Update the wl pointer for RSDB bsscfg Move */
void
wl_update_if(struct wl_info *from_wl, struct wl_info *to_wl, wl_if_t *from_wlif,
	struct wlc_if *to_wlcif)
{
	wl_if_t *new_wlif;
	struct wlc_if * from_wlcif;
	uint8 cur_ifindex = 0;
	hnd_dev_t *bus = from_wl->dev->chained;

	if (!from_wlif) { /* primary bsscfg */
		bus_ops->rebinddev(bus, to_wl->dev, 0); /* real dev  is passed */
	} else {
		cur_ifindex = from_wlif->wlcif->index;
		ASSERT(from_wlif->wlcif->type == WL_IFTYPE_BSS);
		from_wlcif = from_wlif->wlcif;
		new_wlif = wl_alloc_if(to_wl, from_wlif->wlcif->type,
			WLC_BSSCFG_IDX(from_wlcif->u.bsscfg), to_wlcif, TRUE);
		ASSERT(new_wlif);
		/*  Modify the device name for new interface.
		 * OS is not aware of bsscfg move during join
		 * and still sees the old interface name (ex wl0.2)
		 */
		strcpy(wl_ifname(to_wl, new_wlif), wl_ifname(from_wl, from_wlif));
		bus_ops->rebinddev(bus, new_wlif->dev, cur_ifindex); /* virtual dev is passed */

#ifdef ARPOE
		if (ARPOE_ENAB(to_wl->pub))
			wl_arp_clone_arpi(from_wlif->arpi, new_wlif->arpi);
#endif // endif
#ifdef WLNDOE
		if (NDOE_ENAB(to_wl->pub))
			wl_nd_clone_ifndi(from_wlif->ndi, new_wlif->ndi);
#endif // endif
		to_wlcif->wlif = new_wlif;
		wl_free_if(from_wl, from_wlif);
		from_wlcif->wlif = NULL;
	}
	to_wlcif->index = cur_ifindex;
}
#endif /* WLRSDB */

void
wl_del_if(wl_info_t *wl, struct wl_if *wlif)
{
#ifdef DONGLEBUILD
	hnd_dev_t *bus = wl->dev->chained;

	if (bus_ops->unbinddev(bus, wlif->dev) < 1)
		WL_ERROR(("wl%d: %s: bus_unbinddev failed\n", wl->pub->unit, __FUNCTION__));
#endif // endif
	WL_TRACE(("wl%d: %s: bus_unbinddev idx %d\n", wl->pub->unit, __FUNCTION__,
		wlif->wlcif->index));
	wl_free_if(wl, wlif);
}
#endif /* WLC_HIGH */

bool
wl_max_if_reached(wl_info_t *wl)
{
	return bus_ops->maxdevs_reached((void *)wl->dev->chained);
}

static void
wl_timer_main(hnd_timer_t *t)
{
	hnd_timer_auxfn_t auxfn = hnd_timer_get_auxfn(t);
	void *data = hnd_timer_get_data(t);

	ASSERT(auxfn != NULL);
	auxfn(data);
}

#undef wl_init_timer	/* see space saving hack in wl_export.h */

struct wl_timer *
wl_init_timer(wl_info_t *wl, void (*fn)(void* arg), void *arg, const char *name)
{
	return (struct wl_timer *)hnd_timer_create(wl, arg, wl_timer_main, fn);
}

void
wl_free_timer(wl_info_t *wl, struct wl_timer *t)
{
	hnd_timer_free((hnd_timer_t *)t);
}

void
wl_add_timer(wl_info_t *wl, struct wl_timer *t, uint ms, int periodic)
{
	ASSERT(t != NULL);
	hnd_timer_start((hnd_timer_t *)t, ms, periodic);
}

bool
wl_del_timer(wl_info_t *wl, struct wl_timer *t)
{
	if (t == NULL)
		return TRUE;
	return hnd_timer_stop((hnd_timer_t *)t);
}

#ifdef PROP_TXSTATUS
#ifdef PROP_TXSTATUS_DEBUG
static void
hndrte_wlfc_info_dump(void *arg, int argc, char *argv[])
{
	extern void wlfc_display_debug_info(void* _wlc, int hi, int lo);
	wlfc_info_state_t* wlfc = wlfc_state_get(((wlc_info_t*)arg)->wl);
	int hi = 0;
	int lo = 0;
	int i;

	if (argc > 2) {
		hi = atoi(argv[1]);
		lo = atoi(argv[2]);
	}
	printf("packets: (from_host-nost-sig,status_back,stats_other,credit_back,creditin) = "
		"(%d-%d-%d,%d,%d,%d,%d)\n",
		wlfc->stats.packets_from_host,
		wlfc->dbgstats->nost_from_host,
		wlfc->dbgstats->sig_from_host,
		wlfc->stats.txstatus_count,
		wlfc->stats.txstats_other,
		wlfc->dbgstats->creditupdates,
		wlfc->dbgstats->creditin);
	printf("credits for fifo: fifo[0-5] = (");
	for (i = 0; i < NFIFO; i++)
		printf("%d,", wlfc->dbgstats->credits[i]);
	printf(")\n");
	printf("stats: (header_only_alloc, realloc_in_sendup): (%d,%d)\n",
		wlfc->dbgstats->nullpktallocated,
		wlfc->dbgstats->realloc_in_sendup);
	printf("wlc_toss, wlc_sup = (%d, %d)\n",
		wlfc->dbgstats->wlfc_wlfc_toss,
		wlfc->dbgstats->wlfc_wlfc_sup);
	printf("debug counts:for_D11,to_D11,free_exceptions:(%d,%d,%d)\n",
		(wlfc->stats.packets_from_host - (wlfc->dbgstats->wlfc_wlfc_toss +
		wlfc->dbgstats->wlfc_wlfc_sup)
		+ wlfc->stats.txstats_other),
		wlfc->dbgstats->wlfc_to_D11,
		wlfc->dbgstats->wlfc_pktfree_except);
#ifdef AP
	wlfc_display_debug_info((void*)arg, hi, lo);
#endif // endif
	return;
}
#endif /* PROP_TXSTATUS_DEBUG */

static int
BCMATTACHFN(wlfc_initialize)(wl_info_t *wl, wlc_info_t *wlc)
{
	wlc_tunables_t *tunables = wlc->pub->tunables;

#ifdef PROP_TXSTATUS_DEBUG
	if (wlc_pcb_fn_set(wlc->pcb, 2, WLF2_PCB3_WLFC, wlfc_hostpkt_callback) != BCME_OK) {
		WL_ERROR(("wlc_pcb_fn_set(wlfc) failed in %s()", __FUNCTION__));
		goto fail;
	}
#endif // endif
	wl->wlfc_info = (wlfc_info_state_t*)MALLOCZ(wl->pub->osh, sizeof(wlfc_info_state_t));
	if (wl->wlfc_info == NULL) {
		WL_ERROR(("MALLOC() failed in %s() for wlfc_info", __FUNCTION__));
		goto fail;
	}

#ifdef PROP_TXSTATUS_DEBUG
	wl->wlfc_info->dbgstats = (wlfc_fw_dbgstats_t*)MALLOC(wl->pub->osh,
	    sizeof(wlfc_fw_dbgstats_t));
	if (wl->wlfc_info->dbgstats == NULL) {
		WL_ERROR(("MALLOC() failed in %s() for dbgstats", __FUNCTION__));
		goto fail;
	}
	memset(wl->wlfc_info->dbgstats, 0, sizeof(wlfc_fw_dbgstats_t));
#endif /* PROP_TXSTATUS_DEBUG */

	wl->wlfc_info->pending_datathresh = WLFC_PENDING_TRIGGER_WATERMARK;
	wl->wlfc_info->fifo_credit_threshold[TX_AC_BE_FIFO] =
		tunables->wlfc_fifo_cr_pending_thresh_ac_be;
	wl->wlfc_info->fifo_credit_threshold[TX_AC_BK_FIFO] =
		tunables->wlfc_fifo_cr_pending_thresh_ac_bk;
	wl->wlfc_info->fifo_credit_threshold[TX_AC_VI_FIFO] =
		tunables->wlfc_fifo_cr_pending_thresh_ac_vi;
	wl->wlfc_info->fifo_credit_threshold[TX_AC_VO_FIFO] =
		tunables->wlfc_fifo_cr_pending_thresh_ac_vo;
	wl->wlfc_info->fifo_credit_threshold[TX_BCMC_FIFO] =
		tunables->wlfc_fifo_cr_pending_thresh_bcmc;
	wl->wlfc_info->fifo_credit_threshold[TX_ATIM_FIFO] = 1;	/* send credit back ASAP */
	wl->wlfc_info->fifo_credit[TX_AC_BE_FIFO] =
		tunables->wlfcfifocreditac1;
	wl->wlfc_info->fifo_credit[TX_AC_BK_FIFO] =
		tunables->wlfcfifocreditac0;
	wl->wlfc_info->fifo_credit[TX_AC_VI_FIFO] =
		tunables->wlfcfifocreditac2;
	wl->wlfc_info->fifo_credit[TX_AC_VO_FIFO] =
		tunables->wlfcfifocreditac3;
	wl->wlfc_info->fifo_credit[TX_BCMC_FIFO] =
		tunables->wlfcfifocreditbcmc;
	wl->wlfc_info->totalcredittohost = 0;
	wl->wlfc_info->total_credit = wl->wlfc_info->fifo_credit[TX_AC_BE_FIFO] +
		wl->wlfc_info->fifo_credit[TX_AC_BK_FIFO] +
		wl->wlfc_info->fifo_credit[TX_AC_VI_FIFO] +
		wl->wlfc_info->fifo_credit[TX_AC_VO_FIFO] +
		wl->wlfc_info->fifo_credit[TX_BCMC_FIFO];
	wl->wlfc_info->wlfc_trigger = tunables->wlfc_trigger;
	wl->wlfc_info->wlfc_fifo_bo_cr_ratio = tunables->wlfc_fifo_bo_cr_ratio;
	wl->wlfc_info->wlfc_comp_txstatus_thresh = tunables->wlfc_comp_txstatus_thresh;

	wlc->wlfc_data = MALLOCZ(wl->pub->osh, sizeof(wlfc_mac_desc_handle_map_t));
	if (wlc->wlfc_data == NULL) {
		WL_ERROR(("MALLOC() failed in %s() for wlfc_mac_desc_handle_map_t", __FUNCTION__));
		goto fail;
	}

	wlc->wlfc_vqdepth = WLFC_DEFAULT_FWQ_DEPTH;

	/* init and add a timer for periodic wlfc signal sendup */
	wl->wlfc_info->wl_info = wl;
	if (!(((wlfc_info_state_t*)wl->wlfc_info)->fctimer = wl_init_timer(wl,
		wlfc_sendup_timer, wl->wlfc_info, "wlfctimer"))) {
		WL_ERROR(("wl%d: wl_init_timer for wlfc timer failed\n", wl->pub->unit));
		goto fail;
	}
	if (!BCMPCIEDEV_ENAB()) {
		wlc_eventq_set_ind(wlc->eventq, WLC_E_FIFO_CREDIT_MAP, TRUE);
		wlc_eventq_set_ind(wlc->eventq, WLC_E_BCMC_CREDIT_SUPPORT, TRUE);
	}
#ifdef PROP_TXSTATUS_DEBUG
	wlc->wlfc_flags = WLFC_FLAGS_RSSI_SIGNALS | WLFC_FLAGS_XONXOFF_SIGNALS |
		WLFC_FLAGS_CREDIT_STATUS_SIGNALS;
	hnd_cons_add_cmd("np", hndrte_wlfc_info_dump, wlc);
#else
	/* All TLV s are turned off by default */
	wlc->wlfc_flags = 0;
#endif // endif

	/* Enable compressed txstatus by default */
	wlc->comp_stat_enab = TRUE;

	return BCME_OK;

fail:
	return BCME_ERROR;
} /* wlfc_initialize */

static void
wlfc_sendup_timer(void* arg)
{
	wlfc_info_state_t* wlfc = (wlfc_info_state_t*)arg;

	wlfc->timer_started = 0;
	wlfc_sendup_ctl_info_now(wlfc->wl_info);
}

#ifdef PROP_TXSTATUS_DEBUG
static void
wlfc_hostpkt_callback(wlc_info_t *wlc, void *p, uint txstatus)
{
	wlc_pkttag_t *pkttag;

	ASSERT(p != NULL);
	pkttag = WLPKTTAG(p);
	if (WL_TXSTATUS_GET_FLAGS(pkttag->wl_hdr_information) & WLFC_PKTFLAG_PKTFROMHOST) {
		wlc->wl->wlfc_info->dbgstats->wlfc_pktfree_except++;
	}
}
#endif /* PROP_TXSTATUS_DEBUG */

uint8
wlfc_allocate_MAC_descriptor_handle(wlc_info_t* wlc)
{
	int i;

	uint8* bitmap;
	struct wlfc_mac_desc_handle_map* map;

	map = (struct wlfc_mac_desc_handle_map*) wlc->wlfc_data;
	if (!map->bitmap) {
		map->bitmap = (uint32 *)MALLOCZ(wlc->osh, (WLFC_BITMAP_NDWORD * sizeof(uint32)));
		if (!map->bitmap) {
			return WLFC_MAC_DESC_ID_INVALID;
		}
	}
	bitmap = (uint8*)map->bitmap;
	for (i = 0; i < (WLFC_BITMAP_NDWORD * NBITS(uint32)); i++) {
		if (isclr(bitmap, i)) {
			setbit(bitmap, i);
			/* we would use 2 bits only */
			map->replay_counter++;
			/* ensure a non-zero replay counter value */
			if (!(map->replay_counter & 3))
				map->replay_counter = 1;
			return (i | map->replay_counter << 6);
		}
	}
	return WLFC_MAC_DESC_ID_INVALID;
}

void
wlfc_release_MAC_descriptor_handle(struct wlfc_mac_desc_handle_map* map, uint8 handle)
{

	if (handle < WLFC_MAC_DESC_ID_INVALID) {
		/* unset the allocation flag in bitmap */
		clrbit((uint8*)map->bitmap, WLFC_MAC_DESC_GET_LOOKUP_INDEX(handle));
	}
	return;
}

#endif /* PROP_TXSTATUS */

#define	CID_FMT_HEX	0x9999

#ifdef FWID
static const char BCMATTACHDATA(rstr_fmt_hello)[] =
	"wl%d: Broadcom BCM%s 802.11 Wireless Controller %s FWID 01-%x\n";
#else
static const char BCMATTACHDATA(rstr_fmt_hello)[] =
	"wl%d: Broadcom BCM%s 802.11 Wireless Controller %s\n";
#endif /* FWID */

#if defined(WLC_HIGH) && defined(WLC_LOW)
static void
wl_devpwrstchg_notify(void *wl_p, bool hostmem_access_enabled)
{
	wl_info_t *wl = (wl_info_t *)wl_p;
	WL_PRINT(("%s: hostmem_access_enabled %d\n", __FUNCTION__, hostmem_access_enabled));
	wlc_devpwrstchg_change(wl->wlc, hostmem_access_enabled);
}
static void
wl_generate_pme_to_host(void *wl_p, bool state)
{
	wl_info_t *wl = (wl_info_t *)wl_p;
	wlc_generate_pme_to_host(wl->wlc, state);
}
#endif /* defined(WLC_HIGH) && defined(WLC_LOW) */

/**
 * xxx wl_probe is always a RAM function.  It should be static, but is
 * not because BCMROMBUILD builds say 'defined but not used'.
 */
void *
BCMATTACHFN(wl_probe)(hnd_dev_t *dev, void *regs, uint bus, uint16 devid,
                      uint coreid, uint unit)
{
	wl_info_t *wl;
	wlc_info_t *wlc;
	wl_cmn_data_t *pdata = NULL;
	osl_t *osh;
	uint err;
	char cidstr[8];
	uint orig_unit = unit;

#ifdef BCMPCIEDEV
	/* When we have two full dongle radios hooked up to the host (as is the case
	 * with router platforms), we need to make sure the instances come up with
	 * different wl units - wl0 and wl1. But, since one dongle is not aware of
	 * the other, we need this information to come from the host.
	 */
	uint wlunit = 0;
	extern char *_vars;
	wlunit = getintvar(_vars, "wlunit");
	unit += wlunit;
#endif /* BCMPCIEDEV */

	if (dev->commondata)
		pdata = (wl_cmn_data_t *)dev->commondata;
	else {
		/* allocate private info */
		if (!(pdata = dev->commondata =
			(wl_cmn_data_t *)MALLOCZ(NULL, sizeof(wl_cmn_data_t)))) {
			WL_ERROR(("wl%d: Private data shared MALLOC failed\n", unit));
			return NULL;
		}
	}

	/* allocate private info */
	if (!(wl = (wl_info_t *)MALLOCZ(NULL, sizeof(wl_info_t)))) {
		WL_ERROR(("wl%d: MALLOC failed\n", unit));
		return NULL;
	}

	wl->unit = unit;
	wl->dev = dev;

#ifdef PROP_TXSTATUS
	/* default use hanger */
	wl->wlfc_mode = WLFC_MODE_HANGER;

#if defined(BCMPCIEDEV) && !defined(REUSE_SEQ_DISABLED)
	if (BCMPCIEDEV_ENAB()) {
		WLFC_SET_REUSESEQ(wl->wlfc_mode, 1);
	}
#endif /* BCMPCIEDEV */
#endif /* PROP_TXSTATUS */

#ifdef SHARED_OSL_CMN
	/* Need to pass existing osl_cmn */
	if (pdata)
		osh = osl_attach(dev, &pdata->osh_cmn);
	else
		osh = osl_attach(dev, NULL);
#else
	osh = osl_attach(dev);
#endif /* SHARED_OSL_CMN */

#ifdef ECOUNTERS
	if (ECOUNTERS_ENAB()) {
		ecounters_register_entity_entry_point(ECOUNTERS_TOP_LEVEL_SW_ENTITY_WL,
			wl_ecounters_entry_point);
	}
#endif // endif

#ifdef WLC_LOW_ONLY
	wl->rpc_th = bcm_rpc_tp_attach(osh, dev);
	if (wl->rpc_th == NULL) {
		WL_ERROR(("wl%d: bcm_rpc_tp_attach failed\n", unit));
		goto fail;
	}
	bcm_rpc_tp_txflowctlcb_init(wl->rpc_th, wl, wl_rpc_txflowctl);

	wl->rpc = bcm_rpc_attach(NULL, NULL, wl->rpc_th, NULL);

	if (wl->rpc == NULL) {
		WL_ERROR(("wl%d: bcm_rpc_attach failed\n", unit));
		goto fail;
	}

	bcm_rpc_tp_agg_limit_set(wl->rpc_th,
		BCM_RPC_TP_DNGL_AGG_DEFAULT_SFRAME, BCM_RPC_TP_DNGL_AGG_DEFAULT_BYTE);

#endif /* WLC_LOW_ONLY */

#ifdef WL_OBJ_REGISTRY
	/* Registry is to be created only once for RSDB;
	 * Both WLC will share information over wlc->objr
	*/
	if (pdata) {
		if (!pdata->objreg) {
			wl->objr =
				obj_registry_alloc(osh, OBJR_MAX_KEYS);
				obj_registry_set(wl->objr, OBJR_SELF, wl->objr);
			pdata->objreg = (void *) wl->objr;
		} else {
			wl->objr = pdata->objreg;
		}
	}

	obj_registry_ref(wl->objr, OBJR_SELF);
#endif /* WL_OBJ_REGISTRY */
	/* common load-time initialization */
	if (!(wlc = wlc_attach(wl,			/* wl */
	                       VENDOR_BROADCOM,		/* vendor */
	                       devid,			/* device */
	                       unit,			/* unit */
	                       FALSE,			/* piomode */
	                       osh,			/* osh */
	                       regs,			/* regsva */
	                       bus,			/* bustype */
#ifdef WLC_LOW_ONLY
	                       wl->rpc,			/* BMAC, overloading, to change */
#else
			       wl,			/* sdh */
#endif // endif
	                       wl->objr,		/* Registry for RSDB usage */
	                       &err))) {		/* perr */
		WL_ERROR(("wl%d: wlc_attach failed with error %d\n", unit, err));
		goto fail;
	}

	wl->wlc = (void *)wlc;
	wl->pub = wlc_pub((void *)wlc);
	wl->wlc_hw = wlc->hw;

	wl->dpcTimer = hnd_timer_create(NULL, wl, _wl_dpc, NULL);
	if (wl->dpcTimer == NULL)
		goto fail;

	snprintf(dev->name, HND_DEV_NAME_MAX, "wl%d", unit);

	if (si_chipid(wlc->hw->sih) < CID_FMT_HEX)
		snprintf(cidstr, sizeof(cidstr), "%04x", si_chipid(wlc->hw->sih));
	else
		snprintf(cidstr, sizeof(cidstr), "%d", si_chipid(wlc->hw->sih));

#ifdef FWID
	/* print hello string */
	printf(rstr_fmt_hello, unit, cidstr, EPI_VERSION_STR, gFWID);
#else
	/* print hello string */
	printf(rstr_fmt_hello, unit, cidstr, EPI_VERSION_STR);
#endif // endif

#ifndef RTE_POLL
	if (hnd_isr_register(0, coreid, orig_unit, wl_isr, dev, bus) ||
#ifdef THREAD_SUPPORT
	    hnd_dpc_register(0, coreid, orig_unit, wl_dpc_thread, dev, bus) ||
#endif // endif
	    FALSE) {
		WL_ERROR(("wl%d: hnd_isr_register failed\n", unit));
		goto fail;
	}
#endif	/* RTE_POLL */

#ifdef PROP_TXSTATUS
	if (PROP_TXSTATUS_ENAB(wlc->pub)) {
		if (BCME_OK != wlfc_initialize(wl, wlc)) {
			WL_ERROR(("wl%d: wlfc_initialize failed\n", unit));
			goto fail;
		}
	}
#endif // endif

#ifdef WLC_LOW_ONLY
	wl->rpc_dispatch_ctx.rpc = wl->rpc;
	wl->rpc_dispatch_ctx.wlc = wlc;
	wl->rpc_dispatch_ctx.wlc_hw = wlc->hw;
	bcm_rpc_rxcb_init(wl->rpc, &wl->rpc_dispatch_ctx, wl_rpc_bmac_dispatch, wl,
	                  wl_rpc_down, wl_rpc_resync, NULL);

	hnd_cons_add_cmd("wlhist", do_wlhist_cmd, wl);
	hnd_cons_add_cmd("dpcdump", do_wldpcdump_cmd, wl);

#if defined(HND_PT_GIANT) && defined(DMA_TX_FREE)
	wl->lowmem_free_info.free_fn = wl_lowmem_free;
	wl->lowmem_free_info.free_arg = wl;

	hnd_pt_lowmem_register(&wl->lowmem_free_info);
#endif /* HND_PT_GIANT && DMA_TX_FREE */

#else /* WLC_LOW_ONLY */

#ifdef STA
	/* algin watchdog with tbtt indication handling in PS mode */
	wl->pub->align_wd_tbtt = TRUE;

	/* Enable TBTT Interrupt */
	wlc_bmac_enable_tbtt(wlc->hw, TBTT_WD_MASK, TBTT_WD_MASK);
#endif // endif

	wlc_eventq_set_ind(wlc->eventq, WLC_E_IF, TRUE);

#if defined(WLPFN) && !defined(WLPFN_DISABLED)
	/* initialize PFN handler state */
	if ((wl->pfn = wl_pfn_attach(wlc)) == NULL) {
		WL_ERROR(("wl%d: wl_pfn_attach failed\n", unit));
		goto fail;
	}
	wl->pub->_wlpfn = TRUE;
#endif /* WLPFN */

#if defined(TOE) && !defined(TOE_DISABLED)
	/* allocate the toe info struct */
	if ((wl->toei = wl_toe_attach(wlc)) == NULL) {
		WL_ERROR(("wl%d: wl_toe_attach failed\n", unit));
		goto fail;
	}
#endif /* TOE && !TOE_DISABLED */

	/* allocate the keep-alive info struct */
	if ((wl->keep_alive_info = wl_keep_alive_attach(wlc)) == NULL) {
		WL_ERROR(("wl%d: wl_keep_alive_attach failed\n", unit));
		goto fail;
	}

#ifdef WL_EVENTQ
	/* allocate wl_eventq info struct */
	if ((wl->wlevtq = wl_eventq_attach(wlc)) == NULL) {
		WL_ERROR(("wl%d: wl_eventq_attach failed\n", unit));
		goto fail;
	}
#endif /* WL_EVENTQ */

#if (defined(P2PO) && !defined(P2PO_DISABLED)) || (defined(ANQPO) && \
	!defined(ANQPO_DISABLED)) || (defined(WL_MBO) && !defined(WL_MBO_DISABLED))
	/* allocate gas info struct */
	if ((wl->gas = wl_gas_attach(wlc, wl->wlevtq)) == NULL) {
		WL_ERROR(("wl%d: wl_gas_attach failed\n", unit));
		goto fail;
	}
#endif /* P2PO || ANQPO */

#if defined(P2PO) && !defined(P2PO_DISABLED)
	/* allocate the disc info struct */
	if ((wl->disc = wl_disc_attach(wlc)) == NULL) {
		WL_ERROR(("wl%d: wl_disc_attach failed\n", unit));
		goto fail;
	}

	/* allocate the p2po info struct */
	if ((wl->p2po = wl_p2po_attach(wlc, wl->wlevtq, wl->gas, wl->disc)) == NULL) {
		WL_ERROR(("wl%d: wl_p2po_attach failed\n", unit));
		goto fail;
	}
#endif /* defined(P2PO) && !defined(P2PO_DISABLED) */

#if defined(ANQPO) && !defined(ANQPO_DISABLED)
	/* allocate the anqpo info struct */
	if ((wl->anqpo = wl_anqpo_attach(wlc, wl->gas)) == NULL) {
		WL_ERROR(("wl%d: wl_anqpo_attach failed\n", unit));
		goto fail;
	}
#endif /* defined(ANQPO) && !defined(ANPO_DISABLED) */

	/* allocate the packet filter info struct */
	if ((wl->pkt_filter_info = wlc_pkt_filter_attach(wlc)) == NULL) {
		WL_ERROR(("wl%d: wlc_pkt_filter_attach failed\n", unit));
		goto fail;
	}
#if defined(D0_COALESCING)
	/* allocate the packet filter info struct */
	if ((wl->d0_filter_info = wlc_d0_filter_attach(wlc)) == NULL) {
		WL_ERROR(("wl%d: wlc_pkt_filter_attach failed\n", unit));
		goto fail;
	}
#endif /* D0_COALESCING */

#if defined(NWOE) && !defined(NWOE_DISABLED)
	/* allocate the nwoe info struct */
	if ((wl->nwoei = wl_nwoe_attach(wlc)) == NULL) {
		WL_ERROR(("wl%d: wl_nwoe_attach failed\n", unit));
		goto fail;
	}
#endif /* NWOE && !NWOE_DISABLED */

#endif /* WLC_HIGH */

#if defined(BCM_OL_DEV) || defined(WLC_HIGH)
#if defined(WLNDOE) && !defined(WLNDOE_DISABLED)
	/* allocate the nd info struct */
	if ((wl->ndi = wl_nd_attach(wlc)) == NULL) {
		WL_ERROR(("wl%d: wl_nd_attach failed\n", unit));
		goto fail;
	}
#endif // endif
#if defined(WLNFC) && !defined(WLNFC_DISABLED)
	/* allocate the nfc info struct */
	if ((wl->nfci = wl_nfc_attach(wlc)) == NULL) {
		WL_ERROR(("wl%d: wl_nfc_attach failed\n", unit));
		goto fail;
	}
#endif /* WLNFC && !WLNFC_DISABLED */
#if defined(ARPOE) && !defined(ARPOE_DISABLED)
	/* allocate the arp info struct */
	if ((wl->arpi = wl_arp_attach(wlc)) == NULL) {
		WL_ERROR(("wl%d: wl_arp_attach failed\n", unit));
		goto fail;
	}
	wl->pub->_arpoe_support = TRUE;
#endif /* ARPOE && !ARPOE_DISABLED */
#ifdef TCPKAOE
	/* allocate the ICMP info struct */
	if ((wl->icmpi = wl_icmp_attach(wlc)) == NULL) {
		WL_ERROR(("wl%d: wl_icmp_attach failed\n", unit));
		goto fail;
	}

	/* allocate the TCP keep-alive info struct */
	if ((wl->tcp_keep_info = wl_tcp_keep_attach(wlc)) == NULL) {
		WL_ERROR(("wl%d: wlc_tcp_keep_attach failed\n", unit));
		goto fail;
	}
#endif // endif
#endif /* defined(BCM_OL_DEV) || defined(WLC_HIGH) */

#ifdef BCMDBG
	hnd_cons_add_cmd("wlmsg", do_wlmsg_cmd, wl);
#endif // endif
#ifdef WLC_HIGH

	/* register module */
	if (wlc_module_register(wlc->pub, wl_iovars, "wl", wl, wl_doiovar, NULL, NULL, NULL)) {
		WL_ERROR(("wl%d: wlc_module_register() failed\n", unit));
		goto fail;
	}

#if defined(BCMDBG) || defined(WLDUMP)
	wlc_dump_register(wl->pub, "wl", (dump_fn_t)wl_dump, (void *)wl);
#endif // endif

#endif /* WLC_HIGH */

#ifdef MSGTRACE
	/* We set up the event and start the tracing immediately */
	wlc_eventq_set_ind(wlc->eventq, WLC_E_TRACE, TRUE);
	msgtrace_init(wlc, wl->dev, (msgtrace_func_send_t)wlc_event_sendup_trace);
#endif // endif

#ifdef LOGTRACE
	/* We set up the event and start the tracing immediately */
	wlc_eventq_set_ind(wlc->eventq, WLC_E_TRACE, TRUE);
	logtrace_init(wlc, wl->dev, (msgtrace_func_send_t)wlc_event_sendup_trace);
	logtrace_start();
#endif // endif
#if defined(WL11K_AP)&& defined(WL_MBO) && !defined(WL_MBO_DISABLED)
	wlc_rrm_update_gasi(wlc, (void*)(wl->gas));
#endif /* WL11K_AP && WL_MBO && !WL_MBO_DISABLED */

	return (wl);

fail:
	wl_free(wl, osh);
	return NULL;
} /* wl_probe */

static void
BCMATTACHFN(wl_free)(wl_info_t *wl, osl_t *osh)
{
#ifndef BCMNODOWN
	if (wl->dpcTimer != NULL)
		hnd_timer_free(wl->dpcTimer);
#if defined(ARPOE) && !defined(ARPOE_DISABLED)
if (wl->arpi)
	wl_arp_detach(wl->arpi);
#endif /* ARPOE && !ARPOE_DISABLED */
#ifdef TCPKAOE
	if (wl->icmpi)
		wl_icmp_detach(wl->icmpi);
	if (wl->tcp_keep_info)
		wl_tcp_keep_detach(wl->tcp_keep_info);
#endif // endif
#ifdef WLC_HIGH
#if defined(D0_COALESCING)
	if (wl->d0_filter_info)
		wlc_d0_filter_detach(wl->d0_filter_info);
#endif /* D0_COALESCING */
	if (wl->pkt_filter_info)
		wlc_pkt_filter_detach(wl->pkt_filter_info);
	if (wl->keep_alive_info)
		wl_keep_alive_detach(wl->keep_alive_info);

#if defined(TOE) && !defined(TOE_DISABLED)
	if (wl->toei)
		wl_toe_detach(wl->toei);
#endif /* TOE && !TOE_DISABLED */

#if defined(NWOE) && !defined(NWOE_DISABLED)
	if (wl->nwoei)
		wl_nwoe_detach(wl->nwoei);
#endif /* NWOE && !NWOE_DISABLED */
#ifdef WLPFN
	if (WLPFN_ENAB(wl->pub) && wl->pfn)
		wl_pfn_detach(wl->pfn);
#endif // endif
#if defined(P2PO) && !defined(P2PO_DISABLED)
	if (wl->p2po)
		wl_p2po_detach(wl->p2po);
	if (wl->disc)
		wl_disc_detach(wl->disc);
#endif /* defined(P2PO) && !defined(P2PO_DISABLED) */
#if defined(ANQPO) && !defined(ANQPO_DISABLED)
	if (wl->anqpo)
		wl_anqpo_detach(wl->anqpo);
#endif /* defined(ANQPO) && !defined(ANQPO_DISABLED) */
#if (defined(P2PO) && !defined(P2PO_DISABLED)) || (defined(ANQPO) && \
	!defined(ANQPO_DISABLED))
	if (wl->gas)
		wl_gas_detach(wl->gas);
#endif	/* P2PO || ANQPO */
#ifdef WL_EVENTQ
	if (wl->wlevtq)
		wl_eventq_detach(wl->wlevtq);
#endif /* WL_EVENTQ */
#endif /* WLC_HIGH */

#if defined(HND_PT_GIANT) && defined(DMA_TX_FREE)
	hnd_pt_lowmem_unregister(&wl->lowmem_free_info);
#endif // endif

#if defined(WLNDOE) && !defined(WLNDOE_DISABLED)
	if (wl->ndi)
		wl_nd_detach(wl->ndi);
#endif // endif

#if defined(WLNFC) && !defined(WLNFC_DISABLED)
	if (wl->nfci)
		wl_nfc_detach(wl->nfci);
#endif // endif

	/* common code detach */
	if (wl->wlc)
		wlc_detach(wl->wlc);

#ifdef WLC_LOW_ONLY
	/* rpc, rpc_transport detach */
	if (wl->rpc)
		bcm_rpc_detach(wl->rpc);
	if (wl->rpc_th)
		bcm_rpc_tp_detach(wl->rpc_th);
#endif /* WLC_LOG_ONLY */

#ifdef WL_OBJ_REGISTRY
	if (wl->objr && (obj_registry_unref(wl->objr, OBJR_SELF) == 0)) {
		obj_registry_set(wl->objr, OBJR_SELF, NULL);
		obj_registry_free(wl->objr, osh);
	}
#endif // endif

#endif /* BCMNODOWN */
	MFREE(osh, wl, sizeof(wl_info_t));
} /* wl_free */

static void
wl_isr(void *cbdata)
{
	hnd_dev_t *dev = cbdata;
#ifdef THREAD_SUPPORT
	wl_info_t *wl = dev->softc;

	/* deassert interrupt */
	wlc_intrs_deassert(wl->wlc);
#else
	wl_run(dev);
#endif	/* THREAD_SUPPORT */
}

#ifdef THREAD_SUPPORT
/** THREAD_SUPPORT specific function */
static void
wl_dpc_thread(void *cbdata)
{
	hnd_dev_t *dev = cbdata;
	wl_run(dev);
}
#endif	/* THREAD_SUPPORT */

static void
wl_run(hnd_dev_t *dev)
{
	wl_info_t *wl = dev->softc;
	bool dpc;

	WL_TRACE(("wl%d: wl_run\n", wl->unit));

	/* call common first level interrupt handler */
	if (wlc_isr(wl->wlc, &dpc)) {
		/* if more to do... */
		if (dpc) {
			wl_dpc(wl);
		}
	}
}

static void
wl_dpc(wl_info_t *wl)
{
	bool resched = 0;
	bool bounded = TRUE;

	/* call the common second level interrupt handler if we have enough memory */
	if (wl->wlc_hw->up) {
		wlc_dpc_info_t dpci = {0};
#ifdef WLC_LOW_ONLY
		if (!wl->dpc_stopped) {
			if (wl->wlc_hw->rpc_dngl_agg & BCM_RPC_TP_DNGL_AGG_DPC) {
				bcm_rpc_tp_agg_set(wl->rpc_th, BCM_RPC_TP_DNGL_AGG_DPC, TRUE);
			}

			resched = wlc_dpc(wl->wlc, bounded, &dpci);

			if (wl->wlc_hw->rpc_dngl_agg & BCM_RPC_TP_DNGL_AGG_DPC) {
				bcm_rpc_tp_agg_set(wl->rpc_th, BCM_RPC_TP_DNGL_AGG_DPC, FALSE);
			}
		} else {
			WL_TRACE(("dpc_stop is set!\n"));
			wl->dpc_requested = TRUE;
			return;
		}
#else
		resched = wlc_dpc(wl->wlc, bounded, &dpci);
#endif /* WLC_LOW_ONLY */
	}

	/* wlc_dpc() may bring the driver down */
	if (!wl->wlc_hw->up)
		return;

	/* Driver is not down. Flush accumulated txrxstatus here */
	wl_busioctl(wl, BUS_FLUSH_CHAINED_PKTS, NULL,
		0, NULL, NULL, FALSE);

	/* re-schedule dpc or re-enable interrupts */
	if (resched) {
		if (!hnd_timer_start(wl->dpcTimer, 0, FALSE))
			ASSERT(FALSE);
	} else
		wlc_intrson(wl->wlc);
} /* wl_dpc */

static void
_wl_dpc(hnd_timer_t *timer)
{
	wl_info_t *wl = (wl_info_t *) hnd_timer_get_data(timer);

	if (wl->wlc_hw->up) {
		wlc_intrsupd(wl->wlc);
		wl_dpc(wl);
	}
}

static int
wl_open(hnd_dev_t *dev)
{
	wl_info_t *wl = dev->softc;
	int ret;

	WL_ERROR(("wl%d: wl_open\n", wl->unit));

#ifdef BCM_OL_DEV
	ret = wlc_up(wl->wlc);
#endif /* BCM_OL_DEV */

	if ((ret = wlc_ioctl(wl->wlc, WLC_UP, NULL, 0, NULL)))
		return ret;

#ifdef HND_JOIN_SSID
	/*
	 * Feature useful for repetitious testing: if Make defines HND_JOIN_SSID
	 * to an SSID string, automatically join that SSID at driver startup.
	 */
	{
		wlc_info_t *wlc = wl->wlc;
		int infra = 1;
		int auth = 0;
		char *ss = HND_JOIN_SSID;
		wlc_ssid_t ssid;

		printf("Joining %s:\n", ss);
		/* set infrastructure mode */
		printf("  Set Infra\n");
		wlc_ioctl(wlc, WLC_SET_INFRA, &infra, sizeof(int), NULL);
		printf("  Set Auth\n");
		wlc_ioctl(wlc, WLC_SET_AUTH, &auth, sizeof(int), NULL);
		printf("  Set SSID %s\n", ss);
		ssid.SSID_len = strlen(ss);
		bcopy(ss, ssid.SSID, ssid.SSID_len);
		wlc_ioctl(wlc, WLC_SET_SSID, &ssid, sizeof(wlc_ssid_t), NULL);
	}
#endif /* HND_JOIN_SSID */

#if defined(PROP_TXSTATUS) || defined(BCMPCIEDEV)
	if (PROP_TXSTATUS_ENAB(((wlc_info_t *)(wl->wlc))->pub) ||
		BCMPCIEDEV_ENAB())
	{
		wlc_bsscfg_t* bsscfg = wlc_bsscfg_find_by_wlcif(wl->wlc, NULL);
#ifdef PROP_TXSTATUS
		wl_send_credit_map(wl);
#endif // endif
		wlc_if_event(wl->wlc, WLC_E_IF_ADD, bsscfg->wlcif);
	}
#endif /* PROP_TXSTATUS */

	return (ret);
}
#ifdef BCM_OL_DEV
void * wl_get_arpi(wl_info_t *wl, struct wl_if *wlif)
{
		return wl->arpi;
}
#ifdef TCPKAOE
void * wl_get_icmpi(wl_info_t *wl, struct wl_if *wlif)
{
		return wl->icmpi;
}
void * wl_get_tcpkeepi(wl_info_t *wl, struct wl_if *wlif)
{
		return wl->tcp_keep_info;
}
#endif /* TCPKAOE */
#endif /* BCM_OL_DEV */

#ifdef WLC_HIGH
#ifdef TOE
/** Process toe frames in receive direction */
static int
_wl_toe_recv_proc(wl_info_t *wl, void *p)
{
	if (TOE_ENAB(wl->pub))
		(void)wl_toe_recv_proc(wl->toei, p);
	return 0;
}
#endif /* TOE */

static bool
wl_hwfilter(wl_info_t *wl, void *p)
{
	struct ether_header *eh = (struct ether_header *)PKTDATA(wl->pub->osh, p);

	if (((wl->hwfflags & HWFFLAG_UCAST) && !ETHER_ISMULTI(eh->ether_dhost)) ||
	    ((wl->hwfflags & HWFFLAG_BCAST) && ETHER_ISBCAST(eh->ether_dhost)))
		return TRUE;

	return FALSE;
}

#ifdef PROP_TXSTATUS
/** PROP_TXSTATUS specific */
int
wlfc_MAC_table_update(struct wl_info *wl, uint8* ea,
	uint8 add_del, uint8 mac_handle, uint8 ifidx)
{
	/* space for type(1), length(1) and value */
	uint8	results[1+1+WLFC_CTL_VALUE_LEN_MACDESC];

	results[0] = add_del;
	results[1] = WLFC_CTL_VALUE_LEN_MACDESC;
	results[2] = mac_handle;
	results[3] = ifidx;
	memcpy(&results[4], ea, ETHER_ADDR_LEN);
	return wlfc_push_signal_data(wl, results, sizeof(results), FALSE);
}

/** PROP_TXSTATUS specific */
wlfc_info_state_t*
wlfc_state_get(struct wl_info *wl)
{
	if (wl != NULL)
		return wl->wlfc_info;
	return NULL;
}

/** PROP_TXSTATUS specific */
int
wlfc_psmode_request(struct wl_info *wl, uint8 mac_handle, uint8 count,
	uint8 ac_bitmap, uint8 request_type)
{
	/* space for type(1), length(1) and value */
	uint8	results[1+1+WLFC_CTL_VALUE_LEN_REQUEST_CREDIT];
	int ret;

	results[0] = request_type;
	if (request_type == WLFC_CTL_TYPE_MAC_REQUEST_PACKET)
		results[1] = WLFC_CTL_VALUE_LEN_REQUEST_PACKET;
	else
		results[1] = WLFC_CTL_VALUE_LEN_REQUEST_CREDIT;
	results[2] = count;
	results[3] = mac_handle;
	results[4] = ac_bitmap;
	ret = wlfc_push_signal_data(wl, results, sizeof(results), FALSE);

	if (ret == BCME_OK)
		ret = wlfc_sendup_ctl_info_now(wl);

	return ret;
}

/** PROP_TXSTATUS specific */
int
wlfc_sendup_ctl_info_now(struct wl_info *wl)
{
	/*
	typical overhead BCMDONGLEOVERHEAD,
	but aggregated sd packets can take 2*BCMDONGLEOVERHEAD
	octets
	*/
	int header_overhead = BCMDONGLEOVERHEAD*3;
	struct lbuf *wlfc_pkt;
	ASSERT(wl != NULL);
	ASSERT(wl->wlfc_info != NULL);

	if (!WLFC_CONTROL_SIGNALS_TO_HOST_ENAB(((wlc_info_t *)(wl->wlc))->pub))
		return BCME_OK;

	/* Two possible reasons for being here - pending data or pending credit */
	if (!wl->wlfc_info->pending_datalen && !wl->wlfc_info->fifo_credit_back_pending)
		return BCME_OK;

	if ((wlfc_pkt = PKTGET(wl->pub->osh,
		(wl->wlfc_info->pending_datalen + header_overhead),
		TRUE)) == NULL) {
		/* what can be done to deal with this?? */
		/* set flag and try later again? */
		WL_ERROR(("PKTGET pkt size %d failed\n", wl->wlfc_info->pending_datalen));
		return BCME_NOMEM;
	}
	PKTPULL(wl->pub->osh, wlfc_pkt, header_overhead + wl->wlfc_info->pending_datalen);
	PKTSETLEN(wl->pub->osh, wlfc_pkt, 0);
	PKTSETTYPEEVENT(wl->pub->osh, wlfc_pkt);
	wl_sendup(wl, NULL, wlfc_pkt, 1);
	if (wl->wlfc_info->pending_datalen) {
		/* not sent by wl_sendup() due to memory issue? */
		WL_ERROR(("wl_sendup failed to send pending_datalen\n"));
		return BCME_NOMEM;
	}

#ifdef PROP_TXSTATUS_DEBUG
	wl->wlfc_info->dbgstats->nullpktallocated++;
#endif // endif
	return BCME_OK;
} /* wlfc_sendup_ctl_info_now */

/** PROP_TXSTATUS specific */
int
wlfc_push_credit_data(struct wl_info *wl, void* p)
{
	uint8 ac;
	uint32 threshold;

	ac = WL_TXSTATUS_GET_FIFO(WLPKTTAG(p)->wl_hdr_information);
	WLPKTTAG(p)->flags |= WLF_CREDITED;

#ifdef PROP_TXSTATUS_DEBUG
	wl->wlfc_info->dbgstats->creditupdates++;
	wl->wlfc_info->dbgstats->credits[ac]++;
#endif // endif
	wl->wlfc_info->fifo_credit_back[ac]++;
	wl->wlfc_info->fifo_credit_back_pending = 1;
	threshold = wl->wlfc_info->fifo_credit_threshold[ac];

	if (wl->wlfc_info->wlfc_trigger & WLFC_CREDIT_TRIGGER) {

		if (wl->wlfc_info->fifo_credit_in[ac] > wl->wlfc_info->fifo_credit[ac]) {
			/* borrow happened */
			threshold = wl->wlfc_info->total_credit /
				wl->wlfc_info->wlfc_fifo_bo_cr_ratio;
		}

		/*
		monitor how much credit is being gathered here. If credit pending is
		larger than a preset threshold, send_it_now(). The idea is to keep
		the host busy pushing packets to keep the pipeline filled.
		*/
		if (wl->wlfc_info->fifo_credit_back[ac] >= threshold) {
			if ((wlfc_sendup_ctl_info_now(wl) != BCME_OK) &&
				(!wl->wlfc_info->timer_started)) {
				wl_add_timer(wl, wl->wlfc_info->fctimer,
					WLFC_SENDUP_TIMER_INTERVAL, 0);
				wl->wlfc_info->timer_started = 1;
			}
		}
	}

	return BCME_OK;
}

/** PROP_TXSTATUS specific */
int
wlfc_queue_avail(struct wl_info *wl)
{
	ASSERT(wl->wlfc_info->pending_datalen <= WLFC_MAX_PENDING_DATALEN);

	return (WLFC_MAX_PENDING_DATALEN - wl->wlfc_info->pending_datalen);
}

/** PROP_TXSTATUS specific */
int
wlfc_queue_signal_data(struct wl_info *wl, void* data, uint8 len)
{
	uint8 type = ((uint8*)data)[0];
	bool skip_cp = FALSE;

	ASSERT(wl != NULL);
	ASSERT(wl->wlfc_info != NULL);
	ASSERT((wl->wlfc_info->pending_datalen + len) <= WLFC_MAX_PENDING_DATALEN);

	if ((wl->wlfc_info->pending_datalen + len) > WLFC_MAX_PENDING_DATALEN) {
		WL_ERROR(("wlfc queue full: %d > %d\n",
			wl->wlfc_info->pending_datalen + len,
			WLFC_MAX_PENDING_DATALEN));
		return BCME_ERROR;
	}

	if ((type == WLFC_CTL_TYPE_TXSTATUS) && ((wlc_info_t *)wl->wlc)->comp_stat_enab) {
		uint32 xstatusdata, statusdata = *((uint32 *)((uint8 *)data + TLV_HDR_LEN));
		uint8 xcnt, cnt = WL_TXSTATUS_GET_FREERUNCTR(statusdata);
		uint8 xac, ac = WL_TXSTATUS_GET_FIFO(statusdata);
		uint16 xhslot, hslot = WL_TXSTATUS_GET_HSLOT(statusdata);
		uint8 xstatus, status = WL_TXSTATUS_GET_STATUS(statusdata);
		uint8 cur_pos = 0;
		uint8 bBatched = 0;
		uint32 compcnt_offset = TLV_HDR_LEN + WLFC_CTL_VALUE_LEN_TXSTATUS;

		uint16 xseq = 0, seq = 0;
		uint8 xseq_fromfw = 0, seq_fromfw = 0;
		uint16 xseq_num = 0, seq_num = 0;

		((uint8 *)data)[TLV_TAG_OFF] = WLFC_CTL_TYPE_COMP_TXSTATUS;
		((uint8 *)data)[TLV_LEN_OFF]++;

		if (WLFC_GET_REUSESEQ(wl->wlfc_mode)) { /* TRUE if d11 seq nums are to be reused */
			compcnt_offset += WLFC_CTL_VALUE_LEN_SEQ;
			seq = *((uint16 *)((uint8 *)data + TLV_HDR_LEN +
				WLFC_CTL_VALUE_LEN_TXSTATUS));
			seq_fromfw = WL_SEQ_GET_FROMFW(seq);
			seq_num = WL_SEQ_GET_NUM(seq);
		}

		while (cur_pos < wl->wlfc_info->pending_datalen) {
			if ((wl->wlfc_info->data[cur_pos] == WLFC_CTL_TYPE_COMP_TXSTATUS)) {
				xstatusdata = *((uint32 *)(wl->wlfc_info->data + cur_pos +
					TLV_HDR_LEN));
				/* next expected free run counter */
				xcnt = (WL_TXSTATUS_GET_FREERUNCTR(xstatusdata) +
					wl->wlfc_info->data[cur_pos + compcnt_offset]) &
					WL_TXSTATUS_FREERUNCTR_MASK;
				/* next expected fifo number */
				xac = WL_TXSTATUS_GET_FIFO(statusdata);
				/* next expected slot number */
				xhslot = WL_TXSTATUS_GET_HSLOT(xstatusdata);
				if (!WLFC_GET_AFQ(wl->wlfc_mode)) {
					/* for hanger, it needs to be consective */
					xhslot = (xhslot + wl->wlfc_info->data[cur_pos +
						compcnt_offset]) & WL_TXSTATUS_HSLOT_MASK;
				}
				xstatus = WL_TXSTATUS_GET_STATUS(xstatusdata);

				if (WLFC_GET_REUSESEQ(wl->wlfc_mode)) {
					xseq = *((uint16 *)(wl->wlfc_info->data + cur_pos +
						TLV_HDR_LEN + WLFC_CTL_VALUE_LEN_TXSTATUS));
					xseq_fromfw = WL_SEQ_GET_FROMFW(xseq);
					/* next expected seq num */
					xseq_num = (WL_SEQ_GET_NUM(xseq) + wl->wlfc_info->data[
						cur_pos + compcnt_offset]) & WL_SEQ_NUM_MASK;
				}

				if ((cnt == xcnt) && (hslot == xhslot) &&
					(status == xstatus) && (ac == xac)) {
					if (!WLFC_GET_REUSESEQ(wl->wlfc_mode) ||
						((seq_fromfw == xseq_fromfw) &&
						(!seq_fromfw || (seq_num == xseq_num)))) {
						wl->wlfc_info->data[cur_pos + compcnt_offset]++;
						bBatched = 1;
						wl->wlfc_info->compressed_stat_cnt++;
						break;
					}
				}
			}
			cur_pos += wl->wlfc_info->data[cur_pos + TLV_LEN_OFF] + TLV_HDR_LEN;
		}

		if (!bBatched) {
			memcpy(&wl->wlfc_info->data[wl->wlfc_info->pending_datalen], data, len);
			wl->wlfc_info->data[wl->wlfc_info->pending_datalen + len] = 1;
			wl->wlfc_info->pending_datalen += len + 1;
		}

		skip_cp = TRUE;
	}

	if (!skip_cp) {
		memcpy(&wl->wlfc_info->data[wl->wlfc_info->pending_datalen], data, len);
		wl->wlfc_info->pending_datalen += len;
	}

	return BCME_OK;
} /* wlfc_queue_signal_data */

/**
 * PROP_TXSTATUS specific, called by various wlc_*.c files to signal wlfc events (e.g. MAC_OPEN or
 * MAC_CLOSE) that are ultimately consumed by the host (USB) or by the firmware bus layer (PCIe).
 *     @param [in] data : a single TLV
 *     @param [in] len  : length of TLV in bytes, including 'TL' fields.
 */
int
wlfc_push_signal_data(struct wl_info *wl, void* data, uint8 len, bool hold)
{
	int rc = BCME_OK;
	uint8 type = ((uint8*)data)[0]; /* tlv type */
	uint8 tlv_flag;		/* how wlfc between host and fw is configured */
	uint32 tlv_mask;	/* to prevent certain TLV types from reaching the host */

	ASSERT(wl != NULL);
	ASSERT(wl->wlfc_info != NULL);

	tlv_flag = ((wlc_info_t *)(wl->wlc))->wlfc_flags;

	tlv_mask = ((tlv_flag & WLFC_FLAGS_XONXOFF_SIGNALS) ?
		WLFC_FLAGS_XONXOFF_MASK : 0) |
#ifdef WLFCTS /* time stamp */
		((WLFCTS_ENAB(wl->pub) &&
		(tlv_flag & WLFC_FLAGS_PKT_STAMP_SIGNALS)) ?
		WLFC_FLAGS_PKT_STAMP_MASK : 0) |
#endif /* WLFCTS */
		((WLFC_CONTROL_SIGNALS_TO_HOST_ENAB(wl->pub) &&
		(tlv_flag & WLFC_FLAGS_CREDIT_STATUS_SIGNALS)) ?
		WLFC_FLAGS_CREDIT_STATUS_MASK : 0) |
		0;

#ifdef BCMPCIEDEV
	if (BCMPCIEDEV_ENAB() && WLFC_INFO_TO_BUS_ENAB(wl->pub)) {
		 rc = wlfc_push_signal_bus_data(wl, data, len);
	} else
#endif /* BCMPCIEDEV */
	{
		/* if the host does not want these TLV signals, drop it */
		if (!(tlv_mask & (1 << type))) {
			WLFC_DBGMESG(("%s() Dropping signal, type:%d, mask:%08x, flag:%d\n",
				__FUNCTION__, type, tlv_mask, tlv_flag));
			return BCME_OK;
		}

		if ((wl->wlfc_info->pending_datalen + len) > WLFC_MAX_PENDING_DATALEN) {
			if (BCME_OK != (rc = wlfc_sendup_ctl_info_now(wl))) {
				/* at least the caller knows we have failed */
				WL_ERROR(("%s() Dropping %d bytes data\n", __FUNCTION__, len));
				return rc;
			}
		}

		wlfc_queue_signal_data(wl, data, len);
		if (!(((wlc_info_t *)wl->wlc)->comp_stat_enab))
			hold = FALSE;

		rc = wlfc_send_signal_data(wl, hold);
	}

	return rc;
} /* wlfc_push_signal_data */

/** PROP_TXSTATUS specific */
int
wlfc_send_signal_data(struct wl_info *wl, bool hold)
{
	int rc = BCME_OK;

	ASSERT(wl != NULL);
	ASSERT(wl->wlfc_info != NULL);

	if ((wl->wlfc_info->pending_datalen > wl->wlfc_info->pending_datathresh) ||
		(!hold && (wl->wlfc_info->wlfc_trigger & WLFC_TXSTATUS_TRIGGER) &&
		(wl->wlfc_info->compressed_stat_cnt > wl->wlfc_info->wlfc_comp_txstatus_thresh))) {
		rc = wlfc_sendup_ctl_info_now(wl);
		if (rc == BCME_OK)
			return BCME_OK;
	}

	if (!wl->wlfc_info->timer_started) {
		wl_add_timer(wl, wl->wlfc_info->fctimer,
			WLFC_SENDUP_TIMER_INTERVAL, 0);
		wl->wlfc_info->timer_started = 1;
	}
	return rc;
}

void
wlfc_process_wlhdr_complete_txstatus(struct wl_info *wl, uint8 status_flag, void* p,
	void *ptxs, bool hold)
{
	wlc_info_t *wlc = (wlc_info_t *)wl->wlc;
#ifdef WLFCTS
	tx_status_t *txs = (tx_status_t *)ptxs;
#endif /* WLFCTS */
	/* space for type(1), length(1) and value */
	uint8	results[TLV_HDR_LEN + WLFC_CTL_VALUE_LEN_TXSTATUS + WLFC_CTL_VALUE_LEN_SEQ +
		sizeof(wl_txstatus_additional_info_t)];
	uint32  statussize = 0, statusdata = 0;
	uint32	wlhinfo;
	bool	amsdu_pkt, pushdata;
	uint16	amsdu_seq = 0;
#ifdef DMATXRC
	void    *phdr = NULL;
#endif /* DMATXRC */
#if defined(DMATXRC) && defined(PKTC_TX_DONGLE)
	int     n_wlhdr = 0;
	int     i = 0;
#endif /* DMATXRC && PKTC_TX_DONGLE */
	uint16	seq;
	bool metadatabuf_avial = TRUE;
	bool short_status = TRUE;

#if defined(DMATXRC)
	if (DMATXRC_ENAB(wlc->pub) && (WLPKTTAG(p)->flags & WLF_PHDR)) {
		txrc_ctxt_t *rctxt;

		phdr = p;
		rctxt = TXRC_PKTTAIL(wlc->osh, phdr);
		ASSERT(TXRC_ISMARKER(rctxt));

#ifndef BCMDBG_ASSERT
		BCM_REFERENCE(rctxt);
#endif // endif

#ifdef PKTC_TX_DONGLE
		if (PKTC_ENAB(wlc->pub) && TXRC_ISRECLAIMED(rctxt) && TXRC_ISWLHDR(rctxt)) {
			n_wlhdr = rctxt->n;
			ASSERT(n_wlhdr);
		}
#endif // endif
	}
#endif /* DMATXRC */

	amsdu_pkt = WLPKTFLAG_AMSDU(WLPKTTAG(p));
#ifdef BCMPCIEDEV
	if (BCMPCIEDEV_ENAB() && amsdu_pkt) {
		if (WLFC_GET_REUSESEQ(wlfc_query_mode(wl))) {
			/* Use same seq number for all pkts in AMSDU
			 * for suppression so we can re-aggregate
			 * them.
			 */
			amsdu_seq = WLPKTTAG(p)->seq;
			if (WL_SEQ_GET_FROMFW(amsdu_seq))
				WL_SEQ_SET_AMSDU(amsdu_seq, 1);
		} else
			amsdu_seq = 0;
	}
#endif /* BCMPCIEDEV */

	if (!TXMETADATA_TO_HOST_ENAB(wlc->pub))
		metadatabuf_avial = FALSE;
#ifdef BCMPCIEDEV
	else if (BCMPCIEDEV_ENAB()) {
		metadatabuf_avial = !!PKTFRAGMETADATALEN(wlc->osh, p);
	}
#endif /* BCMPCIEDEV */

	do {
		wlhinfo = WLPKTTAG(p)->wl_hdr_information;
		statusdata = 0;
		BCM_REFERENCE(statusdata);
		pushdata = TRUE;

		/* send txstatus only if this packet came from the host */
		if (WL_TXSTATUS_GET_FLAGS(wlhinfo) & WLFC_PKTFLAG_PKTFROMHOST) {
			if (!(WLPKTTAG(p)->flags & WLF_PROPTX_PROCESSED)) {
				WLPKTTAG(p)->flags |= WLF_PROPTX_PROCESSED;
				if (WLFC_CONTROL_SIGNALS_TO_HOST_ENAB(wlc->pub) &&
					!(WL_TXSTATUS_GET_FLAGS(wlhinfo) &
					WLFC_PKTFLAG_PKT_REQUESTED)) {
					/* if this packet was intended for AC FIFO and ac credit
					 * has not been sent back, push a credit here
					 */
					if (!(WLPKTTAG(p)->flags & WLF_CREDITED)) {
						wlfc_push_credit_data(wl, p);
					}
				}

				/* When BCM_DHDHDR enabled, save txs and seq info at
				 * latest 2B, 4B od pkttag, because we may not have the
				 * data buffer for the packet.
				 */
#ifdef BCM_DHDHDR
				if (BCMDHDHDR_ENAB()) {
					ASSERT(!metadatabuf_avial);

					/* Save txstatus */
					PKTFRAGSETTXSTATUS(wlc->osh, p, status_flag);

					/* Save seq */
					PKTSETWLFCSEQ(wlc->osh, p, 0);
				}
#endif /* BCM_DHDHDR */

				while (pushdata) {
					if (metadatabuf_avial) {
						results[TLV_TAG_OFF] = WLFC_CTL_TYPE_TXSTATUS;
						results[TLV_LEN_OFF] = WLFC_CTL_VALUE_LEN_TXSTATUS;

						WL_TXSTATUS_SET_PKTID(statusdata,
							WL_TXSTATUS_GET_PKTID(wlhinfo));
						WL_TXSTATUS_SET_FIFO(statusdata,
							WL_TXSTATUS_GET_FIFO(wlhinfo));
						WL_TXSTATUS_SET_FLAGS(statusdata, status_flag);
						WL_TXSTATUS_SET_GENERATION(statusdata,
							WL_TXSTATUS_GET_GENERATION(wlhinfo));
						memcpy(&results[TLV_BODY_OFF + statussize],
							&statusdata, sizeof(uint32));
						statussize += sizeof(uint32);
						short_status = FALSE;
					} else if (!status_flag)
						goto send_shortstatus;
					/* temporarily not return original seqenuce number
					 * for amsdu packet
					 * XXX: Why? Was this done for SDIO specifically?
					 */
					seq = amsdu_pkt ? amsdu_seq : WLPKTTAG(p)->seq;
#ifdef WLFCTS
					/* Send a timestamp back to host only if enabled */
					if (WLFCTS_ENAB(wlc->pub) && metadatabuf_avial &&
					   (wl->wlfc_flags &
					   WLFC_FLAGS_PKT_STAMP_SIGNALS)) {
						wl_txstatus_additional_info_t tx_add_info;

						if (txs) {
							tx_add_info.rspec =	WLPKTTAG(p)->rspec;
							tx_add_info.enq_ts = txs->dequeuetime;
							tx_add_info.last_ts = txs->lasttxtime;

							/* Append sequence number and RTS
							 * count
							 */
							tx_add_info.seq =
								WLPKTTAG(p)->seq << SEQNUM_SHIFT;
							tx_add_info.rts_cnt =
								txs->status.rts_tx_cnt;
							tx_add_info.tx_cnt =
								WLPKTFLAG_AMPDU(WLPKTTAG(p)) ? 1 :
								txs->status.frag_tx_cnt;
						} else {
							tx_add_info.rspec = 0;
							tx_add_info.enq_ts = 0;

							if (si_iscoreup(wlc->pub->sih))
								tx_add_info.last_ts =
								R_REG(wlc->osh,
								&wl->regs->tsf_timerlow);
							else
								tx_add_info.last_ts = 0;

							tx_add_info.seq = 0;
							tx_add_info.rts_cnt = 0;
							tx_add_info.tx_cnt = 0;
						}

						/* Include TX entry timestamp */
						tx_add_info.entry_ts =
							WLPKTTAG(p)->shared.tx_entry_tstamp;

						memcpy(&results[TLV_BODY_OFF + statussize],
							&tx_add_info, sizeof(tx_add_info));
						statussize += sizeof(tx_add_info);
					} else
#endif /* WLFCTS */
					if ((metadatabuf_avial ||
						WLFC_INFO_TO_BUS_ENAB(wlc->pub)) &&
						WLFC_GET_REUSESEQ(wlfc_query_mode(wl))) {
						if ((status_flag ==
							WLFC_CTL_PKTFLAG_D11SUPPRESS) ||
						    (status_flag ==
							WLFC_CTL_PKTFLAG_WLSUPPRESS)) {
							if (WL_SEQ_GET_FROMDRV(seq)) {
								WL_SEQ_SET_FROMFW(seq, 1);
							}
							WL_SEQ_SET_FROMDRV(seq, 0);
						} else {
							seq = 0;
						}
						/* If we could not populate txstatus, but need to
						* update, then do it now
						*/
						/* We did above to save txstatus already */
						if (!BCMDHDHDR_ENAB()) {
							if ((seq &&
								WLFC_INFO_TO_BUS_ENAB(wlc->pub)) &&
								!metadatabuf_avial) {
								results[TLV_TAG_OFF] =
									WLFC_CTL_TYPE_TXSTATUS;
								results[TLV_LEN_OFF] =
									WLFC_CTL_VALUE_LEN_TXSTATUS;
								WL_TXSTATUS_SET_FLAGS(statusdata,
									status_flag);
								memcpy(&results[TLV_BODY_OFF],
									&statusdata,
									sizeof(uint32));
								short_status = FALSE;
							}
						}
						if (metadatabuf_avial ||
							(seq && WLFC_INFO_TO_BUS_ENAB(wlc->pub))) {
#ifdef BCM_DHDHDR
							if (BCMDHDHDR_ENAB()) {
								/* Updata seq if needed */
								PKTSETWLFCSEQ(wlc->osh, p, seq);
							} else
#endif /* BCM_DHDHDR */
							{
								memcpy(&results[TLV_HDR_LEN +
									WLFC_CTL_VALUE_LEN_TXSTATUS
									], &seq,
									WLFC_CTL_VALUE_LEN_SEQ);
								results[TLV_LEN_OFF] +=
									WLFC_CTL_VALUE_LEN_SEQ;
								statussize = results[TLV_LEN_OFF];
							}
						}
					}
					statussize += TLV_HDR_LEN;
					send_shortstatus :

					if (!BCMDHDHDR_ENAB()) {
						if (short_status) {
							results[0] = status_flag;
							statussize = 1;
						}
					}
#ifdef BCMPCIEDEV
					if (!WLFC_CONTROL_SIGNALS_TO_HOST_ENAB(wlc->pub)) {
#ifdef BCM_DHDHDR
						if (BCMDHDHDR_ENAB()) {
							/* Set state to TXstatus processed */
							PKTSETTXSPROCESSED(wlc->osh, p);
						} else
#endif /* BCM_DHDHDR */
						{
							wlfc_push_pkt_txstatus(wl, p, results,
								statussize);
						}
					}
					else
#endif /* BCMPCIEDEV  */
						wlfc_push_signal_data(wl, results,
							statussize, hold);

#if defined(DMATXRC) && defined(PKTC_TX_DONGLE)
					/* If pkt chain was reclaimed, we loop through wlhdr[]
					 * (if any) and with phdr only, we end outside loop since
					 * there's no next pkt
					 */
					if (DMATXRC_ENAB(wlc->pub) &&
						PKTC_ENAB(wlc->pub) && (i < n_wlhdr)) {
						wlhinfo = TXRC_PKTTAIL(wlc->osh, phdr)->wlhdr[i];
						seq = amsdu_pkt ?
							0 : TXRC_PKTTAIL(wlc->osh, phdr)->seq[i];
						i++;
						ASSERT(wlhinfo);
					} else
#endif // endif
						break;
				}
				/* Dont clear, we use hdrinfo  even after posting status
				 * The new WLF_PROPTX_PROCESSED flag prevents duplicate posting
				 * WLPKTTAG(p)->wl_hdr_information = 0;
				 */
				if (WLFC_CONTROL_SIGNALS_TO_HOST_ENAB(wlc->pub) &&
					(status_flag == WLFC_CTL_PKTFLAG_D11SUPPRESS)) {
					wlfc_sendup_ctl_info_now(wl);
				}
				WLFC_COUNTER_TXSTATUS_COUNT(wlc);
			}
		}
		else {
			WLFC_COUNTER_TXSTATUS_OTHER(wlc);
		}
	} while (amsdu_pkt && ((p = PKTNEXT(wlc->osh, p)) != NULL));

	return;
}

#ifdef METADATA_TO_HOST
/** METADATA_TO_HOST && PROP_TXSTATUS specific */
static int
wl_sendup_txstatus(wl_info_t *wl, void **pp)
{
	wlfc_info_state_t* wlfc = (wlfc_info_state_t*)wl->wlfc_info;
	uint8* wlfchp;
	uint8 required_headroom;
	uint8 wl_hdr_words = 0;
	uint8 fillers = 0;
	uint8 rssi_space = 0, tstamp_space = 0;
	uint8 seqnumber_space = 0;
	uint8 fcr_tlv_space = 0;
	uint8 ampdu_reorder_info_space = 0;
	void *p = *pp;
	uint32 datalen, datalen_min;
	wlc_info_t *wlc = (wlc_info_t *)wl->wlc;

	wl->wlfc_info->compressed_stat_cnt = 0;

	/* For DATA packets: plugin a RSSI value that belongs to this packet.
	   RSSI TLV = TLV_HDR_LEN + WLFC_CTL_VALUE_LEN_RSSI
	 */
	if (!PKTTYPEEVENT(wl->pub->osh, p)) {
		/* is the RSSI TLV reporting enabled? */
		if ((RXMETADATA_TO_HOST_ENAB(wl->pub)) &&
			(wlc->wlfc_flags & WLFC_FLAGS_RSSI_SIGNALS)) {
			rssi_space = TLV_HDR_LEN + WLFC_CTL_VALUE_LEN_RSSI;
#ifdef WLFCTS
			if (WLFCTS_ENAB(wl->pub))
				rssi_space += 3;	/* will include SNR and sequence */
#endif /* WLFCTS */
		}
#ifdef WLFCTS
		/* Send a timestamp to host only if enabled */
		if (WLFCTS_ENAB(wl->pub) &&
			(wlc->wlfc_flags & WLFC_FLAGS_PKT_STAMP_SIGNALS)) {
			tstamp_space = TLV_HDR_LEN + WLFC_CTL_VALUE_LEN_TIMESTAMP;
		}
#endif /* WLFCTS */
#ifdef WLAMPDU_HOSTREORDER
		/* check if the host reordering info needs to be added from pkttag */
		if (AMPDU_HOST_REORDER_ENAB(wl->pub)) {
			wlc_pkttag_t *pkttag;
			pkttag = WLPKTTAG(p);
			if (pkttag->flags2 & WLF2_HOSTREORDERAMPDU_INFO) {
				ampdu_reorder_info_space = WLHOST_REORDERDATA_LEN + TLV_HDR_LEN;
			}
		}
#endif /* WLAMPDU_HOSTREORDER */
	 }

#ifdef WLFCHOST_TRANSACTION_ID
	seqnumber_space = TLV_HDR_LEN + WLFC_TYPE_TRANS_ID_LEN;
#endif // endif

	if (WLFC_CONTROL_SIGNALS_TO_HOST_ENAB(wl->pub) && wlfc->fifo_credit_back_pending) {
			fcr_tlv_space = TLV_HDR_LEN + WLFC_CTL_VALUE_LEN_FIFO_CREDITBACK;
	}

	datalen_min = rssi_space + tstamp_space
		+ ampdu_reorder_info_space + seqnumber_space;

	datalen = wlfc->pending_datalen + fcr_tlv_space + datalen_min;
	fillers = ROUNDUP(datalen, 4) - datalen;
	required_headroom = datalen + fillers;
	wl_hdr_words = required_headroom >> 2;

	if (PKTHEADROOM(wl->pub->osh, p) < required_headroom) {
		void *p1;
		int plen = PKTLEN(wl->pub->osh, p);

		/* Allocate a packet that will fit all the data */
		if ((p1 = PKTGET(wl->pub->osh, (plen + required_headroom), TRUE)) == NULL) {
			WL_ERROR(("PKTGET pkt size %d failed\n", plen));

			/* There should still be enough room for datalen_min */
			datalen = datalen_min;
			fillers = ROUNDUP(datalen, 4) - datalen;
			required_headroom = datalen + fillers;
			ASSERT(PKTHEADROOM(wl->pub->osh, p) >= required_headroom);
			if (PKTHEADROOM(wl->pub->osh, p) < required_headroom) {
				PKTFREE(wl->pub->osh, p, TRUE);
				return TRUE;
			}

			wl_hdr_words = required_headroom >> 2;
			PKTPUSH(wl->pub->osh, p, required_headroom);
		} else {
			/* Transfer other fields */
			PKTSETPRIO(p1, PKTPRIO(p));
			PKTSETSUMGOOD(p1, PKTSUMGOOD(p));
			bcopy(PKTDATA(wl->pub->osh, p),
				(PKTDATA(wl->pub->osh, p1) + required_headroom), plen);
			wlc_pkttag_info_move(wlc, p, p1);
			PKTFREE(wl->pub->osh, p, TRUE);
			p = p1;
			*pp = p1;
#ifdef PROP_TXSTATUS_DEBUG
			wlfc->dbgstats->realloc_in_sendup++;
#endif // endif
		}
	} else
		PKTPUSH(wl->pub->osh, p, required_headroom);

	wlfchp = PKTDATA(wl->pub->osh, p);

#ifdef WLFCHOST_TRANSACTION_ID
	if (seqnumber_space) {
		uint32 timestamp;

		/* byte 0: ver, byte 1: seqnumber, byte2:byte6 timestamps */
		wlfchp[0] = WLFC_CTL_TYPE_TRANS_ID;
		wlfchp[1] = WLFC_TYPE_TRANS_ID_LEN;
		wlfchp += TLV_HDR_LEN;

		wlfchp[0] = 0;
		/* wrap around is fine */
		wlfchp[1] = wlfc->txseqtohost++;

		/* time stamp of the packet */
		timestamp = hnd_get_reftime_ms();
		bcopy(&timestamp, &wlfchp[2], sizeof(uint32));

		wlfchp += WLFC_TYPE_TRANS_ID_LEN;
	}
#endif /* WLFCHOST_TRANSACTION_ID */

#ifdef WLAMPDU_HOSTREORDER
	if (AMPDU_HOST_REORDER_ENAB(wl->pub) && ampdu_reorder_info_space) {

		wlc_pkttag_t *pkttag = WLPKTTAG(p);
		PKTSETNODROP(wl->pub->osh, p);

		wlfchp[0] = WLFC_CTL_TYPE_HOST_REORDER_RXPKTS;
		wlfchp[1] = WLHOST_REORDERDATA_LEN;
		wlfchp += TLV_HDR_LEN;

		/* zero out the tag value */
		bzero(wlfchp, WLHOST_REORDERDATA_LEN);

		wlfchp[WLHOST_REORDERDATA_FLOWID_OFFSET] =
			pkttag->u.ampdu_info_to_host.ampdu_flow_id;
		wlfchp[WLHOST_REORDERDATA_MAXIDX_OFFSET] =
			AMPDU_BA_MAX_WSIZE -  1;		/* 0 based...so -1 */
		wlfchp[WLHOST_REORDERDATA_FLAGS_OFFSET] =
			pkttag->u.ampdu_info_to_host.flags;
		wlfchp[WLHOST_REORDERDATA_CURIDX_OFFSET] =
			pkttag->u.ampdu_info_to_host.cur_idx;
		wlfchp[WLHOST_REORDERDATA_EXPIDX_OFFSET] =
			pkttag->u.ampdu_info_to_host.exp_idx;

		WL_INFORM(("flow:%d idx(%d, %d, %d), flags 0x%02x\n",
			wlfchp[WLHOST_REORDERDATA_FLOWID_OFFSET],
			wlfchp[WLHOST_REORDERDATA_CURIDX_OFFSET],
			wlfchp[WLHOST_REORDERDATA_EXPIDX_OFFSET],
			wlfchp[WLHOST_REORDERDATA_MAXIDX_OFFSET],
			wlfchp[WLHOST_REORDERDATA_FLAGS_OFFSET]));
		wlfchp += WLHOST_REORDERDATA_LEN;
	}
#endif /* WLAMPDU_HOSTREORDER */

#ifdef WLFCTS
	if (WLFCTS_ENAB(wl->pub) && tstamp_space) {
		uint32 tsf_l = 0;
		if (si_iscoreup(wlc->pub->sih))
			tsf_l = R_REG(wlc->osh, &wlc->regs->tsf_timerlow);

		wlfchp[0] = WLFC_CTL_TYPE_RX_STAMP;
		wlfchp[1] = WLFC_CTL_VALUE_LEN_TIMESTAMP;

		/* convert RX rate, and keep RX retried flag */
		memcpy(&wlfchp[2], &(WLPKTTAG(p)->rspec), 4);
		memcpy(&wlfchp[6], &tsf_l, 4);
		memcpy(&wlfchp[10], &(((wlc_pkttag_t*)WLPKTTAG(p))->shared.rx_tstamp), 4);
		wlfchp += TLV_HDR_LEN + WLFC_CTL_VALUE_LEN_TIMESTAMP;
	}
#endif /* WLFCTS */

	if ((RXMETADATA_TO_HOST_ENAB(wl->pub)) && rssi_space) {
		wlfchp[0] = WLFC_CTL_TYPE_RSSI;
		wlfchp[1] = rssi_space - TLV_HDR_LEN;
		wlfchp[2] = ((wlc_pkttag_t*)WLPKTTAG(p))->pktinfo.misc.rssi;
#ifdef WLFCTS
		if (WLFCTS_ENAB(wl->pub)) {
			wlfchp[3] = ((wlc_pkttag_t*)WLPKTTAG(p))->pktinfo.misc.snr;
			memcpy(&wlfchp[4], &(((wlc_pkttag_t*)WLPKTTAG(p))->seq), 2);
		}
#endif /* WLFCTS */
		wlfchp += rssi_space;
	}

	if (datalen > datalen_min) {
		/* this packet is carrying signals */
		PKTSETNODROP(wl->pub->osh, p);

		if (wlfc->pending_datalen) {
			memcpy(&wlfchp[0], wlfc->data, wlfc->pending_datalen);
			wlfchp += wlfc->pending_datalen;
			wlfc->pending_datalen = 0;
		}

		/* if there're any fifo credit pending, append it (after pending data) */
		if (WLFC_CONTROL_SIGNALS_TO_HOST_ENAB(wl->pub) && fcr_tlv_space) {
			int i = 0;
			wlfchp[0] = WLFC_CTL_TYPE_FIFO_CREDITBACK;
			wlfchp[1] = WLFC_CTL_VALUE_LEN_FIFO_CREDITBACK;
			memcpy(&wlfchp[2], wlfc->fifo_credit_back,
				WLFC_CTL_VALUE_LEN_FIFO_CREDITBACK);

			for (i = 0; i < WLFC_CTL_VALUE_LEN_FIFO_CREDITBACK; i++) {
				if (wlfc->fifo_credit_back[i]) {
					wlfc->fifo_credit_in[i] -=
						wlfc->fifo_credit_back[i];
					wlfc->fifo_credit_back[i] = 0;
				}
			}
			wlfc->fifo_credit_back_pending = 0;
				wlfchp += TLV_HDR_LEN + WLFC_CTL_VALUE_LEN_FIFO_CREDITBACK;
		}
	}

	if (fillers) {
		memset(&wlfchp[0], WLFC_CTL_TYPE_FILLER, fillers);
	}

	PKTSETDATAOFFSET(p, wl_hdr_words);

	if (wlfc->timer_started) {
		/* cancel timer */
		wl_del_timer(wl, wlfc->fctimer);
		wlfc->timer_started = 0;
	}
	return FALSE;
} /* wl_sendup_txstatus */

#endif /* METADATA_TO_HOST */

/** PROP_TXSTATUS specific */
uint32 wlfc_query_mode(struct wl_info *wl)
{
	return (uint32)(wl->wlfc_mode);
}

#endif /* PROP_TXSTATUS */

static void *
wl_pkt_header_push(wl_info_t *wl, void *p, uint8 *wl_hdr_words)
{
	wl_header_t *h;
	osl_t *osh = wl->pub->osh;
	wlc_pkttag_t *pkttag = WLPKTTAG(p);
	int8 rssi = pkttag->pktinfo.misc.rssi;

	if (PKTHEADROOM(osh, p) < WL_HEADER_LEN) {
		void *p1;
		int plen = PKTLEN(osh, p);

		/* Alloc a packet that will fit all the data; chaining the header won't work */
		if ((p1 = PKTGET(osh, plen + WL_HEADER_LEN, TRUE)) == NULL) {
			WL_ERROR(("PKTGET pkt size %d failed\n", plen));
			PKTFREE(osh, p, TRUE);
			return NULL;
		}

		/* Transfer other fields */
		PKTSETPRIO(p1, PKTPRIO(p));
		PKTSETSUMGOOD(p1, PKTSUMGOOD(p));

		bcopy(PKTDATA(osh, p), PKTDATA(osh, p1) + WL_HEADER_LEN, plen);
		PKTFREE(osh, p, TRUE);

		p = p1;
	} else
		PKTPUSH(osh, p, WL_HEADER_LEN);

	h = (wl_header_t *)PKTDATA(osh, p);
	h->type = WL_HEADER_TYPE;
	h->version = WL_HEADER_VER;
	h->rssi = rssi;
	h->pad = 0;
	/* Return header length in words */
	*wl_hdr_words = WL_HEADER_LEN/4;
	return p;
}

static void
wl_pkt_header_pull(wl_info_t *wl, void *p)
{
	/* Currently this is a placeholder function. We don't process wl header
	   on Tx side as no meaningful fields defined for tx currently.
	 */

#ifdef BCM_DHDHDR
	if (BCMDHDHDR_ENAB()) {
		bzero(PKTFRAGFCTLV(wl->pub->osh, p), PKTDATAOFFSET(p) << 2);
	} else
#endif /* BCM_DHDHDR */
	{
		PKTPULL(wl->pub->osh, p, PKTDATAOFFSET(p));
	}

	return;
}

/* Return the proper arpi pointer for either corr to an IF or
*	default. For IF case, Check if arpi is present. It is possible that, upon a
*	down->arpoe_en->up scenario, interfaces are not reallocated, and
*	so, wl->arpi could be NULL. If so, allocate it and use.
*/
static wl_arp_info_t *
wl_get_arpi(wl_info_t *wl, struct wl_if *wlif)
{
	ASSERT(ARPOE_SUPPORT(wl->pub));

	if (wlif != NULL) {
		if (wlif->arpi == NULL)
			wlif->arpi = wl_arp_alloc_ifarpi(wl->arpi, wlif->wlcif);
		/* note: this could be null if the above wl_arp_alloc_ifarpi fails */
		return wlif->arpi;
	} else
		return wl->arpi;
}

void *
wl_get_ifctx(wl_info_t *wl, int ctx_id, wl_if_t *wlif)
{
	if (ctx_id == IFCTX_ARPI)
		return (void *)wlif->arpi;

#ifdef WLNDOE
	if (ctx_id == IFCTX_NDI)
		return (void *)wlif->ndi;
#endif // endif
	return NULL;
}

#ifdef WLNDOE
/* Return the proper ndi pointer for either corr to an IF or
*	default. For IF case, Check if arpi is present. It is possible that, upon a
*	down->ndoe_en->up scenario, interfaces are not reallocated, and
*	so, wl->ndi could be NULL. If so, allocate it and use.
*/
static wl_nd_info_t *
wl_get_ndi(wl_info_t *wl, struct wl_if *wlif)
{
	if (wlif != NULL) {
		if (wlif->ndi == NULL)
			wlif->ndi = wl_nd_alloc_ifndi(wl->ndi, wlif->wlcif);
		/* note: this could be null if the above wl_arp_alloc_ifarpi fails */
		return wlif->ndi;
	} else
		return wl->ndi;
}
#endif /* WLNDOE */

#ifdef WLNFC
static wl_nfc_info_t *
wl_get_nfci(wl_info_t *wl, struct wl_if *wlif)
{
	if (wlif != NULL) {
		if (wlif->nfci == NULL) {
			wlif->nfci = wl_nfc_alloc_ifnfci(wl->nfci, wlif->wlcif);
		}
		return wlif->nfci;
	} else {
		return wl->nfci;
	}
}
#endif /* WLNFC */

/**
 * The last parameter was added for the build. Caller of this function should pass 1 for now.
 */
void
wl_sendup(wl_info_t *wl, struct wl_if *wlif, void *p, int numpkt)
{
	struct lbuf *lb;
	hnd_dev_t *dev;
	hnd_dev_t *chained;
	int ret_val;
	int no_filter;
	uint8 *buf;
	bool brcm_specialpkt;

	WL_TRACE(("wl%d: wl_sendup: %d bytes\n", wl->unit, PKTLEN(NULL, p)));

	no_filter = 0;
	if (wlif == NULL)
		dev = wl->dev;
	else
		dev = wlif->dev;
	chained = dev->chained;

#ifdef	WL_FRWD_REORDER
	if (FRWD_REORDER_ENAB(((wlc_info_t *)wl->wlc)->pub) &&
		AMPDU_HOST_REORDER_ENAB(wl->pub)) {
		if ((p = wlc_ampdu_frwd_handle_host_reorder(((wlc_info_t *)(wl->wlc))->ampdu_rx,
			p, FALSE)) == NULL)
			return;
	}
#endif /* WLC_FRWD_PKT_REORDER */

	buf = PKTDATA(wl->pub->osh, p);
#ifdef PROP_TXSTATUS
	if (PROP_TXSTATUS_ENAB(((wlc_info_t *)(wl->wlc))->pub))
		brcm_specialpkt = !!PKTTYPEEVENT(wl->pub->osh, p);
	else
#endif // endif
	brcm_specialpkt = ntoh16_ua(buf + ETHER_TYPE_OFFSET) == ETHER_TYPE_BRCM;

	if (!brcm_specialpkt) {
#ifdef TOE
		/* check TOE */
		_wl_toe_recv_proc(wl, p);
#endif /* TOE */

#ifdef NWOE
		if (NWOE_ENAB(wl->pub))
		{
			ret_val = wl_nwoe_recv_proc(wl->nwoei, wl->pub->osh, p);
			if (ret_val == NWOE_PKT_CONSUMED)
				return;
		}
#endif /* NWOE */

		/* Apply ARP offload */
		if (ARPOE_ENAB(wl->pub)) {
			wl_arp_info_t *arpi = wl_get_arpi(wl, wlif);
			struct wlc_if *wlcif = (wlif != NULL) ? wlif->wlcif : NULL;
			wlc_bsscfg_t *bsscfg = wlc_bsscfg_find_by_wlcif(wl->wlc, wlcif);

			if (arpi) {
				ret_val = wl_arp_recv_proc(arpi, p);
				switch (ret_val) {
					case ARP_REQ_SINK:
						if (bsscfg && BSSCFG_AP(bsscfg) &&
							bsscfg->ap_isolate)
						/* for pcie, don't sink, pass it to host */
						break;
						/* fall through is intentional */
					case ARP_REPLY_PEER:
						PKTFREE(wl->pub->osh, p, FALSE);
						return;
					case ARP_FORCE_FORWARD:
						no_filter = 1;
						break;
				}
			}
		}
#ifdef WLNDOE
		/* Apply NS offload */
		if (NDOE_ENAB(wl->pub)) {
			wl_nd_info_t *ndi = wl_get_ndi(wl, wlif);
			if (ndi) {
				ret_val = wl_nd_recv_proc(ndi, p);
				if ((ret_val == ND_REQ_SINK) || (ret_val == ND_REPLY_PEER)) {
					PKTFREE(wl->pub->osh, p, FALSE);
					return;
				}
				if (ret_val == ND_FORCE_FORWARD) {
					no_filter = 1;
				}
			}
		}
#endif // endif

#ifdef WLNFC
		/* Apply Secure WiFi thru NFC */
		if (NFC_ENAB(wl->pub)) {
			wl_nfc_info_t *nfci = wl_get_nfci(wl, wlif);
			if (nfci) {
				wl_nfc_recv_proc(nfci, p);
			}
		}
#endif // endif

#ifdef WL_TBOW
		if (TBOW_ENAB(wl->pub)) {
			if (!tbow_recv_wlandata(((wlc_info_t *)(wl->wlc))->tbow_info, p)) {
				/* tbow packet, don't send up */
				return;
			}
		}
#endif // endif
	}

#ifdef EXT_STA
	if (WLEXTSTA_ENAB(wl->pub) && !brcm_specialpkt) {
		/* rx_ctxt_t is included per packet under EXT_STA
		 * We push the packet and copy it to the front.
		 * From host we need to retrieve it from front and
		 * and pull it back.
		 */
		rx_ctxt_t * rx_ctxt;
		wlc_pkttag_t *pkttag = PKTTAG(p);

		/* Push the packet to make space for rx context */
		PKTPUSH(wl->pub->osh, p, sizeof(rx_ctxt_t));

		rx_ctxt = (rx_ctxt_t *)PKTDATA(wl->pub->osh, p);

		/* convert rpsec to rate in 500Kbps units */
		rx_ctxt->rate = RSPEC2KBPS(pkttag->rspec)/500;
		rx_ctxt->rssi = pkttag->pktinfo.misc.rssi;
		rx_ctxt->channel = pkttag->rxchannel;

	}
#endif /* EXT_STA */

	if (chained) {

		/* Internally generated events have the special ether-type of
		 * ETHER_TYPE_BRCM; do not run these events through data packet filters.
		 */
		if (!brcm_specialpkt) {
			/* Apply packet filter */
			if ((chained->flags & RTEDEVFLAG_HOSTASLEEP) &&
			    wl->hwfflags && !wl_hwfilter(wl, p)) {
				PKTFREE(wl->pub->osh, p, FALSE);
				return;
			}

			/* Apply packet filtering. */
			if (!no_filter && PKT_FILTER_ENAB(wl->pub)) {
				if (!wlc_pkt_filter_recv_proc(wl->pkt_filter_info, p)) {
					/* Discard received packet. */
					PKTFREE(wl->pub->osh, p, FALSE);
					return;
				}
			}
#if defined(D0_COALESCING)
			/* Apply D0 packet filtering. */
			if (D0_FILTER_ENAB(wl->pub)) {
				if (!wlc_d0_filter_recv_proc(wl->d0_filter_info, p)) {
					return;
				}
			}
#endif /* D0_COALESCING */

		}

#ifdef PROP_TXSTATUS
		if (PROP_TXSTATUS_ENAB(wl->pub)) {
#ifdef BCMPCIEDEV
			if (!BCMPCIEDEV_ENAB() || !PKTTYPEEVENT(wl->pub->osh, p))
#endif // endif
			{
				if (RXMETADATA_TO_HOST_ENAB(wl->pub) &&
					wl_sendup_txstatus(wl, &p)) {
					return;
				}
			}
		} else
#endif /* PROP_TXSTATUS */
		{
#ifdef BCMPCIEDEV
			if (!BCMPCIEDEV_ENAB())
#endif // endif
			{
				uint8 wl_hdr_words = 0;
				if ((p = wl_pkt_header_push(wl, p, &wl_hdr_words)) == NULL) {
					return;
				}

				PKTSETDATAOFFSET(p, wl_hdr_words);
			}
		}
		lb = PKTTONATIVE(wl->pub->osh, p);
		if (chained->ops->xmit(dev, chained, lb) != 0) {
			WL_ERROR(("%s: xmit failed; free pkt 0x%p\n", __FUNCTION__, lb));
			lb_free(lb);
		}
	} else {

		/* only AP mode can be non chained */
#ifndef ATE_BUILD  /* for SDIO lite in 4365 */
		ASSERT(AP_ENAB(wl->pub));
#endif // endif
		PKTFREE(wl->pub->osh, p, FALSE);
	}
} /* wl_sendup */
#endif /* WLC_HIGH */

#if defined(D0_COALESCING) || defined(WLAWDL)
void
wl_sendup_no_filter(wl_info_t *wl, struct wl_if *wlif, void *p, int numpkt)
{
	struct lbuf *lb;
	hnd_dev_t *dev;
	hnd_dev_t *chained;

	WL_TRACE(("wl%d: wl_sendup: %d bytes\n", wl->unit, PKTLEN(NULL, p)));

	if (wlif == NULL)
		dev = wl->dev;
	else
		dev = wlif->dev;
	chained = dev->chained;

	if (chained) {

#ifdef PROP_TXSTATUS
		if (PROP_TXSTATUS_ENAB(((wlc_info_t *)(wl->wlc))->pub) &&
			(RXMETADATA_TO_HOST_ENAB(wl->pub))) {
			if (wl_sendup_txstatus(wl, &p)) {
				return;
			}
		}
#endif /* PROP_TXSTATUS */

		lb = PKTTONATIVE(wl->pub->osh, p);
		if (chained->ops->xmit(dev, chained, lb) != 0) {
			WL_ERROR(("wl_sendup: xmit failed; free pkt 0x%p\n", lb));
			lb_free(lb);
		}
	} else {
		/* only AP mode can be non chained */
#ifndef ATE_BUILD  /* for SDIO lite in 4365 */
		ASSERT(AP_ENAB(wl->pub));
#endif // endif
		PKTFREE(wl->pub->osh, p, FALSE);
	}
}
#endif /* defined(D0_COALESCING) || defined(WLAWDL) */

/* buffer received from BUS driver(e.g USB, SDIO) in dongle framework
 *   For normal driver, push it to common driver sendpkt
 *   For BMAC driver, forward to RPC layer to process
 */
#ifdef WLC_HIGH
#ifdef TOE
/**
 * Process toe frames in transmit direction
 * most of the cases TOE is not in ROM, so to avoid big invalidation hooks to check TOE
 */
static void
_wl_toe_send_proc(wl_info_t *wl, void *p)
{
	if (TOE_ENAB(wl->pub))
		wl_toe_send_proc(wl->toei, p);
}
#endif /* TOE */

#ifdef PROP_TXSTATUS
/** PROP_TXSTATUS specific function, called by wl_send(). */
static int
wl_send_txstatus(wl_info_t *wl, void *p)
{
	uint8* wlhdrtodev;
	wlc_pkttag_t *wlpkttag;
	uint8 wlhdrlen;
	uint8 processed = 0;
	uint32 wl_hdr_information = 0;
	uint16 seq = 0;
#ifdef WLFCTS
	uint32 tx_entry_tstamp = 0;
#endif // endif

	ASSERT(wl != NULL);

	wlhdrlen = PKTDATAOFFSET(p) << 2;

#ifdef BCMPCIEDEV
	if (BCMPCIEDEV_ENAB()) {
		/* We do not expect host to set BDC and wl header on PCIEDEV path, So set it now */
		WL_TXSTATUS_SET_FLAGS(wl_hdr_information, WLFC_PKTFLAG_PKTFROMHOST);
	}
#endif // endif
	if (wlhdrlen != 0) {
#ifdef BCM_DHDHDR
		if (BCMDHDHDR_ENAB()) {
			wlhdrtodev = PKTFRAGFCTLV(wl->pub->osh, p);
		} else
#endif /* BCM_DHDHDR */
		{
			wlhdrtodev = (uint8*)PKTDATA(wl->pub->osh, p);
		}

		while (processed < wlhdrlen) {
			if (wlhdrtodev[processed] == WLFC_CTL_TYPE_PKTTAG) {
				wl_hdr_information |=
					ltoh32_ua(&wlhdrtodev[processed + TLV_HDR_LEN]);

				if (WLFC_GET_REUSESEQ(wl->wlfc_mode))
				{
					uint16 reuseseq = ltoh16_ua(&wlhdrtodev[processed +
						TLV_HDR_LEN + WLFC_CTL_VALUE_LEN_TXSTATUS]);
					if (WL_SEQ_GET_FROMDRV(reuseseq)) {
						seq = reuseseq;
					}
				}

				if (WLFC_CONTROL_SIGNALS_TO_HOST_ENAB(wl->pub) &&
					!(WL_TXSTATUS_GET_FLAGS(wl_hdr_information) &
					WLFC_PKTFLAG_PKT_REQUESTED)) {
						uint8 ac = WL_TXSTATUS_GET_FIFO
							(wl_hdr_information);
						wl->wlfc_info->fifo_credit_in[ac]++;
				}
#ifdef WLFCTS
				if (WLFCTS_ENAB(wl->pub)) {
					/* Send a timestamp back to host only if enabled */
					if ((WL_TXSTATUS_GET_FLAGS(wl_hdr_information) &
						WLFC_PKTFLAG_PKTFROMHOST) &&
					    (((wlc_info_t *)(wl->wlc))->wlfc_flags &
						WLFC_FLAGS_PKT_STAMP_SIGNALS)) {
						wlc_process_wlfc_dbg_update((wlc_info_t *)(wl->wlc),
							WLFC_CTL_TYPE_TX_ENTRY_STAMP, p);
					}
				}
#endif /* WLFCTS */
			} else if (wlhdrtodev[processed] ==
				WLFC_CTL_TYPE_PENDING_TRAFFIC_BMP) {
				wlc_scb_update_available_traffic_info(wl->wlc,
					wlhdrtodev[processed+2], wlhdrtodev[processed+3]);
			}

			if (wlhdrtodev[processed] == WLFC_CTL_TYPE_FILLER) {
				/* skip ahead - 1 */
				processed += 1;
			} else {
				/* skip ahead - type[1], len[1], value_len */
				processed += TLV_HDR_LEN + wlhdrtodev[processed + TLV_LEN_OFF];
			}
		}
#ifdef BCM_DHDHDR
		if (BCMDHDHDR_ENAB()) {
			bzero(PKTFRAGFCTLV(wl->pub->osh, p), wlhdrlen);
		} else
#endif /* BCM_DHDHDR */
		{
			PKTPULL(wl->pub->osh, p, wlhdrlen);
		}
		/* Reset DataOffset to 0, since we have consumed the wlhdr */
		PKTSETDATAOFFSET(p, 0);
	} else {
		/* wlhdrlen == 0 */
		if (BCMPCIEDEV_ENAB()) {
#ifdef WLFCTS
			if (WLFCTS_ENAB(wl->pub)) {
				/* Send a timestamp back to host only if enabled */
				wlc_info_t *wlc = (wlc_info_t *)wl->wlc;
				if (wlc->wlfc_flags & WLFC_FLAGS_PKT_STAMP_SIGNALS) {
					if (si_iscoreup(wlc->pub->sih)) {
						tx_entry_tstamp =
							R_REG(wlc->osh, &wlc->regs->tsf_timerlow);
					}
				}
			}
#endif	/* WLFCTS */
		} else {
			WL_INFORM(("No pkttag from host.\n"));
		}
	}

	/* update pkttag */
	wlpkttag = WLPKTTAG(p);
	wlpkttag->wl_hdr_information = wl_hdr_information;
	wlpkttag->seq = seq;
#ifdef WLFCTS
	wlpkttag->shared.tx_entry_tstamp = tx_entry_tstamp;
#endif // endif

	if (wl->wlfc_info != NULL) {
		((wlfc_info_state_t*)wl->wlfc_info)->stats.packets_from_host++;
	}

	if (PKTLEN(wl->pub->osh, p) == 0) {
		/* a signal-only packet from host */
#ifdef PROP_TXSTATUS_DEBUG
		((wlfc_info_state_t*)wl->wlfc_info)->dbgstats->sig_from_host++;
#endif // endif
		PKTFREE(wl->pub->osh, p, TRUE);
		return TRUE;
	}

#ifdef PROP_TXSTATUS_DEBUG
	if ((WL_TXSTATUS_GET_FLAGS(wlpkttag->wl_hdr_information) & WLFC_PKTFLAG_PKTFROMHOST) &&
	    (!(WL_TXSTATUS_GET_FLAGS(wlpkttag->wl_hdr_information) & WLFC_PKTFLAG_PKT_REQUESTED))) {
		((wlfc_info_state_t*)wl->wlfc_info)->dbgstats->creditin++;
	} else {
		((wlfc_info_state_t*)wl->wlfc_info)->dbgstats->nost_from_host++;
	}
	WLF2_PCB3_REG(p, WLF2_PCB3_WLFC);
#endif /* PROP_TXSTATUS_DEBUG */
	return FALSE;
} /* wl_send_txstatus */
#endif /* PROP_TXSTATUS */

#ifdef PKTC_TX_DONGLE
/** PKTC_TX_DONGLE specific function */
static bool
wlconfig_tx_chainable(wl_info_t *wl, wlc_bsscfg_t *bsscfg)
{
	wlc_info_t *wlc = (wlc_info_t *)wl->wlc;

	/* For now don't do chaining for following configs. */
	if ((WLEXTSTA_ENAB(wl->pub) && BSSCFG_SAFEMODE(bsscfg)) ||
	    (wlc->wet && BSSCFG_STA(bsscfg)) ||
	    wlc->mac_spoof ||
#ifdef WLWNM_AP
	    (BSSCFG_AP(bsscfg) && WLWNM_ENAB(wlc->pub) && !wl_wnm_pkt_chainable(bsscfg)) ||
#endif /* WLWNM_AP */
	    CAC_ENAB(wl->pub) ||
	    BSSCFG_AWDL(wlc, bsscfg)) {
		return FALSE;
	}
	return TRUE;
}

static bool
wl_txframe_chainable(wl_info_t *wl, wlc_bsscfg_t *bsscfg, void *p, void *head)
{
	wlc_info_t *wlc = (wlc_info_t *)wl->wlc;
	bool chainable = FALSE;
	struct ether_header *eh, *head_eh;
	void *iph;

	eh = (struct ether_header *) PKTDATA(wlc->osh, p);
	iph = (void *)(eh + 1);

	if (BCMLFRAG_ENAB() && PKTISTXFRAG(wlc->osh, p)) {
		/* For LFRAG packets, we have only the ethernet header. IP header + Payload */
		/* is sitting in the host. So, don't bother to look into the IP Prot field */
		if ((ntoh16(eh->ether_type) == ETHER_TYPE_IP) ||
			(ntoh16(eh->ether_type) == ETHER_TYPE_IPV6))
			chainable = TRUE;
	} else if (ntoh16(eh->ether_type) == ETHER_TYPE_IP) {
		ASSERT(IP_VER(iph) == IP_VER_4);

		if (IPV4_PROT(iph) == IP_PROT_TCP ||
		    IPV4_PROT(iph) == IP_PROT_UDP) {
			chainable = TRUE;
		}
	} else if (ntoh16(eh->ether_type) == ETHER_TYPE_IPV6) {
		ASSERT(IP_VER(iph) == IP_VER_6);

		if (IPV6_PROT(iph) == IP_PROT_TCP ||
		    IPV6_PROT(iph) == IP_PROT_UDP) {
			chainable = TRUE;
		}
	}

	if (!chainable)
		goto exit;

	chainable = !ETHER_ISNULLDEST(eh->ether_dhost) &&
			!ETHER_ISMULTI(eh->ether_dhost);

	/* For PCIe Dev,
	 * In AP mode, all the packets in chain would have the same DA and PRIO
	 * In STA mode, all the packets in chain have same PRIO and would be transmitted to AP
	 * We don't need DA comparison.
	 */
	if (!BCMPCIEDEV_ENAB() && (head != NULL) && chainable) {
		head_eh = (struct ether_header *) PKTDATA(wlc->osh, head);
		chainable = !eacmp(eh->ether_dhost, head_eh->ether_dhost) &&
			(PKTPRIO(p) == PKTPRIO(head));
	}

exit:
	return chainable;
} /* wl_txframe_chainable */
#endif /* PKTC_TX_DONGLE */

#ifdef BCMPCIEDEV
/**
 * For e.g. 802.1x packets, it is necessary to transfer the full packet from host memory into CPU
 * RAM, so firmware can parse packet contents before transmission.
 */
static void
wl_tx_pktfetch(wl_info_t *wl, struct lbuf *lb, hnd_dev_t *src, hnd_dev_t *dev)
{
	struct pktfetch_info *pinfo = NULL;
	struct pktfetch_generic_ctx *pctx = NULL;
	int ctx_count = 4;	/* No. of ctx variables needed to be saved */

	pinfo = MALLOCZ(wl->pub->osh, sizeof(struct pktfetch_info));
	if (!pinfo) {
		WL_ERROR(("%s: Out of mem: Unable to alloc pktfetch ctx!\n", __FUNCTION__));
		goto error;
	}

	pctx = MALLOCZ(wl->pub->osh, sizeof(struct pktfetch_generic_ctx) +
		ctx_count*sizeof(void*));
	if (!pctx) {
		WL_ERROR(("%s: Out of mem: Unable to alloc pktfetch ctx!\n", __FUNCTION__));
		goto error;
	}

	/* Fill up context */
	pctx->ctx_count = ctx_count;
	pctx->ctx[0] = (void *)wl;
	pctx->ctx[1] = (void *)src;
	pctx->ctx[2] = (void *)dev;
	pctx->ctx[3] = (void *)pinfo;

	/* Fill up pktfetch info */
#ifdef BCM_DHDHDR
	if (BCMDHDHDR_ENAB()) {
		pinfo->host_offset = (-DOT11_LLC_SNAP_HDR_LEN);
	} else
#endif /* BCM_DHDHDR */
	{
		pinfo->host_offset = 0;
	}

	/* Need headroom of atleast 224 for TXOFF/amsdu headroom
	 * Rounded to 256
	 */
	pinfo->headroom = PKTFETCH_DEFAULT_HEADROOM;
	pinfo->lfrag = (void*)lb;
	pinfo->cb = wl_send_cb;
	pinfo->ctx = (void *)pctx;
	pinfo->next = NULL;
	pinfo->osh = wl->pub->osh;
	if (hnd_pktfetch(pinfo) != BCME_OK) {
		WL_ERROR(("%s: pktfetch request rejected\n", __FUNCTION__));
		goto error;
	}

	return;

error:
	if (pinfo)
		MFREE(wl->pub->osh, pinfo, sizeof(struct pktfetch_info));

	if (pctx)
		MFREE(wl->pub->osh, pctx, sizeof(struct pktfetch_generic_ctx) + 4*sizeof(uint32));

	if (lb)
		PKTFREE(wl->pub->osh, lb, TRUE);
}

/** Packet fetch callback. BCMPCIEDEV specific */
static void
wl_send_cb(void *lbuf, void *orig_lfrag, void *ctx, bool cancelled)
{
	wl_info_t *wl;
	struct pktfetch_info *pinfo;
	hnd_dev_t *src, *dev;
	struct pktfetch_generic_ctx *pctx = (struct pktfetch_generic_ctx *)ctx;

	/* Retrieve contexts */
	wl = (wl_info_t *)pctx->ctx[0];
	src = (hnd_dev_t *)pctx->ctx[1];
	dev = (hnd_dev_t *)pctx->ctx[2];
	pinfo = (struct pktfetch_info *)pctx->ctx[3];

	PKTSETNEXT(wl->pub->osh, orig_lfrag, lbuf);
	PKTSETFRAGTOTLEN(wl->pub->osh, orig_lfrag, 0);
	PKTSETFRAGLEN(wl->pub->osh, orig_lfrag, 1, 0);

	/* When BCM_DHDHDR is enabled, all tx packets that need to be fetched will
	 * include llc snap 8B header at start of lbuf. Then these packets go
	 * through original TX path (non-DHDHDR path).  So we still need to clear the
	 * frag total number by PKTSETFRAGTOTNUM.
	 */
	PKTSETFRAGTOTNUM(wl->pub->osh, orig_lfrag, 0);

	/* Free the original pktfetch_info and generic ctx  */
	MFREE(wl->pub->osh, pinfo, sizeof(struct pktfetch_info));
	MFREE(wl->pub->osh, pctx, sizeof(struct pktfetch_generic_ctx)
		+ pctx->ctx_count*sizeof(void *));

	/* The hnd_pktfetch_dispatch may get lbuf from PKTALLOC and the pktalloced counter
	 * will be increased by 1, later in the wl_send the PKTFROMNATIVE will increase 1 again
	 * for !lb_pool lbuf. (dobule increment)
	 * Here do PKTTONATIVE to decrease it before wl_send.
	 */
	if (!PKTPOOL(wl->pub->osh, lbuf)) {
		PKTTONATIVE(wl->pub->osh, lbuf);
	}
#ifdef DONGLEBUILD
	/* Interface (dev) might be fetch request and complete,
	 * Validate it before forwarding packet.
	 */
	if (bus_ops->validatedev((void *)wl->dev->chained, dev) == BCME_OK) {
		wl_send(src, dev, orig_lfrag);
	} else {
		/* Interface doesn't exist, discard packet */
		void *p;

		p = PKTFRMNATIVE(wl->pub->osh, orig_lfrag);
		PKTFREE(wl->pub->osh, p, TRUE);
	}
#else
	wl_send(src, dev, orig_lfrag);
#endif /* DONGLEBUILD */
}

/**
 * Per packet key check for SW TKIP MIC requirement, code largely borrowed from wlc_sendpkt.
 * BCMPCIEDEV specific.
 */
static bool
wl_sw_tkip_mic_enab(wl_info_t *wl, struct wlc_if *wlcif, wlc_bsscfg_t *bsscfg, struct lbuf *lb)
{
	struct scb *scb = NULL;
	struct ether_header *eh;
	struct ether_addr *dst;
#ifdef WDS
	struct ether_addr *wds = NULL;
#endif // endif
	wlc_key_info_t key_info;
	wlc_key_t *key = NULL;
	uint bandunit;
	bool tkip_enab = FALSE;
	wlc_info_t *wlc = (wlc_info_t *)wl->wlc;

	/* WLDPT, WLTDLS, IAPP, WLAWDL cases currently not handled */

	/* Get dest. */
	eh = (struct ether_header*) PKTDATA(wl->pub->osh, lb);

#ifdef WDS
	if (wlcif && wlcif->type == WLC_IFTYPE_WDS) {
		scb = wlcif->u.scb;
		wds = &scb->ea;
	}

	if (wds)
		dst = wds;
	else
#endif /* WDS */
	if (BSSCFG_AP(bsscfg)) {
#ifdef WLWNM_AP
		/* Do the WNM processing */
		if (WLWNM_ENAB(wlc->pub) && wlc_wnm_dms_amsdu_on(wlc, bsscfg) &&
		    WLPKTTAGSCBGET(lb) != NULL) {
			dst = &(WLPKTTAGSCBGET(lb)->ea);
		} else
#endif /* WLWNM_AP */
		dst = (struct ether_addr*)eh->ether_dhost;
	} else {
		dst = bsscfg->BSS ? &bsscfg->BSSID : (struct ether_addr*)eh->ether_dhost;
	}

	/* Get key */
	bandunit = CHSPEC_WLCBANDUNIT(bsscfg->current_bss->chanspec);

	/* Class 3 (BSS) frame */
	if (TRUE &&
#ifdef WDS
		!wds &&
#endif // endif
		bsscfg->BSS && !ETHER_ISMULTI(dst)) {
		scb = wlc_scbfindband(wlc, bsscfg, dst, bandunit);
	}
	/* Class 1 (IBSS/DPT) or 4 (WDS) frame */
	else {
		if (ETHER_ISMULTI(dst))
			scb = WLC_BCMCSCB_GET(wlc, bsscfg);
		else
			scb = wlc_scblookupband(wlc, bsscfg, dst, bandunit);
	}

	key = wlc_keymgmt_get_tx_key(wlc->keymgmt, scb, bsscfg, &key_info);

	if (scb && (key_info.algo == CRYPTO_ALGO_OFF)) {
		WL_INFORM(("wl%d: %s: key_info algo is off, use bss tx key instead\n",
			wl->unit, __FUNCTION__));
		key = wlc_keymgmt_get_bss_tx_key(wlc->keymgmt, bsscfg, FALSE, &key_info);
	}

	if (key == NULL)
		WL_ERROR(("wl%d: %s: key is NULL!\n", wl->unit, __FUNCTION__));

	/* If security algo is TKIP and MIC key is in HW, or PMF */
	if (((key_info.algo == CRYPTO_ALGO_TKIP) && (ETHER_ISMULTI(dst) ||
		!(WLC_KEY_MIC_IN_HW(&key_info)) ||
		(scb && wlc_is_packet_fragmented(wlc, scb, bsscfg, (void *)lb)))) ||
		WLC_KEY_SW_ONLY(&key_info)) {
		tkip_enab = TRUE;
	}
	return tkip_enab;
}

/** BCMPCIEDEV specific */
static bool
wl_tx_pktfetch_required(wl_info_t *wl, wl_if_t *wlif, wlc_bsscfg_t *bsscfg, struct lbuf *lb)
{
	struct wlc_if *wlcif = wlif != NULL ? wlif->wlcif : NULL;

	if (WSEC_ENABLED(bsscfg->wsec) && (WLPKTFLAG_PMF(WLPKTTAG(lb)) ||
	   (WSEC_TKIP_ENABLED(bsscfg->wsec) && wl_sw_tkip_mic_enab(wl, wlcif, bsscfg, lb))))
		return TRUE;

	if (ARPOE_ENAB(wl->pub)) {
		wl_arp_info_t *arpi = wl_get_arpi(wl, wlif);
		if (arpi) {
			if (wl_arp_send_pktfetch_required(arpi, lb))
				return TRUE;
		}
	}

	if (ntoh16_ua((const void *)(PKTDATA(wl->pub->osh, lb) + ETHER_TYPE_OFFSET))
		== ETHER_TYPE_802_1X) {
		return TRUE;
	}

	return FALSE;
}
#endif	/* BCMPCIEDEV */

/** Called by the RTOS when a transmit packet is to be handed over to the dot11 stack */
static int
wl_send(hnd_dev_t *src, hnd_dev_t *dev, struct lbuf *lb)
{
	wl_info_t *wl = dev->softc;
	wl_if_t *wlif = WL_IF(wl, dev);
	struct wlc_if *wlcif = wlif != NULL ? wlif->wlcif : NULL;
	void *p;
	struct lbuf *n;
	bool discarded = FALSE;
#ifdef BCMPCIEDEV
	bool pktfetch = FALSE;
#endif // endif
#ifdef PKTC_TX_DONGLE
	bool chainable = FALSE;
	bool cfg_chainable = FALSE;
	void *head = NULL, *tail = NULL;
#endif // endif
#if defined(BCMPCIEDEV) || defined(PKTC_TX_DONGLE)
	wlc_bsscfg_t *bsscfg;

	bsscfg = wlc_bsscfg_find_by_wlcif(wl->wlc, wlcif);
	ASSERT(bsscfg != NULL);
#endif // endif

#ifdef BCMPCIEDEV
	if (BCMPCIEDEV_ENAB() && PKTISTXFRAG(wl->pub->osh, lb) &&
	    (PKTFRAGTOTLEN(wl->pub->osh, lb) > 0))
		pktfetch = TRUE;
#endif // endif

#ifdef PKTC_TX_DONGLE
	cfg_chainable = wlconfig_tx_chainable(wl, bsscfg);
#endif // endif

	while (lb != NULL) {
		n = PKTLINK(lb);
		PKTSETLINK(lb, NULL);
		p = PKTFRMNATIVE(wl->pub->osh, lb);

#ifdef PROP_TXSTATUS
		if (PROP_TXSTATUS_ENAB(((wlc_info_t *)(wl->wlc))->pub)) {
			/*
			 * proptxstatus header should be processed only once when
			 * pktfetch is true
			 */
			if (TRUE &&
#ifdef BCMPCIEDEV
			    pktfetch &&
#endif // endif
			    wl_send_txstatus(wl, p)) {
				goto nextlb;
			}
		} else
#endif /* PROP_TXSTATUS */
		{
			/* Pull wl header. Currently no information is included on transmit side */
			wl_pkt_header_pull(wl, p);
		}

#ifdef BCMPCIEDEV
		/* For TKIP, MAC layer fragmentation does not work with current split-tx
		 * approach. We need to pull down the remaining payload and recreate the
		 * original 802.3 packet. Packet chaining, if any, is broken here anyway
		 * Need to check wl_sw_tkip_mic_enab call for all pkts in chain at this point
		 */
		if (pktfetch && wl_tx_pktfetch_required(wl, wlif, bsscfg, lb)) {
			wl_tx_pktfetch(wl, lb, src, dev); /* queues a host fetch request */
			goto nextlb;
		}
#endif // endif
		WL_TRACE(("wl%d: wl_send: len %d\n", wl->unit, PKTLEN(wl->pub->osh, p)));

		/* Apply ARP offload */
		if (ARPOE_ENAB(wl->pub)) {
			wl_arp_info_t *arpi = wl_get_arpi(wl, wlif);
			if (arpi) {
				if (wl_arp_send_proc(arpi, p) ==
					ARP_REPLY_HOST) {
					PKTFREE(wl->pub->osh, p, TRUE);
					goto nextlb;
				}
			}
		}
#ifdef WLNDOE
		/* Apply NS offload */
		if (NDOE_ENAB(wl->pub)) {
			wl_nd_info_t *ndi = wl_get_ndi(wl, wlif);
			if (ndi) {
				wl_nd_send_proc(ndi, p);
			}
		}
#endif // endif

#ifdef WLNFC
		/* Apply NFC offload */
		if (NFC_ENAB(wl->pub)) {
			wl_nfc_info_t *nfci = wl_get_nfci(wl, wlif);
			wl_nfc_send_proc(nfci, p);
		}
#endif // endif

#ifdef TOE
		/* check TOE */
		_wl_toe_send_proc(wl, p);
#endif // endif

#ifdef PKTC_TX_DONGLE
		if (PKTC_ENAB(wl->pub)) {
			chainable = cfg_chainable && wl_txframe_chainable(wl, bsscfg, p, head);

			if (chainable) {
				if (n) {
					PKTSETCHAINED(wl->pub->osh, p);
				}
				PKTCENQTAIL(head, tail, p);
			}

			if (head != NULL &&
			    (n == NULL || !chainable)) {
				if (wlc_sendpkt(wl->wlc, head, wlcif))
					discarded = TRUE;
				head = tail = NULL;
			}
		}

		if (!chainable)
#endif /* PKTC_TX_DONGLE */
		{

			if (wlc_sendpkt(wl->wlc, p, wlcif))
				discarded = TRUE;
		}

nextlb:
		lb = n;
	}

#ifdef PKTC_TX_DONGLE
	/* if last pkt was header only, send remaining chain */
	if (PKTC_ENAB(wl->pub) && (head != NULL)) {
		if (wlc_sendpkt(wl->wlc, head, wlcif))
			discarded = TRUE;
	}
#endif // endif

	return discarded;
} /* wl_send */
#else
static int
wl_send(hnd_dev_t *src, hnd_dev_t *dev, struct lbuf *lb)
{
	wl_info_t *wl = dev->softc;

	WL_TRACE(("wl%d: wl_send: len %d\n", wl->unit, lb->len));

	bcm_rpc_tp_rx_from_dnglbus(wl->rpc_th, lb);

	return FALSE;
}
#endif /* WLC_HIGH */

#if defined(BCM_OL_DEV) && defined(WLNDOE)
void * wl_get_ndi(wl_info_t *wl, struct wl_if *wlif)
{
	return wl->ndi;
}
#endif /* defined (BCM_OL_DEV) && defined (WLNDOE) */

int wl_busioctl(wl_info_t *wl, uint32 cmd, void *buf, int len, int *used, int *needed, int set)
{
	hnd_dev_t *chained = wl->dev->chained;
	if (chained && chained->ops->ioctl)
		return chained->ops->ioctl(chained, cmd, buf, len, used, needed, set);
	else
		return BCME_ERROR;
}

#ifdef WLC_HIGH

void
wl_txflowcontrol(wl_info_t *wl, struct wl_if *wlif, bool state, int prio)
{
	hnd_dev_t *chained = wl->dev->chained;

	/* sta mode must be chained */
	if (chained && chained->ops->txflowcontrol)
		chained->ops->txflowcontrol(chained, state, prio);
	else
		ASSERT(AP_ENAB(wl->pub));
}

void
wl_event(wl_info_t *wl, char *ifname, wlc_event_t *e)
{
#ifdef WLPFN
	/* Tunnel events into PFN for analysis */
	if (WLPFN_ENAB(wl->pub))
		wl_pfn_event(wl->pfn, e);
#endif /* WLPFN */

	switch (e->event.event_type) {
	case WLC_E_LINK:
	case WLC_E_NDIS_LINK:
		wl->link = e->event.flags&WLC_EVENT_MSG_LINK;
		if (wl->link) {
			WL_ERROR(("wl%d: link up (%s)\n", wl->unit, ifname));
#ifdef WLNDOE_RA
			/* clear Router Advertisement Filter cache on every
			 * assoc event
			 */
			wl_nd_ra_filter_clear_cache(wl->ndi);
#endif /* WLNDOE_RA */
		}
/* Getting too many */
#ifndef EXT_STA
		else
			WL_ERROR(("wl%d: link down (%s)\n", wl->unit, ifname));
#endif /* EXT_STA */
		break;
#if ((defined(STA) && defined(BCMSUP_PSK)) || (defined(AP) && defined(BCMAUTH_PSK)))
	case WLC_E_MIC_ERROR:
		{
			int err = BCME_OK;
			wlc_bsscfg_t *cfg = wlc_bsscfg_find(wl->wlc, e->event.bsscfgidx, &err);
			if (cfg != NULL) {
#if defined(STA) && defined(BCMSUP_PSK)
				if (SUP_ENAB(wl->pub) && BSSCFG_STA(cfg))
					wlc_sup_mic_error(cfg,
						(e->event.flags&WLC_EVENT_MSG_GROUP) ==
						WLC_EVENT_MSG_GROUP);
#endif /* STA && BCMSUP_PSK */
#if defined(AP) && defined(BCMAUTH_PSK)
				if (BCMAUTH_PSK_ENAB(wl->pub) && BSSCFG_AP(cfg))
					wlc_auth_tkip_micerr_handle(wl->wlc, cfg);
#endif /* AP && BCMAUTH_PSK */
			}
		}
		break;
#endif /* (STA && BCMSUP_PSK) || (AP && BCMAUTH_PSK) */
	}
} /* wl_event */
#endif /* WLC_HIGH */

void
wl_event_sync(wl_info_t *wl, char *ifname, wlc_event_t *e)
{
#ifdef WL_EVENTQ
	/* duplicate event for local event q */
	wl_eventq_dup_event(wl->wlevtq, e);
#endif /* WL_EVENTQ */
}

void
wl_event_sendup(wl_info_t *wl, const wlc_event_t *e, uint8 *data, uint32 len)
{
}

#ifndef WLC_LOW_ONLY
#ifdef WLOTA_EN
static int
wlc_iovar_wlota_filter(wlc_info_t * wlc, char * name, uint32 cmd)
{

	int bcmerror = BCME_UNSUPPORTED;
	char allowed[5][25] = {
			"ota_trigger",
			"ota_loadtest",
			"ota_teststatus",
			"ota_teststop",
			"bcmerrorstr"
			};
	uint8 i;
	if (wlc->iov_block == NULL)
		return BCME_OK;

	if (*wlc->iov_block != WL_OTA_TEST_ACTIVE)
		return BCME_OK;

	if (cmd <= 1)
		return BCME_OK;

	for (i = 0; i < 5; i++) {
		if (strcmp(name, allowed[i]) == 0) {
			bcmerror = BCME_OK;
			break;
		}
	}
	return bcmerror;
}
#endif /* WLOTA */

static int
wl_ioctl(hnd_dev_t *dev, uint32 cmd, void *buf, int len, int *used, int *needed, int set)
{
	wl_info_t *wl = dev->softc;
	wl_if_t *wlif = WL_IF(wl, dev);
	struct wlc_if *wlcif = wlif != NULL ? wlif->wlcif : NULL;
	wlc_bsscfg_t *cfg = NULL;
	int ret = 0;
	int origcmd = cmd;
	int status = 0;
	uint32 *ret_int_ptr = (uint32 *)buf;

	WL_TRACE(("wl%d: wl_ioctl: cmd 0x%x\n", wl->unit, cmd));

	cfg = wlc_bsscfg_find_by_wlcif(wl->wlc, wlcif);
	ASSERT(cfg != NULL);
	switch (cmd) {
	case RTEGHWADDR:
		ret = wlc_iovar_op(wl->wlc, "cur_etheraddr", NULL, 0, buf, len, IOV_GET, wlcif);
		break;
	case RTESHWADDR:
		ret = wlc_iovar_op(wl->wlc, "cur_etheraddr", NULL, 0, buf, len, IOV_SET, wlcif);
		break;
	case RTEGPERMADDR:
		ret = wlc_iovar_op(wl->wlc, "perm_etheraddr", NULL, 0, buf, len, IOV_GET, wlcif);
		break;
	case RTEGMTU:
		*ret_int_ptr = ETHER_MAX_DATA;
		break;
#ifdef WLC_HIGH
	case RTEGSTATS:
		wl_statsupd(wl);
		bcopy(&wl->stats, buf, MIN(len, sizeof(wl->stats)));
		break;

	case RTEGALLMULTI:
		*ret_int_ptr = cfg->allmulti;
		break;
	case RTESALLMULTI:
		cfg->allmulti = *((uint32 *) buf);
		break;
#endif /* WLC_HIGH */
	case RTEGPROMISC:
		cmd = WLC_GET_PROMISC;
		break;
	case RTESPROMISC:
		cmd = WLC_SET_PROMISC;
		break;
#ifdef WLC_HIGH
	case RTESMULTILIST: {
		int i;

		/* copy the list of multicasts into our private table */
		cfg->nmulticast = len / ETHER_ADDR_LEN;
		for (i = 0; i < cfg->nmulticast; i++)
			cfg->multicast[i] = ((struct ether_addr *)buf)[i];
		break;
	}
#endif /* WLC_HIGH */
	case RTEGUP:
		cmd = WLC_GET_UP;
		break;
	case RTEDEVPWRSTCHG:
#if defined(WLC_HIGH) && defined(WLC_LOW)
	if (*ret_int_ptr == 0)
		wl_devpwrstchg_notify(wl, FALSE);
	else
		wl_devpwrstchg_notify(wl, TRUE);
#endif // endif
		break;
	case RTEDEVPMETOGGLE:
		wl_generate_pme_to_host(wl, *(uint32*)buf);
		break;
	default:
		/* force call to wlc ioctl handler */
		origcmd = -1;
		break;
	}

#ifdef WLOTA_EN
	if ((ret = wlc_iovar_wlota_filter(wl->wlc, buf, cmd)) != BCME_OK) {
		return ret;
	}
#endif /* WLOTA */

	if (cmd != origcmd) {
		ret = wlc_ioctl(wl->wlc, cmd, buf, len, wlcif);
	}

	if (status)
		return status;

	return (ret);
}
#else

#ifdef BCM_OL_DEV

void
wl_watchdog(wl_info_t *wl)
{
	wlc_info_t *wlc = wl->wlc;

	wlc->pub->now++;

	wlc_dngl_ol_watchdog(wlc->wlc_dngl_ol);
	wlc_scanol_watchdog(wl->wlc_hw);
	wlc_macol_watchdog(wl->wlc_hw);
}

/* Message to HOST */
void wl_msgup(wl_info_t *wl, osl_t *osh, void* resp)
{

	hnd_dev_t *dev;
	hnd_dev_t *chained;
	struct lbuf *lb;

	dev = wl->dev;
	chained = dev->chained;

	lb = PKTTONATIVE(osh, resp);

	if (chained->ops->xmit(dev, chained, lb) != 0) {
		WL_ERROR(("%s: xmit failed; free pkt 0x%p\n", __FUNCTION__, lb));
		lb_free(lb);
	}

}

void
wl_sendup(wl_info_t *wl, struct wl_if *wlif, void *p, int numpkt)
{

	int ret_val = 0;
#ifdef BCM_OL_DEV
	wlc_info_t *wlc = (wlc_info_t *)wl->wlc;
#endif /* BCM_OL_DEV */
#ifdef ARPOE
	bool suppressed = FALSE;
	wl_arp_info_t *arpi = wl_get_arpi(wl, wlif);
#endif // endif
#ifdef TCPKAOE
	wl_icmp_info_t *icmpi = wl_get_icmpi(wl, wlif);
	wl_tcp_keep_info_t *tcp_keep_info = wl_get_tcpkeepi(wl, wlif);
#endif // endif
#ifdef WLNDOE
	wl_nd_info_t *ndi = wl_get_ndi(wl, wlif);
#endif // endif

#ifdef BCM_OL_DEV
	BCM_REFERENCE(wlc);
#endif /* BCM_OL_DEV */

#ifdef ARPOE
	if (arpi) {
		ret_val = wl_arp_recv_proc(arpi, p);
		if (ret_val >= 0) {
			if ((ret_val == ARP_REQ_SINK) || (ret_val == ARP_REPLY_PEER)) {
				suppressed = TRUE;
				if (wlc_dngl_ol_supr_frame(wlc, WLPKTTAG(p)->frameptr)) {
					RXOEINC(wlc->wlc_dngl_ol, rxoe_arpsupresscnt);
				}
			} else {
				wlc_dngl_ol_push_to_host(wl->wlc);
			}

			wl_arp_update_stats(arpi, suppressed);
			return;
		}
	}
#endif /* ARPOE */

#if defined(WLNDOE) && !defined(WLNDOE_DISABLED)
	if (ndi) {
		ret_val = wl_nd_recv_proc(ndi, p);
		if (ret_val >= 0) {
			if ((ret_val == ND_REQ_SINK) || (ret_val == ND_REPLY_PEER)) {
				suppressed = TRUE;
				if (wlc_dngl_ol_supr_frame(wlc, WLPKTTAG(p)->frameptr)) {
					RXOEINC(wlc->wlc_dngl_ol, rxoe_nssupresscnt);
				}
			} else {
				wlc_dngl_ol_push_to_host(wl->wlc);
			}

			wl_nd_update_stats(ndi, suppressed);
			return;
		}
	}
#endif /* WLNDOE */

#ifdef TCPKAOE

	if (icmpi) {
		ret_val = wl_icmp_recv_proc(icmpi, p);
		if (ret_val >= 0) {
			return;
		}
	}

	if (tcp_keep_info) {
		ret_val = wl_tcpkeep_recv_proc(tcp_keep_info, p);
		if (ret_val >= 0) {
			return;
		}
	}
#endif	/* TCPKAOE */

#ifdef PACKET_FILTER
	ret_val = wlc_pkt_filter_ol_process(wlc, p);

	if (ret_val >= 0) {
		if (wlc_dngl_ol_supr_frame(wlc, WLPKTTAG(p)->frameptr)) {
			RXOEINC(wlc->wlc_dngl_ol, rxoe_pkt_filter_supresscnt);
		} else {
			wlc_dngl_ol_push_to_host(wl->wlc);
		}

		return;
		}
#endif	/* PACKET_FILTER */

	wlc_dngl_ol_push_to_host(wl->wlc);
	return;
}

/* We may register different msg recepients with callbacks */

static int
wl_handle_msg(hnd_dev_t *dev, void *buf, int len)
{
	wl_info_t *wl = dev->softc;
	wlc_info_t *wlc = wl->wlc;

	olmsg_header *msg_hdr = buf;

	BCM_REFERENCE(wl);
	BCM_REFERENCE(msg_hdr);

	WL_INFORM(("wl%d: wl_handle_msg for msg type %d\n", wl->unit, msg_hdr->type));
#ifdef BCM_OL_DEV
	wlc_dngl_ol_process_msg(wlc->wlc_dngl_ol, buf, len);
#endif // endif
	return 0;
}

static int
wl_ioctl(hnd_dev_t *dev, uint32 cmd, void *buf, int len, int *used, int *needed, int set)
{
	wl_info_t *wl = dev->softc;
	int status = 0;

	BCM_REFERENCE(wl);

	WL_INFORM(("wl%d: wl_ioctl: cmd 0x%x\n", wl->unit, cmd));

	switch (cmd) {
	case 0:	/* PCI msg */
		wl_handle_msg(dev, buf, len);
		break;
	default:
		WL_ERROR(("wl%d: wl_ioctl: cmd 0x%x\n", wl->unit, cmd));
		break;
	}

	return status;
}
#endif /* BCM_OL_DEV */

#endif /* WLC_LOW_ONLY */

static int
BCMUNINITFN(wl_close)(hnd_dev_t *dev)
{
	wl_info_t *wl = dev->softc;
	BCM_REFERENCE(wl);
	uint8 objreg_freed = 0;

	WL_TRACE(("wl%d: wl_close\n", wl->unit));

#ifdef WL_OBJ_REGISTRY
	if (wl->objr && dev->devid) {
		objreg_freed = obj_registry_islast(wl->objr);
	}
#endif // endif

#ifdef WLC_HIGH
	/* BMAC_NOTE: ? */
	wl_down(wl);
#endif // endif

	if (objreg_freed)
		MFREE(NULL, (dev->commondata), sizeof(wl_cmn_data_t));

	return 0;
}

#ifdef WLC_HIGH
static void
wl_statsupd(wl_info_t *wl)
{
	hnd_dev_stats_t *stats;

	WL_TRACE(("wl%d: wl_get_stats\n", wl->unit));

	stats = &wl->stats;

	/* refresh stats */
	if (wl->pub->up)
		wlc_statsupd(wl->wlc);

	stats->rx_packets = WLCNTVAL(wl->pub->_cnt->rxframe);
	stats->tx_packets = WLCNTVAL(wl->pub->_cnt->txframe);
	stats->rx_bytes = WLCNTVAL(wl->pub->_cnt->rxbyte);
	stats->tx_bytes = WLCNTVAL(wl->pub->_cnt->txbyte);
	stats->rx_errors = WLCNTVAL(wl->pub->_cnt->rxerror);
	stats->tx_errors = WLCNTVAL(wl->pub->_cnt->txerror);
	stats->rx_dropped = 0;
	stats->tx_dropped = 0;
	stats->multicast = WLCNTVAL(wl->pub->_cnt->rxmulti);
}
#endif /* WLC_HIGH */

void
BCMATTACHFN(wl_isucodereclaimed)(uint8 *value)
{
#ifdef DONGLEBUILD
	*value = (uint8)preattach_part_reclaimed;
#endif /* DONGLEBUILD */
}

void
BCMATTACHFN(wl_reclaim)(void)
{
#ifdef DONGLEBUILD
	bool postattach_part_reclaimed_tmp = postattach_part_reclaimed;
	bool ucodes_reclaimed_tmp = ucodes_reclaimed;
#ifdef BCMRECLAIM
	bcmreclaimed = TRUE;
#endif /* BCMRECLAIM */
	attach_part_reclaimed = TRUE;
	postattach_part_reclaimed = TRUE;
	ucodes_reclaimed = TRUE;
	hnd_reclaim();
	postattach_part_reclaimed = postattach_part_reclaimed_tmp;
	ucodes_reclaimed = ucodes_reclaimed_tmp;
#endif /* DONGLEBUILD */
}

/* postattach is assumed to be called after wlc_bmac_init */
void
wl_reclaim_postattach(void)
{
#ifdef DONGLEBUILD
	bool ucodes_reclaimed_tmp = ucodes_reclaimed;
#ifdef BCMRECLAIM
	bool bcmreclaimed_tmp = bcmreclaimed;
	bcmreclaimed = TRUE;
#endif /* BCMRECLAIM */
	ucodes_reclaimed = TRUE;
	hnd_reclaim();
#ifdef BCMRECLAIM
	bcmreclaimed = bcmreclaimed_tmp;
#endif /* BCMRECLAIM */
	ucodes_reclaimed = ucodes_reclaimed_tmp;
#endif /* DONGLEBUILD */
}

void
wl_reclaim_ucode(void)
{
#ifdef DONGLEBUILD
	bool postattach_part_reclaimed_tmp = postattach_part_reclaimed;
#ifdef BCMRECLAIM
	bool bcmreclaimed_tmp = bcmreclaimed;
	bcmreclaimed = TRUE;
#endif /* BCMRECLAIM */
	postattach_part_reclaimed = TRUE;
	hnd_reclaim();
	postattach_part_reclaimed = postattach_part_reclaimed_tmp;
#ifdef BCMRECLAIM
	bcmreclaimed = bcmreclaimed_tmp;
#endif // endif
#endif /* DONGLEBUILD */
}

#if defined(ATE_BUILD)
int
wl_get(void *wlc, int cmd, void *buf, int len)
{
	return wlc_ioctl(wlc, cmd, buf, len, NULL);
}

int
wl_set(void *wlc, int cmd, void *buf, int len)
{
	return wlc_ioctl(wlc, cmd, buf, len, NULL);
}

void
do_wl_cmd(uint32 arg, uint argc, char *argv[])
{
	wl_info_t *wl = (wl_info_t *)arg;
	cmd_t *cmd;
	int ret = 0;

	if (argc < 2)
		printf("missing subcmd\n");
	else {
		bool supported = TRUE;

		/* search for command */
		cmd = wlu_find_cmd(argv[1]);

		/* defaults to using the set_var and get_var commands */
		if (cmd == NULL) {
			supported = FALSE;
			cmd = &wl_varcmd;
		}

		/* do command */
		ret = (*cmd->func)(wl->wlc, cmd, argv + 1);
#ifdef ATE_BUILD
		if ((ret != BCME_OK) && (supported == FALSE))
			printf("ATE: Command not supported!!!\n");
#endif // endif
		printf("ret=%d (%s)\n", ret, bcmerrorstr(ret));
	}
} /* do_wl_cmd */

#endif  /* ATE_BUILD */

#ifdef WLC_LOW_ONLY
/* Temp command to during performance improvements */
static void
do_wlhist_cmd(void *arg, int argc, char *argv[])
{
	wl_info_t *wl = (wl_info_t *)arg;

	if (strcmp(argv[1], "clear") == 0) {
		wlc_rpc_bmac_dump_txfifohist(wl->wlc_hw, FALSE);
		return;
	}

	wlc_rpc_bmac_dump_txfifohist(wl->wlc_hw, TRUE);
}

static void
do_wldpcdump_cmd(void *arg, int argc, char *argv[])
{
	wl_info_t *wl = (wl_info_t *)arg;

	printf("wlc_dpc(): stopped = %d, requested = %d\n", wl->dpc_stopped, wl->dpc_requested);
	printf("\n");
}
#endif /* WLC_LOW_ONLY */

#ifdef BCMDBG
/* Mini command to control msglevel for BCMDBG builds */
static void
do_wlmsg_cmd(void *arg, int argc, char *argv[])
{
	switch (argc) {
	case 3:
		/* Set both msglevel and msglevel2 */
		wl_msg_level2 = strtoul(argv[2], 0, 0);
		/* fall through */
	case 2:
		/* Set msglevel */
		wl_msg_level = strtoul(argv[1], 0, 0);
		break;
	case 1:
		/* Display msglevel and msglevel2 */
		printf("msglvl1=0x%x msglvl2=0x%x\n", wl_msg_level, wl_msg_level2);
		break;
	}
}
#endif /* BCMDBG */

#ifdef NOT_YET
static int
BCMATTACHFN(wl_module_init)(si_t *sih)
{
	uint16 id;

	WL_TRACE(("wl_module_init: add WL device\n"));

	if ((id = si_d11_devid(sih)) == 0xffff)
		id = BCM4318_D11G_ID;

	return hnd_add_device(sih, &bcmwl, D11_CORE_ID, id);
}

RTE_MODULE_INIT(wl_module_init);

#endif /* NOT_YET */

#ifdef WLC_LOW_ONLY

#if defined(HND_PT_GIANT) && defined(DMA_TX_FREE)
static void
wl_lowmem_free(void *wlh)
{
	wl_info_t *wl = (wl_info_t*)wlh;
	wlc_info_t *wlc = wl->wlc;
	int i;

	/* process any tx reclaims */
	for (i = 0; i < NFIFO; i++) {
		hnddma_t *di = WLC_HW_DI(wlc, i);
		if (di == NULL)
			continue;
		dma_txreclaim(di, HNDDMA_RANGE_TRANSFERED);
	}
}
#endif /* HND_PT_GIANT && DMA_TX_FREE */

static void
wl_rpc_tp_txflowctl(hnd_dev_t *dev, bool state, int prio)
{
	wl_info_t *wl = dev->softc;

	bcm_rpc_tp_txflowctl(wl->rpc_th, state, prio);
}

static void
wl_rpc_down(void *wlh)
{
	wl_info_t *wl = (wl_info_t*)(wlh);

	(void)wl;

	if (wlc_bmac_down_prep(wl->wlc_hw) == 0)
		(void)wlc_bmac_down_finish(wl->wlc_hw);
}

static void
wl_rpc_resync(void *wlh)
{
	wl_info_t *wl = (wl_info_t*)(wlh);

	/* reinit to all the default values */
	wlc_bmac_info_init(wl->wlc_hw);

	/* reload original  macaddr */
	wlc_bmac_reload_mac(wl->wlc_hw);
}

/* CLIENT dongle driver RPC dispatch routine, called by bcm_rpc_buf_recv()
 *  Based on request, push to common driver or send back result
 */
static void
wl_rpc_bmac_dispatch(void *ctx, struct rpc_buf* buf)
{
	wlc_rpc_ctx_t *rpc_ctx = (wlc_rpc_ctx_t *)ctx;

	wlc_rpc_bmac_dispatch(rpc_ctx, buf);
}

static void
wl_rpc_txflowctl(void *wlh, bool on)
{
	wl_info_t *wl = (wl_info_t *)(wlh);

	if (!wl->wlc_hw->up) {
		wl->dpc_stopped = FALSE;
		wl->dpc_requested = FALSE;
		return;
	}

	if (on) {	/* flowcontrol activated */
		if (!wl->dpc_stopped) {
			WL_TRACE(("dpc_stopped set!\n"));
			wl->dpc_stopped = TRUE;
		}
	} else {	/* flowcontrol released */

		if (!wl->dpc_stopped)
			return;

		WL_TRACE(("dpc_stopped cleared!\n"));
		wl->dpc_stopped = FALSE;

		/* if there is dpc requeset pending, run it */
		if (wl->dpc_requested) {
			wl->dpc_requested = FALSE;
			wl_dpc(wl);
		}
	}
}
#endif /* WLC_LOW_ONLY */

#if defined(WL_WOWL_MEDIA) || defined(WOWLPF)
void wl_wowl_dngldown(struct wl_info *wl)
{
	hnd_dev_t *chained = NULL;
	hnd_dev_t *dev = NULL;

	dev = wl->dev;
	if (dev)
	  chained = dev->chained;

	if (chained && chained->ops->wowldown) {
		chained->ops->wowldown(chained);
	}
}
#endif // endif

#ifdef WLC_LOW_ONLY
bool wl_dngl_is_ss(struct wl_info *wl)
{
	hnd_dev_t *chained = NULL;
	hnd_dev_t *dev = NULL;
	uint val;
	int ret = BCME_ERROR;

	dev = wl->dev;
	if (dev)
	  chained = dev->chained;

	if (chained && chained->ops->ioctl) {
		ret = chained->ops->ioctl(chained, HND_RTE_DNGL_IS_SS, &val,
			sizeof(val), NULL, NULL, FALSE);
	}
	if (ret == BCME_OK && val == TRUE)
		return TRUE;
	return FALSE;
}
#endif /* WLC_LOW_ONLY */

#ifdef BCMPCIEDEV
/** @param[in] op	e.g. FLOW_RING_CREATE, FLOW_RING_FLUSH, FLOW_RING_TIM_RESET */
static uint32 wl_flowring_update(hnd_dev_t *dev, uint16 flowid, uint8 op, uint8 * sa,
	uint8 *da, uint8 tid)
{
#ifdef PROP_TXSTATUS
	wl_info_t *wl = dev->softc;
	wl_if_t *wlif = WL_IF(wl, dev);
	struct wlc_if *wlcif = wlif != NULL ? wlif->wlcif : NULL;
	return wlc_link_txflow_scb(wl->wlc, wlcif, flowid, op, sa, da, tid);
#else
	return 0xFF;
#endif // endif
}
#endif /* BCMPCIEDEV */

#if defined(WLWFDS)
#endif /* WLWFDS */

/**
 * PROP_TXSTATUS && BCMPCIEDEV specific function. Called when the WL layer wants to report a flow
 * control related event (e.g. MAC_OPEN), this function will lead the event towards a higher
 * firmware layer that consumes the event.
 */
void wl_flowring_ctl(wl_info_t *wl, uint32 op, void *opdata)
{
#ifdef BCMPCIEDEV
	if (BCMPCIEDEV_ENAB()) { /* forward event to bus layer */
		bus_ops->flowring_ctl((void *)wl->dev->chained, op, (void *)opdata);
	}
#endif // endif
}

#ifdef BCM_HOST_MEM_UCODE
/* Read SB to PCIe translation addr */
int wl_sbaddr(wl_info_t *wl, uint32 *addr, uint32 *len)
{
	uint32 ret = BCME_OK;
#ifdef BCMPCIEDEV_ENABLED
	uint32 buf[16];

	strcpy((char *)buf, "bus:sbaddr");
	ret = wl_busioctl(wl, BUS_GET_VAR, (void *)buf, sizeof(buf), NULL, NULL, 0);

	*addr = buf[0];
	*len = buf[1];
#endif // endif
	return ret;
}
#endif /* BCM_HOST_MEM_UCODE */

#if defined(PROP_TXSTATUS) && defined(BCMPCIEDEV)
/**
 * PROP_TXSTATUS && BCMPCIEDEV specific function. Called when the WL layer wants to report a flow
 * control related event (e.g. MAC_OPEN), this function will lead the event towards a higher
 * firmware layer that consumes the event.
 */
static int wlfc_push_signal_bus_data(struct wl_info *wl, void* data, uint8 len)
{
	uint8 type = ((uint8*)data)[0];
	flowring_op_data_t	op_data;

	bzero(&op_data, sizeof(flowring_op_data_t));

	switch (type) {
		case WLFC_CTL_TYPE_MAC_OPEN:
		case WLFC_CTL_TYPE_MAC_CLOSE:
			op_data.handle = ((uint8*)data)[2];
			break;

		case WLFC_CTL_TYPE_MACDESC_ADD:
		case WLFC_CTL_TYPE_MACDESC_DEL:
			op_data.handle = ((uint8*)data)[2];
			op_data.ifindex = ((uint8*)data)[3];
			memcpy(op_data.addr, (char *)&((uint8*)data)[4], ETHER_ADDR_LEN);
			break;

		case WLFC_CTL_TYPE_INTERFACE_OPEN:
		case WLFC_CTL_TYPE_INTERFACE_CLOSE:
			op_data.ifindex = ((uint8*)data)[2];
			break;

		case WLFC_CTL_TYPE_TID_OPEN:
		case WLFC_CTL_TYPE_TID_CLOSE:
			op_data.tid = ((uint8*)data)[2];
			break;
		case WLFC_CTL_TYPE_MAC_REQUEST_PACKET:
			op_data.handle = ((uint8*)data)[3];
			op_data.tid = ((uint8*)data)[4]; /* ac bit map */
			op_data.minpkts = ((uint8*)data)[2];
			break;
		default :
			return BCME_ERROR;
	}
	wl_flowring_ctl(wl, type, (void *)&op_data);
	return BCME_OK;
} /* wlfc_push_signal_bus_data */

#ifdef BCMPCIEDEV
/**
 * PROP_TXSTATUS && BCMPCIEDEV specific function. Copies caller provided status array 'txs' into
 * caller provided packet 'p'.
 */
void wlfc_push_pkt_txstatus(struct wl_info *wl, void* p, void *txs, uint32 sz)
{
#ifdef BCM_DHDHDR
	if (BCMDHDHDR_ENAB()) {
		/* Caller has handled what this function will do,
		 * So when BCM_DHDHDR enabled we should run into here
		 */
		ASSERT(0);
	}
#endif // endif

	/* Set state to TXstatus processed */
	PKTSETTXSPROCESSED(wl->pub->osh, p);

	if (sz == 1) {
		*((uint8*)(PKTDATA(wl->pub->osh, p) + BCMPCIE_D2H_METADATA_HDRLEN)) =
			*((uint8*)txs);
		PKTSETLEN(wl->pub->osh, p, sz);
		return;
	}

	memcpy(PKTDATA(wl->pub->osh, p) + BCMPCIE_D2H_METADATA_HDRLEN, txs, sz);
	PKTSETLEN(wl->pub->osh, p, BCMPCIE_D2H_METADATA_HDRLEN + sz);
	PKTSETDATAOFFSET(p, ROUNDUP(BCMPCIE_D2H_METADATA_HDRLEN + sz, 4) >> 2);
}
#endif /* BCMPCIEDEV */

int
wlfc_upd_flr_weight(struct wl_info *wl, uint8 mac_handle, uint8 tid, void* params)
{
#ifdef BCMPCIEDEV
	flowring_op_data_t	op_data;

	if (BCMPCIEDEV_ENAB()) {
		bzero(&op_data, sizeof(flowring_op_data_t));

		op_data.tid = tid;
		op_data.handle = mac_handle;
		op_data.extra_params = params;

		wl_flowring_ctl(wl, WLFC_CTL_TYPE_UPD_FLR_WEIGHT, (void *)&op_data);
	}
#endif /* BCMPCIEDEV */
	return BCME_OK;
}

/** Enable/Disable Fair Fetch Scheduling in pciedev layer */
int
wlfc_enab_fair_fetch_scheduling(struct wl_info *wl, void* params)
{
#ifdef BCMPCIEDEV
	flowring_op_data_t	op_data;

	if (BCMPCIEDEV_ENAB()) {
		bzero(&op_data, sizeof(flowring_op_data_t));
		op_data.extra_params = params;
		wl_flowring_ctl(wl, WLFC_CTL_TYPE_ENAB_FFSCH, (void *)&op_data);
	}
#endif /* BCMPCIEDEV */
	return BCME_OK;
}

/** Get Fair Fetch Scheduling status from pciedev layer.
 * status - is the status (1 - on, 0 - off)
 */
int
wlfc_get_fair_fetch_scheduling(struct wl_info *wl, uint32 *status)
{
	int rv = BCME_OK;
#ifdef BCMPCIEDEV
	if (BCMPCIEDEV_ENAB()) {
		/* Need to generate an ioctl/iovar request to the bus */
		int cmd_len = strlen("bus:ffsched");
		int cmd_buf_len = cmd_len + 1 + sizeof(uint32);
		char* cmd_buf = (char*)MALLOC(wl->pub->osh, cmd_buf_len);
		if (cmd_buf != NULL) {
			strncpy(cmd_buf, "bus:ffsched", cmd_len);
			cmd_buf[cmd_len] = '\0';
			rv = wl_busioctl(wl, BUS_GET_VAR, cmd_buf,
			       cmd_buf_len, NULL, NULL, FALSE);
			if (rv == BCME_OK)
				*status = *((uint32*)cmd_buf);
			else
				WL_ERROR(("wl%d: %s: BUS IOCTL failed, error %d\n",
				  wl->unit, __FUNCTION__, rv));
			MFREE(wl->pub->osh, cmd_buf, cmd_buf_len);
		} else {
			WL_ERROR(("wl%d: %s: MALLOC failed\n",
			  wl->unit, __FUNCTION__));
			rv = BCME_NOMEM;
		}
	} else
		rv = BCME_UNSUPPORTED;

#endif /* BCMPCIEDEV */
	return rv;
}

#endif /* PROP_TXSTATUS */

#ifdef DONGLEBUILD
/* Force function into RAM because behavior is dependent on used interface */
void
wl_flush_rxreorderqeue_flow(struct wl_info *wl, struct reorder_rxcpl_id_list *list)
{
#ifdef BCMPCIEDEV
	if (BCMPCIEDEV_ENAB()) {
		uint32 buf[2];
		int ret;

		buf[0] = (uint32)list->head;
		buf[1] = list->cnt;

		ret = wl_busioctl(wl, BUS_FLUSH_RXREORDER_Q, buf,
			2*sizeof(uint32), NULL, NULL, FALSE);

		if (ret) {
			list->cnt = 0;
			list->head = list->tail = 0;
		}
	}
#endif /* BCMPCIEDEV */
}

uint32
wl_chain_rxcomplete_id(struct reorder_rxcpl_id_list *list, uint16 id, bool head)
{
#ifdef BCMPCIEDEV
	if (BCMPCIEDEV_ENAB()) {
		if (list->cnt == 0) {
			list->head = list->tail = id;
		} else {
			if (head) {
				bcm_chain_rxcplid(id, list->head);
				list->head = id;
			} else {
				bcm_chain_rxcplid(list->tail, id);
				list->tail = id;
			}
		}
	}
	list->cnt++;
#endif /* BCMPCIEDEV */
	return 0;
}

/** link up rx cplid of amsdu subframes */
void
wl_chain_rxcompletions_amsdu(osl_t *osh, void *p, bool norxcpl)
{
#ifdef BCMPCIEDEV
	void *p1;
	uint16 head,  current;

	if (BCMPCIEDEV_ENAB()) {
		head = PKTRXCPLID(osh, p);
		p1 = PKTNEXT(osh, p);
		while (p1 != NULL) {
			current = PKTRXCPLID(osh, p1);
			if (current == 0) {
				return;
			}
		/* Chain rx completions */
		bcm_chain_rxcplid(head, current);
		head = current;
		/* dont send out rx completions if pkt is queued up in pending list */
		if (norxcpl)
			PKTSETNORXCPL(osh, p1);
		p1 = PKTNEXT(osh, p1);
		}
	}
#endif /* BCMPCIEDEV */
}
void
wl_indicate_maccore_state(struct wl_info *wl, uint8 state)
{
#ifdef BCMPCIEDEV
	if (BCMPCIEDEV_ENAB()) {
		uint32 buf;

		buf = (uint32)state;

		wl_busioctl(wl, BUS_SET_LTR_STATE, &buf,
			sizeof(uint32), NULL, NULL, FALSE);
	}
#endif /* BCMPCIEDEV */
}

#ifdef ECOUNTERS
int
BCMATTACHFN(wl_ecounters_register_source)(uint16 stats_type, wl_ecounters_get_stats some_fn,
	void *context)
{
	return  ecounters_register_source(stats_type, ECOUNTERS_TOP_LEVEL_SW_ENTITY_WL,
		(ecounters_get_stats) some_fn, context);
}

/* The wl_ecounters code know that fn is actually pointer to a
 * funciton of one of its modules.
 * So cast it to ecounters_get_stats fn and run it.
 */
static int
wl_ecounters_entry_point(ecounters_get_stats fn, uint16 stats_type,
	void *context)
{
	wl_ecounters_get_stats wfn = (wl_ecounters_get_stats) fn;
	return (wfn) ? wfn(stats_type, context) : BCME_ERROR;
}
#endif /* ECOUNTERS */

void
wl_set_copycount_bytes(struct wl_info *wl, uint16 copycount, uint16 d11rxoffset)
{
	uint32 buf[2] = {0};
	buf[0] = copycount;
	buf[1] = d11rxoffset;

	wl_busioctl(wl, BUS_SET_COPY_COUNT, buf,
		2 * sizeof(uint32), NULL, NULL, FALSE);
}

#if defined(WL_MONITOR) && !defined(WL_MONITOR_DISABLED)
void
wl_set_monitor_mode(struct wl_info *wl, uint32  monitor_mode)
{
	uint32 buf = 0;
	buf = monitor_mode;

	wl_busioctl(wl, BUS_SET_MONITOR_MODE, (void *)&buf,
			sizeof(uint32), NULL, NULL, FALSE);
}
#endif /* WL_MONITOR */

#endif /* DONGLEBUILD */

#ifdef HOST_HDR_FETCH
/* Reclaim pkts from pciedev layer */
int wl_reclaim_bus_txpkts(wl_info_t *wl, struct spktq *pkt_list, uint16 fifo, bool free)
{
	return bus_ops->reclaim_txpkts((void *)wl->dev->chained, pkt_list, fifo, free);
}
/* map pkts from pciedev layer */
void wl_map_bus_txpkts(wl_info_t *wl, map_pkts_cb_fn cb, void *ctx)
{
	return bus_ops->map_txpkts((void *)wl->dev->chained, cb, ctx);
}
/* Push TX header from dongle memory to host scratch area */
int wl_txhdr_push(wl_info_t *wl, void *p, uint queue, bool commit)
{
	return bus_ops->txhdr_push((void *)wl->dev->chained, p, queue, commit);
}
/* Commit now : last packet in the chunk */
void
wl_txhdr_commit(wl_info_t *wl)
{
	bus_ops->txhdr_commit((void *)wl->dev->chained);
}
/* Submit a pkt to MAC DMA engine */
static void
wl_mac_dma_submit(hnd_dev_t *dev, void *p,  uint queue, bool commit, bool first_entry)
{
	wl_info_t *wl = dev->softc;
	wlc_bmac_dma_submit(wl->wlc, p, queue, commit, first_entry);
}
/* Commit a bulk of pkts to MAC DMA engine */
static void
wl_mac_dma_commit(hnd_dev_t *dev, uint queue)
{
	wl_info_t *wl = dev->softc;
	wlc_bmac_dma_commit(wl->wlc, queue);
}
#endif /* HOST_HDR_FETCH */
