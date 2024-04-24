/*
 * Copyright (C) 2021 Andreas Steffen
 *
 * Copyright (C) secunet Security Networks AG
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

#ifdef TSS_TSS2_V2

#include "tpm_tss_tss2_session.h"
#include "tpm_tss_tss2_names.h"

#define LABEL	"TPM 2.0 - "

typedef struct private_tpm_tss_tss2_session_t private_tpm_tss_tss2_session_t;

/**
 * Private data of an tpm_tss_tss2_session_t object.
 */
struct private_tpm_tss_tss2_session_t {

	/**
	 * Public tpm_tss_tss2_session_t interface.
	 */
	tpm_tss_tss2_session_t public;

	/**
	 * Session handle for protected communication with TPM 2.0
	 */
	uint32_t session_handle;

	/**
	 * Session key for protected communication with TPM 2.0
	 */
	chunk_t session_key;

	/**
	 * Hash algorithm to be used for protected communication with TPM 2.0
	 */
	TPM2_ALG_ID hash_alg;

	/**
	 * nonceCaller used for protected communication with TPM 2.0
	 */
	TPM2B_NONCE nonceCaller;

	/**
	 * nonceTPM used for protected communication with TPM 2.0
	 */
	TPM2B_NONCE nonceTPM;

	/**
	 * AES-CFB key size in bytes
	 */
	size_t aes_key_len;

	/**
	 * SYS context
	 */
	TSS2_SYS_CONTEXT  *sys_context;

};

/**
 * Two functions shared with tpm_tss_tss2_v2.c
 */

hash_algorithm_t hash_alg_from_tpm_alg_id(TPM2_ALG_ID alg);

size_t hash_len_from_tpm_alg_id(TPM2_ALG_ID alg);


/**
 * Convert TPM2_ALG_ID to PRF algorithm
 */
static pseudo_random_function_t prf_alg_from_tpm_alg_id(TPM2_ALG_ID alg)
{
	switch (alg)
	{
		case TPM2_ALG_SHA1:
			return PRF_HMAC_SHA1;
		case TPM2_ALG_SHA256:
			return PRF_HMAC_SHA2_256;
		case TPM2_ALG_SHA384:
			return PRF_HMAC_SHA2_384;
		case TPM2_ALG_SHA512:
			return PRF_HMAC_SHA2_512;
		default:
			return PRF_UNDEFINED;
	}
}

static bool generate_nonce(size_t size, TPM2B_NONCE *nonce)
{
	nonce_gen_t *nonce_gen;
	bool success;

	nonce_gen = lib->crypto->create_nonce_gen(lib->crypto);
	if (!nonce_gen)
	{
		DBG1(DBG_PTS, "no nonce generator available");
		return FALSE;
	}
	nonce->size = size;
	success = nonce_gen->get_nonce(nonce_gen, nonce->size, nonce->buffer);
	nonce_gen->destroy(nonce_gen);

	if (!success)
	{
		DBG1(DBG_PTS, "generation of nonce failed");
		return FALSE;
	}

	return TRUE;
}

