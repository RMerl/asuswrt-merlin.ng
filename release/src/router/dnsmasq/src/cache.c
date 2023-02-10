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

static struct crec *cache_head = NULL, *cache_tail = NULL, **hash_table = NULL;
#ifdef HAVE_DHCP
static struct crec *dhcp_spare = NULL;
#endif
static struct crec *new_chain = NULL;
static int insert_error;
static union bigname *big_free = NULL;
static int bignames_left, hash_size;

static void make_non_terminals(struct crec *source);
static struct crec *really_insert(char *name, union all_addr *addr, unsigned short class,
				  time_t now,  unsigned long ttl, unsigned int flags);
static void dump_cache_entry(struct crec *cache, time_t now);

/* type->string mapping: this is also used by the name-hash function as a mixing table. */
/* taken from https://www.iana.org/assignments/dns-parameters/dns-parameters.xhtml */
static const struct {
  unsigned int type;
  const char * const name;
} typestr[] = {
  { 1,   "A" }, /* a host address [RFC1035] */
  { 2,   "NS" }, /* an authoritative name server [RFC1035] */
  { 3,   "MD" }, /* a mail destination (OBSOLETE - use MX) [RFC1035] */
  { 4,   "MF" }, /* a mail forwarder (OBSOLETE - use MX) [RFC1035] */
  { 5,   "CNAME" }, /* the canonical name for an alias [RFC1035] */
  { 6,   "SOA" }, /* marks the start of a zone of authority [RFC1035] */
  { 7,   "MB" }, /* a mailbox domain name (EXPERIMENTAL) [RFC1035] */
  { 8,   "MG" }, /* a mail group member (EXPERIMENTAL) [RFC1035] */
  { 9,   "MR" }, /* a mail rename domain name (EXPERIMENTAL) [RFC1035] */
  { 10,  "NULL" }, /* a null RR (EXPERIMENTAL) [RFC1035] */
  { 11,  "WKS" }, /* a well known service description [RFC1035] */
  { 12,  "PTR" }, /* a domain name pointer [RFC1035] */
  { 13,  "HINFO" }, /* host information [RFC1035] */
  { 14,  "MINFO" }, /* mailbox or mail list information [RFC1035] */
  { 15,  "MX" }, /* mail exchange [RFC1035] */
  { 16,  "TXT" }, /* text strings [RFC1035] */
  { 17,  "RP" }, /* for Responsible Person [RFC1183] */
  { 18,  "AFSDB" }, /* for AFS Data Base location [RFC1183][RFC5864] */
  { 19,  "X25" }, /* for X.25 PSDN address [RFC1183] */
  { 20,  "ISDN" }, /* for ISDN address [RFC1183] */
  { 21,  "RT" }, /* for Route Through [RFC1183] */
  { 22,  "NSAP" }, /* for NSAP address, NSAP style A record [RFC1706] */
  { 23,  "NSAP_PTR" }, /* for domain name pointer, NSAP style [RFC1348][RFC1637][RFC1706] */
  { 24,  "SIG" }, /* for security signature [RFC2535][RFC2536][RFC2537][RFC2931][RFC3008][RFC3110][RFC3755][RFC4034] */
  { 25,  "KEY" }, /* for security key [RFC2535][RFC2536][RFC2537][RFC2539][RFC3008][RFC3110][RFC3755][RFC4034] */
  { 26,  "PX" }, /* X.400 mail mapping information [RFC2163] */
  { 27,  "GPOS" }, /* Geographical Position [RFC1712] */
  { 28,  "AAAA" }, /* IP6 Address [RFC3596] */
  { 29,  "LOC" }, /* Location Information [RFC1876] */
  { 30,  "NXT" }, /* Next Domain (OBSOLETE) [RFC2535][RFC3755] */
  { 31,  "EID" }, /* Endpoint Identifier [Michael_Patton][http://ana-3.lcs.mit.edu/~jnc/nimrod/dns.txt] 1995-06*/
  { 32,  "NIMLOC" }, /* Nimrod Locator [1][Michael_Patton][http://ana-3.lcs.mit.edu/~jnc/nimrod/dns.txt] 1995-06*/
  { 33,  "SRV" }, /* Server Selection [1][RFC2782] */
  { 34,  "ATMA" }, /* ATM Address [ ATM Forum Technical Committee, "ATM Name System, V2.0", Doc ID: AF-DANS-0152.000, July 2000. Available from and held in escrow by IANA.] */
  { 35,  "NAPTR" }, /* Naming Authority Pointer [RFC2168][RFC2915][RFC3403] */
  { 36,  "KX" }, /* Key Exchanger [RFC2230] */
  { 37,  "CERT" }, /* CERT [RFC4398] */
  { 38,  "A6" }, /* A6 (OBSOLETE - use AAAA) [RFC2874][RFC3226][RFC6563] */
  { 39,  "DNAME" }, /* DNAME [RFC6672] */
  { 40,  "SINK" }, /* SINK [Donald_E_Eastlake][http://tools.ietf.org/html/draft-eastlake-kitchen-sink] 1997-11*/
  { 41,  "OPT" }, /* OPT [RFC3225][RFC6891] */
  { 42,  "APL" }, /* APL [RFC3123] */
  { 43,  "DS" }, /* Delegation Signer [RFC3658][RFC4034] */
  { 44,  "SSHFP" }, /* SSH Key Fingerprint [RFC4255] */
  { 45,  "IPSECKEY" }, /* IPSECKEY [RFC4025] */
  { 46,  "RRSIG" }, /* RRSIG [RFC3755][RFC4034] */
  { 47,  "NSEC" }, /* NSEC [RFC3755][RFC4034][RFC9077] */
  { 48,  "DNSKEY" }, /* DNSKEY [RFC3755][RFC4034] */
  { 49,  "DHCID" }, /* DHCID [RFC4701] */
  { 50,  "NSEC3" }, /* NSEC3 [RFC5155][RFC9077] */
  { 51,  "NSEC3PARAM" }, /* NSEC3PARAM [RFC5155] */
  { 52,  "TLSA" }, /* TLSA [RFC6698] */
  { 53,  "SMIMEA" }, /* S/MIME cert association [RFC8162] SMIMEA/smimea-completed-template 2015-12-01*/
  { 55,  "HIP" }, /* Host Identity Protocol [RFC8005] */
  { 56,  "NINFO" }, /* NINFO [Jim_Reid] NINFO/ninfo-completed-template 2008-01-21*/
  { 57,  "RKEY" }, /* RKEY [Jim_Reid] RKEY/rkey-completed-template 2008-01-21*/
  { 58,  "TALINK" }, /* Trust Anchor LINK [Wouter_Wijngaards] TALINK/talink-completed-template 2010-02-17*/
  { 59,  "CDS" }, /* Child DS [RFC7344] CDS/cds-completed-template 2011-06-06*/
  { 60,  "CDNSKEY" }, /* DNSKEY(s) the Child wants reflected in DS [RFC7344] 2014-06-16*/
  { 61,  "OPENPGPKEY" }, /* OpenPGP Key [RFC7929] OPENPGPKEY/openpgpkey-completed-template 2014-08-12*/
  { 62,  "CSYNC" }, /* Child-To-Parent Synchronization [RFC7477] 2015-01-27*/
  { 63,  "ZONEMD" }, /* Message Digest Over Zone Data [RFC8976] ZONEMD/zonemd-completed-template 2018-12-12*/
  { 64,  "SVCB" }, /* Service Binding [draft-ietf-dnsop-svcb-https-00] SVCB/svcb-completed-template 2020-06-30*/
  { 65,  "HTTPS" }, /* HTTPS Binding [draft-ietf-dnsop-svcb-https-00] HTTPS/https-completed-template 2020-06-30*/
  { 99,  "SPF" }, /* [RFC7208] */
  { 100, "UINFO" }, /* [IANA-Reserved] */
  { 101, "UID" }, /* [IANA-Reserved] */
  { 102, "GID" }, /* [IANA-Reserved] */
  { 103, "UNSPEC" }, /* [IANA-Reserved] */
  { 104, "NID" }, /* [RFC6742] ILNP/nid-completed-template */
  { 105, "L32" }, /* [RFC6742] ILNP/l32-completed-template */
  { 106, "L64" }, /* [RFC6742] ILNP/l64-completed-template */
  { 107, "LP" }, /* [RFC6742] ILNP/lp-completed-template */
  { 108, "EUI48" }, /* an EUI-48 address [RFC7043] EUI48/eui48-completed-template 2013-03-27*/
  { 109, "EUI64" }, /* an EUI-64 address [RFC7043] EUI64/eui64-completed-template 2013-03-27*/
  { 249, "TKEY" }, /* Transaction Key [RFC2930] */
  { 250, "TSIG" }, /* Transaction Signature [RFC8945] */
  { 251, "IXFR" }, /* incremental transfer [RFC1995] */
  { 252, "AXFR" }, /* transfer of an entire zone [RFC1035][RFC5936] */
  { 253, "MAILB" }, /* mailbox-related RRs (MB, MG or MR) [RFC1035] */
  { 254, "MAILA" }, /* mail agent RRs (OBSOLETE - see MX) [RFC1035] */
  { 255, "ANY" }, /* A request for some or all records the server has available [RFC1035][RFC6895][RFC8482] */
  { 256, "URI" }, /* URI [RFC7553] URI/uri-completed-template 2011-02-22*/
  { 257, "CAA" }, /* Certification Authority Restriction [RFC8659] CAA/caa-completed-template 2011-04-07*/
  { 258, "AVC" }, /* Application Visibility and Control [Wolfgang_Riedel] AVC/avc-completed-template 2016-02-26*/
  { 259, "DOA" }, /* Digital Object Architecture [draft-durand-doa-over-dns] DOA/doa-completed-template 2017-08-30*/
  { 260, "AMTRELAY" }, /* Automatic Multicast Tunneling Relay [RFC8777] AMTRELAY/amtrelay-completed-template 2019-02-06*/
  { 32768,  "TA" }, /* DNSSEC Trust Authorities [Sam_Weiler][http://cameo.library.cmu.edu/][ Deploying DNSSEC Without a Signed Root. Technical Report 1999-19, Information Networking Institute, Carnegie Mellon University, April 2004.] 2005-12-13*/
  { 32769,  "DLV" }, /* DNSSEC Lookaside Validation (OBSOLETE) [RFC8749][RFC4431] */
};

