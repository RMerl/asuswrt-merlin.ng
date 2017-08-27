/**
 * \file pcm/pcm_rate.c
 * \ingroup PCM_Plugins
 * \brief PCM Rate Plugin Interface
 * \author Abramo Bagnara <abramo@alsa-project.org>
 * \author Jaroslav Kysela <perex@perex.cz>
 * \date 2000-2004
 */
/*
 *  PCM - Rate conversion
 *  Copyright (c) 2000 by Abramo Bagnara <abramo@alsa-project.org>
 *                2004 by Jaroslav Kysela <perex@perex.cz>
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
#include <inttypes.h>
#include <byteswap.h>
#include "pcm_local.h"
#include "pcm_plugin.h"
#include "pcm_rate.h"
#include "iatomic.h"

#include "plugin_ops.h"


#ifndef PIC
/* entry for static linking */
const char *_snd_module_pcm_rate = "";
#endif

#ifndef DOC_HIDDEN

typedef struct _snd_pcm_rate snd_pcm_rate_t;

struct _snd_pcm_rate {
	snd_pcm_generic_t gen;
	snd_atomic_write_t watom;
	snd_pcm_uframes_t appl_ptr, hw_ptr;
	snd_pcm_uframes_t last_commit_ptr;
	snd_pcm_uframes_t orig_avail_min;
	snd_pcm_sw_params_t sw_params;
	snd_pcm_format_t sformat;
	unsigned int srate;
	snd_pcm_channel_area_t *pareas;	/* areas for splitted period (rate pcm) */
	snd_pcm_channel_area_t *sareas;	/* areas for splitted period (slave pcm) */
	snd_pcm_rate_info_t info;
	void *obj;
	snd_pcm_rate_ops_t ops;
	unsigned int get_idx;
	unsigned int put_idx;
	int16_t *src_buf;
	int16_t *dst_buf;
	int start_pending; /* start is triggered but not commited to slave */
	snd_htimestamp_t trigger_tstamp;
	unsigned int plugin_version;
	unsigned int rate_min, rate_max;
};

#define SND_PCM_RATE_PLUGIN_VERSION_OLD	0x010001	/* old rate plugin */

#endif /* DOC_HIDDEN */

static int snd_pcm_rate_hw_refine_cprepare(snd_pcm_t *pcm ATTRIBUTE_UNUSED, snd_pcm_hw_params_t *params)
{
	snd_pcm_rate_t *rate = pcm->private_data;
	int err;
	snd_pcm_access_mask_t access_mask = { SND_PCM_ACCBIT_SHM };
	snd_pcm_format_mask_t format_mask = { SND_PCM_FMTBIT_LINEAR };
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
	if (rate->rate_min) {
		err = _snd_pcm_hw_param_set_min(params, SND_PCM_HW_PARAM_RATE,
						rate->rate_min, 0);
		if (err < 0)
			return err;
	}
	if (rate->rate_max) {
		err = _snd_pcm_hw_param_set_max(params, SND_PCM_HW_PARAM_RATE,
						rate->rate_max, 0);
		if (err < 0)
			return err;
	}
	params->info &= ~(SND_PCM_INFO_MMAP | SND_PCM_INFO_MMAP_VALID);
	return 0;
}

static int snd_pcm_rate_hw_refine_sprepare(snd_pcm_t *pcm, snd_pcm_hw_params_t *sparams)
{
	snd_pcm_rate_t *rate = pcm->private_data;
	snd_pcm_access_mask_t saccess_mask = { SND_PCM_ACCBIT_MMAP };
	_snd_pcm_hw_params_any(sparams);
	_snd_pcm_hw_param_set_mask(sparams, SND_PCM_HW_PARAM_ACCESS,
				   &saccess_mask);
	if (rate->sformat != SND_PCM_FORMAT_UNKNOWN) {
		_snd_pcm_hw_params_set_format(sparams, rate->sformat);
		_snd_pcm_hw_params_set_subformat(sparams, SND_PCM_SUBFORMAT_STD);
	}
	_snd_pcm_hw_param_set_minmax(sparams, SND_PCM_HW_PARAM_RATE,
				     rate->srate, 0, rate->srate + 1, -1);
	return 0;
}

static int snd_pcm_rate_hw_refine_schange(snd_pcm_t *pcm, snd_pcm_hw_params_t *params,
					  snd_pcm_hw_params_t *sparams)
{
	snd_pcm_rate_t *rate = pcm->private_data;
	snd_interval_t t, buffer_size;
	const snd_interval_t *srate, *crate;
	int err;
	unsigned int links = (SND_PCM_HW_PARBIT_CHANNELS |
			      SND_PCM_HW_PARBIT_PERIOD_TIME |
			      SND_PCM_HW_PARBIT_TICK_TIME);
	if (rate->sformat == SND_PCM_FORMAT_UNKNOWN)
		links |= (SND_PCM_HW_PARBIT_FORMAT |
			  SND_PCM_HW_PARBIT_SUBFORMAT |
			  SND_PCM_HW_PARBIT_SAMPLE_BITS |
			  SND_PCM_HW_PARBIT_FRAME_BITS);
	snd_interval_copy(&buffer_size, snd_pcm_hw_param_get_interval(params, SND_PCM_HW_PARAM_BUFFER_SIZE));
	snd_interval_unfloor(&buffer_size);
	crate = snd_pcm_hw_param_get_interval(params, SND_PCM_HW_PARAM_RATE);
	srate = snd_pcm_hw_param_get_interval(sparams, SND_PCM_HW_PARAM_RATE);
	snd_interval_muldiv(&buffer_size, srate, crate, &t);
	err = _snd_pcm_hw_param_set_interval(sparams, SND_PCM_HW_PARAM_BUFFER_SIZE, &t);
	if (err < 0)
		return err;
	err = _snd_pcm_hw_params_refine(sparams, links, params);
	if (err < 0)
		return err;
	return 0;
}
	
