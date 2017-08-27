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

#define _GNU_SOURCE /* for stdndup() */
#include <string.h>

#include "xauth_manager.h"

#include <collections/linked_list.h>
#include <threading/rwlock.h>

typedef struct private_xauth_manager_t private_xauth_manager_t;
typedef struct xauth_entry_t xauth_entry_t;

/**
 * XAuth constructor entry
 */
struct xauth_entry_t {

	/**
	 * Xauth backend name
	 */
	char *name;

	/**
	 * Role of the method, XAUTH_SERVER or XAUTH_PEER
	 */
	xauth_role_t role;

	/**
	 * constructor function to create instance
	 */
	xauth_constructor_t constructor;
};

/**
 * private data of xauth_manager
 */
struct private_xauth_manager_t {

	/**
	 * public functions
	 */
	xauth_manager_t public;

	/**
	 * list of eap_entry_t's
	 */
	linked_list_t *methods;

	/**
	 * rwlock to lock methods
	 */
	rwlock_t *lock;
};

METHOD(xauth_manager_t, add_method, void,
	private_xauth_manager_t *this, char *name, xauth_role_t role,
	xauth_constructor_t constructor)
{
	xauth_entry_t *entry;

	INIT(entry,
		.name = name,
		.role = role,
		.constructor = constructor,
	);

	this->lock->write_lock(this->lock);
	this->methods->insert_last(this->methods, entry);
	this->lock->unlock(this->lock);
}

METHOD(xauth_manager_t, remove_method, void,
	private_xauth_manager_t *this, xauth_constructor_t constructor)
{
	enumerator_t *enumerator;
	xauth_entry_t *entry;

	this->lock->write_lock(this->lock);
	enumerator = this->methods->create_enumerator(this->methods);
	while (enumerator->enumerate(enumerator, &entry))
	{
		if (constructor == entry->constructor)
		{
			this->methods->remove_at(this->methods, enumerator);
			free(entry);
		}
	}
	enumerator->destroy(enumerator);
	this->lock->unlock(this->lock);
}

METHOD(xauth_manager_t, create_instance, xauth_method_t*,
	private_xauth_manager_t *this, char *name, xauth_role_t role,
	identification_t *server, identification_t *peer)
{
	enumerator_t *enumerator;
	xauth_entry_t *entry;
	xauth_method_t *method = NULL;
	char *profile = NULL;

	if (name)
	{
		profile = strchr(name, ':');
		if (profile)
		{
			name = strndup(name, profile - name);
			profile++;
		}
	}

	this->lock->read_lock(this->lock);
	enumerator = this->methods->create_enumerator(this->methods);
	while (enumerator->enumerate(enumerator, &entry))
	{
		if (!name && streq(entry->name, "noauth"))
		{	/* xauth-noauth has to be configured explicitly */
			continue;
		}
		if (role == entry->role && (!name || streq(name, entry->name)))
		{
			method = entry->constructor(server, peer, profile);
			if (method)
			{
				break;
			}
		}
	}
	enumerator->destroy(enumerator);
	this->lock->unlock(this->lock);
	if (profile)
	{
		free(name);
	}
	return method;
}

METHOD(xauth_manager_t, destroy, void,
	private_xauth_manager_t *this)
{
	this->methods->destroy_function(this->methods, free);
	this->lock->destroy(this->lock);
	free(this);
}

/*
 * See header
 */
xauth_manager_t *xauth_manager_create()
{
	private_xauth_manager_t *this;

	INIT(this,
		.public = {
			.add_method = _add_method,
			.remove_method = _remove_method,
			.create_instance = _create_instance,
			.destroy = _destroy,
		},
		.methods = linked_list_create(),
		.lock = rwlock_create(RWLOCK_TYPE_DEFAULT),
	);

	return &this->public;
}
