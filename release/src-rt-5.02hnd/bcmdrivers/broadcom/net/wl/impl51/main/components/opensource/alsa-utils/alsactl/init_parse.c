/*
 *  Advanced Linux Sound Architecture Control Program - Parse initialization files
 *  Copyright (c) by Jaroslav Kysela <perex@perex.cz>,
 *		     Greg Kroah-Hartman <greg@kroah.com>,
 *		     Kay Sievers <kay.sievers@vrfy.org>
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

#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>
#include <fnmatch.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <sys/types.h>
#include <dirent.h>
#include <math.h>
#include <alsa/asoundlib.h>
#include "aconfig.h"
#include "alsactl.h"
#include "list.h"

#define PATH_SIZE	512
#define NAME_SIZE	128
#define EJUSTRETURN	0x7fffffff

enum key_op {
	KEY_OP_UNSET,
	KEY_OP_MATCH,
	KEY_OP_NOMATCH,
	KEY_OP_ADD,
	KEY_OP_ASSIGN,
	KEY_OP_ASSIGN_FINAL
};

struct pair {
	char *key;
	char *value;
	struct pair *next;
};

struct space {
	struct pair *pairs;
	char *rootdir;
	char *go_to;
	char *program_result;
	const char *filename;
	int linenum;
	int log_run;
	int exit_code;
	int quit;
	unsigned int ctl_id_changed;
	snd_hctl_t *ctl_handle;
	snd_ctl_card_info_t *ctl_card_info;
	snd_ctl_elem_id_t *ctl_id;
	snd_ctl_elem_info_t *ctl_info;
	snd_ctl_elem_value_t *ctl_value;
};

static void Perror(struct space *space, const char *fmt, ...)
{
	va_list arg;
	va_start(arg, fmt);
	fprintf(stderr, "%s:%i: ", space->filename, space->linenum);
	vfprintf(stderr, fmt, arg);
	putc('\n', stderr);
	va_end(arg);
}

#include "init_sysdeps.c"
#include "init_utils_string.c"
#include "init_utils_run.c"
#include "init_sysfs.c"

static void free_space(struct space *space)
{
	struct pair *pair = space->pairs;
	struct pair *next = pair;

	while (next) {
		pair = next;
		next = pair->next;
		free(pair->value);
		free(pair->key);
		free(pair);
	}
	space->pairs = NULL;
	if (space->ctl_value) {
		snd_ctl_elem_value_free(space->ctl_value);
		space->ctl_value = NULL;
	}
	if (space->ctl_info) {
		snd_ctl_elem_info_free(space->ctl_info);
		space->ctl_info = NULL;
	}
	if (space->ctl_id) {
		snd_ctl_elem_id_free(space->ctl_id);
		space->ctl_id = NULL;
	}
	if (space->ctl_card_info) {
		snd_ctl_card_info_free(space->ctl_card_info);
		space->ctl_card_info = NULL;
	}
	if (space->ctl_handle) {
		snd_hctl_close(space->ctl_handle);
		space->ctl_handle = NULL;
	}
	if (space->rootdir)
		free(space->rootdir);
	if (space->program_result)
		free(space->program_result);
	if (space->go_to)
		free(space->go_to);
	free(space);
}

static struct pair *value_find(struct space *space, const char *key)
{
	struct pair *pair = space->pairs;
	
	while (pair && strcmp(pair->key, key) != 0)
		pair = pair->next;
	return pair;
}

static int value_set(struct space *space, const char *key, const char *value)
{
	struct pair *pair;
	
	pair = value_find(space, key);
	if (pair) {
		free(pair->value);
		pair->value = strdup(value);
		if (pair->value == NULL)
			return -ENOMEM;
	} else {
		pair = malloc(sizeof(struct pair));
		if (pair == NULL)
			return -ENOMEM;
		pair->key = strdup(key);
		if (pair->key == NULL) {
			free(pair);
			return -ENOMEM;
		}
		pair->value = strdup(value);
		if (pair->value == NULL) {
			free(pair->key);
			free(pair);
			return -ENOMEM;
		}
		pair->next = space->pairs;
		space->pairs = pair;
	}
	return 0;
}

static int init_space(struct space **space, int card)
{
	struct space *res;
	char device[16];
	int err;

	res = calloc(1, sizeof(struct space));
	if (res == NULL)
		return -ENOMEM;
	res->ctl_id_changed = ~0;
	res->linenum = -1;
	sprintf(device, "hw:%u", card);
	err = snd_hctl_open(&res->ctl_handle, device, 0);
	if (err < 0)
		goto error;
	err = snd_hctl_load(res->ctl_handle);
	if (err < 0)
		goto error;
	err = snd_ctl_card_info_malloc(&res->ctl_card_info);
	if (err < 0)
		goto error;
	err = snd_ctl_card_info(snd_hctl_ctl(res->ctl_handle), res->ctl_card_info);
	if (err < 0)
		goto error;
	err = snd_ctl_elem_id_malloc(&res->ctl_id);
	if (err < 0)
		goto error;
	err = snd_ctl_elem_info_malloc(&res->ctl_info);
	if (err < 0)
		goto error;
	err = snd_ctl_elem_value_malloc(&res->ctl_value);
	if (err < 0)
		goto error;
	*space = res;
	return 0;
 error:
 	free_space(res);
 	return err;
}

static const char *cardinfo_get(struct space *space, const char *attr)
{
	if (strncasecmp(attr, "CARD", 4) == 0) {
		static char res[16];
		sprintf(res, "%u", snd_ctl_card_info_get_card(space->ctl_card_info));
		return res;
	}
	if (strncasecmp(attr, "ID", 2) == 0)
		return snd_ctl_card_info_get_id(space->ctl_card_info);
	if (strncasecmp(attr, "DRIVER", 6) == 0)
		return snd_ctl_card_info_get_driver(space->ctl_card_info);
	if (strncasecmp(attr, "NAME", 4) == 0)
		return snd_ctl_card_info_get_name(space->ctl_card_info);
	if (strncasecmp(attr, "LONGNAME", 8) == 0)
		return snd_ctl_card_info_get_longname(space->ctl_card_info);
	if (strncasecmp(attr, "MIXERNAME", 9) == 0)
		return snd_ctl_card_info_get_mixername(space->ctl_card_info);
	if (strncasecmp(attr, "COMPONENTS", 10) == 0)
		return snd_ctl_card_info_get_components(space->ctl_card_info);
	Perror(space, "unknown cardinfo{} attribute '%s'", attr);
	return NULL;
}

static int check_id_changed(struct space *space, unsigned int what)
{
	snd_hctl_elem_t *elem;
	int err;

	if ((space->ctl_id_changed & what & 1) != 0) {
		snd_ctl_elem_id_set_numid(space->ctl_id, 0);
		elem = snd_hctl_find_elem(space->ctl_handle, space->ctl_id);
		if (!elem)
			return -ENOENT;
		err = snd_hctl_elem_info(elem, space->ctl_info);
		if (err == 0)
			space->ctl_id_changed &= ~1;
		return err;
	}
	if ((space->ctl_id_changed & what & 2) != 0) {
		snd_ctl_elem_id_set_numid(space->ctl_id, 0);
		elem = snd_hctl_find_elem(space->ctl_handle, space->ctl_id);
		if (!elem)
			return -ENOENT;
		err = snd_hctl_elem_read(elem, space->ctl_value);
		if (err == 0)
			space->ctl_id_changed &= ~2;
		return err;
	}
	return 0;
}

static const char *get_ctl_value(struct space *space)
{
	snd_ctl_elem_type_t type;
	unsigned int idx, count;
	static char res[1024], tmp[16];
	static const char hex[] = "0123456789abcdef";
	char *pos;
	const char *pos1;

	type = snd_ctl_elem_info_get_type(space->ctl_info);
	count = snd_ctl_elem_info_get_count(space->ctl_info);
	res[0] = '\0';
	switch (type) {
	case SND_CTL_ELEM_TYPE_BOOLEAN:
		for (idx = 0; idx < count; idx++) {
			if (idx > 0)
				strlcat(res, ",", sizeof(res));
			strlcat(res, snd_ctl_elem_value_get_boolean(space->ctl_value, idx) ? "on" : "off", sizeof(res));
		}
		break;
	case SND_CTL_ELEM_TYPE_INTEGER:
		for (idx = 0; idx < count; idx++) {
			if (idx > 0)
				strlcat(res, ",", sizeof(res));
			snprintf(tmp, sizeof(tmp), "%li", snd_ctl_elem_value_get_integer(space->ctl_value, idx));
			strlcat(res, tmp, sizeof(res));
		}
		break;
	case SND_CTL_ELEM_TYPE_INTEGER64:
		for (idx = 0; idx < count; idx++) {
			if (idx > 0)
				strlcat(res, ",", sizeof(res));
			snprintf(tmp, sizeof(tmp), "%lli", snd_ctl_elem_value_get_integer64(space->ctl_value, idx));
			strlcat(res, tmp, sizeof(res));
		}
		break;
	case SND_CTL_ELEM_TYPE_ENUMERATED:
		for (idx = 0; idx < count; idx++) {
			if (idx > 0)
				strlcat(res, ",", sizeof(res));
			snprintf(tmp, sizeof(tmp), "%u", snd_ctl_elem_value_get_enumerated(space->ctl_value, idx));
			strlcat(res, tmp, sizeof(res));
		}
		break;
	case SND_CTL_ELEM_TYPE_BYTES:
	case SND_CTL_ELEM_TYPE_IEC958:
		if (type == SND_CTL_ELEM_TYPE_IEC958)
			count = sizeof(snd_aes_iec958_t);
		if (count > (sizeof(res)-1)/2)
			count = (sizeof(res)-1/2);
		pos = res;
		pos1 = snd_ctl_elem_value_get_bytes(space->ctl_value);
		while (count > 0) {
			idx = *pos1++;
			*pos++ = hex[idx >> 4];
			*pos++ = hex[idx & 0x0f];
			count++;
		}
		*pos++ = '\0';
		break;
	default:
		Perror(space, "unknown element type '%i'", type);
		return NULL;
	}
	return res;
}

/* Function to convert from percentage to volume. val = percentage */
#define convert_prange1(val, min, max) \
        ceil((val) * ((max) - (min)) * 0.01 + (min))

