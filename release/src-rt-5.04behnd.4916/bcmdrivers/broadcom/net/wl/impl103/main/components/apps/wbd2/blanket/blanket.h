/** @file blanket.h
 *  @brief Function prototypes for Wi-Fi Blanket API 2.
 *
 * Broadcom Blanket library include file
 *
 * Copyright (C) 2024, Broadcom. All Rights Reserved.
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
 * $Id: blanket.h 837830 2024-03-15 10:58:04Z $
 *
 *  @author
 *  @bug No known bugs.
 */

#ifndef __BLANKET_H__
#define __BLANKET_H__

#include "ieee1905.h"

#define BKT_MAX_PROCESS_NAME	30	/* Maximum length of process NAME */

/* Global variable to get process name */
extern char g_wbd_process_name[BKT_MAX_PROCESS_NAME];

/* Check if chanspec belongs to the operating class with center channel */
#define BLANKET_CENTER_CH_OPCLASS_CHSPEC(c)	((CHSPEC_IS6G(c) && CHSPEC_IS40(c)) || \
							CHSPEC_IS80(c) || CHSPEC_IS160(c))

/* Blanket Debug Message Level */
#define BKT_DEBUG_NONE		0x0000
#define BKT_DEBUG_ERROR		0x0001
#define BKT_DEBUG_WARNING	0x0002
#define BKT_DEBUG_INFO		0x0004
#define BKT_DEBUG_DETAIL	0x0008
#define BKT_DEBUG_TRACE		0X0010
#define BKT_DEBUG_DEFAULT	BKT_DEBUG_ERROR

/* Information to be passed to initialize the blanket module */
typedef struct blanket_module_info {
	unsigned int msglevel;	/* Message level of type BKT_DEBUG_XXX */
	uint16 flags;		/* FLAGS of type BKT_INFO_FLAGS_XXX */
} blanket_module_info_t;

/* Blanket Neighbor info */
typedef struct blanket_nbr_info {
	struct ether_addr bssid;
	uint32 bssid_info;
	uint8 reg;
	uint8 channel;
	uint8 phytype;
	uint8 bss_trans_preference;
	wlc_ssid_t ssid;
	chanspec_t chanspec;
} blanket_nbr_info_t;

typedef struct blanket_counters {
	uint32 txframe;	/* tx data frames */
	uint32 txbyte;	/* tx data bytes */
	uint32 txerror;	/* tx data errors (derived: sum of others) */
	uint32 rxframe;	/* rx data frames */
	uint32 rxbyte;	/* rx data bytes */
	uint32 rxerror;	/* rx data errors (derived: sum of others) */
	uint32 txmcastframe; /* tx multicast frames */
	uint32 rxmcastframe; /* rx multicast frames */
	uint32 txbcastframe; /* tx broadcast frames */
	uint32 rxbcastframe; /* rx broadcast frames */
} blanket_counters_t;

typedef struct blanket_bcnreq {
	bcnreq_t bcnreq;
	struct ether_addr src_bssid;
	struct ether_addr target_bssid;
	uint8 opclass;
} blanket_bcnreq_t;

/* fields to parse radio capability per ragulatoy class */
typedef struct radio_cap_sub_info {
	uint8 regclass;
	uint8 tx_pwr;
	uint8 n_chan;
	uint8 list_chan[0];
} __attribute__ ((__packed__)) radio_cap_sub_info_t;

/* String or space separated list compare options */
typedef enum bkt_cmp_option {
	BKT_CMP_STR_WITHOUT_CASE	= 0,
	BKT_CMP_STR_WITH_CASE		= 1,
	BKT_CMP_LIST_WITH_CASE		= 2
} bkt_cmp_option_t;

/** @brief Initialize blanket module
 *
 * @param info		info structure for inializing the module
 *
 * @return		status of the call. 0 Success. Non Zero Failure
 */
int blanket_module_init(blanket_module_info_t *info);

/** @brief Set message level for printing debug messages
 *
 * @param msglevel	level to be set
 *
 * @return		None
 */
void blanket_set_msglevel(unsigned int msglevel);

/** @brief Check if interface is valid BRCM radio or not, and get ioctl version
 *
 * @param ifname	interface name of the BSS
 *
 * @return		wl ioctl version
 */
int blanket_probe(char *ifname);

/** @brief Blanket Node get the current MAC address of Interface on this node
 *
 * @param ifname	interface name of the BSS
 * @param out_mac	Out MAC address of Interface on this node
 *
 * @return		status of the call. 0 Success. Non Zero Failure
 */
int blanket_get_bss_mac(char *ifname, struct ether_addr *out_mac);

/** @brief Blanket Node get the current MAC address of the Radio (Primary Interface)
 *
 * @param primary_prefix	Prefix of the Primary Interface
 * @param out_mac	Out MAC address of the Radio (Primary Interface)
 *
 * @return		status of the call. 0 Success. Non Zero Failure
 */
int blanket_get_radio_mac(char* primary_prefix, struct ether_addr *out_mac);

/** @brief De-auth the sta with reason code
 *
 * @param ifname	interface name of the BSS
 * @param addr		MAC address of sta
 * @param reason	reason for de-auth
 *
 * @return		status of the call. 0 Success. Non Zero Failure
 */
int blanket_deauth_sta(char* ifname, struct ether_addr* addr, int reason);

/** @brief Send Disassoc to the sta
 *
 * @param ifname	interface name of the BSS
 * @param addr		MAC address of sta
 *
 * @return		status of the call. 0 Success. Non Zero Failure
 */
int blanket_disassoc_sta(char* ifname, struct ether_addr* addr);

/** @brief To get the BSS information of Interface
 *
 * @param ifname	interface name of the BSS
 * @param out_bss_info	Out object of type wl_bss_info_t, filled in bss_info object
 *
 * @return		status of the call. 0 Success. Non Zero Failure
 */
int blanket_get_bss_info(char *ifname, wl_bss_info_t **out_bss_info);

/** @brief To get the Beamformer of Interface
 *
 * @param ifname	interface name
 * @param out_bfr_cap	Out object, bfr_cap  enabled/disabled
 *
 * @return		status of the call. 0 Success. Non Zero Failure
 */
int blanket_get_bfr_cap(char *ifname, int *out_bfr_cap);

/** @brief To get the MU Features of Radio
 *
 * @param ifname	interface name
 * @param out_mu_features	Out object, out_mu_features enabled/disabled
 *
 * @return		status of the call. 0 Success. Non Zero Failure
 */
