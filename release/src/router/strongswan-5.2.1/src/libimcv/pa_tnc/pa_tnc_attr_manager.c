/*
 * Copyright (C) 2011-2014 Andreas Steffen
 *
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

#include "pa_tnc_attr_manager.h"

#include "imcv.h"
#include "pa_tnc_attr.h"
#include "ietf/ietf_attr_pa_tnc_error.h"

#include <collections/linked_list.h>
#include <utils/debug.h>

typedef struct private_pa_tnc_attr_manager_t private_pa_tnc_attr_manager_t;
typedef struct entry_t entry_t;

struct entry_t {
	pen_t vendor_id;
	enum_name_t *attr_names;
	pa_tnc_attr_create_t attr_create;
};

/**
 * Private data of a pa_tnc_attr_manager_t object.
 *
 */
struct private_pa_tnc_attr_manager_t {

	/**
	 * Public pa_tnc_attr_manager_t interface.
	 */
	pa_tnc_attr_manager_t public;

	/**
	 * List of PA-TNC vendor attributes
	 */
	linked_list_t *list;
};

METHOD(pa_tnc_attr_manager_t, add_vendor, void,
	private_pa_tnc_attr_manager_t *this, pen_t vendor_id,
	pa_tnc_attr_create_t attr_create, enum_name_t *attr_names)
{
	entry_t *entry;

	entry = malloc_thing(entry_t);
	entry->vendor_id = vendor_id;
	entry->attr_create = attr_create;
	entry->attr_names = attr_names;

	this->list->insert_last(this->list, entry);
	DBG2(DBG_TNC, "added %N attributes", pen_names, vendor_id);
}

METHOD(pa_tnc_attr_manager_t, remove_vendor, void,
	private_pa_tnc_attr_manager_t *this, pen_t vendor_id)
{
	enumerator_t *enumerator;
	entry_t *entry;

	enumerator = this->list->create_enumerator(this->list);
	while (enumerator->enumerate(enumerator, &entry))
	{
		if (entry->vendor_id == vendor_id)
		{
			this->list->remove_at(this->list, enumerator);
			free(entry);
			DBG2(DBG_TNC, "removed %N attributes", pen_names, vendor_id);
		}
	}
	enumerator->destroy(enumerator);
}

METHOD(pa_tnc_attr_manager_t, get_names, enum_name_t*,
	private_pa_tnc_attr_manager_t *this, pen_t vendor_id)
{
	enumerator_t *enumerator;
	entry_t *entry;
	enum_name_t *attr_names = NULL;

	enumerator = this->list->create_enumerator(this->list);
	while (enumerator->enumerate(enumerator, &entry))
	{
		if (entry->vendor_id == vendor_id)
		{
			attr_names = entry->attr_names;
			break;
		}
	}
	enumerator->destroy(enumerator);

	return attr_names;
}

