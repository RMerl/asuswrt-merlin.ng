/*
 * Copyright (C) 2010-2018 Tobias Brunner
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

/**
 * Internal data types and functions shared between the parser and t.
 *
 * @defgroup settings_types settings_types
 * @{ @ingroup settings
 */

#ifndef SETTINGS_TYPES_H_
#define SETTINGS_TYPES_H_

typedef struct kv_t kv_t;
typedef struct section_ref_t section_ref_t;
typedef struct section_t section_t;

#include "collections/array.h"

/**
 * Key/value pair.
 */
struct kv_t {

	/**
	 * Key string, relative, not the full name.
	 */
	char *key;

	/**
	 * Value as string.
	 */
	char *value;
};

/**
 * Section reference.
 */
struct section_ref_t {

	/**
	 * Name of the referenced section.
	 */
	char *name;

	/**
	 * TRUE for permanent references that were added programmatically via
	 * add_fallback() and are not removed during reloads/purges.
	 */
	bool permanent;
};

/**
 * Section containing subsections and key value pairs.
 */
struct section_t {

	/**
	 * Name of the section.
	 */
	char *name;

	/**
	 * Referenced sections, as section_ref_t.
	 */
	array_t *references;

	/**
	 * Subsections, as section_t.
	 */
	array_t *sections;

	/**
	 * Subsections in original order, as section_t (pointer to obj in sections).
	 */
	array_t *sections_order;

	/**
	 * Key value pairs, as kv_t.
	 */
	array_t *kv;

	/**
	 * Key value pairs in original order, as kv_t (pointer to obj in kv).
	 */
	array_t *kv_order;
};

/**
 * Create a key/value pair.
 *
 * @param key		key (gets adopted)
 * @param value		value (gets adopted)
 * @return			allocated key/value pair
 */
kv_t *settings_kv_create(char *key, char *value);

/**
 * Destroy a key/value pair.
 *
 * @param this		key/value pair to destroy
 * @param contents	optional array to store the value in
 */
void settings_kv_destroy(kv_t *this, array_t *contents);

/**
 * Set the value of the given key/value pair.
 *
 * @param kv		key/value pair
 * @param value		new value (gets adopted), may be NULL
 * @param contents	optional array to store replaced values in
 */
void settings_kv_set(kv_t *kv, char *value, array_t *contents);

/**
 * Add the given key/value pair to the given section.
 *
 * @param section	section to add pair to
 * @param kv		key/value pair to add (gets adopted)
 * @param contents	optional array to store replaced values in
 */
void settings_kv_add(section_t *section, kv_t *kv, array_t *contents);

/**
 * Add a reference to another section.
 *
 * @param section	section to which to add the reference
 * @param name		name of the referenced section (adopted)
 * @param permanent	whether the reference is not removed during reloads
 */
void settings_reference_add(section_t *section, char *name, bool permanent);

/**
 * Create a section with the given name.
 *
 * @param name		name (gets adopted)
 * @return			allocated section
 */
section_t *settings_section_create(char *name);

/**
 * Destroy a section.
 *
 * @param this		section to destroy
 * @param contents	optional array to store values of removed key/value pairs
 */
void settings_section_destroy(section_t *this, array_t *contents);

/**
 * Add the given section to the given parent section.
 *
 * @param parent	section to add section to
 * @param section	section to add (gets adopted)
 * @param contents	optional array to store replaced values in
 */
void settings_section_add(section_t *parent, section_t *section,
						  array_t *contents);

/**
 * Extend the first section with the values and sub-sections of the second
 * section, from where they are consequently removed.
 *
 * @param base		base section to extend
 * @param extension	section whose data is extracted
 * @param contents	optional array to store replaced values in
 * @param purge		TRUE to remove settings and sections not found in the
 *					extension (unless (sub-)sections have/are fallbacks)
 */
void settings_section_extend(section_t *base, section_t *extension,
							 array_t *contents, bool purge);

/**
 * Callback to find a section by name
 */
int settings_section_find(const void *a, const void *b);

/**
 * Callback to sort sections by name
 */
int settings_section_sort(const void *a, const void *b, void *user);

/**
 * Callback to find a key/value pair by key
 */
int settings_kv_find(const void *a, const void *b);

/**
 * Callback to sort kv pairs by key
 */
int settings_kv_sort(const void *a, const void *b, void *user);

#endif /** SETTINGS_TYPES_H_ @}*/