static int set_ctl_value(struct space *space, const char *value, int all)
{
	snd_ctl_elem_type_t type;
	unsigned int idx, idx2, count, items;
	const char *pos, *pos2;
	snd_hctl_elem_t *elem;
	int val;
	long lval;

	type = snd_ctl_elem_info_get_type(space->ctl_info);
	count = snd_ctl_elem_info_get_count(space->ctl_info);
	switch (type) {
	case SND_CTL_ELEM_TYPE_BOOLEAN:
		for (idx = 0; idx < count; idx++) {
			while (*value == ' ')
				value++;
			if (*value == '\0')
				goto missing;
			val = strncasecmp(value, "true", 4) == 0 ||
				strncasecmp(value, "yes", 3) == 0 ||
				strncasecmp(value, "on", 2) == 0 ||
				strncasecmp(value, "1", 1) == 0;
			snd_ctl_elem_value_set_boolean(space->ctl_value, idx, val);
			if (all)
				continue;
			pos = strchr(value, ',');
			value = pos ? pos + 1 : value + strlen(value) - 1;
		}
		break;
	case SND_CTL_ELEM_TYPE_INTEGER:
		for (idx = 0; idx < count; idx++) {
			while (*value == ' ')
				value++;
			pos = strchr(value, ',');
			if (pos)
				*(char *)pos = '\0';
			remove_trailing_chars((char *)value, ' ');
			items = pos ? pos - value : strlen(value);
			if (items > 1 && value[items-1] == '%') {
				val = convert_prange1(strtol(value, NULL, 0), snd_ctl_elem_info_get_min(space->ctl_info), snd_ctl_elem_info_get_max(space->ctl_info));
				snd_ctl_elem_value_set_integer(space->ctl_value, idx, val);
			} else if (items > 2 && value[items-2] == 'd' && value[items-1] == 'B') {
				val = strtol(value, NULL, 0) * 100;
				if ((pos2 = strchr(value, '.')) != NULL) {
					if (isdigit(*(pos2-1)) && isdigit(*(pos2-2))) {
						if (val < 0)
							val -= strtol(pos2 + 1, NULL, 0);
						else
							val += strtol(pos2 + 1, NULL, 0);
					} else if (isdigit(*(pos2-1))) {
						if (val < 0)
							val -= strtol(pos2 + 1, NULL, 0) * 10;
						else
							val += strtol(pos2 + 1, NULL, 0) * 10;
					}
				}
				val = snd_ctl_convert_from_dB(snd_hctl_ctl(space->ctl_handle), space->ctl_id, val, &lval, -1);
				if (val < 0) {
					dbg("unable to convert dB value '%s' to internal integer range", value);
					return val;
				}
				snd_ctl_elem_value_set_integer(space->ctl_value, idx, lval);
			} else {
				snd_ctl_elem_value_set_integer(space->ctl_value, idx, strtol(value, NULL, 0));
			}
			if (all)
				continue;
			value = pos ? pos + 1 : value + strlen(value) - 1;
		}
		break;
	case SND_CTL_ELEM_TYPE_INTEGER64:
		for (idx = 0; idx < count; idx++) {
			while (*value == ' ')
				value++;
			snd_ctl_elem_value_set_integer64(space->ctl_value, idx, strtoll(value, NULL, 0));
			if (all)
				continue;
			pos = strchr(value, ',');
			value = pos ? pos + 1 : value + strlen(value) - 1;
		}
		break;
	case SND_CTL_ELEM_TYPE_ENUMERATED:
		for (idx = 0; idx < count; idx++) {
			while (*value == ' ')
				value++;
			pos = strchr(value, ',');
			if (isdigit(value[0]) || value[0] == '-') {
				snd_ctl_elem_value_set_enumerated(space->ctl_value, idx, strtol(value, NULL, 0));
			} else {
				if (pos)
					*(char *)pos = '\0';
				remove_trailing_chars((char *)value, ' ');
				items = snd_ctl_elem_info_get_items(space->ctl_info);
				for (idx2 = 0; idx2 < items; idx2++) {
					snd_ctl_elem_info_set_item(space->ctl_info, idx2);
					elem = snd_hctl_find_elem(space->ctl_handle, space->ctl_id);
					if (elem == NULL)
						return -ENOENT;
					val = snd_hctl_elem_info(elem, space->ctl_info);
					if (val < 0)
						return val;
					if (strcasecmp(snd_ctl_elem_info_get_item_name(space->ctl_info), value) == 0) {
						snd_ctl_elem_value_set_enumerated(space->ctl_value, idx, idx2);
						break;
					}
				}
				if (idx2 >= items) {
					Perror(space, "wrong enum identifier '%s'", value);
					return -EINVAL;
				}
			}
			if (all)
				continue;
			value = pos ? pos + 1 : value + strlen(value) - 1;
		}
		break;
	case SND_CTL_ELEM_TYPE_BYTES:
	case SND_CTL_ELEM_TYPE_IEC958:
		if (type == SND_CTL_ELEM_TYPE_IEC958)
			count = sizeof(snd_aes_iec958_t);
		while (*value == ' ')
			value++;
		if (strlen(value) != count * 2) {
			Perror(space, "bad ctl value hexa length (should be %u bytes)", count);
			return -EINVAL;
		}
		for (idx = 0; idx < count; idx += 2) {
			val = hextodigit(*(value++)) << 4;
			val |= hextodigit(*(value++));
			if (val > 255) {
				Perror(space, "bad ctl hexa value");
				return -EINVAL;
			}
			snd_ctl_elem_value_set_byte(space->ctl_value, idx, val);
		}
		break;
	default:
		Perror(space, "unknown element type '%i'", type);
		return -EINVAL;
	}
	return 0;
  missing:
  	printf("%i %i\n", type, count);
  	Perror(space, "missing some ctl values (line %i)", space->linenum);
  	return -EINVAL;
}

