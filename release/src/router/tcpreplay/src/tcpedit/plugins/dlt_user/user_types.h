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

#ifdef __cplusplus
extern "C" {
#endif

typedef enum { TCPEDIT_USER_DLT_BOTH, TCPEDIT_USER_DLT_S2C, TCPEDIT_USER_DLT_C2S } tcpedit_user_dlt_direction;

/*
 * FIXME: structure to hold any data parsed from the packet by the decoder.
 * Example: Ethernet VLAN tag info
 */
typedef struct {
    /* dummy entry for SunPro compiler which doesn't like empty structs */
    int dummy;
} user_extra_t;

#define USER_L2MAXLEN 255

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
    u_int16_t dlt;
    int length;
    u_char l2client[USER_L2MAXLEN];
    u_char l2server[USER_L2MAXLEN];
} user_config_t;

#ifdef __cplusplus
}
#endif
