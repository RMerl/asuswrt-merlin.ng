/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Freescale Layerscape MC I/O wrapper
 *
 * Copyright 2015-2016 Freescale Semiconductor, Inc.
 * Copyright 2017 NXP
 * Author: Prabhakar Kushwaha <prabhakar@freescale.com>
 */

#ifndef __FSL_DPMAC_H
#define __FSL_DPMAC_H

/* DPMAC Version */
#define DPMAC_VER_MAJOR				4
#define DPMAC_VER_MINOR				2

/* Command IDs */
#define DPMAC_CMDID_CLOSE			0x8001
#define DPMAC_CMDID_OPEN			0x80c1
#define DPMAC_CMDID_CREATE			0x90c1
#define DPMAC_CMDID_DESTROY			0x98c1
#define DPMAC_CMDID_GET_API_VERSION             0xa0c1

#define DPMAC_CMDID_RESET			0x0051

#define DPMAC_CMDID_SET_LINK_STATE		0x0c31
#define DPMAC_CMDID_GET_COUNTER			0x0c41

/* Macros for accessing command fields smaller than 1byte */
#define DPMAC_MASK(field)        \
	GENMASK(DPMAC_##field##_SHIFT + DPMAC_##field##_SIZE - 1, \
		DPMAC_##field##_SHIFT)
#define dpmac_set_field(var, field, val) \
	((var) |= (((val) << DPMAC_##field##_SHIFT) & DPMAC_MASK(field)))
#define dpmac_get_field(var, field)      \
	(((var) & DPMAC_MASK(field)) >> DPMAC_##field##_SHIFT)

#pragma pack(push, 1)
struct dpmac_cmd_open {
	__le32 dpmac_id;
};

struct dpmac_cmd_create {
	__le32 mac_id;
};

struct dpmac_cmd_destroy {
	__le32 dpmac_id;
};

#define DPMAC_STATE_SIZE		1
#define DPMAC_STATE_SHIFT		0
#define DPMAC_STATE_VALID_SIZE		1
#define DPMAC_STATE_VALID_SHIFT		1

struct dpmac_cmd_set_link_state {
	__le64 options;
	__le32 rate;
	__le32 pad;
	/* only least significant bit is valid */
	u8 up;
	u8 pad0[7];
	__le64 supported;
	__le64 advertising;
};

struct dpmac_cmd_get_counter {
	u8 type;
};

struct dpmac_rsp_get_counter {
	__le64 pad;
	__le64 counter;
};

#pragma pack(pop)

/* Data Path MAC API
 * Contains initialization APIs and runtime control APIs for DPMAC
 */

struct fsl_mc_io;

int dpmac_open(struct fsl_mc_io *mc_io, u32 cmd_flags, int dpmac_id, u16 *token);

int dpmac_close(struct fsl_mc_io *mc_io, u32 cmd_flags, u16 token);

/**
 * struct dpmac_cfg - Structure representing DPMAC configuration
 * @mac_id:	Represents the Hardware MAC ID; in case of multiple WRIOP,
 *		the MAC IDs are continuous.
 *		For example:  2 WRIOPs, 16 MACs in each:
 *				MAC IDs for the 1st WRIOP: 1-16,
 *				MAC IDs for the 2nd WRIOP: 17-32.
 */
struct dpmac_cfg {
	int mac_id;
};

int dpmac_create(struct fsl_mc_io *mc_io, u16 dprc_token, u32 cmd_flags,
		 const struct dpmac_cfg *cfg, u32 *obj_id);

int dpmac_destroy(struct fsl_mc_io *mc_io, u16 dprc_token, u32 cmd_flags,
		  u32 object_id);

/**
 * enum dpmac_link_type -  DPMAC link type
 * @DPMAC_LINK_TYPE_NONE: No link
 * @DPMAC_LINK_TYPE_FIXED: Link is fixed type
 * @DPMAC_LINK_TYPE_PHY: Link by PHY ID
 * @DPMAC_LINK_TYPE_BACKPLANE: Backplane link type
 */
enum dpmac_link_type {
	DPMAC_LINK_TYPE_NONE,
	DPMAC_LINK_TYPE_FIXED,
	DPMAC_LINK_TYPE_PHY,
	DPMAC_LINK_TYPE_BACKPLANE
};

/**
 * enum dpmac_eth_if - DPMAC Ethrnet interface
 * @DPMAC_ETH_IF_MII: MII interface
 * @DPMAC_ETH_IF_RMII: RMII interface
 * @DPMAC_ETH_IF_SMII: SMII interface
 * @DPMAC_ETH_IF_GMII: GMII interface
 * @DPMAC_ETH_IF_RGMII: RGMII interface
 * @DPMAC_ETH_IF_SGMII: SGMII interface
 * @DPMAC_ETH_IF_QSGMII: QSGMII interface
 * @DPMAC_ETH_IF_XAUI: XAUI interface
 * @DPMAC_ETH_IF_XFI: XFI interface
 */
enum dpmac_eth_if {
	DPMAC_ETH_IF_MII,
	DPMAC_ETH_IF_RMII,
	DPMAC_ETH_IF_SMII,
	DPMAC_ETH_IF_GMII,
	DPMAC_ETH_IF_RGMII,
	DPMAC_ETH_IF_SGMII,
	DPMAC_ETH_IF_QSGMII,
	DPMAC_ETH_IF_XAUI,
	DPMAC_ETH_IF_XFI
};

/* DPMAC IRQ Index and Events */

/* IRQ index */
#define DPMAC_IRQ_INDEX						0
/* IRQ event - indicates a change in link state */
#define DPMAC_IRQ_EVENT_LINK_CFG_REQ		0x00000001
/* irq event - Indicates that the link state changed */
#define DPMAC_IRQ_EVENT_LINK_CHANGED		0x00000002

/**
 * struct dpmac_attr - Structure representing DPMAC attributes
 * @id:		DPMAC object ID
 * @phy_id:	PHY ID
 * @link_type: link type
 * @eth_if: Ethernet interface
 * @max_rate: Maximum supported rate - in Mbps
 * @version:	DPMAC version
 */
struct dpmac_attr {
	int			id;
	int			phy_id;
	enum dpmac_link_type	link_type;
	enum dpmac_eth_if	eth_if;
	u32		max_rate;
};

/* DPMAC link configuration/state options */

/* Enable auto-negotiation */
#define DPMAC_LINK_OPT_AUTONEG		0x0000000000000001ULL
/* Enable half-duplex mode */
#define DPMAC_LINK_OPT_HALF_DUPLEX	0x0000000000000002ULL
/* Enable pause frames */
#define DPMAC_LINK_OPT_PAUSE		0x0000000000000004ULL
/* Enable a-symmetric pause frames */
#define DPMAC_LINK_OPT_ASYM_PAUSE	0x0000000000000008ULL

/**
 * struct dpmac_link_state - DPMAC link configuration request
 * @rate: Rate in Mbps
 * @options: Enable/Disable DPMAC link cfg features (bitmap)
 * @up: Link state
 * @state_valid: Ignore/Update the state of the link
 * @supported: Speeds capability of the phy (bitmap)
 * @advertising: Speeds that are advertised for autoneg (bitmap)
 */
struct dpmac_link_state {
	u32 rate;
	u64 options;
	int up;
	int state_valid;
	u64 supported;
	u64 advertising;
};

int dpmac_set_link_state(struct fsl_mc_io *mc_io, u32 cmd_flags, u16 token,
			 struct dpmac_link_state *link_state);
/**
 * enum dpni_counter - DPNI counter types
 * @DPMAC_CNT_ING_FRAME_64: counts 64-octet frame, good or bad.
 * @DPMAC_CNT_ING_FRAME_127: counts 65- to 127-octet frame, good or bad.
 * @DPMAC_CNT_ING_FRAME_255: counts 128- to 255-octet frame, good or bad.
 * @DPMAC_CNT_ING_FRAME_511: counts 256- to 511-octet frame, good or bad.
 * @DPMAC_CNT_ING_FRAME_1023: counts 512- to 1023-octet frame, good or bad.
 * @DPMAC_CNT_ING_FRAME_1518: counts 1024- to 1518-octet frame, good or bad.
 * @DPMAC_CNT_ING_FRAME_1519_MAX: counts 1519-octet frame and larger
 *				  (up to max frame length specified),
 *				  good or bad.
 * @DPMAC_CNT_ING_FRAG: counts packet which is shorter than 64 octets received
 *			with a wrong CRC
 * @DPMAC_CNT_ING_JABBER: counts packet longer than the maximum frame length
 *			  specified, with a bad frame check sequence.
 * @DPMAC_CNT_ING_FRAME_DISCARD: counts dropped packet due to internal errors.
 *				 Occurs when a receive FIFO overflows.
 *				 Includes also packets truncated as a result of
 *				 the receive FIFO overflow.
 * @DPMAC_CNT_ING_ALIGN_ERR: counts frame with an alignment error
 *			     (optional used for wrong SFD)
 * @DPMAC_CNT_EGR_UNDERSIZED: counts packet transmitted that was less than 64
 *			      octets long with a good CRC.
 * @DPMAC_CNT_ING_OVERSIZED: counts packet longer than the maximum frame length
 *			     specified, with a good frame check sequence.
 * @DPMAC_CNT_ING_VALID_PAUSE_FRAME: counts valid pause frame (regular and PFC).
 * @DPMAC_CNT_EGR_VALID_PAUSE_FRAME: counts valid pause frame transmitted
 *				     (regular and PFC).
 * @DPMAC_CNT_ING_BYTE: counts octet received except preamble for all valid
 *				frames and valid pause frames.
 * @DPMAC_CNT_ING_MCAST_FRAME: counts received multicast frame
 * @DPMAC_CNT_ING_BCAST_FRAME: counts received broadcast frame
 * @DPMAC_CNT_ING_ALL_FRAME: counts each good or bad packet received.
 * @DPMAC_CNT_ING_UCAST_FRAME: counts received unicast frame
 * @DPMAC_CNT_ING_ERR_FRAME: counts frame received with an error
 *			     (except for undersized/fragment frame)
 * @DPMAC_CNT_EGR_BYTE: counts octet transmitted except preamble for all valid
 *			frames and valid pause frames transmitted.
 * @DPMAC_CNT_EGR_MCAST_FRAME: counts transmitted multicast frame
 * @DPMAC_CNT_EGR_BCAST_FRAME: counts transmitted broadcast frame
 * @DPMAC_CNT_EGR_UCAST_FRAME: counts transmitted unicast frame
 * @DPMAC_CNT_EGR_ERR_FRAME: counts frame transmitted with an error
 * @DPMAC_CNT_ING_GOOD_FRAME: counts frame received without error, including
 *			      pause frames.
 * @DPMAC_CNT_EGR_GOOD_FRAME: counts frames transmitted without error, including
 *			      pause frames.
 */
enum dpmac_counter {
	DPMAC_CNT_ING_FRAME_64,
	DPMAC_CNT_ING_FRAME_127,
	DPMAC_CNT_ING_FRAME_255,
	DPMAC_CNT_ING_FRAME_511,
	DPMAC_CNT_ING_FRAME_1023,
	DPMAC_CNT_ING_FRAME_1518,
	DPMAC_CNT_ING_FRAME_1519_MAX,
	DPMAC_CNT_ING_FRAG,
	DPMAC_CNT_ING_JABBER,
	DPMAC_CNT_ING_FRAME_DISCARD,
	DPMAC_CNT_ING_ALIGN_ERR,
	DPMAC_CNT_EGR_UNDERSIZED,
	DPMAC_CNT_ING_OVERSIZED,
	DPMAC_CNT_ING_VALID_PAUSE_FRAME,
	DPMAC_CNT_EGR_VALID_PAUSE_FRAME,
	DPMAC_CNT_ING_BYTE,
	DPMAC_CNT_ING_MCAST_FRAME,
	DPMAC_CNT_ING_BCAST_FRAME,
	DPMAC_CNT_ING_ALL_FRAME,
	DPMAC_CNT_ING_UCAST_FRAME,
	DPMAC_CNT_ING_ERR_FRAME,
	DPMAC_CNT_EGR_BYTE,
	DPMAC_CNT_EGR_MCAST_FRAME,
	DPMAC_CNT_EGR_BCAST_FRAME,
	DPMAC_CNT_EGR_UCAST_FRAME,
	DPMAC_CNT_EGR_ERR_FRAME,
	DPMAC_CNT_ING_GOOD_FRAME,
	DPMAC_CNT_EGR_GOOD_FRAME,
};

int dpmac_get_counter(struct fsl_mc_io *mc_io, u32 cmd_flags, u16 token,
		      enum dpmac_counter type, u64 *counter);

int dpmac_get_api_version(struct fsl_mc_io *mc_io, u32 cmd_flags,
			  u16 *major_ver, u16 *minor_ver);

#endif /* __FSL_DPMAC_H */
