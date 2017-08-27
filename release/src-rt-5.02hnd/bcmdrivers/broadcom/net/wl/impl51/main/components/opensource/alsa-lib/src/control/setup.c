/**
 * \file control/setup.c
 * \brief Routines to setup control primitives from configuration
 * \author Abramo Bagnara <abramo@alsa-project.org>
 * \author Jaroslav Kysela <perex@perex.cz>
 * \date 2001
 *
 * Routines to setup control primitives from configuration
 */
/*
 *  Control Interface - routines for setup from configuration
 *  Copyright (c) 2001 by Abramo Bagnara <abramo@alsa-project.org>
 *			  Jaroslav Kysela <perex@perex.cz>
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
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include "local.h"

#ifndef DOC_HIDDEN
typedef struct {
	unsigned int lock: 1;
	unsigned int preserve: 1;
	snd_ctl_elem_id_t *id;
	snd_ctl_elem_info_t *info;
	snd_ctl_elem_value_t *val;
	snd_ctl_elem_value_t *mask;
	snd_ctl_elem_value_t *old;
	struct list_head list;
} snd_sctl_elem_t;

struct _snd_sctl {
	int mode;
	snd_ctl_t *ctl;
	struct list_head elems;
};
#endif /* DOC_HIDDEN */

static int free_elems(snd_sctl_t *h)
{
	int err = 0;
	while (!list_empty(&h->elems)) {
		snd_sctl_elem_t *elem = list_entry(h->elems.next, snd_sctl_elem_t, list);
		snd_ctl_elem_id_free(elem->id);
		snd_ctl_elem_info_free(elem->info);
		snd_ctl_elem_value_free(elem->val);
		snd_ctl_elem_value_free(elem->mask);
		snd_ctl_elem_value_free(elem->old);
		list_del(&elem->list);
		free(elem);
	}
	if ((h->mode & SND_SCTL_NOFREE) == 0)
		err = snd_ctl_close(h->ctl);
	free(h);
	return err;
}

/**
 * \brief Install given values to control elements
 * \param h Setup control handle
 * \result zero if success, otherwise a negative error code
 */
int snd_sctl_install(snd_sctl_t *h)
{
	struct list_head *pos;
	int err;
	unsigned int k;
	assert(h);
	list_for_each(pos, &h->elems) {
		snd_sctl_elem_t *elem = list_entry(pos, snd_sctl_elem_t, list);
		unsigned int count;
		snd_ctl_elem_type_t type;
		if (elem->lock) {
			err = snd_ctl_elem_lock(h->ctl, elem->id);
			if (err < 0) {
				SNDERR("Cannot lock ctl elem");
				return err;
			}
		}
		err = snd_ctl_elem_read(h->ctl, elem->old);
		if (err < 0) {
			SNDERR("Cannot read ctl elem");
			return err;
		}
		count = snd_ctl_elem_info_get_count(elem->info);
		type = snd_ctl_elem_info_get_type(elem->info);
		switch (type) {
		case SND_CTL_ELEM_TYPE_BOOLEAN:
			for (k = 0; k < count; ++k) {
				int old, val, mask;
				old = snd_ctl_elem_value_get_boolean(elem->old, k);
				mask = snd_ctl_elem_value_get_boolean(elem->mask, k);
				old &= ~mask;
				if (old) {
					val = snd_ctl_elem_value_get_boolean(elem->val, k);
					val |= old;
					snd_ctl_elem_value_set_boolean(elem->val, k, val);
				}
			}
			break;
		case SND_CTL_ELEM_TYPE_INTEGER:
			for (k = 0; k < count; ++k) {
				long old, val, mask;
				old = snd_ctl_elem_value_get_integer(elem->old, k);
				mask = snd_ctl_elem_value_get_integer(elem->mask, k);
				old &= ~mask;
				if (old) {
					val = snd_ctl_elem_value_get_integer(elem->val, k);
					val |= old;
					snd_ctl_elem_value_set_integer(elem->val, k, val);
				}
			}
			break;
		case SND_CTL_ELEM_TYPE_ENUMERATED:
			for (k = 0; k < count; ++k) {
				unsigned int old, val, mask;
				old = snd_ctl_elem_value_get_enumerated(elem->old, k);
				mask = snd_ctl_elem_value_get_enumerated(elem->mask, k);
				old &= ~mask;
				if (old) {
					val = snd_ctl_elem_value_get_enumerated(elem->val, k);
					val |= old;
					snd_ctl_elem_value_set_enumerated(elem->val, k, val);
				}
			}
			break;
		case SND_CTL_ELEM_TYPE_IEC958:
			count = sizeof(snd_aes_iec958_t);
			/* Fall through */
		case SND_CTL_ELEM_TYPE_BYTES:
			for (k = 0; k < count; ++k) {
				unsigned char old, val, mask;
				old = snd_ctl_elem_value_get_byte(elem->old, k);
				mask = snd_ctl_elem_value_get_byte(elem->mask, k);
				old &= ~mask;
				if (old) {
					val = snd_ctl_elem_value_get_byte(elem->val, k);
					val |= old;
					snd_ctl_elem_value_set_byte(elem->val, k, val);
				}
			}
			break;
		default:
			assert(0);
			break;
		}
		err = snd_ctl_elem_write(h->ctl, elem->val);
		if (err < 0) {
			SNDERR("Cannot write ctl elem");
			return err;
		}
	}
	return 0;
}

