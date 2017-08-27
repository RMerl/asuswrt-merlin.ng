/*
 *  PCM Interface - mmap
 *  Copyright (c) 2000 by Abramo Bagnara <abramo@alsa-project.org>
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
#include <malloc.h>
#include <string.h>
#include <sys/poll.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include "pcm_local.h"

size_t page_size(void)
{
	long s = sysconf(_SC_PAGE_SIZE);
	assert(s > 0);
	return s;
}

size_t page_align(size_t size)
{
	size_t r;
	long psz = page_size();
	r = size % psz;
	if (r)
		return size + psz - r;
	return size;
}

size_t page_ptr(size_t object_offset, size_t object_size, size_t *offset, size_t *mmap_offset)
{
	size_t r;
	long psz = page_size();
	assert(offset);
	assert(mmap_offset);
	*mmap_offset = object_offset;
	object_offset %= psz;
	*mmap_offset -= object_offset;
	object_size += object_offset;
	r = object_size % psz;
	if (r)
		r = object_size + psz - r;
	else
		r = object_size;
	*offset = object_offset;
	return r;
}

void snd_pcm_mmap_appl_backward(snd_pcm_t *pcm, snd_pcm_uframes_t frames)
{
	snd_pcm_sframes_t appl_ptr = *pcm->appl.ptr;
	appl_ptr -= frames;
	if (appl_ptr < 0)
		appl_ptr += pcm->boundary;
	*pcm->appl.ptr = appl_ptr;
}

void snd_pcm_mmap_appl_forward(snd_pcm_t *pcm, snd_pcm_uframes_t frames)
{
	snd_pcm_uframes_t appl_ptr = *pcm->appl.ptr;
	appl_ptr += frames;
	if (appl_ptr >= pcm->boundary)
		appl_ptr -= pcm->boundary;
	*pcm->appl.ptr = appl_ptr;
}

void snd_pcm_mmap_hw_backward(snd_pcm_t *pcm, snd_pcm_uframes_t frames)
{
	snd_pcm_sframes_t hw_ptr = *pcm->hw.ptr;
	hw_ptr -= frames;
	if (hw_ptr < 0)
		hw_ptr += pcm->boundary;
	*pcm->hw.ptr = hw_ptr;
}

void snd_pcm_mmap_hw_forward(snd_pcm_t *pcm, snd_pcm_uframes_t frames)
{
	snd_pcm_uframes_t hw_ptr = *pcm->hw.ptr;
	hw_ptr += frames;
	if (hw_ptr >= pcm->boundary)
		hw_ptr -= pcm->boundary;
	*pcm->hw.ptr = hw_ptr;
}

static snd_pcm_sframes_t snd_pcm_mmap_write_areas(snd_pcm_t *pcm,
						  const snd_pcm_channel_area_t *areas,
						  snd_pcm_uframes_t offset,
						  snd_pcm_uframes_t size)
{
	snd_pcm_uframes_t xfer = 0;

	if (snd_pcm_mmap_playback_avail(pcm) < size) {
		SNDMSG("too short avail %ld to size %ld", snd_pcm_mmap_playback_avail(pcm), size);
		return -EPIPE;
	}
	while (size > 0) {
		const snd_pcm_channel_area_t *pcm_areas;
		snd_pcm_uframes_t pcm_offset;
		snd_pcm_uframes_t frames = size;
		snd_pcm_sframes_t result;

		snd_pcm_mmap_begin(pcm, &pcm_areas, &pcm_offset, &frames);
		snd_pcm_areas_copy(pcm_areas, pcm_offset,
				   areas, offset, 
				   pcm->channels, 
				   frames, pcm->format);
		result = snd_pcm_mmap_commit(pcm, pcm_offset, frames);
		if (result < 0)
			return xfer > 0 ? (snd_pcm_sframes_t)xfer : result;
		offset += result;
		xfer += result;
		size -= result;
	}
	return (snd_pcm_sframes_t)xfer;
}

static snd_pcm_sframes_t snd_pcm_mmap_read_areas(snd_pcm_t *pcm,
						 const snd_pcm_channel_area_t *areas,
						 snd_pcm_uframes_t offset,
						 snd_pcm_uframes_t size)
{
	snd_pcm_uframes_t xfer = 0;

	if (snd_pcm_mmap_capture_avail(pcm) < size) {
		SNDMSG("too short avail %ld to size %ld", snd_pcm_mmap_capture_avail(pcm), size);
		return -EPIPE;
	}
	while (size > 0) {
		const snd_pcm_channel_area_t *pcm_areas;
		snd_pcm_uframes_t pcm_offset;
		snd_pcm_uframes_t frames = size;
		snd_pcm_sframes_t result;
		
		snd_pcm_mmap_begin(pcm, &pcm_areas, &pcm_offset, &frames);
		snd_pcm_areas_copy(areas, offset,
				   pcm_areas, pcm_offset,
				   pcm->channels, 
				   frames, pcm->format);
		result = snd_pcm_mmap_commit(pcm, pcm_offset, frames);
		if (result < 0)
			return xfer > 0 ? (snd_pcm_sframes_t)xfer : result;
		offset += result;
		xfer += result;
		size -= result;
	}
	return (snd_pcm_sframes_t)xfer;
}

/**
 * \brief Write interleaved frames to a PCM using direct buffer (mmap)
 * \param pcm PCM handle
 * \param buffer frames containing buffer
 * \param size frames to be written
 * \return a positive number of frames actually written otherwise a
 * negative error code
 * \retval -EBADFD PCM is not in the right state (#SND_PCM_STATE_PREPARED or #SND_PCM_STATE_RUNNING)
 * \retval -EPIPE an underrun occurred
 * \retval -ESTRPIPE a suspend event occurred (stream is suspended and waiting for an application recovery)
 *
 * If the blocking behaviour is selected, then routine waits until
 * all requested bytes are played or put to the playback ring buffer.
 * The count of bytes can be less only if a signal or underrun occurred.
 *
 * If the non-blocking behaviour is selected, then routine doesn't wait at all.
 */
