/*
 * Broadcom Corporation OUI and vendor specific assignments
 * Copyright (c) 2020, Broadcom Corporation.
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#ifndef BRCM_VENDOR_H
#define BRCM_VENDOR_H

/*
 * This file is a registry of identifier assignments from the Broadcom
 * OUI 00:10:18 for purposes other than MAC address assignment. New identifiers
 * can be assigned through normal review process for changes to the upstream
 * hostap.git repository.
 */

#define OUI_BRCM    0x001018

/**
 * enum brcm_nl80211_vendor_subcmds - BRCM nl80211 vendor command identifiers
 *
 * @BRCM_VENDOR_SCMD_UNSPEC: Reserved value 0
 *
 * @BRCM_VENDOR_SCMD_PRIV_STR: Provide vendor private cmds to send to FW.
 *
 * @BRCM_VENDOR_SCMD_BCM_STR:  Provide vendor cmds to BCMDHD driver.
 *
 * @BRCM_VENDOR_SCMD_BCM_PSK: Used to set SAE password.
 *
 * @BRCM_VENDOR_SCMD_SET_PMK: Command to check driver support
 *	for DFS offloading.
 *
 * @BRCM_VENDOR_SCMD_GET_FEATURES: Command to get the features
 *      supported by the driver.
 *
 * @BRCM_VENDOR_SCMD_SET_MAC: Set random mac address for P2P interface.
 *
 * @BRCM_VENDOR_SCMD_SET_CONNECT_PARAMS: Set some connect parameters.
 *      Used for the case that FW handle SAE.
 *
 * @BRCM_VENDOR_SCMD_SET_START_AP_PARAMS: Set SoftAP paramters.
 *      Used for the case that FW handle SAE.
 *
 * @BRCM_VENDOR_SCMD_ACS: ACS command/event which is used to
 *	invoke the ACS function in device and pass selected channels to
 *	hostapd. Uses enum qca_wlan_vendor_attr_acs_offload attributes.
 *
 * @BRCM_VENDOR_SCMD_MAX: This acts as a the tail of cmds list.
 *      Make sure it located at the end of the list.
 *
 */
enum brcm_nl80211_vendor_subcmds {
	BRCM_VENDOR_SCMD_UNSPEC			= 0,
	BRCM_VENDOR_SCMD_PRIV_STR		= 1,
	BRCM_VENDOR_SCMD_BCM_STR		= 2,
	BRCM_VENDOR_SCMD_BCM_PSK		= 3,
	BRCM_VENDOR_SCMD_SET_PMK		= 4,
	BRCM_VENDOR_SCMD_GET_FEATURES		= 5,
	BRCM_VENDOR_SCMD_SET_MAC		= 6,
	BRCM_VENDOR_SCMD_SET_CONNECT_PARAMS	= 7,
	BRCM_VENDOR_SCMD_SET_START_AP_PARAMS	= 8,
	BRCM_VENDOR_SCMD_ACS			= 9,
	BRCM_VENDOR_SCMD_MAX			= 10
};

/**
 * enum brcm_nl80211_vendor_events - BRCM nl80211 asynchoronous event identifiers
 *
 * @BRCM_VENDOR_EVENT_UNSPEC: Reserved value 0
 *
 * @BRCM_VENDOR_EVENT_PRIV_STR: String command/event
 */
enum brcm_nl80211_vendor_events {
	BRCM_VENDOR_EVENT_UNSPEC		= 0,
	BRCM_VENDOR_EVENT_PRIV_STR		= 1,
#ifdef CONFIG_DRIVER_NL80211_BRCM
	BRCM_VENDOR_EVENT_PASN			= 2,
#else
	GOOGLE_GSCAN_SIGNIFICANT_EVENT		= 2,
	GOOGLE_GSCAN_GEOFENCE_FOUND_EVENT	= 3,
	GOOGLE_GSCAN_BATCH_SCAN_EVENT		= 4,
	GOOGLE_SCAN_FULL_RESULTS_EVENT		= 5,
	GOOGLE_RTT_COMPLETE_EVENT		= 6,
	GOOGLE_SCAN_COMPLETE_EVENT		= 7,
	GOOGLE_GSCAN_GEOFENCE_LOST_EVENT	= 8,
	GOOGLE_SCAN_EPNO_EVENT			= 9,
	GOOGLE_DEBUG_RING_EVENT			= 10,
	GOOGLE_FW_DUMP_EVENT			= 11,
	GOOGLE_PNO_HOTSPOT_FOUND_EVENT		= 12,
	GOOGLE_RSSI_MONITOR_EVENT		= 13,
	GOOGLE_MKEEP_ALIVE_EVENT		= 14,

