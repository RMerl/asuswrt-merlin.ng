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

#ifdef HAVE_PCAPNAV
#include <pcapnav.h>
#define _FAKEPCAPNAV_H_
#endif

#include "defines.h"
#include "config.h"

#ifndef HAVE_PCAPNAV

typedef struct pcapnav pcapnav_t;

struct pcapnav {
    pcap_t *pcap;
};

void pcapnav_init(void);
pcapnav_t *pcapnav_open_offline(const char *);
void pcapnav_close(pcapnav_t *);
pcap_t *pcapnav_pcap(pcapnav_t *);

#endif
