/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2013-2016 Freescale Semiconductor, Inc.
 * Copyright 2017 NXP
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
#define DPNI_CMDID_GET_BUFFER_LAYOUT		0x2641
#define DPNI_CMDID_SET_ERRORS_BEHAVIOR		0x20B1

#define DPNI_CMDID_GET_QDID			0x2101
#define DPNI_CMDID_GET_TX_DATA_OFFSET		0x2121
#define DPNI_CMDID_GET_LINK_STATE		0x2151
#define DPNI_CMDID_SET_LINK_CFG			0x21A1

#define DPNI_CMDID_SET_PRIM_MAC			0x2241
#define DPNI_CMDID_GET_PRIM_MAC			0x2251
#define DPNI_CMDID_ADD_MAC_ADDR			0x2261
#define DPNI_CMDID_REMOVE_MAC_ADDR		0x2271

#define DPNI_CMDID_GET_STATISTICS		0x25D1
#define DPNI_CMDID_RESET_STATISTICS		0x25E1
#define DPNI_CMDID_GET_QUEUE			0x25F1
#define DPNI_CMDID_SET_QUEUE			0x2601
#define DPNI_CMDID_SET_TX_CONFIRMATION_MODE	0x2661

/*                cmd, param, offset, width, type, arg_name */
#define DPNI_CMD_OPEN(cmd, dpni_id) \
	MC_CMD_OP(cmd,	 0,	0,	32,	int,	dpni_id)

/*                cmd, param, offset, width, type, arg_name */
#define DPNI_PREP_CFG(param, cfg) \
do { \
	MC_PREP_OP(param, 0, 0,   32, uint16_t, cfg->adv.options); \
	MC_PREP_OP(param, 0, 32,   8, uint16_t, cfg->adv.num_queues); \
	MC_PREP_OP(param, 0, 40,   8, uint16_t, cfg->adv.num_tcs); \
	MC_PREP_OP(param, 0, 48,   8, uint16_t, cfg->adv.mac_entries); \
	MC_PREP_OP(param, 1, 0,   8, uint16_t, cfg->adv.vlan_entries); \
	MC_PREP_OP(param, 1, 16,   8, uint16_t, cfg->adv.qos_entries); \
	MC_PREP_OP(param, 1, 32,   16, uint16_t, cfg->adv.fs_entries); \
} while (0)

/*                cmd, param, offset, width, type, arg_name */
#define DPNI_EXT_CFG(param, cfg) \
do { \
	MC_EXT_OP(param, 0, 0,   32, uint16_t, cfg->adv.options); \
	MC_EXT_OP(param, 0, 32,   8, uint16_t, cfg->adv.num_queues); \
	MC_EXT_OP(param, 0, 40,   8, uint16_t, cfg->adv.num_tcs); \
	MC_EXT_OP(param, 0, 48,   8, uint16_t, cfg->adv.mac_entries); \
	MC_EXT_OP(param, 1, 0,   8, uint16_t, cfg->adv.vlan_entries); \
	MC_EXT_OP(param, 1, 16,   8, uint16_t, cfg->adv.qos_entries); \
	MC_EXT_OP(param, 1, 32,   16, uint16_t, cfg->adv.fs_entries); \
} while (0)

/*                cmd, param, offset, width, type, arg_name */
#define DPNI_CMD_CREATE(cmd, cfg) \
do { \
	MC_CMD_OP(cmd, 0, 0,	32,	uint32_t,  cfg->adv.options); \
	MC_CMD_OP(cmd, 0, 32,	8,	uint8_t,   cfg->adv.num_queues); \
	MC_CMD_OP(cmd, 0, 40,	8,	uint8_t,   cfg->adv.num_tcs); \
	MC_CMD_OP(cmd, 0, 48,	8,	uint8_t,   cfg->adv.mac_entries); \
	MC_CMD_OP(cmd, 1, 0,	8,	uint8_t,   cfg->adv.vlan_entries); \
	MC_CMD_OP(cmd, 1, 16,	8,	uint8_t,   cfg->adv.qos_entries); \
	MC_CMD_OP(cmd, 1, 32,	16,	uint8_t,   cfg->adv.fs_entries); \
} while (0)

/*                cmd, param, offset, width, type, arg_name */
#define DPNI_CMD_SET_POOLS(cmd, cfg) \
do { \
	MC_CMD_OP(cmd, 0, 0,  8,  uint8_t,  cfg->num_dpbp); \
	MC_CMD_OP(cmd, 0, 8,  1,  int,      cfg->pools[0].backup_pool); \
	MC_CMD_OP(cmd, 0, 9,  1,  int,      cfg->pools[1].backup_pool); \
	MC_CMD_OP(cmd, 0, 10, 1,  int,      cfg->pools[2].backup_pool); \
	MC_CMD_OP(cmd, 0, 11, 1,  int,      cfg->pools[3].backup_pool); \
	MC_CMD_OP(cmd, 0, 12, 1,  int,      cfg->pools[4].backup_pool); \
	MC_CMD_OP(cmd, 0, 13, 1,  int,      cfg->pools[5].backup_pool); \
	MC_CMD_OP(cmd, 0, 14, 1,  int,      cfg->pools[6].backup_pool); \
	MC_CMD_OP(cmd, 0, 15, 1,  int,      cfg->pools[7].backup_pool); \
	MC_CMD_OP(cmd, 0, 32, 32, int,      cfg->pools[0].dpbp_id); \
	MC_CMD_OP(cmd, 4, 32, 16, uint16_t, cfg->pools[0].buffer_size);\
	MC_CMD_OP(cmd, 1, 0,  32, int,      cfg->pools[1].dpbp_id); \
	MC_CMD_OP(cmd, 4, 48, 16, uint16_t, cfg->pools[1].buffer_size);\
	MC_CMD_OP(cmd, 1, 32, 32, int,      cfg->pools[2].dpbp_id); \
	MC_CMD_OP(cmd, 5, 0,  16, uint16_t, cfg->pools[2].buffer_size);\
	MC_CMD_OP(cmd, 2, 0,  32, int,      cfg->pools[3].dpbp_id); \
	MC_CMD_OP(cmd, 5, 16, 16, uint16_t, cfg->pools[3].buffer_size);\
	MC_CMD_OP(cmd, 2, 32, 32, int,      cfg->pools[4].dpbp_id); \
	MC_CMD_OP(cmd, 5, 32, 16, uint16_t, cfg->pools[4].buffer_size);\
	MC_CMD_OP(cmd, 3, 0,  32, int,      cfg->pools[5].dpbp_id); \
	MC_CMD_OP(cmd, 5, 48, 16, uint16_t, cfg->pools[5].buffer_size);\
	MC_CMD_OP(cmd, 3, 32, 32, int,      cfg->pools[6].dpbp_id); \
	MC_CMD_OP(cmd, 6, 0,  16, uint16_t, cfg->pools[6].buffer_size);\
	MC_CMD_OP(cmd, 4, 0,  32, int,      cfg->pools[7].dpbp_id); \
	MC_CMD_OP(cmd, 6, 16, 16, uint16_t, cfg->pools[7].buffer_size);\
} while (0)

