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

#include "swima_record.h"
#include "swima_data_model.h"

typedef struct private_swima_record_t private_swima_record_t;

/**
 * Private data of a swima_record_t object.
 *
 */
struct private_swima_record_t {

	/**
	 * Public swima_record_t interface.
	 */
	swima_record_t public;

	/**
	 * Record ID
	 */
	uint32_t record_id;

	/**
	 * Software Identity
	 */
	chunk_t sw_id;

	/**
	 * Optional Software Locator
	 */
	chunk_t sw_locator;

	/**
	 * Data Model
	 */
	pen_type_t data_model;

	/**
	 * Source ID
	 */
	uint8_t source_id;

	/**g
	 * Optional Software Inventory Evidence Record
	 */
	chunk_t record;

	/**
	 * Reference count
	 */
	refcount_t ref;
};

METHOD(swima_record_t, get_record_id, uint32_t,
	private_swima_record_t *this)
{
	return this->record_id;
}

METHOD(swima_record_t, get_sw_id, chunk_t,
	private_swima_record_t *this, chunk_t *sw_locator)
{
	if (sw_locator)
	{
		*sw_locator = this->sw_locator;
	}
	return this->sw_id;
}

METHOD(swima_record_t, set_data_model, void,
	private_swima_record_t *this, pen_type_t data_model)
{
	this->data_model = data_model;
}

METHOD(swima_record_t, get_data_model, pen_type_t,
	private_swima_record_t *this)
{
	return this->data_model;
}

METHOD(swima_record_t, set_source_id, void,
	private_swima_record_t *this, uint8_t source_id)
{
	this->source_id = source_id;
}

METHOD(swima_record_t, get_source_id, uint8_t,
	private_swima_record_t *this)
{
	return this->source_id;
}

METHOD(swima_record_t, set_record, void,
	private_swima_record_t *this, chunk_t record)
{
	chunk_free(&this->record);
	this->record = chunk_clone(record);
}

METHOD(swima_record_t, get_record, chunk_t,
	private_swima_record_t *this)
{
	return this->record;
}

METHOD(swima_record_t, get_ref, swima_record_t*,
	private_swima_record_t *this)
{
	ref_get(&this->ref);
	return &this->public;
}

METHOD(swima_record_t, destroy, void,
	private_swima_record_t *this)
{
	if (ref_put(&this->ref))
	{
		free(this->sw_id.ptr);
		free(this->sw_locator.ptr);
		free(this->record.ptr);
		free(this);
	}
}

/**
 * See header
 */
swima_record_t *swima_record_create(uint32_t record_id, chunk_t sw_id,
									chunk_t sw_locator)
{
	private_swima_record_t *this;

	INIT(this,
		.public = {
			.get_record_id = _get_record_id,
			.get_sw_id = _get_sw_id,
			.set_data_model = _set_data_model,
			.get_data_model = _get_data_model,
			.set_source_id = _set_source_id,
			.get_source_id = _get_source_id,
			.set_record = _set_record,
			.get_record = _get_record,
			.get_ref = _get_ref,
			.destroy = _destroy,
		},
		.record_id = record_id,
		.data_model = swima_data_model_iso_2015_swid_xml,
		.sw_id = chunk_clone(sw_id),
		.ref = 1,
	);

	if (sw_locator.len > 0)
	{
		this->sw_locator = chunk_clone(sw_locator);
	}

	return &this->public;
}