static int do_match(const char *key, enum key_op op,
		    const char *key_value, const char *value)
{
	int match;

	if (value == NULL)
		return 0;
	dbg("match %s '%s' <-> '%s'", key, key_value, value);
	match = fnmatch(key_value, value, 0) == 0;
	if (match && op == KEY_OP_MATCH) {
		dbg("%s is true (matching value)", key);
		return 1;
	}
	if (!match && op == KEY_OP_NOMATCH) {
		dbg("%s is true (non-matching value)", key);
		return 1;
	}
	dbg("%s is false", key);
	return 0;
}

static int ctl_match(snd_ctl_elem_id_t *pattern, snd_ctl_elem_id_t *id)
{
	if (snd_ctl_elem_id_get_interface(pattern) != -1 &&
	    snd_ctl_elem_id_get_interface(pattern) != snd_ctl_elem_id_get_interface(id))
	    	return 0;
	if (snd_ctl_elem_id_get_device(pattern) != -1 &&
	    snd_ctl_elem_id_get_device(pattern) != snd_ctl_elem_id_get_device(id))
		return 0;
	if (snd_ctl_elem_id_get_subdevice(pattern) != -1 &&
	    snd_ctl_elem_id_get_subdevice(pattern) != snd_ctl_elem_id_get_subdevice(id))
	    	return 0;
	if (snd_ctl_elem_id_get_index(pattern) != -1 &&
	    snd_ctl_elem_id_get_index(pattern) != snd_ctl_elem_id_get_index(id))
	    	return 0;
	if (fnmatch(snd_ctl_elem_id_get_name(pattern), snd_ctl_elem_id_get_name(id), 0) != 0)
		return 0;
	return 1;
}

