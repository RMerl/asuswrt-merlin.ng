/*
 * eapcommon.c    rfc2284 & rfc2869 implementation
 *
 * code common to clients and to servers.
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
 * Copyright 2000-2003,2006  The FreeRADIUS server project
 * Copyright 2001  hereUare Communications, Inc. <raghud@hereuare.com>
 * Copyright 2003  Alan DeKok <aland@freeradius.org>
 * Copyright 2003  Michael Richardson <mcr@sandelman.ottawa.on.ca>
 */
/*
 *  EAP PACKET FORMAT
 *  --- ------ ------
 *  0		   1		   2		   3
 *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |     Code      |  Identifier   |	    Length	     |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |    Data ...
 * +-+-+-+-+
 *
 *
 * EAP Request and Response Packet Format
 * --- ------- --- -------- ------ ------
 *  0		   1		   2		   3
 *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |     Code      |  Identifier   |	    Length	     |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |     Type      |  Type-Data ...
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
 *
 *
 * EAP Success and Failure Packet Format
 * --- ------- --- ------- ------ ------
 *  0		   1		   2		   3
 *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |     Code      |  Identifier   |	    Length	     |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 */

RCSID("$Id$")

#include <freeradius-devel/libradius.h>
#include "eap_types.h"

const FR_NAME_NUMBER eap_rcode_table[] = {
	{ "notfound",		EAP_NOTFOUND		},
	{ "found",		EAP_OK			},
	{ "ok",			EAP_FAIL		},
	{ "fail",		EAP_NOOP		},
	{ "noop",		EAP_INVALID		},
	{ "invalid",		EAP_VALID		},
	{ "valid",		EAP_MAX_RCODES		},

	{  NULL , -1 }
};

/** Return an EAP-Type for a particular name
 *
 * Converts a name into an IANA EAP type.
 *
 * @param name to convert.
 * @return The IANA EAP type or PW_EAP_INVALID if the name doesn't match any
 * known types.
 */
eap_type_t eap_name2type(char const *name)
{
	DICT_VALUE	*dv;

	dv = dict_valbyname(PW_EAP_TYPE, 0, name);
	if (dv) {
		return dv->value;
	}

	return PW_EAP_INVALID;
}

/** Return an EAP-name for a particular type
 *
 * Resolve
 */
char const *eap_type2name(eap_type_t method)
{
	DICT_VALUE	*dv;

	dv = dict_valbyattr(PW_EAP_TYPE, 0, method);
	if (dv) {
		return dv->name;
	}

	return "unknown";
}

/*
 *	EAP packet format to be sent over the wire
 *
 *	i.e. code+id+length+data where data = null/type+typedata
 *	based on code.
 *
 * INPUT to function is reply->code
 *		      reply->id
 *		      reply->type   - setup with data
 *
 * OUTPUT reply->packet is setup with wire format, and will
 *		      be allocated to the right size.
 *
 */
int eap_wireformat(eap_packet_t *reply)
{
	eap_packet_raw_t	*header;
	uint16_t total_length = 0;

	if (!reply) return EAP_INVALID;

	/*
	 *	If reply->packet is set, then the wire format
	 *	has already been calculated, just succeed.
	 */
	if(reply->packet != NULL) return EAP_VALID;

	total_length = EAP_HEADER_LEN;
	if (reply->code < 3) {
		total_length += 1/* EAP Method */;
		if (reply->type.data && reply->type.length > 0) {
			total_length += reply->type.length;
		}
	}

	reply->packet = talloc_array(reply, uint8_t, total_length);
	header = (eap_packet_raw_t *)reply->packet;
	if (!header) {
		return EAP_INVALID;
	}

	header->code = (reply->code & 0xFF);
	header->id = (reply->id & 0xFF);

	total_length = htons(total_length);
	memcpy(header->length, &total_length, sizeof(total_length));

	/*
	 *	Request and Response packets are special.
	 */
	if ((reply->code == PW_EAP_REQUEST) ||
	    (reply->code == PW_EAP_RESPONSE)) {
		header->data[0] = (reply->type.num & 0xFF);

		/*
		 * Here since we cannot know the typedata format and length
		 *
		 * Type_data is expected to be wired by each EAP-Type
		 *
		 * Zero length/No typedata is supported as long as
		 * type is defined
		 */
		if (reply->type.data && reply->type.length > 0) {
			memcpy(&header->data[1], reply->type.data, reply->type.length);
			talloc_free(reply->type.data);
			reply->type.data = reply->packet + EAP_HEADER_LEN + 1/*EAPtype*/;
		}
	}

	return EAP_VALID;
}


/*
 *	compose EAP reply packet in EAP-Message attr of RADIUS.  If
 *	EAP exceeds 253, frame it in multiple EAP-Message attrs.
 */