METHOD(tpm_tss_tss2_session_t, set_cmd_auths, bool,
	private_tpm_tss_tss2_session_t *this)
{
	size_t hash_len, param_size, cp_size;
	const uint8_t *param_buffer, *cp_buffer;
	uint8_t cc_buffer[4];
	hash_algorithm_t hash_algorithm;
	hasher_t *hasher;
	pseudo_random_function_t prf_alg;
	prf_t *prf;
	chunk_t data, cp_hash, nonce_caller, nonce_tpm, session_attributes;
	bool success;
	uint32_t rval;

	TSS2L_SYS_AUTH_COMMAND cmd;
	TPM2B_DIGEST cpHash;

	cmd.count = 1;
	cmd.auths[0].sessionHandle = this->session_handle;
	cmd.auths[0].sessionAttributes = TPMA_SESSION_CONTINUESESSION |
									 TPMA_SESSION_ENCRYPT;
	session_attributes = chunk_create(&cmd.auths[0].sessionAttributes, 1);

	hash_len = hash_len_from_tpm_alg_id(this->hash_alg);

	if (!generate_nonce(hash_len, &this->nonceCaller))
	{
		return FALSE;
	}
	cmd.auths[0].nonce.size = this->nonceCaller.size;
	memcpy(cmd.auths[0].nonce.buffer, this->nonceCaller.buffer,
									  this->nonceCaller.size);

	rval = Tss2_Sys_GetEncryptParam(this->sys_context, &param_size,
													   &param_buffer);
	if (rval == TSS2_SYS_RC_NO_ENCRYPT_PARAM)
	{
		DBG2(DBG_PTS, LABEL "parameter encryption not possible");
		return FALSE;
	}

	rval = Tss2_Sys_GetCommandCode(this->sys_context, cc_buffer);
	if (rval != TSS2_RC_SUCCESS)
	{
		DBG1(DBG_PTS, LABEL "Tss2_Sys_GetCommandCode failed: 0x%06x", rval);
		return FALSE;
	}

	rval = Tss2_Sys_GetCpBuffer(this->sys_context, &cp_size, &cp_buffer);
	if (rval != TSS2_RC_SUCCESS)
	{
		DBG1(DBG_PTS, LABEL "Tss2_GetCpBuffer failed: 0x%06x", rval);
		return FALSE;
	}

	/* compute cpHash */
	hash_algorithm = hash_alg_from_tpm_alg_id(this->hash_alg);
	hasher = lib->crypto->create_hasher(lib->crypto, hash_algorithm);
	if (!hasher)
	{
		DBG1(DBG_PTS, "hasher could not be created");
		return FALSE;
	}

	data = chunk_alloc(4 + cp_size);
	memcpy(data.ptr, cc_buffer, 4);
	memcpy(data.ptr + 4, cp_buffer, cp_size);

	success = hasher->get_hash(hasher, data, cpHash.buffer);
	cpHash.size = hasher->get_hash_size(hasher);
	hasher->destroy(hasher);
	chunk_free(&data);

	if (!success)
	{
		DBG1(DBG_PTS, "computation of cpHash failed");
		return FALSE;
	}
	cp_hash = chunk_create(cpHash.buffer, cpHash.size);

	/* compute cp HMAC */
	prf_alg = prf_alg_from_tpm_alg_id(this->hash_alg);
	prf = lib->crypto->create_prf(lib->crypto, prf_alg);
	if (!prf)
	{
		DBG1(DBG_PTS, "could not create PRF");
		return FALSE;
	}
	if (!prf->set_key(prf, this->session_key))
	{
		DBG1(DBG_PTS, "could not set PRF key");
		prf->destroy(prf);
		return FALSE;
	}

	nonce_caller = chunk_create(this->nonceCaller.buffer, this->nonceCaller.size);
	nonce_tpm = chunk_create(this->nonceTPM.buffer, this->nonceTPM.size);

	success = prf->get_bytes(prf, cp_hash, NULL)       &&
			  prf->get_bytes(prf, nonce_caller, NULL)  &&
			  prf->get_bytes(prf, nonce_tpm, NULL)     &&
			  prf->get_bytes(prf, session_attributes, cmd.auths[0].hmac.buffer);
	cmd.auths[0].hmac.size = prf->get_block_size(prf);
	prf->destroy(prf);

	if (!success)
	{
		DBG1(DBG_PTS, "cpHmac computation failed");
		return FALSE;
	}
	DBG2(DBG_PTS, LABEL "cpHmac: %b", cmd.auths[0].hmac.buffer,
		 cmd.auths[0].hmac.size);

	rval = Tss2_Sys_SetCmdAuths(this->sys_context, &cmd);
	if (rval != TSS2_RC_SUCCESS)
	{
		DBG1(DBG_PTS, LABEL "Tss2_Sys_SetCmdAuths failed: 0x%06x", rval);
		return FALSE;
	}

	return TRUE;
}

/**
 * Key Derivation Function using Counter Mode as defined by NIST SP800-108
 * - the label is expected to be NUL terminated
 */
