/**
 * \file pcm/pcm_softvol.c
 * \ingroup PCM_Plugins
 * \brief PCM Soft Volume Plugin Interface
 * \author Takashi Iwai <tiwai@suse.de>
 * \date 2004
 */
/*
 *  PCM - Soft Volume Plugin
 *  Copyright (c) 2004 by Takashi Iwai <tiwai@suse.de>
 *
 *
 *   This library is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser General Public License as
 *   published by the Free Software Foundation; either version 2.1 of
 *   the License, or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this library; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 */

#include <byteswap.h>
#include <math.h>
#include "pcm_local.h"
#include "pcm_plugin.h"

#ifndef PIC
/* entry for static linking */
const char *_snd_module_pcm_softvol = "";
#endif

#ifndef DOC_HIDDEN

typedef struct {
	/* This field need to be the first */
	snd_pcm_plugin_t plug;
	snd_pcm_format_t sformat;
	unsigned int cchannels;
	snd_ctl_t *ctl;
	snd_ctl_elem_value_t elem;
	unsigned int cur_vol[2];
	unsigned int max_val;     /* max index */
	unsigned int zero_dB_val; /* index at 0 dB */
	double min_dB;
	double max_dB;
	unsigned int *dB_value;
} snd_pcm_softvol_t;

#define VOL_SCALE_SHIFT		16
#define VOL_SCALE_MASK          ((1 << VOL_SCALE_SHIFT) - 1)

#define PRESET_RESOLUTION	256
#define PRESET_MIN_DB		-51.0
#define ZERO_DB                  0.0
#define MAX_DB_UPPER_LIMIT      50

static const unsigned int preset_dB_value[PRESET_RESOLUTION] = {
	0x00b8, 0x00bd, 0x00c1, 0x00c5, 0x00ca, 0x00cf, 0x00d4, 0x00d9,
	0x00de, 0x00e3, 0x00e8, 0x00ed, 0x00f3, 0x00f9, 0x00fe, 0x0104,
	0x010a, 0x0111, 0x0117, 0x011e, 0x0124, 0x012b, 0x0132, 0x0139,
	0x0140, 0x0148, 0x0150, 0x0157, 0x015f, 0x0168, 0x0170, 0x0179,
	0x0181, 0x018a, 0x0194, 0x019d, 0x01a7, 0x01b0, 0x01bb, 0x01c5,
	0x01cf, 0x01da, 0x01e5, 0x01f1, 0x01fc, 0x0208, 0x0214, 0x0221,
	0x022d, 0x023a, 0x0248, 0x0255, 0x0263, 0x0271, 0x0280, 0x028f,
	0x029e, 0x02ae, 0x02be, 0x02ce, 0x02df, 0x02f0, 0x0301, 0x0313,
	0x0326, 0x0339, 0x034c, 0x035f, 0x0374, 0x0388, 0x039d, 0x03b3,
	0x03c9, 0x03df, 0x03f7, 0x040e, 0x0426, 0x043f, 0x0458, 0x0472,
	0x048d, 0x04a8, 0x04c4, 0x04e0, 0x04fd, 0x051b, 0x053a, 0x0559,
	0x0579, 0x0599, 0x05bb, 0x05dd, 0x0600, 0x0624, 0x0648, 0x066e,
	0x0694, 0x06bb, 0x06e3, 0x070c, 0x0737, 0x0762, 0x078e, 0x07bb,
	0x07e9, 0x0818, 0x0848, 0x087a, 0x08ac, 0x08e0, 0x0915, 0x094b,
	0x0982, 0x09bb, 0x09f5, 0x0a30, 0x0a6d, 0x0aab, 0x0aeb, 0x0b2c,
	0x0b6f, 0x0bb3, 0x0bf9, 0x0c40, 0x0c89, 0x0cd4, 0x0d21, 0x0d6f,
	0x0dbf, 0x0e11, 0x0e65, 0x0ebb, 0x0f12, 0x0f6c, 0x0fc8, 0x1026,
	0x1087, 0x10e9, 0x114e, 0x11b5, 0x121f, 0x128b, 0x12fa, 0x136b,
	0x13df, 0x1455, 0x14ce, 0x154a, 0x15c9, 0x164b, 0x16d0, 0x1758,
	0x17e4, 0x1872, 0x1904, 0x1999, 0x1a32, 0x1ace, 0x1b6e, 0x1c11,
	0x1cb9, 0x1d64, 0x1e13, 0x1ec7, 0x1f7e, 0x203a, 0x20fa, 0x21bf,
	0x2288, 0x2356, 0x2429, 0x2500, 0x25dd, 0x26bf, 0x27a6, 0x2892,
	0x2984, 0x2a7c, 0x2b79, 0x2c7c, 0x2d85, 0x2e95, 0x2fab, 0x30c7,
	0x31ea, 0x3313, 0x3444, 0x357c, 0x36bb, 0x3801, 0x394f, 0x3aa5,
	0x3c02, 0x3d68, 0x3ed6, 0x404d, 0x41cd, 0x4355, 0x44e6, 0x4681,
	0x4826, 0x49d4, 0x4b8c, 0x4d4f, 0x4f1c, 0x50f3, 0x52d6, 0x54c4,
	0x56be, 0x58c3, 0x5ad4, 0x5cf2, 0x5f1c, 0x6153, 0x6398, 0x65e9,
	0x6849, 0x6ab7, 0x6d33, 0x6fbf, 0x7259, 0x7503, 0x77bd, 0x7a87,
	0x7d61, 0x804d, 0x834a, 0x8659, 0x897a, 0x8cae, 0x8ff5, 0x934f,
	0x96bd, 0x9a40, 0x9dd8, 0xa185, 0xa548, 0xa922, 0xad13, 0xb11b,
	0xb53b, 0xb973, 0xbdc5, 0xc231, 0xc6b7, 0xcb58, 0xd014, 0xd4ed,
	0xd9e3, 0xdef6, 0xe428, 0xe978, 0xeee8, 0xf479, 0xfa2b, 0xffff,
};

