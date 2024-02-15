/* dnsmasq is Copyright (c) 2000-2024 Simon Kelley

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 dated June, 1991, or
   (at your option) version 3 dated 29 June, 2007.
 
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
     
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "dnsmasq.h"

static int order(char *qdomain, size_t qlen, struct server *serv);
static int order_qsort(const void *a, const void *b);
static int order_servers(struct server *s, struct server *s2);

/* If the server is USE_RESOLV or LITERAL_ADDRES, it lives on the local_domains chain. */
#define SERV_IS_LOCAL (SERV_USE_RESOLV | SERV_LITERAL_ADDRESS)

void build_server_array(void)
{
  struct server *serv;
  int count = 0;
  
  for (serv = daemon->servers; serv; serv = serv->next)
#ifdef HAVE_LOOP
    if (!(serv->flags & SERV_LOOP))
#endif
      {
	count++;
	if (serv->flags & SERV_WILDCARD)
	  daemon->server_has_wildcard = 1;
      }
  
  for (serv = daemon->local_domains; serv; serv = serv->next)
    {
      count++;
      if (serv->flags & SERV_WILDCARD)
	daemon->server_has_wildcard = 1;
    }
  
  daemon->serverarraysz = count;

  if (count > daemon->serverarrayhwm)
    {
      struct server **new;

      count += 10; /* A few extra without re-allocating. */

      if ((new = whine_malloc(count * sizeof(struct server *))))
	{
	  if (daemon->serverarray)
	    free(daemon->serverarray);
	  
	  daemon->serverarray = new;
	  daemon->serverarrayhwm = count;
	}
    }

  count = 0;
  
  for (serv = daemon->servers; serv; serv = serv->next)
#ifdef HAVE_LOOP
    if (!(serv->flags & SERV_LOOP))
#endif
      {
	daemon->serverarray[count] = serv;
	serv->serial = count;
	serv->last_server = -1;
	count++;
      }
  
  for (serv = daemon->local_domains; serv; serv = serv->next, count++)
    daemon->serverarray[count] = serv;
  
  qsort(daemon->serverarray, daemon->serverarraysz, sizeof(struct server *), order_qsort);
  
  /* servers need the location in the array to find all the whole
     set of equivalent servers from a pointer to a single one. */
  for (count = 0; count < daemon->serverarraysz; count++)
    if (!(daemon->serverarray[count]->flags & SERV_IS_LOCAL))
      daemon->serverarray[count]->arrayposn = count;
}

