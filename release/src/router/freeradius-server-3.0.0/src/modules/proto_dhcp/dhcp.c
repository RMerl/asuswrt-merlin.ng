/*
 * dhcp.c	Functions to send/receive dhcp packets.
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
 * Copyright 2008 The FreeRADIUS server project
 * Copyright 2008 Alan DeKok <aland@deployingradius.com>
 */

RCSID("$Id$")

#include <freeradius-devel/libradius.h>
#include <freeradius-devel/udpfromto.h>
#include <freeradius-devel/dhcp.h>

#ifndef __MINGW32__
#include <sys/ioctl.h>
#endif

#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifndef __MINGW32__
#include <net/if_arp.h>
#endif

#define DHCP_CHADDR_LEN	(16)
#define DHCP_SNAME_LEN	(64)
#define DHCP_FILE_LEN	(128)
#define DHCP_VEND_LEN	(308)
#define DHCP_OPTION_MAGIC_NUMBER (0x63825363)

#ifndef INADDR_BROADCAST
#define INADDR_BROADCAST INADDR_NONE
#endif

/* @todo: this is a hack */
#  define DEBUG			if (fr_debug_flag && fr_log_fp) fr_printf_log
void fr_strerror_printf(char const *fmt, ...);
#  define debug_pair(vp)	do { if (fr_debug_flag && fr_log_fp) { \
					vp_print(fr_log_fp, vp); \
				     } \
				} while(0)

typedef struct dhcp_packet_t {
	uint8_t		opcode;
	uint8_t		htype;
	uint8_t		hlen;
	uint8_t		hops;
	uint32_t	xid;	/* 4 */
	uint16_t	secs;	/* 8 */
	uint16_t	flags;
	uint32_t	ciaddr;	/* 12 */
	uint32_t	yiaddr;	/* 16 */
	uint32_t	siaddr;	/* 20 */
	uint32_t	giaddr;	/* 24 */
	uint8_t		chaddr[DHCP_CHADDR_LEN]; /* 28 */
	uint8_t		sname[DHCP_SNAME_LEN]; /* 44 */
	uint8_t		file[DHCP_FILE_LEN]; /* 108 */
	uint32_t	option_format; /* 236 */
	uint8_t		options[DHCP_VEND_LEN];
} dhcp_packet_t;

typedef struct dhcp_option_t {
	uint8_t		code;
	uint8_t		length;
} dhcp_option_t;

/*
 *	INADDR_ANY : 68 -> INADDR_BROADCAST : 67	DISCOVER
 *	INADDR_BROADCAST : 68 <- SERVER_IP : 67		OFFER
 *	INADDR_ANY : 68 -> INADDR_BROADCAST : 67	REQUEST
 *	INADDR_BROADCAST : 68 <- SERVER_IP : 67		ACK
 */
static char const *dhcp_header_names[] = {
	"DHCP-Opcode",
	"DHCP-Hardware-Type",
	"DHCP-Hardware-Address-Length",
	"DHCP-Hop-Count",
	"DHCP-Transaction-Id",
	"DHCP-Number-of-Seconds",
	"DHCP-Flags",
	"DHCP-Client-IP-Address",
	"DHCP-Your-IP-Address",
	"DHCP-Server-IP-Address",
	"DHCP-Gateway-IP-Address",
	"DHCP-Client-Hardware-Address",
	"DHCP-Server-Host-Name",
	"DHCP-Boot-Filename",

	NULL
};

static char const *dhcp_message_types[] = {
	"invalid",
	"DHCP-Discover",
	"DHCP-Offer",
	"DHCP-Request",
	"DHCP-Decline",
	"DHCP-Ack",
	"DHCP-NAK",
	"DHCP-Release",
	"DHCP-Inform",
	"DHCP-Force-Renew",
};

static int dhcp_header_sizes[] = {
	1, 1, 1, 1,
	4, 2, 2, 4,
	4, 4, 4,
	DHCP_CHADDR_LEN,
	DHCP_SNAME_LEN,
	DHCP_FILE_LEN
};


/*
 *	Some clients silently ignore responses less than 300 bytes.
 */
#define MIN_PACKET_SIZE (244)
#define DEFAULT_PACKET_SIZE (300)
#define MAX_PACKET_SIZE (1500 - 40)

#define DHCP_OPTION_FIELD (0)
#define DHCP_FILE_FIELD	  (1)
#define DHCP_SNAME_FIELD  (2)

static uint8_t *dhcp_get_option(dhcp_packet_t *packet, size_t packet_size,
				unsigned int option)
{
	int overload = 0;
	int field = DHCP_OPTION_FIELD;
	size_t where, size;
	uint8_t *data = packet->options;

	where = 0;
	size = packet_size - offsetof(dhcp_packet_t, options);
	data = &packet->options[where];

	while (where < size) {
		if (data[0] == 0) { /* padding */
			where++;
			continue;
		}

		if (data[0] == 255) { /* end of options */
			if ((field == DHCP_OPTION_FIELD) &&
			    (overload & DHCP_FILE_FIELD)) {
				data = packet->file;
				where = 0;
				size = sizeof(packet->file);
				field = DHCP_FILE_FIELD;
				continue;

			} else if ((field == DHCP_FILE_FIELD) &&
				   (overload & DHCP_SNAME_FIELD)) {
				data = packet->sname;
				where = 0;
				size = sizeof(packet->sname);
				field = DHCP_SNAME_FIELD;
				continue;
			}

			return NULL;
		}

		/*
		 *	We MUST have a real option here.
		 */
		if ((where + 2) > size) {
			fr_strerror_printf("Options overflow field at %u",
					   (unsigned int) (data - (uint8_t *) packet));
			return NULL;
		}

		if ((where + 2 + data[1]) > size) {
			fr_strerror_printf("Option length overflows field at %u",
					   (unsigned int) (data - (uint8_t *) packet));
			return NULL;
		}

		if (data[0] == option) return data;

		if (data[0] == 52) { /* overload sname and/or file */
			overload = data[3];
		}

		where += data[1] + 2;
		data += data[1] + 2;
	}

	return NULL;
}

