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

static struct frec *get_new_frec(time_t now, struct server *serv, int force);
static struct frec *lookup_frec(unsigned short id, int fd, void *hash, int *firstp, int *lastp);
static struct frec *lookup_frec_by_query(void *hash, unsigned int flags, unsigned int flagmask);
#ifdef HAVE_DNSSEC
static struct frec *lookup_frec_dnssec(char *target, int class, int flags, struct dns_header *header);
#endif

static unsigned short get_id(void);
static void free_frec(struct frec *f);
static void query_full(time_t now, char *domain);

static void return_reply(time_t now, struct frec *forward, struct dns_header *header, ssize_t n, int status);

/* Send a UDP packet with its source address set as "source" 
   unless nowild is true, when we just send it with the kernel default */
int send_from(int fd, int nowild, char *packet, size_t len, 
	      union mysockaddr *to, union all_addr *source,
	      unsigned int iface)
{
  struct msghdr msg;
  struct iovec iov[1]; 
  union {
    struct cmsghdr align; /* this ensures alignment */
#if defined(HAVE_LINUX_NETWORK)
    char control[CMSG_SPACE(sizeof(struct in_pktinfo))];
#elif defined(IP_SENDSRCADDR)
    char control[CMSG_SPACE(sizeof(struct in_addr))];
#endif
    char control6[CMSG_SPACE(sizeof(struct in6_pktinfo))];
  } control_u;
  
  iov[0].iov_base = packet;
  iov[0].iov_len = len;

  msg.msg_control = NULL;
  msg.msg_controllen = 0;
  msg.msg_flags = 0;
  msg.msg_name = to;
  msg.msg_namelen = sa_len(to);
  msg.msg_iov = iov;
  msg.msg_iovlen = 1;
  
  if (!nowild)
    {
      struct cmsghdr *cmptr;
      msg.msg_control = &control_u;
      msg.msg_controllen = sizeof(control_u);
      cmptr = CMSG_FIRSTHDR(&msg);

      if (to->sa.sa_family == AF_INET)
	{
#if defined(HAVE_LINUX_NETWORK)
	  struct in_pktinfo p;
	  p.ipi_ifindex = 0;
	  p.ipi_spec_dst = source->addr4;
	  msg.msg_controllen = CMSG_SPACE(sizeof(struct in_pktinfo));
	  memcpy(CMSG_DATA(cmptr), &p, sizeof(p));
	  cmptr->cmsg_len = CMSG_LEN(sizeof(struct in_pktinfo));
	  cmptr->cmsg_level = IPPROTO_IP;
	  cmptr->cmsg_type = IP_PKTINFO;
#elif defined(IP_SENDSRCADDR)
	  msg.msg_controllen = CMSG_SPACE(sizeof(struct in_addr));
	  memcpy(CMSG_DATA(cmptr), &(source->addr4), sizeof(source->addr4));
	  cmptr->cmsg_len = CMSG_LEN(sizeof(struct in_addr));
	  cmptr->cmsg_level = IPPROTO_IP;
	  cmptr->cmsg_type = IP_SENDSRCADDR;
#endif
	}
      else
	{
	  struct in6_pktinfo p;
	  p.ipi6_ifindex = iface; /* Need iface for IPv6 to handle link-local addrs */
	  p.ipi6_addr = source->addr6;
	  msg.msg_controllen = CMSG_SPACE(sizeof(struct in6_pktinfo));
	  memcpy(CMSG_DATA(cmptr), &p, sizeof(p));
	  cmptr->cmsg_len = CMSG_LEN(sizeof(struct in6_pktinfo));
	  cmptr->cmsg_type = daemon->v6pktinfo;
	  cmptr->cmsg_level = IPPROTO_IPV6;
	}
    }
  
  while (retry_send(sendmsg(fd, &msg, 0)));

  if (errno != 0)
    {
#ifdef HAVE_LINUX_NETWORK
      /* If interface is still in DAD, EINVAL results - ignore that. */
      if (errno != EINVAL)
	my_syslog(LOG_ERR, _("failed to send packet: %s"), strerror(errno));
#endif
      return 0;
    }
  
  return 1;
}
          
#ifdef HAVE_CONNTRACK
static void set_outgoing_mark(struct frec *forward, int fd)
{
  /* Copy connection mark of incoming query to outgoing connection. */
  unsigned int mark;
  if (get_incoming_mark(&forward->frec_src.source, &forward->frec_src.dest, 0, &mark))
    setsockopt(fd, SOL_SOCKET, SO_MARK, &mark, sizeof(unsigned int));
}
#endif

static void log_query_mysockaddr(unsigned int flags, char *name, union mysockaddr *addr, char *arg, unsigned short type)
{
  if (addr->sa.sa_family == AF_INET)
    {
      if (flags & F_SERVER)
	type = ntohs(addr->in.sin_port);
      log_query(flags | F_IPV4, name, (union all_addr *)&addr->in.sin_addr, arg, type);
    }
  else
    {
      if (flags & F_SERVER)
	type = ntohs(addr->in6.sin6_port);
      log_query(flags | F_IPV6, name, (union all_addr *)&addr->in6.sin6_addr, arg, type);
    }
}

static void server_send(struct server *server, int fd,
			const void *header, size_t plen, int flags)
{
  while (retry_send(sendto(fd, header, plen, flags,
			   &server->addr.sa,
			   sa_len(&server->addr))));
}

static int domain_no_rebind(char *domain)
{
  struct rebind_domain *rbd;
  size_t tlen, dlen = strlen(domain);
  char *dots = strchr(domain, '.');

  /* Match whole labels only. Empty domain matches no dots (any single label) */
  for (rbd = daemon->no_rebind; rbd; rbd = rbd->next)
    {
      if (dlen >= (tlen = strlen(rbd->domain)) &&
	hostname_isequal(rbd->domain, &domain[dlen - tlen]) &&
	(dlen == tlen || domain[dlen - tlen - 1] == '.'))
      return 1;

      if (tlen == 0 && !dots)
	return 1;
    }
  
  return 0;
}

static int forward_query(int udpfd, union mysockaddr *udpaddr,
			 union all_addr *dst_addr, unsigned int dst_iface,
			 struct dns_header *header, size_t plen,  char *limit, time_t now, 
			 struct frec *forward, int ad_reqd, int do_bit, int fast_retry)
{
  unsigned int flags = 0;
  unsigned int fwd_flags = 0;
  int is_dnssec = forward && (forward->flags & (FREC_DNSKEY_QUERY | FREC_DS_QUERY));
  struct server *master;
  void *hash = hash_questions(header, plen, daemon->namebuff);
  unsigned int gotname = extract_request(header, plen, daemon->namebuff, NULL);
  unsigned char *oph = find_pseudoheader(header, plen, NULL, NULL, NULL, NULL);
  int old_src = 0, old_reply = 0;
  int first, last, start = 0;
  int cacheable, forwarded = 0;
  size_t edns0_len;
  unsigned char *pheader;
  int ede = EDE_UNSET;
  (void)do_bit;
  
  if (header->hb4 & HB4_CD)
    fwd_flags |= FREC_CHECKING_DISABLED;
  if (ad_reqd)
    fwd_flags |= FREC_AD_QUESTION;
  if (oph)
    fwd_flags |= FREC_HAS_PHEADER;
#ifdef HAVE_DNSSEC
  if (do_bit)
    fwd_flags |= FREC_DO_QUESTION;
#endif
  
  /* Check for retry on existing query.
     FREC_DNSKEY and FREC_DS_QUERY are never set in flags, so the test below 
     ensures that no frec created for internal DNSSEC query can be returned here.
     
     Similarly FREC_NO_CACHE is never set in flags, so a query which is
     contigent on a particular source address EDNS0 option will never be matched. */
  if (forward)
    {
      old_src = 1;
      old_reply = 1;
    }
  else if ((forward = lookup_frec_by_query(hash, fwd_flags,
					   FREC_CHECKING_DISABLED | FREC_AD_QUESTION | FREC_DO_QUESTION |
					   FREC_HAS_PHEADER | FREC_DNSKEY_QUERY | FREC_DS_QUERY | FREC_NO_CACHE)))
    {
      struct frec_src *src;
      
      for (src = &forward->frec_src; src; src = src->next)
	if (src->orig_id == ntohs(header->id) && 
	    sockaddr_isequal(&src->source, udpaddr))
	  break;
      
      if (src)
	{
	  old_src = 1;
	  /* If a query is retried, use the log_id for the retry when logging the answer. */
	  src->log_id = daemon->log_id;
	}
      else
	{
	  /* Existing query, but from new source, just add this 
	     client to the list that will get the reply.*/
	  
	  /* Note whine_malloc() zeros memory. */
	  if (!daemon->free_frec_src &&
	      daemon->frec_src_count < daemon->ftabsize &&
	      (daemon->free_frec_src = whine_malloc(sizeof(struct frec_src))))
	    {
	      daemon->frec_src_count++;
	      daemon->free_frec_src->next = NULL;
	    }
	  
	  /* If we've been spammed with many duplicates, return REFUSED. */
	  if (!daemon->free_frec_src)
	    {
	      query_full(now, NULL);
	      /* This is tricky; if we're blasted with the same query
		 over and over, we'll end up taking this path each time
		 and never resetting until the frec gets deleted by
		 aging followed by the receipt of a different query. This
		 is a bit of a DoS vuln. Avoid by explicitly deleting the
		 frec once it expires. */
	      if (difftime(now, forward->time) >= TIMEOUT)
		free_frec(forward);
	      goto reply;
	    }
	  
	  src = daemon->free_frec_src;
	  daemon->free_frec_src = src->next;
	  src->next = forward->frec_src.next;
	  forward->frec_src.next = src;
	  src->orig_id = ntohs(header->id);
	  src->source = *udpaddr;
	  src->dest = *dst_addr;
	  src->log_id = daemon->log_id;
	  src->iface = dst_iface;
	  src->fd = udpfd;

	  /* closely spaced identical queries cannot be a try and a retry, so
	     it's safe to wait for the reply from the first without
	     forwarding the second. */
	  if (difftime(now, forward->time) < 2)
	    return 0;
	}
    }

  /* new query */
  if (!forward)
    {
      /* If the query is malformed, we can't forward it because
	 we can't get a reliable hash to recognise the answer. */
      if (!hash)
	{
	  flags = 0;
	  ede = EDE_INVALID_DATA;
	  goto reply;
	}
      
      if (lookup_domain(daemon->namebuff, gotname, &first, &last))
	flags = is_local_answer(now, first, daemon->namebuff);
      else
	{
	  /* no available server. */
	  ede = EDE_NOT_READY;
	  flags = 0;
	}
       
      /* don't forward A or AAAA queries for simple names, except the empty name */
      if (!flags &&
	  option_bool(OPT_NODOTS_LOCAL) &&
	  (gotname & (F_IPV4 | F_IPV6)) &&
	  !strchr(daemon->namebuff, '.') &&
	  strlen(daemon->namebuff) != 0)
	flags = check_for_local_domain(daemon->namebuff, now) ? F_NOERR : F_NXDOMAIN;
      
      /* Configured answer. */
      if (flags || ede == EDE_NOT_READY)
	goto reply;
      
      master = daemon->serverarray[first];
      
      if (!(forward = get_new_frec(now, master, 0)))
	goto reply;
      /* table full - flags == 0, return REFUSED */
      
      /* Keep copy of query if we're doing fast retry. */
      if (daemon->fast_retry_time != 0)
	{
	  forward->stash = blockdata_alloc((char *)header, plen);
	  forward->stash_len = plen;
	}
      
      forward->frec_src.log_id = daemon->log_id;
      forward->frec_src.source = *udpaddr;
      forward->frec_src.orig_id = ntohs(header->id);
      forward->frec_src.dest = *dst_addr;
      forward->frec_src.iface = dst_iface;
      forward->frec_src.next = NULL;
      forward->frec_src.fd = udpfd;
      forward->new_id = get_id();
      memcpy(forward->hash, hash, HASH_SIZE);
      forward->forwardall = 0;
      forward->flags = fwd_flags;
      if (domain_no_rebind(daemon->namebuff))
	forward->flags |= FREC_NOREBIND;
      if (header->hb4 & HB4_CD)
	forward->flags |= FREC_CHECKING_DISABLED;
      if (ad_reqd)
	forward->flags |= FREC_AD_QUESTION;
#ifdef HAVE_DNSSEC
      forward->work_counter = DNSSEC_WORK;
      if (do_bit)
	forward->flags |= FREC_DO_QUESTION;
#endif
      
      start = first;

      if (option_bool(OPT_ALL_SERVERS))
	forward->forwardall = 1;

      if (!option_bool(OPT_ORDER))
	{
	  if (master->forwardcount++ > FORWARD_TEST ||
	      difftime(now, master->forwardtime) > FORWARD_TIME ||
	      master->last_server == -1)
	    {
	      master->forwardtime = now;
	      master->forwardcount = 0;
	      forward->forwardall = 1;
	    }
	  else
	    start = master->last_server;
	}
    }
  else
    {
#ifdef HAVE_DNSSEC
      /* If we've already got an answer to this query, but we're awaiting keys for validation,
	 there's no point retrying the query, retry the key query instead...... */
      while (forward->blocking_query)
	forward = forward->blocking_query;

      if (forward->flags & (FREC_DNSKEY_QUERY | FREC_DS_QUERY))
	{
	  int is_sign;
	  unsigned char *pheader;
	  
	  /* log_id should match previous DNSSEC query. */
	  daemon->log_display_id = forward->frec_src.log_id;
	  
	  blockdata_retrieve(forward->stash, forward->stash_len, (void *)header);
	  plen = forward->stash_len;
	  /* get query for logging. */
	  extract_request(header, plen, daemon->namebuff, NULL);
	  
	  if (find_pseudoheader(header, plen, NULL, &pheader, &is_sign, NULL) && !is_sign)
	    PUTSHORT(SAFE_PKTSZ, pheader);
	  
	  /* Find suitable servers: should never fail. */
	  if (!filter_servers(forward->sentto->arrayposn, F_DNSSECOK, &first, &last))
	    return 0;
	  
	  is_dnssec = 1;
	  forward->forwardall = 1;
	}
      else
#endif
	{
	  /* retry on existing query, from original source. Send to all available servers  */
	  if (udpfd == -1 && !fast_retry)
	    forward->sentto->failed_queries++;
	  else
	    forward->sentto->retrys++;
	  
	  if (!filter_servers(forward->sentto->arrayposn, F_SERVER, &first, &last))
	    goto reply;
	  
	  master = daemon->serverarray[first];
	  
	  /* Forward to all available servers on retry of query from same host. */
	  if (!option_bool(OPT_ORDER) && old_src && !fast_retry)
	    forward->forwardall = 1;
	  else
	    {
	      start = forward->sentto->arrayposn;
	      
	      if (option_bool(OPT_ORDER) && !fast_retry)
		{
		  /* In strict order mode, there must be a server later in the list
		     left to send to, otherwise without the forwardall mechanism,
		     code further on will cycle around the list forwever if they
		     all return REFUSED. If at the last, give up.
		     Note that we can get here EITHER because a client retried,
		     or an upstream server returned REFUSED. The above only
		     applied in the later case. For client retries,
		     keep trying the last server.. */
		  if (++start == last)
		    {
		      if (old_reply)
			goto reply;
		      else
			start--;
		    }
		}
	    }	  
	}
      
      /* If we didn't get an answer advertising a maximal packet in EDNS,
	 fall back to 1280, which should work everywhere on IPv6.
	 If that generates an answer, it will become the new default
	 for this server */
      forward->flags |= FREC_TEST_PKTSZ;
    }