/* we're looking for the server whose domain is the longest exact match
   to the RH end of qdomain, or a local address if the flags match.
   Add '.' to the LHS of the query string so
   server=/.example.com/ works.

   A flag of F_SERVER returns an upstream server only.
   A flag of F_DNSSECOK returns a DNSSEC capable server only and
   also disables NODOTS servers from consideration.
   A flag of F_DOMAINSRV returns a domain-specific server only.
   A flag of F_CONFIG returns anything that generates a local
   reply of IPv4 or IPV6.
   return 0 if nothing found, 1 otherwise.
*/
int lookup_domain(char *domain, int flags, int *lowout, int *highout)
{
  int rc, crop_query, nodots;
  ssize_t qlen;
  int try, high, low = 0;
  int nlow = 0, nhigh = 0;
  char *cp, *qdomain = domain;

  /* may be no configured servers. */
  if (daemon->serverarraysz == 0)
    return 0;
  
  /* find query length and presence of '.' */
  for (cp = qdomain, nodots = 1, qlen = 0; *cp; qlen++, cp++)
    if (*cp == '.')
      nodots = 0;

  /* Handle empty name, and searches for DNSSEC queries without
     diverting to NODOTS servers. */
  if (qlen == 0 || flags & F_DNSSECOK)
    nodots = 0;

  /* Search shorter and shorter RHS substrings for a match */
  while (qlen >= 0)
    {
      /* Note that when we chop off a label, all the possible matches
	 MUST be at a larger index than the nearest failing match with one more
	 character, since the array is sorted longest to smallest. Hence 
	 we don't reset low to zero here, we can go further below and crop the 
	 search string to the size of the largest remaining server
	 when this match fails. */
      high = daemon->serverarraysz;
      crop_query = 1;
      
      /* binary search */
      while (1) 
	{
	  try = (low + high)/2;

	  if ((rc = order(qdomain, qlen, daemon->serverarray[try])) == 0)
	    break;
	  
	  if (rc < 0)
	    {
	      if (high == try)
		{
		  /* qdomain is longer or same length as longest domain, and try == 0 
		     crop the query to the longest domain. */
		  crop_query = qlen - daemon->serverarray[try]->domain_len;
		  break;
		}
	      high = try;
	    }
	  else
	    {
	      if (low == try)
		{
		  /* try now points to the last domain that sorts before the query, so 
		     we know that a substring of the query shorter than it is required to match, so
		     find the largest domain that's shorter than try. Note that just going to
		     try+1 is not optimal, consider searching bbb in (aaa,ccc,bb). try will point
		     to aaa, since ccc sorts after bbb, but the first domain that has a chance to 
		     match is bb. So find the length of the first domain later than try which is
		     is shorter than it. 
		     There's a nasty edge case when qdomain sorts before _any_ of the 
		     server domains, where try _doesn't point_ to the last domain that sorts
		     before the query, since no such domain exists. In that case, the loop 
		     exits via the rc < 0 && high == try path above and this code is
		     not executed. */
		  ssize_t len, old = daemon->serverarray[try]->domain_len;
		  while (++try != daemon->serverarraysz)
		    {
		      if (old != (len = daemon->serverarray[try]->domain_len))
			{
			  crop_query = qlen - len;
			  break;
			}
		    }
		  break;
		}
	      low = try;
	    }
	};
      
      if (rc == 0)
	{
	  int found = 1;

	  if (daemon->server_has_wildcard)
	    {
	      /* if we have example.com and *example.com we need to check against *example.com, 
		 but the binary search may have found either. Use the fact that example.com is sorted before *example.com
		 We favour example.com in the case that both match (ie www.example.com) */
	      while (try != 0 && order(qdomain, qlen, daemon->serverarray[try-1]) == 0)
		try--;
	      
	      if (!(qdomain == domain || *qdomain == 0 || *(qdomain-1) == '.'))
		{
		  while (try < daemon->serverarraysz-1 && order(qdomain, qlen, daemon->serverarray[try+1]) == 0)
		    try++;
		  
		  if (!(daemon->serverarray[try]->flags & SERV_WILDCARD))
		     found = 0;
		}
	    }
	  
	  if (found && filter_servers(try, flags, &nlow, &nhigh))
	    /* We have a match, but it may only be (say) an IPv6 address, and
	       if the query wasn't for an AAAA record, it's no good, and we need
	       to continue generalising */
	    {
	      /* We've matched a setting which says to use servers without a domain.
		 Continue the search with empty query. We set the F_SERVER flag
		 so that --address=/#/... doesn't match. */
	      if (daemon->serverarray[nlow]->flags & SERV_USE_RESOLV)
		{
		  crop_query = qlen;
		  flags |= F_SERVER;
		}
	      else
		break;
	    }
	}
      
      /* crop_query must be at least one always. */
      if (crop_query == 0)
	crop_query = 1;

      /* strip chars off the query based on the largest possible remaining match,
	 then continue to the start of the next label unless we have a wildcard
	 domain somewhere, in which case we have to go one at a time. */
      qlen -= crop_query;
      qdomain += crop_query;
      if (!daemon->server_has_wildcard)
	while (qlen > 0 &&  (*(qdomain-1) != '.'))
	  qlen--, qdomain++;
    }

  /* domain has no dots, and we have at least one server configured to handle such,
     These servers always sort to the very end of the array. 
     A configured server eg server=/lan/ will take precdence. */
  if (nodots &&
      (daemon->serverarray[daemon->serverarraysz-1]->flags & SERV_FOR_NODOTS) &&
      (nlow == nhigh || daemon->serverarray[nlow]->domain_len == 0))
    filter_servers(daemon->serverarraysz-1, flags, &nlow, &nhigh);
  
  if (lowout)
    *lowout = nlow;
  
  if (highout)
    *highout = nhigh;

  /* qlen == -1 when we failed to match even an empty query, if there are no default servers. */
  if (nlow == nhigh || qlen == -1)
    return 0;
  
  return 1;
}

