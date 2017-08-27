/*
 *  Timer Interface - main file
 *  Copyright (c) 1998-2001 by Jaroslav Kysela <perex@perex.cz>
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
#include "timer_local.h"

#ifndef PIC
/* entry for static linking */
const char *_snd_module_timer_hw = "";
#endif

#define SNDRV_FILE_TIMER		ALSA_DEVICE_DIRECTORY "timer"
#define SNDRV_TIMER_VERSION_MAX	SNDRV_PROTOCOL_VERSION(2, 0, 5)

#define SNDRV_TIMER_IOCTL_STATUS_OLD	_IOW('T', 0x14, struct sndrv_timer_status)

enum {
	SNDRV_TIMER_IOCTL_START_OLD = _IO('T', 0x20),
	SNDRV_TIMER_IOCTL_STOP_OLD = _IO('T', 0x21),
	SNDRV_TIMER_IOCTL_CONTINUE_OLD = _IO('T', 0x22),
	SNDRV_TIMER_IOCTL_PAUSE_OLD = _IO('T', 0x23),
};

static int snd_timer_hw_close(snd_timer_t *handle)
{
	snd_timer_t *tmr = handle;
	int res;

	if (!tmr)
		return -EINVAL;
	res = close(tmr->poll_fd) < 0 ? -errno : 0;
	return res;
}

static int snd_timer_hw_nonblock(snd_timer_t *timer, int nonblock)
{
	long flags;
	assert(timer);
	if ((flags = fcntl(timer->poll_fd, F_GETFL)) < 0)
		return -errno;
	if (nonblock)
		flags |= O_NONBLOCK;
	else
		flags &= ~O_NONBLOCK;
	if (fcntl(timer->poll_fd, F_SETFL, flags) < 0)
		return -errno;
	return 0;
}