/**
 *  PA-TNC attribute
 *
 *                       1                   2                   3
 *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |     Flags     |          PA-TNC Attribute Vendor ID           |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                     PA-TNC Attribute Type                     |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                    PA-TNC Attribute Length                    |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                 Attribute Value (Variable Length)             |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

METHOD(pa_tnc_attr_manager_t, create, pa_tnc_attr_t*,
	private_pa_tnc_attr_manager_t *this, bio_reader_t *reader, bool segmented,
	uint32_t *offset, chunk_t msg_info, pa_tnc_attr_t **error)
{
	uint8_t flags;
	uint32_t type, length, value_len;
	chunk_t value;
	ietf_attr_pa_tnc_error_t *error_attr;
	pen_t vendor_id;
	pen_type_t unsupported_type;
	pen_type_t error_code = { PEN_IETF, PA_ERROR_INVALID_PARAMETER };
	enum_name_t *pa_attr_names;
	pa_tnc_attr_t *attr = NULL;
	enumerator_t *enumerator;
	entry_t *entry;

	/* properly initialize error return argument in case of no error */
	*error = NULL;

	if (reader->remaining(reader) < PA_TNC_ATTR_HEADER_SIZE)
	{
		DBG1(DBG_TNC, "insufficient bytes for PA-TNC attribute header");
		*error = ietf_attr_pa_tnc_error_create_with_offset(error_code,
							msg_info, *offset);
		return NULL;
	}
	reader->read_uint8 (reader, &flags);
	reader->read_uint24(reader, &vendor_id);
	reader->read_uint32(reader, &type);
	reader->read_uint32(reader, &length);

	pa_attr_names = imcv_pa_tnc_attributes->get_names(imcv_pa_tnc_attributes,
													  vendor_id);
	if (pa_attr_names)
	{
		DBG2(DBG_TNC, "processing PA-TNC attribute type '%N/%N' "
					  "0x%06x/0x%08x", pen_names, vendor_id,
					   pa_attr_names, type, vendor_id, type);
	}
	else
	{
		DBG2(DBG_TNC, "processing PA-TNC attribute type '%N' "
					  "0x%06x/0x%08x", pen_names, vendor_id,
					   vendor_id, type);
	}

	if (length < PA_TNC_ATTR_HEADER_SIZE)
	{
		DBG1(DBG_TNC, "%u bytes too small for PA-TNC attribute length",
					   length);
		*error = ietf_attr_pa_tnc_error_create_with_offset(error_code,
						msg_info, *offset + PA_TNC_ATTR_INFO_SIZE);
		return NULL;
	}
	length -= PA_TNC_ATTR_HEADER_SIZE;
	value_len = segmented ? reader->remaining(reader) : length;

	if (!reader->read_data(reader, value_len, &value))
	{
		DBG1(DBG_TNC, "insufficient bytes for PA-TNC attribute value");
		*error = ietf_attr_pa_tnc_error_create_with_offset(error_code,
						msg_info, *offset + PA_TNC_ATTR_INFO_SIZE);
		return NULL;
	}
	DBG3(DBG_TNC, "%B", &value);

	if (vendor_id == PEN_RESERVED)
	{
		*error = ietf_attr_pa_tnc_error_create_with_offset(error_code,
						msg_info, *offset + 1);
		return NULL;
	}
	if (type == IETF_ATTR_RESERVED)
	{
		*error = ietf_attr_pa_tnc_error_create_with_offset(error_code,
						msg_info, *offset + 4);
		return NULL;
	}

	/* check if the attribute type is registered */
	enumerator = this->list->create_enumerator(this->list);
	while (enumerator->enumerate(enumerator, &entry))
	{
		if (entry->vendor_id == vendor_id)
		{
			if (entry->attr_create)
			{
				attr = entry->attr_create(type, length, value);
			}
			break;
		}
	}
	enumerator->destroy(enumerator);

	if (!attr)
	{
		if (!(flags & PA_TNC_ATTR_FLAG_NOSKIP))
		{
			DBG1(DBG_TNC, "skipping unsupported PA-TNC attribute");
			(*offset) += PA_TNC_ATTR_HEADER_SIZE + length;
			return NULL;
		}

		DBG1(DBG_TNC, "unsupported PA-TNC attribute with NOSKIP flag");
		unsupported_type = pen_type_create(vendor_id, type);
		error_code = pen_type_create(PEN_IETF, PA_ERROR_ATTR_TYPE_NOT_SUPPORTED);
		*error = ietf_attr_pa_tnc_error_create(error_code, msg_info);
		error_attr = (ietf_attr_pa_tnc_error_t*)(*error);
		error_attr->set_unsupported_attr(error_attr, flags, unsupported_type);
		return NULL;
	}
	(*offset) += PA_TNC_ATTR_HEADER_SIZE;

	return attr;
}

METHOD(pa_tnc_attr_manager_t, construct, pa_tnc_attr_t*,
	private_pa_tnc_attr_manager_t *this, pen_t vendor_id, uint32_t type,
	chunk_t value)
{
	enum_name_t *pa_attr_names;
	pa_tnc_attr_t *attr = NULL;
	enumerator_t *enumerator;
	entry_t *entry;

	pa_attr_names = imcv_pa_tnc_attributes->get_names(imcv_pa_tnc_attributes,
													  vendor_id);
	if (pa_attr_names)
	{
		DBG2(DBG_TNC, "generating PA-TNC attribute type '%N/%N' "
					  "0x%06x/0x%08x", pen_names, vendor_id,
					   pa_attr_names, type, vendor_id, type);
	}
	else
	{
		DBG2(DBG_TNC, "generating PA-TNC attribute type '%N' "
					  "0x%06x/0x%08x", pen_names, vendor_id,
					   vendor_id, type);
	}
	enumerator = this->list->create_enumerator(this->list);
	while (enumerator->enumerate(enumerator, &entry))
	{
		if (entry->vendor_id == vendor_id)
		{
			if (entry->attr_create)
			{
				attr = entry->attr_create(type, value.len, value);
			}
			break;
		}
	}
	enumerator->destroy(enumerator);
	return attr;
}

METHOD(pa_tnc_attr_manager_t, destroy, void,
	private_pa_tnc_attr_manager_t *this)
{
	this->list->destroy_function(this->list, free);
	free(this);
}

/**
 * See header
 */
pa_tnc_attr_manager_t *pa_tnc_attr_manager_create(void)
{
	private_pa_tnc_attr_manager_t *this;

	INIT(this,
		.public = {
			.add_vendor = _add_vendor,
			.remove_vendor = _remove_vendor,
			.get_names = _get_names,
			.create = _create,
			.construct = _construct,
			.destroy = _destroy,
		},
		.list = linked_list_create(),
	);

	return &this->public;
}

