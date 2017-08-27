/*
 *  RawMIDI - Hardware
 *  Copyright (c) 2000 by Jaroslav Kysela <perex@perex.cz>
 *                        Abramo Bagnara <abramo@alsa-project.org>
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
#include <fcntl.h>
#include <sys/ioctl.h>
#include "../control/control_local.h"
#include "rawmidi_local.h"

#ifndef PIC
/* entry for static linking */
const char *_snd_module_rawmidi_hw = "";
#endif

#define SNDRV_FILE_RAWMIDI		ALSA_DEVICE_DIRECTORY "midiC%iD%i"
#define SNDRV_RAWMIDI_VERSION_MAX	SNDRV_PROTOCOL_VERSION(2, 0, 0)

#ifndef DOC_HIDDEN
typedef struct {
	int open;
	int fd;
	int card, device, subdevice;
} snd_rawmidi_hw_t;
#endif

static int snd_rawmidi_hw_close(snd_rawmidi_t *rmidi)
{
	snd_rawmidi_hw_t *hw = rmidi->private_data;
	int err = 0;

	hw->open--;
	if (hw->open)
		return 0;
	if (close(hw->fd)) {
		err = -errno;
		SYSERR("close failed\n");
	}
	free(hw);
	return err;
}

static int snd_rawmidi_hw_nonblock(snd_rawmidi_t *rmidi, int nonblock)
{
	snd_rawmidi_hw_t *hw = rmidi->private_data;
	long flags;

	if ((flags = fcntl(hw->fd, F_GETFL)) < 0) {
		SYSERR("F_GETFL failed");
		return -errno;
	}
	if (nonblock)
		flags |= O_NONBLOCK;
	else
		flags &= ~O_NONBLOCK;
	if (fcntl(hw->fd, F_SETFL, flags) < 0) {
		SYSERR("F_SETFL for O_NONBLOCK failed");
		return -errno;
	}
	return 0;
}

static int snd_rawmidi_hw_info(snd_rawmidi_t *rmidi, snd_rawmidi_info_t * info)
{
	snd_rawmidi_hw_t *hw = rmidi->private_data;
	info->stream = rmidi->stream;
	if (ioctl(hw->fd, SNDRV_RAWMIDI_IOCTL_INFO, info) < 0) {
		SYSERR("SNDRV_RAWMIDI_IOCTL_INFO failed");
		return -errno;
	}
	return 0;
}

static int snd_rawmidi_hw_params(snd_rawmidi_t *rmidi, snd_rawmidi_params_t * params)
{
	snd_rawmidi_hw_t *hw = rmidi->private_data;
	params->stream = rmidi->stream;
	if (ioctl(hw->fd, SNDRV_RAWMIDI_IOCTL_PARAMS, params) < 0) {
		SYSERR("SNDRV_RAWMIDI_IOCTL_PARAMS failed");
		return -errno;
	}
	return 0;
}

static int snd_rawmidi_hw_status(snd_rawmidi_t *rmidi, snd_rawmidi_status_t * status)
{
	snd_rawmidi_hw_t *hw = rmidi->private_data;
	status->stream = rmidi->stream;
	if (ioctl(hw->fd, SNDRV_RAWMIDI_IOCTL_STATUS, status) < 0) {
		SYSERR("SNDRV_RAWMIDI_IOCTL_STATUS failed");
		return -errno;
	}
	return 0;
}

static int snd_rawmidi_hw_drop(snd_rawmidi_t *rmidi)
{
	snd_rawmidi_hw_t *hw = rmidi->private_data;
	int str = rmidi->stream;
	if (ioctl(hw->fd, SNDRV_RAWMIDI_IOCTL_DROP, &str) < 0) {
		SYSERR("SNDRV_RAWMIDI_IOCTL_DROP failed");
		return -errno;
	}
	return 0;
}