static const char *elemid_get(struct space *space, const char *attr)
{
	long long val;
	snd_ctl_elem_type_t type;
	static char res[256];

	if (strncasecmp(attr, "numid", 5) == 0) {
		val = snd_ctl_elem_id_get_numid(space->ctl_id);
	    	goto value;
	}
	if (strncasecmp(attr, "iface", 5) == 0 ||
	    strncasecmp(attr, "interface", 9) == 0)
	    	return snd_ctl_elem_iface_name(snd_ctl_elem_id_get_interface(space->ctl_id));
	if (strncasecmp(attr, "device", 6) == 0) {
		val = snd_ctl_elem_id_get_device(space->ctl_id);
	    	goto value;
	}
	if (strncasecmp(attr, "subdev", 6) == 0) {
		val = snd_ctl_elem_id_get_subdevice(space->ctl_id);
	    	goto value;
	}
	if (strncasecmp(attr, "name", 4) == 0)
		return snd_ctl_elem_id_get_name(space->ctl_id);
	if (strncasecmp(attr, "index", 5) == 0) {
		val = snd_ctl_elem_id_get_index(space->ctl_id);
	    	goto value;
	}
	if (strncasecmp(attr, "type", 4) == 0) {
		if (check_id_changed(space, 1))
			return NULL;
		return snd_ctl_elem_type_name(snd_ctl_elem_info_get_type(space->ctl_info));
	}
	if (strncasecmp(attr, "attr", 4) == 0) {
		if (check_id_changed(space, 1))
			return NULL;
		res[0] = '\0';
		if (snd_ctl_elem_info_is_readable(space->ctl_info))
			strcat(res, "r");
		if (snd_ctl_elem_info_is_writable(space->ctl_info))
			strcat(res, "w");
		if (snd_ctl_elem_info_is_volatile(space->ctl_info))
			strcat(res, "v");
		if (snd_ctl_elem_info_is_inactive(space->ctl_info))
			strcat(res, "i");
		if (snd_ctl_elem_info_is_locked(space->ctl_info))
			strcat(res, "l");
		if (snd_ctl_elem_info_is_tlv_readable(space->ctl_info))
			strcat(res, "R");
		if (snd_ctl_elem_info_is_tlv_writable(space->ctl_info))
			strcat(res, "W");
		if (snd_ctl_elem_info_is_tlv_commandable(space->ctl_info))
			strcat(res, "C");
		if (snd_ctl_elem_info_is_owner(space->ctl_info))
			strcat(res, "o");
		if (snd_ctl_elem_info_is_user(space->ctl_info))
			strcat(res, "u");
		return res;
	}
	if (strncasecmp(attr, "owner", 5) == 0) {
		if (check_id_changed(space, 1))
			return NULL;
		val = snd_ctl_elem_info_get_owner(space->ctl_info);
		goto value;
	}
	if (strncasecmp(attr, "count", 5) == 0) {
		if (check_id_changed(space, 1))
			return NULL;
		val = snd_ctl_elem_info_get_count(space->ctl_info);
		goto value;
	}
	if (strncasecmp(attr, "min", 3) == 0) {
		if (check_id_changed(space, 1))
			return NULL;
		type = snd_ctl_elem_info_get_type(space->ctl_info);
		if (type == SND_CTL_ELEM_TYPE_INTEGER64)
			val = snd_ctl_elem_info_get_min64(space->ctl_info);
		else if (type == SND_CTL_ELEM_TYPE_INTEGER)
			val = snd_ctl_elem_info_get_min(space->ctl_info);
		else
			goto empty;
		goto value;
	}
	if (strncasecmp(attr, "max", 3) == 0) {
		if (check_id_changed(space, 1))
			return NULL;
		type = snd_ctl_elem_info_get_type(space->ctl_info);
		if (type == SND_CTL_ELEM_TYPE_INTEGER64)
			val = snd_ctl_elem_info_get_max64(space->ctl_info);
		else if (type == SND_CTL_ELEM_TYPE_INTEGER)
			val = snd_ctl_elem_info_get_max(space->ctl_info);
		else
			goto empty;
		goto value;
	}
	if (strncasecmp(attr, "step", 3) == 0) {
		if (check_id_changed(space, 1))
			return NULL;
		type = snd_ctl_elem_info_get_type(space->ctl_info);
		if (type == SND_CTL_ELEM_TYPE_INTEGER64)
			val = snd_ctl_elem_info_get_step64(space->ctl_info);
		else if (type == SND_CTL_ELEM_TYPE_INTEGER)
			val = snd_ctl_elem_info_get_step(space->ctl_info);
		else
			goto empty;
		goto value;
	}
	if (strncasecmp(attr, "items", 5) == 0) {
		if (check_id_changed(space, 1))
			return NULL;
		if (snd_ctl_elem_info_get_type(space->ctl_info) == SND_CTL_ELEM_TYPE_ENUMERATED)
			val = snd_ctl_elem_info_get_items(space->ctl_info);
		else {
		  empty:
			res[0] = '\0';
			return res;
		}
		goto value;
	}
	if (strncasecmp(attr, "value", 5) == 0) {
		if (check_id_changed(space, 3))
			return NULL;
		return get_ctl_value(space);
	}
	if (strncasecmp(attr, "dBmin", 5) == 0) {
		long min, max;
		if (check_id_changed(space, 1))
			return NULL;
		if (snd_ctl_get_dB_range(snd_hctl_ctl(space->ctl_handle), space->ctl_id, &min, &max) < 0)
			goto empty;
		val = min;
dbvalue:
		sprintf(res, "%li.%02idB", (long)(val / 100), (int)abs(val % 100));
		return res;
	}
	if (strncasecmp(attr, "dBmax", 5) == 0) {
		long min, max;
		if (check_id_changed(space, 1))
			return NULL;
		if (snd_ctl_get_dB_range(snd_hctl_ctl(space->ctl_handle), space->ctl_id, &min, &max) < 0)
			goto empty;
		val = max;
		goto dbvalue;
	}
	if (strncasecmp(attr, "enums", 5) == 0) {
		unsigned int idx, items;
		snd_hctl_elem_t *elem;
		if (check_id_changed(space, 1))
			return NULL;
		if (snd_ctl_elem_info_get_type(space->ctl_info) != SND_CTL_ELEM_TYPE_ENUMERATED)
			goto empty;
		items = snd_ctl_elem_info_get_items(space->ctl_info);
		strcpy(res, "|");
		for (idx = 0; idx < items; idx++) {
			snd_ctl_elem_info_set_item(space->ctl_info, idx);
			elem = snd_hctl_find_elem(space->ctl_handle, space->ctl_id);
			if (elem == NULL)
				break;
			if (snd_hctl_elem_info(elem, space->ctl_info) < 0)
				break;
			strlcat(res, snd_ctl_elem_info_get_item_name(space->ctl_info), sizeof(res));
			strlcat(res, "|", sizeof(res));
		}
		return res;
	}
	if (strncasecmp(attr, "do_search", 9) == 0) {
		int err, index = 0;
		snd_hctl_elem_t *elem;
		snd_ctl_elem_id_t *id;
		char *pos = strchr(attr, ' ');
		if (pos)
			index = strtol(pos, NULL, 0);
		err = snd_ctl_elem_id_malloc(&id);
		if (err < 0)
			return NULL;
		elem = snd_hctl_first_elem(space->ctl_handle);
		while (elem) {
			snd_hctl_elem_get_id(elem, id);
			if (!ctl_match(space->ctl_id, id))
				goto next_search;
			if (index > 0) {
				index--;
				goto next_search;
			}
			strcpy(res, "1");
			snd_ctl_elem_id_copy(space->ctl_id, id);
			snd_ctl_elem_id_free(id);
			dbg("do_ctl_search found a control");
			return res;
		      next_search:
			elem = snd_hctl_elem_next(elem);
		}
		snd_ctl_elem_id_free(id);
		strcpy(res, "0");
		return res;
	}
	if (strncasecmp(attr, "do_count", 8) == 0) {
		int err, index = 0;
		snd_hctl_elem_t *elem;
		snd_ctl_elem_id_t *id;
		err = snd_ctl_elem_id_malloc(&id);
		if (err < 0)
			return NULL;
		elem = snd_hctl_first_elem(space->ctl_handle);
		while (elem) {
			snd_hctl_elem_get_id(elem, id);
			if (ctl_match(space->ctl_id, id))
				index++;
			elem = snd_hctl_elem_next(elem);
		}
		snd_ctl_elem_id_free(id);
		sprintf(res, "%u", index);
		dbg("do_ctl_count found %s controls", res);
		return res;
	}
	Perror(space, "unknown ctl{} attribute '%s'", attr);
	return NULL;
  value:
  	sprintf(res, "%lli", val);
  	return res;
}

static int elemid_set(struct space *space, const char *attr, const char *value)
{
	unsigned int val;
	void (*fcn)(snd_ctl_elem_id_t *, unsigned int);
	snd_ctl_elem_iface_t iface;
	int err;

	if (strncasecmp(attr, "numid", 5) == 0) {
		fcn = snd_ctl_elem_id_set_numid;
	    	goto value;
	}
	if (strncasecmp(attr, "iface", 5) == 0 ||
	    strncasecmp(attr, "interface", 9) == 0 ||
	    strncasecmp(attr, "reset", 5) == 0 ||
	    strncasecmp(attr, "search", 6) == 0) {
	    	if (strlen(value) == 0 && strncasecmp(attr, "search", 6) == 0) {
	    		iface = 0;
	    		goto search;
		}
	    	for (iface = 0; iface <= SND_CTL_ELEM_IFACE_LAST; iface++) {
	    		if (strcasecmp(value, snd_ctl_elem_iface_name(iface)) == 0) {
			    	if (strncasecmp(attr, "reset", 5) == 0)
			    		snd_ctl_elem_id_clear(space->ctl_id);
			    	if (strncasecmp(attr, "search", 5) == 0) {
			    	  search:
			    		snd_ctl_elem_id_clear(space->ctl_id);
			    		/* -1 means all */
			    		snd_ctl_elem_id_set_interface(space->ctl_id, -1);
			    		snd_ctl_elem_id_set_device(space->ctl_id, -1);
			    		snd_ctl_elem_id_set_subdevice(space->ctl_id, -1);
			    		snd_ctl_elem_id_set_name(space->ctl_id, "*");
			    		snd_ctl_elem_id_set_index(space->ctl_id, -1);
			    		if (strlen(value) == 0)
			    			return 0;
				}
				snd_ctl_elem_id_set_interface(space->ctl_id, iface);
				space->ctl_id_changed = ~0;
			    	return 0;
			}
		}
		Perror(space, "unknown control interface name '%s'", value);
		return -EINVAL;
	}
	if (strncasecmp(attr, "device", 6) == 0) {
		fcn = snd_ctl_elem_id_set_device;
	    	goto value;
	}
	if (strncasecmp(attr, "subdev", 6) == 0) {
		fcn = snd_ctl_elem_id_set_subdevice;
	    	goto value;
	}
	if (strncasecmp(attr, "name", 4) == 0) {
		snd_ctl_elem_id_set_name(space->ctl_id, value);
	  	space->ctl_id_changed = ~0;
		return 0;
	}
	if (strncasecmp(attr, "index", 5) == 0) {
		fcn = snd_ctl_elem_id_set_index;
	    	goto value;
	}
	if (strncasecmp(attr, "values", 6) == 0 ||
	    strncasecmp(attr, "value", 5) == 0) {
		err = check_id_changed(space, 1);
		if (err < 0) {
			Perror(space, "control element not found");
			return err;
		}
		err = set_ctl_value(space, value, strncasecmp(attr, "values", 6) == 0);
		if (err < 0) {
			space->ctl_id_changed |= 2;
		} else {
			space->ctl_id_changed &= ~2;
			snd_ctl_elem_value_set_id(space->ctl_value, space->ctl_id);
			err = snd_ctl_elem_write(snd_hctl_ctl(space->ctl_handle), space->ctl_value);
			if (err < 0) {
				Perror(space, "value write error: %s", snd_strerror(err));
				return err;
			}
		}
	    	return err;
	}
	Perror(space, "unknown CTL{} attribute '%s'", attr);
	return -EINVAL;
  value:
  	val = (unsigned int)strtol(value, NULL, 0);
  	fcn(space->ctl_id, val);
  	space->ctl_id_changed = ~0;
  	return 0;
}

