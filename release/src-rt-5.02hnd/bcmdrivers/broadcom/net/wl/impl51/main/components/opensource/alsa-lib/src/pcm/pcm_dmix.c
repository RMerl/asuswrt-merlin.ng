/**
 * \file pcm/pcm_dmix.c
 * \ingroup PCM_Plugins
 * \brief PCM Direct Stream Mixing (dmix) Plugin Interface
 * \author Jaroslav Kysela <perex@perex.cz>
 * \date 2003
 */
/*
 *  PCM - Direct Stream Mixing
 *  Copyright (c) 2003 by Jaroslav Kysela <perex@perex.cz>
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
#include <ctype.h>
#include <grp.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/mman.h>
#include "pcm_direct.h"

#ifndef PIC
/* entry for static linking */
const char *_snd_module_pcm_dmix = "";
#endif

#ifndef DOC_HIDDEN
/* start is pending - this state happens when rate plugin does a delayed commit */
#define STATE_RUN_PENDING	1024
#endif

/*
 *
 */

static int shm_sum_discard(snd_pcm_direct_t *dmix);

/*
 *  sum ring buffer shared memory area 
 */
static int shm_sum_create_or_connect(snd_pcm_direct_t *dmix)
{
	struct shmid_ds buf;
	int tmpid, err;
	size_t size;

	size = dmix->shmptr->s.channels *
	       dmix->shmptr->s.buffer_size *
	       sizeof(signed int);	
retryshm:
	dmix->u.dmix.shmid_sum = shmget(dmix->ipc_key + 1, size,
					IPC_CREAT | dmix->ipc_perm);
	err = -errno;
	if (dmix->u.dmix.shmid_sum < 0) {
		if (errno == EINVAL)
		if ((tmpid = shmget(dmix->ipc_key + 1, 0, dmix->ipc_perm)) != -1)
		if (!shmctl(tmpid, IPC_STAT, &buf))
	    	if (!buf.shm_nattch) 
		/* no users so destroy the segment */
		if (!shmctl(tmpid, IPC_RMID, NULL))
		    goto retryshm;
		return err;
	}
	if (shmctl(dmix->u.dmix.shmid_sum, IPC_STAT, &buf) < 0) {
		err = -errno;
		shm_sum_discard(dmix);
		return err;
	}
	if (dmix->ipc_gid >= 0) {
		buf.shm_perm.gid = dmix->ipc_gid;
		shmctl(dmix->u.dmix.shmid_sum, IPC_SET, &buf); 
	}
	dmix->u.dmix.sum_buffer = shmat(dmix->u.dmix.shmid_sum, 0, 0);
	if (dmix->u.dmix.sum_buffer == (void *) -1) {
		err = -errno;
		shm_sum_discard(dmix);
		return err;
	}
	mlock(dmix->u.dmix.sum_buffer, size);
	return 0;
}

static int shm_sum_discard(snd_pcm_direct_t *dmix)
{
	struct shmid_ds buf;
	int ret = 0;

	if (dmix->u.dmix.shmid_sum < 0)
		return -EINVAL;
	if (dmix->u.dmix.sum_buffer != (void *) -1 && shmdt(dmix->u.dmix.sum_buffer) < 0)
		return -errno;
	dmix->u.dmix.sum_buffer = (void *) -1;
	if (shmctl(dmix->u.dmix.shmid_sum, IPC_STAT, &buf) < 0)
		return -errno;
	if (buf.shm_nattch == 0) {	/* we're the last user, destroy the segment */
		if (shmctl(dmix->u.dmix.shmid_sum, IPC_RMID, NULL) < 0)
			return -errno;
		ret = 1;
	}
	dmix->u.dmix.shmid_sum = -1;
	return ret;
}

static void dmix_server_free(snd_pcm_direct_t *dmix)
{
	/* remove the memory region */
	shm_sum_create_or_connect(dmix);
	shm_sum_discard(dmix);
}


#include "pcm_dmix_generic.c"
#if defined(__i386__)
#include "pcm_dmix_i386.c"
#elif defined(__x86_64__)
#include "pcm_dmix_x86_64.c"
#else
#ifndef DOC_HIDDEN
#define mix_select_callbacks(x)	generic_mix_select_callbacks(x)
#define dmix_supported_format generic_dmix_supported_format
#endif
#endif

static void mix_areas(snd_pcm_direct_t *dmix,
		      const snd_pcm_channel_area_t *src_areas,
		      const snd_pcm_channel_area_t *dst_areas,
		      snd_pcm_uframes_t src_ofs,
		      snd_pcm_uframes_t dst_ofs,
		      snd_pcm_uframes_t size)
{
	unsigned int src_step, dst_step;
	unsigned int chn, dchn, channels, sample_size;
	mix_areas_t *do_mix_areas;
	
	channels = dmix->channels;
	switch (dmix->shmptr->s.format) {
	case SND_PCM_FORMAT_S16_LE:
	case SND_PCM_FORMAT_S16_BE:
		sample_size = 2;
		do_mix_areas = (mix_areas_t *)dmix->u.dmix.mix_areas_16;
		break;
	case SND_PCM_FORMAT_S32_LE:
	case SND_PCM_FORMAT_S32_BE:
		sample_size = 4;
		do_mix_areas = (mix_areas_t *)dmix->u.dmix.mix_areas_32;
		break;
	case SND_PCM_FORMAT_S24_LE:
		sample_size = 4;
		do_mix_areas = (mix_areas_t *)dmix->u.dmix.mix_areas_24;
		break;
	case SND_PCM_FORMAT_S24_3LE:
		sample_size = 3;
		do_mix_areas = (mix_areas_t *)dmix->u.dmix.mix_areas_24;
		break;
	case SND_PCM_FORMAT_U8:
		sample_size = 1;
		do_mix_areas = (mix_areas_t *)dmix->u.dmix.mix_areas_u8;
		break;
	default:
		return;
	}
	if (dmix->interleaved) {
		/*
		 * process all areas in one loop
		 * it optimizes the memory accesses for this case
		 */
		do_mix_areas(size * channels,
			     (unsigned char *)dst_areas[0].addr + sample_size * dst_ofs * channels,
			     (unsigned char *)src_areas[0].addr + sample_size * src_ofs * channels,
			     dmix->u.dmix.sum_buffer + dst_ofs * channels,
			     sample_size,
			     sample_size,
			     sizeof(signed int));
		return;
	}
	for (chn = 0; chn < channels; chn++) {
		dchn = dmix->bindings ? dmix->bindings[chn] : chn;
		if (dchn >= dmix->shmptr->s.channels)
			continue;
		src_step = src_areas[chn].step / 8;
		dst_step = dst_areas[dchn].step / 8;
		do_mix_areas(size,
			     ((unsigned char *)dst_areas[dchn].addr + dst_areas[dchn].first / 8) + dst_ofs * dst_step,
			     ((unsigned char *)src_areas[chn].addr + src_areas[chn].first / 8) + src_ofs * src_step,
			     dmix->u.dmix.sum_buffer + channels * dst_ofs + chn,
			     dst_step,
			     src_step,
			     channels * sizeof(signed int));
	}
}

