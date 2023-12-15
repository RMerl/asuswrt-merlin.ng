/* -*- mode: c; c-basic-offset: 2 -*- */
/* 
 * Copyright (C) 2007-2012 David Bird (Coova Technologies) <support@coova.com>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * 
 */

#ifndef _GARDEN_H_
#define _GARDEN_H_

#include "pkt.h"

typedef struct pass_through_t {
  struct in_addr host;              /* IP or Network */
  struct in_addr mask;              /* Netmask */
  uint8_t proto;                    /* TCP, UDP, or ICMP */
  uint16_t port;                    /* TCP or UDP Port */
#ifdef ENABLE_GARDENEXT
  time_t expiry;
#endif
} pass_through;

#define pt_equal(a,b) (\
  (a)->host.s_addr == (b)->host.s_addr && \
  (a)->mask.s_addr == (b)->mask.s_addr && \
  (a)->proto       == (b)->proto       && \
  (a)->port        == (b)->port)

#ifdef ENABLE_CHILLIREDIR
typedef struct regex_pass_through_t {
  char regex_host[512];
  char regex_path[512];
  char regex_qs[512];
  regex_t re_host;
  regex_t re_path;
  regex_t re_qs;
  char inuse:1;
  char neg_host:1;
  char neg_path:1;
  char neg_qs:1;
  char reserved:4;
} regex_pass_through;

int regex_pass_throughs_from_string(regex_pass_through *ptlist, 
				    uint32_t ptlen, uint32_t *ptcnt, 
				    char *s, char is_dyn);
#endif

int pass_through_add(pass_through *ptlist, 
		     uint32_t ptlen, uint32_t *ptcnt, 
		     pass_through *pt, char is_dyn
#ifdef HAVE_PATRICIA
		     , patricia_tree_t *ptree
#endif
		     );

int pass_through_rem(pass_through *ptlist, 
		     uint32_t *ptcnt, 
		     pass_through *pt
#ifdef HAVE_PATRICIA
		     , patricia_tree_t *ptree
#endif
		     );

int pass_throughs_from_string(pass_through *ptlist, 
			      uint32_t ptlen, uint32_t *ptcnt, 
			      char *s, char is_dyn, char is_rem
#ifdef HAVE_PATRICIA
			      , patricia_tree_t *ptree
#endif
			      );

int garden_check(pass_through *ptlist, uint32_t *ptcnt, 
		 pass_through **pt_match,
		 struct pkt_ipphdr_t *ipph, int dst
#ifdef HAVE_PATRICIA
		 , patricia_tree_t *ptree
#endif
		 );

#ifdef ENABLE_CHILLIQUERY
void garden_print(int fd);
#endif

#ifdef HAVE_PATRICIA
int garden_patricia_print(int fd, patricia_tree_t *ptree);
int garden_patricia_check(patricia_tree_t *ptree, 
			  pass_through *ptlist, uint32_t *ptcnt,
			  struct pkt_ipphdr_t *ipph, int dst);
void garden_patricia_load_list(patricia_tree_t **pptree,
			       pass_through *ptlist,
			       uint32_t ptcnt);
void garden_patricia_reload();
#endif

#ifdef ENABLE_UAMDOMAINFILE
void garden_load_domainfile();
void garden_free_domainfile();
int  garden_check_domainfile(char *question);
#endif

#endif