	/*
	 * BRCM specific events should be placed after
	 * the Generic events so that enums don't mismatch
	 * between the DHD and HAL
	 */
	GOOGLE_NAN_EVENT_ENABLED		= 15,
	GOOGLE_NAN_EVENT_DISABLED		= 16,
	GOOGLE_NAN_EVENT_SUBSCRIBE_MATCH	= 17,
	GOOGLE_NAN_EVENT_REPLIED		= 18,
	GOOGLE_NAN_EVENT_PUBLISH_TERMINATED	= 19,
	GOOGLE_NAN_EVENT_SUBSCRIBE_TERMINATED	= 20,
	GOOGLE_NAN_EVENT_DE_EVENT		= 21,
	GOOGLE_NAN_EVENT_FOLLOWUP		= 22,
	GOOGLE_NAN_EVENT_TRANSMIT_FOLLOWUP_IND	= 23,
	GOOGLE_NAN_EVENT_DATA_REQUEST		= 24,
	GOOGLE_NAN_EVENT_DATA_CONFIRMATION	= 25,
	GOOGLE_NAN_EVENT_DATA_END		= 26,
	GOOGLE_NAN_EVENT_BEACON			= 27,
	GOOGLE_NAN_EVENT_SDF			= 28,
	GOOGLE_NAN_EVENT_TCA			= 29,
	GOOGLE_NAN_EVENT_SUBSCRIBE_UNMATCH	= 30,
	GOOGLE_NAN_EVENT_UNKNOWN		= 31,
	GOOGLE_ROAM_EVENT_START			= 32,
	BRCM_VENDOR_EVENT_HANGED                = 33,
	BRCM_VENDOR_EVENT_SAE_KEY               = 34,
	BRCM_VENDOR_EVENT_BEACON_RECV           = 35,
	BRCM_VENDOR_EVENT_PORT_AUTHORIZED       = 36,
	GOOGLE_FILE_DUMP_EVENT			= 37,
	BRCM_VENDOR_EVENT_CU			= 38,
	BRCM_VENDOR_EVENT_WIPS			= 39,
	NAN_ASYNC_RESPONSE_DISABLED		= 40,
	BRCM_VENDOR_EVENT_RCC_INFO		= 41,
	BRCM_VENDOR_EVENT_ACS			= 42,
#endif /* CONFIG_DRIVER_NL80211_BRCM */
	BRCM_VENDOR_EVENT_LAST

};

#ifdef CONFIG_BRCM_SAE
enum wifi_sae_key_attr {
	BRCM_SAE_KEY_ATTR_BSSID,
	BRCM_SAE_KEY_ATTR_PMK,
	BRCM_SAE_KEY_ATTR_PMKID
};
#endif /* CONFIG_BRCM_SAE */

enum wl_vendor_attr_acs_offload {
	BRCM_VENDOR_ATTR_ACS_CHANNEL_INVALID = 0,
	BRCM_VENDOR_ATTR_ACS_PRIMARY_FREQ,
	BRCM_VENDOR_ATTR_ACS_SECONDARY_FREQ,
	BRCM_VENDOR_ATTR_ACS_VHT_SEG0_CENTER_CHANNEL,
	BRCM_VENDOR_ATTR_ACS_VHT_SEG1_CENTER_CHANNEL,

	BRCM_VENDOR_ATTR_ACS_HW_MODE,
	BRCM_VENDOR_ATTR_ACS_HT_ENABLED,
	BRCM_VENDOR_ATTR_ACS_HT40_ENABLED,
	BRCM_VENDOR_ATTR_ACS_VHT_ENABLED,
	BRCM_VENDOR_ATTR_ACS_CHWIDTH,
	BRCM_VENDOR_ATTR_ACS_CH_LIST,
	BRCM_VENDOR_ATTR_ACS_FREQ_LIST,

