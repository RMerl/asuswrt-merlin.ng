/**
 * \file pcm/pcm_null.c
 * \ingroup PCM_Plugins
 * \brief PCM Null Plugin Interface
 * \author Abramo Bagnara <abramo@alsa-project.org>
 * \date 2000-2001
 */
/*
 *  PCM - Null plugin
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
#include <limits.h>
#include <sys/shm.h>
#include "pcm_local.h"
#include "pcm_plugin.h"

#ifndef PIC
/* entry for static linking */
const char *_snd_module_pcm_null = "";
#endif

#ifndef DOC_HIDDEN
typedef struct {
	snd_htimestamp_t trigger_tstamp;
	snd_pcm_state_t state;
	snd_pcm_uframes_t appl_ptr;
	snd_pcm_uframes_t hw_ptr;
	int poll_fd;
} snd_pcm_null_t;
#endif

static int snd_pcm_null_close(snd_pcm_t *pcm)
{
	snd_pcm_null_t *null = pcm->private_data;
	close(null->poll_fd);
	free(null);
	return 0;
}

static int snd_pcm_null_nonblock(snd_pcm_t *pcm ATTRIBUTE_UNUSED, int nonblock ATTRIBUTE_UNUSED)
{
	return 0;
}

static int snd_pcm_null_async(snd_pcm_t *pcm ATTRIBUTE_UNUSED, int sig ATTRIBUTE_UNUSED, pid_t pid ATTRIBUTE_UNUSED)
{
	return -ENOSYS;
}

static int snd_pcm_null_info(snd_pcm_t *pcm, snd_pcm_info_t * info)
{
	memset(info, 0, sizeof(*info));
	info->stream = pcm->stream;
	info->card = -1;
	if (pcm->name) {
		strncpy((char *)info->id, pcm->name, sizeof(info->id));
		strncpy((char *)info->name, pcm->name, sizeof(info->name));
		strncpy((char *)info->subname, pcm->name, sizeof(info->subname));
	}
	info->subdevices_count = 1;
	return 0;
}

static int snd_pcm_null_status(snd_pcm_t *pcm, snd_pcm_status_t * status)
{
	snd_pcm_null_t *null = pcm->private_data;
	memset(status, 0, sizeof(*status));
	status->state = null->state;
	status->trigger_tstamp = null->trigger_tstamp;
	gettimestamp(&status->tstamp, pcm->monotonic);
	status->avail = pcm->buffer_size;
	status->avail_max = status->avail;
	return 0;
}

static snd_pcm_state_t snd_pcm_null_state(snd_pcm_t *pcm)
{
	snd_pcm_null_t *null = pcm->private_data;
	return null->state;
}

static int snd_pcm_null_hwsync(snd_pcm_t *pcm ATTRIBUTE_UNUSED)
{
	return 0;
}

static int snd_pcm_null_delay(snd_pcm_t *pcm ATTRIBUTE_UNUSED, snd_pcm_sframes_t *delayp)
{
	*delayp = 0;
	return 0;
}

static int snd_pcm_null_prepare(snd_pcm_t *pcm)
{
	snd_pcm_null_t *null = pcm->private_data;
	null->state = SND_PCM_STATE_PREPARED;
	*pcm->appl.ptr = 0;
	*pcm->hw.ptr = 0;
	return 0;
}

static int snd_pcm_null_reset(snd_pcm_t *pcm)
{
	*pcm->appl.ptr = 0;
	*pcm->hw.ptr = 0;
	return 0;
}

static int snd_pcm_null_start(snd_pcm_t *pcm)
{
	snd_pcm_null_t *null = pcm->private_data;
	assert(null->state == SND_PCM_STATE_PREPARED);
	null->state = SND_PCM_STATE_RUNNING;
	if (pcm->stream == SND_PCM_STREAM_CAPTURE)
		*pcm->hw.ptr = *pcm->appl.ptr + pcm->buffer_size;
	else
		*pcm->hw.ptr = *pcm->appl.ptr;
	return 0;
}

static int snd_pcm_null_drop(snd_pcm_t *pcm)
{
	snd_pcm_null_t *null = pcm->private_data;
	assert(null->state != SND_PCM_STATE_OPEN);
	null->state = SND_PCM_STATE_SETUP;
	return 0;
}

static int snd_pcm_null_drain(snd_pcm_t *pcm)
{
	snd_pcm_null_t *null = pcm->private_data;
	assert(null->state != SND_PCM_STATE_OPEN);
	null->state = SND_PCM_STATE_SETUP;
	return 0;
}

static int snd_pcm_null_pause(snd_pcm_t *pcm, int enable)
{
	snd_pcm_null_t *null = pcm->private_data;
	if (enable) {
		if (null->state != SND_PCM_STATE_RUNNING)
			return -EBADFD;
		null->state = SND_PCM_STATE_PAUSED;
	} else {
		if (null->state != SND_PCM_STATE_PAUSED)
			return -EBADFD;
		null->state = SND_PCM_STATE_RUNNING;
	}
	return 0;
}