static int get_key(char **line, char **key, enum key_op *op, char **value)
{
	char *linepos;
	char *temp;

	linepos = *line;
	if (linepos == NULL && linepos[0] == '\0')
		return -EINVAL;

	/* skip whitespace */
	while (isspace(linepos[0]) || linepos[0] == ',')
		linepos++;

	/* get the key */
	if (linepos[0] == '\0')
		return -EINVAL;
	*key = linepos;

	while (1) {
		linepos++;
		if (linepos[0] == '\0')
			return -1;
		if (isspace(linepos[0]))
			break;
		if (linepos[0] == '=')
			break;
		if (linepos[0] == '+')
			break;
		if (linepos[0] == '!')
			break;
		if (linepos[0] == ':')
			break;
	}

	/* remember end of key */
	temp = linepos;

	/* skip whitespace after key */
	while (isspace(linepos[0]))
		linepos++;
	if (linepos[0] == '\0')
		return -EINVAL;

	/* get operation type */
	if (linepos[0] == '=' && linepos[1] == '=') {
		*op = KEY_OP_MATCH;
		linepos += 2;
		dbg("operator=match");
	} else if (linepos[0] == '!' && linepos[1] == '=') {
		*op = KEY_OP_NOMATCH;
		linepos += 2;
		dbg("operator=nomatch");
	} else if (linepos[0] == '+' && linepos[1] == '=') {
		*op = KEY_OP_ADD;
		linepos += 2;
		dbg("operator=add");
	} else if (linepos[0] == '=') {
		*op = KEY_OP_ASSIGN;
		linepos++;
		dbg("operator=assign");
	} else if (linepos[0] == ':' && linepos[1] == '=') {
		*op = KEY_OP_ASSIGN_FINAL;
		linepos += 2;
		dbg("operator=assign_final");
	} else
		return -EINVAL;

	/* terminate key */
	temp[0] = '\0';
	dbg("key='%s'", *key);

	/* skip whitespace after operator */
	while (isspace(linepos[0]))
		linepos++;
	if (linepos[0] == '\0')
		return -EINVAL;

	/* get the value*/
	if (linepos[0] != '"')
		return -EINVAL;
	linepos++;
	*value = linepos;

	while (1) {
		temp = strchr(linepos, '"');
		if (temp && temp[-1] == '\\') {
			linepos = temp + 1;
			continue;
		}
		break;
	}
	if (!temp)
		return -EINVAL;
	temp[0] = '\0';
	temp++;
	dbg("value='%s'", *value);

	/* move line to next key */
	*line = temp;

	return 0;
}

/* extract possible KEY{attr} */
static char *get_key_attribute(struct space *space, char *str, char *res, size_t ressize)
{
	char *pos;
	char *attr;

	attr = strchr(str, '{');
	if (attr != NULL) {
		attr++;
		pos = strchr(attr, '}');
		if (pos == NULL) {
			Perror(space, "missing closing brace for format");
			return NULL;
		}
		pos[0] = '\0';
		strlcpy(res, attr, ressize);
		pos[0] = '}';
		dbg("attribute='%s'", res);
		return res;
	}

	return NULL;
}

/* extract possible {attr} and move str behind it */
static char *get_format_attribute(struct space *space, char **str)
{
	char *pos;
	char *attr = NULL;

	if (*str[0] == '{') {
		pos = strchr(*str, '}');
		if (pos == NULL) {
			Perror(space, "missing closing brace for format");
			return NULL;
		}
		pos[0] = '\0';
		attr = *str+1;
		*str = pos+1;
		dbg("attribute='%s', str='%s'", attr, *str);
	}
	return attr;
}

/* extract possible format length and move str behind it*/
static int get_format_len(struct space *space, char **str)
{
	int num;
	char *tail;

	if (isdigit(*str[0])) {
		num = (int) strtoul(*str, &tail, 10);
		if (num > 0) {
			*str = tail;
			dbg("format length=%i", num);
			return num;
		} else {
			Perror(space, "format parsing error '%s'", *str);
		}
	}
	return -1;
}

