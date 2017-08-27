/**
 * \file pcm/pcm_linear.c
 * \ingroup PCM_Plugins
 * \brief PCM Linear Conversion Plugin Interface
 * \author Abramo Bagnara <abramo@alsa-project.org>
 * \date 2000-2001
 */
/*
 *  PCM - Linear conversion
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
const char *_snd_module_pcm_linear = "";
#endif

#ifndef DOC_HIDDEN
typedef struct {
	/* This field need to be the first */
	snd_pcm_plugin_t plug;
	unsigned int use_getput;
	unsigned int conv_idx;
	unsigned int get_idx, put_idx;
	snd_pcm_format_t sformat;
} snd_pcm_linear_t;
#endif

#ifndef DOC_HIDDEN

int snd_pcm_linear_convert_index(snd_pcm_format_t src_format,
				 snd_pcm_format_t dst_format)
{
	int src_endian, dst_endian, sign, src_width, dst_width;

	sign = (snd_pcm_format_signed(src_format) !=
		snd_pcm_format_signed(dst_format));
#ifdef SND_LITTLE_ENDIAN
	src_endian = snd_pcm_format_big_endian(src_format);
	dst_endian = snd_pcm_format_big_endian(dst_format);
#else
	src_endian = snd_pcm_format_little_endian(src_format);
	dst_endian = snd_pcm_format_little_endian(dst_format);
#endif

	if (src_endian < 0)
		src_endian = 0;
	if (dst_endian < 0)
		dst_endian = 0;

	src_width = snd_pcm_format_width(src_format) / 8 - 1;
	dst_width = snd_pcm_format_width(dst_format) / 8 - 1;

	return src_width * 32 + src_endian * 16 + sign * 8 + dst_width * 2 + dst_endian;
}

int snd_pcm_linear_get_index(snd_pcm_format_t src_format, snd_pcm_format_t dst_format)
{
	int sign, width, pwidth, endian;
	sign = (snd_pcm_format_signed(src_format) != 
		snd_pcm_format_signed(dst_format));
#ifdef SND_LITTLE_ENDIAN
	endian = snd_pcm_format_big_endian(src_format);
#else
	endian = snd_pcm_format_little_endian(src_format);
#endif
	if (endian < 0)
		endian = 0;
	pwidth = snd_pcm_format_physical_width(src_format);
	width = snd_pcm_format_width(src_format);
	if (pwidth == 24) {
		switch (width) {
		case 24:
			width = 0; break;
		case 20:
			width = 1; break;
		case 18:
		default:
			width = 2; break;
		}
		return width * 4 + endian * 2 + sign + 16;
	} else {
		width = width / 8 - 1;
		return width * 4 + endian * 2 + sign;
	}
}

int snd_pcm_linear_get32_index(snd_pcm_format_t src_format, snd_pcm_format_t dst_format)
{
	return snd_pcm_linear_get_index(src_format, dst_format);
}

int snd_pcm_linear_put_index(snd_pcm_format_t src_format, snd_pcm_format_t dst_format)
{
	int sign, width, endian;
	sign = (snd_pcm_format_signed(src_format) != 
		snd_pcm_format_signed(dst_format));
	width = snd_pcm_format_width(dst_format) / 8 - 1;
#ifdef SND_LITTLE_ENDIAN
	endian = snd_pcm_format_big_endian(dst_format);
#else
	endian = snd_pcm_format_little_endian(dst_format);
#endif
	if (endian < 0)
		endian = 0;
	return width * 4 + endian * 2 + sign;
}

