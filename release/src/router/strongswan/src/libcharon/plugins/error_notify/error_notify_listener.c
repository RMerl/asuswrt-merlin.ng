/*
 * Copyright (C) 2012 Martin Willi
 *
 * Copyright (C) secunet Security Networks AG
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

#include "error_notify_listener.h"

#include <daemon.h>

typedef struct private_error_notify_listener_t private_error_notify_listener_t;

/**
 * Private data of an error_notify_listener_t object.
 */
struct private_error_notify_listener_t {

	/**
	 * Public error_notify_listener_t interface.
	 */
	error_notify_listener_t public;

	/**
	 * Socket to send notifications over
	 */
	error_notify_socket_t *socket;
};

METHOD(listener_t, alert, bool,
	private_error_notify_listener_t *this, ike_sa_t *ike_sa,
	alert_t alert, va_list args)
{
	error_notify_msg_t msg;
	message_t *message;
	host_t *host;
	identification_t *id;
	linked_list_t *list, *list2;
	peer_cfg_t *peer_cfg;
	certificate_t *cert;
	time_t not_before, not_after;
	int num;

	if (!this->socket->has_listeners(this->socket))
	{
		return TRUE;
	}

	memset(&msg, 0, sizeof(msg));

	switch (alert)
	{
		case ALERT_RADIUS_NOT_RESPONDING:
			msg.type = htonl(ERROR_NOTIFY_RADIUS_NOT_RESPONDING);
			snprintf(msg.str, sizeof(msg.str),
					 "a RADIUS request message timed out");
			break;
		case ALERT_LOCAL_AUTH_FAILED:
			msg.type = htonl(ERROR_NOTIFY_LOCAL_AUTH_FAILED);
			snprintf(msg.str, sizeof(msg.str), "local authentication failed");
			break;
		case ALERT_PEER_AUTH_FAILED:
			msg.type = htonl(ERROR_NOTIFY_PEER_AUTH_FAILED);
			snprintf(msg.str, sizeof(msg.str), "peer authentication failed");
			break;
		case ALERT_PEER_ADDR_FAILED:
			msg.type = htonl(ERROR_NOTIFY_PEER_ADDR_FAILED);
			snprintf(msg.str, sizeof(msg.str), "failed to resolve peer address");
			break;
		case ALERT_PEER_INIT_UNREACHABLE:
			msg.type = htonl(ERROR_NOTIFY_PEER_INIT_UNREACHABLE);
			num = va_arg(args, int) + 1;
			snprintf(msg.str, sizeof(msg.str), "peer did not respond to initial "
					 "message, try %d", num);
			break;
		case ALERT_INVALID_IKE_SPI:
			msg.type = htonl(ERROR_NOTIFY_INVALID_IKE_SPI);
			message = va_arg(args, message_t*);
			snprintf(msg.str, sizeof(msg.str), "received IKE message with unknown "
					 "SPI from %#H", message->get_source(message));
			break;
		case ALERT_PARSE_ERROR_HEADER:
			msg.type = htonl(ERROR_NOTIFY_PARSE_ERROR_HEADER);
			message = va_arg(args, message_t*);
			snprintf(msg.str, sizeof(msg.str), "parsing IKE header from "
					 "%#H failed", message->get_source(message));
			break;
		case ALERT_PARSE_ERROR_BODY:
			msg.type = htonl(ERROR_NOTIFY_PARSE_ERROR_BODY);
			message = va_arg(args, message_t*);
			snprintf(msg.str, sizeof(msg.str), "parsing IKE message from "
					 "%#H failed", message->get_source(message));
			break;
		case ALERT_RETRANSMIT_SEND:
			msg.type = htonl(ERROR_NOTIFY_RETRANSMIT_SEND);
			va_arg(args, packet_t*);
			snprintf(msg.str, sizeof(msg.str), "IKE message retransmission "
					 "number %u", va_arg(args, u_int));
			break;
		case ALERT_RETRANSMIT_SEND_CLEARED:
			msg.type = htonl(ERROR_NOTIFY_RETRANSMIT_CLEARED);
			snprintf(msg.str, sizeof(msg.str), "IKE response received after "
					 "retransmission");
			break;
		case ALERT_RETRANSMIT_SEND_TIMEOUT:
			msg.type = htonl(ERROR_NOTIFY_RETRANSMIT_SEND_TIMEOUT);
			snprintf(msg.str, sizeof(msg.str),
					 "IKE message retransmission timed out");
			break;
		case ALERT_RETRANSMIT_RECEIVE:
			msg.type = htonl(ERROR_NOTIFY_RETRANSMIT_RECEIVE);
			message = va_arg(args, message_t*);
			snprintf(msg.str, sizeof(msg.str), "received retransmit of request "
					 "with ID %d from %#H", message->get_message_id(message),
					 message->get_source(message));
			break;
		case ALERT_HALF_OPEN_TIMEOUT:
			msg.type = htonl(ERROR_NOTIFY_HALF_OPEN_TIMEOUT);
			snprintf(msg.str, sizeof(msg.str), "IKE_SA timed out before it "
					 "could be established");
			break;
		case ALERT_PROPOSAL_MISMATCH_IKE:
			msg.type = htonl(ERROR_NOTIFY_PROPOSAL_MISMATCH_IKE);
			list = va_arg(args, linked_list_t*);
			snprintf(msg.str, sizeof(msg.str), "the received IKE_SA proposals "
					 "did not match: %#P", list);
			break;
		case ALERT_PROPOSAL_MISMATCH_CHILD:
			msg.type = htonl(ERROR_NOTIFY_PROPOSAL_MISMATCH_CHILD);
			list = va_arg(args, linked_list_t*);
			snprintf(msg.str, sizeof(msg.str), "the received CHILD_SA proposals "
					 "did not match: %#P", list);
			break;
		case ALERT_TS_MISMATCH:
			msg.type = htonl(ERROR_NOTIFY_TS_MISMATCH);
			list = va_arg(args, linked_list_t*);
			list2 = va_arg(args, linked_list_t*);
			snprintf(msg.str, sizeof(msg.str), "the received traffic selectors "
					 "did not match: %#R === %#R", list, list2);
			break;
		case ALERT_TS_NARROWED:
			msg.type = htonl(ERROR_NOTIFY_TS_NARROWED);
			num = va_arg(args, int);
			list = va_arg(args, linked_list_t*);
			snprintf(msg.str, sizeof(msg.str), "%s traffic selectors narrowed "
					 "to %#R", num ? "local" : "remote", list);
			break;
		case ALERT_INSTALL_CHILD_SA_FAILED:
			msg.type = htonl(ERROR_NOTIFY_INSTALL_CHILD_SA_FAILED);
			snprintf(msg.str, sizeof(msg.str), "installing IPsec SA failed");
			break;
		case ALERT_INSTALL_CHILD_POLICY_FAILED:
			msg.type = htonl(ERROR_NOTIFY_INSTALL_CHILD_POLICY_FAILED);
			snprintf(msg.str, sizeof(msg.str), "installing IPsec policy failed");
			break;
		case ALERT_UNIQUE_REPLACE:
			msg.type = htonl(ERROR_NOTIFY_UNIQUE_REPLACE);
			snprintf(msg.str, sizeof(msg.str),
					 "replaced old IKE_SA due to uniqueness policy");
			break;
		case ALERT_UNIQUE_KEEP:
			msg.type = htonl(ERROR_NOTIFY_UNIQUE_KEEP);
			snprintf(msg.str, sizeof(msg.str), "keep existing in favor of "
					 "rejected new IKE_SA due to uniqueness policy");
			break;
		case ALERT_KEEP_ON_CHILD_SA_FAILURE:
			msg.type = htonl(ERROR_NOTIFY_KEEP_ON_CHILD_SA_FAILURE);
			num = va_arg(args, int);
			snprintf(msg.str, sizeof(msg.str), "keeping IKE_SA after failing to "
					 "establish %sCHILD_SA", num ? "first " : "");
			break;
		case ALERT_VIP_FAILURE:
			msg.type = htonl(ERROR_NOTIFY_VIP_FAILURE);
			list = va_arg(args, linked_list_t*);
			if (list->get_first(list, (void**)&host) == SUCCESS)
			{
				snprintf(msg.str, sizeof(msg.str),
					"allocating a virtual IP failed, requested was %H", host);
			}
			else
			{
				snprintf(msg.str, sizeof(msg.str),
					"expected a virtual IP request, but none found");
			}
			break;
		case ALERT_AUTHORIZATION_FAILED:
			msg.type = htonl(ERROR_NOTIFY_AUTHORIZATION_FAILED);
			snprintf(msg.str, sizeof(msg.str), "an authorization plugin "
					 "prevented establishment of an IKE_SA");
			break;
		case ALERT_IKE_SA_EXPIRED:
			msg.type = htonl(ERROR_NOTIFY_IKE_SA_EXPIRED);
			snprintf(msg.str, sizeof(msg.str), "IKE_SA expired without "
					 "replacement");
			break;
		case ALERT_CERT_EXPIRED:
			msg.type = htonl(ERROR_NOTIFY_CERT_EXPIRED);
			cert = va_arg(args, certificate_t*);
			cert->get_validity(cert, NULL, &not_before, &not_after);
			snprintf(msg.str, sizeof(msg.str), "certificate expired: '%Y' "
					 "(valid from %T to %T)", cert->get_subject(cert),
					 &not_before, TRUE, &not_after, TRUE);
			break;
		case ALERT_CERT_REVOKED:
			msg.type = htonl(ERROR_NOTIFY_CERT_REVOKED);
			cert = va_arg(args, certificate_t*);
			snprintf(msg.str, sizeof(msg.str), "certificate revoked: '%Y'",
					 cert->get_subject(cert));
			break;
		case ALERT_CERT_VALIDATION_FAILED:
			msg.type = htonl(ERROR_NOTIFY_CERT_VALIDATION_FAILED);
			cert = va_arg(args, certificate_t*);
			snprintf(msg.str, sizeof(msg.str), "failed to validate certificate "
					 "status: '%Y'", cert->get_subject(cert));
			break;
		case ALERT_CERT_NO_ISSUER:
			msg.type = htonl(ERROR_NOTIFY_NO_ISSUER_CERT);
			cert = va_arg(args, certificate_t*);
			snprintf(msg.str, sizeof(msg.str), "no trusted issuer certificate "
					 "found: '%Y'", cert->get_issuer(cert));
			break;
		case ALERT_CERT_UNTRUSTED_ROOT:
			msg.type = htonl(ERROR_NOTIFY_UNTRUSTED_ROOT_CERT);
			cert = va_arg(args, certificate_t*);
			snprintf(msg.str, sizeof(msg.str), "self-signed issuer certificate "
					 "is not trusted: '%Y'", cert->get_issuer(cert));
			break;
		case ALERT_CERT_EXCEEDED_PATH_LEN:
			msg.type = htonl(ERROR_NOTIFY_CERT_EXCEEDED_PATH_LEN);
			cert = va_arg(args, certificate_t*);
			snprintf(msg.str, sizeof(msg.str), "maximum path length in trust "
					 "chain exceeded: '%Y'", cert->get_issuer(cert));
			break;
		case ALERT_CERT_POLICY_VIOLATION:
			msg.type = htonl(ERROR_NOTIFY_CERT_POLICY_VIOLATION);
			cert = va_arg(args, certificate_t*);
			snprintf(msg.str, sizeof(msg.str), "certificate rejected because of "
					 "policy violation: '%Y'", cert->get_issuer(cert));
			break;
		case ALERT_SHUTDOWN_SIGNAL:
			return TRUE;
	}

	if (ike_sa)
	{
		id = ike_sa->get_other_eap_id(ike_sa);
		if (id->get_type(id) != ID_ANY)
		{
			snprintf(msg.id, sizeof(msg.id), "%Y", id);
		}
		host = ike_sa->get_other_host(ike_sa);
		if (!host->is_anyaddr(host))
		{
			snprintf(msg.ip, sizeof(msg.ip), "%#H", host);
		}
		peer_cfg = ike_sa->get_peer_cfg(ike_sa);
		if (peer_cfg)
		{
			snprintf(msg.name, sizeof(msg.name), "%s",
					 peer_cfg->get_name(peer_cfg));
		}
	}

	this->socket->notify(this->socket, &msg);

	return TRUE;
}

METHOD(error_notify_listener_t, destroy, void,
	private_error_notify_listener_t *this)
{
	free(this);
}

/**
 * See header
 */
error_notify_listener_t *error_notify_listener_create(error_notify_socket_t *s)
{
	private_error_notify_listener_t *this;

	INIT(this,
		.public = {
			.listener = {
				.alert = _alert,
			},
			.destroy = _destroy,
		},
		.socket = s,
	);

	return &this->public;
}
