/*
 * Copyright (C) 2013-2014 Andreas Steffen
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

#include "ntru_ke.h"
#include "ntru_drbg.h"
#include "ntru_param_set.h"
#include "ntru_private_key.h"
#include "ntru_public_key.h"

#include <crypto/diffie_hellman.h>
#include <utils/debug.h>

typedef struct private_ntru_ke_t private_ntru_ke_t;

/* Best bandwidth and speed, no X9.98 compatibility */
static const ntru_param_set_id_t param_sets_optimum[] = {
	NTRU_EES401EP2, NTRU_EES439EP1, NTRU_EES593EP1, NTRU_EES743EP1
};

/* X9.98/IEEE 1363.1 parameter sets for best speed */
static const ntru_param_set_id_t param_sets_x9_98_speed[] = {
	NTRU_EES659EP1, NTRU_EES761EP1, NTRU_EES1087EP1, NTRU_EES1499EP1
};

/* X9.98/IEEE 1363.1 parameter sets for best bandwidth (smallest size) */
static const ntru_param_set_id_t param_sets_x9_98_bandwidth[] = {
	NTRU_EES401EP1, NTRU_EES449EP1, NTRU_EES677EP1, NTRU_EES1087EP2
};

/* X9.98/IEEE 1363.1 parameter sets balancing speed and bandwidth */
static const ntru_param_set_id_t param_sets_x9_98_balance[] = {
	NTRU_EES541EP1, NTRU_EES613EP1, NTRU_EES887EP1, NTRU_EES1171EP1
};

/**
 * Private data of an ntru_ke_t object.
 */
struct private_ntru_ke_t {
	/**
	 * Public ntru_ke_t interface.
	 */
	ntru_ke_t public;

	/**
	 * Diffie Hellman group number.
	 */
	diffie_hellman_group_t group;

	/**
	 * NTRU Parameter Set
	 */
	const ntru_param_set_t *param_set;

	/**
	 * Cryptographical strength in bits of the NTRU Parameter Set
	 */
	uint32_t strength;

	/**
	 * NTRU Public Key
	 */
	ntru_public_key_t *pubkey;

	/**
	 * NTRU Private Key
	 */
	ntru_private_key_t *privkey;

	/**
	 * NTRU encrypted shared secret
	 */
	chunk_t ciphertext;

	/**
	 * Shared secret
	 */
	chunk_t shared_secret;

	/**
	 * True if peer is responder
	 */
	bool responder;

	/**
	 * True if shared secret is computed
	 */
	bool computed;

	/**
	 * True Random Generator
	 */
	rng_t *entropy;

	/**
	 * Deterministic Random Bit Generator
	 */
	ntru_drbg_t *drbg;
};

METHOD(diffie_hellman_t, get_my_public_value, bool,
	private_ntru_ke_t *this, chunk_t *value)
{
	*value = chunk_empty;

	if (this->responder)
	{
		if (this->ciphertext.len)
		{
			*value = chunk_clone(this->ciphertext);
		}
	}
	else
	{
		if (!this->pubkey)
		{
			/* generate a random NTRU public/private key pair */
			this->privkey = ntru_private_key_create(this->drbg, this->param_set);
			if (!this->privkey)
			{
				DBG1(DBG_LIB, "NTRU keypair generation failed");
				return FALSE;
			}
			this->pubkey = this->privkey->get_public_key(this->privkey);
		}
		*value = chunk_clone(this->pubkey->get_encoding(this->pubkey));
		DBG3(DBG_LIB, "NTRU public key: %B", value);
	}
	return TRUE;
}

METHOD(diffie_hellman_t, get_shared_secret, bool,
	private_ntru_ke_t *this, chunk_t *secret)
{
	if (!this->computed || !this->shared_secret.len)
	{
		*secret = chunk_empty;
		return FALSE;
	}
	*secret = chunk_clone(this->shared_secret);

	return TRUE;
}

