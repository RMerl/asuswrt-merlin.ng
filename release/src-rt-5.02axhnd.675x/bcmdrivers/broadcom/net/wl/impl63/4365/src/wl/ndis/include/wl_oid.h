
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
 * $Id: wl_oid.h 502316 2014-09-12 15:32:17Z $
 */
#ifndef _wl_oid_h_
#define _wl_oid_h_

/* forward declare */
typedef struct wl_oid wl_oid_t;

#ifdef WL_OIDS

/* Flag bits for NDIS key structure */
#define WSEC_NDIS_TX_KEY			(1<<31)
#define WSEC_NDIS_PAIR_KEY			(1<<30)
#define WSEC_NDIS_IV_VALID			(1<<29)
#define WSEC_NDIS_AUTHENTICATOR			(1<<28)

/* used to indicate "empty" SSID (none)...as opposed to NULL SSID (any) */
#define DOT11_INVALID_SSID_LEN			(DOT11_MAX_SSID_LEN + 1)

/* Flag bits for wep (size) bitvector */
#define EXT_STA_WEP				(1<<0)
#define EXT_STA_WEP40				(1<<1)
#define EXT_STA_WEP104				(1<<2)

/* Flag bits for mcast cipher bitvector (requires a bit for None to represent WEP/None) */
#define EXT_STA_MCAST_NONE			(1<<0)
#define EXT_STA_MCAST_WEP			(1<<1)
#define EXT_STA_MCAST_WEP40			(1<<2)
#define EXT_STA_MCAST_WEP104			(1<<3)
#define EXT_STA_MCAST_TKIP			(1<<4)
#define EXT_STA_MCAST_AES			(1<<5)
#ifdef BCMWAPI_WPI
#define EXT_STA_MCAST_SMS4			(1<<6)
#endif /* BCMWAPI_WPI */

/* Max number of SSIDs to scan for directly per scan request (spec requires min of 4) */
#define MAXSCAN_SSIDS				4

/* Max number of excluded MAC address list entries */
#define	MAX_EXCLUDED_MAC_ADDRS			4

#if defined(D0_COALESCING)
/* Max number of D0_COALESCING filters supported */
#define MAX_D0_FILTERS				8

typedef struct wl_d0_filters {
	NDIS_RECEIVE_FILTER_ID			FilterId;
	ULONG elementsCount;
	PNDIS_RECEIVE_FILTER_FIELD_PARAMETERS	pFieldParam;
} wl_d0_filters_t;
#endif /* D0_COALESCING */

typedef struct peer_info {
	uint32 state;			/* current state */
	assoc_decision_t assoc_dc;
	struct ether_addr mac;
    DOT11_AUTH_ALGORITHM        AuthAlgo;
    DOT11_CIPHER_ALGORITHM      UnicastCipher;
    DOT11_CIPHER_ALGORITHM      MulticastCipher;
	struct peer_info *next;
} peer_info_t;

typedef struct _ibss_peer {
	struct ether_addr	addr;
	uint16			cap;
	wlc_rateset_t		rateset;
	LARGE_INTEGER		assoc_time;
} ibss_peer_t;

/* OS provided additional association ies */
typedef struct {
	struct ether_addr bssid;	/* bssid */
	char *assoc_req_ies;	/* OS-provided IEs for assoc request */
	uint assoc_req_ies_len;	/* length of OS-provided IEs for assoc request */
} assoc_ie_t;

#define MAX_EXEMPTS	32	/* max exempts */

#if (NDISVER >= 0x0630)
#define MAX_MAC_NUM	4		/* maximum MACs including default one */
#elif (NDISVER == 0x0620)
#define MAX_MAC_NUM	2		/* maximum MACs including default one */
#elif (NDISVER == 0x0600)
#define MAX_MAC_NUM	1		/* maximum MACs including default one */
#endif /* (NDISVER >= 0x0630) */

#if (NDISVER >= 0x0620)
/* bcn and prb ies */
typedef struct {
	bcm_tlv_t	*bcn;	/* beacon wcn ie */
	bcm_tlv_t	*prb;	/* probe response wcn ie */
} wcn_ie_t;

