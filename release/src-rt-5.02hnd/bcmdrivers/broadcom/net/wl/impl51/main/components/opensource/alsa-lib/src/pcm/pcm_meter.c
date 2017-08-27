/**
 * \file pcm/pcm_meter.c
 * \brief Helper functions for #SND_PCM_TYPE_METER PCM scopes
 * \author Abramo Bagnara <abramo@alsa-project.org>
 * \date 2001
 *
 * Helper functions for #SND_PCM_TYPE_METER PCM scopes
 */
/*
 *  PCM - Meter plugin
 *  Copyright (c) 2001 by Abramo Bagnara <abramo@alsa-project.org>
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
#include <time.h>
#include <pthread.h>
#include <dlfcn.h>
#include "pcm_local.h"
#include "pcm_plugin.h"
#include "iatomic.h"

#ifndef PIC
/* entry for static linking */
const char *_snd_module_pcm_meter = "";
#endif

#ifndef DOC_HIDDEN
#define FREQUENCY 50

struct _snd_pcm_scope {
	int enabled;
	char *name;
	const snd_pcm_scope_ops_t *ops;
	void *private_data;
	struct list_head list;
};

typedef struct _snd_pcm_meter {
	snd_pcm_generic_t gen;
	snd_pcm_uframes_t rptr;
	snd_pcm_uframes_t buf_size;
	snd_pcm_channel_area_t *buf_areas;
	snd_pcm_uframes_t now;
	unsigned char *buf;
	struct list_head scopes;
	int closed;
	int running;
	atomic_t reset;
	pthread_t thread;
	pthread_mutex_t update_mutex;
	pthread_mutex_t running_mutex;
	pthread_cond_t running_cond;
	struct timespec delay;
	void *dl_handle;
} snd_pcm_meter_t;

static void snd_pcm_meter_add_frames(snd_pcm_t *pcm,
				     const snd_pcm_channel_area_t *areas,
				     snd_pcm_uframes_t ptr,
				     snd_pcm_uframes_t frames)
{
	snd_pcm_meter_t *meter = pcm->private_data;
	while (frames > 0) {
		snd_pcm_uframes_t n = frames;
		snd_pcm_uframes_t dst_offset = ptr % meter->buf_size;
		snd_pcm_uframes_t src_offset = ptr % pcm->buffer_size;
		snd_pcm_uframes_t dst_cont = meter->buf_size - dst_offset;
		snd_pcm_uframes_t src_cont = pcm->buffer_size - src_offset;
		if (n > dst_cont)
			n = dst_cont;
		if (n > src_cont)
			n = src_cont;
		snd_pcm_areas_copy(meter->buf_areas, dst_offset, 
				   areas, src_offset,
				   pcm->channels, n, pcm->format);
		frames -= n;
		ptr += n;
		if (ptr == pcm->boundary)
			ptr = 0;
	}
}

static void snd_pcm_meter_update_main(snd_pcm_t *pcm)
{
	snd_pcm_meter_t *meter = pcm->private_data;
	snd_pcm_sframes_t frames;
	snd_pcm_uframes_t rptr, old_rptr;
	const snd_pcm_channel_area_t *areas;
	int locked;
	locked = (pthread_mutex_trylock(&meter->update_mutex) >= 0);
	areas = snd_pcm_mmap_areas(pcm);
	rptr = *pcm->hw.ptr;
	old_rptr = meter->rptr;
	meter->rptr = rptr;
	frames = rptr - old_rptr;
	if (frames < 0)
		frames += pcm->boundary;
	if (frames > 0) {
		assert((snd_pcm_uframes_t) frames <= pcm->buffer_size);
		snd_pcm_meter_add_frames(pcm, areas, old_rptr,
					 (snd_pcm_uframes_t) frames);
	}
	if (locked)
		pthread_mutex_unlock(&meter->update_mutex);
}

static int snd_pcm_meter_update_scope(snd_pcm_t *pcm)
{
	snd_pcm_meter_t *meter = pcm->private_data;
	snd_pcm_sframes_t frames;
	snd_pcm_uframes_t rptr, old_rptr;
	const snd_pcm_channel_area_t *areas;
	int reset = 0;
	/* Wait main thread */
	pthread_mutex_lock(&meter->update_mutex);
	areas = snd_pcm_mmap_areas(pcm);
 _again:
	rptr = *pcm->hw.ptr;
	old_rptr = meter->rptr;
	rmb();
	if (atomic_read(&meter->reset)) {
		reset = 1;
		atomic_dec(&meter->reset);
		goto _again;
	}
	meter->rptr = rptr;
	frames = rptr - old_rptr;
	if (frames < 0)
		frames += pcm->boundary;
	if (frames > 0) {
		assert((snd_pcm_uframes_t) frames <= pcm->buffer_size);
		snd_pcm_meter_add_frames(pcm, areas, old_rptr,
					 (snd_pcm_uframes_t) frames);
	}
	pthread_mutex_unlock(&meter->update_mutex);
	return reset;
}

static int snd_pcm_scope_remove(snd_pcm_scope_t *scope)
{
	free(scope->name);
	scope->ops->close(scope);
	list_del(&scope->list);
	free(scope);
	return 0;
}

static int snd_pcm_scope_enable(snd_pcm_scope_t *scope)
{
	int err;
	assert(!scope->enabled);
	err = scope->ops->enable(scope);
	scope->enabled = (err >= 0);
	return err;
}

