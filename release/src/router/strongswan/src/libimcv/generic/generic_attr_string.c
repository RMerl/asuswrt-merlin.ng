/*
 * Copyright (C) 2013-2015 Andreas Steffen
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

#include "generic_attr_string.h"

#include <imcv.h>
#include <pen/pen.h>
#include <utils/debug.h>

typedef struct private_generic_attr_string_t private_generic_attr_string_t;

/**
 * Private data of an generic_attr_string_t object.
 */
struct private_generic_attr_string_t {

	/**
	 * Public members of generic_attr_string_t
	 */
	generic_attr_string_t public;

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
	 * Reference count
	 */
	refcount_t ref;
};

METHOD(pa_tnc_attr_t, get_type, pen_type_t,
	private_generic_attr_string_t *this)
{
	return this->type;
}

METHOD(pa_tnc_attr_t, get_value, chunk_t,
	private_generic_attr_string_t *this)
{
	return this->value;
}

METHOD(pa_tnc_attr_t, get_noskip_flag, bool,
	private_generic_attr_string_t *this)
{
	return this->noskip_flag;
}

METHOD(pa_tnc_attr_t, set_noskip_flag,void,
	private_generic_attr_string_t *this, bool noskip)
{
	this->noskip_flag = noskip;
}

METHOD(pa_tnc_attr_t, build, void,
	private_generic_attr_string_t *this)
{
	return;
}

METHOD(pa_tnc_attr_t, process, status_t,
	private_generic_attr_string_t *this, uint32_t *offset)
{
	enum_name_t *pa_attr_names;
	u_char *pos;
	*offset = 0;

	if (this->value.len < this->length)
	{
		return NEED_MORE;
	}
	pa_attr_names = imcv_pa_tnc_attributes->get_names(imcv_pa_tnc_attributes,
													  this->type.vendor_id);
	if (this->value.len > this->length)
	{
		DBG1(DBG_TNC, "inconsistent length of %N/%N string attribute",
			 pen_names, this->type.vendor_id, pa_attr_names, this->type.type);
		return FAILED;
	}

	pos = memchr(this->value.ptr, '\0', this->value.len);
	if (pos)
	{
		DBG1(DBG_TNC, "nul termination in %N/%N string attribute",
			 pen_names, this->type.vendor_id, pa_attr_names, this->type.type);
		*offset = pos - this->value.ptr;
		return FAILED;
	}

	return SUCCESS;
}

METHOD(pa_tnc_attr_t, add_segment, void,
	private_generic_attr_string_t *this, chunk_t segment)
{
	this->value = chunk_cat("mc", this->value, segment);
}

METHOD(pa_tnc_attr_t, get_ref, pa_tnc_attr_t*,
	private_generic_attr_string_t *this)
{
	ref_get(&this->ref);
	return &this->public.pa_tnc_attribute;
}

METHOD(pa_tnc_attr_t, destroy, void,
	private_generic_attr_string_t *this)
{
	if (ref_put(&this->ref))
	{
		free(this->value.ptr);
		free(this);
	}
}

/**
 * Described in header.
 */
pa_tnc_attr_t *generic_attr_string_create_from_data(size_t length,
					 				chunk_t value, pen_type_t type)
{
	private_generic_attr_string_t *this;

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
		},
		.type = type,
		.length = length,
		.value = chunk_clone(value),
		.ref = 1,
	);

	return &this->public.pa_tnc_attribute;
}

/**
 * Described in header.
 */
pa_tnc_attr_t *generic_attr_string_create(chunk_t value, pen_type_t type)
{
	return generic_attr_string_create_from_data(value.len, value, type);
}

