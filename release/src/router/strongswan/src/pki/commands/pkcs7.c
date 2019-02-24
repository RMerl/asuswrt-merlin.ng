/*
 * Copyright (C) 2012 Martin Willi
 * Copyright (C) 2012 revosec AG
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

#include "pki.h"

#include <asn1/oid.h>
#include <asn1/asn1.h>
#include <credentials/containers/pkcs7.h>
#include <credentials/sets/mem_cred.h>

/**
 * Read input data as chunk
 */
static chunk_t read_from_stream(FILE *stream)
{
	char buf[8096];
	size_t len, total = 0;

	while (TRUE)
	{
		len = fread(buf + total, 1, sizeof(buf) - total, stream);
		if (len < (sizeof(buf) - total))
		{
			if (ferror(stream))
			{
				return chunk_empty;
			}
			if (feof(stream))
			{
				return chunk_clone(chunk_create(buf, total + len));
			}
		}
		total += len;
		if (total == sizeof(buf))
		{
			fprintf(stderr, "buffer too small to read input!\n");
			return chunk_empty;
		}
	}
}

/**
 * Write output data from chunk to stream
 */
static bool write_to_stream(FILE *stream, chunk_t data)
{
	size_t len, total = 0;

	set_file_mode(stream, CERT_ASN1_DER);
	while (total < data.len)
	{
		len = fwrite(data.ptr + total, 1, data.len - total, stream);
		if (len <= 0)
		{
			return FALSE;
		}
		total += len;
	}
	return TRUE;
}

/**
 * Verify PKCS#7 signed-data
 */
static int verify(chunk_t chunk)
{
	container_t *container;
	pkcs7_t *pkcs7;
	enumerator_t *enumerator;
	certificate_t *cert;
	auth_cfg_t *auth;
	chunk_t data;
	time_t t;
	bool verified = FALSE;

	container = lib->creds->create(lib->creds, CRED_CONTAINER, CONTAINER_PKCS7,
								   BUILD_BLOB_ASN1_DER, chunk, BUILD_END);
	if (!container)
	{
		return 1;
	}

	if (container->get_type(container) != CONTAINER_PKCS7_SIGNED_DATA)
	{
		fprintf(stderr, "verification failed, container is %N\n",
				container_type_names, container->get_type(container));
		container->destroy(container);
		return 1;
	}

	pkcs7 = (pkcs7_t*)container;
	enumerator = container->create_signature_enumerator(container);
	while (enumerator->enumerate(enumerator, &auth))
	{
		verified = TRUE;
		cert = auth->get(auth, AUTH_RULE_SUBJECT_CERT);
		if (cert)
		{
			fprintf(stderr, "signed by '%Y'", cert->get_subject(cert));

			if (pkcs7->get_attribute(pkcs7, OID_PKCS9_SIGNING_TIME,
									 enumerator, &data))
			{
				t = asn1_to_time(&data, ASN1_UTCTIME);
				if (t != UNDEFINED_TIME)
				{
					fprintf(stderr, " at %T", &t, FALSE);
				}
				free(data.ptr);
			}
			fprintf(stderr, "\n");
		}
	}
	enumerator->destroy(enumerator);

	if (!verified)
	{
		fprintf(stderr, "no trusted signature found\n");
	}

	if (verified)
	{
		if (container->get_data(container, &data))
		{
			write_to_stream(stdout, data);
			free(data.ptr);
		}
		else
		{
			verified = FALSE;
		}
	}
	container->destroy(container);

	return verified ? 0 : 1;
}

/**
 * Sign data into PKCS#7 signed-data
 */
static int sign(chunk_t chunk, certificate_t *cert, private_key_t *key)
{
	container_t *container;
	chunk_t encoding;
	int res = 1;

	container = lib->creds->create(lib->creds,
								   CRED_CONTAINER, CONTAINER_PKCS7_SIGNED_DATA,
								   BUILD_BLOB, chunk,
								   BUILD_SIGNING_CERT, cert,
								   BUILD_SIGNING_KEY, key,
								   BUILD_END);
	if (container)
	{
		if (container->get_encoding(container, &encoding))
		{
			write_to_stream(stdout, encoding);
			free(encoding.ptr);
		}
		container->destroy(container);
	}
	return res;
}

/**
 * Encrypt data to a PKCS#7 enveloped-data
 */
static int encrypt(chunk_t chunk, certificate_t *cert)
{
	container_t *container;
	chunk_t encoding;
	int res = 1;

	container = lib->creds->create(lib->creds,
								   CRED_CONTAINER, CONTAINER_PKCS7_ENVELOPED_DATA,
								   BUILD_BLOB, chunk, BUILD_CERT, cert,
								   BUILD_END);
	if (container)
	{
		if (container->get_encoding(container, &encoding))
		{
			write_to_stream(stdout, encoding);
			free(encoding.ptr);
		}
		container->destroy(container);
	}
	return res;
}

/**
 * Decrypt PKCS#7 enveloped-data
 */
static int decrypt(chunk_t chunk)
{
	container_t *container;
	chunk_t data;

	container = lib->creds->create(lib->creds, CRED_CONTAINER, CONTAINER_PKCS7,
								   BUILD_BLOB_ASN1_DER, chunk, BUILD_END);
	if (!container)
	{
		return 1;
	}
	if (container->get_type(container) != CONTAINER_PKCS7_ENVELOPED_DATA)
	{
		fprintf(stderr, "decryption failed, container is %N\n",
				container_type_names, container->get_type(container));
		container->destroy(container);
		return 1;
	}
	if (!container->get_data(container, &data))
	{
		fprintf(stderr, "PKCS#7 decryption failed\n");
		container->destroy(container);
		return 1;
	}
	container->destroy(container);

	write_to_stream(stdout, data);
	free(data.ptr);

	return 0;
}

