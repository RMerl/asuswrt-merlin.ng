/*
 *  RawMIDI - Virtual (sequencer mode)
 *  Copyright (c) 2003 by Takashi Iwai <tiwai@suse.de>
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
#include "rawmidi_local.h"
#include "seq.h"
#include "seq_midi_event.h"

#ifndef PIC
/* entry for static linking */
const char *_snd_module_rawmidi_virt = "";
#endif


#ifndef DOC_HIDDEN
typedef struct {
	int open;

	snd_seq_t *handle;
	int port;

	snd_midi_event_t *midi_event;

	snd_seq_event_t *in_event;
	int in_buf_size;
	int in_buf_ofs;
	char *in_buf_ptr;
	char in_tmp_buf[16];

	snd_seq_event_t out_event;
	int pending;
} snd_rawmidi_virtual_t;

int _snd_seq_open_lconf(snd_seq_t **seqp, const char *name, 
			int streams, int mode, snd_config_t *lconf,
			snd_config_t *parent_conf);
#endif

static int snd_rawmidi_virtual_close(snd_rawmidi_t *rmidi)
{
	snd_rawmidi_virtual_t *virt = rmidi->private_data;
	virt->open--;
	if (virt->open)
		return 0;
	snd_seq_close(virt->handle);
	if (virt->midi_event)
		snd_midi_event_free(virt->midi_event);
	free(virt);
	return 0;
}

static int snd_rawmidi_virtual_nonblock(snd_rawmidi_t *rmidi, int nonblock)
{
	snd_rawmidi_virtual_t *virt = rmidi->private_data;

	return snd_seq_nonblock(virt->handle, nonblock);
}

static int snd_rawmidi_virtual_info(snd_rawmidi_t *rmidi, snd_rawmidi_info_t * info)
{
	// snd_rawmidi_virtual_t *virt = rmidi->private_data;

	info->stream = rmidi->stream;
	info->card = 0;
	info->device = 0;
	info->subdevice = 0;
	info->flags = 0;
	strcpy((char *)info->id, "Virtual");
	strcpy((char *)info->name, "Virtual RawMIDI");
	strcpy((char *)info->subname, "Virtual RawMIDI");
	info->subdevices_count = 1;
	info->subdevices_avail = 0;
	return 0;
}

static int snd_rawmidi_virtual_input_params(snd_rawmidi_virtual_t *virt, snd_rawmidi_params_t *params)
{
	int err;

	// snd_rawmidi_drain_input(substream);
	if (params->buffer_size < sizeof(snd_seq_event_t) ||
	    params->buffer_size > 1024L * 1024L) {
		return -EINVAL;
	}
	if (params->buffer_size != snd_seq_get_input_buffer_size(virt->handle)) {
		err = snd_seq_set_input_buffer_size(virt->handle, params->buffer_size);
		if (err < 0)
			return err;
		params->buffer_size = snd_seq_get_input_buffer_size(virt->handle);
	}
	return 0;
}


static int snd_rawmidi_virtual_output_params(snd_rawmidi_virtual_t *virt, snd_rawmidi_params_t *params)
{
	int err;

	// snd_rawmidi_drain_output(substream);
	if (params->buffer_size < sizeof(snd_seq_event_t) ||
	    params->buffer_size > 1024L * 1024L) {
		return -EINVAL;
	}
	if (params->buffer_size != snd_seq_get_output_buffer_size(virt->handle)) {
		err = snd_seq_set_output_buffer_size(virt->handle, params->buffer_size);
		if (err < 0)
			return err;
		params->buffer_size = snd_seq_get_output_buffer_size(virt->handle);
	}
	return 0;
}


static int snd_rawmidi_virtual_params(snd_rawmidi_t *rmidi, snd_rawmidi_params_t * params)
{
	snd_rawmidi_virtual_t *virt = rmidi->private_data;
	params->stream = rmidi->stream;

	if (rmidi->stream == SND_RAWMIDI_STREAM_INPUT)
		return snd_rawmidi_virtual_input_params(virt, params);
	else
		return snd_rawmidi_virtual_output_params(virt, params);
}

static int snd_rawmidi_virtual_status(snd_rawmidi_t *rmidi, snd_rawmidi_status_t * status)
{
	// snd_rawmidi_virtual_t *virt = rmidi->private_data;
	memset(status, 0, sizeof(*status));
	status->stream = rmidi->stream;
	return 0;
}

static int snd_rawmidi_virtual_drop(snd_rawmidi_t *rmidi)
{
	snd_rawmidi_virtual_t *virt = rmidi->private_data;
	if (rmidi->stream == SND_RAWMIDI_STREAM_OUTPUT) {
		snd_seq_drop_output(virt->handle);
		snd_midi_event_reset_encode(virt->midi_event);
		virt->pending = 0;
	} else {
		snd_seq_drop_input(virt->handle);
		snd_midi_event_reset_decode(virt->midi_event);
		virt->in_buf_ofs = 0;
	}
	return 0;
}

