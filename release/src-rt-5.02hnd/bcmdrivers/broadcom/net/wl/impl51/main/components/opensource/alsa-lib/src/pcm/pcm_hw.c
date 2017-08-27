/**
 * \file pcm/pcm_hw.c
 * \ingroup PCM_Plugins
 * \brief PCM HW Plugin Interface
 * \author Abramo Bagnara <abramo@alsa-project.org>
 * \author Jaroslav Kysela <perex@perex.cz>
 * \date 2000-2001
 */
/*
 *  PCM - Hardware
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
#include <stddef.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include "pcm_local.h"
#include "../control/control_local.h"
#include "../timer/timer_local.h"

//#define DEBUG_RW		/* use to debug readi/writei/readn/writen */
//#define DEBUG_MMAP		/* debug mmap_commit */

#ifndef PIC
/* entry for static linking */
const char *_snd_module_pcm_hw = "";
#endif

#ifndef DOC_HIDDEN

#ifndef F_SETSIG
#define F_SETSIG 10
#endif

/*
 *  Compatibility
 */

struct sndrv_pcm_hw_params_old {
	unsigned int flags;
	unsigned int masks[SNDRV_PCM_HW_PARAM_SUBFORMAT -
			   SNDRV_PCM_HW_PARAM_ACCESS + 1];
	struct sndrv_interval intervals[SNDRV_PCM_HW_PARAM_TICK_TIME -
					SNDRV_PCM_HW_PARAM_SAMPLE_BITS + 1];
	unsigned int rmask;
	unsigned int cmask;
	unsigned int info;
	unsigned int msbits;
	unsigned int rate_num;
	unsigned int rate_den;
	sndrv_pcm_uframes_t fifo_size;
	unsigned char reserved[64];
};

#define SND_PCM_IOCTL_HW_REFINE_OLD _IOWR('A', 0x10, struct sndrv_pcm_hw_params_old)
#define SND_PCM_IOCTL_HW_PARAMS_OLD _IOWR('A', 0x11, struct sndrv_pcm_hw_params_old)

static int use_old_hw_params_ioctl(int fd, unsigned int cmd, snd_pcm_hw_params_t *params);
static snd_pcm_sframes_t snd_pcm_hw_avail_update(snd_pcm_t *pcm);
static const snd_pcm_fast_ops_t snd_pcm_hw_fast_ops;
static const snd_pcm_fast_ops_t snd_pcm_hw_fast_ops_timer;

/*
 *
 */

typedef struct {
	int version;
	int fd;
	int card, device, subdevice;
	int sync_ptr_ioctl;
	volatile struct sndrv_pcm_mmap_status * mmap_status;
	struct sndrv_pcm_mmap_control *mmap_control;
	struct sndrv_pcm_sync_ptr *sync_ptr;
	snd_pcm_uframes_t hw_ptr;
	snd_pcm_uframes_t appl_ptr;
	int period_event;
	snd_timer_t *period_timer;
	struct pollfd period_timer_pfd;
	int period_timer_need_poll;
	/* restricted parameters */
	snd_pcm_format_t format;
	int rate;
	int channels;
} snd_pcm_hw_t;

#define SNDRV_FILE_PCM_STREAM_PLAYBACK		ALSA_DEVICE_DIRECTORY "pcmC%iD%ip"
#define SNDRV_FILE_PCM_STREAM_CAPTURE		ALSA_DEVICE_DIRECTORY "pcmC%iD%ic"
#define SNDRV_PCM_VERSION_MAX			SNDRV_PROTOCOL_VERSION(2, 0, 9)

/* update appl_ptr with driver */
#define FAST_PCM_STATE(hw) \
	((enum sndrv_pcm_state) (hw)->mmap_status->state)
#define FAST_PCM_TSTAMP(hw) \
	((hw)->mmap_status->tstamp)

struct timespec snd_pcm_hw_fast_tstamp(snd_pcm_t *pcm)
{
	struct timespec res;
	snd_pcm_hw_t *hw = pcm->private_data;
	res = FAST_PCM_TSTAMP(hw);
	if (SNDRV_PROTOCOL_VERSION(2, 0, 5) > hw->version)
		res.tv_nsec *= 1000L;
	return res;
}
#endif /* DOC_HIDDEN */

static int sync_ptr1(snd_pcm_hw_t *hw, unsigned int flags)
{
	int err;
	hw->sync_ptr->flags = flags;
	err = ioctl((hw)->fd, SNDRV_PCM_IOCTL_SYNC_PTR, (hw)->sync_ptr);
	if (err < 0) {
		err = -errno;
		SYSMSG("SNDRV_PCM_IOCTL_SYNC_PTR failed (%i)", err);
		return err;
	}
	return 0;
}

static inline int sync_ptr(snd_pcm_hw_t *hw, unsigned int flags)
{
	return hw->sync_ptr ? sync_ptr1(hw, flags) : 0;
}

static int snd_pcm_hw_clear_timer_queue(snd_pcm_hw_t *hw)
{
	if (hw->period_timer_need_poll) {
		while (poll(&hw->period_timer_pfd, 1, 0) > 0) {
			snd_timer_tread_t rbuf[4];
			snd_timer_read(hw->period_timer, rbuf, sizeof(rbuf));
		}
	} else {
		snd_timer_tread_t rbuf[4];
		snd_timer_read(hw->period_timer, rbuf, sizeof(rbuf));
	}
	return 0;
}

static int snd_pcm_hw_poll_descriptors_count(snd_pcm_t *pcm ATTRIBUTE_UNUSED)
{
	return 2;
}

static int snd_pcm_hw_poll_descriptors(snd_pcm_t *pcm, struct pollfd *pfds, unsigned int space)
{
	snd_pcm_hw_t *hw = pcm->private_data;

	if (space < 2)
		return -ENOMEM;
	pfds[0].fd = hw->fd;
	pfds[0].events = pcm->poll_events | POLLERR | POLLNVAL;
	pfds[1].fd = hw->period_timer_pfd.fd;
	pfds[1].events = POLLIN | POLLERR | POLLNVAL;
	return 2;
}

static int snd_pcm_hw_poll_revents(snd_pcm_t *pcm, struct pollfd *pfds, unsigned nfds, unsigned short *revents)
{
	snd_pcm_hw_t *hw = pcm->private_data;
	unsigned int events;

	if (nfds != 2 || pfds[0].fd != hw->fd || pfds[1].fd != hw->period_timer_pfd.fd)
		return -EINVAL;
	events = pfds[0].revents;
	if (pfds[1].revents & POLLIN) {
		snd_pcm_hw_clear_timer_queue(hw);
		events |= pcm->poll_events & ~(POLLERR|POLLNVAL);
	}
	*revents = events;
	return 0;
}

static int snd_pcm_hw_nonblock(snd_pcm_t *pcm, int nonblock)
{
	long flags;
	snd_pcm_hw_t *hw = pcm->private_data;
	int fd = hw->fd, err;

	if ((flags = fcntl(fd, F_GETFL)) < 0) {
		err = -errno;
		SYSMSG("F_GETFL failed (%i)", err);
		return err;
	}
	if (nonblock)
		flags |= O_NONBLOCK;
	else
		flags &= ~O_NONBLOCK;
	if (fcntl(fd, F_SETFL, flags) < 0) {
		err = -errno;
		SYSMSG("F_SETFL for O_NONBLOCK failed (%i)", err);
		return err;
	}
	return 0;
}

