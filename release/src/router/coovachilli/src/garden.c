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
#include "debug.h"

#ifdef HAVE_PATRICIA
struct node_pass_through_list {
  uint32_t ptcnt;
  pass_through ptlist[1];
};
#endif

#ifdef ENABLE_CHILLIQUERY
void garden_print_list(int fd, pass_through *ptlist, int ptcnt) {
  char mask[32];
  char line[512];
  pass_through *pt;
  int i;

  for (i = 0; i < ptcnt; i++) {
    pt = &ptlist[i];
    
    safe_strncpy(mask, inet_ntoa(pt->mask), sizeof(mask));
    
    safe_snprintf(line, sizeof(line),
		  "host=%-16s mask=%-16s proto=%-3d port=%-3d"
#ifdef ENABLE_GARDENEXT
		  " expiry=%-3d"
#endif
		  "\n",
		  inet_ntoa(pt->host), mask,
		  pt->proto, pt->port
#ifdef ENABLE_GARDENEXT
		  , pt->expiry ? pt->expiry - mainclock_now() : 0
#endif
		  );
    
    if (!safe_write(fd, line, strlen(line))) /* error */;
  }
}

#ifdef ENABLE_SESSGARDEN
int garden_print_appconn(struct app_conn_t *appconn, void *d) {
  char line[512];
  int fd = * (int *) d;
#ifdef HAVE_PATRICIA
  void cb (prefix_t *prefix, void *data) {
    struct node_pass_through_list *nd = 
      (struct node_pass_through_list *)data;
    garden_print_list(fd, nd->ptlist, nd->ptcnt);
  }
#endif
  if (appconn->s_params.pass_through_count > 0) {
    safe_snprintf(line, sizeof line, 
		  "subscriber %s (%d/%d):\n",
		  inet_ntoa(appconn->hisip),
		  appconn->s_params.pass_through_count,
		  SESSION_PASS_THROUGH_MAX);
    if (!safe_write(fd, line, strlen(line))) /* error */;
#ifdef HAVE_PATRICIA
    if (appconn->ptree) {
      patricia_process(appconn->ptree, cb);
    } else 
#endif
      garden_print_list(fd, 
			appconn->s_params.pass_throughs, 
			appconn->s_params.pass_through_count);
  }
  return 0;
}
#endif

void garden_print(int fd) {
  char line[512];

#ifdef HAVE_PATRICIA
  void cb (prefix_t *prefix, void *data) {
    struct node_pass_through_list *nd = 
      (struct node_pass_through_list *)data;
    garden_print_list(fd, nd->ptlist, nd->ptcnt);
  }
#endif
  
  safe_snprintf(line, sizeof line, 
		"static garden (%d/%d):\n",
		_options.num_pass_throughs,
		MAX_PASS_THROUGHS);
  if (!safe_write(fd, line, strlen(line))) /* error */;

#ifdef HAVE_PATRICIA
  if (dhcp->ptree) {
    patricia_process(dhcp->ptree, cb);
  } else 
#endif
    garden_print_list(fd, 
		      _options.pass_throughs, 
		      _options.num_pass_throughs);

  safe_snprintf(line, sizeof line, 
		"dynamic garden (%d/%d):\n",
		dhcp->num_pass_throughs,
		MAX_PASS_THROUGHS);
  if (!safe_write(fd, line, strlen(line))) /* error */;
  
#ifdef HAVE_PATRICIA
  if (dhcp->ptree_dyn) {
    patricia_process(dhcp->ptree_dyn, cb);
  } else 
#endif
    garden_print_list(fd, 
		      dhcp->pass_throughs, 
		      dhcp->num_pass_throughs);

#ifdef ENABLE_SESSGARDEN
  chilli_appconn_run(garden_print_appconn, &fd);
#endif
}
#endif

#ifdef HAVE_PATRICIA
int garden_patricia_print(int fd, patricia_tree_t *ptree) {
  return 0;
}

