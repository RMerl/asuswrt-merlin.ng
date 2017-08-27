/*
 * vqp.c	Functions to send/receive VQP packets.
 *
 * Version:	$Id$
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) any later version.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 *   Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this library; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 *
 * Copyright 2007 Alan DeKok <aland@deployingradius.com>
 */

RCSID("$Id$");

#include	<freeradius-devel/libradius.h>
#include	<freeradius-devel/udpfromto.h>

#include	"vqp.h"

#define MAX_VMPS_LEN (MAX_STRING_LEN - 1)

/* @todo: this is a hack */
#  define DEBUG			if (fr_debug_flag && fr_log_fp) fr_printf_log
void fr_strerror_printf(char const *fmt, ...);
#  define debug_pair(vp)	do { if (fr_debug_flag && fr_log_fp) { \
					vp_print(fr_log_fp, vp); \
				     } \
				} while(0)
/*
 *  http://www.openbsd.org/cgi-bin/cvsweb/src/usr.sbin/tcpdump/print-vqp.c
 *
 *  Some of how it works:
 *
 *  http://www.hackingciscoexposed.com/pdf/chapter12.pdf
 *
 * VLAN Query Protocol (VQP)
 *
 *    0		   1		   2		   3
 *    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   |    Version    |    Opcode     | Response Code |  Data Count   |
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   |			 Transaction ID			|
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   |			    Type (1)			   |
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   |	     Length	    |	    Data	       /
 *   /							       /
 *   /							       /
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   |			    Type (n)			   |
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   |	     Length	    |	    Data	       /
 *   /							       /
 *   /							       /
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * VQP is layered over UDP.  The default destination port is 1589.
 *
 */
#define VQP_HDR_LEN (8)
#define VQP_VERSION (1)
#define VQP_MAX_ATTRIBUTES (12)


/*
 *	Wrapper for sendto which handles sendfromto, IPv6, and all
 *	possible combinations.
 *
 *	FIXME:  This is just a copy of rad_sendto().
 *	Duplicate code is bad.
 */
static int vqp_sendto(int sockfd, void *data, size_t data_len, int flags,
#ifdef WITH_UDPFROMTO
		      fr_ipaddr_t *src_ipaddr,
#else
		      UNUSED fr_ipaddr_t *src_ipaddr,
#endif
		      fr_ipaddr_t *dst_ipaddr,
		      int dst_port)
{
	struct sockaddr_storage	dst;
	socklen_t		sizeof_dst;

#ifdef WITH_UDPFROMTO
	struct sockaddr_storage	src;
	socklen_t		sizeof_src;

	if (!fr_ipaddr2sockaddr(src_ipaddr, 0, &src, &sizeof_src)) {
		return -1;   /* Unknown address family, Die Die Die! */
	}
#endif

	if (!fr_ipaddr2sockaddr(dst_ipaddr, dst_port, &dst, &sizeof_dst)) {
		return -1;   /* Unknown address family, Die Die Die! */
	}

#ifdef WITH_UDPFROMTO
	/*
	 *	Only IPv4 is supported for udpfromto.
	 *
	 *	And if they don't specify a source IP address, don't
	 *	use udpfromto.
	 */
	if ((dst_ipaddr->af == AF_INET) &&
	    (src_ipaddr->af != AF_UNSPEC)) {
		return sendfromto(sockfd, data, data_len, flags,
				  (struct sockaddr *)&src, sizeof_src,
				  (struct sockaddr *)&dst, sizeof_dst);
	}
#endif

	/*
	 *	No udpfromto, OR an IPv6 socket, fail gracefully.
	 */
	return sendto(sockfd, data, data_len, flags,
		      (struct sockaddr *)&dst, sizeof_dst);
}

/*
 *	Wrapper for recvfrom, which handles recvfromto, IPv6, and all
 *	possible combinations.
 *
 *	FIXME:  This is copied from rad_recvfrom, with minor edits.
 */
