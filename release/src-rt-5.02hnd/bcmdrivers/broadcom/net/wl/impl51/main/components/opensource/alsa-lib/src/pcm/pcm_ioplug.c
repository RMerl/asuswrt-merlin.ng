/**
 * \file pcm/pcm_ioplug.c
 * \ingroup Plugin_SDK
 * \brief I/O Plugin SDK
 * \author Takashi Iwai <tiwai@suse.de>
 * \date 2005
 */
/*
 *  PCM - External I/O Plugin SDK
 *  Copyright (c) 2005 by Takashi Iwai <tiwai@suse.de>
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
#include "pcm_ioplug.h"
#include "pcm_ext_parm.h"
#include "pcm_generic.h"

#ifndef PIC
/* entry for static linking */
const char *_snd_module_pcm_ioplug = "";
#endif

#ifndef DOC_HIDDEN

/* hw_params */
typedef struct snd_pcm_ioplug_priv {
	snd_pcm_ioplug_t *data;
	struct snd_ext_parm params[SND_PCM_IOPLUG_HW_PARAMS];
	unsigned int last_hw;
	snd_pcm_uframes_t avail_max;
	snd_htimestamp_t trigger_tstamp;
} ioplug_priv_t;

/* update the hw pointer */
static void snd_pcm_ioplug_hw_ptr_update(snd_pcm_t *pcm)
{
	ioplug_priv_t *io = pcm->private_data;
	snd_pcm_sframes_t hw;

	hw = io->data->callback->pointer(io->data);
	if (hw >= 0) {
		unsigned int delta;
		if ((unsigned int)hw >= io->last_hw)
			delta = hw - io->last_hw;
		else
			delta = pcm->buffer_size + hw - io->last_hw;
		io->data->hw_ptr += delta;
		io->last_hw = hw;
	} else
		io->data->state = SNDRV_PCM_STATE_XRUN;
}

static int snd_pcm_ioplug_info(snd_pcm_t *pcm, snd_pcm_info_t *info)
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

static int snd_pcm_ioplug_channel_info(snd_pcm_t *pcm, snd_pcm_channel_info_t *info)
{
	return snd_pcm_channel_info_shm(pcm, info, -1);
}

static int snd_pcm_ioplug_status(snd_pcm_t *pcm, snd_pcm_status_t * status)
{
	ioplug_priv_t *io = pcm->private_data;

	memset(status, 0, sizeof(*status));
	snd_pcm_ioplug_hw_ptr_update(pcm);
	status->state = io->data->state;
	status->trigger_tstamp = io->trigger_tstamp;
	status->avail = snd_pcm_mmap_avail(pcm);
	status->avail_max = io->avail_max;
	return 0;
}

static snd_pcm_state_t snd_pcm_ioplug_state(snd_pcm_t *pcm)
{
	ioplug_priv_t *io = pcm->private_data;
	return io->data->state;
}

static int snd_pcm_ioplug_hwsync(snd_pcm_t *pcm)
{
	snd_pcm_ioplug_hw_ptr_update(pcm);
	return 0;
}

static int snd_pcm_ioplug_delay(snd_pcm_t *pcm, snd_pcm_sframes_t *delayp)
{
	ioplug_priv_t *io = pcm->private_data;

	if (io->data->version >= 0x010001 &&
	    io->data->callback->delay)
		return io->data->callback->delay(io->data, delayp);
	else {
		snd_pcm_ioplug_hw_ptr_update(pcm);
		*delayp = snd_pcm_mmap_hw_avail(pcm);
	}
	return 0;
}

static int snd_pcm_ioplug_reset(snd_pcm_t *pcm)
{
	ioplug_priv_t *io = pcm->private_data;

	io->data->appl_ptr = 0;
	io->data->hw_ptr = 0;
	io->last_hw = 0;
	io->avail_max = 0;
	return 0;
}

static int snd_pcm_ioplug_prepare(snd_pcm_t *pcm)
{
	ioplug_priv_t *io = pcm->private_data;

	io->data->state = SND_PCM_STATE_PREPARED;
	snd_pcm_ioplug_reset(pcm);
	if (io->data->callback->prepare)
		return io->data->callback->prepare(io->data);
	return 0;
}

static const int hw_params_type[SND_PCM_IOPLUG_HW_PARAMS] = {
	[SND_PCM_IOPLUG_HW_ACCESS] = SND_PCM_HW_PARAM_ACCESS,
	[SND_PCM_IOPLUG_HW_FORMAT] = SND_PCM_HW_PARAM_FORMAT,
	[SND_PCM_IOPLUG_HW_CHANNELS] = SND_PCM_HW_PARAM_CHANNELS,
	[SND_PCM_IOPLUG_HW_RATE] = SND_PCM_HW_PARAM_RATE,
	[SND_PCM_IOPLUG_HW_PERIOD_BYTES] = SND_PCM_HW_PARAM_PERIOD_BYTES,
	[SND_PCM_IOPLUG_HW_BUFFER_BYTES] = SND_PCM_HW_PARAM_BUFFER_BYTES,
	[SND_PCM_IOPLUG_HW_PERIODS] = SND_PCM_HW_PARAM_PERIODS,
};

