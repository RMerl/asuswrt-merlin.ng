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

int extract_name(struct dns_header *header, size_t plen, unsigned char **pp, 
		 char *name, int isExtract, int extrabytes)
{
  unsigned char *cp = (unsigned char *)name, *p = *pp, *p1 = NULL;
  unsigned int j, l, namelen = 0, hops = 0;
  int retvalue = 1;
  
  if (isExtract)
    *cp = 0;

  while (1)
    { 
      unsigned int label_type;

      if (!CHECK_LEN(header, p, plen, 1))
	return 0;
      
      if ((l = *p++) == 0) 
	/* end marker */
	{
	  /* check that there are the correct no. of bytes after the name */
	  if (!CHECK_LEN(header, p1 ? p1 : p, plen, extrabytes))
	    return 0;
	  
	  if (isExtract)
	    {
	      if (cp != (unsigned char *)name)
		cp--;
	      *cp = 0; /* terminate: lose final period */
	    }
	  else if (*cp != 0)
	    retvalue = 2;
	  
	  if (p1) /* we jumped via compression */
	    *pp = p1;
	  else
	    *pp = p;
	  
	  return retvalue;
	}

      label_type = l & 0xc0;
      
      if (label_type == 0xc0) /* pointer */
	{ 
	  if (!CHECK_LEN(header, p, plen, 1))
	    return 0;
	      
	  /* get offset */
	  l = (l&0x3f) << 8;
	  l |= *p++;
	  
	  if (!p1) /* first jump, save location to go back to */
	    p1 = p;
	      
	  hops++; /* break malicious infinite loops */
	  if (hops > 255)
	    return 0;
	  
	  p = l + (unsigned char *)header;
	}
      else if (label_type == 0x00)
	{ /* label_type = 0 -> label. */
	  namelen += l + 1; /* include period */
	  if (namelen >= MAXDNAME)
	    return 0;
	  if (!CHECK_LEN(header, p, plen, l))
	    return 0;
	  
	  for(j=0; j<l; j++, p++)
	    if (isExtract)
	      {
		unsigned char c = *p;

		if (c == 0 || c == '.' || c == NAME_ESCAPE)
		  {
		    *cp++ = NAME_ESCAPE;
		    *cp++ = c+1;
		  }
		else
		  *cp++ = c; 
	      }
	    else 
	      {
		unsigned char c1 = *cp, c2 = *p;
		
		if (c1 == 0)
		  retvalue = 2;
		else 
		  {
		    cp++;
		    if (c1 >= 'A' && c1 <= 'Z')
		      c1 += 'a' - 'A';

		    if (c1 == NAME_ESCAPE)
		      c1 = (*cp++)-1;
		    
		    if (c2 >= 'A' && c2 <= 'Z')
		      c2 += 'a' - 'A';
		     
		    if (c1 != c2)
		      retvalue =  2;
		  }
	      }
	    
	  if (isExtract)
	    *cp++ = '.';
	  else if (*cp != 0 && *cp++ != '.')
	    retvalue = 2;
	}
      else
	return 0; /* label types 0x40 and 0x80 not supported */
    }
}
 
/* Max size of input string (for IPv6) is 75 chars.) */
#define MAXARPANAME 75
int in_arpa_name_2_addr(char *namein, union all_addr *addrp)
{
  int j;
  char name[MAXARPANAME+1], *cp1;
  unsigned char *addr = (unsigned char *)addrp;
  char *lastchunk = NULL, *penchunk = NULL;
  
  if (strlen(namein) > MAXARPANAME)
    return 0;

  memset(addrp, 0, sizeof(union all_addr));

  /* turn name into a series of asciiz strings */
  /* j counts no. of labels */
  for(j = 1,cp1 = name; *namein; cp1++, namein++)
    if (*namein == '.')
      {
	penchunk = lastchunk;
        lastchunk = cp1 + 1;
	*cp1 = 0;
	j++;
      }
    else
      *cp1 = *namein;
  
  *cp1 = 0;

  if (j<3)
    return 0;

  if (hostname_isequal(lastchunk, "arpa") && hostname_isequal(penchunk, "in-addr"))
    {
      /* IP v4 */
      /* address arrives as a name of the form
	 www.xxx.yyy.zzz.in-addr.arpa
	 some of the low order address octets might be missing
	 and should be set to zero. */
      for (cp1 = name; cp1 != penchunk; cp1 += strlen(cp1)+1)
	{
	  /* check for digits only (weeds out things like
	     50.0/24.67.28.64.in-addr.arpa which are used 
	     as CNAME targets according to RFC 2317 */
	  char *cp;
	  for (cp = cp1; *cp; cp++)
	    if (!isdigit((unsigned char)*cp))
	      return 0;
	  
	  addr[3] = addr[2];
	  addr[2] = addr[1];
	  addr[1] = addr[0];
	  addr[0] = atoi(cp1);
	}

      return F_IPV4;
    }
  else if (hostname_isequal(penchunk, "ip6") && 
	   (hostname_isequal(lastchunk, "int") || hostname_isequal(lastchunk, "arpa")))
    {
      /* IP v6:
         Address arrives as 0.1.2.3.4.5.6.7.8.9.a.b.c.d.e.f.ip6.[int|arpa]
    	 or \[xfedcba9876543210fedcba9876543210/128].ip6.[int|arpa]
      
	 Note that most of these the various representations are obsolete and 
	 left-over from the many DNS-for-IPv6 wars. We support all the formats
	 that we can since there is no reason not to.
      */

      if (*name == '\\' && *(name+1) == '[' && 
	  (*(name+2) == 'x' || *(name+2) == 'X'))
	{	  
	  for (j = 0, cp1 = name+3; *cp1 && isxdigit((unsigned char) *cp1) && j < 32; cp1++, j++)
	    {
	      char xdig[2];
	      xdig[0] = *cp1;
	      xdig[1] = 0;
	      if (j%2)
		addr[j/2] |= strtol(xdig, NULL, 16);
	      else
		addr[j/2] = strtol(xdig, NULL, 16) << 4;
	    }
	  
	  if (*cp1 == '/' && j == 32)
	    return F_IPV6;
	}
      else
	{
	  for (cp1 = name; cp1 != penchunk; cp1 += strlen(cp1)+1)
	    {
	      if (*(cp1+1) || !isxdigit((unsigned char)*cp1))
		return 0;
	      
	      for (j = sizeof(struct in6_addr)-1; j>0; j--)
		addr[j] = (addr[j] >> 4) | (addr[j-1] << 4);
	      addr[0] = (addr[0] >> 4) | (strtol(cp1, NULL, 16) << 4);
	    }
	  
	  return F_IPV6;
	}
    }
  
  return 0;
}

unsigned char *skip_name(unsigned char *ansp, struct dns_header *header, size_t plen, int extrabytes)
{
  while(1)
    {
      unsigned int label_type;
      
      if (!CHECK_LEN(header, ansp, plen, 1))
	return NULL;
      
      label_type = (*ansp) & 0xc0;

      if (label_type == 0xc0)
	{
	  /* pointer for compression. */
	  ansp += 2;	
	  break;
	}
      else if (label_type == 0x80)
	return NULL; /* reserved */
      else if (label_type == 0x40)
	{
	  /* Extended label type */
	  unsigned int count;
	  
	  if (!CHECK_LEN(header, ansp, plen, 2))
	    return NULL;
	  
	  if (((*ansp++) & 0x3f) != 1)
	    return NULL; /* we only understand bitstrings */
	  
	  count = *(ansp++); /* Bits in bitstring */
	  
	  if (count == 0) /* count == 0 means 256 bits */
	    ansp += 32;
	  else
	    ansp += ((count-1)>>3)+1;
	}
      else
	{ /* label type == 0 Bottom six bits is length */
	  unsigned int len = (*ansp++) & 0x3f;
	  
	  if (!ADD_RDLEN(header, ansp, plen, len))
	    return NULL;

	  if (len == 0)
	    break; /* zero length label marks the end. */
	}
    }

  if (!CHECK_LEN(header, ansp, plen, extrabytes))
    return NULL;
  
  return ansp;
}

unsigned char *skip_questions(struct dns_header *header, size_t plen)
{
  int q;
  unsigned char *ansp = (unsigned char *)(header+1);

  for (q = ntohs(header->qdcount); q != 0; q--)
    {
      if (!(ansp = skip_name(ansp, header, plen, 4)))
	return NULL;
      ansp += 4; /* class and type */
    }
  
  return ansp;
}

unsigned char *skip_section(unsigned char *ansp, int count, struct dns_header *header, size_t plen)
{
  int i, rdlen;
  
  for (i = 0; i < count; i++)
    {
      if (!(ansp = skip_name(ansp, header, plen, 10)))
	return NULL; 
      ansp += 8; /* type, class, TTL */
      GETSHORT(rdlen, ansp);
      if (!ADD_RDLEN(header, ansp, plen, rdlen))
	return NULL;
    }

  return ansp;
}

size_t resize_packet(struct dns_header *header, size_t plen, unsigned char *pheader, size_t hlen)
{
  unsigned char *ansp = skip_questions(header, plen);
    
  /* if packet is malformed, just return as-is. */
  if (!ansp)
    return plen;
  
  if (!(ansp = skip_section(ansp, ntohs(header->ancount) + ntohs(header->nscount) + ntohs(header->arcount),
			    header, plen)))
    return plen;
    
  /* restore pseudoheader */
  if (pheader && ntohs(header->arcount) == 0)
    {
      /* must use memmove, may overlap */
      memmove(ansp, pheader, hlen);
      header->arcount = htons(1);
      ansp += hlen;
    }

  return ansp - (unsigned char *)header;
}

