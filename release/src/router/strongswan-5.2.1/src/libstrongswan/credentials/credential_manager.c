/*
 * Copyright (C) 2007 Martin Willi
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

#include "credential_manager.h"

#include <library.h>
#include <utils/debug.h>
#include <threading/thread_value.h>
#include <threading/mutex.h>
#include <threading/rwlock.h>
#include <collections/linked_list.h>
#include <credentials/sets/cert_cache.h>
#include <credentials/sets/auth_cfg_wrapper.h>
#include <credentials/certificates/x509.h>

/**
 * Maximum length of a certificate trust chain
 */
#define MAX_TRUST_PATH_LEN 7

typedef struct private_credential_manager_t private_credential_manager_t;

/**
 * private data of credential_manager
 */
struct private_credential_manager_t {

	/**
	 * public functions
	 */
	credential_manager_t public;

	/**
	 * list of credential sets
	 */
	linked_list_t *sets;

	/**
	 * thread local set of credentials, linked_list_t with credential_set_t's
	 */
	thread_value_t *local_sets;

	/**
	 * Exclusive local sets, linked_list_t with credential_set_t
	 */
	thread_value_t *exclusive_local_sets;

	/**
	 * trust relationship and certificate cache
	 */
	cert_cache_t *cache;

	/**
	 * certificates queued for persistent caching
	 */
	linked_list_t *cache_queue;

	/**
	 * list of certificate validators, cert_validator_t
	 */
	linked_list_t *validators;

	/**
	 * read-write lock to sets list
	 */
	rwlock_t *lock;

	/**
	 * mutex for cache queue
	 */
	mutex_t *queue_mutex;

	/**
	 * Registered hook to call on validation errors
	 */
	credential_hook_t hook;

	/**
	 * Registered data to pass to hook
	 */
	void *hook_data;
};

/** data to pass to create_private_enumerator */
typedef struct {
	private_credential_manager_t *this;
	key_type_t type;
	identification_t* keyid;
} private_data_t;

/** data to pass to create_cert_enumerator */
typedef struct {
	private_credential_manager_t *this;
	certificate_type_t cert;
	key_type_t key;
	identification_t *id;
	bool trusted;
} cert_data_t;

/** data to pass to create_cdp_enumerator */
typedef struct {
	private_credential_manager_t *this;
	certificate_type_t type;
	identification_t *id;
} cdp_data_t;

/** data to pass to create_shared_enumerator */
typedef struct {
	private_credential_manager_t *this;
	shared_key_type_t type;
	identification_t *me;
	identification_t *other;
} shared_data_t;

/** enumerator over local and global sets */
typedef struct {
	/** implements enumerator_t */
	enumerator_t public;
	/** enumerator over global sets */
	enumerator_t *global;
	/** enumerator over local sets */
	enumerator_t *local;
	/** enumerator over exclusive local sets */
	enumerator_t *exclusive;
} sets_enumerator_t;

METHOD(credential_manager_t, set_hook, void,
	private_credential_manager_t *this, credential_hook_t hook, void *data)
{
	this->hook = hook;
	this->hook_data = data;
}

METHOD(credential_manager_t, call_hook, void,
	private_credential_manager_t *this, credential_hook_type_t type,
	certificate_t *cert)
{
	if (this->hook)
	{
		this->hook(this->hook_data, type, cert);
	}
}

METHOD(enumerator_t, sets_enumerate, bool,
	sets_enumerator_t *this, credential_set_t **set)
{
	if (this->exclusive)
	{
		if (this->exclusive->enumerate(this->exclusive, set))
		{	/* only enumerate last added */
			this->exclusive->destroy(this->exclusive);
			this->exclusive = NULL;
			return TRUE;
		}
	}
	if (this->global)
	{
		if (this->global->enumerate(this->global, set))
		{
			return TRUE;
		}
		/* end of global sets, look for local */
		this->global->destroy(this->global);
		this->global = NULL;
	}
	if (this->local)
	{
		return this->local->enumerate(this->local, set);
	}
	return FALSE;
}

METHOD(enumerator_t, sets_destroy, void,
	sets_enumerator_t *this)
{
	DESTROY_IF(this->global);
	DESTROY_IF(this->local);
	DESTROY_IF(this->exclusive);
	free(this);
}

/**
 * create an enumerator over both, global and local sets
 */
static enumerator_t *create_sets_enumerator(private_credential_manager_t *this)
{
	sets_enumerator_t *enumerator;
	linked_list_t *list;

	INIT(enumerator,
		.public = {
			.enumerate = (void*)_sets_enumerate,
			.destroy = _sets_destroy,
		},
	);

	list = this->exclusive_local_sets->get(this->exclusive_local_sets);
	if (list && list->get_count(list))
	{
		enumerator->exclusive = list->create_enumerator(list);
	}
	else
	{
		enumerator->global = this->sets->create_enumerator(this->sets);
		list = this->local_sets->get(this->local_sets);
		if (list)
		{
			enumerator->local = list->create_enumerator(list);
		}
	}
	return &enumerator->public;
}

