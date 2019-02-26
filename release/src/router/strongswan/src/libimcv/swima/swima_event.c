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

#include "swima_event.h"
#include "swima_data_model.h"

typedef struct private_swima_event_t private_swima_event_t;

/**
 * Private data of a swima_event_t object.
 *
 */
struct private_swima_event_t {

	/**
	 * Public swima_event_t interface.
	 */
	swima_event_t public;

	/**
	 * Event ID
	 */
	uint32_t eid;

	/**
	 * Timestamp
	 */
	chunk_t timestamp;

	/**
	 * Action
	 */
	uint8_t action;

	/**
	 * Software [Identifier] record
	 */
	swima_record_t *sw_record;

	/**
	 * Reference count
	 */
	refcount_t ref;
};

METHOD(swima_event_t, get_eid, uint32_t,
	private_swima_event_t *this, chunk_t *timestamp)
{
	if (timestamp)
	{
		*timestamp = this->timestamp;
	}
	return this->eid;
}

METHOD(swima_event_t, get_action, uint8_t,
	private_swima_event_t *this)
{
	return this->action;
}

METHOD(swima_event_t, get_sw_record, swima_record_t*,
	private_swima_event_t *this)
{
	return this->sw_record;
}


METHOD(swima_event_t, get_ref, swima_event_t*,
	private_swima_event_t *this)
{
	ref_get(&this->ref);
	return &this->public;
}

METHOD(swima_event_t, destroy, void,
	private_swima_event_t *this)
{
	if (ref_put(&this->ref))
	{
		this->sw_record->destroy(this->sw_record);
		free(this->timestamp.ptr);
		free(this);
	}
}

/**
 * See header
 */
swima_event_t *swima_event_create(uint32_t eid, chunk_t timestamp,
								  uint8_t action, swima_record_t *sw_record)
{
	private_swima_event_t *this;

	INIT(this,
		.public = {
			.get_eid = _get_eid,
			.get_action = _get_action,
			.get_sw_record = _get_sw_record,
			.get_ref = _get_ref,
			.destroy = _destroy,
		},
		.eid = eid,
		.timestamp = chunk_clone(timestamp),
		.action = action,
		.sw_record = sw_record,
		.ref = 1,
	);

	return &this->public;
}

