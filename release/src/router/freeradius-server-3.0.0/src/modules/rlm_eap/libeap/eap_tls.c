/*
 * eap_tls.c
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

/*
 *
 *  TLS Packet Format in EAP
 *  --- ------ ------ -- ---
 * 0		   1		   2		   3
 * 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |     Code      |   Identifier  |	    Length	     |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |     Type      |     Flags     |      TLS Message Length
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |     TLS Message Length	|       TLS Data...
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 */

RCSID("$Id$")
USES_APPLE_DEPRECATED_API	/* OpenSSL API has been deprecated by Apple */

#include <assert.h>

#include "eap_tls.h"

/*
 *	Send an initial eap-tls request to the peer.
 *
 *	Frame eap reply packet.
 *	len = header + type + tls_typedata
 *	tls_typedata = flags(Start (S) bit set, and no data)
 *
 *	Once having received the peer's Identity, the EAP server MUST
 *	respond with an EAP-TLS/Start packet, which is an
 *	EAP-Request packet with EAP-Type=EAP-TLS, the Start (S) bit
 *	set, and no data.  The EAP-TLS conversation will then begin,
 *	with the peer sending an EAP-Response packet with
 *	EAP-Type = EAP-TLS.  The data field of that packet will
 *	be the TLS data.
 *
 *	Fragment length is Framed-MTU - 4.
 */
tls_session_t *eaptls_session(fr_tls_server_conf_t *tls_conf, eap_handler_t *handler, int client_cert)
{
	tls_session_t	*ssn;
	int		verify_mode = 0;
	REQUEST		*request = handler->request;

	handler->tls = true;
	handler->finished = false;

	/*
	 *	Every new session is started only from EAP-TLS-START.
	 *	Before Sending EAP-TLS-START, open a new SSL session.
	 *	Create all the required data structures & store them
	 *	in Opaque.  So that we can use these data structures
	 *	when we get the response
	 */
	ssn = tls_new_session(tls_conf, request, client_cert);
	if (!ssn) {
		return NULL;
	}

	/*
	 *	Verify the peer certificate, if asked.
	 */
	if (client_cert) {
		RDEBUG2("Requiring client certificate");
		verify_mode = SSL_VERIFY_PEER;
		verify_mode |= SSL_VERIFY_FAIL_IF_NO_PEER_CERT;
		verify_mode |= SSL_VERIFY_CLIENT_ONCE;
	}
	SSL_set_verify(ssn->ssl, verify_mode, cbtls_verify);

	/*
	 *	Create a structure for all the items required to be
	 *	verified for each client and set that as opaque data
	 *	structure.
	 *
	 *	NOTE: If we want to set each item sepearately then
	 *	this index should be global.
	 */
	SSL_set_ex_data(ssn->ssl, FR_TLS_EX_INDEX_HANDLER, (void *)handler);
	SSL_set_ex_data(ssn->ssl, FR_TLS_EX_INDEX_CONF, (void *)tls_conf);
	SSL_set_ex_data(ssn->ssl, FR_TLS_EX_INDEX_CERTS, (void *)&(handler->certs));
	SSL_set_ex_data(ssn->ssl, FR_TLS_EX_INDEX_IDENTITY, (void *)&(handler->identity));
#ifdef HAVE_OPENSSL_OCSP_H
	SSL_set_ex_data(ssn->ssl, FR_TLS_EX_INDEX_STORE, (void *)tls_conf->ocsp_store);
#endif
	SSL_set_ex_data(ssn->ssl, FR_TLS_EX_INDEX_SSN, (void *)ssn);

	return talloc_steal(handler, ssn); /* ssn */
}

/*
   The S flag is set only within the EAP-TLS start message
   sent from the EAP server to the peer.
*/
int eaptls_start(EAP_DS *eap_ds, int peap_flag)
{
	EAPTLS_PACKET 	reply;

	reply.code = FR_TLS_START;
	reply.length = TLS_HEADER_LEN + 1/*flags*/;

	reply.flags = peap_flag;
	reply.flags = SET_START(reply.flags);

	reply.data = NULL;
	reply.dlen = 0;

	eaptls_compose(eap_ds, &reply);

	return 1;
}

