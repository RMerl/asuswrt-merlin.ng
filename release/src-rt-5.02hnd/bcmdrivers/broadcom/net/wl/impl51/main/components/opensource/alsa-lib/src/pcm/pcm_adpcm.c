/**
 * \file pcm/pcm_adpcm.c
 * \ingroup PCM_Plugins
 * \brief PCM Ima-ADPCM Conversion Plugin Interface
 * \author Abramo Bagnara <abramo@alsa-project.org>
 * \author Uros Bizjak <uros@kss-loka.si>
 * \author Jaroslav Kysela <perex@perex.cz>
 * \date 2000-2001
 */
/*
 *  PCM - Ima-ADPCM conversion
 *  Copyright (c) 2000 by Abramo Bagnara <abramo@alsa-project.org>
 *  Copyright (c) 1999 by Uros Bizjak <uros@kss-loka.si>
 *                        Jaroslav Kysela <perex@perex.cz>
 *
 *  Based on Version 1.2, 18-Dec-92 implementation of Intel/DVI ADPCM code
 *  by Jack Jansen, CWI, Amsterdam <Jack.Jansen@cwi.nl>, Copyright 1992
 *  by Stichting Mathematisch Centrum, Amsterdam, The Netherlands.
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
  
/*
These routines convert 16 bit linear PCM samples to 4 bit ADPCM code
and vice versa. The ADPCM code used is the Intel/DVI ADPCM code which
is being recommended by the IMA Digital Audio Technical Working Group.

The algorithm for this coder was taken from:
Proposal for Standardized Audio Interstreamge Formats,
IMA compatibility project proceedings, Vol 2, Issue 2, May 1992.

- No, this is *not* a G.721 coder/decoder. The algorithm used by G.721
  is very complicated, requiring oodles of floating-point ops per
  sample (resulting in very poor performance). I have not done any
  tests myself but various people have assured my that 721 quality is
  actually lower than DVI quality.

- No, it probably isn't a RIFF ADPCM decoder either. Trying to decode
  RIFF ADPCM with these routines seems to result in something
  recognizable but very distorted.

- No, it is not a CDROM-XA coder either, as far as I know. I haven't
  come across a good description of XA yet.
 */

#include <byteswap.h>
#include "pcm_local.h"
#include "pcm_plugin.h"

#include "plugin_ops.h"

#ifndef PIC
/* entry for static linking */
const char *_snd_module_pcm_adpcm = "";
#endif

#ifndef DOC_HIDDEN

typedef void (*adpcm_f)(const snd_pcm_channel_area_t *dst_areas,
			snd_pcm_uframes_t dst_offset,
			const snd_pcm_channel_area_t *src_areas,
			snd_pcm_uframes_t src_offset,
			unsigned int channels, snd_pcm_uframes_t frames,
			unsigned int getputidx,
			snd_pcm_adpcm_state_t *states);

typedef struct {
	/* This field need to be the first */
	snd_pcm_plugin_t plug;
	unsigned int getput_idx;
	adpcm_f func;
	snd_pcm_format_t sformat;
	snd_pcm_adpcm_state_t *states;
} snd_pcm_adpcm_t;

#endif

/* First table lookup for Ima-ADPCM quantizer */
static const char IndexAdjust[8] = { -1, -1, -1, -1, 2, 4, 6, 8 };

/* Second table lookup for Ima-ADPCM quantizer */
static const short StepSize[89] = {
	7, 8, 9, 10, 11, 12, 13, 14, 16, 17,
	19, 21, 23, 25, 28, 31, 34, 37, 41, 45,
	50, 55, 60, 66, 73, 80, 88, 97, 107, 118,
	130, 143, 157, 173, 190, 209, 230, 253, 279, 307,
	337, 371, 408, 449, 494, 544, 598, 658, 724, 796,
	876, 963, 1060, 1166, 1282, 1411, 1552, 1707, 1878, 2066,
	2272, 2499, 2749, 3024, 3327, 3660, 4026, 4428, 4871, 5358,
	5894, 6484, 7132, 7845, 8630, 9493, 10442, 11487, 12635, 13899,
	15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767
};

