// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2013-2016 Freescale Semiconductor, Inc.
 * Copyright 2017, 2023 NXP
 */

#include <fsl-mc/fsl_mc_sys.h>
#include <fsl-mc/fsl_mc_cmd.h>
#include <fsl-mc/fsl_dpni.h>

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
int dpni_open(struct fsl_mc_io *mc_io, u32 cmd_flags, int dpni_id, u16 *token)
{
	struct dpni_cmd_open *cmd_params;
	struct mc_command cmd = { 0 };

	int err;

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPNI_CMDID_OPEN,
					  cmd_flags,
					  0);
	cmd_params = (struct dpni_cmd_open *)cmd.params;
	cmd_params->dpni_id = cpu_to_le32(dpni_id);

	/* send command to mc*/
	err = mc_send_command(mc_io, &cmd);
	if (err)
		return err;

	/* retrieve response parameters */
	*token = mc_cmd_hdr_read_token(&cmd);

	return 0;
}

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
int dpni_close(struct fsl_mc_io *mc_io, u32 cmd_flags, u16 token)
{
	struct mc_command cmd = { 0 };

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPNI_CMDID_CLOSE,
					  cmd_flags,
					  token);

	/* send command to mc*/
	return mc_send_command(mc_io, &cmd);
}

/**
 * dpni_create() - Create the DPNI object
 * @mc_io:	Pointer to MC portal's I/O object
 * @dprc_token:	Parent container token; '0' for default container
 * @cmd_flags:	Command flags; one or more of 'MC_CMD_FLAG_'
 * @cfg:	Configuration structure
 * @obj_id:	Returned object id
 *
 * Create the DPNI object, allocate required resources and
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
int dpni_create(struct fsl_mc_io *mc_io, u16 dprc_token, u32 cmd_flags,
		const struct dpni_cfg *cfg, u32 *obj_id)
{
	struct dpni_cmd_create *cmd_params;
	struct mc_command cmd = { 0 };
	int err;

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPNI_CMDID_CREATE,
					  cmd_flags,
					  dprc_token);
	cmd_params = (struct dpni_cmd_create *)cmd.params;
	cmd_params->options = cpu_to_le32(cfg->options);
	cmd_params->num_queues = cfg->num_queues;
	cmd_params->num_tcs = cfg->num_tcs;
	cmd_params->mac_filter_entries = cfg->mac_filter_entries;
	cmd_params->num_rx_tcs = cfg->num_rx_tcs;
	cmd_params->vlan_filter_entries =  cfg->vlan_filter_entries;
	cmd_params->qos_entries = cfg->qos_entries;
	cmd_params->fs_entries = cpu_to_le16(cfg->fs_entries);
	cmd_params->num_cgs = cfg->num_cgs;
	cmd_params->num_opr = cfg->num_opr;
	cmd_params->dist_key_size = cfg->dist_key_size;
	cmd_params->num_channels = cfg->num_channels;

	/* send command to mc*/
	err = mc_send_command(mc_io, &cmd);
	if (err)
		return err;

	/* retrieve response parameters */
	*obj_id = mc_cmd_read_object_id(&cmd);

	return 0;
}

