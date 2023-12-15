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

#include "tcpreplay_api.h"
#include <pcap.h>

void send_packets(tcpreplay_t *ctx, pcap_t *pcap, int idx);
void send_dual_packets(tcpreplay_t *ctx, pcap_t *pcap1, int idx1, pcap_t *pcap2, int idx2);
void *cache_mode(tcpreplay_t *ctx, char *cachedata, COUNTER packet_num);
void preload_pcap_file(tcpreplay_t *ctx, int idx);
