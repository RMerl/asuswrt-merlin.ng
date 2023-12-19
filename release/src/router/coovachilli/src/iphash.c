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

#include "chilli.h"
#include "iphash.h"

uint32_t iphash_hash4(struct in_addr *addr) {
  return lookup((unsigned char *)&addr->s_addr, sizeof(addr->s_addr), 0);
}

#if (0)
uint32_t iphash_hash6(struct in6_addr *addr) {
  return lookup((unsigned char *)addr->u6_addr8, sizeof(addr->u6_addr8), 0);
}
#endif

int iphash_hashadd(struct iphash_t *this, struct iphashm_t *member) {
  uint32_t hash;
  struct iphashm_t *p;
  struct iphashm_t *p_prev = 0; 

  hash = iphash_hash4(&member->addr) & this->hashmask;

  for (p = this->hash[hash]; p; p = p->nexthash)
    p_prev = p;

  if (!p_prev)
    this->hash[hash] = member;
  else 
    p_prev->nexthash = member;

  return 0;
}

int iphash_new(struct iphash_t **this, struct iphashm_t **member, int listsize, iphash_callback callback) {
  struct iphash_t *n;
  int i;

  if (!(n = calloc(sizeof(struct iphash_t), 1))) {
    log_err(0, "Failed to allocate memory for iphash");
    return -1;
  }
  
  n->callback = callback;
  n->listsize = listsize;
  n->member = member;

  for (n->hashlog = 0; 
       ((1 << n->hashlog) < listsize);
       n->hashlog++);
  
  n->hashsize = 1 << n->hashlog;
  n->hashmask = n->hashsize -1;

  n->first = 0;
  n->last = 0;

  for (i = 0; i < listsize; i++) {
    n->member[i]->prev = n->last;
    if (n->last) {
      n->last->next = n->member[i];
    }
    else {
      n->first = n->member[i];
    }
    n->last = n->member[i];
    n->member[i]->next = NULL; 
  }
  
  if (!(n->hash = calloc(sizeof(struct iphashm_t *), n->hashsize))){
    log_err(0, "Failed to allocate memory for iphash");
    free(n);
    return -1;
  }
  
  *this = n;
  return 0;
}

int iphash_get(struct iphash_t *this, struct iphashm_t **member,  struct in_addr *addr, uint16_t port) {
  struct iphashm_t *p;
  uint32_t hash;

  hash = iphash_hash4(addr) & this->hashmask;
  for (p = this->hash[hash]; p; p = p->nexthash) {
    if ( p->addr.s_addr == addr->s_addr && 
	 (p->port == 0 || p->port == port) &&
	 p->flags & IPHASH_INUSE ) {
      if (member) *member = p;
      return 0;
    }
  }
  
  if (member) *member = 0;
  return -1;
}

int iphash_hashdel(struct iphash_t *this, struct iphashm_t *member) {
  uint32_t hash;
  struct iphashm_t *p;
  struct iphashm_t *p_prev = 0; 

  hash = iphash_hash4(&member->addr) & this->hashmask;
  for (p = this->hash[hash]; p; p = p->nexthash) {
    if (p == member) {
      break;
    }
    p_prev = p;
  }

  if (p != member) {
    log_err(0, "iphash_hashdel: Tried to delete member not in hash table");
    return -1;
  }

  if (!p_prev)
    this->hash[hash] = p->nexthash;
  else
    p_prev->nexthash = p->nexthash;

  return 0;
}

int iphash_add(struct iphash_t *this, struct iphashm_t **member, struct in_addr *addr, uint16_t port) {
  struct iphashm_t *p;
  /*uint32_t hash;*/

  log_dbg("IPHASH IP: %s %d", inet_ntoa(*addr), ntohs(port));

  p = this->first;
  
  if (!p) {
    *member = 0;
    return -1;
  }

  if (p->prev) 
    p->prev->next = p->next;
  else
    this->first = p->next;

  if (p->next) 
    p->next->prev = p->prev;
  else
    this->last = p->prev;

  p->addr.s_addr = addr->s_addr;
  p->port = port;
  p->next = 0;
  p->prev = 0;
  p->flags = IPHASH_INUSE; 

  iphash_hashadd(this, p);
    
  *member = p;

  return 0;
}

int iphash_expire(struct iphash_t *this, int timeout) {
  return 0;
}

int iphash_free(struct iphash_t *this) {
  free(this->member);
  free(this->hash);
  free(this);
  return 0;
}
