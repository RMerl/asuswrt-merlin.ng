/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2007, 2011 Freescale Semiconductor, Inc.
 * Authors: York Sun <yorksun@freescale.com>
 *          Timur Tabi <timur@freescale.com>
 *
 * FSL DIU Framebuffer driver
 */

int fsl_diu_init(u16 xres, u16 yres, u32 pixel_format, int gamma_fix);

/* Prototypes for external board-specific functions */
int platform_diu_init(unsigned int xres, unsigned int yres, const char *port);
void diu_set_pixel_clock(unsigned int pixclock);