static int snd_timer_hw_async(snd_timer_t *timer, int sig, pid_t pid)
{
	long flags;
	int fd;

	assert(timer);
	fd = timer->poll_fd;
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

static int snd_timer_hw_info(snd_timer_t *handle, snd_timer_info_t * info)
{
	snd_timer_t *tmr;

	tmr = handle;
	if (!tmr || !info)
		return -EINVAL;
	if (ioctl(tmr->poll_fd, SNDRV_TIMER_IOCTL_INFO, info) < 0)
		return -errno;
	return 0;
}

static int snd_timer_hw_params(snd_timer_t *handle, snd_timer_params_t * params)
{
	snd_timer_t *tmr;

	tmr = handle;
	if (!tmr || !params)
		return -EINVAL;
	if (ioctl(tmr->poll_fd, SNDRV_TIMER_IOCTL_PARAMS, params) < 0)
		return -errno;
	return 0;
}

static int snd_timer_hw_status(snd_timer_t *handle, snd_timer_status_t * status)
{
	snd_timer_t *tmr;
	int cmd;

	tmr = handle;
	if (!tmr || !status)
		return -EINVAL;
	if (tmr->version < SNDRV_PROTOCOL_VERSION(2, 0, 1))
		cmd = SNDRV_TIMER_IOCTL_STATUS_OLD;
	else
		cmd = SNDRV_TIMER_IOCTL_STATUS;
	if (ioctl(tmr->poll_fd, cmd, status) < 0)
		return -errno;
	return 0;
}

static int snd_timer_hw_start(snd_timer_t *handle)
{
	snd_timer_t *tmr;
	unsigned int cmd;

	tmr = handle;
	if (!tmr)
		return -EINVAL;
	if (tmr->version < SNDRV_PROTOCOL_VERSION(2, 0, 4))
		cmd = SNDRV_TIMER_IOCTL_START_OLD;
	else
		cmd = SNDRV_TIMER_IOCTL_START;
	if (ioctl(tmr->poll_fd, cmd) < 0)
		return -errno;
	return 0;
}

static int snd_timer_hw_stop(snd_timer_t *handle)
{
	snd_timer_t *tmr;
	unsigned int cmd;

	tmr = handle;
	if (!tmr)
		return -EINVAL;
	if (tmr->version < SNDRV_PROTOCOL_VERSION(2, 0, 4))
		cmd = SNDRV_TIMER_IOCTL_STOP_OLD;
	else
		cmd = SNDRV_TIMER_IOCTL_STOP;
	if (ioctl(tmr->poll_fd, cmd) < 0)
		return -errno;
	return 0;
}

static int snd_timer_hw_continue(snd_timer_t *handle)
{
	snd_timer_t *tmr;
	unsigned int cmd;

	tmr = handle;
	if (!tmr)
		return -EINVAL;
	if (tmr->version < SNDRV_PROTOCOL_VERSION(2, 0, 4))
		cmd = SNDRV_TIMER_IOCTL_CONTINUE_OLD;
	else
		cmd = SNDRV_TIMER_IOCTL_CONTINUE;
	if (ioctl(tmr->poll_fd, cmd) < 0)
		return -errno;
	return 0;
}

static ssize_t snd_timer_hw_read(snd_timer_t *handle, void *buffer, size_t size)
{
	snd_timer_t *tmr;
	ssize_t result;

	tmr = handle;
	if (!tmr || (!buffer && size > 0))
		return -EINVAL;
	result = read(tmr->poll_fd, buffer, size);
	if (result < 0)
		return -errno;
	return result;
}

static const snd_timer_ops_t snd_timer_hw_ops = {
	.close = snd_timer_hw_close,
	.nonblock = snd_timer_hw_nonblock,
	.async = snd_timer_hw_async,
	.info = snd_timer_hw_info,
	.params = snd_timer_hw_params,
	.status = snd_timer_hw_status,
	.rt_start = snd_timer_hw_start,
	.rt_stop = snd_timer_hw_stop,
	.rt_continue = snd_timer_hw_continue,
	.read = snd_timer_hw_read,
};

int snd_timer_hw_open(snd_timer_t **handle, const char *name, int dev_class, int dev_sclass, int card, int device, int subdevice, int mode)
{
	int fd, ver, tmode, ret;
	snd_timer_t *tmr;
	struct sndrv_timer_select sel;

	*handle = NULL;

	tmode = O_RDONLY;
	if (mode & SND_TIMER_OPEN_NONBLOCK)
		tmode |= O_NONBLOCK;	
	fd = snd_open_device(SNDRV_FILE_TIMER, tmode);
	if (fd < 0)
		return -errno;
	if (ioctl(fd, SNDRV_TIMER_IOCTL_PVERSION, &ver) < 0) {
		ret = -errno;
		close(fd);
		return ret;
	}
	if (SNDRV_PROTOCOL_INCOMPATIBLE(ver, SNDRV_TIMER_VERSION_MAX)) {
		close(fd);
		return -SND_ERROR_INCOMPATIBLE_VERSION;
	}
	if (mode & SND_TIMER_OPEN_TREAD) {
		int arg = 1;
		if (ver < SNDRV_PROTOCOL_VERSION(2, 0, 3)) {
			ret = -ENOTTY;
			goto __no_tread;
		}
		if (ioctl(fd, SNDRV_TIMER_IOCTL_TREAD, &arg) < 0) {
			ret = -errno;
		      __no_tread:
			close(fd);
			SNDMSG("extended read is not supported (SNDRV_TIMER_IOCTL_TREAD)");
			return ret;
		}
	}
	memset(&sel, 0, sizeof(sel));
	sel.id.dev_class = dev_class;
	sel.id.dev_sclass = dev_sclass;
	sel.id.card = card;
	sel.id.device = device;
	sel.id.subdevice = subdevice;
	if (ioctl(fd, SNDRV_TIMER_IOCTL_SELECT, &sel) < 0) {
		ret = -errno;
		close(fd);
		return ret;
	}
	tmr = (snd_timer_t *) calloc(1, sizeof(snd_timer_t));
	if (tmr == NULL) {
		close(fd);
		return -ENOMEM;
	}
	tmr->type = SND_TIMER_TYPE_HW;
	tmr->version = ver;
	tmr->mode = tmode;
	tmr->name = strdup(name);
	tmr->poll_fd = fd;
	tmr->ops = &snd_timer_hw_ops;
	INIT_LIST_HEAD(&tmr->async_handlers);
	*handle = tmr;
	return 0;
}

int _snd_timer_hw_open(snd_timer_t **timer, char *name,
		       snd_config_t *root ATTRIBUTE_UNUSED,
		       snd_config_t *conf, int mode)
{
	snd_config_iterator_t i, next;
	long dev_class = SND_TIMER_CLASS_GLOBAL, dev_sclass = SND_TIMER_SCLASS_NONE;
	long card = 0, device = 0, subdevice = 0;
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
		if (strcmp(id, "class") == 0) {
			err = snd_config_get_integer(n, &dev_class);
			if (err < 0)
				return err;
			continue;
		}
		if (strcmp(id, "sclass") == 0) {
			err = snd_config_get_integer(n, &dev_sclass);
			if (err < 0)
				return err;
			continue;
		}
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
		SNDERR("Unexpected field %s", id);
		return -EINVAL;
	}
	if (card < 0)
		return -EINVAL;
	return snd_timer_hw_open(timer, name, dev_class, dev_sclass, card, device, subdevice, mode);
}
SND_DLSYM_BUILD_VERSION(_snd_timer_hw_open, SND_TIMER_DLSYM_VERSION);
