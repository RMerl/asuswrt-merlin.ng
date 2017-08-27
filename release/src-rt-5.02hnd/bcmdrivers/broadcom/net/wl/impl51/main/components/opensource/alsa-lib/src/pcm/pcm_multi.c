/**
 * \file pcm/pcm_multi.c
 * \ingroup PCM_Plugins
 * \brief PCM Multi Streams to One Conversion Plugin Interface
 * \author Abramo Bagnara <abramo@alsa-project.org>
 * \date 2000-2001
 */
/*
 *  PCM - Multi
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
  
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include "pcm_local.h"
#include "pcm_generic.h"

#ifndef PIC
/* entry for static linking */
const char *_snd_module_pcm_multi = "";
#endif

#ifndef DOC_HIDDEN

typedef struct {
	snd_pcm_t *pcm;
	unsigned int channels_count;
	int close_slave;
	snd_pcm_t *linked;
} snd_pcm_multi_slave_t;

typedef struct {
	int slave_idx;
	unsigned int slave_channel;
} snd_pcm_multi_channel_t;

typedef struct {
	unsigned int slaves_count;
	unsigned int master_slave;
	snd_pcm_multi_slave_t *slaves;
	unsigned int channels_count;
	snd_pcm_multi_channel_t *channels;
} snd_pcm_multi_t;

#endif

static int snd_pcm_multi_close(snd_pcm_t *pcm)
{
	snd_pcm_multi_t *multi = pcm->private_data;
	unsigned int i;
	int ret = 0;
	for (i = 0; i < multi->slaves_count; ++i) {
		snd_pcm_multi_slave_t *slave = &multi->slaves[i];
		if (slave->close_slave) {
			int err = snd_pcm_close(slave->pcm);
			if (err < 0)
				ret = err;
		}
	}
	free(multi->slaves);
	free(multi->channels);
	free(multi);
	return ret;
}

static int snd_pcm_multi_nonblock(snd_pcm_t *pcm ATTRIBUTE_UNUSED, int nonblock ATTRIBUTE_UNUSED)
{
	return 0;
}

static int snd_pcm_multi_async(snd_pcm_t *pcm, int sig, pid_t pid)
{
	snd_pcm_multi_t *multi = pcm->private_data;
	snd_pcm_t *slave_0 = multi->slaves[multi->master_slave].pcm;
	return snd_pcm_async(slave_0, sig, pid);
}

static int snd_pcm_multi_poll_descriptors_count(snd_pcm_t *pcm)
{
	snd_pcm_multi_t *multi = pcm->private_data;
	snd_pcm_t *slave_0 = multi->slaves[multi->master_slave].pcm;
	return snd_pcm_poll_descriptors_count(slave_0);
}

static int snd_pcm_multi_poll_descriptors(snd_pcm_t *pcm, struct pollfd *pfds, unsigned int space)
{
	snd_pcm_multi_t *multi = pcm->private_data;
	snd_pcm_t *slave;
	snd_pcm_t *slave_0 = multi->slaves[multi->master_slave].pcm;
	int err;
	unsigned int i;

	for (i = 0; i < multi->slaves_count; ++i) {
		slave = multi->slaves[i].pcm;
		if (slave == slave_0)
			continue;
		err = snd_pcm_poll_descriptors(slave, pfds, space);
		if (err < 0)
			return err;
	}
	/* finally overwrite with master's pfds */
	return snd_pcm_poll_descriptors(slave_0, pfds, space);
}

static int snd_pcm_multi_poll_revents(snd_pcm_t *pcm, struct pollfd *pfds, unsigned int nfds, unsigned short *revents)
{
	snd_pcm_multi_t *multi = pcm->private_data;
	snd_pcm_t *slave_0 = multi->slaves[multi->master_slave].pcm;
	return snd_pcm_poll_descriptors_revents(slave_0, pfds, nfds, revents);
}

static int snd_pcm_multi_info(snd_pcm_t *pcm, snd_pcm_info_t *info)
{
	snd_pcm_multi_t *multi = pcm->private_data;
	int err, n;
	assert(info->subdevice < multi->slaves_count);
	n = info->subdevice;
	info->subdevice = 0;
	err = snd_pcm_info(multi->slaves[n].pcm, info);
	if (err < 0)
		return err;
	info->subdevices_count = multi->slaves_count;
	return 0;
}

static int snd_pcm_multi_hw_refine_cprepare(snd_pcm_t *pcm, snd_pcm_hw_params_t *params)
{
	snd_pcm_multi_t *multi = pcm->private_data;
	snd_pcm_access_mask_t access_mask;
	int err;
	snd_pcm_access_mask_any(&access_mask);
	snd_pcm_access_mask_reset(&access_mask, SND_PCM_ACCESS_MMAP_INTERLEAVED);
	err = _snd_pcm_hw_param_set_mask(params, SND_PCM_HW_PARAM_ACCESS,
					 &access_mask);
	if (err < 0)
		return err;
	err = _snd_pcm_hw_param_set(params, SND_PCM_HW_PARAM_CHANNELS,
				    multi->channels_count, 0);
	if (err < 0)
		return err;
	params->info = ~0U;
	return 0;
}

static int snd_pcm_multi_hw_refine_sprepare(snd_pcm_t *pcm, unsigned int slave_idx,
					    snd_pcm_hw_params_t *sparams)
{
	snd_pcm_multi_t *multi = pcm->private_data;
	snd_pcm_multi_slave_t *slave = &multi->slaves[slave_idx];
	snd_pcm_access_mask_t saccess_mask = { SND_PCM_ACCBIT_MMAP };
	_snd_pcm_hw_params_any(sparams);
	_snd_pcm_hw_param_set_mask(sparams, SND_PCM_HW_PARAM_ACCESS,
				   &saccess_mask);
	_snd_pcm_hw_param_set(sparams, SND_PCM_HW_PARAM_CHANNELS,
			      slave->channels_count, 0);
	return 0;
}

