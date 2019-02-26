/*
 * Copyright (C) 2010-2016 Tobias Brunner
 * HSR Hochschule fuer Technik Rapperswil
 *
 * Copyright (C) 2010 Martin Willi
 * Copyright (C) 2010 revosec AG
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

#include "mem_cred.h"

#include <threading/rwlock.h>
#include <collections/linked_list.h>

typedef struct private_mem_cred_t private_mem_cred_t;

/**
 * Private data of an mem_cred_t object.
 */
struct private_mem_cred_t {

	/**
	 * Public mem_cred_t interface.
	 */
	mem_cred_t public;

	/**
	 * Lock for this set
	 */
	rwlock_t *lock;

	/**
	 * List of trusted certificates, certificate_t
	 */
	linked_list_t *trusted;

	/**
	 * List of trusted and untrusted certificates, certificate_t
	 */
	linked_list_t *untrusted;

	/**
	 * List of private keys, private_key_t
	 */
	linked_list_t *keys;

	/**
	 * List of shared keys, as shared_entry_t
	 */
	linked_list_t *shared;

	/**
	 * List of CDPs, as cdp_t
	 */
	linked_list_t *cdps;
};

/**
 * Data for the certificate enumerator
 */
typedef struct {
	rwlock_t *lock;
	certificate_type_t cert;
	key_type_t key;
	identification_t *id;
} cert_data_t;

CALLBACK(cert_data_destroy, void,
	cert_data_t *data)
{
	data->lock->unlock(data->lock);
	free(data);
}

CALLBACK(certs_filter, bool,
	cert_data_t *data, enumerator_t *orig, va_list args)
{
	public_key_t *public;
	certificate_t *cert, **out;

	VA_ARGS_VGET(args, out);

	while (orig->enumerate(orig, &cert))
	{
		if (data->cert != CERT_ANY && data->cert != cert->get_type(cert))
		{
			continue;
		}
		public = cert->get_public_key(cert);
		if (public)
		{
			if (data->key == KEY_ANY || data->key == public->get_type(public))
			{
				if (data->id && public->has_fingerprint(public,
											data->id->get_encoding(data->id)))
				{
					public->destroy(public);
					*out = cert;
					return TRUE;
				}
			}
			public->destroy(public);
		}
		else if (data->key != KEY_ANY)
		{
			continue;
		}
		if (!data->id || cert->has_subject(cert, data->id))
		{
			*out = cert;
			return TRUE;
		}
	}
	return FALSE;
}

METHOD(credential_set_t, create_cert_enumerator, enumerator_t*,
	private_mem_cred_t *this, certificate_type_t cert, key_type_t key,
	identification_t *id, bool trusted)
{
	cert_data_t *data;
	enumerator_t *enumerator;

	INIT(data,
		.lock = this->lock,
		.cert = cert,
		.key = key,
		.id = id,
	);
	this->lock->read_lock(this->lock);
	if (trusted)
	{
		enumerator = this->trusted->create_enumerator(this->trusted);
	}
	else
	{
		enumerator = this->untrusted->create_enumerator(this->untrusted);
	}
	return enumerator_create_filter(enumerator, certs_filter, data,
									cert_data_destroy);
}

CALLBACK(certificate_equals, bool,
	certificate_t *item, va_list args)
{
	certificate_t *cert;

	VA_ARGS_VGET(args, cert);
	return item->equals(item, cert);
}

/**
 * Add a certificate the the cache. Returns a reference to "cert" or a
 * previously cached certificate that equals "cert".
 */
static certificate_t *add_cert_internal(private_mem_cred_t *this, bool trusted,
										certificate_t *cert)
{
	certificate_t *cached;
	this->lock->write_lock(this->lock);
	if (this->untrusted->find_first(this->untrusted, certificate_equals,
									(void**)&cached, cert))
	{
		cert->destroy(cert);
		cert = cached->get_ref(cached);
	}
	else
	{
		if (trusted)
		{
			this->trusted->insert_first(this->trusted, cert->get_ref(cert));
		}
		this->untrusted->insert_first(this->untrusted, cert->get_ref(cert));
	}
	this->lock->unlock(this->lock);
	return cert;
}