static void remix_areas(snd_pcm_direct_t *dmix,
			const snd_pcm_channel_area_t *src_areas,
			const snd_pcm_channel_area_t *dst_areas,
			snd_pcm_uframes_t src_ofs,
			snd_pcm_uframes_t dst_ofs,
			snd_pcm_uframes_t size)
{
	unsigned int src_step, dst_step;
	unsigned int chn, dchn, channels, sample_size;
	mix_areas_t *do_remix_areas;
	
	channels = dmix->channels;
	switch (dmix->shmptr->s.format) {
	case SND_PCM_FORMAT_S16_LE:
	case SND_PCM_FORMAT_S16_BE:
		sample_size = 2;
		do_remix_areas = (mix_areas_t *)dmix->u.dmix.remix_areas_16;
		break;
	case SND_PCM_FORMAT_S32_LE:
	case SND_PCM_FORMAT_S32_BE:
		sample_size = 4;
		do_remix_areas = (mix_areas_t *)dmix->u.dmix.remix_areas_32;
		break;
	case SND_PCM_FORMAT_S24_LE:
		sample_size = 4;
		do_remix_areas = (mix_areas_t *)dmix->u.dmix.remix_areas_24;
		break;
	case SND_PCM_FORMAT_S24_3LE:
		sample_size = 3;
		do_remix_areas = (mix_areas_t *)dmix->u.dmix.remix_areas_24;
		break;
	case SND_PCM_FORMAT_U8:
		sample_size = 1;
		do_remix_areas = (mix_areas_t *)dmix->u.dmix.remix_areas_u8;
		break;
	default:
		return;
	}
	if (dmix->interleaved) {
		/*
		 * process all areas in one loop
		 * it optimizes the memory accesses for this case
		 */
		do_remix_areas(size * channels,
			       (unsigned char *)dst_areas[0].addr + sample_size * dst_ofs * channels,
			       (unsigned char *)src_areas[0].addr + sample_size * src_ofs * channels,
			       dmix->u.dmix.sum_buffer + dst_ofs * channels,
			       sample_size,
			       sample_size,
			       sizeof(signed int));
		return;
	}
	for (chn = 0; chn < channels; chn++) {
		dchn = dmix->bindings ? dmix->bindings[chn] : chn;
		if (dchn >= dmix->shmptr->s.channels)
			continue;
		src_step = src_areas[chn].step / 8;
		dst_step = dst_areas[dchn].step / 8;
		do_remix_areas(size,
			       ((unsigned char *)dst_areas[dchn].addr + dst_areas[dchn].first / 8) + dst_ofs * dst_step,
			       ((unsigned char *)src_areas[chn].addr + src_areas[chn].first / 8) + src_ofs * src_step,
			       dmix->u.dmix.sum_buffer + channels * dst_ofs + chn,
			       dst_step,
			       src_step,
			       channels * sizeof(signed int));
	}
}

/*
 * if no concurrent access is allowed in the mixing routines, we need to protect
 * the area via semaphore
 */
#ifndef DOC_HIDDEN
#ifdef NO_CONCURRENT_ACCESS
#define dmix_down_sem(dmix) snd_pcm_direct_semaphore_down(dmix, DIRECT_IPC_SEM_CLIENT)
#define dmix_up_sem(dmix) snd_pcm_direct_semaphore_up(dmix, DIRECT_IPC_SEM_CLIENT)
#else
#define dmix_down_sem(dmix)
#define dmix_up_sem(dmix)
#endif
#endif

/*
 *  synchronize shm ring buffer with hardware
 */
