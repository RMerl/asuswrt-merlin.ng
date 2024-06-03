// SPDX-License-Identifier: GPL-2.0+
/*
 * Texas Instruments System Control Interface Protocol Driver
 * Based on drivers/firmware/ti_sci.c from Linux.
 *
 * Copyright (C) 2018 Texas Instruments Incorporated - http://www.ti.com/
 *	Lokesh Vutla <lokeshvutla@ti.com>
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <mailbox.h>
#include <dm/device.h>
#include <linux/compat.h>
#include <linux/err.h>
#include <linux/soc/ti/k3-sec-proxy.h>
#include <linux/soc/ti/ti_sci_protocol.h>

#include "ti_sci.h"

/* List of all TI SCI devices active in system */
static LIST_HEAD(ti_sci_list);

/**
 * struct ti_sci_xfer - Structure representing a message flow
 * @tx_message:	Transmit message
 * @rx_len:	Receive message length
 */
struct ti_sci_xfer {
	struct k3_sec_proxy_msg tx_message;
	u8 rx_len;
};

/**
 * struct ti_sci_rm_type_map - Structure representing TISCI Resource
 *				management representation of dev_ids.
 * @dev_id:	TISCI device ID
 * @type:	Corresponding id as identified by TISCI RM.
 *
 * Note: This is used only as a work around for using RM range apis
 *	for AM654 SoC. For future SoCs dev_id will be used as type
 *	for RM range APIs. In order to maintain ABI backward compatibility
 *	type is not being changed for AM654 SoC.
 */
struct ti_sci_rm_type_map {
	u32 dev_id;
	u16 type;
};

/**
 * struct ti_sci_desc - Description of SoC integration
 * @default_host_id:	Host identifier representing the compute entity
 * @max_rx_timeout_ms:	Timeout for communication with SoC (in Milliseconds)
 * @max_msgs: Maximum number of messages that can be pending
 *		  simultaneously in the system
 * @max_msg_size: Maximum size of data per message that can be handled.
 * @rm_type_map: RM resource type mapping structure.
 */
struct ti_sci_desc {
	u8 default_host_id;
	int max_rx_timeout_ms;
	int max_msgs;
	int max_msg_size;
	struct ti_sci_rm_type_map *rm_type_map;
};

/**
 * struct ti_sci_info - Structure representing a TI SCI instance
 * @dev:	Device pointer
 * @desc:	SoC description for this instance
 * @handle:	Instance of TI SCI handle to send to clients.
 * @chan_tx:	Transmit mailbox channel
 * @chan_rx:	Receive mailbox channel
 * @xfer:	xfer info
 * @list:	list head
 * @is_secure:	Determines if the communication is through secure threads.
 * @host_id:	Host identifier representing the compute entity
 * @seq:	Seq id used for verification for tx and rx message.
 */
struct ti_sci_info {
	struct udevice *dev;
	const struct ti_sci_desc *desc;
	struct ti_sci_handle handle;
	struct mbox_chan chan_tx;
	struct mbox_chan chan_rx;
	struct mbox_chan chan_notify;
	struct ti_sci_xfer xfer;
	struct list_head list;
	bool is_secure;
	u8 host_id;
	u8 seq;
};

#define handle_to_ti_sci_info(h) container_of(h, struct ti_sci_info, handle)

/**
 * ti_sci_setup_one_xfer() - Setup one message type
 * @info:	Pointer to SCI entity information
 * @msg_type:	Message type
 * @msg_flags:	Flag to set for the message
 * @buf:	Buffer to be send to mailbox channel
 * @tx_message_size: transmit message size
 * @rx_message_size: receive message size
 *
 * Helper function which is used by various command functions that are
 * exposed to clients of this driver for allocating a message traffic event.
 *
 * Return: Corresponding ti_sci_xfer pointer if all went fine,
 *	   else appropriate error pointer.
 */
static struct ti_sci_xfer *ti_sci_setup_one_xfer(struct ti_sci_info *info,
						 u16 msg_type, u32 msg_flags,
						 u32 *buf,
						 size_t tx_message_size,
						 size_t rx_message_size)
{
	struct ti_sci_xfer *xfer = &info->xfer;
	struct ti_sci_msg_hdr *hdr;

	/* Ensure we have sane transfer sizes */
	if (rx_message_size > info->desc->max_msg_size ||
	    tx_message_size > info->desc->max_msg_size ||
	    rx_message_size < sizeof(*hdr) || tx_message_size < sizeof(*hdr))
		return ERR_PTR(-ERANGE);

	info->seq = ~info->seq;
	xfer->tx_message.buf = buf;
	xfer->tx_message.len = tx_message_size;
	xfer->rx_len = (u8)rx_message_size;

	hdr = (struct ti_sci_msg_hdr *)buf;
	hdr->seq = info->seq;
	hdr->type = msg_type;
	hdr->host = info->host_id;
	hdr->flags = msg_flags;

	return xfer;
}

/**
 * ti_sci_get_response() - Receive response from mailbox channel
 * @info:	Pointer to SCI entity information
 * @xfer:	Transfer to initiate and wait for response
 * @chan:	Channel to receive the response
 *
 * Return: -ETIMEDOUT in case of no response, if transmit error,
 *	   return corresponding error, else if all goes well,
 *	   return 0.
 */
static inline int ti_sci_get_response(struct ti_sci_info *info,
				      struct ti_sci_xfer *xfer,
				      struct mbox_chan *chan)
{
	struct k3_sec_proxy_msg *msg = &xfer->tx_message;
	struct ti_sci_secure_msg_hdr *secure_hdr;
	struct ti_sci_msg_hdr *hdr;
	int ret;

	/* Receive the response */
	ret = mbox_recv(chan, msg, info->desc->max_rx_timeout_ms * 1000);
	if (ret) {
		dev_err(info->dev, "%s: Message receive failed. ret = %d\n",
			__func__, ret);
		return ret;
	}

	/* ToDo: Verify checksum */
	if (info->is_secure) {
		secure_hdr = (struct ti_sci_secure_msg_hdr *)msg->buf;
		msg->buf = (u32 *)((void *)msg->buf + sizeof(*secure_hdr));
	}

	/* msg is updated by mailbox driver */
	hdr = (struct ti_sci_msg_hdr *)msg->buf;

	/* Sanity check for message response */
	if (hdr->seq != info->seq) {
		dev_dbg(info->dev, "%s: Message for %d is not expected\n",
			__func__, hdr->seq);
		return ret;
	}

	if (msg->len > info->desc->max_msg_size) {
		dev_err(info->dev, "%s: Unable to handle %zu xfer (max %d)\n",
			__func__, msg->len, info->desc->max_msg_size);
		return -EINVAL;
	}

	if (msg->len < xfer->rx_len) {
		dev_err(info->dev, "%s: Recv xfer %zu < expected %d length\n",
			__func__, msg->len, xfer->rx_len);
	}

	return ret;
}

/**
 * ti_sci_do_xfer() - Do one transfer
 * @info:	Pointer to SCI entity information
 * @xfer:	Transfer to initiate and wait for response
 *
 * Return: 0 if all went fine, else return appropriate error.
 */
static inline int ti_sci_do_xfer(struct ti_sci_info *info,
				 struct ti_sci_xfer *xfer)
{
	struct k3_sec_proxy_msg *msg = &xfer->tx_message;
	u8 secure_buf[info->desc->max_msg_size];
	struct ti_sci_secure_msg_hdr secure_hdr;
	int ret;

	if (info->is_secure) {
		/* ToDo: get checksum of the entire message */
		secure_hdr.checksum = 0;
		secure_hdr.reserved = 0;
		memcpy(&secure_buf[sizeof(secure_hdr)], xfer->tx_message.buf,
		       xfer->tx_message.len);

		xfer->tx_message.buf = (u32 *)secure_buf;
		xfer->tx_message.len += sizeof(secure_hdr);
		xfer->rx_len += sizeof(secure_hdr);
	}

	/* Send the message */
	ret = mbox_send(&info->chan_tx, msg);
	if (ret) {
		dev_err(info->dev, "%s: Message sending failed. ret = %d\n",
			__func__, ret);
		return ret;
	}

	return ti_sci_get_response(info, xfer, &info->chan_rx);
}

/**
 * ti_sci_cmd_get_revision() - command to get the revision of the SCI entity
 * @handle:	pointer to TI SCI handle
 *
 * Updates the SCI information in the internal data structure.
 *
 * Return: 0 if all went fine, else return appropriate error.
 */
static int ti_sci_cmd_get_revision(struct ti_sci_handle *handle)
{
	struct ti_sci_msg_resp_version *rev_info;
	struct ti_sci_version_info *ver;
	struct ti_sci_msg_hdr hdr;
	struct ti_sci_info *info;
	struct ti_sci_xfer *xfer;
	int ret;

	if (IS_ERR(handle))
		return PTR_ERR(handle);
	if (!handle)
		return -EINVAL;

	info = handle_to_ti_sci_info(handle);

	xfer = ti_sci_setup_one_xfer(info, TI_SCI_MSG_VERSION,
				     TI_SCI_FLAG_REQ_ACK_ON_PROCESSED,
				     (u32 *)&hdr, sizeof(struct ti_sci_msg_hdr),
				     sizeof(*rev_info));
	if (IS_ERR(xfer)) {
		ret = PTR_ERR(xfer);
		dev_err(info->dev, "Message alloc failed(%d)\n", ret);
		return ret;
	}

	ret = ti_sci_do_xfer(info, xfer);
	if (ret) {
		dev_err(info->dev, "Mbox communication fail %d\n", ret);
		return ret;
	}

	rev_info = (struct ti_sci_msg_resp_version *)xfer->tx_message.buf;

	ver = &handle->version;
	ver->abi_major = rev_info->abi_major;
	ver->abi_minor = rev_info->abi_minor;
	ver->firmware_revision = rev_info->firmware_revision;
	strncpy(ver->firmware_description, rev_info->firmware_description,
		sizeof(ver->firmware_description));

	return 0;
}

/**
 * ti_sci_is_response_ack() - Generic ACK/NACK message checkup
 * @r:	pointer to response buffer
 *
 * Return: true if the response was an ACK, else returns false.
 */
static inline bool ti_sci_is_response_ack(void *r)
{
	struct ti_sci_msg_hdr *hdr = r;

	return hdr->flags & TI_SCI_FLAG_RESP_GENERIC_ACK ? true : false;
}

/**
 * cmd_set_board_config_using_msg() - Common command to send board configuration
 *                                    message
 * @handle:	pointer to TI SCI handle
 * @msg_type:	One of the TISCI message types to set board configuration
 * @addr:	Address where the board config structure is located
 * @size:	Size of the board config structure
 *
 * Return: 0 if all went well, else returns appropriate error value.
 */
static int cmd_set_board_config_using_msg(const struct ti_sci_handle *handle,
					  u16 msg_type, u64 addr, u32 size)
{
	struct ti_sci_msg_board_config req;
	struct ti_sci_msg_hdr *resp;
	struct ti_sci_info *info;
	struct ti_sci_xfer *xfer;
	int ret = 0;

	if (IS_ERR(handle))
		return PTR_ERR(handle);
	if (!handle)
		return -EINVAL;

	info = handle_to_ti_sci_info(handle);

	xfer = ti_sci_setup_one_xfer(info, msg_type,
				     TI_SCI_FLAG_REQ_ACK_ON_PROCESSED,
				     (u32 *)&req, sizeof(req), sizeof(*resp));
	if (IS_ERR(xfer)) {
		ret = PTR_ERR(xfer);
		dev_err(info->dev, "Message alloc failed(%d)\n", ret);
		return ret;
	}
	req.boardcfgp_high = (addr >> 32) & 0xffffffff;
	req.boardcfgp_low = addr & 0xffffffff;
	req.boardcfg_size = size;

	ret = ti_sci_do_xfer(info, xfer);
	if (ret) {
		dev_err(info->dev, "Mbox send fail %d\n", ret);
		return ret;
	}

	resp = (struct ti_sci_msg_hdr *)xfer->tx_message.buf;

	if (!ti_sci_is_response_ack(resp))
		return -ENODEV;

	return ret;
}

/**
 * ti_sci_cmd_set_board_config() - Command to send board configuration message
 * @handle:	pointer to TI SCI handle
 * @addr:	Address where the board config structure is located
 * @size:	Size of the board config structure
 *
 * Return: 0 if all went well, else returns appropriate error value.
 */
static int ti_sci_cmd_set_board_config(const struct ti_sci_handle *handle,
				       u64 addr, u32 size)
{
	return cmd_set_board_config_using_msg(handle,
					      TI_SCI_MSG_BOARD_CONFIG,
					      addr, size);
}

/**
 * ti_sci_cmd_set_board_config_rm() - Command to send board resource
 *				      management configuration
 * @handle:	pointer to TI SCI handle
 * @addr:	Address where the board RM config structure is located
 * @size:	Size of the RM config structure
 *
 * Return: 0 if all went well, else returns appropriate error value.
 */
static
int ti_sci_cmd_set_board_config_rm(const struct ti_sci_handle *handle,
				   u64 addr, u32 size)
{
	return cmd_set_board_config_using_msg(handle,
					      TI_SCI_MSG_BOARD_CONFIG_RM,
					      addr, size);
}

/**
 * ti_sci_cmd_set_board_config_security() - Command to send board security
 *					    configuration message
 * @handle:	pointer to TI SCI handle
 * @addr:	Address where the board security config structure is located
 * @size:	Size of the security config structure
 *
 * Return: 0 if all went well, else returns appropriate error value.
 */