int eaptls_success(eap_handler_t *handler, int peap_flag)
{
	EAPTLS_PACKET	reply;
	REQUEST *request = handler->request;
	tls_session_t *tls_session = handler->opaque;

	handler->finished = true;
	reply.code = FR_TLS_SUCCESS;
	reply.length = TLS_HEADER_LEN;
	reply.flags = peap_flag;
	reply.data = NULL;
	reply.dlen = 0;

	tls_success(tls_session, request);

	/*
	 *	Call compose AFTER checking for cached data.
	 */
	eaptls_compose(handler->eap_ds, &reply);

	/*
	 *	Automatically generate MPPE keying material.
	 */
	if (tls_session->prf_label) {
		eaptls_gen_mppe_keys(handler->request,
				     tls_session->ssl, tls_session->prf_label);
	} else {
		RWDEBUG("Not adding MPPE keys because there is no PRF label");
	}

	eaptls_gen_eap_key(handler->request->reply, tls_session->ssl,
			   handler->type);
	return 1;
}

int eaptls_fail(eap_handler_t *handler, int peap_flag)
{
	EAPTLS_PACKET	reply;
	tls_session_t *tls_session = handler->opaque;

	handler->finished = true;
	reply.code = FR_TLS_FAIL;
	reply.length = TLS_HEADER_LEN;
	reply.flags = peap_flag;
	reply.data = NULL;
	reply.dlen = 0;

	tls_fail(tls_session);

	eaptls_compose(handler->eap_ds, &reply);

	return 1;
}

/*
   A single TLS record may be up to 16384 octets in length, but a TLS
   message may span multiple TLS records, and a TLS certificate message
   may in principle be as long as 16MB.
*/

/*
 *	Frame the Dirty data that needs to be send to the client in an
 *	EAP-Request.  We always embed the TLS-length in all EAP-TLS
 *	packets that we send, for easy reference purpose.  Handle
 *	fragmentation and sending the next fragment etc.
 */
int eaptls_request(EAP_DS *eap_ds, tls_session_t *ssn)
{
	EAPTLS_PACKET	reply;
	unsigned int	size;
	unsigned int 	nlen;
	unsigned int 	lbit = 0;

	/* This value determines whether we set (L)ength flag for
		EVERY packet we send and add corresponding
		"TLS Message Length" field.

	length_flag = true;
		This means we include L flag and "TLS Msg Len" in EVERY
		packet we send out.

	length_flag = false;
		This means we include L flag and "TLS Msg Len" **ONLY**
		in First packet of a fragment series. We do not use
		it anywhere else.

		Having L flag in every packet is prefered.

	*/
	if (ssn->length_flag) {
		lbit = 4;
	}
	if (ssn->fragment == 0) {
		ssn->tls_msg_len = ssn->dirty_out.used;
	}

	reply.code = FR_TLS_REQUEST;
	reply.flags = ssn->peap_flag;

	/* Send data, NOT more than the FRAGMENT size */
	if (ssn->dirty_out.used > ssn->offset) {
		size = ssn->offset;
		reply.flags = SET_MORE_FRAGMENTS(reply.flags);
		/* Length MUST be included if it is the First Fragment */
		if (ssn->fragment == 0) {
			lbit = 4;
		}
		ssn->fragment = 1;
	} else {
		size = ssn->dirty_out.used;
		ssn->fragment = 0;
	}

	reply.dlen = lbit + size;
	reply.length = TLS_HEADER_LEN + 1/*flags*/ + reply.dlen;

	reply.data = talloc_array(eap_ds, uint8_t, reply.length);
	if (!reply.data) return 0;

	if (lbit) {
		nlen = htonl(ssn->tls_msg_len);
		memcpy(reply.data, &nlen, lbit);
		reply.flags = SET_LENGTH_INCLUDED(reply.flags);
	}
	(ssn->record_minus)(&ssn->dirty_out, reply.data + lbit, size);

	eaptls_compose(eap_ds, &reply);
	talloc_free(reply.data);
	reply.data = NULL;

	return 1;
}


