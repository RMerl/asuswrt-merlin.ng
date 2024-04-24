/*
 * Copyright (C) 2022 Andreas Steffen, strongSec GmbH
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

#define _GNU_SOURCE
#include <stdio.h>
#include <errno.h>

#include "pki.h"
#include "pki_cert.h"

#include <credentials/certificates/certificate.h>
#include <credentials/certificates/x509.h>
#include <credentials/containers/pkcs7.h>

/*
 * Certificate types
 */
typedef enum {
    CERT_TYPE_ROOT_CA,
    CERT_TYPE_SUB_CA,
    CERT_TYPE_RA
} pki_cert_type_t;

static char *cert_type_label[] DBG_UNUSED = { "Root CA", "Sub CA", "RA" };

/**
 * Determine certificate type based on X.509 certificate flags
 */
static pki_cert_type_t get_pki_cert_type(certificate_t *cert)
{
	x509_t *x509;
	x509_flag_t flags;

	x509 = (x509_t*)cert;
	flags = x509->get_flags(x509);

	if (flags & X509_CA)
	{
		if (flags & X509_SELF_SIGNED)
		{
			return CERT_TYPE_ROOT_CA;
		}
		else
		{
			return CERT_TYPE_SUB_CA;
		}
	}
	else
	{
		return CERT_TYPE_RA;
	}
}

/**
 * Output cert type, subject as well as SHA256 and SHA1 fingerprints
 */
static bool print_cert_info(certificate_t *cert, pki_cert_type_t cert_type)
{
	hasher_t *hasher = NULL;
	char digest_buf[HASH_SIZE_SHA256];
	char base64_buf[HASH_SIZE_SHA256];
	chunk_t cert_digest = {digest_buf, HASH_SIZE_SHA256};
	chunk_t cert_id DBG_UNUSED, serial DBG_UNUSED, encoding = chunk_empty;
	x509_t *x509;
	bool success = FALSE;

	DBG1(DBG_APP, "%s cert \"%Y\"", cert_type_label[cert_type],
									cert->get_subject(cert));
	x509 = (x509_t*)cert;
	serial = x509->get_serial(x509);
	DBG1(DBG_APP, "  serial: %#B", &serial);

	if (!cert->get_encoding(cert, CERT_ASN1_DER, &encoding))
	{
		DBG1(DBG_APP, "could not get certificate encoding");
		return FALSE;
	}

	/* SHA256 certificate digest */
	hasher = lib->crypto->create_hasher(lib->crypto, HASH_SHA256);
	if (!hasher)
	{
		DBG1(DBG_APP, "could not create SHA256 hasher");
		goto end;
	}
	if (!hasher->get_hash(hasher, encoding, digest_buf))
	{
		DBG1(DBG_APP, "could not compute SHA256 hash");
		goto end;
	}
	hasher->destroy(hasher);

	DBG1(DBG_APP, "  SHA256: %#B", &cert_digest);

	/* SHA1 certificate digest */
	hasher = lib->crypto->create_hasher(lib->crypto, HASH_SHA1);
	if (!hasher)
	{
		DBG1(DBG_APP, "could not create SHA1 hasher");
		goto end;
	}
	if (!hasher->get_hash(hasher, encoding, digest_buf))
	{
		DBG1(DBG_APP, "could not compute SHA1 hash");
		goto end;
	}
	cert_digest.len = HASH_SIZE_SHA1;
	cert_id = chunk_to_base64(cert_digest, base64_buf);

	DBG1(DBG_APP, "  SHA1  : %#B (%.*s)", &cert_digest,
										   cert_id.len-1, cert_id.ptr);
	success = TRUE;

end:
	DESTROY_IF(hasher);
	chunk_free(&encoding);

	return success;
}

/**
 * Build a CA or RA pathname
 */
static bool build_pathname(char **path, pki_cert_type_t cert_type,
						   int *cert_type_count, char *caout, char *raout,
						   cred_encoding_type_t form)
{
	char *basename, *extension, *dot, *suffix;
	int  count, len;
	bool number;

	basename = caout;
	extension = "";
	suffix = (form == CERT_ASN1_DER) ? "der" : "pem";

	count = cert_type_count[cert_type];
	number = count > 1;

	switch (cert_type)
	{
		default:
		case CERT_TYPE_ROOT_CA:
			if (count > 1)
			{
				extension = "-root";
			}
			break;
		case CERT_TYPE_SUB_CA:
			number = TRUE;
			break;
		case CERT_TYPE_RA:
			if (raout)
			{
				basename = raout;
			}
			else
			{
				extension = "-ra";
			}
			break;
	}

	/* skip if no path is defined */
	if (!basename)
	{
		*path = NULL;
		return TRUE;
	}

	/* check for a file suffix */
	dot = strrchr(basename, '.');
	len = dot ? (dot - basename) : strlen(basename);
	if (dot && (dot[1] != '\0'))
	{
		suffix = dot + 1;
	}

	if (number)
	{
		return asprintf(path, "%.*s%s-%d.%s", len, basename, extension,
						count, suffix) > 0;
	}
	else
	{
		return asprintf(path, "%.*s%s.%s", len, basename, extension, suffix) > 0;
	}
}