/* is addr in the non-globally-routed IP space? */ 
int private_net(struct in_addr addr, int ban_localhost) 
{
  in_addr_t ip_addr = ntohl(addr.s_addr);

  return
    (((ip_addr & 0xFF000000) == 0x7F000000) && ban_localhost)  /* 127.0.0.0/8    (loopback) */ ||
    (((ip_addr & 0xFF000000) == 0x00000000) && ban_localhost) /* RFC 5735 section 3. "here" network */ ||
    ((ip_addr & 0xFF000000) == 0x0A000000)  /* 10.0.0.0/8     (private)  */ ||
    ((ip_addr & 0xFFF00000) == 0xAC100000)  /* 172.16.0.0/12  (private)  */ ||
    ((ip_addr & 0xFFFF0000) == 0xC0A80000)  /* 192.168.0.0/16 (private)  */ ||
    ((ip_addr & 0xFFFF0000) == 0xA9FE0000)  /* 169.254.0.0/16 (zeroconf) */ ||
    ((ip_addr & 0xFFFFFF00) == 0xC0000200)  /* 192.0.2.0/24   (test-net) */ ||
    ((ip_addr & 0xFFFFFF00) == 0xC6336400)  /* 198.51.100.0/24(test-net) */ ||
    ((ip_addr & 0xFFFFFF00) == 0xCB007100)  /* 203.0.113.0/24 (test-net) */ ||
    ((ip_addr & 0xFFFFFFFF) == 0xFFFFFFFF)  /* 255.255.255.255/32 (broadcast)*/ ;
}

static int private_net6(struct in6_addr *a, int ban_localhost)
{
  /* Block IPv4-mapped IPv6 addresses in private IPv4 address space */
  if (IN6_IS_ADDR_V4MAPPED(a))
    {
      struct in_addr v4;
      v4.s_addr = ((const uint32_t *) (a))[3];
      return private_net(v4, ban_localhost);
    }

  return
    (IN6_IS_ADDR_UNSPECIFIED(a) && ban_localhost) || /* RFC 6303 4.3 */
    (IN6_IS_ADDR_LOOPBACK(a) && ban_localhost) ||    /* RFC 6303 4.3 */
    IN6_IS_ADDR_LINKLOCAL(a) ||   /* RFC 6303 4.5 */
    IN6_IS_ADDR_SITELOCAL(a) ||
    ((unsigned char *)a)[0] == 0xfd ||   /* RFC 6303 4.4 */
    ((u32 *)a)[0] == htonl(0x20010db8); /* RFC 6303 4.6 */
}

int do_doctor(struct dns_header *header, size_t qlen, char *namebuff)
{
  unsigned char *p;
  int i, qtype, qclass, rdlen;
  int done = 0;
  
  if (!(p = skip_questions(header, qlen)))
    return done;
  
  for (i = 0; i < ntohs(header->ancount) + ntohs(header->arcount); i++)
    {
      /* Skip over auth section */
      if (i == ntohs(header->ancount) && !(p = skip_section(p, ntohs(header->nscount), header, qlen)))
	return done;
      
      if (!extract_name(header, qlen, &p, namebuff, 1, 10))
	return done; /* bad packet */
      
      GETSHORT(qtype, p); 
      GETSHORT(qclass, p);
      p += 4; /* ttl */
      GETSHORT(rdlen, p);
      
      if (qclass == C_IN && qtype == T_A)
	{
	  struct doctor *doctor;
	  union all_addr addr;
	  
	  if (!CHECK_LEN(header, p, qlen, INADDRSZ))
	    return done;
	  
	  /* alignment */
	  memcpy(&addr.addr4, p, INADDRSZ);
	  
	  for (doctor = daemon->doctors; doctor; doctor = doctor->next)
	    {
	      if (doctor->end.s_addr == 0)
		{
		  if (!is_same_net(doctor->in, addr.addr4, doctor->mask))
		    continue;
		}
	      else if (ntohl(doctor->in.s_addr) > ntohl(addr.addr4.s_addr) || 
		       ntohl(doctor->end.s_addr) < ntohl(addr.addr4.s_addr))
		continue;
	      
	      addr.addr4.s_addr &= ~doctor->mask.s_addr;
	      addr.addr4.s_addr |= (doctor->out.s_addr & doctor->mask.s_addr);
	      /* Since we munged the data, the server it came from is no longer authoritative */
	      header->hb3 &= ~HB3_AA;
#ifdef HAVE_DNSSEC
	      /* remove validated flag from this RR, since we changed it! */
	      if (option_bool(OPT_DNSSEC_VALID) && i <  ntohs(header->ancount))
		daemon->rr_status[i] = 0;
#endif
	      done = 1;
	      memcpy(p, &addr.addr4, INADDRSZ);
	      log_query(F_FORWARD | F_CONFIG | F_IPV4, namebuff, &addr, NULL, 0);
	      break;
	    }
	}
      
      if (!ADD_RDLEN(header, p, qlen, rdlen))
	 return done; /* bad packet */
    }

  return done;
}

/* Find SOA RR in auth section to get TTL for negative caching of name. 
   Cache said SOA and return the difference in length between name and the name of the 
   SOA RR so we can look it up again.
*/
static int find_soa(struct dns_header *header, size_t qlen, char *name, int *substring, unsigned long *ttlp, int no_cache, time_t now)
{
  unsigned char *p, *psave;
  int qtype, qclass, rdlen;
  unsigned long ttl, minttl;
  int i, j;
  size_t name_len, soa_len, len;
  union all_addr addr;

  /* first move to NS section and find TTL from  SOA RR */
  if (!(p = skip_questions(header, qlen)) ||
      !(p = skip_section(p, ntohs(header->ancount), header, qlen)))
    return 0;  /* bad packet */

  name_len = strlen(name);
  
  if (substring)
    *substring = name_len;

  if (ttlp)
    *ttlp = daemon->neg_ttl;
  
  for (i = 0; i < ntohs(header->nscount); i++)
    {
      if (!extract_name(header, qlen, &p, daemon->workspacename, 1, 0))
	return 0; /* bad packet */
      
      GETSHORT(qtype, p); 
      GETSHORT(qclass, p);
      GETLONG(ttl, p);
      GETSHORT(rdlen, p);

      psave = p;
      
      if ((qclass == C_IN) && (qtype == T_SOA))
	{
	  soa_len = strlen(daemon->workspacename);

	  /* SOA must be for the name we're interested in. */
	  if (soa_len <= name_len && memcmp(daemon->workspacename, name + name_len - soa_len, soa_len) == 0)
	    {
	      int prefix = name_len - soa_len;
	      
	      if (!no_cache)
		{
		  if (!(addr.rrblock.rrdata = blockdata_alloc(NULL, 0)))
		    return 0;
		  addr.rrblock.rrtype = T_SOA;
		  addr.rrblock.datalen = 0;
		}
	      
	      for (j = 0; j < 2; j++) /* MNAME, RNAME */
		{
		  if (!extract_name(header, qlen, &p, daemon->workspacename, 1, 0))
		    {
		      if (!no_cache)
			blockdata_free(addr.rrblock.rrdata);
		      return 0;
		    }
		  
		  if (!no_cache)
		    {
		      len = to_wire(daemon->workspacename);
		      if (!blockdata_expand(addr.rrblock.rrdata, addr.rrblock.datalen, daemon->workspacename, len))
			{
			  blockdata_free(addr.rrblock.rrdata);
			  return 0;
			}

		      addr.rrblock.datalen += len;
		    }
		}

	      if (!CHECK_LEN(header, p, qlen, 20))
		{
		  if (!no_cache)
		    blockdata_free(addr.rrblock.rrdata);
		  return 0;
		}
	      
	      /* rest of RR */
	      if (!no_cache && !blockdata_expand(addr.rrblock.rrdata, addr.rrblock.datalen, (char *)p, 20))
		{
		  blockdata_free(addr.rrblock.rrdata);
		  return 0;
		}

	      addr.rrblock.datalen += 20;
	      
	      if (!no_cache)
		{
		  int secflag = 0;

#ifdef HAVE_DNSSEC
		  if (option_bool(OPT_DNSSEC_VALID) && daemon->rr_status[i + ntohs(header->ancount)] != 0)
		    {
		      secflag = F_DNSSECOK; 
		  
		      /* limit TTL based on signature. */
		      if (daemon->rr_status[i + ntohs(header->ancount)] < ttl)
			ttl = daemon->rr_status[i + ntohs(header->ancount)];
		    }
#endif
		  
		  if (!cache_insert(name + prefix, &addr, C_IN, now, ttl, F_FORWARD | F_RR | F_KEYTAG | secflag))
		    {
		      blockdata_free(addr.rrblock.rrdata);
		      return 0;
		    }
		}
	      
	      p += 16; /* SERIAL REFRESH RETRY EXPIRE */
	      
	      GETLONG(minttl, p); /* minTTL */
	      if (ttl < minttl)
		minttl = ttl;

	      if (substring)
		*substring = prefix;
	      
	      if (ttlp)
		*ttlp = minttl;

	      return 1;
	    }
	}

      p = psave;
      
      if (!ADD_RDLEN(header, p, qlen, rdlen))
	return 0; /* bad packet */
    }
  
  return 0;
}

/* Print TXT reply to log */
static int log_txt(char *name, unsigned char *p, const int ardlen, int flag)
{
  unsigned char *p1 = p;
 
  /* Loop over TXT payload */
  while ((p1 - p) < ardlen)
    {
      unsigned int i, len = *p1;
      unsigned char *p3 = p1;
      if ((p1 + len - p) >= ardlen)
	return 0; /* bad packet */

      /* make counted string zero-term and sanitise */
      for (i = 0; i < len; i++)
	{
	  if (!isprint((unsigned char)*(p3+1)))
	    break;
	  *p3 = *(p3+1);
	  p3++;
	}

      *p3 = 0;
      log_query(flag, name, NULL, (char*)p1, 0);
      /* restore */
      memmove(p1 + 1, p1, i);
      *p1 = len;
      p1 += len+1;
    }
  return 1;
}

