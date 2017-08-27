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

#include "manager.h"

#include "gateway.h"

#include <collections/linked_list.h>

typedef struct private_manager_t private_manager_t;

/**
 * private data of manager
 */
struct private_manager_t {

	/**
	 * public functions
	 */
	manager_t public;

	/**
	 * underlying storage database
	 */
	storage_t *store;

	/**
	 * user id, if we are logged in
	 */
	int user;

	/**
	 * selected gateway
	 */
	gateway_t *gateway;
};

METHOD(manager_t, create_gateway_enumerator, enumerator_t*,
	private_manager_t *this)
{
	return this->store->create_gateway_enumerator(this->store, this->user);
}

METHOD(manager_t, select_gateway, gateway_t*,
	private_manager_t *this, int select_id)
{
	if (select_id != 0)
	{
		enumerator_t *enumerator;
		int id, port;
		char *name, *address;
		host_t *host;

		if (this->gateway) this->gateway->destroy(this->gateway);
		this->gateway = NULL;

		enumerator = this->store->create_gateway_enumerator(this->store, this->user);
		while (enumerator->enumerate(enumerator, &id, &name, &port, &address))
		{
			if (select_id == id)
			{
				if (port == 0)
				{
					this->gateway = gateway_create_unix(name);
				}
				else
				{
					host = host_create_from_string(address, port);
					if (host)
					{
						this->gateway = gateway_create_tcp(name, host);
					}
				}
				break;
			}
		}
		enumerator->destroy(enumerator);
	}
	return this->gateway;
}

METHOD(manager_t, logged_in, bool,
	private_manager_t *this)
{
	return this->user != 0;
}

METHOD(manager_t, login, bool,
	private_manager_t *this, char *username, char *password)
{
	if (!this->user)
	{
		this->user = this->store->login(this->store, username, password);
	}
	return this->user != 0;
}

METHOD(manager_t, logout, void,
	private_manager_t *this)
{
	if (this->gateway)
	{
		this->gateway->destroy(this->gateway);
		this->gateway = NULL;
	}
	this->user = 0;
}

METHOD(fast_context_t, destroy, void,
	private_manager_t *this)
{
	if (this->gateway) this->gateway->destroy(this->gateway);
	free(this);
}

/*
 * see header file
 */
manager_t *manager_create(storage_t *storage)
{
	private_manager_t *this;

	INIT(this,
		.public = {
			.login = _login,
			.logged_in = _logged_in,
			.logout = _logout,
			.create_gateway_enumerator = _create_gateway_enumerator,
			.select_gateway = _select_gateway,
			.context = {
				.destroy = _destroy,
			},
		},
		.store = storage,
	);

	return &this->public;
}