  /* We may be resending a DNSSEC query here, for which the below processing is not necessary. */
  if (!is_dnssec)
    {
      header->id = htons(forward->new_id);
      
      plen = add_edns0_config(header, plen, ((unsigned char *)header) + PACKETSZ, &forward->frec_src.source, now, &cacheable);
      
      if (!cacheable)
	forward->flags |= FREC_NO_CACHE;
      
#ifdef HAVE_DNSSEC
      if (option_bool(OPT_DNSSEC_VALID) && (master->flags & SERV_DO_DNSSEC))
	{
	  plen = add_do_bit(header, plen, ((unsigned char *) header) + PACKETSZ);
	  
	  /* For debugging, set Checking Disabled, otherwise, have the upstream check too,
	     this allows it to select auth servers when one is returning bad data. */
	  if (option_bool(OPT_DNSSEC_DEBUG))
	    header->hb4 |= HB4_CD;
	  
	}
#endif
      
      if (find_pseudoheader(header, plen, &edns0_len, &pheader, NULL, NULL))
	{
	  /* If there wasn't a PH before, and there is now, we added it. */
	  if (!oph)
	    forward->flags |= FREC_ADDED_PHEADER;
	  
	  /* If we're sending an EDNS0 with any options, we can't recreate the query from a reply. */
	  if (edns0_len > 11)
	    forward->flags |= FREC_HAS_EXTRADATA;
	  
	  /* Reduce udp size on retransmits. */
	  if (forward->flags & FREC_TEST_PKTSZ)
	    PUTSHORT(SAFE_PKTSZ, pheader);
	}
    }
  
  if (forward->forwardall)
    start = first;

  forwarded = 0;
  
  /* check for send errors here (no route to host) 
     if we fail to send to all nameservers, send back an error
     packet straight away (helps modem users when offline)  */

  while (1)
    { 
      int fd;
      struct server *srv = daemon->serverarray[start];
      
      if ((fd = allocate_rfd(&forward->rfds, srv)) != -1)
	{
	  
#ifdef HAVE_CONNTRACK
	  /* Copy connection mark of incoming query to outgoing connection. */
	  if (option_bool(OPT_CONNTRACK))
	    set_outgoing_mark(forward, fd);
#endif
	  
#ifdef HAVE_DNSSEC
	  if (option_bool(OPT_DNSSEC_VALID) && (forward->flags & FREC_ADDED_PHEADER))
	    {
	      /* Difficult one here. If our client didn't send EDNS0, we will have set the UDP
		 packet size to 512. But that won't provide space for the RRSIGS in many cases.
		 The RRSIGS will be stripped out before the answer goes back, so the packet should
		 shrink again. So, if we added a do-bit, bump the udp packet size to the value
		 known to be OK for this server. We check returned size after stripping and set
		 the truncated bit if it's still too big. */		  
	      unsigned char *pheader;
	      int is_sign;
	      if (find_pseudoheader(header, plen, NULL, &pheader, &is_sign, NULL) && !is_sign)
		PUTSHORT(srv->edns_pktsz, pheader);
	    }
#endif
	  
	  if (retry_send(sendto(fd, (char *)header, plen, 0,
				&srv->addr.sa,
				sa_len(&srv->addr))))
	    continue;
	  
	  if (errno == 0)
	    {
#ifdef HAVE_DUMPFILE
	      dump_packet_udp(DUMP_UP_QUERY, (void *)header, plen, NULL, &srv->addr, fd);
#endif
	      
	      /* Keep info in case we want to re-send this packet */
	      daemon->srv_save = srv;
	      daemon->packet_len = plen;
	      daemon->fd_save = fd;
	      
	      if (!(forward->flags & (FREC_DNSKEY_QUERY | FREC_DS_QUERY)))
		{
		  if (!gotname)
		    strcpy(daemon->namebuff, "query");
		  log_query_mysockaddr(F_SERVER | F_FORWARD, daemon->namebuff,
				       &srv->addr, NULL, 0);
		}
#ifdef HAVE_DNSSEC
	      else
		log_query_mysockaddr(F_NOEXTRA | F_DNSSEC | F_SERVER, daemon->namebuff, &srv->addr,
				     (forward->flags & FREC_DNSKEY_QUERY) ? "dnssec-retry[DNSKEY]" : "dnssec-retry[DS]", 0);
#endif

	      srv->queries++;
	      forwarded = 1;
	      forward->sentto = srv;
	      if (!forward->forwardall) 
		break;
	      forward->forwardall++;
	    }
	}
      
      if (++start == last)
	break;
    }
  
  if (forwarded || is_dnssec)
    {
      forward->forward_timestamp = dnsmasq_milliseconds();
      return 1;
    }
  
  /* could not send on, prepare to return */ 
  header->id = htons(forward->frec_src.orig_id);
  free_frec(forward); /* cancel */
  ede = EDE_NETERR;
  
 reply:
  if (udpfd != -1)
    {
      if (!(plen = make_local_answer(flags, gotname, plen, header, daemon->namebuff, limit, first, last, ede)))
	return 0;
      
      if (oph)
	{
	  u16 swap = htons((u16)ede);

	  if (ede != EDE_UNSET)
	    plen = add_pseudoheader(header, plen, (unsigned char *)limit, daemon->edns_pktsz, EDNS0_OPTION_EDE, (unsigned char *)&swap, 2, do_bit, 0);
	  else
	    plen = add_pseudoheader(header, plen, (unsigned char *)limit, daemon->edns_pktsz, 0, NULL, 0, do_bit, 0);
	}
      
#if defined(HAVE_CONNTRACK) && defined(HAVE_UBUS)
      if (option_bool(OPT_CMARK_ALST_EN))
	{
	  unsigned int mark;
	  int have_mark = get_incoming_mark(udpaddr, dst_addr, /* istcp: */ 0, &mark);
	  if (have_mark && ((u32)mark & daemon->allowlist_mask))
	    report_addresses(header, plen, mark);
	}
#endif
      
      send_from(udpfd, option_bool(OPT_NOWILD) || option_bool(OPT_CLEVERBIND), (char *)header, plen, udpaddr, dst_addr, dst_iface);
    }
	  
  return 0;
}

/* Check if any frecs need to do a retry, and action that if so. 
   Return time in milliseconds until he next retry will be required,
   or -1 if none. */
int fast_retry(time_t now)
{
  struct frec *f;
  int ret = -1;
  
  if (daemon->fast_retry_time != 0)
    {
      u32 millis = dnsmasq_milliseconds();
      
      for (f = daemon->frec_list; f; f = f->next)
	if (f->sentto && f->stash && difftime(now, f->time) < daemon->fast_retry_timeout)
	  {
#ifdef HAVE_DNSSEC
	    if (f->blocking_query)
	      continue;
#endif
	    /* t is milliseconds since last query sent. */ 
	    int to_run, t = (int)(millis - f->forward_timestamp);
	    
	    if (t < f->forward_delay)
	      to_run = f->forward_delay - t;
	    else
	      {
		unsigned char *udpsz;
		unsigned short udp_size =  PACKETSZ; /* default if no EDNS0 */
		struct dns_header *header = (struct dns_header *)daemon->packet;
		
		/* packet buffer overwritten */
		daemon->srv_save = NULL;
		
		blockdata_retrieve(f->stash, f->stash_len, (void *)header);
		
		/* UDP size already set in saved query. */
		if (find_pseudoheader(header, f->stash_len, NULL, &udpsz, NULL, NULL))
		  GETSHORT(udp_size, udpsz);
		
		daemon->log_display_id = f->frec_src.log_id;
		
		forward_query(-1, NULL, NULL, 0, header, f->stash_len, ((char *) header) + udp_size, now, f,
			      f->flags & FREC_AD_QUESTION, f->flags & FREC_DO_QUESTION, 1);

		to_run = f->forward_delay = 2 * f->forward_delay;
	      }

	    if (ret == -1 || ret > to_run)
	      ret = to_run;
	  }
      
    }
  return ret;
}

static struct ipsets *domain_find_sets(struct ipsets *setlist, const char *domain) {
  /* Similar algorithm to search_servers. */
  struct ipsets *ipset_pos, *ret = NULL;
  unsigned int namelen = strlen(domain);
  unsigned int matchlen = 0;
  for (ipset_pos = setlist; ipset_pos; ipset_pos = ipset_pos->next) 
    {
      unsigned int domainlen = strlen(ipset_pos->domain);
      const char *matchstart = domain + namelen - domainlen;
      if (namelen >= domainlen && hostname_isequal(matchstart, ipset_pos->domain) &&
          (domainlen == 0 || namelen == domainlen || *(matchstart - 1) == '.' ) &&
          domainlen >= matchlen) 
        {
          matchlen = domainlen;
          ret = ipset_pos;
        }
    }

  return ret;
}

static size_t process_reply(struct dns_header *header, time_t now, struct server *server, size_t n, int check_rebind, 
			    int no_cache, int cache_secure, int bogusanswer, int ad_reqd, int do_bit, int added_pheader, 
			    union mysockaddr *query_source, unsigned char *limit, int ede)
{
  unsigned char *pheader, *sizep;
  struct ipsets *ipsets = NULL, *nftsets = NULL;
  int munged = 0, is_sign;
  unsigned int rcode = RCODE(header);
  size_t plen; 
    
  (void)ad_reqd;
  (void)do_bit;
  (void)bogusanswer;

#ifdef HAVE_IPSET
  if (daemon->ipsets && extract_request(header, n, daemon->namebuff, NULL))
    ipsets = domain_find_sets(daemon->ipsets, daemon->namebuff);
#endif

#ifdef HAVE_NFTSET
  if (daemon->nftsets && extract_request(header, n, daemon->namebuff, NULL))
    nftsets = domain_find_sets(daemon->nftsets, daemon->namebuff);
#endif

  if ((pheader = find_pseudoheader(header, n, &plen, &sizep, &is_sign, NULL)))
    {
      /* Get extended RCODE. */
      rcode |= sizep[2] << 4;

      if (option_bool(OPT_CLIENT_SUBNET) && !check_source(header, plen, pheader, query_source))
	{
	  my_syslog(LOG_WARNING, _("discarding DNS reply: subnet option mismatch"));
	  return 0;
	}
      
      if (!is_sign)
	{
	  if (added_pheader)
	    {
	      /* client didn't send EDNS0, we added one, strip it off before returning answer. */
	      n = rrfilter(header, n, RRFILTER_EDNS0);
	      pheader = NULL;
	    }
	  else
	    {
	      /* If upstream is advertising a larger UDP packet size
		 than we allow, trim it so that we don't get overlarge
		 requests for the client. We can't do this for signed packets. */
	      unsigned short udpsz;
	      GETSHORT(udpsz, sizep);
	      if (udpsz > daemon->edns_pktsz)
		{
		  sizep -= 2;
		  PUTSHORT(daemon->edns_pktsz, sizep);
		}

#ifdef HAVE_DNSSEC
	      /* If the client didn't set the do bit, but we did, reset it. */
	      if (option_bool(OPT_DNSSEC_VALID) && !do_bit)
		{
		  unsigned short flags;
		  sizep += 2; /* skip RCODE */
		  GETSHORT(flags, sizep);
		  flags &= ~0x8000;
		  sizep -= 2;
		  PUTSHORT(flags, sizep);
		}
#endif
	    }
	}
    }
  