/**
 * dpni_destroy() - Destroy the DPNI object and release all its resources.
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
int dpni_destroy(struct fsl_mc_io *mc_io, u16 dprc_token, u32 cmd_flags,
		 u32 object_id)
{
	struct dpni_cmd_destroy *cmd_params;
	struct mc_command cmd = { 0 };

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPNI_CMDID_DESTROY,
					  cmd_flags,
					  dprc_token);
	/* set object id to destroy */
	cmd_params = (struct dpni_cmd_destroy *)cmd.params;
	cmd_params->dpni_id = cpu_to_le32(object_id);

	/* send command to mc*/
	return mc_send_command(mc_io, &cmd);
}

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
int dpni_set_pools(struct fsl_mc_io *mc_io, u32 cmd_flags, u16 token,
		   const struct dpni_pools_cfg *cfg)
{
	struct mc_command cmd = { 0 };
	struct dpni_cmd_set_pools *cmd_params;
	int i;

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPNI_CMDID_SET_POOLS,
					  cmd_flags,
					  token);
	cmd_params = (struct dpni_cmd_set_pools *)cmd.params;
	cmd_params->num_dpbp = cfg->num_dpbp;
	cmd_params->pool_options = cfg->pool_options;
	for (i = 0; i < DPNI_MAX_DPBP; i++) {
		cmd_params->pool[i].dpbp_id =
			cpu_to_le16(cfg->pools[i].dpbp_id);
		cmd_params->pool[i].priority_mask =
			cfg->pools[i].priority_mask;
		cmd_params->buffer_size[i] =
			cpu_to_le16(cfg->pools[i].buffer_size);
		cmd_params->backup_pool_mask |=
			DPNI_BACKUP_POOL(cfg->pools[i].backup_pool, i);
	}

	/* send command to mc*/
	return mc_send_command(mc_io, &cmd);
}

/**
 * dpni_enable() - Enable the DPNI, allow sending and receiving frames.
 * @mc_io:	Pointer to MC portal's I/O object
 * @cmd_flags:	Command flags; one or more of 'MC_CMD_FLAG_'
 * @token:	Token of DPNI object
 *
 * Return:	'0' on Success; Error code otherwise.
 */
int dpni_enable(struct fsl_mc_io *mc_io, u32 cmd_flags, u16 token)
{
	struct mc_command cmd = { 0 };

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPNI_CMDID_ENABLE,
					  cmd_flags,
					  token);

	/* send command to mc*/
	return mc_send_command(mc_io, &cmd);
}

/**
 * dpni_disable() - Disable the DPNI, stop sending and receiving frames.
 * @mc_io:	Pointer to MC portal's I/O object
 * @cmd_flags:	Command flags; one or more of 'MC_CMD_FLAG_'
 * @token:	Token of DPNI object
 *
 * Return:	'0' on Success; Error code otherwise.
 */
int dpni_disable(struct fsl_mc_io *mc_io, u32 cmd_flags, u16 token)
{
	struct mc_command cmd = { 0 };

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPNI_CMDID_DISABLE,
					  cmd_flags,
					  token);

	/* send command to mc*/
	return mc_send_command(mc_io, &cmd);
}

/**
 * dpni_reset() - Reset the DPNI, returns the object to initial state.
 * @mc_io:	Pointer to MC portal's I/O object
 * @cmd_flags:	Command flags; one or more of 'MC_CMD_FLAG_'
 * @token:	Token of DPNI object
 *
 * Return:	'0' on Success; Error code otherwise.
 */
int dpni_reset(struct fsl_mc_io *mc_io, u32 cmd_flags, u16 token)
{
	struct mc_command cmd = { 0 };

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPNI_CMDID_RESET,
					  cmd_flags,
					  token);

	/* send command to mc*/
	return mc_send_command(mc_io, &cmd);
}

/**
 * dpni_get_attributes() - Retrieve DPNI attributes.
 * @mc_io:	Pointer to MC portal's I/O object
 * @cmd_flags:	Command flags; one or more of 'MC_CMD_FLAG_'
 * @token:	Token of DPNI object
 * @attr:	Object's attributes
 *
 * Return:	'0' on Success; Error code otherwise.
 */
int dpni_get_attributes(struct fsl_mc_io *mc_io, u32 cmd_flags, u16 token,
			struct dpni_attr *attr)
{
	struct mc_command cmd = { 0 };
	struct dpni_rsp_get_attr *rsp_params;

