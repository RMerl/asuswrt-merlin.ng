/*
 * DHD debugability header file
 *
 * Copyright (C) 2025, Broadcom. All Rights Reserved.
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
 * <<Broadcom-WL-IPTag/Dual:>>
 *
 * $Id: dhd_debug.h 841233 2024-06-13 08:13:24Z $
 */

#ifndef _dhd_debug_h_
#define _dhd_debug_h_
#include <event_log.h>
#include <bcmutils.h>
#include <eldbg_ring.h>
#include <linux_dbg.h>

enum {
	/* Feature set */
	DBG_MEMORY_DUMP_SUPPORTED = (1 << (0)), /* Memory dump of FW */
	DBG_PER_PACKET_TX_RX_STATUS_SUPPORTED = (1 << (1)), /* PKT Status */
	DBG_CONNECT_EVENT_SUPPORTED = (1 << (2)), /* Connectivity Event */
	DBG_POWER_EVENT_SUPOORTED = (1 << (3)), /* POWER of Driver */
	DBG_WAKE_LOCK_SUPPORTED = (1 << (4)), /* WAKE LOCK of Driver */
	DBG_VERBOSE_LOG_SUPPORTED = (1 << (5)), /* verbose log of FW */
	DBG_HEALTH_CHECK_SUPPORTED = (1 << (6)), /* monitor the health of FW */
	DBG_DRIVER_DUMP_SUPPORTED = (1 << (7)), /* dumps driver state */
	DBG_PACKET_FATE_SUPPORTED = (1 << (8)), /* tracks connection packets' fate */
	DBG_NAN_EVENT_SUPPORTED = (1 << (9)), /* NAN Events */
	DBG_PACKET_LOG_SUPPORTED = (1 << (10)), /* Packet log with dbgring */
};

#define TLV_LOG_SIZE(tlv) ((tlv) ? (sizeof(tlv_log) + (tlv)->len) : 0)
#define TLV_LOG_NEXT(tlv) \
	((tlv) ? ((tlv_log *)((uint8 *)tlv + TLV_LOG_SIZE(tlv))) : 0)

enum {
	/* driver receive association command from kernel */
	WIFI_EVENT_ASSOCIATION_REQUESTED	= 0,
	WIFI_EVENT_AUTH_COMPLETE,
	WIFI_EVENT_ASSOC_COMPLETE,
	/* received firmware event indicating auth frames are sent */
	WIFI_EVENT_FW_AUTH_STARTED,
	/* received firmware event indicating assoc frames are sent */
	WIFI_EVENT_FW_ASSOC_STARTED,
	/* received firmware event indicating reassoc frames are sent */
	WIFI_EVENT_FW_RE_ASSOC_STARTED,
	WIFI_EVENT_DRIVER_SCAN_REQUESTED,
	WIFI_EVENT_DRIVER_SCAN_RESULT_FOUND,
	WIFI_EVENT_DRIVER_SCAN_COMPLETE,
	WIFI_EVENT_G_SCAN_STARTED,
	WIFI_EVENT_G_SCAN_COMPLETE,
	WIFI_EVENT_DISASSOCIATION_REQUESTED,
	WIFI_EVENT_RE_ASSOCIATION_REQUESTED,
	WIFI_EVENT_ROAM_REQUESTED,
	/* received beacon from AP (event enabled only in verbose mode) */
	WIFI_EVENT_BEACON_RECEIVED,
	/* firmware has triggered a roam scan (not g-scan) */
	WIFI_EVENT_ROAM_SCAN_STARTED,
	/* firmware has completed a roam scan (not g-scan) */
	WIFI_EVENT_ROAM_SCAN_COMPLETE,
	/* firmware has started searching for roam candidates (with reason =xx) */
	WIFI_EVENT_ROAM_SEARCH_STARTED,
	/* firmware has stopped searching for roam candidates (with reason =xx) */
	WIFI_EVENT_ROAM_SEARCH_STOPPED,
	WIFI_EVENT_UNUSED_0,
	/* received channel switch anouncement from AP */
	WIFI_EVENT_CHANNEL_SWITCH_ANOUNCEMENT,
	/* fw start transmit eapol frame, with EAPOL index 1-4 */
	WIFI_EVENT_FW_EAPOL_FRAME_TRANSMIT_START,
	/* fw gives up eapol frame, with rate, success/failure and number retries */
	WIFI_EVENT_FW_EAPOL_FRAME_TRANSMIT_STOP,
	/* kernel queue EAPOL for transmission in driver with EAPOL index 1-4 */
	WIFI_EVENT_DRIVER_EAPOL_FRAME_TRANSMIT_REQUESTED,
	/* with rate, regardless of the fact that EAPOL frame is accepted or
	 * rejected by firmware
	 */
	WIFI_EVENT_FW_EAPOL_FRAME_RECEIVED,
	WIFI_EVENT_UNUSED_1,
	/* with rate, and eapol index, driver has received */
	/* EAPOL frame and will queue it up to wpa_supplicant */
	WIFI_EVENT_DRIVER_EAPOL_FRAME_RECEIVED,
	/* with success/failure, parameters */
	WIFI_EVENT_BLOCK_ACK_NEGOTIATION_COMPLETE,
	WIFI_EVENT_BT_COEX_BT_SCO_START,
	WIFI_EVENT_BT_COEX_BT_SCO_STOP,
	/* for paging/scan etc..., when BT starts transmiting twice per BT slot */
	WIFI_EVENT_BT_COEX_BT_SCAN_START,
	WIFI_EVENT_BT_COEX_BT_SCAN_STOP,
	WIFI_EVENT_BT_COEX_BT_HID_START,
	WIFI_EVENT_BT_COEX_BT_HID_STOP,
	/* firmware sends auth frame in roaming to next candidate */
	WIFI_EVENT_ROAM_AUTH_STARTED,
	/* firmware receive auth confirm from ap */
	WIFI_EVENT_ROAM_AUTH_COMPLETE,
	/* firmware sends assoc/reassoc frame in */
	WIFI_EVENT_ROAM_ASSOC_STARTED,
	/* firmware receive assoc/reassoc confirm from ap */
	WIFI_EVENT_ROAM_ASSOC_COMPLETE,
	/* firmware sends stop G_SCAN */
	WIFI_EVENT_G_SCAN_STOP,
	/* firmware indicates G_SCAN scan cycle started */
	WIFI_EVENT_G_SCAN_CYCLE_STARTED,
	/* firmware indicates G_SCAN scan cycle completed */
	WIFI_EVENT_G_SCAN_CYCLE_COMPLETED,
	/* firmware indicates G_SCAN scan start for a particular bucket */
	WIFI_EVENT_G_SCAN_BUCKET_STARTED,
	/* firmware indicates G_SCAN scan completed for particular bucket */
	WIFI_EVENT_G_SCAN_BUCKET_COMPLETED,
	/* Event received from firmware about G_SCAN scan results being available */
	WIFI_EVENT_G_SCAN_RESULTS_AVAILABLE,
	/* Event received from firmware with G_SCAN capabilities */
	WIFI_EVENT_G_SCAN_CAPABILITIES,
	/* Event received from firmware when eligible candidate is found */
	WIFI_EVENT_ROAM_CANDIDATE_FOUND,
	/* Event received from firmware when roam scan configuration gets enabled or disabled */
	WIFI_EVENT_ROAM_SCAN_CONFIG,
	/* firmware/driver timed out authentication */
	WIFI_EVENT_AUTH_TIMEOUT,
	/* firmware/driver timed out association */
	WIFI_EVENT_ASSOC_TIMEOUT,
	/* firmware/driver encountered allocation failure */
	WIFI_EVENT_MEM_ALLOC_FAILURE
};

