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

#include "load_tester_ipsec.h"

#include <time.h>

typedef struct private_load_tester_ipsec_t private_load_tester_ipsec_t;

/**
 * Private variables and functions of kernel_pfkey class.
 */
struct private_load_tester_ipsec_t {
	/**
	 * Public interface.
	 */
	load_tester_ipsec_t public;

	/**
	 * faked SPI counter
	 */
	refcount_t spi;
};

METHOD(kernel_ipsec_t, get_spi, status_t,
	private_load_tester_ipsec_t *this, host_t *src, host_t *dst,
	uint8_t protocol, uint32_t *spi)
{
	*spi = (uint32_t)ref_get(&this->spi);
	return SUCCESS;
}

METHOD(kernel_ipsec_t, get_cpi, status_t,
	private_load_tester_ipsec_t *this, host_t *src, host_t *dst,
	uint16_t *cpi)
{
	return FAILED;
}

METHOD(kernel_ipsec_t, add_sa, status_t,
	private_load_tester_ipsec_t *this, kernel_ipsec_sa_id_t *id,
	kernel_ipsec_add_sa_t *data)
{
	return SUCCESS;
}

METHOD(kernel_ipsec_t, update_sa, status_t,
	private_load_tester_ipsec_t *this, kernel_ipsec_sa_id_t *id,
	kernel_ipsec_update_sa_t *data)
{
	return SUCCESS;
}

METHOD(kernel_ipsec_t, query_sa, status_t,
	private_load_tester_ipsec_t *this, kernel_ipsec_sa_id_t *id,
	kernel_ipsec_query_sa_t *data, uint64_t *bytes, uint64_t *packets,
	time_t *time)
{
	return NOT_SUPPORTED;
}

METHOD(kernel_ipsec_t, del_sa, status_t,
	private_load_tester_ipsec_t *this, kernel_ipsec_sa_id_t *id,
	kernel_ipsec_del_sa_t *data)
{
	return SUCCESS;
}

METHOD(kernel_ipsec_t, add_policy, status_t,
	private_load_tester_ipsec_t *this, kernel_ipsec_policy_id_t *id,
	kernel_ipsec_manage_policy_t *data)
{
	return SUCCESS;
}

METHOD(kernel_ipsec_t, query_policy, status_t,
	private_load_tester_ipsec_t *this, kernel_ipsec_policy_id_t *id,
	kernel_ipsec_query_policy_t *data, time_t *use_time)
{
	*use_time = 1;
	return SUCCESS;
}

METHOD(kernel_ipsec_t, del_policy, status_t,
	private_load_tester_ipsec_t *this, kernel_ipsec_policy_id_t *id,
	kernel_ipsec_manage_policy_t *data)
{
	return SUCCESS;
}

METHOD(kernel_ipsec_t, destroy, void,
	private_load_tester_ipsec_t *this)
{
	free(this);
}

/*
 * Described in header.
 */
load_tester_ipsec_t *load_tester_ipsec_create()
{
	private_load_tester_ipsec_t *this;

	INIT(this,
		.public = {
			.interface = {
				.get_spi = _get_spi,
				.get_cpi = _get_cpi,
				.add_sa = _add_sa,
				.update_sa = _update_sa,
				.query_sa = _query_sa,
				.del_sa = _del_sa,
				.flush_sas = (void*)return_failed,
				.add_policy = _add_policy,
				.query_policy = _query_policy,
				.del_policy = _del_policy,
				.flush_policies = (void*)return_failed,
				.bypass_socket = (void*)return_true,
				.enable_udp_decap = (void*)return_true,
				.destroy = _destroy,
			},
		},
		.spi = 0,
	);

	return &this->public;
}
