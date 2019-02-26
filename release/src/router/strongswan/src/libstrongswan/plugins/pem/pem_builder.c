/*
 * Copyright (C) 2013 Tobias Brunner
 * Copyright (C) 2009 Martin Willi
 * Copyright (C) 2001-2008 Andreas Steffen
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

#include "pem_builder.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stddef.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <utils/debug.h>
#include <library.h>
#include <utils/lexparser.h>
#include <asn1/asn1.h>
#include <crypto/hashers/hasher.h>
#include <crypto/crypters/crypter.h>
#include <credentials/certificates/x509.h>

#define PKCS5_SALT_LEN	8	/* bytes */

/**
 * check the presence of a pattern in a character string, skip if found
 */
static bool present(char* pattern, chunk_t* ch)
{
	u_int len = strlen(pattern);

	if (ch->len >= len && strneq(ch->ptr, pattern, len))
	{
		*ch = chunk_skip(*ch, len);
		return TRUE;
	}
	return FALSE;
}

/**
 * find a boundary of the form -----tag name-----
 */
static bool find_boundary(char* tag, chunk_t *line)
{
	chunk_t name = chunk_empty;

	if (!present("-----", line) ||
		!present(tag, line) ||
		!line->len || *line->ptr != ' ')
	{
		return FALSE;
	}
	*line = chunk_skip(*line, 1);

	/* extract name */
	name.ptr = line->ptr;
	while (line->len > 0)
	{
		if (present("-----", line))
		{
			DBG2(DBG_ASN, "  -----%s %.*s-----", tag, (int)name.len, name.ptr);
			return TRUE;
		}
		line->ptr++;  line->len--;  name.len++;
	}
	return FALSE;
}

/*
 * decrypts a passphrase protected encrypted data block
 */
static status_t pem_decrypt(chunk_t *blob, encryption_algorithm_t alg,
							size_t key_size, chunk_t iv, chunk_t passphrase)
{
	hasher_t *hasher;
	crypter_t *crypter;
	chunk_t salt = { iv.ptr, PKCS5_SALT_LEN };
	chunk_t hash;
	chunk_t decrypted;
	chunk_t key = {alloca(key_size), key_size};
	uint8_t padding, *last_padding_pos, *first_padding_pos;

	/* build key from passphrase and IV */
	hasher = lib->crypto->create_hasher(lib->crypto, HASH_MD5);
	if (hasher == NULL)
	{
		DBG1(DBG_ASN, "  MD5 hash algorithm not available");
		return NOT_SUPPORTED;
	}
	hash.len = hasher->get_hash_size(hasher);
	hash.ptr = alloca(hash.len);
	if (!hasher->get_hash(hasher, passphrase, NULL) ||
		!hasher->get_hash(hasher, salt, hash.ptr))
	{
		return FAILED;
	}
	memcpy(key.ptr, hash.ptr, hash.len);

	if (key.len > hash.len)
	{
		if (!hasher->get_hash(hasher, hash, NULL) ||
			!hasher->get_hash(hasher, passphrase, NULL) ||
			!hasher->get_hash(hasher, salt, hash.ptr))
		{
			return FAILED;
		}
		memcpy(key.ptr + hash.len, hash.ptr, key.len - hash.len);
	}
	hasher->destroy(hasher);

	/* decrypt blob */
	crypter = lib->crypto->create_crypter(lib->crypto, alg, key_size);
	if (crypter == NULL)
	{
		DBG1(DBG_ASN, "  %N encryption algorithm not available",
			 encryption_algorithm_names, alg);
		return NOT_SUPPORTED;
	}

	if (iv.len != crypter->get_iv_size(crypter) ||
		blob->len % crypter->get_block_size(crypter))
	{
		crypter->destroy(crypter);
		DBG1(DBG_ASN, "  data size is not multiple of block size");
		return PARSE_ERROR;
	}
	if (!crypter->set_key(crypter, key) ||
		!crypter->decrypt(crypter, *blob, iv, &decrypted))
	{
		crypter->destroy(crypter);
		return FAILED;
	}
	crypter->destroy(crypter);
	memcpy(blob->ptr, decrypted.ptr, blob->len);
	chunk_free(&decrypted);

	/* determine amount of padding */
	last_padding_pos = blob->ptr + blob->len - 1;
	padding = *last_padding_pos;
	if (padding > blob->len)
	{
		first_padding_pos = blob->ptr;
	}
	else
	{
		first_padding_pos = last_padding_pos - padding;
	}
	/* check the padding pattern */
	while (--last_padding_pos > first_padding_pos)
	{
		if (*last_padding_pos != padding)
		{
			DBG1(DBG_ASN, "  invalid passphrase");
			return INVALID_ARG;
		}
	}
	/* remove padding */
	blob->len -= padding;
	return SUCCESS;
}

/**
 * Converts a PEM encoded file into its binary form (RFC 1421, RFC 934)
 */