	int err;

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPNI_CMDID_GET_ATTR,
					  cmd_flags,
					  token);

	/* send command to mc*/
	err = mc_send_command(mc_io, &cmd);
	if (err)
		return err;

	/* retrieve response parameters */
	rsp_params = (struct dpni_rsp_get_attr *)cmd.params;
	attr->options = le32_to_cpu(rsp_params->options);
	attr->num_queues = rsp_params->num_queues;
	attr->num_rx_tcs = rsp_params->num_rx_tcs;
	attr->num_tx_tcs = rsp_params->num_tx_tcs;
	attr->mac_filter_entries = rsp_params->mac_filter_entries;
	attr->vlan_filter_entries = rsp_params->vlan_filter_entries;
	attr->num_channels = rsp_params->num_channels;
	attr->qos_entries = rsp_params->qos_entries;
	attr->fs_entries = le16_to_cpu(rsp_params->fs_entries);
	attr->num_opr = le16_to_cpu(rsp_params->num_opr);
	attr->qos_key_size = rsp_params->qos_key_size;
	attr->fs_key_size = rsp_params->fs_key_size;
	attr->wriop_version = le16_to_cpu(rsp_params->wriop_version);
	attr->num_cgs = rsp_params->num_cgs;

	return 0;
}

/**
 * dpni_set_buffer_layout() - Set buffer layout configuration.
 * @mc_io:	Pointer to MC portal's I/O object
 * @cmd_flags:	Command flags; one or more of 'MC_CMD_FLAG_'
 * @token:	Token of DPNI object
 * @qtype:	Type of queue this configuration applies to
 * @layout:	Buffer layout configuration
 *
 * Return:	'0' on Success; Error code otherwise.
 *
 * @warning	Allowed only when DPNI is disabled
 */
int dpni_set_buffer_layout(struct fsl_mc_io *mc_io, u32 cmd_flags, u16 token,
			   enum dpni_queue_type qtype,
			   const struct dpni_buffer_layout *layout)
{
	struct dpni_cmd_set_buffer_layout *cmd_params;
	struct mc_command cmd = { 0 };

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPNI_CMDID_SET_BUFFER_LAYOUT,
					  cmd_flags,
					  token);
	cmd_params = (struct dpni_cmd_set_buffer_layout *)cmd.params;
	cmd_params->qtype = qtype;
	cmd_params->options = cpu_to_le16((u16)layout->options);
	dpni_set_field(cmd_params->flags, PASS_TS, layout->pass_timestamp);
	dpni_set_field(cmd_params->flags, PASS_PR, layout->pass_parser_result);
	dpni_set_field(cmd_params->flags, PASS_FS, layout->pass_frame_status);
	dpni_set_field(cmd_params->flags, PASS_SWO, layout->pass_sw_opaque);
	cmd_params->private_data_size = cpu_to_le16(layout->private_data_size);
	cmd_params->data_align = cpu_to_le16(layout->data_align);
	cmd_params->head_room = cpu_to_le16(layout->data_head_room);
	cmd_params->tail_room = cpu_to_le16(layout->data_tail_room);

	/* send command to mc*/
	return mc_send_command(mc_io, &cmd);
}

/**
 * dpni_get_qdid() - Get the Queuing Destination ID (QDID) that should be used
 *			for enqueue operations
 * @mc_io:	Pointer to MC portal's I/O object
 * @cmd_flags:	Command flags; one or more of 'MC_CMD_FLAG_'
 * @token:	Token of DPNI object
 * @qtype:	Type of queue to receive QDID for
 * @qdid:	Returned virtual QDID value that should be used as an argument
 *			in all enqueue operations
 *
 * Return:	'0' on Success; Error code otherwise.
 *
 * If dpni object is created using multiple Tc channels this function will return
 * qdid value for the first channel
 */