static snd_pcm_sframes_t snd_pcm_null_rewind(snd_pcm_t *pcm, snd_pcm_uframes_t frames)
{
	snd_pcm_null_t *null = pcm->private_data;
	switch (null->state) {
	case SND_PCM_STATE_RUNNING:
		snd_pcm_mmap_hw_backward(pcm, frames);
		/* Fall through */
	case SND_PCM_STATE_PREPARED:
		snd_pcm_mmap_appl_backward(pcm, frames);
		return frames;
	default:
		return -EBADFD;
	}
}

static snd_pcm_sframes_t snd_pcm_null_forward(snd_pcm_t *pcm, snd_pcm_uframes_t frames)
{
	snd_pcm_null_t *null = pcm->private_data;
	switch (null->state) {
	case SND_PCM_STATE_RUNNING:
		snd_pcm_mmap_hw_forward(pcm, frames);
		/* Fall through */
	case SND_PCM_STATE_PREPARED:
		snd_pcm_mmap_appl_forward(pcm, frames);
		return frames;
	default:
		return -EBADFD;
	}
}

static int snd_pcm_null_resume(snd_pcm_t *pcm ATTRIBUTE_UNUSED)
{
	return 0;
}

static snd_pcm_sframes_t snd_pcm_null_xfer_areas(snd_pcm_t *pcm,
						 const snd_pcm_channel_area_t *areas ATTRIBUTE_UNUSED,
						 snd_pcm_uframes_t offset ATTRIBUTE_UNUSED,
						 snd_pcm_uframes_t size)
{
	snd_pcm_mmap_appl_forward(pcm, size);
	snd_pcm_mmap_hw_forward(pcm, size);
	return size;
}

static snd_pcm_sframes_t snd_pcm_null_writei(snd_pcm_t *pcm, const void *buffer ATTRIBUTE_UNUSED, snd_pcm_uframes_t size)
{
	return snd_pcm_write_areas(pcm, NULL, 0, size, snd_pcm_null_xfer_areas);
}

static snd_pcm_sframes_t snd_pcm_null_writen(snd_pcm_t *pcm, void **bufs ATTRIBUTE_UNUSED, snd_pcm_uframes_t size)
{
	return snd_pcm_write_areas(pcm, NULL, 0, size, snd_pcm_null_xfer_areas);
}

static snd_pcm_sframes_t snd_pcm_null_readi(snd_pcm_t *pcm, void *buffer ATTRIBUTE_UNUSED, snd_pcm_uframes_t size)
{
	return snd_pcm_read_areas(pcm, NULL, 0, size, snd_pcm_null_xfer_areas);
}

static snd_pcm_sframes_t snd_pcm_null_readn(snd_pcm_t *pcm, void **bufs ATTRIBUTE_UNUSED, snd_pcm_uframes_t size)
{
	return snd_pcm_read_areas(pcm, NULL, 0, size, snd_pcm_null_xfer_areas);
}

static snd_pcm_sframes_t snd_pcm_null_mmap_commit(snd_pcm_t *pcm,
						  snd_pcm_uframes_t offset ATTRIBUTE_UNUSED,
						  snd_pcm_uframes_t size)
{
	return snd_pcm_null_forward(pcm, size);
}

static snd_pcm_sframes_t snd_pcm_null_avail_update(snd_pcm_t *pcm)
{
	return pcm->buffer_size;
}

static int snd_pcm_null_hw_refine(snd_pcm_t *pcm ATTRIBUTE_UNUSED, snd_pcm_hw_params_t *params)
{
	int err = snd_pcm_hw_refine_soft(pcm, params);
	params->info = SND_PCM_INFO_MMAP | SND_PCM_INFO_MMAP_VALID |
		       SND_PCM_INFO_RESUME | SND_PCM_INFO_PAUSE;
	params->fifo_size = 0;
	return err;
}

static int snd_pcm_null_hw_params(snd_pcm_t *pcm ATTRIBUTE_UNUSED, snd_pcm_hw_params_t * params ATTRIBUTE_UNUSED)
{
	return 0;
}

static int snd_pcm_null_hw_free(snd_pcm_t *pcm ATTRIBUTE_UNUSED)
{
	return 0;
}

static int snd_pcm_null_sw_params(snd_pcm_t *pcm ATTRIBUTE_UNUSED, snd_pcm_sw_params_t * params ATTRIBUTE_UNUSED)
{
	return 0;
}

static void snd_pcm_null_dump(snd_pcm_t *pcm, snd_output_t *out)
{
	snd_output_printf(out, "Null PCM\n");
	if (pcm->setup) {
		snd_output_printf(out, "Its setup is:\n");
		snd_pcm_dump_setup(pcm, out);
	}
}

static const snd_pcm_ops_t snd_pcm_null_ops = {
	.close = snd_pcm_null_close,
	.info = snd_pcm_null_info,
	.hw_refine = snd_pcm_null_hw_refine,
	.hw_params = snd_pcm_null_hw_params,
	.hw_free = snd_pcm_null_hw_free,
	.sw_params = snd_pcm_null_sw_params,
	.channel_info = snd_pcm_generic_channel_info,
	.dump = snd_pcm_null_dump,
	.nonblock = snd_pcm_null_nonblock,
	.async = snd_pcm_null_async,
	.mmap = snd_pcm_generic_mmap,
	.munmap = snd_pcm_generic_munmap,
};