  /* RFC 4035 sect 4.6 para 3 */
  if (!is_sign && !option_bool(OPT_DNSSEC_PROXY))
     header->hb4 &= ~HB4_AD;

  header->hb4 |= HB4_RA; /* recursion if available */

  if (OPCODE(header) != QUERY)
    return resize_packet(header, n, pheader, plen);

  if (rcode != NOERROR && rcode != NXDOMAIN)
    {
      union all_addr a;
      a.log.rcode = rcode;
      a.log.ede = ede;
      log_query(F_UPSTREAM | F_RCODE, "error", &a, NULL, 0);
      
      return resize_packet(header, n, pheader, plen);
    }
  
  /* Complain loudly if the upstream server is non-recursive. */
  if (!(header->hb4 & HB4_RA) && rcode == NOERROR &&
      server && !(server->flags & SERV_WARNED_RECURSIVE))
    {
      (void)prettyprint_addr(&server->addr, daemon->namebuff);
      my_syslog(LOG_WARNING, _("nameserver %s refused to do a recursive query"), daemon->namebuff);
      if (!option_bool(OPT_LOG))
	server->flags |= SERV_WARNED_RECURSIVE;
    }  

  if (daemon->bogus_addr && rcode != NXDOMAIN &&
      check_for_bogus_wildcard(header, n, daemon->namebuff, now))
    {
      munged = 1;
      SET_RCODE(header, NXDOMAIN);
      header->hb3 &= ~HB3_AA;
      cache_secure = 0;
      ede = EDE_BLOCKED;
    }
  else 
    {
      int doctored = 0;
      
      if (rcode == NXDOMAIN && 
	  extract_request(header, n, daemon->namebuff, NULL))
	{
	  if (check_for_local_domain(daemon->namebuff, now) ||
	      lookup_domain(daemon->namebuff, F_CONFIG, NULL, NULL))
	    {
	      /* if we forwarded a query for a locally known name (because it was for 
		 an unknown type) and the answer is NXDOMAIN, convert that to NODATA,
		 since we know that the domain exists, even if upstream doesn't */
	      munged = 1;
	      header->hb3 |= HB3_AA;
	      SET_RCODE(header, NOERROR);
	      cache_secure = 0;
	    }
	}

      /* Before extract_addresses() */
      if (rcode == NOERROR)
	{
	  if (option_bool(OPT_FILTER_A))
	    n = rrfilter(header, n, RRFILTER_A);

	  if (option_bool(OPT_FILTER_AAAA))
	    n = rrfilter(header, n, RRFILTER_AAAA);
	}

      switch (extract_addresses(header, n, daemon->namebuff, now, ipsets, nftsets, is_sign, check_rebind, no_cache, cache_secure, &doctored))
	{
	case 1:
	  my_syslog(LOG_WARNING, _("possible DNS-rebind attack detected: %s"), daemon->namebuff);
	  munged = 1;
	  cache_secure = 0;
	  ede = EDE_BLOCKED;
	  break;
	  
	  /* extract_addresses() found a malformed answer. */
	case 2:
	  munged = 1;
	  SET_RCODE(header, SERVFAIL);
	  cache_secure = 0;
	  ede = EDE_OTHER;
	  break;
	}

      if (doctored)
	cache_secure = 0;
    }
  
#ifdef HAVE_DNSSEC
  if (bogusanswer && !(header->hb4 & HB4_CD) && !option_bool(OPT_DNSSEC_DEBUG))
    {
      /* Bogus reply, turn into SERVFAIL */
      SET_RCODE(header, SERVFAIL);
      munged = 1;
    }

  if (option_bool(OPT_DNSSEC_VALID))
    {
      header->hb4 &= ~HB4_AD;
      
      if (!(header->hb4 & HB4_CD) && ad_reqd && cache_secure)
	header->hb4 |= HB4_AD;
      
      /* If the requestor didn't set the DO bit, don't return DNSSEC info. */
      if (!do_bit)
	n = rrfilter(header, n, RRFILTER_DNSSEC);
    }
#endif

  /* do this after extract_addresses. Ensure NODATA reply and remove
     nameserver info. */
  if (munged)
    {
      header->ancount = htons(0);
      header->nscount = htons(0);
      header->arcount = htons(0);
      header->hb3 &= ~HB3_TC;
    }
  
  /* the bogus-nxdomain stuff, doctor and NXDOMAIN->NODATA munging can all elide
     sections of the packet. Find the new length here and put back pseudoheader
     if it was removed. */
  n = resize_packet(header, n, pheader, plen);

  if (pheader && ede != EDE_UNSET)
    {
      u16 swap = htons((u16)ede);
      n = add_pseudoheader(header, n, limit, daemon->edns_pktsz, EDNS0_OPTION_EDE, (unsigned char *)&swap, 2, do_bit, 1);
    }

  if (RCODE(header) == NXDOMAIN)
    server->nxdomain_replies++;

  return n;
}

#ifdef HAVE_DNSSEC
static void dnssec_validate(struct frec *forward, struct dns_header *header,
			    ssize_t plen, int status, time_t now)
{
  daemon->log_display_id = forward->frec_src.log_id;
  
  /* We've had a reply already, which we're validating. Ignore this duplicate */
  if (forward->blocking_query)
    return;
  
  /* Truncated answer can't be validated.
     If this is an answer to a DNSSEC-generated query, we still
     need to get the client to retry over TCP, so return
     an answer with the TC bit set, even if the actual answer fits.
  */
  if (header->hb3 & HB3_TC)
    status = STAT_TRUNCATED;

  /* If all replies to a query are REFUSED, give up. */
  if (RCODE(header) == REFUSED)
    status = STAT_ABANDONED;
  
  /* As soon as anything returns BOGUS, we stop and unwind, to do otherwise
     would invite infinite loops, since the answers to DNSKEY and DS queries
     will not be cached, so they'll be repeated. */
  if (!STAT_ISEQUAL(status, STAT_BOGUS) && !STAT_ISEQUAL(status, STAT_TRUNCATED) && !STAT_ISEQUAL(status, STAT_ABANDONED))
    {
      if (forward->flags & FREC_DNSKEY_QUERY)
	status = dnssec_validate_by_ds(now, header, plen, daemon->namebuff, daemon->keyname, forward->class);
      else if (forward->flags & FREC_DS_QUERY)
	status = dnssec_validate_ds(now, header, plen, daemon->namebuff, daemon->keyname, forward->class);
      else
	status = dnssec_validate_reply(now, header, plen, daemon->namebuff, daemon->keyname, &forward->class, 
				       !option_bool(OPT_DNSSEC_IGN_NS) && (forward->sentto->flags & SERV_DO_DNSSEC),
				       NULL, NULL, NULL);
#ifdef HAVE_DUMPFILE
      if (STAT_ISEQUAL(status, STAT_BOGUS))
	dump_packet_udp((forward->flags & (FREC_DNSKEY_QUERY | FREC_DS_QUERY)) ? DUMP_SEC_BOGUS : DUMP_BOGUS,
			header, (size_t)plen, &forward->sentto->addr, NULL, -daemon->port);
#endif
    }
  
  /* Can't validate, as we're missing key data. Put this
     answer aside, whilst we get that. */     
  if (STAT_ISEQUAL(status, STAT_NEED_DS) || STAT_ISEQUAL(status, STAT_NEED_KEY))
    {
      struct frec *new = NULL;
      struct blockdata *stash;
      
      /* Now save reply pending receipt of key data */
      if ((stash = blockdata_alloc((char *)header, plen)))
	{
	  /* validate routines leave name of required record in daemon->keyname */
	  unsigned int flags = STAT_ISEQUAL(status, STAT_NEED_KEY) ? FREC_DNSKEY_QUERY : FREC_DS_QUERY;

	  if ((new = lookup_frec_dnssec(daemon->keyname, forward->class, flags, header)))
	    {
	      /* This is tricky; it detects loops in the dependency
		 graph for DNSSEC validation, say validating A requires DS B
		 and validating DS B requires DNSKEY C and validating DNSKEY C requires DS B.
		 This should never happen in correctly signed records, but it's
		 likely the case that sufficiently broken ones can cause our validation
		 code requests to exhibit cycles. The result is that the ->blocking_query list
		 can form a cycle, and under certain circumstances that can lock us in 
		 an infinite loop. Here we transform the situation into ABANDONED. */
	      struct frec *f;
	      for (f = new; f; f = f->blocking_query)
		if (f == forward)
		  break;

	      if (!f)
		{
		  forward->next_dependent = new->dependent;
		  new->dependent = forward;
		  /* Make consistent, only replace query copy with unvalidated answer
		     when we set ->blocking_query. */
		  if (forward->stash)
		    blockdata_free(forward->stash);
		  forward->blocking_query = new;
		  forward->stash_len = plen;
		  forward->stash = stash;
		  return;
		}
	    }
	  else
	    {
	      struct server *server;
	      struct frec *orig;
	      void *hash;
	      size_t nn;
	      int serverind, fd;
	      struct randfd_list *rfds = NULL;
	      
	      /* Find the original query that started it all.... */
	      for (orig = forward; orig->dependent; orig = orig->dependent);
	      
	      /* Make sure we don't expire and free the orig frec during the
		 allocation of a new one: third arg of get_new_frec() does that. */
	      if ((serverind = dnssec_server(forward->sentto, daemon->keyname, NULL, NULL)) != -1 &&
		  (server = daemon->serverarray[serverind]) &&
		  (nn = dnssec_generate_query(header, ((unsigned char *) header) + server->edns_pktsz,
					      daemon->keyname, forward->class,
					      STAT_ISEQUAL(status, STAT_NEED_KEY) ? T_DNSKEY : T_DS, server->edns_pktsz)) && 
		  (hash = hash_questions(header, nn, daemon->namebuff)) &&
		  --orig->work_counter != 0 &&
		  (fd = allocate_rfd(&rfds, server)) != -1 &&
		  (new = get_new_frec(now, server, 1)))
		{
		  struct frec *next = new->next;
		  
		  *new = *forward; /* copy everything, then overwrite */
		  new->next = next;
		  new->blocking_query = NULL;
		  
		  new->frec_src.log_id = daemon->log_display_id = ++daemon->log_id;
		  new->sentto = server;
		  new->rfds = rfds;
		  new->frec_src.next = NULL;
		  new->flags &= ~(FREC_DNSKEY_QUERY | FREC_DS_QUERY | FREC_HAS_EXTRADATA);
		  new->flags |= flags;
		  new->forwardall = 0;
		  
		  forward->next_dependent = NULL;
		  new->dependent = forward; /* to find query awaiting new one. */
		  
		  /* Make consistent, only replace query copy with unvalidated answer
		     when we set ->blocking_query. */
		  forward->blocking_query = new; 
		  if (forward->stash)
		    blockdata_free(forward->stash);
		  forward->stash_len = plen;
		  forward->stash = stash;
		  
		  memcpy(new->hash, hash, HASH_SIZE);
		  new->new_id = get_id();
		  header->id = htons(new->new_id);
		  /* Save query for retransmission and de-dup */
		  new->stash = blockdata_alloc((char *)header, nn);
		  new->stash_len = nn;
		  if (daemon->fast_retry_time != 0)
		    new->forward_timestamp = dnsmasq_milliseconds();
		  
		  /* Don't resend this. */
		  daemon->srv_save = NULL;
		  
#ifdef HAVE_CONNTRACK
		  if (option_bool(OPT_CONNTRACK))
		    set_outgoing_mark(orig, fd);
#endif
		  
		  server_send(server, fd, header, nn, 0);
		  server->queries++;
#ifdef HAVE_DUMPFILE
		  dump_packet_udp(DUMP_SEC_QUERY, (void *)header, (size_t)nn, NULL, &server->addr, fd);
#endif
		  log_query_mysockaddr(F_NOEXTRA | F_DNSSEC | F_SERVER, daemon->keyname, &server->addr,
				       STAT_ISEQUAL(status, STAT_NEED_KEY) ? "dnssec-query[DNSKEY]" : "dnssec-query[DS]", 0);
		  return;
		}
	      
	      free_rfds(&rfds); /* error unwind */
	    }
	  
	  blockdata_free(stash); /* don't leak this on failure. */
	}

      /* sending DNSSEC query failed or loop detected. */
      status = STAT_ABANDONED;
    }

  /* Validated original answer, all done. */
  if (!forward->dependent)
    return_reply(now, forward, header, plen, status);
  else
    {
      /* validated subsidiary query/queries, (and cached result)
	 pop that and return to the previous query/queries we were working on. */
      struct frec *prev, *nxt = forward->dependent;
      
      free_frec(forward);
      
      while ((prev = nxt))
	{
	  /* ->next_dependent will have changed after return from recursive call below. */
	  nxt = prev->next_dependent;
	  prev->blocking_query = NULL; /* already gone */
	  blockdata_retrieve(prev->stash, prev->stash_len, (void *)header);
	  dnssec_validate(prev, header, prev->stash_len, status, now);
	}
    }
}
#endif

