// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2013-2016 Freescale Semiconductor, Inc.
 * Copyright 2017, 2023 NXP
 */

#include <fsl-mc/fsl_mc_sys.h>
#include <fsl-mc/fsl_mc_cmd.h>
#include <fsl-mc/fsl_dpio.h>

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
 * associated with the specific object ID and any MC portals
 * assigned to the parent container; this token must be used in
 * all subsequent commands for this specific object.
 *
 * Return:	'0' on Success; Error code otherwise.
 */
int dpio_open(struct fsl_mc_io *mc_io, u32 cmd_flags, int dpio_id,
	      u16 *token)
{
	struct dpio_cmd_open *cmd_params;
	struct mc_command cmd = { 0 };
	int err;

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPIO_CMDID_OPEN,
					  cmd_flags,
					  0);
	cmd_params = (struct dpio_cmd_open *)cmd.params;
	cmd_params->dpio_id = cpu_to_le32(dpio_id);

	/* send command to mc*/
	err = mc_send_command(mc_io, &cmd);
	if (err)
		return err;

	/* retrieve response parameters */
	*token = mc_cmd_hdr_read_token(&cmd);

	return 0;
}

/**
 * dpio_close() - Close the control session of the object
 * @mc_io:	Pointer to MC portal's I/O object
 * @cmd_flags:	Command flags; one or more of 'MC_CMD_FLAG_'
 * @token:	Token of DPIO object
 *
 * Return:	'0' on Success; Error code otherwise.
 */
int dpio_close(struct fsl_mc_io *mc_io, u32 cmd_flags, u16 token)
{
	struct mc_command cmd = { 0 };

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPIO_CMDID_CLOSE,
					  cmd_flags,
					  token);

	/* send command to mc*/
	return mc_send_command(mc_io, &cmd);
}

/**
 * dpio_create() - Create the DPIO object.
 * @mc_io:	Pointer to MC portal's I/O object
 * @dprc_token:	Parent container token; '0' for default container
 * @cmd_flags:	Command flags; one or more of 'MC_CMD_FLAG_'
 * @cfg:	Configuration structure
 * @obj_id:	Returned object id
 *
 * Create the DPIO object, allocate required resources and
 * perform required initialization.
 *
 * The object can be created either by declaring it in the
 * DPL file, or by calling this function.
 *
 * The function accepts an authentication token of a parent
 * container that this object should be assigned to. The token
 * can be '0' so the object will be assigned to the default container.
 * The newly created object can be opened with the returned
 * object id and using the container's associated tokens and MC portals.
 *
 * Return:	'0' on Success; Error code otherwise.
 */
int dpio_create(struct fsl_mc_io *mc_io, u16 dprc_token, u32 cmd_flags,
		const struct dpio_cfg *cfg, u32 *obj_id)
{
	struct dpio_cmd_create *cmd_params;
	struct mc_command cmd = { 0 };
	int err;

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPIO_CMDID_CREATE,
					  cmd_flags,
					  dprc_token);
	cmd_params = (struct dpio_cmd_create *)cmd.params;
	cmd_params->num_priorities = cfg->num_priorities;
	dpio_set_field(cmd_params->channel_mode,
		       CHANNEL_MODE,
		       cfg->channel_mode);

	/* send command to mc*/
	err = mc_send_command(mc_io, &cmd);
	if (err)
		return err;

	/* retrieve response parameters */
	*obj_id = mc_cmd_read_object_id(&cmd);

	return 0;
}

/**
 * dpio_destroy() - Destroy the DPIO object and release all its resources.
 * @mc_io:	Pointer to MC portal's I/O object
 * @dprc_token: Parent container token; '0' for default container
 * @cmd_flags:	Command flags; one or more of 'MC_CMD_FLAG_'
 * @object_id:	The object id; it must be a valid id within the container that
 *		created this object;
 *
 * The function accepts the authentication token of the parent container that
 * created the object (not the one that currently owns the object). The object
 * is searched within parent using the provided 'object_id'.
 * All tokens to the object must be closed before calling destroy.
 *
 * Return:	'0' on Success; Error code otherwise
 */
