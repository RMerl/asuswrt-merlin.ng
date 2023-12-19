/* $Id$ */

/*
 *   Copyright (c) 2001-2010 Aaron Turner <aturner at synfin dot net>
 *   Copyright (c) 2013-2022 Fred Klassen <tcpreplay at appneta dot com> - AppNeta
 *
 *   The Tcpreplay Suite of tools is free software: you can redistribute it
 *   and/or modify it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation, either version 3 of the
 *   License, or with the authors permission any later version.
 *
 *   The Tcpreplay Suite is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with the Tcpreplay Suite.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "ethernet.h"
#include <assert.h>
#include <string.h>

/*
 * Takes a ptr to an ethernet address and returns
 * 1 if it is unicast or 0 if it is multicast or
 * broadcast. As per RFC7042.
 */
int
is_unicast_ethernet(tcpeditdlt_t *ctx, const u_char *ether)
{
    assert(ctx);
    assert(ether);

    /* ff:ff:ff:ff:ff:ff - broadcast */
    if (memcmp(ether, BROADCAST_MAC, ETHER_ADDR_LEN) == 0)
        return 0;

    /* 01:00:5e:(00:00:00-7f:ff:ff) - IPv4 Multicast and MLPS Multicast */
    if (memcmp(&ether[0], IPV4_MULTICAST_MAC, 3) == 0)
        return 0;

    /* 33:33 - IPv6 Multicast */
    if (memcmp(&ether[0], IPV6_MULTICAST_MAC, 2) == 0)
        return 0;

    /*
     * 00:00:5e:(00:01:00 – 00:01:ff) - IPv4 Virtual Router Redundancy Protocol
     * 00:00:5e:(00:02:00 – 00:02:ff) - IPv6 Virtual Router Redundancy Protocol
     */
    if (memcmp(&ether[0], IPV4_VRRP, 5) == 0 || memcmp(&ether[0], IPV6_VRRP, 5) == 0)
        return 0;

    /* everything else is unicast */
    return 1;
}
