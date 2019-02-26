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

#include "swid_gen_info.h"

#include <library.h>
#include <utils/lexparser.h>

typedef struct private_swid_gen_info_t private_swid_gen_info_t;

/**
 * Private data of an swid_gen_info_t object.
 */
struct private_swid_gen_info_t {

	/**
	 * Public members of swid_gen_info_state_t
	 */
	swid_gen_info_t public;

	/**
	 * tagCreator
	 */
	char *tag_creator;

	/**
	 * OS string 'Name_Version-Arch'
	 */
	char *os;

	/**
	 * Product string 'Name Version Arch'
	 */
	char *product;

	/**
	 * OS info about endpoint
	 */
	imc_os_info_t *os_info;

};

/**
 * Replaces invalid character by a valid one
 */
static void sanitize_uri(char *uri, char a, char b)
{
	char *pos = uri;

	while (TRUE)
	{
		pos = strchr(pos, a);
		if (!pos)
		{
			break;
		}
		*pos = b;
		pos++;
	}
}

METHOD(swid_gen_info_t, get_os_type, os_type_t,
	private_swid_gen_info_t *this)
{
	return this->os_info->get_type(this->os_info);
}

METHOD(swid_gen_info_t, get_os, char*,
	private_swid_gen_info_t *this, char **product)
{
	if (product)
	{
		*product = this->product;
	}
	return this->os;
}

METHOD(swid_gen_info_t, create_sw_id, char*,
	private_swid_gen_info_t *this, char *package, char *version)
{
	char *sw_id;

	if (asprintf(&sw_id, "%s__%s-%s%s%s", this->tag_creator, this->os,
				 package, strlen(version) ? "-" : "", version) == -1)
	{
		return NULL;
	}
	sanitize_uri(sw_id, ':', '~');
	sanitize_uri(sw_id, '+', '~');

	return sw_id;
}

METHOD(swid_gen_info_t, destroy, void,
	private_swid_gen_info_t *this)
{
	this->os_info->destroy(this->os_info);
	free(this->os);
	free(this->product);
	free(this->tag_creator);
	free(this);
}

/**
 * Described in header.
 */
swid_gen_info_t *swid_gen_info_create(void)
{
	private_swid_gen_info_t *this;
	chunk_t os_name, os_version, os_arch;
	char *tag_creator;

	tag_creator = lib->settings->get_str(lib->settings,
					"libimcv.swid_gen.tag_creator.regid", "strongswan.org");

	INIT(this,
		.public = {
			.get_os_type = _get_os_type,
			.get_os = _get_os,
			.create_sw_id = _create_sw_id,
			.destroy = _destroy,
		},
		.os_info = imc_os_info_create(),
		.tag_creator = strdup(tag_creator),
	);

	os_name = this->os_info->get_name(this->os_info);
	os_arch = this->os_info->get_version(this->os_info);

	/* get_version() returns version followed by arch */
	if (!extract_token(&os_version, ' ', &os_arch))
	{
		DBG1(DBG_IMC, "separation of OS version from arch failed");
		destroy(this);
		return NULL;
	}

	/* construct OS string */
	if (asprintf(&this->os, "%.*s_%.*s-%.*s", (int)os_name.len, os_name.ptr,
				 (int)os_version.len, os_version.ptr, (int)os_arch.len,
				 os_arch.ptr) == -1)
	{
		DBG1(DBG_IMC, "constructon of OS string failed");
		destroy(this);
		return NULL;
	}

	/* construct product string */
	if (asprintf(&this->product, "%.*s %.*s %.*s", (int)os_name.len,
				 os_name.ptr, (int)os_version.len, os_version.ptr,
				 (int)os_arch.len, os_arch.ptr) == -1)
	{
		DBG1(DBG_IMC, "constructon of product string failed");
		destroy(this);
		return NULL;
	}

	return &this->public;
}
