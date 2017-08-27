/*
 *  Control Interface - Hardware
 *  Copyright (c) 1998,1999,2000 by Jaroslav Kysela <perex@perex.cz>
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
#include <signal.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "control_local.h"

#ifndef PIC
/* entry for static linking */
const char *_snd_module_control_hw = "";
#endif

#ifndef F_SETSIG
#define F_SETSIG 10
#endif

#ifndef DOC_HIDDEN
#define SNDRV_FILE_CONTROL	ALSA_DEVICE_DIRECTORY "controlC%i"
#define SNDRV_CTL_VERSION_MAX	SNDRV_PROTOCOL_VERSION(2, 0, 4)

typedef struct {
	int card;
	int fd;
	unsigned int protocol;
} snd_ctl_hw_t;
#endif /* DOC_HIDDEN */

static int snd_ctl_hw_close(snd_ctl_t *handle)
{
	snd_ctl_hw_t *hw = handle->private_data;
	int res;
	res = close(hw->fd) < 0 ? -errno : 0;
	free(hw);
	return res;
}

static int snd_ctl_hw_nonblock(snd_ctl_t *handle, int nonblock)
{
	snd_ctl_hw_t *hw = handle->private_data;
	long flags;
	int fd = hw->fd;
	if ((flags = fcntl(fd, F_GETFL)) < 0) {
		SYSERR("F_GETFL failed");
		return -errno;
	}
	if (nonblock)
		flags |= O_NONBLOCK;
	else
		flags &= ~O_NONBLOCK;
	if (fcntl(fd, F_SETFL, flags) < 0) {
		SYSERR("F_SETFL for O_NONBLOCK failed");
		return -errno;
	}
	return 0;
}

static int snd_ctl_hw_async(snd_ctl_t *ctl, int sig, pid_t pid)
{
	long flags;
	snd_ctl_hw_t *hw = ctl->private_data;
	int fd = hw->fd;

	if ((flags = fcntl(fd, F_GETFL)) < 0) {
		SYSERR("F_GETFL failed");
		return -errno;
	}
	if (sig >= 0)
		flags |= O_ASYNC;
	else
		flags &= ~O_ASYNC;
	if (fcntl(fd, F_SETFL, flags) < 0) {
		SYSERR("F_SETFL for O_ASYNC failed");
		return -errno;
	}
	if (sig < 0)
		return 0;
	if (fcntl(fd, F_SETSIG, (long)sig) < 0) {
		SYSERR("F_SETSIG failed");
		return -errno;
	}
	if (fcntl(fd, F_SETOWN, (long)pid) < 0) {
		SYSERR("F_SETOWN failed");
		return -errno;
	}
	return 0;
}

static int snd_ctl_hw_subscribe_events(snd_ctl_t *handle, int subscribe)
{
	snd_ctl_hw_t *hw = handle->private_data;
	if (ioctl(hw->fd, SNDRV_CTL_IOCTL_SUBSCRIBE_EVENTS, &subscribe) < 0) {
		SYSERR("SNDRV_CTL_IOCTL_SUBSCRIBE_EVENTS failed");
		return -errno;
	}
	return 0;
}

static int snd_ctl_hw_card_info(snd_ctl_t *handle, snd_ctl_card_info_t *info)
{
	snd_ctl_hw_t *hw = handle->private_data;
	if (ioctl(hw->fd, SNDRV_CTL_IOCTL_CARD_INFO, info) < 0) {
		SYSERR("SNDRV_CTL_IOCTL_CARD_INFO failed");
		return -errno;
	}
	return 0;
}

static int snd_ctl_hw_elem_list(snd_ctl_t *handle, snd_ctl_elem_list_t *list)
{
	snd_ctl_hw_t *hw = handle->private_data;
	if (ioctl(hw->fd, SNDRV_CTL_IOCTL_ELEM_LIST, list) < 0)
		return -errno;
	return 0;
}

static int snd_ctl_hw_elem_info(snd_ctl_t *handle, snd_ctl_elem_info_t *info)
{
	snd_ctl_hw_t *hw = handle->private_data;
	if (ioctl(hw->fd, SNDRV_CTL_IOCTL_ELEM_INFO, info) < 0)
		return -errno;
	return 0;
}

