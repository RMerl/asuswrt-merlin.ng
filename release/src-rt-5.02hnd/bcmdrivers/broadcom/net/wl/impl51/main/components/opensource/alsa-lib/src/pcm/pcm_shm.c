/**
 * \file pcm/pcm_shm.c
 * \ingroup PCM_Plugins
 * \brief PCM Shared Memory Plugin Interface
 * \author Abramo Bagnara <abramo@alsa-project.org>
 * \date 2000-2001
 */
/*
 *  PCM - Shared Memory Client
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
#include <limits.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/shm.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <sys/un.h>
#include <sys/mman.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <netdb.h>
#include "aserver.h"

#ifndef PIC
/* entry for static linking */
const char *_snd_module_pcm_shm = "";
#endif

#ifndef DOC_HIDDEN
typedef struct {
	int socket;
	volatile snd_pcm_shm_ctrl_t *ctrl;
} snd_pcm_shm_t;
#endif

static long snd_pcm_shm_action_fd0(snd_pcm_t *pcm, int *fd)
{
	snd_pcm_shm_t *shm = pcm->private_data;
	int err;
	char buf[1];
	volatile snd_pcm_shm_ctrl_t *ctrl = shm->ctrl;

	err = write(shm->socket, buf, 1);
	if (err != 1)
		return -EBADFD;
	err = snd_receive_fd(shm->socket, buf, 1, fd);
	if (err != 1)
		return -EBADFD;
	if (ctrl->cmd) {
		SNDERR("Server has not done the cmd");
		return -EBADFD;
	}
	return ctrl->result;
}

static int snd_pcm_shm_new_rbptr(snd_pcm_t *pcm, snd_pcm_shm_t *shm,
				 snd_pcm_rbptr_t *rbptr, volatile snd_pcm_shm_rbptr_t *shm_rbptr)
{
	if (!shm_rbptr->use_mmap) {
		if (&pcm->hw == rbptr)
			snd_pcm_set_hw_ptr(pcm, &shm_rbptr->ptr, -1, 0);
		else
			snd_pcm_set_appl_ptr(pcm, &shm_rbptr->ptr, -1, 0);
	} else {
		void *ptr;
		size_t mmap_size, mmap_offset, offset;
		int fd;
		long result;
		
		shm->ctrl->cmd = &pcm->hw == rbptr ? SND_PCM_IOCTL_HW_PTR_FD : SND_PCM_IOCTL_APPL_PTR_FD;
		result = snd_pcm_shm_action_fd0(pcm, &fd);
		if (result < 0)
			return result;
		mmap_size = page_ptr(shm_rbptr->offset, sizeof(snd_pcm_uframes_t), &offset, &mmap_offset);
		ptr = mmap(NULL, mmap_size, PROT_READ|PROT_WRITE, MAP_FILE|MAP_SHARED, fd, mmap_offset);
		if (ptr == MAP_FAILED || ptr == NULL) {
			SYSERR("shm rbptr mmap failed");
			return -errno;
		}
		if (&pcm->hw == rbptr)
			snd_pcm_set_hw_ptr(pcm, (snd_pcm_uframes_t *)((char *)ptr + offset), fd, shm_rbptr->offset);
		else
			snd_pcm_set_appl_ptr(pcm, (snd_pcm_uframes_t *)((char *)ptr + offset), fd, shm_rbptr->offset);
	}
	return 0;
}

static long snd_pcm_shm_action(snd_pcm_t *pcm)
{
	snd_pcm_shm_t *shm = pcm->private_data;
	int err, result;
	char buf[1];
	volatile snd_pcm_shm_ctrl_t *ctrl = shm->ctrl;

	if (ctrl->hw.changed || ctrl->appl.changed)
		return -EBADFD;
	err = write(shm->socket, buf, 1);
	if (err != 1)
		return -EBADFD;
	err = read(shm->socket, buf, 1);
	if (err != 1)
		return -EBADFD;
	if (ctrl->cmd) {
		SNDERR("Server has not done the cmd");
		return -EBADFD;
	}
	result = ctrl->result;
	if (ctrl->hw.changed) {
		err = snd_pcm_shm_new_rbptr(pcm, shm, &pcm->hw, &ctrl->hw);
		if (err < 0)
			return err;
		ctrl->hw.changed = 0;
	}
	if (ctrl->appl.changed) {
		err = snd_pcm_shm_new_rbptr(pcm, shm, &pcm->appl, &ctrl->appl);
		if (err < 0)
			return err;
		ctrl->appl.changed = 0;
	}
	return result;
}

