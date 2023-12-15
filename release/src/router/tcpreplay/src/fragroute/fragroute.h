/* $Id$ */

/*
 *   Copyright (c) 2001 Dug Song <dugsong@monkey.org>
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

#include "config.h"
#include "pkt.h"

#define FRAGROUTE_ERRBUF_LEN 1024

/* Fragroute context. */
struct fragroute_s {
    struct addr src;
    struct addr dst;
    struct addr smac;
    struct addr dmac;
    int dlt;
    int mtu;
    int first_packet; /* have we called getfragment() yet after process()? */
    int l2len;
    u_char l2header[50];
    char errbuf[FRAGROUTE_ERRBUF_LEN];
    struct pktq *pktq; /* packet chain */
};

typedef struct fragroute_s fragroute_t;

int fragroute_process(fragroute_t *ctx, void *buf, size_t len);
int fragroute_getfragment(fragroute_t *ctx, char **packet);
fragroute_t *fragroute_init(const int mtu, const int dlt, const char *config, char *errbuf);
void fragroute_close(fragroute_t *ctx);
