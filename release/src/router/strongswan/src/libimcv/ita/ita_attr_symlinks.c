/*
 * Copyright (C) 2020 Andreas Steffen
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

#include "ita_attr_symlinks.h"

#include <pa_tnc/pa_tnc_msg.h>
#include <bio/bio_writer.h>
#include <bio/bio_reader.h>
#include <collections/linked_list.h>
#include <utils/debug.h>

typedef struct private_ita_attr_symlinks_t private_ita_attr_symlinks_t;

/**
 * List of Symbolic Links pointing to Directories
 *
 *					   1				   2				   3
 *   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                      Number of Symlinks                       |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |       Symlink #1 Length       |  Symlink #1 Path (Var Len)    ~
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                Symlink #1 Path (Variable Length)              |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |       Directory #1 Length     |  Directory #1 Path (Var Len)  ~
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |              Directory #1 Path (Variable Length)              |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |       Symlink #2 Length       |  Symlink #2 Path (Var Len)    ~
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                Symlink #2 Path (Variable Length)              |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |       Directory #2 Length     |  Directory #2 Path (Var Len)  ~
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |              Directory #2 Path (Variable Length)              |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *					 ...........................
 */

#define ITA_ATTR_SYMLINKS_SIZE		4

/**
 * Private data of an ita_attr_symlinks_t object.
 */
struct private_ita_attr_symlinks_t {

	/**
	 * Public members of ita_attr_symlinks_t
	 */
	ita_attr_symlinks_t public;

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
	 * Number of symbolic link entries
	 */
	uint32_t count;

	/**
	 * List of symbolic links
	 */
	pts_symlinks_t *symlinks;

	/**
	 * Reference count
	 */
	refcount_t ref;
};

METHOD(pa_tnc_attr_t, get_type, pen_type_t,
	private_ita_attr_symlinks_t *this)
{
	return this->type;
}

METHOD(pa_tnc_attr_t, get_value, chunk_t,
	private_ita_attr_symlinks_t *this)
{
	return this->value;
}

METHOD(pa_tnc_attr_t, get_noskip_flag, bool,
	private_ita_attr_symlinks_t *this)
{
	return this->noskip_flag;
}

METHOD(pa_tnc_attr_t, set_noskip_flag,void,
	private_ita_attr_symlinks_t *this, bool noskip)
{
	this->noskip_flag = noskip;
}

METHOD(pa_tnc_attr_t, build, void,
	private_ita_attr_symlinks_t *this)
{
	bio_writer_t *writer;
	enumerator_t *enumerator;
	chunk_t symlink, dir;

	if (this->value.ptr)
	{
		return;
	}
	this->count = this->symlinks->get_count(this->symlinks);

	writer = bio_writer_create(ITA_ATTR_SYMLINKS_SIZE);
	writer->write_uint32(writer, this->count);

	enumerator = this->symlinks->create_enumerator(this->symlinks);
	while (enumerator->enumerate(enumerator, &symlink, &dir))
	{
		writer->write_data16(writer, symlink);
		writer->write_data16(writer, dir);
	}
	enumerator->destroy(enumerator);

	this->value = writer->extract_buf(writer);
	this->segment = this->value;
	this->length = this->value.len;
	writer->destroy(writer);
}

METHOD(pa_tnc_attr_t, process, status_t,
	private_ita_attr_symlinks_t *this, uint32_t *offset)
{
	bio_reader_t *reader;
	chunk_t symlink, dir;
	status_t status = NEED_MORE;

	if (this->offset == 0)
	{
		if (this->length < ITA_ATTR_SYMLINKS_SIZE)
		{
			DBG1(DBG_TNC, "insufficient data for %N/%N", pen_names, PEN_ITA,
						   ita_attr_names, this->type.type);
			*offset = this->offset;
			return FAILED;
		}
		if (this->value.len < ITA_ATTR_SYMLINKS_SIZE)
		{
			return NEED_MORE;
		}
		reader = bio_reader_create(this->value);
		reader->read_uint32(reader, &this->count);
		this->offset = ITA_ATTR_SYMLINKS_SIZE;
		this->value = reader->peek(reader);
		reader->destroy(reader);
	}

	this->symlinks = pts_symlinks_create();
	reader = bio_reader_create(this->value);

	while (this->count)
	{
		if (!reader->read_data16(reader, &symlink) ||
			!reader->read_data16(reader, &dir))
		{
			goto end;
		}
		this->offset += this->value.len - reader->remaining(reader);
		this->value = reader->peek(reader);
		this->symlinks->add(this->symlinks, symlink, dir);
		this->count--;
	}

	status = SUCCESS;

	if (this->length != this->offset)
	{
		DBG1(DBG_TNC, "inconsistent length for %N/%N", pen_names, PEN_ITA,
					   ita_attr_names, this->type.type);
		*offset = this->offset;
		status = FAILED;
	}

end:
	reader->destroy(reader);
	return status;
}

METHOD(pa_tnc_attr_t, add_segment, void,
	private_ita_attr_symlinks_t *this, chunk_t segment)
{
	this->value = chunk_cat("cc", this->value, segment);
	chunk_free(&this->segment);
	this->segment = this->value;
}

METHOD(pa_tnc_attr_t, get_ref, pa_tnc_attr_t*,
	private_ita_attr_symlinks_t *this)
{
	ref_get(&this->ref);
	return &this->public.pa_tnc_attribute;
}

METHOD(pa_tnc_attr_t, destroy, void,
	private_ita_attr_symlinks_t *this)
{
	if (ref_put(&this->ref))
	{
		DESTROY_IF(this->symlinks);
		free(this->segment.ptr);
		free(this);
	}
}

METHOD(ita_attr_symlinks_t, get_symlinks, pts_symlinks_t*,
	private_ita_attr_symlinks_t *this)
{
	return this->symlinks;
}

/**
 * Described in header.
 */
pa_tnc_attr_t *ita_attr_symlinks_create(pts_symlinks_t *symlinks)
{
	private_ita_attr_symlinks_t *this;

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
			.get_symlinks = _get_symlinks,
		},
		.type = { PEN_ITA, ITA_ATTR_SYMLINKS },
		.symlinks = symlinks->get_ref(symlinks),
		.ref = 1,
	);

	return &this->public.pa_tnc_attribute;
}


/**
 * Described in header.
 */
pa_tnc_attr_t *ita_attr_symlinks_create_from_data(size_t length, chunk_t data)
{
	private_ita_attr_symlinks_t *this;

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
			.get_symlinks = _get_symlinks,
		},
		.type = { PEN_ITA, ITA_ATTR_SYMLINKS },
		.length = length,
		.segment = chunk_clone(data),
		.ref = 1,
	);

	/* received either complete attribute value or first segment */
	this->value = this->segment;

	return &this->public.pa_tnc_attribute;
}
