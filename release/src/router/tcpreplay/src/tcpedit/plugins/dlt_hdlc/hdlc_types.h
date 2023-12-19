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

#pragma once

#include "plugins_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * structure to hold any data parsed from the packet by the decoder.
 * Example: Ethernet VLAN tag info
 */
typedef struct {
    union {
        struct {
            int hdlc; /* set to 1 if values below are filled out */
            u_int8_t address;
            u_int8_t control;
        };
        u_char packet[MAXPACKET];
    };
} hdlc_extra_t;

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
    /* user defined values.  65535 == unset */
    u_int16_t address;
    u_int16_t control;
} hdlc_config_t;

/* Cisco HDLC has a simple 32 bit header */
#define CISCO_HDLC_LEN 4

typedef struct {
    u_int8_t address;
#define CISCO_HDLC_ADDR_UNICAST 0x0F
#define CISCO_HDLC_ADDR_BROADCAST 0x8F
    u_int8_t control; // always zero
    u_int16_t protocol;
} cisco_hdlc_t;

#ifdef __cplusplus
}
#endif
