/*
 * Copyright (C) 2013-2014 Andreas Steffen
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

#include "tcg_swid_attr_req.h"

#include "swid/swid_tag_id.h"

#include <pa_tnc/pa_tnc_msg.h>
#include <bio/bio_writer.h>
#include <bio/bio_reader.h>
#include <utils/debug.h>
#include <collections/linked_list.h>

typedef struct private_tcg_swid_attr_req_t private_tcg_swid_attr_req_t;

/**
 * SWID Request
 * see section 4.7 of TCG TNC SWID Message and Attributes for IF-M
 *
 *                       1                   2                   3
 *   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |R|S|C| Reserved|                   Tag ID Count                |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                          Request ID                           |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                         Earliest EID                          |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |       Tag Creator Length      | Tag Creator (variable length) |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |    Unique Software ID Length  |Unique Software ID (var length)|
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

#define SWID_REQ_RESERVED_MASK			0xE0

/**
 * Private data of an tcg_swid_attr_req_t object.
 */
struct private_tcg_swid_attr_req_t {

	/**
	 * Public members of tcg_swid_attr_req_t
	 */
	tcg_swid_attr_req_t public;

	/**
	 * Vendor-specific attribute type
	 */
	pen_type_t type;

	/**
	 * Length of attribute value
	 */
	size_t length;


	/**
	 * Attribute value or segment
	 */
	chunk_t value;

	/**
	 * Noskip flag
	 */
	bool noskip_flag;

	/**
	 * SWID request flags
	 */
	u_int8_t flags;

	/**
	 * Request ID
	 */
	u_int32_t request_id;

	/**
	 * Earliest EID
	 */
	u_int32_t earliest_eid;

	/**
	 * List of Target Tag Identifiers
	 */
	swid_inventory_t *targets;

	/**
	 * Reference count
	 */
	refcount_t ref;
};

METHOD(pa_tnc_attr_t, get_type, pen_type_t,
	private_tcg_swid_attr_req_t *this)
{
	return this->type;
}

METHOD(pa_tnc_attr_t, get_value, chunk_t,
	private_tcg_swid_attr_req_t *this)
{
	return this->value;
}

METHOD(pa_tnc_attr_t, get_noskip_flag, bool,
	private_tcg_swid_attr_req_t *this)
{
	return this->noskip_flag;
}

METHOD(pa_tnc_attr_t, set_noskip_flag,void,
	private_tcg_swid_attr_req_t *this, bool noskip)
{
	this->noskip_flag = noskip;
}

METHOD(pa_tnc_attr_t, build, void,
	private_tcg_swid_attr_req_t *this)
{
	bio_writer_t *writer;
	chunk_t tag_creator, unique_sw_id;
	swid_tag_id_t *tag_id;
	enumerator_t *enumerator;

	if (this->value.ptr)
	{
		return;
	}

	writer = bio_writer_create(TCG_SWID_REQ_MIN_SIZE);
	writer->write_uint8 (writer, this->flags);
	writer->write_uint24(writer, this->targets->get_count(this->targets));
	writer->write_uint32(writer, this->request_id);
	writer->write_uint32(writer, this->earliest_eid);

	enumerator = this->targets->create_enumerator(this->targets);
	while (enumerator->enumerate(enumerator, &tag_id))
	{
		tag_creator = tag_id->get_tag_creator(tag_id);
		unique_sw_id = tag_id->get_unique_sw_id(tag_id, NULL);
		writer->write_data16(writer, tag_creator);
		writer->write_data16(writer, unique_sw_id);
	}
	enumerator->destroy(enumerator);

	this->value = writer->extract_buf(writer);
	this->length = this->value.len;
	writer->destroy(writer);
}