/**
 * cleanup function for cert data
 */
static void destroy_cert_data(cert_data_t *data)
{
	data->this->lock->unlock(data->this->lock);
	free(data);
}

/**
 * enumerator constructor for certificates
 */
static enumerator_t *create_cert(credential_set_t *set, cert_data_t *data)
{
	return set->create_cert_enumerator(set, data->cert, data->key,
									   data->id, data->trusted);
}

METHOD(credential_manager_t, create_cert_enumerator, enumerator_t*,
	private_credential_manager_t *this, certificate_type_t certificate,
	key_type_t key, identification_t *id, bool trusted)
{
	cert_data_t *data = malloc_thing(cert_data_t);
	data->this = this;
	data->cert = certificate;
	data->key = key;
	data->id = id;
	data->trusted = trusted;

	this->lock->read_lock(this->lock);
	return enumerator_create_nested(create_sets_enumerator(this),
									(void*)create_cert, data,
									(void*)destroy_cert_data);
}

METHOD(credential_manager_t, get_cert, certificate_t*,
	private_credential_manager_t *this, certificate_type_t cert, key_type_t key,
	identification_t *id, bool trusted)
{
	certificate_t *current, *found = NULL;
	enumerator_t *enumerator;

	enumerator = create_cert_enumerator(this, cert, key, id, trusted);
	if (enumerator->enumerate(enumerator, &current))
	{
		/* TODO: best match? order by keyid, subject, sualtname */
		found = current->get_ref(current);
	}
	enumerator->destroy(enumerator);
	return found;
}


/**
 * cleanup function for cdp data
 */
static void destroy_cdp_data(cdp_data_t *data)
{
	data->this->lock->unlock(data->this->lock);
	free(data);
}

/**
 * enumerator constructor for CDPs
 */
static enumerator_t *create_cdp(credential_set_t *set, cdp_data_t *data)
{
	return set->create_cdp_enumerator(set, data->type, data->id);
}

METHOD(credential_manager_t, create_cdp_enumerator, enumerator_t*,
	private_credential_manager_t *this, certificate_type_t type,
	identification_t *id)
{
	cdp_data_t *data;

	INIT(data,
		.this = this,
		.type = type,
		.id = id,
	);
	this->lock->read_lock(this->lock);
	return enumerator_create_nested(create_sets_enumerator(this),
									(void*)create_cdp, data,
									(void*)destroy_cdp_data);
}

/**
 * cleanup function for private data
 */
static void destroy_private_data(private_data_t *data)
{
	data->this->lock->unlock(data->this->lock);
	free(data);
}

/**
 * enumerator constructor for private keys
 */
static enumerator_t *create_private(credential_set_t *set, private_data_t *data)
{
	return set->create_private_enumerator(set, data->type, data->keyid);
}

/**
 * Create an enumerator over private keys
 */
static enumerator_t *create_private_enumerator(
	private_credential_manager_t *this, key_type_t key, identification_t *keyid)
{
	private_data_t *data;

	INIT(data,
		.this = this,
		.type = key,
		.keyid = keyid,
	);
	this->lock->read_lock(this->lock);
	return enumerator_create_nested(create_sets_enumerator(this),
									(void*)create_private, data,
									(void*)destroy_private_data);
}

/**
 * Look up a private key by its key identifier
 */
static private_key_t* get_private_by_keyid(private_credential_manager_t *this,
									key_type_t key, identification_t *keyid)
{
	private_key_t *found = NULL;
	enumerator_t *enumerator;

	enumerator = create_private_enumerator(this, key, keyid);
	if (enumerator->enumerate(enumerator, &found))
	{
		found->get_ref(found);
	}
	enumerator->destroy(enumerator);
	return found;
}

/**
 * cleanup function for shared data
 */
static void destroy_shared_data(shared_data_t *data)
{
	data->this->lock->unlock(data->this->lock);
	free(data);
}

/**
 * enumerator constructor for shared keys
 */
static enumerator_t *create_shared(credential_set_t *set, shared_data_t *data)
{
	return set->create_shared_enumerator(set, data->type, data->me, data->other);
}

METHOD(credential_manager_t, create_shared_enumerator, enumerator_t*,
	private_credential_manager_t *this, shared_key_type_t type,
	identification_t *me, identification_t *other)
{
	shared_data_t *data;

	INIT(data,
		.this = this,
		.type = type,
		.me = me,
		.other = other,
	);
	this->lock->read_lock(this->lock);
	return enumerator_create_nested(create_sets_enumerator(this),
									(void*)create_shared, data,
									(void*)destroy_shared_data);
}

