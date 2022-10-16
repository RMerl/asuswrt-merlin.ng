/*
 * Copyright (C) 2020 Andreas Steffen
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

#include "pts_symlinks.h"

#include <collections/linked_list.h>
#include <utils/debug.h>

typedef struct private_pts_symlinks_t private_pts_symlinks_t;
typedef struct entry_t entry_t;

/**
 * Private data of a pts_symlinks_t object.
 *
 */
struct private_pts_symlinks_t {

	/**
	 * Public pts_symlinks_t interface.
	 */
	pts_symlinks_t public;

	/**
	 * List of symbolic links pointing to directories
	 */
	linked_list_t *list;

	/**
	 * Reference count
	 */
	refcount_t ref;

};

/**
 * Symlink entry
 */
struct entry_t {
	chunk_t symlink;
	chunk_t dir;
};

/**
 * Free an entry_t object
 */
static void free_entry(entry_t *entry)
{
	if (entry)
	{
		free(entry->symlink.ptr);
		free(entry->dir.ptr);
		free(entry);
	}
}

METHOD(pts_symlinks_t, get_count, int,
	private_pts_symlinks_t *this)
{
	return this->list->get_count(this->list);
}

METHOD(pts_symlinks_t, add, void,
	private_pts_symlinks_t *this, chunk_t symlink, chunk_t dir)
{
	entry_t *entry;

	entry = malloc_thing(entry_t);
	entry->symlink = chunk_clone(symlink);
	entry->dir = chunk_clone(dir);

	this->list->insert_last(this->list, entry);
}

CALLBACK(symlink_filter, bool,
	void *null, enumerator_t *orig, va_list args)
{
	entry_t *entry;
	chunk_t *symlink;
	chunk_t *dir;

	VA_ARGS_VGET(args, symlink, dir);

	if (orig->enumerate(orig, &entry))
	{
		*symlink = entry->symlink;
		*dir     = entry->dir;
		return TRUE;
	}
	return FALSE;
}

METHOD(pts_symlinks_t, create_enumerator, enumerator_t*,
	private_pts_symlinks_t *this)
{
	return enumerator_create_filter(this->list->create_enumerator(this->list),
									symlink_filter, NULL, NULL);
}

METHOD(pts_symlinks_t, get_ref, pts_symlinks_t*,
	private_pts_symlinks_t *this)
{
	ref_get(&this->ref);
	return &this->public;
}

METHOD(pts_symlinks_t, destroy, void,
	private_pts_symlinks_t *this)
{
	if (ref_put(&this->ref))
	{
		this->list->destroy_function(this->list, (void *)free_entry);
		free(this);
	}
}

/**
 * See header
 */
pts_symlinks_t *pts_symlinks_create()
{
	private_pts_symlinks_t *this;

	INIT(this,
		.public = {
			.get_count = _get_count,
			.add = _add,
			.create_enumerator = _create_enumerator,
			.get_ref = _get_ref,
			.destroy = _destroy,
		},
		.list = linked_list_create(),
		.ref = 1,
	);

	return &this->public;
}

