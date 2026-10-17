/* SPDX-License-Identifier: GPL-2.0+ */

#ifndef __IMX_VIDEO_H_
#define __IMX_VIDEO_H_

#include <linux/fb.h>
#include <ipu_pixfmt.h>

struct display_info_t {
	int	bus;
	int	addr;
	int	pixfmt;
	int	di;
	int	(*detect)(struct display_info_t const *dev);
	void	(*enable)(struct display_info_t const *dev);
	struct	fb_videomode mode;
};

#ifdef CONFIG_IMX_HDMI
extern int detect_hdmi(struct display_info_t const *dev);
#endif

#ifdef CONFIG_IMX_VIDEO_SKIP
extern struct display_info_t const displays[];
extern size_t display_count;
#endif

int ipu_set_ldb_clock(int rate);
int ipu_displays_init(void);
#endif