/**
 * \brief Remove (restore) previous values from control elements
 * \param h Setup control handle
 * \result zero if success, otherwise a negative error code
 */
int snd_sctl_remove(snd_sctl_t *h)
{
	struct list_head *pos;
	int err;
	assert(h);
	list_for_each(pos, &h->elems) {
		snd_sctl_elem_t *elem = list_entry(pos, snd_sctl_elem_t, list);
		if (elem->lock) {
			err = snd_ctl_elem_unlock(h->ctl, elem->id);
			if (err < 0) {
				SNDERR("Cannot unlock ctl elem");
				return err;
			}
		}
		/* Only restore the old value if it differs from the requested
		 * value, because if it has changed restoring the old value
		 * overrides the change.  Take for example, a voice modem with
		 * a .conf that sets preserve off-hook.  Start playback (on-hook
		 * to off-hook), start record (off-hook to off-hook), stop
		 * playback (off-hook to restore on-hook), stop record (on-hook
		 * to restore off-hook), Clearly you don't want to leave the
		 * modem "on the phone" now that there isn't any playback or
		 * recording active.
		 */
		if (elem->preserve && snd_ctl_elem_value_compare(elem->val, elem->old)) {
			err = snd_ctl_elem_write(h->ctl, elem->old);
			if (err < 0) {
				SNDERR("Cannot restore ctl elem");
				return err;
			}
		}
	}
	return 0;
}

static int snd_config_get_ctl_elem_enumerated(snd_config_t *n, snd_ctl_t *ctl,
					      snd_ctl_elem_info_t *info)
{
	const char *str;
	long val;
	unsigned int idx, items;
	switch (snd_config_get_type(n)) {
	case SND_CONFIG_TYPE_INTEGER:
		snd_config_get_integer(n, &val);
		return val;
	case SND_CONFIG_TYPE_STRING:
		snd_config_get_string(n, &str);
		break;
	default:
		return -1;
	}
	items = snd_ctl_elem_info_get_items(info);
	for (idx = 0; idx < items; idx++) {
		int err;
		snd_ctl_elem_info_set_item(info, idx);
		err = snd_ctl_elem_info(ctl, info);
		if (err < 0) {
			SNDERR("Cannot obtain info for CTL elem");
			return err;
		}
		if (strcmp(str, snd_ctl_elem_info_get_item_name(info)) == 0)
			return idx;
	}
	return -1;
}

