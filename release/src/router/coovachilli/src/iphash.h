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


#ifndef _IPHASH_H
#define _IPHASH_H

struct iphash_t;
struct iphashm_t;

typedef int (*iphash_callback)(int, struct iphash_t *, struct iphashm_t *);

struct iphash_t {
  int listsize;                  /* Total number of addresses */
  int hashsize;                  /* Size of hash table */
  int hashlog;                   /* Log2 size of hash table */
  int hashmask;                  /* Bitmask for calculating hash */
  struct iphashm_t ** member;    /* Listsize array of members */
  struct iphashm_t ** hash;      /* Hashsize array of pointer to member */
  struct iphashm_t  * first;     /* Pointer to first free member */
  struct iphashm_t  * last;      /* Pointer to last free member */
  iphash_callback callback;
};

#define IPHASH_INUSE (1<<0)

struct iphashm_t {
  struct in_addr addr;           /* IP address of this member */
  uint16_t port;                 /* port */
  uint8_t flags;                 /* flags */
  struct iphashm_t *nexthash;    /* Linked list part of hash table */
  struct iphashm_t *prev, *next; /* Linked list of free dynamic or static */
  time_t last_used;              /* Last used timestamp */
};

int iphash_new(struct iphash_t **this, struct iphashm_t **member, int listsize, iphash_callback callback);

#endif	/* !_IPHASH_H */