/*                cmd, param, offset, width, type, arg_name */
#define DPNI_RSP_GET_ATTR(cmd, attr) \
do { \
	MC_RSP_OP(cmd, 0, 0,  32, int,	    attr->options);\
	MC_RSP_OP(cmd, 0, 32,  8,  uint8_t,  attr->max_num_queues); \
	MC_RSP_OP(cmd, 0, 40,  8,  uint8_t,  attr->max_num_tcs); \
	MC_RSP_OP(cmd, 0, 48,  8,  uint8_t,  attr->max_mac_entries); \
	MC_RSP_OP(cmd, 1,  0,  8,  uint8_t,  attr->max_vlan_entries); \
	MC_RSP_OP(cmd, 1, 16,  8,  uint8_t,  attr->max_qos_entries); \
	MC_RSP_OP(cmd, 1, 32, 16,  uint16_t,  attr->max_fs_entries); \
	MC_RSP_OP(cmd, 2,  0,  8,  uint8_t,  attr->max_qos_key_size); \
	MC_RSP_OP(cmd, 2,  8,  8,  uint8_t,  attr->max_fs_key_size); \
	MC_RSP_OP(cmd, 2, 16, 16,  uint16_t,  attr->wriop_version); \
} while (0)

/*                cmd, param, offset, width, type, arg_name */
#define DPNI_CMD_SET_ERRORS_BEHAVIOR(cmd, cfg) \
do { \
	MC_CMD_OP(cmd, 0, 0,  32, uint32_t, cfg->errors); \
	MC_CMD_OP(cmd, 0, 32, 4,  enum dpni_error_action, cfg->error_action); \
	MC_CMD_OP(cmd, 0, 36, 1,  int,      cfg->set_frame_annotation); \
} while (0)

/*                cmd, param, offset, width, type, arg_name */
#define DPNI_CMD_SET_BUFFER_LAYOUT(cmd, layout, queue) \
do { \
	MC_CMD_OP(cmd, 0, 0,  8, enum dpni_queue_type, queue); \
	MC_CMD_OP(cmd, 1, 0,  16, uint16_t, layout->private_data_size); \
	MC_CMD_OP(cmd, 1, 16, 16, uint16_t, layout->data_align); \
	MC_CMD_OP(cmd, 0, 32, 16, uint16_t, layout->options); \
	MC_CMD_OP(cmd, 0, 48,  1,  int,	    layout->pass_timestamp); \
	MC_CMD_OP(cmd, 0, 49,  1,  int,	    layout->pass_parser_result); \
	MC_CMD_OP(cmd, 0, 50,  1,  int,	    layout->pass_frame_status); \
	MC_CMD_OP(cmd, 1, 32, 16, uint16_t, layout->data_head_room); \
	MC_CMD_OP(cmd, 1, 48, 16, uint16_t, layout->data_tail_room); \
} while (0)

/*                cmd, param, offset, width, type, arg_name */
#define DPNI_RSP_GET_QDID(cmd, qdid) \
	MC_RSP_OP(cmd, 0, 0,  16, uint16_t, qdid)

/*                cmd, param, offset, width, type, arg_name */
#define DPNI_RSP_GET_TX_DATA_OFFSET(cmd, data_offset) \
	MC_RSP_OP(cmd, 0, 0,  16, uint16_t, data_offset)

/*                cmd, param, offset, width, type, arg_name */
#define DPNI_CMD_SET_LINK_CFG(cmd, cfg) \
do { \
	MC_CMD_OP(cmd, 1, 0,  32, uint32_t, cfg->rate);\
	MC_CMD_OP(cmd, 2, 0,  64, uint64_t, cfg->options);\
} while (0)

/*                cmd, param, offset, width, type, arg_name */
#define DPNI_RSP_GET_LINK_STATE(cmd, state) \
do { \
	MC_RSP_OP(cmd, 0, 32,  1, int,      state->up);\
	MC_RSP_OP(cmd, 1, 0,  32, uint32_t, state->rate);\
	MC_RSP_OP(cmd, 2, 0,  64, uint64_t, state->options);\
} while (0)



/*                cmd, param, offset, width, type, arg_name */
#define DPNI_CMD_SET_PRIMARY_MAC_ADDR(cmd, mac_addr) \
do { \
	MC_CMD_OP(cmd, 0, 16, 8,  uint8_t,  mac_addr[5]); \
	MC_CMD_OP(cmd, 0, 24, 8,  uint8_t,  mac_addr[4]); \
	MC_CMD_OP(cmd, 0, 32, 8,  uint8_t,  mac_addr[3]); \
	MC_CMD_OP(cmd, 0, 40, 8,  uint8_t,  mac_addr[2]); \
	MC_CMD_OP(cmd, 0, 48, 8,  uint8_t,  mac_addr[1]); \
	MC_CMD_OP(cmd, 0, 56, 8,  uint8_t,  mac_addr[0]); \
} while (0)

/*                cmd, param, offset, width, type, arg_name */
#define DPNI_RSP_GET_PRIMARY_MAC_ADDR(cmd, mac_addr) \
do { \
	MC_RSP_OP(cmd, 0, 16, 8,  uint8_t,  mac_addr[5]); \
	MC_RSP_OP(cmd, 0, 24, 8,  uint8_t,  mac_addr[4]); \
	MC_RSP_OP(cmd, 0, 32, 8,  uint8_t,  mac_addr[3]); \
	MC_RSP_OP(cmd, 0, 40, 8,  uint8_t,  mac_addr[2]); \
	MC_RSP_OP(cmd, 0, 48, 8,  uint8_t,  mac_addr[1]); \
	MC_RSP_OP(cmd, 0, 56, 8,  uint8_t,  mac_addr[0]); \
} while (0)

/*                cmd, param, offset, width, type, arg_name */
#define DPNI_CMD_ADD_MAC_ADDR(cmd, mac_addr) \
do { \
	MC_CMD_OP(cmd, 0, 16, 8,  uint8_t,  mac_addr[5]); \
	MC_CMD_OP(cmd, 0, 24, 8,  uint8_t,  mac_addr[4]); \
	MC_CMD_OP(cmd, 0, 32, 8,  uint8_t,  mac_addr[3]); \
	MC_CMD_OP(cmd, 0, 40, 8,  uint8_t,  mac_addr[2]); \
	MC_CMD_OP(cmd, 0, 48, 8,  uint8_t,  mac_addr[1]); \
	MC_CMD_OP(cmd, 0, 56, 8,  uint8_t,  mac_addr[0]); \
} while (0)