static int snd_config_get_ctl_elem_value(snd_config_t *conf,
					 snd_ctl_t *ctl,
					 snd_ctl_elem_value_t *val,
					 snd_ctl_elem_value_t *mask,
					 snd_ctl_elem_info_t *info)
{
	int err;
	snd_config_iterator_t i, next;
	snd_ctl_elem_id_t *id;
	snd_ctl_elem_type_t type;
	unsigned int count;
	long v;
	long idx;
	snd_ctl_elem_id_alloca(&id);
	snd_ctl_elem_value_get_id(val, id);
	count = snd_ctl_elem_info_get_count(info);
	type = snd_ctl_elem_info_get_type(info);
	if (count == 1) {
		switch (type) {
		case SND_CTL_ELEM_TYPE_BOOLEAN:
			v = snd_config_get_bool(conf);
			if (v >= 0) {
				snd_ctl_elem_value_set_boolean(val, 0, v);
				if (mask)
					snd_ctl_elem_value_set_boolean(mask, 0, 1);
				return 0;
			}
			break;
		case SND_CTL_ELEM_TYPE_INTEGER:
			err = snd_config_get_integer(conf, &v);
			if (err == 0) {
				snd_ctl_elem_value_set_integer(val, 0, v);
				if (mask)
					snd_ctl_elem_value_set_integer(mask, 0, ~0L);
				return 0;
			}
			break;
		case SND_CTL_ELEM_TYPE_ENUMERATED:
			v = snd_config_get_ctl_elem_enumerated(conf, ctl, info);
			if (v >= 0) {
				snd_ctl_elem_value_set_enumerated(val, 0, v);
				if (mask)
					snd_ctl_elem_value_set_enumerated(mask, 0, ~0);
				return 0;
			}
			break;
		case SND_CTL_ELEM_TYPE_BYTES:
		case SND_CTL_ELEM_TYPE_IEC958:
			break;
		default:
			SNDERR("Unknown control type: %d", type);
			return -EINVAL;
		}
	}
	switch (type) {
	case SND_CTL_ELEM_TYPE_IEC958:
		count = sizeof(snd_aes_iec958_t);
		/* Fall through */
	case SND_CTL_ELEM_TYPE_BYTES:
	{
		const char *buf;
		err = snd_config_get_string(conf, &buf);
		if (err >= 0) {
			int c1 = 0;
			unsigned int len = strlen(buf);
			unsigned int idx = 0;
			if (len % 2 != 0 || len > count * 2) {
			_bad_content:
				SNDERR("bad value content\n");
				return -EINVAL;
			}
			while (*buf) {
				int c = *buf++;
				if (c >= '0' && c <= '9')
					c -= '0';
				else if (c >= 'a' && c <= 'f')
					c = c - 'a' + 10;
				else if (c >= 'A' && c <= 'F')
					c = c - 'A' + 10;
				else {
					goto _bad_content;
				}
				if (idx % 2 == 1) {
					snd_ctl_elem_value_set_byte(val, idx / 2, c1 << 4 | c);
					if (mask)
						snd_ctl_elem_value_set_byte(mask, idx / 2, 0xff);
				} else
					c1 = c;
				idx++;
			}
			return 0;
		}
	}
	default:
		break;
	}
	if (snd_config_get_type(conf) != SND_CONFIG_TYPE_COMPOUND) {
		SNDERR("bad value type");
		return -EINVAL;
	}

	snd_config_for_each(i, next, conf) {
		snd_config_t *n = snd_config_iterator_entry(i);
		const char *id;
		if (snd_config_get_id(n, &id) < 0)
			continue;
		err = safe_strtol(id, &idx);
		if (err < 0 || idx < 0 || (unsigned int) idx >= count) {
			SNDERR("bad value index");
			return -EINVAL;
		}
		switch (type) {
		case SND_CTL_ELEM_TYPE_BOOLEAN:
			v = snd_config_get_bool(n);
			if (v < 0)
				goto _bad_content;
			snd_ctl_elem_value_set_boolean(val, idx, v);
			if (mask)
				snd_ctl_elem_value_set_boolean(mask, idx, 1);
			break;
		case SND_CTL_ELEM_TYPE_INTEGER:
			err = snd_config_get_integer(n, &v);
			if (err < 0)
				goto _bad_content;
			snd_ctl_elem_value_set_integer(val, idx, v);
			if (mask)
				snd_ctl_elem_value_set_integer(mask, idx, ~0L);
			break;
		case SND_CTL_ELEM_TYPE_ENUMERATED:
			v = snd_config_get_ctl_elem_enumerated(n, ctl, info);
			if (v < 0)
				goto _bad_content;
			snd_ctl_elem_value_set_enumerated(val, idx, v);
			if (mask)
				snd_ctl_elem_value_set_enumerated(mask, idx, ~0);
			break;
		case SND_CTL_ELEM_TYPE_BYTES:
		case SND_CTL_ELEM_TYPE_IEC958:
			err = snd_config_get_integer(n, &v);
			if (err < 0 || v < 0 || v > 255)
				goto _bad_content;
			snd_ctl_elem_value_set_byte(val, idx, v);
			if (mask)
				snd_ctl_elem_value_set_byte(mask, idx, 0xff);
			break;
		default:
			break;
		}
	}
	return 0;
}

