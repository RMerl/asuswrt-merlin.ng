/*
 *  Advanced Linux Sound Architecture Control Program
 *  Copyright (c) by Abramo Bagnara <abramo@alsa-project.org>
 *                   Jaroslav Kysela <perex@perex.cz>
 *
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 */

#include "aconfig.h"
#include "version.h"
#include <getopt.h>
#include <stdarg.h>
#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include <alsa/asoundlib.h>
#include "alsactl.h"


#define ARRAY_SIZE(a) (sizeof (a) / sizeof (a)[0])


static char *id_str(snd_ctl_elem_id_t *id)
{
	static char str[128];
	assert(id);
	sprintf(str, "%i,%i,%i,%s,%i", 
		snd_ctl_elem_id_get_interface(id),
		snd_ctl_elem_id_get_device(id),
		snd_ctl_elem_id_get_subdevice(id),
		snd_ctl_elem_id_get_name(id),
		snd_ctl_elem_id_get_index(id));
	return str;
}

static char *num_str(long n)
{
	static char str[32];
	sprintf(str, "%ld", n);
	return str;
}

static int snd_config_integer_add(snd_config_t *father, char *id, long integer)
{
	int err;
	snd_config_t *leaf;
	err = snd_config_make_integer(&leaf, id);
	if (err < 0)
		return err;
	err = snd_config_add(father, leaf);
	if (err < 0) {
		snd_config_delete(leaf);
		return err;
	}
	err = snd_config_set_integer(leaf, integer);
	if (err < 0) {
		snd_config_delete(leaf);
		return err;
	}
	return 0;
}

static int snd_config_integer64_add(snd_config_t *father, char *id, long long integer)
{
	int err;
	snd_config_t *leaf;
	err = snd_config_make_integer64(&leaf, id);
	if (err < 0)
		return err;
	err = snd_config_add(father, leaf);
	if (err < 0) {
		snd_config_delete(leaf);
		return err;
	}
	err = snd_config_set_integer64(leaf, integer);
	if (err < 0) {
		snd_config_delete(leaf);
		return err;
	}
	return 0;
}

static int snd_config_string_add(snd_config_t *father, const char *id, const char *string)
{
	int err;
	snd_config_t *leaf;
	err = snd_config_make_string(&leaf, id);
	if (err < 0)
		return err;
	err = snd_config_add(father, leaf);
	if (err < 0) {
		snd_config_delete(leaf);
		return err;
	}
	err = snd_config_set_string(leaf, string);
	if (err < 0) {
		snd_config_delete(leaf);
		return err;
	}
	return 0;
}

static int snd_config_compound_add(snd_config_t *father, const char *id, int join,
				   snd_config_t **node)
{
	int err;
	snd_config_t *leaf;
	err = snd_config_make_compound(&leaf, id, join);
	if (err < 0)
		return err;
	err = snd_config_add(father, leaf);
	if (err < 0) {
		snd_config_delete(leaf);
		return err;
	}
	*node = leaf;
	return 0;
}

#define MAX_USER_TLV_SIZE	64

static char *tlv_to_str(unsigned int *tlv)
{
	int i, len = tlv[1] / 4 + 2;
	char *s, *p;

	if (len >= MAX_USER_TLV_SIZE)
		return NULL;
	s = malloc(len * 8 + 1);
	if (! s)
		return NULL;
	p = s;
	for (i = 0; i < len; i++) {
		sprintf(p, "%08x", tlv[i]);
		p += 8;
	}
	return s;
}

static unsigned int *str_to_tlv(const char *s)
{
	int i, j, c, len;
	unsigned int *tlv;
			
	len = strlen(s);
	if (len % 8) /* aligned to 4 bytes (= 8 letters) */
		return NULL;
	len /= 8;
	if (len > MAX_USER_TLV_SIZE)
		return NULL;
	tlv = malloc(sizeof(int) * len);
	if (! tlv)
		return NULL;
	for (i = 0; i < len; i++) {
		tlv[i] = 0;
		for (j = 0; j < 8; j++) {
			if ((c = hextodigit(*s++)) < 0) {
				free(tlv);
				return NULL;
			}
			tlv[i] = (tlv[i] << 4) | c;
		}
	}
	return tlv;
}

/*
 * add the TLV string and dB ranges to comment fields
 */
static int add_tlv_comments(snd_ctl_t *handle, snd_ctl_elem_id_t *id,
			    snd_ctl_elem_info_t *info, snd_config_t *comment)
{
	unsigned int tlv[MAX_USER_TLV_SIZE];
	unsigned int *db;
	long dbmin, dbmax;
	int err;

	if (snd_ctl_elem_tlv_read(handle, id, tlv, sizeof(tlv)) < 0)
		return 0; /* ignore error */

	if (snd_ctl_elem_info_is_tlv_writable(info)) {
		char *s = tlv_to_str(tlv);
		if (s) {
			err = snd_config_string_add(comment, "tlv", s);
			if (err < 0) {
				error("snd_config_string_add: %s", snd_strerror(err));
				return err;
			}
			free(s);
		}
	}

	err = snd_tlv_parse_dB_info(tlv, sizeof(tlv), &db);
	if (err <= 0)
		return 0;

	snd_tlv_get_dB_range(db, snd_ctl_elem_info_get_min(info),
			     snd_ctl_elem_info_get_max(info),
			     &dbmin, &dbmax);
	if (err < 0)
		return err;
	snd_config_integer_add(comment, "dbmin", dbmin);
	snd_config_integer_add(comment, "dbmax", dbmax);
	return 0;
}

