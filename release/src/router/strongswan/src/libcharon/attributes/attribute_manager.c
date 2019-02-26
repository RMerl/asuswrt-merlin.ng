/*
 * Copyright (C) 2008 Martin Willi
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

#include "attribute_manager.h"

#include <utils/debug.h>
#include <collections/linked_list.h>
#include <threading/rwlock.h>

typedef struct private_attribute_manager_t private_attribute_manager_t;

/**
 * private data of attribute_manager
 */
struct private_attribute_manager_t {

	/**
	 * public functions
	 */
	attribute_manager_t public;

	/**
	 * list of registered providers
	 */
	linked_list_t *providers;

	/**
	 * list of registered handlers
	 */
	linked_list_t *handlers;

	/**
	 * rwlock provider list
	 */
	rwlock_t *lock;
};

/**
 * Data to pass to enumerator filters
 */
typedef struct {
	/** attribute group pools */
	linked_list_t *pools;
	/** associated IKE_SA */
	ike_sa_t *ike_sa;
	/** requesting/assigned virtual IPs */
	linked_list_t *vips;
} enum_data_t;

METHOD(attribute_manager_t, acquire_address, host_t*,
	private_attribute_manager_t *this, linked_list_t *pools,
	ike_sa_t *ike_sa, host_t *requested)
{
	enumerator_t *enumerator;
	attribute_provider_t *current;
	host_t *host = NULL;

	this->lock->read_lock(this->lock);
	enumerator = this->providers->create_enumerator(this->providers);
	while (enumerator->enumerate(enumerator, &current))
	{
		host = current->acquire_address(current, pools, ike_sa, requested);
		if (host)
		{
			break;
		}
	}
	enumerator->destroy(enumerator);
	this->lock->unlock(this->lock);

	return host;
}

METHOD(attribute_manager_t, release_address, bool,
	private_attribute_manager_t *this, linked_list_t *pools, host_t *address,
	ike_sa_t *ike_sa)
{
	enumerator_t *enumerator;
	attribute_provider_t *current;
	bool found = FALSE;

	this->lock->read_lock(this->lock);
	enumerator = this->providers->create_enumerator(this->providers);
	while (enumerator->enumerate(enumerator, &current))
	{
		if (current->release_address(current, pools, address, ike_sa))
		{
			found = TRUE;
			break;
		}
	}
	enumerator->destroy(enumerator);
	this->lock->unlock(this->lock);

	return found;
}

/**
 * inner enumerator constructor for responder attributes
 */
static enumerator_t *responder_enum_create(attribute_provider_t *provider,
										   enum_data_t *data)
{
	return provider->create_attribute_enumerator(provider, data->pools,
												 data->ike_sa, data->vips);
}

METHOD(attribute_manager_t, create_responder_enumerator, enumerator_t*,
	private_attribute_manager_t *this, linked_list_t *pools,
	ike_sa_t *ike_sa, linked_list_t *vips)
{
	enum_data_t *data;

	INIT(data,
		.pools = pools,
		.ike_sa = ike_sa,
		.vips = vips,
	);
	this->lock->read_lock(this->lock);
	return enumerator_create_cleaner(
				enumerator_create_nested(
					this->providers->create_enumerator(this->providers),
					(void*)responder_enum_create, data, free),
				(void*)this->lock->unlock, this->lock);
}

METHOD(attribute_manager_t, add_provider, void,
	private_attribute_manager_t *this, attribute_provider_t *provider)
{
	this->lock->write_lock(this->lock);
	this->providers->insert_last(this->providers, provider);
	this->lock->unlock(this->lock);
}

METHOD(attribute_manager_t, remove_provider, void,
	private_attribute_manager_t *this, attribute_provider_t *provider)
{
	this->lock->write_lock(this->lock);
	this->providers->remove(this->providers, provider, NULL);
	this->lock->unlock(this->lock);
}

METHOD(attribute_manager_t, handle, attribute_handler_t*,
	private_attribute_manager_t *this, ike_sa_t *ike_sa,
	attribute_handler_t *handler, configuration_attribute_type_t type,
	chunk_t data)
{
	enumerator_t *enumerator;
	attribute_handler_t *current, *handled = NULL;

	this->lock->read_lock(this->lock);

	/* try to find the passed handler */
	enumerator = this->handlers->create_enumerator(this->handlers);
	while (enumerator->enumerate(enumerator, &current))
	{
		if (current == handler && current->handle(current, ike_sa, type, data))
		{
			handled = current;
			break;
		}
	}
	enumerator->destroy(enumerator);
	if (!handled)
	{	/* handler requesting this attribute not found, try any other */
		enumerator = this->handlers->create_enumerator(this->handlers);
		while (enumerator->enumerate(enumerator, &current))
		{
			if (current->handle(current, ike_sa, type, data))
			{
				handled = current;
				break;
			}
		}
		enumerator->destroy(enumerator);
	}
	this->lock->unlock(this->lock);

	if (!handled)
	{
		DBG1(DBG_CFG, "handling %N attribute failed",
			 configuration_attribute_type_names, type);
	}
	return handled;
}