static void apply_format(struct space *space, char *string, size_t maxsize)
{
	char temp[PATH_SIZE];
	char temp2[PATH_SIZE];
	char *head, *tail, *pos, *cpos, *attr, *rest;
	struct pair *pair;
	int len;
	int i;
	int count;
	enum subst_type {
		SUBST_UNKNOWN,
		SUBST_CARDINFO,
		SUBST_CTL,
		SUBST_RESULT,
		SUBST_ATTR,
		SUBST_SYSFSROOT,
		SUBST_ENV,
		SUBST_CONFIG,
	};
	static const struct subst_map {
		char *name;
		char fmt;
		enum subst_type type;
	} map[] = {
		{ .name = "cardinfo",	.fmt = 'i',	.type = SUBST_CARDINFO },
		{ .name = "ctl",	.fmt = 'C',	.type = SUBST_CTL },
		{ .name = "result",	.fmt = 'c',	.type = SUBST_RESULT },
		{ .name = "attr",	.fmt = 's',	.type = SUBST_ATTR },
		{ .name = "sysfsroot",	.fmt = 'r',	.type = SUBST_SYSFSROOT },
		{ .name = "env",	.fmt = 'E',	.type = SUBST_ENV },
		{ .name = "config",	.fmt = 'g',	.type = SUBST_CONFIG },
		{ NULL, '\0', 0 }
	};
	enum subst_type type;
	const struct subst_map *subst;

	head = string;
	while (1) {
		len = -1;
		while (head[0] != '\0') {
			if (head[0] == '$') {
				/* substitute named variable */
				if (head[1] == '\0')
					break;
				if (head[1] == '$') {
					strlcpy(temp, head+2, sizeof(temp));
					strlcpy(head+1, temp, maxsize);
					head++;
					continue;
				}
				head[0] = '\0';
				for (subst = map; subst->name; subst++) {
					if (strncasecmp(&head[1], subst->name, strlen(subst->name)) == 0) {
						type = subst->type;
						tail = head + strlen(subst->name)+1;
						dbg("will substitute format name '%s'", subst->name);
						goto found;
					}
				}
			} else if (head[0] == '%') {
				/* substitute format char */
				if (head[1] == '\0')
					break;
				if (head[1] == '%') {
					strlcpy(temp, head+2, sizeof(temp));
					strlcpy(head+1, temp, maxsize);
					head++;
					continue;
				}
				head[0] = '\0';
				tail = head+1;
				len = get_format_len(space, &tail);
				for (subst = map; subst->name; subst++) {
					if (tail[0] == subst->fmt) {
						type = subst->type;
						tail++;
						dbg("will substitute format char '%c'", subst->fmt);
						goto found;
					}
				}
			}
			head++;
		}
		break;
found:
		attr = get_format_attribute(space, &tail);
		strlcpy(temp, tail, sizeof(temp));
		dbg("format=%i, string='%s', tail='%s'", type ,string, tail);

		switch (type) {
		case SUBST_CARDINFO:
			if (attr == NULL)
				Perror(space, "missing identification parametr for cardinfo");
			else {
				const char *value = cardinfo_get(space, attr);
				if (value == NULL)
					break;
				strlcat(string, value, maxsize);
				dbg("substitute cardinfo{%s} '%s'", attr, value);
			}
			break;
		case SUBST_CTL:
			if (attr == NULL)
				Perror(space, "missing identification parametr for ctl");
			else {
				const char *value = elemid_get(space, attr);
				if (value == NULL)
					break;
				strlcat(string, value, maxsize);
				dbg("substitute ctl{%s} '%s'", attr, value);
			}
			break;
		case SUBST_RESULT:
			if (space->program_result == NULL)
				break;
			/* get part part of the result string */
			i = 0;
			if (attr != NULL)
				i = strtoul(attr, &rest, 10);
			if (i > 0) {
				dbg("request part #%d of result string", i);
				cpos = space->program_result;
				while (--i) {
					while (cpos[0] != '\0' && !isspace(cpos[0]))
						cpos++;
					while (isspace(cpos[0]))
						cpos++;
				}
				if (i > 0) {
					Perror(space, "requested part of result string not found");
					break;
				}
				strlcpy(temp2, cpos, sizeof(temp2));
				/* %{2+}c copies the whole string from the second part on */
				if (rest[0] != '+') {
					cpos = strchr(temp2, ' ');
					if (cpos)
						cpos[0] = '\0';
				}
				strlcat(string, temp2, maxsize);
				dbg("substitute part of result string '%s'", temp2);
			} else {
				strlcat(string, space->program_result, maxsize);
				dbg("substitute result string '%s'", space->program_result);
			}
			break;
		case SUBST_ATTR:
			if (attr == NULL)
				Perror(space, "missing file parameter for attr");
			else {
				const char *value = NULL;
				size_t size;

				pair = value_find(space, "sysfs_device");
				if (pair == NULL)
					break;
				value = sysfs_attr_get_value(pair->value, attr);

				if (value == NULL)
					break;

				/* strip trailing whitespace and replace untrusted characters of sysfs value */
				size = strlcpy(temp2, value, sizeof(temp2));
				if (size >= sizeof(temp2))
					size = sizeof(temp2)-1;
				while (size > 0 && isspace(temp2[size-1]))
					temp2[--size] = '\0';
				count = replace_untrusted_chars(temp2);
				if (count > 0)
					Perror(space, "%i untrusted character(s) replaced" , count);
				strlcat(string, temp2, maxsize);
				dbg("substitute sysfs value '%s'", temp2);
			}
			break;
		case SUBST_SYSFSROOT:
			strlcat(string, sysfs_path, maxsize);
			dbg("substitute sysfs_path '%s'", sysfs_path);
			break;
		case SUBST_ENV:
			if (attr == NULL) {
				dbg("missing attribute");
				break;
			}
			pos = getenv(attr);
			if (pos == NULL) {
				dbg("env '%s' not available", attr);
				break;
			}
			dbg("substitute env '%s=%s'", attr, pos);
			strlcat(string, pos, maxsize);
			break;
		case SUBST_CONFIG:
			if (attr == NULL) {
				dbg("missing attribute");
				break;
			}
			pair = value_find(space, attr);
			if (pair == NULL)
				break;
			strlcat(string, pair->value, maxsize);
			break;
		default:
			Perror(space, "unknown substitution type=%i", type);
			break;
		}
		/* possibly truncate to format-char specified length */
		if (len != -1) {
			head[len] = '\0';
			dbg("truncate to %i chars, subtitution string becomes '%s'", len, head);
		}
		strlcat(string, temp, maxsize);
	}
	/* unescape strings */
	head = tail = string;
	while (*head != '\0') {
		if (*head == '\\') {
			head++;
			if (*head == '\0')
				break;
			switch (*head) {
			case 'a': *tail++ = '\a'; break;
			case 'b': *tail++ = '\b'; break;
			case 'n': *tail++ = '\n'; break;
			case 'r': *tail++ = '\r'; break;
			case 't': *tail++ = '\t'; break;
			case 'v': *tail++ = '\v'; break;
			case '\\': *tail++ = '\\'; break;
			default: *tail++ = *head; break;
			}
			head++;
			continue;
		}
		if (*head)
			*tail++ = *head++;
	}
	*tail = 0;
}

static
int run_program1(struct space *space,
		 const char *command0, char *result,
		 size_t ressize, size_t *reslen, int log)
{
	if (strncmp(command0, "__ctl_search", 12) == 0) {
		const char *res = elemid_get(space, "do_search");
		if (res == NULL || strcmp(res, "1") != 0)
			return EXIT_FAILURE;
		return EXIT_SUCCESS;
	}
	if (strncmp(command0, "__ctl_count", 11) == 0) {
		const char *res = elemid_get(space, "do_count");
		if (res == NULL || strcmp(res, "0") == 0)
			return EXIT_FAILURE;
		strlcpy(result, res, ressize);
		return EXIT_SUCCESS;
	}
	Perror(space, "unknown buildin command '%s'", command0);
	return EXIT_FAILURE;
}

static int parse(struct space *space, const char *filename);

static char *new_root_dir(const char *filename)
{
	char *res, *tmp;

	res = strdup(filename);
	if (res) {
		tmp = strrchr(res, '/');
		if (tmp)
			*tmp = '\0';
	}
	dbg("new_root_dir '%s' '%s'", filename, res);
	return res;
}