METHOD(credential_manager_t, get_shared, shared_key_t*,
	private_credential_manager_t *this, shared_key_type_t type,
	identification_t *me, identification_t *other)
{
	shared_key_t *current, *found = NULL;
	id_match_t best_me = ID_MATCH_NONE, best_other = ID_MATCH_NONE;
	id_match_t match_me, match_other;
	enumerator_t *enumerator;

	enumerator = create_shared_enumerator(this, type, me, other);
	while (enumerator->enumerate(enumerator, &current, &match_me, &match_other))
	{
		if (match_other > best_other ||
			(match_other == best_other && match_me > best_me))
		{
			DESTROY_IF(found);
			found = current->get_ref(current);
			best_me = match_me;
			best_other = match_other;
		}
		if (best_me == ID_MATCH_PERFECT && best_other == ID_MATCH_PERFECT)
		{
			break;
		}
	}
	enumerator->destroy(enumerator);
	return found;
}

METHOD(credential_manager_t, add_local_set, void,
	private_credential_manager_t *this, credential_set_t *set, bool exclusive)
{
	linked_list_t *sets;
	thread_value_t *tv;

	if (exclusive)
	{
		tv = this->exclusive_local_sets;
	}
	else
	{
		tv = this->local_sets;
	}
	sets = tv->get(tv);
	if (!sets)
	{
		sets = linked_list_create();
		tv->set(tv, sets);
	}
	if (exclusive)
	{
		sets->insert_first(sets, set);
	}
	else
	{
		sets->insert_last(sets, set);
	}
}

METHOD(credential_manager_t, remove_local_set, void,
	private_credential_manager_t *this, credential_set_t *set)
{
	linked_list_t *sets;
	thread_value_t *tv;

	tv = this->local_sets;
	sets = tv->get(tv);
	if (sets && sets->remove(sets, set, NULL) && sets->get_count(sets) == 0)
	{
		tv->set(tv, NULL);
		sets->destroy(sets);
	}
	tv = this->exclusive_local_sets;
	sets = tv->get(tv);
	if (sets && sets->remove(sets, set, NULL) && sets->get_count(sets) == 0)
	{
		tv->set(tv, NULL);
		sets->destroy(sets);
	}
}

METHOD(credential_manager_t, issued_by, bool,
	private_credential_manager_t *this, certificate_t *subject,
	certificate_t *issuer, signature_scheme_t *scheme)
{
	if (this->cache)
	{
		return this->cache->issued_by(this->cache, subject, issuer, scheme);
	}
	return subject->issued_by(subject, issuer, scheme);
}

METHOD(credential_manager_t, cache_cert, void,
	private_credential_manager_t *this, certificate_t *cert)
{
	credential_set_t *set;
	enumerator_t *enumerator;

	if (this->lock->try_write_lock(this->lock))
	{
		enumerator = this->sets->create_enumerator(this->sets);
		while (enumerator->enumerate(enumerator, &set))
		{
			set->cache_cert(set, cert);
		}
		enumerator->destroy(enumerator);
		this->lock->unlock(this->lock);
	}
	else
	{	/* we can't cache now as other threads are active, queue for later */
		this->queue_mutex->lock(this->queue_mutex);
		this->cache_queue->insert_last(this->cache_queue, cert->get_ref(cert));
		this->queue_mutex->unlock(this->queue_mutex);
	}
}

/**
 * Try to cache certificates queued for caching
 */
static void cache_queue(private_credential_manager_t *this)
{
	credential_set_t *set;
	certificate_t *cert;
	enumerator_t *enumerator;

	this->queue_mutex->lock(this->queue_mutex);
	if (this->cache_queue->get_count(this->cache_queue) > 0 &&
		this->lock->try_write_lock(this->lock))
	{
		while (this->cache_queue->remove_last(this->cache_queue,
											  (void**)&cert) == SUCCESS)
		{
			enumerator = this->sets->create_enumerator(this->sets);
			while (enumerator->enumerate(enumerator, &set))
			{
				set->cache_cert(set, cert);
			}
			enumerator->destroy(enumerator);
			cert->destroy(cert);
		}
		this->lock->unlock(this->lock);
	}
	this->queue_mutex->unlock(this->queue_mutex);
}

/**
 * Use validators to check the lifetime of certificates
 */
