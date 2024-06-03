/*
 * The Linux nl80211 HAL device driver header file
 *
 * Copyright (C) 2023, Broadcom. All Rights Reserved.
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
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: $
 */

#ifndef _rdk_nl80211_hal_h_
#define _rdk_nl80211_hal_h_

#define OUI_COMCAST 0xF0463B

/**
 * enum rdk_vendor_subcmd - supported nl80211 Comcast sub commands.
 *
 * @RDK_VENDOR_NL80211_SUBCMD_UNSPECIFIED: unspecified command to catch
 *	errors.
 *
 * @RDK_VENDOR_NL80211_SUBCMD_GET_STATION: Get station attributes for
 *	station	identified by %RDK_VENDOR_ATTR_MAC_ADDR. The command is
 *	extension of standard NL80211_CMD_GET_STATION with custom parameters.
 *
 * @RDK_VENDOR_NL80211_SUBCMD_GET_STATION_LIST: Returns station number
 *	%RDK_VENDOR_ATTR_STATION_NUM and MAC address list
 *	%RDK_VENDOR_ATTR_STATION_LIST.
 *
 */

enum rdk_vendor_subcmd {
	RDK_VENDOR_NL80211_SUBCMD_UNSPECIFIED,
	RDK_VENDOR_NL80211_SUBCMD_GET_STATION,
	RDK_VENDOR_NL80211_SUBCMD_GET_STATION_LIST,
};

/**
 * enum rdk_vendor_attr - Comcast nl80211 netlink attributes
 *
 * @RDK_VENDOR_ATTR_UNSPECIFIED: unspecified attribute to catch errors.
 *
 * @RDK_VENDOR_ATTR_PAD: attribute used for padding for 64-bit alignment.
 *
 * RDK_VENDOR_ATTR_MAC: MAC address.
 *
 * RDK_VENDOR_ATTR_STATION_NUM: number of stations known by the
 *	driver. (u32)
 *
 * RDK_VENDOR_ATTR_STATION_LIST: array of nested MAC addresses.
 *
 * RDK_VENDOR_ATTR_STA_INFO_STA_FLAGS: contains a
 *	&struct nl80211_sta_flag_update.
 *
 * RDK_VENDOR_ATTR_STA_INFO_RX_BITRATE_LAST: the median PHY rate of the most
 *	recent 16 unicast data frame transmissions from the associated device to
 *	the access point. (u32, kbps)
 *
 * RDK_VENDOR_ATTR_STA_INFO_TX_BITRATE_LAST: the median PHY rate of the most
 *	recent 16 unicast data frame transmissions from the access point to the
 *	associated device. (u32, kbps)
 *
 * RDK_VENDOR_ATTR_STA_INFO_SIGNAL_AVG: an indicator of radio signal
 *	strength of the uplink from the associated device to the access point,
 *	measured in dBm, as an average of the last 100 packets received from
 *	the device. (s32)
 *
 * RDK_VENDOR_ATTR_STA_INFO_TX_RETRIES_PERCENT: the number of packets that
 *	had to be re-transmitted, from the last 100 packets sent to the
 *	associated device. Multiple re-transmissions of the same packet count
 *	as one. (u32)
 *
 * RDK_VENDOR_ATTR_STA_INFO_ACTIVE: whether or not this station is currently
 *	present in the WiFi AccessPoint network. (u8)
 *
 * RDK_VENDOR_ATTR_STA_INFO_OPER_STANDARD: radio standard the associated
 *	station is operating under.
 *	See &enum rdk_vendor_nl80211_standard. (u8)
 *
 * RDK_VENDOR_ATTR_STA_INFO_OPER_CHANNEL_BW: the operating channel bandwidth
 *	of the associated station.
 *	See &enum rdk_vendor_nl80211_chan_width. (u8)
 *
 * RDK_VENDOR_ATTR_STA_INFO_SNR: A signal-to-noise ratio (SNR) compares the
 *	level of the Wi-Fi signal to the level of background noise. (s32)
 *
 * RDK_VENDOR_ATTR_STA_INFO_TX_PACKETS_ACK: indicates the total number of
 *	MSDU frames marked as duplicates and non duplicates acknowledged. (u64)
 *
 * RDK_VENDOR_ATTR_STA_INFO_TX_PACKETS_NO_ACK: indicates the total number
 *	of MSDU frames retransmitted out of the interface (i.e., marked as
 *	duplicate and non-duplicate) and not acknowledged, but exclude
 *	lost frames (%RDK_VENDOR_ATTR_STA_INFO_TX_FAILED_RETRIES). (u64)
 *
 * RDK_VENDOR_ATTR_STA_INFO_TX_BYTES64: the total number of bytes
 *	transmitted to the station.
 *
 * RDK_VENDOR_ATTR_STA_INFO_RX_BYTES64: the total number of bytes
 *	received from the station.
 *
 * RDK_VENDOR_ATTR_STA_INFO_SIGNAL_MIN: the minimum RSSI for last
 *	100 frames (s32).
 *
 * RDK_VENDOR_ATTR_STA_INFO_SIGNAL_MAX: the maximum RSSI for last
 *	100 frames (s32).
 *
 * RDK_VENDOR_ATTR_STA_INFO_DISASSOC_NUM: represents the total number of
 *	station disassociations. Resets every 24hrs or reboot. (u32)
 *
 * RDK_VENDOR_ATTR_STA_INFO_AUTH_FAILS: indicates the total number of
 *	authentication failures.  Resets every 24hrs or reboot. (u32)
 *
 * RDK_VENDOR_ATTR_STA_INFO_ASSOC_NUM: number of station associations. (u32)
 *
 * RDK_VENDOR_ATTR_STA_INFO_TX_PACKETS64: total number of packets
 *	transmitted to the station. (u64)
 *
 * RDK_VENDOR_ATTR_STA_INFO_RX_PACKETS64: total number of packets
 *	received from the station. (u64)
 *
 * RDK_VENDOR_ATTR_STA_INFO_TX_ERRORS: total number of outbound packets
 *	that could not be transmitted because of errors.
 *	This might be due to the number of retransmissions exceeding the retry
 *	limit or from other causes. (u64)
 *
 * RDK_VENDOR_ATTR_STA_INFO_TX_RETRANSMIT: total number of transmitted
 *	packets which were retransmissions. Two retransmissions of the same
 *	frame results in this counter incrementing by two. (u64)
 *
 * RDK_VENDOR_ATTR_STA_INFO_TX_FAILED_RETRIES: number of packets that
 *	were not transmitted successfully due to the number of retransmission
 *	attempts exceeding an 802.11 retry limit. (u64)
 *
 * RDK_VENDOR_ATTR_STA_INFO_TX_RETRIES: the number of packets that were
 *	successfully transmitted after one or more retransmissions. (u64)
 *
 * RDK_VENDOR_ATTR_STA_INFO_TX_MULT_RETRIES: the number of packets that
 *	were successfully transmitted after more than one retransmission. (u64)
 *
 * RDK_VENDOR_ATTR_STA_INFO_TX_RATE_MAX: the max data transmit rate from
 *	access point to station. (u32, kbps)
 *
 * RDK_VENDOR_ATTR_STA_INFO_RX_RATE_MAX: the max data receive rate from
 *	station to access point. (u32, kbps)
 *
 * RDK_VENDOR_ATTR_STA_INFO_SPATIAL_STREAM_NUM: the number of active
 *	spatial streams in the session between AP and station at the
 *	moment of polling. (u8)
 *
 * RDK_VENDOR_ATTR_STA_INFO_TX_FRAMES: the number of transmitted frames to
 *	station. (u64)
 *
 * RDK_VENDOR_ATTR_STA_INFO_RX_RETRIES: number of rx retries. (u64)
 *
 * RDK_VENDOR_ATTR_STA_INFO_RX_ERRORS: number of rx errors. (u64)
 *
 * RDK_VENDOR_ATTR_NUM: internal
 *
 * RDK_VENDOR_ATTR_MAX: highest possible attribute
 *
 */