/* AP dfs state */
enum {
	DFS_AP_RUNNING,
	DFS_AP_STOPPED,
	DFS_AP_RESUME
};
#endif /* (NDISVER >= 0x0620) */

/* Block Windows scans if rx/tx throughput exceeds this number of packets per second */
#define SCAN_BLOCK_THROUGHPUT			200
/* Windows scans every 63 seconds */
#define PERIODIC_SCAN_INTERVAL			59
/* Tolerance to the above interval, for purposes of scan blocking */
#define PERIODIC_SCAN_VARIANCE			2

#define SAFEMODE_CCX					1
#define SAFEMODE_UNUSED					2
#define SAFEMODE_MACSPOOF				8
#define SAFEMODE_WET					0x10

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

#if (NDISVER >= 0x0630)
/* forward declaration */
typedef struct wl_timer wl_timer_t;

#define	MAX_WFD_DISC_FILTERS	2	/* max wfd discovery request filters */

/* discovery filter option */
#define	WFD_DISC_FLT_DEV	1	/* find wfd device only */
#define WFD_DISC_FLT_GO		2	/* find specified wfd GO only */
#define WFD_DISC_FLT_EITHER	3	/* find either device or go */
#define WFD_DISC_FLT_ANY	4	/* find wfd device or go hosted by wfd device */

/* discovery filter */
typedef struct _wfd_disc_flt {
	uint8	dev_id[ETHER_ADDR_LEN];	/* device id */
	uint	option;
	wlc_ssid_t	grp_ssid;	/* GO ssid */
	bool	found;		/* true if found */
} wfd_disc_flt_t;

typedef struct _wfd_client_info {
	struct _wfd_client_info	*next;
	uint entry_sz;		/* size of structure */
	p2p_client_info_t cinfo;	/* variable length */
} wfd_client_info_t;

typedef struct {
	LIST_ENTRY ListEntry;
	void	*ies;
	uint	ies_len;
} recv_ie_t;

/* WFD config applied to both WFD device port and role port */
typedef struct {
	uint8		mac_addr[ETHER_ADDR_LEN];	/* mac address */

	/* cap bitmap */
	uint8		wfd_dev_cap_bitmap;	/* WFD device capability bitmap */
	uint8		wfd_grp_cap_bitmap;	/* WFD GO owner capability bitmap */
	uint		wfd_grp_gc_limit;	/* max number of P2P clients GO allows */

	/* device info */
	uint		wfd_wps_ver;		/* WPS version enabled for WFD */
	uint16		wfd_cfg_methods;	/* methods supported on the WFD device */
	DOT11_WFD_DEVICE_TYPE wfd_primary_dev_type;	/* WFD primary device type */
	DOT11_WFD_DEVICE_TYPE wfd_sec_dev_type[WFD_MAX_SEC_DEV_TYPES]; /* sec dev types */
	uint		wfd_num_sec_dev_type;	/* number of WFD secondary device types */
	DOT11_WPS_DEVICE_NAME wfd_dev_name;	/* friendly device name for WFD device */

	/* driver created P2P IEs(tx: D - device, C - client, G - GO) */
	uint8		*p2p_probreq_ies;	/* p2p IE for probe request(D, C) */
	uint		p2p_probreq_ies_len;
	uint8		*p2p_probrsp_ies;	/* p2p IE for probe response(D, G) */
	uint		p2p_probrsp_ies_len;

	/* OS-provided additional IEs(not P2P IEs) */
	uint8		*wfd_os_go_bcn_ies;	/* OS-provided WFD GO beacon IEs */
	uint		wfd_os_go_bcn_ies_len;	/* length of OS-provided WFD GO beacon IEs */
	uint8		*wfd_os_go_prbresp_ies;	/* OS-provided WFD GO probe response IEs */
	uint		wfd_os_go_prbresp_ies_len;	/* len of OS-provided GO prb resp IEs */
	uint8		*wfd_os_def_prbreq_ies;	/* OS-provided WFD default probe request IEs */
	uint		wfd_os_def_prbreq_ies_len;	/* len of OS-provided def prb req IEs */
	uint8		*wfd_os_prbreq_ies;		/* OS-provided prb req IEs */
	uint		wfd_os_prbreq_ies_len;	/* len of OS-provided prb req IEs */

	/* application-provided additional IEs(tx: D - device, C - client, G - GO) */
	uint8		*app_probreq_ies;	/* app p2p IE for probe request(D, C) */
	uint		app_probreq_ies_len;
	uint8		*app_probrsp_ies;	/* app p2p IE for probe response(D, G) */
	uint		app_probrsp_ies_len;

	uint		wfd_bsscfg_idx;		/* bsscfg index */
} wfd_gen_cfg_t;