/**
 * Write CA/RA certificate to file in DER or PEM format
 */
static bool write_cert(certificate_t *cert, pki_cert_type_t cert_type,
					   bool trusted, char *path, cred_encoding_type_t form,
					   bool force)
{
	chunk_t encoding = chunk_empty;
	bool written;

	if (path)
	{
		if (!cert->get_encoding(cert, form, &encoding))
		{
			DBG1(DBG_APP, "could not get certificate encoding");
			return FALSE;
		}

		written = chunk_write(encoding, path, 0022, force);
		chunk_free(&encoding);

		if (!written)
		{
			DBG1(DBG_APP, "could not write cert file '%s': %s",
				 path, strerror(errno));
			return FALSE;
		}
	}
	else if (form == CERT_PEM)
	{
		if (!cert->get_encoding(cert, form, &encoding))
		{
			DBG1(DBG_APP, "could not get certificate encoding");
			return FALSE;
		}
		printf("%.*s", (int)encoding.len, encoding.ptr);
		chunk_free(&encoding);
		path = "stdout";
	}

#if DEBUG_LEVEL >= 1
	time_t until;
	bool valid = cert->get_validity(cert, NULL, NULL, &until);
	DBG1(DBG_APP, "%s cert is %strusted, %s %T, %s'%s'",
		 cert_type_label[cert_type], trusted ? "" : "un",
		 valid ? "valid until" : "invalid since", &until, FALSE,
		 path ? "written to " : "", path ? path : "not written");
#endif
	return TRUE;
}

/**
 * Extract X.509 CA [and SCEP RA] certificates from PKCS#7 container,
 * check trust as well as validity and write to files
 */
bool pki_cert_extract_cacerts(chunk_t data, char *caout, char *raout,
							  bool is_scep, cred_encoding_type_t form,
							  bool force)
{
	container_t *container;
	mem_cred_t *creds = NULL;
	certificate_t *cert;
	pki_cert_type_t cert_type;
	bool written = FALSE, success = FALSE;
	char *path;

	int cert_type_count[] = { 0, 0, 0 };

	creds = mem_cred_create();
	lib->credmgr->add_set(lib->credmgr, &creds->set);

	container = lib->creds->create(lib->creds, CRED_CONTAINER, CONTAINER_PKCS7,
								   BUILD_BLOB_ASN1_DER, data, BUILD_END);
	if (!container)
	{
		if (is_scep)
		{
			/* no PKCS#7 encoded certificates, assume single root CA cert */
			cert = lib->creds->create(lib->creds, CRED_CERTIFICATE, CERT_X509,
									  BUILD_BLOB, data, BUILD_END);
			if (!cert)
			{
				DBG1(DBG_APP, "could not parse single CA certificate");
				goto end;
			}
			cert_type = get_pki_cert_type(cert);
			cert_type_count[cert_type]++;

			if (print_cert_info(cert, cert_type) &&
				build_pathname(&path, cert_type, cert_type_count, caout, raout,
							   form))
			{
				written = write_cert(cert, cert_type, FALSE, path, form, force);
			}
		}
		else
		{
			DBG1(DBG_APP, "did not receive a valid pkcs7 container");
			goto end;
		}
	}
	else
	{
		enumerator_t *enumerator;
		pkcs7_t *pkcs7 = (pkcs7_t*)container;
		certificate_t *cert_found;
		enumerator_t *certs;
		bool trusted;

		enumerator = pkcs7->create_cert_enumerator(pkcs7);
		while (enumerator->enumerate(enumerator, &cert))
		{
			trusted = FALSE;

			cert_type = get_pki_cert_type(cert);
			if (cert_type == CERT_TYPE_ROOT_CA)
			{
				if (!print_cert_info(cert, cert_type))
				{
					goto end;
				}

				/* same root CA as trusted TLS root CA already in cred set? */
				certs = lib->credmgr->create_trusted_enumerator(lib->credmgr,
									KEY_ANY, cert->get_subject(cert), FALSE);
				while (certs->enumerate(certs, &cert_found, NULL))
				{
					if (cert->equals(cert, cert_found))
					{
						DBG1(DBG_APP, "Root CA equals trusted TLS Root CA");
						trusted = TRUE;
						break;
					}
					else
					{
						DBG1(DBG_APP, "non-matching TLS Root CA of same name");
					}
				}
				certs->destroy(certs);

				/* otherwise trust in root CA has to be established manually */
				if (!trusted)
				{
					creds->add_cert(creds, TRUE, cert->get_ref(cert));
					trusted = FALSE;
				}
				cert_type_count[cert_type]++;

				if (build_pathname(&path, cert_type, cert_type_count, caout,
								   raout, form))
				{
					written = write_cert(cert, cert_type, trusted, path, form,
										 force);
					free(path);
				}
				if (!written)
				{
					break;
				}
			}
			else
			{
				/* trust relative to root CA will be established in round 2 */
				creds->add_cert(creds, FALSE, cert->get_ref(cert));
			}
		}
		enumerator->destroy(enumerator);

		if (!written)
		{
			goto end;
		}

		enumerator = pkcs7->create_cert_enumerator(pkcs7);
		while (enumerator->enumerate(enumerator, &cert))
		{
			written = FALSE;
			trusted = FALSE;

			cert_type = get_pki_cert_type(cert);
			if (cert_type != CERT_TYPE_ROOT_CA)
			{
				if (!print_cert_info(cert, cert_type))
				{
					break;
				}

				/* establish trust relative to root CA */
				certs = lib->credmgr->create_trusted_enumerator(lib->credmgr,
									KEY_ANY, cert->get_subject(cert), FALSE);
				while (certs->enumerate(certs, &cert_found, NULL))
				{
					if (cert->equals(cert, cert_found))
					{
						trusted = TRUE;
						break;
					}
					else
					{
						DBG1(DBG_APP, "non-matching TLS Sub CA of same name");
					}
				}
				certs->destroy(certs);

				cert_type_count[cert_type]++;

				if (build_pathname(&path, cert_type, cert_type_count, caout,
								   raout, form))
				{
					written = write_cert(cert, cert_type, trusted, path, form,
										 force);
					free(path);
				}
				if (!written)
				{
					break;
				}
			}
		}
		enumerator->destroy(enumerator);
	}
	success = TRUE;

end:
	/* cleanup */
	lib->credmgr->remove_set(lib->credmgr, &creds->set);
	creds->destroy(creds);
	DESTROY_IF(container);

	return success;
}