/*                cmd, param, offset, width, type, arg_name */
#define DPNI_CMD_REMOVE_MAC_ADDR(cmd, mac_addr) \
do { \
	MC_CMD_OP(cmd, 0, 16, 8,  uint8_t,  mac_addr[5]); \
	MC_CMD_OP(cmd, 0, 24, 8,  uint8_t,  mac_addr[4]); \
	MC_CMD_OP(cmd, 0, 32, 8,  uint8_t,  mac_addr[3]); \
	MC_CMD_OP(cmd, 0, 40, 8,  uint8_t,  mac_addr[2]); \
	MC_CMD_OP(cmd, 0, 48, 8,  uint8_t,  mac_addr[1]); \
	MC_CMD_OP(cmd, 0, 56, 8,  uint8_t,  mac_addr[0]); \
} while (0)

#define DPNI_CMD_GET_QUEUE(cmd, type, tc, index) \
do { \
	MC_CMD_OP(cmd, 0,  0,  8, enum dpni_queue_type, type); \
	MC_CMD_OP(cmd, 0,  8,  8, uint8_t, tc); \
	MC_CMD_OP(cmd, 0, 16,  8, uint8_t, index); \
} while (0)

#define DPNI_RSP_GET_QUEUE(cmd, queue) \
do { \
	MC_RSP_OP(cmd, 1,  0, 32, uint32_t, (queue)->destination.id); \
	MC_RSP_OP(cmd, 1, 56,  4, enum dpni_dest, (queue)->destination.type); \
	MC_RSP_OP(cmd, 1, 62,  1, char, (queue)->destination.stash_ctrl); \
	MC_RSP_OP(cmd, 1, 63,  1, char, (queue)->destination.hold_active); \
	MC_RSP_OP(cmd, 2,  0, 64, uint64_t, (queue)->flc); \
	MC_RSP_OP(cmd, 3,  0, 64, uint64_t, (queue)->user_context); \
	MC_RSP_OP(cmd, 4,  0, 32, uint32_t, (queue)->fqid); \
	MC_RSP_OP(cmd, 4, 32, 16, uint16_t, (queue)->qdbin); \
} while (0)

#define DPNI_CMD_SET_QUEUE(cmd, type, tc, index, queue) \
do { \
	MC_CMD_OP(cmd, 0,  0,  8, enum dpni_queue_type, type); \
	MC_CMD_OP(cmd, 0,  8,  8, uint8_t, tc); \
	MC_CMD_OP(cmd, 0, 16,  8, uint8_t, index); \
	MC_CMD_OP(cmd, 0, 24,  8, uint8_t, (queue)->options); \
	MC_CMD_OP(cmd, 1,  0, 32, uint32_t, (queue)->destination.id); \
	MC_CMD_OP(cmd, 1, 56,  4, enum dpni_dest, (queue)->destination.type); \
	MC_CMD_OP(cmd, 1, 62,  1, char, (queue)->destination.stash_ctrl); \
	MC_CMD_OP(cmd, 1, 63,  1, char, (queue)->destination.hold_active); \
	MC_CMD_OP(cmd, 1,  0, 32, uint32_t, (queue)->destination.id); \
	MC_CMD_OP(cmd, 2,  0, 64, uint64_t, (queue)->flc); \
	MC_CMD_OP(cmd, 3,  0, 64, uint64_t, (queue)->user_context); \
} while (0)

/*			cmd, param, offset, width, type, arg_name */
#define DPNI_CMD_GET_STATISTICS(cmd, page) \
	MC_CMD_OP(cmd, 0, 0, 8, uint8_t, page)

/*                cmd, param, offset, width, type, arg_name */
#define DPNI_RSP_GET_STATISTICS(cmd, stat) \
do { \
	MC_RSP_OP(cmd, 0, 0, 64, uint64_t, (stat)->counter0); \
	MC_RSP_OP(cmd, 1, 0, 64, uint64_t, (stat)->counter1); \
	MC_RSP_OP(cmd, 2, 0, 64, uint64_t, (stat)->counter2); \
	MC_RSP_OP(cmd, 3, 0, 64, uint64_t, (stat)->counter3); \
	MC_RSP_OP(cmd, 4, 0, 64, uint64_t, (stat)->counter4); \
	MC_RSP_OP(cmd, 5, 0, 64, uint64_t, (stat)->counter5); \
	MC_RSP_OP(cmd, 6, 0, 64, uint64_t, (stat)->counter6); \
} while (0)

enum net_prot {
	NET_PROT_NONE = 0,
	NET_PROT_PAYLOAD,
	NET_PROT_ETH,
	NET_PROT_VLAN,
	NET_PROT_IPV4,
	NET_PROT_IPV6,
	NET_PROT_IP,
	NET_PROT_TCP,
	NET_PROT_UDP,
	NET_PROT_UDP_LITE,
	NET_PROT_IPHC,
	NET_PROT_SCTP,
	NET_PROT_SCTP_CHUNK_DATA,
	NET_PROT_PPPOE,
	NET_PROT_PPP,
	NET_PROT_PPPMUX,
	NET_PROT_PPPMUX_SUBFRM,
	NET_PROT_L2TPV2,
	NET_PROT_L2TPV3_CTRL,
	NET_PROT_L2TPV3_SESS,
	NET_PROT_LLC,
	NET_PROT_LLC_SNAP,
	NET_PROT_NLPID,
	NET_PROT_SNAP,
	NET_PROT_MPLS,
	NET_PROT_IPSEC_AH,
	NET_PROT_IPSEC_ESP,
	NET_PROT_UDP_ENC_ESP, /* RFC 3948 */
	NET_PROT_MACSEC,
	NET_PROT_GRE,
	NET_PROT_MINENCAP,
	NET_PROT_DCCP,
	NET_PROT_ICMP,
	NET_PROT_IGMP,
	NET_PROT_ARP,
	NET_PROT_CAPWAP_DATA,
	NET_PROT_CAPWAP_CTRL,
	NET_PROT_RFC2684,
	NET_PROT_ICMPV6,
	NET_PROT_FCOE,
	NET_PROT_FIP,
	NET_PROT_ISCSI,
	NET_PROT_GTP,
	NET_PROT_USER_DEFINED_L2,
	NET_PROT_USER_DEFINED_L3,
	NET_PROT_USER_DEFINED_L4,
	NET_PROT_USER_DEFINED_L5,
	NET_PROT_USER_DEFINED_SHIM1,
	NET_PROT_USER_DEFINED_SHIM2,

	NET_PROT_DUMMY_LAST
};

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
#define DPNI_ALL_TCS				(uint8_t)(-1)
/* All flows within traffic class considered; see dpni_set_rx_flow() */
#define DPNI_ALL_TC_FLOWS			(uint16_t)(-1)
/* Generate new flow ID; see dpni_set_tx_flow() */
#define DPNI_NEW_FLOW_ID			(uint16_t)(-1)
/* use for common tx-conf queue; see dpni_set_tx_conf_<x>() */
#define DPNI_COMMON_TX_CONF			(uint16_t)(-1)

