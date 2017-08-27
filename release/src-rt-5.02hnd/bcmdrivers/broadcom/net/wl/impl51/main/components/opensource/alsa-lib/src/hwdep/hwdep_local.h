/*
 *  HwDep interface - local header file
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
#include <limits.h>
#include "local.h"

typedef struct {
	int (*close)(snd_hwdep_t *hwdep);
	int (*nonblock)(snd_hwdep_t *hwdep, int nonblock);
	int (*info)(snd_hwdep_t *hwdep, snd_hwdep_info_t *info);
	int (*ioctl)(snd_hwdep_t *hwdep, unsigned int request, void * arg);
	ssize_t (*write)(snd_hwdep_t *hwdep, const void *buffer, size_t size);
	ssize_t (*read)(snd_hwdep_t *hwdep, void *buffer, size_t size);
} snd_hwdep_ops_t;

struct _snd_hwdep {
	void *dl_handle;
	char *name;
	snd_hwdep_type_t type;
	int mode;
	int poll_fd;
	const snd_hwdep_ops_t *ops;
	void *private_data;
};

int snd_hwdep_hw_open(snd_hwdep_t **handle, const char *name, int card, int device, int mode);
