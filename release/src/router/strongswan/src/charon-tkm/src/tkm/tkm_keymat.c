/*
 * Copyright (C) 2015 Tobias Brunner
 * Copyright (C) 2012 Reto Buerki
 * Copyright (C) 2012 Adrian-Ken Rueegsegger
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

#include <daemon.h>
#include <tkm/constants.h>
#include <tkm/client.h>
#include <crypto/hashers/hash_algorithm_set.h>

#include "tkm.h"
#include "tkm_types.h"
#include "tkm_utils.h"
#include "tkm_diffie_hellman.h"
#include "tkm_keymat.h"

typedef struct private_tkm_keymat_t private_tkm_keymat_t;

/**
 * Private data of a keymat_t object.
 */
struct private_tkm_keymat_t {

	/**
	 * Public tkm_keymat_t interface.
	 */
	tkm_keymat_t public;

	/**
	 * IKE_SA Role, initiator or responder.
	 */
	bool initiator;

	/**
	 * Inbound AEAD.
	 */
	aead_t *aead_in;

	/**
	 * Outbound AEAD.
	 */
	aead_t *aead_out;

	/**
	 * ISA context id.
	 */
	isa_id_type isa_ctx_id;

	/**
	 * AE context id.
	 */
	ae_id_type ae_ctx_id;

	/**
	 * AUTH payload chunk.
	 */
	chunk_t auth_payload;

	/**
	 * Peer init message chunk.
	 */
	chunk_t other_init_msg;

	/**
	 * Set of hash algorithms supported by peer for signature authentication
	 */
	hash_algorithm_set_t *hash_algorithms;
};

/**
 * Create AEAD transforms from given key chunks.
 *
 * @param in			inbound AEAD transform to allocate, NULL if failed
 * @param out			outbound AEAD transform to allocate, NULL if failed
 * @param sk_ai			SK_ai key chunk
 * @param sk_ar			SK_ar key chunk
 * @param sk_ei			SK_ei key chunk
 * @param sk_er			SK_er key chunk
 * @param enc_alg		encryption algorithm to use
 * @param int_alg		integrity algorithm to use
 * @param key_size		encryption key size in bytes
 * @param initiator		TRUE if initiator
 */
static void aead_create_from_keys(aead_t **in, aead_t **out,
	   const chunk_t * const sk_ai, const chunk_t * const sk_ar,
	   const chunk_t * const sk_ei, const chunk_t * const sk_er,
	   const uint16_t enc_alg, const uint16_t int_alg,
	   const uint16_t key_size, bool initiator)
{
	*in = *out = NULL;
	signer_t *signer_i, *signer_r;
	crypter_t *crypter_i, *crypter_r;
	iv_gen_t *ivg_i, *ivg_r;

	signer_i = lib->crypto->create_signer(lib->crypto, int_alg);
	signer_r = lib->crypto->create_signer(lib->crypto, int_alg);
	if (signer_i == NULL || signer_r == NULL)
	{
		DBG1(DBG_IKE, "%N %N not supported!",
			 transform_type_names, INTEGRITY_ALGORITHM,
			 integrity_algorithm_names, int_alg);
		return;
	}
	crypter_i = lib->crypto->create_crypter(lib->crypto, enc_alg, key_size);
	crypter_r = lib->crypto->create_crypter(lib->crypto, enc_alg, key_size);
	if (crypter_i == NULL || crypter_r == NULL)
	{
		signer_i->destroy(signer_i);
		signer_r->destroy(signer_r);
		DBG1(DBG_IKE, "%N %N (key size %d) not supported!",
			 transform_type_names, ENCRYPTION_ALGORITHM,
			 encryption_algorithm_names, enc_alg, key_size);
		return;
	}

	DBG4(DBG_IKE, "Sk_ai %B", sk_ai);
	if (!signer_i->set_key(signer_i, *sk_ai))
	{
		return;
	}
	DBG4(DBG_IKE, "Sk_ar %B", sk_ar);
	if (!signer_r->set_key(signer_r, *sk_ar))
	{
		return;
	}
	DBG4(DBG_IKE, "Sk_ei %B", sk_ei);
	if (!crypter_i->set_key(crypter_i, *sk_ei))
	{
		return;
	}
	DBG4(DBG_IKE, "Sk_er %B", sk_er);
	if (!crypter_r->set_key(crypter_r, *sk_er))
	{
		return;
	}

	ivg_i = iv_gen_create_for_alg(enc_alg);
	ivg_r = iv_gen_create_for_alg(enc_alg);
	if (!ivg_i || !ivg_r)
	{
		return;
	}
	if (initiator)
	{
		*in = aead_create(crypter_r, signer_r, ivg_r);
		*out = aead_create(crypter_i, signer_i, ivg_i);
	}
	else
	{
		*in = aead_create(crypter_i, signer_i, ivg_i);
		*out = aead_create(crypter_r, signer_r, ivg_r);
	}
}