/* Return first server in group of equivalent servers; this is the "master" record. */
int server_samegroup(struct server *a, struct server *b)
{
  return order_servers(a, b) == 0;
}

int filter_servers(int seed, int flags, int *lowout, int *highout)
{
  int nlow = seed, nhigh = seed;
  int i;
  
  /* expand nlow and nhigh to cover all the records with the same domain 
     nlow is the first, nhigh - 1 is the last. nlow=nhigh means no servers,
     which can happen below. */
  while (nlow > 0 && order_servers(daemon->serverarray[nlow-1], daemon->serverarray[nlow]) == 0)
    nlow--;
  
  while (nhigh < daemon->serverarraysz-1 && order_servers(daemon->serverarray[nhigh], daemon->serverarray[nhigh+1]) == 0)
    nhigh++;
  
  nhigh++;
  
#define SERV_LOCAL_ADDRESS (SERV_6ADDR | SERV_4ADDR | SERV_ALL_ZEROS)
  
  if (flags & F_CONFIG)
    {
      /* We're just lookin for any matches that return an RR. */
      for (i = nlow; i < nhigh; i++)
	if (daemon->serverarray[i]->flags & SERV_LOCAL_ADDRESS)
	  break;
      
      /* failed, return failure. */
      if (i == nhigh)
	nhigh = nlow;
    }
  else
    {
      /* Now the servers are on order between low and high, in the order
	 IPv6 addr, IPv4 addr, return zero for both, resolvconf servers, send upstream, no-data return.
	 
	 See which of those match our query in that priority order and narrow (low, high) */
      
      for (i = nlow; i < nhigh && (daemon->serverarray[i]->flags & SERV_6ADDR); i++);
      
      if (!(flags & F_SERVER) && i != nlow && (flags & F_IPV6))
	nhigh = i;
      else
	{
	  nlow = i;
	  
	  for (i = nlow; i < nhigh && (daemon->serverarray[i]->flags & SERV_4ADDR); i++);
	  
	  if (!(flags & F_SERVER) && i != nlow && (flags & F_IPV4))
	    nhigh = i;
	  else
	    {
	      nlow = i;
	      
	      for (i = nlow; i < nhigh && (daemon->serverarray[i]->flags & SERV_ALL_ZEROS); i++);
	      
	      if (!(flags & F_SERVER) && i != nlow && (flags & (F_IPV4 | F_IPV6)))
		nhigh = i;
	      else
		{
		  nlow = i;
		  
		  /* Short to resolv.conf servers */
		  for (i = nlow; i < nhigh && (daemon->serverarray[i]->flags & SERV_USE_RESOLV); i++);
		  
		  if (i != nlow)
		    nhigh = i;
		  else
		    {
		      /* now look for a server */
		      for (i = nlow; i < nhigh && !(daemon->serverarray[i]->flags & SERV_LITERAL_ADDRESS); i++);
		      
		      if (i != nlow)
			{
			  /* If we want a server that can do DNSSEC, and this one can't, 
			     return nothing, similarly if were looking only for a server
			     for a particular domain. */
			  if ((flags & F_DNSSECOK) && !(daemon->serverarray[nlow]->flags & SERV_DO_DNSSEC))
			    nlow = nhigh;
			  else if ((flags & F_DOMAINSRV) && daemon->serverarray[nlow]->domain_len == 0)
			    nlow = nhigh;
			  else
			    nhigh = i;
			}
		      else
			{
			  /* --local=/domain/, only return if we don't need a server. */
			  if (flags & (F_DNSSECOK | F_DOMAINSRV | F_SERVER))
			    nhigh = i;
			}
		    }
		}
	    }
	}
    }

  *lowout = nlow;
  *highout = nhigh;
  
  return (nlow != nhigh);
}