/*
 *	DHCPv4 is only for IPv4.  Broadcast only works if udpfromto is
 *	defined.
 */
RADIUS_PACKET *fr_dhcp_recv(int sockfd)
{
	uint32_t		magic;
	struct sockaddr_storage	src;
	struct sockaddr_storage	dst;
	socklen_t		sizeof_src;
	socklen_t		sizeof_dst;
	RADIUS_PACKET		*packet;
	int port;
	uint8_t			*code;

	packet = rad_alloc(NULL, 0);
	if (!packet) {
		fr_strerror_printf("Failed allocating packet");
		return NULL;
	}

	packet->data = talloc_zero_array(packet, uint8_t, MAX_PACKET_SIZE);
	if (!packet->data) {
		fr_strerror_printf("Out of memory");
		rad_free(&packet);
		return NULL;
	}

	packet->sockfd = sockfd;
	sizeof_src = sizeof(src);
#ifdef WITH_UDPFROMTO
	sizeof_dst = sizeof(dst);
	packet->data_len = recvfromto(sockfd, packet->data, MAX_PACKET_SIZE, 0,
				      (struct sockaddr *)&src, &sizeof_src,
				      (struct sockaddr *)&dst, &sizeof_dst);
#else
	packet->data_len = recvfrom(sockfd, packet->data, MAX_PACKET_SIZE, 0,
				    (struct sockaddr *)&src, &sizeof_src);
#endif

	if (packet->data_len <= 0) {
		fr_strerror_printf("Failed reading DHCP socket: %s", strerror(errno));
		rad_free(&packet);
		return NULL;
	}

	if (packet->data_len < MIN_PACKET_SIZE) {
		fr_strerror_printf("DHCP packet is too small (%zu < %d)",
		      		   packet->data_len, MIN_PACKET_SIZE);
		rad_free(&packet);
		return NULL;
	}

	if (packet->data[1] != 1) {
		fr_strerror_printf("DHCP can only receive ethernet requests, not type %02x",
		      packet->data[1]);
		rad_free(&packet);
		return NULL;
	}

	if (packet->data[2] != 6) {
		fr_strerror_printf("Ethernet HW length is wrong length %d",
			packet->data[2]);
		rad_free(&packet);
		return NULL;
	}

	memcpy(&magic, packet->data + 236, 4);
	magic = ntohl(magic);
	if (magic != DHCP_OPTION_MAGIC_NUMBER) {
		fr_strerror_printf("Cannot do BOOTP");
		rad_free(&packet);
		return NULL;
	}

	/*
	 *	Create unique keys for the packet.
	 */
	memcpy(&magic, packet->data + 4, 4);
	packet->id = ntohl(magic);

	code = dhcp_get_option((dhcp_packet_t *) packet->data,
			       packet->data_len, 53);
	if (!code) {
		fr_strerror_printf("No message-type option was found in the packet");
		rad_free(&packet);
		return NULL;
	}

	if ((code[1] < 1) || (code[2] == 0) || (code[2] > 8)) {
		fr_strerror_printf("Unknown value for message-type option");
		rad_free(&packet);
		return NULL;
	}

	packet->code = code[2] | PW_DHCP_OFFSET;

	/*
	 *	Create a unique vector from the MAC address and the
	 *	DHCP opcode.  This is a hack for the RADIUS
	 *	infrastructure in the rest of the server.
	 *
	 *	Note: packet->data[2] == 6, which is smaller than
	 *	sizeof(packet->vector)
	 *
	 *	FIXME:  Look for client-identifier in packet,
	 *      and use that, too?
	 */
	memset(packet->vector, 0, sizeof(packet->vector));
	memcpy(packet->vector, packet->data + 28, packet->data[2]);
	packet->vector[packet->data[2]] = packet->code & 0xff;

	/*
	 *	FIXME: for DISCOVER / REQUEST: src_port == dst_port + 1
	 *	FIXME: for OFFER / ACK       : src_port = dst_port - 1
	 */

	/*
	 *	Unique keys are xid, client mac, and client ID?
	 */

	/*
	 *	FIXME: More checks, like DHCP packet type?
	 */

	sizeof_dst = sizeof(dst);

#ifndef WITH_UDPFROMTO
	/*
	 *	This should never fail...
	 */
	if (getsockname(sockfd, (struct sockaddr *) &dst, &sizeof_dst) < 0) {
		fr_strerror_printf("getsockname failed: %s", strerror(errno));
		rad_free(&packet);
		return NULL;
	}
#endif

	fr_sockaddr2ipaddr(&dst, sizeof_dst, &packet->dst_ipaddr, &port);
	packet->dst_port = port;

	fr_sockaddr2ipaddr(&src, sizeof_src, &packet->src_ipaddr, &port);
	packet->src_port = port;

	if (fr_debug_flag > 1) {
		char type_buf[64];
		char const *name = type_buf;
		char src_ip_buf[256], dst_ip_buf[256];

		if ((packet->code >= PW_DHCP_DISCOVER) &&
		    (packet->code <= PW_DHCP_INFORM)) {
			name = dhcp_message_types[packet->code - PW_DHCP_OFFSET];
		} else {
			snprintf(type_buf, sizeof(type_buf), "%d",
				 packet->code - PW_DHCP_OFFSET);
		}

		DEBUG("Received %s of id %08x from %s:%d to %s:%d\n",
		       name, (unsigned int) packet->id,
		       inet_ntop(packet->src_ipaddr.af,
				 &packet->src_ipaddr.ipaddr,
				 src_ip_buf, sizeof(src_ip_buf)),
		       packet->src_port,
		       inet_ntop(packet->dst_ipaddr.af,
				 &packet->dst_ipaddr.ipaddr,
				 dst_ip_buf, sizeof(dst_ip_buf)),
		       packet->dst_port);
	}

	return packet;
}


/*
 *	Send a DHCP packet.
 */