static ssize_t vqp_recvfrom(int sockfd, RADIUS_PACKET *packet, int flags,
			    fr_ipaddr_t *src_ipaddr, uint16_t *src_port,
			    fr_ipaddr_t *dst_ipaddr, uint16_t *dst_port)
{
	struct sockaddr_storage	src;
	struct sockaddr_storage	dst;
	socklen_t		sizeof_src = sizeof(src);
	socklen_t		sizeof_dst = sizeof(dst);
	ssize_t			data_len;
	uint8_t			header[4];
	size_t			len;
	int			port;

	memset(&src, 0, sizeof_src);
	memset(&dst, 0, sizeof_dst);

	/*
	 *	Get address family, etc. first, so we know if we
	 *	need to do udpfromto.
	 *
	 *	FIXME: udpfromto also does this, but it's not
	 *	a critical problem.
	 */
	if (getsockname(sockfd, (struct sockaddr *)&dst,
			&sizeof_dst) < 0) return -1;

	/*
	 *	Read the length of the packet, from the packet.
	 *	This lets us allocate the buffer to use for
	 *	reading the rest of the packet.
	 */
	data_len = recvfrom(sockfd, header, sizeof(header), MSG_PEEK,
			    (struct sockaddr *)&src, &sizeof_src);
	if (data_len < 0) return -1;

	/*
	 *	Too little data is available, discard the packet.
	 */
	if (data_len < 4) {
		rad_recv_discard(sockfd);

		return 0;

	/*
	 *	Invalid version, packet type, or too many
	 *	attributes.  Die.
	 */
	} else if ((header[0] != VQP_VERSION) ||
		   (header[1] < 1) ||
		   (header[1] > 4) ||
		   (header[3] > VQP_MAX_ATTRIBUTES)) {
		rad_recv_discard(sockfd);

		return 0;

	} else {		/* we got 4 bytes of data. */
		/*
		 *	We don't care about the contents for now...
		 */
#if 0
		/*
		 *	How many attributes are in the packet.
		 */
		len = header[3];

		if ((header[1] == 1) || (header[1] == 3)) {
			if (len != VQP_MAX_ATTRIBUTES) {
				rad_recv_discard(sockfd);

				return 0;
			}
			/*
			 *	Maximum length we support.
			 */
			len = (12 * (4 + 4 + MAX_VMPS_LEN));

		} else {
			if (len != 2) {
				rad_recv_discard(sockfd);

				return 0;
			}
			/*
			 *	Maximum length we support.
			 */
			len = (12 * (4 + 4 + MAX_VMPS_LEN));
		}
#endif
	}

	/*
	 *	For now, be generous.
	 */
	len = (12 * (4 + 4 + MAX_VMPS_LEN));

	packet->data = talloc_array(packet, uint8_t, len);
	if (!packet->data) return -1;

	/*
	 *	Receive the packet.  The OS will discard any data in the
	 *	packet after "len" bytes.
	 */
#ifdef WITH_UDPFROMTO
	if (dst.ss_family == AF_INET) {
		data_len = recvfromto(sockfd, packet->data, len, flags,
				      (struct sockaddr *)&src, &sizeof_src,
				      (struct sockaddr *)&dst, &sizeof_dst);
	} else
#endif
		/*
		 *	No udpfromto, OR an IPv6 socket.  Fail gracefully.
		 */
		data_len = recvfrom(sockfd, packet->data, len, flags,
				    (struct sockaddr *)&src, &sizeof_src);
	if (data_len < 0) {
		return data_len;
	}

	if (!fr_sockaddr2ipaddr(&src, sizeof_src, src_ipaddr, &port)) {
		return -1;	/* Unknown address family, Die Die Die! */
	}
	*src_port = port;

	fr_sockaddr2ipaddr(&dst, sizeof_dst, dst_ipaddr, &port);
	*dst_port = port;

	/*
	 *	Different address families should never happen.
	 */
	if (src.ss_family != dst.ss_family) {
		return -1;
	}

	return data_len;
}

