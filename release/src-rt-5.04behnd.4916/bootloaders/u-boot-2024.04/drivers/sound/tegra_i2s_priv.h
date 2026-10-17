/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2018 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#ifndef __TEGRA_I2S_PRIV_H
#define __TEGRA_I2S_PRIV_H

enum {
	/* Set i2s device (in buf) */
	AHUB_MISCOP_SET_I2S,
};

/*
 * tegra_i2s_set_cif_tx_ctrl() - Set the I2C port to send to
 *
 * The CIF is not really part of I2S -- it's for Audio Hub to control
 * the interface between I2S and Audio Hub.  However since it's put in
 * the I2S registers domain instead of the Audio Hub, we need to export
 * this as a function.
 *
 * @dev: I2S device
 * @value: Value to write to CIF_TX_CTRL register
 * Return: 0
 */
int tegra_i2s_set_cif_tx_ctrl(struct udevice *dev, u32 value);

#endif
