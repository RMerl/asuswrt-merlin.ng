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

#include "settings_types.h"

/*
 * Described in header
 */
kv_t *settings_kv_create(char *key, char *value)
{
	kv_t *this;

	INIT(this,
		.key = key,
		.value = value,
	);
	return this;
}

/*
 * Described in header
 */
void settings_kv_destroy(kv_t *this, array_t *contents)
{
	free(this->key);
	if (contents && this->value)
	{
		array_insert(contents, ARRAY_TAIL, this->value);
	}
	else
	{
		free(this->value);
	}
	free(this);
}

/*
 * Described in header
 */
section_t *settings_section_create(char *name)
{
	section_t *this;

	INIT(this,
		.name = name,
	);
	return this;
}

static void section_destroy(section_t *section, int idx, array_t *contents)
{
	settings_section_destroy(section, contents);
}

static void kv_destroy(kv_t *kv, int idx, array_t *contents)
{
	settings_kv_destroy(kv, contents);
}

static void ref_destroy(section_ref_t *ref, int idx, void *ctx)
{
	free(ref->name);
	free(ref);
}

/*
 * Described in header
 */
void settings_section_destroy(section_t *this, array_t *contents)
{
	array_destroy_function(this->sections, (void*)section_destroy, contents);
	array_destroy(this->sections_order);
	array_destroy_function(this->kv, (void*)kv_destroy, contents);
	array_destroy(this->kv_order);
	array_destroy_function(this->references, (void*)ref_destroy, NULL);
	free(this->name);
	free(this);
}

/*
 * Described in header
 */
void settings_kv_set(kv_t *kv, char *value, array_t *contents)
{
	if (value && kv->value && streq(value, kv->value))
	{	/* no update required */
		free(value);
		return;
	}

	/* if the new value was shorter we could overwrite the existing one but that
	 * could lead to reads of partially updated values from other threads that
	 * have a pointer to the existing value, so we replace it anyway */
	if (kv->value && contents)
	{
		array_insert(contents, ARRAY_TAIL, kv->value);
	}
	else
	{
		free(kv->value);
	}
	kv->value = value;
}

/*
 * Described in header
 */
void settings_kv_add(section_t *section, kv_t *kv, array_t *contents)
{
	kv_t *found;

	if (array_bsearch(section->kv, kv->key, settings_kv_find, &found) == -1)
	{
		array_insert_create(&section->kv, ARRAY_TAIL, kv);
		array_sort(section->kv, settings_kv_sort, NULL);
		array_insert_create(&section->kv_order, ARRAY_TAIL, kv);
	}
	else
	{
		settings_kv_set(found, kv->value, contents);
		kv->value = NULL;
		settings_kv_destroy(kv, NULL);
	}
}

/*
 * Described in header
 */
void settings_reference_add(section_t *section, char *name, bool permanent)
{
	section_ref_t *ref;
	int i;

	for (i = 0; i < array_count(section->references); i++)
	{
		array_get(section->references, i, &ref);
		if (ref->permanent && !permanent)
		{	/* add it before any permanent references */
			break;
		}
		if (ref->permanent == permanent && streq(name, ref->name))
		{
			free(name);
			return;
		}
	}

	INIT(ref,
		.name = name,
		.permanent = permanent,
	);
	array_insert_create(&section->references, i, ref);
}

/*
 * Add a section to the given parent, optionally remove settings/subsections
 * not found when extending an existing section
 */
static void add_section(section_t *parent, section_t *section,
						array_t *contents, bool purge)
{
	section_t *found;

	if (array_bsearch(parent->sections, section->name, settings_section_find,
					  &found) == -1)
	{
		array_insert_create(&parent->sections, ARRAY_TAIL, section);
		array_sort(parent->sections, settings_section_sort, NULL);
		array_insert_create(&parent->sections_order, ARRAY_TAIL, section);
	}
	else
	{
		settings_section_extend(found, section, contents, purge);
		settings_section_destroy(section, contents);
	}
}

/*
 * Described in header
 */
void settings_section_add(section_t *parent, section_t *section,
						  array_t *contents)
{
	add_section(parent, section, contents, FALSE);
}

/**
 * Purge contents of a section, returns TRUE if section can be safely removed.
 */
static bool section_purge(section_t *this, array_t *contents)
{
	section_t *current;
	section_ref_t *ref;
	int i, idx;

	array_destroy_function(this->kv, (void*)kv_destroy, contents);
	this->kv = NULL;
	array_destroy(this->kv_order);
	this->kv_order = NULL;
	/* remove non-permanent references */
	for (i = array_count(this->references) - 1; i >= 0; i--)
	{
		array_get(this->references, i, &ref);
		if (!ref->permanent)
		{
			array_remove(this->references, i, NULL);
			ref_destroy(ref, 0, NULL);
		}
	}
	if (!array_count(this->references))
	{
		array_destroy(this->references);
		this->references = NULL;
	}
	for (i = array_count(this->sections_order) - 1; i >= 0; i--)
	{
		array_get(this->sections_order, i, &current);
		if (section_purge(current, contents))
		{
			array_remove(this->sections_order, i, NULL);
			idx = array_bsearch(this->sections, current->name,
								settings_section_find, NULL);
			array_remove(this->sections, idx, NULL);
			settings_section_destroy(current, contents);
		}
	}
	/* we ensure sections configured with permanent references (or having any
	 * such subsections) are not removed */
	return !this->references && !array_count(this->sections);
}

