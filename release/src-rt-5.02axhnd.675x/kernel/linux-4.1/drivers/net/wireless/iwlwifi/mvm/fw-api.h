/******************************************************************************
 *
 * This file is provided under a dual BSD/GPLv2 license.  When using or
 * redistributing this file, you may do so under either license.
 *
 * GPL LICENSE SUMMARY
 *
 * Copyright(c) 2012 - 2014 Intel Corporation. All rights reserved.
 * Copyright(c) 2013 - 2014 Intel Mobile Communications GmbH
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110,
 * USA
 *
 * The full GNU General Public License is included in this distribution
 * in the file called COPYING.
 *
 * Contact Information:
 *  Intel Linux Wireless <ilw@linux.intel.com>
 * Intel Corporation, 5200 N.E. Elam Young Parkway, Hillsboro, OR 97124-6497
 *
 * BSD LICENSE
 *
 * Copyright(c) 2012 - 2014 Intel Corporation. All rights reserved.
 * Copyright(c) 2013 - 2014 Intel Mobile Communications GmbH
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *  * Neither the name Intel Corporation nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *****************************************************************************/

#ifndef __fw_api_h__
#define __fw_api_h__

#include "fw-api-rs.h"
#include "fw-api-tx.h"
#include "fw-api-sta.h"
#include "fw-api-mac.h"
#include "fw-api-power.h"
#include "fw-api-d3.h"
#include "fw-api-coex.h"
#include "fw-api-scan.h"
#include "fw-api-stats.h"

/* Tx queue numbers */
enum {
	IWL_MVM_OFFCHANNEL_QUEUE = 8,
	IWL_MVM_CMD_QUEUE = 9,
};

enum iwl_mvm_tx_fifo {
	IWL_MVM_TX_FIFO_BK = 0,
	IWL_MVM_TX_FIFO_BE,
	IWL_MVM_TX_FIFO_VI,
	IWL_MVM_TX_FIFO_VO,
	IWL_MVM_TX_FIFO_MCAST = 5,
	IWL_MVM_TX_FIFO_CMD = 7,
};

#define IWL_MVM_STATION_COUNT	16

#define IWL_MVM_TDLS_STA_COUNT	4

/* commands */
enum {
	MVM_ALIVE = 0x1,
	REPLY_ERROR = 0x2,

	INIT_COMPLETE_NOTIF = 0x4,

	/* PHY context commands */
	PHY_CONTEXT_CMD = 0x8,
	DBG_CFG = 0x9,
	ANTENNA_COUPLING_NOTIFICATION = 0xa,

	/* UMAC scan commands */
	SCAN_CFG_CMD = 0xc,
	SCAN_REQ_UMAC = 0xd,
	SCAN_ABORT_UMAC = 0xe,
	SCAN_COMPLETE_UMAC = 0xf,

	/* station table */
	ADD_STA_KEY = 0x17,
	ADD_STA = 0x18,
	REMOVE_STA = 0x19,

	/* TX */
	TX_CMD = 0x1c,
	TXPATH_FLUSH = 0x1e,
	MGMT_MCAST_KEY = 0x1f,

	/* scheduler config */
	SCD_QUEUE_CFG = 0x1d,

	/* global key */
	WEP_KEY = 0x20,

	/* Memory */
	SHARED_MEM_CFG = 0x25,

	/* TDLS */
	TDLS_CHANNEL_SWITCH_CMD = 0x27,
	TDLS_CHANNEL_SWITCH_NOTIFICATION = 0xaa,
	TDLS_CONFIG_CMD = 0xa7,

	/* MAC and Binding commands */
	MAC_CONTEXT_CMD = 0x28,
	TIME_EVENT_CMD = 0x29, /* both CMD and response */
	TIME_EVENT_NOTIFICATION = 0x2a,
	BINDING_CONTEXT_CMD = 0x2b,
	TIME_QUOTA_CMD = 0x2c,
	NON_QOS_TX_COUNTER_CMD = 0x2d,

	LQ_CMD = 0x4e,

	/* Calibration */
	TEMPERATURE_NOTIFICATION = 0x62,
	CALIBRATION_CFG_CMD = 0x65,
	CALIBRATION_RES_NOTIFICATION = 0x66,
	CALIBRATION_COMPLETE_NOTIFICATION = 0x67,
	RADIO_VERSION_NOTIFICATION = 0x68,

	/* Scan offload */
	SCAN_OFFLOAD_REQUEST_CMD = 0x51,
	SCAN_OFFLOAD_ABORT_CMD = 0x52,
	HOT_SPOT_CMD = 0x53,
	SCAN_OFFLOAD_COMPLETE = 0x6D,
	SCAN_OFFLOAD_UPDATE_PROFILES_CMD = 0x6E,
	SCAN_OFFLOAD_CONFIG_CMD = 0x6f,
	MATCH_FOUND_NOTIFICATION = 0xd9,
	SCAN_ITERATION_COMPLETE = 0xe7,

	/* Phy */
	PHY_CONFIGURATION_CMD = 0x6a,
	CALIB_RES_NOTIF_PHY_DB = 0x6b,
	/* PHY_DB_CMD = 0x6c, */

	/* Power - legacy power table command */
	POWER_TABLE_CMD = 0x77,
	PSM_UAPSD_AP_MISBEHAVING_NOTIFICATION = 0x78,
	LTR_CONFIG = 0xee,

	/* Thermal Throttling*/
	REPLY_THERMAL_MNG_BACKOFF = 0x7e,

	/* Scanning */
	SCAN_REQUEST_CMD = 0x80,
	SCAN_ABORT_CMD = 0x81,
	SCAN_START_NOTIFICATION = 0x82,
	SCAN_RESULTS_NOTIFICATION = 0x83,
	SCAN_COMPLETE_NOTIFICATION = 0x84,

	/* NVM */
	NVM_ACCESS_CMD = 0x88,

	SET_CALIB_DEFAULT_CMD = 0x8e,

	BEACON_NOTIFICATION = 0x90,
	BEACON_TEMPLATE_CMD = 0x91,
	TX_ANT_CONFIGURATION_CMD = 0x98,
	STATISTICS_CMD = 0x9c,
	STATISTICS_NOTIFICATION = 0x9d,
	EOSP_NOTIFICATION = 0x9e,
	REDUCE_TX_POWER_CMD = 0x9f,

	/* RF-KILL commands and notifications */
	CARD_STATE_CMD = 0xa0,
	CARD_STATE_NOTIFICATION = 0xa1,

	MISSED_BEACONS_NOTIFICATION = 0xa2,

	/* Power - new power table command */
	MAC_PM_POWER_TABLE = 0xa9,

	MFUART_LOAD_NOTIFICATION = 0xb1,

	REPLY_RX_PHY_CMD = 0xc0,
	REPLY_RX_MPDU_CMD = 0xc1,
	BA_NOTIF = 0xc5,

	/* Location Aware Regulatory */
	MCC_UPDATE_CMD = 0xc8,
	MCC_CHUB_UPDATE_CMD = 0xc9,

	MARKER_CMD = 0xcb,

	/* BT Coex */
	BT_COEX_PRIO_TABLE = 0xcc,
	BT_COEX_PROT_ENV = 0xcd,
	BT_PROFILE_NOTIFICATION = 0xce,
	BT_CONFIG = 0x9b,
	BT_COEX_UPDATE_SW_BOOST = 0x5a,
	BT_COEX_UPDATE_CORUN_LUT = 0x5b,
	BT_COEX_UPDATE_REDUCED_TXP = 0x5c,
	BT_COEX_CI = 0x5d,

	REPLY_SF_CFG_CMD = 0xd1,
	REPLY_BEACON_FILTERING_CMD = 0xd2,

	/* DTS measurements */
	CMD_DTS_MEASUREMENT_TRIGGER = 0xdc,
	DTS_MEASUREMENT_NOTIFICATION = 0xdd,

	REPLY_DEBUG_CMD = 0xf0,
	DEBUG_LOG_MSG = 0xf7,

	BCAST_FILTER_CMD = 0xcf,
	MCAST_FILTER_CMD = 0xd0,

	/* D3 commands/notifications */
	D3_CONFIG_CMD = 0xd3,
	PROT_OFFLOAD_CONFIG_CMD = 0xd4,
	OFFLOADS_QUERY_CMD = 0xd5,
	REMOTE_WAKE_CONFIG_CMD = 0xd6,
	D0I3_END_CMD = 0xed,

	/* for WoWLAN in particular */
	WOWLAN_PATTERNS = 0xe0,
	WOWLAN_CONFIGURATION = 0xe1,
	WOWLAN_TSC_RSC_PARAM = 0xe2,
	WOWLAN_TKIP_PARAM = 0xe3,
	WOWLAN_KEK_KCK_MATERIAL = 0xe4,
	WOWLAN_GET_STATUSES = 0xe5,
	WOWLAN_TX_POWER_PER_DB = 0xe6,

	/* and for NetDetect */
	SCAN_OFFLOAD_PROFILES_QUERY_CMD = 0x56,
	SCAN_OFFLOAD_HOTSPOTS_CONFIG_CMD = 0x58,
	SCAN_OFFLOAD_HOTSPOTS_QUERY_CMD = 0x59,

	REPLY_MAX = 0xff,
};

/**
 * struct iwl_cmd_response - generic response struct for most commands
 * @status: status of the command asked, changes for each one
 */
struct iwl_cmd_response {
	__le32 status;
};

/*
 * struct iwl_tx_ant_cfg_cmd
 * @valid: valid antenna configuration
 */
struct iwl_tx_ant_cfg_cmd {
	__le32 valid;
} __packed;

/*
 * Calibration control struct.
 * Sent as part of the phy configuration command.
 * @flow_trigger: bitmap for which calibrations to perform according to
 *		flow triggers.
 * @event_trigger: bitmap for which calibrations to perform according to
 *		event triggers.
 */