/* (32bit x 16bit) >> 16 */
typedef union {
	int i;
	short s[2];
} val_t;
static inline int MULTI_DIV_32x16(int a, unsigned short b)
{
	val_t v, x, y;
	v.i = a;
	y.i = 0;
#if __BYTE_ORDER == __LITTLE_ENDIAN
	x.i = (unsigned short)v.s[0];
	x.i *= b;
	y.s[0] = x.s[1];
	y.i += (int)v.s[1] * b;
#else
	x.i = (unsigned int)v.s[1] * b;
	y.s[1] = x.s[0];
	y.i += (int)v.s[0] * b;
#endif
	return y.i;
}

static inline int MULTI_DIV_int(int a, unsigned int b, int swap)
{
	unsigned int gain = (b >> VOL_SCALE_SHIFT);
	int fraction;
	a = swap ? (int)bswap_32(a) : a;
	fraction = MULTI_DIV_32x16(a, b & VOL_SCALE_MASK);
	if (gain) {
		long long amp = (long long)a * gain + fraction;
		if (amp > (int)0x7fffffff)
			amp = (int)0x7fffffff;
		else if (amp < (int)0x80000000)
			amp = (int)0x80000000;
		return swap ? (int)bswap_32((int)amp) : (int)amp;
	}
	return swap ? (int)bswap_32(fraction) : fraction;
}

/* always little endian */
static inline int MULTI_DIV_24(int a, unsigned int b)
{
	unsigned int gain = b >> VOL_SCALE_SHIFT;
	int fraction;
	fraction = MULTI_DIV_32x16(a, b & VOL_SCALE_MASK);
	if (gain) {
		long long amp = (long long)a * gain + fraction;
		if (amp > (int)0x7fffff)
			amp = (int)0x7fffff;
		else if (amp < (int)0x800000)
			amp = (int)0x800000;
		return (int)amp;
	}
	return fraction;
}

static inline short MULTI_DIV_short(short a, unsigned int b, int swap)
{
	unsigned int gain = b >> VOL_SCALE_SHIFT;
	int fraction;
	a = swap ? (short)bswap_16(a) : a;
	fraction = (int)(a * (b & VOL_SCALE_MASK)) >> VOL_SCALE_SHIFT;
	if (gain) {
		int amp = a * gain + fraction;
		if (abs(amp) > 0x7fff)
			amp = (a<0) ? (short)0x8000 : (short)0x7fff;
		return swap ? (short)bswap_16((short)amp) : (short)amp;
	}
	return swap ? (short)bswap_16((short)fraction) : (short)fraction;
}

#endif /* DOC_HIDDEN */

/*
 * apply volumue attenuation
 *
 * TODO: use SIMD operations
 */

#ifndef DOC_HIDDEN
#define CONVERT_AREA(TYPE, swap) do {	\
	unsigned int ch, fr; \
	TYPE *src, *dst; \
	for (ch = 0; ch < channels; ch++) { \
		src_area = &src_areas[ch]; \
		dst_area = &dst_areas[ch]; \
		src = snd_pcm_channel_area_addr(src_area, src_offset); \
		dst = snd_pcm_channel_area_addr(dst_area, dst_offset); \
		src_step = snd_pcm_channel_area_step(src_area) / sizeof(TYPE); \
		dst_step = snd_pcm_channel_area_step(dst_area) / sizeof(TYPE); \
		GET_VOL_SCALE; \
		fr = frames; \
		if (! vol_scale) { \
			while (fr--) { \
				*dst = 0; \
				dst += dst_step; \
			} \
		} else if (vol_scale == 0xffff) { \
			while (fr--) { \
				*dst = *src; \
				src += src_step; \
				dst += dst_step; \
			} \
		} else { \
			while (fr--) { \
				*dst = (TYPE) MULTI_DIV_##TYPE(*src, vol_scale, swap); \
				src += src_step; \
				dst += dst_step; \
			} \
		} \
	} \
} while (0)

#define CONVERT_AREA_S24_3LE() do {					\
	unsigned int ch, fr;						\
	unsigned char *src, *dst;					\
	int tmp;							\
	for (ch = 0; ch < channels; ch++) {				\
		src_area = &src_areas[ch];				\
		dst_area = &dst_areas[ch];				\
		src = snd_pcm_channel_area_addr(src_area, src_offset);	\
		dst = snd_pcm_channel_area_addr(dst_area, dst_offset);	\
		src_step = snd_pcm_channel_area_step(src_area);		\
		dst_step = snd_pcm_channel_area_step(dst_area);		\
		GET_VOL_SCALE;						\
		fr = frames;						\
		if (! vol_scale) {					\
			while (fr--) {					\
				dst[0] = dst[1] = dst[2] = 0;		\
				dst += dst_step;			\
			}						\
		} else if (vol_scale == 0xffff) {			\
			while (fr--) {					\
				dst[0] = src[0];			\
				dst[1] = src[1];			\
				dst[2] = src[2];			\
				src += dst_step;			\
				dst += src_step;			\
			}						\
		} else {						\
			while (fr--) {					\
				tmp = src[0] |				\
				      (src[1] << 8) |			\
				      (((signed char *) src)[2] << 16);	\
				tmp = MULTI_DIV_24(tmp, vol_scale);	\
				dst[0] = tmp;				\
				dst[1] = tmp >> 8;			\
				dst[2] = tmp >> 16;			\
				src += dst_step;			\
				dst += src_step;			\
			}						\
		}							\
	}								\
} while (0)
		