int dpni_get_qdid(struct fsl_mc_io *mc_io, u32 cmd_flags, u16 token,
		  enum dpni_queue_type qtype, u16 *qdid)
{
	struct mc_command cmd = { 0 };
	struct dpni_cmd_get_qdid *cmd_params;
	struct dpni_rsp_get_qdid *rsp_params;
	int err;

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPNI_CMDID_GET_QDID,
					  cmd_flags,
					  token);
	cmd_params = (struct dpni_cmd_get_qdid *)cmd.params;
	cmd_params->qtype = qtype;

	/* send command to mc*/
	err = mc_send_command(mc_io, &cmd);
	if (err)
		return err;

	/* retrieve response parameters */
	rsp_params = (struct dpni_rsp_get_qdid *)cmd.params;
	*qdid = le16_to_cpu(rsp_params->qdid);

	return 0;
}

/**
 * dpni_get_tx_data_offset() - Get the Tx data offset (from start of buffer)
 * @mc_io:	Pointer to MC portal's I/O object
 * @cmd_flags:	Command flags; one or more of 'MC_CMD_FLAG_'
 * @token:	Token of DPNI object
 * @data_offset: Tx data offset (from start of buffer)
 *
 * Return:	'0' on Success; Error code otherwise.
 */
int dpni_get_tx_data_offset(struct fsl_mc_io *mc_io, u32 cmd_flags, u16 token,
			    u16 *data_offset)
{
	struct mc_command cmd = { 0 };
	struct dpni_rsp_get_tx_data_offset *rsp_params;
	int err;

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPNI_CMDID_GET_TX_DATA_OFFSET,
					  cmd_flags,
					  token);

	/* send command to mc*/
	err = mc_send_command(mc_io, &cmd);
	if (err)
		return err;

	/* retrieve response parameters */
	rsp_params = (struct dpni_rsp_get_tx_data_offset *)cmd.params;
	*data_offset = le16_to_cpu(rsp_params->data_offset);

	return 0;
}

/**
 * dpni_set_link_cfg() - set the link configuration.
 * @mc_io:	Pointer to MC portal's I/O object
 * @cmd_flags:	Command flags; one or more of 'MC_CMD_FLAG_'
 * @token:	Token of DPNI object
 * @cfg:	Link configuration
 *
 * Return:	'0' on Success; Error code otherwise.
 */
int dpni_set_link_cfg(struct fsl_mc_io *mc_io, u32 cmd_flags, u16 token,
		      const struct dpni_link_cfg *cfg)
{
	struct mc_command cmd = { 0 };
	struct dpni_cmd_set_link_cfg *cmd_params;

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPNI_CMDID_SET_LINK_CFG,
					  cmd_flags,
					  token);
	cmd_params = (struct dpni_cmd_set_link_cfg *)cmd.params;
	cmd_params->rate = cpu_to_le32(cfg->rate);
	cmd_params->options = cpu_to_le64(cfg->options);
	cmd_params->advertising = cpu_to_le64(cfg->advertising);

	/* send command to mc*/
	return mc_send_command(mc_io, &cmd);
}

/**
 * dpni_get_link_state() - Return the link state (either up or down)
 * @mc_io:	Pointer to MC portal's I/O object
 * @cmd_flags:	Command flags; one or more of 'MC_CMD_FLAG_'
 * @token:	Token of DPNI object
 * @state:	Returned link state;
 *
 * Return:	'0' on Success; Error code otherwise.
 */
int dpni_get_link_state(struct fsl_mc_io *mc_io, u32 cmd_flags, u16 token,
			struct dpni_link_state *state)
{
	struct mc_command cmd = { 0 };
	struct dpni_rsp_get_link_state *rsp_params;
	int err;

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPNI_CMDID_GET_LINK_STATE,
					  cmd_flags,
					  token);

	/* send command to mc*/
	err = mc_send_command(mc_io, &cmd);
	if (err)
		return err;

	/* retrieve response parameters */
	rsp_params = (struct dpni_rsp_get_link_state *)cmd.params;
	state->up = dpni_get_field(rsp_params->flags, LINK_STATE);
	state->state_valid = dpni_get_field(rsp_params->flags, STATE_VALID);
	state->rate = le32_to_cpu(rsp_params->rate);
	state->options = le64_to_cpu(rsp_params->options);
	state->supported = le64_to_cpu(rsp_params->supported);
	state->advertising = le64_to_cpu(rsp_params->advertising);

	return 0;
}

