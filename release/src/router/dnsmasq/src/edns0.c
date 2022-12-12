/* dnsmasq is Copyright (c) 2000-2022 Simon Kelley

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

unsigned char *find_pseudoheader(struct dns_header *header, size_t plen, size_t  *len, unsigned char **p, int *is_sign, int *is_last)
{
  /* See if packet has an RFC2671 pseudoheader, and if so return a pointer to it. 
     also return length of pseudoheader in *len and pointer to the UDP size in *p
     Finally, check to see if a packet is signed. If it is we cannot change a single bit before
     forwarding. We look for TSIG in the addition section, and TKEY queries (for GSS-TSIG) */
  
  int i, arcount = ntohs(header->arcount);
  unsigned char *ansp = (unsigned char *)(header+1);
  unsigned short rdlen, type, class;
  unsigned char *ret = NULL;

  if (is_sign)
    {
      *is_sign = 0;

      if (OPCODE(header) == QUERY)
	{
	  for (i = ntohs(header->qdcount); i != 0; i--)
	    {
	      if (!(ansp = skip_name(ansp, header, plen, 4)))
		return NULL;
	      
	      GETSHORT(type, ansp); 
	      GETSHORT(class, ansp);
	      
	      if (class == C_IN && type == T_TKEY)
		*is_sign = 1;
	    }
	}
    }
  else
    {
      if (!(ansp = skip_questions(header, plen)))
	return NULL;
    }
    
  if (arcount == 0)
    return NULL;
  
  if (!(ansp = skip_section(ansp, ntohs(header->ancount) + ntohs(header->nscount), header, plen)))
    return NULL; 
  
  for (i = 0; i < arcount; i++)
    {
      unsigned char *save, *start = ansp;
      if (!(ansp = skip_name(ansp, header, plen, 10)))
	return NULL; 

      GETSHORT(type, ansp);
      save = ansp;
      GETSHORT(class, ansp);
      ansp += 4; /* TTL */
      GETSHORT(rdlen, ansp);
      if (!ADD_RDLEN(header, ansp, plen, rdlen))
	return NULL;
      if (type == T_OPT)
	{
	  if (len)
	    *len = ansp - start;

	  if (p)
	    *p = save;
	  
	  if (is_last)
	    *is_last = (i == arcount-1);

	  ret = start;
	}
      else if (is_sign && 
	       i == arcount - 1 && 
	       class == C_ANY && 
	       type == T_TSIG)
	*is_sign = 1;
    }
  
  return ret;
}
 

