/*
 * Copyright (C) 2010-2014 Tobias Brunner
 * Copyright (C) 2008 Martin Willi
 * Hochschule fuer Technik Rapperswil
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

#define _GNU_SOURCE
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ctype.h>

#include "settings.h"
#include "settings_types.h"

#include "collections/array.h"
#include "collections/hashtable.h"
#include "collections/linked_list.h"
#include "threading/rwlock.h"
#include "utils/debug.h"

typedef struct private_settings_t private_settings_t;

/**
 * Parse function provided by the generated parser.
 */
bool settings_parser_parse_file(section_t *root, char *name);

/**
 * Private data of settings
 */
struct private_settings_t {

	/**
	 * Public interface
	 */
	settings_t public;

	/**
	 * Top level section
	 */
	section_t *top;

	/**
	 * Contents of replaced settings (char*)
	 *
	 * FIXME: This is required because the pointer returned by get_str()
	 * is not refcounted.  Might cause ever increasing usage stats.
	 */
	array_t *contents;

	/**
	 * Lock to safely access the settings
	 */
	rwlock_t *lock;
};

/**
 * Print a format key, but consume already processed arguments
 */
static bool print_key(char *buf, int len, char *start, char *key, va_list args)
{
	va_list copy;
	char *pos = start;
	bool res;

	va_copy(copy, args);
	while (TRUE)
	{
		pos = memchr(pos, '%', key - pos);
		if (!pos)
		{
			break;
		}
		pos++;
		switch (*pos)
		{
			case 'd':
				va_arg(copy, int);
				break;
			case 's':
				va_arg(copy, char*);
				break;
			case 'N':
				va_arg(copy, enum_name_t*);
				va_arg(copy, int);
				break;
			case '%':
				break;
			default:
				DBG1(DBG_CFG, "settings with %%%c not supported!", *pos);
				break;
		}
		pos++;
	}
	res = vsnprintf(buf, len, key, copy) < len;
	va_end(copy);
	return res;
}

/**
 * Find a section by a given key, using buffered key, reusable buffer.
 * If "ensure" is TRUE, the sections are created if they don't exist.
 */
static section_t *find_section_buffered(section_t *section,
					char *start, char *key, va_list args, char *buf, int len,
					bool ensure)
{
	char *pos;
	section_t *found = NULL;

	if (section == NULL)
	{
		return NULL;
	}
	pos = strchr(key, '.');
	if (pos)
	{
		*pos = '\0';
		pos++;
	}
	if (!print_key(buf, len, start, key, args))
	{
		return NULL;
	}
	if (!strlen(buf))
	{
		found = section;
	}
	else if (array_bsearch(section->sections, buf, settings_section_find,
						   &found) == -1)
	{
		if (ensure)
		{
			found = settings_section_create(strdup(buf));
			settings_section_add(section, found, NULL);
		}
	}
	if (found && pos)
	{
		return find_section_buffered(found, start, pos, args, buf, len, ensure);
	}
	return found;
}

/**
 * Find all sections via a given key considering fallbacks, using buffered key,
 * reusable buffer.
 */
static void find_sections_buffered(section_t *section, char *start, char *key,
						va_list args, char *buf, int len, array_t **sections)
{
	section_t *found = NULL, *fallback;
	char *pos;
	int i;

	if (!section)
	{
		return;
	}
	pos = strchr(key, '.');
	if (pos)
	{
		*pos = '\0';
	}
	if (!print_key(buf, len, start, key, args))
	{
		return;
	}
	if (pos)
	{	/* restore so we can follow fallbacks */
		*pos = '.';
	}
	if (!strlen(buf))
	{
		found = section;
	}
	else
	{
		array_bsearch(section->sections, buf, settings_section_find, &found);
	}
	if (found)
	{
		if (pos)
		{
			find_sections_buffered(found, start, pos+1, args, buf, len,
								   sections);
		}
		else
		{
			array_insert_create(sections, ARRAY_TAIL, found);
			for (i = 0; i < array_count(found->fallbacks); i++)
			{
				array_get(found->fallbacks, i, &fallback);
				array_insert_create(sections, ARRAY_TAIL, fallback);
			}
		}
	}
	if (section->fallbacks)
	{
		for (i = 0; i < array_count(section->fallbacks); i++)
		{
			array_get(section->fallbacks, i, &fallback);
			find_sections_buffered(fallback, start, key, args, buf, len,
								   sections);
		}
	}
}