static char adpcm_encoder(int sl, snd_pcm_adpcm_state_t * state)
{
	short diff;		/* Difference between sl and predicted sample */
	short pred_diff;	/* Predicted difference to next sample */

	unsigned char sign;	/* sign of diff */
	short step;		/* holds previous StepSize value */
	unsigned char adjust_idx;	/* Index to IndexAdjust lookup table */

	int i;

	/* Compute difference to previous predicted value */
	diff = sl - state->pred_val;
	sign = (diff < 0) ? 0x8 : 0x0;
	if (sign) {
		diff = -diff;
	}

	/*
	 * This code *approximately* computes:
	 *    adjust_idx = diff * 4 / step;
	 *    pred_diff = (adjust_idx + 0.5) * step / 4;
	 *
	 * But in shift step bits are dropped. The net result of this is
	 * that even if you have fast mul/div hardware you cannot put it to
	 * good use since the fix-up would be too expensive.
	 */

	step = StepSize[state->step_idx];

	/* Divide and clamp */
	pred_diff = step >> 3;
	for (adjust_idx = 0, i = 0x4; i; i >>= 1, step >>= 1) {
		if (diff >= step) {
			adjust_idx |= i;
			diff -= step;
			pred_diff += step;
		}
	}

	/* Update and clamp previous predicted value */
	state->pred_val += sign ? -pred_diff : pred_diff;

	if (state->pred_val > 32767) {
		state->pred_val = 32767;
	} else if (state->pred_val < -32768) {
		state->pred_val = -32768;
	}

	/* Update and clamp StepSize lookup table index */
	state->step_idx += IndexAdjust[adjust_idx];

	if (state->step_idx < 0) {
		state->step_idx = 0;
	} else if (state->step_idx > 88) {
		state->step_idx = 88;
	}
	return (sign | adjust_idx);
}


static int adpcm_decoder(unsigned char code, snd_pcm_adpcm_state_t * state)
{
	short pred_diff;	/* Predicted difference to next sample */
	short step;		/* holds previous StepSize value */
	char sign;

	int i;

	/* Separate sign and magnitude */
	sign = code & 0x8;
	code &= 0x7;

	/*
	 * Computes pred_diff = (code + 0.5) * step / 4,
	 * but see comment in adpcm_coder.
	 */

	step = StepSize[state->step_idx];

	/* Compute difference and new predicted value */
	pred_diff = step >> 3;
	for (i = 0x4; i; i >>= 1, step >>= 1) {
		if (code & i) {
			pred_diff += step;
		}
	}
	state->pred_val += (sign) ? -pred_diff : pred_diff;

	/* Clamp output value */
	if (state->pred_val > 32767) {
		state->pred_val = 32767;
	} else if (state->pred_val < -32768) {
		state->pred_val = -32768;
	}

	/* Find new StepSize index value */
	state->step_idx += IndexAdjust[code];

	if (state->step_idx < 0) {
		state->step_idx = 0;
	} else if (state->step_idx > 88) {
		state->step_idx = 88;
	}
	return (state->pred_val);
}

#ifndef DOC_HIDDEN

void snd_pcm_adpcm_decode(const snd_pcm_channel_area_t *dst_areas,
			  snd_pcm_uframes_t dst_offset,
			  const snd_pcm_channel_area_t *src_areas,
			  snd_pcm_uframes_t src_offset,
			  unsigned int channels, snd_pcm_uframes_t frames,
			  unsigned int putidx,
			  snd_pcm_adpcm_state_t *states)
{
#define PUT16_LABELS
#include "plugin_ops.h"
#undef PUT16_LABELS
	void *put = put16_labels[putidx];
	unsigned int channel;
	for (channel = 0; channel < channels; ++channel, ++states) {
		const char *src;
		int srcbit;
		char *dst;
		int src_step, srcbit_step, dst_step;
		snd_pcm_uframes_t frames1;
		const snd_pcm_channel_area_t *src_area = &src_areas[channel];
		const snd_pcm_channel_area_t *dst_area = &dst_areas[channel];
		srcbit = src_area->first + src_area->step * src_offset;
		src = (const char *) src_area->addr + srcbit / 8;
		srcbit %= 8;
		src_step = src_area->step / 8;
		srcbit_step = src_area->step % 8;
		dst = snd_pcm_channel_area_addr(dst_area, dst_offset);
		dst_step = snd_pcm_channel_area_step(dst_area);
		frames1 = frames;
		while (frames1-- > 0) {
			int16_t sample;
			unsigned char v;
			if (srcbit)
				v = *src & 0x0f;
			else
				v = (*src >> 4) & 0x0f;
			sample = adpcm_decoder(v, states);
			goto *put;
#define PUT16_END after
#include "plugin_ops.h"
#undef PUT16_END
		after:
			src += src_step;
			srcbit += srcbit_step;
			if (srcbit == 8) {
				src++;
				srcbit = 0;
			}
			dst += dst_step;
		}
	}
}