struct iwl_calib_ctrl {
	__le32 flow_trigger;
	__le32 event_trigger;
} __packed;

/* This enum defines the bitmap of various calibrations to enable in both
 * init ucode and runtime ucode through CALIBRATION_CFG_CMD.
 */
enum iwl_calib_cfg {
	IWL_CALIB_CFG_XTAL_IDX			= BIT(0),
	IWL_CALIB_CFG_TEMPERATURE_IDX		= BIT(1),
	IWL_CALIB_CFG_VOLTAGE_READ_IDX		= BIT(2),
	IWL_CALIB_CFG_PAPD_IDX			= BIT(3),
	IWL_CALIB_CFG_TX_PWR_IDX		= BIT(4),
	IWL_CALIB_CFG_DC_IDX			= BIT(5),
	IWL_CALIB_CFG_BB_FILTER_IDX		= BIT(6),
	IWL_CALIB_CFG_LO_LEAKAGE_IDX		= BIT(7),
	IWL_CALIB_CFG_TX_IQ_IDX			= BIT(8),
	IWL_CALIB_CFG_TX_IQ_SKEW_IDX		= BIT(9),
	IWL_CALIB_CFG_RX_IQ_IDX			= BIT(10),
	IWL_CALIB_CFG_RX_IQ_SKEW_IDX		= BIT(11),
	IWL_CALIB_CFG_SENSITIVITY_IDX		= BIT(12),
	IWL_CALIB_CFG_CHAIN_NOISE_IDX		= BIT(13),
	IWL_CALIB_CFG_DISCONNECTED_ANT_IDX	= BIT(14),
	IWL_CALIB_CFG_ANT_COUPLING_IDX		= BIT(15),
	IWL_CALIB_CFG_DAC_IDX			= BIT(16),
	IWL_CALIB_CFG_ABS_IDX			= BIT(17),
	IWL_CALIB_CFG_AGC_IDX			= BIT(18),
};

/*
 * Phy configuration command.
 */
struct iwl_phy_cfg_cmd {
	__le32	phy_cfg;
	struct iwl_calib_ctrl calib_control;
} __packed;

#define PHY_CFG_RADIO_TYPE	(BIT(0) | BIT(1))
#define PHY_CFG_RADIO_STEP	(BIT(2) | BIT(3))
#define PHY_CFG_RADIO_DASH	(BIT(4) | BIT(5))
#define PHY_CFG_PRODUCT_NUMBER	(BIT(6) | BIT(7))
#define PHY_CFG_TX_CHAIN_A	BIT(8)
#define PHY_CFG_TX_CHAIN_B	BIT(9)
#define PHY_CFG_TX_CHAIN_C	BIT(10)
#define PHY_CFG_RX_CHAIN_A	BIT(12)
#define PHY_CFG_RX_CHAIN_B	BIT(13)
#define PHY_CFG_RX_CHAIN_C	BIT(14)


/* Target of the NVM_ACCESS_CMD */
enum {
	NVM_ACCESS_TARGET_CACHE = 0,
	NVM_ACCESS_TARGET_OTP = 1,
	NVM_ACCESS_TARGET_EEPROM = 2,
};

/* Section types for NVM_ACCESS_CMD */
enum {
	NVM_SECTION_TYPE_SW = 1,
	NVM_SECTION_TYPE_REGULATORY = 3,
	NVM_SECTION_TYPE_CALIBRATION = 4,
	NVM_SECTION_TYPE_PRODUCTION = 5,
	NVM_SECTION_TYPE_MAC_OVERRIDE = 11,
	NVM_SECTION_TYPE_PHY_SKU = 12,
	NVM_MAX_NUM_SECTIONS = 13,
};

/**
 * struct iwl_nvm_access_cmd_ver2 - Request the device to send an NVM section
 * @op_code: 0 - read, 1 - write
 * @target: NVM_ACCESS_TARGET_*
 * @type: NVM_SECTION_TYPE_*
 * @offset: offset in bytes into the section
 * @length: in bytes, to read/write
 * @data: if write operation, the data to write. On read its empty
 */
struct iwl_nvm_access_cmd {
	u8 op_code;
	u8 target;
	__le16 type;
	__le16 offset;
	__le16 length;
	u8 data[];
} __packed; /* NVM_ACCESS_CMD_API_S_VER_2 */

/**
 * struct iwl_nvm_access_resp_ver2 - response to NVM_ACCESS_CMD
 * @offset: offset in bytes into the section
 * @length: in bytes, either how much was written or read
 * @type: NVM_SECTION_TYPE_*
 * @status: 0 for success, fail otherwise
 * @data: if read operation, the data returned. Empty on write.
 */
struct iwl_nvm_access_resp {
	__le16 offset;
	__le16 length;
	__le16 type;
	__le16 status;
	u8 data[];
} __packed; /* NVM_ACCESS_CMD_RESP_API_S_VER_2 */

/* MVM_ALIVE 0x1 */

/* alive response is_valid values */
#define ALIVE_RESP_UCODE_OK	BIT(0)
#define ALIVE_RESP_RFKILL	BIT(1)

/* alive response ver_type values */
enum {
	FW_TYPE_HW = 0,
	FW_TYPE_PROT = 1,
	FW_TYPE_AP = 2,
	FW_TYPE_WOWLAN = 3,
	FW_TYPE_TIMING = 4,
	FW_TYPE_WIPAN = 5
};

/* alive response ver_subtype values */
enum {
	FW_SUBTYPE_FULL_FEATURE = 0,
	FW_SUBTYPE_BOOTSRAP = 1, /* Not valid */
	FW_SUBTYPE_REDUCED = 2,
	FW_SUBTYPE_ALIVE_ONLY = 3,
	FW_SUBTYPE_WOWLAN = 4,
	FW_SUBTYPE_AP_SUBTYPE = 5,
	FW_SUBTYPE_WIPAN = 6,
	FW_SUBTYPE_INITIALIZE = 9
};

#define IWL_ALIVE_STATUS_ERR 0xDEAD
#define IWL_ALIVE_STATUS_OK 0xCAFE

#define IWL_ALIVE_FLG_RFKILL	BIT(0)

struct mvm_alive_resp_ver1 {
	__le16 status;
	__le16 flags;
	u8 ucode_minor;
	u8 ucode_major;
	__le16 id;
	u8 api_minor;
	u8 api_major;
	u8 ver_subtype;
	u8 ver_type;
	u8 mac;
	u8 opt;
	__le16 reserved2;
	__le32 timestamp;
	__le32 error_event_table_ptr;	/* SRAM address for error log */
	__le32 log_event_table_ptr;	/* SRAM address for event log */
	__le32 cpu_register_ptr;
	__le32 dbgm_config_ptr;
	__le32 alive_counter_ptr;
	__le32 scd_base_ptr;		/* SRAM address for SCD */
} __packed; /* ALIVE_RES_API_S_VER_1 */

struct mvm_alive_resp_ver2 {
	__le16 status;
	__le16 flags;
	u8 ucode_minor;
	u8 ucode_major;
	__le16 id;
	u8 api_minor;
	u8 api_major;
	u8 ver_subtype;
	u8 ver_type;
	u8 mac;
	u8 opt;
	__le16 reserved2;
	__le32 timestamp;
	__le32 error_event_table_ptr;	/* SRAM address for error log */
	__le32 log_event_table_ptr;	/* SRAM address for LMAC event log */
	__le32 cpu_register_ptr;
	__le32 dbgm_config_ptr;
	__le32 alive_counter_ptr;
	__le32 scd_base_ptr;		/* SRAM address for SCD */
	__le32 st_fwrd_addr;		/* pointer to Store and forward */
	__le32 st_fwrd_size;
	u8 umac_minor;			/* UMAC version: minor */
	u8 umac_major;			/* UMAC version: major */
	__le16 umac_id;			/* UMAC version: id */
	__le32 error_info_addr;		/* SRAM address for UMAC error log */
	__le32 dbg_print_buff_addr;
} __packed; /* ALIVE_RES_API_S_VER_2 */

struct mvm_alive_resp {
	__le16 status;
	__le16 flags;
	__le32 ucode_minor;
	__le32 ucode_major;
	u8 ver_subtype;
	u8 ver_type;
	u8 mac;
	u8 opt;
	__le32 timestamp;
	__le32 error_event_table_ptr;	/* SRAM address for error log */
	__le32 log_event_table_ptr;	/* SRAM address for LMAC event log */
	__le32 cpu_register_ptr;
	__le32 dbgm_config_ptr;
	__le32 alive_counter_ptr;
	__le32 scd_base_ptr;		/* SRAM address for SCD */
	__le32 st_fwrd_addr;		/* pointer to Store and forward */
	__le32 st_fwrd_size;
	__le32 umac_minor;		/* UMAC version: minor */
	__le32 umac_major;		/* UMAC version: major */
	__le32 error_info_addr;		/* SRAM address for UMAC error log */
	__le32 dbg_print_buff_addr;
} __packed; /* ALIVE_RES_API_S_VER_3 */

/* Error response/notification */
enum {
	FW_ERR_UNKNOWN_CMD = 0x0,
	FW_ERR_INVALID_CMD_PARAM = 0x1,
	FW_ERR_SERVICE = 0x2,
	FW_ERR_ARC_MEMORY = 0x3,
	FW_ERR_ARC_CODE = 0x4,
	FW_ERR_WATCH_DOG = 0x5,
	FW_ERR_WEP_GRP_KEY_INDX = 0x10,
	FW_ERR_WEP_KEY_SIZE = 0x11,
	FW_ERR_OBSOLETE_FUNC = 0x12,
	FW_ERR_UNEXPECTED = 0xFE,
	FW_ERR_FATAL = 0xFF
};

