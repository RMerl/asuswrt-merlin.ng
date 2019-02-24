/*
 * Copyright (C) 2017 Tobias Brunner
 * Copyright (C) 2012-2014 Reto Buerki
 * Copyright (C) 2012 Adrian-Ken Rueegsegger
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

#include <errno.h>
#include <netinet/udp.h>
#include <linux/xfrm.h>
#include <utils/debug.h>
#include <utils/chunk.h>
#include <tkm/constants.h>
#include <tkm/client.h>

#include "tkm.h"
#include "tkm_utils.h"
#include "tkm_types.h"
#include "tkm_keymat.h"
#include "tkm_kernel_ipsec.h"

/** From linux/in.h */
#ifndef IP_XFRM_POLICY
#define IP_XFRM_POLICY 17
#endif

typedef struct private_tkm_kernel_ipsec_t private_tkm_kernel_ipsec_t;

/**
 * Private variables and functions of TKM kernel ipsec instance.
 */
struct private_tkm_kernel_ipsec_t {

	/**
	 * Public tkm_kernel_ipsec interface.
	 */
	tkm_kernel_ipsec_t public;

	/**
	 * RNG used for SPI generation.
	 */
	rng_t *rng;

};

METHOD(kernel_ipsec_t, get_features, kernel_feature_t,
	private_tkm_kernel_ipsec_t *this)
{
	return KERNEL_POLICY_SPI;
}

METHOD(kernel_ipsec_t, get_spi, status_t,
	private_tkm_kernel_ipsec_t *this, host_t *src, host_t *dst,
	uint8_t protocol, uint32_t *spi)
{
	bool result;

	if (!this->rng)
	{
		this->rng = lib->crypto->create_rng(lib->crypto, RNG_WEAK);
		if (!this->rng)
		{
			DBG1(DBG_KNL, "unable to create RNG");
			return FAILED;
		}
	}

	result = this->rng->get_bytes(this->rng, sizeof(uint32_t),
								  (uint8_t *)spi);
	return result ? SUCCESS : FAILED;
}

METHOD(kernel_ipsec_t, get_cpi, status_t,
	private_tkm_kernel_ipsec_t *this, host_t *src, host_t *dst,
	uint16_t *cpi)
{
	return NOT_SUPPORTED;
}

