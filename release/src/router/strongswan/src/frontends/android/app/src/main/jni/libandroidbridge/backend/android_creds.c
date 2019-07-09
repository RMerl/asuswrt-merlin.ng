/*
 * Copyright (C) 2012-2017 Tobias Brunner
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

#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <time.h>

#include "android_creds.h"
#include "../charonservice.h"

#include <daemon.h>
#include <library.h>
#include <credentials/sets/mem_cred.h>
#include <threading/rwlock.h>

#define CRL_PREFIX "crl-"

typedef struct private_android_creds_t private_android_creds_t;

/**
 * Private data of an android_creds_t object
 */
struct private_android_creds_t {

	/**
	 * Public interface
	 */
	android_creds_t public;

	/**
	 * Credential set storing trusted certificates and user credentials
	 */
	mem_cred_t *creds;

	/**
	 * read/write lock to make sure certificates are only loaded once
	 */
	rwlock_t *lock;

	/**
	 * TRUE if certificates have been loaded via JNI
	 */
	bool loaded;

	/**
	 * Directory for CRLs
	 */
	char *crldir;
};

/**
 * Free allocated DER encoding
 */
static void free_encoding(chunk_t *chunk)
{
	chunk_free(chunk);
	free(chunk);
}

/**
 * Load trusted certificates via charonservice (JNI).
 */
static void load_trusted_certificates(private_android_creds_t *this)
{
	linked_list_t *certs;
	certificate_t *cert;
	chunk_t *current;

	certs = charonservice->get_trusted_certificates(charonservice);
	if (certs)
	{
		while (certs->remove_first(certs, (void**)&current) == SUCCESS)
		{
			cert = lib->creds->create(lib->creds, CRED_CERTIFICATE, CERT_X509,
									  BUILD_BLOB_ASN1_DER, *current, BUILD_END);
			if (cert)
			{
				DBG2(DBG_CFG, "loaded CA certificate '%Y'",
					 cert->get_subject(cert));
				this->creds->add_cert(this->creds, TRUE, cert);
			}
			free_encoding(current);
		}
		certs->destroy(certs);
	}
}

/**
 * Load a CRL from a file
 */
static void load_crl(private_android_creds_t *this, char *file)
{
	certificate_t *cert;
	time_t now, notAfter;

	cert = lib->creds->create(lib->creds, CRED_CERTIFICATE, CERT_X509_CRL,
							  BUILD_FROM_FILE, file, BUILD_END);
	if (cert)
	{
		now = time(NULL);
		if (cert->get_validity(cert, &now, NULL, &notAfter))
		{
			DBG1(DBG_CFG, "loaded crl issued by '%Y'", cert->get_issuer(cert));
			this->creds->add_crl(this->creds, (crl_t*)cert);
		}
		else
		{
			DBG1(DBG_CFG, "deleted crl issued by '%Y', expired (%V ago)",
				 cert->get_issuer(cert), &now, &notAfter);
			unlink(file);
		}
	}
	else
	{
		DBG1(DBG_CFG, "loading crl failed");
		unlink(file);
	}
}

/**
 * Load cached CRLs
 */
static void load_crls(private_android_creds_t *this)
{
	enumerator_t *enumerator;
	struct stat st;
	char *rel, *abs;

	enumerator = enumerator_create_directory(this->crldir);
	if (enumerator)
	{
		while (enumerator->enumerate(enumerator, &rel, &abs, &st))
		{
			if (S_ISREG(st.st_mode) && strpfx(rel, CRL_PREFIX))
			{
				load_crl(this, abs);
			}
		}
		enumerator->destroy(enumerator);
	}
	else
	{
		DBG1(DBG_CFG, "  reading directory '%s' failed", this->crldir);
	}
}

METHOD(credential_set_t, create_cert_enumerator, enumerator_t*,
	private_android_creds_t *this, certificate_type_t cert, key_type_t key,
	identification_t *id, bool trusted)
{
	enumerator_t *enumerator;

	switch (cert)
	{
		case CERT_ANY:
		case CERT_X509:
		case CERT_X509_CRL:
			break;
		default:
			return NULL;
	}
	this->lock->read_lock(this->lock);
	if (!this->loaded)
	{
		this->lock->unlock(this->lock);
		this->lock->write_lock(this->lock);
		/* check again after acquiring the write lock */
		if (!this->loaded)
		{
			load_crls(this);
			load_trusted_certificates(this);
			this->loaded = TRUE;
		}
		this->lock->unlock(this->lock);
		this->lock->read_lock(this->lock);
	}
	enumerator = this->creds->set.create_cert_enumerator(&this->creds->set,
													cert, key, id, trusted);
	return enumerator_create_cleaner(enumerator, (void*)this->lock->unlock,
									 this->lock);
}

