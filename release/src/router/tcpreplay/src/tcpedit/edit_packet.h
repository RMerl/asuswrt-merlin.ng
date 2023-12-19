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

#include "common.h"
#include "tcpedit.h"

int untrunc_packet(tcpedit_t *tcpedit,
                   struct pcap_pkthdr *pkthdr,
                   u_char **pktdata,
                   ipv4_hdr_t *ip_hdr,
                   ipv6_hdr_t *ip6_hdr);

int
randomize_ipv4(tcpedit_t *tcpedit, struct pcap_pkthdr *pktdhr, const u_char *pktdata, ipv4_hdr_t *ip_hdr, int l3len);

int
randomize_ipv6(tcpedit_t *tcpedit, struct pcap_pkthdr *pktdhr, const u_char *pktdata, ipv6_hdr_t *ip_hdr, int l3len);

int randomize_iparp(tcpedit_t *tcpedit, struct pcap_pkthdr *pkthdr, const u_char *pktdata, int datalink, int l3len);

int fix_ipv4_checksums(tcpedit_t *tcpedit, struct pcap_pkthdr *pkdhdr, ipv4_hdr_t *ip_hdr, size_t l2len);

int fix_ipv6_checksums(tcpedit_t *tcpedit, struct pcap_pkthdr *pkdhdr, ipv6_hdr_t *ip_hdr, size_t l2len);

int fix_ipv4_length(struct pcap_pkthdr *pkthdr, ipv4_hdr_t *ip_hdr, size_t l2len);

int fix_ipv6_length(struct pcap_pkthdr *pkthdr, ipv6_hdr_t *ip6_hdr, size_t l2len);

int extract_data(tcpedit_t *tcpedit, const u_char *pktdata, int caplen, char *l7data[]);

int rewrite_ipv4l3(tcpedit_t *tcpedit, ipv4_hdr_t *ip_hdr, tcpr_dir_t direction, int l3len);

int rewrite_ipv6l3(tcpedit_t *tcpedit, ipv6_hdr_t *ip_hdr, tcpr_dir_t direction, int l3len);

int rewrite_iparp(tcpedit_t *tcpedit, arp_hdr_t *arp_hdr, int direction);

int rewrite_ipv4_ttl(tcpedit_t *tcpedit, ipv4_hdr_t *ip_hdr);

int rewrite_ipv6_hlim(tcpedit_t *tcpedit, ipv6_hdr_t *ip6_hdr);

int extract_ipv4_multicast_mac(tcpedit_t *tcpedit, uint32_t ip, u_char *mac[ETHER_ADDR_LEN]);

int extract_ipv6_multicast_mac(tcpedit_t *tcpedit, struct tcpr_in6_addr *ip6, u_char *mac[ETHER_ADDR_LEN]);

#define BROADCAST_IP 4294967295
