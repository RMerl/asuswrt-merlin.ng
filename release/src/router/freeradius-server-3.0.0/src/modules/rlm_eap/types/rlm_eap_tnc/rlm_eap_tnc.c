/*
 *   Portions of this code unrelated to FreeRADIUS are available
 *   separately under a commercial license.  If you require an
 *   implementation of EAP-TNC that is not under the GPLv2, please
 *   contact trust@f4-i.fh-hannover.de for details.
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
 */

/**
 * $Id$
 * @file rlm_eap_tnc.c
 * @brief Interfaces with the naeap library to provide EAP-TNC inner method.
 *
 * @copyright 2013 The FreeRADIUS project
 * @copyright 2007 Alan DeKok <aland@deployingradius.com>
 * @copyright 2006-2009 FH Hannover
 */

/*
 * EAP-TNC Packet with EAP Header, general structure
 *
 *  0		   1		   2		   3
 *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |      Code     |   Identifier  |	    Length	     |
 * |	       |	       |			       |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |      Type     |  Flags  | Ver |	  Data Length	  |
 * |	       |L M S R R| =1  |			       |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |	  Data Length	  |	   Data ...
 * |			       |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

#include <stdio.h>
#include <stdlib.h>

#include <freeradius-devel/rad_assert.h>

#include "eap.h"
#include <naaeap/naaeap.h>
#include <netinet/in.h>

#define VERSION "0.7.0"
#define SET_START(x) 		((x) | (0x20))

typedef struct rlm_eap_tnc {
	char	*connection_string;
} rlm_eap_tnc_t;

static CONF_PARSER module_config[] = {
	{ "connection_string", PW_TYPE_STRING_PTR,
	  offsetof(rlm_eap_tnc_t, connection_string), NULL,
	  "NAS Port: %{NAS-Port} NAS IP: %{NAS-IP-Address} NAS_PORT_TYPE: %{NAS-Port-Type}"},

 	{ NULL, -1, 0, NULL, NULL }	   /* end the list */
};

static int tnc_attach(CONF_SECTION *cs, void **instance)
{
	rlm_eap_tnc_t *inst;
	TNC_Result result;

	*instance = inst = talloc_zero(cs, rlm_eap_tnc_t);
	if (!inst) return -1;

	/*
	 *	Parse the configuration attributes.
	 */
	if (cf_section_parse(cs, inst, module_config) < 0) {
		return -1;
	}

	result = initializeDefault();
	if (result != TNC_RESULT_SUCCESS) {
		ERROR("rlm_eap_tnc: NAA-EAP initializeDefault returned an "
		      "error code");

		return -1;
	}

	return 0;
}

static int mod_detach(void *instance)
{
	TNC_Result result;

	talloc_free(instance);

	result = terminate();
	if (result != TNC_RESULT_SUCCESS) {
		ERROR("rlm_eap_tnc: NAA-EAP terminate returned an "
		      "error code whilst detaching");
		return -1;
	}

	return 0;
}

static void tnc_free(void *conn_id)
{
	talloc_free(conn_id);
}

/*
 * This function is called when the first EAP_IDENTITY_RESPONSE message
 * was received.
 *
 * Initiates the EPA_TNC session by sending the first EAP_TNC_RESPONSE
 * to the peer. The packet has the Start-Bit set and contains no data.
 *
 *  0		   1		   2		   3
 *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |      Code     |   Identifier  |	    Length	     |
 * |	       |	       |			       |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |      Type     |  Flags  | Ver |
 * |	       |0 0 1 0 0|0 0 1|
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * For this package, only 'Identifier' has to be set dynamically. Any
 * other information is static.
 */