void snd_pcm_adpcm_encode(const snd_pcm_channel_area_t *dst_areas,
			  snd_pcm_uframes_t dst_offset,
			  const snd_pcm_channel_area_t *src_areas,
			  snd_pcm_uframes_t src_offset,
			  unsigned int channels, snd_pcm_uframes_t frames,
			  unsigned int getidx,
			  snd_pcm_adpcm_state_t *states)
{
#define GET16_LABELS
#include "plugin_ops.h"
#undef GET16_LABELS
	void *get = get16_labels[getidx];
	unsigned int channel;
	int16_t sample = 0;
	for (channel = 0; channel < channels; ++channel, ++states) {
		const char *src;
		char *dst;
		int dstbit;
		int src_step, dst_step, dstbit_step;
		snd_pcm_uframes_t frames1;
		const snd_pcm_channel_area_t *src_area = &src_areas[channel];
		const snd_pcm_channel_area_t *dst_area = &dst_areas[channel];
		src = snd_pcm_channel_area_addr(src_area, src_offset);
		src_step = snd_pcm_channel_area_step(src_area);
		dstbit = dst_area->first + dst_area->step * dst_offset;
		dst = (char *) dst_area->addr + dstbit / 8;
		dstbit %= 8;
		dst_step = dst_area->step / 8;
		dstbit_step = dst_area->step % 8;
		frames1 = frames;
		while (frames1-- > 0) {
			int v;
			goto *get;
#define GET16_END after
#include "plugin_ops.h"
#undef GET16_END
		after:
			v = adpcm_encoder(sample, states);
			if (dstbit)
				*dst = (*dst & 0xf0) | v;
			else
				*dst = (*dst & 0x0f) | (v << 4);
			src += src_step;
			dst += dst_step;
			dstbit += dstbit_step;
			if (dstbit == 8) {
				dst++;
				dstbit = 0;
			}
		}
	}
}

#endif

static int snd_pcm_adpcm_hw_refine_cprepare(snd_pcm_t *pcm, snd_pcm_hw_params_t *params)
{
	snd_pcm_adpcm_t *adpcm = pcm->private_data;
	int err;
	snd_pcm_access_mask_t access_mask = { SND_PCM_ACCBIT_SHM };
	err = _snd_pcm_hw_param_set_mask(params, SND_PCM_HW_PARAM_ACCESS,
					 &access_mask);
	if (err < 0)
		return err;
	if (adpcm->sformat == SND_PCM_FORMAT_IMA_ADPCM) {
		snd_pcm_format_mask_t format_mask = { SND_PCM_FMTBIT_LINEAR };
		err = _snd_pcm_hw_param_set_mask(params, SND_PCM_HW_PARAM_FORMAT,
						 &format_mask);
	} else {
		err = _snd_pcm_hw_params_set_format(params,
						   SND_PCM_FORMAT_IMA_ADPCM);
	}
	if (err < 0)
		return err;
	err = _snd_pcm_hw_params_set_subformat(params,
					       SND_PCM_SUBFORMAT_STD);
	if (err < 0)
		return err;
	params->info &= ~(SND_PCM_INFO_MMAP | SND_PCM_INFO_MMAP_VALID);
	return 0;
}

static int snd_pcm_adpcm_hw_refine_sprepare(snd_pcm_t *pcm, snd_pcm_hw_params_t *sparams)
{
	snd_pcm_adpcm_t *adpcm = pcm->private_data;
	snd_pcm_access_mask_t saccess_mask = { SND_PCM_ACCBIT_MMAP };
	_snd_pcm_hw_params_any(sparams);
	_snd_pcm_hw_param_set_mask(sparams, SND_PCM_HW_PARAM_ACCESS,
				   &saccess_mask);
	_snd_pcm_hw_params_set_format(sparams, adpcm->sformat);
	_snd_pcm_hw_params_set_subformat(sparams, SND_PCM_SUBFORMAT_STD);
	return 0;
}

