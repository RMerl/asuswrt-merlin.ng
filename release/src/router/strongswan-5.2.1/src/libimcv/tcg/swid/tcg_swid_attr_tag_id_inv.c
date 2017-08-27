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

#include "tcg_swid_attr_tag_id_inv.h"

#include <pa_tnc/pa_tnc_msg.h>
#include <bio/bio_writer.h>
#include <bio/bio_reader.h>
#include <utils/debug.h>


typedef struct private_tcg_swid_attr_tag_id_inv_t private_tcg_swid_attr_tag_id_inv_t;

/**
 * SWID Tag Identifier Inventory
 * see section 4.8 of TCG TNC SWID Message and Attributes for IF-M
 *
 *					   1				   2				   3
 *   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |   Reserved    |                 Tag ID Count                  | 
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                        Request ID Copy                        | 
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                           EID Epoch                           |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                           Last EID                            |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |       Tag Creator Length      | Tag Creator (variable length) |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |    Unique Software ID Length  |Unique Software ID (var length)|
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |       Instance ID Length      | Instance ID (variable length) |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

#define TCG_SWID_TAG_ID_INV_RESERVED	0x00

/**
 * Private data of an tcg_swid_attr_tag_id_inv_t object.
 */
struct private_tcg_swid_attr_tag_id_inv_t {

	/**
	 * Public members of tcg_swid_attr_tag_id_inv_t
	 */
	tcg_swid_attr_tag_id_inv_t public;

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
	 * Event ID Epoch
	 */
	uint32_t eid_epoch;

	/**
	 * Last Event ID
	 */
	uint32_t last_eid;

	/**
	 * Number of SWID Tag IDs in attribute
	 */
	uint32_t tag_id_count;

	/**
	 * SWID Tag ID Inventory
	 */
	swid_inventory_t *inventory;

	/**
	 * Reference count
	 */
	refcount_t ref;
};

METHOD(pa_tnc_attr_t, get_type, pen_type_t,
	private_tcg_swid_attr_tag_id_inv_t *this)
{
	return this->type;
}

METHOD(pa_tnc_attr_t, get_value, chunk_t,
	private_tcg_swid_attr_tag_id_inv_t *this)
{
	return this->value;
}

METHOD(pa_tnc_attr_t, get_noskip_flag, bool,
	private_tcg_swid_attr_tag_id_inv_t *this)
{
	return this->noskip_flag;
}

METHOD(pa_tnc_attr_t, set_noskip_flag,void,
	private_tcg_swid_attr_tag_id_inv_t *this, bool noskip)
{
	this->noskip_flag = noskip;
}

METHOD(pa_tnc_attr_t, build, void,
	private_tcg_swid_attr_tag_id_inv_t *this)
{
	bio_writer_t *writer;
	swid_tag_id_t *tag_id;
	chunk_t tag_creator, unique_sw_id, instance_id;
	enumerator_t *enumerator;

	if (this->value.ptr)
	{
		return;
	}

	writer = bio_writer_create(TCG_SWID_TAG_ID_INV_MIN_SIZE);
	writer->write_uint8 (writer, TCG_SWID_TAG_ID_INV_RESERVED);
	writer->write_uint24(writer, this->inventory->get_count(this->inventory));
	writer->write_uint32(writer, this->request_id);
	writer->write_uint32(writer, this->eid_epoch);
	writer->write_uint32(writer, this->last_eid);

	enumerator = this->inventory->create_enumerator(this->inventory);
	while (enumerator->enumerate(enumerator, &tag_id))
	{
		tag_creator = tag_id->get_tag_creator(tag_id);
		unique_sw_id = tag_id->get_unique_sw_id(tag_id, &instance_id);
		writer->write_data16(writer, tag_creator);
		writer->write_data16(writer, unique_sw_id);
		writer->write_data16(writer, instance_id);
	}
	enumerator->destroy(enumerator);

	this->value = writer->extract_buf(writer);
	this->segment = this->value;
	this->length = this->value.len;
	writer->destroy(writer);
}

