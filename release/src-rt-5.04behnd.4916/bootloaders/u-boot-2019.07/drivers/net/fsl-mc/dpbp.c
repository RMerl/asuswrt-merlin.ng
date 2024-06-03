// SPDX-License-Identifier: GPL-2.0+
/*
 * Freescale Layerscape MC I/O wrapper
 *
 * Copyright 2013-2016 Freescale Semiconductor, Inc.
 * Copyright 2017 NXP
 */
#include <fsl-mc/fsl_mc_sys.h>
#include <fsl-mc/fsl_mc_cmd.h>
#include <fsl-mc/fsl_dpbp.h>

int dpbp_open(struct fsl_mc_io *mc_io,
	      uint32_t cmd_flags,
	      int dpbp_id,
	      uint16_t *token)
{
	struct mc_command cmd = { 0 };
	int err;

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPBP_CMDID_OPEN,
					  cmd_flags,
					  0);
	DPBP_CMD_OPEN(cmd, dpbp_id);

	/* send command to mc*/
	err = mc_send_command(mc_io, &cmd);
	if (err)
		return err;

	/* retrieve response parameters */
	*token = MC_CMD_HDR_READ_TOKEN(cmd.header);

	return err;
}

int dpbp_close(struct fsl_mc_io *mc_io,
	       uint32_t cmd_flags,
	       uint16_t token)
{
	struct mc_command cmd = { 0 };

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPBP_CMDID_CLOSE, cmd_flags,
					  token);

	/* send command to mc*/
	return mc_send_command(mc_io, &cmd);
}

int dpbp_create(struct fsl_mc_io *mc_io,
		uint16_t dprc_token,
		uint32_t cmd_flags,
		const struct dpbp_cfg *cfg,
		uint32_t *obj_id)
{
	struct mc_command cmd = { 0 };
	int err;

	(void)(cfg); /* unused */

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPBP_CMDID_CREATE,
					  cmd_flags,
					  dprc_token);

	/* send command to mc*/
	err = mc_send_command(mc_io, &cmd);
	if (err)
		return err;

	/* retrieve response parameters */
	MC_CMD_READ_OBJ_ID(cmd, *obj_id);

	return 0;
}

int dpbp_destroy(struct fsl_mc_io *mc_io,
		 uint16_t dprc_token,
		 uint32_t cmd_flags,
		 uint32_t obj_id)
{
	struct mc_command cmd = { 0 };

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPBP_CMDID_DESTROY,
					  cmd_flags,
					  dprc_token);

	/* set object id to destroy */
	CMD_DESTROY_SET_OBJ_ID_PARAM0(cmd, obj_id);

	/* send command to mc*/
	return mc_send_command(mc_io, &cmd);
}

int dpbp_enable(struct fsl_mc_io *mc_io,
		uint32_t cmd_flags,
		uint16_t token)
{
	struct mc_command cmd = { 0 };

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPBP_CMDID_ENABLE, cmd_flags,
					  token);

	/* send command to mc*/
	return mc_send_command(mc_io, &cmd);
}

int dpbp_disable(struct fsl_mc_io *mc_io,
		 uint32_t cmd_flags,
		 uint16_t token)
{
	struct mc_command cmd = { 0 };

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPBP_CMDID_DISABLE,
					  cmd_flags,
					  token);

	/* send command to mc*/
	return mc_send_command(mc_io, &cmd);
}

int dpbp_reset(struct fsl_mc_io *mc_io,
	       uint32_t cmd_flags,
	       uint16_t token)
{
	struct mc_command cmd = { 0 };

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPBP_CMDID_RESET,
					  cmd_flags,
					  token);

	/* send command to mc*/
	return mc_send_command(mc_io, &cmd);
}

int dpbp_get_attributes(struct fsl_mc_io *mc_io,
			uint32_t cmd_flags,
			uint16_t token,
			struct dpbp_attr *attr)
{
	struct mc_command cmd = { 0 };
	int err;

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPBP_CMDID_GET_ATTR,
					  cmd_flags,
					  token);

	/* send command to mc*/
	err = mc_send_command(mc_io, &cmd);
	if (err)
		return err;

	/* retrieve response parameters */
	DPBP_RSP_GET_ATTRIBUTES(cmd, attr);

	return 0;
}

int dpbp_get_api_version(struct fsl_mc_io *mc_io,
			 u32 cmd_flags,
			 u16 *major_ver,
			 u16 *minor_ver)
{
	struct mc_command cmd = { 0 };
	int err;

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPBP_CMDID_GET_API_VERSION,
					  cmd_flags, 0);

	/* send command to mc */
	err = mc_send_command(mc_io, &cmd);
	if (err)
		return err;

	/* retrieve response parameters */
	mc_cmd_read_api_version(&cmd, major_ver, minor_ver);

	return 0;
}
