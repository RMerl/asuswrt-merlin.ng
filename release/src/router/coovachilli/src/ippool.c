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

/*#define _DEBUG_PRINT_ 1*/

const unsigned int IPPOOL_STATSIZE = 0x10000;

int ippool_print(int fd, struct ippool_t *this) {
  int n;
  char line[1024];
  char useLine[16];
  char peerLine[128];

  time_t now = mainclock_now();

  char * sep = "-- %-15s ------------------------------------------------------------\n";

#define ERR 0
#define USED 1
#define FREE 2
#define LIST 3
  int dyn[4] = { 0, 0, 0, 0};
  int stat[4] = { 0, 0, 0, 0};

  safe_snprintf(line, sizeof(line),
		"DHCP lease time %d sec, grace period %d sec\n"
		"First available dynamic %d Last %d\n"
		"First available static %d Last %d\n"
		"List size %d\n",
		(int) (dhcp->lease), _options.leaseplus,
		(int) (this->firstdyn ? this->firstdyn - this->member : -1),
		(int) (this->lastdyn ? this->lastdyn - this->member : -1),
		(int) (this->firststat ? this->firststat - this->member : -1),
		(int) (this->laststat ? this->laststat - this->member : -1),
		this->listsize);
  
  safe_write(fd, line, strlen(line));
  
  safe_snprintf(line, sizeof(line), sep, "Dynamic Pool");
  safe_write(fd, line, strlen(line));

  for (n=0; n < this->listsize; n++) {
    int *st = (n >= this->dynsize) ? stat : dyn;

    if (this->member[n].in_use) {
      if (this->member[n].next == 0 && this->member[n].prev == 0) {
	st[USED]++;
      } else {
	st[ERR]++;
      }
    } else {
      if (this->member[n].next == 0 && 
	  (this->member[n].is_static ? this->laststat : this->lastdyn) != &this->member[n]) {
	st[ERR]++;
      } else if (this->member[n].prev == 0 && 
	  (this->member[n].is_static ? this->firststat : this->firstdyn) != &this->member[n]) {
	st[ERR]++;
      } else {
	st[FREE]++;
      }
    }

    if (n == this->dynsize) {
      safe_snprintf(line, sizeof(line), sep, "Static Pool");
      safe_write(fd, line, strlen(line));
    }
    
    if (this->member[n].peer) {
      struct app_conn_t *appconn = (struct app_conn_t *) this->member[n].peer;
      struct dhcp_conn_t *dhcpconn = (struct dhcp_conn_t *) appconn->dnlink;
      safe_snprintf(peerLine, sizeof(peerLine),
		    "%s mac=%.2X-%.2X-%.2X-%.2X-%.2X-%.2X ip=%s age=%d", 
		    dhcpconn ? dhcpconn->is_reserved ? " reserved" : "" : "",
		    appconn->hismac[0],appconn->hismac[1],appconn->hismac[2],
		    appconn->hismac[3],appconn->hismac[4],appconn->hismac[5],
		    inet_ntoa(appconn->hisip), 
		    dhcpconn ? ((int)(now - dhcpconn->lasttime)) : -1);
    } else {
      peerLine[0]=0;
    }

    if (this->member[n].in_use) {
      safe_snprintf(useLine, sizeof(useLine), "-inuse-");
    } else {
      safe_snprintf(useLine, sizeof(useLine), "%3d/%3d",
		    this->member[n].prev ? (int)(this->member[n].prev - this->member) : -1,
	       this->member[n].next ? (int)(this->member[n].next - this->member) : -1);
    }

    safe_snprintf(line, sizeof(line), 
		  "Unit %3d : %7s : %15s :%s%s\n", 
		  n, useLine,
		  inet_ntoa(this->member[n].addr),	
		  this->member[n].is_static ? " static" : "",
		  peerLine
		  );

    safe_write(fd, line, strlen(line));
  }

  {
    struct ippoolm_t *p = this->firstdyn;
    while (p) { dyn[LIST]++; p = p->next; }
    p = this->firststat;
    while (p) { stat[LIST]++; p = p->next; }
  }
  
  safe_snprintf(line, sizeof(line), 
		"Dynamic address: free %d, avail %d, used %d, err %d, sum %d/%d%s\n",
		dyn[FREE], dyn[LIST], dyn[USED], dyn[ERR], dyn[0]+dyn[1]+dyn[2], this->dynsize,
		dyn[FREE] != dyn[LIST] ? " - Problem!" : "");
  safe_write(fd, line, strlen(line));
  
  safe_snprintf(line, sizeof(line), 
		"Static address: free %d, avail %d, used %d, err %d, sum %d/%d%s\n",
		stat[FREE], stat[LIST], stat[USED], stat[ERR], stat[0]+stat[1]+stat[2], this->statsize,
		stat[FREE] != stat[LIST] ? " - Problem!" : "");
  safe_write(fd, line, strlen(line));

  return 0;
}