/* sets new last_server */
void reply_query(int fd, time_t now)
{
  /* packet from peer server, extract data for cache, and send to
     original requester */
  struct dns_header *header;
  union mysockaddr serveraddr;
  struct frec *forward;
  socklen_t addrlen = sizeof(serveraddr);
  ssize_t n = recvfrom(fd, daemon->packet, daemon->packet_buff_sz, 0, &serveraddr.sa, &addrlen);
  struct server *server;
  void *hash;
  int first, last, c;
    
  /* packet buffer overwritten */
  daemon->srv_save = NULL;

  /* Determine the address of the server replying  so that we can mark that as good */
  if (serveraddr.sa.sa_family == AF_INET6)
    serveraddr.in6.sin6_flowinfo = 0;
  
  header = (struct dns_header *)daemon->packet;

  if (n < (int)sizeof(struct dns_header) || !(header->hb3 & HB3_QR))
    return;

  hash = hash_questions(header, n, daemon->namebuff);
  
  if (!(forward = lookup_frec(ntohs(header->id), fd, hash, &first, &last)))
    return;
  
  /* spoof check: answer must come from known server, also
     we may have sent the same query to multiple servers from
     the same local socket, and would like to know which one has answered. */
  for (c = first; c != last; c++)
    if (sockaddr_isequal(&daemon->serverarray[c]->addr, &serveraddr))
      break;
  
  if (c == last)
    return;

  server = daemon->serverarray[c];

  if (RCODE(header) != REFUSED)
    daemon->serverarray[first]->last_server = c;
  else if (daemon->serverarray[first]->last_server == c)
    daemon->serverarray[first]->last_server = -1;

  /* If sufficient time has elapsed, try and expand UDP buffer size again. */
  if (difftime(now, server->pktsz_reduced) > UDP_TEST_TIME)
    server->edns_pktsz = daemon->edns_pktsz;

  /* log_query gets called indirectly all over the place, so 
     pass these in global variables - sorry. */
  daemon->log_display_id = forward->frec_src.log_id;
  daemon->log_source_addr = &forward->frec_src.source;
  
#ifdef HAVE_DUMPFILE
  dump_packet_udp((forward->flags & (FREC_DNSKEY_QUERY | FREC_DS_QUERY)) ? DUMP_SEC_REPLY : DUMP_UP_REPLY,
		  (void *)header, n, &serveraddr, NULL, fd);
#endif

  if (daemon->ignore_addr && RCODE(header) == NOERROR &&
      check_for_ignored_address(header, n))
    return;
  
  /* Note: if we send extra options in the EDNS0 header, we can't recreate
     the query from the reply. */
  if ((RCODE(header) == REFUSED || RCODE(header) == SERVFAIL) &&
      forward->forwardall == 0 &&
      !(forward->flags & FREC_HAS_EXTRADATA))
    /* for broken servers, attempt to send to another one. */
    {
      unsigned char *pheader, *udpsz;
      unsigned short udp_size =  PACKETSZ; /* default if no EDNS0 */
      size_t plen;
      int is_sign;
      size_t nn = 0;
      
#ifdef HAVE_DNSSEC
      /* The query MAY have got a good answer, and be awaiting
	 the results of further queries, in which case
	 The Stash contains something else and we don't need to retry anyway. */
      if (forward->blocking_query)
	return;
      
      if (forward->flags & (FREC_DNSKEY_QUERY | FREC_DS_QUERY))
	{
	  /* DNSSEC queries have a copy of the original query stashed. */
	  blockdata_retrieve(forward->stash, forward->stash_len, (void *)header);
	  nn = forward->stash_len;
	  udp_size = daemon->edns_pktsz;
	}
      else
#endif
	{
	  /* in fast retry mode, we have a copy of the query. */
	  if (daemon->fast_retry_time != 0 && forward->stash)
	    {
	      blockdata_retrieve(forward->stash, forward->stash_len, (void *)header);
	      nn = forward->stash_len;
	      /* UDP size already set in saved query. */
	      if (find_pseudoheader(header, (size_t)n, NULL, &udpsz, NULL, NULL))
		GETSHORT(udp_size, udpsz);
	    }
	  else
	    {
	      /* recreate query from reply */
	      if ((pheader = find_pseudoheader(header, (size_t)n, &plen, &udpsz, &is_sign, NULL)))
		GETSHORT(udp_size, udpsz);
	      
	      /* If the client provides an EDNS0 UDP size, use that to limit our reply.
		 (bounded by the maximum configured). If no EDNS0, then it
		 defaults to 512 */
	      if (udp_size > daemon->edns_pktsz)
		udp_size = daemon->edns_pktsz;
	      else if (udp_size < PACKETSZ)
		udp_size = PACKETSZ; /* Sanity check - can't reduce below default. RFC 6891 6.2.3 */
	      
	      header->ancount = htons(0);
	      header->nscount = htons(0);
	      header->arcount = htons(0);
	      header->hb3 &= ~(HB3_QR | HB3_AA | HB3_TC);
	      header->hb4 &= ~(HB4_RA | HB4_RCODE | HB4_CD | HB4_AD);
	      if (forward->flags & FREC_CHECKING_DISABLED)
		header->hb4 |= HB4_CD;
	      if (forward->flags & FREC_AD_QUESTION)
		header->hb4 |= HB4_AD;

	      if (!is_sign &&
		  (nn = resize_packet(header, (size_t)n, pheader, plen)) &&
		  (forward->flags & FREC_DO_QUESTION))
		add_do_bit(header, nn,  (unsigned char *)pheader + plen);
	    }
	}
      
      if (nn)
	{
	  forward_query(-1, NULL, NULL, 0, header, nn, ((char *) header) + udp_size, now, forward,
			forward->flags & FREC_AD_QUESTION, forward->flags & FREC_DO_QUESTION, 0);
	  return;
	}
    }

  /* If the answer is an error, keep the forward record in place in case
     we get a good reply from another server. Kill it when we've
     had replies from all to avoid filling the forwarding table when
     everything is broken */

  /* decrement count of replies recieved if we sent to more than one server. */
  if (forward->forwardall && (--forward->forwardall > 1) && RCODE(header) == REFUSED)
    return;

  /* We tried resending to this server with a smaller maximum size and got an answer.
     Make that permanent. To avoid reduxing the packet size for a single dropped packet,
     only do this when we get a truncated answer, or one larger than the safe size. */
  if (server->edns_pktsz > SAFE_PKTSZ && (forward->flags & FREC_TEST_PKTSZ) && 
      ((header->hb3 & HB3_TC) || n >= SAFE_PKTSZ))
    {
      server->edns_pktsz = SAFE_PKTSZ;
      server->pktsz_reduced = now;
      (void)prettyprint_addr(&server->addr, daemon->addrbuff);
      my_syslog(LOG_WARNING, _("reducing DNS packet size for nameserver %s to %d"), daemon->addrbuff, SAFE_PKTSZ);
    }

  forward->sentto = server;

  /* We have a good answer, and will now validate it or return it. 
     It may be some time before this the validation completes, but we don't need
     any more answers, so close the socket(s) on which we were expecting
     answers, to conserve file descriptors, and to save work reading and
     discarding answers for other upstreams. */
  free_rfds(&forward->rfds);

  /* calculate modified moving average of server latency */
  if (server->query_latency == 0)
    server->mma_latency = (dnsmasq_milliseconds() - forward->forward_timestamp) * 128; /* init */
  else
    server->mma_latency += dnsmasq_milliseconds() - forward->forward_timestamp - server->query_latency;
  /* denominator controls how many queries we average over. */
  server->query_latency = server->mma_latency/128;
  
  
#ifdef HAVE_DNSSEC
  if ((forward->sentto->flags & SERV_DO_DNSSEC) && 
      option_bool(OPT_DNSSEC_VALID) &&
      !(forward->flags & FREC_CHECKING_DISABLED))
    dnssec_validate(forward, header, n, STAT_OK, now);
  else
#endif
    return_reply(now, forward, header, n, STAT_OK); 
}

static void return_reply(time_t now, struct frec *forward, struct dns_header *header, ssize_t n, int status)
{
  int check_rebind = 0, no_cache_dnssec = 0, cache_secure = 0, bogusanswer = 0;
  size_t nn;
  int ede = EDE_UNSET;

  (void)status;

  daemon->log_display_id = forward->frec_src.log_id;
  daemon->log_source_addr = &forward->frec_src.source;
  
  /* Don't cache replies where DNSSEC validation was turned off, either
     the upstream server told us so, or the original query specified it.  */
  if ((header->hb4 & HB4_CD) || (forward->flags & FREC_CHECKING_DISABLED))
    no_cache_dnssec = 1;

#ifdef HAVE_DNSSEC
  if (!STAT_ISEQUAL(status, STAT_OK))
    {
      /* status is STAT_OK when validation not turned on. */
      no_cache_dnssec = 0;
      
      if (STAT_ISEQUAL(status, STAT_TRUNCATED))
	header->hb3 |= HB3_TC;
      else
	{
	  char *result, *domain = "result";
	  union all_addr a;

	  a.log.ede = ede = errflags_to_ede(status);

	  if (STAT_ISEQUAL(status, STAT_ABANDONED))
	    {
	      result = "ABANDONED";
	      status = STAT_BOGUS;
	    }
	  else
	    result = (STAT_ISEQUAL(status, STAT_SECURE) ? "SECURE" : (STAT_ISEQUAL(status, STAT_INSECURE) ? "INSECURE" : "BOGUS"));
	  
	  if (STAT_ISEQUAL(status, STAT_SECURE))
	    cache_secure = 1;
	  else if (STAT_ISEQUAL(status, STAT_BOGUS))
	    {
	      no_cache_dnssec = 1;
	      bogusanswer = 1;
	      
	      if (extract_request(header, n, daemon->namebuff, NULL))
		domain = daemon->namebuff;
	    }
	  
	  log_query(F_SECSTAT, domain, &a, result, 0);
	}
    }
#endif
  
  if (option_bool(OPT_NO_REBIND))
    check_rebind = !(forward->flags & FREC_NOREBIND);
  
  /* restore CD bit to the value in the query */
  if (forward->flags & FREC_CHECKING_DISABLED)
    header->hb4 |= HB4_CD;
  else
    header->hb4 &= ~HB4_CD;
  
  /* Never cache answers which are contingent on the source or MAC address EDSN0 option,
     since the cache is ignorant of such things. */
  if (forward->flags & FREC_NO_CACHE)
    no_cache_dnssec = 1;
  
  if ((nn = process_reply(header, now, forward->sentto, (size_t)n, check_rebind, no_cache_dnssec, cache_secure, bogusanswer, 
			  forward->flags & FREC_AD_QUESTION, forward->flags & FREC_DO_QUESTION, 
			  forward->flags & FREC_ADDED_PHEADER, &forward->frec_src.source,
			  ((unsigned char *)header) + daemon->edns_pktsz, ede)))
    {
      struct frec_src *src;
      
      header->id = htons(forward->frec_src.orig_id);
#ifdef HAVE_DNSSEC
      /* We added an EDNSO header for the purpose of getting DNSSEC RRs, and set the value of the UDP payload size
	 greater than the no-EDNS0-implied 512 to have space for the RRSIGS. If, having stripped them and the EDNS0
	 header, the answer is still bigger than 512, truncate it and mark it so. The client then retries with TCP. */
      if (option_bool(OPT_DNSSEC_VALID) && (forward->flags & FREC_ADDED_PHEADER) && (nn > PACKETSZ))
	{
	  header->ancount = htons(0);
	  header->nscount = htons(0);
	  header->arcount = htons(0);
	  header->hb3 |= HB3_TC;
	  nn = resize_packet(header, nn, NULL, 0);
	}
#endif
      
      for (src = &forward->frec_src; src; src = src->next)
	{
	  header->id = htons(src->orig_id);
	  
#if defined(HAVE_CONNTRACK) && defined(HAVE_UBUS)
	  if (option_bool(OPT_CMARK_ALST_EN))
	    {
	      unsigned int mark;
	      int have_mark = get_incoming_mark(&src->source, &src->dest, /* istcp: */ 0, &mark);
	      if (have_mark && ((u32)mark & daemon->allowlist_mask))
		report_addresses(header, nn, mark);
	    }
#endif
	  
	  if (src->fd != -1)
	    {
#ifdef HAVE_DUMPFILE
	      dump_packet_udp(DUMP_REPLY, daemon->packet, (size_t)nn, NULL, &src->source, src->fd);
#endif 
	      send_from(src->fd, option_bool(OPT_NOWILD) || option_bool (OPT_CLEVERBIND), daemon->packet, nn, 
			&src->source, &src->dest, src->iface);
	      
	      if (option_bool(OPT_EXTRALOG) && src != &forward->frec_src)
		{
		  daemon->log_display_id = src->log_id;
		  daemon->log_source_addr = &src->source;
		  log_query(F_UPSTREAM, "query", NULL, "duplicate", 0);
		}
	    }
	}
    }

  free_frec(forward); /* cancel */
}


#ifdef HAVE_CONNTRACK
static int is_query_allowed_for_mark(u32 mark, const char *name)
{
  int is_allowable_name, did_validate_name = 0;
  struct allowlist *allowlists;
  char **patterns_pos;
  
  for (allowlists = daemon->allowlists; allowlists; allowlists = allowlists->next)
    if (allowlists->mark == (mark & daemon->allowlist_mask & allowlists->mask))
      for (patterns_pos = allowlists->patterns; *patterns_pos; patterns_pos++)
	{
	  if (!strcmp(*patterns_pos, "*"))
	    return 1;
	  if (!did_validate_name)
	    {
	      is_allowable_name = name ? is_valid_dns_name(name) : 0;
	      did_validate_name = 1;
	    }
	  if (is_allowable_name && is_dns_name_matching_pattern(name, *patterns_pos))
	    return 1;
	}
  return 0;
}

static size_t answer_disallowed(struct dns_header *header, size_t qlen, u32 mark, const char *name)
{
  unsigned char *p;
  (void)name;
  (void)mark;
  
#ifdef HAVE_UBUS
  if (name)
    ubus_event_bcast_connmark_allowlist_refused(mark, name);
#endif
  
  setup_reply(header, /* flags: */ 0, EDE_BLOCKED);
  
  if (!(p = skip_questions(header, qlen)))
    return 0;
  return p - (unsigned char *)header;
}
#endif

