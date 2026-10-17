/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2013-2016 Freescale Semiconductor, Inc.
 * Copyright 2017, 2023 NXP
 */

#ifndef _FSL_DPIO_H
#define _FSL_DPIO_H

/* DPIO Version */
#define DPIO_VER_MAJOR				4
#define DPIO_VER_MINOR				2

/* Command IDs */
#define DPIO_CMDID_CLOSE					0x8001
#define DPIO_CMDID_OPEN						0x8031
#define DPIO_CMDID_CREATE					0x9031
#define DPIO_CMDID_DESTROY					0x9831
#define DPIO_CMDID_GET_API_VERSION				0xa031

#define DPIO_CMDID_ENABLE					0x0021
#define DPIO_CMDID_DISABLE					0x0031
#define DPIO_CMDID_GET_ATTR					0x0041

/* Macros for accessing command fields smaller than 1byte */
#define DPIO_MASK(field)        \
	GENMASK(DPIO_##field##_SHIFT + DPIO_##field##_SIZE - 1, \
		DPIO_##field##_SHIFT)
#define dpio_set_field(var, field, val) \
	((var) |= (((val) << DPIO_##field##_SHIFT) & DPIO_MASK(field)))
#define dpio_get_field(var, field)      \
	(((var) & DPIO_MASK(field)) >> DPIO_##field##_SHIFT)

#pragma pack(push, 1)
struct dpio_cmd_open {
	__le32 dpio_id;
};

#define DPIO_CHANNEL_MODE_SHIFT		0
#define DPIO_CHANNEL_MODE_SIZE		2

struct dpio_cmd_create {
	__le16 pad1;
	/* from LSB: channel_mode:2 */
	u8 channel_mode;
	u8 pad2;
	u8 num_priorities;
};

struct dpio_cmd_destroy {
	__le32 dpio_id;
};

#define DPIO_ATTR_CHANNEL_MODE_SHIFT	0
#define DPIO_ATTR_CHANNEL_MODE_SIZE	4

struct dpio_rsp_get_attr {
	__le32 id;
	__le16 qbman_portal_id;
	u8 num_priorities;
	/* from LSB: channel_mode:4 */
	u8 channel_mode;
	__le64 qbman_portal_ce_offset;
	__le64 qbman_portal_ci_offset;
	__le32 qbman_version;
	__le32 pad;
	__le32 clk;
};

#pragma pack(pop)

/* Data Path I/O Portal API
 * Contains initialization APIs and runtime control APIs for DPIO
 */

struct fsl_mc_io;

int dpio_open(struct fsl_mc_io *mc_io, u32 cmd_flags, int dpio_id,
	      u16 *token);

int dpio_close(struct fsl_mc_io *mc_io, u32 cmd_flags, u16 token);

/**
 * enum dpio_channel_mode - DPIO notification channel mode
 * @DPIO_NO_CHANNEL:	No support for notification channel
 * @DPIO_LOCAL_CHANNEL:	Notifications on data availability can be received by a
 *	dedicated channel in the DPIO; user should point the queue's
 *	destination in the relevant interface to this DPIO
 */
enum dpio_channel_mode {
	DPIO_NO_CHANNEL = 0,
	DPIO_LOCAL_CHANNEL = 1,
};

/**
 * struct dpio_cfg - Structure representing DPIO configuration
 * @channel_mode:	Notification channel mode
 * @num_priorities:	Number of priorities for the notification channel (1-8);
 *			relevant only if 'channel_mode = DPIO_LOCAL_CHANNEL'
 */
struct dpio_cfg {
	enum dpio_channel_mode channel_mode;
	u8 num_priorities;
};

int dpio_create(struct fsl_mc_io *mc_io, u16 dprc_token, u32 cmd_flags,
		const struct dpio_cfg *cfg, u32 *obj_id);

int dpio_destroy(struct fsl_mc_io *mc_io, u16 dprc_token, u32 cmd_flags,
		 u32 object_id);

int dpio_enable(struct fsl_mc_io *mc_io, u32 cmd_flags, u16 token);

int dpio_disable(struct fsl_mc_io *mc_io, u32 cmd_flags, u16 token);

/**
 * struct dpio_attr - Structure representing DPIO attributes
 * @id:				DPIO object ID
 * @qbman_portal_ce_offset:	Offset of the software portal cache-enabled area
 * @qbman_portal_ci_offset:	Offset of the software portal
 *				cache-inhibited area
 * @qbman_portal_id:		Software portal ID
 * @channel_mode:		Notification channel mode
 * @num_priorities:		Number of priorities for the notification
 *				channel (1-8); relevant only if
 *				'channel_mode = DPIO_LOCAL_CHANNEL'
 * @qbman_version:		QBMAN version
 */
struct dpio_attr {
	int id;
	u64 qbman_portal_ce_offset;
	u64 qbman_portal_ci_offset;
	u16 qbman_portal_id;
	enum dpio_channel_mode channel_mode;
	u8 num_priorities;
	u32 qbman_version;
	u32 clk;
};

int dpio_get_attributes(struct fsl_mc_io *mc_io, u32 cmd_flags, u16 token,
			struct dpio_attr *attr);

int dpio_get_api_version(struct fsl_mc_io *mc_io, u32 cmd_flags,
			 u16 *major_ver, u16 *minor_ver);
#endif /* _FSL_DPIO_H */