static int snd_pcm_hw_async(snd_pcm_t *pcm, int sig, pid_t pid)
{
	long flags;
	snd_pcm_hw_t *hw = pcm->private_data;
	int fd = hw->fd, err;

	if ((flags = fcntl(fd, F_GETFL)) < 0) {
		err = -errno;
		SYSMSG("F_GETFL failed (%i)", err);
		return err;
	}
	if (sig >= 0)
		flags |= O_ASYNC;
	else
		flags &= ~O_ASYNC;
	if (fcntl(fd, F_SETFL, flags) < 0) {
		err = -errno;
		SYSMSG("F_SETFL for O_ASYNC failed (%i)", err);
		return err;
	}
	if (sig < 0)
		return 0;
	if (fcntl(fd, F_SETSIG, (long)sig) < 0) {
		err = -errno;
		SYSMSG("F_SETSIG failed (%i)", err);
		return err;
	}
	if (fcntl(fd, F_SETOWN, (long)pid) < 0) {
		err = -errno;
		SYSMSG("F_SETOWN failed (%i)", err);
		return err;
	}
	return 0;
}

static int snd_pcm_hw_info(snd_pcm_t *pcm, snd_pcm_info_t * info)
{
	snd_pcm_hw_t *hw = pcm->private_data;
	int fd = hw->fd, err;
	if (ioctl(fd, SNDRV_PCM_IOCTL_INFO, info) < 0) {
		err = -errno;
		SYSMSG("SNDRV_PCM_IOCTL_INFO failed (%i)", err);
		return err;
	}
	return 0;
}

static inline int hw_refine_call(snd_pcm_hw_t *pcm_hw, snd_pcm_hw_params_t *params)
{
	/* check for new hw_params structure; it's available from 2.0.2 version of PCM API */
	if (SNDRV_PROTOCOL_VERSION(2, 0, 2) <= pcm_hw->version)
		return ioctl(pcm_hw->fd, SNDRV_PCM_IOCTL_HW_REFINE, params);
	return use_old_hw_params_ioctl(pcm_hw->fd, SND_PCM_IOCTL_HW_REFINE_OLD, params);
}

static int snd_pcm_hw_hw_refine(snd_pcm_t *pcm, snd_pcm_hw_params_t *params)
{
	snd_pcm_hw_t *hw = pcm->private_data;
	int err;

	if (hw->format != SND_PCM_FORMAT_UNKNOWN) {
		err = _snd_pcm_hw_params_set_format(params, hw->format);
		if (err < 0)
			return err;
	}
	if (hw->channels > 0) {
		err = _snd_pcm_hw_param_set(params, SND_PCM_HW_PARAM_CHANNELS,
					    hw->channels, 0);
		if (err < 0)
			return err;
	}
	if (hw->rate > 0) {
		err = _snd_pcm_hw_param_set_minmax(params, SND_PCM_HW_PARAM_RATE,
						   hw->rate, 0, hw->rate + 1, -1);
		if (err < 0)
			return err;
	}

	if (hw_refine_call(hw, params) < 0) {
		err = -errno;
		// SYSMSG("SNDRV_PCM_IOCTL_HW_REFINE failed");
		return err;
	}

	if (params->info != ~0U) {
		params->info &= ~0xf0000000;
		params->info |= (pcm->monotonic ? SND_PCM_INFO_MONOTONIC : 0);
	}
	
	return 0;
}

static inline int hw_params_call(snd_pcm_hw_t *pcm_hw, snd_pcm_hw_params_t *params)
{
	/* check for new hw_params structure; it's available from 2.0.2 version of PCM API */
	if (SNDRV_PROTOCOL_VERSION(2, 0, 2) <= pcm_hw->version)
		return ioctl(pcm_hw->fd, SNDRV_PCM_IOCTL_HW_PARAMS, params);
	return use_old_hw_params_ioctl(pcm_hw->fd, SND_PCM_IOCTL_HW_PARAMS_OLD, params);
}

static int snd_pcm_hw_hw_params(snd_pcm_t *pcm, snd_pcm_hw_params_t * params)
{
	snd_pcm_hw_t *hw = pcm->private_data;
	int err;
	if (hw_params_call(hw, params) < 0) {
		err = -errno;
		SYSMSG("SNDRV_PCM_IOCTL_HW_PARAMS failed (%i)", err);
		return err;
	}
	params->info &= ~0xf0000000;
	params->info |= (pcm->monotonic ? SND_PCM_INFO_MONOTONIC : 0);
	err = sync_ptr(hw, 0);
	if (err < 0)
		return err;
	if (pcm->stream == SND_PCM_STREAM_CAPTURE) {
		snd_pcm_set_appl_ptr(pcm, &hw->mmap_control->appl_ptr, hw->fd,
				     SNDRV_PCM_MMAP_OFFSET_CONTROL);
	}
	return 0;
}

static void snd_pcm_hw_close_timer(snd_pcm_hw_t *hw)
{
	if (hw->period_timer) {
		snd_timer_close(hw->period_timer);
		hw->period_timer = NULL;
	}
}

static int snd_pcm_hw_change_timer(snd_pcm_t *pcm, int enable)
{
	snd_pcm_hw_t *hw = pcm->private_data;
	snd_timer_params_t *params;
	unsigned int suspend, resume;
	int err;
	
	if (enable) {
		snd_timer_params_alloca(&params);
		err = snd_timer_hw_open(&hw->period_timer, "hw-pcm-period-event", SND_TIMER_CLASS_PCM, SND_TIMER_SCLASS_NONE, hw->card, hw->device, (hw->subdevice << 1) | (pcm->stream & 1), SND_TIMER_OPEN_NONBLOCK | SND_TIMER_OPEN_TREAD);
		if (err < 0) {
			err = snd_timer_hw_open(&hw->period_timer, "hw-pcm-period-event", SND_TIMER_CLASS_PCM, SND_TIMER_SCLASS_NONE, hw->card, hw->device, (hw->subdevice << 1) | (pcm->stream & 1), SND_TIMER_OPEN_NONBLOCK);
			return err;
		}
		if (snd_timer_poll_descriptors_count(hw->period_timer) != 1) {
			snd_pcm_hw_close_timer(hw);
			return -EINVAL;
		}
		hw->period_timer_pfd.events = POLLIN;
 		hw->period_timer_pfd.revents = 0;
		snd_timer_poll_descriptors(hw->period_timer, &hw->period_timer_pfd, 1);
		hw->period_timer_need_poll = 0;
		suspend = 1<<SND_TIMER_EVENT_MSUSPEND;
		resume = 1<<SND_TIMER_EVENT_MRESUME;
		/*
		 * hacks for older kernel drivers
		 */
		{
			int ver = 0;
			ioctl(hw->period_timer_pfd.fd, SNDRV_TIMER_IOCTL_PVERSION, &ver);
			/* In older versions, check via poll before read() is needed
                         * because of the confliction between TIMER_START and
                         * FIONBIO ioctls.
                         */
			if (ver < SNDRV_PROTOCOL_VERSION(2, 0, 4))
				hw->period_timer_need_poll = 1;
			/*
			 * In older versions, timer uses pause events instead
			 * suspend/resume events.
			 */
			if (ver < SNDRV_PROTOCOL_VERSION(2, 0, 5)) {
				suspend = 1<<SND_TIMER_EVENT_MPAUSE;
				resume = 1<<SND_TIMER_EVENT_MCONTINUE;
			}
		}
		snd_timer_params_set_auto_start(params, 1);
		snd_timer_params_set_ticks(params, 1);
		snd_timer_params_set_filter(params, (1<<SND_TIMER_EVENT_TICK) |
					    suspend | resume);
		err = snd_timer_params(hw->period_timer, params);
		if (err < 0) {
			snd_pcm_hw_close_timer(hw);
			return err;
		}
		err = snd_timer_start(hw->period_timer);
		if (err < 0) {
			snd_pcm_hw_close_timer(hw);
			return err;
		}
		pcm->fast_ops = &snd_pcm_hw_fast_ops_timer;
	} else {
		snd_pcm_hw_close_timer(hw);
		pcm->fast_ops = &snd_pcm_hw_fast_ops;
		hw->period_event = 0;
	}
	return 0;
}