METHOD(mem_cred_t, add_cert, void,
	private_mem_cred_t *this, bool trusted, certificate_t *cert)
{
	certificate_t *cached = add_cert_internal(this, trusted, cert);
	cached->destroy(cached);
}

METHOD(mem_cred_t, add_cert_ref, certificate_t*,
	private_mem_cred_t *this, bool trusted, certificate_t *cert)
{
	return add_cert_internal(this, trusted, cert);
}

METHOD(mem_cred_t, get_cert_ref, certificate_t*,
	private_mem_cred_t *this, certificate_t *cert)
{
	certificate_t *cached;

	this->lock->read_lock(this->lock);
	if (this->untrusted->find_first(this->untrusted, certificate_equals,
									(void**)&cached, cert))
	{
		cert->destroy(cert);
		cert = cached->get_ref(cached);
	}
	this->lock->unlock(this->lock);

	return cert;
}

METHOD(mem_cred_t, add_crl, bool,
	private_mem_cred_t *this, crl_t *crl)
{
	certificate_t *current, *cert = &crl->certificate;
	enumerator_t *enumerator;
	bool new = TRUE;

	this->lock->write_lock(this->lock);
	enumerator = this->untrusted->create_enumerator(this->untrusted);
	while (enumerator->enumerate(enumerator, (void**)&current))
	{
		if (current->get_type(current) == CERT_X509_CRL)
		{
			chunk_t base;
			bool found = FALSE;
			crl_t *crl_c = (crl_t*)current;
			chunk_t authkey = crl->get_authKeyIdentifier(crl);
			chunk_t authkey_c = crl_c->get_authKeyIdentifier(crl_c);

			/* compare authorityKeyIdentifiers if available */
			if (chunk_equals(authkey, authkey_c))
			{
				found = TRUE;
			}
			else
			{
				identification_t *issuer = cert->get_issuer(cert);
				identification_t *issuer_c = current->get_issuer(current);

				/* otherwise compare issuer distinguished names */
				if (issuer->equals(issuer, issuer_c))
				{
					found = TRUE;
				}
			}
			if (found)
			{
				/* we keep at most one delta CRL for each base CRL */
				if (crl->is_delta_crl(crl, &base))
				{
					if (!crl_c->is_delta_crl(crl_c, NULL))
					{
						if (chunk_equals(base, crl_c->get_serial(crl_c)))
						{	/* keep the added delta and the existing base CRL
							 * but check if this is the newest delta CRL for
							 * the same base */
							continue;
						}
					}
				}
				else if (crl_c->is_delta_crl(crl_c, &base))
				{
					if (chunk_equals(base, crl->get_serial(crl)))
					{	/* keep the existing delta and the added base CRL,
						 * but check if we don't store it already */
						continue;
					}
				}
				new = crl_is_newer(crl, crl_c);
				if (!new)
				{
					cert->destroy(cert);
					break;
				}
				/* we remove the existing older CRL but there might be other
				 * delta or base CRLs we can replace */
				this->untrusted->remove_at(this->untrusted, enumerator);
				current->destroy(current);
			}
		}
	}
	enumerator->destroy(enumerator);

	if (new)
	{
		this->untrusted->insert_first(this->untrusted, cert);
	}
	this->lock->unlock(this->lock);
	return new;
}

/**
 * Data for key enumerator
 */
typedef struct {
	rwlock_t *lock;
	key_type_t type;
	identification_t *id;
} key_data_t;

CALLBACK(key_data_destroy, void,
	key_data_t *data)
{
	data->lock->unlock(data->lock);
	free(data);
}

CALLBACK(key_filter, bool,
	key_data_t *data, enumerator_t *orig, va_list args)
{
	private_key_t *key, **out;

	VA_ARGS_VGET(args, out);

	while (orig->enumerate(orig, &key))
	{
		if (data->type == KEY_ANY || data->type == key->get_type(key))
		{
			if (data->id == NULL ||
				key->has_fingerprint(key, data->id->get_encoding(data->id)))
			{
				*out = key;
				return TRUE;
			}
		}
	}
	return FALSE;
}

