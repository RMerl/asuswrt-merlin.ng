/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2013-2016 Freescale Semiconductor, Inc.
 * Copyright 2017 NXP
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
#define DPIO_CMDID_RESET					0x0051

/*                cmd, param, offset, width, type, arg_name */
#define DPIO_CMD_OPEN(cmd, dpio_id) \
	MC_CMD_OP(cmd, 0, 0,  32, int,     dpio_id)

/*                cmd, param, offset, width, type, arg_name */
#define DPIO_CMD_CREATE(cmd, cfg) \
do { \
	MC_CMD_OP(cmd, 0, 16, 2,  enum dpio_channel_mode,	\
					   cfg->channel_mode);\
	MC_CMD_OP(cmd, 0, 32, 8,  uint8_t, cfg->num_priorities);\
} while (0)

/*                cmd, param, offset, width, type, arg_name */
#define DPIO_RSP_GET_ATTR(cmd, attr) \
do { \
	MC_RSP_OP(cmd, 0, 0,  32, int,	    attr->id);\
	MC_RSP_OP(cmd, 0, 32, 16, uint16_t, attr->qbman_portal_id);\
	MC_RSP_OP(cmd, 0, 48, 8,  uint8_t,  attr->num_priorities);\
	MC_RSP_OP(cmd, 0, 56, 4,  enum dpio_channel_mode, attr->channel_mode);\
	MC_RSP_OP(cmd, 1, 0,  64, uint64_t, attr->qbman_portal_ce_offset);\
	MC_RSP_OP(cmd, 2, 0,  64, uint64_t, attr->qbman_portal_ci_offset);\
	MC_RSP_OP(cmd, 3, 32, 32, uint32_t, attr->qbman_version);\
} while (0)

/* Data Path I/O Portal API
 * Contains initialization APIs and runtime control APIs for DPIO
 */

struct fsl_mc_io;

/**
 * dpio_open() - Open a control session for the specified object
 * @mc_io:	Pointer to MC portal's I/O object
 * @cmd_flags:	Command flags; one or more of 'MC_CMD_FLAG_'
 * @dpio_id:	DPIO unique ID
 * @token:	Returned token; use in subsequent API calls
 *
 * This function can be used to open a control session for an
 * already created object; an object may have been declared in
 * the DPL or by calling the dpio_create() function.
 * This function returns a unique authentication token,
 * associated with the specific object ID and the specific MC
 * portal; this token must be used in all subsequent commands for
 * this specific object.
 *
 * Return:	'0' on Success; Error code otherwise.
 */
int dpio_open(struct fsl_mc_io	*mc_io,
	      uint32_t		cmd_flags,
	      uint32_t		dpio_id,
	      uint16_t		*token);

/**
 * dpio_close() - Close the control session of the object
 * @mc_io:	Pointer to MC portal's I/O object
 * @cmd_flags:	Command flags; one or more of 'MC_CMD_FLAG_'
 * @token:	Token of DPIO object
 *
 * Return:	'0' on Success; Error code otherwise.
 */
int dpio_close(struct fsl_mc_io	*mc_io,
	       uint32_t		cmd_flags,
	       uint16_t		token);

/**
 * enum dpio_channel_mode - DPIO notification channel mode
 * @DPIO_NO_CHANNEL: No support for notification channel
 * @DPIO_LOCAL_CHANNEL: Notifications on data availability can be received by a
 *	dedicated channel in the DPIO; user should point the queue's
 *	destination in the relevant interface to this DPIO
 */
enum dpio_channel_mode {
	DPIO_NO_CHANNEL = 0,
	DPIO_LOCAL_CHANNEL = 1,
};

/**
 * struct dpio_cfg - Structure representing DPIO configuration
 * @channel_mode: Notification channel mode
 * @num_priorities: Number of priorities for the notification channel (1-8);
 *			relevant only if 'channel_mode = DPIO_LOCAL_CHANNEL'
 */
struct dpio_cfg {
	enum dpio_channel_mode	channel_mode;
	uint8_t		num_priorities;
};

/**
 * dpio_create() - Create the DPIO object.
 * @mc_io:	Pointer to MC portal's I/O object
 * @token:	Authentication token.
 * @cmd_flags:	Command flags; one or more of 'MC_CMD_FLAG_'
 * @cfg:	Configuration structure
 * @obj_id:	Returned obj_id; use in subsequent API calls
 *
 * Create the DPIO object, allocate required resources and
 * perform required initialization.
 *
 * The object can be created either by declaring it in the
 * DPL file, or by calling this function.
 *
 * This function returns a unique authentication token,
 * associated with the specific object ID and the specific MC
 * portal; this token must be used in all subsequent calls to
 * this specific object. For objects that are created using the
 * DPL file, call dpio_open() function to get an authentication
 * token first.
 *
 * Return:	'0' on Success; Error code otherwise.
 */