/**
 * dpni_open() - Open a control session for the specified object
 * @mc_io:	Pointer to MC portal's I/O object
 * @cmd_flags:	Command flags; one or more of 'MC_CMD_FLAG_'
 * @dpni_id:	DPNI unique ID
 * @token:	Returned token; use in subsequent API calls
 *
 * This function can be used to open a control session for an
 * already created object; an object may have been declared in
 * the DPL or by calling the dpni_create() function.
 * This function returns a unique authentication token,
 * associated with the specific object ID and the specific MC
 * portal; this token must be used in all subsequent commands for
 * this specific object.
 *
 * Return:	'0' on Success; Error code otherwise.
 */
int dpni_open(struct fsl_mc_io	*mc_io,
	      uint32_t		cmd_flags,
	      int		dpni_id,
	      uint16_t		*token);

/**
 * dpni_close() - Close the control session of the object
 * @mc_io:	Pointer to MC portal's I/O object
 * @cmd_flags:	Command flags; one or more of 'MC_CMD_FLAG_'
 * @token:	Token of DPNI object
 *
 * After this function is called, no further operations are
 * allowed on the object without opening a new control session.
 *
 * Return:	'0' on Success; Error code otherwise.
 */
int dpni_close(struct fsl_mc_io	*mc_io,
	       uint32_t		cmd_flags,
	       uint16_t		token);

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

struct dpni_cfg {
		uint8_t mac_addr[6];
	struct {
		uint32_t		options;
		uint16_t		fs_entries;
		uint8_t			num_queues;
		uint8_t			num_tcs;
		uint8_t			mac_entries;
		uint8_t			vlan_entries;
		uint8_t			qos_entries;
	} adv;
};

/**
 * struct dpni_extended_cfg - Structure representing extended DPNI configuration
 * @tc_cfg: TCs configuration
 * @ipr_cfg: IP reassembly configuration
 */
struct dpni_extended_cfg {
	/**
	 * struct tc_cfg - TC configuration
	 * @max_dist: Maximum distribution size for Rx traffic class;
	 *	supported values: 1,2,3,4,6,7,8,12,14,16,24,28,32,48,56,64,96,
	 *	112,128,192,224,256,384,448,512,768,896,1024;
	 *	value '0' will be treated as '1'.
	 *	other unsupported values will be round down to the nearest
	 *	supported value.
	 * @max_fs_entries: Maximum FS entries for Rx traffic class;
	 *	'0' means no support for this TC;
	 */
	struct {
		uint16_t	max_dist;
		uint16_t	max_fs_entries;
	} tc_cfg[DPNI_MAX_TC];
	/**
	 * struct ipr_cfg - Structure representing IP reassembly configuration
	 * @max_reass_frm_size: Maximum size of the reassembled frame
	 * @min_frag_size_ipv4: Minimum fragment size of IPv4 fragments
	 * @min_frag_size_ipv6: Minimum fragment size of IPv6 fragments
	 * @max_open_frames_ipv4: Maximum concurrent IPv4 packets in reassembly
	 *		process
	 * @max_open_frames_ipv6: Maximum concurrent IPv6 packets in reassembly
	 *		process
	 */
	struct {
		uint16_t max_reass_frm_size;
		uint16_t min_frag_size_ipv4;
		uint16_t min_frag_size_ipv6;
		uint16_t max_open_frames_ipv4;
		uint16_t max_open_frames_ipv6;
	} ipr_cfg;
};

/**
 * dpni_prepare_cfg() - function prepare parameters
 * @cfg: cfg structure
 * @cfg_buf: Zeroed 256 bytes of memory before mapping it to DMA
 *
 * This function has to be called before dpni_create()
 */
int dpni_prepare_cfg(const struct dpni_cfg	*cfg,
		     uint8_t			*cfg_buf);
/**
 * dpni_create() - Create the DPNI object
 * @mc_io:	Pointer to MC portal's I/O object
 * @token:	Authentication token.
 * @cmd_flags:	Command flags; one or more of 'MC_CMD_FLAG_'
 * @cfg:	Configuration structure
 * @obj_id:	Returned obj_id; use in subsequent API calls
 *
 * Create the DPNI object, allocate required resources and
 * perform required initialization.
 *
 * The object can be created either by declaring it in the
 * DPL file, or by calling this function.
 *
 * This function returns a unique authentication token,
 * associated with the specific object ID and the specific MC
 * portal; this token must be used in all subsequent calls to
 * this specific object. For objects that are created using the
 * DPL file, call dpni_open() function to get an authentication
 * token first.
 *
 * Return:	'0' on Success; Error code otherwise.
 */
int dpni_create(struct fsl_mc_io	*mc_io,
		uint16_t		token,
		uint32_t		cmd_flags,
		const struct dpni_cfg	*cfg,
		uint32_t		*obj_id);

/**
 * dpni_destroy() - Destroy the DPNI object and release all its resources.
 * @mc_io:	Pointer to MC portal's I/O object
 * @token:	Authentication token.
 * @cmd_flags:	Command flags; one or more of 'MC_CMD_FLAG_'
 * @obj_id:	Returned obj_id; use in subsequent API calls
 *
 * Return:	'0' on Success; error code otherwise.
 */
int dpni_destroy(struct fsl_mc_io	*mc_io,
		 uint16_t		token,
		 uint32_t		cmd_flags,
		 uint32_t		obj_id);

/**
 * struct dpni_pools_cfg - Structure representing buffer pools configuration
 * @num_dpbp: Number of DPBPs
 * @pools: Array of buffer pools parameters; The number of valid entries
 *	must match 'num_dpbp' value
 */
struct dpni_pools_cfg {
	uint8_t num_dpbp;
	/**
	 * struct pools - Buffer pools parameters
	 * @dpbp_id: DPBP object ID
	 * @buffer_size: Buffer size
	 * @backup_pool: Backup pool
	 */
	struct {
		int		dpbp_id;
		uint16_t	buffer_size;
		int		backup_pool;
	} pools[DPNI_MAX_DPBP];
};

/**
 * dpni_set_pools() - Set buffer pools configuration
 * @mc_io:	Pointer to MC portal's I/O object
 * @cmd_flags:	Command flags; one or more of 'MC_CMD_FLAG_'
 * @token:	Token of DPNI object
 * @cfg:	Buffer pools configuration
 *
 * mandatory for DPNI operation
 * warning:Allowed only when DPNI is disabled
 *
 * Return:	'0' on Success; Error code otherwise.
 */
int dpni_set_pools(struct fsl_mc_io		*mc_io,
		   uint32_t			cmd_flags,
		   uint16_t			token,
		   const struct dpni_pools_cfg	*cfg);