/* x = a * b */
static int rule_mul(snd_pcm_hw_params_t *params, int x, int a, int b)
{
	snd_interval_t t;

	snd_interval_mul(hw_param_interval(params, a),
			 hw_param_interval(params, b), &t);
	return snd_interval_refine(hw_param_interval(params, x), &t);
}

/* x = a / b */
static int rule_div(snd_pcm_hw_params_t *params, int x, int a, int b)
{
	snd_interval_t t;

	snd_interval_div(hw_param_interval(params, a),
			 hw_param_interval(params, b), &t);
	return snd_interval_refine(hw_param_interval(params, x), &t);
}

/* x = a * b / k */
static int rule_muldivk(snd_pcm_hw_params_t *params, int x, int a, int b, int k)
{
	snd_interval_t t;

	snd_interval_muldivk(hw_param_interval(params, a),
			     hw_param_interval(params, b), k, &t);
	return snd_interval_refine(hw_param_interval(params, x), &t);
}

/* x = a * k / b */
static int rule_mulkdiv(snd_pcm_hw_params_t *params, int x, int a, int k, int b)
{
	snd_interval_t t;

	snd_interval_mulkdiv(hw_param_interval(params, a), k,
			     hw_param_interval(params, b), &t);
	return snd_interval_refine(hw_param_interval(params, x), &t);
}


/* refine *_TIME and *_SIZE, then update *_BYTES */
static int refine_time_and_size(snd_pcm_hw_params_t *params,
				int time, int size, int bytes)
{
	int err, change1 = 0;

	/* size = time * rate / 1000000 */
	err = rule_muldivk(params, size, time,
			   SND_PCM_HW_PARAM_RATE, 1000000);
	if (err < 0)
		return err;
	change1 |= err;

	/* bytes = size * framebits / 8 */
	err = rule_muldivk(params, bytes, size,
			   SND_PCM_HW_PARAM_FRAME_BITS, 8);
	if (err < 0)
		return err;
	change1 |= err;
	return change1;
}

/* refine *_TIME and *_SIZE from *_BYTES */
static int refine_back_time_and_size(snd_pcm_hw_params_t *params,
				     int time, int size, int bytes)
{
	int err;

	/* size = bytes * 8 / framebits */
	err = rule_mulkdiv(params, size, bytes, 8, SND_PCM_HW_PARAM_FRAME_BITS);
	if (err < 0)
		return err;
	/* time = size * 1000000 / rate */
	err = rule_mulkdiv(params, time, size, 1000000, SND_PCM_HW_PARAM_RATE);
	if (err < 0)
		return err;
	return 0;
}