static bool check_lifetime(private_credential_manager_t *this,
						   certificate_t *cert, char *label,
						   int pathlen, bool trusted, auth_cfg_t *auth)
{
	time_t not_before, not_after;
	cert_validator_t *validator;
	enumerator_t *enumerator;
	status_t status = NEED_MORE;

	enumerator = this->validators->create_enumerator(this->validators);
	while (enumerator->enumerate(enumerator, &validator))
	{
		if (!validator->check_lifetime)
		{
			continue;
		}
		status = validator->check_lifetime(validator, cert,
										   pathlen, trusted, auth);
		if (status != NEED_MORE)
		{
			break;
		}
	}
	enumerator->destroy(enumerator);

	switch (status)
	{
		case NEED_MORE:
			if (!cert->get_validity(cert, NULL, &not_before, &not_after))
			{
				DBG1(DBG_CFG, "%s certificate invalid (valid from %T to %T)",
					 label, &not_before, FALSE, &not_after, FALSE);
				break;
			}
			return TRUE;
		case SUCCESS:
			return TRUE;
		case FAILED:
		default:
			break;
	}
	call_hook(this, CRED_HOOK_EXPIRED, cert);
	return FALSE;
}

/**
 * check a certificate for its lifetime
 */
static bool check_certificate(private_credential_manager_t *this,
				certificate_t *subject, certificate_t *issuer, bool online,
				int pathlen, bool trusted, auth_cfg_t *auth)
{
	cert_validator_t *validator;
	enumerator_t *enumerator;

	if (!check_lifetime(this, subject, "subject", pathlen, FALSE, auth) ||
		!check_lifetime(this, issuer, "issuer", pathlen + 1, trusted, auth))
	{
		return FALSE;
	}

	enumerator = this->validators->create_enumerator(this->validators);
	while (enumerator->enumerate(enumerator, &validator))
	{
		if (!validator->validate)
		{
			continue;
		}
		if (!validator->validate(validator, subject, issuer,
								 online, pathlen, trusted, auth))
		{
			enumerator->destroy(enumerator);
			return FALSE;
		}
	}
	enumerator->destroy(enumerator);
	return TRUE;
}

/**
 * Get a trusted certificate from a credential set
 */
static certificate_t *get_pretrusted_cert(private_credential_manager_t *this,
										  key_type_t type, identification_t *id)
{
	certificate_t *subject;
	public_key_t *public;

	subject = get_cert(this, CERT_ANY, type, id, TRUE);
	if (!subject)
	{
		return NULL;
	}
	public = subject->get_public_key(subject);
	if (!public)
	{
		subject->destroy(subject);
		return NULL;
	}
	public->destroy(public);
	return subject;
}

/**
 * Get the issuing certificate of a subject certificate
 */
static certificate_t *get_issuer_cert(private_credential_manager_t *this,
									  certificate_t *subject, bool trusted,
									  signature_scheme_t *scheme)
{
	enumerator_t *enumerator;
	certificate_t *issuer = NULL, *candidate;

	enumerator = create_cert_enumerator(this, subject->get_type(subject), KEY_ANY,
										subject->get_issuer(subject), trusted);
	while (enumerator->enumerate(enumerator, &candidate))
	{
		if (issued_by(this, subject, candidate, scheme))
		{
			issuer = candidate->get_ref(candidate);
			break;
		}
	}
	enumerator->destroy(enumerator);
	return issuer;
}

/**
 * Get the strength of certificate, add it to auth
 */
static void get_key_strength(certificate_t *cert, auth_cfg_t *auth)
{
	uintptr_t strength;
	public_key_t *key;
	key_type_t type;

	key = cert->get_public_key(cert);
	if (key)
	{
		type = key->get_type(key);
		strength = key->get_keysize(key);
		DBG2(DBG_CFG, "  certificate \"%Y\" key: %d bit %N",
			 cert->get_subject(cert), strength, key_type_names, type);
		switch (type)
		{
			case KEY_RSA:
				auth->add(auth, AUTH_RULE_RSA_STRENGTH, strength);
				break;
			case KEY_ECDSA:
				auth->add(auth, AUTH_RULE_ECDSA_STRENGTH, strength);
				break;
			default:
				break;
		}
		key->destroy(key);
	}
}

/**
 * try to verify the trust chain of subject, return TRUE if trusted
 */
