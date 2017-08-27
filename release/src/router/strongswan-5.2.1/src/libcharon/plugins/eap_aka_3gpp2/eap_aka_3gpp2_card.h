/*
 * Copyright (C) 2008-2009 Martin Willi
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
 * @defgroup eap_aka_3gpp2_card eap_aka_3gpp2_card
 * @{ @ingroup eap_aka_3gpp2
 */

#ifndef EAP_AKA_3GPP2_CARD_H_
#define EAP_AKA_3GPP2_CARD_H_

#include "eap_aka_3gpp2_functions.h"

#include <simaka_card.h>

typedef struct eap_aka_3gpp2_card_t eap_aka_3gpp2_card_t;

/**
 * SIM card implementation using a set of AKA functions.
 */
struct eap_aka_3gpp2_card_t {

	/**
	 * Implements simaka_card_t interface
	 */
	simaka_card_t card;

	/**
	 * Destroy a eap_aka_3gpp2_card_t.
	 */
	void (*destroy)(eap_aka_3gpp2_card_t *this);
};

/**
 * Create a eap_aka_3gpp2_card instance.
 *
 * @param f		AKA functions
 */
eap_aka_3gpp2_card_t *eap_aka_3gpp2_card_create(eap_aka_3gpp2_functions_t *f);

#endif /** EAP_AKA_3GPP2_CARD_H_ @}*/