#define GET_VOL_SCALE \
	switch (ch) { \
	case 0: \
	case 2: \
		vol_scale = (channels == ch + 1) ? vol_c : vol[0]; \
		break; \
	case 4: \
	case 5: \
		vol_scale = vol_c; \
		break; \
	default: \
		vol_scale = vol[ch & 1]; \
		break; \
	}

#endif /* DOC_HIDDEN */

/* 2-channel stereo control */
static void softvol_convert_stereo_vol(snd_pcm_softvol_t *svol,
				       const snd_pcm_channel_area_t *dst_areas,
				       snd_pcm_uframes_t dst_offset,
				       const snd_pcm_channel_area_t *src_areas,
				       snd_pcm_uframes_t src_offset,
				       unsigned int channels,
				       snd_pcm_uframes_t frames)
{
	const snd_pcm_channel_area_t *dst_area, *src_area;
	unsigned int src_step, dst_step;
	unsigned int vol_scale, vol[2], vol_c;

	if (svol->cur_vol[0] == 0 && svol->cur_vol[1] == 0) {
		snd_pcm_areas_silence(dst_areas, dst_offset, channels, frames,
				      svol->sformat);
		return;
	} else if (svol->zero_dB_val && svol->cur_vol[0] == svol->zero_dB_val &&
		   svol->cur_vol[1] == svol->zero_dB_val) {
		snd_pcm_areas_copy(dst_areas, dst_offset, src_areas, src_offset,
				   channels, frames, svol->sformat);
		return;
	}

	if (svol->max_val == 1) {
		vol[0] = svol->cur_vol[0] ? 0xffff : 0;
		vol[1] = svol->cur_vol[1] ? 0xffff : 0;
		vol_c = vol[0] | vol[1];
	} else {
		vol[0] = svol->dB_value[svol->cur_vol[0]];
		vol[1] = svol->dB_value[svol->cur_vol[1]];
		vol_c = svol->dB_value[(svol->cur_vol[0] + svol->cur_vol[1]) / 2];
	}
	switch (svol->sformat) {
	case SND_PCM_FORMAT_S16_LE:
	case SND_PCM_FORMAT_S16_BE:
		/* 16bit samples */
		CONVERT_AREA(short, 
			     !snd_pcm_format_cpu_endian(svol->sformat));
		break;
	case SND_PCM_FORMAT_S32_LE:
	case SND_PCM_FORMAT_S32_BE:
		/* 32bit samples */
		CONVERT_AREA(int,
			     !snd_pcm_format_cpu_endian(svol->sformat));
		break;
	case SND_PCM_FORMAT_S24_3LE:
		CONVERT_AREA_S24_3LE();
		break;
	default:
		break;
	}
}

#undef GET_VOL_SCALE
#define GET_VOL_SCALE

/* mono control */
static void softvol_convert_mono_vol(snd_pcm_softvol_t *svol,
				     const snd_pcm_channel_area_t *dst_areas,
				     snd_pcm_uframes_t dst_offset,
				     const snd_pcm_channel_area_t *src_areas,
				     snd_pcm_uframes_t src_offset,
				     unsigned int channels,
				     snd_pcm_uframes_t frames)
{
	const snd_pcm_channel_area_t *dst_area, *src_area;
	unsigned int src_step, dst_step;
	unsigned int vol_scale;

	if (svol->cur_vol[0] == 0) {
		snd_pcm_areas_silence(dst_areas, dst_offset, channels, frames,
				      svol->sformat);
		return;
	} else if (svol->zero_dB_val && svol->cur_vol[0] == svol->zero_dB_val) {
		snd_pcm_areas_copy(dst_areas, dst_offset, src_areas, src_offset,
				   channels, frames, svol->sformat);
		return;
	}

	if (svol->max_val == 1)
		vol_scale = svol->cur_vol[0] ? 0xffff : 0;
	else
		vol_scale = svol->dB_value[svol->cur_vol[0]];
	switch (svol->sformat) {
	case SND_PCM_FORMAT_S16_LE:
	case SND_PCM_FORMAT_S16_BE:
		/* 16bit samples */
		CONVERT_AREA(short, 
			     !snd_pcm_format_cpu_endian(svol->sformat));
		break;
	case SND_PCM_FORMAT_S32_LE:
	case SND_PCM_FORMAT_S32_BE:
		/* 32bit samples */
		CONVERT_AREA(int,
			     !snd_pcm_format_cpu_endian(svol->sformat));
		break;
	case SND_PCM_FORMAT_S24_3LE:
		CONVERT_AREA_S24_3LE();
		break;
	default:
		break;
	}
}

/*
 * get the current volume value from driver
 *
 * TODO: mmap support?
 */
static void get_current_volume(snd_pcm_softvol_t *svol)
{
	unsigned int val;
	unsigned int i;

	if (snd_ctl_elem_read(svol->ctl, &svol->elem) < 0)
		return;
	for (i = 0; i < svol->cchannels; i++) {
		val = svol->elem.value.integer.value[i];
		if (val > svol->max_val)
			val = svol->max_val;
		svol->cur_vol[i] = val;
	}
}

static void softvol_free(snd_pcm_softvol_t *svol)
{
	if (svol->plug.gen.close_slave)
		snd_pcm_close(svol->plug.gen.slave);
	if (svol->ctl)
		snd_ctl_close(svol->ctl);
	if (svol->dB_value && svol->dB_value != preset_dB_value)
		free(svol->dB_value);
	free(svol);
}