/*
 *	Similarly, when the EAP server receives an EAP-Response with
 *	the M bit set, it MUST respond with an EAP-Request with
 *	EAP-Type=EAP-TLS and no data. This serves as a fragment ACK.
 *
 *	In order to prevent errors in the processing of fragments, the
 *	EAP server MUST use increment the Identifier value for each
 *	fragment ACK contained within an EAP-Request, and the peer
 *	MUST include this Identifier value in the subsequent fragment
 *	contained within an EAP- Reponse.
 *
 *	EAP server sends an ACK when it determines there are More
 *	fragments to receive to make the complete
 *	TLS-record/TLS-Message
 */
static int eaptls_send_ack(EAP_DS *eap_ds, int peap_flag)
{
	EAPTLS_PACKET 	reply;

	reply.code = FR_TLS_ACK;
	reply.length = TLS_HEADER_LEN + 1/*flags*/;
	reply.flags = peap_flag;
	reply.data = NULL;
	reply.dlen = 0;

	eaptls_compose(eap_ds, &reply);

	return 1;
}

/*
 *	The S flag is set only within the EAP-TLS start message sent
 *	from the EAP server to the peer.
 *
 *	Similarly, when the EAP server receives an EAP-Response with
 *	the M bit set, it MUST respond with an EAP-Request with
 *	EAP-Type=EAP-TLS and no data. This serves as a fragment
 *	ACK. The EAP peer MUST wait.
 */
static fr_tls_status_t eaptls_verify(eap_handler_t *handler)
{
	EAP_DS *eap_ds = handler->eap_ds;
	EAP_DS *prev_eap_ds = handler->prev_eapds;
	eaptls_packet_t	*eaptls_packet, *eaptls_prev = NULL;
	REQUEST *request = handler->request;

	/*
	 *	We don't check ANY of the input parameters.  It's all
	 *	code which works together, so if something is wrong,
	 *	we SHOULD core dump.
	 *
	 *	e.g. if eap_ds is NULL, of if eap_ds->response is
	 *	NULL, of if it's NOT an EAP-Response, or if the packet
	 *	is too short.  See eap_validation()., in ../../eap.c
	 *
	 *	Also, eap_method_select() takes care of selecting the
	 *	appropriate type, so we don't need to check
	 *	eap_ds->response->type.num == PW_EAP_TLS, or anything
	 *	else.
	 */
	eaptls_packet = (eaptls_packet_t *)eap_ds->response->type.data;
	if (prev_eap_ds && prev_eap_ds->response)
		eaptls_prev = (eaptls_packet_t *)prev_eap_ds->response->type.data;

	/*
	 *	check for ACK
	 *
	 *	If there's no TLS data, or there's 1 byte of TLS data,
	 *	with the flags set to zero, then it's an ACK.
	 *
	 *	Find if this is a reply to the previous request sent
	 */
	if ((!eaptls_packet) ||
	    ((eap_ds->response->length == EAP_HEADER_LEN + 2) &&
	     ((eaptls_packet->flags & 0xc0) == 0x00))) {

		if (prev_eap_ds &&
		    (prev_eap_ds->request->id == eap_ds->response->id)) {
			/*
			 *	Run the ACK handler directly from here.
			 */
			RDEBUG2("Received TLS ACK");
			return tls_ack_handler(handler->opaque, request);
		} else {
			RERROR("Received Invalid TLS ACK");
			return FR_TLS_INVALID;
		}
	}

	/*
	 *	We send TLS_START, but do not receive it.
	 */
	if (TLS_START(eaptls_packet->flags)) {
		RDEBUG("Received unexpected EAP-TLS Start message");
		return FR_TLS_INVALID;
	}

	/*
	 *	The L bit (length included) is set to indicate the
	 *	presence of the four octet TLS Message Length field,
	 *	and MUST be set for the first fragment of a fragmented
	 *	TLS message or set of messages.
	 *
	 *	The M bit (more fragments) is set on all but the last
	 *	fragment.
	 *
	 *	The S bit (EAP-TLS start) is set in an EAP-TLS Start
	 *	message. This differentiates the EAP-TLS Start message
	 *	from a fragment acknowledgement.
	 */
	if (TLS_LENGTH_INCLUDED(eaptls_packet->flags)) {
		DEBUG2("  TLS Length %d",
		       eaptls_packet->data[2] * 256 | eaptls_packet->data[3]);
		if (TLS_MORE_FRAGMENTS(eaptls_packet->flags)) {
			/*
			 * FIRST_FRAGMENT is identified
			 * 1. If there is no previous EAP-response received.
			 * 2. If EAP-response received, then its M bit not set.
			 * 	(It is because Last fragment will not have M bit set)
			 */
			if (!prev_eap_ds ||
			    (!prev_eap_ds->response) ||
			    (!eaptls_prev) ||
			    !TLS_MORE_FRAGMENTS(eaptls_prev->flags)) {

				RDEBUG2("Received EAP-TLS First Fragment of the message");
				return FR_TLS_FIRST_FRAGMENT;
			} else {

				RDEBUG2("More Fragments with length included");
				return FR_TLS_MORE_FRAGMENTS_WITH_LENGTH;
			}
		} else {
			RDEBUG2("Length Included");
			return FR_TLS_LENGTH_INCLUDED;
		}
	}

	if (TLS_MORE_FRAGMENTS(eaptls_packet->flags)) {
		RDEBUG2("More fragments to follow");
		return FR_TLS_MORE_FRAGMENTS;
	}

	/*
	 *	None of the flags are set, but it's still a valid
	 *	EAPTLS packet.
	 */
	return FR_TLS_OK;
}

