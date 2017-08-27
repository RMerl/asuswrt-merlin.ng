/**
 * \file pcm/pcm_iec958.c
 * \ingroup PCM_Plugins
 * \brief PCM IEC958 Subframe Conversion Plugin Interface
 * \author Takashi Iwai <tiwai@suse.de>
 * \date 2004
 */
/*
 *  PCM - IEC958 Subframe Conversion Plugin
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
#include "pcm_local.h"
#include "pcm_plugin.h"

#include "plugin_ops.h"

#ifndef PIC
/* entry for static linking */
const char *_snd_module_pcm_iec958 = "";
#endif

/*
 */

#ifndef DOC_HIDDEN

typedef struct snd_pcm_iec958 snd_pcm_iec958_t;

typedef void (*iec958_f)(snd_pcm_iec958_t *iec,
			 const snd_pcm_channel_area_t *dst_areas,
			 snd_pcm_uframes_t dst_offset,
			 const snd_pcm_channel_area_t *src_areas,
			 snd_pcm_uframes_t src_offset,
			 unsigned int channels, snd_pcm_uframes_t frames);

struct snd_pcm_iec958 {
	/* This field need to be the first */
	snd_pcm_plugin_t plug;
	unsigned int getput_idx;
	iec958_f func;
	snd_pcm_format_t sformat;
	snd_pcm_format_t format;
	unsigned int counter;
	unsigned char status[24];
	unsigned int byteswap;
	unsigned char preamble[3];	/* B/M/W or Z/X/Y */
};

enum { PREAMBLE_Z, PREAMBLE_X, PREAMBLE_Y };

#endif /* DOC_HIDDEN */

/*
 * Determine parity for time slots 4 upto 30
 * to be sure that bit 4 upt 31 will carry
 * an even number of ones and zeros.
 */
static unsigned int iec958_parity(unsigned int data)
{
	unsigned int parity;
	int bit;

	data >>= 4;     /* start from bit 4 */
	parity = 0;
	for (bit = 4; bit <= 30; bit++) {
		if (data & 1)
			parity++;
		data >>= 1;
	}
	return (parity & 1);
}

/*
 * Compose 32bit IEC958 subframe, two sub frames
 * build one frame with two channels.
 *
 * bit 0-3  = preamble
 *     4-7  = AUX (=0)
 *     8-27 = data (12-27 for 16bit, 8-27 for 20bit, and 24bit without AUX)
 *     28   = validity (0 for valid data, else 'in error')
 *     29   = user data (0)
 *     30   = channel status (24 bytes for 192 frames)
 *     31   = parity
 */

static inline u_int32_t iec958_subframe(snd_pcm_iec958_t *iec, u_int32_t data, int channel)
{
	unsigned int byte = iec->counter >> 3;
	unsigned int mask = 1 << (iec->counter - (byte << 3));

	/* bit 4-27 */
	data >>= 4;
	data &= ~0xf;

	/* set IEC status bits (up to 192 bits) */
	if (iec->status[byte] & mask)
		data |= 0x40000000;

	if (iec958_parity(data))	/* parity bit 4-30 */
		data |= 0x80000000;

	/* Preamble */
	if (channel)
		data |= iec->preamble[PREAMBLE_Y];	/* odd sub frame, 'Y' */
	else if (! iec->counter)
		data |= iec->preamble[PREAMBLE_Z];	/* Block start, 'Z' */
	else
		data |= iec->preamble[PREAMBLE_X];	/* even sub frame, 'X' */

	if (iec->byteswap)
		data = bswap_32(data);

	return data;
}

static inline int32_t iec958_to_s32(snd_pcm_iec958_t *iec, u_int32_t data)
{
	if (iec->byteswap)
		data = bswap_32(data);
	data &= ~0xf;
	data <<= 4;
	return (int32_t)data;
}