int fr_dhcp_send(RADIUS_PACKET *packet)
{
	struct sockaddr_storage	dst;
	socklen_t		sizeof_dst;
#ifdef WITH_UDPFROMTO
	struct sockaddr_storage	src;
	socklen_t		sizeof_src;

	fr_ipaddr2sockaddr(&packet->src_ipaddr, packet->src_port,
	    &src, &sizeof_src);
#endif

	fr_ipaddr2sockaddr(&packet->dst_ipaddr, packet->dst_port,
			   &dst, &sizeof_dst);

	if (fr_debug_flag > 1) {
		char type_buf[64];
		char const *name = type_buf;
#ifdef WITH_UDPFROMTO
		char src_ip_buf[INET6_ADDRSTRLEN];
#endif
		char dst_ip_buf[INET6_ADDRSTRLEN];

		if ((packet->code >= PW_DHCP_DISCOVER) &&
		    (packet->code <= PW_DHCP_INFORM)) {
			name = dhcp_message_types[packet->code - PW_DHCP_OFFSET];
		} else {
			snprintf(type_buf, sizeof(type_buf), "%d",
			    packet->code - PW_DHCP_OFFSET);
		}

		DEBUG(
#ifdef WITH_UDPFROMTO
		"Sending %s of id %08x from %s:%d to %s:%d\n",
#else
		"Sending %s of id %08x to %s:%d\n",
#endif
		   name, (unsigned int) packet->id,
#ifdef WITH_UDPFROMTO
		   inet_ntop(packet->src_ipaddr.af, &packet->src_ipaddr.ipaddr, src_ip_buf, sizeof(src_ip_buf)),
		   packet->src_port,
#endif
		   inet_ntop(packet->dst_ipaddr.af, &packet->dst_ipaddr.ipaddr, dst_ip_buf, sizeof(dst_ip_buf)),
		   packet->dst_port);
	}

#ifndef WITH_UDPFROMTO
	/*
	 *	Assume that the packet is encoded before sending it.
	 */
	return sendto(packet->sockfd, packet->data, packet->data_len, 0,
		      (struct sockaddr *)&dst, sizeof_dst);
#else

	return sendfromto(packet->sockfd, packet->data, packet->data_len, 0,
			  (struct sockaddr *)&src, sizeof_src,
			  (struct sockaddr *)&dst, sizeof_dst);
#endif
}

static int fr_dhcp_attr2vp(RADIUS_PACKET *packet, VALUE_PAIR *vp, uint8_t const *p, size_t alen);

static int decode_tlv(RADIUS_PACKET *packet, VALUE_PAIR *tlv, uint8_t const *data, size_t data_len)
{
	uint8_t const *p;
	VALUE_PAIR *head, *vp;
	vp_cursor_t cursor;

	/*
	 *	Take a pass at parsing it.
	 */
	p = data;
	while (p < (data + data_len)) {
		if ((p + 2) > (data + data_len)) goto make_tlv;

		if ((p + p[1] + 2) > (data + data_len)) goto make_tlv;
		p += 2 + p[1];
	}

	/*
	 *	Got here... must be well formed.
	 */
	head = NULL;
	paircursor(&cursor, &head);

	p = data;
	while (p < (data + data_len)) {
		vp = paircreate(packet, tlv->da->attr | (p[0] << 8), DHCP_MAGIC_VENDOR);
		if (!vp) {
			pairfree(&head);
			goto make_tlv;
		}

		if (fr_dhcp_attr2vp(packet, vp, p + 2, p[1]) < 0) {
			pairfree(&head);
			goto make_tlv;
		}

		pairinsert(&cursor, vp);
		p += 2 + p[1];
	}

	/*
	 *	The caller allocated TLV, so we need to copy the FIRST
	 *	attribute over top of that.
	 */
	if (head) {
		memcpy(tlv, head, sizeof(*tlv));
		head->next = NULL;
		pairfree(&head);
	}

	return 0;

make_tlv:
	tlv->vp_tlv = talloc_array(tlv, uint8_t, data_len);
	if (!tlv->vp_tlv) {
		fr_strerror_printf("No memory");
		return -1;
	}
	memcpy(tlv->vp_tlv, data, data_len);
	tlv->length = data_len;

	return 0;
}


/*
 *	Decode ONE value into a VP
 */
static int fr_dhcp_attr2vp(RADIUS_PACKET *packet, VALUE_PAIR *vp, uint8_t const *p, size_t alen)
{
	char *q;

	switch (vp->da->type) {
	case PW_TYPE_BYTE:
		if (alen != 1) goto raw;
		vp->vp_integer = p[0];
		break;

	case PW_TYPE_SHORT:
		if (alen != 2) goto raw;
		memcpy(&vp->vp_integer, p, 2);
		vp->vp_integer = ntohs(vp->vp_integer);
		break;

	case PW_TYPE_INTEGER:
		if (alen != 4) goto raw;
		memcpy(&vp->vp_integer, p, 4);
		vp->vp_integer = ntohl(vp->vp_integer);
		break;

	case PW_TYPE_IPADDR:
		if (alen != 4) goto raw;
		/*
		 *	Keep value in Network Order!
		 */
		memcpy(&vp->vp_ipaddr, p , 4);
		vp->length = 4;
		break;

	case PW_TYPE_STRING:
		vp->vp_strvalue = q = talloc_array(vp, char, alen + 1);
		vp->type = VT_DATA;
		memcpy(q, p , alen);
		q[alen] = '\0';
		break;

	case PW_TYPE_ETHERNET:
		memcpy(vp->vp_ether, p, sizeof(vp->vp_ether));
		vp->length = sizeof(vp->vp_ether);
		break;

	/*
	 *	Value doesn't match up with attribute type, overwrite the
	 *	vp's original DICT_ATTR with an unknown one.
	 */
	raw:
		if (pair2unknown(vp) < 0) return -1;

	case PW_TYPE_OCTETS:
		if (alen > 253) return -1;
		pairmemcpy(vp, p, alen);
		break;

	case PW_TYPE_TLV:
		return decode_tlv(packet, vp, p, alen);

	default:
		fr_strerror_printf("Internal sanity check %d %d", vp->da->type, __LINE__);
		return -1;
	} /* switch over type */

	vp->length = alen;
	return 0;
}

