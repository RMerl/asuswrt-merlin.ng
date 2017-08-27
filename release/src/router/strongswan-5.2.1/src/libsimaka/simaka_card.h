/*
 * Copyright (C) 2008-2011 Martin Willi
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
 * @defgroup simaka_card simaka_card
 * @{ @ingroup libsimaka
 */

#ifndef SIMAKA_CARD_H_
#define SIMAKA_CARD_H_

typedef struct simaka_card_t simaka_card_t;

#include "simaka_manager.h"

#include <utils/identification.h>

/**
 * Interface for a (U)SIM card (used as EAP client).
 *
 * The SIM card completes triplets/quintuplets requested in a challenge
 * received from the server.
 * An implementation supporting only one of SIM/AKA authentication may
 * implement the other methods with return_false()/return NOT_SUPPORTED/NULL.
 */
struct simaka_card_t {

	/**
	 * Calculate SRES/KC from a RAND for SIM authentication.
	 *
	 * @param id		permanent identity to get a triplet for
	 * @param rand		RAND input buffer, fixed size 16 bytes
	 * @param sres		SRES output buffer, fixed size 4 byte
	 * @param kc		KC output buffer, fixed size 8 bytes
	 * @return			TRUE if SRES/KC calculated, FALSE on error/wrong identity
	 */
	bool (*get_triplet)(simaka_card_t *this, identification_t *id,
						char rand[SIM_RAND_LEN], char sres[SIM_SRES_LEN],
						char kc[SIM_KC_LEN]);

	/**
	 * Calculate CK/IK/RES from RAND/AUTN for AKA authentication.
	 *
	 * If the received sequence number (in autn) is out of sync, INVALID_STATE
	 * is returned.
	 * The RES value is the only one with variable length. Pass a buffer
	 * of at least AKA_RES_MAX, the actual number of bytes is written to the
	 * res_len value. While the standard would allow any bit length between
	 * 32 and 128 bits, we support only full bytes for now.
	 *
	 * @param id		permanent identity to request quintuplet for
	 * @param rand		random value rand
	 * @param autn		authentication token autn
	 * @param ck		buffer receiving encryption key ck
	 * @param ik		buffer receiving integrity key ik
	 * @param res		buffer receiving authentication result res
	 * @param res_len	nubmer of bytes written to res buffer
	 * @return			SUCCESS, FAILED, or INVALID_STATE if out of sync
	 */
	status_t (*get_quintuplet)(simaka_card_t *this, identification_t *id,
							   char rand[AKA_RAND_LEN], char autn[AKA_AUTN_LEN],
							   char ck[AKA_CK_LEN], char ik[AKA_IK_LEN],
							   char res[AKA_RES_MAX], int *res_len);

	/**
	 * Calculate AUTS from RAND for AKA resynchronization.
	 *
	 * @param id		permanent identity to request quintuplet for
	 * @param rand		random value rand
	 * @param auts		resynchronization parameter auts
	 * @return			TRUE if parameter generated successfully
	 */
	bool (*resync)(simaka_card_t *this, identification_t *id,
				   char rand[AKA_RAND_LEN], char auts[AKA_AUTS_LEN]);

	/**
	 * Set the pseudonym to use for next authentication.
	 *
	 * @param id		permanent identity of the peer
	 * @param pseudonym	pseudonym identity received from the server
	 */
	void (*set_pseudonym)(simaka_card_t *this, identification_t *id,
						  identification_t *pseudonym);

	/**
	 * Get the pseudonym previously stored via set_pseudonym().
	 *
	 * @param id		permanent identity of the peer
	 * @return			associated pseudonym identity, NULL if none stored
	 */
	identification_t* (*get_pseudonym)(simaka_card_t *this, identification_t *id);

	/**
	 * Store parameters to use for the next fast reauthentication.
	 *
	 * @param id		permanent identity of the peer
	 * @param next		next fast reauthentication identity to use
	 * @param mk		master key MK to store for reauthentication
	 * @param counter	counter value to store, host order
	 */
	void (*set_reauth)(simaka_card_t *this, identification_t *id,
					   identification_t *next, char mk[HASH_SIZE_SHA1],
					   u_int16_t counter);

	/**
	 * Retrieve parameters for fast reauthentication stored via set_reauth().
	 *
	 * @param id		permanent identity of the peer
	 * @param mk		buffer receiving master key MK
	 * @param counter	pointer receiving counter value, in host order
	 * @return			fast reauthentication identity, NULL if not found
	 */
	identification_t* (*get_reauth)(simaka_card_t *this, identification_t *id,
									char mk[HASH_SIZE_SHA1], u_int16_t *counter);
};

#endif /** SIMAKA_CARD_H_ @}*/
