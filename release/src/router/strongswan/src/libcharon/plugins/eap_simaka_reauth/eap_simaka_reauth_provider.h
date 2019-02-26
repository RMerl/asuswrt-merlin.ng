/*
 * Copyright (C) 2009 Martin Willi
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
 * @defgroup eap_simaka_reauth_provider eap_simaka_reauth_provider
 * @{ @ingroup eap_simaka_reauth
 */

#ifndef EAP_SIMAKA_REAUTH_PROVIDER_H_
#define EAP_SIMAKA_REAUTH_PROVIDER_H_

#include <simaka_provider.h>

typedef struct eap_simaka_reauth_provider_t eap_simaka_reauth_provider_t;

/**
 * SIM provider implementing volatile in-memory reauthentication data storage.
 */
struct eap_simaka_reauth_provider_t {

	/**
	 * Implements simaka_provider_t interface.
	 */
	simaka_provider_t provider;

	/**
	 * Destroy a eap_simaka_reauth_provider_t.
	 */
	void (*destroy)(eap_simaka_reauth_provider_t *this);
};

/**
 * Create a eap_simaka_reauth_provider instance.
 */
eap_simaka_reauth_provider_t *eap_simaka_reauth_provider_create();

#endif /** EAP_SIMAKA_REAUTH_PROVIDER_H_ @}*/
