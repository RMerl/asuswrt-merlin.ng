/*
 * Copyright (C) 2008 Martin Willi
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

#include "nm_creds.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <daemon.h>
#include <threading/rwlock.h>
#include <credentials/certificates/x509.h>

typedef struct private_nm_creds_t private_nm_creds_t;

/**
 * private data of nm_creds
 */
struct private_nm_creds_t {

	/**
	 * public functions
	 */
	nm_creds_t public;

	/**
	 * List of trusted certificates, certificate_t*
	 */
	linked_list_t *certs;

	/**
	 * User name
	 */
	identification_t *user;

	/**
	 * User password
	 */
	char *pass;

	/**
	 * Private key decryption password / smartcard pin
	 */
	char *keypass;

	/**
	 * private key ID of smartcard key
	 */
	chunk_t keyid;

	/**
	 * users certificate
	 */
	certificate_t *usercert;

	/**
	 * users private key
	 */
	private_key_t *key;

	/**
	 * read/write lock
	 */
	rwlock_t *lock;
};

/**
 * Enumerator for user certificate
 */
static enumerator_t *create_usercert_enumerator(private_nm_creds_t *this,
							certificate_type_t cert, key_type_t key)
{
	public_key_t *public;

	if (cert != CERT_ANY && cert != this->usercert->get_type(this->usercert))
	{
		return NULL;
	}
	if (key != KEY_ANY)
	{
		public = this->usercert->get_public_key(this->usercert);
		if (!public)
		{
			return NULL;
		}
		if (public->get_type(public) != key)
		{
			public->destroy(public);
			return NULL;
		}
		public->destroy(public);
	}
	this->lock->read_lock(this->lock);
	return enumerator_create_cleaner(
								enumerator_create_single(this->usercert, NULL),
								(void*)this->lock->unlock, this->lock);
}

/**
 * CA certificate enumerator data
 */
typedef struct {
	/** ref to credential credential store */
	private_nm_creds_t *this;
	/** type of key we are looking for */
	key_type_t key;
	/** CA certificate ID */
	identification_t *id;
} cert_data_t;

/**
 * Destroy CA certificate enumerator data
 */
static void cert_data_destroy(cert_data_t *data)
{
	data->this->lock->unlock(data->this->lock);
	free(data);
}

/**
 * Filter function for certificates enumerator
 */
static bool cert_filter(cert_data_t *data, certificate_t **in,
						 certificate_t **out)
{
	certificate_t *cert = *in;
	public_key_t *public;

	public = cert->get_public_key(cert);
	if (!public)
	{
		return FALSE;
	}
	if (data->key != KEY_ANY && public->get_type(public) != data->key)
	{
		public->destroy(public);
		return FALSE;
	}
	if (data->id && data->id->get_type(data->id) == ID_KEY_ID &&
		public->has_fingerprint(public, data->id->get_encoding(data->id)))
	{
		public->destroy(public);
		*out = cert;
		return TRUE;
	}
	public->destroy(public);
	if (data->id && !cert->has_subject(cert, data->id))
	{
		return FALSE;
	}
	*out = cert;
	return TRUE;
}

/**
 * Create enumerator for trusted certificates
 */
static enumerator_t *create_trusted_cert_enumerator(private_nm_creds_t *this,
										key_type_t key, identification_t *id)
{
	cert_data_t *data;

	INIT(data,
		.this = this,
		.id = id,
		.key = key,
	);

	this->lock->read_lock(this->lock);
	return enumerator_create_filter(
					this->certs->create_enumerator(this->certs),
					(void*)cert_filter, data, (void*)cert_data_destroy);
}

METHOD(credential_set_t, create_cert_enumerator, enumerator_t*,
	private_nm_creds_t *this, certificate_type_t cert, key_type_t key,
	identification_t *id, bool trusted)
{
	if (id && this->usercert &&
		id->equals(id, this->usercert->get_subject(this->usercert)))
	{
		return create_usercert_enumerator(this, cert, key);
	}
	if (cert == CERT_X509 || cert == CERT_ANY)
	{
		return create_trusted_cert_enumerator(this, key, id);
	}
	return NULL;
}