static int snd_pcm_rate_hw_refine_cchange(snd_pcm_t *pcm, snd_pcm_hw_params_t *params,
					  snd_pcm_hw_params_t *sparams)
{
	snd_pcm_rate_t *rate = pcm->private_data;
	snd_interval_t t;
#ifdef DEBUG_REFINE
	snd_output_t *out;
#endif
	const snd_interval_t *sbuffer_size, *buffer_size;
	const snd_interval_t *srate, *crate;
	int err;
	unsigned int links = (SND_PCM_HW_PARBIT_CHANNELS |
			      SND_PCM_HW_PARBIT_PERIOD_TIME |
			      SND_PCM_HW_PARBIT_TICK_TIME);
	if (rate->sformat == SND_PCM_FORMAT_UNKNOWN)
		links |= (SND_PCM_HW_PARBIT_FORMAT |
			  SND_PCM_HW_PARBIT_SUBFORMAT |
			  SND_PCM_HW_PARBIT_SAMPLE_BITS |
			  SND_PCM_HW_PARBIT_FRAME_BITS);
	sbuffer_size = snd_pcm_hw_param_get_interval(sparams, SND_PCM_HW_PARAM_BUFFER_SIZE);
	crate = snd_pcm_hw_param_get_interval(params, SND_PCM_HW_PARAM_RATE);
	srate = snd_pcm_hw_param_get_interval(sparams, SND_PCM_HW_PARAM_RATE);
	snd_interval_muldiv(sbuffer_size, crate, srate, &t);
	snd_interval_floor(&t);
	err = _snd_pcm_hw_param_set_interval(params, SND_PCM_HW_PARAM_BUFFER_SIZE, &t);
	if (err < 0)
		return err;
	buffer_size = snd_pcm_hw_param_get_interval(params, SND_PCM_HW_PARAM_BUFFER_SIZE);
	/*
	 * this condition probably needs more work:
	 *   in case when the buffer_size is known and we are looking
	 *   for best period_size, we should prefer situation when
	 *   (buffer_size / period_size) * period_size == buffer_size
	 */
	if (snd_interval_single(buffer_size) && buffer_size->integer) {
		snd_interval_t *period_size;
		period_size = (snd_interval_t *)snd_pcm_hw_param_get_interval(params, SND_PCM_HW_PARAM_PERIOD_SIZE);
		if (!snd_interval_checkempty(period_size) &&
		    period_size->openmin && period_size->openmax &&
		    period_size->min + 1 == period_size->max) {
			if (period_size->min > 0 && (buffer_size->min / period_size->min) * period_size->min == buffer_size->min) {
		    		snd_interval_set_value(period_size, period_size->min);
		    	} else if ((buffer_size->max / period_size->max) * period_size->max == buffer_size->max) {
		    		snd_interval_set_value(period_size, period_size->max);
		    	}
		}
	}
#ifdef DEBUG_REFINE
	snd_output_stdio_attach(&out, stderr, 0);
	snd_output_printf(out, "REFINE (params):\n");
	snd_pcm_hw_params_dump(params, out);
	snd_output_printf(out, "REFINE (slave params):\n");
	snd_pcm_hw_params_dump(sparams, out);
	snd_output_close(out);
#endif
	err = _snd_pcm_hw_params_refine(params, links, sparams);
#ifdef DEBUG_REFINE
	snd_output_stdio_attach(&out, stderr, 0);
	snd_output_printf(out, "********************\n");
	snd_output_printf(out, "REFINE (params) (%i):\n", err);
	snd_pcm_hw_params_dump(params, out);
	snd_output_printf(out, "REFINE (slave params):\n");
	snd_pcm_hw_params_dump(sparams, out);
	snd_output_close(out);
#endif
	if (err < 0)
		return err;
	return 0;
}

static int snd_pcm_rate_hw_refine(snd_pcm_t *pcm, 
				  snd_pcm_hw_params_t *params)
{
	return snd_pcm_hw_refine_slave(pcm, params,
				       snd_pcm_rate_hw_refine_cprepare,
				       snd_pcm_rate_hw_refine_cchange,
				       snd_pcm_rate_hw_refine_sprepare,
				       snd_pcm_rate_hw_refine_schange,
				       snd_pcm_generic_hw_refine);
}

static int snd_pcm_rate_hw_params(snd_pcm_t *pcm, snd_pcm_hw_params_t * params)
{
	snd_pcm_rate_t *rate = pcm->private_data;
	snd_pcm_t *slave = rate->gen.slave;
	snd_pcm_rate_side_info_t *sinfo, *cinfo;
	unsigned int channels, cwidth, swidth, chn;
	int err = snd_pcm_hw_params_slave(pcm, params,
					  snd_pcm_rate_hw_refine_cchange,
					  snd_pcm_rate_hw_refine_sprepare,
					  snd_pcm_rate_hw_refine_schange,
					  snd_pcm_generic_hw_params);
	if (err < 0)
		return err;

	if (pcm->stream == SND_PCM_STREAM_PLAYBACK) {
		cinfo = &rate->info.in;
		sinfo = &rate->info.out;
	} else {
		sinfo = &rate->info.in;
		cinfo = &rate->info.out;
	}
	err = INTERNAL(snd_pcm_hw_params_get_format)(params, &cinfo->format);
	if (err < 0)
		return err;
	err = INTERNAL(snd_pcm_hw_params_get_rate)(params, &cinfo->rate, 0);
	if (err < 0)
		return err;
	err = INTERNAL(snd_pcm_hw_params_get_period_size)(params, &cinfo->period_size, 0);
	if (err < 0)
		return err;
	err = INTERNAL(snd_pcm_hw_params_get_buffer_size)(params, &cinfo->buffer_size);
	if (err < 0)
		return err;
	err = INTERNAL(snd_pcm_hw_params_get_channels)(params, &channels);
	if (err < 0)
		return err;

	rate->info.channels = channels;
	sinfo->format = slave->format;
	sinfo->rate = slave->rate;
	sinfo->buffer_size = slave->buffer_size;
	sinfo->period_size = slave->period_size;

	if (CHECK_SANITY(rate->pareas)) {
		SNDMSG("rate plugin already in use");
		return -EBUSY;
	}
	err = rate->ops.init(rate->obj, &rate->info);
	if (err < 0)
		return err;

	rate->pareas = malloc(2 * channels * sizeof(*rate->pareas));
	if (rate->pareas == NULL)
		goto error;

	cwidth = snd_pcm_format_physical_width(cinfo->format);
	swidth = snd_pcm_format_physical_width(sinfo->format);
	rate->pareas[0].addr = malloc(((cwidth * channels * cinfo->period_size) / 8) +
				      ((swidth * channels * sinfo->period_size) / 8));
	if (rate->pareas[0].addr == NULL)
		goto error;

	rate->sareas = rate->pareas + channels;
	rate->sareas[0].addr = (char *)rate->pareas[0].addr + ((cwidth * channels * cinfo->period_size) / 8);
	for (chn = 0; chn < channels; chn++) {
		rate->pareas[chn].addr = rate->pareas[0].addr + (cwidth * chn * cinfo->period_size) / 8;
		rate->pareas[chn].first = 0;
		rate->pareas[chn].step = cwidth;
		rate->sareas[chn].addr = rate->sareas[0].addr + (swidth * chn * sinfo->period_size) / 8;
		rate->sareas[chn].first = 0;
		rate->sareas[chn].step = swidth;
	}

	if (rate->ops.convert_s16) {
		rate->get_idx = snd_pcm_linear_get_index(rate->info.in.format, SND_PCM_FORMAT_S16);
		rate->put_idx = snd_pcm_linear_put_index(SND_PCM_FORMAT_S16, rate->info.out.format);
		free(rate->src_buf);
		rate->src_buf = malloc(channels * rate->info.in.period_size * 2);
		free(rate->dst_buf);
		rate->dst_buf = malloc(channels * rate->info.out.period_size * 2);
		if (! rate->src_buf || ! rate->dst_buf)
			goto error;
	}

	return 0;

 error:
	if (rate->pareas) {
		free(rate->pareas[0].addr);
		free(rate->pareas);
		rate->pareas = NULL;
	}
	if (rate->ops.free)
		rate->ops.free(rate->obj);
	return -ENOMEM;
}

static int snd_pcm_rate_hw_free(snd_pcm_t *pcm)
{
	snd_pcm_rate_t *rate = pcm->private_data;
	if (rate->pareas) {
		free(rate->pareas[0].addr);
		free(rate->pareas);
		rate->pareas = NULL;
		rate->sareas = NULL;
	}
	if (rate->ops.free)
		rate->ops.free(rate->obj);
	free(rate->src_buf);
	free(rate->dst_buf);
	rate->src_buf = rate->dst_buf = NULL;
	return snd_pcm_hw_free(rate->gen.slave);
}