static
int ti_sci_cmd_set_board_config_security(const struct ti_sci_handle *handle,
					 u64 addr, u32 size)
{
	return cmd_set_board_config_using_msg(handle,
					      TI_SCI_MSG_BOARD_CONFIG_SECURITY,
					      addr, size);
}

/**
 * ti_sci_cmd_set_board_config_pm() - Command to send board power and clock
 *				      configuration message
 * @handle:	pointer to TI SCI handle
 * @addr:	Address where the board PM config structure is located
 * @size:	Size of the PM config structure
 *
 * Return: 0 if all went well, else returns appropriate error value.
 */
static int ti_sci_cmd_set_board_config_pm(const struct ti_sci_handle *handle,
					  u64 addr, u32 size)
{
	return cmd_set_board_config_using_msg(handle,
					      TI_SCI_MSG_BOARD_CONFIG_PM,
					      addr, size);
}

/**
 * ti_sci_set_device_state() - Set device state helper
 * @handle:	pointer to TI SCI handle
 * @id:		Device identifier
 * @flags:	flags to setup for the device
 * @state:	State to move the device to
 *
 * Return: 0 if all went well, else returns appropriate error value.
 */
static int ti_sci_set_device_state(const struct ti_sci_handle *handle,
				   u32 id, u32 flags, u8 state)
{
	struct ti_sci_msg_req_set_device_state req;
	struct ti_sci_msg_hdr *resp;
	struct ti_sci_info *info;
	struct ti_sci_xfer *xfer;
	int ret = 0;

	if (IS_ERR(handle))
		return PTR_ERR(handle);
	if (!handle)
		return -EINVAL;

	info = handle_to_ti_sci_info(handle);

	xfer = ti_sci_setup_one_xfer(info, TI_SCI_MSG_SET_DEVICE_STATE,
				     flags | TI_SCI_FLAG_REQ_ACK_ON_PROCESSED,
				     (u32 *)&req, sizeof(req), sizeof(*resp));
	if (IS_ERR(xfer)) {
		ret = PTR_ERR(xfer);
		dev_err(info->dev, "Message alloc failed(%d)\n", ret);
		return ret;
	}
	req.id = id;
	req.state = state;

	ret = ti_sci_do_xfer(info, xfer);
	if (ret) {
		dev_err(info->dev, "Mbox send fail %d\n", ret);
		return ret;
	}

	resp = (struct ti_sci_msg_hdr *)xfer->tx_message.buf;

	if (!ti_sci_is_response_ack(resp))
		return -ENODEV;

	return ret;
}

/**
 * ti_sci_get_device_state() - Get device state helper
 * @handle:	Handle to the device
 * @id:		Device Identifier
 * @clcnt:	Pointer to Context Loss Count
 * @resets:	pointer to resets
 * @p_state:	pointer to p_state
 * @c_state:	pointer to c_state
 *
 * Return: 0 if all went fine, else return appropriate error.
 */
static int ti_sci_get_device_state(const struct ti_sci_handle *handle,
				   u32 id,  u32 *clcnt,  u32 *resets,
				   u8 *p_state,  u8 *c_state)
{
	struct ti_sci_msg_resp_get_device_state *resp;
	struct ti_sci_msg_req_get_device_state req;
	struct ti_sci_info *info;
	struct ti_sci_xfer *xfer;
	int ret = 0;

	if (IS_ERR(handle))
		return PTR_ERR(handle);
	if (!handle)
		return -EINVAL;

	if (!clcnt && !resets && !p_state && !c_state)
		return -EINVAL;

	info = handle_to_ti_sci_info(handle);

	xfer = ti_sci_setup_one_xfer(info, TI_SCI_MSG_GET_DEVICE_STATE,
				     TI_SCI_FLAG_REQ_ACK_ON_PROCESSED,
				     (u32 *)&req, sizeof(req), sizeof(*resp));
	if (IS_ERR(xfer)) {
		ret = PTR_ERR(xfer);
		dev_err(info->dev, "Message alloc failed(%d)\n", ret);
		return ret;
	}
	req.id = id;

	ret = ti_sci_do_xfer(info, xfer);
	if (ret) {
		dev_err(dev, "Mbox send fail %d\n", ret);
		return ret;
	}

	resp = (struct ti_sci_msg_resp_get_device_state *)xfer->tx_message.buf;
	if (!ti_sci_is_response_ack(resp))
		return -ENODEV;

	if (clcnt)
		*clcnt = resp->context_loss_count;
	if (resets)
		*resets = resp->resets;
	if (p_state)
		*p_state = resp->programmed_state;
	if (c_state)
		*c_state = resp->current_state;

	return ret;
}

/**
 * ti_sci_cmd_get_device() - command to request for device managed by TISCI
 * @handle:	Pointer to TISCI handle as retrieved by *ti_sci_get_handle
 * @id:		Device Identifier
 *
 * Request for the device - NOTE: the client MUST maintain integrity of
 * usage count by balancing get_device with put_device. No refcounting is
 * managed by driver for that purpose.
 *
 * NOTE: The request is for exclusive access for the processor.
 *
 * Return: 0 if all went fine, else return appropriate error.
 */
static int ti_sci_cmd_get_device(const struct ti_sci_handle *handle, u32 id)
{
	return ti_sci_set_device_state(handle, id,
				       MSG_FLAG_DEVICE_EXCLUSIVE,
				       MSG_DEVICE_SW_STATE_ON);
}

/**
 * ti_sci_cmd_idle_device() - Command to idle a device managed by TISCI
 * @handle:	Pointer to TISCI handle as retrieved by *ti_sci_get_handle
 * @id:		Device Identifier
 *
 * Request for the device - NOTE: the client MUST maintain integrity of
 * usage count by balancing get_device with put_device. No refcounting is
 * managed by driver for that purpose.
 *
 * Return: 0 if all went fine, else return appropriate error.
 */
static int ti_sci_cmd_idle_device(const struct ti_sci_handle *handle, u32 id)
{
	return ti_sci_set_device_state(handle, id,
				       MSG_FLAG_DEVICE_EXCLUSIVE,
				       MSG_DEVICE_SW_STATE_RETENTION);
}

/**
 * ti_sci_cmd_put_device() - command to release a device managed by TISCI
 * @handle:	Pointer to TISCI handle as retrieved by *ti_sci_get_handle
 * @id:		Device Identifier
 *
 * Request for the device - NOTE: the client MUST maintain integrity of
 * usage count by balancing get_device with put_device. No refcounting is
 * managed by driver for that purpose.
 *
 * Return: 0 if all went fine, else return appropriate error.
 */
static int ti_sci_cmd_put_device(const struct ti_sci_handle *handle, u32 id)
{
	return ti_sci_set_device_state(handle, id,
				       0, MSG_DEVICE_SW_STATE_AUTO_OFF);
}

/**
 * ti_sci_cmd_dev_is_valid() - Is the device valid
 * @handle:	Pointer to TISCI handle as retrieved by *ti_sci_get_handle
 * @id:		Device Identifier
 *
 * Return: 0 if all went fine and the device ID is valid, else return
 * appropriate error.
 */
static int ti_sci_cmd_dev_is_valid(const struct ti_sci_handle *handle, u32 id)
{
	u8 unused;

	/* check the device state which will also tell us if the ID is valid */
	return ti_sci_get_device_state(handle, id, NULL, NULL, NULL, &unused);
}

/**
 * ti_sci_cmd_dev_get_clcnt() - Get context loss counter
 * @handle:	Pointer to TISCI handle
 * @id:		Device Identifier
 * @count:	Pointer to Context Loss counter to populate
 *
 * Return: 0 if all went fine, else return appropriate error.
 */
static int ti_sci_cmd_dev_get_clcnt(const struct ti_sci_handle *handle, u32 id,
				    u32 *count)
{
	return ti_sci_get_device_state(handle, id, count, NULL, NULL, NULL);
}

/**
 * ti_sci_cmd_dev_is_idle() - Check if the device is requested to be idle
 * @handle:	Pointer to TISCI handle
 * @id:		Device Identifier
 * @r_state:	true if requested to be idle
 *
 * Return: 0 if all went fine, else return appropriate error.
 */
static int ti_sci_cmd_dev_is_idle(const struct ti_sci_handle *handle, u32 id,
				  bool *r_state)
{
	int ret;
	u8 state;

	if (!r_state)
		return -EINVAL;

	ret = ti_sci_get_device_state(handle, id, NULL, NULL, &state, NULL);
	if (ret)
		return ret;

	*r_state = (state == MSG_DEVICE_SW_STATE_RETENTION);

	return 0;
}

/**
 * ti_sci_cmd_dev_is_stop() - Check if the device is requested to be stopped
 * @handle:	Pointer to TISCI handle
 * @id:		Device Identifier
 * @r_state:	true if requested to be stopped
 * @curr_state:	true if currently stopped.
 *
 * Return: 0 if all went fine, else return appropriate error.
 */
static int ti_sci_cmd_dev_is_stop(const struct ti_sci_handle *handle, u32 id,
				  bool *r_state,  bool *curr_state)
{
	int ret;
	u8 p_state, c_state;

	if (!r_state && !curr_state)
		return -EINVAL;

	ret =
	    ti_sci_get_device_state(handle, id, NULL, NULL, &p_state, &c_state);
	if (ret)
		return ret;

	if (r_state)
		*r_state = (p_state == MSG_DEVICE_SW_STATE_AUTO_OFF);
	if (curr_state)
		*curr_state = (c_state == MSG_DEVICE_HW_STATE_OFF);

	return 0;
}

/**
 * ti_sci_cmd_dev_is_on() - Check if the device is requested to be ON
 * @handle:	Pointer to TISCI handle
 * @id:		Device Identifier
 * @r_state:	true if requested to be ON
 * @curr_state:	true if currently ON and active
 *
 * Return: 0 if all went fine, else return appropriate error.
 */
static int ti_sci_cmd_dev_is_on(const struct ti_sci_handle *handle, u32 id,
				bool *r_state,  bool *curr_state)
{
	int ret;
	u8 p_state, c_state;

	if (!r_state && !curr_state)
		return -EINVAL;

	ret =
	    ti_sci_get_device_state(handle, id, NULL, NULL, &p_state, &c_state);
	if (ret)
		return ret;

	if (r_state)
		*r_state = (p_state == MSG_DEVICE_SW_STATE_ON);
	if (curr_state)
		*curr_state = (c_state == MSG_DEVICE_HW_STATE_ON);

	return 0;
}

/**
 * ti_sci_cmd_dev_is_trans() - Check if the device is currently transitioning
 * @handle:	Pointer to TISCI handle
 * @id:		Device Identifier
 * @curr_state:	true if currently transitioning.
 *
 * Return: 0 if all went fine, else return appropriate error.
 */
static int ti_sci_cmd_dev_is_trans(const struct ti_sci_handle *handle, u32 id,
				   bool *curr_state)
{
	int ret;
	u8 state;

	if (!curr_state)
		return -EINVAL;

	ret = ti_sci_get_device_state(handle, id, NULL, NULL, NULL, &state);
	if (ret)
		return ret;

	*curr_state = (state == MSG_DEVICE_HW_STATE_TRANS);

	return 0;
}

/**
 * ti_sci_cmd_set_device_resets() - command to set resets for device managed
 *				    by TISCI
 * @handle:	Pointer to TISCI handle as retrieved by *ti_sci_get_handle
 * @id:		Device Identifier
 * @reset_state: Device specific reset bit field
 *
 * Return: 0 if all went fine, else return appropriate error.
 */
static int ti_sci_cmd_set_device_resets(const struct ti_sci_handle *handle,
					u32 id, u32 reset_state)
{
	struct ti_sci_msg_req_set_device_resets req;
	struct ti_sci_msg_hdr *resp;
	struct ti_sci_info *info;
	struct ti_sci_xfer *xfer;
	int ret = 0;

	if (IS_ERR(handle))
		return PTR_ERR(handle);
	if (!handle)
		return -EINVAL;

	info = handle_to_ti_sci_info(handle);

	xfer = ti_sci_setup_one_xfer(info, TI_SCI_MSG_SET_DEVICE_RESETS,
				     TI_SCI_FLAG_REQ_ACK_ON_PROCESSED,
				     (u32 *)&req, sizeof(req), sizeof(*resp));
	if (IS_ERR(xfer)) {
		ret = PTR_ERR(xfer);
		dev_err(info->dev, "Message alloc failed(%d)\n", ret);
		return ret;
	}
	req.id = id;
	req.resets = reset_state;

	ret = ti_sci_do_xfer(info, xfer);
	if (ret) {
		dev_err(info->dev, "Mbox send fail %d\n", ret);
		return ret;
	}

	resp = (struct ti_sci_msg_hdr *)xfer->tx_message.buf;

	if (!ti_sci_is_response_ack(resp))
		return -ENODEV;

	return ret;
}

/**
 * ti_sci_cmd_get_device_resets() - Get reset state for device managed
 *				    by TISCI
 * @handle:		Pointer to TISCI handle
 * @id:			Device Identifier
 * @reset_state:	Pointer to reset state to populate
 *
 * Return: 0 if all went fine, else return appropriate error.
 */
static int ti_sci_cmd_get_device_resets(const struct ti_sci_handle *handle,
					u32 id, u32 *reset_state)
{
	return ti_sci_get_device_state(handle, id, NULL, reset_state, NULL,
				       NULL);
}

/**
 * ti_sci_set_clock_state() - Set clock state helper
 * @handle:	pointer to TI SCI handle
 * @dev_id:	Device identifier this request is for
 * @clk_id:	Clock identifier for the device for this request.
 *		Each device has it's own set of clock inputs. This indexes
 *		which clock input to modify.
 * @flags:	Header flags as needed
 * @state:	State to request for the clock.
 *
 * Return: 0 if all went well, else returns appropriate error value.
 */