METHOD(credential_set_t, create_private_enumerator, enumerator_t*,
	private_mem_cred_t *this, key_type_t type, identification_t *id)
{
	key_data_t *data;

	INIT(data,
		.lock = this->lock,
		.type = type,
		.id = id,
	);
	this->lock->read_lock(this->lock);
	return enumerator_create_filter(this->keys->create_enumerator(this->keys),
									key_filter, data, key_data_destroy);
}

METHOD(mem_cred_t, add_key, void,
	private_mem_cred_t *this, private_key_t *key)
{
	enumerator_t *enumerator;
	private_key_t *current;

	this->lock->write_lock(this->lock);

	enumerator = this->keys->create_enumerator(this->keys);
	while (enumerator->enumerate(enumerator, &current))
	{
		if (current->equals(current, key))
		{
			this->keys->remove_at(this->keys, enumerator);
			current->destroy(current);
			break;
		}
	}
	enumerator->destroy(enumerator);

	this->keys->insert_first(this->keys, key);

	this->lock->unlock(this->lock);
}

METHOD(mem_cred_t, remove_key, bool,
	private_mem_cred_t *this, chunk_t fp)
{
	enumerator_t *enumerator;
	private_key_t *current;
	bool found = FALSE;

	this->lock->write_lock(this->lock);

	enumerator = this->keys->create_enumerator(this->keys);
	while (enumerator->enumerate(enumerator, &current))
	{
		if (current->has_fingerprint(current, fp))
		{
			this->keys->remove_at(this->keys, enumerator);
			current->destroy(current);
			found = TRUE;
			break;
		}
	}
	enumerator->destroy(enumerator);

	this->lock->unlock(this->lock);
	return found;
}

/**
 * Shared key entry
 */
typedef struct {
	/** shared key */
	shared_key_t *shared;
	/** list of owners, identification_t */
	linked_list_t *owners;
	/** optional unique identifier */
	char *id;
} shared_entry_t;

/**
 * Clean up a shared entry
 */
static void shared_entry_destroy(shared_entry_t *entry)
{
	entry->owners->destroy_offset(entry->owners,
								  offsetof(identification_t, destroy));
	entry->shared->destroy(entry->shared);
	free(entry->id);
	free(entry);
}

/**
 * Check if two shared key entries are equal (ignoring the unique identifier)
 */
static bool shared_entry_equals(shared_entry_t *a, shared_entry_t *b)
{
	enumerator_t *e1, *e2;
	identification_t *id1, *id2;
	bool equals = TRUE;

	if (a->shared->get_type(a->shared) != b->shared->get_type(b->shared))
	{
		return FALSE;
	}
	if (!chunk_equals(a->shared->get_key(a->shared),
					  b->shared->get_key(b->shared)))
	{
		return FALSE;
	}
	if (a->owners->get_count(a->owners) != b->owners->get_count(b->owners))
	{
		return FALSE;
	}
	e1 = a->owners->create_enumerator(a->owners);
	e2 = b->owners->create_enumerator(b->owners);
	while (e1->enumerate(e1, &id1) && e2->enumerate(e2, &id2))
	{
		if (!id1->equals(id1, id2))
		{
			equals = FALSE;
			break;
		}
	}
	e1->destroy(e1);
	e2->destroy(e2);

	return equals;
}

/**
 * Data for the shared_key enumerator
 */
typedef struct {
	rwlock_t *lock;
	identification_t *me;
	identification_t *other;
	shared_key_type_t type;
} shared_data_t;

CALLBACK(shared_data_destroy, void,
	shared_data_t *data)
{
	data->lock->unlock(data->lock);
	free(data);
}

/**
 * Get the best match of an owner in an entry.
 */
static id_match_t has_owner(shared_entry_t *entry, identification_t *owner)
{
	enumerator_t *enumerator;
	id_match_t match, best = ID_MATCH_NONE;
	identification_t *current;

	enumerator = entry->owners->create_enumerator(entry->owners);
	while (enumerator->enumerate(enumerator, &current))
	{
		match  = owner->matches(owner, current);
		if (match > best)
		{
			best = match;
		}
	}
	enumerator->destroy(enumerator);
	return best;
}