snd_pcm_sframes_t snd_pcm_mmap_writei(snd_pcm_t *pcm, const void *buffer, snd_pcm_uframes_t size)
{
	snd_pcm_channel_area_t areas[pcm->channels];
	snd_pcm_areas_from_buf(pcm, areas, (void*)buffer);
	return snd_pcm_write_areas(pcm, areas, 0, size,
				   snd_pcm_mmap_write_areas);
}

/**
 * \brief Write non interleaved frames to a PCM using direct buffer (mmap)
 * \param pcm PCM handle
 * \param bufs frames containing buffers (one for each channel)
 * \param size frames to be written
 * \return a positive number of frames actually written otherwise a
 * negative error code
 * \retval -EBADFD PCM is not in the right state (#SND_PCM_STATE_PREPARED or #SND_PCM_STATE_RUNNING)
 * \retval -EPIPE an underrun occurred
 * \retval -ESTRPIPE a suspend event occurred (stream is suspended and waiting for an application recovery)
 *
 * If the blocking behaviour is selected, then routine waits until
 * all requested bytes are played or put to the playback ring buffer.
 * The count of bytes can be less only if a signal or underrun occurred.
 *
 * If the non-blocking behaviour is selected, then routine doesn't wait at all.
 */
snd_pcm_sframes_t snd_pcm_mmap_writen(snd_pcm_t *pcm, void **bufs, snd_pcm_uframes_t size)
{
	snd_pcm_channel_area_t areas[pcm->channels];
	snd_pcm_areas_from_bufs(pcm, areas, bufs);
	return snd_pcm_write_areas(pcm, areas, 0, size,
				   snd_pcm_mmap_write_areas);
}