static int snd_pcm_softvol_close(snd_pcm_t *pcm)
{
	snd_pcm_softvol_t *svol = pcm->private_data;
	softvol_free(svol);
	return 0;
}

static int snd_pcm_softvol_hw_refine_cprepare(snd_pcm_t *pcm,
					      snd_pcm_hw_params_t *params)
{
	int err;
	snd_pcm_softvol_t *svol = pcm->private_data;
	snd_pcm_access_mask_t access_mask = { SND_PCM_ACCBIT_SHM };
	snd_pcm_format_mask_t format_mask = {
		{
			(1ULL << SND_PCM_FORMAT_S16_LE) |
			(1ULL << SND_PCM_FORMAT_S16_BE) |
			(1ULL << SND_PCM_FORMAT_S32_LE) |
 			(1ULL << SND_PCM_FORMAT_S32_BE),
			(1ULL << (SND_PCM_FORMAT_S24_3LE - 32))
		}
	};
	if (svol->sformat != SND_PCM_FORMAT_UNKNOWN) {
		snd_pcm_format_mask_none(&format_mask);
		snd_pcm_format_mask_set(&format_mask, svol->sformat);
	}
	err = _snd_pcm_hw_param_set_mask(params, SND_PCM_HW_PARAM_ACCESS,
					 &access_mask);
	if (err < 0)
		return err;
	err = _snd_pcm_hw_param_set_mask(params, SND_PCM_HW_PARAM_FORMAT,
					 &format_mask);
	if (err < 0)
		return err;
	err = _snd_pcm_hw_params_set_subformat(params, SND_PCM_SUBFORMAT_STD);
	if (err < 0)
		return err;
	err = _snd_pcm_hw_param_set_min(params, SND_PCM_HW_PARAM_CHANNELS, 1, 0);
	if (err < 0)
		return err;
	params->info &= ~(SND_PCM_INFO_MMAP | SND_PCM_INFO_MMAP_VALID);
	return 0;
}

static int snd_pcm_softvol_hw_refine_sprepare(snd_pcm_t *pcm, snd_pcm_hw_params_t *sparams)
{
	snd_pcm_softvol_t *svol = pcm->private_data;
	snd_pcm_access_mask_t saccess_mask = { SND_PCM_ACCBIT_MMAP };
	_snd_pcm_hw_params_any(sparams);
	_snd_pcm_hw_param_set_mask(sparams, SND_PCM_HW_PARAM_ACCESS,
				   &saccess_mask);
	if (svol->sformat != SND_PCM_FORMAT_UNKNOWN) {
		_snd_pcm_hw_params_set_format(sparams, svol->sformat);
		_snd_pcm_hw_params_set_subformat(sparams, SND_PCM_SUBFORMAT_STD);
	}
	return 0;
}

/*
 * refine the access mask
 */
static int check_access_mask(snd_pcm_hw_params_t *src,
			     snd_pcm_hw_params_t *dst)
{
	const snd_pcm_access_mask_t *mask;
	snd_pcm_access_mask_t smask;

	mask = snd_pcm_hw_param_get_mask(src, SND_PCM_HW_PARAM_ACCESS);
	snd_mask_none(&smask);
	if (snd_pcm_access_mask_test(mask, SND_PCM_ACCESS_RW_INTERLEAVED) ||
	    snd_pcm_access_mask_test(mask, SND_PCM_ACCESS_MMAP_INTERLEAVED)) {
		snd_pcm_access_mask_set(&smask,
					SND_PCM_ACCESS_RW_INTERLEAVED);
		snd_pcm_access_mask_set(&smask,
					SND_PCM_ACCESS_MMAP_INTERLEAVED);
	}
	if (snd_pcm_access_mask_test(mask, SND_PCM_ACCESS_RW_NONINTERLEAVED) ||
	    snd_pcm_access_mask_test(mask, SND_PCM_ACCESS_MMAP_NONINTERLEAVED))  {
		snd_pcm_access_mask_set(&smask,
					SND_PCM_ACCESS_RW_NONINTERLEAVED);
		snd_pcm_access_mask_set(&smask,
					SND_PCM_ACCESS_MMAP_NONINTERLEAVED);
	}
	if (snd_pcm_access_mask_test(mask, SND_PCM_ACCESS_MMAP_COMPLEX))
		snd_pcm_access_mask_set(&smask,
					SND_PCM_ACCESS_MMAP_COMPLEX);

	return _snd_pcm_hw_param_set_mask(dst, SND_PCM_HW_PARAM_ACCESS, &smask);
}

static int snd_pcm_softvol_hw_refine_schange(snd_pcm_t *pcm,
					     snd_pcm_hw_params_t *params,
					     snd_pcm_hw_params_t *sparams)
{
	snd_pcm_softvol_t *svol = pcm->private_data;
	int err;
	unsigned int links = (SND_PCM_HW_PARBIT_CHANNELS |
			      SND_PCM_HW_PARBIT_RATE |
			      SND_PCM_HW_PARBIT_PERIODS |
			      SND_PCM_HW_PARBIT_PERIOD_SIZE |
			      SND_PCM_HW_PARBIT_PERIOD_TIME |
			      SND_PCM_HW_PARBIT_BUFFER_SIZE |
			      SND_PCM_HW_PARBIT_BUFFER_TIME |
			      SND_PCM_HW_PARBIT_TICK_TIME);
	if (svol->sformat == SND_PCM_FORMAT_UNKNOWN)
		links |= (SND_PCM_HW_PARBIT_FORMAT | 
			  SND_PCM_HW_PARBIT_SUBFORMAT |
			  SND_PCM_HW_PARBIT_SAMPLE_BITS);
	err = _snd_pcm_hw_params_refine(sparams, links, params);
	if (err < 0)
		return err;

	err = check_access_mask(params, sparams);
	if (err < 0)
		return err;

	return 0;
}
	