/**
 * struct iwl_error_resp - FW error indication
 * ( REPLY_ERROR = 0x2 )
 * @error_type: one of FW_ERR_*
 * @cmd_id: the command ID for which the error occured
 * @bad_cmd_seq_num: sequence number of the erroneous command
 * @error_service: which service created the error, applicable only if
 *	error_type = 2, otherwise 0
 * @timestamp: TSF in usecs.
 */
struct iwl_error_resp {
	__le32 error_type;
	u8 cmd_id;
	u8 reserved1;
	__le16 bad_cmd_seq_num;
	__le32 error_service;
	__le64 timestamp;
} __packed;


/* Common PHY, MAC and Bindings definitions */

#define MAX_MACS_IN_BINDING	(3)
#define MAX_BINDINGS		(4)
#define AUX_BINDING_INDEX	(3)
#define MAX_PHYS		(4)

/* Used to extract ID and color from the context dword */
#define FW_CTXT_ID_POS	  (0)
#define FW_CTXT_ID_MSK	  (0xff << FW_CTXT_ID_POS)
#define FW_CTXT_COLOR_POS (8)
#define FW_CTXT_COLOR_MSK (0xff << FW_CTXT_COLOR_POS)
#define FW_CTXT_INVALID	  (0xffffffff)

#define FW_CMD_ID_AND_COLOR(_id, _color) ((_id << FW_CTXT_ID_POS) |\
					  (_color << FW_CTXT_COLOR_POS))

/* Possible actions on PHYs, MACs and Bindings */
enum {
	FW_CTXT_ACTION_STUB = 0,
	FW_CTXT_ACTION_ADD,
	FW_CTXT_ACTION_MODIFY,
	FW_CTXT_ACTION_REMOVE,
	FW_CTXT_ACTION_NUM
}; /* COMMON_CONTEXT_ACTION_API_E_VER_1 */

/* Time Events */

/* Time Event types, according to MAC type */
enum iwl_time_event_type {
	/* BSS Station Events */
	TE_BSS_STA_AGGRESSIVE_ASSOC,
	TE_BSS_STA_ASSOC,
	TE_BSS_EAP_DHCP_PROT,
	TE_BSS_QUIET_PERIOD,

	/* P2P Device Events */
	TE_P2P_DEVICE_DISCOVERABLE,
	TE_P2P_DEVICE_LISTEN,
	TE_P2P_DEVICE_ACTION_SCAN,
	TE_P2P_DEVICE_FULL_SCAN,

	/* P2P Client Events */
	TE_P2P_CLIENT_AGGRESSIVE_ASSOC,
	TE_P2P_CLIENT_ASSOC,
	TE_P2P_CLIENT_QUIET_PERIOD,

	/* P2P GO Events */
	TE_P2P_GO_ASSOC_PROT,
	TE_P2P_GO_REPETITIVE_NOA,
	TE_P2P_GO_CT_WINDOW,

	/* WiDi Sync Events */
	TE_WIDI_TX_SYNC,

	/* Channel Switch NoA */
	TE_CHANNEL_SWITCH_PERIOD,

	TE_MAX
}; /* MAC_EVENT_TYPE_API_E_VER_1 */



/* Time event - defines for command API v1 */

/*
 * @TE_V1_FRAG_NONE: fragmentation of the time event is NOT allowed.
 * @TE_V1_FRAG_SINGLE: fragmentation of the time event is allowed, but only
 *	the first fragment is scheduled.
 * @TE_V1_FRAG_DUAL: fragmentation of the time event is allowed, but only
 *	the first 2 fragments are scheduled.
 * @TE_V1_FRAG_ENDLESS: fragmentation of the time event is allowed, and any
 *	number of fragments are valid.
 *
 * Other than the constant defined above, specifying a fragmentation value 'x'
 * means that the event can be fragmented but only the first 'x' will be
 * scheduled.
 */
enum {
	TE_V1_FRAG_NONE = 0,
	TE_V1_FRAG_SINGLE = 1,
	TE_V1_FRAG_DUAL = 2,
	TE_V1_FRAG_ENDLESS = 0xffffffff
};

/* If a Time Event can be fragmented, this is the max number of fragments */
#define TE_V1_FRAG_MAX_MSK	0x0fffffff
/* Repeat the time event endlessly (until removed) */
#define TE_V1_REPEAT_ENDLESS	0xffffffff
/* If a Time Event has bounded repetitions, this is the maximal value */
#define TE_V1_REPEAT_MAX_MSK_V1	0x0fffffff

/* Time Event dependencies: none, on another TE, or in a specific time */
enum {
	TE_V1_INDEPENDENT		= 0,
	TE_V1_DEP_OTHER			= BIT(0),
	TE_V1_DEP_TSF			= BIT(1),
	TE_V1_EVENT_SOCIOPATHIC		= BIT(2),
}; /* MAC_EVENT_DEPENDENCY_POLICY_API_E_VER_2 */

/*
 * @TE_V1_NOTIF_NONE: no notifications
 * @TE_V1_NOTIF_HOST_EVENT_START: request/receive notification on event start
 * @TE_V1_NOTIF_HOST_EVENT_END:request/receive notification on event end
 * @TE_V1_NOTIF_INTERNAL_EVENT_START: internal FW use
 * @TE_V1_NOTIF_INTERNAL_EVENT_END: internal FW use.
 * @TE_V1_NOTIF_HOST_FRAG_START: request/receive notification on frag start
 * @TE_V1_NOTIF_HOST_FRAG_END:request/receive notification on frag end
 * @TE_V1_NOTIF_INTERNAL_FRAG_START: internal FW use.
 * @TE_V1_NOTIF_INTERNAL_FRAG_END: internal FW use.
 *
 * Supported Time event notifications configuration.
 * A notification (both event and fragment) includes a status indicating weather
 * the FW was able to schedule the event or not. For fragment start/end
 * notification the status is always success. There is no start/end fragment
 * notification for monolithic events.
 */
enum {
	TE_V1_NOTIF_NONE = 0,
	TE_V1_NOTIF_HOST_EVENT_START = BIT(0),
	TE_V1_NOTIF_HOST_EVENT_END = BIT(1),
	TE_V1_NOTIF_INTERNAL_EVENT_START = BIT(2),
	TE_V1_NOTIF_INTERNAL_EVENT_END = BIT(3),
	TE_V1_NOTIF_HOST_FRAG_START = BIT(4),
	TE_V1_NOTIF_HOST_FRAG_END = BIT(5),
	TE_V1_NOTIF_INTERNAL_FRAG_START = BIT(6),
	TE_V1_NOTIF_INTERNAL_FRAG_END = BIT(7),
}; /* MAC_EVENT_ACTION_API_E_VER_2 */

/* Time event - defines for command API */

/*
 * @TE_V2_FRAG_NONE: fragmentation of the time event is NOT allowed.
 * @TE_V2_FRAG_SINGLE: fragmentation of the time event is allowed, but only
 *  the first fragment is scheduled.
 * @TE_V2_FRAG_DUAL: fragmentation of the time event is allowed, but only
 *  the first 2 fragments are scheduled.
 * @TE_V2_FRAG_ENDLESS: fragmentation of the time event is allowed, and any
 *  number of fragments are valid.
 *
 * Other than the constant defined above, specifying a fragmentation value 'x'
 * means that the event can be fragmented but only the first 'x' will be
 * scheduled.
 */
enum {
	TE_V2_FRAG_NONE = 0,
	TE_V2_FRAG_SINGLE = 1,
	TE_V2_FRAG_DUAL = 2,
	TE_V2_FRAG_MAX = 0xfe,
	TE_V2_FRAG_ENDLESS = 0xff
};

/* Repeat the time event endlessly (until removed) */
#define TE_V2_REPEAT_ENDLESS	0xff
/* If a Time Event has bounded repetitions, this is the maximal value */
#define TE_V2_REPEAT_MAX	0xfe

#define TE_V2_PLACEMENT_POS	12
#define TE_V2_ABSENCE_POS	15

/* Time event policy values
 * A notification (both event and fragment) includes a status indicating weather
 * the FW was able to schedule the event or not. For fragment start/end
 * notification the status is always success. There is no start/end fragment
 * notification for monolithic events.
 *
 * @TE_V2_DEFAULT_POLICY: independent, social, present, unoticable
 * @TE_V2_NOTIF_HOST_EVENT_START: request/receive notification on event start
 * @TE_V2_NOTIF_HOST_EVENT_END:request/receive notification on event end
 * @TE_V2_NOTIF_INTERNAL_EVENT_START: internal FW use
 * @TE_V2_NOTIF_INTERNAL_EVENT_END: internal FW use.
 * @TE_V2_NOTIF_HOST_FRAG_START: request/receive notification on frag start
 * @TE_V2_NOTIF_HOST_FRAG_END:request/receive notification on frag end
 * @TE_V2_NOTIF_INTERNAL_FRAG_START: internal FW use.
 * @TE_V2_NOTIF_INTERNAL_FRAG_END: internal FW use.
 * @TE_V2_DEP_OTHER: depends on another time event
 * @TE_V2_DEP_TSF: depends on a specific time
 * @TE_V2_EVENT_SOCIOPATHIC: can't co-exist with other events of tha same MAC
 * @TE_V2_ABSENCE: are we present or absent during the Time Event.
 */
enum {
	TE_V2_DEFAULT_POLICY = 0x0,

	/* notifications (event start/stop, fragment start/stop) */
	TE_V2_NOTIF_HOST_EVENT_START = BIT(0),
	TE_V2_NOTIF_HOST_EVENT_END = BIT(1),
	TE_V2_NOTIF_INTERNAL_EVENT_START = BIT(2),
	TE_V2_NOTIF_INTERNAL_EVENT_END = BIT(3),