/* GO negotiation request sending parameters */
typedef struct {
	uint8		intent;	/* GO neg req intent with tie breaker bit */
	uint32		go_cfg_timeout;	/* time(ms) needed to be GO */
	uint32		client_cfg_timeout; /* time(ms) needed to be client */
	uint8		intf_addr[ETHER_ADDR_LEN];	/* intended intf addr */
	uint8		grp_cap_bitmap; /* grp cap bitmap of P2P cap IE */
} wfd_gon_req_param_t;

/* GO negotiation reponse sending parameters */
typedef struct {
	DOT11_WFD_STATUS_CODE	status;	/* GO neg resp status */
	uint8		intent;	/* GO neg resp intent value */
	uint32		go_cfg_timeout;	/* time(ms) needed to be GO */
	uint32		client_cfg_timeout; /* time(ms) needed to be client */
	uint8		intf_addr[ETHER_ADDR_LEN];	/* intended intf addr */
	uint8		grp_dev_addr[ETHER_ADDR_LEN];	/* GO device address */
	wlc_ssid_t	grp_ssid;	/* GO ssid */
	uint8		grp_cap_bitmap; /* grp cap bitmap of P2P cap IE */
	bool		use_group_id;	/* include grp id attribute */
	void		*recvd_gon_req_ies;	/* pointer to received go neg req ies */
} wfd_gon_resp_param_t;

/* GO negotiation reponse confirm sending parameters */
typedef struct {
	DOT11_WFD_STATUS_CODE	status;	/* GO neg rsp confirm status */
	uint8		grp_dev_addr[ETHER_ADDR_LEN];	/* GO dev addr */
	wlc_ssid_t	grp_ssid;	/* GO ssid */
	bool		use_group_id;	/* include grp id attribute */
	uint8		grp_cap_bitmap;	/* grp cap bitmask of P2P cap IE */
	void		*recvd_gon_rsp_ies;	/* pointer to received go neg resp ies */
} wfd_gon_resp_conf_param_t;

/* Invitation request sending parameters */
typedef struct {
	uint32		go_cfg_timeout;	/* time(ms) needed to be GO */
	uint32		client_cfg_timeout; /* time(ms) needed to be client */
	uint8		flags;	/* bit 0: 0 = Join active group, 1 = Reinvoke */
	uint8		grp_bssid[ETHER_ADDR_LEN];	/* bssid for group */
	bool		use_group_bssid;	/* true to use provided grp bssid */
	bool		use_op_chan;	/* true to use provided op channel */
	uint8		grp_dev_addr[ETHER_ADDR_LEN];	/* GO device address */
	wlc_ssid_t	grp_ssid;	/* GO ssid */
} wfd_invite_req_param_t;

/* Invitation response sending parameters */
typedef struct {
	DOT11_WFD_STATUS_CODE	status;	/* status */
	uint32		go_cfg_timeout;	/* time(ms) needed to be GO */
	uint32		client_cfg_timeout; /* time(ms) needed to be client */
	uint8		grp_bssid[ETHER_ADDR_LEN];	/* bssid for group */
	bool		use_group_bssid;	/* true to use provided grp bssid */
	bool		use_op_chan;	/* true to use provided op channel */
	void		*recvd_invite_req_ies;	/* pointer to received invitation req ies */
} wfd_invite_resp_param_t;