static int get_control(snd_ctl_t *handle, snd_ctl_elem_id_t *id, snd_config_t *top)
{
	snd_ctl_elem_value_t *ctl;
	snd_ctl_elem_info_t *info;
	snd_config_t *control, *comment, *item, *value;
	const char *s;
	char buf[256];
	unsigned int idx;
	int err;
	unsigned int device, subdevice, index;
	const char *name;
	snd_ctl_elem_type_t type;
	unsigned int count;
	snd_ctl_elem_value_alloca(&ctl);
	snd_ctl_elem_info_alloca(&info);
	snd_ctl_elem_info_set_id(info, id);
	err = snd_ctl_elem_info(handle, info);
	if (err < 0) {
		error("Cannot read control info '%s': %s", id_str(id), snd_strerror(err));
		return err;
	}

	if (snd_ctl_elem_info_is_inactive(info) ||
				!snd_ctl_elem_info_is_readable(info))
		return 0;
	snd_ctl_elem_value_set_id(ctl, id);
	err = snd_ctl_elem_read(handle, ctl);
	if (err < 0) {
		error("Cannot read control '%s': %s", id_str(id), snd_strerror(err));
		return err;
	}

	err = snd_config_compound_add(top, num_str(snd_ctl_elem_info_get_numid(info)), 0, &control);
	if (err < 0) {
		error("snd_config_compound_add: %s", snd_strerror(err));
		return err;
	}
	err = snd_config_compound_add(control, "comment", 1, &comment);
	if (err < 0) {
		error("snd_config_compound_add: %s", snd_strerror(err));
		return err;
	}

	buf[0] = '\0';
	buf[1] = '\0';
	if (snd_ctl_elem_info_is_readable(info))
		strcat(buf, " read");
	if (snd_ctl_elem_info_is_writable(info))
		strcat(buf, " write");
	if (snd_ctl_elem_info_is_inactive(info))
		strcat(buf, " inactive");
	if (snd_ctl_elem_info_is_volatile(info))
		strcat(buf, " volatile");
	if (snd_ctl_elem_info_is_locked(info))
		strcat(buf, " locked");
	if (snd_ctl_elem_info_is_user(info))
		strcat(buf, " user");
	err = snd_config_string_add(comment, "access", buf + 1);
	if (err < 0) {
		error("snd_config_string_add: %s", snd_strerror(err));
		return err;
	}

	type = snd_ctl_elem_info_get_type(info);
	device = snd_ctl_elem_info_get_device(info);
	subdevice = snd_ctl_elem_info_get_subdevice(info);
	index = snd_ctl_elem_info_get_index(info);
	name = snd_ctl_elem_info_get_name(info);
	count = snd_ctl_elem_info_get_count(info);
	s = snd_ctl_elem_type_name(type);
	err = snd_config_string_add(comment, "type", s);
	if (err < 0) {
		error("snd_config_string_add: %s", snd_strerror(err));
		return err;
	}
	err = snd_config_integer_add(comment, "count", count);
	if (err < 0) {
		error("snd_config_integer_add: %s", snd_strerror(err));
		return err;
	}

	switch (type) {
	case SND_CTL_ELEM_TYPE_BOOLEAN:
		break;
	case SND_CTL_ELEM_TYPE_INTEGER:
	{
		long min = snd_ctl_elem_info_get_min(info);
		long max = snd_ctl_elem_info_get_max(info);
		long step = snd_ctl_elem_info_get_step(info);
		if (step)
			sprintf(buf, "%li - %li (step %li)", min, max, step);
		else
			sprintf(buf, "%li - %li", min, max);
		err = snd_config_string_add(comment, "range", buf);
		if (err < 0) {
			error("snd_config_string_add: %s", snd_strerror(err));
			return err;
		}
		if (snd_ctl_elem_info_is_tlv_readable(info)) {
			err = add_tlv_comments(handle, id, info, comment);
			if (err < 0)
				return err;
		}
		break;
	}
	case SND_CTL_ELEM_TYPE_INTEGER64:
	{
		long long min = snd_ctl_elem_info_get_min64(info);
		long long max = snd_ctl_elem_info_get_max64(info);
		long long step = snd_ctl_elem_info_get_step64(info);
		if (step)
			sprintf(buf, "%Li - %Li (step %Li)", min, max, step);
		else
			sprintf(buf, "%Li - %Li", min, max);
		err = snd_config_string_add(comment, "range", buf);
		if (err < 0) {
			error("snd_config_string_add: %s", snd_strerror(err));
			return err;
		}
		break;
	}
	case SND_CTL_ELEM_TYPE_ENUMERATED:
	{
		unsigned int items;
		err = snd_config_compound_add(comment, "item", 1, &item);
		if (err < 0) {
			error("snd_config_compound_add: %s", snd_strerror(err));
			return err;
		}
		items = snd_ctl_elem_info_get_items(info);
		for (idx = 0; idx < items; idx++) {
			snd_ctl_elem_info_set_item(info, idx);
			err = snd_ctl_elem_info(handle, info);
			if (err < 0) {
				error("snd_ctl_card_info: %s", snd_strerror(err));
				return err;
			}
			err = snd_config_string_add(item, num_str(idx), snd_ctl_elem_info_get_item_name(info));
			if (err < 0) {
				error("snd_config_string_add: %s", snd_strerror(err));
				return err;
			}
		}
		break;
	}
	default:
		break;
	}
	s = snd_ctl_elem_iface_name(snd_ctl_elem_info_get_interface(info));
	err = snd_config_string_add(control, "iface", s);
	if (err < 0) {
		error("snd_config_string_add: %s", snd_strerror(err));
		return err;
	}
	if (device != 0) {
		err = snd_config_integer_add(control, "device", device);
		if (err < 0) {
			error("snd_config_integer_add: %s", snd_strerror(err));
			return err;
		}
	}
	if (subdevice != 0) {
		err = snd_config_integer_add(control, "subdevice", subdevice);
		if (err < 0) {
			error("snd_config_integer_add: %s", snd_strerror(err));
			return err;
		}
	}
	err = snd_config_string_add(control, "name", name);
	if (err < 0) {
		error("snd_config_string_add: %s", snd_strerror(err));
		return err;
	}
	if (index != 0) {
		err = snd_config_integer_add(control, "index", index);
		if (err < 0) {
			error("snd_config_integer_add: %s", snd_strerror(err));
			return err;
		}
	}

	switch (type) {
	case SND_CTL_ELEM_TYPE_BYTES:
	case SND_CTL_ELEM_TYPE_IEC958:
	{
		size_t size = type == SND_CTL_ELEM_TYPE_BYTES ?
			count : sizeof(snd_aes_iec958_t);
		char buf[size * 2 + 1];
		char *p = buf;
		char *hex = "0123456789abcdef";
		const unsigned char *bytes = 
		  (const unsigned char *)snd_ctl_elem_value_get_bytes(ctl);
		for (idx = 0; idx < size; idx++) {
			int v = bytes[idx];
			*p++ = hex[v >> 4];
			*p++ = hex[v & 0x0f];
		}
		*p = '\0';
		err = snd_config_string_add(control, "value", buf);
		if (err < 0) {
			error("snd_config_string_add: %s", snd_strerror(err));
			return err;
		}
		return 0;
	}
	default:
		break;
	}

	if (count == 1) {
		switch (type) {
		case SND_CTL_ELEM_TYPE_BOOLEAN:
			err = snd_config_string_add(control, "value", snd_ctl_elem_value_get_boolean(ctl, 0) ? "true" : "false");
			if (err < 0) {
				error("snd_config_string_add: %s", snd_strerror(err));
				return err;
			}
			return 0;
		case SND_CTL_ELEM_TYPE_INTEGER:
			err = snd_config_integer_add(control, "value", snd_ctl_elem_value_get_integer(ctl, 0));
			if (err < 0) {
				error("snd_config_integer_add: %s", snd_strerror(err));
				return err;
			}
			return 0;
		case SND_CTL_ELEM_TYPE_INTEGER64:
			err = snd_config_integer64_add(control, "value", snd_ctl_elem_value_get_integer64(ctl, 0));
			if (err < 0) {
				error("snd_config_integer64_add: %s", snd_strerror(err));
				return err;
			}
			return 0;
		case SND_CTL_ELEM_TYPE_ENUMERATED:
		{
			unsigned int v = snd_ctl_elem_value_get_enumerated(ctl, 0);
			snd_config_t *c;
			err = snd_config_search(item, num_str(v), &c);
			if (err == 0) {
				err = snd_config_get_string(c, &s);
				assert(err == 0);
				err = snd_config_string_add(control, "value", s);
			} else {
				err = snd_config_integer_add(control, "value", v);
			}
			if (err < 0)
				error("snd_config add: %s", snd_strerror(err));
			return 0;
		}
		default:
			error("Unknown control type: %d\n", type);
			return -EINVAL;
		}
	}

	err = snd_config_compound_add(control, "value", 1, &value);
	if (err < 0) {
		error("snd_config_compound_add: %s", snd_strerror(err));
		return err;
	}

	switch (type) {
	case SND_CTL_ELEM_TYPE_BOOLEAN:
		for (idx = 0; idx < count; idx++) {
			err = snd_config_string_add(value, num_str(idx), snd_ctl_elem_value_get_boolean(ctl, idx) ? "true" : "false");
			if (err < 0) {
				error("snd_config_string_add: %s", snd_strerror(err));
				return err;
			}
		}
		break;
	case SND_CTL_ELEM_TYPE_INTEGER:
		for (idx = 0; idx < count; idx++) {
			err = snd_config_integer_add(value, num_str(idx), snd_ctl_elem_value_get_integer(ctl, idx));
			if (err < 0) {
				error("snd_config_integer_add: %s", snd_strerror(err));
				return err;
			}
		}
		break;
	case SND_CTL_ELEM_TYPE_INTEGER64:
		for (idx = 0; idx < count; idx++) {
			err = snd_config_integer64_add(value, num_str(idx), snd_ctl_elem_value_get_integer64(ctl, idx));
			if (err < 0) {
				error("snd_config_integer64_add: %s", snd_strerror(err));
				return err;
			}
		}
		break;
	case SND_CTL_ELEM_TYPE_ENUMERATED:
		for (idx = 0; idx < count; idx++) {
			unsigned int v = snd_ctl_elem_value_get_enumerated(ctl, idx);
			snd_config_t *c;
			err = snd_config_search(item, num_str(v), &c);
			if (err == 0) {
				err = snd_config_get_string(c, &s);
				assert(err == 0);
				err = snd_config_string_add(value, num_str(idx), s);
			} else {
				err = snd_config_integer_add(value, num_str(idx), v);
			}
			if (err < 0) {
				error("snd_config add: %s", snd_strerror(err));
				return err;
			}
		}
		break;
	default:
		error("Unknown control type: %d\n", type);
		return -EINVAL;
	}
	
	return 0;
}
	
