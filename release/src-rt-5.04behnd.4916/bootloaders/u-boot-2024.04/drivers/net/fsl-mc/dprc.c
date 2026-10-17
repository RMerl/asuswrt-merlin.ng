// SPDX-License-Identifier: GPL-2.0+
/*
 * Freescale Layerscape MC I/O wrapper
 *
 * Copyright 2013-2016 Freescale Semiconductor, Inc.
 * Copyright 2017, 2023 NXP
 */

#include <fsl-mc/fsl_mc_sys.h>
#include <fsl-mc/fsl_mc_cmd.h>
#include <fsl-mc/fsl_dprc.h>

/**
 * dprc_get_container_id - Get container ID associated with a given portal.
 * @mc_io:		Pointer to Mc portal's I/O object
 * @cmd_flags:		Command flags; one or more of 'MC_CMD_FLAG_'
 * @container_id:	Requested container id
 *
 * Return:	'0' on Success; Error code otherwise.
 */
int dprc_get_container_id(struct fsl_mc_io *mc_io, u32 cmd_flags, int *container_id)
{
	struct mc_command cmd = { 0 };
	int err;

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPRC_CMDID_GET_CONT_ID,
					  cmd_flags,
					  0);

	/* send command to mc*/
	err = mc_send_command(mc_io, &cmd);
	if (err)
		return err;

	/* retrieve response parameters */
	*container_id = (int)mc_cmd_read_object_id(&cmd);

	return 0;
}

/**
 * dprc_open() - Open DPRC object for use
 * @mc_io:		Pointer to MC portal's I/O object
 * @cmd_flags:		Command flags; one or more of 'MC_CMD_FLAG_'
 * @container_id:	Container ID to open
 * @token:		Returned token of DPRC object
 *
 * Return:	'0' on Success; Error code otherwise.
 *
 * @warning	Required before any operation on the object.
 */
int dprc_open(struct fsl_mc_io *mc_io, u32 cmd_flags, int container_id, u16 *token)
{
	struct dprc_cmd_open *cmd_params;
	struct mc_command cmd = { 0 };
	int err;

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPRC_CMDID_OPEN, cmd_flags,
					  0);
	cmd_params = (struct dprc_cmd_open *)cmd.params;
	cmd_params->container_id = cpu_to_le32(container_id);

	/* send command to mc*/
	err = mc_send_command(mc_io, &cmd);
	if (err)
		return err;

	/* retrieve response parameters */
	*token = mc_cmd_hdr_read_token(&cmd);

	return 0;
}

/**
 * dprc_close() - Close the control session of the object
 * @mc_io:	Pointer to MC portal's I/O object
 * @cmd_flags:	Command flags; one or more of 'MC_CMD_FLAG_'
 * @token:	Token of DPRC object
 *
 * After this function is called, no further operations are
 * allowed on the object without opening a new control session.
 *
 * Return:	'0' on Success; Error code otherwise.
 */
int dprc_close(struct fsl_mc_io *mc_io, u32 cmd_flags, u16 token)
{
	struct mc_command cmd = { 0 };

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPRC_CMDID_CLOSE, cmd_flags,
					  token);

	/* send command to mc*/
	return mc_send_command(mc_io, &cmd);
}

/**
 * dprc_create_container() - Create child container
 * @mc_io:		Pointer to MC portal's I/O object
 * @cmd_flags:		Command flags; one or more of 'MC_CMD_FLAG_'
 * @token:		Token of DPRC object
 * @cfg:		Child container configuration
 * @child_container_id:	Returned child container ID
 * @child_portal_offset:Returned child portal offset from MC portal base
 *
 * Return:	'0' on Success; Error code otherwise.
 */
