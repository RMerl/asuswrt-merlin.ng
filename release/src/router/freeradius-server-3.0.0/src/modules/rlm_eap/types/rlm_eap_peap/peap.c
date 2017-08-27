/*
 * peap.c contains the interfaces that are called from eap
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
 *   Copyright 2003 Alan DeKok <aland@freeradius.org>
 *   Copyright 2006 The FreeRADIUS server project
 */

RCSID("$Id$")
USES_APPLE_DEPRECATED_API	/* OpenSSL API has been deprecated by Apple */

#include "eap_peap.h"

static int setup_fake_request(REQUEST *request, REQUEST *fake, peap_tunnel_t *t);

/*
 *	Send protected EAP-Failure
 *
 *       Result-TLV = Failure
 */
static int eappeap_failure(eap_handler_t *handler, tls_session_t *tls_session)
{
	uint8_t tlv_packet[11];
	REQUEST *request = handler->request;

	RDEBUG2("FAILURE");

	tlv_packet[0] = PW_EAP_REQUEST;
	tlv_packet[1] = handler->eap_ds->response->id +1;
	tlv_packet[2] = 0;
	tlv_packet[3] = 11;	/* length of this packet */
	tlv_packet[4] = PW_EAP_TLV;
	tlv_packet[5] = 0x80;
	tlv_packet[6] = EAP_TLV_ACK_RESULT;
	tlv_packet[7] = 0;
	tlv_packet[8] = 2;	/* length of the data portion */
	tlv_packet[9] = 0;
	tlv_packet[10] = EAP_TLV_FAILURE;

	(tls_session->record_plus)(&tls_session->clean_in, tlv_packet, 11);

	/*
	 *	FIXME: Check the return code.
	 */
	tls_handshake_send(request, tls_session);

	return 1;
}


/*
 *	Send protected EAP-Success
 *
 *       Result-TLV = Success
 */
static int eappeap_success(eap_handler_t *handler, tls_session_t *tls_session)
{
	uint8_t tlv_packet[11];
	REQUEST *request = handler->request;

	RDEBUG2("SUCCESS");

	tlv_packet[0] = PW_EAP_REQUEST;
	tlv_packet[1] = handler->eap_ds->response->id +1;
	tlv_packet[2] = 0;
	tlv_packet[3] = 11;	/* length of this packet */
	tlv_packet[4] = PW_EAP_TLV;
	tlv_packet[5] = 0x80;	/* mandatory AVP */
	tlv_packet[6] = EAP_TLV_ACK_RESULT;
	tlv_packet[7] = 0;
	tlv_packet[8] = 2;	/* length of the data portion */
	tlv_packet[9] = 0;
	tlv_packet[10] = EAP_TLV_SUCCESS;

	(tls_session->record_plus)(&tls_session->clean_in, tlv_packet, 11);

	/*
	 *	FIXME: Check the return code.
	 */
	tls_handshake_send(request, tls_session);

	return 1;
}


static int eappeap_identity(eap_handler_t *handler, tls_session_t *tls_session)
{
	eap_packet_raw_t eap_packet;

	eap_packet.code = PW_EAP_REQUEST;
	eap_packet.id = handler->eap_ds->response->id + 1;
	eap_packet.length[0] = 0;
	eap_packet.length[1] = EAP_HEADER_LEN + 1;
	eap_packet.data[0] = PW_EAP_IDENTITY;

	(tls_session->record_plus)(&tls_session->clean_in,
				  &eap_packet, sizeof(eap_packet));

	tls_handshake_send(handler->request, tls_session);
	(tls_session->record_init)(&tls_session->clean_in);

	return 1;
}

/*
 * Send an MS SoH request
 */
static int eappeap_soh(eap_handler_t *handler, tls_session_t *tls_session)
{
	uint8_t tlv_packet[20];

	tlv_packet[0] = 254;	/* extended type */

	tlv_packet[1] = 0;
	tlv_packet[2] = 0x01;	/* ms vendor */
	tlv_packet[3] = 0x37;

	tlv_packet[4] = 0;	/* ms soh eap */
	tlv_packet[5] = 0;
	tlv_packet[6] = 0;
	tlv_packet[7] = 0x21;

	tlv_packet[8] = 0;	/* vendor-spec tlv */
	tlv_packet[9] = 7;

	tlv_packet[10] = 0;
	tlv_packet[11] = 8;	/* payload len */

	tlv_packet[12] = 0;	/* ms vendor */
	tlv_packet[13] = 0;
	tlv_packet[14] = 0x01;
	tlv_packet[15] = 0x37;

	tlv_packet[16] = 0;
	tlv_packet[17] = 2;
	tlv_packet[18] = 0;
	tlv_packet[19] = 0;

	(tls_session->record_plus)(&tls_session->clean_in, tlv_packet, 20);
	tls_handshake_send(handler->request, tls_session);
	return 1;
}

