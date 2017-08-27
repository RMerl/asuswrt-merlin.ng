/**
 * \file pcm/pcm_copy.c
 * \ingroup PCM_Plugins
 * \brief PCM Copy Plugin Interface
 * \author Abramo Bagnara <abramo@alsa-project.org>
 * \date 2000-2001
 */
/*
 *  PCM - Copy conversion
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

#ifndef PIC
/* entry for static linking */
const char *_snd_module_pcm_copy = "";
#endif

#ifndef DOC_HIDDEN
typedef struct {
	/* This field need to be the first */
	snd_pcm_plugin_t plug;
} snd_pcm_copy_t;
#endif

static int snd_pcm_copy_hw_refine_cprepare(snd_pcm_t *pcm ATTRIBUTE_UNUSED, snd_pcm_hw_params_t *params)
{
	int err;
	snd_pcm_access_mask_t access_mask = { SND_PCM_ACCBIT_SHM };
	err = _snd_pcm_hw_param_set_mask(params, SND_PCM_HW_PARAM_ACCESS,
					 &access_mask);
	if (err < 0)
		return err;
	params->info &= ~(SND_PCM_INFO_MMAP | SND_PCM_INFO_MMAP_VALID);
	return 0;
}

static int snd_pcm_copy_hw_refine_sprepare(snd_pcm_t *pcm ATTRIBUTE_UNUSED, snd_pcm_hw_params_t *sparams)
{
	snd_pcm_access_mask_t saccess_mask = { SND_PCM_ACCBIT_MMAP };
	_snd_pcm_hw_params_any(sparams);
	_snd_pcm_hw_param_set_mask(sparams, SND_PCM_HW_PARAM_ACCESS,
				   &saccess_mask);
	return 0;
}

static int snd_pcm_copy_hw_refine_schange(snd_pcm_t *pcm ATTRIBUTE_UNUSED, snd_pcm_hw_params_t *params,
					  snd_pcm_hw_params_t *sparams)
{
	int err;
	unsigned int links = ~SND_PCM_HW_PARBIT_ACCESS;
	err = _snd_pcm_hw_params_refine(sparams, links, params);
	if (err < 0)
		return err;
	return 0;
}
	
static int snd_pcm_copy_hw_refine_cchange(snd_pcm_t *pcm ATTRIBUTE_UNUSED, snd_pcm_hw_params_t *params,
					  snd_pcm_hw_params_t *sparams)
{
	int err;
	unsigned int links = ~SND_PCM_HW_PARBIT_ACCESS;
	err = _snd_pcm_hw_params_refine(params, links, sparams);
	if (err < 0)
		return err;
	return 0;
}

static int snd_pcm_copy_hw_refine(snd_pcm_t *pcm, snd_pcm_hw_params_t *params)
{
	return snd_pcm_hw_refine_slave(pcm, params,
				       snd_pcm_copy_hw_refine_cprepare,
				       snd_pcm_copy_hw_refine_cchange,
				       snd_pcm_copy_hw_refine_sprepare,
				       snd_pcm_copy_hw_refine_schange,
				       snd_pcm_generic_hw_refine);
}

static int snd_pcm_copy_hw_params(snd_pcm_t *pcm, snd_pcm_hw_params_t *params)
{
	return snd_pcm_hw_params_slave(pcm, params,
				       snd_pcm_copy_hw_refine_cchange,
				       snd_pcm_copy_hw_refine_sprepare,
				       snd_pcm_copy_hw_refine_schange,
				       snd_pcm_generic_hw_params);
}

static snd_pcm_uframes_t
snd_pcm_copy_write_areas(snd_pcm_t *pcm,
			 const snd_pcm_channel_area_t *areas,
			 snd_pcm_uframes_t offset,
			 snd_pcm_uframes_t size,
			 const snd_pcm_channel_area_t *slave_areas,
			 snd_pcm_uframes_t slave_offset,
			 snd_pcm_uframes_t *slave_sizep)
{
	if (size > *slave_sizep)
		size = *slave_sizep;
	snd_pcm_areas_copy(slave_areas, slave_offset,
			   areas, offset,
			   pcm->channels, size, pcm->format);
	*slave_sizep = size;
	return size;
}

static snd_pcm_uframes_t
snd_pcm_copy_read_areas(snd_pcm_t *pcm,
			const snd_pcm_channel_area_t *areas,
			snd_pcm_uframes_t offset,
			snd_pcm_uframes_t size,
			const snd_pcm_channel_area_t *slave_areas,
			snd_pcm_uframes_t slave_offset,
			snd_pcm_uframes_t *slave_sizep)
{
	if (size > *slave_sizep)
		size = *slave_sizep;
	snd_pcm_areas_copy(areas, offset, 
			   slave_areas, slave_offset,
			   pcm->channels, size, pcm->format);
	*slave_sizep = size;
	return size;
}

