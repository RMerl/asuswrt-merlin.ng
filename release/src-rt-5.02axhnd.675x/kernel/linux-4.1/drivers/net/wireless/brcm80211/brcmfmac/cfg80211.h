/*
 * Copyright (c) 2010 Broadcom Corporation
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
 */

#ifndef BRCMFMAC_CFG80211_H
#define BRCMFMAC_CFG80211_H

/* for brcmu_d11inf */
#include <brcmu_d11.h>

#define WL_NUM_SCAN_MAX			10
#define WL_NUM_PMKIDS_MAX		MAXPMKID
#define WL_TLV_INFO_MAX			1024
#define WL_BSS_INFO_MAX			2048
#define WL_ASSOC_INFO_MAX		512	/* assoc related fil max buf */
#define WL_EXTRA_BUF_MAX		2048
#define WL_ROAM_TRIGGER_LEVEL		-75
#define WL_ROAM_DELTA			20
#define WL_BEACON_TIMEOUT		3

#define WL_SCAN_CHANNEL_TIME		40
#define WL_SCAN_UNASSOC_TIME		40
#define WL_SCAN_PASSIVE_TIME		120

#define WL_ESCAN_BUF_SIZE		(1024 * 64)
#define WL_ESCAN_TIMER_INTERVAL_MS	10000 /* E-Scan timeout */

#define WL_ESCAN_ACTION_START		1
#define WL_ESCAN_ACTION_CONTINUE	2
#define WL_ESCAN_ACTION_ABORT		3

#define WL_AUTH_SHARED_KEY		1	/* d11 shared authentication */
#define IE_MAX_LEN			512

/* IE TLV processing */
#define TLV_LEN_OFF			1	/* length offset */
#define TLV_HDR_LEN			2	/* header length */
#define TLV_BODY_OFF			2	/* body offset */
#define TLV_OUI_LEN			3	/* oui id length */

/* 802.11 Mgmt Packet flags */
#define BRCMF_VNDR_IE_BEACON_FLAG	0x1
#define BRCMF_VNDR_IE_PRBRSP_FLAG	0x2
#define BRCMF_VNDR_IE_ASSOCRSP_FLAG	0x4
#define BRCMF_VNDR_IE_AUTHRSP_FLAG	0x8
#define BRCMF_VNDR_IE_PRBREQ_FLAG	0x10
#define BRCMF_VNDR_IE_ASSOCREQ_FLAG	0x20
/* vendor IE in IW advertisement protocol ID field */
#define BRCMF_VNDR_IE_IWAPID_FLAG	0x40
/* allow custom IE id */
#define BRCMF_VNDR_IE_CUSTOM_FLAG	0x100

/* P2P Action Frames flags (spec ordered) */
#define BRCMF_VNDR_IE_GONREQ_FLAG     0x001000
#define BRCMF_VNDR_IE_GONRSP_FLAG     0x002000
#define BRCMF_VNDR_IE_GONCFM_FLAG     0x004000
#define BRCMF_VNDR_IE_INVREQ_FLAG     0x008000
#define BRCMF_VNDR_IE_INVRSP_FLAG     0x010000
#define BRCMF_VNDR_IE_DISREQ_FLAG     0x020000
#define BRCMF_VNDR_IE_DISRSP_FLAG     0x040000
#define BRCMF_VNDR_IE_PRDREQ_FLAG     0x080000
#define BRCMF_VNDR_IE_PRDRSP_FLAG     0x100000

#define BRCMF_VNDR_IE_P2PAF_SHIFT	12

#define BRCMF_MAX_DEFAULT_KEYS		4


/**
 * enum brcmf_scan_status - scan engine status
 *
 * @BRCMF_SCAN_STATUS_BUSY: scanning in progress on dongle.
 * @BRCMF_SCAN_STATUS_ABORT: scan being aborted on dongle.
 * @BRCMF_SCAN_STATUS_SUPPRESS: scanning is suppressed in driver.
 */