/* Note that the following code can create CNAME chains that don't point to a real record,
   either because of lack of memory, or lack of SOA records.  These are treated by the cache code as 
   expired and cleaned out that way. 
   Return 1 if we reject an address because it look like part of dns-rebinding attack. 
   Return 2 if the packet is malformed.
*/
int extract_addresses(struct dns_header *header, size_t qlen, char *name, time_t now, 
		      struct ipsets *ipsets, struct ipsets *nftsets, int is_sign, int check_rebind,
		      int no_cache_dnssec, int secure)
{
  unsigned char *p, *p1, *endrr, *namep;
  int j, qtype, qclass, aqtype, aqclass, ardlen, res;
  unsigned long ttl = 0;
  union all_addr addr;
#ifdef HAVE_IPSET
  char **ipsets_cur;
#else
  (void)ipsets; /* unused */
#endif
#ifdef HAVE_NFTSET
  char **nftsets_cur;
#else
  (void)nftsets; /* unused */
#endif
  int found = 0, cname_count = CNAME_CHAIN;
  struct crec *cpp = NULL;
  int flags = RCODE(header) == NXDOMAIN ? F_NXDOMAIN : 0;
#ifdef HAVE_DNSSEC
  int cname_short = 0;
#endif
  unsigned long cttl = ULONG_MAX, attl;

  cache_start_insert();

  namep = p = (unsigned char *)(header+1);
  
  if (ntohs(header->qdcount) != 1 || !extract_name(header, qlen, &p, name, 1, 4))
    return 2; /* bad packet */
  
  GETSHORT(qtype, p); 
  GETSHORT(qclass, p);
  
  if (qclass != C_IN)
    return 0;
  
  /* PTRs: we chase CNAMEs here, since we have no way to 
     represent them in the cache. */
  if (qtype == T_PTR)
    { 
      int insert = 1, name_encoding = in_arpa_name_2_addr(name, &addr);
      
      if (!(flags & F_NXDOMAIN))
	{
	cname_loop:
	  if (!(p1 = skip_questions(header, qlen)))
	    return 2;
	  
	  for (j = 0; j < ntohs(header->ancount); j++) 
	    {
	      int secflag = 0;
	      if (!(res = extract_name(header, qlen, &p1, name, 0, 10)))
		return 2; /* bad packet */
	      
	      GETSHORT(aqtype, p1); 
	      GETSHORT(aqclass, p1);
	      GETLONG(attl, p1);
	      
	      if ((daemon->max_ttl != 0) && (attl > daemon->max_ttl) && !is_sign)
		{
		  (p1) -= 4;
		  PUTLONG(daemon->max_ttl, p1);
		}
	      GETSHORT(ardlen, p1);
	      endrr = p1+ardlen;
	      
	      /* TTL of record is minimum of CNAMES and PTR */
	      if (attl < cttl)
		cttl = attl;
	      
	      if (aqclass == C_IN && res != 2 && (aqtype == T_CNAME || aqtype == T_PTR))
		{
#ifdef HAVE_DNSSEC
		  if (option_bool(OPT_DNSSEC_VALID) && daemon->rr_status[j] != 0)
		    {
		      /* validated RR anywhere in CNAME chain, don't cache. */
		      if (cname_short || aqtype == T_CNAME)
			insert = 0;
		      
		      secflag = F_DNSSECOK;
		      /* limit TTL based on signature. */
		      if (daemon->rr_status[j] < cttl)
			cttl = daemon->rr_status[j];
		    }
#endif

		  if (aqtype == T_CNAME)
		    log_query(secflag | F_CNAME | F_FORWARD | F_UPSTREAM, name, NULL, NULL, 0);
		  
		  if (!extract_name(header, qlen, &p1, name, 1, 0))
		    return 2;
		  
		  if (aqtype == T_CNAME)
		    {
		      if (!cname_count--)
			return 0; /* looped CNAMES, we can't cache. */
#ifdef HAVE_DNSSEC
		      cname_short = 1;
#endif
		      goto cname_loop;
		    }
		  
		  found = 1; 
		  
		  if (!name_encoding)
		    log_query(secflag | F_FORWARD | F_UPSTREAM, name, NULL, NULL, aqtype);
		  else
		    {
		      log_query(name_encoding | secflag | F_REVERSE | F_UPSTREAM, name, &addr, NULL, 0);
		      if (insert)
			cache_insert(name, &addr, C_IN, now, cttl, name_encoding | secflag | F_REVERSE);
		    }
		}

	      p1 = endrr;
	      if (!CHECK_LEN(header, p1, qlen, 0))
		return 2; /* bad packet */
	    }
	}
      
      if (!found && !option_bool(OPT_NO_NEG))
	{
	  /* For reverse records, we use the name field to store the SOA name. */
	  int substring, have_soa = find_soa(header, qlen, name, &substring, &ttl, no_cache_dnssec, now);
	  
	  flags |= F_NEG | (secure ?  F_DNSSECOK : 0);
	  if (name_encoding && ttl)
	    {
	      flags |= F_REVERSE | name_encoding;
	      if (!have_soa)
		flags |= F_NO_RR; /* Marks no SOA found. */
	      cache_insert(name + substring, &addr, C_IN, now, ttl, flags);
	    }
	  
	  log_query(flags | F_UPSTREAM, name, &addr, NULL, 0);
	}
    }
  else
    {
      /* everything other than PTR */
      struct crec *newc;
      int addrlen = 0, insert = 1;
      
      if (qtype == T_A)
	{
	  addrlen = INADDRSZ;
	  flags |= F_IPV4;
	}
      else if (qtype == T_AAAA)
	{
	  addrlen = IN6ADDRSZ;
	  flags |= F_IPV6;
	}
      else if (qtype != T_CNAME &&
	       (qtype == T_SRV || rr_on_list(daemon->cache_rr, qtype) || rr_on_list(daemon->cache_rr, T_ANY)))
	flags |= F_RR;
      else
	insert = 0; /* NOTE: do not cache data from CNAME queries. */
      
    cname_loop1:
      if (!(p1 = skip_questions(header, qlen)))
	return 2;
      
      for (j = 0; j < ntohs(header->ancount); j++) 
	{
	  int secflag = 0;
	  
	  if (!(res = extract_name(header, qlen, &p1, name, 0, 10)))
	    return 2; /* bad packet */
	  
	  GETSHORT(aqtype, p1); 
	  GETSHORT(aqclass, p1);
	  GETLONG(attl, p1);
	  if ((daemon->max_ttl != 0) && (attl > daemon->max_ttl) && !is_sign)
	    {
	      (p1) -= 4;
	      PUTLONG(daemon->max_ttl, p1);
	    }
	  GETSHORT(ardlen, p1);
	  endrr = p1+ardlen;

	  if (!CHECK_LEN(header, endrr, qlen, 0))
	    return 2; /* bad packet */
	  
	  /* Not what we're looking for? */
	  if (aqclass != C_IN || res == 2)
	    {
	      p1 = endrr;
	      continue;
	    }
	  
#ifdef HAVE_DNSSEC
	  if (option_bool(OPT_DNSSEC_VALID) && daemon->rr_status[j] != 0)
	    {
	      secflag = F_DNSSECOK;
	      
	      /* limit TTl based on sig. */
	      if (daemon->rr_status[j] < attl)
		attl = daemon->rr_status[j];
	    }
#endif	  
	  
	  if (aqtype == T_CNAME)
	    {
	      if (!cname_count--)
		return 0; /* looped CNAMES */
	      
	      log_query(secflag | F_CNAME | F_FORWARD | F_UPSTREAM, name, NULL, NULL, 0);
	      
	      if (insert)
		{
		  if ((newc = cache_insert(name, NULL, C_IN, now, attl, F_CNAME | F_FORWARD | secflag)))
		    {
		      newc->addr.cname.target.cache = NULL;
		      newc->addr.cname.is_name_ptr = 0; 
		      if (cpp)
			{
			  next_uid(newc);
			  cpp->addr.cname.target.cache = newc;
			  cpp->addr.cname.uid = newc->uid;
			}
		    }
		  
		  cpp = newc;
		  if (attl < cttl)
		    cttl = attl;
		}
	      
	      namep = p1;
	      if (!extract_name(header, qlen, &p1, name, 1, 0))
		return 2;
	      
	      if (qtype != T_CNAME)
		goto cname_loop1;

	      found = 1;
	    }
	  else if (qtype == T_ANY || aqtype != qtype)
	    {
#ifdef HAVE_DNSSEC
	      if (!option_bool(OPT_DNSSEC_VALID) || aqtype != T_RRSIG)
#endif
		log_query(secflag | F_FORWARD | F_UPSTREAM | F_RRNAME, name, NULL, NULL, aqtype);
	    }
	  else if (!(flags & F_NXDOMAIN))
	    {
	      found = 1;
	      
	      if (flags & F_RR)
		{
		  short desc, *rrdesc = rrfilter_desc(aqtype);
		  unsigned char *tmp = namep;
		  
		  if (!CHECK_LEN(header, p1, qlen, ardlen))
		    return 2; /* bad packet */
		  
		  /* If the data has no names and is small enough, store it in
		     the crec address field rather than allocate a block. */
		  if (*rrdesc == -1 && ardlen <= (int)RR_IMDATALEN)
		    {
		       addr.rrdata.rrtype = aqtype;
		       addr.rrdata.datalen = (char)ardlen;
		       flags &= ~F_KEYTAG; /* in case of >1 answer, not all the same. */ 
		       if (ardlen != 0)
			 memcpy(addr.rrdata.data, p1, ardlen);
		    }
		  else
		    {
		      addr.rrblock.rrtype = aqtype;
		      addr.rrblock.datalen = 0;
		      flags |= F_KEYTAG; /* discriminates between rrdata and rrblock */
		      
		      /* The RR data may include names, and those names may include
			 compression, which will be rendered meaningless when
			 copied into another packet. 
			 Here we go through a description of the packet type to
			 find the names, and extract them to a c-string and then
			 re-encode them to standalone DNS format without compression. */
		      if (!(addr.rrblock.rrdata = blockdata_alloc(NULL, 0)))
			return 0;
		      do
			{
			  desc = *rrdesc++;
			  
			  if (desc == -1)
			    {
			      /* Copy the rest of the RR and end. */
			      if (!blockdata_expand(addr.rrblock.rrdata, addr.rrblock.datalen, (char *)p1, endrr - p1))
				{
				  blockdata_free(addr.rrblock.rrdata);
				  return 0;
				}
			      addr.rrblock.datalen += endrr - p1;
			    }
			  else if (desc == 0)
			    {
			      /* Name, extract it then re-encode. */
			      int len;
			      
			      if (!extract_name(header, qlen, &p1, name, 1, 0))
				{
				  blockdata_free(addr.rrblock.rrdata);
				  return 2;
				}
			      
			      len = to_wire(name);
			      if (!blockdata_expand(addr.rrblock.rrdata, addr.rrblock.datalen, name, len))
				{
				  blockdata_free(addr.rrblock.rrdata);
				  return 0;
				}
			      
			      addr.rrblock.datalen += len;
			    }
			  else
			    {
			      /* desc is length of a block of data to be used as-is */
			      if (desc > endrr - p1)
				desc = endrr - p1;

			      if (!blockdata_expand(addr.rrblock.rrdata, addr.rrblock.datalen, (char *)p1, desc))
				{
				  blockdata_free(addr.rrblock.rrdata);
				  return 0;
				}

			      addr.rrblock.datalen += desc;
			      p1 += desc;
			    }
			} while (desc != -1);
		      
		      /* we overwrote the original name, so get it back here. */
		      if (!extract_name(header, qlen, &tmp, name, 1, 0))
			{
			  blockdata_free(addr.rrblock.rrdata);
			  return 2;
			}
		    }
		} 
	      else if (flags & (F_IPV4 | F_IPV6))
		{
		  /* copy address into aligned storage */
		  if (!CHECK_LEN(header, p1, qlen, addrlen))
		    return 2; /* bad packet */
		  memcpy(&addr, p1, addrlen);
		  
		  /* check for returned address in private space */
		  if (check_rebind)
		    {
		      if ((flags & F_IPV4) &&
			  private_net(addr.addr4, !option_bool(OPT_LOCAL_REBIND)))
			return 1;
		      
		      if ((flags & F_IPV6) &&
			  private_net6(&addr.addr6, !option_bool(OPT_LOCAL_REBIND)))
			return 1;
		    }
		  
#ifdef HAVE_IPSET
		  if (ipsets && (flags & (F_IPV4 | F_IPV6)))
		    for (ipsets_cur = ipsets->sets; *ipsets_cur; ipsets_cur++)
		      if (add_to_ipset(*ipsets_cur, &addr, flags, 0) == 0)
			log_query((flags & (F_IPV4 | F_IPV6)) | F_IPSET, ipsets->domain, &addr, *ipsets_cur, 1);
#endif
#ifdef HAVE_NFTSET
		  if (nftsets && (flags & (F_IPV4 | F_IPV6)))
		    for (nftsets_cur = nftsets->sets; *nftsets_cur; nftsets_cur++)
		      if (add_to_nftset(*nftsets_cur, &addr, flags, 0) == 0)
			log_query((flags & (F_IPV4 | F_IPV6)) | F_IPSET, nftsets->domain, &addr, *nftsets_cur, 0);
#endif
		}
	      
	      if (insert)
		{
		  newc = cache_insert(name, &addr, C_IN, now, attl, flags | F_FORWARD | secflag);
		  if (newc && cpp)
		    {
		      next_uid(newc);
		      cpp->addr.cname.target.cache = newc;
		      cpp->addr.cname.uid = newc->uid;
		    }
		  cpp = NULL;
		  
		  /* cache insert failed, don't leak blockdata. */
		  if (!newc && (flags & F_RR) && (flags & F_KEYTAG))
		    blockdata_free(addr.rrblock.rrdata);  
		}
	      
	      /* We're filtering this RRtype. It will be removed from the 
		 returned packet in process_reply() but gets cached here anyway
		 and will be filtered again on the way out of the cache. Here,
		 we just need to alter the logging. */
	      if (qtype != T_ANY && rr_on_list(daemon->filter_rr, qtype))
		secflag = F_NEG | F_CONFIG;
	      
	      if (aqtype == T_TXT)
		log_txt(name, p1, ardlen, flags | F_FORWARD | F_UPSTREAM | secflag);
	      else
		log_query(flags | F_FORWARD | F_UPSTREAM | secflag, name, &addr, NULL, aqtype);
	    }
	  
	  p1 = endrr;
	  if (!CHECK_LEN(header, p1, qlen, 0))
	    return 2; /* bad packet */
	}
      
      if (!found && (qtype != T_ANY || (flags & F_NXDOMAIN)))
	{
	  if (flags & F_NXDOMAIN)
	    {
	      flags &= ~(F_IPV4 | F_IPV6 | F_RR);
	      
	      /* Can store NXDOMAIN reply for any qtype. */
	      insert = 1;
	    }
	  
	  log_query(F_UPSTREAM | F_FORWARD | F_NEG | flags | (secure ? F_DNSSECOK : 0), name, NULL, NULL, 0);
	  
	  if (insert && !option_bool(OPT_NO_NEG))
	    {
	      int substring, have_soa = find_soa(header, qlen, name, &substring, &ttl, no_cache_dnssec, now);
	      
	      /* If there's no SOA to get the TTL from, but there is a CNAME 
		 pointing at this, inherit its TTL */
	      if (ttl || cpp)
		{
		  if (!ttl)
		    ttl = cttl;
		  
		  addr.rrdata.datalen = substring;
		  addr.rrdata.rrtype = qtype;
		  
		  if (!have_soa)
		    flags |= F_NO_RR; /* Marks no SOA found. */
		}
	      
	      newc = cache_insert(name, &addr, C_IN, now, ttl, F_FORWARD | F_NEG | flags | (secure ? F_DNSSECOK : 0));	
	      if (newc && cpp)
		{
		  next_uid(newc);
		  cpp->addr.cname.target.cache = newc;
		  cpp->addr.cname.uid = newc->uid;
		}
	    }
	}
    }

  /* Don't cache replies from non-recursive nameservers, since we may get a 
     reply containing a CNAME but not its target, even though the target 
     does exist. */
  if (!(header->hb4 & HB4_CD) &&
      (header->hb4 & HB4_RA) &&
      !no_cache_dnssec)
    cache_end_insert();

  return 0;
}