/* replace == 2 ->delete existing option only. */
size_t add_pseudoheader(struct dns_header *header, size_t plen, unsigned char *limit, 
			unsigned short udp_sz, int optno, unsigned char *opt, size_t optlen, int set_do, int replace)
{ 
  unsigned char *lenp, *datap, *p, *udp_len, *buff = NULL;
  int rdlen = 0, is_sign, is_last;
  unsigned short flags = set_do ? 0x8000 : 0, rcode = 0;

  p = find_pseudoheader(header, plen, NULL, &udp_len, &is_sign, &is_last);
  
  if (is_sign)
    return plen;

  if (p)
    {
      /* Existing header */
      int i;
      unsigned short code, len;

      p = udp_len;
      GETSHORT(udp_sz, p);
      GETSHORT(rcode, p);
      GETSHORT(flags, p);

      if (set_do)
	{
	  p -= 2;
	  flags |= 0x8000;
	  PUTSHORT(flags, p);
	}

      lenp = p;
      GETSHORT(rdlen, p);
      if (!CHECK_LEN(header, p, plen, rdlen))
	return plen; /* bad packet */
      datap = p;

       /* no option to add */
      if (optno == 0)
	return plen;
      	  
      /* check if option already there */
      for (i = 0; i + 4 < rdlen;)
	{
	  GETSHORT(code, p);
	  GETSHORT(len, p);
	  
	  /* malformed option, delete the whole OPT RR and start again. */
	  if (i + 4 + len > rdlen)
	    {
	      rdlen = 0;
	      is_last = 0;
	      break;
	    }
	  
	  if (code == optno)
	    {
	      if (replace == 0)
		return plen;

	      /* delete option if we're to replace it. */
	      p -= 4;
	      rdlen -= len + 4;
	      memmove(p, p+len+4, rdlen - i);
	      PUTSHORT(rdlen, lenp);
	      lenp -= 2;
	    }
	  else
	    {
	      p += len;
	      i += len + 4;
	    }
	}

      /* If we're going to extend the RR, it has to be the last RR in the packet */
      if (!is_last)
	{
	  /* First, take a copy of the options. */
	  if (rdlen != 0 && (buff = whine_malloc(rdlen)))
	    memcpy(buff, datap, rdlen);	      
	  
	  /* now, delete OPT RR */
	  plen = rrfilter(header, plen, RRFILTER_EDNS0);
	  
	  /* Now, force addition of a new one */
	  p = NULL;	  
	}
    }
  
  if (!p)
    {
      /* We are (re)adding the pseudoheader */
      if (!(p = skip_questions(header, plen)) ||
	  !(p = skip_section(p, 
			     ntohs(header->ancount) + ntohs(header->nscount) + ntohs(header->arcount), 
			     header, plen)))
      {
	free(buff);
	return plen;
      }
      if (p + 11 > limit)
      {
        free(buff);
        return plen; /* Too big */
      }
      *p++ = 0; /* empty name */
      PUTSHORT(T_OPT, p);
      PUTSHORT(udp_sz, p); /* max packet length, 512 if not given in EDNS0 header */
      PUTSHORT(rcode, p);    /* extended RCODE and version */
      PUTSHORT(flags, p); /* DO flag */
      lenp = p;
      PUTSHORT(rdlen, p);    /* RDLEN */
      datap = p;
      /* Copy back any options */
      if (buff)
	{
          if (p + rdlen > limit)
          {
            free(buff);
            return plen; /* Too big */
          }
	  memcpy(p, buff, rdlen);
	  free(buff);
	  p += rdlen;
	}
      
      /* Only bump arcount if RR is going to fit */ 
      if (((ssize_t)optlen) <= (limit - (p + 4)))
	header->arcount = htons(ntohs(header->arcount) + 1);
    }
  
  if (((ssize_t)optlen) > (limit - (p + 4)))
    return plen; /* Too big */
  
  /* Add new option */
  if (optno != 0 && replace != 2)
    {
      if (p + 4 > limit)
       return plen; /* Too big */
      PUTSHORT(optno, p);
      PUTSHORT(optlen, p);
      if (p + optlen > limit)
       return plen; /* Too big */
      memcpy(p, opt, optlen);
      p += optlen;  
      PUTSHORT(p - datap, lenp);
    }
  return p - (unsigned char *)header;
}

size_t add_do_bit(struct dns_header *header, size_t plen, unsigned char *limit)
{
  return add_pseudoheader(header, plen, (unsigned char *)limit, PACKETSZ, 0, NULL, 0, 1, 0);
}

static unsigned char char64(unsigned char c)
{
  return "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[c & 0x3f];
}

static void encoder(unsigned char *in, char *out)
{
  out[0] = char64(in[0]>>2);
  out[1] = char64((in[0]<<4) | (in[1]>>4));
  out[2] = char64((in[1]<<2) | (in[2]>>6));
  out[3] = char64(in[2]);
}

/* OPT_ADD_MAC = MAC is added (if available)
   OPT_ADD_MAC + OPT_STRIP_MAC = MAC is replaced, if not available, it is only removed
   OPT_STRIP_MAC = MAC is removed */
static size_t add_dns_client(struct dns_header *header, size_t plen, unsigned char *limit,
			     union mysockaddr *l3, time_t now, int *cacheablep)
{
  int replace = 0, maclen = 0;
  unsigned char mac[DHCP_CHADDR_MAX];
  char encode[18]; /* handle 6 byte MACs ONLY */

  if ((option_bool(OPT_MAC_B64) || option_bool(OPT_MAC_HEX)) && (maclen = find_mac(l3, mac, 1, now)) == 6)
    {
      if (option_bool(OPT_STRIP_MAC))
	 replace = 1;
       *cacheablep = 0;
    
       if (option_bool(OPT_MAC_HEX))
	 print_mac(encode, mac, maclen);
       else
	 {
	   encoder(mac, encode);
	   encoder(mac+3, encode+4);
	   encode[8] = 0;
	 }
    }
  else if (option_bool(OPT_STRIP_MAC))
    replace = 2;

  if (replace != 0 || maclen == 6)
    plen = add_pseudoheader(header, plen, limit, PACKETSZ, EDNS0_OPTION_NOMDEVICEID, (unsigned char *)encode, strlen(encode), 0, replace);

  return plen;
}


/* OPT_ADD_MAC = MAC is added (if available)
   OPT_ADD_MAC + OPT_STRIP_MAC = MAC is replaced, if not available, it is only removed
   OPT_STRIP_MAC = MAC is removed */