static void cache_free(struct crec *crecp);
static void cache_unlink(struct crec *crecp);
static void cache_link(struct crec *crecp);
static void rehash(int size);
static void cache_hash(struct crec *crecp);

void next_uid(struct crec *crecp)
{
  static unsigned int uid = 0;

  if (crecp->uid == UID_NONE)
    {
      uid++;
  
      /* uid == 0 used to indicate CNAME to interface name. */
      if (uid == UID_NONE)
	uid++;
      
      crecp->uid = uid;
    }
}

void cache_init(void)
{
  struct crec *crecp;
  int i;
 
  bignames_left = daemon->cachesize/10;
  
  if (daemon->cachesize > 0)
    {
      crecp = safe_malloc(daemon->cachesize*sizeof(struct crec));
      
      for (i=0; i < daemon->cachesize; i++, crecp++)
	{
	  cache_link(crecp);
	  crecp->flags = 0;
	  crecp->uid = UID_NONE;
	}
    }
  
  /* create initial hash table*/
  rehash(daemon->cachesize);
}

/* In most cases, we create the hash table once here by calling this with (hash_table == NULL)
   but if the hosts file(s) are big (some people have 50000 ad-block entries), the table
   will be much too small, so the hosts reading code calls rehash every 1000 addresses, to
   expand the table. */
static void rehash(int size)
{
  struct crec **new, **old, *p, *tmp;
  int i, new_size, old_size;

  /* hash_size is a power of two. */
  for (new_size = 64; new_size < size/10; new_size = new_size << 1);
  
  /* must succeed in getting first instance, failure later is non-fatal */
  if (!hash_table)
    new = safe_malloc(new_size * sizeof(struct crec *));
  else if (new_size <= hash_size || !(new = whine_malloc(new_size * sizeof(struct crec *))))
    return;

  for (i = 0; i < new_size; i++)
    new[i] = NULL;

  old = hash_table;
  old_size = hash_size;
  hash_table = new;
  hash_size = new_size;
  
  if (old)
    {
      for (i = 0; i < old_size; i++)
	for (p = old[i]; p ; p = tmp)
	  {
	    tmp = p->hash_next;
	    cache_hash(p);
	  }
      free(old);
    }
}
  
static struct crec **hash_bucket(char *name)
{
  unsigned int c, val = 017465; /* Barker code - minimum self-correlation in cyclic shift */
  const unsigned char *mix_tab = (const unsigned char*)typestr; 

  while((c = (unsigned char) *name++))
    {
      /* don't use tolower and friends here - they may be messed up by LOCALE */
      if (c >= 'A' && c <= 'Z')
	c += 'a' - 'A';
      val = ((val << 7) | (val >> (32 - 7))) + (mix_tab[(val + c) & 0x3F] ^ c);
    } 
  
  /* hash_size is a power of two */
  return hash_table + ((val ^ (val >> 16)) & (hash_size - 1));
}

static void cache_hash(struct crec *crecp)
{
  /* maintain an invariant that all entries with F_REVERSE set
     are at the start of the hash-chain  and all non-reverse
     immortal entries are at the end of the hash-chain.
     This allows reverse searches and garbage collection to be optimised */

  char *name = cache_get_name(crecp);
  struct crec **up = hash_bucket(name);
  unsigned int flags = crecp->flags & (F_IMMORTAL | F_REVERSE);
  
  if (!(flags & F_REVERSE))
    {
      while (*up && ((*up)->flags & F_REVERSE))
	up = &((*up)->hash_next); 
      
      if (flags & F_IMMORTAL)
	while (*up && !((*up)->flags & F_IMMORTAL))
	  up = &((*up)->hash_next);
    }

  /* Preserve order when inserting the same name multiple times.
     Do not mess up the flag invariants. */
  while (*up &&
	 hostname_isequal(cache_get_name(*up), name) &&
	 flags == ((*up)->flags & (F_IMMORTAL | F_REVERSE)))
    up = &((*up)->hash_next);
  
  crecp->hash_next = *up;
  *up = crecp;
}

static void cache_blockdata_free(struct crec *crecp)
{
  if (!(crecp->flags & F_NEG))
    {
      if (crecp->flags & F_SRV)
	blockdata_free(crecp->addr.srv.target);
#ifdef HAVE_DNSSEC
      else if (crecp->flags & F_DNSKEY)
	blockdata_free(crecp->addr.key.keydata);
      else if (crecp->flags & F_DS)
	blockdata_free(crecp->addr.ds.keydata);
#endif
    }
}

static void cache_free(struct crec *crecp)
{
  crecp->flags &= ~F_FORWARD;
  crecp->flags &= ~F_REVERSE;
  crecp->uid = UID_NONE; /* invalidate CNAMES pointing to this. */

  if (cache_tail)
    cache_tail->next = crecp;
  else
    cache_head = crecp;
  crecp->prev = cache_tail;
  crecp->next = NULL;
  cache_tail = crecp;
  
  /* retrieve big name for further use. */
  if (crecp->flags & F_BIGNAME)
    {
      crecp->name.bname->next = big_free;
      big_free = crecp->name.bname;
      crecp->flags &= ~F_BIGNAME;
    }

  cache_blockdata_free(crecp);
}    

/* insert a new cache entry at the head of the list (youngest entry) */
static void cache_link(struct crec *crecp)
{
  if (cache_head) /* check needed for init code */
    cache_head->prev = crecp;
  crecp->next = cache_head;
  crecp->prev = NULL;
  cache_head = crecp;
  if (!cache_tail)
    cache_tail = crecp;
}

/* remove an arbitrary cache entry for promotion */ 
static void cache_unlink (struct crec *crecp)
{
  if (crecp->prev)
    crecp->prev->next = crecp->next;
  else
    cache_head = crecp->next;

  if (crecp->next)
    crecp->next->prev = crecp->prev;
  else
    cache_tail = crecp->prev;
}

char *cache_get_name(struct crec *crecp)
{
  if (crecp->flags & F_BIGNAME)
    return crecp->name.bname->name;
  else if (crecp->flags & F_NAMEP) 
    return crecp->name.namep;
  
  return crecp->name.sname;
}

char *cache_get_cname_target(struct crec *crecp)
{
  if (crecp->addr.cname.is_name_ptr)
     return crecp->addr.cname.target.name;
  else
    return cache_get_name(crecp->addr.cname.target.cache);
}



struct crec *cache_enumerate(int init)
{
  static int bucket;
  static struct crec *cache;

  if (init)
    {
      bucket = 0;
      cache = NULL;
    }
  else if (cache && cache->hash_next)
    cache = cache->hash_next;
  else
    {
       cache = NULL; 
       while (bucket < hash_size)
	 if ((cache = hash_table[bucket++]))
	   break;
    }
  
  return cache;
}

static int is_outdated_cname_pointer(struct crec *crecp)
{
  if (!(crecp->flags & F_CNAME) || crecp->addr.cname.is_name_ptr)
    return 0;
  
  /* NB. record may be reused as DS or DNSKEY, where uid is 
     overloaded for something completely different */
  if (crecp->addr.cname.target.cache && 
      !(crecp->addr.cname.target.cache->flags & (F_DNSKEY | F_DS)) &&
      crecp->addr.cname.uid == crecp->addr.cname.target.cache->uid)
    return 0;
  
  return 1;
}

static int is_expired(time_t now, struct crec *crecp)
{
  /* Don't dump expired entries if they are within the accepted timeout range.
     The cache becomes approx. LRU. Never use expired DS or DNSKEY entries.
     Possible values for daemon->cache_max_expiry:
      -1  == serve cached content regardless how long ago it expired
       0  == the option is disabled, expired content isn't served
      <n> == serve cached content only if it expire less than <n> seconds
             ago (where n is a positive integer) */
  if (daemon->cache_max_expiry != 0 &&
      (daemon->cache_max_expiry == -1 ||
       difftime(now, crecp->ttd) < daemon->cache_max_expiry) &&
      !(crecp->flags & (F_DS | F_DNSKEY)))
    return 0;

  if (crecp->flags & F_IMMORTAL)
    return 0;

  if (difftime(now, crecp->ttd) < 0)
    return 0;
  
  return 1;
}

/* Remove entries with a given UID from the cache */
unsigned int cache_remove_uid(const unsigned int uid)
{
  int i;
  unsigned int removed = 0;
  struct crec *crecp, **up;

  for (i = 0; i < hash_size; i++)
    for (crecp = hash_table[i], up = &hash_table[i]; crecp; crecp = crecp->hash_next)
      if ((crecp->flags & (F_HOSTS | F_DHCP | F_CONFIG)) && crecp->uid == uid)
	{
	  *up = crecp->hash_next;
	  free(crecp);
	  removed++;
	}
      else
	up = &crecp->hash_next;
  
  return removed;
}