METHOD(credential_set_t, create_private_enumerator, enumerator_t*,
	private_nm_creds_t *this, key_type_t type, identification_t *id)
{
	if (this->key == NULL)
	{
		return NULL;
	}
	if (type != KEY_ANY && type != this->key->get_type(this->key))
	{
		return NULL;
	}
	if (id && id->get_type(id) != ID_ANY)
	{
		if (id->get_type(id) != ID_KEY_ID ||
			!this->key->has_fingerprint(this->key, id->get_encoding(id)))
		{
			return NULL;
		}
	}
	this->lock->read_lock(this->lock);
	return enumerator_create_cleaner(enumerator_create_single(this->key, NULL),
									 (void*)this->lock->unlock, this->lock);
}

/**
 * shared key enumerator implementation
 */
typedef struct {
	enumerator_t public;
	private_nm_creds_t *this;
	shared_key_t *key;
	bool done;
} shared_enumerator_t;

METHOD(enumerator_t, shared_enumerate, bool,
	shared_enumerator_t *this, shared_key_t **key, id_match_t *me,
	id_match_t *other)
{
	if (this->done)
	{
		return FALSE;
	}
	*key = this->key;
	if (me)
	{
		*me = ID_MATCH_PERFECT;
	}
	if (other)
	{
		*other = ID_MATCH_ANY;
	}
	this->done = TRUE;
	return TRUE;
}

METHOD(enumerator_t, shared_destroy, void,
	shared_enumerator_t *this)
{
	this->key->destroy(this->key);
	this->this->lock->unlock(this->this->lock);
	free(this);
}

METHOD(credential_set_t, create_shared_enumerator, enumerator_t*,
	private_nm_creds_t *this, shared_key_type_t type, identification_t *me,
	identification_t *other)
{
	shared_enumerator_t *enumerator;
	chunk_t key;

	this->lock->read_lock(this->lock);

	switch (type)
	{
		case SHARED_EAP:
		case SHARED_IKE:
			if (!this->pass || !this->user)
			{
				goto no_secret;
			}
			if (me && !me->equals(me, this->user))
			{
				goto no_secret;
			}
			key = chunk_create(this->pass, strlen(this->pass));
			break;
		case SHARED_PRIVATE_KEY_PASS:
			if (!this->keypass)
			{
				goto no_secret;
			}
			key = chunk_create(this->keypass, strlen(this->keypass));
			break;
		case SHARED_PIN:
			if (!this->keypass || !me ||
				!chunk_equals(me->get_encoding(me), this->keyid))
			{
				goto no_secret;
			}
			key = chunk_create(this->keypass, strlen(this->keypass));
			break;
		default:
			goto no_secret;
	}

	INIT(enumerator,
		.public = {
			.enumerate = (void*)_shared_enumerate,
			.destroy = _shared_destroy,
		},
		.this = this,
	);
	enumerator->key = shared_key_create(type, chunk_clone(key));
	return &enumerator->public;

no_secret:
	this->lock->unlock(this->lock);
	return NULL;
}

METHOD(nm_creds_t, add_certificate, void,
	private_nm_creds_t *this, certificate_t *cert)
{
	this->lock->write_lock(this->lock);
	this->certs->insert_last(this->certs, cert);
	this->lock->unlock(this->lock);
}

/**
 * Load a certificate file
 */
static void load_ca_file(private_nm_creds_t *this, char *file)
{
	certificate_t *cert;

	/* We add the CA constraint, as many CAs miss it */
	cert = lib->creds->create(lib->creds, CRED_CERTIFICATE, CERT_X509,
							  BUILD_FROM_FILE, file, BUILD_END);
	if (!cert)
	{
		DBG1(DBG_CFG, "loading CA certificate '%s' failed", file);
	}
	else
	{
		DBG2(DBG_CFG, "loaded CA certificate '%Y'", cert->get_subject(cert));
		x509_t *x509 = (x509_t*)cert;
		if (!(x509->get_flags(x509) & X509_SELF_SIGNED))
		{
			DBG1(DBG_CFG, "%Y is not self signed", cert->get_subject(cert));
		}
		this->certs->insert_last(this->certs, cert);
	}
}