RADIUS_PACKET *vqp_recv(int sockfd)
{
	uint8_t *ptr;
	ssize_t length;
	uint32_t id;
	RADIUS_PACKET *packet;

	/*
	 *	Allocate the new request data structure
	 */
	packet = rad_alloc(NULL, 0);
	if (!packet) {
		fr_strerror_printf("out of memory");
		return NULL;
	}

	length = vqp_recvfrom(sockfd, packet, 0,
			      &packet->src_ipaddr, &packet->src_port,
			      &packet->dst_ipaddr, &packet->dst_port);

	/*
	 *	Check for socket errors.
	 */
	if (length < 0) {
		fr_strerror_printf("Error receiving packet: %s", strerror(errno));
		/* packet->data is NULL */
		rad_free(&packet);
		return NULL;
	}
	packet->data_len = length; /* unsigned vs signed */

	/*
	 *	We can only receive packets formatted in a way we
	 *	expect.  However, we accept MORE attributes in a
	 *	packet than normal implementations may send.
	 */
	if (packet->data_len < VQP_HDR_LEN) {
		fr_strerror_printf("VQP packet is too short");
		rad_free(&packet);
		return NULL;
	}

	ptr = packet->data;

	if (0) {
		size_t i;
		for (i = 0; i < packet->data_len; i++) {
		  if ((i & 0x0f) == 0) fprintf(stderr, "%02x: ", (int) i);
			fprintf(stderr, "%02x ", ptr[i]);
			if ((i & 0x0f) == 0x0f) fprintf(stderr, "\n");
		}

	}

	if (ptr[3] > VQP_MAX_ATTRIBUTES) {
		fr_strerror_printf("Too many VQP attributes");
		rad_free(&packet);
		return NULL;
	}

	if (packet->data_len > VQP_HDR_LEN) {
		int attrlen;

		/*
		 *	Skip the header.
		 */
		ptr += VQP_HDR_LEN;
		length = packet->data_len - VQP_HDR_LEN;

		while (length > 0) {
			if (length < 7) {
				fr_strerror_printf("Packet contains malformed attribute");
				rad_free(&packet);
				return NULL;
			}

			/*
			 *	Attributes are 4 bytes
			 *	0x00000c01 ... 0x00000c08
			 */
			if ((ptr[0] != 0) || (ptr[1] != 0) ||
			    (ptr[2] != 0x0c) || (ptr[3] < 1) || (ptr[3] > 8)) {
				fr_strerror_printf("Packet contains invalid attribute");
				rad_free(&packet);
				return NULL;
			}

			/*
			 *	Length is 2 bytes
			 *
			 *	We support lengths 1..253, for internal
			 *	server reasons.  Also, there's no reason
			 *	for bigger lengths to exist... admins
			 *	won't be typing in a 32K vlan name.
			 *
			 *	Except for received ethernet frames...
			 *	they get chopped to 253 internally.
			 */
			if ((ptr[3] != 5) &&
			    ((ptr[4] != 0) || (ptr[5] > MAX_VMPS_LEN))) {
				fr_strerror_printf("Packet contains attribute with invalid length %02x %02x", ptr[4], ptr[5]);
				rad_free(&packet);
				return NULL;
			}
			attrlen = (ptr[4] << 8) | ptr[5];
			ptr += 6 + attrlen;
			length -= (6 + attrlen);
		}
	}

	packet->sockfd = sockfd;
	packet->vps = NULL;

	/*
	 *	This is more than a bit of a hack.
	 */
	packet->code = PW_AUTHENTICATION_REQUEST;

	memcpy(&id, packet->data + 4, 4);
	packet->id = ntohl(id);

	/*
	 *	FIXME: Create a fake "request authenticator", to
	 *	avoid duplicates?  Or is the VQP sequence number
	 *	adequate for this purpose?
	 */

	return packet;
}

/*
 *	We do NOT  mirror the old-style RADIUS code  that does encode,
 *	sign && send in one function.  For VQP, the caller MUST perform
 *	each task manually, and separately.
 */
int vqp_send(RADIUS_PACKET *packet)
{
	if (!packet || !packet->data || (packet->data_len < 8)) return -1;

	/*
	 *	Don't print out the attributes, they were printed out
	 *	when it was encoded.
	 */

	/*
	 *	And send it on it's way.
	 */
	return vqp_sendto(packet->sockfd, packet->data, packet->data_len, 0,
			  &packet->src_ipaddr, &packet->dst_ipaddr,
			  packet->dst_port);
}