static int snd_rawmidi_hw_drain(snd_rawmidi_t *rmidi)
{
	snd_rawmidi_hw_t *hw = rmidi->private_data;
	int str = rmidi->stream;
	if (ioctl(hw->fd, SNDRV_RAWMIDI_IOCTL_DRAIN, &str) < 0) {
		SYSERR("SNDRV_RAWMIDI_IOCTL_DRAIN failed");
		return -errno;
	}
	return 0;
}

static ssize_t snd_rawmidi_hw_write(snd_rawmidi_t *rmidi, const void *buffer, size_t size)
{
	snd_rawmidi_hw_t *hw = rmidi->private_data;
	ssize_t result;
	result = write(hw->fd, buffer, size);
	if (result < 0)
		return -errno;
	return result;
}

static ssize_t snd_rawmidi_hw_read(snd_rawmidi_t *rmidi, void *buffer, size_t size)
{
	snd_rawmidi_hw_t *hw = rmidi->private_data;
	ssize_t result;
	result = read(hw->fd, buffer, size);
	if (result < 0)
		return -errno;
	return result;
}

static const snd_rawmidi_ops_t snd_rawmidi_hw_ops = {
	.close = snd_rawmidi_hw_close,
	.nonblock = snd_rawmidi_hw_nonblock,
	.info = snd_rawmidi_hw_info,
	.params = snd_rawmidi_hw_params,
	.status = snd_rawmidi_hw_status,
	.drop = snd_rawmidi_hw_drop,
	.drain = snd_rawmidi_hw_drain,
	.write = snd_rawmidi_hw_write,
	.read = snd_rawmidi_hw_read,
};


