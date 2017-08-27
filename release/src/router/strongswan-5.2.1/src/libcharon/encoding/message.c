/*
 * Copyright (C) 2006-2014 Tobias Brunner
 * Copyright (C) 2005-2010 Martin Willi
 * Copyright (C) 2010 revosec AG
 * Copyright (C) 2006 Daniel Roethlisberger
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

#include <stdlib.h>
#include <string.h>

#include "message.h"

#include <library.h>
#include <bio/bio_writer.h>
#include <collections/array.h>
#include <daemon.h>
#include <sa/ikev1/keymat_v1.h>
#include <encoding/generator.h>
#include <encoding/parser.h>
#include <encoding/payloads/encodings.h>
#include <encoding/payloads/payload.h>
#include <encoding/payloads/hash_payload.h>
#include <encoding/payloads/encrypted_payload.h>
#include <encoding/payloads/encrypted_fragment_payload.h>
#include <encoding/payloads/unknown_payload.h>
#include <encoding/payloads/cp_payload.h>
#include <encoding/payloads/fragment_payload.h>

/**
 * Max number of notify payloads per IKEv2 message
 */
#define MAX_NOTIFY_PAYLOADS 20

/**
 * Max number of delete payloads per IKEv2 message
 */
#define MAX_DELETE_PAYLOADS 20

/**
 * Max number of certificate payloads per IKEv2 message
 */
#define MAX_CERT_PAYLOADS 8

/**
 * Max number of vendor ID payloads per IKEv2 message
 */
#define MAX_VID_PAYLOADS 20

/**
 * Max number of certificate request payloads per IKEv1 message
 */
#define MAX_CERTREQ_PAYLOADS 20

/**
 * Max number of NAT-D payloads per IKEv1 message
 */
#define MAX_NAT_D_PAYLOADS 10

/**
 * A payload rule defines the rules for a payload
 * in a specific message rule. It defines if and how
 * many times a payload must/can occur in a message
 * and if it must be encrypted.
 */
typedef struct {
	/* Payload type */
	 payload_type_t type;
	/* Minimal occurrence of this payload. */
	size_t min_occurence;
	/* Max occurrence of this payload. */
	size_t max_occurence;
	/* TRUE if payload must be encrypted */
	bool encrypted;
	/* If payload occurs, the message rule is fulfilled */
	bool sufficient;
} payload_rule_t;

/**
 * payload ordering structure allows us to reorder payloads according to RFC.
 */
typedef struct {
	/** payload type */
	payload_type_t type;
	/** notify type, if payload == PLV2_NOTIFY */
	notify_type_t notify;
} payload_order_t;

/**
 * A message rule defines the kind of a message,
 * if it has encrypted contents and a list
 * of payload ordering rules and payload parsing rules.
 */
typedef struct {
	/** Type of message. */
	exchange_type_t exchange_type;
	/** Is message a request or response. */
	bool is_request;
	/** Message contains encrypted payloads. */
	bool encrypted;
	/** Number of payload rules which will follow */
	int rule_count;
	/** Pointer to first payload rule */
	payload_rule_t *rules;
	/** Number of payload order rules */
	int order_count;
	/** payload ordering rules */
	payload_order_t *order;
} message_rule_t;

/**
 * Message rule for IKE_SA_INIT from initiator.
 */
static payload_rule_t ike_sa_init_i_rules[] = {
/*	payload type					min	max						encr	suff */
	{PLV2_NOTIFY,					0,	MAX_NOTIFY_PAYLOADS,	FALSE,	FALSE},
	{PLV2_SECURITY_ASSOCIATION,		1,	1,						FALSE,	FALSE},
	{PLV2_KEY_EXCHANGE,				1,	1,						FALSE,	FALSE},
	{PLV2_NONCE,					1,	1,						FALSE,	FALSE},
	{PLV2_VENDOR_ID,				0,	MAX_VID_PAYLOADS,		FALSE,	FALSE},
};

/**
 * payload order for IKE_SA_INIT initiator
 */
static payload_order_t ike_sa_init_i_order[] = {
/*	payload type					notify type */
	{PLV2_NOTIFY,					COOKIE},
	{PLV2_SECURITY_ASSOCIATION,		0},
	{PLV2_KEY_EXCHANGE,				0},
	{PLV2_NONCE,					0},
	{PLV2_NOTIFY,					NAT_DETECTION_SOURCE_IP},
	{PLV2_NOTIFY,					NAT_DETECTION_DESTINATION_IP},
	{PLV2_NOTIFY,					0},
	{PLV2_VENDOR_ID,				0},
};

/**
 * Message rule for IKE_SA_INIT from responder.
 */
static payload_rule_t ike_sa_init_r_rules[] = {
/*	payload type					min	max						encr	suff */
	{PLV2_NOTIFY,					0,	MAX_NOTIFY_PAYLOADS,	FALSE,	TRUE},
	{PLV2_SECURITY_ASSOCIATION,		1,	1,						FALSE,	FALSE},
	{PLV2_KEY_EXCHANGE,				1,	1,						FALSE,	FALSE},
	{PLV2_NONCE,					1,	1,						FALSE,	FALSE},
	{PLV2_CERTREQ,					0,	MAX_CERTREQ_PAYLOADS,	FALSE,	FALSE},
	{PLV2_VENDOR_ID,				0,	MAX_VID_PAYLOADS,		FALSE,	FALSE},
};

/**
 * payload order for IKE_SA_INIT responder
 */
static payload_order_t ike_sa_init_r_order[] = {
/*	payload type					notify type */
	{PLV2_SECURITY_ASSOCIATION,		0},
	{PLV2_KEY_EXCHANGE,				0},
	{PLV2_NONCE,					0},
	{PLV2_NOTIFY,					NAT_DETECTION_SOURCE_IP},
	{PLV2_NOTIFY,					NAT_DETECTION_DESTINATION_IP},
	{PLV2_NOTIFY,					HTTP_CERT_LOOKUP_SUPPORTED},
	{PLV2_CERTREQ,					0},
	{PLV2_NOTIFY,					0},
	{PLV2_VENDOR_ID,				0},
};

/**
 * Message rule for IKE_AUTH from initiator.
 */
static payload_rule_t ike_auth_i_rules[] = {
/*	payload type					min	max						encr	suff */
	{PLV2_NOTIFY,					0,	MAX_NOTIFY_PAYLOADS,	TRUE,	FALSE},
	{PLV2_EAP,						0,	1,						TRUE,	TRUE},
	{PLV2_AUTH,						0,	1,						TRUE,	TRUE},
	{PLV2_ID_INITIATOR,				0,	1,						TRUE,	FALSE},
	{PLV2_CERTIFICATE,				0,	MAX_CERT_PAYLOADS,		TRUE,	FALSE},
	{PLV2_CERTREQ,					0,	MAX_CERTREQ_PAYLOADS,	TRUE,	FALSE},
	{PLV2_ID_RESPONDER,				0,	1,						TRUE,	FALSE},
#ifdef ME
	{PLV2_SECURITY_ASSOCIATION,		0,	1,						TRUE,	FALSE},
	{PLV2_TS_INITIATOR,				0,	1,						TRUE,	FALSE},
	{PLV2_TS_RESPONDER,				0,	1,						TRUE,	FALSE},
#else
	{PLV2_SECURITY_ASSOCIATION,		0,	1,						TRUE,	FALSE},
	{PLV2_TS_INITIATOR,				0,	1,						TRUE,	FALSE},
	{PLV2_TS_RESPONDER,				0,	1,						TRUE,	FALSE},
#endif /* ME */
	{PLV2_CONFIGURATION,			0,	1,						TRUE,	FALSE},
	{PLV2_VENDOR_ID,				0,	MAX_VID_PAYLOADS,		TRUE,	FALSE},
};

/**
 * payload order for IKE_AUTH initiator
 */
static payload_order_t ike_auth_i_order[] = {
/*	payload type					notify type */
	{PLV2_ID_INITIATOR,				0},
	{PLV2_CERTIFICATE,				0},
	{PLV2_NOTIFY,					INITIAL_CONTACT},
	{PLV2_NOTIFY,					HTTP_CERT_LOOKUP_SUPPORTED},
	{PLV2_CERTREQ,					0},
	{PLV2_ID_RESPONDER,				0},
	{PLV2_AUTH,						0},
	{PLV2_EAP,						0},
	{PLV2_CONFIGURATION,			0},
	{PLV2_NOTIFY,					IPCOMP_SUPPORTED},
	{PLV2_NOTIFY,					USE_TRANSPORT_MODE},
	{PLV2_NOTIFY,					ESP_TFC_PADDING_NOT_SUPPORTED},
	{PLV2_NOTIFY,					NON_FIRST_FRAGMENTS_ALSO},
	{PLV2_SECURITY_ASSOCIATION,		0},
	{PLV2_TS_INITIATOR,				0},
	{PLV2_TS_RESPONDER,				0},
	{PLV2_NOTIFY,					MOBIKE_SUPPORTED},
	{PLV2_NOTIFY,					ADDITIONAL_IP4_ADDRESS},
	{PLV2_NOTIFY,					ADDITIONAL_IP6_ADDRESS},
	{PLV2_NOTIFY,					NO_ADDITIONAL_ADDRESSES},
	{PLV2_NOTIFY,					0},
	{PLV2_VENDOR_ID,				0},
};

/**
 * Message rule for IKE_AUTH from responder.
 */
static payload_rule_t ike_auth_r_rules[] = {
/*	payload type					min	max						encr	suff */
	{PLV2_NOTIFY,					0,	MAX_NOTIFY_PAYLOADS,	TRUE,	TRUE},
	{PLV2_EAP,						0,	1,						TRUE,	TRUE},
	{PLV2_AUTH,						0,	1,						TRUE,	TRUE},
	{PLV2_CERTIFICATE,				0,	MAX_CERT_PAYLOADS,		TRUE,	FALSE},
	{PLV2_ID_RESPONDER,				0,	1,						TRUE,	FALSE},
	{PLV2_SECURITY_ASSOCIATION,		0,	1,						TRUE,	FALSE},
	{PLV2_TS_INITIATOR,				0,	1,						TRUE,	FALSE},
	{PLV2_TS_RESPONDER,				0,	1,						TRUE,	FALSE},
	{PLV2_CONFIGURATION,			0,	1,						TRUE,	FALSE},
	{PLV2_VENDOR_ID,				0,	MAX_VID_PAYLOADS,		TRUE,	FALSE},
};

/**
 * payload order for IKE_AUTH responder
 */
static payload_order_t ike_auth_r_order[] = {
/*	payload type					notify type */
	{PLV2_ID_RESPONDER,				0},
	{PLV2_CERTIFICATE,				0},
	{PLV2_AUTH,						0},
	{PLV2_EAP,						0},
	{PLV2_CONFIGURATION,			0},
	{PLV2_NOTIFY,					IPCOMP_SUPPORTED},
	{PLV2_NOTIFY,					USE_TRANSPORT_MODE},
	{PLV2_NOTIFY,					ESP_TFC_PADDING_NOT_SUPPORTED},
	{PLV2_NOTIFY,					NON_FIRST_FRAGMENTS_ALSO},
	{PLV2_SECURITY_ASSOCIATION,		0},
	{PLV2_TS_INITIATOR,				0},
	{PLV2_TS_RESPONDER,				0},
	{PLV2_NOTIFY,					AUTH_LIFETIME},
	{PLV2_NOTIFY,					MOBIKE_SUPPORTED},
	{PLV2_NOTIFY,					ADDITIONAL_IP4_ADDRESS},
	{PLV2_NOTIFY,					ADDITIONAL_IP6_ADDRESS},
	{PLV2_NOTIFY,					NO_ADDITIONAL_ADDRESSES},
	{PLV2_NOTIFY,					0},
	{PLV2_VENDOR_ID,				0},
};

/**
 * Message rule for INFORMATIONAL from initiator.
 */
static payload_rule_t informational_i_rules[] = {
/*	payload type					min	max						encr	suff */
	{PLV2_NOTIFY,					0,	MAX_NOTIFY_PAYLOADS,	TRUE,	FALSE},
	{PLV2_CONFIGURATION,			0,	1,						TRUE,	FALSE},
	{PLV2_DELETE,					0,	MAX_DELETE_PAYLOADS,	TRUE,	FALSE},
	{PLV2_VENDOR_ID,				0,	MAX_VID_PAYLOADS,		TRUE,	FALSE},
};