static void snd_pcm_dmix_sync_area(snd_pcm_t *pcm)
{
	snd_pcm_direct_t *dmix = pcm->private_data;
	snd_pcm_uframes_t slave_hw_ptr, slave_appl_ptr, slave_size;
	snd_pcm_uframes_t appl_ptr, size, transfer;
	const snd_pcm_channel_area_t *src_areas, *dst_areas;
	
	/* calculate the size to transfer */
	/* check the available size in the local buffer
	 * last_appl_ptr keeps the last updated position
	 */
	size = dmix->appl_ptr - dmix->last_appl_ptr;
	if (! size)
		return;
	if (size >= pcm->boundary / 2)
		size = pcm->boundary - size;

	/* the slave_app_ptr can be far behind the slave_hw_ptr */
	/* reduce mixing and errors here - just skip not catched writes */
	if (dmix->slave_hw_ptr <= dmix->slave_appl_ptr)
		slave_size = dmix->slave_appl_ptr - dmix->slave_hw_ptr;
	else
		slave_size = dmix->slave_appl_ptr + (dmix->slave_boundary - dmix->slave_hw_ptr);
	if (slave_size > dmix->slave_buffer_size) {
		transfer = dmix->slave_buffer_size - slave_size;
		if (transfer > size)
			transfer = size;
		dmix->last_appl_ptr += transfer;
		dmix->last_appl_ptr %= pcm->boundary;
		dmix->slave_appl_ptr += transfer;
		dmix->slave_appl_ptr %= dmix->slave_boundary;
		size = dmix->appl_ptr - dmix->last_appl_ptr;
		if (! size)
			return;
		if (size >= pcm->boundary / 2)
			size = pcm->boundary - size;
	}

	/* check the available size in the slave PCM buffer */
	slave_hw_ptr = dmix->slave_hw_ptr;
	/* don't write on the last active period - this area may be cleared
	 * by the driver during mix operation...
	 */
	slave_hw_ptr -= slave_hw_ptr % dmix->slave_period_size;
	slave_hw_ptr += dmix->slave_buffer_size;
	if (slave_hw_ptr >= dmix->slave_boundary)
		slave_hw_ptr -= dmix->slave_boundary;
	if (slave_hw_ptr < dmix->slave_appl_ptr)
		slave_size = slave_hw_ptr + (dmix->slave_boundary - dmix->slave_appl_ptr);
	else
		slave_size = slave_hw_ptr - dmix->slave_appl_ptr;
	if (slave_size < size)
		size = slave_size;
	if (! size)
		return;

	/* add sample areas here */
	src_areas = snd_pcm_mmap_areas(pcm);
	dst_areas = snd_pcm_mmap_areas(dmix->spcm);
	appl_ptr = dmix->last_appl_ptr % pcm->buffer_size;
	dmix->last_appl_ptr += size;
	dmix->last_appl_ptr %= pcm->boundary;
	slave_appl_ptr = dmix->slave_appl_ptr % dmix->slave_buffer_size;
	dmix->slave_appl_ptr += size;
	dmix->slave_appl_ptr %= dmix->slave_boundary;
	dmix_down_sem(dmix);
	for (;;) {
		transfer = size;
		if (appl_ptr + transfer > pcm->buffer_size)
			transfer = pcm->buffer_size - appl_ptr;
		if (slave_appl_ptr + transfer > dmix->slave_buffer_size)
			transfer = dmix->slave_buffer_size - slave_appl_ptr;
		mix_areas(dmix, src_areas, dst_areas, appl_ptr, slave_appl_ptr, transfer);
		size -= transfer;
		if (! size)
			break;
		slave_appl_ptr += transfer;
		slave_appl_ptr %= dmix->slave_buffer_size;
		appl_ptr += transfer;
		appl_ptr %= pcm->buffer_size;
	}
	dmix_up_sem(dmix);
}

/*
 *  synchronize hardware pointer (hw_ptr) with ours
 */
static int snd_pcm_dmix_sync_ptr(snd_pcm_t *pcm)
{
	snd_pcm_direct_t *dmix = pcm->private_data;
	snd_pcm_uframes_t slave_hw_ptr, old_slave_hw_ptr, avail;
	snd_pcm_sframes_t diff;
	
	switch (snd_pcm_state(dmix->spcm)) {
	case SND_PCM_STATE_DISCONNECTED:
		dmix->state = SND_PCM_STATE_DISCONNECTED;
		return -ENODEV;
	default:
		break;
	}
	if (dmix->slowptr)
		snd_pcm_hwsync(dmix->spcm);
	old_slave_hw_ptr = dmix->slave_hw_ptr;
	slave_hw_ptr = dmix->slave_hw_ptr = *dmix->spcm->hw.ptr;
	diff = slave_hw_ptr - old_slave_hw_ptr;
	if (diff == 0)		/* fast path */
		return 0;
	if (dmix->state != SND_PCM_STATE_RUNNING &&
	    dmix->state != SND_PCM_STATE_DRAINING)
		/* not really started yet - don't update hw_ptr */
		return 0;
	if (diff < 0) {
		slave_hw_ptr += dmix->slave_boundary;
		diff = slave_hw_ptr - old_slave_hw_ptr;
	}
	dmix->hw_ptr += diff;
	dmix->hw_ptr %= pcm->boundary;
	if (pcm->stop_threshold >= pcm->boundary)	/* don't care */
		return 0;
	avail = snd_pcm_mmap_playback_avail(pcm);
	if (avail > dmix->avail_max)
		dmix->avail_max = avail;
	if (avail >= pcm->stop_threshold) {
		snd_timer_stop(dmix->timer);
		gettimestamp(&dmix->trigger_tstamp, pcm->monotonic);
		if (dmix->state == SND_PCM_STATE_RUNNING) {
			dmix->state = SND_PCM_STATE_XRUN;
			return -EPIPE;
		}
		dmix->state = SND_PCM_STATE_SETUP;
		/* clear queue to remove pending poll events */
		snd_pcm_direct_clear_timer_queue(dmix);
	}
	return 0;
}

/*
 *  plugin implementation
 */

static snd_pcm_state_t snd_pcm_dmix_state(snd_pcm_t *pcm)
{
	snd_pcm_direct_t *dmix = pcm->private_data;
	snd_pcm_state_t state;
	state = snd_pcm_state(dmix->spcm);
	switch (state) {
	case SND_PCM_STATE_SUSPENDED:
		return state;
	case SND_PCM_STATE_DISCONNECTED:
		return state;
	default:
		break;
	}
	if (dmix->state == STATE_RUN_PENDING)
		return SNDRV_PCM_STATE_RUNNING;
	return dmix->state;
}

static int snd_pcm_dmix_status(snd_pcm_t *pcm, snd_pcm_status_t * status)
{
	snd_pcm_direct_t *dmix = pcm->private_data;

	switch (dmix->state) {
	case SNDRV_PCM_STATE_DRAINING:
	case SNDRV_PCM_STATE_RUNNING:
		snd_pcm_dmix_sync_ptr(pcm);
		break;
	default:
		break;
	}
	memset(status, 0, sizeof(*status));
	status->state = snd_pcm_dmix_state(pcm);
	status->trigger_tstamp = dmix->trigger_tstamp;
	gettimestamp(&status->tstamp, pcm->monotonic);
	status->avail = snd_pcm_mmap_playback_avail(pcm);
	status->avail_max = status->avail > dmix->avail_max ? status->avail : dmix->avail_max;
	dmix->avail_max = 0;
	return 0;
}

static int snd_pcm_dmix_delay(snd_pcm_t *pcm, snd_pcm_sframes_t *delayp)
{
	snd_pcm_direct_t *dmix = pcm->private_data;
	int err;
	
	switch(dmix->state) {
	case SNDRV_PCM_STATE_DRAINING:
	case SNDRV_PCM_STATE_RUNNING:
		err = snd_pcm_dmix_sync_ptr(pcm);
		if (err < 0)
			return err;
		/* fallthru */
	case SNDRV_PCM_STATE_PREPARED:
	case SNDRV_PCM_STATE_SUSPENDED:
	case STATE_RUN_PENDING:
		*delayp = snd_pcm_mmap_playback_hw_avail(pcm);
		return 0;
	case SNDRV_PCM_STATE_XRUN:
		return -EPIPE;
	case SNDRV_PCM_STATE_DISCONNECTED:
		return -ENODEV;
	default:
		return -EBADFD;
	}
}