/* Provision discovery request sending parameters */
typedef struct {
	uint8		dev_addr[ETHER_ADDR_LEN];	/* device address */
	wlc_ssid_t	grp_ssid;	/* group ssid */
	bool		use_group_id;	/* true to include grp id attri in req */
	uint8		grp_cap_bitmap; /* grp cap bitmap of P2P cap IE */
} wfd_prov_disc_req_param_t;

/* Presence Response sending parameters */
typedef struct {

	DOT11_WFD_STATUS_CODE status;	/* status */
	uint8   num_noa_desc;		/* number of NoA descriptors (0 or 1) */
	uint8	noa_index;		/* Index */
	uint8	noa_ops_enable;		/* OppPS enable/disable */
	uint8   noa_ops_ctw;		/* CTWindow */
	uint8	noa_cnt_type;		/* Count/Type */
	uint32	noa_duration;		/* Duration */
	uint32	noa_interval;		/* Interval */
	uint32	noa_start;		/* Start Time */

} wfd_presence_rsp_param_t;

typedef struct {
	uint8	requestor_addr[ETHER_ADDR_LEN];	/* Requestor MAC addr */
	uint8	requestor_dialog_token;		/* Dialog token from the requestor */
} wfd_godisc_req_param_t;

/* Device Discovery response sending parameters */
typedef struct {
	DOT11_WFD_STATUS_CODE status; /* status */
} wfd_devdisc_rsp_param_t;

/* WFD device port extension */
typedef struct {
	wfd_gen_cfg_t	gen;

	/* driver created P2P IEs(tx: D - device, C - client, G - GO) */
	uint8		*p2p_pri_if_ies;	/* p2p IE for prb req, assoc req (D) */
	uint		p2p_pri_if_ies_len; /* on primary interface */

	/* p2p listening phase */
	uint		wfd_listen_method;	/* listen method */
	uint8		wfd_listen_channel;	/* listen channel */
	uint16		wfd_listen_period;	/* period in p2p listen state in ms */
	uint16		wfd_listen_interval;	/* listen interval */
	wl_timer_t	*wfd_listen_timer;	/* listen timer */
	bool		wfd_discoverable;	/* set for discoverable */

	/* p2p device discovery */
	uint8		wfd_disc_type;		/* discovry type */
	bool		wfd_disc_type_forced;	/* partial discovery allowed */
	uint32		wfd_disc_timeout;	/* discovery timeout in ms */
	uint		wfd_disc_filters;	/* number of disc filters */
	wfd_disc_flt_t wfd_disc_filter[MAX_WFD_DISC_FILTERS];	/* disc filters */
	bool		wfd_disc_legacy_nets;	/* force scan legacy networks */
	bool		wfd_disc_active_scan;	/* set for active scan */
	bool		wfd_disc_fast;	/* fast disc */
	uint		wfd_disc_retries;	/* discovery request retries */
	wl_timer_t	*wfd_disc_timer;	/* discovery timer */
	wl_timer_t	*wfd_disc_timeout_timer;	/* timeout timer */
	int			wfd_orig_scan_home_time;	/* saved original scan home time */
	uint		wfd_disc_state;	/* discovery state */
	LONGLONG	wfd_disc_start_tm;	/* discovery start time */
	PDOT11_WFD_DEVICE_ENTRY	wfd_devtbl[MAXBSS];
	uint		wfd_num_devs;

	uint8		wfd_go_neg_req_intent;	/* intent in gon req */

	uint8		sc_phy_id;	/* phy id for p2p social channels */

	wl_timer_t	*wfd_tx_dwell_timer;	/* tx dwell timer */
	wl_timer_t	*wfd_tx_retry_timer;	/* tx retry timer */
	wl_timer_t	*wfd_fast_disc_scan_timer;	/* fast discovery scan timer */

	wfd_tx_frm_info_entry_t	*wfd_tx_frm_info_list;	/* tx act frm info link list */
	wfd_peer_chan_entry_t	*wfd_peer_dev_chan_list;	/* peer device channel list */
	wfd_peer_chan_entry_t	*wfd_peer_dev_opchan_list;	/* peer device op channel list */

	DOT11_WFD_CHANNEL wfd_msg_channel;	/* operating channel suggested in msg exchange */
	DOT11_WFD_CHANNEL wfd_prefered_channel;	/* prefered channel info */
	DOT11_WFD_CHANNEL wfd_op_channel;	/* operating channel info */

	p2p_chanlist_t	*wfd_supported_chan_list;	/* all channels driver supports */

	LIST_ENTRY	recv_ie_list;
	uint8	recent_act_frm_rcvd;
	uint8	fast_scan_special_case;
} wfd_dev_ext_t;

