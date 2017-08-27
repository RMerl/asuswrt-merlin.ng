/**
 * \file pcm/pcm_mulaw.c
 * \ingroup PCM_Plugins
 * \brief PCM Mu-Law Conversion Plugin Interface
 * \author Abramo Bagnara <abramo@alsa-project.org>
 * \date 2000-2001
 */
/*
 *  PCM - Mu-Law conversion
 *  Copyright (c) 2000 by Abramo Bagnara <abramo@alsa-project.org>
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
#include "pcm_local.h"
#include "pcm_plugin.h"

#include "plugin_ops.h"

#ifndef PIC
/* entry for static linking */
const char *_snd_module_pcm_mulaw = "";
#endif

#ifndef DOC_HIDDEN

typedef void (*mulaw_f)(const snd_pcm_channel_area_t *src_areas,
			snd_pcm_uframes_t src_offset,
			const snd_pcm_channel_area_t *dst_areas,
			snd_pcm_uframes_t dst_offset,
			unsigned int channels, snd_pcm_uframes_t frames,
			unsigned int getputidx);

typedef struct {
	/* This field need to be the first */
	snd_pcm_plugin_t plug;
	unsigned int getput_idx;
	mulaw_f func;
	snd_pcm_format_t sformat;
} snd_pcm_mulaw_t;

#endif

static inline int val_seg(int val)
{
	int r = 0;
	val >>= 7;
	if (val & 0xf0) {
		val >>= 4;
		r += 4;
	}
	if (val & 0x0c) {
		val >>= 2;
		r += 2;
	}
	if (val & 0x02)
		r += 1;
	return r;
}

/*
 * s16_to_ulaw() - Convert a linear PCM value to u-law
 *
 * In order to simplify the encoding process, the original linear magnitude
 * is biased by adding 33 which shifts the encoding range from (0 - 8158) to
 * (33 - 8191). The result can be seen in the following encoding table:
 *
 *	Biased Linear Input Code	Compressed Code
 *	------------------------	---------------
 *	00000001wxyza			000wxyz
 *	0000001wxyzab			001wxyz
 *	000001wxyzabc			010wxyz
 *	00001wxyzabcd			011wxyz
 *	0001wxyzabcde			100wxyz
 *	001wxyzabcdef			101wxyz
 *	01wxyzabcdefg			110wxyz
 *	1wxyzabcdefgh			111wxyz
 *
 * Each biased linear code has a leading 1 which identifies the segment
 * number. The value of the segment number is equal to 7 minus the number
 * of leading 0's. The quantization interval is directly available as the
 * four bits wxyz.  * The trailing bits (a - h) are ignored.
 *
 * Ordinarily the complement of the resulting code word is used for
 * transmission, and so the code word is complemented before it is returned.
 *
 * For further information see John C. Bellamy's Digital Telephony, 1982,
 * John Wiley & Sons, pps 98-111 and 472-476.
 */

static unsigned char s16_to_ulaw(int pcm_val)	/* 2's complement (16-bit range) */
{
	int mask;
	int seg;
	unsigned char uval;

	if (pcm_val < 0) {
		pcm_val = 0x84 - pcm_val;
		mask = 0x7f;
	} else {
		pcm_val += 0x84;
		mask = 0xff;
	}
	if (pcm_val > 0x7fff)
		pcm_val = 0x7fff;

	/* Convert the scaled magnitude to segment number. */
	seg = val_seg(pcm_val);

	/*
	 * Combine the sign, segment, quantization bits;
	 * and complement the code word.
	 */
	uval = (seg << 4) | ((pcm_val >> (seg + 3)) & 0x0f);
	return uval ^ mask;
}

/*
 * ulaw_to_s16() - Convert a u-law value to 16-bit linear PCM
 *
 * First, a biased linear code is derived from the code word. An unbiased
 * output can then be obtained by subtracting 33 from the biased code.
 *
 * Note that this function expects to be passed the complement of the
 * original code word. This is in keeping with ISDN conventions.
 */
static int ulaw_to_s16(unsigned char u_val)
{
	int t;

	/* Complement to obtain normal u-law value. */
	u_val = ~u_val;

	/*
	 * Extract and bias the quantization bits. Then
	 * shift up by the segment number and subtract out the bias.
	 */
	t = ((u_val & 0x0f) << 3) + 0x84;
	t <<= (u_val & 0x70) >> 4;

	return ((u_val & 0x80) ? (0x84 - t) : (t - 0x84));
}

#ifndef DOC_HIDDEN

