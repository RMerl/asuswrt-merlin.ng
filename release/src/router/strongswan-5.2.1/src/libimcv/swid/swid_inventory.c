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

#include "swid_inventory.h"
#include "swid_tag.h"
#include "swid_tag_id.h"

#include <collections/linked_list.h>
#include <bio/bio_writer.h>
#include <utils/debug.h>

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <libgen.h>
#include <errno.h>

typedef struct private_swid_inventory_t private_swid_inventory_t;

/**
 * Private data of a swid_inventory_t object.
 *
 */
struct private_swid_inventory_t {

	/**
	 * Public swid_inventory_t interface.
	 */
	swid_inventory_t public;

	/**
	 * Full SWID tags or just SWID tag IDs
	 */
	bool full_tags;

	/**
	 * List of SWID tags or tag IDs
	 */
	linked_list_t *list;
};

/**
 * Read SWID tags issued by the swid_generator tool
 */
static status_t read_swid_tags(private_swid_inventory_t *this, FILE *file)
{
	swid_tag_t *tag;
	bio_writer_t *writer;
	chunk_t tag_encoding, tag_file_path = chunk_empty;
	bool more_tags = TRUE, last_newline;
	char line[8192];
	size_t len;

	while (more_tags)
	{
		last_newline = TRUE;
		writer = bio_writer_create(512);
		while (TRUE)
		{
			if (!fgets(line, sizeof(line), file))
			{
				more_tags = FALSE;
				break;
			}
			len = strlen(line);

			if (last_newline && line[0] == '\n')
			{
				break;
			}
			else
			{
				last_newline = (line[len-1] == '\n');
				writer->write_data(writer, chunk_create(line, len));
			}
		}

		tag_encoding = writer->get_buf(writer);

		if (tag_encoding.len > 1)
		{
			/* remove trailing newline if present */
			if (tag_encoding.ptr[tag_encoding.len - 1] == '\n')
			{
				tag_encoding.len--;
			}
			DBG3(DBG_IMC, "  %.*s", tag_encoding.len, tag_encoding.ptr);

			tag = swid_tag_create(tag_encoding, tag_file_path);
			this->list->insert_last(this->list, tag);
		}
		writer->destroy(writer);
	}

	return SUCCESS;
}

/**
 * Read SWID tag or software IDs issued by the swid_generator tool
 */
static status_t read_swid_tag_ids(private_swid_inventory_t *this, FILE *file)
{
	swid_tag_id_t *tag_id;
	chunk_t tag_creator, unique_sw_id, tag_file_path = chunk_empty;
	char line[BUF_LEN];

	while (TRUE)
	{
		char *separator;
		size_t len;

		if (!fgets(line, sizeof(line), file))
		{
			return SUCCESS;
		}
		len = strlen(line);

		/* remove trailing newline if present */
		if (len > 0 && line[len - 1] == '\n')
		{
			len--;
		}
		DBG3(DBG_IMC, "  %.*s", len, line);

		separator = strchr(line, '_');
		if (!separator)
		{
			DBG1(DBG_IMC, "separation of regid from unique software ID failed");
			return FAILED;
		}
		tag_creator = chunk_create(line, separator - line);
		separator++;

		unique_sw_id = chunk_create(separator, len - (separator - line));
		tag_id = swid_tag_id_create(tag_creator, unique_sw_id, tag_file_path);
		this->list->insert_last(this->list, tag_id);
	}
}