METHOD(pa_tnc_attr_t, process, status_t,
	private_tcg_swid_attr_req_t *this, u_int32_t *offset)
{
	bio_reader_t *reader;
	u_int32_t tag_id_count;
	chunk_t tag_creator, unique_sw_id;
	swid_tag_id_t *tag_id;

	*offset = 0;

	if (this->value.len < this->length)
	{
		return NEED_MORE;
	}
	if (this->value.len < TCG_SWID_REQ_MIN_SIZE)
	{
		DBG1(DBG_TNC, "insufficient data for SWID Request");
		return FAILED;
	}

	reader = bio_reader_create(this->value);
	reader->read_uint8 (reader, &this->flags);
	reader->read_uint24(reader, &tag_id_count);
	reader->read_uint32(reader, &this->request_id);
	reader->read_uint32(reader, &this->earliest_eid);

	if (this->request_id == 0)
	{
		*offset = 4;
		return FAILED;
	}
	*offset = TCG_SWID_REQ_MIN_SIZE;

	this->flags &= SWID_REQ_RESERVED_MASK;

	while (tag_id_count--)
	{
		if (!reader->read_data16(reader, &tag_creator))
		{
			DBG1(DBG_TNC, "insufficient data for Tag Creator field");
			return FAILED;
		}
		*offset += 2 + tag_creator.len;

		if (!reader->read_data16(reader, &unique_sw_id))
		{
			DBG1(DBG_TNC, "insufficient data for Unique Software ID");
			return FAILED;
		}
		*offset += 2 + unique_sw_id.len;
		
		tag_id = swid_tag_id_create(tag_creator, unique_sw_id, chunk_empty);
		this->targets->add(this->targets, tag_id);
	}
	reader->destroy(reader);

	return SUCCESS;
}

METHOD(pa_tnc_attr_t, add_segment, void,
	private_tcg_swid_attr_req_t *this, chunk_t segment)
{
	this->value = chunk_cat("mc", this->value, segment);
}

METHOD(pa_tnc_attr_t, get_ref, pa_tnc_attr_t*,
	private_tcg_swid_attr_req_t *this)
{
	ref_get(&this->ref);
	return &this->public.pa_tnc_attribute;
}

METHOD(pa_tnc_attr_t, destroy, void,
	private_tcg_swid_attr_req_t *this)
{
	if (ref_put(&this->ref))
	{
		this->targets->destroy(this->targets);
		free(this->value.ptr);
		free(this);
	}
}

METHOD(tcg_swid_attr_req_t, get_flags, u_int8_t,
	private_tcg_swid_attr_req_t *this)
{
	return this->flags;
}

METHOD(tcg_swid_attr_req_t, get_request_id, u_int32_t,
	private_tcg_swid_attr_req_t *this)
{
	return this->request_id;
}

METHOD(tcg_swid_attr_req_t, get_earliest_eid, u_int32_t,
	private_tcg_swid_attr_req_t *this)
{
	return this->earliest_eid;
}

METHOD(tcg_swid_attr_req_t, add_target, void,
	private_tcg_swid_attr_req_t *this, swid_tag_id_t *tag_id)
{
	this->targets->add(this->targets, tag_id);
}

METHOD(tcg_swid_attr_req_t, get_targets, swid_inventory_t*,
	private_tcg_swid_attr_req_t *this)
{
	return this->targets;
}

/**
 * Described in header.
 */
pa_tnc_attr_t *tcg_swid_attr_req_create(u_int8_t flags, u_int32_t request_id,
										u_int32_t eid)
{
	private_tcg_swid_attr_req_t *this;

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
			.get_earliest_eid = _get_earliest_eid,
			.add_target = _add_target,
			.get_targets = _get_targets,
		},
		.type = { PEN_TCG, TCG_SWID_REQUEST },
		.flags = flags & SWID_REQ_RESERVED_MASK,
		.request_id = request_id,
		.earliest_eid = eid,
		.targets = swid_inventory_create(FALSE),
		.ref = 1,
	);

	return &this->public.pa_tnc_attribute;
}

/**
 * Described in header.
 */
pa_tnc_attr_t *tcg_swid_attr_req_create_from_data(size_t length, chunk_t data)
{
	private_tcg_swid_attr_req_t *this;

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
			.get_earliest_eid = _get_earliest_eid,
			.add_target = _add_target,
			.get_targets = _get_targets,
		},
		.type = { PEN_TCG, TCG_SWID_REQUEST },
		.length = length,
		.value = chunk_clone(data),
		.targets = swid_inventory_create(FALSE),
		.ref = 1,
	);

	return &this->public.pa_tnc_attribute;
}