static int snd_pcm_multi_hw_refine_schange(snd_pcm_t *pcm ATTRIBUTE_UNUSED,
					   unsigned int slave_idx ATTRIBUTE_UNUSED,
					   snd_pcm_hw_params_t *params,
					   snd_pcm_hw_params_t *sparams)
{
	int err;
	unsigned int links = (SND_PCM_HW_PARBIT_FORMAT |
			      SND_PCM_HW_PARBIT_SUBFORMAT |
			      SND_PCM_HW_PARBIT_RATE |
			      SND_PCM_HW_PARBIT_PERIOD_SIZE |
			      SND_PCM_HW_PARBIT_PERIOD_TIME |
			      SND_PCM_HW_PARBIT_PERIODS |
			      SND_PCM_HW_PARBIT_BUFFER_SIZE |
			      SND_PCM_HW_PARBIT_BUFFER_TIME |
			      SND_PCM_HW_PARBIT_TICK_TIME);
	const snd_pcm_access_mask_t *access_mask = snd_pcm_hw_param_get_mask(params, SND_PCM_HW_PARAM_ACCESS);
	if (!snd_pcm_access_mask_test(access_mask, SND_PCM_ACCESS_RW_INTERLEAVED) &&
	    !snd_pcm_access_mask_test(access_mask, SND_PCM_ACCESS_RW_NONINTERLEAVED) &&
	    !snd_pcm_access_mask_test(access_mask, SND_PCM_ACCESS_MMAP_NONINTERLEAVED)) {
		snd_pcm_access_mask_t saccess_mask;
		snd_pcm_access_mask_any(&saccess_mask);
		snd_pcm_access_mask_reset(&saccess_mask, SND_PCM_ACCESS_MMAP_NONINTERLEAVED);
		err = _snd_pcm_hw_param_set_mask(sparams, SND_PCM_HW_PARAM_ACCESS,
						 &saccess_mask);
		if (err < 0)
			return err;
	}
	err = _snd_pcm_hw_params_refine(sparams, links, params);
	if (err < 0)
		return err;
	return 0;
}
	
static int snd_pcm_multi_hw_refine_cchange(snd_pcm_t *pcm ATTRIBUTE_UNUSED,
					   unsigned int slave_idx ATTRIBUTE_UNUSED,
					   snd_pcm_hw_params_t *params,
					   snd_pcm_hw_params_t *sparams)
{
	int err;
	unsigned int links = (SND_PCM_HW_PARBIT_FORMAT |
			      SND_PCM_HW_PARBIT_SUBFORMAT |
			      SND_PCM_HW_PARBIT_RATE |
			      SND_PCM_HW_PARBIT_PERIOD_SIZE |
			      SND_PCM_HW_PARBIT_PERIOD_TIME |
			      SND_PCM_HW_PARBIT_PERIODS |
			      SND_PCM_HW_PARBIT_BUFFER_SIZE |
			      SND_PCM_HW_PARBIT_BUFFER_TIME |
			      SND_PCM_HW_PARBIT_TICK_TIME);
	snd_pcm_access_mask_t access_mask;
	const snd_pcm_access_mask_t *saccess_mask = snd_pcm_hw_param_get_mask(sparams, SND_PCM_HW_PARAM_ACCESS);
	snd_pcm_access_mask_any(&access_mask);
	snd_pcm_access_mask_reset(&access_mask, SND_PCM_ACCESS_MMAP_INTERLEAVED);
	if (!snd_pcm_access_mask_test(saccess_mask, SND_PCM_ACCESS_MMAP_NONINTERLEAVED))
		snd_pcm_access_mask_reset(&access_mask, SND_PCM_ACCESS_MMAP_NONINTERLEAVED);
	if (!snd_pcm_access_mask_test(saccess_mask, SND_PCM_ACCESS_MMAP_COMPLEX) &&
	    !snd_pcm_access_mask_test(saccess_mask, SND_PCM_ACCESS_MMAP_INTERLEAVED))
		snd_pcm_access_mask_reset(&access_mask, SND_PCM_ACCESS_MMAP_COMPLEX);
	err = _snd_pcm_hw_param_set_mask(params, SND_PCM_HW_PARAM_ACCESS,
					 &access_mask);
	if (err < 0)
		return err;
	err = _snd_pcm_hw_params_refine(params, links, sparams);
	if (err < 0)
		return err;
	params->info &= sparams->info;
	return 0;
}

static int snd_pcm_multi_hw_refine_slave(snd_pcm_t *pcm,
					 unsigned int slave_idx,
					 snd_pcm_hw_params_t *sparams)
{
	snd_pcm_multi_t *multi = pcm->private_data;
	snd_pcm_t *slave = multi->slaves[slave_idx].pcm;
	return snd_pcm_hw_refine(slave, sparams);
}

