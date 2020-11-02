
/*
 * NDIS OID_802_11 handler for
 * Broadcom 802.11abg Networking Device Driver
 *
 * Portions
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
 * $Id: wl_oid5.h 486057 2014-06-18 16:49:15Z $
 */
#ifndef _wl_oid5_h_
#define _wl_oid5_h_

/* forward declare */
typedef struct wl_oid wl_oid_t;

#ifdef WL_OIDS

/* Flag bits for NDIS key structure */
#define WSEC_NDIS_TX_KEY			(1<<31)
#define WSEC_NDIS_PAIR_KEY			(1<<30)
#define WSEC_NDIS_IV_VALID			(1<<29)
#define WSEC_NDIS_AUTHENTICATOR			(1<<28)

/* Block Windows scans if rx/tx throughput exceeds this number of packets per second */
#define SCAN_BLOCK_THROUGHPUT			200
/* Windows scans every 63 seconds */
#define PERIODIC_SCAN_INTERVAL			63
/* Tolerance to the above interval, for purposes of scan blocking */
#define PERIODIC_SCAN_VARIANCE			2

/*
 * Increased from 64s to 128s. OS scans being lower priority than
 * WFD scans get blocked and take more than 64s to complete.
 * Increasing the BSS age timeout keeps the entries in the
 * table longer to report when queried by OID_DOT11_ENUM_BSS_LIST.
 */
#define WL_BSS_DEFAULT_AGE_TIMEOUT		128

#define  PSCAN_DEFAULT_IDLE_THRESHOLD	2
#define  PSCAN_DEFAULT_PARTIAL_INTERVAL 2
#define  PSCAN_DEFAULT_FULL_INTERVAL	6

#define	ESCAN_SYNC_ID 0x1234

#ifdef WL11AC
#define MAX_PHY_IDS	5	/* A, B, G, N, AC */
#elif WL11N
#define MAX_PHY_IDS	4	/* A, B, G, N */
#else
#define MAX_PHY_IDS	3	/* A, B, G */
#endif // endif

/* WL11N Support */
#define WL_N_ENAB(wl) wl_oid_get_nenab(wl)

typedef struct _oid_phy {
	uint	num_phy_ids;				/* num cached PhyIDs */
	uint	phy_id2type[MAX_PHY_IDS];	/* cached phy_types */
	uint	phy_channel[MAX_PHY_IDS];	/* OS-specified channel, per-phy */
	uint	phy_band[MAX_PHY_IDS];
} oid_phy_t;

#define MFP_CAP(oid, bsscfg) ((bsscfg)->bsscfg_idx == 0 && \
	(oid)->NDIS_infra == Ndis802_11Infrastructure && \
	((oid)->NDIS_auth == DOT11_AUTH_ALGO_RSNA || \
	(oid)->NDIS_auth == DOT11_AUTH_ALGO_RSNA_PSK) && (bsscfg)->wsec & AES_ENABLED)

/* common part of oids */
typedef struct {
	struct wl_info	*wl;	/* pointer for dereferencing wl directly, not through wlc */

	bool		WPA;		/* Set from WPA key from the registry */

	bool		legacy_link;	/* IBSS legacy (XP Gold) link behavior */

	/* did the user config a band lock, WLC_BAND_AUTO, WLC_BAND_2G, WLC_BAND_5G */
	int		bandlock;

	bool		NDISradio_disabled;	/* radio disabled via OID_802_11_DISASSOC */

	bool		scan_request_pending;	/* scan pending that
							* scan needs to be done when possible
							*/
	wl_oid_t	*scan_pending_oid;	/* scan pending oid */
	wl_oid_t	*scan_active_oid;	/* active scan oid */

	bool		quiet;

	uint32		media_stream_bitmap;	/* one bit per if. set if streaming enabled */

	/* receive stat counters */
	uint32		rxundec_lu;	/* val taken @ link_up vs. running
					 * WLCNTVAL(wl->pub->_cnt.rxundec)
					 * ct
					 */
	uint32		rxind_lu;	/* val taken @ link_up vs. running wl->rxind */
#if defined(BCMRECLAIM)
	bool		reclaim;	/* allow registry override to reclaim behavior */
#endif /* BCMRECLAIM */

	bool 		scan_overrides;		/* Use users overrides or compute our own */

	uint		nmode_default;	/* To set automode properly */
	uint		gmode_default;	/* To set automode properly */

	struct ether_addr 	addrOverride; /* Read from the registry at init time */
	int	bcmc_timeout;	/* waiting time for bcmc packet */
} wl_oid_cmn_t;