int garden_patricia_check(patricia_tree_t *ptree, 
			  pass_through *ptlist, uint32_t *ptcnt,
			  struct pkt_ipphdr_t *ipph, int dst) {
  int found = 0;
  prefix_t *prefix;
  patricia_node_t *pfx;
  struct in_addr sin;
  
  sin.s_addr = dst ? ipph->daddr : ipph->saddr;
  prefix = patricia_prefix_new (AF_INET, &sin, 32);
  
  pfx = patricia_search_best(ptree, prefix);
  if (pfx) {
    struct node_pass_through_list *
      nd = PATRICIA_DATA_GET(pfx, struct node_pass_through_list);

    if (nd) {
      pass_through *pt=0;
      switch (garden_check(nd->ptlist, &nd->ptcnt, 
			   &pt, ipph, dst, ptree)) {
      case 1:
	found = 1;
	break;
      case -1:
	if (pt)
	  pass_through_rem(ptlist, ptcnt, pt, ptree);
	break;
      }
    }
  }

  patricia_prefix_deref (prefix);
  return found;
}

int garden_patricia_add(pass_through *pt, patricia_tree_t *ptree) {
  uint32_t mask;
  unsigned char count;
  prefix_t *prefix;
  patricia_node_t *pfx;
  struct in_addr sin;

  for (count = 0, mask = 0x80000000; mask != 0; mask >>= 1) {
    if (pt->mask.s_addr & mask)
      count++;
  }

  sin.s_addr = pt->host.s_addr;
  prefix = patricia_prefix_new (AF_INET, &sin, count);

  pfx = patricia_lookup (ptree, prefix);

  if (pfx != NULL) {
    struct node_pass_through_list *
      nd = PATRICIA_DATA_GET(pfx, struct node_pass_through_list);

    if (nd == NULL) {
      nd = (struct node_pass_through_list *)
	malloc(sizeof(struct node_pass_through_list)+sizeof(pass_through));
      if (nd) {
	nd->ptcnt = 1;
	memcpy(nd->ptlist, pt, sizeof(*pt));
      }
    } else {
      int i;
      for (i=0; i < nd->ptcnt; i++) {
	if (pt_equal(&nd->ptlist[i], pt)) {
	  log_dbg("Uamallowed already exists #%d:%d: proto=%d host=%s port=%d", 
		  i, nd->ptcnt, pt->proto, inet_ntoa(pt->host), pt->port);
	  break;
	}
      }
      if (i == nd->ptcnt) {
	nd->ptcnt++;
	nd = realloc(nd, 
		     sizeof(struct node_pass_through_list)+
		     (sizeof(pass_through)*nd->ptcnt));
	memcpy(&nd->ptlist[nd->ptcnt-1], pt, sizeof(*pt));
      }
    }
    
    PATRICIA_DATA_SET(pfx, nd);
  }

  patricia_prefix_deref (prefix);
  return 0;
}

int garden_patricia_rem(pass_through *pt, patricia_tree_t *ptree) {
  uint32_t mask;
  unsigned char count;
  prefix_t *prefix;
  patricia_node_t *pfx;
  struct in_addr sin;

  for (count = 0, mask = 0x80000000; mask != 0; mask >>= 1) {
    if (pt->mask.s_addr & mask)
      count++;
  }

  sin.s_addr = pt->host.s_addr;
  prefix = patricia_prefix_new (AF_INET, &sin, count);

  pfx = patricia_search_exact (ptree, prefix);
  if (pfx != NULL) {
    struct node_pass_through_list *
      nd = PATRICIA_DATA_GET(pfx, struct node_pass_through_list);

    if (nd != NULL) {
      int i;

      for (i=0; i < nd->ptcnt; i++) {
	if (pt_equal(&nd->ptlist[i], pt)) {
	  log_dbg("Uamallowed removing #%d:%d: proto=%d host=%s port=%d", 
		  i, nd->ptcnt, pt->proto, inet_ntoa(pt->host), pt->port);

	  log_dbg("Shifting uamallowed list %d to %d", i, nd->ptcnt);

	  for (; i < nd->ptcnt-1; i++) 
	    memcpy(&nd->ptlist[i], &nd->ptlist[i+1], sizeof(pass_through));

	  nd->ptcnt--;

	  if (nd->ptcnt > 0) {

	    nd = realloc(nd, 
			 sizeof(struct node_pass_through_list)+
			 (sizeof(pass_through)*nd->ptcnt));

	    PATRICIA_DATA_SET(pfx, nd);

	  } else {
	    free(nd);
	    patricia_remove (ptree, pfx);
	  }

	  break;
	}
      }
    }
  }

  patricia_prefix_deref (prefix);
  return 0;
}

