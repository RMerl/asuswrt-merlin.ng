/**
 * \file pcm/pcm_share.c
 * \ingroup PCM_Plugins
 * \brief PCM Share Plugin Interface
 * \author Abramo Bagnara <abramo@alsa-project.org>
 * \date 2000-2001
 */
/*
 *  PCM - Share
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
#include <limits.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <math.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <sys/shm.h>
#include <pthread.h>
#include "pcm_local.h"

#ifndef PIC
/* entry for static linking */
const char *_snd_module_pcm_share = "";
#endif

#ifndef DOC_HIDDEN

static LIST_HEAD(snd_pcm_share_slaves);
static pthread_mutex_t snd_pcm_share_slaves_mutex = PTHREAD_MUTEX_INITIALIZER;

#ifdef MUTEX_DEBUG
#define Pthread_mutex_lock(mutex) \
char *snd_pcm_share_slaves_mutex_holder;
do { \
	int err = pthread_mutex_trylock(mutex); \
	if (err < 0) { \
		fprintf(stderr, "lock " #mutex " is busy (%s): waiting in " __FUNCTION__ "\n", *(mutex##_holder)); \
		pthread_mutex_lock(mutex); \
		fprintf(stderr, "... got\n"); \
	} \
	*(mutex##_holder) = __FUNCTION__; \
} while (0)

#define Pthread_mutex_unlock(mutex) \
do { \
	*(mutex##_holder) = 0; \
	pthread_mutex_unlock(mutex); \
} while (0)
#else
#define Pthread_mutex_lock(mutex) pthread_mutex_lock(mutex)
#define Pthread_mutex_unlock(mutex) pthread_mutex_unlock(mutex)
#endif

typedef struct {
	struct list_head clients;
	struct list_head list;
	snd_pcm_t *pcm;
	snd_pcm_format_t format;
	int rate;
	unsigned int channels;
	snd_pcm_sframes_t period_time;
	snd_pcm_sframes_t buffer_time;
	unsigned int open_count;
	unsigned int setup_count;
	unsigned int prepared_count;
	unsigned int running_count;
	snd_pcm_uframes_t safety_threshold;
	snd_pcm_uframes_t silence_frames;
	snd_pcm_sw_params_t sw_params;
	snd_pcm_uframes_t hw_ptr;
	int poll[2];
	int polling;
	pthread_t thread;
	pthread_mutex_t mutex;
#ifdef MUTEX_DEBUG
	char *mutex_holder;
#endif
	pthread_cond_t poll_cond;
} snd_pcm_share_slave_t;

typedef struct {
	struct list_head list;
	snd_pcm_t *pcm;
	snd_pcm_share_slave_t *slave;
	unsigned int channels;
	unsigned int *slave_channels;
	int drain_silenced;
	snd_htimestamp_t trigger_tstamp;
	snd_pcm_state_t state;
	snd_pcm_uframes_t hw_ptr;
	snd_pcm_uframes_t appl_ptr;
	int ready;
	int client_socket;
	int slave_socket;
} snd_pcm_share_t;

#endif /* DOC_HIDDEN */

static void _snd_pcm_share_stop(snd_pcm_t *pcm, snd_pcm_state_t state);

static snd_pcm_uframes_t snd_pcm_share_slave_avail(snd_pcm_share_slave_t *slave)
{
	snd_pcm_sframes_t avail;
	snd_pcm_t *pcm = slave->pcm;
  	avail = slave->hw_ptr - *pcm->appl.ptr;
	if (pcm->stream == SND_PCM_STREAM_PLAYBACK)
		avail += pcm->buffer_size;
	if (avail < 0)
		avail += pcm->boundary;
	return avail;
}

/* Warning: take the mutex before to call this */
/* Return number of frames to mmap_commit the slave */
static snd_pcm_uframes_t _snd_pcm_share_slave_forward(snd_pcm_share_slave_t *slave)
{
	struct list_head *i;
	snd_pcm_uframes_t buffer_size, boundary;
	snd_pcm_uframes_t slave_appl_ptr;
	snd_pcm_sframes_t frames, safety_frames;
	snd_pcm_sframes_t min_frames, max_frames;
	snd_pcm_uframes_t avail, slave_avail;
	snd_pcm_uframes_t slave_hw_avail;
	slave_avail = snd_pcm_share_slave_avail(slave);
	boundary = slave->pcm->boundary;
	buffer_size = slave->pcm->buffer_size;
	min_frames = slave_avail;
	max_frames = 0;
	slave_appl_ptr = *slave->pcm->appl.ptr;
	list_for_each(i, &slave->clients) {
		snd_pcm_share_t *share = list_entry(i, snd_pcm_share_t, list);
		snd_pcm_t *pcm = share->pcm;
		switch (share->state) {
		case SND_PCM_STATE_RUNNING:
			break;
		case SND_PCM_STATE_DRAINING:
			if (pcm->stream != SND_PCM_STREAM_PLAYBACK)
				continue;
			break;
		default:
			continue;
		}
		avail = snd_pcm_mmap_avail(pcm);
		frames = slave_avail - avail;
		if (frames > max_frames)
			max_frames = frames;
		if (share->state != SND_PCM_STATE_RUNNING)
			continue;
		if (frames < min_frames)
			min_frames = frames;
	}
	if (max_frames == 0)
		return 0;
	frames = min_frames;
	/* Slave xrun prevention */
	slave_hw_avail = buffer_size - slave_avail;
	safety_frames = slave->safety_threshold - slave_hw_avail;
	if (safety_frames > 0 &&
	    frames < safety_frames) {
		/* Avoid to pass over the last */
		if (max_frames < safety_frames)
			frames = max_frames;
		else
			frames = safety_frames;
	}
	if (frames < 0)
		return 0;
	return frames;
}


/* 
   - stop PCM on xrun
   - update poll status
   - draining silencing
   - return distance in frames to next event
*/
static snd_pcm_uframes_t _snd_pcm_share_missing(snd_pcm_t *pcm)
{
	snd_pcm_share_t *share = pcm->private_data;
	snd_pcm_share_slave_t *slave = share->slave;
	snd_pcm_t *spcm = slave->pcm;
	snd_pcm_uframes_t buffer_size = spcm->buffer_size;
	int ready = 1, running = 0;
	snd_pcm_uframes_t avail = 0, slave_avail;
	snd_pcm_sframes_t hw_avail;
	snd_pcm_uframes_t missing = INT_MAX;
	snd_pcm_sframes_t ready_missing;
	// printf("state=%s hw_ptr=%ld appl_ptr=%ld slave appl_ptr=%ld safety=%ld silence=%ld\n", snd_pcm_state_name(share->state), slave->hw_ptr, share->appl_ptr, *slave->pcm->appl_ptr, slave->safety_threshold, slave->silence_frames);
	switch (share->state) {
	case SND_PCM_STATE_RUNNING:
		break;
	case SND_PCM_STATE_DRAINING:
		if (pcm->stream == SND_PCM_STREAM_PLAYBACK)
			break;
		/* Fall through */
	default:
		return INT_MAX;
	}
	share->hw_ptr = slave->hw_ptr;
	avail = snd_pcm_mmap_avail(pcm);
	if (avail >= pcm->stop_threshold) {
		_snd_pcm_share_stop(pcm, share->state == SND_PCM_STATE_DRAINING ? SND_PCM_STATE_SETUP : SND_PCM_STATE_XRUN);
		goto update_poll;
	}
	hw_avail = buffer_size - avail;
	slave_avail = snd_pcm_share_slave_avail(slave);
	if (avail < slave_avail) {
		/* Some frames need still to be transferred */
		snd_pcm_sframes_t slave_hw_avail = buffer_size - slave_avail;
		snd_pcm_sframes_t safety_missing = slave_hw_avail - slave->safety_threshold;
		if (safety_missing < 0) {
			snd_pcm_sframes_t err;
			snd_pcm_sframes_t frames = slave_avail - avail;
			if (-safety_missing <= frames) {
				frames = -safety_missing;
				missing = 1;
			}
			err = snd_pcm_mmap_commit(spcm, snd_pcm_mmap_offset(spcm), frames);
			if (err < 0) {
				SYSMSG("snd_pcm_mmap_commit error");
				return INT_MAX;
			}
			if (err != frames)
				SYSMSG("commit returns %ld for size %ld", err, frames);
			slave_avail -= err;
		} else {
			if (safety_missing == 0)
				missing = 1;
			else
				missing = safety_missing;
		}
	}
	switch (share->state) {
	case SND_PCM_STATE_DRAINING:
		if (pcm->stream == SND_PCM_STREAM_PLAYBACK) {
			if (hw_avail <= 0) {
				_snd_pcm_share_stop(pcm, SND_PCM_STATE_SETUP);
				break;
			}
			if ((snd_pcm_uframes_t)hw_avail < missing)
				missing = hw_avail;
			running = 1;
			ready = 0;
		}
		break;
	case SND_PCM_STATE_RUNNING:
		if (avail >= pcm->stop_threshold) {
			_snd_pcm_share_stop(pcm, SND_PCM_STATE_XRUN);
			break;
		} else {
			snd_pcm_uframes_t xrun_missing = pcm->stop_threshold - avail;
			if (missing > xrun_missing)
				missing = xrun_missing;
		}
		ready_missing = pcm->avail_min - avail;
		if (ready_missing > 0) {
			ready = 0;
			if (missing > (snd_pcm_uframes_t)ready_missing)
				missing = ready_missing;
		}
		running = 1;
		break;
	default:
		SNDERR("invalid shared PCM state %d", share->state);
		return INT_MAX;
	}

 update_poll:
	if (ready != share->ready) {
		char buf[1];
		if (pcm->stream == SND_PCM_STREAM_PLAYBACK) {
			if (ready)
				read(share->slave_socket, buf, 1);
			else
				write(share->client_socket, buf, 1);
		} else {
			if (ready)
				write(share->slave_socket, buf, 1);
			else
				read(share->client_socket, buf, 1);
		}
		share->ready = ready;
	}
	if (!running)
		return INT_MAX;
	if (pcm->stream == SND_PCM_STREAM_PLAYBACK &&
	    share->state == SND_PCM_STATE_DRAINING &&
	    !share->drain_silenced) {
		/* drain silencing */
		if (avail >= slave->silence_frames) {
			snd_pcm_uframes_t offset = share->appl_ptr % buffer_size;
			snd_pcm_uframes_t xfer = 0;
			snd_pcm_uframes_t size = slave->silence_frames;
			while (xfer < size) {
				snd_pcm_uframes_t frames = size - xfer;
				snd_pcm_uframes_t cont = buffer_size - offset;
				if (cont < frames)
					frames = cont;
				snd_pcm_areas_silence(pcm->running_areas, offset, pcm->channels, frames, pcm->format);
				offset += frames;
				if (offset >= buffer_size)
					offset = 0;
				xfer += frames;
			}
			share->drain_silenced = 1;
		} else {
			snd_pcm_uframes_t silence_missing;
			silence_missing = slave->silence_frames - avail;
			if (silence_missing < missing)
				missing = silence_missing;
		}
	}
	// printf("missing=%d\n", missing);
	return missing;
}

static snd_pcm_uframes_t _snd_pcm_share_slave_missing(snd_pcm_share_slave_t *slave)
{
	snd_pcm_uframes_t missing = INT_MAX;
	struct list_head *i;
	/* snd_pcm_sframes_t avail = */ snd_pcm_avail_update(slave->pcm);
	slave->hw_ptr = *slave->pcm->hw.ptr;
	list_for_each(i, &slave->clients) {
		snd_pcm_share_t *share = list_entry(i, snd_pcm_share_t, list);
		snd_pcm_t *pcm = share->pcm;
		snd_pcm_uframes_t m = _snd_pcm_share_missing(pcm);
		if (m < missing)
			missing = m;
	}
	return missing;
}

static void *snd_pcm_share_thread(void *data)
{
	snd_pcm_share_slave_t *slave = data;
	snd_pcm_t *spcm = slave->pcm;
	struct pollfd pfd[2];
	int err;

	pfd[0].fd = slave->poll[0];
	pfd[0].events = POLLIN;
	err = snd_pcm_poll_descriptors(spcm, &pfd[1], 1);
	if (err != 1) {
		SNDERR("invalid poll descriptors %d", err);
		return NULL;
	}
	Pthread_mutex_lock(&slave->mutex);
	err = pipe(slave->poll);
	if (err < 0) {
		SYSERR("can't create a pipe");
		return NULL;
	}
	while (slave->open_count > 0) {
		snd_pcm_uframes_t missing;
		// printf("begin min_missing\n");
		missing = _snd_pcm_share_slave_missing(slave);
		// printf("min_missing=%ld\n", missing);
		if (missing < INT_MAX) {
			snd_pcm_uframes_t hw_ptr;
			snd_pcm_sframes_t avail_min;
			hw_ptr = slave->hw_ptr + missing;
			hw_ptr += spcm->period_size - 1;
			if (hw_ptr >= spcm->boundary)
				hw_ptr -= spcm->boundary;
			hw_ptr -= hw_ptr % spcm->period_size;
			avail_min = hw_ptr - *spcm->appl.ptr;
			if (spcm->stream == SND_PCM_STREAM_PLAYBACK)
				avail_min += spcm->buffer_size;
			if (avail_min < 0)
				avail_min += spcm->boundary;
			// printf("avail_min=%d\n", avail_min);
			if ((snd_pcm_uframes_t)avail_min != spcm->avail_min) {
				snd_pcm_sw_params_set_avail_min(spcm, &slave->sw_params, avail_min);
				err = snd_pcm_sw_params(spcm, &slave->sw_params);
				if (err < 0) {
					SYSERR("snd_pcm_sw_params error");
					return NULL;
				}
			}
			slave->polling = 1;
			Pthread_mutex_unlock(&slave->mutex);
			err = poll(pfd, 2, -1);
			Pthread_mutex_lock(&slave->mutex);
			if (pfd[0].revents & POLLIN) {
				char buf[1];
				read(pfd[0].fd, buf, 1);
			}
		} else {
			slave->polling = 0;
			pthread_cond_wait(&slave->poll_cond, &slave->mutex);
		}
	}
	Pthread_mutex_unlock(&slave->mutex);
	return NULL;
}

static void _snd_pcm_share_update(snd_pcm_t *pcm)
{
	snd_pcm_share_t *share = pcm->private_data;
	snd_pcm_share_slave_t *slave = share->slave;
	snd_pcm_t *spcm = slave->pcm;
	snd_pcm_uframes_t missing;
	/* snd_pcm_sframes_t avail = */ snd_pcm_avail_update(spcm);
	slave->hw_ptr = *slave->pcm->hw.ptr;
	missing = _snd_pcm_share_missing(pcm);
	// printf("missing %ld\n", missing);
	if (!slave->polling) {
		pthread_cond_signal(&slave->poll_cond);
		return;
	}
	if (missing < INT_MAX) {
		snd_pcm_uframes_t hw_ptr;
		snd_pcm_sframes_t avail_min;
		hw_ptr = slave->hw_ptr + missing;
		hw_ptr += spcm->period_size - 1;
		if (hw_ptr >= spcm->boundary)
			hw_ptr -= spcm->boundary;
		hw_ptr -= hw_ptr % spcm->period_size;
		avail_min = hw_ptr - *spcm->appl.ptr;
		if (spcm->stream == SND_PCM_STREAM_PLAYBACK)
			avail_min += spcm->buffer_size;
		if (avail_min < 0)
			avail_min += spcm->boundary;
		if ((snd_pcm_uframes_t)avail_min < spcm->avail_min) {
			int err;
			snd_pcm_sw_params_set_avail_min(spcm, &slave->sw_params, avail_min);
			err = snd_pcm_sw_params(spcm, &slave->sw_params);
			if (err < 0) {
				SYSERR("snd_pcm_sw_params error");
				return;
			}
		}
	}
}

static int snd_pcm_share_nonblock(snd_pcm_t *pcm ATTRIBUTE_UNUSED, int nonblock ATTRIBUTE_UNUSED)
{
	return 0;
}

static int snd_pcm_share_async(snd_pcm_t *pcm ATTRIBUTE_UNUSED, int sig ATTRIBUTE_UNUSED, pid_t pid ATTRIBUTE_UNUSED)
{
	return -ENOSYS;
}

static int snd_pcm_share_info(snd_pcm_t *pcm, snd_pcm_info_t *info)
{
	snd_pcm_share_t *share = pcm->private_data;
	return snd_pcm_info(share->slave->pcm, info);
}

static int snd_pcm_share_hw_refine_cprepare(snd_pcm_t *pcm, snd_pcm_hw_params_t *params)
{
	snd_pcm_share_t *share = pcm->private_data;
	snd_pcm_share_slave_t *slave = share->slave;
	snd_pcm_access_mask_t access_mask;
	int err;
	snd_pcm_access_mask_any(&access_mask);
	snd_pcm_access_mask_reset(&access_mask, SND_PCM_ACCESS_MMAP_INTERLEAVED);
	err = _snd_pcm_hw_param_set_mask(params, SND_PCM_HW_PARAM_ACCESS,
					 &access_mask);
	if (err < 0)
		return err;
	err = _snd_pcm_hw_param_set(params, SND_PCM_HW_PARAM_CHANNELS,
				    share->channels, 0);
	if (err < 0)
		return err;
	if (slave->format != SND_PCM_FORMAT_UNKNOWN) {
		err = _snd_pcm_hw_params_set_format(params, slave->format);
		if (err < 0)
			return err;
	}

	if (slave->rate >= 0) {
		err = _snd_pcm_hw_param_set(params, SND_PCM_HW_PARAM_RATE,
					    slave->rate, 0);
		if (err < 0)
			return err;
	}
	if (slave->period_time >= 0) {
		err = _snd_pcm_hw_param_set(params, SND_PCM_HW_PARAM_PERIOD_TIME,
					    slave->period_time, 0);
		if (err < 0)
			return err;
	}
	if (slave->buffer_time >= 0) {
		err = _snd_pcm_hw_param_set(params, SND_PCM_HW_PARAM_BUFFER_TIME,
					    slave->buffer_time, 0);
		if (err < 0)
			return err;
	}
	params->info |= SND_PCM_INFO_DOUBLE;
	return 0;
}

static int snd_pcm_share_hw_refine_sprepare(snd_pcm_t *pcm, snd_pcm_hw_params_t *sparams)
{
	snd_pcm_share_t *share = pcm->private_data;
	snd_pcm_share_slave_t *slave = share->slave;
	snd_pcm_access_mask_t saccess_mask = { SND_PCM_ACCBIT_MMAP };
	_snd_pcm_hw_params_any(sparams);
	_snd_pcm_hw_param_set_mask(sparams, SND_PCM_HW_PARAM_ACCESS,
				   &saccess_mask);
	_snd_pcm_hw_param_set(sparams, SND_PCM_HW_PARAM_CHANNELS,
			      slave->channels, 0);
	return 0;
}

static int snd_pcm_share_hw_refine_schange(snd_pcm_t *pcm ATTRIBUTE_UNUSED, snd_pcm_hw_params_t *params,
					  snd_pcm_hw_params_t *sparams)
{
	int err;
	unsigned int links = (SND_PCM_HW_PARBIT_FORMAT |
			      SND_PCM_HW_PARBIT_SUBFORMAT |
			      SND_PCM_HW_PARBIT_RATE |
			      SND_PCM_HW_PARBIT_PERIOD_SIZE |
			      SND_PCM_HW_PARBIT_PERIOD_TIME |
			      SND_PCM_HW_PARBIT_BUFFER_SIZE |
			      SND_PCM_HW_PARBIT_BUFFER_TIME |
			      SND_PCM_HW_PARBIT_PERIODS);
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
	
static int snd_pcm_share_hw_refine_cchange(snd_pcm_t *pcm ATTRIBUTE_UNUSED, snd_pcm_hw_params_t *params,
					   snd_pcm_hw_params_t *sparams)
{
	int err;
	unsigned int links = (SND_PCM_HW_PARBIT_FORMAT |
			      SND_PCM_HW_PARBIT_SUBFORMAT |
			      SND_PCM_HW_PARBIT_RATE |
			      SND_PCM_HW_PARBIT_PERIOD_SIZE |
			      SND_PCM_HW_PARBIT_PERIOD_TIME |
			      SND_PCM_HW_PARBIT_BUFFER_SIZE |
			      SND_PCM_HW_PARBIT_BUFFER_TIME |
			      SND_PCM_HW_PARBIT_PERIODS);
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
	return 0;
}

static int snd_pcm_share_hw_refine_slave(snd_pcm_t *pcm, snd_pcm_hw_params_t *params)
{
	snd_pcm_share_t *share = pcm->private_data;
	return snd_pcm_hw_refine(share->slave->pcm, params);
}

static int snd_pcm_share_hw_params_slave(snd_pcm_t *pcm, snd_pcm_hw_params_t *params)
{
	snd_pcm_share_t *share = pcm->private_data;
	return _snd_pcm_hw_params(share->slave->pcm, params);
}

static int snd_pcm_share_hw_refine(snd_pcm_t *pcm, snd_pcm_hw_params_t *params)
{
	return snd_pcm_hw_refine_slave(pcm, params,
				       snd_pcm_share_hw_refine_cprepare,
				       snd_pcm_share_hw_refine_cchange,
				       snd_pcm_share_hw_refine_sprepare,
				       snd_pcm_share_hw_refine_schange,
				       snd_pcm_share_hw_refine_slave);
}

static int snd_pcm_share_hw_params(snd_pcm_t *pcm, snd_pcm_hw_params_t *params)
{
	snd_pcm_share_t *share = pcm->private_data;
	snd_pcm_share_slave_t *slave = share->slave;
	snd_pcm_t *spcm = slave->pcm;
	int err = 0;
	Pthread_mutex_lock(&slave->mutex);
	if (slave->setup_count) {
		err = _snd_pcm_hw_params_set_format(params, spcm->format);
		if (err < 0)
			goto _err;
		err = _snd_pcm_hw_params_set_subformat(params, spcm->subformat);
		if (err < 0)
			goto _err;
		err = _snd_pcm_hw_param_set_minmax(params, SND_PCM_HW_PARAM_RATE,
						   spcm->rate, 0, 
						   spcm->rate, 1);
		if (err < 0)
			goto _err;
		err = _snd_pcm_hw_param_set_minmax(params, SND_PCM_HW_PARAM_PERIOD_TIME,
						   spcm->period_time, 0,
						   spcm->period_time, 1);
		if (err < 0)
			goto _err;
		err = _snd_pcm_hw_param_set(params, SND_PCM_HW_PARAM_BUFFER_SIZE,
					    spcm->buffer_size, 0);
	_err:
		if (err < 0) {
			SNDERR("slave is already running with incompatible setup");
			err = -EBUSY;
			goto _end;
		}
	} else {
		err = snd_pcm_hw_params_slave(pcm, params,
					      snd_pcm_share_hw_refine_cchange,
					      snd_pcm_share_hw_refine_sprepare,
					      snd_pcm_share_hw_refine_schange,
					      snd_pcm_share_hw_params_slave);
		if (err < 0)
			goto _end;
		snd_pcm_sw_params_current(slave->pcm, &slave->sw_params);
		/* >= 30 ms */
		slave->safety_threshold = slave->pcm->rate * 30 / 1000;
		slave->safety_threshold += slave->pcm->period_size - 1;
		slave->safety_threshold -= slave->safety_threshold % slave->pcm->period_size;
		slave->silence_frames = slave->safety_threshold;
		if (slave->pcm->stream == SND_PCM_STREAM_PLAYBACK)
			snd_pcm_areas_silence(slave->pcm->running_areas, 0, slave->pcm->channels, slave->pcm->buffer_size, slave->pcm->format);
	}
	share->state = SND_PCM_STATE_SETUP;
	slave->setup_count++;
 _end:
	Pthread_mutex_unlock(&slave->mutex);
	return err;
}

static int snd_pcm_share_hw_free(snd_pcm_t *pcm)
{
	snd_pcm_share_t *share = pcm->private_data;
	snd_pcm_share_slave_t *slave = share->slave;
	int err = 0;
	Pthread_mutex_lock(&slave->mutex);
	slave->setup_count--;
	if (slave->setup_count == 0)
		err = snd_pcm_hw_free(slave->pcm);
	share->state = SND_PCM_STATE_OPEN;
	Pthread_mutex_unlock(&slave->mutex);
	return err;
}

static int snd_pcm_share_sw_params(snd_pcm_t *pcm ATTRIBUTE_UNUSED, snd_pcm_sw_params_t *params ATTRIBUTE_UNUSED)
{
	return 0;
}

static int snd_pcm_share_status(snd_pcm_t *pcm, snd_pcm_status_t *status)
{
	snd_pcm_share_t *share = pcm->private_data;
	snd_pcm_share_slave_t *slave = share->slave;
	int err = 0;
	snd_pcm_sframes_t sd = 0, d = 0;
	Pthread_mutex_lock(&slave->mutex);
	if (pcm->stream == SND_PCM_STREAM_PLAYBACK) {
		status->avail = snd_pcm_mmap_playback_avail(pcm);
		if (share->state != SND_PCM_STATE_RUNNING &&
		    share->state != SND_PCM_STATE_DRAINING)
			goto _notrunning;
		d = pcm->buffer_size - status->avail;
	} else {
		status->avail = snd_pcm_mmap_capture_avail(pcm);
		if (share->state != SND_PCM_STATE_RUNNING)
			goto _notrunning;
		d = status->avail;
	}
	err = snd_pcm_delay(slave->pcm, &sd);
	if (err < 0)
		goto _end;
 _notrunning:
	status->delay = sd + d;
	status->state = share->state;
	status->trigger_tstamp = share->trigger_tstamp;
 _end:
	Pthread_mutex_unlock(&slave->mutex);
	return err;
}

static snd_pcm_state_t snd_pcm_share_state(snd_pcm_t *pcm)
{
	snd_pcm_share_t *share = pcm->private_data;
	return share->state;
}

static int _snd_pcm_share_hwsync(snd_pcm_t *pcm)
{
	snd_pcm_share_t *share = pcm->private_data;
	snd_pcm_share_slave_t *slave = share->slave;
	switch (share->state) {
	case SND_PCM_STATE_XRUN:
		return -EPIPE;
	default:
		break;
	}
	return snd_pcm_hwsync(slave->pcm);
}

static int snd_pcm_share_hwsync(snd_pcm_t *pcm)
{
	snd_pcm_share_t *share = pcm->private_data;
	snd_pcm_share_slave_t *slave = share->slave;
	int err;
	Pthread_mutex_lock(&slave->mutex);
	err = _snd_pcm_share_hwsync(pcm);
	Pthread_mutex_unlock(&slave->mutex);
	return err;
}

static int _snd_pcm_share_delay(snd_pcm_t *pcm, snd_pcm_sframes_t *delayp)
{
	snd_pcm_share_t *share = pcm->private_data;
	snd_pcm_share_slave_t *slave = share->slave;
	switch (share->state) {
	case SND_PCM_STATE_XRUN:
		return -EPIPE;
	case SND_PCM_STATE_RUNNING:
		break;
	case SND_PCM_STATE_DRAINING:
		if (pcm->stream == SND_PCM_STREAM_PLAYBACK)
			break;
		/* Fall through */
	default:
		return -EBADFD;
	}
	return snd_pcm_delay(slave->pcm, delayp);
}

static int snd_pcm_share_delay(snd_pcm_t *pcm, snd_pcm_sframes_t *delayp)
{
	snd_pcm_share_t *share = pcm->private_data;
	snd_pcm_share_slave_t *slave = share->slave;
	int err;
	Pthread_mutex_lock(&slave->mutex);
	err = _snd_pcm_share_delay(pcm, delayp);
	Pthread_mutex_unlock(&slave->mutex);
	return err;
}

static snd_pcm_sframes_t snd_pcm_share_avail_update(snd_pcm_t *pcm)
{
	snd_pcm_share_t *share = pcm->private_data;
	snd_pcm_share_slave_t *slave = share->slave;
	snd_pcm_sframes_t avail;
	Pthread_mutex_lock(&slave->mutex);
	if (share->state == SND_PCM_STATE_RUNNING) {
		avail = snd_pcm_avail_update(slave->pcm);
		if (avail < 0) {
			Pthread_mutex_unlock(&slave->mutex);
			return avail;
		}
		share->hw_ptr = *slave->pcm->hw.ptr;
	}
	Pthread_mutex_unlock(&slave->mutex);
	avail = snd_pcm_mmap_avail(pcm);
	if ((snd_pcm_uframes_t)avail > pcm->buffer_size)
		return -EPIPE;
	return avail;
}

static int snd_pcm_share_htimestamp(snd_pcm_t *pcm, snd_pcm_uframes_t *avail,
				    snd_htimestamp_t *tstamp)
{
	snd_pcm_share_t *share = pcm->private_data;
	snd_pcm_share_slave_t *slave = share->slave;
	int err;
	Pthread_mutex_lock(&slave->mutex);
	err = snd_pcm_htimestamp(slave->pcm, avail, tstamp);
	Pthread_mutex_unlock(&slave->mutex);
	return err;
}

/* Call it with mutex held */
static snd_pcm_sframes_t _snd_pcm_share_mmap_commit(snd_pcm_t *pcm,
						    snd_pcm_uframes_t offset ATTRIBUTE_UNUSED,
						    snd_pcm_uframes_t size)
{
	snd_pcm_share_t *share = pcm->private_data;
	snd_pcm_share_slave_t *slave = share->slave;
	snd_pcm_t *spcm = slave->pcm;
	snd_pcm_sframes_t ret;
	snd_pcm_sframes_t frames;
	if (pcm->stream == SND_PCM_STREAM_PLAYBACK &&
	    share->state == SND_PCM_STATE_RUNNING) {
		frames = *spcm->appl.ptr - share->appl_ptr;
		if (frames > (snd_pcm_sframes_t)pcm->buffer_size)
			frames -= pcm->boundary;
		else if (frames < -(snd_pcm_sframes_t)pcm->buffer_size)
			frames += pcm->boundary;
		if (frames > 0) {
			/* Latecomer PCM */
			ret = snd_pcm_rewind(spcm, frames);
			if (ret < 0)
				return ret;
		}
	}
	snd_pcm_mmap_appl_forward(pcm, size);
	if (share->state == SND_PCM_STATE_RUNNING) {
		frames = _snd_pcm_share_slave_forward(slave);
		if (frames > 0) {
			snd_pcm_sframes_t err;
			err = snd_pcm_mmap_commit(spcm, snd_pcm_mmap_offset(spcm), frames);
			if (err < 0) {
				SYSMSG("snd_pcm_mmap_commit error");
				return err;
			}
			if (err != frames) {
				SYSMSG("commit returns %ld for size %ld", err, frames);
				return err;
			}
		}
		_snd_pcm_share_update(pcm);
	}
	return size;
}

static snd_pcm_sframes_t snd_pcm_share_mmap_commit(snd_pcm_t *pcm,
						   snd_pcm_uframes_t offset,
						   snd_pcm_uframes_t size)
{
	snd_pcm_share_t *share = pcm->private_data;
	snd_pcm_share_slave_t *slave = share->slave;
	snd_pcm_sframes_t ret;
	Pthread_mutex_lock(&slave->mutex);
	ret = _snd_pcm_share_mmap_commit(pcm, offset, size);
	Pthread_mutex_unlock(&slave->mutex);
	return ret;
}

static int snd_pcm_share_prepare(snd_pcm_t *pcm)
{
	snd_pcm_share_t *share = pcm->private_data;
	snd_pcm_share_slave_t *slave = share->slave;
	int err = 0;
	Pthread_mutex_lock(&slave->mutex);
	switch (share->state) {
	case SND_PCM_STATE_OPEN:
		err = -EBADFD;
		goto _end;
	case SND_PCM_STATE_RUNNING:
		err = -EBUSY;
		goto _end;
	case SND_PCM_STATE_PREPARED:
		err = 0;
		goto _end;
	default:	/* nothing todo */
		break;
	}
	if (slave->prepared_count == 0) {
		err = snd_pcm_prepare(slave->pcm);
		if (err < 0)
			goto _end;
	}
	slave->prepared_count++;
	share->hw_ptr = 0;
	share->appl_ptr = 0;
	share->state = SND_PCM_STATE_PREPARED;
 _end:
	Pthread_mutex_unlock(&slave->mutex);
	return err;
}

static int snd_pcm_share_reset(snd_pcm_t *pcm)
{
	snd_pcm_share_t *share = pcm->private_data;
	snd_pcm_share_slave_t *slave = share->slave;
	int err = 0;
	Pthread_mutex_lock(&slave->mutex);
	snd_pcm_areas_silence(pcm->running_areas, 0, pcm->channels, pcm->buffer_size, pcm->format);
	share->hw_ptr = *slave->pcm->hw.ptr;
	share->appl_ptr = share->hw_ptr;
	Pthread_mutex_unlock(&slave->mutex);
	return err;
}

static int snd_pcm_share_start(snd_pcm_t *pcm)
{
	snd_pcm_share_t *share = pcm->private_data;
	snd_pcm_share_slave_t *slave = share->slave;
	snd_pcm_t *spcm = slave->pcm;
	int err = 0;
	if (share->state != SND_PCM_STATE_PREPARED)
		return -EBADFD;
	Pthread_mutex_lock(&slave->mutex);
	share->state = SND_PCM_STATE_RUNNING;
	if (pcm->stream == SND_PCM_STREAM_PLAYBACK) {
		snd_pcm_uframes_t hw_avail = snd_pcm_mmap_playback_hw_avail(pcm);
		snd_pcm_uframes_t xfer = 0;
		if (hw_avail == 0) {
			err = -EPIPE;
			goto _end;
		}
		if (slave->running_count) {
			snd_pcm_sframes_t sd;
			err = snd_pcm_delay(spcm, &sd);
			if (err < 0)
				goto _end;
			err = snd_pcm_rewind(spcm, sd);
			if (err < 0)
				goto _end;
		}
		assert(share->hw_ptr == 0);
		share->hw_ptr = *spcm->hw.ptr;
		share->appl_ptr = *spcm->appl.ptr;
		while (xfer < hw_avail) {
			snd_pcm_uframes_t frames = hw_avail - xfer;
			snd_pcm_uframes_t offset = snd_pcm_mmap_offset(pcm);
			snd_pcm_uframes_t cont = pcm->buffer_size - offset;
			if (cont < frames)
				frames = cont;
			if (pcm->stopped_areas != NULL)
				snd_pcm_areas_copy(pcm->running_areas, offset,
						   pcm->stopped_areas, xfer,
						   pcm->channels, frames,
						   pcm->format);
			xfer += frames;
		}
		snd_pcm_mmap_appl_forward(pcm, hw_avail);
		if (slave->running_count == 0) {
			snd_pcm_sframes_t res;
			res = snd_pcm_mmap_commit(spcm, snd_pcm_mmap_offset(spcm), hw_avail);
			if (res < 0) {
				err = res;
				goto _end;
			}
			assert((snd_pcm_uframes_t)res == hw_avail);
		}
	}
	if (slave->running_count == 0) {
		err = snd_pcm_start(spcm);
		if (err < 0)
			goto _end;
	}
	slave->running_count++;
	_snd_pcm_share_update(pcm);
	gettimestamp(&share->trigger_tstamp, pcm->monotonic);
 _end:
	Pthread_mutex_unlock(&slave->mutex);
	return err;
}

static int snd_pcm_share_pause(snd_pcm_t *pcm ATTRIBUTE_UNUSED, int enable ATTRIBUTE_UNUSED)
{
	return -ENOSYS;
}

static int snd_pcm_share_resume(snd_pcm_t *pcm ATTRIBUTE_UNUSED)
{
	return -ENXIO;
}

static int snd_pcm_share_channel_info(snd_pcm_t *pcm, snd_pcm_channel_info_t *info)
{
	snd_pcm_share_t *share = pcm->private_data;
	snd_pcm_share_slave_t *slave = share->slave;
	unsigned int channel = info->channel;
	int c = share->slave_channels[channel];
	int err;
	info->channel = c;
	err = snd_pcm_channel_info(slave->pcm, info);
	info->channel = channel;
	return err;
}

static snd_pcm_sframes_t _snd_pcm_share_rewind(snd_pcm_t *pcm, snd_pcm_uframes_t frames)
{
	snd_pcm_share_t *share = pcm->private_data;
	snd_pcm_share_slave_t *slave = share->slave;
	snd_pcm_sframes_t n;
	switch (share->state) {
	case SND_PCM_STATE_RUNNING:
		break;
	case SND_PCM_STATE_PREPARED:
		if (pcm->stream != SND_PCM_STREAM_PLAYBACK)
			return -EBADFD;
		break;
	case SND_PCM_STATE_DRAINING:
		if (pcm->stream != SND_PCM_STREAM_CAPTURE)
			return -EBADFD;
		break;
	case SND_PCM_STATE_XRUN:
		return -EPIPE;
	default:
		return -EBADFD;
	}
	n = snd_pcm_mmap_hw_avail(pcm);
	assert(n >= 0);
	if ((snd_pcm_uframes_t)n > frames)
		frames = n;
	if (share->state == SND_PCM_STATE_RUNNING && frames > 0) {
		snd_pcm_sframes_t ret = snd_pcm_rewind(slave->pcm, frames);
		if (ret < 0)
			return ret;
		frames = ret;
	}
	snd_pcm_mmap_appl_backward(pcm, frames);
	_snd_pcm_share_update(pcm);
	return n;
}

static snd_pcm_sframes_t snd_pcm_share_rewindable(snd_pcm_t *pcm)
{
	snd_pcm_share_t *share = pcm->private_data;
	snd_pcm_share_slave_t *slave = share->slave;
	snd_pcm_sframes_t ret;
	Pthread_mutex_lock(&slave->mutex);
	ret = snd_pcm_rewindable(slave->pcm);
	Pthread_mutex_unlock(&slave->mutex);
	return ret;
}

static snd_pcm_sframes_t snd_pcm_share_rewind(snd_pcm_t *pcm, snd_pcm_uframes_t frames)
{
	snd_pcm_share_t *share = pcm->private_data;
	snd_pcm_share_slave_t *slave = share->slave;
	snd_pcm_sframes_t ret;
	Pthread_mutex_lock(&slave->mutex);
	ret = _snd_pcm_share_rewind(pcm, frames);
	Pthread_mutex_unlock(&slave->mutex);
	return ret;
}

static snd_pcm_sframes_t _snd_pcm_share_forward(snd_pcm_t *pcm, snd_pcm_uframes_t frames)
{
	snd_pcm_share_t *share = pcm->private_data;
	snd_pcm_share_slave_t *slave = share->slave;
	snd_pcm_sframes_t n;
	switch (share->state) {
	case SND_PCM_STATE_RUNNING:
		break;
	case SND_PCM_STATE_PREPARED:
		if (pcm->stream != SND_PCM_STREAM_PLAYBACK)
			return -EBADFD;
		break;
	case SND_PCM_STATE_DRAINING:
		if (pcm->stream != SND_PCM_STREAM_CAPTURE)
			return -EBADFD;
		break;
	case SND_PCM_STATE_XRUN:
		return -EPIPE;
	default:
		return -EBADFD;
	}
	n = snd_pcm_mmap_avail(pcm);
	if ((snd_pcm_uframes_t)n > frames)
		frames = n;
	if (share->state == SND_PCM_STATE_RUNNING && frames > 0) {
		snd_pcm_sframes_t ret = INTERNAL(snd_pcm_forward)(slave->pcm, frames);
		if (ret < 0)
			return ret;
		frames = ret;
	}
	snd_pcm_mmap_appl_forward(pcm, frames);
	_snd_pcm_share_update(pcm);
	return n;
}

static snd_pcm_sframes_t snd_pcm_share_forwardable(snd_pcm_t *pcm)
{
	snd_pcm_share_t *share = pcm->private_data;
	snd_pcm_share_slave_t *slave = share->slave;
	snd_pcm_sframes_t ret;
	Pthread_mutex_lock(&slave->mutex);
	ret = snd_pcm_forwardable(slave->pcm);
	Pthread_mutex_unlock(&slave->mutex);
	return ret;
}

static snd_pcm_sframes_t snd_pcm_share_forward(snd_pcm_t *pcm, snd_pcm_uframes_t frames)
{
	snd_pcm_share_t *share = pcm->private_data;
	snd_pcm_share_slave_t *slave = share->slave;
	snd_pcm_sframes_t ret;
	Pthread_mutex_lock(&slave->mutex);
	ret = _snd_pcm_share_forward(pcm, frames);
	Pthread_mutex_unlock(&slave->mutex);
	return ret;
}

/* Warning: take the mutex before to call this */
static void _snd_pcm_share_stop(snd_pcm_t *pcm, snd_pcm_state_t state)
{
	snd_pcm_share_t *share = pcm->private_data;
	snd_pcm_share_slave_t *slave = share->slave;
	gettimestamp(&share->trigger_tstamp, pcm->monotonic);
	if (pcm->stream == SND_PCM_STREAM_CAPTURE) {
		snd_pcm_areas_copy(pcm->stopped_areas, 0,
				   pcm->running_areas, 0,
				   pcm->channels, pcm->buffer_size,
				   pcm->format);
	} else if (slave->running_count > 1) {
		int err;
		snd_pcm_sframes_t delay;
		snd_pcm_areas_silence(pcm->running_areas, 0, pcm->channels,
				      pcm->buffer_size, pcm->format);
		err = snd_pcm_delay(slave->pcm, &delay);
		if (err >= 0 && delay > 0)
			snd_pcm_rewind(slave->pcm, delay);
		share->drain_silenced = 0;
	}
	share->state = state;
	slave->prepared_count--;
	slave->running_count--;
	if (slave->running_count == 0) {
		int err = snd_pcm_drop(slave->pcm);
		assert(err >= 0);
	}
}

static int snd_pcm_share_drain(snd_pcm_t *pcm)
{
	snd_pcm_share_t *share = pcm->private_data;
	snd_pcm_share_slave_t *slave = share->slave;
	int err = 0;
	Pthread_mutex_lock(&slave->mutex);
	switch (share->state) {
	case SND_PCM_STATE_OPEN:
		err = -EBADFD;
		goto _end;
	case SND_PCM_STATE_PREPARED:
		share->state = SND_PCM_STATE_SETUP;
		goto _end;
	case SND_PCM_STATE_SETUP:
		goto _end;
	default:
		break;
	}
	if (pcm->stream == SND_PCM_STREAM_PLAYBACK) {
		switch (share->state) {
		case SND_PCM_STATE_XRUN:
			share->state = SND_PCM_STATE_SETUP;
			goto _end;
		case SND_PCM_STATE_DRAINING:
		case SND_PCM_STATE_RUNNING:
			share->state = SND_PCM_STATE_DRAINING;
			_snd_pcm_share_update(pcm);
			Pthread_mutex_unlock(&slave->mutex);
			if (!(pcm->mode & SND_PCM_NONBLOCK))
				snd_pcm_wait(pcm, -1);
			return 0;
		default:
			assert(0);
			break;
		}
	} else {
		switch (share->state) {
		case SND_PCM_STATE_RUNNING:
			_snd_pcm_share_stop(pcm, SND_PCM_STATE_DRAINING);
			_snd_pcm_share_update(pcm);
			/* Fall through */
		case SND_PCM_STATE_XRUN:
		case SND_PCM_STATE_DRAINING:
			if (snd_pcm_mmap_capture_avail(pcm) <= 0)
				share->state = SND_PCM_STATE_SETUP;
			else
				share->state = SND_PCM_STATE_DRAINING;
			break;
		default:
			assert(0);
			break;
		}
	}
 _end:
	Pthread_mutex_unlock(&slave->mutex);
	return err;
}

static int snd_pcm_share_drop(snd_pcm_t *pcm)
{
	snd_pcm_share_t *share = pcm->private_data;
	snd_pcm_share_slave_t *slave = share->slave;
	int err = 0;
	Pthread_mutex_lock(&slave->mutex);
	switch (share->state) {
	case SND_PCM_STATE_OPEN:
		err = -EBADFD;
		goto _end;
	case SND_PCM_STATE_SETUP:
		break;
	case SND_PCM_STATE_DRAINING:
		if (pcm->stream == SND_PCM_STREAM_CAPTURE) {
			share->state = SND_PCM_STATE_SETUP;
			break;
		}
		/* Fall through */
	case SND_PCM_STATE_RUNNING:
		_snd_pcm_share_stop(pcm, SND_PCM_STATE_SETUP);
		_snd_pcm_share_update(pcm);
		break;
	case SND_PCM_STATE_PREPARED:
	case SND_PCM_STATE_XRUN:
		share->state = SND_PCM_STATE_SETUP;
		break;
	default:
		assert(0);
		break;
	}
	
	share->appl_ptr = share->hw_ptr = 0;
 _end:
	Pthread_mutex_unlock(&slave->mutex);
	return err;
}

static int snd_pcm_share_close(snd_pcm_t *pcm)
{
	snd_pcm_share_t *share = pcm->private_data;
	snd_pcm_share_slave_t *slave = share->slave;
	int err = 0;

	Pthread_mutex_lock(&snd_pcm_share_slaves_mutex);
	Pthread_mutex_lock(&slave->mutex);
	slave->open_count--;
	if (slave->open_count == 0) {
		pthread_cond_signal(&slave->poll_cond);
		Pthread_mutex_unlock(&slave->mutex);
		err = pthread_join(slave->thread, 0);
		assert(err == 0);
		err = snd_pcm_close(slave->pcm);
		pthread_mutex_destroy(&slave->mutex);
		pthread_cond_destroy(&slave->poll_cond);
		list_del(&slave->list);
		free(slave);
		list_del(&share->list);
	} else {
		list_del(&share->list);
		Pthread_mutex_unlock(&slave->mutex);
	}
	Pthread_mutex_unlock(&snd_pcm_share_slaves_mutex);
	close(share->client_socket);
	close(share->slave_socket);
	free(share->slave_channels);
	free(share);
	return err;
}

static int snd_pcm_share_mmap(snd_pcm_t *pcm ATTRIBUTE_UNUSED)
{
	return 0;
}

static int snd_pcm_share_munmap(snd_pcm_t *pcm ATTRIBUTE_UNUSED)
{
	return 0;
}

static void snd_pcm_share_dump(snd_pcm_t *pcm, snd_output_t *out)
{
	snd_pcm_share_t *share = pcm->private_data;
	snd_pcm_share_slave_t *slave = share->slave;
	unsigned int k;
	snd_output_printf(out, "Share PCM\n");
	snd_output_printf(out, "  Channel bindings:\n");
	for (k = 0; k < share->channels; ++k)
		snd_output_printf(out, "    %d: %d\n", k, share->slave_channels[k]);
	if (pcm->setup) {
		snd_output_printf(out, "Its setup is:\n");
		snd_pcm_dump_setup(pcm, out);
	}
	snd_output_printf(out, "Slave: ");
	snd_pcm_dump(slave->pcm, out);
}

static const snd_pcm_ops_t snd_pcm_share_ops = {
	.close = snd_pcm_share_close,
	.info = snd_pcm_share_info,
	.hw_refine = snd_pcm_share_hw_refine,
	.hw_params = snd_pcm_share_hw_params,
	.hw_free = snd_pcm_share_hw_free,
	.sw_params = snd_pcm_share_sw_params,
	.channel_info = snd_pcm_share_channel_info,
	.dump = snd_pcm_share_dump,
	.nonblock = snd_pcm_share_nonblock,
	.async = snd_pcm_share_async,
	.mmap = snd_pcm_share_mmap,
	.munmap = snd_pcm_share_munmap,
};

static const snd_pcm_fast_ops_t snd_pcm_share_fast_ops = {
	.status = snd_pcm_share_status,
	.state = snd_pcm_share_state,
	.hwsync = snd_pcm_share_hwsync,
	.delay = snd_pcm_share_delay,
	.prepare = snd_pcm_share_prepare,
	.reset = snd_pcm_share_reset,
	.start = snd_pcm_share_start,
	.drop = snd_pcm_share_drop,
	.drain = snd_pcm_share_drain,
	.pause = snd_pcm_share_pause,
	.writei = snd_pcm_mmap_writei,
	.writen = snd_pcm_mmap_writen,
	.readi = snd_pcm_mmap_readi,
	.readn = snd_pcm_mmap_readn,
	.rewindable = snd_pcm_share_rewindable,
	.rewind = snd_pcm_share_rewind,
	.forwardable = snd_pcm_share_forwardable,
	.forward = snd_pcm_share_forward,
	.resume = snd_pcm_share_resume,
	.avail_update = snd_pcm_share_avail_update,
	.htimestamp = snd_pcm_share_htimestamp,
	.mmap_commit = snd_pcm_share_mmap_commit,
};

/**
 * \brief Creates a new Share PCM
 * \param pcmp Returns created PCM handle
 * \param name Name of PCM
 * \param sname Slave name
 * \param sformat Slave format
 * \param srate Slave rate
 * \param schannels Slave channels
 * \param speriod_time Slave period time
 * \param sbuffer_time Slave buffer time
 * \param channels Count of channels
 * \param channels_map Map of channels
 * \param stream Direction
 * \param mode PCM mode
 * \retval zero on success otherwise a negative error code
 * \warning Using of this function might be dangerous in the sense
 *          of compatibility reasons. The prototype might be freely
 *          changed in future.
 */
int snd_pcm_share_open(snd_pcm_t **pcmp, const char *name, const char *sname,
		       snd_pcm_format_t sformat, int srate,
		       unsigned int schannels,
		       int speriod_time, int sbuffer_time,
		       unsigned int channels, unsigned int *channels_map,
		       snd_pcm_stream_t stream, int mode)
{
	snd_pcm_t *pcm;
	snd_pcm_share_t *share;
	int err;
	struct list_head *i;
	char slave_map[32] = { 0 };
	unsigned int k;
	snd_pcm_share_slave_t *slave = NULL;
	int sd[2];

	assert(pcmp);
	assert(channels > 0 && sname && channels_map);

	for (k = 0; k < channels; ++k) {
		if (channels_map[k] >= sizeof(slave_map) / sizeof(slave_map[0])) {
			SNDERR("Invalid slave channel (%d) in binding", channels_map[k]);
			return -EINVAL;
		}
		if (slave_map[channels_map[k]]) {
			SNDERR("Repeated slave channel (%d) in binding", channels_map[k]);
			return -EINVAL;
		}
		slave_map[channels_map[k]] = 1;
		assert((unsigned)channels_map[k] < schannels);
	}

	share = calloc(1, sizeof(snd_pcm_share_t));
	if (!share)
		return -ENOMEM;

	share->channels = channels;
	share->slave_channels = calloc(channels, sizeof(*share->slave_channels));
	if (!share->slave_channels) {
		free(share);
		return -ENOMEM;
	}
	memcpy(share->slave_channels, channels_map, channels * sizeof(*share->slave_channels));

	err = snd_pcm_new(&pcm, SND_PCM_TYPE_SHARE, name, stream, mode);
	if (err < 0) {
		free(share->slave_channels);
		free(share);
		return err;
	}
	err = socketpair(AF_LOCAL, SOCK_STREAM, 0, sd);
	if (err < 0) {
		snd_pcm_free(pcm);
		free(share->slave_channels);
		free(share);
		return -errno;
	}
		
	if (stream == SND_PCM_STREAM_PLAYBACK) {
		int bufsize = 1;
		err = setsockopt(sd[0], SOL_SOCKET, SO_SNDBUF, &bufsize, sizeof(bufsize));
		if (err >= 0) {
			struct pollfd pfd;
			pfd.fd = sd[0];
			pfd.events = POLLOUT;
			while ((err = poll(&pfd, 1, 0)) == 1) {
				char buf[1];
				err = write(sd[0], buf, 1);
				assert(err != 0);
				if (err != 1)
					break;
			}
		}
	}
	if (err < 0) {
		err = -errno;
		close(sd[0]);
		close(sd[1]);
		snd_pcm_free(pcm);
		free(share->slave_channels);
		free(share);
		return err;
	}

	Pthread_mutex_lock(&snd_pcm_share_slaves_mutex);
	list_for_each(i, &snd_pcm_share_slaves) {
		snd_pcm_share_slave_t *s = list_entry(i, snd_pcm_share_slave_t, list);
		if (s->pcm->name && strcmp(s->pcm->name, sname) == 0) {
			slave = s;
			break;
		}
	}
	if (!slave) {
		snd_pcm_t *spcm;
		err = snd_pcm_open(&spcm, sname, stream, mode);
		if (err < 0) {
			Pthread_mutex_unlock(&snd_pcm_share_slaves_mutex);
			close(sd[0]);
			close(sd[1]);
			snd_pcm_free(pcm);
			free(share->slave_channels);
			free(share);
			return err;
		}
		/* there is a memory leak somewhere, but I'm unable to trace it --jk */
		slave = calloc(1, sizeof(snd_pcm_share_slave_t) * 8);
		if (!slave) {
			Pthread_mutex_unlock(&snd_pcm_share_slaves_mutex);
			snd_pcm_close(spcm);
			close(sd[0]);
			close(sd[1]);
			snd_pcm_free(pcm);
			free(share->slave_channels);
			free(share);
			return err;
		}
		INIT_LIST_HEAD(&slave->clients);
		slave->pcm = spcm;
		slave->channels = schannels;
		slave->format = sformat;
		slave->rate = srate;
		slave->period_time = speriod_time;
		slave->buffer_time = sbuffer_time;
		pthread_mutex_init(&slave->mutex, NULL);
		pthread_cond_init(&slave->poll_cond, NULL);
		list_add_tail(&slave->list, &snd_pcm_share_slaves);
		Pthread_mutex_lock(&slave->mutex);
		err = pthread_create(&slave->thread, NULL, snd_pcm_share_thread, slave);
		assert(err == 0);
		Pthread_mutex_unlock(&snd_pcm_share_slaves_mutex);
	} else {
		Pthread_mutex_lock(&slave->mutex);
		Pthread_mutex_unlock(&snd_pcm_share_slaves_mutex);
		list_for_each(i, &slave->clients) {
			snd_pcm_share_t *sh = list_entry(i, snd_pcm_share_t, list);
			for (k = 0; k < sh->channels; ++k) {
				if (slave_map[sh->slave_channels[k]]) {
					SNDERR("Slave channel %d is already in use", sh->slave_channels[k]);
					Pthread_mutex_unlock(&slave->mutex);
					close(sd[0]);
					close(sd[1]);
					snd_pcm_free(pcm);
					free(share->slave_channels);
					free(share);
					return -EBUSY;
				}
			}
		}
	}

	share->slave = slave;
	share->pcm = pcm;
	share->client_socket = sd[0];
	share->slave_socket = sd[1];
	
	pcm->mmap_rw = 1;
	pcm->ops = &snd_pcm_share_ops;
	pcm->fast_ops = &snd_pcm_share_fast_ops;
	pcm->private_data = share;
	pcm->poll_fd = share->client_socket;
	pcm->poll_events = stream == SND_PCM_STREAM_PLAYBACK ? POLLOUT : POLLIN;
	pcm->monotonic = slave->pcm->monotonic;
	snd_pcm_set_hw_ptr(pcm, &share->hw_ptr, -1, 0);
	snd_pcm_set_appl_ptr(pcm, &share->appl_ptr, -1, 0);

	slave->open_count++;
	list_add_tail(&share->list, &slave->clients);

	Pthread_mutex_unlock(&slave->mutex);

	*pcmp = pcm;
	return 0;
}

/*! \page pcm_plugins

\section pcm_plugins_share Plugin: Share

This plugin allows sharing of multiple channels with more clients. The access
to each channel is exlusive (samples are not mixed together). It means, if
the channel zero is used with first client, the channel cannot be used with
second one. If you are looking for a mixing plugin, use the
\ref pcm_plugins_dmix "dmix plugin".

The difference from \ref pcm_plugins_dshare "dshare plugin" is that
share plugin requires the server program "aserver", while dshare plugin
doesn't need the explicit server but access to the shared buffer.

\code
pcm.name {
        type share              # Share PCM
        slave STR               # Slave name
        # or
        slave {                 # Slave definition
                pcm STR         # Slave PCM name
                [format STR]    # Slave format
                [channels INT]  # Slave channels
                [rate INT]      # Slave rate
                [period_time INT] # Slave period time in us
                [buffer_time INT] # Slave buffer time in us
        }
	bindings {
		N INT		# Slave channel INT for client channel N
	}
}
\endcode

\subsection pcm_plugins_share_funcref Function reference

<UL>
  <LI>snd_pcm_share_open()
  <LI>_snd_pcm_share_open()
</UL>

*/

/**
 * \brief Creates a new Share PCM
 * \param pcmp Returns created PCM handle
 * \param name Name of PCM
 * \param root Root configuration node
 * \param conf Configuration node with Share PCM description
 * \param stream Stream type
 * \param mode Stream mode
 * \retval zero on success otherwise a negative error code
 * \warning Using of this function might be dangerous in the sense
 *          of compatibility reasons. The prototype might be freely
 *          changed in future.
 */
int _snd_pcm_share_open(snd_pcm_t **pcmp, const char *name,
			snd_config_t *root, snd_config_t *conf,
			snd_pcm_stream_t stream, int mode)
{
	snd_config_iterator_t i, next;
	const char *sname = NULL;
	snd_config_t *bindings = NULL;
	int err;
	snd_config_t *slave = NULL, *sconf;
	unsigned int *channels_map = NULL;
	unsigned int channels = 0;
	snd_pcm_format_t sformat = SND_PCM_FORMAT_UNKNOWN;
	int schannels = -1;
	int srate = -1;
	int speriod_time= -1, sbuffer_time = -1;
	unsigned int schannel_max = 0;
	
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
		if (strcmp(id, "bindings") == 0) {
			if (snd_config_get_type(n) != SND_CONFIG_TYPE_COMPOUND) {
				SNDERR("Invalid type for %s", id);
				return -EINVAL;
			}
			bindings = n;
			continue;
		}
		SNDERR("Unknown field %s", id);
		return -EINVAL;
	}
	if (!slave) {
		SNDERR("slave is not defined");
		return -EINVAL;
	}
	err = snd_pcm_slave_conf(root, slave, &sconf, 5,
				 SND_PCM_HW_PARAM_FORMAT, 0, &sformat,
				 SND_PCM_HW_PARAM_CHANNELS, 0, &schannels,
				 SND_PCM_HW_PARAM_RATE, 0, &srate,
				 SND_PCM_HW_PARAM_PERIOD_TIME, 0, &speriod_time,
				 SND_PCM_HW_PARAM_BUFFER_TIME, 0, &sbuffer_time);
	if (err < 0)
		return err;

	err = snd_config_get_string(sconf, &sname);
	sname = err >= 0 && sname ? strdup(sname) : NULL;
	snd_config_delete(sconf);
	if (sname == NULL) {
		SNDERR("slave.pcm is not a string");
		return err;
	}

	if (!bindings) {
		SNDERR("bindings is not defined");
		err = -EINVAL;
		goto _free;
	}
	snd_config_for_each(i, next, bindings) {
		long cchannel = -1;
		snd_config_t *n = snd_config_iterator_entry(i);
		const char *id;
		if (snd_config_get_id(n, &id) < 0)
			continue;
		err = safe_strtol(id, &cchannel);
		if (err < 0 || cchannel < 0) {
			SNDERR("Invalid client channel in binding: %s", id);
			err = -EINVAL;
			goto _free;
		}
		if ((unsigned)cchannel >= channels)
			channels = cchannel + 1;
	}
	if (channels == 0) {
		SNDERR("No bindings defined");
		err = -EINVAL;
		goto _free;
	}
	channels_map = calloc(channels, sizeof(*channels_map));
	if (! channels_map) {
		err = -ENOMEM;
		goto _free;
	}

	snd_config_for_each(i, next, bindings) {
		snd_config_t *n = snd_config_iterator_entry(i);
		const char *id;
		long cchannel;
		long schannel = -1;
		if (snd_config_get_id(n, &id) < 0)
			continue;
		cchannel = atoi(id);
		err = snd_config_get_integer(n, &schannel);
		if (err < 0) {
			goto _free;
		}
		assert(schannel >= 0);
		assert(schannels <= 0 || schannel < schannels);
		channels_map[cchannel] = schannel;
		if ((unsigned)schannel > schannel_max)
			schannel_max = schannel;
	}
	if (schannels <= 0)
		schannels = schannel_max + 1;
	err = snd_pcm_share_open(pcmp, name, sname, sformat, srate, 
				 (unsigned int) schannels,
				 speriod_time, sbuffer_time,
				 channels, channels_map, stream, mode);
_free:
	free(channels_map);
	free((char *)sname);
	return err;
}
#ifndef DOC_HIDDEN
SND_DLSYM_BUILD_VERSION(_snd_pcm_share_open, SND_PCM_DLSYM_VERSION);
#endif
