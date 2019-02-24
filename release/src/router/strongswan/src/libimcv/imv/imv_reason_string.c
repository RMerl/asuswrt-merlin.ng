/*
 * Copyright (C) 2012 Andreas Steffen
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

#include "imv_reason_string.h"

#include <utils/debug.h>

typedef struct private_imv_reason_string_t private_imv_reason_string_t;

/**
 * Private data of an imv_reason_string_t object.
 */
struct private_imv_reason_string_t {

	/**
	 * Public members of imv_reason_string_t
	 */
	imv_reason_string_t public;

	/**
	 * Preferred language
	 */
	char *lang;

	/**
	 * Separator concatenating multiple reasons
	 */
	char *separator;

	/**
	 * Contains the concatenated reasons
	 */
	chunk_t reasons;

};

METHOD(imv_reason_string_t, add_reason, void,
	private_imv_reason_string_t *this, imv_lang_string_t reason[])
{
	char *s_reason;

	s_reason = imv_lang_string_select_string(reason, this->lang);

	if (this->reasons.len)
	{
		/* append any further reasons */
		this->reasons = chunk_cat("mcc", this->reasons,
								  chunk_from_str(this->separator),
								  chunk_create(s_reason, strlen(s_reason)));
	}
	else
	{
		/* add the first reason */
		this->reasons = chunk_clone(chunk_create(s_reason, strlen(s_reason)));
	}
}

METHOD(imv_reason_string_t, get_encoding, chunk_t,
	private_imv_reason_string_t *this)
{
	return this->reasons;
}

METHOD(imv_reason_string_t, destroy, void,
	private_imv_reason_string_t *this)
{
	free(this->reasons.ptr);
	free(this);
}

/**
 * Described in header.
 */
imv_reason_string_t *imv_reason_string_create(char *lang, char *separator)
{
	private_imv_reason_string_t *this;

	INIT(this,
		.public = {
			.add_reason = _add_reason,
			.get_encoding = _get_encoding,
			.destroy = _destroy,
		},
		.lang = lang,
		.separator = separator,
	);

	return &this->public;
}