/**
 * payload order for INFORMATIONAL initiator
 */
static payload_order_t informational_i_order[] = {
/*	payload type					notify type */
	{PLV2_NOTIFY,					UPDATE_SA_ADDRESSES},
	{PLV2_NOTIFY,					NAT_DETECTION_SOURCE_IP},
	{PLV2_NOTIFY,					NAT_DETECTION_DESTINATION_IP},
	{PLV2_NOTIFY,					COOKIE2},
	{PLV2_NOTIFY,					0},
	{PLV2_DELETE,					0},
	{PLV2_CONFIGURATION,			0},
};

/**
 * Message rule for INFORMATIONAL from responder.
 */
static payload_rule_t informational_r_rules[] = {
/*	payload type					min	max						encr	suff */
	{PLV2_NOTIFY,					0,	MAX_NOTIFY_PAYLOADS,	TRUE,	FALSE},
	{PLV2_CONFIGURATION,			0,	1,						TRUE,	FALSE},
	{PLV2_DELETE,					0,	MAX_DELETE_PAYLOADS,	TRUE,	FALSE},
	{PLV2_VENDOR_ID,				0,	MAX_VID_PAYLOADS,		TRUE,	FALSE},
};

/**
 * payload order for INFORMATIONAL responder
 */
static payload_order_t informational_r_order[] = {
/*	payload type					notify type */
	{PLV2_NOTIFY,					UPDATE_SA_ADDRESSES},
	{PLV2_NOTIFY,					NAT_DETECTION_SOURCE_IP},
	{PLV2_NOTIFY,					NAT_DETECTION_DESTINATION_IP},
	{PLV2_NOTIFY,					COOKIE2},
	{PLV2_NOTIFY,					0},
	{PLV2_DELETE,					0},
	{PLV2_CONFIGURATION,			0},
};

/**
 * Message rule for CREATE_CHILD_SA from initiator.
 */
static payload_rule_t create_child_sa_i_rules[] = {
/*	payload type					min	max						encr	suff */
	{PLV2_NOTIFY,					0,	MAX_NOTIFY_PAYLOADS,	TRUE,	FALSE},
	{PLV2_SECURITY_ASSOCIATION,		1,	1,						TRUE,	FALSE},
	{PLV2_NONCE,					1,	1,						TRUE,	FALSE},
	{PLV2_KEY_EXCHANGE,				0,	1,						TRUE,	FALSE},
	{PLV2_TS_INITIATOR,				0,	1,						TRUE,	FALSE},
	{PLV2_TS_RESPONDER,				0,	1,						TRUE,	FALSE},
	{PLV2_CONFIGURATION,			0,	1,						TRUE,	FALSE},
	{PLV2_VENDOR_ID,				0,	MAX_VID_PAYLOADS,		TRUE,	FALSE},
};

/**
 * payload order for CREATE_CHILD_SA from initiator.
 */
static payload_order_t create_child_sa_i_order[] = {
/*	payload type					notify type */
	{PLV2_NOTIFY,					REKEY_SA},
	{PLV2_NOTIFY,					IPCOMP_SUPPORTED},
	{PLV2_NOTIFY,					USE_TRANSPORT_MODE},
	{PLV2_NOTIFY,					ESP_TFC_PADDING_NOT_SUPPORTED},
	{PLV2_NOTIFY,					NON_FIRST_FRAGMENTS_ALSO},
	{PLV2_SECURITY_ASSOCIATION,		0},
	{PLV2_NONCE,					0},
	{PLV2_KEY_EXCHANGE,				0},
	{PLV2_TS_INITIATOR,				0},
	{PLV2_TS_RESPONDER,				0},
	{PLV2_NOTIFY,					0},
};

/**
 * Message rule for CREATE_CHILD_SA from responder.
 */
static payload_rule_t create_child_sa_r_rules[] = {
/*	payload type					min	max						encr	suff */
	{PLV2_NOTIFY,					0,	MAX_NOTIFY_PAYLOADS,	TRUE,	TRUE},
	{PLV2_SECURITY_ASSOCIATION,		1,	1,						TRUE,	FALSE},
	{PLV2_NONCE,					1,	1,						TRUE,	FALSE},
	{PLV2_KEY_EXCHANGE,				0,	1,						TRUE,	FALSE},
	{PLV2_TS_INITIATOR,				0,	1,						TRUE,	FALSE},
	{PLV2_TS_RESPONDER,				0,	1,						TRUE,	FALSE},
	{PLV2_CONFIGURATION,			0,	1,						TRUE,	FALSE},
	{PLV2_VENDOR_ID,				0,	MAX_VID_PAYLOADS,		TRUE,	FALSE},
};

/**
 * payload order for CREATE_CHILD_SA from responder.
 */
static payload_order_t create_child_sa_r_order[] = {
/*	payload type					notify type */
	{PLV2_NOTIFY,					IPCOMP_SUPPORTED},
	{PLV2_NOTIFY,					USE_TRANSPORT_MODE},
	{PLV2_NOTIFY,					ESP_TFC_PADDING_NOT_SUPPORTED},
	{PLV2_NOTIFY,					NON_FIRST_FRAGMENTS_ALSO},
	{PLV2_SECURITY_ASSOCIATION,		0},
	{PLV2_NONCE,					0},
	{PLV2_KEY_EXCHANGE,				0},
	{PLV2_TS_INITIATOR,				0},
	{PLV2_TS_RESPONDER,				0},
	{PLV2_NOTIFY,					ADDITIONAL_TS_POSSIBLE},
	{PLV2_NOTIFY,					0},
};

#ifdef ME
/**
 * Message rule for ME_CONNECT from initiator.
 */
static payload_rule_t me_connect_i_rules[] = {
/*	payload type					min	max						encr	suff */
	{PLV2_NOTIFY,					0,	MAX_NOTIFY_PAYLOADS,	TRUE,	TRUE},
	{PLV2_ID_PEER,					1,	1,						TRUE,	FALSE},
	{PLV2_VENDOR_ID,				0,	MAX_VID_PAYLOADS,		TRUE,	FALSE}
};

/**
 * payload order for ME_CONNECT from initiator.
 */
static payload_order_t me_connect_i_order[] = {
/*	payload type					notify type */
	{PLV2_NOTIFY,					0},
	{PLV2_ID_PEER,					0},
	{PLV2_VENDOR_ID,				0},
};

/**
 * Message rule for ME_CONNECT from responder.
 */
static payload_rule_t me_connect_r_rules[] = {
/*	payload type					min	max						encr	suff */
	{PLV2_NOTIFY,					0,	MAX_NOTIFY_PAYLOADS,	TRUE,	TRUE},
	{PLV2_VENDOR_ID,				0,	MAX_VID_PAYLOADS,		TRUE,	FALSE}
};

/**
 * payload order for ME_CONNECT from responder.
 */
static payload_order_t me_connect_r_order[] = {
/*	payload type					notify type */
	{PLV2_NOTIFY,					0},
	{PLV2_VENDOR_ID,				0},
};
#endif /* ME */

#ifdef USE_IKEV1
/**
 * Message rule for ID_PROT from initiator.
 */
static payload_rule_t id_prot_i_rules[] = {
/*	payload type					min	max						encr	suff */
	{PLV1_NOTIFY,					0,	MAX_NOTIFY_PAYLOADS,	FALSE,	FALSE},
	{PLV1_SECURITY_ASSOCIATION,		0,	1,						FALSE,	FALSE},
	{PLV1_KEY_EXCHANGE,				0,	1,						FALSE,	FALSE},
	{PLV1_NONCE,					0,	1,						FALSE,	FALSE},
	{PLV1_VENDOR_ID,				0,	MAX_VID_PAYLOADS,		FALSE,	FALSE},
	{PLV1_CERTREQ,					0,	MAX_CERTREQ_PAYLOADS,	FALSE,	FALSE},
	{PLV1_NAT_D,					0,	MAX_NAT_D_PAYLOADS,		FALSE,	FALSE},
	{PLV1_NAT_D_DRAFT_00_03,		0,	MAX_NAT_D_PAYLOADS,		FALSE,	FALSE},
	{PLV1_ID,						0,	1,						TRUE,	FALSE},
	{PLV1_CERTIFICATE,				0,	MAX_CERT_PAYLOADS,		TRUE,	FALSE},
	{PLV1_SIGNATURE,				0,	1,						TRUE,	FALSE},
	{PLV1_HASH,						0,	1,						TRUE,	FALSE},
	{PLV1_FRAGMENT,					0,	1,						FALSE,	TRUE},
};

/**
 * payload order for ID_PROT from initiator.
 */
static payload_order_t id_prot_i_order[] = {
/*	payload type					notify type */
	{PLV1_SECURITY_ASSOCIATION,		0},
	{PLV1_KEY_EXCHANGE,				0},
	{PLV1_NONCE,					0},
	{PLV1_ID,						0},
	{PLV1_CERTIFICATE,				0},
	{PLV1_SIGNATURE,				0},
	{PLV1_HASH,						0},
	{PLV1_CERTREQ,					0},
	{PLV1_NOTIFY,					0},
	{PLV1_VENDOR_ID,				0},
	{PLV1_NAT_D,					0},
	{PLV1_NAT_D_DRAFT_00_03,		0},
	{PLV1_FRAGMENT,					0},
};

/**
 * Message rule for ID_PROT from responder.
 */
static payload_rule_t id_prot_r_rules[] = {
/*	payload type					min	max						encr	suff */
	{PLV1_NOTIFY,					0,	MAX_NOTIFY_PAYLOADS,	FALSE,	FALSE},
	{PLV1_SECURITY_ASSOCIATION,		0,	1,						FALSE,	FALSE},
	{PLV1_KEY_EXCHANGE,				0,	1,						FALSE,	FALSE},
	{PLV1_NONCE,					0,	1,						FALSE,	FALSE},
	{PLV1_VENDOR_ID,				0,	MAX_VID_PAYLOADS,		FALSE,	FALSE},
	{PLV1_CERTREQ,					0,	MAX_CERTREQ_PAYLOADS,	FALSE,	FALSE},
	{PLV1_NAT_D,					0,	MAX_NAT_D_PAYLOADS,		FALSE,	FALSE},
	{PLV1_NAT_D_DRAFT_00_03,		0,	MAX_NAT_D_PAYLOADS,		FALSE,	FALSE},
	{PLV1_ID,						0,	1,						TRUE,	FALSE},
	{PLV1_CERTIFICATE,				0,	MAX_CERT_PAYLOADS,		TRUE,	FALSE},
	{PLV1_SIGNATURE,				0,	1,						TRUE,	FALSE},
	{PLV1_HASH,						0,	1,						TRUE,	FALSE},
	{PLV1_FRAGMENT,					0,	1,						FALSE,	TRUE},
};

/**
 * payload order for ID_PROT from responder.
 */
static payload_order_t id_prot_r_order[] = {
/*	payload type					notify type */
	{PLV1_SECURITY_ASSOCIATION,		0},
	{PLV1_KEY_EXCHANGE,				0},
	{PLV1_NONCE,					0},
	{PLV1_ID,						0},
	{PLV1_CERTIFICATE,				0},
	{PLV1_SIGNATURE,				0},
	{PLV1_HASH,						0},
	{PLV1_CERTREQ,					0},
	{PLV1_NOTIFY,					0},
	{PLV1_VENDOR_ID,				0},
	{PLV1_NAT_D,					0},
	{PLV1_NAT_D_DRAFT_00_03,		0},
	{PLV1_FRAGMENT,					0},
};

/**
 * Message rule for AGGRESSIVE from initiator.
 */
static payload_rule_t aggressive_i_rules[] = {
/*	payload type					min	max						encr	suff */
	{PLV1_NOTIFY,					0,	MAX_NOTIFY_PAYLOADS,	FALSE,	FALSE},
	{PLV1_SECURITY_ASSOCIATION,		0,	1,						FALSE,	FALSE},
	{PLV1_KEY_EXCHANGE,				0,	1,						FALSE,	FALSE},
	{PLV1_NONCE,					0,	1,						FALSE,	FALSE},
	{PLV1_VENDOR_ID,				0,	MAX_VID_PAYLOADS,		FALSE,	FALSE},
	{PLV1_CERTREQ,					0,	MAX_CERTREQ_PAYLOADS,	FALSE,	FALSE},
	{PLV1_NAT_D,					0,	MAX_NAT_D_PAYLOADS,		FALSE,	FALSE},
	{PLV1_NAT_D_DRAFT_00_03,		0,	MAX_NAT_D_PAYLOADS,		FALSE,	FALSE},
	{PLV1_ID,						0,	1,						FALSE,	FALSE},
	{PLV1_CERTIFICATE,				0,	1,						TRUE,	FALSE},
	{PLV1_SIGNATURE,				0,	1,						TRUE,	FALSE},
	{PLV1_HASH,						0,	1,						TRUE,	FALSE},
	{PLV1_FRAGMENT,					0,	1,						FALSE,	TRUE},
};

