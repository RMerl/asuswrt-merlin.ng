/*
 * Copyright (C) 2012 Giuliano Grassi
 * Copyright (C) 2012 Ralf Sager
 * Copyright (C) 2012 Tobias Brunner
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

#include "ipsec.h"

#include <utils/debug.h>

typedef struct private_ipsec_t private_ipsec_t;

/**
 * Private additions to ipsec_t.
 */
struct private_ipsec_t {

	/**
	 * Public members of ipsec_t.
	 */
	ipsec_t public;
};

/**
 * Single instance of ipsec_t.
 */
ipsec_t *ipsec;

/**
 * Described in header.
 */
void libipsec_deinit()
{
	private_ipsec_t *this = (private_ipsec_t*)ipsec;
	DESTROY_IF(this->public.processor);
	DESTROY_IF(this->public.events);
	DESTROY_IF(this->public.policies);
	DESTROY_IF(this->public.sas);
	free(this);
	ipsec = NULL;
}

/**
 * Described in header.
 */
bool libipsec_init()
{
	private_ipsec_t *this;

	INIT(this);
	ipsec = &this->public;

	if (lib->integrity &&
		!lib->integrity->check(lib->integrity, "libipsec", libipsec_init))
	{
		DBG1(DBG_LIB, "integrity check of libipsec failed");
		return FALSE;
	}

	this->public.sas = ipsec_sa_mgr_create();
	this->public.policies = ipsec_policy_mgr_create();
	this->public.events = ipsec_event_relay_create();
	this->public.processor = ipsec_processor_create();
	return TRUE;
}

