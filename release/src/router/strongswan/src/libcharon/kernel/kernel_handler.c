/*
 * Copyright (C) 2010 Tobias Brunner
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

#include "kernel_handler.h"

#include <daemon.h>
#include <processing/jobs/acquire_job.h>
#include <processing/jobs/delete_child_sa_job.h>
#include <processing/jobs/migrate_job.h>
#include <processing/jobs/rekey_child_sa_job.h>
#include <processing/jobs/roam_job.h>
#include <processing/jobs/update_sa_job.h>

typedef struct private_kernel_handler_t private_kernel_handler_t;

/**
 * Private data of a kernel_handler_t object.
 */
struct private_kernel_handler_t {

	/**
	 * Public part of kernel_handler_t object.
	 */
	kernel_handler_t public;
};

/**
 * convert an IP protocol identifier to the IKEv2 specific protocol identifier.
 */
static inline protocol_id_t proto_ip2ike(uint8_t protocol)
{
	switch (protocol)
	{
		case IPPROTO_ESP:
			return PROTO_ESP;
		case IPPROTO_AH:
			return PROTO_AH;
		default:
			return protocol;
	}
}

METHOD(kernel_listener_t, acquire, bool,
	private_kernel_handler_t *this, uint32_t reqid,
	traffic_selector_t *src_ts, traffic_selector_t *dst_ts)
{
	if (src_ts && dst_ts)
	{
		DBG1(DBG_KNL, "creating acquire job for policy %R === %R with "
			 "reqid {%u}", src_ts, dst_ts, reqid);
	}
	else
	{
		DBG1(DBG_KNL, "creating acquire job for policy with reqid {%u}", reqid);
	}
	lib->processor->queue_job(lib->processor,
							(job_t*)acquire_job_create(reqid, src_ts, dst_ts));
	return TRUE;
}

METHOD(kernel_listener_t, expire, bool,
	private_kernel_handler_t *this, uint8_t protocol, uint32_t spi,
	host_t *dst, bool hard)
{
	protocol_id_t proto = proto_ip2ike(protocol);

	DBG1(DBG_KNL, "creating %s job for CHILD_SA %N/0x%08x/%H",
		 hard ? "delete" : "rekey", protocol_id_names, proto, ntohl(spi), dst);

	if (hard)
	{
		lib->processor->queue_job(lib->processor,
				(job_t*)delete_child_sa_job_create(proto, spi, dst, hard));
	}
	else
	{
		lib->processor->queue_job(lib->processor,
				(job_t*)rekey_child_sa_job_create(proto, spi, dst));
	}
	return TRUE;
}

METHOD(kernel_listener_t, mapping, bool,
	private_kernel_handler_t *this, uint8_t protocol, uint32_t spi,
	host_t *dst, host_t *remote)
{
	protocol_id_t proto = proto_ip2ike(protocol);

	DBG1(DBG_KNL, "NAT mappings of CHILD_SA %N/0x%08x/%H changed to %#H, "
		 "queuing update job", protocol_id_names, proto, ntohl(spi), dst,
		 remote);

	lib->processor->queue_job(lib->processor,
						(job_t*)update_sa_job_create(proto, spi, dst, remote));
	return TRUE;
}

METHOD(kernel_listener_t, migrate, bool,
	private_kernel_handler_t *this, uint32_t reqid,
	traffic_selector_t *src_ts, traffic_selector_t *dst_ts,
	policy_dir_t direction, host_t *local, host_t *remote)
{
	DBG1(DBG_KNL, "creating migrate job for policy %R === %R %N with reqid {%u}",
		 src_ts, dst_ts, policy_dir_names, direction, reqid, local);

	lib->processor->queue_job(lib->processor,
						(job_t*)migrate_job_create(reqid, src_ts, dst_ts,
												   direction, local, remote));
	return TRUE;
}

METHOD(kernel_listener_t, roam, bool,
	private_kernel_handler_t *this, bool address)
{
	DBG2(DBG_KNL, "creating roam job %s",
		 address ? "due to address/link change" : "due to route change");

	lib->processor->queue_job(lib->processor, (job_t*)roam_job_create(address));
	return TRUE;
}

METHOD(kernel_handler_t, destroy, void,
	private_kernel_handler_t *this)
{
	charon->kernel->remove_listener(charon->kernel, &this->public.listener);
	free(this);
}

kernel_handler_t *kernel_handler_create()
{
	private_kernel_handler_t *this;

	INIT(this,
		.public = {
			.listener = {
				.acquire = _acquire,
				.expire = _expire,
				.mapping = _mapping,
				.migrate = _migrate,
				.roam = _roam,
			},
			.destroy = _destroy,
		},
	);

	charon->kernel->add_listener(charon->kernel, &this->public.listener);

	return &this->public;
}