static void snd_pcm_copy_dump(snd_pcm_t *pcm, snd_output_t *out)
{
	snd_pcm_copy_t *copy = pcm->private_data;
	snd_output_printf(out, "Copy conversion PCM\n");
	if (pcm->setup) {
		snd_output_printf(out, "Its setup is:\n");
		snd_pcm_dump_setup(pcm, out);
	}
	snd_output_printf(out, "Slave: ");
	snd_pcm_dump(copy->plug.gen.slave, out);
}

static const snd_pcm_ops_t snd_pcm_copy_ops = {
	.close = snd_pcm_generic_close,
	.info = snd_pcm_generic_info,
	.hw_refine = snd_pcm_copy_hw_refine,
	.hw_params = snd_pcm_copy_hw_params,
	.hw_free = snd_pcm_generic_hw_free,
	.sw_params = snd_pcm_generic_sw_params,
	.channel_info = snd_pcm_generic_channel_info,
	.dump = snd_pcm_copy_dump,
	.nonblock = snd_pcm_generic_nonblock,
	.async = snd_pcm_generic_async,
	.mmap = snd_pcm_generic_mmap,
	.munmap = snd_pcm_generic_munmap,
};

/**
 * \brief Creates a new copy PCM
 * \param pcmp Returns created PCM handle
 * \param name Name of PCM
 * \param slave Slave PCM handle
 * \param close_slave When set, the slave PCM handle is closed with copy PCM
 * \retval zero on success otherwise a negative error code
 * \warning Using of this function might be dangerous in the sense
 *          of compatibility reasons. The prototype might be freely
 *          changed in future.
 */
int snd_pcm_copy_open(snd_pcm_t **pcmp, const char *name, snd_pcm_t *slave, int close_slave)
{
	snd_pcm_t *pcm;
	snd_pcm_copy_t *copy;
	int err;
	assert(pcmp && slave);
	copy = calloc(1, sizeof(snd_pcm_copy_t));
	if (!copy) {
		return -ENOMEM;
	}
	snd_pcm_plugin_init(&copy->plug);
	copy->plug.read = snd_pcm_copy_read_areas;
	copy->plug.write = snd_pcm_copy_write_areas;
	copy->plug.undo_read = snd_pcm_plugin_undo_read_generic;
	copy->plug.undo_write = snd_pcm_plugin_undo_write_generic;
	copy->plug.gen.slave = slave;
	copy->plug.gen.close_slave = close_slave;

	err = snd_pcm_new(&pcm, SND_PCM_TYPE_COPY, name, slave->stream, slave->mode);
	if (err < 0) {
		free(copy);
		return err;
	}
	pcm->ops = &snd_pcm_copy_ops;
	pcm->fast_ops = &snd_pcm_plugin_fast_ops;
	pcm->private_data = copy;
	pcm->poll_fd = slave->poll_fd;
	pcm->poll_events = slave->poll_events;
	pcm->monotonic = slave->monotonic;
	snd_pcm_set_hw_ptr(pcm, &copy->plug.hw_ptr, -1, 0);
	snd_pcm_set_appl_ptr(pcm, &copy->plug.appl_ptr, -1, 0);
	*pcmp = pcm;

	return 0;
}

/*! \page pcm_plugins

\section pcm_plugins_copy Plugin: copy

This plugin copies samples from master copy PCM to given slave PCM.
The channel count, format and rate must match for both of them. 

\code
pcm.name {
	type copy		# Copy PCM
	slave STR		# Slave name
	# or
	slave {			# Slave definition
		pcm STR		# Slave PCM name
		# or
		pcm { }		# Slave PCM definition
	}
}
\endcode

\subsection pcm_plugins_copy_funcref Function reference

<UL>
  <LI>snd_pcm_copy_open()
  <LI>_snd_pcm_copy_open()
</UL>

*/

/**
 * \brief Creates a new copy PCM
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
int _snd_pcm_copy_open(snd_pcm_t **pcmp, const char *name,
		       snd_config_t *root, snd_config_t *conf, 
		       snd_pcm_stream_t stream, int mode)
{
	snd_config_iterator_t i, next;
	int err;
	snd_pcm_t *spcm;
	snd_config_t *slave = NULL, *sconf;
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
	err = snd_pcm_slave_conf(root, slave, &sconf, 0);
	if (err < 0)
		return err;
	err = snd_pcm_open_slave(&spcm, root, sconf, stream, mode, conf);
	snd_config_delete(sconf);
	if (err < 0)
		return err;
	err = snd_pcm_copy_open(pcmp, name, spcm, 1);
	if (err < 0)
		snd_pcm_close(spcm);
	return err;
}
#ifndef DOC_HIDDEN
SND_DLSYM_BUILD_VERSION(_snd_pcm_copy_open, SND_PCM_DLSYM_VERSION);
#endif