METHOD(keymat_t, get_version, ike_version_t,
	private_tkm_keymat_t *this)
{
	return IKEV2;
}

METHOD(keymat_t, create_dh, diffie_hellman_t*,
	private_tkm_keymat_t *this, diffie_hellman_group_t group)
{
	return lib->crypto->create_dh(lib->crypto, group);
}

METHOD(keymat_t, create_nonce_gen, nonce_gen_t*,
	private_tkm_keymat_t *this)
{
	return lib->crypto->create_nonce_gen(lib->crypto);
}

METHOD(keymat_v2_t, derive_ike_keys, bool,
	private_tkm_keymat_t *this, proposal_t *proposal, diffie_hellman_t *dh,
	chunk_t nonce_i, chunk_t nonce_r, ike_sa_id_t *id,
	pseudo_random_function_t rekey_function, chunk_t rekey_skd)
{
	uint16_t enc_alg, int_alg, key_size;
	uint64_t nc_id, spi_loc, spi_rem;
	chunk_t *nonce, c_ai, c_ar, c_ei, c_er;
	tkm_diffie_hellman_t *tkm_dh;
	dh_id_type dh_id;
	nonce_type nonce_rem;
	result_type res;
	key_type sk_ai, sk_ar, sk_ei, sk_er;

	/* Check encryption and integrity algorithms */
	if (!proposal->get_algorithm(proposal, ENCRYPTION_ALGORITHM, &enc_alg,
								 &key_size))
	{
		DBG1(DBG_IKE, "no %N selected", transform_type_names,
			 ENCRYPTION_ALGORITHM);
		return FALSE;
	}
	if (encryption_algorithm_is_aead(enc_alg))
	{
		DBG1(DBG_IKE, "AEAD algorithm %N not supported",
			 encryption_algorithm_names, enc_alg);
		return FALSE;
	}
	if (!proposal->get_algorithm(proposal, INTEGRITY_ALGORITHM, &int_alg, NULL))
	{
		DBG1(DBG_IKE, "no %N selected", transform_type_names,
			 INTEGRITY_ALGORITHM);
		return FALSE;
	}
	if (!(enc_alg == ENCR_AES_CBC && key_size == 256 &&
		  int_alg == AUTH_HMAC_SHA2_512_256))
	{
		DBG1(DBG_IKE, "the TKM only supports aes256-sha512 at the moment, "
			 "please update your configuration");
		return FALSE;
	}

	DBG2(DBG_IKE, "using %N for encryption, %N for integrity",
		 encryption_algorithm_names, enc_alg, integrity_algorithm_names,
		 int_alg);

	/* Acquire nonce context id */
	nonce = this->initiator ? &nonce_i : &nonce_r;
	nc_id = tkm->chunk_map->get_id(tkm->chunk_map, nonce);
	if (!nc_id)
	{
		DBG1(DBG_IKE, "unable to acquire context id for nonce");
		return FALSE;
	}

	/* Get DH context id */
	tkm_dh = (tkm_diffie_hellman_t *)dh;
	dh_id = tkm_dh->get_id(tkm_dh);

	if (this->initiator)
	{
		chunk_to_sequence(&nonce_r, &nonce_rem, sizeof(nonce_type));
		spi_loc = id->get_initiator_spi(id);
		spi_rem = id->get_responder_spi(id);
	}
	else
	{
		chunk_to_sequence(&nonce_i, &nonce_rem, sizeof(nonce_type));
		spi_loc = id->get_responder_spi(id);
		spi_rem = id->get_initiator_spi(id);
	}

	if (rekey_function == PRF_UNDEFINED)
	{
		this->ae_ctx_id = tkm->idmgr->acquire_id(tkm->idmgr, TKM_CTX_AE);
		if (!this->ae_ctx_id)
		{
			DBG1(DBG_IKE, "unable to acquire ae context id");
			return FALSE;
		}
		DBG1(DBG_IKE, "deriving IKE keys (nc: %llu, dh: %llu, spi_loc: %llx, "
			 "spi_rem: %llx)", nc_id, dh_id, spi_loc, spi_rem);
		res = ike_isa_create(this->isa_ctx_id, this->ae_ctx_id, 1, dh_id, nc_id,
							 nonce_rem, this->initiator, spi_loc, spi_rem,
							 &sk_ai, &sk_ar, &sk_ei, &sk_er);
	}
	else
	{
		isa_info_t isa_info;

		if (rekey_skd.ptr == NULL || rekey_skd.len != sizeof(isa_info_t))
		{
			DBG1(DBG_IKE, "unable to retrieve parent isa info");
			return FALSE;
		}
		isa_info = *((isa_info_t *)(rekey_skd.ptr));
		DBG1(DBG_IKE, "deriving IKE keys (parent_isa: %llu, ae: %llu, nc: %llu,"
			 " dh: %llu, spi_loc: %llx, spi_rem: %llx)", isa_info.parent_isa_id,
			 isa_info.ae_id, nc_id, dh_id, spi_loc, spi_rem);

		if (!tkm->idmgr->acquire_ref(tkm->idmgr, TKM_CTX_AE, isa_info.ae_id))
		{
			DBG1(DBG_IKE, "unable to acquire reference for ae: %llu",
				 isa_info.ae_id);
			return FALSE;
		}
		this->ae_ctx_id = isa_info.ae_id;
		res = ike_isa_create_child(this->isa_ctx_id, isa_info.parent_isa_id, 1,
								   dh_id, nc_id, nonce_rem, this->initiator,
								   spi_loc, spi_rem, &sk_ai, &sk_ar, &sk_ei,
								   &sk_er);
		chunk_free(&rekey_skd);
	}

	if (res != TKM_OK)
	{
		DBG1(DBG_IKE, "key derivation failed (isa: %llu)", this->isa_ctx_id);
		return FALSE;
	}

	sequence_to_chunk(sk_ai.data, sk_ai.size, &c_ai);
	sequence_to_chunk(sk_ar.data, sk_ar.size, &c_ar);
	sequence_to_chunk(sk_ei.data, sk_ei.size, &c_ei);
	sequence_to_chunk(sk_er.data, sk_er.size, &c_er);

	aead_create_from_keys(&this->aead_in, &this->aead_out, &c_ai, &c_ar, &c_ei,
						  &c_er, enc_alg, int_alg, key_size / 8,
						  this->initiator);

	chunk_clear(&c_ai);
	chunk_clear(&c_ar);
	chunk_clear(&c_ei);
	chunk_clear(&c_er);

	if (!this->aead_in || !this->aead_out)
	{
		DBG1(DBG_IKE, "could not initialize AEAD transforms");
		return FALSE;
	}

	/* TODO: Add failure handler (see keymat_v2.c) */

	tkm->chunk_map->remove(tkm->chunk_map, nonce);
	if (ike_nc_reset(nc_id) != TKM_OK)
	{
		DBG1(DBG_IKE, "failed to reset nonce context %llu", nc_id);
	}
	tkm->idmgr->release_id(tkm->idmgr, TKM_CTX_NONCE, nc_id);

	return TRUE;
}

