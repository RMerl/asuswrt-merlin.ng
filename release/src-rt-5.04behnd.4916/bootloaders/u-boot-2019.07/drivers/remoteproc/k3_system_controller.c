// SPDX-License-Identifier: GPL-2.0+
/*
 * Texas Instruments' K3 System Controller Driver
 *
 * Copyright (C) 2017-2018 Texas Instruments Incorporated - http://www.ti.com/
 *	Lokesh Vutla <lokeshvutla@ti.com>
 */

#include <common.h>
#include <dm.h>
#include <remoteproc.h>
#include <errno.h>
#include <mailbox.h>
#include <linux/soc/ti/k3-sec-proxy.h>

#define K3_MSG_R5_TO_M3_M3FW			0x8105
#define K3_MSG_M3_TO_R5_CERT_RESULT		0x8805
#define K3_MSG_M3_TO_R5_BOOT_NOTIFICATION	0x000A

#define K3_FLAGS_MSG_CERT_AUTH_PASS		0x555555
#define K3_FLAGS_MSG_CERT_AUTH_FAIL		0xffffff

/**
 * struct k3_sysctrler_msg_hdr - Generic Header for Messages and responses.
 * @cmd_id:	Message ID. One of K3_MSG_*
 * @host_id:	Host ID of the message
 * @seq_ne:	Message identifier indicating a transfer sequence.
 * @flags:	Flags for the message.
 */
struct k3_sysctrler_msg_hdr {
	u16 cmd_id;
	u8 host_id;
	u8 seq_nr;
	u32 flags;
} __packed;

/**
 * struct k3_sysctrler_load_msg - Message format for Firmware loading
 * @hdr:		Generic message hdr
 * @buffer_address:	Address at which firmware is located.
 * @buffer_size:	Size of the firmware.
 */
struct k3_sysctrler_load_msg {
	struct k3_sysctrler_msg_hdr hdr;
	u32 buffer_address;
	u32 buffer_size;
} __packed;

/**
 * struct k3_sysctrler_boot_notification_msg - Message format for boot
 *					       notification
 * @checksum:		Checksum for the entire message
 * @reserved:		Reserved for future use.
 * @hdr:		Generic message hdr
 */
struct k3_sysctrler_boot_notification_msg {
	u16 checksum;
	u16 reserved;
	struct k3_sysctrler_msg_hdr hdr;
} __packed;

/**
 * struct k3_sysctrler_desc - Description of SoC integration.
 * @host_id:	Host identifier representing the compute entity
 * @max_rx_timeout_ms:	Timeout for communication with SoC (in Milliseconds)
 * @max_msg_size: Maximum size of data per message that can be handled.
 */
struct k3_sysctrler_desc {
	u8 host_id;
	int max_rx_timeout_us;
	int max_msg_size;
};

/**
 * struct k3_sysctrler_privdata - Structure representing System Controller data.
 * @chan_tx:	Transmit mailbox channel
 * @chan_rx:	Receive mailbox channel
 * @desc:	SoC description for this instance
 * @seq_nr:	Counter for number of messages sent.
 */
struct k3_sysctrler_privdata {
	struct mbox_chan chan_tx;
	struct mbox_chan chan_rx;
	struct k3_sysctrler_desc *desc;
	u32 seq_nr;
};

static inline
void k3_sysctrler_load_msg_setup(struct k3_sysctrler_load_msg *fw,
				 struct k3_sysctrler_privdata *priv,
				 ulong addr, ulong size)
{
	fw->hdr.cmd_id = K3_MSG_R5_TO_M3_M3FW;
	fw->hdr.host_id = priv->desc->host_id;
	fw->hdr.seq_nr = priv->seq_nr++;
	fw->hdr.flags = 0x0;
	fw->buffer_address = addr;
	fw->buffer_size = size;
}

static int k3_sysctrler_load_response(u32 *buf)
{
	struct k3_sysctrler_load_msg *fw;

	fw = (struct k3_sysctrler_load_msg *)buf;

	/* Check for proper response ID */
	if (fw->hdr.cmd_id != K3_MSG_M3_TO_R5_CERT_RESULT) {
		dev_err(dev, "%s: Command expected 0x%x, but received 0x%x\n",
			__func__, K3_MSG_M3_TO_R5_CERT_RESULT, fw->hdr.cmd_id);
		return -EINVAL;
	}

	/* Check for certificate authentication result */
	if (fw->hdr.flags == K3_FLAGS_MSG_CERT_AUTH_FAIL) {
		dev_err(dev, "%s: Firmware certificate authentication failed\n",
			__func__);
		return -EINVAL;
	} else if (fw->hdr.flags != K3_FLAGS_MSG_CERT_AUTH_PASS) {
		dev_err(dev, "%s: Firmware Load response Invalid %d\n",
			__func__, fw->hdr.flags);
		return -EINVAL;
	}

	debug("%s: Firmware authentication passed\n", __func__);

	return 0;
}

static int k3_sysctrler_boot_notification_response(u32 *buf)
{
	struct k3_sysctrler_boot_notification_msg *boot;

	boot = (struct k3_sysctrler_boot_notification_msg *)buf;

	/* ToDo: Verify checksum */

	/* Check for proper response ID */
	if (boot->hdr.cmd_id != K3_MSG_M3_TO_R5_BOOT_NOTIFICATION) {
		dev_err(dev, "%s: Command expected 0x%x, but received 0x%x\n",
			__func__, K3_MSG_M3_TO_R5_BOOT_NOTIFICATION,
			boot->hdr.cmd_id);
		return -EINVAL;
	}

	debug("%s: Boot notification received\n", __func__);

	return 0;
}

