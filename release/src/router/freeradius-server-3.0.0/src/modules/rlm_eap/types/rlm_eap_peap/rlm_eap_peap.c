/*
 * rlm_eap_peap.c  contains the interfaces that are called from eap
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

#include "eap_peap.h"

typedef struct rlm_eap_peap_t {
	/*
	 *	TLS configuration
	 */
	char	*tls_conf_name;
	fr_tls_server_conf_t *tls_conf;

	/*
	 *	Default tunneled EAP type
	 */
	char	*default_method_name;
	int	default_method;

	/*
	 *	Use the reply attributes from the tunneled session in
	 *	the non-tunneled reply to the client.
	 */
	int	use_tunneled_reply;

	/*
	 *	Use SOME of the request attributes from outside of the
	 *	tunneled session in the tunneled request
	 */
	int	copy_request_to_tunnel;

#ifdef WITH_PROXY
	/*
	 *	Proxy tunneled session as EAP, or as de-capsulated
	 *	protocol.
	 */
	int	proxy_tunneled_request_as_eap;
#endif

	/*
	 *	Virtual server for inner tunnel session.
	 */
  	char	*virtual_server;

	/*
	 * 	Do we do SoH request?
	 */
	int	soh;
  	char	*soh_virtual_server;

	/*
	 * 	Do we do require a client cert?
	 */
	int	req_client_cert;
} rlm_eap_peap_t;


static CONF_PARSER module_config[] = {
	{ "tls", PW_TYPE_STRING_PTR,
	  offsetof(rlm_eap_peap_t, tls_conf_name), NULL, NULL },

	{ "default_method", PW_TYPE_STRING_PTR,
	  offsetof(rlm_eap_peap_t, default_method_name), NULL, "mschapv2" },

	{ "copy_request_to_tunnel", PW_TYPE_BOOLEAN,
	  offsetof(rlm_eap_peap_t, copy_request_to_tunnel), NULL, "no" },

	{ "use_tunneled_reply", PW_TYPE_BOOLEAN,
	  offsetof(rlm_eap_peap_t, use_tunneled_reply), NULL, "no" },

#ifdef WITH_PROXY
	{ "proxy_tunneled_request_as_eap", PW_TYPE_BOOLEAN,
	  offsetof(rlm_eap_peap_t, proxy_tunneled_request_as_eap), NULL, "yes" },
#endif

	{ "virtual_server", PW_TYPE_STRING_PTR,
	  offsetof(rlm_eap_peap_t, virtual_server), NULL, NULL },

	{ "soh", PW_TYPE_BOOLEAN,
	  offsetof(rlm_eap_peap_t, soh), NULL, "no" },

	{ "require_client_cert", PW_TYPE_BOOLEAN,
	  offsetof(rlm_eap_peap_t, req_client_cert), NULL, "no" },

	{ "soh_virtual_server", PW_TYPE_STRING_PTR,
	  offsetof(rlm_eap_peap_t, soh_virtual_server), NULL, NULL },

 	{ NULL, -1, 0, NULL, NULL }	   /* end the list */
};


/*
 *	Attach the module.
 */
static int eappeap_attach(CONF_SECTION *cs, void **instance)
{
	rlm_eap_peap_t		*inst;

	*instance = inst = talloc_zero(cs, rlm_eap_peap_t);
	if (!inst) return -1;

	/*
	 *	Parse the configuration attributes.
	 */
	if (cf_section_parse(cs, inst, module_config) < 0) {
		return -1;
	}

	/*
	 *	Convert the name to an integer, to make it easier to
	 *	handle.
	 */
	inst->default_method = eap_name2type(inst->default_method_name);
	if (inst->default_method < 0) {
		ERROR("rlm_eap_peap: Unknown EAP type %s",
		       inst->default_method_name);
		return -1;
	}

	/*
	 *	Read tls configuration, either from group given by 'tls'
	 *	option, or from the eap-tls configuration.
	 */
	inst->tls_conf = eaptls_conf_parse(cs, "tls");

	if (!inst->tls_conf) {
		ERROR("rlm_eap_peap: Failed initializing SSL context");
		return -1;
	}

	return 0;
}