/*
 * EAPTLS_PACKET
 * code   =  EAP-code
 * id     =  EAP-id
 * length = code + id + length + flags + tlsdata
 *	=  1   +  1 +   2    +  1    +  X
 * length = EAP-length - 1(EAP-Type = 1 octet)
 * flags  = EAP-typedata[0] (1 octet)
 * dlen   = EAP-typedata[1-4] (4 octets), if L flag set
 *	= length - 5(code+id+length+flags), otherwise
 * data   = EAP-typedata[5-n], if L flag set
 *	= EAP-typedata[1-n], otherwise
 * packet = EAP-typedata (complete typedata)
 *
 * Points to consider during EAP-TLS data extraction
 * 1. In the received packet, No data will be present incase of ACK-NAK
 * 2. Incase if more fragments need to be received then ACK after retreiving this fragment.
 *
 *  RFC 2716 Section 4.2.  PPP EAP TLS Request Packet
 *
 *  0		   1		   2		   3
 *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |     Code      |   Identifier  |	    Length	     |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |     Type      |     Flags     |      TLS Message Length
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |     TLS Message Length	|       TLS Data...
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 *  The Length field is two octets and indicates the length of the EAP
 *  packet including the Code, Identifir, Length, Type, and TLS data
 *  fields.
 */