CALLBACK(shared_filter, bool,
	shared_data_t *data, enumerator_t *orig, va_list args)
{
	id_match_t my_match = ID_MATCH_NONE, other_match = ID_MATCH_NONE;
	shared_entry_t *entry;
	shared_key_t **out;
	id_match_t *me, *other;

	VA_ARGS_VGET(args, out, me, other);

	while (orig->enumerate(orig, &entry))
	{
		if (data->type != SHARED_ANY &&
			entry->shared->get_type(entry->shared) != data->type)
		{
			continue;
		}
		if (data->me)
		{
			my_match = has_owner(entry, data->me);
		}
		if (data->other)
		{
			other_match = has_owner(entry, data->other);
		}
		if ((data->me || data->other) && (!my_match && !other_match))
		{
			continue;
		}
		*out = entry->shared;
		if (me)
		{
			*me = my_match;
		}
		if (other)
		{
			*other = other_match;
		}
		return TRUE;
	}
	return FALSE;
}

METHOD(credential_set_t, create_shared_enumerator, enumerator_t*,
	private_mem_cred_t *this, shared_key_type_t type,
	identification_t *me, identification_t *other)
{
	shared_data_t *data;

	INIT(data,
		.lock = this->lock,
		.me = me,
		.other = other,
		.type = type,
	);
	data->lock->read_lock(data->lock);
	return enumerator_create_filter(
						this->shared->create_enumerator(this->shared),
						shared_filter, data, shared_data_destroy);
}

METHOD(mem_cred_t, add_shared_unique, void,
	private_mem_cred_t *this, char *id, shared_key_t *shared,
	linked_list_t* owners)
{
	shared_entry_t *current, *new;
	enumerator_t *enumerator;

	INIT(new,
		.shared = shared,
		.owners = owners,
		.id = strdupnull(id),
	);

	this->lock->write_lock(this->lock);

	enumerator = this->shared->create_enumerator(this->shared);
	while (enumerator->enumerate(enumerator, &current))
	{
		/* always replace keys with the same unique identifier, only compare
		 * them if both have no unique id assigned */
		if ((id && streq(id, current->id)) ||
			(!id && !current->id && shared_entry_equals(current, new)))
		{
			this->shared->remove_at(this->shared, enumerator);
			shared_entry_destroy(current);
			break;
		}
	}
	enumerator->destroy(enumerator);

	this->shared->insert_first(this->shared, new);

	this->lock->unlock(this->lock);
}

METHOD(mem_cred_t, add_shared_list, void,
	private_mem_cred_t *this, shared_key_t *shared, linked_list_t* owners)
{
	add_shared_unique(this, NULL, shared, owners);
}

METHOD(mem_cred_t, add_shared, void,
	private_mem_cred_t *this, shared_key_t *shared, ...)
{
	identification_t *id;
	linked_list_t *owners = linked_list_create();
	va_list args;

	va_start(args, shared);
	do
	{
		id = va_arg(args, identification_t*);
		if (id)
		{
			owners->insert_first(owners, id);
		}
	}
	while (id);
	va_end(args);

	add_shared_list(this, shared, owners);
}

METHOD(mem_cred_t, remove_shared_unique, void,
	private_mem_cred_t *this, char *id)
{
	enumerator_t *enumerator;
	shared_entry_t *current;

	if (!id)
	{
		return;
	}

	this->lock->write_lock(this->lock);

	enumerator = this->shared->create_enumerator(this->shared);
	while (enumerator->enumerate(enumerator, &current))
	{
		if (streq(id, current->id))
		{
			this->shared->remove_at(this->shared, enumerator);
			shared_entry_destroy(current);
			break;
		}
	}
	enumerator->destroy(enumerator);

	this->lock->unlock(this->lock);
}

