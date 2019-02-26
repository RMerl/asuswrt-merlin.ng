/*
 * Copyright (C) 2013-2014 Tobias Brunner
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

#include "conf_parser.h"

#include <collections/array.h>
#include <collections/hashtable.h>

/**
 * Provided by the generated parser
 */
bool conf_parser_parse_file(conf_parser_t *this, char *file);

typedef struct private_conf_parser_t private_conf_parser_t;
typedef struct section_t section_t;

/**
 * Private data
 */
struct private_conf_parser_t {

	/**
	 * Public interface
	 */
	conf_parser_t public;

	/**
	 * Path to config file
	 */
	char *file;

	/**
	 * TRUE while parsing the config file
	 */
	bool parsing;

	/**
	 * Hashtable for ca sections, as section_t
	 */
	hashtable_t *cas;

	/**
	 * Hashtable for conn sections, as section_t
	 */
	hashtable_t *conns;

	/**
	 * Array to keep track of the order of conn sections, as section_t
	 */
	array_t *conns_order;

	/**
	 * config setup section
	 */
	section_t *config_setup;

	/**
	 * Pointer to the current section (the one added last) during parsing
	 */
	section_t *current_section;

	/**
	 * Refcount for this parser instance (also used by dictionaries)
	 */
	refcount_t ref;
};

typedef struct {
	/** Key of the setting */
	char *key;
	/** Value of the setting */
	char *value;
} setting_t;

int setting_find(const void *a, const void *b)
{
	const char *key = a;
	const setting_t *setting = b;
	return strcmp(key, setting->key);
}

int setting_sort(const void *a, const void *b, void *user)
{
	const setting_t *sa = a, *sb = b;
	return strcmp(sa->key, sb->key);
}

static void setting_destroy(setting_t *this)
{
	free(this->key);
	free(this->value);
	free(this);
}

struct section_t {
	/** Name of the section */
	char *name;
	/** Sorted array of settings, as setting_t */
	array_t *settings;
	/** Array of also= settings (in reversed order, i.e. most important to least
	 * important), as setting_t */
	array_t *also;
	/** Array of linearized parent objects derived from also= settings, in their
	 * order of importance (most to least, i.e. %default) as section_t
	 * NULL if not yet determined */
	array_t *parents;
};

static section_t *section_create(char *name)
{
	section_t *this;

	INIT(this,
		.name = name,
	);
	return this;
}

static void section_destroy(section_t *this)
{
	array_destroy_function(this->settings, (void*)setting_destroy, NULL);
	array_destroy_function(this->also, (void*)setting_destroy, NULL);
	array_destroy(this->parents);
	free(this->name);
	free(this);
}

typedef struct {
	/** Public interface */
	dictionary_t public;
	/** Parser object */
	private_conf_parser_t *parser;
	/** Section object */
	section_t *section;
} section_dictionary_t;

typedef struct {
	/** Public interface */
	enumerator_t public;
	/** Current settings enumerator */
	enumerator_t *settings;
	/** Enumerator into parent list */
	enumerator_t *parents;
	/** Hashtable to keep track of already enumerated settings */
	hashtable_t *seen;
} dictionary_enumerator_t;

METHOD(enumerator_t, dictionary_enumerate, bool,
	dictionary_enumerator_t *this, va_list args)
{
	setting_t *setting;
	section_t *parent;
	char **key, **value;

	VA_ARGS_VGET(args, key, value);

	while (TRUE)
	{
		if (this->settings &&
			this->settings->enumerate(this->settings, &setting))
		{
			if (this->seen->get(this->seen, setting->key))
			{
				continue;
			}
			this->seen->put(this->seen, setting->key, setting->key);
			if (!setting->value)
			{
				continue;
			}
			if (key)
			{
				*key = setting->key;
			}
			if (value)
			{
				*value = setting->value;
			}
			return TRUE;
		}
		DESTROY_IF(this->settings);
		this->settings = NULL;
		if (this->parents &&
			this->parents->enumerate(this->parents, &parent))
		{
			if (parent->settings)
			{
				this->settings = array_create_enumerator(parent->settings);
			}
			continue;
		}
		DESTROY_IF(this->parents);
		this->parents = NULL;
		break;
	}
	return FALSE;
}

