/*
 * Copyright (C) 2008 Martin Willi
 * Copyright (C) 2016 Andreas Steffen
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

#include "cert_cache.h"

#include <time.h>

#include <library.h>
#include <threading/rwlock.h>
#include <collections/linked_list.h>
#include <credentials/certificates/crl.h>

/** cache size, a power of 2 for fast modulo */
#define CACHE_SIZE 32

/** attempts to acquire a cache lock */
#define REPLACE_TRIES 5

typedef struct private_cert_cache_t private_cert_cache_t;
typedef struct relation_t relation_t;

/**
 * A trusted relation between subject and issuer
 */
struct relation_t {

	/**
	 * subject of this relation
	 */
	certificate_t *subject;

	/**
	 * issuer of this relation
	 */
	certificate_t *issuer;

	/**
	 * Signature scheme and parameters used to sign this relation
	 */
	signature_params_t *scheme;

	/**
	 * Cache hits
	 */
	u_int hits;

	/**
	 * Lock for this relation
	 */
	rwlock_t *lock;
};

/**
 * private data of cert_cache
 */
struct private_cert_cache_t {

	/**
	 * public functions
	 */
	cert_cache_t public;

	/**
	 * array of trusted subject-issuer relations
	 */
	relation_t relations[CACHE_SIZE];
};

/**
 * Cache relation in a free slot/replace an other
 */
static void cache(private_cert_cache_t *this,
				  certificate_t *subject, certificate_t *issuer,
				  signature_params_t *scheme)
{
	relation_t *rel;
	int i, offset, try;
	u_int total_hits = 0;

	/* cache a CRL by replacing a previous CRL cache entry if present */
	if (subject->get_type(subject) == CERT_X509_CRL)
	{
		crl_t *crl, *cached_crl;

		/* cache a delta CRL ? */
		crl = (crl_t*)subject;

		for (i = 0; i < CACHE_SIZE; i++)
		{
			rel = &this->relations[i];

			if (rel->subject &&
				rel->subject->get_type(rel->subject) == CERT_X509_CRL &&
				rel->lock->try_write_lock(rel->lock))
			{
				/* double-check having lock */
				if (rel->subject->get_type(rel->subject) == CERT_X509_CRL &&
					rel->issuer->equals(rel->issuer, issuer))
				{
					cached_crl = (crl_t*)rel->subject;

					if (cached_crl->is_delta_crl(cached_crl, NULL) ==
							   crl->is_delta_crl(crl, NULL) &&
						crl_is_newer(crl, cached_crl))
					{
						rel->subject->destroy(rel->subject);
						rel->subject = subject->get_ref(subject);
						signature_params_destroy(rel->scheme);
						rel->scheme = signature_params_clone(scheme);
						return rel->lock->unlock(rel->lock);
					}
				}
				rel->lock->unlock(rel->lock);
			}
		}
	}

	/* check for a unused relation slot first */
	for (i = 0; i < CACHE_SIZE; i++)
	{
		rel = &this->relations[i];

		if (!rel->subject && rel->lock->try_write_lock(rel->lock))
		{
			/* double-check having lock */
			if (!rel->subject)
			{
				rel->subject = subject->get_ref(subject);
				rel->issuer = issuer->get_ref(issuer);
				rel->scheme = signature_params_clone(scheme);
				return rel->lock->unlock(rel->lock);
			}
			rel->lock->unlock(rel->lock);
		}
		total_hits += rel->hits;
	}
	/* run several attempts to replace a random slot, never block. */
	for (try = 0; try < REPLACE_TRIES; try++)
	{
		/* replace a random relation */
		offset = random();
		for (i = 0; i < CACHE_SIZE; i++)
		{
			rel = &this->relations[(i + offset) % CACHE_SIZE];

			if (rel->hits > total_hits / CACHE_SIZE)
			{	/* skip often used slots */
				continue;
			}
			if (rel->lock->try_write_lock(rel->lock))
			{
				if (rel->subject)
				{
					rel->subject->destroy(rel->subject);
					rel->issuer->destroy(rel->issuer);
					signature_params_destroy(rel->scheme);
				}
				rel->subject = subject->get_ref(subject);
				rel->issuer = issuer->get_ref(issuer);
				rel->scheme = signature_params_clone(scheme);
				rel->hits = 0;
				return rel->lock->unlock(rel->lock);
			}
		}
		/* give other threads a chance to release locks */
		sched_yield();
	}
}

METHOD(cert_cache_t, issued_by, bool,
	private_cert_cache_t *this, certificate_t *subject, certificate_t *issuer,
	signature_params_t **schemep)
{
	certificate_t *cached_issuer = NULL;
	relation_t *found = NULL, *current;
	signature_params_t *scheme;
	int i;

	for (i = 0; i < CACHE_SIZE; i++)
	{
		current = &this->relations[i];

		current->lock->read_lock(current->lock);
		if (current->subject)
		{
			if (issuer->equals(issuer, current->issuer))
			{
				if (subject->equals(subject, current->subject))
				{
					current->hits++;
					found = current;
					if (schemep)
					{
						*schemep = signature_params_clone(current->scheme);
					}
				}
				else if (!cached_issuer)
				{
					cached_issuer = current->issuer->get_ref(current->issuer);
				}
			}
		}
		current->lock->unlock(current->lock);
		if (found)
		{
			DESTROY_IF(cached_issuer);
			return TRUE;
		}
	}
	if (subject->issued_by(subject, issuer, &scheme))
	{
		cache(this, subject, cached_issuer ?: issuer, scheme);
		if (schemep)
		{
			*schemep = scheme;
		}
		else
		{
			signature_params_destroy(scheme);
		}
		DESTROY_IF(cached_issuer);
		return TRUE;
	}
	DESTROY_IF(cached_issuer);
	return FALSE;
}