ssize_t fr_dhcp_decode_options(RADIUS_PACKET *packet,
			       uint8_t const *data, size_t len, VALUE_PAIR **head)
{
	int i;
	VALUE_PAIR *vp;
	vp_cursor_t cursor;
	uint8_t const *p, *next;
	next = data;

	*head = NULL;
	paircursor(&cursor, head);

	/*
	 *	FIXME: This should also check sname && file fields.
	 *	See the dhcp_get_option() function above.
	 */
	while (next < (data + len)) {
		int num_entries, alen;
		DICT_ATTR const *da;

		p = next;

		if (*p == 0) break;
		if (*p == 255) break; /* end of options signifier */
		if ((p + 2) > (data + len)) break;

		next = p + 2 + p[1];

		if (p[1] >= 253) {
			fr_strerror_printf("Attribute too long %u %u",
					   p[0], p[1]);
			continue;
		}

		da = dict_attrbyvalue(p[0], DHCP_MAGIC_VENDOR);
		if (!da) {
			fr_strerror_printf("Attribute not in our dictionary: %u",
					   p[0]);
			continue;
		}

		vp = NULL;
		num_entries = 1;
		alen = p[1];
		p += 2;

		/*
		 *	Could be an array of bytes, integers, etc.
		 */
		if (da->flags.array) {
			switch (da->type) {
			case PW_TYPE_BYTE:
				num_entries = alen;
				alen = 1;
				break;

			case PW_TYPE_SHORT: /* ignore any trailing data */
				num_entries = alen >> 1;
				alen = 2;
				break;

			case PW_TYPE_IPADDR:
			case PW_TYPE_INTEGER:
			case PW_TYPE_DATE: /* ignore any trailing data */
				num_entries = alen >> 2;
				alen = 4;
				break;

			default:

				break; /* really an internal sanity failure */
			}
		}

		/*
		 *	Loop over all of the entries, building VPs
		 */
		for (i = 0; i < num_entries; i++) {
			vp = pairmake(packet, NULL, da->name, NULL, T_OP_ADD);
			if (!vp) {
				fr_strerror_printf("Cannot build attribute %s",
					fr_strerror());
				pairfree(head);
				return -1;
			}

			/*
			 *	Hack for ease of use.
			 */
			if ((da->vendor == DHCP_MAGIC_VENDOR) &&
			    (da->attr == 61) && !da->flags.array &&
			    (alen == 7) && (*p == 1) && (num_entries == 1)) {
				pairmemcpy(vp, p + 1, 6);

			} else if (fr_dhcp_attr2vp(packet, vp, p, alen) < 0) {
				pairfree(&vp);
				pairfree(head);
				return -1;
			}

			pairinsert(&cursor, vp);

			for (vp = paircurrent(&cursor);
			     vp;
			     vp = pairnext(&cursor)) {
				debug_pair(vp);
			}
			p += alen;
		} /* loop over array entries */
	} /* loop over the entire packet */

	return next - data;
}

