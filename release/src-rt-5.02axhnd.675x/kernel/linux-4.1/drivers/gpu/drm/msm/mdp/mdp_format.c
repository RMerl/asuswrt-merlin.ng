/*
 * Copyright (c) 2014 The Linux Foundation. All rights reserved.
 * Copyright (C) 2013 Red Hat
 * Author: Rob Clark <robdclark@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include "msm_drv.h"
#include "mdp_kms.h"

static struct csc_cfg csc_convert[CSC_MAX] = {
	[CSC_RGB2RGB] = {
		.type = CSC_RGB2RGB,
		.matrix = {
			0x0200, 0x0000, 0x0000,
			0x0000, 0x0200, 0x0000,
			0x0000, 0x0000, 0x0200
		},
		.pre_bias =	{ 0x0, 0x0, 0x0 },
		.post_bias =	{ 0x0, 0x0, 0x0 },
		.pre_clamp =	{ 0x0, 0xff, 0x0, 0xff, 0x0, 0xff },
		.post_clamp =	{ 0x0, 0xff, 0x0, 0xff, 0x0, 0xff },
	},
	[CSC_YUV2RGB] = {
		.type = CSC_YUV2RGB,
		.matrix = {
			0x0254, 0x0000, 0x0331,
			0x0254, 0xff37, 0xfe60,
			0x0254, 0x0409, 0x0000
		},
		.pre_bias =	{ 0xfff0, 0xff80, 0xff80 },
		.post_bias =	{ 0x00, 0x00, 0x00 },
		.pre_clamp =	{ 0x00, 0xff, 0x00, 0xff, 0x00, 0xff },
		.post_clamp =	{ 0x00, 0xff, 0x00, 0xff, 0x00, 0xff },
	},
	[CSC_RGB2YUV] = {
		.type = CSC_RGB2YUV,
		.matrix = {
			0x0083, 0x0102, 0x0032,
			0x1fb5, 0x1f6c, 0x00e1,
			0x00e1, 0x1f45, 0x1fdc
		},
		.pre_bias =	{ 0x00, 0x00, 0x00 },
		.post_bias =	{ 0x10, 0x80, 0x80 },
		.pre_clamp =	{ 0x00, 0xff, 0x00, 0xff, 0x00, 0xff },
		.post_clamp =	{ 0x10, 0xeb, 0x10, 0xf0, 0x10, 0xf0 },
	},
	[CSC_YUV2YUV] = {
		.type = CSC_YUV2YUV,
		.matrix = {
			0x0200, 0x0000, 0x0000,
			0x0000, 0x0200, 0x0000,
			0x0000, 0x0000, 0x0200
		},
		.pre_bias =	{ 0x00, 0x00, 0x00 },
		.post_bias =	{ 0x00, 0x00, 0x00 },
		.pre_clamp =	{ 0x00, 0xff, 0x00, 0xff, 0x00, 0xff },
		.post_clamp =	{ 0x00, 0xff, 0x00, 0xff, 0x00, 0xff },
	},
};

#define FMT(name, a, r, g, b, e0, e1, e2, e3, alpha, tight, c, cnt, fp, cs) { \
		.base = { .pixel_format = DRM_FORMAT_ ## name }, \
		.bpc_a = BPC ## a ## A,                          \
		.bpc_r = BPC ## r,                               \
		.bpc_g = BPC ## g,                               \
		.bpc_b = BPC ## b,                               \
		.unpack = { e0, e1, e2, e3 },                    \
		.alpha_enable = alpha,                           \
		.unpack_tight = tight,                           \
		.cpp = c,                                        \
		.unpack_count = cnt,                             \
		.fetch_type = fp,                                \
		.chroma_sample = cs                              \
}

#define BPC0A 0

/*
 * Note: Keep RGB formats 1st, followed by YUV formats to avoid breaking
 * mdp_get_rgb_formats()'s implementation.
 */
static const struct mdp_format formats[] = {
	/*  name      a  r  g  b   e0 e1 e2 e3  alpha   tight  cpp cnt ... */
	FMT(ARGB8888, 8, 8, 8, 8,  1, 0, 2, 3,  true,   true,  4,  4,
			MDP_PLANE_INTERLEAVED, CHROMA_RGB),
	FMT(XRGB8888, 8, 8, 8, 8,  1, 0, 2, 3,  false,  true,  4,  4,
			MDP_PLANE_INTERLEAVED, CHROMA_RGB),
	FMT(RGB888,   0, 8, 8, 8,  1, 0, 2, 0,  false,  true,  3,  3,
			MDP_PLANE_INTERLEAVED, CHROMA_RGB),
	FMT(BGR888,   0, 8, 8, 8,  2, 0, 1, 0,  false,  true,  3,  3,
			MDP_PLANE_INTERLEAVED, CHROMA_RGB),
	FMT(RGB565,   0, 5, 6, 5,  1, 0, 2, 0,  false,  true,  2,  3,
			MDP_PLANE_INTERLEAVED, CHROMA_RGB),
	FMT(BGR565,   0, 5, 6, 5,  2, 0, 1, 0,  false,  true,  2,  3,
			MDP_PLANE_INTERLEAVED, CHROMA_RGB),

	/* --- RGB formats above / YUV formats below this line --- */

	FMT(NV12,     0, 8, 8, 8,  1, 2, 0, 0,  false,  true,  2, 2,
			MDP_PLANE_PSEUDO_PLANAR, CHROMA_420),
	FMT(NV21,     0, 8, 8, 8,  2, 1, 0, 0,  false,  true,  2, 2,
			MDP_PLANE_PSEUDO_PLANAR, CHROMA_420),
};

/*
 * Note:
 * @rgb_only must be set to true, when requesting
 * supported formats for RGB pipes.
 */
uint32_t mdp_get_formats(uint32_t *pixel_formats, uint32_t max_formats,
		bool rgb_only)
{
	uint32_t i;
	for (i = 0; i < ARRAY_SIZE(formats); i++) {
		const struct mdp_format *f = &formats[i];

		if (i == max_formats)
			break;

		if (rgb_only && MDP_FORMAT_IS_YUV(f))
			break;

		pixel_formats[i] = f->base.pixel_format;
	}

	return i;
}

const struct msm_format *mdp_get_format(struct msm_kms *kms, uint32_t format)
{
	int i;
	for (i = 0; i < ARRAY_SIZE(formats); i++) {
		const struct mdp_format *f = &formats[i];
		if (f->base.pixel_format == format)
			return &f->base;
	}
	return NULL;
}

struct csc_cfg *mdp_get_default_csc_cfg(enum csc_type type)
{
	if (unlikely(WARN_ON(type >= CSC_MAX)))
		return NULL;

	return &csc_convert[type];
}
