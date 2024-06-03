// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2004
 * Pierre Aubert, Staubli Faverges , <p.aubert@staubli.com>
 * Copyright 2011 Freescale Semiconductor, Inc.
 */

/************************************************************************
  Get Parameters for the video mode:
  The default video mode can be defined in CONFIG_SYS_DEFAULT_VIDEO_MODE.
  If undefined, default video mode is set to 0x301
  Parameters can be set via the variable "videomode" in the environment.
  2 diferent ways are possible:
  "videomode=301"   - 301 is a hexadecimal number describing the VESA
		      mode. Following modes are implemented:

		      Colors	640x480 800x600 1024x768 1152x864 1280x1024
		     --------+---------------------------------------------
		      8 bits |	0x301	0x303	 0x305	  0x161	    0x307
		     15 bits |	0x310	0x313	 0x316	  0x162	    0x319
		     16 bits |	0x311	0x314	 0x317	  0x163	    0x31A
		     24 bits |	0x312	0x315	 0x318	    ?	    0x31B
		     --------+---------------------------------------------
  "videomode=bootargs"
		   - the parameters are parsed from the bootargs.
		      The format is "NAME:VALUE,NAME:VALUE" etc.
		      Ex.:
		      "bootargs=video=ctfb:x:800,y:600,depth:16,pclk:25000"
		      Parameters not included in the list will be taken from
		      the default mode, which is one of the following:
		      mode:0  640x480x24
		      mode:1  800x600x16
		      mode:2  1024x768x8
		      mode:3  960x720x24
		      mode:4  1152x864x16
		      mode:5  1280x1024x8

		      if "mode" is not provided within the parameter list,
		      mode:0 is assumed.
		      Following parameters are supported:
		      x	      xres = visible resolution horizontal
		      y	      yres = visible resolution vertical
		      pclk    pixelclocks in pico sec
		      le      left_marging time from sync to picture in pixelclocks
		      ri      right_marging time from picture to sync in pixelclocks
		      up      upper_margin time from sync to picture
		      lo      lower_margin
		      hs      hsync_len length of horizontal sync
		      vs      vsync_len length of vertical sync
		      sync    see FB_SYNC_*
		      vmode   see FB_VMODE_*
		      depth   Color depth in bits per pixel
		      All other parameters in the variable bootargs are ignored.
		      It is also possible to set the parameters direct in the
		      variable "videomode", or in another variable i.e.
		      "myvideo" and setting the variable "videomode=myvideo"..
****************************************************************************/

#include <common.h>
#include <edid.h>
#include <errno.h>
#include <linux/ctype.h>

#include "videomodes.h"