/**
 * \brief Read interleaved frames from a PCM using direct buffer (mmap)
 * \param pcm PCM handle
 * \param buffer frames containing buffer
 * \param size frames to be written
 * \return a positive number of frames actually read otherwise a
 * negative error code
 * \retval -EBADFD PCM is not in the right state (#SND_PCM_STATE_PREPARED or #SND_PCM_STATE_RUNNING)
 * \retval -EPIPE an overrun occurred
 * \retval -ESTRPIPE a suspend event occurred (stream is suspended and waiting for an application recovery)
 *
 * If the blocking behaviour was selected, then routine waits until
 * all requested bytes are filled. The count of bytes can be less only
 * if a signal or underrun occurred.
 *
 * If the non-blocking behaviour is selected, then routine doesn't wait at all.
 */
snd_pcm_sframes_t snd_pcm_mmap_readi(snd_pcm_t *pcm, void *buffer, snd_pcm_uframes_t size)
{
	snd_pcm_channel_area_t areas[pcm->channels];
	snd_pcm_areas_from_buf(pcm, areas, buffer);
	return snd_pcm_read_areas(pcm, areas, 0, size,
				  snd_pcm_mmap_read_areas);
}

/**
 * \brief Read non interleaved frames to a PCM using direct buffer (mmap)
 * \param pcm PCM handle
 * \param bufs frames containing buffers (one for each channel)
 * \param size frames to be written
 * \return a positive number of frames actually read otherwise a
 * negative error code
 * \retval -EBADFD PCM is not in the right state (#SND_PCM_STATE_PREPARED or #SND_PCM_STATE_RUNNING)
 * \retval -EPIPE an overrun occurred
 * \retval -ESTRPIPE a suspend event occurred (stream is suspended and waiting for an application recovery)
 *
 * If the blocking behaviour was selected, then routine waits until
 * all requested bytes are filled. The count of bytes can be less only
 * if a signal or underrun occurred.
 *
 * If the non-blocking behaviour is selected, then routine doesn't wait at all.
 */
snd_pcm_sframes_t snd_pcm_mmap_readn(snd_pcm_t *pcm, void **bufs, snd_pcm_uframes_t size)
{
	snd_pcm_channel_area_t areas[pcm->channels];
	snd_pcm_areas_from_bufs(pcm, areas, bufs);
	return snd_pcm_read_areas(pcm, areas, 0, size,
				  snd_pcm_mmap_read_areas);
}

int snd_pcm_channel_info_shm(snd_pcm_t *pcm, snd_pcm_channel_info_t *info, int shmid)
{
	switch (pcm->access) {
	case SND_PCM_ACCESS_MMAP_INTERLEAVED:
	case SND_PCM_ACCESS_RW_INTERLEAVED:
		info->first = info->channel * pcm->sample_bits;
		info->step = pcm->frame_bits;
		break;
	case SND_PCM_ACCESS_MMAP_NONINTERLEAVED:
	case SND_PCM_ACCESS_RW_NONINTERLEAVED:
		info->first = 0;
		info->step = pcm->sample_bits;
		break;
	default:
		SNDMSG("invalid access type %d", pcm->access);
		return -EINVAL;
	}
	info->addr = 0;
	if (pcm->hw_flags & SND_PCM_HW_PARAMS_EXPORT_BUFFER) {
		info->type = SND_PCM_AREA_SHM;
		info->u.shm.shmid = shmid;
		info->u.shm.area = NULL;
	} else
		info->type = SND_PCM_AREA_LOCAL;
	return 0;
}	

