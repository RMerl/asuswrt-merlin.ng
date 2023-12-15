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

#include "config.h"
#include "tcpbridge.h"
#include <lib/tree.h>
#include <tcpedit/tcpedit.h>

/*
 * RBTree node object for tracking which side of tcpreplay where
 * each source MAC address lives
 */
struct macsrc_t {
    RB_ENTRY(macsrc_t) node;
    u_char key[ETHER_ADDR_LEN];
    u_char source;    /* interface device name we first saw the source MAC */
    sendpacket_t *sp; /* sendpacket handle to send packets out */
};

/* pri and secondary pcap interfaces */
#define PCAP_INT1 0
#define PCAP_INT2 1

/* our custom pcap_dispatch handler user struct */
struct live_data_t {
    u_int32_t linktype;
    int l2enabled;
    int l2len;
    u_char source;
    char *l2data;
    pcap_t *pcap;
    tcpedit_t *tcpedit;
    tcpbridge_opt_t *options;
};

void rbinit(void);
void do_bridge(tcpbridge_opt_t *, tcpedit_t *);