enum brcmf_scan_status {
	BRCMF_SCAN_STATUS_BUSY,
	BRCMF_SCAN_STATUS_ABORT,
	BRCMF_SCAN_STATUS_SUPPRESS,
};

/* dongle configuration */
struct brcmf_cfg80211_conf {
	u32 frag_threshold;
	u32 rts_threshold;
	u32 retry_short;
	u32 retry_long;
	s32 tx_power;
	struct ieee80211_channel channel;
};

/* basic structure of scan request */
struct brcmf_cfg80211_scan_req {
	struct brcmf_ssid_le ssid_le;
};

/* basic structure of information element */
struct brcmf_cfg80211_ie {
	u16 offset;
	u8 buf[WL_TLV_INFO_MAX];
};

/* security information with currently associated ap */
struct brcmf_cfg80211_security {
	u32 wpa_versions;
	u32 auth_type;
	u32 cipher_pairwise;
	u32 cipher_group;
	u32 wpa_auth;
};

/**
 * struct brcmf_cfg80211_profile - profile information.
 *
 * @ssid: ssid of associated/associating ap.
 * @bssid: bssid of joined/joining ibss.
 * @sec: security information.
 * @key: key information
 */
struct brcmf_cfg80211_profile {
	struct brcmf_ssid ssid;
	u8 bssid[ETH_ALEN];
	struct brcmf_cfg80211_security sec;
	struct brcmf_wsec_key key[BRCMF_MAX_DEFAULT_KEYS];
};

/**
 * enum brcmf_vif_status - bit indices for vif status.
 *
 * @BRCMF_VIF_STATUS_READY: ready for operation.
 * @BRCMF_VIF_STATUS_CONNECTING: connect/join in progress.
 * @BRCMF_VIF_STATUS_CONNECTED: connected/joined succesfully.
 * @BRCMF_VIF_STATUS_DISCONNECTING: disconnect/disable in progress.
 * @BRCMF_VIF_STATUS_AP_CREATING: interface configured for AP operation.
 * @BRCMF_VIF_STATUS_AP_CREATED: AP operation started.
 */
enum brcmf_vif_status {
	BRCMF_VIF_STATUS_READY,
	BRCMF_VIF_STATUS_CONNECTING,
	BRCMF_VIF_STATUS_CONNECTED,
	BRCMF_VIF_STATUS_DISCONNECTING,
	BRCMF_VIF_STATUS_AP_CREATING,
	BRCMF_VIF_STATUS_AP_CREATED
};

/**
 * struct vif_saved_ie - holds saved IEs for a virtual interface.
 *
 * @probe_req_ie: IE info for probe request.
 * @probe_res_ie: IE info for probe response.
 * @beacon_ie: IE info for beacon frame.
 * @probe_req_ie_len: IE info length for probe request.
 * @probe_res_ie_len: IE info length for probe response.
 * @beacon_ie_len: IE info length for beacon frame.
 */
struct vif_saved_ie {
	u8  probe_req_ie[IE_MAX_LEN];
	u8  probe_res_ie[IE_MAX_LEN];
	u8  beacon_ie[IE_MAX_LEN];
	u8  assoc_req_ie[IE_MAX_LEN];
	u32 probe_req_ie_len;
	u32 probe_res_ie_len;
	u32 beacon_ie_len;
	u32 assoc_req_ie_len;
};

/**
 * struct brcmf_cfg80211_vif - virtual interface specific information.
 *
 * @ifp: lower layer interface pointer
 * @wdev: wireless device.
 * @profile: profile information.
 * @roam_off: roaming state.
 * @sme_state: SME state using enum brcmf_vif_status bits.
 * @pm_block: power-management blocked.
 * @list: linked list.
 * @mgmt_rx_reg: registered rx mgmt frame types.
 * @mbss: Multiple BSS type, set if not first AP (not relevant for P2P).
 */
