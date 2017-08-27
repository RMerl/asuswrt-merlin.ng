/*
 * Copyright (C) 2011-2012 Reto Guadagnini
 * Hochschule fuer Technik Rapperswil
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

#include "resolver_manager.h"

#include <utils/debug.h>

typedef struct private_resolver_manager_t private_resolver_manager_t;

/**
 * private data of resolver_manager
 */
struct private_resolver_manager_t {

	/**
	 * public functions
	 */
	resolver_manager_t public;

	/**
	 * constructor function to create resolver instances
	 */
	resolver_constructor_t constructor;
};

METHOD(resolver_manager_t, add_resolver, void,
	private_resolver_manager_t *this, resolver_constructor_t constructor)
{
	if (!this->constructor)
	{
		this->constructor = constructor;
	}
}

METHOD(resolver_manager_t, remove_resolver, void,
	private_resolver_manager_t *this, resolver_constructor_t constructor)
{
	if (this->constructor == constructor)
	{
		this->constructor = NULL;
	}
}

METHOD(resolver_manager_t, create, resolver_t*,
	private_resolver_manager_t *this)
{
	if (this->constructor)
	{
		return this->constructor();
	}
	return NULL;
}

METHOD(resolver_manager_t, destroy, void,
	private_resolver_manager_t *this)
{
	free(this);
}

/*
 * See header
 */
resolver_manager_t *resolver_manager_create()
{
	private_resolver_manager_t *this;

	INIT(this,
			.public = {
				.add_resolver = _add_resolver,
				.remove_resolver = _remove_resolver,
				.create = _create,
				.destroy = _destroy,
			},
	);

	return &this->public;
}

