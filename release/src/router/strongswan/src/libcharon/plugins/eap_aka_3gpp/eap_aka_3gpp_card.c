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
/*
 * Copyright (C) 2015 Thomas Strangert
 * Polystar System AB, Sweden
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "eap_aka_3gpp_card.h"

#include <daemon.h>

typedef struct private_eap_aka_3gpp_card_t private_eap_aka_3gpp_card_t;

/**
 * Private data of an eap_aka_3gpp_card_t object.
 */
struct private_eap_aka_3gpp_card_t {

	/**
	 * Public eap_aka_3gpp_card_t interface.
	 */
	eap_aka_3gpp_card_t public;

	/**
	 * AKA functions
	 */
	eap_aka_3gpp_functions_t *f;

	/**
	 * do sequence number checking?
	 */
	bool seq_check;

	/**
	 * SQN stored in this pseudo-USIM
	 */
	uint8_t sqn[AKA_SQN_LEN];
};

METHOD(simaka_card_t, get_quintuplet, status_t,
	private_eap_aka_3gpp_card_t *this, identification_t *id,
	char rand[AKA_RAND_LEN], char autn[AKA_AUTN_LEN], char ck[AKA_CK_LEN],
	char ik[AKA_IK_LEN], char res[AKA_RES_MAX], int *res_len)
{
	uint8_t *amf, *mac;
	uint8_t k[AKA_K_LEN], opc[AKA_OPC_LEN], ak[AKA_AK_LEN], sqn[AKA_SQN_LEN],
			xmac[AKA_MAC_LEN];

	if (!eap_aka_3gpp_get_k_opc(id, k, opc))
	{
		DBG1(DBG_IKE, "no EAP key found for %Y to authenticate with AKA", id);
		return FAILED;
	}
	DBG4(DBG_IKE, "EAP key found for id %Y, using K %b and OPc %b", id, k,
		 AKA_K_LEN, opc, AKA_OPC_LEN);

	/* AUTN = SQN xor AK | AMF | MAC */
	memcpy(sqn, autn, AKA_SQN_LEN);
	amf = autn + AKA_SQN_LEN;
	mac = autn + AKA_SQN_LEN + AKA_AMF_LEN;
	DBG3(DBG_IKE, "received AUTN %b", autn, AKA_AUTN_LEN);
	DBG3(DBG_IKE, "received AMF %b", amf, AKA_AMF_LEN);
	DBG3(DBG_IKE, "received MAC %b", mac, AKA_MAC_LEN);

	/* generate RES, CK, IK, AK from received RAND */
	DBG3(DBG_IKE, "received RAND %b", rand, AKA_RAND_LEN);
	if (!this->f->f2345(this->f, k, opc, rand, res, ck, ik, ak))
	{
		return FAILED;
	}
	*res_len = AKA_RES_LEN;
	DBG3(DBG_IKE, "using RES %b", res, AKA_RES_LEN);
	DBG3(DBG_IKE, "using CK %b", ck, AKA_CK_LEN);
	DBG3(DBG_IKE, "using IK %b", ik, AKA_IK_LEN);
	DBG3(DBG_IKE, "using AK %b", ak, AKA_AK_LEN);

	/* XOR anonymity key AK into SQN to decrypt it */
	memxor(sqn, ak, AKA_SQN_LEN);
	DBG3(DBG_IKE, "using SQN %b", sqn, AKA_SQN_LEN);

	/* calculate expected MAC and compare against received one */
	if (!this->f->f1(this->f, k, opc, rand, sqn, amf, xmac))
	{
		return FAILED;
	}
	if (!memeq_const(mac, xmac, AKA_MAC_LEN))
	{
		DBG1(DBG_IKE, "received MAC does not match XMAC");
		DBG3(DBG_IKE, "MAC %b\nXMAC %b", mac, AKA_MAC_LEN, xmac, AKA_MAC_LEN);
		return FAILED;
	}
	DBG3(DBG_IKE, "MAC equals XMAC %b", mac, AKA_MAC_LEN);

	if (this->seq_check && memcmp(this->sqn, sqn, AKA_SQN_LEN) >= 0)
	{
		DBG3(DBG_IKE, "received SQN %b\ncurrent SQN %b",
			 sqn, AKA_SQN_LEN, this->sqn, AKA_SQN_LEN);
		return INVALID_STATE;
	}

	/* update stored SQN to the received one */
	memcpy(this->sqn, sqn, AKA_SQN_LEN);

	return SUCCESS;
}

METHOD(simaka_card_t, resync, bool,
	private_eap_aka_3gpp_card_t *this, identification_t *id,
	char rand[AKA_RAND_LEN], char auts[AKA_AUTS_LEN])
{
	uint8_t amf[AKA_AMF_LEN], k[AKA_K_LEN], opc[AKA_OPC_LEN], aks[AKA_AK_LEN],
			macs[AKA_MAC_LEN];

	if (!eap_aka_3gpp_get_k_opc(id, k, opc))
	{
		DBG1(DBG_IKE, "no EAP key found for %Y to resync AKA", id);
		return FALSE;
	}
	DBG4(DBG_IKE, "EAP key found for id %Y, using K %b and OPc %b to resync AKA",
		 id, k, AKA_K_LEN, opc, AKA_OPC_LEN);

	/* AMF is set to zero in resync */
	memset(amf, 0, AKA_AMF_LEN);
	if (!this->f->f5star(this->f, k, opc, rand, aks) ||
	    !this->f->f1star(this->f, k, opc, rand, this->sqn, amf, macs))
	{
		return FALSE;
	}
	/* AUTS = SQN xor AKS | MACS */
	memcpy(auts, this->sqn, AKA_SQN_LEN);
	memxor(auts, aks, AKA_AK_LEN);
	memcpy(auts + AKA_AK_LEN, macs, AKA_MAC_LEN);
	DBG3(DBG_IKE, "generated AUTS %b", auts, AKA_AUTN_LEN);

	return TRUE;
}

METHOD(eap_aka_3gpp_card_t, destroy, void,
	private_eap_aka_3gpp_card_t *this)
{
	free(this);
}

/**
 * See header
 */
eap_aka_3gpp_card_t *eap_aka_3gpp_card_create(eap_aka_3gpp_functions_t *f)
{
	private_eap_aka_3gpp_card_t *this;

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
									"%s.plugins.eap-aka-3gpp.seq_check",
#ifdef SEQ_CHECK /* handle legacy compile time configuration as default */
									TRUE,
#else /* !SEQ_CHECK */
									FALSE,
#endif /* SEQ_CHECK */
									lib->ns),
	);

	eap_aka_3gpp_get_sqn(this->sqn, 0);

	return &this->public;
}