void receive_query(struct listener *listen, time_t now)
{
  struct dns_header *header = (struct dns_header *)daemon->packet;
  union mysockaddr source_addr;
  unsigned char *pheader;
  unsigned short type, udp_size = PACKETSZ; /* default if no EDNS0 */
  union all_addr dst_addr;
  struct in_addr netmask, dst_addr_4;
  size_t m;
  ssize_t n;
  int if_index = 0, auth_dns = 0, do_bit = 0, have_pseudoheader = 0;
#ifdef HAVE_CONNTRACK
  unsigned int mark = 0;
  int have_mark = 0;
  int is_single_query = 0, allowed = 1;
#endif
#ifdef HAVE_AUTH
  int local_auth = 0;
#endif
  struct iovec iov[1];
  struct msghdr msg;
  struct cmsghdr *cmptr;
  union {
    struct cmsghdr align; /* this ensures alignment */
    char control6[CMSG_SPACE(sizeof(struct in6_pktinfo))];
#if defined(HAVE_LINUX_NETWORK)
    char control[CMSG_SPACE(sizeof(struct in_pktinfo))];
#elif defined(IP_RECVDSTADDR) && defined(HAVE_SOLARIS_NETWORK)
    char control[CMSG_SPACE(sizeof(struct in_addr)) +
		 CMSG_SPACE(sizeof(unsigned int))];
#elif defined(IP_RECVDSTADDR)
    char control[CMSG_SPACE(sizeof(struct in_addr)) +
		 CMSG_SPACE(sizeof(struct sockaddr_dl))];
#endif
  } control_u;
  int family = listen->addr.sa.sa_family;
   /* Can always get recvd interface for IPv6 */
  int check_dst = !option_bool(OPT_NOWILD) || family == AF_INET6;
  
  /* packet buffer overwritten */
  daemon->srv_save = NULL;

  dst_addr_4.s_addr = dst_addr.addr4.s_addr = 0;
  netmask.s_addr = 0;
  
  if (option_bool(OPT_NOWILD) && listen->iface)
    {
      auth_dns = listen->iface->dns_auth;
     
      if (family == AF_INET)
	{
	  dst_addr_4 = dst_addr.addr4 = listen->iface->addr.in.sin_addr;
	  netmask = listen->iface->netmask;
	}
    }
  
  iov[0].iov_base = daemon->packet;
  iov[0].iov_len = daemon->edns_pktsz;
    
  msg.msg_control = control_u.control;
  msg.msg_controllen = sizeof(control_u);
  msg.msg_flags = 0;
  msg.msg_name = &source_addr;
  msg.msg_namelen = sizeof(source_addr);
  msg.msg_iov = iov;
  msg.msg_iovlen = 1;
  
  if ((n = recvmsg(listen->fd, &msg, 0)) == -1)
    return;
  
  if (n < (int)sizeof(struct dns_header) || 
      (msg.msg_flags & MSG_TRUNC) ||
      (header->hb3 & HB3_QR))
    return;

  /* Clear buffer beyond request to avoid risk of
     information disclosure. */
  memset(daemon->packet + n, 0, daemon->edns_pktsz - n);
  
  source_addr.sa.sa_family = family;
  
  if (family == AF_INET)
    {
       /* Source-port == 0 is an error, we can't send back to that. 
	  http://www.ietf.org/mail-archive/web/dnsop/current/msg11441.html */
      if (source_addr.in.sin_port == 0)
	return;
    }
  else
    {
      /* Source-port == 0 is an error, we can't send back to that. */
      if (source_addr.in6.sin6_port == 0)
	return;
      source_addr.in6.sin6_flowinfo = 0;
    }
  
  /* We can be configured to only accept queries from at-most-one-hop-away addresses. */
  if (option_bool(OPT_LOCAL_SERVICE))
    {
      struct addrlist *addr;

      if (family == AF_INET6) 
	{
	  for (addr = daemon->interface_addrs; addr; addr = addr->next)
	    if ((addr->flags & ADDRLIST_IPV6) &&
		is_same_net6(&addr->addr.addr6, &source_addr.in6.sin6_addr, addr->prefixlen))
	      break;
	}
      else
	{
	  struct in_addr netmask;
	  for (addr = daemon->interface_addrs; addr; addr = addr->next)
	    {
	      netmask.s_addr = htonl(~(in_addr_t)0 << (32 - addr->prefixlen));
	      if (!(addr->flags & ADDRLIST_IPV6) &&
		  is_same_net(addr->addr.addr4, source_addr.in.sin_addr, netmask))
		break;
	    }
	}
      if (!addr)
	{
	  static int warned = 0;
	  if (!warned)
	    {
	      prettyprint_addr(&source_addr, daemon->addrbuff);
	      my_syslog(LOG_WARNING, _("ignoring query from non-local network %s (logged only once)"), daemon->addrbuff);
	      warned = 1;
	    }
	  return;
	}
    }
		
  if (check_dst)
    {
      struct ifreq ifr;

      if (msg.msg_controllen < sizeof(struct cmsghdr))
	return;

#if defined(HAVE_LINUX_NETWORK)
      if (family == AF_INET)
	for (cmptr = CMSG_FIRSTHDR(&msg); cmptr; cmptr = CMSG_NXTHDR(&msg, cmptr))
	  if (cmptr->cmsg_level == IPPROTO_IP && cmptr->cmsg_type == IP_PKTINFO)
	    {
	      union {
		unsigned char *c;
		struct in_pktinfo *p;
	      } p;
	      p.c = CMSG_DATA(cmptr);
	      dst_addr_4 = dst_addr.addr4 = p.p->ipi_spec_dst;
	      if_index = p.p->ipi_ifindex;
	    }
#elif defined(IP_RECVDSTADDR) && defined(IP_RECVIF)
      if (family == AF_INET)
	{
	  for (cmptr = CMSG_FIRSTHDR(&msg); cmptr; cmptr = CMSG_NXTHDR(&msg, cmptr))
	    {
	      union {
		unsigned char *c;
		unsigned int *i;
		struct in_addr *a;
#ifndef HAVE_SOLARIS_NETWORK
		struct sockaddr_dl *s;
#endif
	      } p;
	       p.c = CMSG_DATA(cmptr);
	       if (cmptr->cmsg_level == IPPROTO_IP && cmptr->cmsg_type == IP_RECVDSTADDR)
		 dst_addr_4 = dst_addr.addr4 = *(p.a);
	       else if (cmptr->cmsg_level == IPPROTO_IP && cmptr->cmsg_type == IP_RECVIF)
#ifdef HAVE_SOLARIS_NETWORK
		 if_index = *(p.i);
#else
  	         if_index = p.s->sdl_index;
#endif
	    }
	}
#endif
      
      if (family == AF_INET6)
	{
	  for (cmptr = CMSG_FIRSTHDR(&msg); cmptr; cmptr = CMSG_NXTHDR(&msg, cmptr))
	    if (cmptr->cmsg_level == IPPROTO_IPV6 && cmptr->cmsg_type == daemon->v6pktinfo)
	      {
		union {
		  unsigned char *c;
		  struct in6_pktinfo *p;
		} p;
		p.c = CMSG_DATA(cmptr);
		  
		dst_addr.addr6 = p.p->ipi6_addr;
		if_index = p.p->ipi6_ifindex;
	      }
	}
      
      /* enforce available interface configuration */
      
      if (!indextoname(listen->fd, if_index, ifr.ifr_name))
	return;
      
      if (!iface_check(family, &dst_addr, ifr.ifr_name, &auth_dns))
	{
	   if (!option_bool(OPT_CLEVERBIND))
	     enumerate_interfaces(0); 
	   if (!loopback_exception(listen->fd, family, &dst_addr, ifr.ifr_name) &&
	       !label_exception(if_index, family, &dst_addr))
	     return;
	}

      if (family == AF_INET && option_bool(OPT_LOCALISE))
	{
	  struct irec *iface;
	  
	  /* get the netmask of the interface which has the address we were sent to.
	     This is no necessarily the interface we arrived on. */
	  
	  for (iface = daemon->interfaces; iface; iface = iface->next)
	    if (iface->addr.sa.sa_family == AF_INET &&
		iface->addr.in.sin_addr.s_addr == dst_addr_4.s_addr)
	      break;
	  
	  /* interface may be new */
	  if (!iface && !option_bool(OPT_CLEVERBIND))
	    enumerate_interfaces(0); 
	  
	  for (iface = daemon->interfaces; iface; iface = iface->next)
	    if (iface->addr.sa.sa_family == AF_INET &&
		iface->addr.in.sin_addr.s_addr == dst_addr_4.s_addr)
	      break;
	  
	  /* If we failed, abandon localisation */
	  if (iface)
	    netmask = iface->netmask;
	  else
	    dst_addr_4.s_addr = 0;
	}
    }
   
  /* log_query gets called indirectly all over the place, so 
     pass these in global variables - sorry. */
  daemon->log_display_id = ++daemon->log_id;
  daemon->log_source_addr = &source_addr;

#ifdef HAVE_DUMPFILE
  dump_packet_udp(DUMP_QUERY, daemon->packet, (size_t)n, &source_addr, NULL, listen->fd);
#endif
  
#ifdef HAVE_CONNTRACK
  if (option_bool(OPT_CMARK_ALST_EN))
    have_mark = get_incoming_mark(&source_addr, &dst_addr, /* istcp: */ 0, &mark);
#endif
	  
  if (extract_request(header, (size_t)n, daemon->namebuff, &type))
    {
#ifdef HAVE_AUTH
      struct auth_zone *zone;
#endif
      log_query_mysockaddr(F_QUERY | F_FORWARD, daemon->namebuff,
			   &source_addr, auth_dns ? "auth" : "query", type);
      
#ifdef HAVE_CONNTRACK
      is_single_query = 1;
#endif

#ifdef HAVE_AUTH
      /* find queries for zones we're authoritative for, and answer them directly */
      if (!auth_dns && !option_bool(OPT_LOCALISE))
	for (zone = daemon->auth_zones; zone; zone = zone->next)
	  if (in_zone(zone, daemon->namebuff, NULL))
	    {
	      auth_dns = 1;
	      local_auth = 1;
	      break;
	    }
#endif
      
#ifdef HAVE_LOOP
      /* Check for forwarding loop */
      if (detect_loop(daemon->namebuff, type))
	return;
#endif
    }
  
  if (find_pseudoheader(header, (size_t)n, NULL, &pheader, NULL, NULL))
    { 
      unsigned short flags;
      
      have_pseudoheader = 1;
      GETSHORT(udp_size, pheader);
      pheader += 2; /* ext_rcode */
      GETSHORT(flags, pheader);
      
      if (flags & 0x8000)
	do_bit = 1;/* do bit */ 
	
      /* If the client provides an EDNS0 UDP size, use that to limit our reply.
	 (bounded by the maximum configured). If no EDNS0, then it
	 defaults to 512. We write this value into the query packet too, so that
	 if it's forwarded, we don't specify a maximum size greater than we can handle. */
      if (udp_size > daemon->edns_pktsz)
	udp_size = daemon->edns_pktsz;
      else if (udp_size < PACKETSZ)
	udp_size = PACKETSZ; /* Sanity check - can't reduce below default. RFC 6891 6.2.3 */

      pheader -= 6; /* ext_class */
      PUTSHORT(udp_size, pheader); /* Bounding forwarded queries to maximum configured */
    }
  
#ifdef HAVE_CONNTRACK
#ifdef HAVE_AUTH
  if (!auth_dns || local_auth)
#endif
    if (option_bool(OPT_CMARK_ALST_EN) && have_mark && ((u32)mark & daemon->allowlist_mask))
      allowed = is_query_allowed_for_mark((u32)mark, is_single_query ? daemon->namebuff : NULL);
#endif
  
  if (0);
#ifdef HAVE_CONNTRACK
  else if (!allowed)
    {
      u16 swap = htons(EDE_BLOCKED);

      m = answer_disallowed(header, (size_t)n, (u32)mark, is_single_query ? daemon->namebuff : NULL);
      
      if (have_pseudoheader && m != 0)
	m = add_pseudoheader(header,  m,  ((unsigned char *) header) + udp_size, daemon->edns_pktsz,
			     EDNS0_OPTION_EDE, (unsigned char *)&swap, 2, do_bit, 0);
      
      if (m >= 1)
	{
#ifdef HAVE_DUMPFILE
	  dump_packet_udp(DUMP_REPLY, daemon->packet, m, NULL, &source_addr, listen->fd);
#endif
	  send_from(listen->fd, option_bool(OPT_NOWILD) || option_bool(OPT_CLEVERBIND),
		    (char *)header, m, &source_addr, &dst_addr, if_index);
	  daemon->metrics[METRIC_DNS_LOCAL_ANSWERED]++;
	}
    }
#endif
#ifdef HAVE_AUTH
  else if (auth_dns)
    {
      m = answer_auth(header, ((char *) header) + udp_size, (size_t)n, now, &source_addr, 
		      local_auth, do_bit, have_pseudoheader);
      if (m >= 1)
	{
#ifdef HAVE_DUMPFILE
	  dump_packet_udp(DUMP_REPLY, daemon->packet, m, NULL, &source_addr, listen->fd);
#endif
#if defined(HAVE_CONNTRACK) && defined(HAVE_UBUS)
	  if (local_auth)
	    if (option_bool(OPT_CMARK_ALST_EN) && have_mark && ((u32)mark & daemon->allowlist_mask))
	      report_addresses(header, m, mark);
#endif
	  send_from(listen->fd, option_bool(OPT_NOWILD) || option_bool(OPT_CLEVERBIND),
		    (char *)header, m, &source_addr, &dst_addr, if_index);
	  daemon->metrics[METRIC_DNS_AUTH_ANSWERED]++;
	}
    }
#endif
  else
    {
      int stale;
      int ad_reqd = do_bit;
      u16 hb3 = header->hb3, hb4 = header->hb4;
      int fd = listen->fd;
      
      /* RFC 6840 5.7 */
      if (header->hb4 & HB4_AD)
	ad_reqd = 1;
      
      m = answer_request(header, ((char *) header) + udp_size, (size_t)n, 
			 dst_addr_4, netmask, now, ad_reqd, do_bit, have_pseudoheader, &stale);
      
      if (m >= 1)
	{
	  if (stale && have_pseudoheader)
	    {
	      u16 swap = htons(EDE_STALE);
	      
	      m = add_pseudoheader(header,  m,  ((unsigned char *) header) + udp_size, daemon->edns_pktsz,
				   EDNS0_OPTION_EDE, (unsigned char *)&swap, 2, do_bit, 0);
	    }
#ifdef HAVE_DUMPFILE
	  dump_packet_udp(DUMP_REPLY, daemon->packet, m, NULL, &source_addr, listen->fd);
#endif
#if defined(HAVE_CONNTRACK) && defined(HAVE_UBUS)
	  if (option_bool(OPT_CMARK_ALST_EN) && have_mark && ((u32)mark & daemon->allowlist_mask))
	    report_addresses(header, m, mark);
#endif
	  send_from(listen->fd, option_bool(OPT_NOWILD) || option_bool(OPT_CLEVERBIND),
		    (char *)header, m, &source_addr, &dst_addr, if_index);
	  daemon->metrics[METRIC_DNS_LOCAL_ANSWERED]++;
	  if (stale)
	    daemon->metrics[METRIC_DNS_STALE_ANSWERED]++;
	}
      
      if (m == 0 || stale)
	{
	  if (m != 0)
	    {
	      size_t plen;
	      
	      /* We answered with stale cache data, so forward the query anyway to
		 refresh that. Restore the query from the answer packet. */
	      pheader = find_pseudoheader(header, (size_t)m, &plen, NULL, NULL, NULL);
	      
	      header->hb3 = hb3;
	      header->hb4 = hb4;
	      header->ancount = htons(0);
	      header->nscount = htons(0);
	      header->arcount = htons(0);

	      m = resize_packet(header, m, pheader, plen);

	      /* We've already answered the client, so don't send it the answer 
		 when it comes back. */
	      fd = -1;
	    }
	  
	  if (forward_query(fd, &source_addr, &dst_addr, if_index,
			    header, (size_t)n,  ((char *) header) + udp_size, now, NULL, ad_reqd, do_bit, 0))
	    daemon->metrics[METRIC_DNS_QUERIES_FORWARDED]++;
	  else
	    daemon->metrics[METRIC_DNS_LOCAL_ANSWERED]++;
	}
    }
}

