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

#include "eap_aka_3gpp_provider.h"

#include <daemon.h>

typedef struct private_eap_aka_3gpp_provider_t private_eap_aka_3gpp_provider_t;

/**
 * Private data of an eap_aka_3gpp_provider_t object.
 */
struct private_eap_aka_3gpp_provider_t {

	/**
	 * Public eap_aka_3gpp_provider_t interface.
	 */
	eap_aka_3gpp_provider_t public;

	/**
	 * AKA functions
	 */
	eap_aka_3gpp_functions_t *f;

	/**
	 * time based SQN, we use the same for all peers
	 */
	uint8_t sqn[AKA_SQN_LEN];
};

/** Authentication management field, AMF, as defined in 3GPP TS 33.102 V12.2.0
 *
 * The 16 bits in the AMF are numbered from "0" to "15" where bit "0" is
 * the most significant bit and bit "15" is the least significant bit.
 * Bit "0" is called the "AMF separation bit". It is used for the purposes
 * of EPS (Evolved Packet System) and is specified in
 * -	TS 33.401 [28] for E-UTRAN access to EPS;
 * -	TS 33.402 [29] for non-3GPP access to EPS.
 * Bits "1" to "7" are reserved for future standardization use.
 * Bits "1" to "7" shall be set to 0 while not yet specified for a particular use.
 * Bits "8" to "15" can be used for proprietary purposes.
 */
static const uint8_t amf[AKA_AMF_LEN] = {0x80, 0x00};

METHOD(simaka_provider_t, get_quintuplet, bool,
	private_eap_aka_3gpp_provider_t *this, identification_t *id,
	char rand[AKA_RAND_LEN], char xres[AKA_RES_MAX], int *xres_len,
	char ck[AKA_CK_LEN], char ik[AKA_IK_LEN], char autn[AKA_AUTN_LEN])
{
	rng_t *rng;
	uint8_t maca[AKA_MAC_LEN], ak[AKA_AK_LEN], k[AKA_K_LEN], opc[AKA_OPC_LEN];

	/* generate RAND: we use a RNG already registered as f0(). */
	rng = lib->crypto->create_rng(lib->crypto, RNG_WEAK);
	if (!rng || !rng->get_bytes(rng, AKA_RAND_LEN, rand))
	{
		DBG1(DBG_IKE, "generating RAND for AKA failed");
		DESTROY_IF(rng);
		return FALSE;
	}
	rng->destroy(rng);
	DBG3(DBG_IKE, "generated rand %b", rand, AKA_RAND_LEN);

	if (!eap_aka_3gpp_get_k_opc(id, k, opc))
	{
		DBG1(DBG_IKE, "no EAP key found for %Y to authenticate with AKA", id);
		return FALSE;
	}
	DBG4(DBG_IKE, "EAP key found for id %Y, using K %b and OPc %b", id, k,
		 AKA_K_LEN, opc, AKA_OPC_LEN);

	/* generate MAC and XRES, CK, IK, AK */
	if (!this->f->f1(this->f, k, opc, rand, this->sqn, amf, maca) ||
	    !this->f->f2345(this->f, k, opc, rand, xres, ck, ik, ak))
	{
		return FALSE;
	}
	*xres_len = AKA_RES_LEN;

	/* create AUTN = (SQN xor AK) || AMF || MAC */
	memcpy(autn, this->sqn, AKA_SQN_LEN);
	memxor(autn, ak, AKA_AK_LEN);
	memcpy(autn + AKA_SQN_LEN, amf, AKA_AMF_LEN);
	memcpy(autn + AKA_SQN_LEN + AKA_AMF_LEN, maca, AKA_MAC_LEN);
	DBG3(DBG_IKE, "AUTN %b", autn, AKA_AUTN_LEN);

	chunk_increment(chunk_create(this->sqn, AKA_SQN_LEN));

	return TRUE;
}

METHOD(simaka_provider_t, resync, bool,
	private_eap_aka_3gpp_provider_t *this, identification_t *id,
	char rand[AKA_RAND_LEN], char auts[AKA_AUTS_LEN])
{
	uint8_t *sqn, *macs;
	uint8_t aks[AKA_AK_LEN], k[AKA_K_LEN], opc[AKA_OPC_LEN], amfs[AKA_AMF_LEN],
			xmacs[AKA_MAC_LEN];

	if (!eap_aka_3gpp_get_k_opc(id, k, opc))
	{
		DBG1(DBG_IKE, "no EAP key found for %Y to authenticate with AKA", id);
		return FALSE;
	}
	DBG4(DBG_IKE, "EAP key found for id %Y, using K %b and OPc %b", id, k,
		 AKA_K_LEN, opc, AKA_OPC_LEN);

	/* get SQNms out of the AUTS the card created as:
	 * AUTS = (SQNms xor AKS) || MAC-S */
	sqn = auts;
	macs = auts + AKA_SQN_LEN;
	if (!this->f->f5star(this->f, k, opc, rand, aks))
	{
		return FALSE;
	}
	memxor(sqn, aks, AKA_AK_LEN);

	/* generate resync XMAC-S... */
	memset(amfs, 0, AKA_AMF_LEN);
	if (!this->f->f1star(this->f, k, opc, rand, sqn, amfs, xmacs))
	{
		return FALSE;
	}
	/* ...and compare it with the card's MAC-S */
	if (!memeq_const(xmacs, macs, AKA_MAC_LEN))
	{
		DBG1(DBG_IKE, "received MACS does not match XMACS");
		DBG3(DBG_IKE, "MACS %b XMACS %b",
			 macs, AKA_MAC_LEN, xmacs, AKA_MAC_LEN);
		return FALSE;
	}
	/* update stored SQN to received SQN + 1 */
	memcpy(this->sqn, sqn, AKA_SQN_LEN);
	chunk_increment(chunk_create(this->sqn, AKA_SQN_LEN));
	return TRUE;
}

METHOD(eap_aka_3gpp_provider_t, destroy, void,
	private_eap_aka_3gpp_provider_t *this)
{
	free(this);
}

/**
 * See header
 */
eap_aka_3gpp_provider_t *eap_aka_3gpp_provider_create(
												eap_aka_3gpp_functions_t *f)
{
	private_eap_aka_3gpp_provider_t *this;

	INIT(this,
		.public = {
			.provider = {
				.get_triplet = (void*)return_false,
				.get_quintuplet = _get_quintuplet,
				.resync = _resync,
				.is_pseudonym = (void*)return_null,
				.gen_pseudonym = (void*)return_null,
				.is_reauth = (void*)return_null,
				.gen_reauth = (void*)return_null,
			},
			.destroy = _destroy,
		},
		.f = f,
	);
	/* use an offset to accept clock skew between client/server without resync */
	eap_aka_3gpp_get_sqn(this->sqn, SQN_TIME_OFFSET);

	return &this->public;
}
