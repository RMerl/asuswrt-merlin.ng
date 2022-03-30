/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2012,2015 Stephen Warren
 */

#ifndef _BCM2835_MSG_H
#define _BCM2835_MSG_H

/**
 * bcm2835_power_on_module() - power on an SoC module
 *
 * @module: ID of module to power on (BCM2835_MBOX_POWER_DEVID_...)
 * @return 0 if OK, -EIO on error
 */
int bcm2835_power_on_module(u32 module);

/**
 * bcm2835_get_mmc_clock() - get the frequency of the MMC clock
 *
 * @clock_id: ID of clock to get frequency for
 * @return clock frequency, or -ve on error
 */
int bcm2835_get_mmc_clock(u32 clock_id);

/**
 * bcm2835_get_video_size() - get the current display size
 *
 * @widthp: Returns the width in pixels
 * @heightp: Returns the height in pixels
 * @return 0 if OK, -ve on error
 */
int bcm2835_get_video_size(int *widthp, int *heightp);

/**
 * bcm2835_set_video_params() - set the video parameters
 *
 * @widthp: Video width to request (returns the actual width selected)
 * @heightp: Video height to request (returns the actual height selected)
 * @depth_bpp: Requested bit depth
 * @pixel_order: Pixel order to use (BCM2835_MBOX_PIXEL_ORDER_...)
 * @alpha_mode: Alpha transparency mode to use (BCM2835_MBOX_ALPHA_MODE_...)
 * @fb_basep: Returns base address of frame buffer
 * @fb_sizep: Returns size of frame buffer
 * @pitchp: Returns number of bytes in each frame buffer line
 * @return 0 if OK, -ve on error
 */
int bcm2835_set_video_params(int *widthp, int *heightp, int depth_bpp,
			     int pixel_order, int alpha_mode, ulong *fb_basep,
			     ulong *fb_sizep, int *pitchp);

#endif