static void eapsoh_verify(REQUEST *request, RADIUS_PACKET *packet,
			  uint8_t const *data, unsigned int data_len) {

	VALUE_PAIR *vp;
	uint8_t eap_method_base;
	uint32_t eap_vendor;
	uint32_t eap_method;
	int rv;

	vp = pairmake(packet, &packet->vps, "SoH-Supported", "no", T_OP_EQ);
	if (data && data[0] == PW_EAP_NAK) {
		RDEBUG("SoH - client NAKed");
		return;
	}

	if (!data || data_len < 8) {
		RDEBUG("SoH - eap payload too short");
		return;
	}

	eap_method_base = *data++;
	if (eap_method_base != 254) {
		RDEBUG("SoH - response is not extended EAP: %i", eap_method_base);
		return;
	}

	eap_vendor = soh_pull_be_24(data); data += 3;
	if (eap_vendor != 0x137) {
		RDEBUG("SoH - extended eap vendor %08x is not Microsoft", eap_vendor);
		return;
	}

	eap_method = soh_pull_be_32(data); data += 4;
	if (eap_method != 0x21) {
		RDEBUG("SoH - response eap type %08x is not EAP-SoH", eap_method);
		return;
	}


	rv = soh_verify(request, data, data_len - 8);
	if (rv<0) {
		RDEBUG("SoH - error decoding payload: %s", fr_strerror());
	} else {
		vp->vp_integer = 1;
	}
}

/*
 *	Verify the tunneled EAP message.
 */
static int eapmessage_verify(REQUEST *request,
			     uint8_t const *data, unsigned int data_len)
{
	eap_packet_raw_t const *eap_packet = (eap_packet_raw_t const *) data;
	eap_type_t eap_method;

	/*
	 *	No data, OR only 1 byte of EAP type.
	 */
	if (!data || (data_len == 0) ||
	    ((data_len <= 1) && (data[0] != PW_EAP_IDENTITY))) {
		return 0;
	}

	eap_method = *data;
	switch (eap_method) {
	case PW_EAP_IDENTITY:
		if (data_len == 1) {
			RDEBUG2("Identity - ");
			return 1;
		}
		RDEBUG2("Identity - %*s",
		       data_len - 1, data + 1);
		return 1;
		break;

		/*
		 *	If the first byte of the packet is
		 *	EAP-Response, and the EAP data is a TLV,
		 *	then it looks OK...
		 */
	case PW_EAP_RESPONSE:
		if (eap_packet->data[0] == PW_EAP_TLV) {
			RDEBUG2("Received EAP-TLV response.");
			return 1;
		}
		RDEBUG2("Got something weird.");
		break;


		/*
		 *	We normally do Microsoft MS-CHAPv2 (26), versus
		 *	Cisco MS-CHAPv2 (29).
		 */
	case PW_EAP_MSCHAPV2:
	default:
		RDEBUG2("EAP type %s (%d)", eap_type2name(eap_method),
			eap_method);
		return 1;
		break;
	}

	return 0;
}

/*
 *	Convert a pseudo-EAP packet to a list of VALUE_PAIR's.
 */
static VALUE_PAIR *eap2vp(REQUEST *request, RADIUS_PACKET *packet,
			  EAP_DS *eap_ds,
			  uint8_t const *data, size_t data_len)
{
	size_t total;
	uint8_t *p;
	VALUE_PAIR *vp = NULL, *head = NULL;
	vp_cursor_t cursor;

	if (data_len > 65535) return NULL; /* paranoia */

	vp = paircreate(packet, PW_EAP_MESSAGE, 0);
	if (!vp) {
		RDEBUG2("Failure in creating VP");
		return NULL;
	}

	total = data_len;
	if (total > 249) total = 249;

	/*
	 *	Hand-build an EAP packet from the crap in PEAP version 0.
	 */
	vp->length = EAP_HEADER_LEN + total;
	vp->vp_octets = p = talloc_array(vp, uint8_t, vp->length);

	p[0] = PW_EAP_RESPONSE;
	p[1] = eap_ds->response->id;
	p[2] = (data_len + EAP_HEADER_LEN) >> 8;
	p[3] = (data_len + EAP_HEADER_LEN) & 0xff;

	memcpy(p + EAP_HEADER_LEN, data, total);

	paircursor(&cursor, &head);
	pairinsert(&cursor, vp);
	while (total < data_len) {
		vp = paircreate(packet, PW_EAP_MESSAGE, 0);
		if (!vp) {
			RDEBUG2("Failure in creating VP");
			pairfree(&head);
			return NULL;
		}

		pairmemcpy(vp, data + total, (data_len - total));

		total += vp->length;

		pairinsert(&cursor, vp);
	}

	return head;
}


/*
 *	Convert a list of VALUE_PAIR's to an EAP packet, through the
 *	simple expedient of dumping the EAP message
 */
