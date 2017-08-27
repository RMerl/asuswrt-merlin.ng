/*
 *  radsniff.h	Structures and defines for the RADIUS sniffer.
 *
 *  Version:    $Id$
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 *
 *  Copyright 2006  The FreeRADIUS server project
 *  Copyright 2006  Nicolas Baradakis <nicolas.baradakis@cegetel.net>
 */

RCSIDH(radsniff_h, "$Id$")

#include <sys/types.h>
#include <netinet/in.h>

/*
 *	The number of bytes in an ethernet (MAC) address.
 */
#define ETHER_ADDR_LEN		6

/*
 *	Structure of a DEC/Intel/Xerox or 802.3 Ethernet header.
 */
struct  ethernet_header {
	uint8_t	ethernet_dhost[ETHER_ADDR_LEN];
	uint8_t	ethernet_shost[ETHER_ADDR_LEN];
	uint16_t	ethernet_type;
};

/*
 *	Length of a DEC/Intel/Xerox or 802.3 Ethernet header.
 *	Note that some compilers may pad "struct ether_header" to
 *	a multiple of 4 *bytes, for example, so "sizeof (struct
 *	ether_header)" may not give the right answer.
 */
#define ETHER_HDRLEN		14

/*
 *	Structure of an internet header, naked of options.
 */
struct ip_header {
	uint8_t	ip_vhl;	 /* header length, version */
#define IP_V(ip)	(((ip)->ip_vhl & 0xf0) >> 4)
#define IP_HL(ip)       ((ip)->ip_vhl & 0x0f)
	uint8_t	ip_tos;	 /* type of service */
	uint16_t       ip_len;	 /* total length */
	uint16_t       ip_id;	  /* identification */
	uint16_t       ip_off;	 /* fragment offset field */
#define I_DF 0x4000		    /* dont fragment flag */
#define IP_MF 0x2000		    /* more fragments flag */
#define IP_OFFMASK 0x1fff	       /* mask for fragmenting bits */
	uint8_t	ip_ttl;	 /* time to live */
	uint8_t	ip_p;	   /* protocol */
	uint16_t       ip_sum;	 /* checksum */
	struct in_addr  ip_src,ip_dst;  /* source and dest address */
};

/*
 *	UDP protocol header.
 *	Per RFC 768, September, 1981.
 */
struct udp_header {
	uint16_t       udp_sport;	       /* source port */
	uint16_t       udp_dport;	       /* destination port */
	uint16_t       udp_ulen;		/* udp length */
	uint16_t       udp_sum;		 /* udp checksum */
};

/*
 *	RADIUS packet length.
 *	RFC 2865, Section 3., subsection 'length' says:
 *	" ... and maximum length is 4096."
 */
#define MAX_RADIUS_LEN 4096
#define MIN_RADIUS_LEN 20
#define SNAPLEN (sizeof(struct ethernet_header) + sizeof(struct ip_header) + sizeof(struct udp_header) + MAX_RADIUS_LEN)

typedef struct radius_packet_t {
	uint8_t       code;
	uint8_t       id;
	uint8_t       length[2];
	uint8_t       vector[AUTH_VECTOR_LEN];
	uint8_t       data[1];
} radius_packet_t;

#define AUTH_HDR_LEN 20
