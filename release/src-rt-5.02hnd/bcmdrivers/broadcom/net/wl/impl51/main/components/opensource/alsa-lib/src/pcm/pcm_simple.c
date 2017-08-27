/**
 * \file pcm/pcm_simple.c
 * \ingroup PCM_Simple
 * \brief PCM Simple Interface
 * \author Jaroslav Kysela <perex@perex.cz>
 * \date 2004
 */
/*
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

static int set_buffer_time(snd_spcm_latency_t latency,
			   unsigned int *buffer_time)
{
	switch (latency) {
	case SND_SPCM_LATENCY_STANDARD:
		*buffer_time = 350000;
		break;
	case SND_SPCM_LATENCY_MEDIUM:
		*buffer_time = 25000;
		break;
	case SND_SPCM_LATENCY_REALTIME:
		*buffer_time = 2500;
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

static int set_hw_params(snd_pcm_t *pcm,
			 snd_pcm_hw_params_t *hw_params,
			 unsigned int *rate,
			 unsigned int channels,
			 snd_pcm_format_t format,
			 snd_pcm_subformat_t subformat,
			 unsigned int *buffer_time,
			 unsigned int *period_time,
			 snd_pcm_access_t access)
{
	int err;

	/*
	 * hardware parameters
	 */	
	err = snd_pcm_hw_params_any(pcm, hw_params);
	if (err < 0)
		return err;
	err = snd_pcm_hw_params_set_access(pcm, hw_params, access);
	if (err < 0)
		return err;
	err = snd_pcm_hw_params_set_format(pcm, hw_params, format);
	if (err < 0)
		return err;
	if (subformat != SND_PCM_SUBFORMAT_STD) {
		err = snd_pcm_hw_params_set_subformat(pcm, hw_params, subformat);
		if (err < 0)
			return err;
	}
	err = snd_pcm_hw_params_set_channels(pcm, hw_params, channels);
	if (err < 0)
		return err;
	err = INTERNAL(snd_pcm_hw_params_set_rate_near)(pcm, hw_params, rate, 0);
	if (err < 0)
		return err;
	err = INTERNAL(snd_pcm_hw_params_set_buffer_time_near)(pcm, hw_params, buffer_time, NULL);
	if (err < 0)
		return err;
	if (period_time == NULL || *period_time == 0) {
		unsigned int periods = 3;
		err = INTERNAL(snd_pcm_hw_params_set_periods_near)(pcm, hw_params, &periods, NULL);
		if (err < 0)
			return err;
		if (periods == 1)
			return -EINVAL;
		if (*period_time == 0) {
			err = INTERNAL(snd_pcm_hw_params_get_period_time)(hw_params, period_time, NULL);
			if (err < 0)
				return err;
		}			
	} else {
		err = snd_pcm_hw_params_set_period_time(pcm, hw_params, *period_time, 0);
		if (err < 0)
			return err;
		if (*buffer_time == *period_time)
			return -EINVAL;
	}
	err = snd_pcm_hw_params(pcm, hw_params);
	if (err < 0)
		return err;
	return 0;
}		

static int set_sw_params(snd_pcm_t *pcm,
			 snd_pcm_sw_params_t *sw_params,
		         snd_spcm_xrun_type_t xrun_type)
{
	int err;

	err = snd_pcm_sw_params_current(pcm, sw_params);		
	if (err < 0)
		return err;
	err = snd_pcm_sw_params_set_start_threshold(pcm, sw_params, (pcm->buffer_size / pcm->period_size) * pcm->period_size);
	if (err < 0)
		return err;
	err = snd_pcm_sw_params_set_avail_min(pcm, sw_params, pcm->period_size);
	if (err < 0)
		return err;
	switch (xrun_type) {
	case SND_SPCM_XRUN_STOP:
		err = snd_pcm_sw_params_set_stop_threshold(pcm, sw_params, pcm->buffer_size);
		break;
	case SND_SPCM_XRUN_IGNORE:
		err = snd_pcm_sw_params_set_stop_threshold(pcm, sw_params, pcm->boundary);
		break;
	default:
		return -EINVAL;
	}
	if (err < 0)
		return err;
	err = snd_pcm_sw_params(pcm, sw_params);
	if (err < 0)
		return err;
	return 0;
}

/**
 * \brief Set up a simple PCM
 * \param pcm PCM handle
 * \param rate Sample rate
 * \param channels Number of channels
 * \param format PCM format
 * \param subformat PCM subformat
 * \param latency Latency type
 * \param access PCM acceess type
 * \param xrun_type XRUN type
 * \return 0 if successful, or a negative error code
 *
 * \warning The simple PCM API may be broken in the current release.
 */