static int snd_rawmidi_virtual_drain(snd_rawmidi_t *rmidi)
{
	snd_rawmidi_virtual_t *virt = rmidi->private_data;
	int err;

	if (rmidi->stream == SND_RAWMIDI_STREAM_OUTPUT) {
		if (virt->pending) {
			err = snd_seq_event_output(virt->handle, &virt->out_event);
			if (err < 0)
				return err;
			virt->pending = 0;
		}
		snd_seq_drain_output(virt->handle);
		snd_seq_sync_output_queue(virt->handle);
	}
	return snd_rawmidi_virtual_drop(rmidi);
}

static ssize_t snd_rawmidi_virtual_write(snd_rawmidi_t *rmidi, const void *buffer, size_t size)
{
	snd_rawmidi_virtual_t *virt = rmidi->private_data;
	ssize_t result = 0;
	ssize_t size1;
	int err;

	if (virt->pending) {
		err = snd_seq_event_output(virt->handle, &virt->out_event);
		if (err < 0) {
			if (err != -EAGAIN)
				/* we got some fatal error. removing this event
				 * at the next time
				 */
				virt->pending = 0;
			return err;
		}
		virt->pending = 0;
	}

	while (size > 0) {
		size1 = snd_midi_event_encode(virt->midi_event, buffer, size, &virt->out_event);
		if (size1 <= 0)
			break;
		size -= size1;
		result += size1;
		buffer += size1;
		if (virt->out_event.type == SND_SEQ_EVENT_NONE)
			continue;
		snd_seq_ev_set_subs(&virt->out_event);
		snd_seq_ev_set_source(&virt->out_event, virt->port);
		snd_seq_ev_set_direct(&virt->out_event);
		err = snd_seq_event_output(virt->handle, &virt->out_event);
		if (err < 0) {
			virt->pending = 1;
			return result > 0 ? result : err;
		}
	}

	if (result > 0)
		snd_seq_drain_output(virt->handle);

	return result;
}

static ssize_t snd_rawmidi_virtual_read(snd_rawmidi_t *rmidi, void *buffer, size_t size)
{
	snd_rawmidi_virtual_t *virt = rmidi->private_data;
	ssize_t result = 0;
	int size1, err;

	while (size > 0) {
		if (! virt->in_buf_ofs) {
			err = snd_seq_event_input_pending(virt->handle, 1);
			if (err <= 0 && result > 0)
				return result;
			err = snd_seq_event_input(virt->handle, &virt->in_event);
			if (err < 0)
				return result > 0 ? result : err;

			if (virt->in_event->type == SND_SEQ_EVENT_SYSEX) {
				virt->in_buf_ptr = virt->in_event->data.ext.ptr;
				virt->in_buf_size = virt->in_event->data.ext.len;
			} else {
				virt->in_buf_ptr = virt->in_tmp_buf;
				virt->in_buf_size = snd_midi_event_decode(virt->midi_event,
									  (unsigned char *)virt->in_tmp_buf,
									  sizeof(virt->in_tmp_buf),
									  virt->in_event);
			}
			if (virt->in_buf_size <= 0)
				continue;
		}
		size1 = virt->in_buf_size - virt->in_buf_ofs;
		if ((size_t)size1 > size) {
			virt->in_buf_ofs += size1 - size;
			memcpy(buffer, virt->in_buf_ptr, size);
			result += size;
			break;
		}
		memcpy(buffer, virt->in_buf_ptr + virt->in_buf_ofs, size1);
		size -= size1;
		result += size1;
		buffer += size1;
		virt->in_buf_ofs = 0;
	}

	return result;
}

static const snd_rawmidi_ops_t snd_rawmidi_virtual_ops = {
	.close = snd_rawmidi_virtual_close,
	.nonblock = snd_rawmidi_virtual_nonblock,
	.info = snd_rawmidi_virtual_info,
	.params = snd_rawmidi_virtual_params,
	.status = snd_rawmidi_virtual_status,
	.drop = snd_rawmidi_virtual_drop,
	.drain = snd_rawmidi_virtual_drain,
	.write = snd_rawmidi_virtual_write,
	.read = snd_rawmidi_virtual_read,
};


/*! \page rawmidi RawMidi interface

\section rawmidi_virt Virtual RawMidi interface

The "virtual" plugin creates a virtual RawMidi instance on the ALSA
sequencer, which can be accessed through the connection of the sequencer
ports.
There is no connection established as default.

For creating a virtual RawMidi instance, pass "virtual" as its name at
creation.

Example:
\code
snd_rawmidi_open(&read_handle, &write_handle, "virtual", 0);
\endcode

*/

