// SPDX-License-Identifier: GPL-2.0+
/*
 * Freescale Layerscape MC I/O wrapper
 *
 * Copyright 2015-2016 Freescale Semiconductor, Inc.
 * Copyright 2017 NXP
 * Author: Prabhakar Kushwaha <prabhakar@freescale.com>
 */

#include <fsl-mc/fsl_mc_sys.h>
#include <fsl-mc/fsl_mc_cmd.h>
#include <fsl-mc/fsl_dpmac.h>

/**
 * dpmac_open() - Open a control session for the specified object.
 * @mc_io:	Pointer to MC portal's I/O object
 * @cmd_flags:	Command flags; one or more of 'MC_CMD_FLAG_'
 * @dpmac_id:	DPMAC unique ID
 * @token:	Returned token; use in subsequent API calls
 *
 * This function can be used to open a control session for an
 * already created object; an object may have been declared in
 * the DPL or by calling the dpmac_create function.
 * This function returns a unique authentication token,
 * associated with the specific object ID and the specific MC
 * portal; this token must be used in all subsequent commands for
 * this specific object
 *
 * Return:	'0' on Success; Error code otherwise.
 */
int dpmac_open(struct fsl_mc_io *mc_io, u32 cmd_flags, int dpmac_id, u16 *token)
{
	struct dpmac_cmd_open *cmd_params;
	struct mc_command cmd = { 0 };
	int err;

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPMAC_CMDID_OPEN, cmd_flags, 0);
	cmd_params = (struct dpmac_cmd_open *)cmd.params;
	cmd_params->dpmac_id = cpu_to_le32(dpmac_id);

	/* send command to mc*/
	err = mc_send_command(mc_io, &cmd);
	if (err)
		return err;

	/* retrieve response parameters */
	*token = mc_cmd_hdr_read_token(&cmd);

	return err;
}

/**
 * dpmac_close() - Close the control session of the object
 * @mc_io:	Pointer to MC portal's I/O object
 * @cmd_flags:	Command flags; one or more of 'MC_CMD_FLAG_'
 * @token:	Token of DPMAC object
 *
 * After this function is called, no further operations are
 * allowed on the object without opening a new control session.
 *
 * Return:	'0' on Success; Error code otherwise.
 */
int dpmac_close(struct fsl_mc_io *mc_io, u32 cmd_flags, u16 token)
{
	struct mc_command cmd = { 0 };

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPMAC_CMDID_CLOSE, cmd_flags, token);

	/* send command to mc*/
	return mc_send_command(mc_io, &cmd);
}

/**
 * dpmac_create() - Create the DPMAC object.
 * @mc_io:	Pointer to MC portal's I/O object
 * @dprc_token: Parent container token; '0' for default container
 * @cmd_flags:	Command flags; one or more of 'MC_CMD_FLAG_'
 * @cfg:	Configuration structure
 * @obj_id:	Returned object id
 *
 * Create the DPMAC object, allocate required resources and
 * perform required initialization.
 *
 * The function accepts an authentication token of a parent
 * container that this object should be assigned to. The token
 * can be '0' so the object will be assigned to the default container.
 * The newly created object can be opened with the returned
 * object id and using the container's associated tokens and MC portals.
 *
 * Return:	'0' on Success; Error code otherwise.
 */
int dpmac_create(struct fsl_mc_io *mc_io, u16 dprc_token, u32 cmd_flags,
		 const struct dpmac_cfg *cfg, u32 *obj_id)
{
	struct dpmac_cmd_create *cmd_params;
	struct mc_command cmd = { 0 };
	int err;

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPMAC_CMDID_CREATE, cmd_flags, dprc_token);
	cmd_params = (struct dpmac_cmd_create *)cmd.params;
	cmd_params->mac_id = cpu_to_le32(cfg->mac_id);

	/* send command to mc*/
	err = mc_send_command(mc_io, &cmd);
	if (err)
		return err;

	/* retrieve response parameters */
	*obj_id = mc_cmd_read_object_id(&cmd);

	return 0;
}

/**
 * dpmac_destroy() - Destroy the DPMAC object and release all its resources.
 * @mc_io:	Pointer to MC portal's I/O object
 * @dprc_token: Parent container token; '0' for default container
 * @cmd_flags:	Command flags; one or more of 'MC_CMD_FLAG_'
 * @object_id:	The object id; it must be a valid id within the container that
 * created this object;
 *
 * The function accepts the authentication token of the parent container that
 * created the object (not the one that currently owns the object). The object
 * is searched within parent using the provided 'object_id'.
 * All tokens to the object must be closed before calling destroy.
 *
 * Return:	'0' on Success; error code otherwise.
 */
