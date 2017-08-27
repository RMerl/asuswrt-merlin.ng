/*
 * Copyright (C) 2008 Andreas Steffen
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

#include "migrate_job.h"

#include <daemon.h>

#include <config/child_cfg.h>


typedef struct private_migrate_job_t private_migrate_job_t;

/**
 * Private data of a migrate_job_t object.
 */
struct private_migrate_job_t {
	/**
	 * Public migrate_job_t interface.
	 */
	migrate_job_t public;

	/**
	 * reqid of the CHILD_SA if it already exists
	 */
	u_int32_t reqid;

	/**
	 * source traffic selector
	 */
	traffic_selector_t *src_ts;

	/**
	 * destination traffic selector
	 */
	traffic_selector_t *dst_ts;

	/**
	 * local host address to be used for IKE
	 */
	host_t *local;

	/**
	 * remote host address to be used for IKE
	 */
	host_t *remote;
};

METHOD(job_t, destroy, void,
	private_migrate_job_t *this)
{
	DESTROY_IF(this->src_ts);
	DESTROY_IF(this->dst_ts);
	DESTROY_IF(this->local);
	DESTROY_IF(this->remote);
	free(this);
}

METHOD(job_t, execute, job_requeue_t,
	private_migrate_job_t *this)
{
	ike_sa_t *ike_sa = NULL;

	if (this->reqid)
	{
		ike_sa = charon->ike_sa_manager->checkout_by_id(charon->ike_sa_manager,
														this->reqid, TRUE);
	}
	if (ike_sa)
	{
		enumerator_t *children, *enumerator;
		child_sa_t *child_sa;
		host_t *host;
		linked_list_t *vips;

		children = ike_sa->create_child_sa_enumerator(ike_sa);
		while (children->enumerate(children, (void**)&child_sa))
		{
			if (child_sa->get_reqid(child_sa) == this->reqid)
			{
				break;
			}
		}
		children->destroy(children);
		DBG2(DBG_JOB, "found CHILD_SA with reqid {%d}", this->reqid);

		ike_sa->set_kmaddress(ike_sa, this->local, this->remote);

		host = this->local->clone(this->local);
		host->set_port(host, charon->socket->get_port(charon->socket, FALSE));
		ike_sa->set_my_host(ike_sa, host);

		host = this->remote->clone(this->remote);
		host->set_port(host, IKEV2_UDP_PORT);
		ike_sa->set_other_host(ike_sa, host);

		vips = linked_list_create();
		enumerator = ike_sa->create_virtual_ip_enumerator(ike_sa, TRUE);
		while (enumerator->enumerate(enumerator, &host))
		{
			vips->insert_last(vips, host);
		}
		enumerator->destroy(enumerator);

		if (child_sa->update(child_sa, this->local, this->remote, vips,
				ike_sa->has_condition(ike_sa, COND_NAT_ANY)) == NOT_SUPPORTED)
		{
			ike_sa->rekey_child_sa(ike_sa, child_sa->get_protocol(child_sa),
								   child_sa->get_spi(child_sa, TRUE));
		}
		charon->ike_sa_manager->checkin(charon->ike_sa_manager, ike_sa);
		vips->destroy(vips);
	}
	else
	{
		DBG1(DBG_JOB, "no CHILD_SA found with reqid {%d}", this->reqid);
	}
	return JOB_REQUEUE_NONE;
}

METHOD(job_t, get_priority, job_priority_t,
	private_migrate_job_t *this)
{
	return JOB_PRIO_MEDIUM;
}

/*
 * Described in header
 */
migrate_job_t *migrate_job_create(u_int32_t reqid,
								  traffic_selector_t *src_ts,
								  traffic_selector_t *dst_ts,
								  policy_dir_t dir,
								  host_t *local, host_t *remote)
{
	private_migrate_job_t *this;

	INIT(this,
		.public = {
			.job_interface = {
				.execute = _execute,
				.get_priority = _get_priority,
				.destroy = _destroy,
			},
		},
		.reqid = reqid,
		.src_ts = (dir == POLICY_OUT) ? src_ts : dst_ts,
		.dst_ts = (dir == POLICY_OUT) ? dst_ts : src_ts,
		.local = local,
		.remote = remote,
	);

	return &this->public;
}