static int snd_pcm_scope_disable(snd_pcm_scope_t *scope)
{
	assert(scope->enabled);
	scope->ops->disable(scope);
	scope->enabled = 0;
	return 0;
}

static void *snd_pcm_meter_thread(void *data)
{
	snd_pcm_t *pcm = data;
	snd_pcm_meter_t *meter = pcm->private_data;
	snd_pcm_t *spcm = meter->gen.slave;
	struct list_head *pos;
	snd_pcm_scope_t *scope;
	int reset;
	list_for_each(pos, &meter->scopes) {
		scope = list_entry(pos, snd_pcm_scope_t, list);
		snd_pcm_scope_enable(scope);
	}
	while (!meter->closed) {
		snd_pcm_sframes_t now;
		snd_pcm_status_t status;
		int err;
		pthread_mutex_lock(&meter->running_mutex);
		err = snd_pcm_status(spcm, &status);
		assert(err >= 0);
		if (status.state != SND_PCM_STATE_RUNNING &&
		    (status.state != SND_PCM_STATE_DRAINING ||
		     spcm->stream != SND_PCM_STREAM_PLAYBACK)) {
			if (meter->running) {
				list_for_each(pos, &meter->scopes) {
					scope = list_entry(pos, snd_pcm_scope_t, list);
					scope->ops->stop(scope);
				}
				meter->running = 0;
			}
			pthread_cond_wait(&meter->running_cond,
					  &meter->running_mutex);
			pthread_mutex_unlock(&meter->running_mutex);
			continue;
		}
		pthread_mutex_unlock(&meter->running_mutex);
		if (pcm->stream == SND_PCM_STREAM_PLAYBACK) {
			now = status.appl_ptr - status.delay;
			if (now < 0)
				now += pcm->boundary;
		} else {
			now = status.appl_ptr + status.delay;
			if ((snd_pcm_uframes_t) now >= pcm->boundary)
				now -= pcm->boundary;
		}
		meter->now = now;
		if (pcm->stream == SND_PCM_STREAM_CAPTURE)
			reset = snd_pcm_meter_update_scope(pcm);
		else {
			reset = 0;
			while (atomic_read(&meter->reset)) {
				reset = 1;
				atomic_dec(&meter->reset);
			}
		}
		if (reset) {
			list_for_each(pos, &meter->scopes) {
				scope = list_entry(pos, snd_pcm_scope_t, list);
				if (scope->enabled)
					scope->ops->reset(scope);
			}
			continue;
		}
		if (!meter->running) {
			list_for_each(pos, &meter->scopes) {
				scope = list_entry(pos, snd_pcm_scope_t, list);
				if (scope->enabled)
					scope->ops->start(scope);
			}
			meter->running = 1;
		}
		list_for_each(pos, &meter->scopes) {
			scope = list_entry(pos, snd_pcm_scope_t, list);
			if (scope->enabled)
				scope->ops->update(scope);
		}
	        nanosleep(&meter->delay, NULL);
	}
	list_for_each(pos, &meter->scopes) {
		scope = list_entry(pos, snd_pcm_scope_t, list);
		if (scope->enabled)
			snd_pcm_scope_disable(scope);
	}
	return NULL;
}

static int snd_pcm_meter_close(snd_pcm_t *pcm)
{
	snd_pcm_meter_t *meter = pcm->private_data;
	struct list_head *pos, *npos;
	int err = 0;
	pthread_mutex_destroy(&meter->update_mutex);
	pthread_mutex_destroy(&meter->running_mutex);
	pthread_cond_destroy(&meter->running_cond);
	if (meter->gen.close_slave)
		err = snd_pcm_close(meter->gen.slave);
	list_for_each_safe(pos, npos, &meter->scopes) {
		snd_pcm_scope_t *scope;
		scope = list_entry(pos, snd_pcm_scope_t, list);
		snd_pcm_scope_remove(scope);
	}
	if (meter->dl_handle)
		snd_dlclose(meter->dl_handle);
	free(meter);
	return err;
}

static int snd_pcm_meter_prepare(snd_pcm_t *pcm)
{
	snd_pcm_meter_t *meter = pcm->private_data;
	int err;
	atomic_inc(&meter->reset);
	err = snd_pcm_prepare(meter->gen.slave);
	if (err >= 0) {
		if (pcm->stream == SND_PCM_STREAM_PLAYBACK)
			meter->rptr = *pcm->appl.ptr;
		else
			meter->rptr = *pcm->hw.ptr;
	}
	return err;
}

static int snd_pcm_meter_reset(snd_pcm_t *pcm)
{
	snd_pcm_meter_t *meter = pcm->private_data;
	int err = snd_pcm_reset(meter->gen.slave);
	if (err >= 0) {
		if (pcm->stream == SND_PCM_STREAM_PLAYBACK)
			meter->rptr = *pcm->appl.ptr;
	}
	return err;
}

static int snd_pcm_meter_start(snd_pcm_t *pcm)
{
	snd_pcm_meter_t *meter = pcm->private_data;
	int err;
	pthread_mutex_lock(&meter->running_mutex);
	err = snd_pcm_start(meter->gen.slave);
	if (err >= 0)
		pthread_cond_signal(&meter->running_cond);
	pthread_mutex_unlock(&meter->running_mutex);
	return err;
}

