/*
 * Copyright (C) 2007 Tobias Brunner
 * Copyright (C) 2005-2006 Martin Willi
 * Copyright (C) 2005 Jan Hutter
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


#include "payload.h"

#include <encoding/payloads/ike_header.h>
#include <encoding/payloads/sa_payload.h>

#include <encoding/payloads/nonce_payload.h>
#include <encoding/payloads/id_payload.h>
#include <encoding/payloads/ke_payload.h>
#include <encoding/payloads/notify_payload.h>
#include <encoding/payloads/auth_payload.h>
#include <encoding/payloads/cert_payload.h>
#include <encoding/payloads/certreq_payload.h>
#include <encoding/payloads/encrypted_payload.h>
#include <encoding/payloads/encrypted_fragment_payload.h>
#include <encoding/payloads/ts_payload.h>
#include <encoding/payloads/delete_payload.h>
#include <encoding/payloads/vendor_id_payload.h>
#include <encoding/payloads/cp_payload.h>
#include <encoding/payloads/configuration_attribute.h>
#include <encoding/payloads/eap_payload.h>
#include <encoding/payloads/hash_payload.h>
#include <encoding/payloads/fragment_payload.h>
#include <encoding/payloads/unknown_payload.h>

ENUM_BEGIN(payload_type_names, PL_NONE, PL_NONE,
	"PL_NONE");
ENUM_NEXT(payload_type_names, PLV1_SECURITY_ASSOCIATION, PLV1_CONFIGURATION, PL_NONE,
	"SECURITY_ASSOCIATION_V1",
	"PROPOSAL_V1",
	"TRANSFORM_V1",
	"KEY_EXCHANGE_V1",
	"ID_V1",
	"CERTIFICATE_V1",
	"CERTREQ_V1",
	"HASH_V1",
	"SIGNATURE_V1",
	"NONCE_V1",
	"NOTIFY_V1",
	"DELETE_V1",
	"VENDOR_ID_V1",
	"CONFIGURATION_V1");
ENUM_NEXT(payload_type_names, PLV1_NAT_D, PLV1_NAT_OA, PLV1_CONFIGURATION,
	"NAT_D_V1",
	"NAT_OA_V1");
ENUM_NEXT(payload_type_names, PLV2_SECURITY_ASSOCIATION, PLV2_FRAGMENT, PLV1_NAT_OA,
	"SECURITY_ASSOCIATION",
	"KEY_EXCHANGE",
	"ID_INITIATOR",
	"ID_RESPONDER",
	"CERTIFICATE",
	"CERTREQ",
	"AUTH",
	"NONCE",
	"NOTIFY",
	"DELETE",
	"VENDOR_ID",
	"TS_INITIATOR",
	"TS_RESPONDER",
	"ENCRYPTED",
	"CONFIGURATION",
	"EAP",
	"GSPM",
	"GROUP_ID",
	"GROUP_SECURITY_ASSOCIATION",
	"KEY_DOWNLOAD",
	"ENCRYPTED_FRAGMENT");
#ifdef ME
ENUM_NEXT(payload_type_names, PLV2_ID_PEER, PLV2_ID_PEER, PLV2_FRAGMENT,
	"ID_PEER");
ENUM_NEXT(payload_type_names, PLV1_NAT_D_DRAFT_00_03, PLV1_FRAGMENT, PLV2_ID_PEER,
	"NAT_D_DRAFT_V1",
	"NAT_OA_DRAFT_V1",
	"FRAGMENT");
#else
ENUM_NEXT(payload_type_names, PLV1_NAT_D_DRAFT_00_03, PLV1_FRAGMENT, PLV2_FRAGMENT,
	"NAT_D_DRAFT_V1",
	"NAT_OA_DRAFT_V1",
	"FRAGMENT");
#endif /* ME */
ENUM_NEXT(payload_type_names, PL_HEADER, PLV1_ENCRYPTED, PLV1_FRAGMENT,
	"HEADER",
	"PROPOSAL_SUBSTRUCTURE",
	"PROPOSAL_SUBSTRUCTURE_V1",
	"TRANSFORM_SUBSTRUCTURE",
	"TRANSFORM_SUBSTRUCTURE_V1",
	"TRANSFORM_ATTRIBUTE",
	"TRANSFORM_ATTRIBUTE_V1",
	"TRAFFIC_SELECTOR_SUBSTRUCTURE",
	"CONFIGURATION_ATTRIBUTE",
	"CONFIGURATION_ATTRIBUTE_V1",
	"ENCRYPTED_V1");