/**
 * payload order for AGGRESSIVE from initiator.
 */
static payload_order_t aggressive_i_order[] = {
/*	payload type					notify type */
	{PLV1_SECURITY_ASSOCIATION,		0},
	{PLV1_KEY_EXCHANGE,				0},
	{PLV1_NONCE,					0},
	{PLV1_ID,						0},
	{PLV1_CERTIFICATE,				0},
	{PLV1_NAT_D,					0},
	{PLV1_NAT_D_DRAFT_00_03,		0},
	{PLV1_SIGNATURE,				0},
	{PLV1_HASH,						0},
	{PLV1_CERTREQ,					0},
	{PLV1_NOTIFY,					0},
	{PLV1_VENDOR_ID,				0},
	{PLV1_FRAGMENT,					0},
};

/**
 * Message rule for AGGRESSIVE from responder.
 */
static payload_rule_t aggressive_r_rules[] = {
/*	payload type					min	max						encr	suff */
	{PLV1_NOTIFY,					0,	MAX_NOTIFY_PAYLOADS,	FALSE,	FALSE},
	{PLV1_SECURITY_ASSOCIATION,		0,	1,						FALSE,	FALSE},
	{PLV1_KEY_EXCHANGE,				0,	1,						FALSE,	FALSE},
	{PLV1_NONCE,					0,	1,						FALSE,	FALSE},
	{PLV1_VENDOR_ID,				0,	MAX_VID_PAYLOADS,		FALSE,	FALSE},
	{PLV1_CERTREQ,					0,	MAX_CERTREQ_PAYLOADS,	FALSE,	FALSE},
	{PLV1_NAT_D,					0,	MAX_NAT_D_PAYLOADS,		FALSE,	FALSE},
	{PLV1_NAT_D_DRAFT_00_03,		0,	MAX_NAT_D_PAYLOADS,		FALSE,	FALSE},
	{PLV1_ID,						0,	1,						FALSE,	FALSE},
	{PLV1_CERTIFICATE,				0,	1,						FALSE,	FALSE},
	{PLV1_SIGNATURE,				0,	1,						FALSE,	FALSE},
	{PLV1_HASH,						0,	1,						FALSE,	FALSE},
	{PLV1_FRAGMENT,					0,	1,						FALSE,	TRUE},
};

/**
 * payload order for AGGRESSIVE from responder.
 */
static payload_order_t aggressive_r_order[] = {
/*	payload type					notify type */
	{PLV1_SECURITY_ASSOCIATION,		0},
	{PLV1_KEY_EXCHANGE,				0},
	{PLV1_NONCE,					0},
	{PLV1_ID,						0},
	{PLV1_CERTIFICATE,				0},
	{PLV1_NAT_D,					0},
	{PLV1_NAT_D_DRAFT_00_03,		0},
	{PLV1_SIGNATURE,				0},
	{PLV1_HASH,						0},
	{PLV1_CERTREQ,					0},
	{PLV1_NOTIFY,					0},
	{PLV1_VENDOR_ID,				0},
	{PLV1_FRAGMENT,					0},
};

/**
 * Message rule for INFORMATIONAL_V1 from initiator.
 */
static payload_rule_t informational_i_rules_v1[] = {
/*	payload type					min	max						encr	suff */
	{PLV1_NOTIFY,					0,	MAX_NOTIFY_PAYLOADS,	FALSE,	FALSE},
	{PLV1_NOTIFY,					0,	MAX_NOTIFY_PAYLOADS,	TRUE,	FALSE},
	{PLV1_DELETE,					0,	MAX_DELETE_PAYLOADS,	TRUE,	FALSE},
	{PLV1_VENDOR_ID,				0,	MAX_VID_PAYLOADS,		TRUE,	FALSE},
};

/**
 * payload order for INFORMATIONAL_V1 from initiator.
 */
static payload_order_t informational_i_order_v1[] = {
/*	payload type					notify type */
	{PLV1_NOTIFY,					0},
	{PLV1_DELETE,					0},
	{PLV1_VENDOR_ID,				0},
};

/**
 * Message rule for INFORMATIONAL_V1 from responder.
 */
static payload_rule_t informational_r_rules_v1[] = {
/*	payload type					min	max						encr	suff */
	{PLV1_NOTIFY,					0,	MAX_NOTIFY_PAYLOADS,	FALSE,	FALSE},
	{PLV1_NOTIFY,					0,	MAX_NOTIFY_PAYLOADS,	TRUE,	FALSE},
	{PLV1_DELETE,					0,	MAX_DELETE_PAYLOADS,	TRUE,	FALSE},
	{PLV1_VENDOR_ID,				0,	MAX_VID_PAYLOADS,		TRUE,	FALSE},
};

/**
 * payload order for INFORMATIONAL_V1 from responder.
 */
static payload_order_t informational_r_order_v1[] = {
/*	payload type					notify type */
	{PLV1_NOTIFY,					0},
	{PLV1_DELETE,					0},
	{PLV1_VENDOR_ID,				0},
};

/**
 * Message rule for QUICK_MODE from initiator.
 */
static payload_rule_t quick_mode_i_rules[] = {
/*	payload type					min	max						encr	suff */
	{PLV1_NOTIFY,					0,	MAX_NOTIFY_PAYLOADS,	TRUE,	FALSE},
	{PLV1_VENDOR_ID,				0,	MAX_VID_PAYLOADS,		TRUE,	FALSE},
	{PLV1_HASH,						0,	1,						TRUE,	FALSE},
	{PLV1_SECURITY_ASSOCIATION,		0,	2,						TRUE,	FALSE},
	{PLV1_NONCE,					0,	1,						TRUE,	FALSE},
	{PLV1_KEY_EXCHANGE,				0,	1,						TRUE,	FALSE},
	{PLV1_ID,						0,	2,						TRUE,	FALSE},
	{PLV1_NAT_OA,					0,	2,						TRUE,	FALSE},
	{PLV1_NAT_OA_DRAFT_00_03,		0,	2,						TRUE,	FALSE},
};

/**
 * payload order for QUICK_MODE from initiator.
 */
static payload_order_t quick_mode_i_order[] = {
/*	payload type					notify type */
	{PLV1_NOTIFY,					0},
	{PLV1_VENDOR_ID,				0},
	{PLV1_HASH,						0},
	{PLV1_SECURITY_ASSOCIATION,		0},
	{PLV1_NONCE,					0},
	{PLV1_KEY_EXCHANGE,				0},
	{PLV1_ID,						0},
	{PLV1_NAT_OA,					0},
	{PLV1_NAT_OA_DRAFT_00_03,		0},
};

/**
 * Message rule for QUICK_MODE from responder.
 */
static payload_rule_t quick_mode_r_rules[] = {
/*	payload type					min	max						encr	suff */
	{PLV1_NOTIFY,					0,	MAX_NOTIFY_PAYLOADS,	TRUE,	FALSE},
	{PLV1_VENDOR_ID,				0,	MAX_VID_PAYLOADS,		TRUE,	FALSE},
	{PLV1_HASH,						0,	1,						TRUE,	FALSE},
	{PLV1_SECURITY_ASSOCIATION,		0,	2,						TRUE,	FALSE},
	{PLV1_NONCE,					0,	1,						TRUE,	FALSE},
	{PLV1_KEY_EXCHANGE,				0,	1,						TRUE,	FALSE},
	{PLV1_ID,						0,	2,						TRUE,	FALSE},
	{PLV1_NAT_OA,					0,	2,						TRUE,	FALSE},
	{PLV1_NAT_OA_DRAFT_00_03,		0,	2,						TRUE,	FALSE},
};

/**
 * payload order for QUICK_MODE from responder.
 */
static payload_order_t quick_mode_r_order[] = {
/*	payload type					notify type */
	{PLV1_NOTIFY,					0},
	{PLV1_VENDOR_ID,				0},
	{PLV1_HASH,						0},
	{PLV1_SECURITY_ASSOCIATION,		0},
	{PLV1_NONCE,					0},
	{PLV1_KEY_EXCHANGE,				0},
	{PLV1_ID,						0},
	{PLV1_NAT_OA,					0},
	{PLV1_NAT_OA_DRAFT_00_03,		0},
};

/**
 * Message rule for TRANSACTION.
 */
static payload_rule_t transaction_payload_rules_v1[] = {
/*	payload type					min	max	encr	suff */
	{PLV1_HASH,						0,	1,	TRUE,	FALSE},
	{PLV1_CONFIGURATION,			1,	1,	FALSE,	FALSE},
};

/**
 * Payload order for TRANSACTION.
 */
static payload_order_t transaction_payload_order_v1[] = {
/*	payload type					notify type */
	{PLV1_HASH,						0},
	{PLV1_CONFIGURATION,			0},
};

#endif /* USE_IKEV1 */

/**
 * Message rules, defines allowed payloads.
 */
static message_rule_t message_rules[] = {
	{IKE_SA_INIT,		TRUE,	FALSE,
		countof(ike_sa_init_i_rules), ike_sa_init_i_rules,
		countof(ike_sa_init_i_order), ike_sa_init_i_order,
	},
	{IKE_SA_INIT,		FALSE,	FALSE,
		countof(ike_sa_init_r_rules), ike_sa_init_r_rules,
		countof(ike_sa_init_r_order), ike_sa_init_r_order,
	},
	{IKE_AUTH,			TRUE,	TRUE,
		countof(ike_auth_i_rules), ike_auth_i_rules,
		countof(ike_auth_i_order), ike_auth_i_order,
	},
	{IKE_AUTH,			FALSE,	TRUE,
		countof(ike_auth_r_rules), ike_auth_r_rules,
		countof(ike_auth_r_order), ike_auth_r_order,
	},
	{INFORMATIONAL,		TRUE,	TRUE,
		countof(informational_i_rules), informational_i_rules,
		countof(informational_i_order), informational_i_order,
	},
	{INFORMATIONAL,		FALSE,	TRUE,
		countof(informational_r_rules), informational_r_rules,
		countof(informational_r_order), informational_r_order,
	},
	{CREATE_CHILD_SA,	TRUE,	TRUE,
		countof(create_child_sa_i_rules), create_child_sa_i_rules,
		countof(create_child_sa_i_order), create_child_sa_i_order,
	},
	{CREATE_CHILD_SA,	FALSE,	TRUE,
		countof(create_child_sa_r_rules), create_child_sa_r_rules,
		countof(create_child_sa_r_order), create_child_sa_r_order,
	},
#ifdef ME
	{ME_CONNECT,		TRUE,	TRUE,
		countof(me_connect_i_rules), me_connect_i_rules,
		countof(me_connect_i_order), me_connect_i_order,
	},
	{ME_CONNECT,		FALSE,	TRUE,
		countof(me_connect_r_rules), me_connect_r_rules,
		countof(me_connect_r_order), me_connect_r_order,
	},
#endif /* ME */
#ifdef USE_IKEV1
	{ID_PROT,			TRUE,	FALSE,
		countof(id_prot_i_rules), id_prot_i_rules,
		countof(id_prot_i_order), id_prot_i_order,
	},
	{ID_PROT,			FALSE,	FALSE,
		countof(id_prot_r_rules), id_prot_r_rules,
		countof(id_prot_r_order), id_prot_r_order,
	},
	{AGGRESSIVE,		TRUE,	FALSE,
		countof(aggressive_i_rules), aggressive_i_rules,
		countof(aggressive_i_order), aggressive_i_order,
	},
	{AGGRESSIVE,		FALSE,	FALSE,
		countof(aggressive_r_rules), aggressive_r_rules,
		countof(aggressive_r_order), aggressive_r_order,
	},
	{INFORMATIONAL_V1,	TRUE,	TRUE,
		countof(informational_i_rules_v1), informational_i_rules_v1,
		countof(informational_i_order_v1), informational_i_order_v1,
	},
	{INFORMATIONAL_V1,	FALSE,	TRUE,
		countof(informational_r_rules_v1), informational_r_rules_v1,
		countof(informational_r_order_v1), informational_r_order_v1,
	},
	{QUICK_MODE,		TRUE,	TRUE,
		countof(quick_mode_i_rules), quick_mode_i_rules,
		countof(quick_mode_i_order), quick_mode_i_order,
	},
	{QUICK_MODE,		FALSE,	TRUE,
		countof(quick_mode_r_rules), quick_mode_r_rules,
		countof(quick_mode_r_order), quick_mode_r_order,
	},
	{TRANSACTION,		TRUE,	TRUE,
		countof(transaction_payload_rules_v1), transaction_payload_rules_v1,
		countof(transaction_payload_order_v1), transaction_payload_order_v1,
	},
	{TRANSACTION,		FALSE,	TRUE,
		countof(transaction_payload_rules_v1), transaction_payload_rules_v1,
		countof(transaction_payload_order_v1), transaction_payload_order_v1,
	},
	/* TODO-IKEv1: define rules for other exchanges */
#endif /* USE_IKEV1 */
};