enum {
	WIFI_TAG_VENDOR_SPECIFIC = 0, /* take a byte stream as parameter */
	WIFI_TAG_BSSID, /* takes a 6 bytes MAC address as parameter */
	WIFI_TAG_ADDR, /* takes a 6 bytes MAC address as parameter */
	WIFI_TAG_SSID, /* takes a 32 bytes SSID address as parameter */
	WIFI_TAG_STATUS, /* takes an integer as parameter */
	WIFI_TAG_CHANNEL_SPEC, /* takes one or more wifi_channel_spec as parameter */
	WIFI_TAG_WAKE_LOCK_EVENT, /* takes a wake_lock_event struct as parameter */
	WIFI_TAG_ADDR1, /* takes a 6 bytes MAC address as parameter */
	WIFI_TAG_ADDR2, /* takes a 6 bytes MAC address as parameter */
	WIFI_TAG_ADDR3, /* takes a 6 bytes MAC address as parameter */
	WIFI_TAG_ADDR4, /* takes a 6 bytes MAC address as parameter */
	WIFI_TAG_TSF, /* take a 64 bits TSF value as parameter */
	WIFI_TAG_IE,
	/* take one or more specific 802.11 IEs parameter, IEs are in turn
	 * indicated in TLV format as per 802.11 spec
	 */
	WIFI_TAG_INTERFACE,	/* take interface name as parameter */
	WIFI_TAG_REASON_CODE,	/* take a reason code as per 802.11 as parameter */
	WIFI_TAG_RATE_MBPS,	/* take a wifi rate in 0.5 mbps */
	WIFI_TAG_REQUEST_ID,	/* take an integer as parameter */
	WIFI_TAG_BUCKET_ID,	/* take an integer as parameter */
	WIFI_TAG_SCAN_ID,	/* take an integer as parameter */
	WIFI_TAG_RSSI,		/* takes s16 as parameter */
	WIFI_TAG_CHANNEL,	/* takes u16 as parameter */
	WIFI_TAG_LINK_ID,	/* take an integer as parameter */
	WIFI_TAG_LINK_ROLE,	/* take an integer as parameter */
	WIFI_TAG_LINK_STATE,	/* take an integer as parameter */
	WIFI_TAG_LINK_TYPE,	/* take an integer as parameter */
	WIFI_TAG_TSCO,		/* take an integer as parameter */
	WIFI_TAG_RSCO,		/* take an integer as parameter */
	WIFI_TAG_EAPOL_MESSAGE_TYPE /* take an integer as parameter */
};