static int vp2eap(REQUEST *request, tls_session_t *tls_session, VALUE_PAIR *vp)
{
	rad_assert(vp != NULL);
	VALUE_PAIR *this;
	vp_cursor_t cursor;

	/*
	 *	Skip the id, code, and length.  Just write the EAP
	 *	type & data to the client.
	 */
#ifndef NDEBUG
	if ((debug_flag > 2) && fr_log_fp) {
		size_t i, total, start = EAP_HEADER_LEN;
		total = 0;

		for (this = paircursor(&cursor, &vp); this; this = pairnext(&cursor)) {
			for (i = start; i < vp->length; i++) {
				if ((total & 0x0f) == 0) {
					fprintf(fr_log_fp, "  PEAP tunnel data out %04x: ", (int) total);
				}
				fprintf(fr_log_fp, "%02x ", vp->vp_octets[i]);

				if ((total & 0x0f) == 0x0f) {
					fprintf(fr_log_fp, "\n");
				}

				total++;
			}

			start = 0;
		}

		if ((total & 0x0f) != 0) {
			fprintf(fr_log_fp, "\n");
		}
	}
#endif

	/*
	 *	Send the EAP data, WITHOUT the header.
	 */
	(tls_session->record_plus)(&tls_session->clean_in, vp->vp_octets + EAP_HEADER_LEN, vp->length - EAP_HEADER_LEN);

	/*
	 *	Send the rest of the EAP data.
	 */
	for (this = paircursor(&cursor, &vp);
	     this;
	     this = pairnext(&cursor)) {
		(tls_session->record_plus)(&tls_session->clean_in, this->vp_octets, this->length);
	}

	tls_handshake_send(request, tls_session);

	return 1;
}


/*
 *	See if there's a TLV in the response.
 */
static int eappeap_check_tlv(REQUEST *request, uint8_t const *data,
			     size_t data_len)
{
	eap_packet_raw_t const *eap_packet = (eap_packet_raw_t const *) data;

	if (data_len < 11) return 0;

	/*
	 *	Look for success or failure.
	 */
	if ((eap_packet->code == PW_EAP_RESPONSE) &&
	    (eap_packet->data[0] == PW_EAP_TLV)) {
		if (data[10] == EAP_TLV_SUCCESS) {
			return 1;
		}

		if (data[10] == EAP_TLV_FAILURE) {
			RDEBUG2("Client rejected our response.  The password is probably incorrect.");
			return 0;
		}
	}

	RDEBUG("Unknown TLV %02x", data[10]);

	return 0;
}


/*
 *	Use a reply packet to determine what to do.
 */
static int process_reply(eap_handler_t *handler, tls_session_t *tls_session,
			 REQUEST *request, RADIUS_PACKET *reply)
{
	int rcode = RLM_MODULE_REJECT;
	VALUE_PAIR *vp;
	peap_tunnel_t *t = tls_session->opaque;

	if ((debug_flag > 0) && fr_log_fp) {
		RDEBUG("Got tunneled reply RADIUS code %d",
		       reply->code);

		debug_pair_list(reply->vps);
	}

	switch (reply->code) {
	case PW_AUTHENTICATION_ACK:
		RDEBUG2("Tunneled authentication was successful.");
		t->status = PEAP_STATUS_SENT_TLV_SUCCESS;
		eappeap_success(handler, tls_session);
		rcode = RLM_MODULE_HANDLED;

		/*
		 *	If we've been told to use the attributes from
		 *	the reply, then do so.
		 *
		 *	WARNING: This may leak information about the
		 *	tunneled user!
		 */
		if (t->use_tunneled_reply) {
			RDEBUG2("Saving tunneled attributes for later");

			/*
			 *	Clean up the tunneled reply.
			 */
			pairdelete(&reply->vps, PW_PROXY_STATE, 0, TAG_ANY);
			pairdelete(&reply->vps, PW_EAP_MESSAGE, 0, TAG_ANY);
			pairdelete(&reply->vps, PW_MESSAGE_AUTHENTICATOR, 0, TAG_ANY);

			/*
			 *	Delete MPPE keys & encryption policy.  We don't
			 *	want these here.
			 */
			pairdelete(&reply->vps, 7, VENDORPEC_MICROSOFT, TAG_ANY);
			pairdelete(&reply->vps, 8, VENDORPEC_MICROSOFT, TAG_ANY);
			pairdelete(&reply->vps, 16, VENDORPEC_MICROSOFT, TAG_ANY);
			pairdelete(&reply->vps, 17, VENDORPEC_MICROSOFT, TAG_ANY);

			rad_assert(!t->accept_vps);
			pairfilter(t, &t->accept_vps, &reply->vps, 0, 0, TAG_ANY);
			rad_assert(!reply->vps);
		}
		break;

	case PW_AUTHENTICATION_REJECT:
		RDEBUG2("Tunneled authentication was rejected.");
		t->status = PEAP_STATUS_SENT_TLV_FAILURE;
		eappeap_failure(handler, tls_session);
		rcode = RLM_MODULE_HANDLED;
		break;

	case PW_ACCESS_CHALLENGE:
		RDEBUG2("Got tunneled Access-Challenge");

		/*
		 *	Keep the State attribute, if necessary.
		 *
		 *	Get rid of the old State, too.
		 */
		pairfree(&t->state);
		pairfilter(t, &t->state, &(reply->vps), PW_STATE, 0, TAG_ANY);

		/*
		 *	PEAP takes only EAP-Message attributes inside
		 *	of the tunnel.  Any Reply-Message in the
		 *	Access-Challenge is ignored.
		 */
		vp = NULL;
		pairfilter(t, &vp, &(reply->vps), PW_EAP_MESSAGE, 0, TAG_ANY);

		/*
		 *	Handle EAP-MSCHAP-V2, where Access-Accept's
		 *	from the home server may contain MS-CHAP-Success,
		 *	which the module turns into challenges, so that
		 *	the client may respond to the challenge with
		 *	an "ack" packet.
		 */
		if (t->home_access_accept && t->use_tunneled_reply) {
			RDEBUG2("Saving tunneled attributes for later");

			/*
			 *	Clean up the tunneled reply.
			 */
			pairdelete(&reply->vps, PW_PROXY_STATE, 0, TAG_ANY);
			pairdelete(&reply->vps, PW_MESSAGE_AUTHENTICATOR, 0, TAG_ANY);

			rad_assert(!t->accept_vps);
			pairfilter(t, &t->accept_vps, &reply->vps,
				  0, 0, TAG_ANY);
			rad_assert(!reply->vps);
		}

		/*
		 *	Handle the ACK, by tunneling any necessary reply
		 *	VP's back to the client.
		 */
		if (vp) {
			vp2eap(request, tls_session, vp);
			pairfree(&vp);
		}

		rcode = RLM_MODULE_HANDLED;
		break;

	default:
		RDEBUG2("Unknown RADIUS packet type %d: rejecting tunneled user", reply->code);
		rcode = RLM_MODULE_REJECT;
		break;
	}

	return rcode;
}