int dpmac_destroy(struct fsl_mc_io *mc_io, u16 dprc_token, u32 cmd_flags,
		  u32 object_id)
{
	struct dpmac_cmd_destroy *cmd_params;
	struct mc_command cmd = { 0 };

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPMAC_CMDID_DESTROY,
					  cmd_flags,
					  dprc_token);
	cmd_params = (struct dpmac_cmd_destroy *)cmd.params;
	cmd_params->dpmac_id = cpu_to_le32(object_id);

	/* send command to mc*/
	return mc_send_command(mc_io, &cmd);
}

/**
 * dpmac_set_link_state() - Set the Ethernet link status
 * @mc_io:	Pointer to opaque I/O object
 * @cmd_flags:	Command flags; one or more of 'MC_CMD_FLAG_'
 * @token:	Token of DPMAC object
 * @link_state:	Link state configuration
 *
 * Return:	'0' on Success; Error code otherwise.
 */
int dpmac_set_link_state(struct fsl_mc_io *mc_io, u32 cmd_flags, u16 token,
			 struct dpmac_link_state *link_state)
{
	struct dpmac_cmd_set_link_state *cmd_params;
	struct mc_command cmd = { 0 };

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPMAC_CMDID_SET_LINK_STATE, cmd_flags, token);
	cmd_params = (struct dpmac_cmd_set_link_state *)cmd.params;
	cmd_params->options = cpu_to_le64(link_state->options);
	cmd_params->rate = cpu_to_le32(link_state->rate);
	cmd_params->up = dpmac_get_field(link_state->up, STATE);
	dpmac_set_field(cmd_params->up, STATE_VALID, link_state->state_valid);
	cmd_params->supported = cpu_to_le64(link_state->supported);
	cmd_params->advertising = cpu_to_le64(link_state->advertising);

	/* send command to mc*/
	return mc_send_command(mc_io, &cmd);
}

/**
 * dpmac_get_counter() - Read a specific DPMAC counter
 * @mc_io:	Pointer to opaque I/O object
 * @cmd_flags:	Command flags; one or more of 'MC_CMD_FLAG_'
 * @token:	Token of DPMAC object
 * @type:	The requested counter
 * @counter:	Returned counter value
 *
 * Return:	The requested counter; '0' otherwise.
 */
int dpmac_get_counter(struct fsl_mc_io *mc_io, u32 cmd_flags, u16 token,
		      enum dpmac_counter type, uint64_t *counter)
{
	struct dpmac_cmd_get_counter *dpmac_cmd;
	struct dpmac_rsp_get_counter *dpmac_rsp;
	struct mc_command cmd = { 0 };
	int err = 0;

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPMAC_CMDID_GET_COUNTER,
					  cmd_flags,
					  token);
	dpmac_cmd = (struct dpmac_cmd_get_counter *)cmd.params;
	dpmac_cmd->type = type;

	/* send command to mc*/
	err = mc_send_command(mc_io, &cmd);
	if (err)
		return err;

	dpmac_rsp = (struct dpmac_rsp_get_counter *)cmd.params;
	*counter = le64_to_cpu(dpmac_rsp->counter);

	return 0;
}

/**
 * dpmac_get_api_version() - Get Data Path MAC version
 * @mc_io:	Pointer to MC portal's I/O object
 * @cmd_flags:	Command flags; one or more of 'MC_CMD_FLAG_'
 * @major_ver:	Major version of data path mac API
 * @minor_ver:	Minor version of data path mac API
 *
 * Return:  '0' on Success; Error code otherwise.
 */
int dpmac_get_api_version(struct fsl_mc_io *mc_io, u32 cmd_flags,
			  u16 *major_ver, u16 *minor_ver)
{
	struct mc_command cmd = { 0 };
	int err;

	cmd.header = mc_encode_cmd_header(DPMAC_CMDID_GET_API_VERSION,
					cmd_flags,
					0);

	err = mc_send_command(mc_io, &cmd);
	if (err)
		return err;

	mc_cmd_read_api_version(&cmd, major_ver, minor_ver);

	return 0;
}
