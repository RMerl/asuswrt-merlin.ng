/* -*- mode: c; c-basic-offset: 2 -*- */
/* 
 * Copyright (C) 2003, 2004, 2005 Mondru AB.
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


#ifndef _IPPOOL_H
#define _IPPOOL_H

/* Assuming that the address space is fragmented we need a hash table
   in order to return the addresses.

   The list pool should provide for both IPv4 and IPv6 addresses.

   When initialising a new address pool it should be possible to pass
   a string of CIDR format networks: "10.0.0.0/24 10.15.0.0/20" would
   translate to 256 addresses starting at 10.0.0.0 and 1024 addresses
   starting at 10.15.0.0. 

   The above also applies to IPv6 which can be specified as described
   in RFC2373.
*/

#define IPPOOL_NOIP6

#include "system.h"
#include "lookup.h"

struct ippoolm_t;                /* Forward declaration */

struct ippool_t {
  int dynsize;                   /* Total number of dynamic addresses */
  int statsize;                  /* Total number of static addresses */
  int listsize;                  /* Total number of addresses */
  int allowdyn;                  /* Allow dynamic IP address allocation */
  int allowstat;                 /* Allow static IP address allocation */
  struct in_addr stataddr;       /* Static address range network address */
  struct in_addr statmask;       /* Static address range network mask */
  struct ippoolm_t *member;      /* Listsize array of members */
  int hashsize;                  /* Size of hash table */
  int hashlog;                   /* Log2 size of hash table */
  int hashmask;                  /* Bitmask for calculating hash */
  struct ippoolm_t **hash;       /* Hashsize array of pointer to member */
  struct ippoolm_t *firstdyn;    /* Pointer to first free dynamic member */
  struct ippoolm_t *lastdyn;     /* Pointer to last free dynamic member */
  struct ippoolm_t *firststat;   /* Pointer to first free static member */
  struct ippoolm_t *laststat;    /* Pointer to last free static member */
};

struct ippoolm_t {
#ifndef IPPOOL_NOIP6
  struct in6_addr addr;          /* IP address of this member */
#else
  struct in_addr addr;           /* IP address of this member */
#endif
  char in_use;                   /* 0=available; 1= used */
  char is_static;                /* 0= dynamic; 1 = static */
  struct ippoolm_t *nexthash;    /* Linked list part of hash table */
  struct ippoolm_t *prev, *next; /* Linked list of free dynamic or static */
  void *peer;                    /* Pointer to peer protocol handler */
};

/* The above structures require approximately 20+4 = 24 bytes for
   each address (IPv4). For IPv6 the corresponding value is 32+4 = 36
   bytes for each address. */

/* Hash an IP address using code based on Bob Jenkins lookupa */
extern uint32_t ippool_hash4(struct in_addr *addr);

/* Create new address pool */
extern int ippool_new(struct ippool_t **this, 
		      char *dyn, int start, int end, 
		      char *stat, int allowdyn, int allowstat);

/* Delete existing address pool */
extern int ippool_free(struct ippool_t *this);

/* Find an IP address in the pool */
extern int ippool_getip(struct ippool_t *this, struct ippoolm_t **member,
		 struct in_addr *addr);

/* Get an IP address. If addr = 0.0.0.0 get a dynamic IP address. Otherwise
   check to see if the given address is available */
extern int ippool_newip(struct ippool_t *this, struct ippoolm_t **member,
			struct in_addr *addr, int statip);

/* Return a previously allocated IP address */
extern int ippool_freeip(struct ippool_t *this, struct ippoolm_t *member);

/* Get net and mask based on ascii string */
extern int ippool_aton(struct in_addr *addr, struct in_addr *mask,
		       char *pool, int number);

int ippool_hashadd(struct ippool_t *this, struct ippoolm_t *member);

int ippool_print(int fd, struct ippool_t *this);

#ifndef IPPOOL_NOIP6
extern uint32_t ippool_hash6(struct in6_addr *addr);
extern int ippool_getip6(struct ippool_t *this, struct in6_addr *addr);
extern int ippool_returnip6(struct ippool_t *this, struct in6_addr *addr);
#endif

#endif	/* !_IPPOOL_H */
