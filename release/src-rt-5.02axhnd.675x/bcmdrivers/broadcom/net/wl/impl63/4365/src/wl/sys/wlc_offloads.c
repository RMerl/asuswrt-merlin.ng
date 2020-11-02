/**
 * @file
 * @brief
 * In a 'NIC+offloads' model, additional power savings for the host are possible by offloading
 * certain tasks to dongle firmware, allowing the host to switch to a lower power state.
 *
 * This file is used by host (NIC) builds, and not by firmware builds.
 *
 * Broadcom 802.11 host offload module
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
 * $Id: wlc_offloads.c 708017 2017-06-29 14:11:45Z $
 */

/**
 * @file
 * @brief
 * XXX Twiki: [OffloadsDesign] [OffloadsPhase2]
 */

#if defined(WLOFFLD)
#include <stdarg.h>
#include <wlc_cfg.h>
#include <typedefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <bcmdevs.h>
#include <pcicfg.h>
#include <siutils.h>
#include <bcmendian.h>
#include <nicpci.h>
#include <wlioctl.h>
#include <pcie_core.h>
#include <sbhnddma.h>
#include <hnddma.h>
#include <d11.h>
#include <wlc_rate.h>
#include <wlc_keymgmt.h>
#include <wlc_pub.h>
#include <wlc_bsscfg.h>
#include <wlc.h>
#include <wlioctl.h>
#include <wlc_assoc.h>
#include <bcmwpa.h>
#include <bcm_ol_msg.h>
#if defined(BCMWAPI_WPI) || defined(BCMWAPI_WAI)
#include <wlc_wapi.h>
#endif // endif
#include <wlc_hw.h>
#include <wlc_phy_int.h>
#include <wlc_bmac.h>
#include <wlc_hw_priv.h>
#include <wlc_phy_hal.h>
#include <wlc_phy_ac.h>
#include <wlc_phyreg_ac.h>
#include <wlc_scb.h>
#include <wlc_scan.h>
#include <wlc_ampdu.h>
#include <wlc_wowl.h>
#include <wlc_eventlog_ol.h>
#ifdef WL_LTR
#include <wlc_ltr.h>
#endif /* Latency Tolerance Reporting */
#include <wl_export.h>
#include <wlc_stf.h>
#include <wl_dbg.h>
#include <wlc_offloads.h>
#if !defined(MACOSX) || defined(BCMBUILD)
#ifdef EMBED_IMAGE_4352PCI
#include <rtecdc_4352pci.h>
#endif	/* EMBED_IMAGE_4352PCI */
#ifdef EMBED_IMAGE_4350PCI
#include <rtecdc_4350pci.h>
#endif /* EMBED_IMAGE_4350PCI */
#ifdef EMBED_IMAGE_43602PCI
#include <rtecdc_43602pci.h>
#endif /* EMBED_IMAGE_43602PCI */
#else /* !MACOSX || BCMBUILD */
#ifdef EMBED_IMAGE_4352PCI
#if defined(BCMDBG) || defined(WLTEST)
#include <dbg-olbin_4352pci.h>
#else
#include <rel-olbin_4352pci.h>
#endif // endif
#endif	/* EMBED_IMAGE_4352PCI */
#ifdef EMBED_IMAGE_4350PCI
#if defined(BCMDBG) || defined(WLTEST)
#include <dbg-olbin_4350pci.h>
#else
#include <rel-olbin_4350pci.h>
#endif // endif
#endif /* EMBED_IMAGE_4350PCI */
#ifdef EMBED_IMAGE_43602PCI
#if defined(BCMDBG) || defined(WLTEST)
#include <dbg-olbin_43602pci.h>
#else
#include <rel-olbin_43602pci.h>
#endif // endif
#endif /* EMBED_IMAGE_43602PCI */
#endif /* !MACOSX || BCMBUILD */
#include <wlc_hw_priv.h>
#include <wlc_tx.h>

#include <wlc_sup.h>

#define RSSI_UPDATE_TIME	(30)
#define WOWL_CONS_BUF_SZ	8192

uint8	wowl_cons_buf[WOWL_CONS_BUF_SZ + 1];
uint32	wowl_cons_buf_size = 0;

typedef struct wlc_arp_ol_info {
	struct ipv4_addr host_ip;
} wlc_arp_ol_info_t;

typedef struct wlc_nd_ol_info {
	struct ipv6_addr host_ipv6[ND_MULTIHOMING_MAX];
} wlc_nd_ol_info_t;

typedef struct wlc_ie_ol_info {
	uint8		iemask[IEMASK_SZ];
	vndriemask_info	vndriemask[MAX_VNDR_IE];
} wlc_ie_ol_info_t;

struct wlc_ol_info_t {
	wlc_info_t		*wlc;
	sbpcieregs_t		*pcieregs;
	osl_t			*osh;
	si_t			*sih;
	uint16			ol_flags;
	uint8			*msgbuf;
	uint32			msgbuf_sz;
	dmaaddr_t		msgbufpa;
	osldma_t		*dmah;
	uint 			alloced;
	uint32			mb_intstatus;
	olmsg_info		*msg_info;
	uint32 			console_addr;
	uint16			bcnmbsscount;
	uint16			rx_deferral_cnt;
	uint32			rambase;
	uint32			ramsize;
	uint32			text_start;
	uchar* 			dlarray;
	uint32 			dlarray_len;
	uchar*			bar1_addr;
	uint32			bar1_length;
	bool			ol_up;
	uint16			pso_blk;
	olmsg_shared_info_t 	*shared_info;
	wlc_arp_ol_info_t	arp_info;
	wlc_nd_ol_info_t	nd_info;
	wlc_ie_ol_info_t	ie_info;
	wlc_l2keepalive_ol_params_t	l2keepalive_info;
	bool 			updn;
	bool			frame_del;
	bool			num_bsscfg_allow;
	uint32			disablemask;	/* Deferral disable mask */
	bool			ol_arm_txenable;
	uint32			prev_cntr;
	bool 			trap_flag;
#ifdef WLTCPKEEPA
	 /* for now the data that is being used for TCP keep-alives */
	bool	tcp_keep_conns;
	wl_mtcpkeep_alive_conn_pkt_t	tcp_keepalive_conn_rec;
	wl_mtcpkeep_alive_timers_pkt_t  tcp_keepalive_timers;
	int	tcp_keepalive_num;
	int	tcp_keepalive_timers_tosend;
#endif /* WLTCPKEEPA */
	wlc_ssid_t	prefssidlist[MAX_SSID_CNT];
	uint		prefssid_cnt;
	scanol_params_t *scanparams;
	uint		scanparams_size;
	bool		ulp;
	uint32		rssi_cnt_host;	 /* Number of times rssi value fullfiled by host */
	uint32		rssi_cnt_arm;	 /* Number of times rssi value fullfuled by ARM */
	uint32		rssi_cnt_events; /* Number of LOW RSSI event received by host */
	char		country_abbrev[WLC_CNTRY_BUF_SZ];	/* current advertised ccode */
	uint8		curpwr_cache[MAXCHANNEL]; /* txpower of ea channel for offload */
	sar_limit_t	sarlimit;
	uint32		rssithrsh;

	/* Event Handler function for each type */
	wlc_eventlog_print_handler_fn_t	eventlog_print_fn[WLC_EL_LAST];
	uint8 eventlog[MAX_OL_EVENTS];
};

#define OL_ARMTX_NEEDED(_ol) (\
	((_ol)->ol_flags & \
		(OL_ARP_ENAB | OL_ND_ENAB | OL_WOWL_ENAB)) != 0)

#define VNDR_IE_ID	(221)

struct beacon_ie_notify_cmd {
	uint32			id;
	uint32			enable;
	struct ipv4_addr	vndriemask;
};

enum {
	IOV_OL,
	IOV_ARP_HOSTIP,
	IOV_ND_HOSTIP,
	IOV_OL_DEFER_RXCNT,
	IOV_OL_FRAME_DEL,
	IOV_OL_IE_NOTIFICATION,
	IOV_OL_RSSI,
	IOV_OL_SCAN_ENAB,
	IOV_OL_SCAN_RESULTS,
	IOV_OL_SCAN_INIT,
	IOV_OL_SCAN_PARAMS,
	IOV_OL_PREFSSIDS,
	IOV_OL_PFN_LIST,
	IOV_OL_PFN_ADD,
	IOV_OL_PFN_DEL,
	IOV_OL_ULP,
	IOV_OL_CURPWR,
	IOV_OL_SARLIMIT,
	IOV_OL_RSSITHRSH,
#ifdef BCMDBG
	IOV_OL_SCAN,
	IOV_OL_SCAN_DUMP,
	IOV_OL_MSGLEVEL,
	IOV_OL_MSGLEVEL2,
#endif /* BCMDBG */
	IOV_OL_PKT_FILTER,
	IOV_OL_PKT_FILTER_ADD,
	IOV_OL_L2KEEPALIVE,
	IOV_OL_NOISE,
#ifdef WLTCPKEEPA
	IOV_OL_TCPKEEPALIVE_CONN,
	IOV_OL_TCPKEEPALIVE_TIMERS,
	IOV_OL_TCPKEEPALIVE_WOWL,
#endif /* WLTCPKEEPA */
	IOV_OL_STATS,
	IOV_OL_CONS,
	IOV_OL_WOWL_CONS,
	IOV_OL_CLR,
	IOV_OL_EVENTLOG,
	IOV_OL_LAST
};

STATIC const bcm_iovar_t ol_iovars[] = {
	{"offloads", IOV_OL, (0), IOVT_UINT32, 0},
	{"ol_arp_hostip", IOV_ARP_HOSTIP, (0), IOVT_UINT32, 0},
	{"ol_nd_hostip", IOV_ND_HOSTIP, (0), IOVT_BUFFER, IPV6_ADDR_LEN},
	{"ol_deferral_cnt", IOV_OL_DEFER_RXCNT, IOVF_SET_UP, IOVT_UINT16, 0},
	{"ol_frame_del", IOV_OL_FRAME_DEL, IOVF_SET_DOWN, IOVT_BOOL, 0},
	{"ol_notify_bcn_ie", IOV_OL_IE_NOTIFICATION, (0), IOVT_BUFFER, 0},
	{"ol_rssi", IOV_OL_RSSI, (0), IOVT_BUFFER, 0},
	{"ol_scanenab", IOV_OL_SCAN_ENAB, 0, IOVT_UINT32, 0},
	{"ol_scanresults", IOV_OL_SCAN_RESULTS, 0, IOVT_UINT32, 0},
	{"ol_scaninit", IOV_OL_SCAN_INIT, 0, IOVT_UINT32, 0},
	{"ol_scanparams", IOV_OL_SCAN_PARAMS, 0, IOVT_BUFFER, 0},
	{"ol_prefssid", IOV_OL_PREFSSIDS, 0, IOVT_BUFFER, 0},
	{"ol_pfnlist", IOV_OL_PFN_LIST, 0, IOVT_BUFFER, 0},
	{"ol_pfnadd", IOV_OL_PFN_ADD, 0, IOVT_BUFFER, sizeof(pfn_olmsg_params)},
	{"ol_pfndel", IOV_OL_PFN_DEL, 0, IOVT_BUFFER, sizeof(wlc_ssid_t)},
	{"ol_ulp", IOV_OL_ULP, 0, IOVT_UINT32, 0},
	{"ol_curpwr", IOV_OL_CURPWR, 0, IOVT_BUFFER, 0},
	{"ol_sarlimit", IOV_OL_SARLIMIT, 0, IOVT_BUFFER, 0},
	{"ol_rssithrsh", IOV_OL_RSSITHRSH, 0, IOVT_UINT32, 0},
#ifdef BCMDBG
	{"ol_scan", IOV_OL_SCAN, 0, IOVT_UINT32, 0},
	{"ol_scandump", IOV_OL_SCAN_DUMP, 0, IOVT_UINT32, 0},
	{"ol_msglevel", IOV_OL_MSGLEVEL, 0, IOVT_UINT32, 0},
	{"ol_msglevel2", IOV_OL_MSGLEVEL2, 0, IOVT_UINT32, 0},
#endif /* BCMDBG */
	{"ol_pkt_filter", IOV_OL_PKT_FILTER, (0), IOVT_UINT32, 0},
	{"ol_pkt_filter_add", IOV_OL_PKT_FILTER_ADD, (0), IOVT_BUFFER, 0},
	{"ol_l2keepalive", IOV_OL_L2KEEPALIVE, (0), IOVT_BUFFER, 0},
	{"ol_noise", IOV_OL_NOISE, (0), IOVT_BUFFER, 0},
#ifdef WLTCPKEEPA
	{"wl_tcpkeepalive_conn", IOV_OL_TCPKEEPALIVE_CONN, (0), IOVT_BUFFER, 0},
	{"wl_tcpkeepalive_timers", IOV_OL_TCPKEEPALIVE_TIMERS, (0), IOVT_BUFFER, 0},
	{"wl_tcpkeepalive_wowl", IOV_OL_TCPKEEPALIVE_WOWL, (0), IOVT_UINT32, 0},
#endif /* WLTCPKEEPA */
	{"ol_stats", IOV_OL_STATS, (0), IOVT_BUFFER, 0},
	{"ol_cons", IOV_OL_CONS, (IOVF_SET_UP), IOVT_BUFFER, 0},
	{"ol_wowl_cons", IOV_OL_WOWL_CONS, (0), IOVT_BUFFER, 0},
	{"ol_clr", IOV_OL_CLR, (0), IOVT_UINT32, 0},
	{"ol_eventlog", IOV_OL_EVENTLOG, (0), IOVT_BUFFER, 0},
	{NULL, 0, 0, 0, 0}
};

#define SET_ID(a, id)	(a[id/8] |= (1 << (id%8)))
#define GET_ID(a, id)	(a[id/8] & (1 << (id%8)))
#define RESET_ID(a, id)	(a[id/8] &= (~(1 << (id%8))))

#define ALIGN_BITS    2

/*
Deferral flags:
Bit 0 : Beacons will go to ARM.
Bit 1 : All Multicast/Broadcast will go to ARM.
Bit 15 : Enable ALT_TFS (tx status for ATIM fif). This needs to be always set.
*/
#define OL_DEF_BCN_IDX		0
#define OL_DEF_MC_BC_IDX 	1
#define OL_ALT_TFS_IDX		15

#define OL_DEF_BCN_ENAB			(1 << OL_DEF_BCN_IDX)
#define OL_DEF_MC_BC_ENAB		(1 << OL_DEF_MC_BC_IDX)
#define OL_ALT_TFS_ENAB			(1 << OL_ALT_TFS_IDX)

#define WL_MSG_READ_MAXLEN			2048

#define WLC_DYNAMIC_TEMPSENSE_DELTA_TEMP        (10)

olmsg_rssi_init rssi_init_prev;
static bool wlc_ol_check_rssi_param_changes(olmsg_rssi_init *r, olmsg_rssi_init *rp, bool force);
void wlc_ol_rssi_init_values(wlc_ol_info_t *ol, bool force);
int8 wlc_ol_rssi_get_value(wlc_ol_info_t *ol);
int8 wlc_ol_noise_get_value(wlc_ol_info_t *ol);

static void wlc_ol_watchdog(void *context);

static int wlc_ol_stats(void *context, struct bcmstrbuf *b);
static int wlc_ol_get_cons(wlc_ol_info_t *ol, char** buf, uint32 *size);
static int wlc_ol_cons(void *context, struct bcmstrbuf *b);
static int wlc_ol_clr(void *context, struct bcmstrbuf *b);
static int wlc_ol_wowl_cons(void *context, struct bcmstrbuf *b);

static bool wlc_ol_msg_q_create(wlc_ol_info_t *ol);
static bool wlc_ol_validate_shared_info(wlc_ol_info_t *ol);
static void wlc_ol_init_shared_info(wlc_ol_info_t *ol);
static int wlc_ol_go(wlc_ol_info_t * ol);
static int wlc_ol_download_fw(wlc_ol_info_t *ol);
static void wlc_ol_msg_send(wlc_ol_info_t *ol, void* msg, uint16 len);
static void wlc_ol_int_arm(wlc_ol_info_t *ol);
static int
wlc_ol_doiovar(void *context, const bcm_iovar_t *vi, uint32 actionid, const char *name,
	void *p, uint plen, void *a, int alen, int vsize, struct wlc_if *wlcif);
static void wlc_ol_bsscfg_updn(void *ctx, bsscfg_up_down_event_data_t *evt);
/* Process messages sent by CR4 */
static int wlc_ol_msg_receive(wlc_ol_info_t *ol);
static int wlc_ol_bcn_enable(wlc_ol_info_t *ol, wlc_bsscfg_t *cfg);
static int wlc_ol_bcn_disable(wlc_ol_info_t *ol, wlc_bsscfg_t *cfg);

static int wlc_ol_nd_enable(wlc_ol_info_t *ol, wlc_bsscfg_t *cfg);
static int wlc_ol_nd_setip(wlc_ol_info_t *ol, struct ipv6_addr *host_ip);
static int wlc_ol_nd_disable(wlc_ol_info_t *ol);

static int wlc_ol_arp_enable(wlc_ol_info_t *ol, wlc_bsscfg_t *cfg, struct ipv4_addr *host_ip);
static int wlc_ol_arp_setip(wlc_ol_info_t *ol, struct ipv4_addr *host_ip);
#ifdef WLTCPKEEPA
static int wlc_ol_tcp_keepalive(wlc_ol_info_t *ol);
#endif /* WLTCPKEEPA */
static int wlc_ol_arp_disable(wlc_ol_info_t *ol);

static int wlc_ol_pkt_filter_enable(wlc_ol_info_t *ol, wlc_bsscfg_t *cfg);
static int wlc_ol_pkt_filter_disable(wlc_ol_info_t *ol);
static int wlc_ol_pkt_filter_add(
		    wlc_ol_info_t *ol,
		    wl_wowl_pattern_t *wowl_pattern,
		    uint wowl_pattern_len);

static void wlc_ol_wowl_enable_completed(wlc_ol_info_t *ol);

static void wlc_ol_set_default_iemask(wlc_ol_info_t *ol);
static struct scb * wlc_ol_get_scb(wlc_info_t *wlc);
static void wlc_ol_key_update(wlc_info_t *wlc, ol_key_info *dkey, wlc_key_t *skey,
	wlc_key_info_t *key_info);

static int wlc_notification_set_id(wlc_ol_info_t *ol, wlc_bsscfg_t *cfg, int id, bool enable);
static int wlc_notification_set_flag(wlc_ol_info_t *ol, wlc_bsscfg_t *cfg, bool enable);

int8 wlc_ol_noise_avg_offload(void *w);
static int wlc_ol_getint(wlc_ol_info_t *ol, uint type);
static int wlc_ol_setint(wlc_ol_info_t *ol, uint32 val, uint type);

static int wlc_eventlog_cons(wlc_ol_info_t *ol, struct bcmstrbuf *b);
static void wlc_eventlog_print(wlc_ol_info_t *ol, struct bcmstrbuf *b, wlc_ol_eventlog_t *e);
static void wlc_beacon_eventlog_handler(wlc_ol_info_t *ol, struct bcmstrbuf *b, uint8 event_type,
	uint32 event_time, uint32 event_data);

const static scanol_params_t scanol_params_default = {
	SCANOL_PARAMS_VERSION,
	(SCANOL_BCAST_SSID | SCANOL_ENABLED),
	SCANOL_UNASSOC_TIME,
	SCANOL_PASSIVE_TIME,
	SCANOL_IDLE_REST_TIME,
	SCANOL_IDLE_REST_MULTIPLIER,
	SCANOL_ACTIVE_REST_TIME,
	SCANOL_ACTIVE_REST_MULTIPLIER,
	SCANOL_CYCLE_IDLE_REST_TIME,
	SCANOL_CYCLE_IDLE_REST_MULTIPLIER,
	SCANOL_CYCLE_ACTIVE_REST_TIME,
	SCANOL_CYCLE_ACTIVE_REST_MULTIPLIER,
	SCANOL_MAX_REST_TIME,
	SCANOL_CYCLE_DEFAULT,
	SCANOL_NPROBES,
	SCANOL_SCAN_START_DLY,
	0,
	0
};

/* The device is offload capable only if hardware allows it */
bool
wlc_ol_cap(wlc_info_t *wlc)
{
	uchar *bar1va = NULL;
	uint32 bar1_size = 0;

	if (wlc->ol == NULL) {
		if (wlc->pub->sih->chip == BCM4345_CHIP_ID) {
			WL_ERROR(("%s: Offload support temporarily disabled for 4345\n",
				__FUNCTION__));
			return FALSE;
		}

		if (D11REV_LT(wlc->pub->corerev, 42) ||
			D11REV_IS(wlc->pub->corerev, 44)) {
			WL_ERROR(("%s: Offload support not present for core %d\n",
			__FUNCTION__, wlc->pub->corerev));
			return FALSE;
		}
		if (wlc->wl) {
			bar1_size = wl_pcie_bar1(wlc->wl, &bar1va);
#ifdef BCMQT
			WL_ERROR(("bar1va 0x%x, bar1_size 0x%x\n", (uint32)bar1va, bar1_size));
#endif /* BCMQT */
			if (bar1va == NULL || bar1_size == 0) {
				WL_ERROR(("%s: Offload support disabled since PCI BAR1 not found\n",
				__FUNCTION__));
				return FALSE;
			}
		}
	}
	return TRUE;
}

static void
wlc_beacon_eventlog_handler(wlc_ol_info_t *ol, struct bcmstrbuf *b, uint8 event_type,
	uint32 event_time, uint32 event_data)
{
	switch (event_type) {
		case WLC_EL_BEACON_LOST:
			bcm_bprintf(b, "Lost beacon (& connectivity as well)\n");
			break;

		case WLC_EL_BEACON_IE_CHANGED:
			bcm_bprintf(b, "Beacon IE %d changed\n", event_data);
			break;
	}
}