static int ti_sci_set_clock_state(const struct ti_sci_handle *handle,
				  u32 dev_id, u8 clk_id,
				  u32 flags, u8 state)
{
	struct ti_sci_msg_req_set_clock_state req;
	struct ti_sci_msg_hdr *resp;
	struct ti_sci_info *info;
	struct ti_sci_xfer *xfer;
	int ret = 0;

	if (IS_ERR(handle))
		return PTR_ERR(handle);
	if (!handle)
		return -EINVAL;

	info = handle_to_ti_sci_info(handle);

	xfer = ti_sci_setup_one_xfer(info, TI_SCI_MSG_SET_CLOCK_STATE,
				     flags | TI_SCI_FLAG_REQ_ACK_ON_PROCESSED,
				     (u32 *)&req, sizeof(req), sizeof(*resp));
	if (IS_ERR(xfer)) {
		ret = PTR_ERR(xfer);
		dev_err(info->dev, "Message alloc failed(%d)\n", ret);
		return ret;
	}
	req.dev_id = dev_id;
	req.clk_id = clk_id;
	req.request_state = state;

	ret = ti_sci_do_xfer(info, xfer);
	if (ret) {
		dev_err(info->dev, "Mbox send fail %d\n", ret);
		return ret;
	}

	resp = (struct ti_sci_msg_hdr *)xfer->tx_message.buf;

	if (!ti_sci_is_response_ack(resp))
		return -ENODEV;

	return ret;
}

/**
 * ti_sci_cmd_get_clock_state() - Get clock state helper
 * @handle:	pointer to TI SCI handle
 * @dev_id:	Device identifier this request is for
 * @clk_id:	Clock identifier for the device for this request.
 *		Each device has it's own set of clock inputs. This indexes
 *		which clock input to modify.
 * @programmed_state:	State requested for clock to move to
 * @current_state:	State that the clock is currently in
 *
 * Return: 0 if all went well, else returns appropriate error value.
 */
static int ti_sci_cmd_get_clock_state(const struct ti_sci_handle *handle,
				      u32 dev_id, u8 clk_id,
				      u8 *programmed_state, u8 *current_state)
{
	struct ti_sci_msg_resp_get_clock_state *resp;
	struct ti_sci_msg_req_get_clock_state req;
	struct ti_sci_info *info;
	struct ti_sci_xfer *xfer;
	int ret = 0;

	if (IS_ERR(handle))
		return PTR_ERR(handle);
	if (!handle)
		return -EINVAL;

	if (!programmed_state && !current_state)
		return -EINVAL;

	info = handle_to_ti_sci_info(handle);

	xfer = ti_sci_setup_one_xfer(info, TI_SCI_MSG_GET_CLOCK_STATE,
				     TI_SCI_FLAG_REQ_ACK_ON_PROCESSED,
				     (u32 *)&req, sizeof(req), sizeof(*resp));
	if (IS_ERR(xfer)) {
		ret = PTR_ERR(xfer);
		dev_err(info->dev, "Message alloc failed(%d)\n", ret);
		return ret;
	}
	req.dev_id = dev_id;
	req.clk_id = clk_id;

	ret = ti_sci_do_xfer(info, xfer);
	if (ret) {
		dev_err(info->dev, "Mbox send fail %d\n", ret);
		return ret;
	}

	resp = (struct ti_sci_msg_resp_get_clock_state *)xfer->tx_message.buf;

	if (!ti_sci_is_response_ack(resp))
		return -ENODEV;

	if (programmed_state)
		*programmed_state = resp->programmed_state;
	if (current_state)
		*current_state = resp->current_state;

	return ret;
}

/**
 * ti_sci_cmd_get_clock() - Get control of a clock from TI SCI
 * @handle:	pointer to TI SCI handle
 * @dev_id:	Device identifier this request is for
 * @clk_id:	Clock identifier for the device for this request.
 *		Each device has it's own set of clock inputs. This indexes
 *		which clock input to modify.
 * @needs_ssc: 'true' if Spread Spectrum clock is desired, else 'false'
 * @can_change_freq: 'true' if frequency change is desired, else 'false'
 * @enable_input_term: 'true' if input termination is desired, else 'false'
 *
 * Return: 0 if all went well, else returns appropriate error value.
 */
static int ti_sci_cmd_get_clock(const struct ti_sci_handle *handle, u32 dev_id,
				u8 clk_id, bool needs_ssc, bool can_change_freq,
				bool enable_input_term)
{
	u32 flags = 0;

	flags |= needs_ssc ? MSG_FLAG_CLOCK_ALLOW_SSC : 0;
	flags |= can_change_freq ? MSG_FLAG_CLOCK_ALLOW_FREQ_CHANGE : 0;
	flags |= enable_input_term ? MSG_FLAG_CLOCK_INPUT_TERM : 0;

	return ti_sci_set_clock_state(handle, dev_id, clk_id, flags,
				      MSG_CLOCK_SW_STATE_REQ);
}

/**
 * ti_sci_cmd_idle_clock() - Idle a clock which is in our control
 * @handle:	pointer to TI SCI handle
 * @dev_id:	Device identifier this request is for
 * @clk_id:	Clock identifier for the device for this request.
 *		Each device has it's own set of clock inputs. This indexes
 *		which clock input to modify.
 *
 * NOTE: This clock must have been requested by get_clock previously.
 *
 * Return: 0 if all went well, else returns appropriate error value.
 */
static int ti_sci_cmd_idle_clock(const struct ti_sci_handle *handle,
				 u32 dev_id, u8 clk_id)
{
	return ti_sci_set_clock_state(handle, dev_id, clk_id, 0,
				      MSG_CLOCK_SW_STATE_UNREQ);
}

/**
 * ti_sci_cmd_put_clock() - Release a clock from our control back to TISCI
 * @handle:	pointer to TI SCI handle
 * @dev_id:	Device identifier this request is for
 * @clk_id:	Clock identifier for the device for this request.
 *		Each device has it's own set of clock inputs. This indexes
 *		which clock input to modify.
 *
 * NOTE: This clock must have been requested by get_clock previously.
 *
 * Return: 0 if all went well, else returns appropriate error value.
 */
static int ti_sci_cmd_put_clock(const struct ti_sci_handle *handle,
				u32 dev_id, u8 clk_id)
{
	return ti_sci_set_clock_state(handle, dev_id, clk_id, 0,
				      MSG_CLOCK_SW_STATE_AUTO);
}

/**
 * ti_sci_cmd_clk_is_auto() - Is the clock being auto managed
 * @handle:	pointer to TI SCI handle
 * @dev_id:	Device identifier this request is for
 * @clk_id:	Clock identifier for the device for this request.
 *		Each device has it's own set of clock inputs. This indexes
 *		which clock input to modify.
 * @req_state: state indicating if the clock is auto managed
 *
 * Return: 0 if all went well, else returns appropriate error value.
 */
static int ti_sci_cmd_clk_is_auto(const struct ti_sci_handle *handle,
				  u32 dev_id, u8 clk_id, bool *req_state)
{
	u8 state = 0;
	int ret;

	if (!req_state)
		return -EINVAL;

	ret = ti_sci_cmd_get_clock_state(handle, dev_id, clk_id, &state, NULL);
	if (ret)
		return ret;

	*req_state = (state == MSG_CLOCK_SW_STATE_AUTO);
	return 0;
}

/**
 * ti_sci_cmd_clk_is_on() - Is the clock ON
 * @handle:	pointer to TI SCI handle
 * @dev_id:	Device identifier this request is for
 * @clk_id:	Clock identifier for the device for this request.
 *		Each device has it's own set of clock inputs. This indexes
 *		which clock input to modify.
 * @req_state: state indicating if the clock is managed by us and enabled
 * @curr_state: state indicating if the clock is ready for operation
 *
 * Return: 0 if all went well, else returns appropriate error value.
 */
static int ti_sci_cmd_clk_is_on(const struct ti_sci_handle *handle, u32 dev_id,
				u8 clk_id, bool *req_state, bool *curr_state)
{
	u8 c_state = 0, r_state = 0;
	int ret;

	if (!req_state && !curr_state)
		return -EINVAL;

	ret = ti_sci_cmd_get_clock_state(handle, dev_id, clk_id,
					 &r_state, &c_state);
	if (ret)
		return ret;

	if (req_state)
		*req_state = (r_state == MSG_CLOCK_SW_STATE_REQ);
	if (curr_state)
		*curr_state = (c_state == MSG_CLOCK_HW_STATE_READY);
	return 0;
}

/**
 * ti_sci_cmd_clk_is_off() - Is the clock OFF
 * @handle:	pointer to TI SCI handle
 * @dev_id:	Device identifier this request is for
 * @clk_id:	Clock identifier for the device for this request.
 *		Each device has it's own set of clock inputs. This indexes
 *		which clock input to modify.
 * @req_state: state indicating if the clock is managed by us and disabled
 * @curr_state: state indicating if the clock is NOT ready for operation
 *
 * Return: 0 if all went well, else returns appropriate error value.
 */
static int ti_sci_cmd_clk_is_off(const struct ti_sci_handle *handle, u32 dev_id,
				 u8 clk_id, bool *req_state, bool *curr_state)
{
	u8 c_state = 0, r_state = 0;
	int ret;

	if (!req_state && !curr_state)
		return -EINVAL;

	ret = ti_sci_cmd_get_clock_state(handle, dev_id, clk_id,
					 &r_state, &c_state);
	if (ret)
		return ret;

	if (req_state)
		*req_state = (r_state == MSG_CLOCK_SW_STATE_UNREQ);
	if (curr_state)
		*curr_state = (c_state == MSG_CLOCK_HW_STATE_NOT_READY);
	return 0;
}

/**
 * ti_sci_cmd_clk_set_parent() - Set the clock source of a specific device clock
 * @handle:	pointer to TI SCI handle
 * @dev_id:	Device identifier this request is for
 * @clk_id:	Clock identifier for the device for this request.
 *		Each device has it's own set of clock inputs. This indexes
 *		which clock input to modify.
 * @parent_id:	Parent clock identifier to set
 *
 * Return: 0 if all went well, else returns appropriate error value.
 */
static int ti_sci_cmd_clk_set_parent(const struct ti_sci_handle *handle,
				     u32 dev_id, u8 clk_id, u8 parent_id)
{
	struct ti_sci_msg_req_set_clock_parent req;
	struct ti_sci_msg_hdr *resp;
	struct ti_sci_info *info;
	struct ti_sci_xfer *xfer;
	int ret = 0;

	if (IS_ERR(handle))
		return PTR_ERR(handle);
	if (!handle)
		return -EINVAL;

	info = handle_to_ti_sci_info(handle);

	xfer = ti_sci_setup_one_xfer(info, TI_SCI_MSG_SET_CLOCK_PARENT,
				     TI_SCI_FLAG_REQ_ACK_ON_PROCESSED,
				     (u32 *)&req, sizeof(req), sizeof(*resp));
	if (IS_ERR(xfer)) {
		ret = PTR_ERR(xfer);
		dev_err(info->dev, "Message alloc failed(%d)\n", ret);
		return ret;
	}
	req.dev_id = dev_id;
	req.clk_id = clk_id;
	req.parent_id = parent_id;

	ret = ti_sci_do_xfer(info, xfer);
	if (ret) {
		dev_err(info->dev, "Mbox send fail %d\n", ret);
		return ret;
	}

	resp = (struct ti_sci_msg_hdr *)xfer->tx_message.buf;

	if (!ti_sci_is_response_ack(resp))
		return -ENODEV;

	return ret;
}

/**
 * ti_sci_cmd_clk_get_parent() - Get current parent clock source
 * @handle:	pointer to TI SCI handle
 * @dev_id:	Device identifier this request is for
 * @clk_id:	Clock identifier for the device for this request.
 *		Each device has it's own set of clock inputs. This indexes
 *		which clock input to modify.
 * @parent_id:	Current clock parent
 *
 * Return: 0 if all went well, else returns appropriate error value.
 */
static int ti_sci_cmd_clk_get_parent(const struct ti_sci_handle *handle,
				     u32 dev_id, u8 clk_id, u8 *parent_id)
{
	struct ti_sci_msg_resp_get_clock_parent *resp;
	struct ti_sci_msg_req_get_clock_parent req;
	struct ti_sci_info *info;
	struct ti_sci_xfer *xfer;
	int ret = 0;

	if (IS_ERR(handle))
		return PTR_ERR(handle);
	if (!handle || !parent_id)
		return -EINVAL;

	info = handle_to_ti_sci_info(handle);

	xfer = ti_sci_setup_one_xfer(info, TI_SCI_MSG_GET_CLOCK_PARENT,
				     TI_SCI_FLAG_REQ_ACK_ON_PROCESSED,
				     (u32 *)&req, sizeof(req), sizeof(*resp));
	if (IS_ERR(xfer)) {
		ret = PTR_ERR(xfer);
		dev_err(info->dev, "Message alloc failed(%d)\n", ret);
		return ret;
	}
	req.dev_id = dev_id;
	req.clk_id = clk_id;

	ret = ti_sci_do_xfer(info, xfer);
	if (ret) {
		dev_err(info->dev, "Mbox send fail %d\n", ret);
		return ret;
	}

	resp = (struct ti_sci_msg_resp_get_clock_parent *)xfer->tx_message.buf;

	if (!ti_sci_is_response_ack(resp))
		ret = -ENODEV;
	else
		*parent_id = resp->parent_id;

	return ret;
}