	TE_V2_NOTIF_HOST_FRAG_START = BIT(4),
	TE_V2_NOTIF_HOST_FRAG_END = BIT(5),
	TE_V2_NOTIF_INTERNAL_FRAG_START = BIT(6),
	TE_V2_NOTIF_INTERNAL_FRAG_END = BIT(7),
	T2_V2_START_IMMEDIATELY = BIT(11),

	TE_V2_NOTIF_MSK = 0xff,

	/* placement characteristics */
	TE_V2_DEP_OTHER = BIT(TE_V2_PLACEMENT_POS),
	TE_V2_DEP_TSF = BIT(TE_V2_PLACEMENT_POS + 1),
	TE_V2_EVENT_SOCIOPATHIC = BIT(TE_V2_PLACEMENT_POS + 2),

	/* are we present or absent during the Time Event. */
	TE_V2_ABSENCE = BIT(TE_V2_ABSENCE_POS),
};

/**
 * struct iwl_time_event_cmd_api - configuring Time Events
 * with struct MAC_TIME_EVENT_DATA_API_S_VER_2 (see also
 * with version 1. determined by IWL_UCODE_TLV_FLAGS)
 * ( TIME_EVENT_CMD = 0x29 )
 * @id_and_color: ID and color of the relevant MAC
 * @action: action to perform, one of FW_CTXT_ACTION_*
 * @id: this field has two meanings, depending on the action:
 *	If the action is ADD, then it means the type of event to add.
 *	For all other actions it is the unique event ID assigned when the
 *	event was added by the FW.
 * @apply_time: When to start the Time Event (in GP2)
 * @max_delay: maximum delay to event's start (apply time), in TU
 * @depends_on: the unique ID of the event we depend on (if any)
 * @interval: interval between repetitions, in TU
 * @duration: duration of event in TU
 * @repeat: how many repetitions to do, can be TE_REPEAT_ENDLESS
 * @max_frags: maximal number of fragments the Time Event can be divided to
 * @policy: defines whether uCode shall notify the host or other uCode modules
 *	on event and/or fragment start and/or end
 *	using one of TE_INDEPENDENT, TE_DEP_OTHER, TE_DEP_TSF
 *	TE_EVENT_SOCIOPATHIC
 *	using TE_ABSENCE and using TE_NOTIF_*
 */
struct iwl_time_event_cmd {
	/* COMMON_INDEX_HDR_API_S_VER_1 */
	__le32 id_and_color;
	__le32 action;
	__le32 id;
	/* MAC_TIME_EVENT_DATA_API_S_VER_2 */
	__le32 apply_time;
	__le32 max_delay;
	__le32 depends_on;
	__le32 interval;
	__le32 duration;
	u8 repeat;
	u8 max_frags;
	__le16 policy;
} __packed; /* MAC_TIME_EVENT_CMD_API_S_VER_2 */

/**
 * struct iwl_time_event_resp - response structure to iwl_time_event_cmd
 * @status: bit 0 indicates success, all others specify errors
 * @id: the Time Event type
 * @unique_id: the unique ID assigned (in ADD) or given (others) to the TE
 * @id_and_color: ID and color of the relevant MAC
 */
struct iwl_time_event_resp {
	__le32 status;
	__le32 id;
	__le32 unique_id;
	__le32 id_and_color;
} __packed; /* MAC_TIME_EVENT_RSP_API_S_VER_1 */

/**
 * struct iwl_time_event_notif - notifications of time event start/stop
 * ( TIME_EVENT_NOTIFICATION = 0x2a )
 * @timestamp: action timestamp in GP2
 * @session_id: session's unique id
 * @unique_id: unique id of the Time Event itself
 * @id_and_color: ID and color of the relevant MAC
 * @action: one of TE_NOTIF_START or TE_NOTIF_END
 * @status: true if scheduled, false otherwise (not executed)
 */
struct iwl_time_event_notif {
	__le32 timestamp;
	__le32 session_id;
	__le32 unique_id;
	__le32 id_and_color;
	__le32 action;
	__le32 status;
} __packed; /* MAC_TIME_EVENT_NTFY_API_S_VER_1 */


/* Bindings and Time Quota */

/**
 * struct iwl_binding_cmd - configuring bindings
 * ( BINDING_CONTEXT_CMD = 0x2b )
 * @id_and_color: ID and color of the relevant Binding
 * @action: action to perform, one of FW_CTXT_ACTION_*
 * @macs: array of MAC id and colors which belong to the binding
 * @phy: PHY id and color which belongs to the binding
 */
struct iwl_binding_cmd {
	/* COMMON_INDEX_HDR_API_S_VER_1 */
	__le32 id_and_color;
	__le32 action;
	/* BINDING_DATA_API_S_VER_1 */
	__le32 macs[MAX_MACS_IN_BINDING];
	__le32 phy;
} __packed; /* BINDING_CMD_API_S_VER_1 */

/* The maximal number of fragments in the FW's schedule session */
#define IWL_MVM_MAX_QUOTA 128

/**
 * struct iwl_time_quota_data - configuration of time quota per binding
 * @id_and_color: ID and color of the relevant Binding
 * @quota: absolute time quota in TU. The scheduler will try to divide the
 *	remainig quota (after Time Events) according to this quota.
 * @max_duration: max uninterrupted context duration in TU
 */
struct iwl_time_quota_data {
	__le32 id_and_color;
	__le32 quota;
	__le32 max_duration;
} __packed; /* TIME_QUOTA_DATA_API_S_VER_1 */

/**
 * struct iwl_time_quota_cmd - configuration of time quota between bindings
 * ( TIME_QUOTA_CMD = 0x2c )
 * @quotas: allocations per binding
 */
struct iwl_time_quota_cmd {
	struct iwl_time_quota_data quotas[MAX_BINDINGS];
} __packed; /* TIME_QUOTA_ALLOCATION_CMD_API_S_VER_1 */


/* PHY context */

/* Supported bands */
#define PHY_BAND_5  (0)
#define PHY_BAND_24 (1)

/* Supported channel width, vary if there is VHT support */
#define PHY_VHT_CHANNEL_MODE20	(0x0)
#define PHY_VHT_CHANNEL_MODE40	(0x1)
#define PHY_VHT_CHANNEL_MODE80	(0x2)
#define PHY_VHT_CHANNEL_MODE160	(0x3)

/*
 * Control channel position:
 * For legacy set bit means upper channel, otherwise lower.
 * For VHT - bit-2 marks if the control is lower/upper relative to center-freq
 *   bits-1:0 mark the distance from the center freq. for 20Mhz, offset is 0.
 *                                   center_freq
 *                                        |
 * 40Mhz                          |_______|_______|
 * 80Mhz                  |_______|_______|_______|_______|
 * 160Mhz |_______|_______|_______|_______|_______|_______|_______|_______|
 * code      011     010     001     000  |  100     101     110    111
 */
#define PHY_VHT_CTRL_POS_1_BELOW  (0x0)
#define PHY_VHT_CTRL_POS_2_BELOW  (0x1)
#define PHY_VHT_CTRL_POS_3_BELOW  (0x2)
#define PHY_VHT_CTRL_POS_4_BELOW  (0x3)
#define PHY_VHT_CTRL_POS_1_ABOVE  (0x4)
#define PHY_VHT_CTRL_POS_2_ABOVE  (0x5)
#define PHY_VHT_CTRL_POS_3_ABOVE  (0x6)
#define PHY_VHT_CTRL_POS_4_ABOVE  (0x7)

/*
 * @band: PHY_BAND_*
 * @channel: channel number
 * @width: PHY_[VHT|LEGACY]_CHANNEL_*
 * @ctrl channel: PHY_[VHT|LEGACY]_CTRL_*
 */
struct iwl_fw_channel_info {
	u8 band;
	u8 channel;
	u8 width;
	u8 ctrl_pos;
} __packed;

#define PHY_RX_CHAIN_DRIVER_FORCE_POS	(0)
#define PHY_RX_CHAIN_DRIVER_FORCE_MSK \
	(0x1 << PHY_RX_CHAIN_DRIVER_FORCE_POS)
#define PHY_RX_CHAIN_VALID_POS		(1)
#define PHY_RX_CHAIN_VALID_MSK \
	(0x7 << PHY_RX_CHAIN_VALID_POS)
#define PHY_RX_CHAIN_FORCE_SEL_POS	(4)
#define PHY_RX_CHAIN_FORCE_SEL_MSK \
	(0x7 << PHY_RX_CHAIN_FORCE_SEL_POS)
#define PHY_RX_CHAIN_FORCE_MIMO_SEL_POS	(7)
#define PHY_RX_CHAIN_FORCE_MIMO_SEL_MSK \
	(0x7 << PHY_RX_CHAIN_FORCE_MIMO_SEL_POS)
#define PHY_RX_CHAIN_CNT_POS		(10)
#define PHY_RX_CHAIN_CNT_MSK \
	(0x3 << PHY_RX_CHAIN_CNT_POS)
#define PHY_RX_CHAIN_MIMO_CNT_POS	(12)
#define PHY_RX_CHAIN_MIMO_CNT_MSK \
	(0x3 << PHY_RX_CHAIN_MIMO_CNT_POS)
#define PHY_RX_CHAIN_MIMO_FORCE_POS	(14)
#define PHY_RX_CHAIN_MIMO_FORCE_MSK \
	(0x1 << PHY_RX_CHAIN_MIMO_FORCE_POS)

/* TODO: fix the value, make it depend on firmware at runtime? */
#define NUM_PHY_CTX	3