static int snd_pcm_dmix_hwsync(snd_pcm_t *pcm)
{
	snd_pcm_direct_t *dmix = pcm->private_data;

	switch(dmix->state) {
	case SNDRV_PCM_STATE_DRAINING:
	case SNDRV_PCM_STATE_RUNNING:
		/* sync slave PCM */
		return snd_pcm_dmix_sync_ptr(pcm);
	case SNDRV_PCM_STATE_PREPARED:
	case SNDRV_PCM_STATE_SUSPENDED:
	case STATE_RUN_PENDING:
		return 0;
	case SNDRV_PCM_STATE_XRUN:
		return -EPIPE;
	case SNDRV_PCM_STATE_DISCONNECTED:
		return -ENODEV;
	default:
		return -EBADFD;
	}
}

static int snd_pcm_dmix_prepare(snd_pcm_t *pcm)
{
	snd_pcm_direct_t *dmix = pcm->private_data;

	snd_pcm_direct_check_interleave(dmix, pcm);
	dmix->state = SND_PCM_STATE_PREPARED;
	dmix->appl_ptr = dmix->last_appl_ptr = 0;
	dmix->hw_ptr = 0;
	return snd_pcm_direct_set_timer_params(dmix);
}

static void reset_slave_ptr(snd_pcm_t *pcm, snd_pcm_direct_t *dmix)
{
	dmix->slave_appl_ptr = dmix->slave_hw_ptr = *dmix->spcm->hw.ptr;
	if (pcm->buffer_size > pcm->period_size * 2)
		return;
	/* If we have too litte periods, better to align the start position
	 * to the period boundary so that the interrupt can be handled properly
	 * at the right time.
	 */
	dmix->slave_appl_ptr = ((dmix->slave_appl_ptr + dmix->slave_period_size - 1)
				/ dmix->slave_period_size) * dmix->slave_period_size;
}

static int snd_pcm_dmix_reset(snd_pcm_t *pcm)
{
	snd_pcm_direct_t *dmix = pcm->private_data;
	dmix->hw_ptr %= pcm->period_size;
	dmix->appl_ptr = dmix->last_appl_ptr = dmix->hw_ptr;
	reset_slave_ptr(pcm, dmix);
	return 0;
}

static int snd_pcm_dmix_start_timer(snd_pcm_t *pcm, snd_pcm_direct_t *dmix)
{
	int err;

	snd_pcm_hwsync(dmix->spcm);
	reset_slave_ptr(pcm, dmix);
	err = snd_timer_start(dmix->timer);
	if (err < 0)
		return err;
	dmix->state = SND_PCM_STATE_RUNNING;
	return 0;
}

static int snd_pcm_dmix_start(snd_pcm_t *pcm)
{
	snd_pcm_direct_t *dmix = pcm->private_data;
	snd_pcm_sframes_t avail;
	int err;
	
	if (dmix->state != SND_PCM_STATE_PREPARED)
		return -EBADFD;
	avail = snd_pcm_mmap_playback_hw_avail(pcm);
	if (avail == 0)
		dmix->state = STATE_RUN_PENDING;
	else if (avail < 0)
		return 0;
	else {
		if ((err = snd_pcm_dmix_start_timer(pcm, dmix)) < 0)
			return err;
		snd_pcm_dmix_sync_area(pcm);
	}
	gettimestamp(&dmix->trigger_tstamp, pcm->monotonic);
	return 0;
}

static int snd_pcm_dmix_drop(snd_pcm_t *pcm)
{
	snd_pcm_direct_t *dmix = pcm->private_data;
	if (dmix->state == SND_PCM_STATE_OPEN)
		return -EBADFD;
	dmix->state = SND_PCM_STATE_SETUP;
	snd_pcm_direct_timer_stop(dmix);
	return 0;
}

static int snd_pcm_dmix_drain(snd_pcm_t *pcm)
{
	snd_pcm_direct_t *dmix = pcm->private_data;
	snd_pcm_uframes_t stop_threshold;
	int err;

	if (dmix->state == SND_PCM_STATE_OPEN)
		return -EBADFD;
	if (pcm->mode & SND_PCM_NONBLOCK)
		return -EAGAIN;
	if (dmix->state == SND_PCM_STATE_PREPARED) {
		if (snd_pcm_mmap_playback_hw_avail(pcm) > 0)
			snd_pcm_dmix_start(pcm);
		else {
			snd_pcm_dmix_drop(pcm);
			return 0;
		}
	}

	if (dmix->state == SND_PCM_STATE_XRUN) {
		snd_pcm_dmix_drop(pcm);
		return 0;
	}

	stop_threshold = pcm->stop_threshold;
	if (pcm->stop_threshold > pcm->buffer_size)
		pcm->stop_threshold = pcm->buffer_size;
	dmix->state = SND_PCM_STATE_DRAINING;
	do {
		err = snd_pcm_dmix_sync_ptr(pcm);
		if (err < 0) {
			snd_pcm_dmix_drop(pcm);
			return err;
		}
		if (dmix->state == SND_PCM_STATE_DRAINING) {
			snd_pcm_dmix_sync_area(pcm);
			snd_pcm_wait_nocheck(pcm, -1);
			snd_pcm_direct_clear_timer_queue(dmix); /* force poll to wait */
		}
	} while (dmix->state == SND_PCM_STATE_DRAINING);
	pcm->stop_threshold = stop_threshold;
	return 0;
}

static int snd_pcm_dmix_pause(snd_pcm_t *pcm ATTRIBUTE_UNUSED, int enable ATTRIBUTE_UNUSED)
{
	return -EIO;
}

static snd_pcm_sframes_t snd_pcm_dmix_rewindable(snd_pcm_t *pcm)
{
	return snd_pcm_mmap_hw_avail(pcm);
}

