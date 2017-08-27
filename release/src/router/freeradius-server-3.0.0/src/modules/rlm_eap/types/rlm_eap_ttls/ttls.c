/*
 * rlm_eap_ttls.c  contains the interfaces that are called from eap
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

#include "eap_ttls.h"

/*
 *    0		   1		   2		   3
 *    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   |			   AVP Code			    |
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   |V M r r r r r r|		  AVP Length		   |
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   |			Vendor-ID (opt)			|
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   |    Data ...
 *   +-+-+-+-+-+-+-+-+
 */

/*
 *	Verify that the diameter packet is valid.
 */
static int diameter_verify(REQUEST *request, uint8_t const *data, unsigned int data_len)
{
	uint32_t attr;
	uint32_t length;
	unsigned int hdr_len;
	unsigned int remaining = data_len;

	while (remaining > 0) {
		hdr_len = 12;

		if (remaining < hdr_len) {
		  RDEBUG2(" Diameter attribute is too small (%u) to contain a Diameter header", remaining);
			return 0;
		}

		memcpy(&attr, data, sizeof(attr));
		attr = ntohl(attr);
		memcpy(&length, data + 4, sizeof(length));
		length = ntohl(length);

		if ((data[4] & 0x80) != 0) {
			if (remaining < 16) {
				RDEBUG2(" Diameter attribute is too small to contain a Diameter header with Vendor-Id");
				return 0;
			}

			hdr_len = 16;
		}

		/*
		 *	Get the length.  If it's too big, die.
		 */
		length &= 0x00ffffff;

		/*
		 *	Too short or too long is bad.
		 */
		if (length <= (hdr_len - 4)) {
			RDEBUG2("Tunneled attribute %u is too short (%u < %u) to contain anything useful.", attr,
				length, hdr_len);
			return 0;
		}

		if (length > remaining) {
			RDEBUG2("Tunneled attribute %u is longer than room remaining in the packet (%u > %u).", attr,
				length, remaining);
			return 0;
		}

		/*
		 *	Check for broken implementations, which don't
		 *	pad the AVP to a 4-octet boundary.
		 */
		if (remaining == length) break;

		/*
		 *	The length does NOT include the padding, so
		 *	we've got to account for it here by rounding up
		 *	to the nearest 4-byte boundary.
		 */
		length += 0x03;
		length &= ~0x03;

		/*
		 *	If the rest of the diameter packet is larger than
		 *	this attribute, continue.
		 *
		 *	Otherwise, if the attribute over-flows the end
		 *	of the packet, die.
		 */
		if (remaining < length) {
			REDEBUG2("Diameter attribute overflows packet!");
			return 0;
		}

		/*
		 *	remaining > length, continue.
		 */
		remaining -= length;
		data += length;
	}

	/*
	 *	We got this far.  It looks OK.
	 */
	return 1;
}


/*
 *	Convert diameter attributes to our VALUE_PAIR's
 */