/**
 * ti_sci_cmd_clk_get_num_parents() - Get num parents of the current clk source
 * @handle:	pointer to TI SCI handle
 * @dev_id:	Device identifier this request is for
 * @clk_id:	Clock identifier for the device for this request.
 *		Each device has it's own set of clock inputs. This indexes
 *		which clock input to modify.
 * @num_parents: Returns he number of parents to the current clock.
 *
 * Return: 0 if all went well, else returns appropriate error value.
 */
static int ti_sci_cmd_clk_get_num_parents(const struct ti_sci_handle *handle,
					  u32 dev_id, u8 clk_id,
					  u8 *num_parents)
{
	struct ti_sci_msg_resp_get_clock_num_parents *resp;
	struct ti_sci_msg_req_get_clock_num_parents req;
	struct ti_sci_info *info;
	struct ti_sci_xfer *xfer;
	int ret = 0;

	if (IS_ERR(handle))
		return PTR_ERR(handle);
	if (!handle || !num_parents)
		return -EINVAL;

	info = handle_to_ti_sci_info(handle);

	xfer = ti_sci_setup_one_xfer(info, TI_SCI_MSG_GET_NUM_CLOCK_PARENTS,
				     TI_SCI_FLAG_REQ_ACK_ON_PROCESSED,
				     (u32 *)&req, sizeof(req), sizeof(*resp));
	if (IS_ERR(xfer)) {
		ret = PTR_ERR(xfer);
		dev_err(info->dev, "Message alloc failed(%d)\n", ret);
		return ret;
	}
	req.dev_id = dev_id;
	req.clk_id = clk_id;

	ret = ti_sci_do_xfer(info, xfer);
	if (ret) {
		dev_err(info->dev, "Mbox send fail %d\n", ret);
		return ret;
	}

	resp = (struct ti_sci_msg_resp_get_clock_num_parents *)
							xfer->tx_message.buf;

	if (!ti_sci_is_response_ack(resp))
		ret = -ENODEV;
	else
		*num_parents = resp->num_parents;

	return ret;
}

/**
 * ti_sci_cmd_clk_get_match_freq() - Find a good match for frequency
 * @handle:	pointer to TI SCI handle
 * @dev_id:	Device identifier this request is for
 * @clk_id:	Clock identifier for the device for this request.
 *		Each device has it's own set of clock inputs. This indexes
 *		which clock input to modify.
 * @min_freq:	The minimum allowable frequency in Hz. This is the minimum
 *		allowable programmed frequency and does not account for clock
 *		tolerances and jitter.
 * @target_freq: The target clock frequency in Hz. A frequency will be
 *		processed as close to this target frequency as possible.
 * @max_freq:	The maximum allowable frequency in Hz. This is the maximum
 *		allowable programmed frequency and does not account for clock
 *		tolerances and jitter.
 * @match_freq:	Frequency match in Hz response.
 *
 * Return: 0 if all went well, else returns appropriate error value.
 */
static int ti_sci_cmd_clk_get_match_freq(const struct ti_sci_handle *handle,
					 u32 dev_id, u8 clk_id, u64 min_freq,
					 u64 target_freq, u64 max_freq,
					 u64 *match_freq)
{
	struct ti_sci_msg_resp_query_clock_freq *resp;
	struct ti_sci_msg_req_query_clock_freq req;
	struct ti_sci_info *info;
	struct ti_sci_xfer *xfer;
	int ret = 0;

	if (IS_ERR(handle))
		return PTR_ERR(handle);
	if (!handle || !match_freq)
		return -EINVAL;

	info = handle_to_ti_sci_info(handle);

	xfer = ti_sci_setup_one_xfer(info, TI_SCI_MSG_QUERY_CLOCK_FREQ,
				     TI_SCI_FLAG_REQ_ACK_ON_PROCESSED,
				     (u32 *)&req, sizeof(req), sizeof(*resp));
	if (IS_ERR(xfer)) {
		ret = PTR_ERR(xfer);
		dev_err(info->dev, "Message alloc failed(%d)\n", ret);
		return ret;
	}
	req.dev_id = dev_id;
	req.clk_id = clk_id;
	req.min_freq_hz = min_freq;
	req.target_freq_hz = target_freq;
	req.max_freq_hz = max_freq;

	ret = ti_sci_do_xfer(info, xfer);
	if (ret) {
		dev_err(info->dev, "Mbox send fail %d\n", ret);
		return ret;
	}

	resp = (struct ti_sci_msg_resp_query_clock_freq *)xfer->tx_message.buf;

	if (!ti_sci_is_response_ack(resp))
		ret = -ENODEV;
	else
		*match_freq = resp->freq_hz;

	return ret;
}

/**
 * ti_sci_cmd_clk_set_freq() - Set a frequency for clock
 * @handle:	pointer to TI SCI handle
 * @dev_id:	Device identifier this request is for
 * @clk_id:	Clock identifier for the device for this request.
 *		Each device has it's own set of clock inputs. This indexes
 *		which clock input to modify.
 * @min_freq:	The minimum allowable frequency in Hz. This is the minimum
 *		allowable programmed frequency and does not account for clock
 *		tolerances and jitter.
 * @target_freq: The target clock frequency in Hz. A frequency will be
 *		processed as close to this target frequency as possible.
 * @max_freq:	The maximum allowable frequency in Hz. This is the maximum
 *		allowable programmed frequency and does not account for clock
 *		tolerances and jitter.
 *
 * Return: 0 if all went well, else returns appropriate error value.
 */
static int ti_sci_cmd_clk_set_freq(const struct ti_sci_handle *handle,
				   u32 dev_id, u8 clk_id, u64 min_freq,
				   u64 target_freq, u64 max_freq)
{
	struct ti_sci_msg_req_set_clock_freq req;
	struct ti_sci_msg_hdr *resp;
	struct ti_sci_info *info;
	struct ti_sci_xfer *xfer;
	int ret = 0;

	if (IS_ERR(handle))
		return PTR_ERR(handle);
	if (!handle)
		return -EINVAL;

	info = handle_to_ti_sci_info(handle);

	xfer = ti_sci_setup_one_xfer(info, TI_SCI_MSG_SET_CLOCK_FREQ,
				     TI_SCI_FLAG_REQ_ACK_ON_PROCESSED,
				     (u32 *)&req, sizeof(req), sizeof(*resp));
	if (IS_ERR(xfer)) {
		ret = PTR_ERR(xfer);
		dev_err(info->dev, "Message alloc failed(%d)\n", ret);
		return ret;
	}
	req.dev_id = dev_id;
	req.clk_id = clk_id;
	req.min_freq_hz = min_freq;
	req.target_freq_hz = target_freq;
	req.max_freq_hz = max_freq;

	ret = ti_sci_do_xfer(info, xfer);
	if (ret) {
		dev_err(info->dev, "Mbox send fail %d\n", ret);
		return ret;
	}

	resp = (struct ti_sci_msg_hdr *)xfer->tx_message.buf;

	if (!ti_sci_is_response_ack(resp))
		return -ENODEV;

	return ret;
}

/**
 * ti_sci_cmd_clk_get_freq() - Get current frequency
 * @handle:	pointer to TI SCI handle
 * @dev_id:	Device identifier this request is for
 * @clk_id:	Clock identifier for the device for this request.
 *		Each device has it's own set of clock inputs. This indexes
 *		which clock input to modify.
 * @freq:	Currently frequency in Hz
 *
 * Return: 0 if all went well, else returns appropriate error value.
 */
static int ti_sci_cmd_clk_get_freq(const struct ti_sci_handle *handle,
				   u32 dev_id, u8 clk_id, u64 *freq)
{
	struct ti_sci_msg_resp_get_clock_freq *resp;
	struct ti_sci_msg_req_get_clock_freq req;
	struct ti_sci_info *info;
	struct ti_sci_xfer *xfer;
	int ret = 0;

	if (IS_ERR(handle))
		return PTR_ERR(handle);
	if (!handle || !freq)
		return -EINVAL;

	info = handle_to_ti_sci_info(handle);

	xfer = ti_sci_setup_one_xfer(info, TI_SCI_MSG_GET_CLOCK_FREQ,
				     TI_SCI_FLAG_REQ_ACK_ON_PROCESSED,
				     (u32 *)&req, sizeof(req), sizeof(*resp));
	if (IS_ERR(xfer)) {
		ret = PTR_ERR(xfer);
		dev_err(info->dev, "Message alloc failed(%d)\n", ret);
		return ret;
	}
	req.dev_id = dev_id;
	req.clk_id = clk_id;

	ret = ti_sci_do_xfer(info, xfer);
	if (ret) {
		dev_err(info->dev, "Mbox send fail %d\n", ret);
		return ret;
	}

	resp = (struct ti_sci_msg_resp_get_clock_freq *)xfer->tx_message.buf;

	if (!ti_sci_is_response_ack(resp))
		ret = -ENODEV;
	else
		*freq = resp->freq_hz;

	return ret;
}

/**
 * ti_sci_cmd_core_reboot() - Command to request system reset
 * @handle:	pointer to TI SCI handle
 *
 * Return: 0 if all went well, else returns appropriate error value.
 */
static int ti_sci_cmd_core_reboot(const struct ti_sci_handle *handle)
{
	struct ti_sci_msg_req_reboot req;
	struct ti_sci_msg_hdr *resp;
	struct ti_sci_info *info;
	struct ti_sci_xfer *xfer;
	int ret = 0;

	if (IS_ERR(handle))
		return PTR_ERR(handle);
	if (!handle)
		return -EINVAL;

	info = handle_to_ti_sci_info(handle);

	xfer = ti_sci_setup_one_xfer(info, TI_SCI_MSG_SYS_RESET,
				     TI_SCI_FLAG_REQ_ACK_ON_PROCESSED,
				     (u32 *)&req, sizeof(req), sizeof(*resp));
	if (IS_ERR(xfer)) {
		ret = PTR_ERR(xfer);
		dev_err(info->dev, "Message alloc failed(%d)\n", ret);
		return ret;
	}

	ret = ti_sci_do_xfer(info, xfer);
	if (ret) {
		dev_err(dev, "Mbox send fail %d\n", ret);
		return ret;
	}

	resp = (struct ti_sci_msg_hdr *)xfer->tx_message.buf;

	if (!ti_sci_is_response_ack(resp))
		return -ENODEV;

	return ret;
}

static int ti_sci_get_resource_type(struct ti_sci_info *info, u16 dev_id,
				    u16 *type)
{
	struct ti_sci_rm_type_map *rm_type_map = info->desc->rm_type_map;
	bool found = false;
	int i;

	/* If map is not provided then assume dev_id is used as type */
	if (!rm_type_map) {
		*type = dev_id;
		return 0;
	}

	for (i = 0; rm_type_map[i].dev_id; i++) {
		if (rm_type_map[i].dev_id == dev_id) {
			*type = rm_type_map[i].type;
			found = true;
			break;
		}
	}

	if (!found)
		return -EINVAL;

	return 0;
}

/**
 * ti_sci_get_resource_range - Helper to get a range of resources assigned
 *			       to a host. Resource is uniquely identified by
 *			       type and subtype.
 * @handle:		Pointer to TISCI handle.
 * @dev_id:		TISCI device ID.
 * @subtype:		Resource assignment subtype that is being requested
 *			from the given device.
 * @s_host:		Host processor ID to which the resources are allocated
 * @range_start:	Start index of the resource range
 * @range_num:		Number of resources in the range
 *
 * Return: 0 if all went fine, else return appropriate error.
 */
static int ti_sci_get_resource_range(const struct ti_sci_handle *handle,
				     u32 dev_id, u8 subtype, u8 s_host,
				     u16 *range_start, u16 *range_num)
{
	struct ti_sci_msg_resp_get_resource_range *resp;
	struct ti_sci_msg_req_get_resource_range req;
	struct ti_sci_xfer *xfer;
	struct ti_sci_info *info;
	u16 type;
	int ret = 0;

	if (IS_ERR(handle))
		return PTR_ERR(handle);
	if (!handle)
		return -EINVAL;

	info = handle_to_ti_sci_info(handle);

	xfer = ti_sci_setup_one_xfer(info, TI_SCI_MSG_GET_RESOURCE_RANGE,
				     TI_SCI_FLAG_REQ_ACK_ON_PROCESSED,
				     (u32 *)&req, sizeof(req), sizeof(*resp));
	if (IS_ERR(xfer)) {
		ret = PTR_ERR(xfer);
		dev_err(dev, "Message alloc failed(%d)\n", ret);
		return ret;
	}

	ret = ti_sci_get_resource_type(info, dev_id, &type);
	if (ret) {
		dev_err(dev, "rm type lookup failed for %u\n", dev_id);
		goto fail;
	}

	req.secondary_host = s_host;
	req.type = type & MSG_RM_RESOURCE_TYPE_MASK;
	req.subtype = subtype & MSG_RM_RESOURCE_SUBTYPE_MASK;

	ret = ti_sci_do_xfer(info, xfer);
	if (ret) {
		dev_err(dev, "Mbox send fail %d\n", ret);
		goto fail;
	}

	resp = (struct ti_sci_msg_resp_get_resource_range *)xfer->tx_message.buf;
	if (!ti_sci_is_response_ack(resp)) {
		ret = -ENODEV;
	} else if (!resp->range_start && !resp->range_num) {
		ret = -ENODEV;
	} else {
		*range_start = resp->range_start;
		*range_num = resp->range_num;
	};

fail:
	return ret;
}

