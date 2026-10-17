/* SPDX-License-Identifier: GPL-2.0 */
/*
 * MIPI DSI Bus
 *
 * Copyright (C) 2012-2013, Samsung Electronics, Co., Ltd.
 * Copyright (C) 2018 STMicroelectronics - All Rights Reserved
 * Author(s): Andrzej Hajda <a.hajda@samsung.com>
 *            Yannick Fertre <yannick.fertre@st.com>
 *            Philippe Cornu <philippe.cornu@st.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef MIPI_DSI_H
#define MIPI_DSI_H

#include <mipi_display.h>
#include <linux/bitops.h>

struct mipi_dsi_host;
struct mipi_dsi_device;

/* request ACK from peripheral */
#define MIPI_DSI_MSG_REQ_ACK	BIT(0)
/* use Low Power Mode to transmit message */
#define MIPI_DSI_MSG_USE_LPM	BIT(1)

/**
 * struct mipi_dsi_msg - read/write DSI buffer
 * @channel: virtual channel id
 * @type: payload data type
 * @flags: flags controlling this message transmission
 * @tx_len: length of @tx_buf
 * @tx_buf: data to be written
 * @rx_len: length of @rx_buf
 * @rx_buf: data to be read, or NULL
 */
struct mipi_dsi_msg {
	u8 channel;
	u8 type;
	u16 flags;

	size_t tx_len;
	const void *tx_buf;

	size_t rx_len;
	void *rx_buf;
};

bool mipi_dsi_packet_format_is_short(u8 type);
bool mipi_dsi_packet_format_is_long(u8 type);

/**
 * struct mipi_dsi_packet - represents a MIPI DSI packet in protocol format
 * @size: size (in bytes) of the packet
 * @header: the four bytes that make up the header (Data ID, Word Count or
 *     Packet Data, and ECC)
 * @payload_length: number of bytes in the payload
 * @payload: a pointer to a buffer containing the payload, if any
 */
struct mipi_dsi_packet {
	size_t size;
	u8 header[4];
	size_t payload_length;
	const u8 *payload;
};

int mipi_dsi_create_packet(struct mipi_dsi_packet *packet,
			   const struct mipi_dsi_msg *msg);

/**
 * struct mipi_dsi_host_ops - DSI bus operations
 * @attach: attach DSI device to DSI host
 * @detach: detach DSI device from DSI host
 * @transfer: transmit a DSI packet
 *
 * DSI packets transmitted by .transfer() are passed in as mipi_dsi_msg
 * structures. This structure contains information about the type of packet
 * being transmitted as well as the transmit and receive buffers. When an
 * error is encountered during transmission, this function will return a
 * negative error code. On success it shall return the number of bytes
 * transmitted for write packets or the number of bytes received for read
 * packets.
 *
 * Note that typically DSI packet transmission is atomic, so the .transfer()
 * function will seldomly return anything other than the number of bytes
 * contained in the transmit buffer on success.
 */
struct mipi_dsi_host_ops {
	int (*attach)(struct mipi_dsi_host *host,
		      struct mipi_dsi_device *dsi);
	int (*detach)(struct mipi_dsi_host *host,
		      struct mipi_dsi_device *dsi);
	ssize_t (*transfer)(struct mipi_dsi_host *host,
			    const struct mipi_dsi_msg *msg);
};

/**
 * struct mipi_dsi_phy_timing - DSI host phy timings
 * @data_hs2lp: High Speed to Low Speed Data Transition Time
 * @data_lp2hs: Low Speed to High Speed Data Transition Time
 * @clk_hs2lp: High Speed to Low Speed Clock Transition Time
 * @clk_lp2hs: Low Speed to High Speed Clock Transition Time
 */
struct mipi_dsi_phy_timing {
	u16 data_hs2lp;
	u16 data_lp2hs;
	u16 clk_hs2lp;
	u16 clk_lp2hs;
};

/**
 * struct mipi_dsi_phy_ops - DSI host physical operations
 * @init: initialized host physical part
 * @get_lane_mbps: get lane bitrate per lane (mbps)
 * @post_set_mode: operation that should after set mode
 */
struct mipi_dsi_phy_ops {
	int (*init)(void *priv_data);
	int (*get_lane_mbps)(void *priv_data, struct display_timing *timings,
			     u32 lanes, u32 format, unsigned int *lane_mbps);
	void (*post_set_mode)(void *priv_data,  unsigned long mode_flags);
	int (*get_timing)(void *priv_data, unsigned int lane_mbps,
			  struct mipi_dsi_phy_timing *timing);
	void (*get_esc_clk_rate)(void *priv_data, unsigned int *esc_clk_rate);
};

