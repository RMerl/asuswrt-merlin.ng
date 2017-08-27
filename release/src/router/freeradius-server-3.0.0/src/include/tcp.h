/*
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
 */
#ifndef FR_TCP_H
#define FR_TCP_H
/*
 * $Id$
 *
 * @file tcp.h
 * @brief RADIUS over TCP
 *
 * @copyright 2009 Dante http://dante.net
 */

RCSIDH(tcp_h, "$Id$")

int fr_tcp_socket(fr_ipaddr_t *ipaddr, int port);
int fr_tcp_client_socket(fr_ipaddr_t *src_ipaddr, fr_ipaddr_t *dst_ipaddr, int dst_port);
int fr_tcp_read_packet(RADIUS_PACKET *packet, int flags);
RADIUS_PACKET *fr_tcp_recv(int sockfd, int flags);
RADIUS_PACKET *fr_tcp_accept(int sockfd);
ssize_t fr_tcp_write_packet(RADIUS_PACKET *packet);
#endif /* FR_TCP_H */