static snd_pcm_sframes_t snd_pcm_dmix_rewind(snd_pcm_t *pcm, snd_pcm_uframes_t frames)
{
	snd_pcm_direct_t *dmix = pcm->private_data;
	snd_pcm_uframes_t slave_appl_ptr, slave_size;
	snd_pcm_uframes_t appl_ptr, size, transfer, result;
	const snd_pcm_channel_area_t *src_areas, *dst_areas;

	if (dmix->state == SND_PCM_STATE_RUNNING ||
	    dmix->state == SND_PCM_STATE_DRAINING)
	    	return snd_pcm_dmix_hwsync(pcm);

	if (dmix->last_appl_ptr < dmix->appl_ptr)
		size = dmix->appl_ptr - dmix->last_appl_ptr;
	else
		size = dmix->appl_ptr + (pcm->boundary - dmix->last_appl_ptr);
	if (frames < size)
		size = frames;
	snd_pcm_mmap_appl_backward(pcm, size);
	frames -= size;
	if (!frames)
		return size;
	result = size;

	if (dmix->hw_ptr < dmix->appl_ptr)
		size = dmix->appl_ptr - dmix->hw_ptr;
	else
		size = dmix->appl_ptr + (pcm->boundary - dmix->hw_ptr);
	if (size > frames)
		size = frames;
	if (dmix->slave_hw_ptr < dmix->slave_appl_ptr)
		slave_size = dmix->slave_appl_ptr - dmix->slave_hw_ptr;
	else
		slave_size = dmix->slave_appl_ptr + (pcm->boundary - dmix->slave_hw_ptr);
	if (slave_size < size)
		size = slave_size;
	frames -= size;
	result += size;
		
	/* add sample areas here */
	src_areas = snd_pcm_mmap_areas(pcm);
	dst_areas = snd_pcm_mmap_areas(dmix->spcm);
	dmix->last_appl_ptr -= size;
	dmix->last_appl_ptr %= pcm->boundary;
	appl_ptr = dmix->last_appl_ptr % pcm->buffer_size;
	dmix->slave_appl_ptr -= size;
	dmix->slave_appl_ptr %= dmix->slave_boundary;
	slave_appl_ptr = dmix->slave_appl_ptr % dmix->slave_buffer_size;
	dmix_down_sem(dmix);
	for (;;) {
		transfer = size;
		if (appl_ptr + transfer > pcm->buffer_size)
			transfer = pcm->buffer_size - appl_ptr;
		if (slave_appl_ptr + transfer > dmix->slave_buffer_size)
			transfer = dmix->slave_buffer_size - slave_appl_ptr;
		remix_areas(dmix, src_areas, dst_areas, appl_ptr, slave_appl_ptr, transfer);
		size -= transfer;
		if (! size)
			break;
		slave_appl_ptr += transfer;
		slave_appl_ptr %= dmix->slave_buffer_size;
		appl_ptr += transfer;
		appl_ptr %= pcm->buffer_size;
	}
	dmix->last_appl_ptr -= frames;
	dmix->last_appl_ptr %= pcm->boundary;
	dmix->slave_appl_ptr -= frames;
	dmix->slave_appl_ptr %= dmix->slave_boundary;
	dmix_up_sem(dmix);

	snd_pcm_mmap_appl_backward(pcm, frames);

	return result + frames;
}

static snd_pcm_sframes_t snd_pcm_dmix_forwardable(snd_pcm_t *pcm)
{
	return snd_pcm_mmap_avail(pcm);
}

static snd_pcm_sframes_t snd_pcm_dmix_forward(snd_pcm_t *pcm, snd_pcm_uframes_t frames)
{
	snd_pcm_sframes_t avail;

	avail = snd_pcm_mmap_playback_avail(pcm);
	if (avail < 0)
		return 0;
	if (frames > (snd_pcm_uframes_t)avail)
		frames = avail;
	snd_pcm_mmap_appl_forward(pcm, frames);
	return frames;
}

static snd_pcm_sframes_t snd_pcm_dmix_readi(snd_pcm_t *pcm ATTRIBUTE_UNUSED, void *buffer ATTRIBUTE_UNUSED, snd_pcm_uframes_t size ATTRIBUTE_UNUSED)
{
	return -ENODEV;
}

static snd_pcm_sframes_t snd_pcm_dmix_readn(snd_pcm_t *pcm ATTRIBUTE_UNUSED, void **bufs ATTRIBUTE_UNUSED, snd_pcm_uframes_t size ATTRIBUTE_UNUSED)
{
	return -ENODEV;
}

static int snd_pcm_dmix_close(snd_pcm_t *pcm)
{
	snd_pcm_direct_t *dmix = pcm->private_data;

	if (dmix->timer)
		snd_timer_close(dmix->timer);
	snd_pcm_direct_semaphore_down(dmix, DIRECT_IPC_SEM_CLIENT);
	snd_pcm_close(dmix->spcm);
 	if (dmix->server)
 		snd_pcm_direct_server_discard(dmix);
 	if (dmix->client)
 		snd_pcm_direct_client_discard(dmix);
 	shm_sum_discard(dmix);
	if (snd_pcm_direct_shm_discard(dmix)) {
		if (snd_pcm_direct_semaphore_discard(dmix))
			snd_pcm_direct_semaphore_up(dmix, DIRECT_IPC_SEM_CLIENT);
	} else
		snd_pcm_direct_semaphore_up(dmix, DIRECT_IPC_SEM_CLIENT);
	free(dmix->bindings);
	pcm->private_data = NULL;
	free(dmix);
	return 0;
}