static int snd_pcm_hw_hw_free(snd_pcm_t *pcm)
{
	snd_pcm_hw_t *hw = pcm->private_data;
	int fd = hw->fd, err;
	snd_pcm_hw_change_timer(pcm, 0);
	if (ioctl(fd, SNDRV_PCM_IOCTL_HW_FREE) < 0) {
		err = -errno;
		SYSMSG("SNDRV_PCM_IOCTL_HW_FREE failed (%i)", err);
		return err;
	}
	return 0;
}

static int snd_pcm_hw_sw_params(snd_pcm_t *pcm, snd_pcm_sw_params_t * params)
{
	snd_pcm_hw_t *hw = pcm->private_data;
	int fd = hw->fd, err;
	int old_period_event = params->period_event;
	params->period_event = 0;
	if ((snd_pcm_tstamp_t) params->tstamp_mode == pcm->tstamp_mode &&
	    params->period_step == pcm->period_step &&
	    params->start_threshold == pcm->start_threshold &&
	    params->stop_threshold == pcm->stop_threshold &&
	    params->silence_threshold == pcm->silence_threshold &&
	    params->silence_size == pcm->silence_size &&
	    old_period_event == hw->period_event) {
		hw->mmap_control->avail_min = params->avail_min;
		return sync_ptr(hw, 0);
	}
	if (ioctl(fd, SNDRV_PCM_IOCTL_SW_PARAMS, params) < 0) {
		err = -errno;
		SYSMSG("SNDRV_PCM_IOCTL_SW_PARAMS failed (%i)", err);
		return err;
	}
	params->period_event = old_period_event;
	hw->mmap_control->avail_min = params->avail_min;
	if (hw->period_event != old_period_event) {
		err = snd_pcm_hw_change_timer(pcm, old_period_event);
		if (err < 0)
			return err;
		hw->period_event = old_period_event;
	}
	return 0;
}

static int snd_pcm_hw_channel_info(snd_pcm_t *pcm, snd_pcm_channel_info_t * info)
{
	snd_pcm_hw_t *hw = pcm->private_data;
	struct sndrv_pcm_channel_info i;
	int fd = hw->fd, err;
	i.channel = info->channel;
	if (ioctl(fd, SNDRV_PCM_IOCTL_CHANNEL_INFO, &i) < 0) {
		err = -errno;
		SYSMSG("SNDRV_PCM_IOCTL_CHANNEL_INFO failed (%i)", err);
		return err;
	}
	info->channel = i.channel;
	info->addr = 0;
	info->first = i.first;
	info->step = i.step;
	info->type = SND_PCM_AREA_MMAP;
	info->u.mmap.fd = fd;
	info->u.mmap.offset = i.offset;
	return 0;
}

static int snd_pcm_hw_status(snd_pcm_t *pcm, snd_pcm_status_t * status)
{
	snd_pcm_hw_t *hw = pcm->private_data;
	int fd = hw->fd, err;
	if (ioctl(fd, SNDRV_PCM_IOCTL_STATUS, status) < 0) {
		err = -errno;
		SYSMSG("SNDRV_PCM_IOCTL_STATUS failed (%i)", err);
		return err;
	}
	if (SNDRV_PROTOCOL_VERSION(2, 0, 5) > hw->version) {
		status->tstamp.tv_nsec *= 1000L;
		status->trigger_tstamp.tv_nsec *= 1000L;
	}
	return 0;
}

static snd_pcm_state_t snd_pcm_hw_state(snd_pcm_t *pcm)
{
	snd_pcm_hw_t *hw = pcm->private_data;
	int err = sync_ptr(hw, 0);
	if (err < 0)
		return err;
	return (snd_pcm_state_t) hw->mmap_status->state;
}

static int snd_pcm_hw_delay(snd_pcm_t *pcm, snd_pcm_sframes_t *delayp)
{
	snd_pcm_hw_t *hw = pcm->private_data;
	int fd = hw->fd, err;
	if (ioctl(fd, SNDRV_PCM_IOCTL_DELAY, delayp) < 0) {
		err = -errno;
		SYSMSG("SNDRV_PCM_IOCTL_DELAY failed (%i)", err);
		return err;
	}
	return 0;
}

static int snd_pcm_hw_hwsync(snd_pcm_t *pcm)
{
	snd_pcm_hw_t *hw = pcm->private_data;
	int fd = hw->fd, err;
	if (SNDRV_PROTOCOL_VERSION(2, 0, 3) <= hw->version) {
		if (hw->sync_ptr) {
			err = sync_ptr1(hw, SNDRV_PCM_SYNC_PTR_HWSYNC);
			if (err < 0)
				return err;
		} else {
			if (ioctl(fd, SNDRV_PCM_IOCTL_HWSYNC) < 0) {
				err = -errno;
				SYSMSG("SNDRV_PCM_IOCTL_HWSYNC failed (%i)", err);
				return err;
			}
		}
	} else {
		snd_pcm_sframes_t delay;
		int err = snd_pcm_hw_delay(pcm, &delay);
		if (err < 0) {
			switch (FAST_PCM_STATE(hw)) {
			case SND_PCM_STATE_PREPARED:
			case SND_PCM_STATE_SUSPENDED:
				return 0;
			default:
				return err;
			}
		}
	}
	return 0;
}

static int snd_pcm_hw_prepare(snd_pcm_t *pcm)
{
	snd_pcm_hw_t *hw = pcm->private_data;
	int fd = hw->fd, err;
	if (ioctl(fd, SNDRV_PCM_IOCTL_PREPARE) < 0) {
		err = -errno;
		SYSMSG("SNDRV_PCM_IOCTL_PREPARE failed (%i)", err);
		return err;
	}
	return sync_ptr(hw, SNDRV_PCM_SYNC_PTR_APPL);
}

static int snd_pcm_hw_reset(snd_pcm_t *pcm)
{
	snd_pcm_hw_t *hw = pcm->private_data;
	int fd = hw->fd, err;
	if (ioctl(fd, SNDRV_PCM_IOCTL_RESET) < 0) {
		err = -errno;
		SYSMSG("SNDRV_PCM_IOCTL_RESET failed (%i)", err);
		return err;
	}
	return sync_ptr(hw, SNDRV_PCM_SYNC_PTR_APPL);
}

static int snd_pcm_hw_start(snd_pcm_t *pcm)
{
	snd_pcm_hw_t *hw = pcm->private_data;
	int err;
	sync_ptr(hw, 0);
	if (ioctl(hw->fd, SNDRV_PCM_IOCTL_START) < 0) {
		err = -errno;
		SYSMSG("SNDRV_PCM_IOCTL_START failed (%i)", err);
		return err;
	}
	return 0;
}

