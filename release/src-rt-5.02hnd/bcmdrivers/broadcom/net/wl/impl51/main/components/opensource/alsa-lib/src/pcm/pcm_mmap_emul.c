/**
 * \file pcm/pcm_mmap_emul.c
 * \ingroup PCM_Plugins
 * \brief PCM Mmap-Emulation Plugin Interface
 * \author Takashi Iwai <tiwai@suse.de>
 * \date 2007
 */
/*
 *  PCM - Mmap-Emulation
 *  Copyright (c) 2007 by Takashi Iwai <tiwai@suse.de>
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
  
#include "pcm_local.h"
#include "pcm_generic.h"

#ifndef PIC
/* entry for static linking */
const char *_snd_module_pcm_mmap_emul = "";
#endif

#ifndef DOC_HIDDEN
/*
 *
 */

typedef struct {
	snd_pcm_generic_t gen;
	unsigned int mmap_emul :1;
	snd_pcm_uframes_t hw_ptr;
	snd_pcm_uframes_t appl_ptr;
	snd_pcm_uframes_t start_threshold;
} mmap_emul_t;
#endif

/*
 * here goes a really tricky part; hw_refine falls back to ACCESS_RW_* type
 * when ACCESS_MMAP_* isn't supported by the hardware.
 */
static int snd_pcm_mmap_emul_hw_refine(snd_pcm_t *pcm,
				       snd_pcm_hw_params_t *params)
{
	mmap_emul_t *map = pcm->private_data;
	int err = 0;
	snd_pcm_access_mask_t oldmask =
		*snd_pcm_hw_param_get_mask(params, SND_PCM_HW_PARAM_ACCESS);
	snd_pcm_access_mask_t mask;
	const snd_mask_t *pmask;

	snd_mask_none(&mask);
	err = snd_pcm_hw_refine(map->gen.slave, params);
	if (err < 0) {
		snd_pcm_hw_params_t new = *params;

		/* try to use RW_* */
		if (snd_pcm_access_mask_test(&oldmask,
					     SND_PCM_ACCESS_MMAP_INTERLEAVED) &&
		    !snd_pcm_access_mask_test(&oldmask,
					      SND_PCM_ACCESS_RW_INTERLEAVED))
			snd_pcm_access_mask_set(&mask,
						SND_PCM_ACCESS_RW_INTERLEAVED);
		if (snd_pcm_access_mask_test(&oldmask,
					     SND_PCM_ACCESS_MMAP_NONINTERLEAVED) &&
		    !snd_pcm_access_mask_test(&oldmask,
					      SND_PCM_ACCESS_RW_NONINTERLEAVED))
			snd_pcm_access_mask_set(&mask,
						SND_PCM_ACCESS_RW_NONINTERLEAVED);
		if (snd_pcm_access_mask_empty(&mask))
			return err;
		pmask = snd_pcm_hw_param_get_mask(&new,
						  SND_PCM_HW_PARAM_ACCESS);
		*(snd_mask_t *)pmask = mask;
		err = snd_pcm_hw_refine(map->gen.slave, &new);
		if (err < 0)
			return err;
		*params = new;
	}

	pmask = snd_pcm_hw_param_get_mask(params, SND_PCM_HW_PARAM_ACCESS);
	if (snd_pcm_access_mask_test(pmask, SND_PCM_ACCESS_MMAP_INTERLEAVED) ||
	    snd_pcm_access_mask_test(pmask, SND_PCM_ACCESS_MMAP_NONINTERLEAVED) ||
	    snd_pcm_access_mask_test(pmask, SND_PCM_ACCESS_MMAP_COMPLEX))
		return 0;
	if (snd_pcm_access_mask_test(&mask, SND_PCM_ACCESS_RW_INTERLEAVED)) {
		if (snd_pcm_access_mask_test(pmask,
					     SND_PCM_ACCESS_RW_INTERLEAVED))
			snd_pcm_access_mask_set((snd_pcm_access_mask_t *)pmask,
						SND_PCM_ACCESS_MMAP_INTERLEAVED);
		snd_pcm_access_mask_reset((snd_pcm_access_mask_t *)pmask,
					  SND_PCM_ACCESS_RW_INTERLEAVED);
		params->cmask |= 1<<SND_PCM_HW_PARAM_ACCESS;
	}
	if (snd_pcm_access_mask_test(&mask, SND_PCM_ACCESS_RW_NONINTERLEAVED)) {
		if (snd_pcm_access_mask_test(pmask,
					     SND_PCM_ACCESS_RW_NONINTERLEAVED))
			snd_pcm_access_mask_set((snd_pcm_access_mask_t *)pmask,
						SND_PCM_ACCESS_MMAP_NONINTERLEAVED);
		snd_pcm_access_mask_reset((snd_pcm_access_mask_t *)pmask,
					  SND_PCM_ACCESS_RW_NONINTERLEAVED);
		params->cmask |= 1<<SND_PCM_HW_PARAM_ACCESS;
	}
	if (snd_pcm_access_mask_test(&oldmask, SND_PCM_ACCESS_MMAP_INTERLEAVED)) {
		if (snd_pcm_access_mask_test(&oldmask,
					     SND_PCM_ACCESS_RW_INTERLEAVED)) {
			if (snd_pcm_access_mask_test(pmask,
						     SND_PCM_ACCESS_RW_INTERLEAVED)) {
				snd_pcm_access_mask_set((snd_pcm_access_mask_t *)pmask,
							SND_PCM_ACCESS_MMAP_INTERLEAVED);
				params->cmask |= 1<<SND_PCM_HW_PARAM_ACCESS;
			}
		}
	}
	if (snd_pcm_access_mask_test(&oldmask, SND_PCM_ACCESS_MMAP_NONINTERLEAVED)) {
		if (snd_pcm_access_mask_test(&oldmask,
					     SND_PCM_ACCESS_RW_NONINTERLEAVED)) {
			if (snd_pcm_access_mask_test(pmask,
						     SND_PCM_ACCESS_RW_NONINTERLEAVED)) {
				snd_pcm_access_mask_set((snd_pcm_access_mask_t *)pmask,
							SND_PCM_ACCESS_MMAP_NONINTERLEAVED);
				params->cmask |= 1<<SND_PCM_HW_PARAM_ACCESS;
			}
		}
	}
	return 0;
}