int vqp_decode(RADIUS_PACKET *packet)
{
	uint8_t *ptr, *end;
	int attribute, length;
	vp_cursor_t cursor;
	VALUE_PAIR *vp;

	if (!packet || !packet->data) return -1;

	if (packet->data_len < VQP_HDR_LEN) return -1;

	paircursor(&cursor, &packet->vps);
	vp = paircreate(packet, PW_VQP_PACKET_TYPE, 0);
	if (!vp) {
		fr_strerror_printf("No memory");
		return -1;
	}
	vp->vp_integer = packet->data[1];
	debug_pair(vp);
	pairinsert(&cursor, vp);

	vp = paircreate(packet, PW_VQP_ERROR_CODE, 0);
	if (!vp) {
		fr_strerror_printf("No memory");
		return -1;
	}
	vp->vp_integer = packet->data[2];
	debug_pair(vp);
	pairinsert(&cursor, vp);

	vp = paircreate(packet, PW_VQP_SEQUENCE_NUMBER, 0);
	if (!vp) {
		fr_strerror_printf("No memory");
		return -1;
	}
	vp->vp_integer = packet->id; /* already set by vqp_recv */
	debug_pair(vp);
	pairinsert(&cursor, vp);

	ptr = packet->data + VQP_HDR_LEN;
	end = packet->data + packet->data_len;

	/*
	 *	Note that vqp_recv() MUST ensure that the packet is
	 *	formatted in a way we expect, and that vqp_recv() MUST
	 *	be called before vqp_decode().
	 */
	while (ptr < end) {
		char *p;

		attribute = (ptr[2] << 8) | ptr[3];
		length = (ptr[4] << 8) | ptr[5];
		ptr += 6;

		/*
		 *	Hack to get the dictionaries to work correctly.
		 */
		attribute |= 0x2000;
		vp = paircreate(packet, attribute, 0);
		if (!vp) {
			pairfree(&packet->vps);

			fr_strerror_printf("No memory");
			return -1;
		}

		switch (vp->da->type) {
		case PW_TYPE_IPADDR:
			if (length == 4) {
				memcpy(&vp->vp_ipaddr, ptr, 4);
				vp->length = 4;
				break;
			}

			/*
			 *	Value doesn't match the type we have for the
			 *	valuepair so we must change it's da to an
			 *	unknown attr.
			 */
			vp->da = dict_attrunknown(vp->da->attr, vp->da->vendor,
						  true);
			/* FALL-THROUGH */

		default:
		case PW_TYPE_OCTETS:
			pairmemcpy(vp, ptr, length);
			break;

		case PW_TYPE_STRING:
			vp->length = length;
			vp->vp_strvalue = p = talloc_array(vp, char, vp->length + 1);
			vp->type = VT_DATA;
			memcpy(p, ptr, vp->length);
			p[vp->length] = '\0';
			break;
		}
		ptr += length;
		debug_pair(vp);
		pairinsert(&cursor, vp);
	}

	/*
	 *	FIXME: Map attributes to Calling-Station-Id, etc...
	 */

	return 0;
}

/*
 *	These are the MUST HAVE contents for a VQP packet.
 *
 *	We don't allow the caller to give less than these, because
 *	it won't work.  We don't encode more than these, because the
 *	clients will ignore it.
 *
 *	FIXME: Be more generous?  Look for CISCO + VQP attributes?
 */
static int contents[5][VQP_MAX_ATTRIBUTES] = {
	{ 0,      0,      0,      0,      0,      0 },
	{ 0x0c01, 0x0c02, 0x0c03, 0x0c04, 0x0c07, 0x0c05 }, /* Join request */
	{ 0x0c03, 0x0c08, 0,      0,      0,      0 },	/* Join Response */
	{ 0x0c01, 0x0c02, 0x0c03, 0x0c04, 0x0c07, 0x0c08 }, /* Reconfirm */
	{ 0x0c03, 0x0c08, 0,      0,      0,      0 }
};

