/*
 * Copyright (C) 2012-2014 Andreas Steffen
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

#include "ietf_attr_installed_packages.h"

#include <string.h>

#include <pa_tnc/pa_tnc_msg.h>
#include <bio/bio_writer.h>
#include <bio/bio_reader.h>
#include <collections/linked_list.h>
#include <utils/debug.h>


typedef struct private_ietf_attr_installed_packages_t private_ietf_attr_installed_packages_t;
typedef struct package_entry_t package_entry_t;

/**
 * PA-TNC Installed Packages Type  (see section 4.2.7 of RFC 5792)
 *
 *                       1                   2                   3
 *   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |          Reserved             |         Package Count         |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  | Pkg Name Len  |        Package Name (Variable Length)         |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |  Version Len  |    Package Version Number (Variable Length)   |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

/**
 * Private data of an ietf_attr_installed_packages_t object.
 */
struct private_ietf_attr_installed_packages_t {

	/**
	 * Public members of ietf_attr_installed_packages_t
	 */
	ietf_attr_installed_packages_t public;

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
	 * Number of Installed Packages in attribute
	 */
	uint16_t count;

	/**
	 * List of Installed Package entries
	 */
	linked_list_t *packages;

	/**
	 * Reference count
	 */
	refcount_t ref;
};

/**
 * Package entry
 */
struct package_entry_t {
	chunk_t name;
	chunk_t version;
};

/**
 * Free a package entry
 */
static void free_package_entry(package_entry_t *entry)
{
	free(entry->name.ptr);
	free(entry->version.ptr);
	free(entry);
}

METHOD(pa_tnc_attr_t, get_type, pen_type_t,
	private_ietf_attr_installed_packages_t *this)
{
	return this->type;
}

METHOD(pa_tnc_attr_t, get_value, chunk_t,
	private_ietf_attr_installed_packages_t *this)
{
	return this->value;
}

METHOD(pa_tnc_attr_t, get_noskip_flag, bool,
	private_ietf_attr_installed_packages_t *this)
{
	return this->noskip_flag;
}

METHOD(pa_tnc_attr_t, set_noskip_flag,void,
	private_ietf_attr_installed_packages_t *this, bool noskip)
{
	this->noskip_flag = noskip;
}

METHOD(pa_tnc_attr_t, build, void,
	private_ietf_attr_installed_packages_t *this)
{
	bio_writer_t *writer;
	enumerator_t *enumerator;
	package_entry_t *entry;

	if (this->value.ptr)
	{
		return;
	}
	writer = bio_writer_create(IETF_INSTALLED_PACKAGES_MIN_SIZE);
	writer->write_uint16(writer, 0x0000);
	writer->write_uint16(writer, this->packages->get_count(this->packages));

	enumerator = this->packages->create_enumerator(this->packages);
	while (enumerator->enumerate(enumerator, &entry))
	{
		writer->write_data8(writer, entry->name);
		writer->write_data8(writer, entry->version);
	}
	enumerator->destroy(enumerator);

	this->value = writer->extract_buf(writer);
	this->segment = this->value;
	this->length = this->value.len;
	writer->destroy(writer);
}

METHOD(pa_tnc_attr_t, process, status_t,
	private_ietf_attr_installed_packages_t *this, uint32_t *offset)
{
	bio_reader_t *reader;
	package_entry_t *entry;
	status_t status = NEED_MORE;
	chunk_t name, version;
	uint16_t reserved;
	u_char *pos;

	if (this->offset == 0)
	{
		if (this->length < IETF_INSTALLED_PACKAGES_MIN_SIZE)
		{
			DBG1(DBG_TNC, "insufficient data for %N/%N", pen_names, PEN_IETF,
						   ietf_attr_names, this->type.type);
			*offset = this->offset;
			return FAILED;
		}
		if (this->value.len < IETF_INSTALLED_PACKAGES_MIN_SIZE)
		{
			return NEED_MORE;
		}
		reader = bio_reader_create(this->value);
		reader->read_uint16(reader, &reserved);
		reader->read_uint16(reader, &this->count);
		this->offset = IETF_INSTALLED_PACKAGES_MIN_SIZE;
		this->value = reader->peek(reader);
		reader->destroy(reader);
	}

	reader = bio_reader_create(this->value);

	while (this->count)
	{
		if (!reader->read_data8(reader, &name) ||
			!reader->read_data8(reader, &version))
		{
			goto end;
		}
		pos = memchr(name.ptr, '\0', name.len);
		if (pos)
		{
			DBG1(DBG_TNC, "nul termination in IETF installed package name");
			*offset = this->offset + 1 + (pos - name.ptr);
			status = FAILED;
			goto end;
		}
		pos = memchr(version.ptr, '\0', version.len);
		if (pos)
		{
			DBG1(DBG_TNC, "nul termination in IETF installed package version");
			*offset = this->offset + 1 + name.len + 1 + (pos - version.ptr);
			status = FAILED;
			goto end;
		}
		this->offset += this->value.len - reader->remaining(reader);
		this->value = reader->peek(reader);

		entry = malloc_thing(package_entry_t);
		entry->name = chunk_clone(name);
		entry->version = chunk_clone(version);
		this->packages->insert_last(this->packages, entry);

		/* at least one tag ID was processed */
		status = SUCCESS;
		this->count--;
	}

	if (this->length != this->offset)
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
	private_ietf_attr_installed_packages_t *this, chunk_t segment)
{
	this->value = chunk_cat("cc", this->value, segment);
	chunk_free(&this->segment);
	this->segment = this->value;
}

