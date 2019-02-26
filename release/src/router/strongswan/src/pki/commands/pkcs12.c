/*
 * Copyright (C) 2014 Tobias Brunner
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

#include <errno.h>

#include "pki.h"

#include <credentials/certificates/x509.h>
#include <credentials/containers/pkcs12.h>

/**
 * Show info about PKCS#12 container
 */
static int show(pkcs12_t *pkcs12)
{
	enumerator_t *enumerator;
	certificate_t *cert;
	private_key_t *key;
	int index = 1;

	printf("Certificates:\n");
	enumerator = pkcs12->create_cert_enumerator(pkcs12);
	while (enumerator->enumerate(enumerator, &cert))
	{
		x509_t *x509 = (x509_t*)cert;

		if (x509->get_flags(x509) & X509_CA)
		{
			printf("[%2d] \"%Y\" (CA)\n", index++, cert->get_subject(cert));
		}
		else
		{
			printf("[%2d] \"%Y\"\n", index++, cert->get_subject(cert));
		}
	}
	enumerator->destroy(enumerator);

	printf("Private keys:\n");
	enumerator = pkcs12->create_key_enumerator(pkcs12);
	while (enumerator->enumerate(enumerator, &key))
	{
		printf("[%2d] %N %d bits\n", index++, key_type_names,
			   key->get_type(key), key->get_keysize(key));
	}
	enumerator->destroy(enumerator);
	return 0;
}

static int export(pkcs12_t *pkcs12, int index, char *outform)
{
	cred_encoding_type_t form;
	enumerator_t *enumerator;
	certificate_t *cert;
	private_key_t *key;
	chunk_t encoding;
	int i = 1;

	enumerator = pkcs12->create_cert_enumerator(pkcs12);
	while (enumerator->enumerate(enumerator, &cert))
	{
		if (i++ == index)
		{
			form = CERT_ASN1_DER;
			if (outform && !get_form(outform, &form, CRED_CERTIFICATE))
			{
				enumerator->destroy(enumerator);
				return command_usage("invalid output format");
			}
			if (cert->get_encoding(cert, form, &encoding))
			{
				set_file_mode(stdout, form);
				if (fwrite(encoding.ptr, encoding.len, 1, stdout) == 1)
				{
					free(encoding.ptr);
					enumerator->destroy(enumerator);
					return 0;
				}
				free(encoding.ptr);
			}
			fprintf(stderr, "certificate export failed\n");
			enumerator->destroy(enumerator);
			return 1;
		}
	}
	enumerator->destroy(enumerator);

	enumerator = pkcs12->create_key_enumerator(pkcs12);
	while (enumerator->enumerate(enumerator, &key))
	{
		if (i++ == index)
		{
			form = PRIVKEY_ASN1_DER;
			if (outform && !get_form(outform, &form, CRED_PRIVATE_KEY))
			{
				enumerator->destroy(enumerator);
				return command_usage("invalid output format");
			}
			if (key->get_encoding(key, form, &encoding))
			{
				set_file_mode(stdout, form);
				if (fwrite(encoding.ptr, encoding.len, 1, stdout) == 1)
				{
					free(encoding.ptr);
					enumerator->destroy(enumerator);
					return 0;
				}
				free(encoding.ptr);
			}
			fprintf(stderr, "private key export failed\n");
			enumerator->destroy(enumerator);
			return 0;
		}
	}
	enumerator->destroy(enumerator);

	fprintf(stderr, "invalid index %d\n", index);
	return 1;
}


/**
 * Handle PKCs#12 containers
 */
static int pkcs12()
{
	char *arg, *file = NULL, *outform = NULL;
	pkcs12_t *p12 = NULL;
	int res = 1, index = 0;
	enum {
		OP_NONE,
		OP_LIST,
		OP_EXPORT,
	} op = OP_NONE;

	while (TRUE)
	{
		switch (command_getopt(&arg))
		{
			case 'h':
				return command_usage(NULL);
			case 'i':
				file = arg;
				continue;
			case 'l':
				if (op != OP_NONE)
				{
					goto invalid;
				}
				op = OP_LIST;
				continue;
			case 'e':
				if (op != OP_NONE)
				{
					goto invalid;
				}
				op = OP_EXPORT;
				index = atoi(arg);
				continue;
			case 'f':
				outform = arg;
				continue;
			case EOF:
				break;
			default:
			invalid:
				return command_usage("invalid --pkcs12 option");
		}
		break;
	}

	if (file)
	{
		p12 = lib->creds->create(lib->creds, CRED_CONTAINER, CONTAINER_PKCS12,
								  BUILD_FROM_FILE, file, BUILD_END);
	}
	else
	{
		chunk_t chunk;

		set_file_mode(stdin, CERT_ASN1_DER);
		if (!chunk_from_fd(0, &chunk))
		{
			fprintf(stderr, "reading input failed: %s\n", strerror(errno));
			return 1;
		}
		p12 = lib->creds->create(lib->creds, CRED_CONTAINER, CONTAINER_PKCS12,
								  BUILD_BLOB, chunk, BUILD_END);
		free(chunk.ptr);
	}

	if (!p12)
	{
		fprintf(stderr, "reading input failed!\n");
		goto end;
	}

	switch (op)
	{
		case OP_LIST:
			res = show(p12);
			break;
		case OP_EXPORT:
			res = export(p12, index, outform);
			break;
		default:
			p12->container.destroy(&p12->container);
			return command_usage(NULL);
	}

end:
	if (p12)
	{
		p12->container.destroy(&p12->container);
	}
	return res;
}

/**
 * Register the command.
 */
static void __attribute__ ((constructor))reg()
{
	command_register((command_t) {
		pkcs12, 'u', "pkcs12", "PKCS#12 functions",
		{"--export index|--list [--in file]",
		 "[--outform der|pem]"},
		{
			{"help",	'h', 0, "show usage information"},
			{"in",		'i', 1, "input file, default: stdin"},
			{"list",	'l', 0, "list certificates and keys"},
			{"export",	'e', 1, "export the credential with the given index"},
			{"outform",	'f', 1, "encoding of exported credentials, default: der"},
		}
	});
}