static void recalc(snd_pcm_t *pcm, snd_pcm_uframes_t *val)
{
	snd_pcm_rate_t *rate = pcm->private_data;
	snd_pcm_t *slave = rate->gen.slave;
	unsigned long div;

	if (*val == pcm->buffer_size) {
		*val = slave->buffer_size;
	} else {
		div = *val / pcm->period_size;
		if (div * pcm->period_size == *val)
			*val = div * slave->period_size;
		else
			*val = muldiv_near(*val, slave->period_size, pcm->period_size);
	}
}

static int snd_pcm_rate_sw_params(snd_pcm_t *pcm, snd_pcm_sw_params_t * params)
{
	snd_pcm_rate_t *rate = pcm->private_data;
	snd_pcm_t *slave = rate->gen.slave;
	snd_pcm_sw_params_t *sparams;
	snd_pcm_uframes_t boundary1, boundary2, sboundary;
	int err;

	sparams = &rate->sw_params;
	err = snd_pcm_sw_params_current(slave, sparams);
	if (err < 0)
		return err;
	sboundary = sparams->boundary;
	*sparams = *params;
	boundary1 = pcm->buffer_size;
	boundary2 = slave->buffer_size;
	while (boundary1 * 2 <= LONG_MAX - pcm->buffer_size &&
	       boundary2 * 2 <= LONG_MAX - slave->buffer_size) {
		boundary1 *= 2;
		boundary2 *= 2;
	}
	params->boundary = boundary1;
	sparams->boundary = sboundary;

	if (rate->ops.adjust_pitch)
		rate->ops.adjust_pitch(rate->obj, &rate->info);

	recalc(pcm, &sparams->avail_min);
	rate->orig_avail_min = sparams->avail_min;
	recalc(pcm, &sparams->start_threshold);
	if (sparams->avail_min < 1) sparams->avail_min = 1;
	if (sparams->start_threshold <= slave->buffer_size) {
		if (sparams->start_threshold > (slave->buffer_size / sparams->avail_min) * sparams->avail_min)
			sparams->start_threshold = (slave->buffer_size / sparams->avail_min) * sparams->avail_min;
	}
	if (sparams->stop_threshold >= params->boundary) {
		sparams->stop_threshold = sparams->boundary;
	} else {
		recalc(pcm, &sparams->stop_threshold);
	}
	recalc(pcm, &sparams->silence_threshold);
	if (sparams->silence_size >= params->boundary) {
		sparams->silence_size = sparams->boundary;
	} else {
		recalc(pcm, &sparams->silence_size);
	}
	return snd_pcm_sw_params(slave, sparams);
}

static int snd_pcm_rate_init(snd_pcm_t *pcm)
{
	snd_pcm_rate_t *rate = pcm->private_data;

	if (rate->ops.reset)
		rate->ops.reset(rate->obj);
	rate->last_commit_ptr = 0;
	rate->start_pending = 0;
	return 0;
}

static void convert_to_s16(snd_pcm_rate_t *rate, int16_t *buf,
			   const snd_pcm_channel_area_t *areas,
			   snd_pcm_uframes_t offset, unsigned int frames,
			   unsigned int channels)
{
#ifndef DOC_HIDDEN
#define GET16_LABELS
#include "plugin_ops.h"
#undef GET16_LABELS
#endif /* DOC_HIDDEN */
	void *get = get16_labels[rate->get_idx];
	const char *src;
	int16_t sample;
	const char *srcs[channels];
	int src_step[channels];
	unsigned int c;

	for (c = 0; c < channels; c++) {
		srcs[c] = snd_pcm_channel_area_addr(areas + c, offset);
		src_step[c] = snd_pcm_channel_area_step(areas + c);
	}

	while (frames--) {
		for (c = 0; c < channels; c++) {
			src = srcs[c];
			goto *get;
#ifndef DOC_HIDDEN
#define GET16_END after_get
#include "plugin_ops.h"
#undef GET16_END
#endif /* DOC_HIDDEN */
		after_get:
			*buf++ = sample;
			srcs[c] += src_step[c];
		}
	}
}

static void convert_from_s16(snd_pcm_rate_t *rate, const int16_t *buf,
			     const snd_pcm_channel_area_t *areas,
			     snd_pcm_uframes_t offset, unsigned int frames,
			     unsigned int channels)
{
#ifndef DOC_HIDDEN
#define PUT16_LABELS
#include "plugin_ops.h"
#undef PUT16_LABELS
#endif /* DOC_HIDDEN */
	void *put = put16_labels[rate->put_idx];
	char *dst;
	int16_t sample;
	char *dsts[channels];
	int dst_step[channels];
	unsigned int c;

	for (c = 0; c < channels; c++) {
		dsts[c] = snd_pcm_channel_area_addr(areas + c, offset);
		dst_step[c] = snd_pcm_channel_area_step(areas + c);
	}

	while (frames--) {
		for (c = 0; c < channels; c++) {
			dst = dsts[c];
			sample = *buf++;
			goto *put;
#ifndef DOC_HIDDEN
#define PUT16_END after_put
#include "plugin_ops.h"
#undef PUT16_END
#endif /* DOC_HIDDEN */
		after_put:
			dsts[c] += dst_step[c];
		}
	}
}

static void do_convert(const snd_pcm_channel_area_t *dst_areas,
		       snd_pcm_uframes_t dst_offset, unsigned int dst_frames,
		       const snd_pcm_channel_area_t *src_areas,
		       snd_pcm_uframes_t src_offset, unsigned int src_frames,
		       unsigned int channels,
		       snd_pcm_rate_t *rate)
{
	if (rate->ops.convert_s16) {
		const int16_t *src;
		int16_t *dst;
		if (! rate->src_buf)
			src = src_areas->addr + src_offset * 2 * channels;
		else {
			convert_to_s16(rate, rate->src_buf, src_areas, src_offset,
				       src_frames, channels);
			src = rate->src_buf;
		}
		if (! rate->dst_buf)
			dst = dst_areas->addr + dst_offset * 2 * channels;
		else
			dst = rate->dst_buf;
		rate->ops.convert_s16(rate->obj, dst, dst_frames, src, src_frames);
		if (dst == rate->dst_buf)
			convert_from_s16(rate, rate->dst_buf, dst_areas, dst_offset,
					 dst_frames, channels);
	} else {
		rate->ops.convert(rate->obj, dst_areas, dst_offset, dst_frames,
				   src_areas, src_offset, src_frames);
	}
}

static inline void
snd_pcm_rate_write_areas1(snd_pcm_t *pcm,
			 const snd_pcm_channel_area_t *areas,
			 snd_pcm_uframes_t offset,
			 const snd_pcm_channel_area_t *slave_areas,
			 snd_pcm_uframes_t slave_offset)
{
	snd_pcm_rate_t *rate = pcm->private_data;
	do_convert(slave_areas, slave_offset, rate->gen.slave->period_size,
		   areas, offset, pcm->period_size,
		   pcm->channels, rate);
}

