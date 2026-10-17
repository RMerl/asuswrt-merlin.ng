/* SPDX-License-Identifier: GPL-2.0+ */
/*
 *  (C) Copyright 2010
 *  NVIDIA Corporation <www.nvidia.com>
 */

#ifndef __ASM_ARCH_TEGRA_DISPLAY_H
#define __ASM_ARCH_TEGRA_DISPLAY_H

#include <asm/arch-tegra/dc.h>

/* This holds information about a window which can be displayed */
struct disp_ctl_win {
	enum win_color_depth_id fmt;	/* Color depth/format */
	unsigned int	bpp;		/* Bits per pixel */
	phys_addr_t	phys_addr;	/* Physical address in memory */
	unsigned int	x;		/* Horizontal address offset (bytes) */
	unsigned int	y;		/* Veritical address offset (bytes) */
	unsigned int	w;		/* Width of source window */
	unsigned int	h;		/* Height of source window */
	unsigned int	stride;		/* Number of bytes per line */
	unsigned int	out_x;		/* Left edge of output window (col) */
	unsigned int	out_y;		/* Top edge of output window (row) */
	unsigned int	out_w;		/* Width of output window in pixels */
	unsigned int	out_h;		/* Height of output window in pixels */
};

#endif /*__ASM_ARCH_TEGRA_DISPLAY_H*/