/**
 * Data for fragment reassembly.
 */
typedef struct {

	/**
	 * For IKEv1 the number of the last fragment (in case we receive them out
	 * of order), since the first one starts with 1 this defines the number of
	 * fragments we expect.
	 * For IKEv2 we store the total number of fragment we received last.
	 */
	u_int16_t last;

	/**
	 * Length of all currently received fragments.
	 */
	size_t len;

	/**
	 * Maximum length of a fragmented packet.
	 */
	size_t max_packet;

} fragment_data_t;

typedef struct private_message_t private_message_t;

/**
 * Private data of an message_t object.
 */
struct private_message_t {

	/**
	 * Public part of a message_t object.
	 */
	message_t public;

	/**
	 * Minor version of message.
	 */
	u_int8_t major_version;

	/**
	 * Major version of message.
	 */
	u_int8_t minor_version;

	/**
	 * First Payload in message.
	 */
	payload_type_t first_payload;

	/**
	 * Assigned exchange type.
	 */
	exchange_type_t exchange_type;

	/**
	 * TRUE if message is a request, FALSE if a reply.
	 */
	bool is_request;

	/**
	 * The message is encrypted (IKEv1)
	 */
	bool is_encrypted;

	/**
	 * Higher version supported?
	 */
	bool version_flag;

	/**
	 * Reserved bits in IKE header
	 */
	bool reserved[2];

	/**
	 * Sorting of message disabled?
	 */
	bool sort_disabled;

	/**
	 * Message ID of this message.
	 */
	u_int32_t message_id;

	/**
	 * ID of assigned IKE_SA.
	 */
	ike_sa_id_t *ike_sa_id;

	/**
	 * Assigned UDP packet, stores incoming packet or last generated one.
	 */
	packet_t *packet;

	/**
	 * Array of generated fragments (if any), as packet_t*.
	 * If defragmenting (i.e. frag != NULL) this contains fragment_t*
	 */
	array_t *fragments;

	/**
	 * Linked List where payload data are stored in.
	 */
	linked_list_t *payloads;

	 /**
	  * Assigned parser to parse Header and Body of this message.
	  */
	parser_t *parser;

	/**
	 * The message rule for this message instance
	 */
	message_rule_t *rule;

	/**
	 * Data used to reassemble a fragmented message
	 */
	fragment_data_t *frag;
};

/**
 * Maximum number of fragments we will handle
 */
#define MAX_FRAGMENTS 255

/**
 * A single fragment within a fragmented message
 */
typedef struct {

	/** fragment number */
	u_int8_t num;

	/** fragment data */
	chunk_t data;

} fragment_t;

static void fragment_destroy(fragment_t *this)
{
	chunk_free(&this->data);
	free(this);
}

static void reset_defrag(private_message_t *this)
{
	array_destroy_function(this->fragments, (void*)fragment_destroy, NULL);
	this->fragments = NULL;
	this->frag->last = 0;
	this->frag->len = 0;
}

/**
 * Get the message rule that applies to this message
 */
static message_rule_t* get_message_rule(private_message_t *this)
{
	int i;

	for (i = 0; i < countof(message_rules); i++)
	{
		if ((this->exchange_type == message_rules[i].exchange_type) &&
			(this->is_request == message_rules[i].is_request))
		{
			return &message_rules[i];
		}
	}
	return NULL;
}

/**
 * Look up a payload rule
 */
static payload_rule_t* get_payload_rule(private_message_t *this,
										payload_type_t type)
{
	int i;

	for (i = 0; i < this->rule->rule_count;i++)
	{
		if (this->rule->rules[i].type == type)
		{
			return &this->rule->rules[i];
		}
	}
	return NULL;
}

METHOD(message_t, set_ike_sa_id, void,
	private_message_t *this,ike_sa_id_t *ike_sa_id)
{
	DESTROY_IF(this->ike_sa_id);
	this->ike_sa_id = ike_sa_id->clone(ike_sa_id);
}

METHOD(message_t, get_ike_sa_id, ike_sa_id_t*,
	private_message_t *this)
{
	return this->ike_sa_id;
}

METHOD(message_t, set_message_id, void,
	private_message_t *this,u_int32_t message_id)
{
	this->message_id = message_id;
}

METHOD(message_t, get_message_id, u_int32_t,
	private_message_t *this)
{
	return this->message_id;
}

METHOD(message_t, get_initiator_spi, u_int64_t,
	private_message_t *this)
{
	return (this->ike_sa_id->get_initiator_spi(this->ike_sa_id));
}

METHOD(message_t, get_responder_spi, u_int64_t,
	private_message_t *this)
{
	return (this->ike_sa_id->get_responder_spi(this->ike_sa_id));
}

METHOD(message_t, set_major_version, void,
	private_message_t *this, u_int8_t major_version)
{
	this->major_version = major_version;
}

METHOD(message_t, get_major_version, u_int8_t,
	private_message_t *this)
{
	return this->major_version;
}

METHOD(message_t, set_minor_version, void,
	private_message_t *this,u_int8_t minor_version)
{
	this->minor_version = minor_version;
}

METHOD(message_t, get_minor_version, u_int8_t,
	private_message_t *this)
{
	return this->minor_version;
}

METHOD(message_t, set_exchange_type, void,
	private_message_t *this, exchange_type_t exchange_type)
{
	this->exchange_type = exchange_type;
}

METHOD(message_t, get_exchange_type, exchange_type_t,
	private_message_t *this)
{
	return this->exchange_type;
}

METHOD(message_t, get_first_payload_type, payload_type_t,
	private_message_t *this)
{
	return this->first_payload;
}

METHOD(message_t, set_request, void,
	private_message_t *this, bool request)
{
	this->is_request = request;
}

METHOD(message_t, get_request, bool,
	private_message_t *this)
{
	return this->is_request;
}

METHOD(message_t, set_version_flag, void,
	private_message_t *this)
{
	this->version_flag = TRUE;
}

METHOD(message_t, get_reserved_header_bit, bool,
	private_message_t *this, u_int nr)
{
	if (nr < countof(this->reserved))
	{
		return this->reserved[nr];
	}
	return FALSE;
}

METHOD(message_t, set_reserved_header_bit, void,
	private_message_t *this, u_int nr)
{
	if (nr < countof(this->reserved))
	{
		this->reserved[nr] = TRUE;
	}
}

METHOD(message_t, is_encoded, bool,
	private_message_t *this)
{
	return this->packet->get_data(this->packet).ptr != NULL;
}

METHOD(message_t, is_fragmented, bool,
	private_message_t *this)
{
	return array_count(this->fragments) > 0;
}

METHOD(message_t, add_payload, void,
	private_message_t *this, payload_t *payload)
{
	payload_t *last_payload;

	if (this->payloads->get_count(this->payloads) > 0)
	{
		this->payloads->get_last(this->payloads, (void **)&last_payload);
		last_payload->set_next_type(last_payload, payload->get_type(payload));
	}
	else
	{
		this->first_payload = payload->get_type(payload);
	}
	payload->set_next_type(payload, PL_NONE);
	this->payloads->insert_last(this->payloads, payload);

	DBG2(DBG_ENC ,"added payload of type %N to message",
		 payload_type_names, payload->get_type(payload));
}

METHOD(message_t, add_notify, void,
	private_message_t *this, bool flush, notify_type_t type, chunk_t data)
{
	notify_payload_t *notify;
	payload_t *payload;

	if (flush)
	{
		while (this->payloads->remove_last(this->payloads,
												(void**)&payload) == SUCCESS)
		{
			payload->destroy(payload);
		}
	}
	if (this->major_version == IKEV2_MAJOR_VERSION)
	{
		notify = notify_payload_create(PLV2_NOTIFY);
	}
	else
	{
		notify = notify_payload_create(PLV1_NOTIFY);
	}
	notify->set_notify_type(notify, type);
	notify->set_notification_data(notify, data);
	add_payload(this, (payload_t*)notify);
}

METHOD(message_t, set_source, void,
	private_message_t *this, host_t *host)
{
	this->packet->set_source(this->packet, host);
}

METHOD(message_t, set_destination, void,
	private_message_t *this, host_t *host)
{
	this->packet->set_destination(this->packet, host);
}

METHOD(message_t, get_source, host_t*,
	private_message_t *this)
{
	return this->packet->get_source(this->packet);
}

METHOD(message_t, get_destination, host_t*,
	private_message_t *this)
{
	return this->packet->get_destination(this->packet);
}

METHOD(message_t, create_payload_enumerator, enumerator_t*,
	private_message_t *this)
{
	return this->payloads->create_enumerator(this->payloads);
}

METHOD(message_t, remove_payload_at, void,
	private_message_t *this, enumerator_t *enumerator)
{
	this->payloads->remove_at(this->payloads, enumerator);
}

METHOD(message_t, get_payload, payload_t*,
	private_message_t *this, payload_type_t type)
{
	payload_t *current, *found = NULL;
	enumerator_t *enumerator;

	enumerator = create_payload_enumerator(this);
	while (enumerator->enumerate(enumerator, &current))
	{
		if (current->get_type(current) == type)
		{
			found = current;
			break;
		}
	}
	enumerator->destroy(enumerator);
	return found;
}

METHOD(message_t, get_notify, notify_payload_t*,
	private_message_t *this, notify_type_t type)
{
	enumerator_t *enumerator;
	notify_payload_t *notify = NULL;
	payload_t *payload;

	enumerator = create_payload_enumerator(this);
	while (enumerator->enumerate(enumerator, &payload))
	{
		if (payload->get_type(payload) == PLV2_NOTIFY ||
			payload->get_type(payload) == PLV1_NOTIFY)
		{
			notify = (notify_payload_t*)payload;
			if (notify->get_notify_type(notify) == type)
			{
				break;
			}
			notify = NULL;
		}
	}
	enumerator->destroy(enumerator);
	return notify;
}

/**
 * get a string representation of the message
 */