const struct ctfb_vesa_modes vesa_modes[VESA_MODES_COUNT] = {
	{0x301, RES_MODE_640x480, 8},
	{0x310, RES_MODE_640x480, 15},
	{0x311, RES_MODE_640x480, 16},
	{0x312, RES_MODE_640x480, 24},
	{0x303, RES_MODE_800x600, 8},
	{0x313, RES_MODE_800x600, 15},
	{0x314, RES_MODE_800x600, 16},
	{0x315, RES_MODE_800x600, 24},
	{0x305, RES_MODE_1024x768, 8},
	{0x316, RES_MODE_1024x768, 15},
	{0x317, RES_MODE_1024x768, 16},
	{0x318, RES_MODE_1024x768, 24},
	{0x161, RES_MODE_1152x864, 8},
	{0x162, RES_MODE_1152x864, 15},
	{0x163, RES_MODE_1152x864, 16},
	{0x307, RES_MODE_1280x1024, 8},
	{0x319, RES_MODE_1280x1024, 15},
	{0x31A, RES_MODE_1280x1024, 16},
	{0x31B, RES_MODE_1280x1024, 24},
};
const struct ctfb_res_modes res_mode_init[RES_MODES_COUNT] = {
	/*  x     y  hz  pixclk ps/kHz   le   ri  up  lo   hs vs  s  vmode */
#ifndef CONFIG_VIDEO_STD_TIMINGS
	{ 640,  480, 60, 39721,  25180,  40,  24, 32, 11,  96, 2, 0, FB_VMODE_NONINTERLACED},
	{ 800,  600, 60, 27778,  36000,  64,  24, 22,  1,  72, 2, 0, FB_VMODE_NONINTERLACED},
	{1024,  768, 60, 15384,  65000, 168,   8, 29,  3, 144, 4, 0, FB_VMODE_NONINTERLACED},
	{ 960,  720, 80, 13100,  76335, 160,  40, 32,  8,  80, 4, 0, FB_VMODE_NONINTERLACED},
	{1152,  864, 60, 12004,  83300, 200,  64, 32, 16,  80, 4, 0, FB_VMODE_NONINTERLACED},
	{1280, 1024, 60,  9090, 110000, 200,  48, 26,  1, 184, 3, 0, FB_VMODE_NONINTERLACED},
#else
	{ 640,  480, 60, 39683,  25200,  48,  16, 33, 10,  96, 2, 0, FB_VMODE_NONINTERLACED},
	{ 800,  600, 60, 25000,  40000,  88,  40, 23,  1, 128, 4, FB_SYNC_HOR_HIGH_ACT | FB_SYNC_VERT_HIGH_ACT, FB_VMODE_NONINTERLACED},
	{1024,  768, 60, 15384,  65000, 160,  24, 29,  3, 136, 6, 0, FB_VMODE_NONINTERLACED},
	{ 960,  720, 75, 13468,  74250, 176,  72, 27,  1, 112, 2, 0, FB_VMODE_NONINTERLACED},
	{1152,  864, 75,  9259, 108000, 256,  64, 32,  1, 128, 3, FB_SYNC_HOR_HIGH_ACT | FB_SYNC_VERT_HIGH_ACT, FB_VMODE_NONINTERLACED},
	{1280, 1024, 60,  9259, 108000, 248,  48, 38,  1, 112, 3, FB_SYNC_HOR_HIGH_ACT | FB_SYNC_VERT_HIGH_ACT, FB_VMODE_NONINTERLACED},
#endif
	{1280,  720, 60, 13468,  74250, 220, 110, 20,  5,  40, 5, FB_SYNC_HOR_HIGH_ACT | FB_SYNC_VERT_HIGH_ACT, FB_VMODE_NONINTERLACED},
	{1360,  768, 60, 11696,  85500, 256,  64, 17,  3, 112, 7, 0, FB_VMODE_NONINTERLACED},
	{1920, 1080, 60,  6734, 148500, 148,  88, 36,  4,  44, 5, FB_SYNC_HOR_HIGH_ACT | FB_SYNC_VERT_HIGH_ACT, FB_VMODE_NONINTERLACED},
	{1920, 1200, 60,  6494, 154000,  80,  48, 26,  3,  32, 6, FB_SYNC_HOR_HIGH_ACT, FB_VMODE_NONINTERLACED},
};

/************************************************************************
 * Get Parameters for the video mode:
 */
/*********************************************************************
 * returns the length to the next seperator
 */
static int
video_get_param_len(const char *start, char sep)
{
	int i = 0;
	while ((*start != 0) && (*start != sep)) {
		start++;
		i++;
	}
	return i;
}

static int
video_search_param (char *start, char *param)
{
	int len, totallen, i;
	char *p = start;
	len = strlen (param);
	totallen = len + strlen (start);
	for (i = 0; i < totallen; i++) {
		if (strncmp (p++, param, len) == 0)
			return (i);
	}
	return -1;
}

/***************************************************************
 * Get parameter via the environment as it is done for the
 * linux kernel i.e:
 * video=ctfb:x:800,xv:1280,y:600,yv:1024,depth:16,mode:0,pclk:25000,
 *	 le:56,ri:48,up:26,lo:5,hs:152,vs:2,sync:0,vmode:0,accel:0
 *
 * penv is a pointer to the environment, containing the string, or the name of
 * another environment variable. It could even be the term "bootargs"
 */

#define GET_OPTION(name,var)				\
	if(strncmp(p,name,strlen(name))==0) {		\
		val_s=p+strlen(name);			\
		var=simple_strtoul(val_s, NULL, 10);	\
	}

int video_get_params (struct ctfb_res_modes *pPar, char *penv)
{
	char *p, *s, *val_s;
	int i = 0;
	int bpp;
	int mode;

	/* first search for the environment containing the real param string */
	s = penv;

	p = env_get(s);
	if (p)
		s = p;

	/*
	 * in case of the bootargs line, we have to start
	 * after "video=ctfb:"
	 */
	i = video_search_param (s, "video=ctfb:");
	if (i >= 0) {
		s += i;
		s += strlen ("video=ctfb:");
	}
	/* search for mode as a default value */
	p = s;
	mode = 0;		/* default */

	while ((i = video_get_param_len (p, ',')) != 0) {
		GET_OPTION ("mode:", mode)
			p += i;
		if (*p != 0)
			p++;	/* skip ',' */
	}

	if (mode >= RES_MODES_COUNT)
		mode = 0;

	*pPar = res_mode_init[mode];	/* copy default values */
	bpp = 24 - ((mode % 3) * 8);
	p = s;			/* restart */

	while ((i = video_get_param_len (p, ',')) != 0) {
		GET_OPTION ("x:", pPar->xres)
			GET_OPTION ("y:", pPar->yres)
			GET_OPTION ("refresh:", pPar->refresh)
			GET_OPTION ("le:", pPar->left_margin)
			GET_OPTION ("ri:", pPar->right_margin)
			GET_OPTION ("up:", pPar->upper_margin)
			GET_OPTION ("lo:", pPar->lower_margin)
			GET_OPTION ("hs:", pPar->hsync_len)
			GET_OPTION ("vs:", pPar->vsync_len)
			GET_OPTION ("sync:", pPar->sync)
			GET_OPTION ("vmode:", pPar->vmode)
			GET_OPTION ("pclk:", pPar->pixclock)
			GET_OPTION ("pclk_khz:", pPar->pixclock_khz)
			GET_OPTION ("depth:", bpp)
			p += i;
		if (*p != 0)
			p++;	/* skip ',' */
	}
	return bpp;
}