/**
 * certificate enumerator implementation
 */
typedef struct {
	/** implements enumerator_t interface */
	enumerator_t public;
	/** type of requested certificate */
	certificate_type_t cert;
	/** type of requested key */
	key_type_t key;
	/** ID to get a cert for */
	identification_t *id;
	/** cache */
	relation_t *relations;
	/** current position in array cache */
	int index;
	/** currently locked relation */
	int locked;
} cert_enumerator_t;

METHOD(enumerator_t, cert_enumerate, bool,
	cert_enumerator_t *this, va_list args)
{
	public_key_t *public;
	relation_t *rel;
	certificate_t **out;

	VA_ARGS_VGET(args, out);

	if (this->locked >= 0)
	{
		rel = &this->relations[this->locked];
		rel->lock->unlock(rel->lock);
		this->locked = -1;
	}

	while (++this->index < CACHE_SIZE)
	{
		rel = &this->relations[this->index];
		rel->lock->read_lock(rel->lock);
		this->locked = this->index;
		if (rel->subject)
		{
			/* CRL lookup is done using issuer/authkeyidentifier */
			if (this->key == KEY_ANY && this->id &&
				(this->cert == CERT_ANY || this->cert == CERT_X509_CRL) &&
				rel->subject->get_type(rel->subject) == CERT_X509_CRL &&
				rel->subject->has_issuer(rel->subject, this->id))
			{
				*out = rel->subject;
				return TRUE;
			}
			if ((this->cert == CERT_ANY ||
				 rel->subject->get_type(rel->subject) == this->cert) &&
				(!this->id || rel->subject->has_subject(rel->subject, this->id)))
			{
				if (this->key == KEY_ANY)
				{
					*out = rel->subject;
					return TRUE;
				}
				public = rel->subject->get_public_key(rel->subject);
				if (public)
				{
					if (public->get_type(public) == this->key)
					{
						public->destroy(public);
						*out = rel->subject;
						return TRUE;
					}
					public->destroy(public);
				}
			}
		}
		this->locked = -1;
		rel->lock->unlock(rel->lock);
	}
	return FALSE;
}

METHOD(enumerator_t, cert_enumerator_destroy, void,
	cert_enumerator_t *this)
{
	relation_t *rel;

	if (this->locked >= 0)
	{
		rel = &this->relations[this->locked];
		rel->lock->unlock(rel->lock);
	}
	free(this);
}

METHOD(credential_set_t, create_enumerator, enumerator_t*,
	private_cert_cache_t *this, certificate_type_t cert, key_type_t key,
	identification_t *id, bool trusted)
{
	cert_enumerator_t *enumerator;

	if (trusted)
	{
		return NULL;
	}
	INIT(enumerator,
		.public = {
			.enumerate = enumerator_enumerate_default,
			.venumerate = _cert_enumerate,
			.destroy = _cert_enumerator_destroy,
		},
		.cert = cert,
		.key = key,
		.id = id,
		.relations = this->relations,
		.index = -1,
		.locked = -1,
	);
	return &enumerator->public;
}

METHOD(cert_cache_t, flush, void,
	private_cert_cache_t *this, certificate_type_t type)
{
	relation_t *rel;
	int i;

	for (i = 0; i < CACHE_SIZE; i++)
	{
		rel = &this->relations[i];
		if (!rel->subject)
		{
			continue;
		}
		/* check with cheap read lock first */
		if (type != CERT_ANY)
		{
			rel->lock->read_lock(rel->lock);
			if (!rel->subject || type != rel->subject->get_type(rel->subject))
			{
				rel->lock->unlock(rel->lock);
				continue;
			}
			rel->lock->unlock(rel->lock);
		}
		/* double check in write lock */
		rel->lock->write_lock(rel->lock);
		if (rel->subject)
		{
			if (type == CERT_ANY || type == rel->subject->get_type(rel->subject))
			{
				rel->subject->destroy(rel->subject);
				rel->issuer->destroy(rel->issuer);
				signature_params_destroy(rel->scheme);
				rel->subject = NULL;
				rel->issuer = NULL;
				rel->scheme = NULL;
				rel->hits = 0;
			}
		}
		rel->lock->unlock(rel->lock);
	}
}

METHOD(cert_cache_t, destroy, void,
	private_cert_cache_t *this)
{
	relation_t *rel;
	int i;

	for (i = 0; i < CACHE_SIZE; i++)
	{
		rel = &this->relations[i];
		if (rel->subject)
		{
			rel->subject->destroy(rel->subject);
			rel->issuer->destroy(rel->issuer);
			signature_params_destroy(rel->scheme);
		}
		rel->lock->destroy(rel->lock);
	}
	free(this);
}

/*
 * see header file
 */
cert_cache_t *cert_cache_create()
{
	private_cert_cache_t *this;
	int i;

	INIT(this,
		.public = {
			.set = {
				.create_cert_enumerator = _create_enumerator,
				.create_private_enumerator = (void*)return_null,
				.create_shared_enumerator = (void*)return_null,
				.create_cdp_enumerator = (void*)return_null,
				.cache_cert = (void*)nop,
			},
			.issued_by = _issued_by,
			.flush = _flush,
			.destroy = _destroy,
		},
	);

	for (i = 0; i < CACHE_SIZE; i++)
	{
		this->relations[i].subject = NULL;
		this->relations[i].issuer = NULL;
		this->relations[i].scheme = NULL;
		this->relations[i].hits = 0;
		this->relations[i].lock = rwlock_create(RWLOCK_TYPE_DEFAULT);
	}

	return &this->public;
}