#if defined(HAVE_CONNTRACK) && defined(HAVE_UBUS)
/* Don't pass control chars and weird escapes to UBus. */
static int safe_name(char *name)
{
  unsigned char *r;
  
  for (r = (unsigned char *)name; *r; r++)
    if (!isprint((int)*r))
      return 0;
  
  return 1;
}

void report_addresses(struct dns_header *header, size_t len, u32 mark)
{
  unsigned char *p, *endrr;
  int i;
  unsigned long attl;
  struct allowlist *allowlists;
  char **pattern_pos;
  
  if (RCODE(header) != NOERROR)
    return;
  
  for (allowlists = daemon->allowlists; allowlists; allowlists = allowlists->next)
    if (allowlists->mark == (mark & daemon->allowlist_mask & allowlists->mask))
      for (pattern_pos = allowlists->patterns; *pattern_pos; pattern_pos++)
	if (!strcmp(*pattern_pos, "*"))
	  return;
  
  if (!(p = skip_questions(header, len)))
    return;
  for (i = ntohs(header->ancount); i != 0; i--)
    {
      int aqtype, aqclass, ardlen;
      
      if (!extract_name(header, len, &p, daemon->namebuff, 1, 10))
	return;
      
      if (!CHECK_LEN(header, p, len, 10))
	return;
      GETSHORT(aqtype, p);
      GETSHORT(aqclass, p);
      GETLONG(attl, p);
      GETSHORT(ardlen, p);
      
      if (!CHECK_LEN(header, p, len, ardlen))
	return;
      endrr = p+ardlen;
      
      if (aqclass == C_IN)
	{
	  if (aqtype == T_CNAME)
	    {
	      if (!extract_name(header, len, &p, daemon->workspacename, 1, 0))
		return;
	      if (safe_name(daemon->namebuff) && safe_name(daemon->workspacename))
		ubus_event_bcast_connmark_allowlist_resolved(mark, daemon->namebuff, daemon->workspacename, attl);
	    }
	  if (aqtype == T_A)
	    {
	      struct in_addr addr;
	      char ip[INET_ADDRSTRLEN];
	      if (ardlen != INADDRSZ)
		return;
	      memcpy(&addr, p, ardlen);
	      if (inet_ntop(AF_INET, &addr, ip, sizeof ip) && safe_name(daemon->namebuff))
		ubus_event_bcast_connmark_allowlist_resolved(mark, daemon->namebuff, ip, attl);
	    }
	  else if (aqtype == T_AAAA)
	    {
	      struct in6_addr addr;
	      char ip[INET6_ADDRSTRLEN];
	      if (ardlen != IN6ADDRSZ)
		return;
	      memcpy(&addr, p, ardlen);
	      if (inet_ntop(AF_INET6, &addr, ip, sizeof ip) && safe_name(daemon->namebuff))
		ubus_event_bcast_connmark_allowlist_resolved(mark, daemon->namebuff, ip, attl);
	    }
	}
      
      p = endrr;
    }
}
#endif

/* If the packet holds exactly one query
   return F_IPV4 or F_IPV6  and leave the name from the query in name */