int snd_pcm_linear_put32_index(snd_pcm_format_t src_format, snd_pcm_format_t dst_format)
{
	int sign, width, pwidth, endian;
	sign = (snd_pcm_format_signed(src_format) != 
		snd_pcm_format_signed(dst_format));
#ifdef SND_LITTLE_ENDIAN
	endian = snd_pcm_format_big_endian(dst_format);
#else
	endian = snd_pcm_format_little_endian(dst_format);
#endif
	if (endian < 0)
		endian = 0;
	pwidth = snd_pcm_format_physical_width(dst_format);
	width = snd_pcm_format_width(dst_format);
	if (pwidth == 24) {
		switch (width) {
		case 24:
			width = 0; break;
		case 20:
			width = 1; break;
		case 18:
		default:
			width = 2; break;
		}
		return width * 4 + endian * 2 + sign + 16;
	} else {
		width = width / 8 - 1;
		return width * 4 + endian * 2 + sign;
	}
}

void snd_pcm_linear_convert(const snd_pcm_channel_area_t *dst_areas, snd_pcm_uframes_t dst_offset,
			    const snd_pcm_channel_area_t *src_areas, snd_pcm_uframes_t src_offset,
			    unsigned int channels, snd_pcm_uframes_t frames,
			    unsigned int convidx)
{
#define CONV_LABELS
#include "plugin_ops.h"
#undef CONV_LABELS
	void *conv = conv_labels[convidx];
	unsigned int channel;
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
			goto *conv;
#define CONV_END after
#include "plugin_ops.h"
#undef CONV_END
		after:
			src += src_step;
			dst += dst_step;
		}
	}
}

void snd_pcm_linear_getput(const snd_pcm_channel_area_t *dst_areas, snd_pcm_uframes_t dst_offset,
			   const snd_pcm_channel_area_t *src_areas, snd_pcm_uframes_t src_offset,
			   unsigned int channels, snd_pcm_uframes_t frames,
			   unsigned int get_idx, unsigned int put_idx)
{
#define CONV24_LABELS
#include "plugin_ops.h"
#undef CONV24_LABELS
	void *get = get32_labels[get_idx];
	void *put = put32_labels[put_idx];
	unsigned int channel;
	u_int32_t sample = 0;
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
#define CONV24_END after
#include "plugin_ops.h"
#undef CONV24_END
		after:
			src += src_step;
			dst += dst_step;
		}
	}
}

#endif /* DOC_HIDDEN */

static int snd_pcm_linear_hw_refine_cprepare(snd_pcm_t *pcm ATTRIBUTE_UNUSED, snd_pcm_hw_params_t *params)
{
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
	params->info &= ~(SND_PCM_INFO_MMAP | SND_PCM_INFO_MMAP_VALID);
	return 0;
}

static int snd_pcm_linear_hw_refine_sprepare(snd_pcm_t *pcm, snd_pcm_hw_params_t *sparams)
{
	snd_pcm_linear_t *linear = pcm->private_data;
	snd_pcm_access_mask_t saccess_mask = { SND_PCM_ACCBIT_MMAP };
	_snd_pcm_hw_params_any(sparams);
	_snd_pcm_hw_param_set_mask(sparams, SND_PCM_HW_PARAM_ACCESS,
				   &saccess_mask);
	_snd_pcm_hw_params_set_format(sparams, linear->sformat);
	_snd_pcm_hw_params_set_subformat(sparams, SND_PCM_SUBFORMAT_STD);
	return 0;
}