typedef struct {
    uint16 tag;
    uint16 len; /* length of value */
    uint8 value[0];
} tlv_log;

typedef struct per_packet_status_entry {
    uint8 flags;
    uint8 tid; /* transmit or received tid */
    uint16 MCS; /* modulation and bandwidth */
	/*
	* TX: RSSI of ACK for that packet
	* RX: RSSI of packet
	*/
    uint8 rssi;
    uint8 num_retries; /* number of attempted retries */
    uint16 last_transmit_rate; /* last transmit rate in .5 mbps */
	 /* transmit/reeive sequence for that MPDU packet */
    uint16 link_layer_transmit_sequence;
	/*
	* TX: firmware timestamp (us) when packet is queued within firmware buffer
	* for SDIO/HSIC or into PCIe buffer
	* RX : firmware receive timestamp
	*/
    uint64 firmware_entry_timestamp;
	/*
	* firmware timestamp (us) when packet start contending for the
	* medium for the first time, at head of its AC queue,
	* or as part of an MPDU or A-MPDU. This timestamp is not updated
	* for each retry, only the first transmit attempt.
	*/
    uint64 start_contention_timestamp;
	/*
	* fimrware timestamp (us) when packet is successfully transmitted
	* or aborted because it has exhausted its maximum number of retries
	*/
	uint64 transmit_success_timestamp;
	/*
	* packet data. The length of packet data is determined by the entry_size field of
	* the wifi_ring_buffer_entry structure. It is expected that first bytes of the
	* packet, or packet headers only (up to TCP or RTP/UDP headers) will be copied into the ring
	*/
    uint8 *data;
} per_packet_status_entry_t;

#if defined(__linux__)
#define PACKED_STRUCT __attribute__ ((packed))
#else
#define PACKED_STRUCT
#endif

#if defined(__linux__)
typedef struct log_conn_event {
    uint16 event;
    tlv_log tlvs[0];
	/*
	* separate parameter structure per event to be provided and optional data
	* the event_data is expected to include an official android part, with some
	* parameter as transmit rate, num retries, num scan result found etc...
	* as well, event_data can include a vendor proprietary part which is
	* understood by the developer only.
	*/
} PACKED_STRUCT log_conn_event_t;
#endif /* defined(__linux__) */

/*
 * Ring buffer name for power events ring. note that power event are extremely frequents
 * and thus should be stored in their own ring/file so as not to clobber connectivity events
 */

typedef struct wake_lock_event {
    uint32 status; /* 0 taken, 1 released */
    uint32 reason; /* reason why this wake lock is taken */
    char *name; /* null terminated */
} wake_lock_event_t;

typedef struct wifi_power_event {
    uint16 event;
    tlv_log *tlvs;
} wifi_power_event_t;

struct log_level_table {
	int log_level;
	uint16 tag;
	char *desc;
};

#ifdef SHOW_LOGTRACE
/* message levels */
#define DHD_ERROR_VAL	0x0001
#define DHD_TRACE_VAL	0x0002
#define DHD_INFO_VAL	0x0004
#define DHD_DATA_VAL	0x0008
#define DHD_CTL_VAL	0x0010
#define DHD_TIMER_VAL	0x0020
#define DHD_HDRS_VAL	0x0040
#define DHD_BYTES_VAL	0x0080
#define DHD_INTR_VAL	0x0100
#define DHD_LOG_VAL	0x0200
#define DHD_GLOM_VAL	0x0400
#define DHD_EVENT_VAL	0x0800
#define DHD_BTA_VAL	0x1000
#define DHD_ISCAN_VAL	0x2000
#define DHD_ARPOE_VAL	0x4000
#define DHD_REORDER_VAL	0x8000
#define DHD_WL_VAL	0x10000
#define DHD_NOCHECKDIED_VAL	0x20000 /* UTF WAR */
#define DHD_WL_VAL2	0x40000
#define DHD_RTT_VAL	0x100000
#define DHD_MSGTRACE_VAL	0x200000
#define DHD_FWLOG_VAL	0x400000
#define DHD_DBGIF_VAL	0x800000
#define DHD_PKT_MON_VAL	0x2000000
#define DHD_PKT_PCAP_VAL	0x2000000
#define DHD_PKT_MON_DUMP_VAL	0x4000000
#define DHD_DNGL_IOVAR_SET_VAL	0x10000000 /**< logs the setting of dongle iovars */

#define DHD_PKT_MON_ON()	(dhd_msg_level & DHD_PKT_MON_VAL)
#define DHD_PKT_MON_DUMP_ON()	(dhd_msg_level & DHD_PKT_MON_DUMP_VAL)