static bool verify_trust_chain(private_credential_manager_t *this,
							   certificate_t *subject, auth_cfg_t *result,
							   bool trusted, bool online)
{
	certificate_t *current, *issuer;
	auth_cfg_t *auth;
	signature_scheme_t scheme;
	int pathlen;

	auth = auth_cfg_create();
	get_key_strength(subject, auth);
	current = subject->get_ref(subject);
	auth->add(auth, AUTH_RULE_SUBJECT_CERT, current->get_ref(current));

	for (pathlen = 0; pathlen <= MAX_TRUST_PATH_LEN; pathlen++)
	{
		issuer = get_issuer_cert(this, current, TRUE, &scheme);
		if (issuer)
		{
			/* accept only self-signed CAs as trust anchor */
			if (issued_by(this, issuer, issuer, NULL))
			{
				auth->add(auth, AUTH_RULE_CA_CERT, issuer->get_ref(issuer));
				DBG1(DBG_CFG, "  using trusted ca certificate \"%Y\"",
							  issuer->get_subject(issuer));
				trusted = TRUE;
			}
			else
			{
				auth->add(auth, AUTH_RULE_IM_CERT, issuer->get_ref(issuer));
				DBG1(DBG_CFG, "  using trusted intermediate ca certificate "
					 "\"%Y\"", issuer->get_subject(issuer));
			}
			auth->add(auth, AUTH_RULE_SIGNATURE_SCHEME, scheme);
		}
		else
		{
			issuer = get_issuer_cert(this, current, FALSE, &scheme);
			if (issuer)
			{
				if (current->equals(current, issuer))
				{
					DBG1(DBG_CFG, "  self-signed certificate \"%Y\" is not "
						 "trusted", current->get_subject(current));
					issuer->destroy(issuer);
					call_hook(this, CRED_HOOK_UNTRUSTED_ROOT, current);
					break;
				}
				auth->add(auth, AUTH_RULE_IM_CERT, issuer->get_ref(issuer));
				DBG1(DBG_CFG, "  using untrusted intermediate certificate "
					 "\"%Y\"", issuer->get_subject(issuer));
				auth->add(auth, AUTH_RULE_SIGNATURE_SCHEME, scheme);
			}
			else
			{
				DBG1(DBG_CFG, "no issuer certificate found for \"%Y\"",
					 current->get_subject(current));
				call_hook(this, CRED_HOOK_NO_ISSUER, current);
				break;
			}
		}
		if (!check_certificate(this, current, issuer, online,
							   pathlen, trusted, auth))
		{
			trusted = FALSE;
			issuer->destroy(issuer);
			break;
		}
		if (issuer)
		{
			get_key_strength(issuer, auth);
		}
		current->destroy(current);
		current = issuer;
		if (trusted)
		{
			DBG1(DBG_CFG, "  reached self-signed root ca with a "
				 "path length of %d", pathlen);
			break;
		}
	}
	current->destroy(current);
	if (pathlen > MAX_TRUST_PATH_LEN)
	{
		DBG1(DBG_CFG, "maximum path length of %d exceeded", MAX_TRUST_PATH_LEN);
		call_hook(this, CRED_HOOK_EXCEEDED_PATH_LEN, subject);
	}
	if (trusted)
	{
		result->merge(result, auth, FALSE);
	}
	auth->destroy(auth);
	return trusted;
}

/**
 * List find match function for certificates
 */
static bool cert_equals(certificate_t *a, certificate_t *b)
{
	return a->equals(a, b);
}

/**
 * enumerator for trusted certificates
 */
typedef struct {
	/** implements enumerator_t interface */
	enumerator_t public;
	/** enumerator over candidate peer certificates */
	enumerator_t *candidates;
	/** reference to the credential_manager */
	private_credential_manager_t *this;
	/** type of the requested key */
	key_type_t type;
	/** identity the requested key belongs to */
	identification_t *id;
	/** TRUE to do CRL/OCSP checking */
	bool online;
	/** pretrusted certificate we have served at first invocation */
	certificate_t *pretrusted;
	/** currently enumerating auth config */
	auth_cfg_t *auth;
	/** list of failed candidates */
	linked_list_t *failed;
} trusted_enumerator_t;

METHOD(enumerator_t, trusted_enumerate, bool,
	trusted_enumerator_t *this, certificate_t **cert, auth_cfg_t **auth)
{
	certificate_t *current;

	DESTROY_IF(this->auth);
	this->auth = auth_cfg_create();

	if (!this->candidates)
	{
		/* first invocation, build enumerator for next one */
		this->candidates = create_cert_enumerator(this->this, CERT_ANY,
												  this->type, this->id, FALSE);
		/* check if we have a trusted certificate for that peer */
		this->pretrusted = get_pretrusted_cert(this->this, this->type, this->id);
		if (this->pretrusted)
		{
			/* if we find a trusted self signed certificate, we just accept it.
			 * However, in order to fulfill authorization rules, we try to build
			 * the trust chain if it is not self signed */
			if (issued_by(this->this, this->pretrusted, this->pretrusted, NULL) ||
				verify_trust_chain(this->this, this->pretrusted, this->auth,
								   TRUE, this->online))
			{
				DBG1(DBG_CFG, "  using trusted certificate \"%Y\"",
					 this->pretrusted->get_subject(this->pretrusted));
				*cert = this->pretrusted;
				if (!this->auth->get(this->auth, AUTH_RULE_SUBJECT_CERT))
				{	/* add cert to auth info, if not returned by trustchain */
					this->auth->add(this->auth, AUTH_RULE_SUBJECT_CERT,
									this->pretrusted->get_ref(this->pretrusted));
				}
				if (auth)
				{
					*auth = this->auth;
				}
				return TRUE;
			}
		}
	}
	/* try to verify the trust chain for each certificate found */
	while (this->candidates->enumerate(this->candidates, &current))
	{
		if (this->pretrusted &&
			this->pretrusted->equals(this->pretrusted, current))
		{	/* skip pretrusted certificate we already served */
			continue;
		}

		if (this->failed->find_first(this->failed, (void*)cert_equals,
									 NULL, current) == SUCCESS)
		{	/* check each candidate only once */
			continue;
		}

		DBG1(DBG_CFG, "  using certificate \"%Y\"",
			 current->get_subject(current));
		if (verify_trust_chain(this->this, current, this->auth, FALSE,
							   this->online))
		{
			*cert = current;
			if (auth)
			{
				*auth = this->auth;
			}
			return TRUE;
		}
		this->failed->insert_last(this->failed, current->get_ref(current));
	}
	return FALSE;
}