static snd_pcm_sframes_t snd_pcm_meter_rewind(snd_pcm_t *pcm, snd_pcm_uframes_t frames)
{
	snd_pcm_meter_t *meter = pcm->private_data;
	snd_pcm_sframes_t err = snd_pcm_rewind(meter->gen.slave, frames);
	if (err > 0 && pcm->stream == SND_PCM_STREAM_PLAYBACK)
		meter->rptr = *pcm->appl.ptr;
	return err;
}

static snd_pcm_sframes_t snd_pcm_meter_forward(snd_pcm_t *pcm, snd_pcm_uframes_t frames)
{
	snd_pcm_meter_t *meter = pcm->private_data;
	snd_pcm_sframes_t err = INTERNAL(snd_pcm_forward)(meter->gen.slave, frames);
	if (err > 0 && pcm->stream == SND_PCM_STREAM_PLAYBACK)
		meter->rptr = *pcm->appl.ptr;
	return err;
}

static snd_pcm_sframes_t snd_pcm_meter_mmap_commit(snd_pcm_t *pcm,
						   snd_pcm_uframes_t offset,
						   snd_pcm_uframes_t size)
{
	snd_pcm_meter_t *meter = pcm->private_data;
	snd_pcm_uframes_t old_rptr = *pcm->appl.ptr;
	snd_pcm_sframes_t result = snd_pcm_mmap_commit(meter->gen.slave, offset, size);
	if (result <= 0)
		return result;
	if (pcm->stream == SND_PCM_STREAM_PLAYBACK) {
		snd_pcm_meter_add_frames(pcm, snd_pcm_mmap_areas(pcm), old_rptr, result);
		meter->rptr = *pcm->appl.ptr;
	}
	return result;
}

static snd_pcm_sframes_t snd_pcm_meter_avail_update(snd_pcm_t *pcm)
{
	snd_pcm_meter_t *meter = pcm->private_data;
	snd_pcm_sframes_t result = snd_pcm_avail_update(meter->gen.slave);
	if (result <= 0)
		return result;
	if (pcm->stream == SND_PCM_STREAM_CAPTURE)
		snd_pcm_meter_update_main(pcm);
	return result;
}

static int snd_pcm_meter_hw_refine_cprepare(snd_pcm_t *pcm ATTRIBUTE_UNUSED, snd_pcm_hw_params_t *params)
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

static int snd_pcm_meter_hw_refine_sprepare(snd_pcm_t *pcm ATTRIBUTE_UNUSED, snd_pcm_hw_params_t *sparams)
{
	snd_pcm_access_mask_t saccess_mask = { SND_PCM_ACCBIT_MMAP };
	_snd_pcm_hw_params_any(sparams);
	_snd_pcm_hw_param_set_mask(sparams, SND_PCM_HW_PARAM_ACCESS,
				   &saccess_mask);
	return 0;
}

static int snd_pcm_meter_hw_refine_schange(snd_pcm_t *pcm ATTRIBUTE_UNUSED, snd_pcm_hw_params_t *params,
					  snd_pcm_hw_params_t *sparams)
{
	int err;
	unsigned int links = ~SND_PCM_HW_PARBIT_ACCESS;
	err = _snd_pcm_hw_params_refine(sparams, links, params);
	if (err < 0)
		return err;
	return 0;
}
	
static int snd_pcm_meter_hw_refine_cchange(snd_pcm_t *pcm ATTRIBUTE_UNUSED, snd_pcm_hw_params_t *params,
					  snd_pcm_hw_params_t *sparams)
{
	int err;
	unsigned int links = ~SND_PCM_HW_PARBIT_ACCESS;
	err = _snd_pcm_hw_params_refine(params, links, sparams);
	if (err < 0)
		return err;
	return 0;
}

static int snd_pcm_meter_hw_refine_slave(snd_pcm_t *pcm, snd_pcm_hw_params_t *params)
{
	snd_pcm_meter_t *meter = pcm->private_data;
	return snd_pcm_hw_refine(meter->gen.slave, params);
}

static int snd_pcm_meter_hw_params_slave(snd_pcm_t *pcm, snd_pcm_hw_params_t *params)
{
	snd_pcm_meter_t *meter = pcm->private_data;
	return _snd_pcm_hw_params(meter->gen.slave, params);
}

static int snd_pcm_meter_hw_refine(snd_pcm_t *pcm, snd_pcm_hw_params_t *params)
{
	return snd_pcm_hw_refine_slave(pcm, params,
				       snd_pcm_meter_hw_refine_cprepare,
				       snd_pcm_meter_hw_refine_cchange,
				       snd_pcm_meter_hw_refine_sprepare,
				       snd_pcm_meter_hw_refine_schange,
				       snd_pcm_meter_hw_refine_slave);
}