static int snd_pcm_multi_hw_refine(snd_pcm_t *pcm, snd_pcm_hw_params_t *params)
{
	snd_pcm_multi_t *multi = pcm->private_data;
	unsigned int k;
	snd_pcm_hw_params_t sparams[multi->slaves_count];
	int err;
	unsigned int cmask, changed;
	err = snd_pcm_multi_hw_refine_cprepare(pcm, params);
	if (err < 0)
		return err;
	for (k = 0; k < multi->slaves_count; ++k) {
		err = snd_pcm_multi_hw_refine_sprepare(pcm, k, &sparams[k]);
		if (err < 0) {
			SNDERR("Slave PCM #%d not usable", k);
			return err;
		}
	}
	do {
		cmask = params->cmask;
		params->cmask = 0;
		for (k = 0; k < multi->slaves_count; ++k) {
			err = snd_pcm_multi_hw_refine_schange(pcm, k, params, &sparams[k]);
			if (err >= 0)
				err = snd_pcm_multi_hw_refine_slave(pcm, k, &sparams[k]);
			if (err < 0) {
				snd_pcm_multi_hw_refine_cchange(pcm, k, params, &sparams[k]);
				return err;
			}
			err = snd_pcm_multi_hw_refine_cchange(pcm, k, params, &sparams[k]);
			if (err < 0)
				return err;
		}
		err = snd_pcm_hw_refine_soft(pcm, params);
		changed = params->cmask;
		params->cmask |= cmask;
		if (err < 0)
			return err;
	} while (changed);
	return 0;
}

static int snd_pcm_multi_hw_params_slave(snd_pcm_t *pcm,
					 unsigned int slave_idx,
					 snd_pcm_hw_params_t *sparams)
{
	snd_pcm_multi_t *multi = pcm->private_data;
	snd_pcm_t *slave = multi->slaves[slave_idx].pcm;
	int err = snd_pcm_hw_params(slave, sparams);
	if (err < 0)
		return err;
	err = snd_pcm_areas_silence(slave->running_areas, 0, slave->channels, slave->buffer_size, slave->format);
	if (err < 0)
		return err;
	if (slave->stopped_areas) {
		err = snd_pcm_areas_silence(slave->stopped_areas, 0, slave->channels, slave->buffer_size, slave->format);
		if (err < 0)
			return err;
	}
	return 0;
}

/* reset links to the normal state
 * slave #0 = trigger master
 * slave #1-(N-1) = trigger slaves, linked is set to #0
 */
static void reset_links(snd_pcm_multi_t *multi)
{
	unsigned int i;

	for (i = 0; i < multi->slaves_count; ++i) {
		if (multi->slaves[i].linked)
			snd_pcm_unlink(multi->slaves[i].linked);
		multi->slaves[0].linked = NULL;
		if (! i)
			continue;
		if (snd_pcm_link(multi->slaves[0].pcm, multi->slaves[i].pcm) >= 0)
			multi->slaves[i].linked = multi->slaves[0].pcm;
	}
}

static int snd_pcm_multi_hw_params(snd_pcm_t *pcm, snd_pcm_hw_params_t *params)
{
	snd_pcm_multi_t *multi = pcm->private_data;
	unsigned int i;
	snd_pcm_hw_params_t sparams[multi->slaves_count];
	int err;
	for (i = 0; i < multi->slaves_count; ++i) {
		err = snd_pcm_multi_hw_refine_sprepare(pcm, i, &sparams[i]);
		assert(err >= 0);
		err = snd_pcm_multi_hw_refine_schange(pcm, i, params, &sparams[i]);
		assert(err >= 0);
		err = snd_pcm_multi_hw_params_slave(pcm, i, &sparams[i]);
		if (err < 0) {
			snd_pcm_multi_hw_refine_cchange(pcm, i, params, &sparams[i]);
			return err;
		}
	}
	reset_links(multi);
	return 0;
}

static int snd_pcm_multi_hw_free(snd_pcm_t *pcm)
{
	snd_pcm_multi_t *multi = pcm->private_data;
	unsigned int i;
	int err = 0;
	for (i = 0; i < multi->slaves_count; ++i) {
		snd_pcm_t *slave = multi->slaves[i].pcm;
		int e = snd_pcm_hw_free(slave);
		if (e < 0)
			err = e;
		if (!multi->slaves[i].linked)
			continue;
		e = snd_pcm_unlink(slave);
		if (e < 0)
			err = e;
		multi->slaves[i].linked = NULL;
	}
	return err;
}

static int snd_pcm_multi_sw_params(snd_pcm_t *pcm, snd_pcm_sw_params_t *params)
{
	snd_pcm_multi_t *multi = pcm->private_data;
	unsigned int i;
	int err;
	for (i = 0; i < multi->slaves_count; ++i) {
		snd_pcm_t *slave = multi->slaves[i].pcm;
		err = snd_pcm_sw_params(slave, params);
		if (err < 0)
			return err;
	}
	return 0;
}

static int snd_pcm_multi_status(snd_pcm_t *pcm, snd_pcm_status_t *status)
{
	snd_pcm_multi_t *multi = pcm->private_data;
	snd_pcm_t *slave = multi->slaves[multi->master_slave].pcm;
	return snd_pcm_status(slave, status);
}

static snd_pcm_state_t snd_pcm_multi_state(snd_pcm_t *pcm)
{
	snd_pcm_multi_t *multi = pcm->private_data;
	snd_pcm_t *slave = multi->slaves[multi->master_slave].pcm;
	return snd_pcm_state(slave);
}

static int snd_pcm_multi_hwsync(snd_pcm_t *pcm)
{
	snd_pcm_multi_t *multi = pcm->private_data;
	snd_pcm_t *slave = multi->slaves[multi->master_slave].pcm;
	return snd_pcm_hwsync(slave);
}

static int snd_pcm_multi_delay(snd_pcm_t *pcm, snd_pcm_sframes_t *delayp)
{
	snd_pcm_multi_t *multi = pcm->private_data;
	snd_pcm_t *slave = multi->slaves[multi->master_slave].pcm;
	return snd_pcm_delay(slave, delayp);
}