METHOD(enumerator_t, dictionary_enumerator_destroy, void,
	dictionary_enumerator_t *this)
{
	DESTROY_IF(this->settings);
	DESTROY_IF(this->parents);
	this->seen->destroy(this->seen);
	free(this);
}

METHOD(dictionary_t, dictionary_create_enumerator, enumerator_t*,
	section_dictionary_t *this)
{
	dictionary_enumerator_t *enumerator;

	INIT(enumerator,
		.public = {
			.enumerate = enumerator_enumerate_default,
			.venumerate = _dictionary_enumerate,
			.destroy = _dictionary_enumerator_destroy,
		},
		.seen = hashtable_create(hashtable_hash_str, hashtable_equals_str, 8),
	);
	if (this->section->settings)
	{
		enumerator->settings = array_create_enumerator(this->section->settings);
	}
	if (this->section->parents)
	{
		enumerator->parents = array_create_enumerator(this->section->parents);
	}
	return &enumerator->public;
}

METHOD(dictionary_t, dictionary_get, void*,
	section_dictionary_t *this, const void *key)
{
	enumerator_t *parents;
	section_t *section;
	setting_t *setting;
	char *value = NULL;

	section = this->section;
	if (array_bsearch(section->settings, key, setting_find, &setting) != -1)
	{
		return setting->value;
	}
	parents = array_create_enumerator(section->parents);
	while (parents->enumerate(parents, &section))
	{
		if (array_bsearch(section->settings, key, setting_find, &setting) != -1)
		{
			value = setting->value;
			break;
		}
	}
	parents->destroy(parents);
	return value;
}

METHOD(dictionary_t, dictionary_destroy, void,
	section_dictionary_t *this)
{
	this->parser->public.destroy(&this->parser->public);
	free(this);
}

static dictionary_t *section_dictionary_create(private_conf_parser_t *parser,
											   section_t *section)
{
	section_dictionary_t *this;

	INIT(this,
		.public = {
			.create_enumerator = _dictionary_create_enumerator,
			.get = _dictionary_get,
			.destroy = _dictionary_destroy,
		},
		.parser = parser,
		.section = section,
	);

	ref_get(&parser->ref);

	return &this->public;
}

CALLBACK(conn_filter, bool,
	void *unused, enumerator_t *orig, va_list args)
{
	section_t *section;
	char **name;

	VA_ARGS_VGET(args, name);

	while (orig->enumerate(orig, &section))
	{
		if (!streq(section->name, "%default"))
		{
			*name = section->name;
			return TRUE;
		}
	}
	return FALSE;
}

CALLBACK(ca_filter, bool,
	void *unused, enumerator_t *orig, va_list args)
{
	void *key;
	section_t *section;
	char **name;

	VA_ARGS_VGET(args, name);

	while (orig->enumerate(orig, &key, &section))
	{
		if (!streq(section->name, "%default"))
		{
			*name = section->name;
			return TRUE;
		}
	}
	return FALSE;
}

METHOD(conf_parser_t, get_sections, enumerator_t*,
	private_conf_parser_t *this, conf_parser_section_t type)
{
	switch (type)
	{
		case CONF_PARSER_CONN:
			return enumerator_create_filter(
									array_create_enumerator(this->conns_order),
									conn_filter, NULL, NULL);
		case CONF_PARSER_CA:
			return enumerator_create_filter(
									this->cas->create_enumerator(this->cas),
									ca_filter, NULL, NULL);
		case CONF_PARSER_CONFIG_SETUP:
		default:
			return enumerator_create_empty();
	}
}

METHOD(conf_parser_t, get_section, dictionary_t*,
	private_conf_parser_t *this, conf_parser_section_t type, char *name)
{
	section_t *section = NULL;

	if (name && streq(name, "%default"))
	{
		return NULL;
	}
	switch (type)
	{
		case CONF_PARSER_CONFIG_SETUP:
			section = this->config_setup;
			break;
		case CONF_PARSER_CONN:
			section = this->conns->get(this->conns, name);
			break;
		case CONF_PARSER_CA:
			section = this->cas->get(this->cas, name);
			break;
		default:
			break;
	}
	return section ? section_dictionary_create(this, section) : NULL;
}

