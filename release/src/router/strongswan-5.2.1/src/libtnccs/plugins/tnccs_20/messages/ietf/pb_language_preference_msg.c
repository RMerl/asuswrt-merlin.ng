/*
 * Copyright (C) 2010 Sansar Choinyambuu
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

#include "pb_language_preference_msg.h"

#include <bio/bio_writer.h>
#include <bio/bio_reader.h>
#include <utils/debug.h>

typedef struct private_pb_language_preference_msg_t private_pb_language_preference_msg_t;

/**
 *   PB-Language-Preference message (see section 4.10 of RFC 5793)
 *
 *      0                   1                   2                   3
 *      0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *     |              Language Preference (Variable Length)            |
 *     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

#define PB_LANG_PREFIX			"Accept-Language: "
#define PB_LANG_PREFIX_LEN		strlen(PB_LANG_PREFIX)

/**
 * Private data of a pb_language_preference_msg_t object.
 *
 */
struct private_pb_language_preference_msg_t {
	/**
	 * Public pb_access_recommendation_msg_t interface.
	 */
	pb_language_preference_msg_t public;

	/**
	 * PB-TNC message type
	 */
	pen_type_t type;

	/**
	 * Language preference
	 */
	chunk_t language_preference;

	/**
	 * Encoded message
	 */
	chunk_t encoding;
};

METHOD(pb_tnc_msg_t, get_type, pen_type_t,
	private_pb_language_preference_msg_t *this)
{
	return this->type;
}

METHOD(pb_tnc_msg_t, get_encoding, chunk_t,
	private_pb_language_preference_msg_t *this)
{
	return this->encoding;
}

METHOD(pb_tnc_msg_t, build, void,
	private_pb_language_preference_msg_t *this)
{
	if (this->encoding.ptr)
	{
		return;
	}
	this->encoding = chunk_cat("cc",
		 	 			chunk_create(PB_LANG_PREFIX, PB_LANG_PREFIX_LEN),
						this->language_preference);
}

METHOD(pb_tnc_msg_t, process, status_t,
	private_pb_language_preference_msg_t *this, u_int32_t *offset)
{
	chunk_t lang;

	if (this->encoding.len >= PB_LANG_PREFIX_LEN &&
		memeq(this->encoding.ptr, PB_LANG_PREFIX, PB_LANG_PREFIX_LEN))
	{
		lang = chunk_skip(this->encoding, PB_LANG_PREFIX_LEN);
		this->language_preference = lang.len ? chunk_clone(lang) : chunk_empty;
	}
	else
    {
		DBG1(DBG_TNC, "language preference must be preceded by '%s'",
					   PB_LANG_PREFIX);
		*offset = 0;
		return FAILED;
	}

	if (this->language_preference.len &&
		this->language_preference.ptr[this->language_preference.len-1] == '\0')
	{
		DBG1(DBG_TNC, "language preference must not be null terminated");
		*offset = PB_LANG_PREFIX_LEN + this->language_preference.len - 1;
		return FAILED;
	}

	return SUCCESS;
}

METHOD(pb_tnc_msg_t, destroy, void,
	private_pb_language_preference_msg_t *this)
{
	free(this->encoding.ptr);
	free(this->language_preference.ptr);
	free(this);
}

METHOD(pb_language_preference_msg_t, get_language_preference, chunk_t,
	private_pb_language_preference_msg_t *this)
{
	return this->language_preference;
}

/**
 * See header
 */
pb_tnc_msg_t *pb_language_preference_msg_create_from_data(chunk_t data)
{
	private_pb_language_preference_msg_t *this;

	INIT(this,
		.public = {
			.pb_interface = {
				.get_type = _get_type,
				.get_encoding = _get_encoding,
				.build = _build,
				.process = _process,
				.destroy = _destroy,
			},
			.get_language_preference = _get_language_preference,
		},
		.type = { PEN_IETF, PB_MSG_LANGUAGE_PREFERENCE },
		.encoding = chunk_clone(data),
	);

	return &this->public.pb_interface;
}

/**
 * See header
 */
pb_tnc_msg_t *pb_language_preference_msg_create(chunk_t language_preference)
{
	private_pb_language_preference_msg_t *this;

	INIT(this,
		.public = {
			.pb_interface = {
				.get_type = _get_type,
				.get_encoding = _get_encoding,
				.build = _build,
				.process = _process,
				.destroy = _destroy,
			},
			.get_language_preference = _get_language_preference,
		},
		.type = { PEN_IETF, PB_MSG_LANGUAGE_PREFERENCE },
		.language_preference = chunk_clone(language_preference),
	);

	return &this->public.pb_interface;
}
