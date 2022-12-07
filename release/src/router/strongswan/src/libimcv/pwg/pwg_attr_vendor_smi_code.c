/*
 * Copyright (C) 2015 Andreas Steffen
 *
 * Copyright (C) secunet Security Networks AG
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

#include "pwg_attr_vendor_smi_code.h"

#include <pa_tnc/pa_tnc_msg.h>
#include <bio/bio_writer.h>
#include <bio/bio_reader.h>
#include <utils/debug.h>

typedef struct private_pwg_attr_vendor_smi_code_t private_pwg_attr_vendor_smi_code_t;

/**
 * PWG HCD PA-TNC Vendor SMI Code
 *
 *                       1                   2                   3
 *   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |    Reserved   |                 Vendor SMI Code               |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

#define VENDOR_SMI_CODE_SIZE	4

/**
 * Private data of an pwg_attr_vendor_smi_code_t object.
 */
struct private_pwg_attr_vendor_smi_code_t {

	/**
	 * Public members of pwg_attr_vendor_smi_code_t
	 */
	pwg_attr_vendor_smi_code_t public;

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
	 * Vendor SMI code
	 */
	pen_t vendor_smi_code;

	/**
	 * Reference count
	 */
	refcount_t ref;
};

METHOD(pa_tnc_attr_t, get_type, pen_type_t,
	private_pwg_attr_vendor_smi_code_t *this)
{
	return this->type;
}

METHOD(pa_tnc_attr_t, get_value, chunk_t,
	private_pwg_attr_vendor_smi_code_t *this)
{
	return this->value;
}

METHOD(pa_tnc_attr_t, get_noskip_flag, bool,
	private_pwg_attr_vendor_smi_code_t *this)
{
	return this->noskip_flag;
}

METHOD(pa_tnc_attr_t, set_noskip_flag,void,
	private_pwg_attr_vendor_smi_code_t *this, bool noskip)
{
	this->noskip_flag = noskip;
}

METHOD(pa_tnc_attr_t, build, void,
	private_pwg_attr_vendor_smi_code_t *this)
{
	bio_writer_t *writer;

	if (this->value.ptr)
	{
		return;
	}
	writer = bio_writer_create(VENDOR_SMI_CODE_SIZE);
	writer->write_uint32(writer, this->vendor_smi_code);

	this->value = writer->extract_buf(writer);
	this->length = this->value.len;
	writer->destroy(writer);
}

METHOD(pa_tnc_attr_t, process, status_t,
	private_pwg_attr_vendor_smi_code_t *this, uint32_t *offset)
{
	bio_reader_t *reader;
	uint32_t vendor_smi_code;
	uint8_t reserved;

	*offset = 0;

	if (this->value.len < this->length)
	{
		return NEED_MORE;
	}
	if (this->value.len != VENDOR_SMI_CODE_SIZE)
	{
		DBG1(DBG_TNC, "incorrect attribute length for PWG HCD Vendor SMI Code");
		return FAILED;
	}
	reader = bio_reader_create(this->value);
	reader->read_uint8 (reader, &reserved);
	reader->read_uint24(reader, &vendor_smi_code);
	reader->destroy(reader);
	this->vendor_smi_code = vendor_smi_code;

	return SUCCESS;
}

METHOD(pa_tnc_attr_t, add_segment, void,
	private_pwg_attr_vendor_smi_code_t *this, chunk_t segment)
{
	this->value = chunk_cat("mc", this->value, segment);
}

METHOD(pa_tnc_attr_t, get_ref, pa_tnc_attr_t*,
	private_pwg_attr_vendor_smi_code_t *this)
{
	ref_get(&this->ref);
	return &this->public.pa_tnc_attribute;
}

METHOD(pa_tnc_attr_t, destroy, void,
	private_pwg_attr_vendor_smi_code_t *this)
{
	if (ref_put(&this->ref))
	{
		free(this->value.ptr);
		free(this);
	}
}

METHOD(pwg_attr_vendor_smi_code_t, get_vendor_smi_code, pen_t,
	private_pwg_attr_vendor_smi_code_t *this)
{
	return this->vendor_smi_code;
}

/**
 * Described in header.
 */
pa_tnc_attr_t *pwg_attr_vendor_smi_code_create(pen_t vendor_smi_code)
{
	private_pwg_attr_vendor_smi_code_t *this;

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
			.get_vendor_smi_code = _get_vendor_smi_code,
		},
		.type = { PEN_PWG, PWG_HCD_VENDOR_SMI_CODE },
		.vendor_smi_code = vendor_smi_code,
		.ref = 1,
	);

	return &this->public.pa_tnc_attribute;
}

/**
 * Described in header.
 */
pa_tnc_attr_t *pwg_attr_vendor_smi_code_create_from_data(size_t length,
														 chunk_t data)
{
	private_pwg_attr_vendor_smi_code_t *this;

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
			.get_vendor_smi_code = _get_vendor_smi_code,
		},
		.type = { PEN_PWG, PWG_HCD_VENDOR_SMI_CODE },
		.length = length,
		.value = chunk_clone(data),
		.ref = 1,
	);

	return &this->public.pa_tnc_attribute;
}