/**
 * k3_sysctrler_load() - Loadup the K3 remote processor
 * @dev:	corresponding K3 remote processor device
 * @addr:	Address in memory where image binary is stored
 * @size:	Size in bytes of the image binary
 *
 * Return: 0 if all goes good, else appropriate error message.
 */
static int k3_sysctrler_load(struct udevice *dev, ulong addr, ulong size)
{
	struct k3_sysctrler_privdata *priv = dev_get_priv(dev);
	struct k3_sysctrler_load_msg firmware;
	struct k3_sec_proxy_msg msg;
	int ret;

	debug("%s: Loading binary from 0x%08lX, size 0x%08lX\n",
	      __func__, addr, size);

	memset(&firmware, 0, sizeof(firmware));
	memset(&msg, 0, sizeof(msg));

	/* Setup the message */
	k3_sysctrler_load_msg_setup(&firmware, priv, addr, size);
	msg.len = sizeof(firmware);
	msg.buf = (u32 *)&firmware;

	/* Send the message */
	ret = mbox_send(&priv->chan_tx, &msg);
	if (ret) {
		dev_err(dev, "%s: Firmware Loading failed. ret = %d\n",
			__func__, ret);
		return ret;
	}

	/* Receive the response */
	ret = mbox_recv(&priv->chan_rx, &msg, priv->desc->max_rx_timeout_us);
	if (ret) {
		dev_err(dev, "%s: Firmware Load response failed. ret = %d\n",
			__func__, ret);
		return ret;
	}

	/* Process the response */
	ret = k3_sysctrler_load_response(msg.buf);
	if (ret)
		return ret;

	debug("%s: Firmware Loaded successfully on dev %s\n",
	      __func__, dev->name);

	return 0;
}

/**
 * k3_sysctrler_start() - Start the remote processor
 *		Note that while technically the K3 system controller starts up
 *		automatically after its firmware got loaded we still want to
 *		utilize the rproc start operation for other startup-related
 *		tasks.
 * @dev:	device to operate upon
 *
 * Return: 0 if all went ok, else return appropriate error
 */
static int k3_sysctrler_start(struct udevice *dev)
{
	struct k3_sysctrler_privdata *priv = dev_get_priv(dev);
	struct k3_sec_proxy_msg msg;
	int ret;

	debug("%s(dev=%p)\n", __func__, dev);

	/* Receive the boot notification. Note that it is sent only once. */
	ret = mbox_recv(&priv->chan_rx, &msg, priv->desc->max_rx_timeout_us);
	if (ret) {
		dev_err(dev, "%s: Boot Notification response failed. ret = %d\n",
			__func__, ret);
		return ret;
	}

	/* Process the response */
	ret = k3_sysctrler_boot_notification_response(msg.buf);
	if (ret)
		return ret;

	debug("%s: Boot notification received successfully on dev %s\n",
	      __func__, dev->name);

	return 0;
}

static const struct dm_rproc_ops k3_sysctrler_ops = {
	.load = k3_sysctrler_load,
	.start = k3_sysctrler_start,
};

/**
 * k3_of_to_priv() - generate private data from device tree
 * @dev:	corresponding k3 remote processor device
 * @priv:	pointer to driver specific private data
 *
 * Return: 0 if all goes good, else appropriate error message.
 */
static int k3_of_to_priv(struct udevice *dev,
			 struct k3_sysctrler_privdata *priv)
{
	int ret;

	ret = mbox_get_by_name(dev, "tx", &priv->chan_tx);
	if (ret) {
		dev_err(dev, "%s: Acquiring Tx channel failed. ret = %d\n",
			__func__, ret);
		return ret;
	}

	ret = mbox_get_by_name(dev, "rx", &priv->chan_rx);
	if (ret) {
		dev_err(dev, "%s: Acquiring Rx channel failed. ret = %d\n",
			__func__, ret);
		return ret;
	}

	return 0;
}

/**
 * k3_sysctrler_probe() - Basic probe
 * @dev:	corresponding k3 remote processor device
 *
 * Return: 0 if all goes good, else appropriate error message.
 */
static int k3_sysctrler_probe(struct udevice *dev)
{
	struct k3_sysctrler_privdata *priv;
	int ret;

	debug("%s(dev=%p)\n", __func__, dev);

	priv = dev_get_priv(dev);

	ret = k3_of_to_priv(dev, priv);
	if (ret) {
		dev_err(dev, "%s: Probe failed with error %d\n", __func__, ret);
		return ret;
	}

	priv->desc = (void *)dev_get_driver_data(dev);
	priv->seq_nr = 0;

	return 0;
}

static const struct k3_sysctrler_desc k3_sysctrler_am654_desc = {
	.host_id = 4,				/* HOST_ID_R5_1 */
	.max_rx_timeout_us = 800000,
	.max_msg_size = 60,
};

static const struct udevice_id k3_sysctrler_ids[] = {
	{
		.compatible = "ti,am654-system-controller",
		.data = (ulong)&k3_sysctrler_am654_desc,
	},
	{}
};

U_BOOT_DRIVER(k3_sysctrler) = {
	.name = "k3_system_controller",
	.of_match = k3_sysctrler_ids,
	.id = UCLASS_REMOTEPROC,
	.ops = &k3_sysctrler_ops,
	.probe = k3_sysctrler_probe,
	.priv_auto_alloc_size = sizeof(struct k3_sysctrler_privdata),
};