/**
 * dpni_add_mac_addr() - Add MAC address filter
 * @mc_io:	Pointer to MC portal's I/O object
 * @cmd_flags:	Command flags; one or more of 'MC_CMD_FLAG_'
 * @token:	Token of DPNI object
 * @mac_addr:	MAC address to add
 * @flags :0 - tc_id and flow_id will be ignored.
 *				 Pkt with this mac_id will be passed to the next
 *				 classification stages
 *          DPNI_MAC_SET_QUEUE_ACTION
 *				 Pkt with this mac will be forward directly to
 *				 queue defined by the tc_id and flow_id
 * @tc_id : Traffic class selection (0-7)
 * @flow_id : Selects the specific queue out of the set allocated for the
 *            same as tc_id. Value must be in range 0 to NUM_QUEUES - 1
 * Return:	'0' on Success; Error code otherwise.
 */
int dpni_add_mac_addr(struct fsl_mc_io *mc_io, u32 cmd_flags, u16 token,
		      const u8 mac_addr[6], u8 flags,
		      u8 tc_id, u8 flow_id)
{
	struct dpni_cmd_add_mac_addr *cmd_params;
	struct mc_command cmd = { 0 };
	int i;

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPNI_CMDID_ADD_MAC_ADDR,
					  cmd_flags,
					  token);
	cmd_params = (struct dpni_cmd_add_mac_addr *)cmd.params;
	cmd_params->flags = flags;
	cmd_params->tc_id = tc_id;
	cmd_params->fq_id = flow_id;

	for (i = 0; i < 6; i++)
		cmd_params->mac_addr[i] = mac_addr[5 - i];

	/* send command to mc*/
	return mc_send_command(mc_io, &cmd);
}

/**
 * dpni_get_api_version() - Get Data Path Network Interface API version
 * @mc_io:  Pointer to MC portal's I/O object
 * @cmd_flags:	Command flags; one or more of 'MC_CMD_FLAG_'
 * @major_ver:	Major version of data path network interface API
 * @minor_ver:	Minor version of data path network interface API
 *
 * Return:  '0' on Success; Error code otherwise.
 */
int dpni_get_api_version(struct fsl_mc_io *mc_io, u32 cmd_flags,
			 u16 *major_ver, u16 *minor_ver)
{
	struct mc_command cmd = { 0 };
	int err;

	cmd.header = mc_encode_cmd_header(DPNI_CMDID_GET_API_VERSION,
					cmd_flags,
					0);

	err = mc_send_command(mc_io, &cmd);
	if (err)
		return err;

	mc_cmd_read_api_version(&cmd, major_ver, minor_ver);

	return 0;
}

/**
 * dpni_set_queue() - Set queue parameters
 * @mc_io:	Pointer to MC portal's I/O object
 * @cmd_flags:	Command flags; one or more of 'MC_CMD_FLAG_'
 * @token:	Token of DPNI object
 * @qtype:	Type of queue - all queue types are supported, although
 *		the command is ignored for Tx
 * @tc:		Traffic class, in range 0 to NUM_TCS - 1
 * @index:	Selects the specific queue out of the set allocated for the
 *		same TC. Value must be in range 0 to NUM_QUEUES - 1
 * @options:	A combination of DPNI_QUEUE_OPT_ values that control what
 *		configuration options are set on the queue
 * @queue:	Queue structure
 *
 * Return:	'0' on Success; Error code otherwise.
 */
