/*
 * Copyright (C) 2012 Martin Willi
 * Copyright (C) 2012 revosec AG
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
 * @defgroup eap_radius_dae eap_radius_dae
 * @{ @ingroup eap_radius
 */

#ifndef EAP_RADIUS_DAE_H_
#define EAP_RADIUS_DAE_H_

#include "eap_radius_accounting.h"

typedef struct eap_radius_dae_t eap_radius_dae_t;

/**
 * Dynamic Authorization Extensions (RFC 5176) for EAP-RADIUS.
 */
struct eap_radius_dae_t {

	/**
	 * Destroy a eap_radius_dae_t.
	 */
	void (*destroy)(eap_radius_dae_t *this);
};

/**
 * Create a eap_radius_dae instance.
 */
eap_radius_dae_t *eap_radius_dae_create(eap_radius_accounting_t *accounting);

#endif /** EAP_RADIUS_DAE_H_ @}*/
