// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2012 Samsung Electronics
 * Donghwa Lee <dh09.lee@samsung.com>
 */

#include <common.h>
#include <lcd.h>
#include <libtizen.h>

#include "tizen_logo_16bpp.h"
#include "tizen_logo_16bpp_gzip.h"

#ifdef CONFIG_LCD
void get_tizen_logo_info(vidinfo_t *vid)
{
	switch (vid->vl_bpix) {
	case 4:
		vid->logo_width = TIZEN_LOGO_16BPP_WIDTH;
		vid->logo_height = TIZEN_LOGO_16BPP_HEIGHT;
		vid->logo_x_offset = TIZEN_LOGO_16BPP_X_OFFSET;
		vid->logo_y_offset = TIZEN_LOGO_16BPP_Y_OFFSET;
#if defined(CONFIG_VIDEO_BMP_GZIP)
		vid->logo_addr = (ulong)tizen_logo_16bpp_gzip;
#else
		vid->logo_addr = (ulong)tizen_logo_16bpp;
#endif
		break;
	default:
		vid->logo_addr = 0;
		break;
	}
}
#endif
