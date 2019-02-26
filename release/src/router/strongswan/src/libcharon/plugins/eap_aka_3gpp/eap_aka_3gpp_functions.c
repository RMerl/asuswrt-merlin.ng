/*
 * Copyright (C) 2017 Tobias Brunner
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

#include "eap_aka_3gpp_functions.h"

#include <limits.h>
#include <ctype.h>
#include <daemon.h>

typedef struct private_eap_aka_3gpp_functions_t private_eap_aka_3gpp_functions_t;

/**
 * Private data of an eap_aka_3gpp_functions_t object.
 */
struct private_eap_aka_3gpp_functions_t {

	/**
	 * Public eap_aka_3gpp_functions_t interface.
	 */
	eap_aka_3gpp_functions_t public;

	/**
	 * AES instance
	 */
	crypter_t *crypter;
};

/*
 * Described in header
 */
bool eap_aka_3gpp_get_k_opc(identification_t *id, uint8_t k[AKA_K_LEN],
							uint8_t opc[AKA_OPC_LEN])
{
	shared_key_t *shared;
	chunk_t key;

	shared = lib->credmgr->get_shared(lib->credmgr, SHARED_EAP, id, NULL);
	if (!shared)
	{
		return FALSE;
	}
	key = shared->get_key(shared);

	if (key.len == AKA_K_LEN)
	{
		memcpy(k, key.ptr, AKA_K_LEN);
		/* set OPc to a neutral default value, harmless to XOR with */
		memset(opc, '\0', AKA_OPC_LEN);
	}
	else if (key.len == AKA_K_LEN + AKA_OPC_LEN)
	{
		memcpy(k, key.ptr, AKA_K_LEN);
		memcpy(opc, key.ptr + AKA_K_LEN, AKA_OPC_LEN);
	}
	else
	{
		DBG1(DBG_IKE, "invalid EAP K or K+OPc key found for %Y to authenticate "
			 "with AKA, should be a %d or %d byte long binary value", id,
			 AKA_K_LEN, AKA_K_LEN + AKA_OPC_LEN);
		shared->destroy(shared);
		return FALSE;
	}
	shared->destroy(shared);
	return TRUE;
}

/*
 * Described in header
 */
void eap_aka_3gpp_get_sqn(uint8_t sqn[AKA_SQN_LEN], int offset)
{
	timeval_t time;

	gettimeofday(&time, NULL);
	/* set sqn to an integer containing 4 bytes seconds + 2 bytes usecs */
	time.tv_sec = htonl(time.tv_sec + offset);
	/* usec's are never larger than 0x000f423f, so we shift the 12 first bits */
	time.tv_usec = htonl(time.tv_usec << 12);
	memcpy(sqn, (uint8_t*)&time.tv_sec + sizeof(time_t) - 4, 4);
	memcpy(sqn + 4, &time.tv_usec, 2);
}

static bool f1andf1star(private_eap_aka_3gpp_functions_t *this,
	const uint8_t k[AKA_K_LEN], const uint8_t opc[AKA_OPC_LEN],
	const uint8_t rand[AKA_RAND_LEN], const uint8_t sqn[AKA_SQN_LEN],
	const uint8_t amf[AKA_AMF_LEN], uint8_t mac[16])
{
	uint8_t i, data[16], in[16], iv[16] = { 0 };

	if (!this->crypter->set_key(this->crypter,
								chunk_create((uint8_t*)k, AKA_K_LEN)))
	{
		return FALSE;
	}

	/* XOR RAND and OPc */
	memcpy(data, rand, sizeof(data));
	memxor(data, opc, sizeof(data));
	if (!this->crypter->encrypt(this->crypter, chunk_create(data, sizeof(data)),
								chunk_create(iv, sizeof(iv)), NULL))
	{
		return FALSE;
	}

	/* concatenate SQN || AMF ||SQN || AMF */
	memcpy(in, sqn, 6);
	memcpy(&in[6], amf, 2);
	memcpy(&in[8], in, 8);

	/* XOR opc and in, rotate by r1=64, and XOR
	 * on the constant c1 (which is all zeroes) and finally the output above */
	for (i = 0; i < 16; i++)
	{
		data[(i + 8) % 16] ^= in[i] ^ opc[i];
	}
	if (!this->crypter->encrypt(this->crypter, chunk_create(data, sizeof(data)),
								chunk_create(iv, sizeof(iv)), NULL))
	{
		return FALSE;
	}
	memxor(data, opc, sizeof(data));
	memcpy(mac, data, 16);
	return TRUE;
}