static int snd_pcm_ioplug_hw_refine(snd_pcm_t *pcm, snd_pcm_hw_params_t *params)
{
	int change = 0, change1, change2, err;
	ioplug_priv_t *io = pcm->private_data;
	struct snd_ext_parm *p;
	unsigned int i;

	/* access, format */
	for (i = SND_PCM_IOPLUG_HW_ACCESS; i <= SND_PCM_IOPLUG_HW_FORMAT; i++) {
		err = snd_ext_parm_mask_refine(hw_param_mask(params, hw_params_type[i]),
					       io->params, i);
		if (err < 0)
			return err;
		change |= err;
	}
	/* channels, rate */
	for (; i <= SND_PCM_IOPLUG_HW_RATE; i++) {
		err = snd_ext_parm_interval_refine(hw_param_interval(params, hw_params_type[i]),
						   io->params, i);
		if (err < 0)
			return err;
		change |= err;
	}

	if (params->rmask & ((1 << SND_PCM_HW_PARAM_ACCESS) |
			     (1 << SND_PCM_HW_PARAM_FORMAT) |
			     (1 << SND_PCM_HW_PARAM_SUBFORMAT) |
			     (1 << SND_PCM_HW_PARAM_CHANNELS) |
			     (1 << SND_PCM_HW_PARAM_RATE))) {
		err = snd_pcm_hw_refine_soft(pcm, params);
		if (err < 0)
			return err;
		change |= err;
	}

	change1 = refine_time_and_size(params, SND_PCM_HW_PARAM_PERIOD_TIME,
				       SND_PCM_HW_PARAM_PERIOD_SIZE,
				       SND_PCM_HW_PARAM_PERIOD_BYTES);
	if (change1 < 0)
		return change1;
	err = snd_ext_parm_interval_refine(hw_param_interval(params, SND_PCM_HW_PARAM_PERIOD_BYTES),
					   io->params, SND_PCM_IOPLUG_HW_PERIOD_BYTES);
	if (err < 0)
		return err;
	change1 |= err;
	if (change1) {
		change |= change1;
		err = refine_back_time_and_size(params, SND_PCM_HW_PARAM_PERIOD_TIME,
						SND_PCM_HW_PARAM_PERIOD_SIZE,
						SND_PCM_HW_PARAM_PERIOD_BYTES);
		if (err < 0)
			return err;
	}

	change1 = refine_time_and_size(params, SND_PCM_HW_PARAM_BUFFER_TIME,
				       SND_PCM_HW_PARAM_BUFFER_SIZE,
				       SND_PCM_HW_PARAM_BUFFER_BYTES);
	if (change1 < 0)
		return change1;
	change |= change1;

	do {
		change2 = 0;
		err = snd_ext_parm_interval_refine(hw_param_interval(params, SND_PCM_HW_PARAM_BUFFER_BYTES),
						   io->params, SND_PCM_IOPLUG_HW_BUFFER_BYTES);
		if (err < 0)
			return err;
		change2 |= err;
		/* periods = buffer_bytes / period_bytes */
		err = rule_div(params, SND_PCM_HW_PARAM_PERIODS,
			       SND_PCM_HW_PARAM_BUFFER_BYTES,
			       SND_PCM_HW_PARAM_PERIOD_BYTES);
		if (err < 0)
			return err;
		change2 |= err;
		err = snd_ext_parm_interval_refine(hw_param_interval(params, SND_PCM_HW_PARAM_PERIODS),
						   io->params, SND_PCM_IOPLUG_HW_PERIODS);
		if (err < 0)
			return err;
		change2 |= err;
		/* buffer_bytes = periods * period_bytes */
		err = rule_mul(params, SND_PCM_HW_PARAM_BUFFER_BYTES,
			       SND_PCM_HW_PARAM_PERIOD_BYTES,
			       SND_PCM_HW_PARAM_PERIODS);
		if (err < 0)
			return err;
		change2 |= err;
		change1 |= change2;
	} while (change2);
	change |= change1;

	if (change1) {
		err = refine_back_time_and_size(params, SND_PCM_HW_PARAM_BUFFER_TIME,
						SND_PCM_HW_PARAM_BUFFER_SIZE,
						SND_PCM_HW_PARAM_BUFFER_BYTES);
		if (err < 0)
			return err;
	}

	/* period_bytes = buffer_bytes / periods */
	err = rule_div(params, SND_PCM_HW_PARAM_PERIOD_BYTES,
		       SND_PCM_HW_PARAM_BUFFER_BYTES,
		       SND_PCM_HW_PARAM_PERIODS);
	if (err < 0)
		return err;
	if (err) {
		/* update period_size and period_time */
		change |= err;
		err = snd_ext_parm_interval_refine(hw_param_interval(params, SND_PCM_HW_PARAM_PERIOD_BYTES),
						   io->params, SND_PCM_IOPLUG_HW_PERIOD_BYTES);
		if (err < 0)
			return err;
		err = refine_back_time_and_size(params, SND_PCM_HW_PARAM_PERIOD_TIME,
						SND_PCM_HW_PARAM_PERIOD_SIZE,
						SND_PCM_HW_PARAM_PERIOD_BYTES);
		if (err < 0)
			return err;
	}

	params->info = SND_PCM_INFO_BLOCK_TRANSFER;
	p = &io->params[SND_PCM_IOPLUG_HW_ACCESS];
	if (p->active) {
		for (i = 0; i < p->num_list; i++)
			switch (p->list[i]) {
			case SND_PCM_ACCESS_MMAP_INTERLEAVED:
			case SND_PCM_ACCESS_RW_INTERLEAVED:
				params->info |= SND_PCM_INFO_INTERLEAVED;
				break;
			case SND_PCM_ACCESS_MMAP_NONINTERLEAVED:
			case SND_PCM_ACCESS_RW_NONINTERLEAVED:
				params->info |= SND_PCM_INFO_NONINTERLEAVED;
				break;
			}
	}
	if (io->data->callback->pause)
		params->info |= SND_PCM_INFO_PAUSE;
	if (io->data->callback->resume)
		params->info |= SND_PCM_INFO_RESUME;

	return change;
}

static int snd_pcm_ioplug_hw_params(snd_pcm_t *pcm, snd_pcm_hw_params_t *params)
{
	ioplug_priv_t *io = pcm->private_data;
	int err;

	INTERNAL(snd_pcm_hw_params_get_access)(params, &io->data->access);
	INTERNAL(snd_pcm_hw_params_get_format)(params, &io->data->format);
	INTERNAL(snd_pcm_hw_params_get_channels)(params, &io->data->channels);
	INTERNAL(snd_pcm_hw_params_get_rate)(params, &io->data->rate, 0);
	INTERNAL(snd_pcm_hw_params_get_period_size)(params, &io->data->period_size, 0);
	INTERNAL(snd_pcm_hw_params_get_buffer_size)(params, &io->data->buffer_size);
	if (io->data->callback->hw_params) {
		err = io->data->callback->hw_params(io->data, params);
		if (err < 0)
			return err;
		INTERNAL(snd_pcm_hw_params_get_access)(params, &io->data->access);
		INTERNAL(snd_pcm_hw_params_get_format)(params, &io->data->format);
		INTERNAL(snd_pcm_hw_params_get_channels)(params, &io->data->channels);
		INTERNAL(snd_pcm_hw_params_get_rate)(params, &io->data->rate, 0);
		INTERNAL(snd_pcm_hw_params_get_period_size)(params, &io->data->period_size, 0);
		INTERNAL(snd_pcm_hw_params_get_buffer_size)(params, &io->data->buffer_size);
	}
	return 0;
}