int snd_pcm_mmap(snd_pcm_t *pcm)
{
	int err;
	unsigned int c;
	assert(pcm);
	if (CHECK_SANITY(! pcm->setup)) {
		SNDMSG("PCM not set up");
		return -EIO;
	}
	if (CHECK_SANITY(pcm->mmap_channels || pcm->running_areas)) {
		SNDMSG("Already mmapped");
		return -EBUSY;
	}
	err = pcm->ops->mmap(pcm);
	if (err < 0)
		return err;
	if (pcm->mmap_shadow)
		return 0;
	pcm->mmap_channels = calloc(pcm->channels, sizeof(pcm->mmap_channels[0]));
	if (!pcm->mmap_channels)
		return -ENOMEM;
	pcm->running_areas = calloc(pcm->channels, sizeof(pcm->running_areas[0]));
	if (!pcm->running_areas) {
		free(pcm->mmap_channels);
		pcm->mmap_channels = NULL;
		return -ENOMEM;
	}
	for (c = 0; c < pcm->channels; ++c) {
		snd_pcm_channel_info_t *i = &pcm->mmap_channels[c];
		i->channel = c;
		err = snd_pcm_channel_info(pcm, i);
		if (err < 0)
			return err;
	}
	for (c = 0; c < pcm->channels; ++c) {
		snd_pcm_channel_info_t *i = &pcm->mmap_channels[c];
		snd_pcm_channel_area_t *a = &pcm->running_areas[c];
		char *ptr;
		size_t size;
		unsigned int c1;
		if (i->addr) {
        		a->addr = i->addr;
        		a->first = i->first;
        		a->step = i->step;
		        continue;
                }
                size = i->first + i->step * (pcm->buffer_size - 1) + pcm->sample_bits;
		for (c1 = c + 1; c1 < pcm->channels; ++c1) {
			snd_pcm_channel_info_t *i1 = &pcm->mmap_channels[c1];
			size_t s;
			if (i1->type != i->type)
				continue;
			switch (i1->type) {
			case SND_PCM_AREA_MMAP:
				if (i1->u.mmap.fd != i->u.mmap.fd ||
				    i1->u.mmap.offset != i->u.mmap.offset)
					continue;
				break;
			case SND_PCM_AREA_SHM:
				if (i1->u.shm.shmid != i->u.shm.shmid)
					continue;
				break;
			case SND_PCM_AREA_LOCAL:
				break;
			default:
				assert(0);
			}
			s = i1->first + i1->step * (pcm->buffer_size - 1) + pcm->sample_bits;
			if (s > size)
				size = s;
		}
		size = (size + 7) / 8;
		size = page_align(size);
		switch (i->type) {
		case SND_PCM_AREA_MMAP:
			ptr = mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_FILE|MAP_SHARED, i->u.mmap.fd, i->u.mmap.offset);
			if (ptr == MAP_FAILED) {
				SYSERR("mmap failed");
				return -errno;
			}
			i->addr = ptr;
			break;
		case SND_PCM_AREA_SHM:
			if (i->u.shm.shmid < 0) {
				int id;
				id = shmget(IPC_PRIVATE, size, 0666);
				if (id < 0) {
					SYSERR("shmget failed");
					return -errno;
				}
				i->u.shm.shmid = id;
				ptr = shmat(i->u.shm.shmid, 0, 0);
				if (ptr == (void *) -1) {
					SYSERR("shmat failed");
					return -errno;
				}
				/* automatically remove segment if not used */
				if (shmctl(id, IPC_RMID, NULL) < 0){
					SYSERR("shmctl mark remove failed");
					return -errno;
				}
				i->u.shm.area = snd_shm_area_create(id, ptr);
				if (i->u.shm.area == NULL) {
					SYSERR("snd_shm_area_create failed");
					return -ENOMEM;
				}
				if (pcm->access == SND_PCM_ACCESS_MMAP_INTERLEAVED ||
				    pcm->access == SND_PCM_ACCESS_RW_INTERLEAVED) {
					unsigned int c1;
					for (c1 = c + 1; c1 < pcm->channels; c1++) {
						snd_pcm_channel_info_t *i1 = &pcm->mmap_channels[c1];
						if (i1->u.shm.shmid < 0) {
							i1->u.shm.shmid = id;
							i1->u.shm.area = snd_shm_area_share(i->u.shm.area);
						}
					}
				}
			} else {
				ptr = shmat(i->u.shm.shmid, 0, 0);
				if (ptr == (void*) -1) {
					SYSERR("shmat failed");
					return -errno;
				}
			}
			i->addr = ptr;
			break;
		case SND_PCM_AREA_LOCAL:
			ptr = malloc(size);
			if (ptr == NULL) {
				SYSERR("malloc failed");
				return -errno;
			}
			i->addr = ptr;
			break;
		default:
			assert(0);
		}
		for (c1 = c + 1; c1 < pcm->channels; ++c1) {
			snd_pcm_channel_info_t *i1 = &pcm->mmap_channels[c1];
			if (i1->type != i->type)
				continue;
			switch (i1->type) {
			case SND_PCM_AREA_MMAP:
				if (i1->u.mmap.fd != i->u.mmap.fd ||
                                    i1->u.mmap.offset != i->u.mmap.offset)
					continue;
				break;
			case SND_PCM_AREA_SHM:
				if (i1->u.shm.shmid != i->u.shm.shmid)
					continue;
				/* follow thru */
			case SND_PCM_AREA_LOCAL:
				if (pcm->access != SND_PCM_ACCESS_MMAP_INTERLEAVED &&
				    pcm->access != SND_PCM_ACCESS_RW_INTERLEAVED)
				        continue;
				break;
			default:
				assert(0);
			}
			i1->addr = i->addr;
		}
		a->addr = i->addr;
		a->first = i->first;
		a->step = i->step;
	}
	return 0;
}