static size_t add_mac(struct dns_header *header, size_t plen, unsigned char *limit,
		      union mysockaddr *l3, time_t now, int *cacheablep)
{
  int maclen = 0, replace = 0;
  unsigned char mac[DHCP_CHADDR_MAX];
    
  if (option_bool(OPT_ADD_MAC) && (maclen = find_mac(l3, mac, 1, now)) != 0)
    {
      *cacheablep = 0;
      if (option_bool(OPT_STRIP_MAC))
	replace = 1;
    }
  else if (option_bool(OPT_STRIP_MAC))
    replace = 2;
  
  if (replace != 0 || maclen != 0)
    plen = add_pseudoheader(header, plen, limit, PACKETSZ, EDNS0_OPTION_MAC, mac, maclen, 0, replace);

  return plen; 
}

struct subnet_opt {
  u16 family;
  u8 source_netmask, scope_netmask; 
  u8 addr[IN6ADDRSZ];
};

static void *get_addrp(union mysockaddr *addr, const short family) 
{
  if (family == AF_INET6)
    return &addr->in6.sin6_addr;

  return &addr->in.sin_addr;
}

static size_t calc_subnet_opt(struct subnet_opt *opt, union mysockaddr *source, int *cacheablep)
{
  /* http://tools.ietf.org/html/draft-vandergaast-edns-client-subnet-02 */
  
  int len;
  void *addrp = NULL;
  int sa_family = source->sa.sa_family;
  int cacheable = 0;
  
  opt->source_netmask = 0;
  opt->scope_netmask = 0;
    
  if (source->sa.sa_family == AF_INET6 && daemon->add_subnet6)
    {
      opt->source_netmask = daemon->add_subnet6->mask;
      if (daemon->add_subnet6->addr_used) 
	{
	  sa_family = daemon->add_subnet6->addr.sa.sa_family;
	  addrp = get_addrp(&daemon->add_subnet6->addr, sa_family);
	  cacheable = 1;
	} 
      else 
	addrp = &source->in6.sin6_addr;
    }

  if (source->sa.sa_family == AF_INET && daemon->add_subnet4)
    {
      opt->source_netmask = daemon->add_subnet4->mask;
      if (daemon->add_subnet4->addr_used)
	{
	  sa_family = daemon->add_subnet4->addr.sa.sa_family;
	  addrp = get_addrp(&daemon->add_subnet4->addr, sa_family);
	  cacheable = 1; /* Address is constant */
	} 
	else 
	  addrp = &source->in.sin_addr;
    }
  
  opt->family = htons(sa_family == AF_INET6 ? 2 : 1);
  
  if (addrp && opt->source_netmask != 0)
    {
      len = ((opt->source_netmask - 1) >> 3) + 1;
      memcpy(opt->addr, addrp, len);
      if (opt->source_netmask & 7)
	opt->addr[len-1] &= 0xff << (8 - (opt->source_netmask & 7));
    }
  else
    {
      cacheable = 1; /* No address ever supplied. */
      len = 0;
    }

  if (cacheablep)
    *cacheablep = cacheable;
  
  return len + 4;
}
 
/* OPT_CLIENT_SUBNET = client subnet is added
   OPT_CLIENT_SUBNET + OPT_STRIP_ECS = client subnet is replaced
   OPT_STRIP_ECS = client subnet is removed */
static size_t add_source_addr(struct dns_header *header, size_t plen, unsigned char *limit,
			      union mysockaddr *source, int *cacheable)
{
  /* http://tools.ietf.org/html/draft-vandergaast-edns-client-subnet-02 */
  
  int replace = 0, len = 0;
  struct subnet_opt opt;
  
  if (option_bool(OPT_CLIENT_SUBNET))
    {
      if (option_bool(OPT_STRIP_ECS))
	replace = 1;
      len = calc_subnet_opt(&opt, source, cacheable);
    }
  else if (option_bool(OPT_STRIP_ECS))
    replace = 2;
  else
    return plen;

  return add_pseudoheader(header, plen, (unsigned char *)limit, PACKETSZ, EDNS0_OPTION_CLIENT_SUBNET, (unsigned char *)&opt, len, 0, replace);
}