/* WFD role port extension */
typedef struct {
	wfd_gen_cfg_t	gen;

	uint16		wfd_role_flag;	/* wfd role operating flags */

	/* driver created P2P IEs(tx: D - device, C - client, G - GO) */
	uint8		*p2p_beacon_ies;	/* p2p IE for beacon(G) */
	uint		p2p_beacon_ies_len;
	uint8		*p2p_assocreq_ies;	/* p2p IE for association request(C) */
	uint		p2p_assocreq_ies_len;
	uint8		*p2p_assocrsp_ies;	/* p2p IE for association response(G) */
	uint		p2p_assocrsp_ies_len;

	/* app supplied P2P IEs(tx: D - device, C - client, G - GO) */
	uint8		*app_beacon_ies;	/* app p2p IE for beacon(G) */
	uint		app_beacon_ies_len;
	uint8		*app_assocreq_ies;	/* app p2p IE for association request(C) */
	uint		app_assocreq_ies_len;
	uint8		*app_assocrsp_ies;	/* app p2p IE for association response(G) */
	uint		app_assocrsp_ies_len;

	uint8		wfd_desired_grp_dev_addr[ETHER_ADDR_LEN];	/* desired GO dev addr */

	/* incoming assoc request */
	uint8		*wfd_incoming_assoc_ies;
	uint		wfd_incoming_assoc_ies_len;
	uint8		wfd_assocrsp_status;

	DOT11_WFD_CHANNEL wfd_op_channel;	/* operating channel info */

	uint32		wfd_grp_cfg_time;	/* cfg time for GO that GC can join */
	uint8		wfd_grp_cfg_wait_cnt;	/* wait count for GO up in GC connection */

	/* client info for connected P2P clients */
	wfd_client_info_t	*client_list;
	uint		client_list_count;

	uint		wfd_role_state;		/* role operating state */
	wl_timer_t	*wfd_state_timer;	/* state timer */
} wfd_role_ext_t;
#endif /* (NDISVER >= 0x0630) */

enum {
	ALT_LQ_BRCM = 0,
	ALT_LQ_VEN1,
	ALT_LQ_VEN2,
	ALT_LQ_VEN3,
	ALT_LQ_VEN4
};

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
					 * WLCNTVAL(cnt.rxundec)
					 * ct
					 */
	uint32		rxind_lu;	/* val taken @ link_up vs. running wl->rxind */

	bool 		scan_overrides;		/* Use users overrides or compute our own */

	uint		nmode_default;	/* To set automode properly */
	uint		gmode_default;	/* To set automode properly */

	mbool		sw_phy_state;	/* per-phy NIC power state */
	oid_phy_t	phy_def;	/* Phy defaults */
	oid_phy_t	phy;
	struct ether_addr 	addrOverride; /* Read from the registry at init time */

#if (NDISVER >= 0x0620)
#ifdef NOT_YET
	uint		preferred_macs;	/* number of macs in preferred mac list */
	ulong		mac_preferred[MAX_MAC_NUM];	/* preferred mac list */
#endif /* NOT_YET */