static char* get_string(private_message_t *this, char *buf, int len)
{
	enumerator_t *enumerator;
	payload_t *payload;
	int written;
	char *pos = buf;

	memset(buf, 0, len);
	len--;

	written = snprintf(pos, len, "%N %s %u [",
					   exchange_type_names, this->exchange_type,
					   this->is_request ? "request" : "response",
					   this->message_id);
	if (written >= len || written < 0)
	{
		return "";
	}
	pos += written;
	len -= written;

	enumerator = create_payload_enumerator(this);
	while (enumerator->enumerate(enumerator, &payload))
	{
		written = snprintf(pos, len, " %N", payload_type_short_names,
						   payload->get_type(payload));
		if (written >= len || written < 0)
		{
			return buf;
		}
		pos += written;
		len -= written;
		if (payload->get_type(payload) == PLV2_NOTIFY ||
			payload->get_type(payload) == PLV1_NOTIFY)
		{
			notify_payload_t *notify;
			notify_type_t type;
			chunk_t data;

			notify = (notify_payload_t*)payload;
			type = notify->get_notify_type(notify);
			data = notify->get_notification_data(notify);
			if (type == MS_NOTIFY_STATUS && data.len == 4)
			{
				written = snprintf(pos, len, "(%N(%d))", notify_type_short_names,
								   type, untoh32(data.ptr));
			}
			else
			{
				written = snprintf(pos, len, "(%N)", notify_type_short_names,
								   type);
			}
			if (written >= len || written < 0)
			{
				return buf;
			}
			pos += written;
			len -= written;
		}
		if (payload->get_type(payload) == PLV2_EAP)
		{
			eap_payload_t *eap = (eap_payload_t*)payload;
			u_int32_t vendor;
			eap_type_t type;
			char method[64] = "";

			type = eap->get_type(eap, &vendor);
			if (type)
			{
				if (vendor)
				{
					snprintf(method, sizeof(method), "/%d-%d", type, vendor);
				}
				else
				{
					snprintf(method, sizeof(method), "/%N",
							 eap_type_short_names, type);
				}
			}
			written = snprintf(pos, len, "/%N%s", eap_code_short_names,
							   eap->get_code(eap), method);
			if (written >= len || written < 0)
			{
				return buf;
			}
			pos += written;
			len -= written;
		}
		if (payload->get_type(payload) == PLV2_CONFIGURATION ||
			payload->get_type(payload) == PLV1_CONFIGURATION)
		{
			cp_payload_t *cp = (cp_payload_t*)payload;
			enumerator_t *attributes;
			configuration_attribute_t *attribute;
			bool first = TRUE;
			char *pfx;

			switch (cp->get_type(cp))
			{
				case CFG_REQUEST:
					pfx = "RQ(";
					break;
				case CFG_REPLY:
					pfx = "RP(";
					break;
				case CFG_SET:
					pfx = "S(";
					break;
				case CFG_ACK:
					pfx = "A(";
					break;
				default:
					pfx = "(";
					break;
			}

			attributes = cp->create_attribute_enumerator(cp);
			while (attributes->enumerate(attributes, &attribute))
			{
				written = snprintf(pos, len, "%s%N", first ? pfx : " ",
								   configuration_attribute_type_short_names,
								   attribute->get_type(attribute));
				if (written >= len || written < 0)
				{
					return buf;
				}
				pos += written;
				len -= written;
				first = FALSE;
			}
			attributes->destroy(attributes);
			if (!first)
			{
				written = snprintf(pos, len, ")");
				if (written >= len || written < 0)
				{
					return buf;
				}
				pos += written;
				len -= written;
			}
		}
	}
	enumerator->destroy(enumerator);

	/* remove last space */
	snprintf(pos, len, " ]");
	return buf;
}

METHOD(message_t, disable_sort, void,
	private_message_t *this)
{
	this->sort_disabled = TRUE;
}

/**
 * reorder payloads depending on reordering rules
 */
static void order_payloads(private_message_t *this)
{
	linked_list_t *list;
	payload_t *payload;
	int i;

	DBG2(DBG_ENC, "order payloads in message");

	/* move to temp list */
	list = linked_list_create();
	while (this->payloads->remove_last(this->payloads,
									   (void**)&payload) == SUCCESS)
	{
		list->insert_first(list, payload);
	}
	/* for each rule, ... */
	for (i = 0; i < this->rule->order_count; i++)
	{
		enumerator_t *enumerator;
		notify_payload_t *notify;
		payload_order_t order;

		order = this->rule->order[i];

		/* ... find all payload ... */
		enumerator = list->create_enumerator(list);
		while (enumerator->enumerate(enumerator, &payload))
		{
			/* ... with that type ... */
			if (payload->get_type(payload) == order.type)
			{
				notify = (notify_payload_t*)payload;

				/**... and check notify for type. */
				if (order.type != PLV2_NOTIFY || order.notify == 0 ||
					order.notify == notify->get_notify_type(notify))
				{
					list->remove_at(list, enumerator);
					add_payload(this, payload);
				}
			}
		}
		enumerator->destroy(enumerator);
	}
	/* append all payloads without a rule to the end */
	while (list->remove_last(list, (void**)&payload) == SUCCESS)
	{
		/* do not complain about payloads in private use space */
		if (payload->get_type(payload) < 128)
		{
			DBG1(DBG_ENC, "payload %N has no ordering rule in %N %s",
				 payload_type_names, payload->get_type(payload),
				 exchange_type_names, this->rule->exchange_type,
				 this->rule->is_request ? "request" : "response");
		}
		add_payload(this, payload);
	}
	list->destroy(list);
}

/**
 * Wrap payloads in an encrypted payload
 */
static encrypted_payload_t* wrap_payloads(private_message_t *this)
{
	encrypted_payload_t *encrypted = NULL;
	linked_list_t *payloads;
	payload_t *current;

	/* move all payloads to a temporary list */
	payloads = linked_list_create();
	while (this->payloads->remove_first(this->payloads,
										(void**)&current) == SUCCESS)
	{
		if (current->get_type(current) == PLV2_FRAGMENT)
		{	/* treat encrypted fragment payload as encrypted payload */
			encrypted = (encrypted_payload_t*)current;
		}
		else
		{
			payloads->insert_last(payloads, current);
		}
	}
	if (encrypted)
	{	/* simply adopt all the unencrypted payloads */
		this->payloads->destroy(this->payloads);
		this->payloads = payloads;
		return encrypted;
	}

	if (this->is_encrypted)
	{
		encrypted = encrypted_payload_create(PLV1_ENCRYPTED);
	}
	else
	{
		encrypted = encrypted_payload_create(PLV2_ENCRYPTED);
	}
	while (payloads->remove_first(payloads, (void**)&current) == SUCCESS)
	{
		payload_rule_t *rule;
		payload_type_t type;
		bool encrypt = TRUE;

		type = current->get_type(current);
		rule = get_payload_rule(this, type);
		if (rule)
		{
			encrypt = rule->encrypted;
		}
		if (encrypt || this->is_encrypted)
		{	/* encryption is forced for IKEv1 */
			DBG2(DBG_ENC, "insert payload %N into encrypted payload",
				 payload_type_names, type);
			encrypted->add_payload(encrypted, current);
		}
		else
		{
			DBG2(DBG_ENC, "insert payload %N unencrypted",
				 payload_type_names, type);
			add_payload(this, current);
		}
	}
	payloads->destroy(payloads);

	return encrypted;
}

/**
 * Creates the IKE header for this message
 */
static ike_header_t *create_header(private_message_t *this)
{
	ike_header_t *ike_header;
	bool *reserved;
	int i;

	ike_header = ike_header_create_version(this->major_version,
										   this->minor_version);
	ike_header->set_exchange_type(ike_header, this->exchange_type);
	ike_header->set_message_id(ike_header, this->message_id);
	if (this->major_version == IKEV2_MAJOR_VERSION)
	{
		ike_header->set_response_flag(ike_header, !this->is_request);
		ike_header->set_version_flag(ike_header, this->version_flag);
		ike_header->set_initiator_flag(ike_header,
						this->ike_sa_id->is_initiator(this->ike_sa_id));
	}
	else
	{
		ike_header->set_encryption_flag(ike_header, this->is_encrypted);
	}
	ike_header->set_initiator_spi(ike_header,
						this->ike_sa_id->get_initiator_spi(this->ike_sa_id));
	ike_header->set_responder_spi(ike_header,
						this->ike_sa_id->get_responder_spi(this->ike_sa_id));

	for (i = 0; i < countof(this->reserved); i++)
	{
		reserved = payload_get_field(&ike_header->payload_interface,
									 RESERVED_BIT, i);
		if (reserved)
		{
			*reserved = this->reserved[i];
		}
	}
	return ike_header;
}

/**
 * Generates the message, if needed, wraps the payloads in an encrypted payload.
 *
 * The generator and the possible enrypted payload are returned.  The latter
 * is not yet encrypted (but the transform is set).  It is also not added to
 * the payload list (so unless there are unencrypted payloads that list will
 * be empty afterwards).
 */
static status_t generate_message(private_message_t *this, keymat_t *keymat,
				generator_t **out_generator, encrypted_payload_t **encrypted)
{
	keymat_v1_t *keymat_v1 = (keymat_v1_t*)keymat;
	generator_t *generator;
	payload_type_t next_type;
	enumerator_t *enumerator;
	aead_t *aead = NULL;
	chunk_t hash = chunk_empty;
	char str[BUF_LEN];
	ike_header_t *ike_header;
	payload_t *payload, *next;
	bool encrypting = FALSE;

	if (this->exchange_type == EXCHANGE_TYPE_UNDEFINED)
	{
		DBG1(DBG_ENC, "exchange type is not defined");
		return INVALID_STATE;
	}

	if (this->packet->get_source(this->packet) == NULL ||
		this->packet->get_destination(this->packet) == NULL)
	{
		DBG1(DBG_ENC, "source/destination not defined");
		return INVALID_STATE;
	}

	this->rule = get_message_rule(this);
	if (!this->rule)
	{
		DBG1(DBG_ENC, "no message rules specified for this message type");
		return NOT_SUPPORTED;
	}

	if (!this->sort_disabled)
	{
		order_payloads(this);
	}

	if (keymat && keymat->get_version(keymat) == IKEV1)
	{
		/* get a hash for this message, if any is required */
		if (keymat_v1->get_hash_phase2(keymat_v1, &this->public, &hash))
		{	/* insert a HASH payload as first payload */
			hash_payload_t *hash_payload;

			hash_payload = hash_payload_create(PLV1_HASH);
			hash_payload->set_hash(hash_payload, hash);
			this->payloads->insert_first(this->payloads, hash_payload);
			if (this->exchange_type == INFORMATIONAL_V1)
			{
				this->is_encrypted = encrypting = TRUE;
			}
			chunk_free(&hash);
		}
	}

	if (this->major_version == IKEV2_MAJOR_VERSION)
	{
		encrypting = this->rule->encrypted;
	}
	else if (!encrypting)
	{
		/* If at least one payload requires encryption, encrypt the message.
		 * If no key material is available, the flag will be reset below. */
		enumerator = this->payloads->create_enumerator(this->payloads);
		while (enumerator->enumerate(enumerator, (void**)&payload))
		{
			payload_rule_t *rule;

			rule = get_payload_rule(this, payload->get_type(payload));
			if (rule && rule->encrypted)
			{
				this->is_encrypted = encrypting = TRUE;
				break;
			}
		}
		enumerator->destroy(enumerator);
	}

	DBG1(DBG_ENC, "generating %s", get_string(this, str, sizeof(str)));

	if (keymat)
	{
		aead = keymat->get_aead(keymat, FALSE);
	}
	if (aead && encrypting)
	{
		*encrypted = wrap_payloads(this);
		(*encrypted)->set_transform(*encrypted, aead);
	}
	else
	{
		DBG2(DBG_ENC, "not encrypting payloads");
		this->is_encrypted = FALSE;
	}

	/* generate all payloads with proper next type */
	*out_generator = generator = generator_create();
	ike_header = create_header(this);
	payload = (payload_t*)ike_header;
	enumerator = create_payload_enumerator(this);
	while (enumerator->enumerate(enumerator, &next))
	{
		payload->set_next_type(payload, next->get_type(next));
		generator->generate_payload(generator, payload);
		payload = next;
	}
	enumerator->destroy(enumerator);

	next_type = PL_NONE;
	if (this->is_encrypted)
	{	/* for encrypted IKEv1 messages */
		next_type = (*encrypted)->payload_interface.get_next_type(
														(payload_t*)*encrypted);
	}
	else if (*encrypted)
	{	/* use proper IKEv2 encrypted (fragment) payload type */
		next_type = (*encrypted)->payload_interface.get_type(
														(payload_t*)*encrypted);
	}
	payload->set_next_type(payload, next_type);
	generator->generate_payload(generator, payload);
	ike_header->destroy(ike_header);
	return SUCCESS;
}

/**
 * Encrypts and adds the encrypted payload (if any) to the payload list and
 * finalizes the message generation.  Destroys the given generator.
 */