static int snd_pcm_ioplug_hw_free(snd_pcm_t *pcm)
{
	ioplug_priv_t *io = pcm->private_data;

	if (io->data->callback->hw_free)
		return io->data->callback->hw_free(io->data);
	return 0;
}

static int snd_pcm_ioplug_sw_params(snd_pcm_t *pcm, snd_pcm_sw_params_t *params)
{
	ioplug_priv_t *io = pcm->private_data;

	if (io->data->callback->sw_params)
		return io->data->callback->sw_params(io->data, params);
	return 0;
}


static int snd_pcm_ioplug_start(snd_pcm_t *pcm)
{
	ioplug_priv_t *io = pcm->private_data;
	int err;
	
	if (io->data->state != SND_PCM_STATE_PREPARED)
		return -EBADFD;

	err = io->data->callback->start(io->data);
	if (err < 0)
		return err;

	gettimestamp(&io->trigger_tstamp, pcm->monotonic);
	io->data->state = SND_PCM_STATE_RUNNING;

	return 0;
}

static int snd_pcm_ioplug_drop(snd_pcm_t *pcm)
{
	ioplug_priv_t *io = pcm->private_data;

	if (io->data->state == SND_PCM_STATE_OPEN)
		return -EBADFD;

	io->data->callback->stop(io->data);

	gettimestamp(&io->trigger_tstamp, pcm->monotonic);
	io->data->state = SND_PCM_STATE_SETUP;

	return 0;
}

static int snd_pcm_ioplug_drain(snd_pcm_t *pcm)
{
	ioplug_priv_t *io = pcm->private_data;

	if (io->data->state == SND_PCM_STATE_OPEN)
		return -EBADFD;
	if (io->data->callback->drain)
		io->data->callback->drain(io->data);
	return snd_pcm_ioplug_drop(pcm);
}

static int snd_pcm_ioplug_pause(snd_pcm_t *pcm, int enable)
{
	ioplug_priv_t *io = pcm->private_data;
	static const snd_pcm_state_t states[2] = {
		SND_PCM_STATE_RUNNING, SND_PCM_STATE_PAUSED
	};
	int prev, err;

	prev = !enable;
	enable = !prev;
	if (io->data->state != states[prev])
		return -EBADFD;
	if (io->data->callback->pause) {
		err = io->data->callback->pause(io->data, enable);
		if (err < 0)
			return err;
	}
	io->data->state = states[enable];
	return 0;
}

static snd_pcm_sframes_t snd_pcm_ioplug_rewindable(snd_pcm_t *pcm)
{
	return snd_pcm_mmap_hw_avail(pcm);
}

static snd_pcm_sframes_t snd_pcm_ioplug_rewind(snd_pcm_t *pcm, snd_pcm_uframes_t frames)
{
	snd_pcm_mmap_appl_backward(pcm, frames);
	return frames;
}

static snd_pcm_sframes_t snd_pcm_ioplug_forwardable(snd_pcm_t *pcm)
{
	return snd_pcm_mmap_avail(pcm);
}

static snd_pcm_sframes_t snd_pcm_ioplug_forward(snd_pcm_t *pcm, snd_pcm_uframes_t frames)
{
	snd_pcm_mmap_appl_forward(pcm, frames);
	return frames;
}

static int snd_pcm_ioplug_resume(snd_pcm_t *pcm)
{
	ioplug_priv_t *io = pcm->private_data;

	if (io->data->callback->resume)
		io->data->callback->resume(io->data);
	return 0;
}

static snd_pcm_sframes_t ioplug_priv_transfer_areas(snd_pcm_t *pcm,
						       const snd_pcm_channel_area_t *areas,
						       snd_pcm_uframes_t offset,
						       snd_pcm_uframes_t size)
{
	ioplug_priv_t *io = pcm->private_data;
	snd_pcm_sframes_t result;
		
	if (! size)
		return 0;
	if (io->data->callback->transfer)
		result = io->data->callback->transfer(io->data, areas, offset, size);
	else
		result = size;
	if (result > 0)
		snd_pcm_mmap_appl_forward(pcm, result);
	return result;
}

static snd_pcm_sframes_t snd_pcm_ioplug_writei(snd_pcm_t *pcm, const void *buffer, snd_pcm_uframes_t size)
{
	if (pcm->mmap_rw)
		return snd_pcm_mmap_writei(pcm, buffer, size);
	else {
		snd_pcm_channel_area_t areas[pcm->channels];
		snd_pcm_areas_from_buf(pcm, areas, (void*)buffer);
		return snd_pcm_write_areas(pcm, areas, 0, size, 
					   ioplug_priv_transfer_areas);
	}
}

