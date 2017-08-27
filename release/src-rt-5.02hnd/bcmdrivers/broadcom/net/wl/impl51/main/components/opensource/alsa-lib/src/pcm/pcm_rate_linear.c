/*
 *  Linear rate converter plugin
 * 
 *  Copyright (c) 2000 by Abramo Bagnara <abramo@alsa-project.org>
 *                2004 by Jaroslav Kysela <perex@perex.cz>
 *                2006 by Takashi Iwai <tiwai@suse.de>
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
 */

#include <inttypes.h>
#include <byteswap.h>
#include "pcm_local.h"
#include "pcm_plugin.h"
#include "pcm_rate.h"

#include "plugin_ops.h"


/* LINEAR_DIV needs to be large enough to handle resampling from 192000 -> 8000 */
#define LINEAR_DIV_SHIFT 19
#define LINEAR_DIV (1<<LINEAR_DIV_SHIFT)

struct rate_linear {
	unsigned int get_idx;
	unsigned int put_idx;
	unsigned int pitch;
	unsigned int pitch_shift;	/* for expand interpolation */
	unsigned int channels;
	int16_t *old_sample;
	void (*func)(struct rate_linear *rate,
		     const snd_pcm_channel_area_t *dst_areas,
		     snd_pcm_uframes_t dst_offset, unsigned int dst_frames,
		     const snd_pcm_channel_area_t *src_areas,
		     snd_pcm_uframes_t src_offset, unsigned int src_frames);
};

static snd_pcm_uframes_t input_frames(void *obj, snd_pcm_uframes_t frames)
{
	struct rate_linear *rate = obj;
	if (frames == 0)
		return 0;
	/* Round toward zero */
	return muldiv_near(frames, LINEAR_DIV, rate->pitch);
}

static snd_pcm_uframes_t output_frames(void *obj, snd_pcm_uframes_t frames)
{
	struct rate_linear *rate = obj;
	if (frames == 0)
		return 0;
	/* Round toward zero */
	return muldiv_near(frames, rate->pitch, LINEAR_DIV);
}

static void linear_expand(struct rate_linear *rate,
			  const snd_pcm_channel_area_t *dst_areas,
			  snd_pcm_uframes_t dst_offset, unsigned int dst_frames,
			  const snd_pcm_channel_area_t *src_areas,
			  snd_pcm_uframes_t src_offset, unsigned int src_frames)
{
#define GET16_LABELS
#define PUT16_LABELS
#include "plugin_ops.h"
#undef GET16_LABELS
#undef PUT16_LABELS
	void *get = get16_labels[rate->get_idx];
	void *put = put16_labels[rate->put_idx];
	unsigned int get_threshold = rate->pitch;
	unsigned int channel;
	unsigned int src_frames1;
	unsigned int dst_frames1;
	int16_t sample = 0;
	unsigned int pos;
	
	for (channel = 0; channel < rate->channels; ++channel) {
		const snd_pcm_channel_area_t *src_area = &src_areas[channel];
		const snd_pcm_channel_area_t *dst_area = &dst_areas[channel];
		const char *src;
		char *dst;
		int src_step, dst_step;
		int16_t old_sample = 0;
		int16_t new_sample;
		int old_weight, new_weight;
		src = snd_pcm_channel_area_addr(src_area, src_offset);
		dst = snd_pcm_channel_area_addr(dst_area, dst_offset);
		src_step = snd_pcm_channel_area_step(src_area);
		dst_step = snd_pcm_channel_area_step(dst_area);
		src_frames1 = 0;
		dst_frames1 = 0;
		new_sample = rate->old_sample[channel];
		pos = get_threshold;
		while (dst_frames1 < dst_frames) {
			if (pos >= get_threshold) {
				pos -= get_threshold;
				old_sample = new_sample;
				if (src_frames1 < src_frames) {
					goto *get;
#define GET16_END after_get
#include "plugin_ops.h"
#undef GET16_END
				after_get:
					new_sample = sample;
				}
			}
			new_weight = (pos << (16 - rate->pitch_shift)) / (get_threshold >> rate->pitch_shift);
			old_weight = 0x10000 - new_weight;
			sample = (old_sample * old_weight + new_sample * new_weight) >> 16;
			goto *put;
#define PUT16_END after_put
#include "plugin_ops.h"
#undef PUT16_END
		after_put:
			dst += dst_step;
			dst_frames1++;
			pos += LINEAR_DIV;
			if (pos >= get_threshold) {
				src += src_step;
				src_frames1++;
			}
		} 
		rate->old_sample[channel] = new_sample;
	}
}