static status_t finalize_message(private_message_t *this, keymat_t *keymat,
						generator_t *generator, encrypted_payload_t *encrypted)
{
	keymat_v1_t *keymat_v1 = (keymat_v1_t*)keymat;
	chunk_t chunk;
	u_int32_t *lenpos;

	if (encrypted)
	{
		if (this->is_encrypted)
		{	/* for IKEv1 instead of associated data we provide the IV */
			if (!keymat_v1->get_iv(keymat_v1, this->message_id, &chunk))
			{
				generator->destroy(generator);
				encrypted->destroy(encrypted);
				return FAILED;
			}
		}
		else
		{	/* build associated data (without header of encrypted payload) */
			chunk = generator->get_chunk(generator, &lenpos);
			/* fill in length, including encrypted payload */
			htoun32(lenpos, chunk.len + encrypted->get_length(encrypted));
		}
		this->payloads->insert_last(this->payloads, encrypted);
		if (encrypted->encrypt(encrypted, this->message_id, chunk) != SUCCESS)
		{
			generator->destroy(generator);
			return INVALID_STATE;
		}
		generator->generate_payload(generator, &encrypted->payload_interface);
	}
	chunk = generator->get_chunk(generator, &lenpos);
	htoun32(lenpos, chunk.len);
	this->packet->set_data(this->packet, chunk_clone(chunk));
	if (this->is_encrypted && this->exchange_type != INFORMATIONAL_V1)
	{
		/* update the IV for the next IKEv1 message */
		chunk_t last_block;
		aead_t *aead;
		size_t bs;

		aead = keymat->get_aead(keymat, FALSE);
		bs = aead->get_block_size(aead);
		last_block = chunk_create(chunk.ptr + chunk.len - bs, bs);
		if (!keymat_v1->update_iv(keymat_v1, this->message_id, last_block) ||
			!keymat_v1->confirm_iv(keymat_v1, this->message_id))
		{
			generator->destroy(generator);
			return FAILED;
		}
	}
	generator->destroy(generator);
	return SUCCESS;
}

METHOD(message_t, generate, status_t,
	private_message_t *this, keymat_t *keymat, packet_t **packet)
{
	generator_t *generator = NULL;
	encrypted_payload_t *encrypted = NULL;
	status_t status;

	status = generate_message(this, keymat, &generator, &encrypted);
	if (status != SUCCESS)
	{
		DESTROY_IF(generator);
		return status;
	}
	status = finalize_message(this, keymat, generator, encrypted);
	if (status != SUCCESS)
	{
		return status;
	}
	if (packet)
	{
		*packet = this->packet->clone(this->packet);
	}
	return SUCCESS;
}

/**
 * Creates a (basic) clone of the given message
 */
static message_t *clone_message(private_message_t *this)
{
	message_t *message;
	host_t *src, *dst;

	src = this->packet->get_source(this->packet);
	dst = this->packet->get_destination(this->packet);

	message = message_create(this->major_version, this->minor_version);
	message->set_ike_sa_id(message, this->ike_sa_id);
	message->set_message_id(message, this->message_id);
	message->set_request(message, this->is_request);
	message->set_source(message, src->clone(src));
	message->set_destination(message, dst->clone(dst));
	message->set_exchange_type(message, this->exchange_type);
	memcpy(((private_message_t*)message)->reserved, this->reserved,
		   sizeof(this->reserved));
	return message;
}

/**
 * Create a single fragment with the given data
 */
static message_t *create_fragment(private_message_t *this, payload_type_t next,
								  u_int16_t num, u_int16_t count, chunk_t data)
{
	enumerator_t *enumerator;
	payload_t *fragment, *payload;
	message_t *message;
	peer_cfg_t *peer_cfg;
	ike_sa_t *ike_sa;

	message = clone_message(this);
	if (this->major_version == IKEV1_MAJOR_VERSION)
	{
		/* other implementations seem to just use 0 as message ID, so here we go */
		message->set_message_id(message, 0);
		/* always use the initial message type for fragments, even for quick mode
		 * or transaction messages. */
		ike_sa = charon->bus->get_sa(charon->bus);
		if (ike_sa && (peer_cfg = ike_sa->get_peer_cfg(ike_sa)) &&
			peer_cfg->use_aggressive(peer_cfg))
		{
			message->set_exchange_type(message, AGGRESSIVE);
		}
		else
		{
			message->set_exchange_type(message, ID_PROT);
		}
		fragment = (payload_t*)fragment_payload_create_from_data(
													num, num == count, data);
	}
	else
	{
		fragment = (payload_t*)encrypted_fragment_payload_create_from_data(
													num, count, data);
		if (num == 1)
		{
			/* only in the first fragment is this set to the type of the first
			 * payload in the encrypted payload */
			fragment->set_next_type(fragment, next);
			/* move unencrypted payloads to the first fragment */
			enumerator = this->payloads->create_enumerator(this->payloads);
			while (enumerator->enumerate(enumerator, &payload))
			{
				if (payload->get_type(payload) != PLV2_ENCRYPTED)
				{
					this->payloads->remove_at(this->payloads, enumerator);
					message->add_payload(message, payload);
				}
			}
			enumerator->destroy(enumerator);
		}
	}
	message->add_payload(message, (payload_t*)fragment);
	return message;
}

/**
 * Destroy all fragments
 */
static void clear_fragments(private_message_t *this)
{
	array_destroy_offset(this->fragments, offsetof(packet_t, destroy));
	this->fragments = NULL;
}

/**
 * Reduce the fragment length but ensure it stays > 0
 */
#define REDUCE_FRAG_LEN(fl, amount) ({ \
	fl = max(1, (ssize_t)fl - (amount)); \
})

METHOD(message_t, fragment, status_t,
	private_message_t *this, keymat_t *keymat, size_t frag_len,
	enumerator_t **fragments)
{
	encrypted_payload_t *encrypted = NULL;
	generator_t *generator = NULL;
	message_t *fragment;
	packet_t *packet;
	payload_type_t next = PL_NONE;
	u_int16_t num, count;
	host_t *src, *dst;
	chunk_t data;
	status_t status;
	u_int32_t *lenpos;
	size_t len;

	src = this->packet->get_source(this->packet);
	dst = this->packet->get_destination(this->packet);
	if (!frag_len)
	{
		frag_len = (src->get_family(src) == AF_INET) ? 576 : 1280;
	}
	/* frag_len is the complete IP datagram length, account for overhead (we
	 * assume no IP options/extension headers are used) */
	REDUCE_FRAG_LEN(frag_len, (src->get_family(src) == AF_INET) ? 20 : 40);
	/* 8 (UDP header) */
	REDUCE_FRAG_LEN(frag_len, 8);
	if (dst->get_port(dst) != IKEV2_UDP_PORT &&
		src->get_port(src) != IKEV2_UDP_PORT)
	{	/* reduce length due to non-ESP marker */
		REDUCE_FRAG_LEN(frag_len, 4);
	}

	if (is_encoded(this))
	{
		if (this->major_version == IKEV2_MAJOR_VERSION)
		{
			encrypted = (encrypted_payload_t*)get_payload(this, PLV2_ENCRYPTED);
		}
		data = this->packet->get_data(this->packet);
		len = data.len;
	}
	else
	{
		status = generate_message(this, keymat, &generator, &encrypted);
		if (status != SUCCESS)
		{
			DESTROY_IF(generator);
			return status;
		}
		data = generator->get_chunk(generator, &lenpos);
		len = data.len + (encrypted ? encrypted->get_length(encrypted) : 0);
	}

	/* check if we actually need to fragment the message and if we have an
	 * encrypted payload for IKEv2 */
	if (len <= frag_len ||
	   (this->major_version == IKEV2_MAJOR_VERSION && !encrypted))
	{
		if (generator)
		{
			status = finalize_message(this, keymat, generator, encrypted);
			if (status != SUCCESS)
			{
				return status;
			}
		}
		*fragments = enumerator_create_single(this->packet, NULL);
		return SUCCESS;
	}

	/* frag_len denoted the maximum IKE message size so far, later on it will
	 * denote the maximum content size of a fragment payload, therefore,
	 * account for IKE header */
	REDUCE_FRAG_LEN(frag_len, 28);

	if (this->major_version == IKEV1_MAJOR_VERSION)
	{
		if (generator)
		{
			status = finalize_message(this, keymat, generator, encrypted);
			if (status != SUCCESS)
			{
				return status;
			}
			data = this->packet->get_data(this->packet);
			generator = NULL;
		}
		/* overhead for the fragmentation payload header */
		REDUCE_FRAG_LEN(frag_len, 8);
	}
	else
	{
		aead_t *aead;

		if (generator)
		{
			generator->destroy(generator);
			generator = generator_create();
		}
		else
		{	/* do not log again if it was generated previously */
			generator = generator_create_no_dbg();
		}
		next = encrypted->payload_interface.get_next_type((payload_t*)encrypted);
		encrypted->generate_payloads(encrypted, generator);
		data = generator->get_chunk(generator, &lenpos);
		if (!is_encoded(this))
		{
			encrypted->destroy(encrypted);
		}
		aead = keymat->get_aead(keymat, FALSE);
		/* overhead for the encrypted fragment payload */
		REDUCE_FRAG_LEN(frag_len, aead->get_iv_size(aead));
		REDUCE_FRAG_LEN(frag_len, aead->get_icv_size(aead));
		/* header */
		REDUCE_FRAG_LEN(frag_len, 8);
		/* padding and padding length */
		frag_len = round_down(frag_len, aead->get_block_size(aead));
		REDUCE_FRAG_LEN(frag_len, 1);
		/* TODO-FRAG: if there are unencrypted payloads, should we account for
		 * their length in the first fragment? we still would have to add
		 * an encrypted fragment payload (albeit empty), even so we couldn't
		 * prevent IP fragmentation in every case */
	}

	count = data.len / frag_len + (data.len % frag_len ? 1 : 0);
	this->fragments = array_create(0, count);
	DBG1(DBG_ENC, "splitting IKE message with length of %zu bytes into "
		 "%hu fragments", len, count);
	for (num = 1; num <= count; num++)
	{
		len = min(data.len, frag_len);
		fragment = create_fragment(this, next, num, count,
								   chunk_create(data.ptr, len));
		status = fragment->generate(fragment, keymat, &packet);
		fragment->destroy(fragment);
		if (status != SUCCESS)
		{
			DBG1(DBG_ENC, "failed to generate IKE fragment");
			clear_fragments(this);
			DESTROY_IF(generator);
			return FAILED;
		}
		array_insert(this->fragments, ARRAY_TAIL, packet);
		data = chunk_skip(data, len);
	}
	*fragments = array_create_enumerator(this->fragments);
	DESTROY_IF(generator);
	return SUCCESS;
}

METHOD(message_t, get_packet, packet_t*,
	private_message_t *this)
{
	return this->packet->clone(this->packet);
}

METHOD(message_t, get_packet_data, chunk_t,
	private_message_t *this)
{
	return this->packet->get_data(this->packet);
}

METHOD(message_t, get_fragments, enumerator_t*,
	private_message_t *this)
{
	return array_create_enumerator(this->fragments);
}

METHOD(message_t, parse_header, status_t,
	private_message_t *this)
{
	ike_header_t *ike_header;
	status_t status;
	bool *reserved;
	int i;

	DBG2(DBG_ENC, "parsing header of message");

	if (!this->parser)
	{	/* reassembled IKEv2 message, header is inherited from fragments */
		return SUCCESS;
	}
	this->parser->reset_context(this->parser);
	status = this->parser->parse_payload(this->parser, PL_HEADER,
										 (payload_t**)&ike_header);
	if (status != SUCCESS)
	{
		DBG1(DBG_ENC, "header could not be parsed");
		return status;

	}

	status = ike_header->payload_interface.verify(
										&ike_header->payload_interface);
	if (status != SUCCESS)
	{
		DBG1(DBG_ENC, "header verification failed");
		ike_header->destroy(ike_header);
		return status;
	}

	DESTROY_IF(this->ike_sa_id);
	this->ike_sa_id = ike_sa_id_create(
									ike_header->get_maj_version(ike_header),
									ike_header->get_initiator_spi(ike_header),
									ike_header->get_responder_spi(ike_header),
									ike_header->get_initiator_flag(ike_header));

	this->exchange_type = ike_header->get_exchange_type(ike_header);
	this->message_id = ike_header->get_message_id(ike_header);
	this->major_version = ike_header->get_maj_version(ike_header);
	this->minor_version = ike_header->get_min_version(ike_header);
	if (this->major_version == IKEV2_MAJOR_VERSION)
	{
		this->is_request = !ike_header->get_response_flag(ike_header);
	}
	else
	{
		this->is_encrypted = ike_header->get_encryption_flag(ike_header);
	}
	this->first_payload = ike_header->payload_interface.get_next_type(
												&ike_header->payload_interface);
	if (this->first_payload == PLV1_FRAGMENT && this->is_encrypted)
	{	/* racoon sets the encrypted bit when sending a fragment, but these
		 * messages are really not encrypted */
		this->is_encrypted = FALSE;
	}

	for (i = 0; i < countof(this->reserved); i++)
	{
		reserved = payload_get_field(&ike_header->payload_interface,
									 RESERVED_BIT, i);
		if (reserved)
		{
			this->reserved[i] = *reserved;
		}
	}
	ike_header->destroy(ike_header);

	DBG2(DBG_ENC, "parsed a %N %s header", exchange_type_names,
		 this->exchange_type, this->major_version == IKEV1_MAJOR_VERSION ?
		 "message" : (this->is_request ? "request" : "response"));
	return SUCCESS;
}