static int snd_ctl_hw_elem_add(snd_ctl_t *handle, snd_ctl_elem_info_t *info)
{
	snd_ctl_hw_t *hw = handle->private_data;
	if (ioctl(hw->fd, SNDRV_CTL_IOCTL_ELEM_ADD, info) < 0)
		return -errno;
	return 0;
}

static int snd_ctl_hw_elem_replace(snd_ctl_t *handle, snd_ctl_elem_info_t *info)
{
	snd_ctl_hw_t *hw = handle->private_data;
	if (ioctl(hw->fd, SNDRV_CTL_IOCTL_ELEM_REPLACE, info) < 0)
		return -errno;
	return 0;
}

static int snd_ctl_hw_elem_remove(snd_ctl_t *handle, snd_ctl_elem_id_t *id)
{
	snd_ctl_hw_t *hw = handle->private_data;
	if (ioctl(hw->fd, SNDRV_CTL_IOCTL_ELEM_REMOVE, id) < 0)
		return -errno;
	return 0;
}

static int snd_ctl_hw_elem_read(snd_ctl_t *handle, snd_ctl_elem_value_t *control)
{
	snd_ctl_hw_t *hw = handle->private_data;
	if (ioctl(hw->fd, SNDRV_CTL_IOCTL_ELEM_READ, control) < 0)
		return -errno;
	return 0;
}

static int snd_ctl_hw_elem_write(snd_ctl_t *handle, snd_ctl_elem_value_t *control)
{
	snd_ctl_hw_t *hw = handle->private_data;
	if (ioctl(hw->fd, SNDRV_CTL_IOCTL_ELEM_WRITE, control) < 0)
		return -errno;
	return 0;
}

static int snd_ctl_hw_elem_lock(snd_ctl_t *handle, snd_ctl_elem_id_t *id)
{
	snd_ctl_hw_t *hw = handle->private_data;
	if (ioctl(hw->fd, SNDRV_CTL_IOCTL_ELEM_LOCK, id) < 0)
		return -errno;
	return 0;
}

static int snd_ctl_hw_elem_unlock(snd_ctl_t *handle, snd_ctl_elem_id_t *id)
{
	snd_ctl_hw_t *hw = handle->private_data;
	if (ioctl(hw->fd, SNDRV_CTL_IOCTL_ELEM_UNLOCK, id) < 0)
		return -errno;
	return 0;
}

static int snd_ctl_hw_elem_tlv(snd_ctl_t *handle, int op_flag,
			       unsigned int numid,
			       unsigned int *tlv, unsigned int tlv_size)
{
	int inum;
	snd_ctl_hw_t *hw = handle->private_data;
	struct sndrv_ctl_tlv *xtlv;
	
	/* we don't support TLV on protocol ver 2.0.3 or earlier */
	if (hw->protocol < SNDRV_PROTOCOL_VERSION(2, 0, 4))
		return -ENXIO;

	switch (op_flag) {
	case -1: inum = SNDRV_CTL_IOCTL_TLV_COMMAND; break;
 	case 0:	inum = SNDRV_CTL_IOCTL_TLV_READ; break;
	case 1:	inum = SNDRV_CTL_IOCTL_TLV_WRITE; break;
	default: return -EINVAL;
	}
	xtlv = malloc(sizeof(struct sndrv_ctl_tlv) + tlv_size);
	if (xtlv == NULL)
		return -ENOMEM; 
	xtlv->numid = numid;
	xtlv->length = tlv_size;
	memcpy(xtlv->tlv, tlv, tlv_size);
	if (ioctl(hw->fd, inum, xtlv) < 0) {
		free(xtlv);
		return -errno;
	}
	if (op_flag == 0) {
		if (xtlv->tlv[1] + 2 * sizeof(unsigned int) > tlv_size)
			return -EFAULT;
		memcpy(tlv, xtlv->tlv, xtlv->tlv[1] + 2 * sizeof(unsigned int));
	}
	free(xtlv);
	return 0;
}

static int snd_ctl_hw_hwdep_next_device(snd_ctl_t *handle, int * device)
{
	snd_ctl_hw_t *hw = handle->private_data;
	if (ioctl(hw->fd, SNDRV_CTL_IOCTL_HWDEP_NEXT_DEVICE, device) < 0)
		return -errno;
	return 0;
}