static struct crec *cache_scan_free(char *name, union all_addr *addr, unsigned short class, time_t now,
				    unsigned int flags, struct crec **target_crec, unsigned int *target_uid)
{
  /* Scan and remove old entries.
     If (flags & F_FORWARD) then remove any forward entries for name and any expired
     entries but only in the same hash bucket as name.
     If (flags & F_REVERSE) then remove any reverse entries for addr and any expired
     entries in the whole cache.
     If (flags == 0) remove any expired entries in the whole cache. 

     In the flags & F_FORWARD case, the return code is valid, and returns a non-NULL pointer
     to a cache entry if the name exists in the cache as a HOSTS or DHCP entry (these are never deleted)

     We take advantage of the fact that hash chains have stuff in the order <reverse>,<other>,<immortal>
     so that when we hit an entry which isn't reverse and is immortal, we're done. 

     If we free a crec which is a CNAME target, return the entry and uid in target_crec and target_uid.
     This entry will get re-used with the same name, to preserve CNAMEs. */
 
  struct crec *crecp, **up;

  (void)class;
  
  if (flags & F_FORWARD)
    {
      for (up = hash_bucket(name), crecp = *up; crecp; crecp = crecp->hash_next)
	{
	  if ((crecp->flags & F_FORWARD) && hostname_isequal(cache_get_name(crecp), name))
	    {
	      /* Don't delete DNSSEC in favour of a CNAME, they can co-exist */
	      if ((flags & crecp->flags & (F_IPV4 | F_IPV6 | F_SRV | F_NXDOMAIN)) || 
		  (((crecp->flags | flags) & F_CNAME) && !(crecp->flags & (F_DNSKEY | F_DS))))
		{
		  if (crecp->flags & (F_HOSTS | F_DHCP | F_CONFIG))
		    return crecp;
		  *up = crecp->hash_next;
		  /* If this record is for the name we're inserting and is the target
		     of a CNAME record. Make the new record for the same name, in the same
		     crec, with the same uid to avoid breaking the existing CNAME. */
		  if (crecp->uid != UID_NONE)
		    {
		      if (target_crec)
			*target_crec = crecp;
		      if (target_uid)
			*target_uid = crecp->uid;
		    }
		  cache_unlink(crecp);
		  cache_free(crecp);
		  continue;
		}
	      
#ifdef HAVE_DNSSEC
	      /* Deletion has to be class-sensitive for DS and DNSKEY */
	      if ((flags & crecp->flags & (F_DNSKEY | F_DS)) && crecp->uid == class)
		{
		  if (crecp->flags & F_CONFIG)
		    return crecp;
		  *up = crecp->hash_next;
		  cache_unlink(crecp);
		  cache_free(crecp);
		  continue;
		}
#endif
	    }

	  if (is_expired(now, crecp) || is_outdated_cname_pointer(crecp))
	    { 
	      *up = crecp->hash_next;
	      if (!(crecp->flags & (F_HOSTS | F_DHCP | F_CONFIG)))
		{
		  cache_unlink(crecp);
		  cache_free(crecp);
		}
	      continue;
	    } 
	  
	  up = &crecp->hash_next;
	}
    }
  else
    {
      int i;
      int addrlen = (flags & F_IPV6) ? IN6ADDRSZ : INADDRSZ;

      for (i = 0; i < hash_size; i++)
	for (crecp = hash_table[i], up = &hash_table[i]; 
	     crecp && ((crecp->flags & F_REVERSE) || !(crecp->flags & F_IMMORTAL));
	     crecp = crecp->hash_next)
	  if (is_expired(now, crecp))
	    {
	      *up = crecp->hash_next;
	      if (!(crecp->flags & (F_HOSTS | F_DHCP | F_CONFIG)))
		{ 
		  cache_unlink(crecp);
		  cache_free(crecp);
		}
	    }
	  else if (!(crecp->flags & (F_HOSTS | F_DHCP | F_CONFIG)) &&
		   (flags & crecp->flags & F_REVERSE) && 
		   (flags & crecp->flags & (F_IPV4 | F_IPV6)) &&
		   addr && memcmp(&crecp->addr, addr, addrlen) == 0)
	    {
	      *up = crecp->hash_next;
	      cache_unlink(crecp);
	      cache_free(crecp);
	    }
	  else
	    up = &crecp->hash_next;
    }
  
  return NULL;
}

/* Note: The normal calling sequence is
   cache_start_insert
   cache_insert * n
   cache_end_insert

   but an abort can cause the cache_end_insert to be missed 
   in which can the next cache_start_insert cleans things up. */

void cache_start_insert(void)
{
  /* Free any entries which didn't get committed during the last
     insert due to error.
  */
  while (new_chain)
    {
      struct crec *tmp = new_chain->next;
      cache_free(new_chain);
      new_chain = tmp;
    }
  new_chain = NULL;
  insert_error = 0;
}

struct crec *cache_insert(char *name, union all_addr *addr, unsigned short class,
			  time_t now,  unsigned long ttl, unsigned int flags)
{
#ifdef HAVE_DNSSEC
  if (flags & (F_DNSKEY | F_DS)) 
    {
      /* The DNSSEC validation process works by getting needed records into the
	 cache, then retrying the validation until they are all in place.
	 This can be messed up by very short TTLs, and _really_ messed up by
	 zero TTLs, so we force the TTL to be at least long enough to do a validation.
	 Ideally, we should use some kind of reference counting so that records are
	 locked until the validation that asked for them is complete, but this
	 is much easier, and just as effective. */
      if (ttl < DNSSEC_MIN_TTL)
	ttl = DNSSEC_MIN_TTL;
    }
  else
#endif
    {
      if (daemon->max_cache_ttl != 0 && daemon->max_cache_ttl < ttl)
	ttl = daemon->max_cache_ttl;
      if (daemon->min_cache_ttl != 0 && daemon->min_cache_ttl > ttl)
	ttl = daemon->min_cache_ttl;
    }	
  
  return really_insert(name, addr, class, now, ttl, flags);
}


static struct crec *really_insert(char *name, union all_addr *addr, unsigned short class,
				  time_t now,  unsigned long ttl, unsigned int flags)
{
  struct crec *new, *target_crec = NULL;
  union bigname *big_name = NULL;
  int freed_all = (flags & F_REVERSE);
  struct crec *free_avail = NULL;
  unsigned int target_uid;
  
  /* if previous insertion failed give up now. */
  if (insert_error)
    return NULL;

  /* we don't cache zero-TTL records. */
  if (ttl == 0)
    {
      insert_error = 1;
      return NULL;
    }
  
  /* First remove any expired entries and entries for the name/address we
     are currently inserting. */
  if ((new = cache_scan_free(name, addr, class, now, flags, &target_crec, &target_uid)))
    {
      /* We're trying to insert a record over one from 
	 /etc/hosts or DHCP, or other config. If the 
	 existing record is for an A or AAAA or CNAME and
	 the record we're trying to insert is the same, 
	 just drop the insert, but don't error the whole process. */
      if ((flags & (F_IPV4 | F_IPV6)) && (flags & F_FORWARD) && addr)
	{
	  if ((flags & F_IPV4) && (new->flags & F_IPV4) &&
	      new->addr.addr4.s_addr == addr->addr4.s_addr)
	    return new;
	  else if ((flags & F_IPV6) && (new->flags & F_IPV6) &&
		   IN6_ARE_ADDR_EQUAL(&new->addr.addr6, &addr->addr6))
	    return new;
	}

      insert_error = 1;
      return NULL;
    }
  
  /* Now get a cache entry from the end of the LRU list */
  if (!target_crec)
    while (1) {
      if (!(new = cache_tail)) /* no entries left - cache is too small, bail */
	{
	  insert_error = 1;
	  return NULL;
	}
      
      /* Free entry at end of LRU list, use it. */
      if (!(new->flags & (F_FORWARD | F_REVERSE)))
	break; 

      /* End of LRU list is still in use: if we didn't scan all the hash
	 chains for expired entries do that now. If we already tried that
	 then it's time to start spilling things. */
      
      /* If free_avail set, we believe that an entry has been freed.
	 Bugs have been known to make this not true, resulting in
	 a tight loop here. If that happens, abandon the
	 insert. Once in this state, all inserts will probably fail. */
      if (free_avail)
	{
	  my_syslog(LOG_ERR, _("Internal error in cache."));
	  /* Log the entry we tried to delete. */
	  dump_cache_entry(free_avail, now);
	  insert_error = 1;
	  return NULL;
	}
      
      if (freed_all)
	{
	  /* For DNSSEC records, uid holds class. */
	  free_avail = new; /* Must be free space now. */
	  
	  /* condition valid when stale-caching */
	  if (difftime(now, new->ttd) < 0)
	    daemon->metrics[METRIC_DNS_CACHE_LIVE_FREED]++;
	  
	  cache_scan_free(cache_get_name(new), &new->addr, new->uid, now, new->flags, NULL, NULL); 
	}
      else
	{
	  cache_scan_free(NULL, NULL, class, now, 0, NULL, NULL);
	  freed_all = 1;
	}
    }
      
  /* Check if we need to and can allocate extra memory for a long name.
     If that fails, give up now, always succeed for DNSSEC records. */
  if (name && (strlen(name) > SMALLDNAME-1))
    {
      if (big_free)
	{ 
	  big_name = big_free;
	  big_free = big_free->next;
	}
      else if ((bignames_left == 0 && !(flags & (F_DS | F_DNSKEY))) ||
	       !(big_name = (union bigname *)whine_malloc(sizeof(union bigname))))
	{
	  insert_error = 1;
	  return NULL;
	}
      else if (bignames_left != 0)
	bignames_left--;
      
    }

  /* If we freed a cache entry for our name which was a CNAME target, use that.
     and preserve the uid, so that existing CNAMES are not broken. */
  if (target_crec)
    {
      new = target_crec;
      new->uid = target_uid;
    }
  
  /* Got the rest: finally grab entry. */
  cache_unlink(new);
  
  new->flags = flags;
  if (big_name)
    {
      new->name.bname = big_name;
      new->flags |= F_BIGNAME;
    }

  if (name)
    strcpy(cache_get_name(new), name);
  else
    *cache_get_name(new) = 0;

#ifdef HAVE_DNSSEC
  if (flags & (F_DS | F_DNSKEY))
    new->uid = class;
#endif

  if (addr)
    new->addr = *addr;	

  new->ttd = now + (time_t)ttl;
  new->next = new_chain;
  new_chain = new;
  
  return new;
}

