/*
 * Copyright (C) 2014 Andreas Steffen
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

#include "imv_os_info.h"

typedef struct private_imv_os_info_t private_imv_os_info_t;

/**
 * Private data of an imv_os_info_t object.
 *
 */
struct private_imv_os_info_t {

	/**
	 * Public imv_os_info_t interface.
	 */
	imv_os_info_t public;

	/**
	 * OS type
	 */
	os_type_t type;

	/**
	 * OS name
	 */
	chunk_t name;

	/**
	 * OS version
	 */
	chunk_t version;

	/**
	 * This flag allows the OS version to be empty
	 */
	bool version_is_set;

	/**
	 * OS Product Information (OS Name | OS Version)
	 */
	char *info;

};

METHOD(imv_os_info_t, get_type, os_type_t,
	private_imv_os_info_t *this)
{
	return this->type;
}

METHOD(imv_os_info_t, set_name, void,
	private_imv_os_info_t *this, chunk_t name)
{
	/* Has the OS name already been set? */
	if (this->name.len)
	{
		if (chunk_equals(name, this->name))
		{
			return;
		}
		free(this->name.ptr);

		/* Also clear the OS info string */
		free(this->info);
		this->info = NULL;
	}
	this->name = chunk_clone(name);
	this->type = os_type_from_name(name); 
}

METHOD(imv_os_info_t, get_name, chunk_t,
	private_imv_os_info_t *this)
{
	return this->name;
}

METHOD(imv_os_info_t, set_version, void,
	private_imv_os_info_t *this, chunk_t version)
{
	/* Has the OS version already been set? */
	if (this->version_is_set)
	{
		if (chunk_equals(version, this->version))
		{
			return;
		}
		free(this->version.ptr);

		/* Also clear the OS info string */
		free(this->info);
		this->info = NULL;
	}
	this->version = chunk_clone(version);
	this->version_is_set = TRUE;
}

METHOD(imv_os_info_t, get_version, chunk_t,
	private_imv_os_info_t *this)
{
	return this->version;
}

METHOD(imv_os_info_t, get_info, char*,
	private_imv_os_info_t *this)
{
	int len;

	if (!this->info)
	{
		/* Have both OS name and OS version been set? */
		if (this->name.len == 0 || !this->version_is_set)
		{
			return NULL;
		}

		/* OS info is a concatenation of OS name and OS version */
		len = this->name.len + 1 + this->version.len + 1;
		this->info = malloc(len);
		snprintf(this->info, len, "%.*s %.*s",
				(int)this->name.len, this->name.ptr,
				(int)this->version.len, this->version.ptr);
	}
	return this->info;
}

METHOD(imv_os_info_t, destroy, void,
	private_imv_os_info_t *this)
{
	free(this->name.ptr);
	free(this->version.ptr);
	free(this->info);
	free(this);
}

/**
 * See header
 */
imv_os_info_t *imv_os_info_create(void)
{
	private_imv_os_info_t *this;

	INIT(this,
		.public = {
			.get_type = _get_type,
			.set_name = _set_name,
			.get_name = _get_name,
			.set_version = _set_version,
			.get_version = _get_version,
			.get_info = _get_info,
			.destroy = _destroy,
		},
	);

	return &this->public;
}
