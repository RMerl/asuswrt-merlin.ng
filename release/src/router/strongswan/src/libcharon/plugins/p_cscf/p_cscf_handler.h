/*
 * Copyright (C) 2016 Tobias Brunner
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

/**
 * @defgroup p_cscf_handler p_cscf_handler
 * @{ @ingroup p_cscf
 */

#ifndef P_CSCF_HANDLER_H_
#define P_CSCF_HANDLER_H_

#include <attributes/attribute_handler.h>

typedef struct p_cscf_handler_t p_cscf_handler_t;

/**
 * Attribute handler for P-CSCF server addresses.
 */
struct p_cscf_handler_t {

	/**
	 * Implements attribute_handler_t.
	 */
	attribute_handler_t handler;

	/**
	 * Destroy a p_cscf_handler_t.
	 */
	void (*destroy)(p_cscf_handler_t *this);
};

/**
 * Create an p_cscf_handler_t instance.
 */
p_cscf_handler_t *p_cscf_handler_create();

#endif /** P_CSCF_HANDLER_H_ @}*/
