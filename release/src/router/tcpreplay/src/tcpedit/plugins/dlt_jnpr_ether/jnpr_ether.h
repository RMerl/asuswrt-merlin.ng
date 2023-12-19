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

#include "jnpr_ether_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define JUNIPER_ETHER_HEADER_LEN 6
#define JUNIPER_ETHER_MAGIC_LEN 3
#define JUNIPER_ETHER_MAGIC "\x4d\x47\x43"
#define JUNIPER_ETHER_OPTIONS_OFFSET 3
#define JUNIPER_ETHER_L2PRESENT 0x80
#define JUNIPER_ETHER_DIRECTION 0x01
#define JUNIPER_ETHER_EXTLEN_OFFSET 4

int dlt_jnpr_ether_register(tcpeditdlt_t *ctx);
int dlt_jnpr_ether_init(tcpeditdlt_t *ctx);
int dlt_jnpr_ether_post_init(tcpeditdlt_t *ctx);
int dlt_jnpr_ether_cleanup(tcpeditdlt_t *ctx);
int dlt_jnpr_ether_parse_opts(tcpeditdlt_t *ctx);
int dlt_jnpr_ether_decode(tcpeditdlt_t *ctx, const u_char *packet, int pktlen);
int dlt_jnpr_ether_encode(tcpeditdlt_t *ctx, u_char *packet, int pktlen, tcpr_dir_t dir);
int dlt_jnpr_ether_proto(tcpeditdlt_t *ctx, const u_char *packet, int pktlen);
u_char *dlt_jnpr_ether_get_layer3(tcpeditdlt_t *ctx, u_char *packet, int pktlen);
u_char *
dlt_jnpr_ether_merge_layer3(tcpeditdlt_t *ctx, u_char *packet, int pktlen, u_char *ipv4_data, u_char *ipv6_data);
tcpeditdlt_l2addr_type_t dlt_jnpr_ether_l2addr_type(void);
int dlt_jnpr_ether_l2len(tcpeditdlt_t *ctx, const u_char *packet, int pktlen);
u_char *dlt_jnpr_ether_get_mac(tcpeditdlt_t *ctx, tcpeditdlt_mac_type_t mac, const u_char *packet, int pktlen);

#ifdef __cplusplus
}
#endif
