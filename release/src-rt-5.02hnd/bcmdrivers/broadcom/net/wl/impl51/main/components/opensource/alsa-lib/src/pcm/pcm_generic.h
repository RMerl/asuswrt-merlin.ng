/*
 *  PCM - Common generic plugin code
 *  Copyright (c) 2004 by Jaroslav Kysela <perex@perex.cz>
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
  
typedef struct {
	snd_pcm_t *slave;
	int close_slave;
} snd_pcm_generic_t;	

/* make local functions really local */
#define snd_pcm_generic_close \
	snd1_pcm_generic_close
#define snd_pcm_generic_nonblock \
	snd1_pcm_generic_nonblock
#define snd_pcm_generic_async \
	snd1_pcm_generic_async
#define snd_pcm_generic_poll_descriptors_count \
	snd1_pcm_generic_poll_descriptors_count
#define snd_pcm_generic_poll_descriptors \
	snd1_pcm_generic_poll_descriptors
#define snd_pcm_generic_poll_revents \
	snd1_pcm_generic_poll_revents
#define snd_pcm_generic_info \
	snd1_pcm_generic_info
#define snd_pcm_generic_hw_free \
	snd1_pcm_generic_hw_free
#define snd_pcm_generic_sw_params \
	snd1_pcm_generic_sw_params
#define snd_pcm_generic_hw_refine \
	snd1_pcm_generic_hw_refine
#define snd_pcm_generic_hw_params \
	snd1_pcm_generic_hw_params
#define snd_pcm_generic_channel_info \
	snd1_pcm_generic_channel_info
#define snd_pcm_generic_channel_info_no_buffer \
	snd1_pcm_generic_channel_info_no_buffer
#define snd_pcm_generic_status \
	snd1_pcm_generic_status
#define snd_pcm_generic_state \
	snd1_pcm_generic_state
#define snd_pcm_generic_prepare \
	snd1_pcm_generic_prepare
#define snd_pcm_generic_hwsync \
	snd1_pcm_generic_hwsync
#define snd_pcm_generic_reset \
	snd1_pcm_generic_reset
#define snd_pcm_generic_start \
	snd1_pcm_generic_start
#define snd_pcm_generic_drop \
	snd1_pcm_generic_drop
#define snd_pcm_generic_drain \
	snd1_pcm_generic_drain
#define snd_pcm_generic_pause \
	snd1_pcm_generic_pause
#define snd_pcm_generic_resume \
	snd1_pcm_generic_resume
#define snd_pcm_generic_delay \
	snd1_pcm_generic_delay
#define snd_pcm_generic_forwardable \
	snd1_pcm_generic_forwardable
#define snd_pcm_generic_forward \
	snd1_pcm_generic_forward
#define snd_pcm_generic_rewindable \
	snd1_pcm_generic_rewindable
#define snd_pcm_generic_rewind \
	snd1_pcm_generic_rewind
#define snd_pcm_generic_link \
	snd1_pcm_generic_link
#define snd_pcm_generic_link_slaves \
	snd1_pcm_generic_link_slaves
#define snd_pcm_generic_unlink \
	snd1_pcm_generic_unlink
#define snd_pcm_generic_writei \
	snd1_pcm_generic_writei
#define snd_pcm_generic_writen \
	snd1_pcm_generic_writen
#define snd_pcm_generic_readi \
	snd1_pcm_generic_readi
#define snd_pcm_generic_readn \
	snd1_pcm_generic_readn
#define snd_pcm_generic_mmap_commit \
	snd1_pcm_generic_mmap_commit
#define snd_pcm_generic_avail_update	\
	snd1_pcm_generic_avail_update
#define snd_pcm_generic_mmap \
	snd1_pcm_generic_mmap
#define snd_pcm_generic_munmap \
	snd1_pcm_generic_munmap