METHOD(nm_creds_t, load_ca_dir, void,
	private_nm_creds_t *this, char *dir)
{
	enumerator_t *enumerator;
	char *rel, *abs;
	struct stat st;

	enumerator = enumerator_create_directory(dir);
	if (enumerator)
	{
		while (enumerator->enumerate(enumerator, &rel, &abs, &st))
		{
			/* skip '.', '..' and hidden files */
			if (rel[0] != '.')
			{
				if (S_ISDIR(st.st_mode))
				{
					load_ca_dir(this, abs);
				}
				else if (S_ISREG(st.st_mode))
				{
					load_ca_file(this, abs);
				}
			}
		}
		enumerator->destroy(enumerator);
	}
}

METHOD(nm_creds_t, set_username_password, void,
	private_nm_creds_t *this, identification_t *id, char *password)
{
	this->lock->write_lock(this->lock);
	DESTROY_IF(this->user);
	this->user = id->clone(id);
	free(this->pass);
	this->pass = strdupnull(password);
	this->lock->unlock(this->lock);
}

METHOD(nm_creds_t, set_key_password, void,
	private_nm_creds_t *this, char *password)
{
	this->lock->write_lock(this->lock);
	free(this->keypass);
	this->keypass = strdupnull(password);
	this->lock->unlock(this->lock);
}

METHOD(nm_creds_t, set_pin, void,
	private_nm_creds_t *this, chunk_t keyid, char *pin)
{
	this->lock->write_lock(this->lock);
	free(this->keypass);
	free(this->keyid.ptr);
	this->keypass = strdupnull(pin);
	this->keyid = chunk_clone(keyid);
	this->lock->unlock(this->lock);
}

METHOD(nm_creds_t, set_cert_and_key, void,
	private_nm_creds_t *this, certificate_t *cert, private_key_t *key)
{
	this->lock->write_lock(this->lock);
	DESTROY_IF(this->key);
	DESTROY_IF(this->usercert);
	this->key = key;
	this->usercert = cert;
	this->lock->unlock(this->lock);
}

METHOD(nm_creds_t, clear, void,
	private_nm_creds_t *this)
{
	certificate_t *cert;

	while (this->certs->remove_last(this->certs, (void**)&cert) == SUCCESS)
	{
		cert->destroy(cert);
	}
	DESTROY_IF(this->user);
	free(this->pass);
	free(this->keypass);
	free(this->keyid.ptr);
	DESTROY_IF(this->usercert);
	DESTROY_IF(this->key);
	this->key = NULL;
	this->usercert = NULL;
	this->pass = NULL;
	this->user = NULL;
	this->keypass = NULL;
	this->keyid = chunk_empty;
}

METHOD(nm_creds_t, destroy, void,
	private_nm_creds_t *this)
{
	clear(this);
	this->certs->destroy(this->certs);
	this->lock->destroy(this->lock);
	free(this);
}

/*
 * see header file
 */
nm_creds_t *nm_creds_create()
{
	private_nm_creds_t *this;

	INIT(this,
		.public = {
			.set = {
				.create_private_enumerator = _create_private_enumerator,
				.create_cert_enumerator = _create_cert_enumerator,
				.create_shared_enumerator = _create_shared_enumerator,
				.create_cdp_enumerator = (void*)return_null,
				.cache_cert = (void*)nop,
			},
			.add_certificate = _add_certificate,
			.load_ca_dir = _load_ca_dir,
			.set_username_password = _set_username_password,
			.set_key_password = _set_key_password,
			.set_pin = _set_pin,
			.set_cert_and_key = _set_cert_and_key,
			.clear = _clear,
			.destroy = _destroy,
		},
		.lock = rwlock_create(RWLOCK_TYPE_DEFAULT),
		.certs = linked_list_create(),
	);
	return &this->public;
}

