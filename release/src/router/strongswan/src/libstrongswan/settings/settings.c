/*
 * Copyright (C) 2010-2018 Tobias Brunner
 * Copyright (C) 2008 Martin Willi
 * HSR Hochschule fuer Technik Rapperswil
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
 * Parse functions provided by the generated parser.
 */
bool settings_parser_parse_file(section_t *root, char *name);
bool settings_parser_parse_string(section_t *root, char *settings);

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
 * Note that key and start point into the same string
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
 * Check if the given section is contained in the given array.
 */
static bool has_section(array_t *array, section_t *section)
{
	section_t *current;
	int i;

	for (i = 0; i < array_count(array); i++)
	{
		array_get(array, i, &current);
		if (current == section)
		{
			return TRUE;
		}
	}
	return FALSE;
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
 * Forward declaration
 */
static array_t *find_sections(private_settings_t *this, section_t *section,
							  char *key, va_list args, array_t **sections);

/**
 * Resolve the given reference. Not thread-safe.
 * Only a vararg function to get an empty va_list.
 */
static void resolve_reference(private_settings_t *this, section_ref_t *ref,
						array_t **sections, ...)
{
	va_list args;

	va_start(args, sections);
	find_sections(this, this->top, ref->name, args, sections);
	va_end(args);
}

/**
 * Find all sections via a given key considering references, using buffered key,
 * reusable buffer.
 */
static void find_sections_buffered(private_settings_t *this, section_t *section,
								   char *start, char *key, va_list args,
								   char *buf, int len, bool ignore_refs,
								   array_t **sections)
{
	section_t *found = NULL, *reference;
	array_t *references;
	section_ref_t *ref;
	char *pos;
	int i, j;

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
	{	/* restore so we can follow references */
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
			find_sections_buffered(this, found, start, pos+1, args, buf, len,
								   FALSE, sections);
		}
		else if (!has_section(*sections, found))
		{
			/* ignore if already added to avoid loops */
			array_insert_create(sections, ARRAY_TAIL, found);
			/* add all sections that are referenced here (also resolves
			 * references in parent sections of the referenced section) */
			for (i = 0; i < array_count(found->references); i++)
			{
				array_get(found->references, i, &ref);
				resolve_reference(this, ref, sections);
			}
		}
	}
	if (!ignore_refs && section != found && section->references)
	{
		/* find matching sub-sections relative to the referenced sections */
		for (i = 0; i < array_count(section->references); i++)
		{
			array_get(section->references, i, &ref);
			references = NULL;
			resolve_reference(this, ref, &references);
			for (j = 0; j < array_count(references); j++)
			{
				array_get(references, j, &reference);
				/* ignore references in this referenced section, they were
				 * resolved via resolve_reference() */
				find_sections_buffered(this, reference, start, key, args,
									   buf, len, TRUE, sections);
			}
			array_destroy(references);
		}
	}
}

/**
 * Ensure that the section with the given key exists (not thread-safe).
 */
static section_t *ensure_section(private_settings_t *this, section_t *section,
								 const char *key, va_list args)
{
	char buf[128], keybuf[512];

	if (snprintf(keybuf, sizeof(keybuf), "%s", key) >= sizeof(keybuf))
	{
		return NULL;
	}
	return find_section_buffered(section, keybuf, keybuf, args, buf,
								 sizeof(buf), TRUE);
}

/**
 * Find a section by a given key with resolved references (not thread-safe!).
 * The array is allocated. NULL is returned if no sections are found.
 */
static array_t *find_sections(private_settings_t *this, section_t *section,
							  char *key, va_list args, array_t **sections)
{
	char buf[128], keybuf[512];

	if (snprintf(keybuf, sizeof(keybuf), "%s", key) >= sizeof(keybuf))
	{
		return NULL;
	}
	find_sections_buffered(this, section, keybuf, keybuf, args, buf,
						   sizeof(buf), FALSE, sections);
	return *sections;
}

/**
 * Find the key/value pair for a key, using buffered key, reusable buffer
 * There are two modes: 1. To find a key at an exact location and create the
 * sections (and key/value pair) if necessary, don't pass an array for sections.
 * 2. To find a key and follow references pass a pointer to an array to store
 * visited sections. NULL is returned in this case if the key is not found.
 */