int snd_spcm_init(snd_pcm_t *pcm,
		  unsigned int rate,
		  unsigned int channels,
		  snd_pcm_format_t format,
		  snd_pcm_subformat_t subformat,
		  snd_spcm_latency_t latency,
		  snd_pcm_access_t access,
		  snd_spcm_xrun_type_t xrun_type)
{
	int err;
	snd_pcm_hw_params_t *hw_params;
	snd_pcm_sw_params_t *sw_params;
	unsigned int rrate;
	unsigned int buffer_time;

	snd_pcm_hw_params_alloca(&hw_params);
	snd_pcm_sw_params_alloca(&sw_params);

	assert(pcm);
	assert(rate > 5000 && rate < 192000);
	assert(channels > 1 && channels < 512);

	rrate = rate;
	err = set_buffer_time(latency, &buffer_time);
	if (err < 0)
		return err;
	err = set_hw_params(pcm, hw_params,
			    &rrate, channels, format, subformat,
			    &buffer_time, NULL, access);
	if (err < 0)
		return err;

	err = set_sw_params(pcm, sw_params, xrun_type);
	if (err < 0)
		return err;

	return 0;
}

/**
 * \brief Initialize simple PCMs in the duplex mode
 * \param playback_pcm PCM handle for playback
 * \param capture_pcm PCM handle for capture
 * \param rate Sample rate
 * \param channels Number of channels
 * \param format PCM format
 * \param subformat PCM subformat
 * \param latency Latency type
 * \param access PCM acceess type
 * \param xrun_type XRUN type
 * \param duplex_type Duplex mode
 * \return 0 if successful, or a negative error code
 *
 * \warning The simple PCM API may be broken in the current release.
 */
int snd_spcm_init_duplex(snd_pcm_t *playback_pcm,
			 snd_pcm_t *capture_pcm,
			 unsigned int rate,
			 unsigned int channels,
			 snd_pcm_format_t format,
			 snd_pcm_subformat_t subformat,
			 snd_spcm_latency_t latency,
			 snd_pcm_access_t access,
			 snd_spcm_xrun_type_t xrun_type,
			 snd_spcm_duplex_type_t duplex_type)
{
	int err, i;
	snd_pcm_hw_params_t *hw_params;
	snd_pcm_sw_params_t *sw_params;
	unsigned int rrate;
	unsigned int xbuffer_time, buffer_time[2];
	unsigned int period_time[2];
	snd_pcm_t *pcms[2];

	snd_pcm_hw_params_alloca(&hw_params);
	snd_pcm_sw_params_alloca(&sw_params);

	assert(playback_pcm);
	assert(capture_pcm);
	assert(rate > 5000 && rate < 192000);
	assert(channels > 1 && channels < 512);

	pcms[0] = playback_pcm;
	pcms[1] = capture_pcm;

	/*
	 * hardware parameters
	 */
	err = set_buffer_time(latency, &xbuffer_time);	
	if (err < 0)
		return err;
	
	for (i = 0; i < 2; i++) {
		buffer_time[i] = xbuffer_time;
		period_time[i] = i > 0 ? period_time[0] : 0;
		rrate = rate;
		err = set_hw_params(pcms[i], hw_params,
				    &rrate, channels, format, subformat,
				    &buffer_time[i], &period_time[i], access);
		if (err < 0)
			return err;
	}
	if (buffer_time[0] == buffer_time[1] &&
	    period_time[0] == period_time[1])
		goto __sw_params;
	if (duplex_type == SND_SPCM_DUPLEX_LIBERAL)
		goto __sw_params;
	return -EINVAL;

	/*
	 * software parameters
	 */
      __sw_params:
	for (i = 0; i < 2; i++) {
		err = set_sw_params(pcms[i], sw_params, xrun_type);
		if (err < 0)
			return err;
	}

	return 0;
}

/**
 * \brief Get the set up of simple PCM
 * \param pcm PCM handle
 * \param rate Pointer to store the current sample rate
 * \param buffer_size Pointer to store the current buffer size
 * \param period_size Pointer to store the current period size
 * \return 0 if successful, or a negative error code
 *
 * \warning The simple PCM API may be broken in the current release.
 */
int snd_spcm_init_get_params(snd_pcm_t *pcm,
			     unsigned int *rate,
			     snd_pcm_uframes_t *buffer_size,
			     snd_pcm_uframes_t *period_size)
{
	assert(pcm);
	if (!pcm->setup)
		return -EBADFD;
	if (rate)
		*rate = pcm->rate;
	if (buffer_size)
		*buffer_size = pcm->buffer_size;
	if (period_size)
		*period_size = pcm->period_size;
	return 0;
}
