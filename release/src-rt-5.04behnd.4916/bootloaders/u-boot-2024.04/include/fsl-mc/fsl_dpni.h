/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2013-2016 Freescale Semiconductor, Inc.
 * Copyright 2017, 2023 NXP
 */
#ifndef _FSL_DPNI_H
#define _FSL_DPNI_H

/* DPNI Version */
#define DPNI_VER_MAJOR				7
#define DPNI_VER_MINOR				3

/* Command IDs */
#define DPNI_CMDID_OPEN				0x8011
#define DPNI_CMDID_CLOSE			0x8001
#define DPNI_CMDID_CREATE			0x9011
#define DPNI_CMDID_DESTROY			0x9811
#define DPNI_CMDID_GET_API_VERSION              0xa011

#define DPNI_CMDID_ENABLE			0x0021
#define DPNI_CMDID_DISABLE			0x0031
#define DPNI_CMDID_GET_ATTR			0x0041
#define DPNI_CMDID_RESET			0x0051

#define DPNI_CMDID_SET_POOLS			0x2002
#define DPNI_CMDID_SET_BUFFER_LAYOUT		0x2651

#define DPNI_CMDID_GET_QDID			0x2101
#define DPNI_CMDID_GET_TX_DATA_OFFSET		0x2121
#define DPNI_CMDID_GET_LINK_STATE		0x2151
#define DPNI_CMDID_SET_LINK_CFG			0x21A1

#define DPNI_CMDID_ADD_MAC_ADDR			0x2261

#define DPNI_CMDID_GET_STATISTICS		0x25D1
#define DPNI_CMDID_GET_QUEUE			0x25F1
#define DPNI_CMDID_SET_QUEUE			0x2601
#define DPNI_CMDID_SET_TX_CONFIRMATION_MODE	0x2661