CALLBACK(unique_filter, bool,
	void *unused, enumerator_t *orig, va_list args)
{
	shared_entry_t *entry;
	char **id;

	VA_ARGS_VGET(args, id);

	while (orig->enumerate(orig, &entry))
	{
		if (!entry->id)
		{
			continue;
		}
		if (id)
		{
			*id = entry->id;
		}
		return TRUE;
	}
	return FALSE;
}

METHOD(mem_cred_t, create_unique_shared_enumerator, enumerator_t*,
	private_mem_cred_t *this)
{
	this->lock->read_lock(this->lock);
	return enumerator_create_filter(
								this->shared->create_enumerator(this->shared),
								unique_filter, this->lock,
								(void*)this->lock->unlock);
}

/**
 * Certificate distribution point
 */
typedef struct {
	certificate_type_t type;
	identification_t *id;
	char *uri;
} cdp_t;

/**
 * Destroy a CDP entry
 */
static void cdp_destroy(cdp_t *this)
{
	this->id->destroy(this->id);
	free(this->uri);
	free(this);
}

METHOD(mem_cred_t, add_cdp, void,
	private_mem_cred_t *this, certificate_type_t type,
	identification_t *id, char *uri)
{
	cdp_t *cdp;

	INIT(cdp,
		.type = type,
		.id = id->clone(id),
		.uri = strdup(uri),
	);
	this->lock->write_lock(this->lock);
	this->cdps->insert_last(this->cdps, cdp);
	this->lock->unlock(this->lock);
}

/**
 * CDP enumerator data
 */
typedef struct {
	certificate_type_t type;
	identification_t *id;
	rwlock_t *lock;
} cdp_data_t;

CALLBACK(cdp_data_destroy, void,
	cdp_data_t *data)
{
	data->lock->unlock(data->lock);
	free(data);
}

CALLBACK(cdp_filter, bool,
	cdp_data_t *data, enumerator_t *orig, va_list args)
{
	cdp_t *cdp;
	char **uri;

	VA_ARGS_VGET(args, uri);

	while (orig->enumerate(orig, &cdp))
	{
		if (data->type != CERT_ANY && data->type != cdp->type)
		{
			continue;
		}
		if (data->id && !cdp->id->matches(cdp->id, data->id))
		{
			continue;
		}
		*uri = cdp->uri;
		return TRUE;
	}
	return FALSE;
}

METHOD(credential_set_t, create_cdp_enumerator, enumerator_t*,
	private_mem_cred_t *this, certificate_type_t type, identification_t *id)
{
	cdp_data_t *data;

	INIT(data,
		.type = type,
		.id = id,
		.lock = this->lock,
	);
	this->lock->read_lock(this->lock);
	return enumerator_create_filter(this->cdps->create_enumerator(this->cdps),
									cdp_filter, data, cdp_data_destroy);

}

static void reset_certs(private_mem_cred_t *this)
{
	this->trusted->destroy_offset(this->trusted,
								  offsetof(certificate_t, destroy));
	this->untrusted->destroy_offset(this->untrusted,
									offsetof(certificate_t, destroy));
	this->trusted = linked_list_create();
	this->untrusted = linked_list_create();
}

static void copy_certs(linked_list_t *dst, linked_list_t *src, bool clone)
{
	enumerator_t *enumerator;
	certificate_t *cert;

	enumerator = src->create_enumerator(src);
	while (enumerator->enumerate(enumerator, &cert))
	{
		if (clone)
		{
			cert = cert->get_ref(cert);
		}
		else
		{
			src->remove_at(src, enumerator);
		}
		dst->insert_last(dst, cert);
	}
	enumerator->destroy(enumerator);
}

METHOD(mem_cred_t, replace_certs, void,
	private_mem_cred_t *this, mem_cred_t *other_set, bool clone)
{
	private_mem_cred_t *other = (private_mem_cred_t*)other_set;

	this->lock->write_lock(this->lock);
	reset_certs(this);
	copy_certs(this->untrusted, other->untrusted, clone);
	copy_certs(this->trusted, other->trusted, clone);
	this->lock->unlock(this->lock);
}