#ifdef DHD_LOG_DUMP
/* LOG_DUMP defines common to EFI and NON-EFI */
/* NON-EFI builds with LOG DUMP enabled */
/* Re-using 'DHD_MSGTRACE_VAL' for controlling printing of ecounter binary event
* logs to console and debug dump -- need to cleanup in the future to use separate
* 'DHD_ECNTR_VAL' bitmap flag. 'DHD_MSGTRACE_VAL' will be defined only
* for non-android builds.
*/

#else /* DHD_LOG_DUMP */
/* !DHD_LOG_DUMP */
#define DHD_ECNTR_LOG(args)	DHD_EVENT(args)
#define DHD_PRSRV_MEM(args)	DHD_EVENT(args)
#define DHD_PKT_MON(args)	DHD_ERROR(args)
#endif /* DHD_LOG_DUMP */
#endif /* SHOW_LOGTRACE */

/*
 * Packet logging - HAL specific data
 * These should be moved to wl_cfgvendor.h
 */

#define MD5_PREFIX_LEN				4
#define MAX_FATE_LOG_LEN			32
#define MAX_FRAME_LEN_ETHERNET		1518
#define MAX_FRAME_LEN_80211_MGMT	2352 /* 802.11-2012 Fig. 8-34 */

typedef enum {
	/* Sent over air and ACKed. */
	TX_PKT_FATE_ACKED,

	/* Sent over air but not ACKed. (Normal for broadcast/multicast.) */
	TX_PKT_FATE_SENT,

	/* Queued within firmware, but not yet sent over air. */
	TX_PKT_FATE_FW_QUEUED,

	/*
	 * Dropped by firmware as invalid. E.g. bad source address,
	 * bad checksum, or invalid for current state.
	 */
	TX_PKT_FATE_FW_DROP_INVALID,

	/* Dropped by firmware due to lifetime expiration. */
	TX_PKT_FATE_FW_DROP_EXPTIME,

	/*
	 * Dropped by firmware for any other reason. Includes
	 * frames that were sent by driver to firmware, but
	 * unaccounted for by firmware.
	 */
	TX_PKT_FATE_FW_DROP_OTHER,

	/* Queued within driver, not yet sent to firmware. */
	TX_PKT_FATE_DRV_QUEUED,

	/*
	 * Dropped by driver as invalid. E.g. bad source address,
	 * or invalid for current state.
	 */
	TX_PKT_FATE_DRV_DROP_INVALID,

	/* Dropped by driver due to lack of buffer space. */
	TX_PKT_FATE_DRV_DROP_NOBUFS,

	/*  Dropped by driver for any other reason. */
	TX_PKT_FATE_DRV_DROP_OTHER,

	/* Packet free by firmware. */
	TX_PKT_FATE_FW_PKT_FREE,

	/* Firmware dropped the frame after suppress retries reached max */
	TX_PKT_FATE_FW_MAX_SUP_RETR,

	/* Firmware forced packet lifetime expiry */
	TX_PKT_FATE_FW_FORCED_EXPIRED,

#ifdef DHD_PKT_LOGGING_DBGRING
	/* Indicate to wait for updating txfate. */
	TX_PKT_FATE_DRV_WAIT_UPDATE = 0x80000000,
#endif /* DHD_PKT_LOGGING_DBGRING */
	} wifi_tx_packet_fate;

typedef enum {
	/* Valid and delivered to network stack (e.g., netif_rx()). */
	RX_PKT_FATE_SUCCESS,

	/* Queued within firmware, but not yet sent to driver. */
	RX_PKT_FATE_FW_QUEUED,

	/* Dropped by firmware due to host-programmable filters. */
	RX_PKT_FATE_FW_DROP_FILTER,

	/*
	 * Dropped by firmware as invalid. E.g. bad checksum,
	 * decrypt failed, or invalid for current state.
	 */
	RX_PKT_FATE_FW_DROP_INVALID,

	/* Dropped by firmware due to lack of buffer space. */
	RX_PKT_FATE_FW_DROP_NOBUFS,

	/* Dropped by firmware for any other reason. */
	RX_PKT_FATE_FW_DROP_OTHER,

	/* Queued within driver, not yet delivered to network stack. */
	RX_PKT_FATE_DRV_QUEUED,

	/* Dropped by driver due to filter rules. */
	RX_PKT_FATE_DRV_DROP_FILTER,

	/* Dropped by driver as invalid. E.g. not permitted in current state. */
	RX_PKT_FATE_DRV_DROP_INVALID,

	/* Dropped by driver due to lack of buffer space. */
	RX_PKT_FATE_DRV_DROP_NOBUFS,

	/* Dropped by driver for any other reason. */
	RX_PKT_FATE_DRV_DROP_OTHER,

	/* Indicate RX Host Wake up packet. */
	RX_PKT_FATE_WAKE_PKT,

	} wifi_rx_packet_fate;

typedef enum {
	FRAME_TYPE_UNKNOWN,
	FRAME_TYPE_ETHERNET_II,
	FRAME_TYPE_80211_MGMT,
	} frame_type;

