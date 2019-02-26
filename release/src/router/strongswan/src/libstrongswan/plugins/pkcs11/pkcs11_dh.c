/*
 * Copyright (C) 2011 Tobias Brunner
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

#include "pkcs11_dh.h"

#include <utils/debug.h>
#include <library.h>
#include <asn1/asn1.h>
#include <asn1/oid.h>

#include "pkcs11_manager.h"

typedef struct private_pkcs11_dh_t private_pkcs11_dh_t;

/**
 * Private data of an pkcs11_dh_t object.
 */
struct private_pkcs11_dh_t {

	/**
	 * Public pkcs11_dh_t interface
	 */
	pkcs11_dh_t public;

	/**
	 * PKCS#11 library
	 */
	pkcs11_library_t *lib;

	/**
	 * Session handle for this objct
	 */
	CK_SESSION_HANDLE session;

	/**
	 * Diffie Hellman group number.
	 */
	diffie_hellman_group_t group;

	/**
	 * Handle for own private value
	 */
	CK_OBJECT_HANDLE pri_key;

	/**
	 * Own public value
	 */
	chunk_t pub_key;

	/**
	 * Shared secret
	 */
	chunk_t secret;

	/**
	 * Mechanism to use to generate a key pair
	 */
	CK_MECHANISM_TYPE mech_key;

	/**
	 * Mechanism to use to derive a shared secret
	 */
	CK_MECHANISM_TYPE mech_derive;

};

/**
 * Derive a DH/ECDH shared secret.
 *
 * If this succeeds the shared secret is stored in this->secret.
 */
static bool derive_secret(private_pkcs11_dh_t *this, chunk_t other)
{
	CK_OBJECT_CLASS klass = CKO_SECRET_KEY;
	CK_KEY_TYPE type = CKK_GENERIC_SECRET;
	CK_ATTRIBUTE attr[] = {
		{ CKA_CLASS, &klass, sizeof(klass) },
		{ CKA_KEY_TYPE, &type, sizeof(type) },
	};
	CK_MECHANISM mech = {
		this->mech_derive,
		other.ptr,
		other.len,
	};
	CK_OBJECT_HANDLE secret;
	CK_RV rv;

	rv = this->lib->f->C_DeriveKey(this->session, &mech, this->pri_key,
								   attr, countof(attr), &secret);
	if (rv != CKR_OK)
	{
		DBG1(DBG_CFG, "C_DeriveKey() error: %N", ck_rv_names, rv);
		return FALSE;
	}
	if (!this->lib->get_ck_attribute(this->lib, this->session, secret,
									 CKA_VALUE, &this->secret))
	{
		chunk_free(&this->secret);
		return FALSE;
	}
	return TRUE;
}

METHOD(diffie_hellman_t, set_other_public_value, bool,
	private_pkcs11_dh_t *this, chunk_t value)
{
	if (!diffie_hellman_verify_value(this->group, value))
	{
		return FALSE;
	}

	switch (this->group)
	{
		case ECP_192_BIT:
		case ECP_224_BIT:
		case ECP_256_BIT:
		case ECP_384_BIT:
		case ECP_521_BIT:
		{	/* we expect the public value to just be the concatenated x and y
			 * coordinates, so we tag the value as an uncompressed ECPoint */
			chunk_t tag = chunk_from_chars(0x04);
			chunk_t pubkey = chunk_cata("cc", tag, value);
			CK_ECDH1_DERIVE_PARAMS params = {
				CKD_NULL,
				0,
				NULL,
				pubkey.len,
				pubkey.ptr,
			};

			if (!lib->settings->get_bool(lib->settings,
									"%s.ecp_x_coordinate_only", TRUE, lib->ns))
			{	/* we only get the x coordinate back */
				return FALSE;
			}
			value = chunk_from_thing(params);
			break;
		}
		default:
			break;
	}
	return derive_secret(this, value);
}

METHOD(diffie_hellman_t, get_my_public_value, bool,
	private_pkcs11_dh_t *this, chunk_t *value)
{
	*value = chunk_clone(this->pub_key);
	return TRUE;
}