/* TODO: complete missing documentation */
/**
 * struct iwl_phy_context_cmd - config of the PHY context
 * ( PHY_CONTEXT_CMD = 0x8 )
 * @id_and_color: ID and color of the relevant Binding
 * @action: action to perform, one of FW_CTXT_ACTION_*
 * @apply_time: 0 means immediate apply and context switch.
 *	other value means apply new params after X usecs
 * @tx_param_color: ???
 * @channel_info:
 * @txchain_info: ???
 * @rxchain_info: ???
 * @acquisition_data: ???
 * @dsp_cfg_flags: set to 0
 */
struct iwl_phy_context_cmd {
	/* COMMON_INDEX_HDR_API_S_VER_1 */
	__le32 id_and_color;
	__le32 action;
	/* PHY_CONTEXT_DATA_API_S_VER_1 */
	__le32 apply_time;
	__le32 tx_param_color;
	struct iwl_fw_channel_info ci;
	__le32 txchain_info;
	__le32 rxchain_info;
	__le32 acquisition_data;
	__le32 dsp_cfg_flags;
} __packed; /* PHY_CONTEXT_CMD_API_VER_1 */

/*
 * Aux ROC command
 *
 * Command requests the firmware to create a time event for a certain duration
 * and remain on the given channel. This is done by using the Aux framework in
 * the FW.
 * The command was first used for Hot Spot issues - but can be used regardless
 * to Hot Spot.
 *
 * ( HOT_SPOT_CMD 0x53 )
 *
 * @id_and_color: ID and color of the MAC
 * @action: action to perform, one of FW_CTXT_ACTION_*
 * @event_unique_id: If the action FW_CTXT_ACTION_REMOVE then the
 *	event_unique_id should be the id of the time event assigned by ucode.
 *	Otherwise ignore the event_unique_id.
 * @sta_id_and_color: station id and color, resumed during "Remain On Channel"
 *	activity.
 * @channel_info: channel info
 * @node_addr: Our MAC Address
 * @reserved: reserved for alignment
 * @apply_time: GP2 value to start (should always be the current GP2 value)
 * @apply_time_max_delay: Maximum apply time delay value in TU. Defines max
 *	time by which start of the event is allowed to be postponed.
 * @duration: event duration in TU To calculate event duration:
 *	timeEventDuration = min(duration, remainingQuota)
 */
struct iwl_hs20_roc_req {
	/* COMMON_INDEX_HDR_API_S_VER_1 hdr */
	__le32 id_and_color;
	__le32 action;
	__le32 event_unique_id;
	__le32 sta_id_and_color;
	struct iwl_fw_channel_info channel_info;
	u8 node_addr[ETH_ALEN];
	__le16 reserved;
	__le32 apply_time;
	__le32 apply_time_max_delay;
	__le32 duration;
} __packed; /* HOT_SPOT_CMD_API_S_VER_1 */

/*
 * values for AUX ROC result values
 */
enum iwl_mvm_hot_spot {
	HOT_SPOT_RSP_STATUS_OK,
	HOT_SPOT_RSP_STATUS_TOO_MANY_EVENTS,
	HOT_SPOT_MAX_NUM_OF_SESSIONS,
};

/*
 * Aux ROC command response
 *
 * In response to iwl_hs20_roc_req the FW sends this command to notify the
 * driver the uid of the timevent.
 *
 * ( HOT_SPOT_CMD 0x53 )
 *
 * @event_unique_id: Unique ID of time event assigned by ucode
 * @status: Return status 0 is success, all the rest used for specific errors
 */
struct iwl_hs20_roc_res {
	__le32 event_unique_id;
	__le32 status;
} __packed; /* HOT_SPOT_RSP_API_S_VER_1 */

#define IWL_RX_INFO_PHY_CNT 8
#define IWL_RX_INFO_ENERGY_ANT_ABC_IDX 1
#define IWL_RX_INFO_ENERGY_ANT_A_MSK 0x000000ff
#define IWL_RX_INFO_ENERGY_ANT_B_MSK 0x0000ff00
#define IWL_RX_INFO_ENERGY_ANT_C_MSK 0x00ff0000
#define IWL_RX_INFO_ENERGY_ANT_A_POS 0
#define IWL_RX_INFO_ENERGY_ANT_B_POS 8
#define IWL_RX_INFO_ENERGY_ANT_C_POS 16

#define IWL_RX_INFO_AGC_IDX 1
#define IWL_RX_INFO_RSSI_AB_IDX 2
#define IWL_OFDM_AGC_A_MSK 0x0000007f
#define IWL_OFDM_AGC_A_POS 0
#define IWL_OFDM_AGC_B_MSK 0x00003f80
#define IWL_OFDM_AGC_B_POS 7
#define IWL_OFDM_AGC_CODE_MSK 0x3fe00000
#define IWL_OFDM_AGC_CODE_POS 20
#define IWL_OFDM_RSSI_INBAND_A_MSK 0x00ff
#define IWL_OFDM_RSSI_A_POS 0
#define IWL_OFDM_RSSI_ALLBAND_A_MSK 0xff00
#define IWL_OFDM_RSSI_ALLBAND_A_POS 8
#define IWL_OFDM_RSSI_INBAND_B_MSK 0xff0000
#define IWL_OFDM_RSSI_B_POS 16
#define IWL_OFDM_RSSI_ALLBAND_B_MSK 0xff000000
#define IWL_OFDM_RSSI_ALLBAND_B_POS 24

/**
 * struct iwl_rx_phy_info - phy info
 * (REPLY_RX_PHY_CMD = 0xc0)
 * @non_cfg_phy_cnt: non configurable DSP phy data byte count
 * @cfg_phy_cnt: configurable DSP phy data byte count
 * @stat_id: configurable DSP phy data set ID
 * @reserved1:
 * @system_timestamp: GP2  at on air rise
 * @timestamp: TSF at on air rise
 * @beacon_time_stamp: beacon at on-air rise
 * @phy_flags: general phy flags: band, modulation, ...
 * @channel: channel number
 * @non_cfg_phy_buf: for various implementations of non_cfg_phy
 * @rate_n_flags: RATE_MCS_*
 * @byte_count: frame's byte-count
 * @frame_time: frame's time on the air, based on byte count and frame rate
 *	calculation
 * @mac_active_msk: what MACs were active when the frame was received
 *
 * Before each Rx, the device sends this data. It contains PHY information
 * about the reception of the packet.
 */
struct iwl_rx_phy_info {
	u8 non_cfg_phy_cnt;
	u8 cfg_phy_cnt;
	u8 stat_id;
	u8 reserved1;
	__le32 system_timestamp;
	__le64 timestamp;
	__le32 beacon_time_stamp;
	__le16 phy_flags;
	__le16 channel;
	__le32 non_cfg_phy[IWL_RX_INFO_PHY_CNT];
	__le32 rate_n_flags;
	__le32 byte_count;
	__le16 mac_active_msk;
	__le16 frame_time;
} __packed;

struct iwl_rx_mpdu_res_start {
	__le16 byte_count;
	__le16 reserved;
} __packed;

/**
 * enum iwl_rx_phy_flags - to parse %iwl_rx_phy_info phy_flags
 * @RX_RES_PHY_FLAGS_BAND_24: true if the packet was received on 2.4 band
 * @RX_RES_PHY_FLAGS_MOD_CCK:
 * @RX_RES_PHY_FLAGS_SHORT_PREAMBLE: true if packet's preamble was short
 * @RX_RES_PHY_FLAGS_NARROW_BAND:
 * @RX_RES_PHY_FLAGS_ANTENNA: antenna on which the packet was received
 * @RX_RES_PHY_FLAGS_AGG: set if the packet was part of an A-MPDU
 * @RX_RES_PHY_FLAGS_OFDM_HT: The frame was an HT frame
 * @RX_RES_PHY_FLAGS_OFDM_GF: The frame used GF preamble
 * @RX_RES_PHY_FLAGS_OFDM_VHT: The frame was a VHT frame
 */
enum iwl_rx_phy_flags {
	RX_RES_PHY_FLAGS_BAND_24	= BIT(0),
	RX_RES_PHY_FLAGS_MOD_CCK	= BIT(1),
	RX_RES_PHY_FLAGS_SHORT_PREAMBLE	= BIT(2),
	RX_RES_PHY_FLAGS_NARROW_BAND	= BIT(3),
	RX_RES_PHY_FLAGS_ANTENNA	= (0x7 << 4),
	RX_RES_PHY_FLAGS_ANTENNA_POS	= 4,
	RX_RES_PHY_FLAGS_AGG		= BIT(7),
	RX_RES_PHY_FLAGS_OFDM_HT	= BIT(8),
	RX_RES_PHY_FLAGS_OFDM_GF	= BIT(9),
	RX_RES_PHY_FLAGS_OFDM_VHT	= BIT(10),
};