METHOD(eap_aka_3gpp_functions_t, f1, bool,
	private_eap_aka_3gpp_functions_t *this,	const uint8_t k[AKA_K_LEN],
	const uint8_t opc[AKA_OPC_LEN],	const uint8_t rand[AKA_RAND_LEN],
	const uint8_t sqn[AKA_SQN_LEN],	const uint8_t amf[AKA_AMF_LEN],
	uint8_t maca[AKA_MAC_LEN])
{
	uint8_t mac[16];

	if (!f1andf1star(this, k, opc, rand, sqn, amf, mac))
	{
		return FALSE;
	}
	/* only diff between f1 and f1* is here:
	 * f1  uses bytes 0-7  as MAC-A
	 * f1* uses bytes 8-15 as MAC-S */
	memcpy(maca, mac, AKA_MAC_LEN);
	return TRUE;
}

METHOD(eap_aka_3gpp_functions_t, f1star, bool,
	private_eap_aka_3gpp_functions_t *this, const uint8_t k[AKA_K_LEN],
	const uint8_t opc[AKA_OPC_LEN], const uint8_t rand[AKA_RAND_LEN],
	const uint8_t sqn[AKA_SQN_LEN], const uint8_t amf[AKA_AMF_LEN],
	uint8_t macs[AKA_MAC_LEN])
{
	uint8_t mac[16];

	if (!f1andf1star(this, k, opc, rand, sqn, amf, mac))
	{
		return FALSE;
	}
	/* only diff between f1 and f1* is here:
	 * f1  uses bytes 0-7  as MAC-A
	 * f1* uses bytes 8-15 as MAC-S */
	memcpy(macs, &mac[8], AKA_MAC_LEN);
	return TRUE;
}

METHOD(eap_aka_3gpp_functions_t, f2345, bool,
	private_eap_aka_3gpp_functions_t *this, const uint8_t k[AKA_K_LEN],
	const uint8_t opc[AKA_OPC_LEN], const uint8_t rand[AKA_RAND_LEN],
	uint8_t res[AKA_RES_LEN], uint8_t ck[AKA_CK_LEN], uint8_t ik[AKA_IK_LEN],
	uint8_t ak[AKA_AK_LEN])
{
	uint8_t data[16], iv[16] = { 0 };
	chunk_t temp;
	uint8_t i;

	if (!this->crypter->set_key(this->crypter,
								chunk_create((uint8_t*)k, AKA_K_LEN)))
	{
		return FALSE;
	}

	/* XOR RAND and OPc */
	memcpy(data, rand, sizeof(data));
	memxor(data, opc, sizeof(data));
	if (!this->crypter->encrypt(this->crypter, chunk_create(data, sizeof(data)),
								chunk_create(iv, sizeof(iv)), &temp))
	{
		return FALSE;
	}

	/* to obtain output block OUT2: XOR OPc and TEMP,
	 * rotate by r2=0, and XOR on the constant c2 (which is all zeroes except
	 * that the last bit is 1). */
	for (i = 0; i < 16; i++)
	{
		data[i] = temp.ptr[i] ^ opc[i];
	}
	data[15] ^= 1;

	if (!this->crypter->encrypt(this->crypter, chunk_create(data, sizeof(data)),
								chunk_create(iv, sizeof(iv)), NULL))
	{
		chunk_free(&temp);
		return FALSE;
	}
	memxor(data, opc, sizeof(data));

	/* f5 output */
	memcpy(ak, data, 6);
	/* f2 output */
	memcpy(res, &data[8], 8);

	/* to obtain output block OUT3: XOR OPc and TEMP,
	 * rotate by r3=32, and XOR on the constant c3 (which
	 * is all zeroes except that the next to last bit is 1) */
	for (i = 0; i < 16; i++)
	{
		data[(i + 12) % 16] = temp.ptr[i] ^ opc[i];
	}
	data[15] ^= 2;

	if (!this->crypter->encrypt(this->crypter, chunk_create(data, sizeof(data)),
								chunk_create(iv, sizeof(iv)), NULL))
	{
		chunk_free(&temp);
		return FALSE;
	}
	memxor(data, opc, sizeof(data));

	/* f3 output */
	memcpy(ck, data, 16);

	/* to obtain output block OUT4: XOR OPc and TEMP,
	 * rotate by r4=64, and XOR on the constant c4 (which
	 * is all zeroes except that the 2nd from last bit is 1). */
	for (i = 0; i < 16; i++)
	{
		data[(i + 8) % 16] = temp.ptr[i] ^ opc[i];
	}
	data[15] ^= 4;

	if (!this->crypter->encrypt(this->crypter, chunk_create(data, sizeof(data)),
								chunk_create(iv, sizeof(iv)), NULL))
	{
		chunk_free(&temp);
		return FALSE;
	}
	memxor(data, opc, sizeof(data));
	/* f4 output */
	memcpy(ik, data, 16);
	chunk_free(&temp);
	return TRUE;

}

