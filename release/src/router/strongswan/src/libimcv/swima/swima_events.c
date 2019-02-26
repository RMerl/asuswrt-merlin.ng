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

#include "swima_events.h"
#include "swima_record.h"

#include <collections/linked_list.h>
#include <utils/debug.h>

typedef struct private_swima_events_t private_swima_events_t;

/**
 * Private data of a swima_events_t object.
 *
 */
struct private_swima_events_t {

	/**
	 * Public swima_events_t interface.
	 */
	swima_events_t public;

	/**
	 * Epoch of Event IDs
	 */
	uint32_t epoch;

	/**
	 * Last Event ID
	 */
	uint32_t last_eid;

	/**
	 * Last Consulted Event ID
	 */
	uint32_t last_consulted_eid;

	/**
	 * List of SW records
	 */
	linked_list_t *list;

	/**
	 * Reference count
	 */
	refcount_t ref;

};

METHOD(swima_events_t, add, void,
	private_swima_events_t *this, swima_event_t *event)
{
	this->list->insert_last(this->list, event);
}

METHOD(swima_events_t, get_count, int,
	private_swima_events_t *this)
{
	return this->list->get_count(this->list);
}

METHOD(swima_events_t, set_eid, void,
	private_swima_events_t *this, uint32_t eid, uint32_t epoch)
{
	this->last_eid = this->last_consulted_eid = eid;
	this->epoch = epoch;
}

METHOD(swima_events_t, set_last_eid, void,
	private_swima_events_t *this, uint32_t last_eid)
{
	this->last_eid = last_eid;
}

METHOD(swima_events_t, get_eid, uint32_t,
	private_swima_events_t *this, uint32_t *epoch, uint32_t *last_eid)
{
	if (epoch)
	{
		*epoch = this->epoch;
	}
	if (last_eid)
	{
		*last_eid = this->last_eid;
	}
	return this->last_consulted_eid;
}

METHOD(swima_events_t, create_enumerator, enumerator_t*,
	private_swima_events_t *this)
{
	return this->list->create_enumerator(this->list);
}

METHOD(swima_events_t, get_ref, swima_events_t*,
	private_swima_events_t *this)
{
	ref_get(&this->ref);
	return &this->public;
}

METHOD(swima_events_t, clear, void,
	private_swima_events_t *this)
{
	this->list->destroy_offset(this->list, offsetof(swima_event_t, destroy));
	this->list = linked_list_create();
}

METHOD(swima_events_t, destroy, void,
	private_swima_events_t *this)
{
	if (ref_put(&this->ref))
	{
		this->list->destroy_offset(this->list, offsetof(swima_event_t, destroy));
		free(this);
	}
}

/**
 * See header
 */
swima_events_t *swima_events_create(void)
{
	private_swima_events_t *this;

	INIT(this,
		.public = {
			.add = _add,
			.get_count = _get_count,
			.set_eid = _set_eid,
			.set_last_eid = _set_last_eid,
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
