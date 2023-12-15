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

#include "chilli.h"
#include "debug.h"

#define antidnstunnel _options.dnsparanoia

extern struct dhcp_t *dhcp;

ssize_t
dns_fullname(char *data, size_t dlen,      /* buffer to store name */
	     uint8_t *res, size_t reslen,  /* current resource */
	     uint8_t *opkt, size_t olen,   /* original packet */
	     int lvl) {
  int ret = 0;
  char *d = data;
  unsigned char l;

  if (lvl >= 15) return -1;

#if(_debug_ > 1)
  log_dbg("%s dlen=%d reslen=%d olen=%d lvl=%d", 
	  __FUNCTION__, dlen, reslen, olen, lvl);
#endif

  /* only capture the first name in query */
  if (d && d[0]) d = 0;
  
  while (reslen-- > 0 && ++ret && (l = *res++) != 0) {

    if ((l & 0xC0) == 0xC0) {
      if (reslen == 0) return -1;
      else {
	unsigned short offset = ((l & ~0xC0) << 8) + *res;

	ret++;
	
	if (offset > olen) {
	  log_dbg("bad value");
	  return -1;
	}
	
#if(_debug_ > 1)
	log_dbg("skip[%d] olen=%d", offset, olen);
#endif
	
	if (dns_fullname(d, dlen, 
			 opkt + (size_t) offset, 
			 olen - (size_t) offset, 
			 opkt, olen, lvl+1) < 0)
	  return -1;
	break;
      } 
    }
    
    if (l >= dlen || l >= olen) {
      log_dbg("bad value %d/%d/%d", l, dlen, olen);
      return -1;
    }
    
#if(_debug_ > 1)
    log_dbg("part[%.*s] reslen=%d l=%d dlen=%d",
	    l, res, reslen, l, dlen);
#endif

    if (d) {
      memcpy(d, res, l);
      d += l; 
      dlen -= l;
    }
    res += l;
    reslen -= l;
    ret += l;

    if (d) {
      *d = '.';
      d += 1; 
      dlen -= 1;
    }
  }
  
  if (lvl == 0 && d) {
    int len = strlen((char *)data);
    if (len && len == (d - data) && data[len-1] == '.')
      data[len-1]=0;
  }

  return ret;
}

static void 
add_A_to_garden(uint8_t *p) {
  struct in_addr reqaddr;
  pass_through pt;
  memcpy(&reqaddr.s_addr, p, 4);
  memset(&pt, 0, sizeof(pass_through));
  pt.mask.s_addr = 0xffffffff;
  pt.host = reqaddr;
  if (pass_through_add(dhcp->pass_throughs,
		       MAX_PASS_THROUGHS,
		       &dhcp->num_pass_throughs,
		       &pt, 1
#ifdef HAVE_PATRICIA
		       , dhcp->ptree_dyn
#endif
		       ))
    ;
}

