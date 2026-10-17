/* SPDX-License-Identifier:    GPL-2.0
 *
 * Copyright (C) 2018 Marvell International Ltd.
 */

#ifndef __CGX_INTF_H__
#define __CGX_INTF_H__

#define CGX_FIRMWARE_MAJOR_VER		1
#define CGX_FIRMWARE_MINOR_VER		0

/* Register offsets */
#define CGX_CMR_INT		0x87e0e0000040
#define CGX_CMR_SCRATCH0	0x87e0e0001050
#define CGX_CMR_SCRATCH1	0x87e0e0001058

#define CGX_SHIFT(x)		(0x1000000 * ((x) & 0x3))
#define CMR_SHIFT(x)		(0x40000 * ((x) & 0x3))

/* CGX error types. set for cmd response status as CGX_STAT_FAIL */
enum cgx_error_type {
	CGX_ERR_NONE,
	CGX_ERR_LMAC_NOT_ENABLED,
	CGX_ERR_LMAC_MODE_INVALID,
	CGX_ERR_REQUEST_ID_INVALID,
	CGX_ERR_PREV_ACK_NOT_CLEAR,
	CGX_ERR_PHY_LINK_DOWN,
	CGX_ERR_PCS_RESET_FAIL,
	CGX_ERR_AN_CPT_FAIL,
	CGX_ERR_TX_NOT_IDLE,
	CGX_ERR_RX_NOT_IDLE,
	CGX_ERR_SPUX_BR_BLKLOCK_FAIL,
	CGX_ERR_SPUX_RX_ALIGN_FAIL,
	CGX_ERR_SPUX_TX_FAULT,
	CGX_ERR_SPUX_RX_FAULT,
	CGX_ERR_SPUX_RESET_FAIL,
	CGX_ERR_SPUX_AN_RESET_FAIL,
	CGX_ERR_SPUX_USX_AN_RESET_FAIL,
	CGX_ERR_SMUX_RX_LINK_NOT_OK,
	CGX_ERR_PCS_LINK_FAIL,
	CGX_ERR_TRAINING_FAIL,
	CGX_ERR_RX_EQU_FAIL,
	CGX_ERR_SPUX_BER_FAIL,
	CGX_ERR_SPUX_RSFEC_ALGN_FAIL,
	CGX_ERR_SPUX_MARKER_LOCK_FAIL,
	CGX_ERR_SET_FEC_INVALID,
	CGX_ERR_SET_FEC_FAIL,
	CGX_ERR_MODULE_INVALID,
	CGX_ERR_MODULE_NOT_PRESENT,
	CGX_ERR_SPEED_CHANGE_INVALID,	/* = 28 */
	/* FIXME : add more error types when adding support for new modes */
};

/* LINK speed types */
enum cgx_link_speed {
	CGX_LINK_NONE,
	CGX_LINK_10M,
	CGX_LINK_100M,
	CGX_LINK_1G,
	CGX_LINK_2HG,	/* 2.5 Gbps */
	CGX_LINK_5G,
	CGX_LINK_10G,
	CGX_LINK_20G,
	CGX_LINK_25G,
	CGX_LINK_40G,
	CGX_LINK_50G,
	CGX_LINK_80G,
	CGX_LINK_100G,
	CGX_LINK_MAX,
};

/* REQUEST ID types. Input to firmware */
enum cgx_cmd_id {
	CGX_CMD_NONE = 0,
	CGX_CMD_GET_FW_VER,
	CGX_CMD_GET_MAC_ADDR,
	CGX_CMD_SET_MTU,
	CGX_CMD_GET_LINK_STS,		/* optional to user */
	CGX_CMD_LINK_BRING_UP,		/* = 5 */
	CGX_CMD_LINK_BRING_DOWN,
	CGX_CMD_INTERNAL_LBK,
	CGX_CMD_EXTERNAL_LBK,
	CGX_CMD_HIGIG,
	CGX_CMD_LINK_STAT_CHANGE,	/* = 10 */
	CGX_CMD_MODE_CHANGE,		/* hot plug support */
	CGX_CMD_INTF_SHUTDOWN,
	CGX_CMD_GET_MKEX_SIZE,
	CGX_CMD_GET_MKEX_PROFILE,
	CGX_CMD_GET_FWD_BASE,		/* get base address of shared FW data */
	CGX_CMD_GET_LINK_MODES,		/* Supported Link Modes */
	CGX_CMD_SET_LINK_MODE,
	CGX_CMD_GET_SUPPORTED_FEC,
	CGX_CMD_SET_FEC,
	CGX_CMD_GET_AN,			/* = 20 */
	CGX_CMD_SET_AN,
	CGX_CMD_GET_ADV_LINK_MODES,
	CGX_CMD_GET_ADV_FEC,
	CGX_CMD_GET_PHY_MOD_TYPE, /* line-side modulation type: NRZ or PAM4 */
	CGX_CMD_SET_PHY_MOD_TYPE,	/* = 25 */
	CGX_CMD_PRBS,
	CGX_CMD_DISPLAY_EYE,
	CGX_CMD_GET_PHY_FEC_STATS,
	CGX_CMD_DISPLAY_SERDES,
	CGX_CMD_AN_LOOPBACK,	/* = 30 */
	CGX_CMD_GET_PERSIST_IGNORE,
	CGX_CMD_SET_PERSIST_IGNORE,
	CGX_CMD_SET_MAC_ADDR,
};