int fr_dhcp_decode(RADIUS_PACKET *packet)
{
	size_t i;
	uint8_t *p;
	uint32_t giaddr;
	vp_cursor_t cursor;
	VALUE_PAIR *head = NULL, *vp;
	VALUE_PAIR *maxms, *mtu;

	paircursor(&cursor, &head);
	p = packet->data;

	if ((fr_debug_flag > 2) && fr_log_fp) {
		for (i = 0; i < packet->data_len; i++) {
			if ((i & 0x0f) == 0x00) fprintf(fr_log_fp, "%d: ", (int) i);
			fprintf(fr_log_fp, "%02x ", packet->data[i]);
			if ((i & 0x0f) == 0x0f) fprintf(fr_log_fp, "\n");
		}
		fprintf(fr_log_fp, "\n");
	}

	if (packet->data[1] != 1) {
		fr_strerror_printf("Packet is not Ethernet: %u",
		      packet->data[1]);
		return -1;
	}

	/*
	 *	Decode the header.
	 */
	for (i = 0; i < 14; i++) {
		char *q;

		vp = pairmake(packet, NULL, dhcp_header_names[i], NULL, T_OP_EQ);
		if (!vp) {
			char buffer[256];
			strlcpy(buffer, fr_strerror(), sizeof(buffer));
			fr_strerror_printf("Cannot decode packet due to internal error: %s", buffer);
			pairfree(&head);
			return -1;
		}

		if ((i == 11) &&
		    (packet->data[1] == 1) &&
		    (packet->data[2] != 6)) {
		    	fr_strerror_printf("chaddr of incorrect length for ethernet");
		}

		switch (vp->da->type) {
		case PW_TYPE_BYTE:
			vp->vp_integer = p[0];
			vp->length = 1;
			break;

		case PW_TYPE_SHORT:
			vp->vp_integer = (p[0] << 8) | p[1];
			vp->length = 2;
			break;

		case PW_TYPE_INTEGER:
			memcpy(&vp->vp_integer, p, 4);
			vp->vp_integer = ntohl(vp->vp_integer);
			vp->length = 4;
			break;

		case PW_TYPE_IPADDR:
			memcpy(&vp->vp_ipaddr, p, 4);
			vp->length = 4;
			break;

		case PW_TYPE_STRING:
			vp->vp_strvalue = q = talloc_array(vp, char, dhcp_header_sizes[i] + 1);
			vp->type = VT_DATA;
			memcpy(q, p, dhcp_header_sizes[i]);
			q[dhcp_header_sizes[i]] = '\0';
			vp->length = strlen(vp->vp_strvalue);
			if (vp->length == 0) {
				pairfree(&vp);
			}
			break;

		case PW_TYPE_OCTETS:
			pairmemcpy(vp, p, packet->data[2]);
			break;

		case PW_TYPE_ETHERNET:
			memcpy(vp->vp_ether, p, sizeof(vp->vp_ether));
			vp->length = sizeof(vp->vp_ether);
			break;

		default:
			fr_strerror_printf("BAD TYPE %d", vp->da->type);
			pairfree(&vp);
			break;
		}
		p += dhcp_header_sizes[i];

		if (!vp) continue;

		debug_pair(vp);
		pairinsert(&cursor, vp);
	}

	/*
	 *	Loop over the options.
	 */

	/*
	 * 	Nothing uses tail after this call, if it does in the future
	 *	it'll need to find the new tail...
	 */
	{
		VALUE_PAIR *options = NULL;

		if (fr_dhcp_decode_options(packet,
					   packet->data + 240, packet->data_len - 240,
					   &options) < 0) {
			return -1;
		}

		if (options) {
			pairinsert(&cursor, options);
		}
	}

	/*
	 *	If DHCP request, set ciaddr to zero.
	 */

	/*
	 *	Set broadcast flag for broken vendors, but only if
	 *	giaddr isn't set.
	 */
	memcpy(&giaddr, packet->data + 24, sizeof(giaddr));
	if (giaddr == htonl(INADDR_ANY)) {
		/*
		 *	DHCP Opcode is request
		 */
		vp = pairfind(head, 256, DHCP_MAGIC_VENDOR, TAG_ANY);
		if (vp && vp->vp_integer == 3) {
			/*
			 *	Vendor is "MSFT 98"
			 */
			vp = pairfind(head, 63, DHCP_MAGIC_VENDOR, TAG_ANY);
			if (vp && (strcmp(vp->vp_strvalue, "MSFT 98") == 0)) {
				vp = pairfind(head, 262, DHCP_MAGIC_VENDOR, TAG_ANY);

				/*
				 *	Reply should be broadcast.
				 */
				if (vp) vp->vp_integer |= 0x8000;
				packet->data[10] |= 0x80;
			}
		}
	}

	/*
	 *	FIXME: Nuke attributes that aren't used in the normal
	 *	header for discover/requests.
	 */
	packet->vps = head;

	/*
	 *	Client can request a LARGER size, but not a smaller
	 *	one.  They also cannot request a size larger than MTU.
	 */
	maxms = pairfind(packet->vps, 57, DHCP_MAGIC_VENDOR, TAG_ANY);
	mtu = pairfind(packet->vps, 26, DHCP_MAGIC_VENDOR, TAG_ANY);

	if (mtu && (mtu->vp_integer < DEFAULT_PACKET_SIZE)) {
		fr_strerror_printf("DHCP Fatal: Client says MTU is smaller than minimum permitted by the specification.");
		return -1;
	}

	if (maxms && (maxms->vp_integer < DEFAULT_PACKET_SIZE)) {
		fr_strerror_printf("DHCP WARNING: Client says maximum message size is smaller than minimum permitted by the specification: fixing it");
		maxms->vp_integer = DEFAULT_PACKET_SIZE;
	}

	if (maxms && mtu && (maxms->vp_integer > mtu->vp_integer)) {
		fr_strerror_printf("DHCP WARNING: Client says MTU is smaller than maximum message size: fixing it");
		maxms->vp_integer = mtu->vp_integer;
	}

	if (fr_debug_flag) fflush(stdout);

	return 0;
}


static int attr_cmp(void const *one, void const *two)
{
	VALUE_PAIR const * const *a = one;
	VALUE_PAIR const * const *b = two;

	/*
	 *	DHCP-Message-Type is first, for simplicity.
	 */
	if (((*a)->da->attr == 53) &&
	    (*b)->da->attr != 53) return -1;

	/*
	 *	Relay-Agent is last
	 */
	if (((*a)->da->attr == 82) &&
	    (*b)->da->attr != 82) return 1;

	return ((*a)->da->attr - (*b)->da->attr);
}

/*
 * @todo Check room!
 */
static size_t fr_dhcp_vp2attr(VALUE_PAIR *vp, uint8_t *p, UNUSED size_t room)
{
	size_t length;
	uint32_t lvalue;

	/*
	 *	Search for all attributes of the same
	 *	type, and pack them into the same
	 *	attribute.
	 */
	switch (vp->da->type) {
	case PW_TYPE_BYTE:
		length = 1;
		*p = vp->vp_integer & 0xff;
		break;

	case PW_TYPE_SHORT:
		length = 2;
		p[0] = (vp->vp_integer >> 8) & 0xff;
		p[1] = vp->vp_integer & 0xff;
		break;

	case PW_TYPE_INTEGER:
		length = 4;
		lvalue = htonl(vp->vp_integer);
		memcpy(p, &lvalue, 4);
		break;

	case PW_TYPE_IPADDR:
		length = 4;
		memcpy(p, &vp->vp_ipaddr, 4);
		break;

	case PW_TYPE_ETHERNET:
		length = 6;
		memcpy(p, &vp->vp_ether, 6);
		break;

	case PW_TYPE_STRING:
		memcpy(p, vp->vp_strvalue, vp->length);
		length = vp->length;
		break;

	case PW_TYPE_TLV:	/* FIXME: split it on 255? */
		memcpy(p, vp->vp_tlv, vp->length);
		length = vp->length;
		break;

	case PW_TYPE_OCTETS:
		memcpy(p, vp->vp_octets, vp->length);
		length = vp->length;
		break;

	default:
		fr_strerror_printf("BAD TYPE2 %d", vp->da->type);
		length = 0;
		break;
	}

	return length;
}

