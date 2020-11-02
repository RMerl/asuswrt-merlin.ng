/*
 * Custom OID/ioctl definitions for
 *
 *
 * Broadcom 802.11abg Networking Device Driver
 *
 * Definitions subject to change without notice.
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
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: wlioctl.h 787840 2020-06-12 22:59:01Z $
 */

#ifndef _wlioctl_h_
#define	_wlioctl_h_

#include <typedefs.h>
#include <ethernet.h>
#include <bcmip.h>
#include <bcmeth.h>
#include <bcmip.h>
#include <bcmipv6.h>
#include <bcmevent.h>
#include <802.11.h>
#include <802.11s.h>
#include <802.1d.h>
#include <bcmwifi_channels.h>
#include <bcmwifi_rates.h>
#include <wlioctl_defs.h>
#include <bcmipv6.h>

#include <bcm_mpool_pub.h>
#include <bcmcdc.h>

/* 11ax trigger frame format - versioning info */
#define TRIG_FRAME_FORMAT_11AX_DRAFT_1P1 0

#define WLC_DC_DWDS_DATA_LENGTH 1
/*  XXX: LINUX_POSTMOGRIFY_REMOVAL: undefined during compile phase, so its
 *  a no-op for most cases. For hybrid and other open source releases,
 *  its defined during a second pass and mogrified out for distribution.
 */

typedef struct {
	uint32 num;
	chanspec_t list[1];
} chanspec_list_t;

#define RSN_KCK_LENGTH	16
#define RSN_KEK_LENGTH	16
#define TPK_FTM_LEN		16
#ifndef INTF_NAME_SIZ
#define INTF_NAME_SIZ	16
#endif // endif

/**Used to send ioctls over the transport pipe */
typedef struct remote_ioctl {
	cdc_ioctl_t	msg;
	uint32		data_len;
	char           intf_name[INTF_NAME_SIZ];
} rem_ioctl_t;
#define REMOTE_SIZE	sizeof(rem_ioctl_t)

#define BCM_IOV_XTLV_VERSION 0

#define MAX_NUM_D11CORES 2

/**DFS Forced param */
typedef struct wl_dfs_forced_params {
	chanspec_t chspec;
	uint16 version;
	chanspec_list_t chspec_list;
} wl_dfs_forced_t;

#define DFS_PREFCHANLIST_VER 0x01
#define WL_CHSPEC_LIST_FIXED_SIZE	OFFSETOF(chanspec_list_t, list)
/* size of dfs forced param size given n channels are in the list */
#define WL_DFS_FORCED_PARAMS_SIZE(n) \
	(sizeof(wl_dfs_forced_t) + (((n) < 1) ? (0) : (((n) - 1)* sizeof(chanspec_t))))
#define WL_DFS_FORCED_PARAMS_FIXED_SIZE \
	(WL_CHSPEC_LIST_FIXED_SIZE + OFFSETOF(wl_dfs_forced_t, chspec_list))
#define WL_DFS_FORCED_PARAMS_MAX_SIZE \
	WL_DFS_FORCED_PARAMS_FIXED_SIZE + (WL_NUMCHANSPECS * sizeof(chanspec_t))

/**association decision information */
typedef struct {
	uint8		assoc_approved;		/**< (re)association approved */
	uint8		reject_data;		/**< rejection data */
	uint16		reject_reason;		/**< reason code for rejecting association */
	struct		ether_addr   da;
	uint8		pad1[6];
#if defined(NDIS) && (NDISVER >= 0x0620)
	LARGE_INTEGER	sys_time;		/**< current system time */
#else
	int64		sys_time;		/**< current system time */
#endif // endif
} assoc_decision_t;

#define DFS_SCAN_S_IDLE		-1
#define DFS_SCAN_S_RADAR_FREE 0
#define DFS_SCAN_S_RADAR_FOUND 1
#define DFS_SCAN_S_INPROGESS 2
#define DFS_SCAN_S_SCAN_ABORTED 3
#define DFS_SCAN_S_SCAN_MODESW_INPROGRESS 4
#define DFS_SCAN_S_MAX 5

#define ACTION_FRAME_SIZE 1800

typedef struct wl_action_frame {
	struct ether_addr 	da;
	uint16 			len;
	uint32 			packetId;
	uint8			data[ACTION_FRAME_SIZE];
} wl_action_frame_t;

#define WL_WIFI_ACTION_FRAME_SIZE sizeof(struct wl_action_frame)

typedef struct ssid_info
{
	uint8		ssid_len;	/**< the length of SSID */
	uint8		ssid[32];	/**< SSID string */
} ssid_info_t;

typedef struct wl_af_params {
	uint32			channel;
	int32			dwell_time;
	struct ether_addr	BSSID;
	uint8 PAD[2];
	wl_action_frame_t	action_frame;
} wl_af_params_t;

#define WL_WIFI_AF_PARAMS_SIZE sizeof(struct wl_af_params)

#define MFP_TEST_FLAG_NORMAL	0
#define MFP_TEST_FLAG_ANY_KEY	1
typedef struct wl_sa_query {
	uint32 flag;
	uint8  action;
	uint8  PAD;
	uint16 id;
	struct ether_addr da;
	uint16  PAD;
} wl_sa_query_t;

/* EXT_STA */
/**association information */
typedef struct {
	uint32		assoc_req;	/**< offset to association request frame */
	uint32		assoc_req_len;	/**< association request frame length */
	uint32		assoc_rsp;	/**< offset to association response frame */
	uint32		assoc_rsp_len;	/**< association response frame length */
	uint32		bcn;		/**< offset to AP beacon */
	uint32		bcn_len;	/**< AP beacon length */
	uint32		wsec;		/**< ucast security algo */
	uint32		wpaie;		/**< offset to WPA ie */
	uint8		auth_alg;	/**< 802.11 authentication mode */
	uint8		WPA_auth;	/**< WPA: authenticated key management */
	uint8		ewc_cap;	/**< EWC (MIMO) capable */
	uint8		ofdm;		/**< OFDM */
} assoc_info_t;
/* defined(EXT_STA) */

/* Flags for OBSS IOVAR Parameters */
#define WL_OBSS_DYN_BWSW_FLAG_ACTIVITY_PERIOD        (0x01)
#define WL_OBSS_DYN_BWSW_FLAG_NOACTIVITY_PERIOD      (0x02)
#define WL_OBSS_DYN_BWSW_FLAG_NOACTIVITY_INCR_PERIOD (0x04)
#define WL_OBSS_DYN_BWSW_FLAG_PSEUDO_SENSE_PERIOD    (0x08)
#define WL_OBSS_DYN_BWSW_FLAG_RX_CRS_PERIOD          (0x10)
#define WL_OBSS_DYN_BWSW_FLAG_DUR_THRESHOLD          (0x20)
#define WL_OBSS_DYN_BWSW_FLAG_TXOP_PERIOD            (0x40)

/* OBSS IOVAR Version information */
#define WL_PROT_OBSS_CONFIG_PARAMS_VERSION 1

#include <packed_section_start.h>
typedef BWL_PRE_PACKED_STRUCT struct {
	uint8 obss_bwsw_activity_cfm_count_cfg; /**< configurable count in
		* seconds before we confirm that OBSS is present and
		* dynamically activate dynamic bwswitch.
		*/
	uint8 obss_bwsw_no_activity_cfm_count_cfg; /**< configurable count in
		* seconds before we confirm that OBSS is GONE and
		* dynamically start pseudo upgrade. If in pseudo sense time, we
		* will see OBSS, [means that, we false detected that OBSS-is-gone
		* in watchdog] this count will be incremented in steps of
		* obss_bwsw_no_activity_cfm_count_incr_cfg for confirming OBSS
		* detection again. Note that, at present, max 30seconds is
		* allowed like this. [OBSS_BWSW_NO_ACTIVITY_MAX_INCR_DEFAULT]
		*/
	uint8 obss_bwsw_no_activity_cfm_count_incr_cfg; /* see above
		*/
	uint16 obss_bwsw_pseudo_sense_count_cfg; /**< number of msecs/cnt to be in
		* pseudo state. This is used to sense/measure the stats from lq.
		*/
	uint8 obss_bwsw_rx_crs_threshold_cfg; /**< RX CRS default threshold */
	uint8 obss_bwsw_dur_thres; /**< OBSS dyn bwsw trigger/RX CRS Sec */
	uint8 obss_bwsw_txop_threshold_cfg; /**< TXOP default threshold */
} BWL_POST_PACKED_STRUCT wlc_obss_dynbwsw_config_t;
#include <packed_section_end.h>

#include <packed_section_start.h>
typedef BWL_PRE_PACKED_STRUCT struct {
	uint32 version;	/**< version field */
	uint32 config_mask;
	uint32 reset_mask;
	wlc_obss_dynbwsw_config_t config_params;
} BWL_POST_PACKED_STRUCT obss_config_params_t;
#include <packed_section_end.h>

/**bsscfg type */
typedef enum bsscfg_type {
	BSSCFG_TYPE_GENERIC = 0,	/**< Generic AP/STA/IBSS BSS */
	BSSCFG_TYPE_P2P = 1,		/**< P2P BSS */
	/* index 2 earlier used for BTAMP */
	BSSCFG_TYPE_PSTA = 3,
	BSSCFG_TYPE_TDLS = 4,
	BSSCFG_TYPE_SLOTTED_BSS = 5,
	BSSCFG_TYPE_PROXD = 6,
	BSSCFG_TYPE_NAN = 7,
	BSSCFG_TYPE_MESH = 8,
	BSSCFG_TYPE_AIBSS = 9
} bsscfg_type_t;

/* bsscfg subtype */
typedef enum bsscfg_subtype {
	BSSCFG_SUBTYPE_NONE = 0,
	BSSCFG_GENERIC_STA = 1,		/* GENERIC */
	BSSCFG_GENERIC_AP = 2,
	BSSCFG_GENERIC_IBSS = 6,
	BSSCFG_P2P_GC = 3,		/* P2P */
	BSSCFG_P2P_GO = 4,
	BSSCFG_P2P_DISC = 5,
	/* Index 7 & 8 earlier used for BTAMP */
	BSSCFG_SUBTYPE_AWDL = 9, /* SLOTTED_BSS_TYPE */
	BSSCFG_SUBTYPE_NAN_MGMT = 10,
	BSSCFG_SUBTYPE_NAN_DATA = 11,
	BSSCFG_SUBTYPE_NAN_MGMT_DATA = 12
} bsscfg_subtype_t;

typedef struct wlc_bsscfg_info {
	uint32 type;
	uint32 subtype;
} wlc_bsscfg_info_t;

/* ULP SHM Offsets info */
typedef struct ulp_shm_info {
	uint32 m_ulp_ctrl_sdio;
	uint32 m_ulp_wakeevt_ind;
	uint32 m_ulp_wakeind;
} ulp_shm_info_t;

/* Note: Due to unpredictable size, bool type should not be used in any ioctl argument structure
 * Cf PR53622
 */

/* Legacy structure to help keep backward compatible wl tool and tray app */

#define	LEGACY_WL_BSS_INFO_VERSION	107	/**< older version of wl_bss_info struct */

typedef struct wl_bss_info_107 {
	uint32		version;		/**< version field */
	uint32		length;			/**< byte length of data in this record,
						 * starting at version and including IEs
						 */
	struct ether_addr BSSID;
	uint16		beacon_period;		/**< units are Kusec */
	uint16		capability;		/**< Capability information */
	uint8		SSID_len;
	uint8		SSID[32];
	uint8		PAD;
	struct {
		uint32	count;			/**< # rates in this set */
		uint8	rates[16];		/**< rates in 500kbps units w/hi bit set if basic */
	} rateset;				/**< supported rates */
	uint8		channel;		/**< Channel no. */
	uint8		PAD;
	uint16		atim_window;		/**< units are Kusec */
	uint8		dtim_period;		/**< DTIM period */
	uint8		PAD;
	int16		RSSI;			/**< receive signal strength (in dBm) */
	int8		phy_noise;		/**< noise (in dBm) */
	uint8		PAD[3];
	uint32		ie_length;		/**< byte length of Information Elements */
	/* variable length Information Elements */
} wl_bss_info_107_t;

/*
 * Per-BSS information structure.
 */

#define	LEGACY2_WL_BSS_INFO_VERSION	108		/**< old version of wl_bss_info struct */

/**
 * BSS info structure
 * Applications MUST CHECK ie_offset field and length field to access IEs and
 * next bss_info structure in a vector (in wl_scan_results_t)
 */
typedef struct wl_bss_info_108 {
	uint32		version;		/**< version field */
	uint32		length;			/**< byte length of data in this record,
						 * starting at version and including IEs
						 */
	struct ether_addr BSSID;
	uint16		beacon_period;		/**< units are Kusec */
	uint16		capability;		/**< Capability information */
	uint8		SSID_len;
	uint8		SSID[32];
	uint8		PAD[1];
	struct {
		uint32	count;			/**< # rates in this set */
		uint8	rates[16];		/**< rates in 500kbps units w/hi bit set if basic */
	} rateset;				/**< supported rates */
	chanspec_t	chanspec;		/**< chanspec for bss */
	uint16		atim_window;		/**< units are Kusec */
	uint8		dtim_period;		/**< DTIM period */
	uint8		PAD;
	int16		RSSI;			/**< receive signal strength (in dBm) */
	int8		phy_noise;		/**< noise (in dBm) */

	uint8		n_cap;			/**< BSS is 802.11N Capable */
	uint8		PAD[2];
	uint32		nbss_cap;		/**< 802.11N BSS Capabilities (based on HT_CAP_*) */
	uint8		ctl_ch;			/**< 802.11N BSS control channel number */
	uint8		PAD[3];
	uint32		reserved32[1];		/**< Reserved for expansion of BSS properties */
	uint8		flags;			/**< flags */
	uint8		reserved[3];		/**< Reserved for expansion of BSS properties */
	uint8		basic_mcs[MCSSET_LEN];	/**< 802.11N BSS required MCS set */

	uint16		ie_offset;		/**< offset at which IEs start, from beginning */
	uint8		PAD[2];
	uint32		ie_length;		/**< byte length of Information Elements */
	/* Add new fields here */
	/* variable length Information Elements */
} wl_bss_info_108_t;

#define	WL_BSS_INFO_VERSION	109		/**< current version of wl_bss_info struct */

/**
 * BSS info structure
 * Applications MUST CHECK ie_offset field and length field to access IEs and
 * next bss_info structure in a vector (in wl_scan_results_t)
 */
typedef struct wl_bss_info {
	uint32		version;		/**< version field */
	uint32		length;			/**< byte length of data in this record,
						 * starting at version and including IEs
						 */
	struct ether_addr BSSID;
	uint16		beacon_period;		/**< units are Kusec */
	uint16		capability;		/**< Capability information */
	uint8		SSID_len;
	uint8		SSID[32];
	uint8		bcnflags;		/* additional flags w.r.t. beacon */
	struct {
		uint32	count;			/**< # rates in this set */
		uint8	rates[16];		/**< rates in 500kbps units w/hi bit set if basic */
	} rateset;				/**< supported rates */
	chanspec_t	chanspec;		/**< chanspec for bss */
	uint16		atim_window;		/**< units are Kusec */
	uint8		dtim_period;		/**< DTIM period */
	uint8		accessnet;		/* from beacon interwork IE (if bcnflags) */
	int16		RSSI;			/**< receive signal strength (in dBm) */
	int8		phy_noise;		/**< noise (in dBm) */
	uint8		n_cap;			/**< BSS is 802.11N Capable */
	uint16		freespace1;		/* make implicit padding explicit */
	uint32		nbss_cap;		/**< 802.11N+AC BSS Capabilities */
	uint8		ctl_ch;			/**< 802.11N BSS control channel number */
	uint8		padding1[3];		/**< explicit struct alignment padding */
	uint16		vht_rxmcsmap;	/**< VHT rx mcs map (802.11ac IE, VHT_CAP_MCS_MAP_*) */
	uint16		vht_txmcsmap;	/**< VHT tx mcs map (802.11ac IE, VHT_CAP_MCS_MAP_*) */
	uint8		flags;			/**< flags */
	uint8		vht_cap;		/**< BSS is vht capable */
	uint8		reserved[2];		/**< Reserved for expansion of BSS properties */
	uint8		basic_mcs[MCSSET_LEN];	/**< 802.11N BSS required MCS set */

	uint16		ie_offset;		/**< offset at which IEs start, from beginning */
	uint16		freespace2;		/* making implicit padding explicit */
	uint32		ie_length;		/**< byte length of Information Elements */
	int16		SNR;			/**< average SNR of during frame reception */
	uint16		vht_mcsmap;		/**< STA's Associated vhtmcsmap */
	uint16		vht_mcsmap_prop;	/**< STA's Associated prop vhtmcsmap */
	uint16		vht_txmcsmap_prop;	/**< prop VHT tx mcs prop */
} wl_bss_info_v109_t;

/**
 * BSS info structure
 * Applications MUST CHECK ie_offset field and length field to access IEs and
 * next bss_info structure in a vector (in wl_scan_results_t)
 */
typedef struct wl_bss_info_v109_1 {
	uint32		version;		/**< version field */
	uint32		length;			/**< byte length of data in this record,
						 * starting at version and including IEs
						 */
	struct ether_addr BSSID;
	uint16		beacon_period;		/**< units are Kusec */
	uint16		capability;		/**< Capability information */
	uint8		SSID_len;
	uint8		SSID[32];
	uint8		bcnflags;		/* additional flags w.r.t. beacon */
	struct {
		uint32	count;			/**< # rates in this set */
		uint8	rates[16];		/**< rates in 500kbps units w/hi bit set if basic */
	} rateset;				/**< supported rates */
	chanspec_t	chanspec;		/**< chanspec for bss */
	uint16		atim_window;		/**< units are Kusec */
	uint8		dtim_period;		/**< DTIM period */
	uint8		accessnet;		/* from beacon interwork IE (if bcnflags) */
	int16		RSSI;			/**< receive signal strength (in dBm) */
	int8		phy_noise;		/**< noise (in dBm) */
	uint8		n_cap;			/**< BSS is 802.11N Capable */
	uint8		he_cap;			/**< BSS is he capable */
	uint8		freespace1;		/* make implicit padding explicit */
	uint32		nbss_cap;		/**< 802.11N+AC BSS Capabilities */
	uint8		ctl_ch;			/**< 802.11N BSS control channel number */
	uint8		padding1[3];		/**< explicit struct alignment padding */
	uint16		vht_rxmcsmap;	/**< VHT rx mcs map (802.11ac IE, VHT_CAP_MCS_MAP_*) */
	uint16		vht_txmcsmap;	/**< VHT tx mcs map (802.11ac IE, VHT_CAP_MCS_MAP_*) */
	uint8		flags;			/**< flags */
	uint8		vht_cap;		/**< BSS is vht capable */
	uint8		reserved[2];		/**< Reserved for expansion of BSS properties */
	uint8		basic_mcs[MCSSET_LEN];	/**< 802.11N BSS required MCS set */

	uint16		ie_offset;		/**< offset at which IEs start, from beginning */
	uint16		freespace2;		/* making implicit padding explicit */
	uint32		ie_length;		/**< byte length of Information Elements */
	int16		SNR;			/**< average SNR of during frame reception */
	uint16		vht_mcsmap;		/**< STA's Associated vhtmcsmap */
	uint16		vht_mcsmap_prop;	/**< STA's Associated prop vhtmcsmap */
	uint16		vht_txmcsmap_prop;	/**< prop VHT tx mcs prop */
	uint32		timestamp;		/**< in ms since boot, OSL_SYSUPTIME() */
	uint16		he_sup_bw80_tx_mcs;	/**< supported HE (11ax) 80Mhz tx mcs map */
	uint16		he_sup_bw80_rx_mcs;	/**< supported HE (11ax) 80Mhz rx mcs map */
	uint16		he_sup_bw160_tx_mcs;	/**< supported HE (11ax) 160Mhz tx mcs map */
	uint16		he_sup_bw160_rx_mcs;	/**< supported HE (11ax) 160Mhz rx mcs map */
	uint16		he_sup_bw80p80_tx_mcs;	/**< supported HE (11ax) 80+80Mhz tx mcs map */
	uint16		he_sup_bw80p80_rx_mcs;	/**< supported HE (11ax) 80+80Mhz rx mcs map */
	uint16		he_neg_bw80_tx_mcs;	/**< negotiated HE (11ax) 80Mhz tx mcs map */
	uint16		he_neg_bw80_rx_mcs;	/**< negotiated HE (11ax) 80Mhz rx mcs map */
	uint16		he_neg_bw160_tx_mcs;	/**< negotiated HE (11ax) 160Mhz tx mcs map */
	uint16		he_neg_bw160_rx_mcs;	/**< negotiated HE (11ax) 160Mhz rx mcs map */
	uint16		he_neg_bw80p80_tx_mcs;	/**< negotiated HE (11ax) 80+80Mhz tx mcs map */
	uint16		he_neg_bw80p80_rx_mcs;	/**< negotiated HE (11ax) 80+80Mhz rx mcs map */
} wl_bss_info_v109_1_t;

#ifndef WL_BSS_INFO_TYPEDEF_HAS_ALIAS
typedef wl_bss_info_v109_t wl_bss_info_t;
#endif // endif

/* bssinfo flag for nbss_cap these are incontinuation of BI flags defined in wlioctl_defs.h */
#define VHT_BI_SGI_160MHZ                   0x00001000
#define VHT_BI_CAP_SU_BEAMFMR               0x00002000
#define VHT_BI_CAP_MU_BEAMFMR               0x00004000

#define WL_GSCAN_FULL_RESULT_VERSION	2	/* current version of wl_gscan_result_t struct */

typedef struct wl_gscan_bss_info {
	uint32      timestamp[2];
	wl_bss_info_v109_t info;
	/* Do not add any more members below, fixed  */
	/* and variable length Information Elements to follow */
} wl_gscan_bss_info_v2_t;

typedef struct wl_gscan_bss_info_v3 {
	uint32      timestamp[2];
	uint8 info[];	/* var length wl_bss_info_X structures */
	/* Do not add any more members below, fixed  */
	/* and variable length Information Elements to follow */
} wl_gscan_bss_info_v3_t;

#ifndef WL_BSS_INFO_TYPEDEF_HAS_ALIAS
typedef wl_gscan_bss_info_v2_t wl_gscan_bss_info_t;
#define WL_GSCAN_INFO_FIXED_FIELD_SIZE   (sizeof(wl_gscan_bss_info_t) - sizeof(wl_bss_info_t))
#endif // endif

typedef struct wl_bsscfg {
	uint32  bsscfg_idx;
	uint32  wsec;
	uint32  WPA_auth;
	uint32  wsec_index;
	uint32  associated;
	uint32  BSS;
	uint32  phytest_on;
	struct ether_addr   prev_BSSID;
	struct ether_addr   BSSID;
	uint32  targetbss_wpa2_flags;
	uint32 assoc_type;
	uint32 assoc_state;
} wl_bsscfg_t;

typedef struct wl_if_add {
	uint32  bsscfg_flags;
	uint32  if_flags;
	uint32  ap;
	struct ether_addr   mac_addr;
	uint16  PAD;
	uint32  wlc_index;
} wl_if_add_t;

typedef struct wl_bss_config {
	uint32	atim_window;
	uint32	beacon_period;
	uint32	chanspec;
} wl_bss_config_t;

#define WL_BSS_USER_RADAR_CHAN_SELECT	0x1	/**< User application will randomly select
						 * radar channel.
						 */

#define DLOAD_HANDLER_VER		1	/**< Downloader version */
#define DLOAD_FLAG_VER_MASK		0xf000	/**< Downloader version mask */
#define DLOAD_FLAG_VER_SHIFT		12	/**< Downloader version shift */

#define DL_CRC_NOT_INUSE	0x0001
#ifndef LINUX_POSTMOGRIFY_REMOVAL
#define DL_BEGIN		0x0002
#define DL_END			0x0004

/* Flags for Major/Minor/Date number shift and mask */
#define EPI_VER_SHIFT     16
#define EPI_VER_MASK      0xFFFF
#endif /* LINUX_POSTMOGRIFY_REMOVAL */
/** generic download types & flags */
enum {
	DL_TYPE_UCODE = 1,
	DL_TYPE_CLM = 2
};

/** ucode type values */
enum {
	UCODE_FW,
	INIT_VALS,
	BS_INIT_VALS
};

struct wl_dload_data {
	uint16 flag;
	uint16 dload_type;
	uint32 len;
	uint32 crc;
	uint8  data[1];
};
typedef struct wl_dload_data wl_dload_data_t;

struct wl_ucode_info {
	uint32 ucode_type;
	uint32 num_chunks;
	uint32 chunk_len;
	uint32 chunk_num;
	uint8  data_chunk[1];
};
typedef struct wl_ucode_info wl_ucode_info_t;

struct wl_clm_dload_info {
	uint32 ds_id;
	uint32 clm_total_len;
	uint32 num_chunks;
	uint32 chunk_len;
	uint32 chunk_offset;
	uint8  data_chunk[1];
};
typedef struct wl_clm_dload_info wl_clm_dload_info_t;

typedef struct wlc_ssid {
	uint32		SSID_len;
	uint8		SSID[DOT11_MAX_SSID_LEN];
} wlc_ssid_t;

typedef struct wlc_ssid_ext {
	uint8      hidden;
	uint8      PAD;
	uint16     flags;
	uint8      SSID_len;
	int8       rssi_thresh;
	uint8      SSID[DOT11_MAX_SSID_LEN];
} wlc_ssid_ext_t;

#define MAX_PREFERRED_AP_NUM     5
typedef struct wlc_fastssidinfo {
	uint32			SSID_channel[MAX_PREFERRED_AP_NUM];
	wlc_ssid_t		SSID_info[MAX_PREFERRED_AP_NUM];
} wlc_fastssidinfo_t;

typedef struct wnm_url {
	uint8   len;
	uint8   data[1];
} wnm_url_t;

typedef struct chan_scandata {
	uint8		txpower;
	uint8		pad;
	chanspec_t	channel;		/**< Channel num, bw, ctrl_sb and band */
	uint32		channel_mintime;
	uint32		channel_maxtime;
} chan_scandata_t;

typedef enum wl_scan_type {
	EXTDSCAN_FOREGROUND_SCAN,
	EXTDSCAN_BACKGROUND_SCAN,
	EXTDSCAN_FORCEDBACKGROUND_SCAN
} wl_scan_type_t;

#define WLC_EXTDSCAN_MAX_SSID		5

typedef struct wl_extdscan_params {
	int8 		nprobes;		/**< 0, passive, otherwise active */
	int8    	split_scan;		/**< split scan */
	int8		band;			/**< band */
	int8		pad;
	wlc_ssid_t 	ssid[WLC_EXTDSCAN_MAX_SSID]; /**< ssid list */
	uint32		tx_rate;		/**< in 500ksec units */
	wl_scan_type_t	scan_type;		/**< enum */
	int32 		channel_num;
	chan_scandata_t channel_list[1];	/**< list of chandata structs */
} wl_extdscan_params_t;

#define WL_EXTDSCAN_PARAMS_FIXED_SIZE 	(sizeof(wl_extdscan_params_t) - sizeof(chan_scandata_t))

#define WL_SCAN_PARAMS_SSID_MAX 	10

typedef struct wl_scan_params {
	wlc_ssid_t ssid;		/**< default: {0, ""} */
	struct ether_addr bssid;	/**< default: bcast */
	int8 bss_type;			/**< default: any,
					 * DOT11_BSSTYPE_ANY/INFRASTRUCTURE/INDEPENDENT
					 */
	uint8 scan_type;		/**< flags, 0 use default */

	int32 nprobes;			/**< -1 use default, number of probes per channel */
	int32 active_time;		/**< -1 use default, dwell time per channel for
					 * active scanning
					 */
	int32 passive_time;		/**< -1 use default, dwell time per channel
					 * for passive scanning
					 */
	int32 home_time;		/**< -1 use default, dwell time for the home channel
					 * between channel scans
					 */
	int32 channel_num;		/**< count of channels and ssids that follow
					 *
					 * low half is count of channels in channel_list, 0
					 * means default (use all available channels)
					 *
					 * high half is entries in wlc_ssid_t array that
					 * follows channel_list, aligned for int32 (4 bytes)
					 * meaning an odd channel count implies a 2-byte pad
					 * between end of channel_list and first ssid
					 *
					 * if ssid count is zero, single ssid in the fixed
					 * parameter portion is assumed, otherwise ssid in
					 * the fixed portion is ignored
					 */
	uint16 channel_list[1];		/**< list of chanspecs */
} wl_scan_params_t;

#define WL_SCAN_PARAMS_EXTRA_SIZE_1 0

#define WL_SCAN_PARAMS_EXTRA_SIZE_2 0

/** size of wl_scan_params not including variable length array */
#define WL_SCAN_PARAMS_FIXED_SIZE (64 + WL_SCAN_PARAMS_EXTRA_SIZE_1 + WL_SCAN_PARAMS_EXTRA_SIZE_2)

#define WL_MAX_ROAMSCAN_DATSZ	(WL_SCAN_PARAMS_FIXED_SIZE + (WL_NUMCHANNELS * sizeof(uint16)))

#define ISCAN_REQ_VERSION 1

/** incremental scan struct */
typedef struct wl_iscan_params {
	uint32 version;
	uint16 action;
	uint16 scan_duration;
	wl_scan_params_t params;
} wl_iscan_params_t;

/** 3 fields + size of wl_scan_params, not including variable length array */
#define WL_ISCAN_PARAMS_FIXED_SIZE (OFFSETOF(wl_iscan_params_t, params) + sizeof(wlc_ssid_t))

typedef struct wl_scan_results {
	uint32 buflen;
	uint32 version;
	uint32 count;
	wl_bss_info_v109_t bss_info[1];
} wl_scan_results_v109_t;

typedef struct wl_scan_results_v2 {
	uint32 buflen;
	uint32 version;
	uint32 count;
	uint8 bss_info[];	/* var length wl_bss_info_X structures */
} wl_scan_results_v2_t;

#ifndef WL_BSS_INFO_TYPEDEF_HAS_ALIAS
typedef wl_scan_results_v109_t wl_scan_results_t;
/** size of wl_scan_results not including variable length array */
#define WL_SCAN_RESULTS_FIXED_SIZE (sizeof(wl_scan_results_t) - sizeof(wl_bss_info_t))
#endif // endif

#ifndef LINUX_POSTMOGRIFY_REMOVAL
#if defined(SIMPLE_ISCAN)
/** the buf lengh can be WLC_IOCTL_MAXLEN (8K) to reduce iteration */
#define WLC_IW_ISCAN_MAXLEN   2048
typedef struct iscan_buf {
	struct iscan_buf * next;
	int8   iscan_buf[WLC_IW_ISCAN_MAXLEN];
} iscan_buf_t;
#endif /* SIMPLE_ISCAN */
#endif /* LINUX_POSTMOGRIFY_REMOVAL */
#define ESCAN_REQ_VERSION 1

/** event scan reduces amount of SOC memory needed to store scan results */
typedef struct wl_escan_params {
	uint32 version;
	uint16 action;
	uint16 sync_id;
	wl_scan_params_t params;
} wl_escan_params_t;

#define WL_ESCAN_PARAMS_FIXED_SIZE (OFFSETOF(wl_escan_params_t, params) + sizeof(wlc_ssid_t))

/** event scan reduces amount of SOC memory needed to store scan results */
typedef struct wl_escan_result {
	uint32 buflen;
	uint32 version;
	uint16 sync_id;
	uint16 bss_count;
	wl_bss_info_v109_t bss_info[1];
} wl_escan_result_v109_t;

/** event scan reduces amount of SOC memory needed to store scan results */
typedef struct wl_escan_result_v2 {
	uint32 buflen;
	uint32 version;
	uint16 sync_id;
	uint16 bss_count;
	uint8 bss_info[];	/* var length wl_bss_info_X structures */
} wl_escan_result_v2_t;

#ifndef WL_BSS_INFO_TYPEDEF_HAS_ALIAS
typedef wl_escan_result_v109_t wl_escan_result_t;
#define WL_ESCAN_RESULTS_FIXED_SIZE (sizeof(wl_escan_result_t) - sizeof(wl_bss_info_t))
#endif // endif

#ifndef LINUX_POSTMOGRIFY_REMOVAL
typedef struct wl_gscan_result {
	uint32 buflen;
	uint32 version;
	uint32 scan_ch_bucket;
	wl_gscan_bss_info_v2_t bss_info[1];
} wl_gscan_result_v2_t;

typedef struct wl_gscan_result_v2_1 {
	uint32 buflen;
	uint32 version;
	uint32 scan_ch_bucket;
	uint8 bss_info[];	/* var length wl_bss_info_X structures */
} wl_gscan_result_v2_1_t;

#ifndef WL_BSS_INFO_TYPEDEF_HAS_ALIAS
typedef wl_gscan_result_v2_t wl_gscan_result_t;
#define WL_GSCAN_RESULTS_FIXED_SIZE (sizeof(wl_gscan_result_t) - sizeof(wl_gscan_bss_info_t))
#endif // endif
#endif /* LINUX_POSTMOGRIFY_REMOVAL */

/** incremental scan results struct */
typedef struct wl_iscan_results {
	uint32 status;
	wl_scan_results_v109_t results;
} wl_iscan_results_v109_t;

/** incremental scan results struct */
typedef struct wl_iscan_results_v2 {
	uint32 status;
	wl_scan_results_v2_t results;
} wl_iscan_results_v2_t;

#ifndef WL_BSS_INFO_TYPEDEF_HAS_ALIAS
typedef wl_iscan_results_v109_t wl_iscan_results_t;
/** size of wl_iscan_results not including variable length array */
#define WL_ISCAN_RESULTS_FIXED_SIZE \
	(WL_SCAN_RESULTS_FIXED_SIZE + OFFSETOF(wl_iscan_results_t, results))
#endif // endif

typedef struct wl_probe_params {
	wlc_ssid_t ssid;
	struct ether_addr bssid;
	struct ether_addr mac;
} wl_probe_params_t;

#define WL_MAXRATES_IN_SET		16	/**< max # of rates in a rateset */

typedef struct wl_rateset {
	uint32	count;				/**< # rates in this set */
	uint8	rates[WL_MAXRATES_IN_SET];	/**< rates in 500kbps units w/hi bit set if basic */
} wl_rateset_t;

#define DYNTXC_MAX_COEFNUM	5 /**< max # of dyntxc candidates */
typedef struct wl_txs_dyntxc_info {
	bool	enable;		/**< Configure the enable */
	uint8	feature;	/**< Configure the supported cap */
	uint8	subperiod_num;	/**< Configure the training period */
	uint8	coefset;	/**< Configure the candidate coremask */
} wl_txs_dyntxc_info_t;

#define WL_VHT_CAP_MCS_MAP_NSS_MAX	8

typedef struct wl_rateset_args_v1 {
	uint32	count;				/**< # rates in this set */
	uint8	rates[WL_MAXRATES_IN_SET];	/**< rates in 500kbps units w/hi bit set if basic */
	uint8   mcs[MCSSET_LEN];	/**< supported mcs index bit map */
	uint16 vht_mcs[WL_VHT_CAP_MCS_MAP_NSS_MAX]; /**< supported mcs index bit map per nss */
} wl_rateset_args_v1_t;

#define RATESET_ARGS_V1		(1)
#define RATESET_ARGS_V2		(2)

/* XXX: RATESET_VERSION_ENABLED is defined in wl.mk post J branch.
 * Guidelines to use wl_rateset_args_t:
 * [a] in wlioctl.h: Add macro RATESET_ARGS_VX where X is the new version number.
 * [b] in wlioctl.h: Add a new structure with wl_rateset_args_vX_t
 * [c] in wlu.c app: Add support to parse new structure under RATESET_ARGS_VX
 * [d] in wlc_types.h: in respective branch and trunk: redefine wl_rateset_args_t with
 *	new wl_rateset_args_vX_t
 */
#ifndef RATESET_VERSION_ENABLED
/* rateset structure before versioning. legacy. DONOT update anymore here */
#define RATESET_ARGS_VERSION	(RATESET_ARGS_v1)
typedef wl_rateset_args_v1_t wl_rateset_args_t;
#endif /* RATESET_VERSION_ENABLED */

/* Note: dependent structures: sta_info_vX_t. When any update to this structure happens,
 *	update sta_info_vX_t also.
 */
#define WL_HE_CAP_MCS_MAP_NSS_MAX	8

typedef struct wl_rateset_args_v2 {
	uint16 version;		/**< version. */
	uint16 len;		/**< length */
	uint32	count;		/**< # rates in this set */
	uint8	rates[WL_MAXRATES_IN_SET];	/**< rates in 500kbps units w/hi bit set if basic */
	uint8   mcs[MCSSET_LEN];		/**< supported mcs index bit map */
	uint16 vht_mcs[WL_VHT_CAP_MCS_MAP_NSS_MAX]; /**< supported mcs index bit map per nss */
	uint16 he_mcs[WL_HE_CAP_MCS_MAP_NSS_MAX]; /**< supported he mcs index bit map per nss */
} wl_rateset_args_v2_t;

/* HE Rates BITMAP */
#define WL_HE_CAP_MCS_0_7_MAP		0x00ff
#define WL_HE_CAP_MCS_0_8_MAP		0x01ff
#define WL_HE_CAP_MCS_0_9_MAP		0x03ff
#define WL_HE_CAP_MCS_0_10_MAP		0x07ff
#define WL_HE_CAP_MCS_0_11_MAP		0x0fff

#define TXBF_RATE_MCS_ALL		4
#define TXBF_RATE_VHT_ALL		4
#define TXBF_RATE_OFDM_ALL		8

typedef struct wl_txbf_rateset {
	uint8	txbf_rate_mcs[TXBF_RATE_MCS_ALL];	/**< one for each stream */
	uint8	txbf_rate_mcs_bcm[TXBF_RATE_MCS_ALL];	/**< one for each stream */
	uint16	txbf_rate_vht[TXBF_RATE_VHT_ALL];	/**< one for each stream */
	uint16	txbf_rate_vht_bcm[TXBF_RATE_VHT_ALL];	/**< one for each stream */
	uint8	txbf_rate_ofdm[TXBF_RATE_OFDM_ALL]; /**< bitmap of ofdm rates that enables txbf */
	uint8	txbf_rate_ofdm_bcm[TXBF_RATE_OFDM_ALL]; /* bitmap of ofdm rates that enables txbf */
	uint8	txbf_rate_ofdm_cnt;
	uint8	txbf_rate_ofdm_cnt_bcm;
} wl_txbf_rateset_t;

#define NUM_BFGAIN_ARRAY_1RX	2
#define NUM_BFGAIN_ARRAY_2RX	3
#define NUM_BFGAIN_ARRAY_3RX	4
#define NUM_BFGAIN_ARRAY_4RX	5

typedef struct wl_txbf_expgainset {
	/* bitmap for each element: B[4:0]=>c0, B[9:5]=>c1, B[14:10]=>c2, B[19:15]=>c[3-7]
	 * B[24:20]=>c[8-9], B[29:25]=>c[10-11]
	 */
	uint32	bfgain_2x1[NUM_BFGAIN_ARRAY_1RX]; /* exp     1ss, imp 1ss */
	uint32	bfgain_2x2[NUM_BFGAIN_ARRAY_2RX]; /* exp [1-2]ss, imp 1ss */
	uint32	bfgain_3x1[NUM_BFGAIN_ARRAY_1RX];
	uint32	bfgain_3x2[NUM_BFGAIN_ARRAY_2RX];
	uint32	bfgain_3x3[NUM_BFGAIN_ARRAY_3RX]; /* exp [1-3]ss, imp 1ss */
	uint32	bfgain_4x1[NUM_BFGAIN_ARRAY_1RX];
	uint32	bfgain_4x2[NUM_BFGAIN_ARRAY_2RX];
	uint32	bfgain_4x3[NUM_BFGAIN_ARRAY_3RX];
	uint32	bfgain_4x4[NUM_BFGAIN_ARRAY_4RX]; /* exp [1-4]ss, imp 1ss */
} wl_txbf_expgainset_t;

#define OFDM_RATE_MASK			0x0000007f
typedef uint8 ofdm_rates_t;

typedef struct wl_rates_info {
	wl_rateset_t rs_tgt;
	uint32 phy_type;
	int32 bandtype;
	uint8 cck_only;
	uint8 rate_mask;
	uint8 mcsallow;
	uint8 bw;
	uint8 txstreams;
	uint8 PAD[3];
} wl_rates_info_t;

/**uint32 list */
typedef struct wl_uint32_list {
	/** in - # of elements, out - # of entries */
	uint32 count;
	/** variable length uint32 list */
	uint32 element[1];
} wl_uint32_list_t;

/* WLC_SET_ALLOW_MODE values */
#define ALLOW_MODE_ANY_BSSID		0
#define ALLOW_MODE_ONLY_DESIRED_BSSID	1
#define ALLOW_MODE_NO_BSSID		2

/** used for association with a specific BSSID and chanspec list */
typedef struct wl_assoc_params {
	struct ether_addr bssid;	/**< 00:00:00:00:00:00: broadcast scan */
	uint16 bssid_cnt;		/**< 0: use chanspec_num, and the single bssid,
					* otherwise count of chanspecs in chanspec_list
					* AND paired bssids following chanspec_list
					* also, chanspec_num has to be set to zero
					* for bssid list to be used
					*/
	int32 chanspec_num;		/**< 0: all available channels,
					* otherwise count of chanspecs in chanspec_list
					*/
	chanspec_t chanspec_list[1];	/**< list of chanspecs */
} wl_assoc_params_t;

#define WL_ASSOC_PARAMS_FIXED_SIZE 	OFFSETOF(wl_assoc_params_t, chanspec_list)

/** used for reassociation/roam to a specific BSSID and channel */
typedef wl_assoc_params_t wl_reassoc_params_t;
#define WL_REASSOC_PARAMS_FIXED_SIZE	WL_ASSOC_PARAMS_FIXED_SIZE

/** used for association to a specific BSSID and channel */
typedef wl_assoc_params_t wl_join_assoc_params_t;
#define WL_JOIN_ASSOC_PARAMS_FIXED_SIZE	WL_ASSOC_PARAMS_FIXED_SIZE

/** used for join with or without a specific bssid and channel list */
typedef struct wl_join_params {
	wlc_ssid_t ssid;
	wl_assoc_params_t params;	/**< optional field, but it must include the fixed portion
					 * of the wl_assoc_params_t struct when it does present.
					 */
} wl_join_params_t;

#define WL_JOIN_PARAMS_FIXED_SIZE 	(OFFSETOF(wl_join_params_t, params) + \
					 WL_ASSOC_PARAMS_FIXED_SIZE)

typedef struct wlc_roam_exp_params {
	int8 a_band_boost_threshold;
	int8 a_band_penalty_threshold;
	int8 a_band_boost_factor;
	int8 a_band_penalty_factor;
	int8 cur_bssid_boost;
	int8 alert_roam_trigger_threshold;
	int16 a_band_max_boost;
} wlc_roam_exp_params_t;

#define ROAM_EXP_CFG_VERSION     1

#define ROAM_EXP_ENABLE_FLAG             (1 << 0)

#define ROAM_EXP_CFG_PRESENT             (1 << 1)

typedef struct wl_roam_exp_cfg {
	uint16 version;
	uint16 flags;
	wlc_roam_exp_params_t params;
} wl_roam_exp_cfg_t;

typedef struct wl_bssid_pref_list {
	struct ether_addr bssid;
	/* Add this to modify rssi */
	int8 rssi_factor;
	int8 flags;
} wl_bssid_pref_list_t;

#define BSSID_PREF_LIST_VERSION        1
#define ROAM_EXP_CLEAR_BSSID_PREF        (1 << 0)

typedef struct wl_bssid_pref_cfg {
	uint16 version;
	uint16 flags;
	uint16 count;
	uint16 reserved;
	wl_bssid_pref_list_t bssids[];
} wl_bssid_pref_cfg_t;

#define SSID_WHITELIST_VERSION         1

#define ROAM_EXP_CLEAR_SSID_WHITELIST    (1 << 0)

/* Roam SSID whitelist, ssids in this list are ok to  */
/* be considered as targets to join when considering a roam */

typedef struct wl_ssid_whitelist {

	uint16 version;
	uint16 flags;

	uint8 ssid_count;
	uint8 reserved[3];
	wlc_ssid_t ssids[];
} wl_ssid_whitelist_t;

#define ROAM_EXP_EVENT_VERSION       1

typedef struct wl_roam_exp_event {

	uint16 version;
	uint16 flags;
	wlc_ssid_t cur_ssid;
} wl_roam_exp_event_t;

/** scan params for extended join */
typedef struct wl_join_scan_params {
	uint8 scan_type;		/**< 0 use default, active or passive scan */
	uint8 PAD[3];
	int32 nprobes;			/**< -1 use default, number of probes per channel */
	int32 active_time;		/**< -1 use default, dwell time per channel for
					 * active scanning
					 */
	int32 passive_time;		/**< -1 use default, dwell time per channel
					 * for passive scanning
					 */
	int32 home_time;		/**< -1 use default, dwell time for the home channel
					 * between channel scans
					 */
} wl_join_scan_params_t;

/** extended join params */
typedef struct wl_extjoin_params {
	wlc_ssid_t ssid;		/**< {0, ""}: wildcard scan */
	wl_join_scan_params_t scan;
	wl_join_assoc_params_t assoc;	/**< optional field, but it must include the fixed portion
					 * of the wl_join_assoc_params_t struct when it does
					 * present.
					 */
} wl_extjoin_params_t;
#define WL_EXTJOIN_PARAMS_FIXED_SIZE 	(OFFSETOF(wl_extjoin_params_t, assoc) + \
					 WL_JOIN_ASSOC_PARAMS_FIXED_SIZE)

#define ANT_SELCFG_MAX		4	/**< max number of antenna configurations */
#define MAX_STREAMS_SUPPORTED	4	/**< max number of streams supported */
typedef struct {
	uint8 ant_config[ANT_SELCFG_MAX];	/**< antenna configuration */
	uint8 num_antcfg;			/**< number of available antenna configurations */
} wlc_antselcfg_t;

typedef struct {
	uint32 duration;	/**< millisecs spent sampling this channel */
	uint32 congest_ibss;	/**< millisecs in our bss (presumably this traffic will */
				/**<  move if cur bss moves channels) */
	uint32 congest_obss;	/**< traffic not in our bss */
	uint32 interference;	/**< millisecs detecting a non 802.11 interferer. */
	uint32 timestamp;	/**< second timestamp */
	uint32 txnode[8];
	uint32 rxnode[8];
	uint32 xxobss;
} cca_congest_t;

typedef enum cca_mesh_cmd {
	CCA_MESH_GET = 0,
	CCA_MESH_SET = 1,
	CCA_MESH_DEL = 2,
	CCA_MESH_DEL_ALL = 3,
	CCA_MESH_ENABLE  = 4,
	CCA_MESH_DISABLE = 5,
	CCA_MESH_DUMP = 6
} cca_mesh_cmd_type;

typedef struct {
	chanspec_t chanspec;	/**< Which channel? */
	uint16 num_secs;	/**< How many secs worth of data */
	cca_congest_t  secs[1];	/**< Data */
} cca_congest_channel_req_t;

typedef struct {
	uint16 ver;
	uint16 len;
	uint8 count;		/**< how many MACs are set with CCA_CAP */
	uint8 cmd_type;		/**< Which cmd? (get, set, del, del_all) */
	struct ether_addr mac;
	cca_congest_channel_req_t data;
} cca_mesh_req_t;
#ifndef  LINUX_POSTMOGRIFY_REMOVAL
typedef struct {
	uint32 duration;	/**< millisecs spent sampling this channel */
	uint32 congest;		/**< millisecs detecting busy CCA */
	uint32 timestamp;	/**< second timestamp */
} cca_congest_simple_t;

typedef struct {
	uint16 status;
	uint16 id;
	chanspec_t chanspec;			/**< Which channel? */
	uint16 len;
	union {
		cca_congest_simple_t  cca_busy;	/**< CCA busy */
		cca_congest_t cca_busy_ext;	/**< Extended CCA report */
		int32 noise;			/**< noise floor */
	};
} cca_chan_qual_event_t;

typedef struct {
	uint32 msrmnt_time;	/**< Time for Measurement (msec) */
	uint32 msrmnt_done;	/**< flag set when measurement complete */
	char buf[];
} cca_stats_n_flags;

typedef struct {
	uint32 msrmnt_query;    /* host to driver query for measurement done */
	uint32 time_req;        /* time required for measurement */
	uint8 report_opt;       /* option to print different stats in report */
	uint8 PAD[3];
} cca_msrmnt_query;
#endif /* LINUX_POSTMOGRIFY_REMOVAL */

/* interference sources */
enum interference_source {
	ITFR_NONE = 0,			/**< interference */
	ITFR_PHONE,			/**< wireless phone */
	ITFR_VIDEO_CAMERA,		/**< wireless video camera */
	ITFR_MICROWAVE_OVEN,		/**< microwave oven */
	ITFR_BABY_MONITOR,		/**< wireless baby monitor */
	ITFR_BLUETOOTH,			/**< bluetooth */
	ITFR_VIDEO_CAMERA_OR_BABY_MONITOR,	/**< wireless camera or baby monitor */
	ITFR_BLUETOOTH_OR_BABY_MONITOR,	/**< bluetooth or baby monitor */
	ITFR_VIDEO_CAMERA_OR_PHONE,	/**< video camera or phone */
	ITFR_UNIDENTIFIED		/**< interference from unidentified source */
};

/** structure for interference source report */
typedef struct {
	uint32 flags;		/**< flags.  bit definitions below */
	uint32 source;		/**< last detected interference source */
	uint32 timestamp;	/**< second timestamp on interferenced flag change */
} interference_source_rep_t;

#define WLC_CNTRY_BUF_SZ	4		/**< Country string is 3 bytes + NUL */

typedef struct wl_country {
	char country_abbrev[WLC_CNTRY_BUF_SZ];	/**< nul-terminated country code used in
						 * the Country IE
						 */
	int32 rev;				/**< revision specifier for ccode
						 * on set, -1 indicates unspecified.
						 * on get, rev >= 0
						 */
	char ccode[WLC_CNTRY_BUF_SZ];		/**< nul-terminated built-in country code.
						 * variable length, but fixed size in
						 * struct allows simple allocation for
						 * expected country strings <= 3 chars.
						 */
} wl_country_t;

#ifndef LINUX_POSTMOGRIFY_REMOVAL

#define CCODE_INFO_VERSION 1

typedef enum wl_ccode_role {
	WLC_CCODE_ROLE_ACTIVE = 0,
	WLC_CCODE_ROLE_HOST,
	WLC_CCODE_ROLE_80211D_ASSOC,
	WLC_CCODE_ROLE_80211D_SCAN,
	WLC_CCODE_ROLE_DEFAULT,
	WLC_CCODE_LAST
} wl_ccode_role_t;
#define WLC_NUM_CCODE_INFO WLC_CCODE_LAST

typedef struct wl_ccode_entry {
	uint16 reserved;
	uint8 band;
	uint8 role;
	char	ccode[WLC_CNTRY_BUF_SZ];
} wl_ccode_entry_t;

typedef struct wl_ccode_info {
	uint16 version;
	uint16 count;   /**< Number of ccodes entries in the set */
	wl_ccode_entry_t ccodelist[1];
} wl_ccode_info_t;
#define WL_CCODE_INFO_FIXED_LEN	OFFSETOF(wl_ccode_info_t, ccodelist)
#endif /* LINUX_POSTMOGRIFY_REMOVAL */
typedef struct wl_channels_in_country {
	uint32 buflen;
	uint32 band;
	char country_abbrev[WLC_CNTRY_BUF_SZ];
	uint32 count;
	uint32 channel[1];
} wl_channels_in_country_t;

typedef struct wl_country_list {
	uint32 buflen;
	uint32 band_set;
	uint32 band;
	uint32 count;
	char country_abbrev[1];
} wl_country_list_t;

typedef struct wl_rm_req_elt {
	int8	type;
	int8	flags;
	chanspec_t	chanspec;
	uint32	token;		/**< token for this measurement */
	uint32	tsf_h;		/**< TSF high 32-bits of Measurement start time */
	uint32	tsf_l;		/**< TSF low 32-bits */
	uint32	dur;		/**< TUs */
} wl_rm_req_elt_t;

typedef struct wl_rm_req {
	uint32	token;		/**< overall measurement set token */
	uint32	count;		/**< number of measurement requests */
	void	*cb;		/**< completion callback function: may be NULL */
	void	*cb_arg;	/**< arg to completion callback function */
	wl_rm_req_elt_t	req[1];	/**< variable length block of requests */
} wl_rm_req_t;
#define WL_RM_REQ_FIXED_LEN	OFFSETOF(wl_rm_req_t, req)

typedef struct wl_rm_rep_elt {
	int8	type;
	int8	flags;
	chanspec_t	chanspec;
	uint32	token;		/**< token for this measurement */
	uint32	tsf_h;		/**< TSF high 32-bits of Measurement start time */
	uint32	tsf_l;		/**< TSF low 32-bits */
	uint32	dur;		/**< TUs */
	uint32	len;		/**< byte length of data block */
	uint8	data[1];	/**< variable length data block */
} wl_rm_rep_elt_t;
#define WL_RM_REP_ELT_FIXED_LEN	24	/**< length excluding data block */

#define WL_RPI_REP_BIN_NUM 8
typedef struct wl_rm_rpi_rep {
	uint8	rpi[WL_RPI_REP_BIN_NUM];
	int8	rpi_max[WL_RPI_REP_BIN_NUM];
} wl_rm_rpi_rep_t;

typedef struct wl_rm_rep {
	uint32	token;		/**< overall measurement set token */
	uint32	len;		/**< length of measurement report block */
	wl_rm_rep_elt_t	rep[1];	/**< variable length block of reports */
} wl_rm_rep_t;
#define WL_RM_REP_FIXED_LEN	8

typedef enum sup_auth_status {
	/* Basic supplicant authentication states */
	WLC_SUP_DISCONNECTED = 0,
	WLC_SUP_CONNECTING,
	WLC_SUP_IDREQUIRED,
	WLC_SUP_AUTHENTICATING,
	WLC_SUP_AUTHENTICATED,
	WLC_SUP_KEYXCHANGE,
	WLC_SUP_KEYED,
	WLC_SUP_TIMEOUT,
	WLC_SUP_LAST_BASIC_STATE,

	/* Extended supplicant authentication states */
	/** Waiting to receive handshake msg M1 */
	WLC_SUP_KEYXCHANGE_WAIT_M1 = WLC_SUP_AUTHENTICATED,
	/** Preparing to send handshake msg M2 */
	WLC_SUP_KEYXCHANGE_PREP_M2 = WLC_SUP_KEYXCHANGE,
	/* Waiting to receive handshake msg M3 */
	WLC_SUP_KEYXCHANGE_WAIT_M3 = WLC_SUP_LAST_BASIC_STATE,
	WLC_SUP_KEYXCHANGE_PREP_M4,	/**< Preparing to send handshake msg M4 */
	WLC_SUP_KEYXCHANGE_WAIT_G1,	/**< Waiting to receive handshake msg G1 */
	WLC_SUP_KEYXCHANGE_PREP_G2	/**< Preparing to send handshake msg G2 */
} sup_auth_status_t;

typedef struct wl_wsec_key {
	uint32		index;		/**< key index */
	uint32		len;		/**< key length */
	uint8		data[DOT11_MAX_KEY_SIZE];	/**< key data */
	uint32		pad_1[18];
	uint32		algo;		/**< CRYPTO_ALGO_AES_CCM, CRYPTO_ALGO_WEP128, etc */
	uint32		flags;		/**< misc flags */
	uint32		pad_2[2];
	int32		pad_3;
	int32		iv_initialized;	/**< has IV been initialized already? */
	int32		pad_4;
	/* Rx IV */
	struct {
		uint32	hi;		/**< upper 32 bits of IV */
		uint16	lo;		/**< lower 16 bits of IV */
		uint16	PAD;
	} rxiv;
	uint32		pad_5[2];
	struct ether_addr ea;		/**< per station */
	uint16	PAD;
} wl_wsec_key_t;

#define WSEC_MIN_PSK_LEN	8
#define WSEC_MAX_PSK_LEN	64

/** Flag for key material needing passhash'ing */
#define WSEC_PASSPHRASE		(1<<0)

/**receptacle for WLC_SET_WSEC_PMK parameter */
typedef struct wsec_pmk {
	ushort	key_len;		/**< octets in key material */
	ushort	flags;			/**< key handling qualification */
	uint8	key[WSEC_MAX_PSK_LEN];	/**< PMK material */
} wsec_pmk_t;

typedef struct _pmkid {
	struct ether_addr	BSSID;
	uint8			PMKID[WPA2_PMKID_LEN];
} pmkid_t;

typedef struct _pmkid_list {
	uint32	npmkid;
	pmkid_t	pmkid[1];
} pmkid_list_t;

typedef struct _pmkid_cand {
	struct ether_addr	BSSID;
	uint8			preauth;
} pmkid_cand_t;

typedef struct _pmkid_cand_list {
	uint32	npmkid_cand;
	pmkid_cand_t	pmkid_cand[1];
} pmkid_cand_list_t;

#define WL_STA_ANT_MAX		4	/**< max possible rx antennas */

typedef struct wl_assoc_info {
	uint32		req_len;
	uint32		resp_len;
	uint32		flags;
	struct dot11_assoc_req req;
	struct ether_addr reassoc_bssid; /**< used in reassoc's */
	struct dot11_assoc_resp resp;
} wl_assoc_info_t;

typedef struct wl_led_info {
	uint32      index;      /**< led index */
	uint32      behavior;
	uint8       activehi;
	uint8       PAD[3];
} wl_led_info_t;

/** srom read/write struct passed through ioctl */
typedef struct {
	uint32	byteoff;	/**< byte offset */
	uint32	nbytes;		/**< number of bytes */
	uint16	buf[];
} srom_rw_t;

#define CISH_FLAG_PCIECIS	(1 << 15)	/**< write CIS format bit for PCIe CIS */

/** similar cis (srom or otp) struct [iovar: may not be aligned] */
typedef struct {
	uint16	source;		/**< cis source */
	uint16	flags;		/**< flags */
	uint32	byteoff;	/**< byte offset */
	uint32	nbytes;		/**< number of bytes */
	/* data follows here */
} cis_rw_t;

/** R_REG and W_REG struct passed through ioctl */
typedef struct {
	uint32	byteoff;	/**< byte offset of the field in d11regs_t */
	uint32	val;		/**< read/write value of the field */
	uint32	size;		/**< sizeof the field */
	uint32	band;		/**< band (optional) */
} rw_reg_t;

/**
 * Structure used by GET/SET_ATTEN ioctls - it controls power in b/g-band
 * PCL - Power Control Loop
 */
typedef struct {
	uint16	auto_ctrl;	/**< WL_ATTEN_XX */
	uint16	bb;		/**< Baseband attenuation */
	uint16	radio;		/**< Radio attenuation */
	uint16	txctl1;		/**< Radio TX_CTL1 value */
} atten_t;

/** Per-AC retry parameters */
struct wme_tx_params_s {
	uint8  short_retry;
	uint8  short_fallback;
	uint8  long_retry;
	uint8  long_fallback;
	uint16 max_rate;  /**< In units of 512 Kbps */
};

typedef struct wme_tx_params_s wme_tx_params_t;

#define WL_WME_TX_PARAMS_IO_BYTES (sizeof(wme_tx_params_t) * AC_COUNT)

/**Used to get specific link/ac parameters */
typedef struct {
	int32 ac;
	uint8 val;
	struct ether_addr ea;
	uint8 PAD;
} link_val_t;

#define WL_PM_MUTE_TX_VER 1

typedef struct wl_pm_mute_tx {
	uint16 version;		/**< version */
	uint16 len;		/**< length */
	uint16 deadline;	/**< deadline timer (in milliseconds) */
	uint8  enable;		/**< set to 1 to enable mode; set to 0 to disable it */
	uint8 PAD;
} wl_pm_mute_tx_t;

typedef struct {
	uint32 packets;
	uint32 bytes;
} wl_traffic_stats_t;

typedef struct {
	uint16			ver;
	uint16			len;		/* length in bytes of this structure */
	struct ether_addr	ea;		/* Station address */
	uint16			pad;
	uint32                  tx_rspec;       /* Rate of last successful tx frame */
	uint32                  rx_rspec;       /* Rate of last successful rx frame */
} sta_report_t;

#define WLC_MAX_ASSOC_OUI_NUM 6
typedef struct {
	uint8 count;
	uint8 oui[WLC_MAX_ASSOC_OUI_NUM][DOT11_OUI_LEN];
} sta_vendor_oui_t;

#define WL_STA_REPORT_VER		1

/* XXX
 * Please update the following when modifying this structure:
 *   StaInfo Twiki page flags section - description of the sta_info_t struct
 *    src/wl/exe/wlu.c - print of sta_info_t
 * Pay attention to version if structure changes.
 */

/* sta_info_t version 4 */
typedef struct {
	uint16			ver;		/**< version of this struct */
	uint16			len;		/**< length in bytes of this structure */
	uint16			cap;		/**< sta's advertised capabilities */
	uint16			PAD;
	uint32			flags;		/**< flags defined below */
	uint32			idle;		/**< time since data pkt rx'd from sta */
	struct ether_addr	ea;		/**< Station address */
	uint16			PAD;
	wl_rateset_t	rateset;	/**< rateset in use */
	uint32			in;		/**< seconds elapsed since associated */
	uint32			listen_interval_inms; /**< Min Listen interval in ms for this STA */
	uint32			tx_pkts;	/**< # of user packets transmitted (unicast) */
	uint32			tx_failures;	/**< # of user packets failed */
	uint32			rx_ucast_pkts;	/**< # of unicast packets received */
	uint32			rx_mcast_pkts;	/**< # of multicast packets received */
	uint32			tx_rate;	/**< Rate used by last tx frame */
	uint32			rx_rate;	/**< Rate of last successful rx frame */
	uint32			rx_decrypt_succeeds;	/**< # of packet decrypted successfully */
	uint32			rx_decrypt_failures;	/**< # of packet decrypted unsuccessfully */
	uint32			tx_tot_pkts;	/**< # of user tx pkts (ucast + mcast) */
	uint32			rx_tot_pkts;	/**< # of data packets recvd (uni + mcast) */
	uint32			tx_mcast_pkts;	/**< # of mcast pkts txed */
	uint64			tx_tot_bytes;	/**< data bytes txed (ucast + mcast) */
	uint64			rx_tot_bytes;	/**< data bytes recvd (ucast + mcast) */
	uint64			tx_ucast_bytes;	/**< data bytes txed (ucast) */
	uint64			tx_mcast_bytes;	/**< # data bytes txed (mcast) */
	uint64			rx_ucast_bytes;	/**< data bytes recvd (ucast) */
	uint64			rx_mcast_bytes;	/**< data bytes recvd (mcast) */
	int8			rssi[WL_STA_ANT_MAX]; /**< average rssi per antenna
							* of data frames
							*/
	int8			nf[WL_STA_ANT_MAX];	/**< per antenna noise floor */
	uint16			aid;			/**< association ID */
	uint16			ht_capabilities;	/**< advertised ht caps */
	uint16			vht_flags;		/**< converted vht flags */
	uint16			PAD;
	uint32			tx_pkts_retried;	/**< # of frames where a retry was
							 * necessary
							 */
	uint32			tx_pkts_retry_exhausted; /**< # of user frames where a retry
							  * was exhausted
							  */
	int8			rx_lastpkt_rssi[WL_STA_ANT_MAX]; /**< Per antenna RSSI of last
								  * received data frame.
								  */
	/* TX WLAN retry/failure statistics:
	 * Separated for host requested frames and WLAN locally generated frames.
	 * Include unicast frame only where the retries/failures can be counted.
	 */
	uint32			tx_pkts_total;		/**< # user frames sent successfully */
	uint32			tx_pkts_retries;	/**< # user frames retries */
	uint32			tx_pkts_fw_total;	/**< # FW generated sent successfully */
	uint32			tx_pkts_fw_retries;	/**< # retries for FW generated frames */
	uint32			tx_pkts_fw_retry_exhausted;	/**< # FW generated where a retry
								 * was exhausted
								 */
	uint32			rx_pkts_retried;	/**< # rx with retry bit set */
	uint32			tx_rate_fallback;	/**< lowest fallback TX rate */
	/* Fields above this line are common to sta_info_t versions 4 and 5 */

	uint32			rx_dur_total;	/* total user RX duration (estimated) */

	chanspec_t		chanspec;       /** chanspec this sta is on */
	uint16			PAD;
	wl_rateset_args_v1_t	rateset_adv;	/* rateset along with mcs index bitmap */
	uint32			PAD;
} sta_info_v4_t;

/* Note: Version 4 is the latest version of sta_info_t.  Version 5 is abandoned.
 * Please add new fields to version 4, not version 5.
 */
/* sta_info_t version 5 */
typedef struct {
	uint16			ver;		/**< version of this struct */
	uint16			len;		/**< length in bytes of this structure */
	uint16			cap;		/**< sta's advertised capabilities */
	uint16			PAD;
	uint32			flags;		/**< flags defined below */
	uint32			idle;		/**< time since data pkt rx'd from sta */
	struct ether_addr	ea;		/**< Station address */
	uint16			PAD;
	wl_rateset_t		rateset;	/**< rateset in use */
	uint32			in;		/**< seconds elapsed since associated */
	uint32			listen_interval_inms; /**< Min Listen interval in ms for this STA */
	uint32			tx_pkts;	/**< # of user packets transmitted (unicast) */
	uint32			tx_failures;	/**< # of user packets failed */
	uint32			rx_ucast_pkts;	/**< # of unicast packets received */
	uint32			rx_mcast_pkts;	/**< # of multicast packets received */
	uint32			tx_rate;	/**< Rate used by last tx frame */
	uint32			rx_rate;	/**< Rate of last successful rx frame */
	uint32			rx_decrypt_succeeds;	/**< # of packet decrypted successfully */
	uint32			rx_decrypt_failures;	/**< # of packet decrypted unsuccessfully */
	uint32			tx_tot_pkts;	/**< # of user tx pkts (ucast + mcast) */
	uint32			rx_tot_pkts;	/**< # of data packets recvd (uni + mcast) */
	uint32			tx_mcast_pkts;	/**< # of mcast pkts txed */
	uint64			tx_tot_bytes;	/**< data bytes txed (ucast + mcast) */
	uint64			rx_tot_bytes;	/**< data bytes recvd (ucast + mcast) */
	uint64			tx_ucast_bytes;	/**< data bytes txed (ucast) */
	uint64			tx_mcast_bytes;	/**< # data bytes txed (mcast) */
	uint64			rx_ucast_bytes;	/**< data bytes recvd (ucast) */
	uint64			rx_mcast_bytes;	/**< data bytes recvd (mcast) */
	int8			rssi[WL_STA_ANT_MAX]; /**< average rssi per antenna
							* of data frames
							*/
	int8			nf[WL_STA_ANT_MAX];	/**< per antenna noise floor */
	uint16			aid;			/**< association ID */
	uint16			ht_capabilities;	/**< advertised ht caps */
	uint16			vht_flags;		/**< converted vht flags */
	uint16			PAD;
	uint32			tx_pkts_retried;	/**< # of frames where a retry was
							 * necessary
							 */
	uint32			tx_pkts_retry_exhausted; /**< # of user frames where a retry
							  * was exhausted
							  */
	int8			rx_lastpkt_rssi[WL_STA_ANT_MAX]; /**< Per antenna RSSI of last
								  * received data frame.
								  */
	/* TX WLAN retry/failure statistics:
	 * Separated for host requested frames and WLAN locally generated frames.
	 * Include unicast frame only where the retries/failures can be counted.
	 */
	uint32			tx_pkts_total;		/**< # user frames sent successfully */
	uint32			tx_pkts_retries;	/**< # user frames retries */
	uint32			tx_pkts_fw_total;	/**< # FW generated sent successfully */
	uint32			tx_pkts_fw_retries;	/**< # retries for FW generated frames */
	uint32			tx_pkts_fw_retry_exhausted;	/**< # FW generated where a retry
								 * was exhausted
								 */
	uint32			rx_pkts_retried;	/**< # rx with retry bit set */
	uint32			tx_rate_fallback;	/**< lowest fallback TX rate */
	/* Fields above this line are common to sta_info_t versions 4 and 5 */

	chanspec_t		chanspec;       /** chanspec this sta is on */
	uint16			PAD;
	wl_rateset_args_v1_t	rateset_adv;	/* rateset along with mcs index bitmap */
} sta_info_v5_t;

/* XXX
 * Please update the following when modifying this structure:
 *   StaInfo Twiki page flags section - description of the sta_info_t struct
 *    src/wl/exe/wlu.c - print of sta_info_t
 * Pay attention to version if structure changes.
 */

/* sta_info_t version 6
	changes to wl_rateset_args_t is leading to update this struct version as well.
 */
typedef struct {
	uint16			ver;		/**< version of this struct */
	uint16			len;		/**< length in bytes of this structure */
	uint16			cap;		/**< sta's advertised capabilities */
	uint16			PAD;
	uint32			flags;		/**< flags defined below */
	uint32			idle;		/**< time since data pkt rx'd from sta */
	struct ether_addr	ea;		/**< Station address */
	uint16			PAD;
	wl_rateset_t	rateset;	/**< rateset in use */
	uint32			in;		/**< seconds elapsed since associated */
	uint32			listen_interval_inms; /**< Min Listen interval in ms for this STA */
	uint32			tx_pkts;	/**< # of user packets transmitted (unicast) */
	uint32			tx_failures;	/**< # of user packets failed */
	uint32			rx_ucast_pkts;	/**< # of unicast packets received */
	uint32			rx_mcast_pkts;	/**< # of multicast packets received */
	uint32			tx_rate;	/**< Rate used by last tx frame */
	uint32			rx_rate;	/**< Rate of last successful rx frame */
	uint32			rx_decrypt_succeeds;	/**< # of packet decrypted successfully */
	uint32			rx_decrypt_failures;	/**< # of packet decrypted unsuccessfully */
	uint32			tx_tot_pkts;	/**< # of user tx pkts (ucast + mcast) */
	uint32			rx_tot_pkts;	/**< # of data packets recvd (uni + mcast) */
	uint32			tx_mcast_pkts;	/**< # of mcast pkts txed */
	uint64			tx_tot_bytes;	/**< data bytes txed (ucast + mcast) */
	uint64			rx_tot_bytes;	/**< data bytes recvd (ucast + mcast) */
	uint64			tx_ucast_bytes;	/**< data bytes txed (ucast) */
	uint64			tx_mcast_bytes;	/**< # data bytes txed (mcast) */
	uint64			rx_ucast_bytes;	/**< data bytes recvd (ucast) */
	uint64			rx_mcast_bytes;	/**< data bytes recvd (mcast) */
	int8			rssi[WL_STA_ANT_MAX]; /**< average rssi per antenna
							* of data frames
							*/
	int8			nf[WL_STA_ANT_MAX];	/**< per antenna noise floor */
	uint16			aid;			/**< association ID */
	uint16			ht_capabilities;	/**< advertised ht caps */
	uint16			vht_flags;		/**< converted vht flags */
	uint16			PAD;
	uint32			tx_pkts_retried;	/**< # of frames where a retry was
							 * necessary
							 */
	uint32			tx_pkts_retry_exhausted; /**< # of user frames where a retry
							  * was exhausted
							  */
	int8			rx_lastpkt_rssi[WL_STA_ANT_MAX]; /**< Per antenna RSSI of last
								  * received data frame.
								  */
	/* TX WLAN retry/failure statistics:
	 * Separated for host requested frames and WLAN locally generated frames.
	 * Include unicast frame only where the retries/failures can be counted.
	 */
	uint32			tx_pkts_total;		/**< # user frames sent successfully */
	uint32			tx_pkts_retries;	/**< # user frames retries */
	uint32			tx_pkts_fw_total;	/**< # FW generated sent successfully */
	uint32			tx_pkts_fw_retries;	/**< # retries for FW generated frames */
	uint32			tx_pkts_fw_retry_exhausted;	/**< # FW generated where a retry
								 * was exhausted
								 */
	uint32			rx_pkts_retried;	/**< # rx with retry bit set */
	uint32			tx_rate_fallback;	/**< lowest fallback TX rate */
	/* Fields above this line are common to sta_info_t versions 4 and 5 */

	uint32			rx_dur_total;	/* total user RX duration (estimated) */

	chanspec_t		chanspec;       /** chanspec this sta is on */
	uint16			PAD;
	wl_rateset_args_v2_t	rateset_adv;	/* rateset along with mcs index bitmap */
} sta_info_v6_t;

/* sta_info_t version 7
 * sta_info_t version 6 is not compatible with all the router branches so we extended version 6
 * to support all the router branches
 */
typedef struct {
	uint16                  ver;            /**< version of this struct */
	uint16                  len;            /**< length in bytes of this structure */
	uint16                  cap;            /**< sta's advertised capabilities */
	uint16                  PAD;
	uint32                  flags;          /**< flags defined below */
	uint32                  idle;           /**< time since data pkt rx'd from sta */
	struct ether_addr       ea;             /**< Station address */
	uint16                  PAD;
	wl_rateset_t		rateset;        /**< rateset in use */
	uint32                  in;             /**< seconds elapsed since associated */
	uint32                  listen_interval_inms; /**< Min Listen interval in ms for this STA */
	uint32                  tx_pkts;        /**< # of user packets transmitted (unicast) */
	uint32                  tx_failures;    /**< # of user packets failed */
	uint32                  rx_ucast_pkts;  /**< # of unicast packets received */
	uint32                  rx_mcast_pkts;  /**< # of multicast packets received */
	uint32                  tx_rate;        /**< Rate used by last tx frame */
	uint32                  rx_rate;        /**< Rate of last successful rx frame */
	uint32                  rx_decrypt_succeeds;    /**< # of packet decrypted successfully */
	uint32                  rx_decrypt_failures;    /**< # of packet decrypted unsuccessfully */
	uint32                  tx_tot_pkts;    /**< # of user tx pkts (ucast + mcast) */
	uint32                  rx_tot_pkts;    /**< # of data packets recvd (uni + mcast) */
	uint32                  tx_mcast_pkts;  /**< # of mcast pkts txed */
	uint64                  tx_tot_bytes;   /**< data bytes txed (ucast + mcast) */
	uint64                  rx_tot_bytes;   /**< data bytes recvd (ucast + mcast) */
	uint64                  tx_ucast_bytes; /**< data bytes txed (ucast) */
	uint64                  tx_mcast_bytes; /**< # data bytes txed (mcast) */
	uint64                  rx_ucast_bytes; /**< data bytes recvd (ucast) */
	uint64                  rx_mcast_bytes; /**< data bytes recvd (mcast) */
	int8                    rssi[WL_STA_ANT_MAX]; /**< average rssi per antenna
						       * of data frames
						       */
	int8                    nf[WL_STA_ANT_MAX];     /**< per antenna noise floor */
	uint16                  aid;                    /**< association ID */
	uint16                  ht_capabilities;        /**< advertised ht caps */
	uint16                  vht_flags;              /**< converted vht flags */
	uint16                  PAD;
	uint32                  tx_pkts_retried;        /**< # of frames where a retry was
							 * necessary
							 */
	uint32                  tx_pkts_retry_exhausted; /**< # of user frames where a retry
							  * was exhausted
							  */
	int8                    rx_lastpkt_rssi[WL_STA_ANT_MAX]; /**< Per antenna RSSI of last
								  * received data frame.
								  */
	/* TX WLAN retry/failure statistics:
	 * Separated for host requested frames and WLAN locally generated frames.
	 * Include unicast frame only where the retries/failures can be counted.
	 */
	uint32                  tx_pkts_total;          /**< # user frames sent successfully */
	uint32                  tx_pkts_retries;        /**< # user frames retries */
	uint32                  tx_pkts_fw_total;       /**< # FW generated sent successfully */
	uint32                  tx_pkts_fw_retries;     /**< # retries for FW generated frames */
	uint32                  tx_pkts_fw_retry_exhausted;     /**< # FW generated where a retry
								 * was exhausted
								 */
	uint32                  rx_pkts_retried;        /**< # rx with retry bit set */
	uint32                  tx_rate_fallback;       /**< lowest fallback TX rate */
	/* Fields above this line are common to sta_info_t versions 4 and 5 */

	uint32                  rx_dur_total;   /* total user RX duration (estimated) */

	chanspec_t              chanspec;       /** chanspec this sta is on */
	uint16                  PAD;
	wl_rateset_args_v2_t    rateset_adv;    /* rateset along with mcs index bitmap */
	uint16                  wpauth;                 /* authentication type */
	uint8                   algo;                   /* crypto algorithm */
	uint8			PAD;
	uint32                  tx_rspec;       /* Rate of last successful tx frame */
	uint32                  rx_rspec;       /* Rate of last successful rx frame */
	uint32                  wnm_cap;        /* wnm capabilities */

	uint16                  he_flags;	/* converted he flags */
	uint16                  PAD;
	sta_vendor_oui_t        sta_vendor_oui;
	uint8			link_bw;
} sta_info_v7_t;

/* sta_info_t version 8
 * change in size of wpauth
 */
typedef struct {
	uint16                  ver;            /**< version of this struct */
	uint16                  len;            /**< length in bytes of this structure */
	uint16                  cap;            /**< sta's advertised capabilities */
	uint16                  PAD;
	uint32                  flags;          /**< flags defined below */
	uint32                  idle;           /**< time since data pkt rx'd from sta */
	struct ether_addr       ea;             /**< Station address */
	uint16                  PAD;
	wl_rateset_t		rateset;        /**< rateset in use */
	uint32                  in;             /**< seconds elapsed since associated */
	uint32                  listen_interval_inms; /**< Min Listen interval in ms for this STA */
	uint32                  tx_pkts;        /**< # of user packets transmitted (unicast) */
	uint32                  tx_failures;    /**< # of user packets failed */
	uint32                  rx_ucast_pkts;  /**< # of unicast packets received */
	uint32                  rx_mcast_pkts;  /**< # of multicast packets received */
	uint32                  tx_rate;        /**< Rate used by last tx frame */
	uint32                  rx_rate;        /**< Rate of last successful rx frame */
	uint32                  rx_decrypt_succeeds;    /**< # of packet decrypted successfully */
	uint32                  rx_decrypt_failures;    /**< # of packet decrypted unsuccessfully */
	uint32                  tx_tot_pkts;    /**< # of user tx pkts (ucast + mcast) */
	uint32                  rx_tot_pkts;    /**< # of data packets recvd (uni + mcast) */
	uint32                  tx_mcast_pkts;  /**< # of mcast pkts txed */
	uint64                  tx_tot_bytes;   /**< data bytes txed (ucast + mcast) */
	uint64                  rx_tot_bytes;   /**< data bytes recvd (ucast + mcast) */
	uint64                  tx_ucast_bytes; /**< data bytes txed (ucast) */
	uint64                  tx_mcast_bytes; /**< # data bytes txed (mcast) */
	uint64                  rx_ucast_bytes; /**< data bytes recvd (ucast) */
	uint64                  rx_mcast_bytes; /**< data bytes recvd (mcast) */
	int8                    rssi[WL_STA_ANT_MAX]; /**< average rssi per antenna
						       * of data frames
						       */
	int8                    nf[WL_STA_ANT_MAX];     /**< per antenna noise floor */
	uint16                  aid;                    /**< association ID */
	uint16                  ht_capabilities;        /**< advertised ht caps */
	uint16                  vht_flags;              /**< converted vht flags */
	uint16                  PAD;
	uint32                  tx_pkts_retried;        /**< # of frames where a retry was
							 * necessary
							 */
	uint32                  tx_pkts_retry_exhausted; /**< # of user frames where a retry
							  * was exhausted
							  */
	int8                    rx_lastpkt_rssi[WL_STA_ANT_MAX]; /**< Per antenna RSSI of last
								  * received data frame.
								  */
	/* TX WLAN retry/failure statistics:
	 * Separated for host requested frames and WLAN locally generated frames.
	 * Include unicast frame only where the retries/failures can be counted.
	 */
	uint32                  tx_pkts_total;          /**< # user frames sent successfully */
	uint32                  tx_pkts_retries;        /**< # user frames retries */
	uint32                  tx_pkts_fw_total;       /**< # FW generated sent successfully */
	uint32                  tx_pkts_fw_retries;     /**< # retries for FW generated frames */
	uint32                  tx_pkts_fw_retry_exhausted;     /**< # FW generated where a retry
								 * was exhausted
								 */
	uint32                  rx_pkts_retried;        /**< # rx with retry bit set */
	uint32                  tx_rate_fallback;       /**< lowest fallback TX rate */
	/* Fields above this line are common to sta_info_t versions 4 and 5 */

	uint32                  rx_dur_total;   /* total user RX duration (estimated) */

	chanspec_t              chanspec;       /** chanspec this sta is on */
	uint16                  PAD;
	wl_rateset_args_v2_t    rateset_adv;    /* rateset along with mcs index bitmap */
	uint16                  PAD;
	uint8                   algo;                   /* crypto algorithm */
	uint8			PAD;
	uint32                  tx_rspec;       /* Rate of last successful tx frame */
	uint32                  rx_rspec;       /* Rate of last successful rx frame */
	uint32                  wnm_cap;        /* wnm capabilities */

	uint32                  he_flags;	/* converted he flags */
	sta_vendor_oui_t        sta_vendor_oui;
	uint8			link_bw;
	uint32			wpauth;		/* authentication type */
	int8			srssi;		/* smoothed rssi info */
	uint8			twt_info;
	uint16                  he_omi;

	/* #ifdef WL_EAP_STATS */
	uint32			tx_mgmt_pkts;	/**< # of management packets txed by driver */
	uint32			tx_ctl_pkts;	/**< # of control packets txed by driver */
	uint32			rx_mgmt_pkts;	/**< # of management packets rxed by driver */
	uint32			rx_ctl_pkts;	/**< # of control packets rxed by driver */
	/* #endif WL_EAP_STATS */

	uint8		rrm_capabilities[DOT11_RRM_CAP_LEN];	/* rrm capabilities of station */
	uint8			PAD;
	uint16			map_flags;	/* MultiAP Flags. WL_MAP_STA_XXX */
} sta_info_v8_t;

/* XXX: define to help support one version older sta_info_t from user level
 * applications.
 */
#define WL_OLD_STAINFO_SIZE	OFFSETOF(sta_info_t, tx_tot_pkts)

#define WL_STA_VER_4		4
#define WL_STA_VER_5		5
#define WL_STA_VER_7		7
#define WL_STA_VER_8		8
#define WL_STA_VER		WL_STA_VER_8

#define WL_STA_INFO_ALL_VER_1	1
typedef struct {
	uint16          version;	/* version of this struct */
	uint16          length;		/* length in bytes of this structure */
	uint16		count;		/* number of sta info elements */
	uint16          sta_info_ver;	/* variable data sta_info version */
	uint16		data_offset;	/* Offset to 'variable data' from beginning of this
					 * struct. Fields may be added between data_offset
					 * and sta_info.
					 */
	uint16		PAD;		/* reserved/padding for 4 byte align */
	uint8		data[];	/* sta_info */
} sta_info_all_t;

#define SWDIV_STATS_VERSION_2 2
#define SWDIV_STATS_CURRENT_VERSION SWDIV_STATS_VERSION_2

struct wlc_swdiv_stats_v1 {
	uint32 auto_en;
	uint32 active_ant;
	uint32 rxcount;
	int32 avg_snr_per_ant0;
	int32 avg_snr_per_ant1;
	int32 avg_snr_per_ant2;
	uint32 swap_ge_rxcount0;
	uint32 swap_ge_rxcount1;
	uint32 swap_ge_snrthresh0;
	uint32 swap_ge_snrthresh1;
	uint32 swap_txfail0;
	uint32 swap_txfail1;
	uint32 swap_timer0;
	uint32 swap_timer1;
	uint32 swap_alivecheck0;
	uint32 swap_alivecheck1;
	uint32 rxcount_per_ant0;
	uint32 rxcount_per_ant1;
	uint32 acc_rxcount;
	uint32 acc_rxcount_per_ant0;
	uint32 acc_rxcount_per_ant1;
	uint32 tx_auto_en;
	uint32 tx_active_ant;
	uint32 rx_policy;
	uint32 tx_policy;
	uint32 cell_policy;
	uint32 swap_snrdrop0;
	uint32 swap_snrdrop1;
	uint32 mws_antsel_ovr_tx;
	uint32 mws_antsel_ovr_rx;
	uint8 swap_trig_event_id;
};

struct wlc_swdiv_stats_v2 {
	uint16	version;	/* version of the structure
						* as defined by SWDIV_STATS_CURRENT_VERSION
						*/
	uint16	length;		/* length of the entire structure */
	uint32 auto_en;
	uint32 active_ant;
	uint32 rxcount;
	int32 avg_snr_per_ant0;
	int32 avg_snr_per_ant1;
	int32 avg_snr_per_ant2;
	uint32 swap_ge_rxcount0;
	uint32 swap_ge_rxcount1;
	uint32 swap_ge_snrthresh0;
	uint32 swap_ge_snrthresh1;
	uint32 swap_txfail0;
	uint32 swap_txfail1;
	uint32 swap_timer0;
	uint32 swap_timer1;
	uint32 swap_alivecheck0;
	uint32 swap_alivecheck1;
	uint32 rxcount_per_ant0;
	uint32 rxcount_per_ant1;
	uint32 acc_rxcount;
	uint32 acc_rxcount_per_ant0;
	uint32 acc_rxcount_per_ant1;
	uint32 tx_auto_en;
	uint32 tx_active_ant;
	uint32 rx_policy;
	uint32 tx_policy;
	uint32 cell_policy;
	uint32 swap_snrdrop0;
	uint32 swap_snrdrop1;
	uint32 mws_antsel_ovr_tx;
	uint32 mws_antsel_ovr_rx;
	uint32 swap_trig_event_id;
};

#define	WLC_NUMRATES	16	/**< max # of rates in a rateset */

typedef struct wlc_supp_chan_list {
	uint8 version;			/* version of wl sta_supp_chan */
	uint8 length;			/* Length of this buffer */
	uint8 count;			/* count of supported channels */
	uint8 chanset[MAXCHANNEL/8];	/* bitmap of Supported channels set */
} wlc_supp_chan_list_t;

#define WL_STA_SUPP_CHAN_VER	1

/**Used to get specific STA parameters */
typedef struct {
	uint32	val;
	struct ether_addr ea;
	uint16	PAD;
} scb_val_t;

/**Used by iovar versions of some ioctls, i.e. WLC_SCB_AUTHORIZE et al */
typedef struct {
	uint32 code;
	scb_val_t ioctl_args;
} authops_t;

/** channel encoding */
typedef struct channel_info {
	int32 hw_channel;
	int32 target_channel;
	int32 scan_channel;
} channel_info_t;

/** For ioctls that take a list of MAC addresses */
typedef struct maclist {
	uint32 count;			/**< number of MAC addresses */
	struct ether_addr ea[1];	/**< variable length array of MAC addresses */
} maclist_t;

typedef struct wds_client_info {
	char	ifname[INTF_NAME_SIZ];	/* WDS ifname */
	struct	ether_addr ea;		/* WDS client MAC address */
} wds_client_info_t;

#define WDS_MACLIST_MAGIC	0xFFFFFFFF
#define WDS_MACLIST_VERSION	1

/* For wds MAC list ioctls */
typedef struct wds_maclist {
	uint32 count;				/* Number of WDS clients */
	uint32 magic;				/* Magic number */
	uint32 version;				/* Version number */
	struct wds_client_info  client_list[1]; /* Variable length array of WDS clients */
} wds_maclist_t;

/**get pkt count struct passed through ioctl */
typedef struct get_pktcnt {
	uint32 rx_good_pkt;
	uint32 rx_bad_pkt;
	uint32 tx_good_pkt;
	uint32 tx_bad_pkt;
	uint32 rx_ocast_good_pkt; /**< unicast packets destined for others */
} get_pktcnt_t;

/* NINTENDO2 */
#define LQ_IDX_MIN              0
#define LQ_IDX_MAX              1
#define LQ_IDX_AVG              2
#define LQ_IDX_SUM              2
#define LQ_IDX_LAST             3
#define LQ_STOP_MONITOR         0
#define LQ_START_MONITOR        1

/** Get averages RSSI, Rx PHY rate and SNR values */
/* Link Quality */
typedef struct {
	int32 rssi[LQ_IDX_LAST];  /**< Array to keep min, max, avg rssi */
	int32 snr[LQ_IDX_LAST];   /**< Array to keep min, max, avg snr */
	int32 isvalid;            /**< Flag indicating whether above data is valid */
} wl_lq_t;

typedef enum wl_wakeup_reason_type {
	LCD_ON = 1,
	LCD_OFF,
	DRC1_WAKE,
	DRC2_WAKE,
	REASON_LAST
} wl_wr_type_t;

typedef struct {
	/** Unique filter id */
	uint32	id;
	/** stores the reason for the last wake up */
	uint8	reason;
	uint8	PAD[3];
} wl_wr_t;

/** Get MAC specific rate histogram command */
typedef struct {
	struct	ether_addr ea;	/**< MAC Address */
	uint8	ac_cat;	/**< Access Category */
	uint8	num_pkts;	/**< Number of packet entries to be averaged */
} wl_mac_ratehisto_cmd_t;
#ifndef LINUX_POSTMOGRIFY_REMOVAL
/** Get MAC rate histogram response */
typedef struct {
	uint32	rate[DOT11_RATE_MAX + 1];	/**< Rates */
	uint32	mcs[WL_RATESET_SZ_HT_IOCTL * WL_TX_CHAINS_MAX];	/**< MCS counts */
	uint32	vht[WL_RATESET_SZ_VHT_MCS][WL_TX_CHAINS_MAX];	/**< VHT counts */
	uint32	tsf_timer[2][2];	/**< Start and End time for 8bytes value */
	uint32	prop11n_mcs[WLC_11N_LAST_PROP_MCS - WLC_11N_FIRST_PROP_MCS + 1]; /** MCS counts */
} wl_mac_ratehisto_res_t;

/* sta_info ecounters */
typedef struct {
	struct ether_addr   ea;				/* Station MAC addr */
	struct ether_addr   BSSID;			/* BSSID of the BSS */
	uint32              tx_pkts_fw_total;		/* # FW generated sent successfully */
	uint32              tx_pkts_fw_retries;		/* # retries for FW generated frames */
	uint32              tx_pkts_fw_retry_exhausted;	/* # FW generated which
							 * failed after retry
							 */
} sta_info_ecounters_t;

#define STAMON_MODULE_VER		1
#endif /* LINUX_POSTMOGRIFY_REMOVAL */

/**Linux network driver ioctl encoding */
typedef struct wl_ioctl {
	uint32 cmd;	/**< common ioctl definition */
	void *buf;	/**< pointer to user buffer */
	uint32 len;	/**< length of user buffer */
	uint8 set;		/**< 1=set IOCTL; 0=query IOCTL */
	uint32 used;	/**< bytes read or written (optional) */
	uint32 needed;	/**< bytes needed (optional) */
} wl_ioctl_t;

#ifdef CONFIG_COMPAT
typedef struct compat_wl_ioctl {
	uint32 cmd;	/**< common ioctl definition */
	uint32 buf;	/**< pointer to user buffer */
	uint32 len;	/**< length of user buffer */
	uint8 set;		/**< 1=set IOCTL; 0=query IOCTL */
	uint32 used;	/**< bytes read or written (optional) */
	uint32 needed;	/**< bytes needed (optional) */
} compat_wl_ioctl_t;
#endif /* CONFIG_COMPAT */

#define WL_NUM_RATES_CCK		4 /**< 1, 2, 5.5, 11 Mbps */
#define WL_NUM_RATES_OFDM		8 /**< 6, 9, 12, 18, 24, 36, 48, 54 Mbps SISO/CDD */
#define WL_NUM_RATES_MCS_1STREAM	8 /**< MCS 0-7 1-stream rates - SISO/CDD/STBC/MCS */
#define WL_NUM_RATES_EXTRA_VHT		2 /**< Additional VHT 11AC rates */
#define WL_NUM_RATES_VHT		10
#define WL_NUM_RATES_MCS32		1

/*
 * Structure for passing hardware and software
 * revision info up from the driver.
 */
typedef struct wlc_rev_info {
	uint32		vendorid;	/**< PCI vendor id */
	uint32		deviceid;	/**< device id of chip */
	uint32		radiorev;	/**< radio revision */
	uint32		chiprev;	/**< chip revision */
	uint32		corerev;	/**< core revision */
	uint32		boardid;	/**< board identifier (usu. PCI sub-device id) */
	uint32		boardvendor;	/**< board vendor (usu. PCI sub-vendor id) */
	uint32		boardrev;	/**< board revision */
	uint32		driverrev;	/**< driver version */
	uint32		ucoderev;	/**< microcode version */
	uint32		bus;		/**< bus type */
	uint32		chipnum;	/**< chip number */
	uint32		phytype;	/**< phy type */
	uint32		phyrev;		/**< phy revision */
	uint32		anarev;		/**< anacore rev */
	uint32		chippkg;	/**< chip package info */
	uint32		nvramrev;	/**< nvram revision number */
	uint32		phyminorrev;	/**< phy minor rev */
	uint32		coreminorrev;	/**< core minor rev */
	uint32		drvrev_major;	/**< driver version: major */
	uint32		drvrev_minor;	/**< driver version: minor */
	uint32		drvrev_rc;	/**< driver version: rc */
	uint32		drvrev_rc_inc;	/**< driver version: rc incremental */
	uint32		otpflag;	/**< driver specific OTP info */
	uint32		ucoderev2;	/**< microcode revision */
} wlc_rev_info_t;

#define WL_REV_INFO_LEGACY_LENGTH	48

#define WL_BRAND_MAX 10
typedef struct wl_instance_info {
	uint32 instance;
	int8 brand[WL_BRAND_MAX];
	int8 PAD[4-(WL_BRAND_MAX%4)];
} wl_instance_info_t;

/** structure to change size of tx fifo */
typedef struct wl_txfifo_sz {
	uint16	magic;
	uint16	fifo;
	uint16	size;
} wl_txfifo_sz_t;

/* Transfer info about an IOVar from the driver */
/**Max supported IOV name size in bytes, + 1 for nul termination */
#define WLC_IOV_NAME_LEN	(32 + 1)

typedef struct wlc_iov_trx_s {
	uint8 module;
	uint8 type;
	char name[WLC_IOV_NAME_LEN];
} wlc_iov_trx_t;

/** bump this number if you change the ioctl interface */
#define WLC_IOCTL_VERSION	2
#define WLC_IOCTL_VERSION_LEGACY_IOTYPES	1
#ifndef LINUX_POSTMOGRIFY_REMOVAL
/* ifdef EXT_STA */
typedef struct _wl_assoc_result {
	ulong associated;
	ulong NDIS_auth;
	ulong NDIS_infra;
} wl_assoc_result_t;
/* EXT_STA */
#endif /* LINUX_POSTMOGRIFY_REMOVAL */

#define WL_PHY_PAVARS_LEN	32	/**< Phytype, Bandrange, chain, a[0], b[0], c[0], d[0] .. */

#define WL_PHY_PAVAR_VER	1	/**< pavars version */
#define WL_PHY_PAVARS2_NUM	3	/**< a1, b0, b1 */
typedef struct wl_pavars2 {
	uint16 ver;		/**< version of this struct */
	uint16 len;		/**< len of this structure */
	uint16 inuse;		/**< driver return 1 for a1,b0,b1 in current band range */
	uint16 phy_type;	/**< phy type */
	uint16 bandrange;
	uint16 chain;
	uint16 inpa[WL_PHY_PAVARS2_NUM];	/**< phy pavars for one band range */
} wl_pavars2_t;

typedef struct wl_po {
	uint16	phy_type;	/**< Phy type */
	uint16	band;
	uint16	cckpo;
	uint16	PAD;
	uint32	ofdmpo;
	uint16	mcspo[8];
} wl_po_t;

#define WL_NUM_RPCALVARS 5	/**< number of rpcal vars */

typedef struct wl_rpcal {
	uint16 value;
	uint16 update;
} wl_rpcal_t;

#define WL_NUM_RPCALPHASEVARS 5	/* number of rpcal phase vars */

typedef struct wl_rpcal_phase {
	uint16 value;
	uint16 update;
} wl_rpcal_phase_t;

typedef struct wl_aci_args {
	int32 enter_aci_thresh; /* Trigger level to start detecting ACI */
	int32 exit_aci_thresh; /* Trigger level to exit ACI mode */
	int32 usec_spin; /* microsecs to delay between rssi samples */
	int32 glitch_delay; /* interval between ACI scans when glitch count is consistently high */
	uint16 nphy_adcpwr_enter_thresh;	/**< ADC power to enter ACI mitigation mode */
	uint16 nphy_adcpwr_exit_thresh;	/**< ADC power to exit ACI mitigation mode */
	uint16 nphy_repeat_ctr;		/**< Number of tries per channel to compute power */
	uint16 nphy_num_samples;	/**< Number of samples to compute power on one channel */
	uint16 nphy_undetect_window_sz;	/**< num of undetects to exit ACI Mitigation mode */
	uint16 nphy_b_energy_lo_aci;	/**< low ACI power energy threshold for bphy */
	uint16 nphy_b_energy_md_aci;	/**< mid ACI power energy threshold for bphy */
	uint16 nphy_b_energy_hi_aci;	/**< high ACI power energy threshold for bphy */
	uint16 nphy_noise_noassoc_glitch_th_up; /**< wl interference 4 */
	uint16 nphy_noise_noassoc_glitch_th_dn;
	uint16 nphy_noise_assoc_glitch_th_up;
	uint16 nphy_noise_assoc_glitch_th_dn;
	uint16 nphy_noise_assoc_aci_glitch_th_up;
	uint16 nphy_noise_assoc_aci_glitch_th_dn;
	uint16 nphy_noise_assoc_enter_th;
	uint16 nphy_noise_noassoc_enter_th;
	uint16 nphy_noise_assoc_rx_glitch_badplcp_enter_th;
	uint16 nphy_noise_noassoc_crsidx_incr;
	uint16 nphy_noise_assoc_crsidx_incr;
	uint16 nphy_noise_crsidx_decr;
} wl_aci_args_t;

#define WL_ACI_ARGS_LEGACY_LENGTH	16	/**< bytes of pre NPHY aci args */
#define	WL_SAMPLECOLLECT_T_VERSION	2	/**< version of wl_samplecollect_args_t struct */
typedef struct wl_samplecollect_args {
	/* version 0 fields */
	uint8 coll_us;
	uint8 PAD[3];
	int32 cores;
	/* add'l version 1 fields */
	uint16 version;     /**< see definition of WL_SAMPLECOLLECT_T_VERSION */
	uint16 length;      /**< length of entire structure */
	int8 trigger;
	uint8 PAD;
	uint16 timeout;
	uint16 mode;
	uint16 PAD;
	uint32 pre_dur;
	uint32 post_dur;
	uint8 gpio_sel;
	uint8 downsamp;
	uint8 be_deaf;
	uint8 agc;		/**< loop from init gain and going down */
	uint8 filter;		/**< override high pass corners to lowest */
	/* add'l version 2 fields */
	uint8 trigger_state;
	uint8 module_sel1;
	uint8 module_sel2;
	uint16 nsamps;
	uint16 PAD;
	int32 bitStart;
	uint32 gpioCapMask;
	uint8 gpio_collection;
	uint8 PAD[3];
} wl_samplecollect_args_t;

#define	WL_SAMPLEDATA_T_VERSION		1	/**< version of wl_samplecollect_args_t struct */
/* version for unpacked sample data, int16 {(I,Q),Core(0..N)} */
#define	WL_SAMPLEDATA_T_VERSION_SPEC_AN 2

typedef struct wl_sampledata {
	uint16 version;	/**< structure version */
	uint16 size;	/**< size of structure */
	uint16 tag;	/**< Header/Data */
	uint16 length;	/**< data length */
	uint32 flag;	/**< bit def */
} wl_sampledata_t;

/* WL_OTA START */
/* OTA Test Status */
enum {
	WL_OTA_TEST_IDLE = 0,		/**< Default Idle state */
	WL_OTA_TEST_ACTIVE = 1,		/**< Test Running */
	WL_OTA_TEST_SUCCESS = 2,	/**< Successfully Finished Test */
	WL_OTA_TEST_FAIL = 3		/**< Test Failed in the Middle */
};

/* OTA SYNC Status */
enum {
	WL_OTA_SYNC_IDLE = 0,	/**< Idle state */
	WL_OTA_SYNC_ACTIVE = 1,	/**< Waiting for Sync */
	WL_OTA_SYNC_FAIL = 2	/**< Sync pkt not recieved */
};

/* Various error states dut can get stuck during test */
enum {
	WL_OTA_SKIP_TEST_CAL_FAIL = 1,		/**< Phy calibration failed */
	WL_OTA_SKIP_TEST_SYNCH_FAIL = 2,	/**< Sync Packet not recieved */
	WL_OTA_SKIP_TEST_FILE_DWNLD_FAIL = 3,	/**< Cmd flow file download failed */
	WL_OTA_SKIP_TEST_NO_TEST_FOUND = 4,	/**< No test found in Flow file */
	WL_OTA_SKIP_TEST_WL_NOT_UP = 5,		/**< WL UP failed */
	WL_OTA_SKIP_TEST_UNKNOWN_CALL		/**< Unintentional scheduling on ota test */
};

/* Differentiator for ota_tx and ota_rx */
enum {
	WL_OTA_TEST_TX = 0,		/**< ota_tx */
	WL_OTA_TEST_RX = 1,		/**< ota_rx */
};

/* Catch 3 modes of operation: 20Mhz, 40Mhz, 20 in 40 Mhz */
enum {
	WL_OTA_TEST_BW_20_IN_40MHZ = 0,		/**< 20 in 40 operation */
	WL_OTA_TEST_BW_20MHZ = 1,		/**< 20 Mhz operation */
	WL_OTA_TEST_BW_40MHZ = 2,		/**< full 40Mhz operation */
	WL_OTA_TEST_BW_80MHZ = 3		/* full 80Mhz operation */
};
#ifndef LINUX_POSTMOGRIFY_REMOVAL
#define HT_MCS_INUSE	0x00000080	/* HT MCS in use,indicates b0-6 holds an mcs */
#define VHT_MCS_INUSE	0x00000100	/* VHT MCS in use,indicates b0-6 holds an mcs */
#define OTA_RATE_MASK 0x0000007f	/* rate/mcs value */
#define OTA_STF_SISO	0
#define OTA_STF_CDD		1
#define OTA_STF_STBC	2
#define OTA_STF_SDM		3
#endif /* LINUX_POSTMOGRIFY_REMOVAL */

typedef struct ota_rate_info {
	uint8 rate_cnt;					/**< Total number of rates */
	uint8 PAD;
	uint16 rate_val_mbps[WL_OTA_TEST_MAX_NUM_RATE];	/**< array of rates from 1mbps to 130mbps */
							/**< for legacy rates : ratein mbps * 2 */
							/**< for HT rates : mcs index */
} ota_rate_info_t;

typedef struct ota_power_info {
	int8 pwr_ctrl_on;	/**< power control on/off */
	int8 start_pwr;		/**< starting power/index */
	int8 delta_pwr;		/**< delta power/index */
	int8 end_pwr;		/**< end power/index */
} ota_power_info_t;

typedef struct ota_packetengine {
	uint16 delay;           /**< Inter-packet delay */
				/**< for ota_tx, delay is tx ifs in micro seconds */
				/* for ota_rx, delay is wait time in milliseconds */
	uint16 nframes;         /**< Number of frames */
	uint16 length;          /**< Packet length */
} ota_packetengine_t;

/*
 * OTA txant/rxant parameter
 *    bit7-4: 4 bits swdiv_tx/rx_policy bitmask, specify antenna-policy for SW diversity
 *    bit3-0: 4 bits TxCore bitmask, specify cores used for transmit frames
 *            (maximum spatial expansion)
 */
#define WL_OTA_TEST_ANT_MASK	0xF0
#define WL_OTA_TEST_CORE_MASK	0x0F

/* OTA txant/rxant 'ant_mask' field; map to Tx/Rx antenna policy for SW diversity */
enum {
	WL_OTA_TEST_FORCE_ANT0 = 0x10,	/* force antenna to Ant 0 */
	WL_OTA_TEST_FORCE_ANT1 = 0x20,	/* force antenna to Ant 1 */
};

/* antenna/core fields access */
#define WL_OTA_TEST_GET_ANT(_txant) ((_txant) & WL_OTA_TEST_ANT_MASK)
#define WL_OTA_TEST_GET_CORE(_txant) ((_txant) & WL_OTA_TEST_CORE_MASK)

/** Test info vector */
typedef struct wl_ota_test_args {
	uint8 cur_test;			/**< test phase */
	uint8 chan;			/**< channel */
	uint8 bw;			/**< bandwidth */
	uint8 control_band;		/**< control band */
	uint8 stf_mode;			/**< stf mode */
	uint8 PAD;
	ota_rate_info_t rt_info;	/**< Rate info */
	ota_packetengine_t pkteng;	/**< packeteng info */
	uint8 txant;			/**< tx antenna */
	uint8 rxant;			/**< rx antenna */
	ota_power_info_t pwr_info;	/**< power sweep info */
	uint8 wait_for_sync;		/**< wait for sync or not */
	uint8 ldpc;
	uint8 sgi;
	uint8 PAD;
	/* Update WL_OTA_TESTVEC_T_VERSION for adding new members to this structure */
} wl_ota_test_args_t;

#define WL_OTA_TESTVEC_T_VERSION		1	/* version of wl_ota_test_vector_t struct */
typedef struct wl_ota_test_vector {
	uint16 version;
	wl_ota_test_args_t test_arg[WL_OTA_TEST_MAX_NUM_SEQ];	/**< Test argument struct */
	uint16 test_cnt;					/**< Total no of test */
	uint8 file_dwnld_valid;					/**< File successfully downloaded */
	uint8 sync_timeout;					/**< sync packet timeout */
	int8 sync_fail_action;					/**< sync fail action */
	struct ether_addr sync_mac;				/**< macaddress for sync pkt */
	struct ether_addr tx_mac;				/**< macaddress for tx */
	struct ether_addr rx_mac;				/**< macaddress for rx */
	int8 loop_test;					/**< dbg feature to loop the test */
	uint16 test_rxcnt;
	/* Update WL_OTA_TESTVEC_T_VERSION for adding new members to this structure */
} wl_ota_test_vector_t;

/** struct copied back form dongle to host to query the status */
typedef struct wl_ota_test_status {
	int16 cur_test_cnt;		/**< test phase */
	int8 skip_test_reason;		/**< skip test reasoin */
	uint8 PAD;
	wl_ota_test_args_t test_arg;	/**< cur test arg details */
	uint16 test_cnt;		/**< total no of test downloaded */
	uint8 file_dwnld_valid;		/**< file successfully downloaded ? */
	uint8 sync_timeout;		/**< sync timeout */
	int8 sync_fail_action;		/**< sync fail action */
	struct ether_addr sync_mac;	/**< macaddress for sync pkt */
	struct ether_addr tx_mac;	/**< tx mac address */
	struct ether_addr rx_mac;	/**< rx mac address */
	uint8  test_stage;		/**< check the test status */
	int8 loop_test;			/**< Debug feature to puts test enfine in a loop */
	uint8 sync_status;		/**< sync status */
} wl_ota_test_status_t;

/* For wds MAC list ioctls */
typedef struct dwds_maclist {
	uint16 version;		/* Version number */
	uint16 length;		/* Length of fixed and variable data */
	uint16 count;		/* Number of DWDS clients */
	uint16 data_offset;	/* offset to 'variable data' from beginning of this struct.
				 * fields may be added between data_offset and client_info.
				 */
	/* ADD NEW FIELDS HERE */
	struct wds_client_info  client_info[]; /* Client specific data; could be empty */
} dwds_maclist_t;

#define DWDS_MACLIST_VER_1		1

/* FOR ioctl that take the sta monitor information */
typedef struct stamon_data {
	struct ether_addr  ea;
	uint8 PAD[2];
	int32 rssi;
} stamon_data_t;

typedef struct stamon_info {
	int32 version;
	uint32 count;
	stamon_data_t sta_data[1];
} stamon_info_t;

/* stamon counters for data, mngt and ctl pkts */
typedef struct stamon_cnts {
	uint16 version;
	uint16 length;
	uint32 rxstamondata;
	uint32 rxstamonmngt;
	uint32 rxstamoncntl;
} stamon_cnts_t;

#ifndef LINUX_POSTMOGRIFY_REMOVAL
typedef struct wl_ota_rx_rssi {
	uint16	pktcnt;		/* Pkt count used for this rx test */
	chanspec_t chanspec;	/* Channel info on which the packets are received */
	int16	rssi;		/* Average RSSI of the first 50% packets received */
} wl_ota_rx_rssi_t;

#define	WL_OTARSSI_T_VERSION		1	/* version of wl_ota_test_rssi_t struct */
#define WL_OTA_TEST_RSSI_FIXED_SIZE	OFFSETOF(wl_ota_test_rssi_t, rx_rssi)

typedef struct wl_ota_test_rssi {
	uint8 version;
	uint8	testcnt;		/* total measured RSSI values, valid on output only */
	wl_ota_rx_rssi_t rx_rssi[1]; /* Variable length array of wl_ota_rx_rssi_t */
} wl_ota_test_rssi_t;
#endif /* LINUX_POSTMOGRIFY_REMOVAL */

/* WL_OTA END */

/**wl_radar_args_t */
typedef struct {
	int32 npulses;	/**< required number of pulses at n * t_int */
	int32 ncontig;	/**< required number of pulses at t_int */
	int32 min_pw;	/**< minimum pulse width (20 MHz clocks) */
	int32 max_pw;	/**< maximum pulse width (20 MHz clocks) */
	uint16 thresh0;	/**< Radar detection, thresh 0 */
	uint16 thresh1;	/**< Radar detection, thresh 1 */
	uint16 blank;	/**< Radar detection, blank control */
	uint16 fmdemodcfg;	/**< Radar detection, fmdemod config */
	int32 npulses_lp;  /**< Radar detection, minimum long pulses */
	int32 min_pw_lp; /**< Minimum pulsewidth for long pulses */
	int32 max_pw_lp; /**< Maximum pulsewidth for long pulses */
	int32 min_fm_lp; /**< Minimum fm for long pulses */
	int32 max_span_lp;  /**< Maximum deltat for long pulses */
	int32 min_deltat; /**< Minimum spacing between pulses */
	int32 max_deltat; /**< Maximum spacing between pulses */
	uint16 autocorr;	/**< Radar detection, autocorr on or off */
	uint16 st_level_time;	/**< Radar detection, start_timing level */
	uint16 t2_min; /**< minimum clocks needed to remain in state 2 */
	uint8 PAD[2];
	uint32 version; /**< version */
	uint32 fra_pulse_err;	/**< sample error margin for detecting French radar pulsed */
	int32 npulses_fra;  /**< Radar detection, minimum French pulses set */
	int32 npulses_stg2;  /**< Radar detection, minimum staggered-2 pulses set */
	int32 npulses_stg3;  /**< Radar detection, minimum staggered-3 pulses set */
	uint16 percal_mask;	/**< defines which period cal is masked from radar detection */
	uint8 PAD[2];
	int32 quant;	/**< quantization resolution to pulse positions */
	uint32 min_burst_intv_lp;	/**< minimum burst to burst interval for bin3 radar */
	uint32 max_burst_intv_lp;	/**< maximum burst to burst interval for bin3 radar */
	int32 nskip_rst_lp;	/**< number of skipped pulses before resetting lp buffer */
	int32 max_pw_tol; /* maximum tolerance allowd in detected pulse width for radar detection */
	uint16 feature_mask; /**< 16-bit mask to specify enabled features */
	uint16 thresh0_sc;	/**< Radar detection, thresh 0 */
	uint16 thresh1_sc;	/**< Radar detection, thresh 1 */
	uint8 PAD[2];
} wl_radar_args_t;

#define WL_RADAR_ARGS_VERSION 2

typedef struct {
	uint32 version; /**< version */
	uint16 thresh0_20_lo;	/**< Radar detection, thresh 0 (range 5250-5350MHz) for BW 20MHz */
	uint16 thresh1_20_lo;	/**< Radar detection, thresh 1 (range 5250-5350MHz) for BW 20MHz */
	uint16 thresh0_40_lo;	/**< Radar detection, thresh 0 (range 5250-5350MHz) for BW 40MHz */
	uint16 thresh1_40_lo;	/**< Radar detection, thresh 1 (range 5250-5350MHz) for BW 40MHz */
	uint16 thresh0_80_lo;	/**< Radar detection, thresh 0 (range 5250-5350MHz) for BW 80MHz */
	uint16 thresh1_80_lo;	/**< Radar detection, thresh 1 (range 5250-5350MHz) for BW 80MHz */
	uint16 thresh0_20_hi;	/**< Radar detection, thresh 0 (range 5470-5725MHz) for BW 20MHz */
	uint16 thresh1_20_hi;	/**< Radar detection, thresh 1 (range 5470-5725MHz) for BW 20MHz */
	uint16 thresh0_40_hi;	/**< Radar detection, thresh 0 (range 5470-5725MHz) for BW 40MHz */
	uint16 thresh1_40_hi;	/**< Radar detection, thresh 1 (range 5470-5725MHz) for BW 40MHz */
	uint16 thresh0_80_hi;	/**< Radar detection, thresh 0 (range 5470-5725MHz) for BW 80MHz */
	uint16 thresh1_80_hi;	/**< Radar detection, thresh 1 (range 5470-5725MHz) for BW 80MHz */
	uint16 thresh0_160_lo;	/**< Radar detection, thresh 0 (range 5250-5350MHz) for BW 160MHz */
	uint16 thresh1_160_lo;	/**< Radar detection, thresh 1 (range 5250-5350MHz) for BW 160MHz */
	uint16 thresh0_160_hi;	/**< Radar detection, thresh 0 (range 5470-5725MHz) for BW 160MHz */
	uint16 thresh1_160_hi;	/**< Radar detection, thresh 1 (range 5470-5725MHz) for BW 160MHz */
} wl_radar_thr_t;

typedef struct {
	uint32 version; /* version */
	uint16 thresh0_sc_20_lo;
	uint16 thresh1_sc_20_lo;
	uint16 thresh0_sc_40_lo;
	uint16 thresh1_sc_40_lo;
	uint16 thresh0_sc_80_lo;
	uint16 thresh1_sc_80_lo;
	uint16 thresh0_sc_20_hi;
	uint16 thresh1_sc_20_hi;
	uint16 thresh0_sc_40_hi;
	uint16 thresh1_sc_40_hi;
	uint16 thresh0_sc_80_hi;
	uint16 thresh1_sc_80_hi;
	uint16 thresh0_sc_160_lo;
	uint16 thresh1_sc_160_lo;
	uint16 thresh0_sc_160_hi;
	uint16 thresh1_sc_160_hi;
	uint16 fc_varth_sb;
	uint16 fc_varth_bin5_sb;
	uint16 notradar_enb;
	uint16 max_notradar_lp;
	uint16 max_notradar;
	uint16 max_notradar_lp_sc;
	uint16 max_notradar_sc;
	uint16 highpow_war_enb;
	uint16 highpow_sp_ratio;	//unit is 0.5
	uint16 fm_chk_opt;
	uint16 fm_chk_pw;
	uint16 fm_var_chk_pw;
	uint16 fm_thresh_sp1;
	uint16 fm_thresh_sp2;
	uint16 fm_thresh_sp3;
	uint16 fm_thresh_etsi4;
	uint16 fm_thresh_p1c;
	uint16 fm_tol_div;
} wl_radar_thr2_t;

#define WL_RADAR_THR_VERSION	2

typedef struct {
	uint32 ver;
	uint32 len;
	int32  rssi_th[3];
	uint8  rssi_gain_80[4];
	uint8  rssi_gain_160[4];
} wl_dyn_switch_th_t;

#define WL_PHY_DYN_SWITCH_TH_VERSION	1

/** RSSI per antenna */
typedef struct {
	uint32	version;		/**< version field */
	uint32	count;			/**< number of valid antenna rssi */
	int8 rssi_ant[WL_RSSI_ANT_MAX];	/**< rssi per antenna */
} wl_rssi_ant_t;

/* SNR per antenna */
typedef struct {
	uint32  version;				/* version field */
	uint32  count;					/* number of valid antenna snr */
	int8 snr_ant[WL_RSSI_ANT_MAX];	/* snr per antenna */
} wl_snr_ant_t;

#define MAX_NUM_ULOFDMA_USERS 16

typedef struct {
	uint8 num_users;
	uint8 mu_fstr_error;
	uint8 mu_error_code;
	uint8 user_mac_idx[MAX_NUM_ULOFDMA_USERS];
	int16 user_rssi[MAX_NUM_ULOFDMA_USERS];
	uint8 user_snr[MAX_NUM_ULOFDMA_USERS];
	int16 user_freq_error[MAX_NUM_ULOFDMA_USERS];
} wl_ulofdma_per_user_rxstats_t;

typedef struct {
	uint16   phy_temp_reduce_cnt;
	uint16   phy_temp_incr_cnt;
	uint16   phy_temp_1_tx_reduce;
	uint16   phy_temp_2_tx_reduce;
	uint16   phy_temp_3_tx_reduce;
} phy_txcore_temp_cnt_t;

typedef struct {
	bool   pos_freq;
	uint16 freq_offset_l, freq_offset_h;
	uint32 freq_offset_total;
	uint32 tracked_freq_offsetHz;
	uint32 pll_wild_base_offset;
} wl_tracked_freq_offset_t;

/* used with IOV_OMN_MASTER in get/set */
#define WL_OMNM_VER			1

/* lock on current omn_masters expires in these many seconds since last successful set */
#define WL_OMNM_LOCK_SECONDS		10

/* with WL_OMNM_ACT_SET, one of the following flags may be used to override others' lock */
#define WL_OMNM_FLAG_YANK		0	/* override if lock is held by lower rank module */
#define WL_OMNM_FLAG_SKIP		1	/* just skip even if locked by lower rank module */

/* order of these modules helps determine the priority; lower the number, higher the ranking */
typedef enum {
	WL_OMNM_MOD_NONE		= 0,	/* none - does not influence ranking */
	WL_OMNM_MOD_ZDFS		= 1,	/* ZDFS 3+1 mode switches NSS */
	WL_OMNM_MOD_AIRIQ		= 2,	/* AirIQ 3+1 mode switches NSS */
	WL_OMNM_MOD_OBSS_DBS		= 3,	/* downgrades BW on OBSS */
	WL_OMNM_MOD_DYN160		= 4,	/* 160-2x2 <-> 80-4x4 for certain chips */
	WL_OMNM_MOD_BW160		= 5,	/* 160-4x4 <-> 80-4x4 to ease LESI/TDCS */
	WL_OMNM_MOD_LAST
} wl_omnm_mod_t;

typedef enum {
	WL_OMNM_ACT_GET			= 0,	/* get current OMN master details */
	WL_OMNM_ACT_SET			= 1,	/* attempt to set a module as master */
	WL_OMNM_ACT_CLEAR		= 2,	/* clear mentioned module as master */
	WL_OMNM_ACT_LAST
} wl_omnm_act_t;

typedef struct {
	uint16 ver;				/* see WL_OMNM_VER */
	uint16 len;				/* helps append optional fields for the same ver */
	uint16 act;				/* see wl_omnm_act_t */
	uint16 flags;				/* see WL_OMNM_FLAG_* */
	uint16 mod;				/* GET/SET : current master; see wl_omnm_mod_t */
	uint16 lock_life;			/* GET only: master lock expires after these secs */
} wl_omnm_info_t;

/** data structure used in 'dfs_status' wl interface, which is used to query dfs status */
typedef struct {
	uint32 state;		/**< noted by WL_DFS_CACSTATE_XX. */
	uint32 duration;		/**< time spent in ms in state. */
	/**
	 * as dfs enters ISM state, it removes the operational channel from quiet channel
	 * list and notes the channel in channel_cleared. set to 0 if no channel is cleared
	 */
	chanspec_t chanspec_cleared;
	/** chanspec cleared used to be a uint32, add another to uint16 to maintain size */
	uint16 pad;
} wl_dfs_status_t;

typedef struct {
	uint32 state;		/* noted by WL_DFS_CACSTATE_XX */
	uint32 duration;		/* time spent in ms in state */
	chanspec_t chanspec;	/* chanspec of this core */
	chanspec_t chanspec_last_cleared; /* chanspec last cleared for operation by scanning */
	uint16 sub_type;	/* currently just the index of the core or the respective PLL */
	uint16 pad;
} wl_dfs_sub_status_t;

#define WL_DFS_STATUS_ALL_VERSION	(1)
typedef struct {
	uint16 version;		/* version field; current max version 1 */
	uint16 num_sub_status;
	wl_dfs_sub_status_t  dfs_sub_status[1]; /* struct array of length num_sub_status */
} wl_dfs_status_all_t;

#define WL_DFS_AP_MOVE_VERSION	(1)

struct wl_dfs_ap_move_status_v1 {
	int16 dfs_status;	/* DFS scan status */
	chanspec_t chanspec;	/* New AP Chanspec */
	wl_dfs_status_t cac_status;	/* CAC status */
};

typedef struct wl_dfs_ap_move_status_v2 {
	int8 version;            /* version field; current max version 1 */
	int8 move_status;        /* DFS move status */
	chanspec_t chanspec;     /* New AP Chanspec */
	wl_dfs_status_all_t scan_status; /* status; see dfs_status_all for wl_dfs_status_all_t */
} wl_dfs_ap_move_status_v2_t;

#define WL_DFS_AP_MOVE_ABORT -1		/* Abort any dfs_ap_move in progress immediately */
#define WL_DFS_AP_MOVE_STUNT -2		/* Stunt move but continue background CSA if in progress */

/* event data passed in WLC_E_CAC_STATE_CHANGE update */
#define WLC_E_CAC_STATE_VERSION	1
#define WLC_E_CAC_STATE_TYPE_DFS_STATUS_ALL 1
/*
 * struct wlc_cac_event - cac_state_change event.
 *
 * @version: version of this event structure.
 * @chanspec_cleared: current chanspec.
 * @state: New CAC state.
 * @scan_status: status; see dfs_status_all for wl_dfs_status_all_t.
 */
typedef struct wlc_cac_event {
	uint16 version;
	uint16 length;		/* Total length from version to end of variable scan_status */
	uint16 type;
	uint16 pad;		/* reserved */
	wl_dfs_status_all_t scan_status;
} wlc_cac_event_t;

#define WLC_E_MBO_BSS_STATUS_VERSION	1

/*
 * struct wlc_mbo_bss_status.
 *
 * @version: version of this event structure.
 * @length.
 * @assoc_disallowed:
 * @ap_attr
 * @mbo_bss_enable
 * @pad
 */
typedef struct wlc_mbo_bss_status {
	uint16	version;
	uint16	length;		/* Total length from version to end of structure */
	uint8	assoc_allowance_status;
	uint8	ap_attr;
	uint8	mbo_bss_enable;	/* currently bss is supporting mbo or not */
	uint8	pad;		/* reserved */
} wlc_mbo_bss_status_t;

/** data structure used in 'radar_status' wl interface, which is use to query radar det status */
typedef struct {
	uint8 detected;
	uint8 PAD[3];
	int32 count;
	uint8 pretended;
	uint8 PAD[3];
	uint32 radartype;
	uint32 timenow;
	uint32 timefromL;
	int32  lp_csect_single;
	int32  detected_pulse_index;
	int32  nconsecq_pulses;
	chanspec_t ch;
	uint8 PAD[2];
	int32  pw[10];
	int32  intv[10];
	int32  fm[10];
#ifdef BCMDBG
	int32  fc[10];
	int32  chirp[10];
#endif // endif
} wl_radar_status_t;

#define NUM_PWRCTRL_RATES 12

typedef struct {
	uint8 txpwr_band_max[NUM_PWRCTRL_RATES];	/**< User set target */
	uint8 txpwr_limit[NUM_PWRCTRL_RATES];		/**< reg and local power limit */
	uint8 txpwr_local_max;				/**< local max according to the AP */
	uint8 txpwr_local_constraint;			/**< local constraint according to the AP */
	uint8 txpwr_chan_reg_max;			/**< Regulatory max for this channel */
	uint8 txpwr_target[2][NUM_PWRCTRL_RATES];	/**< Latest target for 2.4 and 5 Ghz */
	uint8 txpwr_est_Pout[2];			/**< Latest estimate for 2.4 and 5 Ghz */
	uint8 txpwr_opo[NUM_PWRCTRL_RATES];		/**< On G phy, OFDM power offset */
	uint8 txpwr_bphy_cck_max[NUM_PWRCTRL_RATES];	/**< Max CCK power for this band (SROM) */
	uint8 txpwr_bphy_ofdm_max;			/**< Max OFDM power for this band (SROM) */
	uint8 txpwr_aphy_max[NUM_PWRCTRL_RATES];	/**< Max power for A band (SROM) */
	int8  txpwr_antgain[2];				/**< Ant gain for each band - from SROM */
	uint8 txpwr_est_Pout_gofdm;			/**< Pwr estimate for 2.4 OFDM */
} tx_power_legacy_t;

#define WL_TX_POWER_RATES_LEGACY    45
#define WL_TX_POWER_MCS20_FIRST         12
#define WL_TX_POWER_MCS20_NUM           16
#define WL_TX_POWER_MCS40_FIRST         28
#define WL_TX_POWER_MCS40_NUM           17

typedef struct {
	uint32 flags;
	chanspec_t chanspec;                 /**< txpwr report for this channel */
	chanspec_t local_chanspec;           /**< channel on which we are associated */
	uint8 local_max;                 /**< local max according to the AP */
	uint8 local_constraint;              /**< local constraint according to the AP */
	int8  antgain[2];                /**< Ant gain for each band - from SROM */
	uint8 rf_cores;                  /**< count of RF Cores being reported */
	uint8 est_Pout[4];                           /**< Latest tx power out estimate per RF
							  * chain without adjustment
							  */
	uint8 est_Pout_cck;                          /**< Latest CCK tx power out estimate */
	uint8 user_limit[WL_TX_POWER_RATES_LEGACY];  /**< User limit */
	uint8 reg_limit[WL_TX_POWER_RATES_LEGACY];   /**< Regulatory power limit */
	uint8 board_limit[WL_TX_POWER_RATES_LEGACY]; /**< Max power board can support (SROM) */
	uint8 target[WL_TX_POWER_RATES_LEGACY];      /**< Latest target power */
	uint8 PAD[2];
} tx_power_legacy2_t;

#define WL_NUM_2x2_ELEMENTS		4
#define WL_NUM_3x3_ELEMENTS		6
#define WL_NUM_4x4_ELEMENTS		10

typedef struct {
	uint16 ver;				/**< version of this struct */
	uint16 len;				/**< length in bytes of this structure */
	uint32 flags;
	chanspec_t chanspec;			/**< txpwr report for this channel */
	chanspec_t local_chanspec;		/**< channel on which we are associated */
	uint32     buflen;			/**< ppr buffer length */
	uint8      pprbuf[1];			/**< Latest target power buffer */
} wl_txppr_t;

#define WL_TXPPR_VERSION	1
#define WL_TXPPR_LENGTH	(sizeof(wl_txppr_t))
#define TX_POWER_T_VERSION	45
#define TX_CTRLWD_T_VERSION	1

/* curpower ppr types */
enum {
	PPRTYPE_TARGETPOWER	=	1,
	PPRTYPE_BOARDLIMITS	=	2,
	PPRTYPE_REGLIMITS	=	3,
	PPRTYPE_RU_REGLIMITS    =       4,
	PPRTYPE_RU_BOARDLIMITS  =       5,
	PPRTYPE_RU_TARGETPOWER  =       6,
	PPRTYPE_LAST
};

/* number of ppr serialization buffers */
#define WL_TXPPR_SER_BUF_NUM    (PPRTYPE_LAST - 1)
/* PPRTYPE_TARGETPOWER, PPRTYPE_BOARDLIMITS, PPRTYPE_REGLIMITS */
#define WL_LEGACY_TXPPR_SER_BUF_NUM	3

typedef struct chanspec_txpwr_max {
	chanspec_t chanspec;   /**< chanspec */
	uint8 txpwr_max;       /**< max txpwr in all the rates */
	uint8 padding;
} chanspec_txpwr_max_t;

typedef struct  wl_chanspec_txpwr_max {
	uint16 ver;			/**< version of this struct */
	uint16 len;			/**< length in bytes of this structure */
	uint32 count;		/**< number of elements of (chanspec, txpwr_max) pair */
	chanspec_txpwr_max_t txpwr[1];	/**< array of (chanspec, max_txpwr) pair */
} wl_chanspec_txpwr_max_t;

#define WL_CHANSPEC_TXPWR_MAX_VER	1
#define WL_CHANSPEC_TXPWR_MAX_LEN	(sizeof(wl_chanspec_txpwr_max_t))

typedef struct tx_inst_power {
	uint8 txpwr_est_Pout[2];			/**< Latest estimate for 2.4 and 5 Ghz */
	uint8 txpwr_est_Pout_gofdm;			/**< Pwr estimate for 2.4 OFDM */
} tx_inst_power_t;

#define WL_NUM_TXCHAIN_MAX	4
typedef struct wl_txchain_pwr_offsets {
	int8 offset[WL_NUM_TXCHAIN_MAX];	/**< quarter dBm signed offset for each chain */
} wl_txchain_pwr_offsets_t;

/** maximum channels returned by the get valid channels iovar */
#define WL_NUMCHANNELS		64
#define WL_NUMCHANNELS_MANY_CHAN 10
#define WL_ITER_LIMIT_MANY_CHAN 5

#define WL_MIMO_PS_CFG_VERSION_1 1

typedef struct wl_mimops_cfg {
	uint8 version;
	/* active_chains: 0 for all, 1 for 1 chain. */
	uint8 active_chains;
	/* static (0) or dynamic (1).or disabled (3) Mode applies only when active_chains = 0. */
	uint8 mode;
	/* bandwidth = Full (0), 20M (1), 40M (2), 80M (3). */
	uint8 bandwidth;
	uint8 applychangesafterlearning;
	uint8 pad[3];
} wl_mimops_cfg_t;

/* This event is for tracing MIMO PS metrics snapshot calls.
 * It is helpful to debug out-of-sync issue between
 * ucode SHM values and FW snapshot calculation.
 * It is part of the EVENT_LOG_TAG_MIMO_PS_TRACE.
 */
#define WL_MIMO_PS_METRICS_SNAPSHOT_TRACE_TYPE	0
typedef struct wl_mimo_ps_metrics_snapshot_trace {
	/* type field for this TLV: */
	uint16  type;
	/* length field for this TLV */
	uint16  len;
	uint32  idle_slotcnt_mimo;	/* MIMO idle slotcnt raw SHM value */
	uint32  last_idle_slotcnt_mimo;	/* stored value snapshot */
	uint32  idle_slotcnt_siso;	/* SISO idle slotcnt raw SHM value */
	uint32  last_idle_slotcnt_siso;	/* stored value snapshot */
	uint32	rx_time_mimo;		/* Rx MIMO raw SHM value */
	uint32	last_rx_time_mimo;	/* stored value snapshot */
	uint32	rx_time_siso;		/* RX SISO raw SHM value */
	uint32	last_rx_time_siso;	/* stored value snapshot */
	uint32  tx_time_1chain;		/* Tx 1-chain raw SHM value */
	uint32  last_tx_time_1chain;	/* stored value snapshot */
	uint32	tx_time_2chain;		/* Tx 2-chain raw SHM value */
	uint32	last_tx_time_2chain;	/* stored value snapshot */
	uint32  tx_time_3chain;		/* Tx 3-chain raw SHM value */
	uint32  last_tx_time_3chain;	/* stored value snapshot */
	uint16	reason;			/* reason for snapshot call, see below */
	/* Does the call reset last values after delta calculation */
	uint16	reset_last;
} wl_mimo_ps_metrics_snapshot_trace_t;
/* reason codes for mimo ps metrics snapshot function calls */
#define WL_MIMOPS_METRICS_SNAPSHOT_REPORT		1
#define WL_MIMOPS_METRICS_SNAPSHOT_RXCHAIN_SET		2
#define WL_MIMOPS_METRICS_SNAPSHOT_ARBI			3
#define WL_MIMOPS_METRICS_SNAPSHOT_SLOTUPD		4
#define WL_MIMOPS_METRICS_SNAPSHOT_PMBCNRX		5
#define WL_MIMOPS_METRICS_SNAPSHOT_BMACINIT		6
#define WL_MIMOPS_METRICS_SNAPSHOT_HT_COMPLETE		7
#define WL_MIMOPS_METRICS_SNAPSHOT_OCL                  8

#define WL_MIMO_PS_STATUS_VERSION_2	2
typedef struct wl_mimo_ps_status {
	uint8 version;
	uint8 ap_cap;			/* The associated AP's capability (BW, MIMO/SISO). */
	uint8 association_status;	/* How we are associated to the AP (MIMO/SISO). */
	uint8 mimo_ps_state;		/* mimo_ps_cfg states: [0-5]. See below for values */
	uint8 mrc_state;		/* MRC state: NONE (0), ACTIVE(1) */
	uint8 bss_rxchain;		/* bss rxchain bitmask */
	uint8 bss_txchain;		/* bss txchain bitmask */
	uint8 bss_bw;			/* bandwidth: Full (0), 20M (1), 40M (2), 80M (3), etc */
	uint16 hw_state;		/* bitmask of hw state. See below for values */
	uint8 hw_rxchain;		/* actual HW rxchain bitmask */
	uint8 hw_txchain;		/* actual HW txchain bitmask */
	uint8 hw_bw;			/* bandwidth: Full (0), 20M (1), 40M (2), 80M (3), etc */
	uint8 pm_bcnrx_state;		/* actual state of ucode flag */
	uint8 basic_rates_present;	/* internal flag to trigger siso bcmc rx */
	uint8 siso_bcmc_rx_state;	/* actual state of ucode flag */
} wl_mimo_ps_status_t;

#define WL_MIMO_PS_STATUS_VERSION_1	1
typedef struct wl_mimo_ps_status_v1 {
	uint8 version;
	uint8 ap_cap;			/* The associated AP's capability (BW, MIMO/SISO). */
	uint8 association_status;	/* How we are associated to the AP (MIMO/SISO). */
	uint8 mimo_ps_state;		/* mimo_ps_cfg states: [0-5]. See below for values */
	uint8 mrc_state;		/* MRC state: NONE (0), ACTIVE(1) */
	uint8 bss_rxchain;		/* bss rxchain bitmask */
	uint8 bss_txchain;		/* bss txchain bitmask */
	uint8 bss_bw;			/* bandwidth: Full (0), 20M (1), 40M (2), 80M (3), etc */
	uint16 hw_state;		/* bitmask of hw state. See below for values */
	uint8 hw_rxchain;		/* actual HW rxchain bitmask */
	uint8 hw_txchain;		/* actual HW txchain bitmask */
	uint8 hw_bw;			/* bandwidth: Full (0), 20M (1), 40M (2), 80M (3), etc */
	uint8 pad[3];
} wl_mimo_ps_status_v1_t;

#define WL_MIMO_PS_STATUS_AP_CAP(ap_cap)	(ap_cap & 0x0F)
#define WL_MIMO_PS_STATUS_AP_CAP_BW(ap_cap)	(ap_cap >> 4)
#define WL_MIMO_PS_STATUS_ASSOC_BW_SHIFT 4

/* version 3: assoc status: low nibble is status enum, high other flags */
#define WL_MIMO_PS_STATUS_VERSION_3			3
#define WL_MIMO_PS_STATUS_ASSOC_STATUS_MASK		0x0F
#define WL_MIMO_PS_STATUS_ASSOC_STATUS_VHT_WITHOUT_OMN	0x80

/* mimo_ps_status: ap_cap/association status */
enum {
	WL_MIMO_PS_STATUS_ASSOC_NONE	= 0,
	WL_MIMO_PS_STATUS_ASSOC_SISO	= 1,
	WL_MIMO_PS_STATUS_ASSOC_MIMO	= 2,
	WL_MIMO_PS_STATUS_ASSOC_LEGACY	= 3
};

/* mimo_ps_status: mimo_ps_cfg states */
enum {
	WL_MIMO_PS_CFG_STATE_NONE			= 0,
	WL_MIMO_PS_CFG_STATE_INFORM_AP_INPROGRESS	= 1,
	WL_MIMO_PS_CFG_STATE_INFORM_AP_DONE		= 2,
	WL_MIMO_PS_CFG_STATE_LEARNING			= 3,
	WL_MIMO_PS_CFG_STATE_HW_CONFIGURE		= 4,
	WL_MIMO_PS_CFG_STATE_INFORM_AP_PENDING		= 5
};

/* mimo_ps_status: hw_state values */
#define WL_MIMO_PS_STATUS_HW_STATE_NONE			0
#define WL_MIMO_PS_STATUS_HW_STATE_LTECOEX		(0x1 << 0)
#define WL_MIMO_PS_STATUS_HW_STATE_MIMOPS_BSS		(0x1 << 1)
#define WL_MIMO_PS_STATUS_HW_STATE_AWDL_BSS		(0x1 << 2)
#define WL_MIMO_PS_STATUS_HW_STATE_SCAN			(0x1 << 3)
#define WL_MIMO_PS_STATUS_HW_STATE_TXPPR		(0x1 << 4)
#define WL_MIMO_PS_STATUS_HW_STATE_PWRTHOTTLE		(0x1 << 5)
#define WL_MIMO_PS_STATUS_HW_STATE_TMPSENSE		(0x1 << 6)
#define WL_MIMO_PS_STATUS_HW_STATE_IOVAR		(0x1 << 7)
#define WL_MIMO_PS_STATUS_HW_STATE_AP_BSS		(0x1 << 8)

/* mimo_ps_status: mrc states */
#define WL_MIMO_PS_STATUS_MRC_NONE	0
#define WL_MIMO_PS_STATUS_MRC_ACTIVE	1

/* mimo_ps_status: core flag states for single-core beacon and siso-bcmc rx */
#define WL_MIMO_PS_STATUS_MHF_FLAG_NONE		0
#define WL_MIMO_PS_STATUS_MHF_FLAG_ACTIVE	1
#define WL_MIMO_PS_STATUS_MHF_FLAG_COREDOWN	2
#define WL_MIMO_PS_STATUS_MHF_FLAG_INVALID	3

/* Type values for the REASON */
#define WL_MIMO_PS_PS_LEARNING_ABORTED          (1 << 0)
#define WL_MIMO_PS_PS_LEARNING_COMPLETED        (1 << 1)
#define WL_MIMO_PS_PS_LEARNING_ONGOING          (1 << 2)

typedef struct wl_mimo_ps_learning_event_data {
	uint32 startTimeStamp;
	uint32 endTimeStamp;
	uint16 reason;
	struct ether_addr BSSID;
	uint32 totalSISO_below_rssi_threshold;
	uint32 totalMIMO_below_rssi_threshold;
	uint32 totalSISO_above_rssi_threshold;
	uint32 totalMIMO_above_rssi_threshold;
} wl_mimo_ps_learning_event_data_t;

#define WL_MIMO_PS_PS_LEARNING_CFG_ABORT	(1 << 0)
#define WL_MIMO_PS_PS_LEARNING_CFG_STATUS	(1 << 1)
#define WL_MIMO_PS_PS_LEARNING_CFG_CONFIG	(1 << 2)

#define WL_MIMO_PS_PS_LEARNING_CFG_V1 1

typedef struct wl_mimops_learning_cfg {
	/* flag:  bit 0 for abort */
	/* flag:  bit 1 for status */
	/* flag:  bit 2 for configuring no of packets and rssi */
	uint8                  flag;
	/* mimo ps learning version, compatible version is 0 */
	uint8                  version;
	/* if version is 0 or rssi is 0, ignored */
	int8                   learning_rssi_threshold;
	uint8                  reserved;
	uint32                 no_of_packets_for_learning;
	wl_mimo_ps_learning_event_data_t mimops_learning_data;
} wl_mimops_learning_cfg_t;

#define WL_OCL_STATUS_VERSION 1
typedef struct ocl_status_info {
	uint8  version;
	uint8  len;
	uint16 fw_status;     /* Bits representing FW disable reasons */
	uint8  hw_status;     /* Bits for actual HW config and SISO/MIMO coremask */
	uint8  coremask;      /* The ocl core mask (indicating listening core) */
} ocl_status_info_t;

/* MWS OCL map */
#define WL_MWS_OCL_OVERRIDE_VERSION 1
typedef struct wl_mws_ocl_override {
	uint16  version;    /* Structure version */
	uint16	bitmap_2g; /* bitmap for 2.4G channels bits 1-13 */
	uint16	bitmap_5g_lo;  /* bitmap for 5G low channels by 2:
				*34-48, 52-56, 60-64, 100-102
				*/
	uint16	bitmap_5g_mid; /* bitmap for 5G mid channels by 2:
				* 104, 108-112, 116-120, 124-128,
				* 132-136, 140, 149-151
				*/
	uint16	bitmap_5g_high; /* bitmap for 5G high channels by 2
				* 153, 157-161, 165
				*/
} wl_mws_ocl_override_t;

/* Bits for fw_status */
#define OCL_DISABLED_HOST		0x01   /* Host has disabled through ocl_enable */
#define OCL_DISABLED_RSSI		0x02   /* Disabled because of ocl_rssi_threshold */
#define OCL_DISABLED_LTEC		0x04   /* Disabled due to LTE Coex activity */
#define OCL_DISABLED_SISO		0x08   /* Disabled while in SISO mode */
#define OCL_DISABLED_CAL		0x10   /* Disabled during active calibration */
#define OCL_DISABLED_CHANSWITCH		0x20   /* Disabled during active channel switch */
#define OCL_DISABLED_ASPEND     0x40   /* Disabled due to assoc pending */

/* Bits for hw_status */
#define OCL_HWCFG			0x01   /* State of OCL config bit in phy HW */
#define OCL_HWMIMO			0x02   /* Set if current coremask is > 1 bit */
#define OCL_COREDOWN			0x80   /* Set if core is currently down */

#define WL_OPS_CFG_VERSION_1  1
/* Common IOVAR struct */
typedef struct wl_ops_cfg_v1 {
	uint16  version;
	uint16 len;		/* total length includes fixed fields and variable data[] */
	uint16 subcmd_id;	/* subcommand id */
	uint16 padding;		/* reserved / padding for 4 byte align */
	uint8 data[];		/* subcommand data; could be empty */
} wl_ops_cfg_v1_t;

/* subcommands ids */
enum {
	WL_OPS_CFG_SUBCMD_ENABLE =		0,	/* OPS enable/disable mybss and obss
				                         * for nav and plcp options
				                         */
	WL_OPS_CFG_SUBCMD_MAX_SLEEP_DUR =	1,	/* Max sleep duration used for OPS */
	WL_OPS_CFG_SUBCMD_RESET_STATS =		2	/* Reset stats part of ops_status
			                                 * on both slices
				                         */
};

#define WL_OPS_CFG_MASK				0xffff
#define WL_OPS_CFG_CAP_MASK			0xffff0000
#define WL_OPS_CFG_CAP_SHIFT			16	/* Shift bits to locate the OPS CAP */
#define WL_OPS_MAX_SLEEP_DUR			12500	/* max ops duration in us */
#define WL_OPS_MINOF_MAX_SLEEP_DUR		512	/* minof max ops duration in us */
#define WL_OPS_SUPPORTED_CFG			(WL_OPS_MYBSS_PLCP_DUR | WL_OPS_MYBSS_NAV_DUR \
		                                 | WL_OPS_OBSS_PLCP_DUR | WL_OPS_OBSS_NAV_DUR)
#define WL_OPS_DEFAULT_CFG		        WL_OPS_SUPPORTED_CFG

/* WL_OPS_CFG_SUBCMD_ENABLE */
typedef struct wl_ops_cfg_enable {
	uint32   bits;   /* selectively enable ops for mybss and obss */
} wl_ops_cfg_enable_t;
/* Bits for WL_OPS_CFG_SUBCMD_ENABLE Parameter */
#define WL_OPS_MYBSS_PLCP_DUR		0x1	/* OPS based on mybss 11b & 11n mixed HT frames
		                                 * PLCP header duration
		                                 */
#define WL_OPS_MYBSS_NAV_DUR		0x2	/* OPS based on mybss RTS-CTS duration */
#define WL_OPS_OBSS_PLCP_DUR		0x4	/* OPS based on obss 11b & 11n mixed HT frames
		                                 * PLCP header duration
		                                 */
#define WL_OPS_OBSS_NAV_DUR		0x8	/* OPS based on obss RTS-CTS duration */

/* WL_OPS_CFG_SUBCMD_MAX_SLEEP_DUR */
typedef struct wl_ops_cfg_max_sleep_dur {
	uint32  val;   /* maximum sleep duration (us) used for OPS */
} wl_ops_cfg_max_sleep_dur_t;

/* WL_OPS_CFG_SUBCMD_RESET_STATS */
typedef struct wl_ops_cfg_reset_stats {
	uint32  val;   /* bitmap of slices, 0 means all slices */
} wl_ops_cfg_reset_stats_t;

#define WL_OPS_STATUS_VERSION_1 1
#define OPS_DUR_HIST_BINS	5	/* number of bins used, 0-1, 1-2, 2-4, 4-8, >8 msec */
typedef struct wl_ops_status_v1 {
	uint16  version;
	uint16  len;			/* Total length including all fixed fields */
	uint8	slice_index;		/* Slice for which status is reported */
	uint8   disable_obss;		/* indicate if obss cfg is disabled */
	uint8   pad[2];			/* 4-byte alignment */
	uint32  disable_reasons;	/* FW disable reasons */
	uint32  disable_duration;	/* ops disable time(ms) due to disable reasons */
	uint32  applied_ops_config;	/* currently applied ops config */
	uint32  partial_ops_dur;	/* Total time (in usec) of partial ops duration */
	uint32  full_ops_dur;		/* Total time (in usec) of full ops duration */
	uint32  count_dur_hist[OPS_DUR_HIST_BINS];	/* ops occurrence histogram */
	uint32  nav_cnt;		/* number of times ops triggered based NAV duration */
	uint32  plcp_cnt;		/* number of times ops triggered based PLCP duration */
	uint32  mybss_cnt;		/* number of times mybss ops trigger */
	uint32  obss_cnt;		/* number of times obss ops trigger */
	uint32  miss_dur_cnt;		/* number of times ops couldn't happen
		                         * due to insufficient duration
		                         */
	uint32  miss_premt_cnt;		/* number of times ops couldn't happen due
		                         * to not meeting Phy preemption thresh
		                         */
	uint32  max_dur_cnt;		/* number of times ops did not trigger due to
		                         * frames exceeding max sleep duration
		                         */
	uint32	wake_cnt;		/* number of ops miss due to wake reason */
	uint32	bcn_wait_cnt;		/* number of ops miss due to waiting for bcn */
} wl_ops_status_v1_t;
/* Bits for disable_reasons */
#define OPS_DISABLED_HOST	0x01	/* Host has disabled through ops_cfg */
#define OPS_DISABLED_UNASSOC	0x02	/* Disabled because the slice is in unassociated state */
#define OPS_DISABLED_SCAN	0x04	/* Disabled because the slice is in scan state */
#define OPS_DISABLED_BCN_MISS	0x08	/* Disabled because beacon missed for a duration */

/*
 * Join preference iovar value is an array of tuples. Each tuple has a one-byte type,
 * a one-byte length, and a variable length value.  RSSI type tuple must be present
 * in the array.
 *
 * Types are defined in "join preference types" section.
 *
 * Length is the value size in octets. It is reserved for WL_JOIN_PREF_WPA type tuple
 * and must be set to zero.
 *
 * Values are defined below.
 *
 * 1. RSSI - 2 octets
 * offset 0: reserved
 * offset 1: reserved
 *
 * 2. WPA - 2 + 12 * n octets (n is # tuples defined below)
 * offset 0: reserved
 * offset 1: # of tuples
 * offset 2: tuple 1
 * offset 14: tuple 2
 * ...
 * offset 2 + 12 * (n - 1) octets: tuple n
 *
 * struct wpa_cfg_tuple {
 *   uint8 akm[DOT11_OUI_LEN+1];     akm suite
 *   uint8 ucipher[DOT11_OUI_LEN+1]; unicast cipher suite
 *   uint8 mcipher[DOT11_OUI_LEN+1]; multicast cipher suite
 * };
 *
 * multicast cipher suite can be specified as a specific cipher suite or WL_WPA_ACP_MCS_ANY.
 *
 * 3. BAND - 2 octets
 * offset 0: reserved
 * offset 1: see "band preference" and "band types"
 *
 * 4. BAND RSSI - 2 octets
 * offset 0: band types
 * offset 1: +ve RSSI boost value in dB
 */

struct tsinfo_arg {
	uint8 octets[3];
};

#define RATE_CCK_1MBPS 0
#define RATE_CCK_2MBPS 1
#define RATE_CCK_5_5MBPS 2
#define RATE_CCK_11MBPS 3

#define RATE_LEGACY_OFDM_6MBPS 0
#define RATE_LEGACY_OFDM_9MBPS 1
#define RATE_LEGACY_OFDM_12MBPS 2
#define RATE_LEGACY_OFDM_18MBPS 3
#define RATE_LEGACY_OFDM_24MBPS 4
#define RATE_LEGACY_OFDM_36MBPS 5
#define RATE_LEGACY_OFDM_48MBPS 6
#define RATE_LEGACY_OFDM_54MBPS 7

#define WL_BSSTRANS_RSSI_RATE_MAP_VERSION 1

typedef struct wl_bsstrans_rssi {
	int8 rssi_2g;	/**< RSSI in dbm for 2.4 G */
	int8 rssi_5g;	/**< RSSI in dbm for 5G, unused for cck */
} wl_bsstrans_rssi_t;

#define RSSI_RATE_MAP_MAX_STREAMS 4	/**< max streams supported */

/** RSSI to rate mapping, all 20Mhz, no SGI */
typedef struct wl_bsstrans_rssi_rate_map {
	uint16 ver;
	uint16 len; /**< length of entire structure */
	wl_bsstrans_rssi_t cck[WL_NUM_RATES_CCK]; /**< 2.4G only */
	wl_bsstrans_rssi_t ofdm[WL_NUM_RATES_OFDM]; /**< 6 to 54mbps */
	wl_bsstrans_rssi_t phy_n[RSSI_RATE_MAP_MAX_STREAMS][WL_NUM_RATES_MCS_1STREAM]; /* MCS0-7 */
	wl_bsstrans_rssi_t phy_ac[RSSI_RATE_MAP_MAX_STREAMS][WL_NUM_RATES_VHT]; /**< MCS0-9 */
} wl_bsstrans_rssi_rate_map_t;

#define WL_BSSTRANS_ROAMTHROTTLE_VERSION 1

/** Configure number of scans allowed per throttle period */
typedef struct wl_bsstrans_roamthrottle {
	uint16 ver;
	uint16 period;
	uint16 scans_allowed;
} wl_bsstrans_roamthrottle_t;

#define	NFIFO			6	/**< # tx/rx fifopairs */

#if defined(BCM_DMA_CT) && !defined(BCM_DMA_CT_DISABLED)
#define NFIFO_EXT		32		/* 6 traditional FIFOs + 2 rsvd + 24 MU FIFOs */
#elif defined(WL11AX) && defined(WL11AX_TRIGGERQ_ENABLED)
#define NFIFO_EXT		10
#else
#define NFIFO_EXT		NFIFO
#endif // endif

/* Reinit reason codes */
enum {
	WL_REINIT_RC_NONE             = 0,
	WL_REINIT_RC_PS_SYNC          = 1,
	WL_REINIT_RC_PSM_WD           = 2,
	WL_REINIT_RC_MAC_WAKE         = 3,
	WL_REINIT_RC_MAC_SUSPEND      = 4,
	WL_REINIT_RC_MAC_SPIN_WAIT    = 5,
	WL_REINIT_RC_AXI_BUS_ERROR    = 6,
	WL_REINIT_RC_DEVICE_REMOVED   = 7,
	WL_REINIT_RC_PCIE_FATAL_ERROR = 8,
	WL_REINIT_RC_OL_FW_TRAP       = 9,
	WL_REINIT_RC_FIFO_ERR         = 10,
	WL_REINIT_RC_INV_TX_STATUS    = 11,
	WL_REINIT_RC_MQ_ERROR         = 12,
	WL_REINIT_RC_PHYTXERR_THRESH  = 13,
	WL_REINIT_RC_USER_FORCED      = 14,
	WL_REINIT_RC_FULL_RESET       = 15,
	WL_REINIT_RC_AP_BEACON        = 16,
	WL_REINIT_RC_PM_EXCESSED      = 17,
	WL_REINIT_RC_NO_CLK           = 18,
	WL_REINIT_RC_SW_ASSERT        = 19,
	WL_REINIT_RC_PSM_JMP0         = 20,
	WL_REINIT_RC_PSM_RUN          = 21,
	WL_REINIT_RC_ENABLE_MAC       = 22,
	WL_REINIT_RC_SCAN_TIMEOUT     = 23,
	WL_REINIT_RC_JOIN_TIMEOUT     = 24,
	/* Below error codes are generated during D3 exit validation */
	WL_REINIT_RC_LINK_NOT_ACTIVE  = 25,
	WL_REINIT_RC_PCI_CFG_RD_FAIL  = 26,
	WL_REINIT_RC_INV_VEN_ID       = 27,
	WL_REINIT_RC_INV_DEV_ID       = 28,
	WL_REINIT_RC_INV_BAR0         = 29,
	WL_REINIT_RC_INV_BAR2         = 30,
	WL_REINIT_RC_AER_UC_FATAL     = 31,
	WL_REINIT_RC_AER_UC_NON_FATAL = 32,
	WL_REINIT_RC_AER_CORR         = 33,
	WL_REINIT_RC_AER_DEV_STS      = 34,
	WL_REINIT_RC_PCIe_STS         = 35,
	WL_REINIT_RC_MMIO_RD_FAIL     = 36,
	WL_REINIT_RC_MMIO_RD_INVAL    = 37,
	WL_REINIT_RC_MMIO_ARM_MEM_RD_FAIL = 38,
	WL_REINIT_RC_MMIO_ARM_MEM_INVAL   = 39,
	WL_REINIT_RC_SROM_LOAD_FAILED     = 40,
	WL_REINIT_RC_PHY_CRASH            = 41,
	WL_REINIT_TX_STALL                = 42,
	WL_REINIT_RC_TX_FLOW_CONTROL_BLOCKED	= 43,
	WL_REINIT_RC_RX_HC_FAIL           = 44,
	WL_REINIT_RC_RX_DMA_STALL         = 45,
	WL_REINIT_UTRACE_BUF_OVERLAP_SR	  = 46,
	WL_REINIT_UTRACE_TPL_OUT_BOUNDS   = 47,
	WL_REINIT_UTRACE_TPL_OSET_STRT0   = 48,
	WL_REINIT_RC_PHYTXERR             = 49,
	WL_REINIT_RC_PSM_FATAL_SUSP       = 50,
	WL_REINIT_RC_TX_FIFO_SUSP         = 51,
	WL_REINIT_RC_MAC_ENABLE           = 52,
	WL_REINIT_RC_SCAN_STALLED         = 53,
	WL_REINIT_RC_RX_DMA_BUF_EMPTY     = 54,
	WL_REINIT_RC_LAST	/* This must be the last entry */
};

#define NREINITREASONCOUNT	8
/* XXX NREINITREASONCOUNT is 8 in other branches.
 * Any change to this will break wl tool compatibility with other branches
 * #define NREINITREASONCOUNT	WL_REINIT_RC_LAST
 */

#define REINITRSNIDX(_x)	(((_x) < WL_REINIT_RC_LAST) ? (_x) : 0)

#define	WL_CNT_T_VERSION	30	/**< current version of wl_cnt_t struct */
#define WL_CNT_VERSION_6	6
#define WL_CNT_VERSION_11	11
#define WL_CNT_VERSION_XTLV	30

#define WL_COUNTERS_IOV_VERSION_1	1
#define WL_SUBCNTR_IOV_VER		WL_COUNTERS_IOV_VERSION_1
/* First two uint16 are version and lenght fields. So offset of the first counter will be 4 */
#define FIRST_COUNTER_OFFSET		0x04

#define WLC_WITH_XTLV_CNT

/**
 * tlv IDs uniquely identifies counter component
 * packed into wl_cmd_t container
 */
enum wl_cnt_xtlv_id {
	WL_CNT_XTLV_SLICE_IDX = 0x1,		/**< Slice index */
	WL_CNT_XTLV_WLC = 0x100,		/**< WLC layer counters */
	WL_CNT_XTLV_WLC_RINIT_RSN = 0x101,	/**< WLC layer reinitreason extension */
	WL_CNT_XTLV_CNTV_LE10_UCODE = 0x200,	/**< wl counter ver < 11 UCODE MACSTAT */
	WL_CNT_XTLV_LT40_UCODE_V1 = 0x300,	/**< corerev < 40 UCODE MACSTAT */
	WL_CNT_XTLV_GE40_UCODE_V1 = 0x400,	/**< corerev >= 40 UCODE MACSTAT */
	WL_CNT_XTLV_GE64_UCODEX_V1 = 0x800,	/* corerev >= 64 UCODEX MACSTAT */
	WL_CNT_XTLV_GE80_UCODE_V1 = 0x900,	/* corerev >= 80 UCODEX MACSTAT */
	WL_CNT_XTLV_GE80_TXFUNFL_UCODE_V1 = 0x1000,	/* corerev >= 80 UCODEX MACSTAT */
	WL_CNT_XTLV_EAP = 0x2000	/* EAP added counters */
};

/* structure holding tdm counters that interface to iovar */
typedef struct {
	uint16 ver;
	uint16 length; /* length of this structure */
	uint32 tdmtx_flg;
	uint32 tdmtx_rd;
	uint32 tdmtx_wr;
	uint32 tdmtx_txa_on; /* TXA on requests */
	uint32 tdmtx_txa_tmcnt; /* Total number of TXA timeout */
	uint32 tdmtx_por_on; /* TXA POR requests */
	uint32 tdmtx_txpuen; /* Path enable requests */
	uint32 tdmtx_txpudis; /* Total number of times Tx path is muted on the slice */
	uint32 tdmtx_txpri_on; /* Total number of times Tx priority was obtained by the slice */
	uint32 tdmtx_txdefer; /* Total number of times Tx priority was obtained by the slice */
	uint32 tdmtx_txa_dur; /* Total time txa on */
	uint32 tdmtx_txpri_dur; /* Total time TXPri */
	uint32 tdmtx_txdefer_dur; /* Total time txdefer */
	uint32 tdmtx_txa_max;
	uint32 tdmtx_defer_th;
	uint32 tdmtx_txpri_per;
	/* TDMTX input fields */
	uint32 tdmtx_txpri;
	uint32 tdmtx_defer;
	uint32 tdmtx_threshold;
	uint32 tdmtx_rssi_threshold;
	uint32 tdmtx_txpwrboff;
	uint32 tdmtx_txpwrboff_dt;
} tdmtx_cnt_v1_t;

/* structure holding counters that match exactly shm field sizes */
typedef struct {
	uint16 tdmtx_rd;
	uint16 tdmtx_wr;
	uint16 tdmtx_txa_on; /* TXA on requests */
	uint16 tdmtx_tmcnt; /* TXA on requests */
	uint16 tdmtx_por_on; /* TXA POR requests */
	uint16 tdmtx_txpuen; /* Path enable requests */
	uint16 tdmtx_txpudis; /* Total number of times Tx path is muted on the slice */
	uint16 tdmtx_txpri_on; /* Total number of times Tx priority was obtained by the slice */
	uint16 tdmtx_txdefer; /* Total number of times Tx priority was obtained by the slice */
	uint16 tdmtx_txa_dur_l; /* Total time (low 16 bits) txa on */
	uint16 tdmtx_txa_dur_h; /* Total time (low 16 bits) txa on */
	uint16 tdmtx_txpri_dur_l; /* Total time (low 16 bits) TXPri */
	uint16 tdmtx_txpri_dur_h; /* Total time (high 16 bits) TXPri */
	uint16 tdmtx_txdefer_dur_l; /* Total time (low 16 bits)  txdefer */
	uint16 tdmtx_txdefer_dur_h; /* Total time (high 16 bits) txdefer */
} tdmtx_cnt_shm_t;

typedef struct wl_tdmtx_ioc {
	uint16 id;	/* ID of the sub-command */
	uint16 len;	/* total length of all data[] */
	uint8  data[];	/* var len payload */
} wl_tdmtx_ioc_t;

/*
 * iovar subcommand ids
 */
enum {
	IOV_TDMTX_ENB = 1,
	IOV_TDMTX_STATUS = 2,
	IOV_TDMTX_TXPRI = 3,
	IOV_TDMTX_DEFER = 4,
	IOV_TDMTX_TXA = 5,
	IOV_TDMTX_CFG = 6,
	IOV_TDMTX_LAST
};

/**
 * The number of variables in wl macstat cnt struct.
 * (wl_cnt_ge40mcst_v1_t, wl_cnt_lt40mcst_v1_t, wl_cnt_v_le10_mcst_t)
 */
#define WL_CNT_MCST_VAR_NUM 64
/* sizeof(wl_cnt_ge40mcst_v1_t), sizeof(wl_cnt_lt40mcst_v1_t), and sizeof(wl_cnt_v_le10_mcst_t) */
#define WL_CNT_MCST_STRUCT_SZ ((uint32)sizeof(uint32) * WL_CNT_MCST_VAR_NUM)
#define WL_CNT_REV80_MCST_STRUCT_SZ ((uint32)sizeof(wl_cnt_ge80mcst_v1_t))
#define WL_CNT_REV80_MCST_TXFUNFlW_STRUCT_SZ ((uint32)sizeof(wl_cnt_ge80_txfunfl_v1_t))

#define WL_CNT_TDMTX_STRUCT_SZ (sizeof(tdmtx_cnt_v1_t))
#define WL_CNT_TDMTX_SHM_SZ (sizeof(tdmtx_cnt_shm_t))

#define WL_CNT_MCXST_STRUCT_SZ ((uint32)sizeof(wl_cnt_ge64mcxst_v1_t))

#define INVALID_CNT_VAL (uint32)(-1)

#define WL_XTLV_CNTBUF_MAX_SIZE ((uint32)(OFFSETOF(wl_cnt_info_t, data)) +	\
		(uint32)BCM_XTLV_HDR_SIZE + (uint32)sizeof(wl_cnt_wlc_t) +		\
		(uint32)BCM_XTLV_HDR_SIZE + WL_CNT_MCST_STRUCT_SZ +              \
		(uint32)BCM_XTLV_HDR_SIZE + WL_CNT_MCXST_STRUCT_SZ)

#define WL_CNTBUF_MAX_SIZE MAX(WL_XTLV_CNTBUF_MAX_SIZE, (uint32)sizeof(wl_cnt_ver_11_t))

/* XXX: Please refer to the twiki for counters addition/deletion.
 * http://hwnbu-twiki.sj.broadcom.com/bin/view/Mwgroup/WlCounters#Counter_Edition
 */

/** Top structure of counters IOVar buffer */
typedef struct {
	uint16	version;	/**< see definition of WL_CNT_T_VERSION */
	uint16	datalen;	/**< length of data including all paddings. */
	uint8   data [];	/**< variable length payload:
				 * 1 or more bcm_xtlv_t type of tuples.
				 * each tuple is padded to multiple of 4 bytes.
				 * 'datalen' field of this structure includes all paddings.
				 */
} wl_cnt_info_t;

/* Top structure of subcounters IOVar buffer
 * Whenever we make any change in this structure
 * WL_SUBCNTR_IOV_VER should be updated accordingly
 * The structure definition should remain consistant b/w
 * FW and wl/WLM app.
 */
typedef struct {
	uint16	version;	  /* Version of IOVAR structure. Used for backward
				   * compatibility in future. Whenever we make any
				   * changes to this structure then value of WL_SUBCNTR_IOV_VER
				   * needs to be updated properly.
				   */
	uint16	length;		  /* length in bytes of this structure */
	uint16	counters_version; /* see definition of WL_CNT_T_VERSION
				   * wl app will send the version of counters
				   * which is used to calculate the offset of counters.
				   * It must match the version of counters FW is using
				   * else FW will return error with his version of counters
				   * set in this field.
				   */
	uint16	num_subcounters;  /* Number of counter offset passed by wl app to FW. */
	uint32	data[1];	  /* variable length payload:
				   * Offsets to the counters will be passed to FW
				   * throught this data field. FW will return the value of counters
				   * at the offsets passed by wl app in this fiels itself.
				   */
} wl_subcnt_info_t;

/** wlc layer counters */
typedef struct {
	/* transmit stat counters */
	uint32	txframe;	/**< tx data frames */
	uint32	txbyte;		/**< tx data bytes */
	uint32	txretrans;	/**< tx mac retransmits */
	uint32	txerror;	/**< tx data errors (derived: sum of others) */
	uint32	txctl;		/**< tx management frames */
	uint32	txprshort;	/**< tx short preamble frames */
	uint32	txserr;		/**< tx status errors */
	uint32	txnobuf;	/**< tx out of buffers errors */
	uint32	txnoassoc;	/**< tx discard because we're not associated */
	uint32	txrunt;		/**< tx runt frames */
	uint32	txchit;		/**< tx header cache hit (fastpath) */
	uint32	txcmiss;	/**< tx header cache miss (slowpath) */

	/* transmit chip error counters */
	uint32	txuflo;		/**< tx fifo underflows */
	uint32	txphyerr;	/**< tx phy errors (indicated in tx status) */
	uint32	txphycrs;

	/* receive stat counters */
	uint32	rxframe;	/**< rx data frames */
	uint32	rxbyte;		/**< rx data bytes */
	uint32	rxerror;	/**< rx data errors (derived: sum of others) */
	uint32	rxctl;		/**< rx management frames */
	uint32	rxnobuf;	/**< rx out of buffers errors */
	uint32	rxnondata;	/**< rx non data frames in the data channel errors */
	uint32	rxbadds;	/**< rx bad DS errors */
	uint32	rxbadcm;	/**< rx bad control or management frames */
	uint32	rxfragerr;	/**< rx fragmentation errors */
	uint32	rxrunt;		/**< rx runt frames */
	uint32	rxgiant;	/**< rx giant frames */
	uint32	rxnoscb;	/**< rx no scb error */
	uint32	rxbadproto;	/**< rx invalid frames */
	uint32	rxbadsrcmac;	/**< rx frames with Invalid Src Mac */
	uint32	rxbadda;	/**< rx frames tossed for invalid da */
	uint32	rxfilter;	/**< rx frames filtered out */

	/* receive chip error counters */
	uint32	rxoflo;		/**< rx fifo overflow errors */
	uint32	rxuflo[NFIFO];	/**< rx dma descriptor underflow errors */

	uint32	d11cnt_txrts_off;	/**< d11cnt txrts value when reset d11cnt */
	uint32	d11cnt_rxcrc_off;	/**< d11cnt rxcrc value when reset d11cnt */
	uint32	d11cnt_txnocts_off;	/**< d11cnt txnocts value when reset d11cnt */

	/* misc counters */
	uint32	dmade;		/**< tx/rx dma descriptor errors */
	uint32	dmada;		/**< tx/rx dma data errors */
	uint32	dmape;		/**< tx/rx dma descriptor protocol errors */
	uint32	reset;		/**< reset count */
	uint32	tbtt;		/**< cnts the TBTT int's */
	uint32	txdmawar;
	uint32	pkt_callback_reg_fail;	/**< callbacks register failure */

	/* 802.11 MIB counters, pp. 614 of 802.11 reaff doc. */
	uint32	txfrag;		/**< dot11TransmittedFragmentCount */
	uint32	txmulti;	/**< dot11MulticastTransmittedFrameCount */
	uint32	txfail;		/**< dot11FailedCount */
	uint32	txretry;	/**< dot11RetryCount */
	uint32	txretrie;	/**< dot11MultipleRetryCount */
	uint32	rxdup;		/**< dot11FrameduplicateCount */
	uint32	txrts;		/**< dot11RTSSuccessCount */
	uint32	txnocts;	/**< dot11RTSFailureCount */
	uint32	txnoack;	/**< dot11ACKFailureCount */
	uint32	rxfrag;		/**< dot11ReceivedFragmentCount */
	uint32	rxmulti;	/**< dot11MulticastReceivedFrameCount */
	uint32	rxcrc;		/**< dot11FCSErrorCount */
	uint32	txfrmsnt;	/**< dot11TransmittedFrameCount (bogus MIB?) */
	uint32	rxundec;	/**< dot11WEPUndecryptableCount */

	/* WPA2 counters (see rxundec for DecryptFailureCount) */
	uint32	tkipmicfaill;	/**< TKIPLocalMICFailures */
	uint32	tkipcntrmsr;	/**< TKIPCounterMeasuresInvoked */
	uint32	tkipreplay;	/**< TKIPReplays */
	uint32	ccmpfmterr;	/**< CCMPFormatErrors */
	uint32	ccmpreplay;	/**< CCMPReplays */
	uint32	ccmpundec;	/**< CCMPDecryptErrors */
	uint32	fourwayfail;	/**< FourWayHandshakeFailures */
	uint32	wepundec;	/**< dot11WEPUndecryptableCount */
	uint32	wepicverr;	/**< dot11WEPICVErrorCount */
	uint32	decsuccess;	/**< DecryptSuccessCount */
	uint32	tkipicverr;	/**< TKIPICVErrorCount */
	uint32	wepexcluded;	/**< dot11WEPExcludedCount */

	uint32	txchanrej;	/**< Tx frames suppressed due to channel rejection */
	uint32	psmwds;		/**< Count PSM watchdogs */
	uint32	phywatchdog;	/**< Count Phy watchdogs (triggered by ucode) */

	/* MBSS counters, AP only */
	uint32	prq_entries_handled;	/**< PRQ entries read in */
	uint32	prq_undirected_entries;	/**<    which were bcast bss & ssid */
	uint32	prq_bad_entries;	/**<    which could not be translated to info */
	uint32	atim_suppress_count;	/**< TX suppressions on ATIM fifo */
	uint32	bcn_template_not_ready;	/**< Template marked in use on send bcn ... */
	uint32	bcn_template_not_ready_done; /**< ...but "DMA done" interrupt rcvd */
	uint32	late_tbtt_dpc;	/**< TBTT DPC did not happen in time */

	/* per-rate receive stat counters */
	uint32  rx1mbps;	/**< packets rx at 1Mbps */
	uint32  rx2mbps;	/**< packets rx at 2Mbps */
	uint32  rx5mbps5;	/**< packets rx at 5.5Mbps */
	uint32  rx6mbps;	/**< packets rx at 6Mbps */
	uint32  rx9mbps;	/**< packets rx at 9Mbps */
	uint32  rx11mbps;	/**< packets rx at 11Mbps */
	uint32  rx12mbps;	/**< packets rx at 12Mbps */
	uint32  rx18mbps;	/**< packets rx at 18Mbps */
	uint32  rx24mbps;	/**< packets rx at 24Mbps */
	uint32  rx36mbps;	/**< packets rx at 36Mbps */
	uint32  rx48mbps;	/**< packets rx at 48Mbps */
	uint32  rx54mbps;	/**< packets rx at 54Mbps */
	uint32  rx108mbps;	/**< packets rx at 108mbps */
	uint32  rx162mbps;	/**< packets rx at 162mbps */
	uint32  rx216mbps;	/**< packets rx at 216 mbps */
	uint32  rx270mbps;	/**< packets rx at 270 mbps */
	uint32  rx324mbps;	/**< packets rx at 324 mbps */
	uint32  rx378mbps;	/**< packets rx at 378 mbps */
	uint32  rx432mbps;	/**< packets rx at 432 mbps */
	uint32  rx486mbps;	/**< packets rx at 486 mbps */
	uint32  rx540mbps;	/**< packets rx at 540 mbps */

	uint32	rfdisable;	/**< count of radio disables */

	uint32	txexptime;	/**< Tx frames suppressed due to timer expiration */

	uint32	txmpdu_sgi;	/**< count for sgi transmit */
	uint32	rxmpdu_sgi;	/**< count for sgi received */
	uint32	txmpdu_stbc;	/**< count for stbc transmit */
	uint32	rxmpdu_stbc;	/**< count for stbc received */

	uint32	rxundec_mcst;	/**< dot11WEPUndecryptableCount */

	/* WPA2 counters (see rxundec for DecryptFailureCount) */
	uint32	tkipmicfaill_mcst;	/**< TKIPLocalMICFailures */
	uint32	tkipcntrmsr_mcst;	/**< TKIPCounterMeasuresInvoked */
	uint32	tkipreplay_mcst;	/**< TKIPReplays */
	uint32	ccmpfmterr_mcst;	/**< CCMPFormatErrors */
	uint32	ccmpreplay_mcst;	/**< CCMPReplays */
	uint32	ccmpundec_mcst;	/**< CCMPDecryptErrors */
	uint32	fourwayfail_mcst;	/**< FourWayHandshakeFailures */
	uint32	wepundec_mcst;	/**< dot11WEPUndecryptableCount */
	uint32	wepicverr_mcst;	/**< dot11WEPICVErrorCount */
	uint32	decsuccess_mcst;	/**< DecryptSuccessCount */
	uint32	tkipicverr_mcst;	/**< TKIPICVErrorCount */
	uint32	wepexcluded_mcst;	/**< dot11WEPExcludedCount */

	uint32	dma_hang;	/**< count for dma hang */
	uint32	reinit;		/**< count for reinit */

	uint32  pstatxucast;	/**< count of ucast frames xmitted on all psta assoc */
	uint32  pstatxnoassoc;	/**< count of txnoassoc frames xmitted on all psta assoc */
	uint32  pstarxucast;	/**< count of ucast frames received on all psta assoc */
	uint32  pstarxbcmc;	/**< count of bcmc frames received on all psta */
	uint32  pstatxbcmc;	/**< count of bcmc frames transmitted on all psta */

	uint32  cso_passthrough; /**< hw cso required but passthrough */
	uint32	cso_normal;	/**< hw cso hdr for normal process */
	uint32	chained;	/**< number of frames chained */
	uint32	chainedsz1;	/**< number of chain size 1 frames */
	uint32	unchained;	/**< number of frames not chained */
	uint32	maxchainsz;	/**< max chain size so far */
	uint32	currchainsz;	/**< current chain size */
	uint32	pciereset;	/**< Secondary Bus Reset issued by driver */
	uint32	cfgrestore;	/**< configspace restore by driver */
	uint32	reinitreason[NREINITREASONCOUNT]; /**< reinitreason counters; 0: Unknown reason */
	uint32	rxrtry;
	uint32  rxmpdu_mu;      /**< Number of MU MPDUs received */

	/* detailed control/management frames */
	uint32	txbar;		/**< Number of TX BAR */
	uint32	rxbar;		/**< Number of RX BAR */
	uint32	txpspoll;	/**< Number of TX PS-poll */
	uint32	rxpspoll;	/**< Number of RX PS-poll */
	uint32	txnull;		/**< Number of TX NULL_DATA */
	uint32	rxnull;		/**< Number of RX NULL_DATA */
	uint32	txqosnull;	/**< Number of TX NULL_QoSDATA */
	uint32	rxqosnull;	/**< Number of RX NULL_QoSDATA */
	uint32	txassocreq;	/**< Number of TX ASSOC request */
	uint32	rxassocreq;	/**< Number of RX ASSOC request */
	uint32	txreassocreq;	/**< Number of TX REASSOC request */
	uint32	rxreassocreq;	/**< Number of RX REASSOC request */
	uint32	txdisassoc;	/**< Number of TX DISASSOC */
	uint32	rxdisassoc;	/**< Number of RX DISASSOC */
	uint32	txassocrsp;	/**< Number of TX ASSOC response */
	uint32	rxassocrsp;	/**< Number of RX ASSOC response */
	uint32	txreassocrsp;	/**< Number of TX REASSOC response */
	uint32	rxreassocrsp;	/**< Number of RX REASSOC response */
	uint32	txauth;		/**< Number of TX AUTH */
	uint32	rxauth;		/**< Number of RX AUTH */
	uint32	txdeauth;	/**< Number of TX DEAUTH */
	uint32	rxdeauth;	/**< Number of RX DEAUTH */
	uint32	txprobereq;	/**< Number of TX probe request */
	uint32	rxprobereq;	/**< Number of RX probe request */
	uint32	txprobersp;	/**< Number of TX probe response */
	uint32	rxprobersp;	/**< Number of RX probe response */
	uint32	txaction;	/**< Number of TX action frame */
	uint32	rxaction;	/**< Number of RX action frame */
	uint32  ampdu_wds;	/**< Number of AMPDU watchdogs */
	uint32  txlost;		/**< Number of lost packets reported in txs */
	uint32	txdatamcast;	/**< Number of TX multicast data packets */
	uint32	txdatabcast;	/**< Number of TX broadcast data packets */
	uint32	psmxwds;	/**< Number of PSMx watchdogs */
	uint32  rxback;
	uint32  txback;
	uint32  p2p_tbtt;	/**< Number of P2P TBTT Events */
	uint32  p2p_tbtt_miss;	/**< Number of P2P TBTT Events Miss */
	uint32	txqueue_start;
	uint32	txqueue_end;
	uint32  txbcast;        /* Broadcast TransmittedFrameCount */
	uint32  txdropped;      /* tx dropped pkts */
	uint32  rxbcast;        /* BroadcastReceivedFrameCount */
	uint32  rxdropped;      /* rx dropped pkts (derived: sum of others) */
	/* XXX: Do not remove or rename inthe middle of this struct.
	 * All counter variables have to be of uint32.
	 * Please follow the instruction in
	 * http://hwnbu-twiki.sj.broadcom.com/bin/view/Mwgroup/WlCounters#Counter_Edition
	 */
} wl_cnt_wlc_t;

/* Reinit reasons - do not put anything else other than reinit reasons here */
typedef struct {
	uint32 rsn[WL_REINIT_RC_LAST];
} reinit_rsns_t;

/* MACXSTAT counters for ucodex (corerev >= 64) */
typedef struct {
	uint32 macxsusp;
	uint32 m2vmsg;
	uint32 v2mmsg;
	uint32 mboxout;
	uint32 musnd;
	uint32 sfb2v;
} wl_cnt_ge64mcxst_v1_t;

/** MACSTAT counters for ucode (corerev >= 40) */
typedef struct {
	/* MAC counters: 32-bit version of d11.h's macstat_t */
	uint32	txallfrm;	/**< total number of frames sent, incl. Data, ACK, RTS, CTS,
				 * Control Management (includes retransmissions)
				 */
	uint32	txrtsfrm;	/**< number of RTS sent out by the MAC */
	uint32	txctsfrm;	/**< number of CTS sent out by the MAC */
	uint32	txackfrm;	/**< number of ACK frames sent out */
	uint32	txdnlfrm;	/**< number of Null-Data transmission generated from template  */
	uint32	txbcnfrm;	/**< beacons transmitted */
	uint32	txfunfl[6];	/**< per-fifo tx underflows */
	uint32	txampdu;	/**< number of AMPDUs transmitted */
	uint32	txmpdu;		/**< number of MPDUs transmitted */
	uint32	txtplunfl;	/**< Template underflows (mac was too slow to transmit ACK/CTS
				 * or BCN)
				 */
	uint32	txphyerror;	/**< Transmit phy error, type of error is reported in tx-status for
				 * driver enqueued frames
				 */
	uint32  pktengrxducast; /**< unicast frames rxed by the pkteng code */
	uint32  pktengrxdmcast; /**< multicast frames rxed by the pkteng code */
	uint32	rxfrmtoolong;	/**< Received frame longer than legal limit (2346 bytes) */
	uint32	rxfrmtooshrt; /**< Received frame did not contain enough bytes for its frame type */
	uint32	rxanyerr;	/**< Any RX error that is not counted by other counters. */
	uint32	rxbadfcs;	/**< number of frames for which the CRC check failed in the MAC */
	uint32	rxbadplcp;	/**< parity check of the PLCP header failed */
	uint32	rxcrsglitch;	/**< PHY was able to correlate the preamble but not the header */
	uint32	rxstrt;		/**< Number of received frames with a good PLCP
				 * (i.e. passing parity check)
				 */
	uint32	rxdtucastmbss; /**< number of received DATA frames with good FCS and matching RA */
	uint32	rxmgucastmbss; /**< number of received mgmt frames with good FCS and matching RA */
	uint32	rxctlucast; /**< number of received CNTRL frames with good FCS and matching RA */
	uint32	rxrtsucast;	/**< number of unicast RTS addressed to the MAC (good FCS) */
	uint32	rxctsucast;	/**< number of unicast CTS addressed to the MAC (good FCS) */
	uint32	rxackucast;	/**< number of ucast ACKS received (good FCS) */
	uint32	rxdtocast; /**< number of received DATA frames (good FCS and not matching RA) */
	uint32	rxmgocast; /**< number of received MGMT frames (good FCS and not matching RA) */
	uint32	rxctlocast; /**< number of received CNTRL frame (good FCS and not matching RA) */
	uint32	rxrtsocast;	/**< number of received RTS not addressed to the MAC */
	uint32	rxctsocast;	/**< number of received CTS not addressed to the MAC */
	uint32	rxdtmcast;	/**< number of RX Data multicast frames received by the MAC */
	uint32	rxmgmcast;	/**< number of RX Management multicast frames received by the MAC */
	uint32	rxctlmcast;	/**< number of RX Control multicast frames received by the MAC
				 * (unlikely to see these)
				 */
	uint32	rxbeaconmbss;	/**< beacons received from member of BSS */
	uint32	rxdtucastobss; /**< number of unicast frames addressed to the MAC from
				  * other BSS (WDS FRAME)
				  */
	uint32	rxbeaconobss;	/**< beacons received from other BSS */
	uint32	rxrsptmout;	/**< number of response timeouts for transmitted frames
				 * expecting a response
				 */
	uint32	bcntxcancl;	/**< transmit beacons canceled due to receipt of beacon (IBSS) */
	uint32	rxnodelim;	/**< number of no valid delimiter detected by ampdu parser */
	uint32	rxf0ovfl;	/**< number of receive fifo 0 overflows */
	uint32	rxf1ovfl;	/**< number of receive fifo 1 overflows */
	uint32	rxhlovfl;	/**< number of length / header fifo overflows */
	uint32	missbcn_dbg;	/**< number of beacon missed to receive */
	uint32	pmqovfl;	/**< number of PMQ overflows */
	uint32	rxcgprqfrm;	/**< number of received Probe requests that made it into
				 * the PRQ fifo
				 */
	uint32	rxcgprsqovfl;	/**< Rx Probe Request Que overflow in the AP */
	uint32	txcgprsfail;	/**< Tx Probe Response Fail. AP sent probe response but did
				 * not get ACK
				 */
	uint32	txcgprssuc;	/**< Tx Probe Response Success (ACK was received) */
	uint32	prs_timeout;	/**< number of probe requests that were dropped from the PRQ
				 * fifo because a probe response could not be sent out within
				 * the time limit defined in M_PRS_MAXTIME
				 */
	uint32	txrtsfail;	/**< number of rts transmission failure that reach retry limit */
	uint32	txucast;	/**< number of unicast tx expecting response other than cts/cwcts */
	uint32  txinrtstxop;	/**< number of data frame transmissions during rts txop */
	uint32	rxback;		/**< blockack rxcnt */
	uint32	txback;		/**< blockack txcnt */
	uint32	bphy_rxcrsglitch;	/**< PHY count of bphy glitches */
	uint32	rxdrop20s;	/**< drop secondary cnt */
	uint32	rxtoolate;	/**< receive too late */
	uint32  bphy_badplcp;	/**< number of bad PLCP reception on BPHY rate */
} wl_cnt_ge40mcst_v1_t;

/** MACSTAT counters for ucode (corerev < 40) */
typedef struct {
	/* MAC counters: 32-bit version of d11.h's macstat_t */
	uint32	txallfrm;	/**< total number of frames sent, incl. Data, ACK, RTS, CTS,
				 * Control Management (includes retransmissions)
				 */
	uint32	txrtsfrm;	/**< number of RTS sent out by the MAC */
	uint32	txctsfrm;	/**< number of CTS sent out by the MAC */
	uint32	txackfrm;	/**< number of ACK frames sent out */
	uint32	txdnlfrm;	/**< number of Null-Data transmission generated from template  */
	uint32	txbcnfrm;	/**< beacons transmitted */
	uint32	txfunfl[6];	/**< per-fifo tx underflows */
	uint32	txampdu;	/**< number of AMPDUs transmitted */
	uint32	txmpdu;		/**< number of MPDUs transmitted */
	uint32	txtplunfl;	/**< Template underflows (mac was too slow to transmit ACK/CTS
				 * or BCN)
				 */
	uint32	txphyerror;	/**< Transmit phy error, type of error is reported in tx-status for
				 * driver enqueued frames
				 */
	uint32  pktengrxducast; /**< unicast frames rxed by the pkteng code */
	uint32  pktengrxdmcast; /**< multicast frames rxed by the pkteng code */
	uint32	rxfrmtoolong;	/**< Received frame longer than legal limit (2346 bytes) */
	uint32	rxfrmtooshrt; /**< Received frame did not contain enough bytes for its frame type */
	uint32	rxanyerr;	/**< Any RX error that is not counted by other counters. */
	uint32	rxbadfcs;	/**< number of frames for which the CRC check failed in the MAC */
	uint32	rxbadplcp;	/**< parity check of the PLCP header failed */
	uint32	rxcrsglitch;	/**< PHY was able to correlate the preamble but not the header */
	uint32	rxstrt;		/**< Number of received frames with a good PLCP
				 * (i.e. passing parity check)
				 */
	uint32	rxdtucastmbss; /**< number of received DATA frames with good FCS and matching RA */
	uint32	rxmgucastmbss; /**< number of received mgmt frames with good FCS and matching RA */
	uint32	rxctlucast; /**< number of received CNTRL frames with good FCS and matching RA */
	uint32	rxrtsucast;	/**< number of unicast RTS addressed to the MAC (good FCS) */
	uint32	rxctsucast;	/**< number of unicast CTS addressed to the MAC (good FCS) */
	uint32	rxackucast;	/**< number of ucast ACKS received (good FCS) */
	uint32	rxdtocast;  /**< number of received DATA frames (good FCS and not matching RA) */
	uint32	rxmgocast;  /**< number of received MGMT frames (good FCS and not matching RA) */
	uint32	rxctlocast; /**< number of received CNTRL frame (good FCS and not matching RA) */
	uint32	rxrtsocast;	/**< number of received RTS not addressed to the MAC */
	uint32	rxctsocast;	/**< number of received CTS not addressed to the MAC */
	uint32	rxdtmcast;	/**< number of RX Data multicast frames received by the MAC */
	uint32	rxmgmcast;	/**< number of RX Management multicast frames received by the MAC */
	uint32	rxctlmcast;	/**< number of RX Control multicast frames received by the MAC
				 * (unlikely to see these)
				 */
	uint32	rxbeaconmbss;	/**< beacons received from member of BSS */
	uint32	rxdtucastobss; /**< number of unicast frames addressed to the MAC from
				  * other BSS (WDS FRAME)
				  */
	uint32	rxbeaconobss;	/**< beacons received from other BSS */
	uint32	rxrsptmout;	/**< number of response timeouts for transmitted frames
				 * expecting a response
				 */
	uint32	bcntxcancl;	/**< transmit beacons canceled due to receipt of beacon (IBSS) */
	uint32	rxnodelim;	/**< number of no valid delimiter detected by ampdu parser */
	uint32	rxf0ovfl;	/**< number of receive fifo 0 overflows */
	uint32	dbgoff46;	/**< BTCX protection failure count,
				 * getting RX antenna in PHY DEBUG,
				 * PR84273 timeout count
				 */
	uint32	dbgoff47;	/**< BTCX preemption failure count,
				 * getting RX antenna in PHY DEBUG,
				 * PR84273 reset CCA count,
				 * RATEENGDBG
				 */
	uint32	dbgoff48;	/**< Used for counting txstatus queue overflow (corerev <= 4)  */
	uint32	pmqovfl;	/**< number of PMQ overflows */
	uint32	rxcgprqfrm;	/**< number of received Probe requests that made it into
				 * the PRQ fifo
				 */
	uint32	rxcgprsqovfl;	/**< Rx Probe Request Que overflow in the AP */
	uint32	txcgprsfail;	/**< Tx Probe Response Fail. AP sent probe response but did
				 * not get ACK
				 */
	uint32	txcgprssuc;	/**< Tx Probe Response Success (ACK was received) */
	uint32	prs_timeout;	/**< number of probe requests that were dropped from the PRQ
				 * fifo because a probe response could not be sent out within
				 * the time limit defined in M_PRS_MAXTIME
				 */
	uint32	txrtsfail;	/**< number of rts transmission failure that reach retry limit */
	uint32	txucast;	/**< number of unicast tx expecting response other than cts/cwcts */
	uint32  txinrtstxop;	/**< number of data frame transmissions during rts txop */
	uint32	rxback;		/**< blockack rxcnt */
	uint32	txback;		/**< blockack txcnt */
	uint32	bphy_rxcrsglitch;	/**< PHY count of bphy glitches */
	uint32	phywatch;	/**< number of phywatchdog to kill any pending transmissions.
				 * (PR 38187 corerev == 11)
				 */
	uint32	rxtoolate;	/**< receive too late */
	uint32  bphy_badplcp;	/**< number of bad PLCP reception on BPHY rate */
} wl_cnt_lt40mcst_v1_t;

/** MACSTAT counters for ucode (corerev >= 80) */
typedef struct {
	/* MAC counters: 32-bit version of d11.h's macstat_t */
	/* Start of PSM2HOST stats(72) block */
	uint32	txallfrm;	/**< total number of frames sent, incl. Data, ACK, RTS, CTS,
				 * Control Management (includes retransmissions)
				 */
	uint32	txrtsfrm;	/**< number of RTS sent out by the MAC */
	uint32	txctsfrm;	/**< number of CTS sent out by the MAC */
	uint32	txackfrm;	/**< number of ACK frames sent out */
	uint32	txdnlfrm;	/**< number of Null-Data transmission generated from template  */
	uint32	txbcnfrm;	/**< beacons transmitted */
	uint32	txampdu;	/**< number of AMPDUs transmitted */
	uint32	txmpdu;		/**< number of MPDUs transmitted */
	uint32	txtplunfl;	/**< Template underflows (mac was too slow to transmit ACK/CTS
				 * or BCN)
				 */
	uint32	txphyerror;	/**< Transmit phy error, type of error is reported in tx-status for
				 * driver enqueued frames
				 */
	uint32  pktengrxducast; /**< unicast frames rxed by the pkteng code */
	uint32  pktengrxdmcast; /**< multicast frames rxed by the pkteng code */
	uint32	rxfrmtoolong;	/**< Received frame longer than legal limit (2346 bytes) */
	uint32	rxfrmtooshrt; /**< Received frame did not contain enough bytes for its frame type */
	uint32	rxanyerr;	/**< Any RX error that is not counted by other counters. */
	uint32	rxbadfcs;	/**< number of frames for which the CRC check failed in the MAC */
	uint32	rxbadplcp;	/**< parity check of the PLCP header failed */
	uint32	rxcrsglitch;	/**< PHY was able to correlate the preamble but not the header */
	uint32	rxstrt;		/**< Number of received frames with a good PLCP
				 * (i.e. passing parity check)
				 */
	uint32	rxdtucastmbss; /**< number of received DATA frames with good FCS and matching RA */
	uint32	rxmgucastmbss; /**< number of received mgmt frames with good FCS and matching RA */
	uint32	rxctlucast; /**< number of received CNTRL frames with good FCS and matching RA */
	uint32	rxrtsucast;	/**< number of unicast RTS addressed to the MAC (good FCS) */
	uint32	rxctsucast;	/**< number of unicast CTS addressed to the MAC (good FCS) */
	uint32	rxackucast;	/**< number of ucast ACKS received (good FCS) */
	uint32	rxdtocast; /**< number of received DATA frames (good FCS and not matching RA) */
	uint32	rxmgocast; /**< number of received MGMT frames (good FCS and not matching RA) */
	uint32	rxctlocast; /**< number of received CNTRL frame (good FCS and not matching RA) */
	uint32	rxrtsocast;	/**< number of received RTS not addressed to the MAC */
	uint32	rxctsocast;	/**< number of received CTS not addressed to the MAC */
	uint32	rxdtmcast;	/**< number of RX Data multicast frames received by the MAC */
	uint32	rxmgmcast;	/**< number of RX Management multicast frames received by the MAC */
	uint32	rxctlmcast;	/**< number of RX Control multicast frames received by the MAC
				 * (unlikely to see these)
				 */
	uint32	rxbeaconmbss;	/**< beacons received from member of BSS */
	uint32	rxdtucastobss; /**< number of unicast frames addressed to the MAC from
				  * other BSS (WDS FRAME)
				  */
	uint32	rxbeaconobss;	/**< beacons received from other BSS */
	uint32	rxrsptmout;	/**< number of response timeouts for transmitted frames
				 * expecting a response
				 */
	uint32	bcntxcancl;	/**< transmit beacons canceled due to receipt of beacon (IBSS) */
	uint32	rxnodelim;	/**< number of no valid delimiter detected by ampdu parser */
	uint32	missbcn_dbg;	/**< number of beacon missed to receive */
	uint32	pmqovfl;	/**< number of PMQ overflows */
	uint32	rxcgprqfrm;	/**< number of received Probe requests that made it into
				 * the PRQ fifo
				 */
	uint32	rxcgprsqovfl;	/**< Rx Probe Request Que overflow in the AP */
	uint32	txcgprsfail;	/**< Tx Probe Response Fail. AP sent probe response but did
				 * not get ACK
				 */
	uint32	txcgprssuc;	/**< Tx Probe Response Success (ACK was received) */
	uint32	prs_timeout;	/**< number of probe requests that were dropped from the PRQ
				 * fifo because a probe response could not be sent out within
				 * the time limit defined in M_PRS_MAXTIME
				 */
	uint32	txrtsfail;	/**< number of rts transmission failure that reach retry limit */
	uint32	txucast;	/**< number of unicast tx expecting response other than cts/cwcts */
	uint32  txinrtstxop;	/**< number of data frame transmissions during rts txop */
	uint32	rxback;		/**< blockack rxcnt */
	uint32	txback;		/**< blockack txcnt */
	uint32	bphy_rxcrsglitch;	/**< PHY count of bphy glitches */
	uint32	rxdrop20s;	/**< drop secondary cnt */
	uint32	rxtoolate;	/**< receive too late */
	uint32  bphy_badplcp;	/**< number of bad PLCP reception on BPHY rate */
	uint32	rxtrig_myaid;	/* New counters added in corerev 80 */
	uint32	rxtrig_rand;
	uint32	goodfcs;
	uint32	colormiss;
	uint32	txmampdu;
	uint32	rxmtidback;
	uint32	rxmstaback;
	uint32	txfrag;
	/* End of PSM2HOST stats block */
	/* start of rxerror overflow counter(24) block which are modified/added in corerev 80 */
	uint32	phyovfl;
	uint32	rxf0ovfl;	/**< number of receive fifo 0 overflows */
	uint32	rxf1ovfl;	/**< number of receive fifo 1 overflows */
	uint32	lenfovfl;
	uint32	weppeof;
	uint32	badplcp;
	uint32	msduthresh;
	uint32	strmeof;
	uint32	stsfifofull;
	uint32	stsfifoerr;
	uint32	PAD[6];
	uint32	rxerr_stat;
	uint32	ctx_fifo_full;
	uint32	PAD[38]; /* PAD added for counter elements to be added soon */
} wl_cnt_ge80mcst_v1_t;

typedef struct {
	uint32 txfunfl[NFIFO_EXT];
} wl_cnt_ge80_txfunfl_v1_t;

/** MACSTAT counters for "wl counter" version <= 10 */
typedef struct {
	/* MAC counters: 32-bit version of d11.h's macstat_t */
	uint32	txallfrm;	/**< total number of frames sent, incl. Data, ACK, RTS, CTS,
				 * Control Management (includes retransmissions)
				 */
	uint32	txrtsfrm;	/**< number of RTS sent out by the MAC */
	uint32	txctsfrm;	/**< number of CTS sent out by the MAC */
	uint32	txackfrm;	/**< number of ACK frames sent out */
	uint32	txdnlfrm;	/**< number of Null-Data transmission generated from template  */
	uint32	txbcnfrm;	/**< beacons transmitted */
	uint32	txfunfl[6];	/**< per-fifo tx underflows */
	uint32	txfbw;		/**< transmit at fallback bw (dynamic bw) */
	uint32	PAD0;		/**< number of MPDUs transmitted */
	uint32	txtplunfl;	/**< Template underflows (mac was too slow to transmit ACK/CTS
				 * or BCN)
				 */
	uint32	txphyerror;	/**< Transmit phy error, type of error is reported in tx-status for
				 * driver enqueued frames
				 */
	uint32  pktengrxducast; /**< unicast frames rxed by the pkteng code */
	uint32  pktengrxdmcast; /**< multicast frames rxed by the pkteng code */
	uint32	rxfrmtoolong;	/**< Received frame longer than legal limit (2346 bytes) */
	uint32	rxfrmtooshrt; /**< Received frame did not contain enough bytes for its frame type */
	uint32	rxinvmachdr;	/**< Either the protocol version != 0 or frame type not
				 * data/control/management
				 */
	uint32	rxbadfcs;	/**< number of frames for which the CRC check failed in the MAC */
	uint32	rxbadplcp;	/**< parity check of the PLCP header failed */
	uint32	rxcrsglitch;	/**< PHY was able to correlate the preamble but not the header */
	uint32	rxstrt;		/**< Number of received frames with a good PLCP
				 * (i.e. passing parity check)
				 */
	uint32	rxdfrmucastmbss; /* number of received DATA frames with good FCS and matching RA */
	uint32	rxmfrmucastmbss; /* number of received mgmt frames with good FCS and matching RA */
	uint32	rxcfrmucast; /**< number of received CNTRL frames with good FCS and matching RA */
	uint32	rxrtsucast;  /**< number of unicast RTS addressed to the MAC (good FCS) */
	uint32	rxctsucast;  /**< number of unicast CTS addressed to the MAC (good FCS) */
	uint32	rxackucast;	/**< number of ucast ACKS received (good FCS) */
	uint32	rxdfrmocast; /**< number of received DATA frames (good FCS and not matching RA) */
	uint32	rxmfrmocast; /**< number of received MGMT frames (good FCS and not matching RA) */
	uint32	rxcfrmocast; /**< number of received CNTRL frame (good FCS and not matching RA) */
	uint32	rxrtsocast;	/**< number of received RTS not addressed to the MAC */
	uint32	rxctsocast;	/**< number of received CTS not addressed to the MAC */
	uint32	rxdfrmmcast;	/**< number of RX Data multicast frames received by the MAC */
	uint32	rxmfrmmcast;	/**< number of RX Management multicast frames received by the MAC */
	uint32	rxcfrmmcast;	/**< number of RX Control multicast frames received by the MAC
				 * (unlikely to see these)
				 */
	uint32	rxbeaconmbss;	/**< beacons received from member of BSS */
	uint32	rxdfrmucastobss; /**< number of unicast frames addressed to the MAC from
				  * other BSS (WDS FRAME)
				  */
	uint32	rxbeaconobss;	/**< beacons received from other BSS */
	uint32	rxrsptmout;	/**< number of response timeouts for transmitted frames
				 * expecting a response
				 */
	uint32	bcntxcancl;	/**< transmit beacons canceled due to receipt of beacon (IBSS) */
	uint32	PAD1;
	uint32	rxf0ovfl;	/**< number of receive fifo 0 overflows */
	uint32	rxf1ovfl;	/**< Number of receive fifo 1 overflows (obsolete) */
	uint32	rxf2ovfl;	/**< Number of receive fifo 2 overflows (obsolete) */
	uint32	txsfovfl;	/**< Number of transmit status fifo overflows (obsolete) */
	uint32	pmqovfl;	/**< number of PMQ overflows */
	uint32	rxcgprqfrm;	/**< number of received Probe requests that made it into
				 * the PRQ fifo
				 */
	uint32	rxcgprsqovfl;	/**< Rx Probe Request Que overflow in the AP */
	uint32	txcgprsfail;	/**< Tx Probe Response Fail. AP sent probe response but did
				 * not get ACK
				 */
	uint32	txcgprssuc;	/**< Tx Probe Response Success (ACK was received) */
	uint32	prs_timeout;	/**< number of probe requests that were dropped from the PRQ
				 * fifo because a probe response could not be sent out within
				 * the time limit defined in M_PRS_MAXTIME
				 */
	uint32	rxnack;		/**< obsolete */
	uint32	frmscons;	/**< obsolete */
	uint32  txnack;		/**< obsolete */
	uint32	rxback;		/**< blockack rxcnt */
	uint32	txback;		/**< blockack txcnt */
	uint32	bphy_rxcrsglitch;	/**< PHY count of bphy glitches */
	uint32	rxdrop20s;	/**< drop secondary cnt */
	uint32	rxtoolate;	/**< receive too late */
	uint32  bphy_badplcp;	/**< number of bad PLCP reception on BPHY rate */
} wl_cnt_v_le10_mcst_t;

#define MAX_RX_FIFO 4
#define WL_RXFIFO_CNT_VERSION  1   /* current version of wl_rxfifo_cnt_t */
typedef struct {
	/* Counters for frames received from rx fifos */
	uint16	version;
	uint16	length;		/* length of entire structure */
	uint32	rxf_data[MAX_RX_FIFO];		/* data frames from rx fifo */
	uint32	rxf_mgmtctl[MAX_RX_FIFO];	/* mgmt/ctl frames from rx fifo */
} wl_rxfifo_cnt_t;

typedef struct {
	uint16	version;	/**< see definition of WL_CNT_T_VERSION */
	uint16	length;		/**< length of entire structure */

	/* transmit stat counters */
	uint32	txframe;	/**< tx data frames */
	uint32	txbyte;		/**< tx data bytes */
	uint32	txretrans;	/**< tx mac retransmits */
	uint32	txerror;	/**< tx data errors (derived: sum of others) */
	uint32	txctl;		/**< tx management frames */
	uint32	txprshort;	/**< tx short preamble frames */
	uint32	txserr;		/**< tx status errors */
	uint32	txnobuf;	/**< tx out of buffers errors */
	uint32	txnoassoc;	/**< tx discard because we're not associated */
	uint32	txrunt;		/**< tx runt frames */
	uint32	txchit;		/**< tx header cache hit (fastpath) */
	uint32	txcmiss;	/**< tx header cache miss (slowpath) */

	/* transmit chip error counters */
	uint32	txuflo;		/**< tx fifo underflows */
	uint32	txphyerr;	/**< tx phy errors (indicated in tx status) */
	uint32	txphycrs;

	/* receive stat counters */
	uint32	rxframe;	/**< rx data frames */
	uint32	rxbyte;		/**< rx data bytes */
	uint32	rxerror;	/**< rx data errors (derived: sum of others) */
	uint32	rxctl;		/**< rx management frames */
	uint32	rxnobuf;	/**< rx out of buffers errors */
	uint32	rxnondata;	/**< rx non data frames in the data channel errors */
	uint32	rxbadds;	/**< rx bad DS errors */
	uint32	rxbadcm;	/**< rx bad control or management frames */
	uint32	rxfragerr;	/**< rx fragmentation errors */
	uint32	rxrunt;		/**< rx runt frames */
	uint32	rxgiant;	/**< rx giant frames */
	uint32	rxnoscb;	/**< rx no scb error */
	uint32	rxbadproto;	/**< rx invalid frames */
	uint32	rxbadsrcmac;	/**< rx frames with Invalid Src Mac */
	uint32	rxbadda;	/**< rx frames tossed for invalid da */
	uint32	rxfilter;	/**< rx frames filtered out */

	/* receive chip error counters */
	uint32	rxoflo;		/**< rx fifo overflow errors */
	uint32	rxuflo[NFIFO];	/**< rx dma descriptor underflow errors */

	uint32	d11cnt_txrts_off;	/**< d11cnt txrts value when reset d11cnt */
	uint32	d11cnt_rxcrc_off;	/**< d11cnt rxcrc value when reset d11cnt */
	uint32	d11cnt_txnocts_off;	/**< d11cnt txnocts value when reset d11cnt */

	/* misc counters */
	uint32	dmade;		/**< tx/rx dma descriptor errors */
	uint32	dmada;		/**< tx/rx dma data errors */
	uint32	dmape;		/**< tx/rx dma descriptor protocol errors */
	uint32	reset;		/**< reset count */
	uint32	tbtt;		/**< cnts the TBTT int's */
	uint32	txdmawar;
	uint32	pkt_callback_reg_fail;	/**< callbacks register failure */

	/* MAC counters: 32-bit version of d11.h's macstat_t */
	uint32	txallfrm;	/**< total number of frames sent, incl. Data, ACK, RTS, CTS,
				 * Control Management (includes retransmissions)
				 */
	uint32	txrtsfrm;	/**< number of RTS sent out by the MAC */
	uint32	txctsfrm;	/**< number of CTS sent out by the MAC */
	uint32	txackfrm;	/**< number of ACK frames sent out */
	uint32	txdnlfrm;	/**< Not used */
	uint32	txbcnfrm;	/**< beacons transmitted */
	uint32	txfunfl[6];	/**< per-fifo tx underflows */
	uint32	rxtoolate;	/**< receive too late */
	uint32  txfbw;		/**< transmit at fallback bw (dynamic bw) */
	uint32	txtplunfl;	/**< Template underflows (mac was too slow to transmit ACK/CTS
				 * or BCN)
				 */
	uint32	txphyerror;	/**< Transmit phy error, type of error is reported in tx-status for
				 * driver enqueued frames
				 */
	uint32	rxfrmtoolong;	/**< Received frame longer than legal limit (2346 bytes) */
	uint32	rxfrmtooshrt; /**< Received frame did not contain enough bytes for its frame type */
	uint32	rxinvmachdr;	/**< Either the protocol version != 0 or frame type not
				 * data/control/management
				 */
	uint32	rxbadfcs;	/**< number of frames for which the CRC check failed in the MAC */
	uint32	rxbadplcp;	/**< parity check of the PLCP header failed */
	uint32	rxcrsglitch;	/**< PHY was able to correlate the preamble but not the header */
	uint32	rxstrt;		/**< Number of received frames with a good PLCP
				 * (i.e. passing parity check)
				 */
	uint32	rxdfrmucastmbss; /* Number of received DATA frames with good FCS and matching RA */
	uint32	rxmfrmucastmbss; /* number of received mgmt frames with good FCS and matching RA */
	uint32	rxcfrmucast; /**< number of received CNTRL frames with good FCS and matching RA */
	uint32	rxrtsucast;	/**< number of unicast RTS addressed to the MAC (good FCS) */
	uint32	rxctsucast;	/**< number of unicast CTS addressed to the MAC (good FCS) */
	uint32	rxackucast;	/**< number of ucast ACKS received (good FCS) */
	uint32	rxdfrmocast; /**< number of received DATA frames (good FCS and not matching RA) */
	uint32	rxmfrmocast; /**< number of received MGMT frames (good FCS and not matching RA) */
	uint32	rxcfrmocast; /**< number of received CNTRL frame (good FCS and not matching RA) */
	uint32	rxrtsocast;	/**< number of received RTS not addressed to the MAC */
	uint32	rxctsocast;	/**< number of received CTS not addressed to the MAC */
	uint32	rxdfrmmcast;	/**< number of RX Data multicast frames received by the MAC */
	uint32	rxmfrmmcast;	/**< number of RX Management multicast frames received by the MAC */
	uint32	rxcfrmmcast;	/**< number of RX Control multicast frames received by the MAC
				 * (unlikely to see these)
				 */
	uint32	rxbeaconmbss;	/**< beacons received from member of BSS */
	uint32	rxdfrmucastobss; /**< number of unicast frames addressed to the MAC from
				  * other BSS (WDS FRAME)
				  */
	uint32	rxbeaconobss;	/**< beacons received from other BSS */
	uint32	rxrsptmout;	/**< Number of response timeouts for transmitted frames
				 * expecting a response
				 */
	uint32	bcntxcancl;	/**< transmit beacons canceled due to receipt of beacon (IBSS) */
	uint32	rxf0ovfl;	/**< Number of receive fifo 0 overflows */
	uint32	rxf1ovfl;	/**< Number of receive fifo 1 overflows (obsolete) */
	uint32	rxf2ovfl;	/**< Number of receive fifo 2 overflows (obsolete) */
	uint32	txsfovfl;	/**< Number of transmit status fifo overflows (obsolete) */
	uint32	pmqovfl;	/**< Number of PMQ overflows */
	uint32	rxcgprqfrm;	/**< Number of received Probe requests that made it into
				 * the PRQ fifo
				 */
	uint32	rxcgprsqovfl;	/**< Rx Probe Request Que overflow in the AP */
	uint32	txcgprsfail;	/**< Tx Probe Response Fail. AP sent probe response but did
				 * not get ACK
				 */
	uint32	txcgprssuc;	/**< Tx Probe Response Success (ACK was received) */
	uint32	prs_timeout;	/**< Number of probe requests that were dropped from the PRQ
				 * fifo because a probe response could not be sent out within
				 * the time limit defined in M_PRS_MAXTIME
				 */
	uint32	rxnack;		/**< obsolete */
	uint32	frmscons;	/**< obsolete */
	uint32  txnack;		/**< obsolete */
	uint32	rxback;		/**< blockack rxcnt */
	uint32	txback;		/**< blockack txcnt */

	/* 802.11 MIB counters, pp. 614 of 802.11 reaff doc. */
	uint32	txfrag;		/**< dot11TransmittedFragmentCount */
	uint32	txmulti;	/**< dot11MulticastTransmittedFrameCount */
	uint32	txfail;		/**< dot11FailedCount */
	uint32	txretry;	/**< dot11RetryCount */
	uint32	txretrie;	/**< dot11MultipleRetryCount */
	uint32	rxdup;		/**< dot11FrameduplicateCount */
	uint32	txrts;		/**< dot11RTSSuccessCount */
	uint32	txnocts;	/**< dot11RTSFailureCount */
	uint32	txnoack;	/**< dot11ACKFailureCount */
	uint32	rxfrag;		/**< dot11ReceivedFragmentCount */
	uint32	rxmulti;	/**< dot11MulticastReceivedFrameCount */
	uint32	rxcrc;		/**< dot11FCSErrorCount */
	uint32	txfrmsnt;	/**< dot11TransmittedFrameCount (bogus MIB?) */
	uint32	rxundec;	/**< dot11WEPUndecryptableCount */

	/* WPA2 counters (see rxundec for DecryptFailureCount) */
	uint32	tkipmicfaill;	/**< TKIPLocalMICFailures */
	uint32	tkipcntrmsr;	/**< TKIPCounterMeasuresInvoked */
	uint32	tkipreplay;	/**< TKIPReplays */
	uint32	ccmpfmterr;	/**< CCMPFormatErrors */
	uint32	ccmpreplay;	/**< CCMPReplays */
	uint32	ccmpundec;	/**< CCMPDecryptErrors */
	uint32	fourwayfail;	/**< FourWayHandshakeFailures */
	uint32	wepundec;	/**< dot11WEPUndecryptableCount */
	uint32	wepicverr;	/**< dot11WEPICVErrorCount */
	uint32	decsuccess;	/**< DecryptSuccessCount */
	uint32	tkipicverr;	/**< TKIPICVErrorCount */
	uint32	wepexcluded;	/**< dot11WEPExcludedCount */

	uint32	txchanrej;	/**< Tx frames suppressed due to channel rejection */
	uint32	psmwds;		/**< Count PSM watchdogs */
	uint32	phywatchdog;	/**< Count Phy watchdogs (triggered by ucode) */

	/* MBSS counters, AP only */
	uint32	prq_entries_handled;	/**< PRQ entries read in */
	uint32	prq_undirected_entries;	/**<    which were bcast bss & ssid */
	uint32	prq_bad_entries;	/**<    which could not be translated to info */
	uint32	atim_suppress_count;	/**< TX suppressions on ATIM fifo */
	uint32	bcn_template_not_ready;	/**< Template marked in use on send bcn ... */
	uint32	bcn_template_not_ready_done; /**< ...but "DMA done" interrupt rcvd */
	uint32	late_tbtt_dpc;	/**< TBTT DPC did not happen in time */

	/* per-rate receive stat counters */
	uint32  rx1mbps;	/**< packets rx at 1Mbps */
	uint32  rx2mbps;	/**< packets rx at 2Mbps */
	uint32  rx5mbps5;	/**< packets rx at 5.5Mbps */
	uint32  rx6mbps;	/**< packets rx at 6Mbps */
	uint32  rx9mbps;	/**< packets rx at 9Mbps */
	uint32  rx11mbps;	/**< packets rx at 11Mbps */
	uint32  rx12mbps;	/**< packets rx at 12Mbps */
	uint32  rx18mbps;	/**< packets rx at 18Mbps */
	uint32  rx24mbps;	/**< packets rx at 24Mbps */
	uint32  rx36mbps;	/**< packets rx at 36Mbps */
	uint32  rx48mbps;	/**< packets rx at 48Mbps */
	uint32  rx54mbps;	/**< packets rx at 54Mbps */
	uint32  rx108mbps;	/**< packets rx at 108mbps */
	uint32  rx162mbps;	/**< packets rx at 162mbps */
	uint32  rx216mbps;	/**< packets rx at 216 mbps */
	uint32  rx270mbps;	/**< packets rx at 270 mbps */
	uint32  rx324mbps;	/**< packets rx at 324 mbps */
	uint32  rx378mbps;	/**< packets rx at 378 mbps */
	uint32  rx432mbps;	/**< packets rx at 432 mbps */
	uint32  rx486mbps;	/**< packets rx at 486 mbps */
	uint32  rx540mbps;	/**< packets rx at 540 mbps */

	/* pkteng rx frame stats */
	uint32	pktengrxducast; /**< unicast frames rxed by the pkteng code */
	uint32	pktengrxdmcast; /**< multicast frames rxed by the pkteng code */

	uint32	rfdisable;	/**< count of radio disables */
	uint32	bphy_rxcrsglitch;	/**< PHY count of bphy glitches */
	uint32  bphy_badplcp;

	uint32	txexptime;	/**< Tx frames suppressed due to timer expiration */

	uint32	txmpdu_sgi;	/**< count for sgi transmit */
	uint32	rxmpdu_sgi;	/**< count for sgi received */
	uint32	txmpdu_stbc;	/**< count for stbc transmit */
	uint32	rxmpdu_stbc;	/**< count for stbc received */

	uint32	rxundec_mcst;	/**< dot11WEPUndecryptableCount */

	/* WPA2 counters (see rxundec for DecryptFailureCount) */
	uint32	tkipmicfaill_mcst;	/**< TKIPLocalMICFailures */
	uint32	tkipcntrmsr_mcst;	/**< TKIPCounterMeasuresInvoked */
	uint32	tkipreplay_mcst;	/**< TKIPReplays */
	uint32	ccmpfmterr_mcst;	/**< CCMPFormatErrors */
	uint32	ccmpreplay_mcst;	/**< CCMPReplays */
	uint32	ccmpundec_mcst;	/**< CCMPDecryptErrors */
	uint32	fourwayfail_mcst;	/**< FourWayHandshakeFailures */
	uint32	wepundec_mcst;	/**< dot11WEPUndecryptableCount */
	uint32	wepicverr_mcst;	/**< dot11WEPICVErrorCount */
	uint32	decsuccess_mcst;	/**< DecryptSuccessCount */
	uint32	tkipicverr_mcst;	/**< TKIPICVErrorCount */
	uint32	wepexcluded_mcst;	/**< dot11WEPExcludedCount */

	uint32	dma_hang;	/**< count for dma hang */
	uint32	reinit;		/**< count for reinit */

	uint32  pstatxucast;	/**< count of ucast frames xmitted on all psta assoc */
	uint32  pstatxnoassoc;	/**< count of txnoassoc frames xmitted on all psta assoc */
	uint32  pstarxucast;	/**< count of ucast frames received on all psta assoc */
	uint32  pstarxbcmc;	/**< count of bcmc frames received on all psta */
	uint32  pstatxbcmc;	/**< count of bcmc frames transmitted on all psta */

	uint32  cso_passthrough; /**< hw cso required but passthrough */
	uint32	cso_normal;	/**< hw cso hdr for normal process */
	uint32	chained;	/**< number of frames chained */
	uint32	chainedsz1;	/**< number of chain size 1 frames */
	uint32	unchained;	/**< number of frames not chained */
	uint32	maxchainsz;	/**< max chain size so far */
	uint32	currchainsz;	/**< current chain size */
	uint32	rxdrop20s;	/**< drop secondary cnt */
	uint32	pciereset;	/**< Secondary Bus Reset issued by driver */
	uint32	cfgrestore;	/**< configspace restore by driver */
	uint32	reinitreason[NREINITREASONCOUNT]; /**< reinitreason counters; 0: Unknown reason */
	uint32  rxrtry;		/**< num of received packets with retry bit on */
	uint32	txmpdu;		/**< macstat cnt only valid in ver 11. number of MPDUs txed.  */
	uint32	rxnodelim;	/**< macstat cnt only valid in ver 11.
				 * number of occasions that no valid delimiter is detected
				 * by ampdu parser.
				 */
	uint32  rxmpdu_mu;      /**< Number of MU MPDUs received */

	/* detailed control/management frames */
	uint32	txbar;		/**< Number of TX BAR */
	uint32	rxbar;		/**< Number of RX BAR */
	uint32	txpspoll;	/**< Number of TX PS-poll */
	uint32	rxpspoll;	/**< Number of RX PS-poll */
	uint32	txnull;		/**< Number of TX NULL_DATA */
	uint32	rxnull;		/**< Number of RX NULL_DATA */
	uint32	txqosnull;	/**< Number of TX NULL_QoSDATA */
	uint32	rxqosnull;	/**< Number of RX NULL_QoSDATA */
	uint32	txassocreq;	/**< Number of TX ASSOC request */
	uint32	rxassocreq;	/**< Number of RX ASSOC request */
	uint32	txreassocreq;	/**< Number of TX REASSOC request */
	uint32	rxreassocreq;	/**< Number of RX REASSOC request */
	uint32	txdisassoc;	/**< Number of TX DISASSOC */
	uint32	rxdisassoc;	/**< Number of RX DISASSOC */
	uint32	txassocrsp;	/**< Number of TX ASSOC response */
	uint32	rxassocrsp;	/**< Number of RX ASSOC response */
	uint32	txreassocrsp;	/**< Number of TX REASSOC response */
	uint32	rxreassocrsp;	/**< Number of RX REASSOC response */
	uint32	txauth;		/**< Number of TX AUTH */
	uint32	rxauth;		/**< Number of RX AUTH */
	uint32	txdeauth;	/**< Number of TX DEAUTH */
	uint32	rxdeauth;	/**< Number of RX DEAUTH */
	uint32	txprobereq;	/**< Number of TX probe request */
	uint32	rxprobereq;	/**< Number of RX probe request */
	uint32	txprobersp;	/**< Number of TX probe response */
	uint32	rxprobersp;	/**< Number of RX probe response */
	uint32	txaction;	/**< Number of TX action frame */
	uint32	rxaction;	/**< Number of RX action frame */
	uint32  ampdu_wds;      /**< Number of AMPDU watchdogs */
	uint32  txlost;         /**< Number of lost packets reported in txs */
	uint32  txdatamcast;	/**< Number of TX multicast data packets */
	uint32  txdatabcast;	/**< Number of TX broadcast data packets */
	uint32  txbcast;        /* Broadcast TransmittedFrameCount */
	uint32  txdropped;      /* tx dropped pkts */
	uint32  rxbcast;        /* BroadcastReceivedFrameCount */
	uint32  rxdropped;      /* rx dropped pkts (derived: sum of others) */

	/* XXX: This structure is deprecated and used only for ver <= 11.
	 * All counter variables have to be of uint32.
	 * Please refer to the following twiki before editing.
	 * http://hwnbu-twiki.sj.broadcom.com/bin/view/
	 * Mwgroup/WlCounters#wlc_layer_counters_non_xTLV
	 */
} wl_cnt_ver_11_t;

/** EAP counters */
typedef struct {
	uint32  rxmcs0;		/* packets rx at MCS index 0 */
	uint32  rxmcs1;		/* packets rx at MCS index 1 */
	uint32  rxmcs2;		/* packets rx at MCS index 2 */
	uint32  rxmcs3;		/* packets rx at MCS index 3 */
	uint32  rxmcs4;		/* packets rx at MCS index 4 */
	uint32  rxmcs5;		/* packets rx at MCS index 5 */
	uint32  rxmcs6;		/* packets rx at MCS index 6 */
	uint32  rxmcs7;		/* packets rx at MCS index 7 */
	uint32  rxmcs8;		/* packets rx at MCS index 8 */
	uint32  rxmcs9;		/* packets rx at MCS index 9 */
	uint32  tx1mbps;	/* packets tx at 1Mbps */
	uint32  tx2mbps;	/* packets tx at 2Mbps */
	uint32  tx5mbps5;	/* packets tx at 5.5Mbps */
	uint32  tx6mbps;	/* packets tx at 6Mbps */
	uint32  tx9mbps;	/* packets tx at 9Mbps */
	uint32  tx11mbps;	/* packets tx at 11Mbps */
	uint32  tx12mbps;	/* packets tx at 12Mbps */
	uint32  tx18mbps;	/* packets tx at 18Mbps */
	uint32  tx24mbps;	/* packets tx at 24Mbps */
	uint32  tx36mbps;	/* packets tx at 36Mbps */
	uint32  tx48mbps;	/* packets tx at 48Mbps */
	uint32  tx54mbps;	/* packets tx at 54Mbps */
	uint32  tx1mbps_succ;	/* packets tx at 1Mbps */
	uint32  tx2mbps_succ;	/* packets tx at 2Mbps */
	uint32  tx5mbps5_succ;	/* packets tx at 5.5Mbps */
	uint32  tx6mbps_succ;	/* packets tx at 6Mbps */
	uint32  tx9mbps_succ;	/* packets tx at 9Mbps */
	uint32  tx11mbps_succ;	/* packets tx at 11Mbps */
	uint32  tx12mbps_succ;	/* packets tx at 12Mbps */
	uint32  tx18mbps_succ;	/* packets tx at 18Mbps */
	uint32  tx24mbps_succ;	/* packets tx at 24Mbps */
	uint32  tx36mbps_succ;	/* packets tx at 36Mbps */
	uint32  tx48mbps_succ;	/* packets tx at 48Mbps */
	uint32  tx54mbps_succ;	/* packets tx at 54Mbps */
	uint32  txmcs0;		/* packets tx at MCS index 0 */
	uint32  txmcs1;		/* packets tx at MCS index 1 */
	uint32  txmcs2;		/* packets tx at MCS index 2 */
	uint32  txmcs3;		/* packets tx at MCS index 3 */
	uint32  txmcs4;		/* packets tx at MCS index 4 */
	uint32  txmcs5;		/* packets tx at MCS index 5 */
	uint32  txmcs6;		/* packets tx at MCS index 6 */
	uint32  txmcs7;		/* packets tx at MCS index 7 */
	uint32  txmcs8;		/* packets tx at MCS index 8 */
	uint32  txmcs9;		/* packets tx at MCS index 9 */
	uint32  txmcs0_succ;		/* packets tx at MCS index 0 */
	uint32  txmcs1_succ;		/* packets tx at MCS index 1 */
	uint32  txmcs2_succ;		/* packets tx at MCS index 2 */
	uint32  txmcs3_succ;		/* packets tx at MCS index 3 */
	uint32  txmcs4_succ;		/* packets tx at MCS index 4 */
	uint32  txmcs5_succ;		/* packets tx at MCS index 5 */
	uint32  txmcs6_succ;		/* packets tx at MCS index 6 */
	uint32  txmcs7_succ;		/* packets tx at MCS index 7 */
	uint32  txmcs8_succ;		/* packets tx at MCS index 8 */
	uint32  txmcs9_succ;		/* packets tx at MCS index 9 */
	uint32  probrespexpired;	/* expired probe responses */
	uint32  rxmcs10;		/* packets rx at MCS index 10 */
	uint32  rxmcs11;		/* packets rx at MCS index 11 */
	uint32  txmcs10;		/* packets tx at MCS index 10 */
	uint32  txmcs11;		/* packets tx at MCS index 11 */
	uint32  txmcs10_succ;		/* packets tx at MCS index 10 */
	uint32  txmcs11_succ;		/* packets tx at MCS index 11 */
	uint32  txmgmt;			/* tx mgmt frames */
	uint32  rxmgmt;			/* rx mgmt frames */
} wl_cnt_eap_t;

typedef struct {
	uint16  version;    /**< see definition of WL_CNT_T_VERSION */
	uint16  length;     /**< length of entire structure */

	/* transmit stat counters */
	uint32  txframe;    /**< tx data frames */
	uint32  txbyte;     /**< tx data bytes */
	uint32  txretrans;  /**< tx mac retransmits */
	uint32  txerror;    /**< tx data errors (derived: sum of others) */
	uint32  txctl;      /**< tx management frames */
	uint32  txprshort;  /**< tx short preamble frames */
	uint32  txserr;     /**< tx status errors */
	uint32  txnobuf;    /**< tx out of buffers errors */
	uint32  txnoassoc;  /**< tx discard because we're not associated */
	uint32  txrunt;     /**< tx runt frames */
	uint32  txchit;     /**< tx header cache hit (fastpath) */
	uint32  txcmiss;    /**< tx header cache miss (slowpath) */

	/* transmit chip error counters */
	uint32  txuflo;     /**< tx fifo underflows */
	uint32  txphyerr;   /**< tx phy errors (indicated in tx status) */
	uint32  txphycrs;

	/* receive stat counters */
	uint32  rxframe;    /**< rx data frames */
	uint32  rxbyte;     /**< rx data bytes */
	uint32  rxerror;    /**< rx data errors (derived: sum of others) */
	uint32  rxctl;      /**< rx management frames */
	uint32  rxnobuf;    /**< rx out of buffers errors */
	uint32  rxnondata;  /**< rx non data frames in the data channel errors */
	uint32  rxbadds;    /**< rx bad DS errors */
	uint32  rxbadcm;    /**< rx bad control or management frames */
	uint32  rxfragerr;  /**< rx fragmentation errors */
	uint32  rxrunt;     /**< rx runt frames */
	uint32  rxgiant;    /**< rx giant frames */
	uint32  rxnoscb;    /**< rx no scb error */
	uint32  rxbadproto; /**< rx invalid frames */
	uint32  rxbadsrcmac;    /**< rx frames with Invalid Src Mac */
	uint32  rxbadda;    /**< rx frames tossed for invalid da */
	uint32  rxfilter;   /**< rx frames filtered out */

	/* receive chip error counters */
	uint32  rxoflo;     /**< rx fifo overflow errors */
	uint32  rxuflo[NFIFO];  /**< rx dma descriptor underflow errors */

	uint32  d11cnt_txrts_off;   /**< d11cnt txrts value when reset d11cnt */
	uint32  d11cnt_rxcrc_off;   /**< d11cnt rxcrc value when reset d11cnt */
	uint32  d11cnt_txnocts_off; /**< d11cnt txnocts value when reset d11cnt */

	/* misc counters */
	uint32  dmade;      /**< tx/rx dma descriptor errors */
	uint32  dmada;      /**< tx/rx dma data errors */
	uint32  dmape;      /**< tx/rx dma descriptor protocol errors */
	uint32  reset;      /**< reset count */
	uint32  tbtt;       /**< cnts the TBTT int's */
	uint32  txdmawar;
	uint32  pkt_callback_reg_fail;  /**< callbacks register failure */

	/* MAC counters: 32-bit version of d11.h's macstat_t */
	uint32  txallfrm;   /**< total number of frames sent, incl. Data, ACK, RTS, CTS,
			     * Control Management (includes retransmissions)
			     */
	uint32  txrtsfrm;   /**< number of RTS sent out by the MAC */
	uint32  txctsfrm;   /**< number of CTS sent out by the MAC */
	uint32  txackfrm;   /**< number of ACK frames sent out */
	uint32  txdnlfrm;   /**< Not used */
	uint32  txbcnfrm;   /**< beacons transmitted */
	uint32  txfunfl[6]; /**< per-fifo tx underflows */
	uint32	rxtoolate;	/**< receive too late */
	uint32  txfbw;	    /**< transmit at fallback bw (dynamic bw) */
	uint32  txtplunfl;  /**< Template underflows (mac was too slow to transmit ACK/CTS
			     * or BCN)
			     */
	uint32  txphyerror; /**< Transmit phy error, type of error is reported in tx-status for
			     * driver enqueued frames
			     */
	uint32  rxfrmtoolong;   /**< Received frame longer than legal limit (2346 bytes) */
	uint32  rxfrmtooshrt; /**< Received frame did not contain enough bytes for its frame type */
	uint32  rxinvmachdr;    /**< Either the protocol version != 0 or frame type not
				 * data/control/management
			   */
	uint32  rxbadfcs;   /**< number of frames for which the CRC check failed in the MAC */
	uint32  rxbadplcp;  /**< parity check of the PLCP header failed */
	uint32  rxcrsglitch;    /**< PHY was able to correlate the preamble but not the header */
	uint32  rxstrt;     /**< Number of received frames with a good PLCP
			     * (i.e. passing parity check)
			     */
	uint32  rxdfrmucastmbss; /**< # of received DATA frames with good FCS and matching RA */
	uint32  rxmfrmucastmbss; /**< # of received mgmt frames with good FCS and matching RA */
	uint32  rxcfrmucast;     /**< # of received CNTRL frames with good FCS and matching RA */
	uint32  rxrtsucast; /**< number of unicast RTS addressed to the MAC (good FCS) */
	uint32  rxctsucast; /**< number of unicast CTS addressed to the MAC (good FCS) */
	uint32  rxackucast; /**< number of ucast ACKS received (good FCS) */
	uint32  rxdfrmocast;    /**< # of received DATA frames (good FCS and not matching RA) */
	uint32  rxmfrmocast;    /**< # of received MGMT frames (good FCS and not matching RA) */
	uint32  rxcfrmocast;    /**< # of received CNTRL frame (good FCS and not matching RA) */
	uint32  rxrtsocast; /**< number of received RTS not addressed to the MAC */
	uint32  rxctsocast; /**< number of received CTS not addressed to the MAC */
	uint32  rxdfrmmcast;    /**< number of RX Data multicast frames received by the MAC */
	uint32  rxmfrmmcast;    /**< number of RX Management multicast frames received by the MAC */
	uint32  rxcfrmmcast;    /**< number of RX Control multicast frames received by the MAC
				 * (unlikely to see these)
				 */
	uint32  rxbeaconmbss;   /**< beacons received from member of BSS */
	uint32  rxdfrmucastobss; /**< number of unicast frames addressed to the MAC from
				  * other BSS (WDS FRAME)
				  */
	uint32  rxbeaconobss;   /**< beacons received from other BSS */
	uint32  rxrsptmout; /**< Number of response timeouts for transmitted frames
			     * expecting a response
			     */
	uint32  bcntxcancl; /**< transmit beacons canceled due to receipt of beacon (IBSS) */
	uint32  rxf0ovfl;   /**< Number of receive fifo 0 overflows */
	uint32  rxf1ovfl;   /**< Number of receive fifo 1 overflows (obsolete) */
	uint32  rxf2ovfl;   /**< Number of receive fifo 2 overflows (obsolete) */
	uint32  txsfovfl;   /**< Number of transmit status fifo overflows (obsolete) */
	uint32  pmqovfl;    /**< Number of PMQ overflows */
	uint32  rxcgprqfrm; /**< Number of received Probe requests that made it into
			     * the PRQ fifo
			     */
	uint32  rxcgprsqovfl;   /**< Rx Probe Request Que overflow in the AP */
	uint32  txcgprsfail;    /**< Tx Probe Response Fail. AP sent probe response but did
				 * not get ACK
				 */
	uint32  txcgprssuc; /**< Tx Probe Response Success (ACK was received) */
	uint32  prs_timeout;    /**< Number of probe requests that were dropped from the PRQ
				 * fifo because a probe response could not be sent out within
				 * the time limit defined in M_PRS_MAXTIME
				 */
	uint32  rxnack;
	uint32  frmscons;   /**< XXX Number of frames completed without transmission because of an
			     * Afterburner re-queue
			     */
	uint32  txnack;		/**< obsolete */
	uint32	rxback;		/**< blockack rxcnt */
	uint32	txback;		/**< blockack txcnt */

	/* 802.11 MIB counters, pp. 614 of 802.11 reaff doc. */
	uint32  txfrag;     /**< dot11TransmittedFragmentCount */
	uint32  txmulti;    /**< dot11MulticastTransmittedFrameCount */
	uint32  txfail;     /**< dot11FailedCount */
	uint32  txretry;    /**< dot11RetryCount */
	uint32  txretrie;   /**< dot11MultipleRetryCount */
	uint32  rxdup;      /**< dot11FrameduplicateCount */
	uint32  txrts;      /**< dot11RTSSuccessCount */
	uint32  txnocts;    /**< dot11RTSFailureCount */
	uint32  txnoack;    /**< dot11ACKFailureCount */
	uint32  rxfrag;     /**< dot11ReceivedFragmentCount */
	uint32  rxmulti;    /**< dot11MulticastReceivedFrameCount */
	uint32  rxcrc;      /**< dot11FCSErrorCount */
	uint32  txfrmsnt;   /**< dot11TransmittedFrameCount (bogus MIB?) */
	uint32  rxundec;    /**< dot11WEPUndecryptableCount */

	/* WPA2 counters (see rxundec for DecryptFailureCount) */
	uint32  tkipmicfaill;   /**< TKIPLocalMICFailures */
	uint32  tkipcntrmsr;    /**< TKIPCounterMeasuresInvoked */
	uint32  tkipreplay; /**< TKIPReplays */
	uint32  ccmpfmterr; /**< CCMPFormatErrors */
	uint32  ccmpreplay; /**< CCMPReplays */
	uint32  ccmpundec;  /**< CCMPDecryptErrors */
	uint32  fourwayfail;    /**< FourWayHandshakeFailures */
	uint32  wepundec;   /**< dot11WEPUndecryptableCount */
	uint32  wepicverr;  /**< dot11WEPICVErrorCount */
	uint32  decsuccess; /**< DecryptSuccessCount */
	uint32  tkipicverr; /**< TKIPICVErrorCount */
	uint32  wepexcluded;    /**< dot11WEPExcludedCount */

	uint32  rxundec_mcst;   /**< dot11WEPUndecryptableCount */

	/* WPA2 counters (see rxundec for DecryptFailureCount) */
	uint32  tkipmicfaill_mcst;  /**< TKIPLocalMICFailures */
	uint32  tkipcntrmsr_mcst;   /**< TKIPCounterMeasuresInvoked */
	uint32  tkipreplay_mcst;    /**< TKIPReplays */
	uint32  ccmpfmterr_mcst;    /**< CCMPFormatErrors */
	uint32  ccmpreplay_mcst;    /**< CCMPReplays */
	uint32  ccmpundec_mcst; /**< CCMPDecryptErrors */
	uint32  fourwayfail_mcst;   /**< FourWayHandshakeFailures */
	uint32  wepundec_mcst;  /**< dot11WEPUndecryptableCount */
	uint32  wepicverr_mcst; /**< dot11WEPICVErrorCount */
	uint32  decsuccess_mcst;    /**< DecryptSuccessCount */
	uint32  tkipicverr_mcst;    /**< TKIPICVErrorCount */
	uint32  wepexcluded_mcst;   /**< dot11WEPExcludedCount */

	uint32  txchanrej;  /**< Tx frames suppressed due to channel rejection */
	uint32  txexptime;  /**< Tx frames suppressed due to timer expiration */
	uint32  psmwds;     /**< Count PSM watchdogs */
	uint32  phywatchdog;    /**< Count Phy watchdogs (triggered by ucode) */

	/* MBSS counters, AP only */
	uint32  prq_entries_handled;    /**< PRQ entries read in */
	uint32  prq_undirected_entries; /**<    which were bcast bss & ssid */
	uint32  prq_bad_entries;    /**<    which could not be translated to info */
	uint32  atim_suppress_count;    /**< TX suppressions on ATIM fifo */
	uint32  bcn_template_not_ready; /**< Template marked in use on send bcn ... */
	uint32  bcn_template_not_ready_done; /**< ...but "DMA done" interrupt rcvd */
	uint32  late_tbtt_dpc;  /**< TBTT DPC did not happen in time */

	/* per-rate receive stat counters */
	uint32  rx1mbps;    /**< packets rx at 1Mbps */
	uint32  rx2mbps;    /**< packets rx at 2Mbps */
	uint32  rx5mbps5;   /**< packets rx at 5.5Mbps */
	uint32  rx6mbps;    /**< packets rx at 6Mbps */
	uint32  rx9mbps;    /**< packets rx at 9Mbps */
	uint32  rx11mbps;   /**< packets rx at 11Mbps */
	uint32  rx12mbps;   /**< packets rx at 12Mbps */
	uint32  rx18mbps;   /**< packets rx at 18Mbps */
	uint32  rx24mbps;   /**< packets rx at 24Mbps */
	uint32  rx36mbps;   /**< packets rx at 36Mbps */
	uint32  rx48mbps;   /**< packets rx at 48Mbps */
	uint32  rx54mbps;   /**< packets rx at 54Mbps */
	uint32  rx108mbps;  /**< packets rx at 108mbps */
	uint32  rx162mbps;  /**< packets rx at 162mbps */
	uint32  rx216mbps;  /**< packets rx at 216 mbps */
	uint32  rx270mbps;  /**< packets rx at 270 mbps */
	uint32  rx324mbps;  /**< packets rx at 324 mbps */
	uint32  rx378mbps;  /**< packets rx at 378 mbps */
	uint32  rx432mbps;  /**< packets rx at 432 mbps */
	uint32  rx486mbps;  /**< packets rx at 486 mbps */
	uint32  rx540mbps;  /**< packets rx at 540 mbps */

	/* pkteng rx frame stats */
	uint32  pktengrxducast; /**< unicast frames rxed by the pkteng code */
	uint32  pktengrxdmcast; /**< multicast frames rxed by the pkteng code */

	uint32  rfdisable;  /**< count of radio disables */
	uint32  bphy_rxcrsglitch;   /**< PHY count of bphy glitches */
	uint32  bphy_badplcp;

	uint32  txmpdu_sgi; /**< count for sgi transmit */
	uint32  rxmpdu_sgi; /**< count for sgi received */
	uint32  txmpdu_stbc;    /**< count for stbc transmit */
	uint32  rxmpdu_stbc;    /**< count for stbc received */

	uint32	rxdrop20s;	/**< drop secondary cnt */
} wl_cnt_ver_6_t;

#define	WL_DELTA_STATS_T_VERSION	2	/**< current version of wl_delta_stats_t struct */

typedef struct {
	uint16 version;     /**< see definition of WL_DELTA_STATS_T_VERSION */
	uint16 length;      /**< length of entire structure */

	/* transmit stat counters */
	uint32 txframe;     /**< tx data frames */
	uint32 txbyte;      /**< tx data bytes */
	uint32 txretrans;   /**< tx mac retransmits */
	uint32 txfail;      /**< tx failures */

	/* receive stat counters */
	uint32 rxframe;     /**< rx data frames */
	uint32 rxbyte;      /**< rx data bytes */

	/* per-rate receive stat counters */
	uint32  rx1mbps;	/**< packets rx at 1Mbps */
	uint32  rx2mbps;	/**< packets rx at 2Mbps */
	uint32  rx5mbps5;	/**< packets rx at 5.5Mbps */
	uint32  rx6mbps;	/**< packets rx at 6Mbps */
	uint32  rx9mbps;	/**< packets rx at 9Mbps */
	uint32  rx11mbps;	/**< packets rx at 11Mbps */
	uint32  rx12mbps;	/**< packets rx at 12Mbps */
	uint32  rx18mbps;	/**< packets rx at 18Mbps */
	uint32  rx24mbps;	/**< packets rx at 24Mbps */
	uint32  rx36mbps;	/**< packets rx at 36Mbps */
	uint32  rx48mbps;	/**< packets rx at 48Mbps */
	uint32  rx54mbps;	/**< packets rx at 54Mbps */
	uint32  rx108mbps;	/**< packets rx at 108mbps */
	uint32  rx162mbps;	/**< packets rx at 162mbps */
	uint32  rx216mbps;	/**< packets rx at 216 mbps */
	uint32  rx270mbps;	/**< packets rx at 270 mbps */
	uint32  rx324mbps;	/**< packets rx at 324 mbps */
	uint32  rx378mbps;	/**< packets rx at 378 mbps */
	uint32  rx432mbps;	/**< packets rx at 432 mbps */
	uint32  rx486mbps;	/**< packets rx at 486 mbps */
	uint32  rx540mbps;	/**< packets rx at 540 mbps */

	/* phy stats */
	uint32 rxbadplcp;
	uint32 rxcrsglitch;
	uint32 bphy_rxcrsglitch;
	uint32 bphy_badplcp;

	uint32 slice_index; /**< Slice for which stats are reported */

} wl_delta_stats_t;

/* Partial statistics counter report */
#define WL_CNT_CTL_MGT_FRAMES	0

typedef struct {
	uint16	type;
	uint16	len;

	/* detailed control/management frames */
	uint32	txnull;
	uint32	rxnull;
	uint32	txqosnull;
	uint32	rxqosnull;
	uint32	txassocreq;
	uint32	rxassocreq;
	uint32	txreassocreq;
	uint32	rxreassocreq;
	uint32	txdisassoc;
	uint32	rxdisassoc;
	uint32	txassocrsp;
	uint32	rxassocrsp;
	uint32	txreassocrsp;
	uint32	rxreassocrsp;
	uint32	txauth;
	uint32	rxauth;
	uint32	txdeauth;
	uint32	rxdeauth;
	uint32	txprobereq;
	uint32	rxprobereq;
	uint32	txprobersp;
	uint32	rxprobersp;
	uint32	txaction;
	uint32	rxaction;
	uint32	txrts;
	uint32	rxrts;
	uint32	txcts;
	uint32	rxcts;
	uint32	txack;
	uint32	rxack;
	uint32	txbar;
	uint32	rxbar;
	uint32	txback;
	uint32	rxback;
	uint32	txpspoll;
	uint32	rxpspoll;
} wl_ctl_mgt_cnt_t;

typedef struct {
	uint16	version;	/**< see definition of WL_WME_CNT_VERSION */
	uint16	length;		/**< length of entire structure */

	wl_traffic_stats_t tx[AC_COUNT];	/**< Packets transmitted */
	wl_traffic_stats_t tx_failed[AC_COUNT];	/**< Packets dropped or failed to transmit */
	wl_traffic_stats_t rx[AC_COUNT];	/**< Packets received */
	wl_traffic_stats_t rx_failed[AC_COUNT];	/**< Packets failed to receive */

	wl_traffic_stats_t forward[AC_COUNT];	/**< Packets forwarded by AP */

	wl_traffic_stats_t tx_expired[AC_COUNT]; /**< packets dropped due to lifetime expiry */

} wl_wme_cnt_t;

struct wl_msglevel2 {
	uint32 low;
	uint32 high;
};

#define WL_ICMP_IPV6_CFG_VERSION         1
#define WL_ICMP_IPV6_CLEAR_ALL           (1 << 0)

typedef struct wl_icmp_ipv6_cfg {
	uint16 version;
	uint16 length;
	uint16 fixed_length;
	uint16 flags;
	uint32 num_ipv6;
	/* num_ipv6 to follow */
	struct ipv6_addr host_ipv6[];
} wl_icmp_ipv6_cfg_t;

#define WL_ICMP_CFG_IPV6_FIXED_LEN      OFFSETOF(wl_icmp_ipv6_cfg_t, host_ipv6)
#define WL_ICMP_CFG_IPV6_LEN(count)    (WL_ICMP_CFG_IPV6_FIXED_LEN + \
					((count) * sizeof(struct ipv6_addr)))

typedef struct wl_mkeep_alive_pkt {
	uint16	version; /* Version for mkeep_alive */
	uint16	length; /* length of fixed parameters in the structure */
	uint32	period_msec; /* high bit on means immediate send */
	uint16	len_bytes;
	uint8	keep_alive_id; /* 0 - 3 for N = 4 */
	uint8	data[1];
} wl_mkeep_alive_pkt_t;

#define WL_MKEEP_ALIVE_VERSION		1
#define WL_MKEEP_ALIVE_FIXED_LEN	OFFSETOF(wl_mkeep_alive_pkt_t, data)
#define WL_MKEEP_ALIVE_PRECISION	500
#define WL_MKEEP_ALIVE_PERIOD_MASK  0x7FFFFFFF
#define WL_MKEEP_ALIVE_IMMEDIATE    0x80000000

/** TCP Keep-Alive conn struct */
typedef struct wl_mtcpkeep_alive_conn_pkt {
	struct ether_addr saddr;		/**< src mac address */
	struct ether_addr daddr;		/**< dst mac address */
	struct ipv4_addr sipaddr;		/**< source IP addr */
	struct ipv4_addr dipaddr;		/**< dest IP addr */
	uint16 sport;				/**< src port */
	uint16 dport;				/**< dest port */
	uint32 seq;				/**< seq number */
	uint32 ack;				/**< ACK number */
	uint16 tcpwin;				/**< TCP window */
	uint16 PAD;
} wl_mtcpkeep_alive_conn_pkt_t;

/** TCP Keep-Alive interval struct */
typedef struct wl_mtcpkeep_alive_timers_pkt {
	uint16 interval;		/**< interval timer */
	uint16 retry_interval;		/**< retry_interval timer */
	uint16 retry_count;		/**< retry_count */
} wl_mtcpkeep_alive_timers_pkt_t;

typedef struct wake_info {
	uint32 wake_reason;
	uint32 wake_info_len;		/**< size of packet */
	uint8  packet[];
} wake_info_t;

typedef struct wake_pkt {
	uint32 wake_pkt_len;		/**< size of packet */
	uint8  packet[];
} wake_pkt_t;

#define WL_MTCPKEEP_ALIVE_VERSION		1

/* #ifdef WLBA */

#define WLC_BA_CNT_VERSION  1   /**< current version of wlc_ba_cnt_t */

/** block ack related stats */
typedef struct wlc_ba_cnt {
	uint16  version;    /**< WLC_BA_CNT_VERSION */
	uint16  length;     /**< length of entire structure */

	/* transmit stat counters */
	uint32 txpdu;       /**< pdus sent */
	uint32 txsdu;       /**< sdus sent */
	uint32 txfc;        /**< tx side flow controlled packets */
	uint32 txfci;       /**< tx side flow control initiated */
	uint32 txretrans;   /**< retransmitted pdus */
	uint32 txbatimer;   /**< ba resend due to timer */
	uint32 txdrop;      /**< dropped packets */
	uint32 txaddbareq;  /**< addba req sent */
	uint32 txaddbaresp; /**< addba resp sent */
	uint32 txdelba;     /**< delba sent */
	uint32 txba;        /**< ba sent */
	uint32 txbar;       /**< bar sent */
	uint32 txpad[4];    /**< future */

	/* receive side counters */
	uint32 rxpdu;       /**< pdus recd */
	uint32 rxqed;       /**< pdus buffered before sending up */
	uint32 rxdup;       /**< duplicate pdus */
	uint32 rxnobuf;     /**< pdus discarded due to no buf */
	uint32 rxaddbareq;  /**< addba req recd */
	uint32 rxaddbaresp; /**< addba resp recd */
	uint32 rxdelba;     /**< delba recd */
	uint32 rxba;        /**< ba recd */
	uint32 rxbar;       /**< bar recd */
	uint32 rxinvba;     /**< invalid ba recd */
	uint32 rxbaholes;   /**< ba recd with holes */
	uint32 rxunexp;     /**< unexpected packets */
	uint32 rxpad[4];    /**< future */
} wlc_ba_cnt_t;
/* #endif  WLBA */

/** structure for per-tid ampdu control */
struct ampdu_tid_control {
	uint8 tid;			/* tid */
	uint8 enable;			/* enable/disable */
};

/** structure for per-tid amsdu control */
struct amsdu_tid_control {
	uint8 tid;                  /* tid */
	uint8 enable;               /* enable/disable */
};

/** struct for ampdu tx/rx aggregation control */
struct ampdu_aggr {
	int8 aggr_override;	/**< aggr overrided by dongle. Not to be set by host. */
	uint16 conf_TID_bmap;	/**< bitmap of TIDs to configure */
	uint16 enab_TID_bmap;	/**< enable/disable per TID */
};

/** structure for identifying ea/tid for sending addba/delba */
struct ampdu_ea_tid {
	struct ether_addr ea;		/**< Station address */
	uint8 tid;			/**< tid */
	uint8 initiator;	/**< 0 is recipient, 1 is originator */
};

/** structure for identifying retry/tid for retry_limit_tid/rr_retry_limit_tid */
struct ampdu_retry_tid {
	uint8 tid;	/**< tid */
	uint8 retry;	/**< retry value */
};

#define BDD_FNAME_LEN       32  /**< Max length of friendly name */
typedef struct bdd_fname {
	uint8 len;          /**< length of friendly name */
	uchar name[BDD_FNAME_LEN];  /**< friendly name */
} bdd_fname_t;

/* structure for addts arguments */
/** For ioctls that take a list of TSPEC */
struct tslist {
	int32 count;			/**< number of tspecs */
	struct tsinfo_arg tsinfo[];	/**< variable length array of tsinfo */
};

/* WLTDLS */
/**structure for tdls iovars */
typedef struct tdls_iovar {
	struct ether_addr ea;		/**< Station address */
	uint8 mode;			/**< mode: depends on iovar */
	uint8 PAD;
	chanspec_t chanspec;
	uint16 PAD;
	uint32 pad;			/**< future */
} tdls_iovar_t;

#define TDLS_WFD_IE_SIZE		512
/**structure for tdls wfd ie */
typedef struct tdls_wfd_ie_iovar {
	struct ether_addr ea;		/**< Station address */
	uint8 mode;
	uint8 PAD;
	uint16 length;
	uint8 data[TDLS_WFD_IE_SIZE];
} tdls_wfd_ie_iovar_t;
/* #endif WLTDLS */

/** structure for addts/delts arguments */
typedef struct tspec_arg {
	uint16 version;			/**< see definition of TSPEC_ARG_VERSION */
	uint16 length;			/**< length of entire structure */
	uint32 flag;			/**< bit field */
	/* TSPEC Arguments */
	struct tsinfo_arg tsinfo;	/**< TS Info bit field */
	uint8  PAD;
	uint16 nom_msdu_size;		/**< (Nominal or fixed) MSDU Size (bytes) */
	uint16 max_msdu_size;		/**< Maximum MSDU Size (bytes) */
	uint32 min_srv_interval;		/**< Minimum Service Interval (us) */
	uint32 max_srv_interval;		/**< Maximum Service Interval (us) */
	uint32 inactivity_interval;	/**< Inactivity Interval (us) */
	uint32 suspension_interval;	/**< Suspension Interval (us) */
	uint32 srv_start_time;		/**< Service Start Time (us) */
	uint32 min_data_rate;		/**< Minimum Data Rate (bps) */
	uint32 mean_data_rate;		/**< Mean Data Rate (bps) */
	uint32 peak_data_rate;		/**< Peak Data Rate (bps) */
	uint32 max_burst_size;		/**< Maximum Burst Size (bytes) */
	uint32 delay_bound;		/**< Delay Bound (us) */
	uint32 min_phy_rate;		/**< Minimum PHY Rate (bps) */
	uint16 surplus_bw;		/**< Surplus Bandwidth Allowance (range 1.0 to 8.0) */
	uint16 medium_time;		/**< Medium Time (32 us/s periods) */
	uint8 dialog_token;		/**< dialog token */
	uint8  PAD[3];
} tspec_arg_t;

/** tspec arg for desired station */
typedef	struct tspec_per_sta_arg {
	struct ether_addr ea;
	uint8  PAD[2];
	struct tspec_arg ts;
} tspec_per_sta_arg_t;

/** structure for max bandwidth for each access category */
typedef	struct wme_max_bandwidth {
	uint32	ac[AC_COUNT];	/**< max bandwidth for each access category */
} wme_max_bandwidth_t;

#define WL_WME_MBW_PARAMS_IO_BYTES (sizeof(wme_max_bandwidth_t))

/* current version of wl_tspec_arg_t struct */
#define	TSPEC_ARG_VERSION		2	/**< current version of wl_tspec_arg_t struct */
#define TSPEC_ARG_LENGTH		55	/**< argument length from tsinfo to medium_time */
#define TSPEC_DEFAULT_DIALOG_TOKEN	42	/**< default dialog token */
#define TSPEC_DEFAULT_SBW_FACTOR	0x3000	/**< default surplus bw */

#define WL_WOWL_KEEPALIVE_MAX_PACKET_SIZE  80
#define WLC_WOWL_MAX_KEEPALIVE	2

/** Packet lifetime configuration per ac */
typedef struct wl_lifetime {
	uint32 ac;	        /**< access class */
	uint32 lifetime;    /**< Packet lifetime value in ms */
} wl_lifetime_t;

/** MUTX mpdu size admit threshold per bw */
typedef struct wl_mutx_mpdusz_admit_thresh {
	uint32 bw;	        /**< BW */
	uint32 mpdusz;    /**< mpdu size threshold in Bytes */
} wl_mutx_mpdusz_admit_thresh_t;

#ifndef LINUX_POSTMOGRIFY_REMOVAL
/** Management time configuration */
typedef struct wl_lifetime_mg {
	uint32 mgmt_bitmap;	/**< Mgmt subtype */
	uint32 lifetime;    /**< Packet lifetime value in us */
} wl_lifetime_mg_t;

/* MAC Sample Capture related */
#define	WL_MACCAPTR_DEFSTART_PTR	0xA00
#define	WL_MACCAPTR_DEFSTOP_PTR		0xA3F
#define	WL_MACCAPTR_DEFSZ		0x3F

#define WL_MACCAPTR_DEF_MASK		0xFFFFFFFF

typedef enum {
	WL_MACCAPT_TRIG		= 0,
	WL_MACCAPT_STORE	= 1,
	WL_MACCAPT_TRANS	= 2,
	WL_MACCAPT_MATCH	= 3,
	WL_MACCAPT_STOPMODE	= 4, /* stop mode */
	WL_MACCAPT_EXPERT	= 5, /* expert setting */
	WL_MACCAPT_STOPTRIG	= 6, /* stoptrigger mode */
	WL_MACCAPT_USETXE	= 7, /* use TXE (template) for mac capture */
	WL_MACCAPT_USEPSM	= 8, /* use PSM (GPIO-mon) for mac capture */
	WL_MACCAPT_LOGMODE	= 9, /* Uses 2 bits! 0/1/3: */
		                     /* unconditional/pc_valid/pc_valid && log_en */
	WL_MACCAPT_FILTER	= 11, /* Uses 1 bits! 0/1: */
	WL_MACCAPT_LOGSEL	= 12, /* Uses 3 bits; bitmap for psmx, psmr1, psmr0: */
	WL_MACCAPT_PSMSEL	= 15, /* select which psm controls GPIO-mon */
} maccaptr_optn;

#define WL_MACCAP_SEL_PSMR0	0x1
#define WL_MACCAP_SEL_PSMR1	0x2
#define WL_MACCAP_SEL_PSMX	0x4
#define WL_MACCAP_SEL_MAX	(WL_MACCAP_SEL_PSMR0 |WL_MACCAP_SEL_PSMR1 | WL_MACCAP_SEL_PSMX)

#define WL_MACCAP_SEL_PSMR0_SHIFT	(SC_ENABLE_SHIFT-0)
#define WL_MACCAP_SEL_PSMR1_SHIFT	(SC_ENABLE_SHIFT-1)
#define WL_MACCAP_SEL_PSMX_SHIFT	(SC_ENABLE_SHIFT-2)

typedef enum {
	WL_MACCAPT_STRT	= 1,
	WL_MACCAPT_STOP	= 2,
	WL_MACCAPT_RST	= 3
} maccaptr_cmd_t;

/* MAC Sample Capture Set-up Paramters */
typedef struct wl_maccapture_params {
	uint8	gpio_sel;
	uint8	la_mode;	/* TRUE: GPIO Out Enabled */
	uint16	dbg_flags;	/* dbg flags */
	uint32	start_ptr;	/* Start address to store */
	uint32	stop_ptr;	/* Stop address to store */
	uint16	optn_bmp;	/* Options */
	uint8	PAD[1];
	uint8	psmsel_bmp;	/* psmr0/1/x enable bmp */
	uint32	tr_mask;	/* Trigger Mask */
	uint32	tr_val;		/* Trigger Value */
	uint32	s_mask;		/* Store Mode Mask */
	uint32	x_mask;		/* Trans. Mode Mask */
	uint32	m_mask;		/* Match Mode Mask */
	uint32	m_val;		/* Match Value */
	maccaptr_cmd_t cmd;	/* Start / Stop */
	int32	smp_sz;		/* Sample capture size  */
	uint16	smpl_ctrl;	/* Sample capture size  */
	uint32	dbg_blk;	/* dbg block */
} wl_maccapture_params_t;

#define WL_PKTENG_CMD_MAX_BUF		200
#define WL_PKTENG_CMD_MAX_ARGS		(WL_PKTENG_CMD_MAX_BUF / 4)
#define WL_PKTENG_CMD_KEYSTR_MAXLEN	(WL_PKTENG_CMD_MAX_BUF / 2)
#define WL_PKTENG_CMD_VALSTR_MAXLEN	WL_PKTENG_CMD_KEYSTR_MAXLEN

#define WL_PKTENG_CMD_STRT_DL_CMD	1
#define WL_PKTENG_CMD_STRT_UL_CMD	2
#define WL_PKTENG_CMD_STRT_ULTX_CMD	3
#define PKTENGCMD_FLAGS_VERBOSE		1

/* Definitions for pkteng opcode */
typedef enum {
	/* get cmds */
	PKTENGCMD_OPCODE_PUB_ARGKEY_GET		= 0,	/* general key_val get */
	PKTENGCMD_OPCODE_PUB_ARGINT_GET		= 1,	/* general positional get */
	/* key-val based cmds */
	PKTENGCMD_OPCODE_PUB_ARGKEY_GNRAL	= 2,	/* general key-val */
	PKTENGCMD_OPCODE_PUB_ARGKEY_CFG		= 3,	/* key-val to config usr */
	/* configure cmds */
	PKTENGCMD_OPCODE_PUB_ARGINT_STALIST	= 4,
	PKTENGCMD_OPCODE_PUB_ARGINT_DELSTA	= 5,
	PKTENGCMD_OPCODE_PUB_ARGINT_STACFG	= 6,
	PKTENGCMD_OPCODE_PUB_ARGINT_STACFG1	= 7,
	PKTENGCMD_OPCODE_PUB_ARGINT_TXQCFG	= 8,
	PKTENGCMD_OPCODE_PUB_ARGINT_SIGA	= 9,
	PKTENGCMD_OPCODE_PUB_ARGINT_PCTL	= 10,
	/* control cmds */
	PKTENGCMD_OPCODE_PUB_ARGINT_RESET	= 11,	/* set default value */
	PKTENGCMD_OPCODE_PUB_ARGINT_START	= 12,	/* start pkteng */
	PKTENGCMD_OPCODE_PUB_ARGINT_STOP	= 13,	/* stop pkteng */
	PKTENGCMD_OPCODE_PUB_ARGINT_PAUSE	= 14,	/* pause pkteng */
	PKTENGCMD_OPCODE_PUB_ARGINT_COMMIT	= 15,
	/* opaque buffer based cmds */
	PKTENGCMD_OPCODE_PUB_ARGBUF_GET		= 16,	/* buffer based get */
	PKTENGCMD_OPCODE_PUB_ARGBUF_SET		= 17,	/* buffer based set */
	PKTENGCMD_OPCODE_PUB_END		= 18,	/* end of public opcode */
} wlc_test_pecmd_public_opcode_t;

/* MAC pkteng cmd parameters */
typedef struct wl_pkteng_cmd_params {
	int16	opcode;		/* opaque opcode */
	uint16	flags;
	uint16	ver;
	uint16	len;
	union {
		struct {
			uint8	argc;		/* number of args */
			uint8	pad[3];
			int32	argv[WL_PKTENG_CMD_MAX_ARGS];	/* opaque argv */
		} argint;
		struct {
			int8	val[2];
			uint8	keystr_len;
			uint8	valstr_len;
			char	keystr[WL_PKTENG_CMD_KEYSTR_MAXLEN];
			char	valstr[WL_PKTENG_CMD_VALSTR_MAXLEN];
		} argkeyval;
		struct {
			uint16	sz;		/* buffer size */
			uint8	pad[2];
			char	buf[WL_PKTENG_CMD_MAX_BUF];
		} argbuffer;
	} u;
} wl_pkteng_cmd_params_t;

#endif /* LINUX_POSTMOGRIFY_REMOVAL */

/** Channel Switch Announcement param */
typedef struct wl_chan_switch {
	uint8 mode;		/**< value 0 or 1 */
	uint8 count;		/**< count # of beacons before switching */
	chanspec_t chspec;	/**< chanspec */
	uint8 reg;		/**< regulatory class */
	uint8 frame_type;		/**< csa frame type, unicast or broadcast */
} wl_chan_switch_t;

enum {
	PFN_LIST_ORDER,
	PFN_RSSI
};

enum {
	DISABLE,
	ENABLE
};

enum {
	OFF_ADAPT,
	SMART_ADAPT,
	STRICT_ADAPT,
	SLOW_ADAPT
};

#define SORT_CRITERIA_BIT		0
#define AUTO_NET_SWITCH_BIT		1
#define ENABLE_BKGRD_SCAN_BIT		2
#define IMMEDIATE_SCAN_BIT		3
#define	AUTO_CONNECT_BIT		4
#define	ENABLE_BD_SCAN_BIT		5
#define ENABLE_ADAPTSCAN_BIT		6
#define IMMEDIATE_EVENT_BIT		8
#define SUPPRESS_SSID_BIT		9
#define ENABLE_NET_OFFLOAD_BIT		10
/** report found/lost events for SSID and BSSID networks seperately */
#define REPORT_SEPERATELY_BIT		11

#define SORT_CRITERIA_MASK	0x0001
#define AUTO_NET_SWITCH_MASK	0x0002
#define ENABLE_BKGRD_SCAN_MASK	0x0004
#define IMMEDIATE_SCAN_MASK	0x0008
#define AUTO_CONNECT_MASK	0x0010

#define ENABLE_BD_SCAN_MASK	0x0020
#define ENABLE_ADAPTSCAN_MASK	0x00c0
#define IMMEDIATE_EVENT_MASK	0x0100
#define SUPPRESS_SSID_MASK	0x0200
#define ENABLE_NET_OFFLOAD_MASK	0x0400
/** report found/lost events for SSID and BSSID networks seperately */
#define REPORT_SEPERATELY_MASK	0x0800

#define PFN_VERSION			2

#define PFN_COMPLETE			1
#define PFN_INCOMPLETE			0

#define DEFAULT_BESTN			2
#define DEFAULT_MSCAN			0
#define DEFAULT_REPEAT			10
#define DEFAULT_EXP			2

#define PFN_PARTIAL_SCAN_BIT		0
#define PFN_PARTIAL_SCAN_MASK		1

#ifndef LINUX_POSTMOGRIFY_REMOVAL
#define PFN_SWC_RSSI_WINDOW_MAX   8
#define PFN_SWC_MAX_NUM_APS       16
#define PFN_HOTLIST_MAX_NUM_APS   64

#endif /* LINUX_POSTMOGRIFY_REMOVAL */

#define MAX_EPNO_HIDDEN_SSID    8
#define MAX_WHITELIST_SSID      2

/* Version 1 and 2 for various scan results structures defined below */
#define PFN_SCANRESULTS_VERSION_V1	1
#define PFN_SCANRESULTS_VERSION_V2	2

/** PFN network info structure */
typedef struct wl_pfn_subnet_info_v1 {
	struct ether_addr BSSID;
	uint8	channel; /**< channel number only */
	uint8	SSID_len;
	uint8	SSID[32];
} wl_pfn_subnet_info_v1_t;

typedef struct wl_pfn_subnet_info_v2 {
	struct ether_addr BSSID;
	uint8   channel; /**< channel number only */
	uint8   SSID_len;
	union {
		uint8   SSID[32];
		uint16 index;
	} u;
} wl_pfn_subnet_info_v2_t;

typedef struct wl_pfn_net_info_v1 {
	wl_pfn_subnet_info_v1_t pfnsubnet;
	int16	RSSI; /**< receive signal strength (in dBm) */
	uint16	timestamp; /**< age in seconds */
} wl_pfn_net_info_v1_t;

typedef struct wl_pfn_net_info_v2 {
	wl_pfn_subnet_info_v2_t pfnsubnet;
	int16   RSSI; /**< receive signal strength (in dBm) */
	uint16  timestamp; /**< age in seconds */
} wl_pfn_net_info_v2_t;

/* Version 1 and 2 for various lbest scan results structures below */
#define PFN_LBEST_SCAN_RESULT_VERSION_V1 1
#define PFN_LBEST_SCAN_RESULT_VERSION_V2 2

#define MAX_CHBKT_PER_RESULT		4

typedef struct wl_pfn_lnet_info_v1 {
	wl_pfn_subnet_info_v1_t pfnsubnet; /**< BSSID + channel + SSID len + SSID */
	uint16	flags; /**< partial scan, etc */
	int16	RSSI; /**< receive signal strength (in dBm) */
	uint32	timestamp; /**< age in miliseconds */
	uint16	rtt0; /**< estimated distance to this AP in centimeters */
	uint16	rtt1; /**< standard deviation of the distance to this AP in centimeters */
} wl_pfn_lnet_info_v1_t;

typedef struct wl_pfn_lnet_info_v2 {
	wl_pfn_subnet_info_v2_t pfnsubnet; /**< BSSID + channel + SSID len + SSID */
	uint16  flags; /**< partial scan, etc */
	int16   RSSI; /**< receive signal strength (in dBm) */
	uint32  timestamp; /**< age in miliseconds */
	uint16  rtt0; /**< estimated distance to this AP in centimeters */
	uint16  rtt1; /**< standard deviation of the distance to this AP in centimeters */
} wl_pfn_lnet_info_v2_t;

typedef struct wl_pfn_lscanresults_v1 {
	uint32 version;
	uint32 status;
	uint32 count;
	wl_pfn_lnet_info_v1_t netinfo[1];
} wl_pfn_lscanresults_v1_t;

typedef struct wl_pfn_lscanresults_v2 {
	uint32 version;
	uint16 status;
	uint16 count;
	uint32 scan_ch_buckets[MAX_CHBKT_PER_RESULT];
	wl_pfn_lnet_info_v2_t netinfo[1];
} wl_pfn_lscanresults_v2_t;

/**this is used to report on 1-* pfn scan results */
typedef struct wl_pfn_scanresults_v1 {
	uint32 version;
	uint32 status;
	uint32 count;
	wl_pfn_net_info_v1_t netinfo[1];
} wl_pfn_scanresults_v1_t;

typedef struct wl_pfn_scanresults_v2 {
	uint32 version;
	uint32 status;
	uint32 count;
	uint32 scan_ch_bucket;
	wl_pfn_net_info_v2_t netinfo[1];
} wl_pfn_scanresults_v2_t;

#ifndef LINUX_POSTMOGRIFY_REMOVAL
typedef struct wl_pfn_significant_net {
	uint16 flags;
	uint16 channel;
	struct ether_addr BSSID;
	int8 rssi[PFN_SWC_RSSI_WINDOW_MAX];
} wl_pfn_significant_net_t;

#define PFN_SWC_SCANRESULT_VERSION     1

typedef struct wl_pfn_swc_results {
	uint32 version;
	uint32 pkt_count;   /**< No. of results in current frame */
	uint32 total_count; /**< Total expected results */
	wl_pfn_significant_net_t list[];
} wl_pfn_swc_results_t;
#endif /* LINUX_POSTMOGRIFY_REMOVAL */
typedef struct wl_pfn_net_info_bssid {
	struct ether_addr BSSID;
	uint8 channel;	/**< channel number only */
	int8  RSSI;	/**< receive signal strength (in dBm) */
	uint16 flags;	/**< (e.g. partial scan, off channel) */
	uint16 timestamp; /**< age in seconds */
} wl_pfn_net_info_bssid_t;

typedef struct wl_pfn_scanhist_bssid {
	uint32 version;
	uint32 status;
	uint32 count;
	wl_pfn_net_info_bssid_t netinfo[1];
} wl_pfn_scanhist_bssid_t;

/* Version 1 and 2 for various single scan result */
#define PFN_SCANRESULT_VERSION_V1	1
#define PFN_SCANRESULT_VERSION_V2	2

/* used to report exactly one scan result */
/* plus reports detailed scan info in bss_info */
typedef struct wl_pfn_scanresult_v1 {
	uint32 version;
	uint32 status;
	uint32 count;
	wl_pfn_net_info_v1_t netinfo;
	wl_bss_info_v109_t bss_info;
} wl_pfn_scanresult_v1_t;

typedef struct wl_pfn_scanresult_v2 {
	uint32 version;
	uint32 status;
	uint32 count;
	wl_pfn_net_info_v2_t netinfo;
	wl_bss_info_v109_t bss_info;
} wl_pfn_scanresult_v2_t;

typedef struct wl_pfn_scanresult_v2_1 {
	uint32 version;
	uint32 status;
	uint32 count;
	wl_pfn_net_info_v2_t netinfo;
	uint8 bss_info[];	/* var length wl_bss_info_X structures */
} wl_pfn_scanresult_v2_1_t;

/**PFN data structure */
typedef struct wl_pfn_param {
	int32 version;			/**< PNO parameters version */
	int32 scan_freq;		/**< Scan frequency */
	int32 lost_network_timeout;	/**< Timeout in sec. to declare
								* discovered network as lost
								*/
	int16 flags;			/**< Bit field to control features
							* of PFN such as sort criteria auto
							* enable switch and background scan
							*/
	int16 rssi_margin;		/**< Margin to avoid jitter for choosing a
							* PFN based on RSSI sort criteria
							*/
	uint8 bestn; /**< number of best networks in each scan */
	uint8 mscan; /**< number of scans recorded */
	uint8 repeat; /**< Minimum number of scan intervals
				     *before scan frequency changes in adaptive scan
				     */
	uint8 exp; /**< Exponent of 2 for maximum scan interval */
	int32 slow_freq; /**< slow scan period */
} wl_pfn_param_t;

typedef struct wl_pfn_bssid {
	struct ether_addr  macaddr;
	/* Bit4: suppress_lost, Bit3: suppress_found */
	uint16             flags;
} wl_pfn_bssid_t;
#ifndef LINUX_POSTMOGRIFY_REMOVAL
typedef struct wl_pfn_significant_bssid {
	struct ether_addr	macaddr;
	int8    rssi_low_threshold;
	int8    rssi_high_threshold;
} wl_pfn_significant_bssid_t;
#endif /* LINUX_POSTMOGRIFY_REMOVAL */
#define WL_PFN_SUPPRESSFOUND_MASK	0x08
#define WL_PFN_SUPPRESSLOST_MASK	0x10
#define WL_PFN_SSID_IMPRECISE_MATCH	0x80
#define WL_PFN_SSID_SAME_NETWORK	0x10000
#define WL_PFN_SUPPRESS_AGING_MASK	0x20000
#define WL_PFN_FLUSH_ALL_SSIDS		0x40000

#define WL_PFN_IOVAR_FLAG_MASK		0xFFFF00FF
#define WL_PFN_RSSI_MASK		0xff00
#define WL_PFN_RSSI_SHIFT		8

typedef struct wl_pfn_cfg {
	uint32	reporttype;
	int32	channel_num;
	uint16	channel_list[WL_NUMCHANNELS];
	uint32	flags;
} wl_pfn_cfg_t;

#define WL_PFN_SSID_CFG_VERSION		1
#define WL_PFN_SSID_CFG_CLEAR		0x1

typedef struct wl_pfn_ssid_params {
	int8 min5G_rssi;           /* minimum 5GHz RSSI for a BSSID to be considered      */
	int8 min2G_rssi;           /* minimum 2.4GHz RSSI for a BSSID to be considered   */
	int16 init_score_max;     /* The maximum score that a network can have before bonuses  */

	int16 cur_bssid_bonus;    /* Add to current bssid                                      */
	int16 same_ssid_bonus;    /* score bonus for all networks with the same network flag   */
	int16 secure_bonus;       /* score bonus for networks that are not open          */
	int16 band_5g_bonus;
} wl_pfn_ssid_params_t;

typedef struct wl_ssid_ext_params {
	int8 min5G_rssi;	/* minimum 5GHz RSSI for a BSSID to be considered      */
	int8 min2G_rssi;	/* minimum 2.4GHz RSSI for a BSSID to be considered   */
	int16 init_score_max;	/* The maximum score that a network can have before bonuses  */
	int16 cur_bssid_bonus;	/* Add to current bssid                                      */
	int16 same_ssid_bonus;	/* score bonus for all networks with the same network flag   */
	int16 secure_bonus;	/* score bonus for networks that are not open                */
	int16 band_5g_bonus;
} wl_ssid_ext_params_t;

typedef struct wl_pfn_ssid_cfg {
	uint16 version;
	uint16 flags;
	wl_ssid_ext_params_t params;
} wl_pfn_ssid_cfg_t;

#ifndef LINUX_POSTMOGRIFY_REMOVAL
#define CH_BUCKET_REPORT_NONE                   0
#define CH_BUCKET_REPORT_SCAN_COMPLETE_ONLY     1
#define CH_BUCKET_REPORT_FULL_RESULT            2
#define CH_BUCKET_REPORT_SCAN_COMPLETE    (CH_BUCKET_REPORT_SCAN_COMPLETE_ONLY | \
								CH_BUCKET_REPORT_FULL_RESULT)
#define CH_BUCKET_REPORT_REGULAR            0
#define CH_BUCKET_GSCAN                     4

typedef struct wl_pfn_gscan_ch_bucket_cfg {
	uint8 bucket_end_index;
	uint8 bucket_freq_multiple;
	uint8 flag;
	uint8 reserved;
	uint16 repeat;
	uint16 max_freq_multiple;
} wl_pfn_gscan_ch_bucket_cfg_t;

typedef struct wl_pfn_capabilities {
	uint16 max_mscan;
	uint16 max_bestn;
	uint16 max_swc_bssid;
	uint16 max_hotlist_bssid;
} wl_pfn_capabilities_t;

#define GSCAN_SEND_ALL_RESULTS_MASK          (1 << 0)
#define GSCAN_ALL_BUCKETS_IN_FIRST_SCAN_MASK (1 << 3)
#define GSCAN_CFG_FLAGS_ONLY_MASK            (1 << 7)
#define WL_GSCAN_CFG_VERSION                     1
typedef struct wl_pfn_gscan_cfg {
	uint16 version;
	/**
	 * BIT0 1 = send probes/beacons to HOST
	 * BIT1 Reserved
	 * BIT2 Reserved
	 * Add any future flags here
	 * BIT7 1 = no other useful cfg sent
	 */
	uint8  flags;
	/** Buffer filled threshold in % to generate an event */
	uint8   buffer_threshold;
	/**
	 * No. of BSSIDs with "change" to generate an evt
	 * change - crosses rssi threshold/lost
	 */
	uint8   swc_nbssid_threshold;
	/* Max=8 (for now) Size of rssi cache buffer */
	uint8  swc_rssi_window_size;
	uint8  count_of_channel_buckets;
	uint8  retry_threshold;
	uint16  lost_ap_window;
	wl_pfn_gscan_ch_bucket_cfg_t channel_bucket[1];
} wl_pfn_gscan_cfg_t;
#endif /* LINUX_POSTMOGRIFY_REMOVAL */

#define WL_PFN_REPORT_ALLNET    0
#define WL_PFN_REPORT_SSIDNET   1
#define WL_PFN_REPORT_BSSIDNET  2

#define WL_PFN_CFG_FLAGS_PROHIBITED	0x00000001	/* Accept and use prohibited channels */
#define WL_PFN_CFG_FLAGS_RESERVED	0xfffffffe	/**< Remaining reserved for future use */

typedef struct wl_pfn {
	wlc_ssid_t		ssid;			/**< ssid name and its length */
	int32			flags;			/**< bit2: hidden */
	int32			infra;			/**< BSS Vs IBSS */
	int32			auth;			/**< Open Vs Closed */
	int32			wpa_auth;		/**< WPA type */
	int32			wsec;			/**< wsec value */
} wl_pfn_t;

typedef struct wl_pfn_list {
	uint32		version;
	uint32		enabled;
	uint32		count;
	wl_pfn_t	pfn[1];
} wl_pfn_list_t;

#define PFN_SSID_EXT_VERSION   1

typedef struct wl_pfn_ext {
	uint8 flags;
	int8 rssi_thresh; /* RSSI threshold, track only if RSSI > threshold */
	uint16 wpa_auth; /* Match the wpa auth type defined in wlioctl_defs.h */
	uint8 ssid[DOT11_MAX_SSID_LEN];
	uint8 ssid_len;
	uint8 pad;
} wl_pfn_ext_t;
typedef struct wl_pfn_ext_list {
	uint16 version;
	uint16 count;
	wl_pfn_ext_t pfn_ext[1];
} wl_pfn_ext_list_t;

#define WL_PFN_SSID_EXT_FOUND   0x1
#define WL_PFN_SSID_EXT_LOST    0x2
typedef struct wl_pfn_result_ssid {
	uint8 flags;
	int8 rssi;
	/* channel number */
	uint16 channel;
	/* Assume idx in order of cfg */
	uint32 index;
} wl_pfn_result_ssid_crc32_t;

typedef struct wl_pfn_ssid_ext_result {
	uint16 version;
	uint16 count;
	wl_pfn_result_ssid_crc32_t net[1];
} wl_pfn_ssid_ext_result_t;

#define PFN_EXT_AUTH_CODE_OPEN   1 /* open */
#define PFN_EXT_AUTH_CODE_PSK   2 /* WPA_PSK or WPA2PSK */
#define PFN_EXT_AUTH_CODE_EAPOL 4 /* any EAPOL  */

#define WL_PFN_HIDDEN_BIT		2
#define WL_PFN_HIDDEN_MASK		0x4

#ifndef BESTN_MAX
#define BESTN_MAX			10
#endif // endif

#ifndef MSCAN_MAX
#define MSCAN_MAX			90
#endif // endif

/* Dynamic scan configuration for motion profiles */

#define WL_PFN_MPF_VERSION 1

/* Valid group IDs, may be expanded in the future */
#define WL_PFN_MPF_GROUP_SSID 0
#define WL_PFN_MPF_GROUP_BSSID 1
#define WL_PFN_MPF_MAX_GROUPS 2

/* Max number of MPF states supported in this time */
#define WL_PFN_MPF_STATES_MAX 4

/* Flags for the mpf-specific stuff */
#define WL_PFN_MPF_ADAPT_ON_BIT		0
#define WL_PFN_MPF_ADAPTSCAN_BIT	1

#define WL_PFN_MPF_ADAPT_ON_MASK	0x0001
#define WL_PFN_MPF_ADAPTSCAN_MASK 	0x0006

/* Per-state timing values */
typedef struct wl_pfn_mpf_state_params {
	int32  scan_freq;	/* Scan frequency (secs) */
	int32  lost_network_timeout; /* Timeout to declare net lost (secs) */
	int16  flags;		/* Space for flags: ADAPT etc */
	uint8  exp;		/* Exponent of 2 for max interval for SMART/STRICT_ADAPT */
	uint8  repeat;		/* Number of scans before changing adaptation level */
	int32  slow_freq;	/* Slow scan period for SLOW_ADAPT */
} wl_pfn_mpf_state_params_t;

typedef struct wl_pfn_mpf_param {
	uint16 version;		/* Structure version */
	uint16 groupid;		/* Group ID: 0 (SSID), 1 (BSSID), other: reserved */
	wl_pfn_mpf_state_params_t state[WL_PFN_MPF_STATES_MAX];
} wl_pfn_mpf_param_t;

/* Structure for setting pfn_override iovar */
typedef struct wl_pfn_override_param {
	uint16 version;         /* Structure version */
	uint16 start_offset;    /* Seconds from now to apply new params */
	uint16 duration;        /* Seconds to keep new params applied */
	uint16 reserved;
	wl_pfn_mpf_state_params_t override;
} wl_pfn_override_param_t;
#define WL_PFN_OVERRIDE_VERSION	1

/*
 * Definitions for base MPF configuration
 */

#define WL_MPF_VERSION 1
#define WL_MPF_MAX_BITS 3
#define WL_MPF_MAX_STATES (1 << WL_MPF_MAX_BITS)

#define WL_MPF_STATE_NAME_MAX 12

typedef struct wl_mpf_val {
	uint16 val;		/* Value of GPIO bits */
	uint16 state;		/* State identifier */
	char name[WL_MPF_STATE_NAME_MAX]; /* Optional name */
} wl_mpf_val_t;

typedef struct wl_mpf_map {
	uint16 version;
	uint16 type;
	uint16 mask;		/* Which GPIO bits to use */
	uint8  count;		/* Count of state/value mappings */
	uint8  PAD;
	wl_mpf_val_t vals[WL_MPF_MAX_STATES];
} wl_mpf_map_t;

#define WL_MPF_STATE_AUTO (0xFFFF) /* (uint16)-1) */

typedef struct wl_mpf_state {
	uint16 version;
	uint16 type;
	uint16 state;		/* Get/Set */
	uint8 force;		/* 0 - auto (HW) state, 1 - forced state */
	char name[WL_MPF_STATE_NAME_MAX]; /* Get/Set: Optional/actual name */
	uint8  PAD;
} wl_mpf_state_t;
#ifndef LINUX_POSTMOGRIFY_REMOVAL
/*
 * WLFCTS definition
 */
typedef struct wl_txstatus_additional_info {
	uint32 rspec;
	uint32 enq_ts;
	uint32 last_ts;
	uint32 entry_ts;
	uint16 seq;
	uint8  rts_cnt;
	uint8  tx_cnt;
} wl_txstatus_additional_info_t;

#endif /* LINUX_POSTMOGRIFY_REMOVAL */

/** Service discovery */
typedef struct {
	uint8	transaction_id;	/**< Transaction id */
	uint8	protocol;	/**< Service protocol type */
	uint16	query_len;	/**< Length of query */
	uint16	response_len;	/**< Length of response */
	uint8	qrbuf[];
} wl_p2po_qr_t;

typedef struct {
	uint16			period;			/**< extended listen period */
	uint16			interval;		/**< extended listen interval */
	uint16                  count;                  /* count to repeat */
	uint16                  pad;                    /* pad for 32bit align */
} wl_p2po_listen_t;

/** GAS state machine tunable parameters.  Structure field values of 0 means use the default. */
typedef struct wl_gas_config {
	uint16 max_retransmit;		/**< Max # of firmware/driver retransmits on no Ack
					 * from peer (on top of the ucode retries).
					 */
	uint16 response_timeout;	/**< Max time to wait for a GAS-level response
					 * after sending a packet.
					 */
	uint16 max_comeback_delay;	/**< Max GAS response comeback delay.
					 * Exceeding this fails the GAS exchange.
					 */
	uint16 max_retries;		/**< Max # of GAS state machine retries on failure
					 * of a GAS frame exchange.
					 */
} wl_gas_config_t;

/** P2P Find Offload parameters */
typedef struct wl_p2po_find_config {
	uint16 version;			/**< Version of this struct */
	uint16 length;			/**< sizeof(wl_p2po_find_config_t) */
	int32 search_home_time;		/**< P2P search state home time when concurrent
					 * connection exists.  -1 for default.
					 */
	uint8 num_social_channels;
			/**< Number of social channels up to WL_P2P_SOCIAL_CHANNELS_MAX.
			 * 0 means use default social channels.
			 */
	uint8 flags;
	uint16 social_channels[1];	/**< Variable length array of social channels */
} wl_p2po_find_config_t;
#define WL_P2PO_FIND_CONFIG_VERSION 2	/**< value for version field */

/** wl_p2po_find_config_t flags */
#define P2PO_FIND_FLAG_SCAN_ALL_APS 0x01	/**< Whether to scan for all APs in the p2po_find
						 * periodic scans of all channels.
						 * 0 means scan for only P2P devices.
						 * 1 means scan for P2P devices plus non-P2P APs.
						 */

/** For adding a WFDS service to seek */
typedef struct {
	uint32 seek_hdl;		/**< unique id chosen by host */
	uint8 addr[6];			/**< Seek service from a specific device with this
					 * MAC address, all 1's for any device.
					 */
	uint8 service_hash[P2P_WFDS_HASH_LEN];
	uint8 service_name_len;
	uint8 service_name[MAX_WFDS_SEEK_SVC_NAME_LEN];
					/**< Service name to seek, not null terminated */
	uint8 service_info_req_len;
	uint8 service_info_req[1];	/**< Service info request, not null terminated.
					 * Variable length specified by service_info_req_len.
					 * Maximum length is MAX_WFDS_SEEK_SVC_INFO_LEN.
					 */
} wl_p2po_wfds_seek_add_t;

/** For deleting a WFDS service to seek */
typedef struct {
	uint32 seek_hdl;		/**< delete service specified by id */
} wl_p2po_wfds_seek_del_t;

/** For adding a WFDS service to advertise */
#include <packed_section_start.h>
typedef BWL_PRE_PACKED_STRUCT struct {
	uint32 advertise_hdl;		/**< unique id chosen by host */
	uint8 service_hash[P2P_WFDS_HASH_LEN];
	uint32 advertisement_id;
	uint16 service_config_method;
	uint8 service_name_len;
	uint8 service_name[MAX_WFDS_SVC_NAME_LEN];
					/**< Service name , not null terminated */
	uint8 service_status;
	uint16 service_info_len;
	uint8 service_info[1];		/**< Service info, not null terminated.
					 * Variable length specified by service_info_len.
					 * Maximum length is MAX_WFDS_ADV_SVC_INFO_LEN.
					 */
} BWL_POST_PACKED_STRUCT wl_p2po_wfds_advertise_add_t;
#include <packed_section_end.h>

/** For deleting a WFDS service to advertise */
typedef struct {
	uint32 advertise_hdl;	/**< delete service specified by hdl */
} wl_p2po_wfds_advertise_del_t;

/** P2P Offload discovery mode for the p2po_state iovar */
typedef enum {
	WL_P2PO_DISC_STOP,
	WL_P2PO_DISC_LISTEN,
	WL_P2PO_DISC_DISCOVERY
} disc_mode_t;

/* ANQP offload */

#define ANQPO_MAX_QUERY_SIZE		256
typedef struct {
	uint16 max_retransmit;		/**< ~0 use default, max retransmit on no ACK from peer */
	uint16 response_timeout; /**< ~0 use default, msec to wait for resp after tx packet */
	uint16 max_comeback_delay;	/**< ~0 use default, max comeback delay in resp else fail */
	uint16 max_retries;		/**< ~0 use default, max retries on failure */
	uint16 query_len;		/**< length of ANQP query */
	uint8 query_data[1];		/**< ANQP encoded query (max ANQPO_MAX_QUERY_SIZE) */
} wl_anqpo_set_t;

typedef struct {
	uint16 channel;			/**< channel of the peer */
	struct ether_addr addr;		/**< addr of the peer */
} wl_anqpo_peer_t;

#define ANQPO_MAX_PEER_LIST			64
typedef struct {
	uint16 count;				/**< number of peers in list */
	wl_anqpo_peer_t peer[1];	/**< max ANQPO_MAX_PEER_LIST */
} wl_anqpo_peer_list_t;

#define ANQPO_MAX_IGNORE_SSID		64
typedef struct {
	uint8 is_clear;				/**< set to clear list (not used on GET) */
	uint8 PAD;
	uint16 count;				/**< number of SSID in list */
	wlc_ssid_t ssid[1];			/**< max ANQPO_MAX_IGNORE_SSID */
} wl_anqpo_ignore_ssid_list_t;

#define ANQPO_MAX_IGNORE_BSSID		64
typedef struct {
	uint8 is_clear;				/**< set to clear list (not used on GET) */
	uint8 PAD;
	uint16 count;				/**< number of addr in list */
	struct ether_addr bssid[];	/**< max ANQPO_MAX_IGNORE_BSSID */
} wl_anqpo_ignore_bssid_list_t;

struct toe_ol_stats_t {
	/** Num of tx packets that don't need to be checksummed */
	uint32 tx_summed;

	/* Num of tx packets where checksum is filled by offload engine */
	uint32 tx_iph_fill;
	uint32 tx_tcp_fill;
	uint32 tx_udp_fill;
	uint32 tx_icmp_fill;

	/*  Num of rx packets where toe finds out if checksum is good or bad */
	uint32 rx_iph_good;
	uint32 rx_iph_bad;
	uint32 rx_tcp_good;
	uint32 rx_tcp_bad;
	uint32 rx_udp_good;
	uint32 rx_udp_bad;
	uint32 rx_icmp_good;
	uint32 rx_icmp_bad;

	/* Num of tx packets in which csum error is injected */
	uint32 tx_tcp_errinj;
	uint32 tx_udp_errinj;
	uint32 tx_icmp_errinj;

	/* Num of rx packets in which csum error is injected */
	uint32 rx_tcp_errinj;
	uint32 rx_udp_errinj;
	uint32 rx_icmp_errinj;
};

/** Arp offload statistic counts */
struct arp_ol_stats_t {
	uint32  host_ip_entries; /**< Host IP table addresses (more than one if multihomed) */
	uint32  host_ip_overflow;	/**< Host IP table additions skipped due to overflow */

	uint32  arp_table_entries;	/**< ARP table entries */
	uint32  arp_table_overflow;	/**< ARP table additions skipped due to overflow */

	uint32  host_request;		/**< ARP requests from host */
	uint32  host_reply;		/**< ARP replies from host */
	uint32  host_service;		/**< ARP requests from host serviced by ARP Agent */

	uint32  peer_request;		/**< ARP requests received from network */
	uint32  peer_request_drop;	/**< ARP requests from network that were dropped */
	uint32  peer_reply;		/**< ARP replies received from network */
	uint32  peer_reply_drop;	/**< ARP replies from network that were dropped */
	uint32  peer_service;		/**< ARP request from host serviced by ARP Agent */
};

/** NS offload statistic counts */
struct nd_ol_stats_t {
	uint32  host_ip_entries;    /**< Host IP table addresses (more than one if multihomed) */
	uint32  host_ip_overflow;   /**< Host IP table additions skipped due to overflow */
	uint32  peer_request;       /**< NS requests received from network */
	uint32  peer_request_drop;  /**< NS requests from network that were dropped */
	uint32  peer_reply_drop;    /**< NA replies from network that were dropped */
	uint32  peer_service;       /**< NS request from host serviced by firmware */
};

/*
 * Neighbor Discovery Offloading
 */
enum {
	WL_ND_IPV6_ADDR_TYPE_UNICAST = 0,
	WL_ND_IPV6_ADDR_TYPE_ANYCAST
};

typedef struct wl_nd_host_ip_addr {
	struct ipv6_addr ip_addr;	/* host ip address */
	uint8 type;			/* type of address */
	uint8 pad[3];
} wl_nd_host_ip_addr_t;

typedef struct wl_nd_host_ip_list {
	uint32 count;
	wl_nd_host_ip_addr_t host_ip[1];
} wl_nd_host_ip_list_t;

#define WL_ND_HOSTIP_IOV_VER    1

enum {
	WL_ND_HOSTIP_OP_VER = 0,	/* get version */
	WL_ND_HOSTIP_OP_ADD,		/* add address */
	WL_ND_HOSTIP_OP_DEL,		/* delete specified address */
	WL_ND_HOSTIP_OP_DEL_UC,		/* delete all unicast address */
	WL_ND_HOSTIP_OP_DEL_AC,		/* delete all anycast address */
	WL_ND_HOSTIP_OP_DEL_ALL,	/* delete all addresses */
	WL_ND_HOSTIP_OP_LIST,		/* get list of host ip address */
	WL_ND_HOSTIP_OP_MAX
};

typedef struct wl_nd_hostip {
	uint16 version;				/* version of iovar buf */
	uint16 op_type;				/* operation type */
	uint32 length;				/* length of entire structure */
	union {
		wl_nd_host_ip_addr_t host_ip;	/* set param for add */
		uint16 version;			/* get return for ver */
	} u;
} wl_nd_hostip_t;

#define WL_ND_HOSTIP_FIXED_LEN		OFFSETOF(wl_nd_hostip_t, u)
#define WL_ND_HOSTIP_WITH_ADDR_LEN	(WL_ND_HOSTIP_FIXED_LEN + sizeof(wl_nd_host_ip_addr_t))

/*
 * Keep-alive packet offloading.
 */

/**
 * NAT keep-alive packets format: specifies the re-transmission period, the packet
 * length, and packet contents.
 */
typedef struct wl_keep_alive_pkt {
	uint32	period_msec;	/** Retransmission period (0 to disable packet re-transmits) */
	uint16	len_bytes;	/* Size of packet to transmit (0 to disable packet re-transmits) */
	uint8	data[1];	/** Variable length packet to transmit.  Contents should include
				 * entire ethernet packet (enet header, IP header, UDP header,
				 * and UDP payload) in network byte order.
				 */
} wl_keep_alive_pkt_t;

#define WL_KEEP_ALIVE_FIXED_LEN		OFFSETOF(wl_keep_alive_pkt_t, data)

#ifndef LINUX_POSTMOGRIFY_REMOVAL
#define MAX_RSSI_COUNT			8
typedef struct rssi_struct {
	int8	val[MAX_RSSI_COUNT];	/**< rssi values in AFs */
	int16	sum;			/**< total rssi sum */
	uint8	cnt;			/**< number rssi samples */
	uint8	idx;			/**< next rssi location */
} rssi_struct_t;

#ifdef WLAWDL

/* Capability Bits */
#define WLC_AWDL_CAP_SEC_PAYLOAD        0x1

typedef struct awdl_af_sec_payload {
	uint16	version;	/* Version of this structure */
	uint16	length;		/* Length of this entire struct including payload */
	uint16	pri_pload_bm;	/* Primary payload bitmask */
	uint16	sec_pload_bm;	/* Secondary payload bitmask */
	uint8	payload[];	/* Secondary Payload */
} awdl_af_sec_payload_t;

typedef struct awdl_config_params {
	uint32	version;
	uint8	awdl_chan;		/**< awdl channel */
	uint8	guard_time;		/**< Guard Time */
	uint16	aw_period;		/**< AW interval period */
	uint16  aw_cmn_length;		/**< Radio on Time AW */
	uint16	action_frame_period;	/**< awdl action frame period */
	uint16  awdl_pktlifetime;	/**< max packet life time in msec for awdl action frames  */
	uint16  awdl_maxnomaster;	/**< max master missing time */
	uint16  awdl_extcount;		/**< Max extended period count for traffic  */
	uint16	aw_ext_length;		/**< AW ext period */
	uint16	awdl_nmode;	        /**< Operation mode of awdl interface; * 0 - Legacy mode
					 * 1 - 11n rate only   * 2 - 11n + ampdu rx/tx
					 */
	struct ether_addr ea;		/**< destination bcast/mcast  address to which action frame
					 * need to be sent
					 */
} awdl_config_params_t;

typedef struct wl_awdl_action_frame {
	uint16	len_bytes;
	uint8	awdl_action_frame_data[1];
} wl_awdl_action_frame_t;

#define WL_AWDL_ACTION_FRAME_FIXED_LEN		OFFSETOF(wl_awdl_action_frame_t, awdl_sync_frame)

typedef struct awdl_peer_node {
	uint32	type_state;		/**< Master, slave , etc.. */
	uint16	aw_counter;		/**< avail window counter */
	int8	rssi;			/**< rssi last af was received at */
	int8	last_rssi;		/**< rssi in the last AF */
	uint16	tx_counter;		/**<count down timer to next AW */
	uint16	tx_delay;		/**< ts_hw - ts_fw */
	uint16	period_tu;
	uint16	aw_period;		/**< AW period - aw_cmn + ext * ext_len */
	uint16	aw_cmn_length;		/**< Common AW length */
	uint16	aw_ext_length;		/**< AW_EXT length */
	uint32	self_metrics;		/**< Election Metric */
	uint32	top_master_metrics;	/**< Top Master Metric */
	struct ether_addr	addr;
	struct ether_addr	top_master;
	uint8	dist_top;		/**< Distance from Top */
	uint8  has_private_election_params;
	struct ether_addr private_top_master;
	uint32 private_top_master_metric;
	uint32 private_election_ID;
	uint8  private_distance_from_top;
	uint8  PAD[3];
} awdl_peer_node_t;

typedef struct awdl_peer_table {
	uint16  version;
	uint16	len;
	uint8 peer_nodes[1];
} awdl_peer_table_t;

/* structure for adding advertisers from host */
typedef struct awdl_peer_advet_add {
	uint16	aw_counter;		/* avail window counter */
	uint16	tx_counter;		/* Down counter */
	uint16	tx_delay;		/* ts_hw - ts_fw */
	uint16	period_tu;
	uint16	aw_cmn_length;
	uint16	aw_ext_length;
	uint16	aw_period;
	int8	rssi;			/* RSSI value */
	uint8	guard_time;
	uint8	presence_mode;
	uint8	age;			/* age of this record */
	uint8	dist_top;		/* Distance from Top */
	uint8	PAD;
	uint32  ms;			/* time at which we received the AF */
	uint32	self_metrics;		/* election metric */
	uint32	top_master_metrics;
	struct ether_addr	top_master;	/* Top Master address */
	struct ether_addr	addr;	/* Peer address to be added */
} awdl_peer_advet_add_t;

typedef struct awdl_af_hdr {
	struct ether_addr dst_mac;
	uint8 action_hdr[4]; /**< Category + OUI[3] */
} awdl_af_hdr_t;

typedef struct awdl_oui {
	uint8 oui[3];	/**< default: 0x00 0x17 0xf2 */
	uint8 oui_type; /**< AWDL: 0x08 */
} awdl_oui_t;

typedef struct awdl_hdr {
	uint8	type;		/**< 0x08 AWDL */
	uint8	version;
	uint8	sub_type;	/**< Sub type */
	uint8	rsvd;		/**< Reserved */
	uint32	phy_timestamp;	/**< PHY Tx time */
	uint32	fw_timestamp;	/**< Target Tx time */
} awdl_hdr_t;

#ifndef MACOSX
#define AWDL_DEFAULT_MAX_PEERS		8
#else
#define AWDL_DEFAULT_MAX_PEERS		128
#endif // endif

/* AWDL AF flags for awdl_oob_af iovar */
#define AWDL_OOB_AF_FILL_TSF_PARAMS			0x00000001
#define AWDL_OOB_AF_FILL_SYNC_PARAMS		0x00000002
#define AWDL_OOB_AF_FILL_ELECT_PARAMS		0x00000004
#define AWDL_OOB_AF_PARAMS_SIZE 38

#include <packed_section_start.h>
typedef BWL_PRE_PACKED_STRUCT struct awdl_oob_af_params {
	struct ether_addr bssid;
	struct ether_addr dst_mac;
	uint32 channel;
	uint32 dwell_time;
	uint32 flags;
	uint32 pkt_lifetime;
	uint32 tx_rate;
	uint32 max_retries; /**< for unicast frames only */
	uint16 payload_len;
	uint8  payload[1]; /**< complete AF payload */
} BWL_POST_PACKED_STRUCT awdl_oob_af_params_t;
#include <packed_section_end.h>

#include <packed_section_start.h>
typedef BWL_PRE_PACKED_STRUCT struct awdl_oob_af_params_async {
	uint32 tx_time;     /**< tsf time to transmit, in usec */
	uint16 tag;         /**< packet tag */
	struct ether_addr bssid;
	struct ether_addr dst_mac;
	uint32 channel;
	uint32 dwell_time;
	uint32 flags;
	uint32 pkt_lifetime;
	uint32 tx_rate;
	uint32 max_retries; /**< for unicast frames only */
	uint16 payload_len;
	uint8  payload[1]; /**< complete AF payload */
} BWL_POST_PACKED_STRUCT awdl_oob_af_params_async_t;
#include <packed_section_end.h>

#include <packed_section_start.h>
typedef BWL_PRE_PACKED_STRUCT struct awdl_oob_af_params_auto {
	uint32 tx_chan_map; /**< bitmap for the channels in the chan seq to transmit the af */
	uint32 tx_aws_offset; /**< time to transmit from the aw start, in usec */
	struct ether_addr bssid;
	struct ether_addr dst_mac;
	uint32 channel;
	uint32 dwell_time;
	uint32 flags;
	uint32 pkt_lifetime;
	uint32 tx_rate;
	uint32 max_retries; /**< for unicast frames only */
	uint16 payload_len;
	uint8  payload[1]; /**< complete AF payload */
} BWL_POST_PACKED_STRUCT awdl_oob_af_params_auto_t;
#include <packed_section_end.h>

#include <packed_section_start.h>
typedef BWL_PRE_PACKED_STRUCT struct awdl_sync_params {
	uint8	type;			/**< Type */
	uint16	param_len;		/**< sync param length */
	uint8	tx_chan;		/**< tx channel */
	uint16	tx_counter;		/**< tx down counter */
	uint8	master_chan;		/**< master home channel */
	uint8	guard_time;		/**< Guard Time */
	uint16	aw_period;		/**< AW period */
	uint16	action_frame_period;	/**< awdl action frame period */
	uint16	awdl_flags;		/**< AWDL Flags */
	uint16	aw_ext_length;		/**< AW extention len */
	uint16	aw_cmn_length;		/**< AW common len */
	uint16	aw_remaining;		/**< Remaining AW length */
	uint8	min_ext;		/**< Minimum Extention count */
	uint8	max_ext_multi;		/**< Max multicast Extention count */
	uint8	max_ext_uni;		/**< Max unicast Extention count */
	uint8	max_ext_af;		/**< Max af Extention count */
	struct ether_addr current_master;	/**< Current Master mac addr */
	uint8	presence_mode;		/**< Presence mode */
	uint8	reserved;
	uint16	aw_counter;		/**< AW seq# */
	uint16	ap_bcn_alignment_delta;	/**< AP Beacon alignment delta  */
} BWL_POST_PACKED_STRUCT awdl_sync_params_t;
#include <packed_section_end.h>

#include <packed_section_start.h>
typedef BWL_PRE_PACKED_STRUCT struct awdl_channel_sequence {
	uint8	aw_seq_len;		/**< AW seq length */
	uint8	aw_seq_enc;		/**< AW seq encoding */
	uint8	aw_seq_duplicate_cnt;	/**< AW seq dupilcate count */
	uint8	seq_step_cnt;		/**< Seq spet count */
	uint16	seq_fill_chan;		/**< channel to fill in; 0xffff repeat current channel */
	uint8	chan_sequence[1];	/**< Variable list of channel Sequence */
} BWL_POST_PACKED_STRUCT awdl_channel_sequence_t;
#include <packed_section_end.h>
#define WL_AWDL_CHAN_SEQ_FIXED_LEN   OFFSETOF(awdl_channel_sequence_t, chan_sequence)

#include <packed_section_start.h>
typedef BWL_PRE_PACKED_STRUCT struct awdl_election_info {
	uint8	election_flags;	/**< Election Flags */
	uint16	election_ID;	/**< Election ID */
	uint32	self_metrics;
} BWL_POST_PACKED_STRUCT awdl_election_info_t;
#include <packed_section_end.h>

#include <packed_section_start.h>
typedef BWL_PRE_PACKED_STRUCT struct awdl_election_tree_info {
	uint8	election_flags;	/**< Election Flags */
	uint16	election_ID;	/**< Election ID */
	uint32	self_metrics;
	int8 close_sync_rssi_thld;
	int8 master_rssi_boost;
	int8 edge_sync_rssi_thld;
	int8 close_range_rssi_thld;
	int8 mid_range_rssi_thld;
	uint8 max_higher_masters_close_range;
	uint8 max_higher_masters_mid_range;
	uint8 max_tree_depth;
	/* read only */
	struct ether_addr top_master;	/**< top Master mac addr */
	uint32 top_master_self_metric;
	uint8  current_tree_depth;

	uint8 edge_master_dwell_cnt;
	struct ether_addr private_top_master;	/**< private top Master mac addr */
	uint32 private_top_master_metric;
	uint32 private_election_ID;
	uint8  private_distance_from_top;
} BWL_POST_PACKED_STRUCT awdl_election_tree_info_t;
#include <packed_section_end.h>

#include <packed_section_start.h>
typedef BWL_PRE_PACKED_STRUCT struct awdl_election_params_tlv {
	uint8	type;			/**< Type */
	uint16	param_len;		/**< Election param length */
	uint8	election_flags;	/**< Election Flags */
	uint16	election_ID;	/**< Election ID */
	uint8	dist_top;	/**< Distance from Top */
	uint8	rsvd;		/**< Reserved */
	struct ether_addr top_master;	/**< Top Master mac addr */
	uint32	top_master_metrics;
	uint32	self_metrics;
	uint8	pad[2];		/**< Padding  */
} BWL_POST_PACKED_STRUCT awdl_election_params_tlv_t;
#include <packed_section_end.h>

#include <packed_section_start.h>
typedef BWL_PRE_PACKED_STRUCT struct awdl_dfsp_params_tlv {
	uint8 type;
	uint16 len;
	uint8 ie[];    /* variable length */
} BWL_POST_PACKED_STRUCT awdl_dfsp_params_tlv_t;
#include <packed_section_end.h>

#define AWDL_DFSP_BCN_TLV_VALUE_OFFSET	OFFSETOF(awdl_dfsp_bcn_tlv_t, time_since_bcn)
#include <packed_section_start.h>
typedef BWL_PRE_PACKED_STRUCT struct awdl_dfsp_bcn_tlv {
	uint8 type;
	uint16 len;
	uint16 time_since_bcn;	/* last time beacon seen in ms */
} BWL_POST_PACKED_STRUCT awdl_dfsp_bcn_tlv_t;
#include <packed_section_end.h>

#define AWDL_DFSP_EVT_OFFSET			OFFSETOF(awdl_dfsp_event_data_t, ie)
#define AWDL_DFSP_EVT_FLAGS_AP_ASSOC	(1 << 0)
#define AWDL_DFSP_EVT_FLAGS_AP_BCNMON	(1 << 1)
#define AWDL_DFSP_EVT_FLAGS_PROXY_BCSA	(1 << 2)
#define AWDL_DFSP_EVT_FLAGS_PROXY_UCSA	(1 << 3)
#define AWDL_DFSP_EVT_FLAGS_PROXY_PCSA	(1 << 4)
typedef struct awdl_dfsp_event_data {
	uint16 flags;	/* indicate what triggers the event */
	uint16 ie_len;
	uint8 ie[];    /* variable length */
} awdl_dfsp_event_data_t;

/*
 * Definition in DINGO
 */
typedef struct awdl_opmode_v2 {
	uint8			mode;		/* 0 - Auto; 1 - Fixed; 2 - Forced */
	uint8			role;		/* 0 - slave; 1 - non-elect master; 2 - master */
	uint16			bcast_tu;	/* Bcasting period(TU) for non-elect master */
	struct ether_addr	master;		/* Address of master to sync to */
	uint16			cur_bcast_tu;	/* Current Bcasting Period(TU) */
	uint8			master_type;
	uint8			dist_top;
	uint16			cluster_id;
	uint32			tsf_offset_h;
	uint32			tsf_offset_l;
} awdl_opmode_v2_t;

typedef struct awdl_payload {
	uint16	len;		/**< Payload length */
	uint8	payload[1];	/**< Payload */
} awdl_payload_t;

typedef struct awdl_long_payload {
	uint8   long_psf_period;      /**< transmit every long_psf_perios AWs */
	uint8   long_psf_tx_offset;   /**< delay from aw_start */
	uint16	len;		          /**< Payload length */
	uint8	payload[1];           /**< Payload */
} awdl_long_payload_t;

/* Values for awdl_opmode_t.role */
#define AWDL_ROLE_SLAVE		0
#define AWDL_ROLE_NE_MASTER	1
#define AWDL_ROLE_MASTER	2
/*
 * Definitions on PHO, BIS, TRUNK & IGU branches
 */
typedef struct awdl_opmode_v1 {
	uint8	mode;		/* 0 - Auto; 1 - Fixed */
	uint8	role;		/* 0 - slave; 1 - non-elect master; 2 - master */
	uint16	bcast_tu;	/* Bcasting period(TU) for non-elect master */
	struct ether_addr master; /* Address of master to sync to */
	uint16	cur_bcast_tu;	/* Current Bcasting Period(TU) */
} awdl_opmode_v1_t;

typedef awdl_opmode_v1_t awdl_opmode_t;

typedef union awdl_opmode_un {
	awdl_opmode_v1_t	opmode_v1;
	awdl_opmode_v2_t	opmode_v2;
} awdl_opmode_un_t;

typedef struct awdl_extcount {
	uint8	minExt;			/**< Min extension count */
	uint8	maxExtMulti;	/**< Max extension count for mcast packets */
	uint8	maxExtUni;		/**< Max extension count for unicast packets */
	uint8	maxAfExt;			/**< Max extension count */
} awdl_extcount_t;

#define AWDL_OPMODE_AUTO	0
#define AWDL_OPMODE_FIXED	1
#define AWDL_OPMODE_FORCED	2	/* Fixed Mode with Forced_mode on */

/** peer add/del operation */
typedef struct awdl_peer_op {
	uint8 version;
	uint8 opcode;	/**< see opcode definition */
	struct ether_addr addr;
	uint8 mode;
} awdl_peer_op_t;

/** peer op table */
typedef struct awdl_peer_op_tbl {
	uint16	len;		/**< length */
	uint8	tbl[1];	/**< Peer table */
} awdl_peer_op_tbl_t;

#include <packed_section_start.h>
typedef BWL_PRE_PACKED_STRUCT struct awdl_peer_op_node {
	struct ether_addr addr;
	uint32 flags;		/**< Flags to indicate various states */
	uint16 chanseq_len;
	uint8  chanseq[1];
} BWL_POST_PACKED_STRUCT awdl_peer_op_node_t;
#include <packed_section_end.h>

/* awdl_peer_op_node_t flags */
#define AWDL_PEER_NODE_OP_FLAG_HT        0x01
#define AWDL_PEER_NODE_OP_FLAG_AMPDU     0x02
#define AWDL_PEER_NODE_OP_FLAG_PM        0x04
#define AWDL_PEER_NODE_OP_FLAG_ABAND     0x08
#define AWDL_PEER_NODE_OP_FLAG_QOS       0x10
#define AWDL_PEER_NODE_OP_FLAG_AWDL      0x20
#define AWDL_PEER_NODE_OP_FLAG_VHT       0x40
#define AWDL_PEER_OP_CUR_VER	0

#define AWDL_STATS_VERSION_3 3
#define AWDL_STATS_CURRENT_VERSION AWDL_STATS_VERSION_3

/** AWDL related statistics */
typedef struct awdl_stats_core_v3 {
	uint32	slotstart;		/* AW slot_start */
	uint32	slotend;		/* AW slot_end */
	uint32	slotskip;		/* AW slot_skip */
	uint32	slotstart_partial;	/* AW slot resume */
	uint32	slotend_partial;	/* AW slot pre-empt */
	uint32	psfstart;		/* PSF slot_start */
	uint32	psfend;		/* PSF slot_end */
	uint32	psfskip;	/* PSF slot_skip */
	uint32	psfreqfail;		/* PSF timeslot register fail */
	uint32	psfcnt;		/* Number of PSFs */
	uint32	micnt;		/* Number of MI frames */
	uint32	chansw;		/* Total number of chan switches */
	uint32	awrealignfail;	/* No of awrealign failures */
	uint32  datatx;
	uint32  datarx;
	uint32  txdrop;
	uint32  rxdrop;
	uint32  monrx;
	uint32  txsupr;
	uint32  rx80211;
} awdl_stats_core_v3_t;

typedef struct awdl_stats_cmn_v3 {
	uint32	afrx;
	uint32	aftx;
	uint32	lostmaster;
	uint32	misalign;
	uint32	aws;
	uint32	aw_dur;
	uint32	debug;
	uint32	afrxdrop;
	uint32  awdrop;
	uint32  noawchansw;
	uint32  peeropdrop;
	uint16	chancal;
	uint16	nopreawint;
	uint32	awdropchsw;
	uint32	nopreawchsw;
	uint32  nopreawprep;
	uint32  aws_misalign;
	uint32	txeval_fail;
	uint32	infra_reqrcq;
	uint32	awdl_reqtxq;
	uint32  psfchanswtchskip;       /* # chan. sw skipped during PSF xmit */
	uint32  psfstateupdskip;        /* # of PM updates skipped */
	uint32  infra_offchpsf;
	uint32  awdl_offchpsf;
	uint32  chseqreq;
	uint32  peerdelreq;
	uint32	awend;
	uint32	awrealign;		/* No of AW realigns */
	uint32	awchmismatch;	/* Channel mismatch between expcted ch and scheduled ch */
} awdl_stats_cmn_v3_t;

typedef struct awdl_dfsp_stats {
	uint32  dfsp_csainfra;
	uint32  dfsp_csabcnmon;
	uint32  dfsp_bcsarx;
	uint32  dfsp_ucsarx;
	uint32  dfsp_pcsarx;
	uint32  dfsp_bcsatx;
	uint32  dfsp_ucsatx;
	uint32  dfsp_pcsatx;
	uint32  dfsp_ucsatxfail;
	uint32  dfsp_evtnotif;
	uint32  dfsp_evtsuspect;
	uint32  dfsp_evtresume;
} awdl_dfsp_stats_t;

typedef struct awdl_stats_v3 {
	uint16	version;
	uint16	length;
	awdl_stats_cmn_v3_t cmnstats;
	awdl_stats_core_v3_t corestats[MAX_NUM_D11CORES];
	awdl_dfsp_stats_t dfspstats;
} awdl_stats_v3_t;

typedef struct awdl_stats_v2 {
	uint32  afrx;
	uint32  aftx;
	uint32  datatx;
	uint32  datarx;
	uint32  txdrop;
	uint32  rxdrop;
	uint32  monrx;
	uint32  lostmaster;
	uint32  misalign;
	uint32  aws;
	uint32  aw_dur;
	uint32  debug;
	uint32  txsupr;
	uint32  afrxdrop;
	uint32  awdrop;
	uint32  noawchansw;
	uint32  rx80211;
	uint32  peeropdrop;
	uint16  chancal;
	uint16  nopreawint;
	uint32  awdropchsw;
	uint32  nopreawchsw;
	uint32  nopreawprep;
	uint32  aws_misalign;
	uint32  txeval_fail;
	uint32  infra_reqrcq;
	uint32  awdl_reqtxq;
	uint32  psfchanswtchskip;       /* # chan. sw skipped during PSF xmit */
	uint32  psfstateupdskip;        /* # of PM updates skipped */
	uint32	dfsp_csainfra;
	uint32	dfsp_csabcnmon;
	uint32	dfsp_bcsarx;
	uint32	dfsp_ucsarx;
	uint32	dfsp_pcsarx;
	uint32	dfsp_bcsatx;
	uint32	dfsp_ucsatx;
	uint32	dfsp_pcsatx;
	uint32	dfsp_ucsatxfail;
	uint32	dfsp_evtnotif;
	uint32	dfsp_evtsuspect;
	uint32	dfsp_evtresume;
} awdl_stats_v2_t;

typedef struct awdl_stats_v1 {
	uint32	afrx;
	uint32	aftx;
	uint32	datatx;
	uint32	datarx;
	uint32	txdrop;
	uint32	rxdrop;
	uint32	monrx;
	uint32	lostmaster;
	uint32	misalign;
	uint32	aws;
	uint32	aw_dur;
	uint32	debug;
	uint32  txsupr;
	uint32	afrxdrop;
	uint32  awdrop;
	uint32  noawchansw;
	uint32  rx80211;
	uint32  peeropdrop;
	uint16	chancal;
	uint16	nopreawint;
	uint32	awdropchsw;
	uint32	nopreawchsw;
	uint32  nopreawprep;
	uint32  infra_offchpsf;
	uint32  awdl_offchpsf;
	uint32  pmnoack;
	uint32  scanreq;
	uint32  chseqreq;
	uint32  peerdelreq;
	uint32  aws_misalign;
	uint32  txeval_fail;
	uint32  infra_reqrcq;
	uint32  awdl_reqtxq;
	uint32  psfchanswtchskip;       /* # chan. sw skipped during PSF xmit */
	uint32  psfstateupdskip;        /* # of PM updates skipped */
	uint32	dfsp_csainfra;
	uint32	dfsp_csabcnmon;
	uint32	dfsp_bcsarx;
	uint32	dfsp_ucsarx;
	uint32	dfsp_pcsarx;
	uint32	dfsp_bcsatx;
	uint32	dfsp_ucsatx;
	uint32	dfsp_pcsatx;
	uint32	dfsp_ucsatxfail;
	uint32	dfsp_evtnotif;
	uint32	dfsp_evtsuspect;
	uint32	dfsp_evtresume;
} awdl_stats_v1_t;

typedef struct awdl_uct_stats {
	uint32 aw_proc_in_aw_sched;
	uint32 aw_upd_in_pre_aw_proc;
	uint32 pre_aw_proc_in_aw_set;
	uint32 ignore_pre_aw_proc;
	uint32 miss_pre_aw_intr;
	uint32 aw_dur_zero;
	uint32 aw_sched;
	uint32 aw_proc;
	uint32 pre_aw_proc;
	uint32 not_init;
	uint32 null_awdl;
} awdl_uct_stats_t;

/* peer opcode */
#define AWDL_PEER_OP_ADD	0
#define AWDL_PEER_OP_DEL	1
#define AWDL_PEER_OP_INFO	2
#define AWDL_PEER_OP_UPD	3

/**AWDL Piggy backed scan */
typedef struct wl_awdl_pscan_params {
	wlc_ssid_t ssid;		/**< default: {0, ""} */
	struct ether_addr bssid;	/**< default: bcast */
	uint8 scan_type;		/**< active or passive, 0 use default */
	uint8 pad;			/**< pad */
	int32 nprobes;			/**< -1 use default, number of probes per channel */
	int32 aw_seq_num;	/**< count AW sequence nunbers to be piggy backed for scan */
	int32 nssid;			/**< count of ssid in list */
	int32 rsvd;			/**< Reserved  */
	uint16 aw_counter_list[1];	/**< This is a list contains in following order
					 *    - List aw seq numbers
					 *    - List of SSID's 4 byte aligned.
					 */
} wl_awdl_pscan_params_t;

typedef struct wl_pscan_params {
	uint32 version;
	uint16 action;		/**< PSCAN action type: FW or Host initiated pscan or abort pscan */
	uint16 sync_id;
	wl_awdl_pscan_params_t params;
} wl_pscan_params_t;

#define WL_AWDL_PSCAN_PARAMS_FIXED_SIZE (OFFSETOF(wl_awdl_pscan_params_t, aw_counter_list))
#define WL_AWDL_MAX_NUM_AWSEQ	64
#define AWDL_PSCAN_REQ_VERSION	1

/** awdl pscan action type values */
#define AWDL_HOST_PSCAN		0	/**< Host Initiated PSCAN */
#define AWDL_FW_PSCAN		1	/**< Firmware Initiated PSCAN */
#define AWDL_ABORT_PSCAN	2	/**< Abort any PSCAN */

/* "aftxmode" iovar values */
#define AWDL_AFTXMODE_AUTO	0	/**< Send AF on AWDL channel best effort while outside AW */

/* --- Deprecated ---- */
#define AWDL_AFTXMODE_INFRA	1	/**< Send AF on Infra channel while outside AW */
#define AWDL_AFTXMODE_CUR_CHAN	2	/**< Send AF on Current channel while outside AW */
/* --- Deprecated ---- */

#define AWDL_AFTXMODE_SUPPRESS	3	/**< Suppress AF Tx */
#define AWDL_AFTXMODE_SYNC_PREAW 4	/**< Send AF on master channel/s always in pre AW time */
#define AWDL_AFTXMODE_LAST	4	/**< Last AWDL_AFTXMODE_XXX */

typedef struct awdl_pw_opmode {
	struct ether_addr top_master;	/**< Peer mac addr */
	uint8 mode; /**< 0 - normal; 1 - fast mode */
} awdl_pw_opmode_t;

/** i/f request */
typedef struct wl_awdl_if2 {
	int32 cfg_idx;
	int32 up;
	struct ether_addr bssid;
	struct ether_addr if_addr;
} wl_awdl_if2_t;

typedef struct _aw_start {
	uint8 role;
	struct ether_addr	master;
	uint8	aw_seq_num;
} aw_start_t;

typedef struct _aw_extension_start {
	uint8 aw_ext_num;
} aw_extension_start_t;

typedef struct _awdl_peer_state {
	struct ether_addr peer;
	uint8	state;
} awdl_peer_state_t;
#define AWDL_PEER_STATE_OPEN	0
#define AWDL_PEER_STATE_CLOSE	1

typedef struct _awdl_sync_state_changed {
	uint8	new_role;
	struct ether_addr master;
} awdl_sync_state_changed_t;

typedef struct _awdl_sync_state {
	uint8	role;
	struct ether_addr master;
	uint8	PAD;
	uint32 continuous_election_enable;
} awdl_sync_state_t;

typedef struct _awdl_aw_ap_alignment {
	uint32	enabled;
	int32	offset;
	uint32	align_on_dtim;
} awdl_aw_ap_alignment_t;

typedef struct _awdl_peer_stats {
	uint32 version;
	struct ether_addr address;
	uint8 clear;
	int8 rssi;
	int8 avg_rssi;
	uint8 txRate;
	uint8 rxRate;
	uint8 PAD;
	uint32 numTx;
	uint32 numTxRetries;
	uint32 numTxFailures;
} awdl_peer_stats_t;

#define MAX_NUM_AWDL_KEYS 4
typedef struct _awdl_aes_key {
	uint32 version;
	int32 enable;
	struct ether_addr awdl_peer;
	uint8 keys[MAX_NUM_AWDL_KEYS][16];
	uint8 PAD[2];
} awdl_aes_key_t;

#include <packed_section_start.h>
typedef BWL_PRE_PACKED_STRUCT struct awdl_scan_event_data {
	uint8	scan_usage;		/**< Kind of scan in progress */
	uint8	nscan_chans;		/**< number of channels to be scanned */
	uint8	ncached_chans;		/**< number of cached channels */
	uint8	flags;
	uint8	chan_list[1];		/**< List of cached channels followed by
	                                 * channels to be scanned
	                                 */
} BWL_POST_PACKED_STRUCT awdl_scan_event_data_t;

/* Flags */
/*	bit-0 : Used to indicate if the flags/pad is valid
*	bit-1 : Slice-0 is blocked on scan.
*	bit-2 : Slice-1 is blocked on scan.
*	bit-7 : reserved
*/

#define AWDL_SCAN_EVT_DATA_FLAGS_VALID	(1<<0)
#define AWDL_SCAN_EVT_DATA_FLAGS_SLICE0_BLOCKED	(1<<1)
#define AWDL_SCAN_EVT_DATA_FLAGS_SLICE1_BLOCKED	(1<<2)

#include <packed_section_end.h>

typedef struct adv_struct_norm {
	uint16	aw_counter;		/**< avail window counter */
	uint16	tx_counter;
	uint16	tx_delay;		/**< ts_hw - ts_fw */
	uint16	period_tu;
	uint16	aw_cmn_length;
	uint16	aw_ext_length;
	uint16	aw_period;
	uint16	chan_seq0;
	int8	rssi;			/**< averaged RSSI value */
	uint8	guard_time;
	uint8	presence_mode;
	uint8	age;			/**< age of this record */
	uint8	dist_top;		/**< Distance from Top */
	uint8 	PAD[3];
	uint32  ms;				/**< time at which we received the AF */
	uint32	self_metrics;		/**< election metric */
	uint32	top_master_metrics;
	struct ether_addr	top_master;	/**< Top Master address */
	struct ether_addr	addr;
	struct rssi_struct	rssi_win;	/**< RSSI values */
} adv_struct_norm_t;

/* AWDL configuration/operation flags */
#define AWDL_CONFIG_NON_AWDL_INTERFACE_FLOW_CONTROL	0x00000001
#define AWDL_CONFIG_AWDL_INTERFACE_UPDATE		0x00000002
/* While scanning configure channel from chan seq */
#define AWDL_CONFIG_CHANNEL_HOP_FROM_CHAN_SEQ		0x00000004
#define AWDL_CONFIG_TX_OFF_BEFORE_INFRA_BEACON		0x00000008
#define AWDL_CONFIG_SUPP_PSFTX_PREAW			0x00000010
#define AWDL_CONFIG_SUPP_PSFTX_NOACK                    0x00000020
/*
 * awdl ranging
 * all the fields with multple bytes are in the little Endian order
 */

/* Bit defines for global flags */
#define AWDL_RANGING_ENABLE		(1<<0)	/**< Global enable bit */
#define AWDL_RANGING_RESPOND		(1<<1)	/**< Enable responding to peer's range req */
#define AWDL_RANGING_RANGED		(1<<2)	/**< V2: Report to host if ranged as target */

#include <packed_section_start.h>
typedef BWL_PRE_PACKED_STRUCT struct awdl_ranging_config {
	uint16 flags;
	uint8 sounding_count;		/**< self initiated ranging: number of probes per peer */
	uint8 reserved;
	struct ether_addr allow_mac;
					/**< peer initiated ranging: the allowed peer mac
					 * address, a unicast (for one peer) or
					 * a broadcast for all. Setting it to all zeros
					 * means responding to none,same as not setting
					 * the flag bit AWDL_RANGING_RESPOND
					 */
} BWL_POST_PACKED_STRUCT awdl_ranging_config_t;
#include <packed_section_end.h>

/* list of peers for self initiated ranging */
/* Bit defines for per peer flags */
#define AWDL_RANGING_REPORT (1<<0)	/**< V2: Enable reporting range to target */
#define AWDL_SEQ_EN (1<<1)

#include <packed_section_start.h>
typedef BWL_PRE_PACKED_STRUCT struct awdl_ranging_peer {
	chanspec_t ranging_chanspec; /**< desired chanspec for this peer */
	uint16 flags;                /**< per peer flags, report or not */
	struct ether_addr ea;        /**< peer MAC address */
} BWL_POST_PACKED_STRUCT awdl_ranging_peer_t;
#include <packed_section_end.h>

#include <packed_section_start.h>
typedef BWL_PRE_PACKED_STRUCT struct awdl_ranging_list {
	uint8 count;			/**< number of MAC addresses */
	uint8 num_peers_done;		/**< host set to 0, when read, shows number of peers
					 * completed, success or fail
					 */
	uint8 num_aws;			/**< time period to do the ranging, specified in aws */
	awdl_ranging_peer_t rp[1];	/**< variable length array of peers */
} BWL_POST_PACKED_STRUCT awdl_ranging_list_t;
#include <packed_section_end.h>

/* ranging results, a list for self initiated ranging and one for peer initiated ranging */
/* There will be one structure for each peer */
#define AWDL_RANGING_STATUS_SUCCESS		1
#define AWDL_RANGING_STATUS_FAIL		2
#define AWDL_RANGING_STATUS_TIMEOUT		3
#define AWDL_RANGING_STATUS_ABORT		4 /**< with partial results if sounding count > 0 */
#include <packed_section_start.h>
typedef BWL_PRE_PACKED_STRUCT struct awdl_ranging_result {
	uint8 status;			/**< 1: Success, 2: Fail 3: Timeout 4: Aborted */
	uint8 sounding_count;		/**< number of measurements completed (0 = failure) */
	struct ether_addr ea;		/**< peer MAC address */
	chanspec_t ranging_chanspec;	/**< Chanspec where the ranging was done */
	uint32 timestamp;		/**< 32bits of the TSF timestamp ranging was completed at */
	uint32 distance;		/**< mean distance in meters expressed as Q4 number.
					 * Only valid when sounding_count > 0. Examples:
					 * 0x08 = 0.5m
					 * 0x10 = 1m
					 * 0x18 = 1.5m
					 * set to 0xffffffff to indicate invalid number
					 */
	int32 rtt_var;			/**< standard deviation in 10th of ns of RTTs measured.
					 * Only valid when sounding_count > 0
					 */
} BWL_POST_PACKED_STRUCT awdl_ranging_result_t;
#include <packed_section_end.h>
#define AWDL_RANGING_TYPE_HOST	1
#define AWDL_RANGING_TYPE_PEER	2

#include <packed_section_start.h>
typedef BWL_PRE_PACKED_STRUCT struct awdl_ranging_event_data {
	uint8 type;			/**< 1: Result of host initiated ranging */
					/* V2: 2: Result of peer initiated ranging */
	uint8 reserved;
	uint8 success_count;		/**< number of peers completed successfully */
	uint8 count;			/**< number of peers in the list */
	awdl_ranging_result_t rr[1];	/**< variable array of ranging peers */
} BWL_POST_PACKED_STRUCT awdl_ranging_event_data_t;
typedef BWL_PRE_PACKED_STRUCT struct awdl_ftm_ranging_config {
	uint16 flags;		/* config flags */
	uint8 num_aws;	/* time period to do the ranging, specified in aws */
} BWL_POST_PACKED_STRUCT awdl_ftm_ranging_config_t;
#include <packed_section_end.h>

/** awdl event config bit mask definitions */
#define AWDL_EVENT_AW_EXT	0x01
#define AWDL_EVENT_RANGING	0x02
#define AWDL_RANGING_MAX_PEERS  8

/* awdl peer as a DFS Proxy */
#define AWDL_DFSP_CFG_VERSION		1
#define AWDL_DFSP_FLAGS_ENAB		0x1
typedef struct awdl_dfsp_cfg {
	uint16 version;
	uint16 len;
	uint16 flags;			/**< bit 1 to enable/disable the feature */
	uint16 max_bcn_miss_dur;        /**< maximum beacon miss duration before ceasing data tx */
	uint8 mcsp_ttl;                 /**< remaining number of hops allowed for pcsa message */
	uint8 bcsa_cnt;                 /**< repeat numbers of broadcast CSA */
	chanspec_t mon_chan;            /**< passive monitoring channel spec */
	struct ether_addr mon_bssid;    /**< broadcast means monitoring all */
	uint16 max_bcn_miss_dur_af;	/**< maximum beacon miss duration before ceasing AF tx */
} awdl_dfsp_cfg_t;

#define AWDL_DFSP_UCSA_VERSION		1
typedef struct awdl_dfsp_ucsa {
	uint16 version;
	uint16 len;
	struct ether_addr address;
	uint8 enable;
	uint8 retry_cnt;		/**< just in case host needs to control the value */
} awdl_dfsp_ucsa_t;

typedef struct awdl_dfsp_ucsa_tbl {
	uint8 tbl_num;
	uint8 tbl[];
} awdl_dfsp_ucsa_tbl_t;

#endif /* WLAWDL */
#endif /* LINUX_POSTMOGRIFY_REMOVAL */

/*
 * ptk_start: iovar to start 4-way handshake for secured ranging
*/

/* ptk negotiation security type - determines negotiation parameters */
typedef enum {
	WL_PTK_START_SEC_TYPE_PMK = 1
} wl_ptk_start_sec_type_t;

/* ptk negotiation role */
typedef enum {
	ROLE_NONE	= 0x0,
	ROLE_AUTH	= 0x1,
	ROLE_SUP	= 0x2,
	ROLE_STATIC	= 0x3,
	ROLE_INVALID	= 0xff,
	WL_PTK_START_ROLE_NONE = ROLE_NONE,
	WL_PTK_START_ROLE_AUTH = ROLE_AUTH,
	WL_PTK_START_ROLE_SUP = ROLE_SUP,
	WL_PTK_START_ROLE_STATIC = ROLE_STATIC,
	WL_PTK_START_ROLE_INVALID = ROLE_INVALID
} wl_ptk_start_role_t;

typedef struct wl_ptk_start_tlv {
	uint16 id;
	uint16 len;
	uint8 data[1];
} wl_ptk_start_tlv_t;

typedef enum {
	WL_PTK_START_TLV_PMK	= 1	/* uint8[] */
} wl_ptk_start_tlv_type;

typedef enum {
	WL_PTK_START_FLAG_NO_DATA_PROT	= 1,	/* data frame protection disabled */
	WL_PTK_START_FLAG_GEN_FTM_TPK	= 2	/* Generate FTM Toast/Seq Protection Key */
} wl_ptk_start_flags_t;

typedef struct wl_ptk_start_iov {
	uint16 version;
	uint16 len;				/* length of entire iov from version */
	wl_ptk_start_flags_t flags;
	wl_ptk_start_sec_type_t sec_type;
	wl_ptk_start_role_t role;
	struct ether_addr peer_addr;
	uint16 pad;				/* reserved/32 bit alignment */
	wl_ptk_start_tlv_t tlvs[1];
} wl_ptk_start_iov_t;

/*
 * Dongle pattern matching filter.
 */

#define MAX_WAKE_PACKET_CACHE_BYTES 128 /**< Maximum cached wake packet */

#define MAX_WAKE_PACKET_BYTES	    (DOT11_A3_HDR_LEN +			    \
				     DOT11_QOS_LEN +			    \
				     sizeof(struct dot11_llc_snap_header) + \
				     ETHER_MAX_DATA)

typedef struct pm_wake_packet {
	uint32	status;		/**< Is the wake reason a packet (if all the other field's valid) */
	uint32	pattern_id;	/**< Pattern ID that matched */
	uint32	original_packet_size;
	uint32	saved_packet_size;
	uint8	packet[MAX_WAKE_PACKET_CACHE_BYTES];
} pm_wake_packet_t;

/* Packet filter types. Currently, only pattern matching is supported. */
typedef enum wl_pkt_filter_type {
	WL_PKT_FILTER_TYPE_PATTERN_MATCH=0,       /**< Pattern matching filter */
	WL_PKT_FILTER_TYPE_MAGIC_PATTERN_MATCH=1, /**< Magic packet match */
	WL_PKT_FILTER_TYPE_PATTERN_LIST_MATCH=2,  /**< A pattern list (match all to match filter) */
	WL_PKT_FILTER_TYPE_ENCRYPTED_PATTERN_MATCH=3, /**< SECURE WOWL magic / net pattern match */
	WL_PKT_FILTER_TYPE_APF_MATCH=4, /* Android packet filter match */
	WL_PKT_FILTER_TYPE_PATTERN_MATCH_TIMEOUT=5, /* Pattern matching filter with timeout event */
	WL_PKT_FILTER_TYPE_IMMEDIATE_PATTERN_MATCH=6, /* Immediately pattern matching filter */
	WL_PKT_FILTYER_TYPE_MAX = 7,	/* Pkt filter type MAX */
} wl_pkt_filter_type_t;

#define WL_PKT_FILTER_TYPE wl_pkt_filter_type_t

/* String mapping for types that may be used by applications or debug */
#define WL_PKT_FILTER_TYPE_NAMES \
	{ "PATTERN", WL_PKT_FILTER_TYPE_PATTERN_MATCH },	\
	{ "MAGIC",   WL_PKT_FILTER_TYPE_MAGIC_PATTERN_MATCH },	\
	{ "PATLIST", WL_PKT_FILTER_TYPE_PATTERN_LIST_MATCH },	\
	{ "SECURE WOWL", WL_PKT_FILTER_TYPE_ENCRYPTED_PATTERN_MATCH },	\
	{ "APF", WL_PKT_FILTER_TYPE_APF_MATCH }, \
	{ "PATTERN TIMEOUT", WL_PKT_FILTER_TYPE_PATTERN_MATCH_TIMEOUT }, \
	{ "IMMEDIATE", WL_PKT_FILTER_TYPE_IMMEDIATE_PATTERN_MATCH }

/** Secured WOWL packet was encrypted, need decrypted before check filter match */
typedef struct wl_pkt_decrypter {
	uint8* (*dec_cb)(void* dec_ctx, const void *sdu, int sending);
	void*  dec_ctx;
} wl_pkt_decrypter_t;

/**
 * Pattern matching filter. Specifies an offset within received packets to
 * start matching, the pattern to match, the size of the pattern, and a bitmask
 * that indicates which bits within the pattern should be matched.
 */
typedef struct wl_pkt_filter_pattern {
	uint32	offset;		/**< Offset within received packet to start pattern matching.
				 * Offset '0' is the first byte of the ethernet header.
				 */
	uint32	size_bytes;	/**< Size of the pattern.  Bitmask must be the same size. */
	uint8   mask_and_pattern[]; /**< Variable length mask and pattern data.  mask starts
				      * at offset 0.  Pattern immediately follows mask. for
				      * secured pattern, put the descrypter pointer to the
				      * beginning, mask and pattern postponed correspondingly
				      */
} wl_pkt_filter_pattern_t;

/** A pattern list is a numerically specified list of modified pattern structures. */
typedef struct wl_pkt_filter_pattern_listel {
	uint16 rel_offs;	/**< Offset to begin match (relative to 'base' below) */
	uint16 base_offs;	/**< Base for offset (defined below) */
	uint16 size_bytes;	/**< Size of mask/pattern */
	uint16 match_flags;	/**< Addition flags controlling the match */
	uint8  mask_and_data[]; /**< Variable length mask followed by data, each size_bytes */
} wl_pkt_filter_pattern_listel_t;

typedef struct wl_pkt_filter_pattern_list {
	uint8 list_cnt;		/**< Number of elements in the list */
	uint8 PAD1[1];		/**< Reserved (possible version: reserved) */
	uint16 totsize;		/**< Total size of this pattern list (includes this struct) */
	uint8 patterns[]; /**< Variable number of wl_pkt_filter_pattern_listel_t elements */
} wl_pkt_filter_pattern_list_t;

typedef struct wl_apf_program {
	uint16 version;
	uint16 instr_len;	/* number of instruction blocks */
	uint32 inst_ts;		/* program installation timestamp */
	uint8 instrs[];	/* variable length instructions */
} wl_apf_program_t;

typedef struct wl_pkt_filter_pattern_timeout {
	uint32	offset;	/* Offset within received packet to start pattern matching.
					 * Offset '0' is the first byte of the ethernet header.
					 */
	uint32	size_bytes;	/* Size of the pattern. Bitmask must be the same size. */
	uint32	timeout;	/* Timeout(seconds) */
	uint8	mask_and_pattern[]; /* Variable length mask and pattern data.
								 * mask starts at offset 0. Pattern
								 * immediately follows mask.
								*/
} wl_pkt_filter_pattern_timeout_t;

/** IOVAR "pkt_filter_add" parameter. Used to install packet filters. */
typedef struct wl_pkt_filter {
	uint32	id;		/**< Unique filter id, specified by app. */
	uint32	type;		/**< Filter type (WL_PKT_FILTER_TYPE_xxx). */
	uint32	negate_match;	/**< Negate the result of filter matches */
	union {			/* Filter definitions */
		wl_pkt_filter_pattern_t pattern;	/**< Pattern matching filter */
		wl_pkt_filter_pattern_list_t patlist; /**< List of patterns to match */
		wl_apf_program_t apf_program; /* apf program */
		wl_pkt_filter_pattern_timeout_t pattern_timeout; /* Pattern timeout event filter */
	} u;
	/* XXX Do NOT add structure members after the filter definitions, since they
	 * may include variable length arrays.
	 */
} wl_pkt_filter_t;

/** IOVAR "tcp_keep_set" parameter. Used to install tcp keep_alive stuff. */
typedef struct wl_tcp_keep_set {
	uint32	val1;
	uint32	val2;
} wl_tcp_keep_set_t;

#define WL_PKT_FILTER_FIXED_LEN		  OFFSETOF(wl_pkt_filter_t, u)
#define WL_PKT_FILTER_PATTERN_FIXED_LEN	  OFFSETOF(wl_pkt_filter_pattern_t, mask_and_pattern)
#define WL_PKT_FILTER_PATTERN_LIST_FIXED_LEN OFFSETOF(wl_pkt_filter_pattern_list_t, patterns)
#define WL_PKT_FILTER_PATTERN_LISTEL_FIXED_LEN	\
			OFFSETOF(wl_pkt_filter_pattern_listel_t, mask_and_data)
#define WL_PKT_FILTER_PATTERN_TIMEOUT_FIXED_LEN	\
			OFFSETOF(wl_pkt_filter_pattern_timeout_t, mask_and_pattern)

#define WL_APF_INTERNAL_VERSION	1
#define WL_APF_PROGRAM_MAX_SIZE (2 * 1024)
#define WL_APF_PROGRAM_FIXED_LEN	OFFSETOF(wl_apf_program_t, instrs)
#define WL_APF_PROGRAM_LEN(apf_program)	\
	((apf_program)->instr_len * sizeof((apf_program)->instrs[0]))
#define WL_APF_PROGRAM_TOTAL_LEN(apf_program)	\
	(WL_APF_PROGRAM_FIXED_LEN + WL_APF_PROGRAM_LEN(apf_program))

/** IOVAR "pkt_filter_enable" parameter. */
typedef struct wl_pkt_filter_enable {
	uint32	id;		/**< Unique filter id */
	uint32	enable;		/**< Enable/disable bool */
} wl_pkt_filter_enable_t;

/** IOVAR "pkt_filter_list" parameter. Used to retrieve a list of installed filters. */
typedef struct wl_pkt_filter_list {
	uint32	num;		/**< Number of installed packet filters */
	uint8	filter[];	/**< Variable array of packet filters. */
} wl_pkt_filter_list_t;

#define WL_PKT_FILTER_LIST_FIXED_LEN	  OFFSETOF(wl_pkt_filter_list_t, filter)

/** IOVAR "pkt_filter_stats" parameter. Used to retrieve debug statistics. */
typedef struct wl_pkt_filter_stats {
	uint32	num_pkts_matched;	/**< # filter matches for specified filter id */
	uint32	num_pkts_forwarded;	/**< # packets fwded from dongle to host for all filters */
	uint32	num_pkts_discarded;	/**< # packets discarded by dongle for all filters */
} wl_pkt_filter_stats_t;

/** IOVAR "pkt_filter_ports" parameter.  Configure TCP/UDP port filters. */
typedef struct wl_pkt_filter_ports {
	uint8 version;		/**< Be proper */
	uint8 reserved;		/**< Be really proper */
	uint16 count;		/**< Number of ports following */
	/* End of fixed data */
	uint16 ports[1];	/**< Placeholder for ports[<count>] */
} wl_pkt_filter_ports_t;

#define WL_PKT_FILTER_PORTS_FIXED_LEN	OFFSETOF(wl_pkt_filter_ports_t, ports)

#define WL_PKT_FILTER_PORTS_VERSION	0
#if defined(WL_PKT_FLTR_EXT) && !defined(WL_PKT_FLTR_EXT_DISABLED)
#define WL_PKT_FILTER_PORTS_MAX	256
#else
#define WL_PKT_FILTER_PORTS_MAX	128
#endif /* WL_PKT_FLTR_EXT && !WL_PKT_FLTR_EXT_DISABLED */

#define RSN_REPLAY_LEN 8
typedef struct _gtkrefresh {
	uint8	KCK[RSN_KCK_LENGTH];
	uint8	KEK[RSN_KEK_LENGTH];
	uint8	ReplayCounter[RSN_REPLAY_LEN];
} gtk_keyinfo_t, *pgtk_keyinfo_t;

/** Sequential Commands ioctl */
typedef struct wl_seq_cmd_ioctl {
	uint32 cmd;		/**< common ioctl definition */
	uint32 len;		/**< length of user buffer */
} wl_seq_cmd_ioctl_t;

#define WL_SEQ_CMD_ALIGN_BYTES	4

/**
 * These are the set of get IOCTLs that should be allowed when using
 * IOCTL sequence commands. These are issued implicitly by wl.exe each time
 * it is invoked. We never want to buffer these, or else wl.exe will stop working.
 */
#define WL_SEQ_CMDS_GET_IOCTL_FILTER(cmd) \
	(((cmd) == WLC_GET_MAGIC)		|| \
	 ((cmd) == WLC_GET_VERSION)		|| \
	 ((cmd) == WLC_GET_AP)			|| \
	 ((cmd) == WLC_GET_INSTANCE))

#define MAX_PKTENG_SWEEP_STEPS 40
typedef struct wl_pkteng {
	uint32 flags;
	uint32 delay;			/**< Inter-packet delay */
	uint32 nframes;			/**< Number of frames */
	uint32 length;			/**< Packet length */
	uint8  seqno;			/**< Enable/disable sequence no. */
	struct ether_addr dest;		/**< Destination address */
	struct ether_addr src;		/**< Source address */
	uint8  sweep_steps;		/**< Number of sweep power */
	uint8  PAD[2];
} wl_pkteng_t;

/* IOVAR pkteng_sweep_counters response structure */
#define WL_PKTENG_SWEEP_COUNTERS_VERSION    1
typedef struct wl_pkteng_sweep_ctrs {
	uint16 version;			/**< Version - 1 */
	uint16 size;			/**< Complete Size including sweep_counters */
	uint16 sweep_steps;		/**< Number of steps */
	uint16 PAD;
	uint16 sweep_counter[];		/**< Array of frame counters */
} wl_pkteng_sweep_ctrs_t;

#define WL_PKTENG_RU_FILL_VER_1		1
// struct for ru packet engine
typedef struct wl_pkteng_ru {
	uint16 version;		/* ver is 1 */
	uint16 length;		/* size of complete structure */
	uint8 bw;			/* bandwidth info */
	uint8 ru_alloc_val;		/* ru allocation index number */
	uint8 mcs_val;			/* mcs allocated value */
	uint8 nss_val;			/* num of spatial streams */
	uint32 num_bytes;		/* approx num of bytes to calculate other required params */
	uint8 cp_ltf_val ;		/* GI and LTF symbol size */
	uint8 he_ltf_symb ;		/* num of HE-LTF symbols */
	uint8 stbc;			/* STBC support */
	uint8 coding_val;		/* BCC/LDPC coding support */
	uint8 pe_category;	/* PE duration 0/8/16usecs  */
	uint8 dcm;			/* dual carrier modulation */
	uint8 mumimo_ltfmode; /* ltf mode */
	uint8 tgt_rssi;			/* target rssi value in encoded format */
//	uint8 PAD[1];		/* pad bytes to make structure occupy 4 byte aligned */
} wl_pkteng_ru_fill_t;
typedef struct wl_trig_frame_info {
	/* Structure versioning and structure length params */
	uint16 version;
	uint16 length;
	/* Below params are the fields related to trigger frame contents */
	/* Common Info Params Figure 9-52d - 11ax Draft 1.1 */
	uint16 lsig_len;
	uint16 trigger_type;
	uint16 cascade_indication;
	uint16 cs_req;
	uint16 bw;
	uint16 cp_ltf_type;
	uint16 mu_mimo_ltf_mode;
	uint16 num_he_ltf_syms;
	uint16 stbc;
	uint16 ldpc_extra_symb;
	uint16 ap_tx_pwr;
	uint16 afactor;
	uint16 pe_disambiguity;
	uint16 spatial_resuse;
	uint16 doppler;
	uint16 he_siga_rsvd;
	uint16 cmn_info_rsvd;
	/* User Info Params Figure 9-52e - 11ax Draft 1.1 */
	uint16 aid12;
	uint16 ru_alloc;
	uint16 coding_type;
	uint16 mcs;
	uint16 dcm;
	uint16 ss_alloc;
	uint16 tgt_rssi;
	uint16 usr_info_rsvd;
} wl_trig_frame_info_t;

typedef struct wl_pkteng_stats {
	uint32 lostfrmcnt;		/**< RX PER test: no of frames lost (skip seqno) */
	int32 rssi;			/**< RSSI */
	int32 snr;			/**< signal to noise ratio */
	uint16 rxpktcnt[NUM_80211_RATES+1];
	uint8 rssi_qdb;			/**< qdB portion of the computed rssi */
	uint8  version;
} wl_pkteng_stats_t;

typedef struct wl_txcal_params {
	wl_pkteng_t pkteng;
	uint8 gidx_start;
	int8 gidx_step;
	uint8 gidx_stop;
	uint8  PAD;
} wl_txcal_params_t;

#ifndef LINUX_POSTMOGRIFY_REMOVAL

typedef struct wl_sslpnphy_papd_debug_data {
	uint8 psat_pwr;
	uint8 psat_indx;
	uint8 final_idx;
	uint8 start_idx;
	int32 min_phase;
	int32 voltage;
	int8 temperature;
	uint8  PAD[3];
} wl_sslpnphy_papd_debug_data_t;
typedef struct wl_sslpnphy_debug_data {
	int16 papdcompRe [64];
	int16 papdcompIm [64];
} wl_sslpnphy_debug_data_t;
typedef struct wl_sslpnphy_spbdump_data {
	uint16 tbl_length;
	int16 spbreal[256];
	int16 spbimg[256];
} wl_sslpnphy_spbdump_data_t;
typedef struct wl_sslpnphy_percal_debug_data {
	uint32 cur_idx;
	uint32 tx_drift;
	uint8 prev_cal_idx;
	uint8  PAD[3];
	uint32 percal_ctr;
	int32 nxt_cal_idx;
	uint32 force_1idxcal;
	uint32 onedxacl_req;
	int32 last_cal_volt;
	int8 last_cal_temp;
	uint8  PAD[3];
	uint32 vbat_ripple;
	uint32 exit_route;
	int32 volt_winner;
} wl_sslpnphy_percal_debug_data_t;
#endif /* LINUX_POSTMOGRIFY_REMOVAL */

typedef enum {
	wowl_pattern_type_bitmap = 0,
	wowl_pattern_type_arp,
	wowl_pattern_type_na
} wowl_pattern_type_t;

typedef struct wl_wowl_pattern {
	uint32		    masksize;		/**< Size of the mask in #of bytes */
	uint32		    offset;		/**< Pattern byte offset in packet */
	uint32		    patternoffset;	/**< Offset of start of pattern in the structure */
	uint32		    patternsize;	/**< Size of the pattern itself in #of bytes */
	uint32		    id;			/**< id */
	uint32		    reasonsize;		/**< Size of the wakeup reason code */
	wowl_pattern_type_t type;		/**< Type of pattern */
	/* Mask follows the structure above */
	/* Pattern follows the mask is at 'patternoffset' from the start */
} wl_wowl_pattern_t;

typedef struct wl_wowl_pattern_list {
	uint32			count;
	wl_wowl_pattern_t	pattern[1];
} wl_wowl_pattern_list_t;

typedef struct wl_wowl_wakeind {
	uint8	pci_wakeind;	/**< Whether PCI PMECSR PMEStatus bit was set */
	uint32	ucode_wakeind;	/**< What wakeup-event indication was set by ucode */
} wl_wowl_wakeind_t;

/** per AC rate control related data structure */
typedef struct wl_txrate_class {
	uint8		init_rate;
	uint8		min_rate;
	uint8		max_rate;
} wl_txrate_class_t;

/** structure for Overlap BSS scan arguments */
typedef struct wl_obss_scan_arg {
	int16	passive_dwell;
	int16	active_dwell;
	int16	bss_widthscan_interval;
	int16	passive_total;
	int16	active_total;
	int16	chanwidth_transition_delay;
	int16	activity_threshold;
} wl_obss_scan_arg_t;

#define WL_OBSS_SCAN_PARAM_LEN	sizeof(wl_obss_scan_arg_t)

/** RSSI event notification configuration. */
typedef struct wl_rssi_event {
	uint32 rate_limit_msec;		/**< # of events posted to application will be limited to
					 * one per specified period (0 to disable rate limit).
					 */
	uint8 num_rssi_levels;		/**< Number of entries in rssi_levels[] below */
	int8 rssi_levels[MAX_RSSI_LEVELS];	/**< Variable number of RSSI levels. An event
						 * will be posted each time the RSSI of received
						 * beacons/packets crosses a level.
						 */
	int8 pad[3];
} wl_rssi_event_t;

#define RSSI_MONITOR_VERSION    1
#define RSSI_MONITOR_STOP       (1 << 0)
typedef struct wl_rssi_monitor_cfg {
	uint8 version;
	uint8 flags;
	int8 max_rssi;
	int8 min_rssi;
} wl_rssi_monitor_cfg_t;

typedef struct wl_rssi_monitor_evt {
	uint8 version;
	int8 cur_rssi;
	uint16 pad;
} wl_rssi_monitor_evt_t;

typedef struct wl_ext_auth_evt {
	wlc_ssid_t ssid;
	struct ether_addr bssid;
	unsigned int key_mgmt_suite;
	int status;
} wl_ext_auth_evt_t;

#ifndef LINUX_POSTMOGRIFY_REMOVAL
/* CCA based channel quality event configuration (ID values for both config and report) */
#define WL_CHAN_QUAL_CCA	0
#define WL_CHAN_QUAL_NF		1
#define WL_CHAN_QUAL_NF_LTE	2
#define WL_CHAN_QUAL_TOTAL	3	/* The total IDs supported in both config and report */
/* Additional channel quality event support in report only (>= 0x100)
 * Notice that uint8 is used in configuration struct wl_chan_qual_metric_t, but uint16 is
 * used for report in struct cca_chan_qual_event_t. So the ID values beyond 8-bit are used
 * for reporting purpose only.
 */
#define WL_CHAN_QUAL_FULL_CCA	(0x100 | WL_CHAN_QUAL_CCA)

#define MAX_CHAN_QUAL_LEVELS	8

typedef struct wl_chan_qual_metric {
	uint8 id;				/**< metric ID */
	uint8 num_levels;               	/**< Number of entries in rssi_levels[] below */
	uint16 flags;
	int16 htol[MAX_CHAN_QUAL_LEVELS];	/**< threshold level array: hi-to-lo */
	int16 ltoh[MAX_CHAN_QUAL_LEVELS];	/**< threshold level array: lo-to-hi */
} wl_chan_qual_metric_t;

typedef struct wl_chan_qual_event {
	uint32 rate_limit_msec;		/**< # of events posted to application will be limited to
					 * one per specified period (0 to disable rate limit).
					 */
	uint16 flags;
	uint16 num_metrics;
	wl_chan_qual_metric_t metric[WL_CHAN_QUAL_TOTAL];	/**< metric array */
} wl_chan_qual_event_t;
#endif /* LINUX_POSTMOGRIFY_REMOVAL */
typedef struct wl_action_obss_coex_req {
	uint8 info;
	uint8 num;
	uint8 ch_list[1];
} wl_action_obss_coex_req_t;

/** IOVar parameter block for small MAC address array with type indicator */
#define WL_IOV_MAC_PARAM_LEN  4

#define WL_IOV_PKTQ_LOG_PRECS 16

#include <packed_section_start.h>
typedef BWL_PRE_PACKED_STRUCT struct {
	uint32 num_addrs;
	uint8  addr_type[WL_IOV_MAC_PARAM_LEN];
	struct ether_addr ea[WL_IOV_MAC_PARAM_LEN];
} BWL_POST_PACKED_STRUCT wl_iov_mac_params_t;
#include <packed_section_end.h>

/** This is extra info that follows wl_iov_mac_params_t */
typedef struct {
	uint32 addr_info[WL_IOV_MAC_PARAM_LEN];
} wl_iov_mac_extra_params_t;

/** Combined structure */
typedef struct {
	wl_iov_mac_params_t params;
	wl_iov_mac_extra_params_t extra_params;
} wl_iov_mac_full_params_t;

/** Parameter block for PKTQ_LOG statistics */
/* XXX NOTE: this structure cannot change! It is exported to wlu as a binary format
 * A new format revision number must be created if the interface changes
 * The latest is v06; previous v01...v04 are no longer supported
*/

#define PKTQ_LOG_COUNTERS_V4 \
	/* packets requested to be stored */ \
	uint32 requested; \
	/* packets stored */ \
	uint32 stored; \
	/* packets saved, because a lowest priority queue has given away one packet */ \
	uint32 saved; \
	/* packets saved, because an older packet from the same queue has been dropped */ \
	uint32 selfsaved; \
	/* packets dropped, because pktq is full with higher precedence packets */ \
	uint32 full_dropped; \
	 /* packets dropped because pktq per that precedence is full */ \
	uint32 dropped; \
	/* packets dropped, in order to save one from a queue of a highest priority */ \
	uint32 sacrificed; \
	/* packets droped because of hardware/transmission error */ \
	uint32 busy; \
	/* packets re-sent because they were not received */ \
	uint32 retry; \
	/* packets retried again (ps pretend) prior to moving power save mode */ \
	uint32 ps_retry; \
	 /* suppressed packet count */ \
	uint32 suppress; \
	/* packets finally dropped after retry limit */ \
	uint32 retry_drop; \
	/* the high-water mark of the queue capacity for packets - goes to zero as queue fills */ \
	uint32 max_avail; \
	/* the high-water mark of the queue utilisation for packets - ('inverse' of max_avail) */ \
	uint32 max_used; \
	 /* the maximum capacity of the queue */ \
	uint32 queue_capacity; \
	/* count of rts attempts that failed to receive cts */ \
	uint32 rtsfail; \
	/* count of packets sent (acked) successfully */ \
	uint32 acked; \
	/* running total of phy rate of packets sent successfully */ \
	uint32 txrate_succ; \
	/* running total of phy 'main' rate */ \
	uint32 txrate_main; \
	/* actual data transferred successfully */ \
	uint32 throughput; \
	/* time difference since last pktq_stats */ \
	uint32 time_delta;

typedef struct {
	PKTQ_LOG_COUNTERS_V4
} pktq_log_counters_v04_t;

typedef struct {
	uint8                num_prec[WL_IOV_MAC_PARAM_LEN];
	pktq_log_counters_v04_t  counters[WL_IOV_MAC_PARAM_LEN][WL_IOV_PKTQ_LOG_PRECS];
	uint32               counter_info[WL_IOV_MAC_PARAM_LEN];
	uint32               pspretend_time_delta[WL_IOV_MAC_PARAM_LEN];
	char                 headings[];
} pktq_log_format_v04_t;

/* v5 */
typedef struct {
	/* packets requested to be stored */ \
	uint32 requested;
	/* packets stored */
	uint32 stored;
	/* packets saved, because a lowest priority queue has given away one packet */
	uint32 saved;
	/* packets saved, because an older packet from the same queue has been dropped */
	uint32 selfsaved;
	/* packets dropped, because pktq is full with higher precedence packets */
	uint32 full_dropped;
	/* packets dropped because pktq per that precedence is full */
	uint32 dropped;
	/* packets dropped, in order to save one from a queue of a highest priority */
	uint32 sacrificed;
	/* packets droped because of hardware/transmission error */
	uint32 busy;
	/* packets re-sent because they were not received */
	uint32 retry;
	/* packets retried again (ps pretend) prior to moving power save mode */
	uint32 ps_retry;
	/* suppressed packet count */
	uint32 suppress;
	/* packets finally dropped after retry limit */
	uint32 retry_drop;
	/* the high-water mark of the queue capacity for packets - goes to zero as queue fills */
	uint32 max_avail;
	/* the high-water mark of the queue utilisation for packets - ('inverse' of max_avail) */
	uint32 max_used;
	/* the maximum capacity of the queue */
	uint32 queue_capacity;
	/* count of rts attempts that failed to receive cts */
	uint32 rtsfail;
	/* count of packets sent (acked) successfully */
	uint32 acked;
	/* running total of phy rate of packets sent successfully */
	uint32 txrate_succ;
	/* running total of phy 'main' rate */
	uint32 txrate_main;
	/* actual data transferred successfully */
	uint32 throughput;
	/* time difference since last pktq_stats */
	uint32 time_delta;
	/** cumulative time to transmit */
	uint32 airtime;
} pktq_log_counters_v05_t;

/* this is the scale factor for PHY rate for V5 */
#define PERF_LOG_RATE_FACTOR_500   500

/* v6 */
#include <packed_section_start.h>
typedef BWL_PRE_PACKED_STRUCT struct {
	/* packets requested to be stored */
	uint32 requested;
	/* packets stored */
	uint32 stored;
	/* packets dropped because pktq per that precedence is full */
	uint32 dropped;
	/* packets droped because of hardware/transmission error */
	uint32 busy;
	/* packets re-sent because they were not received */
	uint32 retry;
	/* packets retried again (ps pretend) prior to moving power save mode */
	uint32 ps_retry;
	/* separate scb queue and common queue counters */
	union {
		/* suppressed packet count */
		uint32 suppress;
		/* packets dropped, because pktq is full with higher precedence packets */
		uint32 full_dropped;
	};
	/* packets finally dropped after retry limit */
	uint32 retry_drop;
	/* the high-water mark of the queue utilisation for packets */
	uint16 max_used;
	/* the maximum capacity of the queue */
	uint16 queue_capacity;
	/* count of rts attempts that failed to receive cts */
	uint32 rtsfail;
	/* count of packets sent (acked) successfully */
	uint32 acked;
	/* running total of phy rate of packets sent successfully */
	uint64 txrate_succ;
	/* separate scb queue and common queue counters */
	union {
		/* running total of phy 'main' rate */
		uint64 txrate_main;
		/* packets saved, because a lowest priority queue has given away one packet */
		uint32 saved;
	};
	/* actual data transferred successfully */
	uint64 throughput;
	/* time difference since last pktq_stats */
	uint64 time_delta;
	/* cumulative time to transmit */
	uint64 airtime;
	/* separate scb queue and common queue counters */
	union {
		/* running total of bandwidth used MHz */
		uint64 bandwidth;
		/* packets saved, because an older packet from the same queue has been dropped */
		uint32 selfsaved;
	};
	/* running count of spatial stream in acked packets */
	uint32 nss[4];
	/* separate scb queue and common queue counters */
	union {
		/* total virtual packets requested in SQS */
		uint32 sqsv_requested;
		/* packets dropped, in order to save one from a queue of a highest priority */
		uint32 sacrificed;
	};
	/* high watermark of packets requested in SQS */
	uint16 sqsv_max_used;
	/* bitfield lowest mcs (bits 0-4), middle mcs (bits 5-9), high mcs (bits 10-15) */
	uint16 mcs_bitfield;
	/* count of short guard usage */
	uint32 sgi_count;
	/* reserved for future use */
	uint32 reserved;
} BWL_POST_PACKED_STRUCT pktq_log_counters_v06_t;

/* this is the scale factor for PHY rate for V6 */
#define PERF_LOG_RATE_FACTOR_100   100

enum {
	/* flags used in response only */
	PKTQ_LOG_PREC_MASK = (1 << 16) - 1,
	PKTQ_LOG_AMPDU_PREC = 1 << 17,
	PKTQ_LOG_AQM = 1 << 18,
	PKTQ_LOG_DBG = 1 << 19,
	PKTQ_LOG_DISPLAY_ETHERADDR = 1 << 20,
	PKTQ_LOG_DISPLAY_TIMESTAMP = 1 << 21,
	PKTQ_LOG_DONGLE = 1 << 22,
	PKTQ_LOG_PS_QUEUE = 1 << 23,
	PKTQ_LOG_SQS = 1 << 24,
	PKTQ_LOG_SCB_QUEUE = 1 << 25,

	PKTQ_LOG_DEVELOPMENT_VERSION = 1 << 29,

	/* flags used in request only */
	PKTQ_LOG_AUTO    =  1 << 31,
	PKTQ_LOG_DEF_PREC =  1 << 30,
	/* no reset can be shared with develop flag, as no reset is only used in the request and
	 * the develop only used in the response
	 */
	PKTQ_LOG_NO_RESET = 1 << 29
};

typedef struct BWL_PRE_PACKED_STRUCT {
	uint64 time_delta;
	union {
		struct {
			uint32  reserved[32];
		} ias;
	};
} BWL_POST_PACKED_STRUCT taf_log_counters_v06_t;

enum {
	MAC_LOG_MU_SU,
	MAC_LOG_MU_VHTMU,
	MAC_LOG_MU_HEMMU,
	MAC_LOG_MU_HEOMU,
	MAC_LOG_MU_HEMOM,
	MAC_LOG_MU_MAX
};

enum {
	MAC_LOG_MU_RU_26,
	MAC_LOG_MU_RU_52,
	MAC_LOG_MU_RU_106,
	MAC_LOG_MU_RU_242,
	MAC_LOG_MU_RU_484,
	MAC_LOG_MU_RU_996,
	MAC_LOG_MU_RU_2x996,
	MAC_LOG_MU_RU_MAX
};

#define MAC_LOG_MU_MIMO_USER_MAX  4

typedef struct BWL_PRE_PACKED_STRUCT {
	uint32 count[MAC_LOG_MU_MAX];
	uint32 ru_count[MAC_LOG_MU_RU_MAX];
	uint32 mimo_count[MAC_LOG_MU_MIMO_USER_MAX];
	uint32 reserved[20];
} BWL_POST_PACKED_STRUCT mac_log_mu_counters_v06_t;

#define PKTQ_LOG_V06_HEADINGS_SIZE   40

typedef struct {
	union {
		pktq_log_counters_v06_t   pktq[WL_IOV_PKTQ_LOG_PRECS];
		taf_log_counters_v06_t    taf[NUMPRIO];
		mac_log_mu_counters_v06_t mu[NUMPRIO];
	};
	char    heading[PKTQ_LOG_V06_HEADINGS_SIZE];
}  mac_log_counters_v06_t;

#define COMPILE_ASSERT(type, arg) extern char __compile_assert_##type[(arg) ? 1 : -1];
/* size check */
COMPILE_ASSERT(pktq_log_counters, sizeof(mac_log_counters_v06_t) ==
	(WL_IOV_PKTQ_LOG_PRECS*sizeof(pktq_log_counters_v06_t))+PKTQ_LOG_V06_HEADINGS_SIZE);
#include <packed_section_end.h>

typedef struct {
	uint8                num_prec[WL_IOV_MAC_PARAM_LEN];
	pktq_log_counters_v05_t  counters[WL_IOV_MAC_PARAM_LEN][WL_IOV_PKTQ_LOG_PRECS];
	uint32               counter_info[WL_IOV_MAC_PARAM_LEN];
	uint32               pspretend_time_delta[WL_IOV_MAC_PARAM_LEN];
	char                 headings[];
} pktq_log_format_v05_t;

typedef struct {
	uint8                num_prec[WL_IOV_MAC_PARAM_LEN];
	uint32               timestamp;
	uint32               counter_info[WL_IOV_MAC_PARAM_LEN];
	uint32               reserved[WL_IOV_MAC_PARAM_LEN];
	union {
		uint32       pspretend_time_delta[WL_IOV_MAC_PARAM_LEN];
		uint32       wme_ac[WL_IOV_MAC_PARAM_LEN];
	};
	mac_log_counters_v06_t  counters[WL_IOV_MAC_PARAM_LEN];
} pktq_log_format_v06_t;

typedef struct {
	uint32               version;
	wl_iov_mac_params_t  params;
	union {
		pktq_log_format_v04_t v04;
		/* wl_iov_mac_extra_params_t is used in request form only, not in response */
		wl_iov_mac_extra_params_t extra_params;
		pktq_log_format_v05_t v05;
		pktq_log_format_v06_t v06;
	} pktq_log;
} wl_iov_pktq_log_t;

/* use a 8K guide limit of iovar max size to check everything can fit */
COMPILE_ASSERT(pktq_log_size, sizeof(wl_iov_pktq_log_t) <= 8192);
#undef COMPILE_ASSERT

typedef struct wl_pfn_macaddr_cfg_0 {
	uint8 version;
	uint8 reserved;
	struct ether_addr macaddr;
} wl_pfn_macaddr_cfg_0_t;
#ifndef LINUX_POSTMOGRIFY_REMOVAL
#define LEGACY1_WL_PFN_MACADDR_CFG_VER 0
#endif /* LINUX_POSTMOGRIFY_REMOVAL */
#define WL_PFN_MAC_OUI_ONLY_MASK      1
#define WL_PFN_SET_MAC_UNASSOC_MASK   2
#ifndef LINUX_POSTMOGRIFY_REMOVAL
#define WL_PFN_RESTRICT_LA_MAC_MASK   4
#define WL_PFN_MACADDR_FLAG_MASK     0x7
#endif /* LINUX_POSTMOGRIFY_REMOVAL */
/** To configure pfn_macaddr */
typedef struct wl_pfn_macaddr_cfg {
	uint8 version;
	uint8 flags;
	struct ether_addr macaddr;
} wl_pfn_macaddr_cfg_t;
#define WL_PFN_MACADDR_CFG_VER 1

/*
 * SCB_BS_DATA iovar definitions start.
 */
#define SCB_BS_DATA_STRUCT_VERSION_v1	1
#define SCB_BS_DATA_STRUCT_VERSION		2

/** The actual counters maintained for each station */
typedef struct {
	/* The following counters are a subset of what pktq_stats provides per precedence. */
	uint64 throughput;     /**< actual data transferred successfully */
	uint64 txrate_succ;    /**< running total of phy rate of packets sent successfully */
	uint64 txrate_main;    /**< running total of phy 'main' rate */
	uint32 retry;          /**< packets re-sent because they were not received */
	uint32 retry_drop;     /**< packets finally dropped after retry limit */
	uint32 rtsfail;        /**< count of rts attempts that failed to receive cts */
	uint32 acked;          /**< count of packets sent (acked) successfully */
	uint32 ru_acked;       /**< count of packets sent (acked) successfully in RU */
	uint32 mu_acked;       /**< count of packets sent (acked) successfully in MU */
	uint32 time_delta;     /**< time difference since last pktq_stats */
	uint32 airtime;        /**< cumulative total medium access delay in useconds */
	uint32 txbw;           /* v2 only : bw in use for successfull packets */
	uint32 txmcs;          /* v2 only : mcs in use for successfull packets */
	uint32 txnss;          /* v2 only : nss in use for successfull packets */
} iov_bs_data_counters_t;

#define RATE_SCALE_FACTOR   100

/** The structure for individual station information. */
#include <packed_section_start.h>
typedef BWL_PRE_PACKED_STRUCT struct {
	struct ether_addr	station_address;	/**< The station MAC address */
	uint16			station_flags;		/**< Bit mask of flags, for future use. */
	iov_bs_data_counters_t	station_counters;	/**< The actual counter values */
} BWL_POST_PACKED_STRUCT iov_bs_data_record_t;
#include <packed_section_end.h>

#include <packed_section_start.h>
typedef BWL_PRE_PACKED_STRUCT struct {
	uint16	structure_version;	/**< Structure version number (for wl/wlu matching) */
	uint16	structure_count;	/**< Number of iov_bs_data_record_t records following */
	iov_bs_data_record_t	structure_record[1];	/**< 0 - structure_count records */
} BWL_POST_PACKED_STRUCT iov_bs_data_struct_t;
#include <packed_section_end.h>

/* Bitmask of options that can be passed in to the iovar. */
enum {
	SCB_BS_DATA_FLAG_NO_RESET = (1<<0)	/**< Do not clear the counters after reading */
};

/*
 * SCB_BS_DATA v1 definitions for back compatibility.
 */

typedef struct {
	/* The following counters are a subset of what pktq_stats provides per precedence. */
	uint32 retry;          /**< packets re-sent because they were not received */
	uint32 retry_drop;     /**< packets finally dropped after retry limit */
	uint32 rtsfail;        /**< count of rts attempts that failed to receive cts */
	uint32 acked;          /**< count of packets sent (acked) successfully */
	uint32 txrate_succ;    /**< running total of phy rate of packets sent successfully */
	uint32 txrate_main;    /**< running total of phy 'main' rate */
	uint32 throughput;     /**< actual data transferred successfully */
	uint32 time_delta;     /**< time difference since last pktq_stats */
	uint32 airtime;        /**< cumulative total medium access delay in useconds */
} iov_bs_data_counters_v1_t;

#include <packed_section_start.h>
typedef BWL_PRE_PACKED_STRUCT struct {
	struct ether_addr	station_address;	/**< The station MAC address */
	uint16			station_flags;		/**< Bit mask of flags, for future use. */
	iov_bs_data_counters_v1_t	station_counters;	/**< The actual counter values */
} BWL_POST_PACKED_STRUCT iov_bs_data_record_v1_t;
#include <packed_section_end.h>

#include <packed_section_start.h>
typedef BWL_PRE_PACKED_STRUCT struct {
	uint16	structure_version;	/**< Structure version number (for wl/wlu matching) */
	uint16	structure_count;	/**< Number of iov_bs_data_record_t records following */
	iov_bs_data_record_v1_t	structure_record[1];	/**< 0 - structure_count records */
} BWL_POST_PACKED_STRUCT iov_bs_data_struct_v1_t;
#include <packed_section_end.h>

/*
 * SCB_BS_DATA iovar definitions end.
 */

/*
 * SCB_RX_REPORT iovar definitions start.
 */
#define SCB_RX_REPORT_DATA_STRUCT_VERSION	2

/** The actual counters maintained for each station */
typedef struct {
	uint64 rxbyte;
	uint64 rxphyrate;
	uint32 rxampdu;
	uint32 rxmpdu;
	uint32 rxholes;
	uint32 rxdup;
	uint32 rxoow;
	uint32 rxretried;
	uint32 rxbw;
	uint32 rxmcs;
	uint32 rxnss;
	uint32 rxampdu_ofdma;
	uint32 rxtones;
} iov_rx_report_counters_t;

/** The structure for individual station information. */
#include <packed_section_start.h>
typedef BWL_PRE_PACKED_STRUCT struct {
	struct ether_addr	station_address;	/**< The station MAC address */
	uint16			station_flags;		/**< Bit mask of flags, for future use. */
	uint32 time_delta;					/* time difference since query */
	iov_rx_report_counters_t station_counters[NUMPRIO]; /* counters per tid */
	int8 rssi;
	int8 pad1;
	int8 pad2;
	int8 pad3;
} BWL_POST_PACKED_STRUCT iov_rx_report_record_t;
#include <packed_section_end.h>

#include <packed_section_start.h>
typedef BWL_PRE_PACKED_STRUCT struct {
	uint16	structure_version;	/**< Structure version number (for wl/wlu matching) */
	uint16	structure_count;	/**< Number of iov_bs_data_record_t records following */
	iov_rx_report_record_t	structure_record[1];	/**< 0 - structure_count records */
} BWL_POST_PACKED_STRUCT iov_rx_report_struct_t;
#include <packed_section_end.h>

/* Bitmask of options that can be passed in to the iovar. */
enum {
	SCB_RX_REPORT_DATA_FLAG_NO_RESET = (1<<0)	/**< Don't clear the counters */
};
/*
 * SCB_RX_REPORT iovar definitions end.
 */

typedef struct wlc_extlog_cfg {
	int32 max_number;
	uint16 module;	/**< bitmap */
	uint8 level;
	uint8 flag;
	uint16 version;
	uint16 PAD;
} wlc_extlog_cfg_t;

typedef struct log_record {
	uint32 time;
	uint16 module;
	uint16 id;
	uint8 level;
	uint8 sub_unit;
	uint8 seq_num;
	uint8 pad;
	int32 arg;
	char  str[MAX_ARGSTR_LEN];
	char  PAD[4-MAX_ARGSTR_LEN%4];
} log_record_t;

typedef struct wlc_extlog_req {
	uint32 from_last;
	uint32 num;
} wlc_extlog_req_t;

typedef struct wlc_extlog_results {
	uint16 version;
	uint16 record_len;
	uint32 num;
	log_record_t logs[1];
} wlc_extlog_results_t;

typedef struct log_idstr {
	uint16	id;
	uint16	flag;
	uint8	arg_type;
	const char	*fmt_str;
} log_idstr_t;

#define FMTSTRF_USER		1

/* flat ID definitions
 * New definitions HAVE TO BE ADDED at the end of the table. Otherwise, it will
 * affect backward compatibility with pre-existing apps
 */
typedef enum {
	FMTSTR_DRIVER_UP_ID = 0,
	FMTSTR_DRIVER_DOWN_ID = 1,
	FMTSTR_SUSPEND_MAC_FAIL_ID = 2,
	FMTSTR_NO_PROGRESS_ID = 3,
	FMTSTR_RFDISABLE_ID = 4,
	FMTSTR_REG_PRINT_ID = 5,
	FMTSTR_EXPTIME_ID = 6,
	FMTSTR_JOIN_START_ID = 7,
	FMTSTR_JOIN_COMPLETE_ID = 8,
	FMTSTR_NO_NETWORKS_ID = 9,
	FMTSTR_SECURITY_MISMATCH_ID = 10,
	FMTSTR_RATE_MISMATCH_ID = 11,
	FMTSTR_AP_PRUNED_ID = 12,
	FMTSTR_KEY_INSERTED_ID = 13,
	FMTSTR_DEAUTH_ID = 14,
	FMTSTR_DISASSOC_ID = 15,
	FMTSTR_LINK_UP_ID = 16,
	FMTSTR_LINK_DOWN_ID = 17,
	FMTSTR_RADIO_HW_OFF_ID = 18,
	FMTSTR_RADIO_HW_ON_ID = 19,
	FMTSTR_EVENT_DESC_ID = 20,
	FMTSTR_PNP_SET_POWER_ID = 21,
	FMTSTR_RADIO_SW_OFF_ID = 22,
	FMTSTR_RADIO_SW_ON_ID = 23,
	FMTSTR_PWD_MISMATCH_ID = 24,
	FMTSTR_FATAL_ERROR_ID = 25,
	FMTSTR_AUTH_FAIL_ID = 26,
	FMTSTR_ASSOC_FAIL_ID = 27,
	FMTSTR_IBSS_FAIL_ID = 28,
	FMTSTR_EXTAP_FAIL_ID = 29,
	FMTSTR_MAX_ID
} log_fmtstr_id_t;

/** 11k Neighbor Report element (unversioned, deprecated) */
typedef struct nbr_element {
	uint8 id;
	uint8 len;
	struct ether_addr bssid;
	uint32 bssid_info;
	uint8 reg;
	uint8 channel;
	uint8 phytype;
	uint8 addtype; /* static for manual add or dynamic if auto-learning of neighbors */
	uint8 bss_trans_preference;
	uint8 pad[3];	/* 4 byte alignment */
} nbr_element_t;
#define NBR_ADD_STATIC 0
#define NBR_ADD_DYNAMIC 1

#define WL_RRM_NBR_RPT_VER		1
/** 11k Neighbor Report element */
typedef struct nbr_rpt_elem {
	uint32 version;
	uint8 id;
	uint8 len;
	struct ether_addr bssid;
	uint32 bssid_info;
	uint8 reg;
	uint8 channel;
	uint8 phytype;
	uint8 addtype; /* static for manual add or dynamic if auto-learning of neighbors */
	wlc_ssid_t ssid;
	chanspec_t chanspec;
	uint8 bss_trans_preference;
	uint8 flags;
} nbr_rpt_elem_t;
#define WL_RRM_NBR_RPT_LCI_FLAG     0x1
#define WL_RRM_NBR_RPT_CIVIC_FLAG	0x2

/* Neighbor Report List */
typedef struct nbr_list {
	uint16 version;			/* structure version */
	uint16 count;			/* No. of NBRs returned */
	uint16 fixed_length;		/* fixed length of data fields */
	uint16 length;			/* total length of data fields */
	uint8 nbr_list_elem[];		/* Returned list */
} nbr_list_t;

typedef enum event_msgs_ext_command {
	EVENTMSGS_NONE		=	0,
	EVENTMSGS_SET_BIT	=	1,
	EVENTMSGS_RESET_BIT	=	2,
	EVENTMSGS_SET_MASK	=	3
} event_msgs_ext_command_t;

#define EVENTMSGS_VER 1
#define EVENTMSGS_EXT_STRUCT_SIZE	OFFSETOF(eventmsgs_ext_t, mask[0])

/* len-	for SET it would be mask size from the application to the firmware */
/*		for GET it would be actual firmware mask size */
/* maxgetsize -	is only used for GET. indicate max mask size that the */
/*				application can read from the firmware */
typedef struct eventmsgs_ext
{
	uint8	ver;
	uint8	command;
	uint8	len;
	uint8	maxgetsize;
	uint8	mask[1];
} eventmsgs_ext_t;

#include <packed_section_start.h>
typedef BWL_PRE_PACKED_STRUCT struct pcie_bus_tput_params {
	/** no of host dma descriptors programmed by the firmware before a commit */
	uint16		max_dma_descriptors;

	uint16		host_buf_len; /**< length of host buffer */
	dmaaddr_t	host_buf_addr; /**< physical address for bus_throughput_buf */
} BWL_POST_PACKED_STRUCT pcie_bus_tput_params_t;
#include <packed_section_end.h>

typedef struct pcie_bus_tput_stats {
	uint16		time_taken; /**< no of secs the test is run */
	uint16		nbytes_per_descriptor; /**< no of bytes of data dma ed per descriptor */

	/** no of desciptors for which dma is sucessfully completed within the test time */
	uint32		count;
} pcie_bus_tput_stats_t;

#define HOST_WAKEUP_DATA_VER 1
#include <packed_section_start.h>
/* Bus interface host wakeup data */
typedef BWL_PRE_PACKED_STRUCT struct wl_host_wakeup_data {
	uint16 ver;
	uint16 len;
	uchar data[1];	/* wakeup data */
} BWL_POST_PACKED_STRUCT wl_host_wakeup_data_t;
#include <packed_section_end.h>

#define HOST_WAKEUP_DATA_VER_2 2
#include <packed_section_start.h>
/* Bus interface host wakeup data */
typedef BWL_PRE_PACKED_STRUCT struct wl_host_wakeup_data_v2 {
	uint16 ver;
	uint16 len;
	uint32 gpio_toggle_time; /* gpio toggle time in ms */
	uchar data[1];	/* wakeup data */
} BWL_POST_PACKED_STRUCT wl_host_wakeup_data_v2_t;
#include <packed_section_end.h>

typedef struct keepalives_max_idle {
	uint16  keepalive_count;        /**< nmbr of keepalives per bss_max_idle period */
	uint8   mkeepalive_index;       /**< mkeepalive_index for keepalive frame to be used */
	uint8   PAD;			/**< to align next field */
	uint16  max_interval;           /**< seconds */
} keepalives_max_idle_t;

#define PM_IGNORE_BCMC_PROXY_ARP (1 << 0)
#define PM_IGNORE_BCMC_ALL_DMS_ACCEPTED (1 << 1)

/* ##### HMAP section ##### */
#define PCIE_MAX_HMAP_WINDOWS 8
#define PCIE_HMAPTEST_VERSION 1
#define HMAPTEST_VAL_INVALIDMASK 1
#define HMAPTEST_VAL_WRITEMASK 2
#define HMAPTEST_ACCESS_TYPE_MASK 0b1100
#define HMAPTEST_ACCESS_TYPE_SHIFT 2
#define HMAPTEST_ACCESS_ARM 0
#define HMAPTEST_ACCESS_M2M 1
#define HMAPTEST_ACCESS_D11 2
#define HMAPTEST_ACCESS_NONE 3 /* marker to stop HMAPTEST */

typedef struct pcie_hmaptest {
	uint16	version;		/* Version = 1 */
	uint16	length;		/* Length of entire structure */
	uint32	xfer_len;
	uint32	accesstype;
	uint32	is_write;
	uint32	is_invalid;
	uint32	host_addr_hi;
	uint32	host_addr_lo;
} pcie_hmaptest_t;

/* HMAP window register set */
typedef struct hmapwindow {
	uint32 baseaddr_lo; /* BaseAddrLower */
	uint32 baseaddr_hi; /* BaseAddrUpper */
	uint32 windowlength; /* Window Length */
} hmapwindow_t;

#define PCIE_HMAP_VERSION 1
typedef struct pcie_hmap {
	uint16	version;		/**< Version = 1 */
	uint16	length;			/**< Length of entire structure */
	uint32	enable;			/**< status of HMAP enabled/disabled */
	uint32	nwindows;		/* no. of HMAP windows enabled */
	uint32	window_config;		/* HMAP window_config register */
	uint32	hmap_violationaddr_lo;	/* violating address lo */
	uint32	hmap_violationaddr_hi;	/* violating addr hi */
	uint32	hmap_violation_info;	/* violation info */
	hmapwindow_t hwindows[];	/* Multiple hwindows */
} pcie_hmap_t;

/* ##### Power Stats section ##### */

#define WL_PWRSTATS_VERSION	2

/** Input structure for pwrstats IOVAR */
typedef struct wl_pwrstats_query {
	uint16 length;		/**< Number of entries in type array. */
	uint16 type[1];		/**< Types (tags) to retrieve.
				 * Length 0 (no types) means get all.
				 */
} wl_pwrstats_query_t;

/** This structure is for version 2; version 1 will be deprecated in by FW */
#include <packed_section_start.h>
typedef BWL_PRE_PACKED_STRUCT struct wl_pwrstats {
	uint16 version;		      /**< Version = 2 is TLV format */
	uint16 length;		      /**< Length of entire structure */
	uint8 data[1];		      /**< TLV data, a series of structures,
				       * each starting with type and length.
				       *
				       * Padded as necessary so each section
				       * starts on a 4-byte boundary.
				       *
				       * Both type and len are uint16, but the
				       * upper nibble of length is reserved so
				       * valid len values are 0-4095.
				       */
} BWL_POST_PACKED_STRUCT wl_pwrstats_t;
#include <packed_section_end.h>
#define WL_PWR_STATS_HDRLEN	OFFSETOF(wl_pwrstats_t, data)

/* Bits for wake reasons */
#define WLC_PMD_WAKE_SET		0x1
#define WLC_PMD_PM_AWAKE_BCN		0x2
/* BIT:3 is no longer being used */
#define WLC_PMD_SCAN_IN_PROGRESS	0x8
#define WLC_PMD_RM_IN_PROGRESS		0x10
#define WLC_PMD_AS_IN_PROGRESS		0x20
#define WLC_PMD_PM_PEND			0x40
#define WLC_PMD_PS_POLL			0x80
#define WLC_PMD_CHK_UNALIGN_TBTT	0x100
#define WLC_PMD_APSD_STA_UP		0x200
#define WLC_PMD_TX_PEND_WAR		0x400   /* obsolete, can be reused */
#define WLC_PMD_GPTIMER_STAY_AWAKE	0x800
#ifdef WLAWDL
#define WLC_PMD_AWDL_AWAKE		0x1000
#endif /* WLAWDL */
#define WLC_PMD_PM2_RADIO_SOFF_PEND	0x2000
#define WLC_PMD_NON_PRIM_STA_UP		0x4000
#define WLC_PMD_AP_UP			0x8000

typedef struct wlc_pm_debug {
	uint32 timestamp;	     /**< timestamp in millisecond */
	uint32 reason;		     /**< reason(s) for staying awake */
} wlc_pm_debug_t;

/** WL_PWRSTATS_TYPE_PM_AWAKE1 structures (for 6.25 firmware) */
#define WLC_STA_AWAKE_STATES_MAX_V1	30
#define WLC_PMD_EVENT_MAX_V1		32
/** Data sent as part of pwrstats IOVAR (and EXCESS_PM_WAKE event) */
#include <packed_section_start.h>
typedef BWL_PRE_PACKED_STRUCT struct pm_awake_data_v1 {
	uint32 curr_time;	/**< ms */
	uint32 hw_macc;		/**< HW maccontrol */
	uint32 sw_macc;		/**< SW maccontrol */
	uint32 pm_dur;		/**< Total sleep time in PM, msecs */
	uint32 mpc_dur;		/**< Total sleep time in MPC, msecs */

	/* int32 drifts = remote - local; +ve drift => local-clk slow */
	int32 last_drift;	/**< Most recent TSF drift from beacon */
	int32 min_drift;	/**< Min TSF drift from beacon in magnitude */
	int32 max_drift;	/**< Max TSF drift from beacon in magnitude */

	uint32 avg_drift;	/**< Avg TSF drift from beacon */

	/* Wake history tracking */
	uint8  pmwake_idx;				   /**< for stepping through pm_state */
	wlc_pm_debug_t pm_state[WLC_STA_AWAKE_STATES_MAX_V1]; /**< timestamped wake bits */
	uint32 pmd_event_wake_dur[WLC_PMD_EVENT_MAX_V1];   /**< cumulative usecs per wake reason */
	uint32 drift_cnt;	/**< Count of drift readings over which avg_drift was computed */
} BWL_POST_PACKED_STRUCT pm_awake_data_v1_t;
#include <packed_section_end.h>

#include <packed_section_start.h>
typedef BWL_PRE_PACKED_STRUCT struct wl_pwr_pm_awake_stats_v1 {
	uint16 type;	     /**< WL_PWRSTATS_TYPE_PM_AWAKE */
	uint16 len;	     /**< Up to 4K-1, top 4 bits are reserved */

	pm_awake_data_v1_t awake_data;
	uint32 frts_time;	/**< Cumulative ms spent in frts since driver load */
	uint32 frts_end_cnt;	/**< No of times frts ended since driver load */
} BWL_POST_PACKED_STRUCT wl_pwr_pm_awake_stats_v1_t;
#include <packed_section_end.h>

/** WL_PWRSTATS_TYPE_PM_AWAKE2 structures. Data sent as part of pwrstats IOVAR */
typedef struct pm_awake_data_v2 {
	uint32 curr_time;	/**< ms */
	uint32 hw_macc;		/**< HW maccontrol */
	uint32 sw_macc;		/**< SW maccontrol */
	uint32 pm_dur;		/**< Total sleep time in PM, msecs */
	uint32 mpc_dur;		/**< Total sleep time in MPC, msecs */

	/* int32 drifts = remote - local; +ve drift => local-clk slow */
	int32 last_drift;	/**< Most recent TSF drift from beacon */
	int32 min_drift;	/**< Min TSF drift from beacon in magnitude */
	int32 max_drift;	/**< Max TSF drift from beacon in magnitude */

	uint32 avg_drift;	/**< Avg TSF drift from beacon */

	/* Wake history tracking */

	/* pmstate array (type wlc_pm_debug_t) start offset */
	uint16 pm_state_offset;
	/** pmstate number of array entries */
	uint16 pm_state_len;

	/** array (type uint32) start offset */
	uint16 pmd_event_wake_dur_offset;
	/** pmd_event_wake_dur number of array entries */
	uint16 pmd_event_wake_dur_len;

	uint32 drift_cnt;	/**< Count of drift readings over which avg_drift was computed */
	uint8  pmwake_idx;	/**< for stepping through pm_state */
	uint8  flags;		/**< bit0: 1-sleep, 0- wake. bit1: 0-bit0 invlid, 1-bit0 valid */
	uint8  pad[2];
	uint32 frts_time;	/**< Cumulative ms spent in frts since driver load */
	uint32 frts_end_cnt;	/**< No of times frts ended since driver load */
} pm_awake_data_v2_t;

typedef struct wl_pwr_pm_awake_stats_v2 {
	uint16 type;	     /**< WL_PWRSTATS_TYPE_PM_AWAKE */
	uint16 len;	     /**< Up to 4K-1, top 4 bits are reserved */

	pm_awake_data_v2_t awake_data;
} wl_pwr_pm_awake_stats_v2_t;

/* bit0: 1-sleep, 0- wake. bit1: 0-bit0 invlid, 1-bit0 valid */
#define WL_PWR_PM_AWAKE_STATS_WAKE      0x02
#define WL_PWR_PM_AWAKE_STATS_ASLEEP    0x03
#define WL_PWR_PM_AWAKE_STATS_WAKE_MASK 0x03

/* WL_PWRSTATS_TYPE_PM_AWAKE Version 2 structures taken from 4324/43342 */
/* These structures are only to be used with 4324/43342 devices */

#define WL_STA_AWAKE_STATES_MAX_V2	30
#define WL_PMD_EVENT_MAX_V2		32
#define MAX_P2P_BSS_DTIM_PRD		4

#include <packed_section_start.h>
typedef BWL_PRE_PACKED_STRUCT struct ucode_dbg_v2 {
	uint32 macctrl;
	uint16 m_p2p_hps;
	uint16 m_p2p_bss_dtim_prd[MAX_P2P_BSS_DTIM_PRD];
	uint32 psmdebug[20];
	uint32 phydebug[20];
	uint32 psm_brc;
	uint32 ifsstat;
} BWL_POST_PACKED_STRUCT ucode_dbg_v2_t;
#include <packed_section_end.h>

#include <packed_section_start.h>
typedef BWL_PRE_PACKED_STRUCT struct pmalert_awake_data_v2 {
	uint32 curr_time;	/* ms */
	uint32 hw_macc;		/* HW maccontrol */
	uint32 sw_macc;		/* SW maccontrol */
	uint32 pm_dur;		/* Total sleep time in PM, msecs */
	uint32 mpc_dur;		/* Total sleep time in MPC, msecs */

	/* int32 drifts = remote - local; +ve drift => local-clk slow */
	int32 last_drift;	/* Most recent TSF drift from beacon */
	int32 min_drift;	/* Min TSF drift from beacon in magnitude */
	int32 max_drift;	/* Max TSF drift from beacon in magnitude */

	uint32 avg_drift;	/* Avg TSF drift from beacon */

	/* Wake history tracking */
	uint8  pmwake_idx;				   /* for stepping through pm_state */
	wlc_pm_debug_t pm_state[WL_STA_AWAKE_STATES_MAX_V2]; /* timestamped wake bits */
	uint32 pmd_event_wake_dur[WL_PMD_EVENT_MAX_V2];      /* cumulative usecs per wake reason */
	uint32 drift_cnt;	/* Count of drift readings over which avg_drift was computed */
	uint32	start_event_dur[WL_PMD_EVENT_MAX_V2]; /* start event-duration */
	ucode_dbg_v2_t ud;
	uint32 frts_time;	/* Cumulative ms spent in frts since driver load */
	uint32 frts_end_cnt;	/* No of times frts ended since driver load */
} BWL_POST_PACKED_STRUCT pmalert_awake_data_v2_t;
#include <packed_section_end.h>

#include <packed_section_start.h>
typedef BWL_PRE_PACKED_STRUCT struct pm_alert_data_v2 {
	uint32 version;
	uint32 length; /* Length of entire structure */
	uint32 reasons; /* reason(s) for pm_alert */
	/* Following fields are present only for reasons
	 * PM_DUR_EXCEEDED, MPC_DUR_EXCEEDED & CONST_AWAKE_DUR_EXCEEDED
	 */
	uint32 prev_stats_time;	/* msecs */
	uint32 prev_pm_dur;	/* msecs */
	uint32 prev_mpc_dur;	/* msecs */
	pmalert_awake_data_v2_t awake_data;
} BWL_POST_PACKED_STRUCT pm_alert_data_v2_t;
#include <packed_section_end.h>

#include <packed_section_start.h>
typedef BWL_PRE_PACKED_STRUCT struct wl_pwr_pm_awake_status_v2 {
	uint16 type;	     /* WL_PWRSTATS_TYPE_PM_AWAKE */
	uint16 len;	     /* Up to 4K-1, top 4 bits are reserved */

	pmalert_awake_data_v2_t awake_data;
	uint32 frts_time;	/* Cumulative ms spent in frts since driver load */
	uint32 frts_end_cnt;	/* No of times frts ended since driver load */
} BWL_POST_PACKED_STRUCT wl_pwr_pm_awake_status_v2_t;
#include <packed_section_end.h>

/* Below are latest definitions from PHO25178RC100_BRANCH_6_50 */
/* wl_pwr_pm_awake_stats_v1_t is used for WL_PWRSTATS_TYPE_PM_AWAKE */
/* (at least) the chip independent registers */
typedef struct ucode_dbg_ext {
	uint32 x120;
	uint32 x124;
	uint32 x154;
	uint32 x158;
	uint32 x15c;
	uint32 x180;
	uint32 x184;
	uint32 x188;
	uint32 x18c;
	uint32 x1a0;
	uint32 x1a8;
	uint32 x1e0;
	uint32 scr_x14;
	uint32 scr_x2b;
	uint32 scr_x2c;
	uint32 scr_x2d;
	uint32 scr_x2e;

	uint16 x40a;
	uint16 x480;
	uint16 x490;
	uint16 x492;
	uint16 x4d8;
	uint16 x4b8;
	uint16 x4ba;
	uint16 x4bc;
	uint16 x4be;
	uint16 x500;
	uint16 x50e;
	uint16 x522;
	uint16 x546;
	uint16 x578;
	uint16 x602;
	uint16 x646;
	uint16 x648;
	uint16 x666;
	uint16 x670;
	uint16 x690;
	uint16 x692;
	uint16 x6a0;
	uint16 x6a2;
	uint16 x6a4;
	uint16 x6b2;
	uint16 x7c0;

	uint16 shm_x20;
	uint16 shm_x4a;
	uint16 shm_x5e;
	uint16 shm_x5f;
	uint16 shm_xaab;
	uint16 shm_x74a;
	uint16 shm_x74b;
	uint16 shm_x74c;
	uint16 shm_x74e;
	uint16 shm_x756;
	uint16 shm_x75b;
	uint16 shm_x7b9;
	uint16 shm_x7d4;

	uint16 shm_P2P_HPS;
	uint16 shm_P2P_intr[16];
	uint16 shm_P2P_perbss[48];
} ucode_dbg_ext_t;

#include <packed_section_start.h>
typedef BWL_PRE_PACKED_STRUCT struct pm_alert_data_v1 {
	uint32 version;
	uint32 length; /**< Length of entire structure */
	uint32 reasons; /**< reason(s) for pm_alert */
	/* Following fields are present only for reasons
	 * PM_DUR_EXCEEDED, MPC_DUR_EXCEEDED & CONST_AWAKE_DUR_EXCEEDED
	 */
	uint32 prev_stats_time;	/**< msecs */
	uint32 prev_pm_dur;	/**< msecs */
	uint32 prev_mpc_dur;	/**< msecs */
	pm_awake_data_v1_t awake_data;
	uint32	start_event_dur[WLC_PMD_EVENT_MAX_V1]; /**< start event-duration */
	ucode_dbg_v2_t ud;
	uint32 frts_time;	/**< Cumulative ms spent in frts since driver load */
	uint32 frts_end_cnt;	/**< No of times frts ended since driver load */
	ucode_dbg_ext_t ud_ext;
	uint32 prev_frts_dur; /**< ms */
} BWL_POST_PACKED_STRUCT pm_alert_data_v1_t;
#include <packed_section_end.h>

/* End of 43342/4324 v2 structure definitions */

/* Original bus structure is for HSIC */

typedef struct bus_metrics {
	uint32 suspend_ct;	/**< suspend count */
	uint32 resume_ct;	/**< resume count */
	uint32 disconnect_ct;	/**< disconnect count */
	uint32 reconnect_ct;	/**< reconnect count */
	uint32 active_dur;	/**< msecs in bus, usecs for user */
	uint32 suspend_dur;	/**< msecs in bus, usecs for user */
	uint32 disconnect_dur;	/**< msecs in bus, usecs for user */
} bus_metrics_t;

/** Bus interface info for USB/HSIC */
#include <packed_section_start.h>
typedef BWL_PRE_PACKED_STRUCT struct wl_pwr_usb_hsic_stats {
	uint16 type;	     /**< WL_PWRSTATS_TYPE_USB_HSIC */
	uint16 len;	     /**< Up to 4K-1, top 4 bits are reserved */

	bus_metrics_t hsic;	/**< stats from hsic bus driver */
} BWL_POST_PACKED_STRUCT wl_pwr_usb_hsic_stats_t;
#include <packed_section_end.h>

typedef struct pcie_bus_metrics {
	uint32 d3_suspend_ct;	/**< suspend count */
	uint32 d0_resume_ct;	/**< resume count */
	uint32 perst_assrt_ct;	/**< PERST# assert count */
	uint32 perst_deassrt_ct;	/**< PERST# de-assert count */
	uint32 active_dur;	/**< msecs */
	uint32 d3_suspend_dur;	/**< msecs */
	uint32 perst_dur;	/**< msecs */
	uint32 l0_cnt;		/**< L0 entry count */
	uint32 l0_usecs;	/**< L0 duration in usecs */
	uint32 l1_cnt;		/**< L1 entry count */
	uint32 l1_usecs;	/**< L1 duration in usecs */
	uint32 l1_1_cnt;	/**< L1_1ss entry count */
	uint32 l1_1_usecs;	/**< L1_1ss duration in usecs */
	uint32 l1_2_cnt;	/**< L1_2ss entry count */
	uint32 l1_2_usecs;	/**< L1_2ss duration in usecs */
	uint32 l2_cnt;		/**< L2 entry count */
	uint32 l2_usecs;	/**< L2 duration in usecs */
	uint32 timestamp;	/**< Timestamp on when stats are collected */
	uint32 num_h2d_doorbell;	/**< # of doorbell interrupts - h2d */
	uint32 num_d2h_doorbell;	/**< # of doorbell interrupts - d2h */
	uint32 num_submissions; /**< # of submissions */
	uint32 num_completions; /**< # of completions */
	uint32 num_rxcmplt;	/**< # of rx completions */
	uint32 num_rxcmplt_drbl;	/**< of drbl interrupts for rx complt. */
	uint32 num_txstatus;	/**< # of tx completions */
	uint32 num_txstatus_drbl;	/**< of drbl interrupts for tx complt. */
	uint32 deepsleep_count; /**< # of times chip went to deepsleep */
	uint32 deepsleep_dur;   /**< # of msecs chip was in deepsleep */
	uint32 ltr_active_ct;	/**< # of times chip went to LTR ACTIVE */
	uint32 ltr_active_dur;	/**< # of msecs chip was in LTR ACTIVE */
	uint32 ltr_sleep_ct;	/**< # of times chip went to LTR SLEEP */
	uint32 ltr_sleep_dur;	/**< # of msecs chip was in LTR SLEEP */
} pcie_bus_metrics_t;

/** Bus interface info for PCIE */
typedef struct wl_pwr_pcie_stats {
	uint16 type;	     /**< WL_PWRSTATS_TYPE_PCIE */
	uint16 len;	     /**< Up to 4K-1, top 4 bits are reserved */
	pcie_bus_metrics_t pcie;	/**< stats from pcie bus driver */
} wl_pwr_pcie_stats_t;

/** Scan information history per category */
typedef struct scan_data {
	uint32 count;		/**< Number of scans performed */
	uint32 dur;		/**< Total time (in us) used */
} scan_data_t;

typedef struct wl_pwr_scan_stats {
	uint16 type;	     /**< WL_PWRSTATS_TYPE_SCAN */
	uint16 len;	     /**< Up to 4K-1, top 4 bits are reserved */

	/* Scan history */
	scan_data_t user_scans;	  /**< User-requested scans: (i/e/p)scan */
	scan_data_t assoc_scans;  /**< Scans initiated by association requests */
	scan_data_t roam_scans;	  /**< Scans initiated by the roam engine */
	scan_data_t pno_scans[8]; /**< For future PNO bucketing (BSSID, SSID, etc) */
	scan_data_t other_scans;  /**< Scan engine usage not assigned to the above */
} wl_pwr_scan_stats_t;

typedef struct wl_pwr_connect_stats {
	uint16 type;	     /**< WL_PWRSTATS_TYPE_SCAN */
	uint16 len;	     /**< Up to 4K-1, top 4 bits are reserved */

	/* Connection (Association + Key exchange) data */
	uint32 count;	/**< Number of connections performed */
	uint32 dur;		/**< Total time (in ms) used */
} wl_pwr_connect_stats_t;

typedef struct wl_pwr_phy_stats {
	uint16 type;	    /**< WL_PWRSTATS_TYPE_PHY */
	uint16 len;	    /**< Up to 4K-1, top 4 bits are reserved */
	uint32 tx_dur;	    /**< TX Active duration in us */
	uint32 rx_dur;	    /**< RX Active duration in us */
} wl_pwr_phy_stats_t;

#ifdef WLAWDL
typedef struct wl_pwr_awdl_stats {
	uint16 type;	    /**< WL_PWRSTATS_TYPE_AWDL */
	uint16 len;	    /**< Up to 4K-1, top 4 bits are reserved */
	uint32 tx_dur;	    /**< AWDL TX Active duration in usec */
	uint32 rx_dur;	    /**< AWDL RX Active duration in usec */
	uint32 aw_dur;	    /**< AWDL AW duration in msec */
	uint32 awpscan_dur; /**< AWDL pscans dur in msec */
} wl_pwr_awdl_stats_t;
#endif /* WLAWDL */

typedef struct wl_mimo_meas_metrics_v1 {
	uint16 type;
	uint16 len;
	/* Total time(us) idle in MIMO RX chain configuration */
	uint32 total_idle_time_mimo;
	/* Total time(us) idle in SISO  RX chain configuration */
	uint32 total_idle_time_siso;
	/* Total receive time (us) in SISO RX chain configuration */
	uint32 total_rx_time_siso;
	/* Total receive time (us) in MIMO RX chain configuration */
	uint32 total_rx_time_mimo;
	/* Total 1-chain transmit time(us) */
	uint32 total_tx_time_1chain;
	/* Total 2-chain transmit time(us) */
	uint32 total_tx_time_2chain;
	/* Total 3-chain transmit time(us) */
	uint32 total_tx_time_3chain;
} wl_mimo_meas_metrics_v1_t;

typedef struct wl_mimo_meas_metrics {
	uint16 type;
	uint16 len;
	/* Total time(us) idle in MIMO RX chain configuration */
	uint32 total_idle_time_mimo;
	/* Total time(us) idle in SISO  RX chain configuration */
	uint32 total_idle_time_siso;
	/* Total receive time (us) in SISO RX chain configuration */
	uint32 total_rx_time_siso;
	/* Total receive time (us) in MIMO RX chain configuration */
	uint32 total_rx_time_mimo;
	/* Total 1-chain transmit time(us) */
	uint32 total_tx_time_1chain;
	/* Total 2-chain transmit time(us) */
	uint32 total_tx_time_2chain;
	/* Total 3-chain transmit time(us) */
	uint32 total_tx_time_3chain;
	/* End of original, OCL fields start here */
	/* Total time(us) idle in ocl mode */
	uint32 total_idle_time_ocl;
	/* Total receive time (us) in ocl mode */
	uint32 total_rx_time_ocl;
	/* End of OCL fields, internal adjustment fields here */
	/* Total SIFS idle time in MIMO mode */
	uint32 total_sifs_time_mimo;
	/* Total SIFS idle time in SISO mode */
	uint32 total_sifs_time_siso;
} wl_mimo_meas_metrics_t;

typedef struct wl_pwr_slice_index {
	uint16 type;	     /* WL_PWRSTATS_TYPE_SLICE_INDEX */
	uint16 len;

	uint32 slice_index;	/* Slice index for which stats are meant for */
} wl_pwr_slice_index_t;
/* ##### End of Power Stats section ##### */

/** IPV4 Arp offloads for ndis context */
#include <packed_section_start.h>
BWL_PRE_PACKED_STRUCT struct hostip_id {
	struct ipv4_addr ipa;
	uint8 id;
} BWL_POST_PACKED_STRUCT;
#include <packed_section_end.h>

/* Return values */
#define ND_REPLY_PEER		0x1	/**< Reply was sent to service NS request from peer */
#define ND_REQ_SINK		0x2	/**< Input packet should be discarded */
#define ND_FORCE_FORWARD	0X3	/**< For the dongle to forward req to HOST */

/** Neighbor Solicitation Response Offload IOVAR param */
#include <packed_section_start.h>
typedef BWL_PRE_PACKED_STRUCT struct nd_param {
	struct ipv6_addr	host_ip[2];
	struct ipv6_addr	solicit_ip;
	struct ipv6_addr	remote_ip;
	uint8	host_mac[ETHER_ADDR_LEN];
	uint32	offload_id;
} BWL_POST_PACKED_STRUCT nd_param_t;
#include <packed_section_end.h>

typedef struct wl_pfn_roam_thresh {
	uint32 pfn_alert_thresh; /**< time in ms */
	uint32 roam_alert_thresh; /**< time in ms */
} wl_pfn_roam_thresh_t;

/* Reasons for wl_pmalert_t */
#define PM_DUR_EXCEEDED			(1<<0)
#define MPC_DUR_EXCEEDED		(1<<1)
#define ROAM_ALERT_THRESH_EXCEEDED	(1<<2)
#define PFN_ALERT_THRESH_EXCEEDED	(1<<3)
#define CONST_AWAKE_DUR_ALERT		(1<<4)
#define CONST_AWAKE_DUR_RECOVERY	(1<<5)

#define MIN_PM_ALERT_LEN 9

/** Data sent in EXCESS_PM_WAKE event */
#define WL_PM_ALERT_VERSION 3

/** This structure is for version 3; version 2 will be deprecated in by FW */
#include <packed_section_start.h>
typedef BWL_PRE_PACKED_STRUCT struct wl_pmalert {
	uint16 version;		/**< Version = 3 is TLV format */
	uint16 length;		/**< Length of entire structure */
	uint32 reasons;		/**< reason(s) for pm_alert */
	uint8 data[1];		/**< TLV data, a series of structures,
				 * each starting with type and length.
				 *
				 * Padded as necessary so each section
				 * starts on a 4-byte boundary.
				 *
				 * Both type and len are uint16, but the
				 * upper nibble of length is reserved so
				 * valid len values are 0-4095.
				*/
} BWL_POST_PACKED_STRUCT wl_pmalert_t;
#include <packed_section_end.h>

/* Type values for the data section */
#define WL_PMALERT_FIXED	0	/**< struct wl_pmalert_fixed_t, fixed fields */
#define WL_PMALERT_PMSTATE	1	/**< struct wl_pmalert_pmstate_t, variable */
#define WL_PMALERT_EVENT_DUR	2	/**< struct wl_pmalert_event_dur_t, variable */
#define WL_PMALERT_UCODE_DBG	3	/**< struct wl_pmalert_ucode_dbg_v1, variable */
#define WL_PMALERT_PS_ALLOWED_HIST	4 /**< struct wl_pmalert_ps_allowed_history, variable */
#define WL_PMALERT_EXT_UCODE_DBG	5 /**< struct wl_pmalert_ext_ucode_dbg_t, variable */
#define WL_PMALERT_EPM_START_EVENT_DUR	6 /**< struct wl_pmalert_event_dur_t, variable */
#define WL_PMALERT_UCODE_DBG_V2		7 /**< struct wl_pmalert_ucode_dbg_v2, variable */

typedef struct wl_pmalert_fixed {
	uint16 type;		/**< WL_PMALERT_FIXED */
	uint16 len;		/**< Up to 4K-1, top 4 bits are reserved */
	uint32 prev_stats_time;	/**< msecs */
	uint32 curr_time;	/**< ms */
	uint32 prev_pm_dur;	/**< msecs */
	uint32 pm_dur;		/**< Total sleep time in PM, msecs */
	uint32 prev_mpc_dur;	/**< msecs */
	uint32 mpc_dur;		/**< Total sleep time in MPC, msecs */
	uint32 hw_macc;		/**< HW maccontrol */
	uint32 sw_macc;		/**< SW maccontrol */

	/* int32 drifts = remote - local; +ve drift -> local-clk slow */
	int32 last_drift;	/**< Most recent TSF drift from beacon */
	int32 min_drift;	/**< Min TSF drift from beacon in magnitude */
	int32 max_drift;	/**< Max TSF drift from beacon in magnitude */

	uint32 avg_drift;	/**< Avg TSF drift from beacon */
	uint32 drift_cnt;	/**< Count of drift readings over which avg_drift was computed */
	uint32 frts_time;	/**< Cumulative ms spent in data frts since driver load */
	uint32 frts_end_cnt;	/**< No of times frts ended since driver load */
	uint32 prev_frts_dur;	/**< Data frts duration at start of pm-period */
	uint32 cal_dur;		/**< Cumulative ms spent in calibration */
	uint32 prev_cal_dur;	/**< cal duration at start of pm-period */
} wl_pmalert_fixed_t;

typedef struct wl_pmalert_pmstate {
	uint16 type;	     /**< WL_PMALERT_PMSTATE */
	uint16 len;	     /**< Up to 4K-1, top 4 bits are reserved */

	uint8 pmwake_idx;   /**< for stepping through pm_state */
	uint8 pad[3];
	/* Array of pmstate; len of array is based on tlv len */
	wlc_pm_debug_t pmstate[1];
} wl_pmalert_pmstate_t;

typedef struct wl_pmalert_event_dur {
	uint16 type;	     /**< WL_PMALERT_EVENT_DUR */
	uint16 len;	     /**< Up to 4K-1, top 4 bits are reserved */

	/* Array of event_dur, len of array is based on tlv len */
	uint32 event_dur[1];
} wl_pmalert_event_dur_t;

#include <packed_section_start.h>
BWL_PRE_PACKED_STRUCT struct wl_pmalert_ucode_dbg_v1 {
	uint16 type;         /* WL_PMALERT_UCODE_DBG */
	uint16 len;      /* Up to 4K-1, top 4 bits are reserved */
	uint32 macctrl;
	uint16 m_p2p_hps;
	uint32 psm_brc;
	uint32 ifsstat;
	uint16 m_p2p_bss_dtim_prd[MAX_P2P_BSS_DTIM_PRD];
	uint32 psmdebug[20];
	uint32 phydebug[20];
	uint16 M_P2P_BSS[3][12];
	uint16 M_P2P_PRE_TBTT[3];

	/* Following is valid only for corerevs<40 */
	uint16 xmtfifordy;

	/* Following 3 are valid only for 11ac corerevs (>=40) */
	uint16 psm_maccommand;
	uint16 txe_status1;
	uint16 AQMFifoReady;
} BWL_POST_PACKED_STRUCT;
#include <packed_section_end.h>

#include <packed_section_start.h>
BWL_PRE_PACKED_STRUCT struct wl_pmalert_ucode_dbg_v2 {
	uint16 type;	     /**< WL_PMALERT_UCODE_DBG_V2 */
	uint16 len;	     /**< Up to 4K-1, top 4 bits are reserved */
	uint32 macctrl;
	uint16 m_p2p_hps;
	uint32 psm_brc;
	uint32 ifsstat;
	uint16 m_p2p_bss_dtim_prd[MAX_P2P_BSS_DTIM_PRD];
	uint32 psmdebug[20];
	uint32 phydebug[20];
	uint16 M_P2P_BSS[3][12];
	uint16 M_P2P_PRE_TBTT[3];

	/* Following is valid only for corerevs<40 */
	uint16 xmtfifordy;

	/* Following 3 are valid only for 11ac corerevs (>=40) */
	uint16 psm_maccommand;
	uint16 txe_status1;
	uint32 AQMFifoReady;
} BWL_POST_PACKED_STRUCT;
#include <packed_section_end.h>

typedef struct wlc_ps_debug {
	uint32 timestamp;	     /**< timestamp in millisecond */
	uint32 ps_mask;		     /**< reason(s) for disallowing ps */
} wlc_ps_debug_t;

typedef struct wl_pmalert_ps_allowed_hist {
	uint16 type;	     /**< WL_PMALERT_PS_ALLOWED_HIST */
	uint16 len;	     /**< Up to 4K-1, top 4 bits are reserved */
	uint32 ps_allowed_start_idx;
	/* Array of ps_debug, len of array is based on tlv len */
	wlc_ps_debug_t ps_debug[1];
} wl_pmalert_ps_allowed_hist_t;

/* Structures and constants used for "vndr_ie" IOVar interface */
#define VNDR_IE_CMD_LEN		4	/**< length of the set command string:
					 * "add", "del" (+ NUL)
					 */

#define VNDR_IE_INFO_HDR_LEN	(sizeof(uint32))

#include <packed_section_start.h>
typedef BWL_PRE_PACKED_STRUCT struct {
	uint32 pktflag;			/**< bitmask indicating which packet(s) contain this IE */
	vndr_ie_t vndr_ie_data;		/**< vendor IE data */
} BWL_POST_PACKED_STRUCT vndr_ie_info_t;
#include <packed_section_end.h>

#include <packed_section_start.h>
typedef BWL_PRE_PACKED_STRUCT struct {
	int32 iecount;			/**< number of entries in the vndr_ie_list[] array */
	vndr_ie_info_t vndr_ie_list[1];	/**< variable size list of vndr_ie_info_t structs */
} BWL_POST_PACKED_STRUCT vndr_ie_buf_t;
#include <packed_section_end.h>

#include <packed_section_start.h>
typedef BWL_PRE_PACKED_STRUCT struct {
	char cmd[VNDR_IE_CMD_LEN];	/**< vndr_ie IOVar set command : "add", "del" + NUL */
	vndr_ie_buf_t vndr_ie_buffer;	/**< buffer containing Vendor IE list information */
} BWL_POST_PACKED_STRUCT vndr_ie_setbuf_t;
#include <packed_section_end.h>

/** tag_ID/length/value_buffer tuple */
#include <packed_section_start.h>
typedef BWL_PRE_PACKED_STRUCT struct {
	uint8	id;
	uint8	len;
	uint8	data[1];
} BWL_POST_PACKED_STRUCT tlv_t;
#include <packed_section_end.h>

#include <packed_section_start.h>
typedef BWL_PRE_PACKED_STRUCT struct {
	uint32 pktflag;			/**< bitmask indicating which packet(s) contain this IE */
	tlv_t ie_data;		/**< IE data */
} BWL_POST_PACKED_STRUCT ie_info_t;
#include <packed_section_end.h>

#include <packed_section_start.h>
typedef BWL_PRE_PACKED_STRUCT struct {
	int32 iecount;			/**< number of entries in the ie_list[] array */
	ie_info_t ie_list[1];	/**< variable size list of ie_info_t structs */
} BWL_POST_PACKED_STRUCT ie_buf_t;
#include <packed_section_end.h>

#include <packed_section_start.h>
typedef BWL_PRE_PACKED_STRUCT struct {
	char cmd[VNDR_IE_CMD_LEN];	/**< ie IOVar set command : "add" + NUL */
	ie_buf_t ie_buffer;	/**< buffer containing IE list information */
} BWL_POST_PACKED_STRUCT ie_setbuf_t;
#include <packed_section_end.h>

#include <packed_section_start.h>
typedef BWL_PRE_PACKED_STRUCT struct {
	uint32 pktflag;		/**< bitmask indicating which packet(s) contain this IE */
	uint8 id;		/**< IE type */
} BWL_POST_PACKED_STRUCT ie_getbuf_t;
#include <packed_section_end.h>

/* structures used to define format of wps ie data from probe requests */
/* passed up to applications via iovar "prbreq_wpsie" */
typedef struct sta_prbreq_wps_ie_hdr {
	struct ether_addr staAddr;
	uint16 ieLen;
} sta_prbreq_wps_ie_hdr_t;

#include <packed_section_start.h>
typedef BWL_PRE_PACKED_STRUCT struct sta_prbreq_wps_ie_data {
	sta_prbreq_wps_ie_hdr_t hdr;
	uint8 ieData[1];
} BWL_POST_PACKED_STRUCT sta_prbreq_wps_ie_data_t;
#include <packed_section_end.h>

#include <packed_section_start.h>
typedef BWL_PRE_PACKED_STRUCT struct sta_prbreq_wps_ie_list {
	uint32 totLen;
	uint8 ieDataList[1];
} BWL_POST_PACKED_STRUCT sta_prbreq_wps_ie_list_t;
#include <packed_section_end.h>

#include <packed_section_start.h>
typedef BWL_PRE_PACKED_STRUCT struct {
	uint32 flags;
	chanspec_t chanspec;			/**< txpwr report for this channel */
	chanspec_t local_chanspec;		/**< channel on which we are associated */
	uint8 local_max;			/**< local max according to the AP */
	uint8 local_constraint;			/**< local constraint according to the AP */
	int8  antgain[2];			/**< Ant gain for each band - from SROM */
	uint8 rf_cores;				/**< count of RF Cores being reported */
	uint8 est_Pout[4];			/**< Latest tx power out estimate per RF chain */
	uint8 est_Pout_act[4]; /**< Latest tx power out estimate per RF chain w/o adjustment */
	uint8 est_Pout_cck;			/**< Latest CCK tx power out estimate */
	uint8 tx_power_max[4];			/**< Maximum target power among all rates */
	uint32 tx_power_max_rate_ind[4];  /**< Index of the rate with the max target power */
	int8 sar;				/**< SAR limit for display by wl executable */
	int8 channel_bandwidth;		/**< 20, 40 or 80 MHz bandwidth? */
	uint8 version;				/**< Version of the data format wlu <--> driver */
	uint8 display_core;			/**< Displayed curpower core */
	int8 target_offsets[4];		/**< Target power offsets for current rate per core */
	uint32 last_tx_ratespec;	/**< Ratespec for last transmition */
	uint32 user_target;		/**< user limit */
	uint32 ppr_len;		/**< length of each ppr serialization buffer */
	int8 SARLIMIT[MAX_STREAMS_SUPPORTED];
	uint8  pprdata[1];		/**< ppr serialization buffer */
} BWL_POST_PACKED_STRUCT tx_pwr_rpt_t;
#include <packed_section_end.h>

#include <packed_section_start.h>
typedef BWL_PRE_PACKED_STRUCT struct {
	uint8 txctrlwd_version;	/**< Version of the data format of txctrl wlu <--> driver */
	uint8 frame_type;	/**< To indicate cck/11ag/ht/vht/he */
	uint8 he_format;	/**< To indicate SU/ER/MU/Trigger format  */
	bool not_sounding;	/**< not_sounding */
	bool preamble;		/**< cck/HT preamble type */
	uint8 mcs_nss;		/**< SU mcs_nss */
	bool stbc;		/**< stbc */
	uint8 n_pwr;		/**< Number of physical txchain */
	uint8 n_user;		/**< Number of user for DL-OFDMA of MU-MIMO */
	bool mu;		/**< indicator of MU-MIMO */
	uint8 core_mask;	/**< indicator of core_mask */
	uint8 ant_cfg;		/**< indicator of ant_cfg */
	uint8 pktbw;		/**< indicator of packet BW */
	uint8 subband;		/**< indicator of subband location */
	uint8 partial_ofdma_subband; /**< indicator of partial ofdma subband */
	bool dynBW_present;	/**< indicator of dynBW present */
	bool dynBW_mode;	/**< indicator of dynBW mode */
	uint8 scrambler;	/**< indicator of scrambler info */
	uint32 cfo_comp_val;	/**< indicator of cfo comp value */
	bool cfo_comp_en;	/**< indicator of cfo comp enable */
	bool has_trigger_info;	/**< indicator of trigger info */
	uint8 pwr_offset[4];	/**< Final power offset propogated from ucode */
	uint8 bfm_mu[16];	/**< Per user bfm */
	uint8 usridx_mu[16]; /**< Per user drop indicator */
	uint8 ruidx_mu[16];	/**< Per user ru idx */
	uint8 mcs_nss_mu[16];	/**< Per user mcs/nss */
	bool ldpc_mu[16];	/**< Per user coding */
	uint16 aid_mu[16];	/**< Per user aid, dcm, power loading */
	uint8 rl_backoff;	/**< Regulatory limit backoff under partial loading ofdma */
} BWL_POST_PACKED_STRUCT tx_ctrl_rpt_t;
#include <packed_section_end.h>

#include <packed_section_start.h>
typedef BWL_PRE_PACKED_STRUCT struct {
	struct ipv4_addr	ipv4_addr;
	struct ether_addr nexthop;
} BWL_POST_PACKED_STRUCT ibss_route_entry_t;
#include <packed_section_end.h>

#include <packed_section_start.h>
typedef BWL_PRE_PACKED_STRUCT struct {
	uint32 num_entry;
	ibss_route_entry_t route_entry[1];
} BWL_POST_PACKED_STRUCT ibss_route_tbl_t;
#include <packed_section_end.h>

#define MAX_IBSS_ROUTE_TBL_ENTRY	64

#define TXPWR_TARGET_VERSION  0
#include <packed_section_start.h>
typedef BWL_PRE_PACKED_STRUCT struct {
	int32 version;		/**< version number */
	chanspec_t chanspec;	/**< txpwr report for this channel */
	int8 txpwr[WL_STA_ANT_MAX]; /**< Max tx target power, in qdb */
	uint8 rf_cores;		/**< count of RF Cores being reported */
} BWL_POST_PACKED_STRUCT txpwr_target_max_t;
#include <packed_section_end.h>

#define TXPWR_ADJ_EST_VERSION  1
#include <packed_section_start.h>
typedef BWL_PRE_PACKED_STRUCT struct {
	int32 version;		/* version number */
	chanspec_t chanspec;	/* estpwr report for this channel */
	int8 estpwr[WL_STA_ANT_MAX]; /* Max est power, in qdb */
	uint8 rf_cores;		/* count of RF Cores being reported */
} BWL_POST_PACKED_STRUCT txpwr_adj_est_t;
#include <packed_section_end.h>

#define BSS_PEER_INFO_PARAM_CUR_VER	0
/** Input structure for IOV_BSS_PEER_INFO */
#include <packed_section_start.h>
typedef BWL_PRE_PACKED_STRUCT	struct {
	uint16			version;
	struct	ether_addr ea;	/**< peer MAC address */
} BWL_POST_PACKED_STRUCT bss_peer_info_param_t;
#include <packed_section_end.h>

#define BSS_PEER_INFO_CUR_VER		0

#include <packed_section_start.h>
typedef BWL_PRE_PACKED_STRUCT struct {
	uint16			version;
	struct ether_addr	ea;
	int32			rssi;
	uint32			tx_rate;	/**< current tx rate */
	uint32			rx_rate;	/**< current rx rate */
	wl_rateset_t		rateset;	/**< rateset in use */
	uint32			age;		/**< age in seconds */
} BWL_POST_PACKED_STRUCT bss_peer_info_t;
#include <packed_section_end.h>

#define BSS_PEER_LIST_INFO_CUR_VER	0

#include <packed_section_start.h>
typedef BWL_PRE_PACKED_STRUCT struct {
	uint16			version;
	uint16			bss_peer_info_len;	/**< length of bss_peer_info_t */
	uint32			count;			/**< number of peer info */
	bss_peer_info_t		peer_info[1];		/**< peer info */
} BWL_POST_PACKED_STRUCT bss_peer_list_info_t;
#include <packed_section_end.h>

#define BSS_PEER_LIST_INFO_FIXED_LEN OFFSETOF(bss_peer_list_info_t, peer_info)

#define AIBSS_BCN_FORCE_CONFIG_VER_0	0

/** structure used to configure AIBSS beacon force xmit */
#include <packed_section_start.h>
typedef BWL_PRE_PACKED_STRUCT struct {
	uint16  version;
	uint16	len;
	uint32 initial_min_bcn_dur;	/**< dur in ms to check a bcn in bcn_flood period */
	uint32 min_bcn_dur;	/**< dur in ms to check a bcn after bcn_flood period */
	uint32 bcn_flood_dur; /**< Initial bcn xmit period in ms */
} BWL_POST_PACKED_STRUCT aibss_bcn_force_config_t;
#include <packed_section_end.h>

#define AIBSS_TXFAIL_CONFIG_VER_0    0
#define AIBSS_TXFAIL_CONFIG_VER_1    1
#define AIBSS_TXFAIL_CONFIG_CUR_VER		AIBSS_TXFAIL_CONFIG_VER_1

/** structure used to configure aibss tx fail event */
#include <packed_section_start.h>
typedef BWL_PRE_PACKED_STRUCT struct {
	uint16  version;
	uint16  len;
	uint32 bcn_timeout;     /**< dur in seconds to receive 1 bcn */
	uint32 max_tx_retry;     /**< no of consecutive no acks to send txfail event */
	uint32 max_atim_failure; /**< no of consecutive atim failure */
} BWL_POST_PACKED_STRUCT aibss_txfail_config_t;
#include <packed_section_end.h>

#include <packed_section_start.h>
typedef BWL_PRE_PACKED_STRUCT struct wl_aibss_if {
	uint16 version;
	uint16 len;
	uint32 flags;
	struct ether_addr addr;
	chanspec_t chspec;
} BWL_POST_PACKED_STRUCT wl_aibss_if_t;
#include <packed_section_end.h>

#include <packed_section_start.h>
typedef BWL_PRE_PACKED_STRUCT struct wlc_ipfo_route_entry {
	struct ipv4_addr ip_addr;
	struct ether_addr nexthop;
} BWL_POST_PACKED_STRUCT wlc_ipfo_route_entry_t;
#include <packed_section_end.h>

#include <packed_section_start.h>
typedef BWL_PRE_PACKED_STRUCT struct wlc_ipfo_route_tbl {
	uint32 num_entry;
	wlc_ipfo_route_entry_t route_entry[1];
} BWL_POST_PACKED_STRUCT wlc_ipfo_route_tbl_t;
#include <packed_section_end.h>

/* Version of wlc_btc_stats_t structure.
 * Increment whenever a change is made to wlc_btc_stats_t
 */
#define BTCX_STATS_VER_3 3

typedef struct wlc_btc_stats_v3 {
	uint16 version; /* version number of struct */
	uint16 valid; /* Size of this struct */
	uint32 stats_update_timestamp;	/* tStamp when data is updated. */
	uint32 btc_status; /* Hybrid/TDM indicator: Bit2:Hybrid, Bit1:TDM,Bit0:CoexEnabled */
	uint32 bt_req_type_map; /* BT Antenna Req types since last stats sample */
	uint32 bt_req_cnt; /* #BT antenna requests since last stats sampl */
	uint32 bt_gnt_cnt; /* #BT antenna grants since last stats sample */
	uint32 bt_gnt_dur; /* usec BT owns antenna since last stats sample */
	uint16 bt_abort_cnt; /* #Times WL was preempted due to BT since WL up */
	uint16 bt_rxf1ovfl_cnt; /* #Time PSNULL retry count exceeded since WL up */
	uint16 bt_latency_cnt; /* #Time ucode high latency detected since WL up */
	uint16 rsvd; /* pad to align struct to 32bit bndry	 */
	uint16 bt_succ_pm_protect_cnt; /* successful PM protection */
	uint16 bt_succ_cts_cnt; /* successful CTS2A protection */
	uint16 bt_wlan_tx_preempt_cnt; /* WLAN TX Preemption */
	uint16 bt_wlan_rx_preempt_cnt; /* WLAN RX Preemption */
	uint16 bt_ap_tx_after_pm_cnt; /* AP TX even after PM protection */
	uint16 bt_peraud_cumu_gnt_cnt; /* Grant cnt for periodic audio */
	uint16 bt_peraud_cumu_deny_cnt; /* Deny cnt for periodic audio */
	uint16 bt_a2dp_cumu_gnt_cnt; /* Grant cnt for A2DP */
	uint16 bt_a2dp_cumu_deny_cnt; /* Deny cnt for A2DP */
	uint16 bt_sniff_cumu_gnt_cnt; /* Grant cnt for Sniff */
	uint16 bt_sniff_cumu_deny_cnt; /* Deny cnt for Sniff */
	uint8 pad; /* Padding */
	uint8 slice_index; /* Slice to report */
} wlc_btc_stats_v3_t;

#define BTCX_STATS_VER_2 2

typedef struct wlc_btc_stats_v2 {
	uint16 version; /* version number of struct */
	uint16 valid; /* Size of this struct */
	uint32 stats_update_timestamp;	/* tStamp when data is updated. */
	uint32 btc_status; /* Hybrid/TDM indicator: Bit2:Hybrid, Bit1:TDM,Bit0:CoexEnabled */
	uint32 bt_req_type_map; /* BT Antenna Req types since last stats sample */
	uint32 bt_req_cnt; /* #BT antenna requests since last stats sampl */
	uint32 bt_gnt_cnt; /* #BT antenna grants since last stats sample */
	uint32 bt_gnt_dur; /* usec BT owns antenna since last stats sample */
	uint16 bt_abort_cnt; /* #Times WL was preempted due to BT since WL up */
	uint16 bt_rxf1ovfl_cnt; /* #Time PSNULL retry count exceeded since WL up */
	uint16 bt_latency_cnt; /* #Time ucode high latency detected since WL up */
	uint16 rsvd; /* pad to align struct to 32bit bndry	 */
} wlc_btc_stats_v2_t;

#define WL_IPFO_ROUTE_TBL_FIXED_LEN 4
#define WL_MAX_IPFO_ROUTE_TBL_ENTRY	64

	/* Global ASSERT Logging */
#define ASSERTLOG_CUR_VER	0x0100
#define MAX_ASSRTSTR_LEN	64

	typedef struct assert_record {
		uint32 time;
		uint8 seq_num;
		int8 str[MAX_ASSRTSTR_LEN];
	} assert_record_t;

	typedef struct assertlog_results {
		uint16 version;
		uint16 record_len;
		uint32 num;
		assert_record_t logs[1];
	} assertlog_results_t;

#define LOGRRC_FIX_LEN	8
#define IOBUF_ALLOWED_NUM_OF_LOGREC(type, len) ((len - LOGRRC_FIX_LEN)/sizeof(type))
#ifndef LINUX_POSTMOGRIFY_REMOVAL
/* BCMWAPI_WAI */
#define IV_LEN 16
	struct wapi_sta_msg_t
	{
		uint16	msg_type;
		uint16	datalen;
		uint8	vap_mac[6];
		uint8	reserve_data1[2];
		uint8	sta_mac[6];
		uint8	reserve_data2[2];
		uint8	gsn[IV_LEN];
		uint8	wie[256];
	};
/* #endif BCMWAPI_WAI */
#endif /* LINUX_POSTMOGRIFY_REMOVAL */
	/* chanim acs record */
	typedef struct {
		uint8 valid;
		uint8 trigger;
		chanspec_t selected_chspc;
		int8 bgnoise;
		uint32 glitch_cnt;
		uint8 ccastats;
		uint8 chan_idle;
		uint32 timestamp;
	} chanim_acs_record_t;

	typedef struct {
		chanim_acs_record_t acs_record[CHANIM_ACS_RECORD];
		uint8 count;
		uint32 timestamp;
	} wl_acs_record_t;

#define CCASTATS_V2_MAX 9
typedef struct chanim_stats_v2 {
	uint32 glitchcnt;               /**< normalized as per second count */
	uint32 badplcp;                 /**< normalized as per second count */
	uint8 ccastats[CCASTATS_V2_MAX];   /**< normalized as 0-255 */
	int8 bgnoise;                   /**< background noise level (in dBm) */
	chanspec_t chanspec;            /**< ctrl chanspec of the interface */
	uint32 timestamp;               /**< time stamp at which the stats are collected */
	uint32 bphy_glitchcnt;          /**< normalized as per second count */
	uint32 bphy_badplcp;            /**< normalized as per second count */
	uint8 chan_idle;                /**< normalized as 0~255 */
	uint8 PAD[3];
} chanim_stats_v2_t;

typedef struct chanim_stats {
	uint32 glitchcnt;               /**< normalized as per second count */
	uint32 badplcp;                 /**< normalized as per second count */
	uint8 ccastats[CCASTATS_MAX];   /**< normalized as 0-255 */
	int8 bgnoise;                   /**< background noise level (in dBm) */
	chanspec_t chanspec;            /**< ctrl chanspec of the interface */
	uint32 timestamp;               /**< time stamp at which the stats are collected */
	uint32 bphy_glitchcnt;          /**< normalized as per second count */
	uint32 bphy_badplcp;            /**< normalized as per second count */
	uint8 chan_idle;                /**< normalized as 0~255 */
	uint8 PAD[3];
} chanim_stats_t;

typedef struct chanim_stats_us {
	uint32 total_tm;           /* total time radio was actiave on channel */
	uint32 busy_tm;            /* time channel is busy reciveing traffic */
	uint32 tx_tm;              /* time radio is received traffic on channel */
	uint32 rx_obss;            /* time radio received traffic from overlaping bss */
	uint32 rx_bss;             /* time radio received traffic from connected clients */
	uint32 rxcrs_pri20;        /* rx crs primary 20 */
	uint32 rxcrs_sec20;        /* rx crs secondary 20 */
	uint32 rxcrs_sec40;        /* rx crs secondary 40 */
	uint32 rxcrs_sec80;        /* rx crs secondary 80 */
	chanspec_t chanspec;       /* ctrl chanspec of the interface */
} chanim_stats_us_t;

typedef struct chanim_stats_us_v1 {
	uint32 total_tm;           /* total time radio was actiave on channel */
	uint32 busy_tm;            /* time channel is busy reciveing traffic */
	uint32 ccastats_us[CCASTATS_MAX];  /* in microsecond */
	uint32 rxcrs_pri20;        /* rx crs primary 20 */
	uint32 rxcrs_sec20;        /* rx crs secondary 20 */
	uint32 rxcrs_sec40;        /* rx crs secondary 40 */
	uint32 rxcrs_sec80;        /* rx crs secondary 80 */
	chanspec_t chanspec;       /* ctrl chanspec of the interface */
} chanim_stats_us_v1_t;

typedef struct chanim_stats_us_v2 {
	uint64 total_tm;           /* total time radio was actiave on channel */
	uint64 busy_tm;            /* time channel is busy reciveing traffic */
	uint64 ccastats_us[CCASTATS_MAX];  /* in microsecond */
	uint64 rxcrs_pri20;        /* rx crs primary 20 */
	uint64 rxcrs_sec20;        /* rx crs secondary 20 */
	uint64 rxcrs_sec40;        /* rx crs secondary 40 */
	uint64 rxcrs_sec80;        /* rx crs secondary 80 */
	chanspec_t chanspec;       /* ctrl chanspec of the interface */
} chanim_stats_us_v2_t;
#define WL_CHANIM_STATS_V2 2
#define WL_CHANIM_STATS_VERSION_V3 3
#define WL_CHANIM_BUF_LEN                  (5*1024)
#define WL_CHANIM_STATS_VERSION_2          2
#define WL_CHANIM_STATS_VERSION_3          3
#define WL_CHANIM_STATS_VERSION            WL_CHANIM_STATS_VERSION_3
#define WL_CHANIM_READ_VERSION		   0x00
#define WL_CHANIM_STATS_US_VERSION_1       1
#define WL_CHANIM_STATS_US_VERSION_2       2
#define WL_CHANIM_STATS_US_VERSION         WL_CHANIM_STATS_US_VERSION_2

typedef struct {
	uint32 buflen;
	uint32 version;
	uint32 count;
	chanim_stats_t stats[1];
} wl_chanim_stats_t;

typedef struct {
	uint32 buflen;
	uint32 count;
	chanim_stats_us_t stats_us[1];
} wl_chanim_stats_us_t;

typedef struct {
	uint32 buflen;
	uint32 version;
	uint32 count;
	uint32 dur;
	chanim_stats_us_v1_t stats_us_v1[1];
} wl_chanim_stats_us_v1_t;

typedef struct {
	uint32 buflen;
	uint32 version;
	uint32 count;
	uint32 dur;
	chanim_stats_us_v2_t stats_us_v2[];
} wl_chanim_stats_us_v2_t;

#define WL_CHANIM_STATS_FIXED_LEN OFFSETOF(wl_chanim_stats_t, stats)
#define WL_CHANIM_STATS_US_FIXED_LEN OFFSETOF(wl_chanim_stats_us_v2_t, stats_us_v2)

/** Noise measurement metrics. */
#define NOISE_MEASURE_KNOISE	0x1

/** scb probe parameter */
typedef struct {
	uint32 scb_timeout;
	uint32 scb_activity_time;
	uint32 scb_max_probe;
} wl_scb_probe_t;

/* structure/defines for selective mgmt frame (smf) stats support */

#define SMFS_VERSION 1
/** selected mgmt frame (smf) stats element */
typedef struct wl_smfs_elem {
	uint32 count;
	uint16 code;  /**< SC or RC code */
	uint8 PAD[2];
} wl_smfs_elem_t;

typedef struct wl_smf_stats {
	uint32 version;
	uint16 length;	/**< reserved for future usage */
	uint8 type;
	uint8 codetype;
	uint32 ignored_cnt;
	uint32 malformed_cnt;
	uint32 count_total; /**< count included the interested group */
	wl_smfs_elem_t elem[1];
} wl_smf_stats_t;

#define WL_SET_CHAN_INFO_VER 1
#define WL_SET_CHAN_INFO_TYPE 1
typedef struct wl_set_chan_info {
	uint16 version;
	uint16 length;
	uint16 type;
	uint16 chanspec;
	uint32 per_chan_info;
} wl_set_chan_info_t;

#define WL_SMFSTATS_FIXED_LEN OFFSETOF(wl_smf_stats_t, elem);

enum {
	SMFS_CODETYPE_SC,
	SMFS_CODETYPE_RC
};

typedef enum smfs_type {
	SMFS_TYPE_AUTH,
	SMFS_TYPE_ASSOC,
	SMFS_TYPE_REASSOC,
	SMFS_TYPE_DISASSOC_TX,
	SMFS_TYPE_DISASSOC_RX,
	SMFS_TYPE_DEAUTH_TX,
	SMFS_TYPE_DEAUTH_RX,
	SMFS_TYPE_MAX
} smfs_type_t;

/* #ifdef PHYMON */

#define PHYMON_VERSION 1

typedef struct wl_phycal_core_state {
	/* Tx IQ/LO calibration coeffs */
	int16 tx_iqlocal_a;
	int16 tx_iqlocal_b;
	int8 tx_iqlocal_ci;
	int8 tx_iqlocal_cq;
	int8 tx_iqlocal_di;
	int8 tx_iqlocal_dq;
	int8 tx_iqlocal_ei;
	int8 tx_iqlocal_eq;
	int8 tx_iqlocal_fi;
	int8 tx_iqlocal_fq;

	/** Rx IQ calibration coeffs */
	int16 rx_iqcal_a;
	int16 rx_iqcal_b;

	uint8 tx_iqlocal_pwridx; /**< Tx Power Index for Tx IQ/LO calibration */
	uint8 PAD[3];
	uint32 papd_epsilon_table[64]; /**< PAPD epsilon table */
	int16 papd_epsilon_offset; /**< PAPD epsilon offset */
	uint8 curr_tx_pwrindex; /**< Tx power index */
	int8 idle_tssi; /**< Idle TSSI */
	int8 est_tx_pwr; /**< Estimated Tx Power (dB) */
	int8 est_rx_pwr; /**< Estimated Rx Power (dB) from RSSI */
	uint16 rx_gaininfo; /**< Rx gain applied on last Rx pkt */
	uint16 init_gaincode; /**< initgain required for ACI */
	int8 estirr_tx;
	int8 estirr_rx;
} wl_phycal_core_state_t;

typedef struct wl_phycal_state {
	int32 version;
	int8 num_phy_cores; /**< number of cores */
	int8 curr_temperature; /**< on-chip temperature sensor reading */
	chanspec_t chspec; /**< channspec for this state */
	uint8 aci_state; /**< ACI state: ON/OFF */
	uint8 PAD;
	uint16 crsminpower; /**< crsminpower required for ACI */
	uint16 crsminpowerl; /**< crsminpowerl required for ACI */
	uint16 crsminpoweru; /**< crsminpoweru required for ACI */
	wl_phycal_core_state_t phycal_core[1];
} wl_phycal_state_t;

#define WL_PHYCAL_STAT_FIXED_LEN OFFSETOF(wl_phycal_state_t, phycal_core)
/* endif PHYMON */

/** discovery state */
typedef struct wl_p2p_disc_st {
	uint8 state;	/**< see state */
	uint8 PAD;
	chanspec_t chspec;	/**< valid in listen state */
	uint16 dwell;	/**< valid in listen state, in ms */
} wl_p2p_disc_st_t;

/** scan request */
typedef struct wl_p2p_scan {
	uint8 type;		/**< 'S' for WLC_SCAN, 'E' for "escan" */
	uint8 reserved[3];
	/* scan or escan parms... */
} wl_p2p_scan_t;

/** i/f request */
typedef struct wl_p2p_if {
	struct ether_addr addr;
	uint8 type;	/**< see i/f type */
	uint8 PAD;
	chanspec_t chspec;	/**< for p2p_ifadd GO */
} wl_p2p_if_t;

/** i/f query */
typedef struct wl_p2p_ifq {
	uint32 bsscfgidx;
	char ifname[BCM_MSG_IFNAME_MAX];
} wl_p2p_ifq_t;

/** OppPS & CTWindow */
typedef struct wl_p2p_ops {
	uint8 ops;	/**< 0: disable 1: enable */
	uint8 ctw;	/**< >= 10 */
} wl_p2p_ops_t;

/** absence and presence request */
typedef struct wl_p2p_sched_desc {
	uint32 start;
	uint32 interval;
	uint32 duration;
	uint32 count;	/**< see count */
} wl_p2p_sched_desc_t;

typedef struct wl_p2p_sched {
	uint8 type;	/**< see schedule type */
	uint8 action;	/**< see schedule action */
	uint8 option;	/**< see schedule option */
	uint8 PAD;
	wl_p2p_sched_desc_t desc[1];
} wl_p2p_sched_t;

typedef struct wl_p2p_wfds_hash {
	uint32	advt_id;
	uint16	nw_cfg_method;
	uint8	wfds_hash[6];
	uint8	name_len;
	uint8	service_name[MAX_WFDS_SVC_NAME_LEN];
	uint8	PAD[3];
} wl_p2p_wfds_hash_t;

typedef struct wl_p2p_config_params {
	uint16		enable;			/**< 0: disable 1: enable */
	uint16		chanspec;		/* GO chanspec */
	wlc_ssid_t	ssid;			/* SSID */
} wl_p2p_config_params_t;

typedef struct wl_bcmdcs_data {
	uint32 reason;
	chanspec_t chspec;
	uint8	PAD[2];
} wl_bcmdcs_data_t;
#ifndef LINUX_POSTMOGRIFY_REMOVAL
/* ifdef EXT_STA */
/**
 * Format of IHV data passed to OID_DOT11_NIC_SPECIFIC_EXTENSION.
 */
typedef struct _IHV_NIC_SPECIFIC_EXTENSION {
	uint8			oui[4];     /**< vendor specific OUI value */
	uint32			event;      /**< event code */
	uint8			ihvData[1];    /**< ihv data */
} IHV_NIC_SPECIFIC_EXTENSION, *PIHV_NIC_SPECIFIC_EXTENSION;
#define IHV_NIC_SPECIFIC_EXTENTION_HEADER	OFFSETOF(IHV_NIC_SPECIFIC_EXTENSION, ihvData[0])
/* EXT_STA */
#endif /* LINUX_POSTMOGRIFY_REMOVAL */
/** NAT configuration */
typedef struct {
	uint32 ipaddr;		/**< interface ip address */
	uint32 ipaddr_mask;	/**< interface ip address mask */
	uint32 ipaddr_gateway;	/**< gateway ip address */
	uint8 mac_gateway[6];	/**< gateway mac address */
	uint8	PAD[2];
	uint32 ipaddr_dns;	/**< DNS server ip address, valid only for public if */
	uint8 mac_dns[6];	/**< DNS server mac address,  valid only for public if */
	uint8 GUID[38];		/**< interface GUID */
} nat_if_info_t;

typedef struct {
	uint32 op;		/**< operation code */
	uint8 pub_if;		/**< set for public if, clear for private if */
	uint8	PAD[3];
	nat_if_info_t if_info;	/**< interface info */
} nat_cfg_t;

typedef struct {
	int32 state;	/**< NAT state returned */
} nat_state_t;

typedef struct flush_txfifo {
	uint32 txfifobmp;
	uint32 hwtxfifoflush;
	struct ether_addr ea;
	uint8	PAD[2];
} flush_txfifo_t;

enum {
	SPATIAL_MODE_2G_IDX = 0,
	SPATIAL_MODE_5G_LOW_IDX,
	SPATIAL_MODE_5G_MID_IDX,
	SPATIAL_MODE_5G_HIGH_IDX,
	SPATIAL_MODE_5G_UPPER_IDX,
	SPATIAL_MODE_MAX_IDX
};

#define WLC_TXCORE_MAX		4	/**< max number of txcore supports */
#define WLC_TXCORE_MAX_OLD	2	/**< backward compatibilty for TXCAL */
#define WLC_SUBBAND_MAX		4	/**< max number of sub-band supports */
typedef struct {
	uint8	band2g[WLC_TXCORE_MAX];
	uint8	band5g[WLC_SUBBAND_MAX][WLC_TXCORE_MAX];
} sar_limit_t;

typedef struct tx_inst_power_percore {
	uint8 txpwr_est_Pout[WLC_TXCORE_MAX];	/**< Latest tx pwr estimate */
} tx_inst_power_percore_t;

#define MAX_NUM_TXCAL_MEAS 128
#define MAX_NUM_PWR_STEP 40
#define TXCAL_IOVAR_VERSION	0x1

/* below are used for bphy/ofdm separated LSC */
#define TXCAL_PWR_BPHY    0
#define TXCAL_PWR_OFDM    1

typedef struct wl_txcal_meas_percore {
	uint16 tssi[MAX_NUM_TXCAL_MEAS];
	int16 pwr[MAX_NUM_TXCAL_MEAS];
} wl_txcal_meas_percore_t;

typedef struct wl_txcal_meas_ncore {
	uint16 version;
	uint8 valid_cnt;
	uint8 num_core;
	wl_txcal_meas_percore_t txcal_percore[1];
} wl_txcal_meas_ncore_t;

typedef struct wl_txcal_power_tssi_percore {
	int16 tempsense;
	int16 pwr_start;
	uint8 pwr_start_idx;
	uint8 num_entries;
	uint16 pad;
	uint8 tssi[MAX_NUM_PWR_STEP];
} wl_txcal_power_tssi_percore_t;

typedef struct wl_txcal_power_tssi_ncore {
	uint16 version;
	uint8 set_core;
	uint8 channel;
	uint8 num_core;
	uint8 gen_tbl;
	uint8 ofdm;
	uint8 pad;
	wl_txcal_power_tssi_percore_t tssi_percore[4];
} wl_txcal_power_tssi_ncore_t;

typedef struct wl_txcal_meas {
	uint16 tssi[WLC_TXCORE_MAX][MAX_NUM_TXCAL_MEAS];
	int16 pwr[WLC_TXCORE_MAX][MAX_NUM_TXCAL_MEAS];
	uint8 valid_cnt;
	uint8 PAD;
} wl_txcal_meas_t;

typedef struct wl_txcal_meas_old {
	uint16 tssi[WLC_TXCORE_MAX_OLD][MAX_NUM_TXCAL_MEAS];
	int16 pwr[WLC_TXCORE_MAX_OLD][MAX_NUM_TXCAL_MEAS];
	uint8 valid_cnt;
	uint8 PAD;
} wl_txcal_meas_old_t;

typedef struct wl_txcal_power_tssi {
	uint8 set_core;
	uint8 channel;
	int16 tempsense[WLC_TXCORE_MAX];
	int16 pwr_start[WLC_TXCORE_MAX];
	uint8 pwr_start_idx[WLC_TXCORE_MAX];
	uint8 num_entries[WLC_TXCORE_MAX];
	uint8 tssi[WLC_TXCORE_MAX][MAX_NUM_PWR_STEP];
	uint8 gen_tbl;
	uint8 ofdm;
} wl_txcal_power_tssi_t;

typedef struct wl_txcal_power_tssi_old {
	uint8 set_core;
	uint8 channel;
	int16 tempsense[WLC_TXCORE_MAX_OLD];
	int16 pwr_start[WLC_TXCORE_MAX_OLD];
	uint8 pwr_start_idx[WLC_TXCORE_MAX_OLD];
	uint8 num_entries[WLC_TXCORE_MAX_OLD];
	uint8 tssi[WLC_TXCORE_MAX_OLD][MAX_NUM_PWR_STEP];
	uint8 gen_tbl;
	uint8 ofdm;
} wl_txcal_power_tssi_old_t;

typedef struct wl_olpc_pwr {
	uint16 version;
	uint8 core;
	uint8 channel;
	int16 tempsense;
	uint8 olpc_idx;
	uint8 ofdm;
} wl_olpc_pwr_t;

/** IOVAR "mempool" parameter. Used to retrieve a list of memory pool statistics. */
typedef struct wl_mempool_stats {
	int32 num;		/**< Number of memory pools */
	bcm_mp_stats_t s[1];	/**< Variable array of memory pool stats. */
} wl_mempool_stats_t;

typedef struct {
	uint32 ipaddr;
	uint32 ipaddr_netmask;
	uint32 ipaddr_gateway;
} nwoe_ifconfig_t;

/* Below to be removed after K branch */
/** Traffic management priority classes */
typedef enum trf_mgmt_priority_class {
	trf_mgmt_priority_low           = 0,        /**< Maps to 802.1p BK */
	trf_mgmt_priority_medium        = 1,        /**< Maps to 802.1p BE */
	trf_mgmt_priority_high          = 2,        /**< Maps to 802.1p VI */
	trf_mgmt_priority_nochange	= 3,	    /**< do not update the priority */
	trf_mgmt_priority_invalid       = (trf_mgmt_priority_nochange + 1)
} trf_mgmt_priority_class_t;

/** Traffic management configuration parameters */
typedef struct trf_mgmt_config {
	uint32  trf_mgmt_enabled;                           /**< 0 - disabled, 1 - enabled */
	uint32  flags;                                      /**< See TRF_MGMT_FLAG_xxx defines */
	uint32  host_ip_addr;                              /**< My IP address to determine subnet */
	uint32  host_subnet_mask;                           /**< My subnet mask */
	uint32  downlink_bandwidth;                         /**< In units of kbps */
	uint32  uplink_bandwidth;                           /**< In units of kbps */
	uint32  min_tx_bandwidth[TRF_MGMT_MAX_PRIORITIES];  /**< Minimum guaranteed tx bandwidth */
	uint32  min_rx_bandwidth[TRF_MGMT_MAX_PRIORITIES];  /**< Minimum guaranteed rx bandwidth */
} trf_mgmt_config_t;

/** Traffic management filter */
typedef struct trf_mgmt_filter {
	struct ether_addr           dst_ether_addr;         /**< His L2 address */
	uint8						PAD[2];
	uint32                      dst_ip_addr;            /**< His IP address */
	uint16                      dst_port;               /**< His L4 port */
	uint16                      src_port;               /**< My L4 port */
	uint16                      prot;                   /**< L4 protocol (only TCP or UDP) */
	uint16                      flags;                  /**< TBD. For now, this must be zero. */
	trf_mgmt_priority_class_t   priority;               /**< Priority for filtered packets */
	uint32                      dscp;                   /**< DSCP */
} trf_mgmt_filter_t;

/** Traffic management filter list (variable length) */
typedef struct trf_mgmt_filter_list     {
	uint32              num_filters;
	trf_mgmt_filter_t   filter[1];
} trf_mgmt_filter_list_t;

/** Traffic management global info used for all queues */
typedef struct trf_mgmt_global_info {
	uint32  maximum_bytes_per_second;
	uint32  maximum_bytes_per_sampling_period;
	uint32  total_bytes_consumed_per_second;
	uint32  total_bytes_consumed_per_sampling_period;
	uint32  total_unused_bytes_per_sampling_period;
} trf_mgmt_global_info_t;

/** Traffic management shaping info per priority queue */
typedef struct trf_mgmt_shaping_info {
	uint32  gauranteed_bandwidth_percentage;
	uint32  guaranteed_bytes_per_second;
	uint32  guaranteed_bytes_per_sampling_period;
	uint32  num_bytes_produced_per_second;
	uint32  num_bytes_consumed_per_second;
	uint32  num_queued_packets;                         /**< Number of packets in queue */
	uint32  num_queued_bytes;                           /**< Number of bytes in queue */
} trf_mgmt_shaping_info_t;

/** Traffic management shaping info array */
typedef struct trf_mgmt_shaping_info_array {
	trf_mgmt_global_info_t   tx_global_shaping_info;
	trf_mgmt_shaping_info_t  tx_queue_shaping_info[TRF_MGMT_MAX_PRIORITIES];
	trf_mgmt_global_info_t   rx_global_shaping_info;
	trf_mgmt_shaping_info_t  rx_queue_shaping_info[TRF_MGMT_MAX_PRIORITIES];
} trf_mgmt_shaping_info_array_t;

/** Traffic management statistical counters */
typedef struct trf_mgmt_stats {
	uint32  num_processed_packets;      /**< Number of packets processed */
	uint32  num_processed_bytes;        /**< Number of bytes processed */
	uint32  num_discarded_packets;      /**< Number of packets discarded from queue */
} trf_mgmt_stats_t;

/** Traffic management statistics array */
typedef struct trf_mgmt_stats_array {
	trf_mgmt_stats_t  tx_queue_stats[TRF_MGMT_MAX_PRIORITIES];
	trf_mgmt_stats_t  rx_queue_stats[TRF_MGMT_MAX_PRIORITIES];
} trf_mgmt_stats_array_t;
/* Above to be removed after K branch */

/* Both powersel_params and lpc_params are used by IOVAR lpc_params.
 * The powersel_params is replaced by lpc_params in later WLC versions.
 */
typedef struct powersel_params {
	/* LPC Params exposed via IOVAR */
	int32		tp_ratio_thresh;  /**< Throughput ratio threshold */
	uint8		rate_stab_thresh; /**< Thresh for rate stability based on nupd */
	uint8		pwr_stab_thresh; /**< Number of successes before power step down */
	uint8		pwr_sel_exp_time; /**< Time lapse for expiry of database */
	uint8		PAD;
} powersel_params_t;

#define WL_LPC_PARAMS_VER_2	2
#define WL_LPC_PARAMS_CURRENT_VERSION WL_LPC_PARAMS_VER_2

typedef struct lpc_params {
	uint16		version;
	uint16		length;
	/* LPC Params exposed via IOVAR */
	uint8		rate_stab_thresh; /**< Thresh for rate stability based on nupd */
	uint8		pwr_stab_thresh; /**< Number of successes before power step down */
	uint8		lpc_exp_time; /**< Time lapse for expiry of database */
	uint8		pwrup_slow_step; /**< Step size for slow step up */
	uint8		pwrup_fast_step; /**< Step size for fast step up */
	uint8		pwrdn_slow_step; /**< Step size for slow step down */
} lpc_params_t;

/* tx pkt delay statistics */
#define	SCB_RETRY_SHORT_DEF	7	/**< Default Short retry Limit */
#define WLPKTDLY_HIST_NBINS	16	/**< number of bins used in the Delay histogram */

/** structure to store per-AC delay statistics */
typedef struct scb_delay_stats {
	uint32 txmpdu_lost;	/**< number of MPDUs lost */
	uint32 txmpdu_cnt[SCB_RETRY_SHORT_DEF]; /**< retry times histogram */
	uint32 delay_sum[SCB_RETRY_SHORT_DEF]; /**< cumulative packet latency */
	uint32 delay_min;	/**< minimum packet latency observed */
	uint32 delay_max;	/**< maximum packet latency observed */
	uint32 delay_avg;	/**< packet latency average */
	uint32 delay_hist[WLPKTDLY_HIST_NBINS];	/**< delay histogram */
	uint32 delay_count;	/**< minimum number of time period units before
				consequent packet delay events can be generated
				*/
	uint32 prev_txmpdu_cnt;	/**< Previous value of txmpdu_cnt[] during last iteration */
	uint32 prev_delay_sum;	/**< Previous value of delay_sum[] during last iteration */
} scb_delay_stats_t;

/** structure for txdelay event */
typedef struct txdelay_event {
	uint8				status;
	uint8				PAD[3];
	int32				rssi;
	chanim_stats_t		chanim_stats;
	scb_delay_stats_t	delay_stats[AC_COUNT];
} txdelay_event_t;

/** structure for txdelay parameters */
typedef struct txdelay_params {
	uint16	ratio;	/**< Avg Txdelay Delta */
	uint8	cnt;	/**< Sample cnt */
	uint8	period;	/**< Sample period */
	uint8	tune;	/**< Debug */
	uint8	PAD;
} txdelay_params_t;
#ifndef LINUX_POSTMOGRIFY_REMOVAL
#define MAX_TXDELAY_STATS_SCBS 6
#define TXDELAY_STATS_VERSION 1

enum {
	TXDELAY_STATS_PARTIAL_RESULT = 0,
	TXDELAY_STATS_FULL_RESULT = 1
};

typedef struct scb_total_delay_stats {
	struct  ether_addr ea;
	uint8   pad[2];
	scb_delay_stats_t dlystats[AC_COUNT];
} scb_total_delay_stats_t;

typedef struct txdelay_stats {
	uint32  version;
	uint32  full_result;    /* 0:Partial, 1:full */
	uint32  scb_cnt;        /* in:requested, out:returned */
	scb_total_delay_stats_t scb_delay_stats[1];
} txdelay_stats_t;

#define WL_PKTQ_STATUS_VERSION 2
#define WL_PKTQ_HIST_NBINS     8
typedef struct scb_pktq_status {
	uint16  ver; /* structure  version */
	uint16  size;  /* pad for 4 byte struct alignment */
	struct  ether_addr ea;
	uint8   enable_delay_stat;
	uint8   pad;
} scb_pktq_status_t;

typedef struct pktq_delay_stats {
	uint32 delay_min;	/**< minimum packet latency observed */
	uint32 delay_max;	/**< maximum packet latency observed */
	uint32 delay_hist[WL_PKTQ_HIST_NBINS];	/**< delay histogram */
	uint32 delay_count;	/**< number of packet delay events generated */
} pktq_delay_stats_t;

#define WL_TXDELAY_STATS_FIXED_SIZE \
	(sizeof(txdelay_stats_t)+(MAX_TXDELAY_STATS_SCBS-1)*sizeof(scb_total_delay_stats_t))
#endif /* LINUX_POSTMOGRIFY_REMOVAL */
enum {
	WNM_SERVICE_DMS = 1,
	WNM_SERVICE_FMS = 2,
	WNM_SERVICE_TFS = 3
};

/** Definitions for WNM/NPS TCLAS */
typedef struct wl_tclas {
	uint8 user_priority;
	uint8 fc_len;
	dot11_tclas_fc_t fc;
} wl_tclas_t;

#define WL_TCLAS_FIXED_SIZE	OFFSETOF(wl_tclas_t, fc)

typedef struct wl_tclas_list {
	uint32 num;
	wl_tclas_t tclas[];
} wl_tclas_list_t;

/** Definitions for WNM/NPS Traffic Filter Service */
typedef struct wl_tfs_req {
	uint8 tfs_id;
	uint8 tfs_actcode;
	uint8 tfs_subelem_id;
	uint8 send;
} wl_tfs_req_t;

typedef struct wl_tfs_filter {
	uint8 status;			/**< Status returned by the AP */
	uint8 tclas_proc;		/**< TCLAS processing value (0:and, 1:or)  */
	uint8 tclas_cnt;		/**< count of all wl_tclas_t in tclas array */
	uint8 tclas[1];			/**< VLA of wl_tclas_t */
} wl_tfs_filter_t;
#define WL_TFS_FILTER_FIXED_SIZE	OFFSETOF(wl_tfs_filter_t, tclas)

typedef struct wl_tfs_fset {
	struct ether_addr ea;		/**< Address of AP/STA involved with this filter set */
	uint8 tfs_id;			/**< TFS ID field chosen by STA host */
	uint8 status;			/**< Internal status TFS_STATUS_xxx */
	uint8 actcode;			/**< Action code DOT11_TFS_ACTCODE_xxx */
	uint8 token;			/**< Token used in last request frame */
	uint8 notify;			/**< Notify frame sent/received because of this set */
	uint8 filter_cnt;		/**< count of all wl_tfs_filter_t in filter array */
	uint8 filter[1];		/**< VLA of wl_tfs_filter_t */
} wl_tfs_fset_t;
#define WL_TFS_FSET_FIXED_SIZE		OFFSETOF(wl_tfs_fset_t, filter)

enum {
	TFS_STATUS_DISABLED = 0,	/**< TFS filter set disabled by user */
	TFS_STATUS_DISABLING = 1,	/**< Empty request just sent to AP */
	TFS_STATUS_VALIDATED = 2,	/**< Filter set validated by AP (but maybe not enabled!) */
	TFS_STATUS_VALIDATING = 3,	/**< Filter set just sent to AP */
	TFS_STATUS_NOT_ASSOC = 4,	/**< STA not associated */
	TFS_STATUS_NOT_SUPPORT = 5,	/**< TFS not supported by AP */
	TFS_STATUS_DENIED = 6,		/**< Filter set refused by AP (=> all sets are disabled!) */
};

typedef struct wl_tfs_status {
	uint8 fset_cnt;			/**< count of all wl_tfs_fset_t in fset array */
	wl_tfs_fset_t fset[1];		/**< VLA of wl_tfs_fset_t */
} wl_tfs_status_t;

typedef struct wl_tfs_set {
	uint8 send;		/**< Immediatly register registered sets on AP side */
	uint8 tfs_id;		/**< ID of a specific set (existing or new), or nul for all */
	uint8 actcode;		/**< Action code for this filter set */
	uint8 tclas_proc;	/**< TCLAS processing operator for this filter set */
} wl_tfs_set_t;

typedef struct wl_tfs_term {
	uint8 del;			/**< Delete internal set once confirmation received */
	uint8 tfs_id;			/**< ID of a specific set (existing), or nul for all */
} wl_tfs_term_t;

#define DMS_DEP_PROXY_ARP (1 << 0)

/* Definitions for WNM/NPS Directed Multicast Service */
enum {
	DMS_STATUS_DISABLED = 0,	/**< DMS desc disabled by user */
	DMS_STATUS_ACCEPTED = 1,	/**< Request accepted by AP */
	DMS_STATUS_NOT_ASSOC = 2,	/**< STA not associated */
	DMS_STATUS_NOT_SUPPORT = 3,	/**< DMS not supported by AP */
	DMS_STATUS_DENIED = 4,		/**< Request denied by AP */
	DMS_STATUS_TERM = 5,		/**< Request terminated by AP */
	DMS_STATUS_REMOVING = 6,	/**< Remove request just sent */
	DMS_STATUS_ADDING = 7,		/**< Add request just sent */
	DMS_STATUS_ERROR = 8,		/**< Non compliant AP behvior */
	DMS_STATUS_IN_PROGRESS = 9,	/**< Request just sent */
	DMS_STATUS_REQ_MISMATCH = 10	/**< Conditions for sending DMS req not met */
};

typedef struct wl_dms_desc {
	uint8 user_id;
	uint8 status;
	uint8 token;
	uint8 dms_id;
	uint8 tclas_proc;
	uint8 mac_len;		/**< length of all ether_addr in data array, 0 if STA */
	uint8 tclas_len;	/**< length of all wl_tclas_t in data array */
	uint8 data[1];		/**< VLA of 'ether_addr' and 'wl_tclas_t' (in this order ) */
} wl_dms_desc_t;

#define WL_DMS_DESC_FIXED_SIZE	OFFSETOF(wl_dms_desc_t, data)

typedef struct wl_dms_status {
	uint32 cnt;
	wl_dms_desc_t desc[1];
} wl_dms_status_t;

typedef struct wl_dms_set {
	uint8 send;
	uint8 user_id;
	uint8 tclas_proc;
} wl_dms_set_t;

typedef struct wl_dms_term {
	uint8 del;
	uint8 user_id;
} wl_dms_term_t;

typedef struct wl_service_term {
	uint8 service;
	union {
		wl_dms_term_t dms;
	} u;
} wl_service_term_t;

/** Definitions for WNM/NPS BSS Transistion */
#define WL_BSSTRANS_QUERY_VERSION_1 1
typedef struct wl_bsstrans_query {
	uint16 version;   /* structure  version */
	uint16 pad0;  /* padding for 4-byte allignment */
	wlc_ssid_t ssid; /* SSID of NBR elem to be queried for */
	uint8 reason; /* Reason code of the BTQ */
	uint8 pad1[3];  /* padding for 4-byte allignment */
} wl_bsstrans_query_t;

#define BTM_QUERY_NBR_COUNT_MAX 16

#define WL_BTQ_NBR_LIST_VERSION_1 1
typedef struct wl_btq_nbr_list {
	uint16 version;   /* structure  version */
	uint8 count; /* No. of BTQ NBRs returned */
	uint8 pad;  /* padding for 4-byte allignment */
	nbr_rpt_elem_t btq_nbt_elem[]; /* BTQ NBR elem in a BTQ NBR list */
} wl_btq_nbr_list_t;

typedef struct wl_bsstrans_req {
	uint16 tbtt;			/**< time of BSS to end of life, in unit of TBTT */
	uint16 dur;			/**< time of BSS to keep off, in unit of minute */
	uint8 reqmode;			/**< request mode of BSS transition request */
	uint8 unicast;			/**< request by unicast or by broadcast */
	struct ether_addr sta_mac;	/* sta mac */
	uint8 retry_delay;		/* delay in second to wait before trying association */
	uint8 reason;
	uint8 token;                    /* Dialog token to be used */
} wl_bsstrans_req_t;

enum {
	BSSTRANS_RESP_AUTO = 0,		/**< Currently equivalent to ENABLE */
	BSSTRANS_RESP_DISABLE = 1,	/**< Never answer BSS Trans Req frames */
	BSSTRANS_RESP_ENABLE = 2,	/**< Always answer Req frames with preset data */
	BSSTRANS_RESP_WAIT = 3,		/**< Send ind, wait and/or send preset data (NOT IMPL) */
	BSSTRANS_RESP_IMMEDIATE = 4	/**< After an ind, set data and send resp (NOT IMPL) */
};

typedef struct wl_bsstrans_resp {
	uint8 policy;
	uint8 status;
	uint8 delay;
	struct ether_addr target;
} wl_bsstrans_resp_t;

/* "wnm_bsstrans_policy" argument programs behavior after BSSTRANS Req reception.
 * BSS-Transition feature is used by multiple programs such as NPS-PF, VE-PF,
 * Band-steering, Hotspot 2.0 and customer requirements. Each PF and its test plan
 * mandates different behavior on receiving BSS-transition request. To accomodate
 * such divergent behaviors these policies have been created.
 */
typedef enum {
	WL_BSSTRANS_POLICY_ROAM_ALWAYS = 0,	/**< Roam (or disassociate) in all cases */
	WL_BSSTRANS_POLICY_ROAM_IF_MODE = 1,	/**< Roam only if requested by Request Mode field */
	WL_BSSTRANS_POLICY_ROAM_IF_PREF = 2,	/**< Roam only if Preferred BSS provided */
	WL_BSSTRANS_POLICY_WAIT = 3,		/**< Wait for deauth and send Accepted status */
	WL_BSSTRANS_POLICY_PRODUCT = 4,	/**< Policy for real product use cases (Olympic) */
	WL_BSSTRANS_POLICY_PRODUCT_WBTEXT = 5,	/**< Policy for real product use cases (SS) */
	WL_BSSTRANS_POLICY_MBO = 6,   /**< Policy for MBO certification */
	WL_BSSTRANS_POLICY_MAX = 7
} wnm_bsstrans_policy_type_t;

/** Definitions for WNM/NPS TIM Broadcast */
typedef struct wl_timbc_offset {
	int16 offset;		/**< offset in us */
	uint16 fix_intv;	/**< override interval sent from STA */
	uint16 rate_override;	/**< use rate override to send high rate TIM broadcast frame */
	uint8 tsf_present;	/**< show timestamp in TIM broadcast frame */
	uint8 PAD;
} wl_timbc_offset_t;

typedef struct wl_timbc_set {
	uint8 interval;		/**< Interval in DTIM wished or required. */
	uint8 flags;		/**< Bitfield described below */
	uint16 rate_min;	/**< Minimum rate required for High/Low TIM frames. Optionnal */
	uint16 rate_max;	/**< Maximum rate required for High/Low TIM frames. Optionnal */
} wl_timbc_set_t;

enum {
	WL_TIMBC_SET_TSF_REQUIRED = 1,	/**< Enable TIMBC only if TSF in TIM frames */
	WL_TIMBC_SET_NO_OVERRIDE = 2,	/**< ... if AP does not override interval */
	WL_TIMBC_SET_PROXY_ARP = 4,	/**< ... if AP support Proxy ARP */
	WL_TIMBC_SET_DMS_ACCEPTED = 8	/**< ... if all DMS desc have been accepted */
};

typedef struct wl_timbc_status {
	uint8 status_sta;		/**< Status from internal state machine (check below) */
	uint8 status_ap;		/**< From AP response frame (check 8.4.2.86 from 802.11) */
	uint8 interval;
	uint8 pad;
	int32 offset;
	uint16 rate_high;
	uint16 rate_low;
} wl_timbc_status_t;

enum {
	WL_TIMBC_STATUS_DISABLE = 0,		/**< TIMBC disabled by user */
	WL_TIMBC_STATUS_REQ_MISMATCH = 1,	/**< AP settings do no match user requirements */
	WL_TIMBC_STATUS_NOT_ASSOC = 2,		/**< STA not associated */
	WL_TIMBC_STATUS_NOT_SUPPORT = 3,	/**< TIMBC not supported by AP */
	WL_TIMBC_STATUS_DENIED = 4,		/**< Req to disable TIMBC sent to AP */
	WL_TIMBC_STATUS_ENABLE = 5		/**< TIMBC enabled */
};

/** Definitions for PM2 Dynamic Fast Return To Sleep */
typedef struct wl_pm2_sleep_ret_ext {
	uint8  logic;			/**< DFRTS logic: see WL_DFRTS_LOGIC_* below */
	uint8  PAD;
	uint16 low_ms;			/**< Low FRTS timeout */
	uint16 high_ms;			/**< High FRTS timeout */
	uint16 rx_pkts_threshold;	/**< switching threshold: # rx pkts */
	uint16 tx_pkts_threshold;	/**< switching threshold: # tx pkts */
	uint16 txrx_pkts_threshold;	/**< switching threshold: # (tx+rx) pkts */
	uint32 rx_bytes_threshold;	/**< switching threshold: # rx bytes */
	uint32 tx_bytes_threshold;	/**< switching threshold: # tx bytes */
	uint32 txrx_bytes_threshold;	/**< switching threshold: # (tx+rx) bytes */
} wl_pm2_sleep_ret_ext_t;

#define WL_DFRTS_LOGIC_OFF	0	/**< Feature is disabled */
#define WL_DFRTS_LOGIC_OR	1	/**< OR all non-zero threshold conditions */
#define WL_DFRTS_LOGIC_AND	2	/**< AND all non-zero threshold conditions */

/* Values for the passive_on_restricted_mode iovar.  When set to non-zero, this iovar
 * disables automatic conversions of a channel from passively scanned to
 * actively scanned.  These values only have an effect for country codes such
 * as XZ where some 5 GHz channels are defined to be passively scanned.
 */
#define WL_PASSACTCONV_DISABLE_NONE	0	/**< Enable permanent and temporary conversions */
#define WL_PASSACTCONV_DISABLE_ALL	1	/**< Disable permanent and temporary conversions */
#define WL_PASSACTCONV_DISABLE_PERM	2	/**< Disable only permanent conversions */

/* Definitions for Reliable Multicast */
#define WL_RMC_CNT_VERSION	   1
#define WL_RMC_TR_VERSION	   1
#define WL_RMC_MAX_CLIENT	   32
#define WL_RMC_FLAG_INBLACKLIST	   1
#define WL_RMC_FLAG_ACTIVEACKER	   2
#define WL_RMC_FLAG_RELMCAST	   4
#define WL_RMC_MAX_TABLE_ENTRY     4

#define WL_RMC_VER		   1
#define WL_RMC_INDEX_ACK_ALL       255
#define WL_RMC_NUM_OF_MC_STREAMS   4
#define WL_RMC_MAX_TRS_PER_GROUP   1
#define WL_RMC_MAX_TRS_IN_ACKALL   1
#define WL_RMC_ACK_MCAST0          0x02
#define WL_RMC_ACK_MCAST_ALL       0x01
#define WL_RMC_ACTF_TIME_MIN       300		/**< time in ms */
#define WL_RMC_ACTF_TIME_MAX       20000	/**< time in ms */
#define WL_RMC_MAX_NUM_TRS	   32		/**< maximun transmitters allowed */
#define WL_RMC_ARTMO_MIN           350		/**< time in ms */
#define WL_RMC_ARTMO_MAX           40000	/**< time in ms */

/* RMC events in action frames */
enum rmc_opcodes {
	RELMCAST_ENTRY_OP_DISABLE = 0,   /**< Disable multi-cast group */
	RELMCAST_ENTRY_OP_DELETE  = 1,   /**< Delete multi-cast group */
	RELMCAST_ENTRY_OP_ENABLE  = 2,   /**< Enable multi-cast group */
	RELMCAST_ENTRY_OP_ACK_ALL = 3    /**< Enable ACK ALL bit in AMT */
};

/* RMC operational modes */
enum rmc_modes {
	WL_RMC_MODE_RECEIVER    = 0,	 /**< Receiver mode by default */
	WL_RMC_MODE_TRANSMITTER = 1,	 /**< Transmitter mode using wl ackreq */
	WL_RMC_MODE_INITIATOR   = 2	 /**< Initiator mode using wl ackreq */
};

/** Each RMC mcast client info */
typedef struct wl_relmcast_client {
	uint8 flag;			/**< status of client such as AR, R, or blacklisted */
	uint8 PAD;
	int16 rssi;			/**< rssi value of RMC client */
	struct ether_addr addr;		/**< mac address of RMC client */
} wl_relmcast_client_t;

/** RMC Counters */
typedef struct wl_rmc_cnts {
	uint16  version;		/**< see definition of WL_CNT_T_VERSION */
	uint16  length;			/**< length of entire structure */
	uint16	dupcnt;			/**< counter for duplicate rmc MPDU */
	uint16	ackreq_err;		/**< counter for wl ackreq error    */
	uint16	af_tx_err;		/**< error count for action frame transmit   */
	uint16	null_tx_err;		/**< error count for rmc null frame transmit */
	uint16	af_unicast_tx_err;	/**< error count for rmc unicast frame transmit */
	uint16	mc_no_amt_slot;		/**< No mcast AMT entry available */
	/* Unused. Keep for rom compatibility */
	uint16	mc_no_glb_slot;		/**< No mcast entry available in global table */
	uint16	mc_not_mirrored;	/**< mcast group is not mirrored */
	uint16	mc_existing_tr;		/**< mcast group is already taken by transmitter */
	uint16	mc_exist_in_amt;	/**< mcast group is already programmed in amt */
	/* Unused. Keep for rom compatibility */
	uint16	mc_not_exist_in_gbl;	/**< mcast group is not in global table */
	uint16	mc_not_exist_in_amt;	/**< mcast group is not in AMT table */
	uint16	mc_utilized;		/**< mcast addressed is already taken */
	uint16	mc_taken_other_tr;	/**< multi-cast addressed is already taken */
	uint32	rmc_rx_frames_mac;      /**< no of mc frames received from mac */
	uint32	rmc_tx_frames_mac;      /**< no of mc frames transmitted to mac */
	uint32	mc_null_ar_cnt;         /**< no. of times NULL AR is received */
	uint32	mc_ar_role_selected;	/**< no. of times took AR role */
	uint32	mc_ar_role_deleted;	/**< no. of times AR role cancelled */
	uint32	mc_noacktimer_expired;  /**< no. of times noack timer expired */
	uint16  mc_no_wl_clk;           /**< no wl clk detected when trying to access amt */
	uint16  mc_tr_cnt_exceeded;     /**< No of transmitters in the network exceeded */
} wl_rmc_cnts_t;

/** RMC Status */
typedef struct wl_relmcast_st {
	uint8         ver;		/**< version of RMC */
	uint8         num;		/**< number of clients detected by transmitter */
	wl_relmcast_client_t clients[WL_RMC_MAX_CLIENT];
	uint16        err;		/**< error status (used in infra) */
	uint16        actf_time;	/**< action frame time period */
} wl_relmcast_status_t;

/** Entry for each STA/node */
typedef struct wl_rmc_entry {
	/* operation on multi-cast entry such add,
	 * delete, ack-all
	 */
	int8    flag;
	struct ether_addr addr;		/**< multi-cast group mac address */
} wl_rmc_entry_t;

/** RMC table */
typedef struct wl_rmc_entry_table {
	uint8   index;			/**< index to a particular mac entry in table */
	uint8   opcode;			/**< opcodes or operation on entry */
	wl_rmc_entry_t entry[WL_RMC_MAX_TABLE_ENTRY];
} wl_rmc_entry_table_t;

typedef struct wl_rmc_trans_elem {
	struct ether_addr tr_mac;	/**< transmitter mac */
	struct ether_addr ar_mac;	/**< ar mac */
	uint16 artmo;			/**< AR timeout */
	uint8 amt_idx;			/**< amt table entry */
	uint8 PAD;
	uint16 flag;			/**< entry will be acked, not acked, programmed, full etc */
} wl_rmc_trans_elem_t;

/** RMC transmitters */
typedef struct wl_rmc_trans_in_network {
	uint8         ver;		/**< version of RMC */
	uint8         num_tr;		/**< number of transmitters in the network */
	wl_rmc_trans_elem_t trs[WL_RMC_MAX_NUM_TRS];
} wl_rmc_trans_in_network_t;

/** To update vendor specific ie for RMC */
typedef struct wl_rmc_vsie {
	uint8	oui[DOT11_OUI_LEN];
	uint8	PAD;
	uint16	payload;	/**< IE Data Payload */
} wl_rmc_vsie_t;

/* structures  & defines for proximity detection  */
enum proxd_method {
	PROXD_UNDEFINED_METHOD = 0,
	PROXD_RSSI_METHOD = 1,
	PROXD_TOF_METHOD = 2
};

/* structures for proximity detection device role */
#define WL_PROXD_MODE_DISABLE	0
#define WL_PROXD_MODE_NEUTRAL	1
#define WL_PROXD_MODE_INITIATOR	2
#define WL_PROXD_MODE_TARGET	3

#define WL_PROXD_ACTION_STOP		0
#define WL_PROXD_ACTION_START		1

#define WL_PROXD_FLAG_TARGET_REPORT	0x1
#define WL_PROXD_FLAG_REPORT_FAILURE	0x2
#define WL_PROXD_FLAG_INITIATOR_REPORT	0x4
#define WL_PROXD_FLAG_NOCHANSWT		0x8
#define WL_PROXD_FLAG_NETRUAL		0x10
#define WL_PROXD_FLAG_INITIATOR_RPTRTT	0x20
#define WL_PROXD_FLAG_ONEWAY		0x40
#define WL_PROXD_FLAG_SEQ_EN		0x80

#define WL_PROXD_SETFLAG_K		0x1
#define WL_PROXD_SETFLAG_N		0x2
#define WL_PROXD_SETFLAG_S		0x4

#define WL_PROXD_SETFLAG_K		0x1
#define WL_PROXD_SETFLAG_N		0x2
#define WL_PROXD_SETFLAG_S		0x4

#define WL_PROXD_RANDOM_WAKEUP	0x8000
#ifndef LINUX_POSTMOGRIFY_REMOVAL
#define WL_PROXD_MAXREPORT	8
#endif /* LINUX_POSTMOGRIFY_REMOVAL */

typedef struct wl_proxd_iovar {
	uint16	method;		/**< Proximity Detection method */
	uint16	mode;		/**< Mode (neutral, initiator, target) */
} wl_proxd_iovar_t;

/*
 * structures for proximity detection parameters
 * consists of two parts, common and method specific params
 * common params should be placed at the beginning
 */

typedef struct wl_proxd_params_common	{
	chanspec_t	chanspec;	/**< channel spec */
	int16		tx_power;	/**< tx power of Proximity Detection(PD) frames (in dBm) */
	uint16		tx_rate;	/**< tx rate of PD rames  (in 500kbps units) */
	uint16		timeout;	/**< timeout value */
	uint16		interval;	/**< interval between neighbor finding attempts (in TU) */
	uint16		duration;	/**< duration of neighbor finding attempts (in ms) */
} wl_proxd_params_common_t;

typedef struct wl_proxd_params_rssi_method {
	chanspec_t	chanspec;	/**< chanspec for home channel */
	int16		tx_power;	/**< tx power of Proximity Detection frames (in dBm) */
	uint16		tx_rate;	/**< tx rate of PD frames, 500kbps units */
	uint16		timeout;	/**< state machine wait timeout of the frames (in ms) */
	uint16		interval;	/**< interval between neighbor finding attempts (in TU) */
	uint16		duration;	/**< duration of neighbor finding attempts (in ms) */
					/* method specific ones go after this line */
	int16		rssi_thresh;	/**< RSSI threshold (in dBm) */
	uint16		maxconvergtmo;	/**< max wait converge timeout (in ms) */
} wl_proxd_params_rssi_method_t;

#define Q1_NS			25	/**< Q1 time units */

#define TOF_BW_NUM		3	/**< number of bandwidth that the TOF can support */
#define TOF_BW_SEQ_NUM		(TOF_BW_NUM+2)	/* number of total index */
enum tof_bw_index {
	TOF_BW_20MHZ_INDEX = 0,
	TOF_BW_40MHZ_INDEX = 1,
	TOF_BW_80MHZ_INDEX = 2,
	TOF_BW_SEQTX_INDEX = 3,
	TOF_BW_SEQRX_INDEX = 4
};

#define BANDWIDTH_BASE	20	/**< base value of bandwidth */
#define TOF_BW_20MHZ    (BANDWIDTH_BASE << TOF_BW_20MHZ_INDEX)
#define TOF_BW_40MHZ    (BANDWIDTH_BASE << TOF_BW_40MHZ_INDEX)
#define TOF_BW_80MHZ    (BANDWIDTH_BASE << TOF_BW_80MHZ_INDEX)
#define TOF_BW_10MHZ    10

#define NFFT_BASE		64	/**< base size of fft */
#define TOF_NFFT_20MHZ  (NFFT_BASE << TOF_BW_20MHZ_INDEX)
#define TOF_NFFT_40MHZ  (NFFT_BASE << TOF_BW_40MHZ_INDEX)
#define TOF_NFFT_80MHZ  (NFFT_BASE << TOF_BW_80MHZ_INDEX)

typedef struct wl_proxd_params_tof_method {
	chanspec_t	chanspec;	/**< chanspec for home channel */
	int16		tx_power;	/**< tx power of Proximity Detection(PD) frames (in dBm) */
	uint16		tx_rate;	/**< tx rate of PD rames  (in 500kbps units) */
	uint16		timeout;	/**< state machine wait timeout of the frames (in ms) */
	uint16		interval;	/**< interval between neighbor finding attempts (in TU) */
	uint16		duration;	/**< duration of neighbor finding attempts (in ms) */
	/* specific for the method go after this line */
	struct ether_addr tgt_mac;	/**< target mac addr for TOF method */
	uint16		ftm_cnt;	/**< number of the frames txed by initiator */
	uint16		retry_cnt;	/**< number of retransmit attampts for ftm frames */
	int16		vht_rate;	/**< ht or vht rate */
	/* add more params required for other methods can be added here  */
} wl_proxd_params_tof_method_t;

typedef struct wl_proxd_seq_config
{
	int16 N_tx_log2;
	int16 N_rx_log2;
	int16 N_tx_scale;
	int16 N_rx_scale;
	int16 w_len;
	int16 w_offset;
} wl_proxd_seq_config_t;

#define WL_PROXD_TUNE_VERSION_1		1
#define WL_PROXD_TUNE_VERSION_2		2
#include <packed_section_start.h>
/* For legacy ranging target (e.g. 43430, 43342) */
typedef BWL_PRE_PACKED_STRUCT struct wl_proxd_params_tof_tune_v1 {
	uint32		version;
	uint32		Ki;			/**< h/w delay K factor for initiator */
	uint32		Kt;			/**< h/w delay K factor for target */
	int16		vhtack;			/**< enable/disable VHT ACK */
	int16		N_log2[TOF_BW_SEQ_NUM]; /**< simple threshold crossing */
	int16		w_offset[TOF_BW_NUM];	/**< offset of threshold crossing window(per BW) */
	int16		w_len[TOF_BW_NUM];	/**< length of threshold crossing window(per BW) */
	int32		maxDT;			/**< max time difference of T4/T1 or T3/T2 */
	int32		minDT;			/**< min time difference of T4/T1 or T3/T2 */
	uint8		totalfrmcnt;	/**< total count of transfered measurement frames */
	uint16		rsv_media;		/**< reserve media value for TOF */
	uint32		flags;			/**< flags */
	uint8		core;			/**< core to use for tx */
	uint8		setflags;		/* set flags of K, N. S values  */
	int16		N_scale[TOF_BW_SEQ_NUM]; /**< simple threshold crossing */
	uint8		sw_adj;			/**< enable sw assisted timestamp adjustment */
	uint8		hw_adj;			/**< enable hw assisted timestamp adjustment */
	uint8		seq_en;			/**< enable ranging sequence */
	uint8		ftm_cnt[TOF_BW_SEQ_NUM]; /**< number of ftm frames based on bandwidth */
	int16		N_log2_2g;		/**< simple threshold crossing for 2g channel */
	int16		N_scale_2g;		/**< simple threshold crossing for 2g channel */
	wl_proxd_seq_config_t seq_5g20;
	wl_proxd_seq_config_t seq_2g20;		/* Thresh crossing params for 2G Sequence */
	uint16          bitflip_thresh;		/* bitflip threshold */
	uint16          snr_thresh;		/* SNR threshold */
	int8            recv_2g_thresh;		/* 2g recieve sensitivity threshold */
	uint32          acs_gdv_thresh;
	int8            acs_rssi_thresh;
	uint8           smooth_win_en;
	int32		emu_delay;
} BWL_POST_PACKED_STRUCT wl_proxd_params_tof_tune_v1_t;
#include <packed_section_end.h>

#include <packed_section_start.h>
/* For legacy ranging initiator (including 4364) */
typedef BWL_PRE_PACKED_STRUCT struct wl_proxd_params_tof_tune_v2 {
	uint32		version;
	uint32		Ki;			/**< h/w delay K factor for initiator */
	uint32		Kt;			/**< h/w delay K factor for target */
	int16		vhtack;			/**< enable/disable VHT ACK */
	int16		N_log2[TOF_BW_SEQ_NUM]; /**< simple threshold crossing */
	int16		w_offset[TOF_BW_NUM];	/**< offset of threshold crossing window(per BW) */
	int16		w_len[TOF_BW_NUM];	/**< length of threshold crossing window(per BW) */
	int32		maxDT;			/**< max time difference of T4/T1 or T3/T2 */
	int32		minDT;			/**< min time difference of T4/T1 or T3/T2 */
	uint8		totalfrmcnt;		/**< total count of transfered measurement frames */
	uint16		rsv_media;		/**< reserve media value for TOF */
	uint32		flags;			/**< flags */
	uint8		core;			/**< core to use for tx */
	uint8		setflags;		/* set flags of K, N. S values  */
	int16		N_scale[TOF_BW_SEQ_NUM]; /**< simple threshold crossing */
	uint8		sw_adj;			/**< enable sw assisted timestamp adjustment */
	uint8		hw_adj;			/**< enable hw assisted timestamp adjustment */
	uint8		seq_en;			/**< enable ranging sequence */
	uint8		ftm_cnt[TOF_BW_SEQ_NUM]; /**< number of ftm frames based on bandwidth */
	int16		N_log2_2g;		/**< simple threshold crossing for 2g channel */
	int16		N_scale_2g;		/**< simple threshold crossing for 2g channel */
	wl_proxd_seq_config_t seq_5g20;
	wl_proxd_seq_config_t seq_2g20;		/* Thresh crossing params for 2G Sequence */
	uint16          bitflip_thresh;		/* bitflip threshold */
	uint16          snr_thresh;		/* SNR threshold */
	int8            recv_2g_thresh;		/* 2g recieve sensitivity threshold */
	uint32          acs_gdv_thresh;
	int8            acs_rssi_thresh;
	uint8           smooth_win_en;
	int32		acs_gdmm_thresh;
	int8		acs_delta_rssi_thresh;
	int32		emu_delay;
	uint8		core_mask;		/* core mask selection */
} BWL_POST_PACKED_STRUCT wl_proxd_params_tof_tune_v2_t;
#include <packed_section_end.h>

#define WL_PROXD_TUNE_VERSION_3		3
/* Future ranging support */
typedef struct wl_proxd_params_tof_tune_v3 {
	uint16		version;
	uint16		len;
	uint32		Ki;			/**< h/w delay K factor for initiator */
	uint32		Kt;			/**< h/w delay K factor for target */
	int16		vhtack;			/**< enable/disable VHT ACK */
	uint16		PAD;
	int16		N_log2[TOF_BW_SEQ_NUM]; /**< simple threshold crossing */
	uint16		PAD;
	int16		w_offset[TOF_BW_NUM];	/**< offset of threshold crossing window(per BW) */
	uint16		PAD;
	int16		w_len[TOF_BW_NUM];	/**< length of threshold crossing window(per BW) */
	uint16		PAD;
	int32		maxDT;			/**< max time difference of T4/T1 or T3/T2 */
	int32		minDT;			/**< min time difference of T4/T1 or T3/T2 */
	uint8		totalfrmcnt;	/**< total count of transfered measurement frames */
	uint8		PAD[3];
	uint16		rsv_media;		/**< reserve media value for TOF */
	uint16		PAD;
	uint32		flags;			/**< flags */
	uint8		core;			/**< core to use for tx */
	uint8		setflags;		/* set flags of K, N. S values  */
	uint16		PAD;
	int16		N_scale[TOF_BW_SEQ_NUM]; /**< simple threshold crossing */
	uint8		sw_adj;			/**< enable sw assisted timestamp adjustment */
	uint8		hw_adj;			/**< enable hw assisted timestamp adjustment */
	uint8		seq_en;			/**< enable ranging sequence */
	uint8		PAD[3];
	uint8		ftm_cnt[TOF_BW_SEQ_NUM]; /**< number of ftm frames based on bandwidth */
	uint8		PAD[3];
	int16		N_log2_2g;		/**< simple threshold crossing for 2g channel */
	int16		N_scale_2g;		/**< simple threshold crossing for 2g channel */
	wl_proxd_seq_config_t seq_5g20;
	wl_proxd_seq_config_t seq_2g20;		/* Thresh crossing params for 2G Sequence */
	uint16          bitflip_thresh;		/* bitflip threshold */
	uint16          snr_thresh;		/* SNR threshold */
	int8            recv_2g_thresh;		/* 2g recieve sensitivity threshold */
	uint8		PAD[3];
	uint32          acs_gdv_thresh;
	int8            acs_rssi_thresh;
	uint8           smooth_win_en;
	uint16		PAD;
	int32		acs_gdmm_thresh;
	int8		acs_delta_rssi_thresh;
	uint8		PAD[3];
	int32		emu_delay;
	uint8		core_mask;		/* core mask selection */
	uint8		PAD[3];
} wl_proxd_params_tof_tune_v3_t;

typedef struct wl_proxd_params_iovar {
	uint16	method;			/**< Proximity Detection method */
	uint8	PAD[2];
	union {
		/* common params for pdsvc */
		wl_proxd_params_common_t	cmn_params;	/**< common parameters */
		/*  method specific */
		wl_proxd_params_rssi_method_t	rssi_params;	/**< RSSI method parameters */
		wl_proxd_params_tof_method_t	tof_params;	/**< TOF method parameters */
		/* tune parameters */
		wl_proxd_params_tof_tune_v3_t	tof_tune;	/**< TOF tune parameters */
	} u;				/**< Method specific optional parameters */
} wl_proxd_params_iovar_t;

#define PROXD_COLLECT_GET_STATUS	0
#define PROXD_COLLECT_SET_STATUS	1
#define PROXD_COLLECT_QUERY_HEADER	2
#define PROXD_COLLECT_QUERY_DATA	3
#define PROXD_COLLECT_QUERY_DEBUG	4
#define PROXD_COLLECT_REMOTE_REQUEST	5
#define PROXD_COLLECT_DONE		6

typedef enum {
	WL_PROXD_COLLECT_METHOD_TYPE_DISABLE		= 0x0,
	WL_PROXD_COLLECT_METHOD_TYPE_IOVAR		= 0x1,
	WL_PROXD_COLLECT_METHOD_TYPE_EVENT		= 0x2,
	WL_PROXD_COLLECT_METHOD_TYPE_EVENT_LOG		= 0x4
} wl_proxd_collect_method_type_t;

typedef uint16 wl_proxd_collect_method_t;	/* query status: method to send proxd collect */

#include <packed_section_start.h>
typedef BWL_PRE_PACKED_STRUCT struct wl_proxd_collect_query {
	uint32		method;		/**< method */
	uint8		request;	/**< Query request. */
	uint8		status;		/**< bitmask 0 -- disable, 0x1 -- enable collection, */
					/* 0x2 -- Use generic event, 0x4 -- use event log */
	uint16		index;		/**< The current frame index [0 to total_frames - 1]. */
	uint16		mode;		/**< Initiator or Target */
	uint8		busy;		/**< tof sm is busy */
	uint8		remote;		/**< Remote collect data */
} BWL_POST_PACKED_STRUCT wl_proxd_collect_query_t;
#include <packed_section_end.h>

#include <packed_section_start.h>
typedef BWL_PRE_PACKED_STRUCT struct wl_proxd_collect_header {
	uint16		total_frames;			/**< The total frames for this collect. */
	uint16		nfft;				/**< nfft value */
	uint16		bandwidth;			/**< bandwidth */
	uint16		channel;			/**< channel number */
	uint32		chanspec;			/**< channel spec */
	uint32		fpfactor;			/**< avb timer value factor */
	uint16		fpfactor_shift;			/**< avb timer value shift bits */
	int32		distance;			/**< distance calculated by fw */
	uint32		meanrtt;			/**< mean of RTTs */
	uint32		modertt;			/**< mode of RTTs */
	uint32		medianrtt;			/**< median of RTTs */
	uint32		sdrtt;				/**< standard deviation of RTTs */
	uint32		clkdivisor;			/**< clock divisor */
	uint16		chipnum;			/**< chip type */
	uint8		chiprev;			/**< chip revision */
	uint8		phyver;				/**< phy version */
	struct ether_addr	localMacAddr;		/**< local mac address */
	struct ether_addr	remoteMacAddr;		/**< remote mac address */
	wl_proxd_params_tof_tune_v3_t params;
} BWL_POST_PACKED_STRUCT wl_proxd_collect_header_t;
#include <packed_section_end.h>

/* ifdef WL_NAN */
/*  ********************** NAN wl interface struct types and defs ******************** */
/*
 * Uses new common IOVAR batch processing mechanism
 */

/*
 * NAN config control
 * Bits 0 - 23 can be set by host
 * Bits 24 - 31 - Internal use for firmware, host cannot set it
 */

/*
 * Bit 0 : If set to 1, means event uses nan bsscfg,
 * otherwise uses infra bsscfg. Default is using infra bsscfg
 */
#define WL_NAN_CTRL_ROUTE_EVENT_VIA_NAN_BSSCFG	0x1
/* If set, discovery beacons are transmitted on 2G band */
#define WL_NAN_CTRL_DISC_BEACON_TX_2G		0x2
/* If set, sync beacons are transmitted on 2G band */
#define WL_NAN_CTRL_SYNC_BEACON_TX_2G		0x4
/* If set, discovery beacons are transmitted on 5G band */
#define WL_NAN_CTRL_DISC_BEACON_TX_5G		0x8
/* If set, sync beacons are transmitted on 5G band */
#define WL_NAN_CTRL_SYNC_BEACON_TX_5G		0x10
/* If set, auto datapath responses will be sent by FW */
#define WL_NAN_CTRL_AUTO_DPRESP			0x20
/* If set, auto datapath confirms will be sent by FW */
#define WL_NAN_CTRL_AUTO_DPCONF			0x40
/* If set, auto schedule responses will be sent by FW */
#define WL_NAN_CTRL_AUTO_SCHEDRESP		0x80
/* If set, auto schedule confirms will be sent by FW */
#define WL_NAN_CTRL_AUTO_SCHEDCONF		0x100
/* If set, proprietary rates are supported by FW */
#define WL_NAN_CTRL_PROP_RATE			0x200

/* Value when all host-configurable bits set */
#define WL_NAN_CTRL_MAX_MASK			0xFFFFFF
#define WL_NAN_CFG_CTRL_FW_BITS			8

/* Bit 31:
 * If set - indicates that NAN initialization is successful
 * NOTE: This is a ready-only bit. All sets to this are masked off
 */
#define WL_NAN_PROTO_INIT_DONE		0x80000000
#define WL_NAN_GET_PROTO_INIT_STATUS(x) \
		(((x) >> 31) & 1)
#define WL_NAN_CLEAR_PROTO_INIT_STATUS(x) \
		((x) &= ~WL_NAN_PROTO_INIT_DONE)
#define WL_NAN_SET_PROTO_INIT_STATUS(x) \
		((x) |= (1 << 31))

#define WL_NAN_IOCTL_VERSION			0x2
/* < some sufficient ioc buff size for our module */
#define WL_NAN_IOC_BUFSZ			256
/* some sufficient ioc buff size for dump commands */
#define WL_NAN_IOC_BUFSZ_EXT			1024
#define WL_NAN_MAX_SIDS_IN_BEACONS		127 /* Max allowed SIDs */
#define WL_NAN_MASTER_RANK_LEN			8
#define WL_NAN_RANGE_LIMITED			0x0040 /* Publish/Subscribe flags */

/** The service hash (service id) is exactly this many bytes. */
#define WL_NAN_SVC_HASH_LEN			6
#ifndef LINUX_POSTMOGRIFY_REMOVAL
#define WL_NAN_HASHES_PER_BLOOM			4 /** Number of hash functions per bloom filter */
#endif /* LINUX_POSTMOGRIFY_REMOVAL */

/* no. of max last disc results */
#define WL_NAN_MAX_DISC_RESULTS			3

/* Max len of Rx and Tx filters */
#define WL_NAN_MAX_SVC_MATCH_FILTER_LEN	255

/* Max service name len */
#define WL_NAN_MAX_SVC_NAME_LEN	32

/* Type of Data path connection */
#define WL_NAN_DP_TYPE_UNICAST			0
#define WL_NAN_DP_TYPE_MULTICAST		1

/* MAX security params length PMK field */
#define WL_NAN_NCS_SK_PMK_LEN 32

/* Post disc attr ID type */
typedef uint8 wl_nan_post_disc_attr_id_t;

/*
 * Component IDs
 */
typedef enum {
	WL_NAN_COMPID_CONFIG = 1,
	WL_NAN_COMPID_ELECTION = 2,
	WL_NAN_COMPID_SD = 3,
	WL_NAN_COMPID_TIMESYNC = 4,
	WL_NAN_COMPID_DATA_PATH = 5,
	WL_NAN_COMPID_DEBUG = 15 /* Keep this at the end */
} wl_nan_comp_id_t;

#define WL_NAN_COMP_SHIFT	8
#define WL_NAN_COMP_MASK(_c)	(0x0F & ((uint8)(_c)))
#define WL_NAN_COMP_ID(_c)	(WL_NAN_COMP_MASK(_c) << WL_NAN_COMP_SHIFT)

/* NAN Events */

/** Instance ID type (unique identifier) */
typedef uint8 wl_nan_instance_id_t;

/* Publish sent for a subscribe */
/* WL_NAN_EVENT_REPLIED */

typedef struct wl_nan_ev_replied {
	struct ether_addr	sub_mac; /* Subscriber MAC */
	wl_nan_instance_id_t	pub_id; /* Publisher Instance ID */
	uint8			sub_id; /* Subscriber ID */
	int8			sub_rssi; /* Subscriber RSSI */
	uint8			pad[3];
} wl_nan_ev_replied_t;

typedef struct wl_nan_event_replied {
	struct ether_addr	sub_mac; /* Subscriber MAC */
	wl_nan_instance_id_t	pub_id; /* Publisher Instance ID */
	uint8			sub_id; /* Subscriber ID */
	int8			sub_rssi; /* Subscriber RSSI */
	uint8		attr_num;
	uint16		attr_list_len;  /* sizeof attributes attached to payload */
	uint8		attr_list[0];   /* attributes payload */
} wl_nan_event_replied_t;

/* NAN Tx status of transmitted frames */
#define WL_NAN_TXS_FAILURE	0
#define WL_NAN_TXS_SUCCESS	1

/* NAN frame types */
enum wl_nan_frame_type {
	/* discovery frame types */
	WL_NAN_FRM_TYPE_PUBLISH = 1,
	WL_NAN_FRM_TYPE_SUBSCRIBE = 2,
	WL_NAN_FRM_TYPE_FOLLOWUP = 3,

	/* datapath frame types */
	WL_NAN_FRM_TYPE_DP_REQ = 4,
	WL_NAN_FRM_TYPE_DP_RESP = 5,
	WL_NAN_FRM_TYPE_DP_CONF = 6,
	WL_NAN_FRM_TYPE_DP_INSTALL = 7,
	WL_NAN_FRM_TYPE_DP_END = 8,

	/* schedule frame types */
	WL_NAN_FRM_TYPE_SCHED_REQ = 9,
	WL_NAN_FRM_TYPE_SCHED_RESP = 10,
	WL_NAN_FRM_TYPE_SCHED_CONF = 11,
	WL_NAN_FRM_TYPE_SCHED_UPD = 12,

	/* ranging frame types */
	WL_NAN_FRM_TYPE_RNG_REQ = 13,
	WL_NAN_FRM_TYPE_RNG_RESP = 14,
	WL_NAN_FRM_TYPE_RNG_TERM = 15,
	WL_NAN_FRM_TYPE_RNG_REPORT = 16,
	WL_NAN_FRM_TYPE_INVALID
};

/* NAN Reason codes for tx status */
enum wl_nan_txs_reason_codes {
	WL_NAN_REASON_SUCCESS = 1,    /* NAN status success */
	WL_NAN_REASON_TIME_OUT = 2,   /* timeout reached */
	WL_NAN_REASON_DROPPED = 3,    /* pkt dropped due to internal failure */
	WL_NAN_REASON_MAX_RETRIES_DONE = 4 /* Max retries exceeded */
};

/* For NAN TX status */
typedef struct wl_nan_event_txs {
	uint8 status;	    /* For TX status, success or failure */
	uint8 reason_code;  /* to identify reason when status is failure */
	uint16 host_seq;    /* seq num to keep track of pkts sent by host */
	uint8 type;	    /* frame type */
	uint8 pad;
	uint16 opt_tlvs_len;
	uint8 opt_tlvs[];
} wl_nan_event_txs_t;

/* SD transmit pkt's event status is sent as optional tlv in wl_nan_event_txs_t */
typedef struct wl_nan_event_sd_txs {
	uint8 inst_id;	    /* Publish or subscribe instance id */
	uint8 req_id;	    /* Requestor instance id */
} wl_nan_event_sd_txs_t;

/* Subscribe or Publish instance Terminated */

/* WL_NAN_EVENT_TERMINATED */

#define	NAN_SD_TERM_REASON_TIMEOUT	1
#define	NAN_SD_TERM_REASON_HOSTREQ	2
#define	NAN_SD_TERM_REASON_FWTERM	3
#define	NAN_SD_TERM_REASON_FAIL		4

typedef struct wl_nan_ev_terminated {
	uint8 instance_id;	/* publish / subscribe instance id */
	uint8 reason;		/* 1=timeout, 2=Host/IOVAR, 3=FW Terminated 4=Failure */
	uint8 svctype;		/* 0 - Publish, 0x1 - Subscribe */
	uint8 pad;		/* Align */
} wl_nan_ev_terminated_t;

/* Follow up received against a pub / subscr */
/* WL_NAN_EVENT_RECEIVE */

typedef struct wl_nan_ev_receive {
	struct ether_addr remote_addr;	/* Peer NAN device MAC */
	uint8	local_id;		/* Local subscribe or publish ID */
	uint8	remote_id;		/* Remote subscribe or publish ID */
	int8	fup_rssi;
	uint8	attr_num;
	uint16	attr_list_len;  /* sizeof attributes attached to payload */
	uint8	attr_list[0];   /* attributes payload */
} wl_nan_ev_receive_t;

#define NAN_MAX_BANDS 2

/*
 * TLVs - Below XTLV definitions will be deprecated
 * in due course (soon as all other branches update
 * to the comp ID based XTLVs listed below).
 */
enum wl_nan_cmd_xtlv_id {
	WL_NAN_XTLV_MAC_ADDR = 0x120,
	WL_NAN_XTLV_MATCH_RX = 0x121,
	WL_NAN_XTLV_MATCH_TX = 0x122,
	WL_NAN_XTLV_SVC_INFO = 0x123,
	WL_NAN_XTLV_SVC_NAME = 0x124,
	WL_NAN_XTLV_SR_FILTER = 0x125,
	WL_NAN_XTLV_FOLLOWUP = 0x126,
	WL_NAN_XTLV_SVC_LIFE_COUNT = 0x127,
	WL_NAN_XTLV_AVAIL = 0x128,
	WL_NAN_XTLV_SDF_RX = 0x129,
	WL_NAN_XTLV_SDE_CONTROL = 0x12a,
	WL_NAN_XTLV_SDE_RANGE_LIMIT = 0x12b,
	WL_NAN_XTLV_NAN_AF = 0x12c,
	WL_NAN_XTLV_SD_TERMINATE = 0x12d,
	WL_NAN_XTLV_CLUSTER_ID = 0x12e,
	WL_NAN_XTLV_PEER_RSSI = 0x12f,
	WL_NAN_XTLV_BCN_RX = 0x130,
	WL_NAN_XTLV_REPLIED = 0x131, /* Publish sent for a subscribe */
	WL_NAN_XTLV_RECEIVED = 0x132, /* FUP Received */
	WL_NAN_XTLV_DISC_RESULTS = 0x133, /* Discovery results */
	WL_NAN_XTLV_TXS = 0x134 /* TX status */
};

#define WL_NAN_CMD_GLOBAL		0x00
#define WL_NAN_CMD_CFG_COMP_ID		0x01
#define WL_NAN_CMD_ELECTION_COMP_ID	0x02
#define WL_NAN_CMD_SD_COMP_ID		0x03
#define WL_NAN_CMD_SYNC_COMP_ID		0x04
#define WL_NAN_CMD_DATA_COMP_ID		0x05
#define WL_NAN_CMD_DAM_COMP_ID		0x06
#define WL_NAN_CMD_RANGE_COMP_ID	0x07
#define WL_NAN_CMD_GENERIC_COMP_ID	0x08
#define WL_NAN_CMD_DBG_COMP_ID		0x0f

#define WL_NAN_CMD_COMP_SHIFT		8
#define NAN_CMD(x, y)  (((x) << WL_NAN_CMD_COMP_SHIFT) | (y))

/*
 * Module based NAN TLV IDs
 */
typedef enum wl_nan_tlv {

	WL_NAN_XTLV_CFG_MATCH_RX	= NAN_CMD(WL_NAN_CMD_CFG_COMP_ID, 0x01),
	WL_NAN_XTLV_CFG_MATCH_TX	= NAN_CMD(WL_NAN_CMD_CFG_COMP_ID, 0x02),
	WL_NAN_XTLV_CFG_SR_FILTER	= NAN_CMD(WL_NAN_CMD_CFG_COMP_ID, 0x03),
	WL_NAN_XTLV_CFG_SVC_NAME	= NAN_CMD(WL_NAN_CMD_CFG_COMP_ID, 0x04),
	WL_NAN_XTLV_CFG_NAN_STATUS	= NAN_CMD(WL_NAN_CMD_CFG_COMP_ID, 0x05),
	WL_NAN_XTLV_CFG_SVC_LIFE_COUNT	= NAN_CMD(WL_NAN_CMD_CFG_COMP_ID, 0x06),
	WL_NAN_XTLV_CFG_SVC_HASH	= NAN_CMD(WL_NAN_CMD_CFG_COMP_ID, 0x07),
	WL_NAN_XTLV_CFG_SEC_CSID	= NAN_CMD(WL_NAN_CMD_CFG_COMP_ID, 0x08), /* Security CSID */
	WL_NAN_XTLV_CFG_SEC_PMK		= NAN_CMD(WL_NAN_CMD_CFG_COMP_ID, 0x09), /* Security PMK */
	WL_NAN_XTLV_CFG_SEC_PMKID	= NAN_CMD(WL_NAN_CMD_CFG_COMP_ID, 0x0A),
	WL_NAN_XTLV_CFG_SEC_SCID	= NAN_CMD(WL_NAN_CMD_CFG_COMP_ID, 0x0B),

	WL_NAN_XTLV_SD_SVC_INFO		= NAN_CMD(WL_NAN_CMD_SD_COMP_ID, 0x01),
	WL_NAN_XTLV_SD_FOLLOWUP		= NAN_CMD(WL_NAN_CMD_SD_COMP_ID, 0x02),
	WL_NAN_XTLV_SD_SDF_RX		= NAN_CMD(WL_NAN_CMD_SD_COMP_ID, 0x03),
	WL_NAN_XTLV_SD_SDE_CONTROL	= NAN_CMD(WL_NAN_CMD_SD_COMP_ID, 0x04),
	WL_NAN_XTLV_SD_SDE_RANGE_LIMIT	= NAN_CMD(WL_NAN_CMD_SD_COMP_ID, 0x05),
	WL_NAN_XTLV_SD_NAN_AF		= NAN_CMD(WL_NAN_CMD_SD_COMP_ID, 0x06),
	WL_NAN_XTLV_SD_TERM		= NAN_CMD(WL_NAN_CMD_SD_COMP_ID, 0x07),
	WL_NAN_XTLV_SD_REPLIED		= NAN_CMD(WL_NAN_CMD_SD_COMP_ID, 0x08), /* Pub sent */
	WL_NAN_XTLV_SD_FUP_RECEIVED	= NAN_CMD(WL_NAN_CMD_SD_COMP_ID, 0x09), /* FUP Received */
	WL_NAN_XTLV_SD_DISC_RESULTS	= NAN_CMD(WL_NAN_CMD_SD_COMP_ID, 0x0A), /* Pub RX */
	WL_NAN_XTLV_SD_TXS		= NAN_CMD(WL_NAN_CMD_SD_COMP_ID, 0x0B), /* Tx status */

	WL_NAN_XTLV_SYNC_BCN_RX		= NAN_CMD(WL_NAN_CMD_SYNC_COMP_ID, 0x01),

	WL_NAN_XTLV_DATA_DP_END		= NAN_CMD(WL_NAN_CMD_DATA_COMP_ID, 0x01),
	WL_NAN_XTLV_DATA_DP_INFO	= NAN_CMD(WL_NAN_CMD_DATA_COMP_ID, 0x02),
	WL_NAN_XTLV_DATA_DP_SEC_INST	= NAN_CMD(WL_NAN_CMD_DATA_COMP_ID, 0x03),

	WL_NAN_XTLV_RANGE_INFO		= NAN_CMD(WL_NAN_CMD_RANGE_COMP_ID, 0x01)
} wl_nan_tlv_t;

/* Sub Module ID's for NAN */
enum {
	NAN_MAC = 		0,	/* nan mac */
	NAN_DISC = 		1,	/* nan discovery */
	NAN_DBG = 		2,	/* nan debug */
	NAN_SCHED = 		3,	/* nan sched */
	NAN_AVAIL = 		4,	/* nan avail */
	NAN_PEER_ENTRY = 	5,	/* nan dp peer entry */
	NAN_DAM = 		6,	/* nan dam */
	NAN_NDP = 		7,	/* nan ndp */
	NAN_NDL = 		8,	/* nan ndl */
	NAN_DP = 		9,	/* nan dp core */
	NAN_FSM = 		10,	/* nan fsm registry */
	NAN_RNG = 		11,	/* nan ranging */
	NAN_SEC = 		12,	/* nan sec */
	NAN_LAST = 		13
};

enum wl_nan_sub_cmd_xtlv_id {

	/* Special command - Tag zero */
	WL_NAN_CMD_GLB_NAN_VER = NAN_CMD(WL_NAN_CMD_GLOBAL, 0x00),

	 /* nan cfg sub-commands */

	WL_NAN_CMD_CFG_NAN_INIT = NAN_CMD(WL_NAN_CMD_CFG_COMP_ID, 0x01),
	WL_NAN_CMD_CFG_ROLE = NAN_CMD(WL_NAN_CMD_CFG_COMP_ID, 0x02),
	WL_NAN_CMD_CFG_HOP_CNT = NAN_CMD(WL_NAN_CMD_CFG_COMP_ID, 0x03),
	WL_NAN_CMD_CFG_HOP_LIMIT = NAN_CMD(WL_NAN_CMD_CFG_COMP_ID, 0x04),
	WL_NAN_CMD_CFG_WARMUP_TIME = NAN_CMD(WL_NAN_CMD_CFG_COMP_ID, 0x05),
	WL_NAN_CMD_CFG_STATUS = NAN_CMD(WL_NAN_CMD_CFG_COMP_ID, 0x06),
	WL_NAN_CMD_CFG_OUI = NAN_CMD(WL_NAN_CMD_CFG_COMP_ID, 0x07),
	WL_NAN_CMD_CFG_COUNT = NAN_CMD(WL_NAN_CMD_CFG_COMP_ID, 0x08),
	WL_NAN_CMD_CFG_CLEARCOUNT = NAN_CMD(WL_NAN_CMD_CFG_COMP_ID, 0x09),
	WL_NAN_CMD_CFG_CHANNEL = NAN_CMD(WL_NAN_CMD_CFG_COMP_ID, 0x0A),
	WL_NAN_CMD_CFG_BAND = NAN_CMD(WL_NAN_CMD_CFG_COMP_ID, 0x0B),
	WL_NAN_CMD_CFG_CID = NAN_CMD(WL_NAN_CMD_CFG_COMP_ID, 0x0C),
	WL_NAN_CMD_CFG_IF_ADDR = NAN_CMD(WL_NAN_CMD_CFG_COMP_ID, 0x0D),
	WL_NAN_CMD_CFG_BCN_INTERVAL = NAN_CMD(WL_NAN_CMD_CFG_COMP_ID, 0x0E),
	WL_NAN_CMD_CFG_SDF_TXTIME = NAN_CMD(WL_NAN_CMD_CFG_COMP_ID, 0x0F),
	WL_NAN_CMD_CFG_SID_BEACON = NAN_CMD(WL_NAN_CMD_CFG_COMP_ID, 0x10),
	WL_NAN_CMD_CFG_DW_LEN = NAN_CMD(WL_NAN_CMD_CFG_COMP_ID, 0x11),
	WL_NAN_CMD_CFG_AVAIL = NAN_CMD(WL_NAN_CMD_CFG_COMP_ID, 0x12),
	WL_NAN_CMD_CFG_WFA_TM = NAN_CMD(WL_NAN_CMD_CFG_COMP_ID, 0x13),
	WL_NAN_CMD_CFG_EVENT_MASK =  NAN_CMD(WL_NAN_CMD_CFG_COMP_ID, 0x14),
	WL_NAN_CMD_CFG_NAN_CONFIG = NAN_CMD(WL_NAN_CMD_CFG_COMP_ID, 0x15),
	WL_NAN_CMD_CFG_NAN_ENAB = NAN_CMD(WL_NAN_CMD_CFG_COMP_ID, 0x16),
	WL_NAN_CMD_CFG_ULW = NAN_CMD(WL_NAN_CMD_CFG_COMP_ID, 0x17),
	WL_NAN_CMD_CFG_MAX = WL_NAN_CMD_CFG_NAN_ENAB,
	/* Add new commands before and update */

	/* nan election sub-commands */
	WL_NAN_CMD_ELECTION_HOST_ENABLE = NAN_CMD(WL_NAN_CMD_ELECTION_COMP_ID, 0x01),
	WL_NAN_CMD_ELECTION_METRICS_CONFIG = NAN_CMD(WL_NAN_CMD_ELECTION_COMP_ID, 0x02),
	WL_NAN_CMD_ELECTION_METRICS_STATE = NAN_CMD(WL_NAN_CMD_ELECTION_COMP_ID, 0x03),
	WL_NAN_CMD_ELECTION_LEAVE = NAN_CMD(WL_NAN_CMD_ELECTION_COMP_ID, 0x03),
	WL_NAN_CMD_ELECTION_MERGE = NAN_CMD(WL_NAN_CMD_ELECTION_COMP_ID, 0x04),
	WL_NAN_CMD_ELECTION_ADVERTISERS = NAN_CMD(WL_NAN_CMD_ELECTION_COMP_ID, 0x05),
	WL_NAN_CMD_ELECTION_RSSI_THRESHOLD = NAN_CMD(WL_NAN_CMD_ELECTION_COMP_ID, 0x06),
	WL_NAN_CMD_ELECTION_MAX = WL_NAN_CMD_ELECTION_RSSI_THRESHOLD,
	/* New commands go before and update */

	/* nan SD sub-commands */
	WL_NAN_CMD_SD_PARAMS = NAN_CMD(WL_NAN_CMD_SD_COMP_ID, 0x01),
	WL_NAN_CMD_SD_PUBLISH = NAN_CMD(WL_NAN_CMD_SD_COMP_ID, 0x02),
	WL_NAN_CMD_SD_PUBLISH_LIST = NAN_CMD(WL_NAN_CMD_SD_COMP_ID, 0x03),
	WL_NAN_CMD_SD_CANCEL_PUBLISH = NAN_CMD(WL_NAN_CMD_SD_COMP_ID, 0x04),
	WL_NAN_CMD_SD_SUBSCRIBE = NAN_CMD(WL_NAN_CMD_SD_COMP_ID, 0x05),
	WL_NAN_CMD_SD_SUBSCRIBE_LIST = NAN_CMD(WL_NAN_CMD_SD_COMP_ID, 0x06),
	WL_NAN_CMD_SD_CANCEL_SUBSCRIBE = NAN_CMD(WL_NAN_CMD_SD_COMP_ID, 0x07),
	WL_NAN_CMD_SD_VND_INFO = NAN_CMD(WL_NAN_CMD_SD_COMP_ID, 0x08),
	WL_NAN_CMD_SD_STATS = NAN_CMD(WL_NAN_CMD_SD_COMP_ID, 0x09),
	WL_NAN_CMD_SD_TRANSMIT = NAN_CMD(WL_NAN_CMD_SD_COMP_ID, 0x0A),
	WL_NAN_CMD_SD_FUP_TRANSMIT = NAN_CMD(WL_NAN_CMD_SD_COMP_ID, 0x0B),
	WL_NAN_CMD_SD_CONNECTION = NAN_CMD(WL_NAN_CMD_SD_COMP_ID, 0x0C),
	WL_NAN_CMD_SD_SHOW = NAN_CMD(WL_NAN_CMD_SD_COMP_ID, 0x0D),
	WL_NAN_CMD_SD_MAX = WL_NAN_CMD_SD_SHOW,

	/* nan time sync sub-commands */

	WL_NAN_CMD_SYNC_SOCIAL_CHAN = NAN_CMD(WL_NAN_CMD_SYNC_COMP_ID, 0x01),
	WL_NAN_CMD_SYNC_AWAKE_DWS = NAN_CMD(WL_NAN_CMD_SYNC_COMP_ID, 0x02),
	WL_NAN_CMD_SYNC_BCN_RSSI_NOTIF_THRESHOLD = NAN_CMD(WL_NAN_CMD_SYNC_COMP_ID, 0x03),
	WL_NAN_CMD_SYNC_MAX = WL_NAN_CMD_SYNC_BCN_RSSI_NOTIF_THRESHOLD,

	/* nan2 commands */
	WL_NAN_CMD_DATA_CONFIG = NAN_CMD(WL_NAN_CMD_DATA_COMP_ID, 0x01),
	WL_NAN_CMD_DATA_RSVD02 = NAN_CMD(WL_NAN_CMD_DATA_COMP_ID, 0x02),
	WL_NAN_CMD_DATA_RSVD03 = NAN_CMD(WL_NAN_CMD_DATA_COMP_ID, 0x03),
	WL_NAN_CMD_DATA_DATAREQ = NAN_CMD(WL_NAN_CMD_DATA_COMP_ID, 0x04),
	WL_NAN_CMD_DATA_DATARESP = NAN_CMD(WL_NAN_CMD_DATA_COMP_ID, 0x05),
	WL_NAN_CMD_DATA_DATAEND = NAN_CMD(WL_NAN_CMD_DATA_COMP_ID, 0x06),
	WL_NAN_CMD_DATA_SCHEDUPD = NAN_CMD(WL_NAN_CMD_DATA_COMP_ID, 0x07),
	WL_NAN_CMD_DATA_RSVD08 = NAN_CMD(WL_NAN_CMD_DATA_COMP_ID, 0x08),
	WL_NAN_CMD_DATA_CAP = NAN_CMD(WL_NAN_CMD_DATA_COMP_ID, 0x9),
	WL_NAN_CMD_DATA_STATUS = NAN_CMD(WL_NAN_CMD_DATA_COMP_ID, 0x0A),
	WL_NAN_CMD_DATA_STATS = NAN_CMD(WL_NAN_CMD_DATA_COMP_ID, 0x0B),
	WL_NAN_CMD_DATA_RSVD0C = NAN_CMD(WL_NAN_CMD_DATA_COMP_ID, 0x0C),
	WL_NAN_CMD_DATA_NDP_SHOW = NAN_CMD(WL_NAN_CMD_DATA_COMP_ID, 0x0D),
	WL_NAN_CMD_DATA_DATACONF = NAN_CMD(WL_NAN_CMD_DATA_COMP_ID, 0x0E),
	WL_NAN_CMD_DATA_MIN_TX_RATE = NAN_CMD(WL_NAN_CMD_DATA_COMP_ID, 0x0F),
	WL_NAN_CMD_DATA_MAX_PEERS = NAN_CMD(WL_NAN_CMD_DATA_COMP_ID, 0x10),
	WL_NAN_CMD_DATA_PATH_MAX = WL_NAN_CMD_DATA_MAX_PEERS, /* New ones before and update */

	/* nan dam sub-commands */
	WL_NAN_CMD_DAM_CFG = NAN_CMD(WL_NAN_CMD_DAM_COMP_ID, 0x01),
	WL_NAN_CMD_DAM_MAX = WL_NAN_CMD_DAM_CFG,  /* New ones before and update */

	/* nan2.0 ranging commands */
	WL_NAN_CMD_RANGE_REQUEST = NAN_CMD(WL_NAN_CMD_RANGE_COMP_ID, 0x01),
	WL_NAN_CMD_RANGE_AUTO = NAN_CMD(WL_NAN_CMD_RANGE_COMP_ID, 0x02),
	WL_NAN_CMD_RANGE_RESPONSE = NAN_CMD(WL_NAN_CMD_RANGE_COMP_ID, 0x03),
	WL_NAN_CMD_RANGE_CANCEL = NAN_CMD(WL_NAN_CMD_RANGE_COMP_ID, 0x04),

	/*  nan debug sub-commands  */
	WL_NAN_CMD_DBG_SCAN_PARAMS = NAN_CMD(WL_NAN_CMD_DBG_COMP_ID, 0x01),
	WL_NAN_CMD_DBG_SCAN = NAN_CMD(WL_NAN_CMD_DBG_COMP_ID, 0x02),
	WL_NAN_CMD_DBG_SCAN_RESULTS =  NAN_CMD(WL_NAN_CMD_DBG_COMP_ID, 0x03),
	/* This is now moved under CFG */
	WL_NAN_CMD_DBG_EVENT_MASK =  NAN_CMD(WL_NAN_CMD_DBG_COMP_ID, 0x04),
	WL_NAN_CMD_DBG_EVENT_CHECK =  NAN_CMD(WL_NAN_CMD_DBG_COMP_ID, 0x05),
	WL_NAN_CMD_DBG_DUMP =  NAN_CMD(WL_NAN_CMD_DBG_COMP_ID, 0x06),
	WL_NAN_CMD_DBG_CLEAR =  NAN_CMD(WL_NAN_CMD_DBG_COMP_ID, 0x07),
	WL_NAN_CMD_DBG_RSSI =  NAN_CMD(WL_NAN_CMD_DBG_COMP_ID, 0x08),
	WL_NAN_CMD_DBG_DEBUG =  NAN_CMD(WL_NAN_CMD_DBG_COMP_ID, 0x09),
	WL_NAN_CMD_DBG_TEST1 =  NAN_CMD(WL_NAN_CMD_DBG_COMP_ID, 0x0A),
	WL_NAN_CMD_DBG_TEST2 =  NAN_CMD(WL_NAN_CMD_DBG_COMP_ID, 0x0B),
	WL_NAN_CMD_DBG_TEST3 =  NAN_CMD(WL_NAN_CMD_DBG_COMP_ID, 0x0C),
	WL_NAN_CMD_DBG_DISC_RESULTS =  NAN_CMD(WL_NAN_CMD_DBG_COMP_ID, 0x0D),
	WL_NAN_CMD_DBG_STATS =  NAN_CMD(WL_NAN_CMD_DBG_COMP_ID, 0x0E),
	WL_NAN_CMD_DBG_LEVEL =  NAN_CMD(WL_NAN_CMD_DBG_COMP_ID, 0x0F),
	WL_NAN_CMD_DBG_MAX = WL_NAN_CMD_DBG_LEVEL, /* New ones before and update */

	 /* Generic componenet */
	WL_NAN_CMD_GEN_STATS = NAN_CMD(WL_NAN_CMD_GENERIC_COMP_ID, 0x01),
	WL_NAN_CMD_GEN_MAX = WL_NAN_CMD_GEN_STATS /* New ones before and update */
};

/*
 * Component/Module based NAN TLV IDs for NAN stats
 */
typedef enum wl_nan_stats_tlv {
	WL_NAN_XTLV_SYNC_MAC_STATS		= NAN_CMD(WL_NAN_CMD_SYNC_COMP_ID, 0x01),

	WL_NAN_XTLV_SD_DISC_STATS		= NAN_CMD(WL_NAN_CMD_SD_COMP_ID, 0x01),

	WL_NAN_XTLV_DATA_NDP_STATS		= NAN_CMD(WL_NAN_CMD_DATA_COMP_ID, 0x01),
	WL_NAN_XTLV_DATA_NDL_STATS		= NAN_CMD(WL_NAN_CMD_DATA_COMP_ID, 0x02),
	WL_NAN_XTLV_DATA_SEC_STATS		= NAN_CMD(WL_NAN_CMD_DATA_COMP_ID, 0x03),

	WL_NAN_XTLV_GEN_SCHED_STATS		= NAN_CMD(WL_NAN_CMD_GENERIC_COMP_ID, 0x01),
	WL_NAN_XTLV_GEN_PEER_STATS		= NAN_CMD(WL_NAN_CMD_GENERIC_COMP_ID, 0x02),
	WL_NAN_XTLV_GEN_PEER_STATS_DEVCAP	= NAN_CMD(WL_NAN_CMD_GENERIC_COMP_ID, 0x03),
	WL_NAN_XTLV_GEN_PEER_STATS_NDP		= NAN_CMD(WL_NAN_CMD_GENERIC_COMP_ID, 0x04),
	WL_NAN_XTLV_GEN_PEER_STATS_SCHED	= NAN_CMD(WL_NAN_CMD_GENERIC_COMP_ID, 0x05),

	WL_NAN_XTLV_DAM_STATS			= NAN_CMD(WL_NAN_CMD_DAM_COMP_ID, 0x01),
	WL_NAN_XTLV_DAM_AVAIL_STATS		= NAN_CMD(WL_NAN_CMD_DAM_COMP_ID, 0x02),

	WL_NAN_XTLV_RANGE_STATS			= NAN_CMD(WL_NAN_CMD_RANGE_COMP_ID, 0x01)
} wl_nan_stats_tlv_t;

/* NAN stats WL_NAN_CMD_GEN_STATS command */
/* Input data */
typedef struct wl_nan_cmn_get_stat {
	uint32	modules_btmap;	/* Bitmap to indicate module stats are needed:
				 * See NAN Sub Module ID's above
				 */
	uint8	operation;	/* Get, Get and Clear */
	uint8	arg1;		/* Submodule control variable1 */
	uint8	arg2;		/* Submodule control variable2 */
	uint8	pad;		/* May not be needed as TLV's are aligned,add to pass compile chk */
} wl_nan_cmn_get_stat_t;

/* Output for Stats container */
typedef struct wl_nan_cmn_stat {
	uint32	n_stats;	/* Number of different sub TLV stats present in the container */
	uint32	totlen;		/* Total Length of stats data in container */
	uint8   stats_tlvs [];	/* Stat TLV's container */
} wl_nan_cmn_stat_t;

/* Defines for operation */
#define WLA_NAN_STATS_GET	0
#define WLA_NAN_STATS_GET_CLEAR	1

#define WL_NAN_STAT_ALL 0xFFFFFFFF

/* NAN Mac stats */

typedef struct wl_nan_mac_band_stats {
	uint32 bcn_tx;	/* 2g/5g disc/sync beacon tx count */
	uint32 bcn_rx;	/* 2g/5g disc/sync beacon rx count */
	uint32 dws;	/* Number of 2g/5g DW's */
} wl_nan_mac_band_stats_t;

typedef struct wl_nan_mac_stats {
	wl_nan_mac_band_stats_t	band[NAN_MAX_BANDS];	/* MAC sync band specific stats */
	uint32 naf_tx;			/* NAN AF tx */
	uint32 naf_rx;			/* NAN AF rx */
	uint32 sdf_tx;			/* SDF tx */
	uint32 sdf_rx;			/* SDF rx */
} wl_nan_mac_stats_t;

/* NAN Sched stats */
/* Per core Sched stats */
typedef struct nan_sched_stats_core {
	uint32	slotstart;		/* slot_start */
	uint32	slotend;		/* slot_end */
	uint32	slotskip;		/* slot_skip */
	uint32	slotstart_partial;	/* slot resume */
	uint32	slotend_partial;	/* slot pre-empt */
} nan_sched_stats_core_t;
/* Common Sched stats */
typedef struct nan_sched_stats_cmn {
	uint32	slot_adj_dw;	/* Slot adjusts due to DW changes */
	uint32	slot_dur;	/* Total slot duration in TU's */
} nan_sched_stats_cmn_t;

typedef struct nan_sched_stats {
	nan_sched_stats_cmn_t cmn;
	nan_sched_stats_core_t slice[MAX_NUM_D11CORES];
} nan_sched_stats_t;
/* End NAN Sched stats */

/* NAN Discovery stats */
typedef struct nan_disc_stats {
	uint32 pub_tx;		/* Publish tx */
	uint32 pub_rx;		/* Publish rx */
	uint32 sub_tx;		/* Subscribe tx */
	uint32 sub_rx;		/* Subscribe rx */
	uint32 fup_tx;		/* Followup tx */
	uint32 fup_rx;		/* Followup rx */
} nan_disc_stats_t;
/* NAN Discovery stats end */

/* statistics for nan sec */
typedef struct nan_sec_stats_s {
	uint32 mic_fail;		/* rx mic fail */
	uint32 replay_fail;		/* replay counter */
	uint32 tx_fail;			/* tx fail (from txstatus) */
	uint32 key_info_err;		/* key info field err */
	uint32 ok_sessions;		/* successful mx negotiations */
	uint32 fail_sessions;		/* failed sessions */
	uint32 keydesc_err;		/* key desc error */
	uint32 invalid_cipher;		/* cipher suite not valid */
	uint32 pmk_not_found;		/* no pmk found for given service or for any reason */
	uint32 no_pmk_for_pmkid;	/* no pmk found for give pmkid */
	uint32 key_install_err;		/* failed to install keys */
	uint32 no_keydesc_attr;		/* key desc attr missing */
	uint32 nonce_mismatch;		/* nonce mismatch */
} nan_sec_stats_t;

/* WL_NAN_XTLV_GEN_PEER_STATS */
typedef struct wl_nan_peer_stats {
	struct ether_addr nmi;
	uint8 pad[2];
	uint32 pkt_enq; /* counter for queued pkt of peer */

	/* NDL */
	bool ndl_exist;
	uint8 ndl_state;
	bool counter_proposed;
	uint8 pad1;

	/* NDL QoS */
	uint16 local_max_latency;
	uint16 peer_max_latency;
	uint8 local_min_slots;
	uint8 peer_min_slots;

	/* security association */
	struct ether_addr sec_laddr;	/* local mac addr */
	struct ether_addr sec_raddr;	/* remote mac addr */
	uint8 sec_csid;
	uint8 pad2;
} wl_nan_peer_stats_t;

/* WL_NAN_XTLV_GEN_PEER_STATS_DEVCAP */
typedef struct wl_nan_peer_stats_dev_cap {
	uint8 mapid;
	uint8 awake_dw_2g;
	uint8 awake_dw_5g;
	uint8 bands_supported;
	uint8 op_mode;
	uint8 num_antennas;
	uint16 chan_switch_time;
	uint8 capabilities;
	uint8 pad[3];
} wl_nan_peer_stats_dev_cap_t;

/* WL_NAN_XTLV_GEN_PEER_STATS_NDP */
typedef struct wl_nan_peer_stats_ndp {
	uint8 peer_role;
	uint8 ndp_state;
	uint8 indp_id;	/* initiator ndp id */
	uint8 ndp_ctrl;	/* ndp control field */
	struct ether_addr peer_nmi;
	struct ether_addr peer_ndi;
	struct ether_addr local_ndi;

	/* peer scb info */
	bool scb_allocated;
	bool scb_found;
	uint32 scb_flags;
	uint32 scb_flags2;
	uint32 scb_flags3;
} wl_nan_peer_stats_ndp_t;

/* WL_NAN_XTLV_GEN_PEER_STATS_SCHED */
typedef struct wl_nan_peer_stats_sched_slot {
	uint16 info;			/* will capture NDC and Immutable */
	chanspec_t chanspec;
} wl_nan_peer_stats_sched_slot_t;
typedef struct wl_nan_peer_stats_sched {
	uint8 map_id;
	uint8 seq_id;	/* seq id from NA attr */
	uint8 slot_dur;
	uint8 pad;
	uint16 period;
	uint16 num_slot;
	wl_nan_peer_stats_sched_slot_t slot[];
} wl_nan_peer_stats_sched_t;

/** status - TBD BCME_ vs NAN status - range reserved for BCME_ */
enum {
	/* add new status here... */
	WL_NAN_E_DAM_DOUBLE_REMOVE	= -2097,	/* remove peer schedule already removed */
	WL_NAN_E_DAM_DOUBLE_MERGE	= -2096,	/* merge peer schedule already merged */
	WL_NAN_E_DAM_REJECT_INVALID	= -2095,	/* reject for invalid schedule */
	WL_NAN_E_DAM_REJECT_RANGE	= -2094,
	WL_NAN_E_DAM_REJECT_QOS		= -2093,
	WL_NAN_E_DAM_REJECT_NDC		= -2092,
	WL_NAN_E_DAM_REJECT_PEER_IMMUT	= -2091,
	WL_NAN_E_DAM_REJECT_LCL_IMMUT	= -2090,
	WL_NAN_E_DAM_EXCEED_NUM_SCHED	= -2089,
	WL_NAN_E_DAM_INVALID_SCHED_MAP	= -2088,	/* invalid schedule map list */
	WL_NAN_E_DAM_INVALID_LCL_SCHED	= -2087,
	WL_NAN_E_INVALID_MAP_ID		= -2086,
	WL_NAN_E_CHAN_OVERLAP_ACROSS_MAP = -2085,
	WL_NAN_E_INVALID_CHAN_LIST	= -2084,
	WL_NAN_E_INVALID_RANGE_TBMP	= -2083,
	WL_NAN_E_INVALID_IMMUT_SCHED	= -2082,
	WL_NAN_E_INVALID_NDC_ATTR	= -2081,
	WL_NAN_E_INVALID_TIME_BITMAP	= -2080,
	WL_NAN_E_INVALID_NA_ATTR	= -2079,
	WL_NAN_E_NO_NA_ATTR_IN_AVAIL_MAP = -2078,	/* no na attr saved in avail map */
	WL_NAN_E_INVALID_MAP_IDX	= -2077,
	WL_NAN_E_SEC_SA_NOTFOUND	= -2076,
	WL_NAN_E_BSSCFG_NOTFOUND	= -2075,
	WL_NAN_E_SCB_NOTFOUND		= -2074,
	WL_NAN_E_NCS_SK_KDESC_TYPE      = -2073,
	WL_NAN_E_NCS_SK_KEY_DESC_VER    = -2072,	/* key descr ver */
	WL_NAN_E_NCS_SK_KEY_TYPE        = -2071,	/* key descr type */
	WL_NAN_E_NCS_SK_KEYINFO_FAIL    = -2070,	/* key info (generic) */
	WL_NAN_E_NCS_SK_KEY_LEN         = -2069,	/* key len */
	WL_NAN_E_NCS_SK_KDESC_NOT_FOUND = -2068,	/* key desc not found */
	WL_NAN_E_NCS_SK_INVALID_PARAMS  = -2067,	/* invalid args */
	WL_NAN_E_NCS_SK_KDESC_INVALID   = -2066,	/* key descr is not valid */
	WL_NAN_E_NCS_SK_NONCE_MISMATCH  = -2065,
	WL_NAN_E_NCS_SK_KDATA_SAVE_FAIL = -2064,	/* not able to save key data */
	WL_NAN_E_NCS_SK_AUTH_TOKEN_CALC_FAIL = -2063,
	WL_NAN_E_NCS_SK_PTK_CALC_FAIL   = -2062,
	WL_NAN_E_INVALID_STARTOFFSET	= -2061,
	WL_NAN_E_BAD_NA_ENTRY_TYPE	= -2060,
	WL_NAN_E_INVALID_CHANBMP	= -2059,
	WL_NAN_E_INVALID_OP_CLASS	= -2058,
	WL_NAN_E_NO_IES			= -2057,
	WL_NAN_E_NO_PEER_ENTRY_AVAIL	= -2056,
	WL_NAN_E_INVALID_PEER		= -2055,
	WL_NAN_E_PEER_EXISTS		= -2054,
	WL_NAN_E_PEER_NOTFOUND		= -2053,
	WL_NAN_E_NO_MEM			= -2052,
	WL_NAN_E_INVALID_OPTION		= -2051,
	WL_NAN_E_INVALID_BAND		= -2050,
	WL_NAN_E_INVALID_MAC		= -2049,
	WL_NAN_E_BAD_INSTANCE		= -2048,
	WL_NAN_E_NDC_EXISTS		= -2047,
	WL_NAN_E_NO_NDC_ENTRY_AVAIL	= -2046,
	WL_NAN_E_INVALID_NDC_ENTRY      = -2045,
	WL_NAN_E_SD_TX_LIST_FULL        = -2044,
	WL_NAN_E_ERROR			= -1,
	WL_NAN_E_OK			= 0
};

typedef int32 wl_nan_status_t;

/** nan cmd list entry  */
enum wl_nan_sub_cmd_input_flags {
	WL_NAN_SUB_CMD_FLAG_NONE = 0,
	WL_NAN_SUB_CMD_FLAG_SKIP = 1, /* Skip to next sub-command on error */
	WL_NAN_SUB_CMD_FLAG_TERMINATE = 2, /* Terminate processing and return */
	WL_NAN_SUB_CMD_FLAG_LAST /* Keep this at the end */
};

/** container for nan events */
typedef struct wl_nan_ioc {
	uint16	version;	/**< interface command or event version */
	uint16	id;			/**< nan ioctl cmd  ID  */
	uint16	len;		/**< total length of all tlv records in data[]  */
	uint16	pad;		/**< pad to be 32 bit aligment */
	uint8	data [];	/**< var len payload of bcm_xtlv_t type */
} wl_nan_ioc_t;

/*
 * NAN sub-command data structures
 */

/*
 * Config component WL_NAN_CMD_CFG_XXXX sub-commands
 * WL_NAN_CMD_CFG_ENABLE
 */
enum wl_nan_config_state {
	WL_NAN_CONFIG_STATE_DISABLE = 0,
	WL_NAN_CONFIG_STATE_ENABLE = 1
};

typedef int8 wl_nan_config_state_t;

/* WL_NAN_CMD_CFG_NAN_INIT */

typedef uint8 wl_nan_init_t;

/* WL_NAN_CMD_CFG_NAN_VERSION */
typedef uint16 wl_nan_ver_t;

/* WL_NAN_CMD_CFG_NAN_CONFIG  */
typedef uint32 wl_nan_cfg_ctrl_t;

/*
 * WL_NAN_CMD_CFG_BAND, WL_NAN_CMD_CFG_RSSI_THRESHOLD(Get only)
 */
typedef uint8 wl_nan_band_t;

/*
 * WL_NAN_CMD_CFG_ROLE
 */
enum wl_nan_role {
	WL_NAN_ROLE_AUTO = 0,
	WL_NAN_ROLE_NON_MASTER_NON_SYNC = 1,
	WL_NAN_ROLE_NON_MASTER_SYNC = 2,
	WL_NAN_ROLE_MASTER = 3,
	WL_NAN_ROLE_ANCHOR_MASTER = 4
};

typedef uint8 wl_nan_role_t;

typedef struct wl_nan_device_state
{
	wl_nan_role_t role;		/* Sync Master, Non-Sync Master */
	uint8 state;	/* TBD  */
	uint8 hopcount;	/* Hops to the Anchor Master */
	struct ether_addr immediate_master; /* Master MAC */
	struct ether_addr anchor_master;	/* Anchor Master MAC */
	struct ether_addr cluster_id; /* Cluster ID to which this device belongs to */
	uint8 PAD[3];
	uint32 tsf_high;  /* NAN Cluster TSFs */
	uint32 tsf_low;
} wl_nan_device_state_t;

/*
 * WL_NAN_CMD_CFG_HOP_CNT, WL_NAN_CMD_CFG_HOP_LIMIT
 */
typedef uint8 wl_nan_hop_count_t;

/*
 * WL_NAN_CMD_CFG_WARMUP_TIME
 */
typedef uint32 wl_nan_warmup_time_ticks_t;

/*
 * WL_NAN_CMD_CFG_RSSI_THRESHOLD
 * rssi_close and rssi_mid are used to transition master to non-master
 * role by NAN state machine. rssi thresholds corresponding to the band
 * will be updated.
 */
/* To be deprecated */
typedef struct wl_nan_rssi_threshold {
	wl_nan_band_t band;
	int8 rssi_close;
	int8 rssi_mid;
	uint8 pad;
} wl_nan_rssi_threshold_t;

/* WL_NAN_CMD_ELECTION_RSSI_THRESHOLD */

typedef struct wl_nan_rssi_thld {
	int8 rssi_close_2g;
	int8 rssi_mid_2g;
	int8 rssi_close_5g;
	int8 rssi_mid_5g;
} wl_nan_rssi_thld_t;

/* WL_NAN_CMD_DATA_MAX_PEERS */

typedef uint8 wl_nan_max_peers_t;

/*
 * WL_NAN_CMD_CFG_STATUS
 */

typedef enum wl_nan_election_mode {
	WL_NAN_ELECTION_RUN_BY_HOST = 1,
	WL_NAN_ELECTION_RUN_BY_FW = 2
} wl_nan_election_mode_t;

typedef struct wl_nan_conf_status {
	struct ether_addr	nmi;		/*  NAN mgmt interface address */
	uint8			enabled;	/* NAN is enabled */
	uint8			role;		/* Current nan sync role */
	struct ether_addr	cid;		/*  Current Cluster id */
	uint8			social_chans[2];	/* Social channels */
	uint8			mr[8];		/* Master Rank */
	uint8			amr[8];		/* Anchor Master Rank */
	uint32			ambtt;		/* Anchor master beacon target time */
	uint32			cluster_tsf_h;	/* Current Cluster TSF High */
	uint32			cluster_tsf_l;	/* Current Cluster TSF Low */
	uint8			election_mode; /* Election mode, host or firmware */
	uint8			hop_count;	/* Current Hop count */
	uint8			pad[2];
} wl_nan_conf_status_t;

/*
 * WL_NAN_CMD_CFG_OUI
 */
typedef struct wl_nan_oui_type {
	uint8 nan_oui[DOT11_OUI_LEN];
	uint8 type;
} wl_nan_oui_type_t;

/*
 * WL_NAN_CMD_CFG_COUNT
 */
#ifndef LINUX_POSTMOGRIFY_REMOVAL
typedef struct wl_nan_count {
	uint32 cnt_bcn_tx;		/**< TX disc/sync beacon count */
	uint32 cnt_bcn_rx;		/**< RX disc/sync beacon count */
	uint32 cnt_svc_disc_tx;		/**< TX svc disc frame count */
	uint32 cnt_svc_disc_rx;		/**< RX svc disc frame count */
} wl_nan_count_t;
#endif /* LINUX_POSTMOGRIFY_REMOVAL */
/*
 * Election component WL_NAN_CMD_ELECTION_XXXX sub-commands
 * WL_NAN_CMD_ELECTION_HOST_ENABLE
 */
enum wl_nan_enable_flags {
	WL_NAN_DISABLE_FLAG_HOST_ELECTION = 0,
	WL_NAN_ENABLE_FLAG_HOST_ELECTION = 1
};

/*
 * 0 - disable host based election
 * 1 - enable host based election
 */
typedef uint8 wl_nan_host_enable_t;

/*
 * WL_NAN_CMD_ELECTION_METRICS_CONFIG
 */
/* Set only */
typedef struct wl_nan_election_metric_config {
	uint8 random_factor; /* Configured random factor */
	uint8 master_pref;	/* configured master preference */
	uint8 pad[2];
} wl_nan_election_metric_config_t;

/*
 * WL_NAN_CMD_ELECTION_METRICS_STATE
 */
/* Get only */
typedef struct wl_nan_election_metric_state {
	uint8 random_factor; /* random factor used in MIs */
	uint8 master_pref;	 /* Master advertised in MIs */
	uint8 pad[2];
} wl_nan_election_metric_state_t;

/*
 * WL_NAN_CMD_ELECTION_LEAVE
 * WL_NAN_CMD_ELECTION_STOP
 */
typedef struct ether_addr wl_nan_cluster_id_t;

/*
 * WL_NAN_CMD_ELECTION_MERGE
 * 0 - disable cluster merge
 * 1 - enable cluster merge
 */
typedef uint8 wl_nan_merge_enable_t;

/*
 * WL_NAN_CMD_CFG_ROLE
 * role = 0 means configuration by firmware; otherwise by host
 * when host configures role, also need target master address to sync to
 */
#define NAN_SYNC_MASTER_SELF	0
#define NAN_SYNC_MASTER_AM		1
#define NAN_SYNC_MASTER_INTERMEDIATE	2
/* 	ltsf_h, ltsf_l:
	The local TSF timestamp filled in by FW in the WL_NAN_EVENT_BCN_RX event;
	rtsf_h, rtsf_l:
	The timestamp in the Rx beacon frame, filled in by host
	uint32 ambtt:
	the amtt in the cluster ID attribute in the Rx beacon frame
*/
typedef struct nan_sync_master {
	uint8 flag;	/* 0: self, 1: anchor-master, 2: intermediate master */
	uint8  hop_count;
	struct ether_addr addr;
	struct ether_addr cluster_id;
	chanspec_t channel; /* bcn reception channel */
	uint32 ltsf_h;
	uint32 ltsf_l;
	uint32 rtsf_h;
	uint32 rtsf_l;
	uint8 amr[WL_NAN_MASTER_RANK_LEN];
	uint32 ambtt;
} nan_sync_master_t;

/* NAN advertiser structure */
/* TODO RSDB: add chspec to indicates core corresponds correct core */
typedef struct nan_adv_entry {
	uint8 age;	/* used to remove stale entries */
	uint8 hop_count;
	struct ether_addr addr;
	struct ether_addr cluster_id;
	chanspec_t channel; /* bcn reception channel */
	uint32 ltsf_h;
	uint32 ltsf_l;
	uint32 rtsf_h;
	uint32 rtsf_l;
	uint8 amr[WL_NAN_MASTER_RANK_LEN];
	uint32 ambtt;
	int8	rssi[NAN_MAX_BANDS];		/* rssi last af was received at */
	int8	last_rssi[NAN_MAX_BANDS];	/* rssi in the last AF */
} nan_adv_entry_t;

typedef struct nan_adv_table {
	uint8  num_adv;
	uint8	adv_size;
	uint8	pad[2];
	nan_adv_entry_t adv_nodes[0];
} nan_adv_table_t;

typedef struct wl_nan_role_cfg {
	wl_nan_role_t cfg_role;
	wl_nan_role_t cur_role;
	uint8 pad[2];
	nan_sync_master_t target_master;
} wl_nan_role_cfg_t;

typedef struct wl_nan_role_config {
	wl_nan_role_t role;
	struct ether_addr target_master;
	uint8 pad;
} wl_nan_role_config_t;

typedef int8 wl_nan_sd_optional_field_types_t;

/* Flag bits for Publish and Subscribe (wl_nan_sd_params_t flags) */

#define WL_NAN_RANGE_LIMITED		0x0040

/* Event generation indicator (default is continuous) */

#define WL_NAN_MATCH_ONCE		0x100000
#define WL_NAN_MATCH_NEVER		0x200000

/* Bits specific to Publish */

#define WL_NAN_PUB_UNSOLICIT	0x1000	/* Unsolicited Tx */
#define WL_NAN_PUB_SOLICIT		0x2000	/* Solicited Tx */
#define WL_NAN_PUB_BOTH			0x3000	/* Both the above */

#define WL_NAN_PUB_BCAST		0x4000	/* bcast solicited Tx only */
#define WL_NAN_PUB_EVENT		0x8000	/* Event on each solicited Tx */
#define WL_NAN_PUB_SOLICIT_PENDING	0x10000 /* Used for one-time solicited Publish */

#define WL_NAN_FOLLOWUP			0x20000 /* Follow-up frames */

/* Bits specific to Subscribe */

#define WL_NAN_SUB_ACTIVE		0x1000 /* Active subscribe mode */
#define WL_NAN_SUB_MATCH_IF_SVC_INFO	0x2000 /* Service info in publish */

#define WL_NAN_TTL_UNTIL_CANCEL	0xFFFFFFFF /* Special values for time to live (ttl) parameter */

/*
 * Publish -  runs until first transmission
 * Subscribe - runs until first  DiscoveryResult event
 */
#define WL_NAN_TTL_FIRST	0

/*
 * WL_NAN_CMD_SD_PARAMS
 */
typedef struct wl_nan_sd_params
{
	uint16	length; /* length including options */
	uint8	period; /* period of the unsolicited SDF xmission in DWs */
	uint8   pad;
	uint8	svc_hash[WL_NAN_SVC_HASH_LEN]; /* Hash for the service name */
	uint8	instance_id; /* Instance of the current service */
	int8	proximity_rssi; /* RSSI limit to Rx subscribe or pub SDF 0 no effect */
	uint32	flags; /* bitmap representing aforesaid optional flags */
	int32	ttl; /* TTL for this instance id, -1 will run till cancelled */
	tlv_t	optional[1]; /* optional fields in the SDF  as appropriate */
} wl_nan_sd_params_t;

/*
 * WL_NAN_CMD_SD_PUBLISH_LIST
 * WL_NAN_CMD_SD_SUBSCRIBE_LIST
 */
typedef struct wl_nan_service_info
{
	uint8 instance_id;	/* Publish instance ID */
	uint8 service_hash[WL_NAN_SVC_HASH_LEN]; /* Hash for service name */
} wl_nan_service_info_t;

typedef struct wl_nan_service_list
{
	uint16 id_count; /* Number of registered publish/subscribe services */
	wl_nan_service_info_t list[1]; /* service info defined by nan_service instance */
} wl_nan_service_list_t;

/*
 * WL_NAN_CMD_CFG_BCN_INTERVAL
 */
typedef uint16 wl_nan_disc_bcn_interval_t;

/*
 * WL_NAN_CMD_CFG_SDF_TXTIME
 */
typedef uint16 wl_nan_svc_disc_txtime_t;

/*
 * WL_NAN_CMD_CFG_STOP_BCN_TX
 */
typedef uint16 wl_nan_stop_bcn_tx_t;

/*
 * WL_NAN_CMD_CFG_SID_BEACON
 */
typedef struct wl_nan_sid_beacon_control {
	uint8 sid_enable;	/* Flag to indicate the inclusion of Service IDs in Beacons */
	uint8 sid_count;	/* Limit for number of SIDs to be included in Beacons */
	uint8 pad[2];
} wl_nan_sid_beacon_control_t;

/*
 * WL_NAN_CMD_CFG_DW_LEN
 */
typedef uint16 wl_nan_dw_len_t;

/*
 * WL_NAN_CMD_CFG_AWAKE_DW   Will be deprecated.
 */
typedef struct wl_nan_awake_dw {
	wl_nan_band_t band;	/* 0 - b mode 1- a mode */
	uint8 interval;		/* 1 or 2 or 4 or 8 or 16 */
	uint16 pad;
} wl_nan_awake_dw_t;

/*
 * WL_NAN_CMD_CFG_AWAKE_DWS
 */
typedef struct wl_nan_awake_dws {
	uint8 dw_interval_2g;	/* 2G DW interval */
	uint8 dw_interval_5g;	/* 5G DW interval */
	uint16 pad;
} wl_nan_awake_dws_t;

/* WL_NAN_CMD_SYNC_BCN_RSSI_NOTIF_THRESHOLD */

typedef struct wl_nan_rssi_notif_thld {
	int8 bcn_rssi_2g;
	int8 bcn_rssi_5g;
	int16 pad;
} wl_nan_rssi_notif_thld_t;

/*
 * WL_NAN_CMD_CFG_SOCIAL_CHAN
 */
typedef struct wl_nan_social_channels {
	uint8 soc_chan_2g;	/* 2G social channel */
	uint8 soc_chan_5g;	/* 5G social channel */
	uint16 pad;
} wl_nan_social_channels_t;

/*
 * WL_NAN_CMD_SD_CANCEL_PUBLISH
 * WL_NAN_CMD_SD_CANCEL_SUBSCRIBE
 */
typedef uint8 wl_nan_instance_id; /* Instance ID of an active publish instance */

/*
 * WL_NAN_CMD_SD_VND_INFO
 */
typedef struct wl_nan_sd_vendor_info
{
	uint16 length; /* Size in bytes of the payload following this field */
	uint8 data[];	/* Vendor Information */
} wl_nan_sd_vendor_info_t;

/*
 * WL_NAN_CMD_SD_STATS
 */
typedef struct wl_nan_sd_stats {
	uint32  sdftx;
	uint32  sdfrx;
	uint32  sdsrffail;
	uint32  sdrejrssi;
	uint32  sdfollowuprx;
	uint32  sdsubmatch;
	uint32  sdpubreplied;
	uint32  sdmftfail1;
	uint32  sdmftfail2;
	uint32  sdmftfail3;
	uint32  sdmftfail4;
}  wl_nan_sd_stats_t;

/*
 * WL_NAN_CMD_SD_TRANSMIT
 * WL_NAN_CMD_SD_FUP_TRANSMIT
 */
typedef struct wl_nan_sd_transmit {
	uint8 local_service_id; /* Sender Service ID */
	uint8 requestor_service_id; /* Destination Service ID */
	struct ether_addr destination_addr; /* Destination MAC */
	uint16 token; /* follow_up_token when a follow-up msg is queued successfully */
	uint8 priority; /* requested relative prio */
	uint8 service_info_len; /* size in bytes of the service info payload */
	uint8 service_info[]; /* Service Info payload */
} wl_nan_sd_transmit_t;

/*
 * WL_NAN_CMD_SYNC_TSRESERVE
 */
#ifndef LINUX_POSTMOGRIFY_REMOVAL
/** time slot */
#define NAN_MAX_TIMESLOT	32
typedef struct wl_nan_timeslot {
	uint32	abitmap; /**< available bitmap */
	uint32 chanlist[NAN_MAX_TIMESLOT];
} wl_nan_timeslot_t;
#endif /* LINUX_POSTMOGRIFY_REMOVAL */

/*
 * Deprecated
 *
 * WL_NAN_CMD_SYNC_TSRELEASE
 */
typedef uint32 wl_nan_ts_bitmap_t;

/* nan passive scan params */
#define NAN_SCAN_MAX_CHCNT 8
typedef struct wl_nan_scan_params {
	uint16 scan_time;
	uint16 home_time;
	uint16 ms_intvl; /**< interval between merge scan */
	uint16 ms_dur;  /**< duration of merge scan */
	uint16 chspec_num;
	uint8 pad[2];
	chanspec_t chspec_list[NAN_SCAN_MAX_CHCNT]; /**< act. used 3, 5 rfu */
} wl_nan_scan_params_t;

/*
 * WL_NAN_CMD_DBG_SCAN
 */
typedef struct wl_nan_dbg_scan {
	struct ether_addr cid;
	uint8 pad[2];
} wl_nan_dbg_scan_t;

/* NAN_DBG_LEVEL */
typedef struct wl_nan_dbg_level {
	uint32 nan_err_level; /* for Error levels */
	uint32 nan_dbg_level; /* for bebug logs and trace */
	uint32 nan_info_level; /* for dumps like prhex */
} wl_nan_dbg_level_t;

/*
 * WL_NAN_CMD_DBG_EVENT_MASK
 */
typedef uint32 wl_nan_event_mask_t;

/*
 * WL_NAN_CMD_DBG_EVENT_CHECK
 */
typedef uint8 wl_nan_dbg_ifname[BCM_MSG_IFNAME_MAX];

/*
 * WL_NAN_CMD_DBG_DUMP
 * WL_NAN_CMD_DBG_CLEAR
 */
enum wl_nan_dbg_dump_type {
	WL_NAN_DBG_DT_RSSI_DATA = 1,
	WL_NAN_DBG_DT_STATS_DATA = 2,
	/*
	 * Additional enums before this line
	 */
	WL_NAN_DBG_DT_INVALID
};
typedef int8 wl_nan_dbg_dump_type_t;

/** various params and ctl swithce for nan_debug instance  */
/*
 * WL_NAN_CMD_DBG_DEBUG
 */
typedef struct wl_nan_debug_params {
	uint16	cmd;	/**< debug cmd to perform a debug action */
	uint16	status;
	uint32	msglevel; /**< msg level if enabled */
	uint8	enabled; /**< runtime debuging enabled */
	uint8 collect;
	uint8 PAD[2];
} wl_nan_debug_params_t;

typedef struct wl_nan_sched_svc_timeslot_s {
	uint32 abitmap; /* availability bitmap */
	uint32 chanlist[NAN_MAX_TIMESLOT];
	uint8  res; /* resolution: 0 = 16ms, 1 = 32ms, 2 = 64ms 3 = reserved. REfer NAN spec */
	uint8  mapid; /* mapid from NAN spec. Used to differentiate 2G Vs 5G band */
	uint8 PAD[2];
} wl_nan_sched_svc_timeslot_t;

/* nan passive scan params */
#define NAN_SCAN_MAX_CHCNT 8
typedef struct nan_scan_params {
	uint16 scan_time;
	uint16 home_time;
	uint16 ms_intvl; /**< interval between merge scan */
	uint16 ms_dur;  /**< duration of merge scan */
	uint16 chspec_num;
	uint8 pad[2];
	chanspec_t chspec_list[NAN_SCAN_MAX_CHCNT]; /**< act. used 3, 5 rfu */
} nan_scan_params_t;

/* nan cmd IDs */
enum wl_nan_cmds {
	 /* nan cfg /disc & dbg ioctls */
	WL_NAN_CMD_ENABLE = 1,
	WL_NAN_CMD_ATTR = 2,
	WL_NAN_CMD_NAN_JOIN = 3,
	WL_NAN_CMD_LEAVE = 4,
	WL_NAN_CMD_MERGE = 5,
	WL_NAN_CMD_STATUS = 6,
	WL_NAN_CMD_TSRESERVE = 7,
	WL_NAN_CMD_TSSCHEDULE = 8,
	WL_NAN_CMD_TSRELEASE = 9,
	WL_NAN_CMD_OUI = 10,
	WL_NAN_CMD_OOB_AF = 11,

	WL_NAN_CMD_COUNT = 15,
	WL_NAN_CMD_CLEARCOUNT = 16,

	/*  discovery engine commands */
	WL_NAN_CMD_PUBLISH = 20,
	WL_NAN_CMD_SUBSCRIBE = 21,
	WL_NAN_CMD_CANCEL_PUBLISH = 22,
	WL_NAN_CMD_CANCEL_SUBSCRIBE = 23,
	WL_NAN_CMD_TRANSMIT = 24,
	WL_NAN_CMD_CONNECTION = 25,
	WL_NAN_CMD_SHOW = 26,
	WL_NAN_CMD_STOP = 27,	/* stop nan for a given cluster ID  */
	/*  nan debug iovars & cmds  */
	WL_NAN_CMD_SCAN_PARAMS = 46,
	WL_NAN_CMD_SCAN = 47,
	WL_NAN_CMD_SCAN_RESULTS = 48,
	WL_NAN_CMD_EVENT_MASK = 49,
	WL_NAN_CMD_EVENT_CHECK = 50,
	WL_NAN_CMD_DUMP = 51,
	WL_NAN_CMD_CLEAR = 52,
	WL_NAN_CMD_RSSI = 53,

	WL_NAN_CMD_DEBUG = 60,
	WL_NAN_CMD_TEST1 = 61,
	WL_NAN_CMD_TEST2 = 62,
	WL_NAN_CMD_TEST3 = 63,
	WL_NAN_CMD_DISC_RESULTS = 64,
	/* nan 2.0 data path commands */
	WL_NAN_CMD_DATAPATH = 65
};

/*   NAN DP interface commands  */
enum wl_nan_dp_cmds {
	/* nan 2.0 ioctls */
	WL_NAN_CMD_DP_CAP = 1000,
	WL_NAN_CMD_DP_CONFIG = 1001,
	WL_NAN_CMD_DP_CREATE = 1002,
	WL_NAN_CMD_DP_AUTO_CONNECT = 1003,
	WL_NAN_CMD_DP_DATA_REQ = 1004,
	WL_NAN_CMD_DP_DATA_RESP = 1005,
	WL_NAN_CMD_DP_SCHED_UPD = 1006,
	WL_NAN_CMD_DP_END = 1007,
	WL_NAN_CMD_DP_CONNECT = 1008,
	WL_NAN_CMD_DP_STATUS = 1009
};

/* TODO Should remove this fixed length */
#define WL_NAN_DATA_SVC_SPEC_INFO_LEN 32 /* arbitrary */
#define WL_NAN_DP_MAX_SVC_INFO	      0xFF
#define WL_NAN_DATA_NDP_INST_SUPPORT 16

/* Nan flags */
#define WL_NAN_DP_FLAG_SVC_INFO	      (1 << 0)
#define WL_NAN_DP_FLAG_CONFIRM	      (1 << 1)
#define WL_NAN_DP_FLAG_EXPLICIT_CFM   (1 << 2)
#define WL_NAN_DP_FLAG_SECURITY	      (1 << 3)

/* NAN Datapath host status */
#define WL_NAN_DP_STATUS_ACCEPTED     1
#define WL_NAN_DP_STATUS_REJECTED     0

/* to be done */
typedef struct wl_nan_dp_cap {
	uint8 tbd;
} wl_nan_dp_cap_t;

/** The service hash (service id) is exactly this many bytes. */
#define WL_NAN_SVC_HASH_LEN	6
#ifndef LINUX_POSTMOGRIFY_REMOVAL
/** Number of hash functions per bloom filter */
#define WL_NAN_HASHES_PER_BLOOM 4
#endif /* LINUX_POSTMOGRIFY_REMOVAL */
/* no. of max last disc results */
#define WL_NAN_MAX_DISC_RESULTS	3

/* NAN security related defines */
/* NCS-SK related */
#define WL_NAN_NCS_SK_PMK_LEN	32
#define WL_NAN_NCS_SK_PMKID_LEN	16

/* recent discovery results */
typedef struct wl_nan_disc_result_s
{
	wl_nan_instance_id_t instance_id;	/* instance id of pub/sub req */
	wl_nan_instance_id_t peer_instance_id;	/* peer instance id of pub/sub req/resp */
	uint8 svc_hash[WL_NAN_SVC_HASH_LEN];	/* service descp string */
	struct ether_addr peer_mac;	/* peer mac address */
} wl_nan_disc_result_t;

/* list of recent discovery results */
typedef struct wl_nan_disc_results_s
{
	wl_nan_disc_result_t disc_result[WL_NAN_MAX_DISC_RESULTS];
} wl_nan_disc_results_list_t;

/* nan 1.0 events */
/* To be deprecated - will be replaced by event_disc_result */
typedef struct wl_nan_ev_disc_result {
	wl_nan_instance_id_t pub_id;
	wl_nan_instance_id_t sub_id;
	struct ether_addr pub_mac;
	uint8 opt_tlvs[0];
} wl_nan_ev_disc_result_t;

typedef struct wl_nan_event_disc_result {
	wl_nan_instance_id_t pub_id;
	wl_nan_instance_id_t sub_id;
	struct ether_addr pub_mac;
	int8		publish_rssi;		/* publisher RSSI */
	uint8		attr_num;
	uint16		attr_list_len;	/* length of the all the attributes in the SDF */
	uint8		attr_list[0];	/* list of NAN attributes */
} wl_nan_event_disc_result_t;

typedef struct wl_nan_ev_p2p_avail {
	struct ether_addr sender;
	struct ether_addr p2p_dev_addr;
	uint8 dev_role;
	uint8 resolution;
	uint8 repeat;
	uint8 pad[3];
	chanspec_t chanspec;
	uint32 avail_bmap;
} wl_nan_ev_p2p_avail_t;

/*
* discovery interface event structures *
*/

/* mandatory parameters for OOB action frame */
/* single-shot when bitmap and offset are set to 0; periodic otherwise */
typedef struct wl_nan_oob_af_params_s
{
	/* bitmap for the 32 timeslots in 512TU dw interval */
	uint32 ts_map;
	/* offset from start of dw, in us */
	uint32 tx_offset;
	struct ether_addr bssid;
	struct ether_addr dest;
	uint32 pkt_lifetime;
	uint16 payload_len;
	uint8 payload[1];
} wl_nan_oob_af_params_t;

/* NAN Ranging */

/* Bit defines for global flags */
#define WL_NAN_RANGING_ENABLE		1 /**< enable RTT */
#define WL_NAN_RANGING_RANGED		2 /**< Report to host if ranged as target */
typedef struct nan_ranging_config {
	uint32 chanspec;		/**< Ranging chanspec */
	uint16 timeslot;		/**< NAN RTT start time slot  1-511 */
	uint16 duration;		/**< NAN RTT duration in ms */
	struct ether_addr allow_mac;	/**< peer initiated ranging: the allowed peer mac
					 * address, a unicast (for one peer) or
					 * a broadcast for all. Setting it to all zeros
					 * means responding to none,same as not setting
					 * the flag bit NAN_RANGING_RESPOND
					 */
	uint16 flags;
} wl_nan_ranging_config_t;

/** list of peers for self initiated ranging */
/** Bit defines for per peer flags */
#define WL_NAN_RANGING_REPORT (1<<0)	/**< Enable reporting range to target */
typedef struct nan_ranging_peer {
	uint32 chanspec;		/**< desired chanspec for this peer */
	uint32 abitmap;			/**< available bitmap */
	struct ether_addr ea;		/**< peer MAC address */
	uint8 frmcnt;			/**< frame count */
	uint8 retrycnt;			/**< retry count */
	uint16 flags;			/**< per peer flags, report or not */
	uint16 PAD;
} wl_nan_ranging_peer_t;
typedef struct nan_ranging_list {
	uint8 count;			/**< number of MAC addresses */
	uint8 num_peers_done;		/**< host set to 0, when read, shows number of peers
					 * completed, success or fail
					 */
	uint8 num_dws;			/**< time period to do the ranging, specified in dws */
	uint8 reserve;			/**< reserved field */
	wl_nan_ranging_peer_t rp[1];	/**< variable length array of peers */
} wl_nan_ranging_list_t;

/* ranging results, a list for self initiated ranging and one for peer initiated ranging */
/* There will be one structure for each peer */
#define WL_NAN_RANGING_STATUS_SUCCESS		1
#define WL_NAN_RANGING_STATUS_FAIL		2
#define WL_NAN_RANGING_STATUS_TIMEOUT		3
#define WL_NAN_RANGING_STATUS_ABORT		4 /**< with partial results if sounding count > 0 */
typedef struct nan_ranging_result {
	uint8 status;			/**< 1: Success, 2: Fail 3: Timeout 4: Aborted */
	uint8 sounding_count;		/**< number of measurements completed (0 = failure) */
	struct ether_addr ea;		/**< initiator MAC address */
	uint32 chanspec;		/**< Chanspec where the ranging was done */
	uint32 timestamp;		/**< 32bits of the TSF timestamp ranging was completed at */
	uint32 distance;		/**< mean distance in meters expressed as Q4 number.
					 * Only valid when sounding_count > 0. Examples:
					 * 0x08 = 0.5m
					 * 0x10 = 1m
					 * 0x18 = 1.5m
					 * set to 0xffffffff to indicate invalid number
					 */
	int32 rtt_var;			/**< standard deviation in 10th of ns of RTTs measured.
					 * Only valid when sounding_count > 0
					 */
	struct ether_addr tgtea;	/**< target MAC address */
	uint8 PAD[2];
} wl_nan_ranging_result_t;
typedef struct nan_ranging_event_data {
	uint8 mode;			/**< 1: Result of host initiated ranging */
					/* 2: Result of peer initiated ranging */
	uint8 reserved;
	uint8 success_count;		/**< number of peers completed successfully */
	uint8 count;			/**< number of peers in the list */
	wl_nan_ranging_result_t rr[1];	/**< variable array of ranging peers */
} wl_nan_ranging_event_data_t;

enum {
	WL_NAN_STATS_RSSI = 1,
	WL_NAN_STATS_DATA = 2,
	WL_NAN_STATS_DP = 3,
/*
 * ***** ADD before this line ****
 */
	WL_NAN_STATS_INVALID
};
typedef struct wl_nan_dp_stats {
	uint32 tbd; /* TBD */
} wl_nan_dp_stats_t;

typedef struct wl_nan_stats {
	/* general */
	uint32 cnt_dw; /* DW slots */
	uint32 cnt_disc_bcn_sch; /* disc beacon slots */
	uint32 cnt_amr_exp; /* count of ambtt expiries resetting roles */
	uint32 cnt_bcn_upd; /* count of beacon template updates */
	uint32 cnt_bcn_tx; /* count of sync & disc bcn tx */
	uint32 cnt_bcn_rx; /* count of sync & disc bcn rx */
	uint32 cnt_sync_bcn_tx; /* count of sync bcn tx within DW */
	uint32 cnt_disc_bcn_tx; /* count of disc bcn tx */
	uint32 cnt_sdftx_bcmc; /* count of bcast/mcast sdf tx */
	uint32 cnt_sdftx_uc; /* count of unicast sdf tx */
	uint32 cnt_sdftx_fail; /* count of unicast sdf tx fails */
	uint32 cnt_sdf_rx; /* count of  sdf rx */
	/* NAN roles */
	uint32 cnt_am; /* anchor master */
	uint32 cnt_master; /* master */
	uint32 cnt_nms; /* non master sync */
	uint32 cnt_nmns; /* non master non sync */
	/* TX */
	uint32 cnt_err_txtime; /* txtime in sync bcn frame not a multiple of dw intv */
	uint32 cnt_err_unsch_tx; /* tx while not in DW/ disc bcn slot */
	uint32 cnt_err_bcn_tx; /*  beacon tx error */
	uint32 cnt_sync_bcn_tx_miss; /* no. of times time delta between 2 cosequetive
						* sync beacons is more than expected
						*/
	/* MSCH */
	uint32 cnt_err_msch_reg; /* error is Dw/disc reg with msch */
	uint32 cnt_err_wrong_ch_cb; /* count of msch calbacks in wrong channel */
	uint32 cnt_dw_skip;	/* count of DW rejected */
	uint32 cnt_disc_skip; /* count of disc bcn rejected */
	uint32 cnt_dw_start_early; /* msch cb not at registered time */
	uint32 cnt_dw_start_late; /* no. of delays in slot start */
	/* SCANS */
	uint32 cnt_mrg_scan; /* count of merge scans completed */
	uint32 cnt_err_ms_rej; /* number of merge scan failed */
	uint32 cnt_scan_results; /* no. of nan beacons scanned */
	uint32 cnt_join_scan_rej; /* no. of join scans rejected */
	uint32 cnt_nan_scan_abort; /* no. of join scans rejected */
	/* enable/disable */
	uint32 cnt_nan_enab; /* no. of times nan feature got enabled */
	uint32 cnt_nan_disab; /* no. of times nan feature got disabled */
	uint32 cnt_sync_bcn_rx; /* count of sync bcn rx within DW */
} wl_nan_stats_t;

#define WL_NAN_MAC_MAX_NAN_PEERS 6
#define WL_NAN_MAC_MAX_RSSI_DATA_PER_PEER  10

typedef struct wl_nan_nbr_rssi {
	uint8 rx_chan; /* channel number on which bcn rcvd */
	uint8 PAD[3];
	int32 rssi_raw;  /* received rssi value */
	int32 rssi_avg;  /* normalized rssi value */
} wl_nan_peer_rssi_t;

typedef struct wl_nan_peer_rssi_entry {
	struct ether_addr mac;  /* peer mac address */
	uint8 flags;   /* TODO:rssi data order: latest first, oldest first etc */
	uint8 rssi_cnt;   /* rssi data sample present */
	wl_nan_peer_rssi_t rssi[WL_NAN_MAC_MAX_RSSI_DATA_PER_PEER]; /* RSSI data frm peer */
} wl_nan_peer_rssi_entry_t;

#define WL_NAN_PEER_RSSI      0x1
#define WL_NAN_PEER_RSSI_LIST 0x2

typedef struct wl_nan_nbr_rssi_data {
	uint8 flags;   /* this is a list or single rssi data */
	uint8 peer_cnt; /* number of peers */
	uint16 pad; /* padding */
	wl_nan_peer_rssi_entry_t peers[1]; /* peers data list */
} wl_nan_peer_rssi_data_t;

/* WL_NAN_CMD_DBG_DUMP, GET Resp */
typedef struct wl_nan_dbg_dump_rsp {
	wl_nan_dbg_dump_type_t dump_type; /* dump data type */
	uint8 pad[3];
	union {
		wl_nan_peer_rssi_data_t peer_rssi;
		wl_nan_stats_t		nan_stats;
	} u;
} wl_nan_dbg_dump_rsp_t;

enum nan_termination_status {
	NAN_TERM_REASON_INVALID = 1,
	NAN_TERM_REASON_TIMEOUT = 2,
	NAN_TERM_REASON_USER_REQ = 3,
	NAN_TERM_REASON_FAILURE = 4,
	NAN_TERM_REASON_COUNT_REACHED = 5,
	NAN_TERM_REASON_DE_SHUTDOWN = 6,
	NAN_TERM_REASON_DISABLE_IN_PROGRESS = 7
};

/* nan2 data iovar */
/* nan2 qos */
typedef struct wl_nan_dp_qos
{
	uint8 tid;
	uint8 pad;
	uint16 pkt_size;
	uint16 mean_rate;
	uint16 svc_interval;
} wl_nan_dp_qos_t;

#define WL_NAN_NDL_QOS_MAX_LAT_NO_PREF 0xFFFF

/* nan2 qos */
typedef struct wl_nan_ndl_qos
{
	uint8 min_slots;	/* min slots per dw interval */
	uint8 pad;
	uint16 max_latency;	/* max latency */
} wl_nan_ndl_qos_t;

/* ndp config */
typedef struct wl_nan_ndp_config
{
	uint8 ndp_id;
	uint8 pub_id;
	struct ether_addr pub_addr;
	struct ether_addr data_addr;	/* configure local data addr */
	struct ether_addr init_data_addr;	/* initiator data addr */
	uint8 svc_spec_info[WL_NAN_DATA_SVC_SPEC_INFO_LEN];
	wl_nan_dp_qos_t qos;
	uint16 avail_len;
	uint8 pad[3];
	uint8 data[1];
} wl_nan_ndp_config_t;

/* nan2 device capabilities */
typedef struct wl_nan_ndp_oper_cfg {
	uint8 awake_dw_2g;
	uint8 awake_dw_5g;
	uint8 bands_supported;
	uint8 op_mode;
} wl_nan_ndp_oper_cfg_t;

typedef uint8 wl_nan_ndp_ndpid_t;
typedef uint8 wl_nan_ndp_conn_t;

typedef struct wl_nan_dp_req {
	uint8 type;		    /* 0- unicast 1 - multicast */
	uint8 pub_id;		    /* Publisher ID */
	uint16 flags;
	struct ether_addr peer_mac; /* Peer's NMI addr */
	struct ether_addr mcast_mac; /* Multicast addr */
	struct ether_addr ndi;
	wl_nan_dp_qos_t qos;
	wl_nan_ndl_qos_t ndl_qos;	/* ndl qos */
	uint8 tlv_params[];	/* xtlv parameters for command */
} wl_nan_dp_req_t;

/* TODO  Need to replace ndp_id with lndp_id */
/* Return structure to data req IOVAR */
typedef struct wl_nan_dp_req_ret {
	struct ether_addr indi;	    /* Initiators data mac addr */
	uint8 ndp_id;		    /* Initiators ndpid */
	uint8 pad;
} wl_nan_dp_req_ret_t;

typedef struct wl_nan_dp_resp {
	uint8 type;		    /* 0- unicast 1 - multicast */
	uint8 status;		    /* Accepted or Rejected */
	uint8 reason_code;
	/* Local NDP ID for unicast, mc_id for multicast, 0 for implicit NMSG */
	uint8 ndp_id;
	wl_nan_dp_qos_t qos;
	/* Initiator data address for unicast or multicast address for multicast */
	struct ether_addr mac_addr;
	struct ether_addr ndi;
	uint16 flags;
	wl_nan_ndl_qos_t ndl_qos;	/* ndl qos */
	uint8 tlv_params[];	/* xtlv parameters for command */
} wl_nan_dp_resp_t;

/* Return structure to data resp IOVAR */
typedef struct wl_nan_dp_resp_ret {
	uint8 nmsgid;		    /* NMSG ID or for multicast else 0 */
	uint8 pad[3];
} wl_nan_dp_resp_ret_t;

typedef struct wl_nan_dp_conf {
	uint8 lndp_id;
	uint8 status;		    /* Accepted or Rejected */
	uint8 pad[2];
} wl_nan_dp_conf_t;

typedef struct wl_nan_dp_end
{
	uint8 lndp_id;
	uint8 status;
	uint8 pad[2];
} wl_nan_dp_end_t;

typedef struct wl_nan_dp_schedupd {
	uint8 type;		/* 0: unicast, 1: multicast */
	uint8 flags;
	struct ether_addr addr;	/* peer NMI or multicast addr */
	wl_nan_dp_qos_t qos;
	wl_nan_ndl_qos_t ndl_qos;	/* ndl qos */
	uint8 map_id;
	uint8 pad[3];
} wl_nan_dp_schedupd_t;

/* set: update with notification, unset: NDL setup handshake */
#define WL_NAN_DP_SCHEDUPD_NOTIF (1 << 0)

/* list ndp ids */
typedef struct wl_nan_ndp_id_list {
	uint16 ndp_count;
	uint8 lndp_id[];
} wl_nan_ndp_id_list_t;

/* nan2 status */
typedef struct ndp_session {
	uint8 lndp_id;
	uint8 state;
	uint8 pub_id;
	uint8 pad;
} ndp_session_t;

typedef struct wl_nan_ndp_status {
	struct ether_addr peer_nmi;
	struct ether_addr peer_ndi;
	ndp_session_t session;
	struct ether_addr lndi;
	uint8 pad[2];
} wl_nan_ndp_status_t;

/* events */
#define NAN_DP_SESSION_UNICAST         0
#define NAN_DP_SESSION_MULTICAST       1
#define NAN_DP_SECURITY_NONE           0
#define NAN_DP_SECURITY_CSID           1
#define NAN_DP_SECURITY_MK             2
#define WL_NAN_DATA_NMSGID_LEN    8 /* 8 bytes as per nan spec */

/* Common event structure for Nan Datapath
 * Used for sending NDP Indication, Response, Confirmation, Securty Install and Establish events
 */
typedef struct wl_nan_ev_datapath_cmn {
	uint8 type;
	/* ndp_id is valid only if type is unicast */
	uint8 ndp_id;
	uint8 pub_id;
	uint8 security;
	/* Following two fields are valid only if type is unicast */
	struct ether_addr initiator_ndi;
	struct ether_addr responder_ndi;
	struct ether_addr peer_nmi;
	uint8 status;
	uint8 role;
	/* Following two fields are valid only if type is multicast */
	uint8 nmsg_id[WL_NAN_DATA_NMSGID_LEN];
	uint8 mc_id;
	uint8 pad[1];
	uint16 opt_tlv_len;
	uint8 opt_tlvs[];
} wl_nan_ev_datapath_cmn_t;

typedef struct wl_nan_ev_datapath_end {
	uint8 ndp_id;
	uint8 status;
	uint8 pad[2];
	struct ether_addr peer_nmi;
	struct ether_addr peer_ndi;
} wl_nan_ev_datapath_end_t;

/* NAN2.0 Ranging definitions */

/* result indication bit map */
#define NAN_RANGE_INDICATION_CONT		(1<<0)
#define NAN_RANGE_INDICATION_INGRESS		(1<<1)
#define NAN_RANGE_INIDICATION_EGRESS		(1<<2)

/* responder flags */
#define NAN_RANGE_FLAG_AUTO_ACCEPT	(1 << 0)
#define NAN_RANGE_FLAG_RESULT_REQUIRED	(1 << 1)

typedef struct wl_nan_range_req {
	struct ether_addr peer;
	uint8 publisher_id;
	uint8 indication; /* bit map for result event */
	uint32 resolution; /* default millimeters */
	uint32 ingress; /* ingress limit in mm */
	uint32 egress; /* egress limit in mm */
	uint32 interval; /* max interval(in TU) b/w two ranging measurements */
} wl_nan_range_req_t;

#define NAN_RNG_REQ_IOV_LEN	24

typedef uint8 wl_nan_range_id;

typedef struct wl_nan_range_resp {
	wl_nan_range_id range_id;
	uint8 flags; /* auto response, range result required */
	uint8 status; /* accept, reject */
	uint8 indication; /* bit map for result event */
	uint32 resolution; /* default millimeters */
	uint32 ingress; /* ingress limit in mm */
	uint32 egress; /* egress limit in mm */
	uint32 interval; /* max interval(in TU) b/w two ranging measurements */
} wl_nan_range_resp_t;

#define NAN_RNG_RESP_IOV_LEN	20

#define NAN_RNG_MAX_IOV_LEN	255

typedef struct wl_nan_ev_rng_req_ind {
	struct ether_addr peer_m_addr;
	uint8 rng_id;
	/* ftm parameters */
	uint8 max_burst_dur;
	uint8 min_ftm_delta;
	uint8 max_num_ftm;
	uint8 ftm_format_bw;
	/* location info availability bit map */
	uint8 lc_info_avail;
	/* Last movement indication */
	uint16 last_movement;
	uint8 pad[2];
} wl_nan_ev_rng_req_ind_t;

#define NAN_RNG_REQ_IND_SIZE 14

typedef struct wl_nan_ev_rng_rpt_ind {
	uint32 dist_mm; /* in millimeter */
	struct ether_addr peer_m_addr;
	uint8 indication; /* indication definitions mentioned above */
	uint8 pad;
} wl_nan_ev_rng_rpt_ind_t;

#define NAN_RNG_RPT_IND_SIZE 11

typedef struct wl_nan_ev_rng_term_ind {
	struct ether_addr peer_m_addr;
	uint8 reason_code;
	uint8 pad;
} wl_nan_ev_rng_term_ind_t;

#define NAN_RNG_TERM_IND_SIZE 7

/* ********************* end of NAN section ******************************** */
/* endif WL_NAN */

#define P2P_NAN_IOC_BUFSZ  512 /* some sufficient ioc buff size */
#define WL_P2P_NAN_IOCTL_VERSION    0x1

/* container for p2p nan iovtls & events */
typedef struct wl_p2p_nan_ioc {
	uint16  version;    /* interface command or event version */
	uint16  id;     /* p2p nan ioctl cmd  ID  */
	uint16  len;        /* total length of data[]  */
	uint16  pad;        /* padding */
	uint8   data [];   /* var len payload of bcm_xtlv_t type */
} wl_p2p_nan_ioc_t;

/* p2p nan cmd IDs */
enum wl_p2p_nan_cmds {
	/* p2p nan cfg ioctls */
	WL_P2P_NAN_CMD_ENABLE = 1,
	WL_P2P_NAN_CMD_CONFIG = 2,
	WL_P2P_NAN_CMD_DEL_CONFIG = 3,
	WL_P2P_NAN_CMD_GET_INSTS = 4
};

#define WL_P2P_NAN_CONFIG_VERSION       1

#define WL_P2P_NAN_DEVICE_P2P  0x0
#define WL_P2P_NAN_DEVICE_GO   0x1
#define WL_P2P_NAN_DEVICE_GC   0x2
#define WL_P2P_NAN_DEVICE_INVAL   0xFF

/* NAN P2P operation */
typedef struct p2p_nan_config {
	uint16 version;            /* wl_p2p_nan_config_t structure version */
	uint16 len;                /* total length including version and variable IE */
	uint32 flags;              /* 0x1 to NEW, 0x2 to ADD, 0x4 to DEL */
	uint8  inst_id;            /* publisher/subscriber id */
	uint8  inst_type;          /* publisher/subscriber */
	uint8  dev_role;           /* P2P device role: 'P2P','GO' or 'GC' */
	uint8  pad1;               /* padding */
	uint8  resolution;         /* Availability bitmap resolution */
	uint8  repeat;             /* Whether Availabilty repeat across DW */
	uint16 ie_len;             /* variable ie len */
	struct ether_addr dev_mac; /* P2P device addres */
	uint16 pad2;               /* Padding */
	uint32 avail_bmap;         /* availability interval bitmap */
	uint32 chanspec;           /* Chanspec */
	uint8  ie[];              /* hex ie data */
} wl_p2p_nan_config_t;

#define WL_P2P_NAN_SERVICE_LIST_VERSION 1
typedef enum wl_nan_service_type {
	WL_NAN_SVC_INST_PUBLISHER = 1,
	WL_NAN_SVC_INST_SUBSCRIBER = 2
} wl_nan_service_type_t;

#define WL_P2P_NAN_CONFIG_NEW   0x1
#define WL_P2P_NAN_CONFIG_ADD   0x2
#define WL_P2P_NAN_CONFIG_DEL   0x4

typedef struct wl_nan_svc_inst {
	uint8  inst_id;      /* publisher/subscriber id */
	uint8  inst_type;    /* publisher/subscriber */
} wl_nan_svc_inst_t;

typedef struct wl_nan_svc_inst_list {
	uint16 version;           /* this structure version */
	uint16 len;               /* total length including version and variable svc list */
	uint16 count;             /* service instance count */
	uint16 pad;               /* padding */
	wl_nan_svc_inst_t svc[1]; /* service instance list */
} wl_nan_svc_inst_list_t;

#define NAN_POST_DISC_P2P_DATA_VER  1
/* This structure will be used send peer p2p data with
 * NAN discovery result
 */
typedef struct nan_post_disc_p2p_data {
	uint8 ver;                 /* this structure version */
	uint8 dev_role;            /* P2P Device role */
	uint8 resolution;          /* Availability bitmap resolution */
	uint8 repeat;              /* Whether Availabilty repeat across DW */
	struct ether_addr dev_mac; /* P2P device addres */
	uint16 pad1;               /* Padding */
	uint32 chanspec;           /* Chanspec */
	uint32 avl_bmp;				/* availability interval bitmap */
} nan_post_disc_p2p_data_t;

/* timeslot etc for NAN */
enum {
	WL_TMU_TU            = 0,
	WL_TMU_SEC           = 1,
	WL_TMU_MILLI_SEC     = 2,
	WL_TMU_MICRO_SEC     = 3,
	WL_TMU_NANO_SEC      = 4,
	WL_TMU_PICO_SEC      = 5
};
typedef int16 wl_tmu_t;

typedef struct {
	uint32   intvl;               /* time interval */
	wl_tmu_t tmu;                 /* time unit */
	uint8    pad[2];              /* padding */
} wl_time_interval_t;

/* availabiloty slot flags */
enum {
	WL_AVAIL_SLOT_NONE		= 0x0000,
	WL_AVAIL_SLOT_COM		= 0x0001,		/* committed */
	WL_AVAIL_SLOT_POT		= 0x0002,		/* potential */
	WL_AVAIL_SLOT_PROP		= 0x0004,	/* proposed -  note: not configurable */
	WL_AVAIL_SLOT_PAGED		= 0x0008	/* P-NDL */
	/* 0x0030 - resrved for NDC index */
	/* 0x00c0 - resrved for usage preference */
};
typedef int16 wl_avail_slot_flags_t;

#define WL_AVAIL_SLOT_NDC_MASK 0x0030 /* up to 4 NDCs */
#define WL_AVAIL_SLOT_NDC_SHIFT 4
#define WL_AVAIL_SLOT_NDC(_flags)  (((_flags) & WL_AVAIL_SLOT_NDC_MASK) \
	 >> WL_AVAIL_SLOT_NDC_SHIFT)
#define WL_AVAIL_SLOT_SET_NDC(_flags, _ndc_idx) (((_flags) & ~WL_AVAIL_SLOT_NDC_MASK) |\
	((_ndc_idx) << WL_AVAIL_SLOT_NDC_SHIFT))

#define WL_AVAIL_SLOT_UPREF_MASK 0x00c0 /* up to 4 usage preferences */
#define WL_AVAIL_SLOT_UPREF_SHIFT 6
#define WL_AVAIL_SLOT_UPREF(_flags)  (((_flags) & WL_AVAIL_SLOT_UPREF_MASK) \
	 >> WL_AVAIL_SLOT_UPREF_SHIFT)
#define WL_AVAIL_SLOT_SET_UPREF(_flags, _pref) (((_flags) & ~WL_AVAIL_SLOT_UPREF_MASK) |\
	((_pref) << WL_AVAIL_SLOT_UPREF_SHIFT))

typedef struct wl_avail_slot {
	wl_avail_slot_flags_t flags;
	uint16                PAD;
	wl_time_interval_t    start;        /* from time ref */
	wl_time_interval_t    duration;     /* from start */
	uint32                chanspec;     /* channel spec */
} wl_avail_slot_t;

/* time reference */
enum {
	WL_TIME_REF_NONE        = 0,
	WL_TIME_REF_DEV_TSF     = 1,
	WL_TIME_REF_NAN_DW      = 2,
	WL_TIME_REF_TBTT        = 3,
	WL_TIME_REF_NAN_DW0     = 4
};
typedef int16 wl_time_ref_t;

enum {
	WL_AVAIL_NONE		= 0x0000,
	WL_AVAIL_LOCAL		= 0x0001,
	WL_AVAIL_PEER		= 0x0002,
	WL_AVAIL_NDC		= 0x0003,
	WL_AVAIL_IMMUTABLE	= 0x0004,
	WL_AVAIL_RESPONSE	= 0x0005,
	WL_AVAIL_COUNTER	= 0x0006,
	WL_AVAIL_RANGING	= 0x0007,
	WL_AVAIL_TYPE_MAX	= WL_AVAIL_RANGING	/* New ones before and update */
};
#define WL_AVAIL_TYPE_MASK	0x000F
#define WL_AVAIL_FLAG_RAW_MODE	0x8000
typedef int16 wl_avail_flags_t;

/* availability entry flags */
enum {
	WL_AVAIL_ENTRY_NONE		= 0x0000,
	WL_AVAIL_ENTRY_COM		= 0x0001,		/* committed */
	WL_AVAIL_ENTRY_POT		= 0x0002,		/* potential */
	WL_AVAIL_ENTRY_COND		= 0x0004,	/* conditional */
	WL_AVAIL_ENTRY_PAGED		= 0x0008,	/* P-NDL */
	WL_AVAIL_ENTRY_USAGE		= 0x0030,	/* usage preference */
	WL_AVAIL_ENTRY_BIT_DUR		= 0x00c0,	/* bit duration */
	WL_AVAIL_ENTRY_BAND_PRESENT	= 0x0100,	/* band present */
	WL_AVAIL_ENTRY_CHAN_PRESENT	= 0x0200,	/* channel information present */
	WL_AVAIL_ENTRY_CHAN_ENTRY_PRESENT	= 0x0400,	/* channel entry (opclass+bitmap) */
};

/* bit duration */
enum {
	WL_AVAIL_BIT_DUR_16	= 0,	/* 16TU */
	WL_AVAIL_BIT_DUR_32	= 1,	/* 32TU */
	WL_AVAIL_BIT_DUR_64	= 2,	/* 64TU */
	WL_AVAIL_BIT_DUR_128	= 3,	/* 128TU */
};

/* period */
enum {
	WL_AVAIL_PERIOD_0	= 0,	/* 0TU */
	WL_AVAIL_PERIOD_128	= 1,	/* 128TU */
	WL_AVAIL_PERIOD_256	= 2,	/* 256TU */
	WL_AVAIL_PERIOD_512	= 3,	/* 512TU */
	WL_AVAIL_PERIOD_1024	= 4,	/* 1024TU */
	WL_AVAIL_PERIOD_2048	= 5,	/* 2048TU */
	WL_AVAIL_PERIOD_4096	= 6,	/* 4096TU */
	WL_AVAIL_PERIOD_8192	= 7,	/* 8192TU */
};

/* band */
enum {
	WL_AVAIL_BAND_NONE	= 0,	/* reserved */
	WL_AVAIL_BAND_SUB1G	= 1,	/* sub-1 GHz */
	WL_AVAIL_BAND_2G	= 2,	/* 2.4 GHz */
	WL_AVAIL_BAND_3G	= 3,	/* reserved (for 3.6 GHz) */
	WL_AVAIL_BAND_5G	= 4,	/* 4.9 and 5 GHz */
	WL_AVAIL_BAND_60G	= 5,	/* reserved (for 60 GHz) */
};

#define WL_AVAIL_ENTRY_TYPE_MASK 0x0F
#define WL_AVAIL_ENTRY_USAGE_MASK 0x0030 /* up to 4 usage preferences */
#define WL_AVAIL_ENTRY_USAGE_SHIFT 4
#define WL_AVAIL_ENTRY_USAGE_VAL(_flags)  (((_flags) & WL_AVAIL_ENTRY_USAGE_MASK) \
	>> WL_AVAIL_ENTRY_USAGE_SHIFT)

#define WL_AVAIL_ENTRY_BIT_DUR_MASK 0x00c0 /* 0:16TU, 1:32TU, 2:64TU, 3:128TU */
#define WL_AVAIL_ENTRY_BIT_DUR_SHIFT 6
#define WL_AVAIL_ENTRY_BIT_DUR_VAL(_flags)  (((_flags) & WL_AVAIL_ENTRY_BIT_DUR_MASK) \
	>> WL_AVAIL_ENTRY_BIT_DUR_SHIFT)

#define WL_AVAIL_ENTRY_BAND_MASK 0x0100 /* 0=band not present, 1=present */
#define WL_AVAIL_ENTRY_BAND_SHIFT 8

#define WL_AVAIL_ENTRY_CHAN_MASK 0x0200 /* 0=channel info not present, 1=present */
#define WL_AVAIL_ENTRY_CHAN_SHIFT 9

#define WL_AVAIL_ENTRY_CHAN_ENTRY_MASK 0x0400 /* 0=chanspec, 1=hex channel entry */
#define WL_AVAIL_ENTRY_CHAN_ENTRY_SHIFT 10

#define WL_AVAIL_ENTRY_OPCLASS_MASK 0xFF
#define WL_AVAIL_ENTRY_CHAN_BITMAP_MASK 0xFF00
#define WL_AVAIL_ENTRY_CHAN_BITMAP_SHIFT 8
#define WL_AVAIL_ENTRY_CHAN_BITMAP_VAL(_info) (((_info) & WL_AVAIL_ENTRY_CHAN_BITMAP_MASK) \
	>> WL_AVAIL_ENTRY_CHAN_BITMAP_SHIFT)

/* Used for raw channel entry field input */
#define	MAX_CHAN_ENTRY_LEN 6

typedef struct wl_avail_entry {
	uint16		length;		/* total length */
	uint16		start_offset;	/* in TUs, multiply by 16 for total offset */
	union {
		uint32 channel_info;	/* either chanspec or hex channel entry (opclass +
					 * bitmap per NAN spec), as indicated by setting
					 * WL_AVAIL_ENTRY_HEX_CHAN_ENTRY flag
					 */
		uint32 band;		/* defined by WL_BAND enum, 2=2.4GHz, 4=5GHz */
		uint8	channel_entry[MAX_CHAN_ENTRY_LEN];
	} u;				/* band or channel value, 0=all band/channels */
	uint8		pad[2];
	uint8		period;		/* in TUs, defined by WL_AVAIL_PERIOD enum
					 * 1:128, 2:256, 3:512, 4:1024, 5:2048, 6:4096,
					 * 7:8192
					 */
	uint8		bitmap_len;
	uint16		flags;		/* defined by avail entry flags enum:
					 * type, usage pref, bit duration, band, channel
					 */
	uint8		bitmap[];	/* time bitmap */
} wl_avail_entry_t;

typedef struct wl_avail {
	uint16		length;		/* total length */
	uint16		flags;		/* defined by WL_AVAIL enum
					 * 1=local, 2=peer, 3=ndc, 4=immutable,
					 * 5=response, 6=counter
					 */
	uint8		id;		/* id used for multiple maps/avail */
	uint8		pad[3];
	struct ether_addr addr;	/* peer mac address or ndc id */
	uint8		num_entries;
	uint8		entry_offset;
	/* add additional fields above this line */
	uint8		entry[];
} wl_avail_t;

#define WL_AVAIL_MIN_LEN(n) ((n) ? OFFSETOF(wl_avail_t, entry) + \
		((n) * OFFSETOF(wl_avail_entry_t, bitmap)) : 0)

/* unaligned schedule (window) */
typedef struct wl_avail_ulw {
	uint8	id;		/* schedule ID */
	uint8	overwrite;	/* bit 0: overwrite all
				 * 1-4: map ID if overwrite all is 0
				 */
	uint16	flags;
	uint32	start;		/* start time of first ULW, in us */
	uint32	dur;		/* duration of ULW, in us */
	uint32	period;		/* time between consecutive ULWs, in us */
	union {
		uint32 chanspec;
		uint32 band;
		uint8 chan_entry[MAX_CHAN_ENTRY_LEN];
		uint8 pad[8];
	} u;
	uint8	cntdwn;		/* remaining ULWs before schedule ends */
	uint8	pad[3];
} wl_avail_ulw_t;

/* unset: NAN is not available during ULW, set: NAN is avail depending on ctrl flags */
#define WL_NAN_ULW_CTRL_PRESENT		(1 << 0)
/* unset: band, set: channel */
#define WL_NAN_ULW_CTRL_TYPE		(1 << 1)
/* set: NAN is availabile on specified band/channel */
#define WL_NAN_ULW_CTRL_AVAIL		(1 << 2)
/* channel is provided in raw attribute format */
#define WL_NAN_ULW_CTRL_RAW_CHAN	(1 << 3)

/* nan wfa testmode operations */
enum {
	WL_NAN_WFA_TM_IGNORE_TERMINATE_NAF		= 0x00000001,
	WL_NAN_WFA_TM_IGNORE_RX_DATA_OUTSIDE_CRB	= 0x00000002,
	WL_NAN_WFA_TM_ALLOW_TX_DATA_OUTSIDE_CRB		= 0x00000004,
	WL_NAN_WFA_TM_ENFORCE_NDL_COUNTER		= 0x00000008,
	WL_NAN_WFA_TM_BYPASS_NDL_PROPOSAL_VALIDATION	= 0x00000010,
	/* allow data(pings) tx while ndp sec negotiation */
	WL_NAN_WFA_TM_SEC_SEND_PINGS_BYPASS_NDP_SM	= 0x00000020,
	/* generate and insert incorrect mic */
	WL_NAN_WFA_TM_SEC_INCORRECT_MIC		        = 0x00000040,
	/* send m4 reject deliberately */
	WL_NAN_WFA_TM_SEC_REJECT_STATUS4M4		= 0x00000080,
	/* send mgmt frame (for eg. ndp terminate) in clear txt (bypass security) */
	WL_NAN_WFA_TM_SEC_SEND_MGMT_CLEAR	        = 0x00000100,
	/* validate qos */
	WL_NAN_WFA_TM_NDL_QOS_VALIDATE			= 0x00000200,
	/* firmware generated schedule update */
	WL_NAN_WFA_TM_GEN_SCHED_UPD			= 0x00000400,
	/* add lower 4-bytes of TSF to configured start time */
	WL_NAN_WFA_TM_ULW_START_TIME			= 0x00000800,
	/* enable schedule validation for SDF */
	WL_NAN_WFA_TM_SDF_SCHED_VALIDATE		= 0x00001000,
	/* by pass faw na iovar */
	WL_NAN_WFA_TM_SKIP_RAW_NA_BLOB		        = 0x00002000,
	/* overwrite local NA with peer NA in received frame */
	WL_NAN_WFA_TM_LOCAL_NA_OVERWRITE		= 0x00004000,
	WL_NAN_WFA_TM_FLAG_MASK				= 0x00007fff /* add above & update mask */
};
typedef uint32 wl_nan_wfa_testmode_t;

#define RSSI_THRESHOLD_SIZE 16
#define MAX_IMP_RESP_SIZE 256

typedef struct wl_proxd_rssi_bias {
	int32		version;			/**< version */
	int32		threshold[RSSI_THRESHOLD_SIZE];	/**< threshold */
	int32		peak_offset;			/**< peak offset */
	int32		bias;				/**< rssi bias */
	int32		gd_delta;			/**< GD - GD_ADJ */
	int32		imp_resp[MAX_IMP_RESP_SIZE];	/**< (Hi*Hi)+(Hr*Hr) */
} wl_proxd_rssi_bias_t;

typedef struct wl_proxd_rssi_bias_avg {
	int32		avg_threshold[RSSI_THRESHOLD_SIZE];	/**< avg threshold */
	int32		avg_peak_offset;			/**< avg peak offset */
	int32		avg_rssi;				/**< avg rssi */
	int32		avg_bias;				/**< avg bias */
} wl_proxd_rssi_bias_avg_t;

#include <packed_section_start.h>
typedef BWL_PRE_PACKED_STRUCT struct wl_proxd_collect_info {
	uint16		type;  /**< type: 0 channel table, 1 channel smoothing table, 2 and 3 seq */
	uint16		index;		/**< The current frame index, from 1 to total_frames. */
	uint16		tof_cmd;	/**< M_TOF_CMD      */
	uint16		tof_rsp;	/**< M_TOF_RSP      */
	uint16		tof_avb_rxl;	/**< M_TOF_AVB_RX_L */
	uint16		tof_avb_rxh;	/**< M_TOF_AVB_RX_H */
	uint16		tof_avb_txl;	/**< M_TOF_AVB_TX_L */
	uint16		tof_avb_txh;	/**< M_TOF_AVB_TX_H */
	uint16		tof_id;		/**< M_TOF_ID */
	uint8		tof_frame_type;
	uint8		tof_frame_bw;
	int8		tof_rssi;
	int32		tof_cfo;
	int32		gd_adj_ns;	/**< gound delay */
	int32		gd_h_adj_ns;	/**< group delay + threshold crossing */
	int16		nfft;		/**< number of samples stored in H */
	uint8		num_max_cores;

} BWL_POST_PACKED_STRUCT wl_proxd_collect_info_t;
#include <packed_section_end.h>

#define K_TOF_COLLECT_H_PAD 1
#define K_TOF_COLLECT_SC_20MHZ (64)
/* Maximum possible size of sample capture */
#define K_TOF_COLLECT_SC_80MHZ (2*K_TOF_COLLECT_SC_20MHZ)
/* Maximum possible size of channel dump */
#define K_TOF_COLLECT_CHAN_SIZE (2*K_TOF_COLLECT_SC_80MHZ)

/*
A few extra samples are required to estimate frequency offset
Right now 16 samples are being used. Can be changed in future.
*/
#define K_TOF_COLLECT_SAMP_SIZE_20MHZ (2*(K_TOF_COLLECT_SC_20MHZ)+16+K_TOF_COLLECT_H_PAD)
#define K_TOF_COLLECT_RAW_SAMP_SIZE_20MHZ (2*K_TOF_COLLECT_SAMP_SIZE_20MHZ)
#define K_TOF_COLLECT_H_SIZE_20MHZ (K_TOF_COLLECT_SAMP_SIZE_20MHZ)
#define K_TOF_COLLECT_HRAW_SIZE_20MHZ (K_TOF_COLLECT_RAW_SAMP_SIZE_20MHZ)

#define K_TOF_COLLECT_SAMP_SIZE_80MHZ (2*(K_TOF_COLLECT_SC_80MHZ)+16+K_TOF_COLLECT_H_PAD)
#define K_TOF_COLLECT_RAW_SAMP_SIZE_80MHZ (2*K_TOF_COLLECT_SAMP_SIZE_80MHZ)
#define K_TOF_COLLECT_H_SIZE_80MHZ (K_TOF_COLLECT_SAMP_SIZE_80MHZ)
#define K_TOF_COLLECT_HRAW_SIZE_80MHZ (K_TOF_COLLECT_RAW_SAMP_SIZE_80MHZ)

#define WL_PROXD_COLLECT_DATA_VERSION_1		1
#include <packed_section_start.h>
typedef BWL_PRE_PACKED_STRUCT struct wl_proxd_collect_data_v1 {
	wl_proxd_collect_info_t  info;
	uint8	ri_rr[FTM_TPK_RI_RR_LEN];
	/**< raw data read from phy used to adjust timestamps */
	uint32	H[K_TOF_COLLECT_H_SIZE_20MHZ];
} BWL_POST_PACKED_STRUCT wl_proxd_collect_data_t_v1;
#include <packed_section_end.h>

#define WL_PROXD_COLLECT_DATA_VERSION_2		2
#include <packed_section_start.h>
typedef BWL_PRE_PACKED_STRUCT struct wl_proxd_collect_data_v2 {
	wl_proxd_collect_info_t  info;
	uint8   ri_rr[FTM_TPK_RI_RR_LEN_SECURE_2_0];
	/**< raw data read from phy used to adjust timestamps */
	uint32  H[K_TOF_COLLECT_H_SIZE_20MHZ];
} BWL_POST_PACKED_STRUCT wl_proxd_collect_data_t_v2;
#include <packed_section_end.h>

#define WL_PROXD_COLLECT_DATA_VERSION_3         3
typedef struct wl_proxd_collect_data_v3 {
	uint16			version;
	uint16			len;
	wl_proxd_collect_info_t	info;
	uint8			ri_rr[FTM_TPK_RI_RR_LEN_SECURE_2_0];
	/**< raw data read from phy used to adjust timestamps */
	uint32			H[K_TOF_COLLECT_H_SIZE_20MHZ];
	uint32			chan[4 * K_TOF_COLLECT_CHAN_SIZE];
} wl_proxd_collect_data_t_v3;
#define WL_PROXD_COLLECT_DATA_VERSION_MAX	WL_PROXD_COLLECT_DATA_VERSION_3

typedef struct wl_proxd_debug_data {
	uint8		count;		/**< number of packets */
	uint8		stage;		/**< state machone stage */
	uint8		received;	/**< received or txed */
	uint8		paket_type;	/**< packet type */
	uint8		category;	/**< category field */
	uint8		action;		/**< action field */
	uint8		token;		/**< token number */
	uint8		follow_token;	/**< following token number */
	uint16		index;		/**< index of the packet */
	uint16		tof_cmd;	/**< M_TOF_CMD */
	uint16		tof_rsp;	/**< M_TOF_RSP */
	uint16		tof_avb_rxl;	/**< M_TOF_AVB_RX_L */
	uint16		tof_avb_rxh;	/**< M_TOF_AVB_RX_H */
	uint16		tof_avb_txl;	/**< M_TOF_AVB_TX_L */
	uint16		tof_avb_txh;	/**< M_TOF_AVB_TX_H */
	uint16		tof_id;		/**< M_TOF_ID */
	uint16		tof_status0;	/**< M_TOF_STATUS_0 */
	uint16		tof_status2;	/**< M_TOF_STATUS_2 */
	uint16		tof_chsm0;	/**< M_TOF_CHNSM_0 */
	uint16		tof_phyctl0;	/**< M_TOF_PHYCTL0 */
	uint16		tof_phyctl1;	/**< M_TOF_PHYCTL1 */
	uint16		tof_phyctl2;	/**< M_TOF_PHYCTL2 */
	uint16		tof_lsig;	/**< M_TOF_LSIG */
	uint16		tof_vhta0;	/**< M_TOF_VHTA0 */
	uint16		tof_vhta1;	/**< M_TOF_VHTA1 */
	uint16		tof_vhta2;	/**< M_TOF_VHTA2 */
	uint16		tof_vhtb0;	/**< M_TOF_VHTB0 */
	uint16		tof_vhtb1;	/**< M_TOF_VHTB1 */
	uint16		tof_apmductl;	/**< M_TOF_AMPDU_CTL */
	uint16		tof_apmdudlim;	/**< M_TOF_AMPDU_DLIM */
	uint16		tof_apmdulen;	/**< M_TOF_AMPDU_LEN */
} wl_proxd_debug_data_t;

/** version of the wl_wsec_info structure */
#define WL_WSEC_INFO_VERSION 0x01

/** start enum value for BSS properties */
#define WL_WSEC_INFO_BSS_BASE 0x0100

/** size of len and type fields of wl_wsec_info_tlv_t struct */
#define WL_WSEC_INFO_TLV_HDR_LEN OFFSETOF(wl_wsec_info_tlv_t, data)

/** Allowed wl_wsec_info properties; not all of them may be supported. */
typedef enum {
	WL_WSEC_INFO_NONE = 0,
	WL_WSEC_INFO_MAX_KEYS = 1,
	WL_WSEC_INFO_NUM_KEYS = 2,
	WL_WSEC_INFO_NUM_HW_KEYS = 3,
	WL_WSEC_INFO_MAX_KEY_IDX = 4,
	WL_WSEC_INFO_NUM_REPLAY_CNTRS = 5,
	WL_WSEC_INFO_SUPPORTED_ALGOS = 6,
	WL_WSEC_INFO_MAX_KEY_LEN = 7,
	WL_WSEC_INFO_FLAGS = 8,
	/* add global/per-wlc properties above */
	WL_WSEC_INFO_BSS_FLAGS = (WL_WSEC_INFO_BSS_BASE + 1),
	WL_WSEC_INFO_BSS_WSEC = (WL_WSEC_INFO_BSS_BASE + 2),
	WL_WSEC_INFO_BSS_TX_KEY_ID = (WL_WSEC_INFO_BSS_BASE + 3),
	WL_WSEC_INFO_BSS_ALGO = (WL_WSEC_INFO_BSS_BASE + 4),
	WL_WSEC_INFO_BSS_KEY_LEN = (WL_WSEC_INFO_BSS_BASE + 5),
	WL_WSEC_INFO_BSS_ALGOS = (WL_WSEC_INFO_BSS_BASE + 6),
	/* add per-BSS properties above */
	WL_WSEC_INFO_MAX = 0xffff
} wl_wsec_info_type_t;

typedef struct {
	uint32 algos; /* set algos to be enabled/disabled */
	uint32 mask; /* algos outside mask unaltered */
} wl_wsec_info_algos_t;

/** tlv used to return wl_wsec_info properties */
typedef struct {
	uint16 type;
	uint16 len;		/**< data length */
	uint8 data[1];	/**< data follows */
} wl_wsec_info_tlv_t;

/** input/output data type for wsec_info iovar */
typedef struct wl_wsec_info {
	uint8 version; /**< structure version */
	uint8 pad[2];
	uint8 num_tlvs;
	wl_wsec_info_tlv_t tlvs[1]; /**< tlv data follows */
} wl_wsec_info_t;

/*
 * randmac definitions
 */
#define WL_RANDMAC_MODULE			"randmac"
#define WL_RANDMAC_API_VERSION		0x0100 /**< version 1.0 */
#define WL_RANDMAC_API_MIN_VERSION	0x0100 /**< version 1.0 */

/** subcommands that can apply to randmac */
enum {
	WL_RANDMAC_SUBCMD_NONE				= 0,
	WL_RANDMAC_SUBCMD_GET_VERSION			= 1,
	WL_RANDMAC_SUBCMD_ENABLE			= 2,
	WL_RANDMAC_SUBCMD_DISABLE			= 3,
	WL_RANDMAC_SUBCMD_CONFIG			= 4,
	WL_RANDMAC_SUBCMD_STATS				= 5,
	WL_RANDMAC_SUBCMD_CLEAR_STATS			= 6,

	WL_RANDMAC_SUBCMD_MAX
};
typedef int16 wl_randmac_subcmd_t;

/* Common IOVAR struct */
typedef struct wl_randmac {
	uint16 version;
	uint16 len;			/* total length */
	wl_randmac_subcmd_t subcmd_id;	/* subcommand id */
	uint8 data[0];			/* subcommand data */
} wl_randmac_t;

#define WL_RANDMAC_IOV_HDR_SIZE OFFSETOF(wl_randmac_t, data)

/* randmac version subcommand */
typedef struct wl_randmac_version {
	uint16 version;  /* Randmac method version info */
	uint8 pad[2];    /* Align on 4 byte boundary */
} wl_randmac_version_t;

/*
 * Bitmask for methods supporting MAC randomization feature
 */
#define WL_RANDMAC_USER_NONE		0x0000
#define WL_RANDMAC_USER_FTM		0x0001
#define WL_RANDMAC_USER_NAN		0x0002
#define WL_RANDMAC_USER_SCAN		0x0004
#define WL_RANDMAC_USER_ALL		0xFFFF
typedef uint16 wl_randmac_method_t;

enum {
	WL_RANDMAC_FLAGS_NONE	= 0x00,
	WL_RANDMAC_FLAGS_ADDR	= 0x01,
	WL_RANDMAC_FLAGS_MASK	= 0x02,
	WL_RANDMAC_FLAGS_METHOD	= 0x04,
	WL_RANDMAC_FLAGS_ALL	= 0xFF
};
typedef uint8 wl_randmac_flags_t;

/* randmac statistics subcommand */
typedef struct wl_randmac_stats {
	uint32 set_ok;		/* Set random addr success count */
	uint32 set_fail;	/* Set random addr failed count */
	uint32 set_reqs;	/* Set random addr count */
	uint32 reset_reqs;	/* Restore random addr count */
	uint32 restore_ok;	/* Restore random addr succes count */
	uint32 restore_fail;	/* Restore random addr failed count */
	uint32 events_sent;	/* randmac module events count */
	uint32 events_rcvd;	/* randmac events received count */
} wl_randmac_stats_t;

/* randmac config subcommand */
typedef struct wl_randmac_config {
	struct ether_addr addr;			/* Randomized MAC address */
	struct ether_addr addr_mask;		/* bitmask for randomization */
	wl_randmac_method_t method;		/* Enabled methods */
	wl_randmac_flags_t flags;		/* What config info changed */
	uint8	PAD;
} wl_randmac_config_t;

enum {
	WL_RANDMAC_EVENT_NONE			= 0,	/**< not an event, reserved */
	WL_RANDMAC_EVENT_BSSCFG_ADDR_SET	= 1,	/* bsscfg addr randomized */
	WL_RANDMAC_EVENT_BSSCFG_ADDR_RESTORE	= 2,	/* bsscfg addr restored */
	WL_RANDMAC_EVENT_ENABLED		= 3,	/* randmac module enabled */
	WL_RANDMAC_EVENT_DISABLE		= 4,	/* randmac module disabled */
	WL_RANDMAC_EVENT_BSSCFG_STATUS	= 5,	/* bsscfg enable/disable */

	WL_RANDMAC_EVENT_MAX
};
typedef int16 wl_randmac_event_type_t;
typedef int32 wl_randmac_status_t;
typedef uint32 wl_randmac_event_mask_t;

#define WL_RANDMAC_EVENT_MASK_ALL 0xfffffffe
#define WL_RANDMAC_EVENT_MASK_EVENT(_event_type) (1 << (_event_type))
#define WL_RANDMAC_EVENT_ENABLED(_mask, _event_type) (\
	((_mask) & WL_RANDMAC_EVENT_MASK_EVENT(_event_type)) != 0)

/** tlv IDs - data length 4 bytes unless overridden by type, alignment 32 bits */
enum {
	WL_RANDMAC_TLV_NONE		= 0,
	WL_RANDMAC_TLV_METHOD		= 1,
	WL_RANDMAC_TLV_ADDR		= 2,
	WL_RANDMAC_TLV_MASK		= 3
};
typedef uint16 wl_randmac_tlv_id_t;

typedef struct wl_randmac_tlv {
	wl_randmac_tlv_id_t id;
	uint16 len;		/* Length of variable */
	uint8  data[1];
} wl_randmac_tlv_t;

/** randmac event */
typedef struct wl_randmac_event {
	uint16					version;
	uint16					len;	/* Length of all variables */
	wl_randmac_event_type_t			type;
	wl_randmac_method_t			method;
	uint8					pad[2];
	wl_randmac_tlv_t			tlvs[1];	/**< variable */
} wl_randmac_event_t;

#ifndef LINUX_POSTMOGRIFY_REMOVAL
/*
 * scan MAC definitions
 */

/** common iovar struct */
typedef struct wl_scanmac {
	uint16 subcmd_id;	/**< subcommand id */
	uint16 len;		/**< total length of data[] */
	uint8 data[];		/**< subcommand data */
} wl_scanmac_t;

/* subcommand ids */
#define WL_SCANMAC_SUBCMD_ENABLE   0
#define WL_SCANMAC_SUBCMD_BSSCFG   1   /**< only GET supported */
#define WL_SCANMAC_SUBCMD_CONFIG   2

/** scanmac enable data struct */
typedef struct wl_scanmac_enable {
	uint8 enable;	/**< 1 - enable, 0 - disable */
	uint8 pad[3];	/**< 4-byte struct alignment */
} wl_scanmac_enable_t;

/** scanmac bsscfg data struct */
typedef struct wl_scanmac_bsscfg {
	uint32 bsscfg;	/**< bsscfg index */
} wl_scanmac_bsscfg_t;

/** scanmac config data struct */
typedef struct wl_scanmac_config {
	struct ether_addr mac;	/**< 6 bytes of MAC address or MAC prefix (i.e. OUI) */
	struct ether_addr random_mask;	/**< randomized bits on each scan */
	uint16 scan_bitmap;	/**< scans to use this MAC address */
	uint8 pad[2];	/**< 4-byte struct alignment */
} wl_scanmac_config_t;

/* scan bitmap */
#define WL_SCANMAC_SCAN_UNASSOC		(0x01 << 0)	/**< unassociated scans */
#define WL_SCANMAC_SCAN_ASSOC_ROAM	(0x01 << 1)	/**< associated roam scans */
#define WL_SCANMAC_SCAN_ASSOC_PNO	(0x01 << 2)	/**< associated PNO scans */
#define WL_SCANMAC_SCAN_ASSOC_HOST	(0x01 << 3)	/**< associated host scans */

#endif /* LINUX_POSTMOGRIFY_REMOVAL */
/*
 * bonjour dongle offload definitions
 */

/* common iovar struct */
typedef struct wl_bdo {
	uint16 subcmd_id;	/* subcommand id */
	uint16 len;		/* total length of data[] */
	uint8 data[];		/* subcommand data */
} wl_bdo_t;

/* subcommand ids */
#define WL_BDO_SUBCMD_DOWNLOAD		0	/* Download flattened database  */
#define WL_BDO_SUBCMD_ENABLE		1	/* Start bonjour after download */
#define WL_BDO_SUBCMD_MAX_DOWNLOAD	2	/* Get the max download size    */

/* maximum fragment size */
#define BDO_MAX_FRAGMENT_SIZE	1024

/* download flattened database
 *
 * BDO must be disabled before database download else fail.
 *
 * If database size is within BDO_MAX_FRAGMENT_SIZE then only a single fragment
 * is required (i.e. frag_num = 0, total_size = frag_size).
 * If database size exceeds BDO_MAX_FRAGMENT_SIZE then multiple fragments are required.
 */
typedef struct wl_bdo_download {
	uint16 total_size;	/* total database size */
	uint16 frag_num;	/* fragment number, 0 for first fragment, N-1 for last fragment */
	uint16 frag_size;	/* size of fragment (max BDO_MAX_FRAGMENT_SIZE) */
	uint8 pad[2];		/* 4-byte struct alignment */
	uint8 fragment[BDO_MAX_FRAGMENT_SIZE];  /* fragment data */
} wl_bdo_download_t;

/* enable
 *
 * Enable requires a downloaded database else fail.
 */
typedef struct wl_bdo_enable {
	uint8 enable;	/* 1 - enable, 0 - disable */
	uint8 pad[3];	/* 4-byte struct alignment */
} wl_bdo_enable_t;

/*
 * Get the max download size for Bonjour Offload.
 */
typedef struct wl_bdo_max_download {
	uint16 size;	/* Max download size in bytes */
	uint8 pad[2];	/* 4-byte struct alignment    */
} wl_bdo_max_download_t;

/*
 * TCP keepalive offload definitions
 */

/* common iovar struct */
typedef struct wl_tko {
	uint16 subcmd_id;	/* subcommand id */
	uint16 len;		/* total length of data[] */
	uint8 data[];		/* subcommand data */
} wl_tko_t;

/* subcommand ids */
#define WL_TKO_SUBCMD_MAX_TCP		0	/* max TCP connections supported */
#define WL_TKO_SUBCMD_PARAM		1	/* configure offload common parameters  */
#define WL_TKO_SUBCMD_CONNECT		2	/* TCP connection info */
#define WL_TKO_SUBCMD_ENABLE		3	/* enable/disable */
#define WL_TKO_SUBCMD_STATUS		4	/* TCP connection status */

/* WL_TKO_SUBCMD_MAX_CONNECT subcommand data */
typedef struct wl_tko_max_tcp {
	uint8 max;	/* max TCP connections supported */
	uint8 pad[3];	/* 4-byte struct alignment */
} wl_tko_max_tcp_t;

/* WL_TKO_SUBCMD_PARAM subcommand data */
typedef struct wl_tko_param {
	uint16 interval;	/* keepalive tx interval (secs) */
	uint16 retry_interval;	/* keepalive retry interval (secs) */
	uint16 retry_count;	/* retry_count */
	uint8 pad[2];		/* 4-byte struct alignment */
} wl_tko_param_t;

/* WL_TKO_SUBCMD_CONNECT subcommand data
 * invoke with unique 'index' for each TCP connection
 */
typedef struct wl_tko_connect {
	uint8 index;		/* TCP connection index, 0 to max-1 */
	uint8 ip_addr_type;	/* 0 - IPv4, 1 - IPv6 */
	uint16 local_port;	/* local port */
	uint16 remote_port;	/* remote port */
	uint16 PAD;
	uint32 local_seq;	/* local sequence number */
	uint32 remote_seq;	/* remote sequence number */
	uint16 request_len;	/* TCP keepalive request packet length */
	uint16 response_len;	/* TCP keepalive response packet length */
	uint8 data[];		/* variable length field containing local/remote IPv4/IPv6,
				 * TCP keepalive request packet, TCP keepalive response packet
				 *    For IPv4, length is 4 * 2 + request_length + response_length
				 *       offset 0 - local IPv4
				 *       offset 4 - remote IPv4
				 *       offset 8 - TCP keepalive request packet
				 *       offset 8+request_length - TCP keepalive response packet
				 *    For IPv6, length is 16 * 2 + request_length + response_length
				 *       offset 0 - local IPv6
				 *       offset 16 - remote IPv6
				 *       offset 32 - TCP keepalive request packet
				 *       offset 32+request_length - TCP keepalive response packet
				 */
} wl_tko_connect_t;

/* WL_TKO_SUBCMD_CONNECT subcommand data to GET configured info for specific index */
typedef struct wl_tko_get_connect {
	uint8 index;		/* TCP connection index, 0 to max-1 */
	uint8 pad[3];		/* 4-byte struct alignment */
} wl_tko_get_connect_t;

typedef struct wl_tko_enable {
	uint8 enable;	/* 1 - enable, 0 - disable */
	uint8 pad[3];	/* 4-byte struct alignment */
} wl_tko_enable_t;

/* WL_TKO_SUBCMD_STATUS subcommand data */
/* must be invoked before tko is disabled else status is unavailable */
typedef struct wl_tko_status {
	uint8 count;		/* number of status entries (i.e. equals
				 * max TCP connections supported)
	                         */
	uint8 status[1];	/* variable length field contain status for
				 * each TCP connection index
				 */
} wl_tko_status_t;

typedef enum {
	TKO_STATUS_NORMAL			= 0,	/* TCP connection normal, no error */
	TKO_STATUS_NO_RESPONSE			= 1,	/* no response to TCP keepalive */
	TKO_STATUS_NO_TCP_ACK_FLAG		= 2,	/* TCP ACK flag not set */
	TKO_STATUS_UNEXPECT_TCP_FLAG		= 3,	/* unexpect TCP flags set other than ACK */
	TKO_STATUS_SEQ_NUM_INVALID		= 4,	/* ACK != sequence number */
	TKO_STATUS_REMOTE_SEQ_NUM_INVALID	= 5,	/* SEQ > remote sequence number */
	TKO_STATUS_TCP_DATA			= 6,	/* TCP data available */
	TKO_STATUS_UNAVAILABLE			= 255,	/* not used/configured */
} tko_status_t;

enum rssi_reason {
	RSSI_REASON_UNKNOW = 0,
	RSSI_REASON_LOWRSSI = 1,
	RSSI_REASON_NSYC = 2,
	RSSI_REASON_TIMEOUT = 3
};

enum tof_reason {
	TOF_REASON_OK = 0,
	TOF_REASON_REQEND = 1,
	TOF_REASON_TIMEOUT = 2,
	TOF_REASON_NOACK = 3,
	TOF_REASON_INVALIDAVB = 4,
	TOF_REASON_INITIAL = 5,
	TOF_REASON_ABORT = 6
};

enum rssi_state {
	RSSI_STATE_POLL = 0,
	RSSI_STATE_TPAIRING = 1,
	RSSI_STATE_IPAIRING = 2,
	RSSI_STATE_THANDSHAKE = 3,
	RSSI_STATE_IHANDSHAKE = 4,
	RSSI_STATE_CONFIRMED = 5,
	RSSI_STATE_PIPELINE = 6,
	RSSI_STATE_NEGMODE = 7,
	RSSI_STATE_MONITOR = 8,
	RSSI_STATE_LAST = 9
};

enum tof_state {
	TOF_STATE_IDLE	 = 0,
	TOF_STATE_IWAITM = 1,
	TOF_STATE_TWAITM = 2,
	TOF_STATE_ILEGACY = 3,
	TOF_STATE_IWAITCL = 4,
	TOF_STATE_TWAITCL = 5,
	TOF_STATE_ICONFIRM = 6,
	TOF_STATE_IREPORT = 7
};

enum tof_mode_type {
	TOF_LEGACY_UNKNOWN	= 0,
	TOF_LEGACY_AP		= 1,
	TOF_NONLEGACY_AP	= 2
};

enum tof_way_type {
	TOF_TYPE_ONE_WAY = 0,
	TOF_TYPE_TWO_WAY = 1,
	TOF_TYPE_REPORT = 2
};

enum tof_rate_type {
	TOF_FRAME_RATE_VHT = 0,
	TOF_FRAME_RATE_LEGACY = 1
};

#define TOF_ADJ_TYPE_NUM	4	/**< number of assisted timestamp adjustment */
enum tof_adj_mode {
	TOF_ADJ_SOFTWARE = 0,
	TOF_ADJ_HARDWARE = 1,
	TOF_ADJ_SEQ = 2,
	TOF_ADJ_NONE = 3
};

#define FRAME_TYPE_NUM		4	/**< number of frame type */
enum frame_type {
	FRAME_TYPE_CCK	= 0,
	FRAME_TYPE_OFDM	= 1,
	FRAME_TYPE_11N	= 2,
	FRAME_TYPE_11AC	= 3
};

typedef struct wl_proxd_status_iovar {
	uint16			method;				/**< method */
	uint8			mode;				/**< mode */
	uint8			peermode;			/**< peer mode */
	uint8			state;				/**< state */
	uint8			reason;				/**< reason code */
	uint8			PAD[2];
	uint32			distance;			/**< distance */
	uint32			txcnt;				/**< tx pkt counter */
	uint32			rxcnt;				/**< rx pkt counter */
	struct ether_addr	peer;				/**< peer mac address */
	int8			avg_rssi;			/**< average rssi */
	int8			hi_rssi;			/**< highest rssi */
	int8			low_rssi;			/**< lowest rssi */
	uint8			PAD[3];
	uint32			dbgstatus;			/**< debug status */
	uint16			frame_type_cnt[FRAME_TYPE_NUM];	/**< frame types */
	uint8			adj_type_cnt[TOF_ADJ_TYPE_NUM];	/**< adj types HW/SW */
} wl_proxd_status_iovar_t;

/* ifdef NET_DETECT */
typedef struct net_detect_adapter_features {
	uint8	wowl_enabled;
	uint8	net_detect_enabled;
	uint8	nlo_enabled;
} net_detect_adapter_features_t;

typedef enum net_detect_bss_type {
	nd_bss_any = 0,
	nd_ibss,
	nd_ess
} net_detect_bss_type_t;

typedef struct net_detect_profile {
	wlc_ssid_t		ssid;
	net_detect_bss_type_t   bss_type;	/**< Ignore for now since Phase 1 is only for ESS */
	uint32			cipher_type;	/**< DOT11_CIPHER_ALGORITHM enumeration values */
	uint32			auth_type;	/**< DOT11_AUTH_ALGORITHM enumeration values */
} net_detect_profile_t;

typedef struct net_detect_profile_list {
	uint32			num_nd_profiles;
	net_detect_profile_t	nd_profile[];
} net_detect_profile_list_t;

typedef struct net_detect_config {
	uint8			    nd_enabled;
	uint8				PAD[3];
	uint32			    scan_interval;
	uint32			    wait_period;
	uint8			    wake_if_connected;
	uint8			    wake_if_disconnected;
	uint8				PAD[2];
	net_detect_profile_list_t   nd_profile_list;
} net_detect_config_t;

typedef enum net_detect_wake_reason {
	nd_reason_unknown,
	nd_net_detected,
	nd_wowl_event,
	nd_ucode_error
} net_detect_wake_reason_t;

typedef struct net_detect_wake_data {
	net_detect_wake_reason_t    nd_wake_reason;
	uint32			    nd_wake_date_length;
	uint8			    nd_wake_data[0];	    /**< Wake data (currently unused) */
} net_detect_wake_data_t;

/* endif NET_DETECT */

/* (unversioned, deprecated) */
typedef struct bcnreq {
	uint8 bcn_mode;
	uint8 PAD[3];
	int32 dur;
	int32 channel;
	struct ether_addr da;
	uint16 random_int;
	wlc_ssid_t ssid;
	uint16 reps;
} bcnreq_t;

#define WL_RRM_BCN_REQ_VER		1
typedef struct bcn_req {
	uint8 version;
	uint8 bcn_mode;
	uint8 pad_1[2];
	int32 dur;
	int32 channel;
	struct ether_addr da;
	uint16 random_int;
	wlc_ssid_t ssid;
	uint16 reps;
	uint8 req_elements;
	uint8 pad_2;
	chanspec_list_t chspec_list;
} bcn_req_t;

typedef struct rrmreq {
	struct ether_addr da;
	uint8 reg;
	uint8 chan;
	uint16 random_int;
	uint16 dur;
	uint16 reps;
} rrmreq_t;

typedef struct framereq {
	struct ether_addr da;
	uint8 reg;
	uint8 chan;
	uint16 random_int;
	uint16 dur;
	struct ether_addr ta;
	uint16 reps;
} framereq_t;

typedef struct statreq {
	struct ether_addr da;
	struct ether_addr peer;
	uint16 random_int;
	uint16 dur;
	uint8 group_id;
	uint8 PAD;
	uint16 reps;
} statreq_t;

typedef struct txstrmreq {
	struct ether_addr da;	/* Destination address */
	uint16 random_int;	/* Random interval for measurement start */
	uint16 dur;		/* Measurement duration */
	uint16 reps;		/* number of repetitions */
	struct ether_addr peer;	/* Peer MAC address */
	uint8 tid;		/* Traffic ID */
	uint8 bin0_range;	/* Delay range of the first bin */
} txstrmreq_t;

#define WL_RRM_LCI_REQ_VER		1
typedef struct lci_req {
	uint8 version;
	uint8 len;
	struct ether_addr da;
	uint8 subject;
	uint8 azimuth_req;
	uint16 max_age;
	struct ether_addr orig_addr;
	struct ether_addr target_addr;
} lci_req_t;

#define WL_RRM_CIVIC_REQ_VER		1
typedef struct civic_req {
	uint8 version;
	uint8 len;
	struct ether_addr da;
	uint8 subject;
	uint8 type;
	uint16 service_interval;
	uint8 si_units;
	uint8 pad[3];
	struct ether_addr orig_addr;
	struct ether_addr target_addr;
} civic_req_t;

/* (unversioned, deprecated) */
typedef struct lcireq {
	struct ether_addr da;	/* Destination address */
	uint16 reps;		/* number of repetitions */
	uint8 subj;		/* Local/Remote/Thid party */
	uint8 lat_res;		/* Latitude requested Resolution */
	uint8 lon_res;		/* Longitude requested Resolution */
	uint8 alt_res;		/* Altitude requested Resolution */
} lcireq_t;

/* (unversioned, deprecated) */
typedef struct civicreq {
	struct ether_addr da;	/* Destination address */
	uint16 reps;		/* number of repetitions */
	uint8 subj;		/* Local/Remote/Thid party */
	uint8 civloc_type;	/* Format of location info */
	uint8 siu;		/* Unit of Location service interval */
	uint8 pad;
	uint16 si;		/* Location service interval */
} civicreq_t;

typedef struct locidreq {
	struct ether_addr da;	/* Destination address */
	uint16 reps;		/* number of repetitions */
	uint8 subj;		/* Local/Remote/Thid party */
	uint8 siu;		/* Unit of Location service interval */
	uint16 si;		/* Location service interval */
} locidreq_t;

typedef struct wl_rrm_config_ioc {
	uint16 version; /* command version */
	uint16 id;      /* subiovar cmd ID */
	uint16 len;     /* total length of all bytes in data[] */
	uint16 pad;     /* 4-byte boundary padding */
	uint8 data[1];  /* payload */
} wl_rrm_config_ioc_t;

enum {
	WL_RRM_CONFIG_NONE	= 0,	/* reserved */
	WL_RRM_CONFIG_GET_LCI	= 1,	/* get LCI */
	WL_RRM_CONFIG_SET_LCI	= 2,	/* set LCI */
	WL_RRM_CONFIG_GET_CIVIC	= 3,	/* get civic location */
	WL_RRM_CONFIG_SET_CIVIC	= 4,	/* set civic location */
	WL_RRM_CONFIG_GET_LOCID	= 5,	/* get location identifier */
	WL_RRM_CONFIG_SET_LOCID	= 6,	/* set location identifier */
	WL_RRM_CONFIG_MAX	= 7
};

#define WL_RRM_CONFIG_NAME "rrm_config"
#define WL_RRM_CONFIG_MIN_LENGTH OFFSETOF(wl_rrm_config_ioc_t, data)

enum {
	WL_RRM_EVENT_NONE		= 0,	/* not an event, reserved */
	WL_RRM_EVENT_FRNG_REQ	= 1,	/* Receipt of FRNG request frame */
	WL_RRM_EVENT_FRNG_REP	= 2,	/* Receipt of FRNG report frame */

	WL_RRM_EVENT_MAX
};
typedef int16 wl_rrm_event_type_t;

typedef struct frngreq_target {
	uint32 bssid_info;
	uint8 channel;
	uint8 phytype;
	uint8 reg;
	uint8 pad;
	struct ether_addr bssid;
	chanspec_t chanspec;
	uint32 sid;
} frngreq_target_t;

typedef struct frngreq {
	wl_rrm_event_type_t event;			/* RRM event type */
	struct ether_addr da;
	uint16 max_init_delay;	/* Upper bound of random delay, in TUs */
	uint8 min_ap_count;		/* Min FTM ranges requested (1-15) */
	uint8 num_aps;			/* Number of APs to range, at least min_ap_count */
	uint16 max_age;			/* Max elapsed time before FTM request, 0xFFFF = any */
	uint16 reps;			/* Number of repetitions of this measurement type */
	frngreq_target_t targets[1];	/* Target BSSIDs to range */
} frngreq_t;

typedef struct frngrep_range {
	uint32 start_tsf;		/* 4 lsb of tsf */
	struct ether_addr bssid;
	uint8 pad[2];
	uint32 range;
	uint32 max_err;
	uint8  rsvd;
	uint8 pad2[3];
} frngrep_range_t;

typedef struct frngrep_error {
	uint32 start_tsf;		/* 4 lsb of tsf */
	struct ether_addr bssid;
	uint8  code;
	uint8 pad[1];
} frngrep_error_t;

typedef struct frngrep {
	wl_rrm_event_type_t event;			/* RRM event type */
	struct ether_addr da;
	uint8 range_entry_count;
	uint8 error_entry_count;
	uint16 dialog_token;				/* dialog token */
	frngrep_range_t range_entries[DOT11_FTM_RANGE_ENTRY_MAX_COUNT];
	frngrep_error_t error_entries[DOT11_FTM_RANGE_ERROR_ENTRY_MAX_COUNT];
} frngrep_t;

typedef struct wl_rrm_frng_ioc {
	uint16 version; /* command version */
	uint16 id;      /* subiovar cmd ID */
	uint16 len;     /* total length of all bytes in data[] */
	uint16 pad;     /* 4-byte boundary padding */
	uint8 data[];  /* payload */
} wl_rrm_frng_ioc_t;

enum {
	WL_RRM_FRNG_NONE	= 0,	/* reserved */
	WL_RRM_FRNG_SET_REQ	= 1,	/* send ftm ranging request */
	WL_RRM_FRNG_SET_REP	= 2,	/* send ftm ranging report */
	WL_RRM_FRNG_SET_DIR	= 3,	/* direct ranging (let the driver do ranging) */
	WL_RRM_FRNG_MAX		= 4
};

#define WL_RRM_FRNG_NAME "rrm_frng"
#define WL_RRM_FRNG_MIN_LENGTH OFFSETOF(wl_rrm_frng_ioc_t, data)

#define WL_RRM_RPT_VER		0
#define WL_RRM_RPT_MAX_PAYLOAD	256
#define WL_RRM_RPT_MIN_PAYLOAD	7
#define WL_RRM_RPT_FALG_ERR	0
#define WL_RRM_RPT_FALG_GRP_ID_PROPR	(1 << 0)
#define WL_RRM_RPT_FALG_GRP_ID_0	(1 << 1)
typedef struct {
	uint16 ver;		/**< version */
	struct ether_addr addr;	/**< STA MAC addr */
	uint32 timestamp;	/**< timestamp of the report */
	uint16 flag;		/**< flag */
	uint16 len;		/**< length of payload data */
	uint8 data[WL_RRM_RPT_MAX_PAYLOAD];
} statrpt_t;

typedef struct wlc_dwds_config {
	uint32		enable;
	uint32		mode; /**< STA/AP interface */
	struct ether_addr ea;
	uint8  PAD[2];
} wlc_dwds_config_t;

typedef struct wl_el_set_params_s {
	uint8 set;	/**< Set number */
	uint8  PAD[3];
	uint32 size;	/**< Size to make/expand */
} wl_el_set_params_t;

typedef struct wl_el_tag_params_s {
	uint16 tag;
	uint8 set;
	uint8 flags;
} wl_el_tag_params_t;

/* Below to be removed after K branch */
/** Video Traffic Interference Monitor config */
#define INTFER_VERSION		1
typedef struct wl_intfer_params {
	uint16 version;			/**< version */
	uint8 period;			/**< sample period */
	uint8 cnt;			/**< sample cnt */
	uint8 txfail_thresh;	/**< non-TCP txfail threshold */
	uint8 tcptxfail_thresh;	/**< tcptxfail threshold */
} wl_intfer_params_t;

typedef struct wl_staprio_cfg {
	struct ether_addr ea;	/**< mac addr */
	uint8 prio;		/**< scb priority */
} wl_staprio_cfg_t;

#define STAMON_CNTR_VER		1
#define STAMON_STACONFIG_VER    1
/* size of struct wlc_stamon_sta_config_t elements */
#define STAMON_STACONFIG_LENGTH 24

typedef enum wl_stamon_cfg_cmd_type {
	STAMON_CFG_CMD_DEL = 0,
	STAMON_CFG_CMD_ADD = 1,
	STAMON_CFG_CMD_ENB = 2,
	STAMON_CFG_CMD_DSB = 3,
	STAMON_CFG_CMD_CNT = 4,
	STAMON_CFG_CMD_RSTCNT = 5,
	STAMON_CFG_CMD_GET_STATS = 6,
	STAMON_CFG_CMD_SET_MONTIME = 7
} wl_stamon_cfg_cmd_type_t;

typedef struct wlc_stamon_sta_config {
	wl_stamon_cfg_cmd_type_t cmd;	/**< 0 - delete, 1 - add */
	struct ether_addr ea;
	uint16	version;		/* Command structure version */
	uint16	length;			/* Command structure length */
	chanspec_t chanspec;
	/* Time (ms) for which STA's are monitored. Value ZERO indicates no time limit */
	uint32	monitor_time;
	uint32 offchan_time;	/* Time (ms) for which off-channel STA's are monitored. */
} wlc_stamon_sta_config_t;

#define CSIMON_STACONFIG_VER    1
#define CSIMON_CNTR_VER		1
#define CSIMON_STACONFIG_LENGTH sizeof(wlc_csimon_sta_config_t)
/* Note that the del/add commands are per STA whereas the disable/enable
 * commands are for the CSIMON feature applicable to all STAs
 */
typedef enum wl_csimon_cfg_cmd_type {
	CSIMON_CFG_CMD_DEL = 0,
	CSIMON_CFG_CMD_ADD = 1,
	CSIMON_CFG_CMD_ENB = 2,
	CSIMON_CFG_CMD_DSB = 3,
	CSIMON_CFG_CMD_RSTCNT = 4
} wl_csimon_cfg_cmd_type_t;

typedef struct csimon_state {
	uint32 version;
	uint32 length;
	bool enabled;
	uint32 null_frm_cnt;
	uint32 m2mxfer_cnt;
	uint32 ack_fail_cnt;
	uint32 rec_ovfl_cnt;
	uint32 xfer_fail_cnt;
} csimon_state_t;

typedef struct wlc_csimon_sta_config {
	wl_csimon_cfg_cmd_type_t cmd;	/* 0 - delete, 1 - add */
	struct ether_addr ea;
	uint16	version;		/* Command structure version */
	uint16	length;			/* Command structure length */
	/* Time (ms) interval between successive CSI reports generated for STA */
	uint32	monitor_interval;
} wlc_csimon_sta_config_t;

/* ifdef SR_DEBUG */
typedef struct /* pmu_reg */{
	uint32  pmu_control;
	uint32  pmu_capabilities;
	uint32  pmu_status;
	uint32  res_state;
	uint32  res_pending;
	uint32  pmu_timer1;
	uint32  min_res_mask;
	uint32  max_res_mask;
	uint32  pmu_chipcontrol1[4];
	uint32  pmu_regcontrol[5];
	uint32  pmu_pllcontrol[5];
	uint32  pmu_rsrc_up_down_timer[31];
	uint32  rsrc_dep_mask[31];
} pmu_reg_t;
/* endif SR_DEBUG */

typedef struct wl_taf_define {
	struct ether_addr ea;	/**< STA MAC or 0xFF... */
	uint16 version;         /**< version */
	uint32 sch;             /**< method index */
	uint32 prio;            /**< priority */
	uint32 misc;            /**< used for return value */
	uint8  text[];         /**< used to pass and return ascii text */
} wl_taf_define_t;

/** Received Beacons lengths information */
#define WL_LAST_BCNS_INFO_FIXED_LEN		OFFSETOF(wlc_bcn_len_hist_t, bcnlen_ring)
typedef struct wlc_bcn_len_hist {
	uint16	ver;				/**< version field */
	uint16	cur_index;			/**< current pointed index in ring buffer */
	uint32	max_bcnlen;		/**< Max beacon length received */
	uint32	min_bcnlen;		/**< Min beacon length received */
	uint32	ringbuff_len;		/**< Length of the ring buffer 'bcnlen_ring' */
	uint32	bcnlen_ring[1];	/**< ring buffer storing received beacon lengths */
} wlc_bcn_len_hist_t;

/* WDS net interface types */
#define WL_WDSIFTYPE_NONE  0x0 /**< The interface type is neither WDS nor DWDS. */
#define WL_WDSIFTYPE_WDS   0x1 /**< The interface is WDS type. */
#define WL_WDSIFTYPE_DWDS  0x2 /**< The interface is DWDS type. */

typedef struct wl_bssload_static {
	uint8 is_static;
	uint8  PAD;
	uint16 sta_count;
	uint8 chan_util;
	uint8  PAD;
	uint16 aac;
} wl_bssload_static_t;

/* Buffer of size WLC_SAMPLECOLLECT_MAXLEN (=10240 for 4345a0 ACPHY)
 * gets copied to this, multiple times
 */
typedef enum wl_gpaio_option {
	GPAIO_PMU_AFELDO,
	GPAIO_PMU_TXLDO,
	GPAIO_PMU_VCOLDO,
	GPAIO_PMU_LNALDO,
	GPAIO_PMU_ADCLDO,
	GPAIO_ICTAT_CAL,
	GPAIO_PMU_CLEAR,
	GPAIO_OFF,
	GPAIO_PMU_LOGENLDO,
	GPAIO_PMU_RXLDO2G,
	GPAIO_PMU_RXLDO5G,
	GPAIO_PMU_LPFTXLDO,
	GPAIO_PMU_LDO1P6,
	GPAIO_RCAL,
	GPAIO_IQDAC_BUF_DC_SETUP,
	GPAIO_IQDAC_BUF_DC_MEAS,
	GPAIO_IQDAC_BUF_DC_CLEAR
} wl_gpaio_option_t;

/** IO Var Operations - the Value of iov_op In wlc_ap_doiovar */
typedef enum wlc_ap_iov_bss_operation {
	WLC_AP_IOV_OP_DELETE                   = -1,
	WLC_AP_IOV_OP_DISABLE                  = 0,
	WLC_AP_IOV_OP_ENABLE                   = 1,
	WLC_AP_IOV_OP_MANUAL_AP_BSSCFG_CREATE  = 2,
	WLC_AP_IOV_OP_MANUAL_STA_BSSCFG_CREATE = 3,
	WLC_AP_IOV_OP_MOVE                     = 4
} wlc_ap_iov_bss_oper_t;

/* LTE coex info */
/* Analogue of HCI Set MWS Signaling cmd */
typedef struct {
	int16	mws_rx_assert_offset;
	int16	mws_rx_assert_jitter;
	int16	mws_rx_deassert_offset;
	int16	mws_rx_deassert_jitter;
	int16	mws_tx_assert_offset;
	int16	mws_tx_assert_jitter;
	int16	mws_tx_deassert_offset;
	int16	mws_tx_deassert_jitter;
	int16	mws_pattern_assert_offset;
	int16	mws_pattern_assert_jitter;
	int16	mws_inact_dur_assert_offset;
	int16	mws_inact_dur_assert_jitter;
	int16	mws_scan_freq_assert_offset;
	int16	mws_scan_freq_assert_jitter;
	int16	mws_prio_assert_offset_req;
} wci2_config_t;

/** Analogue of HCI MWS Channel Params */
typedef struct {
	uint16	mws_rx_center_freq; /**< MHz */
	uint16	mws_tx_center_freq;
	uint16	mws_rx_channel_bw;  /**< KHz */
	uint16	mws_tx_channel_bw;
	uint8	mws_channel_en;
	uint8	mws_channel_type;   /**< Don't care for WLAN? */
} mws_params_t;

#define LTECX_MAX_NUM_PERIOD_TYPES	7

/* LTE Frame params */
typedef struct {
	uint16	mws_frame_dur;
	int16	mws_framesync_assert_offset;
	uint16	mws_framesync_assert_jitter;
	uint16  mws_period_dur[LTECX_MAX_NUM_PERIOD_TYPES];
	uint8	mws_period_type[LTECX_MAX_NUM_PERIOD_TYPES];
	uint8	mws_num_periods;
} mws_frame_config_t;

/** MWS wci2 message */
typedef struct {
	uint8	mws_wci2_data; /**< BT-SIG msg */
	uint8	PAD;
	uint16	mws_wci2_interval; /**< Interval in us */
	uint16	mws_wci2_repeat; /**< No of msgs to send */
} mws_wci2_msg_t;
/* MWS ANT map */
typedef struct {
	uint16	combo1; /* mws ant selection 1 */
	uint16	combo2; /* mws ant selection 2 */
	uint16	combo3; /* mws ant selection 3 */
	uint16	combo4; /* mws ant selection 4 */
} mws_ant_map_t;

/* MWS SCAN_REQ Bitmap */
typedef struct mws_scanreq_params {
	uint16 idx;
	uint16 bm_2g;
	uint16 bm_5g_lo;
	uint16 bm_5g_mid;
	uint16 bm_5g_hi;
} mws_scanreq_params_t;

typedef struct {
	uint32 config;	/**< MODE: AUTO (-1), Disable (0), Enable (1) */
	uint32 status;	/**< Current state: Disabled (0), Enabled (1) */
} wl_config_t;

#define WLC_RSDB_MODE_AUTO_MASK 0x80
#define WLC_RSDB_EXTRACT_MODE(val) ((int8)((val) & (~(WLC_RSDB_MODE_AUTO_MASK))))

typedef struct {
	uint16  request; /* type of sensor hub request */
	uint16  enable; /* enable/disable response for specified request */
	uint16  interval; /* interval between responses to the request */
} shub_req_t;

#define	WL_IF_STATS_T_VERSION 1	/**< current version of wl_if_stats structure */

/** per interface counters */
typedef struct wl_if_stats {
	uint16	version;		/**< version of the structure */
	uint16	length;			/**< length of the entire structure */
	uint32	PAD;			/**< padding */

	/* transmit stat counters */
	uint64	txframe;		/**< tx data frames */
	uint64	txbyte;			/**< tx data bytes */
	uint64	txerror;		/**< tx data errors (derived: sum of others) */
	uint64  txnobuf;		/**< tx out of buffer errors */
	uint64  txrunt;			/**< tx runt frames */
	uint64  txfail;			/**< tx failed frames */
	uint64	txretry;		/**< tx retry frames */
	uint64	txretrie;		/**< tx multiple retry frames */
	uint64	txfrmsnt;		/**< tx sent frames */
	uint64	txmulti;		/**< tx mulitcast sent frames */
	uint64	txfrag;			/**< tx fragments sent */

	/* receive stat counters */
	uint64	rxframe;		/**< rx data frames */
	uint64	rxbyte;			/**< rx data bytes */
	uint64	rxerror;		/**< rx data errors (derived: sum of others) */
	uint64	rxnobuf;		/**< rx out of buffer errors */
	uint64  rxrunt;			/**< rx runt frames */
	uint64  rxfragerr;		/**< rx fragment errors */
	uint64	rxmulti;		/**< rx multicast frames */

	uint64	txexptime;		/* DATA Tx frames suppressed due to timer expiration */
	uint64	txrts;			/* RTS/CTS succeeeded count */
	uint64	txnocts;		/* RTS/CTS faled count */

	uint64	txretrans;		/* Number of frame retransmissions */
}
wl_if_stats_t;

typedef struct wl_band {
	uint16		bandtype;		/**< WL_BAND_2G, WL_BAND_5G */
	uint16		bandunit;		/**< bandstate[] index */
	uint16		phytype;		/**< phytype */
	uint16		phyrev;
}
wl_band_t;

/* Version 2 adds versions of ppr_t and ppr_ru_t */
#define	WL_WLC_VERSION_T_VERSION 2 /**< current version of wlc_version structure */

/** wlc interface version */
typedef struct wl_wlc_version {
	uint16	version;		/**< version of the structure */
	uint16	length;			/**< length of the entire structure */

	/* epi version numbers */
	uint16	epi_ver_major;		/**< epi major version number */
	uint16	epi_ver_minor;		/**< epi minor version number */
	uint16	epi_rc_num;		/**< epi RC number */
	uint16	epi_incr_num;		/**< epi increment number */

	/* wlc interface version numbers */
	uint16	wlc_ver_major;		/**< wlc interface major version number */
	uint16	wlc_ver_minor;		/**< wlc interface minor version number */

	/* version of ppr structure */
	/* Added from WL_WLC_VERSION_T_VERSION = 2 */
	uint16 ppr_version;		/* version of pprpbw_t */
	uint16 ppr_ru_version;		/* version of pprpbw_ru_t */
}
wl_wlc_version_t;

/* Highest version of WLC_API_VERSION supported */
#define WLC_API_VERSION_MAJOR_MAX	8
#define WLC_API_VERSION_MINOR_MAX	0

/* begin proxd definitions */
#include <packed_section_start.h>

#define WL_PROXD_API_VERSION 0x0300	/**< version 3.0 */

/** Minimum supported API version */
#define WL_PROXD_API_MIN_VERSION 0x0300

/** proximity detection methods */
enum {
	WL_PROXD_METHOD_NONE	= 0,
	WL_PROXD_METHOD_RSVD1	= 1, /**< backward compatibility - RSSI, not supported */
	WL_PROXD_METHOD_TOF	= 2,
	WL_PROXD_METHOD_RSVD2	= 3, /**< 11v only - if needed */
	WL_PROXD_METHOD_FTM	= 4, /**< IEEE rev mc/2014 */
	WL_PROXD_METHOD_MAX
};
typedef int16 wl_proxd_method_t;

/** global and method configuration flags */
enum {
	WL_PROXD_FLAG_NONE			= 0x00000000,
	WL_PROXD_FLAG_RX_ENABLED		= 0x00000001, /**< respond to requests, per bss */
	WL_PROXD_FLAG_RX_RANGE_REQ		= 0x00000002, /**< 11mc range requests enabled */
	WL_PROXD_FLAG_TX_LCI			= 0x00000004, /**< tx lci, if known */
	WL_PROXD_FLAG_TX_CIVIC			= 0x00000008, /**< tx civic, if known */
	WL_PROXD_FLAG_RX_AUTO_BURST		= 0x00000010, /**< auto respond w/o host action */
	WL_PROXD_FLAG_TX_AUTO_BURST		= 0x00000020, /**< continue tx w/o host action */
	WL_PROXD_FLAG_AVAIL_PUBLISH		= 0x00000040,     /**< publish availability */
	WL_PROXD_FLAG_AVAIL_SCHEDULE		= 0x00000080,    /**< schedule using availability */
	WL_PROXD_FLAG_ASAP_CAPABLE		= 0x00000100, /* ASAP capable */
	WL_PROXD_FLAG_MBURST_FOLLOWUP		= 0x00000200, /* new multi-burst algorithm */
	WL_PROXD_FLAG_SECURE			= 0x00000400, /* per bsscfg option */
	WL_PROXD_FLAG_NO_TSF_SYNC		= 0x00000800, /* disable tsf sync */
	WL_PROXD_FLAG_ALL			= 0xffffffff
};
typedef uint32 wl_proxd_flags_t;

#define WL_PROXD_FLAGS_AVAIL (WL_PROXD_FLAG_AVAIL_PUBLISH | \
	WL_PROXD_FLAG_AVAIL_SCHEDULE)

/** session flags */
enum {
	WL_PROXD_SESSION_FLAG_NONE 		= 0x00000000,  /**< no flags */
	WL_PROXD_SESSION_FLAG_INITIATOR 	= 0x00000001,  /**< local device is initiator */
	WL_PROXD_SESSION_FLAG_TARGET 		= 0x00000002,  /**< local device is target */
	WL_PROXD_SESSION_FLAG_ONE_WAY		= 0x00000004,  /**< (initiated) 1-way rtt */
	WL_PROXD_SESSION_FLAG_AUTO_BURST	= 0x00000008,  /**< created w/ rx_auto_burst */
	WL_PROXD_SESSION_FLAG_PERSIST		= 0x00000010,  /**< good until cancelled */
	WL_PROXD_SESSION_FLAG_RTT_DETAIL	= 0x00000020,  /**< rtt detail in results */
	WL_PROXD_SESSION_FLAG_SECURE		= 0x00000040,  /**< sessionis secure */
	WL_PROXD_SESSION_FLAG_AOA		= 0x00000080,  /**< AOA along w/ RTT */
	WL_PROXD_SESSION_FLAG_RX_AUTO_BURST	= 0x00000100,  /**< Same as proxd flags above */
	WL_PROXD_SESSION_FLAG_TX_AUTO_BURST	= 0x00000200,  /**< Same as proxd flags above */
	WL_PROXD_SESSION_FLAG_NAN_BSS		= 0x00000400,  /**< Use NAN BSS, if applicable */
	WL_PROXD_SESSION_FLAG_TS1		= 0x00000800,  /**< e.g. FTM1 - ASAP-capable */
	WL_PROXD_SESSION_FLAG_RANDMAC		= 0x00001000,  /**< use random mac */
	WL_PROXD_SESSION_FLAG_REPORT_FAILURE	= 0x00002000, /**< report failure to target */
	WL_PROXD_SESSION_FLAG_INITIATOR_RPT	= 0x00004000, /**< report distance to target */
	WL_PROXD_SESSION_FLAG_NOCHANSWT		= 0x00008000,
	WL_PROXD_SESSION_FLAG_NETRUAL		= 0x00010000, /**< netrual mode */
	WL_PROXD_SESSION_FLAG_SEQ_EN		= 0x00020000, /**< Toast */
	WL_PROXD_SESSION_FLAG_NO_PARAM_OVRD	= 0x00040000, /**< no param override from target */
	WL_PROXD_SESSION_FLAG_ASAP		= 0x00080000, /**< ASAP session */
	WL_PROXD_SESSION_FLAG_REQ_LCI		= 0x00100000, /**< transmit LCI req */
	WL_PROXD_SESSION_FLAG_REQ_CIV		= 0x00200000, /**< transmit civic loc req */
	WL_PROXD_SESSION_FLAG_PRE_SCAN		= 0x00400000, /* enable pre-scan for asap=1 */
	WL_PROXD_SESSION_FLAG_AUTO_VHTACK	= 0x00800000, /* use vhtack based on brcm ie */
	WL_PROXD_SESSION_FLAG_VHTACK		= 0x01000000, /* vht ack is in use - output only */
	WL_PROXD_SESSION_FLAG_BDUR_NOPREF	= 0x02000000, /* burst-duration: no preference */
	WL_PROXD_SESSION_FLAG_NUM_FTM_NOPREF	= 0x04000000, /* num of FTM frames: no preference */
	WL_PROXD_SESSION_FLAG_FTM_SEP_NOPREF	= 0x08000000, /* time btw FTM frams: no pref */
	WL_PROXD_SESSION_FLAG_NUM_BURST_NOPREF	= 0x10000000, /* num of bursts: no pref */
	WL_PROXD_SESSION_FLAG_BURST_PERIOD_NOPREF = 0x20000000, /* burst period: no pref */
	WL_PROXD_SESSION_FLAG_MBURST_FOLLOWUP	= 0x40000000, /* new mburst algo  - reserved */
	WL_PROXD_SESSION_FLAG_MBURST_NODELAY	= 0x80000000, /**< good until cancelled */
	WL_PROXD_SESSION_FLAG_ALL		= 0xffffffff

};
typedef uint32 wl_proxd_session_flags_t;

/** time units - mc supports up to 0.1ns resolution */
enum {
	WL_PROXD_TMU_TU			= 0,		/**< 1024us */
	WL_PROXD_TMU_SEC		= 1,
	WL_PROXD_TMU_MILLI_SEC	= 2,
	WL_PROXD_TMU_MICRO_SEC	= 3,
	WL_PROXD_TMU_NANO_SEC	= 4,
	WL_PROXD_TMU_PICO_SEC	= 5
};
typedef int16 wl_proxd_tmu_t;

/** time interval e.g. 10ns */
typedef struct wl_proxd_intvl {
	uint32 intvl;
	wl_proxd_tmu_t tmu;
	uint8	pad[2];
} wl_proxd_intvl_t;

/** commands that can apply to proxd, method or a session */
enum {
	WL_PROXD_CMD_NONE			= 0,
	WL_PROXD_CMD_GET_VERSION		= 1,
	WL_PROXD_CMD_ENABLE			= 2,
	WL_PROXD_CMD_DISABLE			= 3,
	WL_PROXD_CMD_CONFIG			= 4,
	WL_PROXD_CMD_START_SESSION		= 5,
	WL_PROXD_CMD_BURST_REQUEST		= 6,
	WL_PROXD_CMD_STOP_SESSION		= 7,
	WL_PROXD_CMD_DELETE_SESSION		= 8,
	WL_PROXD_CMD_GET_RESULT			= 9,
	WL_PROXD_CMD_GET_INFO			= 10,
	WL_PROXD_CMD_GET_STATUS			= 11,
	WL_PROXD_CMD_GET_SESSIONS		= 12,
	WL_PROXD_CMD_GET_COUNTERS		= 13,
	WL_PROXD_CMD_CLEAR_COUNTERS		= 14,
	WL_PROXD_CMD_COLLECT			= 15,	/* not supported, see 'wl proxd_collect' */
	WL_PROXD_CMD_TUNE			= 16,	/* not supported, see 'wl proxd_tune' */
	WL_PROXD_CMD_DUMP			= 17,
	WL_PROXD_CMD_START_RANGING		= 18,
	WL_PROXD_CMD_STOP_RANGING		= 19,
	WL_PROXD_CMD_GET_RANGING_INFO		= 20,
	WL_PROXD_CMD_IS_TLV_SUPPORTED		= 21,

	WL_PROXD_CMD_MAX
};
typedef int16 wl_proxd_cmd_t;

/* session ids:
 * id 0 is reserved
 * ids 1..0x7fff - allocated by host/app
 * 0x8000-0xffff - allocated by firmware, used for auto/rx
 */
enum {
	 WL_PROXD_SESSION_ID_GLOBAL = 0
};

/* Externally allocated sids */
#define WL_PROXD_SID_EXT_MAX 0x7fff
#define WL_PROXD_SID_EXT_ALLOC(_sid) ((_sid) > 0 && (_sid) <= WL_PROXD_SID_EXT_MAX)

/* block size for reserved sid blocks */
#define WL_PROXD_SID_EXT_BLKSZ 256
#define WL_PROXD_SID_EXT_BLK_START(_i) (WL_PROXD_SID_EXT_MAX - (_i) * WL_PROXD_SID_EXT_BLKSZ + 1)
#define WL_PROXD_SID_EXT_BLK_END(_start) ((_start) + WL_PROXD_SID_EXT_BLKSZ - 1)

/* rrm block */
#define WL_PROXD_SID_RRM_START WL_PROXD_SID_EXT_BLK_START(1)
#define WL_PROXD_SID_RRM_END WL_PROXD_SID_EXT_BLK_END(WL_PROXD_SID_RRM_START)

/* nan block */
#define WL_PROXD_SID_NAN_START WL_PROXD_SID_EXT_BLK_START(2)
#define WL_PROXD_SID_NAN_END WL_PROXD_SID_EXT_BLK_END(WL_PROXD_SID_NAN_START)

/** maximum number sessions that can be allocated, may be less if tunable */
#define WL_PROXD_MAX_SESSIONS 16

typedef uint16 wl_proxd_session_id_t;

/** status - TBD BCME_ vs proxd status - range reserved for BCME_ */
enum {
	WL_PROXD_E_NOAVAIL		= -1056,
	WL_PROXD_E_EXT_SCHED		= -1055,
	WL_PROXD_E_NOT_BCM		= -1054,
	WL_PROXD_E_FRAME_TYPE		= -1053,
	WL_PROXD_E_VERNOSUPPORT		= -1052,
	WL_PROXD_E_SEC_NOKEY		= -1051,
	WL_PROXD_E_SEC_POLICY		= -1050,
	WL_PROXD_E_SCAN_INPROCESS	= -1049,
	WL_PROXD_E_BAD_PARTIAL_TSF	= -1048,
	WL_PROXD_E_SCANFAIL		= -1047,
	WL_PROXD_E_NOTSF		= -1046,
	WL_PROXD_E_POLICY		= -1045,
	WL_PROXD_E_INCOMPLETE		= -1044,
	WL_PROXD_E_OVERRIDDEN		= -1043,
	WL_PROXD_E_ASAP_FAILED		= -1042,
	WL_PROXD_E_NOTSTARTED		= -1041,
	WL_PROXD_E_INVALIDMEAS		= -1040,
	WL_PROXD_E_INCAPABLE		= -1039,
	WL_PROXD_E_MISMATCH		= -1038,
	WL_PROXD_E_DUP_SESSION		= -1037,
	WL_PROXD_E_REMOTE_FAIL		= -1036,
	WL_PROXD_E_REMOTE_INCAPABLE	= -1035,
	WL_PROXD_E_SCHED_FAIL		= -1034,
	WL_PROXD_E_PROTO		= -1033,
	WL_PROXD_E_EXPIRED		= -1032,
	WL_PROXD_E_TIMEOUT		= -1031,
	WL_PROXD_E_NOACK		= -1030,
	WL_PROXD_E_DEFERRED		= -1029,
	WL_PROXD_E_INVALID_SID		= -1028,
	WL_PROXD_E_REMOTE_CANCEL	= -1027,
	WL_PROXD_E_CANCELED		= -1026,	/**< local */
	WL_PROXD_E_INVALID_SESSION	= -1025,
	WL_PROXD_E_BAD_STATE		= -1024,
	WL_PROXD_E_START		= -1024,
	WL_PROXD_E_ERROR		= -1,
	WL_PROXD_E_OK			= 0
};
typedef int32 wl_proxd_status_t;

/* proxd errors from phy */
#define PROXD_TOF_INIT_ERR_BITS 16

enum {
	WL_PROXD_PHY_ERR_LB_CORR_THRESH    = (1 << 0), /* Loopback Correlation threshold */
	WL_PROXD_PHY_ERR_RX_CORR_THRESH    = (1 << 1), /* Received Correlation threshold */
	WL_PROXD_PHY_ERR_LB_PEAK_POWER     = (1 << 2), /* Loopback Peak power   */
	WL_PROXD_PHY_ERR_RX_PEAK_POWER     = (1 << 3), /* Received Peak power   */
	WL_PROXD_PHY_ERR_BITFLIP           = (1 << 4), /* Bitflips */
	WL_PROXD_PHY_ERR_SNR               = (1 << 5), /* SNR */
	WL_PROXD_PHY_RX_STRT_WIN_OFF       = (1 << 6), /* Receive start window is off */
	WL_PROXD_PHY_RX_END_WIN_OFF        = (1 << 7), /* Receive End window is off */
	WL_PROXD_PHY_ERR_LOW_CONFIDENCE    = (1 << 15), /* Low confidence on meas distance */
};
typedef uint32 wl_proxd_phy_error_t;

/** session states */
enum {
	WL_PROXD_SESSION_STATE_NONE			= 0,
	WL_PROXD_SESSION_STATE_CREATED			= 1,
	WL_PROXD_SESSION_STATE_CONFIGURED		= 2,
	WL_PROXD_SESSION_STATE_STARTED			= 3,
	WL_PROXD_SESSION_STATE_DELAY			= 4,
	WL_PROXD_SESSION_STATE_USER_WAIT		= 5,
	WL_PROXD_SESSION_STATE_SCHED_WAIT		= 6,
	WL_PROXD_SESSION_STATE_BURST			= 7,
	WL_PROXD_SESSION_STATE_STOPPING			= 8,
	WL_PROXD_SESSION_STATE_ENDED			= 9,
	WL_PROXD_SESSION_STATE_START_WAIT		= 10,
	WL_PROXD_SESSION_STATE_DESTROYING		= -1
};
typedef int16 wl_proxd_session_state_t;

/** RTT sample flags */
enum {
	WL_PROXD_RTT_SAMPLE_NONE	= 0x00,
	WL_PROXD_RTT_SAMPLE_DISCARD	= 0x01
};
typedef uint8 wl_proxd_rtt_sample_flags_t;
typedef int16 wl_proxd_rssi_t;
typedef uint16 wl_proxd_snr_t;
typedef uint16 wl_proxd_bitflips_t;

/** result flags */
enum {
	WL_PRXOD_RESULT_FLAG_NONE	= 0x0000,
	WL_PROXD_RESULT_FLAG_NLOS	= 0x0001,	/**< LOS - if available */
	WL_PROXD_RESULT_FLAG_LOS	= 0x0002,	/**< NLOS - if available */
	WL_PROXD_RESULT_FLAG_FATAL	= 0x0004,	/**< Fatal error during burst */
	WL_PROXD_RESULT_FLAG_VHTACK	= 0x0008,	/* VHTACK or Legacy ACK used */
	WL_PROXD_REQUEST_SENT		= 0x0010,	/* FTM request was sent */
	WL_PROXD_REQUEST_ACKED		= 0x0020,	/* FTM request was acked */
	WL_PROXD_LTFSEQ_STARTED		= 0x0040,	/* LTF sequence started */
	WL_PROXD_RESULT_FLAG_ALL	= 0xffff
};
typedef int16 wl_proxd_result_flags_t;

#define WL_PROXD_RTT_SAMPLE_VERSION_1	1
typedef struct wl_proxd_rtt_sample_v1 {
	uint8				id;		/**< id for the sample - non-zero */
	wl_proxd_rtt_sample_flags_t	flags;
	wl_proxd_rssi_t			rssi;
	wl_proxd_intvl_t		rtt;		/**< round trip time */
	uint32				ratespec;
	wl_proxd_snr_t                  snr;
	wl_proxd_bitflips_t             bitflips;
	wl_proxd_status_t               status;
	int32                           distance;
	wl_proxd_phy_error_t		tof_phy_error;
	wl_proxd_phy_error_t		tof_tgt_phy_error; /* target phy error bit map */
	wl_proxd_snr_t                  tof_tgt_snr;
	wl_proxd_bitflips_t             tof_tgt_bitflips;
	uint8                           coreid;
	uint8                           pad[3];
} wl_proxd_rtt_sample_v1_t;

#define WL_PROXD_RTT_RESULT_VERSION_1	1
/** rtt measurement result */
typedef struct wl_proxd_rtt_result_v1 {
	wl_proxd_session_id_t		sid;
	wl_proxd_result_flags_t		flags;
	wl_proxd_status_t		status;
	struct ether_addr		peer;
	wl_proxd_session_state_t	state;		/**< current state */
	union {
		wl_proxd_intvl_t	retry_after;	/* hint for errors */
		wl_proxd_intvl_t	burst_duration; /* burst duration */
	} u;
	wl_proxd_rtt_sample_v1_t	avg_rtt;
	uint32				avg_dist;	/* 1/256m units */
	uint16				sd_rtt;		/* RTT standard deviation */
	uint8				num_valid_rtt;	/* valid rtt cnt */
	uint8				num_ftm;	/* actual num of ftm cnt (Configured) */
	uint16				burst_num;	/* in a session */
	uint16				num_rtt;	/* 0 if no detail */
	uint16				num_meas;	/* number of ftm frames seen OTA */
	uint8                           pad[2];
	wl_proxd_rtt_sample_v1_t	rtt[1];		/* variable */
} wl_proxd_rtt_result_v1_t;

#define WL_PROXD_RTT_SAMPLE_VERSION_2	2
typedef struct wl_proxd_rtt_sample_v2 {
	uint16				version;
	uint16				length;
	uint8				id;		/**< id for the sample - non-zero */
	wl_proxd_rtt_sample_flags_t	flags;
	wl_proxd_rssi_t			rssi;
	wl_proxd_intvl_t		rtt;		/**< round trip time */
	uint32				ratespec;
	wl_proxd_snr_t                  snr;
	wl_proxd_bitflips_t             bitflips;
	wl_proxd_status_t               status;
	int32                           distance;
	wl_proxd_phy_error_t		tof_phy_error;
	wl_proxd_phy_error_t		tof_tgt_phy_error; /* target phy error bit map */
	wl_proxd_snr_t                  tof_tgt_snr;
	wl_proxd_bitflips_t             tof_tgt_bitflips;
	uint8                           coreid;
	uint8                           pad[3];
	uint32				chanspec;
} wl_proxd_rtt_sample_v2_t;

#define WL_PROXD_RTT_RESULT_VERSION_2	2
/** rtt measurement result */
typedef struct wl_proxd_rtt_result_v2 {
	uint16				version;
	uint16				length;		/* up to rtt[] */
	wl_proxd_session_id_t		sid;
	wl_proxd_result_flags_t		flags;
	wl_proxd_status_t		status;
	struct ether_addr		peer;
	wl_proxd_session_state_t	state;		/**< current state */
	union {
		wl_proxd_intvl_t	retry_after;	/* hint for errors */
		wl_proxd_intvl_t	burst_duration; /* burst duration */
	} u;
	uint32				avg_dist;	/* 1/256m units */
	uint16				sd_rtt;		/* RTT standard deviation */
	uint8				num_valid_rtt;	/* valid rtt cnt */
	uint8				num_ftm;	/* actual num of ftm cnt (Configured) */
	uint16				burst_num;	/* in a session */
	uint16				num_rtt;	/* 0 if no detail */
	uint16				num_meas;	/* number of ftm frames seen OTA */
	uint8                           pad[2];
	wl_proxd_rtt_sample_v2_t	rtt[1];		/* variable, first element is avg_rtt */
} wl_proxd_rtt_result_v2_t;

/** aoa measurement result */
typedef struct wl_proxd_aoa_result {
	wl_proxd_session_id_t		sid;
	wl_proxd_result_flags_t		flags;
	wl_proxd_status_t		status;
	struct ether_addr		peer;
	wl_proxd_session_state_t	state;
	uint16				burst_num;
	uint8				pad[2];
	/* wl_proxd_aoa_sample_t sample_avg; TBD */
} BWL_POST_PACKED_STRUCT wl_proxd_aoa_result_t;
#include <packed_section_end.h>

/** global stats */
typedef struct wl_proxd_counters {
	uint32 tx;			/* tx frame count */
	uint32 rx;			/* rx frame count */
	uint32 burst;			/* total number of burst */
	uint32 sessions;		/* total number of sessions */
	uint32 max_sessions;		/* max concurrency */
	uint32 sched_fail;		/* scheduling failures */
	uint32 timeouts;		/* timeouts */
	uint32 protoerr;		/* protocol errors */
	uint32 noack;			/* tx w/o ack */
	uint32 txfail;			/* any tx falure */
	uint32 lci_req_tx;		/* tx LCI requests */
	uint32 lci_req_rx;		/* rx LCI requests */
	uint32 lci_rep_tx;		/* tx LCI reports */
	uint32 lci_rep_rx;		/* rx LCI reports */
	uint32 civic_req_tx;		/* tx civic requests */
	uint32 civic_req_rx;		/* rx civic requests */
	uint32 civic_rep_tx;		/* tx civic reports */
	uint32 civic_rep_rx;		/* rx civic reports */
	uint32 rctx;			/* ranging contexts created */
	uint32 rctx_done;		/* count of ranging done */
	uint32 publish_err;		/* availability publishing errors */
	uint32 on_chan;			/* count of scheduler onchan */
	uint32 off_chan;		/* count of scheduler offchan */
	uint32 tsf_lo;			/* local tsf or session tsf */
	uint32 tsf_hi;
	uint32 num_meas;
} wl_proxd_counters_t;

typedef struct wl_proxd_counters wl_proxd_session_counters_t;

enum {
	WL_PROXD_CAP_NONE 		= 0x0000,
	WL_PROXD_CAP_ALL 		= 0xffff
};
typedef int16 wl_proxd_caps_t;

/** method capabilities */
enum {
	WL_PROXD_FTM_CAP_NONE = 0x0000,
	WL_PROXD_FTM_CAP_FTM1 = 0x0001
};
typedef uint16 wl_proxd_ftm_caps_t;

typedef struct wl_proxd_tlv_id_list {
	uint16			num_ids;
	uint16			ids[1];
} wl_proxd_tlv_id_list_t;

typedef struct wl_proxd_session_id_list {
	uint16 num_ids;
	wl_proxd_session_id_t ids[1];
} wl_proxd_session_id_list_t;

typedef struct wl_proxd_tpk {
	struct ether_addr	peer;
	uint8 tpk[TPK_FTM_LEN];
} wl_proxd_tpk_t;

/* tlvs returned for get_info on ftm method
 *	configuration:
 *	proxd flags
 *	event mask
 *	debug mask
 *	session defaults (session tlvs)
 * status tlv - not supported for ftm method
 * info tlv
 */
typedef struct wl_proxd_ftm_info {
	wl_proxd_ftm_caps_t caps;
	uint16 max_sessions;
	uint16 num_sessions;
	uint16 rx_max_burst;
} wl_proxd_ftm_info_t;

enum {
	WL_PROXD_WAIT_NONE	= 0x0000,
	WL_PROXD_WAIT_KEY	= 0x0001,
	WL_PROXD_WAIT_SCHED	= 0x0002,
	WL_PROXD_WAIT_TSF	= 0x0004
};
typedef int16 wl_proxd_wait_reason_t;

/* tlvs returned for get_info on session
 * session config (tlvs)
 * session info tlv
 */
typedef struct wl_proxd_ftm_session_info {
	uint16 sid;
	uint8 bss_index;
	uint8 pad;
	struct ether_addr bssid;
	wl_proxd_session_state_t state;
	wl_proxd_status_t status;
	uint16	burst_num;
	wl_proxd_wait_reason_t wait_reason;
	uint32	meas_start_lo; /* sn tsf of 1st meas for cur/prev burst */
	uint32	meas_start_hi;
} wl_proxd_ftm_session_info_t;

typedef struct wl_proxd_ftm_session_status {
	uint16 sid;
	wl_proxd_session_state_t state;
	wl_proxd_status_t status;
	uint16	burst_num;
	uint16	pad;
} wl_proxd_ftm_session_status_t;

/** rrm range request */
typedef struct wl_proxd_range_req {
	uint16 			num_repeat;
	uint16			init_delay_range;	/**< in TUs */
	uint8			pad;
	uint8			num_nbr;		/**< number of (possible) neighbors */
	nbr_element_t		nbr[1];
} wl_proxd_range_req_t;

#define WL_PROXD_LCI_LAT_OFF 	0
#define WL_PROXD_LCI_LONG_OFF 	5
#define WL_PROXD_LCI_ALT_OFF 	10

#define WL_PROXD_LCI_GET_LAT(_lci, _lat, _lat_err) { \
	unsigned _off = WL_PROXD_LCI_LAT_OFF; \
	_lat_err = (_lci)->data[(_off)] & 0x3f; \
	_lat = (_lci)->data[(_off)+1]; \
	_lat |= (_lci)->data[(_off)+2] << 8; \
	_lat |= (_lci)->data[_(_off)+3] << 16; \
	_lat |= (_lci)->data[(_off)+4] << 24; \
	_lat <<= 2; \
	_lat |= (_lci)->data[(_off)] >> 6; \
}

#define WL_PROXD_LCI_GET_LONG(_lci, _lcilong, _long_err) { \
	unsigned _off = WL_PROXD_LCI_LONG_OFF; \
	_long_err = (_lci)->data[(_off)] & 0x3f; \
	_lcilong = (_lci)->data[(_off)+1]; \
	_lcilong |= (_lci)->data[(_off)+2] << 8; \
	_lcilong |= (_lci)->data[_(_off)+3] << 16; \
	_lcilong |= (_lci)->data[(_off)+4] << 24; \
	__lcilong <<= 2; \
	_lcilong |= (_lci)->data[(_off)] >> 6; \
}

#define WL_PROXD_LCI_GET_ALT(_lci, _alt_type, _alt, _alt_err) { \
	unsigned _off = WL_PROXD_LCI_ALT_OFF; \
	_alt_type = (_lci)->data[_off] & 0x0f; \
	_alt_err = (_lci)->data[(_off)] >> 4; \
	_alt_err |= ((_lci)->data[(_off)+1] & 0x03) << 4; \
	_alt = (_lci)->data[(_off)+2]; \
	_alt |= (_lci)->data[(_off)+3] << 8; \
	_alt |= (_lci)->data[_(_off)+4] << 16; \
	_alt <<= 6; \
	_alt |= (_lci)->data[(_off) + 1] >> 2; \
}

#define WL_PROXD_LCI_VERSION(_lci) ((_lci)->data[15] >> 6)

/* availability. advertising mechanism bss specific */
/** availablity flags */
enum {
	WL_PROXD_AVAIL_NONE = 0,
	WL_PROXD_AVAIL_NAN_PUBLISHED = 0x0001,
	WL_PROXD_AVAIL_SCHEDULED = 0x0002        /**< scheduled by proxd */
};
typedef int16 wl_proxd_avail_flags_t;

/** time reference */
enum {
	WL_PROXD_TREF_NONE = 0,
	WL_PROXD_TREF_DEV_TSF = 1,
	WL_PROXD_TREF_NAN_DW = 2,
	WL_PROXD_TREF_TBTT = 3,
	WL_PROXD_TREF_MAX		/* last entry */
};
typedef int16 wl_proxd_time_ref_t;

/** proxd channel-time slot */
typedef struct {
	wl_proxd_intvl_t start;         /**< from ref */
	wl_proxd_intvl_t duration;      /**< from start */
	uint32  chanspec;
} wl_proxd_time_slot_t;

typedef struct wl_proxd_avail24 {
	wl_proxd_avail_flags_t flags; /**< for query only */
	wl_proxd_time_ref_t time_ref;
	uint16	max_slots; /**< for query only */
	uint16  num_slots;
	wl_proxd_time_slot_t slots[1];	/**< ROM compat - not used */
	wl_proxd_intvl_t 	repeat;
	wl_proxd_time_slot_t ts0[1];
} wl_proxd_avail24_t;
#define WL_PROXD_AVAIL24_TIMESLOT(_avail24, _i) (&(_avail24)->ts0[(_i)])
#define WL_PROXD_AVAIL24_TIMESLOT_OFFSET(_avail24) OFFSETOF(wl_proxd_avail24_t, ts0)
#define WL_PROXD_AVAIL24_TIMESLOTS(_avail24) WL_PROXD_AVAIL24_TIMESLOT(_avail24, 0)
#define WL_PROXD_AVAIL24_SIZE(_avail24, _num_slots) (\
	WL_PROXD_AVAIL24_TIMESLOT_OFFSET(_avail24) + \
	(_num_slots) * sizeof(*WL_PROXD_AVAIL24_TIMESLOT(_avail24, 0)))

typedef struct wl_proxd_avail {
	wl_proxd_avail_flags_t flags; /**< for query only */
	wl_proxd_time_ref_t time_ref;
	uint16	max_slots; /**< for query only */
	uint16  num_slots;
	wl_proxd_intvl_t 	repeat;
	wl_proxd_time_slot_t slots[1];
} wl_proxd_avail_t;
#define WL_PROXD_AVAIL_TIMESLOT(_avail, _i) (&(_avail)->slots[(_i)])
#define WL_PROXD_AVAIL_TIMESLOT_OFFSET(_avail) OFFSETOF(wl_proxd_avail_t, slots)

#define WL_PROXD_AVAIL_TIMESLOTS(_avail) WL_PROXD_AVAIL_TIMESLOT(_avail, 0)
#define WL_PROXD_AVAIL_SIZE(_avail, _num_slots) (\
	WL_PROXD_AVAIL_TIMESLOT_OFFSET(_avail) + \
	(_num_slots) * sizeof(*WL_PROXD_AVAIL_TIMESLOT(_avail, 0)))

/* collect support TBD */

/** debugging */
enum {
	WL_PROXD_DEBUG_NONE		= 0x00000000,
	WL_PROXD_DEBUG_LOG		= 0x00000001,
	WL_PROXD_DEBUG_IOV		= 0x00000002,
	WL_PROXD_DEBUG_EVENT		= 0x00000004,
	WL_PROXD_DEBUG_SESSION		= 0x00000008,
	WL_PROXD_DEBUG_PROTO		= 0x00000010,
	WL_PROXD_DEBUG_SCHED		= 0x00000020,
	WL_PROXD_DEBUG_RANGING		= 0x00000040,
	WL_PROXD_DEBUG_NAN		= 0x00000080,
	WL_PROXD_DEBUG_PKT		= 0x00000100,
	WL_PROXD_DEBUG_SEC		= 0x00000200,
	WL_PROXD_DEBUG_EVENTLOG		= 0x80000000,	/* map/enable EVNET_LOG_TAG_PROXD_INFO */
	WL_PROXD_DEBUG_ALL		= 0xffffffff
};
typedef uint32 wl_proxd_debug_mask_t;

/** tlv IDs - data length 4 bytes unless overridden by type, alignment 32 bits */
typedef enum {
	WL_PROXD_TLV_ID_NONE			= 0,
	WL_PROXD_TLV_ID_METHOD			= 1,
	WL_PROXD_TLV_ID_FLAGS			= 2,
	WL_PROXD_TLV_ID_CHANSPEC		= 3,	/**< note: uint32 */
	WL_PROXD_TLV_ID_TX_POWER		= 4,
	WL_PROXD_TLV_ID_RATESPEC		= 5,
	WL_PROXD_TLV_ID_BURST_DURATION		= 6,	/**< intvl - length of burst */
	WL_PROXD_TLV_ID_BURST_PERIOD		= 7,	/**< intvl - between bursts */
	WL_PROXD_TLV_ID_BURST_FTM_SEP		= 8,	/**< intvl - between FTMs */
	WL_PROXD_TLV_ID_BURST_NUM_FTM		= 9,	/**< uint16 - per burst */
	WL_PROXD_TLV_ID_NUM_BURST		= 10,	/**< uint16 */
	WL_PROXD_TLV_ID_FTM_RETRIES		= 11,	/**< uint16 at FTM level */
	WL_PROXD_TLV_ID_BSS_INDEX		= 12,	/**< uint8 */
	WL_PROXD_TLV_ID_BSSID			= 13,
	WL_PROXD_TLV_ID_INIT_DELAY		= 14,	/**< intvl - optional,non-standalone only */
	WL_PROXD_TLV_ID_BURST_TIMEOUT		= 15,	/**< expect response within - intvl */
	WL_PROXD_TLV_ID_EVENT_MASK		= 16,	/**< interested events - in/out */
	WL_PROXD_TLV_ID_FLAGS_MASK		= 17,	/**< interested flags - in only */
	WL_PROXD_TLV_ID_PEER_MAC		= 18,	/**< mac address of peer */
	WL_PROXD_TLV_ID_FTM_REQ			= 19,	/**< dot11_ftm_req */
	WL_PROXD_TLV_ID_LCI_REQ			= 20,
	WL_PROXD_TLV_ID_LCI			= 21,
	WL_PROXD_TLV_ID_CIVIC_REQ		= 22,
	WL_PROXD_TLV_ID_CIVIC			= 23,
	WL_PROXD_TLV_ID_AVAIL24			= 24,	/**< ROM compatibility */
	WL_PROXD_TLV_ID_SESSION_FLAGS		= 25,
	WL_PROXD_TLV_ID_SESSION_FLAGS_MASK	= 26,	/**< in only */
	WL_PROXD_TLV_ID_RX_MAX_BURST		= 27,	/**< uint16 - limit bursts per session */
	WL_PROXD_TLV_ID_RANGING_INFO		= 28,	/**< ranging info */
	WL_PROXD_TLV_ID_RANGING_FLAGS		= 29,	/**< uint16 */
	WL_PROXD_TLV_ID_RANGING_FLAGS_MASK	= 30,	/**< uint16, in only */
	WL_PROXD_TLV_ID_NAN_MAP_ID		= 31,
	WL_PROXD_TLV_ID_DEV_ADDR		= 32,
	WL_PROXD_TLV_ID_AVAIL			= 33,	/**< wl_proxd_avail_t  */
	WL_PROXD_TLV_ID_TLV_ID			= 34,	/* uint16 tlv-id */
	WL_PROXD_TLV_ID_FTM_REQ_RETRIES		= 35,	/* uint16 FTM request retries */
	WL_PROXD_TLV_ID_TPK			= 36,	/* 32byte TPK  */
	WL_PROXD_TLV_ID_RI_RR			= 36,	/* RI_RR */
	WL_PROXD_TLV_ID_TUNE			= 37,	/* wl_proxd_pararms_tof_tune_t */
	WL_PROXD_TLV_ID_CUR_ETHER_ADDR		= 38,	/* Source Address used for Tx */

	/* output - 512 + x */
	WL_PROXD_TLV_ID_STATUS			= 512,
	WL_PROXD_TLV_ID_COUNTERS		= 513,
	WL_PROXD_TLV_ID_INFO			= 514,
	WL_PROXD_TLV_ID_RTT_RESULT		= 515,
	WL_PROXD_TLV_ID_AOA_RESULT		= 516,
	WL_PROXD_TLV_ID_SESSION_INFO		= 517,
	WL_PROXD_TLV_ID_SESSION_STATUS		= 518,
	WL_PROXD_TLV_ID_SESSION_ID_LIST		= 519,
	WL_PROXD_TLV_ID_RTT_RESULT_V2		= 520,

	/* debug tlvs can be added starting 1024 */
	WL_PROXD_TLV_ID_DEBUG_MASK		= 1024,
	WL_PROXD_TLV_ID_COLLECT			= 1025,	/**< output only */
	WL_PROXD_TLV_ID_STRBUF			= 1026,

	WL_PROXD_TLV_ID_COLLECT_HEADER		= 1025,	/* wl_proxd_collect_header_t */
	WL_PROXD_TLV_ID_COLLECT_INFO		= 1028,	/* wl_proxd_collect_info_t */
	WL_PROXD_TLV_ID_COLLECT_DATA		= 1029,	/* wl_proxd_collect_data_t */
	WL_PROXD_TLV_ID_COLLECT_CHAN_DATA	= 1030,	/* wl_proxd_collect_data_t */
	WL_PROXD_TLV_ID_MF_STATS_DATA		= 1031,	/* mf_stats_buffer */

	WL_PROXD_TLV_ID_COLLECT_INLINE_HEADER	= 1032,
	WL_PROXD_TLV_ID_COLLECT_INLINE_FRAME_INFO	= 1033,
	WL_PROXD_TLV_ID_COLLECT_INLINE_FRAME_DATA	= 1034,
	WL_PROXD_TLV_ID_COLLECT_INLINE_RESULTS	= 1035,

	WL_PROXD_TLV_ID_MAX
} wl_proxd_tlv_types_t;

#define TOF_COLLECT_INLINE_HEADER_INFO_VER_1	1

typedef struct wl_proxd_collect_inline_header_info_v1
{
	uint16			version;
	uint16			pad1;
	uint32			ratespec;		/* override */
	chanspec_t		chanspec;
	uint16			num_ftm;
	struct ether_addr	peer_mac;
	struct ether_addr	cur_ether_addr;		/* source address for Tx */
} wl_proxd_collect_inline_header_info_v1_t;

#define TOF_COLLECT_INLINE_RESULTS_VER_1		1
typedef struct wl_proxd_collect_inline_results_info_v1
{
	uint16 version;
	uint16 pad1;
	uint32 meanrtt;
	uint32 distance;
	uint16 num_rtt;
	uint16 pad2;
	int32 status;
	uint32 ratespec;
} wl_proxd_collect_inline_results_info_v1_t;

#define TOF_COLLECT_INLINE_FRAME_INFO_VER_1	1
typedef struct wl_proxd_collect_inline_frame_info_v1
{
	uint16 version;
	uint16 pad1;
	int32 gd;
	uint32 T[4];
	uint32 prev_t1;
	uint32 prev_t4;
	int32 hadj;
	int8 rssi;
	uint8 pad[3];
} wl_proxd_collect_inline_frame_info_v1_t;

#define TOF_COLLECT_INLINE_FRAME_INFO_VER_2	2
typedef struct wl_proxd_collect_inline_frame_info_v2
{
	uint16 version;
	uint16 pad1;
	int32 gd;
	uint32 T[4];
	int32 hadj;
	int8 rssi;
	uint8 pad[3];
} wl_proxd_collect_inline_frame_info_v2_t;

typedef struct wl_proxd_tlv {
	uint16 id;
	uint16 len;
	uint8  data[1];
} wl_proxd_tlv_t;

/** proxd iovar - applies to proxd, method or session */
typedef struct wl_proxd_iov {
	uint16                  version;
	uint16                  len;
	wl_proxd_cmd_t          cmd;
	wl_proxd_method_t       method;
	wl_proxd_session_id_t   sid;
	uint8                   PAD[2];
	wl_proxd_tlv_t          tlvs[1];	/**< variable */
} wl_proxd_iov_t;

#define WL_PROXD_IOV_HDR_SIZE OFFSETOF(wl_proxd_iov_t, tlvs)

/* The following event definitions may move to bcmevent.h, but sharing proxd types
 * across needs more invasive changes unrelated to proxd
 */
enum {
	WL_PROXD_EVENT_NONE			= 0,	/**< not an event, reserved */
	WL_PROXD_EVENT_SESSION_CREATE		= 1,
	WL_PROXD_EVENT_SESSION_START		= 2,
	WL_PROXD_EVENT_FTM_REQ			= 3,
	WL_PROXD_EVENT_BURST_START		= 4,
	WL_PROXD_EVENT_BURST_END		= 5,
	WL_PROXD_EVENT_SESSION_END		= 6,
	WL_PROXD_EVENT_SESSION_RESTART		= 7,
	WL_PROXD_EVENT_BURST_RESCHED		= 8,	/**< burst rescheduled-e.g. partial TSF */
	WL_PROXD_EVENT_SESSION_DESTROY		= 9,
	WL_PROXD_EVENT_RANGE_REQ		= 10,
	WL_PROXD_EVENT_FTM_FRAME		= 11,
	WL_PROXD_EVENT_DELAY			= 12,
	WL_PROXD_EVENT_VS_INITIATOR_RPT		= 13,	/**< (target) rx initiator-report */
	WL_PROXD_EVENT_RANGING			= 14,
	WL_PROXD_EVENT_LCI_MEAS_REP		= 15,	/* LCI measurement report */
	WL_PROXD_EVENT_CIVIC_MEAS_REP		= 16,	/* civic measurement report */
	WL_PROXD_EVENT_COLLECT			= 17,
	WL_PROXD_EVENT_START_WAIT		= 18,	/* waiting to start */
	WL_PROXD_EVENT_MF_STATS			= 19,	/* mf stats event */

	WL_PROXD_EVENT_MAX
};
typedef int16 wl_proxd_event_type_t;

/** proxd event mask - upto 32 events for now */
typedef uint32 wl_proxd_event_mask_t;

#define WL_PROXD_EVENT_MASK_ALL 0xfffffffe
#define WL_PROXD_EVENT_MASK_EVENT(_event_type) (1 << (_event_type))
#define WL_PROXD_EVENT_ENABLED(_mask, _event_type) (\
	((_mask) & WL_PROXD_EVENT_MASK_EVENT(_event_type)) != 0)

/** proxd event - applies to proxd, method or session */
typedef struct wl_proxd_event {
	uint16			version;
	uint16			len;
	wl_proxd_event_type_t	type;
	wl_proxd_method_t	method;
	wl_proxd_session_id_t	sid;
	uint8			pad[2];		/* This field is used fragmentation purpose */
	wl_proxd_tlv_t		tlvs[1];	/**< variable */
} wl_proxd_event_t;

enum {
	WL_PROXD_RANGING_STATE_NONE = 0,
	WL_PROXD_RANGING_STATE_NOTSTARTED = 1,
	WL_PROXD_RANGING_STATE_INPROGRESS = 2,
	WL_PROXD_RANGING_STATE_DONE = 3
};
typedef int16 wl_proxd_ranging_state_t;

/** proxd ranging flags */
enum {
	WL_PROXD_RANGING_FLAG_NONE = 0x0000,  /**< no flags */
	WL_PROXD_RANGING_FLAG_DEL_SESSIONS_ON_STOP = 0x0001,
	WL_PROXD_RANGING_FLAG_ALL = 0xffff
};
typedef uint16 wl_proxd_ranging_flags_t;

struct wl_proxd_ranging_info {
	wl_proxd_status_t   status;
	wl_proxd_ranging_state_t state;
	wl_proxd_ranging_flags_t flags;
	uint16	num_sids;
	uint16	num_done;
};
typedef struct wl_proxd_ranging_info wl_proxd_ranging_info_t;

#include <packed_section_start.h>
/* Legacy platform i.e. 43342/43430 */
#define WL_PROXD_COLLECT_EVENT_DATA_VERSION_1		1
typedef BWL_PRE_PACKED_STRUCT struct wl_proxd_collect_event_data_v1 {
	uint32                  H_LB[K_TOF_COLLECT_H_SIZE_20MHZ];
	uint32                  H_RX[K_TOF_COLLECT_H_SIZE_20MHZ];
	uint8                   ri_rr[FTM_TPK_LEN];
	wl_proxd_phy_error_t    phy_err_mask;
} BWL_POST_PACKED_STRUCT wl_proxd_collect_event_data_v1_t;

/* Secured 2.0 supoorted devices i.e. 4364 */
#define WL_PROXD_COLLECT_EVENT_DATA_VERSION_2		2
typedef BWL_PRE_PACKED_STRUCT struct wl_proxd_collect_event_data_v2 {
	uint32                  H_LB[K_TOF_COLLECT_H_SIZE_20MHZ];
	uint32                  H_RX[K_TOF_COLLECT_H_SIZE_20MHZ];
	uint8                   ri_rr[FTM_TPK_RI_RR_LEN_SECURE_2_0];
	wl_proxd_phy_error_t    phy_err_mask;
} BWL_POST_PACKED_STRUCT wl_proxd_collect_event_data_v2_t;
#include <packed_section_end.h>

#define WL_PROXD_COLLECT_EVENT_DATA_VERSION_3		3
typedef struct wl_proxd_collect_event_data_v3 {
	uint16			version;
	uint16			length;
	uint32                  H_LB[K_TOF_COLLECT_H_SIZE_20MHZ];
	uint32                  H_RX[K_TOF_COLLECT_H_SIZE_20MHZ];
	uint8                   ri_rr[FTM_TPK_RI_RR_LEN_SECURE_2_0];
	wl_proxd_phy_error_t    phy_err_mask;
} wl_proxd_collect_event_data_v3_t;

#define WL_PROXD_COLLECT_EVENT_DATA_VERSION_MAX	WL_PROXD_COLLECT_EVENT_DATA_VERSION_3

/** Data returned by the bssload_report iovar. This is also the WLC_E_BSS_LOAD event data */
#include <packed_section_start.h>
typedef BWL_PRE_PACKED_STRUCT struct wl_bssload {
	uint16 sta_count;		/**< station count */
	uint16 aac;			/**< available admission capacity */
	uint8 chan_util;		/**< channel utilization */
} BWL_POST_PACKED_STRUCT wl_bssload_t;
#include <packed_section_end.h>

/**
 * Maximum number of configurable BSS Load levels.  The number of BSS Load
 * ranges is always 1 more than the number of configured levels.  eg. if
 * 3 levels of 10, 20, 30 are configured then this defines 4 load ranges:
 * 0-10, 11-20, 21-30, 31-255.  A WLC_E_BSS_LOAD event is generated each time
 * the utilization level crosses into another range, subject to the rate limit.
 */
#define MAX_BSSLOAD_LEVELS 8
#define MAX_BSSLOAD_RANGES (MAX_BSSLOAD_LEVELS + 1)

/** BSS Load event notification configuration. */
typedef struct wl_bssload_cfg {
	uint32 rate_limit_msec;	/**< # of events posted to application will be limited to
				 * one per specified period (0 to disable rate limit).
				 */
	uint8 num_util_levels;	/**< Number of entries in util_levels[] below */
	uint8 util_levels[MAX_BSSLOAD_LEVELS];
				/**< Variable number of BSS Load utilization levels in
				 * low to high order.  An event will be posted each time
				 * a received beacon's BSS Load IE channel utilization
				 * value crosses a level.
				 */
	uint8 PAD[3];
} wl_bssload_cfg_t;

/** Multiple roaming profile suport */
#define WL_MAX_ROAM_PROF_BRACKETS	4

#define WL_ROAM_PROF_VER_0	0
#define WL_ROAM_PROF_VER_1	1
#define WL_MAX_ROAM_PROF_VER	WL_ROAM_PROF_VER_1

#define WL_ROAM_PROF_NONE	(0 << 0)
#define WL_ROAM_PROF_LAZY	(1 << 0)
#define WL_ROAM_PROF_NO_CI	(1 << 1)
#define WL_ROAM_PROF_SUSPEND	(1 << 2)
#define WL_ROAM_PROF_SYNC_DTIM	(1 << 6)
#define WL_ROAM_PROF_DEFAULT	(1 << 7)	/**< backward compatible single default profile */

#define WL_FACTOR_TABLE_MAX_LIMIT 5

#define WL_CU_2G_ROAM_TRIGGER (-60)
#define WL_CU_5G_ROAM_TRIGGER (-70)

#define WL_CU_SCORE_DELTA_DEFAULT 20

#define WL_MAX_CHANNEL_USAGE 0x0FF
#define WL_CU_PERCENTAGE_DISABLE 0
#define WL_CU_PERCENTAGE_DEFAULT 70
#define WL_CU_PERCENTAGE_MAX 100
#define WL_CU_CALC_DURATION_DEFAULT 10 /* seconds */
#define WL_CU_CALC_DURATION_MAX 60 /* seconds */

typedef struct wl_roam_prof_v2 {
	int8	roam_flags;		/**< bit flags */
	int8	roam_trigger;		/**< RSSI trigger level per profile/RSSI bracket */
	int8	rssi_lower;
	int8	roam_delta;

	/* if channel_usage if zero, roam_delta is rssi delta required for new AP */
	/* if channel_usage if non-zero, roam_delta is score delta(%) required for new AP */
	int8	rssi_boost_thresh;	/**< Min RSSI to qualify for RSSI boost */
	int8	rssi_boost_delta;	/**< RSSI boost for AP in the other band */
	uint16	nfscan;			/**< number of full scan to start with */
	uint16	fullscan_period;
	uint16	init_scan_period;
	uint16	backoff_multiplier;
	uint16	max_scan_period;
	uint8	channel_usage;
	uint8	cu_avg_calc_dur;
	uint8	pad[2];
} wl_roam_prof_v2_t;

typedef struct wl_roam_prof_v1 {
	int8	roam_flags;		/**< bit flags */
	int8	roam_trigger;		/**< RSSI trigger level per profile/RSSI bracket */
	int8	rssi_lower;
	int8	roam_delta;

	/* if channel_usage if zero, roam_delta is rssi delta required for new AP */
	/* if channel_usage if non-zero, roam_delta is score delta(%) required for new AP */
	int8	rssi_boost_thresh;	/**< Min RSSI to qualify for RSSI boost */
	int8	rssi_boost_delta;	/**< RSSI boost for AP in the other band */
	uint16	nfscan;			/**< number of full scan to start with */
	uint16	fullscan_period;
	uint16	init_scan_period;
	uint16	backoff_multiplier;
	uint16	max_scan_period;
} wl_roam_prof_v1_t;

typedef struct wl_roam_prof_band_v2 {
	uint32	band;			/**< Must be just one band */
	uint16	ver;			/**< version of this struct */
	uint16	len;			/**< length in bytes of this structure */
	wl_roam_prof_v2_t roam_prof[WL_MAX_ROAM_PROF_BRACKETS];
} wl_roam_prof_band_v2_t;

typedef struct wl_roam_prof_band_v1 {
	uint32	band;			/**< Must be just one band */
	uint16	ver;			/**< version of this struct */
	uint16	len;			/**< length in bytes of this structure */
	wl_roam_prof_v1_t roam_prof[WL_MAX_ROAM_PROF_BRACKETS];
} wl_roam_prof_band_v1_t;

#define BSS_MAXTABLE_SIZE 10
#define WNM_BSS_SELECT_FACTOR_VERSION   1
typedef struct wnm_bss_select_factor_params {
	uint8 low;
	uint8 high;
	uint8 factor;
	uint8 pad;
} wnm_bss_select_factor_params_t;

#define WNM_BSS_SELECT_FIXED_SIZE OFFSETOF(wnm_bss_select_factor_cfg_t, params)
typedef struct wnm_bss_select_factor_cfg {
	uint8 version;
	uint8 band;
	uint16 type;
	uint16 pad;
	uint16 count;
	wnm_bss_select_factor_params_t params[1];
} wnm_bss_select_factor_cfg_t;

#define WNM_BSS_SELECT_WEIGHT_VERSION   1
typedef struct wnm_bss_select_weight_cfg {
	uint8 version;
	uint8 band;
	uint16 type;
	uint16 weight; /* weightage for each type between 0 to 100 */
} wnm_bss_select_weight_cfg_t;

#define WNM_BSS_SELECT_TYPE_RSSI   0
#define WNM_BSS_SELECT_TYPE_CU   1

#define WNM_BSSLOAD_MONITOR_VERSION   1
typedef struct wnm_bssload_monitor_cfg {
	uint8 version;
	uint8 band;
	uint8 duration; /* duration between 1 to 20sec */
} wnm_bssload_monitor_cfg_t;

#define WNM_ROAM_TRIGGER_VERSION   1
typedef struct wnm_roam_trigger_cfg {
	uint8 version;
	uint8 band;
	uint16 type;
	int16 trigger; /* trigger for each type in new roam algorithm */
} wnm_roam_trigger_cfg_t;

/* Data structures for Interface Create/Remove  */

#define WL_INTERFACE_CREATE_VER_1	1
#define WL_INTERFACE_CREATE_VER_2	2
#define WL_INTERFACE_CREATE_VER_3	3

/*
 * The flags filed of the wl_interface_create is designed to be
 * a Bit Mask. As of now only Bit 0 and Bit 1 are used as mentioned below.
 * The rest of the bits can be used, incase we have to provide
 * more information to the dongle
 */

/*
 * Bit 0 of flags field is used to inform whether the interface requested to
 * be created is STA or AP.
 * 0 - Create a STA interface
 * 1 - Create an AP interface
 * NOTE: This Bit 0 is applicable for the WL_INTERFACE_CREATE_VER < 2
 */
#define WL_INTERFACE_CREATE_STA	(0 << 0)
#define WL_INTERFACE_CREATE_AP	(1 << 0)

/*
 * From revision >= 2 Bit 0 of flags field will not used be for STA or AP interface creation.
 * "iftype" field shall be used for identifying the interface type.
 */
typedef enum wl_interface_type {
	WL_INTERFACE_TYPE_STA = 0,
	WL_INTERFACE_TYPE_AP = 1,
	WL_INTERFACE_TYPE_AWDL = 2,
	WL_INTERFACE_TYPE_NAN = 3,
	WL_INTERFACE_TYPE_P2P_GO = 4,
	WL_INTERFACE_TYPE_P2P_GC = 5,
	WL_INTERFACE_TYPE_P2P_DISC = 6,
	WL_INTERFACE_TYPE_IBSS = 7,
	WL_INTERFACE_TYPE_WDS = 8,
	WL_INTERFACE_TYPE_MAX
} wl_interface_type_t;

/*
 * Bit 1 of flags field is used to inform whether MAC is present in the
 * data structure or not.
 * 0 - Ignore mac_addr field
 * 1 - Use the mac_addr field
 */
#define WL_INTERFACE_MAC_DONT_USE	(0 << 1)
#define WL_INTERFACE_MAC_USE		(1 << 1)

/*
 * Bit 2 of flags field is used to inform whether core or wlc index
 * is present in the data structure or not.
 * 0 - Ignore wlc_index field
 * 1 - Use the wlc_index field
 */
#define WL_INTERFACE_WLC_INDEX_DONT_USE	(0 << 2)
#define WL_INTERFACE_WLC_INDEX_USE	(1 << 2)

/*
 * Bit 3 of flags field is used to create interface on the host requested interface index
 * 0 - Ignore if_index field
 * 1 - Use the if_index field
 */
#define WL_INTERFACE_IF_INDEX_USE       (1 << 3)

/*
 * Bit 4 of flags field is used to assign BSSID
 * 0 - Ignore bssid field
 * 1 - Use the bssid field
 */
#define WL_INTERFACE_BSSID_INDEX_USE	(1 << 4)

typedef struct wl_interface_create {
	uint16  ver;                    /**< version of this struct */
	uint8   pad1[2];                /**< Padding bytes */
	uint32  flags;                  /**< flags that defines the operation */
	struct  ether_addr   mac_addr;  /**< Optional Mac address */
	uint8   pad2[2];                /**< Padding bytes */
	uint32  wlc_index;              /**< Optional wlc index */
} wl_interface_create_v1_t;

typedef struct wl_interface_create_v2 {
	uint16  ver;                    /**< version of this struct */
	uint8   pad1[2];                /**< Padding bytes */
	uint32  flags;                  /**< flags that defines the operation */
	struct  ether_addr   mac_addr;  /**< Optional Mac address */
	uint8   iftype;                 /**< Type of interface created */
	uint8   pad2;                   /**< Padding bytes */
	uint32  wlc_index;              /**< Optional wlc index */
} wl_interface_create_v2_t;

typedef struct wl_interface_create_v3 {
	uint16	ver;			/**< version of this struct */
	uint16	len;			/**< length of whole structure including variable length */
	uint16	fixed_len;		/**< Fixed length of this structure excluding data[] */
	uint8	iftype;			/**< Type of interface created */
	uint8	wlc_index;		/**< Optional wlc index */
	uint32  flags;			/**< flags that defines the operation */
	struct	ether_addr   mac_addr;	/**< Optional Mac address */
	struct	ether_addr   bssid;	/**< Optional BSSID */
	uint8	if_index;		/**< interface index requested by Host */
	uint8	pad[3];			/**< Padding bytes to ensure data[] is at 32 bit aligned */
	uint8	data[];			/**< Optional application/Module specific data */
} wl_interface_create_v3_t;

#define WL_INTERFACE_INFO_VER_1		1
#define WL_INTERFACE_INFO_VER_2		2

typedef struct wl_interface_info_v1 {
	uint16  ver;                    /**< version of this struct */
	struct ether_addr    mac_addr;  /**< MAC address of the interface */
	char    ifname[BCM_MSG_IFNAME_MAX]; /**< name of interface */
	uint8   bsscfgidx;              /**< source bsscfg index */
	uint8	PAD;
} wl_interface_info_v1_t;

typedef struct wl_interface_info_v2 {
	uint16                  ver;                    /**< version of this struct */
	uint16                  length;                 /**< length of the whole structure */
	struct  ether_addr      mac_addr;               /**< MAC address of the interface */
	uint8                   bsscfgidx;              /**< source bsscfg index */
	uint8                   if_index;               /**< Interface index allocated by FW */
	char                    ifname[BCM_MSG_IFNAME_MAX]; /**< name of interface */
} wl_interface_info_v2_t;

#define PHY_RXIQEST_AVERAGING_DELAY 10

typedef struct wl_iqest_params {
	uint32 rxiq;
	uint8 niter;
	uint8 delay;
	uint8 PAD[2];
} wl_iqest_params_t;

typedef struct wl_iqest_sweep_params {
	wl_iqest_params_t params;
	uint8 nchannels;
	uint8 channel[3];	/** variable */
} wl_iqest_sweep_params_t;

typedef struct wl_iqest_value {
	uint8 channel;
	uint8 PAD[3];
	uint32 rxiq;
} wl_iqest_value_t;

typedef struct wl_iqest_result {
	uint8 nvalues;
	uint8 PAD[3];
	wl_iqest_value_t value[1];
} wl_iqest_result_t;

#ifndef LINUX_POSTMOGRIFY_REMOVAL
/* BTCX AIBSS (Oxygen) Status */
typedef struct wlc_btc_aibss_info {
	uint32	prev_tsf_l;		// Lower 32 bits of last read of TSF
	uint32	prev_tsf_h;		// Higher 32 bits of last read of TSF
	uint32	last_btinfo;		// Last read of BT info
	uint32	local_btinfo;		// Local BT INFO BitMap
	uint8	bt_out_of_sync_cnt;	// BT not in sync with strobe
	uint8	esco_off_cnt;		// Count incremented when ESCO is off
	uint8	strobe_enabled;		// Set only in AIBSS mode
	uint8	strobe_on;		// strobe to BT is on for Oxygen
	uint8	local_bt_in_sync;	// Sync status of local BT when strobe is on
	uint8	other_bt_in_sync;	// Sync state of BT in other devices in AIBSS
	uint8	local_bt_is_master;	// Local BT is master
	uint8	sco_prot_on;		// eSCO Protection on in local device
	uint8	other_esco_present;	// eSCO status in other devices in AIBSS
	uint8	rx_agg_change;		// Indicates Rx Agg size needs to change
	uint8	rx_agg_modified;	// Rx Agg size modified
	uint8	acl_grant_set;		// ACL grants on for speeding up sync
	uint8	write_ie_err_cnt;	// BTCX Ie write error cnt
	uint8	parse_ie_err_cnt;	// BTCX IE parse error cnt
	uint8	wci2_fail_cnt;		// WCI2 init failure cnt
	uint8	strobe_enable_err_cnt;	// Strobe enable err cnt
	uint8	strobe_init_err_cnt;	// Strobe init err cnt
	uint8	tsf_jump_cnt;		// TSF jump cnt
	uint8	acl_grant_cnt;		// ALC grant cnt
	uint8	pad1;
	uint16	ibss_tsf_shm;		// SHM address of strobe TSF
	uint16	pad2;
} wlc_btc_aibss_info_t;

#define WLC_BTC_AIBSS_STATUS_VER	1
#define WLC_BTC_AIBSS_STATUS_LEN	(sizeof(wlc_btc_aibss_status_t) - 2 * (sizeof(uint16)))

typedef struct wlc_btc_aibss_status {
	uint16	version;		// Version #
	uint16	len;			// Length of the structure(excluding len & version)
	int32	mode;			// Current value of btc_mode
	uint16	bth_period;             // bt coex period. read from shm.
	uint16	agg_off_bm;		// AGG OFF BM read from SHM
	uint8	bth_active;             // bt active session
	uint8	pad[3];
	wlc_btc_aibss_info_t aibss_info;	// Structure definition above
} wlc_btc_aibss_status_t;
#endif /* LINUX_POSTMOGRIFY_REMOVAL */

typedef enum {
	STATE_NONE = 0,

	/* WLAN -> BT */
	W2B_DATA_SET = 21,
	B2W_ACK_SET = 22,
	W2B_DATA_CLEAR = 23,
	B2W_ACK_CLEAR = 24,

	/* BT -> WLAN */
	B2W_DATA_SET = 31,
	W2B_ACK_SET = 32,
	B2W_DATA_CLEAR = 33,
	W2B_ACK_CLEAR = 34
} bwte_gci_intstate_t;

#define WL_BWTE_STATS_VERSION 1 /* version of bwte_stats_t */
typedef struct {
	uint32 version;

	bwte_gci_intstate_t inttobt;
	bwte_gci_intstate_t intfrombt;

	uint32 bt2wl_intrcnt; /* bt->wlan interrrupt count */
	uint32 wl2bt_intrcnt; /* wlan->bt interrupt count  */

	uint32 wl2bt_dset_cnt;
	uint32 wl2bt_dclear_cnt;
	uint32 wl2bt_aset_cnt;
	uint32 wl2bt_aclear_cnt;

	uint32 bt2wl_dset_cnt;
	uint32 bt2wl_dclear_cnt;
	uint32 bt2wl_aset_cnt;
	uint32 bt2wl_aclear_cnt;

	uint32 state_error_1;
	uint32 state_error_2;
	uint32 state_error_3;
	uint32 state_error_4;
} bwte_stats_t;

#define TBOW_MAX_SSID_LEN        32
#define TBOW_MAX_PASSPHRASE_LEN  63

#define WL_TBOW_SETUPINFO_T_VERSION 1 /* version of tbow_setup_netinfo_t */
typedef struct tbow_setup_netinfo {
	uint32 version;
	uint8 opmode;
	uint8 pad;
	uint8 macaddr[ETHER_ADDR_LEN];
	uint32 ssid_len;
	uint8 ssid[TBOW_MAX_SSID_LEN];
	uint8 passphrase_len;
	uint8 passphrase[TBOW_MAX_PASSPHRASE_LEN];
	chanspec_t chanspec;
	uint8 PAD[2];
	uint32 channel;
} tbow_setup_netinfo_t;

typedef enum tbow_ho_opmode {
	TBOW_HO_MODE_START_GO = 0,
	TBOW_HO_MODE_START_STA,
	TBOW_HO_MODE_START_GC,
	TBOW_HO_MODE_TEST_GO,
	TBOW_HO_MODE_STOP_GO = 0x10,
	TBOW_HO_MODE_STOP_STA,
	TBOW_HO_MODE_STOP_GC,
	TBOW_HO_MODE_TEARDOWN
} tbow_ho_opmode_t;

/* Beacon trim feature statistics */
/* configuration */
#define BCNTRIMST_PER			0	/* Number of beacons to trim (0: disable) */
#define BCNTRIMST_TIMEND		1	/* Number of bytes till TIM IE */
#define BCNTRIMST_TSFLMT		2	/* TSF tolerance value (usecs) */
/* internal use */
#define BCNTRIMST_CUR			3	/* PSM's local beacon trim counter */
#define BCNTRIMST_PREVLEN		4	/* Beacon length excluding the TIM IE */
#define BCNTRIMST_TIMLEN		5	/* TIM IE Length */
#define BCNTRIMST_RSSI			6	/* Partial beacon RSSI */
#define BCNTRIMST_CHAN			7	/* Partial beacon channel */
/* debug stat (off by default) */
#define BCNTRIMST_DUR			8	/* RX duration until beacon trimmed */
#define BCNTRIMST_RXMBSS		9	/* MYBSSID beacon received */
#define BCNTRIMST_CANTRIM		10	/* # beacons which were trimmed */
#define BCNTRIMST_LENCHG		11	/* # beacons not trimmed due to length change */
#define BCNTRIMST_TSFDRF		12	/* # beacons not trimmed due to large TSF delta */
#define BCNTRIMST_NOTIM			13	/* # beacons not trimmed due to TIM missing */

#define BCNTRIMST_NUM			14

#define WL_BCNTRIM_STATUS_VERSION_1 1
typedef struct wl_bcntrim_status_query_v1 {
	uint16  version;
	uint16  len;     /* Total length includes fixed fields */
	uint8   reset;   /* reset after reading the stats */
	uint8   pad[3];  /* 4-byte alignment */
} wl_bcntrim_status_query_v1_t;

typedef struct wl_bcntrim_status_v1 {
	uint16  version;
	uint16  len;            /* Total length includes fixed fields and variable data[] */
	uint8   curr_slice_id;  /* slice index of the interface */
	uint8   applied_cfg;    /* applied bcntrim N threshold */
	uint8   pad[2];         /* 4-byte alignment */
	uint32  fw_status;      /* Bits representing bcntrim disable reason in FW */
	uint32  total_disable_dur;    /* total duration (msec) bcntrim remains
	                                 disabled due to FW disable reasons
	                               */
	uint32  data[];         /* variable length data containing stats */
} wl_bcntrim_status_v1_t;

#define BCNTRIM_STATS_MAX		10	/* Total stats part of the status data[] */

/* Bits for FW status */
#define WL_BCNTRIM_DISABLE_HOST		0x1	/* Host disabled bcntrim through bcntrim IOVar */
#define WL_BCNTRIM_DISABLE_PHY_RATE	0x2	/* bcntrim disabled because beacon rx rate is
		                                 * higher than phy_rate_thresh
		                                 */
#define WL_BCNTRIM_DISABLE_QUIET_IE	0x4	/* bcntrim disable when Quiet IE present */
#define WL_BCNTRIM_DISABLE_QBSSLOAD_IE	0x8	/* bcntrim disable when QBSS Load IE present */

#define WL_BCNTRIM_CFG_VERSION_1     1
/* Common IOVAR struct */
typedef struct wl_bcntrim_cfg_v1 {
	uint16  version;
	uint16 len;          /* Total length includes fixed fields and variable data[] */
	uint16 subcmd_id;    /* subcommand id */
	uint16 pad;          /* pad/reserved */
	uint8 data[];        /* subcommand data; could be empty */
} wl_bcntrim_cfg_v1_t;

/* subcommands ids */
enum {
	WL_BCNTRIM_CFG_SUBCMD_PHY_RATE_THRESH =       0,   /* PHY rate threshold above
	                                                      which bcntrim is not applied
	                                                    */
	WL_BCNTRIM_CFG_SUBCMD_OVERRIDE_DISABLE_MASK = 1,   /* Override bcntrim disable reasons */
	WL_BCNTRIM_CFG_SUBCMD_TSF_DRIFT_LIMIT =       2    /* TSF drift limit to consider bcntrim */
};

#define BCNTRIM_MAX_PHY_RATE	48     /* in 500Kbps */
#define BCNTRIM_MAX_TSF_DRIFT   65535  /* in usec */
#define WL_BCNTRIM_OVERRIDE_DISABLE_MASK \
		(WL_BCNTRIM_DISABLE_QUIET_IE | WL_BCNTRIM_DISABLE_QBSSLOAD_IE)

/* WL_BCNTRIM_CFG_SUBCMD_PHY_RATE_TRESH */
typedef struct wl_bcntrim_cfg_phy_rate_thresh {
	uint32 rate;      /* beacon rate (in 500kbps units)  */
} wl_bcntrim_cfg_phy_rate_thresh_t;

/* WL_BCNTRIM_CFG_SUBCMD_OVERRIDE_DISABLE_MASK */
typedef struct wl_bcntrim_cfg_override_disable_mask {
	uint32  mask;     /* bits representing individual disable reason to override */
} wl_bcntrim_cfg_override_disable_mask_t;

/* WL_BCNTRIM_CFG_SUBCMD_TSF_DRIFT_LIMIT */
typedef struct wl_bcntrim_cfg_tsf_drift_limit {
	uint16   drift;   /* tsf drift limit specified in usec */
	uint8    pad[2];  /* 4-byte alignment */
} wl_bcntrim_cfg_tsf_drift_limit_t;

/* --------------  TX Power Cap --------------- */
#define TXPWRCAP_MAX_NUM_CORES 8
#define TXPWRCAP_MAX_NUM_ANTENNAS (TXPWRCAP_MAX_NUM_CORES * 2)

#define TXPWRCAP_NUM_SUBBANDS 5

/* IOVAR txcapconfig enum's */
#define TXPWRCAPCONFIG_WCI2 0
#define TXPWRCAPCONFIG_HOST 1
#define TXPWRCAPCONFIG_WCI2_AND_HOST 2

/* IOVAR txcapstate enum's */
#define TXPWRCAPSTATE_LOW_CAP  0
#define TXPWRCAPSTATE_HIGH_CAP 1
#define TXPWRCAPSTATE_HOST_LOW_WCI2_LOW_CAP	0
#define TXPWRCAPSTATE_HOST_LOW_WCI2_HIGH_CAP	1
#define TXPWRCAPSTATE_HOST_HIGH_WCI2_LOW_CAP	2
#define TXPWRCAPSTATE_HOST_HIGH_WCI2_HIGH_CAP	3

/* IOVAR txcapconfig and txcapstate structure is shared: SET and GET */
#define TXPWRCAPCTL_VERSION 2
typedef struct wl_txpwrcap_ctl {
	uint8   version;
	uint8   ctl[TXPWRCAP_NUM_SUBBANDS];
} wl_txpwrcap_ctl_t;

/* IOVAR txcapdump structure: GET only */
#define TXPWRCAP_DUMP_VERSION 2
typedef struct wl_txpwrcap_dump {
	uint8   version;
	uint8	pad0;
	uint8   current_country[2];
	uint32	current_channel;
	uint8   config[TXPWRCAP_NUM_SUBBANDS];
	uint8   state[TXPWRCAP_NUM_SUBBANDS];
	uint8	high_cap_state_enabled;
	uint8	wci2_cell_status_last;
	uint8   download_present;
	uint8	num_subbands;
	uint8	num_antennas;
	uint8   num_antennas_per_core[TXPWRCAP_MAX_NUM_CORES];
	uint8	num_cc_groups;
	uint8   current_country_cc_group_info_index;
	int8    low_cap[TXPWRCAP_MAX_NUM_ANTENNAS*TXPWRCAP_NUM_SUBBANDS];
	int8    high_cap[TXPWRCAP_MAX_NUM_ANTENNAS*TXPWRCAP_NUM_SUBBANDS];
	uint8	PAD[3];
} wl_txpwrcap_dump_t;

typedef struct wl_txpwrcap_dump_v3 {
	uint8   version;
	uint8	pad0;
	uint8   current_country[2];
	uint32	current_channel;
	uint8   config[TXPWRCAP_NUM_SUBBANDS];
	uint8   state[TXPWRCAP_NUM_SUBBANDS];
	uint8	high_cap_state_enabled;
	uint8	wci2_cell_status_last;
	uint8   download_present;
	uint8	num_subbands;
	uint8	num_antennas;
	uint8   num_antennas_per_core[TXPWRCAP_MAX_NUM_CORES];
	uint8	num_cc_groups;
	uint8   current_country_cc_group_info_index;
	uint8	cap_states_per_cc_group;
	int8    host_low_wci2_low_cap[TXPWRCAP_MAX_NUM_ANTENNAS*TXPWRCAP_NUM_SUBBANDS];
	int8    host_low_wci2_high_cap[TXPWRCAP_MAX_NUM_ANTENNAS*TXPWRCAP_NUM_SUBBANDS];
	int8    host_high_wci2_low_cap[TXPWRCAP_MAX_NUM_ANTENNAS*TXPWRCAP_NUM_SUBBANDS];
	int8    host_high_wci2_high_cap[TXPWRCAP_MAX_NUM_ANTENNAS*TXPWRCAP_NUM_SUBBANDS];
	uint8	PAD[2];
} wl_txpwrcap_dump_v3_t;

typedef struct wl_txpwrcap_tbl {
	uint8 num_antennas_per_core[TXPWRCAP_MAX_NUM_CORES];
	/* Stores values for valid antennas */
	int8 pwrcap_cell_on[TXPWRCAP_MAX_NUM_ANTENNAS]; /* qdBm units */
	int8 pwrcap_cell_off[TXPWRCAP_MAX_NUM_ANTENNAS]; /* qdBm units */
} wl_txpwrcap_tbl_t;

/* ##### Ecounters section ##### */
#define ECOUNTERS_VERSION_1	1

/* Input structure for ecounters IOVAR */
typedef struct ecounters_config_request {
	uint16 version;		/* config version */
	uint16 set;		/* Set where data will go. */
	uint16 size;		/* Size of the set. */
	uint16 timeout;		/* timeout in seconds. */
	uint16 num_events;	/* Number of events to report. */
	uint16 ntypes;		/* Number of entries in type array. */
	uint16 type[1];		/* Statistics Types (tags) to retrieve. */
} ecounters_config_request_t;

#define ECOUNTERS_EVENTMSGS_VERSION_1		1
#define ECOUNTERS_TRIGGER_CONFIG_VERSION_1	1

#define ECOUNTERS_EVENTMSGS_EXT_MASK_OFFSET	\
		OFFSETOF(ecounters_eventmsgs_ext_t, mask[0])

#define ECOUNTERS_TRIG_CONFIG_TYPE_OFFSET	\
		OFFSETOF(ecounters_trigger_config_t, type[0])

typedef struct ecounters_eventmsgs_ext {
	uint8 version;
	uint8 len;
	uint8 mask[1];
} ecounters_eventmsgs_ext_t;

typedef struct ecounters_trigger_config {
	uint16 version;		/* version */
	uint16 set;		/* set where data should go */
	uint16 rsvd;		/* reserved */
	uint16 pad;		/* pad/reserved */
	uint16 ntypes;		/* number of types/tags */
	uint16 type[1];		/* list of types */
} ecounters_trigger_config_t;

#define ECOUNTERS_TRIGGER_REASON_VERSION_1	1
typedef enum {
	/* Triggered due to timer based ecounters */
	ECOUNTERS_TRIGGER_REASON_TIMER = 0,
	/* Triggered due to event based configuration */
	ECOUNTERS_TRIGGER_REASON_EVENTS = 1,
	ECOUNTERS_TRIGGER_REASON_MAX = 2
} ecounters_trigger_reasons_list_t;

typedef struct ecounters_trigger_reason {
	uint16 version;			/* version */
	uint16 trigger_reason;		/* trigger reason */
	uint32 sub_reason_code;		/* sub reason code */
	uint32 trigger_time_now;	/* time in ms  at trigger */
	uint32 host_ref_time;		/* host ref time */
} ecounters_trigger_reason_t;

#define WL_LQM_VERSION_1 1

/* For wl_lqm_t flags field */
#define WL_LQM_CURRENT_BSS_VALID 0x1
#define WL_LQM_TARGET_BSS_VALID 0x2

typedef struct {
	struct ether_addr BSSID;
	chanspec_t chanspec;
	int32 rssi;
	int32 snr;
} wl_rx_signal_metric_t;

typedef struct {
	uint8 version;
	uint8 flags;
	uint16 pad;
	int32 noise_level; /* current noise level */
	wl_rx_signal_metric_t current_bss;
	wl_rx_signal_metric_t target_bss;
} wl_lqm_t;

/* ##### Ecounters v2 section ##### */

#define ECOUNTERS_VERSION_2	2

/* Enumeration of various ecounters request types. This namespace is different from
 * global reportable stats namespace.
*/
enum {
	WL_ECOUNTERS_XTLV_REPORT_REQ = 1
};

/* Input structure for ecounters IOVAR */
typedef struct ecounters_config_request_v2 {
	uint16 version;		/* config version */
	uint16 len;		/* Length of this struct including variable len */
	uint16 logset;		/* Set where data will go. */
	uint16 reporting_period;	/* reporting_period */
	uint16 num_reports;	/* Number of timer expirations to report on */
	uint8 pad[2];		/* Reserved for future use */
	uint8 ecounters_xtlvs[];	/* Statistics Types (tags) to retrieve. */
} ecounters_config_request_v2_t;

#define ECOUNTERS_STATS_TYPES_FLAG_SLICE	0x1
#define ECOUNTERS_STATS_TYPES_FLAG_IFACE	0x2
#define ECOUNTERS_STATS_TYPES_FLAG_GLOBAL	0x4

/* Slice mask bits */
#define ECOUNTERS_STATS_TYPES_SLICE_MASK_SLICE0	0x1
#define ECOUNTERS_STATS_TYPES_SLICE_MASK_SLICE1	0x2

typedef struct ecounters_stats_types_report_req {
	/* flags: bit0 = slice, bit1 = iface, bit2 = global,
	 * rest reserved
	 */
	uint16 flags;
	uint16 if_index;	/* host interface index */
	uint16 slice_mask;	/* bit0 = slice0, bit1=slice1, rest reserved */
	uint8 pad[2];	/* padding */
	uint8 stats_types_req[]; /* XTLVs of requested types */
} ecounters_stats_types_report_req_t;

/* ##### Ecounters_Eventmsgs v2 section ##### */

#define ECOUNTERS_EVENTMSGS_VERSION_2		2

typedef struct event_ecounters_config_request_v2 {
	uint16 version;	/* config version */
	uint16 len;	/* Length of this struct including variable len */
	uint16 logset;	/* Set where data will go. */
	uint16 event_id;	/* Event id for which this config is meant for */
	uint8 flags;	/* Config flags */
	uint8 pad[3];	/* Reserved for future use */
	uint8 ecounters_xtlvs[];	/* Statistics Types (tags) to retrieve. */
} event_ecounters_config_request_v2_t;

#define EVENT_ECOUNTERS_FLAGS_ADD	(1 << 0) /* Add configuration for the event_id if set */
#define EVENT_ECOUNTERS_FLAGS_DEL	(1 << 1) /* Delete configuration for event_id if set */
#define EVENT_ECOUNTERS_FLAGS_ANYIF	(1 << 2) /* Interface filtering disable / off bit */
#define EVENT_ECOUNTERS_FLAGS_BE	(1 << 3) /* If cleared report stats of
						    * one event log buffer
						    */
#define EVENT_ECOUNTERS_FLAGS_DEL_ALL	(1 << 4) /* Delete all the configurations of
						    * event ecounters if set
						    */

/* Ecounters suspend resume */
#define ECOUNTERS_SUSPEND_VERSION_V1	1
/* To be used in populating suspend_mask and suspend_bitmap */
#define ECOUNTERS_SUSPEND_TIMER (1 << ECOUNTERS_TRIGGER_REASON_TIMER)
#define ECOUNTERS_SUSPEND_EVENTS (1 << ECOUNTERS_TRIGGER_REASON_EVENTS)

typedef struct ecounters_suspend {
	uint16 version;
	uint16 len;
	uint32 suspend_bitmap; /* type of ecounter reporting to be suspended */
	uint32 suspend_mask; /* type of ecounter reporting to be suspended */
} ecounters_suspend_t;

/* -------------- dynamic BTCOEX --------------- */
#define DCTL_TROWS	2			/**< currently practical number of rows  */
#define DCTL_TROWS_MAX	4			/**<  2 extra rows RFU */
/* DYNCTL profile flags */
#define DCTL_FLAGS_DISABLED	0		/**< default value: all features disabled */
#define DCTL_FLAGS_DYNCTL	(1 << 0)	/**<  1 - enabled, 0 - legacy only */
#define DCTL_FLAGS_DESENSE	(1 << 1)	/**< auto desense is enabled */
#define DCTL_FLAGS_MSWITCH	(1 << 2)	/**< mode switching is enabled */
#define DCTL_FLAGS_PWRCTRL	(1 << 3)	/**< Tx power control is enabled */
/* for now AGG on/off is handled separately  */
#define DCTL_FLAGS_TX_AGG_OFF	(1 << 4)	/**< TBD: allow TX agg Off */
#define DCTL_FLAGS_RX_AGG_OFF	(1 << 5)	/**< TBD: allow RX agg Off */
/* used for dry run testing only */
#define DCTL_FLAGS_DRYRUN	(1 << 7)	/**< Enables dynctl dry run mode  */
#define IS_DYNCTL_ON(prof)	((prof->flags & DCTL_FLAGS_DYNCTL) != 0)
#define IS_DESENSE_ON(prof)	((prof->flags & DCTL_FLAGS_DESENSE) != 0)
#define IS_MSWITCH_ON(prof)	((prof->flags & DCTL_FLAGS_MSWITCH) != 0)
#define IS_PWRCTRL_ON(prof)	((prof->flags & DCTL_FLAGS_PWRCTRL) != 0)
/* desense level currently in use */
#define DESENSE_OFF	0
#define DFLT_DESENSE_MID	12
#define DFLT_DESENSE_HIGH	2

/**
 * dynctl data points(a set of btpwr & wlrssi thresholds)
 * for mode & desense switching
 */
typedef struct btc_thr_data {
	int8	mode;	/**< used by desense sw */
	int8	bt_pwr;	/**< BT tx power threshold */
	int8	bt_rssi;	/**< BT rssi threshold */
	/* wl rssi range when mode or desense change may be needed */
	int8	wl_rssi_high;
	int8	wl_rssi_low;
} btc_thr_data_t;

/* dynctl. profile data structure  */
#define DCTL_PROFILE_VER 0x01
#include <packed_section_start.h>
typedef BWL_PRE_PACKED_STRUCT struct  dctl_prof {
	uint8 version;  /**< dynctl profile version */
	/* dynctl profile flags bit:0 - dynctl On, bit:1 dsns On, bit:2 mode sw On, */
	uint8 flags;  /**< bit[6:3] reserved, bit7 - Dryrun (sim) - On */
	/**  wl desense levels to apply */
	uint8	dflt_dsns_level;
	uint8	low_dsns_level;
	uint8	mid_dsns_level;
	uint8	high_dsns_level;
	/** mode switching hysteresis in dBm */
	int8	msw_btrssi_hyster;
	/** default btcoex mode */
	uint8	default_btc_mode;
	/** num of active rows in mode switching table */
	uint8	msw_rows;
	/** num of rows in desense table */
	uint8	dsns_rows;
	/** dynctl mode switching data table  */
	btc_thr_data_t msw_data[DCTL_TROWS_MAX];
	/** dynctl desense switching data table */
	btc_thr_data_t dsns_data[DCTL_TROWS_MAX];
} BWL_POST_PACKED_STRUCT dctl_prof_t;
#include <packed_section_end.h>

/**  dynctl status info */
#include <packed_section_start.h>
typedef BWL_PRE_PACKED_STRUCT struct  dynctl_status {
	uint8 sim_on;	/**< true if simulation is On */
	uint16	bt_pwr_shm; /**< BT per/task power as read from ucode  */
	int8	bt_pwr;		/**< BT pwr extracted & converted to dBm */
	int8	bt_rssi;	/**< BT rssi in dBm */
	int8	wl_rssi;	/**< last wl rssi reading used by btcoex */
	uint8	dsns_level; /**< current desense level */
	uint8	btc_mode;   /**< current btcoex mode */
	/* add more status items if needed,  pad to 4 BB if needed */
} BWL_POST_PACKED_STRUCT dynctl_status_t;
#include <packed_section_end.h>

/**  dynctl simulation (dryrun data) */
#include <packed_section_start.h>
typedef BWL_PRE_PACKED_STRUCT struct  dynctl_sim {
	uint8 sim_on;	/**< simulation mode on/off */
	int8 btpwr;		/**< simulated BT power in dBm */
	int8 btrssi;	/**< simulated BT rssi in dBm */
	int8 wlrssi;	/**< simulated WL rssi in dBm */
} BWL_POST_PACKED_STRUCT dynctl_sim_t;
/* no default structure packing */
#include <packed_section_end.h>

/** PTK key maintained per SCB */
#define RSN_TEMP_ENCR_KEY_LEN 16
typedef struct wpa_ptk {
	uint8 kck[RSN_KCK_LENGTH]; /**< EAPOL-Key Key Confirmation Key (KCK) */
	uint8 kek[RSN_KEK_LENGTH]; /**< EAPOL-Key Key Encryption Key (KEK) */
	uint8 tk1[RSN_TEMP_ENCR_KEY_LEN]; /**< Temporal Key 1 (TK1) */
	uint8 tk2[RSN_TEMP_ENCR_KEY_LEN]; /**< Temporal Key 2 (TK2) */
} wpa_ptk_t;

/** GTK key maintained per SCB */
typedef struct wpa_gtk {
	uint32 idx;
	uint32 key_len;
	uint8  key[DOT11_MAX_KEY_SIZE];
} wpa_gtk_t;

/** FBT Auth Response Data structure */
typedef struct wlc_fbt_auth_resp {
	uint8 macaddr[ETHER_ADDR_LEN]; /**< station mac address */
	uint8 pad[2];
	uint8 pmk_r1_name[WPA2_PMKID_LEN];
	wpa_ptk_t ptk; /**< pairwise key */
	wpa_gtk_t gtk; /**< group key */
	uint32 ie_len;
	uint8 status;  /**< Status of parsing FBT authentication
					Request in application
					*/
	uint8 ies[1]; /**< IEs contains MDIE, RSNIE,
					FBTIE (ANonce, SNonce,R0KH-ID, R1KH-ID)
					*/
} wlc_fbt_auth_resp_t;

/** FBT Action Response frame */
typedef struct wlc_fbt_action_resp {
	uint16 version; /**< structure version */
	uint16 length; /**< length of structure */
	uint8 macaddr[ETHER_ADDR_LEN]; /**< station mac address */
	uint8 data_len;  /**< len of ie from Category */
	uint8 data[1]; /**< data contains category, action, sta address, target ap,
						status code,fbt response frame body
						*/
} wlc_fbt_action_resp_t;

#define MACDBG_PMAC_ADDR_INPUT_MAXNUM 16
#define MACDBG_PMAC_OBJ_TYPE_LEN 8

typedef struct _wl_macdbg_pmac_param_t {
	char type[MACDBG_PMAC_OBJ_TYPE_LEN];
	uint8 step;
	uint8 w_en;
	uint16 num;
	uint32 bitmap;
	uint8 addr_raw;
	uint8 addr_num;
	uint16 addr[MACDBG_PMAC_ADDR_INPUT_MAXNUM];
	uint8 pad0[2];
	uint32 w_val;
} wl_macdbg_pmac_param_t;

/* IOVAR 'hwareg' parameter. Used to get/set HWA register value */
typedef struct _wl_hwadbg_reg_param_t {
	char type[MACDBG_PMAC_OBJ_TYPE_LEN];
	uint32 offset;
	uint32 w_val;
	uint32 w_en;
} wl_hwadbg_reg_param_t;

/** IOVAR 'svmp_sampcol' parameter. Used to set and read SVMP_SAMPLE_COLLECT's setting */
typedef struct wl_svmp_sampcol_param {
	uint32 version;           /* version */
	uint8  enable;
	uint8  trigger_mode;      /* SVMP_SAMPCOL_TRIGGER */
	uint8  trigger_mode_s[2]; /* SVMP_SAMPCOL_PKTPROC */
	uint8  data_samplerate;   /* SVMP_SAMPCOL_SAMPLERATE */
	uint8  data_sel_phy1;     /* SVMP_SAMPCOL_PHY1MUX */
	uint8  data_sel_rx1;      /* SVMP_SAMPCOL_RX1MUX without iqCompOut */
	uint8  data_sel_dualcap;  /* SVMP_SAMPCOL_RX1MUX */
	uint8  pack_mode;         /* SVMP_SAMPCOL_PACK */
	uint8  pack_order;
	uint8  pack_cfix_fmt;
	uint8  pack_1core_sel;
	uint16 waitcnt;
	uint16 caplen;
	uint32 buff_addr_start;   /* in word-size (2-bytes) */
	uint32 buff_addr_end;     /* note: Tcl in byte-size, HW in vector-size (8-bytes) */
	uint8  int2vasip;
	uint8  PAD;
	uint16 status;
} wl_svmp_sampcol_t;

#define WL_SVMP_SAMPCOL_PARAMS_VERSION	1

enum {
	SVMP_SAMPCOL_TRIGGER_PKTPROC_TRANSITION = 0,
	SVMP_SAMPCOL_TRIGGER_FORCE_IMMEDIATE,
	SVMP_SAMPCOL_TRIGGER_RADAR_DET
};

enum {
	SVMP_SAMPCOL_PHY1MUX_GPIOOUT = 0,
	SVMP_SAMPCOL_PHY1MUX_FFT,
	SVMP_SAMPCOL_PHY1MUX_DBGHX,
	SVMP_SAMPCOL_PHY1MUX_RX1MUX
};

enum {
	SVMP_SAMPCOL_RX1MUX_FARROWOUT = 4,
	SVMP_SAMPCOL_RX1MUX_IQCOMPOUT,
	SVMP_SAMPCOL_RX1MUX_DCFILTEROUT,
	SVMP_SAMPCOL_RX1MUX_RXFILTEROUT,
	SVMP_SAMPCOL_RX1MUX_ACIFILTEROUT
};

enum {
	SVMP_SAMPCOL_SAMPLERATE_1XBW = 0,
	SVMP_SAMPCOL_SAMPLERATE_2XBW
};

enum {
	SVMP_SAMPCOL_PACK_DUALCAP = 0,
	SVMP_SAMPCOL_PACK_4CORE,
	SVMP_SAMPCOL_PACK_2CORE,
	SVMP_SAMPCOL_PACK_1CORE
};

enum {
	SVMP_SAMPCOL_PKTPROC_RESET = 0,
	SVMP_SAMPCOL_PKTPROC_CARRIER_SEARCH,
	SVMP_SAMPCOL_PKTPROC_WAIT_FOR_NB_PWR,
	SVMP_SAMPCOL_PKTPROC_WAIT_FOR_W1_PWR,
	SVMP_SAMPCOL_PKTPROC_WAIT_FOR_W2_PWR,
	SVMP_SAMPCOL_PKTPROC_OFDM_PHY,
	SVMP_SAMPCOL_PKTPROC_TIMING_SEARCH,
	SVMP_SAMPCOL_PKTPROC_CHAN_EST_1,
	SVMP_SAMPCOL_PKTPROC_LEG_SIG_DEC,
	SVMP_SAMPCOL_PKTPROC_SIG_DECODE_1,
	SVMP_SAMPCOL_PKTPROC_SIG_DECODE_2,
	SVMP_SAMPCOL_PKTPROC_HT_AGC,
	SVMP_SAMPCOL_PKTPROC_CHAN_EST_2,
	SVMP_SAMPCOL_PKTPROC_PAY_DECODE,
	SVMP_SAMPCOL_PKTPROC_DSSS_CCK_PHY,
	SVMP_SAMPCOL_PKTPROC_WAIT_ENERGY_DROP,
	SVMP_SAMPCOL_PKTPROC_WAIT_NCLKS,
	SVMP_SAMPCOL_PKTPROC_PAY_DEC_EXT,
	SVMP_SAMPCOL_PKTPROC_SIG_FAIL_DELAY,
	SVMP_SAMPCOL_PKTPROC_RIFS_SEARCH,
	SVMP_SAMPCOL_PKTPROC_BOARD_SWITCH_DIV_SEARCH,
	SVMP_SAMPCOL_PKTPROC_DSSS_CCK_BOARD_SWITCH_DIV_SEARCH,
	SVMP_SAMPCOL_PKTPROC_CHAN_EST_3,
	SVMP_SAMPCOL_PKTPROC_CHAN_EST_4,
	SVMP_SAMPCOL_PKTPROC_FINE_TIMING_SEARCH,
	SVMP_SAMPCOL_PKTPROC_SET_CLIP_GAIN,
	SVMP_SAMPCOL_PKTPROC_NAP,
	SVMP_SAMPCOL_PKTPROC_VHT_SIGA_DEC,
	SVMP_SAMPCOL_PKTPROC_VHT_SIGB_DEC,
	SVMP_SAMPCOL_PKTPROC_PKT_ABORT,
	SVMP_SAMPCOL_PKTPROC_DCCAL
};

/** IOVAR 'svmp_mem' parameter. Used to read/clear svmp memory */
typedef struct svmp_mem {
	uint32 addr;	/**< offset to read svmp memory from vasip base address */
	uint16 len;	/**< length in count of uint16's */
	uint16 val;	/**< set the range of addr/len with a value */
} svmp_mem_t;

// ucode feature set information -> host as copied from PsmSoftRegDefinitions.h
// Keep in sync with ePsmFeatureSetRegBitDefinitions below
#define EAP_PER_VAP_DTIM_CAP			(1 << 0)
#define EAP_PER_PKT_SOUND_PERIOD_CAP		(1 << 1)
#define EAP_FFT_CAP				(1 << 2)
#define EAP_DURID_OVERRIDE_CAP			(1 << 3)
#define EAP_RUNT_ON_BADFCS_FRM_CAP		(1 << 4)
#define EAP_INT_ON_ALL_BCN_COMPLETE_CAP		(1 << 5)
#define EAP_INT_ON_BCN_AND_BCMC_COMPLETE_CAP	(1 << 6)
#define EAP_RTS_OVERRIDE_CAP			(1 << 7)
#define EAP_SAS_CAP				(1 << 8)
#define EAP_PER_VAP_RATESET_CAP			(1 << 10)
#define EAP_ACK_SUPPRESS_CAP			(1 << 11)
#define EAP_SAS_DEBUG_CAP			(1 << 12)
#define EAP_SAS_RESP_AI_CAP			(1 << 13)
#define EAP_DESC_KEY_CAP			(1 << 14)
#define EAP_PER_PKT_RTX_CAP			(1 << 15)
#define EAP_FIPS_CAP				(1 << 16)
#define EAP_PER_VAP_AMSDU_HWDAGG_DIS_CAP	(1 << 17)
#define EAP_ALLOW_MESH_FRM_CAP			(1 << 18)
#define EAP_AMSDU_CRYPTO_OFFLD_CAP		(1 << 19)
#define EAP_ALLOW_OMGMT_FRM_CAP			(1 << 20)
#define EAP_ACK_RSSI_CAP			(1 << 21)
#define EAP_AMPDU_RTS_RETRIES_CAP		(1 << 22)
#define EAP_FTM_CAP				(1 << 23)

// Keep in sync with above definitions
typedef enum
{
	// DTIM per VAP
	C_UCODE_FEAT_DTIM_VAP_NBIT				= 0,
	// Per packet sound period
	C_UCODE_FEAT_PER_PKT_SOUND_PERIOD_NBIT			= 1,
	// FFT Capture - currently using M_FFT_SMPL_ENABLE
	C_UCODE_FEAT_FFT_CAP_NBIT				= 2,
	// durid override
	C_UCODE_FEAT_DURID_OVERRIDE_NBIT			= 3,
	// force RUNT of BAD-FCS Frames to host
	C_UCODE_FEAT_RUNT_BADFCS_NBIT				= 4,
	// Interrupt when all beacons have been sent
	C_UCODE_FEAT_INT_ON_ALL_BCN_COMPLETE_NBIT		= 5,
	// Interrupt when all beacons and BCMC have been sent
	C_UCODE_FEAT_INT_ON_BCN_AND_BCMC_COMPLETE_NBIT		= 6,
	// RTS Override
	C_UCODE_FEAT_RTS_OVERRIDE_NBIT				= 7,
	// SAS - Smart Antenna System
	C_UCODE_FEAT_SAS					= 8,
	// Outdoor AP
	C_UCODE_FEAT_OUTDOOR_AP					= 9,
	// rateset per VAP
	C_UCODE_FEAT_PER_VAP_RATESET_NBIT			= 10,
	// ACK suppression
	C_UCODE_FEAT_ACK_SUPPRESS				= 11,
	// SAS antenna index debug tracing
	C_UCODE_FEAT_SAS_DEBUG					= 12,
	// SAS response antenna index
	C_UCODE_FEAT_SAS_RESP_AI				= 13,
	// encryption key in the transmit descriptor
	C_UCODE_FEAT_DESC_KEY					= 14,
	// retry limit and fallback thresh in tx desc
	C_UCODE_FEAT_PER_PKT_RTX				= 15
} ePsmFeatureSetRegBitDefinitions;

// Keep in sync with above definitions
typedef enum
{
	// FIPS Loopback
	C_UCODE_FEAT_FIPS_NBIT					= 0,
	// PER VAP AMSDU HW Deaggregation disable
	C_UCODE_FEAT_MBSS_AMSDU_HWDAGG_DIS_NBIT			= 1,
	// Allow null BSSID - mesh frames
	C_UCODE_FEAT_ALLOW_MESH_FRM_NBIT			= 2,
	// AMSDU Crypto offload enable
	C_UCODE_FEAT_AMSDU_CRYPTO_OFFLD_NBIT			= 3,
	// Allow other management frames
	C_UCODE_FEAT_ALLOW_OMGMT_FRM_NBIT			= 4,
	// Include ack RSSI in txstatus
	C_UCODE_FEAT_ACK_RSSI_NBIT				= 5,
	// AMPDU's RTSs use SRL/LRL
	C_FEAT_AMPDU_RTS_RETRIES_NBIT				= 6,
	// Is EAP FTM AVB Capability present
	C_UCODE_FEAT_FTM_NBIT					= 7
} ePsmFeatureSetRegBitDefinitions2;

/** IOVAR 'mu_rate' parameter. read/set mu rate for upto four users */
#define MU_RATE_CFG_VERSION	1
typedef struct mu_rate {
	uint16	version;	/**< version of the structure as defined by MU_RATE_CFG_VERSION */
	uint16	length;		/**< length of entire structure */
	uint8	auto_rate;	/**< enable/disable auto rate */
	uint8	PAD;
	uint16	rate_user[4];	/**< rate per each of four users, set to -1 for no change */
} mu_rate_t;

/** IOVAR 'mu_group' parameter. Used to set and read MU group recommendation setting */
#define WL_MU_GROUP_AUTO_COMMAND      -1
#define WL_MU_GROUP_PARAMS_VERSION     3
#define WL_MU_GROUP_METHOD_NAMELEN    64
#define WL_MU_GROUP_NGROUP_MAX        15
#define WL_MU_GROUP_NUSER_MAX          4
#define WL_MU_GROUP_METHOD_MIN         0
#define WL_MU_GROUP_NUMBER_AUTO_MIN    1
#define WL_MU_GROUP_NUMBER_AUTO_MAX   15
#define WL_MU_GROUP_NUMBER_FORCED_MAX  8
#define WL_MU_GROUP_METHOD_OLD         0
#define WL_MU_GROUP_MODE_AUTO          0
#define WL_MU_GROUP_MODE_FORCED        1
#define WL_MU_GROUP_FORCED_1GROUP      1
#define WL_MU_GROUP_ENTRY_EMPTY       -1
typedef struct mu_group {
	uint32 version;          /* version */
	int16  forced;           /* forced group recommendation */
	int16  forced_group_mcs; /* forced group with mcs */
	int16  forced_group_num; /* forced group number */
	int16  group_option[WL_MU_GROUP_NGROUP_MAX][WL_MU_GROUP_NUSER_MAX];
							/* set mode for forced grouping
							 * and read mode for auto grouping
							 */
	int16  group_GID[WL_MU_GROUP_NGROUP_MAX];
	int16  group_method;     /* methof for VASIP group recommendation */
	int16  group_number;     /* requested number for VASIP group recommendation */
	int16  auto_group_num;   /* exact number from VASIP group recommendation */
	int8   group_method_name[WL_MU_GROUP_METHOD_NAMELEN];
	uint8  PAD[2];
} mu_group_t;

typedef struct mupkteng_sta {
	struct ether_addr ea;
	uint8  PAD[2];
	int32 nrxchain;
	int32 idx;
} mupkteng_sta_t;

typedef struct mupkteng_client {
	int32 rspec;
	int32 idx;
	int32 flen;
	int32 nframes;
} mupkteng_client_t;

typedef struct mupkteng_tx {
	mupkteng_client_t client[8];
	int32 nclients;
	int32 ntx;
} mupkteng_tx_t;

/*
 * MU Packet engine interface.
 * The following two definitions will go into
 * wlioctl_defs.h
 * when wl utility changes are merged to EAGLE TOB & Trunk
 */

#define WL_MUPKTENG_PER_TX_START		0x10
#define WL_MUPKTENG_PER_TX_STOP		        0x20

/** IOVAR 'mu_policy' parameter. Used to configure MU admission control policies */
#define WL_MU_POLICY_PARAMS_VERSION     1
#define WL_MU_POLICY_SCHED_DEFAULT	60
#define WL_MU_POLICY_DISABLED		0
#define WL_MU_POLICY_ENABLED		1
#define WL_MU_POLICY_NRX_MIN		1
#define WL_MU_POLICY_NRX_MAX		2
typedef struct mu_policy {
	uint16 version;
	uint16 length;
	uint32 sched_timer;
	uint32 pfmon;
	uint32 pfmon_gpos;
	uint32 samebw;
	uint32 nrx;
	uint32 max_muclients;
} mu_policy_t;

/** IOVAR 'dynamic_ed_setup' parameter. */

typedef struct dynamic_ed_setup {
	int16 ed_monitor_window; /* Monitoring count/time for dynamic ED. Can be set through WL */
	int16 sed_dis; /* SED higher than this value disables the feature. Can be set through WL */
	int16 sed_lower_bound; /* Lower bound for SED */
	int16 sed_upper_bound; /* Upper bound for SED */
	int16 ed_th_high; /* Maximum ED threshold. Can be set through WL */
	int16 ed_th_low; /* Minimum ED threshold. Can be set through WL */
	int16 ed_inc_step; /* ED increment step. Can be set through WL */
	int16 ed_dec_step; /* ED decrement step. Can be set through WL */
} dynamic_ed_setup_t;

#define WL_NAN_BAND_STR_SIZE 5       /* sizeof ("auto") */

/** Definitions of different NAN Bands */
/* do not change the order */
enum {
		NAN_BAND_B = 0,
		NAN_BAND_A,
		NAN_BAND_AUTO,
		NAN_BAND_INVALID = 0xFF
};

/* ifdef WL11ULB */
/* ULB Mode configured via "ulb_mode" IOVAR */
enum {
	ULB_MODE_DISABLED = 0,
	ULB_MODE_STD_ALONE_MODE = 1,    /* Standalone ULB Mode */
	ULB_MODE_DYN_MODE = 2,      /* Dynamic ULB Mode */
	/* Add all other enums before this */
	MAX_SUPP_ULB_MODES
};

/* ULB BWs configured via "ulb_bw" IOVAR during Standalone Mode Only.
 * Values of this enumeration are also used to specify 'Current Operational Bandwidth'
 * and 'Primary Operational Bandwidth' sub-fields in 'ULB Operations' field (used in
 * 'ULB Operations' Attribute or 'ULB Mode Switch' Attribute)
 */
typedef enum {
	ULB_BW_DISABLED = 0,
	ULB_BW_10MHZ    = 1,    /* Standalone ULB BW in 10 MHz BW */
	ULB_BW_5MHZ = 2,    /* Standalone ULB BW in 5 MHz BW */
	ULB_BW_2P5MHZ   = 3,    /* Standalone ULB BW in 2.5 MHz BW */
	/* Add all other enums before this */
	MAX_SUPP_ULB_BW
} ulb_bw_type_t;
/* endif WL11ULB */

#define WL_MESH_IOCTL_VERSION     1
#define MESH_IOC_BUFSZ            512 /* sufficient ioc buff size for mesh */

#ifdef WLMESH
typedef struct mesh_peer_info_ext {
	mesh_peer_info_t        peer_info;
	uint8   pad1;
	uint16  local_aid;      /* AID generated by *local* to peer */
	uint32  entry_state;    /* see MESH_PEER_ENTRY_STATE_ACTIVE etc; valid
				* ONLY for internal peering requests
				*/
	int8 rssi;
	uint8 pad2;
	struct ether_addr ea; /* peer ea */
} mesh_peer_info_ext_t;

/* #ifdef WLMESH */
typedef struct mesh_peer_info_dump {
	uint32 buflen;
	uint32 version;
	uint16 count;   /* number of results */
	uint16 remaining; /* remaining rsults */
	mesh_peer_info_ext_t    mpi_ext[1];
} mesh_peer_info_dump_t;
#define WL_MESH_PEER_RES_FIXED_SIZE (sizeof(mesh_peer_info_dump_t) - sizeof(mesh_peer_info_ext_t))

#endif /* WLMESH */
/* container for mesh iovtls & events */
typedef struct wl_mesh_ioc {
	uint16  version;        /* interface command or event version */
	uint16  id;             /* mesh ioctl cmd  ID  */
	uint16  len;            /* total length of all tlv records in data[]  */
	uint16  pad;            /* pad to be 32 bit aligment */
	uint8   data[];       /* var len payload of bcm_xtlv_t type */
} wl_mesh_ioc_t;

enum wl_mesh_cmds {
	WL_MESH_CMD_ENABLE = 1,
	WL_MESH_CMD_JOIN = 2,
	WL_MESH_CMD_PEER_STATUS = 3,
	WL_MESH_CMD_ADD_ROUTE = 4,
	WL_MESH_CMD_DEL_ROUTE = 5,
	WL_MESH_CMD_ADD_FILTER = 6,
	WL_MESH_CMD_ENAB_AL_METRIC = 7
};

enum wl_mesh_cmd_xtlv_id {
	WL_MESH_XTLV_ENABLE = 1,
	WL_MESH_XTLV_JOIN = 2,
	WL_MESH_XTLV_STATUS = 3,
	WL_MESH_XTLV_ADD_ROUTE = 4,
	WL_MESH_XTLV_DEL_ROUTE = 5,
	WL_MESH_XTLV_ADD_FILTER = 6,
	WL_MESH_XTLV_ENAB_AIRLINK = 7
};
/* endif WLMESH */

/* Fast BSS Transition parameter configuration */
#define FBT_PARAM_CURRENT_VERSION 0

typedef struct _wl_fbt_params {
	uint16	version;		/* version of the structure
					* as defined by FBT_PARAM_CURRENT_VERSION
					*/
	uint16	length;			/* length of the entire structure */

	uint16 param_type;		/* type of parameter defined below */
	uint16 param_len;		/* length of the param_value */
	uint8 param_value[1];		/* variable length */
} wl_fbt_params_t;

#define WL_FBT_PARAM_TYPE_RSNIE			0
#define WL_FBT_PARAM_TYPE_FTIE			0x1
#define WL_FBT_PARAM_TYPE_SNONCE		0x2
#define WL_FBT_PARAM_TYPE_MDE			0x3
#define WL_FBT_PARAM_TYPE_PMK_R0_NAME		0x4
#define WL_FBT_PARAM_TYPE_R0_KHID		0x5
#define WL_FBT_PARAM_TYPE_R1_KHID		0x6
#define WL_FBT_PARAM_TYPE_FIRST_INVALID		0x7

/* Assoc Mgr commands for fine control of assoc */
#define WL_ASSOC_MGR_CURRENT_VERSION  0x0

typedef struct {
	uint16	version;		/* version of the structure as
					 * defined by WL_ASSOC_MGR_CURRENT_VERSION
					 */
	uint16	length;			/* length of the entire structure */

	uint16 cmd;
	uint16 params;
} wl_assoc_mgr_cmd_t;

#define WL_ASSOC_MGR_CMD_PAUSE_ON_EVT		0 /* have assoc pause on certain events */
#define WL_ASSOC_MGR_CMD_ABORT_ASSOC		1

#define WL_ASSOC_MGR_PARAMS_EVENT_NONE			0 /* use this to resume as well as clear */
#define WL_ASSOC_MGR_PARAMS_PAUSE_EVENT_AUTH_RESP	1

#define WL_WINVER_STRUCT_VER_1 (1)

typedef struct wl_winver {

	/* Version and length of this structure. Length includes all fields in wl_winver_t */
	uint16 struct_version;
	uint16 struct_length;

	/* Windows operating system version info (Microsoft provided) */
	struct {
		uint32 major_ver;
		uint32 minor_ver;
		uint32 build;
	} os_runtime;

	/* NDIS runtime version (Microsoft provided) */
	struct {
		uint16 major_ver;
		uint16 minor_ver;
	} ndis_runtime;

	/* NDIS Driver version (Broadcom provided) */
	struct {
		uint16 major_ver;
		uint16 minor_ver;
	} ndis_driver;

	/* WDI Upper Edge (UE) Driver version (Microsoft provided) */
	struct {
		uint8 major_ver;
		uint8 minor_ver;
		uint8 suffix;
	} wdi_ue;

	/* WDI Lower Edge (LE) Driver version (Broadcom provided) */
	struct {
		uint8 major_ver;
		uint8 minor_ver;
		uint8 suffix;
	} wdi_le;
	uint8 PAD[2];
} wl_winver_t;

/* defined(WLRCC) || defined(ROAM_CHANNEL_CACHE) */
#define MAX_ROAM_CHANNEL      20
typedef struct {
	int32 n;
	chanspec_t channels[MAX_ROAM_CHANNEL];
} wl_roam_channel_list_t;
/* endif RCC || ROAM_CHANNEL_CACHE */

/* values for IOV_MFP arg */
enum {
	WL_MFP_NONE = 0,
	WL_MFP_CAPABLE,
	WL_MFP_REQUIRED
};

typedef enum {
	CHANSW_UNKNOWN = 0,	/* channel switch due to unknown reason */
	CHANSW_SCAN = 1,	/* channel switch due to scan */
	CHANSW_PHYCAL = 2,	/* channel switch due to phy calibration */
	CHANSW_INIT = 3,	/* channel set at WLC up time */
	CHANSW_ASSOC = 4,	/* channel switch due to association */
	CHANSW_ROAM = 5,	/* channel switch due to roam */
	CHANSW_MCHAN = 6,	/* channel switch triggered by mchan module */
	CHANSW_IOVAR = 7,	/* channel switch due to IOVAR */
	CHANSW_CSA_DFS = 8,	/* channel switch due to chan switch  announcement from AP */
	CHANSW_APCS = 9,	/* Channel switch from AP channel select module */
	CHANSW_AWDL = 10,	/* channel switch due to AWDL */
	CHANSW_FBT = 11,	/* Channel switch from FBT module for action frame response */
	CHANSW_UPDBW = 12,	/* channel switch at update bandwidth */
	CHANSW_ULB = 13,	/* channel switch at ULB */
	CHANSW_LAST = 14	/* last channel switch reason */
} chansw_reason_t;

/*
 * WOWL unassociated mode power svae pattern.
 */
typedef struct wowl_radio_duty_cycle {
	uint16 wake_interval;
	uint16  sleep_interval;
} wowl_radio_duty_cycle_t;

typedef struct nd_ra_ol_limits {
	uint16 version;         /* version of the iovar buffer */
	uint16 type;            /* type of data provided */
	uint16 length;          /* length of the entire structure */
	uint16 pad1;            /* pad union to 4 byte boundary */
	union {
		struct {
			uint16 min_time;         /* seconds, min time for RA offload hold */
			uint16 lifetime_percent;
			/* percent, lifetime percentage for offload hold time */
		} lifetime_relative;
		struct {
			uint16 hold_time;        /* seconds, RA offload hold time */
			uint16 pad2;             /* unused */
		} fixed;
	} limits;
} nd_ra_ol_limits_t;

#define ND_RA_OL_LIMITS_VER 1

/* nd_ra_ol_limits sub-types */
#define ND_RA_OL_LIMITS_REL_TYPE   0     /* relative, percent of RA lifetime */
#define ND_RA_OL_LIMITS_FIXED_TYPE 1     /* fixed time */

/* buffer lengths for the different nd_ra_ol_limits types */
#define ND_RA_OL_LIMITS_REL_TYPE_LEN   12
#define ND_RA_OL_LIMITS_FIXED_TYPE_LEN  10

/*
 * Temperature Throttling control mode
 */
typedef struct wl_temp_control {
	uint8 enable;
	uint8 PAD;
	uint16 control_bit;
} wl_temp_control_t;

/* SensorHub Interworking mode */

#define SHUB_CONTROL_VERSION    1
#define SHUB_CONTROL_LEN    12

typedef struct {
	uint16  verison;
	uint16  length;
	uint16  cmd;
	uint16  op_mode;
	uint16  interval;
	uint16  enable;
} shub_control_t;

/* WLC_MAJOR_VER <= 5 */
/* Data structures for non-TLV format */

/* Data structures for rsdb caps */
/*
 * The flags field of the rsdb_caps_response is designed to be
 * a Bit Mask. As of now only Bit 0 is used as mentioned below.
 */

/* Bit-0 in flags is used to indicate if the cores can operate synchronously
* i.e either as 2x2 MIMO or 2(1x1 SISO). This is true only for 4349 variants
* 0 - device can operate only in rsdb mode (eg: 4364)
* 1 - device can operate in both rsdb and mimo (eg : 4359 variants)
*/

#define WL_RSDB_CAPS_VER 2
#define SYNCHRONOUS_OPERATION_TRUE	(1 << 0)
#define WL_RSDB_CAPS_FIXED_LEN  OFFSETOF(rsdb_caps_response_t, num_chains)

typedef struct rsdb_caps_response {
	uint8 ver;		/* Version */
	uint8 len;		/* length of this structure excluding ver and len */
	uint8 rsdb;		/* TRUE for rsdb chip */
	uint8 num_of_cores;	/* no of d11 cores */
	uint16 flags;		/* Flags to indicate various capabilities */
	uint8 num_chains[1];	/* Tx/Rx chains for each core */
} rsdb_caps_response_t;

/* Data structures for rsdb bands */

#define WL_RSDB_BANDS_VER       2
#define WL_RSDB_BANDS_FIXED_LEN  OFFSETOF(rsdb_bands_t, band)

typedef struct rsdb_bands
{
	uint8 ver;
	uint8 len;
	uint16 num_cores;	/* num of D11 cores */
	int16 band[1];		/* The band operating on each of the d11 cores */
} rsdb_bands_t;

/* rsdb config */

#define WL_RSDB_CONFIG_VER 3
#define ALLOW_SIB_PARALLEL_SCAN	(1 << 0)
#define MAX_BANDS 2

#define WL_RSDB_CONFIG_LEN sizeof(rsdb_config_t)

typedef uint8 rsdb_opmode_t;
typedef uint32 rsdb_flags_t;

typedef enum rsdb_modes {
	WLC_SDB_MODE_NOSDB_MAIN = 1, /* 2X2 or MIMO mode (applicable only for 4355) */
	WLC_SDB_MODE_NOSDB_AUX = 2,
	WLC_SDB_MODE_SDB_MAIN = 3, /* This is RSDB mode(default) applicable only for 4364 */
	WLC_SDB_MODE_SDB_AUX = 4,
	WLC_SDB_MODE_SDB_AUTO = 5, /* Same as WLC_RSDB_MODE_RSDB(1+1) mode above */
} rsdb_modes_t;

typedef struct rsdb_config {
	uint8 ver;
	uint8 len;
	uint16 reserved;
	rsdb_opmode_t non_infra_mode;
	rsdb_opmode_t infra_mode[MAX_BANDS];
	rsdb_flags_t flags[MAX_BANDS];
	rsdb_opmode_t current_mode;   /* Valid only in GET, returns the current mode */
	uint8  pad[3];
} rsdb_config_t;

/* WLC_MAJOR_VER > =5 */
/* TLV definitions and data structures for rsdb subcmds */

enum wl_rsdb_cmd_ids {
	/* RSDB ioctls */
	WL_RSDB_CMD_VER = 0,
	WL_RSDB_CMD_CAPS = 1,
	WL_RSDB_CMD_BANDS = 2,
	WL_RSDB_CMD_CONFIG = 3,
	/* Add before this !! */
	WL_RSDB_CMD_LAST
};
#define WL_RSDB_IOV_VERSION	0x1

typedef struct rsdb_caps_response_v1 {
	uint8 rsdb;		/* TRUE for rsdb chip */
	uint8 num_of_cores;	/* no of d11 cores */
	uint16 flags;		/* Flags to indicate various capabilities */
	uint8 num_chains[MAX_NUM_D11CORES];	/* Tx/Rx chains for each core */
	uint8 band_cap[MAX_NUM_D11CORES]; /* band cap bitmask per slice */
} rsdb_caps_response_v1_t;

typedef struct rsdb_bands_v1
{
	uint8 num_cores;		/* num of D11 cores */
	uint8 pad;			/* padding bytes for 4 byte alignment */
	int8 band[MAX_NUM_D11CORES];	/* The band operating on each of the d11 cores */
} rsdb_bands_v1_t;

typedef struct rsdb_config_xtlv {
	rsdb_opmode_t reserved1;	/* Non_infra mode is no more applicable */
	rsdb_opmode_t infra_mode[MAX_BANDS]; /* Target mode for Infra association */
	uint8 pad;	/* pad bytes for 4 byte alignment */
	rsdb_flags_t  flags[MAX_BANDS];
	rsdb_opmode_t current_mode; /* GET only; has current mode of operation */
	uint8 pad1[3];
} rsdb_config_xtlv_t;

/* Definitions for slot_bss chanseq iovar */
#define WL_SLOT_BSS_VERSION 1

enum wl_slotted_bss_cmd_id {
	WL_SLOTTED_BSS_CMD_VER = 0,
	WL_SLOTTED_BSS_CMD_CHANSEQ = 1
};
typedef uint16 chan_seq_type_t;
enum chan_seq_type {
	CHAN_SEQ_TYPE_AWDL = 1,
	CHAN_SEQ_TYPE_SLICE = 2,
	CHAN_SEQ_TYPE_NAN = 3
};
typedef uint8 sched_flag_t;
enum sched_flag {
	NO_SDB_SCHED = 0x1,
	SDB_TDM_SCHED = 0x2,
	SDB_SPLIT_BAND_SCHED = 0x4, /* default mode for 4357 */
	MAIN_ONLY = 0x8,
	AUX_ONLY = 0x10,
	SDB_DUAL_TIME = (MAIN_ONLY | AUX_ONLY),
	NO_SDB_MAIN_ONLY = (NO_SDB_SCHED | MAIN_ONLY), /* default mode for 4364 */
	SDB_TDM_SCHED_MAIN = (SDB_TDM_SCHED | MAIN_ONLY),
	SDB_TDM_SCHED_AUX = (SDB_TDM_SCHED | AUX_ONLY),
	SDB_TDM_SCHED_DUAL_TIME = (SDB_TDM_SCHED | SDB_DUAL_TIME),
	SDB_SPLIT_BAND_SCHED_DUAL_TIME = (SDB_SPLIT_BAND_SCHED | SDB_DUAL_TIME)
};

typedef struct chan_seq_tlv_data {
	uint32 flags;
	uint8 data[1];
} chan_seq_tlv_data_t;

typedef struct chan_seq_tlv {
	chan_seq_type_t type;
	uint16 len;
	chan_seq_tlv_data_t chanseq_data[1];
} chan_seq_tlv_t;

typedef struct sb_channel_sequence {
	sched_flag_t sched_flags; /* (sdb-tdm or sdb-sb or Dual-Time) */
	uint8 num_seq; /* number of chan_seq_tlv following */
	uint16 pad;
	chan_seq_tlv_t seq[1];
} sb_channel_sequence_t;

typedef struct slice_chan_seq {
	uint8 slice_index;  /* 0(Main) or 1 (Aux) */
	uint8 num_chanspecs;
	uint16 pad;
	chanspec_t chanspecs[1];
} slice_chan_seq_t;

#define WL_SLICE_CHAN_SEQ_FIXED_LEN   OFFSETOF(slice_chan_seq_t, chanspecs)

typedef struct sim_pm_params {
	uint32 enabled;
	uint16 cycle;
	uint16 up;
} sim_pm_params_t;

/* Digital napping status */
#define WL_NAP_STATUS_VERSION_1		1
typedef struct wl_nap_status_v1 {
	uint16 version;			/* structure version  */
	uint16 len;			/* length of returned data */
	uint16 fw_status;		/* bitmask of FW disable reasons */
	uint8  hw_status;		/* bitmask for actual HW state info */
	uint8  slice_index;		/* which slice this represents */
	uint32 total_disable_dur;       /* total time (ms) disabled for fw_status */
} wl_nap_status_v1_t;

/* Bits for fw_status */
#define NAP_DISABLED_HOST		0x0001   /* Host has disabled through nap_enable */
#define NAP_DISABLED_RSSI		0x0002   /* Disabled because of nap_rssi_threshold */
#define NAP_DISABLED_SCAN		0x0004   /* Disabled because of scan */
#define NAP_DISABLED_ASSOC		0x0008   /* Disabled because of association */

/* Bits for hw_status */
#define NAP_HWCFG			0x01   /* State of NAP config bit in phy HW */
#define NAP_NOCLK			0x80   /* No clock to read HW (e.g. core down) */

/* ifdef WL_NATOE */
#define WL_NATOE_IOCTL_VERSION		1
#define WL_NATOE_IOC_BUFSZ		512	/* sufficient ioc buff size for natoe */
#define WL_NATOE_DBG_STATS_BUFSZ	2048

/* config natoe STA and AP IP's structure */
typedef struct {
	uint32 sta_ip;
	uint32 sta_netmask;
	uint32 sta_router_ip;
	uint32 sta_dnsip;
	uint32 ap_ip;
	uint32 ap_netmask;
} wl_natoe_config_ips_t;

/* natoe ports config structure */
typedef struct {
	uint16 start_port_num;
	uint16 no_of_ports;
} wl_natoe_ports_config_t;

/* natoe ports exception info */
typedef struct {
	uint16 sta_port_num;
	uint16 dst_port_num;    /* for SIP type protocol, dst_port_num info can be ignored by FW */
	uint32 ip;              /* for SIP ip is APcli_ip and for port clash it is dst_ip */
	uint8  entry_type;      /* Create/Destroy */
	uint8  pad[3];
} wl_natoe_exception_port_t;

/* container for natoe ioctls & events */
typedef struct wl_natoe_ioc {
	uint16  version;        /* interface command or event version */
	uint16  id;             /* natoe ioctl cmd  ID  */
	uint16  len;            /* total length of all tlv records in data[]  */
	uint16  pad;            /* pad to be 32 bit aligment */
	uint8   data[];       /* var len payload of bcm_xtlv_t type */
} wl_natoe_ioc_t;

enum wl_natoe_cmds {
	WL_NATOE_CMD_ENABLE = 1,
	WL_NATOE_CMD_CONFIG_IPS = 2,
	WL_NATOE_CMD_CONFIG_PORTS = 3,
	WL_NATOE_CMD_DBG_STATS = 4,
	WL_NATOE_CMD_EXCEPTION_PORT = 5,
	WL_NATOE_CMD_SKIP_PORT = 6,
	WL_NATOE_CMD_TBL_CNT = 7
};

enum wl_natoe_cmd_xtlv_id {
	WL_NATOE_XTLV_ENABLE = 1,
	WL_NATOE_XTLV_CONFIG_IPS = 2,
	WL_NATOE_XTLV_CONFIG_PORTS = 3,
	WL_NATOE_XTLV_DBG_STATS = 4,
	WL_NATOE_XTLV_EXCEPTION_PORT = 5,
	WL_NATOE_XTLV_SKIP_PORT = 6,
	WL_NATOE_XTLV_TBL_CNT = 7
};

/* endif WL_NATOE */

enum wl_idauth_cmd_ids {
	WL_IDAUTH_CMD_CONFIG				= 1,
	WL_IDAUTH_CMD_PEER_INFO				= 2,
	WL_IDAUTH_CMD_COUNTERS				= 3,
	WL_IDAUTH_CMD_LAST
};
enum wl_idauth_xtlv_id {
	WL_IDAUTH_XTLV_AUTH_ENAB			= 0x1,
	WL_IDAUTH_XTLV_GTK_ROTATION			= 0x2,
	WL_IDAUTH_XTLV_EAPOL_COUNT			= 0x3,
	WL_IDAUTH_XTLV_EAPOL_INTRVL			= 0x4,
	WL_IDAUTH_XTLV_BLKLIST_COUNT			= 0x5,
	WL_IDAUTH_XTLV_BLKLIST_AGE			= 0x6,
	WL_IDAUTH_XTLV_PEERS_INFO			= 0x7,
	WL_IDAUTH_XTLV_COUNTERS				= 0x8
};
enum wl_idauth_stats {
	WL_AUTH_PEER_STATE_AUTHORISED			= 0x01,
	WL_AUTH_PEER_STATE_BLACKLISTED			= 0x02,
	WL_AUTH_PEER_STATE_4WAY_HS_ONGOING		= 0x03,
	WL_AUTH_PEER_STATE_LAST
};
typedef struct {
	uint16 state;				/* Peer State: Authorised or Blacklisted */
	struct ether_addr peer_addr;		/* peer Address */
	uint32 blklist_end_time;		/* Time of blacklist end */
} auth_peer_t;
typedef struct wl_idauth_counters {
	uint32 auth_reqs;			/* No of auth req recvd */
	uint32 mic_fail;			/* No of mic fails */
	uint32 four_way_hs_fail;		/* No of 4-way handshake fails */
} wl_idauth_counters_t;

#define WLC_UTRACE_LEN  512
#define WLC_UTRACE_READ_END 0
#define WLC_UTRACE_MORE_DATA 1
typedef struct wl_utrace_capture_args_v1 {
	uint32 length;
	uint32 flag;
} wl_utrace_capture_args_v1_t;

#define UTRACE_CAPTURE_VER_2	2
typedef struct wl_utrace_capture_args_v2 {
	/* structure control */
	uint16 version;		/**< structure version */
	uint16 length;		/**< length of the response */
	uint32 flag;		/* Indicates if there is more data or not */
} wl_utrace_capture_args_v2_t;

/* Signal read end. */
#define WLC_REGVAL_READ_END 0
/* Signal more data pending. */
#define WLC_REGVAL_MORE_DATA 1
/* Internal read state. */
#define WLC_REGVAL_READ_CONTINUE 2

#define WLC_REGVAL_DUMP_PHYREG 0
#define WLC_REGVAL_DUMP_RADREG 1

#define PHYREGVAL_CAPTURE_BUFFER_LEN 2048

typedef struct wl_regval_capture_args {
	uint32 control_flag;	/* Carries status information. */
} wl_regval_capture_args_t;

/* XTLV IDs for the Health Check "hc" iovar top level container */
enum {
	WL_HC_XTLV_ID_CAT_HC = 1,		/* category for HC as a whole */
	WL_HC_XTLV_ID_CAT_DATAPATH_TX = 2,	/* Datapath Tx */
	WL_HC_XTLV_ID_CAT_DATAPATH_RX = 3,	/* Datapath Rx */
	WL_HC_XTLV_ID_CAT_SCAN	= 4,		/* Scan */
};

/* Health Check: Common XTLV IDs for sub-elements in the top level container
 * Number starts at 0x8000 to be out of the way for category specific IDs.
 */
enum {
	WL_HC_XTLV_ID_ERR       = 0x8000,       /* for sub-command  err return */
	WL_HC_XTLV_ID_IDLIST    = 0x8001,       /* container for uint16 IDs */
};

/* Health Check: Datapath TX IDs */
enum {
	WL_HC_TX_XTLV_ID_VAL_STALL_THRESHOLD   = 1,     /* stall_threshold */
	WL_HC_TX_XTLV_ID_VAL_STALL_SAMPLE_SIZE = 2,     /* stall_sample_size */
	WL_HC_TX_XTLV_ID_VAL_STALL_TIMEOUT     = 3,     /* stall_timeout */
	WL_HC_TX_XTLV_ID_VAL_STALL_FORCE       = 4,     /* stall_force */
	WL_HC_TX_XTLV_ID_VAL_STALL_EXCLUDE     = 5,     /* stall_exclude */
	WL_HC_TX_XTLV_ID_VAL_FC_TIMEOUT        = 6,     /* flow ctl timeout */
	WL_HC_TX_XTLV_ID_VAL_FC_FORCE          = 7,     /* flow ctl force failure */
	WL_HC_TX_XTLV_ID_VAL_DELAY_TO_TRAP     = 8,     /* delay threshold for forced trap */
	WL_HC_TX_XTLV_ID_VAL_DELAY_TO_RPT      = 9,     /* delay threshold for event log report */
	WL_HC_TX_XTLV_ID_VAL_FAILURE_TO_RPT    = 10,	/* threshold for consecutive TX failures */
};

/* Health Check: Datapath RX IDs */
enum {
	WL_HC_RX_XTLV_ID_VAL_DMA_STALL_TIMEOUT = 1,     /* dma_stall_timeout */
	WL_HC_RX_XTLV_ID_VAL_DMA_STALL_FORCE   = 2,     /* dma_stall test trigger */
	WL_HC_RX_XTLV_ID_VAL_STALL_THRESHOLD   = 3,     /* stall_threshold */
	WL_HC_RX_XTLV_ID_VAL_STALL_SAMPLE_SIZE = 4,     /* stall_sample_size */
	WL_HC_RX_XTLV_ID_VAL_STALL_FORCE       = 5,     /* stall test trigger */
};

/* Health Check: Datapath SCAN IDs */
enum {
	WL_HC_XTLV_ID_VAL_SCAN_STALL_THRESHOLD	= 1,	/* scan stall threshold */
};

/* IDs of Health Check report structures for sub types of health checks within WL */
enum {
	WL_HC_DD_UNDEFINED = 0,		/* Undefined */
	WL_HC_DD_RX_DMA_STALL = 1,	/* RX DMA stall check */
	WL_HC_DD_RX_STALL = 2,		/* RX stall check */
	WL_HC_DD_TX_STALL = 3,		/* TX stall check */
	WL_HC_DD_SCAN_STALL = 4,	/* SCAN stall check */
	WL_HC_DD_MAX
};

/*
 * Health Check report structures for sub types of health checks within WL
 */

/* Health Check report structure for Rx DMA Stall check */
typedef struct {
	uint16 type;
	uint16 length;
	uint16 timeout;
	uint16 stalled_dma_bitmap;
} wl_rx_dma_hc_info_t;

/* Health Check report structure for Tx packet failure check */
typedef struct {
	uint16 type;
	uint16 length;
	uint32 stall_bitmap;
	uint32 stall_bitmap1;
	uint32 failure_ac;
	uint32 threshold;
	uint32 tx_all;
	uint32 tx_failure_all;
} wl_tx_hc_info_t;

/* Health Check report structure for Rx dropped packet failure check */
typedef struct {
	uint16 type;
	uint16 length;
	uint32 bsscfg_idx;
	uint32 rx_hc_pkts;
	uint32 rx_hc_dropped_all;
	uint32 rx_hc_alert_th;
} wl_rx_hc_info_t;

/* HE top level command IDs */
enum {
	WL_HE_CMD_ENAB = 0,
	WL_HE_CMD_FEATURES = 1,
	WL_HE_CMD_RESERVED1 = 2, // available
	WL_HE_CMD_TESTBED = 3,
	WL_HE_CMD_BSR_SUPPORT = 4,
	WL_HE_CMD_BSSCOLOR = 5,
	WL_HE_CMD_PARTIAL_BSSCOLOR = 6,
	WL_HE_CMD_CAP = 7,
	WL_HE_CMD_OMI = 8,
	WL_HE_CMD_RANGE_EXT = 9,
	WL_HE_CMD_RTSDURTHRESH = 10,
	WL_HE_CMD_PEDURATION = 11,
	WL_HE_CMD_MUEDCA = 12,
	WL_HE_CMD_DYNFRAG = 13,
	WL_HE_CMD_PPET = 14,
	WL_HE_CMD_HTC = 15,
	WL_HE_CMD_AXMODE = 16,
	WL_HE_CMD_FRAGTX = 17,
	WL_HE_CMD_LAST
};

#define WL_HE_VER_1	1

typedef struct wl_he_bsscolor {
	uint8 color;		/* 1..63, on get returns currently in use color */
	uint8 disabled;		/* 0/1, 0 means disabled is false, so coloring is enabled */
	uint8 switch_count;	/* 0, immediate programming, 1 .. 255 beacon count down */
	uint8 PAD;
} wl_he_bsscolor_t;

typedef struct wl_he_omi {
	struct ether_addr peer;		/* leave it all 0s' for non-AP */
	uint8 rx_nss;			/* 0..7 */
	uint8 channel_width;		/* 0:20, 1:40, 2:80, 3:160 */
	uint8 ul_mu_disable;		/* 0|1 */
	uint8 tx_nsts;			/* 0..7 */
	uint8 er_su_disable;		/* 0|1 */
	uint8 dl_mumimo_resound;	/* 0|1 */
	uint8 ul_mu_data_disable;	/* 0|1 */
	uint8 tx_override;		/* 0, only used for testbed AP */
	uint8 PAD[2];
} wl_he_omi_t;

typedef struct wl_he_edca_v1 {
	uint8 aci_aifsn;
	uint8 ecw_min_max;
	uint8 muedca_timer;
	uint8 PAD;
} wl_he_edca_v1_t;

typedef struct wl_he_muedca_v1 {
	/* structure control */
	uint16 version;	/* structure version */
	uint16 length;	/* data length (starting after this field) */
	wl_he_edca_v1_t ac_param_ap[AC_COUNT];
	wl_he_edca_v1_t ac_param_sta[AC_COUNT];
} wl_he_muedca_v1_t;

/* Muscheduler iovar defs */
#define WL_MUSCHED_CMD_KEYSTR_MAXLEN	20
#define WL_MUSCHED_CMD_ARGSTR_MAXLEN	80
#define WL_MUSCHED_CMD_VAL_MAXNUM	20

#define WL_MUSCHED_FLAGS_VERBOSE_MASK		0x0001
#define WL_MUSCHED_FLAGS_MACADDR_MASK		0x0002
#define WL_MUSCHED_FLAGS_BW_MASK		0x0004
#define WL_MUSCHED_FLAGS_AC_MASK		0x0008
#define WL_MUSCHED_FLAGS_DL_MASK		0x0010
#define WL_MUSCHED_FLAGS_UL_MASK		0x0020

#define WL_MUSCHED_FLAGS_VERBOSE_SET(musched)	(musched->flags |= WL_MUSCHED_FLAGS_VERBOSE_MASK)
#define WL_MUSCHED_FLAGS_MACADDR_SET(musched)	(musched->flags |= WL_MUSCHED_FLAGS_MACADDR_MASK)
#define WL_MUSCHED_FLAGS_BW_SET(musched)	(musched->flags |= WL_MUSCHED_FLAGS_BW_MASK)
#define WL_MUSCHED_FLAGS_AC_SET(musched)	(musched->flags |= WL_MUSCHED_FLAGS_AC_MASK)
#define WL_MUSCHED_FLAGS_DL_SET(musched)	(musched->flags |= WL_MUSCHED_FLAGS_DL_MASK)
#define WL_MUSCHED_FLAGS_UL_SET(musched)	(musched->flags |= WL_MUSCHED_FLAGS_UL_MASK)

#define WL_MUSCHED_FLAGS_VERBOSE(musched)	(musched->flags & WL_MUSCHED_FLAGS_VERBOSE_MASK)
#define WL_MUSCHED_FLAGS_MACADDR(musched)	(musched->flags & WL_MUSCHED_FLAGS_MACADDR_MASK)
#define WL_MUSCHED_FLAGS_BW(musched)		(musched->flags & WL_MUSCHED_FLAGS_BW_MASK)
#define WL_MUSCHED_FLAGS_AC(musched)		(musched->flags & WL_MUSCHED_FLAGS_AC_MASK)
#define WL_MUSCHED_FLAGS_DL(musched)		(musched->flags & WL_MUSCHED_FLAGS_DL_MASK)
#define WL_MUSCHED_FLAGS_UL(musched)		(musched->flags & WL_MUSCHED_FLAGS_UL_MASK)

typedef struct wl_musched_cmd_params {
	uint16	ver;
	uint32	flags;
	uint8	num_vals;
	int8	bw;
	int8	ac;
	int8	row;
	int8	col;
	char	keystr[WL_MUSCHED_CMD_KEYSTR_MAXLEN];
	char	argstr[WL_MUSCHED_CMD_ARGSTR_MAXLEN];
	int16	vals[WL_MUSCHED_CMD_VAL_MAXNUM];
	struct	ether_addr ea;
} wl_musched_cmd_params_t;

/* For txbfcfg iovar */
#define WL_TXBFCFG_FLAGS_VERBOSE_MASK		0x0001
#define WL_TXBFCFG_FLAGS_GETSEL_MASK		0x0002

#define WL_TXBFCFG_FLAGS_VERBOSE_SET(a)	(a->flags |= WL_TXBFCFG_FLAGS_VERBOSE_MASK)
#define WL_TXBFCFG_FLAGS_GETSEL_SET(a)	(a->flags |= WL_TXBFCFG_FLAGS_GETSEL_MASK)
typedef struct wl_txbfcfg_params {
	uint16	ver;
	uint16	pad;
	uint32	flags;
	int16	index;
	int16	val;
	struct	ether_addr ea;
} wl_txbfcfg_params_t;

/* TWT top level command IDs */
enum {
	WL_TWT_CMD_ENAB = 0,
	WL_TWT_CMD_SETUP = 1,
	WL_TWT_CMD_TEARDOWN = 2,
	WL_TWT_CMD_INFO = 3,
	WL_TWT_CMD_LIST = 4,
	WL_TWT_CMD_LAST
};

/* TODO: Remove the follwoing after mering TWT changes to trunk */
#define WL_TWT_CMD_DEF_IN_WLIOCTL 1

#define WL_HEB_VER_1	1

/* HEB top level command IDs */
enum {
	WL_HEB_CMD_ENAB = 0,
	WL_HEB_CMD_NUM_HEB = 1,
	WL_HEB_CMD_COUNTERS = 2,
	WL_HEB_CMD_CLEAR_COUNTERS = 3,
	WL_HEB_CMD_CONFIG = 4,
	WL_HEB_CMD_STATUS = 5,
	WL_HEB_CMD_LAST
};

/* HEB counters structures */
typedef struct wl_heb_int_cnt_v1 {
	uint16 pre_event;
	uint16 start_event;
	uint16 end_event;
	uint16 missed;
} wl_heb_int_cnt_v1_t;

typedef struct wl_heb_cnt_v1 {
	/* structure control */
	uint16 version;	/* structure version */
	uint16 length;	/* data length (starting after this field) */
	wl_heb_int_cnt_v1_t heb_int_cnt[1];
} wl_heb_cnt_v1_t;

// struct for configuring HEB
typedef struct wl_config_heb_fill_v1 {
	uint16 version;	/* structure version */
	uint16 length;	/* data length (starting after this field) */
	uint32 duration;
	uint32 periodicity;
	uint16 heb_idx;
	uint16 preeventtime;
	uint8 count;
	uint8 PAD[3];
} wl_config_heb_fill_v1_t;

typedef struct wl_heb_blk_params_v1 {
	/* Don't change the order of following elements. This is as per the HEB HW spec */
	uint32 event_int_val_l;
	uint32 event_int_val_h;
	uint32 param2;
	uint32 param3;
	uint32 pre_event_intmsk_bmp;
	uint32 start_event_intmsk_bmp;
	uint32 end_event_intmsk_bmp;
	uint32 event_driver_info;
	uint16 param1;
	uint8 event_count;
	uint8 noa_invert;
} wl_heb_blk_params_v1_t;

typedef struct wl_heb_int_status_v1 {
	uint32 heb_idx;
	wl_heb_blk_params_v1_t blk_params;
} wl_heb_reg_status_v1_t;

typedef struct wl_heb_status_v1 {
	uint16 version;	/* structure version */
	uint16 length;	/* data length (starting after this field) */
	wl_heb_reg_status_v1_t heb_status[1];
} wl_heb_status_v1_t;

/* HE PPET command values */
enum {
	WL_HE_PPET_0US = 0,	/* 0 usec */
	WL_HE_PPET_8US = 1,	/* 8 usec */
	WL_HE_PPET_16US = 2,	/* 16 usec */
	WL_HE_PPET_AUTO = 0xff	/* Auto */
};

/* XXX
 * NOTES:
 * wl_twt_sdesc_t is used to support both broadcast TWT and individual TWT.
 */

/* Flow flags */
#define WL_TWT_FLOW_FLAG_BROADCAST	(1 << 0)	/* Individual or Bcast TWT */
#define WL_TWT_FLOW_FLAG_IMPLICIT	(1 << 1)	/* Implicit or Explicit */
#define WL_TWT_FLOW_FLAG_UNANNOUNCED	(1 << 2)	/* Announced/Unannounced */
#define WL_TWT_FLOW_FLAG_TRIGGER	(1 << 3)	/* Trigger based */
#define WL_TWT_FLOW_FLAG_PROTECTION	(1 << 4)	/* TXOP uses NAV protection */

/* TWT id (bcast and individual) */
#define WL_TWT_ID_BCAST_AUTO		0	/* 0 is reserved by spec. Use it for auto select */
#define WL_TWT_ID_BCAST_MIN		1	/* Min Bcast ID, 0 is reserved to address all */
#define WL_TWT_ID_BCAST_MAX		31	/* Max Bcast ID, 5 bit field */
#define WL_TWT_ID_INDV_MIN		0	/* Min Individual ID */
#define WL_TWT_ID_INDV_MAX		7	/* Max Individual ID, 3 bit field */

/* TWT Setup Commands (individual twt setup usage) */
#define WL_TWT_SETUP_CMD_REQUEST	0	/* Request TWT, default method. */
#define WL_TWT_SETUP_CMD_SUGGEST	1	/* Suggest TWT, final params may deviate */
#define WL_TWT_SETUP_CMD_DEMAND		2	/* Demand TWT, params of TWT SP are set by host */

/* TWT descriptor can be used/returned for both AP and non-AP */
typedef struct {
	/* Flow attributes */
	uint8 flow_flags;		/* See WL_TWT_FLOW_FLAG_XXXX above */
	uint8 id;			/* Bcast ID or Flow ID depending on flags */
	/* TWT SP information */
	uint8 setup_command;		/* Indv. setup command, see WL_TWT_SETUP_CMD_... */
	/* Wake interval and persistence is mantissa X ( 2 ^ exponent) usecs */
	uint16 wake_interval_mantissa;	/* TWT Wake Interval Mantissa */
	uint8 wake_interval_exponent;	/* TWT Wake Interval Exponent (5 bits) */
	uint8 wake_duration;		/* Nominal Minimum TWT Wake Duration (unit is 256us) */
	uint8 persistence_mantissa;	/* Broadcast TWT Persistence Mantissa (8 bits) */
	uint8 persistence_exponent;	/* Broadcast TWT Persistence Exponent (3 bits) */
	uint8 channel;			/* Individual Channel bitmap */
} wl_twt_sdesc_t;

#define WL_TWT_SETUP_VER	1	/* Version 1 was introduced on AX spec */

/* HE TWT Setup command */
typedef struct {
	/* structure control */
	uint16 version;		/* structure version */
	uint16 length;		/* data length (starting after this field) */
	/* peer address */
	struct ether_addr peer;	/* leave it all 0s' for non-AP */
	/* setup descriptor */
	wl_twt_sdesc_t desc;
} wl_twt_setup_t;

#define WL_TWT_TEARDOWN_VER	1	/* Version 1 was introduced on AX spec */

/* HE TWT Teardown command */
typedef struct {
	/* structure control */
	uint16 version;		/* structure version */
	uint16 length;		/* data length (starting after this field) */
	/* peer address */
	struct ether_addr peer;	/* leave it all 0s' for non-AP */
	/* flow attributes */
	uint8 flow_flags;	/* Only WL_TWT_FLOW_FLAG_BROADCAST is applicable */
	uint8 id;		/* Bcast ID or Flow ID depending on flags */
} wl_twt_teardown_t;

/* twt information descriptor */
typedef struct {
	uint8 flow_flags;	/* See WL_TWT_INFO_FLAG_XXX below */
	uint8 id;		/* Bcast ID or Flow ID depending on flags */
	uint8 pad[2];
	uint32 next_twt_h;
	uint32 next_twt_l;
} wl_twt_desc_t;

/* Flow flags */
#define WL_TWT_INFO_FLAG_RESP_REQ	(1<<0)	/* Request response */

#define WL_TWT_INFO_VER	1	/* Version 1 was introduced on AX spec */

/* HE TWT Information command */
typedef struct {
	/* structure control */
	uint16 version;	/* structure version */
	uint16 length;	/* data length (starting after this field) */
	/* peer address */
	struct ether_addr peer;	/* leave it all 0s' for AP */
	uint8 pad[2];
	/* information descriptor */
	wl_twt_desc_t desc;
} wl_twt_info_t;

#define WL_TWT_LIST_VER	1	/* Version 1 was introduced on AX spec */

/* HE TWT List command */
typedef struct {
	/* structure control */
	uint16 version;		/* structure version */
	uint16 length;		/* data length (starting after this field) */
	/* peer address */
	struct ether_addr peer;	/* leave it all 0s' for non-AP */
	uint8 bcast_count;	/* Number of bcast descriptors returned in desc */
	uint8 indv_count;	/* Number of individual descriptors returned in desc */
	/* information descriptor, dynamic length (bcast_count + indv_count) */
	wl_twt_sdesc_t desc[0];	/* First the bcast descriptors, then the indv descriptors */
} wl_twt_list_t;

/* Current version for wlc_clm_power_limits_req_t structure and flags */
#define WLC_CLM_POWER_LIMITS_REQ_VERSION 1
/* "clm_power_limits" iovar request structure */
typedef struct wlc_clm_power_limits_req {
	/* Input. Structure and flags version */
	uint32 version;
	/* Full length of buffer (includes this structure and space for TLV-encoded PPR) */
	uint32 buflen;
	/* Input. Flags (see WLC_CLM_POWER_LIMITS_INPUT_FLAG_... below) */
	uint32 input_flags;
	/* Input. CC of region whose data is being requested */
	char cc[WLC_CNTRY_BUF_SZ];
	/* Input. Channel/subchannel in chanspec_t format */
	uint32 chanspec;
	/* Subchannel encoded as clm_limits_type_t */
	uint32 clm_subchannel;
	/* Input. 0-based antenna index */
	uint32 antenna_idx;
	/* Output. General flags (see WLC_CLM_POWER_LIMITS_OUTPUT_FLAG_... below) */
	uint32 output_flags;
	/* Output. 2.4G country flags, encoded as clm_flags_t enum */
	uint32 clm_country_flags_2g;
	/* Output. 5G country flags, encoded as clm_flags_t enum */
	uint32 clm_country_flags_5g;
	/* Output. Length of TLV-encoded PPR data that follows this structure */
	uint32 ppr_tlv_size;
	/* Output. Beginning of buffer for TLV-encoded PPR data */
	uint8 ppr_tlv[1];
} wlc_clm_power_limits_req_t;

/* Input. Do not apply SAR limits */
#define WLC_CLM_POWER_LIMITS_INPUT_FLAG_NO_SAR				0x00000001
/* Input. Do not apply board limits */
#define WLC_CLM_POWER_LIMITS_INPUT_FLAG_NO_BOARD			0x00000002
/* Output. Limits taken from product-specific country data */
#define WLC_CLM_POWER_LIMITS_OUTPUT_FLAG_PRODUCT_LIMITS			0x00000001
/* Output. Limits taken from product-specific worldwide data */
#define WLC_CLM_POWER_LIMITS_OUTPUT_FLAG_WORLDWIDE_LIMITS		0x00000002
/* Output. Limits taken from country-default (all-product) data */
#define WLC_CLM_POWER_LIMITS_OUTPUT_FLAG_DEFAULT_COUNTRY_LIMITS		0x00000004

/*
 * WOG (Wake On Googlecast)
 */

#define MAX_GCAST_APPID_CNT_LIMIT 50
#define MAX_DNS_LABEL 63

typedef struct wog_appid {
	uint8 appID[MAX_DNS_LABEL+1];
} wog_appid_t;

enum {
	WOG_APPID_ADD,
	WOG_APPID_DEL,
	WOG_APPID_CLEAR,
	WOG_APPID_LIST,
	WOG_MAX_APPID_CNT
};

#define WOG_APPID_IOV_VER 1
typedef struct wog_appid_iov {
	/* version for iovar */
	uint32 ver;
	/* add/del/clear/list operation */
	uint32 operation;
	/* for adding or deleting multiple items */
	/* for WOG_MAX_APPID_CNT, this value is used for max count for AppID */
	uint32 cnt;
	/* Application IDs */
	/* If FW found an AppID from this list, FW will respond to discovery */
	/* without wake up the host */
	wog_appid_t appids[1];
} wog_appid_iov_t;

/* dns service record */
/* service name : _googlecast */
typedef struct wog_srv_record {
	uint32 ttl;
	uint16 port; /* tcp 8008 or 8009 */
	uint8 PAD[2];
} wog_srv_record_t;

#define GCAST_MAX_MODEL_NAME_LEN 16
#define GCAST_MAX_FNAME_LEN 64
#define GCAST_MAX_RS_LEN 60

#define GCAST_UUID_LEN 32
#define GCAST_PUBLICKEY_ID_LEN 64
#define GCAST_VER_LEN 2
typedef struct wog_txt_record {
	uint32 ttl;
	/* id : UUID for the receiver */
	char id[GCAST_UUID_LEN+1];

	/* Cast protocol version supported. Begins at 2 */
	/* and is incremented by 1 with each version */
	char ver[GCAST_VER_LEN+1];

	/* 256bit receiver Subject Public Key Identifier from the SSL cert */
	char public_key[GCAST_PUBLICKEY_ID_LEN+1];

	/* A bitfield of device capabilities. */
	/* bit 0 : video_out (1:has video out, 0:no video) */
	/* bit 1 : video_in */
	/* bit 2 : audio_out */
	/* bit 3 : audio_in */
	/* bit 4 : dev_mode */
	/*	 (1:dev mode enabled, 0: not enabled) */
	char capability;

	/* Receiver status flag 0:IDLE, 1(BUSY/JOIN) */
	/* IDLE : The receiver is idle */
	/*	   and doesn't need to be connected now. */
	/* BUSY/JOIN : The receiver is hosting an activity */
	/*	 and invites the sender to join */
	char receiver_status_flag;

	uint8 PAD0[1];

	char friendly_name[GCAST_MAX_FNAME_LEN+1];
	uint8 PAD1[3];

	char model_name[GCAST_MAX_MODEL_NAME_LEN+1];
	uint8 PAD2[3];

	/* Receiver Status text for Cast Protocol v2 */
	/* Spec says that if the status text exceeds 60 characters in length, */
	/* it is truncated at 60 caracters and */
	/* a UTF-8 ellipsis character is appended to indicate trucation. */
	/* But our dongle won't use UTF-8 ellipsis. It's not a big deal. */
	char receiver_status[GCAST_MAX_RS_LEN+1];
	uint8 PAD3[3];
} wog_txt_record_t;

/* ip will be taken from the ip of wog_info_t */
typedef struct wog_a_record {
	uint32 ttl;
} wog_a_record_t;

/* Google Cast protocl uses mDNS SD for its discovery */
#define WOG_SD_RESP_VER 1
typedef struct wog_sd_resp {
	/* version for iovar */
	int32 ver;
	/* device name of Google Cast receiver */
	char device_name[MAX_DNS_LABEL+1];
	/* IP address of Google Cast receiver */
	uint8 ip[4];
	/* ttl of PTR response */
	uint32 ptr_ttl;
	/* DNS TXT record */
	wog_txt_record_t txt;
	/* DNS SRV record */
	wog_srv_record_t srv;
	/* DNS A record */
	wog_a_record_t a;
} wog_sd_resp_t;

#define WL_MBO_IOV_MAJOR_VER 1
#define WL_MBO_IOV_MINOR_VER 1
#define WL_MBO_IOV_MAJOR_VER_SHIFT 8
#define WL_MBO_IOV_VERSION \
	((WL_MBO_IOV_MAJOR_VER << WL_MBO_IOV_MAJOR_VER_SHIFT)| WL_MBO_IOV_MINOR_VER)

enum wl_mbo_cmd_ids {
	WL_MBO_CMD_ADD_CHAN_PREF = 1,
	WL_MBO_CMD_DEL_CHAN_PREF = 2,
	WL_MBO_CMD_LIST_CHAN_PREF = 3,
	WL_MBO_CMD_CELLULAR_DATA_CAP = 4,
	WL_MBO_CMD_DUMP_COUNTERS = 5,
	WL_MBO_CMD_CLEAR_COUNTERS = 6,
	WL_MBO_CMD_FORCE_ASSOC = 7,
	WL_MBO_CMD_BSSTRANS_REJECT = 8,
	WL_MBO_CMD_SEND_NOTIF = 9,
	WL_MBO_CMD_AP_ATTRIBUTE = 10,
	WL_MBO_CMD_AP_ASSOC_DISALLOWED = 11,
	WL_MBO_CMD_AP_FWD_GAS_RQST_TO_APP = 12,
	WL_MBO_CMD_AP_ENAB = 13,
	/* Add before this !! */
	WL_MBO_CMD_LAST
};

enum wl_mbo_xtlv_id {
	WL_MBO_XTLV_OPCLASS			= 0x1,
	WL_MBO_XTLV_CHAN			= 0x2,
	WL_MBO_XTLV_PREFERENCE			= 0x3,
	WL_MBO_XTLV_REASON_CODE			= 0x4,
	WL_MBO_XTLV_CELL_DATA_CAP		= 0x5,
	WL_MBO_XTLV_COUNTERS			= 0x6,
	WL_MBO_XTLV_ENABLE			= 0x7,
	WL_MBO_XTLV_SUB_ELEM_TYPE		= 0x8,
	WL_MBO_XTLV_AP_ATTR			= 0x9,
	WL_MBO_XTLV_AP_ASSOC_DISALLOWED		= 0xA,
	WL_MBO_XTLV_AP_FWD_GAS_RQST_TO_APP	= 0xB,
	WL_MBO_XTLV_AP_ENAB	=		0xC
};

typedef struct wl_mbo_counters {
	/* No of transition req recvd */
	uint16 trans_req_rcvd;
	/* No of transition req with disassoc imminent */
	uint16 trans_req_disassoc;
	/* No of transition req with BSS Termination */
	uint16 trans_req_bss_term;
	/* No of trans req w/ unspecified reason */
	uint16 trans_resn_unspec;
	/* No of trans req w/ reason frame loss */
	uint16 trans_resn_frm_loss;
	/* No of trans req w/ reason traffic delay */
	uint16 trans_resn_traffic_delay;
	/* No of trans req w/ reason insufficient buffer */
	uint16 trans_resn_insuff_bw;
	/* No of trans req w/ reason load balance */
	uint16 trans_resn_load_bal;
	/* No of trans req w/ reason low rssi */
	uint16 trans_resn_low_rssi;
	/* No of trans req w/ reason excessive retransmission */
	uint16 trans_resn_xcess_retransmn;
	/* No of trans req w/ reason gray zone */
	uint16 trans_resn_gray_zone;
	/* No of trans req w/ reason switch to premium AP */
	uint16 trans_resn_prem_ap_sw;
	/* No of transition rejection sent */
	uint16 trans_rejn_sent;
	/* No of trans rejn reason excessive frame loss */
	uint16 trans_rejn_xcess_frm_loss;
	/* No of trans rejn reason excessive traffic delay */
	uint16 trans_rejn_xcess_traffic_delay;
	/* No of trans rejn reason insufficient QoS capability */
	uint16 trans_rejn_insuffic_qos_cap;
	/* No of trans rejn reason low RSSI */
	uint16 trans_rejn_low_rssi;
	/* No of trans rejn reason high interference */
	uint16 trans_rejn_high_interference;
	/* No of trans rejn reason service unavilable */
	uint16 trans_rejn_service_unavail;
	/* No of beacon request rcvd */
	uint16 bcn_req_rcvd;
	/* No of beacon report sent */
	uint16 bcn_rep_sent;
	/* No of null beacon report sent */
	uint16 null_bcn_rep_sent;
	/* No of wifi to cell switch */
	uint16 wifi_to_cell;
} wl_mbo_counters_t;

#define WL_FILS_IOV_MAJOR_VER 1
#define WL_FILS_IOV_MINOR_VER 1
#define WL_FILS_IOV_MAJOR_VER_SHIFT 8
#define WL_FILS_IOV_VERSION \
	((WL_FILS_IOV_MAJOR_VER << WL_FILS_IOV_MAJOR_VER_SHIFT)| WL_FILS_IOV_MINOR_VER)

enum wl_fils_cmd_ids {
	WL_FILS_CMD_ADD_IND_IE		= 1,
	WL_FILS_CMD_ADD_AUTH_DATA	= 2,	/* Deprecated, kept to prevent ROM invalidation */
	WL_FILS_CMD_ADD_HLP_IE		= 3,
	WL_FILS_CMD_ADD_CONNECT_PARAMS	= 4,
	WL_FILS_CMD_GET_CONNECT_PARAMS	= 5,
	/* Add before this !! */
	WL_FILS_CMD_LAST
};

enum wl_fils_xtlv_id {
	WL_FILS_XTLV_IND_IE		= 0x1,
	WL_FILS_XTLV_AUTH_DATA		= 0x2,	/* Deprecated, kept to prevent ROM invalidation */
	WL_FILS_XTLV_HLP_IE		= 0x3,
	WL_FILS_XTLV_ERP_USERNAME	= 0x4,
	WL_FILS_XTLV_ERP_REALM		= 0x5,
	WL_FILS_XTLV_ERP_RRK		= 0x6,
	WL_FILS_XTLV_ERP_NEXT_SEQ_NUM	= 0x7,
	WL_FILS_XTLV_KEK		= 0x8,
	WL_FILS_XTLV_PMK		= 0x9,
	WL_FILS_XTLV_TK			= 0xa,
	WL_FILS_XTLV_PMKID		= 0xb
};

#define WL_OCE_IOV_MAJOR_VER 1
#define WL_OCE_IOV_MINOR_VER 1
#define WL_OCE_IOV_MAJOR_VER_SHIFT 8
#define WL_OCE_IOV_VERSION \
	((WL_OCE_IOV_MAJOR_VER << WL_OCE_IOV_MAJOR_VER_SHIFT)| WL_OCE_IOV_MINOR_VER)

#define OCE_MIN_FD_TX_PERIOD	20
#define OCE_MAX_FD_TX_PERIOD	50
#define OCE_MIN_RSSI_TH	-90
#define OCE_MAX_RSSI_TH -60

enum wl_oce_cmd_ids {
	WL_OCE_CMD_ENABLE = 1,
	WL_OCE_CMD_PROBE_DEF_TIME = 2,
	WL_OCE_CMD_FD_TX_PERIOD = 3,
	WL_OCE_CMD_FD_TX_DURATION = 4,
	WL_OCE_CMD_RSSI_TH = 5,
	WL_OCE_CMD_RWAN_LINKS = 6,
	WL_OCE_CMD_CU_TRIGGER = 7,
	/* Add before this !! */
	WL_OCE_CMD_LAST
};

enum wl_oce_xtlv_id {
	WL_OCE_XTLV_ENABLE  = 0x1,
	WL_OCE_XTLV_PROBE_DEF_TIME  = 0x2,
	WL_OCE_XTLV_FD_TX_PERIOD    = 0x3,
	WL_OCE_XTLV_FD_TX_DURATION  = 0x4,
	WL_OCE_XTLV_RSSI_TH = 0x5,
	WL_OCE_XTLV_RWAN_LINKS = 0x6,
	WL_OCE_XTLV_CU_TRIGGER = 0x7
};

#define WL_ESP_IOV_MAJOR_VER 1
#define WL_ESP_IOV_MINOR_VER 1
#define WL_ESP_IOV_MAJOR_VER_SHIFT 8
#define WL_ESP_IOV_VERSION \
	((WL_ESP_IOV_MAJOR_VER << WL_ESP_IOV_MAJOR_VER_SHIFT)| WL_ESP_IOV_MINOR_VER)

enum wl_esp_cmd_ids {
	WL_ESP_CMD_ENABLE = 1,
	WL_ESP_CMD_STATIC = 2,
	/* Add before this !! */
	WL_ESP_CMD_LAST
};

enum wl_esp_xtlv_id {
	WL_ESP_XTLV_ENABLE  = 0x1,
	WL_ESP_XTLV_STATIC_AC    = 0x2,	/* access category */
	WL_ESP_XTLV_STATIC_TYPE  = 0x3,	/* data type */
	WL_ESP_XTLV_STATIC_VAL   = 0x4
};

/* otpread command */
#define WL_OTPREAD_VER 1

typedef struct {
	uint16 version;		/* cmd structure version */
	uint16 cmd_len;		/* cmd struct len */
	uint32 rdmode;		/* otp read mode */
	uint32 rdoffset;	/* byte offset into otp to start read */
	uint32 rdsize;		/* number of bytes to read */
} wl_otpread_cmd_t;

/* "otpecc_rows" command */
typedef struct {
	uint16 version;		/* version of this structure */
	uint16 len;			/* len in bytes of this structure */
	uint32 cmdtype;		/* command type : 0 : read row data, 1 : ECC lock  */
	uint32 rowoffset;	/* start row offset */
	uint32 numrows;		/* number of rows */
	uint8  rowdata[];	/* read rows data */
} wl_otpecc_rows_t;

#define WL_OTPECC_ROWS_VER  1

#define WL_OTPECC_ROWS_CMD_READ  0
#define WL_OTPECC_ROWS_CMD_LOCK  1

#define WL_OTPECC_ARGIDX_CMDTYPE		0	/* command type */
#define WL_OTPECC_ARGIDX_ROWOFFSET		1	/* start row offset */
#define WL_OTPECC_ARGIDX_NUMROWS		2	/* number of rows */

/* "otpeccrows" raw data size per row */
#define WL_ECCDUMP_ROW_SIZE_BYTE	6 /* 4 bytes row data + 2 bytes ECC status */
#define WL_ECCDUMP_ROW_SIZE_WORD	3

/* otpECCstatus */
#define OTP_ECC_ENAB_SHIFT		13
#define OTP_ECC_ENAB_MASK		0x7
#define OTP_ECC_CORR_ST_SHIFT	12
#define OTP_ECC_CORR_ST_MASK	0x1
#define OTP_ECC_DBL_ERR_SHIFT	11
#define OTP_ECC_DBL_ERR_MASK	0x1
#define OTP_ECC_DED_ST_SHIFT	10
#define OTP_ECC_DED_ST_MASK		0x1
#define OTP_ECC_SEC_ST_SHIFT	9
#define OTP_ECC_SEC_ST_MASK		0x1
#define OTP_ECC_DATA_SHIFT		0
#define OTP_ECC_DATA_MASK		0x7f

/* OTP_ECC_CORR_ST field */
#define OTP_ECC_MODE		1
#define OTP_NO_ECC_MODE		0

/* OTP_ECC_ENAB field (bit15:13) :
 * When 2 or 3 bits are set,
 * it indicates that OTP ECC is enabled on the last row read.
 * Otherwise, ECC is disabled
 */
#define OTP_ECC_ENAB(val) \
	(bcm_bitcount((uint8 *)&(val), sizeof(uint8)) > 1)

#define WL_LEAKY_AP_STATS_GT_TYPE	0
#define WL_LEAKY_AP_STATS_PKT_TYPE	1
typedef struct wlc_leaked_infra_guard_marker {
	/* type field for this TLV: WL_LEAKY_AP_STATS_GT_TYPE */
	uint16  type;
	/* length field for this TLV */
	uint16  len;
	/* guard sample sequence number; Updated by 1 on every guard sample */
	uint32  seq_number;
	/* Guard time start time (tsf; PS indicated and acked) */
	uint32  start_time;
	/* tsf timestamp for the GT end event */
	uint32  gt_tsf_l;
	/* Guard time period in ms */
	uint16  guard_duration;
	/* Number PPDUs in the notification */
	uint16  num_pkts;
	/* Flags to indicate some states see below */
	uint8   flag;
	/* pad for 32-bit alignment */
	uint8   reserved[3];
} wlc_leaked_infra_guard_marker_t;

/* Flag information */
#define WL_LEAKED_GUARD_TIME_NONE	0               /* Not in any guard time */
#define WL_LEAKED_GUARD_TIME_FRTS	(0x01 << 0)     /* Normal FRTS power save */
#define WL_LEAKED_GUARD_TIME_SCAN	(0x01 << 1)     /* Channel switch due to scanning */
#define WL_LEAKED_GUARD_TIME_AWDL_PSF	(0x01 << 2)     /* Channel switch due to AWDL PSF */
#define WL_LEAKED_GUARD_TIME_AWDL_AW	(0x01 << 3)     /* Channel switch due to AWDL AW */
#define WL_LEAKED_GUARD_TIME_INFRA_STA	(0x01 << 4)	/* generic type infra sta channel switch */
#define WL_LEAKED_GUARD_TIME_TERMINATED (0x01 << 7)     /* indicate a GT is terminated early */

typedef struct wlc_leaked_infra_packet_stat {
	uint16  type;			/* type field for this TLV: WL_LEAKY_AP_STATS_PKT_TYPE */
	uint16  len;			/* length field for this TLV */
	uint16  ppdu_len_bytes;		/* PPDU packet length in bytes */
	uint16  num_mpdus;		/* number of the MPDUs in the PPDU */
	uint32  ppdu_time;		/* PPDU arrival time at the begining of the guard time */
	uint32  rate;			/* PPDU packet rate; Received packet's data rate */
	uint16  seq_number;		/* sequence number */
	int8    rssi;			/* RSSI */
	uint8   tid;			/* tid */
} wlc_leaked_infra_packet_stat_t;

/* Wake timer structure definition */
#define WAKE_TIMER_VERSION 1
#define WAKE_TIMER_NOLIMIT 0xFFFF

typedef struct wake_timer {
	uint16 ver;
	uint16 len;
	uint16 limit;   /* number of events to deliver
			* 0-disable, 0xffff-indefinite, num_events otherwise
			*/
	uint16 count;	/* number of events delivered since enable (get only) */
	uint16 period;	/* timeout/period in milliseconds */
} wake_timer_t;

typedef struct wl_desense_restage_gain {
	uint16 version;
	uint16 length;
	uint32 band;
	uint8 num_cores;
	uint8 desense_array[WL_TX_CHAINS_MAX];
	uint8 PAD[3];
} wl_desense_restage_gain_t;

#define MAX_UCM_CHAINS 5
#define MAX_UCM_PROFILES 4
#define UCM_PROFILE_VERSION_1 1

/* UCM per chain attribute struct */
typedef struct wlc_btcx_chain_attr {
	uint16 length;			/* chain attr length, version is same as profile version */
	int8 desense_level;		/* per chain desense level */
	int8 ack_pwr_strong_rssi;	/* per chain ack power at strong rssi */
	int8 ack_pwr_weak_rssi;		/* per chain ack power at weak rssi */
	int8 tx_pwr_strong_rssi;	/* per chain tx power at strong rssi */
	int8 tx_pwr_weak_rssi;		/* per chain tx power at weak rssi */
	uint8 PAD[1];			/* additional bytes for alignment */
} wlc_btcx_chain_attr_t;

typedef struct wlc_btcx_profile_v1 {
	uint16 version;			/* UCM profile version */
	uint16 length;			/* profile size */
	uint16 fixed_length;		/* size of the fixed portion of the profile */
	uint8 init;			/* profile initialized or not */
	uint8 chain_attr_count;		/* Number of elements in chain_attr array */
	uint8 profile_index;		/* profile index */
	uint8 mode_strong_wl_bt;	/* Mode under strong WLAN and BT RSSI */
	uint8 mode_weak_wl;		/* Mode under weak WLAN RSSI */
	uint8 mode_weak_bt;		/* Mode under weak BT RSSI */
	uint8 mode_weak_wl_bt;		/* Mode under weak BT and WLAN RSSI */
	int8 mode_wl_hi_lo_rssi_thresh;	/* Strong to weak WLAN RSSI threshold for mode selection */
	int8 mode_wl_lo_hi_rssi_thresh;	/* Weak to strong WLAN RSSI threshold for mode selection */
	int8 mode_bt_hi_lo_rssi_thresh;	/* Strong to weak BT RSSI threshold for mode selection */
	int8 mode_bt_lo_hi_rssi_thresh;	/* Weak to strong BT RSSI threshold for mode selection */
	int8 desense_wl_hi_lo_rssi_thresh;	/* Strong to weak RSSI threshold for desense */
	int8 desense_wl_lo_hi_rssi_thresh;	/* Weak to strong RSSI threshold for desense */
	int8 ack_pwr_wl_hi_lo_rssi_thresh;	/* Strong to weak RSSI threshold for ACK power */
	int8 ack_pwr_wl_lo_hi_rssi_thresh;	/* Weak to strong RSSI threshold for ACK power */
	int8 tx_pwr_wl_hi_lo_rssi_thresh;	/* Strong to weak RSSI threshold for Tx power */
	int8 tx_pwr_wl_lo_hi_rssi_thresh;	/* Weak to strong RSSI threshold for Tx power */
	uint8 PAD[1];				/* additional bytes for 4 byte alignment */
	wlc_btcx_chain_attr_t chain_attr[];	/* variable length array with chain attributes */
} wlc_btcx_profile_v1_t;

#define COEX_MODE_OFF  0
#define COEX_MODE_GPIO 1
#define COEX_MODE_GCI  2
#define COEX_MODE_ECI  3

typedef struct wlc_phy_btc_config {
	int     mode;
	int     wire;
	int     task;			/* BT task communicated to ucode: eSCO, BLE scan, etc. */
	uint32	gpio_mask;		/* Resolved GPIO Mask */
	uint32	gpio_out;		/* Resolved GPIO out pins */
	uint8	btc_wlan_gpio;		/* 3-wire coex, wlan active/TX_CONF GPIO num */
	uint8   coex_mode;              /* one of COEX_MODE_Xxx defs above */
} wlc_phy_btc_config_t;

#define SSSR_D11_RESET_SEQ_STEPS   5
#define SSSR_REG_INFO_VER   0

typedef struct sssr_reg_info {
	uint16 version;
	uint16 length;  /* length of the structure validated at host */
	struct {
		struct {
			uint32 pmuintmask0;
			uint32 pmuintmask1;
			uint32 resreqtimer;
			uint32 macresreqtimer;
			uint32 macresreqtimer1;
		} base_regs;
	} pmu_regs;
	struct {
		struct {
			uint32 intmask;
			uint32 powerctrl;
			uint32 clockcontrolstatus;
			uint32 powerctrl_mask;
		} base_regs;
	} chipcommon_regs;
	struct {
		struct {
			uint32 clockcontrolstatus;
			uint32 clockcontrolstatus_val;
		} base_regs;
		struct {
			uint32 resetctrl;
			uint32 itopoobb;
		} wrapper_regs;
	} arm_regs;
	struct {
		struct {
			uint32 ltrstate;
			uint32 clockcontrolstatus;
			uint32 clockcontrolstatus_val;
		} base_regs;
		struct {
			uint32 itopoobb;
		} wrapper_regs;
	} pcie_regs;
	struct {
		struct {
			uint32 ioctrl;
		} wrapper_regs;
		uint32 vasip_sr_addr;
		uint32 vasip_sr_size;
	} vasip_regs;
	struct {
		struct {
			uint32 xmtaddress;
			uint32 xmtdata;
			uint32 clockcontrolstatus;
			uint32 clockcontrolstatus_val;
		} base_regs;
		struct {
			uint32 resetctrl;
			uint32 itopoobb;
			uint32 ioctrl;
			uint32 ioctrl_resetseq_val[SSSR_D11_RESET_SEQ_STEPS];
		} wrapper_regs;
		uint32 sr_size;
	} mac_regs[MAX_NUM_D11CORES];
} sssr_reg_info_t;

/* ADaptive Power Save(ADPS) structure definition */
#define WL_ADPS_IOV_MAJOR_VER	1
#define WL_ADPS_IOV_MINOR_VER	0
#define WL_ADPS_IOV_MAJOR_VER_SHIFT	8
#define WL_ADPS_IOV_VER \
	((WL_ADPS_IOV_MAJOR_VER << WL_ADPS_IOV_MAJOR_VER_SHIFT) | WL_ADPS_IOV_MINOR_VER)

#define ADPS_NUM_DIR	2
#define ADPS_RX		0
#define ADPS_TX		1

#define WL_ADPS_IOV_MODE	0x0001
#define WL_ADPS_IOV_RSSI	0x0002
#define WL_ADPS_IOV_DUMP	0x0003
#define WL_ADPS_IOV_DUMP_CLEAR	0x0004

#define ADPS_SUMMARY_STEP_NUM   2
#define ADPS_SUMMARY_STEP_LOW	0
#define ADPS_SUMMARY_STEP_HIGH	1

#define ADPS_SUB_IOV_VERSION_1	1
#define ADPS_SUB_IOV_VERSION_2	2

typedef struct wl_adps_params_v1 {
	uint16 version;
	uint16 length;
	uint8 band;		/* band - 2G or 5G */
	uint8 mode;		/* operation mode, default = 0 (ADPS disable) */
	uint16 padding;
} wl_adps_params_v1_t;

typedef struct wl_adps_rssi {
	int32 thresh_hi;	/* rssi threshold to resume ADPS operation */
	int32 thresh_lo;	/* rssi threshold to suspend ADPS operation */
} wl_adps_rssi_t;

typedef struct wl_adps_rssi_params_v1 {
	uint16 version;
	uint16 length;
	uint8 band;
	uint8 padding[3];
	wl_adps_rssi_t rssi;
} wl_adps_rssi_params_v1_t;

typedef struct adps_stat_elem {
	uint32 duration;	/* each step duration time (mSec) */
	uint32 counts;		/* each step hit count number */
} adps_stat_elem_t;

typedef struct wl_adps_dump_summary_v1 {
	uint16 version;
	uint16 length;
	uint8 mode;					/* operation mode: On/Off */
	uint8 flags;					/* restrict flags */
	uint8 current_step;				/* current step */
	uint8 padding;
	adps_stat_elem_t stat[ADPS_SUMMARY_STEP_NUM];	/* statistics */
} wl_adps_dump_summary_v1_t;

typedef struct wlc_btc_2gchain_dis {
	uint16 ver;
	uint16 len;
	uint8 chain_dis;
	uint8 flag;
} wlc_btc_2gchain_dis_t;

#define WLC_BTC_2GCHAIN_DIS_REASSOC	0x1
#define WLC_BTC_2GCHAIN_DIS_VER1	0x1
#define WLC_BTC_2GCHAIN_DIS_VER1_LEN	6

enum wl_rpsnoa_cmd_ids {
	WL_RPSNOA_CMD_ENABLE = 1,
	WL_RPSNOA_CMD_STATUS,
	WL_RPSNOA_CMD_PARAMS,
	WL_RPSNOA_CMD_LAST
};

typedef struct rpsnoa_cmnhdr {
	uint16 ver;		/* cmd structure version */
	uint16 len;		/* cmd structure len */
	uint32 subcmd;
	uint32 cnt;
} rpsnoa_cmnhdr_t;

typedef struct rpsnoa_data {
	int16 band;
	int16 value;
} rpsnoa_data_t;

typedef struct rpsnoa_param {
	uint16 band;
	uint8 level;
	uint8 stas_assoc_check;
	uint32 pps;
	uint32 quiet_time;
} rpsnoa_param_t;

typedef struct rpsnoa_iovar {
	rpsnoa_cmnhdr_t hdr;
	rpsnoa_data_t data[1];
} rpsnoa_iovar_t;

typedef struct rpsnoa_iovar_params {
	rpsnoa_cmnhdr_t hdr;
	rpsnoa_param_t param[1];
} rpsnoa_iovar_params_t;

/* Per-interface reportable stats types */
enum wl_ifstats_xtlv_id {
	/* global */
	WL_IFSTATS_XTLV_SLICE_INDEX = 1,
	WL_IFSTATS_XTLV_IF_INDEX = 2,
	WL_IFSTATS_XTLV_MAC_ADDR = 3,
	WL_IFSTATS_XTLV_REPORT_CMD = 4,	/* Comes in an iovar */
	WL_IFSTATS_XTLV_BUS_PCIE = 5,

	/* Report data across all SCBs using ecounters */
	/* STA_info ecounters */
	WL_IFSTATS_XTLV_WL_STA_INFO_ECOUNTERS = 0x100,
	/* For AMPDU stat sub-types requested in a different format */
	/* these could be sum and report stats across slices. OR
	 * report sub-types in pairs so host can sum and add.
	 * Information sent here is across slices, therefore global
	 */
	WL_IFSTATS_XTLV_TX_AMPDU_STATS = 0x101,
	WL_IFSTATS_XTLV_RX_AMPDU_STATS = 0x102,

	/* Per-slice information
	 * Per-interface reporting could also include slice specific data
	 */
	/* xtlv container for reporting */
	WL_IFSTATS_XTLV_WL_SLICE = 0x301,
	/* Per-slice AMPDU stats */
	WL_IFSTATS_XTLV_WL_SLICE_TX_AMPDU_DUMP = 0x302,
	WL_IFSTATS_XTLV_WL_SLICE_RX_AMPDU_DUMP = 0x303,
	/* Per-slice BTCOEX stats */
	WL_IFSTATS_XTLV_WL_SLICE_BTCOEX = 0x304,
	/* V11_WLCNTRS used in ecounters */
	WL_IFSTATS_XTLV_WL_SLICE_V11_WLCNTRS = 0x305,
	/* V30_WLCNTRS Used in ecounters */
	WL_IFSTATS_XTLV_WL_SLICE_V30_WLCNTRS = 0x306,
	/* phy,ucode,scan pwrstats */
	WL_IFSTATS_XTLV_WL_SLICE_PWRSTATS_PHY = 0x307,
	WL_IFSTATS_XTLV_WL_SLICE_PWRSTATS_SCAN = 0x308,
	WL_IFSTATS_XTLV_WL_SLICE_PWRSTATS_WAKE_V2 = 0x309,
	/* Per-slice LTECOEX stats */
	WL_IFSTATS_XTLV_WL_SLICE_LTECOEX = 0x30A,
	/* TVPM ecounters */
	WL_IFSTATS_XTLV_WL_SLICE_TVPM = 0x30B,
	/* Per-interface */
	/* XTLV container for reporting */
	WL_IFSTATS_XTLV_IF = 0x501,
	/* Generic stats applicable to all IFs */
	WL_IFSTATS_XTLV_GENERIC = 0x502,
	/* Infra specific */
	WL_IFSTATS_XTLV_INFRA_SPECIFIC = 0x503,
	/* MGT counters infra and softAP */
	WL_IFSTATS_XTLV_MGT_CNT = 0x504,
	/* AMPDU stats on per-IF */
	WL_IFSTATS_XTLV_AMPDU_DUMP = 0x505,
	WL_IFSTATS_XTLV_IF_SPECIFIC = 0x506,
	WL_IFSTATS_XTLV_WL_PWRSTATS_AWDL = 0x507,
	WL_IFSTATS_XTLV_IF_LQM = 0x508
};

/* current version of wl_stats_report_t structure for request */
#define WL_STATS_REPORT_REQUEST_VERSION_V2	2

/* current version of wl_stats_report_t structure for response */
#define WL_STATS_REPORT_RESPONSE_VERSION_V2	2

/** Top structure of if_counters IOVar buffer */
typedef struct wl_stats_report {
	uint16	version;	/**< see version definitions above */
	uint16	length;		/**< length of data including all paddings. */
	uint8   data [];	/**< variable length payload:
				 * 1 or more bcm_xtlv_t type of tuples.
				 * each tuple is padded to multiple of 4 bytes.
				 * 'length' field of this structure includes all paddings.
				 */
} wl_stats_report_t;

/* interface specific mgt count */
#define WL_MGT_STATS_VERSION_V1	1
/* Associated stats type: WL_IFSTATS_MGT_CNT */
typedef struct {
	uint16	version;
	uint16	length;

	/* detailed control/management frames */
	uint32	txnull;
	uint32	rxnull;
	uint32	txqosnull;
	uint32	rxqosnull;
	uint32	txassocreq;
	uint32	rxassocreq;
	uint32	txreassocreq;
	uint32	rxreassocreq;
	uint32	txdisassoc;
	uint32	rxdisassoc;
	uint32	txassocrsp;
	uint32	rxassocrsp;
	uint32	txreassocrsp;
	uint32	rxreassocrsp;
	uint32	txauth;
	uint32	rxauth;
	uint32	txdeauth;
	uint32	rxdeauth;
	uint32	txprobereq;
	uint32	rxprobereq;
	uint32	txprobersp;
	uint32	rxprobersp;
	uint32	txaction;
	uint32	rxaction;
	uint32	txpspoll;
	uint32	rxpspoll;
} wl_if_mgt_stats_t;

#define WL_INFRA_STATS_VERSION_V1	1
/* Associated stats type: WL_IFSTATS_INFRA_SPECIFIC */
typedef struct wl_infra_stats {
	uint16 version;             /**< version of the structure */
	uint16 length;
	uint32 rxbeaconmbss;
	uint32 tbtt;
} wl_if_infra_stats_t;

#define LTECOEX_STATS_VER   1

typedef struct wlc_ltecoex_stats {
	uint16 version;	     /**< WL_IFSTATS_XTLV_WL_SLICE_LTECOEX */
	uint16 len;			/* Length of  wl_ltecx_stats structure */
	uint8 slice_index;	/* Slice unit of  wl_ltecx_stats structure */
	uint8 pad[3];	/* Padding */
	/* LTE noise based eCounters Bins
	 cumulative the wl_cnt_wlc_t and  wl_ctl_mgt_cnt_t
	 counter information based on LTE Coex interference level
	 */
	uint32	txframe_no_LTE;		/* txframe counter in no LTE Coex case */
	uint32	rxframe_no_LTE;		/* rxframe counter in no LTE Coex case */
	uint32	rxrtry_no_LTE;		/* rxrtry counter in no LTE Coex case */
	uint32	txretrans_no_LTE;	/* txretrans counter in no LTE Coex case */
	uint32	txnocts_no_LTE;		/* txnocts counter in no LTE Coex case */
	uint32	txrts_no_LTE;		/* txrts counter in no LTE Coex case */
	uint32	txdeauth_no_LTE;	/* txdeauth counter in no LTE Coex case */
	uint32	txassocreq_no_LTE;	/* txassocreq counter in no LTE Coex case */
	uint32	txassocrsp_no_LTE;		/* txassocrsp counter in no LTE Coex case */
	uint32	txreassocreq_no_LTE;	/* txreassocreq counter in no LTE Coex case */
	uint32	txreassocrsp_no_LTE;	/* txreassocrsp counter in no LTE Coex case */
	uint32	txframe_light_LTE;	/* txframe counter in light LTE Coex case */
	uint32	txretrans_light_LTE;	/* txretrans counter in light LTE Coex case */
	uint32	rxframe_light_LTE;	/* rxframe counter in light LTE Coex case */
	uint32	rxrtry_light_LTE;	/* rxrtry counter in light LTE Coex case */
	uint32	txnocts_light_LTE;	/* txnocts counter in light LTE Coex case */
	uint32	txrts_light_LTE;	/* txrts counter in light LTE Coex case */
	uint32	txdeauth_light_LTE;	/* txdeauth counter in light LTE Coex case */
	uint32	txassocreq_light_LTE;	/* txassocreq counter in light LTE Coex case */
	uint32	txassocrsp_light_LTE;	/* txassocrsp counter in light LTE Coex case */
	uint32	txreassocreq_light_LTE;	/* txreassocreq counter in light LTE Coex case */
	uint32	txreassocrsp_light_LTE;	/* txreassocrsp counter in light LTE Coex case */
	uint32	txframe_heavy_LTE;	/* txframe counter in heavy LTE Coex case */
	uint32	txretrans_heavy_LTE;	/* txretrans counter in heavy LTE Coex case */
	uint32	rxframe_heavy_LTE;	/* rxframe counter in heavy LTE Coex case */
	uint32	rxrtry_heavy_LTE;	/* rxrtry counter in heavy LTE Coex case */
	uint32	txnocts_heavy_LTE;	/* txnocts counter in heavy LTE Coex case */
	uint32	txrts_heavy_LTE;	/* txrts counter in heavy LTE Coex case */
	uint32	txdeauth_heavy_LTE;	/* txdeauth counter in heavy LTE Coex case */
	uint32	txassocreq_heavy_LTE;	/* txassocreq counter in heavy LTE Coex case */
	uint32	txassocrsp_heavy_LTE;	/* txassocrsp counter in heavy LTE Coex case */
	uint32	txreassocreq_heavy_LTE;	/* txreassocreq counter in heavy LTE Coex case */
	uint32	txreassocrsp_heavy_LTE;	/* txreassocrsp counter in heavy LTE Coex case */

	/* LTE specific ecounters */
	uint16	type4_txinhi_dur;	/* Duration of tx inhibit(in ms) due to Type4 */
	uint16	type4_nonzero_cnt;	/* Counts of none zero Type4 msg */
	uint16	type4_timeout_cnt;	/* Counts of Type4 timeout */
	uint16	rx_pri_dur;		/* Duration of wlan_rx_pri assertions */
	uint16	rx_pri_cnt;		/* Count of wlan_rx_pri assertions */
	uint16	type6_dur;		/* duration of LTE Tx power limiting assertions */
	uint16	type6_cnt;		/* Count of LTE Tx power limiting assertions */
	uint16	ts_prot_frm_cnt;	/* count of WLAN protection frames triggered by LTE coex */
	uint16	ts_gr_cnt;		/* count of intervals granted to WLAN in timesharing */
	uint16	ts_gr_dur;		/* duration granted to WLAN in timesharing */
} wlc_ltecoex_stats_t;

#define CSA_EVT_CSA_RXED        (1 << 0)
#define CSA_EVT_CSA_TIMEOUT     (1 << 1)
#define CSA_EVT_FROM_INFRA      (1 << 2)
typedef struct csa_event_data {
	chanspec_t chan_old;
	dot11_ext_csa_ie_t ecsa;
	dot11_mesh_csp_ie_t mcsp;
	dot11_wide_bw_chan_switch_ie_t wbcs;
	uint8 flags;
	uint8 pad[3];
} csa_event_data_t;

/* ifdef (WL_ASSOC_BCN_RPT) */
enum wl_bcn_report_cmd_id {
	WL_BCN_RPT_CMD_VER		= 0,
	WL_BCN_RPT_CMD_CONFIG		= 1,
	WL_BCN_RPT_CMD_VENDOR_IE	= 2,
	WL_BCN_RPT_CMD_LAST
};

/* beacon report specific macros */
#define WL_BCN_RPT_CCX_IE_OVERRIDE	(1u << 0)

/* beacon report specific macros */
#define WL_BCN_RPT_ASSOC_SCAN_UNSOLICITED_MODE	(1u << 1)
#define WL_BCN_RPT_ASSOC_SCAN_SOLICITED_MODE	(1u << 2)
#define WL_BCN_RPT_ASSOC_SCAN_MODE_SHIFT	(1)
#define WL_BCN_RPT_ASSOC_SCAN_MODE_MASK		(WL_BCN_RPT_ASSOC_SCAN_UNSOLICITED_MODE |\
							WL_BCN_RPT_ASSOC_SCAN_SOLICITED_MODE)
#define WL_BCN_RPT_ASSOC_SCAN_MODE_MAX		(WL_BCN_RPT_ASSOC_SCAN_MODE_MASK >> \
							WL_BCN_RPT_ASSOC_SCAN_MODE_SHIFT)
/* beacon report mode specific macro */
#define WL_BCN_RPT_ASSOC_SCAN_MODE_DEFAULT	WL_BCN_RPT_ASSOC_SCAN_UNSOLICITED_MODE

/* beacon report timeout config specific macros */
#define WL_BCN_RPT_ASSOC_SCAN_CACHE_TIMEOUT_DEFAULT	(120000)
#define WL_BCN_RPT_ASSOC_SCAN_CACHE_TIMEOUT_MIN		(60000)
#define WL_BCN_RPT_ASSOC_SCAN_CACHE_TIMEOUT_MAX		(0xFFFFFFFF)

/* beacon report cache count specific macros */
#define WL_BCN_RPT_ASSOC_SCAN_CACHE_COUNT_MIN		(0)
#define WL_BCN_RPT_ASSOC_SCAN_CACHE_COUNT_MAX		(8)
#define WL_BCN_RPT_ASSOC_SCAN_CACHE_COUNT_DEFAULT	(WL_BCN_RPT_ASSOC_SCAN_CACHE_COUNT_MAX)

#define WL_BCN_REPORT_CMD_VERSION		1
struct wl_bcn_report_cfg {
	uint32	flags;			/**< Flags that defines the operation/setting information */
	uint32	scan_cache_timeout;	/**< scan cache timeout value in millisec */
	uint32	scan_cache_timer_pend;	/**< Read only pending time for timer expiry in millisec */
	uint8	scan_cache_cnt;		/**< scan cache count */
};

/* endif (WL_ASSOC_BCN_RPT) */

/* Thermal, Voltage, and Power Mitigation */
#define TVPM_REQ_VERSION_1		1
#define TVPM_REQ_CURRENT_VERSION	TVPM_REQ_VERSION_1

/* tvpm iovar data */
typedef struct {
	uint16	version;		/* TVPM request version */
	uint16	length;			/* Length of the entire structure */

	uint16	req_type;		/* Request type: wl_tvpm_req_type_t */
	uint16	req_len;		/* Length of the following value */
	uint8	value[];		/* Variable length data depending on req_type */
} wl_tvpm_req_t;

/* tvpm iovar request types */
typedef enum {
	WL_TVPM_REQ_CLTM_INDEX,	/* req_value: uint32, range 1...100 */
	WL_TVPM_REQ_PPM_INDEX,	/* req_value: uint32, range 1...100 */
	WL_TVPM_REQ_ENABLE,	/* req_value: uint32, range 0...1 */
	WL_TVPM_REQ_STATUS,	/* req_value: none */
	WL_TVPM_REQ_MAX
} wl_tvpm_req_type_t;

/* structure for data returned by request type WL_TVPM_REQ_STATUS */
typedef struct wl_tvpm_status {
	uint16	enable;			/* whether TVPM is enabled */
	uint16	tx_dutycycle;		/* a percentage: 1-100 */
	int16	tx_power_backoff;	/* 0...-6 */
	uint16	num_active_chains;	/* 1...3 */
	int16	temp;			/* local temperature in degrees C */
	uint8	vbat;			/* local voltage in units of 0.1V */
	uint8	pad;
} wl_tvpm_status_t;

/* TVPM ecounters */
typedef struct wl_tvpm_ecounters_t {
	uint16	version;		/* version field */
	uint16	length;			/* byte length in wl_tvpm_ecounters_t starting at version */
	uint16	tx_dutycycle;		/* a percentage: 1-100 */
	int16	tx_power_backoff;	/* 0...-6 */
	uint16	num_active_chains;	/* 1...3 */
	int16	temp;			/* local temperature */
	uint8	vbat;			/* local voltage */
	uint8	cltm;			/* CLTM index  */
	uint8	ppm;			/* PPM index  */
	uint8	pad;			/* pad to align to uint16 */
} wl_tvpm_ecounters_t;
/*
 * BT log definitions
 */

/* common iovar struct */
typedef struct wl_btl {
	uint16 subcmd_id;	/* subcommand id */
	uint16 len;		/* total length of data[] */
	uint8 data[2];		/* subcommand data, variable length */
} wl_btl_t;

/* subcommand ids */
#define WL_BTL_SUBCMD_ENABLE	0	/* enable/disable logging */
#define WL_BTL_SUBCMD_STATS	1	/* statistics */
#define WL_BTL_SUBCMD_TEST	2	/* testing */

/* WL_BTL_SUBCMD_ENABLE data */
typedef struct wl_blt_enable {
	uint8 enable;			/* 1 - enable, 0 - disable */
	uint8 pad[3];			/* 4-byte struct alignment */
} wl_btl_enable_t;

/* WL_BTL_SUBCMD_STATS data */
typedef struct wl_blt_stats {
	uint32 bt_interrupt;		/* number of BT interrupts */
	uint32 config_req;		/* number of CONFIG_REQ */
	uint32 config_res_success;	/* number of CONFIG_RES successful */
	uint32 config_res_fail;		/* number of CONFIG_RES failed */
	uint32 log_req;			/* number of LOG_REQ */
	uint32 log_res_success;		/* number of LOG_RES successful */
	uint32 log_res_fail;		/* number of LOG_RES failed */
	uint32 indirect_read_fail;	/* number of indirect read fail */
	uint32 indirect_write_fail;	/* number of indirect write fail */
	uint32 dma_fail;		/* number of DMA failed */
	uint32 min_log_req_duration;	/* min log request duration in usec */
	uint32 max_log_req_duration;	/* max log request duration in usec */
} wl_btl_stats_t;

/* WL_BTL_SUBCMD_TEST data */
typedef struct wl_blt_test {
	uint32 test;		/* test data */
} wl_btl_test_t;

/* IOV AWD DATA */

/* AWD DATA structures */
typedef struct {
	uint8 version;	/* Extended trap version info */
	uint8 reserved;	/* currently unused */
	uint16 length;	/* Length of data excluding this header */
	uint8 data[];	/* this data is TLV of tags */
} awd_data_v1_t;

/* AWD TAG structure */
typedef struct {
	uint8 tagid;	/* one of AWD DATA TAGs numbers */
	uint8 length;	/* the data size represented by this field must be aligned to 32 bits */
	uint8 data[];	/* variable size, defined by length field */
} awd_tag_data_v1_t;

/* AWD information structures associated with AWD_DATA_Tags */
/* AWD_DATA_TAG_JOIN_CLASSIFICATION_INFO 10 */
typedef struct {
	uint8 assoc_type;		/* assoc type */
	uint8 assoc_state;		/* current state of assoc state machine */
	uint8 wpa_state;		/* wpa->state */
	uint8 wsec_portopen;		/* shows if security port is open */
	uint8 total_attempts_num;	/* total number of join attempts (bss_retries) */
	uint8 num_of_targets;		/* up to 3, in current design */
	uint8 reserved [2];		/* padding to get 32 bits alignment */
	uint32 wsec;			/* bsscfg->wsec */
	uint32 wpa_auth;		/* bsscfg->WPA_auth */
	uint32 time_to_join;		/* time duration to process WLC_SET_SSID request (ms) */
} join_classification_info_v1_t;

/* AWD_DATA_TAG_JOIN_TARGET_CLASSIFICATION_INFO 11 */
typedef struct {
	int8 rssi;			/* RSSI on current channel */
	uint8 cca;			/* CCA on current channel */
	uint8 channel;			/* current channel */
	uint8 num_of_attempts;		/* (bss_retries) up to 5 */
	uint8 oui[3];			/* the first three octets of the AP's address */
	uint8 reserved;			/* padding to get 32 bits alignment */
	uint32 time_duration;		/* time duration of current attempt (ms) */
} join_target_classification_info_v1_t;

/* AWD_DATA_TAG_ASSOC_STATE 12 */
typedef struct {
	uint8 assoc_state;		/* assoc type */
	uint8 reserved [3];		/* padding to get 32 bits alignment */
} join_assoc_state_v1_t;

/* AWD_DATA_TAG_CHANNEL 13 tag */
typedef struct {
	uint8 channel;			/* last attempt channel */
	uint8 reserved [3];		/* padding to get 32 bits alignment */
} join_channel_v1_t;

/* AWD_DATA_TAG_TOTAL_NUM_OF_JOIN_ATTEMPTS 14 */
typedef struct {
	uint8 total_attempts_num;	/* total number of join attempts (bss_retries) */
	uint8 reserved [3];		/* padding to get 32 bits alignment */
} join_total_attempts_num_v1_t;

/* IOV_ROAM_CACHE structures */

enum wl_rmc_report_cmd_id {
	WL_RMC_RPT_CMD_VER	= 0,
	WL_RMC_RPT_CMD_DATA	= 1,
	WL_RMC_RPT_CMD_LAST
};

enum wl_rmc_report_xtlv_id {
	WL_RMC_RPT_XTLV_VER		= 0x0,
	WL_RMC_RPT_XTLV_BSS_INFO	= 0x1,
	WL_RMC_RPT_XTLV_CANDIDATE_INFO	= 0x2
};

/* WL_RMC_RPT_XTLV_BSS_INFO */
typedef struct {
	int16 rssi;		/* current BSS RSSI */
	uint8 reason;		/* reason code for last full scan */
	uint8 status;		/* last status code for not roaming */
	uint32 fullscan_count;	/* number of full scans performed on current BSS */
	uint32 time_full_scan;	/* delta time (in ms) between cur time and full scan timestamp */
} rmc_bss_info_v1_t;

/* WL_RMC_RPT_XTLV_CANDIDATE_INFO */
typedef struct {
	int16 rssi;		/* last seen rssi */
	uint16 ctl_channel;	/* channel */
	uint32 time_last_seen;	/* delta time (in ms) between cur time and last seen timestamp */
	uint16 bss_load;	/* BSS load */
	uint8 bssid [6];	/* padding to get 32 bits alignment */
} rmc_candidate_info_v1_t;

/* Event aggregation config */
#define EVENT_AGGR_CFG_VERSION		1
#define EVENT_AGGR_DISABLED			0x0
#define EVENT_AGGR_ENABLED			0x1

#define EVENT_AGGR_BUFSIZE_MAX		1512
#define EVENT_AGGR_BUFSIZE_MIN		512

#define EVENT_AGGR_FLUSH_TIMEOUT_DEFAULT	100
#define EVENT_AGGR_FLUSH_TIMEOUT_MAX		2000
#define EVENT_AGGR_NUM_EVENTS_FLUSH		5
typedef struct event_aggr_config {
	uint16 version;
	uint16 len;
	uint16 flags;			/* bit 0 to enable/disable the feature */
	uint16 bufsize;			/* Aggregate buffer size */
	uint16 flush_timeout;		/* Timeout for event flush */
	uint16 num_events_flush;	/* Number of events aggregated before flush */
} event_aggr_config_t;

typedef struct wl_atm_staperc {
	uint16 version;		/* Version */
	uint16 length;		/* Length */
	struct ether_addr ea;	/* MAC addr */
	uint8 perc;		/* SCB ATM percentage */
} wl_atm_staperc_t;

#define WL_TRF_BE 0
#define WL_TRF_BK 1
#define WL_TRF_VI 2
#define WL_TRF_VO 3
#define WL_TRF_TO 4
#define WL_TRF_DU 5
#define WL_TRF_AE 0x10		/* This field is used to enable and disable the stats
				 * collection per access access category.
				 * For sta mode it becomes auto enable flag. When it is
				 * enabled or disabled, it effects all associated stas.
				 * For ap and mac mode it effects own configurations.
				 */
#define WL_TRF_PM 0x20		/* mode to enable disable access category flags per MAC */
#define WL_TRF_MAX_QUEUE 5	/* Max number of access categories queues supported */
#define WL_TRF_MAX_SECS 8	/* Max value suported for num_secs window */

#define WL_TRF_MODE_AP 1
#define WL_TRF_MODE_STA 2
#define WL_TRF_MODE_MAC 3
#define WL_TRF_MODE_WL_FEATURE 4
#define WL_TRF_MODE_CFG_FEATURE 5

typedef struct wl_trf_thold {
	uint16 version;
	uint16 length;
	uint16 ap;
	uint16 type;
	uint16 num_secs;
	uint16 thresh;
	uint16 auto_enable;
	struct ether_addr mac;
} wl_trf_thold_t;
struct get_trf {
	uint16 version;
	uint16 length;
	uint16 enabled;
	uint16 auto_enable;
	uint16 ap;
	wl_trf_thold_t data[WL_TRF_MAX_QUEUE];
};

typedef struct wl_traffic_thresh {
	uint16 version;
	uint16 length;
	uint16 mode;
	uint16 type;
	uint16 num_secs;
	uint16 thresh;
	uint16 enable;
	struct ether_addr mac;
} wl_traffic_thresh_t;
struct get_traffic_thresh {
	uint16 version;
	uint16 length;
	uint16 auto_enable;
	uint16 mode;
	uint16 intf_enab;
	uint16 bss_enab;
	wl_traffic_thresh_t data[WL_TRF_MAX_QUEUE];
};

/* WET host ip and mac parameter configuration */
typedef struct wet_host {
	uint8 buf[6]; /* ip or mac */
	uint8 bssidx;
} wet_host_t;

/* per-SCB per rate histogram defines */
#define WL_HISTO_VER_1		1u
#define WL_HISTO_TYPE_RATE_MAP1 1u /* map of rate indexed counters up to 80MHz */
#define WL_HISTO_TYPE_RATE_MAP2 2u /* map of rate indexed counters up to 160MHz */

#define WL_HISTO_VER_1_FIXED_LEN 12 /* length of fixed fields */

/* when reading, use fixed_length as offset to get to variable length field instead of rpt->data */
#define WL_HISTO_DATA(rpt) ((wl_rate_histo_maps1_t *) (((char *)(rpt)) + (rpt)->fixed_len))

/* Ver 1: rate mode */
#define WL_HISTO_RATE_TYPE_LEGACY	0x00u /* map is index of 11bgn rates/4 */
#define WL_HISTO_RATE_TYPE_HT		0x01u /* map is has ht_mcs number; no bw/sgi info */
#define WL_HISTO_RATE_TYPE_VHT		0x02u /* map is has vht mcs+12*(nss-1); no bw/sgi info */
#define WL_HISTO_RATE_NUM_TYPES		3	/* number of types */

/* VHT: map index to MCS */
#define WL_HISTO_RATE_DECODE_VHT_MCS(index) ((index) % 12)
/* VHT: map index to NSS */
#define WL_HISTO_RATE_DECODE_VHT_NSS(index) (((index) / 12) + 1)

/* structure to report histogram of rates */
typedef struct wl_rate_histo_report {
	uint8 ver;			/* version */
	uint8 type;			/* histogram type */
	uint16 length;			/* length of this structure including length of data */
	struct ether_addr ea;		/* MAC address of the AP/client/peer */
	uint16 fixed_len;		/* length of fixed fields till start of variable data */
	/* optional fields for same ver and type can be added here */
	uint8 data[];			/* data holds struct depending on ver and type */
} wl_rate_histo_report_t;

/* standard number of VHT/HT/Legacy rates for a given bandwidth till NSS 4x4 */
#define WL_NUM_VHT_RATES		(48)
#define WL_NUM_HT_RATES			(32)
#define WL_NUM_LEGACY_RATES		(12)

/* rate histogram type 1 supports 20/40/80 MHz */
#define WL_HISTO_MAP1_NUM_BW		(3) /* 20, 40, 80 MHz */

/* Histogram index to BW. Histogram has WL_NUM_LEGACY_RATES
 * followed by WL_NUM_VHT_RATES for each bandwidth
 */
#define WL_HISTO_INDEX2BW(i)	((i) < WL_NUM_LEGACY_RATES ? 20 : \
		20 * (1 << (((i) - WL_NUM_LEGACY_RATES) / WL_NUM_VHT_RATES)))

/* scb rate histogram mapping array length for histo version 1, rate map type 1
 * rate histogram type 1: 12 legacy plus 48 per BW (20/40/80)
 */
#define WL_HISTO_MAP1_ARR_LEN	(WL_NUM_LEGACY_RATES + (WL_NUM_VHT_RATES * WL_HISTO_MAP1_NUM_BW))

/* structure to store mapping of rates histogram for histo version 1, rate map type 1 */
typedef struct wl_rate_histo_map1 {
	uint16 seconds;			/* seconds of information */
	uint8 recent_index;		/* recently used rate index of this set */
	uint8 recent_type;		/* recently used rate type of this set */
	uint32 arr[WL_HISTO_MAP1_ARR_LEN]; /* count of mpdus per rate */
} wl_rate_histo_map1_t;

/* structure to store rate histogram for histo version 1, rate map type 1 */
typedef struct wl_rate_histo_maps1 {
	wl_rate_histo_map1_t rx, tx;
} wl_rate_histo_maps1_t;

/* rate histogram type 2 supports 160/80p80 MHz also */
#define WL_HISTO_MAP2_NUM_BW		(4) /* 20, 40, 80, 160/80p80 MHz */

/* rate histogram type 2: 12 legacy plus 48 per BW (20/40/80/160) */
#define WL_HISTO_MAP2_ARR_LEN	(WL_NUM_LEGACY_RATES + (WL_NUM_VHT_RATES * WL_HISTO_MAP2_NUM_BW))

/* structure to store mapping of rates histogram for histo version 1, rate map type 2 */
typedef struct wl_rate_histo_map2 {
	uint16 seconds;			/* seconds of information */
	uint8 recent_index;		/* recently used rate index of this set */
	uint8 recent_type;		/* recently used rate type of this set */
	uint32 arr[WL_HISTO_MAP2_ARR_LEN]; /* count of mpdus per rate */
} wl_rate_histo_map2_t;

/* structure to store rate histogram for histo version 1, rate map type 2 */
typedef struct wl_rate_histo_maps2 {
	wl_rate_histo_map2_t rx, tx;
} wl_rate_histo_maps2_t;

#define WL_MONITOR_CONFIG_NUM_AIDS	4
#define WL_MONITOR_CONFIG_FLAG_VHT_AID	0x01
#define WL_MONITOR_CONFIG_FLAG_HE_AID	0x02
#define WL_MONITOR_CONFIG_FLAG_GID	0x04
#define WL_MONITOR_CONFIG_FLAG_BC	0x08
#define WL_MONITOR_CONFIG_FLAG_BSSBMP	0x10
#define WL_MONITOR_CONFIG_FLAG_HETB	0x20
#define WL_MONITOR_CONFIG_FLAG_HETB_MACADDR	0x40
/* monitor_config param */
typedef struct wl_monitor_config_params {
	uint16	ver;		/* version */
	uint32	flags;		/* flags */
	uint32	params;		/* opaque params */
	uint8	vht_num_aids;	/* num of aids for vht */
	uint16	vht_aid[WL_MONITOR_CONFIG_NUM_AIDS];
	uint16	gid;		/* group id */
	uint8	he_num_aids;	/* num of aids for he */
	uint16	he_aid[WL_MONITOR_CONFIG_NUM_AIDS];
	uint8	he_bssidx_bmp;	/* he bss index bitmap */
	uint8	bsscolor;	/* bss color */
	bool    hetb_en;	/* enable hetb sniffing */
	struct ether_addr ea; /* align with ea when hetb_en is set */
	uint8	reserved[4];	/* For future use */
} wl_monitor_config_params_t;

/* For a specific AC, bit 0 is for VHT MU, bit is for 1 HE MU and bit 2 is for OFDMA MU */
#define MU_AC_MASK	0x7
#define MU_AC_MASK_SIZE 3

typedef struct wl_mutx_ac_mg {
	uint8 ac;       /* Access Class */
	uint8 mask;     /* bit 0 for VHT MU, bit 1 OFDMA MU and bit 2 HE-MU */
} wl_mutx_ac_mg_t;

#define WL_WRR_MAX_ENTRIES 15
typedef struct wl_wrr_params {
	/* Number of argument data entries */
	uint16 entries;
	/* Variable argument list for ucode Weighted Round Robin module.
	 * List to be finalizwd, specific structures put in at that time.
	 */
	uint16 data[WL_WRR_MAX_ENTRIES];
} wl_wrr_params_t;

/* Bit definitions for M_ENT_HOST_FLAGS1 */

/* Used to flag ucode to send sounding feedback frames for Transmit Beamforming */
#define EAP_ALLOW_SOUND_FB			    (1 << 3)
/* Used to flag ucode to allow all BSSID - mgmt frames */
#define EAP_ALLOW_MGMT_FRM_CAP			(1 << 4)

typedef struct wl_macreq {
	uint16 flag;
	uint16 type;
	uint16 rsvd;
	uint16 len;
	uint16 lidx_list[16];
} wl_macreq_params_t;

#define WL_MU_CLIENTS_PARAMS_VERSION	1
typedef struct max_clients {
	uint16 version;
	uint16 length;
	uint16 su;
	uint16 vhtmu;
	uint16 hemmu;
	uint16 dlofdma;
	uint16 ulofdma;
} max_clients_t;

typedef struct sr_config {
	char	cmdstr[30];
	char	argstr[30];
	int16	num_vals;
	int16	val;
} sr_config_t;

/* enum for different types Multi Users technologies */
typedef enum {
	WL_MU_NONE	= 0, /* No MU means SU */
	WL_MU_VHTMU	= 1, /* VHT MUMIMO */
	WL_MU_HEMMU	= 2, /* HE MUMIMO */
	WL_MU_DLOFDMA	= 3, /* HE Downlink OFDMA */
	WL_MU_TWT	= 4, /* TWT */
	WL_MU_ULOFDMA	= 5, /* Uplink OFDMA */
	WL_MU_MAX	= 6
} mu_type_t;

#define WL_MU_TX_TYPE_MAX  (WL_MU_TWT - WL_MU_VHTMU + 1)
#define WL_FIFO_CMD_KEYSTR_MAXLEN	20

typedef struct wl_fifo_msg {
	uint16	mutype;		/* MU TX type */
	uint16	value;
	char	keystr[WL_FIFO_CMD_KEYSTR_MAXLEN];
} wl_fifo_msg_t;

/** Dynamic Tx Power Control common interfaces */
#define WL_DTPC_IOV_VERSION   (1u)

typedef enum wl_dtpc_iov_v1 {
	WL_DTPC_CMD_EN =          0x0,  /**< top control switch */
	WL_DTPC_CMD_TXS_THRES =   0x1,  /**< txs PER threshold */
	WL_DTPC_CMD_PROBSTEP =    0x2,  /**< step size */
	WL_DTPC_CMD_ALPHA =       0x3,  /**< mv alpha */
	WL_DTPC_CMD_TRIGINT =     0x4,  /**< pwr prob trigger interval */
	WL_DTPC_CMD_HEADROOM =    0x5,  /**< power headroom */
	WL_DTPC_CMD_CLMOVR =      0x6,  /**< clm validation */
	WL_DTPC_CMD_TOTAL
} wl_dtpc_iov_v1_t;

/* Comment: do we ever use this structure? */
typedef struct wl_dtpc_cfg_v1 {
	uint16 version;              /**< version control */
	uint16 len;                  /**< total struct length */
	uint8 pmac[ETHER_ADDR_LEN];  /**< extention for per-scb query */
} wl_dtpc_cfg_v1_t;

#define MAX_NUM_KNOWN_RATES 640
typedef struct wl_dtpc_cfg_headroom {
	uint16	len;
	uint8   max_bw;
	uint8   max_nss;
	uint32	rspec[MAX_NUM_KNOWN_RATES];
	uint8	headroom[MAX_NUM_KNOWN_RATES];
} wl_dtpc_cfg_headroom_t;

#endif /* _wlioctl_h_ */