/* Send query in packet, qsize to a server determined by first,last,start and
   get the reply. return reply size. */
static ssize_t tcp_talk(int first, int last, int start, unsigned char *packet,  size_t qsize,
			int have_mark, unsigned int mark, struct server **servp)
{
  int firstsendto = -1;
  u16 *length = (u16 *)packet;
  unsigned char *payload = &packet[2];
  struct dns_header *header = (struct dns_header *)payload;
  unsigned char c1, c2;
  unsigned char hash[HASH_SIZE], *hashp;
  unsigned int rsize;
  
  (void)mark;
  (void)have_mark;

  if (!(hashp = hash_questions(header, (unsigned int)qsize, daemon->namebuff)))
    return 0;

  memcpy(hash, hashp, HASH_SIZE);
  
  while (1) 
    {
      int data_sent = 0;
      struct server *serv;
      
      if (firstsendto == -1)
	firstsendto = start;
      else
	{
	  start++;
	  
	  if (start == last)
	    start = first;
	  
	  if (start == firstsendto)
	    break;
	}
      
      serv = daemon->serverarray[start];
      
    retry:
      *length = htons(qsize);
      
      if (serv->tcpfd == -1)
	{
	  if ((serv->tcpfd = socket(serv->addr.sa.sa_family, SOCK_STREAM, 0)) == -1)
	    continue;
	  
#ifdef HAVE_CONNTRACK
	  /* Copy connection mark of incoming query to outgoing connection. */
	  if (have_mark)
	    setsockopt(serv->tcpfd, SOL_SOCKET, SO_MARK, &mark, sizeof(unsigned int));
#endif			  
	  
	  if ((!local_bind(serv->tcpfd,  &serv->source_addr, serv->interface, 0, 1)))
	    {
	      close(serv->tcpfd);
	      serv->tcpfd = -1;
	      continue;
	    }
	  
#ifdef MSG_FASTOPEN
	  server_send(serv, serv->tcpfd, packet, qsize + sizeof(u16), MSG_FASTOPEN);
	  
	  if (errno == 0)
	    data_sent = 1;
#endif
	  
	  if (!data_sent && connect(serv->tcpfd, &serv->addr.sa, sa_len(&serv->addr)) == -1)
	    {
	      close(serv->tcpfd);
	      serv->tcpfd = -1;
	      continue;
	    }
	  
	  daemon->serverarray[first]->last_server = start;
	  serv->flags &= ~SERV_GOT_TCP;
	}
      
      if ((!data_sent && !read_write(serv->tcpfd, packet, qsize + sizeof(u16), 0)) ||
	  !read_write(serv->tcpfd, &c1, 1, 1) ||
	  !read_write(serv->tcpfd, &c2, 1, 1) ||
	  !read_write(serv->tcpfd, payload, (rsize = (c1 << 8) | c2), 1))
	{
	  close(serv->tcpfd);
	  serv->tcpfd = -1;
	  /* We get data then EOF, reopen connection to same server,
	     else try next. This avoids DoS from a server which accepts
	     connections and then closes them. */
	  if (serv->flags & SERV_GOT_TCP)
	    goto retry;
	  else
	    continue;
	}

      /* If the hash of the question section doesn't match the crc we sent, then
	 someone might be attempting to insert bogus values into the cache by 
	 sending replies containing questions and bogus answers. 
	 Try another server, or give up */
      if (!(hashp = hash_questions(header, rsize, daemon->namebuff)) || memcmp(hash, hashp, HASH_SIZE) != 0)
	continue;
      
      serv->flags |= SERV_GOT_TCP;
      
      *servp = serv;
      return rsize;
    }

  return 0;
}
		  
#ifdef HAVE_DNSSEC
/* Recurse down the key hierarchy */
static int tcp_key_recurse(time_t now, int status, struct dns_header *header, size_t n, 
			   int class, char *name, char *keyname, struct server *server, 
			   int have_mark, unsigned int mark, int *keycount)
{
  int first, last, start, new_status;
  unsigned char *packet = NULL;
  struct dns_header *new_header = NULL;
  
  while (1)
    {
      size_t m;
      int log_save;
            
      /* limit the amount of work we do, to avoid cycling forever on loops in the DNS */
      if (--(*keycount) == 0)
	new_status = STAT_ABANDONED;
      else if (STAT_ISEQUAL(status, STAT_NEED_KEY))
	new_status = dnssec_validate_by_ds(now, header, n, name, keyname, class);
      else if (STAT_ISEQUAL(status, STAT_NEED_DS))
	new_status = dnssec_validate_ds(now, header, n, name, keyname, class);
      else 
	new_status = dnssec_validate_reply(now, header, n, name, keyname, &class,
					   !option_bool(OPT_DNSSEC_IGN_NS) && (server->flags & SERV_DO_DNSSEC),
					   NULL, NULL, NULL);
      
      if (!STAT_ISEQUAL(new_status, STAT_NEED_DS) && !STAT_ISEQUAL(new_status, STAT_NEED_KEY))
	break;

      /* Can't validate because we need a key/DS whose name now in keyname.
	 Make query for same, and recurse to validate */
      if (!packet)
	{
	  packet = whine_malloc(65536 + MAXDNAME + RRFIXEDSZ + sizeof(u16));
	  new_header = (struct dns_header *)&packet[2];
	}
      
      if (!packet)
	{
	  new_status = STAT_ABANDONED;
	  break;
	}

      m = dnssec_generate_query(new_header, ((unsigned char *) new_header) + 65536, keyname, class, 
				STAT_ISEQUAL(new_status, STAT_NEED_KEY) ? T_DNSKEY : T_DS, server->edns_pktsz);
      
      if ((start = dnssec_server(server, daemon->keyname, &first, &last)) == -1 ||
	  (m = tcp_talk(first, last, start, packet, m, have_mark, mark, &server)) == 0)
	{
	  new_status = STAT_ABANDONED;
	  break;
	}

      log_save = daemon->log_display_id;
      daemon->log_display_id = ++daemon->log_id;
      
      log_query_mysockaddr(F_NOEXTRA | F_DNSSEC | F_SERVER, keyname, &server->addr,
			    STAT_ISEQUAL(status, STAT_NEED_KEY) ? "dnssec-query[DNSKEY]" : "dnssec-query[DS]", 0);
            
      new_status = tcp_key_recurse(now, new_status, new_header, m, class, name, keyname, server, have_mark, mark, keycount);

      daemon->log_display_id = log_save;
      
      if (!STAT_ISEQUAL(new_status, STAT_OK))
	break;
    }
    
  if (packet)
    free(packet);
    
  return new_status;
}
#endif


/* The daemon forks before calling this: it should deal with one connection,
   blocking as necessary, and then return. Note, need to be a bit careful
   about resources for debug mode, when the fork is suppressed: that's
   done by the caller. */
unsigned char *tcp_request(int confd, time_t now,
			   union mysockaddr *local_addr, struct in_addr netmask, int auth_dns)
{
  size_t size = 0;
  int norebind;
#ifdef HAVE_CONNTRACK
  int is_single_query = 0, allowed = 1;
#endif
#ifdef HAVE_AUTH
  int local_auth = 0;
#endif
  int checking_disabled, do_bit, added_pheader = 0, have_pseudoheader = 0;
  int cacheable, no_cache_dnssec = 0, cache_secure = 0, bogusanswer = 0;
  size_t m;
  unsigned short qtype;
  unsigned int gotname;
  /* Max TCP packet + slop + size */
  unsigned char *packet = whine_malloc(65536 + MAXDNAME + RRFIXEDSZ + sizeof(u16));
  unsigned char *payload = &packet[2];
  unsigned char c1, c2;
  /* largest field in header is 16-bits, so this is still sufficiently aligned */
  struct dns_header *header = (struct dns_header *)payload;
  u16 *length = (u16 *)packet;
  struct server *serv;
  struct in_addr dst_addr_4;
  union mysockaddr peer_addr;
  socklen_t peer_len = sizeof(union mysockaddr);
  int query_count = 0;
  unsigned char *pheader;
  unsigned int mark = 0;
  int have_mark = 0;
  int first, last, stale, do_stale = 0;
  unsigned int flags = 0;
  u16 hb3, hb4;
    
  if (!packet || getpeername(confd, (struct sockaddr *)&peer_addr, &peer_len) == -1)
    return packet;

#ifdef HAVE_CONNTRACK
  /* Get connection mark of incoming query to set on outgoing connections. */
  if (option_bool(OPT_CONNTRACK) || option_bool(OPT_CMARK_ALST_EN))
    {
      union all_addr local;
		      
      if (local_addr->sa.sa_family == AF_INET6)
	local.addr6 = local_addr->in6.sin6_addr;
      else
	local.addr4 = local_addr->in.sin_addr;
      
      have_mark = get_incoming_mark(&peer_addr, &local, 1, &mark);
    }
#endif	

  /* We can be configured to only accept queries from at-most-one-hop-away addresses. */
  if (option_bool(OPT_LOCAL_SERVICE))
    {
      struct addrlist *addr;

      if (peer_addr.sa.sa_family == AF_INET6) 
	{
	  for (addr = daemon->interface_addrs; addr; addr = addr->next)
	    if ((addr->flags & ADDRLIST_IPV6) &&
		is_same_net6(&addr->addr.addr6, &peer_addr.in6.sin6_addr, addr->prefixlen))
	      break;
	}
      else
	{
	  struct in_addr netmask;
	  for (addr = daemon->interface_addrs; addr; addr = addr->next)
	    {
	      netmask.s_addr = htonl(~(in_addr_t)0 << (32 - addr->prefixlen));
	      if (!(addr->flags & ADDRLIST_IPV6) && 
		  is_same_net(addr->addr.addr4, peer_addr.in.sin_addr, netmask))
		break;
	    }
	}
      if (!addr)
	{
	  prettyprint_addr(&peer_addr, daemon->addrbuff);
	  my_syslog(LOG_WARNING, _("ignoring query from non-local network %s"), daemon->addrbuff);
	  return packet;
	}
    }