static inline void
snd_pcm_rate_read_areas1(snd_pcm_t *pcm,
			 const snd_pcm_channel_area_t *areas,
			 snd_pcm_uframes_t offset,
			 const snd_pcm_channel_area_t *slave_areas,
			 snd_pcm_uframes_t slave_offset)
{
	snd_pcm_rate_t *rate = pcm->private_data;
	do_convert(areas, offset, pcm->period_size,
		   slave_areas, slave_offset, rate->gen.slave->period_size,
		   pcm->channels, rate);
}

static inline snd_pcm_sframes_t snd_pcm_rate_move_applptr(snd_pcm_t *pcm, snd_pcm_sframes_t frames)
{
	snd_pcm_rate_t *rate = pcm->private_data;
	snd_pcm_uframes_t orig_appl_ptr, appl_ptr = rate->appl_ptr, slave_appl_ptr;
	snd_pcm_sframes_t diff, ndiff;
	snd_pcm_t *slave = rate->gen.slave;

	orig_appl_ptr = rate->appl_ptr;
	if (frames > 0)
		snd_pcm_mmap_appl_forward(pcm, frames);
	else
		snd_pcm_mmap_appl_backward(pcm, -frames);
	slave_appl_ptr =
		(appl_ptr / pcm->period_size) * rate->gen.slave->period_size;
	diff = slave_appl_ptr - *slave->appl.ptr;
	if (diff < -(snd_pcm_sframes_t)(slave->boundary / 2)) {
		diff = (slave->boundary - *slave->appl.ptr) + slave_appl_ptr;
	} else if (diff > (snd_pcm_sframes_t)(slave->boundary / 2)) {
		diff = -((slave->boundary - slave_appl_ptr) + *slave->appl.ptr);
	}
	if (diff == 0)
		return frames;
	if (diff > 0) {
		ndiff = snd_pcm_forward(rate->gen.slave, diff);
	} else {
		ndiff = snd_pcm_rewind(rate->gen.slave, diff);
	}
	if (ndiff < 0)
		return diff;
	slave_appl_ptr = *slave->appl.ptr;
	rate->appl_ptr =
		(slave_appl_ptr / rate->gen.slave->period_size) * pcm->period_size +
		orig_appl_ptr % pcm->period_size;
	if (pcm->stream == SND_PCM_STREAM_PLAYBACK)
		rate->appl_ptr += rate->ops.input_frames(rate->obj, slave_appl_ptr % rate->gen.slave->period_size);
	else
		rate->appl_ptr += rate->ops.output_frames(rate->obj, slave_appl_ptr % rate->gen.slave->period_size);

	diff = orig_appl_ptr - rate->appl_ptr;
	if (diff < -(snd_pcm_sframes_t)(slave->boundary / 2)) {
		diff = (slave->boundary - rate->appl_ptr) + orig_appl_ptr;
	} else if (diff > (snd_pcm_sframes_t)(slave->boundary / 2)) {
		diff = -((slave->boundary - orig_appl_ptr) + rate->appl_ptr);
	}
	if (frames < 0)
		diff = -diff;

	rate->last_commit_ptr = rate->appl_ptr - rate->appl_ptr % pcm->period_size;

	return diff;
}

static inline void snd_pcm_rate_sync_hwptr(snd_pcm_t *pcm)
{
	snd_pcm_rate_t *rate = pcm->private_data;
	snd_pcm_uframes_t slave_hw_ptr = *rate->gen.slave->hw.ptr;

	if (pcm->stream != SND_PCM_STREAM_PLAYBACK)
		return;
	rate->hw_ptr =
		(slave_hw_ptr / rate->gen.slave->period_size) * pcm->period_size +
		rate->ops.input_frames(rate->obj, slave_hw_ptr % rate->gen.slave->period_size);
}

static int snd_pcm_rate_hwsync(snd_pcm_t *pcm)
{
	snd_pcm_rate_t *rate = pcm->private_data;
	int err = snd_pcm_hwsync(rate->gen.slave);
	if (err < 0)
		return err;
	snd_atomic_write_begin(&rate->watom);
	snd_pcm_rate_sync_hwptr(pcm);
	snd_atomic_write_end(&rate->watom);
	return 0;
}

static int snd_pcm_rate_delay(snd_pcm_t *pcm, snd_pcm_sframes_t *delayp)
{
	snd_pcm_rate_hwsync(pcm);
	if (pcm->stream == SND_PCM_STREAM_PLAYBACK)
		*delayp = snd_pcm_mmap_playback_hw_avail(pcm);
	else
		*delayp = snd_pcm_mmap_capture_hw_avail(pcm);
	return 0;
}

static int snd_pcm_rate_prepare(snd_pcm_t *pcm)
{
	snd_pcm_rate_t *rate = pcm->private_data;
	int err;

	snd_atomic_write_begin(&rate->watom);
	err = snd_pcm_prepare(rate->gen.slave);
	if (err < 0) {
		snd_atomic_write_end(&rate->watom);
		return err;
	}
	*pcm->hw.ptr = 0;
	*pcm->appl.ptr = 0;
	snd_atomic_write_end(&rate->watom);
	err = snd_pcm_rate_init(pcm);
	if (err < 0)
		return err;
	return 0;
}

static int snd_pcm_rate_reset(snd_pcm_t *pcm)
{
	snd_pcm_rate_t *rate = pcm->private_data;
	int err;
	snd_atomic_write_begin(&rate->watom);
	err = snd_pcm_reset(rate->gen.slave);
	if (err < 0) {
		snd_atomic_write_end(&rate->watom);
		return err;
	}
	*pcm->hw.ptr = 0;
	*pcm->appl.ptr = 0;
	snd_atomic_write_end(&rate->watom);
	err = snd_pcm_rate_init(pcm);
	if (err < 0)
		return err;
	return 0;
}

static snd_pcm_sframes_t snd_pcm_rate_rewind(snd_pcm_t *pcm, snd_pcm_uframes_t frames)
{
	snd_pcm_rate_t *rate = pcm->private_data;
	snd_pcm_sframes_t n = snd_pcm_mmap_hw_avail(pcm);

	if ((snd_pcm_uframes_t)n > frames)
		frames = n;
	if (frames == 0)
		return 0;
	
	snd_atomic_write_begin(&rate->watom);
	n = snd_pcm_rate_move_applptr(pcm, -frames);
	snd_atomic_write_end(&rate->watom);
	return n;
}

static snd_pcm_sframes_t snd_pcm_rate_forward(snd_pcm_t *pcm, snd_pcm_uframes_t frames)
{
	snd_pcm_rate_t *rate = pcm->private_data;
	snd_pcm_sframes_t n = snd_pcm_mmap_avail(pcm);

	if ((snd_pcm_uframes_t)n > frames)
		frames = n;
	if (frames == 0)
		return 0;
	
	snd_atomic_write_begin(&rate->watom);
	n = snd_pcm_rate_move_applptr(pcm, frames);
	snd_atomic_write_end(&rate->watom);
	return n;
}