static long snd_pcm_shm_action_fd(snd_pcm_t *pcm, int *fd)
{
	snd_pcm_shm_t *shm = pcm->private_data;
	int err;
	char buf[1];
	volatile snd_pcm_shm_ctrl_t *ctrl = shm->ctrl;

	if (ctrl->hw.changed || ctrl->appl.changed)
		return -EBADFD;
	err = write(shm->socket, buf, 1);
	if (err != 1)
		return -EBADFD;
	err = snd_receive_fd(shm->socket, buf, 1, fd);
	if (err != 1)
		return -EBADFD;
	if (ctrl->cmd) {
		SNDERR("Server has not done the cmd");
		return -EBADFD;
	}
	if (ctrl->hw.changed) {
		err = snd_pcm_shm_new_rbptr(pcm, shm, &pcm->hw, &ctrl->hw);
		if (err < 0)
			return err;
		ctrl->hw.changed = 0;
	}
	if (ctrl->appl.changed) {
		err = snd_pcm_shm_new_rbptr(pcm, shm, &pcm->appl, &ctrl->appl);
		if (err < 0)
			return err;
		ctrl->appl.changed = 0;
	}
	return ctrl->result;
}

static int snd_pcm_shm_nonblock(snd_pcm_t *pcm ATTRIBUTE_UNUSED, int nonblock ATTRIBUTE_UNUSED)
{
	return 0;
}

static int snd_pcm_shm_async(snd_pcm_t *pcm, int sig, pid_t pid)
{
	snd_pcm_shm_t *shm = pcm->private_data;
	volatile snd_pcm_shm_ctrl_t *ctrl = shm->ctrl;
	ctrl->cmd = SND_PCM_IOCTL_ASYNC;
	ctrl->u.async.sig = sig;
	ctrl->u.async.pid = pid;
	return snd_pcm_shm_action(pcm);
}

static int snd_pcm_shm_info(snd_pcm_t *pcm, snd_pcm_info_t * info)
{
	snd_pcm_shm_t *shm = pcm->private_data;
	volatile snd_pcm_shm_ctrl_t *ctrl = shm->ctrl;
	int err;
//	ctrl->u.info = *info;
	ctrl->cmd = SNDRV_PCM_IOCTL_INFO;
	err = snd_pcm_shm_action(pcm);
	if (err < 0)
		return err;
	*info = ctrl->u.info;
	return err;
}

static int snd_pcm_shm_hw_refine_cprepare(snd_pcm_t *pcm ATTRIBUTE_UNUSED, snd_pcm_hw_params_t *params ATTRIBUTE_UNUSED)
{
	return 0;
}

static int snd_pcm_shm_hw_refine_sprepare(snd_pcm_t *pcm ATTRIBUTE_UNUSED, snd_pcm_hw_params_t *sparams)
{
	snd_pcm_access_mask_t saccess_mask = { SND_PCM_ACCBIT_MMAP };
	_snd_pcm_hw_params_any(sparams);
	_snd_pcm_hw_param_set_mask(sparams, SND_PCM_HW_PARAM_ACCESS,
				   &saccess_mask);
	return 0;
}

static int snd_pcm_shm_hw_refine_schange(snd_pcm_t *pcm ATTRIBUTE_UNUSED, snd_pcm_hw_params_t *params,
					  snd_pcm_hw_params_t *sparams)
{
	int err;
	unsigned int links = ~SND_PCM_HW_PARBIT_ACCESS;
	const snd_pcm_access_mask_t *access_mask = snd_pcm_hw_param_get_mask(params, SND_PCM_HW_PARAM_ACCESS);
	if (!snd_pcm_access_mask_test(access_mask, SND_PCM_ACCESS_RW_INTERLEAVED) &&
	    !snd_pcm_access_mask_test(access_mask, SND_PCM_ACCESS_RW_NONINTERLEAVED)) {
		err = _snd_pcm_hw_param_set_mask(sparams, SND_PCM_HW_PARAM_ACCESS,
					     access_mask);
		if (err < 0)
			return err;
	}
	err = _snd_pcm_hw_params_refine(sparams, links, params);
	if (err < 0)
		return err;
	return 0;
}
	