static int get_controls(int cardno, snd_config_t *top)
{
	snd_ctl_t *handle;
	snd_ctl_card_info_t *info;
	snd_config_t *state, *card, *control;
	snd_ctl_elem_list_t *list;
	unsigned int idx;
	int err;
	char name[32];
	unsigned int count;
	const char *id;
	snd_ctl_card_info_alloca(&info);
	snd_ctl_elem_list_alloca(&list);

	sprintf(name, "hw:%d", cardno);
	err = snd_ctl_open(&handle, name, SND_CTL_READONLY);
	if (err < 0) {
		error("snd_ctl_open error: %s", snd_strerror(err));
		return err;
	}
	err = snd_ctl_card_info(handle, info);
	if (err < 0) {
		error("snd_ctl_card_info error: %s", snd_strerror(err));
		goto _close;
	}
	id = snd_ctl_card_info_get_id(info);
	err = snd_config_search(top, "state", &state);
	if (err == 0 &&
	    snd_config_get_type(state) != SND_CONFIG_TYPE_COMPOUND) {
		error("config state node is not a compound");
		err = -EINVAL;
		goto _close;
	}
	if (err < 0) {
		err = snd_config_compound_add(top, "state", 1, &state);
		if (err < 0) {
			error("snd_config_compound_add: %s", snd_strerror(err));
			goto _close;
		}
	}
	err = snd_config_search(state, id, &card);
	if (err == 0 &&
	    snd_config_get_type(card) != SND_CONFIG_TYPE_COMPOUND) {
		error("config state.%s node is not a compound", id);
		err = -EINVAL;
		goto _close;
	}
	if (err < 0) {
		err = snd_config_compound_add(state, id, 0, &card);
		if (err < 0) {
			error("snd_config_compound_add: %s", snd_strerror(err));
			goto _close;
		}
	}
	err = snd_config_search(card, "control", &control);
	if (err == 0) {
		err = snd_config_delete(control);
		if (err < 0) {
			error("snd_config_delete: %s", snd_strerror(err));
			goto _close;
		}
	}
	err = snd_ctl_elem_list(handle, list);
	if (err < 0) {
		error("Cannot determine controls: %s", snd_strerror(err));
		goto _close;
	}
	count = snd_ctl_elem_list_get_count(list);
	err = snd_config_compound_add(card, "control", count > 0, &control);
	if (err < 0) {
		error("snd_config_compound_add: %s", snd_strerror(err));
		goto _close;
	}
	if (count == 0) {
		err = 0;
		goto _close;
	}
	snd_ctl_elem_list_set_offset(list, 0);
	if (snd_ctl_elem_list_alloc_space(list, count) < 0) {
		error("No enough memory...");
		goto _close;
	}
	if ((err = snd_ctl_elem_list(handle, list)) < 0) {
		error("Cannot determine controls (2): %s", snd_strerror(err));
		goto _free;
	}
	for (idx = 0; idx < count; ++idx) {
		snd_ctl_elem_id_t *id;
		snd_ctl_elem_id_alloca(&id);
		snd_ctl_elem_list_get_id(list, idx, id);
		err = get_control(handle, id, control);
		if (err < 0)
			goto _free;
	}		
		
	err = 0;
 _free:
	snd_ctl_elem_list_free_space(list);
 _close:
	snd_ctl_close(handle);
	return err;
}