/* optimized version for S16 format */
static void linear_expand_s16(struct rate_linear *rate,
			      const snd_pcm_channel_area_t *dst_areas,
			      snd_pcm_uframes_t dst_offset, unsigned int dst_frames,
			      const snd_pcm_channel_area_t *src_areas,
			      snd_pcm_uframes_t src_offset, unsigned int src_frames)
{
	unsigned int channel;
	unsigned int src_frames1;
	unsigned int dst_frames1;
	unsigned int get_threshold = rate->pitch;
	unsigned int pos;
	
	for (channel = 0; channel < rate->channels; ++channel) {
		const snd_pcm_channel_area_t *src_area = &src_areas[channel];
		const snd_pcm_channel_area_t *dst_area = &dst_areas[channel];
		const int16_t *src;
		int16_t *dst;
		int src_step, dst_step;
		int16_t old_sample = 0;
		int16_t new_sample;
		int old_weight, new_weight;
		src = snd_pcm_channel_area_addr(src_area, src_offset);
		dst = snd_pcm_channel_area_addr(dst_area, dst_offset);
		src_step = snd_pcm_channel_area_step(src_area) >> 1;
		dst_step = snd_pcm_channel_area_step(dst_area) >> 1;
		src_frames1 = 0;
		dst_frames1 = 0;
		new_sample = rate->old_sample[channel];
		pos = get_threshold;
		while (dst_frames1 < dst_frames) {
			if (pos >= get_threshold) {
				pos -= get_threshold;
				old_sample = new_sample;
				if (src_frames1 < src_frames)
					new_sample = *src;
			}
			new_weight = (pos << (16 - rate->pitch_shift)) / (get_threshold >> rate->pitch_shift);
			old_weight = 0x10000 - new_weight;
			*dst = (old_sample * old_weight + new_sample * new_weight) >> 16;
			dst += dst_step;
			dst_frames1++;
			pos += LINEAR_DIV;
			if (pos >= get_threshold) {
				src += src_step;
				src_frames1++;
			}
		} 
		rate->old_sample[channel] = new_sample;
	}
}

static void linear_shrink(struct rate_linear *rate,
			  const snd_pcm_channel_area_t *dst_areas,
			  snd_pcm_uframes_t dst_offset, unsigned int dst_frames,
			  const snd_pcm_channel_area_t *src_areas,
			  snd_pcm_uframes_t src_offset, unsigned int src_frames)
{
#define GET16_LABELS
#define PUT16_LABELS
#include "plugin_ops.h"
#undef GET16_LABELS
#undef PUT16_LABELS
	void *get = get16_labels[rate->get_idx];
	void *put = put16_labels[rate->put_idx];
	unsigned int get_increment = rate->pitch;
	unsigned int channel;
	unsigned int src_frames1;
	unsigned int dst_frames1;
	int16_t sample = 0;
	unsigned int pos;

	for (channel = 0; channel < rate->channels; ++channel) {
		const snd_pcm_channel_area_t *src_area = &src_areas[channel];
		const snd_pcm_channel_area_t *dst_area = &dst_areas[channel];
		const char *src;
		char *dst;
		int src_step, dst_step;
		int16_t old_sample = 0;
		int16_t new_sample = 0;
		int old_weight, new_weight;
		pos = LINEAR_DIV - get_increment; /* Force first sample to be copied */
		src = snd_pcm_channel_area_addr(src_area, src_offset);
		dst = snd_pcm_channel_area_addr(dst_area, dst_offset);
		src_step = snd_pcm_channel_area_step(src_area);
		dst_step = snd_pcm_channel_area_step(dst_area);
		src_frames1 = 0;
		dst_frames1 = 0;
		while (src_frames1 < src_frames) {
			
			goto *get;
#define GET16_END after_get
#include "plugin_ops.h"
#undef GET16_END
		after_get:
			new_sample = sample;
			src += src_step;
			src_frames1++;
			pos += get_increment;
			if (pos >= LINEAR_DIV) {
				pos -= LINEAR_DIV;
				old_weight = (pos << (32 - LINEAR_DIV_SHIFT)) / (get_increment >> (LINEAR_DIV_SHIFT - 16));
				new_weight = 0x10000 - old_weight;
				sample = (old_sample * old_weight + new_sample * new_weight) >> 16;
				goto *put;
#define PUT16_END after_put
#include "plugin_ops.h"
#undef PUT16_END
			after_put:
				dst += dst_step;
				dst_frames1++;
				if (CHECK_SANITY(dst_frames1 > dst_frames)) {
					SNDERR("dst_frames overflow");
					break;
				}
			}
			old_sample = new_sample;
		}
	}
}