static status_t pem_to_bin(chunk_t *blob, bool *pgp)
{
	typedef enum {
		PEM_PRE    = 0,
		PEM_MSG    = 1,
		PEM_HEADER = 2,
		PEM_BODY   = 3,
		PEM_POST   = 4,
		PEM_ABORT  = 5
	} state_t;

	encryption_algorithm_t alg = ENCR_UNDEFINED;
	size_t key_size = 0;
	bool encrypted = FALSE;
	state_t state  = PEM_PRE;
	chunk_t src    = *blob;
	chunk_t dst    = *blob;
	chunk_t line   = chunk_empty;
	chunk_t iv     = chunk_empty;
	u_char iv_buf[HASH_SIZE_MD5];
	status_t status = NOT_FOUND;
	enumerator_t *enumerator;
	shared_key_t *shared;

	dst.len = 0;
	iv.ptr = iv_buf;
	iv.len = 0;

	while (fetchline(&src, &line))
	{
		if (state == PEM_PRE)
		{
			if (find_boundary("BEGIN", &line))
			{
				state = PEM_MSG;
			}
			continue;
		}
		else
		{
			if (find_boundary("END", &line))
			{
				state = PEM_POST;
				break;
			}
			if (state == PEM_MSG)
			{
				state = PEM_HEADER;
				if (memchr(line.ptr, ':', line.len) == NULL)
				{
					state = PEM_BODY;
				}
			}
			if (state == PEM_HEADER)
			{
				err_t ugh = NULL;
				chunk_t name  = chunk_empty;
				chunk_t value = chunk_empty;

				/* an empty line separates HEADER and BODY */
				if (line.len == 0)
				{
					state = PEM_BODY;
					continue;
				}

				/* we are looking for a parameter: value pair */
				DBG2(DBG_ASN, "  %.*s", (int)line.len, line.ptr);
				ugh = extract_parameter_value(&name, &value, &line);
				if (ugh != NULL)
				{
					continue;
				}
				if (match("Proc-Type", &name) && value.len && *value.ptr == '4')
				{
					encrypted = TRUE;
				}
				else if (match("DEK-Info", &name))
				{
					chunk_t dek;

					if (!extract_token(&dek, ',', &value))
					{
						dek = value;
					}
					if (match("DES-EDE3-CBC", &dek))
					{
						alg = ENCR_3DES;
						key_size = 24;
					}
					else if (match("AES-128-CBC", &dek))
					{
						alg = ENCR_AES_CBC;
						key_size = 16;
					}
					else if (match("AES-192-CBC", &dek))
					{
						alg = ENCR_AES_CBC;
						key_size = 24;
					}
					else if (match("AES-256-CBC", &dek))
					{
						alg = ENCR_AES_CBC;
						key_size = 32;
					}
					else
					{
						DBG1(DBG_ASN, "  encryption algorithm '%.*s'"
							 " not supported", (int)dek.len, dek.ptr);
						return NOT_SUPPORTED;
					}
					if (!eat_whitespace(&value) || value.len > 2*sizeof(iv_buf))
					{
						return PARSE_ERROR;
					}
					iv = chunk_from_hex(value, iv_buf);
				}
			}
			else /* state is PEM_BODY */
			{
				chunk_t data;

				/* remove any trailing whitespace */
				if (!extract_token(&data ,' ', &line))
				{
					data = line;
				}

				/* check for PGP armor checksum */
				if (data.len && *data.ptr == '=')
				{
					*pgp = TRUE;
					data.ptr++;
					data.len--;
					DBG2(DBG_ASN, "  armor checksum: %.*s", (int)data.len,
						 data.ptr);
					continue;
				}

				if (blob->len - dst.len < data.len / 4 * 3)
				{
					state = PEM_ABORT;
				}
				data = chunk_from_base64(data, dst.ptr);

				dst.ptr += data.len;
				dst.len += data.len;
			}
		}
	}
	/* set length to size of binary blob */
	blob->len = dst.len;

	if (state != PEM_POST)
	{
		DBG1(DBG_LIB, "  file coded in unknown format, discarded");
		return PARSE_ERROR;
	}
	if (!encrypted)
	{
		return SUCCESS;
	}

	enumerator = lib->credmgr->create_shared_enumerator(lib->credmgr,
											SHARED_PRIVATE_KEY_PASS, NULL, NULL);
	while (enumerator->enumerate(enumerator, &shared, NULL, NULL))
	{
		chunk_t passphrase, chunk;

		passphrase = shared->get_key(shared);
		chunk = chunk_clone(*blob);
		status = pem_decrypt(&chunk, alg, key_size, iv, passphrase);
		if (status == SUCCESS)
		{
			memcpy(blob->ptr, chunk.ptr, chunk.len);
			blob->len = chunk.len;
		}
		free(chunk.ptr);
		if (status != INVALID_ARG)
		{	/* try again only if passphrase invalid */
			break;
		}
	}
	enumerator->destroy(enumerator);
	return status;
}

