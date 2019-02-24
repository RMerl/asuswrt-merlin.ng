/*
 * Copyright (C) 2008-2009 Martin Willi
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

#include "eap_sim_file_provider.h"

#include <daemon.h>

typedef struct private_eap_sim_file_provider_t private_eap_sim_file_provider_t;

/**
 * Private data of an eap_sim_file_provider_t object.
 */
struct private_eap_sim_file_provider_t {

	/**
	 * Public eap_sim_file_provider_t interface.
	 */
	eap_sim_file_provider_t public;

	/**
	 * source of triplets
	 */
	eap_sim_file_triplets_t *triplets;
};

METHOD(simaka_provider_t, get_triplet, bool,
	 private_eap_sim_file_provider_t *this, identification_t *id,
	 char rand[SIM_RAND_LEN], char sres[SIM_SRES_LEN], char kc[SIM_KC_LEN])
{
	enumerator_t *enumerator;
	identification_t *cand;
	char *c_rand, *c_sres, *c_kc;

	enumerator = this->triplets->create_enumerator(this->triplets);
	while (enumerator->enumerate(enumerator, &cand, &c_rand, &c_sres, &c_kc))
	{
		if (id->matches(id, cand))
		{
			memcpy(rand, c_rand, SIM_RAND_LEN);
			memcpy(sres, c_sres, SIM_SRES_LEN);
			memcpy(kc, c_kc, SIM_KC_LEN);
			enumerator->destroy(enumerator);
			return TRUE;
		}
	}
	enumerator->destroy(enumerator);
	return FALSE;
}

METHOD(eap_sim_file_provider_t, destroy, void,
	private_eap_sim_file_provider_t *this)
{
	free(this);
}

/**
 * See header
 */
eap_sim_file_provider_t *eap_sim_file_provider_create(
											eap_sim_file_triplets_t *triplets)
{
	private_eap_sim_file_provider_t *this;

	INIT(this,
		.public = {
			.provider = {
				.get_triplet = _get_triplet,
				.get_quintuplet = (void*)return_false,
				.resync = (void*)return_false,
				.is_pseudonym = (void*)return_null,
				.gen_pseudonym = (void*)return_null,
				.is_reauth = (void*)return_null,
				.gen_reauth = (void*)return_null,
			},
			.destroy = _destroy,
		},
		.triplets = triplets,
	);

	return &this->public;
}