static snd_pcm_sframes_t snd_pcm_multi_avail_update(snd_pcm_t *pcm)
{
	snd_pcm_multi_t *multi = pcm->private_data;
	snd_pcm_sframes_t ret = LONG_MAX;
	unsigned int i;
	for (i = 0; i < multi->slaves_count; ++i) {
		snd_pcm_sframes_t avail;
		avail = snd_pcm_avail_update(multi->slaves[i].pcm);
		if (avail < 0)
			return avail;
		if (ret > avail)
			ret = avail;
	}
	return ret;
}

static int snd_pcm_multi_htimestamp(snd_pcm_t *pcm, snd_pcm_uframes_t *avail,
				    snd_htimestamp_t *tstamp)
{
	snd_pcm_multi_t *multi = pcm->private_data;
	snd_pcm_t *slave = multi->slaves[multi->master_slave].pcm;
	return snd_pcm_htimestamp(slave, avail, tstamp);
}

static int snd_pcm_multi_prepare(snd_pcm_t *pcm)
{
	snd_pcm_multi_t *multi = pcm->private_data;
	int result = 0, err;
	unsigned int i;
	for (i = 0; i < multi->slaves_count; ++i) {
		/* We call prepare to each slave even if it's linked.
		 * This is to make sure to sync non-mmaped control/status.
		 */
		err = snd_pcm_prepare(multi->slaves[i].pcm);
		if (err < 0)
			result = err;
	}
	return result;
}

static int snd_pcm_multi_reset(snd_pcm_t *pcm)
{
	snd_pcm_multi_t *multi = pcm->private_data;
	int result = 0, err;
	unsigned int i;
	for (i = 0; i < multi->slaves_count; ++i) {
		/* Reset each slave, as well as in prepare */
		err = snd_pcm_reset(multi->slaves[i].pcm);
		if (err < 0) 
			result = err;
	}
	return result;
}

/* when the first slave PCM is linked, it means that the whole multi
 * plugin instance is linked manually to another PCM.  in this case,
 * we need to trigger the master.
 */
static int snd_pcm_multi_start(snd_pcm_t *pcm)
{
	snd_pcm_multi_t *multi = pcm->private_data;
	int err = 0;
	unsigned int i;
	if (multi->slaves[0].linked)
		return snd_pcm_start(multi->slaves[0].linked);
	for (i = 0; i < multi->slaves_count; ++i) {
		if (multi->slaves[i].linked)
			continue;
		err = snd_pcm_start(multi->slaves[i].pcm);
		if (err < 0)
			return err;
	}
	return err;
}

static int snd_pcm_multi_drop(snd_pcm_t *pcm)
{
	snd_pcm_multi_t *multi = pcm->private_data;
	int err = 0;
	unsigned int i;
	if (multi->slaves[0].linked)
		return snd_pcm_drop(multi->slaves[0].linked);
	for (i = 0; i < multi->slaves_count; ++i) {
		if (multi->slaves[i].linked)
			continue;
		err = snd_pcm_drop(multi->slaves[i].pcm);
		if (err < 0)
			return err;
	}
	return err;
}

static int snd_pcm_multi_drain(snd_pcm_t *pcm)
{
	snd_pcm_multi_t *multi = pcm->private_data;
	int err = 0;
	unsigned int i;
	if (multi->slaves[0].linked)
		return snd_pcm_drain(multi->slaves[0].linked);
	for (i = 0; i < multi->slaves_count; ++i) {
		if (multi->slaves[i].linked)
			continue;
		err = snd_pcm_drain(multi->slaves[i].pcm);
		if (err < 0)
			return err;
	}
	return err;
}

static int snd_pcm_multi_pause(snd_pcm_t *pcm, int enable)
{
	snd_pcm_multi_t *multi = pcm->private_data;
	int err = 0;
	unsigned int i;
	if (multi->slaves[0].linked)
		return snd_pcm_pause(multi->slaves[0].linked, enable);
	for (i = 0; i < multi->slaves_count; ++i) {
		if (multi->slaves[i].linked)
			continue;
		err = snd_pcm_pause(multi->slaves[i].pcm, enable);
		if (err < 0)
			return err;
	}
	return err;
}

static int snd_pcm_multi_channel_info(snd_pcm_t *pcm, snd_pcm_channel_info_t *info)
{
	snd_pcm_multi_t *multi = pcm->private_data;
	unsigned int channel = info->channel;
	snd_pcm_multi_channel_t *c = &multi->channels[channel];
	int err;
	if (c->slave_idx < 0)
		return -ENXIO;
	info->channel = c->slave_channel;
	err = snd_pcm_channel_info(multi->slaves[c->slave_idx].pcm, info);
	info->channel = channel;
	return err;
}

static snd_pcm_sframes_t snd_pcm_multi_rewind(snd_pcm_t *pcm, snd_pcm_uframes_t frames)
{
	snd_pcm_multi_t *multi = pcm->private_data;
	unsigned int i;
	snd_pcm_uframes_t pos[multi->slaves_count];
	memset(pos, 0, sizeof(pos));
	for (i = 0; i < multi->slaves_count; ++i) {
		snd_pcm_t *slave_i = multi->slaves[i].pcm;
		snd_pcm_sframes_t f = snd_pcm_rewind(slave_i, frames);
		if (f < 0)
			return f;
		pos[i] = f;
		frames = f;
	}
	/* Realign the pointers */
	for (i = 0; i < multi->slaves_count; ++i) {
		snd_pcm_t *slave_i = multi->slaves[i].pcm;
		snd_pcm_uframes_t f = pos[i] - frames;
		snd_pcm_sframes_t result;
		if (f > 0) {
			result = INTERNAL(snd_pcm_forward)(slave_i, f);
			if (result < 0)
				return result;
			if ((snd_pcm_uframes_t)result != f)
				return -EIO;
		}
	}
	return frames;
}