static int snd_pcm_rate_commit_area(snd_pcm_t *pcm, snd_pcm_rate_t *rate,
				    snd_pcm_uframes_t appl_offset,
				    snd_pcm_uframes_t size,
				    snd_pcm_uframes_t slave_size)
{
	snd_pcm_uframes_t cont = pcm->buffer_size - appl_offset;
	const snd_pcm_channel_area_t *areas;
	const snd_pcm_channel_area_t *slave_areas;
	snd_pcm_uframes_t slave_offset, xfer;
	snd_pcm_uframes_t slave_frames = ULONG_MAX;
	snd_pcm_sframes_t result;

	areas = snd_pcm_mmap_areas(pcm);
	if (cont >= size) {
		result = snd_pcm_mmap_begin(rate->gen.slave, &slave_areas, &slave_offset, &slave_frames);
		if (result < 0)
			return result;
		if (slave_frames < slave_size) {
			snd_pcm_rate_write_areas1(pcm, areas, appl_offset, rate->sareas, 0);
			goto __partial;
		}
		snd_pcm_rate_write_areas1(pcm, areas, appl_offset,
					  slave_areas, slave_offset);
		result = snd_pcm_mmap_commit(rate->gen.slave, slave_offset, slave_size);
		if (result < (snd_pcm_sframes_t)slave_size) {
			if (result < 0)
				return result;
			result = snd_pcm_rewind(rate->gen.slave, result);
			if (result < 0)
				return result;
			return 0;
		}
	} else {
		snd_pcm_areas_copy(rate->pareas, 0,
				   areas, appl_offset,
				   pcm->channels, cont,
				   pcm->format);
		snd_pcm_areas_copy(rate->pareas, cont,
				   areas, 0,
				   pcm->channels, size - cont,
				   pcm->format);

		snd_pcm_rate_write_areas1(pcm, rate->pareas, 0, rate->sareas, 0);

		/* ok, commit first fragment */
		result = snd_pcm_mmap_begin(rate->gen.slave, &slave_areas, &slave_offset, &slave_frames);
		if (result < 0)
			return result;
	      __partial:
		xfer = 0;
		cont = slave_frames;
		if (cont > slave_size)
			cont = slave_size;
		snd_pcm_areas_copy(slave_areas, slave_offset,
				   rate->sareas, 0,
				   pcm->channels, cont,
				   rate->gen.slave->format);
		result = snd_pcm_mmap_commit(rate->gen.slave, slave_offset, cont);
		if (result < (snd_pcm_sframes_t)cont) {
			if (result < 0)
				return result;
			result = snd_pcm_rewind(rate->gen.slave, result);
			if (result < 0)
				return result;
			return 0;
		}
		xfer = cont;

		if (xfer == slave_size)
			goto commit_done;
		
		/* commit second fragment */
		cont = slave_size - cont;
		slave_frames = cont;
		result = snd_pcm_mmap_begin(rate->gen.slave, &slave_areas, &slave_offset, &slave_frames);
		if (result < 0)
			return result;
		snd_pcm_areas_copy(slave_areas, slave_offset,
				   rate->sareas, xfer,
				   pcm->channels, cont,
				   rate->gen.slave->format);
		result = snd_pcm_mmap_commit(rate->gen.slave, slave_offset, cont);
		if (result < (snd_pcm_sframes_t)cont) {
			if (result < 0)
				return result;
			result = snd_pcm_rewind(rate->gen.slave, result + xfer);
			if (result < 0)
				return result;
			return 0;
		}
	}

 commit_done:
	if (rate->start_pending) {
		/* we have pending start-trigger.  let's issue it now */
		snd_pcm_start(rate->gen.slave);
		rate->start_pending = 0;
	}
	return 1;
}

static int snd_pcm_rate_commit_next_period(snd_pcm_t *pcm, snd_pcm_uframes_t appl_offset)
{
	snd_pcm_rate_t *rate = pcm->private_data;

	return snd_pcm_rate_commit_area(pcm, rate, appl_offset, pcm->period_size,
					rate->gen.slave->period_size);
}

static int snd_pcm_rate_grab_next_period(snd_pcm_t *pcm, snd_pcm_uframes_t hw_offset)
{
	snd_pcm_rate_t *rate = pcm->private_data;
	snd_pcm_uframes_t cont = pcm->buffer_size - hw_offset;
	const snd_pcm_channel_area_t *areas;
	const snd_pcm_channel_area_t *slave_areas;
	snd_pcm_uframes_t slave_offset, xfer;
	snd_pcm_uframes_t slave_frames = ULONG_MAX;
	snd_pcm_sframes_t result;

	areas = snd_pcm_mmap_areas(pcm);
	if (cont >= pcm->period_size) {
		result = snd_pcm_mmap_begin(rate->gen.slave, &slave_areas, &slave_offset, &slave_frames);
		if (result < 0)
			return result;
		if (slave_frames < rate->gen.slave->period_size)
			goto __partial;
		snd_pcm_rate_read_areas1(pcm, areas, hw_offset,
					 slave_areas, slave_offset);
		result = snd_pcm_mmap_commit(rate->gen.slave, slave_offset, rate->gen.slave->period_size);
		if (result < (snd_pcm_sframes_t)rate->gen.slave->period_size) {
			if (result < 0)
				return result;
			result = snd_pcm_rewind(rate->gen.slave, result);
			if (result < 0)
				return result;
			return 0;
		}
	} else {
		/* ok, grab first fragment */
		result = snd_pcm_mmap_begin(rate->gen.slave, &slave_areas, &slave_offset, &slave_frames);
		if (result < 0)
			return result;
	      __partial:
		xfer = 0;
		cont = slave_frames;
		if (cont > rate->gen.slave->period_size)
			cont = rate->gen.slave->period_size;
		snd_pcm_areas_copy(rate->sareas, 0,
				   slave_areas, slave_offset,
				   pcm->channels, cont,
				   rate->gen.slave->format);
		result = snd_pcm_mmap_commit(rate->gen.slave, slave_offset, cont);
		if (result < (snd_pcm_sframes_t)cont) {
			if (result < 0)
				return result;
			result = snd_pcm_rewind(rate->gen.slave, result);
			if (result < 0)
				return result;
			return 0;
		}
		xfer = cont;

		if (xfer == rate->gen.slave->period_size)
			goto __transfer;

		/* grab second fragment */
		cont = rate->gen.slave->period_size - cont;
		slave_frames = cont;
		result = snd_pcm_mmap_begin(rate->gen.slave, &slave_areas, &slave_offset, &slave_frames);
		if (result < 0)
			return result;
		snd_pcm_areas_copy(rate->sareas, xfer,
		                   slave_areas, slave_offset,
				   pcm->channels, cont,
				   rate->gen.slave->format);
		result = snd_pcm_mmap_commit(rate->gen.slave, slave_offset, cont);
		if (result < (snd_pcm_sframes_t)cont) {
			if (result < 0)
				return result;
			result = snd_pcm_rewind(rate->gen.slave, result + xfer);
			if (result < 0)
				return result;
			return 0;
		}

	      __transfer:
		cont = pcm->buffer_size - hw_offset;
		if (cont >= pcm->period_size) {
			snd_pcm_rate_read_areas1(pcm, areas, hw_offset,
						 rate->sareas, 0);
		} else {
			snd_pcm_rate_read_areas1(pcm,
						 rate->pareas, 0,
						 rate->sareas, 0);
			snd_pcm_areas_copy(areas, hw_offset,
					   rate->pareas, 0,
					   pcm->channels, cont,
					   pcm->format);
			snd_pcm_areas_copy(areas, 0,
					   rate->pareas, cont,
					   pcm->channels, pcm->period_size - cont,
					   pcm->format);
		}
	}
	return 1;
}