/* Macros for accessing command fields smaller than 1byte */
#define DPNI_MASK(field)	\
	GENMASK(DPNI_##field##_SHIFT + DPNI_##field##_SIZE - 1, \
		DPNI_##field##_SHIFT)
#define dpni_set_field(var, field, val)	\
	((var) |= (((val) << DPNI_##field##_SHIFT) & DPNI_MASK(field)))
#define dpni_get_field(var, field)	\
	(((var) & DPNI_MASK(field)) >> DPNI_##field##_SHIFT)

#pragma pack(push, 1)
struct dpni_cmd_open {
	__le32 dpni_id;
};

struct dpni_cmd_create {
	__le32 options;
	u8 num_queues;
	u8 num_tcs;
	u8 mac_filter_entries;
	u8 num_channels;
	u8 vlan_filter_entries;
	u8 pad2;
	u8 qos_entries;
	u8 pad3;
	__le16 fs_entries;
	u8 num_rx_tcs;
	u8 pad4;
	u8  num_cgs;
	__le16 num_opr;
	u8 dist_key_size;
};

struct dpni_cmd_destroy {
	__le32 dpni_id;
};

#define DPNI_BACKUP_POOL(val, order)	(((val) & 0x1) << (order))

struct dpni_cmd_pool {
	__le16 dpbp_id;
	u8 priority_mask;
	u8 pad;
};

struct dpni_cmd_set_pools {
	u8 num_dpbp;
	u8 backup_pool_mask;
	u8 pad;
	u8 pool_options;
	struct dpni_cmd_pool pool[8];
	__le16 buffer_size[8];
};

struct dpni_rsp_get_attr {
	/* response word 0 */
	__le32 options;
	u8 num_queues;
	u8 num_rx_tcs;
	u8 mac_filter_entries;
	u8 num_tx_tcs;
	/* response word 1 */
	u8 vlan_filter_entries;
	u8 num_channels;
	u8 qos_entries;
	u8 pad2;
	__le16 fs_entries;
	__le16 num_opr;
	/* response word 2 */
	u8 qos_key_size;
	u8 fs_key_size;
	__le16 wriop_version;
	u8 num_cgs;
};

/* There are 3 separate commands for configuring Rx, Tx and Tx confirmation
 * buffer layouts, but they all share the same parameters.
 * If one of the functions changes, below structure needs to be split.
 */

#define DPNI_PASS_TS_SHIFT		0
#define DPNI_PASS_TS_SIZE		1
#define DPNI_PASS_PR_SHIFT		1
#define DPNI_PASS_PR_SIZE		1
#define DPNI_PASS_FS_SHIFT		2
#define DPNI_PASS_FS_SIZE		1
#define DPNI_PASS_SWO_SHIFT		3
#define DPNI_PASS_SWO_SIZE		1

struct dpni_cmd_set_buffer_layout {
	/* cmd word 0 */
	u8 qtype;
	u8 pad0[3];
	__le16 options;
	/* from LSB: pass_timestamp:1, parser_result:1, frame_status:1 */
	u8 flags;
	u8 pad1;
	/* cmd word 1 */
	__le16 private_data_size;
	__le16 data_align;
	__le16 head_room;
	__le16 tail_room;
};

struct dpni_cmd_get_qdid {
	u8 qtype;
};

struct dpni_rsp_get_qdid {
	__le16 qdid;
};

struct dpni_rsp_get_tx_data_offset {
	__le16 data_offset;
};

struct dpni_cmd_set_link_cfg {
	__le64 pad0;
	__le32 rate;
	__le32 pad1;
	__le64 options;
	__le64 advertising;
};

#define DPNI_LINK_STATE_SHIFT		0
#define DPNI_LINK_STATE_SIZE		1
#define DPNI_STATE_VALID_SHIFT		1
#define DPNI_STATE_VALID_SIZE		1

struct dpni_rsp_get_link_state {
	__le32 pad0;
	/* from LSB: up:1 */
	u8 flags;
	u8 pad1[3];
	__le32 rate;
	__le32 pad2;
	__le64 options;
	__le64 supported;
	__le64 advertising;
};

struct dpni_cmd_add_mac_addr {
	u8 flags;
	u8 pad;
	u8 mac_addr[6];
	u8 tc_id;
	u8 fq_id;
};

struct dpni_cmd_get_queue {
	u8 qtype;
	u8 tc;
	u8 index;
	u8 channel_id;
};

#define DPNI_DEST_TYPE_SHIFT		0
#define DPNI_DEST_TYPE_SIZE		4
#define DPNI_CGID_VALID_SHIFT		5
#define DPNI_CGID_VALID_SIZE		1
#define DPNI_STASH_CTRL_SHIFT		6
#define DPNI_STASH_CTRL_SIZE		1
#define DPNI_HOLD_ACTIVE_SHIFT		7
#define DPNI_HOLD_ACTIVE_SIZE		1

struct dpni_rsp_get_queue {
	/* response word 0 */
	__le64 pad0;
	/* response word 1 */
	__le32 dest_id;
	__le16 pad1;
	u8 dest_prio;
	/* From LSB: dest_type:4, pad:1, cgid_valid:1, flc_stash_ctrl:1, hold_active:1 */
	u8 flags;
	/* response word 2 */
	__le64 flc;
	/* response word 3 */
	__le64 user_context;
	/* response word 4 */
	__le32 fqid;
	__le16 qdbin;
	__le16 pad2;
	/* response word 5*/
	u8 cgid;
};

struct dpni_cmd_set_queue {
	/* cmd word 0 */
	u8 qtype;
	u8 tc;
	u8 index;
	u8 options;
	__le32 pad0;
	/* cmd word 1 */
	__le32 dest_id;
	__le16 pad1;
	u8 dest_prio;
	u8 flags;
	/* cmd word 2 */
	__le64 flc;
	/* cmd word 3 */
	__le64 user_context;
	/* cmd word 4 */
	u8 cgid;
	u8 channel_id;
};

struct dpni_tx_confirmation_mode {
	u8 ceetm_ch_idx;
	u8 pad1;
	__le16 pad2;
	u8 confirmation_mode;
};

struct dpni_cmd_get_statistics {
	u8 page_number;
	__le16 param;
};

struct dpni_rsp_get_statistics {
	__le64 counter[7];
};

#pragma pack(pop)

/**
 * Data Path Network Interface API
 * Contains initialization APIs and runtime control APIs for DPNI
 */

struct fsl_mc_io;

/* General DPNI macros */

/* Maximum number of traffic classes */
#define DPNI_MAX_TC				8
/* Maximum number of buffer pools per DPNI */
#define DPNI_MAX_DPBP				8

/* All traffic classes considered; see dpni_set_rx_flow() */
#define DPNI_ALL_TCS				(u8)(-1)
/* All flows within traffic class considered; see dpni_set_rx_flow() */
#define DPNI_ALL_TC_FLOWS			(u16)(-1)
/* Generate new flow ID; see dpni_set_tx_flow() */
#define DPNI_NEW_FLOW_ID			(u16)(-1)
/* use for common tx-conf queue; see dpni_set_tx_conf_<x>() */
#define DPNI_COMMON_TX_CONF			(u16)(-1)

int dpni_open(struct fsl_mc_io *mc_io, u32 cmd_flags, int dpni_id, u16 *token);

int dpni_close(struct fsl_mc_io *mc_io, u32 cmd_flags, u16 token);

/* DPNI configuration options */

/**
 * Allow different distribution key profiles for different traffic classes;
 * if not set, a single key profile is assumed
 */
#define DPNI_OPT_ALLOW_DIST_KEY_PER_TC		0x00000001

/**
 * Disable all non-error transmit confirmation; error frames are reported
 * back to a common Tx error queue
 */
#define DPNI_OPT_TX_CONF_DISABLED		0x00000002

/* Disable per-sender private Tx confirmation/error queue */
#define DPNI_OPT_PRIVATE_TX_CONF_ERROR_DISABLED	0x00000004

/**
 * Support distribution based on hashed key;
 * allows statistical distribution over receive queues in a traffic class
 */
#define DPNI_OPT_DIST_HASH			0x00000010

/**
 * Support distribution based on flow steering;
 * allows explicit control of distribution over receive queues in a traffic
 * class
 */
#define DPNI_OPT_DIST_FS			0x00000020

/* Unicast filtering support */
#define DPNI_OPT_UNICAST_FILTER			0x00000080
/* Multicast filtering support */
#define DPNI_OPT_MULTICAST_FILTER		0x00000100
/* VLAN filtering support */
#define DPNI_OPT_VLAN_FILTER			0x00000200
/* Support IP reassembly on received packets */
#define DPNI_OPT_IPR				0x00000800
/* Support IP fragmentation on transmitted packets */
#define DPNI_OPT_IPF				0x00001000
/* VLAN manipulation support */
#define DPNI_OPT_VLAN_MANIPULATION		0x00010000
/* Support masking of QoS lookup keys */
#define DPNI_OPT_QOS_MASK_SUPPORT		0x00020000
/* Support masking of Flow Steering lookup keys */
#define DPNI_OPT_FS_MASK_SUPPORT		0x00040000

/**
 * enum dpni_queue_type - Identifies a type of queue targeted by the command
 * @DPNI_QUEUE_RX: Rx queue
 * @DPNI_QUEUE_TX: Tx queue
 * @DPNI_QUEUE_TX_CONFIRM: Tx confirmation queue
 * @DPNI_QUEUE_RX_ERR: Rx error queue
 */
enum dpni_queue_type {
	DPNI_QUEUE_RX,
	DPNI_QUEUE_TX,
	DPNI_QUEUE_TX_CONFIRM,
	DPNI_QUEUE_RX_ERR,
};

/**
 * struct dpni_cfg - Structure representing DPNI configuration
 * @options: Any combination of the following options:
 *		DPNI_OPT_TX_FRM_RELEASE
 *		DPNI_OPT_NO_MAC_FILTER
 *		DPNI_OPT_HAS_POLICING
 *		DPNI_OPT_SHARED_CONGESTION
 *		DPNI_OPT_HAS_KEY_MASKING
 *		DPNI_OPT_NO_FS
 *		DPNI_OPT_SINGLE_SENDER
 *		DPNI_OPT_STASHING_DIS
 * @fs_entries: Number of entries in the flow steering table.
 *		This table is used to select the ingress queue for
 *		ingress traffic, targeting a GPP core or another.
 *		In addition it can be used to discard traffic that
 *		matches the set rule. It is either an exact match table
 *		or a TCAM table, depending on DPNI_OPT_ HAS_KEY_MASKING
 *		bit in OPTIONS field. This field is ignored if
 *		DPNI_OPT_NO_FS bit is set in OPTIONS field. Otherwise,
 *		value 0 defaults to 64. Maximum supported value is 1024.
 *		Note that the total number of entries is limited on the
 *		SoC to as low as 512 entries if TCAM is used.
 * @vlan_filter_entries: Number of entries in the VLAN address filtering
 *		table. This is an exact match table used to filter
 *		ingress traffic based on VLAN IDs. Value 0 disables VLAN
 *		filtering. Maximum supported value is 16.
 * @mac_filter_entries: Number of entries in the MAC address filtering
 *		table. This is an exact match table and allows both
 *		unicast and multicast entries. The primary MAC address
 *		of the network interface is not part of this table,
 *		this contains only entries in addition to it. This
 *		field is ignored if DPNI_OPT_ NO_MAC_FILTER is set in
 *		OPTIONS field. Otherwise, value 0 defaults to 80.
 *		Maximum supported value is 80.
 * @num_queues: Number of Tx and Rx queues used for traffic
 *		distribution. This is orthogonal to QoS and is only
 *		used to distribute traffic to multiple GPP cores.
 *		This configuration affects the number of Tx queues
 *		(logical FQs, all associated with a single CEETM queue),
 *		Rx queues and Tx confirmation queues, if applicable.
 *		Value 0 defaults to one queue. Maximum supported value
 *		is 8.
 * @num_tcs: Number of traffic classes (TCs), reserved for the DPNI.
 *		TCs can have different priority levels for the purpose
 *		of Tx scheduling (see DPNI_SET_TX_PRIORITIES), different
 *		BPs (DPNI_ SET_POOLS), policers. There are dedicated QM
 *		queues for traffic classes (including class queues on
 *		Tx). Value 0 defaults to one TC. Maximum supported value
 *		is 16. There are maximum 16 TCs for Tx and 8 TCs for Rx.
 *		When num_tcs>8 Tx will use this value but Rx will have
 *		only 8 traffic classes.
 * @num_rx_tcs: if set to other value than zero represents number
 *		of TCs used for Rx. Maximum value is 8. If set to zero the
 *		number of Rx TCs will be initialized with the value provided
 *		in num_tcs parameter.
 * @qos_entries: Number of entries in the QoS classification table. This
 *		table is used to select the TC for ingress traffic. It
 *		is either an exact match or a TCAM table, depending on
 *		DPNI_OPT_ HAS_KEY_MASKING bit in OPTIONS field. This
 *		field is ignored if the DPNI has a single TC. Otherwise,
 *		a value of 0 defaults to 64. Maximum supported value
 *		is 64.
 * @num_channels: Number of egress channels used by this dpni object. If
 *		set to zero the dpni object will use a single CEETM channel.
 */
struct dpni_cfg {
	u32 options;
	u16 fs_entries;
	u8  vlan_filter_entries;
	u8  mac_filter_entries;
	u8  num_queues;
	u8  num_tcs;
	u8  num_rx_tcs;
	u8  qos_entries;
	u8  num_cgs;
	u16 num_opr;
	u8  dist_key_size;
	u8  num_channels;
};

/**
 * dpni_prepare_cfg() - function prepare parameters
 * @cfg: cfg structure
 * @cfg_buf: Zeroed 256 bytes of memory before mapping it to DMA
 *
 * This function has to be called before dpni_create()
 */
int dpni_prepare_cfg(const struct dpni_cfg	*cfg,
		     u8			*cfg_buf);

int dpni_create(struct fsl_mc_io *mc_io, u16 dprc_token, u32 cmd_flags,
		const struct dpni_cfg *cfg, u32 *obj_id);

int dpni_destroy(struct fsl_mc_io *mc_io, u16 dprc_token, u32 cmd_flags,
		 u32 object_id);

/**
 * struct dpni_pools_cfg - Structure representing buffer pools configuration
 * @num_dpbp:	Number of DPBPs
 * @pool_options: Buffer assignment options
 *                This field is a combination of DPNI_POOL_ASSOC_flags
 * @pools:	Array of buffer pools parameters; The number of valid entries
 *		must match 'num_dpbp' value
 * @pools.dpbp_id:     DPBP object ID
 * @pools.priority:    Priority mask that indicates TC's used with this buffer.
 *		       I set to 0x00 MC will assume value 0xff.
 * @pools.buffer_size: Buffer size
 * @pools.backup_pool: Backup pool
 */

#define DPNI_POOL_ASSOC_QPRI	0
#define DPNI_POOL_ASSOC_QDBIN	1

struct dpni_pools_cfg {
	u8 num_dpbp;
	u8 pool_options;
	struct {
		int		dpbp_id;
		u8		priority_mask;
		u16	buffer_size;
		int		backup_pool;
	} pools[DPNI_MAX_DPBP];
};

int dpni_set_pools(struct fsl_mc_io *mc_io, u32 cmd_flags, u16 token,
		   const struct dpni_pools_cfg *cfg);

int dpni_enable(struct fsl_mc_io *mc_io, u32 cmd_flags, u16 token);

int dpni_disable(struct fsl_mc_io *mc_io, u32 cmd_flags, u16 token);

int dpni_reset(struct fsl_mc_io *mc_io, u32 cmd_flags, u16 token);

/**
 * struct dpni_attr - Structure representing DPNI attributes
 * @options: Any combination of the following options:
 *		DPNI_OPT_TX_FRM_RELEASE
 *		DPNI_OPT_NO_MAC_FILTER
 *		DPNI_OPT_HAS_POLICING
 *		DPNI_OPT_SHARED_CONGESTION
 *		DPNI_OPT_HAS_KEY_MASKING
 *		DPNI_OPT_NO_FS
 *		DPNI_OPT_STASHING_DIS
 * @num_queues: Number of Tx and Rx queues used for traffic distribution.
 * @num_rx_tcs: Number of RX traffic classes (TCs), reserved for the DPNI.
 * @num_tx_tcs: Number of TX traffic classes (TCs), reserved for the DPNI.
 * @mac_filter_entries: Number of entries in the MAC address filtering
 *		table.
 * @vlan_filter_entries: Number of entries in the VLAN address filtering
 *		table.
 * @qos_entries: Number of entries in the QoS classification table.
 * @fs_entries: Number of entries in the flow steering table.
 * @qos_key_size: Size, in bytes, of the QoS look-up key. Defining a key larger
 *			than this when adding QoS entries will result
 *			in an error.
 * @fs_key_size: Size, in bytes, of the flow steering look-up key. Defining a
 *			key larger than this when composing the hash + FS key
 *			will result in an error.
 * @wriop_version: Version of WRIOP HW block.
 *			The 3 version values are stored on 6, 5, 5 bits
 *			respectively.
 *			Values returned:
 *			- 0x400 - WRIOP version 1.0.0, used on LS2080 and
 *			variants,
 *			- 0x421 - WRIOP version 1.1.1, used on LS2088 and
 *			variants,
 *			- 0x422 - WRIOP version 1.1.2, used on LS1088 and
 *			variants.
 *			- 0xC00 - WRIOP version 3.0.0, used on LX2160 and
 *			variants.
 */
struct dpni_attr {
	u32 options;
	u8  num_queues;
	u8  num_rx_tcs;
	u8  num_tx_tcs;
	u8  mac_filter_entries;
	u8  vlan_filter_entries;
	u8  qos_entries;
	u16 fs_entries;
	u16 num_opr;
	u8  qos_key_size;
	u8  fs_key_size;
	u16 wriop_version;
	u8  num_cgs;
	u8  num_channels;
};

int dpni_get_attributes(struct fsl_mc_io *mc_io, u32 cmd_flags, u16 token,
			struct dpni_attr *attr);

/* DPNI buffer layout modification options */

/* Select to modify the time-stamp setting */
#define DPNI_BUF_LAYOUT_OPT_TIMESTAMP		0x00000001
/* Select to modify the parser-result setting; not applicable for Tx */
#define DPNI_BUF_LAYOUT_OPT_PARSER_RESULT	0x00000002
/* Select to modify the frame-status setting */
#define DPNI_BUF_LAYOUT_OPT_FRAME_STATUS	0x00000004
/* Select to modify the private-data-size setting */
#define DPNI_BUF_LAYOUT_OPT_PRIVATE_DATA_SIZE	0x00000008
/* Select to modify the data-alignment setting */
#define DPNI_BUF_LAYOUT_OPT_DATA_ALIGN		0x00000010
/* Select to modify the data-head-room setting */
#define DPNI_BUF_LAYOUT_OPT_DATA_HEAD_ROOM	0x00000020
/*!< Select to modify the data-tail-room setting */
#define DPNI_BUF_LAYOUT_OPT_DATA_TAIL_ROOM	0x00000040
/* Select to modify the sw-opaque value setting */
#define DPNI_BUF_LAYOUT_OPT_SW_OPAQUE		0x00000080
/* Select to disable Scatter Gather and use single buffer */
#define DPNI_BUF_LAYOUT_OPT_NO_SG		0x00000100

/**
 * struct dpni_buffer_layout - Structure representing DPNI buffer layout
 * @options:		Flags representing the suggested modifications to the
 *			buffer layout;
 *			Use any combination of 'DPNI_BUF_LAYOUT_OPT_<X>' flags
 * @pass_timestamp:	Pass timestamp value
 * @pass_parser_result:	Pass parser results
 * @pass_frame_status:	Pass frame status
 * @private_data_size:	Size kept for private data (in bytes)
 * @data_align:		Data alignment
 * @data_head_room:	Data head room
 * @data_tail_room:	Data tail room
 */
struct dpni_buffer_layout {
	u32 options;
	int pass_timestamp;
	int pass_parser_result;
	int pass_frame_status;
	int pass_sw_opaque;
	u16 private_data_size;
	u16 data_align;
	u16 data_head_room;
	u16 data_tail_room;
};

int dpni_set_buffer_layout(struct fsl_mc_io *mc_io, u32 cmd_flags, u16 token,
			   enum dpni_queue_type qtype,
			   const struct dpni_buffer_layout *layout);

int dpni_get_qdid(struct fsl_mc_io *mc_io, u32 cmd_flags, u16 token,
		  enum dpni_queue_type qtype, u16 *qdid);

int dpni_get_tx_data_offset(struct fsl_mc_io *mc_io, u32 cmd_flags, u16 token,
			    u16 *data_offset);

/* Enable auto-negotiation */
#define DPNI_LINK_OPT_AUTONEG		0x0000000000000001ULL
/* Enable half-duplex mode */
#define DPNI_LINK_OPT_HALF_DUPLEX	0x0000000000000002ULL
/* Enable pause frames */
#define DPNI_LINK_OPT_PAUSE		0x0000000000000004ULL
/* Enable a-symmetric pause frames */
#define DPNI_LINK_OPT_ASYM_PAUSE	0x0000000000000008ULL

/**
 * struct - Structure representing DPNI link configuration
 * @rate: Rate
 * @options: Mask of available options; use 'DPNI_LINK_OPT_<X>' values
 * @advertising: Speeds that are advertised for autoneg (bitmap)
 */
struct dpni_link_cfg {
	u32 rate;
	u64 options;
	u64 advertising;
};

int dpni_set_link_cfg(struct fsl_mc_io *mc_io, u32 cmd_flags, u16 token,
		      const struct dpni_link_cfg *cfg);

/**
 * struct dpni_link_state - Structure representing DPNI link state
 * @rate:	Rate
 * @options:	Mask of available options; use 'DPNI_LINK_OPT_<X>' values
 * @up:		Link state; '0' for down, '1' for up
 * @state_valid: Ignore/Update the state of the link
 * @supported: Speeds capability of the phy (bitmap)
 * @advertising: Speeds that are advertised for autoneg (bitmap)
 */
struct dpni_link_state {
	u32 rate;
	u64 options;
	int up;
	int     state_valid;
	u64 supported;
	u64 advertising;
};

int dpni_get_link_state(struct fsl_mc_io *mc_io, u32 cmd_flags, u16 token,
			struct dpni_link_state *state);

int dpni_add_mac_addr(struct fsl_mc_io *mc_io, u32 cmd_flags, u16 token,
		      const u8 mac_addr[6], u8 flags,
		      u8 tc_id, u8 flow_id);

int dpni_get_api_version(struct fsl_mc_io *mc_io, u32 cmd_flags,
			 u16 *major_ver, u16 *minor_ver);

/**
 * enum dpni_dest - DPNI destination types
 * @DPNI_DEST_NONE: Unassigned destination; The queue is set in parked mode and
 *		does not generate FQDAN notifications; user is expected to
 *		dequeue from the queue based on polling or other user-defined
 *		method
 * @DPNI_DEST_DPIO: The queue is set in schedule mode and generates FQDAN
 *		notifications to the specified DPIO; user is expected to dequeue
 *		from the queue only after notification is received
 * @DPNI_DEST_DPCON: The queue is set in schedule mode and does not generate
 *		FQDAN notifications, but is connected to the specified DPCON
 *		object; user is expected to dequeue from the DPCON channel
 */
enum dpni_dest {
	DPNI_DEST_NONE = 0,
	DPNI_DEST_DPIO = 1,
	DPNI_DEST_DPCON = 2
};

/* DPNI Tx flow modification options */

/* Select to modify the settings for dedicate Tx confirmation/error */
#define DPNI_TX_FLOW_OPT_TX_CONF_ERROR	0x00000001
/*!< Select to modify the L3 checksum generation setting */
#define DPNI_TX_FLOW_OPT_L3_CHKSUM_GEN	0x00000010
/*!< Select to modify the L4 checksum generation setting */
#define DPNI_TX_FLOW_OPT_L4_CHKSUM_GEN	0x00000020

/**
 * enum dpni_confirmation_mode - Defines DPNI options supported for Tx
 * confirmation
 * @DPNI_CONF_AFFINE: For each Tx queue set associated with a sender there is
 * an affine Tx Confirmation queue
 * @DPNI_CONF_SINGLE: All Tx queues are associated with a single Tx
 * confirmation queue
 * @DPNI_CONF_DISABLE: Tx frames are not confirmed.  This must be associated
 * with proper FD set-up to have buffers release to a Buffer Pool, otherwise
 * buffers will be leaked
 */
enum dpni_confirmation_mode {
	DPNI_CONF_AFFINE,
	DPNI_CONF_SINGLE,
	DPNI_CONF_DISABLE,
};

/**
 * stashes the whole annotation area (up to 192 bytes)
 */
#define DPNI_FLC_STASH_FRAME_ANNOTATION	0x00000001

/**
 * struct dpni_queue - Queue structure
 * @destination - Destination structure
 * @destination.id:	ID of the destination, only relevant if DEST_TYPE is > 0.
 *	Identifies either a DPIO or a DPCON object.
 *	Not relevant for Tx queues.
 * @destination.type:	May be one of the following:
 *	0 - No destination, queue can be manually
 *		queried, but will not push traffic or
 *		notifications to a DPIO;
 *	1 - The destination is a DPIO. When traffic
 *		becomes available in the queue a FQDAN
 *		(FQ data available notification) will be
 *		generated to selected DPIO;
 *	2 - The destination is a DPCON. The queue is
 *		associated with a DPCON object for the
 *		purpose of scheduling between multiple
 *		queues. The DPCON may be independently
 *		configured to generate notifications.
 *		Not relevant for Tx queues.
 * @destination.hold_active: Hold active, maintains a queue scheduled for longer
 *	in a DPIO during dequeue to reduce spread of traffic.
 *	Only relevant if queues are
 *	not affined to a single DPIO.
 * @user_context: User data, presented to the user along with any frames
 *	from this queue. Not relevant for Tx queues.
 * @flc: FD FLow Context structure
 * @flc.value: Default FLC value for traffic dequeued from
 *      this queue.  Please check description of FD
 *      structure for more information.
 *      Note that FLC values set using dpni_add_fs_entry,
 *      if any, take precedence over values per queue.
 * @flc.stash_control: Boolean, indicates whether the 6 lowest
 *      - significant bits are used for stash control.
 *      significant bits are used for stash control.  If set, the 6
 *      least significant bits in value are interpreted as follows:
 *      - bits 0-1: indicates the number of 64 byte units of context
 *      that are stashed.  FLC value is interpreted as a memory address
 *      in this case, excluding the 6 LS bits.
 *      - bits 2-3: indicates the number of 64 byte units of frame
 *      annotation to be stashed.  Annotation is placed at FD[ADDR].
 *      - bits 4-5: indicates the number of 64 byte units of frame
 *      data to be stashed.  Frame data is placed at FD[ADDR] +
 *      FD[OFFSET].
 *      For more details check the Frame Descriptor section in the
 *      hardware documentation.
 *@cgid :indicate the cgid to set relative to dpni
 */
struct dpni_queue {
	struct {
		u16 id;
		enum dpni_dest type;
		char hold_active;
		u8 priority;
	} destination;
	u64 user_context;
	struct {
		u64 value;
		char stash_control;
	} flc;
	int cgid;
};

/**
 * struct dpni_queue_id - Queue identification, used for enqueue commands
 *				or queue control
 * @fqid:	FQID used for enqueueing to and/or configuration of this
 *			specific FQ
 * @qdbin:	Queueing bin, used to enqueue using QDID, DQBIN, QPRI.
 *			Only relevant for Tx queues.
 */
struct dpni_queue_id {
	u32 fqid;
	u16 qdbin;
};

int dpni_set_queue(struct fsl_mc_io *mc_io, u32 cmd_flags, u16 token,
		   enum dpni_queue_type qtype, u16 param, u8 index,
		   u8 options, const struct dpni_queue *queue);

int dpni_get_queue(struct fsl_mc_io *mc_io, u32 cmd_flags, u16 token,
		   enum dpni_queue_type qtype, u16 param, u8 index,
		   struct dpni_queue *queue, struct dpni_queue_id *qid);

int dpni_set_tx_confirmation_mode(struct fsl_mc_io *mc_io, u32 cmd_flags, u16 token,
				  u8 ceetm_ch_idx, enum dpni_confirmation_mode mode);

#define DPNI_STATISTICS_CNT		7

/**
 * union dpni_statistics - Union describing the DPNI statistics
 * @page_0: Page_0 statistics structure
 * @page_0.ingress_all_frames: Ingress frame count
 * @page_0.ingress_all_bytes: Ingress byte count
 * @page_0.ingress_multicast_frames: Ingress multicast frame count
 * @page_0.ingress_multicast_bytes: Ingress multicast byte count
 * @page_0.ingress_broadcast_frames: Ingress broadcast frame count
 * @page_0.ingress_broadcast_bytes: Ingress broadcast byte count
 * @page_1: Page_1 statistics structure
 * @page_1.egress_all_frames: Egress frame count
 * @page_1.egress_all_bytes: Egress byte count
 * @page_1.egress_multicast_frames: Egress multicast frame count
 * @page_1.egress_multicast_bytes: Egress multicast byte count
 * @page_1.egress_broadcast_frames: Egress broadcast frame count
 * @page_1.egress_broadcast_bytes: Egress broadcast byte count
 * @page_2: Page_2 statistics structure
 * @page_2.ingress_filtered_frames: Ingress filtered frame count
 * @page_2.ingress_discarded_frames: Ingress discarded frame count
 * @page_2.ingress_nobuffer_discards: Ingress discarded frame count due to
 *	lack of buffers
 * @page_2.egress_discarded_frames: Egress discarded frame count
 * @page_2.egress_confirmed_frames: Egress confirmed frame count
 * @page_3: Page_3 statistics structure
 * @page_3.egress_dequeue_bytes: Cumulative count of the number of bytes
 *	dequeued from egress FQs
 * @page_3.egress_dequeue_frames: Cumulative count of the number of frames
 *	dequeued from egress FQs
 * @page_3.egress_reject_bytes: Cumulative count of the number of bytes in
 *	egress frames whose enqueue was rejected
 * @page_3.egress_reject_frames: Cumulative count of the number of egress
 *	frames whose enqueue was rejected
 * @page_4: Page_4 statistics structure: congestion points
 * @page_4.cgr_reject_frames: number of rejected frames due to congestion point
 * @page_4.cgr_reject_bytes: number of rejected bytes due to congestion point
 * @page_5: Page_5 statistics structure: policer
 * @page_5.policer_cnt_red: NUmber of red colored frames
 * @page_5.policer_cnt_yellow: number of yellow colored frames
 * @page_5.policer_cnt_green: number of green colored frames
 * @page_5.policer_cnt_re_red: number of recolored red frames
 * @page_5.policer_cnt_re_yellow: number of recolored yellow frames
 * @page_6: Page_6 statistics structure
 * @page_6.tx_pending_frames: total number of frames pending in egress FQs
 * @raw: raw statistics structure, used to index counters
 */
union dpni_statistics {
	struct {
		u64 ingress_all_frames;
		u64 ingress_all_bytes;
		u64 ingress_multicast_frames;
		u64 ingress_multicast_bytes;
		u64 ingress_broadcast_frames;
		u64 ingress_broadcast_bytes;
	} page_0;
	struct {
		u64 egress_all_frames;
		u64 egress_all_bytes;
		u64 egress_multicast_frames;
		u64 egress_multicast_bytes;
		u64 egress_broadcast_frames;
		u64 egress_broadcast_bytes;
	} page_1;
	struct {
		u64 ingress_filtered_frames;
		u64 ingress_discarded_frames;
		u64 ingress_nobuffer_discards;
		u64 egress_discarded_frames;
		u64 egress_confirmed_frames;
	} page_2;
	struct {
		u64 egress_dequeue_bytes;
		u64 egress_dequeue_frames;
		u64 egress_reject_bytes;
		u64 egress_reject_frames;
	} page_3;
	struct {
		u64 cgr_reject_frames;
		u64 cgr_reject_bytes;
	} page_4;
	struct {
		u64 policer_cnt_red;
		u64 policer_cnt_yellow;
		u64 policer_cnt_green;
		u64 policer_cnt_re_red;
		u64 policer_cnt_re_yellow;
	} page_5;
	struct {
		u64 tx_pending_frames;
	} page_6;
	struct {
		u64 counter[DPNI_STATISTICS_CNT];
	} raw;
};

int dpni_get_statistics(struct fsl_mc_io *mc_io, u32 cmd_flags, u16 token,
			u8 page, u16 param, union dpni_statistics *stat);
#endif /* _FSL_DPNI_H */