enum rdk_vendor_attr {
	RDK_VENDOR_ATTR_UNSPECIFIED,
	RDK_VENDOR_ATTR_PAD,
	RDK_VENDOR_ATTR_MAC,
	RDK_VENDOR_ATTR_STATION_NUM,
	RDK_VENDOR_ATTR_STATION_LIST,
	RDK_VENDOR_ATTR_STA_INFO_STA_FLAGS,
	RDK_VENDOR_ATTR_STA_INFO_RX_BITRATE_LAST,
	RDK_VENDOR_ATTR_STA_INFO_TX_BITRATE_LAST,
	RDK_VENDOR_ATTR_STA_INFO_SIGNAL_AVG,
	RDK_VENDOR_ATTR_STA_INFO_TX_RETRIES_PERCENT,
	RDK_VENDOR_ATTR_STA_INFO_ACTIVE,
	RDK_VENDOR_ATTR_STA_INFO_OPER_STANDARD,
	RDK_VENDOR_ATTR_STA_INFO_OPER_CHANNEL_BW,
	RDK_VENDOR_ATTR_STA_INFO_SNR,
	RDK_VENDOR_ATTR_STA_INFO_TX_PACKETS_ACK,
	RDK_VENDOR_ATTR_STA_INFO_TX_PACKETS_NO_ACK,
	RDK_VENDOR_ATTR_STA_INFO_TX_BYTES64,
	RDK_VENDOR_ATTR_STA_INFO_RX_BYTES64,
	RDK_VENDOR_ATTR_STA_INFO_SIGNAL_MIN,
	RDK_VENDOR_ATTR_STA_INFO_SIGNAL_MAX,
	RDK_VENDOR_ATTR_STA_INFO_DISASSOC_NUM,
	RDK_VENDOR_ATTR_STA_INFO_AUTH_FAILS,
	RDK_VENDOR_ATTR_STA_INFO_ASSOC_NUM,
	RDK_VENDOR_ATTR_STA_INFO_TX_PACKETS64,
	RDK_VENDOR_ATTR_STA_INFO_RX_PACKETS64,
	RDK_VENDOR_ATTR_STA_INFO_TX_ERRORS,
	RDK_VENDOR_ATTR_STA_INFO_TX_RETRANSMIT,
	RDK_VENDOR_ATTR_STA_INFO_TX_FAILED_RETRIES,
	RDK_VENDOR_ATTR_STA_INFO_TX_RETRIES,
	RDK_VENDOR_ATTR_STA_INFO_TX_MULT_RETRIES,
	RDK_VENDOR_ATTR_STA_INFO_TX_RATE_MAX,
	RDK_VENDOR_ATTR_STA_INFO_RX_RATE_MAX,
	RDK_VENDOR_ATTR_STA_INFO_SPATIAL_STREAM_NUM,
	RDK_VENDOR_ATTR_STA_INFO_TX_FRAMES,
	RDK_VENDOR_ATTR_STA_INFO_RX_RETRIES,
	RDK_VENDOR_ATTR_STA_INFO_RX_ERRORS,