static long config_iface(snd_config_t *n)
{
	long i;
	long long li;
	snd_ctl_elem_iface_t idx; 
	const char *str;
	switch (snd_config_get_type(n)) {
	case SND_CONFIG_TYPE_INTEGER:
		snd_config_get_integer(n, &i);
		return i;
	case SND_CONFIG_TYPE_INTEGER64:
		snd_config_get_integer64(n, &li);
		return li;
	case SND_CONFIG_TYPE_STRING:
		snd_config_get_string(n, &str);
		break;
	default:
		return -1;
	}
	for (idx = 0; idx <= SND_CTL_ELEM_IFACE_LAST; idx++) {
		if (strcasecmp(snd_ctl_elem_iface_name(idx), str) == 0)
			return idx;
	}
	return -1;
}

static int config_bool(snd_config_t *n, int doit)
{
	const char *str;
	long val;
	long long lval;

	switch (snd_config_get_type(n)) {
	case SND_CONFIG_TYPE_INTEGER:
		snd_config_get_integer(n, &val);
		if (val < 0 || val > 1)
			return -1;
		return val;
	case SND_CONFIG_TYPE_INTEGER64:
		snd_config_get_integer64(n, &lval);
		if (lval < 0 || lval > 1)
			return -1;
		return (int) lval;
	case SND_CONFIG_TYPE_STRING:
		snd_config_get_string(n, &str);
		break;
	case SND_CONFIG_TYPE_COMPOUND:
		if (!force_restore || !doit)
			return -1;
		n = snd_config_iterator_entry(snd_config_iterator_first(n));
		return config_bool(n, doit);
	default:
		return -1;
	}
	if (strcmp(str, "on") == 0 || strcmp(str, "true") == 0)
		return 1;
	if (strcmp(str, "off") == 0 || strcmp(str, "false") == 0)
		return 0;
	return -1;
}

static int config_enumerated(snd_config_t *n, snd_ctl_t *handle,
			     snd_ctl_elem_info_t *info, int doit)
{
	const char *str;
	long val;
	long long lval;
	unsigned int idx, items;

	switch (snd_config_get_type(n)) {
	case SND_CONFIG_TYPE_INTEGER:
		snd_config_get_integer(n, &val);
		return val;
	case SND_CONFIG_TYPE_INTEGER64:
		snd_config_get_integer64(n, &lval);
		return (int) lval;
	case SND_CONFIG_TYPE_STRING:
		snd_config_get_string(n, &str);
		break;
	case SND_CONFIG_TYPE_COMPOUND:
		if (!force_restore || !doit)
			return -1;
		n = snd_config_iterator_entry(snd_config_iterator_first(n));
		return config_enumerated(n, handle, info, doit);
	default:
		return -1;
	}
	items = snd_ctl_elem_info_get_items(info);
	for (idx = 0; idx < items; idx++) {
		int err;
		snd_ctl_elem_info_set_item(info, idx);
		err = snd_ctl_elem_info(handle, info);
		if (err < 0) {
			error("snd_ctl_elem_info: %s", snd_strerror(err));
			return err;
		}
		if (strcmp(str, snd_ctl_elem_info_get_item_name(info)) == 0)
			return idx;
	}
	return -1;
}

static int config_integer(snd_config_t *n, long *val, int doit)
{
	int err = snd_config_get_integer(n, val);
	if (err < 0 && force_restore && doit) {
		if (snd_config_get_type(n) != SND_CONFIG_TYPE_COMPOUND)
			return err;
		n = snd_config_iterator_entry(snd_config_iterator_first(n));
		return config_integer(n, val, doit);
	}
	return err;
}

static int config_integer64(snd_config_t *n, long long *val, int doit)
{
	int err = snd_config_get_integer64(n, val);
	if (err < 0 && force_restore && doit) {
		if (snd_config_get_type(n) != SND_CONFIG_TYPE_COMPOUND)
			return err;
		n = snd_config_iterator_entry(snd_config_iterator_first(n));
		return config_integer64(n, val, doit);
	}
	return err;
}

static int is_user_control(snd_config_t *conf)
{
	snd_config_iterator_t i, next;

	snd_config_for_each(i, next, conf) {
		snd_config_t *n = snd_config_iterator_entry(i);
		const char *id, *s;
		if (snd_config_get_id(n, &id) < 0)
			continue;
		if (strcmp(id, "access") == 0) {
			if (snd_config_get_string(n, &s) < 0)
				return 0;
			if (strstr(s, "user"))
				return 1;
		}
	}
	return 0;
}

/*
 * get the item type from the given comment config
 */
static int get_comment_type(snd_config_t *n)
{
	static const snd_ctl_elem_type_t types[] = {
		SND_CTL_ELEM_TYPE_BOOLEAN,
		SND_CTL_ELEM_TYPE_INTEGER,
		SND_CTL_ELEM_TYPE_ENUMERATED,
		SND_CTL_ELEM_TYPE_BYTES,
		SND_CTL_ELEM_TYPE_IEC958,
		SND_CTL_ELEM_TYPE_INTEGER64,
	};
	const char *type;
	unsigned int i;

	if (snd_config_get_string(n, &type) < 0)
		return -EINVAL;
	for (i = 0; i < ARRAY_SIZE(types); ++i)
		if (strcmp(type, snd_ctl_elem_type_name(types[i])) == 0)
			return types[i];
	return -EINVAL;
}

/*
 * get the value range from the given comment config
 */
