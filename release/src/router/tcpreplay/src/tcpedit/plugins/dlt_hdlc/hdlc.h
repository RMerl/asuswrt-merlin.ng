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

#include "hdlc_types.h"
#include "plugins_types.h"

int dlt_hdlc_register(tcpeditdlt_t *ctx);
int dlt_hdlc_init(tcpeditdlt_t *ctx);
int dlt_hdlc_cleanup(tcpeditdlt_t *ctx);
int dlt_hdlc_parse_opts(tcpeditdlt_t *ctx);
int dlt_hdlc_decode(tcpeditdlt_t *ctx, const u_char *packet, int pktlen);
int dlt_hdlc_encode(tcpeditdlt_t *ctx, u_char *packet, int pktlen, tcpr_dir_t dir);
int dlt_hdlc_proto(tcpeditdlt_t *ctx, const u_char *packet, int pktlen);
u_char *dlt_hdlc_get_layer3(tcpeditdlt_t *ctx, u_char *packet, int pktlen);
u_char *dlt_hdlc_merge_layer3(tcpeditdlt_t *ctx, u_char *packet, int pktlen, u_char *ipv4_data, u_char *ipv6_data);
tcpeditdlt_l2addr_type_t dlt_hdlc_l2addr_type(void);
int dlt_hdlc_l2len(tcpeditdlt_t *ctx, const u_char *packet, int pktlen);
u_char *dlt_hdlc_get_mac(tcpeditdlt_t *ctx, tcpeditdlt_mac_type_t mac, const u_char *packet, int pktlen);