void garden_patricia_load_list(patricia_tree_t **pptree,
			       pass_through *ptlist,
			       uint32_t ptcnt) {
  patricia_tree_t *ptree = *pptree;
  int i;
  if (ptree) {
    patricia_destroy (ptree, free);
  }
  *pptree = ptree = patricia_new(32);
  for (i=0; i < ptcnt; i++)
    garden_patricia_add(&ptlist[i], ptree);
}

void garden_patricia_reload() {
  if (_options.patricia) {
    garden_patricia_load_list(&dhcp->ptree, 
			      _options.pass_throughs, 
			      _options.num_pass_throughs);
  }
}
#endif

int garden_check(pass_through *ptlist, uint32_t *pcnt,
		 pass_through **pt_match,
		 struct pkt_ipphdr_t *ipph, int dst
#ifdef HAVE_PATRICIA
		 , patricia_tree_t *ptree
#endif
		 ) {
  uint32_t ptcnt = *pcnt;
  pass_through *pt;
  int i;

  for (i = 0; i < ptcnt; i++) {
    pt = &ptlist[i];
    if (pt->proto == 0 || ipph->protocol == pt->proto)
      if (pt->host.s_addr == 0 || 
	  pt->host.s_addr == 
	  ((dst ? ipph->daddr : ipph->saddr) & pt->mask.s_addr))
	if (pt->port == 0 || 
	    ((ipph->protocol == PKT_IP_PROTO_TCP ||
	      ipph->protocol == PKT_IP_PROTO_UDP) && 
	     (dst ? ipph->dport : ipph->sport) == htons(pt->port))) {
	  if (pt_match) *pt_match = pt;
#ifdef ENABLE_GARDENEXT
	  if (pt->expiry && pt->expiry < mainclock_now()) {
	    return -1;
	  }
#endif
	  return 1;
	}
  }
  
  return 0;
}

int pass_through_rem(pass_through *ptlist, uint32_t *ptcnt, 
		     pass_through *pt
#ifdef HAVE_PATRICIA
		     , patricia_tree_t *ptree
#endif
		     ) {
  uint32_t cnt = *ptcnt;
  int i;

  for (i=0; i < cnt; i++) {
    if (pt_equal(&ptlist[i], pt)) {
      log_dbg("Uamallowed removing #%d: proto=%d host=%s port=%d", 
	      i, pt->proto, inet_ntoa(pt->host), pt->port);
      log_dbg("Shifting uamallowed list %d to %d", i, cnt);
      for (; i < cnt-1; i++) 
	memcpy(&ptlist[i], &ptlist[i+1], sizeof(pass_through));
      *ptcnt = *ptcnt - 1;
      break;
    }
  }

#ifdef HAVE_PATRICIA
  if (ptree)
    garden_patricia_rem(pt, ptree);
#endif

  return 0;
}

int pass_through_add(pass_through *ptlist, uint32_t ptlen,
		     uint32_t *ptcnt, pass_through *pt,
		     char is_dyn
#ifdef HAVE_PATRICIA
		     , patricia_tree_t *ptree
#endif
		     ) {
  uint32_t cnt = *ptcnt;
  int i;

  for (i=0; i < cnt; i++) {
    if (pt_equal(&ptlist[i], pt)) {
      log_dbg("Uamallowed already exists #%d:%d: proto=%d host=%s port=%d", 
	      i, ptlen, pt->proto, inet_ntoa(pt->host), pt->port);
      if (is_dyn) { 
	log_dbg("Shifting uamallowed list %d to %d", i, cnt);
	for (; i<cnt-1; i++) 
	  memcpy(&ptlist[i], &ptlist[i+1], sizeof(pass_through));
	cnt = *ptcnt = *ptcnt - 1;
	break;
      } else {
	return 0;
      }
    }
  }
            
  if (cnt == ptlen) {
    if (!is_dyn) {
      log_dbg("No more room for walled garden entries");
      return -1;
    }

    log_dbg("Shifting uamallowed list %d to %d", i, ptlen);
    for (i=0; i<ptlen-1; i++) 
      memcpy(&ptlist[i], &ptlist[i+1], sizeof(pass_through));

    cnt = *ptcnt = *ptcnt - 1;
  }

  log_dbg("Uamallowed IP address #%d:%d: proto=%d host=%s port=%d", 
	  cnt, ptlen, pt->proto, inet_ntoa(pt->host), pt->port);
  
  memcpy(&ptlist[cnt], pt, sizeof(pass_through));
  *ptcnt = cnt + 1;

#ifdef HAVE_PATRICIA
  if (ptree)
    garden_patricia_add(pt, ptree);
#endif

  return 0;
}