int blanket_get_mu_features(char *ifname, int *out_mu_features);

/** @brief To get the Beamformee of Interface
 *
 * @param ifname	interface name
 * @param out_bfr_cap	Out object, bfe_cap  enabled/disabled
 *
 * @return		status of the call. 0 Success. Non Zero Failure
 */
int blanket_get_bfe_cap(char *ifname, int *out_bfe_cap);

/** @brief To get the OFDMA of the Radio Interface
 *
 * @param ifname	interface name
 * @param mu		Out object of type max_clients_t, filled in max_client
 *
 * @return		status of the call. 0 Success. Non Zero Failure
 */
int blanket_get_ofdma(char *ifname, max_clients_t *mu);

/** @brief To get the MU-BSSID of the Radio Interface
 *
 * @param ifname	interface name
 * @param cfg		Out object of type wl_mbssid_t, filled in cfg object
 *
 * @return		status of the call. 0 Success. Non Zero Failure
 */
int blanket_get_mbssid(char *ifname, wl_mbssid_t *cfg);

/** @brief To get the TWT_REQ Enable/disable of the Radio Interface
 *
 * @param ifname	interface name
 * @param twt_req	Out object, twt_req  enabled/disabled
 *
 * @return		status of the call. 0 Success. Non Zero Failure
 */
int blanket_get_twt_req_cap(char *ifname, int *twt_req);

/** @brief To get the TWT_RESP Enable/disable of the Radio Interface
 *
 * @param ifname	interface name
 * @param twt_resp	Out object, twt_resp enabled/disabled
 *
 * @return		status of the call. 0 Success. Non Zero Failure
 */
int blanket_get_twt_resp_cap(char *ifname, int *twt_resp);

/** @brief To get the RTS Enable/disable of the Radio Interface
 *
 * @param ifname	interface name
 * @param twt_resp	Out object, RTS enabled/disabled
 *
 * @return		status of the call. 0 Success. Non Zero Failure
 */
int blanket_get_rts_cap(char *ifname, int *rts);

/** @brief To get the he features Enable/disable of the Radio Interface
 *
 * @param ifname	interface name
 * @param he_features	 object, he_features  enabled/disabled
 * @param he_cmd	 he_cmd
 *
 * @return		status of the call. 0 Success. Non Zero Failure
 */
int blanket_get_cmd_he_features(char *ifname, int *he_features, uint8 he_cmd);

/** @brief Get chanspec of interface
 *
 * @param ifname	interface name of the BSS
 * @param chanspec	chanspec to set for this radio
 *
 * @return		status of the call. 0 Success. Non Zero Failure
 */
int blanket_set_chanspec(char *ifname, chanspec_t chanspec);

/** @brief Set chanspec of interface
 *
 * @param ifname	interface name of the BSS
 * @param out_chanspec	Out object of type chanspec_t, filled in chanspec for this radio
 *
 * @return		status of the call. 0 Success. Non Zero Failure
 */
int blanket_get_chanspec(char *ifname, chanspec_t* out_chanspec);

/** @brief Get list of chanspecs suppported by driver
 *
 * @param ifname	interface name of the BSS
 * @param list		Out object, list of all of the available chanspecs in a variable length
 * @param listlen	length of the memory allocated for list
 *
 * @return		status of the call. 0 Success. Non Zero Failure
 */
int blanket_get_chanspecs_list(char* ifname, wl_uint32_list_t* list, int listlen);

/** @brief Get Interface specific chan_info
 *
 * @param ifname	interface name of the BSS
 * @param channel	Current channel for which chan_info is needed
 * @param band		Band of channel for which chan_info is needed
 * @param out_bitmap	Out object, a combination of a bitflags for channel attributes and
 *                      a bitfield for a channel out-of-service duration
 *
 * @return		status of the call. 0 Success. Non Zero Failure
 */
int blanket_get_chan_info(char* ifname, uint channel, uint band, uint *out_bitmap);

/** @brief Get STA (client) Information
 *
 * @param ifname	interface name of the BSS
 * @param addr		MAC address of sta
 * @param out_sta_info	Out object of type sta_info_t, filled in station information for this MAC.
 *
 * @return		status of the call. 0 Success. Non Zero Failure
 */
int blanket_get_sta_info(char *ifname, struct ether_addr *addr, sta_info_t *out_sta_info);

/** @brief Blanket Node get the current band of Interface on this node
 *
 * @param ifname	interface name of the BSS
 * @param out_band	out: Operating Band of Interface on this node
 *
 * @return		status of the call. 0 Success. Non Zero Failure
 */
int blanket_get_band(char *ifname, int *out_band);

/** @brief Blanket Node get the BSSID of Interface on this node
 *
 * @param ifname	interface name of the BSS
 * @param out_mac	out: BSSID of Interface on this node
 *
 * @return		status of the call. 0 Success. Non Zero Failure
 */
int blanket_get_bssid(char *ifname, struct ether_addr *out_bssid);

/** @brief Blanket Node try getting the BSSID for max_try times for every useconds_gap
 *
 * @param ifname	interface name of the BSS
 * @param ncount	Maximum number of times to repeat getting the BSSID
 * @param useconds_gap	Milliseconds gap between every driver call
 * @param out_mac	out: BSSID of Interface on this node
 *
 * @return		status of the call. 0 Success. Non Zero Failure
 */
int blanket_try_get_bssid(char *ifname, int max_try, int useconds_gap,
	struct ether_addr *out_bssid);

/** @brief Blanket Node get SSID of this STA Interface
 *
 * @param ifname	interface name of the STA
 * @param out_ssid	SSID to which the sta interface is associated
 *
 * @return		status of the call. 0 Success. Non Zero Failure
 */
int blanket_get_ssid(char* ifname, wlc_ssid_t *out_ssid);

/** @brief Blanket Node join SSID with assoc parameters provided
 *
 * @param ifname	interface name of the STA
 * @param join_params	assoc parameters for join
 * @param join_params_len length of assoc parameters
 *
 * @return		status of the call. 0 Success. Non Zero Failure
 */
int blanket_join_ssid(char* ifname, wl_join_params_t *join_params, uint join_params_size);

/** @brief Blanket Node get the current RSSI of sta associated to this Interface on this node
 *
 * @param ifname	interface name of the BSS
 * @param addr		MAC of the asociated sta
 * @param out_rssi	Out: Current RSSI of the sta associated to this Interface on this node
 *
 * @return		status of the call. 0 Success. Non Zero Failure
 */