int snd_rawmidi_hw_open(snd_rawmidi_t **inputp, snd_rawmidi_t **outputp,
			const char *name, int card, int device, int subdevice,
			int mode)
{
	int fd, ver, ret;
	int attempt = 0;
	char filename[sizeof(SNDRV_FILE_RAWMIDI) + 20];
	snd_ctl_t *ctl;
	snd_rawmidi_t *rmidi;
	snd_rawmidi_hw_t *hw = NULL;
	snd_rawmidi_info_t info;
	int fmode;

	if (inputp)
		*inputp = NULL;
	if (outputp)
		*outputp = NULL;
	
	if ((ret = snd_ctl_hw_open(&ctl, NULL, card, 0)) < 0)
		return ret;
	sprintf(filename, SNDRV_FILE_RAWMIDI, card, device);

      __again:
      	if (attempt++ > 3) {
      		snd_ctl_close(ctl);
      		return -EBUSY;
      	}
      	ret = snd_ctl_rawmidi_prefer_subdevice(ctl, subdevice);
	if (ret < 0) {
		snd_ctl_close(ctl);
		return ret;
	}

	if (!inputp)
		fmode = O_WRONLY;
	else if (!outputp)
		fmode = O_RDONLY;
	else
		fmode = O_RDWR;

	if (mode & SND_RAWMIDI_APPEND) {
		assert(outputp);
		fmode |= O_APPEND;
	}

	if (mode & SND_RAWMIDI_NONBLOCK) {
		fmode |= O_NONBLOCK;
	}
	
	if (mode & SND_RAWMIDI_SYNC) {
		fmode |= O_SYNC;
	}

	assert(!(mode & ~(SND_RAWMIDI_APPEND|SND_RAWMIDI_NONBLOCK|SND_RAWMIDI_SYNC)));

	fd = snd_open_device(filename, fmode);
	if (fd < 0) {
		snd_card_load(card);
		fd = snd_open_device(filename, fmode);
		if (fd < 0) {
			snd_ctl_close(ctl);
			SYSERR("open %s failed", filename);
			return -errno;
		}
	}
	if (ioctl(fd, SNDRV_RAWMIDI_IOCTL_PVERSION, &ver) < 0) {
		ret = -errno;
		SYSERR("SNDRV_RAWMIDI_IOCTL_PVERSION failed");
		close(fd);
		snd_ctl_close(ctl);
		return ret;
	}
	if (SNDRV_PROTOCOL_INCOMPATIBLE(ver, SNDRV_RAWMIDI_VERSION_MAX)) {
		close(fd);
		snd_ctl_close(ctl);
		return -SND_ERROR_INCOMPATIBLE_VERSION;
	}
	if (subdevice >= 0) {
		memset(&info, 0, sizeof(info));
		info.stream = outputp ? SNDRV_RAWMIDI_STREAM_OUTPUT : SNDRV_RAWMIDI_STREAM_INPUT;
		if (ioctl(fd, SNDRV_RAWMIDI_IOCTL_INFO, &info) < 0) {
			SYSERR("SNDRV_RAWMIDI_IOCTL_INFO failed");
			ret = -errno;
			close(fd);
			snd_ctl_close(ctl);
			return ret;
		}
		if (info.subdevice != (unsigned int) subdevice) {
			close(fd);
			goto __again;
		}
	}
	snd_ctl_close(ctl);

	hw = calloc(1, sizeof(snd_rawmidi_hw_t));
	if (hw == NULL)
		goto _nomem;
	hw->card = card;
	hw->device = device;
	hw->subdevice = subdevice;
	hw->fd = fd;

	if (inputp) {
		rmidi = calloc(1, sizeof(snd_rawmidi_t));
		if (rmidi == NULL)
			goto _nomem;
		if (name)
			rmidi->name = strdup(name);
		rmidi->type = SND_RAWMIDI_TYPE_HW;
		rmidi->stream = SND_RAWMIDI_STREAM_INPUT;
		rmidi->mode = mode;
		rmidi->poll_fd = fd;
		rmidi->ops = &snd_rawmidi_hw_ops;
		rmidi->private_data = hw;
		hw->open++;
		*inputp = rmidi;
	}
	if (outputp) {
		rmidi = calloc(1, sizeof(snd_rawmidi_t));
		if (rmidi == NULL)
			goto _nomem;
		if (name)
			rmidi->name = strdup(name);
		rmidi->type = SND_RAWMIDI_TYPE_HW;
		rmidi->stream = SND_RAWMIDI_STREAM_OUTPUT;
		rmidi->mode = mode;
		rmidi->poll_fd = fd;
		rmidi->ops = &snd_rawmidi_hw_ops;
		rmidi->private_data = hw;
		hw->open++;
		*outputp = rmidi;
	}
	return 0;

 _nomem:
	close(fd);
	free(hw);
	if (inputp)
		free(*inputp);
	if (outputp)
		free(*outputp);
	return -ENOMEM;
}

int _snd_rawmidi_hw_open(snd_rawmidi_t **inputp, snd_rawmidi_t **outputp,
			 char *name, snd_config_t *root ATTRIBUTE_UNUSED,
			 snd_config_t *conf, int mode)
{
	snd_config_iterator_t i, next;
	long card = -1, device = 0, subdevice = -1;
	const char *str;
	int err;
	snd_config_for_each(i, next, conf) {
		snd_config_t *n = snd_config_iterator_entry(i);
		const char *id;
		if (snd_config_get_id(n, &id) < 0)
			continue;
		if (snd_rawmidi_conf_generic_id(id))
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
		if (strcmp(id, "device") == 0) {
			err = snd_config_get_integer(n, &device);
			if (err < 0)
				return err;
			continue;
		}
		if (strcmp(id, "subdevice") == 0) {
			err = snd_config_get_integer(n, &subdevice);
			if (err < 0)
				return err;
			continue;
		}
		return -EINVAL;
	}
	if (card < 0)
		return -EINVAL;
	return snd_rawmidi_hw_open(inputp, outputp, name, card, device, subdevice, mode);
}
SND_DLSYM_BUILD_VERSION(_snd_rawmidi_hw_open, SND_RAWMIDI_DLSYM_VERSION);
