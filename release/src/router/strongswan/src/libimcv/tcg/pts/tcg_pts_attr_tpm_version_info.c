/*
 * Copyright (C) 2011-2012 Sansar Choinyambuu
 * Copyright (C) 2011-2014 Andreas Steffen
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

#include "tcg_pts_attr_tpm_version_info.h"

#include <pa_tnc/pa_tnc_msg.h>
#include <bio/bio_writer.h>
#include <bio/bio_reader.h>
#include <utils/debug.h>

typedef struct private_tcg_pts_attr_tpm_version_info_t private_tcg_pts_attr_tpm_version_info_t;

/**
 * TPM Version Information
 * see section 3.11 of PTS Protocol: Binding to TNC IF-M Specification
 *
 *					   1				   2				   3
 *   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |		  TPM Version Information (Variable Length)				|
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * see TPM Structure Specification Part 2, section 21.6: TPM_CAP_VERSION_INFO
 */

#define PTS_TPM_VER_INFO_SIZE		4

/**
 * Private data of an tcg_pts_attr_tpm_version_info_t object.
 */
struct private_tcg_pts_attr_tpm_version_info_t {

	/**
	 * Public members of tcg_pts_attr_tpm_version_info_t
	 */
	tcg_pts_attr_tpm_version_info_t public;

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
	 * TPM Version Information
	 */
	chunk_t tpm_version_info;

	/**
	 * Reference count
	 */
	refcount_t ref;
};

METHOD(pa_tnc_attr_t, get_type, pen_type_t,
	private_tcg_pts_attr_tpm_version_info_t *this)
{
	return this->type;
}

METHOD(pa_tnc_attr_t, get_value, chunk_t,
	private_tcg_pts_attr_tpm_version_info_t *this)
{
	return this->value;
}

METHOD(pa_tnc_attr_t, get_noskip_flag, bool,
	private_tcg_pts_attr_tpm_version_info_t *this)
{
	return this->noskip_flag;
}

METHOD(pa_tnc_attr_t, set_noskip_flag,void,
	private_tcg_pts_attr_tpm_version_info_t *this, bool noskip)
{
	this->noskip_flag = noskip;
}

METHOD(pa_tnc_attr_t, build, void,
	private_tcg_pts_attr_tpm_version_info_t *this)
{
	bio_writer_t *writer;

	if (this->value.ptr)
	{
		return;
	}
	writer = bio_writer_create(PTS_TPM_VER_INFO_SIZE);
	writer->write_data(writer, this->tpm_version_info);

	this->value = writer->extract_buf(writer);
	this->length = this->value.len;
	writer->destroy(writer);
}

METHOD(pa_tnc_attr_t, process, status_t,
	private_tcg_pts_attr_tpm_version_info_t *this, uint32_t *offset)
{
	bio_reader_t *reader;

	*offset = 0;

	if (this->value.len < this->length)
	{
		return NEED_MORE;
	}
	if (this->value.len < PTS_TPM_VER_INFO_SIZE)
	{
		DBG1(DBG_TNC, "insufficient data for TPM Version Information");
		return FAILED;
	}
	reader = bio_reader_create(this->value);
	reader->read_data  (reader, this->value.len, &this->tpm_version_info);
	this->tpm_version_info = chunk_clone(this->tpm_version_info);
	reader->destroy(reader);

	return SUCCESS;
}

METHOD(pa_tnc_attr_t, add_segment, void,
	private_tcg_pts_attr_tpm_version_info_t *this, chunk_t segment)
{
	this->value = chunk_cat("mc", this->value, segment);
}

METHOD(pa_tnc_attr_t, get_ref, pa_tnc_attr_t*,
	private_tcg_pts_attr_tpm_version_info_t *this)
{
	ref_get(&this->ref);
	return &this->public.pa_tnc_attribute;
}

METHOD(pa_tnc_attr_t, destroy, void,
	private_tcg_pts_attr_tpm_version_info_t *this)
{
	if (ref_put(&this->ref))
	{
		free(this->value.ptr);
		free(this->tpm_version_info.ptr);
		free(this);
	}
}

METHOD(tcg_pts_attr_tpm_version_info_t, get_tpm_version_info, chunk_t,
	private_tcg_pts_attr_tpm_version_info_t *this)
{
	return this->tpm_version_info;
}

METHOD(tcg_pts_attr_tpm_version_info_t, set_tpm_version_info, void,
		private_tcg_pts_attr_tpm_version_info_t *this,
		chunk_t tpm_version_info)
{
	this->tpm_version_info = tpm_version_info;
}

/**
 * Described in header.
 */
pa_tnc_attr_t *tcg_pts_attr_tpm_version_info_create(chunk_t tpm_version_info)
{
	private_tcg_pts_attr_tpm_version_info_t *this;

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
			.get_tpm_version_info = _get_tpm_version_info,
			.set_tpm_version_info = _set_tpm_version_info,
		},
		.type = { PEN_TCG, TCG_PTS_TPM_VERSION_INFO },
		.tpm_version_info = chunk_clone(tpm_version_info),
		.ref = 1,
	);

	return &this->public.pa_tnc_attribute;
}


/**
 * Described in header.
 */
pa_tnc_attr_t *tcg_pts_attr_tpm_version_info_create_from_data(size_t length,
															  chunk_t data)
{
	private_tcg_pts_attr_tpm_version_info_t *this;

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
			.get_tpm_version_info = _get_tpm_version_info,
			.set_tpm_version_info = _set_tpm_version_info,
		},
		.type = { PEN_TCG, TCG_PTS_TPM_VERSION_INFO },
		.length = length,
		.value = chunk_clone(data),
		.ref = 1,
	);

	return &this->public.pa_tnc_attribute;
}