int blanket_get_rssi(char *ifname, struct ether_addr *addr, int *out_rssi);

/** @brief Get Regulatory class of Interface for particular chanspec
 *
 * @param ifname	interface name of the BSS
 * @param chspec	current chanspec of BSS
 * @param out_rclass	out: regulatory class
 *
 * @return		status of the call. 0 Success. Non Zero Failure
 */
int blanket_get_rclass(char *ifname, chanspec_t chspec, uint8 *out_rclass);

/** @brief Get Global Regulatory class of particular chanspec
 *
 * @param chspec	current chanspec of BSS
 * @param out_rclass	out: regulatory class
 *
 * @return		status of the call. 0 Success. Non Zero Failure
 */
int blanket_get_global_rclass(chanspec_t chanspec, uint8 *out_rclass);

/** @brief Convert global operating class to band
 *
 * @param opclass	global operating class (Table E-4 of Spec)
 *
 * @return		band corresponding to the operating class
 */
int blanket_opclass_to_band(uint opclass);

/** @brief Prepare chanspec from global operating class, channel, bandwidth and band
 *
 * @param channel	Primary or center channel
 * @param opclass	Global operating class
 * @param bw		Bandwidth, if 0 find it from opclass
 * @param band		Band of the radio, if 0 find it from opclass
 *
 * @return		chanspec
 */
chanspec_t blanket_prepare_chanspec(uint channel, uint opclass, uint bw, int band);

/** @brief Mask out the side band details from the givien chanspec
 *
 * @param chspec	chanspec
 *
 * @return		chanspec after side band masked out
 */
chanspec_t blanket_mask_chanspec_sb(chanspec_t chspec);

/** @brief Extract rc_map pointer for given Operating Class
 *
 * @param out_rclass	in: current regulatory class
 *
 * @return		rc_map pointer for given regclasss. NULL for reclass not found.
 */
uint16* blanket_get_chspeclist_fm_rc_map(uint8 rc);

/** @brief Add a Chanspec to Chanspec list
 *
 * @param list	in: Chanspec list of type i5_chspec_item_t items
 * @param chspec		in: Chanspec to add in list
 *
 * @return		status of the call. 0 Success. Non Zero Failure
 */
int blanket_add_chspec_to_list(ieee1905_glist_t *list, chanspec_t chspec);

/** @brief Extract Chanspec from rc_map for given Operating Class & Channel
 *
 * @param rc	in: current regulatory class
 * @param channel		in: current channel
 * @param list_excl	in: i5_chspec_item_t items to be added this list from rc_map
 *
 * @return		Number of Chanspecs added from rc_map
 */
int blanket_add_chspecs_fm_rc_map(unsigned char rc, unsigned char channel,
	ieee1905_glist_t *list_excl);

/** @brief Get rate of Interface
 *
 * @param ifname	interface name of the BSS
 * @param out_rate	out: rate in units of 1000 Kbits/s
 *
 * @return		status of the call. 0 Success. Non Zero Failure
 */
int blanket_get_rate(char *ifname, int *out_rate);

/** @brief Add static neighbor entry
 *
 * @param ifname	interface name of the BSS
 * @param nbr		nbr element to be added in Static list
 *
 * @return		status of the call. 0 Success. Non Zero Failure
 */
int blanket_add_nbr(char *ifname, blanket_nbr_info_t *nbr);

/** @brief Delete static neighbor entry
 *
 * @param ifname	interface name of the BSS
 * @param nbr		MAC of the neighbor to be deleted
 *
 * @return		status of the call. 0 Success. Non Zero Failure
 */
int blanket_del_nbr(char *ifname, struct ether_addr *nbr);

/** @brief To Check BSS Transition Supported or not
 *
 * @param ifname	interface name of the BSS
 * @param sta_mac	MAC of the Associated STA
 *
 * @return		status of the call. 1 Supports BSS Transition 0 Doesn't Support
 */

int blanket_is_bss_trans_supported(char *ifname, struct ether_addr *sta_mac);

/** @brief To Check Global Rclass Supported or not
 *
 * @param ifname	interface name of the BSS
 * @param sta_mac	MAC of the Associated STA
 *
 * @return		status of the call. 1 Supports Global Rclass 0 Doesn't Support
 */

int blanket_is_global_rclass_supported(char *ifname, struct ether_addr *sta_mac);

/** @brief Get Banwidth from regulatory class
 *
 * @param rc	Regulatory class
 * @param bw	Bandwidth
 *
 * @return	status of the call. 0 Success. Non Zero Failure
 */

int blanket_get_bw_from_rc(uint8 rc, uint *bw);

/** @brief Get HT Capabilities from bss_info
 *
 * @param ifname	interface name of the BSS
 * @param in_bi		In object of type wl_bss_info_t,
 *			bss_info object if already extracted (optional, can be NULL)
 * @param out_ht_caps	Out HT capability of interface
 *
 * @return		status of the call. 0 Success. Non Zero Failure
 */
int blanket_get_ht_cap(char *ifname, wl_bss_info_t *in_bi, unsigned char *out_ht_caps);

/** @brief Get VHT MCS Capabilities from bss_info
 *
 * @param ifname	interface name of the BSS
 * @param in_bi		In object of type wl_bss_info_t,
 *			bss_info object if already extracted (optional, can be NULL)
 * @param out_vht_caps	Out VHT capability of interface
 *
 * @return		status of the call. 0 Success. Non Zero Failure
 */
int blanket_get_vht_cap(char *ifname, wl_bss_info_t *in_bi, ieee1905_vht_caps_type *out_vht_caps);

/** @brief Get Basic Capabilities
 *
 * @param out_basic_caps	Out basic capability of interface
 *
 * @return		status of the call. 0 Success. Non Zero Failure
 */
int blanket_get_basic_cap(uint8 *out_basic_caps);

/** @brief Get HE MCS Capabilities from bss_info
 *
 * @param ifname	interface name of the BSS
 * @param in_bi		In object of type wl_bss_info_t,
 *			bss_info object if already extracted (optional, can be NULL)
 * @param out_he_caps	Out HE capability of interface
 *
 * @return		status of the call. 0 Success. Non Zero Failure
 */
int blanket_get_he_cap(char *ifname, wl_bss_info_t *in_bi, ieee1905_he_caps_type *out_he_caps);