unsigned int extract_request(struct dns_header *header, size_t qlen, char *name, unsigned short *typep)
{
  unsigned char *p = (unsigned char *)(header+1);
  int qtype, qclass;

  if (typep)
    *typep = 0;

  *name = 0; /* return empty name if no query found. */
  
  if (ntohs(header->qdcount) != 1 || OPCODE(header) != QUERY)
    return 0; /* must be exactly one query. */
  
  if (!(header->hb3 & HB3_QR) && (ntohs(header->ancount) != 0 || ntohs(header->nscount) != 0))
    return 0; /* non-standard query. */
  
  if (!extract_name(header, qlen, &p, name, 1, 4))
    return 0; /* bad packet */
   
  GETSHORT(qtype, p); 
  GETSHORT(qclass, p);

  if (typep)
    *typep = qtype;

  if (qclass == C_IN)
    {
      if (qtype == T_A)
	return F_IPV4;
      if (qtype == T_AAAA)
	return F_IPV6;
      if (qtype == T_ANY)
	return  F_IPV4 | F_IPV6;
    }

#ifdef HAVE_DNSSEC
  /* F_DNSSECOK as agument to search_servers() inhibits forwarding
     to servers for domains without a trust anchor. This make the
     behaviour for DS and DNSKEY queries we forward the same
     as for DS and DNSKEY queries we originate. */
  if (option_bool(OPT_DNSSEC_VALID) && (qtype == T_DS || qtype == T_DNSKEY))
    return F_DNSSECOK;
#endif
  
  return F_QUERY;
}

void setup_reply(struct dns_header *header, unsigned int flags, int ede)
{
  /* clear authoritative and truncated flags, set QR flag */
  header->hb3 = (header->hb3 & ~(HB3_AA | HB3_TC )) | HB3_QR;
  /* clear AD flag, set RA flag */
  header->hb4 = (header->hb4 & ~HB4_AD) | HB4_RA;

  header->nscount = htons(0);
  header->arcount = htons(0);
  header->ancount = htons(0); /* no answers unless changed below */
  if (flags == F_NOERR)
    SET_RCODE(header, NOERROR); /* empty domain */
  else if (flags == F_NXDOMAIN)
    SET_RCODE(header, NXDOMAIN);
  else if (flags & ( F_IPV4 | F_IPV6))
    {
      SET_RCODE(header, NOERROR);
      header->hb3 |= HB3_AA;
    }
  else /* nowhere to forward to */
    {
      union all_addr a;
      a.log.rcode = REFUSED;
      a.log.ede = ede;
      log_query(F_CONFIG | F_RCODE, "error", &a, NULL, 0);
      SET_RCODE(header, REFUSED);
    }
}

/* check if name matches local names ie from /etc/hosts or DHCP or local mx names. */
int check_for_local_domain(char *name, time_t now)
{
  struct mx_srv_record *mx;
  struct txt_record *txt;
  struct interface_name *intr;
  struct ptr_record *ptr;
  struct naptr *naptr;

  for (naptr = daemon->naptr; naptr; naptr = naptr->next)
     if (hostname_issubdomain(name, naptr->name))
      return 1;

   for (mx = daemon->mxnames; mx; mx = mx->next)
    if (hostname_issubdomain(name, mx->name))
      return 1;

  for (txt = daemon->txt; txt; txt = txt->next)
    if (hostname_issubdomain(name, txt->name))
      return 1;

  for (intr = daemon->int_names; intr; intr = intr->next)
    if (hostname_issubdomain(name, intr->name))
      return 1;

  for (ptr = daemon->ptr; ptr; ptr = ptr->next)
    if (hostname_issubdomain(name, ptr->name))
      return 1;

  if (cache_find_non_terminal(name, now))
    return 1;

  if (is_name_synthetic(F_IPV4, name, NULL) ||
      is_name_synthetic(F_IPV6, name, NULL))
    return 1;

  return 0;
}

static int check_bad_address(struct dns_header *header, size_t qlen, struct bogus_addr *baddr, char *name, unsigned long *ttlp)
{
  unsigned char *p;
  int i, qtype, qclass, rdlen;
  unsigned long ttl;
  struct bogus_addr *baddrp;
  
  /* skip over questions */
  if (!(p = skip_questions(header, qlen)))
    return 0; /* bad packet */

  for (i = ntohs(header->ancount); i != 0; i--)
    {
      if (name && !extract_name(header, qlen, &p, name, 1, 10))
	return 0; /* bad packet */

      if (!name && !(p = skip_name(p, header, qlen, 10)))
	return 0;
      
      GETSHORT(qtype, p); 
      GETSHORT(qclass, p);
      GETLONG(ttl, p);
      GETSHORT(rdlen, p)
      if (ttlp)
	*ttlp = ttl;
      
      if (qclass == C_IN)
	{
	  if (qtype == T_A)
	    {
	      struct in_addr addr;
	      
	      if (!CHECK_LEN(header, p, qlen, INADDRSZ))
		return 0;

	      memcpy(&addr, p, INADDRSZ);

	      for (baddrp = baddr; baddrp; baddrp = baddrp->next)
		if (!baddrp->is6 && is_same_net_prefix(addr, baddrp->addr.addr4, baddrp->prefix))
		  return 1;
	    }
	  else if (qtype == T_AAAA)
	    {
	      struct in6_addr addr;
	      
	      if (!CHECK_LEN(header, p, qlen, IN6ADDRSZ))
		return 0;

	      memcpy(&addr, p, IN6ADDRSZ);

	      for (baddrp = baddr; baddrp; baddrp = baddrp->next)
		if (baddrp->is6 && is_same_net6(&addr, &baddrp->addr.addr6, baddrp->prefix))
		  return 1;
	    }
	}
      
      if (!ADD_RDLEN(header, p, qlen, rdlen))
	return 0;
    }
  
  return 0;
}

/* Is the packet a reply with the answer address equal to addr?
   If so mung is into an NXDOMAIN reply and also put that information
   in the cache. */
int check_for_bogus_wildcard(struct dns_header *header, size_t qlen, char *name, time_t now)
{
  unsigned long ttl;

  if (check_bad_address(header, qlen, daemon->bogus_addr, name, &ttl))
    {
      /* Found a bogus address. Insert that info here, since there no SOA record
	 to get the ttl from in the normal processing */
      cache_start_insert();
      cache_insert(name, NULL, C_IN, now, ttl, F_FORWARD | F_NEG | F_NXDOMAIN);
      cache_end_insert();
      log_query(F_CONFIG | F_FORWARD | F_NEG | F_NXDOMAIN, name, NULL, NULL, 0);

      return 1;
    }

  return 0;
}

int check_for_ignored_address(struct dns_header *header, size_t qlen)
{
  return check_bad_address(header, qlen, daemon->ignore_addr, NULL, NULL);
}

int add_resource_record(struct dns_header *header, char *limit, int *truncp, int nameoffset, unsigned char **pp, 
			unsigned long ttl, int *offset, unsigned short type, unsigned short class, char *format, ...)
{
  va_list ap;
  unsigned char *sav, *p = *pp;
  int j;
  unsigned short usval;
  long lval;
  char *sval;
  
#define CHECK_LIMIT(size) \
  if (limit && p + (size) > (unsigned char*)limit) goto truncated;

  va_start(ap, format);   /* make ap point to 1st unamed argument */
  
  if (truncp && *truncp)
    goto truncated;
  
  if (nameoffset > 0)
    {
      CHECK_LIMIT(2);
      PUTSHORT(nameoffset | 0xc000, p);
    }
  else
    {
      char *name = va_arg(ap, char *);
      if (name && !(p = do_rfc1035_name(p, name, limit)))
	goto truncated;
      
      if (nameoffset < 0)
	{
	  CHECK_LIMIT(2);
	  PUTSHORT(-nameoffset | 0xc000, p);
	}
      else
	{
	  CHECK_LIMIT(1);
	  *p++ = 0;
	}
    }

  /* type (2) + class (2) + ttl (4) + rdlen (2) */
  CHECK_LIMIT(10);
  
  PUTSHORT(type, p);
  PUTSHORT(class, p);
  PUTLONG(ttl, p);      /* TTL */

  sav = p;              /* Save pointer to RDLength field */
  PUTSHORT(0, p);       /* Placeholder RDLength */

  for (; *format; format++)
    switch (*format)
      {
      case '6':
        CHECK_LIMIT(IN6ADDRSZ);
	sval = va_arg(ap, char *); 
	memcpy(p, sval, IN6ADDRSZ);
	p += IN6ADDRSZ;
	break;
	
      case '4':
        CHECK_LIMIT(INADDRSZ);
	sval = va_arg(ap, char *); 
	memcpy(p, sval, INADDRSZ);
	p += INADDRSZ;
	break;
	
      case 'b':
        CHECK_LIMIT(1);
	usval = va_arg(ap, int);
	*p++ = usval;
	break;
	
      case 's':
        CHECK_LIMIT(2);
	usval = va_arg(ap, int);
	PUTSHORT(usval, p);
	break;
	
      case 'l':
        CHECK_LIMIT(4);
	lval = va_arg(ap, long);
	PUTLONG(lval, p);
	break;
	
      case 'd':
        /* get domain-name answer arg and store it in RDATA field */
        if (offset)
          *offset = p - (unsigned char *)header;
        if (!(p = do_rfc1035_name(p, va_arg(ap, char *), limit)))
	  goto truncated;
	CHECK_LIMIT(1);
        *p++ = 0;
	break;
	
      case 't':
	usval = va_arg(ap, int);
        CHECK_LIMIT(usval);
	sval = va_arg(ap, char *);
	if (usval != 0)
	  memcpy(p, sval, usval);
	p += usval;
	break;

      case 'z':
	sval = va_arg(ap, char *);
	usval = sval ? strlen(sval) : 0;
	if (usval > 255)
	  usval = 255;
        CHECK_LIMIT(usval + 1);
	*p++ = (unsigned char)usval;
	memcpy(p, sval, usval);
	p += usval;
	break;
      }

  va_end(ap);	/* clean up variable argument pointer */
  
  /* Now, store real RDLength. sav already checked against limit. */
  j = p - sav - 2;
  PUTSHORT(j, sav);
  
  *pp = p;
  return 1;
  
 truncated:
  va_end(ap);
  if (truncp)
    *truncp = 1;
  return 0;

#undef CHECK_LIMIT
}

static int crec_isstale(struct crec *crecp, time_t now)
{
  return (!(crecp->flags & F_IMMORTAL)) && difftime(crecp->ttd, now) < 0; 
}