/**
 * ti_sci_cmd_get_resource_range - Get a range of resources assigned to host
 *				   that is same as ti sci interface host.
 * @handle:		Pointer to TISCI handle.
 * @dev_id:		TISCI device ID.
 * @subtype:		Resource assignment subtype that is being requested
 *			from the given device.
 * @range_start:	Start index of the resource range
 * @range_num:		Number of resources in the range
 *
 * Return: 0 if all went fine, else return appropriate error.
 */
static int ti_sci_cmd_get_resource_range(const struct ti_sci_handle *handle,
					 u32 dev_id, u8 subtype,
					 u16 *range_start, u16 *range_num)
{
	return ti_sci_get_resource_range(handle, dev_id, subtype,
					 TI_SCI_IRQ_SECONDARY_HOST_INVALID,
					 range_start, range_num);
}

/**
 * ti_sci_cmd_get_resource_range_from_shost - Get a range of resources
 *					      assigned to a specified host.
 * @handle:		Pointer to TISCI handle.
 * @dev_id:		TISCI device ID.
 * @subtype:		Resource assignment subtype that is being requested
 *			from the given device.
 * @s_host:		Host processor ID to which the resources are allocated
 * @range_start:	Start index of the resource range
 * @range_num:		Number of resources in the range
 *
 * Return: 0 if all went fine, else return appropriate error.
 */
static
int ti_sci_cmd_get_resource_range_from_shost(const struct ti_sci_handle *handle,
					     u32 dev_id, u8 subtype, u8 s_host,
					     u16 *range_start, u16 *range_num)
{
	return ti_sci_get_resource_range(handle, dev_id, subtype, s_host,
					 range_start, range_num);
}

/**
 * ti_sci_cmd_query_msmc() - Command to query currently available msmc memory
 * @handle:		pointer to TI SCI handle
 * @msms_start:		MSMC start as returned by tisci
 * @msmc_end:		MSMC end as returned by tisci
 *
 * Return: 0 if all went well, else returns appropriate error value.
 */
static int ti_sci_cmd_query_msmc(const struct ti_sci_handle *handle,
				 u64 *msmc_start, u64 *msmc_end)
{
	struct ti_sci_msg_resp_query_msmc *resp;
	struct ti_sci_msg_hdr req;
	struct ti_sci_info *info;
	struct ti_sci_xfer *xfer;
	int ret = 0;

	if (IS_ERR(handle))
		return PTR_ERR(handle);
	if (!handle)
		return -EINVAL;

	info = handle_to_ti_sci_info(handle);

	xfer = ti_sci_setup_one_xfer(info, TISCI_MSG_QUERY_MSMC,
				     TI_SCI_FLAG_REQ_ACK_ON_PROCESSED,
				     (u32 *)&req, sizeof(req), sizeof(*resp));
	if (IS_ERR(xfer)) {
		ret = PTR_ERR(xfer);
		dev_err(info->dev, "Message alloc failed(%d)\n", ret);
		return ret;
	}

	ret = ti_sci_do_xfer(info, xfer);
	if (ret) {
		dev_err(dev, "Mbox send fail %d\n", ret);
		return ret;
	}

	resp = (struct ti_sci_msg_resp_query_msmc *)xfer->tx_message.buf;

	if (!ti_sci_is_response_ack(resp))
		return -ENODEV;

	*msmc_start = ((u64)resp->msmc_start_high << TISCI_ADDR_HIGH_SHIFT) |
			resp->msmc_start_low;
	*msmc_end = ((u64)resp->msmc_end_high << TISCI_ADDR_HIGH_SHIFT) |
			resp->msmc_end_low;

	return ret;
}

/**
 * ti_sci_cmd_proc_request() - Command to request a physical processor control
 * @handle:	Pointer to TI SCI handle
 * @proc_id:	Processor ID this request is for
 *
 * Return: 0 if all went well, else returns appropriate error value.
 */
static int ti_sci_cmd_proc_request(const struct ti_sci_handle *handle,
				   u8 proc_id)
{
	struct ti_sci_msg_req_proc_request req;
	struct ti_sci_msg_hdr *resp;
	struct ti_sci_info *info;
	struct ti_sci_xfer *xfer;
	int ret = 0;

	if (IS_ERR(handle))
		return PTR_ERR(handle);
	if (!handle)
		return -EINVAL;

	info = handle_to_ti_sci_info(handle);

	xfer = ti_sci_setup_one_xfer(info, TISCI_MSG_PROC_REQUEST,
				     TI_SCI_FLAG_REQ_ACK_ON_PROCESSED,
				     (u32 *)&req, sizeof(req), sizeof(*resp));
	if (IS_ERR(xfer)) {
		ret = PTR_ERR(xfer);
		dev_err(info->dev, "Message alloc failed(%d)\n", ret);
		return ret;
	}
	req.processor_id = proc_id;

	ret = ti_sci_do_xfer(info, xfer);
	if (ret) {
		dev_err(info->dev, "Mbox send fail %d\n", ret);
		return ret;
	}

	resp = (struct ti_sci_msg_hdr *)xfer->tx_message.buf;

	if (!ti_sci_is_response_ack(resp))
		ret = -ENODEV;

	return ret;
}

/**
 * ti_sci_cmd_proc_release() - Command to release a physical processor control
 * @handle:	Pointer to TI SCI handle
 * @proc_id:	Processor ID this request is for
 *
 * Return: 0 if all went well, else returns appropriate error value.
 */
static int ti_sci_cmd_proc_release(const struct ti_sci_handle *handle,
				   u8 proc_id)
{
	struct ti_sci_msg_req_proc_release req;
	struct ti_sci_msg_hdr *resp;
	struct ti_sci_info *info;
	struct ti_sci_xfer *xfer;
	int ret = 0;

	if (IS_ERR(handle))
		return PTR_ERR(handle);
	if (!handle)
		return -EINVAL;

	info = handle_to_ti_sci_info(handle);

	xfer = ti_sci_setup_one_xfer(info, TISCI_MSG_PROC_RELEASE,
				     TI_SCI_FLAG_REQ_ACK_ON_PROCESSED,
				     (u32 *)&req, sizeof(req), sizeof(*resp));
	if (IS_ERR(xfer)) {
		ret = PTR_ERR(xfer);
		dev_err(info->dev, "Message alloc failed(%d)\n", ret);
		return ret;
	}
	req.processor_id = proc_id;

	ret = ti_sci_do_xfer(info, xfer);
	if (ret) {
		dev_err(info->dev, "Mbox send fail %d\n", ret);
		return ret;
	}

	resp = (struct ti_sci_msg_hdr *)xfer->tx_message.buf;

	if (!ti_sci_is_response_ack(resp))
		ret = -ENODEV;

	return ret;
}

/**
 * ti_sci_cmd_proc_handover() - Command to handover a physical processor
 *				control to a host in the processor's access
 *				control list.
 * @handle:	Pointer to TI SCI handle
 * @proc_id:	Processor ID this request is for
 * @host_id:	Host ID to get the control of the processor
 *
 * Return: 0 if all went well, else returns appropriate error value.
 */
static int ti_sci_cmd_proc_handover(const struct ti_sci_handle *handle,
				    u8 proc_id, u8 host_id)
{
	struct ti_sci_msg_req_proc_handover req;
	struct ti_sci_msg_hdr *resp;
	struct ti_sci_info *info;
	struct ti_sci_xfer *xfer;
	int ret = 0;

	if (IS_ERR(handle))
		return PTR_ERR(handle);
	if (!handle)
		return -EINVAL;

	info = handle_to_ti_sci_info(handle);

	xfer = ti_sci_setup_one_xfer(info, TISCI_MSG_PROC_HANDOVER,
				     TI_SCI_FLAG_REQ_ACK_ON_PROCESSED,
				     (u32 *)&req, sizeof(req), sizeof(*resp));
	if (IS_ERR(xfer)) {
		ret = PTR_ERR(xfer);
		dev_err(info->dev, "Message alloc failed(%d)\n", ret);
		return ret;
	}
	req.processor_id = proc_id;
	req.host_id = host_id;

	ret = ti_sci_do_xfer(info, xfer);
	if (ret) {
		dev_err(info->dev, "Mbox send fail %d\n", ret);
		return ret;
	}

	resp = (struct ti_sci_msg_hdr *)xfer->tx_message.buf;

	if (!ti_sci_is_response_ack(resp))
		ret = -ENODEV;

	return ret;
}

/**
 * ti_sci_cmd_set_proc_boot_cfg() - Command to set the processor boot
 *				    configuration flags
 * @handle:		Pointer to TI SCI handle
 * @proc_id:		Processor ID this request is for
 * @config_flags_set:	Configuration flags to be set
 * @config_flags_clear:	Configuration flags to be cleared.
 *
 * Return: 0 if all went well, else returns appropriate error value.
 */
static int ti_sci_cmd_set_proc_boot_cfg(const struct ti_sci_handle *handle,
					u8 proc_id, u64 bootvector,
					u32 config_flags_set,
					u32 config_flags_clear)
{
	struct ti_sci_msg_req_set_proc_boot_config req;
	struct ti_sci_msg_hdr *resp;
	struct ti_sci_info *info;
	struct ti_sci_xfer *xfer;
	int ret = 0;

	if (IS_ERR(handle))
		return PTR_ERR(handle);
	if (!handle)
		return -EINVAL;

	info = handle_to_ti_sci_info(handle);

	xfer = ti_sci_setup_one_xfer(info, TISCI_MSG_SET_PROC_BOOT_CONFIG,
				     TI_SCI_FLAG_REQ_ACK_ON_PROCESSED,
				     (u32 *)&req, sizeof(req), sizeof(*resp));
	if (IS_ERR(xfer)) {
		ret = PTR_ERR(xfer);
		dev_err(info->dev, "Message alloc failed(%d)\n", ret);
		return ret;
	}
	req.processor_id = proc_id;
	req.bootvector_low = bootvector & TISCI_ADDR_LOW_MASK;
	req.bootvector_high = (bootvector & TISCI_ADDR_HIGH_MASK) >>
				TISCI_ADDR_HIGH_SHIFT;
	req.config_flags_set = config_flags_set;
	req.config_flags_clear = config_flags_clear;

	ret = ti_sci_do_xfer(info, xfer);
	if (ret) {
		dev_err(info->dev, "Mbox send fail %d\n", ret);
		return ret;
	}

	resp = (struct ti_sci_msg_hdr *)xfer->tx_message.buf;

	if (!ti_sci_is_response_ack(resp))
		ret = -ENODEV;

	return ret;
}

/**
 * ti_sci_cmd_set_proc_boot_ctrl() - Command to set the processor boot
 *				     control flags
 * @handle:			Pointer to TI SCI handle
 * @proc_id:			Processor ID this request is for
 * @control_flags_set:		Control flags to be set
 * @control_flags_clear:	Control flags to be cleared
 *
 * Return: 0 if all went well, else returns appropriate error value.
 */
static int ti_sci_cmd_set_proc_boot_ctrl(const struct ti_sci_handle *handle,
					 u8 proc_id, u32 control_flags_set,
					 u32 control_flags_clear)
{
	struct ti_sci_msg_req_set_proc_boot_ctrl req;
	struct ti_sci_msg_hdr *resp;
	struct ti_sci_info *info;
	struct ti_sci_xfer *xfer;
	int ret = 0;

	if (IS_ERR(handle))
		return PTR_ERR(handle);
	if (!handle)
		return -EINVAL;

	info = handle_to_ti_sci_info(handle);

	xfer = ti_sci_setup_one_xfer(info, TISCI_MSG_SET_PROC_BOOT_CTRL,
				     TI_SCI_FLAG_REQ_ACK_ON_PROCESSED,
				     (u32 *)&req, sizeof(req), sizeof(*resp));
	if (IS_ERR(xfer)) {
		ret = PTR_ERR(xfer);
		dev_err(info->dev, "Message alloc failed(%d)\n", ret);
		return ret;
	}
	req.processor_id = proc_id;
	req.control_flags_set = control_flags_set;
	req.control_flags_clear = control_flags_clear;

	ret = ti_sci_do_xfer(info, xfer);
	if (ret) {
		dev_err(info->dev, "Mbox send fail %d\n", ret);
		return ret;
	}

	resp = (struct ti_sci_msg_hdr *)xfer->tx_message.buf;

	if (!ti_sci_is_response_ack(resp))
		ret = -ENODEV;

	return ret;
}

/**
 * ti_sci_cmd_proc_auth_boot_image() - Command to authenticate and load the
 *			image and then set the processor configuration flags.
 * @handle:	Pointer to TI SCI handle
 * @image_addr:	Memory address at which payload image and certificate is
 *		located in memory, this is updated if the image data is
 *		moved during authentication.
 * @image_size: This is updated with the final size of the image after
 *		authentication.
 *
 * Return: 0 if all went well, else returns appropriate error value.
 */
static int ti_sci_cmd_proc_auth_boot_image(const struct ti_sci_handle *handle,
					   u64 *image_addr, u32 *image_size)
{
	struct ti_sci_msg_req_proc_auth_boot_image req;
	struct ti_sci_msg_resp_proc_auth_boot_image *resp;
	struct ti_sci_info *info;
	struct ti_sci_xfer *xfer;
	int ret = 0;

	if (IS_ERR(handle))
		return PTR_ERR(handle);
	if (!handle)
		return -EINVAL;

	info = handle_to_ti_sci_info(handle);

	xfer = ti_sci_setup_one_xfer(info, TISCI_MSG_PROC_AUTH_BOOT_IMIAGE,
				     TI_SCI_FLAG_REQ_ACK_ON_PROCESSED,
				     (u32 *)&req, sizeof(req), sizeof(*resp));
	if (IS_ERR(xfer)) {
		ret = PTR_ERR(xfer);
		dev_err(info->dev, "Message alloc failed(%d)\n", ret);
		return ret;
	}
	req.cert_addr_low = *image_addr & TISCI_ADDR_LOW_MASK;
	req.cert_addr_high = (*image_addr & TISCI_ADDR_HIGH_MASK) >>
				TISCI_ADDR_HIGH_SHIFT;

	ret = ti_sci_do_xfer(info, xfer);
	if (ret) {
		dev_err(info->dev, "Mbox send fail %d\n", ret);
		return ret;
	}

	resp = (struct ti_sci_msg_resp_proc_auth_boot_image *)xfer->tx_message.buf;

	if (!ti_sci_is_response_ack(resp))
		return -ENODEV;

	*image_addr = (resp->image_addr_low & TISCI_ADDR_LOW_MASK) |
			(((u64)resp->image_addr_high <<
			  TISCI_ADDR_HIGH_SHIFT) & TISCI_ADDR_HIGH_MASK);
	*image_size = resp->image_size;

	return ret;
}

