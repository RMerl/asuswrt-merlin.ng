/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2000
 * Paolo Scaffardi, AIRVENT SAM s.p.a - RIMINI(ITALY), arsenio@tin.it
 */

#ifndef _VIDEO_FONT_
#define _VIDEO_FONT_

#include <video_font_data.h>

#if defined(CONFIG_VIDEO_FONT_4X6)
#include <video_font_4x6.h>
#endif
#if defined(CONFIG_VIDEO_FONT_8X16)
#include <video_font_8x16.h>
#endif
#if defined(CONFIG_VIDEO_FONT_SUN12X22)
#include <video_font_sun12x22.h>
#endif
#if defined(CONFIG_VIDEO_FONT_16X32)
#include <video_font_ter16x32.h>
#endif

static struct video_fontdata __maybe_unused fonts[] = {
#if defined(CONFIG_VIDEO_FONT_8X16)
	FONT_ENTRY(8, 16, 8x16),
#endif
#if defined(CONFIG_VIDEO_FONT_4X6)
	FONT_ENTRY(4, 6, 4x6),
#endif
#if defined(CONFIG_VIDEO_FONT_SUN12X22)
	FONT_ENTRY(12, 22, 12x22),
#endif
#if defined(CONFIG_VIDEO_FONT_16X32)
	FONT_ENTRY(16, 32, 16x32),
#endif
	{/* list terminator */}
};

#endif /* _VIDEO_FONT_ */