static snd_pcm_sframes_t snd_pcm_multi_forward(snd_pcm_t *pcm, snd_pcm_uframes_t frames)
{
	snd_pcm_multi_t *multi = pcm->private_data;
	unsigned int i;
	snd_pcm_uframes_t pos[multi->slaves_count];
	memset(pos, 0, sizeof(pos));
	for (i = 0; i < multi->slaves_count; ++i) {
		snd_pcm_t *slave_i = multi->slaves[i].pcm;
		snd_pcm_sframes_t f = INTERNAL(snd_pcm_forward)(slave_i, frames);
		if (f < 0)
			return f;
		pos[i] = f;
		frames = f;
	}
	/* Realign the pointers */
	for (i = 0; i < multi->slaves_count; ++i) {
		snd_pcm_t *slave_i = multi->slaves[i].pcm;
		snd_pcm_uframes_t f = pos[i] - frames;
		snd_pcm_sframes_t result;
		if (f > 0) {
			result = snd_pcm_rewind(slave_i, f);
			if (result < 0)
				return result;
			if ((snd_pcm_uframes_t)result != f)
				return -EIO;
		}
	}
	return frames;
}

static int snd_pcm_multi_resume(snd_pcm_t *pcm)
{
	snd_pcm_multi_t *multi = pcm->private_data;
	int err = 0;
	unsigned int i;
	if (multi->slaves[0].linked)
		return snd_pcm_resume(multi->slaves[0].linked);
	for (i = 0; i < multi->slaves_count; ++i) {
		if (multi->slaves[i].linked)
			continue;
		err = snd_pcm_resume(multi->slaves[i].pcm);
		if (err < 0)
			return err;
	}
	return err;
}

/* if a multi plugin instance is linked as slaves, every slave PCMs
 * including the first one has to be relinked to the given master.
 */
static int snd_pcm_multi_link_slaves(snd_pcm_t *pcm, snd_pcm_t *master)
{ 
	snd_pcm_multi_t *multi = pcm->private_data;
	unsigned int i;
	int err;

	for (i = 0; i < multi->slaves_count; ++i) {
		snd_pcm_unlink(multi->slaves[i].pcm);
		multi->slaves[i].linked = NULL;
		err = snd_pcm_link(master, multi->slaves[i].pcm);
		if (err < 0) {
			reset_links(multi);
			return err;
		}
		multi->slaves[i].linked = master;
	}
	return 0;
}

/* linking to a multi as a master is easy - simply link to the first
 * slave element as its own slaves are already linked.
 */
static int snd_pcm_multi_link(snd_pcm_t *pcm1, snd_pcm_t *pcm2)
{
	snd_pcm_multi_t *multi = pcm1->private_data;
	if (multi->slaves[0].pcm->fast_ops->link)
		return multi->slaves[0].pcm->fast_ops->link(multi->slaves[0].pcm, pcm2);
	return -ENOSYS;
}

static int snd_pcm_multi_unlink(snd_pcm_t *pcm)
{
	snd_pcm_multi_t *multi = pcm->private_data;
	unsigned int i;

	for (i = 0; i < multi->slaves_count; ++i) {
		if (multi->slaves[i].linked)
			snd_pcm_unlink(multi->slaves[i].linked);
		multi->slaves[0].linked = NULL;
	}
	return 0;
}

static snd_pcm_sframes_t snd_pcm_multi_mmap_commit(snd_pcm_t *pcm,
						   snd_pcm_uframes_t offset,
						   snd_pcm_uframes_t size)
{
	snd_pcm_multi_t *multi = pcm->private_data;
	snd_pcm_t *slave;
	unsigned int i;
	snd_pcm_sframes_t result;

	for (i = 0; i < multi->slaves_count; ++i) {
		slave = multi->slaves[i].pcm;
		result = snd_pcm_mmap_commit(slave, offset, size);
		if (result < 0)
			return result;
		if ((snd_pcm_uframes_t)result != size)
			return -EIO;
	}
	return size;
}

static int snd_pcm_multi_munmap(snd_pcm_t *pcm)
{
	free(pcm->mmap_channels);
	free(pcm->running_areas);
	pcm->mmap_channels = NULL;
	pcm->running_areas = NULL;
	return 0;
}

static int snd_pcm_multi_mmap(snd_pcm_t *pcm)
{
	snd_pcm_multi_t *multi = pcm->private_data;
	unsigned int c;

	pcm->mmap_channels = calloc(pcm->channels,
				    sizeof(pcm->mmap_channels[0]));
	pcm->running_areas = calloc(pcm->channels,
				    sizeof(pcm->running_areas[0]));
	if (!pcm->mmap_channels || !pcm->running_areas) {
		snd_pcm_multi_munmap(pcm);
		return -ENOMEM;
	}

	/* Copy the slave mmapped buffer data */
	for (c = 0; c < pcm->channels; c++) {
		snd_pcm_multi_channel_t *chan = &multi->channels[c];
		snd_pcm_t *slave;
		if (chan->slave_idx < 0) {
			snd_pcm_multi_munmap(pcm);
			return -ENXIO;
		}
		slave = multi->slaves[chan->slave_idx].pcm;
		pcm->mmap_channels[c] =
			slave->mmap_channels[chan->slave_channel];
		pcm->mmap_channels[c].channel = c;
		pcm->running_areas[c] =
			slave->running_areas[chan->slave_channel];
	}
	return 0;
}