/**
 * Ensure that the section with the given key exists (thread-safe).
 */
static section_t *ensure_section(private_settings_t *this, section_t *section,
								 const char *key, va_list args)
{
	char buf[128], keybuf[512];
	section_t *found;

	if (snprintf(keybuf, sizeof(keybuf), "%s", key) >= sizeof(keybuf))
	{
		return NULL;
	}
	/* we might have to change the tree */
	this->lock->write_lock(this->lock);
	found = find_section_buffered(section, keybuf, keybuf, args, buf,
								  sizeof(buf), TRUE);
	this->lock->unlock(this->lock);
	return found;
}

/**
 * Find a section by a given key with its fallbacks (not thread-safe!).
 * Sections are returned in depth-first order (array is allocated). NULL is
 * returned if no sections are found.
 */
static array_t *find_sections(private_settings_t *this, section_t *section,
							  char *key, va_list args)
{
	char buf[128], keybuf[512];
	array_t *sections = NULL;

	if (snprintf(keybuf, sizeof(keybuf), "%s", key) >= sizeof(keybuf))
	{
		return NULL;
	}
	find_sections_buffered(section, keybuf, keybuf, args, buf,
						   sizeof(buf), &sections);
	return sections;
}

/**
 * Check if the given fallback section already exists
 */
static bool fallback_exists(section_t *section, section_t *fallback)
{
	if (section == fallback)
	{
		return TRUE;
	}
	else if (section->fallbacks)
	{
		section_t *existing;
		int i;

		for (i = 0; i < array_count(section->fallbacks); i++)
		{
			array_get(section->fallbacks, i, &existing);
			if (existing == fallback)
			{
				return TRUE;
			}
		}
	}
	return FALSE;
}

/**
 * Ensure that the section with the given key exists and add the given fallback
 * section (thread-safe).
 */
static void add_fallback_to_section(private_settings_t *this,
							section_t *section, const char *key, va_list args,
							section_t *fallback)
{
	char buf[128], keybuf[512];
	section_t *found;

	if (snprintf(keybuf, sizeof(keybuf), "%s", key) >= sizeof(keybuf))
	{
		return;
	}
	this->lock->write_lock(this->lock);
	found = find_section_buffered(section, keybuf, keybuf, args, buf,
								  sizeof(buf), TRUE);
	if (!fallback_exists(found, fallback))
	{
		/* to ensure sections referred to as fallback are not purged, we create
		 * the array there too */
		if (!fallback->fallbacks)
		{
			fallback->fallbacks = array_create(0, 0);
		}
		array_insert_create(&found->fallbacks, ARRAY_TAIL, fallback);
	}
	this->lock->unlock(this->lock);
}

/**
 * Find the key/value pair for a key, using buffered key, reusable buffer
 * If "ensure" is TRUE, the sections (and key/value pair) are created if they
 * don't exist.
 * Fallbacks are only considered if "ensure" is FALSE.
 */
static kv_t *find_value_buffered(section_t *section, char *start, char *key,
								 va_list args, char *buf, int len, bool ensure)
{
	int i;
	char *pos;
	kv_t *kv = NULL;
	section_t *found = NULL;

	if (section == NULL)
	{
		return NULL;
	}

	pos = strchr(key, '.');
	if (pos)
	{
		*pos = '\0';
		if (!print_key(buf, len, start, key, args))
		{
			return NULL;
		}
		/* restore so we can retry for fallbacks */
		*pos = '.';
		if (!strlen(buf))
		{
			found = section;
		}
		else if (array_bsearch(section->sections, buf, settings_section_find,
							   &found) == -1)
		{
			if (ensure)
			{
				found = settings_section_create(strdup(buf));
				settings_section_add(section, found, NULL);
			}
		}
		if (found)
		{
			kv = find_value_buffered(found, start, pos+1, args, buf, len,
									 ensure);
		}
		if (!kv && !ensure && section->fallbacks)
		{
			for (i = 0; !kv && i < array_count(section->fallbacks); i++)
			{
				array_get(section->fallbacks, i, &found);
				kv = find_value_buffered(found, start, key, args, buf, len,
										 ensure);
			}
		}
	}
	else
	{
		if (!print_key(buf, len, start, key, args))
		{
			return NULL;
		}
		if (array_bsearch(section->kv, buf, settings_kv_find, &kv) == -1)
		{
			if (ensure)
			{
				kv = settings_kv_create(strdup(buf), NULL);
				settings_kv_add(section, kv, NULL);
			}
			else if (section->fallbacks)
			{
				for (i = 0; !kv && i < array_count(section->fallbacks); i++)
				{
					array_get(section->fallbacks, i, &found);
					kv = find_value_buffered(found, start, key, args, buf, len,
											 ensure);
				}
			}
		}
	}
	return kv;
}