/**
 * Check if a payload is for a mediation extension connectivity check
 */
static bool is_connectivity_check(private_message_t *this, payload_t *payload)
{
#ifdef ME
	if (this->exchange_type == INFORMATIONAL &&
		payload->get_type(payload) == PLV2_NOTIFY)
	{
		notify_payload_t *notify = (notify_payload_t*)payload;

		switch (notify->get_notify_type(notify))
		{
			case ME_CONNECTID:
			case ME_ENDPOINT:
			case ME_CONNECTAUTH:
				return TRUE;
			default:
				break;
		}
	}
#endif /* !ME */
	return FALSE;
}

/**
 * Parses and verifies the unencrypted payloads contained in the message
 */
static status_t parse_payloads(private_message_t *this)
{
	payload_type_t type = this->first_payload;
	payload_t *payload;
	status_t status;

	if (this->is_encrypted)
	{	/* wrap the whole encrypted IKEv1 message in a special encrypted
		 * payload which is then handled just like a regular payload */
		encrypted_payload_t *encryption;

		status = this->parser->parse_payload(this->parser, PLV1_ENCRYPTED,
											 (payload_t**)&encryption);
		if (status != SUCCESS)
		{
			DBG1(DBG_ENC, "failed to wrap encrypted IKEv1 message");
			return PARSE_ERROR;
		}
		encryption->payload_interface.set_next_type((payload_t*)encryption,
													this->first_payload);
		this->payloads->insert_last(this->payloads, encryption);
		return SUCCESS;
	}

	while (type != PL_NONE)
	{
		DBG2(DBG_ENC, "starting parsing a %N payload",
			 payload_type_names, type);

		status = this->parser->parse_payload(this->parser, type, &payload);
		if (status != SUCCESS)
		{
			DBG1(DBG_ENC, "payload type %N could not be parsed",
				 payload_type_names, type);
			return PARSE_ERROR;
		}

		DBG2(DBG_ENC, "verifying payload of type %N", payload_type_names, type);
		status = payload->verify(payload);
		if (status != SUCCESS)
		{
			DBG1(DBG_ENC, "%N payload verification failed",
				 payload_type_names, type);
			payload->destroy(payload);
			return VERIFY_ERROR;
		}

		DBG2(DBG_ENC, "%N payload verified, adding to payload list",
			 payload_type_names, type);
		this->payloads->insert_last(this->payloads, payload);

		/* an encrypted (fragment) payload MUST be the last one, so STOP here.
		 * decryption is done later */
		if (type == PLV2_ENCRYPTED || type == PLV2_FRAGMENT)
		{
			DBG2(DBG_ENC, "%N payload found, stop parsing",
				 payload_type_names, type);
			break;
		}
		type = payload->get_next_type(payload);
	}
	return SUCCESS;
}

/**
 * Decrypt an encrypted payload and extract all contained payloads.
 */
static status_t decrypt_and_extract(private_message_t *this, keymat_t *keymat,
						payload_t *previous, encrypted_payload_t *encryption)
{
	payload_t *encrypted;
	payload_type_t type;
	chunk_t chunk;
	aead_t *aead;
	size_t bs;
	status_t status = SUCCESS;

	if (!keymat)
	{
		DBG1(DBG_ENC, "found encrypted payload, but no keymat");
		return INVALID_ARG;
	}
	aead = keymat->get_aead(keymat, TRUE);
	if (!aead)
	{
		DBG1(DBG_ENC, "found encrypted payload, but no transform set");
		return INVALID_ARG;
	}
	if (!this->parser)
	{
		/* reassembled IKEv2 messages are already decrypted, we still call
		 * decrypt() to parse the contained payloads */
		status = encryption->decrypt(encryption, chunk_empty);
	}
	else
	{
		bs = aead->get_block_size(aead);
		encryption->set_transform(encryption, aead);
		chunk = this->packet->get_data(this->packet);
		if (chunk.len < encryption->get_length(encryption) ||
			chunk.len < bs)
		{
			DBG1(DBG_ENC, "invalid payload length");
			return VERIFY_ERROR;
		}
		if (keymat->get_version(keymat) == IKEV1)
		{	/* instead of associated data we provide the IV, we also update
			 * the IV with the last encrypted block */
			keymat_v1_t *keymat_v1 = (keymat_v1_t*)keymat;
			chunk_t iv;

			if (keymat_v1->get_iv(keymat_v1, this->message_id, &iv))
			{
				status = encryption->decrypt(encryption, iv);
				if (status == SUCCESS)
				{
					if (!keymat_v1->update_iv(keymat_v1, this->message_id,
							chunk_create(chunk.ptr + chunk.len - bs, bs)))
					{
						status = FAILED;
					}
				}
			}
			else
			{
				status = FAILED;
			}
		}
		else
		{
			chunk.len -= encryption->get_length(encryption);
			status = encryption->decrypt(encryption, chunk);
		}
	}
	if (status != SUCCESS)
	{
		return status;
	}

	while ((encrypted = encryption->remove_payload(encryption)))
	{
		type = encrypted->get_type(encrypted);
		if (previous)
		{
			previous->set_next_type(previous, type);
		}
		else
		{
			this->first_payload = type;
		}
		DBG2(DBG_ENC, "insert decrypted payload of type %N at end of list",
			 payload_type_names, type);
		this->payloads->insert_last(this->payloads, encrypted);
		previous = encrypted;
	}
	return SUCCESS;
}

/**
 * Decrypt an encrypted fragment payload.
 */
static status_t decrypt_fragment(private_message_t *this, keymat_t *keymat,
								 encrypted_fragment_payload_t *fragment)
{
	encrypted_payload_t *encrypted = (encrypted_payload_t*)fragment;
	chunk_t chunk;
	aead_t *aead;
	size_t bs;

	if (!keymat)
	{
		DBG1(DBG_ENC, "found encrypted fragment payload, but no keymat");
		return INVALID_ARG;
	}
	aead = keymat->get_aead(keymat, TRUE);
	if (!aead)
	{
		DBG1(DBG_ENC, "found encrypted fragment payload, but no transform set");
		return INVALID_ARG;
	}
	bs = aead->get_block_size(aead);
	encrypted->set_transform(encrypted, aead);
	chunk = this->packet->get_data(this->packet);
	if (chunk.len < encrypted->get_length(encrypted) ||
		chunk.len < bs)
	{
		DBG1(DBG_ENC, "invalid payload length");
		return VERIFY_ERROR;
	}
	chunk.len -= encrypted->get_length(encrypted);
	return encrypted->decrypt(encrypted, chunk);
}

/**
 * Do we accept unencrypted ID/HASH payloads in Main Mode, as seen from
 * some SonicWall boxes?
 */
static bool accept_unencrypted_mm(private_message_t *this, payload_type_t type)
{
	if (this->exchange_type == ID_PROT)
	{
		if (type == PLV1_ID || type == PLV1_HASH)
		{
			return lib->settings->get_bool(lib->settings,
									"%s.accept_unencrypted_mainmode_messages",
									FALSE, lib->ns);
		}
	}
	return FALSE;
}

/**
 * Decrypt payload from the encrypted payload
 */
static status_t decrypt_payloads(private_message_t *this, keymat_t *keymat)
{
	payload_t *payload, *previous = NULL;
	enumerator_t *enumerator;
	payload_rule_t *rule;
	payload_type_t type;
	status_t status = SUCCESS;
	char *was_encrypted = NULL;

	enumerator = this->payloads->create_enumerator(this->payloads);
	while (enumerator->enumerate(enumerator, &payload))
	{
		type = payload->get_type(payload);

		DBG2(DBG_ENC, "process payload of type %N", payload_type_names, type);

		if (type == PLV2_ENCRYPTED || type == PLV1_ENCRYPTED ||
			type == PLV2_FRAGMENT)
		{
			if (was_encrypted)
			{
				DBG1(DBG_ENC, "%s can't contain other payloads of type %N",
					 was_encrypted, payload_type_names, type);
				status = VERIFY_ERROR;
				break;
			}
		}

		if (type == PLV2_ENCRYPTED || type == PLV1_ENCRYPTED)
		{
			encrypted_payload_t *encryption;

			DBG2(DBG_ENC, "found an encrypted payload");
			encryption = (encrypted_payload_t*)payload;
			this->payloads->remove_at(this->payloads, enumerator);

			if (enumerator->enumerate(enumerator, NULL))
			{
				DBG1(DBG_ENC, "encrypted payload is not last payload");
				encryption->destroy(encryption);
				status = VERIFY_ERROR;
				break;
			}
			status = decrypt_and_extract(this, keymat, previous, encryption);
			encryption->destroy(encryption);
			if (status != SUCCESS)
			{
				break;
			}
			was_encrypted = "encrypted payload";
		}
		else if (type == PLV2_FRAGMENT)
		{
			encrypted_fragment_payload_t *fragment;

			DBG2(DBG_ENC, "found an encrypted fragment payload");
			fragment = (encrypted_fragment_payload_t*)payload;

			if (enumerator->enumerate(enumerator, NULL))
			{
				DBG1(DBG_ENC, "encrypted fragment payload is not last payload");
				status = VERIFY_ERROR;
				break;
			}
			status = decrypt_fragment(this, keymat, fragment);
			if (status != SUCCESS)
			{
				break;
			}
			was_encrypted = "encrypted fragment payload";
		}

		if (payload_is_known(type) && !was_encrypted &&
			!is_connectivity_check(this, payload) &&
			this->exchange_type != AGGRESSIVE)
		{
			rule = get_payload_rule(this, type);
			if ((!rule || rule->encrypted) &&
				!accept_unencrypted_mm(this, type))
			{
				DBG1(DBG_ENC, "payload type %N was not encrypted",
					 payload_type_names, type);
				status = FAILED;
				break;
			}
		}
		previous = payload;
	}
	enumerator->destroy(enumerator);
	return status;
}

/**
 * Verify a message and all payload according to message/payload rules
 */
static status_t verify(private_message_t *this)
{
	bool complete = FALSE;
	int i;

	DBG2(DBG_ENC, "verifying message structure");

	/* check for payloads with wrong count */
	for (i = 0; i < this->rule->rule_count; i++)
	{
		enumerator_t *enumerator;
		payload_t *payload;
		payload_rule_t *rule;
		int found = 0;

		rule = &this->rule->rules[i];
		enumerator = create_payload_enumerator(this);
		while (enumerator->enumerate(enumerator, &payload))
		{
			payload_type_t type;

			type = payload->get_type(payload);
			if (type == rule->type)
			{
				found++;
				DBG2(DBG_ENC, "found payload of type %N",
					 payload_type_names, type);
				if (found > rule->max_occurence)
				{
					DBG1(DBG_ENC, "payload of type %N more than %d times (%d) "
						 "occurred in current message", payload_type_names,
						 type, rule->max_occurence, found);
					enumerator->destroy(enumerator);
					return VERIFY_ERROR;
				}
			}
		}
		enumerator->destroy(enumerator);

		if (!complete && found < rule->min_occurence)
		{
			DBG1(DBG_ENC, "payload of type %N not occurred %d times (%d)",
				 payload_type_names, rule->type, rule->min_occurence, found);
			return VERIFY_ERROR;
		}
		if (found && rule->sufficient)
		{
			complete = TRUE;
		}
	}
	return SUCCESS;
}

