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

#define _GNU_SOURCE /* for stdndup() */
#include <string.h>

#include "tcg_pts_attr_unix_file_meta.h"

#include <pa_tnc/pa_tnc_msg.h>
#include <bio/bio_writer.h>
#include <bio/bio_reader.h>
#include <collections/linked_list.h>
#include <utils/debug.h>

typedef struct private_tcg_pts_attr_file_meta_t private_tcg_pts_attr_file_meta_t;

/**
 * Unix-Style File Metadata
 * see section 3.17.3 of PTS Protocol: Binding to TNC IF-M Specification
 *
 *					   1				   2				   3
 *   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |				   Number of Files included						|
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |				   Number of Files included						|
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |	File metadata Length	    |	 Type	    |	Reserved	|
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |							File Size							|
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |							File Size							|
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |						File Create Time						|
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |						File Create Time						|
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |						Last Modify Time						|
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |						Last Modify Time						|
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |						Last Access Time						|
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |						Last Access Time						|
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |						File Owner ID							|
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |						File Owner ID							|
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |						File Group ID							|
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |						File Group ID							|
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  ~					Filename (Variable Length)					~
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *					 ...........................
 */

#define PTS_FILE_META_SIZE			8
#define PTS_FILE_MEAS_RESERVED		0x00
#define PTS_FILE_METADATA_SIZE		52

/**
 * Private data of an tcg_pts_attr_file_meta_t object.
 */
struct private_tcg_pts_attr_file_meta_t {

	/**
	 * Public members of tcg_pts_attr_file_meta_t
	 */
	tcg_pts_attr_file_meta_t public;

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
	 * PTS File Metadata
	 */
	pts_file_meta_t *metadata;

	/**
	 * Reference count
	 */
	refcount_t ref;
};

METHOD(pa_tnc_attr_t, get_type, pen_type_t,
	private_tcg_pts_attr_file_meta_t *this)
{
	return this->type;
}

METHOD(pa_tnc_attr_t, get_value, chunk_t,
	private_tcg_pts_attr_file_meta_t *this)
{
	return this->value;
}

METHOD(pa_tnc_attr_t, get_noskip_flag, bool,
	private_tcg_pts_attr_file_meta_t *this)
{
	return this->noskip_flag;
}

METHOD(pa_tnc_attr_t, set_noskip_flag,void,
	private_tcg_pts_attr_file_meta_t *this, bool noskip)
{
	this->noskip_flag = noskip;
}

METHOD(pa_tnc_attr_t, build, void,
	private_tcg_pts_attr_file_meta_t *this)
{
	bio_writer_t *writer;
	enumerator_t *enumerator;
	pts_file_metadata_t *entry;
	u_int64_t number_of_files;

	if (this->value.ptr)
	{
		return;
	}
	number_of_files = this->metadata->get_file_count(this->metadata);
	writer = bio_writer_create(PTS_FILE_META_SIZE);

	writer->write_uint64(writer, number_of_files);

	enumerator = this->metadata->create_enumerator(this->metadata);
	while (enumerator->enumerate(enumerator, &entry))
	{
		writer->write_uint16(writer, PTS_FILE_METADATA_SIZE +
									 strlen(entry->filename));
		writer->write_uint8 (writer, entry->type);
		writer->write_uint8 (writer, PTS_FILE_MEAS_RESERVED);
		writer->write_uint64(writer, entry->filesize);
		writer->write_uint64(writer, entry->created);
		writer->write_uint64(writer, entry->modified);
		writer->write_uint64(writer, entry->accessed);
		writer->write_uint64(writer, entry->owner);
		writer->write_uint64(writer, entry->group);
		writer->write_data  (writer, chunk_create(entry->filename,
												  strlen(entry->filename)));
	}
	enumerator->destroy(enumerator);

	this->value = writer->extract_buf(writer);
	this->length = this->value.len;
	writer->destroy(writer);
}

