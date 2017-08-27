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
#ifndef FR_VQP_H
#define FR_VQP_H

/*
 * $Id$
 *
 * @file vqp.h
 * @brief Structures and prototypes for Cisco's VLAN Query Protocol
 *
 * @copyright 2007  The FreeRADIUS server project
 * @copyright 2007  Alan DeKok <aland@deployingradius.com>
 */

RCSIDH(vqp_h, "$Id$")

#ifdef __cplusplus
extern "C" {
#endif

RADIUS_PACKET *vqp_recv(int sockfd);
int vqp_send(RADIUS_PACKET *packet);
int vqp_decode(RADIUS_PACKET *packet);
int vqp_encode(RADIUS_PACKET *packet, RADIUS_PACKET *original);

#ifdef __cplusplus
}
#endif

#endif /* FR_VQP_H */
