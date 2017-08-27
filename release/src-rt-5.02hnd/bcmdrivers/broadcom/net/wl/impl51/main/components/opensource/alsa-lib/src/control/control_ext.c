/**
 * \file control/control_ext.c
 * \ingroup CtlPlugin_SDK
 * \brief External Control Plugin SDK
 * \author Takashi Iwai <tiwai@suse.de>
 * \date 2005
 */
/*
 *  Control Interface - External Control Plugin SDK
 *
 *  Copyright (c) 2005 Takashi Iwai <tiwai@suse.de>
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
#include "control_local.h"
#include "control_external.h"

#ifndef PIC
/* entry for static linking */
const char *_snd_module_control_ext = "";
#endif

static int snd_ctl_ext_close(snd_ctl_t *handle)
{
	snd_ctl_ext_t *ext = handle->private_data;
	
	if (ext->callback->close)
		ext->callback->close(ext);
	return 0;
}

static int snd_ctl_ext_nonblock(snd_ctl_t *handle, int nonblock)
{
	snd_ctl_ext_t *ext = handle->private_data;

	ext->nonblock = nonblock;
	return 0;
}

static int snd_ctl_ext_async(snd_ctl_t *ctl ATTRIBUTE_UNUSED,
			     int sig ATTRIBUTE_UNUSED,
			     pid_t pid ATTRIBUTE_UNUSED)
{
	return -ENOSYS;
}

static int snd_ctl_ext_subscribe_events(snd_ctl_t *handle, int subscribe)
{
	snd_ctl_ext_t *ext = handle->private_data;

	if (subscribe < 0)
		return ext->subscribed;
	ext->subscribed = !!subscribe;
	if (ext->callback->subscribe_events)
		ext->callback->subscribe_events(ext, subscribe);
	return 0;
}

static int snd_ctl_ext_card_info(snd_ctl_t *handle, snd_ctl_card_info_t *info)
{
	snd_ctl_ext_t *ext = handle->private_data;

	memset(info, 0, sizeof(*info));
	info->card = ext->card_idx;
	memcpy(info->id, ext->id, sizeof(info->id));
	memcpy(info->driver, ext->driver, sizeof(info->driver));
	memcpy(info->name, ext->name, sizeof(info->name));
	memcpy(info->longname, ext->longname, sizeof(info->longname));
	memcpy(info->mixername, ext->mixername, sizeof(info->mixername));
	return 0;
}

static int snd_ctl_ext_elem_list(snd_ctl_t *handle, snd_ctl_elem_list_t *list)
{
	snd_ctl_ext_t *ext = handle->private_data;
	int ret;
	unsigned int i, offset;
	snd_ctl_elem_id_t *ids;

	list->count = ext->callback->elem_count(ext);
	list->used = 0;
	ids = list->pids;
	offset = list->offset;
	for (i = 0; i < list->space; i++) {
		if (offset >= list->count)
			break;
		snd_ctl_elem_id_clear(ids);
		ret = ext->callback->elem_list(ext, offset, ids);
		if (ret < 0)
			return ret;
		ids->numid = offset + 1; /* fake number */
		list->used++;
		offset++;
		ids++;
	}
	return 0;
}

static snd_ctl_ext_key_t get_elem(snd_ctl_ext_t *ext, snd_ctl_elem_id_t *id)
{
	int numid = id->numid;
	if (numid > 0) {
		ext->callback->elem_list(ext, numid - 1, id);
		id->numid = numid;
	} else
		id->numid = 0;
	return ext->callback->find_elem(ext, id);
}