static VALUE_PAIR *diameter2vp(REQUEST *request, REQUEST *fake, SSL *ssl,
			       uint8_t const *data, size_t data_len)
{
	uint32_t	attr;
	uint32_t	vendor;
	uint32_t	length;
	size_t		offset;
	size_t		size;
	size_t		data_left = data_len;
	char		*p;
	VALUE_PAIR	*first = NULL;
	VALUE_PAIR	*vp;
	RADIUS_PACKET	*packet = fake->packet; /* FIXME: api issues */
	vp_cursor_t	out;

	paircursor(&out, &first);

	while (data_left > 0) {
		rad_assert(data_left <= data_len);
		memcpy(&attr, data, sizeof(attr));
		data += 4;
		attr = ntohl(attr);
		vendor = 0;

		memcpy(&length, data, sizeof(length));
		data += 4;
		length = ntohl(length);

		/*
		 *	A "vendor" flag, with a vendor ID of zero,
		 *	is equivalent to no vendor.  This is stupid.
		 */
		offset = 8;
		if ((length & (1 << 31)) != 0) {
			memcpy(&vendor, data, sizeof(vendor));
			vendor = ntohl(vendor);

			data += 4; /* skip the vendor field, it's zero */
			offset += 4; /* offset to value field */

			if (attr > 65535) goto next_attr;
			if (vendor > FR_MAX_VENDOR) goto next_attr;
		}

		/*
		 *	FIXME: Handle the M bit.  For now, we assume that
		 *	some other module takes care of any attribute
		 *	with the M bit set.
		 */

		/*
		 *	Get the length.
		 */
		length &= 0x00ffffff;

		/*
		 *	Get the size of the value portion of the
		 *	attribute.
		 */
		size = length - offset;

		/*
		 *	Vendor attributes can be larger than 255.
		 *	Normal attributes cannot be.
		 */
		if ((attr > 255) && (vendor == 0)) {
			RWDEBUG2("Skipping Diameter attribute %u", attr);
			goto next_attr;
		}

		/*
		 *	EAP-Message AVPs can be larger than 253 octets.
		 *
		 *	For now, we rely on the main decoder in
		 *	src/lib/radius to decode data into VPs.  This
		 *	means putting the data into a RADIUS attribute
		 *	format.  It also means that we can't handle
		 *	"extended" attributes in the Diameter space.  Oh well...
		 */
		if ((size > 253) && !((vendor == 0) && (attr == PW_EAP_MESSAGE))) {
			RWDEBUG2("diameter2vp skipping long attribute %u", attr);
			goto next_attr;
		}

		/*
		 *	RADIUS VSAs are handled as Diameter attributes
		 *	with Vendor-Id == 0, and the VSA data packed
		 *	into the "String" field as per normal.
		 *
		 *	EXCEPT for the MS-CHAP attributes.
		 */
		if ((vendor == 0) && (attr == PW_VENDOR_SPECIFIC)) {
			ssize_t decoded;
			uint8_t buffer[256];

			buffer[0] = PW_VENDOR_SPECIFIC;
			buffer[1] = size + 2;
			memcpy(buffer + 2, data, size);

			vp = NULL;
			decoded = rad_attr2vp(NULL, NULL, NULL,
					      buffer, size + 2, &vp);
			if (decoded < 0) {
				REDEBUG2("diameter2vp failed decoding attr: %s",
					fr_strerror());
				goto do_octets;
			}

			if ((size_t) decoded != size + 2) {
				REDEBUG2("diameter2vp failed to entirely decode VSA");
				pairfree(&vp);
				goto do_octets;
			}

			pairinsert(&out, vp);

			goto next_attr;
		}

		/*
		 *	Create it.  If this fails, it's because we're OOM.
		 */
	do_octets:
		vp = paircreate(packet, attr, vendor);
		if (!vp) {
			RDEBUG2("Failure in creating VP");
			pairfree(&first);
			return NULL;
		}

		/*
		 *	If it's a type from our dictionary, then
		 *	we need to put the data in a relevant place.
		 *
		 *	@todo: Export the lib/radius.c decoder, and use it here!
		 */
		switch (vp->da->type) {
		case PW_TYPE_INTEGER:
		case PW_TYPE_DATE:
			if (size != vp->length) {
				DICT_ATTR const *da;

				/*
				 *	Bad format.  Create a "raw"
				 *	attribute.
				 */
		raw:
				if (vp) pairfree(&vp);
				da = dict_attrunknown(attr, vendor, true);
				if (!da) return NULL;
				vp = pairalloc(NULL, da);
				pairmemcpy(vp, data, size);
				break;
			}
			memcpy(&vp->vp_integer, data, vp->length);

			/*
			 *	Stored in host byte order: change it.
			 */
			vp->vp_integer = ntohl(vp->vp_integer);
			break;

		case PW_TYPE_INTEGER64:
			if (size != vp->length) goto raw;
			memcpy(&vp->vp_integer64, data, vp->length);

			/*
			 *	Stored in host byte order: change it.
			 */
			vp->vp_integer64 = ntohll(vp->vp_integer64);
			break;

		case PW_TYPE_IPADDR:
			if (size != vp->length) {
				RDEBUG2("Invalid length attribute %d",
				       attr);
				pairfree(&first);
				pairfree(&vp);
				return NULL;
			}
			memcpy(&vp->vp_ipaddr, data, vp->length);

			/*
			 *	Stored in network byte order: don't change it.
			 */
			break;

		case PW_TYPE_BYTE:
			if (size != vp->length) goto raw;
			vp->vp_integer = data[0];
			break;

		case PW_TYPE_SHORT:
			if (size != vp->length) goto raw;
			vp->vp_integer = (data[0] * 256) + data[1];
			break;

		case PW_TYPE_SIGNED:
			if (size != vp->length) goto raw;
			memcpy(&vp->vp_signed, data, vp->length);
			vp->vp_signed = ntohl(vp->vp_signed);
			break;

		case PW_TYPE_IPV6ADDR:
			if (size != vp->length) goto raw;
			memcpy(&vp->vp_ipv6addr, data, vp->length);
			break;

		case PW_TYPE_IPV6PREFIX:
			if (size != vp->length) goto raw;
			memcpy(&vp->vp_ipv6prefix, data, vp->length);
			break;

			/*
			 *	Ensure it's NUL terminated.
			 */
		case PW_TYPE_STRING:
			vp->vp_strvalue = p = talloc_array(vp, char, size + 1);
			vp->type = VT_DATA;
			memcpy(p, data, size);
			p[size] = '\0';
			vp->length = strlen(p);
			break;

			/*
			 *	Copy it over verbatim.
			 */
		case PW_TYPE_OCTETS:
		default:
			pairmemcpy(vp, data, size);
			break;
		}

		/*
		 *	Ensure that the client is using the
		 *	correct challenge.  This weirdness is
		 *	to protect against against replay
		 *	attacks, where anyone observing the
		 *	CHAP exchange could pose as that user,
		 *	by simply choosing to use the same
		 *	challenge.
		 *
		 *	By using a challenge based on
		 *	information from the current session,
		 *	we can guarantee that the client is
		 *	not *choosing* a challenge.
		 *
		 *	We're a little forgiving in that we
		 *	have loose checks on the length, and
		 *	we do NOT check the Id (first octet of
		 *	the response to the challenge)
		 *
		 *	But if the client gets the challenge correct,
		 *	we're not too worried about the Id.
		 */
		if (((vp->da->vendor == 0) && (vp->da->attr == PW_CHAP_CHALLENGE)) ||
		    ((vp->da->vendor == VENDORPEC_MICROSOFT) && (vp->da->attr == PW_MSCHAP_CHALLENGE))) {
			uint8_t	challenge[16];

			if ((vp->length < 8) ||
			    (vp->length > 16)) {
				RDEBUG("Tunneled challenge has invalid length");
				pairfree(&first);
				pairfree(&vp);
				return NULL;
			}

			eapttls_gen_challenge(ssl, challenge,
					      sizeof(challenge));

			if (memcmp(challenge, vp->vp_octets,
				   vp->length) != 0) {
				RDEBUG("Tunneled challenge is incorrect");
				pairfree(&first);
				pairfree(&vp);
				return NULL;
			}
		}

		/*
		 *	Update the list.
		 */
		pairinsert(&out, vp);

	next_attr:
		/*
		 *	Catch non-aligned attributes.
		 */
		if (data_left == length) break;

		/*
		 *	The length does NOT include the padding, so
		 *	we've got to account for it here by rounding up
		 *	to the nearest 4-byte boundary.
		 */
		length += 0x03;
		length &= ~0x03;

		rad_assert(data_left >= length);
		data_left -= length;
		data += length - offset; /* already updated */
	}

	/*
	 *	We got this far.  It looks OK.
	 */
	return first;
}