static int tnc_initiate(void *instance, eap_handler_t *handler)
{
	rlm_eap_tnc_t *inst = instance;
	REQUEST *request = NULL;

	char buff[71];
	ssize_t len = 0;

	TNC_Result result;
	TNC_ConnectionID conn_id;

	TNC_BufferReference eap_tnc_request;
	TNC_BufferReference eap_tnc_user;

	VALUE_PAIR *username;

	/*
	 *	Check if we run inside a secure EAP method.
	 *	FIXME check concrete outer EAP method.
	 */
	if (!handler->request || !handler->request->parent) {
		ERROR("rlm_eap_tnc: EAP_TNC must only be used as an "
		      "inner method within a protected tunneled EAP created "
		      "by an outer EAP method");

		return 0;
	}

	request = handler->request->parent;

	/*
	 *	Build the connection string
	 */
	len = radius_xlat(buff, sizeof(buff), request, inst->connection_string, NULL, NULL);
	if (len < 0){
		return 0;
	}

	RDEBUG("Getting connection from NAA-EAP");

	/*
	 *	Get connection (uses a function from the NAA-EAP-library)
	 */
	result = getConnection(buff, &conn_id);
	if (result != TNC_RESULT_SUCCESS) {
		ERROR("rlm_eap_tnc: NAA-EAP getConnection returned an "
		      "error code");

		return 0;
	}

	/*
	 *	Previous code manually parsed the EAP identity response
	 *	this was wrong. rlm_eap will *always* create the Username
	 *	from the EAP Identity response.
	 *
	 *	Something has gone very wrong if the User-Name doesn't exist.
	 */
	username = pairfind(request->packet->vps, PW_USER_NAME, 0, TAG_ANY);

	RDEBUG("Username for TNC connection: %s", username->vp_strvalue);

	/*
	 *	Stores the username associated with the connection
	 *
	 *	What becomes of username? Who knows... but we don't free it
	 *	so not safe to use talloc.
	 */
	MEM(eap_tnc_user = (TNC_BufferReference) strdup(username->vp_strvalue));

	result = storeUsername(conn_id, eap_tnc_user, username->length);
	if (result != TNC_RESULT_SUCCESS) {
		ERROR("rlm_eap_tnc: NAA-EAP storeUsername returned an "
		      "error code");

		return 0;
	}

	/*
	 *	Set connection ID
	 */
	handler->opaque = talloc(handler, TNC_ConnectionID);
	memcpy(handler->opaque, &conn_id, sizeof(TNC_ConnectionID));
	handler->free_opaque = tnc_free;

	/*
	 *	Bild first EAP TNC request
	 */

	MEM(eap_tnc_request = talloc_array(handler->eap_ds->request, uint8_t, 1));
	*eap_tnc_request = SET_START(1);

	handler->eap_ds->request->code = PW_EAP_REQUEST;
	handler->eap_ds->request->type.num = PW_EAP_TNC;

	handler->eap_ds->request->type.length = 1;

	talloc_free(handler->eap_ds->request->type.data);
	handler->eap_ds->request->type.data = eap_tnc_request;

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

/**
 * This function is called when a EAP_TNC_RESPONSE was received.
 * It basically forwards the EAP_TNC data to NAA-TNCS and forms
 * and appropriate EAP_RESPONSE. Furthermore, it sets the VlanID
 * based on the TNC_ConnectionState determined by NAA-TNCS.
 *
 * @param instance The configuration data.
 * @param handler The eap_handler_t.
 * @return True, if successfully, else false.
 */
static int mod_authenticate(UNUSED void *instance, eap_handler_t *handler)
{
	TNC_ConnectionID conn_id;
	TNC_Result result;

	TNC_BufferReference data = NULL;
	TNC_UInt32 datalen = 0;

	TNC_ConnectionState connection_state;
	uint8_t code = 0;
	REQUEST *request = handler->request;

	if (handler->eap_ds->response->type.num != PW_EAP_TNC) {
		ERROR("rlm_eap_tnc: Incorrect response type");

		return 0;
	}

	/*
	 *	Retrieve connection ID
	 */
	conn_id = *((TNC_ConnectionID *) (handler->opaque));

	RDEBUG2("Starting authentication for connection ID %lX",
	       conn_id);

	/*
	 * 	Pass EAP_TNC data to NAA-EAP and get answer data
	 */
	connection_state = TNC_CONNECTION_STATE_CREATE;

	/*
	 * 	Forwards the eap_tnc data to NAA-EAP and gets the response
	 */
	result = processEAPTNCData(conn_id, handler->eap_ds->response->type.data,
				   handler->eap_ds->response->type.length,
				   &data, &datalen, &connection_state);
	if (result != TNC_RESULT_SUCCESS) {
		RDEBUG("NAA-EAP processEAPTNCData returned "
		       "an error code");

		return 0;
	}
	/*
	 *	Determine eap code for the response
	 */
	switch (connection_state) {
	case TNC_CONNECTION_STATE_HANDSHAKE:
		code = PW_EAP_REQUEST;

		break;
	case TNC_CONNECTION_STATE_ACCESS_NONE:
		code = PW_EAP_FAILURE;
		pairmake_config("TNC-Status", "None", T_OP_SET);

		break;

	case TNC_CONNECTION_STATE_ACCESS_ALLOWED:
		code = PW_EAP_SUCCESS;
		pairmake_config("TNC-Status", "Access", T_OP_SET);
		break;

	case TNC_CONNECTION_STATE_ACCESS_ISOLATED:
		code = PW_EAP_SUCCESS;
		pairmake_config("TNC-Status", "Isolate", T_OP_SET);

		break;
	default:
		ERROR("rlm_eap_tnc: Invalid connection state");

		return 0;
	}

	/*
	 *	Build the TNC EAP request
	 */
	handler->eap_ds->request->code = code;
	handler->eap_ds->request->type.num = PW_EAP_TNC;

	handler->eap_ds->request->type.length = datalen;

	talloc_free(handler->eap_ds->request->type.data);

	/*
	 *	"data" is not talloc'd memory.
	 */
	handler->eap_ds->request->type.data = talloc_array(handler->eap_ds->request,
							   uint8_t, datalen);
	memcpy(handler->eap_ds->request->type.data, data, datalen);
	free(data);

	return 1;
}

/*
 *	The module name should be the only globally exported symbol.
 *	That is, everything else should be 'static'.
 */
rlm_eap_module_t rlm_eap_tnc = {
		"eap_tnc",
		tnc_attach,		/* attach */
		tnc_initiate,		/* Start the initial request */
		NULL,			/* authorization */
		mod_authenticate,	/* authentication */
		mod_detach		/* detach */
};