static int snd_ctl_ext_elem_info(snd_ctl_t *handle, snd_ctl_elem_info_t *info)
{
	snd_ctl_ext_t *ext = handle->private_data;
	snd_ctl_ext_key_t key;
	int type, ret;

	key = get_elem(ext, &info->id);
	if (key == SND_CTL_EXT_KEY_NOT_FOUND)
		return -ENOENT;
	ret = ext->callback->get_attribute(ext, key, &type, &info->access, &info->count);
	if (ret < 0)
		goto err;
	info->type = type;
	ret = -EINVAL;
	switch (info->type) {
	case SND_CTL_ELEM_TYPE_BOOLEAN:
		info->value.integer.min = 0;
		info->value.integer.max = 1;
		ret = 0;
		break;
	case SND_CTL_ELEM_TYPE_INTEGER:
		if (! ext->callback->get_integer_info)
			goto err;
		ret = ext->callback->get_integer_info(ext, key, &info->value.integer.min,
						      &info->value.integer.max,
						      &info->value.integer.step);
		break;
	case SND_CTL_ELEM_TYPE_INTEGER64:
		if (! ext->callback->get_integer64_info)
			goto err;
		{
			int64_t xmin, xmax, xstep;
			ret = ext->callback->get_integer64_info(ext, key,
								&xmin,
								&xmax,
								&xstep);
			info->value.integer64.min = xmin;
			info->value.integer64.max = xmax;
			info->value.integer64.step = xstep;
		}
		break;
	case SND_CTL_ELEM_TYPE_ENUMERATED:
		if (! ext->callback->get_enumerated_info)
			goto err;
		ret = ext->callback->get_enumerated_info(ext, key, &info->value.enumerated.items);
		ext->callback->get_enumerated_name(ext, key, info->value.enumerated.item,
						   info->value.enumerated.name,
						   sizeof(info->value.enumerated.name));
		break;
	default:
		ret = 0;
		break;
	}

 err:
	if (ext->callback->free_key)
		ext->callback->free_key(ext, key);

	return ret;
}

static int snd_ctl_ext_elem_add(snd_ctl_t *handle ATTRIBUTE_UNUSED,
				snd_ctl_elem_info_t *info ATTRIBUTE_UNUSED)
{
	return -ENXIO;
}

static int snd_ctl_ext_elem_replace(snd_ctl_t *handle ATTRIBUTE_UNUSED,
				    snd_ctl_elem_info_t *info ATTRIBUTE_UNUSED)
{
	return -ENXIO;
}

static int snd_ctl_ext_elem_remove(snd_ctl_t *handle ATTRIBUTE_UNUSED,
				   snd_ctl_elem_id_t *id ATTRIBUTE_UNUSED)
{
	return -ENXIO;
}

static int snd_ctl_ext_elem_read(snd_ctl_t *handle, snd_ctl_elem_value_t *control)
{
	snd_ctl_ext_t *ext = handle->private_data;
	snd_ctl_ext_key_t key;
	int type, ret;
	unsigned int access, count;

	key = get_elem(ext, &control->id);
	if (key == SND_CTL_EXT_KEY_NOT_FOUND)
		return -ENOENT;
	ret = ext->callback->get_attribute(ext, key, &type, &access, &count);
	if (ret < 0)
		goto err;
	ret = -EINVAL;
	switch (type) {
	case SND_CTL_ELEM_TYPE_BOOLEAN:
	case SND_CTL_ELEM_TYPE_INTEGER:
		if (! ext->callback->read_integer)
			goto err;
		ret = ext->callback->read_integer(ext, key, control->value.integer.value);
		break;
	case SND_CTL_ELEM_TYPE_INTEGER64:
		if (! ext->callback->read_integer64)
			goto err;
		ret = ext->callback->read_integer64(ext, key,
						    (int64_t*)control->value.integer64.value);
		break;
	case SND_CTL_ELEM_TYPE_ENUMERATED:
		if (! ext->callback->read_enumerated)
			goto err;
		ret = ext->callback->read_enumerated(ext, key, control->value.enumerated.item);
		break;
	case SND_CTL_ELEM_TYPE_BYTES:
		if (! ext->callback->read_bytes)
			goto err;
		ret = ext->callback->read_bytes(ext, key, control->value.bytes.data,
						sizeof(control->value.bytes.data));
		break;
	case SND_CTL_ELEM_TYPE_IEC958:
		if (! ext->callback->read_iec958)
			goto err;
		ret = ext->callback->read_iec958(ext, key, (snd_aes_iec958_t *)&control->value.iec958);
		break;
	default:
		break;
	}

 err:
	if (ext->callback->free_key)
		ext->callback->free_key(ext, key);

	return ret;
}