static snd_pcm_sframes_t snd_pcm_ioplug_writen(snd_pcm_t *pcm, void **bufs, snd_pcm_uframes_t size)
{
	if (pcm->mmap_rw)
		return snd_pcm_mmap_writen(pcm, bufs, size);
	else {
		snd_pcm_channel_area_t areas[pcm->channels];
		snd_pcm_areas_from_bufs(pcm, areas, bufs);
		return snd_pcm_write_areas(pcm, areas, 0, size,
					   ioplug_priv_transfer_areas);
	}
}

static snd_pcm_sframes_t snd_pcm_ioplug_readi(snd_pcm_t *pcm, void *buffer, snd_pcm_uframes_t size)
{
	if (pcm->mmap_rw)
		return snd_pcm_mmap_readi(pcm, buffer, size);
	else {
		snd_pcm_channel_area_t areas[pcm->channels];
		snd_pcm_areas_from_buf(pcm, areas, buffer);
		return snd_pcm_read_areas(pcm, areas, 0, size,
					  ioplug_priv_transfer_areas);
	}
}

static snd_pcm_sframes_t snd_pcm_ioplug_readn(snd_pcm_t *pcm, void **bufs, snd_pcm_uframes_t size)
{
	if (pcm->mmap_rw)
		return snd_pcm_mmap_readn(pcm, bufs, size);
	else {
		snd_pcm_channel_area_t areas[pcm->channels];
		snd_pcm_areas_from_bufs(pcm, areas, bufs);
		return snd_pcm_read_areas(pcm, areas, 0, size,
					  ioplug_priv_transfer_areas);
	}
}

static snd_pcm_sframes_t snd_pcm_ioplug_mmap_commit(snd_pcm_t *pcm,
						    snd_pcm_uframes_t offset,
						    snd_pcm_uframes_t size)
{
	if (pcm->stream == SND_PCM_STREAM_PLAYBACK &&
	    pcm->access != SND_PCM_ACCESS_RW_INTERLEAVED &&
	    pcm->access != SND_PCM_ACCESS_RW_NONINTERLEAVED) {
		const snd_pcm_channel_area_t *areas;
		snd_pcm_uframes_t ofs, frames = size;

		snd_pcm_mmap_begin(pcm, &areas, &ofs, &frames);
		if (ofs != offset)
			return -EIO;
		return ioplug_priv_transfer_areas(pcm, areas, offset, frames);
	}

	snd_pcm_mmap_appl_forward(pcm, size);
	return size;
}

static snd_pcm_sframes_t snd_pcm_ioplug_avail_update(snd_pcm_t *pcm)
{
	ioplug_priv_t *io = pcm->private_data;
	snd_pcm_uframes_t avail;

	snd_pcm_ioplug_hw_ptr_update(pcm);
	if (io->data->state == SNDRV_PCM_STATE_XRUN)
		return -EPIPE;
	if (pcm->stream == SND_PCM_STREAM_CAPTURE &&
	    pcm->access != SND_PCM_ACCESS_RW_INTERLEAVED &&
	    pcm->access != SND_PCM_ACCESS_RW_NONINTERLEAVED) {
		if (io->data->callback->transfer) {
			const snd_pcm_channel_area_t *areas;
			snd_pcm_uframes_t offset, size = UINT_MAX;
			snd_pcm_sframes_t result;

			snd_pcm_mmap_begin(pcm, &areas, &offset, &size);
			result = io->data->callback->transfer(io->data, areas, offset, size);
			if (result < 0)
				return result;
		}
	}
	avail = snd_pcm_mmap_avail(pcm);
	if (avail > io->avail_max)
		io->avail_max = avail;
	return (snd_pcm_sframes_t)avail;
}

static int snd_pcm_ioplug_nonblock(snd_pcm_t *pcm, int nonblock)
{
	ioplug_priv_t *io = pcm->private_data;

	io->data->nonblock = nonblock;
	return 0;
}

static int snd_pcm_ioplug_poll_descriptors_count(snd_pcm_t *pcm)
{
	ioplug_priv_t *io = pcm->private_data;

	if (io->data->callback->poll_descriptors_count)
		return io->data->callback->poll_descriptors_count(io->data);
	else
		return 1;
}

static int snd_pcm_ioplug_poll_descriptors(snd_pcm_t *pcm, struct pollfd *pfds, unsigned int space)
{
	ioplug_priv_t *io = pcm->private_data;

	if (io->data->callback->poll_descriptors)
		return io->data->callback->poll_descriptors(io->data, pfds, space);
	if (pcm->poll_fd < 0)
		return -EIO;
	if (space >= 1 && pfds) {
		pfds->fd = pcm->poll_fd;
		pfds->events = pcm->poll_events | POLLERR | POLLNVAL;
	} else {
		return 0;
	}
	return 1;
}