METHOD(keymat_v2_t, derive_child_keys, bool,
	private_tkm_keymat_t *this, proposal_t *proposal, diffie_hellman_t *dh,
	chunk_t nonce_i, chunk_t nonce_r, chunk_t *encr_i, chunk_t *integ_i,
	chunk_t *encr_r, chunk_t *integ_r)
{
	esa_info_t *esa_info_i, *esa_info_r;
	dh_id_type dh_id = 0;

	if (dh)
	{
		dh_id = ((tkm_diffie_hellman_t *)dh)->get_id((tkm_diffie_hellman_t *)dh);
	}

	INIT(esa_info_i,
		 .isa_id = this->isa_ctx_id,
		 .spi_r = proposal->get_spi(proposal),
		 .nonce_i = chunk_clone(nonce_i),
		 .nonce_r = chunk_clone(nonce_r),
		 .is_encr_r = FALSE,
		 .dh_id = dh_id,
	);

	INIT(esa_info_r,
		 .isa_id = this->isa_ctx_id,
		 .spi_r = proposal->get_spi(proposal),
		 .nonce_i = chunk_clone(nonce_i),
		 .nonce_r = chunk_clone(nonce_r),
		 .is_encr_r = TRUE,
		 .dh_id = dh_id,
	);

	DBG1(DBG_CHD, "passing on esa info (isa: %llu, spi_r: %x, dh_id: %llu)",
		 esa_info_i->isa_id, ntohl(esa_info_i->spi_r), esa_info_i->dh_id);

	/* store ESA info in encr_i/r, which is passed to add_sa */
	*encr_i = chunk_create((u_char *)esa_info_i, sizeof(esa_info_t));
	*encr_r = chunk_create((u_char *)esa_info_r, sizeof(esa_info_t));
	*integ_i = chunk_empty;
	*integ_r = chunk_empty;

	return TRUE;
}

