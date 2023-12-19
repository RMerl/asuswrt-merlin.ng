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

#define DEFAULT_FLOW_HASH_BUCKET_SIZE (1 << 16) /* 64K - must be a power of two */

typedef enum flow_entry_type_e {
    FLOW_ENTRY_INVALID,  /* unknown packet type */
    FLOW_ENTRY_NON_IP,   /* is a flow, but non-IP */
    FLOW_ENTRY_NEW,      /* flow never seen before */
    FLOW_ENTRY_EXISTING, /* flow already seen */
    FLOW_ENTRY_EXPIRED,  /* flow existed but was expired */
} flow_entry_type_t;

typedef struct flow_hash_table flow_hash_table_t;

flow_hash_table_t *flow_hash_table_init(size_t n);
void flow_hash_table_release(flow_hash_table_t *table);
flow_entry_type_t flow_decode(flow_hash_table_t *fht,
                              const struct pcap_pkthdr *pkthdr,
                              const u_char *pktdata,
                              const int datalink,
                              const int expiry);