static const snd_pcm_fast_ops_t snd_pcm_null_fast_ops = {
	.status = snd_pcm_null_status,
	.state = snd_pcm_null_state,
	.hwsync = snd_pcm_null_hwsync,
	.delay = snd_pcm_null_delay,
	.prepare = snd_pcm_null_prepare,
	.reset = snd_pcm_null_reset,
	.start = snd_pcm_null_start,
	.drop = snd_pcm_null_drop,
	.drain = snd_pcm_null_drain,
	.pause = snd_pcm_null_pause,
	.rewind = snd_pcm_null_rewind,
	.forward = snd_pcm_null_forward,
	.resume = snd_pcm_null_resume,
	.writei = snd_pcm_null_writei,
	.writen = snd_pcm_null_writen,
	.readi = snd_pcm_null_readi,
	.readn = snd_pcm_null_readn,
	.avail_update = snd_pcm_null_avail_update,
	.mmap_commit = snd_pcm_null_mmap_commit,
	.htimestamp = snd_pcm_generic_real_htimestamp,
};

/**
 * \brief Creates a new null PCM
 * \param pcmp Returns created PCM handle
 * \param name Name of PCM
 * \param stream Stream type
 * \param mode Stream mode
 * \retval zero on success otherwise a negative error code
 * \warning Using of this function might be dangerous in the sense
 *          of compatibility reasons. The prototype might be freely
 *          changed in future.
 */
int snd_pcm_null_open(snd_pcm_t **pcmp, const char *name, snd_pcm_stream_t stream, int mode)
{
	snd_pcm_t *pcm;
	snd_pcm_null_t *null;
	int fd;
	int err;
	assert(pcmp);
	if (stream == SND_PCM_STREAM_PLAYBACK) {
		fd = open("/dev/null", O_WRONLY);
		if (fd < 0) {
			SYSERR("Cannot open /dev/null");
			return -errno;
		}
	} else {
		fd = open("/dev/full", O_RDONLY);
		if (fd < 0) {
			SYSERR("Cannot open /dev/full");
			return -errno;
		}
	}
	null = calloc(1, sizeof(snd_pcm_null_t));
	if (!null) {
		close(fd);
		return -ENOMEM;
	}
	null->poll_fd = fd;
	null->state = SND_PCM_STATE_OPEN;
	
	err = snd_pcm_new(&pcm, SND_PCM_TYPE_NULL, name, stream, mode);
	if (err < 0) {
		close(fd);
		free(null);
		return err;
	}
	pcm->ops = &snd_pcm_null_ops;
	pcm->fast_ops = &snd_pcm_null_fast_ops;
	pcm->private_data = null;
	pcm->poll_fd = fd;
	pcm->poll_events = stream == SND_PCM_STREAM_PLAYBACK ? POLLOUT : POLLIN;
	snd_pcm_set_hw_ptr(pcm, &null->hw_ptr, -1, 0);
	snd_pcm_set_appl_ptr(pcm, &null->appl_ptr, -1, 0);
	*pcmp = pcm;

	return 0;
}

/*! \page pcm_plugins

\section pcm_plugins_null Plugin: Null

This plugin discards contents of a PCM stream or creates a stream with zero
samples.

Note: This implementation uses devices /dev/null (playback, must be writable)
and /dev/full (capture, must be readable).

\code
pcm.name {
        type null               # Null PCM
}
\endcode

\subsection pcm_plugins_null_funcref Function reference

<UL>
  <LI>snd_pcm_null_open()
  <LI>_snd_pcm_null_open()
</UL>

*/

/**
 * \brief Creates a new Null PCM
 * \param pcmp Returns created PCM handle
 * \param name Name of PCM
 * \param root Root configuration node
 * \param conf Configuration node with Null PCM description
 * \param stream Stream type
 * \param mode Stream mode
 * \retval zero on success otherwise a negative error code
 * \warning Using of this function might be dangerous in the sense
 *          of compatibility reasons. The prototype might be freely
 *          changed in future.
 */
int _snd_pcm_null_open(snd_pcm_t **pcmp, const char *name,
		       snd_config_t *root ATTRIBUTE_UNUSED, snd_config_t *conf, 
		       snd_pcm_stream_t stream, int mode)
{
	snd_config_iterator_t i, next;
	snd_config_for_each(i, next, conf) {
		snd_config_t *n = snd_config_iterator_entry(i);
		const char *id;
		if (snd_config_get_id(n, &id) < 0)
			continue;
		if (snd_pcm_conf_generic_id(id))
			continue;
		SNDERR("Unknown field %s", id);
		return -EINVAL;
	}
	return snd_pcm_null_open(pcmp, name, stream, mode);
}
#ifndef DOC_HIDDEN
SND_DLSYM_BUILD_VERSION(_snd_pcm_null_open, SND_PCM_DLSYM_VERSION);
#endif
