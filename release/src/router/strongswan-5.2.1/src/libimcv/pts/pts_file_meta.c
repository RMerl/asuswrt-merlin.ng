/*
 * Copyright (C) 2011 Sansar Choinyambuu
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

#include "pts_file_meta.h"

#include <collections/linked_list.h>
#include <utils/debug.h>

typedef struct private_pts_file_meta_t private_pts_file_meta_t;

/**
 * Private data of a pts_file_meta_t object.
 *
 */
struct private_pts_file_meta_t {

	/**
	 * Public pts_file_meta_t interface.
	 */
	pts_file_meta_t public;

	/**
	 * List of File Metadata
	 */
	linked_list_t *list;
};

/**
 * Free an pts_file_metadata_t object
 */
static void free_entry(pts_file_metadata_t *entry)
{
	if (entry)
	{
		free(entry->filename);
		free(entry);
	}
}

METHOD(pts_file_meta_t, get_file_count, int,
	private_pts_file_meta_t *this)
{
	return this->list->get_count(this->list);
}

METHOD(pts_file_meta_t, add, void,
	private_pts_file_meta_t *this, pts_file_metadata_t *metadata)
{
	this->list->insert_last(this->list, metadata);
}

METHOD(pts_file_meta_t, create_enumerator, enumerator_t*,
	private_pts_file_meta_t *this)
{
	return this->list->create_enumerator(this->list);
}

METHOD(pts_file_meta_t, destroy, void,
	private_pts_file_meta_t *this)
{
	this->list->destroy_function(this->list, (void *)free_entry);
	free(this);
}

/**
 * See header
 */
pts_file_meta_t *pts_file_meta_create()
{
	private_pts_file_meta_t *this;

	INIT(this,
		.public = {
			.get_file_count = _get_file_count,
			.add = _add,
			.create_enumerator = _create_enumerator,
			.destroy = _destroy,
		},
		.list = linked_list_create(),
	);

	return &this->public;
}