METHOD(diffie_hellman_t, set_other_public_value, bool,
	private_ntru_ke_t *this, chunk_t value)
{
	if (this->privkey)
	{
		/* initiator decrypting shared secret */
		if (value.len == 0)
		{
			DBG1(DBG_LIB, "empty NTRU ciphertext");
			return FALSE;
		}
		DBG3(DBG_LIB, "NTRU ciphertext: %B", &value);

		/* decrypt the shared secret */
		if (!this->privkey->decrypt(this->privkey, value, &this->shared_secret))
		{
			DBG1(DBG_LIB, "NTRU decryption of shared secret failed");
			return FALSE;
		}
		this->computed = TRUE;
	}
	else
	{
		ntru_public_key_t *pubkey;

		/* responder generating and encrypting the shared secret */
		this->responder = TRUE;

		DBG3(DBG_LIB, "NTRU public key: %B", &value);
		pubkey = ntru_public_key_create_from_data(this->drbg, value);
		if (!pubkey)
		{
			return FALSE;
		}
		if (pubkey->get_id(pubkey) != this->param_set->id)
		{
			DBG1(DBG_LIB, "received NTRU public key with wrong OUI");
			pubkey->destroy(pubkey);
			return FALSE;
		}
		this->pubkey = pubkey;

		/* shared secret size is chosen as twice the cryptographical strength */
		this->shared_secret = chunk_alloc(2 * this->strength / BITS_PER_BYTE);

		/* generate the random shared secret */
		if (!this->drbg->generate(this->drbg, this->strength,
				this->shared_secret.len, this->shared_secret.ptr))
		{
			DBG1(DBG_LIB, "generation of shared secret failed");
			chunk_free(&this->shared_secret);
			return FALSE;
		}
		this->computed = TRUE;

		/* encrypt the shared secret */
		if (!pubkey->encrypt(pubkey, this->shared_secret, &this->ciphertext))
		{
			DBG1(DBG_LIB, "NTRU encryption of shared secret failed");
			return FALSE;
		}
		DBG3(DBG_LIB, "NTRU ciphertext: %B", &this->ciphertext);
	}
	return this->computed;
}

METHOD(diffie_hellman_t, get_dh_group, diffie_hellman_group_t,
	private_ntru_ke_t *this)
{
	return this->group;
}

METHOD(diffie_hellman_t, destroy, void,
	private_ntru_ke_t *this)
{
	DESTROY_IF(this->privkey);
	DESTROY_IF(this->pubkey);
	this->drbg->destroy(this->drbg);
	this->entropy->destroy(this->entropy);
	chunk_free(&this->ciphertext);
	chunk_clear(&this->shared_secret);
	free(this);
}

/*
 * Described in header.
 */
ntru_ke_t *ntru_ke_create(diffie_hellman_group_t group, chunk_t g, chunk_t p)
{
	private_ntru_ke_t *this;
	const ntru_param_set_id_t *param_sets;
	ntru_param_set_id_t param_set_id;
	rng_t *entropy;
	ntru_drbg_t *drbg;
	char *parameter_set;
	uint32_t strength;

	parameter_set = lib->settings->get_str(lib->settings,
						"%s.plugins.ntru.parameter_set", "optimum", lib->ns);

	if (streq(parameter_set, "x9_98_speed"))
	{
		param_sets = param_sets_x9_98_speed;
	}
	else if (streq(parameter_set, "x9_98_bandwidth"))
	{
		param_sets = param_sets_x9_98_bandwidth;
	}
	else if (streq(parameter_set, "x9_98_balance"))
	{
		param_sets = param_sets_x9_98_balance;
	}
	else
	{
		param_sets = param_sets_optimum;
	}

	switch (group)
	{
		case NTRU_112_BIT:
			strength = 112;
			param_set_id = param_sets[0];
			break;
		case NTRU_128_BIT:
			strength = 128;
			param_set_id = param_sets[1];
			break;
		case NTRU_192_BIT:
			strength = 192;
			param_set_id = param_sets[2];
			break;
		case NTRU_256_BIT:
			strength = 256;
			param_set_id = param_sets[3];
			break;
		default:
			return NULL;
	}
	DBG1(DBG_LIB, "%u bit %s NTRU parameter set %N selected", strength,
				   parameter_set, ntru_param_set_id_names, param_set_id);

	entropy = lib->crypto->create_rng(lib->crypto, RNG_TRUE);
	if (!entropy)
	{
		DBG1(DBG_LIB, "could not attach entropy source for DRBG");
		return NULL;
	}

	drbg = ntru_drbg_create(strength, chunk_from_str("IKE NTRU-KE"), entropy);
	if (!drbg)
	{
		DBG1(DBG_LIB, "could not instantiate DRBG at %u bit security", strength);
		entropy->destroy(entropy);
		return NULL;
	}

	INIT(this,
		.public = {
			.dh = {
				.get_shared_secret = _get_shared_secret,
				.set_other_public_value = _set_other_public_value,
				.get_my_public_value = _get_my_public_value,
				.get_dh_group = _get_dh_group,
				.destroy = _destroy,
			},
		},
		.group = group,
		.param_set = ntru_param_set_get_by_id(param_set_id),
		.strength = strength,
		.entropy = entropy,
		.drbg = drbg,
	);

	return &this->public;
}