static snd_pcm_sframes_t snd_pcm_dmix_mmap_commit(snd_pcm_t *pcm,
						  snd_pcm_uframes_t offset ATTRIBUTE_UNUSED,
						  snd_pcm_uframes_t size)
{
	snd_pcm_direct_t *dmix = pcm->private_data;
	int err;

	switch (snd_pcm_state(dmix->spcm)) {
	case SND_PCM_STATE_XRUN:
		return -EPIPE;
	case SND_PCM_STATE_SUSPENDED:
		return -ESTRPIPE;
	default:
		break;
	}
	if (! size)
		return 0;
	snd_pcm_mmap_appl_forward(pcm, size);
	if (dmix->state == STATE_RUN_PENDING) {
		if ((err = snd_pcm_dmix_start_timer(pcm, dmix)) < 0)
			return err;
	} else if (dmix->state == SND_PCM_STATE_RUNNING ||
		   dmix->state == SND_PCM_STATE_DRAINING)
		snd_pcm_dmix_sync_ptr(pcm);
	if (dmix->state == SND_PCM_STATE_RUNNING ||
	    dmix->state == SND_PCM_STATE_DRAINING) {
		/* ok, we commit the changes after the validation of area */
		/* it's intended, although the result might be crappy */
		snd_pcm_dmix_sync_area(pcm);
		/* clear timer queue to avoid a bogus return from poll */
		if (snd_pcm_mmap_playback_avail(pcm) < pcm->avail_min)
			snd_pcm_direct_clear_timer_queue(dmix);
	}
	return size;
}

static snd_pcm_sframes_t snd_pcm_dmix_avail_update(snd_pcm_t *pcm)
{
	snd_pcm_direct_t *dmix = pcm->private_data;
	
	if (dmix->state == SND_PCM_STATE_RUNNING ||
	    dmix->state == SND_PCM_STATE_DRAINING)
		snd_pcm_dmix_sync_ptr(pcm);
	return snd_pcm_mmap_playback_avail(pcm);
}

static int snd_pcm_dmix_htimestamp(snd_pcm_t *pcm,
				   snd_pcm_uframes_t *avail,
				   snd_htimestamp_t *tstamp)
{
	snd_pcm_direct_t *dmix = pcm->private_data;
	snd_pcm_uframes_t avail1;
	int ok = 0;
	
	while (1) {
		if (dmix->state == SND_PCM_STATE_RUNNING ||
		    dmix->state == SND_PCM_STATE_DRAINING)
			snd_pcm_dmix_sync_ptr(pcm);
		avail1 = snd_pcm_mmap_playback_avail(pcm);
		if (ok && *avail == avail1)
			break;
		*avail = avail1;
		*tstamp = snd_pcm_hw_fast_tstamp(pcm);
	}
	return 0;
}

static int snd_pcm_dmix_poll_revents(snd_pcm_t *pcm, struct pollfd *pfds, unsigned int nfds, unsigned short *revents)
{
	snd_pcm_direct_t *dmix = pcm->private_data;
	if (dmix->state == SND_PCM_STATE_RUNNING)
		snd_pcm_dmix_sync_area(pcm);
	return snd_pcm_direct_poll_revents(pcm, pfds, nfds, revents);
}


static void snd_pcm_dmix_dump(snd_pcm_t *pcm, snd_output_t *out)
{
	snd_pcm_direct_t *dmix = pcm->private_data;

	snd_output_printf(out, "Direct Stream Mixing PCM\n");
	if (pcm->setup) {
		snd_output_printf(out, "Its setup is:\n");
		snd_pcm_dump_setup(pcm, out);
	}
	if (dmix->spcm)
		snd_pcm_dump(dmix->spcm, out);
}

static const snd_pcm_ops_t snd_pcm_dmix_ops = {
	.close = snd_pcm_dmix_close,
	.info = snd_pcm_direct_info,
	.hw_refine = snd_pcm_direct_hw_refine,
	.hw_params = snd_pcm_direct_hw_params,
	.hw_free = snd_pcm_direct_hw_free,
	.sw_params = snd_pcm_direct_sw_params,
	.channel_info = snd_pcm_direct_channel_info,
	.dump = snd_pcm_dmix_dump,
	.nonblock = snd_pcm_direct_nonblock,
	.async = snd_pcm_direct_async,
	.mmap = snd_pcm_direct_mmap,
	.munmap = snd_pcm_direct_munmap,
};

static const snd_pcm_fast_ops_t snd_pcm_dmix_fast_ops = {
	.status = snd_pcm_dmix_status,
	.state = snd_pcm_dmix_state,
	.hwsync = snd_pcm_dmix_hwsync,
	.delay = snd_pcm_dmix_delay,
	.prepare = snd_pcm_dmix_prepare,
	.reset = snd_pcm_dmix_reset,
	.start = snd_pcm_dmix_start,
	.drop = snd_pcm_dmix_drop,
	.drain = snd_pcm_dmix_drain,
	.pause = snd_pcm_dmix_pause,
	.rewindable = snd_pcm_dmix_rewindable,
	.rewind = snd_pcm_dmix_rewind,
	.forwardable = snd_pcm_dmix_forwardable,
	.forward = snd_pcm_dmix_forward,
	.resume = snd_pcm_direct_resume,
	.link = NULL,
	.link_slaves = NULL,
	.unlink = NULL,
	.writei = snd_pcm_mmap_writei,
	.writen = snd_pcm_mmap_writen,
	.readi = snd_pcm_dmix_readi,
	.readn = snd_pcm_dmix_readn,
	.avail_update = snd_pcm_dmix_avail_update,
	.mmap_commit = snd_pcm_dmix_mmap_commit,
	.htimestamp = snd_pcm_dmix_htimestamp,
	.poll_descriptors = NULL,
	.poll_descriptors_count = NULL,
	.poll_revents = snd_pcm_dmix_poll_revents,
};

/**
 * \brief Creates a new dmix PCM
 * \param pcmp Returns created PCM handle
 * \param name Name of PCM
 * \param opts Direct PCM configurations
 * \param params Parameters for slave
 * \param root Configuration root
 * \param sconf Slave configuration
 * \param stream PCM Direction (stream)
 * \param mode PCM Mode
 * \retval zero on success otherwise a negative error code
 * \warning Using of this function might be dangerous in the sense
 *          of compatibility reasons. The prototype might be freely
 *          changed in future.
 */