static int snd_pcm_rate_sync_playback_area(snd_pcm_t *pcm, snd_pcm_uframes_t appl_ptr)
{
	snd_pcm_rate_t *rate = pcm->private_data;
	snd_pcm_t *slave = rate->gen.slave;
	snd_pcm_uframes_t xfer;
	snd_pcm_sframes_t slave_size;
	int err;

	slave_size = snd_pcm_avail_update(slave);
	if (slave_size < 0)
		return slave_size;

	if (appl_ptr < rate->last_commit_ptr)
		xfer = appl_ptr - rate->last_commit_ptr + pcm->boundary;
	else
		xfer = appl_ptr - rate->last_commit_ptr;
	while (xfer >= pcm->period_size &&
	       (snd_pcm_uframes_t)slave_size >= rate->gen.slave->period_size) {
		err = snd_pcm_rate_commit_next_period(pcm, rate->last_commit_ptr % pcm->buffer_size);
		if (err == 0)
			break;
		if (err < 0)
			return err;
		xfer -= pcm->period_size;
		slave_size -= rate->gen.slave->period_size;
		rate->last_commit_ptr += pcm->period_size;
		if (rate->last_commit_ptr >= pcm->boundary)
			rate->last_commit_ptr = 0;
	}
	return 0;
}

static snd_pcm_sframes_t snd_pcm_rate_mmap_commit(snd_pcm_t *pcm,
						  snd_pcm_uframes_t offset ATTRIBUTE_UNUSED,
						  snd_pcm_uframes_t size)
{
	snd_pcm_rate_t *rate = pcm->private_data;
	int err;

	if (size == 0)
		return 0;
	if (pcm->stream == SND_PCM_STREAM_PLAYBACK) {
		err = snd_pcm_rate_sync_playback_area(pcm, rate->appl_ptr + size);
		if (err < 0)
			return err;
	}
	snd_atomic_write_begin(&rate->watom);
	snd_pcm_mmap_appl_forward(pcm, size);
	snd_atomic_write_end(&rate->watom);
	return size;
}

static snd_pcm_sframes_t snd_pcm_rate_avail_update(snd_pcm_t *pcm)
{
	snd_pcm_rate_t *rate = pcm->private_data;
	snd_pcm_t *slave = rate->gen.slave;
	snd_pcm_uframes_t slave_size;

	slave_size = snd_pcm_avail_update(slave);
	if (pcm->stream == SND_PCM_STREAM_CAPTURE)
		goto _capture;
	snd_atomic_write_begin(&rate->watom);
	snd_pcm_rate_sync_hwptr(pcm);
	snd_atomic_write_end(&rate->watom);
	snd_pcm_rate_sync_playback_area(pcm, rate->appl_ptr);
	return snd_pcm_mmap_avail(pcm);
 _capture: {
	snd_pcm_uframes_t xfer, hw_offset, size;
	
	xfer = snd_pcm_mmap_capture_avail(pcm);
	size = pcm->buffer_size - xfer;
	hw_offset = snd_pcm_mmap_hw_offset(pcm);
	while (size >= pcm->period_size &&
	       slave_size >= rate->gen.slave->period_size) {
		int err = snd_pcm_rate_grab_next_period(pcm, hw_offset);
		if (err < 0)
			return err;
		if (err == 0)
			return (snd_pcm_sframes_t)xfer;
		xfer += pcm->period_size;
		size -= pcm->period_size;
		slave_size -= rate->gen.slave->period_size;
		hw_offset += pcm->period_size;
		hw_offset %= pcm->buffer_size;
		snd_pcm_mmap_hw_forward(pcm, pcm->period_size);
	}
	return (snd_pcm_sframes_t)xfer;
 }
}

static int snd_pcm_rate_htimestamp(snd_pcm_t *pcm,
				   snd_pcm_uframes_t *avail,
				   snd_htimestamp_t *tstamp)
{
	snd_pcm_rate_t *rate = pcm->private_data;
	snd_pcm_sframes_t avail1;
	snd_pcm_uframes_t tmp;
	int ok = 0, err;

	while (1) {
		/* the position is from this plugin itself */
		avail1 = snd_pcm_avail_update(pcm);
		if (avail1 < 0)
			return avail1;
		if (ok && (snd_pcm_uframes_t)avail1 == *avail)
			break;
		*avail = avail1;
		/* timestamp is taken from the slave PCM */
		err = snd_pcm_htimestamp(rate->gen.slave, &tmp, tstamp);
		if (err < 0)
			return err;
		ok = 1;
	}
	return 0;
}

static int snd_pcm_rate_poll_revents(snd_pcm_t *pcm, struct pollfd *pfds, unsigned int nfds, unsigned short *revents)
{
	snd_pcm_rate_t *rate = pcm->private_data;
	if (pcm->stream == SND_PCM_STREAM_PLAYBACK) {
		/* Try to sync as much as possible */
		snd_pcm_rate_hwsync(pcm);
		snd_pcm_rate_sync_playback_area(pcm, rate->appl_ptr);
	}
	return snd_pcm_poll_descriptors_revents(rate->gen.slave, pfds, nfds, revents);
}

static int snd_pcm_rate_drain(snd_pcm_t *pcm)
{
	snd_pcm_rate_t *rate = pcm->private_data;

	if (pcm->stream == SND_PCM_STREAM_PLAYBACK) {
		/* commit the remaining fraction (if any) */
		snd_pcm_uframes_t size, ofs, saved_avail_min;
		snd_pcm_sw_params_t sw_params;

		/* temporarily set avail_min to one */
		sw_params = rate->sw_params;
		saved_avail_min = sw_params.avail_min;
		sw_params.avail_min = 1;
		snd_pcm_sw_params(rate->gen.slave, &sw_params);

		size = rate->appl_ptr - rate->last_commit_ptr;
		ofs = rate->last_commit_ptr % pcm->buffer_size;
		while (size > 0) {
			snd_pcm_uframes_t psize, spsize;

			if (snd_pcm_wait(rate->gen.slave, -1) < 0)
				break;
			if (size > pcm->period_size) {
				psize = pcm->period_size;
				spsize = rate->gen.slave->period_size;
			} else {
				psize = size;
				spsize = rate->ops.output_frames(rate->obj, size);
				if (! spsize)
					break;
			}
			snd_pcm_rate_commit_area(pcm, rate, ofs,
						 psize, spsize);
			ofs = (ofs + psize) % pcm->buffer_size;
			size -= psize;
		}
		sw_params.avail_min = saved_avail_min;
		snd_pcm_sw_params(rate->gen.slave, &sw_params);
	}
	return snd_pcm_drain(rate->gen.slave);
}

static snd_pcm_state_t snd_pcm_rate_state(snd_pcm_t *pcm)
{
	snd_pcm_rate_t *rate = pcm->private_data;
	if (rate->start_pending) /* pseudo-state */
		return SND_PCM_STATE_RUNNING;
	return snd_pcm_state(rate->gen.slave);
}


static int snd_pcm_rate_start(snd_pcm_t *pcm)
{
	snd_pcm_rate_t *rate = pcm->private_data;
	snd_pcm_uframes_t avail;
		
	if (pcm->stream == SND_PCM_STREAM_CAPTURE)
		return snd_pcm_start(rate->gen.slave);

	if (snd_pcm_state(rate->gen.slave) != SND_PCM_STATE_PREPARED)
		return -EBADFD;

	gettimestamp(&rate->trigger_tstamp, pcm->monotonic);

	avail = snd_pcm_mmap_playback_hw_avail(rate->gen.slave);
	if (avail == 0) {
		/* postpone the trigger since we have no data committed yet */
		rate->start_pending = 1;
		return 0;
	}
	rate->start_pending = 0;
	return snd_pcm_start(rate->gen.slave);
}