  while (1)
    {
      int ede = EDE_UNSET;

      if (query_count == TCP_MAX_QUERIES)
	return packet;

      if (do_stale)
	{
	  size_t plen;

	  /* We answered the last query with stale data. Now try and get fresh data.
	     Restore query from answer. */
	  pheader = find_pseudoheader(header, m, &plen, NULL, NULL, NULL);
	  
	  header->hb3 = hb3;
	  header->hb4 = hb4;
	  header->ancount = htons(0);
	  header->nscount = htons(0);
	  header->arcount = htons(0);
	  
	  size = resize_packet(header, m, pheader, plen);
	}
      else
	{
	  if (!read_write(confd, &c1, 1, 1) || !read_write(confd, &c2, 1, 1) ||
	      !(size = c1 << 8 | c2) ||
	      !read_write(confd, payload, size, 1))
	    return packet;
	  
	  /* for stale-answer processing. */
	  hb3 = header->hb3;
	  hb4 = header->hb4;
	}
      
      if (size < (int)sizeof(struct dns_header))
	continue;

      /* Clear buffer beyond request to avoid risk of
	 information disclosure. */
      memset(payload + size, 0, 65536 - size);
      
      query_count++;

      /* log_query gets called indirectly all over the place, so 
	 pass these in global variables - sorry. */
      daemon->log_display_id = ++daemon->log_id;
      daemon->log_source_addr = &peer_addr;
      
      /* save state of "cd" flag in query */
      if ((checking_disabled = header->hb4 & HB4_CD))
	no_cache_dnssec = 1;
       
      if ((gotname = extract_request(header, (unsigned int)size, daemon->namebuff, &qtype)))
	{
#ifdef HAVE_AUTH
	  struct auth_zone *zone;
#endif

#ifdef HAVE_CONNTRACK
	  is_single_query = 1;
#endif

	  if (!do_stale)
	    {
	      log_query_mysockaddr(F_QUERY | F_FORWARD, daemon->namebuff,
				   &peer_addr, auth_dns ? "auth" : "query", qtype);
	      
#ifdef HAVE_AUTH
	      /* find queries for zones we're authoritative for, and answer them directly */
	      if (!auth_dns && !option_bool(OPT_LOCALISE))
		for (zone = daemon->auth_zones; zone; zone = zone->next)
		  if (in_zone(zone, daemon->namebuff, NULL))
		    {
		      auth_dns = 1;
		      local_auth = 1;
		      break;
		    }
#endif
	    }
	}
      
      norebind = domain_no_rebind(daemon->namebuff);
      
      if (local_addr->sa.sa_family == AF_INET)
	dst_addr_4 = local_addr->in.sin_addr;
      else
	dst_addr_4.s_addr = 0;
      
      do_bit = 0;

      if (find_pseudoheader(header, (size_t)size, NULL, &pheader, NULL, NULL))
	{ 
	  unsigned short flags;
	  
	  have_pseudoheader = 1;
	  pheader += 4; /* udp_size, ext_rcode */
	  GETSHORT(flags, pheader);
      
	  if (flags & 0x8000)
	    do_bit = 1; /* do bit */ 
	}
      
#ifdef HAVE_CONNTRACK
#ifdef HAVE_AUTH
      if (!auth_dns || local_auth)
#endif
	if (option_bool(OPT_CMARK_ALST_EN) && have_mark && ((u32)mark & daemon->allowlist_mask))
	  allowed = is_query_allowed_for_mark((u32)mark, is_single_query ? daemon->namebuff : NULL);
#endif

      if (0);
#ifdef HAVE_CONNTRACK
      else if (!allowed)
	{
	  u16 swap = htons(EDE_BLOCKED);

	  m = answer_disallowed(header, size, (u32)mark, is_single_query ? daemon->namebuff : NULL);
	  
	  if (have_pseudoheader && m != 0)
	    m = add_pseudoheader(header,  m, ((unsigned char *) header) + 65536, daemon->edns_pktsz,
				 EDNS0_OPTION_EDE, (unsigned char *)&swap, 2, do_bit, 0);
	}
#endif
#ifdef HAVE_AUTH
      else if (auth_dns)
	m = answer_auth(header, ((char *) header) + 65536, (size_t)size, now, &peer_addr, 
			local_auth, do_bit, have_pseudoheader);
#endif
      else
	{
	   int ad_reqd = do_bit;
	   /* RFC 6840 5.7 */
	   if (header->hb4 & HB4_AD)
	     ad_reqd = 1;

	   if (do_stale)
	     m = 0;
	   else
	     /* m > 0 if answered from cache */
	     m = answer_request(header, ((char *) header) + 65536, (size_t)size, 
				dst_addr_4, netmask, now, ad_reqd, do_bit, have_pseudoheader, &stale);
	   
	  /* Do this by steam now we're not in the select() loop */
	  check_log_writer(1); 
	  
	  if (m == 0)
	    {
	      struct server *master;
	      int start;

	      if (lookup_domain(daemon->namebuff, gotname, &first, &last))
		flags = is_local_answer(now, first, daemon->namebuff);
	      else
		{
		  /* No configured servers */
		  ede = EDE_NOT_READY;
		  flags = 0;
		}
	      
	      /* don't forward A or AAAA queries for simple names, except the empty name */
	      if (!flags &&
		  option_bool(OPT_NODOTS_LOCAL) &&
		  (gotname & (F_IPV4 | F_IPV6)) &&
		  !strchr(daemon->namebuff, '.') &&
		  strlen(daemon->namebuff) != 0)
		flags = check_for_local_domain(daemon->namebuff, now) ? F_NOERR : F_NXDOMAIN;
		
	      if (!flags && ede != EDE_NOT_READY)
		{
		  master = daemon->serverarray[first];
		  
		  if (option_bool(OPT_ORDER) || master->last_server == -1)
		    start = first;
		  else
		    start = master->last_server;
		  
		  size = add_edns0_config(header, size, ((unsigned char *) header) + 65536, &peer_addr, now, &cacheable);
		  
#ifdef HAVE_DNSSEC
		  if (option_bool(OPT_DNSSEC_VALID) && (master->flags & SERV_DO_DNSSEC))
		    {
		      size = add_do_bit(header, size, ((unsigned char *) header) + 65536);
		      
		      /* For debugging, set Checking Disabled, otherwise, have the upstream check too,
			 this allows it to select auth servers when one is returning bad data. */
		      if (option_bool(OPT_DNSSEC_DEBUG))
			header->hb4 |= HB4_CD;
		    }
#endif
		  
		  /* Check if we added a pheader on forwarding - may need to
		     strip it from the reply. */
		  if (!have_pseudoheader && find_pseudoheader(header, size, NULL, NULL, NULL, NULL))
		    added_pheader = 1;
		  
		  /* Loop round available servers until we succeed in connecting to one. */
		  if ((m = tcp_talk(first, last, start, packet, size, have_mark, mark, &serv)) == 0)
		    {
		      ede = EDE_NETERR;
		      break;
		    }
		  
		  /* get query name again for logging - may have been overwritten */
		  if (!(gotname = extract_request(header, (unsigned int)size, daemon->namebuff, &qtype)))
		    strcpy(daemon->namebuff, "query");
		  log_query_mysockaddr(F_SERVER | F_FORWARD, daemon->namebuff, &serv->addr, NULL, 0);
		  
#ifdef HAVE_DNSSEC
		  if (option_bool(OPT_DNSSEC_VALID) && !checking_disabled && (master->flags & SERV_DO_DNSSEC))
		    {
		      int keycount = DNSSEC_WORK; /* Limit to number of DNSSEC questions, to catch loops and avoid filling cache. */
		      int status = tcp_key_recurse(now, STAT_OK, header, m, 0, daemon->namebuff, daemon->keyname, 
						   serv, have_mark, mark, &keycount);
		      char *result, *domain = "result";
		      
		      union all_addr a;
		      a.log.ede = ede = errflags_to_ede(status);
		      
		      if (STAT_ISEQUAL(status, STAT_ABANDONED))
			{
			  result = "ABANDONED";
			  status = STAT_BOGUS;
			}
		      else
			result = (STAT_ISEQUAL(status, STAT_SECURE) ? "SECURE" : (STAT_ISEQUAL(status, STAT_INSECURE) ? "INSECURE" : "BOGUS"));
		      
		      if (STAT_ISEQUAL(status, STAT_SECURE))
			cache_secure = 1;
		      else if (STAT_ISEQUAL(status, STAT_BOGUS))
			{
			  no_cache_dnssec = 1;
			  bogusanswer = 1;
			  
			  if (extract_request(header, m, daemon->namebuff, NULL))
			    domain = daemon->namebuff;
			}
		      
		      log_query(F_SECSTAT, domain, &a, result, 0);
		    }
#endif
		  
		  /* restore CD bit to the value in the query */
		  if (checking_disabled)
		    header->hb4 |= HB4_CD;
		  else
		    header->hb4 &= ~HB4_CD;
		  
		  /* Never cache answers which are contingent on the source or MAC address EDSN0 option,
		     since the cache is ignorant of such things. */
		  if (!cacheable)
		    no_cache_dnssec = 1;
		  
		  m = process_reply(header, now, serv, (unsigned int)m, 
				    option_bool(OPT_NO_REBIND) && !norebind, no_cache_dnssec, cache_secure, bogusanswer,
				    ad_reqd, do_bit, added_pheader, &peer_addr, ((unsigned char *)header) + 65536, ede); 
		}
	    }
	}
	
      if (do_stale)
	break;
    
      /* In case of local answer or no connections made. */
      if (m == 0)
	{
	  if (!(m = make_local_answer(flags, gotname, size, header, daemon->namebuff,
				      ((char *) header) + 65536, first, last, ede)))
	    break;
	  
	  if (have_pseudoheader)
	    {
	      u16 swap = htons((u16)ede);
	      
	      if (ede != EDE_UNSET)
		m = add_pseudoheader(header, m, ((unsigned char *) header) + 65536, daemon->edns_pktsz, EDNS0_OPTION_EDE, (unsigned char *)&swap, 2, do_bit, 0);
	      else
		m = add_pseudoheader(header, m, ((unsigned char *) header) + 65536, daemon->edns_pktsz, 0, NULL, 0, do_bit, 0);
	    }
	}
      else if (stale)
	 {
	   u16 swap = htons((u16)EDE_STALE);
	   
	   m = add_pseudoheader(header, m, ((unsigned char *) header) + 65536, daemon->edns_pktsz, EDNS0_OPTION_EDE, (unsigned char *)&swap, 2, do_bit, 0);
	 }
      
      check_log_writer(1);
      
      *length = htons(m);
      
#if defined(HAVE_CONNTRACK) && defined(HAVE_UBUS)
#ifdef HAVE_AUTH
      if (!auth_dns || local_auth)
#endif
	if (option_bool(OPT_CMARK_ALST_EN) && have_mark && ((u32)mark & daemon->allowlist_mask))
	  report_addresses(header, m, mark);
#endif
      if (!read_write(confd, packet, m + sizeof(u16), 0))
	break;
      
      /* If we answered with stale data, this process will now try and get fresh data into
	 the cache then and cannot therefore accept new queries. Close the incoming
	 connection to signal that to the client. Then set do_stale and loop round
	 once more to try and get fresh data, after which we exit. */
      if (stale)
	{
	  shutdown(confd, SHUT_RDWR);
	  close(confd);
	  do_stale = 1;
	}
    }

  /* If we ran once to get fresh data, confd is already closed. */
  if (!do_stale)
    {
      shutdown(confd, SHUT_RDWR);
      close(confd);
    }

  return packet;
}

/* return a UDP socket bound to a random port, have to cope with straying into
   occupied port nos and reserved ones. */
static int random_sock(struct server *s)
{
  int fd;

  if ((fd = socket(s->source_addr.sa.sa_family, SOCK_DGRAM, 0)) != -1)
    {
      /* We need to set IPV6ONLY so we can use the same ports
	 for IPv4 and IPV6, otherwise, in restriced port situations,
	 we can end up with all our available ports in use for 
	 one address family, and the other address family cannot be used. */
      if (s->source_addr.sa.sa_family == AF_INET6)
	{
	  int opt = 1;

	  if (setsockopt(fd, IPPROTO_IPV6, IPV6_V6ONLY, &opt, sizeof(opt)) == -1)
	    {
	      close(fd);
	      return -1;
	    }
	}
      
      if (local_bind(fd, &s->source_addr, s->interface, s->ifindex, 0))
	return fd;

      /* don't log errors due to running out of available ports, we handle those. */
      if (!sockaddr_isnull(&s->source_addr) || errno != EADDRINUSE)
	{
	  if (s->interface[0] == 0)
	    (void)prettyprint_addr(&s->source_addr, daemon->addrbuff);
	  else
	    safe_strncpy(daemon->addrbuff, s->interface, ADDRSTRLEN);
	  
	  my_syslog(LOG_ERR, _("failed to bind server socket to %s: %s"),
		    daemon->addrbuff, strerror(errno));
	}
	  
      close(fd);
    }
  
  return -1;
}

/* compare source addresses and interface, serv2 can be null. */
static int server_isequal(const struct server *serv1,
			 const struct server *serv2)
{
  return (serv2 &&
    serv2->ifindex == serv1->ifindex &&
    sockaddr_isequal(&serv2->source_addr, &serv1->source_addr) &&
    strncmp(serv2->interface, serv1->interface, IF_NAMESIZE) == 0);
}

