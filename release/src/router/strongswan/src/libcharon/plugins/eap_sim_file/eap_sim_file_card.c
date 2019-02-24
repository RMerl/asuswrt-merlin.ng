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

#include "eap_sim_file_card.h"

#include <daemon.h>

typedef struct private_eap_sim_file_card_t private_eap_sim_file_card_t;

/**
 * Private data of an eap_sim_file_card_t object.
 */
struct private_eap_sim_file_card_t {

	/**
	 * Public eap_sim_file_card_t interface.
	 */
	eap_sim_file_card_t public;

	/**
	 * source of triplets
	 */
	eap_sim_file_triplets_t *triplets;
};

METHOD(simaka_card_t, get_triplet, bool,
	private_eap_sim_file_card_t *this, identification_t *id,
	char rand[SIM_RAND_LEN], char sres[SIM_SRES_LEN], char kc[SIM_KC_LEN])
{
	enumerator_t *enumerator;
	identification_t *cand;
	char *c_rand, *c_sres, *c_kc;

	DBG2(DBG_CFG, "looking for triplet: %Y rand %b", id, rand, SIM_RAND_LEN);

	enumerator = this->triplets->create_enumerator(this->triplets);
	while (enumerator->enumerate(enumerator, &cand, &c_rand, &c_sres, &c_kc))
	{
		DBG2(DBG_CFG, "got a triplet: %Y rand %b\nsres %b\n kc %b", cand,
			 c_rand, SIM_RAND_LEN, c_sres, SIM_SRES_LEN, c_kc, SIM_KC_LEN);
		if (id->matches(id, cand))
		{
			if (memeq_const(c_rand, rand, SIM_RAND_LEN))
			{
				DBG2(DBG_CFG, "  => triplet matches");
				memcpy(sres, c_sres, SIM_SRES_LEN);
				memcpy(kc, c_kc, SIM_KC_LEN);
				enumerator->destroy(enumerator);
				return TRUE;
			}
		}
	}
	enumerator->destroy(enumerator);
	return FALSE;
}

METHOD(simaka_card_t, get_quintuplet, status_t,
	private_eap_sim_file_card_t *this, identification_t *id,
	char rand[AKA_RAND_LEN], char autn[AKA_AUTN_LEN], char ck[AKA_CK_LEN],
	char ik[AKA_IK_LEN], char res[AKA_RES_MAX], int *res_len)
{
	return NOT_SUPPORTED;
}

METHOD(eap_sim_file_card_t, destroy, void,
	private_eap_sim_file_card_t *this)
{
	free(this);
}

/**
 * See header
 */
eap_sim_file_card_t *eap_sim_file_card_create(eap_sim_file_triplets_t *triplets)
{
	private_eap_sim_file_card_t *this;

	INIT(this,
		.public = {
			.card = {
				.get_triplet = _get_triplet,
				.get_quintuplet = _get_quintuplet,
				.resync = (void*)return_false,
				.get_pseudonym = (void*)return_null,
				.set_pseudonym = (void*)nop,
				.get_reauth = (void*)return_null,
				.set_reauth = (void*)nop,
			},
			.destroy = _destroy,
		},
		.triplets = triplets,
	);

	return &this->public;
}
