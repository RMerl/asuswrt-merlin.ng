#ifndef FR_PACKET_H
#define FR_PACKET_H

/*
 * packet.h	Structures and prototypes
 *		for packet manipulation
 *
 * Version:	$Id$
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
 * Copyright 2001,2002,2003,2004,2005,2006  The FreeRADIUS server project
 */

RCSIDH(packet_h, "$Id$")

#ifdef __cplusplus
extern "C" {
#endif

int fr_packet_cmp(RADIUS_PACKET const *a, RADIUS_PACKET const *b);
int fr_inaddr_any(fr_ipaddr_t *ipaddr);
void fr_request_from_reply(RADIUS_PACKET *request,
			     RADIUS_PACKET const *reply);
int fr_socket(fr_ipaddr_t *ipaddr, int port);
int fr_nonblock(int fd);

typedef struct fr_packet_list_t fr_packet_list_t;

fr_packet_list_t *fr_packet_list_create(int alloc_id);
void fr_packet_list_free(fr_packet_list_t *pl);
bool fr_packet_list_insert(fr_packet_list_t *pl,
			    RADIUS_PACKET **request_p);

RADIUS_PACKET **fr_packet_list_find(fr_packet_list_t *pl,
				      RADIUS_PACKET *request);
RADIUS_PACKET **fr_packet_list_find_byreply(fr_packet_list_t *pl,
					      RADIUS_PACKET *reply);
bool fr_packet_list_yank(fr_packet_list_t *pl,
			 RADIUS_PACKET *request);
int fr_packet_list_num_elements(fr_packet_list_t *pl);
bool fr_packet_list_id_alloc(fr_packet_list_t *pl, int proto,
			    RADIUS_PACKET **request_p, void **pctx);
bool fr_packet_list_id_free(fr_packet_list_t *pl,
			    RADIUS_PACKET *request, bool yank);
bool fr_packet_list_socket_add(fr_packet_list_t *pl, int sockfd, int proto,
			      fr_ipaddr_t *dst_ipaddr, int dst_port,
			      void *ctx);
bool fr_packet_list_socket_del(fr_packet_list_t *pl, int sockfd);
bool fr_packet_list_socket_freeze(fr_packet_list_t *pl, int sockfd);
bool fr_packet_list_socket_thaw(fr_packet_list_t *pl, int sockfd);
int fr_packet_list_walk(fr_packet_list_t *pl, void *ctx,
			  fr_hash_table_walk_t callback);
int fr_packet_list_fd_set(fr_packet_list_t *pl, fd_set *set);
RADIUS_PACKET *fr_packet_list_recv(fr_packet_list_t *pl, fd_set *set);

int fr_packet_list_num_incoming(fr_packet_list_t *pl);
int fr_packet_list_num_outgoing(fr_packet_list_t *pl);

/*
 *	"find" returns a pointer to the RADIUS_PACKET* member in the
 *	caller's structure.  In order to get the pointer to the *top*
 *	of the caller's structure, you have to subtract the offset to
 *	the member from the returned pointer, and cast it to the
 *	required type.
 */
# define fr_packet2myptr(TYPE, MEMBER, PTR) (TYPE *) (((char *)PTR) - offsetof(TYPE, MEMBER))

#ifdef __cplusplus
}
#endif

#endif /* FR_PACKET_H */