static int snd_pcm_rate_status(snd_pcm_t *pcm, snd_pcm_status_t * status)
{
	snd_pcm_rate_t *rate = pcm->private_data;
	snd_pcm_sframes_t err;
	snd_atomic_read_t ratom;
	snd_atomic_read_init(&ratom, &rate->watom);
 _again:
	snd_atomic_read_begin(&ratom);
	err = snd_pcm_status(rate->gen.slave, status);
	if (err < 0) {
		snd_atomic_read_ok(&ratom);
		return err;
	}
	if (pcm->stream == SND_PCM_STREAM_PLAYBACK) {
		if (rate->start_pending)
			status->state = SND_PCM_STATE_RUNNING;
		status->trigger_tstamp = rate->trigger_tstamp;
	}
	snd_pcm_rate_sync_hwptr(pcm);
	status->appl_ptr = *pcm->appl.ptr;
	status->hw_ptr = *pcm->hw.ptr;
	if (pcm->stream == SND_PCM_STREAM_PLAYBACK) {
		status->delay = snd_pcm_mmap_playback_hw_avail(pcm);
		status->avail = snd_pcm_mmap_playback_avail(pcm);
		status->avail_max = rate->ops.input_frames(rate->obj, status->avail_max);
	} else {
		status->delay = snd_pcm_mmap_capture_hw_avail(pcm);
		status->avail = snd_pcm_mmap_capture_avail(pcm);
		status->avail_max = rate->ops.output_frames(rate->obj, status->avail_max);
	}
	if (!snd_atomic_read_ok(&ratom)) {
		snd_atomic_read_wait(&ratom);
		goto _again;
	}
	return 0;
}

static void snd_pcm_rate_dump(snd_pcm_t *pcm, snd_output_t *out)
{
	snd_pcm_rate_t *rate = pcm->private_data;
	if (rate->sformat == SND_PCM_FORMAT_UNKNOWN)
		snd_output_printf(out, "Rate conversion PCM (%d)\n", 
			rate->srate);
	else
		snd_output_printf(out, "Rate conversion PCM (%d, sformat=%s)\n", 
			rate->srate,
			snd_pcm_format_name(rate->sformat));
	if (rate->ops.dump)
		rate->ops.dump(rate->obj, out);
	snd_output_printf(out, "Protocol version: %x\n", rate->plugin_version);
	if (pcm->setup) {
		snd_output_printf(out, "Its setup is:\n");
		snd_pcm_dump_setup(pcm, out);
	}
	snd_output_printf(out, "Slave: ");
	snd_pcm_dump(rate->gen.slave, out);
}

static int snd_pcm_rate_close(snd_pcm_t *pcm)
{
	snd_pcm_rate_t *rate = pcm->private_data;

	if (rate->ops.close)
		rate->ops.close(rate->obj);
	return snd_pcm_generic_close(pcm);
}

static const snd_pcm_fast_ops_t snd_pcm_rate_fast_ops = {
	.status = snd_pcm_rate_status,
	.state = snd_pcm_rate_state,
	.hwsync = snd_pcm_rate_hwsync,
	.delay = snd_pcm_rate_delay,
	.prepare = snd_pcm_rate_prepare,
	.reset = snd_pcm_rate_reset,
	.start = snd_pcm_rate_start,
	.drop = snd_pcm_generic_drop,
	.drain = snd_pcm_rate_drain,
	.pause = snd_pcm_generic_pause,
	.rewind = snd_pcm_rate_rewind,
	.forward = snd_pcm_rate_forward,
	.resume = snd_pcm_generic_resume,
	.writei = snd_pcm_mmap_writei,
	.writen = snd_pcm_mmap_writen,
	.readi = snd_pcm_mmap_readi,
	.readn = snd_pcm_mmap_readn,
	.avail_update = snd_pcm_rate_avail_update,
	.mmap_commit = snd_pcm_rate_mmap_commit,
	.htimestamp = snd_pcm_rate_htimestamp,
	.poll_descriptors_count = snd_pcm_generic_poll_descriptors_count,
	.poll_descriptors = snd_pcm_generic_poll_descriptors,
	.poll_revents = snd_pcm_rate_poll_revents,
};

static const snd_pcm_ops_t snd_pcm_rate_ops = {
	.close = snd_pcm_rate_close,
	.info = snd_pcm_generic_info,
	.hw_refine = snd_pcm_rate_hw_refine,
	.hw_params = snd_pcm_rate_hw_params,
	.hw_free = snd_pcm_rate_hw_free,
	.sw_params = snd_pcm_rate_sw_params,
	.channel_info = snd_pcm_generic_channel_info,
	.dump = snd_pcm_rate_dump,
	.nonblock = snd_pcm_generic_nonblock,
	.async = snd_pcm_generic_async,
	.mmap = snd_pcm_generic_mmap,
	.munmap = snd_pcm_generic_munmap,
};

/**
 * \brief Get a default converter string
 * \param root Root configuration node
 * \retval A const config item if found, or NULL
 */
const snd_config_t *snd_pcm_rate_get_default_converter(snd_config_t *root)
{
	snd_config_t *n;
	/* look for default definition */
	if (snd_config_search(root, "defaults.pcm.rate_converter", &n) >= 0)
		return n;
	return NULL;
}

#ifdef PIC
static int is_builtin_plugin(const char *type)
{
	return strcmp(type, "linear") == 0;
}

static const char *const default_rate_plugins[] = {
	"speexrate", "linear", NULL
};

static int rate_open_func(snd_pcm_rate_t *rate, const char *type)
{
	char open_name[64];
	snd_pcm_rate_open_func_t open_func;
	int err;

	snprintf(open_name, sizeof(open_name), "_snd_pcm_rate_%s_open", type);
	open_func = snd_dlobj_cache_lookup(open_name);
	if (!open_func) {
		void *h;
		char lib_name[128], *lib = NULL;
		if (!is_builtin_plugin(type)) {
			snprintf(lib_name, sizeof(lib_name),
				 "%s/libasound_module_rate_%s.so", ALSA_PLUGIN_DIR, type);
			lib = lib_name;
		}
		h = snd_dlopen(lib, RTLD_NOW);
		if (!h)
			return -ENOENT;
		open_func = snd_dlsym(h, open_name, NULL);
		if (!open_func) {
			snd_dlclose(h);
			return -ENOENT;
		}
		snd_dlobj_cache_add(open_name, h, open_func);
	}

	rate->rate_min = SND_PCM_PLUGIN_RATE_MIN;
	rate->rate_max = SND_PCM_PLUGIN_RATE_MAX;
	rate->plugin_version = SND_PCM_RATE_PLUGIN_VERSION;

	err = open_func(SND_PCM_RATE_PLUGIN_VERSION, &rate->obj, &rate->ops);
	if (!err) {
		rate->plugin_version = rate->ops.version;
		if (rate->ops.get_supported_rates)
			rate->ops.get_supported_rates(rate->obj,
						      &rate->rate_min,
						      &rate->rate_max);
		return 0;
	}

	/* try to open with the old protocol version */
	rate->plugin_version = SND_PCM_RATE_PLUGIN_VERSION_OLD;
	return open_func(SND_PCM_RATE_PLUGIN_VERSION_OLD,
			 &rate->obj, &rate->ops);
}
#endif