static int snd_pcm_adpcm_hw_refine_schange(snd_pcm_t *pcm ATTRIBUTE_UNUSED, snd_pcm_hw_params_t *params,
					    snd_pcm_hw_params_t *sparams)
{
	int err;
	unsigned int links = (SND_PCM_HW_PARBIT_CHANNELS |
			      SND_PCM_HW_PARBIT_RATE |
			      SND_PCM_HW_PARBIT_PERIOD_SIZE |
			      SND_PCM_HW_PARBIT_BUFFER_SIZE |
			      SND_PCM_HW_PARBIT_PERIODS |
			      SND_PCM_HW_PARBIT_PERIOD_TIME |
			      SND_PCM_HW_PARBIT_BUFFER_TIME |
			      SND_PCM_HW_PARBIT_TICK_TIME);
	err = _snd_pcm_hw_params_refine(sparams, links, params);
	if (err < 0)
		return err;
	return 0;
}
	
static int snd_pcm_adpcm_hw_refine_cchange(snd_pcm_t *pcm ATTRIBUTE_UNUSED, snd_pcm_hw_params_t *params,
					    snd_pcm_hw_params_t *sparams)
{
	int err;
	unsigned int links = (SND_PCM_HW_PARBIT_CHANNELS |
			      SND_PCM_HW_PARBIT_RATE |
			      SND_PCM_HW_PARBIT_PERIOD_SIZE |
			      SND_PCM_HW_PARBIT_BUFFER_SIZE |
			      SND_PCM_HW_PARBIT_PERIODS |
			      SND_PCM_HW_PARBIT_PERIOD_TIME |
			      SND_PCM_HW_PARBIT_BUFFER_TIME |
			      SND_PCM_HW_PARBIT_TICK_TIME);
	err = _snd_pcm_hw_params_refine(params, links, sparams);
	if (err < 0)
		return err;
	return 0;
}

static int snd_pcm_adpcm_hw_refine(snd_pcm_t *pcm, snd_pcm_hw_params_t *params)
{
	return snd_pcm_hw_refine_slave(pcm, params,
				       snd_pcm_adpcm_hw_refine_cprepare,
				       snd_pcm_adpcm_hw_refine_cchange,
				       snd_pcm_adpcm_hw_refine_sprepare,
				       snd_pcm_adpcm_hw_refine_schange,
				       snd_pcm_generic_hw_refine);
}

static int snd_pcm_adpcm_hw_params(snd_pcm_t *pcm, snd_pcm_hw_params_t * params)
{
	snd_pcm_adpcm_t *adpcm = pcm->private_data;
	snd_pcm_format_t format;
	int err = snd_pcm_hw_params_slave(pcm, params,
					  snd_pcm_adpcm_hw_refine_cchange,
					  snd_pcm_adpcm_hw_refine_sprepare,
					  snd_pcm_adpcm_hw_refine_schange,
					  snd_pcm_generic_hw_params);
	if (err < 0)
		return err;

	err = INTERNAL(snd_pcm_hw_params_get_format)(params, &format);
	if (err < 0)
		return err;

	if (pcm->stream == SND_PCM_STREAM_PLAYBACK) {
		if (adpcm->sformat == SND_PCM_FORMAT_IMA_ADPCM) {
			adpcm->getput_idx = snd_pcm_linear_get_index(format, SND_PCM_FORMAT_S16);
			adpcm->func = snd_pcm_adpcm_encode;
		} else {
			adpcm->getput_idx = snd_pcm_linear_put_index(SND_PCM_FORMAT_S16, adpcm->sformat);
			adpcm->func = snd_pcm_adpcm_decode;
		}
	} else {
		if (adpcm->sformat == SND_PCM_FORMAT_IMA_ADPCM) {
			adpcm->getput_idx = snd_pcm_linear_put_index(SND_PCM_FORMAT_S16, format);
			adpcm->func = snd_pcm_adpcm_decode;
		} else {
			adpcm->getput_idx = snd_pcm_linear_get_index(adpcm->sformat, SND_PCM_FORMAT_S16);
			adpcm->func = snd_pcm_adpcm_encode;
		}
	}
	assert(!adpcm->states);
	adpcm->states = malloc(adpcm->plug.gen.slave->channels * sizeof(*adpcm->states));
	if (adpcm->states == NULL)
		return -ENOMEM;
	return 0;
}