/**
 * struct mipi_dsi_host - DSI host device
 * @dev: driver model device node for this DSI host
 * @ops: DSI host operations
 * @list: list management
 */
struct mipi_dsi_host {
	struct device *dev;
	const struct mipi_dsi_host_ops *ops;
	struct list_head list;
};

/* DSI mode flags */

/* video mode */
#define MIPI_DSI_MODE_VIDEO		BIT(0)
/* video burst mode */
#define MIPI_DSI_MODE_VIDEO_BURST	BIT(1)
/* video pulse mode */
#define MIPI_DSI_MODE_VIDEO_SYNC_PULSE	BIT(2)
/* enable auto vertical count mode */
#define MIPI_DSI_MODE_VIDEO_AUTO_VERT	BIT(3)
/* enable hsync-end packets in vsync-pulse and v-porch area */
#define MIPI_DSI_MODE_VIDEO_HSE		BIT(4)
/* disable hfront-porch area */
#define MIPI_DSI_MODE_VIDEO_HFP		BIT(5)
/* disable hback-porch area */
#define MIPI_DSI_MODE_VIDEO_HBP		BIT(6)
/* disable hsync-active area */
#define MIPI_DSI_MODE_VIDEO_HSA		BIT(7)
/* flush display FIFO on vsync pulse */
#define MIPI_DSI_MODE_VSYNC_FLUSH	BIT(8)
/* disable EoT packets in HS mode */
#define MIPI_DSI_MODE_EOT_PACKET	BIT(9)
/* device supports non-continuous clock behavior (DSI spec 5.6.1) */
#define MIPI_DSI_CLOCK_NON_CONTINUOUS	BIT(10)
/* transmit data in low power */
#define MIPI_DSI_MODE_LPM		BIT(11)

enum mipi_dsi_pixel_format {
	MIPI_DSI_FMT_RGB888,
	MIPI_DSI_FMT_RGB666,
	MIPI_DSI_FMT_RGB666_PACKED,
	MIPI_DSI_FMT_RGB565,
};

#define DSI_DEV_NAME_SIZE		20

/**
 * struct mipi_dsi_device_info - template for creating a mipi_dsi_device
 * @type: DSI peripheral chip type
 * @channel: DSI virtual channel assigned to peripheral
 * @node: pointer to OF device node or NULL
 *
 * This is populated and passed to mipi_dsi_device_new to create a new
 * DSI device
 */
struct mipi_dsi_device_info {
	char type[DSI_DEV_NAME_SIZE];
	u32 channel;
	struct device_node *node;
};

/**
 * struct mipi_dsi_device - DSI peripheral device
 * @host: DSI host for this peripheral
 * @dev: driver model device node for this peripheral
 * @name: DSI peripheral chip type
 * @channel: virtual channel assigned to the peripheral
 * @format: pixel format for video mode
 * @lanes: number of active data lanes
 * @mode_flags: DSI operation mode related flags
 */
struct mipi_dsi_device {
	struct mipi_dsi_host *host;
	struct udevice *dev;

	char name[DSI_DEV_NAME_SIZE];
	unsigned int channel;
	unsigned int lanes;
	enum mipi_dsi_pixel_format format;
	unsigned long mode_flags;
};

/**
 * mipi_dsi_pixel_format_to_bpp - obtain the number of bits per pixel for any
 *                                given pixel format defined by the MIPI DSI
 *                                specification
 * @fmt: MIPI DSI pixel format
 *
 * Returns: The number of bits per pixel of the given pixel format.
 */
static inline int mipi_dsi_pixel_format_to_bpp(enum mipi_dsi_pixel_format fmt)
{
	switch (fmt) {
	case MIPI_DSI_FMT_RGB888:
	case MIPI_DSI_FMT_RGB666:
		return 24;

	case MIPI_DSI_FMT_RGB666_PACKED:
		return 18;

	case MIPI_DSI_FMT_RGB565:
		return 16;
	}

	return -EINVAL;
}

/**
 * struct mipi_dsi_panel_plat - DSI panel platform data
 * @device: DSI peripheral device
 * @lanes: number of active data lanes
 * @format: pixel format for video mode
 * @mode_flags: DSI operation mode related flags
 */
struct mipi_dsi_panel_plat {
	struct mipi_dsi_device *device;
	unsigned int lanes;
	enum mipi_dsi_pixel_format format;
	unsigned long mode_flags;
};