/** @brief Get EHT MCS Capabilities from bss_info
 *
 * @param ifname	interface name of the BSS
 * @param in_bi		In object of type wl_bss_info_t,
 *			bss_info object if already extracted (optional, can be NULL)
 * @param out_eht_caps	Out EHT capability of interface
 *
 * @return		status of the call. 0 Success. Non Zero Failure
 */
int blanket_get_eht_cap(char *ifname, wl_bss_info_t *in_bi, ieee1905_eht_caps_type *out_eht_caps);

/** @brief To get the Maximum Tx Power of Interface
 *
 * @param ifname	interface name of the BSS
 * @param tx	Out Maximum Tx Power of Interface
 *
 * @return		status of the call. 0 Success. Non Zero Failure
 */
int blanket_get_txpwr_target_max(char *ifname, int *tx);

/** @brief Get the list of Radio Capabilities for this interface
 *
 * Get the list of chanpecs supported by this radio
 * Get the list of supported operating classes by this radio
 * Get the list of all chanspecs present in those operating classes
 * See if there is any difference between the chanspecs lists 1 & 3
 *    and report those as unsupported chanspecs.
 *
 * @param ifname	interface name of the BSS
 * @param out_radio_caps	Out list of radio capability of interface
 *
 * @return		status of the call. 0 Success. Non Zero Failure
 */
int blanket_get_radio_cap(char *ifname, ieee1905_radio_caps_type *out_radio_caps);

/** @brief Channel validity check aganist provided preference
 *
 * @param rc_map	channel preference map
 * @param rc_count	number of operating class of rc_map
 * @param rclass	operating class of channel
 * @param channel	channel to validate
 * @param min_pref	required minimum preference
 *
 * @return		TRUE if the channel is valid, otherwise FALSE
 */
int blanket_check_channel_validity(ieee1905_chan_pref_rc_map *rc_map, unsigned char rc_count,
	uint8 rclass, uint8 channel, uint8 min_pref);

/** @brief Control channel validity check aganist provided preference
 *
 * @param band		band of ctrl_channel
 * @param ctrl_channel	control channel to validate
 * @param rc_map	channel preference map
 * @param rc_count	number of operating class of rc_map
 *
 * @return		TRUE if the channel is valid, otherwise FALSE
 */
int blanket_check_control_channel_validity(uint8 band, uint8 ctrl_channel,
	ieee1905_chan_pref_rc_map *rc_map, unsigned char rc_count);

/** @brief Check whether the rclass is 20MHz or not
 *
 * @param rclass	regulatory class to be checked
 *
 * @return		TRUE if the rclass is 20MHz, otherwise FALSE
 */
int blanket_is_20MHz_opclass(uint8 rclass);

/** @brief Update STATIC Non-operable Chanspec List of an interface
 *
 * @param band	WBD_BAND Type
 * @param in_radio_caps	In : Radio Capability List of interface
 * @param list_excl	Out STATIC Non-operable Chanspec List of an interface
 *
 * @return		status of the call. 0 Success. Non Zero Failure
 */
int blanket_update_static_nonoperable_chanspec_list(uint8 band,
	ieee1905_radio_caps_type *in_radio_caps, ieee1905_glist_t *list_excl);

/** @brief Update DYNAMIC Non-operable Chanspec List of an interface
 *
 * @param band	WBD_BAND Type
 * @param ifname	interface name of the BSS
 * @param in_chan_prefs	In : Channel Preference List of interface
 * @param list_excl	Out DYNAMIC Non-operable Chanspec List of an interface
 *
 * @return		status of the call. 0 Success. Non Zero Failure
 */
int blanket_update_dynamic_nonoperable_chanspec_list(uint8 band, char *ifname,
	ieee1905_chan_pref_rc_map_array *in_chan_prefs, ieee1905_glist_t *list_excl);

/** @brief Get the list of Channel Scan Capabilities for this interface
 *
 * Get the list of chanpecs supported by this radio based on bandwidth cap.
 * Get the list of supported operating classes by this radio.
 * Get the list of all chanspecs present in those operating classes.
 * Take chanspecs which are Supported.
 *
 * @param ifname	interface name of the BSS
 * @param out_radio_caps	Out list of radio capability of interface
 *
 * @return		status of the call. 0 Success. Non Zero Failure
 */
int blanket_get_channel_scan_cap(char *ifname, ieee1905_channel_scan_caps_type *out_chscan_caps);

/** @brief To get the phy_type of the BSS
 *
 * @param ifname	interface name of the BSS
 * @param in_bi		In object of type wl_bss_info_t,
 *			bss_info object if already extracted (optional, can be NULL)
 * @param out_phy_type	Out phy_type of the BSS
 *
 * @return		status of the call. 0 Success. Non Zero Failure
 */
int blanket_get_phy_type(char *ifname, wl_bss_info_t *in_bi, uint8 *out_phy_type);

/** @brief To get the FBT Enabled value in IOVAR
 *
 * @param ifname	interface name of the BSS
 *
 * @return	1 if FBT is Enabled on BSS, 0 if FBT is Disbled on BSS
 */
int blanket_get_fbt(char *ifname);

/** @brief To get the bssid_info of the BSS
 *
 * Get the bssid information field which can be passed in
 * neighbor report element for 11k and 11v.
 *
 * @param ifname	interface name of the BSS
 * @param in_bi		In object of type wl_bss_info_t,
 *			bss_info object if already extracted (optional, can be NULL)
 * @param out_bssid_info	Out bssid_info of the BSS
 *
 * @return			status of the call. 0 Success. Non Zero Failure
 */
int blanket_get_bssid_info_field(char *ifname, wl_bss_info_t *in_bi, uint32 *out_bssid_info);

/** @brief Get max NSS value of the Interface (radio)
 *
 * @param ifname	interface name of the BSS
 * @param in_bi		In object of type wl_bss_info_t,
 *			bss_info object if already extracted (optional, can be NULL)
 * @param max_nss	Out object, max NSS for this radio
 *
 * @return		status of the call. 0 Success. Non Zero Failure
 */
int blanket_get_max_nss(char* ifname, wl_bss_info_t *in_bi, int *max_nss);

/** @brief Get the list of associated sta's
 *
 * @param ifname	interface name of the BSS
 * @param out_assoclist	Out object, list of all associated sta's
 * @param listlen	length of the memory allocated for list
 *
 * @return		status of the call. 0 Success. Non Zero Failure
 */
int blanket_get_assoclist(char *ifname, struct maclist *out_assoclist, uint32 listlen);