static int snd_pcm_softvol_hw_refine_cchange(snd_pcm_t *pcm,
					     snd_pcm_hw_params_t *params,
					    snd_pcm_hw_params_t *sparams)
{
	snd_pcm_softvol_t *svol = pcm->private_data;
	int err;
	unsigned int links = (SND_PCM_HW_PARBIT_CHANNELS |
			      SND_PCM_HW_PARBIT_RATE |
			      SND_PCM_HW_PARBIT_PERIODS |
			      SND_PCM_HW_PARBIT_PERIOD_SIZE |
			      SND_PCM_HW_PARBIT_PERIOD_TIME |
			      SND_PCM_HW_PARBIT_BUFFER_SIZE |
			      SND_PCM_HW_PARBIT_BUFFER_TIME |
			      SND_PCM_HW_PARBIT_TICK_TIME);
	if (svol->sformat == SND_PCM_FORMAT_UNKNOWN)
		links |= (SND_PCM_HW_PARBIT_FORMAT | 
			  SND_PCM_HW_PARBIT_SUBFORMAT |
			  SND_PCM_HW_PARBIT_SAMPLE_BITS);
	err = _snd_pcm_hw_params_refine(params, links, sparams);
	if (err < 0)
		return err;

	err = check_access_mask(sparams, params);
	if (err < 0)
		return err;

	return 0;
}

static int snd_pcm_softvol_hw_refine(snd_pcm_t *pcm, snd_pcm_hw_params_t *params)
{
	return snd_pcm_hw_refine_slave(pcm, params,
				       snd_pcm_softvol_hw_refine_cprepare,
				       snd_pcm_softvol_hw_refine_cchange,
				       snd_pcm_softvol_hw_refine_sprepare,
				       snd_pcm_softvol_hw_refine_schange,
				       snd_pcm_generic_hw_refine);
}

static int snd_pcm_softvol_hw_params(snd_pcm_t *pcm, snd_pcm_hw_params_t * params)
{
	snd_pcm_softvol_t *svol = pcm->private_data;
	snd_pcm_t *slave = svol->plug.gen.slave;
	int err = snd_pcm_hw_params_slave(pcm, params,
					  snd_pcm_softvol_hw_refine_cchange,
					  snd_pcm_softvol_hw_refine_sprepare,
					  snd_pcm_softvol_hw_refine_schange,
					  snd_pcm_generic_hw_params);
	if (err < 0)
		return err;
	if (slave->format != SND_PCM_FORMAT_S16_LE &&
	    slave->format != SND_PCM_FORMAT_S16_BE &&
	    slave->format != SND_PCM_FORMAT_S24_3LE && 
	    slave->format != SND_PCM_FORMAT_S32_LE &&
	    slave->format != SND_PCM_FORMAT_S32_BE) {
		SNDERR("softvol supports only S16_LE, S16_BE, S24_3LE, S32_LE "
		       " or S32_BE");
		return -EINVAL;
	}
	svol->sformat = slave->format;
	return 0;
}

static snd_pcm_uframes_t
snd_pcm_softvol_write_areas(snd_pcm_t *pcm,
			    const snd_pcm_channel_area_t *areas,
			    snd_pcm_uframes_t offset,
			    snd_pcm_uframes_t size,
			    const snd_pcm_channel_area_t *slave_areas,
			    snd_pcm_uframes_t slave_offset,
			    snd_pcm_uframes_t *slave_sizep)
{
	snd_pcm_softvol_t *svol = pcm->private_data;
	if (size > *slave_sizep)
		size = *slave_sizep;
	get_current_volume(svol);
	if (svol->cchannels == 1)
		softvol_convert_mono_vol(svol, slave_areas, slave_offset,
					 areas, offset, pcm->channels, size);
	else
		softvol_convert_stereo_vol(svol, slave_areas, slave_offset,
					   areas, offset, pcm->channels, size);
	*slave_sizep = size;
	return size;
}

static snd_pcm_uframes_t
snd_pcm_softvol_read_areas(snd_pcm_t *pcm,
			   const snd_pcm_channel_area_t *areas,
			   snd_pcm_uframes_t offset,
			   snd_pcm_uframes_t size,
			   const snd_pcm_channel_area_t *slave_areas,
			   snd_pcm_uframes_t slave_offset,
			   snd_pcm_uframes_t *slave_sizep)
{
	snd_pcm_softvol_t *svol = pcm->private_data;
	if (size > *slave_sizep)
		size = *slave_sizep;
	get_current_volume(svol);
	if (svol->cchannels == 1)
		softvol_convert_mono_vol(svol, areas, offset, slave_areas,
					 slave_offset, pcm->channels, size);
	else
		softvol_convert_stereo_vol(svol, areas, offset, slave_areas,
					   slave_offset, pcm->channels, size);
	*slave_sizep = size;
	return size;
}

static void snd_pcm_softvol_dump(snd_pcm_t *pcm, snd_output_t *out)
{
	snd_pcm_softvol_t *svol = pcm->private_data;
	snd_output_printf(out, "Soft volume PCM\n");
	snd_output_printf(out, "Control: %s\n", svol->elem.id.name);
	if (svol->max_val == 1)
		snd_output_printf(out, "boolean\n");
	else {
		snd_output_printf(out, "min_dB: %g\n", svol->min_dB);
		snd_output_printf(out, "max_dB: %g\n", svol->max_dB);
		snd_output_printf(out, "resolution: %d\n", svol->max_val + 1);
	}
	if (pcm->setup) {
		snd_output_printf(out, "Its setup is:\n");
		snd_pcm_dump_setup(pcm, out);
	}
	snd_output_printf(out, "Slave: ");
	snd_pcm_dump(svol->plug.gen.slave, out);
}