/* Background Periodic Scan in idle case */
typedef struct {
	uint		idle_count;	/* Periodic Scan Count in idle case */
	uint		idle_thresh;	/* Threshold in minutes */
	uint		partial_scan_int;	/* How often to scan active only channels */
	uint		last_partial_scan_time; /* Last partial scan time */
	uint		full_scan_int;	/* How often to scan all channels */
	uint		last_full_scan_time;	/* Last time full scan was done. */
} wl_pscan_t;

typedef struct {
	uint32		timestamp;	/* BSS entry timestamp since system up */
	LARGE_INTEGER	realtime;	/* BSS entry timestamp in real time units */
	bool		passive;	/* This is a passive channel */
} wl_bss_ctl_t;
/*
 * State that could affect the behavior of the OID handlers.
 */
struct wl_oid {
	wl_oid_cmn_t *cmn;		/* common variables */
	wl_bsscfg_t	bsscfg;		/* set of BSS configurations, idx 0 is default */
	bool		link;		/* link state */
	bool		gotbeacons;

	ulong		NDIS_auth;	/* requested authentication mode */
	ulong		NDIS_infra;	/* requested infrastructure mode */

	bool		set_ssid_request_pending; /* flag that a scan needs to be done when
						   * possible
						   */
#ifdef BCMDBG
	uint32		scan_start_time;	/* timestamp at scan request */
#endif /* BCMDBG */
	uint         	last_scan_request;	/* Time of last OID scan request */
	uint		scan_block_thresh;	/* # of rx+tx frames to block periodic scans */
	wlc_ssid_t	pending_set_ssid;
	wl_pscan_t	pscan;
	bool		forcelink;	/* flag to force link up */

	/* OID_802_11_BSSID_LIST_SCAN timing adjustments */
	int		unassoc_passive_time;	/* passive time for scan requests while
						 * unassociated
						 */
	int		assoc_passive_time;	/* passive time for scan requests while
						 * associated
						 */
	uint				num_bss;		/* number of entries */
	NDIS_WLAN_BSSID_EX *BSStable[MAXBSS];	/* current BSS table */
	wl_bss_ctl_t		BSSCtl[MAXBSS];		/* Control Structure for BSS table */
	uint				active_chan_timeout;	/* age timeout for active chnls */
	uint				passive_chan_timeout;	/* age timeout for passive chnls */

	bool				ibsshack;	/* WAR: fake a ibss association when down */
	struct ether_addr 	ibsshack_BSSID;
	NDIS_802_11_SSID	ibsshack_ssid;
	bool 			ibsshack_ch_restore;
	chanspec_t		ibsshack_chanspec_default;
	chanspec_t		ibsshack_assoc_chanspec;

	/* IBSS GMode and Nmode set and restore state */
	int 		gmode_ibss;	/* GMode to use in IBSS mode */
	uint		gmode_prev;	/* GMode to restore when leaving IBSS mode */
	int 		nmode_ibss;	/* NMode to use in IBSS mode */
	uint		nmode_prev;	/* NMode to restore when leaving IBSS mode */
#if defined(WL11AC)
	uint		vhtmode_prev;	/* VHTMode to restore when leaving IBSS mode */
#endif /* WL11AC */

	uint		scan_pending;	/* block overlapping OID_DOT11_SCAN_REQUEST sets
					*    and index into array of scan SSIDs
					*/
	bool		wakeup_scan;	/* wake-up scan active */

#if defined(WLSCANCACHE)
	bool		valid_results;	/* used with scan caching */
	wlc_bss_list_t	scan_cash_results; /* pulled from scan cache */
#endif /* WLSCANCACHE */

	bool		primary;	/* this is primary oid */
	wl_if_t		*wlif;		/* wlif */
};

#define OP_MODE_AP(oid) FALSE

#endif /* WL_OIDS */

#define WL_IOCTL_OID(oid) ((oid >= WL_OID_BASE) && (oid < (WL_OID_BASE + WLC_LAST)))

/* Iterator for STA wlifs */
#define FOREACH_STA_WLIF(wl, wlif) \
	for ((wlif) = (wl)->if_list; (wlif) != NULL; (wlif) = (wlif)->next) \
		if (!((wlif)->oid) || !OP_MODE_AP((wlif)->oid))

extern const bcm_iovar_t oid_iovars[];

#ifdef WL_OIDS
struct wl_info;
extern wl_oid_t *wl_oid_attach(struct wl_info *wl);
extern void wl_oid_detach(wl_oid_t *oid);