static unsigned long crec_ttl(struct crec *crecp, time_t now)
{
  signed long ttl = difftime(crecp->ttd, now);

  /* Return 0 ttl for DHCP entries, which might change
     before the lease expires, unless configured otherwise. */

  if (crecp->flags & F_DHCP)
    {
      int conf_ttl = daemon->use_dhcp_ttl ? daemon->dhcp_ttl : daemon->local_ttl;
      
      /* Apply ceiling of actual lease length to configured TTL. */
      if (!(crecp->flags & F_IMMORTAL) && ttl < conf_ttl)
	return ttl;
      
      return conf_ttl;
    }	  
  
  /* Immortal entries other than DHCP are local, and hold TTL in TTD field. */
  if (crecp->flags & F_IMMORTAL)
    return crecp->ttd;

  /* Stale cache entries. */
  if (ttl < 0)
    return 0;
  
  /* Return the Max TTL value if it is lower than the actual TTL */
  if (daemon->max_ttl == 0 || ((unsigned)ttl < daemon->max_ttl))
    return ttl;
  else
    return daemon->max_ttl;
}

static int cache_validated(const struct crec *crecp)
{
  return (option_bool(OPT_DNSSEC_VALID) && !(crecp->flags & F_DNSSECOK));
}

/* return zero if we can't answer from cache, or packet size if we can */
size_t answer_request(struct dns_header *header, char *limit, size_t qlen,  
		      struct in_addr local_addr, struct in_addr local_netmask, 
		      time_t now, int ad_reqd, int do_bit, int have_pseudoheader,
		      int *stale, int *filtered) 
{
  char *name = daemon->namebuff;
  unsigned char *p, *ansp;
  unsigned int qtype, qclass;
  union all_addr addr;
  int nameoffset;
  unsigned short flag;
  int ans, anscount = 0, nscount = 0, addncount = 0;
  struct crec *crecp, *soa_lookup = NULL;
  int nxdomain = 0, notimp = 0, auth = 1, trunc = 0, sec_data = 1;
  struct mx_srv_record *rec;
  size_t len;
  int rd_bit = (header->hb3 & HB3_RD);
  int count = 255; /* catch loops */
  
  if (stale)
    *stale = 0;

  if (filtered)
    *filtered = 0;
  
  /* never answer queries with RD unset, to avoid cache snooping. */
  if ( ntohs(header->qdcount) != 1 ||
       ntohs(header->ancount) != 0 ||
      ntohs(header->nscount) != 0 ||
      ntohs(header->qdcount) == 0 ||
      OPCODE(header) != QUERY )
    return 0;

  /* Don't return AD set if checking disabled. */
  if (header->hb4 & HB4_CD)
    sec_data = 0;
  
  for (rec = daemon->mxnames; rec; rec = rec->next)
    rec->offset = 0;
  
  /* determine end of question section (we put answers there) */
  if (!(ansp = skip_questions(header, qlen)))
    return 0; /* bad packet */
   
  /* now process each question, answers go in RRs after the question */
  p = (unsigned char *)(header+1);

  /* save pointer to name for copying into answers */
  nameoffset = p - (unsigned char *)header;
  
  /* now extract name as .-concatenated string into name */
  if (!extract_name(header, qlen, &p, name, 1, 4))
    return 0; /* bad packet */
  
  GETSHORT(qtype, p); 
  GETSHORT(qclass, p);
  
  ans = 0; /* have we answered this question */
  
  if (qclass == C_IN)
    while (--count != 0 && (crecp = cache_find_by_name(NULL, name, now, F_CNAME | F_NXDOMAIN)))
      {
	char *cname_target;
	int stale_flag = 0;
	
	if (crec_isstale(crecp, now))
	  {
	    if (stale)
	      *stale = 1;
	    
	    stale_flag = F_STALE;
	  }
	
	if (crecp->flags & F_NEG)
	  soa_lookup = crecp;
	  
	if (crecp->flags & F_NXDOMAIN)
	  {
	    if (qtype == T_CNAME)
	      {
		log_query(stale_flag | crecp->flags, name, NULL, record_source(crecp->uid), 0);
		auth = 0;
		nxdomain = 1;
		ans = 1;
	      }
	    break;
	  }  
	
	cname_target = cache_get_cname_target(crecp);
	
	/* If the client asked for DNSSEC  don't use cached data. */
	if ((crecp->flags & (F_HOSTS | F_DHCP | F_CONFIG)) ||
	    (rd_bit && (!do_bit || cache_validated(crecp))))
	  {
	    if (crecp->flags & F_CONFIG || qtype == T_CNAME)
	      ans = 1;
	    
	    if (!(crecp->flags & F_DNSSECOK))
	      sec_data = 0;
	    
	    log_query(stale_flag | crecp->flags, name, NULL, record_source(crecp->uid), 0);
	    if (add_resource_record(header, limit, &trunc, nameoffset, &ansp, 
				    crec_ttl(crecp, now), &nameoffset,
				    T_CNAME, C_IN, "d", cname_target))
	      anscount++;
	  }
	else
	  return 0; /* give up if any cached CNAME in chain can't be used for DNSSEC reasons. */
	
	if (qtype == T_CNAME)
	  break;
	
	strcpy(name, cname_target);
      }
  
  if (qtype == T_TXT || qtype == T_ANY)
    {
      struct txt_record *t;
      for(t = daemon->txt; t ; t = t->next)
	{
	  if (t->class == qclass && hostname_isequal(name, t->name))
	    {
	      unsigned long ttl = daemon->local_ttl;
	      int ok = 1;
	      
	      ans = 1, sec_data = 0;
#ifndef NO_ID
	      /* Dynamically generate stat record */
	      if (t->stat != 0)
		{
		  ttl = 0;
		  if (!cache_make_stat(t))
		    ok = 0;
		}
#endif
	      if (ok)
		{
		  log_query(F_CONFIG | F_RRNAME, name, NULL, "<TXT>", 0);
		  if (add_resource_record(header, limit, &trunc, nameoffset, &ansp, 
					  ttl, NULL,
					  T_TXT, t->class, "t", t->len, t->txt))
		    anscount++;
		}
	    }
	}
    }
  
  if (qclass == C_CHAOS)
    {
      /* don't forward *.bind and *.server chaos queries - always reply with NOTIMP */
      if (hostname_issubdomain("bind", name) || hostname_issubdomain("server", name))
	{
	  if (!ans)
	    {
	      notimp = 1, auth = 0;
	      
	      addr.log.rcode = NOTIMP;
	      log_query(F_CONFIG | F_RCODE, name, &addr, NULL, 0);
		  
	      ans = 1, sec_data = 0;
	    }
	}
    }
  
  if (qclass == C_IN)
    {
      struct txt_record *t;
      
      for (t = daemon->rr; t; t = t->next)
	if ((t->class == qtype || qtype == T_ANY) && hostname_isequal(name, t->name))
	  {
	    ans = 1;
	    sec_data = 0;
	    log_query(F_CONFIG | F_RRNAME, name, NULL, NULL, t->class);
	    if (add_resource_record(header, limit, &trunc, nameoffset, &ansp, 
				    daemon->local_ttl, NULL,
				    t->class, C_IN, "t", t->len, t->txt))
	      anscount++;
	  }
      
      if (qtype == T_PTR || qtype == T_ANY)
	{
	  /* see if it's w.z.y.z.in-addr.arpa format */
	  int is_arpa = in_arpa_name_2_addr(name, &addr);
	  struct ptr_record *ptr;
	  struct interface_name* intr = NULL;
	  
	  for (ptr = daemon->ptr; ptr; ptr = ptr->next)
	    if (hostname_isequal(name, ptr->name))
	      break;
	  
	  if (is_arpa == F_IPV4)
	    for (intr = daemon->int_names; intr; intr = intr->next)
	      {
		struct addrlist *addrlist;
		
		for (addrlist = intr->addr; addrlist; addrlist = addrlist->next)
		  if (!(addrlist->flags & ADDRLIST_IPV6) && addr.addr4.s_addr == addrlist->addr.addr4.s_addr)
		    break;
		
		if (addrlist)
		  break;
		else if (!(intr->flags & INP4))
		  while (intr->next && strcmp(intr->intr, intr->next->intr) == 0)
		    intr = intr->next;
	      }
	  else if (is_arpa == F_IPV6)
	    for (intr = daemon->int_names; intr; intr = intr->next)
	      {
		struct addrlist *addrlist;
		
		for (addrlist = intr->addr; addrlist; addrlist = addrlist->next)
		  if ((addrlist->flags & ADDRLIST_IPV6) && IN6_ARE_ADDR_EQUAL(&addr.addr6, &addrlist->addr.addr6))
		    break;
		
		if (addrlist)
		  break;
		else if (!(intr->flags & INP6))
		  while (intr->next && strcmp(intr->intr, intr->next->intr) == 0)
		    intr = intr->next;
	      }
	  
	  if (intr)
	    {
	      sec_data = 0;
	      ans = 1;
	      log_query(is_arpa | F_REVERSE | F_CONFIG, intr->name, &addr, NULL, 0);
	      if (add_resource_record(header, limit, &trunc, nameoffset, &ansp, 
				      daemon->local_ttl, NULL,
				      T_PTR, C_IN, "d", intr->name))
		anscount++;
	    }
	  else if (ptr)
	    {
	      ans = 1;
	      sec_data = 0;
	      log_query(F_CONFIG | F_RRNAME, name, NULL, "<PTR>", 0);
	      for (ptr = daemon->ptr; ptr; ptr = ptr->next)
		if (hostname_isequal(name, ptr->name) &&
		    add_resource_record(header, limit, &trunc, nameoffset, &ansp, 
					daemon->local_ttl, NULL,
					T_PTR, C_IN, "d", ptr->ptr))
		  anscount++;
	      
	    }
	  else if (is_arpa && (crecp = cache_find_by_addr(NULL, &addr, now, is_arpa)))
	    {
	      /* Don't use cache when DNSSEC data required, unless we know that
		 the zone is unsigned, which implies that we're doing
		 validation. */
	      if ((crecp->flags & (F_HOSTS | F_DHCP | F_CONFIG)) ||
		  (rd_bit && (!do_bit || cache_validated(crecp)) ))
		{
		  do 
		    { 
		      int stale_flag = 0;
		      
		      if (crec_isstale(crecp, now))
			{
			  if (stale)
			    *stale = 1;
			  
			  stale_flag = F_STALE;
			}
		      
		      /* don't answer wildcard queries with data not from /etc/hosts or dhcp leases */
		      if (qtype == T_ANY && !(crecp->flags & (F_HOSTS | F_DHCP)))
			continue;
		      
		      if (!(crecp->flags & F_DNSSECOK))
			sec_data = 0;
		      
		      ans = 1;
		      
		      if (crecp->flags & F_NEG)
			{
			  auth = 0;
			  if (crecp->flags & F_NXDOMAIN)
			    nxdomain = 1;
			  log_query(stale_flag | (crecp->flags & ~F_FORWARD), name, &addr, NULL, 0);
			  soa_lookup = crecp;
			}
		      else
			{
			  if (!(crecp->flags & (F_HOSTS | F_DHCP)))
			    auth = 0;
			  
			  log_query(stale_flag | (crecp->flags & ~F_FORWARD), cache_get_name(crecp), &addr, 
				    record_source(crecp->uid), 0);
			  
			  if (add_resource_record(header, limit, &trunc, nameoffset, &ansp, 
						  crec_ttl(crecp, now), NULL,
						  T_PTR, C_IN, "d", cache_get_name(crecp)))
			    anscount++;
			}
		    } while ((crecp = cache_find_by_addr(crecp, &addr, now, is_arpa)));
		}
	    }
	  else if (is_rev_synth(is_arpa, &addr, name))
	    {
	      ans = 1;
	      sec_data = 0;
	      log_query(F_CONFIG | F_REVERSE | is_arpa, name, &addr, NULL, 0);
	      
	      if (add_resource_record(header, limit, &trunc, nameoffset, &ansp, 
				      daemon->local_ttl, NULL,
				      T_PTR, C_IN, "d", name))
		anscount++;
	    }
	  else if (option_bool(OPT_BOGUSPRIV) &&
		   ((is_arpa == F_IPV6 && private_net6(&addr.addr6, 1)) || (is_arpa == F_IPV4 && private_net(addr.addr4, 1))) &&
		   !lookup_domain(name, F_DOMAINSRV, NULL, NULL))
	    {
	      /* if no configured server, not in cache, enabled and private IPV4 address, return NXDOMAIN */
	      ans = 1;
	      sec_data = 0;
	      nxdomain = 1;
	      log_query(F_CONFIG | F_REVERSE | is_arpa | F_NEG | F_NXDOMAIN,
			name, &addr, NULL, 0);
	    }
	}
      
      for (flag = F_IPV4; flag; flag = (flag == F_IPV4) ? F_IPV6 : 0)
	{
	  unsigned short type = (flag == F_IPV6) ? T_AAAA : T_A;
	  struct interface_name *intr;
	  
	  if (qtype != type && qtype != T_ANY)
	    continue;
	  
	  /* interface name stuff */
	  for (intr = daemon->int_names; intr; intr = intr->next)
	    if (hostname_isequal(name, intr->name))
	      break;
	  
	  if (intr)
	    {
	      struct addrlist *addrlist;
	      int gotit = 0, localise = 0;
	      
	      enumerate_interfaces(0);
	      
	      /* See if a putative address is on the network from which we received
		 the query, is so we'll filter other answers. */
	      if (local_addr.s_addr != 0 && option_bool(OPT_LOCALISE) && type == T_A)
		for (intr = daemon->int_names; intr; intr = intr->next)
		  if (hostname_isequal(name, intr->name))
		    for (addrlist = intr->addr; addrlist; addrlist = addrlist->next)
		      if (!(addrlist->flags & ADDRLIST_IPV6) && 
			  is_same_net(addrlist->addr.addr4, local_addr, local_netmask))
			{
			  localise = 1;
			  break;
			}
	      
	      for (intr = daemon->int_names; intr; intr = intr->next)
		if (hostname_isequal(name, intr->name))
		  {
		    for (addrlist = intr->addr; addrlist; addrlist = addrlist->next)
		      if (((addrlist->flags & ADDRLIST_IPV6) ? T_AAAA : T_A) == type)
			{
			  if (localise && 
			      !is_same_net(addrlist->addr.addr4, local_addr, local_netmask))
			    continue;
			  
			  if (addrlist->flags & ADDRLIST_REVONLY)
			    continue;
			  
			  ans = 1;	
			  sec_data = 0;
			  gotit = 1;
			  log_query(F_FORWARD | F_CONFIG | flag, name, &addrlist->addr, NULL, 0);
			  if (add_resource_record(header, limit, &trunc, nameoffset, &ansp, 
						  daemon->local_ttl, NULL, type, C_IN, 
						  type == T_A ? "4" : "6", &addrlist->addr))
			    anscount++;
			}
		  }
	      
	      if (!gotit)
		log_query(F_FORWARD | F_CONFIG | flag | F_NEG, name, NULL, NULL, 0);
	      
	      continue;
	    }
	  
	  if ((crecp = cache_find_by_name(NULL, name, now, flag)))
	    {
	      int localise = 0;
	      
	      /* See if a putative address is on the network from which we received
		 the query, is so we'll filter other answers. */
	      if (!(crecp->flags & F_NEG) && local_addr.s_addr != 0 && option_bool(OPT_LOCALISE) && flag == F_IPV4)
		{
		  struct crec *save = crecp;
		  do {
		    if ((crecp->flags & F_HOSTS) &&
			is_same_net(crecp->addr.addr4, local_addr, local_netmask))
		      {
			localise = 1;
			break;
		      } 
		  } while ((crecp = cache_find_by_name(crecp, name, now, flag)));
		  crecp = save;
		}
	      
	      /* If the client asked for DNSSEC  don't use cached data. */
	      if ((crecp->flags & (F_HOSTS | F_DHCP | F_CONFIG)) ||
		  (rd_bit && (!do_bit || cache_validated(crecp)) ))
		do
		  { 
		    int stale_flag = 0;
		    
		    if (crec_isstale(crecp, now))
		      {
			if (stale)
			  *stale = 1;
			
			stale_flag = F_STALE;
		      }
		    
		    /* don't answer wildcard queries with data not from /etc/hosts
		       or DHCP leases */
		    if (qtype == T_ANY && !(crecp->flags & (F_HOSTS | F_DHCP | F_CONFIG)))
		      break;
		    
		    if (!(crecp->flags & F_DNSSECOK))
		      sec_data = 0;
		    
		    if (!(crecp->flags & (F_HOSTS | F_DHCP)))
		      auth = 0;
		    
		    if (qtype != T_ANY && rr_on_list(daemon->filter_rr, qtype) &&
			!(crecp->flags & (F_HOSTS | F_DHCP | F_CONFIG | F_NEG)))
		      {
			/* We have a cached answer but we're filtering it. */
			ans = 1;
			sec_data = 0;
			
			log_query(F_NEG | F_CONFIG | flag, name, NULL, NULL, 0);
			
			if (filtered)
			  *filtered = 1;
		      }
		    else if (crecp->flags & F_NEG)
		      {
			if (qtype != T_ANY)
			  {
			    ans = 1;
			    auth = 0;
			    soa_lookup = crecp;
			    if (crecp->flags & F_NXDOMAIN)
			      nxdomain = 1;
			    
			    log_query(stale_flag | crecp->flags, name, NULL, NULL, 0);
			  }
		      }
		    else 
		      {
			/* If we are returning local answers depending on network,
			   filter here. */
			if (localise && 
			    (crecp->flags & F_HOSTS) &&
			    !is_same_net(crecp->addr.addr4, local_addr, local_netmask))
			  continue;
			
			ans = 1;
			log_query(stale_flag | (crecp->flags & ~F_REVERSE), name, &crecp->addr,
				  record_source(crecp->uid), 0);
			
			if (add_resource_record(header, limit, &trunc, nameoffset, &ansp, 
						crec_ttl(crecp, now), NULL, type, C_IN, 
						type == T_A ? "4" : "6", &crecp->addr))
			  anscount++;
		      }
		  } while ((crecp = cache_find_by_name(crecp, name, now, flag)));
		}
	  else if (is_name_synthetic(flag, name, &addr))
	    {
	      ans = 1, sec_data = 0;
	      log_query(F_FORWARD | F_CONFIG | flag, name, &addr, NULL, 0);
	      if (add_resource_record(header, limit, &trunc, nameoffset, &ansp, 
				      daemon->local_ttl, NULL, type, C_IN, type == T_A ? "4" : "6", &addr))
		anscount++;
	    }
	}
      
      if (qtype == T_MX || qtype == T_ANY)
	{
	  int found = 0;
	  for (rec = daemon->mxnames; rec; rec = rec->next)
	    if (!rec->issrv && hostname_isequal(name, rec->name))
	      {
		int offset;
		
		ans = found = 1;
		sec_data = 0;
		
		log_query(F_CONFIG | F_RRNAME, name, NULL, "<MX>", 0);
		if (add_resource_record(header, limit, &trunc, nameoffset, &ansp, daemon->local_ttl,
					&offset, T_MX, C_IN, "sd", rec->weight, rec->target))
		  {
		    anscount++;
		    if (rec->target)
		      rec->offset = offset;
		  }
	      }
	  
	  if (!found && (option_bool(OPT_SELFMX) || option_bool(OPT_LOCALMX)) &&
	      cache_find_by_name(NULL, name, now, F_HOSTS | F_DHCP | F_NO_RR))
	    { 
	      ans = 1;
	      sec_data = 0;
	      log_query(F_CONFIG | F_RRNAME, name, NULL, "<MX>", 0);
	      if (add_resource_record(header, limit, &trunc, nameoffset, &ansp, daemon->local_ttl, NULL, 
				      T_MX, C_IN, "sd", 1, 
				      option_bool(OPT_SELFMX) ? name : daemon->mxtarget))
		anscount++;
	    }
	}
      
      if (qtype == T_SRV || qtype == T_ANY)
	{
	  struct mx_srv_record *move = NULL, **up = &daemon->mxnames;
	  
	  for (rec = daemon->mxnames; rec; rec = rec->next)
	    if (rec->issrv && hostname_isequal(name, rec->name))
	      {
		int offset;
		
		ans = 1;
		sec_data = 0;
		log_query(F_CONFIG | F_RRNAME, name, NULL, "<SRV>", 0);
		if (add_resource_record(header, limit, &trunc, nameoffset, &ansp, daemon->local_ttl, 
					&offset, T_SRV, C_IN, "sssd", 
					rec->priority, rec->weight, rec->srvport, rec->target))
		  {
		    anscount++;
		    if (rec->target)
		      rec->offset = offset;
		  }
		
		/* unlink first SRV record found */
		if (!move)
		  {
		    move = rec;
			*up = rec->next;
		  }
		else
		  up = &rec->next;      
	      }
	    else
	      up = &rec->next;
	  
	  /* put first SRV record back at the end. */
	  if (move)
	    {
	      *up = move;
	      move->next = NULL;
	    }
	}
      
      if (qtype == T_NAPTR || qtype == T_ANY)
	{
	  struct naptr *na;
	  for (na = daemon->naptr; na; na = na->next)
	    if (hostname_isequal(name, na->name))
	      {
		ans = 1;
		sec_data = 0;
		log_query(F_CONFIG | F_RRNAME, name, NULL, "<NAPTR>", 0);
		if (add_resource_record(header, limit, &trunc, nameoffset, &ansp, daemon->local_ttl, 
					NULL, T_NAPTR, C_IN, "sszzzd", 
					na->order, na->pref, na->flags, na->services, na->regexp, na->replace))
		  anscount++;
	      }
	}
      
      if (qtype == T_MAILB)
	ans = 1, nxdomain = 1, sec_data = 0;
      
      if (qtype == T_SOA && option_bool(OPT_FILTER))
	{
	  ans = 1;
	  sec_data = 0;
	  log_query(F_CONFIG | F_NEG, name, &addr, NULL, 0);
	}
      
      if (!ans)
	{
	  if ((crecp = cache_find_by_name(NULL, name, now, F_RR | F_NXDOMAIN)) &&
	      rd_bit && (!do_bit || cache_validated(crecp)))
	    do
	      {
		int flags = crecp->flags;
		unsigned short rrtype;
		
		if (flags & F_KEYTAG)
		  rrtype = crecp->addr.rrblock.rrtype;
		else
		  rrtype = crecp->addr.rrdata.rrtype;
		
		if ((flags & F_NXDOMAIN) || rrtype == qtype)
		  {
		    char *rrdata = NULL;
		    unsigned short rrlen = 0;
		    
		    if (crec_isstale(crecp, now))
		      {
			if (stale)
			  *stale = 1;
			
			flags |= F_STALE;
		      }
		    
		    if (!(flags & F_DNSSECOK))
		      sec_data = 0;
		    
		    if (flags & F_NXDOMAIN)
		      nxdomain = 1;
		    else if (qtype != T_ANY && rr_on_list(daemon->filter_rr, qtype))
		      flags |=  F_NEG | F_CONFIG;
		    
		    auth = 0;
		    ans = 1;

		    if (flags & F_NEG)
		      soa_lookup = crecp;
		    
		    if (!(flags & F_NEG))
		      {
			if (flags & F_KEYTAG)
			  {
			    rrlen = crecp->addr.rrblock.datalen;
			    rrdata = blockdata_retrieve(crecp->addr.rrblock.rrdata, crecp->addr.rrblock.datalen, NULL);
			  }
			else
			  {
			    rrlen = crecp->addr.rrdata.datalen;
			    rrdata = crecp->addr.rrdata.data;
			  }
		      }
		    
		    if (!(flags & F_NEG) && add_resource_record(header, limit, &trunc, nameoffset, &ansp, 
								crec_ttl(crecp, now), NULL, qtype, C_IN, "t",
								rrlen, rrdata))
		      anscount++;
		    
		    /* log after cache insertion as log_txt mangles rrdata */
		    if (qtype == T_TXT && !(crecp->flags & F_NEG))
		      log_txt(name, (unsigned char *)rrdata, rrlen, crecp->flags & F_DNSSECOK);
		    else
		      log_query(flags, name, &crecp->addr, NULL, 0);
		  }
	      } while ((crecp = cache_find_by_name(crecp, name, now, F_RR)));
	}
      
      if (!ans && option_bool(OPT_FILTER) && (qtype == T_SRV || (qtype == T_ANY && strchr(name, '_'))))
	{
	  ans = 1;
	  sec_data = 0;
	  log_query(F_CONFIG | F_NEG, name, NULL, NULL, 0);
	}
      
      
      if (qtype != T_ANY && !ans && rr_on_list(daemon->filter_rr, qtype))
	{
	  /* We don't have a cached answer and when we get an answer from upstream we're going to
	     filter it anyway. If we have a cached answer for the domain for another RRtype then
	     that may be enough to tell us if the answer should be NODATA and save the round trip.
	     Cached NXDOMAIN has already been handled, so here we look for any record for the domain,
	     since its existence allows us to return a NODATA answer. Note that we never set the AD flag,
	     since we didn't authenticate the record. */
	  
	  if (cache_find_by_name(NULL, name, now, F_IPV4 | F_IPV6 | F_RR | F_CNAME))
	    {
	      ans = 1;
	      sec_data = auth = 0;
	      
	      log_query(F_NEG | F_CONFIG | flag, name, NULL, NULL, 0);
	      
	      if (filtered)
		*filtered = 1;
	    }
	}
    }
  
  if (!ans)
    return 0; /* failed to answer a question */

  /* We found a negative record. See if we have an SOA record to 
     return in the AUTH section. 
     
     For FORWARD NEG records, the addr.rrdata.datalen field of the othewise
     empty addr is used to held an offset in to the name which yields the SOA
     name. For REVERSE NEG records, the otherwise empty name field holds the
     SOA name. If soa_name has zero length, then no SOA is known. soa_lookup
     MUST be a neg record here.
     
     If the F_NO_RR flag is set, there was no SOA record supplied with the RR.  */
  if (soa_lookup && !(soa_lookup->flags & F_NO_RR))
    {
      char *soa_name = soa_lookup->flags & F_REVERSE ? cache_get_name(soa_lookup) : name + soa_lookup->addr.rrdata.datalen;
      
      crecp = NULL;
      while ((crecp = cache_find_by_name(crecp, soa_name, now, F_RR)))
	if (crecp->addr.rrblock.rrtype == T_SOA)
	  {
	    char *rrdata;
	    
	    if (!(crecp->flags & F_NEG) &&
		(rrdata = blockdata_retrieve(crecp->addr.rrblock.rrdata, crecp->addr.rrblock.datalen, NULL)) &&
		add_resource_record(header, limit, &trunc, 0, &ansp, 
				    crec_ttl(crecp, now), NULL, T_SOA, C_IN, "t",
				    soa_name, crecp->addr.rrblock.datalen, rrdata))
	      {
		nscount++;
		
		if (!(crecp->flags & F_DNSSECOK))
		  sec_data = 0;
	      }
	    break;
	  }
    }
      
  /* create an additional data section, for stuff in SRV and MX record replies. */
  for (rec = daemon->mxnames; rec; rec = rec->next)
    if (rec->offset != 0)
      {
	/* squash dupes */
	struct mx_srv_record *tmp;
	for (tmp = rec->next; tmp; tmp = tmp->next)
	  if (tmp->offset != 0 && hostname_isequal(rec->target, tmp->target))
	    tmp->offset = 0;
	
	crecp = NULL;
	while ((crecp = cache_find_by_name(crecp, rec->target, now, F_IPV4 | F_IPV6)))
	  {
	    int type =  crecp->flags & F_IPV4 ? T_A : T_AAAA;

	    if (crecp->flags & F_NEG)
	      continue;

	    if (add_resource_record(header, limit, NULL, rec->offset, &ansp, 
				    crec_ttl(crecp, now), NULL, type, C_IN, 
				    crecp->flags & F_IPV4 ? "4" : "6", &crecp->addr))
	      {
		addncount++;
		if (!(crecp->flags & F_DNSSECOK))
		  sec_data = 0;
	      }
	  }
      }
  
  /* done all questions, set up header and return length of result */
  /* clear authoritative and truncated flags, set QR flag */
  header->hb3 = (header->hb3 & ~(HB3_AA | HB3_TC)) | HB3_QR;
  /* set RA flag */
  header->hb4 |= HB4_RA;
   
  /* authoritative - only hosts and DHCP derived names. */
  if (auth)
    header->hb3 |= HB3_AA;
  
  /* truncation */
  if (trunc)
    header->hb3 |= HB3_TC;
  
  if (nxdomain)
    SET_RCODE(header, NXDOMAIN);
  else if (notimp)
    SET_RCODE(header, NOTIMP);
  else
    SET_RCODE(header, NOERROR); /* no error */
  header->ancount = htons(anscount);
  header->nscount = htons(nscount);
  header->arcount = htons(addncount);

  len = ansp - (unsigned char *)header;
  
  /* Advertise our packet size limit in our reply */
  if (have_pseudoheader)
    len = add_pseudoheader(header, len, (unsigned char *)limit, daemon->edns_pktsz, 0, NULL, 0, do_bit, 0);
  
  if (ad_reqd && sec_data)
    header->hb4 |= HB4_AD;
  else
    header->hb4 &= ~HB4_AD;
  
  return len;
}
