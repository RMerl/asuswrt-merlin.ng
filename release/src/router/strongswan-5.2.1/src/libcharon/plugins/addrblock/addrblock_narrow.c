/*
 * Copyright (C) 2010 Martin Willi
 * Copyright (C) 2010 revosec AG
 * Copyright (C) 2009 Andreas Steffen
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

#include "addrblock_narrow.h"

#include <daemon.h>
#include <credentials/certificates/x509.h>

typedef struct private_addrblock_narrow_t private_addrblock_narrow_t;

/**
 * Private data of an addrblock_narrow_t object.
 */
struct private_addrblock_narrow_t {

	/**
	 * Public addrblock_narrow_t interface.
	 */
	addrblock_narrow_t public;
};

/**
 * Check if the negotiated TS list is acceptable by X509 ipAddrBlock constraints
 */
static bool check_constraints(ike_sa_t *ike_sa, linked_list_t *list)
{
	auth_cfg_t *auth;
	enumerator_t *auth_enum;
	certificate_t *cert = NULL;

	auth_enum = ike_sa->create_auth_cfg_enumerator(ike_sa, FALSE);
	while (auth_enum->enumerate(auth_enum, &auth))
	{
		cert = auth->get(auth, AUTH_HELPER_SUBJECT_CERT);
		if (cert)
		{
			break;
		}
	}
	auth_enum->destroy(auth_enum);

	if (cert && cert->get_type(cert) == CERT_X509)
	{
		x509_t *x509 = (x509_t*)cert;

		if (x509->get_flags(x509) & X509_IP_ADDR_BLOCKS)
		{
			enumerator_t *enumerator, *block_enum;
			traffic_selector_t *ts, *block_ts;

			DBG1(DBG_IKE, "checking certificate-based traffic selector "
						  "constraints [RFC 3779]");
			enumerator = list->create_enumerator(list);
			while (enumerator->enumerate(enumerator, &ts))
			{
				bool contained = FALSE;

				block_enum = x509->create_ipAddrBlock_enumerator(x509);
				while (block_enum->enumerate(block_enum, &block_ts))
				{
					if (ts->is_contained_in(ts, block_ts))
					{
						DBG1(DBG_IKE, "  TS %R is contained in address block"
									  " constraint %R", ts, block_ts);
						contained = TRUE;
						break;
					}
				}
				block_enum->destroy(block_enum);

				if (!contained)
				{
					DBG1(DBG_IKE, "  TS %R is not contained in any"
								  " address block constraint", ts);
					enumerator->destroy(enumerator);
					return FALSE;
				}
			}
			enumerator->destroy(enumerator);
		}
	}
	return TRUE;
}

/**
 * Delete all traffic selectors in a list
 */
static void flush_ts_list(linked_list_t *list)
{
	traffic_selector_t *ts;

	while (list->remove_last(list, (void**)&ts) == SUCCESS)
	{
		ts->destroy(ts);
	}
}

METHOD(listener_t, narrow, bool,
	private_addrblock_narrow_t *this, ike_sa_t *ike_sa, child_sa_t *child_sa,
	narrow_hook_t type, linked_list_t *local, linked_list_t *remote)
{
	switch (type)
	{
		case NARROW_RESPONDER:
		case NARROW_INITIATOR_POST_AUTH:
		case NARROW_INITIATOR_POST_NOAUTH:
			if (!check_constraints(ike_sa, remote))
			{
				flush_ts_list(local);
				flush_ts_list(remote);
			}
			break;
		default:
			break;
	}
	return TRUE;
}

METHOD(addrblock_narrow_t, destroy, void,
	private_addrblock_narrow_t *this)
{
	free(this);
}

/**
 * See header
 */
addrblock_narrow_t *addrblock_narrow_create()
{
	private_addrblock_narrow_t *this;

	INIT(this,
		.public = {
			.listener.narrow = _narrow,
			.destroy = _destroy,
		},
	);

	return &this->public;
}