/**
 * Check if a blob looks like an ASN1 SEQUENCE or SET with BER indefinite length
 */
static bool is_ber_indefinite_length(chunk_t blob)
{
	if (blob.len >= 4)
	{
		switch (blob.ptr[0])
		{
			case ASN1_SEQUENCE:
			case ASN1_SET:
				/* BER indefinite length uses 0x80, and is terminated with
				 * end-of-content using 0x00,0x00 */
				return blob.ptr[1] == 0x80 &&
					   blob.ptr[blob.len - 2] == 0 &&
					   blob.ptr[blob.len - 1] == 0;
			default:
				break;
		}
	}
	return FALSE;
}

/**
 * load the credential from a blob
 */
static void *load_from_blob(chunk_t blob, credential_type_t type, int subtype,
							identification_t *subject, x509_flag_t flags)
{
	void *cred = NULL;
	bool pgp = FALSE;

	blob = chunk_clone(blob);
	if (!is_ber_indefinite_length(blob) && !is_asn1(blob))
	{
		if (pem_to_bin(&blob, &pgp) != SUCCESS)
		{
			chunk_clear(&blob);
			return NULL;
		}
		if (pgp && type == CRED_PRIVATE_KEY)
		{
			/* PGP encoded keys are parsed with a KEY_ANY key type, as it
			 * can contain any type of key. However, ipsec.secrets uses
			 * RSA for PGP keys, which is actually wrong. */
			subtype = KEY_ANY;
		}
	}
	/* if CERT_ANY is given, ASN1 encoded blob is handled as X509 */
	if (type == CRED_CERTIFICATE && subtype == CERT_ANY)
	{
		subtype = pgp ? CERT_GPG : CERT_X509;
	}
	if (type == CRED_CERTIFICATE && subtype == CERT_TRUSTED_PUBKEY && subject)
	{
		cred = lib->creds->create(lib->creds, type, subtype,
							  BUILD_BLOB_ASN1_DER, blob, BUILD_SUBJECT, subject,
							  BUILD_END);
	}
	else
	{
		cred = lib->creds->create(lib->creds, type, subtype,
							  pgp ? BUILD_BLOB_PGP : BUILD_BLOB_ASN1_DER, blob,
							  flags ? BUILD_X509_FLAG : BUILD_END,
							  flags, BUILD_END);
	}
	chunk_clear(&blob);
	return cred;
}

/**
 * load the credential from a file
 */
static void *load_from_file(char *file, credential_type_t type, int subtype,
							identification_t *subject, x509_flag_t flags)
{
	void *cred;
	chunk_t *chunk;

	chunk = chunk_map(file, FALSE);
	if (!chunk)
	{
		DBG1(DBG_LIB, "  opening '%s' failed: %s", file, strerror(errno));
		return NULL;
	}
	cred = load_from_blob(*chunk, type, subtype, subject, flags);
	chunk_unmap(chunk);
	return cred;
}

/**
 * Load all kind of PEM encoded credentials.
 */
static void *pem_load(credential_type_t type, int subtype, va_list args)
{
	char *file = NULL;
	chunk_t pem = chunk_empty;
	identification_t *subject = NULL;
	int flags = 0;

	while (TRUE)
	{
		switch (va_arg(args, builder_part_t))
		{
			case BUILD_FROM_FILE:
				file = va_arg(args, char*);
				continue;
			case BUILD_BLOB:
			case BUILD_BLOB_PEM:
				pem = va_arg(args, chunk_t);
				continue;
			case BUILD_SUBJECT:
				subject = va_arg(args, identification_t*);
				continue;
			case BUILD_X509_FLAG:
				flags = va_arg(args, int);
				continue;
			case BUILD_END:
				break;
			default:
				return NULL;
		}
		break;
	}

	if (pem.len)
	{
		return load_from_blob(pem, type, subtype, subject, flags);
	}
	if (file)
	{
		return load_from_file(file, type, subtype, subject, flags);
	}
	return NULL;
}

/**
 * Private key PEM loader.
 */
private_key_t *pem_private_key_load(key_type_t type, va_list args)
{
	return pem_load(CRED_PRIVATE_KEY, type, args);
}

/**
 * Public key PEM loader.
 */
public_key_t *pem_public_key_load(key_type_t type, va_list args)
{
	return pem_load(CRED_PUBLIC_KEY, type, args);
}

/**
 * Certificate PEM loader.
 */
certificate_t *pem_certificate_load(certificate_type_t type, va_list args)
{
	return pem_load(CRED_CERTIFICATE, type, args);
}

/**
 * Container PEM loader.
 */
container_t *pem_container_load(container_type_t type, va_list args)
{
	return pem_load(CRED_CONTAINER, type, args);
}