static int snd_pcm_ioplug_poll_revents(snd_pcm_t *pcm, struct pollfd *pfds, unsigned int nfds, unsigned short *revents)
{
	ioplug_priv_t *io = pcm->private_data;

	if (io->data->callback->poll_revents)
		return io->data->callback->poll_revents(io->data, pfds, nfds, revents);
	else
		*revents = pfds->revents;
	return 0;
}

static int snd_pcm_ioplug_mmap(snd_pcm_t *pcm ATTRIBUTE_UNUSED)
{
	return 0;
}

static int snd_pcm_ioplug_async(snd_pcm_t *pcm ATTRIBUTE_UNUSED,
				int sig ATTRIBUTE_UNUSED,
				pid_t pid ATTRIBUTE_UNUSED)
{
	return -ENOSYS;
}

static int snd_pcm_ioplug_munmap(snd_pcm_t *pcm ATTRIBUTE_UNUSED)
{
	return 0;
}

static void snd_pcm_ioplug_dump(snd_pcm_t *pcm, snd_output_t *out)
{
	ioplug_priv_t *io = pcm->private_data;

	if (io->data->callback->dump)
		io->data->callback->dump(io->data, out);
	else {
		if (io->data->name)
			snd_output_printf(out, "%s\n", io->data->name);
		else
			snd_output_printf(out, "IO-PCM Plugin\n");
		if (pcm->setup) {
			snd_output_printf(out, "Its setup is:\n");
			snd_pcm_dump_setup(pcm, out);
		}
	}
}

static void clear_io_params(ioplug_priv_t *io)
{
	int i;
	for (i = 0; i < SND_PCM_IOPLUG_HW_PARAMS; i++)
		snd_ext_parm_clear(&io->params[i]);
}

static int snd_pcm_ioplug_close(snd_pcm_t *pcm)
{
	ioplug_priv_t *io = pcm->private_data;

	clear_io_params(io);
	if (io->data->callback->close)
		io->data->callback->close(io->data);
	free(io);

	return 0;
}

static const snd_pcm_ops_t snd_pcm_ioplug_ops = {
	.close = snd_pcm_ioplug_close,
	.nonblock = snd_pcm_ioplug_nonblock,
	.async = snd_pcm_ioplug_async,
	.info = snd_pcm_ioplug_info,
	.hw_refine = snd_pcm_ioplug_hw_refine,
	.hw_params = snd_pcm_ioplug_hw_params,
	.hw_free = snd_pcm_ioplug_hw_free,
	.sw_params = snd_pcm_ioplug_sw_params,
	.channel_info = snd_pcm_ioplug_channel_info,
	.dump = snd_pcm_ioplug_dump,
	.mmap = snd_pcm_ioplug_mmap,
	.munmap = snd_pcm_ioplug_munmap,
};

static const snd_pcm_fast_ops_t snd_pcm_ioplug_fast_ops = {
	.status = snd_pcm_ioplug_status,
	.prepare = snd_pcm_ioplug_prepare,
	.reset = snd_pcm_ioplug_reset,
	.start = snd_pcm_ioplug_start,
	.drop = snd_pcm_ioplug_drop,
	.drain = snd_pcm_ioplug_drain,
	.pause = snd_pcm_ioplug_pause,
	.state = snd_pcm_ioplug_state,
	.hwsync = snd_pcm_ioplug_hwsync,
	.delay = snd_pcm_ioplug_delay,
	.resume = snd_pcm_ioplug_resume,
	.link = NULL,
	.link_slaves = NULL,
	.unlink = NULL,
	.rewindable = snd_pcm_ioplug_rewindable,
	.rewind = snd_pcm_ioplug_rewind,
	.forwardable = snd_pcm_ioplug_forwardable,
	.forward = snd_pcm_ioplug_forward,
	.writei = snd_pcm_ioplug_writei,
	.writen = snd_pcm_ioplug_writen,
	.readi = snd_pcm_ioplug_readi,
	.readn = snd_pcm_ioplug_readn,
	.avail_update = snd_pcm_ioplug_avail_update,
	.mmap_commit = snd_pcm_ioplug_mmap_commit,
	.htimestamp = snd_pcm_generic_real_htimestamp,
	.poll_descriptors_count = snd_pcm_ioplug_poll_descriptors_count,
	.poll_descriptors = snd_pcm_ioplug_poll_descriptors,
	.poll_revents = snd_pcm_ioplug_poll_revents,
};

#endif /* !DOC_HIDDEN */

/*
 * Exported functions
 */

