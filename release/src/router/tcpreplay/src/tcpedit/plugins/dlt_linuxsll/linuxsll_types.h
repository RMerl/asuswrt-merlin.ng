/* $Id$ */

/*
 *   Copyright (c) 2001-2010 Aaron Turner <aturner at synfin dot net>
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

#pragma once

#include "plugins_types.h"
#include "tcpedit_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * structure to hold any data parsed from the packet by the decoder.
 * Example: Ethernet VLAN tag info
 */
typedef struct {
    u_char packet[MAXPACKET];
} linuxsll_extra_t;

/*
 * FIXME: structure to hold any data in the tcpeditdlt_plugin_t->config
 * Things like:
 * - Parsed user options
 * - State between packets
 * - Note, you should only use this for the encoder function, decoder functions should place
 *   "extra" data parsed from the packet in the tcpeditdlt_t->decoded_extra buffer since that
 *   is available to any encoder plugin.
 */
typedef struct {
    /* dummy entry for SunPro compiler which doesn't like empty structs */
    int dummy;
} linuxsll_config_t;

typedef struct {
    u_int16_t source;  /* values 0-4 determine where the packet came and where it's going */
    u_int16_t type;    /* linux ARPHRD_* values for link-layer device type.  See:
                        * http://www.gelato.unsw.edu.au/lxr/source/include/linux/if_arp.h
                        */
#define ARPHRD_ETHER 1 /* ethernet */
    u_int16_t length;  /* source address length */
    u_char address[8]; /* first 8 bytes of source address (may be truncated) */
    u_int16_t proto;   /* Ethernet protocol type */
} linux_sll_header_t;

#ifdef __cplusplus
}
#endif