int ippool_hashadd(struct ippool_t *this, struct ippoolm_t *member) {
  uint32_t hash;
  struct ippoolm_t *p;
  struct ippoolm_t *p_prev = NULL; 

  /* Insert into hash table */
  hash = ippool_hash4(&member->addr) & this->hashmask;

  for (p = this->hash[hash]; p; p = p->nexthash)
    p_prev = p;

  if (!p_prev)
    this->hash[hash] = member;
  else 
    p_prev->nexthash = member;

  return 0; /* Always OK to insert */
}

int ippool_hashdel(struct ippool_t *this, struct ippoolm_t *member) {
  uint32_t hash;
  struct ippoolm_t *p;
  struct ippoolm_t *p_prev = NULL; 

  /* Find in hash table */
  hash = ippool_hash4(&member->addr) & this->hashmask;
  for (p = this->hash[hash]; p; p = p->nexthash) {
    if (p == member) {
      break;
    }
    p_prev = p;
  }

  if (p!= member) {
    log_err(0, "ippool_hashdel: Tried to delete member not in hash table");
    return -1;
  }

  if (!p_prev)
    this->hash[hash] = p->nexthash;
  else
    p_prev->nexthash = p->nexthash;

  return 0;
}

uint32_t ippool_hash4(struct in_addr *addr) {
  return lookup((unsigned char *)&addr->s_addr, sizeof(addr->s_addr), 0);
}

#ifndef IPPOOL_NOIP6
uint32_t ippool_hash6(struct in6_addr *addr) {
  return lookup((unsigned char *)addr->u6_addr8, sizeof(addr->u6_addr8), 0);
}
#endif