static int snd_pcm_shm_hw_refine_cchange(snd_pcm_t *pcm ATTRIBUTE_UNUSED, snd_pcm_hw_params_t *params,
					  snd_pcm_hw_params_t *sparams)
{
	int err;
	unsigned int links = ~SND_PCM_HW_PARBIT_ACCESS;
	snd_pcm_access_mask_t access_mask;
	snd_mask_copy(&access_mask, snd_pcm_hw_param_get_mask(sparams, SND_PCM_HW_PARAM_ACCESS));
	snd_pcm_access_mask_set(&access_mask, SND_PCM_ACCESS_RW_INTERLEAVED);
	snd_pcm_access_mask_set(&access_mask, SND_PCM_ACCESS_RW_NONINTERLEAVED);
	err = _snd_pcm_hw_param_set_mask(sparams, SND_PCM_HW_PARAM_ACCESS,
					 &access_mask);
	if (err < 0)
		return err;
	err = _snd_pcm_hw_params_refine(params, links, sparams);
	if (err < 0)
		return err;
	return 0;
}

static int snd_pcm_shm_hw_refine_slave(snd_pcm_t *pcm,
				       snd_pcm_hw_params_t *params)
{
	snd_pcm_shm_t *shm = pcm->private_data;
	volatile snd_pcm_shm_ctrl_t *ctrl = shm->ctrl;
	int err;
	ctrl->u.hw_refine = *params;
	ctrl->cmd = SNDRV_PCM_IOCTL_HW_REFINE;
	err = snd_pcm_shm_action(pcm);
	*params = ctrl->u.hw_refine;
	return err;
}

static int snd_pcm_shm_hw_refine(snd_pcm_t *pcm, snd_pcm_hw_params_t *params)
{
	return snd_pcm_hw_refine_slave(pcm, params,
				       snd_pcm_shm_hw_refine_cprepare,
				       snd_pcm_shm_hw_refine_cchange,
				       snd_pcm_shm_hw_refine_sprepare,
				       snd_pcm_shm_hw_refine_schange,
				       snd_pcm_shm_hw_refine_slave);
}

static int snd_pcm_shm_hw_params_slave(snd_pcm_t *pcm, 
				       snd_pcm_hw_params_t *params)
{
	snd_pcm_shm_t *shm = pcm->private_data;
	volatile snd_pcm_shm_ctrl_t *ctrl = shm->ctrl;
	int err;
	params->flags |= SND_PCM_HW_PARAMS_EXPORT_BUFFER;
	ctrl->cmd = SNDRV_PCM_IOCTL_HW_PARAMS;
	ctrl->u.hw_params = *params;
	err = snd_pcm_shm_action(pcm);
	*params = ctrl->u.hw_params;
	return err;
}

static int snd_pcm_shm_hw_params(snd_pcm_t *pcm, snd_pcm_hw_params_t * params)
{
	return snd_pcm_hw_params_slave(pcm, params,
				       snd_pcm_shm_hw_refine_cchange,
				       snd_pcm_shm_hw_refine_sprepare,
				       snd_pcm_shm_hw_refine_schange,
				       snd_pcm_shm_hw_params_slave);
}

static int snd_pcm_shm_hw_free(snd_pcm_t *pcm)
{
	snd_pcm_shm_t *shm = pcm->private_data;
	volatile snd_pcm_shm_ctrl_t *ctrl = shm->ctrl;
	ctrl->cmd = SNDRV_PCM_IOCTL_HW_FREE;
	return snd_pcm_shm_action(pcm);
}

static int snd_pcm_shm_sw_params(snd_pcm_t *pcm, snd_pcm_sw_params_t * params)
{
	snd_pcm_shm_t *shm = pcm->private_data;
	volatile snd_pcm_shm_ctrl_t *ctrl = shm->ctrl;
	int err;
	ctrl->cmd = SNDRV_PCM_IOCTL_SW_PARAMS;
	ctrl->u.sw_params = *params;
	err = snd_pcm_shm_action(pcm);
	*params = ctrl->u.sw_params;
	if (err < 0)
		return err;
	return err;
}