METHOD(keymat_t, get_aead, aead_t*,
	private_tkm_keymat_t *this, bool in)
{
	return in ? this->aead_in : this->aead_out;
}

METHOD(keymat_v2_t, get_auth_octets, bool,
	private_tkm_keymat_t *this, bool verify, chunk_t ike_sa_init,
	chunk_t nonce, chunk_t ppk, identification_t *id, char reserved[3],
	chunk_t *octets, array_t *schemes)
{
	sign_info_t *sign;

	if (verify)
	{
		/* store peer init message for authentication step */
		this->other_init_msg = chunk_clone(ike_sa_init);
		*octets = chunk_empty;
		return TRUE;
	}

	INIT(sign,
		 .isa_id = this->isa_ctx_id,
		 .init_message = chunk_clone(ike_sa_init),
	);

	/*
	 * store signature info in AUTH octets, which is passed to the private key
	 * sign() operation
	 */
	*octets = chunk_create((u_char *)sign, sizeof(sign_info_t));
	return TRUE;
}

METHOD(keymat_v2_t, get_skd, pseudo_random_function_t,
	private_tkm_keymat_t *this, chunk_t *skd)
{
	isa_info_t *isa_info;

	INIT(isa_info,
		 .parent_isa_id = this->isa_ctx_id,
		 .ae_id = this->ae_ctx_id,
	);

	*skd = chunk_create((u_char *)isa_info, sizeof(isa_info_t));

	return PRF_HMAC_SHA2_512;
}

METHOD(keymat_v2_t, get_psk_sig, bool,
	private_tkm_keymat_t *this, bool verify, chunk_t ike_sa_init, chunk_t nonce,
	chunk_t secret, chunk_t ppk, identification_t *id, char reserved[3],
	chunk_t *sig)
{
	return FALSE;
}