static int snd_ctl_ext_elem_write(snd_ctl_t *handle, snd_ctl_elem_value_t *control)
{
	snd_ctl_ext_t *ext = handle->private_data;
	snd_ctl_ext_key_t key;
	int type, ret;
	unsigned int access, count;

	key = get_elem(ext, &control->id);
	if (key == SND_CTL_EXT_KEY_NOT_FOUND)
		return -ENOENT;
	ret = ext->callback->get_attribute(ext, key, &type, &access, &count);
	if (ret < 0)
		goto err;
	ret = -EINVAL;
	switch (type) {
	case SND_CTL_ELEM_TYPE_BOOLEAN:
	case SND_CTL_ELEM_TYPE_INTEGER:
		if (! ext->callback->write_integer)
			goto err;
		ret = ext->callback->write_integer(ext, key, control->value.integer.value);
		break;
	case SND_CTL_ELEM_TYPE_INTEGER64:
		if (! ext->callback->write_integer64)
			goto err;
		ret = ext->callback->write_integer64(ext, key, (int64_t *)control->value.integer64.value);
		break;
	case SND_CTL_ELEM_TYPE_ENUMERATED:
		if (! ext->callback->write_enumerated)
			goto err;
		ret = ext->callback->write_enumerated(ext, key, control->value.enumerated.item);
		break;
	case SND_CTL_ELEM_TYPE_BYTES:
		if (! ext->callback->write_bytes)
			goto err;
		ret = ext->callback->write_bytes(ext, key, control->value.bytes.data,
						sizeof(control->value.bytes.data));
		break;
	case SND_CTL_ELEM_TYPE_IEC958:
		if (! ext->callback->write_iec958)
			goto err;
		ret = ext->callback->write_iec958(ext, key, (snd_aes_iec958_t *)&control->value.iec958);
		break;
	default:
		break;
	}

 err:
	if (ext->callback->free_key)
		ext->callback->free_key(ext, key);

	return ret;
}

static int snd_ctl_ext_elem_lock(snd_ctl_t *handle ATTRIBUTE_UNUSED,
				 snd_ctl_elem_id_t *id ATTRIBUTE_UNUSED)
{
	return -ENXIO;
}

static int snd_ctl_ext_elem_unlock(snd_ctl_t *handle ATTRIBUTE_UNUSED,
				   snd_ctl_elem_id_t *id ATTRIBUTE_UNUSED)
{
	return -ENXIO;
}

static int snd_ctl_ext_next_device(snd_ctl_t *handle ATTRIBUTE_UNUSED,
				   int *device ATTRIBUTE_UNUSED)
{
	return -ENXIO;
}

static int snd_ctl_ext_prefer_subdevice(snd_ctl_t *handle ATTRIBUTE_UNUSED,
					int subdev ATTRIBUTE_UNUSED)
{
	return -ENXIO;
}

static int snd_ctl_ext_hwdep_info(snd_ctl_t *handle ATTRIBUTE_UNUSED,
				  snd_hwdep_info_t *info ATTRIBUTE_UNUSED)
{
	return -ENXIO;
}

static int snd_ctl_ext_pcm_info(snd_ctl_t *handle ATTRIBUTE_UNUSED,
				snd_pcm_info_t *info ATTRIBUTE_UNUSED)
{
	return -ENXIO;
}

static int snd_ctl_ext_rawmidi_info(snd_ctl_t *handle ATTRIBUTE_UNUSED,
				    snd_rawmidi_info_t *info ATTRIBUTE_UNUSED)
{
	return -ENXIO;
}

static int snd_ctl_ext_set_power_state(snd_ctl_t *handle ATTRIBUTE_UNUSED,
				       unsigned int state ATTRIBUTE_UNUSED)
{
	return 0;
}

static int snd_ctl_ext_get_power_state(snd_ctl_t *handle ATTRIBUTE_UNUSED,
				       unsigned int *state ATTRIBUTE_UNUSED)
{
	return 0;
}

static int snd_ctl_ext_read(snd_ctl_t *handle, snd_ctl_event_t *event)
{
	snd_ctl_ext_t *ext = handle->private_data;

	memset(event, 0, sizeof(*event));
	return ext->callback->read_event(ext, &event->data.elem.id, &event->data.elem.mask);
}

static int snd_ctl_ext_poll_descriptors_count(snd_ctl_t *handle)
{
	snd_ctl_ext_t *ext = handle->private_data;

	if (ext->callback->poll_descriptors_count)
		return ext->callback->poll_descriptors_count(ext);
	if (ext->poll_fd >= 0)
		return 1;
	return 0;
}