METHOD(pa_tnc_attr_t, process, status_t,
	private_tcg_pts_attr_file_meta_t *this, u_int32_t *offset)
{
	bio_reader_t *reader;
	pts_file_metadata_t *entry;
	u_int8_t type, reserved;
	u_int16_t len;
	u_int64_t number_of_files, filesize, created, modified, accessed;
	u_int64_t owner, group;
	chunk_t filename;
	status_t status = FAILED;

	*offset = 0;

	if (this->value.len < this->length)
	{
		return NEED_MORE;
	}
	if (this->value.len < PTS_FILE_META_SIZE)
	{
		DBG1(DBG_TNC, "insufficient data for PTS Unix-Style file metadata header");
		return FAILED;
	}
	reader = bio_reader_create(this->value);
	reader->read_uint64(reader, &number_of_files);

	this->metadata = pts_file_meta_create();

	while (number_of_files--)
	{
		if (!reader->read_uint16(reader, &len))
		{
			DBG1(DBG_TNC, "insufficient data for PTS file metadata length");
			goto end;
		}
		if (!reader->read_uint8(reader, &type))
		{
			DBG1(DBG_TNC, "insufficient data for file type");
			goto end;
		}
		if (!reader->read_uint8(reader, &reserved))
		{
			DBG1(DBG_TNC, "insufficient data for reserved field");
			goto end;
		}
		if (!reader->read_uint64(reader, &filesize))
		{
			DBG1(DBG_TNC, "insufficient data for file size");
			goto end;
		}
		if (!reader->read_uint64(reader, &created))
		{
			DBG1(DBG_TNC, "insufficient data for file create time");
			goto end;
		}
		if (!reader->read_uint64(reader, &modified))
		{
			DBG1(DBG_TNC, "insufficient data for last modify time");
			goto end;
		}
		if (!reader->read_uint64(reader, &accessed))
		{
			DBG1(DBG_TNC, "insufficient data for last access time");
			goto end;
		}
		if (!reader->read_uint64(reader, &owner))
		{
			DBG1(DBG_TNC, "insufficient data for owner id");
			goto end;
		}
		if (!reader->read_uint64(reader, &group))
		{
			DBG1(DBG_TNC, "insufficient data for group id");
			goto end;
		}
		if (!reader->read_data(reader, len - PTS_FILE_METADATA_SIZE, &filename))
		{
			DBG1(DBG_TNC, "insufficient data for filename");
			goto end;
		}

		entry = malloc_thing(pts_file_metadata_t);
		entry->type = type;
		entry->filesize = filesize;
		entry->created = created;
		entry->modified = modified;
		entry->accessed = accessed;
		entry->owner = owner;
		entry->group = group;
		entry->filename = strndup(filename.ptr, filename.len);

		this->metadata->add(this->metadata, entry);
	}
	status = SUCCESS;

end:
	reader->destroy(reader);
	return status;
}

METHOD(pa_tnc_attr_t, add_segment, void,
	private_tcg_pts_attr_file_meta_t *this, chunk_t segment)
{
	this->value = chunk_cat("mc", this->value, segment);
}

METHOD(pa_tnc_attr_t, get_ref, pa_tnc_attr_t*,
	private_tcg_pts_attr_file_meta_t *this)
{
	ref_get(&this->ref);
	return &this->public.pa_tnc_attribute;
}

METHOD(pa_tnc_attr_t, destroy, void,
	private_tcg_pts_attr_file_meta_t *this)
{
	if (ref_put(&this->ref))
	{
		DESTROY_IF(this->metadata);
		free(this->value.ptr);
		free(this);
	}
}

METHOD(tcg_pts_attr_file_meta_t, get_metadata, pts_file_meta_t*,
	private_tcg_pts_attr_file_meta_t *this)
{
	return this->metadata;
}

/**
 * Described in header.
 */
pa_tnc_attr_t *tcg_pts_attr_unix_file_meta_create(pts_file_meta_t *metadata)
{
	private_tcg_pts_attr_file_meta_t *this;

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
			.get_metadata = _get_metadata,
		},
		.type = { PEN_TCG, TCG_PTS_UNIX_FILE_META },
		.metadata = metadata,
		.ref = 1,
	);

	return &this->public.pa_tnc_attribute;
}


/**
 * Described in header.
 */
pa_tnc_attr_t *tcg_pts_attr_unix_file_meta_create_from_data(size_t length,
															chunk_t data)
{
	private_tcg_pts_attr_file_meta_t *this;

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
			.get_metadata = _get_metadata,
		},
		.type = { PEN_TCG, TCG_PTS_UNIX_FILE_META },
		.length = length,
		.value = chunk_clone(data),
		.ref = 1,
	);

	return &this->public.pa_tnc_attribute;
}