static bool kdf_a(TPMI_ALG_HASH hash_alg, chunk_t key, chunk_t label,
				  chunk_t context_u, chunk_t context_v, uint32_t bytes,
				  chunk_t *key_mat)
{
	pseudo_random_function_t prf_alg;
	chunk_t count_chunk, bits_chunk;
	uint32_t iterations, counter, count, bits;
	uint8_t *pos;
	size_t hlen;
	prf_t *prf;

	bits = htonl(8 * bytes);
	bits_chunk = chunk_create((uint8_t*)&bits, sizeof(bits));

	prf_alg = prf_alg_from_tpm_alg_id(hash_alg);
	prf = lib->crypto->create_prf(lib->crypto, prf_alg);
	if (!prf)
	{
		DBG1(DBG_PTS, "could not create PRF");
		return FALSE;
	}
	if (!prf->set_key(prf, key))
	{
		DBG1(DBG_PTS, "could not set PRF key");
		prf->destroy(prf);
		return FALSE;
	}

	hlen = prf->get_block_size(prf);
	iterations = (bytes + hlen - 1) / hlen;
	*key_mat = chunk_alloc(iterations * hlen);
	pos = key_mat->ptr;

	for (counter = 1; counter <= iterations; counter++)
	{
		count = htonl(counter);
		count_chunk = chunk_create((uint8_t*)&count, sizeof(count));

		if (!prf->get_bytes(prf, count_chunk, NULL) ||
			!prf->get_bytes(prf, label, NULL)       ||
			!prf->get_bytes(prf, context_u, NULL)   ||
			!prf->get_bytes(prf, context_v, NULL)   ||
			!prf->get_bytes(prf, bits_chunk, pos))
		{
			DBG1(DBG_PTS, "KDFa computation failed");
			chunk_free(key_mat);
			prf->destroy(prf);
			return FALSE;
		}
		pos += hlen;
	}
	prf->destroy(prf);

	return TRUE;
}