struct brcmf_cfg80211_vif {
	struct brcmf_if *ifp;
	struct wireless_dev wdev;
	struct brcmf_cfg80211_profile profile;
	s32 roam_off;
	unsigned long sme_state;
	bool pm_block;
	struct vif_saved_ie saved_ie;
	struct list_head list;
	u16 mgmt_rx_reg;
	bool mbss;
	int is_11d;
};

/* association inform */
struct brcmf_cfg80211_connect_info {
	u8 *req_ie;
	s32 req_ie_len;
	u8 *resp_ie;
	s32 resp_ie_len;
};

/* assoc ie length */
struct brcmf_cfg80211_assoc_ielen_le {
	__le32 req_len;
	__le32 resp_len;
};

/* wpa2 pmk list */
struct brcmf_cfg80211_pmk_list {
	struct pmkid_list pmkids;
	struct pmkid foo[MAXPMKID - 1];
};

/* dongle escan state */
enum wl_escan_state {
	WL_ESCAN_STATE_IDLE,
	WL_ESCAN_STATE_SCANNING
};

struct escan_info {
	u32 escan_state;
	u8 escan_buf[WL_ESCAN_BUF_SIZE];
	struct wiphy *wiphy;
	struct brcmf_if *ifp;
	s32 (*run)(struct brcmf_cfg80211_info *cfg, struct brcmf_if *ifp,
		   struct cfg80211_scan_request *request, u16 action);
};

/**
 * struct brcmf_pno_param_le - PNO scan configuration parameters
 *
 * @version: PNO parameters version.
 * @scan_freq: scan frequency.
 * @lost_network_timeout: #sec. to declare discovered network as lost.
 * @flags: Bit field to control features of PFN such as sort criteria auto
 *	enable switch and background scan.
 * @rssi_margin: Margin to avoid jitter for choosing a PFN based on RSSI sort
 *	criteria.
 * @bestn: number of best networks in each scan.
 * @mscan: number of scans recorded.
 * @repeat: minimum number of scan intervals before scan frequency changes
 *	in adaptive scan.
 * @exp: exponent of 2 for maximum scan interval.
 * @slow_freq: slow scan period.
 */
struct brcmf_pno_param_le {
	__le32 version;
	__le32 scan_freq;
	__le32 lost_network_timeout;
	__le16 flags;
	__le16 rssi_margin;
	u8 bestn;
	u8 mscan;
	u8 repeat;
	u8 exp;
	__le32 slow_freq;
};

/**
 * struct brcmf_pno_net_param_le - scan parameters per preferred network.
 *
 * @ssid: ssid name and its length.
 * @flags: bit2: hidden.
 * @infra: BSS vs IBSS.
 * @auth: Open vs Closed.
 * @wpa_auth: WPA type.
 * @wsec: wsec value.
 */
struct brcmf_pno_net_param_le {
	struct brcmf_ssid_le ssid;
	__le32 flags;
	__le32 infra;
	__le32 auth;
	__le32 wpa_auth;
	__le32 wsec;
};

/**
 * struct brcmf_pno_net_info_le - information per found network.
 *
 * @bssid: BSS network identifier.
 * @channel: channel number only.
 * @SSID_len: length of ssid.
 * @SSID: ssid characters.
 * @RSSI: receive signal strength (in dBm).
 * @timestamp: age in seconds.
 */
struct brcmf_pno_net_info_le {
	u8 bssid[ETH_ALEN];
	u8 channel;
	u8 SSID_len;
	u8 SSID[32];
	__le16	RSSI;
	__le16	timestamp;
};

/**
 * struct brcmf_pno_scanresults_le - result returned in PNO NET FOUND event.
 *
 * @version: PNO version identifier.
 * @status: indicates completion status of PNO scan.
 * @count: amount of brcmf_pno_net_info_le entries appended.
 */
struct brcmf_pno_scanresults_le {
	__le32 version;
	__le32 status;
	__le32 count;
};

