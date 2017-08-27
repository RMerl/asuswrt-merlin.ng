/*
 *  Rawmidi interface - local header file
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
#include <limits.h>
#include "local.h"

typedef struct {
	int (*close)(snd_rawmidi_t *rawmidi);
	int (*nonblock)(snd_rawmidi_t *rawmidi, int nonblock);
	int (*info)(snd_rawmidi_t *rawmidi, snd_rawmidi_info_t *info);
	int (*params)(snd_rawmidi_t *rawmidi, snd_rawmidi_params_t *params);
	int (*status)(snd_rawmidi_t *rawmidi, snd_rawmidi_status_t *status);
	int (*drop)(snd_rawmidi_t *rawmidi);
	int (*drain)(snd_rawmidi_t *rawmidi);
	ssize_t (*write)(snd_rawmidi_t *rawmidi, const void *buffer, size_t size);
	ssize_t (*read)(snd_rawmidi_t *rawmidi, void *buffer, size_t size);
} snd_rawmidi_ops_t;

struct _snd_rawmidi {
	void *dl_handle;
	char *name;
	snd_rawmidi_type_t type;
	snd_rawmidi_stream_t stream;
	int mode;
	int poll_fd;
	const snd_rawmidi_ops_t *ops;
	void *private_data;
	size_t buffer_size;
	size_t avail_min;
	unsigned int no_active_sensing: 1;
};

int snd_rawmidi_hw_open(snd_rawmidi_t **input, snd_rawmidi_t **output,
			const char *name, int card, int device, int subdevice,
			int mode);

int snd_rawmidi_virtual_open(snd_rawmidi_t **inputp, snd_rawmidi_t **outputp,
			     const char *name, snd_seq_t *seq_handle, int port,
			     int merge, int mode);

int snd_rawmidi_conf_generic_id(const char *id);