void snd_pcm_mulaw_decode(const snd_pcm_channel_area_t *dst_areas,
			  snd_pcm_uframes_t dst_offset,
			  const snd_pcm_channel_area_t *src_areas,
			  snd_pcm_uframes_t src_offset,
			  unsigned int channels, snd_pcm_uframes_t frames,
			  unsigned int putidx)
{
#define PUT16_LABELS
#include "plugin_ops.h"
#undef PUT16_LABELS
	void *put = put16_labels[putidx];
	unsigned int channel;
	for (channel = 0; channel < channels; ++channel) {
		const unsigned char *src;
		char *dst;
		int src_step, dst_step;
		snd_pcm_uframes_t frames1;
		const snd_pcm_channel_area_t *src_area = &src_areas[channel];
		const snd_pcm_channel_area_t *dst_area = &dst_areas[channel];
		src = snd_pcm_channel_area_addr(src_area, src_offset);
		dst = snd_pcm_channel_area_addr(dst_area, dst_offset);
		src_step = snd_pcm_channel_area_step(src_area);
		dst_step = snd_pcm_channel_area_step(dst_area);
		frames1 = frames;
		while (frames1-- > 0) {
			int16_t sample = ulaw_to_s16(*src);
			goto *put;
#define PUT16_END after
#include "plugin_ops.h"
#undef PUT16_END
		after:
			src += src_step;
			dst += dst_step;
		}
	}
}

void snd_pcm_mulaw_encode(const snd_pcm_channel_area_t *dst_areas,
			  snd_pcm_uframes_t dst_offset,
			  const snd_pcm_channel_area_t *src_areas,
			  snd_pcm_uframes_t src_offset,
			  unsigned int channels, snd_pcm_uframes_t frames,
			  unsigned int getidx)
{
#define GET16_LABELS
#include "plugin_ops.h"
#undef GET16_LABELS
	void *get = get16_labels[getidx];
	unsigned int channel;
	int16_t sample = 0;
	for (channel = 0; channel < channels; ++channel) {
		const char *src;
		char *dst;
		int src_step, dst_step;
		snd_pcm_uframes_t frames1;
		const snd_pcm_channel_area_t *src_area = &src_areas[channel];
		const snd_pcm_channel_area_t *dst_area = &dst_areas[channel];
		src = snd_pcm_channel_area_addr(src_area, src_offset);
		dst = snd_pcm_channel_area_addr(dst_area, dst_offset);
		src_step = snd_pcm_channel_area_step(src_area);
		dst_step = snd_pcm_channel_area_step(dst_area);
		frames1 = frames;
		while (frames1-- > 0) {
			goto *get;
#define GET16_END after
#include "plugin_ops.h"
#undef GET16_END
		after:
			*dst = s16_to_ulaw(sample);
			src += src_step;
			dst += dst_step;
		}
	}
}

#endif /* DOC_HIDDEN */

static int snd_pcm_mulaw_hw_refine_cprepare(snd_pcm_t *pcm, snd_pcm_hw_params_t *params)
{
	snd_pcm_mulaw_t *mulaw = pcm->private_data;
	int err;
	snd_pcm_access_mask_t access_mask = { SND_PCM_ACCBIT_SHM };
	err = _snd_pcm_hw_param_set_mask(params, SND_PCM_HW_PARAM_ACCESS,
					 &access_mask);
	if (err < 0)
		return err;
	if (mulaw->sformat == SND_PCM_FORMAT_MU_LAW) {
		snd_pcm_format_mask_t format_mask= { SND_PCM_FMTBIT_LINEAR };
		err = _snd_pcm_hw_param_set_mask(params, SND_PCM_HW_PARAM_FORMAT,
						 &format_mask);
	} else {
		err = _snd_pcm_hw_params_set_format(params,
						   SND_PCM_FORMAT_MU_LAW);
	}
	err = _snd_pcm_hw_params_set_subformat(params, SND_PCM_SUBFORMAT_STD);
	if (err < 0)
		return err;
	params->info &= ~(SND_PCM_INFO_MMAP | SND_PCM_INFO_MMAP_VALID);
	return 0;
}

static int snd_pcm_mulaw_hw_refine_sprepare(snd_pcm_t *pcm, snd_pcm_hw_params_t *sparams)
{
	snd_pcm_mulaw_t *mulaw = pcm->private_data;
	snd_pcm_access_mask_t saccess_mask = { SND_PCM_ACCBIT_MMAP };
	_snd_pcm_hw_params_any(sparams);
	_snd_pcm_hw_param_set_mask(sparams, SND_PCM_HW_PARAM_ACCESS,
				   &saccess_mask);
	_snd_pcm_hw_params_set_format(sparams, mulaw->sformat);
	_snd_pcm_hw_params_set_subformat(sparams, SND_PCM_SUBFORMAT_STD);
	return 0;
}