extern int wl_oid_iovar(void *hdl, const bcm_iovar_t *vi, uint32 actionid,
	const char *name, void *p, uint plen, void *arg, int len,
	int vsize, wlc_if_t *wlcif);

#ifdef BCMRECLAIM
#define wl_oid_reclaim(oid)		((oid)->cmn->reclaim)
#else
#define wl_oid_reclaim(oid)		0
#endif // endif

extern void wl_set_infra(wl_oid_t *oid, NDIS_802_11_NETWORK_INFRASTRUCTURE imode);
extern void wl_scan_complete(void *wl, int status);
extern int wl_get_chan_info(struct wl_info *wl, void *buf, uint len, wl_if_t *wlif);
extern void wl_oid_dev_ext_init(struct wl_info *wl, wl_oid_t *oid);
extern void wl_free_scan_result(wl_oid_t *oid);
extern void wl_set_scan_result(wl_oid_t *oid, wl_bss_info_t *wl_bi, int count);

extern void wl_init_pscan(wl_oid_t *oid);

extern int wl_scan_abort(wl_oid_t *oid);
extern int wl_register_ie_upd(wl_oid_t *oid, uint32 pktflag, uint8 **old_ie, uint *old_ie_len,
	uint8 *new_ie, uint new_ie_len);
extern int wl_upd_vndr_ies(wl_oid_t *oid, uint ie_len, char *ie_buf, bool add, uint32 pktflag);
extern void wl_free_oid_resources(wl_oid_t *oid);

extern void
wl_append_scan_result(wl_oid_t *oid, wl_bss_info_t *bss_info);

extern void wl_oid_event(wl_oid_t *oid, wl_event_msg_t *evt, void *data);
extern void wl_freebsstable(wl_oid_t *oid);
extern void wl_fill_bsstable(wl_oid_t *oid, wlc_bss_list_t *scan_results);
extern void wl_agebsstable(wl_oid_t *oid);
extern NDIS_STATUS wl_get_oid_request_info(void* wlhandle, const void *info_buf,
	ULONG info_buf_len,	NDIS_OID *ndis_oid, wl_oid_t **oid, PULONG info_hdr_len,
	ULONG **info_bytes_needed, NDIS_STATUS **info_status);
extern NDIS_STATUS wl_query_oid(wl_oid_t *oid, NDIS_OID ndis_oid, PVOID InfoBuf,
	ULONG InfoBufLen, PULONG BytesWritten, PULONG BytesNeeded);
extern NDIS_STATUS wl_set_oid(wl_oid_t *oid, NDIS_OID ndis_oid, PVOID InfoBuf,
	ULONG InfoBufLen, PULONG BytesRead, PULONG BytesNeeded);

extern void wl_process_pending_scan(wl_oid_t *oid, wl_event_msg_t *e);
extern int wl_oid_escan_event(wl_oid_t *oid, wl_event_msg_t *e, void *data);
extern int wl_scan_request_ex(wl_oid_t *oid, int bss_type, const struct ether_addr* bssid,
	int nssid, wlc_ssid_t *ssids, int scan_type, int nprobes,
	int active_time, int passive_time, int home_time,
	const chanspec_t* chanspec_list, int chanspec_num, bool escan);
extern int wl_oid_get_nenab(struct wl_info *wl);
extern int wl_get_wsec_info(struct wl_info *wl, wl_wsec_info_type_t type, void *data,
	uint16 len, wl_if_t *wlif);

#else /* WL_OIDS */

#define wl_oid_attach(a)		(wl_oid_t *)0x0dadbeef
#define wl_oid_detach(a)		do {} while (0)
#define wl_oid_reclaim(oid)		1
#define wl_freebsstable(a)		do {} while (0)
#define wl_agebsstable(a)		do {} while (0)
#define wl_set_infra(a, b)		do {} while (0)
#define wl_scan_complete(a, b, c)	do {} while (0)
#define wl_oid_event(a, b, c)		do {} while (0)
#define wl_fill_bsstable(a, b)		do {} while (0)
#endif /* WL_OIDS */

#define WL_NBANDS(wl) (((struct wlc_info *)((wl)->wlc))->pub->_nbands)
#define WL_ASSOCIATED(wl) (((struct wlc_info *)((wl)->wlc))->pub->associated)
#define WL_SET_LAST_BCMERROR(wl, err) ((wlc_info_t *)((wl)->wlc))->pub->bcmerror = err

#endif /* _wl_oid5_h_ */