METHOD(eap_aka_3gpp_functions_t, f5star, bool,
	private_eap_aka_3gpp_functions_t *this, const uint8_t k[AKA_K_LEN],
	const uint8_t opc[AKA_OPC_LEN], const uint8_t rand[AKA_RAND_LEN],
	uint8_t aks[AKA_AK_LEN])
{
	uint8_t i, data[16], iv[16] = { 0 };
	chunk_t temp;

	if (!this->crypter->set_key(this->crypter,
								chunk_create((uint8_t*)k, AKA_K_LEN)))
	{
		return FALSE;
	}

	/* XOR RAND and OPc */
	memcpy(data, rand, sizeof(data));
	memxor(data, opc, sizeof(data));
	if (!this->crypter->encrypt(this->crypter, chunk_create(data, sizeof(data)),
								chunk_create(iv, sizeof(iv)), &temp))
	{
		return FALSE;
	}

	/* to obtain output block OUT5: XOR OPc and the output above,
	 * rotate by r5=96, and XOR on the constant c5 (which
	 * is all zeroes except that the 3rd from last bit is 1). */
	for (i = 0; i < 16; i++)
	{
		data[(i + 4) % 16] = temp.ptr[i] ^ opc[i];
	}
	data[15] ^= 8;
	chunk_free(&temp);

	if (!this->crypter->encrypt(this->crypter, chunk_create(data, sizeof(data)),
								chunk_create(iv, sizeof(iv)), NULL))
	{
		return FALSE;
	}
	memxor(data, opc, sizeof(data));
	memcpy(aks, data, 6);
	return TRUE;
}

METHOD(eap_aka_3gpp_functions_t, destroy, void,
	private_eap_aka_3gpp_functions_t *this)
{
	this->crypter->destroy(this->crypter);
	free(this);
}

/**
 * See header
 */
eap_aka_3gpp_functions_t *eap_aka_3gpp_functions_create()
{
	private_eap_aka_3gpp_functions_t *this;

	INIT(this,
		.public = {
			.f1 = _f1,
			.f1star = _f1star,
			.f2345 = _f2345,
			.f5star = _f5star,
			.destroy = _destroy,
		},
		.crypter = lib->crypto->create_crypter(lib->crypto, ENCR_AES_CBC, 16),
	);
	if (!this->crypter)
	{
		DBG1(DBG_IKE, "%N not supported, unable to use 3GPP algorithm",
			 encryption_algorithm_names, ENCR_AES_CBC);
		free(this);
		return NULL;
	}
	return &this->public;
}