METHOD(conf_parser_t, add_section, bool,
	private_conf_parser_t *this, conf_parser_section_t type, char *name)
{
	hashtable_t *sections = this->conns;
	array_t *order = this->conns_order;
	section_t *section = NULL;
	bool exists = FALSE;

	if (!this->parsing)
	{
		free(name);
		return exists;
	}
	switch (type)
	{
		case CONF_PARSER_CONFIG_SETUP:
			section = this->config_setup;
			/* we don't expect a name, but just in case */
			free(name);
			break;
		case CONF_PARSER_CA:
			sections = this->cas;
			order = NULL;
			/* fall-through */
		case CONF_PARSER_CONN:
			section = sections->get(sections, name);
			if (!section)
			{
				section = section_create(name);
				sections->put(sections, name, section);
				if (order)
				{
					array_insert(order, ARRAY_TAIL, section);
				}
			}
			else
			{
				exists = TRUE;
				free(name);
			}
			break;

	}
	this->current_section = section;
	return exists;
}

METHOD(conf_parser_t, add_setting, void,
	private_conf_parser_t *this, char *key, char *value)
{
	section_t *section = this->current_section;
	setting_t *setting;

	if (!this->parsing || !this->current_section)
	{
		free(key);
		free(value);
		return;
	}
	if (streq(key, "also"))
	{
		if (!value || !strlen(value) || streq(value, "%default"))
		{	/* we require a name, but all sections inherit from %default */
			free(key);
			free(value);
			return;
		}
		INIT(setting,
			.key = key,
			.value = value,
		);
		array_insert_create(&section->also, ARRAY_HEAD, setting);
		return;
	}
	if (array_bsearch(section->settings, key, setting_find, &setting) == -1)
	{
		INIT(setting,
			.key = key,
			.value = value,
		);
		array_insert_create(&section->settings, ARRAY_TAIL, setting);
		array_sort(section->settings, setting_sort, NULL);
	}
	else
	{
		free(setting->value);
		setting->value = value;
		free(key);
	}
}

/**
 * Check if the given section is contained in the given array.  The search
 * starts at the given index.
 */
static bool is_contained_in(array_t *arr, section_t *section)
{
	section_t *current;
	int i;

	for (i = 0; i < array_count(arr); i++)
	{
		array_get(arr, i, &current);
		if (streq(section->name, current->name))
		{
			return TRUE;
		}
	}
	return FALSE;
}

/**
 * This algorithm to linearize sections uses a bottom-first depth-first
 * semantic, with an additional elimination step that removes all but the
 * last occurrence of each section.
 *
 * Consider this configuration:
 *
 *   conn A
 *   conn B
 *     also=A
 *   conn C
 *     also=A
 *   conn D
 *     also=C
 *     also=B
 *
 * The linearization would yield D B A C A, which gets reduced to D B C A.
 *
 * Ambiguous configurations are handled pragmatically.
 *
 * Consider the following configuration:
 *
 *   conn A
 *   conn B
 *   conn C
 *     also=A
 *     also=B
 *   conn D
 *     also=B
 *     also=A
 *   conn E
 *     also=C
 *     also=D
 *
 * It is ambiguous because D and C include the same two sections but in
 * a different order.
 *
 * The linearization would yield E D A B C B A which gets reduced to E D C B A.
 */
