/*
 * Copyright (C) 2011 Martin Willi
 * Copyright (C) 2011 revosec AG
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

#include "whitelist_listener.h"

#include <daemon.h>
#include <collections/hashtable.h>
#include <threading/rwlock.h>

typedef struct private_whitelist_listener_t private_whitelist_listener_t;

/**
 * Private data of an whitelist_listener_t object.
 */
struct private_whitelist_listener_t {

	/**
	 * Public whitelist_listener_t interface.
	 */
	whitelist_listener_t public;

	/**
	 * Lock for hashtable
	 */
	rwlock_t *lock;

	/**
	 * Hashtable with whitelisted identities
	 */
	hashtable_t *ids;

	/**
	 * Whitelist checking enabled
	 */
	bool enabled;
};

/**
 * Hashtable hash function
 */
static u_int hash(identification_t *key)
{
	return key->hash(key, 0);
}

/**
 * Hashtable equals function
 */
static bool equals(identification_t *a, identification_t *b)
{
	return a->equals(a, b);
}

METHOD(listener_t, authorize, bool,
	private_whitelist_listener_t *this, ike_sa_t *ike_sa,
	bool final, bool *success)
{
	/* check each authentication round */
	if (this->enabled && !final)
	{
		bool whitelisted = FALSE;
		identification_t *id;
		auth_cfg_t *auth;

		auth = ike_sa->get_auth_cfg(ike_sa, FALSE);
		/* for authenticated with EAP, check EAP identity */
		id = auth->get(auth, AUTH_RULE_EAP_IDENTITY);
		if (!id)
		{
			id = auth->get(auth, AUTH_RULE_IDENTITY);
		}
		if (id)
		{
			this->lock->read_lock(this->lock);
			whitelisted = this->ids->get(this->ids, id) != NULL;
			this->lock->unlock(this->lock);
		}
		if (whitelisted)
		{
			DBG2(DBG_CFG, "peer identity '%Y' whitelisted", id);
		}
		else
		{
			DBG1(DBG_CFG, "peer identity '%Y' not whitelisted", id);
			*success = FALSE;
		}
	}
	return TRUE;
}

METHOD(whitelist_listener_t, add, void,
	private_whitelist_listener_t *this, identification_t *id)
{
	id = id->clone(id);
	this->lock->write_lock(this->lock);
	id = this->ids->put(this->ids, id, id);
	this->lock->unlock(this->lock);
	DESTROY_IF(id);
}

METHOD(whitelist_listener_t, remove_, void,
	private_whitelist_listener_t *this, identification_t *id)
{
	this->lock->write_lock(this->lock);
	id = this->ids->remove(this->ids, id);
	this->lock->unlock(this->lock);
	DESTROY_IF(id);
}

CALLBACK(whitelist_filter, bool,
	rwlock_t *lock, enumerator_t *orig, va_list args)
{
	identification_t *key, *value, **out;

	VA_ARGS_VGET(args, out);

	if (orig->enumerate(orig, &key, &value))
	{
		*out = value;
		return TRUE;
	}
	return FALSE;
}

METHOD(whitelist_listener_t, create_enumerator, enumerator_t*,
	private_whitelist_listener_t *this)
{
	this->lock->read_lock(this->lock);
	return enumerator_create_filter(this->ids->create_enumerator(this->ids),
									whitelist_filter, this->lock,
									(void*)this->lock->unlock);
}

METHOD(whitelist_listener_t, flush, void,
	private_whitelist_listener_t *this, identification_t *id)
{
	enumerator_t *enumerator;
	identification_t *key, *value;

	this->lock->write_lock(this->lock);
	enumerator = this->ids->create_enumerator(this->ids);
	while (enumerator->enumerate(enumerator, &key, &value))
	{
		if (value->matches(value, id))
		{
			this->ids->remove_at(this->ids, enumerator);
			value->destroy(value);
		}
	}
	enumerator->destroy(enumerator);
	this->lock->unlock(this->lock);
}

METHOD(whitelist_listener_t, set_active, void,
	private_whitelist_listener_t *this, bool enable)
{
	DBG1(DBG_CFG, "whitelist functionality %s%sabled",
		(this->enabled == enable) ? "was already " : "", enable ? "en" : "dis");
	this->enabled = enable;
}

METHOD(whitelist_listener_t, destroy, void,
	private_whitelist_listener_t *this)
{
	identification_t *key, *value;
	enumerator_t *enumerator;

	enumerator = this->ids->create_enumerator(this->ids);
	while (enumerator->enumerate(enumerator, &key, &value))
	{
		value->destroy(value);
	}
	enumerator->destroy(enumerator);
	this->ids->destroy(this->ids);
	this->lock->destroy(this->lock);
	free(this);
}

/**
 * See header
 */
whitelist_listener_t *whitelist_listener_create()
{
	private_whitelist_listener_t *this;

	INIT(this,
		.public = {
			.listener = {
				.authorize = _authorize,
			},
			.add = _add,
			.remove = _remove_,
			.create_enumerator = _create_enumerator,
			.flush = _flush,
			.set_active = _set_active,
			.destroy = _destroy,
		},
		.lock = rwlock_create(RWLOCK_TYPE_DEFAULT),
		.ids = hashtable_create((hashtable_hash_t)hash,
								(hashtable_equals_t)equals, 32),
		.enabled = lib->settings->get_bool(lib->settings,
								"%s.plugins.whitelist.enable", FALSE, lib->ns),
	);

	return &this->public;
}