static int add_tlv_info(snd_pcm_softvol_t *svol, snd_ctl_elem_info_t *cinfo)
{
	unsigned int tlv[4];
	tlv[0] = SND_CTL_TLVT_DB_SCALE;
	tlv[1] = 2 * sizeof(int);
	tlv[2] = svol->min_dB * 100;
	tlv[3] = (svol->max_dB - svol->min_dB) * 100 / svol->max_val;
	return snd_ctl_elem_tlv_write(svol->ctl, &cinfo->id, tlv);
}

static int add_user_ctl(snd_pcm_softvol_t *svol, snd_ctl_elem_info_t *cinfo, int count)
{
	int err;
	int i;
	unsigned int def_val;
	
	if (svol->max_val == 1)
		err = snd_ctl_elem_add_boolean(svol->ctl, &cinfo->id, count);
	else
		err = snd_ctl_elem_add_integer(svol->ctl, &cinfo->id, count,
					       0, svol->max_val, 0);
	if (err < 0)
		return err;
	if (svol->max_val == 1)
		def_val = 1;
	else {
		add_tlv_info(svol, cinfo);
		/* set zero dB value as default, or max_val if
		   there is no 0 dB setting */
		def_val = svol->zero_dB_val ? svol->zero_dB_val : svol->max_val;
	}
	for (i = 0; i < count; i++)
		svol->elem.value.integer.value[i] = def_val;
	return snd_ctl_elem_write(svol->ctl, &svol->elem);
}

/*
 * load and set up user-control
 * returns 0 if the user-control is found or created,
 * returns 1 if the control is a hw control,
 * or a negative error code
 */
static int softvol_load_control(snd_pcm_t *pcm, snd_pcm_softvol_t *svol,
				int ctl_card, snd_ctl_elem_id_t *ctl_id,
				int cchannels, double min_dB, double max_dB,
				int resolution)
{
	char tmp_name[32];
	snd_pcm_info_t *info;
	snd_ctl_elem_info_t *cinfo;
	int err;
	unsigned int i;

	if (ctl_card < 0) {
		snd_pcm_info_alloca(&info);
		err = snd_pcm_info(pcm, info);
		if (err < 0)
			return err;
		ctl_card = snd_pcm_info_get_card(info);
		if (ctl_card < 0) {
			SNDERR("No card defined for softvol control");
			return -EINVAL;
		}
	}
	sprintf(tmp_name, "hw:%d", ctl_card);
	err = snd_ctl_open(&svol->ctl, tmp_name, 0);
	if (err < 0) {
		SNDERR("Cannot open CTL %s", tmp_name);
		return err;
	}

	svol->elem.id = *ctl_id;
	svol->max_val = resolution - 1;
	svol->min_dB = min_dB;
	svol->max_dB = max_dB;
	if (svol->max_val == 1 || svol->max_dB == ZERO_DB)
		svol->zero_dB_val = svol->max_val;
	else if (svol->max_dB < 0)
		svol->zero_dB_val = 0; /* there is no 0 dB setting */
	else
		svol->zero_dB_val = (min_dB / (min_dB - max_dB)) * svol->max_val;
		
	snd_ctl_elem_info_alloca(&cinfo);
	snd_ctl_elem_info_set_id(cinfo, ctl_id);
	if ((err = snd_ctl_elem_info(svol->ctl, cinfo)) < 0) {
		if (err != -ENOENT) {
			SNDERR("Cannot get info for CTL %s", tmp_name);
			return err;
		}
		err = add_user_ctl(svol, cinfo, cchannels);
		if (err < 0) {
			SNDERR("Cannot add a control");
			return err;
		}
	} else {
		if (! (cinfo->access & SNDRV_CTL_ELEM_ACCESS_USER)) {
			/* hardware control exists */
			return 1; /* notify */

		} else if ((cinfo->type != SND_CTL_ELEM_TYPE_INTEGER &&
			    cinfo->type != SND_CTL_ELEM_TYPE_BOOLEAN) ||
			   cinfo->count != (unsigned int)cchannels ||
			   cinfo->value.integer.min != 0 ||
			   cinfo->value.integer.max != resolution - 1) {
			if ((err = snd_ctl_elem_remove(svol->ctl, &cinfo->id)) < 0) {
				SNDERR("Control %s mismatch", tmp_name);
				return err;
			}
			snd_ctl_elem_info_set_id(cinfo, ctl_id); /* reset numid */
			if ((err = add_user_ctl(svol, cinfo, cchannels)) < 0) {
				SNDERR("Cannot add a control");
				return err;
			}
		} else if (svol->max_val > 1) {
			/* check TLV availability */
			unsigned int tlv[4];
			err = snd_ctl_elem_tlv_read(svol->ctl, &cinfo->id, tlv, sizeof(tlv));
			if (err < 0)
				add_tlv_info(svol, cinfo);
		}
	}

	if (svol->max_val == 1)
		return 0;

	/* set up dB table */
	if (min_dB == PRESET_MIN_DB && max_dB == ZERO_DB && resolution == PRESET_RESOLUTION)
		svol->dB_value = (unsigned int*)preset_dB_value;
	else {
#ifndef HAVE_SOFT_FLOAT
		svol->dB_value = calloc(resolution, sizeof(unsigned int));
		if (! svol->dB_value) {
			SNDERR("cannot allocate dB table");
			return -ENOMEM;
		}
		svol->min_dB = min_dB;
		svol->max_dB = max_dB;
		for (i = 0; i <= svol->max_val; i++) {
			double db = svol->min_dB + (i * (svol->max_dB - svol->min_dB)) / svol->max_val;
			double v = (pow(10.0, db / 20.0) * (double)(1 << VOL_SCALE_SHIFT));
			svol->dB_value[i] = (unsigned int)v;
		}
		if (svol->zero_dB_val)
			svol->dB_value[svol->zero_dB_val] = 65535;
#else
		SNDERR("Cannot handle the given dB range and resolution");
		return -EINVAL;
#endif
	}
	return 0;
}