/** @brief Get power percentage of interface
 *
 * @param ifname	interface name of the BSS
 * @param out_pwr_percent	Out object of type int, get power percent for this radio
 *
 * @return		status of the call. 0 Success. Non Zero Failure
 */
int blanket_get_pwr_percent(char* ifname, int *out_pwr_percent);

/** @brief Set power percentage of interface
 *
 * @param ifname	interface name of the BSS
 * @param pwr_percent	Out object of type int, filled in power percent for this radio
 *
 * @return		status of the call. 0 Success. Non Zero Failure
 */
int blanket_set_pwr_percent(char *ifname, int pwr_percent);

/** @brief Enables STA monitoring feature
 *
 * @param ifname	interface name of the BSS
 *
 * @return		status of the call. 0 Success. Non Zero Failure
 */
int blanket_stamon_enable(char *ifname);

/** @brief Disbles STA monitoring feature
 *
 * @param ifname	interface name of the BSS
 *
 * @return		status of the call. 0 Success. Non Zero Failure
 */
int blanket_stamon_disable(char *ifname);

/** @brief Add a STA's mac address to monitored STA's list
 *
 * @param ifname	interface name of the BSS
 * @param sta_mac	STA's mac address, which needs to be added for monitoring
 * @param offchan_chspec	Chanspec for which AP should go off channel to monitor STA
 * @param offchan_time	Time (ms) for which off-channel STA's are monitored
 *
 * @return		status of the call. 0 Success. Non Zero Failure
 */
int blanket_stamon_add_mac(char *ifname, struct ether_addr *sta_mac,
	chanspec_t offchan_chspec, uint32 offchan_time);

/** @brief Delete a STA's mac address from monitored STA's list
 *
 * @param ifname	interface name of the BSS
 * @param sta_mac	STA's mac address, which needs to be removed for monitoring
 *
 * @return		status of the call. 0 Success. Non Zero Failure
 */
int blanket_stamon_delete_mac(char *ifname, struct ether_addr *sta_mac);

/** @brief Get STAs' monitoed statastics from driver
 *
 * @param ifname	interface name of the BSS
 * @param sta_list	list of mac addresses and their data, which are currently being monitored
 * @param listlen	length of the memory allocated for list
 *
 * @return		status of the call. 0 Success. Non Zero Failure
 */
int blanket_stamon_get_stats(char *ifname, stamon_info_t *sta_list, int listlen);

/** @brief Get counters
 *
 * @param ifname	interface name
 * @param counters	Counters to be filled
 *
 * @return		status of the call. 0 Success. Non Zero Failure
 */
int blanket_get_counters(char *ifname, blanket_counters_t *counters);

/** @brief Check if Interface is Virtual or not
 *
 * @param ifname	interface name
 *
 * @return		TRUE if interface is Virtual(MBSS), FALSE if its Pramary interface
 */
bool blanket_is_interface_virtual(char *ifname);

/** @brief Check if Interface and its Primary Radio is enabled or not
 *
 * @param ifname	interface name
 * @param validate_vif	If interface to be verified is MBSS(Virtual), select TRUE, else FALSE
 * @param error		status of the call. 0 Success. Non Zero Failure
 *
 * @return		1 if interface is enable, 0 if disable
 */
int blanket_is_interface_enabled(char *ifname, bool validate_vif, int *error);

/** @brief Get wlX_ or wlX.y_ Prefix from OS specific interface name
 *
 * @param ifname	interface name
 * @param prefix	out prefix of the Interface name wlX.y_
 * @param prefix_len	length of the buffer which will hold the out prefix
 *
 * @return		status of the call. 0 Success. Non Zero Failure
 */
int blanket_get_interface_prefix(char *ifname, char *prefix, int prefix_len);

/** @brief Get Primary Interface(radio) prefix wlX_, from OS specific MBSS(Virtual) interface name
 *
 * @param ifname	interface name
 * @param prefix	out prefix of the Primary Interface(radio) wlX_
 * @param prefix_len	length of the buffer which will hold the out prefix
 *
 * @return		status of the call. 0 Success. Non Zero Failure
 */
int blanket_get_radio_prefix(char* ifname, char *radio_prefix, int prefix_len);

/** @brief Get Primary Interface(radio) prefix wlX_, from OS specific MBSS(Virtual) interface name
 * based on NVRAM that means without calling driver
 *
 * @param ifname	interface name
 * @param prefix	out prefix of the Primary Interface(radio) wlX_
 * @param prefix_len	length of the buffer which will hold the out prefix
 *
 * @return		status of the call. 0 Success. Non Zero Failure
 */
int blanket_get_radio_prefix_from_nvram(char* ifname, char *radio_prefix, int prefix_len);

/** @brief Blanket API to get the NVRAM value.
 *
 * @param nvram		Name of the NVRAM
 *
 * @return		value of variable or NULL String if undefined
 */
char* blanket_nvram_safe_get(const char *nvram);

/** @brief Blanket API to get the NVRAM value for specific BSS prefix
 *
 * @param prefix	interface prefix of the BSS
 * @param nvram		Name of the NVRAM
 *
 * @return		value of variable or NULL String if undefined
 */
char* blanket_nvram_prefix_safe_get(const char *prefix, const char *nvram);

/** @brief Blanket API to set the NVRAM value for specific BSS prefix
 *
 * @param prefix	interface prefix of the BSS
 * @param nvram		Name of the NVRAM
 * @param nvramval	Value to be set to the NVRAM Name for this prefix
 *
 * @return		status of the call. 0 Success. Non Zero errno on Failure
 */
int blanket_nvram_prefix_set(const char *prefix, const char *nvram, const char *nvramval);

/** @brief Blanket API to unset the NVRAM value for specific BSS prefix
 *
 * @param prefix	interface prefix of the BSS
 * @param nvram		Name of the NVRAM
 *
 * @return		status of the call. 0 Success. Non Zero errno on Failure
 */
int blanket_nvram_prefix_unset(const char *prefix, const char *nvram);

/** @brief Match NVRAM and ARG value, and if mismatch, Set new value in NVRAM
 *
 * @param prefix	interface prefix of the BSS
 * @param nvram		Name of the NVRAM
 * @param new_value	New Value to be compared with, and set to the NVRAM Name in case mismatch
 * @param cmp_option	Of type bkt_cmp_option_t
 *
 * @return		If Value is Overidden, Return 1, else Return 0
 */