ENUM_END(payload_type_names, PLV1_ENCRYPTED);

/* short forms of payload names */
ENUM_BEGIN(payload_type_short_names, PL_NONE, PL_NONE,
	"--");
ENUM_NEXT(payload_type_short_names, PLV1_SECURITY_ASSOCIATION, PLV1_CONFIGURATION, PL_NONE,
	"SA",
	"PROP",
	"TRANS",
	"KE",
	"ID",
	"CERT",
	"CERTREQ",
	"HASH",
	"SIG",
	"No",
	"N",
	"D",
	"V",
	"CP");
ENUM_NEXT(payload_type_short_names, PLV1_NAT_D, PLV1_NAT_OA, PLV1_CONFIGURATION,
	"NAT-D",
	"NAT-OA");
ENUM_NEXT(payload_type_short_names, PLV2_SECURITY_ASSOCIATION, PLV2_FRAGMENT, PLV1_NAT_OA,
	"SA",
	"KE",
	"IDi",
	"IDr",
	"CERT",
	"CERTREQ",
	"AUTH",
	"No",
	"N",
	"D",
	"V",
	"TSi",
	"TSr",
	"E",
	"CP",
	"EAP",
	"GSPM",
	"IDg",
	"GSA",
	"KD",
	"EF");
#ifdef ME
ENUM_NEXT(payload_type_short_names, PLV2_ID_PEER, PLV2_ID_PEER, PLV2_FRAGMENT,
	"IDp");
ENUM_NEXT(payload_type_short_names, PLV1_NAT_D_DRAFT_00_03, PLV1_FRAGMENT, PLV2_ID_PEER,
	"NAT-D",
	"NAT-OA",
	"FRAG");
#else
ENUM_NEXT(payload_type_short_names, PLV1_NAT_D_DRAFT_00_03, PLV1_FRAGMENT, PLV2_FRAGMENT,
	"NAT-D",
	"NAT-OA",
	"FRAG");
#endif /* ME */
ENUM_NEXT(payload_type_short_names, PL_HEADER, PLV1_ENCRYPTED, PLV1_FRAGMENT,
	"HDR",
	"PROP",
	"PROP",
	"TRANS",
	"TRANS",
	"TRANSATTR",
	"TRANSATTR",
	"TSSUB",
	"CATTR",
	"CATTR",
	"E");
ENUM_END(payload_type_short_names, PLV1_ENCRYPTED);

/*
 * see header
 */