static VALUE_PAIR *fr_dhcp_vp2suboption(RADIUS_PACKET *packet, VALUE_PAIR *vps)
{
	int length;
	unsigned int attribute;
	uint8_t *ptr;
	vp_cursor_t cursor;
	VALUE_PAIR *vp, *tlv;

	attribute = vps->da->attr & 0xffff00ff;

	tlv = paircreate(packet, attribute, DHCP_MAGIC_VENDOR);
	if (!tlv) return NULL;

	tlv->length = 0;
	for (vp = paircursor(&cursor, &vps);
	     vp;
	     vp = pairnext(&cursor)) {
		/*
		 *	Group the attributes ONLY until we see a
		 *	non-TLV attribute.
		 */
		if (!vp->da->flags.is_tlv ||
		    vp->da->flags.extended ||
		    ((vp->da->attr & 0xffff00ff) != attribute)) {
			break;
		}

		tlv->length += vp->length + 2;
	}

	if (!tlv->length) {
		pairfree(&tlv);
		return NULL;
	}

	tlv->vp_tlv = talloc_array(tlv, uint8_t, tlv->length);
	if (!tlv->vp_tlv) {
		pairfree(&tlv);
		return NULL;
	}

	ptr = tlv->vp_tlv;
	for (vp = paircursor(&cursor, &vps);
	     vp;
	     vp = pairnext(&cursor)) {
		if (!vp->da->flags.is_tlv ||
		    vp->da->flags.extended ||
		    ((vp->da->attr & 0xffff00ff) != attribute)) {
			break;
		}

		length = fr_dhcp_vp2attr(vp, ptr + 2,
					 tlv->vp_tlv + tlv->length - ptr);
		if (length > 255) {
			pairfree(&tlv);
			return NULL;
		}

		/*
		 *	Pack the attribute.
		 */
		ptr[0] = (vp->da->attr & 0xff00) >> 8;
		ptr[1] = length;

		ptr += length + 2;
	}

	return tlv;
}