static int snd_pcm_meter_hw_params(snd_pcm_t *pcm, snd_pcm_hw_params_t * params)
{
	snd_pcm_meter_t *meter = pcm->private_data;
	unsigned int channel;
	snd_pcm_t *slave = meter->gen.slave;
	size_t buf_size_bytes;
	int err;
	err = snd_pcm_hw_params_slave(pcm, params,
				      snd_pcm_meter_hw_refine_cchange,
				      snd_pcm_meter_hw_refine_sprepare,
				      snd_pcm_meter_hw_refine_schange,
				      snd_pcm_meter_hw_params_slave);
	if (err < 0)
		return err;
	/* more than 1 second of buffer */
	meter->buf_size = slave->buffer_size;
	while (meter->buf_size < slave->rate)
		meter->buf_size *= 2;
	buf_size_bytes = snd_pcm_frames_to_bytes(slave, meter->buf_size);
	assert(!meter->buf);
	meter->buf = malloc(buf_size_bytes);
	if (!meter->buf)
		return -ENOMEM;
	meter->buf_areas = malloc(sizeof(*meter->buf_areas) * slave->channels);
	if (!meter->buf_areas) {
		free(meter->buf);
		return -ENOMEM;
	}
	for (channel = 0; channel < slave->channels; ++channel) {
		snd_pcm_channel_area_t *a = &meter->buf_areas[channel];
		a->addr = meter->buf + buf_size_bytes / slave->channels * channel;
		a->first = 0;
		a->step = slave->sample_bits;
	}
	meter->closed = 0;
	err = pthread_create(&meter->thread, NULL, snd_pcm_meter_thread, pcm);
	assert(err == 0);
	return 0;
}

static int snd_pcm_meter_hw_free(snd_pcm_t *pcm)
{
	snd_pcm_meter_t *meter = pcm->private_data;
	int err;
	meter->closed = 1;
	pthread_mutex_lock(&meter->running_mutex);
	pthread_cond_signal(&meter->running_cond);
	pthread_mutex_unlock(&meter->running_mutex);
	err = pthread_join(meter->thread, 0);
	assert(err == 0);
	free(meter->buf);
	free(meter->buf_areas);
	meter->buf = NULL;
	meter->buf_areas = NULL;
	return snd_pcm_hw_free(meter->gen.slave);
}

static void snd_pcm_meter_dump(snd_pcm_t *pcm, snd_output_t *out)
{
	snd_pcm_meter_t *meter = pcm->private_data;
	snd_output_printf(out, "Meter PCM\n");
	if (pcm->setup) {
		snd_output_printf(out, "Its setup is:\n");
		snd_pcm_dump_setup(pcm, out);
	}
	snd_output_printf(out, "Slave: ");
	snd_pcm_dump(meter->gen.slave, out);
}

static const snd_pcm_ops_t snd_pcm_meter_ops = {
	.close = snd_pcm_meter_close,
	.info = snd_pcm_generic_info,
	.hw_refine = snd_pcm_meter_hw_refine,
	.hw_params = snd_pcm_meter_hw_params,
	.hw_free = snd_pcm_meter_hw_free,
	.sw_params = snd_pcm_generic_sw_params,
	.channel_info = snd_pcm_generic_channel_info,
	.dump = snd_pcm_meter_dump,
	.nonblock = snd_pcm_generic_nonblock,
	.async = snd_pcm_generic_async,
	.mmap = snd_pcm_generic_mmap,
	.munmap = snd_pcm_generic_munmap,
};

static const snd_pcm_fast_ops_t snd_pcm_meter_fast_ops = {
	.status = snd_pcm_generic_status,
	.state = snd_pcm_generic_state,
	.hwsync = snd_pcm_generic_hwsync,
	.delay = snd_pcm_generic_delay,
	.prepare = snd_pcm_meter_prepare,
	.reset = snd_pcm_meter_reset,
	.start = snd_pcm_meter_start,
	.drop = snd_pcm_generic_drop,
	.drain = snd_pcm_generic_drain,
	.pause = snd_pcm_generic_pause,
	.rewindable = snd_pcm_generic_rewindable,
	.rewind = snd_pcm_meter_rewind,
	.forwardable = snd_pcm_generic_forwardable,
	.forward = snd_pcm_meter_forward,
	.resume = snd_pcm_generic_resume,
	.writei = snd_pcm_mmap_writei,
	.writen = snd_pcm_mmap_writen,
	.readi = snd_pcm_mmap_readi,
	.readn = snd_pcm_mmap_readn,
	.avail_update = snd_pcm_meter_avail_update,
	.mmap_commit = snd_pcm_meter_mmap_commit,
	.htimestamp = snd_pcm_generic_htimestamp,
	.poll_descriptors_count = snd_pcm_generic_poll_descriptors_count,
	.poll_descriptors = snd_pcm_generic_poll_descriptors,
	.poll_revents = snd_pcm_generic_poll_revents,
};

/**
 * \brief Creates a new Meter PCM
 * \param pcmp Returns created PCM handle
 * \param name Name of PCM
 * \param frequency Update frequency
 * \param slave Slave PCM handle
 * \param close_slave When set, the slave PCM handle is closed with copy PCM
 * \retval zero on success otherwise a negative error code
 * \warning Using of this function might be dangerous in the sense
 *          of compatibility reasons. The prototype might be freely
 *          changed in future.
 */