/*
 * hw_params needs a similar hack like hw_refine, but it's much simpler
 * because now snd_pcm_hw_params_t takes only one choice for each item.
 *
 * Here, when the normal hw_params call fails, it turns on the mmap_emul
 * flag and tries to use ACCESS_RW_* mode.
 *
 * In mmap_emul mode, the appl_ptr and hw_ptr are handled individually
 * from the layering slave PCM, and they are sync'ed appropriately in
 * each read/write or avail_update/commit call.
 */
static int snd_pcm_mmap_emul_hw_params(snd_pcm_t *pcm,
				       snd_pcm_hw_params_t *params)
{
	mmap_emul_t *map = pcm->private_data;
	snd_pcm_hw_params_t old = *params;
	snd_pcm_access_t access;
	snd_pcm_access_mask_t oldmask;
	snd_pcm_access_mask_t *pmask;
	int err;

	err = _snd_pcm_hw_params(map->gen.slave, params);
	if (err >= 0) {
		map->mmap_emul = 0;
		return err;
	}

	*params = old;
	pmask = (snd_pcm_access_mask_t *)snd_pcm_hw_param_get_mask(params, SND_PCM_HW_PARAM_ACCESS);
	oldmask = *pmask;
	if (INTERNAL(snd_pcm_hw_params_get_access)(params, &access) < 0)
		goto _err;
	switch (access) {
	case SND_PCM_ACCESS_MMAP_INTERLEAVED:
		snd_pcm_access_mask_reset(pmask,
					  SND_PCM_ACCESS_MMAP_INTERLEAVED);
		snd_pcm_access_mask_set(pmask, SND_PCM_ACCESS_RW_INTERLEAVED);
		break;
	case SND_PCM_ACCESS_MMAP_NONINTERLEAVED:
		snd_pcm_access_mask_reset(pmask,
					  SND_PCM_ACCESS_MMAP_NONINTERLEAVED);
		snd_pcm_access_mask_set(pmask,
					SND_PCM_ACCESS_RW_NONINTERLEAVED);
		break;
	default:
		goto _err;
	}
	err = _snd_pcm_hw_params(map->gen.slave, params);
	if (err < 0)
		goto _err;

	/* need to back the access type to relieve apps */
	*pmask = oldmask;

	/* OK, we do fake */
	map->mmap_emul = 1;
	map->appl_ptr = 0;
	map->hw_ptr = 0;
	snd_pcm_set_hw_ptr(pcm, &map->hw_ptr, -1, 0);
	snd_pcm_set_appl_ptr(pcm, &map->appl_ptr, -1, 0);
	return 0;

 _err:
	err = -errno;
	return err;
}

static int snd_pcm_mmap_emul_sw_params(snd_pcm_t *pcm,
				       snd_pcm_sw_params_t *params)
{
	mmap_emul_t *map = pcm->private_data;
	int err;

	map->start_threshold = params->start_threshold;

	/* HACK: don't auto-start in the slave PCM */
	params->start_threshold = pcm->boundary;
	err = snd_pcm_generic_sw_params(pcm, params);
	if (err < 0)
		return err;
	/* restore the value for this PCM */
	params->start_threshold = map->start_threshold;
	return err;
}