/**
 * dpni_enable() - Enable the DPNI, allow sending and receiving frames.
 * @mc_io:	Pointer to MC portal's I/O object
 * @cmd_flags:	Command flags; one or more of 'MC_CMD_FLAG_'
 * @token:		Token of DPNI object
 *
 * Return:	'0' on Success; Error code otherwise.
 */
int dpni_enable(struct fsl_mc_io	*mc_io,
		uint32_t		cmd_flags,
		uint16_t		token);

/**
 * dpni_disable() - Disable the DPNI, stop sending and receiving frames.
 * @mc_io:	Pointer to MC portal's I/O object
 * @cmd_flags:	Command flags; one or more of 'MC_CMD_FLAG_'
 * @token:	Token of DPNI object
 *
 * Return:	'0' on Success; Error code otherwise.
 */
int dpni_disable(struct fsl_mc_io	*mc_io,
		 uint32_t		cmd_flags,
		 uint16_t		token);


/**
 * dpni_reset() - Reset the DPNI, returns the object to initial state.
 * @mc_io:	Pointer to MC portal's I/O object
 * @cmd_flags:	Command flags; one or more of 'MC_CMD_FLAG_'
 * @token:	Token of DPNI object
 *
 * Return:	'0' on Success; Error code otherwise.
 */
int dpni_reset(struct fsl_mc_io	*mc_io,
	       uint32_t		cmd_flags,
	       uint16_t		token);

/**
 * struct dpni_attr - Structure representing DPNI attributes
 * @options: Mask of available options; reflects the value as was given in
 *		object's creation
 * @max_num_queues: Number of queues available (for both Tx and Rx)
 * @max_num_tcs: Maximum number of traffic classes (for both Tx and Rx)
 * @max_mac_entries: Maximum number of traffic classes (for both Tx and Rx)
 * @max_unicast_filters: Maximum number of unicast filters
 * @max_multicast_filters: Maximum number of multicast filters
 * @max_vlan_entries: Maximum number of VLAN filters
 * @max_qos_entries: if 'max_tcs > 1', declares the maximum entries in QoS table
 * @max_fs_entries: declares the maximum entries in flow steering table
 * @max_qos_key_size: Maximum key size for the QoS look-up
 * @max_fs_key_size: Maximum key size for the flow steering
 * @wriop_version: Indicates revision of WRIOP hardware block
 */
struct dpni_attr {
	uint32_t id;
	uint32_t options;
	uint8_t max_num_queues;
	uint8_t max_num_tcs;
	uint8_t max_mac_entries;
	uint8_t max_vlan_entries;
	uint8_t max_qos_entries;
	uint16_t max_fs_entries;
	uint8_t max_qos_key_size;
	uint8_t max_fs_key_size;
	uint16_t wriop_version;
};

/**
 * dpni_get_attributes() - Retrieve DPNI attributes.
 * @mc_io:	Pointer to MC portal's I/O object
 * @cmd_flags:	Command flags; one or more of 'MC_CMD_FLAG_'
 * @token:	Token of DPNI object
 * @attr:	Object's attributes
 *
 * Return:	'0' on Success; Error code otherwise.
 */
int dpni_get_attributes(struct fsl_mc_io	*mc_io,
			uint32_t		cmd_flags,
			uint16_t		token,
			struct dpni_attr	*attr);

/**
 * dpni_extract_cfg() - extract the parameters
 * @cfg: cfg structure
 * @cfg_buf: 256 bytes of DMA-able memory
 *
 * This function has to be called after dpni_get_attributes()
 */
int dpni_extract_cfg(struct dpni_cfg	*cfg,
		     const uint8_t	*cfg_buf);

/**
 * DPNI errors
 */

/**
 * Extract out of frame header error
 */
#define DPNI_ERROR_EOFHE	0x00020000
/**
 * Frame length error
 */
#define DPNI_ERROR_FLE		0x00002000
/**
 * Frame physical error
 */
#define DPNI_ERROR_FPE		0x00001000
/**
 * Parsing header error
 */
#define DPNI_ERROR_PHE		0x00000020
/**
 * Parser L3 checksum error
 */
#define DPNI_ERROR_L3CE		0x00000004
/**
 * Parser L3 checksum error
 */
#define DPNI_ERROR_L4CE		0x00000001

/**
 * enum dpni_error_action - Defines DPNI behavior for errors
 * @DPNI_ERROR_ACTION_DISCARD: Discard the frame
 * @DPNI_ERROR_ACTION_CONTINUE: Continue with the normal flow
 * @DPNI_ERROR_ACTION_SEND_TO_ERROR_QUEUE: Send the frame to the error queue
 */
enum dpni_error_action {
	DPNI_ERROR_ACTION_DISCARD = 0,
	DPNI_ERROR_ACTION_CONTINUE = 1,
	DPNI_ERROR_ACTION_SEND_TO_ERROR_QUEUE = 2
};

/**
 * struct dpni_error_cfg - Structure representing DPNI errors treatment
 * @errors: Errors mask; use 'DPNI_ERROR__<X>
 * @error_action: The desired action for the errors mask
 * @set_frame_annotation: Set to '1' to mark the errors in frame annotation
 *		status (FAS); relevant only for the non-discard action
 */
struct dpni_error_cfg {
	uint32_t		errors;
	enum dpni_error_action	error_action;
	int			set_frame_annotation;
};

/**
 * dpni_set_errors_behavior() - Set errors behavior
 * @mc_io:	Pointer to MC portal's I/O object
 * @cmd_flags:	Command flags; one or more of 'MC_CMD_FLAG_'
 * @token:	Token of DPNI object
 * @cfg:	Errors configuration
 *
 * this function may be called numerous times with different
 * error masks
 *
 * Return:	'0' on Success; Error code otherwise.
 */
int dpni_set_errors_behavior(struct fsl_mc_io		*mc_io,
			     uint32_t			cmd_flags,
			     uint16_t			token,
			     struct dpni_error_cfg	*cfg);

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

/**
 * struct dpni_buffer_layout - Structure representing DPNI buffer layout
 * @options: Flags representing the suggested modifications to the buffer
 *		layout; Use any combination of 'DPNI_BUF_LAYOUT_OPT_<X>' flags
 * @pass_timestamp: Pass timestamp value
 * @pass_parser_result: Pass parser results
 * @pass_frame_status: Pass frame status
 * @private_data_size: Size kept for private data (in bytes)
 * @data_align: Data alignment
 * @data_head_room: Data head room
 * @data_tail_room: Data tail room
 */
struct dpni_buffer_layout {
	uint16_t options;
	int pass_timestamp;
	int pass_parser_result;
	int pass_frame_status;
	uint16_t private_data_size;
	uint16_t data_align;
	uint16_t data_head_room;
	uint16_t data_tail_room;
};