/*
 * Parse the 'video-mode' environment variable
 *
 * Example: "video-mode=fslfb:1280x1024-32@60,monitor=dvi".  See
 * doc/README.video for more information on how to set the variable.
 *
 * @xres: returned value of X-resolution
 * @yres: returned value of Y-resolution
 * @depth: returned value of color depth
 * @freq: returned value of monitor frequency
 * @options: pointer to any remaining options, or NULL
 *
 * Returns 1 if valid values were found, 0 otherwise
 */
int video_get_video_mode(unsigned int *xres, unsigned int *yres,
	unsigned int *depth, unsigned int *freq, const char **options)
{
	char *p = env_get("video-mode");
	if (!p)
		return 0;

	/* Skip over the driver name, which we don't care about. */
	p = strchr(p, ':');
	if (!p)
		return 0;

	/* Get the X-resolution*/
	while (*p && !isdigit(*p))
		p++;
	*xres = simple_strtoul(p, &p, 10);
	if (!*xres)
		return 0;

	/* Get the Y-resolution */
	while (*p && !isdigit(*p))
		p++;
	*yres = simple_strtoul(p, &p, 10);
	if (!*yres)
		return 0;

	/* Get the depth */
	while (*p && !isdigit(*p))
		p++;
	*depth = simple_strtoul(p, &p, 10);
	if (!*depth)
		return 0;

	/* Get the frequency */
	while (*p && !isdigit(*p))
		p++;
	*freq = simple_strtoul(p, &p, 10);
	if (!*freq)
		return 0;

	/* Find the extra options, if any */
	p = strchr(p, ',');
	*options = p ? p + 1 : NULL;

	return 1;
}

/*
 * Parse the 'video-mode' environment variable using video_get_video_mode()
 * and lookup the matching ctfb_res_modes in res_mode_init.
 *
 * @default_mode: RES_MODE_##x## define for the mode to store in mode_ret
 *   when 'video-mode' is not set or does not contain a valid mode
 * @default_depth: depth to set when 'video-mode' is not set
 * @mode_ret: pointer where the mode will be stored
 * @depth_ret: pointer where the depth will be stored
 * @options: pointer to any remaining options, or NULL
 */
void video_get_ctfb_res_modes(int default_mode, unsigned int default_depth,
			      const struct ctfb_res_modes **mode_ret,
			      unsigned int *depth_ret,
			      const char **options)
{
	unsigned int i, xres, yres, depth, refresh;

	*mode_ret = &res_mode_init[default_mode];
	*depth_ret = default_depth;
	*options = NULL;

	if (!video_get_video_mode(&xres, &yres, &depth, &refresh, options))
		return;

	for (i = 0; i < RES_MODES_COUNT; i++) {
		if (res_mode_init[i].xres == xres &&
		    res_mode_init[i].yres == yres &&
		    res_mode_init[i].refresh == refresh) {
			*mode_ret = &res_mode_init[i];
			*depth_ret = depth;
			return;
		}
	}

	printf("video-mode %dx%d-%d@%d not available, falling back to %dx%d-%d@%d\n",
	       xres, yres, depth, refresh, (*mode_ret)->xres,
	       (*mode_ret)->yres, *depth_ret, (*mode_ret)->refresh);
}

/*
 * Find the named string option within the ',' separated options string, and
 * store its value in dest.
 *
 * @options: ',' separated options string
 * @name: name of the option to look for
 * @dest: destination buffer to store the value of the option in
 * @dest_len: length of dest
 * @def: value to store in dest if the option is not present in options
 */