int eap_basic_compose(RADIUS_PACKET *packet, eap_packet_t *reply)
{
	VALUE_PAIR *vp;
	eap_packet_raw_t *eap_packet;
	int rcode;

	if (eap_wireformat(reply) == EAP_INVALID) {
		return RLM_MODULE_INVALID;
	}
	eap_packet = (eap_packet_raw_t *)reply->packet;

	pairdelete(&(packet->vps), PW_EAP_MESSAGE, 0, TAG_ANY);

	vp = eap_packet2vp(packet, eap_packet);
	if (!vp) return RLM_MODULE_INVALID;
	pairadd(&(packet->vps), vp);

	/*
	 *	EAP-Message is always associated with
	 *	Message-Authenticator but not vice-versa.
	 *
	 *	Don't add a Message-Authenticator if it's already
	 *	there.
	 */
	vp = pairfind(packet->vps, PW_MESSAGE_AUTHENTICATOR, 0, TAG_ANY);
	if (!vp) {
		vp = paircreate(packet, PW_MESSAGE_AUTHENTICATOR, 0);
		vp->length = AUTH_VECTOR_LEN;
		vp->vp_octets = talloc_zero_array(vp, uint8_t, vp->length);

		pairadd(&(packet->vps), vp);
	}

	/* Set request reply code, but only if it's not already set. */
	rcode = RLM_MODULE_OK;
	if (!packet->code) switch(reply->code) {
	case PW_EAP_RESPONSE:
	case PW_EAP_SUCCESS:
		packet->code = PW_AUTHENTICATION_ACK;
		rcode = RLM_MODULE_HANDLED;
		break;
	case PW_EAP_FAILURE:
		packet->code = PW_AUTHENTICATION_REJECT;
		rcode = RLM_MODULE_REJECT;
		break;
	case PW_EAP_REQUEST:
		packet->code = PW_ACCESS_CHALLENGE;
		rcode = RLM_MODULE_HANDLED;
		break;
	default:
		/* Should never enter here */
		ERROR("rlm_eap: reply code %d is unknown, Rejecting the request.", reply->code);
		packet->code = PW_AUTHENTICATION_REJECT;
		break;
	}

	return rcode;
}


VALUE_PAIR *eap_packet2vp(RADIUS_PACKET *packet, eap_packet_raw_t const *eap)
{
	int		total, size;
	uint8_t const *ptr;
	VALUE_PAIR	*head = NULL;
	VALUE_PAIR	*vp;
	vp_cursor_t	out;

	total = eap->length[0] * 256 + eap->length[1];

	if (total == 0) {
		DEBUG("Asked to encode empty EAP-Message!");
		return NULL;
	}

	ptr = (uint8_t const *) eap;

	paircursor(&out, &head);
	do {
		size = total;
		if (size > 253) size = 253;

		vp = paircreate(packet, PW_EAP_MESSAGE, 0);
		if (!vp) {
			pairfree(&head);
			return NULL;
		}
		pairmemcpy(vp, ptr, size);

		pairinsert(&out, vp);

		ptr += size;
		total -= size;
	} while (total > 0);

	return head;
}


/*
 * Handles multiple EAP-Message attrs
 * ie concatenates all to get the complete EAP packet.
 *
 * NOTE: Sometimes Framed-MTU might contain the length of EAP-Message,
 *      refer fragmentation in rfc2869.
 */
eap_packet_raw_t *eap_vp2packet(TALLOC_CTX *ctx, VALUE_PAIR *vps)
{
	VALUE_PAIR *first, *i;
	eap_packet_raw_t *eap_packet;
	unsigned char *ptr;
	uint16_t len;
	int total_len;
	vp_cursor_t cursor;

	/*
	 *	Get only EAP-Message attribute list
	 */
	first = pairfind(vps, PW_EAP_MESSAGE, 0, TAG_ANY);
	if (!first) {
		DEBUG("rlm_eap: EAP-Message not found");
		return NULL;
	}

	/*
	 *	Sanity check the length before doing anything.
	 */
	if (first->length < 4) {
		DEBUG("rlm_eap: EAP packet is too short.");
		return NULL;
	}

	/*
	 *	Get the Actual length from the EAP packet
	 *	First EAP-Message contains the EAP packet header
	 */
	memcpy(&len, first->vp_strvalue + 2, sizeof(len));
	len = ntohs(len);

	/*
	 *	Take out even more weird things.
	 */
	if (len < 4) {
		DEBUG("rlm_eap: EAP packet has invalid length.");
		return NULL;
	}

	/*
	 *	Sanity check the length, BEFORE allocating  memory.
	 */
	total_len = 0;
	paircursor(&cursor, &first);
	while ((i = pairfindnext(&cursor, PW_EAP_MESSAGE, 0, TAG_ANY))) {
		total_len += i->length;

		if (total_len > len) {
			DEBUG("rlm_eap: Malformed EAP packet.  Length in packet header does not match actual length");
			return NULL;
		}
	}

	/*
	 *	If the length is SMALLER, die, too.
	 */
	if (total_len < len) {
		DEBUG("rlm_eap: Malformed EAP packet.  Length in packet header does not match actual length");
		return NULL;
	}

	/*
	 *	Now that we know the lengths are OK, allocate memory.
	 */
	eap_packet = (eap_packet_raw_t *) talloc_zero_array(ctx, uint8_t, len);
	if (!eap_packet) {
		return NULL;
	}

	/*
	 *	Copy the data from EAP-Message's over to our EAP packet.
	 */
	ptr = (unsigned char *)eap_packet;

	/* RADIUS ensures order of attrs, so just concatenate all */
	pairfirst(&cursor);
	while ((i = pairfindnext(&cursor, PW_EAP_MESSAGE, 0, TAG_ANY))) {
		memcpy(ptr, i->vp_strvalue, i->length);
		ptr += i->length;
	}

	return eap_packet;
}

/*
 *	Add raw hex data to the reply.
 */
void eap_add_reply(REQUEST *request,
		   char const *name, uint8_t const *value, int len)
{
	VALUE_PAIR *vp;

	vp = pairmake_reply(name, NULL, T_OP_EQ);
	if (!vp) {
		REDEBUG("Did not create attribute %s: %s\n",
			name, fr_strerror());
		return;
	}

	pairmemcpy(vp, value, len);
}