static EAPTLS_PACKET *eaptls_extract(REQUEST *request, EAP_DS *eap_ds, fr_tls_status_t status)
{
	EAPTLS_PACKET	*tlspacket;
	uint32_t	data_len = 0;
	uint32_t	len = 0;
	uint8_t		*data = NULL;

	if (status  == FR_TLS_INVALID)
		return NULL;

	/*
	 *	The main EAP code & eaptls_verify() take care of
	 *	ensuring that the packet is OK, and that we can
	 *	extract the various fields we want.
	 *
	 *	e.g. a TLS packet with zero data is allowed as an ACK,
	 *	but we will never see it here, as we will simply
	 *	send another fragment, instead of trying to extract
	 *	the data.
	 *
	 *	MUST have TLS type octet, followed by flags, followed
	 *	by data.
	 */
	assert(eap_ds->response->length > 2);

	tlspacket = talloc(eap_ds, EAPTLS_PACKET);
	if (!tlspacket) return NULL;

	/*
	 *	Code & id for EAPTLS & EAP are same
	 *	but eaptls_length = eap_length - 1(EAP-Type = 1 octet)
	 *
	 *	length = code + id + length + type + tlsdata
	 *	       =  1   +  1 +   2    +  1    +  X
	 */
	tlspacket->code = eap_ds->response->code;
	tlspacket->id = eap_ds->response->id;
	tlspacket->length = eap_ds->response->length - 1; /* EAP type */
	tlspacket->flags = eap_ds->response->type.data[0];

	/*
	 *	A quick sanity check of the flags.  If we've been told
	 *	that there's a length, and there isn't one, then stop.
	 */
	if (TLS_LENGTH_INCLUDED(tlspacket->flags) &&
	    (tlspacket->length < 5)) { /* flags + TLS message length */
		RDEBUG("Invalid EAP-TLS packet received.  (Length bit is set, but no length was found.)");
		talloc_free(tlspacket);
		return NULL;
	}

	/*
	 *	If the final TLS packet is larger than we can handle, die
	 *	now.
	 *
	 *	Likewise, if the EAP packet says N bytes, and the TLS
	 *	packet says there's fewer bytes, it's a problem.
	 *
	 *	FIXME: Try to ensure that the claimed length is
	 *	consistent across multiple TLS fragments.
	 */
	if (TLS_LENGTH_INCLUDED(tlspacket->flags)) {
		memcpy(&data_len, &eap_ds->response->type.data[1], 4);
		data_len = ntohl(data_len);
		if (data_len > MAX_RECORD_SIZE) {
			RDEBUG("The EAP-TLS packet will contain more data than we can process.");
			talloc_free(tlspacket);
			return NULL;
		}

#if 0
		DEBUG2(" TLS: %d %d\n", data_len, tlspacket->length);

		if (data_len < tlspacket->length) {
			RDEBUG("EAP-TLS packet claims to be smaller than the encapsulating EAP packet.");
			talloc_free(tlspacket);
			return NULL;
		}
#endif
	}

	switch (status) {
	/*
	 *	The TLS Message Length field is four octets, and
	 *	provides the total length of the TLS message or set of
	 *	messages that is being fragmented; this simplifies
	 *	buffer allocation.
	 *
	 *	Dynamic allocation of buffers as & when we know the
	 *	length should solve the problem.
	 */
	case FR_TLS_FIRST_FRAGMENT:
	case FR_TLS_LENGTH_INCLUDED:
	case FR_TLS_MORE_FRAGMENTS_WITH_LENGTH:
		if (tlspacket->length < 5) { /* flags + TLS message length */
			RDEBUG("Invalid EAP-TLS packet received.  (Expected length, got none.)");
			talloc_free(tlspacket);
			return NULL;
		}

		/*
		 *	Extract all the TLS fragments from the
		 *	previous eap_ds Start appending this
		 *	fragment to the above ds
		 */
		memcpy(&data_len, &eap_ds->response->type.data[1], sizeof(uint32_t));
		data_len = ntohl(data_len);
		data = (eap_ds->response->type.data + 5/*flags+TLS-Length*/);
		len = eap_ds->response->type.length - 5/*flags+TLS-Length*/;

		/*
		 *	Hmm... this should be an error, too.
		 */
		if (data_len > len) {
			data_len = len;
		}
		break;

		/*
		 *	Data length is implicit, from the EAP header.
		 */
	case FR_TLS_MORE_FRAGMENTS:
	case FR_TLS_OK:
		data_len = eap_ds->response->type.length - 1/*flags*/;
		data = eap_ds->response->type.data + 1/*flags*/;
		break;

	default:
		RDEBUG("Invalid EAP-TLS packet received");
		talloc_free(tlspacket);
		return NULL;
	}

	tlspacket->dlen = data_len;
	if (data_len) {
		tlspacket->data = talloc_array(tlspacket, uint8_t,
					       data_len);
		if (!tlspacket->data) {
			talloc_free(tlspacket);
			return NULL;
		}
		memcpy(tlspacket->data, data, data_len);
	}

	return tlspacket;
}