/**
 * ti_sci_cmd_get_proc_boot_status() - Command to get the processor boot status
 * @handle:	Pointer to TI SCI handle
 * @proc_id:	Processor ID this request is for
 *
 * Return: 0 if all went well, else returns appropriate error value.
 */
static int ti_sci_cmd_get_proc_boot_status(const struct ti_sci_handle *handle,
					   u8 proc_id, u64 *bv, u32 *cfg_flags,
					   u32 *ctrl_flags, u32 *sts_flags)
{
	struct ti_sci_msg_resp_get_proc_boot_status *resp;
	struct ti_sci_msg_req_get_proc_boot_status req;
	struct ti_sci_info *info;
	struct ti_sci_xfer *xfer;
	int ret = 0;

	if (IS_ERR(handle))
		return PTR_ERR(handle);
	if (!handle)
		return -EINVAL;

	info = handle_to_ti_sci_info(handle);

	xfer = ti_sci_setup_one_xfer(info, TISCI_MSG_GET_PROC_BOOT_STATUS,
				     TI_SCI_FLAG_REQ_ACK_ON_PROCESSED,
				     (u32 *)&req, sizeof(req), sizeof(*resp));
	if (IS_ERR(xfer)) {
		ret = PTR_ERR(xfer);
		dev_err(info->dev, "Message alloc failed(%d)\n", ret);
		return ret;
	}
	req.processor_id = proc_id;

	ret = ti_sci_do_xfer(info, xfer);
	if (ret) {
		dev_err(info->dev, "Mbox send fail %d\n", ret);
		return ret;
	}

	resp = (struct ti_sci_msg_resp_get_proc_boot_status *)
							xfer->tx_message.buf;

	if (!ti_sci_is_response_ack(resp))
		return -ENODEV;
	*bv = (resp->bootvector_low & TISCI_ADDR_LOW_MASK) |
			(((u64)resp->bootvector_high  <<
			  TISCI_ADDR_HIGH_SHIFT) & TISCI_ADDR_HIGH_MASK);
	*cfg_flags = resp->config_flags;
	*ctrl_flags = resp->control_flags;
	*sts_flags = resp->status_flags;

	return ret;
}

/**
 * ti_sci_cmd_ring_config() - configure RA ring
 * @handle:	pointer to TI SCI handle
 * @valid_params: Bitfield defining validity of ring configuration parameters.
 * @nav_id: Device ID of Navigator Subsystem from which the ring is allocated
 * @index: Ring index.
 * @addr_lo: The ring base address lo 32 bits
 * @addr_hi: The ring base address hi 32 bits
 * @count: Number of ring elements.
 * @mode: The mode of the ring
 * @size: The ring element size.
 * @order_id: Specifies the ring's bus order ID.
 *
 * Return: 0 if all went well, else returns appropriate error value.
 *
 * See @ti_sci_msg_rm_ring_cfg_req for more info.
 */
static int ti_sci_cmd_ring_config(const struct ti_sci_handle *handle,
				  u32 valid_params, u16 nav_id, u16 index,
				  u32 addr_lo, u32 addr_hi, u32 count,
				  u8 mode, u8 size, u8 order_id)
{
	struct ti_sci_msg_rm_ring_cfg_resp *resp;
	struct ti_sci_msg_rm_ring_cfg_req req;
	struct ti_sci_xfer *xfer;
	struct ti_sci_info *info;
	int ret = 0;

	if (IS_ERR(handle))
		return PTR_ERR(handle);
	if (!handle)
		return -EINVAL;

	info = handle_to_ti_sci_info(handle);

	xfer = ti_sci_setup_one_xfer(info, TI_SCI_MSG_RM_RING_CFG,
				     TI_SCI_FLAG_REQ_ACK_ON_PROCESSED,
				     (u32 *)&req, sizeof(req), sizeof(*resp));
	if (IS_ERR(xfer)) {
		ret = PTR_ERR(xfer);
		dev_err(info->dev, "RM_RA:Message config failed(%d)\n", ret);
		return ret;
	}
	req.valid_params = valid_params;
	req.nav_id = nav_id;
	req.index = index;
	req.addr_lo = addr_lo;
	req.addr_hi = addr_hi;
	req.count = count;
	req.mode = mode;
	req.size = size;
	req.order_id = order_id;

	ret = ti_sci_do_xfer(info, xfer);
	if (ret) {
		dev_err(info->dev, "RM_RA:Mbox config send fail %d\n", ret);
		goto fail;
	}

	resp = (struct ti_sci_msg_rm_ring_cfg_resp *)xfer->tx_message.buf;

	ret = ti_sci_is_response_ack(resp) ? 0 : -ENODEV;

fail:
	dev_dbg(info->dev, "RM_RA:config ring %u ret:%d\n", index, ret);
	return ret;
}

/**
 * ti_sci_cmd_ring_get_config() - get RA ring configuration
 * @handle:	pointer to TI SCI handle
 * @nav_id: Device ID of Navigator Subsystem from which the ring is allocated
 * @index: Ring index.
 * @addr_lo: returns ring's base address lo 32 bits
 * @addr_hi: returns ring's base address hi 32 bits
 * @count: returns number of ring elements.
 * @mode: returns mode of the ring
 * @size: returns ring element size.
 * @order_id: returns ring's bus order ID.
 *
 * Return: 0 if all went well, else returns appropriate error value.
 *
 * See @ti_sci_msg_rm_ring_get_cfg_req for more info.
 */
static int ti_sci_cmd_ring_get_config(const struct ti_sci_handle *handle,
				      u32 nav_id, u32 index, u8 *mode,
				      u32 *addr_lo, u32 *addr_hi,
				      u32 *count, u8 *size, u8 *order_id)
{
	struct ti_sci_msg_rm_ring_get_cfg_resp *resp;
	struct ti_sci_msg_rm_ring_get_cfg_req req;
	struct ti_sci_xfer *xfer;
	struct ti_sci_info *info;
	int ret = 0;

	if (IS_ERR(handle))
		return PTR_ERR(handle);
	if (!handle)
		return -EINVAL;

	info = handle_to_ti_sci_info(handle);

	xfer = ti_sci_setup_one_xfer(info, TI_SCI_MSG_RM_RING_GET_CFG,
				     TI_SCI_FLAG_REQ_ACK_ON_PROCESSED,
				     (u32 *)&req, sizeof(req), sizeof(*resp));
	if (IS_ERR(xfer)) {
		ret = PTR_ERR(xfer);
		dev_err(info->dev,
			"RM_RA:Message get config failed(%d)\n", ret);
		return ret;
	}
	req.nav_id = nav_id;
	req.index = index;

	ret = ti_sci_do_xfer(info, xfer);
	if (ret) {
		dev_err(info->dev, "RM_RA:Mbox get config send fail %d\n", ret);
		goto fail;
	}

	resp = (struct ti_sci_msg_rm_ring_get_cfg_resp *)xfer->tx_message.buf;

	if (!ti_sci_is_response_ack(resp)) {
		ret = -ENODEV;
	} else {
		if (mode)
			*mode = resp->mode;
		if (addr_lo)
			*addr_lo = resp->addr_lo;
		if (addr_hi)
			*addr_hi = resp->addr_hi;
		if (count)
			*count = resp->count;
		if (size)
			*size = resp->size;
		if (order_id)
			*order_id = resp->order_id;
	};

fail:
	dev_dbg(info->dev, "RM_RA:get config ring %u ret:%d\n", index, ret);
	return ret;
}

static int ti_sci_cmd_rm_psil_pair(const struct ti_sci_handle *handle,
				   u32 nav_id, u32 src_thread, u32 dst_thread)
{
	struct ti_sci_msg_hdr *resp;
	struct ti_sci_msg_psil_pair req;
	struct ti_sci_xfer *xfer;
	struct ti_sci_info *info;
	int ret = 0;

	if (IS_ERR(handle))
		return PTR_ERR(handle);
	if (!handle)
		return -EINVAL;

	info = handle_to_ti_sci_info(handle);

	xfer = ti_sci_setup_one_xfer(info, TI_SCI_MSG_RM_PSIL_PAIR,
				     TI_SCI_FLAG_REQ_ACK_ON_PROCESSED,
				     (u32 *)&req, sizeof(req), sizeof(*resp));
	if (IS_ERR(xfer)) {
		ret = PTR_ERR(xfer);
		dev_err(info->dev, "RM_PSIL:Message alloc failed(%d)\n", ret);
		return ret;
	}
	req.nav_id = nav_id;
	req.src_thread = src_thread;
	req.dst_thread = dst_thread;

	ret = ti_sci_do_xfer(info, xfer);
	if (ret) {
		dev_err(info->dev, "RM_PSIL:Mbox send fail %d\n", ret);
		goto fail;
	}

	resp = (struct ti_sci_msg_hdr *)xfer->tx_message.buf;
	ret = ti_sci_is_response_ack(resp) ? 0 : -ENODEV;

fail:
	dev_dbg(info->dev, "RM_PSIL: nav: %u link pair %u->%u ret:%u\n",
		nav_id, src_thread, dst_thread, ret);
	return ret;
}

static int ti_sci_cmd_rm_psil_unpair(const struct ti_sci_handle *handle,
				     u32 nav_id, u32 src_thread, u32 dst_thread)
{
	struct ti_sci_msg_hdr *resp;
	struct ti_sci_msg_psil_unpair req;
	struct ti_sci_xfer *xfer;
	struct ti_sci_info *info;
	int ret = 0;

	if (IS_ERR(handle))
		return PTR_ERR(handle);
	if (!handle)
		return -EINVAL;

	info = handle_to_ti_sci_info(handle);

	xfer = ti_sci_setup_one_xfer(info, TI_SCI_MSG_RM_PSIL_UNPAIR,
				     TI_SCI_FLAG_REQ_ACK_ON_PROCESSED,
				     (u32 *)&req, sizeof(req), sizeof(*resp));
	if (IS_ERR(xfer)) {
		ret = PTR_ERR(xfer);
		dev_err(info->dev, "RM_PSIL:Message alloc failed(%d)\n", ret);
		return ret;
	}
	req.nav_id = nav_id;
	req.src_thread = src_thread;
	req.dst_thread = dst_thread;

	ret = ti_sci_do_xfer(info, xfer);
	if (ret) {
		dev_err(info->dev, "RM_PSIL:Mbox send fail %d\n", ret);
		goto fail;
	}

	resp = (struct ti_sci_msg_hdr *)xfer->tx_message.buf;
	ret = ti_sci_is_response_ack(resp) ? 0 : -ENODEV;

fail:
	dev_dbg(info->dev, "RM_PSIL: link unpair %u->%u ret:%u\n",
		src_thread, dst_thread, ret);
	return ret;
}

static int ti_sci_cmd_rm_udmap_tx_ch_cfg(
			const struct ti_sci_handle *handle,
			const struct ti_sci_msg_rm_udmap_tx_ch_cfg *params)
{
	struct ti_sci_msg_rm_udmap_tx_ch_cfg_resp *resp;
	struct ti_sci_msg_rm_udmap_tx_ch_cfg_req req;
	struct ti_sci_xfer *xfer;
	struct ti_sci_info *info;
	int ret = 0;

	if (IS_ERR(handle))
		return PTR_ERR(handle);
	if (!handle)
		return -EINVAL;

	info = handle_to_ti_sci_info(handle);

	xfer = ti_sci_setup_one_xfer(info, TISCI_MSG_RM_UDMAP_TX_CH_CFG,
				     TI_SCI_FLAG_REQ_ACK_ON_PROCESSED,
				     (u32 *)&req, sizeof(req), sizeof(*resp));
	if (IS_ERR(xfer)) {
		ret = PTR_ERR(xfer);
		dev_err(info->dev, "Message TX_CH_CFG alloc failed(%d)\n", ret);
		return ret;
	}
	req.valid_params = params->valid_params;
	req.nav_id = params->nav_id;
	req.index = params->index;
	req.tx_pause_on_err = params->tx_pause_on_err;
	req.tx_filt_einfo = params->tx_filt_einfo;
	req.tx_filt_pswords = params->tx_filt_pswords;
	req.tx_atype = params->tx_atype;
	req.tx_chan_type = params->tx_chan_type;
	req.tx_supr_tdpkt = params->tx_supr_tdpkt;
	req.tx_fetch_size = params->tx_fetch_size;
	req.tx_credit_count = params->tx_credit_count;
	req.txcq_qnum = params->txcq_qnum;
	req.tx_priority = params->tx_priority;
	req.tx_qos = params->tx_qos;
	req.tx_orderid = params->tx_orderid;
	req.fdepth = params->fdepth;
	req.tx_sched_priority = params->tx_sched_priority;

	ret = ti_sci_do_xfer(info, xfer);
	if (ret) {
		dev_err(info->dev, "Mbox send TX_CH_CFG fail %d\n", ret);
		goto fail;
	}

	resp =
	      (struct ti_sci_msg_rm_udmap_tx_ch_cfg_resp *)xfer->tx_message.buf;
	ret = ti_sci_is_response_ack(resp) ? 0 : -EINVAL;

fail:
	dev_dbg(info->dev, "TX_CH_CFG: chn %u ret:%u\n", params->index, ret);
	return ret;
}

