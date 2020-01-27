/* dnsmasq is Copyright (c) 2000-2020 Simon Kelley

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


static struct cond_domain *search_domain(struct in_addr addr, struct cond_domain *c);
static struct cond_domain *search_domain6(struct in6_addr *addr, struct cond_domain *c);


int is_name_synthetic(int flags, char *name, union all_addr *addr)
{
  char *p;
  struct cond_domain *c = NULL;
  int prot = (flags & F_IPV6) ? AF_INET6 : AF_INET;

  for (c = daemon->synth_domains; c; c = c->next)
    {
      int found = 0;
      char *tail, *pref;
      
      for (tail = name, pref = c->prefix; *tail != 0 && pref && *pref != 0; tail++, pref++)
	{
	  unsigned int c1 = (unsigned char) *pref;
	  unsigned int c2 = (unsigned char) *tail;
	  
	  if (c1 >= 'A' && c1 <= 'Z')
	    c1 += 'a' - 'A';
	  if (c2 >= 'A' && c2 <= 'Z')
	    c2 += 'a' - 'A';
	  
	  if (c1 != c2)
	    break;
	}
      
      if (pref && *pref != 0)
	continue; /* prefix match fail */

      if (c->indexed)
	{
	  for (p = tail; *p; p++)
	    {
	      char c = *p;
	      
	      if (c < '0' || c > '9')
		break;
	    }
	  
	  if (*p != '.')
	    continue;
	  
	  *p = 0;
	  
	  if (hostname_isequal(c->domain, p+1))
	    {
	      if (prot == AF_INET)
		{
		  unsigned int index = atoi(tail);

		   if (!c->is6 &&
		      index <= ntohl(c->end.s_addr) - ntohl(c->start.s_addr))
		    {
		      addr->addr4.s_addr = htonl(ntohl(c->start.s_addr) + index);
		      found = 1;
		    }
		} 
	      else
		{
		  u64 index = atoll(tail);
		  
		  if (c->is6 &&
		      index <= addr6part(&c->end6) - addr6part(&c->start6))
		    {
		      u64 start = addr6part(&c->start6);
		      addr->addr6 = c->start6;
		      setaddr6part(&addr->addr6, start + index);
		      found = 1;
		    }
		}
	    }
	}
      else
	{
	  /* NB, must not alter name if we return zero */
	  for (p = tail; *p; p++)
	    {
	      char c = *p;
	      
	      if ((c >='0' && c <= '9') || c == '-')
		continue;
	      
	      if (prot == AF_INET6 && ((c >='A' && c <= 'F') || (c >='a' && c <= 'f'))) 
		continue;
	      
	      break;
	    }
	  
	  if (*p != '.')
	    continue;
	  
	  *p = 0;	
	  
	  if (prot == AF_INET6 && strstr(tail, "--ffff-") == tail)
	    {
	      /* special hack for v4-mapped. */
	      memcpy(tail, "::ffff:", 7);
	      for (p = tail + 7; *p; p++)
		if (*p == '-')
		  *p = '.';
	    }
	  else
	    {
	      /* swap . or : for - */
	      for (p = tail; *p; p++)
		if (*p == '-')
		  {
		    if (prot == AF_INET)
		      *p = '.';
		    else
		      *p = ':';
		  }
	    }
	  
	  if (hostname_isequal(c->domain, p+1) && inet_pton(prot, tail, addr))
	    {
	      if (prot == AF_INET)
		{
		  if (!c->is6 &&
		      ntohl(addr->addr4.s_addr) >= ntohl(c->start.s_addr) &&
		      ntohl(addr->addr4.s_addr) <= ntohl(c->end.s_addr))
		    found = 1;
		}
	      else
		{
		  u64 addrpart = addr6part(&addr->addr6);
		  
		  if (c->is6 &&
		      is_same_net6(&addr->addr6, &c->start6, 64) &&
		      addrpart >= addr6part(&c->start6) &&
		      addrpart <= addr6part(&c->end6))
		    found = 1;
		}
	    }

	}

      /* restore name */
      for (p = tail; *p; p++)
	if (*p == '.' || *p == ':')
	  *p = '-';
      
      *p = '.';
      
      
      if (found)
	return 1;
    }
  
  return 0;
}


int is_rev_synth(int flag, union all_addr *addr, char *name)
{
   struct cond_domain *c;

   if (flag & F_IPV4 && (c = search_domain(addr->addr4, daemon->synth_domains))) 
     {
       char *p;
       
       *name = 0;
       if (c->indexed)
	 {
	   unsigned int index = ntohl(addr->addr4.s_addr) - ntohl(c->start.s_addr);
	   snprintf(name, MAXDNAME, "%s%u", c->prefix ? c->prefix : "", index);
	 }
       else
	 {
	   if (c->prefix)
	     strncpy(name, c->prefix, MAXDNAME - ADDRSTRLEN);
       
       	   inet_ntop(AF_INET, &addr->addr4, name + strlen(name), ADDRSTRLEN);
	   for (p = name; *p; p++)
	     if (*p == '.')
	       *p = '-';
	 }
       
       strncat(name, ".", MAXDNAME);
       strncat(name, c->domain, MAXDNAME);

       return 1;
     }

   if ((flag & F_IPV6) && (c = search_domain6(&addr->addr6, daemon->synth_domains))) 
     {
       char *p;
       
       *name = 0;
       if (c->indexed)
	 {
	   u64 index = addr6part(&addr->addr6) - addr6part(&c->start6);
	   snprintf(name, MAXDNAME, "%s%llu", c->prefix ? c->prefix : "", index);
	 }
       else
	 {
	   if (c->prefix)
	     strncpy(name, c->prefix, MAXDNAME - ADDRSTRLEN);
       
	   inet_ntop(AF_INET6, &addr->addr6, name + strlen(name), ADDRSTRLEN);

	   /* IPv6 presentation address can start with ":", but valid domain names
	      cannot start with "-" so prepend a zero in that case. */
	   if (!c->prefix && *name == ':')
	     {
	       *name = '0';
	       inet_ntop(AF_INET6, &addr->addr6, name+1, ADDRSTRLEN);
	     }
	   
	   /* V4-mapped have periods.... */
	   for (p = name; *p; p++)
	     if (*p == ':' || *p == '.')
	       *p = '-';
	   
	 }

       strncat(name, ".", MAXDNAME);
       strncat(name, c->domain, MAXDNAME);
       
       return 1;
     }
   
   return 0;
}


static struct cond_domain *search_domain(struct in_addr addr, struct cond_domain *c)
{
  for (; c; c = c->next)
    if (!c->is6 &&
	ntohl(addr.s_addr) >= ntohl(c->start.s_addr) &&
        ntohl(addr.s_addr) <= ntohl(c->end.s_addr))
      return c;

  return NULL;
}

char *get_domain(struct in_addr addr)
{
  struct cond_domain *c;

  if ((c = search_domain(addr, daemon->cond_domain)))
    return c->domain;

  return daemon->domain_suffix;
} 


static struct cond_domain *search_domain6(struct in6_addr *addr, struct cond_domain *c)
{
  u64 addrpart = addr6part(addr);
  
  for (; c; c = c->next)
    if (c->is6 &&
	is_same_net6(addr, &c->start6, 64) &&
	addrpart >= addr6part(&c->start6) &&
        addrpart <= addr6part(&c->end6))
      return c;
  
  return NULL;
}

char *get_domain6(struct in6_addr *addr)
{
  struct cond_domain *c;

  if (addr && (c = search_domain6(addr, daemon->cond_domain)))
    return c->domain;

  return daemon->domain_suffix;
} 