static int parse_line(struct space *space, char *line, size_t linesize)
{
	char *linepos;
	char *key, *value, *attr, *temp;
	struct pair *pair;
	enum key_op op;
	int err = 0, count;
	char string[PATH_SIZE];
	char result[PATH_SIZE];

	linepos = line;
	while (*linepos != '\0') {
		op = KEY_OP_UNSET;
		
		err = get_key(&linepos, &key, &op, &value);
		if (err < 0)
			goto invalid;

		if (strncasecmp(key, "LABEL", 5) == 0) {
			if (op != KEY_OP_ASSIGN) {
				Perror(space, "invalid LABEL operation");
				goto invalid;
			}
			if (space->go_to && strcmp(space->go_to, value) == 0) {
				free(space->go_to);
				space->go_to = NULL;
			}
			continue;
		}
		
		if (space->go_to) {
			dbg("skip (GOTO '%s')", space->go_to);
			break;		/* not for us */
		}

		if (strncasecmp(key, "CTL{", 4) == 0) {
			attr = get_key_attribute(space, key + 3, string, sizeof(string));
			if (attr == NULL) {
				Perror(space, "error parsing CTL attribute");
				goto invalid;
			}
			if (op == KEY_OP_ASSIGN) {
				strlcpy(result, value, sizeof(result));
				apply_format(space, result, sizeof(result));
				dbg("ctl assign: '%s' '%s'", value, attr);
				err = elemid_set(space, attr, result);
				if (space->program_result) {
					free(space->program_result);
					space->program_result = NULL;
				}
				snprintf(string, sizeof(string), "%i", err);
				space->program_result = strdup(string);
				err = 0;
				if (space->program_result == NULL)
					break;
			} else if (op == KEY_OP_MATCH || op == KEY_OP_NOMATCH) {
				if (strncmp(attr, "write", 5) == 0) {
					strlcpy(result, value, sizeof(result));
					apply_format(space, result, sizeof(result));
					dbg("ctl write: '%s' '%s'", value, attr);
					err = elemid_set(space, "values", result);
					if (err == 0 && op == KEY_OP_NOMATCH)
						break;
					if (err != 0 && op == KEY_OP_MATCH)
						break;
				} else {
					temp = (char *)elemid_get(space, attr);
					dbg("ctl match: '%s' '%s' '%s'", attr, value, temp);
					if (!do_match(key, op, value, temp))
						break;
				}
			} else {
				Perror(space, "invalid CTL{} operation");
				goto invalid;
			}
			continue;
		}
		if (strcasecmp(key, "RESULT") == 0) {
			if (op == KEY_OP_MATCH || op == KEY_OP_NOMATCH) {
				if (!do_match(key, op, value, space->program_result))
					break;
			} else if (op == KEY_OP_ASSIGN) {
				if (space->program_result) {
					free(space->program_result);
					space->program_result = NULL;
				}
				strlcpy(string, value, sizeof(string));
				apply_format(space, string, sizeof(string));
				space->program_result = strdup(string);
				if (space->program_result == NULL)
					break;
 			} else {
				Perror(space, "invalid RESULT operation");
				goto invalid;
			}
			continue;
		}
		if (strcasecmp(key, "PROGRAM") == 0) {
			if (op == KEY_OP_UNSET)
				continue;
			strlcpy(string, value, sizeof(string));
			apply_format(space, string, sizeof(string));
			if (space->program_result) {
				free(space->program_result);
				space->program_result = NULL;
			}
			if (run_program(space, string, result, sizeof(result), NULL, space->log_run) != 0) {
				dbg("PROGRAM '%s' is false", string);
				if (op != KEY_OP_NOMATCH)
					break;
			} else {
				remove_trailing_chars(result, '\n');
				count = replace_untrusted_chars(result);
				if (count)
					info("%i untrusted character(s) replaced", count);
				dbg("PROGRAM '%s' result is '%s'", string, result);
				space->program_result = strdup(result);
				if (space->program_result == NULL)
					break;
				dbg("PROGRAM returned successful");
				if (op == KEY_OP_NOMATCH)
					break;
			}
			dbg("PROGRAM key is true");
			continue;
		}
		if (strncasecmp(key, "CARDINFO{", 9) == 0) {
			attr = get_key_attribute(space, key + 8, string, sizeof(string));
			if (attr == NULL) {
				Perror(space, "error parsing CARDINFO attribute");
				goto invalid;
			}
			if (op == KEY_OP_MATCH || op == KEY_OP_NOMATCH) {
				dbg("cardinfo: '%s' '%s'", value, attr);
				temp = (char *)cardinfo_get(space, attr);
				if (!do_match(key, op, value, temp))
					break;
			} else {
				Perror(space, "invalid CARDINFO{} operation");
				goto invalid;
			}
			continue;
		}
		if (strncasecmp(key, "ATTR{", 5) == 0) {
			attr = get_key_attribute(space, key + 4, string, sizeof(string));
			if (attr == NULL) {
				Perror(space, "error parsing ATTR attribute");
				goto invalid;
			}
			if (op == KEY_OP_MATCH || op == KEY_OP_NOMATCH) {
				pair = value_find(space, "sysfs_device");
				if (pair == NULL)
					break;
				dbg("sysfs_attr: '%s' '%s'", pair->value, attr);
				temp = sysfs_attr_get_value(pair->value, attr);
				if (!do_match(key, op, value, temp))
					break;
			} else {
				Perror(space, "invalid ATTR{} operation");
				goto invalid;
			}
			continue;
		}
		if (strncasecmp(key, "ENV{", 4) == 0) {
			attr = get_key_attribute(space, key + 3, string, sizeof(string));
			if (attr == NULL) {
				Perror(space, "error parsing ENV attribute");
				goto invalid;
			}
			if (op == KEY_OP_MATCH || op == KEY_OP_NOMATCH) {
				temp = getenv(attr);
				dbg("env: '%s' '%s'", attr, temp);
				if (!do_match(key, op, value, temp))
					break;
			} else if (op == KEY_OP_ASSIGN ||
				   op == KEY_OP_ASSIGN_FINAL) {
				strlcpy(result, value, sizeof(result));
				apply_format(space, result, sizeof(result));
				dbg("env set: '%s' '%s'", attr, result);
				if (setenv(attr, result, op == KEY_OP_ASSIGN_FINAL))
					break;
			} else {
				Perror(space, "invalid ENV{} operation");
				goto invalid;
			}
			continue;
		}
		if (strcasecmp(key, "GOTO") == 0) {
			if (op != KEY_OP_ASSIGN) {
				Perror(space, "invalid GOTO operation");
				goto invalid;
			}
			space->go_to = strdup(value);
			if (space->go_to == NULL) {
				err = -ENOMEM;
				break;
			}
			continue;
		}
		if (strcasecmp(key, "INCLUDE") == 0) {
			char *rootdir, *go_to;
			const char *filename;
			struct dirent *dirent;
			DIR *dir;
			int linenum;
			if (op != KEY_OP_ASSIGN) {
				Perror(space, "invalid INCLUDE operation");
				goto invalid;
			}
			if (value[0] == '/')
				strlcpy(string, value, sizeof(string));
			else {
				strlcpy(string, space->rootdir, sizeof(string));
				strlcat(string, "/", sizeof(string));
				strlcat(string, value, sizeof(string));
			}
			rootdir = space->rootdir;
			go_to = space->go_to;
			filename = space->filename;
			linenum = space->linenum;
			dir = opendir(string);
			if (dir) {
				count = strlen(string);
				while ((dirent = readdir(dir)) != NULL) {
					if (strcmp(dirent->d_name, ".") == 0 ||
					    strcmp(dirent->d_name, "..") == 0)
						continue;
					string[count] = '\0';
					strlcat(string, "/", sizeof(string));
					strlcat(string, dirent->d_name, sizeof(string));
					space->go_to = NULL;
					space->rootdir = new_root_dir(string);
					if (space->rootdir) {
						err = parse(space, string);
						free(space->rootdir);
					} else
						err = -ENOMEM;
					if (space->go_to) {
						Perror(space, "unterminated GOTO '%s'", space->go_to);
						free(space->go_to);
					}
					if (err)
						break;
				}
				closedir(dir);
			} else {
				space->go_to = NULL;
				space->rootdir = new_root_dir(string);
				if (space->rootdir) {
					err = parse(space, string);
					free(space->rootdir);
				} else
					err = -ENOMEM;
				if (space->go_to) {
					Perror(space, "unterminated GOTO '%s'", space->go_to);
					free(space->go_to);
				}
			}
			space->go_to = go_to;
			space->rootdir = rootdir;
			space->filename = filename;
			space->linenum = linenum;
			if (space->quit)
				break;
			if (err)
				break;
			continue;
		}
		if (strncasecmp(key, "ACCESS", 6) == 0) {
			if (op == KEY_OP_MATCH || op == KEY_OP_NOMATCH) {
				if (value[0] == '$') {
					strlcpy(string, value, sizeof(string));
					apply_format(space, string, sizeof(string));
					if (string[0] == '/')
						goto __access1;
				}
				if (value[0] != '/') {
					strlcpy(string, space->rootdir, sizeof(string));
					strlcat(string, "/", sizeof(string));
					strlcat(string, value, sizeof(string));
				} else {
					strlcpy(string, value, sizeof(string));
				}
				apply_format(space, string, sizeof(string));
			      __access1:
				count = access(string, F_OK);
				dbg("access(%s) = %i (%s)", string, count, value);
				if (op == KEY_OP_MATCH && count != 0)
					break;
				if (op == KEY_OP_NOMATCH && count == 0)
					break;
			} else {
				Perror(space, "invalid ACCESS operation");
				goto invalid;
			}
			continue;
		}
		if (strncasecmp(key, "PRINT", 5) == 0) {
			if (op != KEY_OP_ASSIGN) {
				Perror(space, "invalid PRINT operation");
				goto invalid;
			}
			strlcpy(string, value, sizeof(string));
			apply_format(space, string, sizeof(string));
			fwrite(string, strlen(string), 1, stdout);
			continue;
		}
		if (strncasecmp(key, "ERROR", 5) == 0) {
			if (op != KEY_OP_ASSIGN) {
				Perror(space, "invalid ERROR operation");
				goto invalid;
			}
			strlcpy(string, value, sizeof(string));
			apply_format(space, string, sizeof(string));
			fwrite(string, strlen(string), 1, stderr);
			continue;
		}
		if (strncasecmp(key, "EXIT", 4) == 0) {
			if (op != KEY_OP_ASSIGN) {
				Perror(space, "invalid EXIT operation");
				goto invalid;
			}
			strlcpy(string, value, sizeof(string));
			apply_format(space, string, sizeof(string));
			if (strcmp(string, "return") == 0)
				return -EJUSTRETURN;
			space->exit_code = strtol(string, NULL, 0);
			space->quit = 1;
			break;
		}
		if (strncasecmp(key, "CONFIG{", 7) == 0) {
			attr = get_key_attribute(space, key + 6, string, sizeof(string));
			if (attr == NULL) {
				Perror(space, "error parsing CONFIG attribute");
				goto invalid;
			}
			strlcpy(result, value, sizeof(result));
			apply_format(space, result, sizeof(result));
			if (op == KEY_OP_ASSIGN) {
				err = value_set(space, attr, result);
				dbg("CONFIG{%s}='%s'", attr, result);
				break;
			} else if (op == KEY_OP_MATCH || op == KEY_OP_NOMATCH) {
				pair = value_find(space, attr);
				if (pair == NULL)
					break;
				if (!do_match(key, op, result, pair->value))
					break;
			} else {
				Perror(space, "invalid CONFIG{} operation");
				goto invalid;
			}
		}

		Perror(space, "unknown key '%s'", key);
	}
	return err;

invalid:
	Perror(space, "invalid rule");
	return -EINVAL;
}