/* Create new address pool */
int ippool_new(struct ippool_t **this, 
	       char *dyn, int start, int end, char *stat, 
	       int allowdyn, int allowstat) {

  /* Parse only first instance of pool for now */

  int i;
  struct in_addr addr;
  struct in_addr mask;
  struct in_addr stataddr;
  struct in_addr statmask;
  struct in_addr naddr;
  uint32_t m;
  uint32_t listsize;
  uint32_t dynsize;
  uint32_t statsize;

  if (!allowdyn) {
    dynsize = 0;
  }
  else {
    if (option_aton(&addr, &mask, dyn, 0)) {
      log_err(0, "Failed to parse dynamic pool");
      return -1;
    }

    /* auto-dhcpstart if not already set */
    if (!start) 
      start = ntohl(addr.s_addr & ~(mask.s_addr));

    /* ensure we have the true network space */
    addr.s_addr = addr.s_addr & mask.s_addr;

    m = ntohl(mask.s_addr);
    dynsize = ((~m)+1); 

    if ( ((ntohl(addr.s_addr) + start) & m) != (ntohl(addr.s_addr) & m) ) {
      addr.s_addr = htonl(ntohl(addr.s_addr) + start);
      log_err(0, "Invalid dhcpstart=%d (%s) (outside of subnet)!",
	      start, inet_ntoa(addr));
      return -1;
    }

    if ( ((ntohl(addr.s_addr) + end) & m) != (ntohl(addr.s_addr) & m) ) {
      log_err(0, "Invalid dhcpend (outside of subnet)!");
      return -1;
    }

    if (start > 0 && end > 0) {

      if (end < start) {
	log_err(0, "Bad arguments dhcpstart=%d and dhcpend=%d", start, end);
	return -1;
      }

      if ((end - start) > dynsize) {
	log_err(0, "Too many IPs between dhcpstart=%d and dhcpend=%d",
		start, end);
	return -1;
      }

      dynsize = end - start;

    } else {

      if (start > 0) {

	/*
	 * if only dhcpstart is set, subtract that from count
	 */
	dynsize -= start;

	dynsize--;/* no broadcast */

      } else if (end > 0) {

	/*
	 * if only dhcpend is set, ensure only that many
	 */
	if (dynsize > end)
	  dynsize = end;

	dynsize--;/* no network */

      } else {
	dynsize-=2;/* no network, no broadcast */
      }

      dynsize--;/* no uamlisten */
    }
  }

  if (!allowstat) {
    statsize = 0;
    stataddr.s_addr = 0;
    statmask.s_addr = 0;
  }
  else {
    if (option_aton(&stataddr, &statmask, stat, 0)) {
      log_err(0, "Failed to parse static range");
      return -1;
    }

    /* ensure we have the true network space */
    stataddr.s_addr = stataddr.s_addr & statmask.s_addr;

    m = ntohl(statmask.s_addr);
    statsize = ((~m)+1);

    if (statsize > IPPOOL_STATSIZE)
      statsize = IPPOOL_STATSIZE;
  }

  listsize = dynsize + statsize; /* Allocate space for static IP addresses */

  if (!(*this = calloc(sizeof(struct ippool_t), 1))) {
    log_err(0, "Failed to allocate memory for ippool");
    return -1;
  }
  
  (*this)->allowdyn  = allowdyn;
  (*this)->allowstat = allowstat;
  (*this)->stataddr  = stataddr;
  (*this)->statmask  = statmask;

  (*this)->dynsize   = dynsize;
  (*this)->statsize  = statsize;
  (*this)->listsize  = listsize;

  if (!((*this)->member = calloc(sizeof(struct ippoolm_t), listsize))){
    log_err(0, "Failed to allocate memory for members in ippool");
    return -1;
  }
  
  for ((*this)->hashlog = 0; 
       ((1 << (*this)->hashlog) < listsize);
       (*this)->hashlog++);

  log_dbg("Hashlog %d %d %d", (*this)->hashlog, listsize, 
	  (1 << (*this)->hashlog));

  /* Determine hashsize */
  (*this)->hashsize = 1 << (*this)->hashlog; /* Fails if mask=0: All Internet*/
  (*this)->hashmask = (*this)->hashsize -1;
  
  /* Allocate hash table */
  if (!((*this)->hash = 
	calloc(sizeof(struct ippoolm_t *), (*this)->hashsize))){
    log_err(0, "Failed to allocate memory for hash members in ippool");
    return -1;
  }

  if (start <= 0) /* adjust for skipping network */
    start = 1; 
  
  (*this)->firstdyn = NULL;
  (*this)->lastdyn = NULL;

  for (i = 0; i < dynsize; i++) {

    naddr.s_addr = htonl(ntohl(addr.s_addr) + i + start);
    if (naddr.s_addr == _options.uamlisten.s_addr ||
	naddr.s_addr == _options.dhcplisten.s_addr) {
      start++; /* skip the uamlisten address! */
      naddr.s_addr = htonl(ntohl(addr.s_addr) + i + start);
    }

    (*this)->member[i].addr.s_addr = naddr.s_addr;
    (*this)->member[i].in_use = 0;
    (*this)->member[i].is_static = 0;

    /* Insert into list of unused */
    (*this)->member[i].prev = (*this)->lastdyn;
    if ((*this)->lastdyn) {
      (*this)->lastdyn->next = &((*this)->member[i]);
    }
    else {
      (*this)->firstdyn = &((*this)->member[i]);
    }
    (*this)->lastdyn = &((*this)->member[i]);
    (*this)->member[i].next = NULL; /* Redundant */

    ippool_hashadd(*this, &(*this)->member[i]);
  }

  (*this)->firststat = NULL;
  (*this)->laststat = NULL;
  for (i = dynsize; i < listsize; i++) {
    (*this)->member[i].addr.s_addr = 0;
    (*this)->member[i].in_use = 0;
    (*this)->member[i].is_static = 1;

    /* Insert into list of unused */
    (*this)->member[i].prev = (*this)->laststat;
    if ((*this)->laststat) {
      (*this)->laststat->next = &((*this)->member[i]);
    }
    else {
      (*this)->firststat = &((*this)->member[i]);
    }
    (*this)->laststat = &((*this)->member[i]);
    (*this)->member[i].next = NULL; /* Redundant */
  }

#ifdef _DEBUG_PRINT_
  if (_options.debug)
    ippool_print(0, *this);
#endif

  return 0;
}