/* module attach/detach */
wlc_ol_info_t *
BCMATTACHFN(wlc_ol_attach)(wlc_info_t *wlc)
{
	wlc_ol_info_t *ol;
	void *regsva = (void *) wlc->regs;

	STATIC_ASSERT(OLMSG_SHARED_INFO_SZ == OLMSG_SHARED_INFO_SZ_NUM);

	/* sanity check */
	ASSERT(wlc != NULL);
	WL_TRACE(("%s: wlc_ol_attach %p \n", __FUNCTION__, regsva));

	/* module states */
	if ((ol = (wlc_ol_info_t *)MALLOCZ(wlc->osh, sizeof(wlc_ol_info_t))) == NULL) {
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
			wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		return NULL;
	}

	ol->wlc = wlc;
	ol->osh = wlc->osh;
	ol->sih = wlc->pub->sih;
	ol->pcieregs = (sbpcieregs_t *)((uchar *)regsva + PCI_16KB0_PCIREGS_OFFSET);

	/* chip memory space settings */
	switch (CHIPID(ol->sih->chip)) {
		case BCM4360_CHIP_ID:
		case BCM4352_CHIP_ID:
			ol->text_start = OL_TEXT_START_4360;
			ol->rambase = OL_RAM_BASE_4360;
			break;
		case BCM4350_CHIP_ID:
		case BCM4354_CHIP_ID:
		case BCM4356_CHIP_ID:
		case BCM43569_CHIP_ID:
		case BCM43570_CHIP_ID:
		case BCM4358_CHIP_ID:
			ol->text_start = OL_TEXT_START_4350;
			ol->rambase = OL_RAM_BASE_4350;
			break;
		case BCM43602_CHIP_ID:
			ol->text_start = OL_TEXT_START_43602;
			ol->rambase = OL_RAM_BASE_43602;
			break;
		default:
			WL_ERROR(("%s: memory layout missing for chip %d\n",
				__FUNCTION__, CHIPID(ol->sih->chip)));
			ASSERT(FALSE);
	}
	ol->ramsize = si_tcm_size(ol->sih);

	/* bsscfg up/down callback */
	if (wlc_bsscfg_updown_register(wlc, wlc_ol_bsscfg_updn, ol) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_bsscfg_updown_register() failed\n",
			wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	ol->updn = TRUE;
	/* register module up/down, watchdog, and iovar callbacks */
	if (wlc_module_register(wlc->pub, ol_iovars, "offloads", ol, wlc_ol_doiovar,
		wlc_ol_watchdog, wlc_ol_up, wlc_ol_down)) {
		WL_ERROR(("wl%d: %s: wlc_module_register() failed\n",
			wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	if ((ol->bar1_length = wl_pcie_bar1(wlc->wl, &ol->bar1_addr)) == 0) {
		WL_ERROR(("bar1 size is zero\n"));
		goto fail;
	}
	if (ol->bar1_addr == NULL) {
		WL_ERROR(("bar1 address is NULL\n"));
		goto fail;
	}

	if (wlc_ol_msg_q_create(ol) == FALSE)
		goto fail;
	ol->frame_del = TRUE;
#ifdef WLTCPKEEPA
	ol->tcp_keep_conns = FALSE;
	ol->tcp_keepalive_num = 0;
	ol->tcp_keepalive_timers_tosend = 0;
	bzero(&ol->tcp_keepalive_conn_rec, sizeof(wl_mtcpkeep_alive_conn_pkt_t));
	bzero(&ol->tcp_keepalive_timers, sizeof(wl_mtcpkeep_alive_timers_pkt_t));
#endif /* WLTCPKEEPA */
	if (!ol->scanparams) {
		ol->scanparams = MALLOC(ol->osh, sizeof(scanol_params_t));
		if (!ol->scanparams) {
			ASSERT(ol->scanparams);
			WL_ERROR(("Out of Memory\n"));
			goto fail;
		}
		ol->scanparams_size = sizeof(scanol_params_t);
		memcpy(ol->scanparams, (void *)&scanol_params_default,
		       sizeof(scanol_params_t) - sizeof(wlc_ssid_t));
		memset(ol->scanparams->ssidlist, 0, sizeof(wlc_ssid_t));
	}
	ol->ulp = TRUE;
	ol->rssithrsh = (uint32)((RSSI_THRESHOLD_2G_DEF << 16) | (RSSI_THRESHOLD_5G_DEF & 0xffff));

	/* Enable Default Offloads */
	wlc->pub->_ol = OL_BCN_ENAB;

	wlc_eventlog_register_print_handler(ol, WLC_EL_BEACON_LOST,
		wlc_beacon_eventlog_handler);
	wlc_eventlog_register_print_handler(ol, WLC_EL_BEACON_IE_CHANGED,
		wlc_beacon_eventlog_handler);

	/* Download the CR4 image as part of wl up process */
	return ol;

fail:
	/* error handling */
	wlc_ol_detach(ol);
	return NULL;
}

static int wlc_ol_fw_get(wlc_ol_info_t *ol)
{
	int ret = BCME_OK;
	wlc_info_t *wlc = ol->wlc;
	switch (CHIPID(wlc->pub->sih->chip)) {
#ifdef EMBED_IMAGE_4352PCI
		case BCM4360_CHIP_ID:
		case BCM4352_CHIP_ID:
			ol->dlarray = dlarray_4352pci;
			ol->dlarray_len = sizeof(dlarray_4352pci);
			break;
#endif	/* EMBED_IMAGE_4352PCI */
#ifdef EMBED_IMAGE_4350PCI
		case BCM4350_CHIP_ID:
		case BCM4354_CHIP_ID:
		case BCM4356_CHIP_ID:
		case BCM43569_CHIP_ID:
		case BCM43570_CHIP_ID:
		case BCM4358_CHIP_ID:
			ol->dlarray = dlarray_4350pci;
			ol->dlarray_len = sizeof(dlarray_4350pci);
			break;
#endif /* EMBED_IMAGE_4350PCI */
#ifdef EMBED_IMAGE_43602PCI
		case BCM43602_CHIP_ID:
			ol->dlarray = dlarray_43602pci;
			ol->dlarray_len = sizeof(dlarray_43602pci);
			break;
#endif /* EMBED_IMAGE_43602PCI */
		default:
			WL_ERROR(("%s: offloads image missing for chip %d\n",
				__FUNCTION__, CHIPID(wlc->pub->sih->chip)));
			ASSERT(FALSE);
			ret = BCME_UNSUPPORTED;
	}

	return ret;
}

void
BCMATTACHFN(wlc_ol_detach)(wlc_ol_info_t *ol)
{
	wlc_info_t *wlc;

	ASSERT(ol != NULL);

	if (ol == NULL)
		return;

	wlc = ol->wlc;

	ASSERT(wlc != NULL);

	if (ol->scanparams != NULL) {
		MFREE(ol->osh, ol->scanparams, ol->scanparams_size);
		ol->scanparams_size = 0;
		ol->scanparams = NULL;
	}

	if (ol->msgbuf != NULL) {
		DMA_FREE_CONSISTENT_FORCE32(ol->osh,
		ol->msgbuf, ol->alloced, ol->msgbufpa, ol->dmah);
		ol->msgbuf = NULL;
	}
	if (ol->msg_info != NULL) {
		MFREE(ol->osh, ol->msg_info, sizeof(olmsg_info));
	}
	wlc_module_unregister(wlc->pub, "offloads", ol);

	if (ol->updn == TRUE)
		wlc_bsscfg_updown_unregister(ol->wlc, wlc_ol_bsscfg_updn, ol);

	ol->updn = FALSE;
	MFREE(ol->osh, ol, sizeof(wlc_ol_info_t));
}

static bool wlc_ol_msg_q_create(wlc_ol_info_t *ol)
{
	ol->msgbuf_sz = OLMSG_BUF_SZ;

	if ((ol->msgbuf = (uint8 *)DMA_ALLOC_CONSISTENT_FORCE32(ol->osh,
		ol->msgbuf_sz, ALIGN_BITS, &ol->alloced,
		&ol->msgbufpa, &ol->dmah)) == NULL) {
		return FALSE;
	}
	WL_ERROR(("%s: phyaddrh:0x%x phyaddrl:0x%x\n",
		__FUNCTION__, (uint32)PHYSADDRHI(ol->msgbufpa), (uint32)PHYSADDRLO(ol->msgbufpa)));
	if (PHYSADDRHI(ol->msgbufpa))
		return FALSE;

	ol->msg_info = MALLOC(ol->osh, sizeof(olmsg_info));
	if (ol->msg_info == NULL) {
		return FALSE;
	}
	bzero(ol->msg_info, sizeof(olmsg_info));

	/* Initialize the message buffer */
	bcm_olmsg_create(ol->msgbuf, ol->msgbuf_sz);
	bcm_olmsg_init(ol->msg_info, ol->msgbuf,
		ol->msgbuf_sz, OLMSG_READ_HOST_INDEX, OLMSG_WRITE_HOST_INDEX);
	return TRUE;
}

void
wlc_ol_print_cons(wlc_ol_info_t *ol)
{
	char    *cons, *c, *c1;
	uint32  size, strsize;
	int err;

	err = wlc_ol_get_cons(ol, &cons, &size);
	if (err != BCME_OK) {
		WL_ERROR(("%s: wlc_ol_get_cons failed with %d \n",
		__FUNCTION__, err));
		return;
	}
	strsize = strlen(cons);
	ASSERT(strsize <= size);
	WL_ERROR(("%s: ############# Dump ARM Console: size %d ########### \n",
	    __FUNCTION__, size));

	/* Print one line at a time
	 * because print functions cannot handle a very big buffer
	 */
	for (c = cons; c && (c < cons + strsize); ) {
		c1 = c;
		c = strchr(c1, '\n');
		if (c) {
			*c = '\0';
			c++;
		}
		WL_ERROR(("%s\n", c1));
	}

	WL_ERROR(("%s: ########### ARM Console End ###########:\n", __FUNCTION__));

	MFREE(ol->osh, cons, size);
}

static void
wlc_ol_watchdog(void *context)
{
	static unsigned int t = 0;
	wlc_ol_info_t *ol;
	wlc_info_t *wlc;
	olmsg_shared_info_t *shared_info;
	uint32 curr_cntr;

	ol = (wlc_ol_info_t *)context;

	if (!ol || !ol->ol_up || !ol->shared_info) {
		/* Do not return an error here */
		return;
	}

	wlc = ol->wlc;

	/* Return now if the chip is off */
	if (!wlc->clk)
		return;

	shared_info = ol->shared_info;

#ifdef WL_OFFLOADSTATS
	/* Update beacon offload stats */
	wlc->pub->offld_cnt_received[OL_BCN_IDX] = ltoh32(shared_info->stats.rxoe_bcncnt);
	wlc->pub->offld_cnt_consumed[OL_BCN_IDX] = ltoh32(shared_info->stats.rxoe_bcndelcnt);

	/* Update ARP offload stats */
	wlc->pub->offld_cnt_received[OL_ARP_IDX] = ltoh32(shared_info->stats.rxoe_totalarpcnt);
	wlc->pub->offld_cnt_consumed[OL_ARP_IDX] = ltoh32(shared_info->stats.rxoe_arpsupresscnt);

	/* Update ND offload stats */
	wlc->pub->offld_cnt_received[OL_ND_IDX] = ltoh32(shared_info->stats.rxoe_totalndcnt);
	wlc->pub->offld_cnt_consumed[OL_ND_IDX] = ltoh32(shared_info->stats.rxoe_nssupresscnt);
#endif /* WL_OFFLOADSTATS */

	/* Checking the updated cntr */
	curr_cntr = shared_info->dngl_watchdog_cntr;
	WL_TRACE(("%s: prev_cntr=%d, curr_cntr=%d, ol->trap_flag=%d\n",
		__FUNCTION__, ol->prev_cntr, curr_cntr, ol->trap_flag));
	if (curr_cntr == ol->prev_cntr)
	{
		if (!ol->trap_flag)
				ol->trap_flag = TRUE;
		else
		{
			WL_ERROR(("wl%d: %s: ### ARM Hung !!! ### Heartbeat stuck at %d.",
				WLCWLUNIT(wlc), __FUNCTION__, curr_cntr));

			wlc_ol_print_cons(ol);

			ol->trap_flag = FALSE;
			ol->prev_cntr = 0;

#if (defined(MACOS) && (VERSION_MAJOR > 11) && defined(BCMDBG))
			{
				extern void write_socram(char *addr, int len);
				write_socram((char *) ol->bar1_addr, ol->ramsize);
			}
#endif /* (VERSION_MAJOR > 11) && BCMDBG */
			wlc_fatal_error(wlc);
		}
	}
	else {
			ol->trap_flag = FALSE;
			ol->prev_cntr = curr_cntr;
	}

	if (wlc_ol_bcn_is_enable(ol)) {
		if (t++ == RSSI_UPDATE_TIME) {
			wlc_ol_rssi_init_values(ol, FALSE);
			t = 0;
		}
	} else {
		t = 0;
	}
}

static int
wlc_ol_stats(void *context, struct bcmstrbuf *b)
{
	olmsg_shared_info_t *shared_info;
	uint i, j, k;
	wlc_ol_info_t *ol = (wlc_ol_info_t *)context;
	wlc_info_t *wlc = ol->wlc;

	/* Return now if the chip is off */
	if (!wlc->clk)
		return BCME_NOCLK;

	if (!ol->shared_info)
		return BCME_NOTREADY;

	shared_info = ol->shared_info;

	/* Beacon stats */
	bcm_bprintf(b,
	            "totolbcncount %u \nbcntohost %u\n"
	            "bcndelcnt %u \nobssidcnt %u capchangedcnt %u\n"
	            "bcnintchangedcnt %u \n",
	            ltoh32(shared_info->stats.rxoe_bcncnt),
	            ltoh32(shared_info->stats.rxoe_bcntohost),
	            ltoh32(shared_info->stats.rxoe_bcndelcnt),
	            ltoh32(shared_info->stats.rxoe_obssidcnt),
	            ltoh32(shared_info->stats.rxoe_capchangedcnt),
	            ltoh32(shared_info->stats.rxoe_bcnintchangedcnt));

	for (i = 0; i < OLMSG_BCN_MAX_IE; i++) {
		if (shared_info->stats.rxoe_iechanged[i]) {
			bcm_bprintf(b, "ie %u iechangedcnt %u\n",
			            i, ltoh16(shared_info->stats.rxoe_iechanged[i]));
		}
	}

	bcm_bprintf(b, "\nRSSI STATS\n");
	bcm_bprintf(b,  "host: %u\toffload: %u\tthreshold: %u\n",
		ol->rssi_cnt_host,
		ol->rssi_cnt_arm,
		ol->rssi_cnt_events);

	i = shared_info->stats.rxoe_arp_statidx;
	bcm_bprintf(b, "\nARP STATS\n");
	bcm_bprintf(b,
            "totalarpcnt %u sta_arpcnt %u\n"
			"arpsupresscnt %u arpresponsecnt %u\n",
			ltoh32(shared_info->stats.rxoe_totalarpcnt),
			ltoh32(shared_info->stats.rxoe_arpcnt),
			ltoh32(shared_info->stats.rxoe_arpsupresscnt),
			ltoh32(shared_info->stats.rxoe_arpresponsecnt));
	bcm_bprintf(b, "\nsrc ip\t\tdest ip\t\toperation\tstatus\t\treply\tarm_tx\n");
	for (j = 0; j < MAX_STAT_ENTRIES; j++) {
		if (IPV4_ADDR_NULL(shared_info->stats.rxoe_arp_stats[i].dest_ip.addr) &&
			IPV4_ADDR_NULL(shared_info->stats.rxoe_arp_stats[i].src_ip.addr)) {
			i = NEXT_STAT(i);
			continue;
		}
		for (k = 0; k < IPV4_ADDR_LEN; k++)
			bcm_bprintf(b, "%d.",
				shared_info->stats.rxoe_arp_stats[i].src_ip.addr[k]);
			bcm_bprintf(b, "\t");
		for (k = 0; k < IPV4_ADDR_LEN; k++)
			bcm_bprintf(b, "%d.",
				shared_info->stats.rxoe_arp_stats[i].dest_ip.addr[k]);

			bcm_bprintf(b, "\t%s\t\t%s\t%s\t%s\n",
				shared_info->stats.rxoe_arp_stats[i].is_request
				? "REQUEST" : "REPLY",
				shared_info->stats.rxoe_arp_stats[i].suppressed ?
					"suppressed" : "not suppressed",
				shared_info->stats.rxoe_arp_stats[i].resp_sent ? "Y" : "N",
				shared_info->stats.rxoe_arp_stats[i].armtx ? "TX" : "NOTX");
			i = NEXT_STAT(i);
	}

	i = shared_info->stats.rxoe_nd_statidx;
	bcm_bprintf(b, "\nND STATS\n");
	bcm_bprintf(b,
            "totalndcnt %u sta_nscnt %u\n"
			"nssupresscnt %u nsresponsecnt %u\n",
			ltoh32(shared_info->stats.rxoe_totalndcnt),
			ltoh32(shared_info->stats.rxoe_nscnt),
			ltoh32(shared_info->stats.rxoe_nssupresscnt),
			ltoh32(shared_info->stats.rxoe_nsresponsecnt));

	bcm_bprintf(b, "\ndest ip \t\t\t\t operation \t status \t reply \t arm_tx\n");
	for (j = 0; j < MAX_STAT_ENTRIES; j++) {
		if (IPV6_ADDR_NULL(shared_info->stats.rxoe_nd_stats[i].dest_ip.addr)) {
			i = NEXT_STAT(i);
			continue;
		}
		for (k = 0; k < IPV6_ADDR_LEN; k++) {
			if (shared_info->stats.rxoe_nd_stats[i].dest_ip.addr[k] < 0xf)
				bcm_bprintf(b, "0%x",
					shared_info->stats.rxoe_nd_stats[i].dest_ip.addr[k]);
			else
				bcm_bprintf(b, "%x",
					shared_info->stats.rxoe_nd_stats[i].dest_ip.addr[k]);
			if (k%2)
				bcm_bprintf(b, ":");
		}
		bcm_bprintf(b, "\t%s\t%s\t%s\t%s\n",
			shared_info->stats.rxoe_nd_stats[i].is_request ? "NS" : "NA",
			shared_info->stats.rxoe_nd_stats[i].suppressed ?
				"suppressed" : "not suppressed",
			shared_info->stats.rxoe_nd_stats[i].resp_sent ? "Y" : "N",
			shared_info->stats.rxoe_nd_stats[i].armtx ? "TX" : "NOTX");
			i = NEXT_STAT(i);
	}

	i = shared_info->stats.rxoe_pkt_filter_statidx;
	bcm_bprintf(b, "\nPacket Filtering STATS\n");

	bcm_bprintf(b,
            "total pkts filtered %u total filter matches %u total magic filter matches %u\n",
	    ltoh32(shared_info->stats.rxoe_total_pkt_filter_cnt),
	    ltoh32(shared_info->stats.rxoe_total_matching_pattern_cnt),
	    ltoh32(shared_info->stats.rxoe_total_matching_magic_cnt));

	bcm_bprintf(b,
	    "\nsuppressed \t\tpattern \tmagic \tpattern id\n");
	for (j = 0; j < MAX_STAT_ENTRIES; j++) {
		if (shared_info->stats.rxoe_pkt_filter_stats[i].pkt_filtered == 0) {
			i = NEXT_STAT(i);
			continue;
		}

		bcm_bprintf(b, "%s\t\t%u\t\t%u\t\t0x%08x\n",
			shared_info->stats.rxoe_pkt_filter_stats[i].suppressed ?
				"suppressed" : "not suppressed",
			shared_info->stats.rxoe_pkt_filter_stats[i].matched_pattern,
			shared_info->stats.rxoe_pkt_filter_stats[i].matched_magic,
			shared_info->stats.rxoe_pkt_filter_stats[i].pattern_id);
			i = NEXT_STAT(i);
	}

	bcm_bprintf(b,
		"\nTotalFrmTx %u DataFrm %u NullFrm %u\n"
		"PSPoll %u ProbeReq %u TxAcked %u TxSupr %u\n",
		ltoh32(shared_info->stats.rxoe_txpktcnt.tottxpkt),
		ltoh32(shared_info->stats.rxoe_txpktcnt.datafrm),
		ltoh32(shared_info->stats.rxoe_txpktcnt.nullfrm),
		ltoh32(shared_info->stats.rxoe_txpktcnt.pspoll),
		ltoh32(shared_info->stats.rxoe_txpktcnt.probereq),
		ltoh32(shared_info->stats.rxoe_txpktcnt.txacked),
		ltoh32(shared_info->stats.rxoe_txpktcnt.txsupressed));
	bcm_bprintf(b,
		"\nTotalFrmRx %u MgmtFrm %u DataFrm %u\n"
		"FrmProcScan %u FrmDrop %u BadFcs %u BadRxChan %u BadFrmLen %u\n",
		ltoh32(shared_info->stats.rxoe_rxpktcnt.totrxpkt),
		ltoh32(shared_info->stats.rxoe_rxpktcnt.mgmtfrm),
		ltoh32(shared_info->stats.rxoe_rxpktcnt.datafrm),
		ltoh32(shared_info->stats.rxoe_rxpktcnt.scanprocessfrm),
		ltoh32(shared_info->stats.rxoe_rxpktcnt.unassfrmdrop),
		ltoh32(shared_info->stats.rxoe_rxpktcnt.badfcs),
		ltoh32(shared_info->stats.rxoe_rxpktcnt.badrxchan),
		ltoh32(shared_info->stats.rxoe_rxpktcnt.badfrmlen));

	return BCME_OK;
}

/*
 * Dump on-chip console buffer
 */
static int
wlc_ol_cons(void *context, struct bcmstrbuf *b)
{
	char *buf;
	uint32	buf_size;
	int err = BCME_OK;
	wlc_ol_info_t *ol = (wlc_ol_info_t *)context;

	err = wlc_ol_get_cons(ol, &buf, &buf_size);
	if (err != BCME_OK) {
		return	err;
	}

	bcm_bprintf(b, "%s", buf);

	MFREE(ol->osh, buf, buf_size);

	return err;
}

static int wlc_ol_clr(void *context, struct bcmstrbuf *b)
{
	wlc_ol_info_t *ol = (wlc_ol_info_t *)context;
	wlc_info_t *wlc = ol->wlc;

	/* Return now if the chip is off */
	if (!wlc->clk)
		return BCME_NOCLK;

	if (!ol->shared_info)
		return BCME_NOTREADY;

	bzero((void *)(&(ol->shared_info->stats)),
		sizeof(ol->shared_info->stats));
	ol->rssi_cnt_host = 0;
	ol->rssi_cnt_arm = 0;
	ol->rssi_cnt_events = 0;
	return BCME_OK;
}

static int
wlc_ol_get_cons(wlc_ol_info_t *ol, char** outbuf, uint32 *size)
{
	uint32	cons_addr;
	uint32	*cons;
	char	*buf;
	char	*local_buf;
	uint32	buf_size, buf_idx;
	uint32	i;
	olmsg_shared_info_t *shared_info = ol->shared_info;

	/* Return now if the chip is off */
	if (!ol->wlc->hw->clk) {
		WL_ERROR(("No Clock.\n"));
		return BCME_NOCLK;
	}

	if (!ol->ol_up) {
		WL_ERROR(("Offload not enabled\n"));
		return BCME_ERROR;
	}

	if (!ol->shared_info) {
		WL_ERROR(("ol->shared_info not initialized\n"));
		return BCME_NOTREADY;
	}
	cons_addr = ltoh32(shared_info->console_addr);
	if (cons_addr > (ol->rambase+ ol->ramsize)) {
		WL_ERROR(("console_addr %x is exceeding ram area\n", cons_addr));
		return BCME_ERROR;
	}
	cons = (uint32 *)(ol->bar1_addr + cons_addr);
	if (ltoh32(cons[0]) > (ol->rambase + ol->ramsize)) {
		WL_ERROR(("The cons[0] is exceeding ram area\n"));
		return BCME_ERROR;
	}

	/* unwrap the circular buffer as we copy */

	/* read the hnd_log info from device memory */
	buf = (char *)(ol->bar1_addr + ltoh32(cons[0]));
	buf_size = ltoh32(cons[1]);

	/* Validate buf_size to prevent divide by 0 */
	if ((buf_size == 0) || (buf == NULL)) {
		WL_ERROR(("Invalid value. Read buf_size of 0\n"));
		return BCME_ERROR;
	}

	buf_idx = ltoh32(cons[2])%buf_size;

	/* skip leading nulls */
	for (i = 0; i < buf_size; i++) {
		if (buf[(buf_idx + i)%buf_size] != '\0')
			break;
	}

	if (buf_idx + i < buf_size) {

		*size = buf_size - i + 1;

		/* allocate a local buffer and copy the log contents */
		local_buf = (char*)MALLOC(ol->osh, *size);
		if (local_buf == NULL) {
			*size = 0;
			return BCME_NOMEM;
		}

		/* copy from start index to the end first */
		bcopy(buf + buf_idx + i, local_buf, buf_size - (buf_idx + i));

		/* then copy remainder from the begining of the buffer up to the startindex */
		bcopy(buf, local_buf + (buf_size - (buf_idx + i)), buf_idx);

	} else {
		i = (buf_idx + i)% buf_size;
		ASSERT(i <= buf_idx);
		*size = buf_idx - i + 1;

		/* allocate a local buffer and copy the log contents */
		local_buf = (char*)MALLOC(ol->osh, *size);
		if (local_buf == NULL) {
			*size = 0;
			return BCME_NOMEM;
		}

		/* copy from start index to the end first */
		bcopy(buf + i, local_buf, buf_idx - i);
	}

	/* null terminate the entire local buffer */
	local_buf[*size-1] = '\0';
	*outbuf = local_buf;
	return BCME_OK;
}

static int
wlc_ol_wowl_get_cons_data(wlc_ol_info_t *ol)
{
	char	*cons;
	uint32	size, start;
	int err;

	wowl_cons_buf_size = 0;
	start = 0;

	err = wlc_ol_get_cons(ol, &cons, &size);
	if (err != BCME_OK) {
		return	err;
	}

	/* see how much we can copy */
	wowl_cons_buf_size = MIN(size, WOWL_CONS_BUF_SZ);
	if (wowl_cons_buf_size == WOWL_CONS_BUF_SZ) {
	    start = size - WOWL_CONS_BUF_SZ;
	}

	bcopy(&cons[start], wowl_cons_buf, wowl_cons_buf_size);

	/* null terminate the entire local buffer and print */
	wowl_cons_buf[wowl_cons_buf_size-1] = '\0';

	MFREE(ol->osh, cons, size);

	return BCME_OK;
}

static int wlc_ol_wowl_cons(void *context, struct bcmstrbuf *b)
{
	if (wowl_cons_buf_size == 0)
		return BCME_NORESOURCE;

	bcm_bprintf(b, "%s", wowl_cons_buf);

	return BCME_OK;
}

void
wlc_ol_enable_intrs(wlc_ol_info_t *ol, bool enable)
{
	sbpcieregs_t *pcieregs = ol->pcieregs;

	if (!ol->ol_up)
		return;

	/*
	 * intmask : Bits 8 & 9 to enable PCIE to SB interrupts
	 * mailboxintmsk: Bits 8 & 9 to enable SB to PCIE interrupts
	 */

	if (enable) {
		OR_REG(ol->osh, &pcieregs->mailboxintmsk, PCIE_MB_TOPCIE_FN0_0);
	} else {
		AND_REG(ol->osh, &pcieregs->mailboxintmsk, ~PCIE_MB_TOPCIE_FN0_0);
	}
}

static int wlc_ol_go(wlc_ol_info_t *ol)
{
	int ret = BCME_OK;
	uint origidx, i;

	/* Remember the original index */
	origidx = si_coreidx(ol->sih);

	si_setcore(ol->sih, ARMCR4_CORE_ID, 0);

	wlc_ol_arm_halt(ol);
	if ((ret = wlc_ol_download_fw(ol))) {
		si_setcoreidx(ol->sih, origidx);
		return ret;
	}

	/* Messaging initialization */
	/* Update msgbuffer registers with shared buffer address */
	wlc_ol_init_shared_info(ol);

	if (CHIPID(ol->sih->chip) == BCM43602_CHIP_ID) {
		/* Firmware crashes on SOCSRAM access when core is in reset */
		if (!(si_setcore(ol->sih, SOCRAM_CORE_ID, 0))) {
			WL_ERROR(("%s: Failed to find SOCRAM core!\n", __FUNCTION__));
			return BCME_ERROR;
		}
		si_core_reset(ol->sih, 0, 0);
		si_setcore(ol->sih, ARMCR4_CORE_ID, 0);
	}

	/* Run ARM  */
	si_wrapperreg(ol->sih, AI_IOCTRL, SICF_CPUHALT, 0);

	/* Restore original core index */
	si_setcoreidx(ol->sih, origidx);

	for (i = 0; (i < 2000) && (ol->shared_info->dngl_watchdog_cntr == 0); i++) {
		OSL_DELAY(1000);
	}

	WL_ERROR(("%s: ARM firmware initialization time %d ms\n", __FUNCTION__, i));

	return ret;
}

static int
wlc_ol_download_fw(wlc_ol_info_t *ol)
{
	int ret = BCME_OK;
	unsigned int index = 0;
	unsigned int len;
	uint32 *bar1_addr32;
	uint32 *dlarray32;
	uint32 vector = 0;

	if ((ret = wlc_ol_fw_get(ol)))
		return ret;

	bar1_addr32 = (uint32 *)(ol->bar1_addr + ol->text_start);
	dlarray32 = (uint32 *)ol->dlarray;

	/* Convert addresses into 32bit to write in 4-byte chunks */
	len = (ol->dlarray_len/4) + 1;

	/* Write firmware image */
	for (index = 0; index < len; index++) {
		bar1_addr32[index] = dlarray32[index];
#ifdef BCMQT
		if ((index % 0x1000) == 0) {
			WL_ERROR(("offload download index 0x%x src 0x%x dst 0x%x\n",
				index, dlarray32[index], bar1_addr32[index]));
		}
#endif // endif
	}

	/* Copy the reset vector to flops */
	if (si_has_flops(ol->sih)) {
		vector = R_REG(ol->osh, dlarray32);
		W_REG(ol->osh, (uint32*)&ol->bar1_addr[0], vector);
	}

	return ret;
}

static bool wlc_ol_validate_shared_info(wlc_ol_info_t *ol)
{
	/* if offload is disabled, shared_info == NULL */
	if (ol->shared_info == NULL)
		return FALSE;

	if ((ol->shared_info->marker_begin == MARKER_BEGIN) &&
		(ol->shared_info->marker_end == MARKER_END)) {
			return TRUE;
		}

		WL_ERROR(("%s: ARM share info structure is corrupted."
			" marker_begin 0x%x, marker_end 0x%x\n",
			__FUNCTION__, ol->shared_info->marker_begin,
			ol->shared_info->marker_end));
		ol->shared_info = NULL;
		return FALSE;
}

static void wlc_ol_init_shared_info(wlc_ol_info_t *ol)
{
	olmsg_shared_info_t *shared_info;
	wlc_pub_t *pub = ol->wlc->pub;

	shared_info = (olmsg_shared_info_t *)(ol->bar1_addr +
		+ ol->rambase + ol->ramsize - OLMSG_SHARED_INFO_SZ);

	shared_info->marker_begin = MARKER_BEGIN;
	shared_info->marker_end = MARKER_END;

	shared_info->msgbufaddr_low = PHYSADDRLO(ol->msgbufpa);
	shared_info->msgbufaddr_high = PHYSADDRHI(ol->msgbufpa);
	shared_info->msgbuf_sz = OLMSG_BUF_SZ;

	/* copy *vars* for ARM to pick up from TCM */
	ASSERT(pub->vars_size < MAXSZ_NVRAM_VARS);
	shared_info->vars_size = pub->vars_size;
	bcopy(pub->vars, (void *)(shared_info->vars), pub->vars_size);

	shared_info->flag[0] = FALSE;
	shared_info->flag[1] = FALSE;

	shared_info->dngl_watchdog_cntr = 0;
	bzero((void *)&shared_info->stats, sizeof(olmsg_dump_stats));
	bzero((void *)&shared_info->wowl_host_info, sizeof(wowl_host_info_t));

	ol->shared_info = shared_info;

#ifdef BCMQT
	WL_ERROR(("bar1_addr %p rambase %x ramsize %x shared_info %p\n", ol->bar1_addr,
		ol->rambase, ol->ramsize, shared_info));
#endif // endif
}

static void
wlc_ol_key_event_cb(void *cb_ctx, wlc_keymgmt_event_data_t *event_data)
{
	wlc_ol_info_t *ol = (wlc_ol_info_t *)cb_ctx;

	/* could be optimized to send just the changed key */
	if (event_data->event == WLC_KEYMGMT_EVENT_KEY_UPDATED)
		wlc_ol_armtx(ol, TRUE);
}

int wlc_ol_down(void *hdl)
{
	wlc_ol_info_t *ol = (wlc_ol_info_t *)hdl;
	wlc_info_t *wlc = ol->wlc;

	uint32 macintmask;

	if (!ol->ol_up)
	    return BCME_OK;

#ifdef WOWL
	/* Keep offloads enabled for wowl mode */
	if (WOWL_ACTIVE(ol->wlc->pub)) {
		WL_ERROR(("%s: wowl active, so return\n", __FUNCTION__));
	    return BCME_OK;
	}
#endif // endif
	/* Disable SB to PCIE interrupt */
	wlc_ol_enable_intrs(ol, FALSE);

	macintmask = wl_intrsoff(ol->wlc->wl);

	wlc_ol_arm_halt(ol);

	ol->shared_info = NULL;

	bcm_olmsg_deinit(ol->msg_info);
	ol->ol_flags = 0;
	ol->ol_up = FALSE;
	ol->ol_arm_txenable = FALSE;
	wl_intrsrestore(ol->wlc->wl, macintmask);

	wlc_keymgmt_event_unregister(wlc->keymgmt,
		WLC_KEYMGMT_EVENT_KEY_UPDATED, wlc_ol_key_event_cb, ol);

	return BCME_OK;
}

/**
 * Callback function. Called during a wlc_init() (which is called either during a 'wl up' or during
 * a big hammer).
 */
int wlc_ol_up(void *hdl)
{
	uint32 macintmask;
	wlc_ol_info_t *ol = (wlc_ol_info_t *)hdl;
	wlc_info_t *wlc = ol->wlc;
	d11regs_t *regs;
	regs = wlc->regs;

	if (ol->ol_up)
		return BCME_OK;

	bcm_olmsg_create(ol->msgbuf, ol->msgbuf_sz);
	bcm_olmsg_init(ol->msg_info, ol->msgbuf,
		ol->msgbuf_sz, OLMSG_READ_HOST_INDEX, OLMSG_WRITE_HOST_INDEX);
	macintmask = wl_intrsoff(wlc->wl);
	W_REG(ol->osh, &regs->intrcvlazy[1], (1 << IRL_FC_SHIFT));
	if (wlc_ol_go(ol) != BCME_OK) {
		WL_ERROR(("%s: Error in running ol firmware \n", __FUNCTION__));
		return BCME_ERROR;
	}

	ol->ol_up = TRUE;
	ol->pso_blk = wlc_read_shm(wlc, M_ARM_PSO_BLK_PTR) * 2;
	ol->rx_deferral_cnt = wlc_read_shm(wlc, (ol->pso_blk + 2));
	ol->prev_cntr = 0;
	ol->trap_flag = FALSE;

	WL_INFORM(("%s: pso blk at 0x%x\n", __FUNCTION__, ol->pso_blk));

	/* restore macintmask */
	wl_intrsrestore(wlc->wl, macintmask);

	/* Enable SB to PCIE interrupt */
	wlc_ol_enable_intrs(wlc->ol, TRUE);

	wlc_ol_set_default_iemask(ol);

#ifdef WL_LTR
	/*
	 * Send ltr_info over to ARM. Do not add LTR_ENAB() conditional
	 * around wlc_ol_ltr() - the value of LTR_ENAB() itself needs to
	 * be sent over regardless of its actual value of 0 or 1.
	 */
	wlc_ol_ltr(ol, wlc->ltr_info);
#endif /* WL_LTR */

	wlc_keymgmt_event_register(wlc->keymgmt, WLC_KEYMGMT_EVENT_KEY_UPDATED,
		wlc_ol_key_event_cb, ol);

	return BCME_OK;
}

static void wlc_ol_bsscfg_updn(void *ctx, bsscfg_up_down_event_data_t *evt)
{
	wlc_ol_info_t *ol = (wlc_ol_info_t *)ctx;
	wlc_bsscfg_t   *cfg;
	int idx;
	wlc_info_t *wlc = ol->wlc;
	int ap = 0;
	int total = 0;
	bool enable = FALSE;

	ASSERT(ctx != NULL);
	ASSERT(evt != NULL);

	FOREACH_BSS(wlc, idx, cfg) {
		if (cfg->up && (!BSSCFG_AWDL(wlc, cfg))) {
			/* Do not count AWDL bsscfg */
			total++;
			if (BSSCFG_AP(cfg))
				ap++;
		}
	}

	if ((evt->bsscfg->up) && (evt->up == FALSE) && (!BSSCFG_AWDL(wlc, evt->bsscfg))) {
		/* This BSS is coming down */
		total--;
		if (BSSCFG_AP(evt->bsscfg))
			ap--;
	}

	cfg = wlc->cfg;
	if ((total == 1) &&
		(ap == 0) &&
		(cfg != NULL) &&
		(cfg->up) &&
		(cfg->BSS) &&
		(!BSSCFG_AP(cfg))) {
		enable = TRUE;
	}

	ol->num_bsscfg_allow = enable;

	WL_INFORM(("%s: total %d, ap %d, enable %d\n", __FUNCTION__, total, ap, enable));

	if (!WLOFFLD_ENAB(ol->wlc->pub))
	    return;

	if (enable == FALSE) {
		wlc_ol_disable(ol, wlc->cfg);
	} else	{
		wlc_ol_restart(wlc->ol);
	}

}

void wlc_ol_restart(wlc_ol_info_t *ol)
{
	wlc_info_t *wlc = ol->wlc;
	wlc_bsscfg_t *cfg = wlc->cfg;

	if (!ol->ol_up)
		wlc_ol_up(ol);

	if (ol->ol_up && cfg != NULL) {
		if (cfg->up &&
			cfg->associated &&
			!ETHER_ISNULLADDR(&(cfg)->BSSID)) {
				wlc_ol_enable(ol, cfg);
		}
	}
}

/*
 * Checks whether interrupt is from CR4
 */
bool
wlc_ol_intstatus(wlc_ol_info_t *ol)
{
	sbpcieregs_t *pcieregs;
	uint retval = 0xFFFFFFFF;

	if (!ol->ol_up)
		return FALSE;

	pcieregs = ol->pcieregs;
	retval = R_REG(ol->osh, &pcieregs->mailboxint);

	if (retval & PCIE_INT_MB_FN0_0) {
		ol->mb_intstatus = retval;
		W_REG(ol->osh, &pcieregs->mailboxint, PCIE_INT_MB_FN0_0);
		/* Disable interrupt also */
		wlc_ol_enable_intrs(ol, FALSE);
		return TRUE;
	}

	return FALSE;
}

static void
wlc_ol_msg_send(wlc_ol_info_t *ol, void* msg, uint16 len)
{
	/* write the message into the message ring */
	if (bcm_olmsg_writemsg(ol->msg_info, msg, len) == BCME_OK) {
		/* interrupt to signal new message to on chip processor */
		wlc_ol_int_arm(ol);
	}
}

/* Process messages posted by CR4 */
static int
wlc_ol_msg_receive(wlc_ol_info_t *ol)
{
	olmsg_header *hdr;
	void *msg;
	uint8 *buf;
	buf = (uint8 *)MALLOC(ol->osh, WL_MSG_READ_MAXLEN);
	if (buf == NULL) {
		ASSERT(FALSE);
		return BCME_ERROR;
	}
	while (bcm_olmsg_readmsg(ol->msg_info, &buf[0], WL_MSG_READ_MAXLEN)) {
		hdr = (olmsg_header *)&buf[0];
		msg = &buf[0];
		switch (hdr->type) {
			case BCM_OL_MSG_TEST:
				WL_ERROR(("%s: BCM_OL_TEST_MSG message \n", __FUNCTION__));
				break;

			case BCM_OL_ARM_TX_DONE:
				WL_TRACE(("%s: BCM_OL_ARM_TX_DONE message \n", __FUNCTION__));
				wlc_ol_armtxdone(ol, msg);
				break;

			case BCM_OL_WOWL_ENABLE_COMPLETED:
				WL_TRACE(("%s: BCM_OL_WOWL_ENABLE_COMPLETED message \n",
				    __FUNCTION__));

				/* Note: This message has no payload */
				wlc_ol_wowl_enable_completed(ol);
				break;

			case BCM_OL_LOW_RSSI_NOTIFICATION: {
				wlc_bsscfg_t *cfg = ol->wlc->cfg;
				int8 rssi = *((int8 *)(hdr + 1));
				int i;
				uint32 rate;

				if (cfg == NULL) {
					break;
				}
				rate = ol->wlc->_wlc_rate_measurement->rxframes_rate;
				if (rate < MA_WINDOW_SZ &&
#ifdef WLP2P
				!BSS_P2P_ENAB(ol->wlc, cfg) &&
#endif // endif
				!cfg->roam->off) {

					WL_ERROR(("%s: RSSI dropped below threshold to: %d\n",
					__FUNCTION__, rssi));

					wlc_ol_inc_rssi_cnt_events(ol);
					wlc_lq_rssi_snr_noise_reset_ma(ol->wlc, cfg, rssi, 0, 0);
					wlc_lq_rssi_init(ol->wlc, rssi);
					for (i = 0; i < cfg->link->rssi_pkt_win_sz; i++)
						wlc_lq_rssi_update_ma(cfg, rssi, 0, FALSE);
					wlc_lq_rssi_event_update(cfg);
					wlc_roamscan_start(cfg, WLC_E_REASON_LOW_RSSI);
				    }
				}
				break;

			default:
				WL_ERROR(("%s: Unhandled message \n", __FUNCTION__));
				bcm_olmsg_dump_msg(ol->msg_info, (olmsg_header *)&buf[0]);
		}
	}
	MFREE(ol->osh, buf, WL_MSG_READ_MAXLEN);
	return BCME_OK;
}

/*
 * DPC: Process mailbox interrupts. Read message written by dongle
 */
void
wlc_ol_dpc(wlc_ol_info_t *ol)
{
	if (ol && ol->mb_intstatus & PCIE_INT_MB_FN0_0) {
		wlc_ol_msg_receive(ol);
		ol->mb_intstatus = 0;
	}
}

/*
 * Poll the mailbox for messages from ARM. This should only be called when
 * the interface is down and interrupts are disabled.
 */
void
wlc_ol_mb_poll(wlc_ol_info_t *ol)
{
	wlc_ol_msg_receive(ol);
}

/*
 * Generate Mailbox interrupt PCIe -> SB
 */
static void
wlc_ol_int_arm(wlc_ol_info_t *ol)
{
	sbpcieregs_t *pcieregs = NULL;

	if (ol) {
		pcieregs = ol->pcieregs;
		OR_REG(ol->osh, &pcieregs->mailboxint, PCIE_MB_TOSB_FN0_0);
	}
}

static int
wlc_ol_key_sw_wowl_update(wlc_info_t *wlc, wlc_key_t *key,
	const wlc_key_info_t *key_info, ol_key_info *skey, bool key_rot)
{
	int i;
	int retval;

	WL_INFORM(("%s\n", __FUNCTION__));

	ASSERT(key_info != NULL);

	/* update key data only if key rotated - must be done before updating seq  */
	if (key_rot) {
		ASSERT(skey->info.key_len <= sizeof(skey->data));
		retval = wlc_key_set_data(key, skey->info.algo, skey->data, sizeof(skey->data));
		if (retval != BCME_OK)
			WL_ERROR(("wl%d: %s: err %d setting key idx %d data\n",
				WLCWLUNIT(wlc), __FUNCTION__, retval, key_info->key_idx));
	}

	/* for pairwise key update txiv */
	if (!WLC_KEY_IS_GROUP(key_info)) {
#ifdef MFP
		if (!WLC_KEY_IS_IGTK(key_info))
#endif // endif
		{
			/* no key rotation for pairwise key */
			ASSERT(!key_rot);

			retval = wlc_key_advance_seq(key, skey->txiv.buf,
				sizeof(skey->txiv.buf), 0, TRUE);
			if (retval != BCME_OK)
				WL_ERROR(("wl%d: %s: err %d setting key idx %d tx seq, rot %d\n",
					WLCWLUNIT(wlc), __FUNCTION__, retval,
					key_info->key_idx, key_rot));
		}
	}

	/* for all keys, update rxivs. without key rotation, seq can only advance */
	for (i = 0; i < WLC_KEY_NUM_RX_SEQ; i++) {
		uint8* seq = skey->rxiv[i].buf;
		size_t seq_len = sizeof(skey->rxiv[i].buf);
		if (key_rot)
			retval = wlc_key_set_seq(key, seq, seq_len, (wlc_key_seq_id_t)i, FALSE);
		else
			retval = wlc_key_advance_seq(key, seq, seq_len, (wlc_key_seq_id_t)i, FALSE);

		if (retval != BCME_OK)
			WL_ERROR(("wl%d: %s: err %d setting key idx %d rx seq %d rot %d\n",
				WLCWLUNIT(wlc), __FUNCTION__, retval,
				key_info->key_idx, i, key_rot));
	}

	return retval;
}

static void wlc_ol_wowl_group_key_update(wlc_ol_info_t *ol, void *msg)
{
	wlc_info_t *wlc = ol->wlc;
	wlc_bsscfg_t *cfg = wlc->cfg;
	int i;
	olmsg_armtxdone *txdone_msg;
	wlc_key_t *key;
	wlc_key_info_t key_info;
#ifdef MFP
	scb_t *scb = wlc_ol_get_scb(ol->wlc);
#endif /* MFP */
	txdone_msg = (olmsg_armtxdone *) msg;
	for (i = 0; i < WLC_KEYMGMT_NUM_GROUP_KEYS; i++) {
		WL_INFORM(("wl%d: %s: Updating bss key with id %d\n",
			wlc->pub->unit, __FUNCTION__, i));
		key = wlc_keymgmt_get_bss_key(wlc->keymgmt, cfg, (wlc_key_id_t)i, &key_info);
		wlc_ol_key_sw_wowl_update(wlc, key, &key_info,
			&txdone_msg->txinfo.sec_info.bss_key_info[i],
			(txdone_msg->txinfo.sec_info.key_rot_id_mask & (1 << i)));
	}

#ifdef MFP
	if (SCB_MFP(scb)) {
		for (i = WLC_KEY_ID_IGTK_1; i <= WLC_KEY_ID_IGTK_2; i++) {
			WL_INFORM(("wl%d: %s: Updating IGTK key with id %d\n",
				wlc->pub->unit, __FUNCTION__, i));
			key = wlc_keymgmt_get_bss_key(wlc->keymgmt,
				cfg, (wlc_key_id_t)i, &key_info);
			wlc_ol_key_sw_wowl_update(wlc, key, &key_info,
				&txdone_msg->txinfo.sec_info.igtk_key_info[i-WLC_KEY_ID_IGTK_1],
				(txdone_msg->txinfo.sec_info.key_rot_id_mask & (1 << i)));
		}
	}
#endif /* MFP */
}

static void
wlc_ol_update_group_keys(wlc_info_t *wlc, ol_key_info *keys, bool igtk)
{
	int i;
	wlc_key_t *key;
	wlc_key_info_t key_info;

	if (igtk) {
#ifdef MFP
		for (i = WLC_KEY_ID_IGTK_1; i <= WLC_KEY_ID_IGTK_2; i++) {
			key = wlc_keymgmt_get_bss_key(wlc->keymgmt,
				wlc->cfg, (wlc_key_id_t)i, &key_info);
			wlc_ol_key_update(wlc, &keys[OL_IGTK_IDX_POS(i)], key, &key_info);
		}
#endif /* MFP */
	}
	else {
		for (i = 0; i < WLC_KEYMGMT_NUM_GROUP_KEYS; i++) {
			key = wlc_keymgmt_get_bss_key(wlc->keymgmt, wlc->cfg,
				(wlc_key_id_t)i, &key_info);
			wlc_ol_key_update(wlc, &keys[i], key, &key_info);
		}
	}
}

void wlc_ol_armtxdone(wlc_ol_info_t *ol, void *msg)
{
	wlc_info_t *wlc = ol->wlc;
	struct scb *scb = NULL;
	wlc_bsscfg_t *cfg = wlc->cfg;
	olmsg_armtxdone *txdone_msg = (olmsg_armtxdone *) msg;
	wlc_key_t *key = NULL;
	wlc_key_info_t key_info;

	scb = wlc_ol_get_scb(wlc);
	if (!scb) {
		WL_ERROR(("%s -- SCB NULL\n", __FUNCTION__));
		return;
	}

	if (WSEC_ENABLED(cfg->wsec)) {
		/* Update s/w information that might have changed during sleep */
		if (ol->ol_flags & OL_WOWL_ENAB) {
			key = wlc_keymgmt_get_scb_key(wlc->keymgmt, scb,
				WLC_KEY_ID_PAIRWISE, WLC_KEY_FLAG_NONE, &key_info);
			if (key_info.algo == CRYPTO_ALGO_TKIP ||
				key_info.algo == CRYPTO_ALGO_AES_CCM) {
				wlc_ol_key_sw_wowl_update(wlc, key, &key_info,
					&txdone_msg->txinfo.sec_info.scb_key_info, FALSE);
			}
			wlc_ol_wowl_group_key_update(ol, txdone_msg);
		}
		wlc_keymgmt_notify(wlc->keymgmt, WLC_KEYMGMT_NOTIF_OFFLOAD,
			cfg, scb, NULL, NULL);
	}
}

bool
wlc_ol_get_arm_txtstatus(wlc_ol_info_t *ol)
{
	if (!ol)
		return FALSE;
	return ol->ol_arm_txenable;
}

static void
wlc_ol_scan_bss(wlc_ol_info_t *ol)
{
	wlc_bsscfg_t *cfg = ol->wlc->cfg;
	olmsg_scan32 bss;
	uint32 *list = bss.list;

	bss.hdr.type = BCM_OL_SCAN_BSS;
	bss.hdr.seq = 0;
	bss.hdr.len = sizeof(olmsg_scan32) - sizeof(olmsg_header);

	bss.count = MAX_OL_SCAN_BSS;
	list[0] = (uint32)cfg->pm->PM;
	list[1] = (uint32)cfg->pm->PMblocked;
	list[2] = (uint32)cfg->pm->WME_PM_blocked;
	list[3] = (uint32)cfg->BSS;
	list[4] = (uint32)cfg->associated;

	wlc_ol_msg_send(ol, (uint8 *)&bss, sizeof(olmsg_scan32));
}

static void
wlc_ol_scan_config(wlc_ol_info_t *ol)
{
	wlc_info_t *wlc = ol->wlc;
	olmsg_scan32 config;
	uint32 *list = config.list;

	config.hdr.type = BCM_OL_SCAN_CONFIG;
	config.hdr.seq = 0;
	config.hdr.len = sizeof(olmsg_scan32) - sizeof(olmsg_header);

	config.count = MAX_OL_SCAN_CONFIG;
	list[0] = (uint32)wlc->pub->_autocountry;
	list[1] = (uint32)wlc->pub->associated;
	list[2] = (uint32)wlc->bandlocked;
	list[3] = (uint32)wlc->PMpending;
	list[4] = (uint32)wlc->bcnmisc_scan;
	list[5] = (uint32)wlc->tx_suspended;
	list[6] = (uint32)wlc->exptime_cnt;
	list[7] = (uint32)wlc->home_chanspec;
	list[8] = (uint32)ol->rssithrsh;

	wlc_ol_msg_send(ol, (uint8 *)&config, sizeof(olmsg_scan32));
}

static void
wlc_ol_scan_txrxchain(wlc_ol_info_t *ol)
{
	wlc_info_t *wlc = ol->wlc;
	olmsg_scan32 msg;
	uint32 *list = msg.list;

	msg.hdr.type = BCM_OL_SCAN_TXRXCHAIN;
	msg.hdr.seq = 0;
	msg.hdr.len = sizeof(olmsg_scan32) - sizeof(olmsg_header);

	msg.count = 1;
	list[0] = (uint32)((wlc->stf->txchain << 8) | wlc->stf->rxchain);

	wlc_ol_msg_send(ol, (uint8 *)&msg, sizeof(olmsg_scan32));
}

static void
wlc_ol_scan_chanvec(wlc_ol_info_t *ol, chanvec_t *chanvec, uint type)
{
	olmsg_chanvec msg;

	msg.hdr.type = type;
	msg.hdr.seq = 0;
	msg.hdr.len = sizeof(olmsg_chanvec) - sizeof(olmsg_header);

	bcopy(chanvec, &msg.chanvec, sizeof(chanvec_t));

	wlc_ol_msg_send(ol, (uint8 *)&msg, sizeof(olmsg_chanvec));
}

extern const char *wlc_11d_get_autocountry_default(wlc_11d_info_t *m11d);

static void
wlc_ol_scan_country(wlc_ol_info_t *ol)
{
	wlc_info_t *wlc = ol->wlc;
	olmsg_country msg;
	const char *def;

	bzero(&msg, sizeof(olmsg_country));

	msg.hdr.type = BCM_OL_SCAN_COUNTRY;
	msg.hdr.seq = 0;
	msg.hdr.len = sizeof(olmsg_country) - sizeof(olmsg_header);

	def = wlc_11d_get_autocountry_default(wlc->m11d);
	bcopy((char *)def, msg.country, WLC_CNTRY_BUF_SZ-1);

	wlc_ol_msg_send(ol, (uint8 *)&msg, sizeof(olmsg_country));
}

static void
wlc_ol_txcoremask(wlc_ol_info_t *ol)
{
	wlc_info_t *wlc = ol->wlc;
	olmsg_scan32 config;
	uint32 *list = config.list;

	config.hdr.type = BCM_OL_TXCORE;
	config.hdr.seq = 0;
	config.hdr.len = sizeof(olmsg_scan32) - sizeof(olmsg_header);

	config.count = 6;
	list[0] = (uint32)(wlc->stf->txcore[0][0] << 8 | wlc->stf->txcore[0][1]);
	list[1] = (uint32)(wlc->stf->txcore[1][0] << 8 | wlc->stf->txcore[1][1]);
	list[2] = (uint32)(wlc->stf->txcore[2][0] << 8 | wlc->stf->txcore[2][1]);
	list[3] = (uint32)(wlc->stf->txcore[3][0] << 8 | wlc->stf->txcore[3][1]);
	list[4] = (uint32)(wlc->stf->txcore[4][0] << 8 | wlc->stf->txcore[4][1]);
	list[5] = (uint32)(wlc->stf->txcore[5][0] << 8 | wlc->stf->txcore[5][1]);

	wlc_ol_msg_send(ol, (uint8 *)&config, sizeof(olmsg_scan32));
}

static void
wlc_ol_etheraddr(wlc_ol_info_t *ol, struct ether_addr *addr, uint type)
{
	olmsg_addr msg;

	msg.hdr.type = type;
	msg.hdr.seq = 0;
	msg.hdr.len = sizeof(olmsg_addr) - sizeof(olmsg_header);

	bcopy(addr, &msg.addr, ETHER_ADDR_LEN);
	wlc_ol_msg_send(ol, (uint8 *)&msg, sizeof(olmsg_addr));
}

static int
wlc_ol_curpwr_set(wlc_ol_info_t *ol)
{
	olmsg_curpwr msg;

	msg.hdr.type = BCM_OL_CURPWR;
	msg.hdr.seq = 0;
	msg.hdr.len = sizeof(ol->curpwr_cache);
	bcopy(ol->curpwr_cache, (uint8 *)msg.curpwr, msg.hdr.len);

	wlc_ol_msg_send(ol, (uint8 *)&msg, sizeof(olmsg_curpwr));
	return BCME_OK;
}

static int
wlc_ol_sarlimit_set(wlc_ol_info_t *ol)
{
#ifdef WL_SARLIMIT
	olmsg_sarlimit msg;
	sar_limit_t sar;

	wlc_channel_sarlimit_get(ol->wlc->cmi, &sar);

	msg.hdr.type = BCM_OL_SARLIMIT;
	msg.hdr.seq = 0;
	msg.hdr.len = sizeof(sar_limit_t);
	bcopy((uint8 *)&sar, (uint8 *)&msg.sarlimit, msg.hdr.len);

	wlc_ol_msg_send(ol, (uint8 *)&msg, sizeof(olmsg_sarlimit));
#endif /* WL_SARLIMIT */
	return BCME_OK;
}

static void
wlc_ol_rssithrsh_set(wlc_ol_info_t *ol, int32 int_val)
{
	int16 rssithrsh2g, rssithrsh5g;

	rssithrsh2g = (int16)(int_val >> 16);
	rssithrsh5g = (int16)(int_val & 0xffff);
	ol->rssithrsh = (uint32)((rssithrsh2g << 16) | (rssithrsh5g & 0xffff));
}

static void
wlc_ol_scan_init(wlc_ol_info_t *ol)
{
	wlc_info_t *wlc = ol->wlc;
	chanvec_t *chanvec;

	wlc_ol_scan_config(ol);
	wlc_ol_scan_bss(ol);

	chanvec = wlc_quiet_chanvec_get(wlc->cmi);
	wlc_ol_scan_chanvec(ol, chanvec, BCM_OL_SCAN_QUIET);

	chanvec = wlc_valid_chanvec_get(wlc->cmi, BAND_2G_INDEX);
	wlc_ol_scan_chanvec(ol, chanvec, BCM_OL_SCAN_VALID2G);

	chanvec = wlc_valid_chanvec_get(wlc->cmi, BAND_5G_INDEX);
	wlc_ol_scan_chanvec(ol, chanvec, BCM_OL_SCAN_VALID5G);

	wlc_ol_scan_txrxchain(ol);

	wlc_ol_scan_country(ol);

	wlc_ol_txcoremask(ol);

	if (wlc->cfg)
		wlc_ol_etheraddr(ol, &wlc->cfg->BSSID, BCM_OL_SCAN_BSSID);
	wlc_ol_etheraddr(ol, &wlc->perm_etheraddr, BCM_OL_MACADDR);

	wlc_ol_curpwr_set(ol);
	wlc_ol_sarlimit_set(ol);

	wlc_ol_setint(ol, (uint32)ol->ulp, BCM_OL_ULP);
}

static int
wlc_ol_scan_params_upd(wlc_ol_info_t *ol, void *p)
{
	uint size;
	uint32 nssid, nchan;
	scanol_params_t *params;
	if (p == NULL)
		return BCME_BADARG;

	params = (scanol_params_t *)p;
	if (params->version != SCANOL_PARAMS_VERSION) {
		WL_ERROR(("Structure version don't match the driver\n"));
		return BCME_VERSION;
	}

	nssid = MIN(params->ssid_count, SCANOL_SSID_MAX);
	nchan = MIN(params->nchannels, MAXCHANNEL);
	size = sizeof(scanol_params_t);
	if (nssid > 0)
		size += sizeof(wlc_ssid_t) * (nssid - 1);
	size += sizeof(chanspec_t) * (nchan);
	if (ol->scanparams != NULL) {
		MFREE(ol->osh, ol->scanparams, ol->scanparams_size);
		ol->scanparams_size = 0;
	}
	ol->scanparams = MALLOC(ol->osh, size);
	if (!ol->scanparams) {
		ol->scanparams_size = 0;
		return BCME_NOMEM;
	}
	ol->scanparams_size = size;

	/* initialize default into scanparams */
	memcpy(ol->scanparams, (void *)&scanol_params_default,
	       sizeof(scanol_params_t) - sizeof(wlc_ssid_t));
	memset(ol->scanparams->ssidlist, 0, sizeof(wlc_ssid_t));

	ol->scanparams->version = params->version;
	ol->scanparams->flags = params->flags;
	if (params->active_time != -1)
		ol->scanparams->active_time =
			MIN(params->active_time, SCANOL_UNASSOC_TIME_MAX);
	if (params->passive_time != -1)
		ol->scanparams->passive_time =
			MIN(params->passive_time, SCANOL_PASSIVE_TIME_MAX);
	if (params->idle_rest_time != -1)
		ol->scanparams->idle_rest_time =
			params->idle_rest_time;
	if (params->idle_rest_time_multiplier != -1)
		ol->scanparams->idle_rest_time_multiplier =
			MIN(params->idle_rest_time_multiplier, SCANOL_MULTIPLIER_MAX);
	if (params->active_rest_time != -1)
		ol->scanparams->active_rest_time =
			params->active_rest_time;
	if (params->active_rest_time_multiplier != -1)
		ol->scanparams->active_rest_time_multiplier =
			MIN(params->active_rest_time_multiplier, SCANOL_MULTIPLIER_MAX);
	if (params->scan_cycle_idle_rest_time != -1)
		ol->scanparams->scan_cycle_idle_rest_time =
			MIN(params->scan_cycle_idle_rest_time, SCANOL_MAX_REST_TIME);
	if (params->scan_cycle_idle_rest_multiplier != -1)
		ol->scanparams->scan_cycle_idle_rest_multiplier =
			MIN(params->scan_cycle_idle_rest_multiplier, SCANOL_MULTIPLIER_MAX);
	if (params->scan_cycle_active_rest_time != -1)
		ol->scanparams->scan_cycle_active_rest_time =
			params->scan_cycle_active_rest_time;
	if (params->scan_cycle_active_rest_multiplier != -1)
		ol->scanparams->scan_cycle_active_rest_multiplier =
			MIN(params->scan_cycle_active_rest_multiplier, SCANOL_MULTIPLIER_MAX);
	if (params->max_rest_time != -1)
		ol->scanparams->max_rest_time =
			MIN(params->max_rest_time, SCANOL_MAX_REST_TIME);
	if (params->max_scan_cycles != -1)
		ol->scanparams->max_scan_cycles =
			MIN(params->max_scan_cycles, SCANOL_CYCLE_MAX);
	if (params->nprobes != -1)
		ol->scanparams->nprobes =
			MIN(params->nprobes, SCANOL_NPROBES_MAX);
	if (params->scan_start_delay != -1)
		ol->scanparams->scan_start_delay =
			MIN(params->scan_start_delay, SCANOL_SCAN_START_DLY_MAX);
	ol->scanparams->ssid_count = nssid;
	if (nssid > 0)
		bcopy(params->ssidlist, ol->scanparams->ssidlist, (sizeof(wlc_ssid_t) * nssid));

	ol->scanparams->nchannels = nchan;
	if (nchan > 0) {
		uint16 *to, *from;
		chanspec_t *chanspec;
		uint i;
		if (nssid == 0) {
			to = (uint16 *)&ol->scanparams->ssidlist[1];
			from = (uint16 *)&params->ssidlist[1];
		} else {
			to = (uint16 *)&ol->scanparams->ssidlist[nssid];
			from = (uint16 *)&params->ssidlist[nssid];
		}
		chanspec = (chanspec_t *)from;
		for (i = 0; i < nchan; i++)
			chanspec[i] = CH20MHZ_CHSPEC(wf_chspec_ctlchan(chanspec[i]));
		bcopy(from, to, (sizeof(uint16) * nchan));
	}

#ifdef BCMDBG
	WL_ERROR(("scanparams:\n"));
	WL_ERROR(("  flags %x\n", ol->scanparams->flags));
	WL_ERROR(("  active_time %d, passive_time %d\n",
		ol->scanparams->active_time, ol->scanparams->passive_time));
	WL_ERROR(("  scan_start_delay %d\n", ol->scanparams->scan_start_delay));
	WL_ERROR(("  scan_cycle_idle_rest_time %d,"
		" scan_cycle_idle_rest_time_multiplier %d\n",
		ol->scanparams->scan_cycle_idle_rest_time,
		ol->scanparams->scan_cycle_idle_rest_multiplier));
	WL_ERROR(("  max_scan_cycles %d, max_rest_time %d\n",
		ol->scanparams->max_scan_cycles, ol->scanparams->max_rest_time));
	WL_ERROR(("Scan Channels (%d):", ol->scanparams->nchannels));
	if (nchan == 0)
		WL_ERROR((" None\n"));
	else {
		chanspec_t *chanspecs;
		uint i;
		if (nssid == 0)
			chanspecs = (chanspec_t *)&ol->scanparams->ssidlist[1];
		else
			chanspecs = (chanspec_t *)&ol->scanparams->ssidlist[nssid];
		WL_ERROR(("\n"));
		for (i = 0; i < nchan; i++) {
			WL_ERROR(("%04x ", chanspecs[i]));
			if (((i + 1) % 10) == 0)
				WL_ERROR(("\n"));
		}
		WL_ERROR(("\n"));
	}
	WL_ERROR(("SSID (%d):", nssid));
	if (ol->scanparams->ssidlist[0].SSID[0] == 0)
		WL_ERROR((" None\n"));
	else {
		uint i;
		for (i = 0; nssid && i < nssid; i++)
			WL_ERROR((" %s", ol->scanparams->ssidlist[i].SSID));
		WL_ERROR(("\n"));
	}
#endif /* BCMDBG */

	return BCME_OK;
}

static int
wlc_ol_scan_params(wlc_ol_info_t *ol)
{
	olmsg_scanparams *msg;
	scanol_params_t *scan_params;
	scanol_params_t *params;
	uint32 size;
	uint32 nssid, nchan;

	scan_params = ol->scanparams;
	if (scan_params == NULL)
		return BCME_BADARG;

	nssid = scan_params->ssid_count;
	nchan = scan_params->nchannels;
	WL_INFORM(("%s: nssid %d, nchan %d\n", __FUNCTION__, nssid, nchan));
	if (nchan > WL_NUMCHANNELS) {
		WL_ERROR(("%s: channel list too long [%d > %d]\n", __FUNCTION__,
			nchan, WL_NUMCHANNELS));
		return BCME_BADARG;
	}

	size = sizeof(olmsg_scanparams);
	if (nssid > 0)
		size += sizeof(wlc_ssid_t) * (nssid - 1);
	size += sizeof(chanspec_t) * nchan;
	msg = (olmsg_scanparams *)MALLOCZ(ol->osh, size);
	if (msg == NULL)
		return BCME_NOMEM;

	msg->hdr.type = BCM_OL_SCAN_PARAMS;
	msg->hdr.seq = 0;
	msg->hdr.len = (size) - sizeof(olmsg_header);
	params = &msg->params;
	bcopy((uint8 *)scan_params, (uint8 *)params, msg->hdr.len);

	wlc_ol_msg_send(ol, (uint8 *)msg, (uint16)size);
	MFREE(ol->osh, msg, size);
	return BCME_OK;
}

static int
wlc_ol_scan_parse_ssid(wlc_ol_info_t *ol)
{
	olmsg_ssids msg;
	uint32 i;

	bzero((uint8 *)&msg, sizeof(olmsg_ssids));
	msg.hdr.type = BCM_OL_PREFSSIDS;
	msg.hdr.seq = 0;
	msg.hdr.len = sizeof(olmsg_ssids) - sizeof(olmsg_header);

	if (ol->prefssid_cnt == 0) {
		/* clear the pref ssid list */
		wlc_ol_msg_send(ol, (uint8 *)&msg, sizeof(olmsg_ssids));
		return BCME_OK;
	}

	for (i = 0; i < ol->prefssid_cnt; i++) {
		/* just an SSID provided */
		bcopy((uint8 *)&ol->prefssidlist[i], (uint8 *)&msg.ssid[0],
		      sizeof(wlc_ssid_t));
		wlc_ol_msg_send(ol, (uint8 *)&msg, sizeof(olmsg_ssids));
	}
	return BCME_OK;
}

/*
 * When type is list, send a message for printing all ssids in pfn.
 * When type is add/del, send a message to add/del given ssid.
 */
static int
wlc_ol_pfn(wlc_ol_info_t *ol, uint8 *arg, uint len, uint type)
{
	olmsg_pfn msg;
	int bcmerror = BCME_OK;

	msg.hdr.type = type;
	msg.hdr.seq = 0;
	msg.hdr.len = sizeof(olmsg_pfn) - sizeof(olmsg_header);

	/* restrict the number of bytes copied  */
	if (len > sizeof(pfn_olmsg_params))
		return BCME_BUFTOOLONG;

	if ((type == IOV_OL_PFN_ADD) || (type == IOV_OL_PFN_DEL))
		bcopy(arg, &msg.params, len);

	wlc_ol_msg_send(ol, (uint8 *)&msg, sizeof(olmsg_pfn));
	return bcmerror;
}

static int
wlc_ol_conscmd(wlc_ol_info_t *ol, char *buf)
{
	int bcmerror = BCME_OK;
	olmsg_ol_conscmd msg;
	int cmdlen;

	bzero((uint8 *)&msg, sizeof(olmsg_ol_conscmd));

	msg.hdr.type = BCM_OL_CONS;
	msg.hdr.seq = 0;
	msg.hdr.len = sizeof(olmsg_ol_conscmd) - sizeof(olmsg_header);

	cmdlen = strlen(buf) + 1;
	bcopy(buf, msg.cmdline, cmdlen);

	if (cmdlen > 1)
		wlc_ol_msg_send(ol, (uint8 *)&msg, sizeof(olmsg_ol_conscmd));
	else
		bcmerror = BCME_ERROR;

	return bcmerror;
}

static int
wlc_ol_setint(wlc_ol_info_t *ol, uint32 val, uint type)
{
	olmsg_test msg;

	if (!ol->wlc->clk || !ol->ol_up)
		return BCME_NOCLK;
	msg.hdr.type = type;
	msg.hdr.seq = 0;
	msg.hdr.len = sizeof(olmsg_test) - sizeof(olmsg_header);
	msg.data = val;
	wlc_ol_msg_send(ol, (uint8 *)&msg, sizeof(olmsg_test));
	return BCME_OK;
}

static int
wlc_ol_getint(wlc_ol_info_t *ol, uint type)
{
	olmsg_test msg;

	if (!ol->wlc->clk || !ol->ol_up)
		return BCME_NOCLK;
	msg.hdr.type = type;
	msg.hdr.seq = 0;
	msg.hdr.len = sizeof(olmsg_test) - sizeof(olmsg_header);
	msg.data = 0;
	wlc_ol_msg_send(ol, (uint8 *)&msg, sizeof(olmsg_test));
	return BCME_OK;
}

static int
wlc_ol_doiovar(void *context, const bcm_iovar_t *vi, uint32 actionid, const char *name,
	void *p, uint plen, void *a, int alen, int vsize, struct wlc_if *wlcif)
{
	wlc_ol_info_t *ol = (wlc_ol_info_t *)context;
	wlc_info_t *wlc = ol->wlc;
	int32 int_val = 0, int_val2 = 0;
	int32 *ret_int_ptr;
	wlc_bsscfg_t *cfg  = wlc->cfg;
	int err = BCME_OK;
	int i = 0;
	struct ipv6_addr host_ipv6;

	if (plen >= (int)sizeof(int_val))
		bcopy(p, &int_val, sizeof(int_val));
	if (plen >= (int)sizeof(int_val) * 2)
		bcopy((void*)((uintptr)p + sizeof(int_val)), &int_val2, sizeof(int_val));

	ret_int_ptr = (int32 *)a;

	switch (actionid) {
		case IOV_GVAL(IOV_OL):
			*ret_int_ptr = wlc->pub->_ol;
			break;
		case IOV_SVAL(IOV_OL):
			if (int_val == wlc->pub->_ol)
			{
				WL_ERROR(("%s: entered offload state %d is same as current !!!\n",
					__FUNCTION__, int_val));
				break;
			}
			/* This iovar should not be used for sleep offloads */
			if (!((!int_val)||(int_val == OL_BCN_ENAB)||
				(int_val == (OL_BCN_ENAB | OL_ARP_ENAB))||
				(int_val == (OL_BCN_ENAB | OL_ND_ENAB))||
				(int_val == (OL_BCN_ENAB | OL_ARP_ENAB |
				OL_ND_ENAB))))
			{
				WL_ERROR(("%s: %d not allowed\n",
					__FUNCTION__, int_val));
				err = BCME_BADARG;
				break;
			}
			if (!wlc->pub->_ol)
			{
				wlc->pub->_ol = int_val;
				wlc_ol_enable(ol, cfg);
			}
			else
			{
				if ((ol->ol_flags & OL_ARP_ENAB) &&
					!(int_val & OL_ARP_ENAB))
					wlc_ol_arp_disable(ol);
				if ((ol->ol_flags & OL_ND_ENAB) &&
					!(int_val & OL_ND_ENAB))
					wlc_ol_nd_disable(ol);
				if ((ol->ol_flags & OL_BCN_ENAB) &&
					!(int_val & OL_BCN_ENAB))
					wlc_ol_bcn_disable(ol, cfg);
					wlc->pub->_ol = int_val;
					wlc_ol_enable(ol, cfg);

			}
			break;
		case IOV_GVAL(IOV_ARP_HOSTIP):
			if (plen < sizeof(struct ipv4_addr)) {
				err = BCME_BUFTOOSHORT;
			} else if (ol) {
				bzero(a, alen);
				bcopy(ol->arp_info.host_ip.addr, a, IPV4_ADDR_LEN);
			} else {
				err = BCME_UNSUPPORTED;
			}
			break;
		case IOV_SVAL(IOV_ARP_HOSTIP):
			if (plen < sizeof(struct ipv4_addr)) {
				err = BCME_BUFTOOSHORT;
				break;
			} else if (!ol) {
				err = BCME_UNSUPPORTED;
				break;
			}
			if (WLOFFLD_ARP_ENAB(ol->wlc->pub)) {
				bcopy(a, ol->arp_info.host_ip.addr, IPV4_ADDR_LEN);

				if (!wlc->cfg->associated) {
					err = BCME_NOTASSOCIATED;
					return err;
				}
				if (ol->ol_flags & OL_ARP_ENAB)
					err = wlc_ol_arp_setip(ol, &(ol->arp_info.host_ip));
				else
					err = wlc_ol_arp_enable(ol,
						wlc->cfg, &(ol->arp_info.host_ip));
			}
			break;
		case IOV_GVAL(IOV_ND_HOSTIP):

			bzero(a, alen);
			/*
			 * Return all IP addresses from host table.
			 * The return buffer is a list of valid IP addresses
			 * terminated by an address of all zeroes.
			 */

			bcopy(ol->nd_info.host_ipv6, a, alen);

			break;
		case IOV_SVAL(IOV_ND_HOSTIP):
			{
			if (plen < sizeof(struct ipv6_addr)) {
				err = BCME_BUFTOOSHORT;
				break;
			} else if (!ol) {
				err = BCME_UNSUPPORTED;
				break;
			}

			bcopy(a, &host_ipv6.addr, IPV6_ADDR_LEN);

			if (WLOFFLD_ND_ENAB(ol->wlc->pub)) {

				if (IPV6_ADDR_NULL(host_ipv6.addr)) {

					bzero(ol->nd_info.host_ipv6,
						(ND_MULTIHOMING_MAX * IPV6_ADDR_LEN));
					wlc_ol_nd_disable(ol);
					return BCME_OK;
				}

				for (i = 0; i < ND_MULTIHOMING_MAX; i++)
				{
					if (!bcmp(&host_ipv6.addr, &(ol->nd_info.host_ipv6[i].addr),
						IPV6_ADDR_LEN)) {
						goto setip_address;
					}
					if (IPV6_ADDR_NULL(ol->nd_info.host_ipv6[i].addr))
					{
						bcopy(&host_ipv6.addr,
							&(ol->nd_info.host_ipv6[i].addr),
							IPV6_ADDR_LEN);
							break;
					}
				}
			}

		setip_address:

			if (!wlc->cfg->associated) {
					err = BCME_NOTASSOCIATED;
					return err;
			}
			if (ol->ol_flags & OL_ND_ENAB)
				err = wlc_ol_nd_setip(ol, &host_ipv6);
			else
				err = wlc_ol_nd_enable(ol, wlc->cfg);
			}
			break;

		case IOV_GVAL(IOV_OL_PKT_FILTER):
			*ret_int_ptr = ((ol->ol_flags & OL_PKT_FILTER_ENAB) != 0);
			break;

		case IOV_SVAL(IOV_OL_PKT_FILTER):
			if (int_val) {
				ASSERT(WOWL_ACTIVE(wlc->pub));
				if (!wlc_bss_connected(cfg)) {
					err = BCME_NOTASSOCIATED;
					break;
				}

				if (!(ol->ol_flags & OL_PKT_FILTER_ENAB)) {
					err = wlc_ol_pkt_filter_enable(ol, wlc->cfg);
				}
			} else {
				if (ol->ol_flags & OL_PKT_FILTER_ENAB) {
					err = wlc_ol_pkt_filter_disable(ol);
				}
			}
			break;

		case IOV_SVAL(IOV_OL_PKT_FILTER_ADD):
			if (plen < sizeof(wl_wowl_pattern_t)) {
				err = BCME_BUFTOOSHORT;
			} else
			if (a == NULL) {
				err = BCME_BADARG;
			} else {
				err = wlc_ol_pkt_filter_add(ol, (wl_wowl_pattern_t *)a, plen);
			}
			break;

#ifdef WLTCPKEEPA
		case IOV_GVAL(IOV_OL_TCPKEEPALIVE_CONN): {
			if (alen <  sizeof(wl_mtcpkeep_alive_conn_pkt_t)) {
				WL_ERROR(("IOV_OL_TCPKEEPALIVE_CONN: buf too small\n"));
				err = BCME_UNSUPPORTED;
				break;
			}
			bzero(a, alen);
			if (ol->tcp_keep_conns != TRUE) {
				WL_ERROR(("IOV_OL_TCPKEEPALIVE_CONN: no valid info\n"));
				err = BCME_UNSUPPORTED;
				break;
			}
			bcopy(&ol->tcp_keepalive_conn_rec, a,
			    sizeof(wl_mtcpkeep_alive_conn_pkt_t));

			break;
			}

		case IOV_SVAL(IOV_OL_TCPKEEPALIVE_CONN): {
			/* for now we only support 1 keepalive connection, so just put it into
			 * the single data structure that I currently have defined. At a later date
			 * will have to manage an array of these.
			 */
			wl_mtcpkeep_alive_conn_pkt_t *ka = (wl_mtcpkeep_alive_conn_pkt_t *)a;

			WL_ERROR(("IOV_OL_TCPKEEPALIVE_CONN: have num: %d: \n",

			ol->tcp_keepalive_num));
			bcopy(ka, &ol->tcp_keepalive_conn_rec,
			    sizeof(wl_mtcpkeep_alive_conn_pkt_t));
			if (ETHER_ISNULLADDR(&ol->tcp_keepalive_conn_rec.saddr))
				bcopy(&cfg->cur_etheraddr,
				    &ol->tcp_keepalive_conn_rec.saddr,
				    sizeof(struct ether_addr));
			ol->tcp_keepalive_num++;
			ol->tcp_keep_conns = TRUE;
			break;
		}
		case IOV_GVAL(IOV_OL_TCPKEEPALIVE_TIMERS): {
				err = BCME_UNSUPPORTED;
				break;
			}
		case IOV_SVAL(IOV_OL_TCPKEEPALIVE_TIMERS): {
			wl_mtcpkeep_alive_timers_pkt_t *ka = (wl_mtcpkeep_alive_timers_pkt_t *)a;
			bcopy(ka, &ol->tcp_keepalive_timers,
			    sizeof(wl_mtcpkeep_alive_timers_pkt_t));
			ol->tcp_keepalive_timers_tosend = 1;
			break;
		}
		case IOV_SVAL(IOV_OL_TCPKEEPALIVE_WOWL): {
			int *updown = (int *)p;
			WL_ERROR(("IOV_OL_TCPKEEPALIVE_WOWL: tcp_keep_conns==%x updown=%s\n",
			    ol->tcp_keep_conns, *updown == 1 ? "Down" : "Up"));
			/* value of 1 means we are going into wowl sleep mode. 0 means we are coming
			 * out of sleep mode.
			 */
			if (*updown == 1) {
				if (ol->tcp_keep_conns == TRUE) {
					err = wlc_ol_tcp_keepalive(ol);
				}
			}
			else if (*updown == 0) {
				ol->tcp_keep_conns = FALSE;
				ol->tcp_keepalive_num = 0;
				bzero(&ol->tcp_keepalive_conn_rec,
				    sizeof(wl_mtcpkeep_alive_conn_pkt_t));
				ol->tcp_keepalive_timers_tosend = 0;
				bzero(&ol->tcp_keepalive_timers,
				    sizeof(wl_mtcpkeep_alive_timers_pkt_t));
			}
			else {
				WL_ERROR(("IOV_OL_TCPKEEPALIVE_WOWL: bad value\n"));
				err = BCME_BADARG;
			}
			break;
		}
#endif /* WLTCPKEEPA */
		case IOV_GVAL(IOV_OL_DEFER_RXCNT):
			*ret_int_ptr = (int32)ol->rx_deferral_cnt;
			break;
		case IOV_SVAL(IOV_OL_DEFER_RXCNT):
			ol->rx_deferral_cnt = (uint16)int_val;
			wlc_write_shm(wlc, (ol->pso_blk + M_DEFER_RXCNT), ol->rx_deferral_cnt);
			break;
		case IOV_GVAL(IOV_OL_FRAME_DEL):
			*ret_int_ptr = ol->frame_del;
			break;
		case IOV_SVAL(IOV_OL_FRAME_DEL):
			ol->frame_del = (int_val != 0);
			break;

		case IOV_GVAL(IOV_OL_RSSI): {
				*ret_int_ptr = (int32) wlc_ol_rssi_get_value(ol);
			}
			break;

		case IOV_GVAL(IOV_OL_NOISE): {
				*ret_int_ptr = (int32) wlc_ol_noise_get_value(ol);
			}
			break;

		case IOV_GVAL(IOV_OL_IE_NOTIFICATION): {
				struct	beacon_ie_notify_cmd *param =
					(struct beacon_ie_notify_cmd *)p;
				char	*ret_ptr = (char *)a;
				uint32	id;
				/* struct	ipv4_addr vndriemask; */

				if (plen < (int)sizeof(struct beacon_ie_notify_cmd)) {
					WL_ERROR(("%s: Parameter Error\n", __FUNCTION__));
					err = BCME_UNSUPPORTED;
				}
				id = param->id;

				WL_ERROR(("%s: GET IOVar for ID: %d\n", __FUNCTION__, id));

				if (id == -1) {
					int i;
					snprintf(ret_ptr, 40,
						"IE Notification Flag: %s\n",
						(ol->ol_flags & OL_IE_NOTIFICATION_ENAB) ?
						"enable" : "disable");
					strncat(ret_ptr, "List of enabled IE: ", 20);
					for (i = 0; i < OLMSG_BCN_MAX_IE; i++) {
						if (GET_ID(ol->ie_info.iemask, i)) {
							char t[8];
							snprintf(t, 8, "%d ", i);
							strncat(ret_ptr, t, 8);
						}
					}
					strncat(ret_ptr, "\n", 4);
				} else if (id != VNDR_IE_ID && id < OLMSG_BCN_MAX_IE) {
					snprintf(ret_ptr, 40,
						"IE Notification Flag for ID %d : %s\n",
						id,
						GET_ID(ol->ie_info.iemask, id) ?
						"enable" : "disable");
				} else if (id == VNDR_IE_ID) {
					snprintf(ret_ptr, 40,
						"Vendor IE notification is not implemented!\n");
				} else {
					err = BCME_UNSUPPORTED;
				}
			}
			break;

		case IOV_SVAL(IOV_OL_IE_NOTIFICATION): {
				struct beacon_ie_notify_cmd *param =
					(struct beacon_ie_notify_cmd *)p;
				uint32		id;
				uint32		enable;
				/* struct ipv4_addr	vndriemask; */

				if (plen < (int)sizeof(struct beacon_ie_notify_cmd)) {
					WL_ERROR(("%s: Parameter Error\n", __FUNCTION__));
					err = BCME_UNSUPPORTED;
				}

				id = param->id;
				enable = param->enable;

				WL_ERROR(("%s: SET IOVar for ID: %d to %s\n", __FUNCTION__, id,
					(enable ? "enable" : "disable")));

				if (WLOFFLD_BCN_ENAB(wlc->pub)) {
					if (id == -1) {
						WL_ERROR(("%s: Setting IE notification: %s\n",
							__FUNCTION__,
							(enable ? "enable" : "disable")));
						err = wlc_notification_set_flag(ol, cfg,
							(bool)enable);
					} else if (id != VNDR_IE_ID && id < OLMSG_BCN_MAX_IE) {
						if (ol->ol_flags &
							OL_IE_NOTIFICATION_ENAB) {
							WL_ERROR(("%s: Setting IE id: %d to %s\n",
								__FUNCTION__, id,
								(enable ? "enable" : "disable")));
							SET_ID(ol->ie_info.iemask, id);
							if (ol->ol_flags & OL_BCN_ENAB) {
								err = wlc_notification_set_id(ol,
									cfg, id, (bool)enable);
							}
						}
					} else if (id == VNDR_IE_ID) {
						err = BCME_UNSUPPORTED;
					} else {
						err = BCME_UNSUPPORTED;
					}
				} else {
					err = BCME_UNSUPPORTED;
				}
			}
			break;

		case IOV_SVAL(IOV_OL_SCAN_INIT):
			if (!wlc->clk || !ol->ol_up)
				return BCME_NOCLK;
			wlc_ol_scan_init(ol);
			if (ol->scanparams) {
				if ((err = wlc_ol_scan_params(ol)))
					break;
			}
			if (ol->prefssid_cnt) {
				if ((err = wlc_ol_scan_parse_ssid(ol)))
					break;
			}
			*ret_int_ptr = 0;
			break;

		case IOV_GVAL(IOV_OL_SCAN_PARAMS):
			if (ol->scanparams == NULL || ol->scanparams_size == 0)
				return BCME_ERROR;
			if (!a || alen < sizeof(ol->scanparams_size))
				return BCME_BUFTOOSHORT;

			bcopy(ol->scanparams, a, ol->scanparams_size);
			break;

		case IOV_SVAL(IOV_OL_SCAN_PARAMS):
		        if (plen < sizeof(scanol_params_t)) {
				WL_ERROR(("%s: plen too short [%d < %d]\n", __FUNCTION__,
					plen, (uint)sizeof(scanol_params_t)));
				return BCME_BUFTOOSHORT;
			}
			if ((err = wlc_ol_scan_params_upd(ol, p)) == BCME_OK) {
				if (wlc->clk && ol->ol_up)
					err = wlc_ol_scan_params(ol);
			}
			break;

		case IOV_SVAL(IOV_OL_PREFSSIDS):
		{
			wlc_ssid_t *ssid;

			/* single fixed-part ssid */
			if (plen < sizeof(wlc_ssid_t)) {
				WL_ERROR(("%s: plen too short [%d < %d]\n", __FUNCTION__,
					plen, (uint)sizeof(wlc_ssid_t)));
				return BCME_BUFTOOSHORT;
			}
			ssid = (wlc_ssid_t *)p;
			if (ssid->SSID[0] == 0) {
				bzero(ol->prefssidlist, sizeof(wlc_ssid_t) * ol->prefssid_cnt);
				ol->prefssid_cnt = 0;
			}
			if (ol->prefssid_cnt >= MAX_SSID_CNT) {
				WL_ERROR(("%s: Pref SSID list exessed maximum allow %d\n",
					__FUNCTION__, MAX_SSID_CNT));
				return BCME_NOMEM;
			}
			if (ssid->SSID[0] != 0) {
				bcopy((uint8 *)ssid, (uint8 *)&ol->prefssidlist[ol->prefssid_cnt],
				      sizeof(wlc_ssid_t));
				ol->prefssid_cnt++;
			}

			if (wlc->clk && ol->ol_up)
				err = wlc_ol_scan_parse_ssid(ol);
			break;
			}
		case IOV_GVAL(IOV_OL_PFN_LIST):
			if (!wlc->clk) return BCME_NOCLK;
			err = wlc_ol_pfn(ol, (uint8 *)p, plen, BCM_OL_PFN_LIST);
			break;
		case IOV_SVAL(IOV_OL_PFN_ADD):
			if (!wlc->clk) return BCME_NOCLK;
			err = wlc_ol_pfn(ol, (uint8 *)p, plen, BCM_OL_PFN_ADD);
			break;
		case IOV_SVAL(IOV_OL_PFN_DEL):
			if (!wlc->clk) return BCME_NOCLK;
			err = wlc_ol_pfn(ol, (uint8 *)p, plen, BCM_OL_PFN_DEL);
			break;
		case IOV_SVAL(IOV_OL_SCAN_ENAB):
			err = wlc_ol_setint(ol, (uint32)int_val, BCM_OL_SCAN_ENAB);
			break;
		case IOV_GVAL(IOV_OL_ULP):
			*ret_int_ptr = (int)ol->ulp;
			break;
		case IOV_SVAL(IOV_OL_ULP):
			ol->ulp = (int_val != 0);
			err = wlc_ol_setint(ol, (uint32)int_val, BCM_OL_ULP);
			break;
		case IOV_GVAL(IOV_OL_CURPWR):
			break;
		case IOV_SVAL(IOV_OL_CURPWR):
			if (ol->ol_up)
				err = wlc_ol_curpwr_set(ol);
			break;
		case IOV_GVAL(IOV_OL_SARLIMIT):
			break;
		case IOV_SVAL(IOV_OL_SARLIMIT):
			if (ol->ol_up)
				err = wlc_ol_sarlimit_set(ol);
			break;
		case IOV_GVAL(IOV_OL_RSSITHRSH):
			*ret_int_ptr = ol->rssithrsh;
			break;
		case IOV_SVAL(IOV_OL_RSSITHRSH):
			wlc_ol_rssithrsh_set(ol, int_val);
			break;

		case IOV_GVAL(IOV_OL_SCAN_RESULTS):
			*ret_int_ptr = wlc_ol_getint(ol, BCM_OL_SCAN_RESULTS);
			break;

		case IOV_GVAL(IOV_OL_STATS):
			{
				struct bcmstrbuf buf_out;
				bcm_binit(&buf_out, (char*)a, alen);
				wlc_ol_stats(ol, &buf_out);
				break;
			}
		case IOV_GVAL(IOV_OL_CONS):
			{
				struct bcmstrbuf buf_out;
				bcm_binit(&buf_out, (char*)a, alen);
				wlc_ol_cons(ol, &buf_out);
				break;
			}
		case IOV_SVAL(IOV_OL_CONS):
			wlc_ol_conscmd(ol, (char *)a);
			break;
		case IOV_GVAL(IOV_OL_WOWL_CONS):
			{
				struct bcmstrbuf buf_out;
				bcm_binit(&buf_out, (char*)a, alen);
				wlc_ol_wowl_cons(ol, &buf_out);
				break;
			}
		case IOV_GVAL(IOV_OL_CLR):
			wlc_ol_clr(ol, NULL);
			break;
#ifdef BCMDBG
		case IOV_GVAL(IOV_OL_SCAN):
			*ret_int_ptr = wlc_ol_getint(ol, BCM_OL_SCAN);
			break;
		case IOV_GVAL(IOV_OL_SCAN_DUMP):
			*ret_int_ptr = wlc_ol_getint(ol, BCM_OL_SCAN_DUMP);
			break;
		case IOV_GVAL(IOV_OL_MSGLEVEL):
			*ret_int_ptr = wlc_ol_getint(ol, BCM_OL_MSGLEVEL);
			break;
		case IOV_GVAL(IOV_OL_MSGLEVEL2):
			*ret_int_ptr = wlc_ol_getint(ol, BCM_OL_MSGLEVEL2);
			break;
		case IOV_SVAL(IOV_OL_MSGLEVEL):
			ol->ulp = (int_val != 0);
			err = wlc_ol_setint(ol, (uint32)int_val, BCM_OL_MSGLEVEL);
			break;
		case IOV_SVAL(IOV_OL_MSGLEVEL2):
			ol->ulp = (int_val != 0);
			err = wlc_ol_setint(ol, (uint32)int_val, BCM_OL_MSGLEVEL2);
			break;
#endif /* BCMDBG */
		case IOV_SVAL(IOV_OL_L2KEEPALIVE):
			if (plen < sizeof(wlc_l2keepalive_ol_params_t)) {
				err = BCME_BUFTOOSHORT;
			} else
			if (a == NULL) {
				err = BCME_BADARG;
			} else
			if (ol) {
				wlc_l2keepalive_ol_params_t	*l2keepalive_info;
				l2keepalive_info = (wlc_l2keepalive_ol_params_t	*)a;
				if ((l2keepalive_info->flags & BCM_OL_KEEPALIVE_PERIODIC_TX) &&
					((l2keepalive_info->flags &
					BCM_OL_KEEPALIVE_PERIODIC_TX_QOS))) {
					WL_ERROR(("Err: Req for both null/qos-null\n"
						"packets during l2keepalive!!!\n"));
					return BCME_BADARG;
				}
				bcopy((uint8 *)a,
					(uint8 *)(&(ol->l2keepalive_info)),
					sizeof(wlc_l2keepalive_ol_params_t));

			} else {
				err = BCME_UNSUPPORTED;
			}
			break;
		case IOV_GVAL(IOV_OL_L2KEEPALIVE):
			if (plen < sizeof(wlc_l2keepalive_ol_params_t)) {
				err = BCME_BUFTOOSHORT;
			} else if (ol) {
				bcopy((uint8 *)(&(ol->l2keepalive_info)),
					(uint8 *)a, sizeof(wlc_l2keepalive_ol_params_t));
			} else {
				err = BCME_UNSUPPORTED;
			}
			break;
		case IOV_GVAL(IOV_OL_EVENTLOG):
			{
				struct bcmstrbuf buf_out;
				bcm_binit(&buf_out, (char*)a, alen);
				wlc_eventlog_cons(ol, &buf_out);
				break;
			}
		default:
			err = BCME_UNSUPPORTED;
			break;
	}

	return err;
}

/* Safe enable/disable of offloads - arbitrates among multiple
 * components that may request enable/disable enables only when no
 * disable requests are current
 */
void wlc_ol_rx_deferral(wlc_ol_info_t *ol, uint32 mask, uint32 val)
{
	uint16 cur_flags = 0;
	uint16 new_flags = 0;

	if (!ol || !ol->ol_up)
		return;

	ol->disablemask = (ol->disablemask & ~mask) | (val & mask);

	cur_flags = wlc_read_shm(ol->wlc, (ol->pso_blk + M_PSO_ENBL_FLGS));

	if (ol->disablemask == 0) { /* Enable */
		new_flags |= OL_ALT_TFS_ENAB;

		if (ol->ol_flags & OL_BCN_ENAB) {
			new_flags |= OL_DEF_BCN_ENAB;
		}

		if (ol->ol_flags &
			(OL_ARP_ENAB |	OL_ND_ENAB)) {
			new_flags |= OL_DEF_MC_BC_ENAB;
		}

		wlc_write_shm(ol->wlc, (ol->pso_blk + M_PSO_ENBL_FLGS), cur_flags | new_flags);

		WL_TRACE(("%s: Turning OFF MI_BG_NOISE\n", __FUNCTION__));
		wlc_bmac_set_defmacintmask(ol->wlc->hw, MI_BG_NOISE, ~MI_BG_NOISE);

		/* Clean up HOST side RSSI Window */
		wlc_lq_rssi_reset_ma(ol->wlc->cfg, WLC_RSSI_EXCELLENT);
		wlc_lq_rssi_init(ol->wlc, WLC_RSSI_EXCELLENT);
		wlc_ol_rssi_init_values(ol, TRUE);
	} else { /* Disable */
		new_flags =
			~(OL_DEF_BCN_ENAB |
			OL_DEF_MC_BC_ENAB);
		new_flags &= cur_flags;

		if (ol->disablemask == OL_CFG_MASK) {
			if (ol->ol_flags & OL_BCN_ENAB) {
				new_flags |= OL_DEF_BCN_ENAB;
			}

			if (ol->ol_flags &
				(OL_ARP_ENAB |
				OL_ND_ENAB)) {
				new_flags |= OL_DEF_MC_BC_ENAB;
			}
			wlc_write_shm(ol->wlc, (ol->pso_blk + M_PSO_ENBL_FLGS), new_flags);
		}
		else {
			if (ol->ol_flags &
			    (OL_BCN_ENAB |
			     OL_ARP_ENAB |
			     OL_ND_ENAB))
				wlc_write_shm(ol->wlc, (ol->pso_blk + M_PSO_ENBL_FLGS), new_flags);
		}
		WL_TRACE(("%s: Turning ON MI_BG_NOISE\n", __FUNCTION__));
		wlc_bmac_set_defmacintmask(ol->wlc->hw, MI_BG_NOISE, MI_BG_NOISE);

		/* Send RSSI_INIT */
		wlc_lq_rssi_reset_ma(ol->wlc->cfg, WLC_RSSI_EXCELLENT);
		wlc_lq_rssi_init(ol->wlc, WLC_RSSI_EXCELLENT);
		wlc_ol_rssi_init_values(ol, TRUE);
	}

	WL_INFORM(("%s: disablemask 0x%x, cur_flags 0x%x, new_flags 0x%x, mask 0x%x, val 0x%x\n",
		__FUNCTION__, ol->disablemask, cur_flags, new_flags, mask, val));

}

static void wlc_ol_set_default_iemask(wlc_ol_info_t *ol)
{
		SET_ID(ol->ie_info.iemask, DOT11_MNG_SSID_ID);
		SET_ID(ol->ie_info.iemask, DOT11_MNG_RATES_ID);
		SET_ID(ol->ie_info.iemask, DOT11_MNG_TIM_ID);
		SET_ID(ol->ie_info.iemask, DOT11_MNG_CHANNEL_SWITCH_ID);
		SET_ID(ol->ie_info.iemask, DOT11_MNG_EXT_CSA_ID);
		SET_ID(ol->ie_info.iemask, DOT11_MNG_VS_ID);
		SET_ID(ol->ie_info.iemask, DOT11_MNG_QUIET_ID);

		ol->ie_info.vndriemask[0].oui.b.id[0] = 0x00;
		ol->ie_info.vndriemask[0].oui.b.id[1] = 0x50;
		ol->ie_info.vndriemask[0].oui.b.id[2] = 0xf2;
		ol->ie_info.vndriemask[0].oui.b.type = 0x2;

}

/* Enable beacon offload */
static int
wlc_ol_bcn_enable(wlc_ol_info_t *ol, wlc_bsscfg_t *cfg)
{
	int err = BCME_OK;
	olmsg_bcn_enable *p_bcn_enb;
	wlc_info_t *wlc = NULL;
	uint16 ie_len;
	wlc_bss_info_t *current_bss = NULL;

	/* Check for valid state to enable beacon offloads */
	if (!ol || !cfg || !BSSCFG_STA(cfg) || !cfg->BSS || !cfg->up ||
		cfg != ol->wlc->cfg || !cfg->associated || !ol->num_bsscfg_allow ||
		(ol->ol_flags & OL_BCN_ENAB) || !(ol->ol_up) ||
		!(cfg->current_bss->bcn_prb)) {
		WL_ERROR(("%s: Invalid params/state. Not enabling beacon offloads \n",
			__FUNCTION__));
		return BCME_BADARG;
	}

	wlc = ol->wlc;

	/* Enable beacon offloads */
	ol->ol_flags |= OL_BCN_ENAB;

	current_bss = cfg->current_bss;
	ie_len = current_bss->bcn_prb_len - DOT11_BCN_PRB_FIXED_LEN;

	/* alloc beacon enable message buffer */
	p_bcn_enb = (olmsg_bcn_enable *)MALLOCZ(ol->osh, sizeof(olmsg_bcn_enable) + ie_len);
	if (p_bcn_enb == NULL) {
		WL_ERROR(("%s: Fail to alloc beacon-enble memory size(%u).\n",
			__FUNCTION__, (uint16)(sizeof(olmsg_bcn_enable) + ie_len)));
		return BCME_NOMEM;
	}

	/* Fill up offload beacon enable message */
	p_bcn_enb->hdr.type = BCM_OL_BEACON_ENABLE;
	p_bcn_enb->hdr.len = sizeof(olmsg_bcn_enable) - sizeof(p_bcn_enb->hdr) + ie_len;
	p_bcn_enb->defcnt = ol->rx_deferral_cnt;
	p_bcn_enb->bcn_length = current_bss->bcn_prb_len;
	bcopy(&cfg->BSSID, &p_bcn_enb->BSSID, sizeof(struct ether_addr));
	bcopy(&cfg->cur_etheraddr,  &p_bcn_enb->cur_etheraddr, sizeof(struct ether_addr));
	p_bcn_enb->bi =  wlc->cfg->current_bss->beacon_period;
	p_bcn_enb->capability = current_bss->bcn_prb->capability;
	p_bcn_enb->rxchannel = CHSPEC_CHANNEL(current_bss->chanspec);
	p_bcn_enb->aid = cfg->AID;
	p_bcn_enb->frame_del = ol->frame_del;

	ol->ol_flags |= OL_IE_NOTIFICATION_ENAB;

	bcopy(ol->ie_info.iemask, p_bcn_enb->iemask, IEMASK_SZ);
	bcopy(ol->ie_info.vndriemask, p_bcn_enb->vndriemask, sizeof(vndriemask_info) * MAX_VNDR_IE);

	/* Copy IEs */
	p_bcn_enb->iedatalen = ie_len;
	bcopy(((uint8 *)current_bss->bcn_prb)+ DOT11_BCN_PRB_LEN, &p_bcn_enb->iedata[0], ie_len);

	/* Send beacon offload enable message to CR4 */
	wlc_ol_msg_send(ol, (uint8 *)p_bcn_enb, sizeof(olmsg_bcn_enable) + ie_len);
	wlc_ol_rx_deferral(ol, OL_CFG_MASK, 0);
	MFREE(ol->osh, p_bcn_enb, sizeof(olmsg_bcn_enable) + ie_len);
	return err;
}

static int
wlc_ol_bcn_disable(wlc_ol_info_t *ol, wlc_bsscfg_t *cfg)
{
	olmsg_bcn_disable bcn_disable;
	int err = BCME_OK;

	if ((cfg != NULL) && (ol->ol_flags & OL_BCN_ENAB)) {
		bcn_disable.hdr.type = BCM_OL_BEACON_DISABLE;
		bcn_disable.hdr.seq = 0;
		bcn_disable.hdr.len = sizeof(olmsg_bcn_disable) - sizeof(olmsg_header);
		bcopy(&cfg->BSSID, &bcn_disable.BSSID, sizeof(struct ether_addr));

		wlc_ol_msg_send(ol, (uint8 *)&bcn_disable, sizeof(olmsg_bcn_disable));

		ol->ol_flags &= ~OL_BCN_ENAB;
		wlc_ol_rx_deferral(ol, OL_CFG_MASK, OL_CFG_MASK);
	}
	else {
		WL_TRACE(("%s: Beacon offload is not active\n", __FUNCTION__));
		err = BCME_ERROR;
	}
	return err;
}

bool wlc_ol_bcn_is_enable(wlc_ol_info_t *ol)
{
	if (ol && WLOFFLD_BCN_ENAB(ol->wlc->pub) &&
		(ol->ol_up) && (ol->ol_flags & OL_BCN_ENAB)) {
		return TRUE;
	} else {
		return FALSE;
	}
}

bool wlc_ol_time_since_bcn(wlc_ol_info_t *ol)
{
	wlc_info_t *wlc;
	uint16 bcncnt;
	bool ret = FALSE;

	if (!wlc_ol_bcn_is_enable(ol))
		return FALSE;
	wlc = ol->wlc;
	bcncnt = wlc_read_shm(wlc, M_UCODE_BSSBCNCNT);

	if (ol->bcnmbsscount != bcncnt)
		ret = TRUE;
	else {
		WL_ASSOC(("%s: M_UCODE_BSSBCNCNT not changed, still %d\n", __FUNCTION__, bcncnt));
		ret = FALSE;
	}

	ol->bcnmbsscount = bcncnt;

	return ret;
}

int
wlc_ol_arp_enable(wlc_ol_info_t *ol, wlc_bsscfg_t *cfg, struct ipv4_addr *host_ip)
{
	int err = BCME_OK;
	olmsg_arp_enable arp_enable;
	scb_t *scb;

	/* Check for valid state to enable arp offloads */
	if (!ol || !cfg || !BSSCFG_STA(cfg) || !cfg->BSS ||	!cfg->up ||
		cfg != ol->wlc->cfg || !cfg->associated || !ol->num_bsscfg_allow ||
		IPV4_ADDR_NULL(host_ip->addr) ||
		(ol->ol_flags & OL_ARP_ENAB) || !(ol->ol_up)) {
		WL_ERROR(("%s: Invalid params/state. Not enabling ARP offloads \n",
			__FUNCTION__));
		return BCME_BADARG;
	}

	scb = wlc_ol_get_scb(ol->wlc);
	if (!scb) {
		WL_ERROR(("wl%d: can't enable arp offloads - no scb\n", WLCWLUNIT(ol->wlc)));
		return BCME_NOTFOUND;
	}

	ol->ol_flags |= OL_ARP_ENAB;
	bzero(&arp_enable, sizeof(olmsg_arp_enable));
	arp_enable.hdr.type = BCM_OL_ARP_ENABLE;
	arp_enable.hdr.seq = 0;
	arp_enable.hdr.len = sizeof(olmsg_arp_enable) - sizeof(olmsg_header);
	bcopy(&cfg->cur_etheraddr, &arp_enable.host_mac,
		sizeof(struct ether_addr));
	bcopy(host_ip->addr, arp_enable.host_ip.addr, IPV4_ADDR_LEN);
	bcopy(host_ip->addr, ol->arp_info.host_ip.addr, IPV4_ADDR_LEN);

	bcopy(&cfg->BSSID, &arp_enable.BSSID, sizeof(struct ether_addr));
	wlc_ol_update_sec_info(ol, cfg, scb, &arp_enable.sec_info);

	wlc_ol_msg_send(ol, (uint8 *)&arp_enable, sizeof(olmsg_arp_enable));
	wlc_ol_rx_deferral(ol, OL_CFG_MASK, 0);
	if (!WSEC_ENABLED(cfg->wsec) ||
		(WSEC_ENABLED(cfg->wsec) && WLC_PORTOPEN(cfg)))
		wlc_ol_armtx(ol, TRUE);

	return err;
}

static int
wlc_ol_arp_disable(wlc_ol_info_t *ol)
{
	int err = BCME_OK;
	olmsg_arp_disable arp_disable;

	if (ol->ol_flags & OL_ARP_ENAB) {
		arp_disable.hdr.type = BCM_OL_ARP_DISABLE;
		arp_disable.hdr.seq = 0;
		arp_disable.hdr.len = sizeof(olmsg_arp_disable) - sizeof(olmsg_header);

		wlc_ol_msg_send(ol, (uint8 *)&arp_disable, sizeof(olmsg_arp_disable));

		ol->ol_flags &= ~OL_ARP_ENAB;
		wlc_ol_rx_deferral(ol, OL_CFG_MASK, OL_CFG_MASK);
		if (!OL_ARMTX_NEEDED(ol))
			wlc_ol_armtx(ol, FALSE);
	} else {
		WL_TRACE(("%s: ARP offload is not active\n", __FUNCTION__));
		err = BCME_ERROR;
	}

	return err;
}

#ifdef WLTCPKEEPA
/* if we have timer stuff set we send that first followed by the connection info */
static int
wlc_ol_tcp_keepalive(wlc_ol_info_t *ol)
{
	int err = BCME_OK;
	olmsg_tcp_keep_conn tcp_keep_conn_msg;
	olmsg_tcp_keep_timers tcp_keep_timers_msg;

	if (ol->tcp_keepalive_timers_tosend == 1) {
		tcp_keep_timers_msg.hdr.type = BCM_OL_TCP_KEEP_TIMERS;
		tcp_keep_timers_msg.hdr.seq = 0;
		tcp_keep_timers_msg.hdr.len = sizeof(olmsg_tcp_keep_timers) - sizeof(olmsg_header);
		bcopy(&ol->tcp_keepalive_timers, &tcp_keep_timers_msg.tcp_keepalive_timers,
		   sizeof(wl_mtcpkeep_alive_timers_pkt_t));
		wlc_ol_msg_send(ol, (uint8 *)&tcp_keep_timers_msg, sizeof(olmsg_tcp_keep_timers));
	}
	tcp_keep_conn_msg.hdr.type = BCM_OL_TCP_KEEP_CONN;
	tcp_keep_conn_msg.hdr.seq = 0;
	tcp_keep_conn_msg.hdr.len = sizeof(olmsg_tcp_keep_conn) - sizeof(olmsg_header);
	bcopy(&ol->tcp_keepalive_conn_rec, &tcp_keep_conn_msg.tcp_keepalive_conn,
	   sizeof(wl_mtcpkeep_alive_conn_pkt_t));

	wlc_ol_msg_send(ol, (uint8 *)&tcp_keep_conn_msg, sizeof(olmsg_tcp_keep_conn));

	return err;
}
#endif /* WLTCPKEEPA */

static int
wlc_ol_arp_setip(wlc_ol_info_t *ol, struct ipv4_addr *host_ip)
{
	int err = BCME_OK;
	olmsg_arp_setip arp_setip;

	if (ol->ol_flags & OL_ARP_ENAB) {
		arp_setip.hdr.type = BCM_OL_ARP_SETIP;
		arp_setip.hdr.seq = 0;
		arp_setip.hdr.len = sizeof(olmsg_arp_setip) - sizeof(olmsg_header);
		bcopy(host_ip, &(arp_setip.host_ip), sizeof(struct ipv4_addr));

		wlc_ol_msg_send(ol, (uint8 *)&arp_setip, sizeof(olmsg_arp_setip));
	}
	else {
		WL_TRACE(("%s: ARP offload is not active\n", __FUNCTION__));
		ASSERT(FALSE);
		err = BCME_ERROR;
	}

	return err;
}

int
wlc_ol_nd_enable(wlc_ol_info_t *ol, wlc_bsscfg_t *cfg)
{
	int err = BCME_OK;
	olmsg_nd_enable nd_enable;
	scb_t *scb;
	int i;

	/* Check for valid state to enable arp offloads */
	if (!ol || !cfg || !BSSCFG_STA(cfg) || !cfg->BSS || !cfg->up ||
		cfg != ol->wlc->cfg || !cfg->associated || !ol->num_bsscfg_allow ||
		(ol->ol_flags & OL_ND_ENAB) || !(ol->ol_up)) {
		WL_ERROR(("%s: Invalid params/state. Not enabling ND offloads \n",
			__FUNCTION__));
		return BCME_BADARG;
	}

	scb = wlc_ol_get_scb(ol->wlc);
	if (!scb) {
		WL_ERROR(("wl%d: can't enable arp offloads - no scb\n", WLCWLUNIT(ol->wlc)));
		return BCME_NOTFOUND;
	}

	ol->ol_flags |= OL_ND_ENAB;
	nd_enable.hdr.type = BCM_OL_ND_ENABLE;
	nd_enable.hdr.seq = 0;
	nd_enable.hdr.len = sizeof(olmsg_nd_enable) - sizeof(olmsg_header);

	bcopy(&cfg->cur_etheraddr, &nd_enable.host_mac, sizeof(struct ether_addr));
	bcopy(&cfg->BSSID, &nd_enable.BSSID, sizeof(struct ether_addr));
	wlc_ol_update_sec_info(ol, cfg, scb, &nd_enable.sec_info);

	wlc_ol_msg_send(ol, (uint8 *)&nd_enable, sizeof(olmsg_nd_enable));

	/* plumb down all the stored ipv6 address on first enable */
	for (i = 0; i < ND_MULTIHOMING_MAX; i++)
	{
		if (!IPV6_ADDR_NULL(ol->nd_info.host_ipv6[i].addr)) {
			wlc_ol_nd_setip(ol, &(ol->nd_info.host_ipv6[i]));
		} else {
			break;
		}
	}

	wlc_ol_rx_deferral(ol, OL_CFG_MASK, 0);
	if (!WSEC_ENABLED(cfg->wsec) ||
		((WSEC_ENABLED(cfg->wsec)) && (WLC_PORTOPEN(cfg))))
		wlc_ol_armtx(ol, TRUE);

	return err;
}

#ifdef BONJOUR
/* Copy mdns database into the shared ARM/host space */
int
wlc_mdns_copydata(wlc_ol_info_t *ol, uint8 *mdns_offload_data, uint32 mdns_offload_len)
{
	if (!ol || !ol->shared_info) {
		WL_ERROR(("%s: Offloads not supported or not up\n", __FUNCTION__));
		return BCME_NOTREADY;
	}

	WL_TRACE(("Enter %s\n", __FUNCTION__));
	if (!mdns_offload_len) {
		WL_ERROR(("%s: glob_len not ready yet\n", __FUNCTION__));
		return BCME_BADARG;
	}
	if (*(int32 *)&mdns_offload_data[0] != 0x424A5030) {    /* Bon Jour Proxy 0 "BJP0" */
		WL_ERROR(("%s: Signature does not match!, len = %d\n",
			__FUNCTION__, mdns_offload_len));
		return BCME_BADARG;
	}
	WL_TRACE(("%s: Valid mDNS Signature, len = %d\n", __FUNCTION__,  mdns_offload_len));

	ol->shared_info->mdns_dbase[0] = 0;

	bcopy(mdns_offload_data, (void *)ol->shared_info->mdns_dbase, mdns_offload_len);
	ol->shared_info->mdns_len = mdns_offload_len;

	return BCME_OK;
}
#endif /* BONJOUR */

static int
wlc_ol_nd_disable(wlc_ol_info_t *ol)
{
	int err = BCME_OK;
	olmsg_nd_disable nd_disable;

	if (ol->ol_flags & OL_ND_ENAB) {
		nd_disable.hdr.type = BCM_OL_ND_DISABLE;
		nd_disable.hdr.seq = 0;
		nd_disable.hdr.len = sizeof(olmsg_nd_disable) - sizeof(olmsg_header);

		wlc_ol_msg_send(ol, (uint8 *)&nd_disable, sizeof(olmsg_nd_disable));

		ol->ol_flags &= ~OL_ND_ENAB;
		wlc_ol_rx_deferral(ol, OL_CFG_MASK, OL_CFG_MASK);
		if (!OL_ARMTX_NEEDED(ol))
			wlc_ol_armtx(ol, FALSE);
	} else {
		WL_TRACE(("%s: ND offload is not active\n", __FUNCTION__));
		err = BCME_ERROR;
	}

	return err;
}

static int
wlc_ol_nd_setip(wlc_ol_info_t *ol, struct ipv6_addr *host_ip)
{
	int err = BCME_OK;
	olmsg_nd_setip nd_setip;

	if (ol->ol_flags & OL_ND_ENAB) {
		nd_setip.hdr.type = BCM_OL_ND_SETIP;
		nd_setip.hdr.seq = 0;
		nd_setip.hdr.len = sizeof(olmsg_nd_setip) - sizeof(olmsg_header);
		bcopy(host_ip, &(nd_setip.host_ip), sizeof(struct ipv6_addr));
		wlc_ol_msg_send(ol, (uint8 *)&nd_setip, sizeof(olmsg_nd_setip));
	}
	else {
		WL_TRACE(("%s: ARP offload is not active\n", __FUNCTION__));
		ASSERT(FALSE);
		err = BCME_ERROR;
	}

	return err;
}

int
wlc_ol_pkt_filter_enable(wlc_ol_info_t *ol, wlc_bsscfg_t *cfg)
{
	int err = BCME_OK;
	olmsg_pkt_filter_enable pkt_filter_enable;

	/* Check for valid state to enable pkt filters.
	 * Note: This is called from wlc_wowl_enable and cfg->up is not 1.
	 */
	if (!ol || !cfg || !BSSCFG_STA(cfg) || !cfg->BSS ||
	    cfg != ol->wlc->cfg || !(ol->ol_up)) {
		WL_ERROR(("%s: Invalid params/state. Not enabling pkt filters\n",
			__FUNCTION__));
		return BCME_BADARG;
	}

	ol->ol_flags |= OL_PKT_FILTER_ENAB;

	bzero(&pkt_filter_enable, sizeof(olmsg_pkt_filter_enable));

	pkt_filter_enable.hdr.type = BCM_OL_PKT_FILTER_ENABLE;
	pkt_filter_enable.hdr.seq = 0;
	pkt_filter_enable.hdr.len = sizeof(olmsg_pkt_filter_enable) - sizeof(olmsg_header);

	bcopy(&cfg->cur_etheraddr, &pkt_filter_enable.host_mac, sizeof(struct ether_addr));

	wlc_ol_msg_send(ol, (uint8 *)&pkt_filter_enable, sizeof(olmsg_pkt_filter_enable));

	return err;
}

int
wlc_ol_pkt_filter_add(wlc_ol_info_t *ol, wl_wowl_pattern_t *wowl_pattern, uint wowl_pattern_len)
{
	int			err = BCME_OK;
	int32			i;
	olmsg_pkt_filter_add    *pkt_filter_add;
	wl_pkt_filter_t         *pkt_filter;
	wl_pkt_filter_pattern_t *pkt_pattern;
	uint16                   msg_len;
	int32			 size_bytes;
	uint8                    *src, *dst;

	/* Sanity check the parameters */
	if (wowl_pattern == NULL) {
		return BCME_BADARG;
	}

	/*
	 * We will convert the filter pattern from the WoWL format to the
	 * wlc_pkt_filter format. The WoWL format is based on the NDIS format
	 * for patterns of type WOL_BITMAP_PATTERN described at
	 * http://msdn.microsoft.com/en-us/library/windows/hardware/ff566768%28v=vs.85%29.aspx.
	 *
	 * First, set the size_bytes equal to minimum of pattern->masksize * 8
	 * and pattern size i.e. the pktfilter pattern is trimmed when
	 * masksize and patternsize of wowl filter do not match
	 * (masksize * 8 != patternsize)
	 */
	size_bytes = MIN((wowl_pattern->masksize * 8), wowl_pattern->patternsize);

	msg_len =
	    (uint16)(sizeof(olmsg_header) + WL_PKT_FILTER_FIXED_LEN +
		WL_PKT_FILTER_PATTERN_FIXED_LEN + (2 * size_bytes));

	if (ol->ol_flags & OL_PKT_FILTER_ENAB) {
		pkt_filter_add = (olmsg_pkt_filter_add *)MALLOCZ(ol->osh, msg_len);
		if (pkt_filter_add == NULL) {
			WL_ERROR(("%s: out of mem, malloced %d bytes\n",
			    __FUNCTION__, MALLOCED(ol->osh)));

			err = BCME_ERROR;
		} else {
			pkt_filter_add->hdr.type = BCM_OL_PKT_FILTER_ADD;
			pkt_filter_add->hdr.seq  = 0;
			pkt_filter_add->hdr.len  = msg_len - sizeof(olmsg_header);

			/* Copy over the applicable data from wowl format to pkt_filter format. */
			pkt_filter = &pkt_filter_add->pkt_filter;

			pkt_filter->id           = wowl_pattern->id;
			pkt_filter->negate_match = FALSE;

			switch (wowl_pattern->type) {
				case wowl_pattern_type_bitmap:
					pkt_filter->type = WL_PKT_FILTER_TYPE_PATTERN_MATCH;

					break;

				case wowl_pattern_type_arp:
					pkt_filter->type = WL_PKT_FILTER_TYPE_PATTERN_MATCH;
					ASSERT(FALSE);

					return BCME_ERROR;

				case wowl_pattern_type_na:
					pkt_filter->type = WL_PKT_FILTER_TYPE_PATTERN_MATCH;
					ASSERT(FALSE);

					return BCME_ERROR;

				default:
					WL_TRACE(("%s: Invalid pattern type = %d\n",
					    __FUNCTION__, wowl_pattern->type));

					ASSERT(FALSE);

					return BCME_ERROR;
			}

			pkt_pattern = &pkt_filter->u.pattern;

			pkt_pattern->offset	= wowl_pattern->offset;
			pkt_pattern->size_bytes = size_bytes;

			/*
			 * Expand the mask - if a bit is set correspoding pattern
			 * byte is compared
			 */
			src = ((uint8*)wowl_pattern + sizeof(wl_wowl_pattern_t));
			dst = ((uint8*)&pkt_pattern->mask_and_pattern[0]);

			for (i = 0; i < size_bytes; i++) {
			    dst[i] = (isset(src, i) ? 0xff:0);
			}

			/* Copy the pattern */
			src = ((uint8*)wowl_pattern + wowl_pattern->patternoffset);
			dst = (uint8 *)&pkt_pattern->mask_and_pattern[size_bytes];
			bcopy(src, dst, size_bytes);

			/* All done */
			wlc_ol_msg_send(ol, (uint8 *)pkt_filter_add, msg_len);

			MFREE(ol->osh, pkt_filter_add, msg_len);
		}

	}
	else {
		WL_TRACE(("%s: Packet filter offload is not active\n", __FUNCTION__));
		ASSERT(FALSE);

		err = BCME_ERROR;
	}

	return err;
}

static int
wlc_ol_pkt_filter_disable(wlc_ol_info_t *ol)
{
	int err = BCME_OK;
	olmsg_pkt_filter_disable pkt_filter_disable;

	if (ol->ol_flags & OL_PKT_FILTER_ENAB) {
		pkt_filter_disable.hdr.type = BCM_OL_PKT_FILTER_DISABLE;
		pkt_filter_disable.hdr.seq = 0;
		pkt_filter_disable.hdr.len = sizeof(olmsg_pkt_filter_disable) -
			sizeof(olmsg_header);

		wlc_ol_msg_send(
			ol,
			(uint8 *)&pkt_filter_disable,
			sizeof(olmsg_pkt_filter_disable));

		ol->ol_flags &= ~OL_PKT_FILTER_ENAB;
	} else {
		WL_TRACE(("%s: Packet filter offload is not active\n", __FUNCTION__));
		err = BCME_ERROR;
	}

	return err;
}

/* Send notification of the start of enabling wowl to ARM */
int
wlc_ol_wowl_enable_start(
	wlc_ol_info_t *ol,
	wlc_bsscfg_t *cfg,
	olmsg_wowl_enable_start *wowl_enable_start,
	uint wowl_enable_len)
{
	int err = BCME_OK;

	ASSERT(wowl_enable_len == sizeof(olmsg_wowl_enable_start));

	/* Check for valid state to start enabling wowl offloads.
	 * Note: This is called from wlc_wowl_enable and cfg->up is not 1.
	 */
	if (!ol || !ol->ol_up || !cfg || !BSSCFG_STA(cfg) ||
	    (!cfg->BSS && wowl_enable_start->wowl_cfg.wowl_test == 0) ||
	    (ol->ol_flags & OL_WOWL_ENAB)) {
		WL_ERROR((
			"%s: Invalid params/state. Not starting the enable of wowl offloads \n",
			__FUNCTION__));
		return BCME_BADARG;
	}

	ol->ol_flags |= OL_WOWL_ENAB;

	wowl_enable_start->hdr.type = BCM_OL_WOWL_ENABLE_START;
	wowl_enable_start->hdr.seq = 0;
	wowl_enable_start->hdr.len = sizeof(olmsg_wowl_enable_start) - sizeof(olmsg_header);

	wlc_ol_msg_send(ol, (uint8 *)wowl_enable_start, sizeof(olmsg_wowl_enable_start));

	return err;
}

/* Send notification of the completion of enabling wowl to ARM */
int
wlc_ol_wowl_enable_complete(wlc_ol_info_t *ol)
{
	int			    err = BCME_OK;
	olmsg_wowl_enable_complete  wowl_enable_cplt;

	/* Check for valid state to completing the wowl offloads.
	 * Note: This must be called when wowl has been enabled.
	 */
	if (!ol || !ol->ol_up || !(ol->ol_flags & OL_WOWL_ENAB)) {
		WL_ERROR((
			"%s: Invalid params/state. Not completing the enable of wowl offloads \n",
			__FUNCTION__));
		return BCME_BADARG;
	}

	wowl_enable_cplt.hdr.type = BCM_OL_WOWL_ENABLE_COMPLETE;
	wowl_enable_cplt.hdr.seq = 0;
	wowl_enable_cplt.hdr.len = sizeof(olmsg_wowl_enable_complete) - sizeof(olmsg_header);

	wlc_ol_msg_send(ol, (uint8 *)&wowl_enable_cplt, sizeof(olmsg_wowl_enable_complete));

	/* Clear rx deferral when completing WoWL mode */
	wlc_ol_rx_deferral(ol, OL_CFG_MASK, OL_CFG_MASK);

	return err;
}

/* Receive notification of the wowl configuration completion from ARM */
static void wlc_ol_wowl_enable_completed(wlc_ol_info_t *ol)
{
	ASSERT(ol->ol_flags & OL_WOWL_ENAB);

	if (!(ol->ol_flags & OL_WOWL_ENAB)) {
	    return;
	}

#ifdef WOWL
	wlc_wowl_enable_completed(ol->wlc->wowl);
#endif /* WOWL */
}

/* ARM Event String must match to enum defined in bcm_ol_msg.h */
const char *arm_event_str[BCM_OL_E_MAX] =  {
	"WOWL_START",		/* BCM_OL_E_WOWL_START */
	"WOWL_COMPLETE",	/* BCM_OL_E_WOWL_COMPLETE */
	"TIME_SINCE_BCN",	/* BCM_OL_E_TIME_SINCE_BCN */
	"BCN_LOSS",		/* BCM_OL_E_BCN_LOSS */
	"DEAUTH",		/* BCM_OL_E_DEAUTH */
	"DISASSOC",		/* BCM_OL_E_DISASSOC */
	"RETROGRADE_TSF",	/* BCM_OL_E_RETROGRADE_TSF */
	"RADIO_DISABLED",	/* BCM_OL_E_RADIO_HW_DISABLED */
	"PME",			/* BCM_OL_E_PME_ASSERTED */
	"UNASSOC",		/* BCM_OL_E_UNASSOC */
	"SCAN_BEGIN",		/* BCM_OL_E_SCAN_BEGIN */
	"SCAN_END",		/* BCM_OL_E_SCAN_END */
	"PREFSSID",		/* BCM_OL_E_PREFSSID_FOUND */
	"CSA"			/* BCM_OL_E_CSA */
};

#define BCM_OL_EVENT_GET_MESSAGESTRING(_x)\
	(((unsigned int)(_x) < BCM_OL_E_MAX)?arm_event_str[(unsigned int)(_x)] : "UNKNOWN")

int
wlc_ol_wowl_disable(wlc_ol_info_t *ol)
{
	int err = BCME_OK;

	/* Validate that share info is still valid */
	wlc_ol_validate_shared_info(ol);

	if (ol->ol_flags & OL_WOWL_ENAB) {
		/*
		 * Process wowl host info returned from ARM and any additional tasks needed
		 * to complete resume to D0.
		 */
		if (ol->shared_info) {
			uint32 idx, i;
			char buffer[512];
			int cStringSize = 0;
			uint8 *ptr;
			bool scan_begin = FALSE;

			wlc_wowl_disable_completed(ol->wlc->wowl,
				(wowl_host_info_t *)(&ol->shared_info->wowl_host_info));

			/* get the ARM events */
			bzero(ol->eventlog, MAX_OL_EVENTS);
			bzero(buffer, sizeof(buffer));
			idx = ol->shared_info->wowl_host_info.eventidx & (MAX_OL_EVENTS - 1);
			ptr = (uint8 *)ol->shared_info->wowl_host_info.eventlog;

			cStringSize = snprintf(&buffer[0], sizeof(buffer),
				"%s: ARM Event Log:", __FUNCTION__);
			for (i = 0; i < MAX_OL_EVENTS; i++) {
				ol->eventlog[i] = ptr[idx];

				/* Concatenate strings */
				cStringSize += snprintf(&buffer[cStringSize],
					(sizeof(buffer)-cStringSize), "  '%s'",
					BCM_OL_EVENT_GET_MESSAGESTRING(ol->eventlog[i]));

				/* Buffer overflow check */
				if (cStringSize >= sizeof(buffer))
					break;

				if (ol->eventlog[i] == BCM_OL_E_SCAN_BEGIN)
					scan_begin = TRUE;
				else if (ol->eventlog[i] == BCM_OL_E_WOWL_START)
					/* terminate print at WOWL_START events */
					break;
				idx = (idx - 1) & (MAX_OL_EVENTS - 1);
			}
			WL_ERROR(("%s\n", buffer));

			if (scan_begin) {
				cStringSize = snprintf(&buffer[0], sizeof(buffer),
						       "ARPT Scan Reason:");

				for (i = 0; i < MAX_OL_EVENTS; i++) {
					/* Buffer overflow check */
					if (cStringSize >= sizeof(buffer))
						break;

					switch (ol->eventlog[i]) {
						case BCM_OL_E_BCN_LOSS:
						case BCM_OL_E_DEAUTH:
						case BCM_OL_E_DISASSOC:
						case BCM_OL_E_CSA:
							/* Concatenate strings */
							cStringSize +=
							snprintf(&buffer[cStringSize],
							(sizeof(buffer)-cStringSize),
							" %s",
							BCM_OL_EVENT_GET_MESSAGESTRING(
								ol->eventlog[i]));
							break;
						default:
							break;
					}
				}
				WL_ERROR(("%s\n", buffer));
			}

			/* get the current console output */
			wlc_ol_wowl_get_cons_data(ol);
		}

		/* Enable rx deferral when exiting WoWL mode */
		wlc_ol_rx_deferral(ol, OL_CFG_MASK, 0);

		ol->ol_flags &= ~OL_WOWL_ENAB;
	} else {
		WL_TRACE(("%s: Wowl offload is not active\n", __FUNCTION__));
		err = BCME_ERROR;
	}

	return err;
}

int8 wlc_ol_noise_get_value(wlc_ol_info_t *ol)
{
	wlc_bsscfg_t *cfg;

	if (ol == NULL) {
		return 0;
	}
	if (!ol->ol_up) {
		return 0;
	}

	cfg = ol->wlc->cfg;

	if ((!WLOFFLD_BCN_ENAB(ol->wlc->pub)) || !cfg || !BSSCFG_STA(cfg) ||
		(!cfg->BSS) || (!cfg->up)) {
		return 0;
	}

	return (ol->shared_info->rssi_info.noise_avg);
}

int8 wlc_ol_rssi_get_value(wlc_ol_info_t *ol)
{
	olmsg_shared_info_t *shared_info = NULL;
	wlc_bsscfg_t	*cfg = NULL;

	if (ol == NULL) {
		return WLC_RSSI_INVALID;
	}
	if (!ol->ol_up) {
		return WLC_RSSI_INVALID;
	}

	cfg = ol->wlc->cfg;

	if ((!WLOFFLD_BCN_ENAB(ol->wlc->pub)) || !cfg || !BSSCFG_STA(cfg) ||
		(!cfg->BSS) || (!cfg->up)) {
		return WLC_RSSI_INVALID;
	}

	shared_info = ol->shared_info;

	wlc_ol_inc_rssi_cnt_arm(ol);
	return (shared_info->rssi_info.rssi); /* Read SHM */
}

int8 wlc_ol_rssi_get_ant(wlc_ol_info_t *ol, uint32 ant_idx)
{
	olmsg_shared_info_t *shared_info = NULL;
	wlc_bsscfg_t	*cfg = NULL;

	if ((ol == NULL) || (ant_idx >= WL_RSSI_ANT_MAX)) {
		return WLC_RSSI_INVALID;
	}
	if (!ol->ol_up) {
		return WLC_RSSI_INVALID;
	}

	cfg = ol->wlc->cfg;

	if ((!WLOFFLD_BCN_ENAB(ol->wlc->pub)) || !cfg || !BSSCFG_STA(cfg) ||
		(!cfg->BSS) || (!cfg->up)) {
		return WLC_RSSI_INVALID;
	}

	shared_info = ol->shared_info;

	return (shared_info->rssi_info.rxpwr[ant_idx]); /* Read SHM */
}

static bool wlc_ol_check_rssi_param_changes(olmsg_rssi_init *r, olmsg_rssi_init *rp, bool force)
{
	int i;
	bool	ret = force;

	if (r->enabled != rp->enabled) {
		rp->enabled = r->enabled;
		ret = TRUE;
	}

	if (r->low_threshold != rp->low_threshold) {
		rp->low_threshold = r->low_threshold;
		ret = TRUE;
	}

	if (r->roam_off != rp->roam_off) {
		rp->roam_off = r->roam_off;
		ret = TRUE;
	}

	if (r->mode != rp->mode) {
		rp->mode = r->mode;
		ret = TRUE;
	}

	if (r->phyrxchain != rp->phyrxchain) {
		rp->phyrxchain = r->phyrxchain;
		ret = TRUE;
	}

	if (ABS(r->current_temp - rp->current_temp) > WLC_DYNAMIC_TEMPSENSE_DELTA_TEMP) {
		rp->current_temp = r->current_temp;
		ret = TRUE;
	}

	if (r->radio_chanspec != rp->radio_chanspec) {
		rp->radio_chanspec = r->radio_chanspec;
		ret = TRUE;
	}

	for (i = 0; i < WL_RSSI_ANT_MAX; i++) {
	    if (r->phy_rssi_gain_error[i] != rp->phy_rssi_gain_error[i]) {
		rp->phy_rssi_gain_error[i] = r->phy_rssi_gain_error[i];
		ret = TRUE;
	    }
	}
	return ret;
}

void wlc_ol_rssi_init_values(wlc_ol_info_t *ol, bool force)
{
	olmsg_rssi_init rssi_init;
	int i;
	phy_info_t *pi;
	wlc_bsscfg_t *cfg;

	if (!ol) {
		return;
	}

	cfg = ol->wlc->cfg;

	if ((!WLOFFLD_BCN_ENAB(ol->wlc->pub)) || !cfg || !BSSCFG_STA(cfg) ||
		(!cfg->BSS) || (!cfg->up)) {
		return;
	}

	if (AS_IN_PROGRESS(ol->wlc) || SCAN_IN_PROGRESS(ol->wlc->scan)) {
		return;
	}

	pi = (phy_info_t *)ol->wlc->hw->band->pi;

	rssi_init.hdr.type = BCM_OL_RSSI_INIT;
	rssi_init.hdr.seq = 0;
	rssi_init.hdr.len = sizeof(olmsg_rssi_init) - sizeof(olmsg_header);

	rssi_init.enabled = 1;
	rssi_init.low_threshold = (int8)ol->wlc->band->roam_trigger;
	rssi_init.roam_off = ol->wlc->cfg->roam->off;
	rssi_init.mode = pi->sh->rssi_mode;
	rssi_init.phyrxchain = pi->sh->phyrxchain;
	rssi_init.current_temp = pi->u.pi_acphy->current_temperature;
	rssi_init.raw_tempsense = pi->srom_rawtempsense;
	rssi_init.radio_chanspec = (uint16) pi->radio_chanspec;
	for (i = 0; i < PHY_CORE_MAX; i++) {
		rssi_init.phy_rssi_gain_error[i] = pi->phy_rssi_gain_error[i];
	}

	if (wlc_ol_check_rssi_param_changes(&rssi_init, &rssi_init_prev, force)) {
		/* there is change so send new params */
		WL_TRACE(("%s: Setting RSSI Parameters (Threshold: %d)\n",
			__FUNCTION__, ol->wlc->band->roam_trigger));
		wlc_ol_msg_send(ol, (uint8 *)&rssi_init, sizeof(olmsg_rssi_init));
	}
}

uint16
wlc_ol_get_state(wlc_ol_info_t *ol)
{
	uint16  flags = 0;
	if (ol && (ol->ol_up)) {
		flags = (uint16) ol->ol_flags;
	}
	return flags;
}

void wlc_ol_enable(wlc_ol_info_t *ol, wlc_bsscfg_t *cfg)
{

	/* Check for valid state to enable beacon offloads */
	if (!ol || !cfg || !BSSCFG_STA(cfg) || !cfg->BSS || !cfg->up ||
		cfg != ol->wlc->cfg || !cfg->associated || !ol->num_bsscfg_allow ||
		!(ol->ol_up)) {
		WL_ERROR(("%s: Invalid params/state. Not enabling any offloads \n",
			__FUNCTION__));
		return;
	}

	if (WLOFFLD_BCN_ENAB(ol->wlc->pub)) {
		wlc_ol_bcn_enable(ol, cfg);
		bzero(&rssi_init_prev, sizeof(rssi_init_prev)); /* Reset values */
		wlc_ol_rssi_init_values(ol, TRUE);
	}

	if (WLOFFLD_ARP_ENAB(ol->wlc->pub))
		wlc_ol_arp_enable(ol, cfg, &(ol->arp_info.host_ip));

	if (WLOFFLD_ND_ENAB(ol->wlc->pub)) {

		/* pass on the ns ip stored */
		wlc_ol_nd_enable(ol, cfg);
	}
}

void wlc_ol_disable(wlc_ol_info_t *ol, wlc_bsscfg_t *cfg)
{

	if (!ol || !cfg || !(ol->ol_up)) {
		WL_ERROR(("%s: Invalid state. Not disabling any offloads \n",
			__FUNCTION__));
		return;
	}

#ifdef WOWL
	/* Keep offloads enabled for wowl mode */
	if (WOWL_ACTIVE(ol->wlc->pub)) {
		WL_ERROR(("%s: wowl active, so return\n", __FUNCTION__));
		return;
	}
#endif // endif

	if (WLOFFLD_BCN_ENAB(ol->wlc->pub)) {
		wlc_ol_bcn_disable(ol, cfg);
	}

	if (WLOFFLD_ARP_ENAB(ol->wlc->pub))
		wlc_ol_arp_disable(ol);

	if (WLOFFLD_ND_ENAB(ol->wlc->pub))
		wlc_ol_nd_disable(ol);

	ol->ol_arm_txenable = FALSE;
}

bool wlc_ol_chkintstatus(wlc_ol_info_t *ol)
{
	if (ol->mb_intstatus)
		return TRUE;
	else
		return FALSE;

}

static ratespec_t
wlc_ol_preferred_rspec(wlc_ol_info_t *ol, wlc_rateset_t *rs)
{
	uint i = 0;
	uint8 r;
	ratespec_t rspec;

	for (i = 0; i < rs->count; i++) {
		r = rs->rates[i] & RATE_MASK;
		if (r == WLC_RATE_6M) {
			rspec = LEGACY_RSPEC(r);
			return rspec;
		}
	}
	rspec = LEGACY_RSPEC(rs->rates[0] & RATE_MASK);
	return rspec;
}

void wlc_ol_key_update(wlc_info_t *wlc, ol_key_info *dkey, wlc_key_t *skey,
	wlc_key_info_t *key_info)
{
	int i;
	int retval;

	/* Offloads can today support only 4 RXIVs. */
	STATIC_ASSERT(WLC_KEY_BASE_RX_SEQ == 4);

	ASSERT(skey && dkey && key_info);

	retval = BCME_BADARG;
	memset(dkey, 0, sizeof(ol_key_info));

	dkey->info = *key_info;
	dkey->hw_idx = wlc_key_get_hw_idx(skey);

	if (key_info->algo == CRYPTO_ALGO_OFF)
		goto done;

	retval = wlc_key_get_data(skey, dkey->data, sizeof(dkey->data), NULL);
	if (retval < 0)
		WL_ERROR(("%s: wlc_key_get_data returned %d\n", __FUNCTION__, retval));

	for (i = 0; i < WLC_KEY_BASE_RX_SEQ; i++) {
		retval = wlc_key_get_seq(skey, dkey->rxiv[i].buf,
			sizeof(dkey->rxiv[i].buf), (wlc_key_seq_id_t)i, FALSE);
		if (retval != sizeof(dkey->rxiv[i].buf))
			WL_ERROR(("%s: err %d from wlc_key_get_seq rx iv %d\n",
				__FUNCTION__, retval, i));
	}

	retval = wlc_key_get_seq(skey, dkey->txiv.buf, sizeof(dkey->txiv.buf), 0, TRUE);
	if (retval != sizeof(dkey->txiv.buf))
		WL_ERROR(("%s: err %d from wlc_key_get_seq tx\n", __FUNCTION__, retval));

done:
	WL_INFORM(("wl%d: %s: algo %d, key id %d, iv_len:%d\n", WLCWLUNIT(wlc),
		__FUNCTION__, dkey->info.algo, dkey->info.key_id, dkey->info.iv_len));
}

bool wlc_ol_arp_nd_enabled(wlc_ol_info_t *ol)
{
	if (ol && ol->ol_up &&
		((ol->ol_flags & OL_ARP_ENAB) || (ol->ol_flags & OL_ND_ENAB)))
		return TRUE;
	else
		return FALSE;
}

static struct scb * wlc_ol_get_scb(wlc_info_t *wlc)
{
	wlc_bsscfg_t *cfg;
	struct scb *scb;
	struct scb_iter scbiter;

	cfg = wlc->cfg;
	scb = wlc_scbfind(wlc, cfg, &cfg->BSSID);

	if (!scb) {
		/* band may have changed during scan */
		FOREACH_BSS_SCB(wlc->scbstate, &scbiter, cfg, scb) {
			if (bcmp(&scb->ea, &cfg->BSSID, ETHER_ADDR_LEN) == 0)
				break;
		}
	}

	return scb;
}

void wlc_ol_update_sec_info(wlc_ol_info_t *ol, wlc_bsscfg_t *cfg,
	scb_t *scb, ol_sec_info *info)
{
	wlc_info_t *wlc = ol->wlc;
	wlc_key_t *key;
	wlc_key_info_t key_info;

	info->flags = 0;
	info->flags |= SCB_QOS(scb) ? OL_SEC_F_QOS : 0;
	info->flags |= !cfg->BSS ? OL_SEC_F_IBSS : 0;
	info->flags |= WLEXSTA_ENAB(wlc->pub) ? OL_SEC_F_WIN7PLUS : 0;
	info->flags |= OL_SEC_F_WAKEON1MICERR;
	info->flags |= (scb->flags & SCB_LEGACY_AES) ? OL_SEC_F_LEGACY_AES : 0;

#ifdef MFP
	info->flags |= SCB_MFP(scb) ? OL_SEC_F_MFP : 0;
#endif /* MFP */

	info->wsec = cfg->wsec;
	info->WPA_auth = cfg->WPA_auth;
	info->wsec_restrict = cfg->wsec_restrict;
	info->bss_tx_key_id = wlc_keymgmt_get_bss_tx_key_id(wlc->keymgmt, cfg, FALSE);

	key = wlc_keymgmt_get_scb_key(wlc->keymgmt, scb, WLC_KEY_ID_PAIRWISE,
		WLC_KEY_FLAG_NONE, &key_info);
	wlc_ol_key_update(wlc, &info->scb_key_info, key, &key_info);

	wlc_ol_update_group_keys(wlc, &info->bss_key_info[0], FALSE);
#ifdef MFP
	if (SCB_MFP(scb))
		wlc_ol_update_group_keys(wlc, &info->igtk_key_info[0], TRUE);
#endif // endif
}

void
wlc_ol_armtx(wlc_ol_info_t *ol, uint8 txEnable)
{
	wlc_info_t *wlc;
	struct scb *scb = NULL;
	olmsg_armtx armtx_msg;
	wlc_bsscfg_t *cfg;
	ratespec_t rspec;
	uint16 phyctl;

	if (!ol || !ol->wlc || !ol->wlc->cfg || !ol->ol_up) {
		WL_ERROR(("wl%d: %s: ignoring request in invalid state.\n",
			((ol && ol->wlc) ? WLCWLUNIT(ol->wlc) : 0), __FUNCTION__));
		return;
	}

	wlc = ol->wlc;
	cfg = wlc->cfg;

	WL_TRACE(("wlc_ol_armtx ps %d assoc state = %d\n", txEnable,
		cfg->assoc->state));

	if (!wlc_bss_connected(cfg))
		return;

	scb = wlc_ol_get_scb(wlc);
	if (!scb) {
		WL_ERROR(("wl%d: %s: error enabling; no scb; arm tx %d->%d \n",
			WLCWLUNIT(wlc), __FUNCTION__, ol->ol_arm_txenable, txEnable));
		return;
	}

	WL_TRACE(("%s: ARM tx enable:  current %d, new %d\n",
		__FUNCTION__, ol->ol_arm_txenable, txEnable));
	bzero(&armtx_msg, sizeof(olmsg_armtx));
	armtx_msg.hdr.type = BCM_OL_ARM_TX;
	armtx_msg.hdr.seq = 0;
	armtx_msg.hdr.len = sizeof(olmsg_armtx) - sizeof(olmsg_header);
	armtx_msg.TX = txEnable;

	/* Fill security related data */
	if (txEnable) {
		txpwr204080_t txpwrs;
		uint8 bw_idx;
		int rctr_len;
		int err;

		bcopy(&cfg->BSSID, &armtx_msg.txinfo.BSSID, sizeof(struct ether_addr));
		bcopy(&cfg->cur_etheraddr, &armtx_msg.txinfo.cur_etheraddr,
			sizeof(struct ether_addr));
		wlc_ol_update_sec_info(ol, cfg, scb, &armtx_msg.txinfo.sec_info);

		rspec = wlc_ol_preferred_rspec(ol, &cfg->current_bss->rateset);

		/* Legacy OFDM support DUP, CCK can only use BW 20MHz */
		if (CHSPEC_IS20(ol->wlc->chanspec) || IS_CCK(rspec))
			rspec |= RSPEC_BW_20MHZ;
		else if (CHSPEC_IS40(ol->wlc->chanspec))
			rspec |= RSPEC_BW_40MHZ;
		else if (CHSPEC_IS80(ol->wlc->chanspec))
			rspec |= RSPEC_BW_80MHZ;
		else if (CHSPEC_IS8080(ol->wlc->chanspec))
			rspec |= RSPEC_BW_160MHZ;
		else if (CHSPEC_IS160(ol->wlc->chanspec))
			rspec |= RSPEC_BW_160MHZ;
		else
			ASSERT(0);

		if (wlc_stf_get_204080_pwrs(wlc, rspec, &txpwrs) != BCME_OK) {
			ASSERT(!"phyctl1 ppr returns error!");
		}

		armtx_msg.txinfo.rate = (uint16)rspec;
		armtx_msg.txinfo.chanspec = ol->wlc->chanspec;
		armtx_msg.txinfo.aid = cfg->AID;

		/* PhyTxControlWord_0 */
		phyctl = wlc_acphy_txctl0_calc_ex(wlc, rspec, WLC_MM_PREAMBLE);
		armtx_msg.txinfo.PhyTxControlWord_0 = htol16(phyctl);

		/* PhyTxControlWord_1 */
		bw_idx = ((rspec&RSPEC_BW_MASK) >> RSPEC_BW_SHIFT) - BW_20MHZ;
		phyctl = wlc_acphy_txctl1_calc_ex(wlc, rspec, 0, txpwrs.pbw[bw_idx][TXBF_OFF_IDX]);
		armtx_msg.txinfo.PhyTxControlWord_1 = htol16(phyctl);

		/* PhyTxControlWord_2 */
		phyctl = wlc_acphy_txctl2_calc_ex(wlc, rspec, 0);
		armtx_msg.txinfo.PhyTxControlWord_2 = htol16(phyctl);

		rctr_len = sizeof(armtx_msg.txinfo.replay_counter);
#ifdef WOWL_OS_OFFLOADS
		err = wlc_wowl_get_replay_counter(wlc->wowl,
			armtx_msg.txinfo.replay_counter, &rctr_len);
		if (err != BCME_OK)
			WL_ERROR(("wl%d: %s: error %d getting replay counter for wowl\n",
				WLCWLUNIT(wlc), __FUNCTION__, err));
#endif /* WOWL_OS_OFFLOADS */
	}

	ol->ol_arm_txenable = txEnable;
	wlc_ol_msg_send(ol, (uint8 *)&armtx_msg, sizeof(olmsg_armtx));
	wlc_keymgmt_notify(wlc->keymgmt, WLC_KEYMGMT_NOTIF_OFFLOAD, cfg, scb, NULL, NULL);
}

static int
wlc_notification_set_id(wlc_ol_info_t *ol, wlc_bsscfg_t *cfg, int id, bool enable)
{
	int err = BCME_OK;
	olmsg_ie_notification_enable  p_ie_notification;
	/* wlc_info_t *wlc = NULL; */
	/* wlc_bss_info_t *current_bss = NULL; */

	/* Check for valid state to enable beacon offloads */
	if (!ol || !cfg || !BSSCFG_STA(cfg) || !cfg->BSS) {
		WL_ERROR(("%s: Invalid params/state."
			" Not enabling beacon IE notification offloads\n",
			__FUNCTION__));
		return BCME_BADARG;
	}

	if ((id < 0) || (id > OLMSG_BCN_MAX_IE)) {
		WL_ERROR(("%s: Invalid params/state while changing IE notification offloads\n",
			__FUNCTION__));
		return BCME_BADARG;
	} else {
		WL_ERROR(("%s: IE (%d) notification offloads: %s\n",
			__FUNCTION__, id, (enable) ? "Enable" : "Disable"));
	}

	/* wlc = ol->wlc; */

	/* current_bss = cfg->current_bss; */

	bzero(&p_ie_notification, sizeof(olmsg_ie_notification_enable));

	/* Fill up notification enable/disable message */

	p_ie_notification.hdr.type = BCM_OL_MSG_IE_NOTIFICATION;
	p_ie_notification.hdr.seq = 0;
	p_ie_notification.hdr.len = sizeof(olmsg_ie_notification_enable)
					- sizeof(olmsg_header);
	bcopy(&cfg->BSSID, &p_ie_notification.BSSID, sizeof(struct ether_addr));
	bcopy(&cfg->cur_etheraddr, &p_ie_notification.cur_etheraddr, sizeof(struct ether_addr));

	p_ie_notification.id = id; /* Which IE */
	p_ie_notification.enable = enable; /* Enable or disable */

	if (enable) {
		SET_ID(ol->ie_info.iemask, id);
	} else {
		RESET_ID(ol->ie_info.iemask, id);
	}

	/* Send ie notification offload enable message to CR4 */
	wlc_ol_msg_send(ol, (uint8 *)&p_ie_notification, sizeof(olmsg_ie_notification_enable));

	return err;
}

static int
wlc_notification_set_flag(wlc_ol_info_t *ol, wlc_bsscfg_t *cfg, bool enable)
{
	int err = BCME_OK;
	olmsg_ie_notification_enable  p_ie_notification;
	/* wlc_info_t *wlc = NULL; */
	/* wlc_bss_info_t *current_bss = NULL; */

	/* Check for valid state to enable beacon offloads */
	if (!ol || !cfg || !BSSCFG_STA(cfg) || (!cfg->BSS) ||
		((enable == TRUE) && (ol->ol_flags & OL_IE_NOTIFICATION_ENAB)) ||
		((enable == FALSE) && !(ol->ol_flags & OL_IE_NOTIFICATION_ENAB))) {
		WL_ERROR(("%s: Invalid params/state. Not changing beacon IE notification flag \n",
			__FUNCTION__));
		return BCME_BADARG;
	} else {
		WL_ERROR(("%s: IE notification offloads flag: %s\n",
			__FUNCTION__, (enable) ? "Enable" : "Disable"));
	}

	/* wlc = ol->wlc; */

	if (enable) {
		ol->ol_flags |= OL_IE_NOTIFICATION_ENAB;
	}

	bzero(&p_ie_notification, sizeof(olmsg_ie_notification_enable));

	/* Fill up notification enable/disable message */

	p_ie_notification.hdr.type = BCM_OL_MSG_IE_NOTIFICATION_FLAG;
	p_ie_notification.hdr.seq = 0;
	p_ie_notification.hdr.len = sizeof(olmsg_ie_notification_enable)
					- sizeof(olmsg_header);
	bcopy(&cfg->BSSID, &p_ie_notification.BSSID, sizeof(struct ether_addr));
	bcopy(&cfg->cur_etheraddr,  &p_ie_notification.cur_etheraddr, sizeof(struct ether_addr));

	p_ie_notification.id = -1; /* THIS IS NOT FOR IE */
	p_ie_notification.enable = enable; /* Enable or disable */

	/* Send ie notification offload enable message to CR4 */
	wlc_ol_msg_send(ol, (uint8 *)&p_ie_notification, sizeof(olmsg_ie_notification_enable));

	if (enable == 0) {
		ol->ol_flags &= ~OL_IE_NOTIFICATION_ENAB;
	}

	return err;
}

int8
wlc_ol_noise_avg_offload(void *w)
{
	wlc_info_t *wlc = NULL;
	wlc_ol_info_t *ol = NULL;
	wlc_bsscfg_t *cfg;

	wlc = (wlc_info_t *)w;
	if (wlc == NULL) {
		return 0;
	}

	if (!wlc->clk) {
		return 0;
	}

	cfg = wlc->cfg;
	if (!cfg || !cfg->associated || !BSSCFG_STA(cfg) || (!cfg->BSS) || (!cfg->up)) {
		return 0;
	}

	ol = wlc->ol;
	if (ol == NULL) {
		return 0;
	}

	if (!ol->ol_up) {
		return 0;
	}

	if (!wlc_ol_bcn_is_enable(ol)) {
		return 0;
	}

	return (ol->shared_info->rssi_info.noise_avg);
}

void wlc_ol_rscupdate(wlc_ol_info_t *ol, void *rkey, void *msg)
{
	olmsg_armtxdone *txdone_msg;
	rsn_rekey_params *rsnkey;
	txdone_msg = (olmsg_armtxdone *) msg;
	rsnkey = (rsn_rekey_params *) rkey;
	WL_INFORM(("%s\n", __FUNCTION__));
	bcopy(txdone_msg->txinfo.replay_counter, rsnkey->replay_counter,
		EAPOL_KEY_REPLAY_LEN);

#ifdef BCMINTSUP
	if (SUP_ENAB(ol->wlc->pub)) {
		wlc_info_t * wlc = ol->wlc;
		struct scb * scb = wlc_scbfind(wlc, wlc->cfg, &wlc->cfg->BSSID);

		if (scb && (BSS_SUP_TYPE(wlc->idsup, SCB_BSSCFG(scb)) != SUP_UNUSED)) {
			wlc_wpa_sup_set_rekey_info(wlc->idsup, wlc->cfg, rsnkey);
		}
	}
#endif // endif

	return;
}

int
wlc_ol_gtk_enable(wlc_ol_info_t *ol,
	rsn_rekey_params *rkey, struct scb *scb, int wpaauth)
{
	int err;
	olmsg_gtk_enable gtk_enable;
	wlc_info_t *wlc;
	wlc_bsscfg_t *cfg;

	/* Check for valid state to enable gtk offloads */
	if (!ol || !ol->ol_up || !ol->wlc) {
		WL_ERROR(("%s: Invalid params/state. Not enabling gtk offloads \n",
			__FUNCTION__));
		return BCME_BADARG;
	}

	err = BCME_OK;
	wlc = ol->wlc;
	if (!scb)
		scb = wlc_ol_get_scb(wlc);

	if (!scb) {
		err = BCME_NOTFOUND;
		goto done;
	}

	cfg = SCB_BSSCFG(scb);

	bzero(&gtk_enable, sizeof(olmsg_gtk_enable));
	gtk_enable.hdr.type = BCM_OL_GTK_ENABLE;
	gtk_enable.hdr.seq = 0;
	gtk_enable.hdr.len = sizeof(olmsg_gtk_enable) - sizeof(olmsg_header);

	bcopy(&cfg->BSSID, &gtk_enable.BSSID, sizeof(struct ether_addr));
	bcopy(&cfg->cur_etheraddr, &gtk_enable.cur_etheraddr, sizeof(struct ether_addr));
	wlc_ol_update_sec_info(ol, SCB_BSSCFG(scb), scb, &gtk_enable.sec_info);
	bcopy(rkey, &gtk_enable.rekey, sizeof(rsn_rekey_params));
	gtk_enable.gtk_algo = wlc_keymgmt_get_bss_key_algo(wlc->keymgmt, cfg, FALSE);

#ifdef MFP
	if (WLC_MFP_ENAB(wlc->pub) && SCB_MFP(scb))
		gtk_enable.igtk_enabled = 1;
#endif /* MFP */

	wlc_ol_msg_send(ol, (uint8 *)&gtk_enable, sizeof(olmsg_gtk_enable));

done:
	if (err != BCME_OK) {
			WL_ERROR(("wl%d:%s: error %d, not enabling gtk offloads\n",
			WLCWLUNIT(wlc), __FUNCTION__, err));
	}

	return err;
}

int
wlc_ol_l2keepalive_enable(wlc_ol_info_t *ol)
{
	int err = BCME_OK;
	olmsg_l2keepalive_enable_t l2keepalive_enable;
	wlc_l2keepalive_ol_params_t *params = &ol->l2keepalive_info;
	wlc_bsscfg_t *cfg = ol->wlc->cfg;

	/* Check for valid state to enable l2keepalive offloads */
	if (!ol || !cfg || !BSSCFG_STA(cfg) || !cfg->BSS ||
		!(ol->ol_up) || !(params->flags) ||
		!wlc_bss_connected(cfg)) {
		WL_ERROR(("%s: Invalid params/state. Not enabling L2KEEPALIVE offloads \n",
			__FUNCTION__));
		return BCME_BADARG;
	}

	bzero(&l2keepalive_enable, sizeof(olmsg_l2keepalive_enable_t));
	l2keepalive_enable.hdr.type = BCM_OL_L2KEEPALIVE_ENABLE;
	l2keepalive_enable.hdr.seq = 0;
	l2keepalive_enable.hdr.len = sizeof(olmsg_l2keepalive_enable_t) - sizeof(olmsg_header);
	l2keepalive_enable.period_ms = params->period_ms;
	l2keepalive_enable.prio = params->prio;
	l2keepalive_enable.flags = params->flags;
	wlc_ol_msg_send(ol, (uint8 *)&l2keepalive_enable, sizeof(olmsg_l2keepalive_enable_t));
	return err;
}

#ifdef WL_LTR
void
wlc_ol_ltr(wlc_ol_info_t *ol, wlc_ltr_info_t *ltr_info)
{
	olmsg_ltr ltr;

	if (!ol || !ol->ol_up)
		return;

	bzero(&ltr, sizeof(olmsg_ltr));
	ltr.hdr.type = BCM_OL_LTR;
	ltr.hdr.seq = 0;
	ltr.hdr.len = sizeof(olmsg_ltr) - sizeof(olmsg_header);
	ltr._ltr = LTR_ENAB(ltr_info->wlc->pub);
	ltr.active_lat = ltr_info->active_lat;
	ltr.active_idle_lat = ltr_info->active_idle_lat;
	ltr.sleep_lat = ltr_info->sleep_lat;
	ltr.hi_wm = ltr_info->hi_wm;
	ltr.lo_wm = ltr_info->lo_wm;
	wlc_ol_msg_send(ol, (uint8 *)&ltr, sizeof(olmsg_ltr));
	return;
}
#endif /* WL_LTR */

void wlc_ol_inc_rssi_cnt_arm(wlc_ol_info_t *ol)
{
	if (ol == NULL)
		return;
	ol->rssi_cnt_arm++;
}

void wlc_ol_inc_rssi_cnt_host(wlc_ol_info_t *ol)
{
	if (!ol || !ol->ol_up)
		return;
	ol->rssi_cnt_host++;
}

void wlc_ol_inc_rssi_cnt_events(wlc_ol_info_t *ol)
{
	if (!ol || !ol->ol_up)
		return;
	ol->rssi_cnt_events++;
}

/* Peterson's algorithm
 * P0: flag[0] = true;
 *   turn = 1;
 *   while (flag[1] == true && turn == 1)
 *   {
 *       / / busy wait
 *   }
 *   / / critical section
 *   ...
 *   / / end of critical section
 *   flag[0] = false;
 */

#define MAX_SEM_LOOP (100000)
void
tcm_sem_enter(wlc_info_t *wlc)
{
	wlc_ol_info_t *ol;
	olmsg_shared_info_t *shared_info = NULL;
	int loop = 0;
	static int max_loop = 0;
	ol = wlc->ol;

	if (!ol || !ol->ol_up || !ol->shared_info) {
		/* Do not return an error here */
		return;
	}

	shared_info = ol->shared_info;

	shared_info->flag[0] = TRUE;
	shared_info->turn = 1;
	while ((shared_info->flag[1] == TRUE && shared_info->turn == 1) &&
		(loop < MAX_SEM_LOOP)) {
		loop++;
	}
	if (loop) {
		if (loop > max_loop) {
			max_loop = loop;
		}
		/* print busy wait count */
		WL_INFORM(("######## %s : loop %d, max_count %d #######\n",
			__FUNCTION__, loop, max_loop));
		ASSERT(loop < MAX_SEM_LOOP);
	}
}

void
tcm_sem_exit(wlc_info_t *wlc)
{
	wlc_ol_info_t *ol;
	olmsg_shared_info_t *shared_info = NULL;
	ol = wlc->ol;

	if (!ol || !ol->ol_up || !ol->shared_info) {
		/* Do not return an error here */
		return;
	}

	shared_info = ol->shared_info;

	/* end of critical section */
	shared_info->flag[0] = FALSE;
}

void
tcm_sem_cleanup(wlc_info_t *wlc)
{
	wlc_ol_info_t *ol;
	olmsg_shared_info_t *shared_info = NULL;
	ol = wlc->ol;

	if (!ol || !ol->ol_up || !ol->shared_info) {
		/* Do not return an error here */
		return;
	}

	shared_info = ol->shared_info;

	/* clean up the semaphore */
	shared_info->flag[0] = FALSE;
	shared_info->flag[1] = FALSE;
}

void
wlc_ol_curpwr_upd(wlc_ol_info_t *ol, int8 target_txpwr_max, chanspec_t chanspec)
{
	if (!ol)
		return;

	/* target_txpwr_max = 0 => initialize the array with min_txpower */
	if (target_txpwr_max == 0) {
		int min_txpower;
		char country_abbrev[WLC_CNTRY_BUF_SZ];
		bcopy(wlc_channel_country_abbrev(ol->wlc->cmi), country_abbrev, WLC_CNTRY_BUF_SZ);
		if (strcmp(ol->country_abbrev, country_abbrev) == 0)
			return;
		if (ol->country_abbrev[0] == 'X') {
			bcopy(country_abbrev, ol->country_abbrev, WLC_CNTRY_BUF_SZ);
			return;
		}
		bcopy(country_abbrev, ol->country_abbrev, WLC_CNTRY_BUF_SZ);
		wlc_iovar_getint(ol->wlc, "min_txpower", &min_txpower);
		min_txpower *= WLC_TXPWR_DB_FACTOR; /* convert to qdb */
		memset(ol->curpwr_cache, min_txpower, sizeof(ol->curpwr_cache));
		return;
	}

	if (SCAN_IN_PROGRESS(ol->wlc->scan) && CHSPEC_IS20(chanspec)) {
		uint8 ctlch;
		ctlch = wf_chspec_ctlchan(chanspec);
		if (ctlch < MAXCHANNEL)
			ol->curpwr_cache[ctlch] = MAX(ol->curpwr_cache[ctlch], target_txpwr_max);
	}
}

/*
 * This function allows any module to register its own print handler for EVENTLOG event
 * type. So when that type message is detected in log it will call handler function
 * to interprete and print Data value
 */
void
wlc_eventlog_register_print_handler(wlc_ol_info_t *ol, uint8 type,
	wlc_eventlog_print_handler_fn_t fn)
{
	if (type < WLC_EL_LAST) {
		ol->eventlog_print_fn[type] = fn;
	}
}

/*
 * Part of big loop that will read all eventlog buffer
 * and go over each of them one by one, and either
 * printing log by itself or calling handler function
 * if user has already registered one
 */
static void
wlc_eventlog_print(wlc_ol_info_t *ol, struct bcmstrbuf *b, wlc_ol_eventlog_t *e)
{
	uint8	type = e->event_type;

	bcm_bprintf(b, "%06u.%03u:\t", e->event_time / 1000, e->event_time % 1000);

	if (ol->eventlog_print_fn[type]) {
		(*(ol->eventlog_print_fn[type]))(ol, b, type, e->event_time, e->event_data);
	} else {
		bcm_bprintf(b, "Type: %u Data: %u\n", type, e->event_data);
	}
}

/*
 * Main function to dump all the eventlog events
 * Maps the ARM memory and reads all the events here
 */
static int
wlc_eventlog_cons(wlc_ol_info_t *ol, struct bcmstrbuf *b)
{
	uint32		event_addr;
	ol_el_buf_t	*eb = NULL;
	ol_el_buf_t	*local_eb = NULL;

	olmsg_shared_info_t *shared_info = ol->shared_info;

	/* Return now if the chip is off */
	if (!ol->wlc->hw->clk) {
		WL_ERROR(("No Clock.\n"));
		return BCME_NOCLK;
	}

	if (!ol->ol_up) {
		WL_ERROR(("Offload not enabled\n"));
		return BCME_ERROR;
	}

	if (!ol->shared_info || !ol->bar1_addr) {
		WL_ERROR(("ol->shared_info or bar1_addr not initialized\n"));
		return BCME_NOTREADY;
	}

	event_addr = ltoh32(shared_info->eventlog_addr);
	if (event_addr > (ol->rambase + ol->ramsize)) {
		WL_ERROR(("event_addr is exceeding ram area\n"));
		return BCME_ERROR;
	}

	eb = (ol_el_buf_t *)(ol->bar1_addr + event_addr);

	local_eb = (ol_el_buf_t *)MALLOC(ol->osh, sizeof(ol_el_buf_t));
	if (local_eb == NULL) {
		WL_ERROR(("Insufficient memory\n"));
		return BCME_NOMEM;
	}

	memcpy(local_eb, eb, sizeof(ol_el_buf_t)); /* Suck everything from dongle */

	bcm_bprintf(b, "\nEvent Log\n");
	bcm_bprintf(b, "\tEvent Count: %u\n", local_eb->count);

	if (local_eb->count) {
		while (!WLC_EL_EMPTY(local_eb)) {
			wlc_eventlog_print(ol, b, &local_eb->event_buffer[local_eb->read_pos]);
			WLC_EL_INC_READ_POS(local_eb);
		}
	}

	MFREE(ol->osh, local_eb, sizeof(ol_el_buf_t));
	return BCME_OK;
}

#ifdef UCODE_SEQ
void
wlc_ol_update_seq_iv(wlc_info_t *wlc, bool frmShm, struct scb *scb)
{
	/* replay/iv is only 6 bytes */
	uchar iv_rc[WOWL_TSCPN_SIZE];
	wlc_key_t *key = NULL;
	wlc_key_info_t key_info;
	int retval;

	if (scb == NULL)
		scb = wlc_scbfind(wlc, wlc->cfg, &wlc->cfg->BSSID);

	if (scb == NULL)
		goto done;

	key = wlc_keymgmt_get_scb_key(wlc->keymgmt, scb, WLC_KEY_ID_PAIRWISE,
		WLC_KEY_FLAG_NONE, &key_info);
	if (key_info.algo == CRYPTO_ALGO_OFF)
		key = wlc_keymgmt_get_bss_tx_key(wlc->keymgmt, wlc->cfg,
			FALSE, &key_info);

	if (key_info.algo != CRYPTO_ALGO_TKIP && key_info.algo != CRYPTO_ALGO_AES_CCM)
		goto upd_seq11;

	if (frmShm) {
		/* update txiv from shm if > key seq */
		wlc_copyfrom_shm(wlc, M_REPCNT_TID, iv_rc, sizeof(iv_rc));
		retval = wlc_key_advance_seq(key, iv_rc, sizeof(iv_rc), 0, TRUE);
		if (retval != BCME_OK) {
			WL_ERROR(("wl%d: %s: err %d from " "wlc_key_advance_seq\n",
				WLCWLUNIT(wlc), __FUNCTION__, retval));
		}
		goto done;
	}

	retval = wlc_key_get_seq(key, iv_rc, sizeof(iv_rc), 0, TRUE);
	if (retval != sizeof(iv_rc)) {
		WL_ERROR(("%s: err %d from wlc_key_get_seq\n", __FUNCTION__, retval));
		memset(iv_rc, 0, sizeof(iv_rc));
	}
	wlc_copyto_shm(wlc, M_REPCNT_TID, iv_rc, sizeof(iv_rc));

upd_seq11:
	if (frmShm)
		SCB_SEQNUM(scb, PRIO_8021D_BK) = wlc_read_shm(wlc, M_SEQNUM_TID);
	else
		wlc_write_shm(wlc, M_SEQNUM_TID, SCB_SEQNUM(scb, PRIO_8021D_BK));

done:
	WL_INFORM(("%s: updated iv %s shm for key idx %d\n",
		frmShm ? "from" : "in", key ? key_info.key_idx : -1));
}
#endif /* UCODE_SEQ */

#define __ARM_ARCH_7R__
#include <sbhndarm.h>
bool wlc_ol_is_arm_halted(wlc_ol_info_t *ol)
{
	uint origidx;
	uint8 *regs;
	bool in_halt = FALSE;

	origidx = si_coreidx(ol->sih);
	regs = (uint8 *) si_setcore(ol->sih, ARMCR4_CORE_ID, 0);
	if ((si_wrapperreg(ol->sih, AI_IOCTRL, 0, 0) & SICF_CPUHALT) == SICF_CPUHALT)
		in_halt = TRUE;
	si_setcoreidx(ol->sih, origidx);
	return in_halt;
}

/* This function should be called with interrupts disabled */
void wlc_ol_arm_halt(wlc_ol_info_t *ol)
{
	/* Put ARM to reset state then HALT it.
	 * flags: SICF_CPUHALT: 0x0020
	 */
	uint origidx;
	uint8 *regs;

	WL_ERROR(("%s: halting ARM\n", __FUNCTION__));

	origidx = si_coreidx(ol->sih);
	regs = (uint8 *) si_setcore(ol->sih, ARMCR4_CORE_ID, 0);

	si_core_reset(ol->sih, SICF_CPUHALT, 0);

	/* CRWLARMCR4-53 WAR: keep BTCM banks on */
	if (CHIPID(ol->sih->chip) == BCM43602_CHIP_ID) {
		W_REG(ol->osh, (uint32*)(regs + SI_CR4_BANKIDX), 5);
		W_REG(ol->osh, (uint32*)(regs + SI_CR4_BANKPDA), 0);
		W_REG(ol->osh, (uint32*)(regs + SI_CR4_BANKIDX), 7);
		W_REG(ol->osh, (uint32*)(regs + SI_CR4_BANKPDA), 0);
	}

	si_setcoreidx(ol->sih, origidx);
	/* clean tcm_sem token while arm halt. */
	tcm_sem_cleanup(ol->wlc);
}

#endif /* defined(WLOFFLD) */