/* after end of insertion, commit the new entries */
void cache_end_insert(void)
{
  if (insert_error)
    return;
  
  while (new_chain)
    { 
      struct crec *tmp = new_chain->next;
      /* drop CNAMEs which didn't find a target. */
      if (is_outdated_cname_pointer(new_chain))
	cache_free(new_chain);
      else
	{
	  cache_hash(new_chain);
	  cache_link(new_chain);
	  daemon->metrics[METRIC_DNS_CACHE_INSERTED]++;

	  /* If we're a child process, send this cache entry up the pipe to the master.
	     The marshalling process is rather nasty. */
	  if (daemon->pipe_to_parent != -1)
	    {
	      char *name = cache_get_name(new_chain);
	      ssize_t m = strlen(name);
	      unsigned int flags = new_chain->flags;
#ifdef HAVE_DNSSEC
	      u16 class = new_chain->uid;
#endif
	      
	      read_write(daemon->pipe_to_parent, (unsigned char *)&m, sizeof(m), 0);
	      read_write(daemon->pipe_to_parent, (unsigned char *)name, m, 0);
	      read_write(daemon->pipe_to_parent, (unsigned char *)&new_chain->ttd, sizeof(new_chain->ttd), 0);
	      read_write(daemon->pipe_to_parent, (unsigned  char *)&flags, sizeof(flags), 0);

	      if (flags & (F_IPV4 | F_IPV6 | F_DNSKEY | F_DS | F_SRV))
		read_write(daemon->pipe_to_parent, (unsigned char *)&new_chain->addr, sizeof(new_chain->addr), 0);
	      if (flags & F_SRV)
		{
		  /* A negative SRV entry is possible and has no data, obviously. */
		  if (!(flags & F_NEG))
		    blockdata_write(new_chain->addr.srv.target, new_chain->addr.srv.targetlen, daemon->pipe_to_parent);
		}
#ifdef HAVE_DNSSEC
	      if (flags & F_DNSKEY)
		{
		  read_write(daemon->pipe_to_parent, (unsigned char *)&class, sizeof(class), 0);
		  blockdata_write(new_chain->addr.key.keydata, new_chain->addr.key.keylen, daemon->pipe_to_parent);
		}
	      else if (flags & F_DS)
		{
		  read_write(daemon->pipe_to_parent, (unsigned char *)&class, sizeof(class), 0);
		  /* A negative DS entry is possible and has no data, obviously. */
		  if (!(flags & F_NEG))
		    blockdata_write(new_chain->addr.ds.keydata, new_chain->addr.ds.keylen, daemon->pipe_to_parent);
		}
#endif
	    }
	}
      
      new_chain = tmp;
    }

  /* signal end of cache insert in master process */
  if (daemon->pipe_to_parent != -1)
    {
      ssize_t m = -1;
      read_write(daemon->pipe_to_parent, (unsigned char *)&m, sizeof(m), 0);
    }
      
  new_chain = NULL;
}


/* A marshalled cache entry arrives on fd, read, unmarshall and insert into cache of master process. */
int cache_recv_insert(time_t now, int fd)
{
  ssize_t m;
  union all_addr addr;
  unsigned long ttl;
  time_t ttd;
  unsigned int flags;
  struct crec *crecp = NULL;
  
  cache_start_insert();
  
  while(1)
    {
 
      if (!read_write(fd, (unsigned char *)&m, sizeof(m), 1))
	return 0;
      
      if (m == -1)
	{
	  cache_end_insert();
	  return 1;
	}

      if (!read_write(fd, (unsigned char *)daemon->namebuff, m, 1) ||
	  !read_write(fd, (unsigned char *)&ttd, sizeof(ttd), 1) ||
	  !read_write(fd, (unsigned char *)&flags, sizeof(flags), 1))
	return 0;

      daemon->namebuff[m] = 0;

      ttl = difftime(ttd, now);
      
      if (flags & (F_IPV4 | F_IPV6 | F_DNSKEY | F_DS | F_SRV))
	{
	  unsigned short class = C_IN;

	  if (!read_write(fd, (unsigned char *)&addr, sizeof(addr), 1))
	    return 0;

	  if ((flags & F_SRV) && !(flags & F_NEG) && !(addr.srv.target = blockdata_read(fd, addr.srv.targetlen)))
	    return 0;
	
#ifdef HAVE_DNSSEC
	   if (flags & F_DNSKEY)
	     {
	       if (!read_write(fd, (unsigned char *)&class, sizeof(class), 1) ||
		   !(addr.key.keydata = blockdata_read(fd, addr.key.keylen)))
		 return 0;
	     }
	   else  if (flags & F_DS)
	     {
	        if (!read_write(fd, (unsigned char *)&class, sizeof(class), 1) ||
		    (!(flags & F_NEG) && !(addr.key.keydata = blockdata_read(fd, addr.key.keylen))))
		  return 0;
	     }
#endif
	       
	  crecp = really_insert(daemon->namebuff, &addr, class, now, ttl, flags);
	}
      else if (flags & F_CNAME)
	{
	  struct crec *newc = really_insert(daemon->namebuff, NULL, C_IN, now, ttl, flags);
	  /* This relies on the fact that the target of a CNAME immediately precedes
	     it because of the order of extraction in extract_addresses, and
	     the order reversal on the new_chain. */
	  if (newc)
	    {
	       newc->addr.cname.is_name_ptr = 0;
	       
	       if (!crecp)
		 newc->addr.cname.target.cache = NULL;
	       else
		{
		  next_uid(crecp);
		  newc->addr.cname.target.cache = crecp;
		  newc->addr.cname.uid = crecp->uid;
		}
	    }
	}
    }
}
	
int cache_find_non_terminal(char *name, time_t now)
{
  struct crec *crecp;

  for (crecp = *hash_bucket(name); crecp; crecp = crecp->hash_next)
    if (!is_outdated_cname_pointer(crecp) &&
	!is_expired(now, crecp) &&
	(crecp->flags & F_FORWARD) &&
	!(crecp->flags & F_NXDOMAIN) && 
	hostname_isequal(name, cache_get_name(crecp)))
      return 1;

  return 0;
}

struct crec *cache_find_by_name(struct crec *crecp, char *name, time_t now, unsigned int prot)
{
  struct crec *ans;
  int no_rr = (prot & F_NO_RR) || option_bool(OPT_NORR);

  prot &= ~F_NO_RR;
  
  if (crecp) /* iterating */
    ans = crecp->next;
  else
    {
      /* first search, look for relevant entries and push to top of list
	 also free anything which has expired */
      struct crec *next, **up, **insert = NULL, **chainp = &ans;
      unsigned int ins_flags = 0;
      
      for (up = hash_bucket(name), crecp = *up; crecp; crecp = next)
	{
	  next = crecp->hash_next;
	  
	  if (!is_expired(now, crecp) && !is_outdated_cname_pointer(crecp))
	    {
	      if ((crecp->flags & F_FORWARD) && 
		  (crecp->flags & prot) &&
		  hostname_isequal(cache_get_name(crecp), name))
		{
		  if (crecp->flags & (F_HOSTS | F_DHCP | F_CONFIG))
		    {
		      *chainp = crecp;
		      chainp = &crecp->next;
		    }
		  else
		    {
		      cache_unlink(crecp);
		      cache_link(crecp);
		    }
	      	      
		  /* Move all but the first entry up the hash chain
		     this implements round-robin. 
		     Make sure that re-ordering doesn't break the hash-chain
		     order invariants. 
		  */
		  if (insert && (crecp->flags & (F_REVERSE | F_IMMORTAL)) == ins_flags)
		    {
		      *up = crecp->hash_next;
		      crecp->hash_next = *insert;
		      *insert = crecp;
		      insert = &crecp->hash_next;
		    }
		  else
		    {
		      if (!insert && !no_rr)
			{
			  insert = up;
			  ins_flags = crecp->flags & (F_REVERSE | F_IMMORTAL);
			}
		      up = &crecp->hash_next; 
		    }
		}
	      else
		/* case : not expired, incorrect entry. */
		up = &crecp->hash_next; 
	    }
	  else
	    {
	      /* expired entry, free it */
	      *up = crecp->hash_next;
	      if (!(crecp->flags & (F_HOSTS | F_DHCP | F_CONFIG)))
		{ 
		  cache_unlink(crecp);
		  cache_free(crecp);
		}
	    }
	}
	  
      *chainp = cache_head;
    }

  if (ans && 
      (ans->flags & F_FORWARD) &&
      (ans->flags & prot) &&     
      hostname_isequal(cache_get_name(ans), name))
    return ans;
  
  return NULL;
}

struct crec *cache_find_by_addr(struct crec *crecp, union all_addr *addr, 
				time_t now, unsigned int prot)
{
  struct crec *ans;
  int addrlen = (prot == F_IPV6) ? IN6ADDRSZ : INADDRSZ;
  
  if (crecp) /* iterating */
    ans = crecp->next;
  else
    {  
      /* first search, look for relevant entries and push to top of list
	 also free anything which has expired. All the reverse entries are at the
	 start of the hash chain, so we can give up when we find the first 
	 non-REVERSE one.  */
       int i;
       struct crec **up, **chainp = &ans;
       
       for (i=0; i<hash_size; i++)
	 for (crecp = hash_table[i], up = &hash_table[i]; 
	      crecp && (crecp->flags & F_REVERSE);
	      crecp = crecp->hash_next)
	   if (!is_expired(now, crecp))
	     {      
	       if ((crecp->flags & prot) &&
		   memcmp(&crecp->addr, addr, addrlen) == 0)
		 {	    
		   if (crecp->flags & (F_HOSTS | F_DHCP | F_CONFIG))
		     {
		       *chainp = crecp;
		       chainp = &crecp->next;
		     }
		   else
		     {
		       cache_unlink(crecp);
		       cache_link(crecp);
		     }
		 }
	       up = &crecp->hash_next;
	     }
	   else
	     {
	       *up = crecp->hash_next;
	       if (!(crecp->flags & (F_HOSTS | F_DHCP | F_CONFIG)))
		 {
		   cache_unlink(crecp);
		   cache_free(crecp);
		 }
	     }
       
       *chainp = cache_head;
    }
  
  if (ans && 
      (ans->flags & F_REVERSE) &&
      (ans->flags & prot) &&
      memcmp(&ans->addr, addr, addrlen) == 0)
    return ans;
  
  return NULL;
}