/* Delete existing address pool */
int ippool_free(struct ippool_t *this) {
  free(this->hash);
  free(this->member);
  free(this);
  return 0; /* Always OK */
}

/* Find an IP address in the pool */
int ippool_getip(struct ippool_t *this, 
		 struct ippoolm_t **member,
		 struct in_addr *addr) {
  struct ippoolm_t *p;
  uint32_t hash;

  /* Find in hash table */
  hash = ippool_hash4(addr) & this->hashmask;
  for (p = this->hash[hash]; p; p = p->nexthash) {
    if ((p->addr.s_addr == addr->s_addr) && (p->in_use)) {
      if (member) *member = p;
      return 0;
    }
  }

  if (member) *member = NULL;
  return -1;
}

/**
 * ippool_newip
 * Get an IP address. If addr = 0.0.0.0 get a dynamic IP address. Otherwise
 * check to see if the given address is available. If available within
 * dynamic address space allocate it there, otherwise allocate within static
 * address space.
**/
int ippool_newip(struct ippool_t *this, 
		 struct ippoolm_t **member,
		 struct in_addr *addr, 
		 int statip) {
  struct ippoolm_t *p;
  struct ippoolm_t *p2 = NULL;
  uint32_t hash;
  
  log_dbg("Requesting new %s ip: %s", 
	  statip ? "static" : "dynamic", inet_ntoa(*addr));

  /* If static:
   *   Look in dynaddr. 
   *     If found remove from firstdyn/lastdyn linked list.
   *   Else allocate from stataddr.
   *    Remove from firststat/laststat linked list.
   *    Insert into hash table.
   *
   * If dynamic
   *   Remove from firstdyn/lastdyn linked list.
   *
   */

#ifdef _DEBUG_PRINT_
  if (_options.debug)
    ippool_print(0, this);
#endif

  /* First, check to see if this type of address is allowed */
  if ((addr) && (addr->s_addr) && statip) { /* IP address given */
#ifdef ENABLE_UAMANYIP
    if (!_options.uamanyip) {
      log_dbg("HERE!!! -> %d", _options.uamanyip);
#endif
      if (!this->allowstat) {
	log_dbg("Static IP address not allowed");
	return -1;
      }
      if ((addr->s_addr & this->statmask.s_addr) != this->stataddr.s_addr) {
	log_err(0, "Static out of range");
	return -1;
      }
#ifdef ENABLE_UAMANYIP
    }
#endif
  }
  else {
    if (!this->allowdyn) {
      log_err(0, "Dynamic IP address not allowed");
      return -1; 
    }
  }
  
  /* If IP address given try to find it in address pool */
  if ((addr) && (addr->s_addr)) { /* IP address given */
    /* Find in hash table */
    hash = ippool_hash4(addr) & this->hashmask;
    for (p = this->hash[hash]; p; p = p->nexthash) {
      if ((p->addr.s_addr == addr->s_addr)) {
	p2 = p;
	break;
      }
    }
  }

#ifdef ENABLE_UAMANYIP
  /* if anyip is set and statip return the same ip */
  if (statip && _options.uamanyip && p2 && p2->is_static) {
    log_dbg("Found already allocated static ip %s", 
	    inet_ntoa(p2->addr));
    *member = p2;
    return 0;
  }
#endif
  
  /* If IP was already allocated we can not use it */
  if ((!statip) && (p2) && (p2->in_use)) {
    p2 = NULL; 
  }
  
  /* If not found yet and dynamic IP then allocate dynamic IP */
  if ((!p2) && (!statip) /*XXX: && (!addr || !addr->s_addr)*/) {
    if (!this->firstdyn) {
      log_err(0, "No more dynamic addresses available");
      return -1;
    }
    else {
      p2 = this->firstdyn;
    }
  }
  
  if (p2) { /* Was allocated from dynamic address pool */
    
    if (p2->in_use) {
      log_err(0, "IP address already in use");
      return -1; /* Already in use / Should not happen */
    }
    
    /* Remove from linked list of free dynamic addresses */

    if (p2->is_static) {
      log_err(0, "Should not happen!");
      return -1;
    }

    if (p2->prev) 
      p2->prev->next = p2->next;
    else
      this->firstdyn = p2->next;

    if (p2->next) 
      p2->next->prev = p2->prev;
    else
      this->lastdyn = p2->prev;
    
    p2->next = NULL;
    p2->prev = NULL;
    p2->in_use = 1;
    
    *member = p2;

#ifdef _DEBUG_PRINT_
    if (_options.debug) 
      ippool_print(0, this);
#endif

    return 0; /* Success */
  }

  /* It was not possible to allocate from dynamic address pool */
  /* Try to allocate from static address space */

  if ((addr) && (addr->s_addr) && (statip 
#ifdef ENABLE_UAMANYIP
				   || _options.uamanyip
#endif
				   )) { /* IP address given */

    if (!this->firststat) {
      log_err(0, "No more static addresses available");
      return -1; /* No more available */
    }
    else {
      p2 = this->firststat;
    }
    
    /* Remove from linked list of free static addresses */

    if (p2->in_use) {
      log_err(0, "IP address already in use");
      return -1; /* Already in use / Should not happen */
    }
    
    if (!p2->is_static) {
      log_err(0, "Should not happen!");
      return -1;
    }

    if (p2->prev) 
      p2->prev->next = p2->next;
    else
      this->firststat = p2->next;

    if (p2->next) 
      p2->next->prev = p2->prev;
    else
      this->laststat = p2->prev;

    p2->next = NULL;
    p2->prev = NULL;
    p2->in_use = 1; 

    p2->addr.s_addr = addr->s_addr;

    *member = p2;

    log_dbg("Assigned a static ip to: %s", inet_ntoa(*addr));

    ippool_hashadd(this, *member);

#ifdef _DEBUG_PRINT_
    if (_options.debug) 
      ippool_print(0, this);
#endif

    return 0; /* Success */
  }

  return -1; 
}