METHOD(tpm_tss_tss2_session_t, get_rsp_auths, bool,
	private_tpm_tss_tss2_session_t *this)
{
	size_t param_size, rp_size, key_len, iv_len;
	const uint8_t *param_buffer, *rp_buffer;
	uint8_t rc_buffer[4] = { 0 };
	uint8_t cc_buffer[4];
	hash_algorithm_t hash_algorithm;
	hasher_t *hasher;
	pseudo_random_function_t prf_alg;
	prf_t *prf;
	crypter_t *crypter;
	chunk_t kdf_label = chunk_from_chars('C','F','B', 0x00);
	chunk_t data, rp_hash, nonce_caller, nonce_tpm, session_attributes;
	chunk_t key_mat, aes_key, aes_iv;
	bool success;
	uint32_t rval;

	TSS2L_SYS_AUTH_RESPONSE rsp;
	TPM2B_DIGEST rpHash, rpHmac;

	rval = Tss2_Sys_GetRspAuths(this->sys_context, &rsp);
	if (rval != TSS2_RC_SUCCESS)
	{
		DBG1(DBG_PTS, LABEL "Tss2_Sys_GetRspAuths failed: 0x%06x", rval);
		return FALSE;
	}

	/* update nonceTPM */
	memcpy(this->nonceTPM.buffer, rsp.auths[0].nonce.buffer,
								  rsp.auths[0].nonce.size);
	this->nonceTPM.size = rsp.auths[0].nonce.size;

	rval = Tss2_Sys_GetRpBuffer(this->sys_context, &rp_size, &rp_buffer);
	if (rval != TSS2_RC_SUCCESS)
	{
		DBG1(DBG_PTS, LABEL "Tss2_Sys_GetRpBuffer failed: 0x%06x", rval);
		return FALSE;
	}

	rval = Tss2_Sys_GetCommandCode(this->sys_context, cc_buffer);
	if (rval != TSS2_RC_SUCCESS)
	{
		DBG1(DBG_PTS, LABEL "Tss2_Sys_GetCommandCode failed: 0x%06x", rval);
		return FALSE;
	}

	/* compute rpHash */
	hash_algorithm = hash_alg_from_tpm_alg_id(this->hash_alg);
	hasher = lib->crypto->create_hasher(lib->crypto, hash_algorithm);
	if (!hasher)
	{
		DBG1(DBG_PTS, "hasher could not be created");
		return FALSE;
	}

	data = chunk_alloc(4 + 4 + rp_size);
	memcpy(data.ptr, rc_buffer, 4);
	memcpy(data.ptr + 4, cc_buffer, 4);
	memcpy(data.ptr + 8, rp_buffer, rp_size);

	success = hasher->get_hash(hasher, data, rpHash.buffer);
	rpHash.size = hasher->get_hash_size(hasher);
	hasher->destroy(hasher);
	chunk_free(&data);

	if (!success)
	{
		DBG1(DBG_PTS, "computation of rpHash failed");
		return FALSE;
	}
	rp_hash = chunk_create(rpHash.buffer, rpHash.size);

	/* compute rpHmac */
	prf_alg = prf_alg_from_tpm_alg_id(this->hash_alg);
	prf = lib->crypto->create_prf(lib->crypto, prf_alg);
	if (!prf)
	{
		DBG1(DBG_PTS, "could not create PRF");
		return FALSE;
	}
	if (!prf->set_key(prf, this->session_key))
	{
		DBG1(DBG_PTS, "could not set PRF key");
		prf->destroy(prf);
		return FALSE;
	}

	nonce_tpm = chunk_create(this->nonceTPM.buffer, this->nonceTPM.size);
	nonce_caller = chunk_create(this->nonceCaller.buffer, this->nonceCaller.size);
	session_attributes = chunk_create(&rsp.auths[0].sessionAttributes, 1);

	success = prf->get_bytes(prf, rp_hash, NULL)       &&
			  prf->get_bytes(prf, nonce_tpm, NULL)  &&
			  prf->get_bytes(prf, nonce_caller, NULL)     &&
			  prf->get_bytes(prf, session_attributes, rpHmac.buffer);
	rpHmac.size = prf->get_block_size(prf);
	prf->destroy(prf);

	if (!success)
	{
		DBG1(DBG_PTS, "computation of rpHmac failed");
		return FALSE;
	}
	DBG2(DBG_PTS, LABEL "rpHMAC: %b", rpHmac.buffer, rpHmac.size);

	/* verify rpHmac */
	if (!memeq(rsp.auths[0].hmac.buffer, rpHmac.buffer, rpHmac.size))
	{
		DBG1(DBG_PTS, LABEL "invalid HMAC received for session 0x%08x",
					  this->session_handle);
		return FALSE;
	}

	/* decrypt parameter */
	rval = Tss2_Sys_GetEncryptParam(this->sys_context, &param_size,
													   &param_buffer);
	if (rval != TSS2_RC_SUCCESS)
	{
		DBG1(DBG_PTS, LABEL "Tss2_Sys_GetEncryptParam failed: 0x%06x", rval);
		return FALSE;
	}

	crypter = lib->crypto->create_crypter(lib->crypto, ENCR_AES_CFB,
													   this->aes_key_len);
	if (!crypter)
	{
		DBG1(DBG_PTS, "could not create %N crypter", encryption_algorithm_names,
													 ENCR_AES_CFB);
		return FALSE;
	}

	key_len = crypter->get_key_size(crypter);
	iv_len  = crypter->get_iv_size(crypter);

	/* derive decryption key using KDFa */
	if (!kdf_a(this->hash_alg, this->session_key, kdf_label, nonce_tpm,
			   nonce_caller,  key_len + iv_len , &key_mat))
	{
		return FALSE;
	}
	aes_key = chunk_create(key_mat.ptr, key_len);
	aes_iv  = chunk_create(key_mat.ptr + key_len, iv_len);

	if (!crypter->set_key(crypter, aes_key))
	{
		crypter->destroy(crypter);
		chunk_clear(&key_mat);
		return FALSE;
	}

	/* copy ciphertext */
	data = chunk_alloc(param_size);
	memcpy(data.ptr, param_buffer, param_size);

	/* decrypt ciphertext */
	success = crypter->decrypt(crypter, data, aes_iv, NULL);
	crypter->destroy(crypter);
	chunk_clear(&key_mat);
	if (!success)
	{
		chunk_free(&data);
		return FALSE;
	}
	DBG4(DBG_PTS, LABEL "plaintext: %B", &data);

	/* copy back plaintext */
	rval = Tss2_Sys_SetEncryptParam(this->sys_context, data.len, data.ptr);
	chunk_clear(&data);

	if (rval != TSS2_RC_SUCCESS)
	{
		DBG1(DBG_PTS, LABEL "Tss2_Sys_SetEncryptParam failed: 0x%06x", rval);
		return FALSE;
	}

	return TRUE;
}