#ifdef WITH_PROXY
/*
 *	Do post-proxy processing,
 */
static int eappeap_postproxy(eap_handler_t *handler, void *data)
{
	int rcode;
	tls_session_t *tls_session = (tls_session_t *) data;
	REQUEST *fake, *request = handler->request;

	rad_assert(request != NULL);
	RDEBUG2("Passing reply from proxy back into the tunnel.");

	/*
	 *	If there was a fake request associated with the proxied
	 *	request, do more processing of it.
	 */
	fake = (REQUEST *) request_data_get(handler->request,
					    handler->request->proxy,
					    REQUEST_DATA_EAP_MSCHAP_TUNNEL_CALLBACK);

	/*
	 *	Do the callback, if it exists, and if it was a success.
	 */
	if (fake && (handler->request->proxy_reply->code == PW_AUTHENTICATION_ACK)) {
		peap_tunnel_t *t = tls_session->opaque;

		t->home_access_accept = true;

		/*
		 *	Terrible hacks.
		 */
		rad_assert(!fake->packet);
		fake->packet = request->proxy;
		fake->packet->src_ipaddr = request->packet->src_ipaddr;
		request->proxy = NULL;

		rad_assert(!fake->reply);
		fake->reply = request->proxy_reply;
		request->proxy_reply = NULL;

		if ((debug_flag > 0) && fr_log_fp) {
			fprintf(fr_log_fp, "server %s {\n", fake->server);
		}

		/*
		 *	Perform a post-auth stage, which will get the EAP
		 *	handler, too...
		 */
		fake->options &= ~RAD_REQUEST_OPTION_PROXY_EAP;
		RDEBUG2("Passing reply back for EAP-MS-CHAP-V2");
		process_post_proxy(0, fake);

		/*
		 *	FIXME: If rcode returns fail, do something
		 *	intelligent...
		 */
		rcode = rad_postauth(fake);

		if ((debug_flag > 0) && fr_log_fp) {
			fprintf(fr_log_fp, "} # server %s\n", fake->server);

			RDEBUG("Final reply from tunneled session code %d",
			       fake->reply->code);

			debug_pair_list(fake->reply->vps);
		}

		/*
		 *	Terrible hacks.
		 */
		request->proxy = fake->packet;
		fake->packet = NULL;
		request->proxy_reply = fake->reply;
		fake->reply = NULL;

		/*
		 *	And we're done with this request.
		 */

		switch (rcode) {
		case RLM_MODULE_FAIL:
			request_free(&fake);
			eaptls_fail(handler, 0);
			return 0;
			break;

		default:  /* Don't Do Anything */
			RDEBUG2("Got reply %d", request->proxy_reply->code);
			break;
		}
	}
	request_free(&fake);	/* robust if !fake */

	/*
	 *	If there was no EAP-Message in the reply packet, then
	 *	we know that we're supposed to re-run the "authenticate"
	 *	stage, in order to get the right kind of handling...
	 */

	/*
	 *	Process the reply from the home server.
	 */

	rcode = process_reply(handler, tls_session, handler->request,
			      handler->request->proxy_reply);

	/*
	 *	The proxy code uses the reply from the home server as
	 *	the basis for the reply to the NAS.  We don't want that,
	 *	so we toss it, after we've had our way with it.
	 */
	pairfree(&handler->request->proxy_reply->vps);

	switch (rcode) {
	case RLM_MODULE_REJECT:
		RDEBUG2("Reply was rejected");
		eaptls_fail(handler, 0);
		return 0;

	case RLM_MODULE_HANDLED:
		RDEBUG2("Reply was handled");
		eaptls_request(handler->eap_ds, tls_session);
		return 1;

	case RLM_MODULE_OK:
		RDEBUG2("Reply was OK");

		/*
		 *	Success: Automatically return MPPE keys.
		 */
		return eaptls_success(handler, 0);

	default:
		RDEBUG2("Reply was unknown.");
		break;
	}

	eaptls_fail(handler, 0);
	return 0;
}
#endif