int pass_throughs_from_string(pass_through *ptlist, uint32_t ptlen, 
			      uint32_t *ptcnt, char *s,
			      char is_dyn, char is_rem
#ifdef HAVE_PATRICIA
			      , patricia_tree_t *ptree
#endif
			      ) {
  struct hostent *host;
  pass_through pt;
  char *t, *p1 = NULL, *p2 = NULL;
  char *p3 = malloc(strlen(s)+1);

  strcpy(p3, s);
  p1 = p3;
  
  if (_options.debug) 
    log_dbg("Uamallowed %s", s);
  
  for ( ; p1; p1 = p2) {
    
    /* save the next entry position */
    if ((p2 = strchr(p1, ','))) { *p2=0; p2++; }
    
    /* clear the pass-through entry in case we partitially filled it already */
    memset(&pt, 0, sizeof(pass_through));
    
    /* eat whitespace */
    while (isspace((int) *p1)) p1++;
    
    /* look for specific protocols */
    if ((t = strchr(p1, ':'))) { 
      int pnum = 0;

      *t = 0;

#ifdef HAVE_GETPROTOENT      
      if (1) {
	struct protoent *proto = getprotobyname(p1);

	if (!proto && !strchr(p1, '.')) 
	  proto = getprotobynumber(atoi(p1));

	if (proto) 
	  pnum = proto->p_proto;
      }
#else
      if      (!strcmp(p1,"tcp"))  { pnum = 6;  }
      else if (!strcmp(p1,"udp"))  { pnum = 17; }
      else if (!strcmp(p1,"icmp")) { pnum = 1;  }
#endif

      if (pnum > 0) {
	/* if a protocol, skip ahead */
	pt.proto = pnum;
	p1 = t + 1;
      } else {
	/* if not a protocol, put the ':' back */
	*t = ':';
      }
    }
    
#ifdef ENABLE_GARDENEXT
    {
      char *e = strchr(p1, '#');
      if (e) {
	int add = atoi(e+1);
	pt.expiry = mainclock_now() + add;
	*e = 0;
      }
    }
#endif
    
    /* look for an optional port */
    if ((t = strchr(p1, ':'))) { 
      pt.port = atoi(t+1); 
      *t = 0; 
    }

    if (strchr(p1, '/')) {	/* parse a network address */
      if (option_aton(&pt.host, &pt.mask, p1, 0)) {
	log_err(0, "Invalid uamallowed network address or mask %s!", s);
	continue;
      } 
      if (is_rem) {
	if (pass_through_rem(ptlist, ptcnt, &pt
#ifdef HAVE_PATRICIA
			     , ptree
#endif
			     ))
	  log_err(0, "Too many pass-throughs! skipped %s", s);
      } else {
	if (pass_through_add(ptlist, ptlen, ptcnt, &pt, is_dyn
#ifdef HAVE_PATRICIA
			     , ptree
#endif
			     ))
	  log_err(0, "Too many pass-throughs! skipped %s", s);
      }
    }
    else {	/* otherwise, parse a host ip or hostname */
      int j = 0;
      pt.mask.s_addr = 0xffffffff;

      if (!(host = gethostbyname(p1))) {
	log_err(errno, "Invalid uamallowed domain or address: %s!", p1);
	continue;
      }

      while (host->h_addr_list[j] != NULL) {
	pt.host = *((struct in_addr *) host->h_addr_list[j++]);
	if (is_rem) {
	  if (pass_through_rem(ptlist, ptcnt, &pt
#ifdef HAVE_PATRICIA
			       , ptree
#endif
			       ))
	    log_err(0, "Too many pass-throughs! skipped %s", s);
	} else {
	  if (pass_through_add(ptlist, ptlen, ptcnt, &pt, is_dyn
#ifdef HAVE_PATRICIA
			       , ptree
#endif
			       ))
	    log_err(0, "Too many pass-throughs! skipped %s", s);
	}
      }
    }
  }

  free(p3);
  return 0;
}