/*! \page pcm_external_plugins PCM External Plugin SDK

\section pcm_ioplug External Plugin: I/O Plugin

The I/O-type plugin is a PCM plugin to work as the input or output terminal point,
i.e. as a user-space PCM driver.

The new plugin is created via #snd_pcm_ioplug_create() function.
The first argument is a pointer of the pluging information.  Some of
this struct must be initialized in prior to call
#snd_pcm_ioplug_create().  Then the function fills other fields in
return.  The rest arguments, name, stream and mode, are usually
identical with the values passed from the ALSA plugin constructor.

The following fields are mandatory: version, name, callback.
Otherfields are optional and should be initialized with zero.

The constant #SND_PCM_IOPLUG_VERSION must be passed to the version
field for the version check in alsa-lib.  A non-NULL ASCII string
has to be passed to the name field.  The callback field contains the 
table of callback functions for this plugin (defined as
#snd_pcm_ioplug_callback_t).

flags field specifies the optional bit-flags.  poll_fd and poll_events
specify the poll file descriptor and the corresponding poll events
(POLLIN, POLLOUT) for the plugin.  If the plugin requires multiple
poll descriptors or poll descriptor(s) dynamically varying, set
poll_descriptors and poll_descriptors_count callbacks to the callback
table.  Then the poll_fd and poll_events field are ignored.

mmap_rw specifies whether the plugin behaves in the pseudo mmap mode.
When this value is set to 1, the plugin creates always a local buffer
and performs read/write calls using this buffer as if it's mmapped.
The address of local buffer can be obtained via
#snd_pcm_ioplug_mmap_areas() function.
When poll_fd, poll_events and mmap_rw fields are changed after
#snd_pcm_ioplug_create(), call #snd_pcm_ioplug_reinit_status() to
reflect the changes.

The driver can set an arbitrary value (pointer) to private_data
field to refer its own data in the callbacks.

The rest fields are filled by #snd_pcm_ioplug_create().  The pcm field
is the resultant PCM handle.  The others are the current status of the
PCM.

The callback functions in #snd_pcm_ioplug_callback_t define the real
behavior of the driver.
At least, start, stop and pointer callbacks must be given.  Other
callbacks are optional.  The start and stop callbacks are called when
the PCM stream is started and stopped, repsectively.  The pointer
callback returns the current DMA position, which may be called at any
time.

The transfer callback is called when any data transfer happens.  It
receives the area array, offset and the size to transfer.  The area
array contains the array of snd_pcm_channel_area_t with the elements
of number of channels.

When the PCM is closed, close callback is called.  If the driver
allocates any internal buffers, they should be released in this
callback.  The hw_params and hw_free callbacks are called when
hw_params are set and reset, respectively.  Note that they may be
called multiple times according to the application.  Similarly,
sw_params callback is called when sw_params is set or changed.

The prepare, drain, pause and resume callbacks are called when
#snd_pcm_prepare(), #snd_pcm_drain(), #snd_pcm_pause(), and
#snd_pcm_resume() are called.  The poll_descriptors_count and
poll_descriptors callbacks are used to return the multiple or dynamic
poll descriptors as mentioned above.  The poll_revents callback is
used to modify poll events.  If the driver needs to mangle the native
poll events to proper poll events for PCM, you can do it in this
callback.

Finally, the dump callback is used to print the status of the plugin.

The hw_params constraints can be defined via either
#snd_pcm_ioplug_set_param_minmax() and #snd_pcm_ioplug_set_param_list()
functions after calling #snd_pcm_ioplug_create().
The former defines the minimal and maximal acceptable values for the
given hw_params parameter (SND_PCM_IOPLUG_HW_XXX).
This function can't be used for the format parameter.  The latter
function specifies the available parameter values as the list.

To clear the parameter constraints, call #snd_pcm_ioplug_params_reset() function.

*/

/**
 * \brief Create an ioplug instance
 * \param ioplug the ioplug handle
 * \param name name of PCM
 * \param stream stream direction
 * \param mode PCM open mode
 * \return 0 if successful, or a negative error code
 *
 * Creates the ioplug instance.
 *
 * The callback is the mandatory field of ioplug handle.  At least, start, stop and
 * pointer callbacks must be set before calling this function.
 *
 */
int snd_pcm_ioplug_create(snd_pcm_ioplug_t *ioplug, const char *name,
			  snd_pcm_stream_t stream, int mode)
{
	ioplug_priv_t *io;
	int err;
	snd_pcm_t *pcm;

	assert(ioplug && ioplug->callback);
	assert(ioplug->callback->start &&
	       ioplug->callback->stop &&
	       ioplug->callback->pointer);

	/* We support 1.0.0 to current */
	if (ioplug->version < 0x010000 ||
	    ioplug->version > SND_PCM_IOPLUG_VERSION) {
		SNDERR("ioplug: Plugin version mismatch\n");
		return -ENXIO;
	}

	io = calloc(1, sizeof(*io));
	if (! io)
		return -ENOMEM;

	io->data = ioplug;
	ioplug->state = SND_PCM_STATE_OPEN;
	ioplug->stream = stream;

	err = snd_pcm_new(&pcm, SND_PCM_TYPE_IOPLUG, name, stream, mode);
	if (err < 0) {
		free(io);
		return err;
	}

	ioplug->pcm = pcm;
	pcm->ops = &snd_pcm_ioplug_ops;
	pcm->fast_ops = &snd_pcm_ioplug_fast_ops;
	pcm->private_data = io;

	snd_pcm_set_hw_ptr(pcm, &ioplug->hw_ptr, -1, 0);
	snd_pcm_set_appl_ptr(pcm, &ioplug->appl_ptr, -1, 0);

	snd_pcm_ioplug_reinit_status(ioplug);

	return 0;
}