static int add_elem(snd_sctl_t *h, snd_config_t *_conf, snd_config_t *private_data)
{
	snd_config_t *conf;
	snd_config_iterator_t i, next;
	char *tmp;
	int iface = SND_CTL_ELEM_IFACE_MIXER;
	const char *name = NULL;
	long index = 0;
	long device = -1;
	long subdevice = -1;
	int lock = 0;
	int preserve = 0;
	int optional = 0;
	snd_config_t *value = NULL, *mask = NULL;
	snd_sctl_elem_t *elem = NULL;
	int err;
	err = snd_config_expand(_conf, _conf, NULL, private_data, &conf);
	if (err < 0)
		return err;
	snd_config_for_each(i, next, conf) {
		snd_config_t *n = snd_config_iterator_entry(i);
		const char *id;
		if (snd_config_get_id(n, &id) < 0)
			continue;
		if (strcmp(id, "comment") == 0)
			continue;
		if (strcmp(id, "iface") == 0 || strcmp(id, "interface") == 0) {
			const char *ptr;
			if ((err = snd_config_get_string(n, &ptr)) < 0) {
				SNDERR("field %s is not a string", id);
				goto _err;
			}
			if ((err = snd_config_get_ctl_iface_ascii(ptr)) < 0) {
				SNDERR("Invalid value for '%s'", id);
				goto _err;
			}
			iface = err;
			continue;
		}
		if (strcmp(id, "name") == 0) {
			if ((err = snd_config_get_string(n, &name)) < 0) {
				SNDERR("field %s is not a string", id);
				goto _err;
			}
			continue;
		}
		if (strcmp(id, "index") == 0) {
			if ((err = snd_config_get_integer(n, &index)) < 0) {
				SNDERR("field %s is not an integer", id);
				goto _err;
			}
			continue;
		}
		if (strcmp(id, "device") == 0) {
			if ((err = snd_config_get_integer(n, &device)) < 0) {
				SNDERR("field %s is not an integer", id);
				goto _err;
			}
			continue;
		}
		if (strcmp(id, "subdevice") == 0) {
			if ((err = snd_config_get_integer(n, &subdevice)) < 0) {
				SNDERR("field %s is not an integer", id);
				goto _err;
			}
			continue;
		}
		if (strcmp(id, "lock") == 0) {
			if ((err = snd_config_get_ascii(n, &tmp)) < 0) {
				SNDERR("field %s has an invalid type", id);
				goto _err;
			}
			err = snd_config_get_bool_ascii(tmp);
			if (err < 0) {
				SNDERR("field %s is not a boolean", id);
				free(tmp);
				goto _err;
			}
			lock = err;
			free(tmp);
			continue;
		}
		if (strcmp(id, "preserve") == 0) {
			if ((err = snd_config_get_ascii(n, &tmp)) < 0) {
				SNDERR("field %s has an invalid type", id);
				goto _err;
			}
			err = snd_config_get_bool_ascii(tmp);
			if (err < 0) {
				SNDERR("field %s is not a boolean", id);
				free(tmp);
				goto _err;
			}
			preserve = err;
			free(tmp);
			continue;
		}
		if (strcmp(id, "value") == 0) {
			value = n;
			continue;
		}
		if (strcmp(id, "mask") == 0) {
			mask = n;
			continue;
		}
		if (strcmp(id, "optional") == 0) {
			if ((err = snd_config_get_ascii(n, &tmp)) < 0) {
				SNDERR("field %s has an invalid type", id);
				goto _err;
			}
			err = snd_config_get_bool_ascii(tmp);
			if (err < 0) {
				SNDERR("field %s is not a boolean", id);
				free(tmp);
				goto _err;
			}
			optional = err;
			free(tmp);
			continue;
		}
		SNDERR("Unknown field %s", id);
		return -EINVAL;
	}
	if (name == NULL) {
		SNDERR("Missing control name");
		err = -EINVAL;
		goto _err;
	}
	if (value == NULL) {
		SNDERR("Missing control value");
		err = -EINVAL;
		goto _err;
	}
	if (device < 0)
		device = 0;
	if (subdevice < 0)
		subdevice = 0;
	elem = calloc(1, sizeof(*elem));
	if (!elem)
		return -ENOMEM;
	err = snd_ctl_elem_id_malloc(&elem->id);
	if (err < 0)
		goto _err;
	err = snd_ctl_elem_info_malloc(&elem->info);
	if (err < 0)
		goto _err;
	err = snd_ctl_elem_value_malloc(&elem->val);
	if (err < 0)
		goto _err;
	err = snd_ctl_elem_value_malloc(&elem->mask);
	if (err < 0)
		goto _err;
	err = snd_ctl_elem_value_malloc(&elem->old);
	if (err < 0)
		goto _err;
	elem->lock = lock;
	elem->preserve = preserve;
	snd_ctl_elem_id_set_interface(elem->id, iface);
	snd_ctl_elem_id_set_name(elem->id, name);
	snd_ctl_elem_id_set_index(elem->id, index);
	snd_ctl_elem_id_set_device(elem->id, device);
	snd_ctl_elem_id_set_subdevice(elem->id, subdevice);
	snd_ctl_elem_info_set_id(elem->info, elem->id);
	err = snd_ctl_elem_info(h->ctl, elem->info);
	if (err < 0) {
		if (! optional)
			SNDERR("Cannot obtain info for CTL elem (%s,'%s',%li,%li,%li): %s", snd_ctl_elem_iface_name(iface), name, index, device, subdevice, snd_strerror(err));
		goto _err;
	}
	snd_ctl_elem_value_set_id(elem->val, elem->id);
	snd_ctl_elem_value_set_id(elem->old, elem->id);
	if (mask) {
		err = snd_config_get_ctl_elem_value(value, h->ctl, elem->val, NULL, elem->info);
		if (err < 0)
			goto _err;
		err = snd_config_get_ctl_elem_value(mask, h->ctl, elem->mask, NULL, elem->info);
		if (err < 0)
			goto _err;
	} else {
		err = snd_config_get_ctl_elem_value(value, h->ctl, elem->val, elem->mask, elem->info);
		if (err < 0)
			goto _err;
	}
		
	err = snd_config_get_ctl_elem_value(value, h->ctl, elem->val, elem->mask, elem->info);
	if (err < 0)
		goto _err;
	list_add_tail(&elem->list, &h->elems);

 _err:
 	if (err < 0 && elem) {
		if (elem->id)
			snd_ctl_elem_id_free(elem->id);
		if (elem->info)
			snd_ctl_elem_info_free(elem->info);
		if (elem->val)
			snd_ctl_elem_value_free(elem->val);
		if (elem->mask)
			snd_ctl_elem_value_free(elem->mask);
		if (elem->old)
			snd_ctl_elem_value_free(elem->old);
		free(elem);
		if (err != -ENOMEM && optional)
			err = 0; /* ignore the error */
	}
	if (conf)
		snd_config_delete(conf);
	return err;
}