/**
 * Find the string value for a key (thread-safe).
 */
static char *find_value(private_settings_t *this, section_t *section,
						char *key, va_list args)
{
	char buf[128], keybuf[512], *value = NULL;
	kv_t *kv;

	if (snprintf(keybuf, sizeof(keybuf), "%s", key) >= sizeof(keybuf))
	{
		return NULL;
	}
	this->lock->read_lock(this->lock);
	kv = find_value_buffered(section, keybuf, keybuf, args, buf, sizeof(buf),
							 FALSE);
	if (kv)
	{
		value = kv->value;
	}
	this->lock->unlock(this->lock);
	return value;
}

/**
 * Set a value to a copy of the given string (thread-safe).
 */
static void set_value(private_settings_t *this, section_t *section,
					  char *key, va_list args, char *value)
{
	char buf[128], keybuf[512];
	kv_t *kv;

	if (snprintf(keybuf, sizeof(keybuf), "%s", key) >= sizeof(keybuf))
	{
		return;
	}
	this->lock->write_lock(this->lock);
	kv = find_value_buffered(section, keybuf, keybuf, args, buf, sizeof(buf),
							 TRUE);
	if (kv)
	{
		settings_kv_set(kv, strdupnull(value), this->contents);
	}
	this->lock->unlock(this->lock);
}

METHOD(settings_t, get_str, char*,
	private_settings_t *this, char *key, char *def, ...)
{
	char *value;
	va_list args;

	va_start(args, def);
	value = find_value(this, this->top, key, args);
	va_end(args);
	if (value)
	{
		return value;
	}
	return def;
}

/**
 * Described in header
 */
inline bool settings_value_as_bool(char *value, bool def)
{
	if (value)
	{
		if (strcaseeq(value, "1") ||
			strcaseeq(value, "yes") ||
			strcaseeq(value, "true") ||
			strcaseeq(value, "enabled"))
		{
			return TRUE;
		}
		else if (strcaseeq(value, "0") ||
				 strcaseeq(value, "no") ||
				 strcaseeq(value, "false") ||
				 strcaseeq(value, "disabled"))
		{
			return FALSE;
		}
	}
	return def;
}

METHOD(settings_t, get_bool, bool,
	private_settings_t *this, char *key, bool def, ...)
{
	char *value;
	va_list args;

	va_start(args, def);
	value = find_value(this, this->top, key, args);
	va_end(args);
	return settings_value_as_bool(value, def);
}

/**
 * Described in header
 */
inline int settings_value_as_int(char *value, int def)
{
	int intval;
	char *end;

	if (value)
	{
		errno = 0;
		intval = strtol(value, &end, 10);
		if (errno == 0 && *end == 0 && end != value)
		{
			return intval;
		}
	}
	return def;
}

METHOD(settings_t, get_int, int,
	private_settings_t *this, char *key, int def, ...)
{
	char *value;
	va_list args;

	va_start(args, def);
	value = find_value(this, this->top, key, args);
	va_end(args);
	return settings_value_as_int(value, def);
}

/**
 * Described in header
 */
inline double settings_value_as_double(char *value, double def)
{
	double dval;
	char *end;

	if (value)
	{
		errno = 0;
		dval = strtod(value, &end);
		if (errno == 0 && *end == 0 && end != value)
		{
			return dval;
		}
	}
	return def;
}

METHOD(settings_t, get_double, double,
	private_settings_t *this, char *key, double def, ...)
{
	char *value;
	va_list args;

	va_start(args, def);
	value = find_value(this, this->top, key, args);
	va_end(args);
	return settings_value_as_double(value, def);
}

/**
 * Described in header
 */
inline u_int32_t settings_value_as_time(char *value, u_int32_t def)
{
	char *endptr;
	u_int32_t timeval;
	if (value)
	{
		errno = 0;
		timeval = strtoul(value, &endptr, 10);
		if (endptr == value)
		{
			return def;
		}
		if (errno == 0)
		{
			while (isspace(*endptr))
			{
				endptr++;
			}
			switch (*endptr)
			{
				case 'd':		/* time in days */
					timeval *= 24 * 3600;
					break;
				case 'h':		/* time in hours */
					timeval *= 3600;
					break;
				case 'm':		/* time in minutes */
					timeval *= 60;
					break;
				case 's':		/* time in seconds */
				case '\0':
					break;
				default:
					return def;
			}
			return timeval;
		}
	}
	return def;
}