int ippool_freeip(struct ippool_t *this, struct ippoolm_t *member) {
  
#ifdef _DEBUG_PRINT_
  if (_options.debug)
    ippool_print(0, this);
#endif

  if (!member->in_use) {
    log_err(0, "Address not in use");
    return -1; /* Not in use: Should not happen */
  }

  if (member->is_static) {
    
    if (ippool_hashdel(this, member))
      return -1;

    member->prev = this->laststat;

    if (this->laststat) {
      this->laststat->next = member;
    }
    else {
      this->firststat = member;
    }
    
    this->laststat = member;
    
    member->in_use = 0;
    member->addr.s_addr = 0;
    member->peer = NULL;
    member->nexthash = NULL;

  } else {

    member->prev = this->lastdyn;

    if (this->lastdyn) {
      this->lastdyn->next = member;
    }
    else {
      this->firstdyn = member;
    }

    this->lastdyn = member;
    
    member->in_use = 0;
    member->peer = NULL;
  }

#ifdef _DEBUG_PRINT_
  if (_options.debug)
    ippool_print(0, this);
#endif

  return 0;
}


#ifndef IPPOOL_NOIP6
extern uint32_t ippool_hash6(struct in6_addr *addr);
extern int ippool_getip6(struct ippool_t *this, struct in6_addr *addr);
extern int ippool_returnip6(struct ippool_t *this, struct in6_addr *addr);
#endif