	/* keep last */
	RDK_VENDOR_ATTR_NUM,
	RDK_VENDOR_ATTR_MAX = RDK_VENDOR_ATTR_NUM - 1,
};

/**
 * enum rdk_vendor_nl80211_standard - Wi-Fi standard.
 */
enum rdk_vendor_nl80211_standard {
	RDK_VENDOR_NL80211_STANDARD_NONE = 0,
	RDK_VENDOR_NL80211_STANDARD_A    = 1 << 0,
	RDK_VENDOR_NL80211_STANDARD_B    = 1 << 1,
	RDK_VENDOR_NL80211_STANDARD_G    = 1 << 2,
	RDK_VENDOR_NL80211_STANDARD_N    = 1 << 3,
	RDK_VENDOR_NL80211_STANDARD_AC   = 1 << 4,
	RDK_VENDOR_NL80211_STANDARD_AD   = 1 << 5,
	RDK_VENDOR_NL80211_STANDARD_AX   = 1 << 6,
	RDK_VENDOR_NL80211_STANDARD_BE   = 1 << 7,
};

/**
 * enum nl80211_chan_width - channel width definitions
 */
enum rdk_vendor_nl80211_chan_width {
	RDK_VENDOR_NL80211_CHAN_WIDTH_20_NOHT,
	RDK_VENDOR_NL80211_CHAN_WIDTH_20,
	RDK_VENDOR_NL80211_CHAN_WIDTH_40,
	RDK_VENDOR_NL80211_CHAN_WIDTH_80,
	RDK_VENDOR_NL80211_CHAN_WIDTH_80P80,
	RDK_VENDOR_NL80211_CHAN_WIDTH_160,
	RDK_VENDOR_NL80211_CHAN_WIDTH_5,
	RDK_VENDOR_NL80211_CHAN_WIDTH_10,
	RDK_VENDOR_NL80211_CHAN_WIDTH_1,
	RDK_VENDOR_NL80211_CHAN_WIDTH_2,
	RDK_VENDOR_NL80211_CHAN_WIDTH_4,
	RDK_VENDOR_NL80211_CHAN_WIDTH_8,
	RDK_VENDOR_NL80211_CHAN_WIDTH_16,
	RDK_VENDOR_NL80211_CHAN_WIDTH_320,
};

#define BW_20MHZ		(WL_RSPEC_BW_20MHZ >> WL_RSPEC_BW_SHIFT)
#define BW_40MHZ		(WL_RSPEC_BW_40MHZ >> WL_RSPEC_BW_SHIFT)
#define BW_80MHZ		(WL_RSPEC_BW_80MHZ >> WL_RSPEC_BW_SHIFT)
#define BW_160MHZ		(WL_RSPEC_BW_160MHZ >> WL_RSPEC_BW_SHIFT)
#define BW_320MHZ		(WL_RSPEC_BW_320MHZ >> WL_RSPEC_BW_SHIFT)

#define STA_ASSOC_COUNT_FILE	"/tmp/sta_assoc_count"
#define STA_ASSOC_COUNT_BUF	8096

#define WL_RSPEC_GI_AUTO	0
#define HEGI_OFFSET		2	/* skip the legacy sgi */

#endif /* _rdk_nl80211_hal_h_ */
