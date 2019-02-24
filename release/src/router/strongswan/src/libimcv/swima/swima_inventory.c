/*
 * Copyright (C) 2017 Andreas Steffen
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

#include "swima_inventory.h"
#include "swima_record.h"

#include <collections/linked_list.h>
#include <utils/debug.h>

typedef struct private_swima_inventory_t private_swima_inventory_t;

/**
 * Private data of a swima_inventory_t object.
 *
 */
struct private_swima_inventory_t {

	/**
	 * Public swima_inventory_t interface.
	 */
	swima_inventory_t public;

	/**
	 * Earliest or last event ID of the inventory
	 */
	uint32_t eid;

	/**
	 * Epoch of event IDs
	 */
	uint32_t epoch;

	/**
	 * List of SW records
	 */
	linked_list_t *list;


	/**
	 * Reference count
	 */
	refcount_t ref;

};

METHOD(swima_inventory_t, add, void,
	private_swima_inventory_t *this, swima_record_t *record)
{
	this->list->insert_last(this->list, record);
}

METHOD(swima_inventory_t, get_count, int,
	private_swima_inventory_t *this)
{
	return this->list->get_count(this->list);
}

METHOD(swima_inventory_t, set_eid, void,
	private_swima_inventory_t *this, uint32_t eid, uint32_t epoch)
{
	this->eid = eid;
	this->epoch = epoch;
}

METHOD(swima_inventory_t, get_eid, uint32_t,
	private_swima_inventory_t *this, uint32_t *epoch)
{
	if (epoch)
	{
		*epoch = this->epoch;
	}
	return this->eid;
}

METHOD(swima_inventory_t, create_enumerator, enumerator_t*,
	private_swima_inventory_t *this)
{
	return this->list->create_enumerator(this->list);
}

METHOD(swima_inventory_t, get_ref, swima_inventory_t*,
	private_swima_inventory_t *this)
{
	ref_get(&this->ref);
	return &this->public;
}

METHOD(swima_inventory_t, clear, void,
	private_swima_inventory_t *this)
{
		this->list->destroy_offset(this->list, offsetof(swima_record_t, destroy));
		this->list = linked_list_create();
}

METHOD(swima_inventory_t, destroy, void,
	private_swima_inventory_t *this)
{
	if (ref_put(&this->ref))
	{
		this->list->destroy_offset(this->list, offsetof(swima_record_t, destroy));
		free(this);
	}
}

/**
 * See header
 */
swima_inventory_t *swima_inventory_create(void)
{
	private_swima_inventory_t *this;

	INIT(this,
		.public = {
			.add = _add,
			.get_count = _get_count,
			.set_eid = _set_eid,
			.get_eid = _get_eid,
			.create_enumerator = _create_enumerator,
			.get_ref = _get_ref,
			.clear = _clear,
			.destroy = _destroy,
		},
		.list = linked_list_create(),
		.ref = 1,
	);

	return &this->public;
}
