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

#include "medcli_listener.h"

#include <daemon.h>
#include <library.h>

typedef struct private_medcli_listener_t private_medcli_listener_t;
typedef enum mediated_state_t mediated_state_t;

/**
 * state of a mediated connection
 */
enum mediated_state_t {
	STATE_DOWN = 1,
	STATE_CONNECTING = 2,
	STATE_UP = 3,
};

/**
 * Private data of an medcli_listener_t object
 */
struct private_medcli_listener_t {

	/**
	 * Public part
	 */
	medcli_listener_t public;

	/**
	 * underlying database handle
	 */
	database_t *db;
};

/**
 * Update connection status in the database
 */
static void set_state(private_medcli_listener_t *this, char *alias,
					  mediated_state_t state)
{
	this->db->execute(this->db, NULL,
					  "UPDATE Connection SET Status = ? WHERE Alias = ?",
					  DB_UINT, state, DB_TEXT, alias);
}

METHOD(listener_t, ike_state_change, bool,
	private_medcli_listener_t *this, ike_sa_t *ike_sa, ike_sa_state_t state)
{
	if (ike_sa)
	{
		switch (state)
		{
			case IKE_CONNECTING:
				set_state(this, ike_sa->get_name(ike_sa), STATE_CONNECTING);
				break;
			case IKE_DESTROYING:
				set_state(this, ike_sa->get_name(ike_sa), STATE_DOWN);
			default:
				break;
		}
	}
	return TRUE;
}

METHOD(listener_t, child_state_change, bool,
	private_medcli_listener_t *this, ike_sa_t *ike_sa, child_sa_t *child_sa,
	child_sa_state_t state)
{
	if (ike_sa && child_sa)
	{
		switch (state)
		{
			case CHILD_INSTALLED:
				set_state(this, child_sa->get_name(child_sa), STATE_UP);
				break;
			case CHILD_DESTROYING:
				set_state(this, child_sa->get_name(child_sa), STATE_DOWN);
				break;
			default:
				break;
		}
	}
	return TRUE;
}

METHOD(medcli_listener_t, destroy, void,
	private_medcli_listener_t *this)
{
	this->db->execute(this->db, NULL, "UPDATE Connection SET Status = ?",
					  DB_UINT, STATE_DOWN);
	free(this);
}

/**
 * Described in header.
 */
medcli_listener_t *medcli_listener_create(database_t *db)
{
	private_medcli_listener_t *this;

	INIT(this,
		.public = {
			.listener = {
				.ike_state_change = _ike_state_change,
				.child_state_change = _child_state_change,
			},
			.destroy = _destroy,
		},
		.db = db,
	);

	db->execute(db, NULL, "UPDATE Connection SET Status = ?",
				DB_UINT, STATE_DOWN);

	return &this->public;
}