void video_get_option_string(const char *options, const char *name,
			     char *dest, int dest_len, const char *def)
{
	const char *p = options;
	const int name_len = strlen(name);
	int i, len;

	while (p && (i = video_get_param_len(p, ',')) != 0) {
		if (strncmp(p, name, name_len) == 0 && p[name_len] == '=') {
			len = i - (name_len + 1);
			if (len >= dest_len)
				len = dest_len - 1;
			memcpy(dest, &p[name_len + 1], len);
			dest[len] = 0;
			return;
		}
		p += i;
		if (*p != 0)
			p++;	/* skip ',' */
	}
	strcpy(dest, def);
}

/*
 * Find the named integer option within the ',' separated options string, and
 * return its value.
 *
 * @options: ',' separated options string
 * @name: name of the option to look for
 * @def: value to return if the option is not present in options
 */
int video_get_option_int(const char *options, const char *name, int def)
{
	const char *p = options;
	const int name_len = strlen(name);
	int i;

	while (p && (i = video_get_param_len(p, ',')) != 0) {
		if (strncmp(p, name, name_len) == 0 && p[name_len] == '=')
			return simple_strtoul(&p[name_len + 1], NULL, 10);

		p += i;
		if (*p != 0)
			p++;	/* skip ',' */
	}
	return def;
}

/**
 * Convert an EDID detailed timing to a struct ctfb_res_modes
 *
 * @param t		The EDID detailed timing to be converted
 * @param mode		Returns the converted timing
 *
 * @return 0 on success, or a negative errno on error
 */
int video_edid_dtd_to_ctfb_res_modes(struct edid_detailed_timing *t,
				     struct ctfb_res_modes *mode)
{
	int margin, h_total, v_total;

	/* Check all timings are non 0 */
	if (EDID_DETAILED_TIMING_PIXEL_CLOCK(*t) == 0 ||
	    EDID_DETAILED_TIMING_HORIZONTAL_ACTIVE(*t) == 0 ||
	    EDID_DETAILED_TIMING_HORIZONTAL_BLANKING(*t) == 0 ||
	    EDID_DETAILED_TIMING_VERTICAL_ACTIVE(*t) == 0 ||
	    EDID_DETAILED_TIMING_VERTICAL_BLANKING(*t) == 0 ||
	    EDID_DETAILED_TIMING_HSYNC_OFFSET(*t) == 0 ||
	    EDID_DETAILED_TIMING_VSYNC_OFFSET(*t) == 0 ||
	    /* 3d formats are not supported */
	    EDID_DETAILED_TIMING_FLAG_STEREO(*t) != 0)
		return -EINVAL;

	mode->xres = EDID_DETAILED_TIMING_HORIZONTAL_ACTIVE(*t);
	mode->yres = EDID_DETAILED_TIMING_VERTICAL_ACTIVE(*t);

	h_total = mode->xres + EDID_DETAILED_TIMING_HORIZONTAL_BLANKING(*t);
	v_total = mode->yres + EDID_DETAILED_TIMING_VERTICAL_BLANKING(*t);
	mode->refresh = EDID_DETAILED_TIMING_PIXEL_CLOCK(*t) /
			(h_total * v_total);

	mode->pixclock_khz = EDID_DETAILED_TIMING_PIXEL_CLOCK(*t) / 1000;
	mode->pixclock = 1000000000L / mode->pixclock_khz;

	mode->right_margin = EDID_DETAILED_TIMING_HSYNC_OFFSET(*t);
	mode->hsync_len = EDID_DETAILED_TIMING_HSYNC_PULSE_WIDTH(*t);
	margin = EDID_DETAILED_TIMING_HORIZONTAL_BLANKING(*t) -
			(mode->right_margin + mode->hsync_len);
	if (margin <= 0)
		return -EINVAL;

	mode->left_margin = margin;

	mode->lower_margin = EDID_DETAILED_TIMING_VSYNC_OFFSET(*t);
	mode->vsync_len = EDID_DETAILED_TIMING_VSYNC_PULSE_WIDTH(*t);
	margin = EDID_DETAILED_TIMING_VERTICAL_BLANKING(*t) -
			(mode->lower_margin + mode->vsync_len);
	if (margin <= 0)
		return -EINVAL;

	mode->upper_margin = margin;

	mode->sync = 0;
	if (EDID_DETAILED_TIMING_FLAG_HSYNC_POLARITY(*t))
		mode->sync |= FB_SYNC_HOR_HIGH_ACT;
	if (EDID_DETAILED_TIMING_FLAG_VSYNC_POLARITY(*t))
		mode->sync |= FB_SYNC_VERT_HIGH_ACT;

	if (EDID_DETAILED_TIMING_FLAG_INTERLACED(*t))
		mode->vmode = FB_VMODE_INTERLACED;
	else
		mode->vmode = FB_VMODE_NONINTERLACED;

	return 0;
}