static char const *peap_state(peap_tunnel_t *t)
{
	switch (t->status) {
		case PEAP_STATUS_TUNNEL_ESTABLISHED:
			return "TUNNEL ESTABLISHED";
		case PEAP_STATUS_WAIT_FOR_SOH_RESPONSE:
			return "WAITING FOR SOH RESPONSE";
		case PEAP_STATUS_INNER_IDENTITY_REQ_SENT:
			return "WAITING FOR INNER IDENTITY";
		case PEAP_STATUS_SENT_TLV_SUCCESS:
			return "send tlv success";
		case PEAP_STATUS_SENT_TLV_FAILURE:
			return "send tlv failure";
		case PEAP_STATUS_PHASE2_INIT:
			return "phase2_init";
		case PEAP_STATUS_PHASE2:
			return "phase2";
		default:
			break;
	}
	return "?";
}

static void print_tunneled_data(uint8_t const *data, size_t data_len)
{
	size_t i;

	if ((debug_flag > 2) && fr_log_fp) {
		for (i = 0; i < data_len; i++) {
		  if ((i & 0x0f) == 0) fprintf(fr_log_fp, "  PEAP tunnel data in %02x: ", (int) i);

			fprintf(fr_log_fp, "%02x ", data[i]);

			if ((i & 0x0f) == 0x0f) fprintf(fr_log_fp, "\n");
		}
		if ((data_len & 0x0f) != 0) fprintf(fr_log_fp, "\n");
	}
}


/*
 *	Process the pseudo-EAP contents of the tunneled data.
 */