int is_local_answer(time_t now, int first, char *name)
{
  int flags = 0;
  int rc = 0;
  
  if ((flags = daemon->serverarray[first]->flags) & SERV_LITERAL_ADDRESS)
    {
      if (flags & SERV_4ADDR)
	rc = F_IPV4;
      else if (flags & SERV_6ADDR)
	rc = F_IPV6;
      else if (flags & SERV_ALL_ZEROS)
	rc = F_IPV4 | F_IPV6;
      else
	{
	  /* argument first is the first struct server which matches the query type;
	     now roll back to the server which is just the same domain, to check if that 
	     provides an answer of a different type. */

	  for (;first > 0 && order_servers(daemon->serverarray[first-1], daemon->serverarray[first]) == 0; first--);
	  
	  if ((daemon->serverarray[first]->flags & SERV_LOCAL_ADDRESS) ||
	      check_for_local_domain(name, now))
	    rc = F_NOERR;
	  else
	    rc = F_NXDOMAIN;
	}
    }

  return rc;
}

size_t make_local_answer(int flags, int gotname, size_t size, struct dns_header *header, char *name, char *limit, int first, int last, int ede)
{
  int trunc = 0, anscount = 0;
  unsigned char *p;
  int start;
  union all_addr addr;
  
  if (flags & (F_NXDOMAIN | F_NOERR))
    log_query(flags | gotname | F_NEG | F_CONFIG | F_FORWARD, name, NULL, NULL, 0);
	  
  setup_reply(header, flags, ede);
	  
  if (!(p = skip_questions(header, size)))
    return 0;
	  
  if (flags & gotname & F_IPV4)
    for (start = first; start != last; start++)
      {
	struct serv_addr4 *srv = (struct serv_addr4 *)daemon->serverarray[start];

	if (srv->flags & SERV_ALL_ZEROS)
	  memset(&addr, 0, sizeof(addr));
	else
	  addr.addr4 = srv->addr;
	
	if (add_resource_record(header, limit, &trunc, sizeof(struct dns_header), &p, daemon->local_ttl, NULL, T_A, C_IN, "4", &addr))
	  anscount++;
	log_query((flags | F_CONFIG | F_FORWARD) & ~F_IPV6, name, (union all_addr *)&addr, NULL, 0);
      }
  
  if (flags & gotname & F_IPV6)
    for (start = first; start != last; start++)
      {
	struct serv_addr6 *srv = (struct serv_addr6 *)daemon->serverarray[start];

	if (srv->flags & SERV_ALL_ZEROS)
	  memset(&addr, 0, sizeof(addr));
	else
	  addr.addr6 = srv->addr;
	
	if (add_resource_record(header, limit, &trunc, sizeof(struct dns_header), &p, daemon->local_ttl, NULL, T_AAAA, C_IN, "6", &addr))
	  anscount++;
	log_query((flags | F_CONFIG | F_FORWARD) & ~F_IPV4, name, (union all_addr *)&addr, NULL, 0);
      }

  if (trunc)
    header->hb3 |= HB3_TC;
  header->ancount = htons(anscount);
  
  return p - (unsigned char *)header;
}

#ifdef HAVE_DNSSEC
int dnssec_server(struct server *server, char *keyname, int *firstp, int *lastp)
{
  int first, last, index;

  /* Find server to send DNSSEC query to. This will normally be the 
     same as for the original query, but may be another if
     servers for domains are involved. */		      
  if (!lookup_domain(keyname, F_DNSSECOK, &first, &last))
    return -1;

  for (index = first; index != last; index++)
    if (daemon->serverarray[index] == server)
      break;
	      
  /* No match to server used for original query.
     Use newly looked up set. */
  if (index == last)
    index =  daemon->serverarray[first]->last_server == -1 ?
      first : daemon->serverarray[first]->last_server;

  if (firstp)
    *firstp = first;

  if (lastp)
    *lastp = last;
   
  return index;
}
#endif