/*
 *	Convert VALUE_PAIR's to diameter attributes, and write them
 *	to an SSL session.
 *
 *	The ONLY VALUE_PAIR's which may be passed to this function
 *	are ones which can go inside of a RADIUS (i.e. diameter)
 *	packet.  So no server-configuration attributes, or the like.
 */
static int vp2diameter(REQUEST *request, tls_session_t *tls_session, VALUE_PAIR *first)
{
	/*
	 *	RADIUS packets are no more than 4k in size, so if
	 *	we've got more than 4k of data to write, it's very
	 *	bad.
	 */
	uint8_t		buffer[4096];
	uint8_t		*p;
	uint32_t	attr;
	uint32_t	length;
	uint32_t	vendor;
	size_t		total;
	uint64_t	attr64;
	VALUE_PAIR	*vp;
	vp_cursor_t	cursor;

	p = buffer;
	total = 0;

	for (vp = paircursor(&cursor, &first); vp; vp = pairnext(&cursor)) {
		/*
		 *	Too much data: die.
		 */
		if ((total + vp->length + 12) >= sizeof(buffer)) {
			RDEBUG2("output buffer is full!");
			return 0;
		}

		/*
		 *	Hmm... we don't group multiple EAP-Messages
		 *	together.  Maybe we should...
		 */

		length = vp->length;
		vendor = vp->da->vendor;
		if (vendor != 0) {
			attr = vp->da->attr & 0xffff;
			length |= (1 << 31);
		} else {
			attr = vp->da->attr;
		}

		/*
		 *	Hmm... set the M bit for all attributes?
		 */
		length |= (1 << 30);

		attr = ntohl(attr);

		memcpy(p, &attr, sizeof(attr));
		p += 4;
		total += 4;

		length += 8;	/* includes 8 bytes of attr & length */

		if (vendor != 0) {
			length += 4; /* include 4 bytes of vendor */

			length = ntohl(length);
			memcpy(p, &length, sizeof(length));
			p += 4;
			total += 4;

			vendor = ntohl(vendor);
			memcpy(p, &vendor, sizeof(vendor));
			p += 4;
			total += 4;
		} else {
			length = ntohl(length);
			memcpy(p, &length, sizeof(length));
			p += 4;
			total += 4;
		}

		switch (vp->da->type) {
		case PW_TYPE_INTEGER:
		case PW_TYPE_DATE:
			attr = htonl(vp->vp_integer); /* stored in host order */
			memcpy(p, &attr, sizeof(attr));
			length = 4;
			break;

		case PW_TYPE_INTEGER64:
			attr64 = htonll(vp->vp_integer64); /* stored in host order */
			memcpy(p, &attr64, sizeof(attr64));
			length = 8;
			break;

		case PW_TYPE_IPADDR:
			memcpy(p, &vp->vp_ipaddr, 4); /* network order */
			length = 4;
			break;

		case PW_TYPE_STRING:
		case PW_TYPE_OCTETS:
		default:
			memcpy(p, vp->vp_strvalue, vp->length);
			length = vp->length;
			break;
		}

		/*
		 *	Skip to the end of the data.
		 */
		p += length;
		total += length;

		/*
		 *	Align the data to a multiple of 4 bytes.
		 */
		if ((total & 0x03) != 0) {
			size_t i;

			length = 4 - (total & 0x03);
			for (i = 0; i < length; i++) {
				*p = '\0';
				p++;
				total++;
			}
		}
	} /* loop over the VP's to write. */

	/*
	 *	Write the data in the buffer to the SSL session.
	 */
	if (total > 0) {
#ifndef NDEBUG
		size_t i;

		if ((debug_flag > 2) && fr_log_fp) {
			for (i = 0; i < total; i++) {
				if ((i & 0x0f) == 0) fprintf(fr_log_fp, "  TTLS tunnel data out %04x: ", (int) i);

				fprintf(fr_log_fp, "%02x ", buffer[i]);

				if ((i & 0x0f) == 0x0f) fprintf(fr_log_fp, "\n");
			}
			if ((total & 0x0f) != 0) fprintf(fr_log_fp, "\n");
		}
#endif

		(tls_session->record_plus)(&tls_session->clean_in, buffer, total);

		/*
		 *	FIXME: Check the return code.
		 */
		tls_handshake_send(request, tls_session);
	}

	/*
	 *	Everything's OK.
	 */
	return 1;
}