int snd_pcm_meter_open(snd_pcm_t **pcmp, const char *name, unsigned int frequency,
		       snd_pcm_t *slave, int close_slave)
{
	snd_pcm_t *pcm;
	snd_pcm_meter_t *meter;
	int err;
	assert(pcmp);
	meter = calloc(1, sizeof(snd_pcm_meter_t));
	if (!meter)
		return -ENOMEM;
	meter->gen.slave = slave;
	meter->gen.close_slave = close_slave;
	meter->delay.tv_sec = 0;
	meter->delay.tv_nsec = 1000000000 / frequency;
	INIT_LIST_HEAD(&meter->scopes);

	err = snd_pcm_new(&pcm, SND_PCM_TYPE_METER, name, slave->stream, slave->mode);
	if (err < 0) {
		free(meter);
		return err;
	}
	pcm->mmap_rw = 1;
	pcm->mmap_shadow = 1;
	pcm->ops = &snd_pcm_meter_ops;
	pcm->fast_ops = &snd_pcm_meter_fast_ops;
	pcm->private_data = meter;
	pcm->poll_fd = slave->poll_fd;
	pcm->poll_events = slave->poll_events;
	pcm->monotonic = slave->monotonic;
	snd_pcm_link_hw_ptr(pcm, slave);
	snd_pcm_link_appl_ptr(pcm, slave);
	*pcmp = pcm;

	pthread_mutex_init(&meter->update_mutex, NULL);
	pthread_mutex_init(&meter->running_mutex, NULL);
	pthread_cond_init(&meter->running_cond, NULL);
	return 0;
}


static int snd_pcm_meter_add_scope_conf(snd_pcm_t *pcm, const char *name,
					snd_config_t *root, snd_config_t *conf)
{
	char buf[256];
	snd_config_iterator_t i, next;
	const char *id;
	const char *lib = NULL, *open_name = NULL, *str = NULL;
	snd_config_t *c, *type_conf = NULL;
	int (*open_func)(snd_pcm_t *, const char *,
			 snd_config_t *, snd_config_t *) = NULL;
	snd_pcm_meter_t *meter = pcm->private_data;
	void *h = NULL;
	int err;

	if (snd_config_get_type(conf) != SND_CONFIG_TYPE_COMPOUND) {
		SNDERR("Invalid type for scope %s", str);
		err = -EINVAL;
		goto _err;
	}
	err = snd_config_search(conf, "type", &c);
	if (err < 0) {
		SNDERR("type is not defined");
		goto _err;
	}
	err = snd_config_get_id(c, &id);
	if (err < 0) {
		SNDERR("unable to get id");
		goto _err;
	}
	err = snd_config_get_string(c, &str);
	if (err < 0) {
		SNDERR("Invalid type for %s", id);
		goto _err;
	}
	err = snd_config_search_definition(root, "pcm_scope_type", str, &type_conf);
	if (err >= 0) {
		snd_config_for_each(i, next, type_conf) {
			snd_config_t *n = snd_config_iterator_entry(i);
			const char *id;
			if (snd_config_get_id(n, &id) < 0)
				continue;
			if (strcmp(id, "comment") == 0)
				continue;
			if (strcmp(id, "lib") == 0) {
				err = snd_config_get_string(n, &lib);
				if (err < 0) {
					SNDERR("Invalid type for %s", id);
					goto _err;
				}
				continue;
			}
			if (strcmp(id, "open") == 0) {
				err = snd_config_get_string(n, &open_name);
				if (err < 0) {
					SNDERR("Invalid type for %s", id);
					goto _err;
				}
				continue;
			}
			SNDERR("Unknown field %s", id);
			err = -EINVAL;
			goto _err;
		}
	}
	if (!open_name) {
		open_name = buf;
		snprintf(buf, sizeof(buf), "_snd_pcm_scope_%s_open", str);
	}
	h = snd_dlopen(lib, RTLD_NOW);
	open_func = h ? dlsym(h, open_name) : NULL;
	err = 0;
	if (!h) {
		SNDERR("Cannot open shared library %s", lib);
		err = -ENOENT;
	} else if (!open_func) {
		SNDERR("symbol %s is not defined inside %s", open_name, lib);
		snd_dlclose(h);
		err = -ENXIO;
	}
       _err:
	if (type_conf)
		snd_config_delete(type_conf);
	if (! err) {
		err = open_func(pcm, name, root, conf);
		if (err < 0)
			snd_dlclose(h);
		else
			meter->dl_handle = h;
	}
	return err;
}

/*! \page pcm_plugins

\section pcm_plugins_meter Plugin: Meter

Show meter (visual waveform representation).

\code
pcm_scope_type.NAME {
	[lib STR]		# Library file (default libasound.so)
	[open STR]		# Open function (default _snd_pcm_scope_NAME_open)
}

pcm_scope.name {
	type STR		# Scope type
	...
}

pcm.name {
        type meter              # Meter PCM
        slave STR               # Slave name
        # or
        slave {                 # Slave definition
                pcm STR         # Slave PCM name
                # or
                pcm { }         # Slave PCM definition
        }
	[frequency INT]		# Updates per second
	scopes {
		ID STR		# Scope name (see pcm_scope)
		# or
		ID { }		# Scope definition (see pcm_scope)
	}
}
\endcode

\subsection pcm_plugins_meter_funcref Function reference

<UL>
  <LI>snd_pcm_meter_open()
  <LI>_snd_pcm_meter_open()
</UL>

*/

/**
 * \brief Creates a new Meter PCM
 * \param pcmp Returns created PCM handle
 * \param name Name of PCM
 * \param root Root configuration node
 * \param conf Configuration node with Meter PCM description
 * \param stream Stream type
 * \param mode Stream mode
 * \retval zero on success otherwise a negative error code
 * \warning Using of this function might be dangerous in the sense
 *          of compatibility reasons. The prototype might be freely
 *          changed in future.
 */