static status_t generate_tags(private_swid_inventory_t *this, char *generator,
							  swid_inventory_t *targets, bool pretty, bool full)
{
	FILE *file;
	char command[BUF_LEN];
	char doc_separator[] = "'\n\n'";

	status_t status = SUCCESS;

	if (targets->get_count(targets) == 0)
	{
		/* Assemble the SWID generator command */
		if (this->full_tags)
		{
			snprintf(command, BUF_LEN, "%s swid --doc-separator %s%s%s",
					 generator, doc_separator, pretty ? " --pretty" : "",
											   full   ? " --full"   : "");
		}
		else
		{
			snprintf(command, BUF_LEN, "%s software-id", generator);
		}

		/* Open a pipe stream for reading the SWID generator output */
		file = popen(command, "r");
		if (!file)
		{
			DBG1(DBG_IMC, "failed to run swid_generator command");
			return NOT_SUPPORTED;
		}

		if (this->full_tags)
		{
			DBG2(DBG_IMC, "SWID tag generation by package manager");
			status = read_swid_tags(this, file);
		}
		else
		{
			DBG2(DBG_IMC, "SWID tag ID generation by package manager");
			status = read_swid_tag_ids(this, file);
		}
		pclose(file);
	}
	else if (this->full_tags)
	{
		swid_tag_id_t *tag_id;
		enumerator_t *enumerator;

		enumerator = targets->create_enumerator(targets);
		while (enumerator->enumerate(enumerator, &tag_id))
		{
			char software_id[BUF_LEN];
			chunk_t tag_creator, unique_sw_id;

			tag_creator  = tag_id->get_tag_creator(tag_id);
			unique_sw_id = tag_id->get_unique_sw_id(tag_id, NULL);
			snprintf(software_id, BUF_LEN, "%.*s_%.*s",
					 tag_creator.len, tag_creator.ptr,
					 unique_sw_id.len, unique_sw_id.ptr);

			/* Assemble the SWID generator command */
			snprintf(command, BUF_LEN, "%s swid --software-id %s%s%s",
					 generator, software_id, pretty ? " --pretty" : "",
											 full   ? " --full"   : "");

			/* Open a pipe stream for reading the SWID generator output */
			file = popen(command, "r");
			if (!file)
			{
				DBG1(DBG_IMC, "failed to run swid_generator command");
				return NOT_SUPPORTED;
			}
			status = read_swid_tags(this, file);
			pclose(file);

			if (status != SUCCESS)
			{
				break;
			}
		}
		enumerator->destroy(enumerator);
	}

	return status;
}