/*
 * To process the TLS,
 *  INCOMING DATA:
 * 	1. EAP-TLS should get the compelete TLS data from the peer.
 * 	2. Store that data in a data structure with any other required info
 *	3. Handle that data structure to the TLS module.
 *	4. TLS module will perform its operations on the data and
 *	handle back to EAP-TLS
 *
 *  OUTGOING DATA:
 * 	1. EAP-TLS if necessary will fragment it and send it to the
 * 	destination.
 *
 *	During EAP-TLS initialization, TLS Context object will be
 *	initialized and stored.  For every new authentication
 *	requests, TLS will open a new session object and that session
 *	object should be maintained even after the session is
 *	completed for session resumption. (Probably later as a feature
 *	as we donot know who maintains these session objects ie,
 *	SSL_CTX (internally) or TLS module(explicitly). If TLS module,
 *	then how to let SSL API know about these sessions.)
 */
static fr_tls_status_t eaptls_operation(fr_tls_status_t status,
					eap_handler_t *handler)
{
	tls_session_t *tls_session;

	tls_session = (tls_session_t *)handler->opaque;

	if ((status == FR_TLS_MORE_FRAGMENTS) ||
	    (status == FR_TLS_MORE_FRAGMENTS_WITH_LENGTH) ||
	    (status == FR_TLS_FIRST_FRAGMENT)) {
		/*
		 *	Send the ACK.
		 */
		eaptls_send_ack(handler->eap_ds, tls_session->peap_flag);
		return FR_TLS_HANDLED;

	}

	/*
	 *	We have the complete TLS-data or TLS-message.
	 *
	 *	Clean the dirty message.
	 *
	 *	Authenticate the user and send
	 *	Success/Failure.
	 *
	 *	If more info
	 *	is required then send another request.
	 */
	if (!tls_handshake_recv(handler->request, tls_session)) {
		DEBUG2("TLS receive handshake failed during operation");
		eaptls_fail(handler, tls_session->peap_flag);
		return FR_TLS_FAIL;
	}

	/*
	 *	FIXME: return success/fail.
	 *
	 *	TLS proper can decide what to do, then.
	 */
	if (tls_session->dirty_out.used > 0) {
		eaptls_request(handler->eap_ds, tls_session);
		return FR_TLS_HANDLED;
	}

	/*
	 *	If there is no data to send i.e
	 *	dirty_out.used <=0 and if the SSL
	 *	handshake is finished, then return a
	 *	EPTLS_SUCCESS
	 */

	if (SSL_is_init_finished(tls_session->ssl)) {
		/*
		 *	Init is finished.  The rest is
		 *	application data.
		 */
		tls_session->info.content_type = application_data;
		return FR_TLS_SUCCESS;
	}

	/*
	 *	Who knows what happened...
	 */
	DEBUG2("TLS failed during operation");
	return FR_TLS_FAIL;
}


/*
 * In the actual authentication first verify the packet and then create the data structure
 */
/*
 * To process the TLS,
 *  INCOMING DATA:
 * 	1. EAP-TLS should get the compelete TLS data from the peer.
 * 	2. Store that data in a data structure with any other required info
 *	3. Hand this data structure to the TLS module.
 *	4. TLS module will perform its operations on the data and hands back to EAP-TLS
 *  OUTGOING DATA:
 * 	1. EAP-TLS if necessary will fragment it and send it to the destination.
 *
 *	During EAP-TLS initialization, TLS Context object will be
 *	initialized and stored.  For every new authentication
 *	requests, TLS will open a new session object and that
 *	session object SHOULD be maintained even after the session
 *	is completed, for session resumption. (Probably later as a
 *	feature, as we do not know who maintains these session
 *	objects ie, SSL_CTX (internally) or TLS module (explicitly). If
 *	TLS module, then how to let SSL API know about these
 *	sessions.)
 */

/*
 *	Process an EAP request
 */
