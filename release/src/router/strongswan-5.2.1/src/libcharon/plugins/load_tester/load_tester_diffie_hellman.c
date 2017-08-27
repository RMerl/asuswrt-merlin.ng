/*
 * Copyright (C) 2008 Martin Willi
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

#include "load_tester_diffie_hellman.h"

/**
 * Implementation of gmp_diffie_hellman_t.get_my_public_value.
 */
static void get_my_public_value(load_tester_diffie_hellman_t *this,
								chunk_t *value)
{
	*value = chunk_empty;
}

/**
 * Implementation of gmp_diffie_hellman_t.get_shared_secret.
 */
static status_t get_shared_secret(load_tester_diffie_hellman_t *this,
								  chunk_t *secret)
{
	*secret = chunk_empty;
	return SUCCESS;
}

/**
 * Implementation of gmp_diffie_hellman_t.get_dh_group.
 */
static diffie_hellman_group_t get_dh_group(load_tester_diffie_hellman_t *this)
{
	return MODP_NULL;
}

/**
 * See header
 */
load_tester_diffie_hellman_t *load_tester_diffie_hellman_create(
												diffie_hellman_group_t group)
{
	load_tester_diffie_hellman_t *this;

	if (group != MODP_NULL)
	{
		return NULL;
	}

	this = malloc_thing(load_tester_diffie_hellman_t);

	this->dh.get_shared_secret = (status_t (*)(diffie_hellman_t *, chunk_t *))get_shared_secret;
	this->dh.set_other_public_value = (void (*)(diffie_hellman_t *, chunk_t ))nop;
	this->dh.get_my_public_value = (void (*)(diffie_hellman_t *, chunk_t *))get_my_public_value;
	this->dh.get_dh_group = (diffie_hellman_group_t (*)(diffie_hellman_t *))get_dh_group;
	this->dh.destroy = (void (*)(diffie_hellman_t *))free;

	return this;
}