/* optimized version for S16 format */
static void linear_shrink_s16(struct rate_linear *rate,
			      const snd_pcm_channel_area_t *dst_areas,
			      snd_pcm_uframes_t dst_offset, unsigned int dst_frames,
			      const snd_pcm_channel_area_t *src_areas,
			      snd_pcm_uframes_t src_offset, unsigned int src_frames)
{
	unsigned int get_increment = rate->pitch;
	unsigned int channel;
	unsigned int src_frames1;
	unsigned int dst_frames1;
	unsigned int pos = 0;

	for (channel = 0; channel < rate->channels; ++channel) {
		const snd_pcm_channel_area_t *src_area = &src_areas[channel];
		const snd_pcm_channel_area_t *dst_area = &dst_areas[channel];
		const int16_t *src;
		int16_t *dst;
		int src_step, dst_step;
		int16_t old_sample = 0;
		int16_t new_sample = 0;
		int old_weight, new_weight;
		pos = LINEAR_DIV - get_increment; /* Force first sample to be copied */
		src = snd_pcm_channel_area_addr(src_area, src_offset);
		dst = snd_pcm_channel_area_addr(dst_area, dst_offset);
		src_step = snd_pcm_channel_area_step(src_area) >> 1;
		dst_step = snd_pcm_channel_area_step(dst_area) >> 1 ;
		src_frames1 = 0;
		dst_frames1 = 0;
		while (src_frames1 < src_frames) {
			
			new_sample = *src;
			src += src_step;
			src_frames1++;
			pos += get_increment;
			if (pos >= LINEAR_DIV) {
				pos -= LINEAR_DIV;
				old_weight = (pos << (32 - LINEAR_DIV_SHIFT)) / (get_increment >> (LINEAR_DIV_SHIFT - 16));
				new_weight = 0x10000 - old_weight;
				*dst = (old_sample * old_weight + new_sample * new_weight) >> 16;
				dst += dst_step;
				dst_frames1++;
				if (CHECK_SANITY(dst_frames1 > dst_frames)) {
					SNDERR("dst_frames overflow");
					break;
				}
			}
			old_sample = new_sample;
		}
	}
}

static void linear_convert(void *obj, 
			   const snd_pcm_channel_area_t *dst_areas,
			   snd_pcm_uframes_t dst_offset, unsigned int dst_frames,
			   const snd_pcm_channel_area_t *src_areas,
			   snd_pcm_uframes_t src_offset, unsigned int src_frames)
{
	struct rate_linear *rate = obj;
	rate->func(rate, dst_areas, dst_offset, dst_frames,
		   src_areas, src_offset, src_frames);
}

static void linear_free(void *obj)
{
	struct rate_linear *rate = obj;

	free(rate->old_sample);
	rate->old_sample = NULL;
}