int dprc_create_container(struct fsl_mc_io *mc_io, u32 cmd_flags, u16 token,
			  struct dprc_cfg *cfg, int *child_container_id,
			  uint64_t *child_portal_offset)
{
	struct dprc_cmd_create_container *cmd_params;
	struct dprc_rsp_create_container *rsp_params;
	struct mc_command cmd = { 0 };
	int err, i;

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPRC_CMDID_CREATE_CONT,
					  cmd_flags, token);
	cmd_params = (struct dprc_cmd_create_container *)cmd.params;
	cmd_params->options = cpu_to_le32(cfg->options);
	cmd_params->icid = cpu_to_le32(cfg->icid);
	cmd_params->portal_id = cpu_to_le32(cfg->portal_id);
	for (i = 0; i < 16; i++)
		cmd_params->label[i] = cfg->label[i];

	/* send command to mc*/
	err = mc_send_command(mc_io, &cmd);
	if (err)
		return err;

	/* retrieve response parameters */
	rsp_params = (struct dprc_rsp_create_container *)cmd.params;
	*child_container_id = le32_to_cpu(rsp_params->child_container_id);
	*child_portal_offset = le64_to_cpu(rsp_params->child_portal_addr);

	return 0;
}

/**
 * dprc_destroy_container() - Destroy child container.
 * @mc_io:		Pointer to MC portal's I/O object
 * @cmd_flags:		Command flags; one or more of 'MC_CMD_FLAG_'
 * @token:		Token of DPRC object
 * @child_container_id:	ID of the container to destroy
 *
 * This function terminates the child container, so following this call the
 * child container ID becomes invalid.
 *
 * Notes:
 * - All resources and objects of the destroyed container are returned to the
 * parent container or destroyed if were created be the destroyed container.
 * - This function destroy all the child containers of the specified
 *   container prior to destroying the container itself.
 *
 * warning: Only the parent container is allowed to destroy a child policy
 *		Container 0 can't be destroyed
 *
 * Return:	'0' on Success; Error code otherwise.
 *
 */
int dprc_destroy_container(struct fsl_mc_io *mc_io, u32 cmd_flags, u16 token,
			   int child_container_id)
{
	struct dprc_cmd_destroy_container *cmd_params;
	struct mc_command cmd = { 0 };

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPRC_CMDID_DESTROY_CONT,
					  cmd_flags, token);
	cmd_params = (struct dprc_cmd_destroy_container *)cmd.params;
	cmd_params->child_container_id = cpu_to_le32(child_container_id);

	/* send command to mc*/
	return mc_send_command(mc_io, &cmd);
}

/**
 * dprc_connect() - Connect two endpoints to create a network link between them
 * @mc_io:	Pointer to MC portal's I/O object
 * @cmd_flags:	Command flags; one or more of 'MC_CMD_FLAG_'
 * @token:	Token of DPRC object
 * @endpoint1:	Endpoint 1 configuration parameters
 * @endpoint2:	Endpoint 2 configuration parameters
 * @cfg:	Connection configuration. The connection configuration
 *		is ignored for connections made to DPMAC objects, where
 *		rate is retrieved from the MAC configuration.
 *
 * Return:	'0' on Success; Error code otherwise.
 */
int dprc_connect(struct fsl_mc_io *mc_io, u32 cmd_flags, u16 token,
		 const struct dprc_endpoint *endpoint1,
		 const struct dprc_endpoint *endpoint2,
		 const struct dprc_connection_cfg *cfg)
{
	struct dprc_cmd_connect *cmd_params;
	struct mc_command cmd = { 0 };
	int i;

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPRC_CMDID_CONNECT,
					  cmd_flags,
					  token);
	cmd_params = (struct dprc_cmd_connect *)cmd.params;
	cmd_params->ep1_id = cpu_to_le32(endpoint1->id);
	cmd_params->ep1_interface_id = cpu_to_le16(endpoint1->if_id);
	cmd_params->ep2_id = cpu_to_le32(endpoint2->id);
	cmd_params->ep2_interface_id = cpu_to_le16(endpoint2->if_id);
	cmd_params->max_rate = cpu_to_le32(cfg->max_rate);
	cmd_params->committed_rate = cpu_to_le32(cfg->committed_rate);
	for (i = 0; i < 16; i++) {
		cmd_params->ep1_type[i] = endpoint1->type[i];
		cmd_params->ep2_type[i] = endpoint2->type[i];
	}

	/* send command to mc*/
	return mc_send_command(mc_io, &cmd);
}