typedef struct wifi_frame_info {
	/*
	 * The type of MAC-layer frame that this frame_info holds.
	 * - For data frames, use FRAME_TYPE_ETHERNET_II.
	 * - For management frames, use FRAME_TYPE_80211_MGMT.
	 * - If the type of the frame is unknown, use FRAME_TYPE_UNKNOWN.
	 */
	frame_type payload_type;

	/*
	 * The number of bytes included in |frame_content|. If the frame
	 * contents are missing (e.g. RX frame dropped in firmware),
	 * |frame_len| should be set to 0.
	 */
	size_t frame_len;

	/*
	 * Host clock when this frame was received by the driver (either
	 *	outbound from the host network stack, or inbound from the
	 *	firmware).
	 *	- The timestamp should be taken from a clock which includes time
	 *	  the host spent suspended (e.g. ktime_get_boottime()).
	 *	- If no host timestamp is available (e.g. RX frame was dropped in
	 *	  firmware), this field should be set to 0.
	 */
	uint32 driver_timestamp_usec;

	/*
	 * Firmware clock when this frame was received by the firmware
	 *	(either outbound from the host, or inbound from a remote
	 *	station).
	 *	- The timestamp should be taken from a clock which includes time
	 *	  firmware spent suspended (if applicable).
	 *	- If no firmware timestamp is available (e.g. TX frame was
	 *	  dropped by driver), this field should be set to 0.
	 *	- Consumers of |frame_info| should _not_ assume any
	 *	  synchronization between driver and firmware clocks.
	 */
	uint32 firmware_timestamp_usec;

	/*
	 * Actual frame content.
	 * - Should be provided for TX frames originated by the host.
	 * - Should be provided for RX frames received by the driver.
	 * - Optionally provided for TX frames originated by firmware. (At
	 *   discretion of HAL implementation.)
	 * - Optionally provided for RX frames dropped in firmware. (At
	 *   discretion of HAL implementation.)
	 * - If frame content is not provided, |frame_len| should be set
	 *   to 0.
	 */
	union {
		char ethernet_ii[MAX_FRAME_LEN_ETHERNET];
		char ieee_80211_mgmt[MAX_FRAME_LEN_80211_MGMT];
	} frame_content;
} wifi_frame_info_t;

typedef struct wifi_tx_report {
	/*
	 * Prefix of MD5 hash of |frame_inf.frame_content|. If frame
	 * content is not provided, prefix of MD5 hash over the same data
	 * that would be in frame_content, if frame content were provided.
	 */
	char md5_prefix[MD5_PREFIX_LEN];
	wifi_tx_packet_fate fate;
	wifi_frame_info_t frame_inf;
} wifi_tx_report_t;

typedef struct wifi_rx_report {
	/*
	 * Prefix of MD5 hash of |frame_inf.frame_content|. If frame
	 * content is not provided, prefix of MD5 hash over the same data
	 * that would be in frame_content, if frame content were provided.
	 */
	char md5_prefix[MD5_PREFIX_LEN];
	wifi_rx_packet_fate fate;
	wifi_frame_info_t frame_inf;
} wifi_rx_report_t;

typedef struct compat_wifi_frame_info {
	frame_type payload_type;

	uint32 frame_len;

	uint32 driver_timestamp_usec;

	uint32 firmware_timestamp_usec;

	union {
		char ethernet_ii[MAX_FRAME_LEN_ETHERNET];
		char ieee_80211_mgmt[MAX_FRAME_LEN_80211_MGMT];
	} frame_content;
} compat_wifi_frame_info_t;

typedef struct compat_wifi_tx_report {
	char md5_prefix[MD5_PREFIX_LEN];
	wifi_tx_packet_fate fate;
	compat_wifi_frame_info_t frame_inf;
} compat_wifi_tx_report_t;

typedef struct compat_wifi_rx_report {
	char md5_prefix[MD5_PREFIX_LEN];
	wifi_rx_packet_fate fate;
	compat_wifi_frame_info_t frame_inf;
} compat_wifi_rx_report_t;

/*
 * Packet logging - internal data
 */

typedef enum dhd_dbg_pkt_mon_state {
	PKT_MON_INVALID = 0,
	PKT_MON_ATTACHED,
	PKT_MON_STARTING,
	PKT_MON_STARTED,
	PKT_MON_STOPPING,
	PKT_MON_STOPPED,
	PKT_MON_DETACHED,
	} dhd_dbg_pkt_mon_state_t;

typedef struct dhd_dbg_pkt_info {
	frame_type payload_type;
	size_t pkt_len;
	uint32 driver_ts;
	uint32 firmware_ts;
	uint32 pkt_hash;
	void *pkt;
} dhd_dbg_pkt_info_t;

typedef struct compat_dhd_dbg_pkt_info {
	frame_type payload_type;
	uint32 pkt_len;
	uint32 driver_ts;
	uint32 firmware_ts;
	uint32 pkt_hash;
	void *pkt;
} compat_dhd_dbg_pkt_info_t;