METHOD(diffie_hellman_t, get_shared_secret, bool,
	private_pkcs11_dh_t *this, chunk_t *secret)
{
	if (!this->secret.ptr)
	{
		return FALSE;
	}
	*secret = chunk_clone(this->secret);
	return TRUE;
}

METHOD(diffie_hellman_t, get_dh_group, diffie_hellman_group_t,
	private_pkcs11_dh_t *this)
{
	return this->group;
}

METHOD(diffie_hellman_t, destroy, void,
	private_pkcs11_dh_t *this)
{
	this->lib->f->C_CloseSession(this->session);
	chunk_clear(&this->pub_key);
	chunk_clear(&this->secret);
	free(this);
}

/**
 * Generate a DH/ECDH key pair.
 *
 * If this succeeds, this->pri_key has a handle to the private key and
 * this->pub_key stores the public key.
 */
static bool generate_key_pair(private_pkcs11_dh_t *this, CK_ATTRIBUTE_PTR pub,
							  int pub_len, CK_ATTRIBUTE_PTR pri, int pri_len,
							  CK_ATTRIBUTE_TYPE attr)
{
	CK_MECHANISM mech = {
		this->mech_key,
		NULL,
		0,
	};
	CK_OBJECT_HANDLE pub_key;
	CK_RV rv;

	rv = this->lib->f->C_GenerateKeyPair(this->session, &mech, pub, pub_len,
							pri, pri_len, &pub_key, &this->pri_key);
	if (rv != CKR_OK)
	{
		DBG1(DBG_CFG, "C_GenerateKeyPair() error: %N", ck_rv_names, rv);
		return FALSE;
	}
	if (!this->lib->get_ck_attribute(this->lib, this->session, pub_key,
									 attr, &this->pub_key))
	{
		chunk_free(&this->pub_key);
		return FALSE;
	}
	return TRUE;
}

/**
 * Generate DH key pair.
 */
static bool generate_key_pair_modp(private_pkcs11_dh_t *this, size_t exp_len,
								   chunk_t g, chunk_t p)
{
	CK_BBOOL ck_true = CK_TRUE;
	CK_ATTRIBUTE pub_attr[] = {
		{ CKA_DERIVE, &ck_true, sizeof(ck_true) },
		{ CKA_PRIME, p.ptr, p.len },
		{ CKA_BASE, g.ptr, g.len },
	};
	CK_ULONG bits = exp_len * 8;
	CK_ATTRIBUTE pri_attr[] = {
		{ CKA_DERIVE, &ck_true, sizeof(ck_true) },
		{ CKA_VALUE_BITS, &bits, sizeof(bits) },
	};
	return generate_key_pair(this, pub_attr, countof(pub_attr), pri_attr,
							 countof(pri_attr), CKA_VALUE);
}

/**
 * Generate ECDH key pair.
 */
static bool generate_key_pair_ecp(private_pkcs11_dh_t *this,
								  chunk_t ecparams)
{
	CK_BBOOL ck_true = CK_TRUE;
	CK_ATTRIBUTE pub_attr[] = {
		{ CKA_DERIVE, &ck_true, sizeof(ck_true) },
		{ CKA_EC_PARAMS, ecparams.ptr, ecparams.len },
	};
	CK_ATTRIBUTE pri_attr[] = {
		{ CKA_DERIVE, &ck_true, sizeof(ck_true) },
	};
	chunk_t pub_key;
	if (!generate_key_pair(this, pub_attr, countof(pub_attr), pri_attr,
						   countof(pri_attr), CKA_EC_POINT))
	{
		return FALSE;
	}
	if (this->pub_key.len <= 0 || this->pub_key.ptr[0] != 0x04)
	{	/* we currently only support the point in uncompressed form which
		 * looks like this: 0x04 || x || y */
		chunk_clear(&this->pub_key);
		return FALSE;
	}
	pub_key = chunk_clone(chunk_skip(this->pub_key, 1));
	chunk_clear(&this->pub_key);
	this->pub_key = pub_key;
	return TRUE;
}

/**
 * Find a token we can use for DH/ECDH algorithm
 */