static int ti_sci_cmd_rm_udmap_rx_ch_cfg(
			const struct ti_sci_handle *handle,
			const struct ti_sci_msg_rm_udmap_rx_ch_cfg *params)
{
	struct ti_sci_msg_rm_udmap_rx_ch_cfg_resp *resp;
	struct ti_sci_msg_rm_udmap_rx_ch_cfg_req req;
	struct ti_sci_xfer *xfer;
	struct ti_sci_info *info;
	int ret = 0;

	if (IS_ERR(handle))
		return PTR_ERR(handle);
	if (!handle)
		return -EINVAL;

	info = handle_to_ti_sci_info(handle);

	xfer = ti_sci_setup_one_xfer(info, TISCI_MSG_RM_UDMAP_RX_CH_CFG,
				     TI_SCI_FLAG_REQ_ACK_ON_PROCESSED,
				     (u32 *)&req, sizeof(req), sizeof(*resp));
	if (IS_ERR(xfer)) {
		ret = PTR_ERR(xfer);
		dev_err(info->dev, "Message RX_CH_CFG alloc failed(%d)\n", ret);
		return ret;
	}

	req.valid_params = params->valid_params;
	req.nav_id = params->nav_id;
	req.index = params->index;
	req.rx_fetch_size = params->rx_fetch_size;
	req.rxcq_qnum = params->rxcq_qnum;
	req.rx_priority = params->rx_priority;
	req.rx_qos = params->rx_qos;
	req.rx_orderid = params->rx_orderid;
	req.rx_sched_priority = params->rx_sched_priority;
	req.flowid_start = params->flowid_start;
	req.flowid_cnt = params->flowid_cnt;
	req.rx_pause_on_err = params->rx_pause_on_err;
	req.rx_atype = params->rx_atype;
	req.rx_chan_type = params->rx_chan_type;
	req.rx_ignore_short = params->rx_ignore_short;
	req.rx_ignore_long = params->rx_ignore_long;

	ret = ti_sci_do_xfer(info, xfer);
	if (ret) {
		dev_err(info->dev, "Mbox send RX_CH_CFG fail %d\n", ret);
		goto fail;
	}

	resp =
	      (struct ti_sci_msg_rm_udmap_rx_ch_cfg_resp *)xfer->tx_message.buf;
	ret = ti_sci_is_response_ack(resp) ? 0 : -EINVAL;

fail:
	dev_dbg(info->dev, "RX_CH_CFG: chn %u ret:%d\n", params->index, ret);
	return ret;
}

static int ti_sci_cmd_rm_udmap_rx_flow_cfg(
			const struct ti_sci_handle *handle,
			const struct ti_sci_msg_rm_udmap_flow_cfg *params)
{
	struct ti_sci_msg_rm_udmap_flow_cfg_resp *resp;
	struct ti_sci_msg_rm_udmap_flow_cfg_req req;
	struct ti_sci_xfer *xfer;
	struct ti_sci_info *info;
	int ret = 0;

	if (IS_ERR(handle))
		return PTR_ERR(handle);
	if (!handle)
		return -EINVAL;

	info = handle_to_ti_sci_info(handle);

	xfer = ti_sci_setup_one_xfer(info, TISCI_MSG_RM_UDMAP_FLOW_CFG,
				     TI_SCI_FLAG_REQ_ACK_ON_PROCESSED,
				     (u32 *)&req, sizeof(req), sizeof(*resp));
	if (IS_ERR(xfer)) {
		ret = PTR_ERR(xfer);
		dev_err(dev, "RX_FL_CFG: Message alloc failed(%d)\n", ret);
		return ret;
	}

	req.valid_params = params->valid_params;
	req.nav_id = params->nav_id;
	req.flow_index = params->flow_index;
	req.rx_einfo_present = params->rx_einfo_present;
	req.rx_psinfo_present = params->rx_psinfo_present;
	req.rx_error_handling = params->rx_error_handling;
	req.rx_desc_type = params->rx_desc_type;
	req.rx_sop_offset = params->rx_sop_offset;
	req.rx_dest_qnum = params->rx_dest_qnum;
	req.rx_src_tag_hi = params->rx_src_tag_hi;
	req.rx_src_tag_lo = params->rx_src_tag_lo;
	req.rx_dest_tag_hi = params->rx_dest_tag_hi;
	req.rx_dest_tag_lo = params->rx_dest_tag_lo;
	req.rx_src_tag_hi_sel = params->rx_src_tag_hi_sel;
	req.rx_src_tag_lo_sel = params->rx_src_tag_lo_sel;
	req.rx_dest_tag_hi_sel = params->rx_dest_tag_hi_sel;
	req.rx_dest_tag_lo_sel = params->rx_dest_tag_lo_sel;
	req.rx_fdq0_sz0_qnum = params->rx_fdq0_sz0_qnum;
	req.rx_fdq1_qnum = params->rx_fdq1_qnum;
	req.rx_fdq2_qnum = params->rx_fdq2_qnum;
	req.rx_fdq3_qnum = params->rx_fdq3_qnum;
	req.rx_ps_location = params->rx_ps_location;

	ret = ti_sci_do_xfer(info, xfer);
	if (ret) {
		dev_err(dev, "RX_FL_CFG: Mbox send fail %d\n", ret);
		goto fail;
	}

	resp =
	       (struct ti_sci_msg_rm_udmap_flow_cfg_resp *)xfer->tx_message.buf;
	ret = ti_sci_is_response_ack(resp) ? 0 : -EINVAL;

fail:
	dev_dbg(info->dev, "RX_FL_CFG: %u ret:%d\n", params->flow_index, ret);
	return ret;
}

/**
 * ti_sci_cmd_set_fwl_region() - Request for configuring a firewall region
 * @handle:    pointer to TI SCI handle
 * @region:    region configuration parameters
 *
 * Return: 0 if all went well, else returns appropriate error value.
 */
static int ti_sci_cmd_set_fwl_region(const struct ti_sci_handle *handle,
				     const struct ti_sci_msg_fwl_region *region)
{
	struct ti_sci_msg_fwl_set_firewall_region_req req;
	struct ti_sci_msg_hdr *resp;
	struct ti_sci_info *info;
	struct ti_sci_xfer *xfer;
	int ret = 0;

	if (IS_ERR(handle))
		return PTR_ERR(handle);
	if (!handle)
		return -EINVAL;

	info = handle_to_ti_sci_info(handle);

	xfer = ti_sci_setup_one_xfer(info, TISCI_MSG_FWL_SET,
				     TI_SCI_FLAG_REQ_ACK_ON_PROCESSED,
				     (u32 *)&req, sizeof(req), sizeof(*resp));
	if (IS_ERR(xfer)) {
		ret = PTR_ERR(xfer);
		dev_err(info->dev, "Message alloc failed(%d)\n", ret);
		return ret;
	}

	req.fwl_id = region->fwl_id;
	req.region = region->region;
	req.n_permission_regs = region->n_permission_regs;
	req.control = region->control;
	req.permissions[0] = region->permissions[0];
	req.permissions[1] = region->permissions[1];
	req.permissions[2] = region->permissions[2];
	req.start_address = region->start_address;
	req.end_address = region->end_address;

	ret = ti_sci_do_xfer(info, xfer);
	if (ret) {
		dev_err(info->dev, "Mbox send fail %d\n", ret);
		return ret;
	}

	resp = (struct ti_sci_msg_hdr *)xfer->tx_message.buf;

	if (!ti_sci_is_response_ack(resp))
		return -ENODEV;

	return 0;
}

/**
 * ti_sci_cmd_get_fwl_region() - Request for getting a firewall region
 * @handle:    pointer to TI SCI handle
 * @region:    region configuration parameters
 *
 * Return: 0 if all went well, else returns appropriate error value.
 */
static int ti_sci_cmd_get_fwl_region(const struct ti_sci_handle *handle,
				     struct ti_sci_msg_fwl_region *region)
{
	struct ti_sci_msg_fwl_get_firewall_region_req req;
	struct ti_sci_msg_fwl_get_firewall_region_resp *resp;
	struct ti_sci_info *info;
	struct ti_sci_xfer *xfer;
	int ret = 0;

	if (IS_ERR(handle))
		return PTR_ERR(handle);
	if (!handle)
		return -EINVAL;

	info = handle_to_ti_sci_info(handle);

	xfer = ti_sci_setup_one_xfer(info, TISCI_MSG_FWL_GET,
				     TI_SCI_FLAG_REQ_ACK_ON_PROCESSED,
				     (u32 *)&req, sizeof(req), sizeof(*resp));
	if (IS_ERR(xfer)) {
		ret = PTR_ERR(xfer);
		dev_err(info->dev, "Message alloc failed(%d)\n", ret);
		return ret;
	}

	req.fwl_id = region->fwl_id;
	req.region = region->region;
	req.n_permission_regs = region->n_permission_regs;

	ret = ti_sci_do_xfer(info, xfer);
	if (ret) {
		dev_err(info->dev, "Mbox send fail %d\n", ret);
		return ret;
	}

	resp = (struct ti_sci_msg_fwl_get_firewall_region_resp *)xfer->tx_message.buf;

	if (!ti_sci_is_response_ack(resp))
		return -ENODEV;

	region->fwl_id = resp->fwl_id;
	region->region = resp->region;
	region->n_permission_regs = resp->n_permission_regs;
	region->control = resp->control;
	region->permissions[0] = resp->permissions[0];
	region->permissions[1] = resp->permissions[1];
	region->permissions[2] = resp->permissions[2];
	region->start_address = resp->start_address;
	region->end_address = resp->end_address;

	return 0;
}

/**
 * ti_sci_cmd_change_fwl_owner() - Request for changing a firewall owner
 * @handle:    pointer to TI SCI handle
 * @region:    region configuration parameters
 *
 * Return: 0 if all went well, else returns appropriate error value.
 */
static int ti_sci_cmd_change_fwl_owner(const struct ti_sci_handle *handle,
				       struct ti_sci_msg_fwl_owner *owner)
{
	struct ti_sci_msg_fwl_change_owner_info_req req;
	struct ti_sci_msg_fwl_change_owner_info_resp *resp;
	struct ti_sci_info *info;
	struct ti_sci_xfer *xfer;
	int ret = 0;

	if (IS_ERR(handle))
		return PTR_ERR(handle);
	if (!handle)
		return -EINVAL;

	info = handle_to_ti_sci_info(handle);

	xfer = ti_sci_setup_one_xfer(info, TISCI_MSG_FWL_CHANGE_OWNER,
				     TI_SCI_FLAG_REQ_ACK_ON_PROCESSED,
				     (u32 *)&req, sizeof(req), sizeof(*resp));
	if (IS_ERR(xfer)) {
		ret = PTR_ERR(xfer);
		dev_err(info->dev, "Message alloc failed(%d)\n", ret);
		return ret;
	}

	req.fwl_id = owner->fwl_id;
	req.region = owner->region;
	req.owner_index = owner->owner_index;

	ret = ti_sci_do_xfer(info, xfer);
	if (ret) {
		dev_err(info->dev, "Mbox send fail %d\n", ret);
		return ret;
	}

	resp = (struct ti_sci_msg_fwl_change_owner_info_resp *)xfer->tx_message.buf;

	if (!ti_sci_is_response_ack(resp))
		return -ENODEV;

	owner->fwl_id = resp->fwl_id;
	owner->region = resp->region;
	owner->owner_index = resp->owner_index;
	owner->owner_privid = resp->owner_privid;
	owner->owner_permission_bits = resp->owner_permission_bits;

	return ret;
}

/*
 * ti_sci_setup_ops() - Setup the operations structures
 * @info:	pointer to TISCI pointer
 */