METHOD(tpm_tss_tss2_session_t, destroy, void,
	private_tpm_tss_tss2_session_t *this)
{
	if (this->session_handle)
	{
		uint32_t rval;

		/* flush session context */
		rval = Tss2_Sys_FlushContext(this->sys_context, this->session_handle);
		if (rval != TPM2_RC_SUCCESS)
		{
			DBG2(DBG_PTS, LABEL "Tss2_Sys_FlushContext failed: 0x%06x", rval);
		}
		chunk_clear(&this->session_key);
	}
	free(this);
}

static chunk_t secret_label = chunk_from_chars('S','E','C','R','E','T', 0x00);

static bool rsa_salt(TPM2B_PUBLIC *public, TPMI_ALG_HASH hash_alg,
					 chunk_t *secret, TPM2B_ENCRYPTED_SECRET *encryptedSalt)
{
	encryption_scheme_t encryption_scheme;
	public_key_t *pubkey = NULL;
	nonce_gen_t *nonce_gen;
	chunk_t encrypted_salt = chunk_empty;
	chunk_t rsa_modulus;
	chunk_t rsa_exponent = chunk_from_chars(0x01, 0x00, 0x01);
	uint32_t exponent;
	size_t hash_len;
	bool success;

	TPM2B_PUBLIC_KEY_RSA *rsa;

	switch (hash_alg)
	{
		case TPM2_ALG_SHA1:
			encryption_scheme = ENCRYPT_RSA_OAEP_SHA1;
			break;
		case TPM2_ALG_SHA256:
			encryption_scheme = ENCRYPT_RSA_OAEP_SHA256;
			break;
		case TPM2_ALG_SHA384:
			encryption_scheme = ENCRYPT_RSA_OAEP_SHA384;
			break;
		case TPM2_ALG_SHA512:
			encryption_scheme = ENCRYPT_RSA_OAEP_SHA512;
			break;
		default:
			DBG1(DBG_PTS, LABEL "unsupported key hash algorithm");
			return FALSE;
	}

	hash_len = hash_len_from_tpm_alg_id(hash_alg);

	/* create a salt nonce to be used as a shared secret */
	nonce_gen = lib->crypto->create_nonce_gen(lib->crypto);
	if (!nonce_gen)
	{
		DBG1(DBG_PTS, "no nonce generator available");
		return FALSE;
	}
	success = nonce_gen->allocate_nonce(nonce_gen, hash_len, secret);
	nonce_gen->destroy(nonce_gen);
	if (!success)
	{
		DBG1(DBG_PTS, "generation of salt nonce failed");
		return FALSE;
	}

	/* get RSA public key */
	rsa = &public->publicArea.unique.rsa;
	rsa_modulus = chunk_create(rsa->buffer, rsa->size);
	exponent = htonl(public->publicArea.parameters.rsaDetail.exponent);
	if (exponent)
	{
		rsa_exponent = chunk_from_thing(exponent);
	}
	pubkey = lib->creds->create(lib->creds, CRED_PUBLIC_KEY, KEY_RSA,
						BUILD_RSA_MODULUS, rsa_modulus, BUILD_RSA_PUB_EXP,
						rsa_exponent, BUILD_END);
	if (!pubkey)
	{
		DBG1(DBG_PTS, "retrieval of EK public key failed");
		chunk_clear(secret);
		return FALSE;
	}

	/* use RSA public key encryption to encrypt secret salt nonce */
	success = pubkey->encrypt(pubkey, encryption_scheme, &secret_label,
							  *secret, &encrypted_salt);
	pubkey->destroy(pubkey);
	if (!success)
	{
		DBG1(DBG_PTS, "encryption of salt failed");
		chunk_clear(secret);
		return FALSE;
	}

	/* copy encryptedSalt to output parameter */
	encryptedSalt->size = encrypted_salt.len;
	memcpy(encryptedSalt->secret, encrypted_salt.ptr, encrypted_salt.len);
	free(encrypted_salt.ptr);

	return TRUE;
}


/**
 * Key Derivation Function used to derive an ecc-based secret
 * - the label is expected to be NUL terminated
 */