static void snd_pcm_multi_dump(snd_pcm_t *pcm, snd_output_t *out)
{
	snd_pcm_multi_t *multi = pcm->private_data;
	unsigned int k;
	snd_output_printf(out, "Multi PCM\n");
	snd_output_printf(out, "  Channel bindings:\n");
	for (k = 0; k < multi->channels_count; ++k) {
		snd_pcm_multi_channel_t *c = &multi->channels[k];
		if (c->slave_idx < 0)
			continue;
		snd_output_printf(out, "    %d: slave %d, channel %d\n", 
			k, c->slave_idx, c->slave_channel);
	}
	if (pcm->setup) {
		snd_output_printf(out, "Its setup is:\n");
		snd_pcm_dump_setup(pcm, out);
	}
	for (k = 0; k < multi->slaves_count; ++k) {
		snd_output_printf(out, "Slave #%d: ", k);
		snd_pcm_dump(multi->slaves[k].pcm, out);
	}
}

static const snd_pcm_ops_t snd_pcm_multi_ops = {
	.close = snd_pcm_multi_close,
	.info = snd_pcm_multi_info,
	.hw_refine = snd_pcm_multi_hw_refine,
	.hw_params = snd_pcm_multi_hw_params,
	.hw_free = snd_pcm_multi_hw_free,
	.sw_params = snd_pcm_multi_sw_params,
	.channel_info = snd_pcm_multi_channel_info,
	.dump = snd_pcm_multi_dump,
	.nonblock = snd_pcm_multi_nonblock,
	.async = snd_pcm_multi_async,
	.mmap = snd_pcm_multi_mmap,
	.munmap = snd_pcm_multi_munmap,
};

static const snd_pcm_fast_ops_t snd_pcm_multi_fast_ops = {
	.status = snd_pcm_multi_status,
	.state = snd_pcm_multi_state,
	.hwsync = snd_pcm_multi_hwsync,
	.delay = snd_pcm_multi_delay,
	.prepare = snd_pcm_multi_prepare,
	.reset = snd_pcm_multi_reset,
	.start = snd_pcm_multi_start,
	.drop = snd_pcm_multi_drop,
	.drain = snd_pcm_multi_drain,
	.pause = snd_pcm_multi_pause,
	.writei = snd_pcm_mmap_writei,
	.writen = snd_pcm_mmap_writen,
	.readi = snd_pcm_mmap_readi,
	.readn = snd_pcm_mmap_readn,
	.rewind = snd_pcm_multi_rewind,
	.forward = snd_pcm_multi_forward,
	.resume = snd_pcm_multi_resume,
	.link = snd_pcm_multi_link,
	.link_slaves = snd_pcm_multi_link_slaves,
	.unlink = snd_pcm_multi_unlink,
	.avail_update = snd_pcm_multi_avail_update,
	.mmap_commit = snd_pcm_multi_mmap_commit,
	.htimestamp = snd_pcm_multi_htimestamp,
	.poll_descriptors_count = snd_pcm_multi_poll_descriptors_count,
	.poll_descriptors = snd_pcm_multi_poll_descriptors,
	.poll_revents = snd_pcm_multi_poll_revents,
};

/**
 * \brief Creates a new Multi PCM
 * \param pcmp Returns created PCM handle
 * \param name Name of PCM
 * \param slaves_count Count of slaves
 * \param master_slave Master slave number
 * \param slaves_pcm Array with slave PCMs
 * \param schannels_count Array with slave channel counts
 * \param channels_count Count of channels
 * \param sidxs Array with channels indexes to slaves
 * \param schannels Array with slave channels
 * \param close_slaves When set, the slave PCM handle is closed
 * \retval zero on success otherwise a negative error code
 * \warning Using of this function might be dangerous in the sense
 *          of compatibility reasons. The prototype might be freely
 *          changed in future.
 */
int snd_pcm_multi_open(snd_pcm_t **pcmp, const char *name,
		       unsigned int slaves_count, unsigned int master_slave,
		       snd_pcm_t **slaves_pcm, unsigned int *schannels_count,
		       unsigned int channels_count,
		       int *sidxs, unsigned int *schannels,
		       int close_slaves)
{
	snd_pcm_t *pcm;
	snd_pcm_multi_t *multi;
	unsigned int i;
	snd_pcm_stream_t stream;
	char slave_map[64][64] = { { 0 } };
	int err;

	assert(pcmp);
	assert(slaves_count > 0 && slaves_pcm && schannels_count);
	assert(channels_count > 0 && sidxs && schannels);
	assert(master_slave < slaves_count);

	multi = calloc(1, sizeof(snd_pcm_multi_t));
	if (!multi) {
		return -ENOMEM;
	}

	stream = slaves_pcm[0]->stream;
	
	multi->slaves_count = slaves_count;
	multi->master_slave = master_slave;
	multi->slaves = calloc(slaves_count, sizeof(*multi->slaves));
	if (!multi->slaves) {
		free(multi);
		return -ENOMEM;
	}
	multi->channels_count = channels_count;
	multi->channels = calloc(channels_count, sizeof(*multi->channels));
	if (!multi->channels) {
		free(multi->slaves);
		free(multi);
		return -ENOMEM;
	}
	for (i = 0; i < slaves_count; ++i) {
		snd_pcm_multi_slave_t *slave = &multi->slaves[i];
		assert(slaves_pcm[i]->stream == stream);
		slave->pcm = slaves_pcm[i];
		slave->channels_count = schannels_count[i];
		slave->close_slave = close_slaves;
	}
	for (i = 0; i < channels_count; ++i) {
		snd_pcm_multi_channel_t *bind = &multi->channels[i];
		assert(sidxs[i] < (int)slaves_count);
		assert(schannels[i] < schannels_count[sidxs[i]]);
		bind->slave_idx = sidxs[i];
		bind->slave_channel = schannels[i];
		if (sidxs[i] < 0)
			continue;
		assert(!slave_map[sidxs[i]][schannels[i]]);
		slave_map[sidxs[i]][schannels[i]] = 1;
	}
	multi->channels_count = channels_count;

	err = snd_pcm_new(&pcm, SND_PCM_TYPE_MULTI, name, stream,
			  multi->slaves[0].pcm->mode);
	if (err < 0) {
		free(multi);
		return err;
	}
	pcm->mmap_rw = 1;
	pcm->mmap_shadow = 1; /* has own mmap method */
	pcm->ops = &snd_pcm_multi_ops;
	pcm->fast_ops = &snd_pcm_multi_fast_ops;
	pcm->private_data = multi;
	pcm->poll_fd = multi->slaves[master_slave].pcm->poll_fd;
	pcm->poll_events = multi->slaves[master_slave].pcm->poll_events;
	pcm->monotonic = multi->slaves[master_slave].pcm->monotonic;
	snd_pcm_link_hw_ptr(pcm, multi->slaves[master_slave].pcm);
	snd_pcm_link_appl_ptr(pcm, multi->slaves[master_slave].pcm);
	*pcmp = pcm;
	return 0;
}