METHOD(kernel_ipsec_t, add_sa, status_t,
	private_tkm_kernel_ipsec_t *this, kernel_ipsec_sa_id_t *id,
	kernel_ipsec_add_sa_t *data)
{
	esa_info_t esa;
	esp_spi_type spi_loc, spi_rem;
	host_t *local, *peer;
	chunk_t *nonce_loc, *nonce_rem;
	nc_id_type nonce_loc_id;
	esa_id_type esa_id;
	nonce_type nc_rem;

	if (data->enc_key.ptr == NULL)
	{
		DBG1(DBG_KNL, "Unable to get ESA information");
		return FAILED;
	}
	esa = *(esa_info_t *)(data->enc_key.ptr);

	/* only handle the case where we have both distinct ESP spi's available */
	if (esa.spi_r == id->spi)
	{
		chunk_free(&esa.nonce_i);
		chunk_free(&esa.nonce_r);
		return SUCCESS;
	}

	if (data->initiator)
	{
		spi_loc = id->spi;
		spi_rem = esa.spi_r;
		local = id->dst;
		peer = id->src;
		nonce_loc = &esa.nonce_i;
		nonce_rem = &esa.nonce_r;
	}
	else
	{
		spi_loc = esa.spi_r;
		spi_rem = id->spi;
		local = id->src;
		peer = id->dst;
		nonce_loc = &esa.nonce_r;
		nonce_rem = &esa.nonce_i;
	}

	esa_id = tkm->idmgr->acquire_id(tkm->idmgr, TKM_CTX_ESA);
	if (esa_id == 0)
	{
		DBG1(DBG_KNL, "unable to acquire esa context id");
		goto esa_id_failure;
	}

	if (!tkm->sad->insert(tkm->sad, esa_id, data->reqid, local, peer,
						  spi_loc, spi_rem, id->proto))
	{
		DBG1(DBG_KNL, "unable to add entry (%llu) to SAD", esa_id);
		goto sad_failure;
	}

	/*
	 * creation of first CHILD SA:
	 * no nonce and no dh contexts because the ones from the IKE SA are re-used
	 */
	nonce_loc_id = tkm->chunk_map->get_id(tkm->chunk_map, nonce_loc);
	if (nonce_loc_id == 0 && esa.dh_id == 0)
	{
		if (ike_esa_create_first(esa_id, esa.isa_id, data->reqid, 1, spi_loc,
								 spi_rem) != TKM_OK)
		{
			DBG1(DBG_KNL, "child SA (%llu, first) creation failed", esa_id);
			goto failure;
		}
	}
	/* creation of child SA without PFS: no dh context */
	else if (nonce_loc_id != 0 && esa.dh_id == 0)
	{
		chunk_to_sequence(nonce_rem, &nc_rem, sizeof(nonce_type));
		if (ike_esa_create_no_pfs(esa_id, esa.isa_id, data->reqid, 1,
								  nonce_loc_id, nc_rem, data->initiator,
								  spi_loc, spi_rem) != TKM_OK)
		{
			DBG1(DBG_KNL, "child SA (%llu, no PFS) creation failed", esa_id);
			goto failure;
		}
		tkm->chunk_map->remove(tkm->chunk_map, nonce_loc);
		tkm->idmgr->release_id(tkm->idmgr, TKM_CTX_NONCE, nonce_loc_id);
	}
	/* creation of subsequent child SA with PFS: nonce and dh context are set */
	else
	{
		chunk_to_sequence(nonce_rem, &nc_rem, sizeof(nonce_type));
		if (ike_esa_create(esa_id, esa.isa_id, data->reqid, 1, esa.dh_id,
						   nonce_loc_id, nc_rem, data->initiator, spi_loc,
						   spi_rem) != TKM_OK)
		{
			DBG1(DBG_KNL, "child SA (%llu) creation failed", esa_id);
			goto failure;
		}
		tkm->chunk_map->remove(tkm->chunk_map, nonce_loc);
		tkm->idmgr->release_id(tkm->idmgr, TKM_CTX_NONCE, nonce_loc_id);
	}

	DBG1(DBG_KNL, "added child SA (esa: %llu, isa: %llu, esp_spi_loc: %x, "
		 "esp_spi_rem: %x, role: %s)", esa_id, esa.isa_id, ntohl(spi_loc),
		 ntohl(spi_rem), data->initiator ? "initiator" : "responder");
	chunk_free(&esa.nonce_i);
	chunk_free(&esa.nonce_r);

	return SUCCESS;

failure:
	ike_esa_reset(esa_id);
	tkm->sad->remove(tkm->sad, esa_id);
sad_failure:
	tkm->idmgr->release_id(tkm->idmgr, TKM_CTX_ESA, esa_id);
esa_id_failure:
	chunk_free(&esa.nonce_i);
	chunk_free(&esa.nonce_r);
	return FAILED;
}

METHOD(kernel_ipsec_t, query_sa, status_t,
	private_tkm_kernel_ipsec_t *this, kernel_ipsec_sa_id_t *id,
	kernel_ipsec_query_sa_t *data, uint64_t *bytes, uint64_t *packets,
	time_t *time)
{
	return NOT_SUPPORTED;
}

METHOD(kernel_ipsec_t, del_sa, status_t,
	private_tkm_kernel_ipsec_t *this, kernel_ipsec_sa_id_t *id,
	kernel_ipsec_del_sa_t *data)
{
	esa_id_type esa_id;

	esa_id = tkm->sad->get_esa_id(tkm->sad, id->src, id->dst,
								  id->spi, id->proto, TRUE);
	if (esa_id)
	{
		DBG1(DBG_KNL, "deleting child SA (esa: %llu, spi: %x)", esa_id,
			 ntohl(id->spi));
		if (ike_esa_reset(esa_id) != TKM_OK)
		{
			DBG1(DBG_KNL, "child SA (%llu) deletion failed", esa_id);
			return FAILED;
		}
		tkm->sad->remove(tkm->sad, esa_id);
		tkm->idmgr->release_id(tkm->idmgr, TKM_CTX_ESA, esa_id);
	}
	return SUCCESS;
}

METHOD(kernel_ipsec_t, update_sa, status_t,
	private_tkm_kernel_ipsec_t *this, kernel_ipsec_sa_id_t *id,
	kernel_ipsec_update_sa_t *data)
{
	return NOT_SUPPORTED;
}

METHOD(kernel_ipsec_t, flush_sas, status_t,
	private_tkm_kernel_ipsec_t *this)
{
	DBG1(DBG_KNL, "flushing child SA entries");
	return SUCCESS;
}

