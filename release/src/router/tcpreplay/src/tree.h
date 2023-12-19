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
#include "tcpr.h"
#include <lib/tree.h>

#define TREEPRINTBUFFLEN 2048

typedef struct tcpr_tree_s {
    RB_ENTRY(tcpr_tree_s) node;
    int family;
    union {
        unsigned long ip; /* ip/network address in network byte order */
        struct tcpr_in6_addr ip6;
    } u;
    int masklen;    /* CIDR network mask length */
    int server_cnt; /* count # of times this entry was flagged server */
    int client_cnt; /* flagged client */
    int type;       /* 1 = server, 0 = client, -1 = undefined */
} tcpr_tree_t;

/*
 * replacement for RB_HEAD() which doesn't actually declare the root
 */
typedef struct tcpr_data_tree_s {
    tcpr_tree_t *rbh_root;
} tcpr_data_tree_t;

typedef struct tcpr_buildcidr_s {
    int type;    /* SERVER|CLIENT|UNKNOWN|ANY */
    int masklen; /* mask size to use to build the CIDR */
} tcpr_buildcidr_t;

#define DNS_QUERY_FLAG 0x8000

void add_tree_ipv4(unsigned long, const u_char *, int, int);
void add_tree_ipv6(const struct tcpr_in6_addr *, const u_char *, int, int);
void add_tree_first_ipv4(const u_char *, int, int);
void add_tree_first_ipv6(const u_char *, int, int);
tcpr_dir_t check_ip_tree(int, unsigned long);
tcpr_dir_t check_ip6_tree(int, const struct tcpr_in6_addr *);
int process_tree();
void tree_calculate(tcpr_data_tree_t *);
int tree_comp(tcpr_tree_t *, tcpr_tree_t *);