int vqp_encode(RADIUS_PACKET *packet, RADIUS_PACKET *original)
{
	int i, code, length;
	VALUE_PAIR *vp;
	uint8_t *ptr;
	VALUE_PAIR	*vps[VQP_MAX_ATTRIBUTES];

	if (!packet) {
		fr_strerror_printf("Failed encoding VQP");
		return -1;
	}

	if (packet->data) return 0;

	vp = pairfind(packet->vps, PW_VQP_PACKET_TYPE, 0, TAG_ANY);
	if (!vp) {
		fr_strerror_printf("Failed to find VQP-Packet-Type in response packet");
		return -1;
	}

	code = vp->vp_integer;
	if ((code < 1) || (code > 4)) {
		fr_strerror_printf("Invalid value %d for VQP-Packet-Type", code);
		return -1;
	}

	length = VQP_HDR_LEN;
	memset(vps, 0, sizeof(vps));

	vp = pairfind(packet->vps, PW_VQP_ERROR_CODE, 0, TAG_ANY);

	/*
	 *	FIXME: Map attributes from calling-station-Id, etc.
	 *
	 *	Maybe do this via rlm_vqp?  That's probably the
	 *	best place to add the code...
	 */

	/*
	 *	No error: encode attributes.
	 */
	if (!vp) for (i = 0; i < VQP_MAX_ATTRIBUTES; i++) {
		if (!contents[code][i]) break;

		vps[i] = pairfind(packet->vps, contents[code][i] | 0x2000, 0, TAG_ANY);

		/*
		 *	FIXME: Print the name...
		 */
		if (!vps[i]) {
			fr_strerror_printf("Failed to find VQP attribute %02x",
				   contents[code][i]);
			return -1;
		}

		length += 6;
		length += vps[i]->length;
	}

	packet->data = talloc_array(packet, uint8_t, length);
	if (!packet->data) {
		fr_strerror_printf("No memory");
		return -1;
	}
	packet->data_len = length;

	ptr = packet->data;

	ptr[0] = VQP_VERSION;
	ptr[1] = code;

	if (!vp) {
		ptr[2] = 0;
	} else {
		ptr[2] = vp->vp_integer & 0xff;
		return 0;
	}

	/*
	 *	The number of attributes is hard-coded.
	 */
	if ((code == 1) || (code == 3)) {
		uint32_t sequence;

		ptr[3] = VQP_MAX_ATTRIBUTES;

		sequence = htonl(packet->id);
		memcpy(ptr + 4, &sequence, 4);
	} else {
		if (!original) {
			fr_strerror_printf("Cannot send VQP response without request");
			return -1;
		}

		/*
		 *	Packet Sequence Number
		 */
		memcpy(ptr + 4, original->data + 4, 4);

		ptr[3] = 2;
	}

	ptr += 8;

	/*
	 *	Encode the VP's.
	 */
	for (i = 0; i < VQP_MAX_ATTRIBUTES; i++) {
		if (!vps[i]) break;
		if (ptr >= (packet->data + packet->data_len)) break;

		vp = vps[i];

		debug_pair(vp);

		/*
		 *	Type.  Note that we look at only the lower 8
		 *	bits, as the upper 8 bits have been hacked.
		 *	See also dictionary.vqp
		 */
		ptr[0] = 0;
		ptr[1] = 0;
		ptr[2] = 0x0c;
		ptr[3] = vp->da->attr & 0xff;

		/* Length */
		ptr[4] = 0;
		ptr[5] = vp->length & 0xff;

		ptr += 6;

		/* Data */
		switch (vp->da->type) {
		case PW_TYPE_IPADDR:
			memcpy(ptr, &vp->vp_ipaddr, 4);
			break;

		default:
		case PW_TYPE_OCTETS:
		case PW_TYPE_STRING:
			memcpy(ptr, vp->vp_octets, vp->length);
			break;
		}
		ptr += vp->length;
	}

	return 0;
}