#if (NDISVER >= 0x0630)
	uint8		country[WLC_CNTRY_BUF_SZ];

	/* wfd device info */
	wl_oid_t	*wfd_dev_oid;	/* wfd device oid pointer */
	uint16		wfd_dev_flag;	/* wfd device operation flags */
	uint8		wfd_dev_addr[ETHER_ADDR_LEN];	/* p2p Device address */

	/* saved scan parameters */
	int		scan_nprobes_saved;	/* saved scan nprobes */
	int		scan_active_time_saved;	/* saved scan active time */
	int		roam_motion_saved;	/* saved roam motion delta */
	int		roam_tbtt_saved;	/* saved roam tbtt */
	bool		scan_param_saved;	/* flag for scan param save */
	int		roam_tbtt_ref_cnt;	/* roam tbtt ref cnt */

	/* .11w */
	bool		d11w_disable;	/* disable .11w(mfp) feature */
#endif /* (NDISVER >= 0x0630) */
#endif /* (NDISVER >= 0x0620) */

	uint32		alt_lq;		/* use alternative link quality formula */

	int	bcmc_timeout;	/* waiting time for bcmc packet */
} wl_oid_cmn_t;

#ifdef NLO
/* NLO */
typedef struct {
	uint	phy_type;
	uint	channel;
} nlo_channel_t;

typedef struct {
	wlc_ssid_t	ssid;
	uint	ucipher;
	uint	auth_algo;
	uint	ch_hints;	/* number of hint channels */
	nlo_channel_t	ch_hint[DOT11_MAX_CHANNEL_HINTS];
	bool	indicated;	/* set if indicated and should not do again */
} nlo_network_t;

typedef struct {
	uint	flags;
	uint	fast_scan_period;
	uint	fast_scans;
	uint	slow_scan_period;
	uint	networks;	/* number of networks for nlo */
	nlo_network_t	network[1];	/* nlo networks */
} nlo_info_t;
#endif /* NLO */

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
#if (NDISVER >= 0x0620)
	wcn_ie_t	wcn_ie;		/* WCN ie information for bss table */
#endif /* (NDISVER >= 0x0620) */
	uint32		phyidx;		/* index of matching phy_id2type[] entry */
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

	bool		opstateOP;	/* ExtSTA INIT: FALSE, ExtSTA OP: TRUE */
	bool		scan_no_flush;	/* save previous scan results and merge */

	int			scan_bss_type;	/* scan DOT11_BSS_TYPE */
	struct 		ether_addr	scan_bssid;	/* scan DOT11_MAC_ADDRESS */
	int			scan_type;	/* scan DOT11_SCAN_TYPE */
	wlc_ssid_t	scan_ssid[MAXSCAN_SSIDS];	/* SSIDs for scan (directed and bcast) */
	ULONG		auto_config_enabled;	/* enable miniport control of MAC and PHY */
	ULONG		default_key_id;	/* key index of primary key */
	wlc_ssid_t	desired_ssid;	/* SSID for connect request */

	bool		safe_mode_enabled;
	uint32		safe_mode_feature_mask;

	bool		hidden_network_enabled;

	bool		int_disassoc;	/* OS-requested disassoc? (reset or disconnect) */

	uint8		wep;		/* bitvector to distinguish WEP40 from WEP104 */
	uint32		mcast;		/* bitvector of enabled multicast ciphers */

	uint		prev_roam_reason; /* on fast roam failure, preserve prev roam reason */

	uint		udt;		/* unreachable detection threshold (in ms) */
	uint		IBSS_IEs_len;	/* length of OS-provided IEs */
	char		*IBSS_IEs;	/* OS-provided IEs */

	uint		prb_ies_len;	/* length of OS-provided IEs appended to active probe req */
	char		*prb_ies;	/* OS-provided IEs appended to active probe req */

	uint		num_peers;	/* number of other nodes in IBSS */
	ibss_peer_t	peers[MAXBSS];	/* IBSS peer table */
	LARGE_INTEGER	assoc_time;	/* timestamp at which association successfully completes */

	ULONG		CurrOpMode;	/* 802.11 current operation mode */

	assoc_ie_t	*assoc_ies;		/* OS-provided IEs for association request */
	uint8		*assoc_matched_ies;	/* bssid-matched assoc ies */
	uint		assoc_matched_ies_len;

	uint		pm_mode;	/* power management mode */
	uint		scan_pending;	/* block overlapping OID_DOT11_SCAN_REQUEST sets
					*    and index into array of scan SSIDs
					*/
	bool		wakeup_scan;	/* wake-up scan active */

	uint		cur_phy_id;	/* subsequent phy OIDs should be applied to this phy */
	uint		active_phy_id;	/* active phy id specified in association indication */
	ulong		desired_phy_id;	/* desired phy id */