typedef struct dhd_dbg_tx_info
{
	wifi_tx_packet_fate fate;
	dhd_dbg_pkt_info_t info;
} dhd_dbg_tx_info_t;

typedef struct dhd_dbg_rx_info
{
	wifi_rx_packet_fate fate;
	dhd_dbg_pkt_info_t info;
} dhd_dbg_rx_info_t;

typedef struct dhd_dbg_tx_report
{
	dhd_dbg_tx_info_t *tx_pkts;
	/* Indicates how many packets queued to send over the air */
	uint16 pkt_pos;
	/* Indicates how many packets sent over the air and received txstatus */
	uint16 status_pos;
} dhd_dbg_tx_report_t;

typedef struct dhd_dbg_rx_report
{
	dhd_dbg_rx_info_t *rx_pkts;
	/* Indicates how many packets sent over the air and received txstatus */
	uint16 pkt_pos;
} dhd_dbg_rx_report_t;

typedef int (*dbg_mon_tx_pkts_t) (dhd_pub_t *dhdp, void *pkt, uint32 pktid,
	frame_type type, uint8 mgmt_acked);
typedef int (*dbg_mon_tx_status_t) (dhd_pub_t *dhdp, void *pkt,
	uint32 pktid, uint16 status);
typedef int (*dbg_mon_rx_pkts_t) (dhd_pub_t *dhdp, void *pkt, frame_type type);

typedef struct dhd_dbg_pkt_mon
{
	dhd_dbg_tx_report_t *tx_report;
	dhd_dbg_rx_report_t *rx_report;
	dhd_dbg_pkt_mon_state_t tx_pkt_state;
	dhd_dbg_pkt_mon_state_t tx_status_state;
	dhd_dbg_pkt_mon_state_t rx_pkt_state;

	/* call backs */
	dbg_mon_tx_pkts_t tx_pkt_mon;
	dbg_mon_tx_status_t tx_status_mon;
	dbg_mon_rx_pkts_t rx_pkt_mon;
} dhd_dbg_pkt_mon_t;

#define PKT_MON_ATTACHED(state) \
		(((state) > PKT_MON_INVALID) && ((state) < PKT_MON_DETACHED))
#define PKT_MON_DETACHED(state) \
		(((state) == PKT_MON_INVALID) || ((state) == PKT_MON_DETACHED))
#define PKT_MON_STARTED(state) ((state) == PKT_MON_STARTED)
#define PKT_MON_STOPPED(state) ((state) == PKT_MON_STOPPED)
#define PKT_MON_NOT_OPERATIONAL(state) \
	(((state) != PKT_MON_STARTED) && ((state) != PKT_MON_STOPPED))
#define PKT_MON_SAFE_TO_FREE(state) \
	(((state) == PKT_MON_STARTING) || ((state) == PKT_MON_STOPPED))
#define PKT_MON_PKT_FULL(pkt_count) ((pkt_count) >= MAX_FATE_LOG_LEN)
#define PKT_MON_STATUS_FULL(pkt_count, status_count) \
		(((status_count) >= (pkt_count)) || ((status_count) >= MAX_FATE_LOG_LEN))

#ifdef DBG_PKT_MON
#define DHD_DBG_PKT_MON_TX(dhdp, pkt, pktid, type, mgmt_acked) \
	do { \
		if ((dhdp) && (dhdp)->dbg && (dhdp)->dbg->pkt_mon.tx_pkt_mon && (pkt)) { \
			(dhdp)->dbg->pkt_mon.tx_pkt_mon((dhdp), (pkt), \
			(pktid), (type), (mgmt_acked)); \
		} \
	} while (0);
#define DHD_DBG_PKT_MON_TX_STATUS(dhdp, pkt, pktid, status) \
	do { \
		if ((dhdp) && (dhdp)->dbg && (dhdp)->dbg->pkt_mon.tx_status_mon && (pkt)) { \
			(dhdp)->dbg->pkt_mon.tx_status_mon((dhdp), (pkt), (pktid), (status)); \
		} \
	} while (0);
#define DHD_DBG_PKT_MON_RX(dhdp, pkt, type) \
	do { \
		if ((dhdp) && (dhdp)->dbg && (dhdp)->dbg->pkt_mon.rx_pkt_mon && (pkt)) { \
			if (ntoh16((pkt)->protocol) != ETHER_TYPE_BRCM) { \
				(dhdp)->dbg->pkt_mon.rx_pkt_mon((dhdp), (pkt), (type)); \
			} \
		} \
	} while (0);

#define DHD_DBG_PKT_MON_START(dhdp) \
		dhd_os_dbg_start_pkt_monitor((dhdp));
#define DHD_DBG_PKT_MON_STOP(dhdp) \
		dhd_os_dbg_stop_pkt_monitor((dhdp));