/* async event ids */
enum cgx_evt_id {
	CGX_EVT_NONE,
	CGX_EVT_LINK_CHANGE,
};

/* event types - cause of interrupt */
enum cgx_evt_type {
	CGX_EVT_ASYNC,
	CGX_EVT_CMD_RESP
};

enum cgx_stat {
	CGX_STAT_SUCCESS,
	CGX_STAT_FAIL
};

enum cgx_cmd_own {
	/* default ownership with kernel/uefi/u-boot */
	CGX_OWN_NON_SECURE_SW,
	/* set by kernel/uefi/u-boot after posting a new request to ATF */
	CGX_OWN_FIRMWARE,
};

/* Supported LINK MODE enums
 * Each link mode is a bit mask of these
 * enums which are represented as bits
 */
enum cgx_mode_t {
	CGX_MODE_SGMII_BIT = 0,
	CGX_MODE_1000_BASEX_BIT,
	CGX_MODE_QSGMII_BIT,
	CGX_MODE_10G_C2C_BIT,
	CGX_MODE_10G_C2M_BIT,
	CGX_MODE_10G_KR_BIT,
	CGX_MODE_20G_C2C_BIT,
	CGX_MODE_25G_C2C_BIT,
	CGX_MODE_25G_C2M_BIT,
	CGX_MODE_25G_2_C2C_BIT,
	CGX_MODE_25G_CR_BIT,
	CGX_MODE_25G_KR_BIT,
	CGX_MODE_40G_C2C_BIT,
	CGX_MODE_40G_C2M_BIT,
	CGX_MODE_40G_CR4_BIT,
	CGX_MODE_40G_KR4_BIT,
	CGX_MODE_40GAUI_C2C_BIT,
	CGX_MODE_50G_C2C_BIT,
	CGX_MODE_50G_C2M_BIT,
	CGX_MODE_50G_4_C2C_BIT,
	CGX_MODE_50G_CR_BIT,
	CGX_MODE_50G_KR_BIT,
	CGX_MODE_80GAUI_C2C_BIT,
	CGX_MODE_100G_C2C_BIT,
	CGX_MODE_100G_C2M_BIT,
	CGX_MODE_100G_CR4_BIT,
	CGX_MODE_100G_KR4_BIT,
	CGX_MODE_MAX_BIT /* = 29 */
};

/* scratchx(0) CSR used for ATF->non-secure SW communication.
 * This acts as the status register
 * Provides details on command ack/status, link status, error details
 */

/* CAUTION : below structures are placed in order based on the bit positions
 * For any updates/new bitfields, corresponding structures needs to be updated
 */
struct cgx_evt_sts_s {			/* start from bit 0 */
	u64 ack:1;
	u64 evt_type:1;		/* cgx_evt_type */
	u64 stat:1;		/* cgx_stat */
	u64 id:6;			/* cgx_evt_id/cgx_cmd_id */
	u64 reserved:55;
};

/* all the below structures are in the same memory location of SCRATCHX(0)
 * value can be read/written based on command ID
 */

/* Resp to command IDs with command status as CGX_STAT_FAIL
 * Not applicable for commands :
 *	CGX_CMD_LINK_BRING_UP/DOWN/CGX_EVT_LINK_CHANGE
 *	check struct cgx_lnk_sts_s comments
 */
struct cgx_err_sts_s {			/* start from bit 9 */
	u64 reserved1:9;
	u64 type:10;		/* cgx_error_type */
	u64 reserved2:35;
};

/* Resp to cmd ID as CGX_CMD_GET_FW_VER with cmd status as CGX_STAT_SUCCESS */
struct cgx_ver_s {			/* start from bit 9 */
	u64 reserved1:9;
	u64 major_ver:4;
	u64 minor_ver:4;
	u64 reserved2:47;
};