int fr_dhcp_encode(RADIUS_PACKET *packet)
{
	unsigned int i, num_vps;
	uint8_t *p;
	vp_cursor_t cursor;
	VALUE_PAIR *vp;
	uint32_t lvalue, mms;
	size_t dhcp_size, length;
#ifndef NDEBUG
	char const *name;
#  ifdef WITH_UDPFROMTO
	char src_ip_buf[256];
#  endif
	char dst_ip_buf[256];
#endif

	if (packet->data) return 0;

	packet->data_len = MAX_PACKET_SIZE;
	packet->data = talloc_zero_array(packet, uint8_t, packet->data_len);

	/* XXX Ugly ... should be set by the caller */
	if (packet->code == 0) packet->code = PW_DHCP_NAK;

#ifndef NDEBUG
	if ((packet->code >= PW_DHCP_DISCOVER) &&
	    (packet->code <= PW_DHCP_INFORM)) {
		name = dhcp_message_types[packet->code - PW_DHCP_OFFSET];
	} else {
		name = "?Unknown?";
	}

	DEBUG(
#ifdef WITH_UDPFROMTO
	      "Encoding %s of id %08x from %s:%d to %s:%d\n",
#else
	      "Encoding %s of id %08x to %s:%d\n",
#endif
	      name, (unsigned int) packet->id,
#ifdef WITH_UDPFROMTO
	      inet_ntop(packet->src_ipaddr.af,
			&packet->src_ipaddr.ipaddr,
			src_ip_buf, sizeof(src_ip_buf)),
	      packet->src_port,
#endif
	      inet_ntop(packet->dst_ipaddr.af,
			&packet->dst_ipaddr.ipaddr,
		     dst_ip_buf, sizeof(dst_ip_buf)),
	      packet->dst_port);
#endif

	p = packet->data;

	mms = DEFAULT_PACKET_SIZE; /* maximum message size */

	/*
	 *	Clients can request a LARGER size, but not a
	 *	smaller one.  They also cannot request a size
	 *	larger than MTU.
	 */

	/* DHCP-DHCP-Maximum-Msg-Size */
	vp = pairfind(packet->vps, 57, DHCP_MAGIC_VENDOR, TAG_ANY);
	if (vp && (vp->vp_integer > mms)) {
		mms = vp->vp_integer;

		if (mms > MAX_PACKET_SIZE) mms = MAX_PACKET_SIZE;
	}

	vp = pairfind(packet->vps, 256, DHCP_MAGIC_VENDOR, TAG_ANY);
	if (vp) {
		*p++ = vp->vp_integer & 0xff;
	} else {
		*p++ = 1;	/* client message */
	}

	/* DHCP-Hardware-Type */
	if ((vp = pairfind(packet->vps, 257, DHCP_MAGIC_VENDOR, TAG_ANY))) {
		*p++ = vp->vp_integer & 0xFF;
	} else {
		*p++ = 1;		/* hardware type = ethernet */
	}

	/* DHCP-Hardware-Address-Length */
	if ((vp = pairfind(packet->vps, 258, DHCP_MAGIC_VENDOR, TAG_ANY))) {
		*p++ = vp->vp_integer & 0xFF;
	} else {
		*p++ = 6;		/* 6 bytes of ethernet */
	}

	/* DHCP-Hop-Count */
	if ((vp = pairfind(packet->vps, 259, DHCP_MAGIC_VENDOR, TAG_ANY))) {
		*p = vp->vp_integer & 0xff;
	}
	p++;

	/* DHCP-Transaction-Id */
	if ((vp = pairfind(packet->vps, 260, DHCP_MAGIC_VENDOR, TAG_ANY))) {
		lvalue = htonl(vp->vp_integer);
	} else {
		lvalue = fr_rand();
	}
	memcpy(p, &lvalue, 4);
	p += 4;

	/* DHCP-Number-of-Seconds */
	if ((vp = pairfind(packet->vps, 261, DHCP_MAGIC_VENDOR, TAG_ANY))) {
		lvalue = htonl(vp->vp_integer);
		memcpy(p, &lvalue, 2);
	}
	p += 2;

	/* DHCP-Flags */
	if ((vp = pairfind(packet->vps, 262, DHCP_MAGIC_VENDOR, TAG_ANY))) {
		lvalue = htons(vp->vp_integer);
		memcpy(p, &lvalue, 2);
	}
	p += 2;

	/* DHCP-Client-IP-Address */
	if ((vp = pairfind(packet->vps, 263, DHCP_MAGIC_VENDOR, TAG_ANY))) {
		memcpy(p, &vp->vp_ipaddr, 4);
	}
	p += 4;

	/* DHCP-Your-IP-address */
	if ((vp = pairfind(packet->vps, 264, DHCP_MAGIC_VENDOR, TAG_ANY))) {
		lvalue = vp->vp_ipaddr;
	} else {
		lvalue = htonl(INADDR_ANY);
	}
	memcpy(p, &lvalue, 4);
	p += 4;

	/* DHCP-Server-IP-Address */
	vp = pairfind(packet->vps, 265, DHCP_MAGIC_VENDOR, TAG_ANY);

	/* DHCP-DHCP-Server-Identifier */
	if (!vp && (vp = pairfind(packet->vps, 54, DHCP_MAGIC_VENDOR, TAG_ANY))) {
		lvalue = vp->vp_ipaddr;
	} else {
		lvalue = htonl(INADDR_ANY);
	}
	memcpy(p, &lvalue, 4);
	p += 4;

	/*
	 *	DHCP-Gateway-IP-Address
	 */
	if ((vp = pairfind(packet->vps, 266, DHCP_MAGIC_VENDOR, TAG_ANY))) {
		lvalue = vp->vp_ipaddr;
	} else {
		lvalue = htonl(INADDR_ANY);
	}
	memcpy(p, &lvalue, 4);
	p += 4;

	/* DHCP-Client-Hardware-Address */
	if ((vp = pairfind(packet->vps, 267, DHCP_MAGIC_VENDOR, TAG_ANY))) {
		if (vp->length > DHCP_CHADDR_LEN) {
			memcpy(p, vp->vp_octets, DHCP_CHADDR_LEN);
		} else {
			memcpy(p, vp->vp_octets, vp->length);
		}
	}
	p += DHCP_CHADDR_LEN;

	/* DHCP-Server-Host-Name */
	if ((vp = pairfind(packet->vps, 268, DHCP_MAGIC_VENDOR, TAG_ANY))) {
		if (vp->length > DHCP_SNAME_LEN) {
			memcpy(p, vp->vp_strvalue, DHCP_SNAME_LEN);
		} else {
			memcpy(p, vp->vp_strvalue, vp->length);
		}
	}
	p += DHCP_SNAME_LEN;

	/*
	 *	Copy over DHCP-Boot-Filename.
	 *
	 *	FIXME: This copy should be delayed until AFTER the options
	 *	have been processed.  If there are too many options for
	 *	the packet, then they go into the sname && filename fields.
	 *	When that happens, the boot filename is passed as an option,
	 *	instead of being placed verbatim in the filename field.
	 */

	/* DHCP-Boot-Filename */
	if ((vp = pairfind(packet->vps, 269, DHCP_MAGIC_VENDOR, TAG_ANY))) {
		if (vp->length > DHCP_FILE_LEN) {
			memcpy(p, vp->vp_strvalue, DHCP_FILE_LEN);
		} else {
			memcpy(p, vp->vp_strvalue, vp->length);
		}
	}
	p += DHCP_FILE_LEN;

	/* DHCP magic number */
	lvalue = htonl(DHCP_OPTION_MAGIC_NUMBER);
	memcpy(p, &lvalue, 4);
	p += 4;

	/*
	 *	Print the header.
	 */
	if (fr_debug_flag > 1) {
		uint8_t *pp = p;

		p = packet->data;

		for (i = 0; i < 14; i++) {
			char *q;

			vp = pairmake(packet, NULL,
				      dhcp_header_names[i], NULL, T_OP_EQ);
			if (!vp) {
				char buffer[256];
				strlcpy(buffer, fr_strerror(), sizeof(buffer));
				fr_strerror_printf("Cannot decode packet due to internal error: %s", buffer);
				return -1;
			}

			switch (vp->da->type) {
			case PW_TYPE_BYTE:
				vp->vp_integer = p[0];
				vp->length = 1;
				break;

			case PW_TYPE_SHORT:
				vp->vp_integer = (p[0] << 8) | p[1];
				vp->length = 2;
				break;

			case PW_TYPE_INTEGER:
				memcpy(&vp->vp_integer, p, 4);
				vp->vp_integer = ntohl(vp->vp_integer);
				vp->length = 4;
				break;

			case PW_TYPE_IPADDR:
				memcpy(&vp->vp_ipaddr, p, 4);
				vp->length = 4;
				break;

			case PW_TYPE_STRING:
				vp->vp_strvalue = q = talloc_array(vp, char, dhcp_header_sizes[i]);
				vp->type = VT_DATA;
				memcpy(q, p, dhcp_header_sizes[i]);
				q[dhcp_header_sizes[i]] = '\0';
				vp->length = strlen(vp->vp_strvalue);
				break;

			case PW_TYPE_OCTETS: /* only for Client HW Address */
				pairmemcpy(vp, p, packet->data[2]);
				break;

			case PW_TYPE_ETHERNET: /* only for Client HW Address */
				memcpy(vp->vp_ether, p, sizeof(vp->vp_ether));
				vp->length = sizeof(vp->vp_ether);
				break;

			default:
				fr_strerror_printf("Internal sanity check failed %d %d", vp->da->type, __LINE__);
				pairfree(&vp);
				break;
			}

			p += dhcp_header_sizes[i];

			debug_pair(vp);
			pairfree(&vp);
		}

		/*
		 *	Jump over DHCP magic number, response, etc.
		 */
		p = pp;
	}

	/*
	 *	Before packing the attributes, re-order them so that
	 *	the array ones are all contiguous.  This simplifies
	 *	the later code.
	 */
	num_vps = 0;
	for (vp = paircursor(&cursor, &packet->vps);
	     vp;
	     vp = pairnext(&cursor)) {
		num_vps++;
	}
	if (num_vps > 1) {
		VALUE_PAIR **array;

		array = talloc_array(packet, VALUE_PAIR*, num_vps);

		i = 0;
		for (vp = paircursor(&cursor, &packet->vps);
		     vp;
		     vp = pairnext(&cursor)) {
			array[i++] = vp;
		}

		/*
		 *	Sort the attributes.
		 */
		qsort(array, (size_t) num_vps, sizeof(VALUE_PAIR *), attr_cmp);

		paircursor(&cursor, &packet->vps);
		for (i = 0; i < num_vps; i++) {
			pairinsert(&cursor, array[i]->next);
		}
		talloc_free(array);
	}

	p[0] = 0x35;		/* DHCP-Message-Type */
	p[1] = 1;
	p[2] = packet->code - PW_DHCP_OFFSET;
	p += 3;

	/*
	 *	Pack in the attributes.
	 */
	vp = packet->vps;
	while (vp) {
		unsigned int num_entries = 1;
		VALUE_PAIR *same;
		uint8_t *plength;

		if (vp->da->vendor != DHCP_MAGIC_VENDOR) goto next;
		if (vp->da->attr == 53) goto next; /* already done */
		if ((vp->da->attr > 255) &&
		    (DHCP_BASE_ATTR(vp->da->attr) != PW_DHCP_OPTION_82)) goto next;

		debug_pair(vp);
		if (vp->da->flags.extended) goto next;

		length = vp->length;

		for (same = paircursor(&cursor, &vp->next);
		     same;
		     same = pairnext(&cursor)) {
			if (same->da->attr != vp->da->attr) break;
			num_entries++;
		}

		/*
		 *	For client-identifier
		 * @todo What's this meant to be doing?!
		 */
#if 0
		if ((vp->da->type == PW_TYPE_ETHERNET) &&
		    (vp->length == 6) &&
		    (num_entries == 1)) {
			vp->da->type = PW_TYPE_OCTETS;
			memmove(vp->vp_octets + 1, vp->vp_octets, 6);
			vp->vp_octets[0] = 1;
		}
#endif
		*(p++) = vp->da->attr & 0xff;
		plength = p;
		*(p++) = 0;	/* header isn't included in attr length */

		for (i = 0; i < num_entries; i++) {
			if (i != 0) debug_pair(vp);

			if (vp->da->flags.is_tlv) {
				VALUE_PAIR *tlv;

				/*
				 *	Should NOT have been encoded yet!
				 */
				tlv = fr_dhcp_vp2suboption(packet, vp);

				/*
				 *	Ignore it if there's an issue
				 *	encoding it.
				 */
				if (!tlv) goto next;

				tlv->next = vp->next;
				vp->next = tlv;
				vp = tlv;
			}

			length = fr_dhcp_vp2attr(vp, p, 0);

			/*
			 *	This will never happen due to FreeRADIUS
			 *	limitations: sizeof(vp->vp_octets) < 255
			 */
			if (length > 255) {
				fr_strerror_printf("WARNING Ignoring too long attribute %s!", vp->da->name);
				break;
			}

			/*
			 *	More than one attribute of the same type
			 *	in a row: they are packed together
			 *	into the same TLV.  If we overflow,
			 *	go bananas!
			 */
			if ((*plength + length) > 255) {
				fr_strerror_printf("WARNING Ignoring too long attribute %s!", vp->da->name);
				break;
			}

			*plength += length;
			p += length;

			if (vp->next &&
			    (vp->next->da->attr == vp->da->attr))
				vp = vp->next;
		} /* loop over num_entries */

	next:
		vp = vp->next;
	}

	p[0] = 0xff;		/* end of option option */
	p[1] = 0x00;
	p += 2;
	dhcp_size = p - packet->data;

	/*
	 *	FIXME: if (dhcp_size > mms),
	 *	  then we put the extra options into the "sname" and "file"
	 *	  fields, AND set the "end option option" in the "options"
	 *	  field.  We also set the "overload option",
	 *	  and put options into the "file" field, followed by
	 *	  the "sname" field.  Where each option is completely
	 *	  enclosed in the "file" and/or "sname" field, AND
	 *	  followed by the "end of option", and MUST be followed
	 *	  by padding option.
	 *
	 *	Yuck.  That sucks...
	 */
	packet->data_len = dhcp_size;

	if (packet->data_len < DEFAULT_PACKET_SIZE) {
		memset(packet->data + packet->data_len, 0,
		       DEFAULT_PACKET_SIZE - packet->data_len);
		packet->data_len = DEFAULT_PACKET_SIZE;
	}

	if ((fr_debug_flag > 2) && fr_log_fp) {
		for (i = 0; i < packet->data_len; i++) {
			if ((i & 0x0f) == 0x00) fprintf(fr_log_fp, "%d: ", i);
			fprintf(fr_log_fp, "%02x ", packet->data[i]);
			if ((i & 0x0f) == 0x0f) fprintf(fr_log_fp, "\n");
		}
		fprintf(fr_log_fp, "\n");
	}

	return 0;
}