static int snd_pcm_shm_mmap(snd_pcm_t *pcm ATTRIBUTE_UNUSED)
{
	return 0;
}

static int snd_pcm_shm_munmap(snd_pcm_t *pcm)
{
	unsigned int c;
	for (c = 0; c < pcm->channels; ++c) {
		snd_pcm_channel_info_t *i = &pcm->mmap_channels[c];
		unsigned int c1;
		int err;
		if (i->type != SND_PCM_AREA_MMAP)
			continue;
		if (i->u.mmap.fd < 0)
			continue;
		for (c1 = c + 1; c1 < pcm->channels; ++c1) {
			snd_pcm_channel_info_t *i1 = &pcm->mmap_channels[c1];
			if (i1->type != SND_PCM_AREA_MMAP)
				continue;
			if (i1->u.mmap.fd != i->u.mmap.fd)
				continue;
			i1->u.mmap.fd = -1;
		}
		err = close(i->u.mmap.fd);
		if (err < 0) {
			SYSERR("close failed");
			return -errno;
		}
	}
	return 0;
}

static int snd_pcm_shm_channel_info(snd_pcm_t *pcm, snd_pcm_channel_info_t * info)
{
	snd_pcm_shm_t *shm = pcm->private_data;
	volatile snd_pcm_shm_ctrl_t *ctrl = shm->ctrl;
	int err;
	int fd;
	ctrl->cmd = SNDRV_PCM_IOCTL_CHANNEL_INFO;
	ctrl->u.channel_info = *info;
	err = snd_pcm_shm_action_fd(pcm, &fd);
	if (err < 0)
		return err;
	*info = ctrl->u.channel_info;
	info->addr = 0;
	switch (info->type) {
	case SND_PCM_AREA_MMAP:
		info->u.mmap.fd = fd;
		break;
	case SND_PCM_AREA_SHM:
		break;
	default:
		assert(0);
		break;
	}
	return err;
}

static int snd_pcm_shm_status(snd_pcm_t *pcm, snd_pcm_status_t * status)
{
	snd_pcm_shm_t *shm = pcm->private_data;
	volatile snd_pcm_shm_ctrl_t *ctrl = shm->ctrl;
	int err;
	ctrl->cmd = SNDRV_PCM_IOCTL_STATUS;
	// ctrl->u.status = *status;
	err = snd_pcm_shm_action(pcm);
	if (err < 0)
		return err;
	*status = ctrl->u.status;
	return err;
}

static snd_pcm_state_t snd_pcm_shm_state(snd_pcm_t *pcm)
{
	snd_pcm_shm_t *shm = pcm->private_data;
	volatile snd_pcm_shm_ctrl_t *ctrl = shm->ctrl;
	ctrl->cmd = SND_PCM_IOCTL_STATE;
	return snd_pcm_shm_action(pcm);
}

static int snd_pcm_shm_hwsync(snd_pcm_t *pcm)
{
	snd_pcm_shm_t *shm = pcm->private_data;
	volatile snd_pcm_shm_ctrl_t *ctrl = shm->ctrl;
	ctrl->cmd = SND_PCM_IOCTL_HWSYNC;
	return snd_pcm_shm_action(pcm);
}

static int snd_pcm_shm_delay(snd_pcm_t *pcm, snd_pcm_sframes_t *delayp)
{
	snd_pcm_shm_t *shm = pcm->private_data;
	volatile snd_pcm_shm_ctrl_t *ctrl = shm->ctrl;
	int err;
	ctrl->cmd = SNDRV_PCM_IOCTL_DELAY;
	err = snd_pcm_shm_action(pcm);
	if (err < 0)
		return err;
	*delayp = ctrl->u.delay.frames;
	return err;
}

static snd_pcm_sframes_t snd_pcm_shm_avail_update(snd_pcm_t *pcm)
{
	snd_pcm_shm_t *shm = pcm->private_data;
	volatile snd_pcm_shm_ctrl_t *ctrl = shm->ctrl;
	int err;
	ctrl->cmd = SND_PCM_IOCTL_AVAIL_UPDATE;
	err = snd_pcm_shm_action(pcm);
	if (err < 0)
		return err;
	return err;
}

