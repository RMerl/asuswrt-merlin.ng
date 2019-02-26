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

#include "ietf_swima_attr_sw_ev.h"
#include "swima/swima_event.h"

#include <pa_tnc/pa_tnc_msg.h>
#include <bio/bio_writer.h>
#include <bio/bio_reader.h>
#include <utils/debug.h>

#define SW_EV_TIMESTAMP_SIZE	20

typedef struct private_ietf_swima_attr_sw_ev_t private_ietf_swima_attr_sw_ev_t;

/**
 * Software [Identifier] Events
 * see sections 5.9/5.11 of RFC 8412 SWIMA
 *
 *                       1                   2                   3
 *   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |     Flags     |           Software Identifier Count           |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |               Request ID Copy / Subscription ID               |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                           EID Epoch                           |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                           Last EID                            |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                       Last Consulted EID                      |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                              EID                              |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                           Timestamp                           |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                           Timestamp                           |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                           Timestamp                           |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                           Timestamp                           |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                           Timestamp                           |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                       Record Identifier                       |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |              Data Model Type PEN              |Data Model Type|
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  | Source ID Num |    Action     |  Software Identifier Length   |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |             Software Identifier (Variable Length)             |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |    Software Locator Length    |  Software Locator (Var. Len)  |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * Software Event only
 * see section 5.11 of IETF SW Inventory Message and Attributes for PA-TNC
 *
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                          Record Length                        |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                   Record (Variable length)                    |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

/**
 * Private data of an ietf_swima_attr_sw_ev_t object.
 */
struct private_ietf_swima_attr_sw_ev_t {

	/**
	 * Public members of ietf_swima_attr_sw_ev_t
	 */
	ietf_swima_attr_sw_ev_t public;

	/**
	 * Vendor-specific attribute type
	 */
	pen_type_t type;

	/**
	 * Length of attribute value
	 */
	size_t length;

	/**
	 * Offset up to which attribute value has been processed
	 */
	size_t offset;

	/**
	 * Current position of attribute value pointer
	 */
	chunk_t value;

	/**
	 * Contains complete attribute or current segment
	 */
	chunk_t segment;

	/**
	 * Noskip flag
	 */
	bool noskip_flag;

	/**
	 * Request ID
	 */
	uint32_t request_id;

	/**
	 * Attribute flags
	 */
	uint8_t flags;

	/**
	 * Number of unprocessed software events in attribute
	 */
	uint32_t event_count;

	/**
	 * Event list
	 */
	swima_events_t *events;

	/**
	 * Reference count
	 */
	refcount_t ref;
};

METHOD(pa_tnc_attr_t, get_type, pen_type_t,
	private_ietf_swima_attr_sw_ev_t *this)
{
	return this->type;
}

METHOD(pa_tnc_attr_t, get_value, chunk_t,
	private_ietf_swima_attr_sw_ev_t *this)
{
	return this->value;
}

METHOD(pa_tnc_attr_t, get_noskip_flag, bool,
	private_ietf_swima_attr_sw_ev_t *this)
{
	return this->noskip_flag;
}

METHOD(pa_tnc_attr_t, set_noskip_flag,void,
	private_ietf_swima_attr_sw_ev_t *this, bool noskip)
{
	this->noskip_flag = noskip;
}

/**
 * This function is shared with ietf_swima_attr_sw_inv.c
 **/
void ietf_swima_attr_sw_ev_build_sw_record(bio_writer_t *writer,
		uint8_t action, swima_record_t *sw_record, bool has_record)
{
	pen_type_t data_model;
	chunk_t sw_locator;

	data_model = sw_record->get_data_model(sw_record);

	writer->write_uint32(writer, sw_record->get_record_id(sw_record));
	writer->write_uint24(writer, data_model.vendor_id);
	writer->write_uint8 (writer, data_model.type);
	writer->write_uint8 (writer, sw_record->get_source_id(sw_record));
	writer->write_uint8 (writer, action);
	writer->write_data16(writer, sw_record->get_sw_id(sw_record, &sw_locator));
	writer->write_data16(writer, sw_locator);

	if (has_record)
	{
		writer->write_data32(writer, sw_record->get_record(sw_record));
	}
}