METHOD(enumerator_t, trusted_destroy, void,
	trusted_enumerator_t *this)
{
	DESTROY_IF(this->pretrusted);
	DESTROY_IF(this->auth);
	DESTROY_IF(this->candidates);
	this->failed->destroy_offset(this->failed, offsetof(certificate_t, destroy));
	free(this);
}

METHOD(credential_manager_t, create_trusted_enumerator, enumerator_t*,
	private_credential_manager_t *this, key_type_t type,
	identification_t *id, bool online)
{
	trusted_enumerator_t *enumerator;

	INIT(enumerator,
		.public = {
			.enumerate = (void*)_trusted_enumerate,
			.destroy = _trusted_destroy,
		},
		.this = this,
		.type = type,
		.id = id,
		.online = online,
		.failed = linked_list_create(),
	);
	return &enumerator->public;
}

/**
 * enumerator for public keys
 */
typedef struct {
	/** implements enumerator_t interface */
	enumerator_t public;
	/** enumerator over candidate peer certificates */
	enumerator_t *inner;
	/** reference to the credential_manager */
	private_credential_manager_t *this;
	/** currently enumerating key */
	public_key_t *current;
	/** credset wrapper around auth config */
	auth_cfg_wrapper_t *wrapper;
} public_enumerator_t;

METHOD(enumerator_t, public_enumerate, bool,
	public_enumerator_t *this, public_key_t **key, auth_cfg_t **auth)
{
	certificate_t *cert;

	while (this->inner->enumerate(this->inner, &cert, auth))
	{
		DESTROY_IF(this->current);
		this->current = cert->get_public_key(cert);
		if (this->current)
		{
			*key = this->current;
			return TRUE;
		}
	}
	return FALSE;
}

METHOD(enumerator_t, public_destroy, void,
	public_enumerator_t *this)
{
	DESTROY_IF(this->current);
	this->inner->destroy(this->inner);
	if (this->wrapper)
	{
		remove_local_set(this->this, &this->wrapper->set);
		this->wrapper->destroy(this->wrapper);
	}
	this->this->lock->unlock(this->this->lock);

	/* check for delayed certificate cache queue */
	cache_queue(this->this);
	free(this);
}

METHOD(credential_manager_t, create_public_enumerator, enumerator_t*,
	private_credential_manager_t *this, key_type_t type, identification_t *id,
	auth_cfg_t *auth)
{
	public_enumerator_t *enumerator;

	INIT(enumerator,
		.public = {
			.enumerate = (void*)_public_enumerate,
			.destroy = _public_destroy,
		},
		.inner = create_trusted_enumerator(this, type, id, TRUE),
		.this = this,
	);
	if (auth)
	{
		enumerator->wrapper = auth_cfg_wrapper_create(auth);
		add_local_set(this, &enumerator->wrapper->set, FALSE);
	}
	this->lock->read_lock(this->lock);
	return &enumerator->public;
}

/**
 * Check if a helper contains a certificate as trust anchor
 */
static bool auth_contains_cacert(auth_cfg_t *auth, certificate_t *cert)
{
	enumerator_t *enumerator;
	identification_t *value;
	auth_rule_t type;
	bool found = FALSE;

	enumerator = auth->create_enumerator(auth);
	while (enumerator->enumerate(enumerator, &type, &value))
	{
		if (type == AUTH_RULE_CA_CERT &&
			cert->equals(cert, (certificate_t*)value))
		{
			found = TRUE;
			break;
		}
	}
	enumerator->destroy(enumerator);
	return found;
}

/**
 * build a trustchain from subject up to a trust anchor in trusted
 */