static const snd_pcm_ops_t snd_pcm_softvol_ops = {
	.close = snd_pcm_softvol_close,
	.info = snd_pcm_generic_info,
	.hw_refine = snd_pcm_softvol_hw_refine,
	.hw_params = snd_pcm_softvol_hw_params,
	.hw_free = snd_pcm_generic_hw_free,
	.sw_params = snd_pcm_generic_sw_params,
	.channel_info = snd_pcm_generic_channel_info,
	.dump = snd_pcm_softvol_dump,
	.nonblock = snd_pcm_generic_nonblock,
	.async = snd_pcm_generic_async,
	.mmap = snd_pcm_generic_mmap,
	.munmap = snd_pcm_generic_munmap,
};

/**
 * \brief Creates a new SoftVolume PCM
 * \param pcmp Returns created PCM handle
 * \param name Name of PCM
 * \param sformat Slave format
 * \param ctl_card card index of the control
 * \param ctl_id The control element
 * \param cchannels PCM channels
 * \param min_dB minimal dB value
 * \param max_dB maximal dB value
 * \param resolution resolution of control
 * \param slave Slave PCM handle
 * \param close_slave When set, the slave PCM handle is closed with copy PCM
 * \retval zero on success otherwise a negative error code
 * \warning Using of this function might be dangerous in the sense
 *          of compatibility reasons. The prototype might be freely
 *          changed in future.
 */
int snd_pcm_softvol_open(snd_pcm_t **pcmp, const char *name,
			 snd_pcm_format_t sformat,
			 int ctl_card, snd_ctl_elem_id_t *ctl_id,
			 int cchannels,
			 double min_dB, double max_dB, int resolution,
			 snd_pcm_t *slave, int close_slave)
{
	snd_pcm_t *pcm;
	snd_pcm_softvol_t *svol;
	int err;
	assert(pcmp && slave);
	if (sformat != SND_PCM_FORMAT_UNKNOWN &&
	    sformat != SND_PCM_FORMAT_S16_LE &&
	    sformat != SND_PCM_FORMAT_S16_BE &&
	    sformat != SND_PCM_FORMAT_S24_3LE && 
	    sformat != SND_PCM_FORMAT_S32_LE &&
	    sformat != SND_PCM_FORMAT_S32_BE)
		return -EINVAL;
	svol = calloc(1, sizeof(*svol));
	if (! svol)
		return -ENOMEM;
	err = softvol_load_control(slave, svol, ctl_card, ctl_id, cchannels,
				   min_dB, max_dB, resolution);
	if (err < 0) {
		softvol_free(svol);
		return err;
	}
	if (err > 0) { /* hardware control - no need for softvol! */
		softvol_free(svol);
		*pcmp = slave; /* just pass the slave */
		if (!slave->name && name)
			slave->name = strdup(name);
		return 0;
	}

	/* do softvol */
	snd_pcm_plugin_init(&svol->plug);
	svol->sformat = sformat;
	svol->cchannels = cchannels;
	svol->plug.read = snd_pcm_softvol_read_areas;
	svol->plug.write = snd_pcm_softvol_write_areas;
	svol->plug.undo_read = snd_pcm_plugin_undo_read_generic;
	svol->plug.undo_write = snd_pcm_plugin_undo_write_generic;
	svol->plug.gen.slave = slave;
	svol->plug.gen.close_slave = close_slave;

	err = snd_pcm_new(&pcm, SND_PCM_TYPE_SOFTVOL, name, slave->stream, slave->mode);
	if (err < 0) {
		softvol_free(svol);
		return err;
	}
	pcm->ops = &snd_pcm_softvol_ops;
	pcm->fast_ops = &snd_pcm_plugin_fast_ops;
	pcm->private_data = svol;
	pcm->poll_fd = slave->poll_fd;
	pcm->poll_events = slave->poll_events;
	/*
	 * Since the softvol converts on the place, and the format/channels
	 * must be identical between source and destination, we don't need
	 * an extra buffer.
	 */
	pcm->mmap_shadow = 1;
	pcm->monotonic = slave->monotonic;
	snd_pcm_set_hw_ptr(pcm, &svol->plug.hw_ptr, -1, 0);
	snd_pcm_set_appl_ptr(pcm, &svol->plug.appl_ptr, -1, 0);
	*pcmp = pcm;

	return 0;
}

/* in pcm_misc.c */
int snd_pcm_parse_control_id(snd_config_t *conf, snd_ctl_elem_id_t *ctl_id, int *cardp,
			     int *cchannelsp, int *hwctlp);

/*! \page pcm_plugins

\section pcm_plugins_softvol Plugin: Soft Volume

This plugin applies the software volume attenuation.
The format, rate and channels must match for both of source and destination.

When the control is stereo (count=2), the channels are assumed to be either
mono, 2.0, 2.1, 4.0, 4.1, 5.1 or 7.1.

If the control already exists and it's a system control (i.e. no
user-defined control), the plugin simply passes its slave without
any changes.

\code
pcm.name {
        type softvol            # Soft Volume conversion PCM
        slave STR               # Slave name
        # or
        slave {                 # Slave definition
                pcm STR         # Slave PCM name
                # or
                pcm { }         # Slave PCM definition
                [format STR]    # Slave format
        }
        control {
	        name STR        # control element id string
		[card STR]      # control card index
		[iface STR]     # interface of the element
		[index INT]     # index of the element
		[device INT]    # device number of the element
		[subdevice INT] # subdevice number of the element
		[count INT]     # control channels 1 or 2 (default: 2)
	}
	[min_dB REAL]           # minimal dB value (default: -51.0)
	[max_dB REAL]           # maximal dB value (default:   0.0)
	[resolution INT]        # resolution (default: 256)
				# resolution = 2 means a mute switch
}
\endcode

\subsection pcm_plugins_softvol_funcref Function reference

<UL>
  <LI>snd_pcm_softvol_open()
  <LI>_snd_pcm_softvol_open()
</UL>

*/