/* order by size, then by dictionary order */
static int order(char *qdomain, size_t qlen, struct server *serv)
{
  size_t dlen = 0;
    
  /* servers for dotless names always sort last 
     searched for name is never dotless. */
  if (serv->flags & SERV_FOR_NODOTS)
    return -1;

  dlen = serv->domain_len;
  
  if (qlen < dlen)
    return 1;
  
  if (qlen > dlen)
    return -1;

  return hostname_order(qdomain, serv->domain);
}

static int order_servers(struct server *s1, struct server *s2)
{
  int rc;

  /* need full comparison of dotless servers in 
     order_qsort() and filter_servers() */

  if (s1->flags & SERV_FOR_NODOTS)
     return (s2->flags & SERV_FOR_NODOTS) ? 0 : 1;
   
  if ((rc = order(s1->domain, s1->domain_len, s2)) != 0)
    return rc;

  /* For identical domains, sort wildcard ones first */
  if (s1->flags & SERV_WILDCARD)
    return (s2->flags & SERV_WILDCARD) ? 0 : 1;

  return (s2->flags & SERV_WILDCARD) ? -1 : 0;
}
  
static int order_qsort(const void *a, const void *b)
{
  int rc;
  
  struct server *s1 = *((struct server **)a);
  struct server *s2 = *((struct server **)b);
  
  rc = order_servers(s1, s2);

  /* Sort all literal NODATA and local IPV4 or IPV6 responses together,
     in a very specific order. We flip the SERV_LITERAL_ADDRESS bit
     so the order is IPv6 literal, IPv4 literal, all-zero literal, 
     unqualified servers, upstream server, NXDOMAIN literal. */
  if (rc == 0)
    rc = ((s2->flags & (SERV_LITERAL_ADDRESS | SERV_4ADDR | SERV_6ADDR | SERV_USE_RESOLV | SERV_ALL_ZEROS)) ^ SERV_LITERAL_ADDRESS) -
      ((s1->flags & (SERV_LITERAL_ADDRESS | SERV_4ADDR | SERV_6ADDR | SERV_USE_RESOLV | SERV_ALL_ZEROS)) ^ SERV_LITERAL_ADDRESS);

  /* Finally, order by appearance in /etc/resolv.conf etc, for --strict-order */
  if (rc == 0)
    if (!(s1->flags & SERV_LITERAL_ADDRESS))
      rc = s1->serial - s2->serial;

  return rc;
}


/* When loading large numbers of server=.... lines during startup,
   there's no possibility that there will be server records that can be reused, but
   searching a long list for each server added grows as O(n^2) and slows things down.
   This flag is set only if is known there may be free server records that can be reused.
   There's a call to mark_servers(0) in read_opts() to reset the flag before
   main config read. */

static int maybe_free_servers = 0;

/* Must be called before  add_update_server() to set daemon->servers_tail */
void mark_servers(int flag)
{
  struct server *serv, *next, **up;

  maybe_free_servers = !!flag;
  
  daemon->servers_tail = NULL;
  
  /* mark everything with argument flag */
  for (serv = daemon->servers; serv; serv = serv->next)
    {
      if (serv->flags & flag)
	serv->flags |= SERV_MARK;
      else
	serv->flags &= ~SERV_MARK;

      daemon->servers_tail = serv;
    }
  
  /* --address etc is different: since they are expected to be 
     1) numerous and 2) not reloaded often. We just delete 
     and recreate. */
  if (flag)
    for (serv = daemon->local_domains, up = &daemon->local_domains; serv; serv = next)
      {
	next = serv->next;

	if (serv->flags & flag)
	  {
	    *up = next;
	    free(serv->domain);
	    free(serv);
	  }
	else 
	  up = &serv->next;
      }
}