rlm_rcode_t eappeap_process(eap_handler_t *handler, tls_session_t *tls_session)
{
	peap_tunnel_t	*t = tls_session->opaque;
	REQUEST		*fake;
	VALUE_PAIR	*vp;
	rlm_rcode_t	rcode = RLM_MODULE_REJECT;
	uint8_t const	*data;
	unsigned int	data_len;
	char *p;

	REQUEST *request = handler->request;
	EAP_DS *eap_ds = handler->eap_ds;

	rad_assert(request != NULL);

	/*
	 *	Just look at the buffer directly, without doing
	 *	record_minus.  This lets us avoid another data copy.
	 */
	data_len = tls_session->clean_out.used;
	tls_session->clean_out.used = 0;
	data = tls_session->clean_out.data;

	RDEBUG2("Peap state %s", peap_state(t));

	if ((t->status != PEAP_STATUS_TUNNEL_ESTABLISHED) &&
	    !eapmessage_verify(request, data, data_len)) {
		RDEBUG2("FAILED processing PEAP: Tunneled data is invalid.");
		if (debug_flag > 2) print_tunneled_data(data, data_len);
		return RLM_MODULE_REJECT;
	}

	switch (t->status) {
	case PEAP_STATUS_TUNNEL_ESTABLISHED:
		/* FIXME: should be no data in the buffer here, check & assert? */

		if (SSL_session_reused(tls_session->ssl)) {
			RDEBUG2("Skipping Phase2 because of session resumption");
			t->session_resumption_state = PEAP_RESUMPTION_YES;
			if (t->soh) {
				t->status = PEAP_STATUS_WAIT_FOR_SOH_RESPONSE;
				RDEBUG2("Requesting SoH from client");
				eappeap_soh(handler, tls_session);
				return RLM_MODULE_HANDLED;
			}
			/* we're good, send success TLV */
			t->status = PEAP_STATUS_SENT_TLV_SUCCESS;
			eappeap_success(handler, tls_session);

		} else {
			/* send an identity request */
			t->session_resumption_state = PEAP_RESUMPTION_NO;
			t->status = PEAP_STATUS_INNER_IDENTITY_REQ_SENT;
			eappeap_identity(handler, tls_session);
		}
		return RLM_MODULE_HANDLED;

	case PEAP_STATUS_INNER_IDENTITY_REQ_SENT:
		/* we're expecting an identity response */
		if (data[0] != PW_EAP_IDENTITY) {
			RDEBUG("Expected EAP-Identity, got something else.");
			return RLM_MODULE_REJECT;
		}

		/*
		 *	Save it for later.
		 */
		t->username = pairmake(t, NULL, "User-Name", NULL, T_OP_EQ);
		rad_assert(t->username != NULL);

		t->username->vp_strvalue = p = talloc_array(t->username, char, data_len);
		memcpy(p, data + 1, data_len - 1);
		t->username->length = data_len - 1;
		p[t->username->length] = 0;
		RDEBUG("Got inner identity '%s'", t->username->vp_strvalue);
		if (t->soh) {
			t->status = PEAP_STATUS_WAIT_FOR_SOH_RESPONSE;
			RDEBUG2("Requesting SoH from client");
			eappeap_soh(handler, tls_session);
			return RLM_MODULE_HANDLED;
		}
		t->status = PEAP_STATUS_PHASE2_INIT;
		break;

	case PEAP_STATUS_WAIT_FOR_SOH_RESPONSE:
		fake = request_alloc_fake(request);
		rad_assert(!fake->packet->vps);
		eapsoh_verify(request, fake->packet, data, data_len);
		setup_fake_request(request, fake, t);

		if (t->soh_virtual_server) {
			fake->server = t->soh_virtual_server;
		}
		RDEBUG("Sending SoH request to server %s", fake->server ? fake->server : "NULL");
		debug_pair_list(fake->packet->vps);
		RDEBUG("server %s {", fake->server);
		rad_virtual_server(fake);
		RDEBUG("} # server %s", fake->server);
		RDEBUG("Got SoH reply");
		debug_pair_list(fake->reply->vps);

		if (fake->reply->code != PW_AUTHENTICATION_ACK) {
			RDEBUG2("SoH was rejected");
			request_free(&fake);
			t->status = PEAP_STATUS_SENT_TLV_FAILURE;
			eappeap_failure(handler, tls_session);
			return RLM_MODULE_HANDLED;
		}

		/* save the SoH VPs */
		rad_assert(!t->soh_reply_vps);
		pairfilter(t, &t->soh_reply_vps, &fake->reply->vps, 0, 0, TAG_ANY);
		rad_assert(!fake->reply->vps);
		request_free(&fake);

		if (t->session_resumption_state == PEAP_RESUMPTION_YES) {
			/* we're good, send success TLV */
			t->status = PEAP_STATUS_SENT_TLV_SUCCESS;
			eappeap_success(handler, tls_session);
			return RLM_MODULE_HANDLED;
		}

		t->status = PEAP_STATUS_PHASE2_INIT;
		break;


	/*
	 *	If we authenticated the user, then it's OK.
	 */
	case PEAP_STATUS_SENT_TLV_SUCCESS:
		if (eappeap_check_tlv(request, data, data_len)) {
			RDEBUG2("Success");
			return RLM_MODULE_OK;
		}

		/*
		 *	Otherwise, the client rejected the session
		 *	resumption.  If the session is being re-used,
		 *	we need to do a full authentication.
		 *
		 *	We do this by sending an EAP-Identity request
		 *	inside of the PEAP tunnel.
		 */
		if (t->session_resumption_state == PEAP_RESUMPTION_YES) {
			RDEBUG2("Client rejected session resumption.  Re-starting full authentication");

			/*
			 *	Mark session resumption status.
			 */
			t->status = PEAP_STATUS_INNER_IDENTITY_REQ_SENT;
			t->session_resumption_state = PEAP_RESUMPTION_NO;

			eappeap_identity(handler, tls_session);
			return RLM_MODULE_HANDLED;
		}

		RDEBUG2("We sent a success, but received something weird in return.");
		return RLM_MODULE_REJECT;

	/*
	 *	Damned if I know why the clients continue sending EAP
	 *	packets after we told them to f*ck off.
	 */
	case PEAP_STATUS_SENT_TLV_FAILURE:
		RDEBUG(" The users session was previously rejected: returning reject (again.)");
		RDEBUG(" *** This means you need to read the PREVIOUS messages in the debug output");
		RDEBUG(" *** to find out the reason why the user was rejected.");
		RDEBUG(" *** Look for \"reject\" or \"fail\".  Those earlier messages will tell you.");
		RDEBUG(" *** what went wrong, and how to fix the problem.");
		return RLM_MODULE_REJECT;

		case PEAP_STATUS_PHASE2_INIT:
			RDEBUG("In state machine in phase2 init?");

		case PEAP_STATUS_PHASE2:
			break;

		default:
			RDEBUG2("Unhandled state in peap");
			return RLM_MODULE_REJECT;
	}

	fake = request_alloc_fake(request);

	rad_assert(!fake->packet->vps);

	switch (t->status) {
		/*
		 *	If we're in PHASE2_INIT, the phase2 method hasn't been
		 *	sent an Identity packet yet; do so from the stored
		 *	username and this will kick off the phase2 eap method
		 */

	case PEAP_STATUS_PHASE2_INIT: {
		size_t len = t->username->length + EAP_HEADER_LEN + 1;
		uint8_t *q;

		t->status = PEAP_STATUS_PHASE2;

		vp = paircreate(fake->packet, PW_EAP_MESSAGE, 0);
		vp->length = len;
		vp->vp_octets = q = talloc_array(vp, uint8_t, vp->length);

		q[0] = PW_EAP_RESPONSE;
		q[1] = eap_ds->response->id;
		q[2] = (len >> 8) & 0xff;
		q[3] = len & 0xff;
		q[4] = PW_EAP_IDENTITY;

		memcpy(q + EAP_HEADER_LEN + 1,
		       t->username->vp_strvalue, t->username->length);

		pairadd(&fake->packet->vps, vp);

		if (t->default_method != 0) {
			RDEBUG2("Setting default EAP type for tunneled EAP session.");
			vp = pairmake(fake, &fake->config_items, "EAP-Type", "0", T_OP_EQ);
			vp->vp_integer = t->default_method;
		}
		break; }

	case PEAP_STATUS_PHASE2:
		fake->packet->vps = eap2vp(request, fake->packet,
					   eap_ds, data, data_len);
		if (!fake->packet->vps) {
			request_free(&fake);
			RDEBUG2("Unable to convert tunneled EAP packet to internal server data structures");
			return RLM_MODULE_REJECT;
		}
		break;

	default:
		RDEBUG("Invalid state change in PEAP");
		return RLM_MODULE_REJECT;
	}

	if ((debug_flag > 0) && fr_log_fp) {
		RDEBUG("Got tunneled request");

		debug_pair_list(fake->packet->vps);

		fprintf(fr_log_fp, "server %s {\n",
			(!fake->server) ? "" : fake->server);
	}

	/*
	 *	Update other items in the REQUEST data structure.
	 */
	if (!t->username) {
		/*
		 *	There's no User-Name in the tunneled session,
		 *	so we add one here, by pulling it out of the
		 *	EAP-Identity packet.
		 */
		if ((data[0] == PW_EAP_IDENTITY) && (data_len > 1)) {
			t->username = pairmake(t, NULL, "User-Name", NULL, T_OP_EQ);
			rad_assert(t->username != NULL);

			t->username->vp_strvalue = p = talloc_array(t->username, char, data_len);
			memcpy(p, data + 1, data_len - 1);
			t->username->length = data_len - 1;
			p[t->username->length] = 0;
			DEBUG2("  PEAP: Got tunneled identity of %s", t->username->vp_strvalue);

			/*
			 *	If there's a default EAP type,
			 *	set it here.
			 */
			if (t->default_method != 0) {
				DEBUG2("  PEAP: Setting default EAP type for tunneled EAP session.");
				vp = pairmake(fake, &fake->config_items, "EAP-Type", "0", T_OP_EQ);
				vp->vp_integer = t->default_method;
			}
		}
	} /* else there WAS a t->username */

	setup_fake_request(request, fake, t);

	if ((vp = pairfind(request->config_items, PW_VIRTUAL_SERVER, 0, TAG_ANY)) != NULL) {
		fake->server = vp->vp_strvalue;

	} else if (t->virtual_server) {
		fake->server = t->virtual_server;

	} /* else fake->server == request->server */

	if ((debug_flag > 0) && fr_log_fp) {
		fprintf(fr_log_fp, "Sending tunneled request\n");

		debug_pair_list(fake->packet->vps);

		fprintf(fr_log_fp, "server %s {\n",
			(!fake->server) ? "" : fake->server);
	}

	/*
	 *	Call authentication recursively, which will
	 *	do PAP, CHAP, MS-CHAP, etc.
	 */
	rad_virtual_server(fake);

	/*
	 *	Note that we don't do *anything* with the reply
	 *	attributes.
	 */
	if ((debug_flag > 0) && fr_log_fp) {
		fprintf(fr_log_fp, "} # server %s\n",
			(!fake->server) ? "" : fake->server);

		RDEBUG("Got tunneled reply code %d", fake->reply->code);

		debug_pair_list(fake->reply->vps);
	}

	/*
	 *	Decide what to do with the reply.
	 */
	switch (fake->reply->code) {
	case 0:			/* No reply code, must be proxied... */
#ifdef WITH_PROXY
		vp = pairfind(fake->config_items, PW_PROXY_TO_REALM, 0, TAG_ANY);

		if (vp) {
			eap_tunnel_data_t *tunnel;

			/*
			 *	The tunneled request was NOT handled,
			 *	it has to be proxied.  This means that
			 *	the "authenticate" stage was never
			 *	performed.
			 *
			 *	If we are told to NOT proxy the
			 *	tunneled request as EAP, then this
			 *	means that we've got to decode it,
			 *	which means that we MUST run the
			 *	"authenticate" portion by hand, here.
			 *
			 *	Once the tunneled EAP session is ALMOST
			 *	done, THEN we proxy it...
			 */
			if (!t->proxy_tunneled_request_as_eap) {
				fake->options |= RAD_REQUEST_OPTION_PROXY_EAP;

				/*
				 *	Hmm... should we check for
				 *	Auth-Type & EAP-Message here?
				 */


				/*
				 *	Run the EAP authentication.
				 */
				DEBUG2("  PEAP: Calling authenticate in order to initiate tunneled EAP session.");
				rcode = process_authenticate(PW_AUTHTYPE_EAP, fake);
				if (rcode == RLM_MODULE_OK) {
					/*
					 *	Authentication succeeded! Rah!
					 */
					fake->reply->code = PW_AUTHENTICATION_ACK;
					goto do_process;
				}

				if (rcode != RLM_MODULE_HANDLED) {
					DEBUG2("  PEAP: Can't handle the return code %d", rcode);
					rcode = RLM_MODULE_REJECT;
					goto done;
				}

				/*
				 *	The module decided it wasn't
				 *	done.  Handle it like normal.
				 */
				if ((fake->options & RAD_REQUEST_OPTION_PROXY_EAP) == 0) {
					DEBUG2("    PEAP: Cancelling proxy to realm %s until the tunneled EAP session has been established", vp->vp_strvalue);
					goto do_process;
				}

				/*
				 *	The module has decoded the
				 *	EAP-Message into another set
				 *	of attributes.
				 */
				pairdelete(&fake->packet->vps,
					   PW_EAP_MESSAGE, 0, TAG_ANY);
			}

			DEBUG2("  PEAP: Tunneled authentication will be proxied to %s", vp->vp_strvalue);

			/*
			 *	Tell the original request that it's going
			 *	to be proxied.
			 */
			pairfilter(request, &(request->config_items),
				   &(fake->config_items),
				   PW_PROXY_TO_REALM, 0, TAG_ANY);

			/*
			 *	Seed the proxy packet with the
			 *	tunneled request.
			 */
			rad_assert(!request->proxy);
			request->proxy = fake->packet;
			memset(&request->proxy->src_ipaddr, 0,
			       sizeof(request->proxy->src_ipaddr));
			memset(&request->proxy->dst_ipaddr, 0,
			       sizeof(request->proxy->dst_ipaddr));
			request->proxy->src_port = 0;
			request->proxy->dst_port = 0;
			fake->packet = NULL;
			rad_free(&fake->reply);
			fake->reply = NULL;

			/*
			 *	Set up the callbacks for the tunnel
			 */
			tunnel = talloc_zero(request, eap_tunnel_data_t);
			tunnel->tls_session = tls_session;
			tunnel->callback = eappeap_postproxy;

			/*
			 *	Associate the callback with the request.
			 */
			rcode = request_data_add(request,
						 request->proxy,
						 REQUEST_DATA_EAP_TUNNEL_CALLBACK,
						 tunnel, false);
			rad_assert(rcode == 0);

			/*
			 *	We're not proxying it as EAP, so we've got
			 *	to do the callback later.
			 */
			if ((fake->options & RAD_REQUEST_OPTION_PROXY_EAP) != 0) {
				DEBUG2("  PEAP: Remembering to do EAP-MS-CHAP-V2 post-proxy.");

				/*
				 *	rlm_eap.c has taken care of associating
				 *	the handler with the fake request.
				 *
				 *	So we associate the fake request with
				 *	this request.
				 */
				talloc_set_destructor(fake, request_opaque_free);
				rcode = request_data_add(request, request->proxy,
							 REQUEST_DATA_EAP_MSCHAP_TUNNEL_CALLBACK,
							 fake, true);
				rad_assert(rcode == 0);

				/*
				 *	Do NOT free the fake request!
				 */
				return RLM_MODULE_UPDATED;
			}

			/*
			 *	Didn't authenticate the packet, but
			 *	we're proxying it.
			 */
			rcode = RLM_MODULE_UPDATED;

		} else
#endif	/* WITH_PROXY */
		  {
			DEBUG2("  PEAP: Unknown RADIUS packet type %d: rejecting tunneled user", fake->reply->code);
			rcode = RLM_MODULE_REJECT;
		  }
		break;

	default:
	do_process:
		rcode = process_reply(handler, tls_session, request,
				      fake->reply);
		break;
	}

 done:
	request_free(&fake);

	return rcode;
}