/**
 * enum iwl_mvm_rx_status - written by fw for each Rx packet
 * @RX_MPDU_RES_STATUS_CRC_OK: CRC is fine
 * @RX_MPDU_RES_STATUS_OVERRUN_OK: there was no RXE overflow
 * @RX_MPDU_RES_STATUS_SRC_STA_FOUND:
 * @RX_MPDU_RES_STATUS_KEY_VALID:
 * @RX_MPDU_RES_STATUS_KEY_PARAM_OK:
 * @RX_MPDU_RES_STATUS_ICV_OK: ICV is fine, if not, the packet is destroyed
 * @RX_MPDU_RES_STATUS_MIC_OK: used for CCM alg only. TKIP MIC is checked
 *	in the driver.
 * @RX_MPDU_RES_STATUS_TTAK_OK: TTAK is fine
 * @RX_MPDU_RES_STATUS_MNG_FRAME_REPLAY_ERR:  valid for alg = CCM_CMAC or
 *	alg = CCM only. Checks replay attack for 11w frames. Relevant only if
 *	%RX_MPDU_RES_STATUS_ROBUST_MNG_FRAME is set.
 * @RX_MPDU_RES_STATUS_SEC_NO_ENC: this frame is not encrypted
 * @RX_MPDU_RES_STATUS_SEC_WEP_ENC: this frame is encrypted using WEP
 * @RX_MPDU_RES_STATUS_SEC_CCM_ENC: this frame is encrypted using CCM
 * @RX_MPDU_RES_STATUS_SEC_TKIP_ENC: this frame is encrypted using TKIP
 * @RX_MPDU_RES_STATUS_SEC_CCM_CMAC_ENC: this frame is encrypted using CCM_CMAC
 * @RX_MPDU_RES_STATUS_SEC_ENC_ERR: this frame couldn't be decrypted
 * @RX_MPDU_RES_STATUS_SEC_ENC_MSK: bitmask of the encryption algorithm
 * @RX_MPDU_RES_STATUS_DEC_DONE: this frame has been successfully decrypted
 * @RX_MPDU_RES_STATUS_PROTECT_FRAME_BIT_CMP:
 * @RX_MPDU_RES_STATUS_EXT_IV_BIT_CMP:
 * @RX_MPDU_RES_STATUS_KEY_ID_CMP_BIT:
 * @RX_MPDU_RES_STATUS_ROBUST_MNG_FRAME: this frame is an 11w management frame
 * @RX_MPDU_RES_STATUS_HASH_INDEX_MSK:
 * @RX_MPDU_RES_STATUS_STA_ID_MSK:
 * @RX_MPDU_RES_STATUS_RRF_KILL:
 * @RX_MPDU_RES_STATUS_FILTERING_MSK:
 * @RX_MPDU_RES_STATUS2_FILTERING_MSK:
 */
enum iwl_mvm_rx_status {
	RX_MPDU_RES_STATUS_CRC_OK			= BIT(0),
	RX_MPDU_RES_STATUS_OVERRUN_OK			= BIT(1),
	RX_MPDU_RES_STATUS_SRC_STA_FOUND		= BIT(2),
	RX_MPDU_RES_STATUS_KEY_VALID			= BIT(3),
	RX_MPDU_RES_STATUS_KEY_PARAM_OK			= BIT(4),
	RX_MPDU_RES_STATUS_ICV_OK			= BIT(5),
	RX_MPDU_RES_STATUS_MIC_OK			= BIT(6),
	RX_MPDU_RES_STATUS_TTAK_OK			= BIT(7),
	RX_MPDU_RES_STATUS_MNG_FRAME_REPLAY_ERR		= BIT(7),
	RX_MPDU_RES_STATUS_SEC_NO_ENC			= (0 << 8),
	RX_MPDU_RES_STATUS_SEC_WEP_ENC			= (1 << 8),
	RX_MPDU_RES_STATUS_SEC_CCM_ENC			= (2 << 8),
	RX_MPDU_RES_STATUS_SEC_TKIP_ENC			= (3 << 8),
	RX_MPDU_RES_STATUS_SEC_EXT_ENC			= (4 << 8),
	RX_MPDU_RES_STATUS_SEC_CCM_CMAC_ENC		= (6 << 8),
	RX_MPDU_RES_STATUS_SEC_ENC_ERR			= (7 << 8),
	RX_MPDU_RES_STATUS_SEC_ENC_MSK			= (7 << 8),
	RX_MPDU_RES_STATUS_DEC_DONE			= BIT(11),
	RX_MPDU_RES_STATUS_PROTECT_FRAME_BIT_CMP	= BIT(12),
	RX_MPDU_RES_STATUS_EXT_IV_BIT_CMP		= BIT(13),
	RX_MPDU_RES_STATUS_KEY_ID_CMP_BIT		= BIT(14),
	RX_MPDU_RES_STATUS_ROBUST_MNG_FRAME		= BIT(15),
	RX_MPDU_RES_STATUS_HASH_INDEX_MSK		= (0x3F0000),
	RX_MPDU_RES_STATUS_STA_ID_MSK			= (0x1f000000),
	RX_MPDU_RES_STATUS_RRF_KILL			= BIT(29),
	RX_MPDU_RES_STATUS_FILTERING_MSK		= (0xc00000),
	RX_MPDU_RES_STATUS2_FILTERING_MSK		= (0xc0000000),
};

/**
 * struct iwl_radio_version_notif - information on the radio version
 * ( RADIO_VERSION_NOTIFICATION = 0x68 )
 * @radio_flavor:
 * @radio_step:
 * @radio_dash:
 */
struct iwl_radio_version_notif {
	__le32 radio_flavor;
	__le32 radio_step;
	__le32 radio_dash;
} __packed; /* RADIO_VERSION_NOTOFICATION_S_VER_1 */

enum iwl_card_state_flags {
	CARD_ENABLED		= 0x00,
	HW_CARD_DISABLED	= 0x01,
	SW_CARD_DISABLED	= 0x02,
	CT_KILL_CARD_DISABLED	= 0x04,
	HALT_CARD_DISABLED	= 0x08,
	CARD_DISABLED_MSK	= 0x0f,
	CARD_IS_RX_ON		= 0x10,
};

/**
 * struct iwl_radio_version_notif - information on the radio version
 * ( CARD_STATE_NOTIFICATION = 0xa1 )
 * @flags: %iwl_card_state_flags
 */
struct iwl_card_state_notif {
	__le32 flags;
} __packed; /* CARD_STATE_NTFY_API_S_VER_1 */

/**
 * struct iwl_missed_beacons_notif - information on missed beacons
 * ( MISSED_BEACONS_NOTIFICATION = 0xa2 )
 * @mac_id: interface ID
 * @consec_missed_beacons_since_last_rx: number of consecutive missed
 *	beacons since last RX.
 * @consec_missed_beacons: number of consecutive missed beacons
 * @num_expected_beacons:
 * @num_recvd_beacons:
 */
struct iwl_missed_beacons_notif {
	__le32 mac_id;
	__le32 consec_missed_beacons_since_last_rx;
	__le32 consec_missed_beacons;
	__le32 num_expected_beacons;
	__le32 num_recvd_beacons;
} __packed; /* MISSED_BEACON_NTFY_API_S_VER_3 */

/**
 * struct iwl_mfuart_load_notif - mfuart image version & status
 * ( MFUART_LOAD_NOTIFICATION = 0xb1 )
 * @installed_ver: installed image version
 * @external_ver: external image version
 * @status: MFUART loading status
 * @duration: MFUART loading time
*/
struct iwl_mfuart_load_notif {
	__le32 installed_ver;
	__le32 external_ver;
	__le32 status;
	__le32 duration;
} __packed; /*MFU_LOADER_NTFY_API_S_VER_1*/

/**
 * struct iwl_set_calib_default_cmd - set default value for calibration.
 * ( SET_CALIB_DEFAULT_CMD = 0x8e )
 * @calib_index: the calibration to set value for
 * @length: of data
 * @data: the value to set for the calibration result
 */
struct iwl_set_calib_default_cmd {
	__le16 calib_index;
	__le16 length;
	u8 data[0];
} __packed; /* PHY_CALIB_OVERRIDE_VALUES_S */

#define MAX_PORT_ID_NUM	2
#define MAX_MCAST_FILTERING_ADDRESSES 256

/**
 * struct iwl_mcast_filter_cmd - configure multicast filter.
 * @filter_own: Set 1 to filter out multicast packets sent by station itself
 * @port_id:	Multicast MAC addresses array specifier. This is a strange way
 *		to identify network interface adopted in host-device IF.
 *		It is used by FW as index in array of addresses. This array has
 *		MAX_PORT_ID_NUM members.
 * @count:	Number of MAC addresses in the array
 * @pass_all:	Set 1 to pass all multicast packets.
 * @bssid:	current association BSSID.
 * @addr_list:	Place holder for array of MAC addresses.
 *		IMPORTANT: add padding if necessary to ensure DWORD alignment.
 */
struct iwl_mcast_filter_cmd {
	u8 filter_own;
	u8 port_id;
	u8 count;
	u8 pass_all;
	u8 bssid[6];
	u8 reserved[2];
	u8 addr_list[0];
} __packed; /* MCAST_FILTERING_CMD_API_S_VER_1 */

#define MAX_BCAST_FILTERS 8
#define MAX_BCAST_FILTER_ATTRS 2

/**
 * enum iwl_mvm_bcast_filter_attr_offset - written by fw for each Rx packet
 * @BCAST_FILTER_OFFSET_PAYLOAD_START: offset is from payload start.
 * @BCAST_FILTER_OFFSET_IP_END: offset is from ip header end (i.e.
 *	start of ip payload).
 */
enum iwl_mvm_bcast_filter_attr_offset {
	BCAST_FILTER_OFFSET_PAYLOAD_START = 0,
	BCAST_FILTER_OFFSET_IP_END = 1,
};

/**
 * struct iwl_fw_bcast_filter_attr - broadcast filter attribute
 * @offset_type:	&enum iwl_mvm_bcast_filter_attr_offset.
 * @offset:	starting offset of this pattern.
 * @val:		value to match - big endian (MSB is the first
 *		byte to match from offset pos).
 * @mask:	mask to match (big endian).
 */
struct iwl_fw_bcast_filter_attr {
	u8 offset_type;
	u8 offset;
	__le16 reserved1;
	__be32 val;
	__be32 mask;
} __packed; /* BCAST_FILTER_ATT_S_VER_1 */

/**
 * enum iwl_mvm_bcast_filter_frame_type - filter frame type
 * @BCAST_FILTER_FRAME_TYPE_ALL: consider all frames.
 * @BCAST_FILTER_FRAME_TYPE_IPV4: consider only ipv4 frames
 */