int snd_pcm_munmap(snd_pcm_t *pcm)
{
	int err;
	unsigned int c;
	assert(pcm);
	if (CHECK_SANITY(! pcm->mmap_channels)) {
		SNDMSG("Not mmapped");
		return -ENXIO;
	}
	if (pcm->mmap_shadow)
		return pcm->ops->munmap(pcm);
	for (c = 0; c < pcm->channels; ++c) {
		snd_pcm_channel_info_t *i = &pcm->mmap_channels[c];
		unsigned int c1;
		size_t size = i->first + i->step * (pcm->buffer_size - 1) + pcm->sample_bits;
		if (!i->addr)
			continue;
		for (c1 = c + 1; c1 < pcm->channels; ++c1) {
			snd_pcm_channel_info_t *i1 = &pcm->mmap_channels[c1];
			size_t s;
			if (i1->addr != i->addr)
				continue;
			i1->addr = NULL;
			s = i1->first + i1->step * (pcm->buffer_size - 1) + pcm->sample_bits;
			if (s > size)
				size = s;
		}
		size = (size + 7) / 8;
		size = page_align(size);
		switch (i->type) {
		case SND_PCM_AREA_MMAP:
			err = munmap(i->addr, size);
			if (err < 0) {
				SYSERR("mmap failed");
				return -errno;
			}
			errno = 0;
			break;
		case SND_PCM_AREA_SHM:
			if (i->u.shm.area) {
				snd_shm_area_destroy(i->u.shm.area);
				i->u.shm.area = NULL;
				if (pcm->access == SND_PCM_ACCESS_MMAP_INTERLEAVED ||
				    pcm->access == SND_PCM_ACCESS_RW_INTERLEAVED) {
					unsigned int c1;
					for (c1 = c + 1; c1 < pcm->channels; c1++) {
						snd_pcm_channel_info_t *i1 = &pcm->mmap_channels[c1];
						if (i1->u.shm.area) {
							snd_shm_area_destroy(i1->u.shm.area);
							i1->u.shm.area = NULL;
						}
					}
				}
			}
			break;
		case SND_PCM_AREA_LOCAL:
			free(i->addr);
			break;
		default:
			assert(0);
		}
		i->addr = NULL;
	}
	err = pcm->ops->munmap(pcm);
	if (err < 0)
		return err;
	free(pcm->mmap_channels);
	free(pcm->running_areas);
	pcm->mmap_channels = NULL;
	pcm->running_areas = NULL;
	return 0;
}

