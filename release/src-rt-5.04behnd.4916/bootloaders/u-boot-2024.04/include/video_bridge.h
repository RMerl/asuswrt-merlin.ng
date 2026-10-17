/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#ifndef __VIDEO_BRIDGE
#define __VIDEO_BRIDGE

#include <asm/gpio.h>

/**
 * struct video_bridge_priv - uclass information for video bridges
 *
 * @sleep:	GPIO to assert to power down the bridge
 * @reset:	GPIO to assert to reset the bridge
 * @hotplug:	Optional GPIO to check if bridge is connected
 */
struct video_bridge_priv {
	struct gpio_desc sleep;
	struct gpio_desc reset;
	struct gpio_desc hotplug;
};

/**
 * Operations for video bridges
 */
struct video_bridge_ops {
	/**
	 * attach() - attach a video bridge
	 *
	 * @return 0 if OK, -ve on error
	 */
	int (*attach)(struct udevice *dev);

	/**
	 * check_attached() - check if a bridge is correctly attached
	 *
	 * This method is optional - if not provided then the hotplug GPIO
	 * will be checked instead.
	 *
	 * @dev:	Device to check
	 * @return 0 if attached, -EENOTCONN if not, or other -ve error
	 */
	int (*check_attached)(struct udevice *dev);

	/**
	 * set_backlight() - Set the backlight brightness
	 *
	 * @dev:	device to adjust
	 * @percent:	brightness percentage (0=off, 100=full brightness)
	 * @return 0 if OK, -ve on error
	 */
	int (*set_backlight)(struct udevice *dev, int percent);

	/**
	 * read_edid() - Read information from EDID
	 *
	 * @dev:	Device to read from
	 * @buf:	Buffer to read into
	 * @buf_size:	Buffer size
	 * @return number of bytes read, <=0 for error
	 */
	int (*read_edid)(struct udevice *dev, u8 *buf, int buf_size);
};

#define video_bridge_get_ops(dev) \
		((struct video_bridge_ops *)(dev)->driver->ops)

/**
 * video_bridge_attach() - attach a video bridge
 *
 * Return: 0 if OK, -ve on error
 */
int video_bridge_attach(struct udevice *dev);

/**
 * video_bridge_set_backlight() - Set the backlight brightness
 *
 * @percent:	brightness percentage (0=off, 100=full brightness)
 * Return: 0 if OK, -ve on error
 */
int video_bridge_set_backlight(struct udevice *dev, int percent);

/**
 * video_bridge_set_active() - take the bridge in/out of reset/powerdown
 *
 * @dev:	Device to adjust
 * @active:	true to power up and reset, false to power down
 */
int video_bridge_set_active(struct udevice *dev, bool active);

/**
 * check_attached() - check if a bridge is correctly attached
 *
 * @dev:	Device to check
 * Return: 0 if attached, -EENOTCONN if not, or other -ve error
 */
int video_bridge_check_attached(struct udevice *dev);

/**
 * video_bridge_read_edid() - Read information from EDID
 *
 * @dev:	Device to read from
 * @buf:	Buffer to read into
 * @buf_size:	Buffer size
 * Return: number of bytes read, <=0 for error
 */
int video_bridge_read_edid(struct udevice *dev, u8 *buf, int buf_size);

#endif