/*
 *	Use a reply packet to determine what to do.
 */
static rlm_rcode_t process_reply(UNUSED eap_handler_t *handler, tls_session_t *tls_session,
				 REQUEST *request, RADIUS_PACKET *reply)
{
	rlm_rcode_t rcode = RLM_MODULE_REJECT;
	VALUE_PAIR *vp;
	ttls_tunnel_t *t = tls_session->opaque;

	rad_assert(request != NULL);
	rad_assert(handler->request == request);

	/*
	 *	If the response packet was Access-Accept, then
	 *	we're OK.  If not, die horribly.
	 *
	 *	FIXME: Take MS-CHAP2-Success attribute, and
	 *	tunnel it back to the client, to authenticate
	 *	ourselves to the client.
	 *
	 *	FIXME: If we have an Access-Challenge, then
	 *	the Reply-Message is tunneled back to the client.
	 *
	 *	FIXME: If we have an EAP-Message, then that message
	 *	must be tunneled back to the client.
	 *
	 *	FIXME: If we have an Access-Challenge with a State
	 *	attribute, then do we tunnel that to the client, or
	 *	keep track of it ourselves?
	 *
	 *	FIXME: EAP-Messages can only start with 'identity',
	 *	NOT 'eap start', so we should check for that....
	 */
	switch (reply->code) {
	case PW_AUTHENTICATION_ACK:
		RDEBUG("Got tunneled Access-Accept");

		rcode = RLM_MODULE_OK;

		/*
		 *	MS-CHAP2-Success means that we do NOT return
		 *	an Access-Accept, but instead tunnel that
		 *	attribute to the client, and keep going with
		 *	the TTLS session.  Once the client accepts
		 *	our identity, it will respond with an empty
		 *	packet, and we will send EAP-Success.
		 */
		vp = NULL;
		pairfilter(tls_session, &vp, &reply->vps, PW_MSCHAP2_SUCCESS, VENDORPEC_MICROSOFT, TAG_ANY);
		if (vp) {
			RDEBUG("Got MS-CHAP2-Success, tunneling it to the client in a challenge.");
			rcode = RLM_MODULE_HANDLED;
			t->authenticated = true;

			/*
			 *	Delete MPPE keys & encryption policy.  We don't
			 *	want these here.
			 */
			pairdelete(&reply->vps, 7, VENDORPEC_MICROSOFT, TAG_ANY);
			pairdelete(&reply->vps, 8, VENDORPEC_MICROSOFT, TAG_ANY);
			pairdelete(&reply->vps, 16, VENDORPEC_MICROSOFT, TAG_ANY);
			pairdelete(&reply->vps, 17, VENDORPEC_MICROSOFT, TAG_ANY);

			/*
			 *	Use the tunneled reply, but not now.
			 */
			if (t->use_tunneled_reply) {
				rad_assert(!t->accept_vps);
				pairfilter(t, &t->accept_vps, &reply->vps,
					  0, 0, TAG_ANY);
				rad_assert(!reply->vps);
			}

		} else { /* no MS-CHAP2-Success */
			/*
			 *	Can only have EAP-Message if there's
			 *	no MS-CHAP2-Success.  (FIXME: EAP-MSCHAP?)
			 *
			 *	We also do NOT tunnel the EAP-Success
			 *	attribute back to the client, as the client
			 *	can figure it out, from the non-tunneled
			 *	EAP-Success packet.
			 */
			pairfilter(tls_session, &vp, &reply->vps, PW_EAP_MESSAGE, 0, TAG_ANY);
			pairfree(&vp);
		}

		/*
		 *	Handle the ACK, by tunneling any necessary reply
		 *	VP's back to the client.
		 */
		if (vp) {
			vp2diameter(request, tls_session, vp);
			pairfree(&vp);
		}

		/*
		 *	If we've been told to use the attributes from
		 *	the reply, then do so.
		 *
		 *	WARNING: This may leak information about the
		 *	tunneled user!
		 */
		if (t->use_tunneled_reply) {
			pairdelete(&reply->vps, PW_PROXY_STATE, 0, TAG_ANY);
			pairfilter(request->reply, &request->reply->vps,
				  &reply->vps, 0, 0, TAG_ANY);
		}
		break;


	case PW_AUTHENTICATION_REJECT:
		RDEBUG("Got tunneled Access-Reject");
		rcode = RLM_MODULE_REJECT;
		break;

		/*
		 *	Handle Access-Challenge, but only if we
		 *	send tunneled reply data.  This is because
		 *	an Access-Challenge means that we MUST tunnel
		 *	a Reply-Message to the client.
		 */
	case PW_ACCESS_CHALLENGE:
		RDEBUG("Got tunneled Access-Challenge");

		/*
		 *	Keep the State attribute, if necessary.
		 *
		 *	Get rid of the old State, too.
		 */
		pairfree(&t->state);
		pairfilter(t, &t->state, &reply->vps, PW_STATE, 0, TAG_ANY);

		/*
		 *	We should really be a bit smarter about this,
		 *	and move over only those attributes which
		 *	are relevant to the authentication request,
		 *	but that's a lot more work, and this "dumb"
		 *	method works in 99.9% of the situations.
		 */
		vp = NULL;
		pairfilter(t, &vp, &reply->vps, PW_EAP_MESSAGE, 0, TAG_ANY);

		/*
		 *	There MUST be a Reply-Message in the challenge,
		 *	which we tunnel back to the client.
		 *
		 *	If there isn't one in the reply VP's, then
		 *	we MUST create one, with an empty string as
		 *	it's value.
		 */
		pairfilter(t, &vp, &reply->vps, PW_REPLY_MESSAGE, 0, TAG_ANY);

		/*
		 *	Handle the ACK, by tunneling any necessary reply
		 *	VP's back to the client.
		 */
		if (vp) {
			vp2diameter(request, tls_session, vp);
			pairfree(&vp);
		}
		rcode = RLM_MODULE_HANDLED;
		break;

	default:
		RDEBUG("Unknown RADIUS packet type %d: rejecting tunneled user", reply->code);
		rcode = RLM_MODULE_INVALID;
		break;
	}

	return rcode;
}


