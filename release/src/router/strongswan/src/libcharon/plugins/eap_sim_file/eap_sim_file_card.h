/*
 * Copyright (C) 2008 Martin Willi
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
 * @defgroup eap_sim_file_card eap_sim_file_card
 * @{ @ingroup eap_sim_file
 */

#ifndef EAP_SIM_FILE_CARD_H_
#define EAP_SIM_FILE_CARD_H_

#include "eap_sim_file_triplets.h"

#include <simaka_card.h>

typedef struct eap_sim_file_card_t eap_sim_file_card_t;

/**
 * SIM card implementation on top of a triplet file.
 */
struct eap_sim_file_card_t {

	/**
	 * Implements simaka_card_t interface
	 */
	simaka_card_t card;

	/**
	 * Destroy a eap_sim_file_card_t.
	 */
	void (*destroy)(eap_sim_file_card_t *this);
};

/**
 * Create a eap_sim_file_card instance.
 *
 * @param triplets		source of triplets
 */
eap_sim_file_card_t *eap_sim_file_card_create(eap_sim_file_triplets_t *triplets);

#endif /** EAP_SIM_FILE_CARD_H_ @}*/