#else
#define DHD_DBG_PKT_MON_TX(dhdp, pkt, pktid, type, mgmt_acked)
#define DHD_DBG_PKT_MON_TX_STATUS(dhdp, pkt, pktid, status)
#define DHD_DBG_PKT_MON_RX(dhdp, pkt, type)
#define DHD_DBG_PKT_MON_START(dhdp)
#define DHD_DBG_PKT_MON_STOP(dhdp)
#endif /* DBG_PKT_MON */

#ifdef DUMP_IOCTL_IOV_LIST
typedef struct dhd_iov_li {
	dll_t list;
	uint32 cmd; /* command number */
	char buff[100]; /* command name */
} dhd_iov_li_t;
#endif /* DUMP_IOCTL_IOV_LIST */

#define IOV_LIST_MAX_LEN 5

#ifdef DHD_DEBUG
typedef struct {
	dll_t list;
	uint32 id; /* wasted chunk id */
	uint32 handle; /* wasted chunk handle */
	uint32 size; /* wasted chunk size */
} dhd_dbg_mwli_t;
#endif /* DHD_DEBUG */

#define DHD_OW_BI_RAW_EVENT_LOG_FMT 0xFFFF

/* LSB 2 bits of format number to identify the type of event log */
#define DHD_EVENT_LOG_HDR_MASK 0x3

#define DHD_EVENT_LOG_FMT_NUM_OFFSET 2
#define DHD_EVENT_LOG_FMT_NUM_MASK 0x3FFF

#define DHD_EXT_FMTNUM_MASK 0xF /* bits[3..0] used */
/**
 * OW:- one word
 * TW:- two word
 * NB:- non binary
 * BI:- binary
 */
#define	DHD_OW_NB_EVENT_LOG_HDR 0
#define DHD_TW_NB_EVENT_LOG_HDR 1
#define DHD_BI_EVENT_LOG_HDR 3
#define DHD_INVALID_EVENT_LOG_HDR 2

#define DHD_TW_VALID_TAG_BITS_MASK 0xF
#define DHD_OW_BI_EVENT_FMT_NUM 0x3FFF
#define DHD_TW_BI_EVENT_FMT_NUM 0x3FFE

#define DHD_TW_EVENT_LOG_TAG_OFFSET 8

#define EVENT_TAG_TIMESTAMP_OFFSET 1
#define EVENT_TAG_TIMESTAMP_EXT_OFFSET 2

/* eldbg_cmn functions */
extern void dhd_dbg_trace_evnt_handler(dhd_pub_t *dhdp, void *event_data, uint datalen);
#ifdef SHOW_LOGTRACE
uint32 dhd_dbg_event_log_block_parser(void *dbg_hdl, char *data,
	uint32 datalen, ring_parser_info_t *el_ring_parser);
void dhd_dbg_msgtrace_log_parser(dhd_pub_t *dhdp, void *event_data,
	uint datalen, bool msgtrace_hdr_present, uint32 msgtrace_seqnum);
#endif /* SHOW_LOGTRACE */

extern int dhd_dbg_start(dhd_pub_t *dhdp, bool start);
extern int dhd_dbg_set_configuration(dhd_pub_t *dhdp, int ring_id,
		int log_level, int flags, uint32 threshold);
extern int dhd_dbg_find_ring_id(dhd_pub_t *dhdp, char *ring_name);
extern void *dhd_dbg_get_priv(dhd_pub_t *dhdp);
extern int dhd_dbg_send_urgent_evt(dhd_pub_t *dhdp, const void *data, const uint32 len);
extern uint32 dhd_dbg_verboselog_printf(eldbg_cmn_t *dbg, prcd_event_log_hdr_t *plog_hdr,
	uint32 *log_ptr, uint32 logset, char *buf_to_write, uint32 buflen);
int dhd_dbg_pull_from_ring(dhd_pub_t *dhdp, int ring_id, void *data, uint32 buf_len);
int dhd_dbg_pull_single_from_ring(dhd_pub_t *dhdp, int ring_id, void *data, uint32 buf_len,
	bool strip_header);
#ifdef DHD_PKT_LOGGING_DBGRING
int dhd_dbg_pull_from_pktlog(dhd_pub_t *dhdp, int ring_id, void *data, uint32 buf_len);
void dhd_os_dbg_urgent_pullreq(void *os_priv, int ring_id);
#endif /* DHD_PKT_LOGGING_DBGRING */
#ifdef DHD_DEBUGABILITY_DEBUG_DUMP
int dhd_debug_dump_ring_push(dhd_pub_t *dhdp, int ring_id, uint32 len, void *data);
#endif /* DHD_DEBUGABILITY_DEBUG_DUMP */
int dhd_dbg_get_ring_status(dhd_pub_t *dhdp, int ring_id,
		eldbg_ring_status_t *eldbg_ring_status);

#ifdef DBG_PKT_MON
extern int dhd_dbg_attach_pkt_monitor(dhd_pub_t *dhdp,
		dbg_mon_tx_pkts_t tx_pkt_mon,
		dbg_mon_tx_status_t tx_status_mon,
		dbg_mon_rx_pkts_t rx_pkt_mon);