/**
 * Extract an X.509 client certificates from PKCS#7 container
 * check trust as well as validity and write to stdout
 */
bool pki_cert_extract_cert(chunk_t data, cred_encoding_type_t form)
{
	pkcs7_t *pkcs7;
	container_t *container;
	certificate_t *cert;
	mem_cred_t *client_creds;
	chunk_t cert_encoding = chunk_empty;
	enumerator_t *enumerator;
	bool stored = FALSE;

	/* parse pkcs7 signed-data container */
	container = lib->creds->create(lib->creds, CRED_CONTAINER, CONTAINER_PKCS7,
								   BUILD_BLOB_ASN1_DER, data, BUILD_END);
	if (!container)
	{
		DBG1(DBG_APP, "could not parse pkcs7 signed-data container");
		return FALSE;
	}

	lib->credmgr->flush_cache(lib->credmgr, CERT_X509);
	client_creds = mem_cred_create();
	lib->credmgr->add_set(lib->credmgr, &client_creds->set);

	/* store the end entity certificate */
	pkcs7 = (pkcs7_t*)container;
	enumerator = pkcs7->create_cert_enumerator(pkcs7);

	while (enumerator->enumerate(enumerator, &cert))
	{
		x509_t *x509 = (x509_t*)cert;
		certificate_t *cert_found = NULL;
		enumerator_t *certs;
		bool trusted DBG_UNUSED;

		if (!(x509->get_flags(x509) & X509_CA))
		{
			DBG1(DBG_APP, "Issued certificate \"%Y\"", cert->get_subject(cert));
#if DEBUG_LEVEL >= 1
			chunk_t serial = x509->get_serial(x509);
			DBG1(DBG_APP, "  serial: %#B", &serial);
#endif
			if (stored)
			{
				DBG1(DBG_APP, "multiple certs received, only first stored");
				continue;
			}

			/* establish trust relative to root CA */
			client_creds->add_cert(client_creds, FALSE, cert->get_ref(cert));
			certs = lib->credmgr->create_trusted_enumerator(lib->credmgr,
								KEY_ANY, cert->get_subject(cert), FALSE);
			trusted = certs->enumerate(certs, &cert_found, NULL) &&
					  (cert_found == cert);
			certs->destroy(certs);

#if DEBUG_LEVEL >= 1
			time_t from, until;
			bool valid = cert->get_validity(cert, NULL, &from, &until);
			DBG1(DBG_APP, "Issued certificate is %strusted, "
						  "valid from %T until %T (currently %svalid)",
						  trusted ? "" : "not ", &from, FALSE, &until, FALSE,
						  valid ? "" : "not ");
#endif
			if (!cert->get_encoding(cert, form, &cert_encoding))
			{
				DBG1(DBG_APP, "encoding certificate failed");
				break;
			}

			set_file_mode(stdout, form);
			stored = fwrite(cert_encoding.ptr, cert_encoding.len, 1, stdout) == 1;
			chunk_free(&cert_encoding);

			if (!stored)
			{
				DBG1(DBG_APP, "writing certificate failed");
				break;
			}
		}
	}
	enumerator->destroy(enumerator);
	container->destroy(container);
	lib->credmgr->remove_set(lib->credmgr, &client_creds->set);
	client_creds->destroy(client_creds);

	return stored;
}