/*
 *	Free the PEAP per-session data
 */
static void peap_free(void *p)
{
	peap_tunnel_t *t = (peap_tunnel_t *) p;

	if (!t) return;

	pairfree(&t->username);
	pairfree(&t->state);
	pairfree(&t->accept_vps);
	pairfree(&t->soh_reply_vps);

	talloc_free(t);
}


/*
 *	Allocate the PEAP per-session data
 */
static peap_tunnel_t *peap_alloc(rlm_eap_peap_t *inst, eap_handler_t *handler)
{
	peap_tunnel_t *t;

	t = talloc_zero(handler, peap_tunnel_t);

	t->default_method = inst->default_method;
	t->copy_request_to_tunnel = inst->copy_request_to_tunnel;
	t->use_tunneled_reply = inst->use_tunneled_reply;
#ifdef WITH_PROXY
	t->proxy_tunneled_request_as_eap = inst->proxy_tunneled_request_as_eap;
#endif
	t->virtual_server = inst->virtual_server;
	t->soh = inst->soh;
	t->soh_virtual_server = inst->soh_virtual_server;
	t->session_resumption_state = PEAP_RESUMPTION_MAYBE;

	return t;
}

/*
 *	Send an initial eap-tls request to the peer, using the libeap functions.
 */
static int eappeap_initiate(void *type_arg, eap_handler_t *handler)
{
	int		status;
	tls_session_t	*ssn;
	rlm_eap_peap_t	*inst;
	VALUE_PAIR	*vp;
	int		client_cert = false;
	REQUEST		*request = handler->request;

	inst = type_arg;

	handler->tls = true;
	handler->finished = false;

	/*
	 *	Check if we need a client certificate.
	 */
	client_cert = inst->req_client_cert;

	/*
	 * EAP-TLS-Require-Client-Cert attribute will override
	 * the require_client_cert configuration option.
	 */
	vp = pairfind(handler->request->config_items, PW_EAP_TLS_REQUIRE_CLIENT_CERT, 0, TAG_ANY);
	if (vp) {
		client_cert = vp->vp_integer;
	}

	ssn = eaptls_session(inst->tls_conf, handler, client_cert);
	if (!ssn) {
		return 0;
	}

	handler->opaque = ((void *)ssn);
	handler->free_opaque = session_free;

	/*
	 *	Set up type-specific information.
	 */
	ssn->prf_label = "client EAP encryption";

	/*
	 *	As it is a poorly designed protocol, PEAP uses
	 *	bits in the TLS header to indicate PEAP
	 *	version numbers.  For now, we only support
	 *	PEAP version 0, so it doesn't matter too much.
	 *	However, if we support later versions of PEAP,
	 *	we will need this flag to indicate which
	 *	version we're currently dealing with.
	 */
	ssn->peap_flag = 0x00;

	/*
	 *	PEAP version 0 requires 'include_length = no',
	 *	so rather than hoping the user figures it out,
	 *	we force it here.
	 */
	ssn->length_flag = 0;

	/*
	 *	TLS session initialization is over.  Now handle TLS
	 *	related handshaking or application data.
	 */
	status = eaptls_start(handler->eap_ds, ssn->peap_flag);
	RDEBUG2("Start returned %d", status);
	if (status == 0) {
		return 0;
	}

	/*
	 *	The next stage to process the packet.
	 */
	handler->stage = AUTHENTICATE;

	return 1;
}

/*
 *	Do authentication, by letting EAP-TLS do most of the work.
 */