METHOD(pa_tnc_attr_t, process, status_t,
	private_tcg_swid_attr_tag_id_inv_t *this, uint32_t *offset)
{
	bio_reader_t *reader;
	uint8_t reserved;
	chunk_t tag_creator, unique_sw_id, instance_id;
	swid_tag_id_t *tag_id;
	status_t status = NEED_MORE;

	if (this->offset == 0)
	{
		if (this->length < TCG_SWID_TAG_ID_INV_MIN_SIZE)
		{
			DBG1(DBG_TNC, "insufficient data for %N/%N", pen_names, PEN_TCG,
						   tcg_attr_names, this->type.type);
			*offset = this->offset;
			return FAILED;
		}
		if (this->value.len < TCG_SWID_TAG_ID_INV_MIN_SIZE)
		{
			return NEED_MORE;
		}
		reader = bio_reader_create(this->value);
		reader->read_uint8 (reader, &reserved);
		reader->read_uint24(reader, &this->tag_id_count);
		reader->read_uint32(reader, &this->request_id);
		reader->read_uint32(reader, &this->eid_epoch);
		reader->read_uint32(reader, &this->last_eid);
		this->offset = TCG_SWID_TAG_ID_INV_MIN_SIZE;
		this->value = reader->peek(reader);
		reader->destroy(reader);
	}

	reader = bio_reader_create(this->value);

	while (this->tag_id_count)
	{
		if (!reader->read_data16(reader, &tag_creator)  ||
			!reader->read_data16(reader, &unique_sw_id) ||
			!reader->read_data16(reader, &instance_id))
		{
			goto end;
		}
		tag_id = swid_tag_id_create(tag_creator, unique_sw_id, instance_id);
		this->inventory->add(this->inventory, tag_id);
		this->offset += this->value.len - reader->remaining(reader);
		this->value = reader->peek(reader);

		/* at least one tag ID was processed */
		status = SUCCESS;
		this->tag_id_count--;
	}

	if (this->length != this->offset)
	{
		DBG1(DBG_TNC, "inconsistent length for %N/%N", pen_names, PEN_TCG,
					   tcg_attr_names, this->type.type);
		*offset = this->offset;
		status = FAILED;
	}

end:
	reader->destroy(reader);
	return status;
}

METHOD(pa_tnc_attr_t, add_segment, void,
	private_tcg_swid_attr_tag_id_inv_t *this, chunk_t segment)
{
	this->value = chunk_cat("cc", this->value, segment);
	chunk_free(&this->segment);
	this->segment = this->value;
}

METHOD(pa_tnc_attr_t, get_ref, pa_tnc_attr_t*,
	private_tcg_swid_attr_tag_id_inv_t *this)
{
	ref_get(&this->ref);
	return &this->public.pa_tnc_attribute;
}

METHOD(pa_tnc_attr_t, destroy, void,
	private_tcg_swid_attr_tag_id_inv_t *this)
{
	if (ref_put(&this->ref))
	{
		this->inventory->destroy(this->inventory);
		free(this->segment.ptr);
		free(this);
	}
}

METHOD(tcg_swid_attr_tag_id_inv_t, add, void,
	private_tcg_swid_attr_tag_id_inv_t *this, swid_tag_id_t *tag_id)
{
	this->inventory->add(this->inventory, tag_id);
}

METHOD(tcg_swid_attr_tag_id_inv_t, get_request_id, uint32_t,
	private_tcg_swid_attr_tag_id_inv_t *this)
{
	return this->request_id;
}

METHOD(tcg_swid_attr_tag_id_inv_t, get_last_eid, uint32_t,
	private_tcg_swid_attr_tag_id_inv_t *this, uint32_t *eid_epoch)
{
	if (eid_epoch)
	{
		*eid_epoch = this->eid_epoch;
	}
	return this->last_eid;
}

METHOD(tcg_swid_attr_tag_id_inv_t, get_tag_id_count, uint32_t,
	private_tcg_swid_attr_tag_id_inv_t *this)
{
	return this->tag_id_count;
}

METHOD(tcg_swid_attr_tag_id_inv_t, get_inventory, swid_inventory_t*,
	private_tcg_swid_attr_tag_id_inv_t *this)
{
	return this->inventory;
}

METHOD(tcg_swid_attr_tag_id_inv_t, clear_inventory, void,
	private_tcg_swid_attr_tag_id_inv_t *this)
{
	this->inventory->destroy(this->inventory);
	this->inventory = swid_inventory_create(FALSE);
}

/**
 * Described in header.
 */
pa_tnc_attr_t *tcg_swid_attr_tag_id_inv_create(uint32_t request_id,
											   uint32_t eid_epoch,
											   uint32_t eid)
{
	private_tcg_swid_attr_tag_id_inv_t *this;

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
			.add = _add,
			.get_request_id = _get_request_id,
			.get_last_eid = _get_last_eid,
			.get_tag_id_count = _get_tag_id_count,
			.get_inventory = _get_inventory,
			.clear_inventory = _clear_inventory,
		},
		.type = { PEN_TCG, TCG_SWID_TAG_ID_INVENTORY },
		.request_id = request_id,
		.eid_epoch = eid_epoch,
		.last_eid = eid,
		.inventory = swid_inventory_create(FALSE),
		.ref = 1,
	);

	return &this->public.pa_tnc_attribute;
}


/**
 * Described in header.
 */
pa_tnc_attr_t *tcg_swid_attr_tag_id_inv_create_from_data(size_t length,
														 chunk_t data)
{
	private_tcg_swid_attr_tag_id_inv_t *this;

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
			.add = _add,
			.get_request_id = _get_request_id,
			.get_last_eid = _get_last_eid,
			.get_tag_id_count = _get_tag_id_count,
			.get_inventory = _get_inventory,
			.clear_inventory = _clear_inventory,
		},
		.type = { PEN_TCG, TCG_SWID_TAG_ID_INVENTORY },
		.length = length,
		.segment = chunk_clone(data),
		.inventory = swid_inventory_create(FALSE),
		.ref = 1,
	);

	/* received either complete attribute value or first segment */
	this->value = this->segment;

	return &this->public.pa_tnc_attribute;
}