int snd_pcm_dmix_open(snd_pcm_t **pcmp, const char *name,
		      struct snd_pcm_direct_open_conf *opts,
		      struct slave_params *params,
		      snd_config_t *root, snd_config_t *sconf,
		      snd_pcm_stream_t stream, int mode)
{
	snd_pcm_t *pcm = NULL, *spcm = NULL;
	snd_pcm_direct_t *dmix = NULL;
	int ret, first_instance;
	int fail_sem_loop = 10;

	assert(pcmp);

	if (stream != SND_PCM_STREAM_PLAYBACK) {
		SNDERR("The dmix plugin supports only playback stream");
		return -EINVAL;
	}

	dmix = calloc(1, sizeof(snd_pcm_direct_t));
	if (!dmix) {
		ret = -ENOMEM;
		goto _err_nosem;
	}
	
	ret = snd_pcm_direct_parse_bindings(dmix, params, opts->bindings);
	if (ret < 0)
		goto _err_nosem;
	
	dmix->ipc_key = opts->ipc_key;
	dmix->ipc_perm = opts->ipc_perm;
	dmix->ipc_gid = opts->ipc_gid;
	dmix->semid = -1;
	dmix->shmid = -1;

	ret = snd_pcm_new(&pcm, dmix->type = SND_PCM_TYPE_DMIX, name, stream, mode);
	if (ret < 0)
		goto _err;

	
	while (1) {
		ret = snd_pcm_direct_semaphore_create_or_connect(dmix);
		if (ret < 0) {
			SNDERR("unable to create IPC semaphore");
			goto _err_nosem;
		}
		ret = snd_pcm_direct_semaphore_down(dmix, DIRECT_IPC_SEM_CLIENT);
		if (ret < 0) {
			snd_pcm_direct_semaphore_discard(dmix);
			if (--fail_sem_loop <= 0)
				goto _err_nosem;
			continue;
		}
		break;
	}
		
	first_instance = ret = snd_pcm_direct_shm_create_or_connect(dmix);
	if (ret < 0) {
		SNDERR("unable to create IPC shm instance");
		goto _err;
	}
		
	pcm->ops = &snd_pcm_dmix_ops;
	pcm->fast_ops = &snd_pcm_dmix_fast_ops;
	pcm->private_data = dmix;
	dmix->state = SND_PCM_STATE_OPEN;
	dmix->slowptr = opts->slowptr;
	dmix->max_periods = opts->max_periods;
	dmix->sync_ptr = snd_pcm_dmix_sync_ptr;

	if (first_instance) {
		/* recursion is already checked in
		   snd_pcm_direct_get_slave_ipc_offset() */
		ret = snd_pcm_open_slave(&spcm, root, sconf, stream,
					 mode | SND_PCM_NONBLOCK, NULL);
		if (ret < 0) {
			SNDERR("unable to open slave");
			goto _err;
		}
	
		if (snd_pcm_type(spcm) != SND_PCM_TYPE_HW) {
			SNDERR("dmix plugin can be only connected to hw plugin");
			ret = -EINVAL;
			goto _err;
		}
		
		ret = snd_pcm_direct_initialize_slave(dmix, spcm, params);
		if (ret < 0) {
			SNDERR("unable to initialize slave");
			goto _err;
		}

		dmix->spcm = spcm;

		if (dmix->shmptr->use_server) {
			dmix->server_free = dmix_server_free;
		
			ret = snd_pcm_direct_server_create(dmix);
			if (ret < 0) {
				SNDERR("unable to create server");
				goto _err;
			}
		}

		dmix->shmptr->type = spcm->type;
	} else {
		if (dmix->shmptr->use_server) {
			/* up semaphore to avoid deadlock */
			snd_pcm_direct_semaphore_up(dmix, DIRECT_IPC_SEM_CLIENT);
			ret = snd_pcm_direct_client_connect(dmix);
			if (ret < 0) {
				SNDERR("unable to connect client");
				goto _err_nosem;
			}
			
			snd_pcm_direct_semaphore_down(dmix, DIRECT_IPC_SEM_CLIENT);
			ret = snd_pcm_direct_open_secondary_client(&spcm, dmix, "dmix_client");
			if (ret < 0)
				goto _err;
		} else {

			ret = snd_pcm_open_slave(&spcm, root, sconf, stream,
						 mode | SND_PCM_NONBLOCK |
						 SND_PCM_APPEND,
						 NULL);
			if (ret < 0) {
				SNDERR("unable to open slave");
				goto _err;
			}
			if (snd_pcm_type(spcm) != SND_PCM_TYPE_HW) {
				SNDERR("dmix plugin can be only connected to hw plugin");
				ret = -EINVAL;
				goto _err;
			}
		
			ret = snd_pcm_direct_initialize_secondary_slave(dmix, spcm, params);
			if (ret < 0) {
				SNDERR("unable to initialize slave");
				goto _err;
			}
		}

		dmix->spcm = spcm;
	}

	ret = shm_sum_create_or_connect(dmix);
	if (ret < 0) {
		SNDERR("unable to initialize sum ring buffer");
		goto _err;
	}

	ret = snd_pcm_direct_initialize_poll_fd(dmix);
	if (ret < 0) {
		SNDERR("unable to initialize poll_fd");
		goto _err;
	}

	mix_select_callbacks(dmix);
		
	pcm->poll_fd = dmix->poll_fd;
	pcm->poll_events = POLLIN;	/* it's different than other plugins */
		
	pcm->mmap_rw = 1;
	snd_pcm_set_hw_ptr(pcm, &dmix->hw_ptr, -1, 0);
	snd_pcm_set_appl_ptr(pcm, &dmix->appl_ptr, -1, 0);
	
	if (dmix->channels == UINT_MAX)
		dmix->channels = dmix->shmptr->s.channels;

	snd_pcm_direct_semaphore_up(dmix, DIRECT_IPC_SEM_CLIENT);

	*pcmp = pcm;
	return 0;
	
 _err:
	if (dmix->timer)
		snd_timer_close(dmix->timer);
	if (dmix->server)
		snd_pcm_direct_server_discard(dmix);
	if (dmix->client)
		snd_pcm_direct_client_discard(dmix);
	if (spcm)
		snd_pcm_close(spcm);
	if (dmix->u.dmix.shmid_sum >= 0)
		shm_sum_discard(dmix);
	if (dmix->shmid >= 0)
		snd_pcm_direct_shm_discard(dmix);
	if (snd_pcm_direct_semaphore_discard(dmix) < 0)
		snd_pcm_direct_semaphore_up(dmix, DIRECT_IPC_SEM_CLIENT);
 _err_nosem:
	if (dmix) {
		free(dmix->bindings);
		free(dmix);
	}
	if (pcm)
		snd_pcm_free(pcm);
	return ret;
}