static int snd_ctl_hw_hwdep_info(snd_ctl_t *handle, snd_hwdep_info_t * info)
{
	snd_ctl_hw_t *hw = handle->private_data;
	if (ioctl(hw->fd, SNDRV_CTL_IOCTL_HWDEP_INFO, info) < 0)
		return -errno;
	return 0;
}

static int snd_ctl_hw_pcm_next_device(snd_ctl_t *handle, int * device)
{
	snd_ctl_hw_t *hw = handle->private_data;
	if (ioctl(hw->fd, SNDRV_CTL_IOCTL_PCM_NEXT_DEVICE, device) < 0)
		return -errno;
	return 0;
}

static int snd_ctl_hw_pcm_info(snd_ctl_t *handle, snd_pcm_info_t * info)
{
	snd_ctl_hw_t *hw = handle->private_data;
	if (ioctl(hw->fd, SNDRV_CTL_IOCTL_PCM_INFO, info) < 0)
		return -errno;
	return 0;
}

static int snd_ctl_hw_pcm_prefer_subdevice(snd_ctl_t *handle, int subdev)
{
	snd_ctl_hw_t *hw = handle->private_data;
	if (ioctl(hw->fd, SNDRV_CTL_IOCTL_PCM_PREFER_SUBDEVICE, &subdev) < 0)
		return -errno;
	return 0;
}

static int snd_ctl_hw_rawmidi_next_device(snd_ctl_t *handle, int * device)
{
	snd_ctl_hw_t *hw = handle->private_data;
	if (ioctl(hw->fd, SNDRV_CTL_IOCTL_RAWMIDI_NEXT_DEVICE, device) < 0)
		return -errno;
	return 0;
}

static int snd_ctl_hw_rawmidi_info(snd_ctl_t *handle, snd_rawmidi_info_t * info)
{
	snd_ctl_hw_t *hw = handle->private_data;
	if (ioctl(hw->fd, SNDRV_CTL_IOCTL_RAWMIDI_INFO, info) < 0)
		return -errno;
	return 0;
}

static int snd_ctl_hw_rawmidi_prefer_subdevice(snd_ctl_t *handle, int subdev)
{
	snd_ctl_hw_t *hw = handle->private_data;
	if (ioctl(hw->fd, SNDRV_CTL_IOCTL_RAWMIDI_PREFER_SUBDEVICE, &subdev) < 0)
		return -errno;
	return 0;
}

static int snd_ctl_hw_set_power_state(snd_ctl_t *handle, unsigned int state)
{
	snd_ctl_hw_t *hw = handle->private_data;
	if (ioctl(hw->fd, SNDRV_CTL_IOCTL_POWER, &state) < 0)
		return -errno;
	return 0;
}

static int snd_ctl_hw_get_power_state(snd_ctl_t *handle, unsigned int *state)
{
	snd_ctl_hw_t *hw = handle->private_data;
	if (ioctl(hw->fd, SNDRV_CTL_IOCTL_POWER_STATE, state) < 0)
		return -errno;
	return 0;
}

static int snd_ctl_hw_read(snd_ctl_t *handle, snd_ctl_event_t *event)
{
	snd_ctl_hw_t *hw = handle->private_data;
	ssize_t res = read(hw->fd, event, sizeof(*event));
	if (res <= 0)
		return -errno;
	if (CHECK_SANITY(res != sizeof(*event))) {
		SNDMSG("snd_ctl_hw_read: read size error (req:%d, got:%d)\n",
		       sizeof(*event), res);
		return -EINVAL;
	}
	return 1;
}

