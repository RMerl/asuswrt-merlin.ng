/*
 * rlm_eap_leap.c    Handles that are called from eap
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

RCSID("$Id$")

#include <stdio.h>
#include <stdlib.h>

#include "eap_leap.h"


/*
 * send an initial eap-leap request
 * ie access challenge to the user/peer.

 * Frame eap reply packet.
 * len = header + type + leap_methoddata
 * leap_methoddata = value_size + value
 */
static int leap_initiate(UNUSED void *instance, eap_handler_t *handler)
{
	REQUEST 	*request = handler->request;
	leap_session_t	*session;
	leap_packet_t	*reply;

	RDEBUG2("Stage 2");

	/*
	 *	LEAP requires a User-Name attribute
	 */
	if (!handler->request->username) {
		REDEBUG("User-Name is required for EAP-LEAP authentication");
		return 0;
	}

	reply = eapleap_initiate(request, handler->eap_ds, handler->request->username);
	if (!reply) {
		return 0;
	}

	eapleap_compose(request, handler->eap_ds, reply);

	handler->opaque = session = talloc(handler, leap_session_t);
	if (!handler->opaque) {
		talloc_free(reply);
		return 0;
	}

	/*
	 *	Remember which stage we're in, and which challenge
	 *	we sent to the AP.  The later stages will take care
	 *	of filling in the peer response.
	 */
	handler->free_opaque = NULL;

	session->stage = 4;	/* the next stage we're in */
	memcpy(session->peer_challenge, reply->challenge, reply->count);

	REDEBUG2("Successfully initiated");

	/*
	 *	The next stage to process the packet.
	 */
	handler->stage = AUTHENTICATE;

	talloc_free(reply);
	return 1;
}

static int mod_authenticate(UNUSED void *instance, eap_handler_t *handler)
{
	int		rcode;
	REQUEST 	*request = handler->request;
	leap_session_t	*session;
	leap_packet_t	*packet;
	leap_packet_t	*reply;
	VALUE_PAIR	*password;

	if (!handler->opaque) {
		REDEBUG("Cannot authenticate without LEAP history");
		return 0;
	}
	session = (leap_session_t *) handler->opaque;
	reply = NULL;

	/*
	 *	Extract the LEAP packet.
	 */
	if (!(packet = eapleap_extract(request, handler->eap_ds))) {
		return 0;
	}

	/*
	 *	The password is never sent over the wire.
	 *	Always get the configured password, for each user.
	 */
	password = pairfind(handler->request->config_items, PW_CLEARTEXT_PASSWORD, 0, TAG_ANY);
	if (!password) {
		password = pairfind(handler->request->config_items, PW_NT_PASSWORD, 0, TAG_ANY);
	}

	if (!password) {
		REDEBUG("No Cleartext-Password or NT-Password configured for this user");
		talloc_free(packet);
		return 0;
	}

	/*
	 *	We've already sent the AP challenge.  This packet
	 *	should contain the NtChallengeResponse
	 */
	switch (session->stage) {
	case 4:			/* Verify NtChallengeResponse */
		RDEBUG2("Stage 4");
		rcode = eapleap_stage4(request, packet, password, session);
		session->stage = 6;

		/*
		 *	We send EAP-Success or EAP-Fail, and not
		 *	any LEAP packet.  So we return here.
		 */
		if (!rcode) {
			handler->eap_ds->request->code = PW_EAP_FAILURE;
			talloc_free(packet);
			return 0;
		}

		handler->eap_ds->request->code = PW_EAP_SUCCESS;

		/*
		 *	Do this only for Success.
		 */
		handler->eap_ds->request->id = handler->eap_ds->response->id + 1;
		handler->eap_ds->set_request_id = 1;

		/*
		 *	LEAP requires a challenge in stage 4, not
		 *	an Access-Accept, which is normally returned
		 *	by eap_compose() in eap.c, when the EAP reply code
		 *	is EAP_SUCCESS.
		 */
		handler->request->reply->code = PW_ACCESS_CHALLENGE;
		talloc_free(packet);
		return 1;

	case 6:			/* Issue session key */
		RDEBUG2("Stage 6");
		reply = eapleap_stage6(request, packet, handler->request->username, password, session);
		break;

		/*
		 *	Stages 1, 3, and 5 are requests from the AP.
		 *	Stage 2 is handled by initiate()
		 */
	default:
		RDEBUG("Internal sanity check failed on stage");
		break;
	}

	talloc_free(packet);

	/*
	 *	Process the packet.  We don't care about any previous
	 *	EAP packets, as
	 */
	if (!reply) {
		return 0;
	}

	eapleap_compose(request, handler->eap_ds, reply);
	talloc_free(reply);
	return 1;
}

/*
 *	The module name should be the only globally exported symbol.
 *	That is, everything else should be 'static'.
 */
rlm_eap_module_t rlm_eap_leap = {
	"eap_leap",
	NULL,			/* attach */
	leap_initiate,		/* Start the initial request, after Identity */
	NULL,			/* authorization */
	mod_authenticate,	/* authentication */
	NULL,			/* detach */
};