static int snd_pcm_hw_drop(snd_pcm_t *pcm)
{
	snd_pcm_hw_t *hw = pcm->private_data;
	int err;
	if (ioctl(hw->fd, SNDRV_PCM_IOCTL_DROP) < 0) {
		err = -errno;
		SYSMSG("SNDRV_PCM_IOCTL_DROP failed (%i)", err);
		return err;
	} else {
	}
	return 0;
}

static int snd_pcm_hw_drain(snd_pcm_t *pcm)
{
	snd_pcm_hw_t *hw = pcm->private_data;
	int err;
	if (ioctl(hw->fd, SNDRV_PCM_IOCTL_DRAIN) < 0) {
		err = -errno;
		SYSMSG("SNDRV_PCM_IOCTL_DRAIN failed (%i)", err);
		return err;
	}
	return 0;
}

static int snd_pcm_hw_pause(snd_pcm_t *pcm, int enable)
{
	snd_pcm_hw_t *hw = pcm->private_data;
	int err;
	if (ioctl(hw->fd, SNDRV_PCM_IOCTL_PAUSE, enable) < 0) {
		err = -errno;
		SYSMSG("SNDRV_PCM_IOCTL_PAUSE failed (%i)", err);
		return err;
	}
	return 0;
}

static snd_pcm_sframes_t snd_pcm_hw_rewindable(snd_pcm_t *pcm)
{
	return snd_pcm_mmap_hw_avail(pcm);
}

static snd_pcm_sframes_t snd_pcm_hw_rewind(snd_pcm_t *pcm, snd_pcm_uframes_t frames)
{
	snd_pcm_hw_t *hw = pcm->private_data;
	int err;
	if (ioctl(hw->fd, SNDRV_PCM_IOCTL_REWIND, &frames) < 0) {
		err = -errno;
		SYSMSG("SNDRV_PCM_IOCTL_REWIND failed (%i)", err);
		return err;
	}
	err = sync_ptr(hw, SNDRV_PCM_SYNC_PTR_APPL);
	if (err < 0)
		return err;
	return frames;
}

static snd_pcm_sframes_t snd_pcm_hw_forwardable(snd_pcm_t *pcm)
{
	return snd_pcm_mmap_avail(pcm);
}

static snd_pcm_sframes_t snd_pcm_hw_forward(snd_pcm_t *pcm, snd_pcm_uframes_t frames)
{
	snd_pcm_hw_t *hw = pcm->private_data;
	int err;
	if (SNDRV_PROTOCOL_VERSION(2, 0, 4) <= hw->version) {
		if (ioctl(hw->fd, SNDRV_PCM_IOCTL_FORWARD, &frames) < 0) {
			err = -errno;
			SYSMSG("SNDRV_PCM_IOCTL_FORWARD failed (%i)", err);
			return err;
		}
		err = sync_ptr(hw, SNDRV_PCM_SYNC_PTR_APPL);
		if (err < 0)
			return err;
		return frames;
	} else {
		snd_pcm_sframes_t avail;

		err = sync_ptr(hw, SNDRV_PCM_SYNC_PTR_HWSYNC);
		if (err < 0)
			return err;
		switch (FAST_PCM_STATE(hw)) {
		case SNDRV_PCM_STATE_RUNNING:
		case SNDRV_PCM_STATE_DRAINING:
		case SNDRV_PCM_STATE_PAUSED:
		case SNDRV_PCM_STATE_PREPARED:
			break;
		case SNDRV_PCM_STATE_XRUN:
			return -EPIPE;
		default:
			return -EBADFD;
		}
		avail = snd_pcm_mmap_avail(pcm);
		if (avail < 0)
			return 0;
		if (frames > (snd_pcm_uframes_t)avail)
			frames = avail;
		snd_pcm_mmap_appl_forward(pcm, frames);
		err = sync_ptr(hw, 0);
		if (err < 0)
			return err;
		return frames;
	}
}

static int snd_pcm_hw_resume(snd_pcm_t *pcm)
{
	snd_pcm_hw_t *hw = pcm->private_data;
	int fd = hw->fd, err;
	if (ioctl(fd, SNDRV_PCM_IOCTL_RESUME) < 0) {
		err = -errno;
		SYSMSG("SNDRV_PCM_IOCTL_RESUME failed (%i)", err);
		return err;
	}
	return 0;
}

static int hw_link(snd_pcm_t *pcm1, snd_pcm_t *pcm2)
{
	snd_pcm_hw_t *hw1 = pcm1->private_data;
	snd_pcm_hw_t *hw2 = pcm2->private_data;
	if (ioctl(hw1->fd, SNDRV_PCM_IOCTL_LINK, hw2->fd) < 0) {
		SYSMSG("SNDRV_PCM_IOCTL_LINK failed (%i)", -errno);
		return -errno;
	}
	return 0;
}

static int snd_pcm_hw_link_slaves(snd_pcm_t *pcm, snd_pcm_t *master)
{
	if (master->type != SND_PCM_TYPE_HW) {
		SYSMSG("Invalid type for SNDRV_PCM_IOCTL_LINK (%i)", master->type);
		return -EINVAL;
	}
	return hw_link(master, pcm);
}

static int snd_pcm_hw_link(snd_pcm_t *pcm1, snd_pcm_t *pcm2)
{
	if (pcm2->type != SND_PCM_TYPE_HW) {
		if (pcm2->fast_ops->link_slaves)
			return pcm2->fast_ops->link_slaves(pcm2, pcm1);
		return -ENOSYS;
	}
	return hw_link(pcm1, pcm2);
 }

static int snd_pcm_hw_unlink(snd_pcm_t *pcm)
{
	snd_pcm_hw_t *hw = pcm->private_data;

	if (ioctl(hw->fd, SNDRV_PCM_IOCTL_UNLINK) < 0) {
		SYSMSG("SNDRV_PCM_IOCTL_UNLINK failed (%i)", -errno);
		return -errno;
	}
	return 0;
}

static snd_pcm_sframes_t snd_pcm_hw_writei(snd_pcm_t *pcm, const void *buffer, snd_pcm_uframes_t size)
{
	int err;
	snd_pcm_hw_t *hw = pcm->private_data;
	int fd = hw->fd;
	struct sndrv_xferi xferi;
	xferi.buf = (char*) buffer;
	xferi.frames = size;
	xferi.result = 0; /* make valgrind happy */
	err = ioctl(fd, SNDRV_PCM_IOCTL_WRITEI_FRAMES, &xferi);
	err = err >= 0 ? sync_ptr(hw, SNDRV_PCM_SYNC_PTR_APPL) : -errno;
#ifdef DEBUG_RW
	fprintf(stderr, "hw_writei: frames = %li, xferi.result = %li, err = %i\n", size, xferi.result, err);
#endif
	if (err < 0)
		return snd_pcm_check_error(pcm, err);
	return xferi.result;
}

static snd_pcm_sframes_t snd_pcm_hw_writen(snd_pcm_t *pcm, void **bufs, snd_pcm_uframes_t size)
{
	int err;
	snd_pcm_hw_t *hw = pcm->private_data;
	int fd = hw->fd;
	struct sndrv_xfern xfern;
	memset(&xfern, 0, sizeof(xfern)); /* make valgrind happy */
	xfern.bufs = bufs;
	xfern.frames = size;
	err = ioctl(fd, SNDRV_PCM_IOCTL_WRITEN_FRAMES, &xfern);
	err = err >= 0 ? sync_ptr(hw, SNDRV_PCM_SYNC_PTR_APPL) : -errno;
#ifdef DEBUG_RW
	fprintf(stderr, "hw_writen: frames = %li, result = %li, err = %i\n", size, xfern.result, err);
#endif
	if (err < 0)
		return snd_pcm_check_error(pcm, err);
	return xfern.result;
}