static int setup_fake_request(REQUEST *request, REQUEST *fake, peap_tunnel_t *t) {

	VALUE_PAIR *vp;
	/*
	 *	Tell the request that it's a fake one.
	 */
	pairmake(fake->packet, &fake->packet->vps,
		 "Freeradius-Proxied-To", "127.0.0.1", T_OP_EQ);

	if (t->username) {
		vp = paircopy(fake->packet, t->username);
		pairadd(&fake->packet->vps, vp);
		fake->username = vp;
		RDEBUG2("Setting User-Name to %s", fake->username->vp_strvalue);
	} else {
		RDEBUG2("No tunnel username (SSL resumption?)");
	}


	/*
	 *	Add the State attribute, too, if it exists.
	 */
	if (t->state) {
		vp = paircopy(fake->packet, t->state);
		if (vp) pairadd(&fake->packet->vps, vp);
	}

	/*
	 *	If this is set, we copy SOME of the request attributes
	 *	from outside of the tunnel to inside of the tunnel.
	 *
	 *	We copy ONLY those attributes which do NOT already
	 *	exist in the tunneled request.
	 *
	 *	This code is copied from ../rlm_eap_ttls/ttls.c
	 */
	if (t->copy_request_to_tunnel) {
		VALUE_PAIR *copy;
		vp_cursor_t cursor;

		for (vp = paircursor(&cursor, &request->packet->vps);
		     vp;
		     vp = pairnext(&cursor)) {
			/*
			 *	The attribute is a server-side thingy,
			 *	don't copy it.
			 */
			if ((vp->da->attr > 255) && (((vp->da->attr >> 16) & 0xffff) == 0)) {
				continue;
			}

			/*
			 *	The outside attribute is already in the
			 *	tunnel, don't copy it.
			 *
			 *	This works for BOTH attributes which
			 *	are originally in the tunneled request,
			 *	AND attributes which are copied there
			 *	from below.
			 */
			if (pairfind(fake->packet->vps, vp->da->attr, vp->da->vendor, TAG_ANY)) {
				continue;
			}

			/*
			 *	Some attributes are handled specially.
			 */
			switch (vp->da->attr) {
				/*
				 *	NEVER copy Message-Authenticator,
				 *	EAP-Message, or State.  They're
				 *	only for outside of the tunnel.
				 */
			case PW_USER_NAME:
			case PW_USER_PASSWORD:
			case PW_CHAP_PASSWORD:
			case PW_CHAP_CHALLENGE:
			case PW_PROXY_STATE:
			case PW_MESSAGE_AUTHENTICATOR:
			case PW_EAP_MESSAGE:
			case PW_STATE:
				continue;
				break;

				/*
				 *	By default, copy it over.
				 */
			default:
				break;
			}

			/*
			 *	Don't copy from the head, we've already
			 *	checked it.
			 */
			copy = paircopy2(fake->packet, vp, vp->da->attr, vp->da->vendor, TAG_ANY);
			pairadd(&fake->packet->vps, copy);
		}
	}

	return 0;
}