/* fdlp points to chain of randomfds already in use by transaction.
   If there's already a suitable one, return it, else allocate a 
   new one and add it to the list. 

   Not leaking any resources in the face of allocation failures
   is rather convoluted here.
   
   Note that rfd->serv may be NULL, when a server goes away.
*/
int allocate_rfd(struct randfd_list **fdlp, struct server *serv)
{
  static int finger = 0;
  int i, j = 0;
  int ports_full = 0;
  struct randfd_list **up, *rfl, *found, **found_link;
  struct randfd *rfd = NULL;
  int fd = 0;
  int ports_avail = 0;
  
  /* We can't have more randomsocks for this AF available than ports in  our port range,
     so check that here, to avoid trying and failing to bind every port
     in local_bind(), called from random_sock(). The actual check is below when 
     ports_avail != 0 */
  if (daemon->max_port != 0)
    {
      ports_avail = daemon->max_port - daemon->min_port + 1;
      if (ports_avail >= SMALL_PORT_RANGE)
	ports_avail = 0;
    }
  
  /* If server has a pre-allocated fd, use that. */
  if (serv->sfd)
    return serv->sfd->fd;
  
  /* existing suitable random port socket linked to this transaction?
     Find the last one in the list and count how many there are. */
  for (found = NULL, found_link = NULL, i = 0, up = fdlp, rfl = *fdlp; rfl; up = &rfl->next, rfl = rfl->next)
    if (server_isequal(serv, rfl->rfd->serv))
      {
	i++;
	found = rfl;
	found_link = up;
      }

  /* We have the maximum number for this query already. Promote
     the last one on the list to the head, to circulate them,
     and return it. */
  if (found && i >= daemon->randport_limit)
    {
      *found_link = found->next;
      found->next = *fdlp;
      *fdlp = found;
      return found->rfd->fd;
    }

  /* check for all available ports in use. */
  if (ports_avail != 0)
    {
      int ports_inuse;

      for (ports_inuse = 0, i = 0; i < daemon->numrrand; i++)
	if (daemon->randomsocks[i].refcount != 0 &&
	    daemon->randomsocks[i].serv->source_addr.sa.sa_family == serv->source_addr.sa.sa_family &&
	    ++ports_inuse >= ports_avail)
	  {
	    ports_full = 1;
	    break;
	  }
    }
  
  /* limit the number of sockets we have open to avoid starvation of 
     (eg) TFTP. Once we have a reasonable number, randomness should be OK */
  if (!ports_full)
    for (i = 0; i < daemon->numrrand; i++)
      if (daemon->randomsocks[i].refcount == 0)
	{
	  if ((fd = random_sock(serv)) != -1)
	    {
	      rfd = &daemon->randomsocks[i];
	      rfd->serv = serv;
	      rfd->fd = fd;
	      rfd->refcount = 1;
	    }
	  break;
	}
    
  /* No good existing. Need new link. */
  if ((rfl = daemon->rfl_spare))
    daemon->rfl_spare = rfl->next;
  else if (!(rfl = whine_malloc(sizeof(struct randfd_list))))
    {
      /* malloc failed, don't leak allocated sock */
      if (rfd)
	{
	  close(rfd->fd);
	  rfd->refcount = 0;
	}

      return -1;
    }
  
  /* No free ones or cannot get new socket, grab an existing one */
  if (!rfd)
    for (j = 0; j < daemon->numrrand; j++)
      {
	i = (j + finger) % daemon->numrrand;
	if (daemon->randomsocks[i].refcount != 0 &&
	    server_isequal(serv, daemon->randomsocks[i].serv) &&
	    daemon->randomsocks[i].refcount != 0xfffe)
	  {
	    struct randfd_list *rl;
	    /* Don't pick one we already have. */
	    for (rl = *fdlp; rl; rl = rl->next)
	      if (rl->rfd == &daemon->randomsocks[i])
		break;

	    if (!rl)
	      {
		finger = i + 1;
		rfd = &daemon->randomsocks[i];
		rfd->refcount++;
		break;
	      }
	  }
      }

  if (!rfd) /* should be when j == daemon->numrrand */
    {
      struct randfd_list *rfl_poll;

      /* there are no free slots, and non with the same parameters we can piggy-back on. 
	 We're going to have to allocate a new temporary record, distinguished by
	 refcount == 0xffff. This will exist in the frec randfd list, never be shared,
	 and be freed when no longer in use. It will also be held on 
	 the daemon->rfl_poll list so the poll system can find it. */

      if ((rfl_poll = daemon->rfl_spare))
	daemon->rfl_spare = rfl_poll->next;
      else
	rfl_poll = whine_malloc(sizeof(struct randfd_list));
      
      if (!rfl_poll ||
	  !(rfd = whine_malloc(sizeof(struct randfd))) ||
	  (fd = random_sock(serv)) == -1)
	{
	  
	  /* Don't leak anything we may already have */
	  rfl->next = daemon->rfl_spare;
	  daemon->rfl_spare = rfl;

	  if (rfl_poll)
	    {
	      rfl_poll->next = daemon->rfl_spare;
	      daemon->rfl_spare = rfl_poll;
	    }
	  
	  if (rfd)
	    free(rfd);
	  
	  return -1; /* doom */
	}

      /* Note rfd->serv not set here, since it's not reused */
      rfd->fd = fd;
      rfd->refcount = 0xffff; /* marker for temp record */

      rfl_poll->rfd = rfd;
      rfl_poll->next = daemon->rfl_poll;
      daemon->rfl_poll = rfl_poll;
    }
  
  rfl->rfd = rfd;
  rfl->next = *fdlp;
  *fdlp = rfl;
  
  return rfl->rfd->fd;
}

void free_rfds(struct randfd_list **fdlp)
{
  struct randfd_list *tmp, *rfl, *poll, *next, **up;
  
  for (rfl = *fdlp; rfl; rfl = tmp)
    {
      if (rfl->rfd->refcount == 0xffff || --(rfl->rfd->refcount) == 0)
	close(rfl->rfd->fd);

      /* temporary overflow record */
      if (rfl->rfd->refcount == 0xffff)
	{
	  free(rfl->rfd);
	  
	  /* go through the link of all these by steam to delete.
	     This list is expected to be almost always empty. */
	  for (poll = daemon->rfl_poll, up = &daemon->rfl_poll; poll; poll = next)
	    {
	      next = poll->next;
	      
	      if (poll->rfd == rfl->rfd)
		{
		  *up = poll->next;
		  poll->next = daemon->rfl_spare;
		  daemon->rfl_spare = poll;
		}
	      else
		up = &poll->next;
	    }
	}

      tmp = rfl->next;
      rfl->next = daemon->rfl_spare;
      daemon->rfl_spare = rfl;
    }

  *fdlp = NULL;
}

static void free_frec(struct frec *f)
{
  struct frec_src *last;
  
  /* add back to freelist if not the record builtin to every frec. */
  for (last = f->frec_src.next; last && last->next; last = last->next) ;
  if (last)
    {
      last->next = daemon->free_frec_src;
      daemon->free_frec_src = f->frec_src.next;
    }
    
  f->frec_src.next = NULL;    
  free_rfds(&f->rfds);
  f->sentto = NULL;
  f->flags = 0;

  if (f->stash)
    {
      blockdata_free(f->stash);
      f->stash = NULL;
    }
  
#ifdef HAVE_DNSSEC
  /* Anything we're waiting on is pointless now, too */
  if (f->blocking_query)
    {
      struct frec *n, **up;

      /* unlink outselves from the blocking query's dependents list. */
      for (n = f->blocking_query->dependent, up = &f->blocking_query->dependent; n; n = n->next_dependent)
	if (n == f)
	  {
	    *up = n->next_dependent;
	    break;
	  }
	else
	  up = &n->next_dependent;

      /* If we were the only/last dependent, free the blocking query too. */
      if (!f->blocking_query->dependent)
	free_frec(f->blocking_query);
    }
  
  f->blocking_query = NULL;
  f->dependent = NULL;
  f->next_dependent = NULL;
#endif
}



/* Impose an absolute
   limit of 4*TIMEOUT before we wipe things (for random sockets).
   If force is set, always return a result, even if we have
   to allocate above the limit, and don'y free any records.
   This is set when allocating for DNSSEC to avoid cutting off
   the branch we are sitting on. */
static struct frec *get_new_frec(time_t now, struct server *master, int force)
{
  struct frec *f, *oldest, *target;
  int count;
  
  /* look for free records, garbage collect old records and count number in use by our server-group. */
  for (f = daemon->frec_list, oldest = NULL, target =  NULL, count = 0; f; f = f->next)
    {
      if (!f->sentto)
	target = f;
      else
	{
#ifdef HAVE_DNSSEC
	  /* Don't free DNSSEC sub-queries here, as we may end up with
	     dangling references to them. They'll go when their "real" query 
	     is freed. */
	  if (!f->dependent && !force)
#endif
	    {
	      if (difftime(now, f->time) >= 4*TIMEOUT)
		{
		  daemon->metrics[METRIC_DNS_UNANSWERED_QUERY]++;
		  free_frec(f);
		  target = f;
		}
	      else if (!oldest || difftime(f->time, oldest->time) <= 0)
		oldest = f;
	    }
	}
      
      if (f->sentto && ((int)difftime(now, f->time)) < TIMEOUT && server_samegroup(f->sentto, master))
	count++;
    }

  if (!force && count >= daemon->ftabsize)
    {
      query_full(now, master->domain);
      return NULL;
    }
  
  if (!target && oldest && ((int)difftime(now, oldest->time)) >= TIMEOUT)
    { 
      /* can't find empty one, use oldest if there is one and it's older than timeout */
      daemon->metrics[METRIC_DNS_UNANSWERED_QUERY]++;
      free_frec(oldest);
      target = oldest;
    }
  
  if (!target && (target = (struct frec *)whine_malloc(sizeof(struct frec))))
    {
      target->next = daemon->frec_list;
      daemon->frec_list = target;
    }

  if (target)
    {
      target->time = now;
      target->forward_delay = daemon->fast_retry_time;
    }
  
  return target;
}

static void query_full(time_t now, char *domain)
{
  static time_t last_log = 0;
  
  if ((int)difftime(now, last_log) > 5)
    {
      last_log = now;
      if (!domain || strlen(domain) == 0)
	my_syslog(LOG_WARNING, _("Maximum number of concurrent DNS queries reached (max: %d)"), daemon->ftabsize);
      else
	my_syslog(LOG_WARNING, _("Maximum number of concurrent DNS queries to %s reached (max: %d)"), domain, daemon->ftabsize);
    }
}


static struct frec *lookup_frec(unsigned short id, int fd, void *hash, int *firstp, int *lastp)
{
  struct frec *f;
  struct server *s;
  int first, last;
  struct randfd_list *fdl;

  if (hash)
    for (f = daemon->frec_list; f; f = f->next)
      if (f->sentto && f->new_id == id && 
	  (memcmp(hash, f->hash, HASH_SIZE) == 0))
	{
	  filter_servers(f->sentto->arrayposn, F_SERVER, firstp, lastp);
	  
	  /* sent from random port */
	  for (fdl = f->rfds; fdl; fdl = fdl->next)
	    if (fdl->rfd->fd == fd)
	      return f;
	  
	  /* Sent to upstream from socket associated with a server. 
	     Note we have to iterate over all the possible servers, since they may
	     have different bound sockets. */
	  for (first = *firstp, last = *lastp; first != last; first++)
	    {
	      s = daemon->serverarray[first];
	      if (s->sfd && s->sfd->fd == fd)
		return f;
	    }
	}
  
  return NULL;
}

static struct frec *lookup_frec_by_query(void *hash, unsigned int flags, unsigned int flagmask)
{
  struct frec *f;

  if (hash)
    for (f = daemon->frec_list; f; f = f->next)
      if (f->sentto &&
	  (f->flags & flagmask) == flags &&
	  memcmp(hash, f->hash, HASH_SIZE) == 0)
	return f;
  
  return NULL;
}

#ifdef HAVE_DNSSEC
/* DNSSEC frecs have the complete query in the block stash.
   Search for an existing query using that. */
static struct frec *lookup_frec_dnssec(char *target, int class, int flags, struct dns_header *header)
{
   struct frec *f;

   for (f = daemon->frec_list; f; f = f->next)
     if (f->sentto &&
	 (f->flags & flags) &&
	 blockdata_retrieve(f->stash, f->stash_len, (void *)header))
       {
	 unsigned char *p = (unsigned char *)(header+1);
	 int hclass;

	 if (extract_name(header, f->stash_len, &p, target, 0, 4) != 1)
	   continue;

	 p += 2;  /* type, known from flags */ 
	 GETSHORT(hclass, p);

	 if (class != hclass)
	   continue;

	 return f;
       }

   return NULL;
}
#endif

/* Send query packet again, if we can. */
void resend_query()
{
  if (daemon->srv_save)
    server_send(daemon->srv_save, daemon->fd_save,
		daemon->packet, daemon->packet_len, 0);
}

/* A server record is going away, remove references to it */
void server_gone(struct server *server)
{
  struct frec *f;
  int i;
  
  for (f = daemon->frec_list; f; f = f->next)
    if (f->sentto && f->sentto == server)
      free_frec(f);

  /* If any random socket refers to this server, NULL the reference.
     No more references to the socket will be created in the future. */
  for (i = 0; i < daemon->numrrand; i++)
    if (daemon->randomsocks[i].refcount != 0 && daemon->randomsocks[i].serv == server)
      daemon->randomsocks[i].serv = NULL;
  
  if (daemon->srv_save == server)
    daemon->srv_save = NULL;
}

/* return unique random ids. */
static unsigned short get_id(void)
{
  unsigned short ret = 0;
  struct frec *f;
  
  while (1)
    {
      ret = rand16();

      /* ensure id is unique. */
      for (f = daemon->frec_list; f; f = f->next)
	if (f->sentto && f->new_id == ret)
	  break;

      if (!f)
	return ret;
    }
}
