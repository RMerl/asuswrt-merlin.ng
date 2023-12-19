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

/* we don't support endpoints w/ tcpbridge */
#define TCPEDIT_ENDPOINTS_DISABLE 1

#include "defines.h"
#include "config.h"
#include "common.h"
#include "tcpedit/tcpedit.h"
#include <fcntl.h>
#include <regex.h>
#include <sys/stat.h>
#include <sys/types.h>

#ifdef ENABLE_DMALLOC
#include <dmalloc.h>
#endif

/* run-time options */
typedef struct {
    char *intf1;
    char *intf2;

    /* store the mac address of each interface here to prevent loops */
    char intf1_mac[ETHER_ADDR_LEN];
    char intf2_mac[ETHER_ADDR_LEN];

    /* truncate packet ? */
    int truncate;

    COUNTER limit_send;

    pcap_t *pcap1;
    pcap_t *pcap2;
    int unidir;
    int snaplen;
    int to_ms;
    int promisc;
    int poll_timeout;

#ifdef ENABLE_VERBOSE
    /* tcpdump verbose printing */
    int verbose;
    char *tcpdump_args;
    tcpdump_t *tcpdump;
#endif

    /* filter options */
    tcpr_xX_t xX;
    tcpr_bpf_t bpf;
    regex_t preg;
    tcpr_cidr_t *cidrdata;

    int mtu;
    int maxpacket;
    int fixcsum;
    u_int16_t l2proto;
    u_int16_t l2_mem_align; /* keep things 4 byte aligned */
} tcpbridge_opt_t;