/*! \page pcm_plugins

\section pcm_plugins_dmix Plugin: dmix

This plugin provides direct mixing of multiple streams. The resolution
for 32-bit mixing is only 24-bit. The low significant byte is filled with
zeros. The extra 8 bits are used for the saturation.

\code
pcm.name {
	type dmix		# Direct mix
	ipc_key INT		# unique IPC key
	ipc_key_add_uid BOOL	# add current uid to unique IPC key
	ipc_perm INT		# IPC permissions (octal, default 0600)
	slave STR
	# or
	slave {			# Slave definition
		pcm STR		# slave PCM name
		# or
		pcm { }		# slave PCM definition
		format STR	# format definition
		rate INT	# rate definition
		channels INT
		period_time INT	# in usec
		# or
		period_size INT	# in bytes
		buffer_time INT	# in usec
		# or
		buffer_size INT # in bytes
		periods INT	# when buffer_size or buffer_time is not specified
	}
	bindings {		# note: this is client independent!!!
		N INT		# maps slave channel to client channel N
	}
	slowptr BOOL		# slow but more precise pointer updates
}
\endcode

<code>ipc_key</code> specfies the unique IPC key in integer.
This number must be unique for each different dmix definition,
since the shared memory is created with this key number.
When <code>ipc_key_add_uid</code> is set true, the uid value is
added to the value set in <code>ipc_key</code>.  This will
avoid the confliction of the same IPC key with different users
concurrently.

Note that the dmix plugin itself supports only a single configuration.
That is, it supports only the fixed rate (default 48000), format
(\c S16), channels (2), and period_time (125000).
For using other configuration, you have to set the value explicitly
in the slave PCM definition.  The rate, format and channels can be
covered by an additional \ref pcm_plugins_dmix "plug plugin",
but there is only one base configuration, anyway.

An example configuration for setting 44100 Hz, \c S32_LE format
as the slave PCM of "hw:0" is like below:
\code
pcm.dmix_44 {
	type dmix
	ipc_key 321456	# any unique value
	ipc_key_add_uid true
	slave {
		pcm "hw:0"
		format S32_LE
		rate 44100
	}
}
\endcode
You can hear 48000 Hz samples still using this dmix pcm via plug plugin
like:
\code
% aplay -Dplug:dmix_44 foo_48k.wav
\endcode

For using the dmix plugin for OSS emulation device, you have to set
the period and the buffer sizes in power of two.  For example,
\code
pcm.dmixoss {
	type dmix
	ipc_key 321456	# any unique value
	ipc_key_add_uid true
	slave {
		pcm "hw:0"
		period_time 0
		period_size 1024  # must be power of 2
		buffer_size 8192  # ditto
	}
}
\endcode
<code>period_time 0</code> must be set, too, for resetting the
default value.  In the case of soundcards with multi-channel IO,
adding the bindings would help
\code
pcm.dmixoss {
	...
	bindings {
		0 0   # map from 0 to 0
		1 1   # map from 1 to 1
	}
}
\endcode
so that only the first two channels are used by dmix.
Also, note that ICE1712 have the limited buffer size, 5513 frames
(corresponding to 640 kB).  In this case, reduce the buffer_size
to 4096.

\subsection pcm_plugins_dmix_funcref Function reference

<UL>
  <LI>snd_pcm_dmix_open()
  <LI>_snd_pcm_dmix_open()
</UL>

*/

/**
 * \brief Creates a new dmix PCM
 * \param pcmp Returns created PCM handle
 * \param name Name of PCM
 * \param root Root configuration node
 * \param conf Configuration node with dmix PCM description
 * \param stream PCM Stream
 * \param mode PCM Mode
 * \warning Using of this function might be dangerous in the sense
 *          of compatibility reasons. The prototype might be freely
 *          changed in future.
 */
int _snd_pcm_dmix_open(snd_pcm_t **pcmp, const char *name,
		       snd_config_t *root, snd_config_t *conf,
		       snd_pcm_stream_t stream, int mode)
{
	snd_config_t *sconf;
	struct slave_params params;
	struct snd_pcm_direct_open_conf dopen;
	int bsize, psize;
	int err;

	err = snd_pcm_direct_parse_open_conf(root, conf, stream, &dopen);
	if (err < 0)
		return err;

	/* the default settings, it might be invalid for some hardware */
	params.format = SND_PCM_FORMAT_S16;
	params.rate = 48000;
	params.channels = 2;
	params.period_time = -1;
	params.buffer_time = -1;
	bsize = psize = -1;
	params.periods = 3;

	err = snd_pcm_slave_conf(root, dopen.slave, &sconf, 8,
				 SND_PCM_HW_PARAM_FORMAT, SCONF_UNCHANGED, &params.format,
				 SND_PCM_HW_PARAM_RATE, 0, &params.rate,
				 SND_PCM_HW_PARAM_CHANNELS, 0, &params.channels,
				 SND_PCM_HW_PARAM_PERIOD_TIME, 0, &params.period_time,
				 SND_PCM_HW_PARAM_BUFFER_TIME, 0, &params.buffer_time,
				 SND_PCM_HW_PARAM_PERIOD_SIZE, 0, &psize,
				 SND_PCM_HW_PARAM_BUFFER_SIZE, 0, &bsize,
				 SND_PCM_HW_PARAM_PERIODS, 0, &params.periods);
	if (err < 0)
		return err;

	/* set a reasonable default */  
	if (psize == -1 && params.period_time == -1)
		params.period_time = 125000;    /* 0.125 seconds */

	if (params.format == -2)
		params.format = SND_PCM_FORMAT_UNKNOWN;
	else if (!(dmix_supported_format & (1ULL << params.format))) {
		/* sorry, limited features */
		SNDERR("Unsupported format");
		snd_config_delete(sconf);
		return -EINVAL;
	}

	params.period_size = psize;
	params.buffer_size = bsize;

	err = snd_pcm_dmix_open(pcmp, name, &dopen, &params,
				root, sconf, stream, mode);
	snd_config_delete(sconf);
	return err;
}
#ifndef DOC_HIDDEN
SND_DLSYM_BUILD_VERSION(_snd_pcm_dmix_open, SND_PCM_DLSYM_VERSION);
#endif
