/* SPDX-License-Identifier: GPL-2.0+
 *
 * Defines for Mobile Industry Processor Interface (MIPI(R))
 * Display Working Group standards: DSI, DCS, DBI, DPI
 *
 * Copyright (C) 2010 Guennadi Liakhovetski <g.liakhovetski@gmx.de>
 * Copyright (C) 2006 Nokia Corporation
 * Author: Imre Deak <imre.deak@nokia.com>
 */

#ifndef MIPI_DISPLAY_H
#define MIPI_DISPLAY_H

/* MIPI DSI Processor-to-Peripheral transaction types */
enum {
	MIPI_DSI_V_SYNC_START				= 0x01,
	MIPI_DSI_V_SYNC_END				= 0x11,
	MIPI_DSI_H_SYNC_START				= 0x21,
	MIPI_DSI_H_SYNC_END				= 0x31,

	MIPI_DSI_COLOR_MODE_OFF				= 0x02,
	MIPI_DSI_COLOR_MODE_ON				= 0x12,
	MIPI_DSI_SHUTDOWN_PERIPHERAL			= 0x22,
	MIPI_DSI_TURN_ON_PERIPHERAL			= 0x32,

	MIPI_DSI_GENERIC_SHORT_WRITE_0_PARAM		= 0x03,
	MIPI_DSI_GENERIC_SHORT_WRITE_1_PARAM		= 0x13,
	MIPI_DSI_GENERIC_SHORT_WRITE_2_PARAM		= 0x23,

	MIPI_DSI_GENERIC_READ_REQUEST_0_PARAM		= 0x04,
	MIPI_DSI_GENERIC_READ_REQUEST_1_PARAM		= 0x14,
	MIPI_DSI_GENERIC_READ_REQUEST_2_PARAM		= 0x24,

	MIPI_DSI_DCS_SHORT_WRITE			= 0x05,
	MIPI_DSI_DCS_SHORT_WRITE_PARAM			= 0x15,

	MIPI_DSI_DCS_READ				= 0x06,

	MIPI_DSI_SET_MAXIMUM_RETURN_PACKET_SIZE		= 0x37,

	MIPI_DSI_END_OF_TRANSMISSION			= 0x08,

	MIPI_DSI_NULL_PACKET				= 0x09,
	MIPI_DSI_BLANKING_PACKET			= 0x19,
	MIPI_DSI_GENERIC_LONG_WRITE			= 0x29,
	MIPI_DSI_DCS_LONG_WRITE				= 0x39,

	MIPI_DSI_LOOSELY_PACKED_PIXEL_STREAM_YCBCR20	= 0x0c,
	MIPI_DSI_PACKED_PIXEL_STREAM_YCBCR24		= 0x1c,
	MIPI_DSI_PACKED_PIXEL_STREAM_YCBCR16		= 0x2c,

	MIPI_DSI_PACKED_PIXEL_STREAM_30			= 0x0d,
	MIPI_DSI_PACKED_PIXEL_STREAM_36			= 0x1d,
	MIPI_DSI_PACKED_PIXEL_STREAM_YCBCR12		= 0x3d,

	MIPI_DSI_PACKED_PIXEL_STREAM_16			= 0x0e,
	MIPI_DSI_PACKED_PIXEL_STREAM_18			= 0x1e,
	MIPI_DSI_PIXEL_STREAM_3BYTE_18			= 0x2e,
	MIPI_DSI_PACKED_PIXEL_STREAM_24			= 0x3e,
};

/* MIPI DSI Peripheral-to-Processor transaction types */
enum {
	MIPI_DSI_RX_ACKNOWLEDGE_AND_ERROR_REPORT	= 0x02,
	MIPI_DSI_RX_END_OF_TRANSMISSION			= 0x08,
	MIPI_DSI_RX_GENERIC_SHORT_READ_RESPONSE_1BYTE	= 0x11,
	MIPI_DSI_RX_GENERIC_SHORT_READ_RESPONSE_2BYTE	= 0x12,
	MIPI_DSI_RX_GENERIC_LONG_READ_RESPONSE		= 0x1a,
	MIPI_DSI_RX_DCS_LONG_READ_RESPONSE		= 0x1c,
	MIPI_DSI_RX_DCS_SHORT_READ_RESPONSE_1BYTE	= 0x21,
	MIPI_DSI_RX_DCS_SHORT_READ_RESPONSE_2BYTE	= 0x22,
};