static int snd_pcm_mmap_emul_prepare(snd_pcm_t *pcm)
{
	mmap_emul_t *map = pcm->private_data;
	int err;

	err = snd_pcm_generic_prepare(pcm);
	if (err < 0)
		return err;
	map->hw_ptr = map->appl_ptr = 0;
	return err;
}

static int snd_pcm_mmap_emul_reset(snd_pcm_t *pcm)
{
	mmap_emul_t *map = pcm->private_data;
	int err;

	err = snd_pcm_generic_reset(pcm);
	if (err < 0)
		return err;
	map->hw_ptr = map->appl_ptr = 0;
	return err;
}

static snd_pcm_sframes_t
snd_pcm_mmap_emul_rewind(snd_pcm_t *pcm, snd_pcm_uframes_t frames)
{
	frames = snd_pcm_generic_rewind(pcm, frames);
	if (frames > 0)
		snd_pcm_mmap_appl_backward(pcm, frames);
	return frames;
}

static snd_pcm_sframes_t
snd_pcm_mmap_emul_forward(snd_pcm_t *pcm, snd_pcm_uframes_t frames)
{
	frames = snd_pcm_generic_forward(pcm, frames);
	if (frames > 0)
		snd_pcm_mmap_appl_forward(pcm, frames);
	return frames;
}

/* write out the uncommitted chunk on mmap buffer to the slave PCM */
static snd_pcm_sframes_t
sync_slave_write(snd_pcm_t *pcm)
{
	mmap_emul_t *map = pcm->private_data;
	snd_pcm_t *slave = map->gen.slave;
	snd_pcm_uframes_t offset;
	snd_pcm_sframes_t size;

	/* HACK: don't start stream automatically at commit in mmap mode */
	pcm->start_threshold = pcm->boundary;

	size = map->appl_ptr - *slave->appl.ptr;
	if (size < 0)
		size += pcm->boundary;
	if (size) {
		offset = *slave->appl.ptr % pcm->buffer_size;
		size = snd_pcm_write_mmap(pcm, offset, size);
	}
	pcm->start_threshold = map->start_threshold; /* restore */
	return size;
}

/* read the available chunk on the slave PCM to mmap buffer */
static snd_pcm_sframes_t
sync_slave_read(snd_pcm_t *pcm)
{
	mmap_emul_t *map = pcm->private_data;
	snd_pcm_t *slave = map->gen.slave;
	snd_pcm_uframes_t offset;
	snd_pcm_sframes_t size;

	size = *slave->hw.ptr - map->hw_ptr;
	if (size < 0)
		size += pcm->boundary;
	if (!size)
		return 0;
	offset = map->hw_ptr % pcm->buffer_size;
	size = snd_pcm_read_mmap(pcm, offset, size);
	if (size > 0)
		snd_pcm_mmap_hw_forward(pcm, size);
	return 0;
}

static snd_pcm_sframes_t
snd_pcm_mmap_emul_mmap_commit(snd_pcm_t *pcm, snd_pcm_uframes_t offset,
			      snd_pcm_uframes_t size)
{
	mmap_emul_t *map = pcm->private_data;
	snd_pcm_t *slave = map->gen.slave;

	if (!map->mmap_emul)
		return snd_pcm_mmap_commit(slave, offset, size);
	snd_pcm_mmap_appl_forward(pcm, size);
	if (pcm->stream == SND_PCM_STREAM_PLAYBACK)
		sync_slave_write(pcm);
	return size;
}

static snd_pcm_sframes_t snd_pcm_mmap_emul_avail_update(snd_pcm_t *pcm)
{
	mmap_emul_t *map = pcm->private_data;
	snd_pcm_t *slave = map->gen.slave;
	snd_pcm_sframes_t avail;

	avail = snd_pcm_avail_update(slave);
	if (!map->mmap_emul)
		return avail;

	if (pcm->stream == SND_PCM_STREAM_PLAYBACK)
		map->hw_ptr = *slave->hw.ptr;
	else
		sync_slave_read(pcm);
	return snd_pcm_mmap_avail(pcm);
}

static void snd_pcm_mmap_emul_dump(snd_pcm_t *pcm, snd_output_t *out)
{
	mmap_emul_t *map = pcm->private_data;

	snd_output_printf(out, "Mmap emulation PCM\n");
	if (pcm->setup) {
		snd_output_printf(out, "Its setup is:\n");
		snd_pcm_dump_setup(pcm, out);
	}
	snd_output_printf(out, "Slave: ");
	snd_pcm_dump(map->gen.slave, out);
}