static int linear_init(void *obj, snd_pcm_rate_info_t *info)
{
	struct rate_linear *rate = obj;

	rate->get_idx = snd_pcm_linear_get_index(info->in.format, SND_PCM_FORMAT_S16);
	rate->put_idx = snd_pcm_linear_put_index(SND_PCM_FORMAT_S16, info->out.format);
	if (info->in.rate < info->out.rate) {
		if (info->in.format == info->out.format && info->in.format == SND_PCM_FORMAT_S16)
			rate->func = linear_expand_s16;
		else
			rate->func = linear_expand;
		/* pitch is get_threshold */
	} else {
		if (info->in.format == info->out.format && info->in.format == SND_PCM_FORMAT_S16)
			rate->func = linear_shrink_s16;
		else
			rate->func = linear_shrink;
		/* pitch is get_increment */
	}
	rate->pitch = (((u_int64_t)info->out.rate * LINEAR_DIV) +
		       (info->in.rate / 2)) / info->in.rate;
	rate->channels = info->channels;

	free(rate->old_sample);
	rate->old_sample = malloc(sizeof(*rate->old_sample) * rate->channels);
	if (! rate->old_sample)
		return -ENOMEM;

	return 0;
}

static int linear_adjust_pitch(void *obj, snd_pcm_rate_info_t *info)
{
	struct rate_linear *rate = obj;
	snd_pcm_uframes_t cframes;

	rate->pitch = (((u_int64_t)info->out.period_size * LINEAR_DIV) +
		       (info->in.period_size/2) ) / info->in.period_size;
			
	cframes = input_frames(rate, info->out.period_size);
	while (cframes != info->in.period_size) {
		snd_pcm_uframes_t cframes_new;
		if (cframes > info->in.period_size)
			rate->pitch++;
		else
			rate->pitch--;
		cframes_new = input_frames(rate, info->out.period_size);
		if ((cframes > info->in.period_size && cframes_new < info->in.period_size) ||
		    (cframes < info->in.period_size && cframes_new > info->in.period_size)) {
			SNDERR("invalid pcm period_size %ld -> %ld",
			       info->in.period_size, info->out.period_size);
			return -EIO;
		}
		cframes = cframes_new;
	}
	if (rate->pitch >= LINEAR_DIV) {
		/* shift for expand linear interpolation */
		rate->pitch_shift = 0;
		while ((rate->pitch >> rate->pitch_shift) >= (1 << 16))
			rate->pitch_shift++;
	}
	return 0;
}

static void linear_reset(void *obj)
{
	struct rate_linear *rate = obj;

	/* for expand */
	if (rate->old_sample)
		memset(rate->old_sample, 0, sizeof(*rate->old_sample) * rate->channels);
}

static void linear_close(void *obj)
{
	free(obj);
}

static int get_supported_rates(ATTRIBUTE_UNUSED void *rate,
			       unsigned int *rate_min, unsigned int *rate_max)
{
	*rate_min = SND_PCM_PLUGIN_RATE_MIN;
	*rate_max = SND_PCM_PLUGIN_RATE_MAX;
	return 0;
}

static void linear_dump(ATTRIBUTE_UNUSED void *rate, snd_output_t *out)
{
	snd_output_printf(out, "Converter: linear-interpolation\n");
}

static const snd_pcm_rate_ops_t linear_ops = {
	.close = linear_close,
	.init = linear_init,
	.free = linear_free,
	.reset = linear_reset,
	.adjust_pitch = linear_adjust_pitch,
	.convert = linear_convert,
	.input_frames = input_frames,
	.output_frames = output_frames,
	.version = SND_PCM_RATE_PLUGIN_VERSION,
	.get_supported_rates = get_supported_rates,
	.dump = linear_dump,
};

int SND_PCM_RATE_PLUGIN_ENTRY(linear) (ATTRIBUTE_UNUSED unsigned int version,
				       void **objp, snd_pcm_rate_ops_t *ops)
{
	struct rate_linear *rate;

	rate = calloc(1, sizeof(*rate));
	if (! rate)
		return -ENOMEM;

	*objp = rate;
	*ops = linear_ops;
	return 0;
}