/**
 * Show info about PKCS#7 container
 */
static int show(chunk_t chunk)
{
	container_t *container;
	pkcs7_t *pkcs7;
	enumerator_t *enumerator;
	certificate_t *cert;
	chunk_t data;

	container = lib->creds->create(lib->creds, CRED_CONTAINER, CONTAINER_PKCS7,
								   BUILD_BLOB_ASN1_DER, chunk, BUILD_END);
	if (!container)
	{
		return 1;
	}
	fprintf(stderr, "%N\n", container_type_names, container->get_type(container));

	if (container->get_type(container) == CONTAINER_PKCS7_SIGNED_DATA)
	{
		pkcs7 = (pkcs7_t*)container;
		enumerator = pkcs7->create_cert_enumerator(pkcs7);
		while (enumerator->enumerate(enumerator, &cert))
		{
			if (cert->get_encoding(cert, CERT_PEM, &data))
			{
				printf("%.*s", (int)data.len, data.ptr);
				free(data.ptr);
			}
		}
		enumerator->destroy(enumerator);
	}
	container->destroy(container);
	return 0;
}

/**
 * Wrap/Unwrap PKCs#7 containers
 */
static int pkcs7()
{
	char *arg, *file = NULL;
	private_key_t *key = NULL;
	certificate_t *cert = NULL;
	chunk_t data = chunk_empty;
	mem_cred_t *creds;
	int res = 1;
	FILE *in;
	enum {
		OP_NONE,
		OP_SIGN,
		OP_VERIFY,
		OP_ENCRYPT,
		OP_DECRYPT,
		OP_SHOW,
	} op = OP_NONE;

	creds = mem_cred_create();

	while (TRUE)
	{
		switch (command_getopt(&arg))
		{
			case 'h':
				creds->destroy(creds);
				return command_usage(NULL);
			case 'i':
				file = arg;
				continue;
			case 's':
				if (op != OP_NONE)
				{
					goto invalid;
				}
				op = OP_SIGN;
				continue;
			case 'u':
				if (op != OP_NONE)
				{
					goto invalid;
				}
				op = OP_VERIFY;
				continue;
			case 'e':
				if (op != OP_NONE)
				{
					goto invalid;
				}
				op = OP_ENCRYPT;
				continue;
			case 'd':
				if (op != OP_NONE)
				{
					goto invalid;
				}
				op = OP_DECRYPT;
				continue;
			case 'p':
				if (op != OP_NONE)
				{
					goto invalid;
				}
				op = OP_SHOW;
				continue;
			case 'k':
				key = lib->creds->create(lib->creds,
										 CRED_PRIVATE_KEY, KEY_RSA,
										 BUILD_FROM_FILE, arg, BUILD_END);
				if (!key)
				{
					fprintf(stderr, "parsing private key failed\n");
					goto end;
				}
				creds->add_key(creds, key);
				continue;
			case 'c':
				cert = lib->creds->create(lib->creds,
										  CRED_CERTIFICATE, CERT_X509,
										  BUILD_FROM_FILE, arg, BUILD_END);
				if (!cert)
				{
					fprintf(stderr, "parsing certificate failed\n");
					goto end;
				}
				creds->add_cert(creds, TRUE, cert);
				continue;
			case EOF:
				break;
			default:
			invalid:
				creds->destroy(creds);
				return command_usage("invalid --pkcs7 option");
		}
		break;
	}

	if (file)
	{
		in = fopen(file, "r");
		if (in)
		{
			data = read_from_stream(in);
			fclose(in);
		}
	}
	else
	{
		data = read_from_stream(stdin);
	}

	if (!data.len)
	{
		fprintf(stderr, "reading input failed!\n");
		goto end;
	}
	if (op != OP_SHOW && !cert)
	{
		fprintf(stderr, "requiring a certificate!\n");
		goto end;
	}

	lib->credmgr->add_local_set(lib->credmgr, &creds->set, FALSE);

	switch (op)
	{
		case OP_SIGN:
			if (!key)
			{
				fprintf(stderr, "signing requires a private key\n");
				res = 1;
				break;
			}
			res = sign(data, cert, key);
			break;
		case OP_VERIFY:
			res = verify(data);
			break;
		case OP_ENCRYPT:
			res = encrypt(data, cert);
			break;
		case OP_DECRYPT:
			if (!key)
			{
				fprintf(stderr, "decryption requires a private key\n");
				res = 1;
				break;
			}
			res = decrypt(data);
			break;
		case OP_SHOW:
			res = show(data);
			break;
		default:
			res = 1;
			break;
	}
	lib->credmgr->remove_local_set(lib->credmgr, &creds->set);

end:
	creds->destroy(creds);
	free(data.ptr);
	return res;
}

/**
 * Register the command.
 */
static void __attribute__ ((constructor))reg()
{
	command_register((command_t) {
		pkcs7, '7', "pkcs7", "PKCS#7 wrap/unwrap functions",
		{"--sign|--verify|--encrypt|--decrypt|--show",
		 "[--in file] [--cert file]+ [--key file]"},
		{
			{"help",	'h', 0, "show usage information"},
			{"sign",	's', 0, "create PKCS#7 signed-data"},
			{"verify",	'u', 0, "verify PKCS#7 signed-data"},
			{"encrypt",	'e', 0, "create PKCS#7 enveloped-data"},
			{"decrypt",	'd', 0, "decrypt PKCS#7 enveloped-data"},
			{"show",	'p', 0, "show info about PKCS#7, print certificates"},
			{"in",		'i', 1, "input file, default: stdin"},
			{"key",		'k', 1, "path to private key for sign/decrypt"},
			{"cert",	'c', 1, "path to certificate for sign/verify/encrypt"},
		}
	});
}