static int snd_pcm_linear_hw_refine_schange(snd_pcm_t *pcm ATTRIBUTE_UNUSED, snd_pcm_hw_params_t *params,
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
	
static int snd_pcm_linear_hw_refine_cchange(snd_pcm_t *pcm ATTRIBUTE_UNUSED, snd_pcm_hw_params_t *params,
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

static int snd_pcm_linear_hw_refine(snd_pcm_t *pcm, snd_pcm_hw_params_t *params)
{
	return snd_pcm_hw_refine_slave(pcm, params,
				       snd_pcm_linear_hw_refine_cprepare,
				       snd_pcm_linear_hw_refine_cchange,
				       snd_pcm_linear_hw_refine_sprepare,
				       snd_pcm_linear_hw_refine_schange,
				       snd_pcm_generic_hw_refine);
}

static int snd_pcm_linear_hw_params(snd_pcm_t *pcm, snd_pcm_hw_params_t *params)
{
	snd_pcm_linear_t *linear = pcm->private_data;
	snd_pcm_format_t format;
	int err = snd_pcm_hw_params_slave(pcm, params,
					  snd_pcm_linear_hw_refine_cchange,
					  snd_pcm_linear_hw_refine_sprepare,
					  snd_pcm_linear_hw_refine_schange,
					  snd_pcm_generic_hw_params);
	if (err < 0)
		return err;
	err = INTERNAL(snd_pcm_hw_params_get_format)(params, &format);
	if (err < 0)
		return err;
	linear->use_getput = (snd_pcm_format_physical_width(format) == 24 ||
			      snd_pcm_format_physical_width(linear->sformat) == 24);
	if (linear->use_getput) {
		if (pcm->stream == SND_PCM_STREAM_PLAYBACK) {
			linear->get_idx = snd_pcm_linear_get32_index(format, SND_PCM_FORMAT_S32);
			linear->put_idx = snd_pcm_linear_put32_index(SND_PCM_FORMAT_S32, linear->sformat);
		} else {
			linear->get_idx = snd_pcm_linear_get32_index(linear->sformat, SND_PCM_FORMAT_S32);
			linear->put_idx = snd_pcm_linear_put32_index(SND_PCM_FORMAT_S32, format);
		}
	} else {
		if (pcm->stream == SND_PCM_STREAM_PLAYBACK)
			linear->conv_idx = snd_pcm_linear_convert_index(format,
									linear->sformat);
		else
			linear->conv_idx = snd_pcm_linear_convert_index(linear->sformat,
									format);
	}
	return 0;
}

static snd_pcm_uframes_t
snd_pcm_linear_write_areas(snd_pcm_t *pcm,
			   const snd_pcm_channel_area_t *areas,
			   snd_pcm_uframes_t offset,
			   snd_pcm_uframes_t size,
			   const snd_pcm_channel_area_t *slave_areas,
			   snd_pcm_uframes_t slave_offset,
			   snd_pcm_uframes_t *slave_sizep)
{
	snd_pcm_linear_t *linear = pcm->private_data;
	if (size > *slave_sizep)
		size = *slave_sizep;
	if (linear->use_getput)
		snd_pcm_linear_getput(slave_areas, slave_offset,
				      areas, offset, 
				      pcm->channels, size,
				      linear->get_idx, linear->put_idx);
	else
		snd_pcm_linear_convert(slave_areas, slave_offset,
				       areas, offset, 
				       pcm->channels, size, linear->conv_idx);
	*slave_sizep = size;
	return size;
}

static snd_pcm_uframes_t
snd_pcm_linear_read_areas(snd_pcm_t *pcm,
			  const snd_pcm_channel_area_t *areas,
			  snd_pcm_uframes_t offset,
			  snd_pcm_uframes_t size,
			  const snd_pcm_channel_area_t *slave_areas,
			  snd_pcm_uframes_t slave_offset,
			  snd_pcm_uframes_t *slave_sizep)
{
	snd_pcm_linear_t *linear = pcm->private_data;
	if (size > *slave_sizep)
		size = *slave_sizep;
	if (linear->use_getput)
		snd_pcm_linear_getput(areas, offset, 
				      slave_areas, slave_offset,
				      pcm->channels, size,
				      linear->get_idx, linear->put_idx);
	else
		snd_pcm_linear_convert(areas, offset, 
				       slave_areas, slave_offset,
				       pcm->channels, size, linear->conv_idx);
	*slave_sizep = size;
	return size;
}

static void snd_pcm_linear_dump(snd_pcm_t *pcm, snd_output_t *out)
{
	snd_pcm_linear_t *linear = pcm->private_data;
	snd_output_printf(out, "Linear conversion PCM (%s)\n", 
		snd_pcm_format_name(linear->sformat));
	if (pcm->setup) {
		snd_output_printf(out, "Its setup is:\n");
		snd_pcm_dump_setup(pcm, out);
	}
	snd_output_printf(out, "Slave: ");
	snd_pcm_dump(linear->plug.gen.slave, out);
}

static const snd_pcm_ops_t snd_pcm_linear_ops = {
	.close = snd_pcm_generic_close,
	.info = snd_pcm_generic_info,
	.hw_refine = snd_pcm_linear_hw_refine,
	.hw_params = snd_pcm_linear_hw_params,
	.hw_free = snd_pcm_generic_hw_free,
	.sw_params = snd_pcm_generic_sw_params,
	.channel_info = snd_pcm_generic_channel_info,
	.dump = snd_pcm_linear_dump,
	.nonblock = snd_pcm_generic_nonblock,
	.async = snd_pcm_generic_async,
	.mmap = snd_pcm_generic_mmap,
	.munmap = snd_pcm_generic_munmap,
};


/**
 * \brief Creates a new linear conversion PCM
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
int snd_pcm_linear_open(snd_pcm_t **pcmp, const char *name, snd_pcm_format_t sformat, snd_pcm_t *slave, int close_slave)
{
	snd_pcm_t *pcm;
	snd_pcm_linear_t *linear;
	int err;
	assert(pcmp && slave);
	if (snd_pcm_format_linear(sformat) != 1)
		return -EINVAL;
	linear = calloc(1, sizeof(snd_pcm_linear_t));
	if (!linear) {
		return -ENOMEM;
	}
	snd_pcm_plugin_init(&linear->plug);
	linear->sformat = sformat;
	linear->plug.read = snd_pcm_linear_read_areas;
	linear->plug.write = snd_pcm_linear_write_areas;
	linear->plug.undo_read = snd_pcm_plugin_undo_read_generic;
	linear->plug.undo_write = snd_pcm_plugin_undo_write_generic;
	linear->plug.gen.slave = slave;
	linear->plug.gen.close_slave = close_slave;

	err = snd_pcm_new(&pcm, SND_PCM_TYPE_LINEAR, name, slave->stream, slave->mode);
	if (err < 0) {
		free(linear);
		return err;
	}
	pcm->ops = &snd_pcm_linear_ops;
	pcm->fast_ops = &snd_pcm_plugin_fast_ops;
	pcm->private_data = linear;
	pcm->poll_fd = slave->poll_fd;
	pcm->poll_events = slave->poll_events;
	pcm->monotonic = slave->monotonic;
	snd_pcm_set_hw_ptr(pcm, &linear->plug.hw_ptr, -1, 0);
	snd_pcm_set_appl_ptr(pcm, &linear->plug.appl_ptr, -1, 0);
	*pcmp = pcm;

	return 0;
}

/*! \page pcm_plugins

\section pcm_plugins_linear Plugin: linear

This plugin converts linear samples from master linear conversion PCM to given
slave PCM. The channel count, format and rate must match for both of them.

\code
pcm.name {
        type linear             # Linear conversion PCM
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

\subsection pcm_plugins_linear_funcref Function reference

<UL>
  <LI>snd_pcm_linear_open()
  <LI>_snd_pcm_linear_open()
</UL>

*/

/**
 * \brief Creates a new linear conversion PCM
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
int _snd_pcm_linear_open(snd_pcm_t **pcmp, const char *name,
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
	if (snd_pcm_format_linear(sformat) != 1) {
		snd_config_delete(sconf);
		SNDERR("slave format is not linear");
		return -EINVAL;
	}
	err = snd_pcm_open_slave(&spcm, root, sconf, stream, mode, conf);
	snd_config_delete(sconf);
	if (err < 0)
		return err;
	err = snd_pcm_linear_open(pcmp, name, sformat, spcm, 1);
	if (err < 0)
		snd_pcm_close(spcm);
	return err;
}
#ifndef DOC_HIDDEN
SND_DLSYM_BUILD_VERSION(_snd_pcm_linear_open, SND_PCM_DLSYM_VERSION);
#endif