int blanket_nvram_prefix_match_set(const char* prefix, char* nvram, char* new_value,
	bkt_cmp_option_t cmp_option);

/**  @brief Gets the string config val from NVARM, if not found applies the default value
 * @param prefix	interface prefix of the BSS
 * @param nvram		Name of the NVRAM
 * @param def		Default value for the NVRAM, in case NVRAM not already set
 *
 * @param value		Either Default value or Fetched value of this NVRAM
 */
void
blanket_get_config_val_str(char* prefix, const char *nvram, char *def, char **val);
/** @brief Gets the unsigned integer config val from NVRAM, if not found applies the default value
 *
 * @param prefix	interface prefix of the BSS
 * @param nvram		Name of the NVRAM
 * @param def		Default uint32 value for the NVRAM, in case NVRAM not already set
 *
 * @return		uint32, Either Default value or Fetched value of this NVRAM
 */
uint32 blanket_get_config_val_uint(char* prefix, const char *nvram, uint32 def);

/** @brief Gets the integer config val from NVARM, if not found applies the default value
 *
 * @param prefix	interface prefix of the BSS
 * @param nvram		Name of the NVRAM
 * @param def		Default int value for the NVRAM, in case NVRAM not already set
 *
 * @return		int, Either Default value or Fetched value of this NVRAM
 */
int blanket_get_config_val_int(char* prefix, const char *nvram, int def);

/** @brief Enable/Disable BSS_LOAD Information Element in Beacon & Probe_Resp
 *
 * @param ifname	interface name
 * @param enable	1 if IE needs to be enabled, else 0 if needs to be disabled
 *
 * @return		status of the call. 0 Success. Non Zero errno on Failure
 */
int blanket_enable_bssload_ie(char* ifname, int enable);

/** @brief Calculate Off-Channel Channel Utilization Using Chanim Stats cca values
 *
 * @param ifname	interface name
 * @param ccastats	ccastats pointer of all CCA Stats
 *
 * @return		Calculated Off-Channel Channel Utilization from Chanim Stats cca values
 */
unsigned char blanket_calc_chanutil_offchan(char *ifname, uint8 *ccastats);

/** @brief Calculate Channel Utilization Using Chanim Stats cca values
 *
 * @param ifname	interface name
 * @param cca0_txdur_needed	flag to decide if TxDur is needed in Chan Util Calculation
 * @param ccastats	ccastats pointer of all CCA Stats
 *
 * @return		Calculated Channel Utilization from Chanim Stats cca values
 */
unsigned char blanket_calc_chanutil(char *ifname, bool cca0_txdur_needed, uint8 *ccastats);

/** @brief Get Chaim Stats
 *
 * @param ifname	interface name
 * @param  count	WL_CHANIM_COUNT_ONE, WL_CHANIM_COUNT_ALL etc
 * @param data_buf	buffer to get the chanim stats
 * @param buflen	length of the memory allocated as buffer
 *
 * @return		status of the call. 0 Success. Non Zero errno on Failure
 */
int blanket_get_chanim_stats(char* ifname, uint32 count, char* data_buf, int buflen);

/** @brief Get count of associated sta
 *
 * @param ifname	interface name of the BSS
 * @param assoc_count	Out object, count of associated STAs to this interface
 *
 * @return		status of the call. 0 Success. Non Zero Failure
 */
int blanket_get_assoc_sta_count(char *ifname, int *assoc_count);

/** @brief Sends beacon request to assocociated STA
 *
 * @param ifname	interface name of the BSS
 * @param bcnreq_extn	Beacon Request data as per 802.11 spec. Includes:  STA MAC,
 *			operating class, channel number, randomization interval, measurement mode,
 *			measurement duration, target bssid, target ssid, repeations, source bssid
 * @param sub_element_data	Optional Subelement binary data, can be set with beacon reuqest
 * @param sub_element_len	Optional Subelement binary data length including TLV header lengths
 * @param ret_token		Token which is used in the beacon request action frame
 *
 * @return		status of the call. 0 Success. Non Zero Failure
 */
int blanket_send_beacon_request(char *ifname, blanket_bcnreq_t *bcnreq_extn,
	uint8 *sub_element_data, int sub_element_len, int *ret_token);

/** @brief Get amsdu enabled/disabled
 *
 * @param ifname	interface name of the BSS
 * @param amsdu		Out object, amsdu enabled/disabled
 *
 * @return		status of the call. 0 Success. Non Zero Failure
 */
int blanket_is_amsdu_enabled(char *ifname, int *amsdu);

/** @brief Get ampdu enabled/disabled
 *
 * @param ifname	interface name of the BSS
 * @param ampdu		Out object, ampdu enabled/disabled
 *
 * @return		status of the call. 0 Success. Non Zero Failure
 */
int blanket_is_ampdu_enabled(char *ifname, int *ampdu);

/** @brief Get ampdu block ack window size
 *
 * @param ifname	interface name of the BSS
 * @param ba_wsize	Out object, ampdu block ack window size
 *
 * @return		status of the call. 0 Success. Non Zero Failure
 */
int blanket_get_ampdu_ba_wsize(char *ifname, int *ba_wsize);

/** @brief Get the Max rate of an interface
 *
 * @param ifname	interface name of the BSS
 * @param outmaxrate	Out object, Max Rate
 *
 * @return		status of the call. 0 Success. Non Zero Failure
 */
int blanket_get_max_rate(char *ifname, int *out_maxrate);

/** @brief Get the STA counters from bs_data
 *
 * @param ifname	interface name of the BSS
 * @param sta_mac	MAC address of the STA
 * @param out_ctr	Out object, counters
 *
 * @return		status of the call. 0 Success. Non Zero Failure
 */
int blanket_get_bs_data_counters(char *ifname, struct ether_addr *sta_mac,
	iov_bs_data_counters_t *out_ctr);

/** @brief Get the bandwidth cap
 *
 * @param ifname	interface name of the BSS
 * @param band		Band WLC_BAND_2G or WLC_BAND_5G
 *
 * @return		returns bw_cap
 */
uint32 blanket_get_bw_cap(char *ifname, uint32 band);

/** @brief Check if the BSS is up or down
 *
 * @param ifname	interface name of the BSS
 * @param bsscfg_idx	Index of BSS
 *
 * @return		returns 0 if the BSS is down, else up
 */
int blanket_is_bss_enabled(char *ifname, int bsscfg_idx);

