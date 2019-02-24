/*
 * Copyright (C) 2009 Martin Willi
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

#include "pgp_utils.h"

#include <utils/debug.h>

ENUM_BEGIN(pgp_pubkey_alg_names, PGP_PUBKEY_ALG_RSA, PGP_PUBKEY_ALG_RSA_SIGN_ONLY,
	"RSA",
	"RSA_ENC_ONLY",
	"RSA_SIGN_ONLY"
);
ENUM_NEXT(pgp_pubkey_alg_names, PGP_PUBKEY_ALG_ELGAMAL_ENC_ONLY, PGP_PUBKEY_ALG_DIFFIE_HELLMAN, PGP_PUBKEY_ALG_RSA_SIGN_ONLY,
	"ELGAMAL_ENC_ONLY",
	"DSA",
	"ECC",
	"ECDSA",
	"ELGAMAL",
	"DIFFIE_HELLMAN"
);
ENUM_END(pgp_pubkey_alg_names, PGP_PUBKEY_ALG_DIFFIE_HELLMAN);

ENUM(pgp_sym_alg_names, PGP_SYM_ALG_PLAIN, PGP_SYM_ALG_TWOFISH,
	"PLAINTEXT",
	"IDEA",
	"3DES",
	"CAST5",
	"BLOWFISH",
	"SAFER",
	"DES",
	"AES_128",
	"AES_192",
	"AES_256",
	"TWOFISH"
);

ENUM_BEGIN(pgp_packet_tag_names, PGP_PKT_RESERVED, PGP_PKT_PUBLIC_SUBKEY,
	"Reserved",
	"Public-Key Encrypted Session Key Packet",
	"Signature Packet",
	"Symmetric-Key Encrypted Session Key Packet",
	"One-Pass Signature Packet",
	"Secret Key Packet",
	"Public Key Packet",
	"Secret Subkey Packet",
	"Compressed Data Packet",
	"Symmetrically Encrypted Data Packet",
	"Marker Packet",
	"Literal Data Packet",
	"Trust Packet",
	"User ID Packet",
	"Public Subkey Packet"
);
ENUM_NEXT(pgp_packet_tag_names, PGP_PKT_USER_ATTRIBUTE, PGP_PKT_MOD_DETECT_CODE, PGP_PKT_PUBLIC_SUBKEY,
	"User Attribute Packet",
	"Sym. Encrypted and Integrity Protected Data Packet",
	"Modification Detection Code Packet"
);
ENUM_END(pgp_packet_tag_names, PGP_PKT_MOD_DETECT_CODE);

/**
 * Read a PGP scalar of bytes length, advance blob
 */
bool pgp_read_scalar(chunk_t *blob, size_t bytes, uint32_t *scalar)
{
	uint32_t res = 0;

	if (bytes > blob->len)
	{
		DBG1(DBG_ASN, "PGP data too short to read %d byte scalar", bytes);
		return FALSE;
	}
	while (bytes-- > 0)
	{
		res = 256 * res + blob->ptr[0];
		*blob = chunk_skip(*blob, 1);
	}
	*scalar = res;
	return TRUE;
}

/**
 * Read a PGP MPI, advance blob
 */
bool pgp_read_mpi(chunk_t *blob, chunk_t *mpi)
{
	uint32_t bits, bytes;

	if (!pgp_read_scalar(blob, 2, &bits))
	{
		DBG1(DBG_ASN, "PGP data too short to read MPI length");
		return FALSE;
	}
	bytes = (bits + 7) / 8;
	if (bytes > blob->len)
	{
		DBG1(DBG_ASN, "PGP data too short to read %d byte MPI", bytes);
		return FALSE;
	}
	*mpi = chunk_create(blob->ptr, bytes);
	*blob = chunk_skip(*blob, bytes);
	return TRUE;
}

/**
 * Read length of an PGP old packet length encoding
 */
static bool pgp_old_packet_length(chunk_t *blob, uint32_t *length)
{
	/* bits 0 and 1 define the packet length type */
	u_char type;

	if (!blob->len)
	{
		return FALSE;
	}
	type = 0x03 & blob->ptr[0];
	*blob = chunk_skip(*blob, 1);

	if (type > 2)
	{
		return FALSE;
	}
	return pgp_read_scalar(blob, type == 0 ? 1 : type * 2, length);
}

/**
 * See header.
 */
bool pgp_read_packet(chunk_t *blob, chunk_t *data, pgp_packet_tag_t *tag)
{
	uint32_t len;
	u_char t;

	if (!blob->len)
	{
		DBG1(DBG_ASN, "missing input");
		return FALSE;
	}
	t = blob->ptr[0];

	/* bit 7 must be set */
	if (!(t & 0x80))
	{
		DBG1(DBG_ASN, "invalid packet tag");
		return FALSE;
	}
	/* bit 6 set defines new packet format */
	if (t & 0x40)
	{
		DBG1(DBG_ASN, "new PGP packet format not supported");
		return FALSE;
	}

	t = (t & 0x3C) >> 2;
	if (!pgp_old_packet_length(blob, &len) || len > blob->len)
	{
		DBG1(DBG_ASN, "invalid packet length");
		return FALSE;
	}
	*data = chunk_create(blob->ptr, len);
	*blob = chunk_skip(*blob, len);
	*tag = t;
	DBG2(DBG_ASN, "L1 - PGP %N (%u bytes)", pgp_packet_tag_names, t, len);
	DBG3(DBG_ASN, "%B", data);
	return TRUE;
}

