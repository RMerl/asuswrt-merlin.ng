/* SPDX-License-Identifier: GPL-2.0+ */
/*
 *  (C) Copyright 2010
 *  NVIDIA Corporation <www.nvidia.com>
 */

#ifndef __ASM_ARCH_TEGRA_DISPLAY_H
#define __ASM_ARCH_TEGRA_DISPLAY_H

/**
 * Register a new display based on device tree configuration.
 *
 * The frame buffer can be positioned by U-Boot or overridden by the fdt.
 * You should pass in the U-Boot address here, and check the contents of
 * struct fdt_disp_config to see what was actually chosen.
 *
 * @param blob			Device tree blob
 * @param default_lcd_base	Default address of LCD frame buffer
 * @return 0 if ok, -1 on error (unsupported bits per pixel)
 */
int tegra_display_probe(const void *blob, void *default_lcd_base);

/**
 * Return the current display configuration
 *
 * @return pointer to display configuration, or NULL if there is no valid
 * config
 */
struct fdt_disp_config *tegra_display_get_config(void);

/**
 * Perform the next stage of the LCD init if it is time to do so.
 *
 * LCD init can be time-consuming because of the number of delays we need
 * while waiting for the backlight power supply, etc. This function can
 * be called at various times during U-Boot operation to advance the
 * initialization of the LCD to the next stage if sufficient time has
 * passed since the last stage. It keeps track of what stage it is up to
 * and the time that it is permitted to move to the next stage.
 *
 * The final call should have wait=1 to complete the init.
 *
 * @param blob	fdt blob containing LCD information
 * @param wait	1 to wait until all init is complete, and then return
 *		0 to return immediately, potentially doing nothing if it is
 *		not yet time for the next init.
 */
int tegra_lcd_check_next_stage(const void *blob, int wait);

/**
 * Set up the maximum LCD size so we can size the frame buffer.
 *
 * @param blob	fdt blob containing LCD information
 */
void tegra_lcd_early_init(const void *blob);

#endif /*__ASM_ARCH_TEGRA_DISPLAY_H*/