static const snd_pcm_ops_t snd_pcm_mmap_emul_ops = {
	.close = snd_pcm_generic_close,
	.info = snd_pcm_generic_info,
	.hw_refine = snd_pcm_mmap_emul_hw_refine,
	.hw_params = snd_pcm_mmap_emul_hw_params,
	.hw_free = snd_pcm_generic_hw_free,
	.sw_params = snd_pcm_mmap_emul_sw_params,
	.channel_info = snd_pcm_generic_channel_info,
	.dump = snd_pcm_mmap_emul_dump,
	.nonblock = snd_pcm_generic_nonblock,
	.async = snd_pcm_generic_async,
	.mmap = snd_pcm_generic_mmap,
	.munmap = snd_pcm_generic_munmap,
};

static const snd_pcm_fast_ops_t snd_pcm_mmap_emul_fast_ops = {
	.status = snd_pcm_generic_status,
	.state = snd_pcm_generic_state,
	.hwsync = snd_pcm_generic_hwsync,
	.delay = snd_pcm_generic_delay,
	.prepare = snd_pcm_mmap_emul_prepare,
	.reset = snd_pcm_mmap_emul_reset,
	.start = snd_pcm_generic_start,
	.drop = snd_pcm_generic_drop,
	.drain = snd_pcm_generic_drain,
	.pause = snd_pcm_generic_pause,
	.rewindable = snd_pcm_generic_rewindable,
	.rewind = snd_pcm_mmap_emul_rewind,
	.forwardable = snd_pcm_generic_forwardable,
	.forward = snd_pcm_mmap_emul_forward,
	.resume = snd_pcm_generic_resume,
	.link = snd_pcm_generic_link,
	.link_slaves = snd_pcm_generic_link_slaves,
	.unlink = snd_pcm_generic_unlink,
	.writei = snd_pcm_generic_writei,
	.writen = snd_pcm_generic_writen,
	.readi = snd_pcm_generic_readi,
	.readn = snd_pcm_generic_readn,
	.avail_update = snd_pcm_mmap_emul_avail_update,
	.mmap_commit = snd_pcm_mmap_emul_mmap_commit,
	.htimestamp = snd_pcm_generic_htimestamp,
	.poll_descriptors = snd_pcm_generic_poll_descriptors,
	.poll_descriptors_count = snd_pcm_generic_poll_descriptors_count,
	.poll_revents = snd_pcm_generic_poll_revents,
};

#ifndef DOC_HIDDEN
int __snd_pcm_mmap_emul_open(snd_pcm_t **pcmp, const char *name,
			     snd_pcm_t *slave, int close_slave)
{
	snd_pcm_t *pcm;
	mmap_emul_t *map;
	int err;

	map = calloc(1, sizeof(*map));
	if (!map)
		return -ENOMEM;
	map->gen.slave = slave;
	map->gen.close_slave = close_slave;

	err = snd_pcm_new(&pcm, SND_PCM_TYPE_MMAP_EMUL, name,
			  slave->stream, slave->mode);
	if (err < 0) {
		free(map);
		return err;
	}
	pcm->ops = &snd_pcm_mmap_emul_ops;
	pcm->fast_ops = &snd_pcm_mmap_emul_fast_ops;
	pcm->private_data = map;
	pcm->poll_fd = slave->poll_fd;
	pcm->poll_events = slave->poll_events;
	pcm->monotonic = slave->monotonic;
	snd_pcm_set_hw_ptr(pcm, &map->hw_ptr, -1, 0);
	snd_pcm_set_appl_ptr(pcm, &map->appl_ptr, -1, 0);
	*pcmp = pcm;

	return 0;
}
#endif

/*! \page pcm_plugins

\section pcm_plugins_mmap_emul Plugin: mmap_emul

\code
pcm.name {
	type mmap_emul
	slave PCM
}
\endcode

\subsection pcm_plugins_mmap_emul_funcref Function reference

<UL>
  <LI>_snd_pcm_hw_open()
</UL>

*/

/**
 * \brief Creates a new mmap_emul PCM
 * \param pcmp Returns created PCM handle
 * \param name Name of PCM
 * \param root Root configuration node
 * \param conf Configuration node with hw PCM description
 * \param stream PCM Stream
 * \param mode PCM Mode
 * \warning Using of this function might be dangerous in the sense
 *          of compatibility reasons. The prototype might be freely
 *          changed in future.
 */
int _snd_pcm_mmap_emul_open(snd_pcm_t **pcmp, const char *name,
			    snd_config_t *root ATTRIBUTE_UNUSED,
			    snd_config_t *conf,
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
	err = __snd_pcm_mmap_emul_open(pcmp, name, spcm, 1);
	if (err < 0)
		snd_pcm_close(spcm);
	return err;
}

#ifndef DOC_HIDDEN
SND_DLSYM_BUILD_VERSION(_snd_pcm_mmap_emul_open, SND_PCM_DLSYM_VERSION);
#endif