int dpio_create(struct fsl_mc_io	*mc_io,
		uint16_t		token,
		uint32_t		cmd_flags,
		const struct dpio_cfg	*cfg,
		uint32_t		*obj_id);

/**
 * dpio_destroy() - Destroy the DPIO object and release all its resources.
 * @mc_io:	Pointer to MC portal's I/O object
 * @token:	Authentication token.
 * @cmd_flags:	Command flags; one or more of 'MC_CMD_FLAG_'
 * @obj_id:	Object ID of DPIO
 *
 * Return:	'0' on Success; Error code otherwise
 */
int dpio_destroy(struct fsl_mc_io	*mc_io,
		 uint16_t		token,
		 uint32_t		cmd_flags,
		 uint32_t		obj_id);

/**
 * dpio_enable() - Enable the DPIO, allow I/O portal operations.
 * @mc_io:	Pointer to MC portal's I/O object
 * @cmd_flags:	Command flags; one or more of 'MC_CMD_FLAG_'
 * @token:	Token of DPIO object
 *
 * Return:	'0' on Success; Error code otherwise
 */
int dpio_enable(struct fsl_mc_io	*mc_io,
		uint32_t		cmd_flags,
		uint16_t		token);

/**
 * dpio_disable() - Disable the DPIO, stop any I/O portal operation.
 * @mc_io:	Pointer to MC portal's I/O object
 * @cmd_flags:	Command flags; one or more of 'MC_CMD_FLAG_'
 * @token:	Token of DPIO object
 *
 * Return:	'0' on Success; Error code otherwise
 */
int dpio_disable(struct fsl_mc_io	*mc_io,
		 uint32_t		cmd_flags,
		 uint16_t		token);

/**
 * dpio_reset() - Reset the DPIO, returns the object to initial state.
 * @mc_io:	Pointer to MC portal's I/O object
 * @cmd_flags:	Command flags; one or more of 'MC_CMD_FLAG_'
 * @token:	Token of DPIO object
 *
 * Return:	'0' on Success; Error code otherwise.
 */
int dpio_reset(struct fsl_mc_io	*mc_io,
	       uint32_t			cmd_flags,
	       uint16_t		token);

/**
 * struct dpio_attr - Structure representing DPIO attributes
 * @id: DPIO object ID
 * @version: DPIO version
 * @qbman_portal_ce_offset: offset of the software portal cache-enabled area
 * @qbman_portal_ci_offset: offset of the software portal cache-inhibited area
 * @qbman_portal_id: Software portal ID
 * @channel_mode: Notification channel mode
 * @num_priorities: Number of priorities for the notification channel (1-8);
 *			relevant only if 'channel_mode = DPIO_LOCAL_CHANNEL'
 * @qbman_version: QBMAN version
 */
struct dpio_attr {
	uint32_t id;
	uint64_t qbman_portal_ce_offset;
	uint64_t qbman_portal_ci_offset;
	uint16_t qbman_portal_id;
	enum dpio_channel_mode channel_mode;
	uint8_t num_priorities;
	uint32_t		qbman_version;
};

/**
 * dpio_get_attributes() - Retrieve DPIO attributes
 * @mc_io:	Pointer to MC portal's I/O object
 * @cmd_flags:	Command flags; one or more of 'MC_CMD_FLAG_'
 * @token:	Token of DPIO object
 * @attr:	Returned object's attributes
 *
 * Return:	'0' on Success; Error code otherwise
 */
int dpio_get_attributes(struct fsl_mc_io	*mc_io,
			uint32_t		cmd_flags,
			uint16_t		token,
			struct dpio_attr	*attr);

/**
 * dpio_get_api_version - Retrieve DPIO Major and Minor version info.
 *
 * @mc_io:	Pointer to MC portal's I/O object
 * @cmd_flags:	Command flags; one or more of 'MC_CMD_FLAG_'
 * @major_ver:	DPIO major version
 * @minor_ver:	DPIO minor version
 *
 * Return:	'0' on Success; Error code otherwise.
 */
int dpio_get_api_version(struct fsl_mc_io *mc_io,
			 u32 cmd_flags,
			 u16 *major_ver,
			 u16 *minor_ver);

#endif /* _FSL_DPIO_H */