METHOD(message_t, parse_body, status_t,
	private_message_t *this, keymat_t *keymat)
{
	status_t status = SUCCESS;
	char str[BUF_LEN];

	DBG2(DBG_ENC, "parsing body of message, first payload is %N",
		 payload_type_names, this->first_payload);

	this->rule = get_message_rule(this);
	if (!this->rule)
	{
		DBG1(DBG_ENC, "no message rules specified for a %N %s",
			 exchange_type_names, this->exchange_type,
			 this->is_request ? "request" : "response");
		return NOT_SUPPORTED;
	}

	/* reassembled IKEv2 messages are already parsed (except for the payloads
	 * contained in the encrypted payload, which are handled below) */
	if (this->parser)
	{
		status = parse_payloads(this);
		if (status != SUCCESS)
		{	/* error is already logged */
			return status;
		}
	}

	status = decrypt_payloads(this, keymat);
	if (status != SUCCESS)
	{
		DBG1(DBG_ENC, "could not decrypt payloads");
		return status;
	}

	status = verify(this);
	if (status != SUCCESS)
	{
		return status;
	}

	DBG1(DBG_ENC, "parsed %s", get_string(this, str, sizeof(str)));

	if (keymat && keymat->get_version(keymat) == IKEV1)
	{
		keymat_v1_t *keymat_v1 = (keymat_v1_t*)keymat;
		chunk_t hash;

		if (keymat_v1->get_hash_phase2(keymat_v1, &this->public, &hash))
		{
			hash_payload_t *hash_payload;
			chunk_t other_hash;

			if (this->first_payload != PLV1_HASH)
			{
				if (this->exchange_type == INFORMATIONAL_V1)
				{
					DBG1(DBG_ENC, "ignoring unprotected INFORMATIONAL from %H",
						 this->packet->get_source(this->packet));
				}
				else
				{
					DBG1(DBG_ENC, "expected HASH payload as first payload");
				}
				chunk_free(&hash);
				return VERIFY_ERROR;
			}
			hash_payload = (hash_payload_t*)get_payload(this, PLV1_HASH);
			other_hash = hash_payload->get_hash(hash_payload);
			DBG3(DBG_ENC, "HASH received %B\nHASH expected %B",
				 &other_hash, &hash);
			if (!chunk_equals(hash, other_hash))
			{
				DBG1(DBG_ENC, "received HASH payload does not match");
				chunk_free(&hash);
				return FAILED;
			}
			chunk_free(&hash);
		}
		if (this->is_encrypted && this->exchange_type != INFORMATIONAL_V1)
		{	/* message verified, confirm IV */
			if (!keymat_v1->confirm_iv(keymat_v1, this->message_id))
			{
				return FAILED;
			}
		}
	}
	return SUCCESS;
}

/**
 * Store the fragment data for the fragment with the given fragment number.
 */
static status_t add_fragment(private_message_t *this, u_int16_t num,
							 chunk_t data)
{
	fragment_t *fragment;
	int i, insert_at = -1;

	for (i = 0; i < array_count(this->fragments); i++)
	{
		array_get(this->fragments, i, &fragment);
		if (fragment->num == num)
		{
			/* ignore a duplicate fragment */
			DBG1(DBG_ENC, "received duplicate fragment #%hu", num);
			return NEED_MORE;
		}
		if (fragment->num > num)
		{
			insert_at = i;
			break;
		}
	}
	this->frag->len += data.len;
	if (this->frag->len > this->frag->max_packet)
	{
		DBG1(DBG_ENC, "fragmented IKE message is too large");
		reset_defrag(this);
		return FAILED;
	}
	INIT(fragment,
		.num = num,
		.data = chunk_clone(data),
	);
	array_insert(this->fragments, insert_at, fragment);
	return SUCCESS;
}

/**
 * Merge the cached fragment data and resets the defragmentation state.
 * Also updates the IP addresses to those of the last received fragment.
 */
static chunk_t merge_fragments(private_message_t *this, message_t *last)
{
	fragment_t *fragment;
	bio_writer_t *writer;
	host_t *src, *dst;
	chunk_t data;
	int i;

	writer = bio_writer_create(this->frag->len);
	for (i = 0; i < array_count(this->fragments); i++)
	{
		array_get(this->fragments, i, &fragment);
		writer->write_data(writer, fragment->data);
	}
	data = writer->extract_buf(writer);
	writer->destroy(writer);

	/* set addresses to those of the last fragment we received */
	src = last->get_source(last);
	dst = last->get_destination(last);
	this->packet->set_source(this->packet, src->clone(src));
	this->packet->set_destination(this->packet, dst->clone(dst));

	reset_defrag(this);
	free(this->frag);
	this->frag = NULL;
	return data;
}

METHOD(message_t, add_fragment_v1, status_t,
	private_message_t *this, message_t *message)
{
	fragment_payload_t *payload;
	chunk_t data;
	u_int8_t num;
	status_t status;

	if (!this->frag)
	{
		return INVALID_STATE;
	}
	payload = (fragment_payload_t*)message->get_payload(message, PLV1_FRAGMENT);
	if (!payload)
	{
		return INVALID_ARG;
	}
	if (!this->fragments || this->message_id != payload->get_id(payload))
	{
		reset_defrag(this);
		this->message_id = payload->get_id(payload);
		/* we don't know the total number of fragments, assume something */
		this->fragments = array_create(0, 4);
	}

	num = payload->get_number(payload);
	data = payload->get_data(payload);
	if (!this->frag->last && payload->is_last(payload))
	{
		this->frag->last = num;
	}
	status = add_fragment(this, num, data);
	if (status != SUCCESS)
	{
		return status;
	}

	if (array_count(this->fragments) != this->frag->last)
	{
		/* there are some fragments missing */
		DBG1(DBG_ENC, "received fragment #%hhu, waiting for complete IKE "
			 "message", num);
		return NEED_MORE;
	}

	DBG1(DBG_ENC, "received fragment #%hhu, reassembling fragmented IKE "
		 "message", num);

	data = merge_fragments(this, message);
	this->packet->set_data(this->packet, data);
	this->parser = parser_create(data);

	if (parse_header(this) != SUCCESS)
	{
		DBG1(DBG_IKE, "failed to parse header of reassembled IKE message");
		return FAILED;
	}
	return SUCCESS;
}

METHOD(message_t, add_fragment_v2, status_t,
	private_message_t *this, message_t *message)
{
	encrypted_fragment_payload_t *encrypted_fragment;
	encrypted_payload_t *encrypted;
	payload_t *payload;
	enumerator_t *enumerator;
	chunk_t data;
	u_int16_t total, num;
	status_t status;

	if (!this->frag)
	{
		return INVALID_STATE;
	}
	payload = message->get_payload(message, PLV2_FRAGMENT);
	if (!payload || this->message_id != message->get_message_id(message))
	{
		return INVALID_ARG;
	}
	encrypted_fragment = (encrypted_fragment_payload_t*)payload;
	total = encrypted_fragment->get_total_fragments(encrypted_fragment);
	if (total > MAX_FRAGMENTS)
	{
		DBG1(DBG_IKE, "maximum fragment count exceeded");
		reset_defrag(this);
		return FAILED;
	}
	if (!this->fragments || total > this->frag->last)
	{
		reset_defrag(this);
		this->frag->last = total;
		this->fragments = array_create(0, total);
	}
	num = encrypted_fragment->get_fragment_number(encrypted_fragment);
	data = encrypted_fragment->get_content(encrypted_fragment);
	status = add_fragment(this, num, data);
	if (status != SUCCESS)
	{
		return status;
	}

	if (num == 1)
	{
		/* the first fragment denotes the payload type of the first payload in
		 * the original encrypted payload, cache that */
		this->first_payload = payload->get_next_type(payload);
		/* move all unencrypted payloads contained in the first fragment */
		enumerator = message->create_payload_enumerator(message);
		while (enumerator->enumerate(enumerator, &payload))
		{
			if (payload->get_type(payload) != PLV2_FRAGMENT)
			{
				message->remove_payload_at(message, enumerator);
				this->payloads->insert_last(this->payloads, payload);
			}
		}
		enumerator->destroy(enumerator);
	}

	if (array_count(this->fragments) != total)
	{
		/* there are some fragments missing */
		DBG1(DBG_ENC, "received fragment #%hu of %hu, waiting for complete IKE "
			 "message", num, total);
		return NEED_MORE;
	}

	DBG1(DBG_ENC, "received fragment #%hu of %hu, reassembling fragmented IKE "
		 "message", num, total);

	data = merge_fragments(this, message);
	encrypted = encrypted_payload_create_from_plain(this->first_payload, data);
	this->payloads->insert_last(this->payloads, encrypted);
	/* update next payload type (could be an unencrypted payload) */
	this->payloads->get_first(this->payloads, (void**)&payload);
	this->first_payload = payload->get_type(payload);
	return SUCCESS;
}

METHOD(message_t, destroy, void,
	private_message_t *this)
{
	DESTROY_IF(this->ike_sa_id);
	DESTROY_IF(this->parser);
	this->payloads->destroy_offset(this->payloads, offsetof(payload_t, destroy));
	this->packet->destroy(this->packet);
	if (this->frag)
	{
		reset_defrag(this);
		free(this->frag);
	}
	else
	{
		array_destroy_offset(this->fragments, offsetof(packet_t, destroy));
	}
	free(this);
}

/*
 * Described in header.
 */
message_t *message_create_from_packet(packet_t *packet)
{
	private_message_t *this;

	INIT(this,
		.public = {
			.set_major_version = _set_major_version,
			.get_major_version = _get_major_version,
			.set_minor_version = _set_minor_version,
			.get_minor_version = _get_minor_version,
			.set_message_id = _set_message_id,
			.get_message_id = _get_message_id,
			.get_initiator_spi = _get_initiator_spi,
			.get_responder_spi = _get_responder_spi,
			.set_ike_sa_id = _set_ike_sa_id,
			.get_ike_sa_id = _get_ike_sa_id,
			.set_exchange_type = _set_exchange_type,
			.get_exchange_type = _get_exchange_type,
			.get_first_payload_type = _get_first_payload_type,
			.set_request = _set_request,
			.get_request = _get_request,
			.set_version_flag = _set_version_flag,
			.get_reserved_header_bit = _get_reserved_header_bit,
			.set_reserved_header_bit = _set_reserved_header_bit,
			.add_payload = _add_payload,
			.add_notify = _add_notify,
			.disable_sort = _disable_sort,
			.generate = _generate,
			.is_encoded = _is_encoded,
			.is_fragmented = _is_fragmented,
			.fragment = _fragment,
			.add_fragment = _add_fragment_v2,
			.set_source = _set_source,
			.get_source = _get_source,
			.set_destination = _set_destination,
			.get_destination = _get_destination,
			.create_payload_enumerator = _create_payload_enumerator,
			.remove_payload_at = _remove_payload_at,
			.get_payload = _get_payload,
			.get_notify = _get_notify,
			.parse_header = _parse_header,
			.parse_body = _parse_body,
			.get_packet = _get_packet,
			.get_packet_data = _get_packet_data,
			.get_fragments = _get_fragments,
			.destroy = _destroy,
		},
		.exchange_type = EXCHANGE_TYPE_UNDEFINED,
		.is_request = TRUE,
		.first_payload = PL_NONE,
		.packet = packet,
		.payloads = linked_list_create(),
		.parser = parser_create(packet->get_data(packet)),
	);

	return &this->public;
}

/*
 * Described in header.
 */
message_t *message_create(int major, int minor)
{
	message_t *this = message_create_from_packet(packet_create());

	this->set_major_version(this, major);
	this->set_minor_version(this, minor);

	return this;
}

/*
 * Described in header.
 */
message_t *message_create_defrag(message_t *fragment)
{
	private_message_t *this;

	if (!fragment->get_payload(fragment, PLV1_FRAGMENT) &&
		!fragment->get_payload(fragment, PLV2_FRAGMENT))
	{
		return NULL;
	}
	this = (private_message_t*)clone_message((private_message_t*)fragment);
	/* we don't need a parser for IKEv2, the one for IKEv1 is created after
	 * reassembling the original message */
	this->parser->destroy(this->parser);
	this->parser = NULL;
	if (fragment->get_major_version(fragment) == IKEV1_MAJOR_VERSION)
	{
		/* we store the fragment ID in the message ID field, which should be
		 * zero for fragments, but make sure */
		this->message_id = 0;
		this->public.add_fragment = _add_fragment_v1;
	}
	INIT(this->frag,
		.max_packet = lib->settings->get_int(lib->settings,
								"%s.max_packet", PACKET_MAX_DEFAULT, lib->ns),
	);
	return &this->public;
}