/**
 * struct brcmf_cfg80211_vif_event - virtual interface event information.
 *
 * @vif_wq: waitqueue awaiting interface event from firmware.
 * @vif_event_lock: protects other members in this structure.
 * @vif_complete: completion for net attach.
 * @action: either add, change, or delete.
 * @vif: virtual interface object related to the event.
 */
struct brcmf_cfg80211_vif_event {
	wait_queue_head_t vif_wq;
	struct mutex vif_event_lock;
	u8 action;
	struct brcmf_cfg80211_vif *vif;
};

/**
 * struct brcmf_cfg80211_info - dongle private data of cfg80211 interface
 *
 * @wiphy: wiphy object for cfg80211 interface.
 * @conf: dongle configuration.
 * @p2p: peer-to-peer specific information.
 * @btcoex: Bluetooth coexistence information.
 * @scan_request: cfg80211 scan request object.
 * @usr_sync: mainly for dongle up/down synchronization.
 * @bss_list: bss_list holding scanned ap information.
 * @scan_req_int: internal scan request object.
 * @bss_info: bss information for cfg80211 layer.
 * @ie: information element object for internal purpose.
 * @conn_info: association info.
 * @pmk_list: wpa2 pmk list.
 * @scan_status: scan activity on the dongle.
 * @pub: common driver information.
 * @channel: current channel.
 * @active_scan: current scan mode.
 * @sched_escan: e-scan for scheduled scan support running.
 * @ibss_starter: indicates this sta is ibss starter.
 * @pwr_save: indicate whether dongle to support power save mode.
 * @dongle_up: indicate whether dongle up or not.
 * @roam_on: on/off switch for dongle self-roaming.
 * @scan_tried: indicates if first scan attempted.
 * @dcmd_buf: dcmd buffer.
 * @extra_buf: mainly to grab assoc information.
 * @debugfsdir: debugfs folder for this device.
 * @escan_info: escan information.
 * @escan_timeout: Timer for catch scan timeout.
 * @escan_timeout_work: scan timeout worker.
 * @escan_ioctl_buf: dongle command buffer for escan commands.
 * @vif_list: linked list of vif instances.
 * @vif_cnt: number of vif instances.
 * @vif_event: vif event signalling.
 * @wowl_enabled; set during suspend, is wowl used.
 * @pre_wowl_pmmode: intermediate storage of pm mode during wowl.
 */
struct brcmf_cfg80211_info {
	struct wiphy *wiphy;
	struct brcmf_cfg80211_conf *conf;
	struct brcmf_p2p_info p2p;
	struct brcmf_btcoex_info *btcoex;
	struct cfg80211_scan_request *scan_request;
	struct mutex usr_sync;
	struct brcmf_cfg80211_scan_req scan_req_int;
	struct wl_cfg80211_bss_info *bss_info;
	struct brcmf_cfg80211_ie ie;
	struct brcmf_cfg80211_connect_info conn_info;
	struct brcmf_cfg80211_pmk_list *pmk_list;
	unsigned long scan_status;
	struct brcmf_pub *pub;
	u32 channel;
	bool active_scan;
	bool sched_escan;
	bool ibss_starter;
	bool pwr_save;
	bool dongle_up;
	bool scan_tried;
	u8 *dcmd_buf;
	u8 *extra_buf;
	struct dentry *debugfsdir;
	struct escan_info escan_info;
	struct timer_list escan_timeout;
	struct work_struct escan_timeout_work;
	u8 *escan_ioctl_buf;
	struct list_head vif_list;
	struct brcmf_cfg80211_vif_event vif_event;
	struct completion vif_disabled;
	struct brcmu_d11inf d11inf;
	bool wowl_enabled;
	u32 pre_wowl_pmmode;
};

/**
 * struct brcmf_tlv - tag_ID/length/value_buffer tuple.
 *
 * @id: tag identifier.
 * @len: number of bytes in value buffer.
 * @data: value buffer.
 */