int 
dns_copy_res(struct dhcp_conn_t *conn, int q, 
	     uint8_t **pktp, size_t *left, 
	     uint8_t *opkt,  size_t olen, 
	     uint8_t *question, size_t qsize,
	     int isReq, int *qmatch, int *modified, int mode) {

#define return_error { log_dbg("failed parsing DNS packet"); return -1; }

  uint8_t *p_pkt = *pktp;
  size_t len = *left;
  
  uint8_t name[PKT_IP_PLEN];
  ssize_t namelen = 0;
  char required = 0;
  
  uint16_t type;
  uint16_t class;
  uint32_t ttl;
  uint16_t rdlen;

#ifdef ENABLE_IPV6
  uint8_t *pkt_type=0;
#endif
  uint8_t *pkt_ttl=0;

  uint32_t ul;
  uint16_t us;

#if(_debug_ > 1)
  log_dbg("%s: left=%d olen=%d qsize=%d",
	  __FUNCTION__, *left, olen, qsize);
#endif

  memset(name, 0, sizeof(name));
  namelen = dns_fullname((char*)name, sizeof(name)-1, 
			 p_pkt, len, opkt, olen, 0);

  if (namelen < 0 || namelen > len) return_error;

  p_pkt += namelen;
  len -= namelen;

  if (antidnstunnel && namelen > 128) {
    log_warn(0,"dropping dns for anti-dnstunnel (namelen: %d)", namelen);
    return -1;
  }

  if (len < 4) return_error;

#ifdef ENABLE_IPV6
  pkt_type = p_pkt;
#endif
  memcpy(&us, p_pkt, sizeof(us));
  type = ntohs(us);
  p_pkt += 2;
  len -= 2;
  
  memcpy(&us, p_pkt, sizeof(us));
  class = ntohs(us);
  p_pkt += 2;
  len -= 2;
  
#if(_debug_)
  log_dbg("It was a dns record type: %d class: %d", type, class);
#endif

  if (q) {
    if (dns_fullname((char *)question, qsize, *pktp, *left, opkt, olen, 0) < 0)
      return_error;

    log_dbg("DNS: %s", question);
    
    *pktp = p_pkt;
    *left = len;

    if (!isReq && *qmatch == -1 && 
	_options.uamdomains && _options.uamdomains[0]) {
      int id;

      for (id=0; _options.uamdomains[id] && id < MAX_UAM_DOMAINS; id++) {
	
	size_t qst_len = strlen((char *)question);
	size_t dom_len = strlen(_options.uamdomains[id]);
	
#if(_debug_)
	log_dbg("checking %s [%s]",
		_options.uamdomains[id], question);
#endif
	
	if ( qst_len && dom_len && 
	     (
	      /*
	       *  Match if question equals the uamdomain
	       */
	      ( qst_len == dom_len &&
		!strcmp(_options.uamdomains[id], (char *)question) ) ||
	      /*
	       *  Match if the question is longer than uamdomain,
	       *  and ends with the '.' followed by uamdomain
	       */
	      ( qst_len > dom_len && 
		(_options.uamdomains[id][0] == '.' ||
		 question[qst_len - dom_len - 1] == '.') &&
		!strcmp(_options.uamdomains[id], 
			(char *)question + qst_len - dom_len) )
	      ) ) {
#if(_debug_)
	  log_dbg("matched %s [%s]", _options.uamdomains[id], question);
#endif
	  *qmatch = 1;
	  break;
	}
      }
    }

#ifdef ENABLE_UAMDOMAINFILE
    if (!isReq && *qmatch == -1 && _options.uamdomainfile) {
      *qmatch = garden_check_domainfile((char *) question);
    }
#endif

#ifdef ENABLE_IPV6
    if (_options.ipv6) {
      if (isReq && type == 28) {
	log_dbg("changing AAAA to A request");
	us = 1;
	us = htons(us);
	memcpy(pkt_type, &us, sizeof(us));
	*modified = 1;
	
      } /*else if (!isReq && type == 1) {   //John add for test@10.27
	log_dbg("changing A to AAAA response");
	us = 28;
	us = htons(us);
	memcpy(pkt_type, &us, sizeof(us));
	*modified = 1;
      }*/
    }
#endif

    return 0;
  } 

  if (len < 6) return_error;

  pkt_ttl = p_pkt;
  memcpy(&ul, p_pkt, sizeof(ul));
  ttl = ntohl(ul);
  p_pkt += 4;
  len -= 4;
  
  memcpy(&us, p_pkt, sizeof(us));
  rdlen = ntohs(us);
  p_pkt += 2;
  len -= 2;
  
#if(_debug_ > 1)
  log_dbg("-> w ttl: %d rdlength: %d/%d", ttl, rdlen, len);
#endif

  if (*qmatch == 1 && ttl > _options.uamdomain_ttl) {
#if(_debug_)
    log_dbg("Rewriting DNS ttl from %d to %d", 
	    (int) ttl, _options.uamdomain_ttl);
#endif
    ul = _options.uamdomain_ttl;
    ul = htonl(ul);
    memcpy(pkt_ttl, &ul, sizeof(ul));
    *modified = 1;
  }

  if (len < rdlen) return_error;
  
  /*
   *  dns records 
   */  
  
  switch (type) {

  default: 
    log_dbg("Record type %d", type);
    return_error;
    break;
    
  case 1:  
#if(_debug_ > 1)
    log_dbg("A record");
#endif
    required = 1;

#ifdef ENABLE_MDNS
    if (mode == DNS_MDNS_MODE) {
      size_t offset;
      for (offset=0; offset < rdlen; offset += 4) {
	struct in_addr reqaddr;
	memcpy(&reqaddr.s_addr, p_pkt+offset, 4);
#if(_debug_)
	log_dbg("mDNS %s = %s", name, inet_ntoa(reqaddr));
#endif
      }
      break;
    }
#endif    

    if (*qmatch == 1) {
      size_t offset;
      for (offset=0; offset < rdlen; offset += 4) {
	add_A_to_garden(p_pkt+offset);
      }
    }
    break;

  case 2: log_dbg("NS record"); required = 1; break;
  case 5: log_dbg("CNAME record %s", name); required = 1; break;
  case 6: log_dbg("SOA record"); break;
    
  case 12: log_dbg("PTR record"); break;
  case 15: log_dbg("MX record"); required = 1; break;

  case 16:/* TXT */
    log_dbg("TXT record %d", rdlen);
    if (_options.debug) {
      char *txt = (char *)p_pkt;
      int txtlen = rdlen;
      while (txtlen-- > 0) {
	uint8_t l = *txt++;
	if (l == 0) break;
	log_dbg("Text: %.*s", (int) l, txt);
	txt += l;
	txtlen -= l;
      }
    }
    break;

  case 28: 
    log_dbg("AAAA record"); 
    required = 1; 
    break;
  case 29: log_dbg("LOC record"); break;
  case 33: log_dbg("SRV record"); break;
  case 47: log_dbg("NSEC record"); break;
  }

  if (antidnstunnel && !required) {
    log_warn(0, "dropping dns for anti-dnstunnel (type %d: length %d)", 
	     type, rdlen);
    return -1;
  }
  
  p_pkt += rdlen;
  len -= rdlen;
  
  *pktp = p_pkt;
  *left = len;

  return 0;
}