extern int dhd_dbg_start_pkt_monitor(dhd_pub_t *dhdp);
extern int dhd_dbg_monitor_tx_pkts(dhd_pub_t *dhdp, void *pkt,
		uint32 pktid, frame_type type, uint8 mgmt_acked);
extern int dhd_dbg_monitor_tx_status(dhd_pub_t *dhdp, void *pkt,
		uint32 pktid, uint16 status);
extern int dhd_dbg_monitor_rx_pkts(dhd_pub_t *dhdp, void *pkt, frame_type type);
extern int dhd_dbg_stop_pkt_monitor(dhd_pub_t *dhdp);
extern int dhd_dbg_monitor_get_tx_pkts(dhd_pub_t *dhdp, void __user *user_buf,
		uint16 req_count, uint16 *resp_count);
extern int dhd_dbg_monitor_get_rx_pkts(dhd_pub_t *dhdp, void __user *user_buf,
		uint16 req_count, uint16 *resp_count);
extern int dhd_dbg_detach_pkt_monitor(dhd_pub_t *dhdp);
#endif /* DBG_PKT_MON */

extern bool dhd_dbg_process_tx_status(dhd_pub_t *dhdp, void *pkt,
		uint32 pktid, uint16 status);

/* os wrapper function */
extern int dhd_os_dbg_attach(dhd_pub_t *dhdp);
extern void dhd_os_dbg_detach(dhd_pub_t *dhdp);
extern int dhd_os_dbg_register_callback(int ring_id,
	void (*dbg_ring_sub_cb)(void *ctx, const int ring_id, const void *data,
		const uint32 len, const eldbg_ring_status_t eldbg_ring_status));
extern int dhd_os_dbg_register_urgent_notifier(dhd_pub_t *dhdp,
	void (*urgent_noti)(void *ctx, const void *data, const uint32 len, const uint32 fw_len));

extern int dhd_os_start_logging(dhd_pub_t *dhdp, char *ring_name, int log_level,
		int flags, int time_intval, int threshold);
extern int dhd_os_reset_logging(dhd_pub_t *dhdp);
extern int dhd_os_suppress_logging(dhd_pub_t *dhdp, bool suppress);

extern int dhd_os_get_ring_status(dhd_pub_t *dhdp, int ring_id,
		eldbg_ring_status_t *eldbg_ring_status);
extern int dhd_os_trigger_get_ring_data(dhd_pub_t *dhdp, char *ring_name);
extern int dhd_os_push_push_ring_data(dhd_pub_t *dhdp, int ring_id, void *data, int32 data_len);
extern int dhd_os_dbg_get_feature(dhd_pub_t *dhdp, int32 *features);

#ifdef DBG_PKT_MON
extern int dhd_os_dbg_attach_pkt_monitor(dhd_pub_t *dhdp);
extern int dhd_os_dbg_start_pkt_monitor(dhd_pub_t *dhdp);
extern int dhd_os_dbg_monitor_tx_pkts(dhd_pub_t *dhdp, void *pkt,
	uint32 pktid, frame_type type, uint8 mgmt_acked);
extern int dhd_os_dbg_monitor_tx_status(dhd_pub_t *dhdp, void *pkt,
	uint32 pktid, uint16 status);
extern int dhd_os_dbg_monitor_rx_pkts(dhd_pub_t *dhdp, void *pkt,
	frame_type type);
extern int dhd_os_dbg_stop_pkt_monitor(dhd_pub_t *dhdp);
extern int dhd_os_dbg_monitor_get_tx_pkts(dhd_pub_t *dhdp,
	void __user *user_buf, uint16 req_count, uint16 *resp_count);
extern int dhd_os_dbg_monitor_get_rx_pkts(dhd_pub_t *dhdp,
	void __user *user_buf, uint16 req_count, uint16 *resp_count);
extern int dhd_os_dbg_detach_pkt_monitor(dhd_pub_t *dhdp);
#endif /* DBG_PKT_MON */

#ifdef DUMP_IOCTL_IOV_LIST
extern void dhd_iov_li_append(dhd_pub_t *dhd, dll_t *list_head, dll_t *node);
extern void dhd_iov_li_print(dll_t *list_head);
extern void dhd_iov_li_delete(dhd_pub_t *dhd, dll_t *list_head);
#endif /* DUMP_IOCTL_IOV_LIST */

#ifdef DHD_DEBUG
extern void dhd_mw_list_delete(dhd_pub_t *dhd, dll_t *list_head);
#endif /* DHD_DEBUG */

typedef void (*print_roam_enhance_log_func)(prcd_event_log_hdr_t *plog_hdr);
typedef struct _pr_roam_tbl {
	uint8 version;
	uint8 id;
	print_roam_enhance_log_func pr_func;
} pr_roam_tbl_t;

int dhd_get_instance_base(void);
#endif /* _dhd_debug_h_ */