METHOD(pa_tnc_attr_t, get_ref, pa_tnc_attr_t*,
	private_ietf_attr_installed_packages_t *this)
{
	ref_get(&this->ref);
	return &this->public.pa_tnc_attribute;
}

METHOD(pa_tnc_attr_t, destroy, void,
	private_ietf_attr_installed_packages_t *this)
{
	if (ref_put(&this->ref))
	{
		this->packages->destroy_function(this->packages, (void*)free_package_entry);
		free(this->segment.ptr);
		free(this);
	}
}

METHOD(ietf_attr_installed_packages_t, add, void,
	private_ietf_attr_installed_packages_t *this, chunk_t name, chunk_t version)
{
	package_entry_t *entry;

	/* restrict package name and package version number fields to 255 octets */
	name.len = min(255, name.len);
	version.len = min(255, version.len);

	entry = malloc_thing(package_entry_t);
	entry->name = chunk_clone(name);
	entry->version = chunk_clone(version);
	this->packages->insert_last(this->packages, entry);
}

CALLBACK(package_filter, bool,
	void *null, enumerator_t *orig, va_list args)
{
	package_entry_t *entry;
	chunk_t *name, *version;

	VA_ARGS_VGET(args, name, version);

	if (orig->enumerate(orig, &entry))
	{
		*name = entry->name;
		*version = entry->version;
		return TRUE;
	}
	return FALSE;
}

METHOD(ietf_attr_installed_packages_t, create_enumerator, enumerator_t*,
	private_ietf_attr_installed_packages_t *this)
{
	return enumerator_create_filter(
						this->packages->create_enumerator(this->packages),
						package_filter, NULL, NULL);
}

METHOD(ietf_attr_installed_packages_t, get_count, uint16_t,
	private_ietf_attr_installed_packages_t *this)
{
	return this->count;
}

METHOD(ietf_attr_installed_packages_t, clear_packages, void,
	private_ietf_attr_installed_packages_t *this)
{
	package_entry_t *entry;

	while (this->packages->remove_first(this->packages,(void**)&entry) == SUCCESS)
	{
		free_package_entry(entry);
	}
}

/**
 * Described in header.
 */
pa_tnc_attr_t *ietf_attr_installed_packages_create(void)
{
	private_ietf_attr_installed_packages_t *this;

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
			.create_enumerator = _create_enumerator,
			.get_count = _get_count,
			.clear_packages = _clear_packages,
		},
		.type = { PEN_IETF, IETF_ATTR_INSTALLED_PACKAGES },
		.packages = linked_list_create(),
		.ref = 1,
	);

	return &this->public.pa_tnc_attribute;
}

/**
 * Described in header.		.length = length,

 */
pa_tnc_attr_t *ietf_attr_installed_packages_create_from_data(size_t length,
															 chunk_t data)
{
	private_ietf_attr_installed_packages_t *this;

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
			.create_enumerator = _create_enumerator,
			.get_count = _get_count,
			.clear_packages = _clear_packages,
		},
		.type = {PEN_IETF, IETF_ATTR_INSTALLED_PACKAGES },
		.length = length,
		.segment = chunk_clone(data),
		.packages = linked_list_create(),
		.ref = 1,
	);

	/* received either complete attribute value or first segment */
	this->value = this->segment;

	return &this->public.pa_tnc_attribute;
}