static int snd_pcm_shm_htimestamp(snd_pcm_t *pcm ATTRIBUTE_UNUSED,
				  snd_pcm_uframes_t *avail ATTRIBUTE_UNUSED,
				  snd_htimestamp_t *tstamp ATTRIBUTE_UNUSED)
{
	return -EIO;	/* not implemented yet */
}

static int snd_pcm_shm_prepare(snd_pcm_t *pcm)
{
	snd_pcm_shm_t *shm = pcm->private_data;
	volatile snd_pcm_shm_ctrl_t *ctrl = shm->ctrl;
	ctrl->cmd = SNDRV_PCM_IOCTL_PREPARE;
	return snd_pcm_shm_action(pcm);
}

static int snd_pcm_shm_reset(snd_pcm_t *pcm)
{
	snd_pcm_shm_t *shm = pcm->private_data;
	volatile snd_pcm_shm_ctrl_t *ctrl = shm->ctrl;
	ctrl->cmd = SNDRV_PCM_IOCTL_RESET;
	return snd_pcm_shm_action(pcm);
}

static int snd_pcm_shm_start(snd_pcm_t *pcm)
{
	snd_pcm_shm_t *shm = pcm->private_data;
	volatile snd_pcm_shm_ctrl_t *ctrl = shm->ctrl;
	ctrl->cmd = SNDRV_PCM_IOCTL_START;
	return snd_pcm_shm_action(pcm);
}

static int snd_pcm_shm_drop(snd_pcm_t *pcm)
{
	snd_pcm_shm_t *shm = pcm->private_data;
	volatile snd_pcm_shm_ctrl_t *ctrl = shm->ctrl;
	ctrl->cmd = SNDRV_PCM_IOCTL_DROP;
	return snd_pcm_shm_action(pcm);
}

static int snd_pcm_shm_drain(snd_pcm_t *pcm)
{
	snd_pcm_shm_t *shm = pcm->private_data;
	volatile snd_pcm_shm_ctrl_t *ctrl = shm->ctrl;
	int err;
	do {
		ctrl->cmd = SNDRV_PCM_IOCTL_DRAIN;
		err = snd_pcm_shm_action(pcm);
		if (err != -EAGAIN)
			break;
		usleep(10000);
	} while (1);
	if (err < 0)
		return err;
	if (!(pcm->mode & SND_PCM_NONBLOCK))
		snd_pcm_wait(pcm, -1);
	return err;
}

static int snd_pcm_shm_pause(snd_pcm_t *pcm, int enable)
{
	snd_pcm_shm_t *shm = pcm->private_data;
	volatile snd_pcm_shm_ctrl_t *ctrl = shm->ctrl;
	ctrl->cmd = SNDRV_PCM_IOCTL_PAUSE;
	ctrl->u.pause.enable = enable;
	return snd_pcm_shm_action(pcm);
}

static snd_pcm_sframes_t snd_pcm_shm_rewindable(snd_pcm_t *pcm ATTRIBUTE_UNUSED)
{
	return 0;	/* FIX ME */
}

static snd_pcm_sframes_t snd_pcm_shm_rewind(snd_pcm_t *pcm, snd_pcm_uframes_t frames)
{
	snd_pcm_shm_t *shm = pcm->private_data;
	volatile snd_pcm_shm_ctrl_t *ctrl = shm->ctrl;
	ctrl->cmd = SNDRV_PCM_IOCTL_REWIND;
	ctrl->u.rewind.frames = frames;
	return snd_pcm_shm_action(pcm);
}

static snd_pcm_sframes_t snd_pcm_shm_forwardable(snd_pcm_t *pcm ATTRIBUTE_UNUSED)
{
	return 0;	/* FIX ME */
}

static snd_pcm_sframes_t snd_pcm_shm_forward(snd_pcm_t *pcm, snd_pcm_uframes_t frames)
{
	snd_pcm_shm_t *shm = pcm->private_data;
	volatile snd_pcm_shm_ctrl_t *ctrl = shm->ctrl;
	ctrl->cmd = SND_PCM_IOCTL_FORWARD;
	ctrl->u.forward.frames = frames;
	return snd_pcm_shm_action(pcm);
}

