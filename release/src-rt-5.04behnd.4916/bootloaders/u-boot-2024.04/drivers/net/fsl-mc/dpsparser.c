// SPDX-License-Identifier: GPL-2.0+
/*
 * Data Path Soft Parser
 *
 * Copyright 2018, 2023 NXP
 */
#include <fsl-mc/fsl_mc_sys.h>
#include <fsl-mc/fsl_mc_cmd.h>
#include <fsl-mc/fsl_dpsparser.h>

/**
 * dpsparser_open() - Open a control session for the specified object.
 * @mc_io:	Pointer to MC portal's I/O object
 * @cmd_flags:	Command flags; one or more of 'MC_CMD_FLAG_'
 * @token:	Returned token; use in subsequent API calls
 *
 * This function can be used to open a control session for an
 * already created object; an object may have been declared in
 * the DPL or by calling the dpsparser_create function.
 * This function returns a unique authentication token,
 * associated with the specific object ID and the specific MC
 * portal; this token must be used in all subsequent commands for
 * this specific object
 *
 * Return:	'0' on Success; Error code otherwise.
 */
int dpsparser_open(struct fsl_mc_io *mc_io, u32 cmd_flags, u16 *token)
{
	struct mc_command cmd = { 0 };
	int err;

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPSPARSER_CMDID_OPEN,
					  cmd_flags,
					  0);

	/* send command to mc*/
	err = mc_send_command(mc_io, &cmd);
	if (err)
		return err;

	/* retrieve response parameters */
	*token = mc_cmd_hdr_read_token(&cmd);

	return err;
}

/**
 * dpsparser_close() - Close the control session of the object
 * @mc_io:	Pointer to MC portal's I/O object
 * @cmd_flags:	Command flags; one or more of 'MC_CMD_FLAG_'
 * @token:	Token of DPSPARSER object
 *
 * After this function is called, no further operations are
 * allowed on the object without opening a new control session.
 *
 * Return:	'0' on Success; Error code otherwise.
 */
int dpsparser_close(struct fsl_mc_io *mc_io, u32 cmd_flags, u16 token)
{
	struct mc_command cmd = { 0 };

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPSPARSER_CMDID_CLOSE, cmd_flags,
					  token);

	/* send command to mc*/
	return mc_send_command(mc_io, &cmd);
}

/**
 * dpsparser_create() - Create the DPSPARSER object.
 * @mc_io:	Pointer to MC portal's I/O object
 * @cmd_flags:	Command flags; one or more of 'MC_CMD_FLAG_'
 * @token:	Returned token; use in subsequent API calls
 *
 * Create the DPSPARSER object, allocate required resources and
 * perform required initialization.
 *
 * The object can be created either by declaring it in the
 * DPL file, or by calling this function.
 * This function returns a unique authentication token,
 * associated with the specific object ID and the specific MC
 * portal; this token must be used in all subsequent calls to
 * this specific object. For objects that are created using the
 * DPL file, call dpsparser_open function to get an authentication
 * token first.
 *
 * Return:	'0' on Success; Error code otherwise.
 */
int dpsparser_create(struct fsl_mc_io *mc_io, u16 token, u32 cmd_flags,
		     u32 *obj_id)
{
	struct mc_command cmd = { 0 };
	int err;

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPSPARSER_CMDID_CREATE,
					  cmd_flags,
					  token);

	/* send command to mc*/
	err = mc_send_command(mc_io, &cmd);
	if (err)
		return err;

	/* retrieve response parameters */
	*obj_id = mc_cmd_read_object_id(&cmd);

	return 0;
}

/**
 * dpsparser_destroy() - Destroy the DPSPARSER object and release all its resources.
 * @mc_io:	Pointer to MC portal's I/O object
 * @cmd_flags:	Command flags; one or more of 'MC_CMD_FLAG_'
 * @token:	Token of DPSPARSER object
 *
 * Return:	'0' on Success; error code otherwise.
 */
int dpsparser_destroy(struct fsl_mc_io *mc_io, u16 token, u32 cmd_flags,
		      u32 obj_id)
{
	struct dpsparser_cmd_destroy *cmd_params;
	struct mc_command cmd = { 0 };

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPSPARSER_CMDID_DESTROY,
					  cmd_flags,
					  token);
	cmd_params = (struct dpsparser_cmd_destroy *)cmd.params;
	cmd_params->dpsparser_id = cpu_to_le32(obj_id);

	/* send command to mc*/
	return mc_send_command(mc_io, &cmd);
}

/**
 * dpsparser_apply_spb() - Applies the Soft Parser Blob loaded at specified address.
 * @mc_io:	Pointer to MC portal's I/O object
 * @cmd_flags:	Command flags; one or more of 'MC_CMD_FLAG_'
 * @token:	Token of DPSPARSER object
 * @blob_addr:	Blob loading address
 * @error: Error reported by MC related to SP Blob parsing and apply
 *
 * Return:	'0' on Success; error code otherwise.
 */
int dpsparser_apply_spb(struct fsl_mc_io *mc_io, u32 cmd_flags, u16 token,
			u64 blob_addr, u16 *error)
{
	struct dpsparser_rsp_blob_report_error *rsp_params;
	struct dpsparser_cmd_blob_set_address *cmd_params;
	struct mc_command cmd = { 0 };
	int err;

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPSPARSER_CMDID_APPLY_SPB,
					  cmd_flags,
					  token);
	cmd_params = (struct dpsparser_cmd_blob_set_address *)cmd.params;
	cmd_params->blob_addr = cpu_to_le64(blob_addr);

	/* send command to mc*/
	err = mc_send_command(mc_io, &cmd);
	if (err)
		return err;

	/* retrieve response parameters: MC error code */
	rsp_params = (struct dpsparser_rsp_blob_report_error *)cmd.params;
	*error = le16_to_cpu(rsp_params->error);

	return 0;
}

/**
 * dpsparser_get_api_version - Retrieve DPSPARSER Major and Minor version info.
 *
 * @mc_io:	Pointer to MC portal's I/O object
 * @cmd_flags:	Command flags; one or more of 'MC_CMD_FLAG_'
 * @major_ver:	DPSPARSER major version
 * @minor_ver:	DPSPARSER minor version
 *
 * Return:	'0' on Success; Error code otherwise.
 */
int dpsparser_get_api_version(struct fsl_mc_io *mc_io, u32 cmd_flags,
			      u16 *major_ver, u16 *minor_ver)
{
	struct mc_command cmd = { 0 };
	int err;

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPSPARSER_CMDID_GET_API_VERSION,
					  cmd_flags, 0);

	/* send command to mc */
	err = mc_send_command(mc_io, &cmd);
	if (err)
		return err;

	/* retrieve response parameters */
	mc_cmd_read_api_version(&cmd, major_ver, minor_ver);

	return 0;
}