static auth_cfg_t *build_trustchain(private_credential_manager_t *this,
									 certificate_t *subject, auth_cfg_t *auth)
{
	certificate_t *issuer, *current;
	auth_cfg_t *trustchain;
	int pathlen = 0;
	bool has_anchor;

	trustchain = auth_cfg_create();
	has_anchor = auth->get(auth, AUTH_RULE_CA_CERT) != NULL;
	current = subject->get_ref(subject);
	while (TRUE)
	{
		if (auth_contains_cacert(auth, current))
		{
			trustchain->add(trustchain, AUTH_RULE_CA_CERT, current);
			return trustchain;
		}
		if (subject == current)
		{
			trustchain->add(trustchain, AUTH_RULE_SUBJECT_CERT, current);
		}
		else
		{
			if (!has_anchor && issued_by(this, current, current, NULL))
			{	/* If no trust anchor specified, accept any CA */
				trustchain->add(trustchain, AUTH_RULE_CA_CERT, current);
				return trustchain;
			}
			trustchain->add(trustchain, AUTH_RULE_IM_CERT, current);
		}
		if (pathlen++ > MAX_TRUST_PATH_LEN)
		{
			break;
		}
		issuer = get_issuer_cert(this, current, FALSE, NULL);
		if (!issuer)
		{
			if (!has_anchor)
			{	/* If no trust anchor specified, accept incomplete chains */
				return trustchain;
			}
			break;
		}
		if (has_anchor && issuer->equals(issuer, current))
		{
			issuer->destroy(issuer);
			break;
		}
		current = issuer;
	}
	trustchain->destroy(trustchain);
	return NULL;
}

/**
 * find a private key of a given certificate
 */
static private_key_t *get_private_by_cert(private_credential_manager_t *this,
										  certificate_t *cert, key_type_t type)
{
	private_key_t *private = NULL;
	identification_t *keyid;
	chunk_t chunk;
	public_key_t *public;

	public = cert->get_public_key(cert);
	if (public)
	{
		if (public->get_fingerprint(public, KEYID_PUBKEY_SHA1, &chunk))
		{
			keyid = identification_create_from_encoding(ID_KEY_ID, chunk);
			private = get_private_by_keyid(this, type, keyid);
			keyid->destroy(keyid);
		}
		public->destroy(public);
	}
	return private;
}

/**
 * Move the actually used certificate to front, so it gets returned with get()
 */
static void prefer_cert(auth_cfg_t *auth, certificate_t *cert)
{
	enumerator_t *enumerator;
	auth_rule_t rule;
	certificate_t *current;

	enumerator = auth->create_enumerator(auth);
	while (enumerator->enumerate(enumerator, &rule, &current))
	{
		if (rule == AUTH_RULE_SUBJECT_CERT)
		{
			current->get_ref(current);
			auth->replace(auth, enumerator, AUTH_RULE_SUBJECT_CERT, cert);
			cert = current;
		}
	}
	enumerator->destroy(enumerator);
	auth->add(auth, AUTH_RULE_SUBJECT_CERT, cert);
}

METHOD(credential_manager_t, get_private, private_key_t*,
	private_credential_manager_t *this, key_type_t type, identification_t *id,
	auth_cfg_t *auth)
{
	enumerator_t *enumerator;
	certificate_t *cert;
	private_key_t *private = NULL;
	auth_cfg_t *trustchain;
	auth_rule_t rule;

	/* check if this is a lookup by key ID, and do it if so */
	if (id && id->get_type(id) == ID_KEY_ID)
	{
		private = get_private_by_keyid(this, type, id);
		if (private)
		{
			return private;
		}
	}

	if (auth)
	{
		/* try to find a trustchain with one of the configured subject certs */
		enumerator = auth->create_enumerator(auth);
		while (enumerator->enumerate(enumerator, &rule, &cert))
		{
			if (rule == AUTH_RULE_SUBJECT_CERT)
			{
				private = get_private_by_cert(this, cert, type);
				if (private)
				{
					trustchain = build_trustchain(this, cert, auth);
					if (trustchain)
					{
						auth->merge(auth, trustchain, FALSE);
						prefer_cert(auth, cert->get_ref(cert));
						trustchain->destroy(trustchain);
						break;
					}
					private->destroy(private);
					private = NULL;
				}
			}
		}
		enumerator->destroy(enumerator);
		if (private)
		{
			return private;
		}

		/* if none yielded a trustchain, enforce the first configured cert */
		cert = auth->get(auth, AUTH_RULE_SUBJECT_CERT);
		if (cert)
		{
			private = get_private_by_cert(this, cert, type);
			if (private)
			{
				trustchain = build_trustchain(this, cert, auth);
				if (trustchain)
				{
					auth->merge(auth, trustchain, FALSE);
					trustchain->destroy(trustchain);
				}
				return private;
			}
		}

		/* try to build a trust chain for each certificate found */
		enumerator = create_cert_enumerator(this, CERT_ANY, type, id, FALSE);
		while (enumerator->enumerate(enumerator, &cert))
		{
			private = get_private_by_cert(this, cert, type);
			if (private)
			{
				trustchain = build_trustchain(this, cert, auth);
				if (trustchain)
				{
					auth->merge(auth, trustchain, FALSE);
					trustchain->destroy(trustchain);
					break;
				}
				private->destroy(private);
				private = NULL;
			}
		}
		enumerator->destroy(enumerator);
	}

	/* if no valid trustchain was found, fall back to the first usable cert */
	if (!private)
	{
		enumerator = create_cert_enumerator(this, CERT_ANY, type, id, FALSE);
		while (enumerator->enumerate(enumerator, &cert))
		{
			private = get_private_by_cert(this, cert, type);
			if (private)
			{
				if (auth)
				{
					auth->add(auth, AUTH_RULE_SUBJECT_CERT, cert->get_ref(cert));
				}
				break;
			}
		}
		enumerator->destroy(enumerator);
	}
	return private;
}

