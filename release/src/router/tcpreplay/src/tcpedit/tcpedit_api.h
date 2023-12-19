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

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Selection of the encoder plugin is usually done by tcpedit_post_args()
 * so when using the config API you must manually specify it using one of
 * the following functions
 */
int tcpedit_set_encoder_dltplugin_byid(tcpedit_t *, int);
int tcpedit_set_encoder_dltplugin_byname(tcpedit_t *, const char *);

/**
 * setters always return TCPEDIT_OK on success or TCPEDIT_ERROR
 * if there is a problem.  You can use tcpedit_geterr() to get the reason
 * for the failure
 */
int tcpedit_set_skip_broadcast(tcpedit_t *, bool);
int tcpedit_set_fixlen(tcpedit_t *, tcpedit_fixlen);
int tcpedit_set_fixcsum(tcpedit_t *, bool);
int tcpedit_set_efcs(tcpedit_t *, bool);
int tcpedit_set_ttl_mode(tcpedit_t *, tcpedit_ttl_mode);
int tcpedit_set_ttl_value(tcpedit_t *, uint8_t);
int tcpedit_set_tos(tcpedit_t *, uint8_t);
int tcpedit_set_tclass(tcpedit_t *, uint8_t);
int tcpedit_set_flowlabel(tcpedit_t *, uint32_t);
int tcpedit_set_seed(tcpedit_t *);
int tcpedit_set_mtu(tcpedit_t *, int);
int tcpedit_set_mtu_truncate(tcpedit_t *, bool);
int tcpedit_set_maxpacket(tcpedit_t *, int);
int tcpedit_set_cidrmap_s2c(tcpedit_t *, char *);
int tcpedit_set_cidrmap_c2s(tcpedit_t *, char *);
int tcpedit_set_srcip_map(tcpedit_t *, char *);
int tcpedit_set_dstip_map(tcpedit_t *, char *);
int tcpedit_set_port_map(tcpedit_t *, char *);

#ifdef __cplusplus
}
#endif