int snd_pcm_generic_close(snd_pcm_t *pcm);
int snd_pcm_generic_nonblock(snd_pcm_t *pcm, int nonblock);
int snd_pcm_generic_async(snd_pcm_t *pcm, int sig, pid_t pid);
int snd_pcm_generic_poll_descriptors_count(snd_pcm_t *pcm);
int snd_pcm_generic_poll_descriptors(snd_pcm_t *pcm, struct pollfd *pfds, unsigned int space);
int snd_pcm_generic_poll_revents(snd_pcm_t *pcm, struct pollfd *pfds, unsigned int nfds, unsigned short *revents);
int snd_pcm_generic_info(snd_pcm_t *pcm, snd_pcm_info_t * info);
int snd_pcm_generic_hw_free(snd_pcm_t *pcm);
int snd_pcm_generic_sw_params(snd_pcm_t *pcm, snd_pcm_sw_params_t *params);
int snd_pcm_generic_hw_refine(snd_pcm_t *pcm, snd_pcm_hw_params_t *params);
int snd_pcm_generic_hw_params(snd_pcm_t *pcm, snd_pcm_hw_params_t *params);
int snd_pcm_generic_channel_info(snd_pcm_t *pcm, snd_pcm_channel_info_t * info);
int snd_pcm_generic_channel_info_no_buffer(snd_pcm_t *pcm, snd_pcm_channel_info_t * info);
int snd_pcm_generic_status(snd_pcm_t *pcm, snd_pcm_status_t * status);
snd_pcm_state_t snd_pcm_generic_state(snd_pcm_t *pcm);
int snd_pcm_generic_prepare(snd_pcm_t *pcm);
int snd_pcm_generic_hwsync(snd_pcm_t *pcm);
int snd_pcm_generic_reset(snd_pcm_t *pcm);
int snd_pcm_generic_start(snd_pcm_t *pcm);
int snd_pcm_generic_drop(snd_pcm_t *pcm);
int snd_pcm_generic_drain(snd_pcm_t *pcm);
int snd_pcm_generic_pause(snd_pcm_t *pcm, int enable);
int snd_pcm_generic_resume(snd_pcm_t *pcm);
int snd_pcm_generic_delay(snd_pcm_t *pcm, snd_pcm_sframes_t *delayp);
snd_pcm_sframes_t snd_pcm_generic_forwardable(snd_pcm_t *pcm);
snd_pcm_sframes_t snd_pcm_generic_forward(snd_pcm_t *pcm, snd_pcm_uframes_t frames);
snd_pcm_sframes_t snd_pcm_generic_rewindable(snd_pcm_t *pcm);
snd_pcm_sframes_t snd_pcm_generic_rewind(snd_pcm_t *pcm, snd_pcm_uframes_t frames);
int snd_pcm_generic_link(snd_pcm_t *pcm1, snd_pcm_t *pcm2);
int snd_pcm_generic_link_slaves(snd_pcm_t *pcm, snd_pcm_t *master);
int snd_pcm_generic_unlink(snd_pcm_t *pcm);
snd_pcm_sframes_t snd_pcm_generic_writei(snd_pcm_t *pcm, const void *buffer, snd_pcm_uframes_t size);
snd_pcm_sframes_t snd_pcm_generic_writen(snd_pcm_t *pcm, void **bufs, snd_pcm_uframes_t size);
snd_pcm_sframes_t snd_pcm_generic_readi(snd_pcm_t *pcm, void *buffer, snd_pcm_uframes_t size);
snd_pcm_sframes_t snd_pcm_generic_readn(snd_pcm_t *pcm, void **bufs, snd_pcm_uframes_t size);
snd_pcm_sframes_t snd_pcm_generic_mmap_commit(snd_pcm_t *pcm,
					      snd_pcm_uframes_t offset,
					      snd_pcm_uframes_t size);
snd_pcm_sframes_t snd_pcm_generic_avail_update(snd_pcm_t *pcm);
int snd_pcm_generic_htimestamp(snd_pcm_t *pcm, snd_pcm_uframes_t *avail,
			       snd_htimestamp_t *timestamp);
int snd_pcm_generic_real_htimestamp(snd_pcm_t *pcm, snd_pcm_uframes_t *avail,
				    snd_htimestamp_t *tstamp);
int snd_pcm_generic_mmap(snd_pcm_t *pcm);
int snd_pcm_generic_munmap(snd_pcm_t *pcm);