int dpio_destroy(struct fsl_mc_io *mc_io, u16 dprc_token, u32 cmd_flags,
		 u32 object_id)
{
	struct dpio_cmd_destroy *cmd_params;
	struct mc_command cmd = { 0 };

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPIO_CMDID_DESTROY,
			cmd_flags,
			dprc_token);

	/* set object id to destroy */
	cmd_params = (struct dpio_cmd_destroy *)cmd.params;
	cmd_params->dpio_id = cpu_to_le32(object_id);

	/* send command to mc*/
	return mc_send_command(mc_io, &cmd);
}

/**
 * dpio_enable() - Enable the DPIO, allow I/O portal operations.
 * @mc_io:	Pointer to MC portal's I/O object
 * @cmd_flags:	Command flags; one or more of 'MC_CMD_FLAG_'
 * @token:	Token of DPIO object
 *
 * Return:	'0' on Success; Error code otherwise
 */
int dpio_enable(struct fsl_mc_io *mc_io, u32 cmd_flags, u16 token)
{
	struct mc_command cmd = { 0 };

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPIO_CMDID_ENABLE,
					  cmd_flags,
					  token);

	/* send command to mc*/
	return mc_send_command(mc_io, &cmd);
}

/**
 * dpio_disable() - Disable the DPIO, stop any I/O portal operation.
 * @mc_io:	Pointer to MC portal's I/O object
 * @cmd_flags:	Command flags; one or more of 'MC_CMD_FLAG_'
 * @token:	Token of DPIO object
 *
 * Return:	'0' on Success; Error code otherwise
 */
int dpio_disable(struct fsl_mc_io *mc_io, u32 cmd_flags, u16 token)
{
	struct mc_command cmd = { 0 };

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPIO_CMDID_DISABLE,
					  cmd_flags,
					  token);

	/* send command to mc*/
	return mc_send_command(mc_io, &cmd);
}

/**
 * dpio_get_attributes() - Retrieve DPIO attributes
 * @mc_io:	Pointer to MC portal's I/O object
 * @cmd_flags:	Command flags; one or more of 'MC_CMD_FLAG_'
 * @token:	Token of DPIO object
 * @attr:	Returned object's attributes
 *
 * Return:	'0' on Success; Error code otherwise
 */
int dpio_get_attributes(struct fsl_mc_io *mc_io, u32 cmd_flags, u16 token,
			struct dpio_attr *attr)
{
	struct dpio_rsp_get_attr *rsp_params;
	struct mc_command cmd = { 0 };
	int err;

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPIO_CMDID_GET_ATTR,
					  cmd_flags,
					  token);

	/* send command to mc*/
	err = mc_send_command(mc_io, &cmd);
	if (err)
		return err;

	/* retrieve response parameters */
	rsp_params = (struct dpio_rsp_get_attr *)cmd.params;
	attr->id = le32_to_cpu(rsp_params->id);
	attr->qbman_portal_id = le16_to_cpu(rsp_params->qbman_portal_id);
	attr->num_priorities = rsp_params->num_priorities;
	attr->qbman_portal_ce_offset = le64_to_cpu(rsp_params->qbman_portal_ce_offset);
	attr->qbman_portal_ci_offset = le64_to_cpu(rsp_params->qbman_portal_ci_offset);
	attr->qbman_version = le32_to_cpu(rsp_params->qbman_version);
	attr->clk = le32_to_cpu(rsp_params->clk);
	attr->channel_mode = dpio_get_field(rsp_params->channel_mode, ATTR_CHANNEL_MODE);

	return 0;
}

/**
 * dpio_get_api_version() - Get Data Path I/O API version
 * @mc_io:	Pointer to MC portal's I/O object
 * @cmd_flags:	Command flags; one or more of 'MC_CMD_FLAG_'
 * @major_ver:	Major version of data path i/o API
 * @minor_ver:	Minor version of data path i/o API
 *
 * Return:  '0' on Success; Error code otherwise.
 */
int dpio_get_api_version(struct fsl_mc_io *mc_io, u32 cmd_flags,
			 u16 *major_ver, u16 *minor_ver)
{
	struct mc_command cmd = { 0 };
	int err;

	cmd.header = mc_encode_cmd_header(DPIO_CMDID_GET_API_VERSION,
					cmd_flags,
					0);

	err = mc_send_command(mc_io, &cmd);
	if (err)
		return err;

	mc_cmd_read_api_version(&cmd, major_ver, minor_ver);

	return 0;
}