METHOD(pa_tnc_attr_t, build, void,
	private_ietf_swima_attr_sw_ev_t *this)
{
	bio_writer_t *writer;
	swima_event_t *sw_event;
	swima_record_t *sw_record;
	chunk_t timestamp;
	uint32_t last_eid, last_consulted_eid, eid_epoch;
	uint8_t action;
	enumerator_t *enumerator;

	if (this->value.ptr)
	{
		return;
	}
	last_consulted_eid = this->events->get_eid(this->events, &eid_epoch,
															 &last_eid);

	writer = bio_writer_create(IETF_SWIMA_SW_EV_MIN_SIZE);
	writer->write_uint8 (writer, this->flags);
	writer->write_uint24(writer, this->events->get_count(this->events));
	writer->write_uint32(writer, this->request_id);
	writer->write_uint32(writer, eid_epoch);
	writer->write_uint32(writer, last_eid);
	writer->write_uint32(writer, last_consulted_eid);

	enumerator = this->events->create_enumerator(this->events);
	while (enumerator->enumerate(enumerator, &sw_event))
	{
		action     = sw_event->get_action(sw_event);
		sw_record  = sw_event->get_sw_record(sw_event);

		writer->write_uint32(writer, sw_event->get_eid(sw_event, &timestamp));
		writer->write_data  (writer, timestamp);

		ietf_swima_attr_sw_ev_build_sw_record(writer, action, sw_record,
								this->type.type == IETF_ATTR_SW_EVENTS);
	}
	enumerator->destroy(enumerator);

	this->value = writer->extract_buf(writer);
	this->segment = this->value;
	this->length = this->value.len;
	writer->destroy(writer);
}

/**
 * This function is shared with ietf_swima_attr_sw_inv.c
 **/
bool ietf_swima_attr_sw_ev_process_sw_record(bio_reader_t *reader,
		uint8_t *action, swima_record_t **sw_record, bool has_record)
{
	pen_type_t data_model;
	swima_record_t *sw_rec;
	uint32_t data_model_pen, record_id;
	uint8_t  data_model_type, source_id, reserved;
	chunk_t sw_id, sw_locator, record = chunk_empty;

	if (!reader->read_uint32(reader, &record_id) ||
		!reader->read_uint24(reader, &data_model_pen) ||
		!reader->read_uint8 (reader, &data_model_type) ||
		!reader->read_uint8 (reader, &source_id) ||
		!reader->read_uint8 (reader, &reserved) ||
		!reader->read_data16(reader, &sw_id) ||
		!reader->read_data16(reader, &sw_locator))
	{
		return FALSE;
	}

	if (action)
	{
		*action = reserved;
	}

	if (has_record && !reader->read_data32(reader, &record))
	{
		return FALSE;
	}

	data_model = pen_type_create(data_model_pen, data_model_type);
	sw_rec = swima_record_create(record_id, sw_id, sw_locator);
	sw_rec->set_data_model(sw_rec, data_model);
	sw_rec->set_source_id(sw_rec, source_id);
	sw_rec->set_record(sw_rec, record);
	*sw_record = sw_rec;

	return TRUE;
}

METHOD(pa_tnc_attr_t, process, status_t,
	private_ietf_swima_attr_sw_ev_t *this, uint32_t *offset)
{
	bio_reader_t *reader;
	uint32_t eid, eid_epoch, last_eid, last_consulted_eid;
	uint8_t  action;
	chunk_t timestamp;
	swima_event_t *sw_event;
	swima_record_t *sw_record;
	status_t status = NEED_MORE;

	if (this->offset == 0)
	{
		if (this->length < IETF_SWIMA_SW_EV_MIN_SIZE)
		{
			DBG1(DBG_TNC, "insufficient data for %N/%N", pen_names, PEN_IETF,
						   ietf_attr_names, this->type.type);
			*offset = this->offset;
			return FAILED;
		}
		if (this->value.len < IETF_SWIMA_SW_EV_MIN_SIZE)
		{
			return NEED_MORE;
		}
		reader = bio_reader_create(this->value);
		reader->read_uint8 (reader, &this->flags);
		reader->read_uint24(reader, &this->event_count);
		reader->read_uint32(reader, &this->request_id);
		reader->read_uint32(reader, &eid_epoch);
		reader->read_uint32(reader, &last_eid);
		reader->read_uint32(reader, &last_consulted_eid);
		this->offset = IETF_SWIMA_SW_EV_MIN_SIZE;
		this->events->set_eid(this->events, last_consulted_eid, eid_epoch);
		this->events->set_last_eid(this->events, last_eid);
		this->value = reader->peek(reader);
		reader->destroy(reader);
	}

	reader = bio_reader_create(this->value);

	while (this->event_count)
	{
		if (!reader->read_uint32(reader, &eid) ||
			!reader->read_data  (reader, SW_EV_TIMESTAMP_SIZE, &timestamp) ||
			!ietf_swima_attr_sw_ev_process_sw_record(reader, &action, &sw_record,
								this->type.type == IETF_ATTR_SW_EVENTS))
		{
			goto end;
		}

		if (action == SWIMA_EVENT_ACTION_NONE ||
			action  > SWIMA_EVENT_ACTION_LAST)
		{
			DBG1(DBG_TNC, "invalid event action value for %N/%N", pen_names,
						   PEN_IETF, ietf_attr_names, this->type.type);
			*offset = this->offset;
			sw_record->destroy(sw_record);
			reader->destroy(reader);

			return FAILED;
		}

		sw_event = swima_event_create(eid, timestamp, action, sw_record);
		this->events->add(this->events, sw_event);
		this->offset += this->value.len - reader->remaining(reader);
		this->value = reader->peek(reader);

		/* at least one software event was processed */
		status = SUCCESS;
		this->event_count--;
	}

	if (this->length == this->offset)
	{
		status = SUCCESS;
	}
	else
	{
		DBG1(DBG_TNC, "inconsistent length for %N/%N", pen_names, PEN_IETF,
					   ietf_attr_names, this->type.type);
		*offset = this->offset;
		status = FAILED;
	}

end:
	reader->destroy(reader);
	return status;
}