static void add_hosts_entry(struct crec *cache, union all_addr *addr, int addrlen, 
			    unsigned int index, struct crec **rhash, int hashsz)
{
  int i;
  unsigned int j; 
  struct crec *lookup = NULL;

  /* Remove duplicates in hosts files. */
  while ((lookup = cache_find_by_name(lookup, cache_get_name(cache), 0, cache->flags & (F_IPV4 | F_IPV6))))
    if ((lookup->flags & F_HOSTS) && memcmp(&lookup->addr, addr, addrlen) == 0)
      {
	free(cache);
	return;
      }
    
  /* Ensure there is only one address -> name mapping (first one trumps) 
     We do this by steam here, The entries are kept in hash chains, linked
     by ->next (which is unused at this point) held in hash buckets in
     the array rhash, hashed on address. Note that rhash and the values
     in ->next are only valid  whilst reading hosts files: the buckets are
     then freed, and the ->next pointer used for other things. 
     Only insert each unique address once into this hashing structure.

     This complexity avoids O(n^2) divergent CPU use whilst reading
     large (10000 entry) hosts files. 

     Note that we only do this process when bulk-reading hosts files, 
     for incremental reads, rhash is NULL, and we use cache lookups
     instead.
  */
  
  if (rhash)
    {
      /* hash address */
      for (j = 0, i = 0; i < addrlen; i++)
	j = (j*2 +((unsigned char *)addr)[i]) % hashsz;
      
      for (lookup = rhash[j]; lookup; lookup = lookup->next)
	if ((lookup->flags & cache->flags & (F_IPV4 | F_IPV6)) &&
	    memcmp(&lookup->addr, addr, addrlen) == 0)
	  {
	    cache->flags &= ~F_REVERSE;
	    break;
	  }
      
      /* maintain address hash chain, insert new unique address */
      if (!lookup)
	{
	  cache->next = rhash[j];
	  rhash[j] = cache;
	}
    }
  else
    {
      /* incremental read, lookup in cache */
      lookup = cache_find_by_addr(NULL, addr, 0, cache->flags & (F_IPV4 | F_IPV6));
      if (lookup && lookup->flags & F_HOSTS)
	cache->flags &= ~F_REVERSE;
    }

  cache->uid = index;
  memcpy(&cache->addr, addr, addrlen);  
  cache_hash(cache);
  make_non_terminals(cache);
}

static int eatspace(FILE *f)
{
  int c, nl = 0;

  while (1)
    {
      if ((c = getc(f)) == '#')
	while (c != '\n' && c != EOF)
	  c = getc(f);
      
      if (c == EOF)
	return 1;

      if (!isspace(c))
	{
	  ungetc(c, f);
	  return nl;
	}

      if (c == '\n')
	nl++;
    }
}
	 
static int gettok(FILE *f, char *token)
{
  int c, count = 0;
 
  while (1)
    {
      if ((c = getc(f)) == EOF)
	return (count == 0) ? -1 : 1;

      if (isspace(c) || c == '#')
	{
	  ungetc(c, f);
	  return eatspace(f);
	}
      
      if (count < (MAXDNAME - 1))
	{
	  token[count++] = c;
	  token[count] = 0;
	}
    }
}

int read_hostsfile(char *filename, unsigned int index, int cache_size, struct crec **rhash, int hashsz)
{  
  FILE *f = fopen(filename, "r");
  char *token = daemon->namebuff, *domain_suffix = NULL;
  int names_done = 0, name_count = cache_size, lineno = 1;
  unsigned int flags = 0;
  union all_addr addr;
  int atnl, addrlen = 0;

  if (!f)
    {
      my_syslog(LOG_ERR, _("failed to load names from %s: %s"), filename, strerror(errno));
      return cache_size;
    }
  
  lineno += eatspace(f);
  
  while ((atnl = gettok(f, token)) != -1)
    {
      if (inet_pton(AF_INET, token, &addr) > 0)
	{
	  flags = F_HOSTS | F_IMMORTAL | F_FORWARD | F_REVERSE | F_IPV4;
	  addrlen = INADDRSZ;
	  domain_suffix = get_domain(addr.addr4);
	}
      else if (inet_pton(AF_INET6, token, &addr) > 0)
	{
	  flags = F_HOSTS | F_IMMORTAL | F_FORWARD | F_REVERSE | F_IPV6;
	  addrlen = IN6ADDRSZ;
	  domain_suffix = get_domain6(&addr.addr6);
	}
      else
	{
	  my_syslog(LOG_ERR, _("bad address at %s line %d"), filename, lineno); 
	  while (atnl == 0)
	    atnl = gettok(f, token);
	  lineno += atnl;
	  continue;
	}
      
      /* rehash every 1000 names. */
      if (rhash && ((name_count - cache_size) > 1000))
	{
	  rehash(name_count);
	  cache_size = name_count;
	} 
      
      while (atnl == 0)
	{
	  struct crec *cache;
	  int fqdn, nomem;
	  char *canon;
	  
	  if ((atnl = gettok(f, token)) == -1)
	    break;

	  fqdn = !!strchr(token, '.');

	  if ((canon = canonicalise(token, &nomem)))
	    {
	      /* If set, add a version of the name with a default domain appended */
	      if (option_bool(OPT_EXPAND) && domain_suffix && !fqdn && 
		  (cache = whine_malloc(SIZEOF_BARE_CREC + strlen(canon) + 2 + strlen(domain_suffix))))
		{
		  strcpy(cache->name.sname, canon);
		  strcat(cache->name.sname, ".");
		  strcat(cache->name.sname, domain_suffix);
		  cache->flags = flags;
		  cache->ttd = daemon->local_ttl;
		  add_hosts_entry(cache, &addr, addrlen, index, rhash, hashsz);
		  name_count++;
		  names_done++;
		}
	      if ((cache = whine_malloc(SIZEOF_BARE_CREC + strlen(canon) + 1)))
		{
		  strcpy(cache->name.sname, canon);
		  cache->flags = flags;
		  cache->ttd = daemon->local_ttl;
		  add_hosts_entry(cache, &addr, addrlen, index, rhash, hashsz);
		  name_count++;
		  names_done++;
		}
	      free(canon);
	      
	    }
	  else if (!nomem)
	    my_syslog(LOG_ERR, _("bad name at %s line %d"), filename, lineno); 
	}

      lineno += atnl;
    } 

  fclose(f);
  
  if (rhash)
    rehash(name_count); 
  
  my_syslog(LOG_INFO, _("read %s - %d names"), filename, names_done);
  
  return name_count;
}
	    