METHOD(credential_manager_t, flush_cache, void,
	private_credential_manager_t *this, certificate_type_t type)
{
	if (this->cache)
	{
		this->cache->flush(this->cache, type);
	}
}

METHOD(credential_manager_t, add_set, void,
	private_credential_manager_t *this, credential_set_t *set)
{
	this->lock->write_lock(this->lock);
	this->sets->insert_last(this->sets, set);
	this->lock->unlock(this->lock);
}

METHOD(credential_manager_t, remove_set, void,
	private_credential_manager_t *this, credential_set_t *set)
{
	this->lock->write_lock(this->lock);
	this->sets->remove(this->sets, set, NULL);
	this->lock->unlock(this->lock);
}

METHOD(credential_manager_t, add_validator, void,
	private_credential_manager_t *this, cert_validator_t *vdtr)
{
	this->lock->write_lock(this->lock);
	this->validators->insert_last(this->validators, vdtr);
	this->lock->unlock(this->lock);
}

METHOD(credential_manager_t, remove_validator, void,
	private_credential_manager_t *this, cert_validator_t *vdtr)
{
	this->lock->write_lock(this->lock);
	this->validators->remove(this->validators, vdtr, NULL);
	this->lock->unlock(this->lock);
}

METHOD(credential_manager_t, destroy, void,
	private_credential_manager_t *this)
{
	cache_queue(this);
	this->cache_queue->destroy(this->cache_queue);
	if (this->cache)
	{
		this->sets->remove(this->sets, this->cache, NULL);
		this->cache->destroy(this->cache);
	}
	this->sets->destroy(this->sets);
	this->local_sets->destroy(this->local_sets);
	this->exclusive_local_sets->destroy(this->exclusive_local_sets);
	this->validators->destroy(this->validators);
	this->lock->destroy(this->lock);
	this->queue_mutex->destroy(this->queue_mutex);
	free(this);
}

/*
 * see header file
 */
credential_manager_t *credential_manager_create()
{
	private_credential_manager_t *this;

	INIT(this,
		.public = {
			.create_cert_enumerator = _create_cert_enumerator,
			.create_shared_enumerator = _create_shared_enumerator,
			.create_cdp_enumerator = _create_cdp_enumerator,
			.get_cert = _get_cert,
			.get_shared = _get_shared,
			.get_private = _get_private,
			.create_trusted_enumerator = _create_trusted_enumerator,
			.create_public_enumerator = _create_public_enumerator,
			.flush_cache = _flush_cache,
			.cache_cert = _cache_cert,
			.issued_by = _issued_by,
			.add_set = _add_set,
			.remove_set = _remove_set,
			.add_local_set = _add_local_set,
			.remove_local_set = _remove_local_set,
			.add_validator = _add_validator,
			.remove_validator = _remove_validator,
			.set_hook = _set_hook,
			.call_hook = _call_hook,
			.destroy = _destroy,
		},
		.sets = linked_list_create(),
		.validators = linked_list_create(),
		.cache_queue = linked_list_create(),
		.lock = rwlock_create(RWLOCK_TYPE_DEFAULT),
		.queue_mutex = mutex_create(MUTEX_TYPE_DEFAULT),
	);

	this->local_sets = thread_value_create((thread_cleanup_t)this->sets->destroy);
	this->exclusive_local_sets = thread_value_create((thread_cleanup_t)this->sets->destroy);
	if (lib->settings->get_bool(lib->settings, "%s.cert_cache", TRUE, lib->ns))
	{
		this->cache = cert_cache_create();
		this->sets->insert_first(this->sets, this->cache);
	}

	return &this->public;
}
