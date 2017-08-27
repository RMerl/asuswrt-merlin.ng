/*
 * Copyright (C) 2010 Tobias Brunner
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

#include "hydra.h"

#include <utils/debug.h>

typedef struct private_hydra_t private_hydra_t;

/**
 * Private additions to hydra_t.
 */
struct private_hydra_t {

	/**
	 * Public members of hydra_t.
	 */
	hydra_t public;

	/**
	 * Integrity check failed?
	 */
	bool integrity_failed;

	/**
	 * Number of times we have been initialized
	 */
	refcount_t ref;
};

/**
 * Single instance of hydra_t.
 */
hydra_t *hydra = NULL;

/**
 * Described in header.
 */
void libhydra_deinit()
{
	private_hydra_t *this = (private_hydra_t*)hydra;

	if (!this || !ref_put(&this->ref))
	{	/* have more users */
		return;
	}

	this->public.attributes->destroy(this->public.attributes);
	this->public.kernel_interface->destroy(this->public.kernel_interface);
	free(this);
	hydra = NULL;
}

/**
 * Described in header.
 */
bool libhydra_init()
{
	private_hydra_t *this;

	if (hydra)
	{	/* already initialized, increase refcount */
		this = (private_hydra_t*)hydra;
		ref_get(&this->ref);
		return !this->integrity_failed;
	}

	INIT(this,
		.public = {
			.attributes = attribute_manager_create(),
		},
		.ref = 1,
	);
	hydra = &this->public;

	this->public.kernel_interface = kernel_interface_create();

	if (lib->integrity &&
		!lib->integrity->check(lib->integrity, "libhydra", libhydra_init))
	{
		DBG1(DBG_LIB, "integrity check of libhydra failed");
		this->integrity_failed = TRUE;
	}
	return !this->integrity_failed;
}