#ifdef SIOCSARP
int fr_dhcp_add_arp_entry(int fd, char const *interface,
			  VALUE_PAIR *macaddr, VALUE_PAIR *ip)
{
	struct sockaddr_in *sin;
	struct arpreq req;

	if (macaddr->length > sizeof (req.arp_ha.sa_data)) {
		fr_strerror_printf("ERROR: DHCP only supports up to %zu octets "
				   "for Client Hardware Address "
				   "(got %zu octets)\n",
				   sizeof(req.arp_ha.sa_data),
				   macaddr->length);
		return -1;
	}

	memset(&req, 0, sizeof(req));
	sin = (struct sockaddr_in *) &req.arp_pa;
	sin->sin_family = AF_INET;
	sin->sin_addr.s_addr = ip->vp_ipaddr;
	strlcpy(req.arp_dev, interface, sizeof(req.arp_dev));
	memcpy(&req.arp_ha.sa_data, macaddr->vp_octets, macaddr->length);

	req.arp_flags = ATF_COM;
	if (ioctl(fd, SIOCSARP, &req) < 0) {
		fr_strerror_printf("DHCP: Failed to add entry in ARP cache: %s (%d)",
				   strerror(errno), errno);
		return -1;
	}

	return 0;
}
#else
int fr_dhcp_add_arp_entry(UNUSED int fd, UNUSED char const *interface,
			  UNUSED VALUE_PAIR *macaddr, UNUSED VALUE_PAIR *ip)
{
	fr_strerror_printf("Adding ARP entry is unsupported on this system");
	return -1;
}
#endif
