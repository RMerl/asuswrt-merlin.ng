/*
 * MIPI DSI Bus
 *
 * Copyright (C) 2012-2013, Samsung Electronics, Co., Ltd.
 * Andrzej Hajda <a.hajda@samsung.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __DRM_MIPI_DSI_H__
#define __DRM_MIPI_DSI_H__

#include <linux/device.h>

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
 * struct mipi_dsi_host - DSI host device
 * @dev: driver model device node for this DSI host
 * @ops: DSI host operations
 */
struct mipi_dsi_host {
	struct device *dev;
	const struct mipi_dsi_host_ops *ops;
};

int mipi_dsi_host_register(struct mipi_dsi_host *host);
void mipi_dsi_host_unregister(struct mipi_dsi_host *host);

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
	struct mipi_dsi_host *host;
	struct device dev;

	unsigned int channel;
	unsigned int lanes;
	enum mipi_dsi_pixel_format format;
	unsigned long mode_flags;
};

static inline struct mipi_dsi_device *to_mipi_dsi_device(struct device *dev)
{
	return container_of(dev, struct mipi_dsi_device, dev);
}

struct mipi_dsi_device *of_find_mipi_dsi_device_by_node(struct device_node *np);
int mipi_dsi_attach(struct mipi_dsi_device *dsi);
int mipi_dsi_detach(struct mipi_dsi_device *dsi);
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

#define MIPI_DSI_DCS_POWER_MODE_DISPLAY (1 << 2)
#define MIPI_DSI_DCS_POWER_MODE_NORMAL  (1 << 3)
#define MIPI_DSI_DCS_POWER_MODE_SLEEP   (1 << 4)
#define MIPI_DSI_DCS_POWER_MODE_PARTIAL (1 << 5)
#define MIPI_DSI_DCS_POWER_MODE_IDLE    (1 << 6)

ssize_t mipi_dsi_dcs_write_buffer(struct mipi_dsi_device *dsi,
				  const void *data, size_t len);
ssize_t mipi_dsi_dcs_write(struct mipi_dsi_device *dsi, u8 cmd,
			   const void *data, size_t len);
ssize_t mipi_dsi_dcs_read(struct mipi_dsi_device *dsi, u8 cmd, void *data,
			  size_t len);
int mipi_dsi_dcs_nop(struct mipi_dsi_device *dsi);
int mipi_dsi_dcs_soft_reset(struct mipi_dsi_device *dsi);
int mipi_dsi_dcs_get_power_mode(struct mipi_dsi_device *dsi, u8 *mode);
int mipi_dsi_dcs_get_pixel_format(struct mipi_dsi_device *dsi, u8 *format);
int mipi_dsi_dcs_enter_sleep_mode(struct mipi_dsi_device *dsi);
int mipi_dsi_dcs_exit_sleep_mode(struct mipi_dsi_device *dsi);
int mipi_dsi_dcs_set_display_off(struct mipi_dsi_device *dsi);
int mipi_dsi_dcs_set_display_on(struct mipi_dsi_device *dsi);
int mipi_dsi_dcs_set_column_address(struct mipi_dsi_device *dsi, u16 start,
				    u16 end);
int mipi_dsi_dcs_set_page_address(struct mipi_dsi_device *dsi, u16 start,
				  u16 end);
int mipi_dsi_dcs_set_tear_off(struct mipi_dsi_device *dsi);
int mipi_dsi_dcs_set_tear_on(struct mipi_dsi_device *dsi,
			     enum mipi_dsi_dcs_tear_mode mode);
int mipi_dsi_dcs_set_pixel_format(struct mipi_dsi_device *dsi, u8 format);

/**
 * struct mipi_dsi_driver - DSI driver
 * @driver: device driver model driver
 * @probe: callback for device binding
 * @remove: callback for device unbinding
 * @shutdown: called at shutdown time to quiesce the device
 */
struct mipi_dsi_driver {
	struct device_driver driver;
	int(*probe)(struct mipi_dsi_device *dsi);
	int(*remove)(struct mipi_dsi_device *dsi);
	void (*shutdown)(struct mipi_dsi_device *dsi);
};

static inline struct mipi_dsi_driver *
to_mipi_dsi_driver(struct device_driver *driver)
{
	return container_of(driver, struct mipi_dsi_driver, driver);
}

static inline void *mipi_dsi_get_drvdata(const struct mipi_dsi_device *dsi)
{
	return dev_get_drvdata(&dsi->dev);
}

static inline void mipi_dsi_set_drvdata(struct mipi_dsi_device *dsi, void *data)
{
	dev_set_drvdata(&dsi->dev, data);
}

int mipi_dsi_driver_register_full(struct mipi_dsi_driver *driver,
				  struct module *owner);
void mipi_dsi_driver_unregister(struct mipi_dsi_driver *driver);

#define mipi_dsi_driver_register(driver) \
	mipi_dsi_driver_register_full(driver, THIS_MODULE)

#define module_mipi_dsi_driver(__mipi_dsi_driver) \
	module_driver(__mipi_dsi_driver, mipi_dsi_driver_register, \
			mipi_dsi_driver_unregister)

#endif /* __DRM_MIPI_DSI__ */