enum iwl_mvm_bcast_filter_frame_type {
	BCAST_FILTER_FRAME_TYPE_ALL = 0,
	BCAST_FILTER_FRAME_TYPE_IPV4 = 1,
};

/**
 * struct iwl_fw_bcast_filter - broadcast filter
 * @discard: discard frame (1) or let it pass (0).
 * @frame_type: &enum iwl_mvm_bcast_filter_frame_type.
 * @num_attrs: number of valid attributes in this filter.
 * @attrs: attributes of this filter. a filter is considered matched
 *	only when all its attributes are matched (i.e. AND relationship)
 */
struct iwl_fw_bcast_filter {
	u8 discard;
	u8 frame_type;
	u8 num_attrs;
	u8 reserved1;
	struct iwl_fw_bcast_filter_attr attrs[MAX_BCAST_FILTER_ATTRS];
} __packed; /* BCAST_FILTER_S_VER_1 */

/**
 * struct iwl_fw_bcast_mac - per-mac broadcast filtering configuration.
 * @default_discard: default action for this mac (discard (1) / pass (0)).
 * @attached_filters: bitmap of relevant filters for this mac.
 */
struct iwl_fw_bcast_mac {
	u8 default_discard;
	u8 reserved1;
	__le16 attached_filters;
} __packed; /* BCAST_MAC_CONTEXT_S_VER_1 */

/**
 * struct iwl_bcast_filter_cmd - broadcast filtering configuration
 * @disable: enable (0) / disable (1)
 * @max_bcast_filters: max number of filters (MAX_BCAST_FILTERS)
 * @max_macs: max number of macs (NUM_MAC_INDEX_DRIVER)
 * @filters: broadcast filters
 * @macs: broadcast filtering configuration per-mac
 */
struct iwl_bcast_filter_cmd {
	u8 disable;
	u8 max_bcast_filters;
	u8 max_macs;
	u8 reserved1;
	struct iwl_fw_bcast_filter filters[MAX_BCAST_FILTERS];
	struct iwl_fw_bcast_mac macs[NUM_MAC_INDEX_DRIVER];
} __packed; /* BCAST_FILTERING_HCMD_API_S_VER_1 */

/*
 * enum iwl_mvm_marker_id - maker ids
 *
 * The ids for different type of markers to insert into the usniffer logs
 */
enum iwl_mvm_marker_id {
	MARKER_ID_TX_FRAME_LATENCY = 1,
}; /* MARKER_ID_API_E_VER_1 */

/**
 * struct iwl_mvm_marker - mark info into the usniffer logs
 *
 * (MARKER_CMD = 0xcb)
 *
 * Mark the UTC time stamp into the usniffer logs together with additional
 * metadata, so the usniffer output can be parsed.
 * In the command response the ucode will return the GP2 time.
 *
 * @dw_len: The amount of dwords following this byte including this byte.
 * @marker_id: A unique marker id (iwl_mvm_marker_id).
 * @reserved: reserved.
 * @timestamp: in milliseconds since 1970-01-01 00:00:00 UTC
 * @metadata: additional meta data that will be written to the unsiffer log
 */
struct iwl_mvm_marker {
	u8 dwLen;
	u8 markerId;
	__le16 reserved;
	__le64 timestamp;
	__le32 metadata[0];
} __packed; /* MARKER_API_S_VER_1 */

/***********************************
 * Smart Fifo API
 ***********************************/
/* Smart Fifo state */
enum iwl_sf_state {
	SF_LONG_DELAY_ON = 0, /* should never be called by driver */
	SF_FULL_ON,
	SF_UNINIT,
	SF_INIT_OFF,
	SF_HW_NUM_STATES
};

/* Smart Fifo possible scenario */
enum iwl_sf_scenario {
	SF_SCENARIO_SINGLE_UNICAST,
	SF_SCENARIO_AGG_UNICAST,
	SF_SCENARIO_MULTICAST,
	SF_SCENARIO_BA_RESP,
	SF_SCENARIO_TX_RESP,
	SF_NUM_SCENARIO
};

#define SF_TRANSIENT_STATES_NUMBER 2	/* SF_LONG_DELAY_ON and SF_FULL_ON */
#define SF_NUM_TIMEOUT_TYPES 2		/* Aging timer and Idle timer */

/* smart FIFO default values */
#define SF_W_MARK_SISO 6144
#define SF_W_MARK_MIMO2 8192
#define SF_W_MARK_MIMO3 6144
#define SF_W_MARK_LEGACY 4096
#define SF_W_MARK_SCAN 4096

/* SF Scenarios timers for default configuration (aligned to 32 uSec) */
#define SF_SINGLE_UNICAST_IDLE_TIMER_DEF 160	/* 150 uSec  */
#define SF_SINGLE_UNICAST_AGING_TIMER_DEF 400	/* 0.4 mSec */
#define SF_AGG_UNICAST_IDLE_TIMER_DEF 160		/* 150 uSec */
#define SF_AGG_UNICAST_AGING_TIMER_DEF 400		/* 0.4 mSec */
#define SF_MCAST_IDLE_TIMER_DEF 160		/* 150 mSec */
#define SF_MCAST_AGING_TIMER_DEF 400		/* 0.4 mSec */
#define SF_BA_IDLE_TIMER_DEF 160			/* 150 uSec */
#define SF_BA_AGING_TIMER_DEF 400			/* 0.4 mSec */
#define SF_TX_RE_IDLE_TIMER_DEF 160			/* 150 uSec */
#define SF_TX_RE_AGING_TIMER_DEF 400		/* 0.4 mSec */

/* SF Scenarios timers for BSS MAC configuration (aligned to 32 uSec) */
#define SF_SINGLE_UNICAST_IDLE_TIMER 320	/* 300 uSec  */
#define SF_SINGLE_UNICAST_AGING_TIMER 2016	/* 2 mSec */
#define SF_AGG_UNICAST_IDLE_TIMER 320		/* 300 uSec */
#define SF_AGG_UNICAST_AGING_TIMER 2016		/* 2 mSec */
#define SF_MCAST_IDLE_TIMER 2016		/* 2 mSec */
#define SF_MCAST_AGING_TIMER 10016		/* 10 mSec */
#define SF_BA_IDLE_TIMER 320			/* 300 uSec */
#define SF_BA_AGING_TIMER 2016			/* 2 mSec */
#define SF_TX_RE_IDLE_TIMER 320			/* 300 uSec */
#define SF_TX_RE_AGING_TIMER 2016		/* 2 mSec */

#define SF_LONG_DELAY_AGING_TIMER 1000000	/* 1 Sec */

#define SF_CFG_DUMMY_NOTIF_OFF	BIT(16)

/**
 * Smart Fifo configuration command.
 * @state: smart fifo state, types listed in enum %iwl_sf_sate.
 * @watermark: Minimum allowed availabe free space in RXF for transient state.
 * @long_delay_timeouts: aging and idle timer values for each scenario
 * in long delay state.
 * @full_on_timeouts: timer values for each scenario in full on state.
 */
struct iwl_sf_cfg_cmd {
	__le32 state;
	__le32 watermark[SF_TRANSIENT_STATES_NUMBER];
	__le32 long_delay_timeouts[SF_NUM_SCENARIO][SF_NUM_TIMEOUT_TYPES];
	__le32 full_on_timeouts[SF_NUM_SCENARIO][SF_NUM_TIMEOUT_TYPES];
} __packed; /* SF_CFG_API_S_VER_2 */

/***********************************
 * Location Aware Regulatory (LAR) API - MCC updates
 ***********************************/

/**
 * struct iwl_mcc_update_cmd - Request the device to update geographic
 * regulatory profile according to the given MCC (Mobile Country Code).
 * The MCC is two letter-code, ascii upper case[A-Z] or '00' for world domain.
 * 'ZZ' MCC will be used to switch to NVM default profile; in this case, the
 * MCC in the cmd response will be the relevant MCC in the NVM.
 * @mcc: given mobile country code
 * @source_id: the source from where we got the MCC, see iwl_mcc_source
 * @reserved: reserved for alignment
 */
struct iwl_mcc_update_cmd {
	__le16 mcc;
	u8 source_id;
	u8 reserved;
} __packed; /* LAR_UPDATE_MCC_CMD_API_S */

/**
 * iwl_mcc_update_resp - response to MCC_UPDATE_CMD.
 * Contains the new channel control profile map, if changed, and the new MCC
 * (mobile country code).
 * The new MCC may be different than what was requested in MCC_UPDATE_CMD.
 * @status: see &enum iwl_mcc_update_status
 * @mcc: the new applied MCC
 * @cap: capabilities for all channels which matches the MCC
 * @source_id: the MCC source, see iwl_mcc_source
 * @n_channels: number of channels in @channels_data (may be 14, 39, 50 or 51
 *		channels, depending on platform)
 * @channels: channel control data map, DWORD for each channel. Only the first
 *	16bits are used.
 */
struct iwl_mcc_update_resp {
	__le32 status;
	__le16 mcc;
	u8 cap;
	u8 source_id;
	__le32 n_channels;
	__le32 channels[0];
} __packed; /* LAR_UPDATE_MCC_CMD_RESP_S */

/**
 * struct iwl_mcc_chub_notif - chub notifies of mcc change
 * (MCC_CHUB_UPDATE_CMD = 0xc9)
 * The Chub (Communication Hub, CommsHUB) is a HW component that connects to
 * the cellular and connectivity cores that gets updates of the mcc, and
 * notifies the ucode directly of any mcc change.
 * The ucode requests the driver to request the device to update geographic
 * regulatory  profile according to the given MCC (Mobile Country Code).
 * The MCC is two letter-code, ascii upper case[A-Z] or '00' for world domain.
 * 'ZZ' MCC will be used to switch to NVM default profile; in this case, the
 * MCC in the cmd response will be the relevant MCC in the NVM.
 * @mcc: given mobile country code
 * @source_id: identity of the change originator, see iwl_mcc_source
 * @reserved1: reserved for alignment
 */
