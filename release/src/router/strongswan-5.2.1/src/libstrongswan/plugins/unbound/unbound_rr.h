/*
 * Copyright (C) 2012 Reto Guadagnini
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

/**
 * @defgroup unbound_rr unbound_rr
 * @{ @ingroup unbound_p
 */

#ifndef UNBOUND_RR_H_
#define UNBOUND_RR_H_

#include <resolver/rr.h>
#include <ldns/ldns.h>

typedef struct unbound_rr_t unbound_rr_t;

/**
 * Implementation of the Resource Record interface using libunbound and libldns.
 */
struct unbound_rr_t {

	/**
	 * Implements the Resource Record interface
	 */
	rr_t interface;
};

/**
 * Create an unbound_rr instance from a Resource Record given by
 * a ldns_struct_rr from the ldns library.
 *
 * @return		Resource Record, NULL on error
 */
unbound_rr_t *unbound_rr_create_frm_ldns_rr(ldns_rr *rr);

#endif /** UNBOUND_RR_H_ @}*/