snd_pcm_sframes_t snd_pcm_write_mmap(snd_pcm_t *pcm, snd_pcm_uframes_t offset,
				     snd_pcm_uframes_t size)
{
	snd_pcm_uframes_t xfer = 0;
	snd_pcm_sframes_t err = 0;
	if (! size)
		return 0;
	while (xfer < size) {
		snd_pcm_uframes_t frames = size - xfer;
		snd_pcm_uframes_t cont = pcm->buffer_size - offset;
		if (cont < frames)
			frames = cont;
		switch (pcm->access) {
		case SND_PCM_ACCESS_MMAP_INTERLEAVED:
		{
			const snd_pcm_channel_area_t *a = snd_pcm_mmap_areas(pcm);
			const char *buf = snd_pcm_channel_area_addr(a, offset);
			err = _snd_pcm_writei(pcm, buf, frames);
			if (err >= 0)
				frames = err;
			break;
		}
		case SND_PCM_ACCESS_MMAP_NONINTERLEAVED:
		{
			unsigned int channels = pcm->channels;
			unsigned int c;
			void *bufs[channels];
			const snd_pcm_channel_area_t *areas = snd_pcm_mmap_areas(pcm);
			for (c = 0; c < channels; ++c) {
				const snd_pcm_channel_area_t *a = &areas[c];
				bufs[c] = snd_pcm_channel_area_addr(a, offset);
			}
			err = _snd_pcm_writen(pcm, bufs, frames);
			if (err >= 0)
				frames = err;
			break;
		}
		default:
			SNDMSG("invalid access type %d", pcm->access);
			return -EINVAL;
		}
		if (err < 0)
			break;
		xfer += frames;
		offset = (offset + frames) % pcm->buffer_size;
	}
	if (xfer > 0)
		return xfer;
	return err;
}

snd_pcm_sframes_t snd_pcm_read_mmap(snd_pcm_t *pcm, snd_pcm_uframes_t offset,
				    snd_pcm_uframes_t size)
{
	snd_pcm_uframes_t xfer = 0;
	snd_pcm_sframes_t err = 0;
	if (! size)
		return 0;
	while (xfer < size) {
		snd_pcm_uframes_t frames = size - xfer;
		snd_pcm_uframes_t cont = pcm->buffer_size - offset;
		if (cont < frames)
			frames = cont;
		switch (pcm->access) {
		case SND_PCM_ACCESS_MMAP_INTERLEAVED:
		{
			const snd_pcm_channel_area_t *a = snd_pcm_mmap_areas(pcm);
			char *buf = snd_pcm_channel_area_addr(a, offset);
			err = _snd_pcm_readi(pcm, buf, frames);
			if (err >= 0)
				frames = err;
			break;
		}
		case SND_PCM_ACCESS_MMAP_NONINTERLEAVED:
		{
			snd_pcm_uframes_t channels = pcm->channels;
			unsigned int c;
			void *bufs[channels];
			const snd_pcm_channel_area_t *areas = snd_pcm_mmap_areas(pcm);
			for (c = 0; c < channels; ++c) {
				const snd_pcm_channel_area_t *a = &areas[c];
				bufs[c] = snd_pcm_channel_area_addr(a, offset);
			}
			err = _snd_pcm_readn(pcm->fast_op_arg, bufs, frames);
			if (err >= 0)
				frames = err;
		}
		default:
			SNDMSG("invalid access type %d", pcm->access);
			return -EINVAL;
		}
		if (err < 0)
			break;
		xfer += frames;
		offset = (offset + frames) % pcm->buffer_size;
	}
	if (xfer > 0)
		return xfer;
	return err;
}