void cache_reload(void)
{
  struct crec *cache, **up, *tmp;
  int revhashsz, i, total_size = daemon->cachesize;
  struct hostsfile *ah;
  struct host_record *hr;
  struct name_list *nl;
  struct cname *a;
  struct crec lrec;
  struct mx_srv_record *mx;
  struct txt_record *txt;
  struct interface_name *intr;
  struct ptr_record *ptr;
  struct naptr *naptr;
#ifdef HAVE_DNSSEC
  struct ds_config *ds;
#endif

  daemon->metrics[METRIC_DNS_CACHE_INSERTED] = 0;
  daemon->metrics[METRIC_DNS_CACHE_LIVE_FREED] = 0;
  
  for (i=0; i<hash_size; i++)
    for (cache = hash_table[i], up = &hash_table[i]; cache; cache = tmp)
      {
	cache_blockdata_free(cache);

	tmp = cache->hash_next;
	if (cache->flags & (F_HOSTS | F_CONFIG))
	  {
	    *up = cache->hash_next;
	    free(cache);
	  }
	else if (!(cache->flags & F_DHCP))
	  {
	    *up = cache->hash_next;
	    if (cache->flags & F_BIGNAME)
	      {
		cache->name.bname->next = big_free;
		big_free = cache->name.bname;
	      }
	    cache->flags = 0;
	  }
	else
	  up = &cache->hash_next;
      }
  
  /* Add locally-configured CNAMEs to the cache */
  for (a = daemon->cnames; a; a = a->next)
    if (a->alias[1] != '*' &&
	((cache = whine_malloc(SIZEOF_POINTER_CREC))))
      {
	cache->flags = F_FORWARD | F_NAMEP | F_CNAME | F_IMMORTAL | F_CONFIG;
	cache->ttd = a->ttl;
	cache->name.namep = a->alias;
	cache->addr.cname.target.name = a->target;
	cache->addr.cname.is_name_ptr = 1;
	cache->uid = UID_NONE;
	cache_hash(cache);
	make_non_terminals(cache);
      }
  
#ifdef HAVE_DNSSEC
  for (ds = daemon->ds; ds; ds = ds->next)
    if ((cache = whine_malloc(SIZEOF_POINTER_CREC)) &&
	(cache->addr.ds.keydata = blockdata_alloc(ds->digest, ds->digestlen)))
      {
	cache->flags = F_FORWARD | F_IMMORTAL | F_DS | F_CONFIG | F_NAMEP;
	cache->ttd = daemon->local_ttl;
	cache->name.namep = ds->name;
	cache->addr.ds.keylen = ds->digestlen;
	cache->addr.ds.algo = ds->algo;
	cache->addr.ds.keytag = ds->keytag;
	cache->addr.ds.digest = ds->digest_type;
	cache->uid = ds->class;
	cache_hash(cache);
	make_non_terminals(cache);
      }
#endif
  
  /* borrow the packet buffer for a temporary by-address hash */
  memset(daemon->packet, 0, daemon->packet_buff_sz);
  revhashsz = daemon->packet_buff_sz / sizeof(struct crec *);
  /* we overwrote the buffer... */
  daemon->srv_save = NULL;

  /* Do host_records in config. */
  for (hr = daemon->host_records; hr; hr = hr->next)
    for (nl = hr->names; nl; nl = nl->next)
      {
	if ((hr->flags & HR_4) &&
	    (cache = whine_malloc(SIZEOF_POINTER_CREC)))
	  {
	    cache->name.namep = nl->name;
	    cache->ttd = hr->ttl;
	    cache->flags = F_HOSTS | F_IMMORTAL | F_FORWARD | F_REVERSE | F_IPV4 | F_NAMEP | F_CONFIG;
	    add_hosts_entry(cache, (union all_addr *)&hr->addr, INADDRSZ, SRC_CONFIG, (struct crec **)daemon->packet, revhashsz);
	  }

	if ((hr->flags & HR_6) &&
	    (cache = whine_malloc(SIZEOF_POINTER_CREC)))
	  {
	    cache->name.namep = nl->name;
	    cache->ttd = hr->ttl;
	    cache->flags = F_HOSTS | F_IMMORTAL | F_FORWARD | F_REVERSE | F_IPV6 | F_NAMEP | F_CONFIG;
	    add_hosts_entry(cache, (union all_addr *)&hr->addr6, IN6ADDRSZ, SRC_CONFIG, (struct crec **)daemon->packet, revhashsz);
	  }
      }
	
  if (option_bool(OPT_NO_HOSTS) && !daemon->addn_hosts)
    {
      if (daemon->cachesize > 0)
	my_syslog(LOG_INFO, _("cleared cache"));
    }
  else
    {
      if (!option_bool(OPT_NO_HOSTS))
	total_size = read_hostsfile(HOSTSFILE, SRC_HOSTS, total_size, (struct crec **)daemon->packet, revhashsz);
      
      daemon->addn_hosts = expand_filelist(daemon->addn_hosts);
      for (ah = daemon->addn_hosts; ah; ah = ah->next)
	if (!(ah->flags & AH_INACTIVE))
	  total_size = read_hostsfile(ah->fname, ah->index, total_size, (struct crec **)daemon->packet, revhashsz);
    }
  
  /* Make non-terminal records for all locally-define RRs */
  lrec.flags = F_FORWARD | F_CONFIG | F_NAMEP | F_IMMORTAL;
  
  for (txt = daemon->txt; txt; txt = txt->next)
    {
      lrec.name.namep = txt->name;
      make_non_terminals(&lrec);
    }

  for (naptr = daemon->naptr; naptr; naptr = naptr->next)
    {
      lrec.name.namep = naptr->name;
      make_non_terminals(&lrec);
    }

  for (mx = daemon->mxnames; mx; mx = mx->next)
    {
      lrec.name.namep = mx->name;
      make_non_terminals(&lrec);
    }

  for (intr = daemon->int_names; intr; intr = intr->next)
    {
      lrec.name.namep = intr->name;
      make_non_terminals(&lrec);
    }
  
  for (ptr = daemon->ptr; ptr; ptr = ptr->next)
    {
      lrec.name.namep = ptr->name;
      make_non_terminals(&lrec);
    }
  
#ifdef HAVE_INOTIFY
  set_dynamic_inotify(AH_HOSTS, total_size, (struct crec **)daemon->packet, revhashsz);
#endif
  
} 

#ifdef HAVE_DHCP
struct in_addr a_record_from_hosts(char *name, time_t now)
{
  struct crec *crecp = NULL;
  struct in_addr ret;
  
  /* If no DNS service, cache not initialised. */
  if (daemon->port != 0)
    while ((crecp = cache_find_by_name(crecp, name, now, F_IPV4)))
      if (crecp->flags & F_HOSTS)
	return crecp->addr.addr4;
  
  my_syslog(MS_DHCP | LOG_WARNING, _("No IPv4 address found for %s"), name);
  
  ret.s_addr = 0;
  return ret;
}

void cache_unhash_dhcp(void)
{
  struct crec *cache, **up;
  int i;

  for (i=0; i<hash_size; i++)
    for (cache = hash_table[i], up = &hash_table[i]; cache; cache = cache->hash_next)
      if (cache->flags & F_DHCP)
	{
	  *up = cache->hash_next;
	  cache->next = dhcp_spare;
	  dhcp_spare = cache;
	}
      else
	up = &cache->hash_next;
}

void cache_add_dhcp_entry(char *host_name, int prot,
			  union all_addr *host_address, time_t ttd) 
{
  struct crec *crec = NULL, *fail_crec = NULL;
  unsigned int flags = F_IPV4;
  int in_hosts = 0;
  size_t addrlen = sizeof(struct in_addr);

  if (prot == AF_INET6)
    {
      flags = F_IPV6;
      addrlen = sizeof(struct in6_addr);
    }
  
  inet_ntop(prot, host_address, daemon->addrbuff, ADDRSTRLEN);
  
  while ((crec = cache_find_by_name(crec, host_name, 0, flags | F_CNAME)))
    {
      /* check all addresses associated with name */
      if (crec->flags & (F_HOSTS | F_CONFIG))
	{
	  if (crec->flags & F_CNAME)
	    my_syslog(MS_DHCP | LOG_WARNING, 
		      _("%s is a CNAME, not giving it to the DHCP lease of %s"),
		      host_name, daemon->addrbuff);
	  else if (memcmp(&crec->addr, host_address, addrlen) == 0)
	    in_hosts = 1;
	  else
	    fail_crec = crec;
	}
      else if (!(crec->flags & F_DHCP))
	{
	  cache_scan_free(host_name, NULL, C_IN, 0, crec->flags & (flags | F_CNAME | F_FORWARD), NULL, NULL);
	  /* scan_free deletes all addresses associated with name */
	  break;
	}
    }
  
  /* if in hosts, don't need DHCP record */
  if (in_hosts)
    return;
  
  /* Name in hosts, address doesn't match */
  if (fail_crec)
    {
      inet_ntop(prot, &fail_crec->addr, daemon->namebuff, MAXDNAME);
      my_syslog(MS_DHCP | LOG_WARNING, 
		_("not giving name %s to the DHCP lease of %s because "
		  "the name exists in %s with address %s"), 
		host_name, daemon->addrbuff,
		record_source(fail_crec->uid), daemon->namebuff);
      return;
    }	  
  
  if ((crec = cache_find_by_addr(NULL, (union all_addr *)host_address, 0, flags)))
    {
      if (crec->flags & F_NEG)
	{
	  flags |= F_REVERSE;
	  cache_scan_free(NULL, (union all_addr *)host_address, C_IN, 0, flags, NULL, NULL);
	}
    }
  else
    flags |= F_REVERSE;
  
  if ((crec = dhcp_spare))
    dhcp_spare = dhcp_spare->next;
  else /* need new one */
    crec = whine_malloc(SIZEOF_POINTER_CREC);
  
  if (crec) /* malloc may fail */
    {
      crec->flags = flags | F_NAMEP | F_DHCP | F_FORWARD;
      if (ttd == 0)
	crec->flags |= F_IMMORTAL;
      else
	crec->ttd = ttd;
      crec->addr = *host_address;
      crec->name.namep = host_name;
      crec->uid = UID_NONE;
      cache_hash(crec);
      make_non_terminals(crec);
    }
}
#endif

/* Called when we put a local or DHCP name into the cache.
   Creates empty cache entries for subnames (ie,
   for three.two.one, for two.one and one), without
   F_IPV4 or F_IPV6 or F_CNAME set. These convert
   NXDOMAIN answers to NoData ones. */
static void make_non_terminals(struct crec *source)
{
  char *name = cache_get_name(source);
  struct crec *crecp, *tmp, **up;
  int type = F_HOSTS | F_CONFIG;
#ifdef HAVE_DHCP
  if (source->flags & F_DHCP)
    type = F_DHCP;
#endif
  
  /* First delete any empty entries for our new real name. Note that
     we only delete empty entries deriving from DHCP for a new DHCP-derived
     entry and vice-versa for HOSTS and CONFIG. This ensures that 
     non-terminals from DHCP go when we reload DHCP and 
     for HOSTS/CONFIG when we re-read. */
  for (up = hash_bucket(name), crecp = *up; crecp; crecp = tmp)
    {
      tmp = crecp->hash_next;

      if (!is_outdated_cname_pointer(crecp) &&
	  (crecp->flags & F_FORWARD) &&
	  (crecp->flags & type) &&
	  !(crecp->flags & (F_IPV4 | F_IPV6 | F_CNAME | F_SRV | F_DNSKEY | F_DS)) && 
	  hostname_isequal(name, cache_get_name(crecp)))
	{
	  *up = crecp->hash_next;
#ifdef HAVE_DHCP
	  if (type & F_DHCP)
	    {
	      crecp->next = dhcp_spare;
	      dhcp_spare = crecp;
	    }
	  else
#endif
	    free(crecp);
	  break;
	}
      else
	 up = &crecp->hash_next;
    }
     
  while ((name = strchr(name, '.')))
    {
      name++;

      /* Look for one existing, don't need another */
      for (crecp = *hash_bucket(name); crecp; crecp = crecp->hash_next)
	if (!is_outdated_cname_pointer(crecp) &&
	    (crecp->flags & F_FORWARD) &&
	    (crecp->flags & type) &&
	    hostname_isequal(name, cache_get_name(crecp)))
	  break;
      
      if (crecp)
	{
	  /* If the new name expires later, transfer that time to
	     empty non-terminal entry. */
	  if (!(crecp->flags & F_IMMORTAL))
	    {
	      if (source->flags & F_IMMORTAL)
		crecp->flags |= F_IMMORTAL;
	      else if (difftime(crecp->ttd, source->ttd) < 0)
		crecp->ttd = source->ttd;
	    }
	  continue;
	}
      
#ifdef HAVE_DHCP
      if ((source->flags & F_DHCP) && dhcp_spare)
	{
	  crecp = dhcp_spare;
	  dhcp_spare = dhcp_spare->next;
	}
      else
#endif
	crecp = whine_malloc(SIZEOF_POINTER_CREC);

      if (crecp)
	{
	  crecp->flags = (source->flags | F_NAMEP) & ~(F_IPV4 | F_IPV6 | F_CNAME | F_SRV | F_DNSKEY | F_DS | F_REVERSE);
	  if (!(crecp->flags & F_IMMORTAL))
	    crecp->ttd = source->ttd;
	  crecp->name.namep = name;
	  
	  cache_hash(crecp);
	}
    }
}