/** @brief Enable(Bring up) the BSS
 *
 * @param ifname	interface name of the BSS
 * @param bsscfg_idx	Index of BSS
 *
 * @return		status of call. 0 Success. Non Zero Failure
 */
int blanket_bss_enable(char *ifname, int bsscfg_idx);

/** @brief Disable(Bring down) the BSS
 *
 * @param ifname	interface name of the BSS
 * @param bsscfg_idx	Index of BSS
 *
 * @return		status of call. 0 Success. Non Zero Failure
 */
int blanket_bss_disable(char *ifname, int bsscfg_idx);

/** @brief Set dfs pref chan list
 *
 * @param ifname	interface name of the BSS
 * @param dfs_frcd	chan list to set
 * @param ioctl_size	input buffer size
 *
 * @return		status of call. 0 Success. Non Zero Failure
 */
int blanket_set_dfs_forced_chspec(char* ifname, wl_dfs_forced_t* dfs_frcd, int ioctl_size);

/** @brief Keep virtual APs up, even if the primary sta is not associated
 *
 * @param ifname	interface name
 * @param up		if 1, keep ap up even if sta not associated, else if 0,
 *			ap down if sta not associated
 *
 * @return		status of call. 0 Success. Non Zero Failure
 */
int blanket_set_keep_ap_up(char* ifname, int up);

/** @brief Enable/diable raoming of STA interface
 *
 * @param ifname	interface name
 * @param enable	if 1, enable cfg80211_roam  else if 0, disable cfg80211_roam
 *
 * @return		status of call. 0 Success. Non Zero Failure
 */
int blanket_set_cfg80211_roam(char* ifname, int enable);

/** @brief Set roam trigger(RSSI)
 *
 * @param ifname	interface name
 * @param rssi		Roam trigger RSSI threshold
 *
 * @return		status of call. 0 Success. Non Zero Failure
 */
int blanket_set_roam_trigger(char* ifname, int rssi_thld);

/** @brief Set scan home time in ms for interface
 *
 * @param ifname	interface name
 * @param scan_home_time  scan home time
 *
 * @return		status of call. 0 Success. Non Zero Failure
 */
int blanket_set_scan_home_time(char *ifname, int scan_home_time);

/** @brief Set roam delta
 *
 * @param ifname	interface name
 * @param delta		roam delta value
 *
 * @return		status of call. 0 Success. Non Zero Failure
 */
int blanket_set_roam_delta(char* ifname, int delta);

/** @brief Get Beacon Period of a BSS
 *
 * @param ifname		interface name of the BSS
 * @param out_beacon_period	Out object of type uint16, beacon period of this BSS
 *
 * @return			status of the call. 0 Success. Non Zero Failure
 */
int blanket_get_beacon_period(char *ifname, uint16 *out_beacon_period);

/** @brief Get the scan version
 *
 *   @param ifname		interface name
 *   @param ver		Out Scan version
 *
 *   @return			status of the call. 0 Success. Non Zero Failure
 */
int blanket_get_escan_ver(char *ifname, wl_scan_version_t *ver);

/** @brief Get the revinfo of radio
 *
 *   @param ifname		interface name
 *   @param revinfo		revinfo of the radio
 *
 *   @return			status of the call. 0 Success. Non Zero Failure
 */

int blanket_get_revinfo(char *ifname, wlc_rev_info_t *out_rev);

/* Prepare scan params from channel scan request */
int blanket_escan_prep_cs(char *ifname, ieee1905_per_radio_opclass_list *ifr,
	void *params, int *params_size, uint16 version);

/* Start the escan */
int blanket_escan_start(char *ifname, void *params, int params_size);

/* Abort scan */
int blanket_escan_abort(char *ifname);

/* get cac capability from driver */
int blanket_get_driver_capability(char *ifname, char *pbuf, uint16 max_len);

/* current CLM is EU compliant ? */
int blanket_get_is_edcrs_eu(char *ifname, uint32 *is_edcrs_eu);

/** @brief Get the backhaul STA MAC address for the WDS interface name
 *
 * @param ifname		interface name of the BSS
 * @param wds_ifname		WDS interface name
 * @param mac			Backhaul STA MAC to be returned
 *
 * @return			status of the call. 0 Success. Non Zero Failure
 */
int blanket_get_bh_sta_mac_from_wds(char *ifname, char *wds_ifname, unsigned char *mac);

/** @brief Get the Channel Utilization & Station Count fields Present in QBSS LOAD IE
 *
 * @param in_bi		In object of type wl_bss_info_t,
 *			bss_info object already extracted
 * @param chan_util	Channel Utilization field Present in QBSS LOAD IE
 * @param sta_count	Station Count field Present in QBSS LOAD IE
 *
 * @return			status of the call. 0 Success. Non Zero Failure
 */
int blanket_get_qbss_load_element(wl_bss_info_t *in_bi,
	unsigned char *chan_util, unsigned short *sta_count);

/** @brief Set or unset Association Disallowed attribute in the Beacon.
 *
 * @param ifname	interface name of the BSS
 * @param reason	Possible values for reason,
 *			0 - Allow association
 *			1 - Reason unspecified
 *			2 - Max sta limit reached
 *			3 - Air interface overload
 *			4 - Auth server overload
 *			5 - Insufficient RSSI
 *
 * @return		status of the call. 0 Success. Non Zero Failure
 */
int blanket_mbo_assoc_disallowed(char *ifname, uint8 reason);

/* get driver dfs cac status */
int blanket_get_dfs_status_all(char *ifname, wl_dfs_status_all_t *dfs_status_all, int len);

/* Get Primary VLAN ID */
int blanket_get_primary_vlan_id(char *ifname, unsigned short *vlan_id);

/** @brief calculates the txrate from eht mcs, nss and bw
 *
 * @param mcs_map	eht mcs map
 * @param nss		nss
 * @param bw		bandwidth
 *
 * @return		status of the call. 0 Success. Non Zero Failure
 */
int blanket_get_txrate_from_eht_mcs(uint32 mcs_map, int nss, int bw, uint32 *out_rate);

/** @brief calculates the txrate from vht mcs, nss and bw
 *
 * @param mcs_map	vht mcs map
 * @param nss		nss
 * @param bw		bandwidth
 * @param he		11ax supported
 *
 * @return		status of the call. 0 Success. Non Zero Failure
 */
int blanket_get_txrate(uint16 mcs_map, int nss, int bw, uint32 *out_rate, bool he);