static bool resolve_also_single(hashtable_t *sections,
							section_t *section, section_t *def, array_t *stack)
{
	enumerator_t *enumerator;
	array_t *parents;
	section_t *parent, *grandparent;
	setting_t *also;
	bool success = TRUE;
	int i;

	array_insert(stack, ARRAY_HEAD, section);
	parents = array_create(0, 0);
	enumerator = array_create_enumerator(section->also);
	while (enumerator->enumerate(enumerator, &also))
	{
		parent = sections->get(sections, also->value);
		if (!parent || is_contained_in(stack, parent))
		{
			if (!parent)
			{
				DBG1(DBG_CFG, "section '%s' referenced in section '%s' not "
					 "found", also->value, section->name);
			}
			else
			{
				DBG1(DBG_CFG, "section '%s' referenced in section '%s' causes "
					 "a loop", parent->name, section->name);
			}
			array_remove_at(section->also, enumerator);
			setting_destroy(also);
			success = FALSE;
			continue;
		}
		if (!parent->parents)
		{
			if (!resolve_also_single(sections, parent, def, stack))
			{
				success = FALSE;
				continue;
			}
		}
		/* add the grandparents and the parent to the list */
		array_insert(parents, ARRAY_TAIL, parent);
		for (i = 0; i < array_count(parent->parents); i++)
		{
			array_get(parent->parents, i, &grandparent);
			array_insert(parents, ARRAY_TAIL, grandparent);
		}
	}
	enumerator->destroy(enumerator);
	array_remove(stack, ARRAY_HEAD, NULL);

	if (success && def && !array_count(parents))
	{
		array_insert(parents, ARRAY_TAIL, def);
	}

	while (success && array_remove(parents, ARRAY_HEAD, &parent))
	{
		if (!is_contained_in(parents, parent))
		{	/* last occurrence of this section */
			array_insert_create(&section->parents, ARRAY_TAIL, parent);
		}
	}
	array_destroy(parents);
	return success;
}

/**
 * Resolve also= statements. The functions returns TRUE if everything is fine,
 * or FALSE if either a referenced section does not exist, or if the section
 * inheritance can't be determined properly (e.g. if there are loops or if a
 * section inherits from multiple sections - perhaps over several levels - in
 * an ambiguous way).
 */
static bool resolve_also(hashtable_t *sections)
{
	enumerator_t *enumerator;
	section_t *def, *section;
	array_t *stack;
	bool success = TRUE;

	stack = array_create(0, 0);

	def = sections->get(sections, "%default");
	if (def)
	{	/* the default section is the only one with an empty parents list */
		def->parents = array_create(0, 0);
	}
	enumerator = sections->create_enumerator(sections);
	while (enumerator->enumerate(enumerator, NULL, &section))
	{
		if (section->parents)
		{	/* already determined */
			continue;
		}
		success = resolve_also_single(sections, section, def, stack) && success;
	}
	enumerator->destroy(enumerator);
	array_destroy(stack);
	return success;
}

METHOD(conf_parser_t, parse, bool,
	private_conf_parser_t *this)
{
	bool success;

	if (!this->file)
	{	/* no file, lets assume this is OK */
		return TRUE;
	}
	this->parsing = TRUE;
	success = conf_parser_parse_file(&this->public, this->file);
	this->parsing = FALSE;
	return success && resolve_also(this->conns) && resolve_also(this->cas);
}

METHOD(conf_parser_t, destroy, void,
	private_conf_parser_t *this)
{
	if (ref_put(&this->ref))
	{
		this->cas->destroy_function(this->cas, (void*)section_destroy);
		this->conns->destroy_function(this->conns, (void*)section_destroy);
		section_destroy(this->config_setup);
		array_destroy(this->conns_order);
		free(this->file);
		free(this);
	}
}

/*
 * Described in header
 */
conf_parser_t *conf_parser_create(const char *file)
{
	private_conf_parser_t *this;

	INIT(this,
		.public = {
			.parse = _parse,
			.get_sections = _get_sections,
			.get_section = _get_section,
			.add_section = _add_section,
			.add_setting = _add_setting,
			.destroy = _destroy,
		},
		.file = strdupnull(file),
		.cas = hashtable_create(hashtable_hash_str,
								hashtable_equals_str, 8),
		.conns = hashtable_create(hashtable_hash_str,
								  hashtable_equals_str, 8),
		.conns_order = array_create(0, 0),
		.config_setup = section_create(NULL),
		.ref = 1,
	);

	return &this->public;
}