void cleanup_servers(void)
{
  struct server *serv, *tmp, **up;

  /* unlink and free anything still marked. */
  for (serv = daemon->servers, up = &daemon->servers, daemon->servers_tail = NULL; serv; serv = tmp) 
    {
      tmp = serv->next;
      if (serv->flags & SERV_MARK)
       {
         server_gone(serv);
         *up = serv->next;
	 free(serv->domain);
	 free(serv);
       }
      else 
	{
	  up = &serv->next;
	  daemon->servers_tail = serv;
	}
    }
}

int add_update_server(int flags,
		      union mysockaddr *addr,
		      union mysockaddr *source_addr,
		      const char *interface,
		      const char *domain,
		      union all_addr *local_addr)
{
  struct server *serv = NULL;
  char *alloc_domain;
  
  if (!domain)
    domain = "";

  /* .domain == domain, for historical reasons. */
  if (*domain == '.')
    while (*domain == '.') domain++;
  else if (*domain == '*')
    {
      domain++;
      if (*domain != 0)
	flags |= SERV_WILDCARD;
    }
  
  if (*domain == 0)
    alloc_domain = whine_malloc(1);
  else
    alloc_domain = canonicalise((char *)domain, NULL);

  if (!alloc_domain)
    return 0;

  if (flags & SERV_IS_LOCAL)
    {
      size_t size;
      
      if (flags & SERV_6ADDR)
	size = sizeof(struct serv_addr6);
      else if (flags & SERV_4ADDR)
	size = sizeof(struct serv_addr4);
      else
	size = sizeof(struct serv_local);
      
      if (!(serv = whine_malloc(size)))
	{
	  free(alloc_domain);
	  return 0;
	}
      
      serv->next = daemon->local_domains;
      daemon->local_domains = serv;
      
      if (flags & SERV_4ADDR)
	((struct serv_addr4*)serv)->addr = local_addr->addr4;
      
      if (flags & SERV_6ADDR)
	((struct serv_addr6*)serv)->addr = local_addr->addr6;
    }
  else
    { 
      /* Upstream servers. See if there is a suitable candidate, if so unmark
	 and move to the end of the list, for order. The entry found may already
	 be at the end. */
      struct server **up, *tmp;

      serv = NULL;
      
      if (maybe_free_servers)
	for (serv = daemon->servers, up = &daemon->servers; serv; serv = tmp)
	  {
	    tmp = serv->next;
	    if ((serv->flags & SERV_MARK) &&
		hostname_isequal(alloc_domain, serv->domain))
	      {
		/* Need to move down? */
		if (serv->next)
		  {
		    *up = serv->next;
		    daemon->servers_tail->next = serv;
		    daemon->servers_tail = serv;
		    serv->next = NULL;
		  }
		break;
	      }
	    else
	      up = &serv->next;
	  }
      
      if (serv)
	{
	  free(alloc_domain);
	  alloc_domain = serv->domain;
	}
      else
	{
	  if (!(serv = whine_malloc(sizeof(struct server))))
	    {
	      free(alloc_domain);
	      return 0;
	    }
	  
	  memset(serv, 0, sizeof(struct server));
	  
	  /* Add to the end of the chain, for order */
	  if (daemon->servers_tail)
	    daemon->servers_tail->next = serv;
	  else
	    daemon->servers = serv;
	  daemon->servers_tail = serv;
	}
      
#ifdef HAVE_LOOP
      serv->uid = rand32();
#endif      
	  
      if (interface)
	safe_strncpy(serv->interface, interface, sizeof(serv->interface));
      if (addr)
	serv->addr = *addr;
      if (source_addr)
	serv->source_addr = *source_addr;
    }
    
  serv->flags = flags;
  serv->domain = alloc_domain;
  serv->domain_len = strlen(alloc_domain);
  
  return 1;
}

