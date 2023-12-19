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

int dlt_raw_register(tcpeditdlt_t *ctx);
int dlt_raw_init(tcpeditdlt_t *ctx);
int dlt_raw_cleanup(tcpeditdlt_t *ctx);
int dlt_raw_parse_opts(tcpeditdlt_t *ctx);
int dlt_raw_decode(tcpeditdlt_t *ctx, const u_char *packet, int pktlen);
int dlt_raw_encode(tcpeditdlt_t *ctx, u_char *packet, int pktlen, tcpr_dir_t dir);
int dlt_raw_proto(tcpeditdlt_t *ctx, const u_char *packet, int pktlen);
u_char *dlt_raw_get_layer3(tcpeditdlt_t *ctx, u_char *packet, int pktlen);
u_char *dlt_raw_merge_layer3(tcpeditdlt_t *ctx, u_char *packet, int pktlen, u_char *ipv4_data, u_char *ipv6_data);
tcpeditdlt_l2addr_type_t dlt_raw_l2addr_type(void);
int dlt_raw_l2len(tcpeditdlt_t *ctx, const u_char *packet, int pktlen);
u_char *dlt_raw_get_mac(tcpeditdlt_t *ctx, tcpeditdlt_mac_type_t mac, const u_char *packet, int pktlen);

/*
 * structure to hold any data parsed from the packet by the decoder.
 * Example: Ethernet VLAN tag info
 */
struct raw_extra_s {
    u_char packet[MAXPACKET];
};
typedef struct raw_extra_s raw_extra_t;

/*
 * FIXME: structure to hold any data in the tcpeditdlt_plugin_t->config
 * Things like:
 * - Parsed user options
 * - State between packets
 * - Note, you should only use this for the encoder function, decoder functions should place
 *   "extra" data parsed from the packet in the tcpeditdlt_t->decoded_extra buffer since that
 *   is available to any encoder plugin.
 */
struct raw_config_s {
    /* dummy entry for SunPro compiler which doesn't like empty structs */
    int dummy;
};
typedef struct raw_config_s raw_config_t;