static kv_t *find_value_buffered(private_settings_t *this, section_t *section,
								 char *start, char *key, va_list args,
								 char *buf, int len, bool ignore_refs,
								 array_t **sections)
{
	section_t *found = NULL;
	kv_t *kv = NULL;
	section_ref_t *ref;
	array_t *references;
	char *pos;
	int i, j;

	if (!section)
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
		/* restore so we can follow references */
		*pos = '.';
		if (!strlen(buf))
		{
			found = section;
		}
		else if (array_bsearch(section->sections, buf, settings_section_find,
							   &found) == -1)
		{
			if (!sections)
			{
				found = settings_section_create(strdup(buf));
				settings_section_add(section, found, NULL);
			}
		}
		if (found)
		{
			kv = find_value_buffered(this, found, start, pos+1, args, buf, len,
									 FALSE, sections);
		}
	}
	else
	{
		if (sections)
		{
			array_insert_create(sections, ARRAY_TAIL, section);
		}
		if (!print_key(buf, len, start, key, args))
		{
			return NULL;
		}
		if (array_bsearch(section->kv, buf, settings_kv_find, &kv) == -1)
		{
			if (!sections)
			{
				kv = settings_kv_create(strdup(buf), NULL);
				settings_kv_add(section, kv, NULL);
			}
		}
	}
	if (!kv && !ignore_refs && sections && section->references)
	{
		/* find key relative to the referenced sections */
		for (i = 0; !kv && i < array_count(section->references); i++)
		{
			array_get(section->references, i, &ref);
			references = NULL;
			resolve_reference(this, ref, &references);
			for (j = 0; !kv && j < array_count(references); j++)
			{
				array_get(references, j, &found);
				/* ignore if already added to avoid loops */
				if (!has_section(*sections, found))
				{
					/* ignore references in this referenced section, they were
					 * resolved via resolve_reference() */
					kv = find_value_buffered(this, found, start, key, args,
											 buf, len, TRUE, sections);
				}
			}
			array_destroy(references);
		}
	}
	return kv;
}

/**
 * Remove the key/value pair for a key, using buffered key, reusable buffer
 */
static void remove_value_buffered(private_settings_t *this, section_t *section,
								  char *start, char *key, va_list args,
								  char *buf, int len)
{
	section_t *found = NULL;
	kv_t *kv = NULL, *ordered = NULL;
	char *pos;
	int idx, i;

	if (!section)
	{
		return;
	}
	pos = strchr(key, '.');
	if (pos)
	{
		*pos = '\0';
		pos++;
	}
	if (!print_key(buf, len, start, key, args))
	{
		return;
	}
	if (!strlen(buf))
	{
		found = section;
	}
	if (pos)
	{
		if (array_bsearch(section->sections, buf, settings_section_find,
						  &found) != -1)
		{
			remove_value_buffered(this, found, start, pos, args, buf, len);
		}
	}
	else
	{
		idx = array_bsearch(section->kv, buf, settings_kv_find, &kv);
		if (idx != -1)
		{
			array_remove(section->kv, idx, NULL);
			for (i = 0; i < array_count(section->kv_order); i++)
			{
				array_get(section->kv_order, i, &ordered);
				if (kv == ordered)
				{
					array_remove(section->kv_order, i, NULL);
					settings_kv_destroy(kv, this->contents);
					break;
				}
			}
		}
	}
}

/*
 * Described in header
 */
void settings_remove_value(settings_t *settings, char *key, ...)
{
	private_settings_t *this = (private_settings_t*)settings;
	char buf[128], keybuf[512];
	va_list args;

	if (snprintf(keybuf, sizeof(keybuf), "%s", key) >= sizeof(keybuf))
	{
		return;
	}
	va_start(args, key);

	this->lock->read_lock(this->lock);
	remove_value_buffered(this, this->top, keybuf, keybuf, args, buf,
						  sizeof(buf));
	this->lock->unlock(this->lock);

	va_end(args);
}

/**
 * Find the string value for a key (thread-safe).
 */