fr_tls_status_t eaptls_process(eap_handler_t *handler)
{
	tls_session_t *tls_session = (tls_session_t *) handler->opaque;
	EAPTLS_PACKET	*tlspacket;
	fr_tls_status_t	status;
	REQUEST *request = handler->request;

	if (!request) return FR_TLS_FAIL;

	RDEBUG2("processing EAP-TLS");
	SSL_set_ex_data(tls_session->ssl, FR_TLS_EX_INDEX_REQUEST, request);

	if (handler->certs) pairadd(&request->packet->vps,
				    paircopy(request->packet, handler->certs));

	/* This case is when SSL generates Alert then we
	 * send that alert to the client and then send the EAP-Failure
	 */
	status = eaptls_verify(handler);
	RDEBUG2("eaptls_verify returned %d\n", status);

	switch (status) {
	default:
	case FR_TLS_INVALID:
	case FR_TLS_FAIL:

		/*
		 *	Success means that we're done the initial
		 *	handshake.  For TTLS, this means send stuff
		 *	back to the client, and the client sends us
		 *	more tunneled data.
		 */
	case FR_TLS_SUCCESS:
		goto done;

		/*
		 *	Normal TLS request, continue with the "get rest
		 *	of fragments" phase.
		 */
	case FR_TLS_REQUEST:
		eaptls_request(handler->eap_ds, tls_session);
		status = FR_TLS_HANDLED;
		goto done;

		/*
		 *	The handshake is done, and we're in the "tunnel
		 *	data" phase.
		 */
	case FR_TLS_OK:
		RDEBUG2("Done initial handshake");

		/*
		 *	Get the rest of the fragments.
		 */
	case FR_TLS_FIRST_FRAGMENT:
	case FR_TLS_MORE_FRAGMENTS:
	case FR_TLS_LENGTH_INCLUDED:
	case FR_TLS_MORE_FRAGMENTS_WITH_LENGTH:
		break;
	}

	/*
	 *	Extract the TLS packet from the buffer.
	 */
	if ((tlspacket = eaptls_extract(request, handler->eap_ds, status)) == NULL) {
		status = FR_TLS_FAIL;
		goto done;
	}

	/*
	 *	Get the session struct from the handler
	 *
	 *	update the dirty_in buffer
	 *
	 *	NOTE: This buffer will contain partial data when M bit is set.
	 *
	 * 	CAUTION while reinitializing this buffer, it should be
	 * 	reinitialized only when this M bit is NOT set.
	 */
	if (tlspacket->dlen !=
	    (tls_session->record_plus)(&tls_session->dirty_in, tlspacket->data, tlspacket->dlen)) {
		talloc_free(tlspacket);
		RDEBUG("Exceeded maximum record size");
		status =FR_TLS_FAIL;
		goto done;
	}

	/*
	 *	No longer needed.
	 */
	talloc_free(tlspacket);

	/*
	 *	SSL initalization is done.  Return.
	 *
	 *	The TLS data will be in the tls_session structure.
	 */
	if (SSL_is_init_finished(tls_session->ssl)) {
		/*
		 *	The initialization may be finished, but if
		 *	there more fragments coming, then send ACK,
		 *	and get the caller to continue the
		 *	conversation.
		 */
		if ((status == FR_TLS_MORE_FRAGMENTS) ||
		    (status == FR_TLS_MORE_FRAGMENTS_WITH_LENGTH) ||
	    	    (status == FR_TLS_FIRST_FRAGMENT)) {
			/*
			 *	Send the ACK.
			 */
			eaptls_send_ack(handler->eap_ds,
					tls_session->peap_flag);
			RDEBUG2("Init is done, but tunneled data is fragmented");
			status = FR_TLS_HANDLED;
			goto done;
		}

		status = tls_application_data(tls_session, request);
		goto done;
	}

	/*
	 *	Continue the handshake.
	 */
	status = eaptls_operation(status, handler);

 done:
	SSL_set_ex_data(tls_session->ssl, FR_TLS_EX_INDEX_REQUEST, NULL);

	return status;
}


/*
 *	compose the TLS reply packet in the EAP reply typedata
 */
