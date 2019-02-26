/*
 * Copyright (C) 2017 Andreas Steffen
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

#define _GNU_SOURCE
#include <stdio.h>

#include "swid_gen.h"

#include <bio/bio_writer.h>

#define SWID_GENERATOR	"/usr/local/bin/swid_generator"

typedef struct private_swid_gen_t private_swid_gen_t;

/**
 * Private data of a swid_gen_t object.
 *
 */
struct private_swid_gen_t {

	/**
	 * Public swid_gen_t interface.
	 */
	swid_gen_t public;

	/**
	 * Path of the SWID generator command
	 */
	char *generator;

	/**
	 * Entity name of the tagCreator
	 */
	char *entity;

	/**
	 * Regid of the tagCreator
	 */
	char *regid;

};

METHOD(swid_gen_t, generate_tag, char*,
	private_swid_gen_t *this, char *sw_id, char *package, char *version,
	bool full, bool pretty)
{
	char *tag = NULL;
	size_t tag_buf_len = 8192;
	char tag_buf[tag_buf_len], command[BUF_LEN];
	bio_writer_t *writer;
	chunk_t swid_tag;
	FILE *file;

	/* Compose the SWID generator command */
	if (full || !package || !version)
	{ 
		snprintf(command, BUF_LEN, "%s swid --entity-name \"%s\" "
				 "--regid %s --software-id %s%s%s",
				 this->generator, this->entity, this->regid, sw_id,
				 full ? " --full" : "", pretty ? " --pretty" : "");
	}
	else
	{ 
		snprintf(command, BUF_LEN, "%s swid --entity-name \"%s\" "
				 "--regid %s --name %s --version-string %s%s",
				 this->generator, this->entity, this->regid, package,
				 version, pretty ? " --pretty" : "");
	}

	/* Open a pipe stream for reading the SWID generator output */
	file = popen(command, "r");
	if (file)
	{
		writer = bio_writer_create(tag_buf_len);
		while (TRUE)
		{
			if (!fgets(tag_buf, tag_buf_len, file))
			{
				break;
			}
			writer->write_data(writer, chunk_create(tag_buf, strlen(tag_buf)));
		}
		pclose(file);
		swid_tag = writer->extract_buf(writer);
		writer->destroy(writer);

		if (swid_tag.len > 0)
		{
			tag = swid_tag.ptr;
			tag[swid_tag.len - 1] = '\0';
		}
		else
		{
			chunk_free(&swid_tag);
		}
	}
	else
	{
		DBG1(DBG_IMC, "failed to run swid_generator command");
	}

	return tag;
}

typedef struct {
	/** public enumerator interface */
	enumerator_t public;
	/** swid_generator output stream */
	FILE *file;
	/** generate software identifier only */
	bool sw_id_only;
} swid_gen_enumerator_t;

METHOD(enumerator_t, enumerate, bool,
	swid_gen_enumerator_t *this, va_list args)
{
	chunk_t *out;

	VA_ARGS_VGET(args, out);

	if (this->sw_id_only)
	{
		char line[BUF_LEN];
		size_t len;

		if (!fgets(line, sizeof(line), this->file))
		{
			return FALSE;
		}
		len = strlen(line);

		if (len == 0)
		{
			return FALSE;
		}

		/* remove trailing newline if present */
		if (line[len - 1] == '\n')
		{
			len--;
		}
		DBG3(DBG_IMC, "  %.*s", len, line);
		*out = chunk_clone(chunk_create(line, len));
	}
	else
	{
		bool last_newline = TRUE;
		size_t len, line_len = 8192;
		char line[line_len];
		bio_writer_t *writer;
		chunk_t swid_tag;

		writer = bio_writer_create(line_len);
		while (TRUE)
		{
			if (!fgets(line, line_len, this->file))
			{
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
		swid_tag = writer->extract_buf(writer);
		writer->destroy(writer);

		if (swid_tag.len <= 1)
		{
			chunk_free(&swid_tag);
			return FALSE;
		}

		/* remove trailing newline if present */
		if (swid_tag.ptr[swid_tag.len - 1] == '\n')
		{
			swid_tag.len--;
		}
		DBG3(DBG_IMC, "  %.*s", swid_tag.len, swid_tag.ptr);
		*out = swid_tag;
	}

	return TRUE;
}

METHOD(enumerator_t, enumerator_destroy, void,
	swid_gen_enumerator_t *this)
{
	pclose(this->file);
	free(this);
}

METHOD(swid_gen_t, create_tag_enumerator, enumerator_t*,
	private_swid_gen_t *this, bool sw_id_only, bool full, bool pretty)
{
	swid_gen_enumerator_t *enumerator;
	char command[BUF_LEN];
	char doc_separator[] = "'\n\n'";
	FILE *file;

	/* Assemble the SWID generator command */
	if (sw_id_only)
	{
		snprintf(command, BUF_LEN, "%s software-id --regid %s ",
				 this->generator, this->regid);
	}
	else
	{
		snprintf(command, BUF_LEN, "%s swid --entity-name \"%s\" --regid %s "
				 "--doc-separator %s%s%s", this->generator, this->entity,
				 this->regid, doc_separator, pretty ? " --pretty" : "",
											 full   ? " --full"   : "");
	}

	/* Open a pipe stream for reading the SWID generator output */
	file = popen(command, "r");
	if (!file)
	{
		DBG1(DBG_IMC, "failed to run swid_generator command");
		return NULL;
	}

	INIT(enumerator,
		.public = {
			.enumerate = enumerator_enumerate_default,
			.venumerate = _enumerate,
			.destroy = _enumerator_destroy,
		},
		.sw_id_only = sw_id_only,
		.file = file,
	);

	return &enumerator->public;
}

METHOD(swid_gen_t, destroy, void,
	private_swid_gen_t *this)
{
	free(this->generator);
	free(this->entity);
	free(this->regid);
	free(this);
}

/**
 * See header
 */
swid_gen_t *swid_gen_create(void)
{
	private_swid_gen_t *this;
	char *entity, *regid, *generator;

	entity = lib->settings->get_str(lib->settings,
				"libimcv.swid_gen.tag_creator.name", "strongSwan Project");
	regid  = lib->settings->get_str(lib->settings,
				"libimcv.swid_gen.tag_creator.regid", "strongswan.org");
	generator = lib->settings->get_str(lib->settings,
				"libimcv.swid_gen.command", SWID_GENERATOR);

	INIT(this,
		.public = {
			.generate_tag = _generate_tag,
			.create_tag_enumerator = _create_tag_enumerator,
			.destroy = _destroy,
		},
		.generator = strdup(generator),
		.entity = strdup(entity),
		.regid = strdup(regid),
	);

	return &this->public;
}