METHOD(settings_t, get_time, u_int32_t,
	private_settings_t *this, char *key, u_int32_t def, ...)
{
	char *value;
	va_list args;

	va_start(args, def);
	value = find_value(this, this->top, key, args);
	va_end(args);
	return settings_value_as_time(value, def);
}

METHOD(settings_t, set_str, void,
	private_settings_t *this, char *key, char *value, ...)
{
	va_list args;
	va_start(args, value);
	set_value(this, this->top, key, args, value);
	va_end(args);
}

METHOD(settings_t, set_bool, void,
	private_settings_t *this, char *key, bool value, ...)
{
	va_list args;
	va_start(args, value);
	set_value(this, this->top, key, args, value ? "1" : "0");
	va_end(args);
}

METHOD(settings_t, set_int, void,
	private_settings_t *this, char *key, int value, ...)
{
	char val[16];
	va_list args;
	va_start(args, value);
	if (snprintf(val, sizeof(val), "%d", value) < sizeof(val))
	{
		set_value(this, this->top, key, args, val);
	}
	va_end(args);
}

METHOD(settings_t, set_double, void,
	private_settings_t *this, char *key, double value, ...)
{
	char val[64];
	va_list args;
	va_start(args, value);
	if (snprintf(val, sizeof(val), "%f", value) < sizeof(val))
	{
		set_value(this, this->top, key, args, val);
	}
	va_end(args);
}

METHOD(settings_t, set_time, void,
	private_settings_t *this, char *key, u_int32_t value, ...)
{
	char val[16];
	va_list args;
	va_start(args, value);
	if (snprintf(val, sizeof(val), "%u", value) < sizeof(val))
	{
		set_value(this, this->top, key, args, val);
	}
	va_end(args);
}

METHOD(settings_t, set_default_str, bool,
	private_settings_t *this, char *key, char *value, ...)
{
	char *old;
	va_list args;

	va_start(args, value);
	old = find_value(this, this->top, key, args);
	va_end(args);

	if (!old)
	{
		va_start(args, value);
		set_value(this, this->top, key, args, value);
		va_end(args);
		return TRUE;
	}
	return FALSE;
}

/**
 * Data for enumerators
 */
typedef struct {
	/** settings_t instance */
	private_settings_t *settings;
	/** sections to enumerate */
	array_t *sections;
	/** sections/keys that were already enumerated */
	hashtable_t *seen;
} enumerator_data_t;

/**
 * Destroy enumerator data
 */
static void enumerator_destroy(enumerator_data_t *this)
{
	this->settings->lock->unlock(this->settings->lock);
	this->seen->destroy(this->seen);
	array_destroy(this->sections);
	free(this);
}

/**
 * Enumerate section names, not sections
 */
static bool section_filter(hashtable_t *seen, section_t **in, char **out)
{
	*out = (*in)->name;
	if (seen->get(seen, *out))
	{
		return FALSE;
	}
	seen->put(seen, *out, *out);
	return TRUE;
}

/**
 * Enumerate sections of the given section
 */
static enumerator_t *section_enumerator(section_t *section,
										enumerator_data_t *data)
{
	return enumerator_create_filter(
			array_create_enumerator(section->sections_order),
				(void*)section_filter, data->seen, NULL);
}

METHOD(settings_t, create_section_enumerator, enumerator_t*,
	private_settings_t *this, char *key, ...)
{
	enumerator_data_t *data;
	array_t *sections;
	va_list args;

	this->lock->read_lock(this->lock);
	va_start(args, key);
	sections = find_sections(this, this->top, key, args);
	va_end(args);

	if (!sections)
	{
		this->lock->unlock(this->lock);
		return enumerator_create_empty();
	}
	INIT(data,
		.settings = this,
		.sections = sections,
		.seen = hashtable_create(hashtable_hash_str, hashtable_equals_str, 8),
	);
	return enumerator_create_nested(array_create_enumerator(sections),
					(void*)section_enumerator, data, (void*)enumerator_destroy);
}

/**
 * Enumerate key and values, not kv_t entries
 */