static snd_pcm_sframes_t snd_pcm_hw_readi(snd_pcm_t *pcm, void *buffer, snd_pcm_uframes_t size)
{
	int err;
	snd_pcm_hw_t *hw = pcm->private_data;
	int fd = hw->fd;
	struct sndrv_xferi xferi;
	xferi.buf = buffer;
	xferi.frames = size;
	xferi.result = 0; /* make valgrind happy */
	err = ioctl(fd, SNDRV_PCM_IOCTL_READI_FRAMES, &xferi);
	err = err >= 0 ? sync_ptr(hw, SNDRV_PCM_SYNC_PTR_APPL) : -errno;
#ifdef DEBUG_RW
	fprintf(stderr, "hw_readi: frames = %li, result = %li, err = %i\n", size, xferi.result, err);
#endif
	if (err < 0)
		return snd_pcm_check_error(pcm, err);
	return xferi.result;
}

static snd_pcm_sframes_t snd_pcm_hw_readn(snd_pcm_t *pcm, void **bufs, snd_pcm_uframes_t size)
{
	int err;
	snd_pcm_hw_t *hw = pcm->private_data;
	int fd = hw->fd;
	struct sndrv_xfern xfern;
	memset(&xfern, 0, sizeof(xfern)); /* make valgrind happy */
	xfern.bufs = bufs;
	xfern.frames = size;
	err = ioctl(fd, SNDRV_PCM_IOCTL_READN_FRAMES, &xfern);
	err = err >= 0 ? sync_ptr(hw, SNDRV_PCM_SYNC_PTR_APPL) : -errno;
#ifdef DEBUG_RW
	fprintf(stderr, "hw_readn: frames = %li, result = %li, err = %i\n", size, xfern.result, err);
#endif
	if (err < 0)
		return snd_pcm_check_error(pcm, err);
	return xfern.result;
}

static int snd_pcm_hw_mmap_status(snd_pcm_t *pcm)
{
	snd_pcm_hw_t *hw = pcm->private_data;
	struct sndrv_pcm_sync_ptr sync_ptr;
	void *ptr;
	int err;
	ptr = MAP_FAILED;
	if (hw->sync_ptr_ioctl == 0)
		ptr = mmap(NULL, page_align(sizeof(struct sndrv_pcm_mmap_status)),
			   PROT_READ, MAP_FILE|MAP_SHARED, 
			   hw->fd, SNDRV_PCM_MMAP_OFFSET_STATUS);
	if (ptr == MAP_FAILED || ptr == NULL) {
		memset(&sync_ptr, 0, sizeof(sync_ptr));
		sync_ptr.c.control.appl_ptr = 0;
		sync_ptr.c.control.avail_min = 1;
		err = ioctl(hw->fd, SNDRV_PCM_IOCTL_SYNC_PTR, &sync_ptr);
		if (err < 0) {
			err = -errno;
			SYSMSG("SNDRV_PCM_IOCTL_SYNC_PTR failed (%i)", err);
			return err;
		}
		hw->sync_ptr = calloc(1, sizeof(struct sndrv_pcm_sync_ptr));
		if (hw->sync_ptr == NULL)
			return -ENOMEM;
		hw->mmap_status = &hw->sync_ptr->s.status;
		hw->mmap_control = &hw->sync_ptr->c.control;
		hw->sync_ptr_ioctl = 1;
	} else {
		hw->mmap_status = ptr;
	}
	snd_pcm_set_hw_ptr(pcm, &hw->mmap_status->hw_ptr, hw->fd, SNDRV_PCM_MMAP_OFFSET_STATUS + offsetof(struct sndrv_pcm_mmap_status, hw_ptr));
	return 0;
}

static int snd_pcm_hw_mmap_control(snd_pcm_t *pcm)
{
	snd_pcm_hw_t *hw = pcm->private_data;
	void *ptr;
	int err;
	if (hw->sync_ptr == NULL) {
		ptr = mmap(NULL, page_align(sizeof(struct sndrv_pcm_mmap_control)),
			   PROT_READ|PROT_WRITE, MAP_FILE|MAP_SHARED, 
			   hw->fd, SNDRV_PCM_MMAP_OFFSET_CONTROL);
		if (ptr == MAP_FAILED || ptr == NULL) {
			err = -errno;
			SYSMSG("control mmap failed (%i)", err);
			return err;
		}
		hw->mmap_control = ptr;
	} else {
		hw->mmap_control->avail_min = 1;
	}
	snd_pcm_set_appl_ptr(pcm, &hw->mmap_control->appl_ptr, hw->fd, SNDRV_PCM_MMAP_OFFSET_CONTROL);
	return 0;
}

static int snd_pcm_hw_munmap_status(snd_pcm_t *pcm)
{
	snd_pcm_hw_t *hw = pcm->private_data;
	int err;
	if (hw->sync_ptr_ioctl) {
		free(hw->sync_ptr);
		hw->sync_ptr = NULL;
	} else {
		if (munmap((void*)hw->mmap_status, page_align(sizeof(*hw->mmap_status))) < 0) {
			err = -errno;
			SYSMSG("status munmap failed (%i)", err);
			return err;
		}
	}
	return 0;
}

static int snd_pcm_hw_munmap_control(snd_pcm_t *pcm)
{
	snd_pcm_hw_t *hw = pcm->private_data;
	int err;
	if (hw->sync_ptr_ioctl) {
		free(hw->sync_ptr);
		hw->sync_ptr = NULL;
	} else {
		if (munmap(hw->mmap_control, page_align(sizeof(*hw->mmap_control))) < 0) {
			err = -errno;
			SYSMSG("control munmap failed (%i)", err);
			return err;
		}
	}
	return 0;
}

static int snd_pcm_hw_mmap(snd_pcm_t *pcm ATTRIBUTE_UNUSED)
{
	return 0;
}

static int snd_pcm_hw_munmap(snd_pcm_t *pcm ATTRIBUTE_UNUSED)
{
	return 0;
}

static int snd_pcm_hw_close(snd_pcm_t *pcm)
{
	snd_pcm_hw_t *hw = pcm->private_data;
	int err = 0;
	if (close(hw->fd)) {
		err = -errno;
		SYSMSG("close failed (%i)\n", err);
	}
	snd_pcm_hw_munmap_status(pcm);
	snd_pcm_hw_munmap_control(pcm);
	free(hw);
	return err;
}

static snd_pcm_sframes_t snd_pcm_hw_mmap_commit(snd_pcm_t *pcm,
						snd_pcm_uframes_t offset ATTRIBUTE_UNUSED,
						snd_pcm_uframes_t size)
{
	snd_pcm_hw_t *hw = pcm->private_data;

	snd_pcm_mmap_appl_forward(pcm, size);
	sync_ptr(hw, 0);
#ifdef DEBUG_MMAP
	fprintf(stderr, "appl_forward: hw_ptr = %li, appl_ptr = %li, size = %li\n", *pcm->hw.ptr, *pcm->appl.ptr, size);
#endif
	return size;
}