int check_source(struct dns_header *header, size_t plen, unsigned char *pseudoheader, union mysockaddr *peer)
{
  /* Section 9.2, Check that subnet option in reply matches. */
  
  int len, calc_len;
  struct subnet_opt opt;
  unsigned char *p;
  int code, i, rdlen;
  
  calc_len = calc_subnet_opt(&opt, peer, NULL);
   
  if (!(p = skip_name(pseudoheader, header, plen, 10)))
    return 1;
  
  p += 8; /* skip UDP length and RCODE */
  
  GETSHORT(rdlen, p);
  if (!CHECK_LEN(header, p, plen, rdlen))
    return 1; /* bad packet */
  
  /* check if option there */
   for (i = 0; i + 4 < rdlen; i += len + 4)
     {
       GETSHORT(code, p);
       GETSHORT(len, p);
       if (code == EDNS0_OPTION_CLIENT_SUBNET)
	 {
	   /* make sure this doesn't mismatch. */
	   opt.scope_netmask = p[3];
	   if (len != calc_len || memcmp(p, &opt, len) != 0)
	     return 0;
	 }
       p += len;
     }
   
   return 1;
}

/* See https://docs.umbrella.com/umbrella-api/docs/identifying-dns-traffic for
 * detailed information on packet formating.
 */
#define UMBRELLA_VERSION    1
#define UMBRELLA_TYPESZ     2

#define UMBRELLA_ASSET      0x0004
#define UMBRELLA_ASSETSZ    sizeof(daemon->umbrella_asset)
#define UMBRELLA_ORG        0x0008
#define UMBRELLA_ORGSZ      sizeof(daemon->umbrella_org)
#define UMBRELLA_IPV4       0x0010
#define UMBRELLA_IPV6       0x0020
#define UMBRELLA_DEVICE     0x0040
#define UMBRELLA_DEVICESZ   sizeof(daemon->umbrella_device)

struct umbrella_opt {
  u8 magic[4];
  u8 version;
  u8 flags;
  /* We have 4 possible fields since we'll never send both IPv4 and
   * IPv6, so using the larger of the two to calculate max buffer size.
   * Each field also has a type header.  So the following accounts for
   * the type headers and each field size to get a max buffer size.
   */
  u8 fields[4 * UMBRELLA_TYPESZ + UMBRELLA_ORGSZ + IN6ADDRSZ + UMBRELLA_DEVICESZ + UMBRELLA_ASSETSZ];
};

static size_t add_umbrella_opt(struct dns_header *header, size_t plen, unsigned char *limit, union mysockaddr *source, int *cacheable)
{
  *cacheable = 0;

  struct umbrella_opt opt = {{"ODNS"}, UMBRELLA_VERSION, 0, {}};
  u8 *u = &opt.fields[0];
  int family = source->sa.sa_family;
  int size = family == AF_INET ? INADDRSZ : IN6ADDRSZ;

  if (daemon->umbrella_org)
    {
      PUTSHORT(UMBRELLA_ORG, u);
      PUTLONG(daemon->umbrella_org, u);
    }
  
  PUTSHORT(family == AF_INET ? UMBRELLA_IPV4 : UMBRELLA_IPV6, u);
  memcpy(u, get_addrp(source, family), size);
  u += size;
  
  if (option_bool(OPT_UMBRELLA_DEVID))
    {
      PUTSHORT(UMBRELLA_DEVICE, u);
      memcpy(u, (char *)&daemon->umbrella_device, UMBRELLA_DEVICESZ);
      u += UMBRELLA_DEVICESZ;
    }

  if (daemon->umbrella_asset)
    {
      PUTSHORT(UMBRELLA_ASSET, u);
      PUTLONG(daemon->umbrella_asset, u);
    }
  
  return add_pseudoheader(header, plen, (unsigned char *)limit, PACKETSZ, EDNS0_OPTION_UMBRELLA, (unsigned char *)&opt, u - (u8 *)&opt, 0, 1);
}

/* Set *check_subnet if we add a client subnet option, which needs to checked 
   in the reply. Set *cacheable to zero if we add an option which the answer
   may depend on. */
size_t add_edns0_config(struct dns_header *header, size_t plen, unsigned char *limit, 
			union mysockaddr *source, time_t now, int *cacheable)    
{
  *cacheable = 1;
  
  plen  = add_mac(header, plen, limit, source, now, cacheable);
  plen = add_dns_client(header, plen, limit, source, now, cacheable);
  
  if (daemon->dns_client_id)
    plen = add_pseudoheader(header, plen, limit, PACKETSZ, EDNS0_OPTION_NOMCPEID, 
			    (unsigned char *)daemon->dns_client_id, strlen(daemon->dns_client_id), 0, 1);

  if (option_bool(OPT_UMBRELLA))
    plen = add_umbrella_opt(header, plen, limit, source, cacheable);
  
  plen = add_source_addr(header, plen, limit, source, cacheable);
  	  
  return plen;
}
