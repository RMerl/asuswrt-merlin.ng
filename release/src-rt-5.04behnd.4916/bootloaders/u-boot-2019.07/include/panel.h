/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2016 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#ifndef _PANEL_H
#define _PANEL_H

struct panel_ops {
	/**
	 * enable_backlight() - Enable the panel backlight
	 *
	 * @dev:	Panel device containing the backlight to enable
	 * @return 0 if OK, -ve on error
	 */
	int (*enable_backlight)(struct udevice *dev);

	/**
	 * set_backlight - Set panel backlight brightness
	 *
	 * @dev:	Panel device containing the backlight to update
	 * @percent:	Brightness value (0 to 100, or BACKLIGHT_... value)
	 * @return 0 if OK, -ve on error
	 */
	int (*set_backlight)(struct udevice *dev, int percent);

	/**
	 * get_timings() - Get display timings from panel.
	 *
	 * @dev:	Panel device containing the display timings
	 * @tim:	Place to put timings
	 * @return 0 if OK, -ve on error
	 */
	int (*get_display_timing)(struct udevice *dev,
				  struct display_timing *timing);
};

#define panel_get_ops(dev)	((struct panel_ops *)(dev)->driver->ops)

/**
 * panel_enable_backlight() - Enable/disable the panel backlight
 *
 * @dev:	Panel device containing the backlight to enable
 * @enable:	true to enable the backlight, false to dis
 * @return 0 if OK, -ve on error
 */
int panel_enable_backlight(struct udevice *dev);

/**
 * panel_set_backlight - Set brightness for the panel backlight
 *
 * @dev:	Panel device containing the backlight to update
 * @percent:	Brightness value (0 to 100, or BACKLIGHT_... value)
 * @return 0 if OK, -ve on error
 */
int panel_set_backlight(struct udevice *dev, int percent);

/**
 * panel_get_display_timing() - Get display timings from panel.
 *
 * @dev:	Panel device containing the display timings
 * @return 0 if OK, -ve on error
 */
int panel_get_display_timing(struct udevice *dev,
			     struct display_timing *timing);

#endif