static int parse(struct space *space, const char *filename)
{
	char *buf, *bufline, *line;
	size_t bufsize, pos, count, linesize;
	unsigned int linenum, i, j, linenum_adj;
	int err;

	dbg("start of file '%s'", filename);

	if (file_map(filename, &buf, &bufsize) != 0) {
		err = errno;
		error("Unable to open file '%s': %s", filename, strerror(err));
		return -err;
	}

	err = 0;	
	pos = 0;
	linenum = 0;
	linesize = 128;
	line = malloc(linesize);
	if (line == NULL)
		return -ENOMEM;
	space->filename = filename;
	while (!err && pos < bufsize && !space->quit) {
		count = line_width(buf, bufsize, pos);
		bufline = buf + pos;
		pos += count + 1;
		linenum++;
		
		/* skip whitespaces */
		while (count > 0 && isspace(bufline[0])) {
			bufline++;
			count--;
		}
		if (count == 0)
			continue;
			
		/* comment check */
		if (bufline[0] == '#')
			continue;
		
		if (count > linesize - 1) {
			free(line);
			linesize = (count + 127 + 1) & ~127;
			if (linesize > 2048) {
				error("file %s, line %i too long", filename, linenum);
				err = -EINVAL;
				break;
			}
			line = malloc(linesize);
			if (line == NULL) {
				err = -EINVAL;
				break;
			}
		}
		
		/* skip backslash and newline from multiline rules */
		linenum_adj = 0;
		for (i = j = 0; i < count; i++) {
			if (bufline[i] == '\\' && bufline[i+1] == '\n') {
				linenum_adj++;
				continue;
			}
			line[j++] = bufline[i];
		}
		line[j] = '\0';

		dbg("read (%i) '%s'", linenum, line);
		space->linenum = linenum;
		err = parse_line(space, line, linesize);
		if (err == -EJUSTRETURN) {
			err = 0;
			break;
		}
		linenum += linenum_adj;
	}

	free(line);
	space->filename = NULL;
	space->linenum = -1;
	file_unmap(buf, bufsize);
	dbg("end of file '%s'", filename);
	return err ? err : -abs(space->exit_code);
}

int init(const char *filename, const char *cardname)
{
	struct space *space;
	int err = 0, card, first;
	
	sysfs_init();
	if (!cardname) {
		first = 1;
		card = -1;
		while (1) {
			if (snd_card_next(&card) < 0)
				break;
			if (card < 0) {
				if (first) {
					error("No soundcards found...");
					return -ENODEV;
				}
				break;
			}
			first = 0;
			err = init_space(&space, card);
			if (err == 0) {
				space->rootdir = new_root_dir(filename);
				if (space->rootdir != NULL)
					err = parse(space, filename);
				free_space(space);
			}
			if (err < 0)
				break;
		}
	} else {
		card = snd_card_get_index(cardname);
		if (card < 0) {
			error("Cannot find soundcard '%s'...", cardname);
			goto error;
		}
		memset(&space, 0, sizeof(space));
		err = init_space(&space, card);
		if (err == 0) {
			space->rootdir = new_root_dir(filename);
			if (space->rootdir  != NULL)
				err = parse(space, filename);
			free_space(space);
		}
	}
  error:
	sysfs_cleanup();
	return err;
}