static pkcs11_library_t *find_token(private_pkcs11_dh_t *this,
									CK_SESSION_HANDLE *session)
{
	enumerator_t *tokens, *mechs;
	pkcs11_manager_t *manager;
	pkcs11_library_t *current, *found = NULL;
	CK_MECHANISM_TYPE type;
	CK_SLOT_ID slot;

	manager = lib->get(lib, "pkcs11-manager");
	if (!manager)
	{
		return NULL;
	}
	tokens = manager->create_token_enumerator(manager);
	while (tokens->enumerate(tokens, &current, &slot))
	{
		mechs = current->create_mechanism_enumerator(current, slot);
		while (mechs->enumerate(mechs, &type, NULL))
		{	/* we assume we can generate key pairs if the derive mechanism
			 * is supported */
			if (type == this->mech_derive)
			{
				if (current->f->C_OpenSession(slot, CKF_SERIAL_SESSION,
											  NULL, NULL, session) == CKR_OK)
				{
					found = current;
					break;
				}
			}
		}
		mechs->destroy(mechs);
		if (found)
		{
			break;
		}
	}
	tokens->destroy(tokens);
	return found;
}

/**
 * Generic internal constructor
 */
static private_pkcs11_dh_t *create_generic(diffie_hellman_group_t group,
										   CK_MECHANISM_TYPE key,
										   CK_MECHANISM_TYPE derive)
{
	private_pkcs11_dh_t *this;

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
		.mech_key = key,
		.mech_derive = derive,
	);

	this->lib = find_token(this, &this->session);
	if (!this->lib)
	{
		free(this);
		return NULL;
	}
	return this;
}

static pkcs11_dh_t *create_ecp(diffie_hellman_group_t group, chunk_t ecparam)
{
	private_pkcs11_dh_t *this = create_generic(group, CKM_EC_KEY_PAIR_GEN,
											   CKM_ECDH1_DERIVE);

	if (this)
	{
		if (generate_key_pair_ecp(this, ecparam))
		{
			chunk_free(&ecparam);
			return &this->public;
		}
		chunk_free(&ecparam);
		free(this);
	}
	return NULL;
}

/**
 * Constructor for MODP DH
 */
static pkcs11_dh_t *create_modp(diffie_hellman_group_t group, size_t exp_len,
								chunk_t g, chunk_t p)
{
	private_pkcs11_dh_t *this = create_generic(group, CKM_DH_PKCS_KEY_PAIR_GEN,
											   CKM_DH_PKCS_DERIVE);

	if (this)
	{
		if (generate_key_pair_modp(this, exp_len, g, p))
		{
			return &this->public;
		}
		free(this);
	}
	return NULL;
}

/**
 * Lookup the EC params for the given group.
 */
static chunk_t ecparams_lookup(diffie_hellman_group_t group)
{
	switch (group)
	{
		case ECP_192_BIT:
			return asn1_build_known_oid(OID_PRIME192V1);
		case ECP_224_BIT:
			return asn1_build_known_oid(OID_SECT224R1);
		case ECP_256_BIT:
			return asn1_build_known_oid(OID_PRIME256V1);
		case ECP_384_BIT:
			return asn1_build_known_oid(OID_SECT384R1);
		case ECP_521_BIT:
			return asn1_build_known_oid(OID_SECT521R1);
		default:
			break;
	}
	return chunk_empty;
}

/**
 * Described in header.
 */
pkcs11_dh_t *pkcs11_dh_create(diffie_hellman_group_t group, ...)
{
	switch (group)
	{
		case MODP_CUSTOM:
		{
			chunk_t g, p;

			VA_ARGS_GET(group, g, p);
			return create_modp(group, p.len, g, p);
		}
		case ECP_192_BIT:
		case ECP_224_BIT:
		case ECP_256_BIT:
		case ECP_384_BIT:
		case ECP_521_BIT:
		{
			chunk_t params = ecparams_lookup(group);
			if (params.ptr)
			{
				return create_ecp(group, params);
			}
			break;
		}
		default:
		{
			diffie_hellman_params_t *params = diffie_hellman_get_params(group);
			if (params)
			{
				return create_modp(group, params->exp_len, params->generator,
								   params->prime);
			}
			break;
		}
	}
	return NULL;
}
