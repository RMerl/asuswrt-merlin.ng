/*
 * eap.h    Header file containing the interfaces for all EAP types.
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
 * Copyright 2001  hereUare Communications, Inc. <raghud@hereuare.com>
 * Copyright 2003  Alan DeKok <aland@freeradius.org>
 * Copyright 2006  The FreeRADIUS server project
 */
#ifndef _EAP_H
#define _EAP_H

RCSIDH(eap_h, "$Id$")

#include <freeradius-devel/radiusd.h>
#include <freeradius-devel/modules.h>
#include <freeradius-devel/rad_assert.h>

#include "eap_types.h"

/* TLS configuration name */
#define TLS_CONFIG_SECTION "tls-config"

/*
 * EAP_DS contains all the received/sending information
 * response = Received EAP packet
 * request = Sending EAP packet
 *
 * Note: We are authentication server,
 *  we get ONLY EAP-Responses and
 *  we send EAP-Request/EAP-success/EAP-failure
 */
typedef struct eap_ds {
	eap_packet_t	*response;
	eap_packet_t	*request;
	int		set_request_id;
} EAP_DS;

/*
 * Currently there are only 2 types
 * of operations defined,
 * apart from attach & detach for each EAP-Type.
 */
typedef enum operation_t {
	INITIATE = 0,
	AUTHORIZE,
	AUTHENTICATE
} operation_t;


/*
 * eap_handler_t is the interface for any EAP-Type.
 * Each handler contains information for one specific EAP-Type.
 * This way we don't need to change any interfaces in future.
 * It is also a list of EAP-request handlers waiting for EAP-response
 * eap_id = copy of the eap packet we sent to the
 *
 * next = pointer to next
 * state = state attribute from the reply we sent
 * state_len = length of data in the state attribute.
 * src_ipaddr = client which sent us the RADIUS request containing
 *	      this EAP conversation.
 * eap_id = copy of EAP id we sent to the client.
 * timestamp  = timestamp when this handler was last used.
 * identity = Identity, as obtained, from EAP-Identity response.
 * request = RADIUS request data structure
 * prev_eapds = Previous EAP request, for which eap_ds contains the response.
 * eap_ds   = Current EAP response.
 * opaque   = EAP-Type holds some data that corresponds to the current
 *		EAP-request/response
 * free_opaque = To release memory held by opaque,
 * 		when this handler is timedout & needs to be deleted.
 * 		It is the responsibility of the specific EAP-TYPE
 * 		to avoid any memory leaks in opaque
 *		Hence this pointer should be provided by the EAP-Type
 *		if opaque is not NULL
 * status   = finished/onhold/..
 */
#define EAP_STATE_LEN (AUTH_VECTOR_LEN)
typedef struct _eap_handler {
	struct _eap_handler *prev, *next;
	uint8_t		state[EAP_STATE_LEN];
	fr_ipaddr_t	src_ipaddr;

	uint8_t		eap_id;		//!< EAP Identifier used to match
					//!< requests and responses.
	eap_type_t	type;		//!< EAP type number.

	time_t		timestamp;

	REQUEST		*request;

	char		*identity;	//!< User name from EAP-Identity

	EAP_DS 		*prev_eapds;
	EAP_DS 		*eap_ds;

	void 		*opaque;
	void 		(*free_opaque)(void *opaque);
	void		*inst_holder;

	int		status;

	int		stage;

	int		trips;

	int		tls;
	int		finished;
	VALUE_PAIR	*certs;
} eap_handler_t;

/*
 * Interface to call EAP sub mdoules
 */
typedef struct rlm_eap_module {
	char const *name;
	int (*attach)(CONF_SECTION *conf, void **instance);
	int (*initiate)(void *instance, eap_handler_t *handler);
	int (*authorize)(void *instance, eap_handler_t *handler);
	int (*authenticate)(void *instance, eap_handler_t *handler);
	int (*detach)(void *instance);
} rlm_eap_module_t;

#define REQUEST_DATA_EAP_HANDLER	 (1)
#define REQUEST_DATA_EAP_TUNNEL_CALLBACK PW_EAP_MESSAGE
#define REQUEST_DATA_EAP_MSCHAP_TUNNEL_CALLBACK ((PW_EAP_MESSAGE << 16) | PW_EAP_MSCHAPV2)
#define RAD_REQUEST_OPTION_PROXY_EAP	(1 << 16)

/*
 *	This is for tunneled callbacks
 */
typedef int (*eap_tunnel_callback_t)(eap_handler_t *handler, void *tls_session);

typedef struct eap_tunnel_data_t {
  void			*tls_session;
  eap_tunnel_callback_t callback;
} eap_tunnel_data_t;

#endif /*_EAP_H*/