static bool collect_tags(private_swid_inventory_t *this, char *pathname,
						 swid_inventory_t *targets)
{
	char *rel_name, *abs_name;
	struct stat st;
	bool success = FALSE;
	enumerator_t *enumerator;

	enumerator = enumerator_create_directory(pathname);
	if (!enumerator)
	{
		DBG1(DBG_IMC, "directory '%s' can not be opened, %s",
			 pathname, strerror(errno));
		return FALSE;
	}
	DBG2(DBG_IMC, "entering %s", pathname);

	while (enumerator->enumerate(enumerator, &rel_name, &abs_name, &st))
	{
		char * start, *stop;
		chunk_t tag_creator;
		chunk_t unique_sw_id = chunk_empty, tag_file_path = chunk_empty;

		if (!strstr(rel_name, "regid."))
		{
			continue;
		}
		if (S_ISDIR(st.st_mode))
		{
			/* In case of a targeted request */
			if (targets->get_count(targets))
			{
				enumerator_t *target_enumerator;
				swid_tag_id_t *tag_id;
				bool match = FALSE;

				target_enumerator = targets->create_enumerator(targets);
				while (target_enumerator->enumerate(target_enumerator, &tag_id))
				{
					if (chunk_equals(tag_id->get_tag_creator(tag_id),
						chunk_from_str(rel_name)))
					{
						match = TRUE;
						break;
					}
				}
				target_enumerator->destroy(target_enumerator);

				if (!match)
				{
					continue;
				}
			}

			if (!collect_tags(this, abs_name, targets))
			{
				goto end;
			}
			continue;
		}

		/* parse the regid filename into its components */
		start = rel_name;
		stop = strchr(start, '_');
		if (!stop)
		{
			DBG1(DBG_IMC, "  %s", rel_name);
			DBG1(DBG_IMC, "  '_' separator not found");
			goto end;
		}
		tag_creator = chunk_create(start, stop-start);
		start = stop + 1;

		stop = strstr(start, ".swidtag");
		if (!stop)
		{
			DBG1(DBG_IMC, "  %s", rel_name);
			DBG1(DBG_IMC, "  swidtag postfix not found");
			goto end;
		}
		unique_sw_id = chunk_create(start, stop-start);
		tag_file_path = chunk_from_str(abs_name);

		/* In case of a targeted request */
		if (targets->get_count(targets))
		{
			chunk_t target_unique_sw_id, target_tag_creator;
			enumerator_t *target_enumerator;
			swid_tag_id_t *tag_id;
			bool match = FALSE;

			target_enumerator = targets->create_enumerator(targets);
			while (target_enumerator->enumerate(target_enumerator, &tag_id))
			{
				target_unique_sw_id = tag_id->get_unique_sw_id(tag_id, NULL);
				target_tag_creator  = tag_id->get_tag_creator(tag_id);

				if (chunk_equals(target_unique_sw_id, unique_sw_id) &&
					chunk_equals(target_tag_creator, tag_creator))
				{
					match = TRUE;
					break;
				}
			}
			target_enumerator->destroy(target_enumerator);

			if (!match)
			{
				continue;
			}
		}
		DBG2(DBG_IMC, "  %s", rel_name);

		if (this->full_tags)
		{
			swid_tag_t *tag;
			chunk_t *xml_tag;

			xml_tag = chunk_map(abs_name, FALSE);
			if (!xml_tag)
			{
				DBG1(DBG_IMC, "  opening '%s' failed: %s", abs_name,
					 strerror(errno));
				goto end;
			}

			tag = swid_tag_create(*xml_tag, tag_file_path);
			this->list->insert_last(this->list, tag);
			chunk_unmap(xml_tag);
		}
		else
		{
			swid_tag_id_t *tag_id;

			tag_id = swid_tag_id_create(tag_creator, unique_sw_id, tag_file_path);
			this->list->insert_last(this->list, tag_id);
		}
	}
	success = TRUE;

end:
	enumerator->destroy(enumerator);
	DBG2(DBG_IMC, "leaving %s", pathname);

	return success;
}

METHOD(swid_inventory_t, collect, bool,
	private_swid_inventory_t *this, char *directory, char *generator,
	swid_inventory_t *targets, bool pretty, bool full)
{
	/**
	 * Tags are generated by a package manager
	 */
	generate_tags(this, generator, targets, pretty, full);

	/**
	 * Collect swidtag files by iteratively entering all directories in
	 * the tree under the "directory" path.
	 */
	return collect_tags(this, directory, targets);
}

METHOD(swid_inventory_t, add, void,
	private_swid_inventory_t *this, void *item)
{
	this->list->insert_last(this->list, item);
}

METHOD(swid_inventory_t, get_count, int,
	private_swid_inventory_t *this)
{
	return this->list->get_count(this->list);
}

METHOD(swid_inventory_t, create_enumerator, enumerator_t*,
	private_swid_inventory_t *this)
{
	return this->list->create_enumerator(this->list);
}

METHOD(swid_inventory_t, destroy, void,
	private_swid_inventory_t *this)
{
	if (this->full_tags)
	{
		this->list->destroy_offset(this->list, offsetof(swid_tag_t, destroy));
	}
	else
	{
		this->list->destroy_offset(this->list, offsetof(swid_tag_id_t, destroy));
	}
	free(this);
}

/**
 * See header
 */
swid_inventory_t *swid_inventory_create(bool full_tags)
{
	private_swid_inventory_t *this;

	INIT(this,
		.public = {
			.collect = _collect,
			.add = _add,
			.get_count = _get_count,
			.create_enumerator = _create_enumerator,
			.destroy = _destroy,
		},
		.full_tags = full_tags,
		.list = linked_list_create(),
	);

	return &this->public;
}