/**
 * \brief Build setup control handle
 * \param sctl Result - setup control handle
 * \param handle Master control handle
 * \param conf Setup configuration
 * \param private_data Private data for runtime evaluation
 * \param mode Build mode - SND_SCTL_xxxx
 * \result zero if success, otherwise a negative error code
 */
int snd_sctl_build(snd_sctl_t **sctl, snd_ctl_t *handle, snd_config_t *conf, snd_config_t *private_data, int mode)
{
	snd_sctl_t *h;
	snd_config_iterator_t i, next;
	int err;

	assert(sctl);
	assert(handle);
	assert(conf);
	*sctl = NULL;
	if (snd_config_get_type(conf) != SND_CONFIG_TYPE_COMPOUND)
		return -EINVAL;
	h = calloc(1, sizeof(*h));
	if (!h) {
		if (mode & SND_SCTL_NOFREE)
			return -ENOMEM;
		snd_ctl_close(handle);
		return -ENOMEM;
	}
	h->mode = mode;
	h->ctl = handle;
	INIT_LIST_HEAD(&h->elems);
	snd_config_for_each(i, next, conf) {
		snd_config_t *n = snd_config_iterator_entry(i);
		err = add_elem(h, n, private_data);
		if (err < 0) {
			free_elems(h);
			return err;
		}
	}
	*sctl = h;
	return 0;
}

/**
 * \brief Free setup control handle
 * \param sctl Setup control handle
 * \result zero if success, otherwise a negative error code
 */
int snd_sctl_free(snd_sctl_t *sctl)
{
	assert(sctl);
	return free_elems(sctl);
}
