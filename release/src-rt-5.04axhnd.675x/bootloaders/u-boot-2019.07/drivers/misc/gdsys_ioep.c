// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2017
 * Mario Six,  Guntermann & Drunck GmbH, mario.six@gdsys.cc
 *
 * based on the cmd_ioloop driver/command, which is
 *
 * (C) Copyright 2014
 * Dirk Eibach, Guntermann & Drunck GmbH, dirk.eibach@gdsys.cc
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <misc.h>
#include <regmap.h>

#include "gdsys_ioep.h"

/**
 * struct gdsys_ioep_priv - Private data structure for IOEP devices
 * @map:   Register map to be used for the device
 * @state: Flag to keep the current status of the RX control (enabled/disabled)
 */
struct gdsys_ioep_priv {
	struct regmap *map;
	bool state;
};

/**
 * enum last_spec - Convenience enum for read data sanity check
 * @READ_DATA_IS_LAST:     The data to be read should be the final data of the
 *			   current packet
 * @READ_DATA_IS_NOT_LAST: The data to be read should not be the final data of
 *			   the current packet
 */
enum last_spec {
	READ_DATA_IS_LAST,
	READ_DATA_IS_NOT_LAST,
};

static int gdsys_ioep_set_receive(struct udevice *dev, bool val)
{
	struct gdsys_ioep_priv *priv = dev_get_priv(dev);
	u16 state;

	priv->state = !priv->state;

	if (val)
		state = CTRL_PROC_RECEIVE_ENABLE;
	else
		state = ~CTRL_PROC_RECEIVE_ENABLE;

	gdsys_ioep_set(priv->map, tx_control, state);

	if (val) {
		/* Set device address to dummy 1 */
		gdsys_ioep_set(priv->map, device_address, 1);
	}

	return !priv->state;
}

static int gdsys_ioep_send(struct udevice *dev, int offset,
			   const void *buf, int size)
{
	struct gdsys_ioep_priv *priv = dev_get_priv(dev);
	int k;
	u16 *p = (u16 *)buf;

	for (k = 0; k < size; ++k)
		gdsys_ioep_set(priv->map, transmit_data, *(p++));

	gdsys_ioep_set(priv->map, tx_control, CTRL_PROC_RECEIVE_ENABLE |
					      CTRL_FLUSH_TRANSMIT_BUFFER);

	return 0;
}

/**
 * receive_byte_buffer() - Read data from a IOEP device
 * @dev:       The IOEP device to read data from
 * @len:       The length of the data to read
 * @buffer:    The buffer to read the data into
 * @last_spec: Flag to indicate if the data to be read in this call should be
 *	       the final data of the current packet (i.e. it should be empty
 *	       after this read)
 *
 * Return: 0 if OK, -ve on error
 */
static int receive_byte_buffer(struct udevice *dev, uint len,
			       u16 *buffer, enum last_spec last_spec)
{
	struct gdsys_ioep_priv *priv = dev_get_priv(dev);
	int k;
	int ret = -EIO;

	for (k = 0; k < len; ++k) {
		u16 rx_tx_status;

		gdsys_ioep_get(priv->map, receive_data, buffer++);

		gdsys_ioep_get(priv->map, rx_tx_status, &rx_tx_status);
		/*
		 * Sanity check: If the data read should have been the last,
		 * but wasn't, something is wrong
		 */
		if (k == (len - 1) && (last_spec == READ_DATA_IS_NOT_LAST ||
				       rx_tx_status & STATE_RX_DATA_LAST))
			ret = 0;
	}

	if (ret)
		debug("%s: Error while receiving bufer (err = %d)\n",
		      dev->name, ret);

	return ret;
}

static int gdsys_ioep_receive(struct udevice *dev, int offset, void *buf,
			      int size)
{
	int ret;
	struct io_generic_packet header;
	u16 *p = (u16 *)buf;
	const int header_words = sizeof(struct io_generic_packet) / sizeof(u16);
	uint len;

	/* Read the packet header */
	ret = receive_byte_buffer(dev, header_words, p, READ_DATA_IS_NOT_LAST);
	if (ret) {
		debug("%s: Failed to read header data (err = %d)\n",
		      dev->name, ret);
		return ret;
	}

	memcpy(&header, p, header_words * sizeof(u16));
	p += header_words;

	/* Get payload data length */
	len = (header.packet_length + 1) / sizeof(u16);

	/* Read the packet payload */
	ret = receive_byte_buffer(dev, len, p, READ_DATA_IS_LAST);
	if (ret) {
		debug("%s: Failed to read payload data (err = %d)\n",
		      dev->name, ret);
		return ret;
	}

	return 0;
}

static int gdsys_ioep_get_and_reset_status(struct udevice *dev, int msgid,
					   void *tx_msg, int tx_size,
					   void *rx_msg, int rx_size)
{
	struct gdsys_ioep_priv *priv = dev_get_priv(dev);
	const u16 mask = STATE_RX_DIST_ERR | STATE_RX_LENGTH_ERR |
			 STATE_RX_FRAME_CTR_ERR | STATE_RX_FCS_ERR |
			 STATE_RX_PACKET_DROPPED | STATE_TX_ERR;
	u16 *status = rx_msg;

	gdsys_ioep_get(priv->map, rx_tx_status, status);

	gdsys_ioep_set(priv->map, rx_tx_status, *status);

	return (*status & mask) ? 1 : 0;
}

static const struct misc_ops gdsys_ioep_ops = {
	.set_enabled = gdsys_ioep_set_receive,
	.write = gdsys_ioep_send,
	.read = gdsys_ioep_receive,
	.call = gdsys_ioep_get_and_reset_status,
};

static int gdsys_ioep_probe(struct udevice *dev)
{
	struct gdsys_ioep_priv *priv = dev_get_priv(dev);
	int ret;

	ret = regmap_init_mem(dev_ofnode(dev), &priv->map);
	if (ret) {
		debug("%s: Could not initialize regmap (err = %d)",
		      dev->name, ret);
		return ret;
	}

	priv->state = false;

	return 0;
}

static const struct udevice_id gdsys_ioep_ids[] = {
	{ .compatible = "gdsys,io-endpoint" },
	{ }
};

U_BOOT_DRIVER(gdsys_ioep) = {
	.name           = "gdsys_ioep",
	.id             = UCLASS_MISC,
	.ops		= &gdsys_ioep_ops,
	.flags		= DM_UC_FLAG_SEQ_ALIAS,
	.of_match       = gdsys_ioep_ids,
	.probe          = gdsys_ioep_probe,
	.priv_auto_alloc_size = sizeof(struct gdsys_ioep_priv),
};