/*! \page pcm_plugins

\section pcm_plugins_multi Plugin: Multiple streams to One

This plugin converts multiple streams to one.

\code
pcm.name {
        type multi              # Multiple streams conversion PCM
        slaves {		# Slaves definition
		ID STR		# Slave PCM name
		# or
		ID {
			pcm STR		# Slave PCM name
			# or
			pcm { } 	# Slave PCM definition
			channels INT	# Slave channels
		}
        }
	bindings {		# Bindings table
		N {
			slave STR	# Slave key
			channel INT	# Slave channel
		}
	}
	[master INT]		# Define the master slave
}
\endcode

For example, to bind two PCM streams with two-channel stereo (hw:0,0 and
hw:0,1) as one 4-channel stereo PCM stream, define like this:
\code
pcm.quad {
	type multi

	slaves.a.pcm "hw:0,0"
	slaves.a.channels 2
	slaves.b.pcm "hw:0,1"
	slaves.b.channels 2

	bindings.0.slave a
	bindings.0.channel 0
	bindings.1.slave a
	bindings.1.channel 1
	bindings.2.slave b
	bindings.2.channel 0
	bindings.3.slave b
	bindings.3.channel 1
}
\endcode
Note that the resultant pcm "quad" is not in the interleaved format
but in the "complex" format.  Hence, it's not accessible by applications
which can handle only the interleaved (or the non-interleaved) format.
In such a case, wrap this PCM with \ref pcm_plugins_route "route" or
\ref pcm_plugins_plug "plug" plugin.
\code
pcm.quad2 {
	type route
	slave.pcm "quad"
	ttable.0.0 1
	ttable.1.1 1
	ttable.2.2 1
	ttable.3.3 1
}
\endcode

\subsection pcm_plugins_multi_funcref Function reference

<UL>
  <LI>snd_pcm_multi_open()
  <LI>_snd_pcm_multi_open()
</UL>

*/

/**
 * \brief Creates a new Multi PCM
 * \param pcmp Returns created PCM handle
 * \param name Name of PCM
 * \param root Root configuration node
 * \param conf Configuration node with Multi PCM description
 * \param stream Stream type
 * \param mode Stream mode
 * \retval zero on success otherwise a negative error code
 * \warning Using of this function might be dangerous in the sense
 *          of compatibility reasons. The prototype might be freely
 *          changed in future.
 */