payload_t *payload_create(payload_type_t type)
{
	switch (type)
	{
		case PL_HEADER:
			return (payload_t*)ike_header_create();
		case PLV2_SECURITY_ASSOCIATION:
		case PLV1_SECURITY_ASSOCIATION:
			return (payload_t*)sa_payload_create(type);
		case PLV2_PROPOSAL_SUBSTRUCTURE:
		case PLV1_PROPOSAL_SUBSTRUCTURE:
			return (payload_t*)proposal_substructure_create(type);
		case PLV2_TRANSFORM_SUBSTRUCTURE:
		case PLV1_TRANSFORM_SUBSTRUCTURE:
			return (payload_t*)transform_substructure_create(type);
		case PLV2_TRANSFORM_ATTRIBUTE:
		case PLV1_TRANSFORM_ATTRIBUTE:
			return (payload_t*)transform_attribute_create(type);
		case PLV2_NONCE:
		case PLV1_NONCE:
			return (payload_t*)nonce_payload_create(type);
		case PLV2_ID_INITIATOR:
		case PLV2_ID_RESPONDER:
		case PLV1_ID:
		case PLV1_NAT_OA:
		case PLV1_NAT_OA_DRAFT_00_03:
#ifdef ME
		case PLV2_ID_PEER:
#endif /* ME */
			return (payload_t*)id_payload_create(type);
		case PLV2_AUTH:
			return (payload_t*)auth_payload_create();
		case PLV2_CERTIFICATE:
		case PLV1_CERTIFICATE:
			return (payload_t*)cert_payload_create(type);
		case PLV2_CERTREQ:
		case PLV1_CERTREQ:
			return (payload_t*)certreq_payload_create(type);
		case PLV2_TRAFFIC_SELECTOR_SUBSTRUCTURE:
			return (payload_t*)traffic_selector_substructure_create();
		case PLV2_TS_INITIATOR:
			return (payload_t*)ts_payload_create(TRUE);
		case PLV2_TS_RESPONDER:
			return (payload_t*)ts_payload_create(FALSE);
		case PLV2_KEY_EXCHANGE:
		case PLV1_KEY_EXCHANGE:
			return (payload_t*)ke_payload_create(type);
		case PLV2_NOTIFY:
		case PLV1_NOTIFY:
			return (payload_t*)notify_payload_create(type);
		case PLV2_DELETE:
		case PLV1_DELETE:
			return (payload_t*)delete_payload_create(type, 0);
		case PLV2_VENDOR_ID:
		case PLV1_VENDOR_ID:
			return (payload_t*)vendor_id_payload_create(type);
		case PLV1_HASH:
		case PLV1_SIGNATURE:
		case PLV1_NAT_D:
		case PLV1_NAT_D_DRAFT_00_03:
			return (payload_t*)hash_payload_create(type);
		case PLV2_CONFIGURATION:
		case PLV1_CONFIGURATION:
			return (payload_t*)cp_payload_create(type);
		case PLV2_CONFIGURATION_ATTRIBUTE:
		case PLV1_CONFIGURATION_ATTRIBUTE:
			return (payload_t*)configuration_attribute_create(type);
		case PLV2_EAP:
			return (payload_t*)eap_payload_create();
		case PLV2_ENCRYPTED:
		case PLV1_ENCRYPTED:
			return (payload_t*)encrypted_payload_create(type);
		case PLV1_FRAGMENT:
			return (payload_t*)fragment_payload_create();
		case PLV2_FRAGMENT:
			return (payload_t*)encrypted_fragment_payload_create();
		default:
			return (payload_t*)unknown_payload_create(type);
	}
}

/**
 * See header.
 */
bool payload_is_known(payload_type_t type)
{
	if (type == PL_HEADER)
	{
		return TRUE;
	}
	if (type >= PLV1_SECURITY_ASSOCIATION && type <= PLV1_CONFIGURATION)
	{
		return TRUE;
	}
	if (type >= PLV1_NAT_D && type <= PLV1_NAT_OA)
	{
		return TRUE;
	}
	if (type >= PLV2_SECURITY_ASSOCIATION && type <= PLV2_EAP)
	{
		return TRUE;
	}
	if (type == PLV2_FRAGMENT)
	{
		return TRUE;
	}
#ifdef ME
	if (type == PLV2_ID_PEER)
	{
		return TRUE;
	}
#endif
	if (type >= PLV1_NAT_D_DRAFT_00_03 && type <= PLV1_FRAGMENT)
	{
		return TRUE;
	}
	return FALSE;
}

/**
 * See header.
 */
void* payload_get_field(payload_t *payload, encoding_type_t type, u_int skip)
{
	encoding_rule_t *rule;
	int i, count;

	count = payload->get_encoding_rules(payload, &rule);
	for (i = 0; i < count; i++)
	{
		if (rule[i].type == type && skip-- == 0)
		{
			return ((char*)payload) + rule[i].offset;
		}
	}
	return NULL;
}