METHOD(kernel_ipsec_t, add_policy, status_t,
	private_tkm_kernel_ipsec_t *this, kernel_ipsec_policy_id_t *id,
	kernel_ipsec_manage_policy_t *data)
{
	esa_id_type esa_id;
	uint32_t spi;
	uint8_t proto;

	if (id->dir == POLICY_OUT && data->type == POLICY_IPSEC &&
		data->prio == POLICY_PRIORITY_DEFAULT)
	{
		if (data->sa->esp.use)
		{
			spi = data->sa->esp.spi;
			proto = IPPROTO_ESP;
		}
		else if (data->sa->ah.use)
		{
			spi = data->sa->ah.spi;
			proto = IPPROTO_AH;
		}
		else
		{
			return FAILED;
		}
		esa_id = tkm->sad->get_esa_id(tkm->sad, data->src, data->dst,
									  spi, proto, FALSE);
		if (!esa_id)
		{
			DBG1(DBG_KNL, "unable to find esa ID for policy (spi: %x)",
				 ntohl(spi));
			return FAILED;
		}
		DBG1(DBG_KNL, "selecting child SA (esa: %llu, spi: %x)", esa_id,
			 ntohl(spi));
		if (ike_esa_select(esa_id) != TKM_OK)
		{
			DBG1(DBG_KNL, "error selecting new child SA (%llu)", esa_id);
			return FAILED;
		}
	}
	return SUCCESS;
}

METHOD(kernel_ipsec_t, query_policy, status_t,
	private_tkm_kernel_ipsec_t *this, kernel_ipsec_policy_id_t *id,
	kernel_ipsec_query_policy_t *data, time_t *use_time)
{
	return NOT_SUPPORTED;
}

METHOD(kernel_ipsec_t, del_policy, status_t,
	private_tkm_kernel_ipsec_t *this, kernel_ipsec_policy_id_t *id,
	kernel_ipsec_manage_policy_t *data)
{
	return SUCCESS;
}

METHOD(kernel_ipsec_t, flush_policies, status_t,
	private_tkm_kernel_ipsec_t *this)
{
	return SUCCESS;
}


METHOD(kernel_ipsec_t, bypass_socket, bool,
	private_tkm_kernel_ipsec_t *this, int fd, int family)
{
	struct xfrm_userpolicy_info policy;
	u_int sol, ipsec_policy;

	switch (family)
	{
		case AF_INET:
			sol = SOL_IP;
			ipsec_policy = IP_XFRM_POLICY;
			break;
		case AF_INET6:
			sol = SOL_IPV6;
			ipsec_policy = IPV6_XFRM_POLICY;
			break;
		default:
			return FALSE;
	}

	memset(&policy, 0, sizeof(policy));
	policy.action = XFRM_POLICY_ALLOW;
	policy.sel.family = family;

	policy.dir = XFRM_POLICY_OUT;
	if (setsockopt(fd, sol, ipsec_policy, &policy, sizeof(policy)) < 0)
	{
		DBG1(DBG_KNL, "unable to set IPSEC_POLICY on socket: %s",
			 strerror(errno));
		return FALSE;
	}
	policy.dir = XFRM_POLICY_IN;
	if (setsockopt(fd, sol, ipsec_policy, &policy, sizeof(policy)) < 0)
	{
		DBG1(DBG_KNL, "unable to set IPSEC_POLICY on socket: %s",
			 strerror(errno));
		return FALSE;
	}
	return TRUE;
}

METHOD(kernel_ipsec_t, enable_udp_decap, bool,
	private_tkm_kernel_ipsec_t *this, int fd, int family, uint16_t port)
{
	int type = UDP_ENCAP_ESPINUDP;

	if (setsockopt(fd, SOL_UDP, UDP_ENCAP, &type, sizeof(type)) < 0)
	{
		DBG1(DBG_KNL, "unable to set UDP_ENCAP: %s", strerror(errno));
		return FALSE;
	}
	return TRUE;
}

METHOD(kernel_ipsec_t, destroy, void,
	private_tkm_kernel_ipsec_t *this)
{
	DESTROY_IF(this->rng);
	free(this);
}

/*
 * Described in header.
 */
tkm_kernel_ipsec_t *tkm_kernel_ipsec_create()
{
	private_tkm_kernel_ipsec_t *this;

	INIT(this,
		.public = {
			.interface = {
				.get_features = _get_features,
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
	);

	return &this->public;
}
