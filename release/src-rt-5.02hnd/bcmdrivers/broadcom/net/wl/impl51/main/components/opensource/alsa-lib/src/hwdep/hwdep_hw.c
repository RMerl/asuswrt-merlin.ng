/*
 *  Hardware dependent Interface - main file for hardware access
 *  Copyright (c) 2001 by Jaroslav Kysela <perex@perex.cz>
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
#include "hwdep_local.h"

#ifndef PIC
/* entry for static linking */
const char *_snd_module_hwdep_hw = "";
#endif

#define SNDRV_FILE_HWDEP	ALSA_DEVICE_DIRECTORY "hwC%iD%i"
#define SNDRV_HWDEP_VERSION_MAX	SNDRV_PROTOCOL_VERSION(1, 0, 1)

static int snd_hwdep_hw_close(snd_hwdep_t *hwdep)
{
	int res;
	assert(hwdep);
	res = close(hwdep->poll_fd) < 0 ? -errno : 0;
	return res;
}

static int snd_hwdep_hw_nonblock(snd_hwdep_t *hwdep, int nonblock)
{
	long flags;
	assert(hwdep);
	if ((flags = fcntl(hwdep->poll_fd, F_GETFL)) < 0)
		return -errno;
	if (nonblock)
		flags |= O_NONBLOCK;
	else
		flags &= ~O_NONBLOCK;
	if (fcntl(hwdep->poll_fd, F_SETFL, flags) < 0)
		return -errno;
	return 0;
}

static int snd_hwdep_hw_info(snd_hwdep_t *hwdep, snd_hwdep_info_t *info)
{
	assert(hwdep && info);
	if (ioctl(hwdep->poll_fd, SNDRV_HWDEP_IOCTL_INFO, info) < 0)
		return -errno;
	return 0;
}

static int snd_hwdep_hw_ioctl(snd_hwdep_t *hwdep, unsigned int request, void * arg)
{
	assert(hwdep);
	if (ioctl(hwdep->poll_fd, request, arg) < 0)
		return -errno;
	return 0;
}

static ssize_t snd_hwdep_hw_write(snd_hwdep_t *hwdep, const void *buffer, size_t size)
{
	ssize_t result;
	assert(hwdep && (buffer || size == 0));
	result = write(hwdep->poll_fd, buffer, size);
	if (result < 0)
		return -errno;
	return result;
}

static ssize_t snd_hwdep_hw_read(snd_hwdep_t *hwdep, void *buffer, size_t size)
{
	ssize_t result;
	assert(hwdep && (buffer || size == 0));
	result = read(hwdep->poll_fd, buffer, size);
	if (result < 0)
		return -errno;
	return result;
}

static const snd_hwdep_ops_t snd_hwdep_hw_ops = {
	.close = snd_hwdep_hw_close,
	.nonblock = snd_hwdep_hw_nonblock,
	.info = snd_hwdep_hw_info,
	.ioctl = snd_hwdep_hw_ioctl,
	.write = snd_hwdep_hw_write,
	.read = snd_hwdep_hw_read,
};

int snd_hwdep_hw_open(snd_hwdep_t **handle, const char *name, int card, int device, int mode)
{
	int fd, ver, ret;
	char filename[sizeof(SNDRV_FILE_HWDEP) + 20];
	snd_hwdep_t *hwdep;
	assert(handle);

	*handle = NULL;
	
	if (card < 0 || card >= 32)
		return -EINVAL;
	sprintf(filename, SNDRV_FILE_HWDEP, card, device);
	fd = snd_open_device(filename, mode);
	if (fd < 0) {
		snd_card_load(card);
		fd = snd_open_device(filename, mode);
		if (fd < 0)
			return -errno;
	}
	if (ioctl(fd, SNDRV_HWDEP_IOCTL_PVERSION, &ver) < 0) {
		ret = -errno;
		close(fd);
		return ret;
	}
	if (SNDRV_PROTOCOL_INCOMPATIBLE(ver, SNDRV_HWDEP_VERSION_MAX)) {
		close(fd);
		return -SND_ERROR_INCOMPATIBLE_VERSION;
	}
	hwdep = (snd_hwdep_t *) calloc(1, sizeof(snd_hwdep_t));
	if (hwdep == NULL) {
		close(fd);
		return -ENOMEM;
	}
	hwdep->name = strdup(name);
	hwdep->poll_fd = fd;
	hwdep->mode = mode;
	hwdep->type = SND_HWDEP_TYPE_HW;
	hwdep->ops = &snd_hwdep_hw_ops;
	*handle = hwdep;
	return 0;
}

int _snd_hwdep_hw_open(snd_hwdep_t **hwdep, char *name,
		       snd_config_t *root ATTRIBUTE_UNUSED,
		       snd_config_t *conf, int mode)
{
	snd_config_iterator_t i, next;
	long card = -1, device = 0;
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
		if (strcmp(id, "device") == 0) {
			err = snd_config_get_integer(n, &device);
			if (err < 0)
				return err;
			continue;
		}
		SNDERR("Unexpected field %s", id);
		return -EINVAL;
	}
	if (card < 0)
		return -EINVAL;
	return snd_hwdep_hw_open(hwdep, name, card, device, mode);
}
SND_DLSYM_BUILD_VERSION(_snd_hwdep_hw_open, SND_HWDEP_DLSYM_VERSION);