static int get_comment_range(snd_config_t *n, int ctype,
			     long *imin, long *imax, long *istep)
{
	const char *s;
	int err;

	if (snd_config_get_string(n, &s) < 0)
		return -EINVAL;
	switch (ctype) {
	case SND_CTL_ELEM_TYPE_INTEGER:
		err = sscanf(s, "%li - %li (step %li)", imin, imax, istep);
		if (err != 3) {
			istep = 0;
			err = sscanf(s, "%li - %li", imin, imax);
			if (err != 2)
				return -EINVAL;
		}
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

static int add_user_control(snd_ctl_t *handle, snd_ctl_elem_info_t *info, snd_config_t *conf)
{
	snd_ctl_elem_id_t *id;
	snd_config_iterator_t i, next;
	long imin, imax, istep;
	snd_ctl_elem_type_t ctype;
	unsigned int count;
	int err;
	unsigned int *tlv;

	imin = imax = istep = 0;
	count = 0;
	ctype = SND_CTL_ELEM_TYPE_NONE;
	tlv = NULL;
	snd_config_for_each(i, next, conf) {
		snd_config_t *n = snd_config_iterator_entry(i);
		const char *id;
		if (snd_config_get_id(n, &id) < 0)
			continue;
		if (strcmp(id, "type") == 0) {
			err = get_comment_type(n);
			if (err < 0)
				return err;
			ctype = err;
			continue;
		}
		if (strcmp(id, "range") == 0) {
			err = get_comment_range(n, ctype, &imin, &imax, &istep);
			if (err < 0)
				return err;
			continue;
		}
		if (strcmp(id, "count") == 0) {
			long v;
			if ((err = snd_config_get_integer(n, &v)) < 0)
				return err;
			count = v;
			continue;
		}
		if (strcmp(id, "tlv") == 0) {
			const char *s;
			if ((err = snd_config_get_string(n, &s)) < 0)
				return -EINVAL;
			if (tlv)
				free(tlv);
			if ((tlv = str_to_tlv(s)) == NULL)
				return -EINVAL;
			continue;
		}
	}

	snd_ctl_elem_id_alloca(&id);
	snd_ctl_elem_info_get_id(info, id);
	if (count <= 0)
		count = 1;
	switch (ctype) {
	case SND_CTL_ELEM_TYPE_INTEGER:
		if (imin > imax || istep > imax - imin)
			return -EINVAL;
		err = snd_ctl_elem_add_integer(handle, id, count, imin, imax, istep);
		if (err < 0)
			goto error;
		if (tlv)
			snd_ctl_elem_tlv_write(handle, id, tlv);
		break;
	case SND_CTL_ELEM_TYPE_BOOLEAN:
		err = snd_ctl_elem_add_boolean(handle, id, count);
		break;
	case SND_CTL_ELEM_TYPE_IEC958:
		err = snd_ctl_elem_add_iec958(handle, id);
		break;
	default:
		err = -EINVAL;
		break;
	}

 error:
	free(tlv);
	if (err < 0)
		return err;
	return snd_ctl_elem_info(handle, info);
}

/*
 * look for a config node with the given item name
 */
static snd_config_t *search_comment_item(snd_config_t *conf, const char *name)
{
	snd_config_iterator_t i, next;
	snd_config_for_each(i, next, conf) {
		snd_config_t *n = snd_config_iterator_entry(i);
		const char *id;
		if (snd_config_get_id(n, &id) < 0)
			continue;
		if (strcmp(id, name) == 0)
			return n;
	}
	return NULL;
}

/*
 * check whether the config item has the same of compatible type
 */
static int check_comment_type(snd_config_t *conf, int type)
{
	snd_config_t *n = search_comment_item(conf, "type");
	int ctype;

	if (!n)
		return 0; /* not defined */
	ctype = get_comment_type(n);
	if (ctype == type)
		return 0;
	if ((ctype == SND_CTL_ELEM_TYPE_BOOLEAN ||
	     ctype == SND_CTL_ELEM_TYPE_INTEGER ||
	     ctype == SND_CTL_ELEM_TYPE_INTEGER64 ||
	     ctype == SND_CTL_ELEM_TYPE_ENUMERATED) &&
	    (type == SND_CTL_ELEM_TYPE_BOOLEAN ||
	     type == SND_CTL_ELEM_TYPE_INTEGER ||
	     type == SND_CTL_ELEM_TYPE_INTEGER64 ||
	     type == SND_CTL_ELEM_TYPE_ENUMERATED))
		return 0; /* OK, compatible */
	return -EINVAL;
}

/*
 * convert from an old value to a new value with the same dB level
 */
static int convert_to_new_db(snd_config_t *value, long omin, long omax,
			     long nmin, long nmax,
			     long odbmin, long odbmax,
			     long ndbmin, long ndbmax,
			     int doit)
{
	long val;
	if (config_integer(value, &val, doit) < 0)
		return -EINVAL;
	if (val < omin || val > omax)
		return -EINVAL;
	val = ((val - omin) * (odbmax - odbmin)) / (omax - omin) + odbmin;
	if (val < ndbmin)
		val = ndbmin;
	else if (val > ndbmax)
		val = ndbmax;
	val = ((val - ndbmin) * (nmax - nmin)) / (ndbmax - ndbmin) + nmin;
	return snd_config_set_integer(value, val);
}

/*
 * compare the current value range with the old range in comments.
 * also, if dB information is available, try to compare them.
 * if any change occurs, try to keep the same dB level.
 */
static int check_comment_range(snd_ctl_t *handle, snd_config_t *conf,
			       snd_ctl_elem_info_t *info, snd_config_t *value,
			       int doit)
{
	snd_config_t *n;
	long omin, omax, ostep;
	long nmin, nmax;
	long odbmin, odbmax;
	long ndbmin, ndbmax;
	snd_ctl_elem_id_t *id;

	n = search_comment_item(conf, "range");
	if (!n)
		return 0;
	if (get_comment_range(n, SND_CTL_ELEM_TYPE_INTEGER,
			      &omin, &omax, &ostep) < 0)
		return 0;
	nmin = snd_ctl_elem_info_get_min(info);
	nmax = snd_ctl_elem_info_get_max(info);
	if (omin != nmin && omax != nmax) {
		/* Hey, the range mismatches */
		if (!force_restore || !doit)
			return -EINVAL;
	}
	if (omin >= omax || nmin >= nmax)
		return 0; /* invalid values */

	n = search_comment_item(conf, "dbmin");
	if (!n)
		return 0;
	if (config_integer(n, &odbmin, doit) < 0)
		return 0;
	n = search_comment_item(conf, "dbmax");
	if (!n)
		return 0;
	if (config_integer(n, &odbmax, doit) < 0)
		return 0;
	if (odbmin >= odbmax)
		return 0; /* invalid values */
	snd_ctl_elem_id_alloca(&id);
	snd_ctl_elem_info_get_id(info, id);
	if (snd_ctl_get_dB_range(handle, id, &ndbmin, &ndbmax) < 0)
		return 0;
	if (ndbmin >= ndbmax)
		return 0; /* invalid values */
	if (omin == nmin && omax == nmax &&
	    odbmin == ndbmin && odbmax == ndbmax)
		return 0; /* OK, identical one */

	/* Let's guess the current value from dB range */
	if (snd_config_get_type(value) == SND_CONFIG_TYPE_COMPOUND) {
		snd_config_iterator_t i, next;
		snd_config_for_each(i, next, value) {
			snd_config_t *n = snd_config_iterator_entry(i);
			convert_to_new_db(n, omin, omax, nmin, nmax,
					  odbmin, odbmax, ndbmin, ndbmax, doit);
		}
	} else
		convert_to_new_db(value, omin, omax, nmin, nmax,
				  odbmin, odbmax, ndbmin, ndbmax, doit);
	return 0;
}

static int restore_config_value(snd_ctl_t *handle, snd_ctl_elem_info_t *info,
				snd_ctl_elem_iface_t type,
				snd_config_t *value,
				snd_ctl_elem_value_t *ctl, int idx,
				int doit)
{
	long val;
	long long lval;
	int err;

	switch (type) {
	case SND_CTL_ELEM_TYPE_BOOLEAN:
		val = config_bool(value, doit);
		if (val >= 0) {
			snd_ctl_elem_value_set_boolean(ctl, idx, val);
			return 1;
		}
		break;
	case SND_CTL_ELEM_TYPE_INTEGER:
		err = config_integer(value, &val, doit);
		if (err == 0) {
			snd_ctl_elem_value_set_integer(ctl, idx, val);
			return 1;
		}
		break;
	case SND_CTL_ELEM_TYPE_INTEGER64:
		err = config_integer64(value, &lval, doit);
		if (err == 0) {
			snd_ctl_elem_value_set_integer64(ctl, idx, lval);
			return 1;
		}
		break;
	case SND_CTL_ELEM_TYPE_ENUMERATED:
		val = config_enumerated(value, handle, info, doit);
		if (val >= 0) {
			snd_ctl_elem_value_set_enumerated(ctl, idx, val);
			return 1;
		}
		break;
	case SND_CTL_ELEM_TYPE_BYTES:
	case SND_CTL_ELEM_TYPE_IEC958:
		break;
	default:
		cerror(doit, "Unknow control type: %d", type);
		return -EINVAL;
	}
	return 0;
}

static int restore_config_value2(snd_ctl_t *handle, snd_ctl_elem_info_t *info,
				 snd_ctl_elem_iface_t type,
				 snd_config_t *value,
				 snd_ctl_elem_value_t *ctl, int idx,
				 unsigned int numid, int doit)
{
	int err = restore_config_value(handle, info, type, value, ctl, idx, doit);
	long val;

	if (err != 0)
		return err;
	switch (type) {
	case SND_CTL_ELEM_TYPE_BYTES:
	case SND_CTL_ELEM_TYPE_IEC958:
		err = snd_config_get_integer(value, &val);
		if (err < 0 || val < 0 || val > 255) {
			cerror(doit, "bad control.%d.value.%d content", numid, idx);
			return force_restore && doit ? 0 : -EINVAL;
		}
		snd_ctl_elem_value_set_byte(ctl, idx, val);
		return 1;
	default:
		break;
	}
	return 0;
}

static int set_control(snd_ctl_t *handle, snd_config_t *control,
		       int *maxnumid, int doit)
{
	snd_ctl_elem_value_t *ctl;
	snd_ctl_elem_info_t *info;
	snd_config_iterator_t i, next;
	unsigned int numid1;
	snd_ctl_elem_iface_t iface = -1;
	int iface1;
	const char *name1;
	unsigned int numid;
	snd_ctl_elem_type_t type;
	unsigned int count;
	long device = -1;
	long device1;
	long subdevice = -1;
	long subdevice1;
	const char *name = NULL;
	long index1;
	long index = -1;
	snd_config_t *value = NULL;
	snd_config_t *comment = NULL;
	unsigned int idx;
	int err;
	char *set;
	const char *id;
	snd_ctl_elem_value_alloca(&ctl);
	snd_ctl_elem_info_alloca(&info);
	if (snd_config_get_type(control) != SND_CONFIG_TYPE_COMPOUND) {
		cerror(doit, "control is not a compound");
		return -EINVAL;
	}
	err = snd_config_get_id(control, &id);
	if (err < 0) {
		cerror(doit, "unable to get id");
		return -EINVAL;
	}
	numid = atoi(id);
	if ((int)numid > *maxnumid)
		*maxnumid = numid;
	snd_config_for_each(i, next, control) {
		snd_config_t *n = snd_config_iterator_entry(i);
		const char *fld;
		if (snd_config_get_id(n, &fld) < 0)
			continue;
		if (strcmp(fld, "comment") == 0) {
			if (snd_config_get_type(n) != SND_CONFIG_TYPE_COMPOUND) {
				cerror(doit, "control.%d.%s is invalid", numid, fld);
				return -EINVAL;
			}
			comment = n;
			continue;
		}
		if (strcmp(fld, "iface") == 0) {
			iface = (snd_ctl_elem_iface_t)config_iface(n);
			continue;
		}
		if (strcmp(fld, "device") == 0) {
			if (snd_config_get_type(n) != SND_CONFIG_TYPE_INTEGER) {
				cerror(doit, "control.%d.%s is invalid", numid, fld);
				return -EINVAL;
			}
			snd_config_get_integer(n, &device);
			continue;
		}
		if (strcmp(fld, "subdevice") == 0) {
			if (snd_config_get_type(n) != SND_CONFIG_TYPE_INTEGER) {
				cerror(doit, "control.%d.%s is invalid", numid, fld);
				return -EINVAL;
			}
			snd_config_get_integer(n, &subdevice);
			continue;
		}
		if (strcmp(fld, "name") == 0) {
			if (snd_config_get_type(n) != SND_CONFIG_TYPE_STRING) {
				cerror(doit, "control.%d.%s is invalid", numid, fld);
				return -EINVAL;
			}
			snd_config_get_string(n, &name);
			continue;
		}
		if (strcmp(fld, "index") == 0) {
			if (snd_config_get_type(n) != SND_CONFIG_TYPE_INTEGER) {
				cerror(doit, "control.%d.%s is invalid", numid, fld);
				return -EINVAL;
			}
			snd_config_get_integer(n, &index);
			continue;
		}
		if (strcmp(fld, "value") == 0) {
			value = n;
			continue;
		}
		cerror(doit, "unknown control.%d.%s field", numid, fld);
	}
	if (!value) {
		cerror(doit, "missing control.%d.value", numid);
		return -EINVAL;
	}
	if (device < 0)
		device = 0;
	if (subdevice < 0)
		subdevice = 0;
	if (index < 0)
		index = 0;

	err = -EINVAL;
	if (!force_restore) {
		snd_ctl_elem_info_set_numid(info, numid);
		err = snd_ctl_elem_info(handle, info);
	}
	if (err < 0 && name) {
		snd_ctl_elem_info_set_numid(info, 0);
		snd_ctl_elem_info_set_interface(info, iface);
		snd_ctl_elem_info_set_device(info, device);
		snd_ctl_elem_info_set_subdevice(info, subdevice);
		snd_ctl_elem_info_set_name(info, name);
		snd_ctl_elem_info_set_index(info, index);
		err = snd_ctl_elem_info(handle, info);
		if (err < 0 && comment && is_user_control(comment)) {
			err = add_user_control(handle, info, comment);
			if (err < 0) {
				cerror(doit, "failed to add user control #%d (%s)",
				       numid, snd_strerror(err));
				return err;
			}
		}
	}
	if (err < 0) {
		cerror(doit, "failed to obtain info for control #%d (%s)", numid, snd_strerror(err));
		return -ENOENT;
	}
	numid1 = snd_ctl_elem_info_get_numid(info);
	iface1 = snd_ctl_elem_info_get_interface(info);
	device1 = snd_ctl_elem_info_get_device(info);
	subdevice1 = snd_ctl_elem_info_get_subdevice(info);
	name1 = snd_ctl_elem_info_get_name(info);
	index1 = snd_ctl_elem_info_get_index(info);
	count = snd_ctl_elem_info_get_count(info);
	type = snd_ctl_elem_info_get_type(info);
	if (err |= numid != numid1 && !force_restore)
		cerror(doit, "warning: numid mismatch (%d/%d) for control #%d", 
		      numid, numid1, numid);
	if (err |= iface != iface1)
		cerror(doit, "warning: iface mismatch (%d/%d) for control #%d", iface, iface1, numid);
	if (err |= device != device1)
		cerror(doit, "warning: device mismatch (%ld/%ld) for control #%d", device, device1, numid);
	if (err |= subdevice != subdevice1)
		cerror(doit, "warning: subdevice mismatch (%ld/%ld) for control #%d", subdevice, subdevice1, numid);
	if (err |= strcmp(name, name1))
		cerror(doit, "warning: name mismatch (%s/%s) for control #%d", name, name1, numid);
	if (err |= index != index1)
		cerror(doit, "warning: index mismatch (%ld/%ld) for control #%d", index, index1, numid);
	if (err < 0) {
		cerror(doit, "failed to obtain info for control #%d (%s)", numid, snd_strerror(err));
		return -ENOENT;
	}

	if (comment) {
		if (check_comment_type(comment, type) < 0)
			cerror(doit, "incompatible field type for control #%d", numid);
		if (type == SND_CTL_ELEM_TYPE_INTEGER) {
			if (check_comment_range(handle, comment, info, value, doit) < 0) {
				cerror(doit, "value range mismatch for control #%d",
				      numid);
				return -EINVAL;
			}
		}
	}

	if (snd_ctl_elem_info_is_inactive(info) ||
				!snd_ctl_elem_info_is_writable(info))
		return 0;
	snd_ctl_elem_value_set_numid(ctl, numid1);

	if (count == 1) {
		err = restore_config_value(handle, info, type, value, ctl, 0, doit);
		if (err < 0)
			return err;
		if (err > 0)
			goto _ok;
	}
	switch (type) {
	case SND_CTL_ELEM_TYPE_BYTES:
	case SND_CTL_ELEM_TYPE_IEC958:
	{
		const char *buf;
		err = snd_config_get_string(value, &buf);
		if (err >= 0) {
			int c1 = 0;
			int len = strlen(buf);
			unsigned int idx = 0;
			int size = type == SND_CTL_ELEM_TYPE_BYTES ?
				count : sizeof(snd_aes_iec958_t);
			if (size * 2 != len) {
				cerror(doit, "bad control.%d.value contents\n", numid);
				return -EINVAL;
			}
			while (*buf) {
				int c = *buf++;
				if ((c = hextodigit(c)) < 0) {
					cerror(doit, "bad control.%d.value contents\n", numid);
					return -EINVAL;
				}
				if (idx % 2 == 1)
					snd_ctl_elem_value_set_byte(ctl, idx / 2, c1 << 4 | c);
				else
					c1 = c;
				idx++;
			}
			goto _ok;
		}
	}
	default:
		break;
	}
	if (snd_config_get_type(value) != SND_CONFIG_TYPE_COMPOUND) {
		if (!force_restore || !doit) {
			cerror(doit, "bad control.%d.value type", numid);
			return -EINVAL;
		}
		for (idx = 0; idx < count; ++idx) {
			err = restore_config_value2(handle, info, type, value,
						    ctl, idx, numid, doit);
			if (err < 0)
				return err;
		}
		goto _ok;
	}

	set = (char*) alloca(count);
	memset(set, 0, count);
	snd_config_for_each(i, next, value) {
		snd_config_t *n = snd_config_iterator_entry(i);
		const char *id;
		if (snd_config_get_id(n, &id) < 0)
			continue;
		idx = atoi(id);
		if (idx >= count || set[idx]) {
			cerror(doit, "bad control.%d.value index", numid);
			if (!force_restore || !doit)
				return -EINVAL;
			continue;
		}
		err = restore_config_value2(handle, info, type, n,
					    ctl, idx, numid, doit);
		if (err < 0)
			return err;
		if (err > 0)
			set[idx] = 1;
	}
	for (idx = 0; idx < count; ++idx) {
		if (!set[idx]) {
			cerror(doit, "control.%d.value.%d is not specified", numid, idx);
			if (!force_restore || !doit)
				return -EINVAL;
		}
	}

 _ok:
	err = doit ? snd_ctl_elem_write(handle, ctl) : 0;
	if (err < 0) {
		error("Cannot write control '%d:%ld:%ld:%s:%ld' : %s", (int)iface, device, subdevice, name, index, snd_strerror(err));
		return err;
	}
	return 0;
}

static int set_controls(int card, snd_config_t *top, int doit)
{
	snd_ctl_t *handle;
	snd_ctl_card_info_t *info;
	snd_config_t *control;
	snd_config_iterator_t i, next;
	int err, maxnumid = -1;
	char name[32], tmpid[16];
	const char *id;
	snd_ctl_card_info_alloca(&info);

	sprintf(name, "hw:%d", card);
	dbg("device='%s', doit=%i", name, doit);
	err = snd_ctl_open(&handle, name, 0);
	if (err < 0) {
		error("snd_ctl_open error: %s", snd_strerror(err));
		return err;
	}
	err = snd_ctl_card_info(handle, info);
	if (err < 0) {
		error("snd_ctl_card_info error: %s", snd_strerror(err));
		goto _close;
	}
	id = snd_ctl_card_info_get_id(info);
	dbg("card-info-id: '%s'", id);
	err = snd_config_searchv(top, &control, "state", id, "control", 0);
	if (err < 0) {
		if (force_restore) {
			sprintf(tmpid, "card%d", card);
			err = snd_config_searchv(top, &control, "state", tmpid, "control", 0);
			if (! err)
				id = tmpid;
		}
		if (err < 0) {
			fprintf(stderr, "No state is present for card %s\n", id);
			goto _close;
		}
		id = tmpid;
	}
	if (snd_config_get_type(control) != SND_CONFIG_TYPE_COMPOUND) {
		cerror(doit, "state.%s.control is not a compound\n", id);
		return -EINVAL;
	}
	snd_config_for_each(i, next, control) {
		snd_config_t *n = snd_config_iterator_entry(i);
		err = set_control(handle, n, &maxnumid, doit);
		if (err < 0 && (!force_restore || !doit))
			goto _close;
	}

	dbg("maxnumid=%i", maxnumid);
	/* check if we have additional controls in driver */
	/* in this case we should go through init procedure */
	if (!doit && maxnumid >= 0) {
		snd_ctl_elem_info_t *info;
		snd_ctl_elem_info_alloca(&info);
		snd_ctl_elem_info_set_numid(info, maxnumid+1);
		if (snd_ctl_elem_info(handle, info) == 0) {
			/* not very informative */
			/* but value is used for check only */
			err = -EAGAIN;
			dbg("more controls than maxnumid?");
			goto _close;
		}
	}

 _close:
	snd_ctl_close(handle);
	dbg("result code: %i", err);
	return err;
}

int save_state(const char *file, const char *cardname)
{
	int err;
	snd_config_t *config;
	snd_input_t *in;
	snd_output_t *out;
	int stdio;

	err = snd_config_top(&config);
	if (err < 0) {
		error("snd_config_top error: %s", snd_strerror(err));
		return err;
	}
	stdio = !strcmp(file, "-");
	if (!stdio && (err = snd_input_stdio_open(&in, file, "r")) >= 0) {
		err = snd_config_load(config, in);
		snd_input_close(in);
	}

	if (!cardname) {
		int card, first = 1;

		card = -1;
		/* find each installed soundcards */
		while (1) {
			if (snd_card_next(&card) < 0)
				break;
			if (card < 0) {
				if (first) {
					if (ignore_nocards) {
						return 0;
					} else {
						error("No soundcards found...");
						return -ENODEV;
					}
				}
				break;
			}
			first = 0;
			if ((err = get_controls(card, config)))
				return err;
		}
	} else {
		int cardno;

		cardno = snd_card_get_index(cardname);
		if (cardno < 0) {
			error("Cannot find soundcard '%s'...", cardname);
			return cardno;
		}
		if ((err = get_controls(cardno, config))) {
			return err;
		}
	}
	
	if (stdio)
		err = snd_output_stdio_attach(&out, stdout, 0);
	else
		err = snd_output_stdio_open(&out, file, "w");
	if (err < 0) {
		error("Cannot open %s for writing: %s", file, snd_strerror(err));
		return -errno;
	}
	err = snd_config_save(config, out);
	snd_output_close(out);
	if (err < 0)
		error("snd_config_save: %s", snd_strerror(err));
	return 0;
}

int load_state(const char *file, const char *initfile, const char *cardname,
	       int do_init)
{
	int err, finalerr = 0;
	snd_config_t *config;
	snd_input_t *in;
	int stdio;

	err = snd_config_top(&config);
	if (err < 0) {
		error("snd_config_top error: %s", snd_strerror(err));
		return err;
	}
	stdio = !strcmp(file, "-");
	if (stdio)
		err = snd_input_stdio_attach(&in, stdin, 0);
	else
		err = snd_input_stdio_open(&in, file, "r");
	if (err >= 0) {
		err = snd_config_load(config, in);
		snd_input_close(in);
		if (err < 0) {
			error("snd_config_load error: %s", snd_strerror(err));
			return err;
		}
	} else {
		int card, first = 1;
		char cardname1[16];

		error("Cannot open %s for reading: %s", file, snd_strerror(err));
		finalerr = err;
		card = -1;
		/* find each installed soundcards */
		while (1) {
			if (snd_card_next(&card) < 0)
				break;
			if (card < 0)
				break;
			first = 0;
			if (!do_init)
				break;
			sprintf(cardname1, "%i", card);
			err = init(initfile, cardname1);
			if (err < 0) {
				finalerr = err;
				initfailed(card, "init", err);
			}
			initfailed(card, "restore", -ENOENT);
		}
		if (first)
			finalerr = 0;	/* no cards, no error code */
		return finalerr;
	}

	if (!cardname) {
		int card, first = 1;
		char cardname1[16];

		card = -1;
		/* find each installed soundcards */
		while (1) {
			if (snd_card_next(&card) < 0)
				break;
			if (card < 0) {
				if (first) {
					if (ignore_nocards) {
						return 0;
					} else {
						error("No soundcards found...");
						return -ENODEV;
					}
				}
				break;
			}
			first = 0;
			/* do a check if controls matches state file */
 			if (do_init && set_controls(card, config, 0)) {
				sprintf(cardname1, "%i", card);
				err = init(initfile, cardname1);
				if (err < 0) {
					initfailed(card, "init", err);
					finalerr = err;
				}
			}
			if ((err = set_controls(card, config, 1))) {
				if (!force_restore)
					finalerr = err;
				initfailed(card, "restore", err);
			}
		}
	} else {
		int cardno;

		cardno = snd_card_get_index(cardname);
		if (cardno < 0) {
			error("Cannot find soundcard '%s'...", cardname);
			return -ENODEV;
		}
		/* do a check if controls matches state file */
		if (do_init && set_controls(cardno, config, 0)) {
			err = init(initfile, cardname);
			if (err < 0) {
				initfailed(cardno, "init", err);
				finalerr = err;
			}
		}
		if ((err = set_controls(cardno, config, 1))) {
			initfailed(cardno, "restore", err);
			if (!force_restore)
				return err;
		}
	}
	return finalerr;
}