/* Resp to cmd ID as CGX_CMD_GET_MAC_ADDR with cmd status as CGX_STAT_SUCCESS
 * Returns each byte of MAC address in a separate bit field
 */
struct cgx_mac_addr_s {			/* start from bit 9 */
	u64 reserved1:9;
	u64 addr_0:8;
	u64 addr_1:8;
	u64 addr_2:8;
	u64 addr_3:8;
	u64 addr_4:8;
	u64 addr_5:8;
	u64 reserved2:7;
};

/* Resp to cmd ID - CGX_CMD_LINK_BRING_UP/DOWN, event ID CGX_EVT_LINK_CHANGE
 * status can be either CGX_STAT_FAIL or CGX_STAT_SUCCESS
 * In case of CGX_STAT_FAIL, it indicates CGX configuration failed when
 * processing link up/down/change command. Both err_type and current link status
 * will be updated
 * In case of CGX_STAT_SUCCESS, err_type will be CGX_ERR_NONE and current
 * link status will be updated
 */
struct cgx_lnk_sts_s {
	u64 reserved1:9;
	u64 link_up:1;
	u64 full_duplex:1;
	u64 speed:4;	/* cgx_link_speed */
	u64 err_type:10;
	u64 an:1;		/* Current AN state : enabled/disabled */
	u64 fec:2;		/* Current FEC type if enabled, if not 0 */
	u64 port:8;	/* Share the current port info if required */
	u64 mode:8;	/* cgx_mode_t enum integer value */
	u64 reserved2:20;
};

struct sh_fwd_base_s {
	u64 reserved1:9;
	u64 addr:55;
};

struct cgx_link_modes_s {
	u64 reserved1:9;
	u64 modes:55;
};

/* Resp to cmd ID - CGX_CMD_GET_ADV_FEC/CGX_CMD_GET_SUPPORTED_FEC
 * fec : 2 bits
 * typedef enum cgx_fec_type {
 *     CGX_FEC_NONE,
 *     CGX_FEC_BASE_R,
 *     CGX_FEC_RS
 * } fec_type_t;
 */
struct cgx_fec_types_s {
	u64 reserved1:9;
	u64 fec:2;
	u64 reserved2:53;
};

/* Resp to cmd ID - CGX_CMD_GET_AN */
struct cgx_get_an_s {
	u64 reserved1:9;
	u64 an:1;
	u64 reserved2:54;
};

/* Resp to cmd ID - CGX_CMD_GET_PHY_MOD_TYPE */
struct cgx_get_phy_mod_type_s {
	u64 reserved1:9;
	u64 mod:1;		/* 0=NRZ, 1=PAM4 */
	u64 reserved2:54;
};

/* Resp to cmd ID - CGX_CMD_GET_PERSIST_IGNORE */
struct cgx_get_flash_ignore_s {
	uint64_t reserved1:9;
	uint64_t ignore:1;
	uint64_t reserved2:54;
};

union cgx_rsp_sts {
	/* Fixed, applicable for all commands/events */
	struct cgx_evt_sts_s evt_sts;
	/* response to CGX_CMD_LINK_BRINGUP/DOWN/LINK_CHANGE */
	struct cgx_lnk_sts_s link_sts;
	/* response to CGX_CMD_GET_FW_VER */
	struct cgx_ver_s ver;
	/* response to CGX_CMD_GET_MAC_ADDR */
	struct cgx_mac_addr_s mac_s;
	/* response to CGX_CMD_GET_FWD_BASE */
	struct sh_fwd_base_s fwd_base_s;
	/* response if evt_status = CMD_FAIL */
	struct cgx_err_sts_s err;
	/* response to CGX_CMD_GET_SUPPORTED_FEC */
	struct cgx_fec_types_s supported_fec;
	/* response to CGX_CMD_GET_LINK_MODES */
	struct cgx_link_modes_s supported_modes;
	/* response to CGX_CMD_GET_ADV_LINK_MODES */
	struct cgx_link_modes_s adv_modes;
	/* response to CGX_CMD_GET_ADV_FEC */
	struct cgx_fec_types_s adv_fec;
	/* response to CGX_CMD_GET_AN */
	struct cgx_get_an_s an;
	/* response to CGX_CMD_GET_PHY_MOD_TYPE */
	struct cgx_get_phy_mod_type_s phy_mod_type;
	/* response to CGX_CMD_GET_PERSIST_IGNORE */
	struct cgx_get_flash_ignore_s persist;
#ifdef NT_FW_CONFIG
	/* response to CGX_CMD_GET_MKEX_SIZE */
	struct cgx_mcam_profile_sz_s prfl_sz;
	/* response to CGX_CMD_GET_MKEX_PROFILE */
	struct cgx_mcam_profile_addr_s prfl_addr;
#endif
};