/**
 * \brief Delete the ioplug instance
 * \param ioplug the ioplug handle
 * \return 0 if successful, or a negative error code
 */
int snd_pcm_ioplug_delete(snd_pcm_ioplug_t *ioplug)
{
	return snd_pcm_close(ioplug->pcm);
}


/**
 * \brief Reset ioplug parameters
 * \param ioplug the ioplug handle
 *
 * Resets the all parameters for the given ioplug handle.
 */
void snd_pcm_ioplug_params_reset(snd_pcm_ioplug_t *ioplug)
{
	ioplug_priv_t *io = ioplug->pcm->private_data;
	clear_io_params(io);
}

/**
 * \brief Set parameter as the list
 * \param ioplug the ioplug handle
 * \param type parameter type
 * \param num_list number of available values
 * \param list the list of available values
 * \return 0 if successful, or a negative error code
 *
 * Sets the parameter as the list.
 * The available values of the given parameter type is restricted to the ones of the given list.
 */
int snd_pcm_ioplug_set_param_list(snd_pcm_ioplug_t *ioplug, int type, unsigned int num_list, const unsigned int *list)
{
	ioplug_priv_t *io = ioplug->pcm->private_data;
	if (type < 0 && type >= SND_PCM_IOPLUG_HW_PARAMS) {
		SNDERR("IOPLUG: invalid parameter type %d", type);
		return -EINVAL;
	}
	if (type == SND_PCM_IOPLUG_HW_PERIODS)
		io->params[type].integer = 1;
	return snd_ext_parm_set_list(&io->params[type], num_list, list);
}

/**
 * \brief Set parameter as the min/max values
 * \param ioplug the ioplug handle
 * \param type parameter type
 * \param min the minimum value
 * \param max the maximum value
 * \return 0 if successful, or a negative error code
 *
 * Sets the parameter as the min/max values.
 * The available values of the given parameter type is restricted between the given
 * minimum and maximum values.
 */
int snd_pcm_ioplug_set_param_minmax(snd_pcm_ioplug_t *ioplug, int type, unsigned int min, unsigned int max)
{
	ioplug_priv_t *io = ioplug->pcm->private_data;
	if (type < 0 && type >= SND_PCM_IOPLUG_HW_PARAMS) {
		SNDERR("IOPLUG: invalid parameter type %d", type);
		return -EINVAL;
	}
	if (type == SND_PCM_IOPLUG_HW_ACCESS || type == SND_PCM_IOPLUG_HW_FORMAT) {
		SNDERR("IOPLUG: invalid parameter type %d", type);
		return -EINVAL;
	}
	if (type == SND_PCM_IOPLUG_HW_PERIODS)
		io->params[type].integer = 1;
	return snd_ext_parm_set_minmax(&io->params[type], min, max);
}

/**
 * \brief Reinitialize the poll and mmap status
 * \param ioplug the ioplug handle
 * \return 0 if successful, or a negative error code
 *
 * Reinitializes the poll and the mmap status of the PCM.
 * Call this function to propagate the status change in the ioplug instance to
 * its PCM internals.
 */
int snd_pcm_ioplug_reinit_status(snd_pcm_ioplug_t *ioplug)
{
	ioplug->pcm->poll_fd = ioplug->poll_fd;
	ioplug->pcm->poll_events = ioplug->poll_events;
	ioplug->pcm->monotonic = (ioplug->flags & SND_PCM_IOPLUG_FLAG_MONOTONIC) != 0;
	ioplug->pcm->mmap_rw = ioplug->mmap_rw;
	return 0;
}

/**
 * \brief Get mmap area of ioplug
 * \param ioplug the ioplug handle
 * \return the mmap channel areas if available, or NULL
 *
 * Returns the mmap channel areas if available.  When mmap_rw field is not set,
 * this function always returns NULL.
 */
const snd_pcm_channel_area_t *snd_pcm_ioplug_mmap_areas(snd_pcm_ioplug_t *ioplug)
{
	if (ioplug->mmap_rw)
		return snd_pcm_mmap_areas(ioplug->pcm);
	return NULL;
}

/**
 * \brief Change the ioplug PCM status
 * \param ioplug the ioplug handle
 * \param state the PCM status
 * \return zero if successful or a negative error code
 *
 * Changes the PCM status of the ioplug to the given value.
 * This function can be used for external plugins to notify the status
 * change, e.g. XRUN.
 */
int snd_pcm_ioplug_set_state(snd_pcm_ioplug_t *ioplug, snd_pcm_state_t state)
{
	ioplug->state = state;
	return 0;
}