/** @brief gets the current country code for the given interface
 *
 * @param ifname	name of the interface
 * @param out_country	Out object for country code
 *
 * @return		status of the call. 0 Success. Non Zero Failure
 */
int blanket_get_country(char *ifname, wl_country_t *out_country);

/** @brief Set Interface specific chan_info
 *
 * @param ifname	name of the interface
 * @param chanspec	chanspec to set this chan info
 * @param per_chan_info	channel information to set
 *
 * @return		status of the call. 0 Success. Non Zero Failure
 */
int blanket_set_chan_info(char* ifname, chanspec_t chanspec, uint32 per_chan_info);

/* create log default nvrams filename */
void blanket_log_get_default_nvram_filename(char *fname, int len);

/* get log of the default nvram names with the values to a file */
void blanket_log_default_nvram(const char *fmt, ...);

/* get nvram settings to enable/disable file operation */
void init_blanket_info(blanket_module_info_t *info);

/** @brief Get vhtmode enabled/disabled
 *
 * @param ifname	interface name of the BSS
 * @param vhtmode	Out object, vhtmode enabled/disabled
 *
 * @return		status of the call. 0 Success. Non Zero Failure
 */
int blanket_get_vhtmode(char *ifname, int *vhtmode);

/** @brief Get wme enabled/disabled
 *
 * @param ifname	interface name of the BSS
 * @param wme		Out object, wme enabled/disabled
 *
 * @return		status of the call. 0 Success. Non Zero Failure
 */

int blanket_get_wme(char *ifname, int *wme);

/** @brief Get wme_apsd enabled/disabled
 *
 * @param ifname	interface name of the BSS
 * @param wme_apsd	Out object, wme_apsd enabled/disabled
 *
 * @return		status of the call. 0 Success. Non Zero Failure
 */
int blanket_get_wme_apsd(char *ifname, int *wme_apsd);

/** @brief Enable/disable bgdfs on the apsta interface
 *
 * @param ifname	interface name
 * @param enable	if 1, enable bgdfs for dfs_ap_move  else if 0, disable dfs_ap_move
 *
 * @return		status of call. 0 Success. Non Zero Failure
 */
int blanket_set_allow_bgdfs_on_apsta(char* ifname, int enable);

/** @brief Check whether the 20MHz channel is in the chanspec
 *
 * @param chanspec	Chanspec on which we need to check
 * @param ctrl_chn	20MHz channel to be checked
 *
 * @return		1 20MHz channel present in chanspec. 0 Not present
 */
int blanket_is_20mhz_channel_in_chanspec(chanspec_t chanspec, uint8 ctrl_chn);

/** @brief Enable/diable sending CSA on the Client(bSTA)
 *
 * @param ifname	interface name
 * @param enable	if 1, enable client csa else if 0, disable client csa
 *
 * @return		status of call. 0 Success. Non Zero Failure
 */
int blanket_set_client_csa(char* ifname, int enable);

/** @brief Checks whether ifname is a member of multiple bssid set
 *
 * @param ifname	interface name
 * @param out_val	out object, multiple bssid enabled/disabled
 *
 * @return		status of call. 0 Success. Non Zero Failure
 */
int blanket_is_bss_partof_mbssid_set(char *ifname, int *out_val);

/** @brief Checks whether ifname is a transmitted bssid or not
 *
 * @param ifname	interface name
 * @param out_val	out object, transmit_bss enabled/disabled
 *
 * @return		status of call. 0 Success. Non Zero Failure
 */
int blanket_is_bss_transmitted_bss(char *ifname, int *out_val);

/** @brief Set FBT reassoc time
 *
 * @param ifname	interface name
 * @param reassoc_time	FBT reassoc time to be set
 *
 * @return		status of call. 0 Success. Non Zero Failure
 */
int blanket_set_fbt_reassoc_time(char *ifname, int reassoc_time);

/** @brief Set tx power of interface
 *
 * @param ifname	interface name of the BSS
 * @param txpwr		tx power in qdBm to be set for this radio
 *
 * @return		status of the call. 0 Success. Non Zero Failure
 */
int blanket_set_txpwr(char *ifname, int txpwr);

/** @brief Check if MLO is enabled or not
 *
 * @param ifname	interface name
 * @param out_enab	out object, MLO eanbled/disabled
 *
 * @return		status of call. 0 Success. Non Zero Failure
 */
int blanket_get_mlo_enab(char* ifname, uint8* out_enab);

/** @brief Get iovar output of type int
 *
 * @param ifname	interface name
 * @param iovar		name of the iovar
 * @param out_val	out object, of int type
 *
 * @return		status of call. 0 Success. Non Zero Failure
 */
int blanket_iovar_getint(char *ifname, char *iovar, int *out_val);

/** @brief check ifname in mldx_ifnames based on whether MLO is eanbled or not
 *
 * @param mld_unit		x in mldx_ifnames NVRAM
 * @param ifname		ifnames to be checked in mldx_ifnames NVRAM
 * @param is_mlo_enabled	MLO is enabled or not
 *
 * @return			NVRAM is matching or not. 1 If it is not matching. 0 if matches
 */
int blanket_nvram_prefix_match_mld_ifnames(int8 mld_unit, char *ifname, bool is_mlo_enabled);

/** @brief Get AP MLD address
 *
 * @param ifname	interface name
 * @param out_ap_mld	out object, MLD address
 *
 * @return		status of call. 0 Success. Non Zero Failure
 */
int blanket_get_ap_mld(char* ifname, struct ether_addr *out_ap_mld);

/** @brief Blanket Node get the peer MLD MAC address of STA
 *
 * @param ifname	interface name of the BSS
 * @param sta_mac	MAC address of STA from AP side. bSTA side, pass BSSID where it connected
 * @param out_peer_mld	Peer MLD MAC address of STA
 *
 * @return		status of the call. 0 Success. Non Zero Failure
 */
int blanket_get_peer_mld_mac(char *ifname, struct ether_addr *sta_mac,
	struct ether_addr *out_peer_mld);

/** @brief Get MLO link stats icluding RSSI
 *
 * @param ifname		interface name of the BSS
 * @param sta_mac		MAC address of STA from AP side
 * @param out_link_stats	MLO link status
 * @param arr_size		link status array size
 *
 * @return			status of the call. 0 Success. Non Zero Failure
 */
int blanket_get_mlo_link_stats(char *ifname, struct ether_addr *sta_mac,
	ieee1905_mlo_sta_link_stats *out_link_stats, int arr_size);

#endif /* __BLANKET_H__ */