#ifndef NO_ID
int cache_make_stat(struct txt_record *t)
{ 
  static char *buff = NULL;
  static int bufflen = 60;
  int len;
  struct server *serv, *serv1;
  char *p;

  if (!buff && !(buff = whine_malloc(60)))
    return 0;

  p = buff;
  
  switch (t->stat)
    {
    case TXT_STAT_CACHESIZE:
      sprintf(buff+1, "%d", daemon->cachesize);
      break;

    case TXT_STAT_INSERTS:
      sprintf(buff+1, "%d", daemon->metrics[METRIC_DNS_CACHE_INSERTED]);
      break;

    case TXT_STAT_EVICTIONS:
      sprintf(buff+1, "%d", daemon->metrics[METRIC_DNS_CACHE_LIVE_FREED]);
      break;

    case TXT_STAT_MISSES:
      sprintf(buff+1, "%u", daemon->metrics[METRIC_DNS_QUERIES_FORWARDED]);
      break;

    case TXT_STAT_HITS:
      sprintf(buff+1, "%u", daemon->metrics[METRIC_DNS_LOCAL_ANSWERED]);
      break;

#ifdef HAVE_AUTH
    case TXT_STAT_AUTH:
      sprintf(buff+1, "%u", daemon->metrics[METRIC_DNS_AUTH_ANSWERED]);
      break;
#endif

    case TXT_STAT_SERVERS:
      /* sum counts from different records for same server */
      for (serv = daemon->servers; serv; serv = serv->next)
	serv->flags &= ~SERV_MARK;
      
      for (serv = daemon->servers; serv; serv = serv->next)
	if (!(serv->flags & SERV_MARK))
	  {
	    char *new, *lenp;
	    int port, newlen, bytes_avail, bytes_needed;
	    unsigned int queries = 0, failed_queries = 0;
	    for (serv1 = serv; serv1; serv1 = serv1->next)
	      if (!(serv1->flags & SERV_MARK) && sockaddr_isequal(&serv->addr, &serv1->addr))
		{
		  serv1->flags |= SERV_MARK;
		  queries += serv1->queries;
		  failed_queries += serv1->failed_queries;
		}
	    port = prettyprint_addr(&serv->addr, daemon->addrbuff);
	    lenp = p++; /* length */
	    bytes_avail = bufflen - (p - buff );
	    bytes_needed = snprintf(p, bytes_avail, "%s#%d %u %u", daemon->addrbuff, port, queries, failed_queries);
	    if (bytes_needed >= bytes_avail)
	      {
		/* expand buffer if necessary */
		newlen = bytes_needed + 1 + bufflen - bytes_avail;
		if (!(new = whine_realloc(buff, newlen)))
		  return 0;
		p = new + (p - buff);
		lenp = p - 1;
		buff = new;
		bufflen = newlen;
		bytes_avail =  bufflen - (p - buff );
		bytes_needed = snprintf(p, bytes_avail, "%s#%d %u %u", daemon->addrbuff, port, queries, failed_queries);
	      }
	    *lenp = bytes_needed;
	    p += bytes_needed;
	  }
      t->txt = (unsigned char *)buff;
      t->len = p - buff;

      return 1;
    }
  
  len = strlen(buff+1);
  t->txt = (unsigned char *)buff;
  t->len = len + 1;
  *buff = len;
  return 1;
}
#endif

/* There can be names in the cache containing control chars, don't 
   mess up logging or open security holes. */
static char *sanitise(char *name)
{
  unsigned char *r;
  if (name)
    for (r = (unsigned char *)name; *r; r++)
      if (!isprint((int)*r))
	return "<name unprintable>";

  return name;
}

static void dump_cache_entry(struct crec *cache, time_t now)
{
  (void)now;
  static char *buff = NULL;
  
  char *p, *t = " ";
  char *a = daemon->addrbuff, *n = cache_get_name(cache);

  /* String length is limited below */
  if (!buff && !(buff = whine_malloc(150)))
    return;
  
  p = buff;
  
  *a = 0;
  if (strlen(n) == 0 && !(cache->flags & F_REVERSE))
    n = "<Root>";
  p += sprintf(p, "%-30.30s ", sanitise(n));
  if ((cache->flags & F_CNAME) && !is_outdated_cname_pointer(cache))
    a = sanitise(cache_get_cname_target(cache));
  else if ((cache->flags & F_SRV) && !(cache->flags & F_NEG))
    {
      int targetlen = cache->addr.srv.targetlen;
      ssize_t len = sprintf(a, "%u %u %u ", cache->addr.srv.priority,
			    cache->addr.srv.weight, cache->addr.srv.srvport);
      
      if (targetlen > (40 - len))
	targetlen = 40 - len;
      blockdata_retrieve(cache->addr.srv.target, targetlen, a + len);
      a[len + targetlen] = 0;		
    }
#ifdef HAVE_DNSSEC
  else if (cache->flags & F_DS)
    {
      if (!(cache->flags & F_NEG))
	sprintf(a, "%5u %3u %3u", cache->addr.ds.keytag,
		cache->addr.ds.algo, cache->addr.ds.digest);
    }
  else if (cache->flags & F_DNSKEY)
    sprintf(a, "%5u %3u %3u", cache->addr.key.keytag,
	    cache->addr.key.algo, cache->addr.key.flags);
#endif
  else if (!(cache->flags & F_NEG) || !(cache->flags & F_FORWARD))
    { 
      a = daemon->addrbuff;
      if (cache->flags & F_IPV4)
	inet_ntop(AF_INET, &cache->addr, a, ADDRSTRLEN);
      else if (cache->flags & F_IPV6)
	inet_ntop(AF_INET6, &cache->addr, a, ADDRSTRLEN);
    }
  
  if (cache->flags & F_IPV4)
    t = "4";
  else if (cache->flags & F_IPV6)
    t = "6";
  else if (cache->flags & F_CNAME)
    t = "C";
  else if (cache->flags & F_SRV)
    t = "V";
#ifdef HAVE_DNSSEC
  else if (cache->flags & F_DS)
    t = "S";
  else if (cache->flags & F_DNSKEY)
    t = "K";
#endif
  else if (!(cache->flags & F_NXDOMAIN)) /* non-terminal */
    t = "!";
  
  p += sprintf(p, "%-40.40s %s%s%s%s%s%s%s%s%s%s ", a, t,
	       cache->flags & F_FORWARD ? "F" : " ",
	       cache->flags & F_REVERSE ? "R" : " ",
	       cache->flags & F_IMMORTAL ? "I" : " ",
	       cache->flags & F_DHCP ? "D" : " ",
	       cache->flags & F_NEG ? "N" : " ",
	       cache->flags & F_NXDOMAIN ? "X" : " ",
	       cache->flags & F_HOSTS ? "H" : " ",
	       cache->flags & F_CONFIG ? "C" : " ",
	       cache->flags & F_DNSSECOK ? "V" : " ");
#ifdef HAVE_BROKEN_RTC
  p += sprintf(p, "%-24lu", cache->flags & F_IMMORTAL ? 0: (unsigned long)(cache->ttd - now));
#else
  p += sprintf(p, "%-24.24s", cache->flags & F_IMMORTAL ? "" : ctime(&(cache->ttd)));
#endif
  if(cache->flags & (F_HOSTS | F_CONFIG) && cache->uid > 0)
    p += sprintf(p, " %-40.40s", record_source(cache->uid));
  
  my_syslog(LOG_INFO, "%s", buff);
}