static int snd_pcm_adpcm_hw_free(snd_pcm_t *pcm)
{
	snd_pcm_adpcm_t *adpcm = pcm->private_data;
	free(adpcm->states);
	adpcm->states = NULL;
	return snd_pcm_hw_free(adpcm->plug.gen.slave);
}

static int snd_pcm_adpcm_init(snd_pcm_t *pcm)
{
	snd_pcm_adpcm_t *adpcm = pcm->private_data;
	unsigned int k;
	for (k = 0; k < pcm->channels; ++k) {
		adpcm->states[k].pred_val = 0;
		adpcm->states[k].step_idx = 0;
	}
	return 0;
}

static snd_pcm_uframes_t
snd_pcm_adpcm_write_areas(snd_pcm_t *pcm,
			  const snd_pcm_channel_area_t *areas,
			  snd_pcm_uframes_t offset,
			  snd_pcm_uframes_t size,
			  const snd_pcm_channel_area_t *slave_areas,
			  snd_pcm_uframes_t slave_offset,
			  snd_pcm_uframes_t *slave_sizep)
{
	snd_pcm_adpcm_t *adpcm = pcm->private_data;
	if (size > *slave_sizep)
		size = *slave_sizep;
	adpcm->func(slave_areas, slave_offset,
		    areas, offset, 
		    pcm->channels, size,
		    adpcm->getput_idx, adpcm->states);
	*slave_sizep = size;
	return size;
}

static snd_pcm_uframes_t
snd_pcm_adpcm_read_areas(snd_pcm_t *pcm,
			 const snd_pcm_channel_area_t *areas,
			 snd_pcm_uframes_t offset,
			 snd_pcm_uframes_t size,
			 const snd_pcm_channel_area_t *slave_areas,
			 snd_pcm_uframes_t slave_offset,
			 snd_pcm_uframes_t *slave_sizep)
{
	snd_pcm_adpcm_t *adpcm = pcm->private_data;
	if (size > *slave_sizep)
		size = *slave_sizep;
	adpcm->func(areas, offset, 
		    slave_areas, slave_offset,
		    pcm->channels, size,
		    adpcm->getput_idx, adpcm->states);
	*slave_sizep = size;
	return size;
}

static void snd_pcm_adpcm_dump(snd_pcm_t *pcm, snd_output_t *out)
{
	snd_pcm_adpcm_t *adpcm = pcm->private_data;
	snd_output_printf(out, "Ima-ADPCM conversion PCM (%s)\n", 
		snd_pcm_format_name(adpcm->sformat));
	if (pcm->setup) {
		snd_output_printf(out, "Its setup is:\n");
		snd_pcm_dump_setup(pcm, out);
	}
	snd_output_printf(out, "Slave: ");
	snd_pcm_dump(adpcm->plug.gen.slave, out);
}

static const snd_pcm_ops_t snd_pcm_adpcm_ops = {
	.close = snd_pcm_generic_close,
	.info = snd_pcm_generic_info,
	.hw_refine = snd_pcm_adpcm_hw_refine,
	.hw_params = snd_pcm_adpcm_hw_params,
	.hw_free = snd_pcm_adpcm_hw_free,
	.sw_params = snd_pcm_generic_sw_params,
	.channel_info = snd_pcm_generic_channel_info,
	.dump = snd_pcm_adpcm_dump,
	.nonblock = snd_pcm_generic_nonblock,
	.async = snd_pcm_generic_async,
	.mmap = snd_pcm_generic_mmap,
	.munmap = snd_pcm_generic_munmap,
};

/**
 * \brief Creates a new Ima-ADPCM conversion PCM
 * \param pcmp Returns created PCM handle
 * \param name Name of PCM
 * \param sformat Slave (destination) format
 * \param slave Slave PCM handle
 * \param close_slave When set, the slave PCM handle is closed with copy PCM
 * \retval zero on success otherwise a negative error code
 * \warning Using of this function might be dangerous in the sense
 *          of compatibility reasons. The prototype might be freely
 *          changed in future.
 */
