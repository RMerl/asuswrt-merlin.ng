/*
 * Copyright (C) 2010 Martin Willi
 * Copyright (C) 2010 revosec AG
 * Copyright (C) 2009 Andreas Steffen
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

static void narrow_addrblock(private_addrblock_narrow_t *this, ike_sa_t *ike_sa,
							 linked_list_t *list)
{
	certificate_t *cert = NULL;
	enumerator_t *enumerator;
	auth_cfg_t *auth;

	enumerator = ike_sa->create_auth_cfg_enumerator(ike_sa, FALSE);
	while (enumerator->enumerate(enumerator, &auth))
	{
		cert = auth->get(auth, AUTH_HELPER_SUBJECT_CERT);
		if (cert)
		{
			break;
		}
	}
	enumerator->destroy(enumerator);

	if (cert && cert->get_type(cert) == CERT_X509)
	{
		x509_t *x509 = (x509_t*)cert;

		if (x509->get_flags(x509) & X509_IP_ADDR_BLOCKS)
		{
			traffic_selector_t *ts, *block, *subset;
			linked_list_t *original;

			original = linked_list_create();
			while (list->remove_last(list, (void**)&ts) == SUCCESS)
			{
				original->insert_first(original, ts);
			}

			DBG1(DBG_IKE, "checking certificate-based traffic selector "
				 "constraints [RFC 3779]");
			while (original->remove_first(original, (void**)&ts) == SUCCESS)
			{
				bool contained = FALSE;

				enumerator = x509->create_ipAddrBlock_enumerator(x509);
				while (enumerator->enumerate(enumerator, &block))
				{
					subset = ts->get_subset(ts, block);
					if (subset)
					{
						DBG1(DBG_IKE, "  TS %R is contained in address block"
							 " constraint %R (subset %R)", ts, block, subset);
						list->insert_last(list, subset);
						contained = TRUE;
					}
				}
				enumerator->destroy(enumerator);

				if (!contained)
				{
					DBG1(DBG_IKE, "  TS %R is not contained in any"
						 " address block constraint", ts);
				}
				ts->destroy(ts);
			}
			original->destroy(original);
		}
	}
}

METHOD(listener_t, narrow, bool,
	private_addrblock_narrow_t *this, ike_sa_t *ike_sa, child_sa_t *child_sa,
	narrow_hook_t type, linked_list_t *local, linked_list_t *remote)
{
	switch (type)
	{
		case NARROW_RESPONDER:
		case NARROW_INITIATOR_PRE_AUTH:
		case NARROW_INITIATOR_POST_AUTH:
		case NARROW_INITIATOR_POST_NOAUTH:
			narrow_addrblock(this, ike_sa, remote);
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