int snd_rawmidi_virtual_open(snd_rawmidi_t **inputp, snd_rawmidi_t **outputp,
			     const char *name, snd_seq_t *seq_handle, int port,
			     int merge, int mode)
{
	int err;
	snd_rawmidi_t *rmidi;
	snd_rawmidi_virtual_t *virt = NULL;
	struct pollfd pfd;

	if (inputp)
		*inputp = 0;
	if (outputp)
		*outputp = 0;

	virt = calloc(1, sizeof(*virt));
	if (virt == NULL) {
		err = -ENOMEM;
		goto _err;
	}
	virt->handle = seq_handle;
	virt->port = port;
	err = snd_midi_event_new(256, &virt->midi_event);
	if (err < 0)
		goto _err;
	snd_midi_event_init(virt->midi_event);
	snd_midi_event_no_status(virt->midi_event, !merge);

	if (inputp) {
		rmidi = calloc(1, sizeof(*rmidi));
		if (rmidi == NULL) {
			err = -ENOMEM;
			goto _err;
		}
		if (name)
			rmidi->name = strdup(name);
		rmidi->type = SND_RAWMIDI_TYPE_VIRTUAL;
		rmidi->stream = SND_RAWMIDI_STREAM_INPUT;
		rmidi->mode = mode;
		err = snd_seq_poll_descriptors(seq_handle, &pfd, 1, POLLIN);
		if (err < 0)
			goto _err;
		rmidi->poll_fd = pfd.fd;
		rmidi->ops = &snd_rawmidi_virtual_ops;
		rmidi->private_data = virt;
		virt->open++;
		*inputp = rmidi;
	}
	if (outputp) {
		rmidi = calloc(1, sizeof(*rmidi));
		if (rmidi == NULL) {
			err = -ENOMEM;
			goto _err;
		}
		if (name)
			rmidi->name = strdup(name);
		rmidi->type = SND_RAWMIDI_TYPE_VIRTUAL;
		rmidi->stream = SND_RAWMIDI_STREAM_OUTPUT;
		rmidi->mode = mode;
		err = snd_seq_poll_descriptors(seq_handle, &pfd, 1, POLLOUT);
		if (err < 0)
			goto _err;
		rmidi->poll_fd = pfd.fd;
		rmidi->ops = &snd_rawmidi_virtual_ops;
		rmidi->private_data = virt;
		virt->open++;
		*outputp = rmidi;
	}

	return 0;

 _err:
	if (seq_handle)
		snd_seq_close(seq_handle);
	if (virt->midi_event)
		snd_midi_event_free(virt->midi_event);
	free(virt);
	if (inputp)
		free(*inputp);
	if (outputp)
		free(*outputp);
	return err;
}

int _snd_rawmidi_virtual_open(snd_rawmidi_t **inputp, snd_rawmidi_t **outputp,
			   char *name, snd_config_t *root ATTRIBUTE_UNUSED,
			   snd_config_t *conf, int mode)
{
	snd_config_iterator_t i, next;
	const char *slave_str = NULL;
	int err;
	int streams, seq_mode;
	int merge = 1;
	int port;
	unsigned int caps;
	snd_seq_t *seq_handle;

	snd_config_for_each(i, next, conf) {
		snd_config_t *n = snd_config_iterator_entry(i);
		const char *id;
		if (snd_config_get_id(n, &id) < 0)
			continue;
		if (snd_rawmidi_conf_generic_id(id))
			continue;
		if (strcmp(id, "slave") == 0) {
			err = snd_config_get_string(n, &slave_str);
			if (err < 0)
				return err;
			continue;
		}
		if (strcmp(id, "merge") == 0) {
			merge = snd_config_get_bool(n);
			continue;
		}
		return -EINVAL;
	}

	streams = 0;
	if (inputp)
		streams |= SND_SEQ_OPEN_INPUT;
	if (outputp)
		streams |= SND_SEQ_OPEN_OUTPUT;
	if (! streams)
		return -EINVAL;

	seq_mode = 0;
	if (mode & SND_RAWMIDI_NONBLOCK)
		seq_mode |= SND_SEQ_NONBLOCK;

	if (! slave_str)
		slave_str = "default";
	err = _snd_seq_open_lconf(&seq_handle, slave_str, streams, seq_mode,
				  root, conf);
	if (err < 0)
		return err;

	caps = 0;
	if (inputp)
		caps |= SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SYNC_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE;
	if (outputp)
		caps |= SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SYNC_READ | SND_SEQ_PORT_CAP_SUBS_READ;
	if (inputp && outputp)
		caps |= SNDRV_SEQ_PORT_CAP_DUPLEX;

	port = snd_seq_create_simple_port(seq_handle, "Virtual RawMIDI",
					  caps, SNDRV_SEQ_PORT_TYPE_MIDI_GENERIC);
	if (port < 0) {
		snd_seq_close(seq_handle);
		return port;
	}

	return snd_rawmidi_virtual_open(inputp, outputp, name, seq_handle, port,
				     merge, mode);
}

#ifndef DOC_HIDDEN
SND_DLSYM_BUILD_VERSION(_snd_rawmidi_virtual_open, SND_RAWMIDI_DLSYM_VERSION);
#endif