METHOD(keymat_v2_t, hash_algorithm_supported, bool,
	private_tkm_keymat_t *this, hash_algorithm_t hash)
{
	if (!this->hash_algorithms)
	{
		return FALSE;
	}
	return this->hash_algorithms->contains(this->hash_algorithms, hash);
}

METHOD(keymat_v2_t, add_hash_algorithm, void,
	private_tkm_keymat_t *this, hash_algorithm_t hash)
{
	if (!this->hash_algorithms)
	{
		this->hash_algorithms = hash_algorithm_set_create();
	}
	this->hash_algorithms->add(this->hash_algorithms, hash);
}

METHOD(keymat_t, destroy, void,
	private_tkm_keymat_t *this)
{
	if (ike_isa_reset(this->isa_ctx_id) != TKM_OK)
	{
		DBG1(DBG_IKE, "failed to reset ISA context %d", this->isa_ctx_id);
	}
	tkm->idmgr->release_id(tkm->idmgr, TKM_CTX_ISA, this->isa_ctx_id);
	/* only reset ae context if set */
	if (this->ae_ctx_id != 0)
	{
		int count;
		count = tkm->idmgr->release_id(tkm->idmgr, TKM_CTX_AE, this->ae_ctx_id);
		if (count == 0 && ike_ae_reset(this->ae_ctx_id) != TKM_OK)
		{
			DBG1(DBG_IKE, "failed to reset AE context %d", this->ae_ctx_id);
		}
	}

	DESTROY_IF(this->hash_algorithms);
	DESTROY_IF(this->aead_in);
	DESTROY_IF(this->aead_out);
	chunk_free(&this->auth_payload);
	chunk_free(&this->other_init_msg);
	free(this);
}

METHOD(tkm_keymat_t, get_isa_id, isa_id_type,
	private_tkm_keymat_t *this)
{
	return this->isa_ctx_id;
}

METHOD(tkm_keymat_t, set_auth_payload, void,
	private_tkm_keymat_t *this, const chunk_t * const payload)
{
	this->auth_payload = chunk_clone(*payload);
}

METHOD(tkm_keymat_t, get_auth_payload, chunk_t*,
	private_tkm_keymat_t *this)
{
	return &this->auth_payload;
}

METHOD(tkm_keymat_t, get_peer_init_msg, chunk_t*,
	private_tkm_keymat_t *this)
{
	return &this->other_init_msg;
}

/**
 * See header.
 */
tkm_keymat_t *tkm_keymat_create(bool initiator)
{
	private_tkm_keymat_t *this;

	INIT(this,
		.public = {
			.keymat_v2 = {
				.keymat = {
					.get_version = _get_version,
					.create_dh = _create_dh,
					.create_nonce_gen = _create_nonce_gen,
					.get_aead = _get_aead,
					.destroy = _destroy,
				},
				.derive_ike_keys = _derive_ike_keys,
				.derive_ike_keys_ppk = (void*)return_false,
				.derive_child_keys = _derive_child_keys,
				.get_skd = _get_skd,
				.get_auth_octets = _get_auth_octets,
				.get_psk_sig = _get_psk_sig,
				.add_hash_algorithm = _add_hash_algorithm,
				.hash_algorithm_supported = _hash_algorithm_supported,
			},
			.get_isa_id = _get_isa_id,
			.set_auth_payload = _set_auth_payload,
			.get_auth_payload = _get_auth_payload,
			.get_peer_init_msg = _get_peer_init_msg,
		},
		.initiator = initiator,
		.isa_ctx_id = tkm->idmgr->acquire_id(tkm->idmgr, TKM_CTX_ISA),
		.ae_ctx_id = 0,
		.auth_payload = chunk_empty,
		.other_init_msg = chunk_empty,
	);

	if (!this->isa_ctx_id)
	{
		free(this);
		return NULL;
	}

	return &this->public;
}