int snd_pcm_adpcm_open(snd_pcm_t **pcmp, const char *name, snd_pcm_format_t sformat, snd_pcm_t *slave, int close_slave)
{
	snd_pcm_t *pcm;
	snd_pcm_adpcm_t *adpcm;
	int err;
	assert(pcmp && slave);
	if (snd_pcm_format_linear(sformat) != 1 &&
	    sformat != SND_PCM_FORMAT_IMA_ADPCM)
		return -EINVAL;
	adpcm = calloc(1, sizeof(snd_pcm_adpcm_t));
	if (!adpcm) {
		return -ENOMEM;
	}
	adpcm->sformat = sformat;
	snd_pcm_plugin_init(&adpcm->plug);
	adpcm->plug.read = snd_pcm_adpcm_read_areas;
	adpcm->plug.write = snd_pcm_adpcm_write_areas;
	adpcm->plug.init = snd_pcm_adpcm_init;
	adpcm->plug.gen.slave = slave;
	adpcm->plug.gen.close_slave = close_slave;

	err = snd_pcm_new(&pcm, SND_PCM_TYPE_ADPCM, name, slave->stream, slave->mode);
	if (err < 0) {
		free(adpcm);
		return err;
	}
	pcm->ops = &snd_pcm_adpcm_ops;
	pcm->fast_ops = &snd_pcm_plugin_fast_ops;
	pcm->private_data = adpcm;
	pcm->poll_fd = slave->poll_fd;
	pcm->poll_events = slave->poll_events;
	pcm->monotonic = slave->monotonic;
	snd_pcm_set_hw_ptr(pcm, &adpcm->plug.hw_ptr, -1, 0);
	snd_pcm_set_appl_ptr(pcm, &adpcm->plug.appl_ptr, -1, 0);
	*pcmp = pcm;

	return 0;
}

/*! \page pcm_plugins

\section pcm_plugins_adpcm Plugin: Ima-ADPCM

This plugin converts Ima-ADPCM samples to linear or linear to Ima-ADPCM samples
from master Ima-ADPCM conversion PCM to given slave PCM. The channel count,
format and rate must match for both of them.

\code
pcm.name {
        type adpcm              # Ima-ADPCM conversion PCM
        slave STR               # Slave name
        # or
        slave {                 # Slave definition
                pcm STR         # Slave PCM name
                # or
                pcm { }         # Slave PCM definition
                format STR      # Slave format
        }
}
\endcode

\subsection pcm_plugins_adpcm_funcref Function reference

<UL>
  <LI>snd_pcm_adpcm_open()
  <LI>_snd_pcm_adpcm_open()
</UL>

*/

/**
 * \brief Creates a new Ima-ADPCM conversion PCM
 * \param pcmp Returns created PCM handle
 * \param name Name of PCM
 * \param root Root configuration node
 * \param conf Configuration node with copy PCM description
 * \param stream Stream type
 * \param mode Stream mode
 * \retval zero on success otherwise a negative error code
 * \warning Using of this function might be dangerous in the sense
 *          of compatibility reasons. The prototype might be freely
 *          changed in future.
 */
int _snd_pcm_adpcm_open(snd_pcm_t **pcmp, const char *name,
			snd_config_t *root, snd_config_t *conf, 
			snd_pcm_stream_t stream, int mode)
{
	snd_config_iterator_t i, next;
	int err;
	snd_pcm_t *spcm;
	snd_config_t *slave = NULL, *sconf;
	snd_pcm_format_t sformat;
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
		SNDERR("Unknown field %s", id);
		return -EINVAL;
	}
	if (!slave) {
		SNDERR("slave is not defined");
		return -EINVAL;
	}
	err = snd_pcm_slave_conf(root, slave, &sconf, 1,
				 SND_PCM_HW_PARAM_FORMAT, SCONF_MANDATORY, &sformat);
	if (err < 0)
		return err;
	if (snd_pcm_format_linear(sformat) != 1 &&
	    sformat != SND_PCM_FORMAT_IMA_ADPCM) {
	    	snd_config_delete(sconf);
		SNDERR("invalid slave format");
		return -EINVAL;
	}
	err = snd_pcm_open_slave(&spcm, root, sconf, stream, mode, conf);
	snd_config_delete(sconf);
	if (err < 0)
		return err;
	err = snd_pcm_adpcm_open(pcmp, name, sformat, spcm, 1);
	if (err < 0)
		snd_pcm_close(spcm);
	return err;
}
#ifndef DOC_HIDDEN
SND_DLSYM_BUILD_VERSION(_snd_pcm_adpcm_open, SND_PCM_DLSYM_VERSION);
#endif