static int snd_pcm_shm_resume(snd_pcm_t *pcm)
{
	snd_pcm_shm_t *shm = pcm->private_data;
	volatile snd_pcm_shm_ctrl_t *ctrl = shm->ctrl;
	ctrl->cmd = SNDRV_PCM_IOCTL_RESUME;
	return snd_pcm_shm_action(pcm);
}

static snd_pcm_sframes_t snd_pcm_shm_mmap_commit(snd_pcm_t *pcm,
						 snd_pcm_uframes_t offset ATTRIBUTE_UNUSED,
						 snd_pcm_uframes_t size)
{
	snd_pcm_shm_t *shm = pcm->private_data;
	volatile snd_pcm_shm_ctrl_t *ctrl = shm->ctrl;
	ctrl->cmd = SND_PCM_IOCTL_MMAP_COMMIT;
	ctrl->u.mmap_commit.offset = offset;
	ctrl->u.mmap_commit.frames = size;
	return snd_pcm_shm_action(pcm);
}

static int snd_pcm_shm_poll_descriptor(snd_pcm_t *pcm)
{
	snd_pcm_shm_t *shm = pcm->private_data;
	volatile snd_pcm_shm_ctrl_t *ctrl = shm->ctrl;
	int fd, err;
	ctrl->cmd = SND_PCM_IOCTL_POLL_DESCRIPTOR;
	err = snd_pcm_shm_action_fd(pcm, &fd);
	if (err < 0)
		return err;
	return fd;
}

static int snd_pcm_shm_close(snd_pcm_t *pcm)
{
	snd_pcm_shm_t *shm = pcm->private_data;
	volatile snd_pcm_shm_ctrl_t *ctrl = shm->ctrl;
	int result;
	ctrl->cmd = SND_PCM_IOCTL_CLOSE;
	result = snd_pcm_shm_action(pcm);
	shmdt((void *)ctrl);
	close(shm->socket);
	close(pcm->poll_fd);
	free(shm);
	return result;
}

static void snd_pcm_shm_dump(snd_pcm_t *pcm, snd_output_t *out)
{
	snd_output_printf(out, "Shm PCM\n");
	if (pcm->setup) {
		snd_output_printf(out, "Its setup is:\n");
		snd_pcm_dump_setup(pcm, out);
	}
}

static const snd_pcm_ops_t snd_pcm_shm_ops = {
	.close = snd_pcm_shm_close,
	.info = snd_pcm_shm_info,
	.hw_refine = snd_pcm_shm_hw_refine,
	.hw_params = snd_pcm_shm_hw_params,
	.hw_free = snd_pcm_shm_hw_free,
	.sw_params = snd_pcm_shm_sw_params,
	.channel_info = snd_pcm_shm_channel_info,
	.dump = snd_pcm_shm_dump,
	.nonblock = snd_pcm_shm_nonblock,
	.async = snd_pcm_shm_async,
	.mmap = snd_pcm_shm_mmap,
	.munmap = snd_pcm_shm_munmap,
};

static const snd_pcm_fast_ops_t snd_pcm_shm_fast_ops = {
	.status = snd_pcm_shm_status,
	.state = snd_pcm_shm_state,
	.hwsync = snd_pcm_shm_hwsync,
	.delay = snd_pcm_shm_delay,
	.prepare = snd_pcm_shm_prepare,
	.reset = snd_pcm_shm_reset,
	.start = snd_pcm_shm_start,
	.drop = snd_pcm_shm_drop,
	.drain = snd_pcm_shm_drain,
	.pause = snd_pcm_shm_pause,
	.rewindable = snd_pcm_shm_rewindable,
	.rewind = snd_pcm_shm_rewind,
	.forwardable = snd_pcm_shm_forwardable,
	.forward = snd_pcm_shm_forward,
	.resume = snd_pcm_shm_resume,
	.writei = snd_pcm_mmap_writei,
	.writen = snd_pcm_mmap_writen,
	.readi = snd_pcm_mmap_readi,
	.readn = snd_pcm_mmap_readn,
	.avail_update = snd_pcm_shm_avail_update,
	.mmap_commit = snd_pcm_shm_mmap_commit,
	.htimestamp = snd_pcm_shm_htimestamp,
};

