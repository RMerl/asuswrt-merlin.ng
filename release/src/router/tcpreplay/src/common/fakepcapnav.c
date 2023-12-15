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

/*  This file implements a fake, non-functioning version of the libpcapnav
 *  API based on libpcap.  It's solely here for people who don't have
 *  libpcapnav installed on their system, and to keep the code maintainable.
 */

#include "defines.h"
#include "config.h"
#include "common.h"
#include <stdlib.h>

#ifndef HAVE_PCAPNAV

/**
 * pcapnav_init does nothing!
 */
void
pcapnav_init(void)
{}

/**
 * pcapnav_open_offline opens a pcap file,
 * and creates the struct for our use
 */
pcapnav_t *
pcapnav_open_offline(const char *filename)
{
    pcapnav_t *pcapnav;
    char errbuf[PCAP_ERRBUF_SIZE];

    pcapnav = (pcapnav_t *)malloc(sizeof(pcapnav_t));
    if (pcapnav == NULL) {
        err(-1, "malloc() error: unable to malloc pcapnav_t");
    }

    pcapnav->pcap = pcap_open_offline(filename, errbuf);
    if (pcapnav->pcap == NULL) {
        errx(-1, "Error opening pcap file %s: %s", filename, errbuf);
    }

    return (pcapnav);
}

/**
 * closes our pcap file and free's the pcapnav
 */
void
pcapnav_close(pcapnav_t *pcapnav)
{
    pcap_close(pcapnav->pcap);
    safe_free(pcapnav);
}

/**
 * returns the pcap_t data struct
 */
pcap_t *
pcapnav_pcap(pcapnav_t *pcapnav)
{
    return (pcapnav->pcap);
}

#endif