static snd_pcm_sframes_t snd_pcm_hw_avail_update(snd_pcm_t *pcm)
{
	snd_pcm_hw_t *hw = pcm->private_data;
	snd_pcm_uframes_t avail;

	sync_ptr(hw, 0);
	avail = snd_pcm_mmap_avail(pcm);
	switch (FAST_PCM_STATE(hw)) {
	case SNDRV_PCM_STATE_RUNNING:
		if (avail >= pcm->stop_threshold) {
			/* SNDRV_PCM_IOCTL_XRUN ioctl has been implemented since PCM kernel API 2.0.1 */
			if (SNDRV_PROTOCOL_VERSION(2, 0, 1) <= hw->version) {
				if (ioctl(hw->fd, SNDRV_PCM_IOCTL_XRUN) < 0)
					return -errno;
			}
			/* everything is ok, state == SND_PCM_STATE_XRUN at the moment */
			return -EPIPE;
		}
		break;
	case SNDRV_PCM_STATE_XRUN:
		return -EPIPE;
	default:
		break;
	}
	return avail;
}

static int snd_pcm_hw_htimestamp(snd_pcm_t *pcm, snd_pcm_uframes_t *avail,
				 snd_htimestamp_t *tstamp)
{
	snd_pcm_sframes_t avail1;
	int ok = 0;

	/* unfortunately, loop is necessary to ensure valid timestamp */
	while (1) {
		avail1 = snd_pcm_hw_avail_update(pcm);
		if (avail1 < 0)
			return avail1;
		if (ok && (snd_pcm_uframes_t)avail1 == *avail)
			break;
		*avail = avail1;
		*tstamp = snd_pcm_hw_fast_tstamp(pcm);
		ok = 1;
	}
	return 0;
}

static void snd_pcm_hw_dump(snd_pcm_t *pcm, snd_output_t *out)
{
	snd_pcm_hw_t *hw = pcm->private_data;
	char *name;
	int err = snd_card_get_name(hw->card, &name);
	if (err < 0) {
		SNDERR("cannot get card name");
		return;
	}
	snd_output_printf(out, "Hardware PCM card %d '%s' device %d subdevice %d\n",
			  hw->card, name, hw->device, hw->subdevice);
	free(name);
	if (pcm->setup) {
		snd_output_printf(out, "Its setup is:\n");
		snd_pcm_dump_setup(pcm, out);
		snd_output_printf(out, "  appl_ptr     : %li\n", hw->mmap_control->appl_ptr);
		snd_output_printf(out, "  hw_ptr       : %li\n", hw->mmap_status->hw_ptr);
	}
}

static const snd_pcm_ops_t snd_pcm_hw_ops = {
	.close = snd_pcm_hw_close,
	.info = snd_pcm_hw_info,
	.hw_refine = snd_pcm_hw_hw_refine,
	.hw_params = snd_pcm_hw_hw_params,
	.hw_free = snd_pcm_hw_hw_free,
	.sw_params = snd_pcm_hw_sw_params,
	.channel_info = snd_pcm_hw_channel_info,
	.dump = snd_pcm_hw_dump,
	.nonblock = snd_pcm_hw_nonblock,
	.async = snd_pcm_hw_async,
	.mmap = snd_pcm_hw_mmap,
	.munmap = snd_pcm_hw_munmap,
};

static const snd_pcm_fast_ops_t snd_pcm_hw_fast_ops = {
	.status = snd_pcm_hw_status,
	.state = snd_pcm_hw_state,
	.hwsync = snd_pcm_hw_hwsync,
	.delay = snd_pcm_hw_delay,
	.prepare = snd_pcm_hw_prepare,
	.reset = snd_pcm_hw_reset,
	.start = snd_pcm_hw_start,
	.drop = snd_pcm_hw_drop,
	.drain = snd_pcm_hw_drain,
	.pause = snd_pcm_hw_pause,
	.rewindable = snd_pcm_hw_rewindable,
	.rewind = snd_pcm_hw_rewind,
	.forwardable = snd_pcm_hw_forwardable,
	.forward = snd_pcm_hw_forward,
	.resume = snd_pcm_hw_resume,
	.link = snd_pcm_hw_link,
	.link_slaves = snd_pcm_hw_link_slaves,
	.unlink = snd_pcm_hw_unlink,
	.writei = snd_pcm_hw_writei,
	.writen = snd_pcm_hw_writen,
	.readi = snd_pcm_hw_readi,
	.readn = snd_pcm_hw_readn,
	.avail_update = snd_pcm_hw_avail_update,
	.mmap_commit = snd_pcm_hw_mmap_commit,
	.htimestamp = snd_pcm_hw_htimestamp,
	.poll_descriptors = NULL,
	.poll_descriptors_count = NULL,
	.poll_revents = NULL,
};

static const snd_pcm_fast_ops_t snd_pcm_hw_fast_ops_timer = {
	.status = snd_pcm_hw_status,
	.state = snd_pcm_hw_state,
	.hwsync = snd_pcm_hw_hwsync,
	.delay = snd_pcm_hw_delay,
	.prepare = snd_pcm_hw_prepare,
	.reset = snd_pcm_hw_reset,
	.start = snd_pcm_hw_start,
	.drop = snd_pcm_hw_drop,
	.drain = snd_pcm_hw_drain,
	.pause = snd_pcm_hw_pause,
	.rewindable = snd_pcm_hw_rewindable,
	.rewind = snd_pcm_hw_rewind,
	.forwardable = snd_pcm_hw_forwardable,
	.forward = snd_pcm_hw_forward,
	.resume = snd_pcm_hw_resume,
	.link = snd_pcm_hw_link,
	.link_slaves = snd_pcm_hw_link_slaves,
	.unlink = snd_pcm_hw_unlink,
	.writei = snd_pcm_hw_writei,
	.writen = snd_pcm_hw_writen,
	.readi = snd_pcm_hw_readi,
	.readn = snd_pcm_hw_readn,
	.avail_update = snd_pcm_hw_avail_update,
	.mmap_commit = snd_pcm_hw_mmap_commit,
	.htimestamp = snd_pcm_hw_htimestamp,
	.poll_descriptors = snd_pcm_hw_poll_descriptors,
	.poll_descriptors_count = snd_pcm_hw_poll_descriptors_count,
	.poll_revents = snd_pcm_hw_poll_revents,
};

/**
 * \brief Creates a new hw PCM
 * \param pcmp Returns created PCM handle
 * \param name Name of PCM
 * \param fd File descriptor
 * \param mmap_emulation Obsoleted parameter
 * \param sync_ptr_ioctl Boolean flag for sync_ptr ioctl
 * \retval zero on success otherwise a negative error code
 * \warning Using of this function might be dangerous in the sense
 *          of compatibility reasons. The prototype might be freely
 *          changed in future.
 */