METHOD(credential_set_t, cache_cert, void,
	private_android_creds_t *this, certificate_t *cert)
{
	if (this->crldir && cert->get_type(cert) == CERT_X509_CRL)
	{
		/* CRLs get written to /<app>/<path>/crl-<authkeyId>[-delta] */
		crl_t *crl = (crl_t*)cert;

		cert->get_ref(cert);
		if (this->creds->add_crl(this->creds, crl))
		{
			char buf[BUF_LEN];
			chunk_t chunk, hex;
			bool is_delta_crl;

			is_delta_crl = crl->is_delta_crl(crl, NULL);
			chunk = crl->get_authKeyIdentifier(crl);
			hex = chunk_to_hex(chunk, NULL, FALSE);
			snprintf(buf, sizeof(buf), "%s/%s%s%s", this->crldir, CRL_PREFIX,
					 hex.ptr, is_delta_crl ? "-delta" : "");
			free(hex.ptr);

			if (cert->get_encoding(cert, CERT_ASN1_DER, &chunk))
			{
				if (chunk_write(chunk, buf, 022, TRUE))
				{
					DBG1(DBG_CFG, "  written crl to file (%d bytes)",
						 chunk.len);
				}
				else
				{
					DBG1(DBG_CFG, "  writing crl to file failed: %s",
						 strerror(errno));
				}
				free(chunk.ptr);
			}
		}
	}
}

METHOD(android_creds_t, add_username_password, void,
	private_android_creds_t *this, char *username, char *password)
{
	shared_key_t *shared_key;
	identification_t *id;
	chunk_t secret;

	secret = chunk_create(password, strlen(password));
	shared_key = shared_key_create(SHARED_EAP, chunk_clone(secret));
	id = identification_create_from_string(username);

	this->creds->add_shared(this->creds, shared_key, id, NULL);
}

METHOD(credential_set_t, create_shared_enumerator, enumerator_t*,
	private_android_creds_t *this, shared_key_type_t type,
	identification_t *me, identification_t *other)
{
	return this->creds->set.create_shared_enumerator(&this->creds->set,
													 type, me, other);
}

METHOD(android_creds_t, load_user_certificate, certificate_t*,
	private_android_creds_t *this)
{
	linked_list_t *encodings;
	certificate_t *cert = NULL, *ca_cert;
	chunk_t *current;

	encodings = charonservice->get_user_certificate(charonservice);
	if (!encodings)
	{
		DBG1(DBG_CFG, "failed to load user certificate and key");
		return NULL;
	}

	while (encodings->remove_first(encodings, (void**)&current) == SUCCESS)
	{
		if (!cert)
		{	/* the first element is the user certificate */
			cert = lib->creds->create(lib->creds, CRED_CERTIFICATE, CERT_X509,
									  BUILD_BLOB_ASN1_DER, *current, BUILD_END);
			if (!cert)
			{
				DBG1(DBG_CFG, "failed to load user certificate");
				free_encoding(current);
				break;
			}
			DBG1(DBG_CFG, "loaded user certificate '%Y' and private key",
				 cert->get_subject(cert));
			cert = this->creds->add_cert_ref(this->creds, TRUE, cert);
			free_encoding(current);
			continue;
		}
		/* the rest are CA certificates, we ignore failures */
		ca_cert = lib->creds->create(lib->creds, CRED_CERTIFICATE, CERT_X509,
									 BUILD_BLOB_ASN1_DER, *current, BUILD_END);
		if (ca_cert)
		{
			DBG1(DBG_CFG, "loaded CA certificate '%Y'",
				 ca_cert->get_subject(ca_cert));
			this->creds->add_cert(this->creds, TRUE, ca_cert);
		}
		free_encoding(current);
	}
	encodings->destroy_function(encodings, (void*)free_encoding);

	if (cert)
	{
		private_key_t *key;

		key = charonservice->get_user_key(charonservice,
										  cert->get_public_key(cert));
		if (key)
		{
			this->creds->add_key(this->creds, key);
		}
		else
		{
			DBG1(DBG_CFG, "failed to load private key");
			cert->destroy(cert);
			cert = NULL;
		}
	}
	return cert;
}

METHOD(credential_set_t, create_private_enumerator, enumerator_t*,
	private_android_creds_t *this, key_type_t type, identification_t *id)
{
	return this->creds->set.create_private_enumerator(&this->creds->set,
													  type, id);
}

METHOD(android_creds_t, clear, void,
	private_android_creds_t *this)
{
	this->lock->write_lock(this->lock);
	this->creds->clear(this->creds);
	this->loaded = FALSE;
	this->lock->unlock(this->lock);
}

METHOD(android_creds_t, destroy, void,
	private_android_creds_t *this)
{
	clear(this);
	this->creds->destroy(this->creds);
	this->lock->destroy(this->lock);
	free(this->crldir);
	free(this);
}

/**
 * Described in header.
 */
android_creds_t *android_creds_create(char *crldir)
{
	private_android_creds_t *this;

	INIT(this,
		.public = {
			.set = {
				.create_cert_enumerator = _create_cert_enumerator,
				.create_shared_enumerator = _create_shared_enumerator,
				.create_private_enumerator = _create_private_enumerator,
				.create_cdp_enumerator = (void*)return_null,
				.cache_cert = _cache_cert,
			},
			.add_username_password = _add_username_password,
			.load_user_certificate = _load_user_certificate,
			.clear = _clear,
			.destroy = _destroy,
		},
		.creds = mem_cred_create(),
		.lock = rwlock_create(RWLOCK_TYPE_DEFAULT),
		.crldir = strdupnull(crldir),
	);

	return &this->public;
}