	BRCM_VENDOR_ATTR_ACS_LAST
};

#ifdef CONFIG_DRIVER_NL80211_BRCM
#ifdef CONFIG_PASN
enum brcm_wlan_vendor_attr_pasn_peer {
	BRCM_WLAN_VENDOR_ATTR_PASN_PEER_INVALID = 0,
	BRCM_WLAN_VENDOR_ATTR_PASN_PEER_SRC_ADDR = 1,
	BRCM_WLAN_VENDOR_ATTR_PASN_PEER_MAC_ADDR = 2,
	BRCM_WLAN_VENDOR_ATTR_PASN_PEER_LTF_KEYSEED_REQUIRED = 3,
	BRCM_WLAN_VENDOR_ATTR_PASN_PEER_STATUS_SUCCESS = 4,

	/* keep last */
	BRCM_WLAN_VENDOR_ATTR_PASN_PEER_AFTER_LAST,
	BRCM_WLAN_VENDOR_ATTR_PASN_PEER_MAX =
	BRCM_WLAN_VENDOR_ATTR_PASN_PEER_AFTER_LAST - 1,
};

/**
 * enum brcm_wlan_vendor_attr_pasn: Defines the attributes used in the
 *      QCA_NL80211_VENDOR_SUBCMD_PASN command.
 *
 * @BRCM_WLAN_VENDOR_ATTR_PASN_ACTION: u32 attribute, possible values are
 *      defined in enum qca_wlan_vendor_pasn_action and used only in an event
 *      from the driver to userspace.
 * @BRCM_WLAN_VENDOR_ATTR_PASN_PEERS: Nested attribute, used to pass PASN peer
 *      details for each peer and used in both an event and a command response.
 *      The nested attributes used inside BRCM_WLAN_VENDOR_ATTR_PASN_PEERS are
 *      defined in enum qca_wlan_vendor_attr_pasn_peer.
 */
enum brcm_wlan_vendor_attr_pasn {
        BRCM_WLAN_VENDOR_ATTR_PASN_INVALID = 0,
        BRCM_WLAN_VENDOR_ATTR_PASN_ACTION = 1,
        BRCM_WLAN_VENDOR_ATTR_PASN_PEERS = 2,
        /* keep last */
        BRCM_WLAN_VENDOR_ATTR_PASN_AFTER_LAST,
        BRCM_WLAN_VENDOR_ATTR_PASN_MAX =
        BRCM_WLAN_VENDOR_ATTR_PASN_AFTER_LAST - 1,
};

/**
 * enum brcm_wlan_vendor_pasn_action - Action to authenticate (and generate keys
 *      for) or drop existing PASN security association for the listed the
 *      peers. Used by BRCM_WLAN_VENDOR_ATTR_PASN_ACTION and sent from the driver
 *      to userspace.
 *
 * @BRCM_WLAN_VENDOR_PASN_ACTION_AUTH: Initiate PASN handshake with the peer
 *      devices indicated with %BRCM_WLAN_VENDOR_ATTR_PASN_PEER_MAC_ADDR.
 * @BRCM_WLAN_VENDOR_PASN_ACTION_DELETE_SECURE_RANGING_CONTEXT: Indication from
 *      the driver to userspace to inform that the existing PASN keys of the
 *      peer devices specified with %BRCM_WLAN_VENDOR_ATTR_PASN_PEER_MAC_ADDR are
 *      not valid anymore.
 */
enum brcm_wlan_vendor_pasn_action {
        BRCM_WLAN_VENDOR_PASN_ACTION_AUTH,
        BRCM_WLAN_VENDOR_PASN_ACTION_DELETE_SECURE_RANGING_CONTEXT,
};

#define WPAS_MAX_PASN_PEERS 10
#endif /* CONFIG_PASN */
#endif /* CONFIG_DRIVER_NL80211_BRCM */

#endif /* BRCM_VENDOR_H */