int _snd_pcm_meter_open(snd_pcm_t **pcmp, const char *name,
			snd_config_t *root, snd_config_t *conf, 
			snd_pcm_stream_t stream, int mode)
{
	snd_config_iterator_t i, next;
	int err;
	snd_pcm_t *spcm;
	snd_config_t *slave = NULL, *sconf;
	long frequency = -1;
	snd_config_t *scopes = NULL;
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
		if (strcmp(id, "frequency") == 0) {
			err = snd_config_get_integer(n, &frequency);
			if (err < 0) {
				SNDERR("Invalid type for %s", id);
				return -EINVAL;
			}
			continue;
		}
		if (strcmp(id, "scopes") == 0) {
			if (snd_config_get_type(n) != SND_CONFIG_TYPE_COMPOUND) {
				SNDERR("Invalid type for %s", id);
				return -EINVAL;
			}
			scopes = n;
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
	err = snd_pcm_meter_open(pcmp, name, frequency > 0 ? (unsigned int) frequency : FREQUENCY, spcm, 1);
	if (err < 0) {
		snd_pcm_close(spcm);
		return err;
	}
	if (!scopes)
		return 0;
	snd_config_for_each(i, next, scopes) {
		snd_config_t *n = snd_config_iterator_entry(i);
		const char *id, *str;
		if (snd_config_get_id(n, &id) < 0)
			continue;
		if (snd_config_get_string(n, &str) >= 0) {
			err = snd_config_search_definition(root, "pcm_scope", str, &n);
			if (err < 0) {
				SNDERR("unknown pcm_scope %s", str);
			} else {
				err = snd_pcm_meter_add_scope_conf(*pcmp, id, root, n);
				snd_config_delete(n);
			}
		} else
			err = snd_pcm_meter_add_scope_conf(*pcmp, id, root, n);
		if (err < 0) {
			snd_pcm_close(*pcmp);
			return err;
		}
	}
	return 0;
}
SND_DLSYM_BUILD_VERSION(_snd_pcm_meter_open, SND_PCM_DLSYM_VERSION);

#endif

/**
 * \brief Add a scope to a #SND_PCM_TYPE_METER PCM
 * \param pcm PCM handle
 * \param scope Scope handle
 * \return 0 on success otherwise a negative error code
 */
int snd_pcm_meter_add_scope(snd_pcm_t *pcm, snd_pcm_scope_t *scope)
{
	snd_pcm_meter_t *meter;
	assert(pcm->type == SND_PCM_TYPE_METER);
	meter = pcm->private_data;
	list_add_tail(&scope->list, &meter->scopes);
	return 0;
}

/**
 * \brief Search an installed scope inside a #SND_PCM_TYPE_METER PCM
 * \param pcm PCM handle
 * \param name scope name
 * \return pointer to found scope or NULL if none is found
 */
snd_pcm_scope_t *snd_pcm_meter_search_scope(snd_pcm_t *pcm, const char *name)
{
	snd_pcm_meter_t *meter;
	struct list_head *pos;
	assert(pcm->type == SND_PCM_TYPE_METER);
	meter = pcm->private_data;
	list_for_each(pos, &meter->scopes) {
		snd_pcm_scope_t *scope;
		scope = list_entry(pos, snd_pcm_scope_t, list);
		if (scope->name && strcmp(scope->name, name) == 0)
			return scope;
	}
	return NULL;
}

/**
 * \brief Get meter buffer size from a #SND_PCM_TYPE_METER PCM
 * \param pcm PCM handle
 * \return meter buffer size in frames
 */
snd_pcm_uframes_t snd_pcm_meter_get_bufsize(snd_pcm_t *pcm)
{
	snd_pcm_meter_t *meter;
	assert(pcm->type == SND_PCM_TYPE_METER);
	meter = pcm->private_data;
	assert(meter->gen.slave->setup);
	return meter->buf_size;
}

/**
 * \brief Get meter channels from a #SND_PCM_TYPE_METER PCM
 * \param pcm PCM handle
 * \return meter channels count
 */
unsigned int snd_pcm_meter_get_channels(snd_pcm_t *pcm)
{
	snd_pcm_meter_t *meter;
	assert(pcm->type == SND_PCM_TYPE_METER);
	meter = pcm->private_data;
	assert(meter->gen.slave->setup);
	return meter->gen.slave->channels;
}

/**
 * \brief Get meter rate from a #SND_PCM_TYPE_METER PCM
 * \param pcm PCM handle
 * \return approximate rate
 */
unsigned int snd_pcm_meter_get_rate(snd_pcm_t *pcm)
{
	snd_pcm_meter_t *meter;
	assert(pcm->type == SND_PCM_TYPE_METER);
	meter = pcm->private_data;
	assert(meter->gen.slave->setup);
	return meter->gen.slave->rate;
}

/**
 * \brief Get meter "now" frame pointer from a #SND_PCM_TYPE_METER PCM
 * \param pcm PCM handle
 * \return "now" frame pointer in frames (0 ... boundary - 1) see #snd_pcm_meter_get_boundary
 */
snd_pcm_uframes_t snd_pcm_meter_get_now(snd_pcm_t *pcm)
{
	snd_pcm_meter_t *meter;
	assert(pcm->type == SND_PCM_TYPE_METER);
	meter = pcm->private_data;
	assert(meter->gen.slave->setup);
	return meter->now;
}

/**
 * \brief Get boundary for frame pointers from a #SND_PCM_TYPE_METER PCM
 * \param pcm PCM handle
 * \return boundary in frames
 */
snd_pcm_uframes_t snd_pcm_meter_get_boundary(snd_pcm_t *pcm)
{
	snd_pcm_meter_t *meter;
	assert(pcm->type == SND_PCM_TYPE_METER);
	meter = pcm->private_data;
	assert(meter->gen.slave->setup);
	return meter->gen.slave->boundary;
}

/**
 * \brief Set name of a #SND_PCM_TYPE_METER PCM scope
 * \param scope PCM meter scope
 * \param val scope name
 */
void snd_pcm_scope_set_name(snd_pcm_scope_t *scope, const char *val)
{
	scope->name = strdup(val);
}

/**
 * \brief Get name of a #SND_PCM_TYPE_METER PCM scope
 * \param scope PCM meter scope
 * \return scope name
 */
const char *snd_pcm_scope_get_name(snd_pcm_scope_t *scope)
{
	return scope->name;
}

/**
 * \brief Set callbacks for a #SND_PCM_TYPE_METER PCM scope
 * \param scope PCM meter scope
 * \param val callbacks
 */
void snd_pcm_scope_set_ops(snd_pcm_scope_t *scope, const snd_pcm_scope_ops_t *val)
{
	scope->ops = val;
}

/**
 * \brief Get callbacks private value for a #SND_PCM_TYPE_METER PCM scope
 * \param scope PCM meter scope
 * \return Private data value
 */
void *snd_pcm_scope_get_callback_private(snd_pcm_scope_t *scope)
{
	return scope->private_data;
}

/**
 * \brief Get callbacks private value for a #SND_PCM_TYPE_METER PCM scope
 * \param scope PCM meter scope
 * \param val Private data value
 */
void snd_pcm_scope_set_callback_private(snd_pcm_scope_t *scope, void *val)
{
	scope->private_data = val;
}

#ifndef DOC_HIDDEN
typedef struct _snd_pcm_scope_s16 {
	snd_pcm_t *pcm;
	snd_pcm_adpcm_state_t *adpcm_states;
	unsigned int index;
	snd_pcm_uframes_t old;
	int16_t *buf;
	snd_pcm_channel_area_t *buf_areas;
} snd_pcm_scope_s16_t;

static int s16_enable(snd_pcm_scope_t *scope)
{
	snd_pcm_scope_s16_t *s16 = scope->private_data;
	snd_pcm_meter_t *meter = s16->pcm->private_data;
	snd_pcm_t *spcm = meter->gen.slave;
	snd_pcm_channel_area_t *a;
	unsigned int c;
	int idx;
	if (spcm->format == SND_PCM_FORMAT_S16 &&
	    spcm->access == SND_PCM_ACCESS_MMAP_NONINTERLEAVED) {
		s16->buf = (int16_t *) meter->buf;
		return -EINVAL;
	}
	switch (spcm->format) {
	case SND_PCM_FORMAT_A_LAW:
	case SND_PCM_FORMAT_MU_LAW:
	case SND_PCM_FORMAT_IMA_ADPCM:
		idx = snd_pcm_linear_put_index(SND_PCM_FORMAT_S16, SND_PCM_FORMAT_S16);
		break;
	case SND_PCM_FORMAT_S8:
	case SND_PCM_FORMAT_S16_LE:
	case SND_PCM_FORMAT_S16_BE:
	case SND_PCM_FORMAT_S24_LE:
	case SND_PCM_FORMAT_S24_BE:
	case SND_PCM_FORMAT_S32_LE:
	case SND_PCM_FORMAT_S32_BE:
	case SND_PCM_FORMAT_U8:
	case SND_PCM_FORMAT_U16_LE:
	case SND_PCM_FORMAT_U16_BE:
	case SND_PCM_FORMAT_U24_LE:
	case SND_PCM_FORMAT_U24_BE:
	case SND_PCM_FORMAT_U32_LE:
	case SND_PCM_FORMAT_U32_BE:
		idx = snd_pcm_linear_convert_index(spcm->format, SND_PCM_FORMAT_S16);
		break;
	default:
		return -EINVAL;
	}
	s16->index = idx;
	if (spcm->format == SND_PCM_FORMAT_IMA_ADPCM) {
		s16->adpcm_states = calloc(spcm->channels, sizeof(*s16->adpcm_states));
		if (!s16->adpcm_states)
			return -ENOMEM;
	}
	s16->buf = malloc(meter->buf_size * 2 * spcm->channels);
	if (!s16->buf) {
		free(s16->adpcm_states);
		return -ENOMEM;
	}
	a = calloc(spcm->channels, sizeof(*a));
	if (!a) {
		free(s16->buf);
		free(s16->adpcm_states);
		return -ENOMEM;
	}
	s16->buf_areas = a;
	for (c = 0; c < spcm->channels; c++, a++) {
		a->addr = s16->buf + c * meter->buf_size;
		a->first = 0;
		a->step = 16;
	}
	return 0;
}

static void s16_disable(snd_pcm_scope_t *scope)
{
	snd_pcm_scope_s16_t *s16 = scope->private_data;
	free(s16->adpcm_states);
	s16->adpcm_states = NULL;
	free(s16->buf);
	s16->buf = NULL;
	free(s16->buf_areas);
	s16->buf_areas = 0;
}

static void s16_close(snd_pcm_scope_t *scope)
{
	snd_pcm_scope_s16_t *s16 = scope->private_data;
	free(s16);
}

static void s16_start(snd_pcm_scope_t *scope ATTRIBUTE_UNUSED)
{
}

static void s16_stop(snd_pcm_scope_t *scope ATTRIBUTE_UNUSED)
{
}

static void s16_update(snd_pcm_scope_t *scope)
{
	snd_pcm_scope_s16_t *s16 = scope->private_data;
	snd_pcm_meter_t *meter = s16->pcm->private_data;
	snd_pcm_t *spcm = meter->gen.slave;
	snd_pcm_sframes_t size;
	snd_pcm_uframes_t offset;
	size = meter->now - s16->old;
	if (size < 0)
		size += spcm->boundary;
	offset = s16->old % meter->buf_size;
	while (size > 0) {
		snd_pcm_uframes_t frames = size;
		snd_pcm_uframes_t cont = meter->buf_size - offset;
		if (frames > cont)
			frames = cont;
		switch (spcm->format) {
		case SND_PCM_FORMAT_A_LAW:
			snd_pcm_alaw_decode(s16->buf_areas, offset,
					    meter->buf_areas, offset,
					    spcm->channels, frames,
					    s16->index);
			break;
		case SND_PCM_FORMAT_MU_LAW:
			snd_pcm_mulaw_decode(s16->buf_areas, offset,
					     meter->buf_areas, offset,
					     spcm->channels, frames,
					     s16->index);
			break;
		case SND_PCM_FORMAT_IMA_ADPCM:
			snd_pcm_adpcm_decode(s16->buf_areas, offset,
					     meter->buf_areas, offset,
					     spcm->channels, frames,
					     s16->index,
					     s16->adpcm_states);
			break;
		default:
			snd_pcm_linear_convert(s16->buf_areas, offset,
					       meter->buf_areas, offset,
					       spcm->channels, frames,
					       s16->index);
			break;
		}
		if (frames == cont)
			offset = 0;
		else
			offset += frames;
		size -= frames;
	}
	s16->old = meter->now;
}

static void s16_reset(snd_pcm_scope_t *scope)
{
	snd_pcm_scope_s16_t *s16 = scope->private_data;
	snd_pcm_meter_t *meter = s16->pcm->private_data;
	s16->old = meter->now;
}

static const snd_pcm_scope_ops_t s16_ops = {
	.enable = s16_enable,
	.disable = s16_disable,
	.close = s16_close,
	.start = s16_start,
	.stop = s16_stop,
	.update = s16_update,
	.reset = s16_reset,
};

#endif

/**
 * \brief Add a s16 pseudo scope to a #SND_PCM_TYPE_METER PCM
 * \param pcm The pcm handle
 * \param name Scope name
 * \param scopep Pointer to newly created and added scope
 * \return 0 on success otherwise a negative error code
 *
 * s16 pseudo scope convert #SND_PCM_TYPE_METER PCM frames in CPU endian 
 * 16 bit frames for use with other scopes. Don't forget to insert it before
 * and to not insert it more time (see #snd_pcm_meter_search_scope)
 */
int snd_pcm_scope_s16_open(snd_pcm_t *pcm, const char *name,
			   snd_pcm_scope_t **scopep)
{
	snd_pcm_meter_t *meter;
	snd_pcm_scope_t *scope;
	snd_pcm_scope_s16_t *s16;
	assert(pcm->type == SND_PCM_TYPE_METER);
	meter = pcm->private_data;
	scope = calloc(1, sizeof(*scope));
	if (!scope)
		return -ENOMEM;
	s16 = calloc(1, sizeof(*s16));
	if (!s16) {
		free(scope);
		return -ENOMEM;
	}
	if (name)
		scope->name = strdup(name);
	s16->pcm = pcm;
	scope->ops = &s16_ops;
	scope->private_data = s16;
	list_add_tail(&scope->list, &meter->scopes);
	*scopep = scope;
	return 0;
}

/**
 * \brief Get s16 pseudo scope frames buffer for a channel
 * \param scope s16 pseudo scope handle
 * \param channel Channel
 * \return Pointer to channel buffer
 */
int16_t *snd_pcm_scope_s16_get_channel_buffer(snd_pcm_scope_t *scope,
					      unsigned int channel)
{
	snd_pcm_scope_s16_t *s16;
	snd_pcm_meter_t *meter;
	assert(scope->ops == &s16_ops);
	s16 = scope->private_data;
	meter = s16->pcm->private_data;
	assert(meter->gen.slave->setup);
	assert(s16->buf_areas);
	assert(channel < meter->gen.slave->channels);
	return s16->buf_areas[channel].addr;
}

/**
 * \brief allocate an invalid #snd_pcm_scope_t using standard malloc
 * \param ptr returned pointer
 * \return 0 on success otherwise negative error code
 */
int snd_pcm_scope_malloc(snd_pcm_scope_t **ptr)
{
	assert(ptr);
	*ptr = calloc(1, sizeof(snd_pcm_scope_t));
	if (!*ptr)
		return -ENOMEM;
	return 0;
}
