/*
 * Copyright (C) 2012 Reto Guadagnini
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

#include "rr_set.h"

#include <library.h>
#include <utils/debug.h>

typedef struct private_rr_set_t private_rr_set_t;

/**
* private data of the rr_set
*/
struct private_rr_set_t {

	/**
	 * public functions
	 */
	rr_set_t public;

	/**
	 * List of Resource Records which form the RRset
	 */
	linked_list_t *rr_list;

	/**
	 * List of the signatures (RRSIGs) of the Resource Records contained in
	 * this set
	 */
	linked_list_t *rrsig_list;
};

METHOD(rr_set_t, create_rr_enumerator, enumerator_t*,
	private_rr_set_t *this)
{
	return this->rr_list->create_enumerator(this->rr_list);
}

METHOD(rr_set_t, create_rrsig_enumerator, enumerator_t*,
	private_rr_set_t *this)
{
	if (this->rrsig_list)
	{
		return this->rrsig_list->create_enumerator(this->rrsig_list);
	}
	return NULL;
}

METHOD(rr_set_t, destroy, void,
	private_rr_set_t *this)
{
	this->rr_list->destroy_offset(this->rr_list,
								  offsetof(rr_t, destroy));
	if (this->rrsig_list)
	{
		this->rrsig_list->destroy_offset(this->rrsig_list,
										 offsetof(rr_t, destroy));
	}
	free(this);
}

/*
 * see header
 */
rr_set_t *rr_set_create(linked_list_t *list_of_rr, linked_list_t *list_of_rrsig)
{
	private_rr_set_t *this;

	INIT(this,
		.public = {
			.create_rr_enumerator = _create_rr_enumerator,
			.create_rrsig_enumerator = _create_rrsig_enumerator,
			.destroy = _destroy,
		},
	);

	if (list_of_rr == NULL)
	{
		DBG1(DBG_LIB, "could not create a rr_set without a list_of_rr");
		_destroy(this);
		return NULL;
	}
	this->rr_list = list_of_rr;
	this->rrsig_list = list_of_rrsig;

	return &this->public;
}