#ifdef WITH_PROXY
/*
 *	Do post-proxy processing,
 */
static int eapttls_postproxy(eap_handler_t *handler, void *data)
{
	int rcode;
	tls_session_t *tls_session = (tls_session_t *) data;
	REQUEST *fake, *request = handler->request;

	rad_assert(request != NULL);
	RDEBUG("Passing reply from proxy back into the tunnel.");

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
	if (fake &&
	    handler->request->proxy_reply &&
	    (handler->request->proxy_reply->code == PW_AUTHENTICATION_ACK)) {
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
			fprintf(fr_log_fp, "server %s {\n",
				(!fake->server) ? "" : fake->server);
		}

		/*
		 *	Perform a post-auth stage for the tunneled
		 *	session.
		 */
		fake->options &= ~RAD_REQUEST_OPTION_PROXY_EAP;
		rcode = rad_postauth(fake);
		RDEBUG2("post-auth returns %d", rcode);

		if ((debug_flag > 0) && fr_log_fp) {
			fprintf(fr_log_fp, "} # server %s\n",
				(!fake->server) ? "" : fake->server);

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
			RDEBUG2("Got reply %d",
			       request->proxy_reply->code);
			break;
		}
	}
	request_free(&fake);	/* robust if !fake */

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
		RDEBUG("Reply was rejected");
		break;

	case RLM_MODULE_HANDLED:
		RDEBUG("Reply was handled");
		eaptls_request(handler->eap_ds, tls_session);
		return 1;

	case RLM_MODULE_OK:
		RDEBUG("Reply was OK");

		/*
		 *	Success: Automatically return MPPE keys.
		 */
		return eaptls_success(handler, 0);

	default:
		RDEBUG("Reply was unknown.");
		break;
	}

	eaptls_fail(handler, 0);
	return 0;
}