int snd_pcm_hw_open_fd(snd_pcm_t **pcmp, const char *name,
		       int fd, int mmap_emulation ATTRIBUTE_UNUSED,
		       int sync_ptr_ioctl)
{
	int ver, mode, monotonic = 0;
	long fmode;
	snd_pcm_t *pcm = NULL;
	snd_pcm_hw_t *hw = NULL;
	snd_pcm_info_t info;
	int ret;

	assert(pcmp);

	memset(&info, 0, sizeof(info));
	if (ioctl(fd, SNDRV_PCM_IOCTL_INFO, &info) < 0) {
		ret = -errno;
		SYSMSG("SNDRV_PCM_IOCTL_INFO failed (%i)", ret);
		close(fd);
		return ret;

	}

	if ((fmode = fcntl(fd, F_GETFL)) < 0) {
		ret = -errno;
		close(fd);
		return ret;
	}
	mode = 0;
	if (fmode & O_NONBLOCK)
		mode |= SND_PCM_NONBLOCK;
	if (fmode & O_ASYNC)
		mode |= SND_PCM_ASYNC;

	if (ioctl(fd, SNDRV_PCM_IOCTL_PVERSION, &ver) < 0) {
		ret = -errno;
		SYSMSG("SNDRV_PCM_IOCTL_PVERSION failed (%i)", ret);
		close(fd);
		return ret;
	}
	if (SNDRV_PROTOCOL_INCOMPATIBLE(ver, SNDRV_PCM_VERSION_MAX))
		return -SND_ERROR_INCOMPATIBLE_VERSION;

#if defined(HAVE_CLOCK_GETTIME) && defined(CLOCK_MONOTONIC)
	if (SNDRV_PROTOCOL_VERSION(2, 0, 9) <= ver) {
		struct timespec timespec;
		if (clock_gettime(CLOCK_MONOTONIC, &timespec) == 0) {
			int on = SNDRV_PCM_TSTAMP_TYPE_MONOTONIC;
			if (ioctl(fd, SNDRV_PCM_IOCTL_TTSTAMP, &on) < 0) {
				ret = -errno;
				SNDMSG("TTSTAMP failed\n");
				return ret;
			}
			monotonic = 1;
		}
	} else
#endif
	  if (SNDRV_PROTOCOL_VERSION(2, 0, 5) <= ver) {
		int on = 1;
		if (ioctl(fd, SNDRV_PCM_IOCTL_TSTAMP, &on) < 0) {
			ret = -errno;
			SNDMSG("TSTAMP failed\n");
			return ret;
		}
	}
	
	hw = calloc(1, sizeof(snd_pcm_hw_t));
	if (!hw) {
		close(fd);
		return -ENOMEM;
	}

	hw->version = ver;
	hw->card = info.card;
	hw->device = info.device;
	hw->subdevice = info.subdevice;
	hw->fd = fd;
	hw->sync_ptr_ioctl = sync_ptr_ioctl;
	/* no restriction */
	hw->format = SND_PCM_FORMAT_UNKNOWN;
	hw->rate = 0;
	hw->channels = 0;

	ret = snd_pcm_new(&pcm, SND_PCM_TYPE_HW, name, info.stream, mode);
	if (ret < 0) {
		free(hw);
		close(fd);
		return ret;
	}

	pcm->ops = &snd_pcm_hw_ops;
	pcm->fast_ops = &snd_pcm_hw_fast_ops;
	pcm->private_data = hw;
	pcm->poll_fd = fd;
	pcm->poll_events = info.stream == SND_PCM_STREAM_PLAYBACK ? POLLOUT : POLLIN;
	pcm->monotonic = monotonic;

	ret = snd_pcm_hw_mmap_status(pcm);
	if (ret < 0) {
		snd_pcm_close(pcm);
		return ret;
	}
	ret = snd_pcm_hw_mmap_control(pcm);
	if (ret < 0) {
		snd_pcm_close(pcm);
		return ret;
	}

	*pcmp = pcm;
	return 0;
}

/**
 * \brief Creates a new hw PCM
 * \param pcmp Returns created PCM handle
 * \param name Name of PCM
 * \param card Number of card
 * \param device Number of device
 * \param subdevice Number of subdevice
 * \param stream PCM Stream
 * \param mode PCM Mode
 * \param mmap_emulation Obsoleted parameter
 * \param sync_ptr_ioctl Use SYNC_PTR ioctl rather than mmap for control structures
 * \retval zero on success otherwise a negative error code
 * \warning Using of this function might be dangerous in the sense
 *          of compatibility reasons. The prototype might be freely
 *          changed in future.
 */
int snd_pcm_hw_open(snd_pcm_t **pcmp, const char *name,
		    int card, int device, int subdevice,
		    snd_pcm_stream_t stream, int mode,
		    int mmap_emulation ATTRIBUTE_UNUSED,
		    int sync_ptr_ioctl)
{
	char filename[sizeof(SNDRV_FILE_PCM_STREAM_PLAYBACK) + 20];
	const char *filefmt;
	int ret = 0, fd = -1;
	int attempt = 0;
	snd_pcm_info_t info;
	int fmode;
	snd_ctl_t *ctl;

	assert(pcmp);

	if ((ret = snd_ctl_hw_open(&ctl, NULL, card, 0)) < 0)
		return ret;

	switch (stream) {
	case SND_PCM_STREAM_PLAYBACK:
		filefmt = SNDRV_FILE_PCM_STREAM_PLAYBACK;
		break;
	case SND_PCM_STREAM_CAPTURE:
		filefmt = SNDRV_FILE_PCM_STREAM_CAPTURE;
		break;
	default:
		SNDERR("invalid stream %d", stream);
		return -EINVAL;
	}
	sprintf(filename, filefmt, card, device);

      __again:
      	if (attempt++ > 3) {
		ret = -EBUSY;
		goto _err;
	}
	ret = snd_ctl_pcm_prefer_subdevice(ctl, subdevice);
	if (ret < 0)
		goto _err;
	fmode = O_RDWR;
	if (mode & SND_PCM_NONBLOCK)
		fmode |= O_NONBLOCK;
	if (mode & SND_PCM_ASYNC)
		fmode |= O_ASYNC;
	if (mode & SND_PCM_APPEND)
		fmode |= O_APPEND;
	fd = snd_open_device(filename, fmode);
	if (fd < 0) {
		ret = -errno;
		SYSMSG("open '%s' failed (%i)", filename, ret);
		goto _err;
	}
	if (subdevice >= 0) {
		memset(&info, 0, sizeof(info));
		if (ioctl(fd, SNDRV_PCM_IOCTL_INFO, &info) < 0) {
			ret = -errno;
			SYSMSG("SNDRV_PCM_IOCTL_INFO failed (%i)", ret);
			goto _err;
		}
		if (info.subdevice != (unsigned int) subdevice) {
			close(fd);
			goto __again;
		}
	}
	snd_ctl_close(ctl);
	return snd_pcm_hw_open_fd(pcmp, name, fd, 0, sync_ptr_ioctl);
       _err:
	snd_ctl_close(ctl);
	return ret;
}

/*! \page pcm_plugins

\section pcm_plugins_hw Plugin: hw

This plugin communicates directly with the ALSA kernel driver. It is a raw
communication without any conversions. The emulation of mmap access can be
optionally enabled, but expect worse latency in the case.

The nonblock option specifies whether the device is opened in a non-blocking
manner.  Note that the blocking behavior for read/write access won't be
changed by this option.  This influences only on the blocking behavior at
opening the device.  If you would like to keep the compatibility with the
older ALSA stuff, turn this option off.

\code
pcm.name {
	type hw			# Kernel PCM
	card INT/STR		# Card name (string) or number (integer)
	[device INT]		# Device number (default 0)
	[subdevice INT]		# Subdevice number (default -1: first available)
	[sync_ptr_ioctl BOOL]	# Use SYNC_PTR ioctl rather than the direct mmap access for control structures
	[nonblock BOOL]		# Force non-blocking open mode
	[format STR]		# Restrict only to the given format
	[channels INT]		# Restrict only to the given channels
	[rate INT]		# Restrict only to the given rate
}
\endcode

\subsection pcm_plugins_hw_funcref Function reference

<UL>
  <LI>snd_pcm_hw_open()
  <LI>_snd_pcm_hw_open()
</UL>

*/