int dpni_set_queue(struct fsl_mc_io *mc_io, u32 cmd_flags, u16 token,
		   enum dpni_queue_type qtype, u16 param, u8 index,
		   u8 options, const struct dpni_queue *queue)
{
	struct mc_command cmd = { 0 };
	struct dpni_cmd_set_queue *cmd_params;

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPNI_CMDID_SET_QUEUE,
					  cmd_flags,
					  token);
	cmd_params = (struct dpni_cmd_set_queue *)cmd.params;
	cmd_params->qtype = qtype;
	cmd_params->tc = (u8)(param & 0xff);
	cmd_params->channel_id = (u8)((param >> 8) & 0xff);
	cmd_params->index = index;
	cmd_params->options = options;
	cmd_params->dest_id = cpu_to_le32(queue->destination.id);
	cmd_params->dest_prio = queue->destination.priority;
	dpni_set_field(cmd_params->flags, DEST_TYPE, queue->destination.type);
	dpni_set_field(cmd_params->flags, STASH_CTRL, queue->flc.stash_control);
	dpni_set_field(cmd_params->flags, HOLD_ACTIVE,
		       queue->destination.hold_active);
	cmd_params->flc = cpu_to_le64(queue->flc.value);
	cmd_params->user_context = cpu_to_le64(queue->user_context);
	cmd_params->cgid = queue->cgid;

	/* send command to mc */
	return mc_send_command(mc_io, &cmd);
}

/**
 * dpni_get_queue() - Get queue parameters
 * @mc_io:	Pointer to MC portal's I/O object
 * @cmd_flags:	Command flags; one or more of 'MC_CMD_FLAG_'
 * @token:	Token of DPNI object
 * @qtype:	Type of queue - all queue types are supported
 * @param:	Traffic class and channel ID.
 *			MSB - channel id; used only for DPNI_QUEUE_TX and
 *			DPNI_QUEUE_TX_CONFIRM, ignored for the rest
 *			LSB - traffic class
 *			Use macro DPNI_BUILD_PARAM() to build correct value.
 *			If dpni uses a single channel (uses only channel zero)
 *			the parameter can receive traffic class directly.
 * @index:	Selects the specific queue out of the set allocated for the
 *		same TC. Value must be in range 0 to NUM_QUEUES - 1
 * @queue:	Queue configuration structure
 * @qid:	Queue identification
 *
 * Return:	'0' on Success; Error code otherwise.
 */
int dpni_get_queue(struct fsl_mc_io *mc_io, u32 cmd_flags, u16 token,
		   enum dpni_queue_type qtype, u16 param, u8 index,
		   struct dpni_queue *queue, struct dpni_queue_id *qid)
{
	struct mc_command cmd = { 0 };
	struct dpni_cmd_get_queue *cmd_params;
	struct dpni_rsp_get_queue *rsp_params;
	int err;

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPNI_CMDID_GET_QUEUE,
					  cmd_flags,
					  token);
	cmd_params = (struct dpni_cmd_get_queue *)cmd.params;
	cmd_params->qtype = qtype;
	cmd_params->tc = (u8)(param & 0xff);
	cmd_params->index = index;
	cmd_params->channel_id = (u8)((param >> 8) & 0xff);

	/* send command to mc */
	err = mc_send_command(mc_io, &cmd);
	if (err)
		return err;

	/* retrieve response parameters */
	rsp_params = (struct dpni_rsp_get_queue *)cmd.params;
	queue->destination.id = le32_to_cpu(rsp_params->dest_id);
	queue->destination.priority = rsp_params->dest_prio;
	queue->destination.type = dpni_get_field(rsp_params->flags, DEST_TYPE);
	queue->flc.stash_control = dpni_get_field(rsp_params->flags, STASH_CTRL);
	queue->destination.hold_active = dpni_get_field(rsp_params->flags, HOLD_ACTIVE);
	queue->flc.value = le64_to_cpu(rsp_params->flc);
	queue->user_context = le64_to_cpu(rsp_params->user_context);
	qid->fqid = le32_to_cpu(rsp_params->fqid);
	qid->qdbin = le16_to_cpu(rsp_params->qdbin);
	if (dpni_get_field(rsp_params->flags, CGID_VALID))
		queue->cgid = rsp_params->cgid;
	else
		queue->cgid = -1;

	return 0;
}

