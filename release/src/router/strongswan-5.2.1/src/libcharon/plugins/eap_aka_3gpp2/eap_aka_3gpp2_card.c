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

#include "eap_aka_3gpp2_card.h"

#include <daemon.h>

typedef struct private_eap_aka_3gpp2_card_t private_eap_aka_3gpp2_card_t;

/**
 * Private data of an eap_aka_3gpp2_card_t object.
 */
struct private_eap_aka_3gpp2_card_t {

	/**
	 * Public eap_aka_3gpp2_card_t interface.
	 */
	eap_aka_3gpp2_card_t public;

	/**
	 * AKA functions
	 */
	eap_aka_3gpp2_functions_t *f;

	/**
	 * do sequence number checking?
	 */
	bool seq_check;

	/**
	 * SQN stored in this pseudo-USIM
	 */
	char sqn[AKA_SQN_LEN];
};

/**
 * Functions from eap_aka_3gpp2_provider.c
 */
bool eap_aka_3gpp2_get_k(identification_t *id, char k[AKA_K_LEN]);
void eap_aka_3gpp2_get_sqn(char sqn[AKA_SQN_LEN], int offset);

METHOD(simaka_card_t, get_quintuplet, status_t,
	private_eap_aka_3gpp2_card_t *this, identification_t *id,
	char rand[AKA_RAND_LEN], char autn[AKA_AUTN_LEN], char ck[AKA_CK_LEN],
	char ik[AKA_IK_LEN], char res[AKA_RES_MAX], int *res_len)
{
	char *amf, *mac;
	char k[AKA_K_LEN], ak[AKA_AK_LEN], sqn[AKA_SQN_LEN], xmac[AKA_MAC_LEN];

	if (!eap_aka_3gpp2_get_k(id, k))
	{
		DBG1(DBG_IKE, "no EAP key found for %Y to authenticate with AKA", id);
		return FAILED;
	}

	/* AUTN = SQN xor AK | AMF | MAC */
	DBG3(DBG_IKE, "received autn %b", autn, AKA_AUTN_LEN);
	DBG3(DBG_IKE, "using K %b", k, AKA_K_LEN);
	DBG3(DBG_IKE, "using rand %b", rand, AKA_RAND_LEN);
	memcpy(sqn, autn, AKA_SQN_LEN);
	amf = autn + AKA_SQN_LEN;
	mac = autn + AKA_SQN_LEN + AKA_AMF_LEN;

	/* XOR anonymity key AK into SQN to decrypt it */
	if (!this->f->f5(this->f, k, rand, ak))
	{
		return FAILED;
	}
	DBG3(DBG_IKE, "using ak %b", ak, AKA_AK_LEN);
	memxor(sqn, ak, AKA_SQN_LEN);
	DBG3(DBG_IKE, "using sqn %b", sqn, AKA_SQN_LEN);

	/* calculate expected MAC and compare against received one */
	if (!this->f->f1(this->f, k, rand, sqn, amf, xmac))
	{
		return FAILED;
	}
	if (!memeq(mac, xmac, AKA_MAC_LEN))
	{
		DBG1(DBG_IKE, "received MAC does not match XMAC");
		DBG3(DBG_IKE, "MAC %b\nXMAC %b", mac, AKA_MAC_LEN, xmac, AKA_MAC_LEN);
		return FAILED;
	}

	if (this->seq_check && memcmp(this->sqn, sqn, AKA_SQN_LEN) >= 0)
	{
		DBG3(DBG_IKE, "received SQN %b\ncurrent SQN %b",
			 sqn, AKA_SQN_LEN, this->sqn, AKA_SQN_LEN);
		return INVALID_STATE;
	}

	/* update stored SQN to the received one */
	memcpy(this->sqn, sqn, AKA_SQN_LEN);

	/* CK/IK, calculate RES */
	if (!this->f->f3(this->f, k, rand, ck) ||
		!this->f->f4(this->f, k, rand, ik) ||
		!this->f->f2(this->f, k, rand, res))
	{
		return FAILED;
	}
	*res_len = AKA_RES_MAX;

	return SUCCESS;
}

METHOD(simaka_card_t, resync, bool,
	private_eap_aka_3gpp2_card_t *this, identification_t *id,
	char rand[AKA_RAND_LEN], char auts[AKA_AUTS_LEN])
{
	char amf[AKA_AMF_LEN], k[AKA_K_LEN], aks[AKA_AK_LEN], macs[AKA_MAC_LEN];

	if (!eap_aka_3gpp2_get_k(id, k))
	{
		DBG1(DBG_IKE, "no EAP key found for %Y to resync AKA", id);
		return FALSE;
	}

	/* AMF is set to zero in resync */
	memset(amf, 0, AKA_AMF_LEN);
	if (!this->f->f5star(this->f, k, rand, aks) ||
		!this->f->f1star(this->f, k, rand, this->sqn, amf, macs))
	{
		return FALSE;
	}
	/* AUTS = SQN xor AKS | MACS */
	memcpy(auts, this->sqn, AKA_SQN_LEN);
	memxor(auts, aks, AKA_AK_LEN);
	memcpy(auts + AKA_AK_LEN, macs, AKA_MAC_LEN);

	return TRUE;
}

METHOD(eap_aka_3gpp2_card_t, destroy, void,
	private_eap_aka_3gpp2_card_t *this)
{
	free(this);
}

/**
 * See header
 */
eap_aka_3gpp2_card_t *eap_aka_3gpp2_card_create(eap_aka_3gpp2_functions_t *f)
{
	private_eap_aka_3gpp2_card_t *this;

	INIT(this,
		.public = {
			.card = {
				.get_triplet = (void*)return_false,
				.get_quintuplet = _get_quintuplet,
				.resync = _resync,
				.get_pseudonym = (void*)return_null,
				.set_pseudonym = (void*)nop,
				.get_reauth = (void*)return_null,
				.set_reauth = (void*)nop,
			},
			.destroy = _destroy,
		},
		.f = f,
		.seq_check = lib->settings->get_bool(lib->settings,
									"%s.plugins.eap-aka-3gpp2.seq_check",
#ifdef SEQ_CHECK /* handle legacy compile time configuration as default */
									TRUE,
#else /* !SEQ_CHECK */
									FALSE,
#endif /* SEQ_CHECK */
									lib->ns),
	);

	eap_aka_3gpp2_get_sqn(this->sqn, 0);

	return &this->public;
}

