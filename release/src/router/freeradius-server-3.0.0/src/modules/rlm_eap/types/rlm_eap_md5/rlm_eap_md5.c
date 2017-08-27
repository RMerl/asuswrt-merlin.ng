/*
 * rlm_eap_md5.c    Handles that are called from eap
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
 * Copyright 2000,2001,2006  The FreeRADIUS server project
 * Copyright 2001  hereUare Communications, Inc. <raghud@hereuare.com>
 */

RCSID("$Id$")

#include <stdio.h>
#include <stdlib.h>

#include "eap_md5.h"

#include <freeradius-devel/rad_assert.h>

/*
 *	Initiate the EAP-MD5 session by sending a challenge to the peer.
 */
static int md5_initiate(UNUSED void *instance, eap_handler_t *handler)
{
	int		i;
	MD5_PACKET	*reply;

	/*
	 *	Allocate an EAP-MD5 packet.
	 */
	reply = talloc(handler, MD5_PACKET);
	if (!reply)  {
		return 0;
	}

	/*
	 *	Fill it with data.
	 */
	reply->code = PW_MD5_CHALLENGE;
	reply->length = 1 + MD5_CHALLENGE_LEN; /* one byte of value size */
	reply->value_size = MD5_CHALLENGE_LEN;

	/*
	 *	Allocate user data.
	 */
	reply->value = talloc_array(reply, uint8_t, reply->value_size);
	if (!reply->value) {
		talloc_free(reply);
		return 0;
	}

	/*
	 *	Get a random challenge.
	 */
	for (i = 0; i < reply->value_size; i++) {
		reply->value[i] = fr_rand();
	}
	DEBUG2("rlm_eap_md5: Issuing Challenge");

	/*
	 *	Keep track of the challenge.
	 */
	handler->opaque = talloc_array(handler, uint8_t, reply->value_size);
	rad_assert(handler->opaque != NULL);
	memcpy(handler->opaque, reply->value, reply->value_size);
	handler->free_opaque = NULL;

	/*
	 *	Compose the EAP-MD5 packet out of the data structure,
	 *	and free it.
	 */
	eapmd5_compose(handler->eap_ds, reply);

	/*
	 *	We don't need to authorize the user at this point.
	 *
	 *	We also don't need to keep the challenge, as it's
	 *	stored in 'handler->eap_ds', which will be given back
	 *	to us...
	 */
	handler->stage = AUTHENTICATE;

	return 1;
}


/*
 *	Authenticate a previously sent challenge.
 */
static int md5_authenticate(UNUSED void *arg, eap_handler_t *handler)
{
	MD5_PACKET	*packet;
	MD5_PACKET	*reply;
	VALUE_PAIR	*password;

	/*
	 *	Get the Cleartext-Password for this user.
	 */
	rad_assert(handler->request != NULL);
	rad_assert(handler->stage == AUTHENTICATE);

	password = pairfind(handler->request->config_items, PW_CLEARTEXT_PASSWORD, 0, TAG_ANY);
	if (!password) {
		DEBUG2("rlm_eap_md5: Cleartext-Password is required for EAP-MD5 authentication");
		return 0;
	}

	/*
	 *	Extract the EAP-MD5 packet.
	 */
	if (!(packet = eapmd5_extract(handler->eap_ds)))
		return 0;

	/*
	 *	Create a reply, and initialize it.
	 */
	reply = talloc(packet, MD5_PACKET);
	if (!reply) {
		talloc_free(packet);
		return 0;
	}
	reply->id = handler->eap_ds->request->id;
	reply->length = 0;

	/*
	 *	Verify the received packet against the previous packet
	 *	(i.e. challenge) which we sent out.
	 */
	if (eapmd5_verify(packet, password, handler->opaque)) {
		reply->code = PW_MD5_SUCCESS;
	} else {
		reply->code = PW_MD5_FAILURE;
	}

	/*
	 *	Compose the EAP-MD5 packet out of the data structure,
	 *	and free it.
	 */
	eapmd5_compose(handler->eap_ds, reply);
	talloc_free(packet);
	return 1;
}

/*
 *	The module name should be the only globally exported symbol.
 *	That is, everything else should be 'static'.
 */
rlm_eap_module_t rlm_eap_md5 = {
	"eap_md5",
	NULL,				/* attach */
	md5_initiate,			/* Start the initial request */
	NULL,				/* authorization */
	md5_authenticate,		/* authentication */
	NULL				/* detach */
};