/**
 * \brief Creates a new Soft Volume PCM
 * \param pcmp Returns created PCM handle
 * \param name Name of PCM
 * \param root Root configuration node
 * \param conf Configuration node with Soft Volume PCM description
 * \param stream Stream type
 * \param mode Stream mode
 * \retval zero on success otherwise a negative error code
 * \warning Using of this function might be dangerous in the sense
 *          of compatibility reasons. The prototype might be freely
 *          changed in future.
 */
int _snd_pcm_softvol_open(snd_pcm_t **pcmp, const char *name,
			  snd_config_t *root, snd_config_t *conf, 
			  snd_pcm_stream_t stream, int mode)
{
	snd_config_iterator_t i, next;
	int err;
	snd_pcm_t *spcm;
	snd_config_t *slave = NULL, *sconf;
	snd_config_t *control = NULL;
	snd_pcm_format_t sformat = SND_PCM_FORMAT_UNKNOWN;
	snd_ctl_elem_id_t *ctl_id;
	int resolution = PRESET_RESOLUTION;
	double min_dB = PRESET_MIN_DB;
	double max_dB = ZERO_DB;
	int card = -1, cchannels = 2;

	snd_config_for_each(i, next, conf) {
		snd_config_t *n = snd_config_iterator_entry(i);
		const char *id;
		if (snd_config_get_id(n, &id) < 0)
			continue;
		if (snd_pcm_conf_generic_id(id))
			continue;
		if (strcmp(id, "slave") == 0) {
			slave = n;
			continue;
		}
		if (strcmp(id, "control") == 0) {
			control = n;
			continue;
		}
		if (strcmp(id, "resolution") == 0) {
			long v;
			err = snd_config_get_integer(n, &v);
			if (err < 0) {
				SNDERR("Invalid resolution value");
				return err;
			}
			resolution = v;
			continue;
		}
		if (strcmp(id, "min_dB") == 0) {
			err = snd_config_get_real(n, &min_dB);
			if (err < 0) {
				SNDERR("Invalid min_dB value");
				return err;
			}
			continue;
		}
		if (strcmp(id, "max_dB") == 0) {
			err = snd_config_get_real(n, &max_dB);
			if (err < 0) {
				SNDERR("Invalid max_dB value");
				return err;
			}
			continue;
		}
		SNDERR("Unknown field %s", id);
		return -EINVAL;
	}
	if (!slave) {
		SNDERR("slave is not defined");
		return -EINVAL;
	}
	if (!control) {
		SNDERR("control is not defined");
		return -EINVAL;
	}
	if (min_dB >= 0) {
		SNDERR("min_dB must be a negative value");
		return -EINVAL;
	}
	if (max_dB <= min_dB || max_dB > MAX_DB_UPPER_LIMIT) {
		SNDERR("max_dB must be larger than min_dB and less than %d dB",
		       MAX_DB_UPPER_LIMIT);
		return -EINVAL;
	}
	if (resolution <= 1 || resolution > 1024) {
		SNDERR("Invalid resolution value %d", resolution);
		return -EINVAL;
	}
	if (mode & SND_PCM_NO_SOFTVOL) {
		err = snd_pcm_slave_conf(root, slave, &sconf, 0);
		if (err < 0)
			return err;
		err = snd_pcm_open_named_slave(pcmp, name, root, sconf, stream,
					       mode, conf);
		snd_config_delete(sconf);
	} else {
		snd_ctl_elem_id_alloca(&ctl_id);
		err = snd_pcm_slave_conf(root, slave, &sconf, 1,
					 SND_PCM_HW_PARAM_FORMAT, 0, &sformat);
		if (err < 0)
			return err;
		if (sformat != SND_PCM_FORMAT_UNKNOWN &&
		    sformat != SND_PCM_FORMAT_S16_LE &&
		    sformat != SND_PCM_FORMAT_S16_BE &&
		    sformat != SND_PCM_FORMAT_S24_3LE && 
		    sformat != SND_PCM_FORMAT_S32_LE &&
		    sformat != SND_PCM_FORMAT_S32_BE) {
			SNDERR("only S16_LE, S16_BE, S24_3LE, S32_LE or S32_BE format "
			       "is supported");
			snd_config_delete(sconf);
			return -EINVAL;
		}
		err = snd_pcm_open_slave(&spcm, root, sconf, stream, mode, conf);
		snd_config_delete(sconf);
		if (err < 0)
			return err;
		if ((err = snd_pcm_parse_control_id(control, ctl_id, &card, &cchannels, NULL)) < 0) {
			snd_pcm_close(spcm);
			return err;
		}
		err = snd_pcm_softvol_open(pcmp, name, sformat, card, ctl_id, cchannels,
					   min_dB, max_dB, resolution, spcm, 1);
		if (err < 0)
			snd_pcm_close(spcm);
	}
	return err;
}
#ifndef DOC_HIDDEN
SND_DLSYM_BUILD_VERSION(_snd_pcm_softvol_open, SND_PCM_DLSYM_VERSION);
#endif