static int snd_ctl_ext_poll_descriptors(snd_ctl_t *handle, struct pollfd *pfds, unsigned int space)
{
	snd_ctl_ext_t *ext = handle->private_data;

	if (ext->callback->poll_descriptors)
		return ext->callback->poll_descriptors(ext, pfds, space);
	if (ext->poll_fd < 0)
		return 0;
	if (space > 0) {
		pfds->fd = ext->poll_fd;
		pfds->events = POLLIN|POLLERR|POLLNVAL;
		return 1;
	}
	return 0;
}

static int snd_ctl_ext_poll_revents(snd_ctl_t *handle, struct pollfd *pfds, unsigned int nfds, unsigned short *revents)
{
	snd_ctl_ext_t *ext = handle->private_data;

	if (ext->callback->poll_revents)
		return ext->callback->poll_revents(ext, pfds, nfds, revents);
	if (nfds == 1) {
		*revents = pfds->revents;
                return 0;
	}
	return -EINVAL;
}

static const snd_ctl_ops_t snd_ctl_ext_ops = {
	.close = snd_ctl_ext_close,
	.nonblock = snd_ctl_ext_nonblock,
	.async = snd_ctl_ext_async,
	.subscribe_events = snd_ctl_ext_subscribe_events,
	.card_info = snd_ctl_ext_card_info,
	.element_list = snd_ctl_ext_elem_list,
	.element_info = snd_ctl_ext_elem_info,
	.element_add = snd_ctl_ext_elem_add,
	.element_replace = snd_ctl_ext_elem_replace,
	.element_remove = snd_ctl_ext_elem_remove,
	.element_read = snd_ctl_ext_elem_read,
	.element_write = snd_ctl_ext_elem_write,
	.element_lock = snd_ctl_ext_elem_lock,
	.element_unlock = snd_ctl_ext_elem_unlock,
	.hwdep_next_device = snd_ctl_ext_next_device,
	.hwdep_info = snd_ctl_ext_hwdep_info,
	.pcm_next_device = snd_ctl_ext_next_device,
	.pcm_info = snd_ctl_ext_pcm_info,
	.pcm_prefer_subdevice = snd_ctl_ext_prefer_subdevice,
	.rawmidi_next_device = snd_ctl_rawmidi_next_device,
	.rawmidi_info = snd_ctl_ext_rawmidi_info,
	.rawmidi_prefer_subdevice = snd_ctl_ext_prefer_subdevice,
	.set_power_state = snd_ctl_ext_set_power_state,
	.get_power_state = snd_ctl_ext_get_power_state,
	.read = snd_ctl_ext_read,
	.poll_descriptors_count = snd_ctl_ext_poll_descriptors_count,
	.poll_descriptors = snd_ctl_ext_poll_descriptors,
	.poll_revents = snd_ctl_ext_poll_revents,
};

/*
 * Exported functions
 */


/**
 * \brief Create an external control plugin instance
 * \param ext the plugin handle
 * \param name name of control
 * \param mode control open mode
 * \return 0 if successful, or a negative error code
 *
 * Creates the external control instance.
 *
 */
int snd_ctl_ext_create(snd_ctl_ext_t *ext, const char *name, int mode)
{
	snd_ctl_t *ctl;
	int err;

	if (ext->version != SND_CTL_EXT_VERSION) {
		SNDERR("ctl_ext: Plugin version mismatch\n");
		return -ENXIO;
	}

	err = snd_ctl_new(&ctl, SND_CTL_TYPE_EXT, name);
	if (err < 0)
		return err;

	ext->handle = ctl;

	ctl->ops = &snd_ctl_ext_ops;
	ctl->private_data = ext;
	ctl->poll_fd = ext->poll_fd;
	if (mode & SND_CTL_NONBLOCK)
		ext->nonblock = 1;

	return 0;
}

/**
 * \brief Delete the external control plugin
 * \param ext the plugin handle
 * \return 0 if successful, or a negative error code
 */
int snd_ctl_ext_delete(snd_ctl_ext_t *ext)
{
	return snd_ctl_close(ext->handle);
}