#ifndef DOC_HIDDEN
static void snd_pcm_iec958_decode(snd_pcm_iec958_t *iec,
				  const snd_pcm_channel_area_t *dst_areas,
				  snd_pcm_uframes_t dst_offset,
				  const snd_pcm_channel_area_t *src_areas,
				  snd_pcm_uframes_t src_offset,
				  unsigned int channels, snd_pcm_uframes_t frames)
{
#define PUT32_LABELS
#include "plugin_ops.h"
#undef PUT32_LABELS
	void *put = put32_labels[iec->getput_idx];
	unsigned int channel;
	for (channel = 0; channel < channels; ++channel) {
		const u_int32_t *src;
		char *dst;
		int src_step, dst_step;
		snd_pcm_uframes_t frames1;
		const snd_pcm_channel_area_t *src_area = &src_areas[channel];
		const snd_pcm_channel_area_t *dst_area = &dst_areas[channel];
		src = snd_pcm_channel_area_addr(src_area, src_offset);
		dst = snd_pcm_channel_area_addr(dst_area, dst_offset);
		src_step = snd_pcm_channel_area_step(src_area) / sizeof(u_int32_t);
		dst_step = snd_pcm_channel_area_step(dst_area);
		frames1 = frames;
		while (frames1-- > 0) {
			int32_t sample = iec958_to_s32(iec, *src);
			goto *put;
#define PUT32_END after
#include "plugin_ops.h"
#undef PUT32_END
		after:
			src += src_step;
			dst += dst_step;
		}
	}
}

static void snd_pcm_iec958_encode(snd_pcm_iec958_t *iec,
				  const snd_pcm_channel_area_t *dst_areas,
				  snd_pcm_uframes_t dst_offset,
				  const snd_pcm_channel_area_t *src_areas,
				  snd_pcm_uframes_t src_offset,
				  unsigned int channels, snd_pcm_uframes_t frames)
{
#define GET32_LABELS
#include "plugin_ops.h"
#undef GET32_LABELS
	void *get = get32_labels[iec->getput_idx];
	unsigned int channel;
	int32_t sample = 0;
	int counter = iec->counter;
	for (channel = 0; channel < channels; ++channel) {
		const char *src;
		u_int32_t *dst;
		int src_step, dst_step;
		snd_pcm_uframes_t frames1;
		const snd_pcm_channel_area_t *src_area = &src_areas[channel];
		const snd_pcm_channel_area_t *dst_area = &dst_areas[channel];
		src = snd_pcm_channel_area_addr(src_area, src_offset);
		dst = snd_pcm_channel_area_addr(dst_area, dst_offset);
		src_step = snd_pcm_channel_area_step(src_area);
		dst_step = snd_pcm_channel_area_step(dst_area) / sizeof(u_int32_t);
		frames1 = frames;
		iec->counter = counter;
		while (frames1-- > 0) {
			goto *get;
#define GET32_END after
#include "plugin_ops.h"
#undef GET32_END
		after:
			sample = iec958_subframe(iec, sample, channel);
			// fprintf(stderr, "%d:%08x\n", frames1, sample);
			*dst = sample;
			src += src_step;
			dst += dst_step;
			iec->counter++;
			iec->counter %= 192;
		}
	}
}
#endif /* DOC_HIDDEN */

static int snd_pcm_iec958_hw_refine_cprepare(snd_pcm_t *pcm, snd_pcm_hw_params_t *params)
{
	snd_pcm_iec958_t *iec = pcm->private_data;
	int err;
	snd_pcm_access_mask_t access_mask = { SND_PCM_ACCBIT_SHM };
	err = _snd_pcm_hw_param_set_mask(params, SND_PCM_HW_PARAM_ACCESS,
					 &access_mask);
	if (err < 0)
		return err;
	if (iec->sformat == SND_PCM_FORMAT_IEC958_SUBFRAME_LE ||
	    iec->sformat == SND_PCM_FORMAT_IEC958_SUBFRAME_BE) {
		snd_pcm_format_mask_t format_mask = { SND_PCM_FMTBIT_LINEAR };
		err = _snd_pcm_hw_param_set_mask(params, SND_PCM_HW_PARAM_FORMAT,
						 &format_mask);
	} else {
		snd_pcm_format_mask_t format_mask = {
			{ (1U << SND_PCM_FORMAT_IEC958_SUBFRAME_LE) |
			  (1U << SND_PCM_FORMAT_IEC958_SUBFRAME_BE) }
		};
		err = _snd_pcm_hw_param_set_mask(params, SND_PCM_HW_PARAM_FORMAT,
						 &format_mask);
	}
	if (err < 0)
		return err;
	err = _snd_pcm_hw_params_set_subformat(params, SND_PCM_SUBFORMAT_STD);
	if (err < 0)
		return err;
	params->info &= ~(SND_PCM_INFO_MMAP | SND_PCM_INFO_MMAP_VALID);
	return 0;
}

