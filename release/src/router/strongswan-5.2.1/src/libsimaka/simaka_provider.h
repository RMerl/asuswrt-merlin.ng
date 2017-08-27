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
 * @defgroup simaka_provider simaka_provider
 * @{ @ingroup libsimaka
 */

#ifndef SIMAKA_PROVIDER_H_
#define SIMAKA_PROVIDER_H_

typedef struct simaka_provider_t simaka_provider_t;

#include "simaka_manager.h"

#include <utils/identification.h>

/**
 * Interface for a triplet/quintuplet provider (used as EAP server).
 *
 * A SIM provider hands out triplets for SIM authentication and quintuplets
 * for AKA authentication. Multiple SIM provider instances can serve as
 * authentication backend to authenticate clients using SIM/AKA.
 * An implementation supporting only one of SIM/AKA authentication may
 * implement the other methods with return_false().
 */
struct simaka_provider_t {

	/**
	 * Create a challenge for SIM authentication.
	 *
	 * @param id		permanent identity of peer to gen triplet for
	 * @param rand		RAND output buffer, fixed size 16 bytes
	 * @param sres		SRES output buffer, fixed size 4 byte
	 * @param kc		KC output buffer, fixed size 8 bytes
	 * @return			TRUE if triplet received, FALSE otherwise
	 */
	bool (*get_triplet)(simaka_provider_t *this, identification_t *id,
						char rand[SIM_RAND_LEN], char sres[SIM_SRES_LEN],
						char kc[SIM_KC_LEN]);

	/**
	 * Create a challenge for AKA authentication.
	 *
	 * The XRES value is the only one with variable length. Pass a buffer
	 * of at least AKA_RES_MAX, the actual number of bytes is written to the
	 * xres_len value. While the standard would allow any bit length between
	 * 32 and 128 bits, we support only full bytes for now.
	 *
	 * @param id		permanent identity of peer to create challenge for
	 * @param rand		buffer receiving random value rand
	 * @param xres		buffer receiving expected authentication result xres
	 * @param xres_len	nubmer of bytes written to xres buffer
	 * @param ck		buffer receiving encryption key ck
	 * @param ik		buffer receiving integrity key ik
	 * @param autn		authentication token autn
	 * @return			TRUE if quintuplet generated successfully
	 */
	bool (*get_quintuplet)(simaka_provider_t *this, identification_t *id,
						   char rand[AKA_RAND_LEN],
						   char xres[AKA_RES_MAX], int *xres_len,
						   char ck[AKA_CK_LEN], char ik[AKA_IK_LEN],
						   char autn[AKA_AUTN_LEN]);

	/**
	 * Process AKA resynchroniusation request of a peer.
	 *
	 * @param id		permanent identity of peer requesting resynchronisation
	 * @param rand		random value rand
	 * @param auts		synchronization parameter auts
	 * @return			TRUE if resynchronized successfully
	 */
	bool (*resync)(simaka_provider_t *this, identification_t *id,
				   char rand[AKA_RAND_LEN], char auts[AKA_AUTS_LEN]);

	/**
	 * Check if peer uses a pseudonym, get permanent identity.
	 *
	 * @param id		pseudonym identity candidate
	 * @return			permanent identity, NULL if id not a pseudonym
	 */
	identification_t* (*is_pseudonym)(simaka_provider_t *this,
									  identification_t *id);

	/**
	 * Generate a pseudonym identitiy for a given peer identity.
	 *
	 * @param id		permanent identity to generate a pseudonym for
	 * @return			generated pseudonym, NULL to not use a pseudonym identity
	 */
	identification_t* (*gen_pseudonym)(simaka_provider_t *this,
									   identification_t *id);

	/**
	 * Check if peer uses reauthentication, retrieve reauth parameters.
	 *
	 * @param id		reauthentication identity (candidate)
	 * @param mk		buffer receiving master key MK
	 * @param counter	pointer receiving current counter value, host order
	 * @return			permanent identity, NULL if id not a reauth identity
	 */
	identification_t* (*is_reauth)(simaka_provider_t *this, identification_t *id,
								   char mk[HASH_SIZE_SHA1], u_int16_t *counter);

	/**
	 * Generate a fast reauthentication identity, associated to a master key.
	 *
	 * @param id		permanent peer identity
	 * @param mk		master key to store along with generated identity
	 * @return			fast reauthentication identity, NULL to not use reauth
	 */
	identification_t* (*gen_reauth)(simaka_provider_t *this, identification_t *id,
									char mk[HASH_SIZE_SHA1]);
};

#endif /** SIMAKA_CARD_H_ @}*/