/**
 * dpni_get_buffer_layout() - Retrieve buffer layout attributes.
 * @mc_io:	Pointer to MC portal's I/O object
 * @cmd_flags:	Command flags; one or more of 'MC_CMD_FLAG_'
 * @token:	Token of DPNI object
 * @layout:	Returns buffer layout attributes
 * @type:	DPNI queue type
 *
 * Return:	'0' on Success; Error code otherwise.
 */
int dpni_get_buffer_layout(struct fsl_mc_io			*mc_io,
			   uint32_t				cmd_flags,
			   uint16_t				token,
			   const struct dpni_buffer_layout	*layout,
			   enum dpni_queue_type			type);

/**
 * dpni_set_buffer_layout() - Set buffer layout configuration.
 * @mc_io:	Pointer to MC portal's I/O object
 * @cmd_flags:	Command flags; one or more of 'MC_CMD_FLAG_'
 * @token:	Token of DPNI object
 * @layout:	Buffer layout configuration
 * @type:	DPNI queue type
 *
 * Return:	'0' on Success; Error code otherwise.
 *
 * @warning	Allowed only when DPNI is disabled
 */
int dpni_set_buffer_layout(struct fsl_mc_io			*mc_io,
			   uint32_t				cmd_flags,
			   uint16_t				token,
			   const struct dpni_buffer_layout	*layout,
			   enum dpni_queue_type			type);

/**
 * dpni_get_qdid() - Get the Queuing Destination ID (QDID) that should be used
 *			for enqueue operations
 * @mc_io:	Pointer to MC portal's I/O object
 * @cmd_flags:	Command flags; one or more of 'MC_CMD_FLAG_'
 * @token:	Token of DPNI object
 * @qdid:	Returned virtual QDID value that should be used as an argument
 *			in all enqueue operations
 *
 * Return:	'0' on Success; Error code otherwise.
 */
int dpni_get_qdid(struct fsl_mc_io	*mc_io,
		  uint32_t		cmd_flags,
		  uint16_t		token,
		  uint16_t		*qdid);

/**
 * dpni_get_tx_data_offset() - Get the Tx data offset (from start of buffer)
 * @mc_io:	Pointer to MC portal's I/O object
 * @cmd_flags:	Command flags; one or more of 'MC_CMD_FLAG_'
 * @token:	Token of DPNI object
 * @data_offset: Tx data offset (from start of buffer)
 *
 * Return:	'0' on Success; Error code otherwise.
 */
int dpni_get_tx_data_offset(struct fsl_mc_io	*mc_io,
			    uint32_t		cmd_flags,
			    uint16_t		token,
			    uint16_t		*data_offset);

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
 */
struct dpni_link_cfg {
	uint32_t rate;
	uint64_t options;
};

/**
 * dpni_set_link_cfg() - set the link configuration.
 * @mc_io:	Pointer to MC portal's I/O object
 * @cmd_flags:	Command flags; one or more of 'MC_CMD_FLAG_'
 * @token:	Token of DPNI object
 * @cfg:	Link configuration
 *
 * Return:	'0' on Success; Error code otherwise.
 */
int dpni_set_link_cfg(struct fsl_mc_io			*mc_io,
		      uint32_t				cmd_flags,
		      uint16_t				token,
		      const struct dpni_link_cfg	*cfg);

/**
 * struct dpni_link_state - Structure representing DPNI link state
 * @rate: Rate
 * @options: Mask of available options; use 'DPNI_LINK_OPT_<X>' values
 * @up: Link state; '0' for down, '1' for up
 */
struct dpni_link_state {
	uint32_t rate;
	uint64_t options;
	int up;
};

/**
 * dpni_get_link_state() - Return the link state (either up or down)
 * @mc_io:	Pointer to MC portal's I/O object
 * @cmd_flags:	Command flags; one or more of 'MC_CMD_FLAG_'
 * @token:	Token of DPNI object
 * @state:	Returned link state;
 *
 * Return:	'0' on Success; Error code otherwise.
 */
int dpni_get_link_state(struct fsl_mc_io	*mc_io,
			uint32_t		cmd_flags,
			uint16_t		token,
			struct dpni_link_state	*state);

/**
 * dpni_set_primary_mac_addr() - Set the primary MAC address
 * @mc_io:	Pointer to MC portal's I/O object
 * @cmd_flags:	Command flags; one or more of 'MC_CMD_FLAG_'
 * @token:	Token of DPNI object
 * @mac_addr:	MAC address to set as primary address
 *
 * Return:	'0' on Success; Error code otherwise.
 */
int dpni_set_primary_mac_addr(struct fsl_mc_io *mc_io,
			      uint32_t		cmd_flags,
			      uint16_t		token,
			      const uint8_t	mac_addr[6]);

/**
 * dpni_get_primary_mac_addr() - Get the primary MAC address
 * @mc_io:	Pointer to MC portal's I/O object
 * @cmd_flags:	Command flags; one or more of 'MC_CMD_FLAG_'
 * @token:	Token of DPNI object
 * @mac_addr:	Returned MAC address
 *
 * Return:	'0' on Success; Error code otherwise.
 */
int dpni_get_primary_mac_addr(struct fsl_mc_io	*mc_io,
			      uint32_t		cmd_flags,
			      uint16_t		token,
			      uint8_t		mac_addr[6]);

/**
 * dpni_add_mac_addr() - Add MAC address filter
 * @mc_io:	Pointer to MC portal's I/O object
 * @cmd_flags:	Command flags; one or more of 'MC_CMD_FLAG_'
 * @token:	Token of DPNI object
 * @mac_addr:	MAC address to add
 *
 * Return:	'0' on Success; Error code otherwise.
 */
int dpni_add_mac_addr(struct fsl_mc_io	*mc_io,
		      uint32_t		cmd_flags,
		      uint16_t		token,
		      const uint8_t	mac_addr[6]);

/**
 * dpni_remove_mac_addr() - Remove MAC address filter
 * @mc_io:	Pointer to MC portal's I/O object
 * @cmd_flags:	Command flags; one or more of 'MC_CMD_FLAG_'
 * @token:	Token of DPNI object
 * @mac_addr:	MAC address to remove
 *
 * Return:	'0' on Success; Error code otherwise.
 */
int dpni_remove_mac_addr(struct fsl_mc_io	*mc_io,
			 uint32_t		cmd_flags,
			 uint16_t		token,
			 const uint8_t		mac_addr[6]);

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

/**
 * struct dpni_dest_cfg - Structure representing DPNI destination parameters
 * @dest_type: Destination type
 * @dest_id: Either DPIO ID or DPCON ID, depending on the destination type
 * @priority: Priority selection within the DPIO or DPCON channel; valid values
 *		are 0-1 or 0-7, depending on the number of priorities in that
 *		channel; not relevant for 'DPNI_DEST_NONE' option
 */