static int make_local_socket(const char *filename)
{
	size_t l = strlen(filename);
	size_t size = offsetof(struct sockaddr_un, sun_path) + l;
	struct sockaddr_un *addr = alloca(size);
	int sock;

	sock = socket(PF_LOCAL, SOCK_STREAM, 0);
	if (sock < 0) {
		SYSERR("socket failed");
		return -errno;
	}
	
	addr->sun_family = AF_LOCAL;
	memcpy(addr->sun_path, filename, l);

	if (connect(sock, (struct sockaddr *) addr, size) < 0) {
		SYSERR("connect failed");
		return -errno;
	}
	return sock;
}


/**
 * \brief Creates a new shared memory PCM
 * \param pcmp Returns created PCM handle
 * \param name Name of PCM
 * \param sockname Unix socket name
 * \param sname Server name
 * \param stream PCM Stream
 * \param mode PCM Mode
 * \retval zero on success otherwise a negative error code
 * \warning Using of this function might be dangerous in the sense
 *          of compatibility reasons. The prototype might be freely
 *          changed in future.
 */
int snd_pcm_shm_open(snd_pcm_t **pcmp, const char *name,
		     const char *sockname, const char *sname,
		     snd_pcm_stream_t stream, int mode)
{
	snd_pcm_t *pcm;
	snd_pcm_shm_t *shm = NULL;
	snd_client_open_request_t *req;
	snd_client_open_answer_t ans;
	size_t snamelen, reqlen;
	int err;
	int result;
	snd_pcm_shm_ctrl_t *ctrl = NULL;
	int sock = -1;
	snamelen = strlen(sname);
	if (snamelen > 255)
		return -EINVAL;

	result = make_local_socket(sockname);
	if (result < 0) {
		SNDERR("server for socket %s is not running", sockname);
		goto _err;
	}
	sock = result;

	reqlen = sizeof(*req) + snamelen;
	req = alloca(reqlen);
	memcpy(req->name, sname, snamelen);
	req->dev_type = SND_DEV_TYPE_PCM;
	req->transport_type = SND_TRANSPORT_TYPE_SHM;
	req->stream = stream;
	req->mode = mode;
	req->namelen = snamelen;
	err = write(sock, req, reqlen);
	if (err < 0) {
		SYSERR("write error");
		result = -errno;
		goto _err;
	}
	if ((size_t) err != reqlen) {
		SNDERR("write size error");
		result = -EINVAL;
		goto _err;
	}
	err = read(sock, &ans, sizeof(ans));
	if (err < 0) {
		SYSERR("read error");
		result = -errno;
		goto _err;
	}
	if (err != sizeof(ans)) {
		SNDERR("read size error");
		result = -EINVAL;
		goto _err;
	}
	result = ans.result;
	if (result < 0)
		goto _err;

	ctrl = shmat(ans.cookie, 0, 0);
	if (!ctrl) {
		SYSERR("shmat error");
		result = -errno;
		goto _err;
	}
		
	shm = calloc(1, sizeof(snd_pcm_shm_t));
	if (!shm) {
		result = -ENOMEM;
		goto _err;
	}

	shm->socket = sock;
	shm->ctrl = ctrl;

	err = snd_pcm_new(&pcm, SND_PCM_TYPE_SHM, name, stream, mode);
	if (err < 0) {
		result = err;
		goto _err;
	}
	pcm->mmap_rw = 1;
	pcm->ops = &snd_pcm_shm_ops;
	pcm->fast_ops = &snd_pcm_shm_fast_ops;
	pcm->private_data = shm;
	err = snd_pcm_shm_poll_descriptor(pcm);
	if (err < 0) {
		snd_pcm_close(pcm);
		return err;
	}
	pcm->poll_fd = err;
	pcm->poll_events = stream == SND_PCM_STREAM_PLAYBACK ? POLLOUT : POLLIN;
	snd_pcm_set_hw_ptr(pcm, &ctrl->hw.ptr, -1, 0);
	snd_pcm_set_appl_ptr(pcm, &ctrl->appl.ptr, -1, 0);
	*pcmp = pcm;
	return 0;

 _err:
	close(sock);
	if (ctrl)
		shmdt(ctrl);
	free(shm);
	return result;
}