static int mod_authenticate(void *arg, eap_handler_t *handler)
{
	int rcode;
	fr_tls_status_t status;
	rlm_eap_peap_t *inst = (rlm_eap_peap_t *) arg;
	tls_session_t *tls_session = (tls_session_t *) handler->opaque;
	peap_tunnel_t *peap = tls_session->opaque;
	REQUEST *request = handler->request;

	/*
	 *	Session resumption requires the storage of data, so
	 *	allocate it if it doesn't already exist.
	 */
	if (!tls_session->opaque) {
		peap = tls_session->opaque = peap_alloc(inst, handler);
		tls_session->free_opaque = peap_free;
	}

	status = eaptls_process(handler);
	RDEBUG2("eaptls_process returned %d\n", status);
	switch (status) {
		/*
		 *	EAP-TLS handshake was successful, tell the
		 *	client to keep talking.
		 *
		 *	If this was EAP-TLS, we would just return
		 *	an EAP-TLS-Success packet here.
		 */
	case FR_TLS_SUCCESS:
		RDEBUG2("FR_TLS_SUCCESS");
		peap->status = PEAP_STATUS_TUNNEL_ESTABLISHED;
		break;

		/*
		 *	The TLS code is still working on the TLS
		 *	exchange, and it's a valid TLS request.
		 *	do nothing.
		 */
	case FR_TLS_HANDLED:
	  /*
	   *	FIXME: If the SSL session is established, grab the state
	   *	and EAP id from the inner tunnel, and update it with
	   *	the expected EAP id!
	   */
		RDEBUG2("FR_TLS_HANDLED");
		return 1;

		/*
		 *	Handshake is done, proceed with decoding tunneled
		 *	data.
		 */
	case FR_TLS_OK:
		RDEBUG2("FR_TLS_OK");
		break;

		/*
		 *	Anything else: fail.
		 */
	default:
		RDEBUG2("FR_TLS_OTHERS");
		return 0;
	}

	/*
	 *	Session is established, proceed with decoding
	 *	tunneled data.
	 */
	RDEBUG2("Session established.  Decoding tunneled attributes.");

	/*
	 *	We may need PEAP data associated with the session, so
	 *	allocate it here, if it wasn't already alloacted.
	 */
	if (!tls_session->opaque) {
		tls_session->opaque = peap_alloc(inst, handler);
		tls_session->free_opaque = peap_free;
	}

	/*
	 *	Process the PEAP portion of the request.
	 */
	rcode = eappeap_process(handler, tls_session);
	switch (rcode) {
	case RLM_MODULE_REJECT:
		eaptls_fail(handler, 0);
		return 0;

	case RLM_MODULE_HANDLED:
		eaptls_request(handler->eap_ds, tls_session);
		return 1;

	case RLM_MODULE_OK:
		/*
		 *	Move the saved VP's from the Access-Accept to
		 *	our Access-Accept.
		 */
		peap = tls_session->opaque;
		if (peap->soh_reply_vps) {
			RDEBUG2("Using saved attributes from the SoH reply");
			debug_pair_list(peap->soh_reply_vps);
			pairfilter(handler->request->reply,
				  &handler->request->reply->vps,
				  &peap->soh_reply_vps, 0, 0, TAG_ANY);
		}
		if (peap->accept_vps) {
			RDEBUG2("Using saved attributes from the original Access-Accept");
			debug_pair_list(peap->accept_vps);
			pairfilter(handler->request->reply,
				  &handler->request->reply->vps,
				  &peap->accept_vps, 0, 0, TAG_ANY);
		}

		/*
		 *	Success: Automatically return MPPE keys.
		 */
		return eaptls_success(handler, 0);

		/*
		 *	No response packet, MUST be proxying it.
		 *	The main EAP module will take care of discovering
		 *	that the request now has a "proxy" packet, and
		 *	will proxy it, rather than returning an EAP packet.
		 */
	case RLM_MODULE_UPDATED:
#ifdef WITH_PROXY
		rad_assert(handler->request->proxy != NULL);
#endif
		return 1;
		break;

	default:
		break;
	}

	eaptls_fail(handler, 0);
	return 0;
}


/*
 *	The module name should be the only globally exported symbol.
 *	That is, everything else should be 'static'.
 */
rlm_eap_module_t rlm_eap_peap = {
	"eap_peap",
	eappeap_attach,			/* attach */
	eappeap_initiate,		/* Start the initial request */
	NULL,				/* authorization */
	mod_authenticate,		/* authentication */
	NULL				/* detach */
};
