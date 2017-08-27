/*
 * Copyright (C) 2009 Martin Willi
 * Copyright (C) 2002-2009 Andreas Steffen
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

#include "pgp_builder.h"
#include "pgp_utils.h"

#include <utils/utils.h>
#include <utils/debug.h>
#include <credentials/keys/private_key.h>

/**
 * Load a generic public key from a PGP packet
 */
static public_key_t *parse_public_key(chunk_t blob)
{
	u_int32_t alg;
	public_key_t *key;

	if (!pgp_read_scalar(&blob, 1, &alg))
	{
		return NULL;
	}
	switch (alg)
	{
		case PGP_PUBKEY_ALG_RSA:
		case PGP_PUBKEY_ALG_RSA_SIGN_ONLY:
			key = lib->creds->create(lib->creds, CRED_PUBLIC_KEY, KEY_RSA,
									 BUILD_BLOB_PGP, blob, BUILD_END);
			break;
		default:
			DBG1(DBG_LIB, "PGP public key algorithm %N not supported",
				 pgp_pubkey_alg_names, alg);
			return NULL;
	}
	return key;
}

/**
 * Load a RSA public key from a PGP packet
 */
static public_key_t *parse_rsa_public_key(chunk_t blob)
{
	chunk_t mpi[2];
	int i;

	for (i = 0; i < 2; i++)
	{
		if (!pgp_read_mpi(&blob, &mpi[i]))
		{
			return NULL;
		}
	}
	return lib->creds->create(lib->creds, CRED_PUBLIC_KEY, KEY_RSA,
						BUILD_RSA_MODULUS, mpi[0], BUILD_RSA_PUB_EXP, mpi[1],
						BUILD_END);
}

/**
 * Load a RSA private key from a PGP packet
 */
static private_key_t *parse_rsa_private_key(chunk_t blob)
{
	chunk_t mpi[6];
	u_int32_t s2k;
	int i;

	for (i = 0; i < 2; i++)
	{
		if (!pgp_read_mpi(&blob, &mpi[i]))
		{
			return NULL;
		}
	}
	if (!pgp_read_scalar(&blob, 1, &s2k))
	{
		return NULL;
	}
	if (s2k == 255 || s2k == 254)
	{
		DBG1(DBG_LIB, "string-to-key specifiers not supported");
		return NULL;
	}
	if (s2k != PGP_SYM_ALG_PLAIN)
	{
		DBG1(DBG_LIB, "%N private key encryption not supported",
			 pgp_sym_alg_names, s2k);
		return NULL;
	}

	for (i = 2; i < 6; i++)
	{
		if (!pgp_read_mpi(&blob, &mpi[i]))
		{
			return NULL;
		}
	}

	/* PGP has uses p < q, but we use p > q */
	return lib->creds->create(lib->creds, CRED_PRIVATE_KEY, KEY_RSA,
						BUILD_RSA_MODULUS, mpi[0], BUILD_RSA_PUB_EXP, mpi[1],
						BUILD_RSA_PRIV_EXP, mpi[2], BUILD_RSA_PRIME2, mpi[3],
						BUILD_RSA_PRIME1, mpi[4], BUILD_RSA_COEFF, mpi[5],
						BUILD_END);
}

/**
 * Implementation of private_key_t.sign for encryption-only keys
 */
static bool sign_not_allowed(private_key_t *this, signature_scheme_t scheme,
							 chunk_t data, chunk_t *signature)
{
	DBG1(DBG_LIB, "signing failed - decryption only key");
	return FALSE;
}

/**
 * Implementation of private_key_t.decrypt for signature-only keys
 */
static bool decrypt_not_allowed(private_key_t *this, encryption_scheme_t scheme,
								chunk_t crypto, chunk_t *plain)
{
	DBG1(DBG_LIB, "decryption failed - signature only key");
	return FALSE;
}

/**
 * Load a generic private key from a PGP packet
 */
static private_key_t *parse_private_key(chunk_t blob)
{
	chunk_t packet;
	pgp_packet_tag_t tag;
	u_int32_t version, created, days, alg;
	private_key_t *key;

	if (!pgp_read_packet(&blob, &packet, &tag))
	{
		return NULL;
	}
	if (!pgp_read_scalar(&packet, 1, &version))
	{
		return NULL;
	}
	switch (version)
	{
		case 3:
			if (!pgp_read_scalar(&packet, 2, &days))
			{
				return NULL;
			}
			break;
		case 4:
			break;
		default:
			DBG1(DBG_LIB, "PGP packet version V%d not supported", version);
			return NULL;
	}
	if (!pgp_read_scalar(&packet, 4, &created))
	{
		return NULL;
	}
	if (!pgp_read_scalar(&packet, 1, &alg))
	{
		return NULL;
	}
	switch (alg)
	{
		case PGP_PUBKEY_ALG_RSA:
			return lib->creds->create(lib->creds, CRED_PRIVATE_KEY, KEY_RSA,
									  BUILD_BLOB_PGP, packet, BUILD_END);
		case PGP_PUBKEY_ALG_RSA_ENC_ONLY:
			key = lib->creds->create(lib->creds, CRED_PRIVATE_KEY, KEY_RSA,
									  BUILD_BLOB_PGP, packet, BUILD_END);
			if (key)
			{
				key->sign = sign_not_allowed;
			}
			return key;
		case PGP_PUBKEY_ALG_RSA_SIGN_ONLY:
			key = lib->creds->create(lib->creds, CRED_PRIVATE_KEY, KEY_RSA,
									  BUILD_BLOB_PGP, packet, BUILD_END);
			if (key)
			{
				key->decrypt = decrypt_not_allowed;
			}
			return key;
		case PGP_PUBKEY_ALG_ECDSA:
			return lib->creds->create(lib->creds, CRED_PRIVATE_KEY, KEY_ECDSA,
									  BUILD_BLOB_PGP, packet, BUILD_END);
		case PGP_PUBKEY_ALG_ELGAMAL_ENC_ONLY:
		case PGP_PUBKEY_ALG_DSA:
		case PGP_PUBKEY_ALG_ECC:
		case PGP_PUBKEY_ALG_ELGAMAL:
		case PGP_PUBKEY_ALG_DIFFIE_HELLMAN:
		default:
			return NULL;
	}
}

/**
 *  See header.
 */
public_key_t *pgp_public_key_load(key_type_t type, va_list args)
{
	chunk_t blob = chunk_empty;

	while (TRUE)
	{
		switch (va_arg(args, builder_part_t))
		{
			case BUILD_BLOB_PGP:
				blob = va_arg(args, chunk_t);
				continue;
			case BUILD_END:
				break;
			default:
				return NULL;
		}
		break;
	}
	switch (type)
	{
		case KEY_ANY:
			return parse_public_key(blob);
		case KEY_RSA:
			return parse_rsa_public_key(blob);
		default:
			return NULL;
	}
}

/**
 * See header.
 */
private_key_t *pgp_private_key_load(key_type_t type, va_list args)
{
	chunk_t blob = chunk_empty;

	while (TRUE)
	{
		switch (va_arg(args, builder_part_t))
		{
			case BUILD_BLOB_PGP:
				blob = va_arg(args, chunk_t);
				continue;
			case BUILD_END:
				break;
			default:
				return NULL;
		}
		break;
	}
	switch (type)
	{
		case KEY_ANY:
			return parse_private_key(blob);
		case KEY_RSA:
			return parse_rsa_private_key(blob);
		default:
			return NULL;
	}
}