/*! \page pcm_plugins

\section pcm_plugins_shm Plugin: shm

This plugin communicates with aserver via shared memory. It is a raw
communication without any conversions, but it can be expected worse
performance.

\code
pcm.name {
        type shm                # Shared memory PCM
	server STR		# Server name
	pcm STR			# PCM name
}
\endcode

\subsection pcm_plugins_shm_funcref Function reference

<UL>
  <LI>snd_pcm_shm_open()
  <LI>_snd_pcm_shm_open()
</UL>

*/

/**
 * \brief Creates a new shm PCM
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
int _snd_pcm_shm_open(snd_pcm_t **pcmp, const char *name,
		      snd_config_t *root, snd_config_t *conf,
		      snd_pcm_stream_t stream, int mode)
{
	snd_config_iterator_t i, next;
	const char *server = NULL;
	const char *pcm_name = NULL;
	snd_config_t *sconfig;
	const char *host = NULL;
	const char *sockname = NULL;
	long port = -1;
	int err;
	int local;
	struct hostent *h;
	snd_config_for_each(i, next, conf) {
		snd_config_t *n = snd_config_iterator_entry(i);
		const char *id;
		if (snd_config_get_id(n, &id) < 0)
			continue;
		if (snd_pcm_conf_generic_id(id))
			continue;
		if (strcmp(id, "server") == 0) {
			err = snd_config_get_string(n, &server);
			if (err < 0) {
				SNDERR("Invalid type for %s", id);
				return -EINVAL;
			}
			continue;
		}
		if (strcmp(id, "pcm") == 0) {
			err = snd_config_get_string(n, &pcm_name);
			if (err < 0) {
				SNDERR("Invalid type for %s", id);
				return -EINVAL;
			}
			continue;
		}
		SNDERR("Unknown field %s", id);
		return -EINVAL;
	}
	if (!pcm_name) {
		SNDERR("pcm is not defined");
		return -EINVAL;
	}
	if (!server) {
		SNDERR("server is not defined");
		return -EINVAL;
	}
	err = snd_config_search_definition(root, "server", server, &sconfig);
	if (err < 0) {
		SNDERR("Unknown server %s", server);
		return -EINVAL;
	}
	if (snd_config_get_type(sconfig) != SND_CONFIG_TYPE_COMPOUND) {
		SNDERR("Invalid type for server %s definition", server);
		goto _err;
	}
	snd_config_for_each(i, next, sconfig) {
		snd_config_t *n = snd_config_iterator_entry(i);
		const char *id;
		if (snd_config_get_id(n, &id) < 0)
			continue;
		if (strcmp(id, "comment") == 0)
			continue;
		if (strcmp(id, "host") == 0) {
			err = snd_config_get_string(n, &host);
			if (err < 0) {
				SNDERR("Invalid type for %s", id);
				goto _err;
			}
			continue;
		}
		if (strcmp(id, "socket") == 0) {
			err = snd_config_get_string(n, &sockname);
			if (err < 0) {
				SNDERR("Invalid type for %s", id);
				goto _err;
			}
			continue;
		}
		if (strcmp(id, "port") == 0) {
			err = snd_config_get_integer(n, &port);
			if (err < 0) {
				SNDERR("Invalid type for %s", id);
				goto _err;
			}
			continue;
		}
		SNDERR("Unknown field %s", id);
	       _err:
		err = -EINVAL;
		goto __error;
	}

	if (!host) {
		SNDERR("host is not defined");
		goto _err;
	}
	if (!sockname) {
		SNDERR("socket is not defined");
		goto _err;
	}
	h = gethostbyname(host);
	if (!h) {
		SNDERR("Cannot resolve %s", host);
		goto _err;
	}
	local = snd_is_local(h);
	if (!local) {
		SNDERR("%s is not the local host", host);
		goto _err;
	}
	err = snd_pcm_shm_open(pcmp, name, sockname, pcm_name, stream, mode);
      __error:
	snd_config_delete(sconfig);
	return err;
}
#ifndef DOC_HIDDEN
SND_DLSYM_BUILD_VERSION(_snd_pcm_shm_open, SND_PCM_DLSYM_VERSION);
#endif