struct brcmf_tlv {
	u8 id;
	u8 len;
	u8 data[1];
};

static inline struct wiphy *cfg_to_wiphy(struct brcmf_cfg80211_info *cfg)
{
	return cfg->wiphy;
}

static inline struct brcmf_cfg80211_info *wiphy_to_cfg(struct wiphy *w)
{
	return (struct brcmf_cfg80211_info *)(wiphy_priv(w));
}

static inline struct brcmf_cfg80211_info *wdev_to_cfg(struct wireless_dev *wd)
{
	return (struct brcmf_cfg80211_info *)(wdev_priv(wd));
}

static inline
struct net_device *cfg_to_ndev(struct brcmf_cfg80211_info *cfg)
{
	struct brcmf_cfg80211_vif *vif;
	vif = list_first_entry(&cfg->vif_list, struct brcmf_cfg80211_vif, list);
	return vif->wdev.netdev;
}

static inline struct brcmf_cfg80211_info *ndev_to_cfg(struct net_device *ndev)
{
	return wdev_to_cfg(ndev->ieee80211_ptr);
}

static inline struct brcmf_cfg80211_profile *ndev_to_prof(struct net_device *nd)
{
	struct brcmf_if *ifp = netdev_priv(nd);
	return &ifp->vif->profile;
}

static inline struct brcmf_cfg80211_vif *ndev_to_vif(struct net_device *ndev)
{
	struct brcmf_if *ifp = netdev_priv(ndev);
	return ifp->vif;
}

static inline struct
brcmf_cfg80211_connect_info *cfg_to_conn(struct brcmf_cfg80211_info *cfg)
{
	return &cfg->conn_info;
}

struct brcmf_cfg80211_info *brcmf_cfg80211_attach(struct brcmf_pub *drvr,
						  struct device *busdev);
void brcmf_cfg80211_detach(struct brcmf_cfg80211_info *cfg);
s32 brcmf_cfg80211_up(struct net_device *ndev);
s32 brcmf_cfg80211_down(struct net_device *ndev);
enum nl80211_iftype brcmf_cfg80211_get_iftype(struct brcmf_if *ifp);

struct brcmf_cfg80211_vif *brcmf_alloc_vif(struct brcmf_cfg80211_info *cfg,
					   enum nl80211_iftype type,
					   bool pm_block);
void brcmf_free_vif(struct brcmf_cfg80211_vif *vif);

s32 brcmf_vif_set_mgmt_ie(struct brcmf_cfg80211_vif *vif, s32 pktflag,
			  const u8 *vndr_ie_buf, u32 vndr_ie_len);
s32 brcmf_vif_clear_mgmt_ies(struct brcmf_cfg80211_vif *vif);
const struct brcmf_tlv *
brcmf_parse_tlvs(const void *buf, int buflen, uint key);
u16 channel_to_chanspec(struct brcmu_d11inf *d11inf,
			struct ieee80211_channel *ch);
bool brcmf_get_vif_state_any(struct brcmf_cfg80211_info *cfg,
			     unsigned long state);
void brcmf_cfg80211_arm_vif_event(struct brcmf_cfg80211_info *cfg,
				  struct brcmf_cfg80211_vif *vif);
bool brcmf_cfg80211_vif_event_armed(struct brcmf_cfg80211_info *cfg);
int brcmf_cfg80211_wait_vif_event_timeout(struct brcmf_cfg80211_info *cfg,
					  u8 action, ulong timeout);
s32 brcmf_notify_escan_complete(struct brcmf_cfg80211_info *cfg,
				struct brcmf_if *ifp, bool aborted,
				bool fw_abort);
void brcmf_set_mpc(struct brcmf_if *ndev, int mpc);
void brcmf_abort_scanning(struct brcmf_cfg80211_info *cfg);
void brcmf_cfg80211_free_netdev(struct net_device *ndev);

#endif /* BRCMFMAC_CFG80211_H */
