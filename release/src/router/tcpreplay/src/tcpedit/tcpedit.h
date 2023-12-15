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

#include "defines.h"
#include "common.h"
#include "parse_args.h"
#include "plugins.h"
#include "tcpedit_api.h"
#include "tcpedit_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * libtcpedit common API
 */

int tcpedit_init(tcpedit_t **tcpedit_ex, int dlt);
char *tcpedit_geterr(tcpedit_t *tcpedit);
char *tcpedit_getwarn(tcpedit_t *tcpedit);

int tcpedit_checkerror(tcpedit_t *tcpedit, int rcode, const char *prefix);
int tcpedit_validate(tcpedit_t *tcpedit);

int tcpedit_packet(tcpedit_t *tcpedit, struct pcap_pkthdr **pkthdr, u_char **pktdata, tcpr_dir_t direction);

int tcpedit_close(tcpedit_t **tcpedit_ex);
int tcpedit_get_output_dlt(tcpedit_t *tcpedit);

const u_char *tcpedit_l3data(tcpedit_t *tcpedit, tcpedit_coder code, u_char *packet, int pktlen);

int tcpedit_l3proto(tcpedit_t *tcpedit, tcpedit_coder code, const u_char *packet, int pktlen);

COUNTER tcpedit_get_total_bytes(tcpedit_t *tcpedit);
COUNTER tcpedit_get_pkts_edited(tcpedit_t *tcpedit);

/**
 * These functions are seen by the outside world, but nobody should ever use them
 * outside of internal tcpedit functions
 */

#define tcpedit_seterr(x, y, ...) __tcpedit_seterr(x, __FUNCTION__, __LINE__, __FILE__, y, __VA_ARGS__)
void __tcpedit_seterr(tcpedit_t *tcpedit, const char *func, int line, const char *file, const char *fmt, ...);
void tcpedit_setwarn(tcpedit_t *tcpedit, const char *fmt, ...);

#ifdef __cplusplus
}
#endif