#if (NDISVER >= 0x0620)
	uint8		*AP_BCN_IEs;	/* OS-provided IEs for beacon */
	uint		AP_BCN_IEs_len;	/* length of OS-provided IEs for beacon */
	uint8		*AP_PROBE_IEs;	/* OS-provided IEs for probe response */
	uint		AP_PROBE_IEs_len;	/* length of OS-provided IEs for probe response */

	uint8		*assocrsp_ies;		/* OS-provided IEs for assoc resp */
	uint		assocrsp_ies_len;	/* OS-provided IEs for assoc resp length */

#if (NDISVER >= 0x0630)
	/* wfd */
	union {
		wfd_dev_ext_t	*wfd_dev_ext;	/* wfd device port extension */
		wfd_role_ext_t	*wfd_role_ext;	/* wfd role port extension */
	} u;

	bool		mfp_cfged;		/* configured to support mfp */

	bool		pwr_auto_mgmt;	/* true if auto power management enabled by OS */
	uint		pwr_save_mode;	/* current hw power saving mode */
	uint		pwr_save_level;	/* current hw power saving level */
#endif /* (NDISVER >= 0x0630) */

	bool		wps;		/* WPS enabled */
	uint		ap_dfs_state;	/* AP dfs state */
#endif /* (NDISVER >= 0x0620) */

#ifdef NLO
	/* NLO(Network List Offload) */
	nlo_info_t	*nlo_info;		/* point to NLO configuration data when enabled */
	uint8		nlo_chs;		/* number of nlo channels in list. zero = all ch */
	uint8		*nlo_ch_list;	/* nlo channel list */
	uint		nlo_networks_indicated;	/* number of nlo networks found and indicated */
	bool		nlo_suspended;	/* NLO suspended */
	bool		nlo_ind_required;	/* NLO OS indication required */
	bool		nlo_net_found;	/* set if found network matching one
								 * in network list at first time
								 */
#endif /* NLO */

#if defined(WLSCANCACHE)
	bool		valid_results;	/* used with scan caching */
	wlc_bss_list_t	scan_cash_results; /* pulled from scan cache */
#endif // endif

	bool		primary;	/* this is primary oid */
	wl_if_t		*wlif;		/* wlif */
	NDIS_SPIN_LOCK	peer_lock;
	peer_info_t		*scb;	/* associated station link list */
#if defined(D0_COALESCING)
	wl_d0_filters_t	filter[MAX_D0_FILTERS];
#endif // endif
};

#if (NDISVER < 0x0620)
#define OP_MODE_AP(oid) FALSE
#elif (NDISVER < 0x0630)
#define OP_MODE_AP(oid) ((oid)->CurrOpMode == DOT11_OPERATION_MODE_EXTENSIBLE_AP)
#else
#define OP_MODE_AP(oid) (((oid)->CurrOpMode == DOT11_OPERATION_MODE_WFD_GROUP_OWNER) || \
			((oid)->CurrOpMode == DOT11_OPERATION_MODE_EXTENSIBLE_AP))
#endif /* NDISVER < 0x0620 */

#endif /* WL_OIDS */

#define WL_IOCTL_OID(oid) ((oid >= WL_OID_BASE) && (oid < (WL_OID_BASE + WLC_LAST)))

/* Iterator for STA wlifs */
#define FOREACH_STA_WLIF(wl, wlif) \
	for ((wlif) = (wl)->if_list; (wlif) != NULL; (wlif) = (wlif)->next) \
		if (!((wlif)->oid) || !OP_MODE_AP((wlif)->oid))

extern const bcm_iovar_t oid_iovars[];

struct wl_info;
extern wl_oid_t *wl_oid_attach(struct wl_info *wl);
extern void wl_oid_detach(wl_oid_t *oid);

