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
#include "defines.h"
#include "common.h"


int parse_mpls(const u_char *pktdata,
               uint32_t datalen,
               uint16_t *protocol,
               uint32_t *l2len,
               uint32_t *l2offset);

int parse_vlan(const u_char *pktdata,
               uint32_t datalen,
               uint16_t *next_protocol,
               uint32_t *l2len);

int get_l2len(const u_char *pktdata, int datalen, int datalink);

int get_l2len_protocol(const u_char *pktdata,
                       uint32_t datalen,
                       int datalink,
                       uint16_t *protocol,
                       uint32_t *l2len,
                       uint32_t *l2offset,
                       uint32_t *vlan_offset);

void *get_layer4_v4(const ipv4_hdr_t *ip_hdr, const u_char *end_ptr);
void *get_layer4_v6(const ipv6_hdr_t *ip_hdr, const u_char *end_ptr);

u_int8_t get_ipv6_l4proto(const ipv6_hdr_t *ip6_hdr, const u_char *end_ptr);

const u_char *get_ipv4(const u_char *pktdata, int datalen, int datalink, u_char **newbuff);
const u_char *get_ipv6(const u_char *pktdata, int datalen, int datalink, u_char **newbuff);

u_int32_t get_name2addr4(const char *hostname, bool dnslookup);
const char *get_addr2name4(u_int32_t ip, bool dnslookup);
const char *get_addr2name6(const struct tcpr_in6_addr *addr, bool dnslookup);
const char *get_pcap_version(void);

int get_name2addr6(const char *hostname, bool dnslookup, struct tcpr_in6_addr *addr);


const char *get_cidr2name(const tcpr_cidr_t *cidr_ptr, bool dnslookup);