static const snd_ctl_ops_t snd_ctl_hw_ops = {
	.close = snd_ctl_hw_close,
	.nonblock = snd_ctl_hw_nonblock,
	.async = snd_ctl_hw_async,
	.subscribe_events = snd_ctl_hw_subscribe_events,
	.card_info = snd_ctl_hw_card_info,
	.element_list = snd_ctl_hw_elem_list,
	.element_info = snd_ctl_hw_elem_info,
	.element_add = snd_ctl_hw_elem_add,
	.element_replace = snd_ctl_hw_elem_replace,
	.element_remove = snd_ctl_hw_elem_remove,
	.element_read = snd_ctl_hw_elem_read,
	.element_write = snd_ctl_hw_elem_write,
	.element_lock = snd_ctl_hw_elem_lock,
	.element_unlock = snd_ctl_hw_elem_unlock,
	.element_tlv = snd_ctl_hw_elem_tlv,
	.hwdep_next_device = snd_ctl_hw_hwdep_next_device,
	.hwdep_info = snd_ctl_hw_hwdep_info,
	.pcm_next_device = snd_ctl_hw_pcm_next_device,
	.pcm_info = snd_ctl_hw_pcm_info,
	.pcm_prefer_subdevice = snd_ctl_hw_pcm_prefer_subdevice,
	.rawmidi_next_device = snd_ctl_hw_rawmidi_next_device,
	.rawmidi_info = snd_ctl_hw_rawmidi_info,
	.rawmidi_prefer_subdevice = snd_ctl_hw_rawmidi_prefer_subdevice,
	.set_power_state = snd_ctl_hw_set_power_state,
	.get_power_state = snd_ctl_hw_get_power_state,
	.read = snd_ctl_hw_read,
};

int snd_ctl_hw_open(snd_ctl_t **handle, const char *name, int card, int mode)
{
	int fd, ver;
	char filename[sizeof(SNDRV_FILE_CONTROL) + 10];
	int fmode;
	snd_ctl_t *ctl;
	snd_ctl_hw_t *hw;
	int err;

	*handle = NULL;	

	if (CHECK_SANITY(card < 0 || card >= 32)) {
		SNDMSG("Invalid card index %d", card);
		return -EINVAL;
	}
	sprintf(filename, SNDRV_FILE_CONTROL, card);
	if (mode & SND_CTL_READONLY)
		fmode = O_RDONLY;
	else
		fmode = O_RDWR;
	if (mode & SND_CTL_NONBLOCK)
		fmode |= O_NONBLOCK;
	if (mode & SND_CTL_ASYNC)
		fmode |= O_ASYNC;
	fd = snd_open_device(filename, fmode);
	if (fd < 0) {
		snd_card_load(card);
		fd = snd_open_device(filename, fmode);
		if (fd < 0)
			return -errno;
	}
	if (ioctl(fd, SNDRV_CTL_IOCTL_PVERSION, &ver) < 0) {
		err = -errno;
		close(fd);
		return err;
	}
	if (SNDRV_PROTOCOL_INCOMPATIBLE(ver, SNDRV_CTL_VERSION_MAX)) {
		close(fd);
		return -SND_ERROR_INCOMPATIBLE_VERSION;
	}
	hw = calloc(1, sizeof(snd_ctl_hw_t));
	if (hw == NULL) {
		close(fd);
		return -ENOMEM;
	}
	hw->card = card;
	hw->fd = fd;
	hw->protocol = ver;

	err = snd_ctl_new(&ctl, SND_CTL_TYPE_HW, name);
	if (err < 0) {
		close(fd);
		free(hw);
	}
	ctl->ops = &snd_ctl_hw_ops;
	ctl->private_data = hw;
	ctl->poll_fd = fd;
	*handle = ctl;
	return 0;
}

int _snd_ctl_hw_open(snd_ctl_t **handlep, char *name, snd_config_t *root ATTRIBUTE_UNUSED, snd_config_t *conf, int mode)
{
	snd_config_iterator_t i, next;
	long card = -1;
	const char *str;
	int err;
	snd_config_for_each(i, next, conf) {
		snd_config_t *n = snd_config_iterator_entry(i);
		const char *id;
		if (snd_config_get_id(n, &id) < 0)
			continue;
		if (strcmp(id, "comment") == 0)
			continue;
		if (strcmp(id, "type") == 0)
			continue;
		if (strcmp(id, "card") == 0) {
			err = snd_config_get_integer(n, &card);
			if (err < 0) {
				err = snd_config_get_string(n, &str);
				if (err < 0)
					return -EINVAL;
				card = snd_card_get_index(str);
				if (card < 0)
					return card;
			}
			continue;
		}
		return -EINVAL;
	}
	if (card < 0)
		return -EINVAL;
	return snd_ctl_hw_open(handlep, name, card, mode);
}
SND_DLSYM_BUILD_VERSION(_snd_ctl_hw_open, SND_CONTROL_DLSYM_VERSION);