METHOD(pa_tnc_attr_t, add_segment, void,
	private_ietf_swima_attr_sw_ev_t *this, chunk_t segment)
{
	this->value = chunk_cat("cc", this->value, segment);
	chunk_free(&this->segment);
	this->segment = this->value;
}

METHOD(pa_tnc_attr_t, get_ref, pa_tnc_attr_t*,
	private_ietf_swima_attr_sw_ev_t *this)
{
	ref_get(&this->ref);
	return &this->public.pa_tnc_attribute;
}

METHOD(pa_tnc_attr_t, destroy, void,
	private_ietf_swima_attr_sw_ev_t *this)
{
	if (ref_put(&this->ref))
	{
		this->events->destroy(this->events);
		free(this->segment.ptr);
		free(this);
	}
}

METHOD(ietf_swima_attr_sw_ev_t, get_flags, uint8_t,
	private_ietf_swima_attr_sw_ev_t *this)
{
	return this->flags;
}

METHOD(ietf_swima_attr_sw_ev_t, get_request_id, uint32_t,
	private_ietf_swima_attr_sw_ev_t *this)
{
	return this->request_id;
}

METHOD(ietf_swima_attr_sw_ev_t, get_event_count, uint32_t,
	private_ietf_swima_attr_sw_ev_t *this)
{
	return this->event_count;
}

METHOD(ietf_swima_attr_sw_ev_t, set_events, void,
	private_ietf_swima_attr_sw_ev_t *this, swima_events_t *events)
{
	this->events->destroy(this->events);
	this->events = events->get_ref(events);
}

METHOD(ietf_swima_attr_sw_ev_t, get_events, swima_events_t*,
	private_ietf_swima_attr_sw_ev_t *this)
{
	return this->events;
}

METHOD(ietf_swima_attr_sw_ev_t, clear_events, void,
	private_ietf_swima_attr_sw_ev_t *this)
{
	this->events->clear(this->events);
}

/**
 * Described in header.
 */
pa_tnc_attr_t *ietf_swima_attr_sw_ev_create(uint8_t flags, uint32_t request_id,
											 bool sw_id_only)
{
	private_ietf_swima_attr_sw_ev_t *this;
	ietf_attr_t type;

	type = sw_id_only ? IETF_ATTR_SW_ID_EVENTS : IETF_ATTR_SW_EVENTS;

	INIT(this,
		.public = {
			.pa_tnc_attribute = {
				.get_type = _get_type,
				.get_value = _get_value,
				.get_noskip_flag = _get_noskip_flag,
				.set_noskip_flag = _set_noskip_flag,
				.build = _build,
				.process = _process,
				.add_segment = _add_segment,
				.get_ref = _get_ref,
				.destroy = _destroy,
			},
			.get_flags = _get_flags,
			.get_request_id = _get_request_id,
			.get_event_count = _get_event_count,
			.set_events = _set_events,
			.get_events = _get_events,
			.clear_events = _clear_events,
		},
		.type = { PEN_IETF, type },
		.flags = flags,
		.request_id = request_id,
		.events = swima_events_create(),
		.ref = 1,
	);

	return &this->public.pa_tnc_attribute;
}


/**
 * Described in header.
 */
pa_tnc_attr_t *ietf_swima_attr_sw_ev_create_from_data(size_t length,
										chunk_t data, bool sw_id_only)
{
	private_ietf_swima_attr_sw_ev_t *this;
	ietf_attr_t type;

	type = sw_id_only ? IETF_ATTR_SW_ID_EVENTS : IETF_ATTR_SW_EVENTS;

	INIT(this,
		.public = {
			.pa_tnc_attribute = {
				.get_type = _get_type,
				.get_value = _get_value,
				.get_noskip_flag = _get_noskip_flag,
				.set_noskip_flag = _set_noskip_flag,
				.build = _build,
				.process = _process,
				.add_segment = _add_segment,
				.get_ref = _get_ref,
				.destroy = _destroy,
			},
			.get_flags = _get_flags,
			.get_request_id = _get_request_id,
			.get_event_count = _get_event_count,
			.set_events = _set_events,
			.get_events = _get_events,
			.clear_events = _clear_events,
		},
		.type = { PEN_IETF, type },
		.length = length,
		.segment = chunk_clone(data),
		.events = swima_events_create(),
		.ref = 1,
	);

	/* received either complete attribute value or first segment */
	this->value = this->segment;

	return &this->public.pa_tnc_attribute;
}