static bool kdf_e(TPMI_ALG_HASH hash_alg, chunk_t z, chunk_t label,
				  chunk_t context_u, chunk_t context_v, uint32_t bytes,
				  chunk_t *key_mat)
{
	hash_algorithm_t hash_algorithm;
	chunk_t count_chunk;
	uint32_t iterations, counter, count;
	uint8_t *pos;
	size_t hlen;
	hasher_t *hasher;

	hash_algorithm = hash_alg_from_tpm_alg_id(hash_alg);
	hasher = lib->crypto->create_hasher(lib->crypto, hash_algorithm);
	if (!hasher)
	{
		DBG1(DBG_PTS, "could not create hasher");
		return FALSE;
	}

	hlen = hasher->get_hash_size(hasher);
	iterations = (bytes + hlen - 1) / hlen;
	*key_mat = chunk_alloc(iterations * hlen);
	pos = key_mat->ptr;

	for (counter = 1; counter <= iterations; counter++)
	{
		count = htonl(counter);
		count_chunk = chunk_create((uint8_t*)&count, sizeof(count));

		if (!hasher->get_hash(hasher, count_chunk, NULL) ||
			!hasher->get_hash(hasher, z, NULL)           ||
			!hasher->get_hash(hasher, label, NULL)       ||
			!hasher->get_hash(hasher, context_u, NULL)   ||
			!hasher->get_hash(hasher, context_v, pos))
		{
			DBG1(DBG_PTS, "KDFe computation failed");
			chunk_free(key_mat);
			hasher->destroy(hasher);
			return FALSE;
		}
		pos += hlen;
	}
	hasher->destroy(hasher);

	return TRUE;
}

static bool ecc_salt(TPM2B_PUBLIC *public, TPMI_ALG_HASH hash_alg,
					 chunk_t *secret, TPM2B_ENCRYPTED_SECRET *encryptedSalt)
{
	key_exchange_method_t ec_ke_method;
	key_exchange_t *ke;
	chunk_t ecdh_pubkey = chunk_empty, ecdh_pubkey_x, ecdh_pubkey_y;
	chunk_t ecc_pubkey  = chunk_empty, ecc_pubkey_x, ecc_pubkey_y;
	chunk_t z = chunk_empty;
	uint16_t len;
	uint8_t *pos;
	size_t hash_len;
	bool success = FALSE;

	switch (public->publicArea.parameters.eccDetail.curveID)
	{
		case TPM2_ECC_NIST_P256:
			ec_ke_method = ECP_256_BIT;
			break;
		case TPM2_ECC_NIST_P384:
			ec_ke_method = ECP_384_BIT;
			break;
		case TPM2_ECC_NIST_P521:
			ec_ke_method = ECP_521_BIT;
			break;
		default:
			DBG1(DBG_PTS, "type of ECC EK key not supported");
			return FALSE;
	}

	/* Generate ECDH key pair */
	ke = lib->crypto->create_ke(lib->crypto, ec_ke_method);
	if (!ke)
	{
		DBG1(DBG_PTS, "DH group could not be created");
		return FALSE;
	}
	if (!ke->get_public_key(ke, &ecdh_pubkey))
	{
		DBG1(DBG_PTS, "DH public key could not be generated");
		ke->destroy(ke);
		return FALSE;
	}
	ecdh_pubkey_x = chunk_create(ecdh_pubkey.ptr, ecdh_pubkey.len / 2);
	ecdh_pubkey_y = chunk_create(ecdh_pubkey.ptr + ecdh_pubkey_x.len,
								 ecdh_pubkey_x.len);

	/* get ECC public key */
	ecc_pubkey_x = chunk_create(public->publicArea.unique.ecc.x.buffer,
								public->publicArea.unique.ecc.x.size);
	ecc_pubkey_y = chunk_create(public->publicArea.unique.ecc.y.buffer,
								public->publicArea.unique.ecc.y.size);
	ecc_pubkey = chunk_cat("cc", ecc_pubkey_x, ecc_pubkey_y);

	/* compute point multiplication of ecc_pubkey with ecdh_privkey */
	if (!ke->set_public_key(ke, ecc_pubkey))
	{
		DBG1(DBG_PTS, "ECC public could not be set");
		goto error;
	}
	if (!ke->get_shared_secret(ke, &z))
	{
		DBG1(DBG_PTS, "could not create shared secret");
		goto error;
	}

	hash_len = hash_len_from_tpm_alg_id(hash_alg);

	/* derive secret using KDFe */
	if (!kdf_e(hash_alg, z, secret_label, ecdh_pubkey_x, ecc_pubkey_x,
			   hash_len, secret))
	{
		goto error;
	}

	/* copy ECDH pubkey to encrypted salt parameter */
	len = htons(ecdh_pubkey_x.len);
	encryptedSalt->size = 2 * sizeof(len) + ecdh_pubkey.len;
	pos = encryptedSalt->secret;
	memcpy(pos, (uint8_t*)&len, sizeof(len));
	pos += sizeof(len);
	memcpy(pos, ecdh_pubkey_x.ptr, ecdh_pubkey_x.len);
	pos += ecdh_pubkey_x.len;
	memcpy(pos, (uint8_t*)&len, sizeof(len));
	pos += sizeof(len);
	memcpy(pos, ecdh_pubkey_y.ptr, ecdh_pubkey_y.len);

	success = TRUE;

error:
	ke->destroy(ke);
	chunk_free(&ecdh_pubkey);
	chunk_free(&ecc_pubkey);
	chunk_clear(&z);

	return success;
}