static bool kv_filter(hashtable_t *seen, kv_t **in, char **key,
					  void *none, char **value)
{
	*key = (*in)->key;
	if (seen->get(seen, *key) || !(*in)->value)
	{
		return FALSE;
	}
	*value = (*in)->value;
	seen->put(seen, *key, *key);
	return TRUE;
}

/**
 * Enumerate key/value pairs of the given section
 */
static enumerator_t *kv_enumerator(section_t *section, enumerator_data_t *data)
{
	return enumerator_create_filter(array_create_enumerator(section->kv_order),
					(void*)kv_filter, data->seen, NULL);
}

METHOD(settings_t, create_key_value_enumerator, enumerator_t*,
	private_settings_t *this, char *key, ...)
{
	enumerator_data_t *data;
	array_t *sections;
	va_list args;

	this->lock->read_lock(this->lock);
	va_start(args, key);
	sections = find_sections(this, this->top, key, args);
	va_end(args);

	if (!sections)
	{
		this->lock->unlock(this->lock);
		return enumerator_create_empty();
	}
	INIT(data,
		.settings = this,
		.sections = sections,
		.seen = hashtable_create(hashtable_hash_str, hashtable_equals_str, 8),
	);
	return enumerator_create_nested(array_create_enumerator(sections),
					(void*)kv_enumerator, data, (void*)enumerator_destroy);
}

METHOD(settings_t, add_fallback, void,
	private_settings_t *this, const char *key, const char *fallback, ...)
{
	section_t *section;
	va_list args;

	/* find/create the fallback */
	va_start(args, fallback);
	section = ensure_section(this, this->top, fallback, args);
	va_end(args);

	va_start(args, fallback);
	add_fallback_to_section(this, this->top, key, args, section);
	va_end(args);
}

/**
 * Load settings from files matching the given file pattern.
 * All sections and values are added relative to "parent".
 * All files (even included ones) have to be loaded successfully.
 * If merge is FALSE the contents of parent are replaced with the parsed
 * contents, otherwise they are merged together.
 */
static bool load_files_internal(private_settings_t *this, section_t *parent,
								char *pattern, bool merge)
{
	section_t *section;

	if (pattern == NULL || !pattern[0])
	{	/* TODO: Clear parent if merge is FALSE? */
		return TRUE;
	}

	section = settings_section_create(NULL);
	if (!settings_parser_parse_file(section, pattern))
	{
		settings_section_destroy(section, NULL);
		return FALSE;
	}

	this->lock->write_lock(this->lock);
	settings_section_extend(parent, section, this->contents, !merge);
	this->lock->unlock(this->lock);

	settings_section_destroy(section, NULL);
	return TRUE;
}

METHOD(settings_t, load_files, bool,
	private_settings_t *this, char *pattern, bool merge)
{
	return load_files_internal(this, this->top, pattern, merge);
}

METHOD(settings_t, load_files_section, bool,
	private_settings_t *this, char *pattern, bool merge, char *key, ...)
{
	section_t *section;
	va_list args;

	va_start(args, key);
	section = ensure_section(this, this->top, key, args);
	va_end(args);

	if (!section)
	{
		return FALSE;
	}
	return load_files_internal(this, section, pattern, merge);
}

METHOD(settings_t, destroy, void,
	private_settings_t *this)
{
	settings_section_destroy(this->top, NULL);
	array_destroy_function(this->contents, (void*)free, NULL);
	this->lock->destroy(this->lock);
	free(this);
}

/*
 * see header file
 */
settings_t *settings_create(char *file)
{
	private_settings_t *this;

	INIT(this,
		.public = {
			.get_str = _get_str,
			.get_int = _get_int,
			.get_double = _get_double,
			.get_time = _get_time,
			.get_bool = _get_bool,
			.set_str = _set_str,
			.set_int = _set_int,
			.set_double = _set_double,
			.set_time = _set_time,
			.set_bool = _set_bool,
			.set_default_str = _set_default_str,
			.create_section_enumerator = _create_section_enumerator,
			.create_key_value_enumerator = _create_key_value_enumerator,
			.add_fallback = _add_fallback,
			.load_files = _load_files,
			.load_files_section = _load_files_section,
			.destroy = _destroy,
		},
		.top = settings_section_create(NULL),
		.contents = array_create(0, 0),
		.lock = rwlock_create(RWLOCK_TYPE_DEFAULT),
	);

	load_files(this, file, FALSE);

	return &this->public;
}
