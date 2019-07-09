/*
 * Copyright (C) 2012 Tobias Brunner
 * Copyright (C) 2012 Giuliano Grassi
 * Copyright (C) 2012 Ralf Sager
 * HSR Hochschule fuer Technik Rapperswil
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.  *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

#include "android_ipsec.h"
#include "../charonservice.h"

#include <utils/debug.h>
#include <library.h>
#include <daemon.h>
#include <ipsec.h>

typedef struct private_kernel_android_ipsec_t private_kernel_android_ipsec_t;

struct private_kernel_android_ipsec_t {

	/**
	 * Public kernel interface
	 */
	kernel_android_ipsec_t public;

	/**
	 * Listener for lifetime expire events
	 */
	ipsec_event_listener_t ipsec_listener;
};

/**
 * Callback registrered with libipsec.
 */
static void expire(uint8_t protocol, uint32_t spi, host_t *dst, bool hard)
{
	charon->kernel->expire(charon->kernel, protocol, spi, dst, hard);
}

METHOD(kernel_ipsec_t, get_spi, status_t,
	private_kernel_android_ipsec_t *this, host_t *src, host_t *dst,
	uint8_t protocol, uint32_t *spi)
{
	return ipsec->sas->get_spi(ipsec->sas, src, dst, protocol, spi);
}

METHOD(kernel_ipsec_t, get_cpi, status_t,
	private_kernel_android_ipsec_t *this, host_t *src, host_t *dst,
	uint16_t *cpi)
{
	return NOT_SUPPORTED;
}

METHOD(kernel_ipsec_t, add_sa, status_t,
	private_kernel_android_ipsec_t *this, kernel_ipsec_sa_id_t *id,
	kernel_ipsec_add_sa_t *data)
{
	return ipsec->sas->add_sa(ipsec->sas, id->src, id->dst, id->spi, id->proto,
					data->reqid, id->mark, data->tfc, data->lifetime,
					data->enc_alg, data->enc_key, data->int_alg, data->int_key,
					data->mode, data->ipcomp, data->cpi, data->initiator,
					data->encap, data->esn, data->inbound, data->update);
}

METHOD(kernel_ipsec_t, update_sa, status_t,
	private_kernel_android_ipsec_t *this, kernel_ipsec_sa_id_t *id,
	kernel_ipsec_update_sa_t *data)
{
	return ipsec->sas->update_sa(ipsec->sas, id->spi, id->proto, data->cpi,
					id->src, id->dst, data->new_src, data->new_dst, data->encap,
					data->new_encap, id->mark);
}

METHOD(kernel_ipsec_t, query_sa, status_t,
	private_kernel_android_ipsec_t *this, kernel_ipsec_sa_id_t *id,
	kernel_ipsec_query_sa_t *data, uint64_t *bytes, uint64_t *packets,
	time_t *time)
{
	return ipsec->sas->query_sa(ipsec->sas, id->src, id->dst, id->spi,
								id->proto, id->mark, bytes, packets, time);
}

METHOD(kernel_ipsec_t, del_sa, status_t,
	private_kernel_android_ipsec_t *this, kernel_ipsec_sa_id_t *id,
	kernel_ipsec_del_sa_t *data)
{
	return ipsec->sas->del_sa(ipsec->sas, id->src, id->dst, id->spi, id->proto,
							  data->cpi, id->mark);
}

METHOD(kernel_ipsec_t, flush_sas, status_t,
	private_kernel_android_ipsec_t *this)
{
	return ipsec->sas->flush_sas(ipsec->sas);
}

METHOD(kernel_ipsec_t, add_policy, status_t,
	private_kernel_android_ipsec_t *this, kernel_ipsec_policy_id_t *id,
	kernel_ipsec_manage_policy_t *data)
{
	return ipsec->policies->add_policy(ipsec->policies, data->src, data->dst,
									   id->src_ts, id->dst_ts, id->dir,
									   data->type, data->sa, id->mark,
									   data->prio);
}

METHOD(kernel_ipsec_t, query_policy, status_t,
	private_kernel_android_ipsec_t *this, kernel_ipsec_policy_id_t *id,
	kernel_ipsec_query_policy_t *data, time_t *use_time)
{
	return NOT_SUPPORTED;
}

METHOD(kernel_ipsec_t, del_policy, status_t,
	private_kernel_android_ipsec_t *this, kernel_ipsec_policy_id_t *id,
	kernel_ipsec_manage_policy_t *data)
{
	return ipsec->policies->del_policy(ipsec->policies, data->src, data->dst,
									   id->src_ts, id->dst_ts, id->dir,
									   data->type, data->sa, id->mark,
									   data->prio);
}

METHOD(kernel_ipsec_t, flush_policies, status_t,
	private_kernel_android_ipsec_t *this)
{
	ipsec->policies->flush_policies(ipsec->policies);
	return SUCCESS;
}

METHOD(kernel_ipsec_t, bypass_socket, bool,
	private_kernel_android_ipsec_t *this, int fd, int family)
{
	return charonservice->bypass_socket(charonservice, fd, family);
}

METHOD(kernel_ipsec_t, enable_udp_decap, bool,
	private_kernel_android_ipsec_t *this, int fd, int family, uint16_t port)
{
	return NOT_SUPPORTED;
}

METHOD(kernel_ipsec_t, destroy, void,
	private_kernel_android_ipsec_t *this)
{
	ipsec->events->unregister_listener(ipsec->events, &this->ipsec_listener);
	free(this);
}

/*
 * Described in header.
 */
kernel_android_ipsec_t *kernel_android_ipsec_create()
{
	private_kernel_android_ipsec_t *this;

	INIT(this,
		.public = {
			.interface = {
				.get_spi = _get_spi,
				.get_cpi = _get_cpi,
				.add_sa  = _add_sa,
				.update_sa = _update_sa,
				.query_sa = _query_sa,
				.del_sa = _del_sa,
				.flush_sas = _flush_sas,
				.add_policy = _add_policy,
				.query_policy = _query_policy,
				.del_policy = _del_policy,
				.flush_policies = _flush_policies,
				.bypass_socket = _bypass_socket,
				.enable_udp_decap = _enable_udp_decap,
				.destroy = _destroy,
			},
		},
		.ipsec_listener = {
			.expire = expire,
		},
	);

	ipsec->events->register_listener(ipsec->events, &this->ipsec_listener);

	return &this->public;
}