void dump_cache(time_t now)
{
  struct server *serv, *serv1;

  my_syslog(LOG_INFO, _("time %lu"), (unsigned long)now);
  my_syslog(LOG_INFO, _("cache size %d, %d/%d cache insertions re-used unexpired cache entries."), 
	    daemon->cachesize, daemon->metrics[METRIC_DNS_CACHE_LIVE_FREED], daemon->metrics[METRIC_DNS_CACHE_INSERTED]);
  my_syslog(LOG_INFO, _("queries forwarded %u, queries answered locally %u"), 
	    daemon->metrics[METRIC_DNS_QUERIES_FORWARDED], daemon->metrics[METRIC_DNS_LOCAL_ANSWERED]);
  if (daemon->cache_max_expiry != 0)
    my_syslog(LOG_INFO, _("queries answered from stale cache %u"), daemon->metrics[METRIC_DNS_STALE_ANSWERED]);
#ifdef HAVE_AUTH
  my_syslog(LOG_INFO, _("queries for authoritative zones %u"), daemon->metrics[METRIC_DNS_AUTH_ANSWERED]);
#endif

  blockdata_report();

  /* sum counts from different records for same server */
  for (serv = daemon->servers; serv; serv = serv->next)
    serv->flags &= ~SERV_MARK;
  
  for (serv = daemon->servers; serv; serv = serv->next)
    if (!(serv->flags & SERV_MARK))
      {
	int port;
	unsigned int queries = 0, failed_queries = 0, nxdomain_replies = 0, retrys = 0;
	unsigned int sigma_latency = 0, count_latency = 0;

	for (serv1 = serv; serv1; serv1 = serv1->next)
	  if (!(serv1->flags & SERV_MARK) && sockaddr_isequal(&serv->addr, &serv1->addr))
	    {
	      serv1->flags |= SERV_MARK;
	      queries += serv1->queries;
	      failed_queries += serv1->failed_queries;
	      nxdomain_replies += serv1->nxdomain_replies;
	      retrys += serv1->retrys;
	      sigma_latency += serv1->query_latency;
	      count_latency++;
	    }
	port = prettyprint_addr(&serv->addr, daemon->addrbuff);
	my_syslog(LOG_INFO, _("server %s#%d: queries sent %u, retried %u, failed %u, nxdomain replies %u, avg. latency %ums"),
		  daemon->addrbuff, port, queries, retrys, failed_queries, nxdomain_replies, sigma_latency/count_latency);
      }

  if (option_bool(OPT_DEBUG) || option_bool(OPT_LOG))
    {
      struct crec *cache;
      int i;
      my_syslog(LOG_INFO, "Host                           Address                                  Flags      Expires                  Source");
      my_syslog(LOG_INFO, "------------------------------ ---------------------------------------- ---------- ------------------------ ------------");
    
      for (i=0; i<hash_size; i++)
	for (cache = hash_table[i]; cache; cache = cache->hash_next)
	  dump_cache_entry(cache, now);
    }
}

char *record_source(unsigned int index)
{
  struct hostsfile *ah;
#ifdef HAVE_INOTIFY
  struct dyndir *dd;
#endif
  
  if (index == SRC_CONFIG)
    return "config";
  else if (index == SRC_HOSTS)
    return HOSTSFILE;

  for (ah = daemon->addn_hosts; ah; ah = ah->next)
    if (ah->index == index)
      return ah->fname;

#ifdef HAVE_INOTIFY
  /* Dynamic directories contain multiple files */
  for (dd = daemon->dynamic_dirs; dd; dd = dd->next)
    for (ah = dd->files; ah; ah = ah->next)
      if (ah->index == index)
	return ah->fname;
#endif

  return "<unknown>";
}

static char *querystr(char *desc, unsigned short type)
{
  unsigned int i;
  int len = 10; /* strlen("type=xxxxx") */
  const char *types = NULL;
  static char *buff = NULL;
  static int bufflen = 0;

  for (i = 0; i < (sizeof(typestr)/sizeof(typestr[0])); i++)
    if (typestr[i].type == type)
      {
	types = typestr[i].name;
	len = strlen(types);
	break;
      }

  if (desc)
    {
       len += 2; /* braces */
       len += strlen(desc);
    }
  len++; /* terminator */
  
  if (!buff || bufflen < len)
    {
      if (buff)
	free(buff);
      else if (len < 20)
	len = 20;
      
      buff = whine_malloc(len);
      bufflen = len;
    }

  if (buff)
    {
      if (desc)
	{
	  if (types)
	    sprintf(buff, "%s[%s]", desc, types);
	  else
	    sprintf(buff, "%s[type=%d]", desc, type);
	}
      else
	{
	  if (types)
	    sprintf(buff, "<%s>", types);
	  else
	    sprintf(buff, "<type=%d>", type);
	}
    }
  
  return buff ? buff : "";
}

static char *edestr(int ede)
{
  switch (ede)
    {
    case EDE_OTHER:                       return "other";
    case EDE_USUPDNSKEY:                  return "unsupported DNSKEY algorithm";
    case EDE_USUPDS:                      return "unsupported DS digest";
    case EDE_STALE:                       return "stale answer";
    case EDE_FORGED:                      return "forged";
    case EDE_DNSSEC_IND:                  return "DNSSEC indeterminate";
    case EDE_DNSSEC_BOGUS:                return "DNSSEC bogus";
    case EDE_SIG_EXP:                     return "DNSSEC signature expired";
    case EDE_SIG_NYV:                     return "DNSSEC sig not yet valid";
    case EDE_NO_DNSKEY:                   return "DNSKEY missing";
    case EDE_NO_RRSIG:                    return "RRSIG missing";
    case EDE_NO_ZONEKEY:                  return "no zone key bit set";
    case EDE_NO_NSEC:                     return "NSEC(3) missing";
    case EDE_CACHED_ERR:                  return "cached error";
    case EDE_NOT_READY:                   return "not ready";
    case EDE_BLOCKED:                     return "blocked";
    case EDE_CENSORED:                    return "censored";
    case EDE_FILTERED:                    return "filtered";
    case EDE_PROHIBITED:                  return "prohibited";
    case EDE_STALE_NXD:                   return "stale NXDOMAIN";
    case EDE_NOT_AUTH:                    return "not authoritative";
    case EDE_NOT_SUP:                     return "not supported";
    case EDE_NO_AUTH:                     return "no reachable authority";
    case EDE_NETERR:                      return "network error";
    case EDE_INVALID_DATA:                return "invalid data";
    default:                              return "unknown";
    }
}

void log_query(unsigned int flags, char *name, union all_addr *addr, char *arg, unsigned short type)
{
  char *source, *dest = arg;
  char *verb = "is";
  char *extra = "";
  char portstring[7]; /* space for #<portnum> */
  
  if (!option_bool(OPT_LOG))
    return;

  /* build query type string if requested */
  if (!(flags & (F_SERVER | F_IPSET)) && type > 0)
    arg = querystr(arg, type);

#ifdef HAVE_DNSSEC
  if ((flags & F_DNSSECOK) && option_bool(OPT_EXTRALOG))
    extra = " (DNSSEC signed)";
#endif

  name = sanitise(name);

  if (addr)
    {
      dest = daemon->addrbuff;

      if (flags & F_KEYTAG)
	sprintf(daemon->addrbuff, arg, addr->log.keytag, addr->log.algo, addr->log.digest);
      else if (flags & F_RCODE)
	{
	  unsigned int rcode = addr->log.rcode;

	  if (rcode == SERVFAIL)
	    dest = "SERVFAIL";
	  else if (rcode == REFUSED)
	    dest = "REFUSED";
	  else if (rcode == NOTIMP)
	    dest = "not implemented";
	  else
	    sprintf(daemon->addrbuff, "%u", rcode);

	  if (addr->log.ede != EDE_UNSET)
	    {
	      extra = daemon->addrbuff;
	      sprintf(extra, " (EDE: %s)", edestr(addr->log.ede));
	    }
	}
      else if (flags & (F_IPV4 | F_IPV6))
	{
	  inet_ntop(flags & F_IPV4 ? AF_INET : AF_INET6,
		    addr, daemon->addrbuff, ADDRSTRLEN);
	  if ((flags & F_SERVER) && type != NAMESERVER_PORT)
	    {
	      extra = portstring;
	      sprintf(portstring, "#%u", type);
	    }
	}
      else
	dest = arg;
    }

  if (flags & F_REVERSE)
    {
      dest = name;
      name = daemon->addrbuff;
    }
  
  if (flags & F_NEG)
    {
      if (flags & F_NXDOMAIN)
	dest = "NXDOMAIN";
      else
	{      
	  if (flags & F_IPV4)
	    dest = "NODATA-IPv4";
	  else if (flags & F_IPV6)
	    dest = "NODATA-IPv6";
	  else
	    dest = "NODATA";
	}
    }
  else if (flags & F_CNAME)
    dest = "<CNAME>";
  else if (flags & F_SRV)
    dest = "<SRV>";
  else if (flags & F_RRNAME)
    dest = arg;
    
  if (flags & F_CONFIG)
    source = "config";
  else if (flags & F_DHCP)
    source = "DHCP";
  else if (flags & F_HOSTS)
    source = arg;
  else if (flags & F_UPSTREAM)
    source = "reply";
  else if (flags & F_SECSTAT)
    {
      if (addr && addr->log.ede != EDE_UNSET && option_bool(OPT_EXTRALOG))
	{
	  extra = daemon->addrbuff;
	  sprintf(extra, " (EDE: %s)", edestr(addr->log.ede));
	}
      source = "validation";
      dest = arg;
    }
  else if (flags & F_AUTH)
    source = "auth";
   else if (flags & F_DNSSEC)
    {
      source = arg;
      verb = "to";
    }
   else if (flags & F_SERVER)
    {
      source = "forwarded";
      verb = "to";
    }
  else if (flags & F_QUERY)
    {
      source = arg;
      verb = "from";
    }
  else if (flags & F_IPSET)
    {
      source = type ? "ipset add" : "nftset add";
      dest = name;
      name = arg;
      verb = daemon->addrbuff;
    }
  else if (flags & F_STALE)
    source = "cached-stale";
  else
    source = "cached";
  
  if (name && !name[0])
    name = ".";

  if (option_bool(OPT_EXTRALOG))
    {
      if (flags & F_NOEXTRA)
	my_syslog(LOG_INFO, "%u %s %s %s %s%s", daemon->log_display_id, source, name, verb, dest, extra);
      else
	{
	   int port = prettyprint_addr(daemon->log_source_addr, daemon->addrbuff2);
	   my_syslog(LOG_INFO, "%u %s/%u %s %s %s %s%s", daemon->log_display_id, daemon->addrbuff2, port, source, name, verb, dest, extra);
	}
    }
  else
    my_syslog(LOG_INFO, "%s %s %s %s%s", source, name, verb, dest, extra);
}