static void ti_sci_setup_ops(struct ti_sci_info *info)
{
	struct ti_sci_ops *ops = &info->handle.ops;
	struct ti_sci_board_ops *bops = &ops->board_ops;
	struct ti_sci_dev_ops *dops = &ops->dev_ops;
	struct ti_sci_clk_ops *cops = &ops->clk_ops;
	struct ti_sci_core_ops *core_ops = &ops->core_ops;
	struct ti_sci_rm_core_ops *rm_core_ops = &ops->rm_core_ops;
	struct ti_sci_proc_ops *pops = &ops->proc_ops;
	struct ti_sci_rm_ringacc_ops *rops = &ops->rm_ring_ops;
	struct ti_sci_rm_psil_ops *psilops = &ops->rm_psil_ops;
	struct ti_sci_rm_udmap_ops *udmap_ops = &ops->rm_udmap_ops;
	struct ti_sci_fwl_ops *fwl_ops = &ops->fwl_ops;

	bops->board_config = ti_sci_cmd_set_board_config;
	bops->board_config_rm = ti_sci_cmd_set_board_config_rm;
	bops->board_config_security = ti_sci_cmd_set_board_config_security;
	bops->board_config_pm = ti_sci_cmd_set_board_config_pm;

	dops->get_device = ti_sci_cmd_get_device;
	dops->idle_device = ti_sci_cmd_idle_device;
	dops->put_device = ti_sci_cmd_put_device;
	dops->is_valid = ti_sci_cmd_dev_is_valid;
	dops->get_context_loss_count = ti_sci_cmd_dev_get_clcnt;
	dops->is_idle = ti_sci_cmd_dev_is_idle;
	dops->is_stop = ti_sci_cmd_dev_is_stop;
	dops->is_on = ti_sci_cmd_dev_is_on;
	dops->is_transitioning = ti_sci_cmd_dev_is_trans;
	dops->set_device_resets = ti_sci_cmd_set_device_resets;
	dops->get_device_resets = ti_sci_cmd_get_device_resets;

	cops->get_clock = ti_sci_cmd_get_clock;
	cops->idle_clock = ti_sci_cmd_idle_clock;
	cops->put_clock = ti_sci_cmd_put_clock;
	cops->is_auto = ti_sci_cmd_clk_is_auto;
	cops->is_on = ti_sci_cmd_clk_is_on;
	cops->is_off = ti_sci_cmd_clk_is_off;

	cops->set_parent = ti_sci_cmd_clk_set_parent;
	cops->get_parent = ti_sci_cmd_clk_get_parent;
	cops->get_num_parents = ti_sci_cmd_clk_get_num_parents;

	cops->get_best_match_freq = ti_sci_cmd_clk_get_match_freq;
	cops->set_freq = ti_sci_cmd_clk_set_freq;
	cops->get_freq = ti_sci_cmd_clk_get_freq;

	core_ops->reboot_device = ti_sci_cmd_core_reboot;
	core_ops->query_msmc = ti_sci_cmd_query_msmc;

	rm_core_ops->get_range = ti_sci_cmd_get_resource_range;
	rm_core_ops->get_range_from_shost =
		ti_sci_cmd_get_resource_range_from_shost;

	pops->proc_request = ti_sci_cmd_proc_request;
	pops->proc_release = ti_sci_cmd_proc_release;
	pops->proc_handover = ti_sci_cmd_proc_handover;
	pops->set_proc_boot_cfg = ti_sci_cmd_set_proc_boot_cfg;
	pops->set_proc_boot_ctrl = ti_sci_cmd_set_proc_boot_ctrl;
	pops->proc_auth_boot_image = ti_sci_cmd_proc_auth_boot_image;
	pops->get_proc_boot_status = ti_sci_cmd_get_proc_boot_status;

	rops->config = ti_sci_cmd_ring_config;
	rops->get_config = ti_sci_cmd_ring_get_config;

	psilops->pair = ti_sci_cmd_rm_psil_pair;
	psilops->unpair = ti_sci_cmd_rm_psil_unpair;

	udmap_ops->tx_ch_cfg = ti_sci_cmd_rm_udmap_tx_ch_cfg;
	udmap_ops->rx_ch_cfg = ti_sci_cmd_rm_udmap_rx_ch_cfg;
	udmap_ops->rx_flow_cfg = ti_sci_cmd_rm_udmap_rx_flow_cfg;

	fwl_ops->set_fwl_region = ti_sci_cmd_set_fwl_region;
	fwl_ops->get_fwl_region = ti_sci_cmd_get_fwl_region;
	fwl_ops->change_fwl_owner = ti_sci_cmd_change_fwl_owner;
}

/**
 * ti_sci_get_handle_from_sysfw() - Get the TI SCI handle of the SYSFW
 * @dev:	Pointer to the SYSFW device
 *
 * Return: pointer to handle if successful, else EINVAL if invalid conditions
 *         are encountered.
 */
const
struct ti_sci_handle *ti_sci_get_handle_from_sysfw(struct udevice *sci_dev)
{
	if (!sci_dev)
		return ERR_PTR(-EINVAL);

	struct ti_sci_info *info = dev_get_priv(sci_dev);

	if (!info)
		return ERR_PTR(-EINVAL);

	struct ti_sci_handle *handle = &info->handle;

	if (!handle)
		return ERR_PTR(-EINVAL);

	return handle;
}

/**
 * ti_sci_get_handle() - Get the TI SCI handle for a device
 * @dev:	Pointer to device for which we want SCI handle
 *
 * Return: pointer to handle if successful, else EINVAL if invalid conditions
 *         are encountered.
 */
const struct ti_sci_handle *ti_sci_get_handle(struct udevice *dev)
{
	if (!dev)
		return ERR_PTR(-EINVAL);

	struct udevice *sci_dev = dev_get_parent(dev);

	return ti_sci_get_handle_from_sysfw(sci_dev);
}

/**
 * ti_sci_get_by_phandle() - Get the TI SCI handle using DT phandle
 * @dev:	device node
 * @propname:	property name containing phandle on TISCI node
 *
 * Return: pointer to handle if successful, else appropriate error value.
 */
const struct ti_sci_handle *ti_sci_get_by_phandle(struct udevice *dev,
						  const char *property)
{
	struct ti_sci_info *entry, *info = NULL;
	u32 phandle, err;
	ofnode node;

	err = ofnode_read_u32(dev_ofnode(dev), property, &phandle);
	if (err)
		return ERR_PTR(err);

	node = ofnode_get_by_phandle(phandle);
	if (!ofnode_valid(node))
		return ERR_PTR(-EINVAL);

	list_for_each_entry(entry, &ti_sci_list, list)
		if (ofnode_equal(dev_ofnode(entry->dev), node)) {
			info = entry;
			break;
		}

	if (!info)
		return ERR_PTR(-ENODEV);

	return &info->handle;
}

/**
 * ti_sci_of_to_info() - generate private data from device tree
 * @dev:	corresponding system controller interface device
 * @info:	pointer to driver specific private data
 *
 * Return: 0 if all goes good, else appropriate error message.
 */
static int ti_sci_of_to_info(struct udevice *dev, struct ti_sci_info *info)
{
	int ret;

	ret = mbox_get_by_name(dev, "tx", &info->chan_tx);
	if (ret) {
		dev_err(dev, "%s: Acquiring Tx channel failed. ret = %d\n",
			__func__, ret);
		return ret;
	}

	ret = mbox_get_by_name(dev, "rx", &info->chan_rx);
	if (ret) {
		dev_err(dev, "%s: Acquiring Rx channel failed. ret = %d\n",
			__func__, ret);
		return ret;
	}

	/* Notify channel is optional. Enable only if populated */
	ret = mbox_get_by_name(dev, "notify", &info->chan_notify);
	if (ret) {
		dev_dbg(dev, "%s: Acquiring notify channel failed. ret = %d\n",
			__func__, ret);
	}

	info->host_id = dev_read_u32_default(dev, "ti,host-id",
					     info->desc->default_host_id);

	info->is_secure = dev_read_bool(dev, "ti,secure-host");

	return 0;
}

/**
 * ti_sci_probe() - Basic probe
 * @dev:	corresponding system controller interface device
 *
 * Return: 0 if all goes good, else appropriate error message.
 */
static int ti_sci_probe(struct udevice *dev)
{
	struct ti_sci_info *info;
	int ret;

	debug("%s(dev=%p)\n", __func__, dev);

	info = dev_get_priv(dev);
	info->desc = (void *)dev_get_driver_data(dev);

	ret = ti_sci_of_to_info(dev, info);
	if (ret) {
		dev_err(dev, "%s: Probe failed with error %d\n", __func__, ret);
		return ret;
	}

	info->dev = dev;
	info->seq = 0xA;

	list_add_tail(&info->list, &ti_sci_list);
	ti_sci_setup_ops(info);

	ret = ti_sci_cmd_get_revision(&info->handle);

	return ret;
}

/*
 * ti_sci_get_free_resource() - Get a free resource from TISCI resource.
 * @res:	Pointer to the TISCI resource
 *
 * Return: resource num if all went ok else TI_SCI_RESOURCE_NULL.
 */
u16 ti_sci_get_free_resource(struct ti_sci_resource *res)
{
	u16 set, free_bit;

	for (set = 0; set < res->sets; set++) {
		free_bit = find_first_zero_bit(res->desc[set].res_map,
					       res->desc[set].num);
		if (free_bit != res->desc[set].num) {
			set_bit(free_bit, res->desc[set].res_map);
			return res->desc[set].start + free_bit;
		}
	}

	return TI_SCI_RESOURCE_NULL;
}

/**
 * ti_sci_release_resource() - Release a resource from TISCI resource.
 * @res:	Pointer to the TISCI resource
 */
void ti_sci_release_resource(struct ti_sci_resource *res, u16 id)
{
	u16 set;

	for (set = 0; set < res->sets; set++) {
		if (res->desc[set].start <= id &&
		    (res->desc[set].num + res->desc[set].start) > id)
			clear_bit(id - res->desc[set].start,
				  res->desc[set].res_map);
	}
}

/**
 * devm_ti_sci_get_of_resource() - Get a TISCI resource assigned to a device
 * @handle:	TISCI handle
 * @dev:	Device pointer to which the resource is assigned
 * @of_prop:	property name by which the resource are represented
 *
 * Note: This function expects of_prop to be in the form of tuples
 *	<type, subtype>. Allocates and initializes ti_sci_resource structure
 *	for each of_prop. Client driver can directly call
 *	ti_sci_(get_free, release)_resource apis for handling the resource.
 *
 * Return: Pointer to ti_sci_resource if all went well else appropriate
 *	   error pointer.
 */
struct ti_sci_resource *
devm_ti_sci_get_of_resource(const struct ti_sci_handle *handle,
			    struct udevice *dev, u32 dev_id, char *of_prop)
{
	u32 resource_subtype;
	u16 resource_type;
	struct ti_sci_resource *res;
	int sets, i, ret;
	u32 *temp;

	res = devm_kzalloc(dev, sizeof(*res), GFP_KERNEL);
	if (!res)
		return ERR_PTR(-ENOMEM);

	sets = dev_read_size(dev, of_prop);
	if (sets < 0) {
		dev_err(dev, "%s resource type ids not available\n", of_prop);
		return ERR_PTR(sets);
	}
	temp = malloc(sets);
	sets /= sizeof(u32);
	res->sets = sets;

	res->desc = devm_kcalloc(dev, res->sets, sizeof(*res->desc),
				 GFP_KERNEL);
	if (!res->desc)
		return ERR_PTR(-ENOMEM);

	ret = ti_sci_get_resource_type(handle_to_ti_sci_info(handle), dev_id,
				       &resource_type);
	if (ret) {
		dev_err(dev, "No valid resource type for %u\n", dev_id);
		return ERR_PTR(-EINVAL);
	}

	ret = dev_read_u32_array(dev, of_prop, temp, res->sets);
	if (ret)
		return ERR_PTR(-EINVAL);

	for (i = 0; i < res->sets; i++) {
		resource_subtype = temp[i];
		ret = handle->ops.rm_core_ops.get_range(handle, dev_id,
							resource_subtype,
							&res->desc[i].start,
							&res->desc[i].num);
		if (ret) {
			dev_err(dev, "type %d subtype %d not allocated for host %d\n",
				resource_type, resource_subtype,
				handle_to_ti_sci_info(handle)->host_id);
			return ERR_PTR(ret);
		}

		dev_dbg(dev, "res type = %d, subtype = %d, start = %d, num = %d\n",
			resource_type, resource_subtype, res->desc[i].start,
			res->desc[i].num);

		res->desc[i].res_map =
			devm_kzalloc(dev, BITS_TO_LONGS(res->desc[i].num) *
				     sizeof(*res->desc[i].res_map), GFP_KERNEL);
		if (!res->desc[i].res_map)
			return ERR_PTR(-ENOMEM);
	}

	return res;
}

/* Description for K2G */
static const struct ti_sci_desc ti_sci_pmmc_k2g_desc = {
	.default_host_id = 2,
	/* Conservative duration */
	.max_rx_timeout_ms = 10000,
	/* Limited by MBOX_TX_QUEUE_LEN. K2G can handle upto 128 messages! */
	.max_msgs = 20,
	.max_msg_size = 64,
	.rm_type_map = NULL,
};

static struct ti_sci_rm_type_map ti_sci_am654_rm_type_map[] = {
	{.dev_id = 56, .type = 0x00b}, /* GIC_IRQ */
	{.dev_id = 179, .type = 0x000}, /* MAIN_NAV_UDMASS_IA0 */
	{.dev_id = 187, .type = 0x009}, /* MAIN_NAV_RA */
	{.dev_id = 188, .type = 0x006}, /* MAIN_NAV_UDMAP */
	{.dev_id = 194, .type = 0x007}, /* MCU_NAV_UDMAP */
	{.dev_id = 195, .type = 0x00a}, /* MCU_NAV_RA */
	{.dev_id = 0, .type = 0x000}, /* end of table */
};

/* Description for AM654 */
static const struct ti_sci_desc ti_sci_pmmc_am654_desc = {
	.default_host_id = 12,
	/* Conservative duration */
	.max_rx_timeout_ms = 10000,
	/* Limited by MBOX_TX_QUEUE_LEN. K2G can handle upto 128 messages! */
	.max_msgs = 20,
	.max_msg_size = 60,
	.rm_type_map = ti_sci_am654_rm_type_map,
};

static const struct udevice_id ti_sci_ids[] = {
	{
		.compatible = "ti,k2g-sci",
		.data = (ulong)&ti_sci_pmmc_k2g_desc
	},
	{
		.compatible = "ti,am654-sci",
		.data = (ulong)&ti_sci_pmmc_am654_desc
	},
	{ /* Sentinel */ },
};

U_BOOT_DRIVER(ti_sci) = {
	.name = "ti_sci",
	.id = UCLASS_FIRMWARE,
	.of_match = ti_sci_ids,
	.probe = ti_sci_probe,
	.priv_auto_alloc_size = sizeof(struct ti_sci_info),
};