struct dpni_dest_cfg {
	enum dpni_dest dest_type;
	int dest_id;
	uint8_t priority;
};

/**
 * enum dpni_flc_type - DPNI FLC types
 * @DPNI_FLC_USER_DEFINED: select the FLC to be used for user defined value
 * @DPNI_FLC_STASH: select the FLC to be used for stash control
 */
enum dpni_flc_type {
	DPNI_FLC_USER_DEFINED = 0,
	DPNI_FLC_STASH = 1,
};

/**
 * enum dpni_stash_size - DPNI FLC stashing size
 * @DPNI_STASH_SIZE_0B: no stash
 * @DPNI_STASH_SIZE_64B: stashes 64 bytes
 * @DPNI_STASH_SIZE_128B: stashes 128 bytes
 * @DPNI_STASH_SIZE_192B: stashes 192 bytes
 */
enum dpni_stash_size {
	DPNI_STASH_SIZE_0B = 0,
	DPNI_STASH_SIZE_64B = 1,
	DPNI_STASH_SIZE_128B = 2,
	DPNI_STASH_SIZE_192B = 3,
};

/* DPNI FLC stash options */

/* stashes the whole annotation area (up to 192 bytes) */
#define DPNI_FLC_STASH_FRAME_ANNOTATION	0x00000001

/**
 * struct dpni_flc_cfg - Structure representing DPNI FLC configuration
 * @flc_type: FLC type
 * @options: Mask of available options;
 *	use 'DPNI_FLC_STASH_<X>' values
 * @frame_data_size: Size of frame data to be stashed
 * @flow_context_size: Size of flow context to be stashed
 * @flow_context: 1. In case flc_type is 'DPNI_FLC_USER_DEFINED':
 *			this value will be provided in the frame descriptor
 *			(FD[FLC])
 *		  2. In case flc_type is 'DPNI_FLC_STASH':
 *			this value will be I/O virtual address of the
 *			flow-context;
 *			Must be cacheline-aligned and DMA-able memory
 */
struct dpni_flc_cfg {
	enum dpni_flc_type flc_type;
	uint32_t options;
	enum dpni_stash_size frame_data_size;
	enum dpni_stash_size flow_context_size;
	uint64_t flow_context;
};

/* DPNI queue modification options */

/* Select to modify the user's context associated with the queue */
#define DPNI_QUEUE_OPT_USER_CTX		0x00000001
/* Select to modify the queue's destination */
#define DPNI_QUEUE_OPT_DEST		0x00000002
/** Select to modify the flow-context parameters;
 * not applicable for Tx-conf/Err queues as the FD comes from the user
 */
#define DPNI_QUEUE_OPT_FLC		0x00000004
/* Select to modify the queue's order preservation */
#define DPNI_QUEUE_OPT_ORDER_PRESERVATION 0x00000008
/* Select to modify the queue's tail-drop threshold */
#define DPNI_QUEUE_OPT_TAILDROP_THRESHOLD 0x00000010

/**
 * struct dpni_queue_cfg - Structure representing queue configuration
 * @options: Flags representing the suggested modifications to the queue;
 *		Use any combination of 'DPNI_QUEUE_OPT_<X>' flags
 * @user_ctx: User context value provided in the frame descriptor of each
 *		dequeued frame; valid only if 'DPNI_QUEUE_OPT_USER_CTX'
 *		is contained in 'options'
 * @dest_cfg: Queue destination parameters;
 *		valid only if 'DPNI_QUEUE_OPT_DEST' is contained in 'options'
 * @flc_cfg: Flow context configuration; in case the TC's distribution
 *		is either NONE or HASH the FLC's settings of flow#0 are used.
 *		in the case of FS (flow-steering) the flow's FLC settings
 *		are used.
 *		valid only if 'DPNI_QUEUE_OPT_FLC' is contained in 'options'
 * @order_preservation_en: enable/disable order preservation;
 *		valid only if 'DPNI_QUEUE_OPT_ORDER_PRESERVATION' is contained
 *		in 'options'
 * @tail_drop_threshold: set the queue's tail drop threshold in bytes;
 *		'0' value disable the threshold; maximum value is 0xE000000;
 *		valid only if 'DPNI_QUEUE_OPT_TAILDROP_THRESHOLD' is contained
 *		in 'options'
 */
struct dpni_queue_cfg {
	uint32_t options;
	uint64_t user_ctx;
	struct dpni_dest_cfg dest_cfg;
	struct dpni_flc_cfg flc_cfg;
	int order_preservation_en;
	uint32_t tail_drop_threshold;
};

/**
 * struct dpni_queue_attr - Structure representing queue attributes
 * @user_ctx: User context value provided in the frame descriptor of each
 *	dequeued frame
 * @dest_cfg: Queue destination configuration
 * @flc_cfg: Flow context configuration
 * @order_preservation_en: enable/disable order preservation
 * @tail_drop_threshold: queue's tail drop threshold in bytes;
 * @fqid: Virtual fqid value to be used for dequeue operations
 */
struct dpni_queue_attr {
	uint64_t user_ctx;
	struct dpni_dest_cfg dest_cfg;
	struct dpni_flc_cfg flc_cfg;
	int order_preservation_en;
	uint32_t tail_drop_threshold;
	uint32_t fqid;
};

/* DPNI Tx flow modification options */

/* Select to modify the settings for dedicate Tx confirmation/error */
#define DPNI_TX_FLOW_OPT_TX_CONF_ERROR	0x00000001
/*!< Select to modify the L3 checksum generation setting */
#define DPNI_TX_FLOW_OPT_L3_CHKSUM_GEN	0x00000010
/*!< Select to modify the L4 checksum generation setting */
#define DPNI_TX_FLOW_OPT_L4_CHKSUM_GEN	0x00000020

/**
 * dpni_get_api_version - Retrieve DPNI Major and Minor version info.
 *
 * @mc_io:	Pointer to MC portal's I/O object
 * @cmd_flags:	Command flags; one or more of 'MC_CMD_FLAG_'
 * @major_ver:	DPNI major version
 * @minor_ver:	DPNI minor version
 *
 * Return:     '0' on Success; Error code otherwise.
 */
int dpni_get_api_version(struct fsl_mc_io *mc_io,
			 u32 cmd_flags,
			 u16 *major_ver,
			 u16 *minor_ver);

/**
 * enum dpni_confirmation_mode - Defines DPNI options supported for Tx
 * confirmation
 * @DPNI_CONF_AFFINE: For each Tx queue set associated with a sender there is
 * an affine Tx Confirmation queue
 * @DPNI_CONF_SINGLE: All Tx queues are associated with a single Tx
 * confirmation queue
 * @DPNI_CONF_DISABLE: Tx frames are not confirmed.  This must be associated
 * with proper FD set-up to have buffers release to a Buffer Pool, otherwise
 * buffers will be leaked.
 */