/**
 * \brief Creates a new rate PCM
 * \param pcmp Returns created PCM handle
 * \param name Name of PCM
 * \param sformat Slave format
 * \param srate Slave rate
 * \param converter SRC type string node
 * \param slave Slave PCM handle
 * \param close_slave When set, the slave PCM handle is closed with copy PCM
 * \retval zero on success otherwise a negative error code
 * \warning Using of this function might be dangerous in the sense
 *          of compatibility reasons. The prototype might be freely
 *          changed in future.
 */
int snd_pcm_rate_open(snd_pcm_t **pcmp, const char *name,
		      snd_pcm_format_t sformat, unsigned int srate,
		      const snd_config_t *converter,
		      snd_pcm_t *slave, int close_slave)
{
	snd_pcm_t *pcm;
	snd_pcm_rate_t *rate;
	const char *type = NULL;
	int err;
#ifndef PIC
	snd_pcm_rate_open_func_t open_func;
	extern int SND_PCM_RATE_PLUGIN_ENTRY(linear) (unsigned int version, void **objp, snd_pcm_rate_ops_t *ops);
#endif

	assert(pcmp && slave);
	if (sformat != SND_PCM_FORMAT_UNKNOWN &&
	    snd_pcm_format_linear(sformat) != 1)
		return -EINVAL;
	rate = calloc(1, sizeof(snd_pcm_rate_t));
	if (!rate) {
		return -ENOMEM;
	}
	rate->gen.slave = slave;
	rate->gen.close_slave = close_slave;
	rate->srate = srate;
	rate->sformat = sformat;
	snd_atomic_write_init(&rate->watom);

	err = snd_pcm_new(&pcm, SND_PCM_TYPE_RATE, name, slave->stream, slave->mode);
	if (err < 0) {
		free(rate);
		return err;
	}

#ifdef PIC
	err = -ENOENT;
	if (!converter) {
		const char *const *types;
		for (types = default_rate_plugins; *types; types++) {
			err = rate_open_func(rate, *types);
			if (!err) {
				type = *types;
				break;
			}
		}
	} else if (!snd_config_get_string(converter, &type))
		err = rate_open_func(rate, type);
	else if (snd_config_get_type(converter) == SND_CONFIG_TYPE_COMPOUND) {
		snd_config_iterator_t i, next;
		snd_config_for_each(i, next, converter) {
			snd_config_t *n = snd_config_iterator_entry(i);
			if (snd_config_get_string(n, &type) < 0)
				break;
			err = rate_open_func(rate, type);
			if (!err)
				break;
		}
	} else {
		SNDERR("Invalid type for rate converter");
		snd_pcm_close(pcm);
		return -EINVAL;
	}
	if (err < 0) {
		SNDERR("Cannot find rate converter");
		snd_pcm_close(pcm);
		return -ENOENT;
	}
#else
	type = "linear";
	open_func = SND_PCM_RATE_PLUGIN_ENTRY(linear);
	err = open_func(SND_PCM_RATE_PLUGIN_VERSION, &rate->obj, &rate->ops);
	if (err < 0) {
		snd_pcm_close(pcm);
		return err;
	}
#endif

	if (! rate->ops.init || ! (rate->ops.convert || rate->ops.convert_s16) ||
	    ! rate->ops.input_frames || ! rate->ops.output_frames) {
		SNDERR("Inproper rate plugin %s initialization", type);
		snd_pcm_close(pcm);
		return err;
	}

	pcm->ops = &snd_pcm_rate_ops;
	pcm->fast_ops = &snd_pcm_rate_fast_ops;
	pcm->private_data = rate;
	pcm->poll_fd = slave->poll_fd;
	pcm->poll_events = slave->poll_events;
	pcm->mmap_rw = 1;
	pcm->monotonic = slave->monotonic;
	snd_pcm_set_hw_ptr(pcm, &rate->hw_ptr, -1, 0);
	snd_pcm_set_appl_ptr(pcm, &rate->appl_ptr, -1, 0);
	*pcmp = pcm;

	return 0;
}

/*! \page pcm_plugins

\section pcm_plugins_rate Plugin: Rate

This plugin converts a stream rate. The input and output formats must be linear.

\code
pcm.name {
	type rate               # Rate PCM
        slave STR               # Slave name
        # or
        slave {                 # Slave definition
                pcm STR         # Slave PCM name
                # or
                pcm { }         # Slave PCM definition
                rate INT        # Slave rate
                [format STR]    # Slave format
        }
	converter STR			# optional
	# or
	converter [ STR1 STR2 ... ]	# optional
				# Converter type, default is taken from
				# defaults.pcm.rate_converter
}
\endcode

\subsection pcm_plugins_rate_funcref Function reference

<UL>
  <LI>snd_pcm_rate_open()
  <LI>_snd_pcm_rate_open()
</UL>

*/

/**
 * \brief Creates a new rate PCM
 * \param pcmp Returns created PCM handle
 * \param name Name of PCM
 * \param root Root configuration node
 * \param conf Configuration node with rate PCM description
 * \param stream Stream type
 * \param mode Stream mode
 * \retval zero on success otherwise a negative error code
 * \warning Using of this function might be dangerous in the sense
 *          of compatibility reasons. The prototype might be freely
 *          changed in future.
 */
int _snd_pcm_rate_open(snd_pcm_t **pcmp, const char *name,
		       snd_config_t *root, snd_config_t *conf, 
		       snd_pcm_stream_t stream, int mode)
{
	snd_config_iterator_t i, next;
	int err;
	snd_pcm_t *spcm;
	snd_config_t *slave = NULL, *sconf;
	snd_pcm_format_t sformat = SND_PCM_FORMAT_UNKNOWN;
	int srate = -1;
	const snd_config_t *converter = NULL;

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
		if (strcmp(id, "converter") == 0) {
			converter = n;
			continue;
		}
		SNDERR("Unknown field %s", id);
		return -EINVAL;
	}
	if (!slave) {
		SNDERR("slave is not defined");
		return -EINVAL;
	}

	err = snd_pcm_slave_conf(root, slave, &sconf, 2,
				 SND_PCM_HW_PARAM_FORMAT, 0, &sformat,
				 SND_PCM_HW_PARAM_RATE, SCONF_MANDATORY, &srate);
	if (err < 0)
		return err;
	if (sformat != SND_PCM_FORMAT_UNKNOWN &&
	    snd_pcm_format_linear(sformat) != 1) {
	    	snd_config_delete(sconf);
		SNDERR("slave format is not linear");
		return -EINVAL;
	}
	err = snd_pcm_open_slave(&spcm, root, sconf, stream, mode, conf);
	snd_config_delete(sconf);
	if (err < 0)
		return err;
	err = snd_pcm_rate_open(pcmp, name, sformat, (unsigned int) srate,
				converter, spcm, 1);
	if (err < 0)
		snd_pcm_close(spcm);
	return err;
}
#ifndef DOC_HIDDEN
SND_DLSYM_BUILD_VERSION(_snd_pcm_rate_open, SND_PCM_DLSYM_VERSION);
#endif
