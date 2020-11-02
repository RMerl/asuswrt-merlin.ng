/** @file blanket.h
 *  @brief Function prototypes for Wi-Fi Blanket API 2.
 *
 * Broadcom Blanket library include file
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
 *
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: blanket.h 786983 2020-05-13 03:06:28Z $
 *
 *  @author
 *  @bug No known bugs.
 */

#ifndef __BLANKET_H__
#define __BLANKET_H__

#include "ieee1905.h"

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
} blanket_module_info_t;

#if defined(WL_STA_VER_8)
typedef sta_info_v8_t sta_info_t;
#endif // endif

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
/* Phytypes */
typedef enum dot11PhyType {
	WBD_DOT11_PHYTYPE_DSSS		= 2,
	WBD_DOT11_PHYTYPE_OFDM		= 4,
	WBD_DOT11_PHYTYPE_HRDSSS	= 5,
	WBD_DOT11_PHYTYPE_ERP		= 6,
	WBD_DOT11_PHYTYPE_HT		= 7,
	WBD_DOT11_PHYTYPE_DMG		= 8,
	WBD_DOT11_PHYTYPE_VHT		= 9,
	WBD_DOT11_PHYTYPE_TVHT		= 10
} dot11Phytype_t;

/** @brief Initialize blanket module
 *
 * @param info		info structure for inializing the module
 *
 * @return		status of the call. 0 Success. Non Zero Failure
 */
int blanket_module_init(blanket_module_info_t *info);

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
 * @param out_bitmap	Out object, a combination of a bitflags for channel attributes and
 *                      a bitfield for a channel out-of-service duration
 *
 * @return		status of the call. 0 Success. Non Zero Failure
 */
int blanket_get_chan_info(char* ifname, uint channel, uint *out_bitmap);

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

/** @brief Get APSTA configuration of interface
 *
 * @param ifname	interface name of the BSS
 * @param apsta		Out mode of interface as APSTA
 *
 * @return		status of the call. 0 Success. Non Zero Failure
 */
int blanket_get_apsta_mode(char *ifname, uchar *apsta);

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

/** @brief Get max no. of sta's that can be associated
 *
 * @param ifname	interface name of the BSS
 * @param maxassoc	Out object, max no. of sta's that can be associated
 *
 * @return		status of the call. 0 Success. Non Zero Failure
 */
int blanket_get_max_assoc(char *ifname, int *maxassoc);

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
 * @param matchcase	Compare with or without case, TRUE = with case, FALSE = without case
 *
 * @return		If Value is Overidden, Return 1, else Return 0
 */
int blanket_nvram_prefix_match_set(const char* prefix, char* nvram, char* new_value,
	bool matchcase);

/**  @brief Gets the string config val from NVARM, if not found applies the default value
 * @param prefix	interface prefix of the BSS
 * @param nvram		Name of the NVRAM
 * @param def		Default value for the NVRAM, in case NVRAM not already set
 *
 * @param value		Either Default value or Fetched value of this NVRAM
 */
void
blanket_get_config_val_str(char* prefix, const char *nvram, char *def, char **val);
/** @brief Gets the unsigned integer config val from NVARM, if not found applies the default value
 *
 * @param prefix	interface prefix of the BSS
 * @param nvram		Name of the NVRAM
 * @param def		Default uint16 value for the NVRAM, in case NVRAM not already set
 *
 * @return		uint16, Either Default value or Fetched value of this NVRAM
 */
uint16 blanket_get_config_val_uint(char* prefix, const char *nvram, uint16 def);

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
 * @param in_bi		In object of type wl_bss_info_t,
 *			bss_info object if already extracted (optional, can be NULL)
 * @param outmaxrate	Out object, Max Rate
 *
 * @return		status of the call. 0 Success. Non Zero Failure
 */
int blanket_get_max_rate(char *ifname, wl_bss_info_t *in_bi, int *outmaxrate);

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
 * @param enable	if 1, enable roam off  else if 0, disable roam off
 *
 * @return		status of call. 0 Success. Non Zero Failure
 */
int blanket_set_roam_off(char* ifname, int enable);

/** @brief Get Beacon Period of a BSS
 *
 * @param ifname		interface name of the BSS
 * @param out_beacon_period	Out object of type uint16, beacon period of this BSS
 *
 * @return			status of the call. 0 Success. Non Zero Failure
 */
int blanket_get_beacon_period(char *ifname, uint16 *out_beacon_period);

/* Prepare scan params from channel scan request */
int blanket_escan_prep_cs(ieee1905_per_radio_opclass_list *ifr,
	wl_scan_params_t *params, int *params_size);

/* Start the escan */
int blanket_escan_start(char *ifname, wl_escan_params_t *params, int params_size);

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
int blanket_get_dfs_status(char *ifname, wl_dfs_status_t *dfs_status);

/* Get Primary VLAN ID */
int blanket_get_primary_vlan_id(char *ifname, unsigned short *vlan_id);

/** @brief calculates the txrate from vht mcs, nss and bw
 *
 * @param mcs_map	vht mcs map
 * @param nss		nss
 * @param bw		bandwidth
 *
 * @return		status of the call. 0 Success. Non Zero Failure
 */
int blanket_get_txrate(uint16 mcs_map, int nss, int bw, uint32 *out_rate);
#endif /* __BLANKET_H__ */
