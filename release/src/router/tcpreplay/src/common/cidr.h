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

#include "cache.h"

struct tcpr_cidr_s {
    int family; /* AF_INET or AF_INET6 */
    union {
        u_int32_t network;
        struct tcpr_in6_addr network6;
    } u;
    int masklen;
    struct tcpr_cidr_s *next;
};

typedef struct tcpr_cidr_s tcpr_cidr_t;

struct tcpr_cidrmap_s {
    tcpr_cidr_t *from;
    tcpr_cidr_t *to;
    struct tcpr_cidrmap_s *next;
};
typedef struct tcpr_cidrmap_s tcpr_cidrmap_t;

int ip_in_cidr(const tcpr_cidr_t *, const unsigned long);
int check_ip_cidr(tcpr_cidr_t *, const unsigned long);
int check_ip6_cidr(tcpr_cidr_t *, const struct tcpr_in6_addr *addr);
int parse_cidr(tcpr_cidr_t **, char *, char *delim);
int parse_cidr_map(tcpr_cidrmap_t **, const char *);
int parse_endpoints(tcpr_cidrmap_t **, tcpr_cidrmap_t **, const char *);
void add_cidr(tcpr_cidr_t **, tcpr_cidr_t **);
tcpr_cidr_t *new_cidr(void);
tcpr_cidrmap_t *new_cidr_map(void);
void destroy_cidr(tcpr_cidr_t *);
void print_cidr(tcpr_cidr_t *);

int ip6_in_cidr(const tcpr_cidr_t *mycidr, const struct tcpr_in6_addr *addr);
int check_ip6_cidr(tcpr_cidr_t *, const struct tcpr_in6_addr *addr);