/**
 * dprc_disconnect() - Disconnect one endpoint to remove its network connection
 * @mc_io:	Pointer to MC portal's I/O object
 * @cmd_flags:	Command flags; one or more of 'MC_CMD_FLAG_'
 * @token:	Token of DPRC object
 * @endpoint:	Endpoint configuration parameters
 *
 * Return:	'0' on Success; Error code otherwise.
 */
int dprc_disconnect(struct fsl_mc_io *mc_io, u32 cmd_flags, u16 token,
		    const struct dprc_endpoint *endpoint)
{
	struct dprc_cmd_disconnect *cmd_params;
	struct mc_command cmd = { 0 };
	int i;

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPRC_CMDID_DISCONNECT,
					  cmd_flags,
					  token);
	cmd_params = (struct dprc_cmd_disconnect *)cmd.params;
	cmd_params->id = cpu_to_le32(endpoint->id);
	cmd_params->interface_id = cpu_to_le32(endpoint->if_id);
	for (i = 0; i < 16; i++)
		cmd_params->type[i] = endpoint->type[i];

	/* send command to mc*/
	return mc_send_command(mc_io, &cmd);
}

/**
 * dprc_get_connection() - Get connected endpoint and link status if connection
 *			exists.
 * @mc_io:	Pointer to MC portal's I/O object
 * @cmd_flags:	Command flags; one or more of 'MC_CMD_FLAG_'
 * @token:	Token of DPRC object
 * @endpoint1:	Endpoint 1 configuration parameters
 * @endpoint2:	Returned endpoint 2 configuration parameters
 * @state:	Returned link state:
 *		1 - link is up;
 *		0 - link is down;
 *		-1 - no connection (endpoint2 information is irrelevant)
 *
 * Return:     '0' on Success; -ENAVAIL if connection does not exist.
 */
int dprc_get_connection(struct fsl_mc_io *mc_io, u32 cmd_flags, u16 token,
			const struct dprc_endpoint *endpoint1,
			struct dprc_endpoint *endpoint2, int *state)
{
	struct dprc_cmd_get_connection *cmd_params;
	struct dprc_rsp_get_connection *rsp_params;
	struct mc_command cmd = { 0 };
	int err, i;

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPRC_CMDID_GET_CONNECTION,
					  cmd_flags,
					  token);
	cmd_params = (struct dprc_cmd_get_connection *)cmd.params;
	cmd_params->ep1_id = cpu_to_le32(endpoint1->id);
	cmd_params->ep1_interface_id = cpu_to_le16(endpoint1->if_id);
	for (i = 0; i < 16; i++)
		cmd_params->ep1_type[i] = endpoint1->type[i];

	/* send command to mc*/
	err = mc_send_command(mc_io, &cmd);
	if (err)
		return err;

	/* retrieve response parameters */
	rsp_params = (struct dprc_rsp_get_connection *)cmd.params;
	endpoint2->id = le32_to_cpu(rsp_params->ep2_id);
	endpoint2->if_id = le16_to_cpu(rsp_params->ep2_interface_id);
	*state = le32_to_cpu(rsp_params->state);
	for (i = 0; i < 16; i++)
		endpoint2->type[i] = rsp_params->ep2_type[i];

	return 0;
}

/**
 * dprc_get_api_version - Get Data Path Resource Container API version
 * @mc_io:	Pointer to Mc portal's I/O object
 * @cmd_flags:	Command flags; one or more of 'MC_CMD_FLAG_'
 * @major_ver:	Major version of Data Path Resource Container API
 * @minor_ver:	Minor version of Data Path Resource Container API
 *
 * Return:	'0' on Success; Error code otherwise.
 */
int dprc_get_api_version(struct fsl_mc_io *mc_io, u32 cmd_flags,
			 u16 *major_ver, u16 *minor_ver)
{
	struct mc_command cmd = { 0 };
	int err;

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPRC_CMDID_GET_API_VERSION,
					  cmd_flags, 0);

	/* send command to mc */
	err = mc_send_command(mc_io, &cmd);
	if (err)
		return err;

	/* retrieve response parameters */
	mc_cmd_read_api_version(&cmd, major_ver, minor_ver);

	return 0;
}