/*
 * Described in header
 */
void settings_section_extend(section_t *base, section_t *extension,
							 array_t *contents, bool purge)
{
	enumerator_t *enumerator;
	section_t *section;
	section_ref_t *ref;
	kv_t *kv;
	array_t *sections = NULL, *kvs = NULL;
	int idx;

	if (purge)
	{	/* remove sections, settings in base not found in extension, the others
		 * are removed too (from the _order list) so they can be inserted in the
		 * order found in extension, non-permanent references are removed */
		enumerator = array_create_enumerator(base->sections_order);
		while (enumerator->enumerate(enumerator, (void**)&section))
		{
			if (array_bsearch(extension->sections, section->name,
							  settings_section_find, NULL) == -1)
			{
				idx = array_bsearch(base->sections, section->name,
									settings_section_find, NULL);
				if (section_purge(section, contents))
				{	/* only remove them if we can purge them */
					array_remove(base->sections, idx, NULL);
					array_remove_at(base->sections_order, enumerator);
					settings_section_destroy(section, contents);
				}
			}
			else
			{
				array_remove_at(base->sections_order, enumerator);
				array_insert_create(&sections, ARRAY_TAIL, section);
				array_sort(sections, settings_section_sort, NULL);
			}
		}
		enumerator->destroy(enumerator);

		while (array_remove(base->kv_order, 0, &kv))
		{
			if (array_bsearch(extension->kv, kv->key, settings_kv_find,
							  NULL) == -1)
			{
				idx = array_bsearch(base->kv, kv->key, settings_kv_find, NULL);
				array_remove(base->kv, idx, NULL);
				settings_kv_destroy(kv, contents);
			}
			else
			{
				array_insert_create(&kvs, ARRAY_TAIL, kv);
				array_sort(kvs, settings_kv_sort, NULL);
			}
		}

		enumerator = array_create_enumerator(base->references);
		while (enumerator->enumerate(enumerator, (void**)&ref))
		{
			if (ref->permanent)
			{	/* permanent references are ignored */
				continue;
			}
			array_remove_at(base->references, enumerator);
			ref_destroy(ref, 0, NULL);
		}
		enumerator->destroy(enumerator);
	}

	while (array_remove(extension->sections_order, 0, &section))
	{
		idx = array_bsearch(sections, section->name,
							settings_section_find, NULL);
		if (idx != -1)
		{
			section_t *existing;

			array_remove(sections, idx, &existing);
			array_insert(base->sections_order, ARRAY_TAIL, existing);
		}
		idx = array_bsearch(extension->sections, section->name,
							settings_section_find, NULL);
		array_remove(extension->sections, idx, NULL);
		add_section(base, section, contents, purge);
	}

	while (array_remove(extension->kv_order, 0, &kv))
	{
		idx = array_bsearch(kvs, kv->key, settings_kv_find, NULL);
		if (idx != -1)
		{
			kv_t *existing;

			array_remove(kvs, idx, &existing);
			array_insert(base->kv_order, ARRAY_TAIL, existing);
		}
		idx = array_bsearch(extension->kv, kv->key, settings_kv_find, NULL);
		array_remove(extension->kv, idx, NULL);
		settings_kv_add(base, kv, contents);
	}

	while (array_remove(extension->references, 0, &ref))
	{
		if (ref->permanent)
		{	/* ignore permanent references in the extension */
			continue;
		}
		settings_reference_add(base, strdup(ref->name), FALSE);
		ref_destroy(ref, 0, NULL);
	}
	array_destroy(sections);
	array_destroy(kvs);
}

/*
 * Described in header
 */
int settings_section_find(const void *a, const void *b)
{
	const char *key = a;
	const section_t *item = b;
	return strcmp(key, item->name);
}

/*
 * Described in header
 */
int settings_section_sort(const void *a, const void *b, void *user)
{
	const section_t *sa = a, *sb = b;
	return strcmp(sa->name, sb->name);
}

/*
 * Described in header
 */
int settings_kv_find(const void *a, const void *b)
{
	const char *key = a;
	const kv_t *item = b;
	return strcmp(key, item->key);
}

/*
 * Described in header
 */
int settings_kv_sort(const void *a, const void *b, void *user)
{
	const kv_t *kva = a, *kvb = b;
	return strcmp(kva->key, kvb->key);
}
