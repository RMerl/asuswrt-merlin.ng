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

#include "load_tester_listener.h"

#include <signal.h>

#include <daemon.h>
#include <processing/jobs/delete_ike_sa_job.h>

typedef struct private_load_tester_listener_t private_load_tester_listener_t;

/**
 * Private data of an load_tester_listener_t object
 */
struct private_load_tester_listener_t {
	/**
	 * Public part
	 */
	load_tester_listener_t public;

	/**
	 * Delete IKE_SA after it has been established
	 */
	bool delete_after_established;

	/**
	 * Number of established SAs
	 */
	u_int established;

	/**
	 * Number of terminated SAs
	 */
	u_int terminated;

	/**
	 * Shutdown the daemon if we have established this SA count
	 */
	u_int shutdown_on;

	/**
	 * Configuration backend
	 */
	load_tester_config_t *config;
};

METHOD(listener_t, ike_updown, bool,
	private_load_tester_listener_t *this, ike_sa_t *ike_sa, bool up)
{
	if (up)
	{
		ike_sa_id_t *id = ike_sa->get_id(ike_sa);

		this->established++;

		if (this->delete_after_established)
		{
			lib->processor->queue_job(lib->processor,
									(job_t*)delete_ike_sa_job_create(id, TRUE));
		}

		if (id->is_initiator(id))
		{
			if (this->shutdown_on == this->established)
			{
				DBG1(DBG_CFG, "load-test complete, raising SIGTERM");
				kill(0, SIGTERM);
			}
		}
	}
	else
	{
		this->terminated++;
	}
	return TRUE;
}

METHOD(listener_t, ike_state_change, bool,
	private_load_tester_listener_t *this, ike_sa_t *ike_sa, ike_sa_state_t state)
{
	if (state == IKE_DESTROYING)
	{
		this->config->delete_ip(this->config, ike_sa->get_my_host(ike_sa));
	}
	return TRUE;
}

METHOD(load_tester_listener_t, get_established, u_int,
	private_load_tester_listener_t *this)
{
	return this->established - this->terminated;
}

METHOD(load_tester_listener_t, destroy, void,
	private_load_tester_listener_t *this)
{
	free(this);
}

load_tester_listener_t *load_tester_listener_create(u_int shutdown_on,
													load_tester_config_t *config)
{
	private_load_tester_listener_t *this;

	INIT(this,
		.public = {
			.listener = {
				.ike_updown = _ike_updown,
				.ike_state_change = _ike_state_change,
			},
			.get_established = _get_established,
			.destroy = _destroy,
		},
		.delete_after_established = lib->settings->get_bool(lib->settings,
					"%s.plugins.load-tester.delete_after_established", FALSE,
					lib->ns),
		.shutdown_on = shutdown_on,
		.config = config,
	);

	return &this->public;
}