/**
 * \brief Creates a new hw PCM
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
int _snd_pcm_hw_open(snd_pcm_t **pcmp, const char *name,
		     snd_config_t *root ATTRIBUTE_UNUSED, snd_config_t *conf,
		     snd_pcm_stream_t stream, int mode)
{
	snd_config_iterator_t i, next;
	long card = -1, device = 0, subdevice = -1;
	const char *str;
	int err, sync_ptr_ioctl = 0;
	int rate = 0, channels = 0;
	snd_pcm_format_t format = SND_PCM_FORMAT_UNKNOWN;
	snd_config_t *n;
	int nonblock = 1; /* non-block per default */
	snd_pcm_hw_t *hw;

	/* look for defaults.pcm.nonblock definition */
	if (snd_config_search(root, "defaults.pcm.nonblock", &n) >= 0) {
		err = snd_config_get_bool(n);
		if (err >= 0)
			nonblock = err;
	}
	snd_config_for_each(i, next, conf) {
		const char *id;
		n = snd_config_iterator_entry(i);
		if (snd_config_get_id(n, &id) < 0)
			continue;
		if (snd_pcm_conf_generic_id(id))
			continue;
		if (strcmp(id, "card") == 0) {
			err = snd_config_get_integer(n, &card);
			if (err < 0) {
				err = snd_config_get_string(n, &str);
				if (err < 0) {
					SNDERR("Invalid type for %s", id);
					return -EINVAL;
				}
				card = snd_card_get_index(str);
				if (card < 0) {
					SNDERR("Invalid value for %s", id);
					return card;
				}
			}
			continue;
		}
		if (strcmp(id, "device") == 0) {
			err = snd_config_get_integer(n, &device);
			if (err < 0) {
				SNDERR("Invalid type for %s", id);
				return err;
			}
			continue;
		}
		if (strcmp(id, "subdevice") == 0) {
			err = snd_config_get_integer(n, &subdevice);
			if (err < 0) {
				SNDERR("Invalid type for %s", id);
				return err;
			}
			continue;
		}
		if (strcmp(id, "sync_ptr_ioctl") == 0) {
			err = snd_config_get_bool(n);
			if (err < 0)
				continue;
			sync_ptr_ioctl = err;
			continue;
		}
		if (strcmp(id, "nonblock") == 0) {
			err = snd_config_get_bool(n);
			if (err < 0)
				continue;
			nonblock = err;
			continue;
		}
		if (strcmp(id, "rate") == 0) {
			long val;
			err = snd_config_get_integer(n, &val);
			if (err < 0) {
				SNDERR("Invalid type for %s", id);
				return err;
			}
			rate = val;
			continue;
		}
		if (strcmp(id, "format") == 0) {
			err = snd_config_get_string(n, &str);
			if (err < 0) {
				SNDERR("invalid type for %s", id);
				return err;
			}
			format = snd_pcm_format_value(str);
			continue;
		}
		if (strcmp(id, "channels") == 0) {
			long val;
			err = snd_config_get_integer(n, &val);
			if (err < 0) {
				SNDERR("Invalid type for %s", id);
				return err;
			}
			channels = val;
			continue;
		}
		SNDERR("Unknown field %s", id);
		return -EINVAL;
	}
	if (card < 0) {
		SNDERR("card is not defined");
		return -EINVAL;
	}
	err = snd_pcm_hw_open(pcmp, name, card, device, subdevice, stream,
			      mode | (nonblock ? SND_PCM_NONBLOCK : 0),
			      0, sync_ptr_ioctl);
	if (err < 0)
		return err;
	if (nonblock && ! (mode & SND_PCM_NONBLOCK)) {
		/* revert to blocking mode for read/write access */
		snd_pcm_hw_nonblock(*pcmp, 0);
		(*pcmp)->mode = mode;
	} else
		/* make sure the SND_PCM_NO_xxx flags don't get lost on the
		 * way */
		(*pcmp)->mode |= mode & (SND_PCM_NO_AUTO_RESAMPLE|
					 SND_PCM_NO_AUTO_CHANNELS|
					 SND_PCM_NO_AUTO_FORMAT|
					 SND_PCM_NO_SOFTVOL);

	hw = (*pcmp)->private_data;
	if (format != SND_PCM_FORMAT_UNKNOWN)
		hw->format = format;
	if (channels > 0)
		hw->channels = channels;
	if (rate > 0)
		hw->rate = rate;

	return 0;
}

#ifndef DOC_HIDDEN
SND_DLSYM_BUILD_VERSION(_snd_pcm_hw_open, SND_PCM_DLSYM_VERSION);
#endif

/*
 *  To be removed helpers, but keep binary compatibility at the time
 */

#ifndef DOC_HIDDEN
#define __OLD_TO_NEW_MASK(x) ((x&7)|((x&0x07fffff8)<<5))
#define __NEW_TO_OLD_MASK(x) ((x&7)|((x&0xffffff00)>>5))
#endif

static void snd_pcm_hw_convert_from_old_params(snd_pcm_hw_params_t *params,
					       struct sndrv_pcm_hw_params_old *oparams)
{
	unsigned int i;

	memset(params, 0, sizeof(*params));
	params->flags = oparams->flags;
	for (i = 0; i < sizeof(oparams->masks) / sizeof(unsigned int); i++)
		params->masks[i].bits[0] = oparams->masks[i];
	memcpy(params->intervals, oparams->intervals, sizeof(oparams->intervals));
	params->rmask = __OLD_TO_NEW_MASK(oparams->rmask);
	params->cmask = __OLD_TO_NEW_MASK(oparams->cmask);
	params->info = oparams->info;
	params->msbits = oparams->msbits;
	params->rate_num = oparams->rate_num;
	params->rate_den = oparams->rate_den;
	params->fifo_size = oparams->fifo_size;
}

static void snd_pcm_hw_convert_to_old_params(struct sndrv_pcm_hw_params_old *oparams,
					     snd_pcm_hw_params_t *params,
					     unsigned int *cmask)
{
	unsigned int i, j;

	memset(oparams, 0, sizeof(*oparams));
	oparams->flags = params->flags;
	for (i = 0; i < sizeof(oparams->masks) / sizeof(unsigned int); i++) {
		oparams->masks[i] = params->masks[i].bits[0];
		for (j = 1; j < sizeof(params->masks[i].bits) / sizeof(unsigned int); j++)
			if (params->masks[i].bits[j]) {
				*cmask |= 1 << i;
				break;
			}
	}
	memcpy(oparams->intervals, params->intervals, sizeof(oparams->intervals));
	oparams->rmask = __NEW_TO_OLD_MASK(params->rmask);
	oparams->cmask = __NEW_TO_OLD_MASK(params->cmask);
	oparams->info = params->info;
	oparams->msbits = params->msbits;
	oparams->rate_num = params->rate_num;
	oparams->rate_den = params->rate_den;
	oparams->fifo_size = params->fifo_size;
}

static int use_old_hw_params_ioctl(int fd, unsigned int cmd, snd_pcm_hw_params_t *params)
{
	struct sndrv_pcm_hw_params_old oparams;
	unsigned int cmask = 0;
	int res;
	
	snd_pcm_hw_convert_to_old_params(&oparams, params, &cmask);
	res = ioctl(fd, cmd, &oparams);
	snd_pcm_hw_convert_from_old_params(params, &oparams);
	params->cmask |= cmask;
	return res;
}