static int snd_pcm_iec958_hw_refine_sprepare(snd_pcm_t *pcm, snd_pcm_hw_params_t *sparams)
{
	snd_pcm_iec958_t *iec = pcm->private_data;
	snd_pcm_access_mask_t saccess_mask = { SND_PCM_ACCBIT_MMAP };
	_snd_pcm_hw_params_any(sparams);
	_snd_pcm_hw_param_set_mask(sparams, SND_PCM_HW_PARAM_ACCESS,
				   &saccess_mask);
	_snd_pcm_hw_params_set_format(sparams, iec->sformat);
	_snd_pcm_hw_params_set_subformat(sparams, SND_PCM_SUBFORMAT_STD);
	return 0;
}

static int snd_pcm_iec958_hw_refine_schange(snd_pcm_t *pcm ATTRIBUTE_UNUSED, snd_pcm_hw_params_t *params,
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
	
static int snd_pcm_iec958_hw_refine_cchange(snd_pcm_t *pcm ATTRIBUTE_UNUSED, snd_pcm_hw_params_t *params,
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

static int snd_pcm_iec958_hw_refine(snd_pcm_t *pcm, snd_pcm_hw_params_t *params)
{
	return snd_pcm_hw_refine_slave(pcm, params,
				       snd_pcm_iec958_hw_refine_cprepare,
				       snd_pcm_iec958_hw_refine_cchange,
				       snd_pcm_iec958_hw_refine_sprepare,
				       snd_pcm_iec958_hw_refine_schange,
				       snd_pcm_generic_hw_refine);
}

static int snd_pcm_iec958_hw_params(snd_pcm_t *pcm, snd_pcm_hw_params_t * params)
{
	snd_pcm_iec958_t *iec = pcm->private_data;
	snd_pcm_format_t format;
	int err = snd_pcm_hw_params_slave(pcm, params,
					  snd_pcm_iec958_hw_refine_cchange,
					  snd_pcm_iec958_hw_refine_sprepare,
					  snd_pcm_iec958_hw_refine_schange,
					  snd_pcm_generic_hw_params);
	if (err < 0)
		return err;

	err = INTERNAL(snd_pcm_hw_params_get_format)(params, &format);
	if (err < 0)
		return err;

	iec->format = format;
	if (pcm->stream == SND_PCM_STREAM_PLAYBACK) {
		if (iec->sformat == SND_PCM_FORMAT_IEC958_SUBFRAME_LE ||
		    iec->sformat == SND_PCM_FORMAT_IEC958_SUBFRAME_BE) {
			iec->getput_idx = snd_pcm_linear_get_index(format, SND_PCM_FORMAT_S32);
			iec->func = snd_pcm_iec958_encode;
			iec->byteswap = iec->sformat != SND_PCM_FORMAT_IEC958_SUBFRAME;
		} else {
			iec->getput_idx = snd_pcm_linear_put_index(SND_PCM_FORMAT_S32, iec->sformat);
			iec->func = snd_pcm_iec958_decode;
			iec->byteswap = format != SND_PCM_FORMAT_IEC958_SUBFRAME;
		}
	} else {
		if (iec->sformat == SND_PCM_FORMAT_IEC958_SUBFRAME_LE ||
		    iec->sformat == SND_PCM_FORMAT_IEC958_SUBFRAME_BE) {
			iec->getput_idx = snd_pcm_linear_put_index(SND_PCM_FORMAT_S32, format);
			iec->func = snd_pcm_iec958_decode;
			iec->byteswap = iec->sformat != SND_PCM_FORMAT_IEC958_SUBFRAME;
		} else {
			iec->getput_idx = snd_pcm_linear_get_index(iec->sformat, SND_PCM_FORMAT_S32);
			iec->func = snd_pcm_iec958_encode;
			iec->byteswap = format != SND_PCM_FORMAT_IEC958_SUBFRAME;
		}
	}
	return 0;
}

static snd_pcm_uframes_t
snd_pcm_iec958_write_areas(snd_pcm_t *pcm,
			   const snd_pcm_channel_area_t *areas,
			   snd_pcm_uframes_t offset,
			   snd_pcm_uframes_t size,
			   const snd_pcm_channel_area_t *slave_areas,
			   snd_pcm_uframes_t slave_offset,
			   snd_pcm_uframes_t *slave_sizep)
{
	snd_pcm_iec958_t *iec = pcm->private_data;
	if (size > *slave_sizep)
		size = *slave_sizep;
	iec->func(iec, slave_areas, slave_offset,
		  areas, offset, 
		  pcm->channels, size);
	*slave_sizep = size;
	return size;
}

static snd_pcm_uframes_t
snd_pcm_iec958_read_areas(snd_pcm_t *pcm,
			  const snd_pcm_channel_area_t *areas,
			  snd_pcm_uframes_t offset,
			  snd_pcm_uframes_t size,
			  const snd_pcm_channel_area_t *slave_areas,
			  snd_pcm_uframes_t slave_offset,
			  snd_pcm_uframes_t *slave_sizep)
{
	snd_pcm_iec958_t *iec = pcm->private_data;
	if (size > *slave_sizep)
		size = *slave_sizep;
	iec->func(iec, areas, offset, 
		  slave_areas, slave_offset,
		  pcm->channels, size);
	*slave_sizep = size;
	return size;
}

static int snd_pcm_iec958_init(snd_pcm_t *pcm)
{
	snd_pcm_iec958_t *iec = pcm->private_data;
	iec->counter = 0;
	return 0;
}

static void snd_pcm_iec958_dump(snd_pcm_t *pcm, snd_output_t *out)
{
	snd_pcm_iec958_t *iec = pcm->private_data;
	snd_output_printf(out, "IEC958 subframe conversion PCM (%s)\n", 
			  snd_pcm_format_name(iec->sformat));
	if (pcm->setup) {
		snd_output_printf(out, "Its setup is:\n");
		snd_pcm_dump_setup(pcm, out);
	}
	snd_output_printf(out, "Slave: ");
	snd_pcm_dump(iec->plug.gen.slave, out);
}

static const snd_pcm_ops_t snd_pcm_iec958_ops = {
	.close = snd_pcm_generic_close,
	.info = snd_pcm_generic_info,
	.hw_refine = snd_pcm_iec958_hw_refine,
	.hw_params = snd_pcm_iec958_hw_params,
	.hw_free = snd_pcm_generic_hw_free,
	.sw_params = snd_pcm_generic_sw_params,
	.channel_info = snd_pcm_generic_channel_info,
	.dump = snd_pcm_iec958_dump,
	.nonblock = snd_pcm_generic_nonblock,
	.async = snd_pcm_generic_async,
	.mmap = snd_pcm_generic_mmap,
	.munmap = snd_pcm_generic_munmap,
};

/**
 * \brief Creates a new IEC958 subframe conversion PCM
 * \param pcmp Returns created PCM handle
 * \param name Name of PCM
 * \param sformat Slave (destination) format
 * \param slave Slave PCM handle
 * \param close_slave When set, the slave PCM handle is closed with copy PCM
 * \param status_bits The IEC958 status bits
 * \param preamble_vals The preamble byte values
 * \retval zero on success otherwise a negative error code
 * \warning Using of this function might be dangerous in the sense
 *          of compatibility reasons. The prototype might be freely
 *          changed in future.
 */           
int snd_pcm_iec958_open(snd_pcm_t **pcmp, const char *name, snd_pcm_format_t sformat,
			snd_pcm_t *slave, int close_slave,
			const unsigned char *status_bits,
			const unsigned char *preamble_vals)
{
	snd_pcm_t *pcm;
	snd_pcm_iec958_t *iec;
	int err;
	static const unsigned char default_status_bits[] = {
		IEC958_AES0_CON_EMPHASIS_NONE,
		IEC958_AES1_CON_ORIGINAL | IEC958_AES1_CON_PCM_CODER,
		0,
		IEC958_AES3_CON_FS_48000
	};

	assert(pcmp && slave);
	if (snd_pcm_format_linear(sformat) != 1 &&
	    sformat != SND_PCM_FORMAT_IEC958_SUBFRAME_LE &&
	    sformat != SND_PCM_FORMAT_IEC958_SUBFRAME_BE)
		return -EINVAL;
	iec = calloc(1, sizeof(snd_pcm_iec958_t));
	if (!iec) {
		return -ENOMEM;
	}
	snd_pcm_plugin_init(&iec->plug);
	iec->sformat = sformat;
	iec->plug.read = snd_pcm_iec958_read_areas;
	iec->plug.write = snd_pcm_iec958_write_areas;
	iec->plug.init = snd_pcm_iec958_init;
	iec->plug.undo_read = snd_pcm_plugin_undo_read_generic;
	iec->plug.undo_write = snd_pcm_plugin_undo_write_generic;
	iec->plug.gen.slave = slave;
	iec->plug.gen.close_slave = close_slave;

	if (status_bits)
		memcpy(iec->status, status_bits, sizeof(iec->status));
	else
		memcpy(iec->status, default_status_bits, sizeof(default_status_bits));

	memcpy(iec->preamble, preamble_vals, 3);

	err = snd_pcm_new(&pcm, SND_PCM_TYPE_IEC958, name, slave->stream, slave->mode);
	if (err < 0) {
		free(iec);
		return err;
	}
	pcm->ops = &snd_pcm_iec958_ops;
	pcm->fast_ops = &snd_pcm_plugin_fast_ops;
	pcm->private_data = iec;
	pcm->poll_fd = slave->poll_fd;
	pcm->poll_events = slave->poll_events;
	pcm->monotonic = slave->monotonic;
	snd_pcm_set_hw_ptr(pcm, &iec->plug.hw_ptr, -1, 0);
	snd_pcm_set_appl_ptr(pcm, &iec->plug.appl_ptr, -1, 0);
	*pcmp = pcm;

	return 0;
}

/*! \page pcm_plugins

\section pcm_plugins_iec958 Plugin: IEC958

This plugin converts 32bit IEC958 subframe samples to linear, or linear to
32bit IEC958 subframe samples.

\code
pcm.name {
        type iec958             # IEC958 subframe conversion PCM
        slave STR               # Slave name
        # or
        slave {                 # Slave definition
                pcm STR         # Slave PCM name
                # or
                pcm { }         # Slave PCM definition
        }
	[status status-bytes]	# IEC958 status bits (given in byte array)
	# IEC958 preamble bits definitions
	# B/M/W or Z/X/Y, B = block start, M = even subframe, W = odd subframe
	# As default, Z = 0x08, Y = 0x04, X = 0x02
	[preamble.z or preamble.b val]
	[preamble.x or preamble.m val]
	[preamble.y or preamble.w val]
}
\endcode

\subsection pcm_plugins_iec958_funcref Function reference

<UL>
  <LI>snd_pcm_iec958_open()
  <LI>_snd_pcm_iec958_open()
</UL>

*/

/**
 * \brief Creates a new IEC958 subframe conversion PCM
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
int _snd_pcm_iec958_open(snd_pcm_t **pcmp, const char *name,
			 snd_config_t *root, snd_config_t *conf, 
			 snd_pcm_stream_t stream, int mode)
{
	snd_config_iterator_t i, next;
	int err;
	snd_pcm_t *spcm;
	snd_config_t *slave = NULL, *sconf;
	snd_config_t *status = NULL, *preamble = NULL;
	snd_pcm_format_t sformat;
	unsigned char status_bits[24];
	unsigned char preamble_vals[3] = {
		0x08, 0x02, 0x04 /* Z, X, Y */
	};

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
		if (strcmp(id, "status") == 0) {
			if (snd_config_get_type(n) != SND_CONFIG_TYPE_COMPOUND) {
				SNDERR("Invalid type for %s", id);
				return -EINVAL;
			}
			status = n;
			continue;
		}
		if (strcmp(id, "preamble") == 0) {
			if (snd_config_get_type(n) != SND_CONFIG_TYPE_COMPOUND) {
				SNDERR("Invalid type for %s", id);
				return -EINVAL;
			}
			preamble = n;
			continue;
		}
		SNDERR("Unknown field %s", id);
		return -EINVAL;
	}
	memset(status_bits, 0, sizeof(status_bits));
	if (status) {
		snd_config_iterator_t i, inext;
		int bytes = 0;
		snd_config_for_each(i, inext, status) {
			long val;
			snd_config_t *n = snd_config_iterator_entry(i);
			if (snd_config_get_type(n) != SND_CONFIG_TYPE_INTEGER) {
				SNDERR("invalid IEC958 status bits");
				return -EINVAL;
			}
			err = snd_config_get_integer(n, &val);
			if (err < 0) {
				SNDERR("invalid IEC958 status bits");
				return err;
			}
			status_bits[bytes] = val;
			bytes++;
			if (bytes >= (int)sizeof(status_bits))
				break;
		}
		// fprintf(stderr, "STATUS bits: %02x %02x %02x %02x\n", status_bits[0], status_bits[1], status_bits[2], status_bits[3]);
	}
	if (preamble) {
		snd_config_iterator_t i, inext;
		snd_config_for_each(i, inext, preamble) {
			long val;
			snd_config_t *n = snd_config_iterator_entry(i);
			const char *id;
			int idx;
			if (snd_config_get_id(n, &id) < 0)
				continue;
			if (strcmp(id, "b") == 0 || strcmp(id, "z") == 0)
				idx = PREAMBLE_Z;
			else if (strcmp(id, "m") == 0 || strcmp(id, "x") == 0)
				idx = PREAMBLE_X;
			else if (strcmp(id, "w") == 0 || strcmp(id, "y") == 0)
				idx = PREAMBLE_Y;
			else {
				SNDERR("invalid IEC958 preamble type %s", id);
				return -EINVAL;
			}
			err = snd_config_get_integer(n, &val);
			if (err < 0) {
				SNDERR("invalid IEC958 preamble value");
				return err;
			}
			preamble_vals[idx] = val;
		}
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
	    sformat != SND_PCM_FORMAT_IEC958_SUBFRAME_LE &&
	    sformat != SND_PCM_FORMAT_IEC958_SUBFRAME_BE) {
	    	snd_config_delete(sconf);
		SNDERR("invalid slave format");
		return -EINVAL;
	}
	err = snd_pcm_open_slave(&spcm, root, sconf, stream, mode, conf);
	snd_config_delete(sconf);
	if (err < 0)
		return err;
	err = snd_pcm_iec958_open(pcmp, name, sformat, spcm, 1,
				  status ? status_bits : NULL,
				  preamble_vals);
	if (err < 0)
		snd_pcm_close(spcm);
	return err;
}
#ifndef DOC_HIDDEN
SND_DLSYM_BUILD_VERSION(_snd_pcm_iec958_open, SND_PCM_DLSYM_VERSION);
#endif