/**
 * dpni_set_tx_confirmation_mode() - Tx confirmation mode
 * @mc_io:	Pointer to MC portal's I/O object
 * @cmd_flags:	Command flags; one or more of 'MC_CMD_FLAG_'
 * @token:	Token of DPNI object
 * @ceetm_ch_idx:	ceetm channel index
 * @mode:	Tx confirmation mode
 *
 * This function is useful only when 'DPNI_OPT_TX_CONF_DISABLED' is not
 * selected at DPNI creation.
 * Calling this function with 'mode' set to DPNI_CONF_DISABLE disables all
 * transmit confirmation (including the private confirmation queues), regardless
 * of previous settings; Note that in this case, Tx error frames are still
 * enqueued to the general transmit errors queue.
 * Calling this function with 'mode' set to DPNI_CONF_SINGLE switches all
 * Tx confirmations to a shared Tx conf queue. 'index' field in dpni_get_queue
 * command will be ignored.
 *
 * Return:	'0' on Success; Error code otherwise.
 */
int dpni_set_tx_confirmation_mode(struct fsl_mc_io *mc_io, u32 cmd_flags, u16 token,
				  u8 ceetm_ch_idx, enum dpni_confirmation_mode mode)
{
	struct dpni_tx_confirmation_mode *cmd_params;
	struct mc_command cmd = { 0 };

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPNI_CMDID_SET_TX_CONFIRMATION_MODE,
					  cmd_flags, token);
	cmd_params = (struct dpni_tx_confirmation_mode *)cmd.params;
	cmd_params->ceetm_ch_idx = ceetm_ch_idx;
	cmd_params->confirmation_mode = mode;

	/* send command to mc*/
	return mc_send_command(mc_io, &cmd);
}

/**
 * dpni_get_statistics() - Get DPNI statistics
 * @mc_io:	Pointer to MC portal's I/O object
 * @cmd_flags:	Command flags; one or more of 'MC_CMD_FLAG_'
 * @token:	Token of DPNI object
 * @page:	Selects the statistics page to retrieve, see
 *		DPNI_GET_STATISTICS output. Pages are numbered 0 to 6.
 * @param:  Custom parameter for some pages used to select
 *		 a certain statistic source, for example the TC.
 *		 - page_0: not used
 *		 - page_1: not used
 *		 - page_2: not used
 *		 - page_3: high_byte - channel_id, low_byte - traffic class
 *		 - page_4: high_byte - queue_index have meaning only if dpni is
 *		 created using option DPNI_OPT_CUSTOM_CG, low_byte - traffic class
 *		 - page_5: not used
 *		 - page_6: not used
 * @stat:	Structure containing the statistics
 *
 * Return:	'0' on Success; Error code otherwise.
 */
int dpni_get_statistics(struct fsl_mc_io *mc_io, u32 cmd_flags, u16 token,
			u8 page, u16 param, union dpni_statistics *stat)
{
	struct dpni_cmd_get_statistics *cmd_params;
	struct dpni_rsp_get_statistics *rsp_params;
	struct mc_command cmd = { 0 };
	int i, err;

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPNI_CMDID_GET_STATISTICS,
					  cmd_flags,
					  token);
	cmd_params = (struct dpni_cmd_get_statistics *)cmd.params;
	cmd_params->page_number = page;
	cmd_params->param = param;

	/* send command to mc */
	err = mc_send_command(mc_io, &cmd);
	if (err)
		return err;

	/* retrieve response parameters */
	rsp_params = (struct dpni_rsp_get_statistics *)cmd.params;
	for (i = 0; i < DPNI_STATISTICS_CNT; i++)
		stat->raw.counter[i] = le64_to_cpu(rsp_params->counter[i]);

	return 0;
}