/**
 * See header
 */
tpm_tss_tss2_session_t* tpm_tss_tss2_session_create(uint32_t ek_handle,
							TPM2B_PUBLIC *public, TSS2_SYS_CONTEXT  *sys_context)
{
	private_tpm_tss_tss2_session_t *this;
	chunk_t secret = chunk_empty;
	chunk_t kdf_label = chunk_from_chars('A','T','H', 0x00);
	chunk_t nonce_caller, nonce_tpm;
	size_t hash_len;
	uint32_t rval;

	TPM2B_ENCRYPTED_SECRET encryptedSalt;
	TPM2_SE sessionType = TPM2_SE_HMAC;
	TPMT_SYM_DEF *sym;

	INIT(this,
		.public = {
			.set_cmd_auths = _set_cmd_auths,
			.get_rsp_auths = _get_rsp_auths,
			.destroy = _destroy,
		},
		.sys_context = sys_context,
		.hash_alg = public->publicArea.nameAlg,
	);

	hash_len = hash_len_from_tpm_alg_id(this->hash_alg);

	if (!generate_nonce(hash_len, &this->nonceCaller))
	{
		goto error;
	}

	/* determine endorsement key type */
	switch (public->publicArea.type)
	{
		case TPM2_ALG_RSA:
			DBG1(DBG_PTS, LABEL "RSA EK handle: 0x%08x", ek_handle);
			if (!rsa_salt(public, this->hash_alg, &secret, &encryptedSalt))
			{
				goto error;
			}
			break;
		case TPM2_ALG_ECC:
			DBG1(DBG_PTS, LABEL "ECC %N EK handle: 0x%08x", tpm_ecc_curve_names,
				 public->publicArea.parameters.eccDetail.curveID, ek_handle);
			if (!ecc_salt(public, this->hash_alg, &secret, &encryptedSalt))
			{
				goto error;
			}
			break;
		default:
			DBG1(DBG_PTS, LABEL "unsupported ek key type");
			goto error;
	}

	sym = (TPMT_SYM_DEF*)&public->publicArea.parameters.asymDetail.symmetric;
	DBG2(DBG_PTS, LABEL "AES-CFB with %u bits", sym->keyBits.aes);
	this->aes_key_len = sym->keyBits.aes / 8;

	rval = Tss2_Sys_StartAuthSession(this->sys_context, ek_handle, TPM2_RH_NULL,
				NULL, &this->nonceCaller, &encryptedSalt, sessionType, sym,
				this->hash_alg, &this->session_handle, &this->nonceTPM, NULL);
	if (rval != TSS2_RC_SUCCESS)
	{
		DBG1(DBG_PTS, LABEL "Tss2_Sys_StartAuthSession failed: 0x%06x", rval);
		goto error;
    }
	DBG2(DBG_PTS, LABEL "session handle: 0x%08x", this->session_handle);

	nonce_tpm = chunk_create(this->nonceTPM.buffer, this->nonceTPM.size);
	nonce_caller = chunk_create(this->nonceCaller.buffer, this->nonceCaller.size);

	/* derive sessionKey using KDFa */
	if (!kdf_a(this->hash_alg, secret, kdf_label, nonce_tpm, nonce_caller,
			   hash_len, &this->session_key))
	{
		goto error;
	}
	chunk_clear(&secret);
	DBG4(DBG_PTS, LABEL "session key: %B", &this->session_key);

	return &this->public;

	error:
		chunk_clear(&secret);
		destroy(this);
		return NULL;
}

#endif /* TSS_TSS2_V2 */