/* MIPI DCS commands */
enum {
	MIPI_DCS_NOP			= 0x00,
	MIPI_DCS_SOFT_RESET		= 0x01,
	MIPI_DCS_GET_DISPLAY_ID		= 0x04,
	MIPI_DCS_GET_RED_CHANNEL	= 0x06,
	MIPI_DCS_GET_GREEN_CHANNEL	= 0x07,
	MIPI_DCS_GET_BLUE_CHANNEL	= 0x08,
	MIPI_DCS_GET_DISPLAY_STATUS	= 0x09,
	MIPI_DCS_GET_POWER_MODE		= 0x0A,
	MIPI_DCS_GET_ADDRESS_MODE	= 0x0B,
	MIPI_DCS_GET_PIXEL_FORMAT	= 0x0C,
	MIPI_DCS_GET_DISPLAY_MODE	= 0x0D,
	MIPI_DCS_GET_SIGNAL_MODE	= 0x0E,
	MIPI_DCS_GET_DIAGNOSTIC_RESULT	= 0x0F,
	MIPI_DCS_ENTER_SLEEP_MODE	= 0x10,
	MIPI_DCS_EXIT_SLEEP_MODE	= 0x11,
	MIPI_DCS_ENTER_PARTIAL_MODE	= 0x12,
	MIPI_DCS_ENTER_NORMAL_MODE	= 0x13,
	MIPI_DCS_EXIT_INVERT_MODE	= 0x20,
	MIPI_DCS_ENTER_INVERT_MODE	= 0x21,
	MIPI_DCS_SET_GAMMA_CURVE	= 0x26,
	MIPI_DCS_SET_DISPLAY_OFF	= 0x28,
	MIPI_DCS_SET_DISPLAY_ON		= 0x29,
	MIPI_DCS_SET_COLUMN_ADDRESS	= 0x2A,
	MIPI_DCS_SET_PAGE_ADDRESS	= 0x2B,
	MIPI_DCS_WRITE_MEMORY_START	= 0x2C,
	MIPI_DCS_WRITE_LUT		= 0x2D,
	MIPI_DCS_READ_MEMORY_START	= 0x2E,
	MIPI_DCS_SET_PARTIAL_AREA	= 0x30,
	MIPI_DCS_SET_SCROLL_AREA	= 0x33,
	MIPI_DCS_SET_TEAR_OFF		= 0x34,
	MIPI_DCS_SET_TEAR_ON		= 0x35,
	MIPI_DCS_SET_ADDRESS_MODE	= 0x36,
	MIPI_DCS_SET_SCROLL_START	= 0x37,
	MIPI_DCS_EXIT_IDLE_MODE		= 0x38,
	MIPI_DCS_ENTER_IDLE_MODE	= 0x39,
	MIPI_DCS_SET_PIXEL_FORMAT	= 0x3A,
	MIPI_DCS_WRITE_MEMORY_CONTINUE	= 0x3C,
	MIPI_DCS_READ_MEMORY_CONTINUE	= 0x3E,
	MIPI_DCS_SET_TEAR_SCANLINE	= 0x44,
	MIPI_DCS_GET_SCANLINE		= 0x45,
	MIPI_DCS_READ_DDB_START		= 0xA1,
	MIPI_DCS_READ_DDB_CONTINUE	= 0xA8,
};

/* MIPI DCS pixel formats */
#define MIPI_DCS_PIXEL_FMT_24BIT	7
#define MIPI_DCS_PIXEL_FMT_18BIT	6
#define MIPI_DCS_PIXEL_FMT_16BIT	5
#define MIPI_DCS_PIXEL_FMT_12BIT	3
#define MIPI_DCS_PIXEL_FMT_8BIT		2
#define MIPI_DCS_PIXEL_FMT_3BIT		1

/* request ACK from peripheral */
#define MIPI_DSI_MSG_REQ_ACK    BIT(0)
/* use Low Power Mode to transmit message */
#define MIPI_DSI_MSG_USE_LPM    BIT(1)

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
	u8 channel;	/* virtual channel id */
	u8 type;	/* payload data type */
	u16 flags;	/* flags controlling this message transmission */
	size_t tx_len;
	const void *tx_buf;
	size_t rx_len;
	void *rx_buf;
};

/* DSI mode flags */

/* video mode */
#define MIPI_DSI_MODE_VIDEO             BIT(0)
/* video burst mode */
#define MIPI_DSI_MODE_VIDEO_BURST       BIT(1)
/* video pulse mode */
#define MIPI_DSI_MODE_VIDEO_SYNC_PULSE  BIT(2)
/* enable auto vertical count mode */
#define MIPI_DSI_MODE_VIDEO_AUTO_VERT   BIT(3)
/* enable hsync-end packets in vsync-pulse and v-porch area */
#define MIPI_DSI_MODE_VIDEO_HSE         BIT(4)
/* disable hfront-porch area */
#define MIPI_DSI_MODE_VIDEO_HFP         BIT(5)
/* disable hback-porch area */
#define MIPI_DSI_MODE_VIDEO_HBP         BIT(6)
/* disable hsync-active area */
#define MIPI_DSI_MODE_VIDEO_HSA         BIT(7)
/* flush display FIFO on vsync pulse */
#define MIPI_DSI_MODE_VSYNC_FLUSH       BIT(8)
/* disable EoT packets in HS mode */
#define MIPI_DSI_MODE_EOT_PACKET        BIT(9)
/* device supports non-continuous clock behavior (DSI spec 5.6.1) */
#define MIPI_DSI_CLOCK_NON_CONTINUOUS   BIT(10)
/* transmit data in low power */
#define MIPI_DSI_MODE_LPM               BIT(11) /* DSI mode flags */

enum mipi_dsi_pixel_format {
	MIPI_DSI_FMT_RGB888,
	MIPI_DSI_FMT_RGB666,
	MIPI_DSI_FMT_RGB666_PACKED,
	MIPI_DSI_FMT_RGB565,
};

/**
 * struct mipi_dsi_device - DSI peripheral device
 * @host: DSI host for this peripheral
 * @dev: driver model device node for this peripheral
 * @channel: virtual channel assigned to the peripheral
 * @format: pixel format for video mode
 * @lanes: number of active data lanes
 * @mode_flags: DSI operation mode related flags
 */
struct mipi_dsi_device {
	unsigned int channel;
	unsigned int lanes;
	enum mipi_dsi_pixel_format format;
	unsigned long mode_flags;
	struct mipi_panel_ops *ops;
	ssize_t (*write_buffer)(struct mipi_dsi_device *dsi,
				const void *data, size_t len);
};

struct mipi_panel_ops {
	int (*init)(struct mipi_dsi_device *dsi, int width, int height);
	int (*prepare)(struct mipi_dsi_device *dsi);
	int (*unprepare)(struct mipi_dsi_device *dsi);
	int (*enable)(struct mipi_dsi_device *dsi);
	int (*disable)(struct mipi_dsi_device *dsi);
	void *private_data;
};

#endif