int eaptls_compose(EAP_DS *eap_ds, EAPTLS_PACKET *reply)
{
	uint8_t *ptr;

	/*
	 *	Don't set eap_ds->request->type.num, as the main EAP
	 *	handler will do that for us.  This allows the TLS
	 *	module to be called from TTLS & PEAP.
	 */

	/*
	 * 	When the EAP server receives an EAP-Response with the
	 * 	M bit set, it MUST respond with an EAP-Request with
	 * 	EAP-Type=EAP-TLS and no data. This serves as a
	 * 	fragment ACK. The EAP peer MUST wait until it receives
	 * 	the EAP-Request before sending another fragment.
	 *
	 *	In order to prevent errors in the processing of
	 *	fragments, the EAP server MUST use increment the
	 *	Identifier value for each fragment ACK contained
	 *	within an EAP-Request, and the peer MUST include this
	 *	Identifier value in the subsequent fragment contained
	 *	within an EAP- Reponse.
	 */
	eap_ds->request->type.data = talloc_array(eap_ds->request, uint8_t,
						  reply->length - TLS_HEADER_LEN + 1);
	if (!eap_ds->request->type.data) {
		return 0;
	}

	/* EAPTLS Header length is excluded while computing EAP typelen */
	eap_ds->request->type.length = reply->length - TLS_HEADER_LEN;

	ptr = eap_ds->request->type.data;
	*ptr++ = (uint8_t)(reply->flags & 0xFF);

	if (reply->dlen) memcpy(ptr, reply->data, reply->dlen);

	switch (reply->code) {
	case FR_TLS_ACK:
	case FR_TLS_START:
	case FR_TLS_REQUEST:
		eap_ds->request->code = PW_EAP_REQUEST;
		break;
	case FR_TLS_SUCCESS:
		eap_ds->request->code = PW_EAP_SUCCESS;
		break;
	case FR_TLS_FAIL:
		eap_ds->request->code = PW_EAP_FAILURE;
		break;
	default:
		/* Should never enter here */
		eap_ds->request->code = PW_EAP_FAILURE;
		break;
	}

	return 1;
}

/*
 *	Parse TLS configuration
 *
 *	If the option given by 'attr' is set, we find the config section
 *	of that name and use that for the TLS configuration. If not, we
 *	fall back to compatibility mode and read the TLS options from
 *	the 'tls' section.
 */
fr_tls_server_conf_t *eaptls_conf_parse(CONF_SECTION *cs, char const *attr)
{
	char const 		*tls_conf_name;
	CONF_PAIR		*cp;
	CONF_SECTION		*parent;
	CONF_SECTION		*tls_cs;
	fr_tls_server_conf_t	*tls_conf;

	if (!cs)
		return NULL;

	rad_assert(attr != NULL);

	parent = cf_item_parent(cf_sectiontoitem(cs));

	cp = cf_pair_find(cs, attr);
	if (cp) {
		tls_conf_name = cf_pair_value(cp);

		tls_cs = cf_section_sub_find_name2(parent, TLS_CONFIG_SECTION, tls_conf_name);

		if (!tls_cs) {
			ERROR("Cannot find tls config '%s'", tls_conf_name);
			return NULL;
		}
	} else {
		/*
		 *	If we can't find the section given by the 'attr', we
		 *	fall-back to looking for the "tls" section, as in
		 *	previous versions.
		 *
		 *	We don't fall back if the 'attr' is specified, but we can't
		 *	find the section - that is just a config error.
		 */
		INFO("debug: '%s' option missing, trying to use legacy configuration", attr);
		tls_cs = cf_section_sub_find(parent, "tls");
	}

	if (!tls_cs)
		return NULL;

	tls_conf = tls_server_conf_parse(tls_cs);

	if (!tls_conf)
		return NULL;

	/*
	 *	The EAP RFC's say 1020, but we're less picky.
	 */
	if (tls_conf->fragment_size < 100) {
		ERROR("Fragment size is too small.");
		return NULL;
	}

	/*
	 *	The maximum size for a RADIUS packet is 4096,
	 *	minus the header (20), Message-Authenticator (18),
	 *	and State (18), etc. results in about 4000 bytes of data
	 *	that can be devoted *solely* to EAP.
	 */
	if (tls_conf->fragment_size > 4000) {
		ERROR("Fragment size is too large.");
		return NULL;
	}

	/*
	 *	Account for the EAP header (4), and the EAP-TLS header
	 *	(6), as per Section 4.2 of RFC 2716.  What's left is
	 *	the maximum amount of data we read from a TLS buffer.
	 */
	tls_conf->fragment_size -= 10;

	return tls_conf;
}