/**
 * mipi_dsi_attach - attach a DSI device to its DSI host
 * @dsi: DSI peripheral
 */
int mipi_dsi_attach(struct mipi_dsi_device *dsi);

/**
 * mipi_dsi_detach - detach a DSI device from its DSI host
 * @dsi: DSI peripheral
 */
int mipi_dsi_detach(struct mipi_dsi_device *dsi);
int mipi_dsi_shutdown_peripheral(struct mipi_dsi_device *dsi);
int mipi_dsi_turn_on_peripheral(struct mipi_dsi_device *dsi);
int mipi_dsi_set_maximum_return_packet_size(struct mipi_dsi_device *dsi,
					    u16 value);

ssize_t mipi_dsi_generic_write(struct mipi_dsi_device *dsi, const void *payload,
			       size_t size);
ssize_t mipi_dsi_generic_read(struct mipi_dsi_device *dsi, const void *params,
			      size_t num_params, void *data, size_t size);

/**
 * enum mipi_dsi_dcs_tear_mode - Tearing Effect Output Line mode
 * @MIPI_DSI_DCS_TEAR_MODE_VBLANK: the TE output line consists of V-Blanking
 *    information only
 * @MIPI_DSI_DCS_TEAR_MODE_VHBLANK : the TE output line consists of both
 *    V-Blanking and H-Blanking information
 */
enum mipi_dsi_dcs_tear_mode {
	MIPI_DSI_DCS_TEAR_MODE_VBLANK,
	MIPI_DSI_DCS_TEAR_MODE_VHBLANK,
};

#define MIPI_DSI_DCS_POWER_MODE_DISPLAY BIT(2)
#define MIPI_DSI_DCS_POWER_MODE_NORMAL  BIT(3)
#define MIPI_DSI_DCS_POWER_MODE_SLEEP   BIT(4)
#define MIPI_DSI_DCS_POWER_MODE_PARTIAL BIT(5)
#define MIPI_DSI_DCS_POWER_MODE_IDLE    BIT(6)

/**
 * mipi_dsi_dcs_write_buffer() - transmit a DCS command with payload
 * @dsi: DSI peripheral device
 * @data: buffer containing data to be transmitted
 * @len: size of transmission buffer
 *
 * This function will automatically choose the right data type depending on
 * the command payload length.
 *
 * Return: The number of bytes successfully transmitted or a negative error
 * code on failure.
 */
ssize_t mipi_dsi_dcs_write_buffer(struct mipi_dsi_device *dsi,
				  const void *data, size_t len);

/**
 * mipi_dsi_dcs_write() - send DCS write command
 * @dsi: DSI peripheral device
 * @cmd: DCS command
 * @data: buffer containing the command payload
 * @len: command payload length
 *
 * This function will automatically choose the right data type depending on
 * the command payload length.

 * code on failure.
 */
ssize_t mipi_dsi_dcs_write(struct mipi_dsi_device *dsi, u8 cmd,
			   const void *data, size_t len);

/**
 * mipi_dsi_dcs_read() - send DCS read request command
 * @dsi: DSI peripheral device
 * @cmd: DCS command
 * @data: buffer in which to receive data
 * @len: size of receive buffer
 *
 * Return: The number of bytes read or a negative error code on failure.
 */
ssize_t mipi_dsi_dcs_read(struct mipi_dsi_device *dsi, u8 cmd, void *data,
			  size_t len);

/**
 * mipi_dsi_dcs_nop() - send DCS nop packet
 * @dsi: DSI peripheral device
 *
 * Return: 0 on success or a negative error code on failure.
 */
int mipi_dsi_dcs_nop(struct mipi_dsi_device *dsi);

/**
 * mipi_dsi_dcs_soft_reset() - perform a software reset of the display module
 * @dsi: DSI peripheral device
 *
 * Return: 0 on success or a negative error code on failure.
 */
int mipi_dsi_dcs_soft_reset(struct mipi_dsi_device *dsi);

/**
 * mipi_dsi_dcs_get_power_mode() - query the display module's current power
 *    mode
 * @dsi: DSI peripheral device
 * @mode: return location for the current power mode
 *
 * Return: 0 on success or a negative error code on failure.
 */
int mipi_dsi_dcs_get_power_mode(struct mipi_dsi_device *dsi, u8 *mode);

/**
 * mipi_dsi_dcs_get_pixel_format() - gets the pixel format for the RGB image
 *    data used by the interface
 * @dsi: DSI peripheral device
 * @format: return location for the pixel format
 *
 * Return: 0 on success or a negative error code on failure.
 */