extern int wl_oid_iovar(void *hdl, const bcm_iovar_t *vi, uint32 actionid,
	const char *name, void *p, uint plen, void *arg, int len, int vsize);

extern int wl_oidext_down(wl_oid_t *oid);

extern void wl_set_infra(wl_oid_t *oid, NDIS_802_11_NETWORK_INFRASTRUCTURE imode);
extern void wl_scan_complete(void *wl, int status);
extern int wl_get_chan_info(struct wl_info *wl, void *buf, uint len, wl_if_t *wlif);
extern void wl_oid_host_ext_init(wl_oid_t *oid);
extern void fake_ibss(struct wl_info *wl, struct ether_addr* addr);
extern void wl_set_safemode_enable(wl_oid_t *oid, bool bool_val);
extern void wl_wakeup_scan_complete(void *wl, int status);
extern void wl_os_scan_complete(wl_oid_t *oid, wl_if_t * wlif);
extern bool wl_get_illed_ap(struct wl_info *wl);
extern bool wl_get_ps_allowed(struct wl_info *wl);
extern void wl_oid_peer_add(wl_oid_t *oid, peer_info_t *scb);
extern peer_info_t *wl_oid_peer_remove(wl_oid_t *oid, struct ether_addr *mac);
extern peer_info_t *wl_oid_peer_get(wl_oid_t *oid, struct ether_addr *mac);
extern peer_info_t *wl_oid_peer_alloc(wl_oid_t *oid, struct ether_addr *mac);
extern void wl_oid_peer_free(wl_oid_t *oid, struct ether_addr *mac);
extern void wl_oid_peer_free_all(wl_oid_t *oid);
extern wl_oid_t *wl_wfd_dev_oid(struct wl_info *wl);
extern sta_info_t *wl_scbfind(struct wl_info *wl, struct ether_addr *addr,
	char *buf, int len, wl_if_t *wlif);
extern void wl_rateset_default(wl_oid_t *oid, wlc_rateset_t *rs_tgt,
	uint phy_type, int bandtype, bool cck_only, uint rate_mask,
	uint8 mcsallow, uint8 bw, uint8 txstreams);
extern bool wl_aps_active(struct wl_info *wl, wl_if_t *wlif);
extern bool wl_stas_active(struct wl_info *wl, wl_if_t *wlif);

extern void wl_oid_dev_ext_init(struct wl_info *wl, wl_oid_t *oid);

extern void wl_free_scan_result(wl_oid_t *oid);
extern void wl_set_scan_result(wl_oid_t *oid, wl_bss_info_t *wl_bi, int count);

#if (NDISVER >= 0x0620)
extern void wl_get_sta_wsec_cfg(uint8 auth_alg, uint8 WPA_auth, wpa_ie_fixed_t *wpaie,
	uint32 wsec, int *auth, int *ucipher, int *mcipher);
extern int wl_add_assoc_ie(wl_oid_t *oid, struct ether_addr *bssid);
#endif /* (NDISVER >= 0x0620) */

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

#ifdef NLO
#define NLO_ACTIVE(oid) ((oid)->nlo_info != NULL)
extern int wl_nlo_suspend(wl_oid_t *oid);
#endif /* NLO */

#if (NDISVER >= 0x0630)
extern wl_uint32_list_t* wl_get_chan_list(wl_oid_t *oid);
extern void wl_free_chan_list(wl_oid_t *oid, wl_uint32_list_t *list);
extern void wl_oidext_event(wl_oid_t *oid, wl_event_msg_t *evt, void *data);
extern void wl_abort_low_priority_scan(wl_oid_t *oid);
extern void wl_wfd_recv_ie_init(wl_oid_t *oid);
extern int wl_wfd_recv_ie_free(wl_oid_t *oid, recv_ie_t *recv_ie);
#endif /* (NDISVER >= 0x0630) */

#define WL_NBANDS(wl) ((wl)->vars._nbands)
#define WL_ASSOCIATED(wl) ((wl)->associated)
#define WL_SET_LAST_BCMERROR(wl, err) (wl)->vars.bcmerror = err

#endif /* _wl_oid_h_ */