enum dpni_confirmation_mode {
	DPNI_CONF_AFFINE,
	DPNI_CONF_SINGLE,
	DPNI_CONF_DISABLE,
};

struct dpni_tx_confirmation_mode {
	uint32_t pad;
	uint8_t confirmation_mode;
};

/**
 * struct dpni_queue - Queue structure
 * @fqid:  FQID used for enqueueing to and/or configuration of this specific FQ
 * @qdbin: Queueing bin, used to enqueue using QDID, DQBIN, QPRI. Only relevant
 *         for Tx queues.
 * @flc:   FLC value for traffic dequeued from this queue.
 * @user_context:    User data, presented to the user along with any frames
 *                   from this queue. Not relevant for Tx queues.
 */
struct dpni_queue {
	/**
	* struct destination - Destination structure
	* @id:   ID of the destination, only relevant if DEST_TYPE is > 0.
	*        Identifies either a DPIO or a DPCON object. Not relevant for Tx
	*        queues.
	* @type: May be one of the following:
	*         0 - No destination, queue can be manually queried, but won't
	*             push traffic or notifications to a DPIO;
	*         1 - The destination is DPIO. When traffic becomes available in
	*             the queue a FQDAN (FQ data available notification) will be
	*             generated to selected DPIO;
	*         2 - The destination is a DPCON. The queue is associated with a
	*             DPCON object for purpose of scheduling between multiple
	*             queues. The DPCON may be independently configured to
	*             generate notifications. Not relevant for Tx queues.
	* @hold_active: Hold active
	*/
	struct {
		uint32_t id;
		enum dpni_dest type;
		char hold_active;
		char stash_ctrl;
	} destination;
	uint8_t  options;
	uint32_t fqid;
	uint16_t qdbin;
	uint64_t flc;
	uint64_t user_context;
};

/**
 * dpni_set_queue() - Set queue parameters
 * @mc_io:      Pointer to MC portal's I/O object
 * @cmd_flags:  Command flags; one or more of 'MC_CMD_FLAG_'
 * @token:      Token of DPNI object
 * @type:       Type of queue
 * @tc:         Traffic class, in range 0 to NUM_TCS - 1
 * @index:      Selects the specific queue out of the set allocated for the same
 *              TC. Value must be in range 0 to NUM_QUEUES - 1
 * @queue:      Queue structure
 *
 * Return:     '0' on Success; Error code otherwise.
 */
int dpni_set_queue(struct fsl_mc_io		*mc_io,
		   uint32_t			cmd_flags,
		   uint16_t			token,
		   enum dpni_queue_type		type,
		   uint8_t			tc,
		   uint8_t			index,
		   const struct dpni_queue	*queue);

/**
 * dpni_get_queue() - Get queue parameters
 * @mc_io:      Pointer to MC portal's I/O object
 * @cmd_flags:  Command flags; one or more of 'MC_CMD_FLAG_'
 * @token:      Token of DPNI object
 * @type:       Type of queue
 * @tc:         Traffic class, in range 0 to NUM_TCS - 1
 * @index:      Selects the specific queue out of the set allocated for the same
 *              TC. Value must be in range 0 to NUM_QUEUES - 1
 * @queue:      Queue structure
 *
 * Return:      '0' on Success; Error code otherwise.
 */
int dpni_get_queue(struct fsl_mc_io		*mc_io,
		   uint32_t			cmd_flags,
		   uint16_t			token,
		   enum dpni_queue_type		type,
		   uint8_t			tc,
		   uint8_t			index,
		   struct dpni_queue		*queue);

/**
 * dpni_set_tx_confirmation_mode() - Set TX conf mode
 * @mc_io:      Pointer to MC portal's I/O object
 * @cmd_flags:  Command flags; one or more of 'MC_CMD_FLAG_'
 * @token:      Token of DPNI object
 * @mode:       DPNI confirmation mode type
 *
 * Return:      '0' on Success; Error code otherwise.
 */
int dpni_set_tx_confirmation_mode(struct fsl_mc_io	*mc_io,
				  uint32_t		cmd_flags,
				  uint16_t		token,
				  enum dpni_confirmation_mode mode);
struct dpni_statistics {
	/**
	 * Page_0 statistics structure
	 * @ingress_all_frames: Ingress frame count
	 * @ingress_all_bytes: Ingress byte count
	 * @ingress_multicast_frames: Ingress multicast frame count
	 * @ingress_multicast_bytes: Ingress multicast byte count
	 * @ingress_broadcast_frames: Ingress broadcast frame count
	 * @ingress_broadcast_bytes: Ingress broadcast byte count
	 *
	 * Page_1 statistics structure
	 * @egress_all_frames: Egress frame count
	 * @egress_all_bytes: Egress byte count
	 * @egress_multicast_frames: Egress multicast frame count
	 * @egress_multicast_bytes: Egress multicast byte count
	 * @egress_broadcast_frames: Egress broadcast frame count
	 * @egress_broadcast_bytes: Egress broadcast byte count
	 *
	 * Page_2 statistics structure
	 * @ingress_filtered_frames: Ingress filtered frame count
	 * @ingress_discarded_frames: Ingress discarded frame count
	 * @ingress_nobuffer_discards: Ingress discarded frame count due to
	 *  lack of buffers.
	 * @egress_discarded_frames: Egress discarded frame count
	 * @egress_confirmed_frames: Egress confirmed frame count
	 */

	uint64_t counter0;
	uint64_t counter1;
	uint64_t counter2;
	uint64_t counter3;
	uint64_t counter4;
	uint64_t counter5;
	uint64_t counter6;
};

/**
 * dpni_get_statistics() - Get DPNI statistics
 * @mc_io:	Pointer to MC portal's I/O object
 * @cmd_flags:	Command flags; one or more of 'MC_CMD_FLAG_'
 * @token:	Token of DPNI object
 * @page:	Selects the statistics page to retrieve, see DPNI_GET_STATISTICS
 *		output. Pages are numbered 0 to 2.
 * @stat:	Structure containing the statistics
 *
 * Return:	'0' on Success; Error code otherwise.
 */
int dpni_get_statistics(struct fsl_mc_io *mc_io,
			uint32_t cmd_flags,
			uint16_t token,
			uint8_t page,
			struct dpni_statistics *stat);

/**
 * dpni_reset_statistics() - Clears DPNI statistics
 * @mc_io:	Pointer to MC portal's I/O object
 * @cmd_flags:	Command flags; one or more of 'MC_CMD_FLAG_'
 * @token:	Token of DPNI object
 *
 * Return:	'0' on Success; Error code otherwise.
 */
int dpni_reset_statistics(struct fsl_mc_io *mc_io,
			  uint32_t cmd_flags,
			  uint16_t token);
#endif /* _FSL_DPNI_H */