int mipi_dsi_dcs_get_pixel_format(struct mipi_dsi_device *dsi, u8 *format);

/**
 * mipi_dsi_dcs_enter_sleep_mode() - disable all unnecessary blocks inside the
 *    display module except interface communication
 * @dsi: DSI peripheral device
 *
 * Return: 0 on success or a negative error code on failure.
 */
int mipi_dsi_dcs_enter_sleep_mode(struct mipi_dsi_device *dsi);

/**
 * mipi_dsi_dcs_exit_sleep_mode() - enable all blocks inside the display
 *    module
 * @dsi: DSI peripheral device
 *
 * Return: 0 on success or a negative error code on failure.
 */
int mipi_dsi_dcs_exit_sleep_mode(struct mipi_dsi_device *dsi);

/**
 * mipi_dsi_dcs_set_display_off() - stop displaying the image data on the
 *    display device
 * @dsi: DSI peripheral device
 *
 * Return: 0 on success or a negative error code on failure.
 */
int mipi_dsi_dcs_set_display_off(struct mipi_dsi_device *dsi);

/**
 * mipi_dsi_dcs_set_display_on() - start displaying the image data on the
 *    display device
 * @dsi: DSI peripheral device
 *
 * Return: 0 on success or a negative error code on failure
 */
int mipi_dsi_dcs_set_display_on(struct mipi_dsi_device *dsi);

/**
 * mipi_dsi_dcs_set_column_address() - define the column extent of the frame
 *    memory accessed by the host processor
 * @dsi: DSI peripheral device
 * @start: first column of frame memory
 * @end: last column of frame memory
 *
 * Return: 0 on success or a negative error code on failure.
 */
int mipi_dsi_dcs_set_column_address(struct mipi_dsi_device *dsi, u16 start,
				    u16 end);
/**
 * mipi_dsi_dcs_set_page_address() - define the page extent of the frame
 *    memory accessed by the host processor
 * @dsi: DSI peripheral device
 * @start: first page of frame memory
 * @end: last page of frame memory
 *
 * Return: 0 on success or a negative error code on failure.
 */
int mipi_dsi_dcs_set_page_address(struct mipi_dsi_device *dsi, u16 start,
				  u16 end);

/**
 * mipi_dsi_dcs_set_tear_off() - turn off the display module's Tearing Effect
 *    output signal on the TE signal line
 * @dsi: DSI peripheral device
 *
 * Return: 0 on success or a negative error code on failure
 */
int mipi_dsi_dcs_set_tear_off(struct mipi_dsi_device *dsi);

/**
 * mipi_dsi_dcs_set_tear_on() - turn on the display module's Tearing Effect
 *    output signal on the TE signal line.
 * @dsi: DSI peripheral device
 * @mode: the Tearing Effect Output Line mode
 *
 * Return: 0 on success or a negative error code on failure
 */
int mipi_dsi_dcs_set_tear_on(struct mipi_dsi_device *dsi,
			     enum mipi_dsi_dcs_tear_mode mode);

/**
 * mipi_dsi_dcs_set_pixel_format() - sets the pixel format for the RGB image
 *    data used by the interface
 * @dsi: DSI peripheral device
 * @format: pixel format
 *
 * Return: 0 on success or a negative error code on failure.
 */
int mipi_dsi_dcs_set_pixel_format(struct mipi_dsi_device *dsi, u8 format);

/**
 * mipi_dsi_dcs_set_tear_scanline() - set the scanline to use as trigger for
 *    the Tearing Effect output signal of the display module
 * @dsi: DSI peripheral device
 * @scanline: scanline to use as trigger
 *
 * Return: 0 on success or a negative error code on failure
 */
int mipi_dsi_dcs_set_tear_scanline(struct mipi_dsi_device *dsi, u16 scanline);

/**
 * mipi_dsi_dcs_set_display_brightness() - sets the brightness value of the
 *    display
 * @dsi: DSI peripheral device
 * @brightness: brightness value
 *
 * Return: 0 on success or a negative error code on failure.
 */
int mipi_dsi_dcs_set_display_brightness(struct mipi_dsi_device *dsi,
					u16 brightness);

/**
 * mipi_dsi_dcs_get_display_brightness() - gets the current brightness value
 *    of the display
 * @dsi: DSI peripheral device
 * @brightness: brightness value
 *
 * Return: 0 on success or a negative error code on failure.
 */
int mipi_dsi_dcs_get_display_brightness(struct mipi_dsi_device *dsi,
					u16 *brightness);

#endif /* MIPI_DSI_H */