METHOD(attribute_manager_t, release, void,
	private_attribute_manager_t *this, attribute_handler_t *handler,
	ike_sa_t *ike_sa, configuration_attribute_type_t type, chunk_t data)
{
	enumerator_t *enumerator;
	attribute_handler_t *current;

	this->lock->read_lock(this->lock);
	enumerator = this->handlers->create_enumerator(this->handlers);
	while (enumerator->enumerate(enumerator, &current))
	{
		if (current == handler)
		{
			current->release(current, ike_sa, type, data);
			break;
		}
	}
	enumerator->destroy(enumerator);
	this->lock->unlock(this->lock);
}

/**
 * Enumerator implementation to enumerate nested initiator attributes
 */
typedef struct {
	/** implements enumerator_t */
	enumerator_t public;
	/** back ref */
	private_attribute_manager_t *this;
	/** currently processing handler */
	attribute_handler_t *handler;
	/** outer enumerator over handlers */
	enumerator_t *outer;
	/** inner enumerator over current handlers attributes */
	enumerator_t *inner;
	/** IKE_SA to request attributes for */
	ike_sa_t *ike_sa;
	/** virtual IPs we are requesting along with attriubutes */
	linked_list_t *vips;
} initiator_enumerator_t;

METHOD(enumerator_t, initiator_enumerate, bool,
	initiator_enumerator_t *this, va_list args)
{
	configuration_attribute_type_t *type;
	attribute_handler_t **handler;
	chunk_t *value;

	VA_ARGS_VGET(args, handler, type, value);
	/* enumerate inner attributes using outer handler enumerator */
	while (!this->inner || !this->inner->enumerate(this->inner, type, value))
	{
		if (!this->outer->enumerate(this->outer, &this->handler))
		{
			return FALSE;
		}
		DESTROY_IF(this->inner);
		this->inner = this->handler->create_attribute_enumerator(this->handler,
													this->ike_sa, this->vips);
	}
	/* inject the handler as additional attribute */
	*handler = this->handler;
	return TRUE;
}

METHOD(enumerator_t, initiator_destroy, void,
	initiator_enumerator_t *this)
{
	this->this->lock->unlock(this->this->lock);
	this->outer->destroy(this->outer);
	DESTROY_IF(this->inner);
	free(this);
}

METHOD(attribute_manager_t, create_initiator_enumerator, enumerator_t*,
	private_attribute_manager_t *this, ike_sa_t *ike_sa, linked_list_t *vips)
{
	initiator_enumerator_t *enumerator;

	this->lock->read_lock(this->lock);

	INIT(enumerator,
		.public = {
			.enumerate = enumerator_enumerate_default,
			.venumerate = _initiator_enumerate,
			.destroy = _initiator_destroy,
		},
		.this = this,
		.ike_sa = ike_sa,
		.vips = vips,
		.outer = this->handlers->create_enumerator(this->handlers),
	);
	return &enumerator->public;
}

METHOD(attribute_manager_t, add_handler, void,
	private_attribute_manager_t *this, attribute_handler_t *handler)
{
	this->lock->write_lock(this->lock);
	this->handlers->insert_last(this->handlers, handler);
	this->lock->unlock(this->lock);
}

METHOD(attribute_manager_t, remove_handler, void,
	private_attribute_manager_t *this, attribute_handler_t *handler)
{
	this->lock->write_lock(this->lock);
	this->handlers->remove(this->handlers, handler, NULL);
	this->lock->unlock(this->lock);
}

METHOD(attribute_manager_t, destroy, void,
	private_attribute_manager_t *this)
{
	this->providers->destroy(this->providers);
	this->handlers->destroy(this->handlers);
	this->lock->destroy(this->lock);
	free(this);
}

/*
 * see header file
 */
attribute_manager_t *attribute_manager_create()
{
	private_attribute_manager_t *this;

	INIT(this,
		.public = {
			.acquire_address = _acquire_address,
			.release_address = _release_address,
			.create_responder_enumerator = _create_responder_enumerator,
			.add_provider = _add_provider,
			.remove_provider = _remove_provider,
			.handle = _handle,
			.release = _release,
			.create_initiator_enumerator = _create_initiator_enumerator,
			.add_handler = _add_handler,
			.remove_handler = _remove_handler,
			.destroy = _destroy,
		},
		.providers = linked_list_create(),
		.handlers = linked_list_create(),
		.lock = rwlock_create(RWLOCK_TYPE_DEFAULT),
	);

	return &this->public;
}