union cgx_scratchx0 {
	u64 u;
	union cgx_rsp_sts s;
};

/* scratchx(1) CSR used for non-secure SW->ATF communication
 * This CSR acts as a command register
 */
struct cgx_cmd {			/* start from bit 2 */
	u64 reserved1:2;
	u64 id:6;			/* cgx_request_id */
	u64 reserved2:56;
};

/* all the below structures are in the same memory location of SCRATCHX(1)
 * corresponding arguments for command Id needs to be updated
 */

/* Any command using enable/disable as an argument need
 * to pass the option via this structure.
 * Ex: Loopback, HiGig...
 */
struct cgx_ctl_args {			/* start from bit 8 */
	u64 reserved1:8;
	u64 enable:1;
	u64 reserved2:55;
};

/* command argument to be passed for cmd ID - CGX_CMD_SET_MTU */
struct cgx_mtu_args {
	u64 reserved1:8;
	u64 size:16;
	u64 reserved2:40;
};

/* command argument to be passed for cmd ID - CGX_CMD_MODE_CHANGE */
struct cgx_mode_change_args {
	uint64_t reserved1:8;
	uint64_t speed:4; /* cgx_link_speed enum */
	uint64_t duplex:1; /* 0 - full duplex, 1 - half duplex */
	uint64_t an:1;	/* 0 - disable AN, 1 - enable AN */
	uint64_t port:8; /* device port */
	uint64_t mode:42;
};

/* command argument to be passed for cmd ID - CGX_CMD_LINK_CHANGE */
struct cgx_link_change_args {		/* start from bit 8 */
	u64 reserved1:8;
	u64 link_up:1;
	u64 full_duplex:1;
	u64 speed:4;		/* cgx_link_speed */
	u64 reserved2:50;
};

/* command argument to be passed for cmd ID - CGX_CMD_SET_LINK_MODE */
struct cgx_set_mode_args {
	u64 reserved1:8;
	u64 mode:56;
};

/* command argument to be passed for cmd ID - CGX_CMD_SET_FEC */
struct cgx_set_fec_args {
	u64 reserved1:8;
	u64 fec:2;
	u64 reserved2:54;
};

/* command argument to be passed for cmd ID - CGX_CMD_SET_PHY_MOD_TYPE */
struct cgx_set_phy_mod_args {
	u64 reserved1:8;
	u64 mod:1;		/* 0=NRZ, 1=PAM4 */
	u64 reserved2:55;
};

/* command argument to be passed for cmd ID - CGX_CMD_SET_PERSIST_IGNORE */
struct cgx_set_flash_ignore_args {
	uint64_t reserved1:8;
	uint64_t ignore:1;
	uint64_t reserved2:55;
};

/* command argument to be passed for cmd ID - CGX_CMD_SET_MAC_ADDR */
struct cgx_mac_addr_args {
	uint64_t reserved1:8;
	uint64_t addr:48;
	uint64_t pf_id:8;
};

struct cgx_prbs_args {
	u64 reserved1:8; /* start from bit 8 */
	u64 lane:8;
	u64 qlm:8;
	u64 stop_on_error:1;
	u64 mode:8;
	u64 time:31;
};

struct cgx_display_eye_args {
	u64 reserved1:8; /* start from bit 8 */
	u64 qlm:8;
	u64 lane:47;
};

union cgx_cmd_s {
	u64 own_status:2;			/* cgx_cmd_own */
	struct cgx_cmd cmd;
	struct cgx_ctl_args cmd_args;
	struct cgx_mtu_args mtu_size;
	struct cgx_link_change_args lnk_args; /* Input to CGX_CMD_LINK_CHANGE */
	struct cgx_set_mode_args mode_args;
	struct cgx_mode_change_args mode_change_args;
	struct cgx_set_fec_args fec_args;
	struct cgx_set_phy_mod_args phy_mod_args;
	struct cgx_set_flash_ignore_args persist_args;
	struct cgx_mac_addr_args mac_args;
	/* any other arg for command id * like : mtu, dmac filtering control */
	struct cgx_prbs_args prbs_args;
	struct cgx_display_eye_args dsp_eye_args;
};

union cgx_scratchx1 {
	u64 u;
	union cgx_cmd_s s;
};

#endif /* __CGX_INTF_H__ */