#endif	/* WITH_PROXY */

/*
 *	Process the "diameter" contents of the tunneled data.
 */
int eapttls_process(eap_handler_t *handler, tls_session_t *tls_session)
{
	int code = PW_AUTHENTICATION_REJECT;
	rlm_rcode_t rcode;
	REQUEST *fake;
	VALUE_PAIR *vp;
	ttls_tunnel_t *t;
	uint8_t const *data;
	size_t data_len;
	REQUEST *request = handler->request;

	rad_assert(request != NULL);

	/*
	 *	Just look at the buffer directly, without doing
	 *	record_minus.
	 */
	data_len = tls_session->clean_out.used;
	tls_session->clean_out.used = 0;
	data = tls_session->clean_out.data;

	t = (ttls_tunnel_t *) tls_session->opaque;

	/*
	 *	If there's no data, maybe this is an ACK to an
	 *	MS-CHAP2-Success.
	 */
	if (data_len == 0) {
		if (t->authenticated) {
			RDEBUG("Got ACK, and the user was already authenticated.");
			return PW_AUTHENTICATION_ACK;
		} /* else no session, no data, die. */

		/*
		 *	FIXME: Call SSL_get_error() to see what went
		 *	wrong.
		 */
		RDEBUG2("SSL_read Error");
		return PW_AUTHENTICATION_REJECT;
	}

#ifndef NDEBUG
	if ((debug_flag > 2) && fr_log_fp) {
		size_t i;

		for (i = 0; i < data_len; i++) {
			if ((i & 0x0f) == 0) fprintf(fr_log_fp, "  TTLS tunnel data in %04x: ", (int) i);

			fprintf(fr_log_fp, "%02x ", data[i]);

			if ((i & 0x0f) == 0x0f) fprintf(fr_log_fp, "\n");
		}
		if ((data_len & 0x0f) != 0) fprintf(fr_log_fp, "\n");
	}
#endif

	if (!diameter_verify(request, data, data_len)) {
		return PW_AUTHENTICATION_REJECT;
	}

	/*
	 *	Allocate a fake REQUEST structe.
	 */
	fake = request_alloc_fake(request);

	rad_assert(!fake->packet->vps);

	/*
	 *	Add the tunneled attributes to the fake request.
	 */
	fake->packet->vps = diameter2vp(request, fake, tls_session->ssl, data, data_len);
	if (!fake->packet->vps) {
		request_free(&fake);
		return PW_AUTHENTICATION_REJECT;
	}

	/*
	 *	Tell the request that it's a fake one.
	 */
	pairmake_packet("Freeradius-Proxied-To", "127.0.0.1", T_OP_EQ);

	if ((debug_flag > 0) && fr_log_fp) {
		RDEBUG("Got tunneled request");

		debug_pair_list(fake->packet->vps);
	}

	/*
	 *	Update other items in the REQUEST data structure.
	 */
	fake->username = pairfind(fake->packet->vps, PW_USER_NAME, 0, TAG_ANY);
	fake->password = pairfind(fake->packet->vps, PW_USER_PASSWORD, 0, TAG_ANY);

	/*
	 *	No User-Name, try to create one from stored data.
	 */
	if (!fake->username) {
		/*
		 *	No User-Name in the stored data, look for
		 *	an EAP-Identity, and pull it out of there.
		 */
		if (!t->username) {
			vp = pairfind(fake->packet->vps, PW_EAP_MESSAGE, 0, TAG_ANY);
			if (vp &&
			    (vp->length >= EAP_HEADER_LEN + 2) &&
			    (vp->vp_strvalue[0] == PW_EAP_RESPONSE) &&
			    (vp->vp_strvalue[EAP_HEADER_LEN] == PW_EAP_IDENTITY) &&
			    (vp->vp_strvalue[EAP_HEADER_LEN + 1] != 0)) {
				char *p;

				/*
				 *	Create & remember a User-Name
				 */
				t->username = pairmake(t, NULL, "User-Name", NULL, T_OP_EQ);
				rad_assert(t->username != NULL);
				t->username->length = vp->length - 5;

				t->username->vp_strvalue = p = talloc_array(t->username, char,
									    t->username->length + 1);
				memcpy(p, vp->vp_octets + 5, t->username->length);
				p[t->username->length] = 0;

				RDEBUG("Got tunneled identity of %s",
				       t->username->vp_strvalue);

				/*
				 *	If there's a default EAP type,
				 *	set it here.
				 */
				if (t->default_method != 0) {
					RDEBUG("Setting default EAP type for tunneled EAP session.");
					vp = paircreate(fake, PW_EAP_TYPE, 0);
					rad_assert(vp != NULL);
					vp->vp_integer = t->default_method;
					pairadd(&fake->config_items, vp);
				}

			} else {
				/*
				 *	Don't reject the request outright,
				 *	as it's permitted to do EAP without
				 *	user-name.
				 */
				RWDEBUG2("No EAP-Identity found to start EAP conversation.");
			}
		} /* else there WAS a t->username */

		if (t->username) {
			vp = paircopy(fake->packet, t->username);
			pairadd(&fake->packet->vps, vp);
			fake->username = pairfind(fake->packet->vps, PW_USER_NAME, 0, TAG_ANY);
		}
	} /* else the request ALREADY had a User-Name */

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
	 */
	if (t->copy_request_to_tunnel) {
		VALUE_PAIR *copy;
		vp_cursor_t cursor;

		for (vp = paircursor(&cursor, &request->packet->vps); vp; vp = pairnext(&cursor)) {
			/*
			 *	The attribute is a server-side thingy,
			 *	don't copy it.
			 */
			if ((vp->da->attr > 255) &&
			    (vp->da->vendor == 0)) {
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

	if ((vp = pairfind(request->config_items, PW_VIRTUAL_SERVER, 0, TAG_ANY)) != NULL) {
		fake->server = vp->vp_strvalue;

	} else if (t->virtual_server) {
		fake->server = t->virtual_server;

	} /* else fake->server == request->server */


	if ((debug_flag > 0) && fr_log_fp) {
		RDEBUG("Sending tunneled request");

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
			RDEBUG("Tunneled authentication will be proxied to %s", vp->vp_strvalue);

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
			memset(&request->proxy->src_ipaddr, 0,
			       sizeof(request->proxy->src_ipaddr));
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
			tunnel->callback = eapttls_postproxy;

			/*
			 *	Associate the callback with the request.
			 */
			code = request_data_add(request, request->proxy, REQUEST_DATA_EAP_TUNNEL_CALLBACK,
						tunnel, false);
			rad_assert(code == 0);

			/*
			 *	rlm_eap.c has taken care of associating
			 *	the handler with the fake request.
			 *
			 *	So we associate the fake request with
			 *	this request.
			 */
			talloc_set_destructor(fake, request_opaque_free);
			code = request_data_add(request, request->proxy, REQUEST_DATA_EAP_MSCHAP_TUNNEL_CALLBACK,
						fake, true);
			rad_assert(code == 0);
			fake = NULL;

			/*
			 *	Didn't authenticate the packet, but
			 *	we're proxying it.
			 */
			code = PW_STATUS_CLIENT;

		} else
#endif	/* WITH_PROXY */
		  {
			RDEBUG("No tunneled reply was found for request %d , and the request was not proxied: rejecting the user.",
			       request->number);
			code = PW_AUTHENTICATION_REJECT;
		}
		break;

	default:
		/*
		 *	Returns RLM_MODULE_FOO, and we want to return PW_FOO
		 */
		rcode = process_reply(handler, tls_session, request, fake->reply);
		switch (rcode) {
		case RLM_MODULE_REJECT:
			code = PW_AUTHENTICATION_REJECT;
			break;

		case RLM_MODULE_HANDLED:
			code = PW_ACCESS_CHALLENGE;
			break;

		case RLM_MODULE_OK:
			code = PW_AUTHENTICATION_ACK;
			break;

		default:
			code = PW_AUTHENTICATION_REJECT;
			break;
		}
		break;
	}

	request_free(&fake);

	return code;
}