#ifdef ENABLE_CHILLIREDIR
int regex_pass_throughs_from_string(regex_pass_through *ptlist, uint32_t ptlen, 
				    uint32_t *ptcnt, char *s,
				    char is_dyn) {
  uint32_t cnt = *ptcnt;
  regex_pass_through pt;
  char *p, *st;
  int stage = 0;
  
  memset(&pt, 0, sizeof(pt));
  
  for (st = s; (p = strtok(st, "::")); st = 0, stage++) {
    int is_wild = !strcmp(p,"*");
    if (!is_wild) {
      int is_negate = (*p == '!');
      if (is_negate) p++;
      switch (stage) {
      case 0: 
	safe_strncpy(pt.regex_host, p, sizeof(pt.regex_host)); 
	pt.neg_host = is_negate; 
	break;
      case 1:
	safe_strncpy(pt.regex_path, p, sizeof(pt.regex_path)); 
	pt.neg_path = is_negate; 
	break;
      case 2: 
	safe_strncpy(pt.regex_qs, p, sizeof(pt.regex_qs));   
	pt.neg_qs   = is_negate; 
	break;
      }
    }
  }

  pt.inuse = 1;
  memcpy(&ptlist[cnt], &pt, sizeof(pt));
  *ptcnt = cnt + 1;
  return 0;
}
#endif

#ifdef ENABLE_UAMDOMAINFILE

typedef struct uamdomain_regex_t {
  regex_t re;
  char neg;
  struct uamdomain_regex_t *next;
} uamdomain_regex;

static uamdomain_regex * _list_head = 0;

void garden_free_domainfile() {
  while (_list_head) {
    uamdomain_regex * n = _list_head;
    _list_head = _list_head->next;
    regfree(&n->re);
    free(n);
  }
}

void garden_load_domainfile() {
  garden_free_domainfile();
  if (!_options.uamdomainfile) return;
  else {
    char * line = 0;
    size_t len = 0;
    ssize_t read;
    FILE* fp;

    uamdomain_regex * uam_end = 0;

    fp = fopen(_options.uamdomainfile, "r");
    if (!fp) { 
      log_err(errno, "could not open file %s", 
	      _options.uamdomainfile); 
      return; 
    }
    
    while ((read = getline(&line, &len, fp)) != -1) {
      if (read <= 0) continue;
      else if (!line[0] || line[0] == '#' || 
	       isspace((int) line[0])) continue;
      else {
	
	uamdomain_regex * uam_re = (uamdomain_regex *)
	  calloc(sizeof(uamdomain_regex), 1);

	char * pline = line;
	
	while (isspace((int) pline[read-1]))
	  pline[--read] = 0;

	if (pline[0] == '!') {
	  uam_re->neg = 1;
	  pline++;
	}
	
	log_dbg("compiling %s", pline);
	if (regcomp(&uam_re->re, pline, REG_EXTENDED | REG_NOSUB)) {
	  log_err(0, "could not compile regex %s", line);
	  free(uam_re);
	  continue;
	}
	
	if (uam_end) {
	  uam_end->next = uam_re;
	  uam_end = uam_re;
	} else {
	  _list_head = uam_end = uam_re;
	}
      }
    }	
    
    fclose(fp);
    
    if (line)
      free(line);
  }
}

int garden_check_domainfile(char *question) {
  uamdomain_regex * uam_re = _list_head;
  
  while (uam_re) {
    int match = !regexec(&uam_re->re, question, 0, 0, 0);
    
#if(_debug_)
    if (match)
      log_dbg("matched DNS name %s", question);
#endif

    if (match) return uam_re->neg ? 0 : 1;
    
    uam_re = uam_re->next;
  }

  return -1;
}

#endif