struct iwl_mcc_chub_notif {
	u16 mcc;
	u8 source_id;
	u8 reserved1;
} __packed; /* LAR_MCC_NOTIFY_S */

enum iwl_mcc_update_status {
	MCC_RESP_NEW_CHAN_PROFILE,
	MCC_RESP_SAME_CHAN_PROFILE,
	MCC_RESP_INVALID,
	MCC_RESP_NVM_DISABLED,
	MCC_RESP_ILLEGAL,
	MCC_RESP_LOW_PRIORITY,
};

enum iwl_mcc_source {
	MCC_SOURCE_OLD_FW = 0,
	MCC_SOURCE_ME = 1,
	MCC_SOURCE_BIOS = 2,
	MCC_SOURCE_3G_LTE_HOST = 3,
	MCC_SOURCE_3G_LTE_DEVICE = 4,
	MCC_SOURCE_WIFI = 5,
	MCC_SOURCE_RESERVED = 6,
	MCC_SOURCE_DEFAULT = 7,
	MCC_SOURCE_UNINITIALIZED = 8,
	MCC_SOURCE_GET_CURRENT = 0x10
};

/* DTS measurements */

enum iwl_dts_measurement_flags {
	DTS_TRIGGER_CMD_FLAGS_TEMP	= BIT(0),
	DTS_TRIGGER_CMD_FLAGS_VOLT	= BIT(1),
};

/**
 * iwl_dts_measurement_cmd - request DTS temperature and/or voltage measurements
 *
 * @flags: indicates which measurements we want as specified in &enum
 *	   iwl_dts_measurement_flags
 */
struct iwl_dts_measurement_cmd {
	__le32 flags;
} __packed; /* TEMPERATURE_MEASUREMENT_TRIGGER_CMD_S */

/**
 * iwl_dts_measurement_notif - notification received with the measurements
 *
 * @temp: the measured temperature
 * @voltage: the measured voltage
 */
struct iwl_dts_measurement_notif {
	__le32 temp;
	__le32 voltage;
} __packed; /* TEMPERATURE_MEASUREMENT_TRIGGER_NTFY_S */

/***********************************
 * TDLS API
 ***********************************/

/* Type of TDLS request */
enum iwl_tdls_channel_switch_type {
	TDLS_SEND_CHAN_SW_REQ = 0,
	TDLS_SEND_CHAN_SW_RESP_AND_MOVE_CH,
	TDLS_MOVE_CH,
}; /* TDLS_STA_CHANNEL_SWITCH_CMD_TYPE_API_E_VER_1 */

/**
 * Switch timing sub-element in a TDLS channel-switch command
 * @frame_timestamp: GP2 timestamp of channel-switch request/response packet
 *	received from peer
 * @max_offchan_duration: What amount of microseconds out of a DTIM is given
 *	to the TDLS off-channel communication. For instance if the DTIM is
 *	200TU and the TDLS peer is to be given 25% of the time, the value
 *	given will be 50TU, or 50 * 1024 if translated into microseconds.
 * @switch_time: switch time the peer sent in its channel switch timing IE
 * @switch_timout: switch timeout the peer sent in its channel switch timing IE
 */
struct iwl_tdls_channel_switch_timing {
	__le32 frame_timestamp; /* GP2 time of peer packet Rx */
	__le32 max_offchan_duration; /* given in micro-seconds */
	__le32 switch_time; /* given in micro-seconds */
	__le32 switch_timeout; /* given in micro-seconds */
} __packed; /* TDLS_STA_CHANNEL_SWITCH_TIMING_DATA_API_S_VER_1 */

#define IWL_TDLS_CH_SW_FRAME_MAX_SIZE 200

/**
 * TDLS channel switch frame template
 *
 * A template representing a TDLS channel-switch request or response frame
 *
 * @switch_time_offset: offset to the channel switch timing IE in the template
 * @tx_cmd: Tx parameters for the frame
 * @data: frame data
 */
struct iwl_tdls_channel_switch_frame {
	__le32 switch_time_offset;
	struct iwl_tx_cmd tx_cmd;
	u8 data[IWL_TDLS_CH_SW_FRAME_MAX_SIZE];
} __packed; /* TDLS_STA_CHANNEL_SWITCH_FRAME_API_S_VER_1 */

/**
 * TDLS channel switch command
 *
 * The command is sent to initiate a channel switch and also in response to
 * incoming TDLS channel-switch request/response packets from remote peers.
 *
 * @switch_type: see &enum iwl_tdls_channel_switch_type
 * @peer_sta_id: station id of TDLS peer
 * @ci: channel we switch to
 * @timing: timing related data for command
 * @frame: channel-switch request/response template, depending to switch_type
 */
struct iwl_tdls_channel_switch_cmd {
	u8 switch_type;
	__le32 peer_sta_id;
	struct iwl_fw_channel_info ci;
	struct iwl_tdls_channel_switch_timing timing;
	struct iwl_tdls_channel_switch_frame frame;
} __packed; /* TDLS_STA_CHANNEL_SWITCH_CMD_API_S_VER_1 */

/**
 * TDLS channel switch start notification
 *
 * @status: non-zero on success
 * @offchannel_duration: duration given in microseconds
 * @sta_id: peer currently performing the channel-switch with
 */
struct iwl_tdls_channel_switch_notif {
	__le32 status;
	__le32 offchannel_duration;
	__le32 sta_id;
} __packed; /* TDLS_STA_CHANNEL_SWITCH_NTFY_API_S_VER_1 */

/**
 * TDLS station info
 *
 * @sta_id: station id of the TDLS peer
 * @tx_to_peer_tid: TID reserved vs. the peer for FW based Tx
 * @tx_to_peer_ssn: initial SSN the FW should use for Tx on its TID vs the peer
 * @is_initiator: 1 if the peer is the TDLS link initiator, 0 otherwise
 */
struct iwl_tdls_sta_info {
	u8 sta_id;
	u8 tx_to_peer_tid;
	__le16 tx_to_peer_ssn;
	__le32 is_initiator;
} __packed; /* TDLS_STA_INFO_VER_1 */

/**
 * TDLS basic config command
 *
 * @id_and_color: MAC id and color being configured
 * @tdls_peer_count: amount of currently connected TDLS peers
 * @tx_to_ap_tid: TID reverved vs. the AP for FW based Tx
 * @tx_to_ap_ssn: initial SSN the FW should use for Tx on its TID vs. the AP
 * @sta_info: per-station info. Only the first tdls_peer_count entries are set
 * @pti_req_data_offset: offset of network-level data for the PTI template
 * @pti_req_tx_cmd: Tx parameters for PTI request template
 * @pti_req_template: PTI request template data
 */
struct iwl_tdls_config_cmd {
	__le32 id_and_color; /* mac id and color */
	u8 tdls_peer_count;
	u8 tx_to_ap_tid;
	__le16 tx_to_ap_ssn;
	struct iwl_tdls_sta_info sta_info[IWL_MVM_TDLS_STA_COUNT];

	__le32 pti_req_data_offset;
	struct iwl_tx_cmd pti_req_tx_cmd;
	u8 pti_req_template[0];
} __packed; /* TDLS_CONFIG_CMD_API_S_VER_1 */

/**
 * TDLS per-station config information from FW
 *
 * @sta_id: station id of the TDLS peer
 * @tx_to_peer_last_seq: last sequence number used by FW during FW-based Tx to
 *	the peer
 */
struct iwl_tdls_config_sta_info_res {
	__le16 sta_id;
	__le16 tx_to_peer_last_seq;
} __packed; /* TDLS_STA_INFO_RSP_VER_1 */

/**
 * TDLS config information from FW
 *
 * @tx_to_ap_last_seq: last sequence number used by FW during FW-based Tx to AP
 * @sta_info: per-station TDLS config information
 */
struct iwl_tdls_config_res {
	__le32 tx_to_ap_last_seq;
	struct iwl_tdls_config_sta_info_res sta_info[IWL_MVM_TDLS_STA_COUNT];
} __packed; /* TDLS_CONFIG_RSP_API_S_VER_1 */

#define TX_FIFO_MAX_NUM		8
#define RX_FIFO_MAX_NUM		2

/**
 * Shared memory configuration information from the FW
 *
 * @shared_mem_addr: shared memory addr (pre 8000 HW set to 0x0 as MARBH is not
 *	accessible)
 * @shared_mem_size: shared memory size
 * @sample_buff_addr: internal sample (mon/adc) buff addr (pre 8000 HW set to
 *	0x0 as accessible only via DBGM RDAT)
 * @sample_buff_size: internal sample buff size
 * @txfifo_addr: start addr of TXF0 (excluding the context table 0.5KB), (pre
 *	8000 HW set to 0x0 as not accessible)
 * @txfifo_size: size of TXF0 ... TXF7
 * @rxfifo_size: RXF1, RXF2 sizes. If there is no RXF2, it'll have a value of 0
 * @page_buff_addr: used by UMAC and performance debug (page miss analysis),
 *	when paging is not supported this should be 0
 * @page_buff_size: size of %page_buff_addr
 */
struct iwl_shared_mem_cfg {
	__le32 shared_mem_addr;
	__le32 shared_mem_size;
	__le32 sample_buff_addr;
	__le32 sample_buff_size;
	__le32 txfifo_addr;
	__le32 txfifo_size[TX_FIFO_MAX_NUM];
	__le32 rxfifo_size[RX_FIFO_MAX_NUM];
	__le32 page_buff_addr;
	__le32 page_buff_size;
} __packed; /* SHARED_MEM_ALLOC_API_S_VER_1 */

#endif /* __fw_api_h__ */
