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
#include "config.h"
#include "tcpedit/tcpedit.h"

#ifdef ENABLE_DMALLOC
#include <dmalloc.h>
#endif

#ifdef ENABLE_FRAGROUTE
#include "fragroute/fragroute.h"
#endif

/* runtime options */
struct tcprewrite_opt_s {
    /* input and output pcap filenames & handles */
    char *infile;
    char *outfile;
    pcap_t *pin;
    pcap_dumper_t *pout;

    /* tcpprep cache data */
    COUNTER cache_packets;
    char *cachedata;

    /* tcpprep cache file comment */
    char *comment;

#ifdef ENABLE_VERBOSE
    /* tcpdump verbose printing */
    int verbose;
    char *tcpdump_args;
#endif

#ifdef ENABLE_FRAGROUTE
    char *fragroute_args;
    fragroute_t *frag_ctx;
#define FRAGROUTE_DIR_C2S 1
#define FRAGROUTE_DIR_S2C 2
#define FRAGROUTE_DIR_BOTH 4
    int fragroute_dir;
#endif
    tcpedit_t *tcpedit;
};

typedef struct tcprewrite_opt_s tcprewrite_opt_t;