int _snd_pcm_multi_open(snd_pcm_t **pcmp, const char *name,
			snd_config_t *root, snd_config_t *conf,
			snd_pcm_stream_t stream, int mode)
{
	snd_config_iterator_t i, inext, j, jnext;
	snd_config_t *slaves = NULL;
	snd_config_t *bindings = NULL;
	int err;
	unsigned int idx;
	const char **slaves_id = NULL;
	snd_config_t **slaves_conf = NULL;
	snd_pcm_t **slaves_pcm = NULL;
	unsigned int *slaves_channels = NULL;
	int *channels_sidx = NULL;
	unsigned int *channels_schannel = NULL;
	unsigned int slaves_count = 0;
	long master_slave = 0;
	unsigned int channels_count = 0;
	snd_config_for_each(i, inext, conf) {
		snd_config_t *n = snd_config_iterator_entry(i);
		const char *id;
		if (snd_config_get_id(n, &id) < 0)
			continue;
		if (snd_pcm_conf_generic_id(id))
			continue;
		if (strcmp(id, "slaves") == 0) {
			if (snd_config_get_type(n) != SND_CONFIG_TYPE_COMPOUND) {
				SNDERR("Invalid type for %s", id);
				return -EINVAL;
			}
			slaves = n;
			continue;
		}
		if (strcmp(id, "bindings") == 0) {
			if (snd_config_get_type(n) != SND_CONFIG_TYPE_COMPOUND) {
				SNDERR("Invalid type for %s", id);
				return -EINVAL;
			}
			bindings = n;
			continue;
		}
		if (strcmp(id, "master") == 0) {
			if (snd_config_get_integer(n, &master_slave) < 0) {
				SNDERR("Invalid type for %s", id);
				return -EINVAL;
			}
			continue;
		}
		SNDERR("Unknown field %s", id);
		return -EINVAL;
	}
	if (!slaves) {
		SNDERR("slaves is not defined");
		return -EINVAL;
	}
	if (!bindings) {
		SNDERR("bindings is not defined");
		return -EINVAL;
	}
	snd_config_for_each(i, inext, slaves) {
		++slaves_count;
	}
	if (master_slave < 0 || master_slave >= (long)slaves_count) {
		SNDERR("Master slave is out of range (0-%u)\n", slaves_count-1);
		return -EINVAL;
	}
	snd_config_for_each(i, inext, bindings) {
		long cchannel;
		snd_config_t *m = snd_config_iterator_entry(i);
		const char *id;
		if (snd_config_get_id(m, &id) < 0)
			continue;
		err = safe_strtol(id, &cchannel);
		if (err < 0 || cchannel < 0) {
			SNDERR("Invalid channel number: %s", id);
			return -EINVAL;
		}
		if ((unsigned long)cchannel >= channels_count)
			channels_count = cchannel + 1;
	}
	if (channels_count == 0) {
		SNDERR("No channels defined");
		return -EINVAL;
	}
	slaves_id = calloc(slaves_count, sizeof(*slaves_id));
	slaves_conf = calloc(slaves_count, sizeof(*slaves_conf));
	slaves_pcm = calloc(slaves_count, sizeof(*slaves_pcm));
	slaves_channels = calloc(slaves_count, sizeof(*slaves_channels));
	channels_sidx = calloc(channels_count, sizeof(*channels_sidx));
	channels_schannel = calloc(channels_count, sizeof(*channels_schannel));
	if (!slaves_id || !slaves_conf || !slaves_pcm || !slaves_channels ||
	    !channels_sidx || !channels_schannel) {
		err = -ENOMEM;
		goto _free;
	}
	idx = 0;
	for (idx = 0; idx < channels_count; ++idx)
		channels_sidx[idx] = -1;
	idx = 0;
	snd_config_for_each(i, inext, slaves) {
		snd_config_t *m = snd_config_iterator_entry(i);
		const char *id;
		int channels;
		if (snd_config_get_id(m, &id) < 0)
			continue;
		slaves_id[idx] = id;
		err = snd_pcm_slave_conf(root, m, &slaves_conf[idx], 1,
					 SND_PCM_HW_PARAM_CHANNELS, SCONF_MANDATORY, &channels);
		if (err < 0)
			goto _free;
		slaves_channels[idx] = channels;
		++idx;
	}

	snd_config_for_each(i, inext, bindings) {
		snd_config_t *m = snd_config_iterator_entry(i);
		long cchannel = -1;
		long schannel = -1;
		int slave = -1;
		long val;
		const char *str;
		const char *id;
		if (snd_config_get_id(m, &id) < 0)
			continue;
		err = safe_strtol(id, &cchannel);
		if (err < 0 || cchannel < 0) {
			SNDERR("Invalid channel number: %s", id);
			err = -EINVAL;
			goto _free;
		}
		snd_config_for_each(j, jnext, m) {
			snd_config_t *n = snd_config_iterator_entry(j);
			const char *id;
			if (snd_config_get_id(n, &id) < 0)
				continue;
			if (strcmp(id, "comment") == 0)
				continue;
			if (strcmp(id, "slave") == 0) {
				char buf[32];
				unsigned int k;
				err = snd_config_get_string(n, &str);
				if (err < 0) {
					err = snd_config_get_integer(n, &val);
					if (err < 0) {
						SNDERR("Invalid value for %s", id);
						goto _free;
					}
					sprintf(buf, "%ld", val);
					str = buf;
				}
				for (k = 0; k < slaves_count; ++k) {
					if (strcmp(slaves_id[k], str) == 0)
						slave = k;
				}
				continue;
			}
			if (strcmp(id, "channel") == 0) {
				err = snd_config_get_integer(n, &schannel);
				if (err < 0) {
					SNDERR("Invalid type for %s", id);
					goto _free;
				}
				continue;
			}
			SNDERR("Unknown field %s", id);
			err = -EINVAL;
			goto _free;
		}
		if (slave < 0 || (unsigned int)slave >= slaves_count) {
			SNDERR("Invalid or missing sidx for channel %s", id);
			err = -EINVAL;
			goto _free;
		}
		if (schannel < 0 || 
		    (unsigned int) schannel >= slaves_channels[slave]) {
			SNDERR("Invalid or missing schannel for channel %s", id);
			err = -EINVAL;
			goto _free;
		}
		channels_sidx[cchannel] = slave;
		channels_schannel[cchannel] = schannel;
	}
	
	for (idx = 0; idx < slaves_count; ++idx) {
		err = snd_pcm_open_slave(&slaves_pcm[idx], root,
					 slaves_conf[idx], stream, mode,
					 conf);
		if (err < 0)
			goto _free;
		snd_config_delete(slaves_conf[idx]);
		slaves_conf[idx] = NULL;
	}
	err = snd_pcm_multi_open(pcmp, name, slaves_count, master_slave,
				 slaves_pcm, slaves_channels,
				 channels_count,
				 channels_sidx, channels_schannel,
				 1);
_free:
	if (err < 0) {
		for (idx = 0; idx < slaves_count; ++idx) {
			if (slaves_pcm[idx])
				snd_pcm_close(slaves_pcm[idx]);
		}
	}
	if (slaves_conf) {
		for (idx = 0; idx < slaves_count; ++idx) {
			if (slaves_conf[idx])
				snd_config_delete(slaves_conf[idx]);
		}
		free(slaves_conf);
	}
	free(slaves_pcm);
	free(slaves_channels);
	free(channels_sidx);
	free(channels_schannel);
	free(slaves_id);
	return err;
}
#ifndef DOC_HIDDEN
SND_DLSYM_BUILD_VERSION(_snd_pcm_multi_open, SND_PCM_DLSYM_VERSION);
#endif