static char *find_value(private_settings_t *this, section_t *section,
						char *key, va_list args)
{
	char buf[128], keybuf[512], *value = NULL;
	array_t *sections = NULL;
	kv_t *kv;

	if (snprintf(keybuf, sizeof(keybuf), "%s", key) >= sizeof(keybuf))
	{
		return NULL;
	}
	this->lock->read_lock(this->lock);
	kv = find_value_buffered(this, section, keybuf, keybuf, args,
							 buf, sizeof(buf), FALSE, &sections);
	if (kv)
	{
		value = kv->value;
	}
	this->lock->unlock(this->lock);
	array_destroy(sections);
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
	kv = find_value_buffered(this, section, keybuf, keybuf, args,
							 buf, sizeof(buf), FALSE, NULL);
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
	private_settings_t *this, char *key, int def, ...)
{
	char *value;
	va_list args;

	/* we can't use bool for def due to this call */
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
	int base = 10;

	if (value)
	{
		errno = 0;
		if (value[0] == '0' && value[1] == 'x')
		{	/* manually detect 0x prefix as we want to avoid octal encoding */
			base = 16;
		}
		intval = strtol(value, &end, base);
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
inline uint64_t settings_value_as_uint64(char *value, uint64_t def)
{
	uint64_t intval;
	char *end;
	int base = 10;

	if (value)
	{
		errno = 0;
		if (value[0] == '0' && value[1] == 'x')
		{	/* manually detect 0x prefix as we want to avoid octal encoding */
			base = 16;
		}
		intval = strtoull(value, &end, base);
		if (errno == 0 && *end == 0 && end != value)
		{
			return intval;
		}
	}
	return def;
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
inline uint32_t settings_value_as_time(char *value, uint32_t def)
{
	time_t val;

	if (timespan_from_string(value, NULL, &val))
	{
		return val;
	}
	return def;
}

METHOD(settings_t, get_time, uint32_t,
	private_settings_t *this, char *key, uint32_t def, ...)
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
	private_settings_t *this, char *key, int value, ...)
{
	va_list args;
	/* we can't use bool for value due to this call */
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
	private_settings_t *this, char *key, uint32_t value, ...)
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

CALLBACK(enumerator_destroy, void,
	enumerator_data_t *this)
{
	this->settings->lock->unlock(this->settings->lock);
	this->seen->destroy(this->seen);
	array_destroy(this->sections);
	free(this);
}

CALLBACK(section_filter, bool,
	hashtable_t *seen, enumerator_t *orig, va_list args)
{
	section_t *section;
	char **out;

	VA_ARGS_VGET(args, out);

	while (orig->enumerate(orig, &section))
	{
		if (seen->get(seen, section->name))
		{
			continue;
		}
		*out = section->name;
		seen->put(seen, section->name, section->name);
		return TRUE;
	}
	return FALSE;
}

/**
 * Enumerate sections of the given section
 */
static enumerator_t *section_enumerator(section_t *section,
										enumerator_data_t *data)
{
	return enumerator_create_filter(
							array_create_enumerator(section->sections_order),
							section_filter, data->seen, NULL);
}

METHOD(settings_t, create_section_enumerator, enumerator_t*,
	private_settings_t *this, char *key, ...)
{
	enumerator_data_t *data;
	array_t *sections = NULL;
	va_list args;

	this->lock->read_lock(this->lock);
	va_start(args, key);
	sections = find_sections(this, this->top, key, args, &sections);
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
						(void*)section_enumerator, data, enumerator_destroy);
}

CALLBACK(kv_filter, bool,
	hashtable_t *seen, enumerator_t *orig, va_list args)
{
	kv_t *kv;
	char **key, **value;

	VA_ARGS_VGET(args, key, value);

	while (orig->enumerate(orig, &kv))
	{
		if (seen->get(seen, kv->key))
		{
			continue;
		}
		seen->put(seen, kv->key, kv->key);
		if (!kv->value)
		{
			continue;
		}
		*key = kv->key;
		*value = kv->value;
		return TRUE;
	}
	return FALSE;
}

/**
 * Enumerate key/value pairs of the given section
 */
static enumerator_t *kv_enumerator(section_t *section, enumerator_data_t *data)
{
	return enumerator_create_filter(array_create_enumerator(section->kv_order),
									kv_filter, data->seen, NULL);
}

METHOD(settings_t, create_key_value_enumerator, enumerator_t*,
	private_settings_t *this, char *key, ...)
{
	enumerator_data_t *data;
	array_t *sections = NULL;
	va_list args;

	this->lock->read_lock(this->lock);
	va_start(args, key);
	sections = find_sections(this, this->top, key, args, &sections);
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
	char buf[512];

	this->lock->write_lock(this->lock);
	va_start(args, fallback);
	section = ensure_section(this, this->top, key, args);
	va_end(args);

	va_start(args, fallback);
	if (section && vsnprintf(buf, sizeof(buf), fallback, args) < sizeof(buf))
	{
		settings_reference_add(section, strdup(buf), TRUE);
	}
	va_end(args);
	this->lock->unlock(this->lock);
}

/**
 * Load settings from files matching the given file pattern or from a string.
 * All files (even included ones) have to be loaded successfully.
 */
static section_t *load_internal(char *pattern, bool string)
{
	section_t *section;
	bool loaded;

	if (pattern == NULL || !pattern[0])
	{
		return settings_section_create(NULL);
	}

	section = settings_section_create(NULL);
	loaded = string ? settings_parser_parse_string(section, pattern) :
					  settings_parser_parse_file(section, pattern);
	if (!loaded)
	{
		settings_section_destroy(section, NULL);
		section = NULL;
	}
	return section;
}

/**
 * Add sections and values in "section" relative to "parent".
 * If merge is FALSE the contents of parent are replaced with the parsed
 * contents, otherwise they are merged together.
 *
 * Releases the write lock and destroys the given section.
 * If parent is NULL this is all that happens.
 */
static bool extend_section(private_settings_t *this, section_t *parent,
						   section_t *section, bool merge)
{
	if (parent)
	{
		settings_section_extend(parent, section, this->contents, !merge);
	}
	this->lock->unlock(this->lock);
	settings_section_destroy(section, NULL);
	return parent != NULL;
}

METHOD(settings_t, load_files, bool,
	private_settings_t *this, char *pattern, bool merge)
{
	section_t *section;

	section = load_internal(pattern, FALSE);
	if (!section)
	{
		return FALSE;
	}

	this->lock->write_lock(this->lock);
	return extend_section(this, this->top, section, merge);
}

METHOD(settings_t, load_files_section, bool,
	private_settings_t *this, char *pattern, bool merge, char *key, ...)
{
	section_t *section, *parent;
	va_list args;

	section = load_internal(pattern, FALSE);
	if (!section)
	{
		return FALSE;
	}

	this->lock->write_lock(this->lock);

	va_start(args, key);
	parent = ensure_section(this, this->top, key, args);
	va_end(args);

	return extend_section(this, parent, section, merge);
}

METHOD(settings_t, load_string, bool,
	private_settings_t *this, char *settings, bool merge)
{
	section_t *section;

	section = load_internal(settings, TRUE);
	if (!section)
	{
		return FALSE;
	}

	this->lock->write_lock(this->lock);
	return extend_section(this, this->top, section, merge);
}

METHOD(settings_t, load_string_section, bool,
	private_settings_t *this, char *settings, bool merge, char *key, ...)
{
	section_t *section, *parent;
	va_list args;

	section = load_internal(settings, TRUE);
	if (!section)
	{
		return FALSE;
	}

	this->lock->write_lock(this->lock);

	va_start(args, key);
	parent = ensure_section(this, this->top, key, args);
	va_end(args);

	return extend_section(this, parent, section, merge);
}

METHOD(settings_t, destroy, void,
	private_settings_t *this)
{
	settings_section_destroy(this->top, NULL);
	array_destroy_function(this->contents, (void*)free, NULL);
	this->lock->destroy(this->lock);
	free(this);
}

static private_settings_t *settings_create_base()
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
			.load_string = _load_string,
			.load_string_section = _load_string_section,
			.destroy = _destroy,
		},
		.top = settings_section_create(NULL),
		.contents = array_create(0, 0),
		.lock = rwlock_create(RWLOCK_TYPE_DEFAULT),
	);
	return this;
}

/*
 * see header file
 */
settings_t *settings_create(char *file)
{
	private_settings_t *this = settings_create_base();

	load_files(this, file, FALSE);

	return &this->public;
}

/*
 * see header file
 */
settings_t *settings_create_string(char *settings)
{
	private_settings_t *this = settings_create_base();

	load_string(this, settings, FALSE);

	return &this->public;
}
