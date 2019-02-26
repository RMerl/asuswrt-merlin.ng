/*
 * Copyright (C) 2012 Tobias Brunner
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

#include "eap_manager.h"

#include <collections/linked_list.h>
#include <threading/rwlock.h>

typedef struct private_eap_manager_t private_eap_manager_t;
typedef struct eap_entry_t eap_entry_t;

/**
 * EAP constructor entry
 */
struct eap_entry_t {

	/**
	 * EAP method type, vendor specific if vendor is set
	 */
	eap_type_t type;

	/**
	 * vendor ID, 0 for default EAP methods
	 */
	uint32_t vendor;

	/**
	 * Role of the method returned by the constructor, EAP_SERVER or EAP_PEER
	 */
	eap_role_t role;

	/**
	 * constructor function to create instance
	 */
	eap_constructor_t constructor;
};

/**
 * private data of eap_manager
 */
struct private_eap_manager_t {

	/**
	 * public functions
	 */
	eap_manager_t public;

	/**
	 * list of eap_entry_t's
	 */
	linked_list_t *methods;

	/**
	 * rwlock to lock methods
	 */
	rwlock_t *lock;
};

METHOD(eap_manager_t, add_method, void,
	private_eap_manager_t *this, eap_type_t type, uint32_t vendor,
	eap_role_t role, eap_constructor_t constructor)
{
	eap_entry_t *entry = malloc_thing(eap_entry_t);

	entry->type = type;
	entry->vendor = vendor;
	entry->role = role;
	entry->constructor = constructor;

	this->lock->write_lock(this->lock);
	this->methods->insert_last(this->methods, entry);
	this->lock->unlock(this->lock);
}

METHOD(eap_manager_t, remove_method, void,
	private_eap_manager_t *this, eap_constructor_t constructor)
{
	enumerator_t *enumerator;
	eap_entry_t *entry;

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

CALLBACK(filter_methods, bool,
	uintptr_t role, enumerator_t *orig, va_list args)
{
	eap_entry_t *entry;
	eap_type_t *type;
	uint32_t *vendor;

	VA_ARGS_VGET(args, type, vendor);

	while (orig->enumerate(orig, &entry))
	{
		if (entry->role != (eap_role_t)role)
		{
			continue;
		}
		if (entry->vendor == 0 &&
		   (entry->type < 4 || entry->type == EAP_EXPANDED ||
		    entry->type > EAP_EXPERIMENTAL))
		{	/* filter invalid types */
			continue;
		}
		if (type)
		{
			*type = entry->type;
		}
		if (vendor)
		{
			*vendor = entry->vendor;
		}
		return TRUE;
	}
	return FALSE;
}

METHOD(eap_manager_t, create_enumerator, enumerator_t*,
	private_eap_manager_t *this, eap_role_t role)
{
	this->lock->read_lock(this->lock);
	return enumerator_create_cleaner(
				enumerator_create_filter(
					this->methods->create_enumerator(this->methods),
					filter_methods, (void*)(uintptr_t)role, NULL),
				(void*)this->lock->unlock, this->lock);
}

METHOD(eap_manager_t, create_instance, eap_method_t*,
	private_eap_manager_t *this, eap_type_t type, uint32_t vendor,
	eap_role_t role, identification_t *server, identification_t *peer)
{
	enumerator_t *enumerator;
	eap_entry_t *entry;
	eap_method_t *method = NULL;

	this->lock->read_lock(this->lock);
	enumerator = this->methods->create_enumerator(this->methods);
	while (enumerator->enumerate(enumerator, &entry))
	{
		if (type == entry->type && vendor == entry->vendor &&
			role == entry->role)
		{
			method = entry->constructor(server, peer);
			if (method)
			{
				break;
			}
		}
	}
	enumerator->destroy(enumerator);
	this->lock->unlock(this->lock);
	return method;
}

METHOD(eap_manager_t, destroy, void,
	private_eap_manager_t *this)
{
	this->methods->destroy_function(this->methods, free);
	this->lock->destroy(this->lock);
	free(this);
}

/*
 * See header
 */
eap_manager_t *eap_manager_create()
{
	private_eap_manager_t *this;

	INIT(this,
			.public = {
				.add_method = _add_method,
				.remove_method = _remove_method,
				.create_enumerator = _create_enumerator,
				.create_instance = _create_instance,
				.destroy = _destroy,
			},
			.methods = linked_list_create(),
			.lock = rwlock_create(RWLOCK_TYPE_DEFAULT),
	);

	return &this->public;
}
