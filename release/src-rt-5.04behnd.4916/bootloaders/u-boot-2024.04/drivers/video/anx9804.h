/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) 2015 Hans de Goede <hdegoede@redhat.com>
 */

/*
 * Support for the ANX9804 bridge chip, which can take pixel data coming
 * from a parallel LCD interface and translate it on the flight into a DP
 * interface for driving eDP TFT displays.
 */

#ifndef _ANX9804_H
#define _ANX9804_H

#define ANX9804_DATA_RATE_1620M				0x06
#define ANX9804_DATA_RATE_2700M				0x0a

#ifdef CONFIG_VIDEO_LCD_PANEL_EDP_4_LANE_1620M_VIA_ANX9804
void anx9804_init(struct udevice *i2c_bus, u8 lanes, u8 data_rate, int bpp);
#else
static inline void anx9804_init(struct udevice *i2c_bus, u8 lanes, u8 data_rate,
				int bpp) {}
#endif

#endif