static void reset_secrets(private_mem_cred_t *this)
{
	this->keys->destroy_offset(this->keys, offsetof(private_key_t, destroy));
	this->shared->destroy_function(this->shared, (void*)shared_entry_destroy);
	this->keys = linked_list_create();
	this->shared = linked_list_create();
}

METHOD(mem_cred_t, replace_secrets, void,
	private_mem_cred_t *this, mem_cred_t *other_set, bool clone)
{
	private_mem_cred_t *other = (private_mem_cred_t*)other_set;
	enumerator_t *enumerator;
	shared_entry_t *entry, *new_entry;
	private_key_t *key;

	this->lock->write_lock(this->lock);

	reset_secrets(this);

	if (clone)
	{
		enumerator = other->keys->create_enumerator(other->keys);
		while (enumerator->enumerate(enumerator, &key))
		{
			this->keys->insert_last(this->keys, key->get_ref(key));
		}
		enumerator->destroy(enumerator);
		enumerator = other->shared->create_enumerator(other->shared);
		while (enumerator->enumerate(enumerator, &entry))
		{
			INIT(new_entry,
				.shared = entry->shared->get_ref(entry->shared),
				.owners = entry->owners->clone_offset(entry->owners,
											offsetof(identification_t, clone)),
			);
			this->shared->insert_last(this->shared, new_entry);
		}
		enumerator->destroy(enumerator);
	}
	else
	{
		while (other->keys->remove_first(other->keys, (void**)&key) == SUCCESS)
		{
			this->keys->insert_last(this->keys, key);
		}
		while (other->shared->remove_first(other->shared,
										  (void**)&entry) == SUCCESS)
		{
			this->shared->insert_last(this->shared, entry);
		}
	}
	this->lock->unlock(this->lock);
}

METHOD(mem_cred_t, clear_secrets, void,
	private_mem_cred_t *this)
{
	this->lock->write_lock(this->lock);
	reset_secrets(this);
	this->lock->unlock(this->lock);
}

METHOD(mem_cred_t, clear_, void,
	private_mem_cred_t *this)
{
	this->lock->write_lock(this->lock);
	this->cdps->destroy_function(this->cdps, (void*)cdp_destroy);
	this->cdps = linked_list_create();
	reset_certs(this);
	reset_secrets(this);
	this->lock->unlock(this->lock);
}

METHOD(mem_cred_t, destroy, void,
	private_mem_cred_t *this)
{
	clear_(this);
	this->trusted->destroy(this->trusted);
	this->untrusted->destroy(this->untrusted);
	this->keys->destroy(this->keys);
	this->shared->destroy(this->shared);
	this->cdps->destroy(this->cdps);
	this->lock->destroy(this->lock);
	free(this);
}

/**
 * See header
 */
mem_cred_t *mem_cred_create()
{
	private_mem_cred_t *this;

	INIT(this,
		.public = {
			.set = {
				.create_shared_enumerator = _create_shared_enumerator,
				.create_private_enumerator = _create_private_enumerator,
				.create_cert_enumerator = _create_cert_enumerator,
				.create_cdp_enumerator  = _create_cdp_enumerator,
				.cache_cert = (void*)nop,
			},
			.add_cert = _add_cert,
			.add_cert_ref = _add_cert_ref,
			.get_cert_ref = _get_cert_ref,
			.add_crl = _add_crl,
			.add_key = _add_key,
			.remove_key = _remove_key,
			.add_shared = _add_shared,
			.add_shared_list = _add_shared_list,
			.add_shared_unique = _add_shared_unique,
			.remove_shared_unique = _remove_shared_unique,
			.create_unique_shared_enumerator = _create_unique_shared_enumerator,
			.add_cdp = _add_cdp,
			.replace_certs = _replace_certs,
			.replace_secrets = _replace_secrets,
			.clear = _clear_,
			.clear_secrets = _clear_secrets,
			.destroy = _destroy,
		},
		.trusted = linked_list_create(),
		.untrusted = linked_list_create(),
		.keys = linked_list_create(),
		.shared = linked_list_create(),
		.cdps = linked_list_create(),
		.lock = rwlock_create(RWLOCK_TYPE_DEFAULT),
	);

	return &this->public;
}
