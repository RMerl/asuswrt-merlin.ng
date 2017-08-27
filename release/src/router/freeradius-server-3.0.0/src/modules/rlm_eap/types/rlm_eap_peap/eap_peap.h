/*
 * eap_peap.h
 *
 * Version:     $Id$
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 *
 * Copyright 2003 Alan DeKok <aland@freeradius.org>
 * Copyright 2006 The FreeRADIUS server project
 */
#ifndef _EAP_PEAP_H
#define _EAP_PEAP_H

RCSIDH(eap_peap_h, "$Id$")

#include "eap_tls.h"
#include <freeradius-devel/soh.h>

typedef struct peap_tunnel_t {
	VALUE_PAIR	*username;
	VALUE_PAIR	*state;
	VALUE_PAIR	*accept_vps;
	int		status;
	int		home_access_accept;
	int		default_method;
	int		copy_request_to_tunnel;
	int		use_tunneled_reply;
	int		proxy_tunneled_request_as_eap;
	char const	*virtual_server;
	int		soh;
	char const	*soh_virtual_server;
	VALUE_PAIR	*soh_reply_vps;
	int		session_resumption_state;
} peap_tunnel_t;

#define PEAP_STATUS_INVALID 0
#define PEAP_STATUS_SENT_TLV_SUCCESS 1
#define PEAP_STATUS_SENT_TLV_FAILURE 2
#define PEAP_STATUS_TUNNEL_ESTABLISHED 3
#define PEAP_STATUS_INNER_IDENTITY_REQ_SENT 4
#define PEAP_STATUS_PHASE2_INIT 5
#define PEAP_STATUS_PHASE2 6
#define PEAP_STATUS_WAIT_FOR_SOH_RESPONSE 7

#define PEAP_RESUMPTION_NO	(0)
#define PEAP_RESUMPTION_YES	(1)
#define PEAP_RESUMPTION_MAYBE	(2)

#define EAP_TLV_SUCCESS (1)
#define EAP_TLV_FAILURE (2)
#define EAP_TLV_ACK_RESULT (3)

#define PW_EAP_TLV 33

/*
 *	Process the PEAP portion of an EAP-PEAP request.
 */
rlm_rcode_t eappeap_process(eap_handler_t *handler, tls_session_t *tls_session);
#endif /* _EAP_PEAP_H */