static int snd_pcm_mulaw_hw_refine_schange(snd_pcm_t *pcm ATTRIBUTE_UNUSED, snd_pcm_hw_params_t *params,
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
	
static int snd_pcm_mulaw_hw_refine_cchange(snd_pcm_t *pcm ATTRIBUTE_UNUSED, snd_pcm_hw_params_t *params,
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

static int snd_pcm_mulaw_hw_refine(snd_pcm_t *pcm, snd_pcm_hw_params_t *params)
{
	return snd_pcm_hw_refine_slave(pcm, params,
				       snd_pcm_mulaw_hw_refine_cprepare,
				       snd_pcm_mulaw_hw_refine_cchange,
				       snd_pcm_mulaw_hw_refine_sprepare,
				       snd_pcm_mulaw_hw_refine_schange,
				       snd_pcm_generic_hw_refine);
}

static int snd_pcm_mulaw_hw_params(snd_pcm_t *pcm, snd_pcm_hw_params_t * params)
{
	snd_pcm_mulaw_t *mulaw = pcm->private_data;
	snd_pcm_format_t format;
	int err = snd_pcm_hw_params_slave(pcm, params,
					  snd_pcm_mulaw_hw_refine_cchange,
					  snd_pcm_mulaw_hw_refine_sprepare,
					  snd_pcm_mulaw_hw_refine_schange,
					  snd_pcm_generic_hw_params);
	if (err < 0)
		return err;

	err = INTERNAL(snd_pcm_hw_params_get_format)(params, &format);
	if (err < 0)
		return err;

	if (pcm->stream == SND_PCM_STREAM_PLAYBACK) {
		if (mulaw->sformat == SND_PCM_FORMAT_MU_LAW) {
			mulaw->getput_idx = snd_pcm_linear_get_index(format, SND_PCM_FORMAT_S16);
			mulaw->func = snd_pcm_mulaw_encode;
		} else {
			mulaw->getput_idx = snd_pcm_linear_put_index(SND_PCM_FORMAT_S16, mulaw->sformat);
			mulaw->func = snd_pcm_mulaw_decode;
		}
	} else {
		if (mulaw->sformat == SND_PCM_FORMAT_MU_LAW) {
			mulaw->getput_idx = snd_pcm_linear_put_index(SND_PCM_FORMAT_S16, format);
			mulaw->func = snd_pcm_mulaw_decode;
		} else {
			mulaw->getput_idx = snd_pcm_linear_get_index(mulaw->sformat, SND_PCM_FORMAT_S16);
			mulaw->func = snd_pcm_mulaw_encode;
		}
	}
	return 0;
}

static snd_pcm_uframes_t
snd_pcm_mulaw_write_areas(snd_pcm_t *pcm,
			  const snd_pcm_channel_area_t *areas,
			  snd_pcm_uframes_t offset,
			  snd_pcm_uframes_t size,
			  const snd_pcm_channel_area_t *slave_areas,
			  snd_pcm_uframes_t slave_offset,
			  snd_pcm_uframes_t *slave_sizep)
{
	snd_pcm_mulaw_t *mulaw = pcm->private_data;
	if (size > *slave_sizep)
		size = *slave_sizep;
	mulaw->func(slave_areas, slave_offset,
		    areas, offset, 
		    pcm->channels, size,
		    mulaw->getput_idx);
	*slave_sizep = size;
	return size;
}

static snd_pcm_uframes_t
snd_pcm_mulaw_read_areas(snd_pcm_t *pcm,
			 const snd_pcm_channel_area_t *areas,
			 snd_pcm_uframes_t offset,
			 snd_pcm_uframes_t size,
			 const snd_pcm_channel_area_t *slave_areas,
			 snd_pcm_uframes_t slave_offset,
			 snd_pcm_uframes_t *slave_sizep)
{
	snd_pcm_mulaw_t *mulaw = pcm->private_data;
	if (size > *slave_sizep)
		size = *slave_sizep;
	mulaw->func(areas, offset, 
		    slave_areas, slave_offset,
		    pcm->channels, size,
		    mulaw->getput_idx);
	*slave_sizep = size;
	return size;
}

static void snd_pcm_mulaw_dump(snd_pcm_t *pcm, snd_output_t *out)
{
	snd_pcm_mulaw_t *mulaw = pcm->private_data;
	snd_output_printf(out, "Mu-Law conversion PCM (%s)\n", 
		snd_pcm_format_name(mulaw->sformat));
	if (pcm->setup) {
		snd_output_printf(out, "Its setup is:\n");
		snd_pcm_dump_setup(pcm, out);
	}
	snd_output_printf(out, "Slave: ");
	snd_pcm_dump(mulaw->plug.gen.slave, out);
}

static const snd_pcm_ops_t snd_pcm_mulaw_ops = {
	.close = snd_pcm_generic_close,
	.info = snd_pcm_generic_info,
	.hw_refine = snd_pcm_mulaw_hw_refine,
	.hw_params = snd_pcm_mulaw_hw_params,
	.hw_free = snd_pcm_generic_hw_free,
	.sw_params = snd_pcm_generic_sw_params,
	.channel_info = snd_pcm_generic_channel_info,
	.dump = snd_pcm_mulaw_dump,
	.nonblock = snd_pcm_generic_nonblock,
	.async = snd_pcm_generic_async,
	.mmap = snd_pcm_generic_mmap,
	.munmap = snd_pcm_generic_munmap,
};

/**
 * \brief Creates a new Mu-Law conversion PCM
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
int snd_pcm_mulaw_open(snd_pcm_t **pcmp, const char *name, snd_pcm_format_t sformat, snd_pcm_t *slave, int close_slave)
{
	snd_pcm_t *pcm;
	snd_pcm_mulaw_t *mulaw;
	int err;
	assert(pcmp && slave);
	if (snd_pcm_format_linear(sformat) != 1 &&
	    sformat != SND_PCM_FORMAT_MU_LAW)
		return -EINVAL;
	mulaw = calloc(1, sizeof(snd_pcm_mulaw_t));
	if (!mulaw) {
		return -ENOMEM;
	}
	snd_pcm_plugin_init(&mulaw->plug);
	mulaw->sformat = sformat;
	mulaw->plug.read = snd_pcm_mulaw_read_areas;
	mulaw->plug.write = snd_pcm_mulaw_write_areas;
	mulaw->plug.undo_read = snd_pcm_plugin_undo_read_generic;
	mulaw->plug.undo_write = snd_pcm_plugin_undo_write_generic;
	mulaw->plug.gen.slave = slave;
	mulaw->plug.gen.close_slave = close_slave;

	err = snd_pcm_new(&pcm, SND_PCM_TYPE_MULAW, name, slave->stream, slave->mode);
	if (err < 0) {
		free(mulaw);
		return err;
	}
	pcm->ops = &snd_pcm_mulaw_ops;
	pcm->fast_ops = &snd_pcm_plugin_fast_ops;
	pcm->private_data = mulaw;
	pcm->poll_fd = slave->poll_fd;
	pcm->poll_events = slave->poll_events;
	pcm->monotonic = slave->monotonic;
	snd_pcm_set_hw_ptr(pcm, &mulaw->plug.hw_ptr, -1, 0);
	snd_pcm_set_appl_ptr(pcm, &mulaw->plug.appl_ptr, -1, 0);
	*pcmp = pcm;

	return 0;
}

/*! \page pcm_plugins

\section pcm_plugins_mulaw Plugin: Mu-Law

This plugin converts Mu-Law samples to linear or linear to Mu-Law samples
from master Mu-Law conversion PCM to given slave PCM. The channel count,
format and rate must match for both of them.

\code
pcm.name {
        type mulaw              # Mu-Law conversion PCM
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

\subsection pcm_plugins_mulaw_funcref Function reference

<UL>
  <LI>snd_pcm_mulaw_open()
  <LI>_snd_pcm_mulaw_open()
</UL>

*/

/**
 * \brief Creates a new Mu-Law conversion PCM
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
int _snd_pcm_mulaw_open(snd_pcm_t **pcmp, const char *name,
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
	    sformat != SND_PCM_FORMAT_MU_LAW) {
		snd_config_delete(sconf);
		SNDERR("invalid slave format");
		return -EINVAL;
	}
	err = snd_pcm_open_slave(&spcm, root, sconf, stream, mode, conf);
	snd_config_delete(sconf);
	if (err < 0)
		return err;
	err = snd_pcm_mulaw_open(pcmp, name, sformat, spcm, 1);
	if (err < 0)
		snd_pcm_close(spcm);
	return err;
}
#ifndef DOC_HIDDEN
SND_DLSYM_BUILD_VERSION(_snd_pcm_mulaw_open, SND_PCM_DLSYM_VERSION);
#endif
