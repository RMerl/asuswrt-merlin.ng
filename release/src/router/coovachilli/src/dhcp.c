/* -*- mode: c; c-basic-offset: 2 -*- */
/* 
 * Copyright (C) 2007-2012 David Bird (Coova Technologies) <support@coova.com>
 * Copyright (C) 2003, 2004, 2005, 2006 Mondru AB.
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
#ifdef ENABLE_MODULES
#include "chilli_module.h"
#endif
#include "debug.h"
#include "rtstate.h"
#include <bcmnvram.h>
#include <shutils.h>



const uint32_t DHCP_OPTION_MAGIC = 0x63825363;
static uint8_t bmac[PKT_ETH_ALEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
static uint8_t nmac[PKT_ETH_ALEN] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static int connections = 0;

extern struct ippool_t *ippool;

struct dhcp_ctx {
  struct dhcp_t *parent;
  int idx;
};

#ifdef ENABLE_CLUSTER
int chilli_peer_count = 0;
struct chilli_peer * chilli_peers = 0;
struct chilli_peer * get_chilli_peer(int id) {
  if (id < 0) id = _options.peerid;
  if (_options.peerid >= 8) {
    log_err(0, "Valid PEERID is 0-7!");
    exit(-1);
  }
  if (!chilli_peers) {
    /* initialize */
    struct chilli_peer *p;
    chilli_peers = (struct chilli_peer *)calloc(8, sizeof(struct chilli_peer));
    p = chilli_peers + _options.peerid;
    memcpy(p->mac, dhcp_nexthop(dhcp), PKT_ETH_ALEN);
    memcpy(&p->addr, &_options.uamlisten, sizeof(_options.uamlisten));
    if (_options.peerid == 0)
      p->state = PEER_STATE_ACTIVE;
    else
      p->state = PEER_STATE_STANDBY;
  }
  if (id < 8) 
    return chilli_peers + id;
  return 0;
}

void print_peers(bstring s) {
  time_t now = mainclock_now();
  char line[512];
  int i;

  for (i=0; i<8; i++) {
    struct chilli_peer *peer = chilli_peers + i;
    int age = (int)(now - peer->last_update);
    char *state = "";

    switch(peer->state) {
    case PEER_STATE_ACTIVE:
      state = "Active";
      break;
    case PEER_STATE_STANDBY:
      state = "Standby";
      break;
    case PEER_STATE_OFFLINE:
      state = "Offline";
      age = 0;
      break;
    case PEER_STATE_ADMCMD:
      state = "AdminCmd";
      break;
    case PEER_STATE_MODULE:
      state = "Module";
      break;
    }

    safe_snprintf(line, sizeof(line),
		  "Peer %d %-4s %.2x:%.2x:%.2x:%.2x:%.2x:%.2x "
		  "/ %-16s %-8s %d sec", 
		  i, _options.peerid == i ? "(*)" : "",
		  peer->mac[0],peer->mac[1],peer->mac[2],
		  peer->mac[3],peer->mac[4],peer->mac[5],
		  inet_ntoa(peer->addr), state, age);
    
    bcatcstr(s, line);
    bcatcstr(s, "\n");
  }
}

int dhcp_send_chilli(uint8_t *pkt, size_t len) {
  return 0;
}

static 
int dhcp_sendCHILLI(uint8_t type, struct in_addr *addr, uint8_t *mac) {
  EVP_CIPHER_CTX ctx;

  uint8_t packet[1500];

  struct pkt_ethhdr_t *packet_ethh;
  struct pkt_chillihdr_t chilli_hdr;
  unsigned char *outbuf;

  int datalen = 0;
  int olen = 0;
  int tlen = 0;

  int ret = -1;

  unsigned char iv[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08 };

  if (_options.peerkey == 0)
    _options.peerkey = "hello!";
  
  /* Get packet default values */
  memset(packet, 0, sizeof(packet));
  packet_ethh = pkt_ethhdr(packet);
  outbuf = (unsigned char *)chilli_ethhdr(packet);
	 
  packet_ethh->prot = htons(PKT_ETH_PROTO_CHILLI);
  memcpy(packet_ethh->dst, bmac, PKT_ETH_ALEN);
  memcpy(packet_ethh->src, dhcp_nexthop(dhcp), PKT_ETH_ALEN);

  chilli_hdr.from = _options.peerid;
  chilli_hdr.type = type;
  
  if (!addr || !mac) {
    struct chilli_peer *p = get_chilli_peer(-1);
    addr = &p->addr;
    mac = p->mac;
    chilli_hdr.state = p->state;
  }
  
  memcpy(&chilli_hdr.addr, addr, sizeof(*addr));
  memcpy(chilli_hdr.mac, mac, PKT_ETH_ALEN);
  
  EVP_CIPHER_CTX_init(&ctx);
  EVP_EncryptInit(&ctx, EVP_bf_cbc(), 
		  (const unsigned char *)_options.peerkey, iv);
  
  if (EVP_EncryptUpdate (&ctx, outbuf, &olen, 
			 (const unsigned char *) &chilli_hdr, 
			 sizeof(chilli_hdr)) != 1) {
    log_err(errno, "CHILLI: peer %d error in encrypt update",
	    _options.peerid);
  } else {
    datalen += olen;
    if (EVP_EncryptFinal (&ctx, outbuf + olen, &tlen) != 1) {
      log_err(errno, "CHILLI: peer %d error in encrypt final",
	      _options.peerid);
    } else {
      datalen += tlen;
      
      log_dbg("CHILLI: peer %d sending %d bytes", 
	      _options.peerid, datalen);
      
      ret = dhcp_send(dhcp, -1, bmac, packet, 
		      sizeofeth(packet) + datalen);
    }
  }

  EVP_CIPHER_CTX_cleanup(&ctx);

  return ret;
}
#endif

#ifdef ENABLE_GARDENACCOUNTING
static int other_data(struct dhcp_conn_t *conn, 
		      struct pkt_iphdr_t *iph, int len, int dst ) {
  if (conn && _options.uamgardendata && _options.uamotherdata) {
    struct app_conn_t *appconn = dhcp_get_appconn_pkt(conn, iph, dst);
    if (appconn) {
      extern struct app_conn_t admin_session;
      if (!len && iph) 
	len = ntohs(iph->tot_len);
      if (_options.swapoctets) {
	if (dst)
	  appconn->s_state.other_output_octets +=len;
	else
	  appconn->s_state.other_input_octets +=len;
	if (admin_session.s_state.authenticated) {
	  if (dst)
	    admin_session.s_state.other_output_octets+=len;
	  else
	    admin_session.s_state.other_input_octets+=len;
	}
      } else {
	if (dst)
	  appconn->s_state.other_input_octets +=len;
	else
	  appconn->s_state.other_output_octets +=len;
	if (admin_session.s_state.authenticated) {
	  if (dst)
	    admin_session.s_state.other_input_octets+=len;
	  else
	    admin_session.s_state.other_output_octets+=len;
	}
      }
    }
  }
  return 0;
}
#define OTHER_RECEIVED(conn,iph) other_data((conn), (iph), 0, 0)
#define OTHER_RECEIVED_LEN(conn,len) other_data((conn), 0, (len), 0)
#define OTHER_SENDING(conn,iph) other_data((conn), (iph), 0, 1)
#define OTHER_SENDING_LEN(conn,len) other_data((conn), 0, (len), 1)
#else
#define OTHER_RECEIVED(conn,iph) 
#define OTHER_RECEIVED_LEN(conn,iph) 
#define OTHER_SENDING(conn,iph) 
#define OTHER_SENDING_LEN(conn,len)
#endif

struct app_conn_t *
dhcp_get_appconn_ip(struct dhcp_conn_t *conn, struct in_addr *dst) {
  struct app_conn_t *appconn = 0;
  struct ippoolm_t *ipm = 0;
  
#if(_debug_ > 1)
  log_dbg("Looking up appconn for %s", inet_ntoa(*dst));
#endif
  
  if (ippool_getip(ippool, &ipm, dst)) {
    log_dbg("No ip assigned for %s", inet_ntoa(*dst));
    return 0;
  }
  
  if (!ipm) {
    log_dbg("unknown ip");
    return 0;
  }
  
  if ((appconn = (struct app_conn_t *)ipm->peer) == NULL) {
    if (chilli_getconn(&appconn, dst->s_addr, 0, 0)) {
      if (conn && chilli_connect(&appconn, conn)) {
	log_err(0, "chilli_connect()");
	return 0;
      }
    }
  }

  return appconn;
}  

struct app_conn_t *
dhcp_get_appconn_pkt(struct dhcp_conn_t *conn, 
		     struct pkt_iphdr_t *iph, 
		     char is_dst) {
#ifdef ENABLE_LAYER3
  switch (conn->authstate) {
  case DHCP_AUTH_ROUTER:
    {
      struct in_addr dst;

      dst.s_addr = ((is_dst) ? 
		    iph->daddr : 
		    iph->saddr);

      return dhcp_get_appconn_ip(conn, &dst);
    }
  }
#endif

  /* log_dbg("Layer2 appconn"); */
  return (struct app_conn_t *) conn ? conn->peer : 0;
}

static struct app_conn_t *
dhcp_get_appconn_addr(struct dhcp_conn_t *conn, struct in_addr *dst) {
#ifdef ENABLE_LAYER3
  switch (conn->authstate) {
  case DHCP_AUTH_ROUTER:
    return dhcp_get_appconn_ip(conn, dst);
  }
#endif

  /*log_dbg("Layer2 appconn");*/

  return (struct app_conn_t *) conn->peer;
}

/**
 * dhcp_sendGARP()
 * Send Gratuitous ARP message to network
 * http://wiki.wireshark.org/Gratuitous_ARP
 **/
int
dhcp_sendGARP(struct dhcp_t *this, int idx) {

  uint8_t packet[1500];

  struct pkt_ethhdr_t *packet_ethh;
  struct arp_packet_t *packet_arp;

  memset(packet, 0, sizeof(packet));

  packet_ethh = pkt_ethhdr(packet);
  packet_arp = pkt_arppkt(packet);
	 
  /* ARP Payload */
  packet_arp->hrd = htons(DHCP_HTYPE_ETH);
  packet_arp->pro = htons(PKT_ETH_PROTO_IP);
  packet_arp->hln = PKT_ETH_ALEN;
  packet_arp->pln = PKT_IP_ALEN;
  packet_arp->op  = htons(DHCP_ARP_REPLY);

  /* Source address */
  memcpy(packet_arp->sha, dhcp_nexthop(this), PKT_ETH_ALEN);
  memcpy(packet_arp->spa, &_options.dhcplisten.s_addr, PKT_IP_ALEN);

  /* Target address */
  memcpy(packet_arp->tha, bmac, PKT_ETH_ALEN);
  memcpy(packet_arp->tpa, &_options.dhcplisten.s_addr, PKT_IP_ALEN);

  log_dbg("GARP: Replying to broadcast");
  
  /* Ethernet header */
  memcpy(packet_ethh->dst, bmac, PKT_ETH_ALEN);
  memcpy(packet_ethh->src, dhcp_nexthop(this), PKT_ETH_ALEN);
  packet_ethh->prot = htons(PKT_ETH_PROTO_ARP);

  return dhcp_send(this, idx, bmac, packet, sizeofarp(packet));
}

void dhcp_authorize_mac(struct dhcp_t *this, uint8_t *hwaddr,
			struct session_params *params) {
  struct dhcp_conn_t *conn;
  if (!dhcp_hashget(this, &conn, hwaddr)) {
    struct app_conn_t * appconn = (struct app_conn_t*) conn->peer;
    if (appconn) {
      memcpy(&appconn->s_params, params, 
	     sizeof(struct session_params));
      session_param_defaults(&appconn->s_params);
      dnprot_accept(appconn);
    }
  }
}

void dhcp_release_mac(struct dhcp_t *this, uint8_t *hwaddr, int term_cause) {
  struct dhcp_conn_t *conn;
  if (!dhcp_hashget(this, &conn, hwaddr)) {
    if (conn->authstate == DHCP_AUTH_DROP &&
	term_cause != RADIUS_TERMINATE_CAUSE_ADMIN_RESET) 
      return;
    dhcp_freeconn(conn, term_cause);
  }
}

#if (0) /* ENABLE_TCPRESET */
void dhcp_reset_tcp_mac(struct dhcp_t *this, uint8_t *hwaddr) {
  struct dhcp_conn_t *conn;
  if (!dhcp_hashget(this, &conn, hwaddr)) {
    int n;
    for (n=0; n<DHCP_DNAT_MAX; n++) {
      if (conn->dnat[n].dst_ip) {
	uint8_t *ip = (uint8_t*)&conn->dnat[n].dst_ip;
	log_dbg("Resetting dst %d.%d.%d.%d:%d", 
		ip[0], ip[1], ip[2], ip[3], conn->dnat[n].dst_port);
      }
      if (conn->dnat[n].src_ip) {
	uint8_t *ip = (uint8_t*)&conn->dnat[n].src_ip;
	log_dbg("Resetting src %d.%d.%d.%d:%d", 
		ip[0], ip[1], ip[2], ip[3], conn->dnat[n].src_port);
      }
    }
  }
}
#endif

void dhcp_block_mac(struct dhcp_t *this, uint8_t *hwaddr) {
  struct dhcp_conn_t *conn;
  if (!dhcp_hashget(this, &conn, hwaddr)) {
    struct app_conn_t *appconn = (struct app_conn_t *)conn->peer;
    conn->authstate = DHCP_AUTH_DROP;
    if (appconn) appconn->dnprot = DNPROT_NULL;
  }
}

int dhcp_net_send(struct _net_interface *netif, unsigned char *hismac, 
		  uint8_t *packet, size_t length) {
#if defined(__linux__)
#ifndef USING_MMAP
  if (hismac) {
    netif->dest.sll_halen = PKT_ETH_ALEN;
    memcpy(netif->dest.sll_addr, hismac, PKT_ETH_ALEN);
  } else {
    netif->dest.sll_halen = 0;
    memset(netif->dest.sll_addr, 0, sizeof(netif->dest.sll_addr));
  }
#endif
  
#if(_debug_ > 1)
  log_dbg("dhcp_send() len=%d", length);
#endif
  
  return net_write_eth(netif, packet, length, &netif->dest);

#elif defined (__FreeBSD__) || defined (__APPLE__) || defined (__OpenBSD__) || defined (__NetBSD__)
  if (safe_write(netif->fd, packet, length) < 0) {
    log_err(errno, "write() failed");
    return -1;
  }
#endif

  return -1;
}

int dhcp_send(struct dhcp_t *this, int idx,
	   unsigned char *hismac, uint8_t *packet, size_t length) {
  net_interface *iface = 0;

  if (_options.tcpwin)
    pkt_shape_tcpwin(pkt_iphdr(packet), _options.tcpwin);

  if (_options.tcpmss)
    pkt_shape_tcpmss(packet, &length);

#ifdef ENABLE_MULTILAN
  if (idx < 0) {
    int i, ret = -1;
    for (i=0; i < MAX_RAWIF && this->rawif[i].fd; i++) 
      ret = dhcp_net_send(&this->rawif[i], hismac, packet, length);
    return ret;
  }
  iface = &this->rawif[idx];
#else
  iface = &this->rawif[0];
#endif

  return dhcp_net_send(iface, hismac, packet, length);
}


/**
 * dhcp_hash()
 * Generates a 32 bit hash based on a mac address
 **/
uint32_t dhcp_hash(uint8_t *hwaddr) {
  return lookup(hwaddr, PKT_ETH_ALEN, 0);
}

/**
 * dhcp_hashinit()
 * Initialises hash tables
 **/
int dhcp_hashinit(struct dhcp_t *this, int listsize) {
  /* Determine hashlog */
  for ((this)->hashlog = 0; 
       ((1 << (this)->hashlog) < listsize);
       (this)->hashlog++);
  
  /* Determine hashsize */
  (this)->hashsize = 1 << (this)->hashlog;
  (this)->hashmask = (this)->hashsize -1;
  
  /* Allocate hash table */
  if (!((this)->hash = 
	calloc(sizeof(struct dhcp_conn_t *), (this)->hashsize))) {
    /* Failed to allocate memory for hash members */
    return -1;
  }
  
  log_dbg("hash table size %d (%d)", this->hashsize, listsize);
  return 0;
}


/**
 * dhcp_hashadd()
 * Adds a connection to the hash table
 **/
int dhcp_hashadd(struct dhcp_t *this, struct dhcp_conn_t *conn) {
  uint32_t hash;
  struct dhcp_conn_t *p;
  struct dhcp_conn_t *p_prev = NULL; 

  /* Insert into hash table */
  hash = dhcp_hash(conn->hismac) & this->hashmask;
  for (p = this->hash[hash]; p; p = p->nexthash)
    p_prev = p;
  if (!p_prev)
    this->hash[hash] = conn;
  else 
    p_prev->nexthash = conn;

  return 0; /* Always OK to insert */
}


/**
 * dhcp_hashdel()
 * Removes a connection from the hash table
 **/
int dhcp_hashdel(struct dhcp_t *this, struct dhcp_conn_t *conn) {
  uint32_t hash;
  struct dhcp_conn_t *p;
  struct dhcp_conn_t *p_prev = NULL; 

  /* Find in hash table */
  hash = dhcp_hash(conn->hismac) & this->hashmask;
  for (p = this->hash[hash]; p; p = p->nexthash) {
    if (p == conn) {
      break;
    }
    p_prev = p;
  }

  if (!p_prev)
    this->hash[hash] = p->nexthash;
  else
    p_prev->nexthash = p->nexthash;
  
  return 0;
}


#ifdef ENABLE_IEEE8021Q
int vlanupdate_script(struct dhcp_conn_t *conn, char* script, 
		      uint16_t oldtag) {  
  int status;
  uint16_t tag;

  if ((status = chilli_fork(CHILLI_PROC_SCRIPT, script)) < 0) {
    log_err(errno, "forking %s", script);
    return 0;
  }

  if (status > 0) { /* Parent */
    return 0; 
  } 

  tag = ntohs(conn->tag8021q & PKT_8021Q_MASK_VID);
  oldtag = ntohs(oldtag & PKT_8021Q_MASK_VID);
  
  set_env("DEV", VAL_STRING, tun(tun, 0).devname, 0);
  set_env("ADDR", VAL_IN_ADDR, &conn->ourip, 0);
  set_env("FRAMED_IP_ADDRESS", VAL_IN_ADDR, &conn->hisip, 0);
  set_env("CALLING_STATION_ID", VAL_MAC_ADDR, conn->hismac, 0);
  set_env("CALLED_STATION_ID", VAL_MAC_ADDR, dhcp_nexthop(dhcp), 0);
  set_env("NAS_ID", VAL_STRING, _options.radiusnasid, 0);
  set_env("8021Q_TAG", VAL_USHORT, &tag, 0);
  set_env("OLD_8021Q_TAG", VAL_USHORT, &oldtag, 0);
  
  if (execl(
#ifdef ENABLE_CHILLISCRIPT
	    SBINDIR "/chilli_script", SBINDIR "/chilli_script", _options.binconfig, 
#else
	    script,
#endif
	    script, (char *) 0) != 0) {
    log_err(errno, "exec %s failed", script);
  }

  exit(0);
}

void dhcp_checktag(struct dhcp_conn_t *conn, uint8_t *pack) {
  if (_options.ieee8021q && is_8021q(pack)) {
    uint16_t tag = get8021q(pack);
    struct app_conn_t * appconn = conn->peer;

    if ((tag & PKT_8021Q_MASK_VID) !=
	(conn->tag8021q & PKT_8021Q_MASK_VID)) {
      uint16_t oldtag = conn->tag8021q;
      conn->tag8021q = tag;
      
      log_dbg("IEEE 802.1Q: %.2x:%.2x:%.2x:%.2x:%.2x:%.2x on VLAN %d", 
	      conn->hismac[0], conn->hismac[1], conn->hismac[2],
	      conn->hismac[3], conn->hismac[4], conn->hismac[5],
	      (int)ntohs(tag & PKT_8021Q_MASK_VID));
      
      if (_options.vlanupdate) {
	vlanupdate_script(conn, _options.vlanupdate, oldtag);
      }

#ifdef ENABLE_LOCATION
      if (_options.vlanlocation && appconn) {
	char vid[16];
	safe_snprintf(vid, sizeof(vid), "%d", 
		      (int)ntohs(tag & PKT_8021Q_MASK_VID));
	chilli_learn_location((uint8_t *)vid, strlen(vid), appconn, 1);
      }
#endif

    }
    if (appconn) {
      appconn->s_state.tag8021q = conn->tag8021q;
    }
  }
}
#endif

/**
 * dhcp_hashget()
 * Uses the hash tables to find a connection based on the mac address.
 * Returns -1 if not found.
 **/
int dhcp_hashget(struct dhcp_t *this, struct dhcp_conn_t **conn, 
		 uint8_t *hwaddr) {
  struct dhcp_conn_t *p;
  uint32_t hash;

  /* Find in hash table */
  hash = dhcp_hash(hwaddr) & this->hashmask;
  for (p = this->hash[hash]; p; p = p->nexthash) {
    if ((!memcmp(p->hismac, hwaddr, PKT_ETH_ALEN)) && (p->inuse)) {
      *conn = p;
      return 0;
    }
  }
  *conn = NULL;
  return -1; /* Address could not be found */
}

/**
 * dhcp_lnkconn()
 * Allocates/link a new connection from the pool. 
 * Returns -1 if unsuccessful.
 **/
int dhcp_lnkconn(struct dhcp_t *this, struct dhcp_conn_t **conn) {

  if (!this->firstfreeconn) {

    if (connections == _options.max_clients) {
      log_err(0, "reached max connections %d!",
	      _options.max_clients);
       return -1;
     }

    ++connections;
    
    if (!(*conn = calloc(1, sizeof(struct dhcp_conn_t)))) {
      log_err(0, "Out of memory!");
      return -1;
    }

  } else {

    *conn = this->firstfreeconn;

    /* Remove from link of free */
    if (this->firstfreeconn->next) {
      this->firstfreeconn->next->prev = NULL;
      this->firstfreeconn = this->firstfreeconn->next;
    }
    else { /* Took the last one */
      this->firstfreeconn = NULL; 
      this->lastfreeconn = NULL;
    }
    
    /* Initialise structures */
    memset(*conn, 0, sizeof(struct dhcp_conn_t));
  }

  /* Insert into link of used */
  if (this->firstusedconn) {
    this->firstusedconn->prev = *conn;
    (*conn)->next = this->firstusedconn;
  }
  else { /* First insert */
    this->lastusedconn = *conn;
  }
  
  this->firstusedconn = *conn;

  return 0; /* Success */
}

int dhcp_getconn(struct dhcp_t *this, 
		 struct dhcp_conn_t **conn, 
		 uint8_t *mac, uint8_t *pkt,
		 char do_alloc) {

  if (dhcp_hashget(this, conn, mac)) {
    
    if (!do_alloc)
      return -1; 

    if (dhcp_newconn(this, conn, mac)) 
      return -1;
  }

  if (!*conn) 
    return -1;

#ifdef ENABLE_IEEE8021Q
  if (_options.ieee8021q) {
    if (pkt) {
#if(_debug_ > 1)
      log_dbg("%s calling dhcp_checktag",__FUNCTION__);
#endif
      dhcp_checktag(*conn, pkt);
    }
  }
#endif

  return 0;
}

/**
 * dhcp_newconn()
 * Allocates a new connection from the pool. 
 * Returns -1 if unsuccessful.
 **/
int dhcp_newconn(struct dhcp_t *this, 
		 struct dhcp_conn_t **conn, 
		 uint8_t *hwaddr)
{
  if (_options.debug) 
    log_dbg("DHCP newconn: %.2x:%.2x:%.2x:%.2x:%.2x:%.2x", 
	    hwaddr[0], hwaddr[1], hwaddr[2],
	    hwaddr[3], hwaddr[4], hwaddr[5]);

  if (dhcp_lnkconn(dhcp, conn) != 0)
    return -1;
  
  (*conn)->inuse = 1;
  (*conn)->parent = this;
  (*conn)->mtu = this->mtu;
  (*conn)->noc2c = this->noc2c;
  
  /* Application specific initialisations */
  memcpy((*conn)->hismac, hwaddr, PKT_ETH_ALEN);
  /*memcpy((*conn)->ourmac, dhcp_nexthop(this), PKT_ETH_ALEN);*/
  
  (*conn)->lasttime = mainclock_now();
  
  dhcp_hashadd(this, *conn);

#ifdef ENABLE_LAYER3
  if (_options.layer3) {
    (*conn)->authstate = DHCP_AUTH_ROUTER;
    (*conn)->dns1.s_addr = _options.dns1.s_addr;
    (*conn)->dns2.s_addr = _options.dns2.s_addr;
  }
  else 
#endif
    /* Inform application that connection was created */
    if (this->cb_connect)
      this->cb_connect(*conn);

  return 0; /* Success */
}

uint8_t * dhcp_nexthop(struct dhcp_t *this) {
#ifdef ENABLE_TAP
  if (_options.usetap && _options.has_nexthop) 
    return _options.nexthop;
#endif
  return this->rawif[0].hwaddr;
}


/**
 * dhcp_freeconn()
 * Returns a connection to the pool. 
 **/
int dhcp_freeconn(struct dhcp_conn_t *conn, int term_cause)
{
  /* TODO: Always returns success? */

  struct dhcp_t *this = conn->parent;

  /* Tell application that we disconnected */
  if (this->cb_disconnect)
    this->cb_disconnect(conn, term_cause);

  if (conn->is_reserved)
    return 0;

  log_dbg("DHCP freeconn: %.2x:%.2x:%.2x:%.2x:%.2x:%.2x", 
	  conn->hismac[0], conn->hismac[1], conn->hismac[2],
	  conn->hismac[3], conn->hismac[4], conn->hismac[5]);

#ifdef HAVE_NETFILTER_COOVA
  if (_options.kname) {
    kmod_coova_release(conn);
  }
#endif

  /* Application specific code */
  /* First remove from hash table */
  dhcp_hashdel(this, conn);

  /* Remove from link of used */
  if ((conn->next) && (conn->prev)) {
    conn->next->prev = conn->prev;
    conn->prev->next = conn->next;
  }
  else if (conn->next) { /* && prev == 0 */
    conn->next->prev = NULL;
    this->firstusedconn = conn->next;
  }
  else if (conn->prev) { /* && next == 0 */
    conn->prev->next = NULL;
    this->lastusedconn = conn->prev;
  }
  else { /* if ((next == 0) && (prev == 0)) */
    this->firstusedconn = NULL;
    this->lastusedconn = NULL;
  }

  /* Initialise structures */
  memset(conn, 0, sizeof(*conn));

  /* Insert into link of free */
  if (this->firstfreeconn) {
    this->firstfreeconn->prev = conn;
  }
  else { /* First insert */
    this->lastfreeconn = conn;
  }

  conn->next = this->firstfreeconn;
  this->firstfreeconn = conn;

  return 0;
}


/**
 * dhcp_checkconn()
 * Checks connections to see if the lease has expired
 **/
int dhcp_checkconn(struct dhcp_t *this) {
  struct dhcp_conn_t *conn = this->firstusedconn;

  while (conn) {
    /*
    if (_options.debug)
      log_dbg("dhcp_checkconn: %d %d", mainclock_diff(conn->lasttime), (int) this->lease);
    */
    struct dhcp_conn_t *check_conn = conn;
    conn = conn->next;
    if (!check_conn->is_reserved && 
	mainclock_diff(check_conn->lasttime) > (int)this->lease + _options.leaseplus) {
      log_dbg("DHCP timeout: Removing connection");
      dhcp_freeconn(check_conn, RADIUS_TERMINATE_CAUSE_LOST_CARRIER);
    }
  }

  return 0;
}

#ifdef HAVE_NETFILTER_QUEUE
static 
int nfqueue_cb_in(struct nfq_q_handle *qh, struct nfgenmsg *nfmsg,
		  struct nfq_data *nfa, void *cbdata) {
  struct nfqnl_msg_packet_hdr *ph;
  struct nfqnl_msg_packet_hw *hw;
  u_int32_t id = 0;
  char *data;
  int ret;

  int result = NF_DROP;

  hw = nfq_get_packet_hw(nfa);
  ph = nfq_get_msg_packet_hdr(nfa);

  if (ph) {
    id = ntohl(ph->packet_id);
  }
  
  /*
    mark = nfq_get_nfmark(nfa);
    ifi = nfq_get_indev(nfa);
    ifi = nfq_get_outdev(nfa);
  */
  
  ret = nfq_get_payload(nfa, &data);

  if (hw && ret > 0) {
    struct pkt_ipphdr_t *pack_iph  = (struct pkt_ipphdr_t *)data;
    struct pkt_tcphdr_t *pack_tcph = (struct pkt_tcphdr_t *)(data + PKT_IP_HLEN);
    struct pkt_udphdr_t *pack_udph = (struct pkt_udphdr_t *)(data + PKT_IP_HLEN);
    struct dhcp_conn_t *conn;
    struct in_addr addr;

    if (!dhcp_hashget(dhcp, &conn, hw->hw_addr)) {
      struct app_conn_t *appconn = (struct app_conn_t *)conn->peer;
      if (appconn->s_state.authenticated) {
	if (chilli_acct_fromsub(appconn, pack_iph))
	  result = NF_DROP;
	else
	  result = NF_ACCEPT;
      }
    }

    if (_options.debug) {
      addr.s_addr = pack_iph->saddr;
      log_dbg("NFQUEUE: From %.2X-%.2X-%.2X-%.2X-%.2X-%.2X %s %s", 
	      hw->hw_addr[0],hw->hw_addr[1],hw->hw_addr[2],
	      hw->hw_addr[3],hw->hw_addr[4],hw->hw_addr[5],
	      inet_ntoa(addr), 
	      result == NF_ACCEPT ? "Accept" : "Drop");
      
      addr.s_addr = pack_iph->daddr;
      log_dbg("NFQUEUE: To %s %s %d %s", 
	      inet_ntoa(addr), 
	      pack_iph->protocol == PKT_IP_PROTO_UDP ? "UDP" : 
	      pack_iph->protocol == PKT_IP_PROTO_TCP ? "TCP" : "Other",
	      pack_iph->protocol == PKT_IP_PROTO_UDP ? ntohs(pack_udph->dst) : 
	      pack_iph->protocol == PKT_IP_PROTO_TCP ? ntohs(pack_tcph->dst) : 0,
	      result == NF_ACCEPT ? "Accept" : "Drop");
    }
  }
  
  return nfq_set_verdict(qh, id, result, 0, NULL);
}

static 
int nfqueue_cb_out(struct nfq_q_handle *qh, struct nfgenmsg *nfmsg,
		   struct nfq_data *nfa, void *cbdata) {
  struct nfqnl_msg_packet_hdr *ph;
  u_int32_t id = 0;
  char *data;
  int ret;

  int result = NF_DROP;

  ph = nfq_get_msg_packet_hdr(nfa);

  if (ph) {
    id = ntohl(ph->packet_id);
  }
  
  ret = nfq_get_payload(nfa, &data);

  if (ret > 0) {
    struct pkt_ipphdr_t *pack_iph  = (struct pkt_ipphdr_t *)data;
    struct pkt_tcphdr_t *pack_tcph = (struct pkt_tcphdr_t *)(data + PKT_IP_HLEN);
    struct pkt_udphdr_t *pack_udph = (struct pkt_udphdr_t *)(data + PKT_IP_HLEN);
    struct in_addr addr;

    struct ippoolm_t *ipm;
    struct app_conn_t *appconn;

    result = NF_DROP;

    addr.s_addr = pack_iph->daddr;

    if (ippool_getip(ippool, &ipm, &addr)) {
      
      if (_options.debug) 
	log_dbg("dropping packet with unknown destination: %s", inet_ntoa(addr));
      
    }
    else {
      
      if ((appconn = (struct app_conn_t *)ipm->peer) == NULL ||
	  (appconn->dnlink) == NULL) {
	log_err(0, "No %s protocol defined for %s", appconn ? "dnlink" : "peer", inet_ntoa(addr));
      }
      else {
	
	if (appconn->s_state.authenticated == 1) {
	  if (chilli_acct_tosub(appconn, pack_iph))
	    result = NF_DROP;
	  else
	    result = NF_ACCEPT;
	}
      }
    }

    if (_options.debug) {
      addr.s_addr = pack_iph->saddr;
      log_dbg("NFQUEUE OUT: From %s %s", 
	      inet_ntoa(addr), 
	      result == NF_ACCEPT ? "Accept" : "Drop");
      
      addr.s_addr = pack_iph->daddr;
      log_dbg("NFQUEUE OUT: To %s %s %d %s", 
	      inet_ntoa(addr), 
	      pack_iph->protocol == PKT_IP_PROTO_UDP ? "UDP" : 
	      pack_iph->protocol == PKT_IP_PROTO_TCP ? "TCP" : "Other",
	      pack_iph->protocol == PKT_IP_PROTO_UDP ? ntohs(pack_udph->dst) : 
	      pack_iph->protocol == PKT_IP_PROTO_TCP ? ntohs(pack_tcph->dst) : 0,
	      result == NF_ACCEPT ? "Accept" : "Drop");
    }
  }
  
  return nfq_set_verdict(qh, id, result, 0, NULL);
}
#endif

/**
 * dhcp_new()
 * Allocates a new instance of the library
 **/
int dhcp_new(struct dhcp_t **pdhcp, int numconn, int hashsize,
	     char *interface, int usemac, uint8_t *mac, int promisc, 
	     struct in_addr *listen, int lease, int allowdyn,
	     struct in_addr *uamlisten, uint16_t uamport, 
	     int noc2c) {
  struct dhcp_t *dhcp;
  
  if (!(dhcp = *pdhcp = calloc(sizeof(struct dhcp_t), 1))) {
    log_err(0, "calloc() failed");
    return -1;
  }

  if (net_init(&dhcp->rawif[0], interface, 
#ifdef ETH_P_ALL
	       ETH_P_ALL, 
#else
	       0,
#endif
	       promisc, usemac ? mac : 0) < 0) {
    free(dhcp);
    return -1; 
  }

#ifdef ENABLE_MULTILAN
  {
    int idx, i;
    for (i=0, idx=1; i < MAX_MOREIF &&
	   _options.moreif[i].dhcpif; i++, idx++) {
      if (net_init(&dhcp->rawif[idx], _options.moreif[i].dhcpif,
#ifdef ETH_P_ALL
		   ETH_P_ALL, 
#else
		   0,
#endif
		   promisc, 0) < 0) {
	log_err(errno, "could not setup interface %s",
		_options.moreif[i].dhcpif);
      } else {
	log_dbg("Configured interface %s/%s fd=%d",
		_options.moreif[i].dhcpif, 
		_options.moreif[i].vlan, 
		dhcp->rawif[idx].fd);
      }
    }
  }
#endif

#ifdef HAVE_NETFILTER_QUEUE
  if (getenv("NFQUEUE_IN") && getenv("NFQUEUE_OUT")) {
    int q1 = 0, q2 = 1;
    char *e;
    if ((e = getenv("NFQUEUE_IN"))) q1 = atoi(e);
    if ((e = getenv("NFQUEUE_OUT"))) q2 = atoi(e);
    if (net_open_nfqueue(&dhcp->qif_in, q1, nfqueue_cb_in) == -1) {
      return -1;
    }
    if (net_open_nfqueue(&dhcp->qif_out, q2, nfqueue_cb_out) == -1) {
      return -1;
    }
  }
#endif

#if defined (__FreeBSD__) || defined (__APPLE__) || defined (__OpenBSD__)
  { 
    int blen=0;
    if (ioctl(dhcp->rawif[0].fd, BIOCGBLEN, &blen) < 0) {
      log_err(errno,"ioctl() failed!");
    }
    log_dbg("BIOCGBLEN=%d",blen);
    dhcp->pb.buflen = blen;
    if (!(dhcp->pb.buf = calloc(dhcp->pb.buflen, 1))) {
      log_err(errno, "malloc() failed");
    }
  }
#endif
  
  if (_options.dhcpgwip.s_addr != 0) {
    struct sockaddr_in addr;
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    int on = 1;
    
    if (fd > 0) {

      memset(&addr, 0, sizeof(addr));
      addr.sin_family = AF_INET;
      if (_options.dhcprelayip.s_addr) 
	addr.sin_addr.s_addr = _options.dhcprelayip.s_addr;
      else
	addr.sin_addr.s_addr = _options.uamlisten.s_addr;

      /*
       * ====[http://tools.ietf.org/id/draft-ietf-dhc-implementation-02.txt]====
       * 4.7.2 Relay Agent Port Usage
       *    Relay agents should use port 67 as the source port number.  Relay
       *    agents always listen on port 67, but port 68 has sometimes been used
       *    as the source port number probably because it was copied from the
       *    source port of the incoming packet.
       * 
       *    Cable modem vendors would like to install filters blocking outgoing
       *    packets with source port 67.
       * 
       *    RECOMMENDATIONS:
       *      O  Relay agents MUST use 67 as their source port number.
       *      O  Relay agents MUST NOT forward packets with non-zero giaddr
       *         unless the source port number on the packet is 67.
       */

      addr.sin_port = htons(_options.dhcpgwport);
      
      if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0) {
	log_err(errno, "Can't set reuse option");
      }
      
      if (bind(fd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
	log_err(errno, "socket or bind failed for dhcp relay!");
	close(fd);
	fd = -1;
      }
    }
      
    if (fd > 0) {
      dhcp->relayfd = fd;
    } else {
      close(dhcp->rawif[0].fd);
      free(dhcp);
      return -1;
    }
  }

  if (dhcp_hashinit(dhcp, hashsize))
    return -1; /* Failed to allocate hash tables */

  /* Initialise various variables */
  dhcp->ourip.s_addr = listen->s_addr;
  dhcp->lease = lease;
  dhcp->allowdyn = allowdyn;
  dhcp->uamlisten.s_addr = uamlisten->s_addr;
  dhcp->uamport = uamport;
  dhcp->mtu = _options.mtu;
  dhcp->noc2c = noc2c;
  dhcp->numconn = numconn;

  /* Initialise call back functions */
  dhcp->cb_data_ind = 0;
  dhcp->cb_request = 0;
  dhcp->cb_disconnect = 0;
  dhcp->cb_connect = 0;

  dhcp_sendGARP(dhcp, -1);

#ifdef ENABLE_IPV6
  net_getip6(dhcp->rawif[0].devname, &dhcp->rawif[0].address_v6);

  if (_options.debug) {
    /*
    struct ifaddrs *ifaddr, *ifa;
    int family, s;
    char host[NI_MAXHOST];
    */
    char fmt[1024];

    inet_ntop(AF_INET6, &dhcp->rawif[0].address_v6.s6_addr,
	      fmt, sizeof(fmt));
    
    log_dbg("dhcpif (%s) IPv6 address %s", 
	    dhcp->rawif[0].devname, fmt);
    
    /*
    if (getifaddrs(&ifaddr) == 0) {
      for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
	if (ifa->ifa_addr) {
	  family = ifa->ifa_addr->sa_family;
	  
	  printf("%s  address family: %d%s\n",
		 ifa->ifa_name, family,
		 (family == AF_PACKET) ? " (AF_PACKET)" :
		 (family == AF_INET) ?   " (AF_INET)" :
		 (family == AF_INET6) ?  " (AF_INET6)" : "");
	  
	  if (family == AF_INET || family == AF_INET6) {
	    s = getnameinfo(ifa->ifa_addr,
			    (family == AF_INET) ? sizeof(struct sockaddr_in) :
			    sizeof(struct sockaddr_in6),
			    host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
	    if (s != 0) {
	      printf("getnameinfo() failed: %s\n", gai_strerror(s));
	    }
	    printf("\taddress: <%s>\n", host);
	  }
	}
      }
      freeifaddrs(ifaddr);
    }
    */
  }
#endif

#ifdef HAVE_PATRICIA
  if (_options.patricia) {
    dhcp->ptree = patricia_new(32);
    dhcp->ptree_dyn = patricia_new(32);
  }
#endif

  return 0;
}

int dhcp_reopen() {
  return 0;
}

int dhcp_reserve_ip(uint8_t *mac, struct in_addr *ip) {
  struct dhcp_conn_t *conn = 0;

  if (dhcp_hashget(dhcp, &conn, mac)) {
    if (dhcp_newconn(dhcp, &conn, mac)) {
      log_err(0, "could not allocate connection");
      return -1;
    }
  }

  conn->is_reserved = 1;
  dhcp->cb_request(conn, ip, 0, 0);

  return 0;
}

int dhcp_reserve_str(char *b, size_t blen) {
  uint8_t mac[PKT_ETH_ALEN];
  unsigned int tmp[PKT_ETH_ALEN];
  struct in_addr ip;
  int state = 0;
  int lineno = 1;
  int newline;
  
  char *bp = b;
  char *t;
  int i;
  
  for (i=0; state >= 0 && i < blen; i++) {
    newline = 0;
    switch(b[i]) {
    case '#':
      {
        while (i < blen && b[i]!='\n')
          i++;
        lineno++;
        break;
      }
    case '\n':
        lineno++;
    case '\r': case ',':
      {
        if (state==-1)
            state = 0;
	newline = 1;
      }
    case ' ': case '\t': case '=':
      {
	b[i]=0;
	switch (state) {
	case 0:
	  {
	    for (t=bp; *t; t++) 
	      if (!isxdigit((int) *t)) 
		*t = 0x20;
	    
	    if (sscanf (bp, "%2x %2x %2x %2x %2x %2x", 
			&tmp[0], &tmp[1], &tmp[2], 
			&tmp[3], &tmp[4], &tmp[5]) != 6) {	
	      log_err(0, "Parse error in ethers file at line %d", lineno);
	      state = -1;
	    } else {
	      mac[0] = (uint8_t) tmp[0];
	      mac[1] = (uint8_t) tmp[1];
	      mac[2] = (uint8_t) tmp[2];
	      mac[3] = (uint8_t) tmp[3];
	      mac[4] = (uint8_t) tmp[4];
	      mac[5] = (uint8_t) tmp[5];
	      state = 1;
	    }
	  }
	  break;
	case 1:
	  {
	    if (!inet_aton(bp, &ip)) {	
	      log_err(0, "Bad IP address!");
	      state = -1;
	    } else {
	      state = 2;
	    }
	  }
	  break;
	}
	
	if (newline || state == 2) {
            log(LOG_NOTICE, "Reserving IP MAC=%.2X-%.2X-%.2X-%.2X-%.2X-%.2X IP %s" , 
                mac[0], mac[1], 
                mac[2], mac[3],
                mac[4], mac[5], 
                inet_ntoa(ip));
	  dhcp_reserve_ip(mac, &ip);
	  state = 0;
	}
	
	while (i < blen && (b[i] == 0 || 
			    b[i] == '\r' || 
			    b[i] == '\n' || 
			    b[i] == ' ' || 
			    b[i] == '\t')) {
            if (b[i]=='\n')
                lineno++;
            i++;
        }
	bp = b + (size_t) i;
	i--;
      }
      break;
    }
  }

  return 0;
}

/**
 * dhcp_set()
 * Set dhcp parameters which can be altered at runtime.
 **/
int dhcp_set(struct dhcp_t *dhcp, char *ethers, int debug) {
  dhcp->debug = debug;
  dhcp->anydns = _options.uamanydns;

  if (ethers && *ethers) {
    int fd = open(ethers, O_RDONLY);
    if (fd > 0) {
      struct stat buf;
      int r, blen;
      char *b;

      fstat(fd, &buf);
      blen = buf.st_size;

      if (blen > 0) {
	b = malloc(blen);
	if (b) {
	  r = safe_read(fd, b, blen);
	  if (r == blen) {
	    dhcp_reserve_str(b, blen);
	  } else {
	    log_err(0, "bad ethers file %s", ethers);
	  }
	  free(b);
	}      
      }

      close(fd);
    } else {
      log_err(0, "could not open ethers file %s", ethers);
    }
  }

  return 0;
}

/**
 * dhcp_free()
 * Releases ressources allocated to the instance of the library
 **/
void dhcp_free(struct dhcp_t *dhcp) {
  struct dhcp_conn_t *conn, *c;

  if (!dhcp) return;
#if defined (__FreeBSD__) || defined (__APPLE__) || defined (__OpenBSD__)
  if (dhcp->pb.buf)
    free (dhcp->pb.buf);
#endif
#ifdef HAVE_PATRICIA
  if (dhcp->ptree)
    patricia_destroy (dhcp->ptree, free);
  if (dhcp->ptree_dyn)
    patricia_destroy (dhcp->ptree_dyn, free);
#endif
  if (dhcp->hash) 
    free(dhcp->hash);
  if (!_options.uid)
    dev_set_flags(dhcp->rawif[0].devname, 
		  dhcp->rawif[0].devflags);
  net_close(&dhcp->rawif[0]);

  for (conn = dhcp->firstfreeconn; conn; ) {
    c = conn;
    conn = conn->next;
    free(c);
  }

  for (conn = dhcp->firstusedconn; conn; ) {
    c = conn;
    conn = conn->next;
    free(c);
  }

  free(dhcp);
}

/**
 * dhcp_timeout()
 * Need to call this function at regular intervals to clean up old connections.
 **/
int dhcp_timeout(struct dhcp_t *this)
{
  /*dhcp_validate(this);*/

  dhcp_checkconn(this);
  
  return 0;
}

/**
 * dhcp_timeleft()
 * Use this function to find out when to call dhcp_timeout()
 * If service is needed after the value given by tvp then tvp
 * is left unchanged.
 **/
struct timeval* dhcp_timeleft(struct dhcp_t *this, struct timeval *tvp) {
  return tvp;
}

#ifdef ENABLE_IPWHITELIST
int dhcp_ipwhitelist(struct pkt_ipphdr_t *iph, unsigned char dst) {
  struct in_addr inp; 
  FILE *fp; 

  if ((fp = fopen(_options.ipwhitelist, "r")) == NULL) {
    log_err(errno, "error openning %s", _options.ipwhitelist);
    return 0;
  }
  
  while (fread(&inp, sizeof(inp), 1, fp) != 0) {
    if (inp.s_addr == (dst ? iph->daddr : iph->saddr)) {
      if (iph->protocol == PKT_IP_PROTO_TCP || 
	  iph->protocol == PKT_IP_PROTO_UDP) {
	log_dbg("DYNAMIC WHITELIST: %s\n", inet_ntoa(inp));
	fclose(fp);
	return 1;
      }
    }
  }

  fclose(fp);
  log_dbg("NO WHITELIST: %s", inet_ntoa(inp));
  return 0;
}
#endif

size_t icmpfrag(uint8_t *pack, size_t plen, uint8_t *orig_pack) {
  /*
   0                   1                   2                   3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |     Type      |     Code      |          Checksum             |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                             unused                            |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |      Internet Header + 64 bits of Original Data Datagram      |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

   Used when we recived a truncated (from recvfrom() where our buffer
   is smaller than IP packet length) IP packet.
  */

  size_t icmp_req_len = PKT_IP_HLEN + 64;

  size_t icmp_ip_len = PKT_IP_HLEN + sizeof(struct pkt_icmphdr_t) + 
    4 + icmp_req_len;

  size_t icmp_full_len = icmp_ip_len + sizeofeth(orig_pack);

  struct pkt_iphdr_t  *orig_pack_iph  = pkt_iphdr(orig_pack);
  struct pkt_ethhdr_t *orig_pack_ethh = pkt_ethhdr(orig_pack);

  if (icmp_full_len > plen) return 0;

  memset(pack, 0, icmp_full_len);
  copy_ethproto(orig_pack, pack);

  {
    struct pkt_ethhdr_t *pack_ethh  = pkt_ethhdr(pack);
    struct pkt_iphdr_t *pack_iph = pkt_iphdr(pack);
    struct pkt_icmphdr_t *pack_icmph = pkt_icmphdr(pack);

    /* eth */
    memcpy(pack_ethh->dst, orig_pack_ethh->src, PKT_ETH_ALEN); 
    memcpy(pack_ethh->src, orig_pack_ethh->dst, PKT_ETH_ALEN); 
    
    /* ip */
    pack_iph->saddr = orig_pack_iph->daddr;
    pack_iph->daddr = orig_pack_iph->saddr;
    pack_iph->protocol = PKT_IP_PROTO_ICMP;
    pack_iph->version_ihl = PKT_IP_VER_HLEN;
    pack_iph->ttl = 0x10;
    pack_iph->tot_len = htons(icmp_ip_len);
    
    pack_icmph->type = 3;
    pack_icmph->code = 4;

    memcpy(pack + (icmp_full_len - icmp_req_len), 
	   orig_pack + sizeofeth(orig_pack), icmp_req_len);
    
    chksum(pack_iph);
  }
  
  return icmp_full_len;
}
  
size_t tcprst(uint8_t *tcp_pack, uint8_t *orig_pack, char reverse) {

  size_t len = sizeofeth(orig_pack) + PKT_IP_HLEN + PKT_TCP_HLEN;

  struct pkt_iphdr_t  *orig_pack_iph  = pkt_iphdr(orig_pack);
  struct pkt_tcphdr_t *orig_pack_tcph = pkt_tcphdr(orig_pack);

  struct pkt_iphdr_t *tcp_pack_iph;
  struct pkt_tcphdr_t *tcp_pack_tcph;

  memcpy(tcp_pack, orig_pack, len); 

  tcp_pack_iph = pkt_iphdr(tcp_pack);
  tcp_pack_tcph = pkt_tcphdr(tcp_pack);
  
  if (reverse) {
    struct pkt_ethhdr_t *tcp_pack_ethh  = pkt_ethhdr(tcp_pack);
    struct pkt_ethhdr_t *orig_pack_ethh = pkt_ethhdr(orig_pack);

    /* eth */
    memcpy(tcp_pack_ethh->dst, orig_pack_ethh->src, PKT_ETH_ALEN); 
    memcpy(tcp_pack_ethh->src, orig_pack_ethh->dst, PKT_ETH_ALEN); 

    /* ip */
    tcp_pack_iph->saddr = orig_pack_iph->daddr;
    tcp_pack_iph->daddr = orig_pack_iph->saddr;
    
    /* tcp */
    tcp_pack_tcph->src = orig_pack_tcph->dst;
    tcp_pack_tcph->dst = orig_pack_tcph->src;
    tcp_pack_tcph->seq = htonl(ntohl(orig_pack_tcph->seq)+1);
  }

  tcp_pack_iph->tot_len = htons(PKT_IP_HLEN + PKT_TCP_HLEN);

  tcp_pack_tcph->flags = TCPHDR_FLAG_RST;
  tcp_pack_tcph->offres = 0x50;

  chksum(tcp_pack_iph);

  return len;
}

#ifdef ENABLE_TCPRESET
static
void tun_sendRESET(struct tun_t *tun, uint8_t *pack, struct app_conn_t *appconn) {
#if !defined(HAVE_NETFILTER_QUEUE) && !defined(HAVE_NETFILTER_COOVA)
  uint8_t tcp_pack[1500];
  tun_encaps(tun, tcp_pack, tcprst(tcp_pack, pack, 1), appconn->s_params.routeidx);
#endif
}

static
void dhcp_sendRESET(struct dhcp_conn_t *conn, uint8_t *pack, char reverse) {
#if !defined(HAVE_NETFILTER_QUEUE) && !defined(HAVE_NETFILTER_COOVA)
  uint8_t tcp_pack[1500];
  struct dhcp_t *this = conn->parent;
  dhcp_send(this, dhcp_conn_idx(conn), conn->hismac, tcp_pack, tcprst(tcp_pack, pack, reverse));
#endif
}
#endif

static
int dhcp_nakDNS(struct dhcp_conn_t *conn, uint8_t *pack, size_t len) {
  struct dhcp_t *this = conn->parent;
  struct pkt_ethhdr_t *ethh = pkt_ethhdr(pack);
  struct pkt_iphdr_t *iph = pkt_iphdr(pack);
  struct pkt_udphdr_t *udph = pkt_udphdr(pack);

  uint8_t answer[1500];

  struct pkt_ethhdr_t *answer_ethh;
  struct pkt_iphdr_t *answer_iph;
  struct pkt_udphdr_t *answer_udph;
  struct dns_packet_t *answer_dns;

  memcpy(answer, pack, len); 

  answer_ethh = pkt_ethhdr(answer);
  answer_iph  = pkt_iphdr(answer);
  answer_udph = pkt_udphdr(answer);
  answer_dns  = pkt_dnspkt(answer);

  /* DNS response, with no host error code */
  answer_dns->flags = htons(0x8583); 
  
  /* UDP */
  answer_udph->src = udph->dst;
  answer_udph->dst = udph->src;
  
  /* IP */
  answer_iph->check = 0; /* Calculate at end of packet */      
  memcpy(&answer_iph->daddr, &iph->saddr, PKT_IP_ALEN);
  memcpy(&answer_iph->saddr, &iph->daddr, PKT_IP_ALEN);
  
  /* Ethernet */
  memcpy(&answer_ethh->dst, &ethh->src, PKT_ETH_ALEN);
  memcpy(&answer_ethh->src, &ethh->dst, PKT_ETH_ALEN);
    
  /* checksums */
  chksum(answer_iph);
  
  OTHER_SENDING(conn, answer_iph);
  dhcp_send(this, dhcp_conn_idx(conn), conn->hismac, answer, len);
  return 0;
}

static 
int dhcp_matchDNS(uint8_t *r, char *name) {
  int r_len = strlen((char *)r);
  int name_len = strlen(name);
  int domain_len = _options.domain ? strlen(_options.domain) : 0;

#if(_debug_ > 1)
  log_dbg("---------  checking dns for %s in %s", name, r);
#endif
  
  if (r_len == name_len && !memcmp(r, name, name_len)) {
    return 1;
  }
  
  if (domain_len > 0 && 
      r_len == (name_len + domain_len + 1) &&
      !memcmp(r, name, name_len) &&
      !memcmp(r + name_len + 1, _options.domain, domain_len) &&
      r[name_len] == '.') {
    return 1;
  }
  
  return 0;
}

/*
 *   dhcp_dns() - Checks DNS for bad packets or locally handled DNS.
 *   returns: 0 = do not forward, 1 = forward DNS
 */
static 
int dhcp_dns(struct dhcp_conn_t *conn, uint8_t *pack, 
	     size_t *plen, char isReq) {

  if (*plen < DHCP_DNS_HLEN + sizeofudp(pack)) {
    
    log_dbg("bad DNS packet of length %d", *plen);
    return 0;
    
  } else {
    
#if defined(ENABLE_DNSLOG) || defined(ENABLE_MODULES)
    struct app_conn_t *appconn = dhcp_get_appconn_pkt(conn, pkt_iphdr(pack), !isReq);
#endif
    struct dns_packet_t *dnsp = pkt_dnspkt(pack);
    
    size_t dlen = *plen - DHCP_DNS_HLEN - sizeofudp(pack);
    size_t olen = dlen;
    
    uint16_t flags   = ntohs(dnsp->flags);
    uint16_t qdcount = ntohs(dnsp->qdcount);
    uint16_t ancount = ntohs(dnsp->ancount);
    uint16_t nscount = ntohs(dnsp->nscount);
    uint16_t arcount = ntohs(dnsp->arcount);
    
    uint8_t *dptr = (uint8_t *)dnsp->records;
    uint8_t q[512];

#ifdef ENABLE_IPV6
    uint8_t *an_mark = 0;
#endif

    int mode = 0;
    int qmatch = -1;
    int mod = -1;
    int i;

#ifdef ENABLE_MDNS
    struct pkt_udphdr_t *udph = pkt_udphdr(pack);
    char isMDNS = 0;
#endif
    
#if(_debug_ > 1)
    uint16_t id      = ntohs(dnsp->id);

    log_dbg("dhcp_dns plen=%d dlen=%d olen=%d", *plen, dlen, olen);
    log_dbg("DNS ID:    %d", id);
    log_dbg("DNS Flags: %d", flags);
#endif

#ifdef ENABLE_MODULES
    { int i, res=0;
      for (i=0; i < MAX_MODULES; i++) {
	if (!_options.modules[i].name[0]) break;
	if (_options.modules[i].ctx) {
	  struct chilli_module *m = 
	    (struct chilli_module *)_options.modules[i].ctx;
	  if (m->dns_handler)
	    res = m->dns_handler(appconn, conn, 
				 pack, plen, isReq); 
	  switch (res) {
	    case CHILLI_DNS_MOD:
	      mod = 1;
	      break;
	  case CHILLI_DNS_NAK:
	    return dhcp_nakDNS(conn,pack,*plen);
	  case CHILLI_DNS_ERR:
	  case CHILLI_DNS_DROP:
	    return 0;
	  }
	}
      }
    }
#endif

#ifdef ENABLE_MDNS
    isMDNS = (udph->dst == udph->src && 
	      udph->src == htons(DHCP_MDNS));

    if (isMDNS) {
      mode = DNS_MDNS_MODE;
    } else {
#endif
    
    if (!isReq) {
      /* it was a query? shouldn't be */
      if (((flags & 0x8000) >> 15) == 0) {
#if(_debug_)
	log_dbg("Dropping unexpected DNS query");
#endif
	return 0;
      }
    } else {
      /* it was a response? shouldn't be */
      if (((flags & 0x8000) >> 15) == 1) {
#if(_debug_)
	log_dbg("Dropping unexpected DNS response");
#endif
	return 0;
      }
    }

#ifdef ENABLE_MDNS
    }
#endif
    
    memset(q, 0, sizeof(q));
    
#undef  copyres
#define copyres(isq,n)			        \
    for (i=0; dlen && i < n ## count; i++) {		\
      if (dns_copy_res(conn, isq, &dptr, &dlen,		\
		       (uint8_t *)dnsp, olen,		\
		       q, sizeof(q), isReq,		\
		       &qmatch, &mod, mode)) {		\
        log_dbg("dropping malformed DNS");		\
	return isReq ? dhcp_nakDNS(conn,pack,*plen) : 0; \
      } \
    }
    
    copyres(1,qd);
#ifdef ENABLE_IPV6
    an_mark = dptr;
#endif
    copyres(0,an);
    copyres(0,ns);
    copyres(0,ar);

#if(_debug_ > 1)
    log_dbg("left (should be zero): %d q=%s", dlen, q);
#endif
    
    if (dlen) {
#if(_debug_)
      log_dbg("remaining length not zero dlen=%d", dlen);
#endif
      return 0;
    }

#ifdef ENABLE_MDNS
    if (isMDNS) {
      if (flags   == 0x0000 && 
	  qdcount >= 0x0001 &&
	  ancount == 0x0000 && 
	  nscount == 0x0000 &&
	  arcount == 0x0000) {
	log_dbg("MDNS Query %s", q);
      }
      else if (flags == 0x8400 &&
	       qdcount == 0x0000 &&
	       ancount > 0) {
	log_dbg("MDNS Response %s", q);
      }
    }
#endif
    
    if (isReq && 
#ifdef ENABLE_MDNS
	(flags  == 0x0100 || flags  == 0x0000) && 
#else
	flags   == 0x0100 && 
#endif
	qdcount >= 0x0001 &&
	ancount == 0x0000 && 
	nscount == 0x0000 &&
	arcount == 0x0000) {
      
      char *hostname = _options.uamhostname;
      char *aliasname = _options.uamaliasname;
      
      uint8_t *p = dnsp->records;
      
      uint8_t query[256];
      uint8_t reply[4];

      int match = 0;

      if (!match) {
	match = dhcp_matchDNS(q, "logout");
	if (match) {
	  memcpy(reply, &_options.uamlogout.s_addr, 4);
	}
      }
      // Added for iphone
#if 0 
      if (!match) {
	match = dhcp_matchDNS(q, "captive.apple.com");
	if (match) {
		struct   in_addr   apple_addr;
   	        apple_addr.s_addr= inet_addr("192.168.182.1");
  		memcpy(reply, &apple_addr.s_addr, 4);
	}
      }
#endif

// ---> check wan status
      if (!match) {
        int wan_unit = 0;
	int wan_conn_flag = 0;
	for (wan_unit = WAN_UNIT_FIRST; wan_unit < WAN_UNIT_MAX; ++wan_unit) {
	  if (is_wan_connect(wan_unit)) {
            wan_conn_flag = 1;
	    break;
	  }
	}

	if (!wan_conn_flag) {
	  match = 1;
	  memcpy(reply, &_options.uamalias.s_addr, 4);
	}
      }
// ---> check wan status
      if (!match && hostname) {
	match = dhcp_matchDNS(q, hostname);
	if (match) {
	  memcpy(reply, &_options.uamlisten.s_addr, 4);
	}
      }
      
      if (!match && aliasname) {
	match = dhcp_matchDNS(q, aliasname);
	if (match) {
	  memcpy(reply, &_options.uamalias.s_addr, 4);
	}
      }
      
      if (!match && _options.domaindnslocal && _options.domain) {
	//int name_len = strlen((char *)q);
	int domain_len = strlen(_options.domain);
	
	int unit=0;
	int conn_flag=0;
	for (unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit){
		if(is_wan_connect(unit)){
			conn_flag=1;
			break;
		}
	}
      	if(!conn_flag){
	    domain_len=0;
	}	   

//	if (name_len >= (domain_len + 1)) {
//	  int off = name_len - domain_len;
	  
	  //if (!memcmp(q + off, _options.domain, domain_len) && q[off - 1] == '.') {
	  if (!memcmp(q, _options.domain, domain_len)) {
	      printf("option_domain=%s\n", _options.domain); 
	    /*
	     * count (recent) dns requests vs responses to get an
	     * overall picture of on-line status.
	     */
	    printf("authestate=%d\n", conn->authstate);
	    if(DHCP_AUTH_PASS == conn->authstate){
    	       struct   in_addr   my_addr;
	       char *lan_ip;
	       lan_ip=nvram_safe_get("lan_ipaddr"); 
   	       //my_addr.s_addr= inet_addr("192.168.1.1");
   	       my_addr.s_addr= inet_addr(lan_ip);
               memcpy(reply, &my_addr.s_addr, 4);
    	    }else  
	       memcpy(reply, &_options.uamalias.s_addr, 4);
	    match = 1;
	  }
//	}
      }
      
      if (match) {
	
	uint8_t answer[1500];
	
	struct pkt_ethhdr_t *ethh = pkt_ethhdr(pack);
	struct pkt_iphdr_t  *iph  = pkt_iphdr(pack);
	struct pkt_udphdr_t *udph = pkt_udphdr(pack);
	
	struct pkt_ethhdr_t *answer_ethh;
	struct pkt_iphdr_t  *answer_iph;
	struct pkt_udphdr_t *answer_udph;
	struct dns_packet_t *answer_dns;
	
	size_t query_len = 0;
	size_t udp_len;
	size_t length;
	
	int n;
	
	p = dnsp->records;
	
#if(_debug_)
	log_dbg("It was a matching query!\n");
#endif
	
	do {
	  if (query_len < 256)
	    query[query_len++] = *p;
	  else 
	    break;
	}
	while (*p++ != 0); /* TODO */
	
	for (n=0; n<4; n++) {
	  if (query_len < 256)
	    query[query_len++] = *p++;
	}
	
	query[query_len++] = 0xc0;
	query[query_len++] = 0x0c;
	query[query_len++] = 0x00;
	query[query_len++] = 0x01;
	query[query_len++] = 0x00;
	query[query_len++] = 0x01;
	query[query_len++] = 0x00;
	query[query_len++] = 0x00;
	query[query_len++] = 0x00;
	query[query_len++] = 0x00;
	query[query_len++] = 0x00;
	query[query_len++] = 0x04;
	memcpy(query + query_len, reply, 4);
	query_len += 4;
	
	memcpy(answer, pack, *plen); /* TODO */
	
	answer_ethh = pkt_ethhdr(answer);
	answer_iph = pkt_iphdr(answer);
	answer_udph = pkt_udphdr(answer);
	answer_dns = pkt_dnspkt(answer);
	
	/* DNS Header */
	answer_dns->id      = dnsp->id;
	answer_dns->flags   = htons(0x8000);
	answer_dns->qdcount = htons(0x0001);
	answer_dns->ancount = htons(0x0001);
	answer_dns->nscount = htons(0x0000);
	answer_dns->arcount = htons(0x0000);
	memcpy(answer_dns->records, query, query_len);
	
	/* UDP header */
	udp_len = query_len + DHCP_DNS_HLEN + PKT_UDP_HLEN;
	answer_udph->len = htons(udp_len);
	answer_udph->src = udph->dst;
	answer_udph->dst = udph->src;
	
	/* Ip header */
	answer_iph->version_ihl = PKT_IP_VER_HLEN;
	answer_iph->tos = 0;
	answer_iph->tot_len = htons(udp_len + PKT_IP_HLEN);
	answer_iph->id = 0;
	answer_iph->opt_off_high = 0;
	answer_iph->off_low = 0;
	answer_iph->ttl = 0x10;
	answer_iph->protocol = 0x11;
	answer_iph->check = 0; /* Calculate at end of packet */      
	memcpy(&answer_iph->daddr, &iph->saddr, PKT_IP_ALEN);
	memcpy(&answer_iph->saddr, &iph->daddr, PKT_IP_ALEN);
	
	/* Ethernet header */
	memcpy(answer_ethh->dst, &ethh->src, PKT_ETH_ALEN);
	memcpy(answer_ethh->src, &ethh->dst, PKT_ETH_ALEN);
	
	/* Work out checksums */
	chksum(answer_iph);
	
	/* Calculate total length */
	length = udp_len + sizeofip(answer);
	
	OTHER_SENDING(conn, answer_iph);
	dhcp_send(dhcp, dhcp_conn_idx(conn), conn->hismac, answer, length);
	return 0;
      }
    }
    
#ifdef ENABLE_DNSLOG
    if (isReq && _options.dnslog) {
      int fd = open(_options.dnslog, O_WRONLY|O_APPEND|O_CREAT, 0666);
      if (fd > 0) {
	char line[512];
	char *username = 0;
	int authenticated = 0;
	
	if (appconn) {
	  username = appconn->s_state.redir.username;
	  authenticated = appconn->s_state.authenticated;
	}
	
	safe_snprintf(line, sizeof(line),
		      "%d,%.2X-%.2X-%.2X-%.2X-%.2X-%.2X,%s,%s,%d,%s\n",
		      time(0),
		      conn->hismac[0], conn->hismac[1],
		      conn->hismac[2], conn->hismac[3],
		      conn->hismac[4], conn->hismac[5],
		      inet_ntoa(conn->hisip),
		      q, authenticated,
		      username ? username : "");
	
	safe_write(fd, line, strlen(line));
	close(fd);
      } else {
	log_err(errno, "could not open log file %s", _options.dnslog);
      }
    }
#endif

#ifdef ENABLE_IPV6
    if (_options.ipv6 && mod > 0 && !isReq && an_mark && ancount > 0) {
      /* repack as IPv6 AAAA addresses */
      uint8_t n[1500];
      uint8_t b[1500];
      uint8_t *p = an_mark, *bp = b, *bt = b;
      int new_ancount = 0;
      int len = olen - (an_mark - ((uint8_t *)dnsp->records));

      for (i=0; i < ancount; i++) {
	
	ssize_t namelen =  dns_fullname((char*)n, sizeof(n)-1, 
					p, len, (uint8_t *)dnsp, olen, 0);
	
	uint16_t type;
	uint16_t class;
	uint32_t ttl;
	uint16_t rdlen;
	
	uint8_t *pkt_head=p;
	//uint8_t *pkt_type=0;
	uint8_t *pkt_ttl=0;
	
	uint32_t ul;
	uint16_t us;
	
	log_dbg("namelen %d", namelen);

	if (namelen <= 0 || namelen > 1500)
	  break;

	if (len < 10) {
	  log_dbg("len too short %d",len);
	  break;
	}
	
	p += namelen;
	len -= namelen;
	
	//pkt_type = p;
	memcpy(&us, p, sizeof(us));
	type = ntohs(us);
	p += 2;
	len -= 2;
	
	memcpy(&us, p, sizeof(us));
	class = ntohs(us);
	p += 2;
	len -= 2;
	
	pkt_ttl = p;
	memcpy(&ul, p, sizeof(ul));
	ttl = ntohl(ul);
	p += 4;
	len -= 4;
	
	memcpy(&us, p, sizeof(us));
	rdlen = ntohs(us);
	p += 2;
	len -= 2;

	log_dbg("DNS t=%d c=%d ttl=%d dlen=%d",
		type, class, ttl, rdlen);

	switch(type) {
	case 1:
	  /* prefix */
	  memcpy(bp, pkt_head, namelen);
	  bp += namelen;
	  *bp++ = 0x00;
	  *bp++ = 0x1c;
	  *bp++ = 0x00;
	  *bp++ = 0x01;
	  *bp++ = pkt_ttl[0];
	  *bp++ = pkt_ttl[1];
	  *bp++ = pkt_ttl[2];
	  *bp++ = pkt_ttl[3];
	  *bp++ = 0x00;
	  *bp++ = 0x10;
	  ipv6_nat64_pack(bp, p);
	  break;
	default:
	  {
	    int l = rdlen + namelen + 10;
	    log_dbg("copy %d", l);
	    memcpy(bp, pkt_head, l);
	    bp += l;
	  }
	  break;
	}
	  
	p += rdlen;
	len -= rdlen;

	new_ancount++;
      }

      if (new_ancount != ancount) {
	log_dbg("Could not pack all answers %d/%d",
		ancount, new_ancount);
      }
      
      if (new_ancount > 0) {
	uint16_t bdiff = (uint16_t)(bp - bt);
	int newlen = (int)(an_mark - pack) + (int)bdiff;
	if (newlen < 0 || newlen > 1500) {
	  log_err(0, "Problem growing DNS packet from %d to %d (%d %d)",
		  *plen, newlen, (an_mark - pack), bdiff);
	} else {
	  uint16_t data_len = bdiff + (uint16_t) (an_mark - ((uint8_t *)dnsp));
	  struct pkt_iphdr_t  *iph  = pkt_iphdr(pack);
	  struct pkt_udphdr_t *udph = pkt_udphdr(pack);
	  log_dbg("Growing DNS packet from %d to %d (%d %d) dt=%d",
		  *plen, newlen, (an_mark - pack), bdiff, data_len);
	  data_len += PKT_UDP_HLEN;
	  udph->len = htons(data_len);
	  data_len += PKT_IP_HLEN;
	  iph->tot_len = htons(data_len);
	  dnsp->ancount = htons(new_ancount);
	  memcpy(an_mark, b, bdiff);
	  *plen = newlen;
	}
      }
    }
#endif

    if (mod > 0) {
      chksum(pkt_iphdr(pack));
    }
  }

  return 1;
}

static 
int dhcp_uam_nat(struct dhcp_conn_t *conn,
		 struct pkt_ethhdr_t *ethh,
		 struct pkt_iphdr_t  *iph,
		 struct pkt_tcphdr_t *tcph,
		 struct in_addr *addr, 
		 uint16_t port) {
  int n;
  int pos = -1;

#if(_debug > 1)
  log_dbg("uam_nat %s:%d", inet_ntoa(*addr), port);
#endif
  
  for (n=0; n < DHCP_DNAT_MAX; n++) {
    if (conn->dnat[n].src_ip == iph->saddr && 
	conn->dnat[n].src_port == tcph->src) {
      pos = n;
#if(_debug > 1)
      log_dbg("uam_nat pos %d", pos);
#endif
      break;
    }
  }
  
  if (pos == -1) {
    pos = conn->nextdnat;
#ifdef ENABLE_TAP
    if (_options.usetap) {
      memcpy(conn->dnat[pos].mac, ethh->dst, PKT_ETH_ALEN); 
    }
#endif
    conn->dnat[pos].src_ip = iph->saddr; 
    conn->dnat[pos].src_port = tcph->src;
    conn->nextdnat = (conn->nextdnat + 1) % DHCP_DNAT_MAX;
  }

  conn->dnat[pos].dst_ip = iph->daddr; 
  conn->dnat[pos].dst_port = tcph->dst;
  
#ifdef ENABLE_TAP
  if (_options.usetap) {
    memcpy(ethh->dst, tuntap(tun).hwaddr, PKT_ETH_ALEN); 
  }
#endif
  
  iph->daddr = addr->s_addr;
  tcph->dst = htons(port);
  
  chksum(iph);
  
  return 0;
}


static 
int dhcp_uam_unnat(struct dhcp_conn_t *conn,
		   struct pkt_ethhdr_t *ethh,
		   struct pkt_iphdr_t  *iph,
		   struct pkt_tcphdr_t *tcph) {
  int n;
  for (n=0; n < DHCP_DNAT_MAX; n++) {
    
    if (iph->daddr == conn->dnat[n].src_ip && 
	tcph->dst == conn->dnat[n].src_port) {
      
#ifdef ENABLE_TAP
      if (_options.usetap) {
	memcpy(ethh->src, conn->dnat[n].mac, PKT_ETH_ALEN);
      }
#endif
      
      iph->saddr = conn->dnat[n].dst_ip;
      tcph->src = conn->dnat[n].dst_port;
      
      chksum(iph);
      
      return 0; 
    }
  }
  return 0; 
}

static 
int dhcp_dnsDNAT(struct dhcp_conn_t *conn, 
		 uint8_t *pack, size_t *len, 
		 char *do_checksum) {
  
  struct dhcp_t *this = conn->parent;
  struct pkt_iphdr_t  *iph  = pkt_iphdr(pack);
  struct pkt_udphdr_t *udph = pkt_udphdr(pack);

#ifdef ENABLE_MDNS
  if (iph->protocol == PKT_IP_PROTO_UDP && 
      udph->src == htons(DHCP_MDNS) &&
      udph->dst == htons(DHCP_MDNS)) {
    log_dbg("mDNS packet");
    if (!dhcp_dns(conn, pack, len, 1)) {
#if(_debug_)
      log_dbg("dhcp_dns()");
#endif
    }
    return -1; /* Drop DNS */
  }
#endif

  /* Was it a DNS request? */
  if ((this->anydns ||
       iph->daddr == conn->dns1.s_addr ||
       iph->daddr == conn->dns2.s_addr) &&
      iph->protocol == PKT_IP_PROTO_UDP && 
      udph->dst == htons(DHCP_DNS)) {

    if (this->anydns) {
      if (
#ifdef ENABLE_LAYER3
	  ! _options.layer3 &&
#endif
	  iph->daddr != conn->dns1.s_addr && 
	  iph->daddr != conn->dns2.s_addr) {
	conn->dnatdns = iph->daddr;
	iph->daddr = conn->dns1.s_addr;
	*do_checksum = 1;
      } else {
	conn->dnatdns = 0;
      }
    }

    if (!dhcp_dns(conn, pack, len, 1)) {
#if(_debug_)
      log_dbg("dhcp_dns()");
#endif
      return -1; /* Drop DNS */
    }

    return 1; /* Is allowed DNS */
  }

  return 0; /* Not DNS */
}

static 
int dhcp_dnsunDNAT(struct dhcp_conn_t *conn, 
		   uint8_t *pack, size_t *len, 
		   char *do_checksum) {
  
  struct dhcp_t *this = conn->parent;
  struct pkt_iphdr_t *iph = pkt_iphdr(pack);
  struct pkt_udphdr_t *udph = pkt_udphdr(pack);

  /* Was it a DNS reply? */
  if ((this->anydns ||
       iph->saddr == conn->dns1.s_addr ||
       iph->saddr == conn->dns2.s_addr) &&
      iph->protocol == PKT_IP_PROTO_UDP && 
      udph->src == htons(DHCP_DNS)) {

    if (this->anydns && 
	conn->dnatdns &&
	iph->saddr != conn->dnatdns) {
      iph->saddr = conn->dnatdns;
      *do_checksum = 1;
    }

    if (!dhcp_dns(conn, pack, len, 0)) {
#if(_debug_)
      log_dbg("dhcp_dns()");
#endif
      return -1; /* Is not allowed DNS */
    }
    
    return 1; /* Is allowed DNS */
  }

  return 0; /* Not DNS */
}

int dhcp_garden_check(struct dhcp_t *this,
		      struct dhcp_conn_t *conn,
		      struct app_conn_t *appconn,
		      struct pkt_ipphdr_t *ipph,
		      int dst) {
  pass_through *pt=0;
  int found = 0;

#ifdef HAVE_PATRICIA
  if (dhcp->ptree && dhcp->ptree_dyn) {

    if (garden_patricia_check(dhcp->ptree, 
			      _options.pass_throughs, 
			      &_options.num_pass_throughs, 
			      ipph, dst))
      found = 1;

    if (!found && garden_patricia_check(dhcp->ptree_dyn, 
					this->pass_throughs, 
					&this->num_pass_throughs, 
					ipph, dst))
      found = 1;

  } else {
#endif

    switch (garden_check(_options.pass_throughs, 
			 &_options.num_pass_throughs, &pt,
			 ipph, dst
#ifdef HAVE_PATRICIA
			 , 0
#endif
			 )) {
    case 1:
      found = 1;
      break;
#ifdef ENABLE_GARDENEXT
    case -1:
      if (pt)
	pass_through_rem(_options.pass_throughs, 
			 &_options.num_pass_throughs, pt
#ifdef HAVE_PATRICIA
			 , 0
#endif
			 );
      break;
#endif
    }
    
    if (!found)
      switch (garden_check(this->pass_throughs, 
			   &this->num_pass_throughs, &pt,
			   ipph, dst
#ifdef HAVE_PATRICIA
			   , 0
#endif
			   )) {
      case 1:
	found = 1;
	break;
#ifdef ENABLE_GARDENEXT
      case -1:
	if (pt)
	  pass_through_rem(this->pass_throughs, 
			   &this->num_pass_throughs, pt
#ifdef HAVE_PATRICIA
			   , 0
#endif
			   );
	break;
#endif
      }
    
#ifdef HAVE_PATRICIA
  }
#endif
  
#ifdef ENABLE_SESSGARDEN
  if (!found) {
    if (!appconn) 
      appconn = dhcp_get_appconn_pkt(conn, 
				     (struct pkt_iphdr_t *)ipph, 
				     !dst);
    if (appconn) {
#ifdef HAVE_PATRICIA
      if (appconn->ptree) {

	if (garden_patricia_check(appconn->ptree, 
				  appconn->s_params.pass_throughs, 
				  &appconn->s_params.pass_through_count, 
				  ipph, dst))
	  found = 1;

      } else {
#endif
	switch (garden_check(appconn->s_params.pass_throughs, 
			     &appconn->s_params.pass_through_count, &pt,
			     ipph, dst
#ifdef HAVE_PATRICIA
			     , 0
#endif
			     )) {
	case 1:
	  found = 1;
	  break;
#ifdef ENABLE_GARDENEXT
	case -1:
	  if (pt)
	    pass_through_rem(appconn->s_params.pass_throughs, 
			     &appconn->s_params.pass_through_count, pt
#ifdef HAVE_PATRICIA
			     , 0
#endif
			     );
	  break;
#endif
	}
#ifdef HAVE_PATRICIA
      }
#endif
    }
  }
#endif

#ifdef ENABLE_IPWHITELIST
  if (!found && 
      _options.ipwhitelist && 
      dhcp_ipwhitelist(ipph, dst))
    found = 1;
#endif

#ifdef ENABLE_GARDENACCOUNTING
  if (_options.uamgardendata) {
    if (found) {
      if (!appconn) 
	appconn = dhcp_get_appconn_pkt(conn, 
				       (struct pkt_iphdr_t *)ipph, 
				       !dst);
      if (appconn) {
	extern struct app_conn_t admin_session;
	int len = ntohs(ipph->tot_len);
	if (_options.swapoctets) {
	  if (!dst) 
	    appconn->s_state.garden_output_octets +=len;
	  else
	    appconn->s_state.garden_input_octets +=len;
	  if (admin_session.s_state.authenticated) {
	    if (!dst) 
	      admin_session.s_state.garden_output_octets+=len;
	    else
	      admin_session.s_state.garden_input_octets+=len;
	  }
	} else {
	  if (!dst)
	    appconn->s_state.garden_input_octets +=len;
	  else
	    appconn->s_state.garden_output_octets +=len;
	  if (admin_session.s_state.authenticated) {
	    if (!dst) 
	      admin_session.s_state.garden_input_octets+=len;
	    else
	      admin_session.s_state.garden_output_octets+=len;
	  }
	}
      }
    }
  }
#endif

  return found;
}
		      

/**
 * dhcp_doDNAT()
 * Change destination address to authentication server.
 **/
int dhcp_doDNAT(struct dhcp_conn_t *conn, uint8_t *pack, 
		size_t len, char do_reset,
		char *do_checksum) {
  struct dhcp_t *this = conn->parent;
  struct pkt_ethhdr_t *ethh = pkt_ethhdr(pack);
  struct pkt_iphdr_t  *iph  = pkt_iphdr(pack);
  struct pkt_tcphdr_t *tcph = pkt_tcphdr(pack);

  /* Allow localhost through network... */
  if (iph->daddr == INADDR_LOOPBACK)
    return 0;

  /* Was it an ICMP request for us? */
  if (iph->protocol == PKT_IP_PROTO_ICMP) {
    if (iph->daddr == conn->ourip.s_addr) {
      OTHER_RECEIVED(conn, iph);
      return 0;
    }
  }

  /* Was it a request for local redirection server? */
  if ( ( iph->protocol == PKT_IP_PROTO_TCP )    &&
       ( iph->daddr == this->uamlisten.s_addr ) &&
       ( tcph->dst == htons(this->uamport) 
#ifdef ENABLE_UAMUIPORT
	 || ( _options.uamuiport && tcph->dst == htons(_options.uamuiport))
#endif
	 ) ) {
    OTHER_RECEIVED(conn, iph);
    return 0; /* Destination was local redir server */
  }
  
  if (dhcp_garden_check(this, conn, 0,
			(struct pkt_ipphdr_t *)iph, 1))
    return 0;

  if (iph->protocol == PKT_IP_PROTO_TCP) {
    if (tcph->dst == htons(DHCP_HTTP) || tcph->dst == htons(DHCP_HTTP_CUSTOM)  //add for lighttpd. John added @2015/1/15 
#ifdef HAVE_SSL
	|| (_options.redirssl && tcph->dst == htons(DHCP_HTTPS))
#endif
	) {
      /* Was it a http request for another server? */
      /* We are changing dest IP and dest port to local UAM server */

      *do_checksum = 1;

      OTHER_RECEIVED(conn, iph);
      return dhcp_uam_nat(conn, ethh, iph, tcph, 
			  &this->uamlisten, this->uamport);
    } 
#ifdef ENABLE_TCPRESET
    else if (do_reset) {

      /* otherwise, RESET and drop */

#if(_debug_ > 1)
      log_dbg("Resetting connection on port %d->%d", 
	      ntohs(tcph->src), ntohs(tcph->dst));
#endif

      dhcp_sendRESET(conn, pack, 1);
    }
#endif
  }
  
  return -1; /* Something else */
}

static 
int dhcp_postauthDNAT(struct dhcp_conn_t *conn, uint8_t *pack, 
		      size_t len, char is_return, char *do_checksum) {
  struct dhcp_t *this = conn->parent;
  struct pkt_ethhdr_t *ethh = pkt_ethhdr(pack);
  struct pkt_iphdr_t  *iph  = pkt_iphdr(pack);
  struct pkt_tcphdr_t *tcph = pkt_tcphdr(pack);

  if (is_return) {
    /* We check here (we also do this in dhcp_dounDNAT()) for UAM */
    if ( ( iph->saddr == this->uamlisten.s_addr ) &&
	 ( iph->protocol == PKT_IP_PROTO_TCP )    &&
	 ( tcph->src == htons(dhcp->uamport) 
#ifdef ENABLE_UAMUIPORT
	   || ( _options.uamuiport && tcph->src == htons(_options.uamuiport))
#endif
	   )) {
      
      *do_checksum = 1;
      dhcp_uam_unnat(conn, ethh, iph, tcph);
    }
  } 

  if (_options.postauth_proxyport > 0) {
    if (is_return) {
      if ((iph->protocol == PKT_IP_PROTO_TCP) &&
	  (iph->saddr == _options.postauth_proxyip.s_addr) &&
	  (tcph->src == htons(_options.postauth_proxyport))) {
	
	*do_checksum = 1;
	return dhcp_uam_unnat(conn, ethh, iph, tcph);
      }
    }
    else {
      if ((iph->protocol == PKT_IP_PROTO_TCP) &&
	  (tcph->dst == htons(DHCP_HTTP) || tcph->dst == htons(DHCP_HTTP_CUSTOM)   //add for lighttpd. John add 2016/1/15
#ifdef HAVE_SSL
	   || (_options.redirssl && tcph->dst == htons(DHCP_HTTPS))
#endif
	   )) {

#if(_debug_ > 0)	
	log_dbg("rewriting packet for post-auth proxy %s:%d",
		inet_ntoa(_options.postauth_proxyip),
		_options.postauth_proxyport);
#endif
	
	*do_checksum = 1;
	return dhcp_uam_nat(conn, ethh, iph, tcph,
			    &_options.postauth_proxyip, 
			    _options.postauth_proxyport);
      }
    }
  }

  return -1; /* Something else */
}

/**
 * dhcp_undoDNAT()
 * Change source address back to original server
 **/
static 
int dhcp_undoDNAT(struct dhcp_conn_t *conn, 
		  uint8_t *pack, size_t *plen,
		  char do_reset, char *do_checksum) {
  struct dhcp_t *this = conn->parent;
  struct pkt_ethhdr_t *ethh = pkt_ethhdr(pack);
  struct pkt_iphdr_t  *iph  = pkt_iphdr(pack);
  struct pkt_tcphdr_t *tcph = pkt_tcphdr(pack);

  /* Allow localhost through network... */
  if (iph->saddr == INADDR_LOOPBACK)
    return 0;

  if (iph->protocol == PKT_IP_PROTO_ICMP) {
    /* Was it an ICMP reply from us? */
    if (iph->saddr == conn->ourip.s_addr) {
      OTHER_SENDING(conn, iph);
      return 0;
    }
  }

  /* Was it a reply from redir server? */
  if ( (iph->saddr == this->uamlisten.s_addr) &&
       (iph->protocol == PKT_IP_PROTO_TCP) &&
       (tcph->src == htons(this->uamport) 
#ifdef ENABLE_UAMUIPORT
	|| (_options.uamuiport && tcph->src == htons(_options.uamuiport))
#endif
	) ) {

    *do_checksum = 1;
    OTHER_SENDING(conn, iph);
    return dhcp_uam_unnat(conn, ethh, iph, tcph);
  }
  
  if (dhcp_garden_check(this, conn, 0,
			(struct pkt_ipphdr_t *)iph, 0))
    return 0;

#ifdef ENABLE_TCPRESET
  if (do_reset && iph->protocol == PKT_IP_PROTO_TCP) {
#if(_debug_ > 1)
    log_dbg("Resetting connection on port %d->%d (undo)", 
	    ntohs(tcph->src), ntohs(tcph->dst));
#endif
    dhcp_sendRESET(conn, pack, 0);
    if (conn->peer) {
      tun_sendRESET(tun, pack, (struct app_conn_t *)conn->peer);
    }
  }
#endif

  return -1; /* Something else */
}

/**
 * dhcp_getdefault()
 * Fill in a DHCP packet with most essential values
 **/
int
dhcp_getdefault(uint8_t *pack) {

  return 0;
}

/**
 * dhcp_create_pkt()
 * Create a new typed DHCP packet
 */
int
dhcp_create_pkt(uint8_t type, uint8_t *pack, uint8_t *req, 
		struct dhcp_conn_t *conn) {

  struct dhcp_t *this = conn->parent;

  struct pkt_ethhdr_t *req_ethh = pkt_ethhdr(req);
  struct dhcp_packet_t *req_dhcp = pkt_dhcppkt(req);

  struct pkt_ethhdr_t *pack_ethh;
  struct pkt_iphdr_t *pack_iph;
  struct pkt_udphdr_t *pack_udph;
  struct dhcp_packet_t *pack_dhcp;

  int pos = 0;

  int is_req_dhcp = (req_dhcp->options[0] == 0x63 &&
		     req_dhcp->options[1] == 0x82 &&
		     req_dhcp->options[2] == 0x53 &&
		     req_dhcp->options[3] == 0x63);

  copy_ethproto(req, pack);

  pack_ethh = pkt_ethhdr(pack);
  pack_iph  = pkt_iphdr(pack);
  pack_udph = pkt_udphdr(pack);
  pack_dhcp = pkt_dhcppkt(pack);

  pack_dhcp->op     = DHCP_BOOTREPLY;
  pack_dhcp->htype  = DHCP_HTYPE_ETH;
  pack_dhcp->hlen   = PKT_ETH_ALEN;

  /* IP header */
  pack_iph->version_ihl = PKT_IP_VER_HLEN;
  pack_iph->tos = 0;
  pack_iph->tot_len = 0; /* Calculate at end of packet */
  pack_iph->id = 0;
  pack_iph->opt_off_high = 0;
  pack_iph->off_low = 0;
  pack_iph->ttl = 0x10;
  pack_iph->protocol = 0x11;
  pack_iph->check = 0; /* Calculate at end of packet */

  if (is_req_dhcp) {
    pack_dhcp->xid      = req_dhcp->xid;
    pack_dhcp->flags[0] = req_dhcp->flags[0];
    pack_dhcp->flags[1] = req_dhcp->flags[1];
    pack_dhcp->giaddr   = req_dhcp->giaddr;

    memcpy(&pack_dhcp->chaddr, &req_dhcp->chaddr, DHCP_CHADDR_LEN);
#ifdef ENABLE_DHCPRADIUS
    memcpy(&pack_dhcp->sname, conn->dhcp_opts.sname, DHCP_SNAME_LEN);
    memcpy(&pack_dhcp->file, conn->dhcp_opts.file, DHCP_FILE_LEN);
#endif

    log_dbg("dhcp server: %s", pack_dhcp->sname);
  }

  switch(type) {
  case DHCPOFFER:
  case DHCPFORCERENEW:
    pack_dhcp->yiaddr = conn->hisip.s_addr;
    break;
  case DHCPACK:
    pack_dhcp->xid    = req_dhcp->xid;
    pack_dhcp->yiaddr = conn->hisip.s_addr;
    break;
  case DHCPNAK:
    break;
  }

  /* Ethernet Header */
  memcpy(pack_ethh->dst, req_ethh->src, PKT_ETH_ALEN);
  memcpy(pack_ethh->src, dhcp_nexthop(this), PKT_ETH_ALEN);
  
  /* UDP and IP Headers */
  pack_udph->src = htons(DHCP_BOOTPS);
  pack_iph->saddr = conn->ourip.s_addr;

  /** http://www.faqs.org/rfcs/rfc1542.html
      BOOTREQUEST fields     BOOTREPLY values for UDP, IP, link-layer
   +-----------------------+-----------------------------------------+
   | 'ciaddr'  'giaddr'  B | UDP dest     IP destination   link dest |
   +-----------------------+-----------------------------------------+
   | non-zero     X      X | BOOTPC (68)  'ciaddr'         normal    |
   | 0.0.0.0   non-zero  X | BOOTPS (67)  'giaddr'         normal    |
   | 0.0.0.0   0.0.0.0   0 | BOOTPC (68)  'yiaddr'         'chaddr'  |
   | 0.0.0.0   0.0.0.0   1 | BOOTPC (68)  255.255.255.255  broadcast |
   +-----------------------+-----------------------------------------+

        B = BROADCAST flag

        X = Don't care

   normal = determine from the given IP destination using normal
            IP routing mechanisms and/or ARP as for any other
            normal datagram

   If the 'giaddr' field in a DHCP message from a client is non-zero,
   the server sends any return messages to the 'DHCP server' port on the
   BOOTP relay agent whose address appears in 'giaddr'. 

   If the 'giaddr' field is zero and the 'ciaddr' field is nonzero, then the
   server unicasts DHCPOFFER and DHCPACK messages to the address in
   'ciaddr'.  

   If 'giaddr' is zero and 'ciaddr' is zero, and the broadcast bit is set,
   then the server broadcasts DHCPOFFER and DHCPACK messages to
   0xffffffff. 

   If the broadcast bit is not set and 'giaddr' is zero and 'ciaddr' is
   zero, then the server unicasts DHCPOFFER and DHCPACK messages to the
   client's hardware address and 'yiaddr' address.  

   In all cases, when 'giaddr' is zero, the server broadcasts any DHCPNAK
   messages to 0xffffffff.

  **/

  if (is_req_dhcp) {
    if (req_dhcp->ciaddr) {
      pack_iph->daddr = req_dhcp->ciaddr; 
      pack_udph->dst = htons(DHCP_BOOTPC);
    } else if (req_dhcp->giaddr) {
      pack_iph->daddr = req_dhcp->giaddr; 
      pack_udph->dst = htons(DHCP_BOOTPS);
    } else if (type == DHCPNAK ||           /* Nak always to broadcast */
	       req_dhcp->flags[0] & 0x80 || /* Broadcast bit set */
	       _options.dhcp_broadcast) {   /* Optional always send to broadcast */
      pack_iph->daddr = ~0; 
      pack_udph->dst = htons(DHCP_BOOTPC);
      pack_dhcp->flags[0] = 0x80;
      if (req_dhcp->flags[0] & 0x80)
	memcpy(pack_ethh->dst, bmac, PKT_ETH_ALEN);
    } else {
      pack_iph->daddr = pack_dhcp->yiaddr; 
      pack_udph->dst = htons(DHCP_BOOTPC);
    }
  } else {
    struct pkt_iphdr_t *iph = pkt_iphdr(req);
    pack_iph->daddr = iph->saddr;
    pack_udph->dst = htons(DHCP_BOOTPC);
  }

  /* Magic cookie */
  pack_dhcp->options[pos++] = 0x63;
  pack_dhcp->options[pos++] = 0x82;
  pack_dhcp->options[pos++] = 0x53;
  pack_dhcp->options[pos++] = 0x63;

  pack_dhcp->options[pos++] = DHCP_OPTION_MESSAGE_TYPE;
  pack_dhcp->options[pos++] = 1;
  pack_dhcp->options[pos++] = type;

#ifdef ENABLE_DHCPRADIUS
  if (pos + conn->dhcp_opts.option_length < DHCP_OPTIONS_LEN) {
    memcpy(&pack_dhcp->options[pos], conn->dhcp_opts.options, 
	   conn->dhcp_opts.option_length);
    pos += conn->dhcp_opts.option_length;
  }
#endif

#ifdef ENABLE_DHCPOPT
  if (_options.dhcp_options[0]) {
    struct dhcp_tag_t *param_list = 0;
    struct pkt_udphdr_t *req_udph = pkt_udphdr(req);
    uint16_t udph_len = ntohs(req_udph->len);

    if (!dhcp_gettag(req_dhcp, udph_len - PKT_UDP_HLEN, 
		     &param_list, DHCP_OPTION_PARAMETER_REQUEST_LIST)) {

      uint8_t *lhead = _options.dhcp_options;
      struct dhcp_tag_t *opt = (struct dhcp_tag_t *)lhead;
      while (opt && opt->t && opt->l) {
	int param_count = param_list->l;
	int i;

	log_dbg("DHCP Type: %d Length: %d", (int)opt->t, (int)opt->l);

	/* for each configured option, iterate the param_list */
	for (i=0; i < param_count; i++) {
	  if (param_list->v[i] == opt->t) {
	    if (pos + opt->l + 2 < DHCP_OPTIONS_LEN) {
	      memcpy(&pack_dhcp->options[pos], opt,
		     opt->l + 2);
	      pos += opt->l + 2;
	    }
	    break;
	  }
	}

	lhead += opt->l + 2;
	opt = (struct dhcp_tag_t *)lhead;
      }
    }
  }
#endif

  return pos;
}


/**
 * dhcp_gettag()
 * Search a DHCP packet for a particular tag.
 * Returns -1 if not found.
 **/
int dhcp_gettag(struct dhcp_packet_t *pack, size_t length,
		struct dhcp_tag_t **tag, uint8_t tagtype) {
  struct dhcp_tag_t *t;
  size_t offset = DHCP_MIN_LEN + DHCP_OPTION_MAGIC_LEN;

  /* if (length > DHCP_LEN) {
    log_warn(0,"Length of dhcp packet larger then %d: %d", DHCP_LEN, length);
    length = DHCP_LEN;
  } */
  
  while ((offset + 2) < length) {
    t = (struct dhcp_tag_t *)(((uint8_t *)pack) + offset);
    if (t->t == tagtype) {
      if ((offset + 2 + (size_t)(t->l)) > length)
	return -1; /* Tag length too long */
      *tag = t;
      return 0;
    }
    offset += 2 + t->l;
  }
  
  return -1; /* Not found  */
}

static int dhcp_accept_opt(struct dhcp_conn_t *conn, uint8_t *o, int pos) {
  struct dhcp_t *this = conn->parent;

  o[pos++] = DHCP_OPTION_SUBNET_MASK;
  o[pos++] = 4;
  if (conn->noc2c)
    memset(&o[pos], 0xff, 4);
  else
    memcpy(&o[pos], &conn->hismask.s_addr, 4);
  pos += 4;

  if (conn->noc2c) {
    o[pos++] = DHCP_OPTION_STATIC_ROUTES;
    o[pos++] = 8;
    memcpy(&o[pos], &conn->ourip.s_addr, 4);
    pos += 4;
    memcpy(&o[pos], &conn->hisip.s_addr, 4);
    pos += 4;
  }

  o[pos++] = DHCP_OPTION_ROUTER_OPTION;
  o[pos++] = 4;
  memcpy(&o[pos], &conn->ourip.s_addr, 4);
  pos += 4;

  if (conn->dns1.s_addr && conn->dns2.s_addr) {
    o[pos++] = DHCP_OPTION_DNS;
    o[pos++] = 8;
    memcpy(&o[pos], &conn->dns1.s_addr, 4);
    pos += 4;
    memcpy(&o[pos], &conn->dns2.s_addr, 4);
    pos += 4;
  }
  else if (conn->dns1.s_addr) {
    o[pos++] = DHCP_OPTION_DNS;
    o[pos++] = 4;
    memcpy(&o[pos], &conn->dns1.s_addr, 4);
    pos += 4;
  }
  else if (conn->dns2.s_addr) {
    o[pos++] = DHCP_OPTION_DNS;
    o[pos++] = 4;
    memcpy(&o[pos], &conn->dns2.s_addr, 4);
    pos += 4;
  }

  if (strlen(conn->domain)) {
    o[pos++] = DHCP_OPTION_DOMAIN_NAME;
    o[pos++] = strlen(conn->domain);
    memcpy(&o[pos], &conn->domain, strlen(conn->domain));
    pos += strlen(conn->domain);
  }

  o[pos++] = DHCP_OPTION_LEASE_TIME;
  o[pos++] = 4;
  o[pos++] = (this->lease >> 24) & 0xFF;
  o[pos++] = (this->lease >> 16) & 0xFF;
  o[pos++] = (this->lease >>  8) & 0xFF;
  o[pos++] = (this->lease >>  0) & 0xFF;

  o[pos++] = DHCP_OPTION_INTERFACE_MTU;
  o[pos++] = 2;
  o[pos++] = (conn->mtu >> 8) & 0xFF;
  o[pos++] = (conn->mtu >> 0) & 0xFF;

  o[pos++] = DHCP_OPTION_SERVER_ID;
  o[pos++] = 4;
  memcpy(&o[pos], &conn->ourip.s_addr, 4);
  pos += 4;

  o[pos++] = DHCP_OPTION_END;

  return pos;
}

static size_t
dhcp_handler(int type,
	     struct dhcp_conn_t *dhcpconn,
	     uint8_t *pack, size_t len,
	     uint8_t *packet, size_t pos) {
#if defined(ENABLE_LOCATION) || defined(ENABLE_MODULES)
  struct app_conn_t *appconn = 0;
#endif

#ifdef ENABLE_LOCATION
  if (_options.location_option_82) {
    switch(type) {
    case CHILLI_DHCP_PROXY:
    case CHILLI_DHCP_OFFER: 
    case CHILLI_DHCP_ACK: 
      {
	struct dhcp_tag_t * opt82 = 0;
	struct dhcp_packet_t * dhcpp = pkt_dhcppkt(pack);
	if (!dhcp_gettag(dhcpp, ntohs(pkt_udphdr(pack)->len)-PKT_UDP_HLEN, 
			 &opt82, DHCP_OPTION_82)) {

	  if (!appconn && dhcpconn)
	    appconn = (struct app_conn_t *) dhcpconn->peer;

	  if (!appconn)
	    chilli_getconn(&appconn, dhcpp->yiaddr, 0, 0);
	  
	  if (appconn) {
	    chilli_learn_location(opt82->v, opt82->l, appconn, 1);
	  } else {
	    log_dbg("no appconn for option 82");
	  }
	}
      }
      break;
    }
  }
#endif

#ifdef ENABLE_MODULES
  {
    int i;
    log_dbg("checking modules...");
    for (i=0; i < MAX_MODULES; i++) {
      if (!_options.modules[i].name[0]) break;
      if (_options.modules[i].ctx) {
	struct chilli_module *m = 
	  (struct chilli_module *)_options.modules[i].ctx;
	if (m->dhcp_handler) {
	  pos = m->dhcp_handler(type,
				appconn, dhcpconn, 
				pack, len, 
				packet, pos);
	}
      }
    }
  }
#endif

  return pos;
}

/**
 * dhcp_sendOFFER()
 * Send of a DHCP offer message to a peer.
 **/
int dhcp_sendOFFER(struct dhcp_conn_t *conn, uint8_t *pack, size_t len) {

  struct dhcp_t *this = conn->parent;

  uint8_t packet[1500];

  struct pkt_iphdr_t *packet_iph;
  struct pkt_udphdr_t *packet_udph;
  struct dhcp_packet_t *packet_dhcp;

  uint16_t length = 576 + 4; /* Maximum length */
  uint16_t udp_len = 576 - 20; /* Maximum length */
  size_t pos = 0;

  /* Get packet default values */
  memset(packet, 0, sizeof(packet));
  pos = dhcp_create_pkt(DHCPOFFER, packet, pack, conn);

  packet_iph  = pkt_iphdr(packet);
  packet_udph = pkt_udphdr(packet);
  packet_dhcp = pkt_dhcppkt(packet);
  
  pos = dhcp_accept_opt(conn, packet_dhcp->options, pos);

  pos = dhcp_handler(CHILLI_DHCP_OFFER,
		     conn, pack, len, packet, pos);

  udp_len = pos + DHCP_MIN_LEN + PKT_UDP_HLEN;
  packet_udph->len = htons(udp_len);
  packet_iph->tot_len = htons(udp_len + PKT_IP_HLEN);
  chksum(packet_iph);

  length = udp_len + sizeofip(packet);

  OTHER_SENDING(conn, packet_iph);
  return dhcp_send(this, dhcp_conn_idx(conn), conn->hismac, packet, length);
}

/**
 * dhcp_sendACK()
 * Send of a DHCP acknowledge message to a peer.
 **/
int dhcp_sendACK(struct dhcp_conn_t *conn, uint8_t *pack, size_t len) {

  struct dhcp_t *this = conn->parent;

  uint8_t packet[1500];

  struct pkt_iphdr_t *packet_iph;
  struct pkt_udphdr_t *packet_udph;
  struct dhcp_packet_t *packet_dhcp;

  uint16_t length = 576 + 4; /* Maximum length */
  uint16_t udp_len = 576 - 20; /* Maximum length */
  size_t pos = 0;

  /* Get packet default values */
  memset(packet, 0, sizeof(packet));
  pos = dhcp_create_pkt(DHCPACK, packet, pack, conn);

  packet_iph  = pkt_iphdr(packet);
  packet_udph = pkt_udphdr(packet);
  packet_dhcp = pkt_dhcppkt(packet);

  pos = dhcp_accept_opt(conn, packet_dhcp->options, pos);

  pos = dhcp_handler(CHILLI_DHCP_ACK,
		     conn, pack, len, packet, pos);

  udp_len = pos + DHCP_MIN_LEN + PKT_UDP_HLEN;
  packet_udph->len = htons(udp_len);
  packet_iph->tot_len = htons(udp_len + PKT_IP_HLEN);
  chksum(packet_iph);

  length = udp_len + sizeofip(packet);

  OTHER_SENDING(conn, packet_iph);
  return dhcp_send(this, dhcp_conn_idx(conn), conn->hismac, packet, length);
}

/**
 * dhcp_sendNAK()
 * Send of a DHCP negative acknowledge message to a peer.
 * NAK messages are always sent to broadcast IP address (
 * except when using a DHCP relay server)
 **/
int dhcp_sendNAK(struct dhcp_conn_t *conn, uint8_t *pack, size_t len) {

  struct dhcp_t *this = conn->parent;
  uint8_t packet[1500];

  struct pkt_iphdr_t *packet_iph;
  struct pkt_udphdr_t *packet_udph;
  struct dhcp_packet_t *packet_dhcp;

  uint16_t length = 576 + 4; /* Maximum length */
  uint16_t udp_len = 576 - 20; /* Maximum length */
  size_t pos = 0;

  /* Get packet default values */
  memset(packet, 0, sizeof(packet));
  pos = dhcp_create_pkt(DHCPNAK, packet, pack, conn);

  packet_iph  = pkt_iphdr(packet);
  packet_udph = pkt_udphdr(packet);
  packet_dhcp = pkt_dhcppkt(packet);

  packet_dhcp->options[pos++] = DHCP_OPTION_SERVER_ID;
  packet_dhcp->options[pos++] = 4;
  memcpy(&packet_dhcp->options[pos], &conn->ourip.s_addr, 4);
  pos += 4;

  packet_dhcp->options[pos++] = DHCP_OPTION_END;

  pos = dhcp_handler(CHILLI_DHCP_NAK,
		     conn, pack, len, packet, pos);

  udp_len = pos + DHCP_MIN_LEN + PKT_UDP_HLEN;
  packet_udph->len = htons(udp_len);
  packet_iph->tot_len = htons(udp_len + PKT_IP_HLEN);

  chksum(packet_iph);
  length = udp_len + sizeofip(packet);

  OTHER_SENDING(conn, packet_iph);
  return dhcp_send(this, dhcp_conn_idx(conn), conn->hismac, packet, length);
}


static int dhcp_relay(struct dhcp_t *this, 
		      uint8_t *pack, size_t len) {
  struct pkt_udphdr_t *pack_udph = pkt_udphdr(pack);
  struct dhcp_packet_t *pack_dhcp = pkt_dhcppkt(pack);
  struct sockaddr_in addr;
  
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = _options.dhcpgwip.s_addr;
  addr.sin_port = htons(_options.dhcpgwport);
  
  if (_options.dhcprelayip.s_addr)
    pack_dhcp->giaddr = _options.dhcprelayip.s_addr;
  else
    pack_dhcp->giaddr = _options.uamlisten.s_addr;
  
  { 
    /* rewrite the server-id, to match the 
       upstream server (should be taken from 
       previous replies) */
    struct dhcp_tag_t *tag = 0;
    if (!dhcp_gettag(pack_dhcp, ntohs(pack_udph->len) - PKT_UDP_HLEN, 
		     &tag, DHCP_OPTION_SERVER_ID)) {
      memcpy(tag->v, &_options.dhcpgwip.s_addr, 4);
    }
  }
  
  pack_dhcp->hops++;
  
  (void) dhcp_handler(CHILLI_DHCP_RELAY,
		      0, pack, len, 0, 0);
  
  log_dbg("Sending DHCP relay packet to %s",
	  inet_ntoa(addr.sin_addr));
  
  /* if we can't send, lets do dhcp ourselves */
  if (sendto(this->relayfd, pack_dhcp, 
	     ntohs(pack_udph->len) - PKT_UDP_HLEN, 0,
	     (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    log_err(errno, "could not relay DHCP request!");
    return -1;
  }

  return 0;
}

/**
 *  dhcp_getreq()
 *  Process a received DHCP request and sends a response.
 **/
int dhcp_getreq(struct dhcp_ctx *ctx, uint8_t *pack, size_t len) {
  struct dhcp_t *this = ctx->parent;
  uint8_t mac[PKT_ETH_ALEN];
  struct dhcp_tag_t *message_type = 0;
  struct dhcp_tag_t *requested_ip = 0;
  struct dhcp_conn_t *conn;
  struct in_addr addr;

  struct pkt_ethhdr_t *pack_ethh = pkt_ethhdr(pack);
  struct pkt_udphdr_t *pack_udph = pkt_udphdr(pack);
  struct dhcp_packet_t *pack_dhcp = pkt_dhcppkt(pack);

  log_dbg("function %s()", __FUNCTION__);

  if (pack_udph->dst != htons(DHCP_BOOTPS)) 
    return 0; /* Not a DHCP packet */

  if (dhcp_gettag(pkt_dhcppkt(pack), ntohs(pack_udph->len)-PKT_UDP_HLEN, 
		  &message_type, DHCP_OPTION_MESSAGE_TYPE)) {
    return -1;
  }

  if (message_type->l != 1)
    return -1; /* Wrong length of message type */

  if (memcmp(pack_dhcp->chaddr, nmac, PKT_ETH_ALEN))
    memcpy(mac, pack_dhcp->chaddr, PKT_ETH_ALEN);
  else
    memcpy(mac, pack_ethh->src, PKT_ETH_ALEN);
  
  switch(message_type->v[0]) {
    
  case DHCPDECLINE:
    log_dbg("DHCP-Decline");
    /* drop through */

  case DHCPRELEASE:
    dhcp_release_mac(this, mac, RADIUS_TERMINATE_CAUSE_LOST_CARRIER);
    
  case DHCPDISCOVER:
  case DHCPREQUEST:
  case DHCPINFORM:
    break;

  default:
    return 0; /* Unsupported message type */
  }

  if (message_type->v[0] == DHCPRELEASE) {
    /* No Reply to client is sent */
    return 0;
  }
  
  if (dhcp_getconn(this, &conn, mac, pack, this->allowdyn)) {
    /* Could not allocate address */
    return 0; 
  }

  dhcp_conn_set_idx(conn, ctx);

  if (this->relayfd > 0) {
    /** Relay the DHCP request **/
    return dhcp_relay(this, pack, len);
  }

  if (conn->authstate == DHCP_AUTH_DROP)
    return 0;

  addr.s_addr = pack_dhcp->ciaddr;

  if (!dhcp_gettag(pkt_dhcppkt(pack), ntohs(pack_udph->len)-PKT_UDP_HLEN, 
		   &requested_ip, DHCP_OPTION_REQUESTED_IP))
    memcpy(&addr.s_addr, requested_ip->v, 4);
  
  if (_options.strictdhcp && addr.s_addr &&
      (addr.s_addr & _options.mask.s_addr) != _options.net.s_addr) {
    log_dbg("NAK: strictdhcp and address not in net");
    return dhcp_sendNAK(conn, pack, len);
  }
  
  /* Request an IP address */ 
  /** if (conn->authstate == DHCP_AUTH_NONE) XXX **/
  {
    if (this->cb_request && 
	this->cb_request(conn, &addr, pack, len)) {
      log_dbg("NAK: auth-none");
      return dhcp_sendNAK(conn, pack, len);
    }
  }
  
  conn->lasttime = mainclock_now();

  /* Discover message */
  /* If an IP address was assigned offer it to the client */
  /* Otherwise ignore the request */
  switch (message_type->v[0]) {
  case DHCPDISCOVER:
    if (conn->hisip.s_addr) 
      dhcp_sendOFFER(conn, pack, len);
    return 0;
  
  case DHCPREQUEST:
    {
      char send_ack = 0;

      if (!conn->hisip.s_addr) {
#if(_debug_)
	log_dbg("hisip not set!");
#endif
	return dhcp_sendNAK(conn, pack, len);
      }
      
      if (!memcmp(&conn->hisip.s_addr, &pack_dhcp->ciaddr, 4))
	send_ack = 1;
      
      if (!send_ack)
	if (!memcmp(&conn->hisip.s_addr, &addr.s_addr, 4))
	  send_ack = 1;
      
      if (send_ack) {
	return dhcp_sendACK(conn, pack, len);
      }
      
#if(_debug_)
      log_dbg("Sending NAK to client");
#endif
      return dhcp_sendNAK(conn, pack, len);
    }
  }
  
  /* 
   *  Unsupported DHCP message: Ignore 
   */
  log_dbg("Unsupported DHCP message ignored");
  return 0;
}


/**
 * dhcp_set_addrs()
 * Set various IP addresses of a connection.
 **/
int dhcp_set_addrs(struct dhcp_conn_t *conn, 
		   struct in_addr *hisip, struct in_addr *hismask, 
		   struct in_addr *ourip, struct in_addr *ourmask, 
		   struct in_addr *dns1,  struct in_addr *dns2) {

  conn->hisip.s_addr = hisip->s_addr;
  conn->hismask.s_addr = hismask->s_addr;
  conn->ourip.s_addr = ourip->s_addr;
  conn->dns1.s_addr = dns1->s_addr;
  conn->dns2.s_addr = dns2->s_addr;

  if (!conn->domain[0] && _options.domain) {
    safe_strncpy(conn->domain, _options.domain, DHCP_DOMAIN_LEN);
  }
  
#if defined(ENABLE_TAP) && defined(SIOCSARP)
  if (_options.usetap && !_options.noarpentries) {
    /*
     *    USETAP ARP
     */
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd > 0) {
      struct arpreq req;
      
      memset(&req, 0, sizeof(req));
      
      /* SET_SA_FAMILY(req.arp_ha, AF_UNSPEC);*/
      SET_SA_FAMILY(req.arp_pa, AF_INET);
      ((struct sockaddr_in *) &req.arp_pa)->sin_addr.s_addr = conn->hisip.s_addr;
      req.arp_flags = ATF_PERM;
      
      memcpy(req.arp_ha.sa_data, conn->hismac, PKT_ETH_ALEN);
      
      log_dbg("ARP Entry: %s -> %.2x:%.2x:%.2x:%.2x:%.2x:%.2x", 
	      inet_ntoa(conn->hisip),
	      conn->hismac[0], conn->hismac[1], conn->hismac[2],
	      conn->hismac[3], conn->hismac[4], conn->hismac[5]);
      
      safe_strncpy(req.arp_dev, tuntap(tun).devname, sizeof(req.arp_dev));
      
      if (ioctl(sockfd, SIOCSARP, &req) < 0) {
	perror("ioctrl()");
      }
      close(sockfd);
    }
  }
#endif
  
#ifdef ENABLE_UAMANYIP
  if (  _options.uamanyip && 
      ! _options.uamnatanyip &&
	(hisip->s_addr & ourmask->s_addr) != 
	(ourip->s_addr & ourmask->s_addr)) {
    /**
     *  We have enabled ''uamanyip'' and the address we are setting does
     *  not fit in ourip's network. In this case, add a route entry. 
     */
    struct app_conn_t *appconn = dhcp_get_appconn_addr(conn, hisip);
    if (appconn) {
      struct ippoolm_t *ipm = (struct ippoolm_t*)appconn->uplink;
      if (ipm && ipm->in_use && ipm->is_static) {
	struct in_addr mask;
	int res;
	mask.s_addr = 0xffffffff;
	res = net_add_route(hisip, ourip, &mask);
	log_dbg("Adding route for %s %d", inet_ntoa(*hisip), res);
      }
    }
  }
#endif

  return 0;
}

int dhcp_receive_eapol(struct dhcp_ctx *ctx, uint8_t *pack);
int dhcp_receive_arp(struct dhcp_ctx *ctx, uint8_t *pack, size_t len);

/**
 *  dhcp_receive_ip()
 *  Received a packet from the dhcpif
 */
int dhcp_receive_ip(struct dhcp_ctx *ctx, uint8_t *pack, size_t len) {
  struct dhcp_t *this = ctx->parent;
  struct pkt_ethhdr_t *pack_ethh = pkt_ethhdr(pack);
  struct pkt_iphdr_t  *pack_iph  = pkt_iphdr(pack);
  struct pkt_tcphdr_t *pack_tcph = pkt_tcphdr(pack);
  struct pkt_udphdr_t *pack_udph = pkt_udphdr(pack);
  struct dhcp_conn_t *conn = 0;
  struct in_addr ourip;
  struct in_addr addr;

  char do_checksum = 0;
  char allowed = 0;
  char has_ip = 0;
  char is_dhcp = 0;

  int authstate = 0;

  struct app_conn_t *appconn = 0;

  uint16_t iph_tot_len;
  uint16_t eth_tot_len;

#if(_debug_ > 1)
  log_dbg("function %s()", __FUNCTION__);
#endif

  if (len < PKT_IP_HLEN + PKT_ETH_HLEN + 4) {
#if(_debug_ > 1)
    log_dbg("too short");
#endif
    return 0;
  }

  if (pack_iph->version_ihl != PKT_IP_VER_HLEN) {
#if(_debug_)
    log_dbg("dropping non-IPv4");
#endif
    return 0;
  }

  /*
   * Sanity check on IP total length
   */
  iph_tot_len = ntohs(pack_iph->tot_len);
  eth_tot_len = iph_tot_len + sizeofeth(pack);

  if (eth_tot_len > (uint16_t) len) {
    log_dbg("dropping ip packet; ip-len=%d + eth-hdr=%d > read-len=%d",
	    iph_tot_len,
	    sizeofeth(pack), (int)len);
    
    if (pack_iph->opt_off_high & 64) { /* Don't Defrag Option */
      uint8_t icmp_pack[1500];

      log_dbg("Sending fragmentation ICMP");
      dhcp_send(this, ctx->idx, pack_ethh->src, icmp_pack, 
		icmpfrag(icmp_pack, sizeof(icmp_pack), pack));

      OTHER_SENDING(conn, pkt_iphdr(icmp_pack));
    }
    
    OTHER_RECEIVED(conn, pack_iph);
    return 0;
  }
  
  if (iph_tot_len > _options.mtu ||
      iphdr_more_frag(pack_iph) ||
      iphdr_offset(pack_iph)) {
    uint8_t icmp_pack[1500];
    log_dbg("ICMP frag for IP packet with length %d", iph_tot_len);
    dhcp_send(this, ctx->idx, pack_ethh->src, icmp_pack, 
	      icmpfrag(icmp_pack, sizeof(icmp_pack), pack));
    OTHER_SENDING(conn, pkt_iphdr(icmp_pack));
    OTHER_RECEIVED(conn, pack_iph);
    return 0;
  }

  /*
   *  Chop off any trailer length
   */
  if (len > (size_t) eth_tot_len) {
    log_dbg("chopping off trailer length %d", len - eth_tot_len);
    len = eth_tot_len;
  }
  
  /*
   * Sanity check on UDP total length
   */
  if (pack_iph->protocol == PKT_IP_PROTO_UDP) {
    uint16_t udph_len = ntohs(pack_udph->len);
    if (udph_len < PKT_UDP_HLEN ||
	iph_tot_len != udph_len + PKT_IP_HLEN) {
      log_dbg("dropping udp packet; ip-len=%d != udp-len=%d + ip-hdr=20",
	      (int) iph_tot_len,
	      (int) udph_len);
      OTHER_RECEIVED(conn, pack_iph);
      return 0;
    }
  }
  
  /* 
   *  Check that the destination MAC address is our MAC or Broadcast 
   */
  if ((memcmp(pack_ethh->dst, dhcp_nexthop(this), PKT_ETH_ALEN)) && 
      (memcmp(pack_ethh->dst, bmac, PKT_ETH_ALEN))) {
#ifdef ENABLE_MDNS
    /* http://en.wikipedia.org/wiki/IP_multicast */
    /* MAC 01:00:5e:xx:xx:xx IP 224.0.0.251 */
    if (pack_ethh->dst[0] == 0x01 &&
	pack_ethh->dst[1] == 0x00 &&
	pack_ethh->dst[2] == 0x5e) {
      log_dbg("Multicast: %.2X-%.2X-%.2X-%.2X-%.2X-%.2X",
	      pack_ethh->dst[0], pack_ethh->dst[1], pack_ethh->dst[2], 
	      pack_ethh->dst[3], pack_ethh->dst[4], pack_ethh->dst[5]);
    } else {
#endif
#if(_debug_)
      log_dbg("Not for our MAC or broadcast: %.2X-%.2X-%.2X-%.2X-%.2X-%.2X",
	      pack_ethh->dst[0], pack_ethh->dst[1], pack_ethh->dst[2], 
	      pack_ethh->dst[3], pack_ethh->dst[4], pack_ethh->dst[5]);
#endif
      OTHER_RECEIVED(conn, pack_iph);
      return 0;
#ifdef ENABLE_MDNS
    }
#endif
  }
  
  ourip.s_addr = this->ourip.s_addr;
  
  /* 
   *  DHCP (BOOTPS) packets for broadcast or us specifically
   */
  is_dhcp = (((pack_iph->daddr == 0) ||
	      (pack_iph->daddr == 0xffffffff) ||
	      (pack_iph->daddr == ourip.s_addr)) &&
	     ((pack_iph->protocol == PKT_IP_PROTO_UDP) &&
	      (pack_udph->dst == htons(DHCP_BOOTPS))));
  
  if (is_dhcp
#ifdef ENABLE_LAYER3
      && !_options.layer3
#endif
      ) {
    log_dbg("dhcp/bootps request being processed");
    (void) dhcp_getreq(ctx, pack, len);
    OTHER_RECEIVED(conn, pack_iph);
    return 0;
  }
  
  /* 
   *  Check to see if we know MAC address
   */
  if (!dhcp_hashget(this, &conn, pack_ethh->src)) {

    if (this->debug) 
      log_dbg("Address found");

    ourip.s_addr = conn->ourip.s_addr;

  } else {

    struct in_addr reqaddr;
    
    memcpy(&reqaddr.s_addr, &pack_iph->saddr, PKT_IP_ALEN);
    
    log_dbg("Address not found (%s)", inet_ntoa(reqaddr)); 
    
    /* Do we allow dynamic allocation of IP addresses? */
    if (!this->allowdyn 
#ifdef ENABLE_UAMANYIP
	&& !_options.uamanyip
#endif
#ifdef ENABLE_LAYER3
	&& !_options.layer3
#endif
	) {
      log_dbg("dropping packet; no dynamic ip and no anyip");
      return 0; 
    }
    
    /* Allocate new connection */
    if (dhcp_newconn(this, &conn, pack_ethh->src)) {
      log_dbg("dropping packet; out of connections");
      return 0; /* Out of connections */
    }
  }

  /* Return if we do not know peer */
  if (!conn) {
    log_dbg("dropping packet; no peer");
    return 0;
  }

  dhcp_conn_set_idx(conn, ctx);

#ifdef ENABLE_IEEE8021Q
  if (_options.ieee8021q) {
#if(_debug_ > 1)
    log_dbg("%s calling dhcp_checktag",__FUNCTION__);
#endif
    dhcp_checktag(conn, pack);
  }
#endif

#ifdef ENABLE_LAYER3
  if (is_dhcp && _options.layer3) {
    log_dbg("forwarding layer2 dhcp/bootps request");

    if (this->relayfd > 0) {
      /** Relay the DHCP request **/
      return dhcp_relay(this, pack, len);
    }

    (void) dhcp_handler(CHILLI_DHCP_PROXY,
			0, pack, len, 0, 0);
      
    this->cb_data_ind(conn, pack, len);
    OTHER_RECEIVED(conn, pack_iph);
    return 0;
  }
#endif

#ifdef ENABLE_LAYER3
  /*
   *  A bit of a hack to decouple the one-to-one relationship
   *  of "dhcp connections" and "app connections". Look for the 
   *  app session based on IP (Layer3) and adjust authstate.
   */
  switch (conn->authstate) {
  case DHCP_AUTH_ROUTER:
    {
      struct in_addr src;

      src.s_addr = pack_iph->saddr;

      appconn = chilli_connect_layer3(&src, conn);

      if (!appconn) return -1;

      app_conn_set_idx(appconn, conn);

      if (!appconn->dnlink)
	appconn->dnlink = conn;

      switch (appconn->s_state.authenticated) {
      case 1:
	authstate = DHCP_AUTH_PASS;
	break;
      default:
	authstate = DHCP_AUTH_DNAT;
	break;
      }

#ifdef ENABLE_UAMANYIP
      if (chilli_assign_snat(appconn, 0) != 0) {
	return -1;
      }
#endif

      has_ip = 1;
    }
    break;

  default:
    has_ip = conn->hisip.s_addr != 0;
    authstate = conn->authstate;
    break;
  }
#else
  has_ip = conn->hisip.s_addr != 0;
  authstate = conn->authstate;
#endif

  /* Request an IP address 
  if (_options.uamanyip && 
      authstate == DHCP_AUTH_NONE) {
    this->cb_request(conn, &pack_iph->saddr);
  } */
  
#ifdef ENABLE_IEEE8021Q
    if (_options.ieee8021q) {
#if(_debug_ > 1)
      log_dbg("%s calling dhcp_checktag",__FUNCTION__);
#endif
      dhcp_checktag(conn, pack);
    }
#endif

  /* 
   *  Request an IP address 
   */
  if ((authstate == DHCP_AUTH_NONE) && 
      (
#ifdef ENABLE_UAMANYIP
       _options.uamanyip || 
#endif
       ((pack_iph->daddr != 0) && 
	(pack_iph->daddr != 0xffffffff)))) {
    addr.s_addr = pack_iph->saddr;
    if (this->cb_request)
      if (this->cb_request(conn, &addr, 0, 0)) {
	log_dbg("dropping packet; ip not known: %s", inet_ntoa(addr));
	OTHER_RECEIVED(conn, pack_iph);
	return 0; /* Ignore request if IP address was not allocated */
      }
  }
  
  conn->lasttime = mainclock_now();
  
  if (
#ifdef ENABLE_LAYER3
      !_options.layer3 &&
#endif
      pack_iph->saddr != conn->hisip.s_addr) {
    log_dbg("Received packet with spoofed source!");
    OTHER_RECEIVED(conn, pack_iph);
    /*dhcp_sendRENEW(conn, pack, len);*/
    return 0;
  }

  switch (pack_iph->protocol) {
    
  case PKT_IP_PROTO_UDP:
    
    if ((pack_iph->daddr & ~_options.mask.s_addr) == 
	(0xffffffff & ~_options.mask.s_addr)) {
#ifdef ENABLE_NETBIOS
      if (pack_udph->dst == htons(137)) {
	log_dbg("NetBIOS NS to port %d", ntohs(pack_udph->dst));
	break;
      }
      else if (pack_udph->dst == htons(138)) {
	log_dbg("NetBIOS DGM to port %d", ntohs(pack_udph->dst));
	break;
      } 
#endif
#if(_debug_)
      log_dbg("Broadcasted UDP to port %d", ntohs(pack_udph->dst));
#endif
      OTHER_RECEIVED(conn, pack_iph);
      return 0;
    }
  
    break; /* UDP */
    
  case PKT_IP_PROTO_TCP:
    
    /* Was it a request for the auto-logout service? */
    if ((pack_iph->daddr == _options.uamlogout.s_addr) &&
	(pack_tcph->dst == htons(DHCP_HTTP))) {
      if (!appconn)
	appconn = dhcp_get_appconn_pkt(conn, pack_iph, 0);
      if (appconn) {
	if (appconn->s_state.authenticated) {
	  terminate_appconn(appconn, RADIUS_TERMINATE_CAUSE_USER_REQUEST);
	  log_dbg("Dropping session due to request for auto-logout ip");
	  appconn->uamexit = 1;
	}
      }
    }
    
    break; /* TCP */
  }
  
  if (_options.uamalias.s_addr && 
      pack_iph->daddr == _options.uamalias.s_addr) {
    
    do_checksum = 1;
    dhcp_uam_nat(conn, pack_ethh, pack_iph, pack_tcph, &this->uamlisten,
#ifdef ENABLE_UAMUIPORT
		 _options.uamuiport ? _options.uamuiport : 
#endif
		 this->uamport);
  }
  
  switch (dhcp_dnsDNAT(conn, pack, &len, &do_checksum)) { 
  case 0:  /* Not DNS */ break;
  case 1:  /* Allowed DNS */ allowed = 1; break; 
  default: /* Drop */ 
    OTHER_RECEIVED(conn, pack_iph);
    return 0;
  } 

#ifdef ENABLE_MULTILAN
  if (!appconn)
    appconn = dhcp_get_appconn_pkt(conn, pack_iph, 0);
  if (appconn)
    app_conn_set_idx(appconn, conn);
#endif
  
  switch (authstate) {

  case DHCP_AUTH_PASS:
#ifdef HAVE_NETFILTER_QUEUE
    if (this->qif_in.fd && this->qif_out.fd) {
      return 1;
    }
#endif
#ifdef HAVE_NETFILTER_COOVA
    if (_options.kname) {
      if (conn->peer) {
	appconn = (struct app_conn_t *)conn->peer;
	appconn->s_state.last_sent_time =
	  appconn->s_state.last_time = mainclock_now();
      }
      return 1;
    }
#endif

    /* Check for post-auth proxy, otherwise pass packets unmodified */
    dhcp_postauthDNAT(conn, pack, len, 0, &do_checksum);
    break; 

  case DHCP_AUTH_UNAUTH_TOS:
    /* Set TOS to specified value (unauthenticated) */
    pack_iph->tos = conn->unauth_cp;
    do_checksum = 1;
    break;

  case DHCP_AUTH_AUTH_TOS:
    /* Set TOS to specified value (authenticated) */
    pack_iph->tos = conn->auth_cp;
    do_checksum = 1;
    break;

  case DHCP_AUTH_SPLASH:
    dhcp_doDNAT(conn, pack, len, 0, &do_checksum);
    break;
    
  case DHCP_AUTH_DNAT:
    /* Destination NAT if request to unknown web server */
    if (dhcp_doDNAT(conn, pack, len, 1, &do_checksum) && !allowed) {
#if(_debug_ > 1)
      log_dbg("dropping packet; not nat'ed");
#endif
      OTHER_RECEIVED(conn, pack_iph);
      return 0;
    }
    break;

  case DHCP_AUTH_DROP: 
    log_dbg("dropping packet; auth-drop");
    OTHER_RECEIVED(conn, pack_iph);
    return 0;
    
  default:
    log_dbg("dropping packet; unhandled auth state %d", authstate);
    OTHER_RECEIVED(conn, pack_iph);
    return 0;
  }

  /*done:*/

#ifdef ENABLE_TAP
  if (_options.usetap) {
    memcpy(pack_ethh->dst, tuntap(tun).hwaddr, PKT_ETH_ALEN);
  }
#endif

  if (do_checksum)
    chksum(pack_iph);

  if (has_ip && (this->cb_data_ind)) {

    this->cb_data_ind(conn, pack, len);

  } else {

    log_dbg("no hisip; packet-drop");

  }
  
  return 0;
}

#ifdef ENABLE_IPV6
int dhcp_receive_ipv6(struct dhcp_ctx *ctx, uint8_t *pack, size_t len) {
  struct dhcp_t *this = ctx->parent;
  struct pkt_ethhdr_t *ethh = pkt_ethhdr(pack);
  struct pkt_ip6hdr_t *iphdr = pkt_ip6hdr(pack);

  int ip_datalen = (int) ntohs(iphdr->data_len);

  log_dbg("Processing IPv6 ver=%d class=%d len=%d datalen=%d",
	  (int) ipv6_version(iphdr),
	  (int) ipv6_class(iphdr),
	  (int) len, ip_datalen);

  log_dbg("src "IPv6_ADDR_FMT" dst "IPv6_ADDR_FMT,
	  ipv6_exlode_addr(iphdr->src_addr),
	  ipv6_exlode_addr(iphdr->dst_addr));

  switch(iphdr->next_header) {
  case ICMPv6_NEXT_HEADER:
    {
      int datalen = 0;
      uint8_t b[256], *data;
      
      struct pkt_icmphdr_t * icmphdr = 
	(struct pkt_icmphdr_t *)
	(((uint8_t *)iphdr) + sizeof(struct pkt_ip6hdr_t));

      void check_options() {
	if (datalen > 0) {
	  uint8_t t, l;
	  int i=0;
	  while (datalen > 0) {
	    int dlen = -1;
	    t = data[i++]; datalen--;
	    if (!datalen) break;
	    l = data[i++]; datalen--;
	    if (!datalen) break;
	    switch(t) {
	    case 1: /* source link-layer address */
	    case 2: /* target link-layer address */
	      dlen = l * PKT_ETH_ALEN;
	      break;
	    default:
	      log_dbg(0, "could not parse ICMP option");
	      datalen = 0;
	      break;
	    }
	    if (dlen > 0) {
	      if (dlen <= datalen) {
		log_dbg("ICMPv6 Option %d %d %d",(int)t,(int)l,dlen);
		memcpy(b, data + i, dlen);
		switch(t) {
		case 1: /* source link-layer address */
		case 2: /* target link-layer address */
		  log_dbg("ICMPv6 Source Link-Layer Address Option "
			  "%.2X%.2X%.2X%.2X%.2X%.2X",
			  b[0],b[1],b[2],b[3],b[4],b[5]);
		  break;
		default:
		  break;
		}
	      }
	      
	      i += dlen;
	      datalen -= dlen;
	    }
	  }
	}
      }

      data = ((uint8_t *)icmphdr) + sizeof(struct pkt_icmphdr_t);
      
      datalen = ip_datalen - sizeof(struct pkt_icmphdr_t);

      log_dbg("ICMPv6 type=%d", icmphdr->type);
      
      /* Check checksum */
      
      switch (icmphdr->type) {
      case 133: /* 133 Router Solicitation (NDP) */

	/*
	  4.1. Router Solicitation Message Format
	  
	  
	  Hosts send Router Solicitations in order to prompt routers to
	  generate Router Advertisements quickly.
	  
	  0                   1                   2                   3
	  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
	  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	  |     Type      |     Code      |          Checksum             |
	  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	  |                            Reserved                           |
	  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	  |   Options ...
	  +-+-+-+-+-+-+-+-+-+-+-+-
	*/

	data += 4;
	datalen -= 4;
	check_options();
	
	/*	  
	  4.2. Router Advertisement Message Format
	  
	  Routers send out Router Advertisement messages periodically, or in
	  response to Router Solicitations.
	  
	  0                   1                   2                   3
	  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
	  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	  |     Type      |     Code      |          Checksum             |
	  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	  | Cur Hop Limit |M|O|  Reserved |       Router Lifetime         |
	  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	  |                         Reachable Time                        |
	  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	  |                          Retrans Timer                        |
	  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	  |   Options ...
	  +-+-+-+-+-+-+-+-+-+-+-+-
	*/
	{
	  uint8_t packet[1500];
	  struct pkt_ethhdr_t *packet_ethh;
	  struct pkt_ip6hdr_t *packet_ip6h;
	  uint16_t data_len = ntohs(iphdr->data_len);
	  uint8_t *payload;
	  
	  struct pkt_icmphdr_t * packet_icmp = 0;
	  
	  memset(packet, 0, sizeof(packet));
	  copy_ethproto(pack, packet);
	  
	  packet_ethh = pkt_ethhdr(packet);
	  packet_ip6h = pkt_ip6hdr(packet);
	  
	  payload = ((uint8_t *)packet_ip6h) + sizeof(struct pkt_ip6hdr_t);
	  packet_icmp = (struct pkt_icmphdr_t *) payload;
	  
	  packet_icmp->type = 134;
	  packet_icmp->code = 0;
	  packet_icmp->check = 0;
	  
	  data_len = 16; 
	  
	  payload += sizeof(struct pkt_icmphdr_t);
	  *payload++ = 255;
	  *payload++ = (1<<7);
	  
	  /* router lifetime short */
	  *payload++ = 1; 
	  *payload++ = 0;
	  
	  /* reachable timer */
	  { uint32_t v=htonl(0);
	    memcpy(payload, &v, 4);
	    payload += 4;	}
	  
	  /* retrans timer */
	  { uint32_t v=htonl(0);
	    memcpy(payload, &v, 4);
	    payload += 4;	}
	  
	  /* Target link-layer address option */
	  data_len += 2 + PKT_ETH_ALEN;
	  *payload++ = 1;
	  *payload++ = 1;
	  memcpy(payload, dhcp_nexthop(dhcp), PKT_ETH_ALEN);
	  payload += PKT_ETH_ALEN;
	  
	  /* MTU option */
	  data_len += 8;
	  *payload++ = 5;
	  *payload++ = 1;
	  *payload++ = 0;*payload++ = 0;
	  { uint32_t v=htonl(_options.mtu);
	    memcpy(payload, &v, 4);
	    payload += 4;	}
	  
	  /* Prefix Information option */
	  data_len += 32;
	  *payload++ = 3;
	  *payload++ = 4;
	  *payload++ = 64;
	  *payload++ = (1<<6)|(1<<7);
	  { uint32_t v=htonl(_options.mtu);
	    memcpy(payload, &v, 4);
	    payload += 4;	}
	  { uint32_t v=htonl(_options.mtu);
	    memcpy(payload, &v, 4);
	    payload += 4;	}
	  *payload++ = 0;*payload++ = 0;*payload++ = 0;*payload++ = 0;
	  
	  *payload++ = 0x11;*payload++ = 0x11;*payload++ = 0;*payload++ = 0;
	  *payload++ = 0;*payload++ = 0;*payload++ = 0;*payload++ = 0;
	  *payload++ = 0;*payload++ = 0;*payload++ = 0;*payload++ = 0;
	  *payload++ = 0;*payload++ = 0;*payload++ = 0;*payload++ = 0;

	  /* Prefix Information option */
	  data_len += 32;
	  *payload++ = 3;
	  *payload++ = 4;
	  *payload++ = 64;
	  *payload++ = (1<<6)|(1<<7);
	  { uint32_t v=htonl(_options.mtu);
	    memcpy(payload, &v, 4);
	    payload += 4;	}
	  { uint32_t v=htonl(_options.mtu);
	    memcpy(payload, &v, 4);
	    payload += 4;	}
	  *payload++ = 0;*payload++ = 0;*payload++ = 0;*payload++ = 0;

	  ipv6_nat64_prefix(payload);
	  
	  packet_ip6h->ver_class_label = iphdr->ver_class_label;
	  packet_ip6h->next_header = iphdr->next_header;
	  packet_ip6h->hop_limit = 255;
	  packet_ip6h->data_len = htons(data_len);
	  
	  memcpy(packet_ip6h->src_addr, 
		 &this->rawif[0].address_v6.s6_addr, PKT_IPv6_ALEN);
	  memcpy(packet_ip6h->dst_addr, iphdr->src_addr, PKT_IPv6_ALEN);
	  
	  memcpy(packet_ethh->dst, ethh->src, PKT_ETH_ALEN);
	  memcpy(packet_ethh->src, dhcp_nexthop(dhcp), PKT_ETH_ALEN);
	  
	  chksum6(packet_ip6h);
	  
	  return dhcp_send(this, ctx->idx, ethh->src, 
			   packet, sizeofeth(packet) + 
			   sizeof(struct pkt_ip6hdr_t) + data_len);
	}
	break;

      case 135: /* 135  Neighbor Solicitation Message */
	{
	  uint8_t packet[1500];
	  struct pkt_ethhdr_t *packet_ethh;
	  struct pkt_ip6hdr_t *packet_ip6h;
	  uint16_t data_len = ntohs(iphdr->data_len);
	  uint8_t *payload, *target;
	  
	  struct pkt_icmphdr_t * packet_icmp = 0;

	  data += 4;
	  datalen -= 4;

	  target = data;

	  data += 16;
	  datalen -= 16;

	  check_options();
	  
	  memset(packet, 0, sizeof(packet));
	  copy_ethproto(pack, packet);
	  
	  packet_ethh = pkt_ethhdr(packet);
	  packet_ip6h = pkt_ip6hdr(packet);
	  
	  payload = ((uint8_t *)packet_ip6h) + sizeof(struct pkt_ip6hdr_t);
	  packet_icmp = (struct pkt_icmphdr_t *) payload;
	  
	  packet_icmp->type = 136;
	  packet_icmp->code = 0;
	  packet_icmp->check = 0;
	  
	  data_len = 24;
	  
	  payload += sizeof(struct pkt_icmphdr_t);
	  *payload++ = (1<<6)|(1<<5);
	  *payload++ = 0;
	  *payload++ = 0;
	  *payload++ = 0;

	  memcpy(payload, target, PKT_IPv6_ALEN);
	  payload += PKT_IPv6_ALEN;
	  
	  /* Target link-layer address option */
	  data_len += 2 + PKT_ETH_ALEN;
	  *payload++ = 2;
	  *payload++ = 1;
	  memcpy(payload, dhcp_nexthop(dhcp), PKT_ETH_ALEN);
	  payload += PKT_ETH_ALEN;
	  
	  packet_ip6h->ver_class_label = iphdr->ver_class_label;
	  packet_ip6h->next_header = iphdr->next_header;
	  packet_ip6h->hop_limit = 255;
	  packet_ip6h->data_len = htons(data_len);
	  
	  memcpy(packet_ip6h->src_addr, 
		 &this->rawif[0].address_v6.s6_addr, PKT_IPv6_ALEN);
	  memcpy(packet_ip6h->dst_addr, iphdr->src_addr, PKT_IPv6_ALEN);
	  
	  memcpy(packet_ethh->dst, ethh->src, PKT_ETH_ALEN);
	  memcpy(packet_ethh->src, dhcp_nexthop(dhcp), PKT_ETH_ALEN);
	  
	  chksum6(packet_ip6h);
	  
	  return dhcp_send(this, ctx->idx, ethh->src, 
			   packet, sizeofeth(packet) + 
			   sizeof(struct pkt_ip6hdr_t) + data_len);
	}
	break;
      }

    }
  case PKT_IP_PROTO_UDP:
    {
      struct pkt_udphdr_t * udphdr = 
	(struct pkt_udphdr_t *)
	(((uint8_t *)iphdr) + sizeof(struct pkt_ip6hdr_t));

      if (ntohs(udphdr->src) == 546 && ntohs(udphdr->dst) == 547) {
	struct pkt_dhcp6hdr_t * dhcp6hdr = 
	  (struct pkt_dhcp6hdr_t *)
	  (((uint8_t *)udphdr) + sizeof(struct pkt_udphdr_t));

	uint8_t * opts = 
	  (((uint8_t *)dhcp6hdr) + sizeof(struct pkt_dhcp6hdr_t));

	int optlen = ip_datalen - sizeof(struct pkt_udphdr_t) 
	  - sizeof(struct pkt_dhcp6hdr_t);

	log_dbg("DHCPv6 message %d 0x%.2X%.2X%.2X",
		dhcp6hdr->type,dhcp6hdr->id[0],
		dhcp6hdr->id[1],dhcp6hdr->id[2]);

	switch(dhcp6hdr->type) {
	case 1: /* Solicit */
	case 3: /* Request */
	case 4: /* Confirm */
	  {
	    struct dhcp_conn_t *conn = 0;
	    uint8_t *duid = 0;
	    uint16_t duid_len = 0;
	    uint16_t ia_type = 0;
	    int status = -1;

	    uint8_t *m;

	    while (optlen > 4) {
	      uint16_t t, l;
	      memcpy(&t, opts, 2); opts+=2; optlen-=2;
	      memcpy(&l, opts, 2); opts+=2; optlen-=2;
	      t = ntohs(t);
	      l = ntohs(l);
	      log_dbg("DHCPv6 Option %d %d", t, l);
	      switch(t) {
	      case 1: /* Client-Identifier */
		duid = opts;
		duid_len = l;
		break;
	      case 3: /* Identity Association for 
			 Non-temporary Addresses Option  */
	      case 4: /* Identity Association for 
			 Temporary Addresses Option */
		ia_type = t;
		break;
	      }
	      opts += l;
	      optlen -= l;
	    }

	    if (dhcp_getconn(this, &conn, ethh->src, 0, 1)) {
	      /* Could not allocate address */
	      return 0; 
	    }

	    this->cb_request(conn, 0, 0, 0);

	    {
	      uint8_t packet[1500];
	      struct pkt_ethhdr_t *packet_ethh;
	      struct pkt_ip6hdr_t *packet_ip6h;
	      struct pkt_udphdr_t *packet_udph;
	      uint8_t *payload;

	      uint16_t data_len = sizeof(struct pkt_udphdr_t) + 
		sizeof(struct pkt_dhcp6hdr_t );

	      uint16_t u;
	      uint32_t u2;
	  
	      copy_ethproto(pack, packet);
	  
	      packet_ethh = pkt_ethhdr(packet);
	      packet_ip6h = pkt_ip6hdr(packet);

	      payload = ((uint8_t *)packet_ip6h) + sizeof(struct pkt_ip6hdr_t);

	      packet_udph = (struct pkt_udphdr_t *)payload;
	      payload += sizeof(struct pkt_udphdr_t);
	      if (dhcp6hdr->type == 1) {
		*payload++=2;
	      } else {
		if (dhcp6hdr->type == 4) 
		  status = 0;
		*payload++=7;
	      }
	      *payload++=dhcp6hdr->id[0];
	      *payload++=dhcp6hdr->id[1];
	      *payload++=dhcp6hdr->id[2];

	      if (duid) {
		data_len += 4 + duid_len;
		u = htons(1); memcpy(payload, &u, 2); payload+=2;
		u = htons(duid_len); memcpy(payload, &u, 2); payload+=2;
		memcpy(payload, duid, duid_len); payload+=duid_len;
	      }

	      data_len += 4 + 14;
	      u = htons(2); memcpy(payload, &u, 2); payload+=2;
	      u = htons(14); memcpy(payload, &u, 2); payload+=2;
	      u = htons(1); memcpy(payload, &u, 2); payload+=2;
	      u = htons(1); memcpy(payload, &u, 2); payload+=2;
	      u2 = htonl(time(0)); memcpy(payload, &u2, 4); payload+=4;
	      memcpy(payload, dhcp_nexthop(dhcp), PKT_ETH_ALEN); 
	      payload+=PKT_ETH_ALEN;

	      if (ia_type) {
		uint16_t olen = 4; /* IAID */
		data_len += 4; /* IA-Type option header */
		switch(ia_type) {
		case 3:
		  olen += 4 + 4; /* T1 and T2 */
		  break;
		}
		olen += 4 + 24; /* IA-Address option */
		data_len += olen;

		u = htons(ia_type); memcpy(payload, &u, 2); payload+=2;
		u = htons(olen); memcpy(payload, &u, 2); payload+=2;
		u2 = htonl(1); memcpy(payload, &u2, 4); payload+=4;
		switch(ia_type) {
		case 3:
		  u2 = htonl(300); memcpy(payload, &u2, 4); payload+=4;
		  u2 = htonl(300); memcpy(payload, &u2, 4); payload+=4;
		  break;
		}
		u = htons(5); memcpy(payload, &u, 2); payload+=2;
		u = htons(24); memcpy(payload, &u, 2); payload+=2;

		ipv6_eui64_pack(payload, ethh->src);

		u2 = htonl(300); memcpy(payload, &u2, 4); payload+=4;
		u2 = htonl(300); memcpy(payload, &u2, 4); payload+=4;
	      }

	      data_len += 4 + 16;
	      u = htons(23); memcpy(payload, &u, 2); payload+=2;
	      u = htons(16); memcpy(payload, &u, 2); payload+=2;

	      m = (uint8_t *)&_options.dns1.s_addr;
	      ipv6_nat64_pack(payload, m);

	      data_len += 4 + strlen(_options.domain) + 1;
	      u = htons(24); memcpy(payload, &u, 2); payload+=2;
	      u = htons(strlen(_options.domain) + 1); memcpy(payload, &u, 2); payload+=2;
	      *payload++ = strlen(_options.domain);
	      memcpy(payload, _options.domain, strlen(_options.domain));
	      payload+=strlen(_options.domain);

	      if (status > -1) {
		data_len += 4 + 2;
		u = htons(13); memcpy(payload, &u, 2); payload+=2;
		u = htons(2); memcpy(payload, &u, 2); payload+=2;
		u = htons((uint16_t)status); memcpy(payload, &u, 2); payload+=2;
	      }

	      data_len += 4 + 2;
	      u = htons(8); memcpy(payload, &u, 2); payload+=2;
	      u = htons(2); memcpy(payload, &u, 2); payload+=2;
	      u = htons(1); memcpy(payload, &u, 2); payload+=2;

	      packet_udph->dst = udphdr->src;
	      packet_udph->src = udphdr->dst;
	      packet_udph->check = 0;
	      packet_udph->len = htons(data_len);
	  
	      packet_ip6h->ver_class_label = iphdr->ver_class_label;
	      packet_ip6h->next_header = iphdr->next_header;
	      packet_ip6h->hop_limit = 255;
	      packet_ip6h->data_len = htons(data_len);
	  
	      memcpy(packet_ip6h->src_addr, 
		     &this->rawif[0].address_v6.s6_addr, PKT_IPv6_ALEN);
	      memcpy(packet_ip6h->dst_addr, iphdr->src_addr, PKT_IPv6_ALEN);
	      
	      memcpy(packet_ethh->dst, ethh->src, PKT_ETH_ALEN);
	      memcpy(packet_ethh->src, dhcp_nexthop(dhcp), PKT_ETH_ALEN);
	      
	      chksum6(packet_ip6h);
	      
	      return dhcp_send(this, ctx->idx, ethh->src, 
			       packet, sizeofeth(packet) + 
			       sizeof(struct pkt_ip6hdr_t) + data_len);
	    }
	  }
	  break;
	}
      }
    }
    break;
  case PKT_IP_PROTO_TCP:
    break;
  }

  /* rewrite to ipv4 and forward */

  switch(iphdr->next_header) {
  case PKT_IP_PROTO_TCP:
  case PKT_IP_PROTO_UDP:
    if (iphdr->dst_addr[0]==0x11 &&
	iphdr->dst_addr[1]==0x12) {
      struct pkt_iphdr_t *ip4hdr;
      struct dhcp_conn_t *dhcpconn = 0;
      struct app_conn_t *appconn = 0;
      uint8_t prot = iphdr->next_header;
      uint32_t ip;

      static uint16_t idcnt = 1;
      
      if (dhcp_getconn(this, &dhcpconn, ethh->src, 0, 1)) {
	/* Could not allocate address */
	return 0; 
      }
      
      appconn = (struct app_conn_t *) dhcpconn->peer;
      if (!appconn) return 0;
      
      memcpy(&ip,&iphdr->dst_addr[12],4);
      memcpy(pack + 20, pack, sizeofeth(pack));
      ethh = pkt_ethhdr(pack + 20);
      ethh->prot = htons(PKT_ETH_PROTO_IP);
      ip4hdr = (struct pkt_iphdr_t *) (pack + 20 + sizeofeth(pack));
      ip4hdr->version_ihl = PKT_IP_VER_HLEN;
      ip4hdr->tos = 0;
      ip4hdr->tot_len = htons(ip_datalen + PKT_IP_HLEN);
      ip4hdr->id = idcnt++;
      ip4hdr->opt_off_high = 0x40;
      ip4hdr->off_low = 0;
      ip4hdr->ttl = 0xff;
      ip4hdr->protocol = prot;
      memcpy(&ip4hdr->saddr,&appconn->hisip.s_addr,4);
      memcpy(&ip4hdr->daddr,&ip,4);

      chksum(ip4hdr);

      return dhcp_receive_ip(ctx, pack + 20, len - 20);
    }
  }
  return 0;
}
#endif

#ifdef ENABLE_PPPOE
static 
int dhcp_pppoes(struct dhcp_ctx *ctx, uint8_t *packet, size_t length) {
  uint8_t *p = packet + sizeofeth(packet);
  struct pkt_pppoe_hdr_t *hdr = (struct pkt_pppoe_hdr_t *)p;
  if (hdr->version_type == PKT_PPPoE_VERSION &&
      hdr->code == 0x0000) {
    int len = ntohs(hdr->length);
    if (len > 0) {
      uint16_t ppp;
      p += sizeof(struct pkt_pppoe_hdr_t);
      ppp = ntohs(*((uint16_t *)p));
      log_dbg("PPPoE Session Code 0x%.2x Session 0x%.4x"
	      " Length %d Proto 0x%.4x", 
	      hdr->code, ntohs(hdr->session_id), len, ppp); 
      switch(ppp) {
      case PKT_PPP_PROTO_LCP:
	{
	  struct pkt_ppp_lcp_t *lcp = (struct pkt_ppp_lcp_t *)(p + 2);
	  log_dbg("PPP LCP code %.2x", lcp->code);
	  switch (lcp->code) {
	  case PPP_LCP_ConfigRequest:
	    {
	      uint8_t answer[1500];
	      
	      struct pkt_ethhdr_t *ethh = pkt_ethhdr(packet);
	      
	      struct pkt_ethhdr_t *answer_ethh;
	      
	      struct dhcp_conn_t *conn;
	      
	      struct pkt_pppoe_hdr_t *pppoe;
	      
	      struct pkt_ppp_lcp_t *nlcp;
	      
	      struct pkt_lcp_opthdr_t *tag;
	      
	      uint16_t lcplen, taglen, pos;
	      
	      /* Find / create connection
	       */
	      
	      if (dhcp_hashget(dhcp, &conn, ethh->src)) {
		if (dhcp_newconn(dhcp, &conn, ethh->src))
		  return 0; /* Out of connections */
	      }

	      dhcp_conn_set_idx(conn, ctx);
	      
	      /* Copy over original packet,
	       * get pointers to answer structure.
	       */
	      
	      memcpy(answer, packet, length); 
	      
	      answer_ethh = pkt_ethhdr(answer);
	      
	      pppoe = (struct pkt_pppoe_hdr_t *) 
		(answer + sizeofeth(answer));
	      
	      nlcp = (struct pkt_ppp_lcp_t *) 
		(answer + sizeofeth(answer) +
		 sizeof(struct pkt_pppoe_hdr_t) + 2);
	      
	      /* Start processing packet...
	       */
	      
	      nlcp->code = PPP_LCP_ConfigNak;
	      
	      lcplen = sizeof(struct pkt_ppp_lcp_t);
	      
	      pos = sizeofeth(answer) + sizeof(struct pkt_pppoe_hdr_t) + 2;
	      
	      taglen = 3;
	      tag = (struct pkt_lcp_opthdr_t *)(answer + pos + lcplen);
	      tag->type = PPP_LCP_OptAuthProto;
	      tag->length = taglen + 2;
	      lcplen += tag->length;
	      *(answer + pos + lcplen - 3) = 0xC2;
	      *(answer + pos + lcplen - 2) = 0x23;
	      *(answer + pos + lcplen - 1) = 0x81;
	      
	      length = 2;
	      length += lcplen;
	      
	      nlcp->length = htons(lcplen);
	      pppoe->length = htons(length);
	      
	      length += sizeof(struct pkt_pppoe_hdr_t);
	      length += sizeofeth(answer);
	      
	      /* Send back answer.
	       */
	      
	      memcpy(&answer_ethh->dst, &ethh->src, PKT_ETH_ALEN);
	      memcpy(&answer_ethh->src, dhcp_nexthop(dhcp), PKT_ETH_ALEN);
	      
	      dhcp_send(dhcp, ctx->idx, conn->hismac, answer, length);
	    }
	    break;
	  }
	}
	break;
      }
    }
  }
  return 0;
}

static 
int dhcp_pppoed(struct dhcp_ctx *ctx, uint8_t *packet, size_t length) {
  uint8_t *p = packet + sizeofeth(packet);
  struct pkt_pppoe_hdr_t *hdr = (struct pkt_pppoe_hdr_t *)p;
  if (hdr->version_type == PKT_PPPoE_VERSION &&
      hdr->session_id == 0x0000) {
    int len = ntohs(hdr->length);
    uint16_t t, l;
    
    uint8_t host_uniq[32];
    uint8_t host_uniq_len=0;
    
    log_dbg("PPPoE Discovery Code 0x%.2x Session 0x%.4x Length %d", 
	    hdr->code, ntohs(hdr->session_id), len); 
    
    p += sizeof(struct pkt_pppoe_hdr_t);
    
    while (len > 0) {
      struct pkt_pppoe_taghdr_t *tag = (struct pkt_pppoe_taghdr_t *) p;
      
      if (tag->type == 0x0000) break;
      
      t = ntohs(tag->type);
      l = ntohs(tag->length);
      
      switch(t) {
      case PPPoE_TAG_ServiceName:
	log_dbg("PPPoE Service-Name: %.*s",
		l, p + sizeof(struct pkt_pppoe_taghdr_t));
	break;
      case PPPoE_TAG_ACName:
	log_dbg("PPPoE AC-Name: %.*s",
		l, p + sizeof(struct pkt_pppoe_taghdr_t));
	break;
      case PPPoE_TAG_HostUniq:
	log_dbg("PPPoE Host-Uniq: %.*x",
		l * 2, p + sizeof(struct pkt_pppoe_taghdr_t));
	if (l > sizeof(host_uniq)) break;
	host_uniq_len = l;
	memcpy(host_uniq, p + sizeof(struct pkt_pppoe_taghdr_t), l);
	break;
      default:
	log_dbg("PPPoE Tag Type 0x%.4x = (%d)[%.*x]", 
		t, l, l * 2, p + sizeof(struct pkt_pppoe_taghdr_t));
	break;
      }
      
      l += sizeof(struct pkt_pppoe_taghdr_t);
      len -= l;
      p += l;
    }
    
    switch(hdr->code) {
    case PKT_PPPoE_PADT:
      /* terminate */
      break;
    case PKT_PPPoE_PADR:
    case PKT_PPPoE_PADI:
      {
	uint8_t answer[1500];
	
	struct pkt_ethhdr_t *ethh = pkt_ethhdr(packet);
	
	struct pkt_ethhdr_t *answer_ethh;
	
	struct dhcp_conn_t *conn;
	
	struct pkt_pppoe_hdr_t *pppoe;

	struct pkt_pppoe_taghdr_t *tag;
	
	uint16_t pos=0, taglen=0;
	
	/* Find / create connection
	 */
	
	if (dhcp_hashget(dhcp, &conn, ethh->src)) {
	  if (dhcp_newconn(dhcp, &conn, ethh->src))
	    return 0; /* Out of connections */
	}

	dhcp_conn_set_idx(conn, ctx);
	
	/* Copy over original packet,
	 * get pointers to answer structure.
	 */
	
	memcpy(answer, packet, length); 
	
	answer_ethh = pkt_ethhdr(answer);
	
	pppoe = (struct pkt_pppoe_hdr_t *) 
	  (answer + sizeofeth(answer));
	
	/* Start processing packet...
	 */
	
	pppoe->length = 0;
	switch(hdr->code) {
	case PKT_PPPoE_PADI:
	  pppoe->code = PKT_PPPoE_PADO;
	  break;
	case PKT_PPPoE_PADR:
	  pppoe->code = PKT_PPPoE_PADS;
	  pppoe->session_id = rand();
	  break;
	}
	
	length  = sizeofeth(answer);
	length += sizeof(struct pkt_pppoe_hdr_t);
	
	pos = length;
	
	taglen = 5;
	tag = (struct pkt_pppoe_taghdr_t *)(answer + length);
	tag->type = htons(PPPoE_TAG_ACName);
	tag->length = htons(taglen);
	length += sizeof(struct pkt_pppoe_taghdr_t);
	memcpy(answer + length, "chilli", 5);
	length += taglen;
	
	if (host_uniq_len) {
	  taglen = host_uniq_len;
	  tag = (struct pkt_pppoe_taghdr_t *)(answer + length);
	  tag->type = htons(PPPoE_TAG_HostUniq);
	  tag->length = htons(taglen);
	  length += sizeof(struct pkt_pppoe_taghdr_t);
	  memcpy(answer + length, host_uniq, host_uniq_len);
	  length += taglen;
	}
	
	pppoe->length = htons(length - pos);
	
	/* Send back answer.
	 */
	
	memcpy(&answer_ethh->dst, &ethh->src, PKT_ETH_ALEN);
	memcpy(&answer_ethh->src, dhcp_nexthop(dhcp), PKT_ETH_ALEN);
	
	dhcp_send(dhcp, dhcp_conn_idx(conn), conn->hismac, answer, length);
      }
      break;
    default:
      break;
    }
  }
  return 0;
}
#endif

#ifdef ENABLE_CLUSTER
#ifndef HAVE_OPENSSL
#error Clustiner requires OpenSSL support currently
#endif
static 
void update_peer(struct pkt_chillihdr_t *chillihdr) {
  struct chilli_peer *p = get_chilli_peer(chillihdr->from);
  p->last_update = mainclock_now();
  memcpy(&p->addr, &chillihdr->addr, sizeof(struct in_addr));
  memcpy(p->mac, chillihdr->mac, PKT_ETH_ALEN);
  p->state = chillihdr->state;
  if (p->state == PEER_STATE_ACTIVE) {
    get_chilli_peer(-1)->state = PEER_STATE_STANDBY;
  }
}

static 
int dhcp_chillipkt(struct dhcp_ctx *ctx, uint8_t *packet, size_t length) {
#ifdef HAVE_OPENSSL
  EVP_CIPHER_CTX cctx;
  
  unsigned char iv[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08 };
  
  struct pkt_ethhdr_t *ethh = pkt_ethhdr(packet);
  
  unsigned char *in = (unsigned char *)chilli_ethhdr(packet);
  unsigned char out[1500];
  
  int n = length - sizeofeth(packet);
  
  int olen = 0;
  int tlen = 0;

  if (_options.peerkey == 0)
    _options.peerkey = "hello!";
  
  log_dbg("CHILLI: peer %d decrypting %d bytes", 
	  _options.peerid, n);
  
  EVP_CIPHER_CTX_init(&cctx);
  EVP_DecryptInit(&cctx, EVP_bf_cbc(), 
		  (const unsigned char *)_options.peerkey, iv);
  
  if (EVP_DecryptUpdate(&cctx, out, &olen, in, n) != 1) {
    log_err(errno, "CHILLI: peer %d error in decrypt update",
	    _options.peerid);
  } else {
    log_dbg("CHILLI: peer %d decrypted %d bytes", 
	    _options.peerid, olen);
    if (EVP_DecryptFinal(&cctx, out + olen, &tlen) != 1) {
      log_err(errno, "CHILLI: peer %d error in decrypt final",
	      _options.peerid);
    } else {
      log_dbg("CHILLI: peer %d decrypted %d bytes", 
	      _options.peerid, tlen);
      
      olen += tlen;
      
      if (olen > sizeof(struct pkt_chillihdr_t)) {
	
	struct pkt_chillihdr_t *chilli_hdr =
	  (struct pkt_chillihdr_t *)out;

	char *cmd = "Unknown";

	switch(chilli_hdr->type) {
	case CHILLI_PEER_CMD:
	  cmd = "Cmd";
	  if (olen == sizeof(struct pkt_chillihdr_t) + 
	      sizeof(CMDSOCK_REQUEST)) {
	    CMDSOCK_REQUEST * req = 
	      (CMDSOCK_REQUEST *)(out + sizeof(struct pkt_chillihdr_t));
	    bstring s = bfromcstr("");
	    chilli_cmd(req, s, 0);
	    log_dbg("%s", s->data);
	    bdestroy(s);
	  }
	  break;
	  
	case CHILLI_PEER_INIT:
	  cmd = "Init";
	  if (chilli_hdr->from == _options.peerid) {
	    log_err(0, "peer %d possible conflicting peerid, oops",
		    _options.peerid);
	  } else {
	    dhcp_sendCHILLI(CHILLI_PEER_HELLO, 0, 0);
	    update_peer(chilli_hdr);
	  }
	  break;
	  
	case CHILLI_PEER_HELLO:
	  cmd = "Hello";
	  if (chilli_hdr->from == _options.peerid) {
	    log_err(0, "peer %d possible conflicting peerid",
		    _options.peerid);
	  } else {
	    update_peer(chilli_hdr);
	  }
	  break;
	}
	
	log_dbg("CHILLI: peer %d received %s from "
		"%.2x:%.2x:%.2x:%.2x:%.2x:%.2x peerid %d len=%d",
		_options.peerid, cmd,
		ethh->src[0],ethh->src[1],ethh->src[2],
		ethh->src[3],ethh->src[4],ethh->src[5],
		chilli_hdr->from, olen);
      }
    }
  }

  EVP_CIPHER_CTX_cleanup(&cctx);

#else
#warning Cluster feature requires OpenSSL
#endif
  return 0;
}

static 
char dhcp_ignore(uint16_t prot, uint8_t *packet, size_t length) {
  struct pkt_ethhdr_t *ethh = pkt_ethhdr(packet);
  char ignore = 0;
  
  ignore = get_chilli_peer(-1)->state != PEER_STATE_ACTIVE;
  
  if (ignore)
    log_dbg("ignore: src=%.2x:%.2x:%.2x:%.2x:%.2x:%.2x "
	    "dst=%.2x:%.2x:%.2x:%.2x:%.2x:%.2x prot=%.4x %d len=%d",
	    ethh->src[0],ethh->src[1],ethh->src[2],
	    ethh->src[3],ethh->src[4],ethh->src[5],
	    ethh->dst[0],ethh->dst[1],ethh->dst[2],
	    ethh->dst[3],ethh->dst[4],ethh->dst[5],
	    prot, (int)prot, length);
  
  return ignore;
}

static time_t last_peer_update = 0;
static time_t peer_update_time = 10;
void dhcp_peer_update(char force) {
  struct chilli_peer *me = get_chilli_peer(-1);
  time_t now = mainclock_now();
  if (force || now > (last_peer_update + peer_update_time)) {
    dhcp_sendCHILLI(CHILLI_PEER_HELLO, 0, 0);
    last_peer_update = now;
    me->last_update = now;
  }
}
#endif

static 
int dhcp_decaps_cb(void *pctx, struct pkt_buffer *pb) {
  struct dhcp_ctx *ctx = (struct dhcp_ctx *)pctx;
  uint16_t prot = 0;
  char ignore = 0;

  uint8_t *packet = pkt_buffer_head(pb);
  size_t length = pkt_buffer_length(pb);

  int min_length = sizeof(struct pkt_ethhdr_t);
  
  if (length < min_length) {
    log_dbg("bad packet length %d", length);
    return 0;
  }
  
#ifdef ENABLE_IEEE8021Q
  if (_options.ieee8021q) {
    if (is_8021q(packet)) {
      struct pkt_ethhdr8021q_t *ethh;
      min_length = sizeof(struct pkt_ethhdr8021q_t);
      if (length < min_length) {
	log_dbg("bad packet length %d", length);
	return 0;
      }
      ethh = ethhdr8021q(packet);
      prot = ntohs(ethh->prot);
    } else if (_options.ieee8021q_only) {
      return 0;
    }
  }
#endif

  if (!prot) {
    struct pkt_ethhdr_t *ethh = pkt_ethhdr(packet);
    prot = ntohs(ethh->prot);
  }

#if(_debug_ > 1)
  if (_options.debug) {
    struct pkt_ethhdr_t *ethh = pkt_ethhdr(packet);
    log_dbg("dhcp_decaps: src=%.2x:%.2x:%.2x:%.2x:%.2x:%.2x "
	    "dst=%.2x:%.2x:%.2x:%.2x:%.2x:%.2x prot=%.4x %d len=%d",
	    ethh->src[0],ethh->src[1],ethh->src[2],
	    ethh->src[3],ethh->src[4],ethh->src[5],
	    ethh->dst[0],ethh->dst[1],ethh->dst[2],
	    ethh->dst[3],ethh->dst[4],ethh->dst[5],
	    prot, (int)prot, length);
  }
#endif

#ifdef ENABLE_CLUSTER
  ignore = dhcp_ignore(prot, packet, length);
#endif

  if (prot < 1518) {
#ifdef ENABLE_IEEE8023
    if (!ignore) {
      uint8_t *p = packet + sizeofeth(packet);
      struct pkt_llc_t * llc = (struct pkt_llc_t *)p;
      
      if (llc->dsap == 0xAA &&
	  llc->ssap == 0xAA &&
	  llc->cntl == 0x03) {
	
	struct pkt_llc_snap_t * snap = 
	  (struct pkt_llc_snap_t *)(p + sizeof(struct pkt_llc_t));
	
	log_dbg("Layer2 PROT: IEEE 802.3 LLC SNAP "
		" EtherType 0x%.4x",
		ntohs(snap->type));
      } else {
	log_dbg("Layer2 PROT: Likely IEEE 802.3: "
		"length %d dsap=0x%.2x ssap=0x%.2x ctrl=0x%.2x", 
		(int) prot, llc->dsap, llc->ssap, llc->cntl); 
      }
    }
#endif

#if(0)
#ifdef ENABLE_LAYER3
    if (!_options.layer3)
#endif
	log_dbg("Layer2 PROT: 0x%.4x dropped", prot); 
#endif

    return 0;
  }
  
  switch (prot) {

#ifdef ENABLE_EAPOL
  case PKT_ETH_PROTO_EAPOL: 
    if (!ignore && _options.eapolenable)
      return dhcp_receive_eapol(ctx, packet);
    break;
#endif

  case PKT_ETH_PROTO_ARP:  
    if (!ignore)
      return dhcp_receive_arp(ctx, packet, length);
    break;

  case PKT_ETH_PROTO_IP:  
#ifdef ENABLE_IPV6
    if (_options.ipv6 && _options.ipv6only)
      ignore = 1;
#endif
    if (!ignore)
      return dhcp_receive_ip(ctx, packet, length);
    break;

#ifdef ENABLE_PPPOE
  case PKT_ETH_PROTO_PPPOES:
    if (!ignore)
      return dhcp_pppoes(ctx, packet, length);
    break;

  case PKT_ETH_PROTO_PPPOED: 
    if (!ignore)
      return dhcp_pppoed(ctx, packet, length);
    break;
#endif

#ifdef ENABLE_CLUSTER
  case PKT_ETH_PROTO_CHILLI:
    return dhcp_chillipkt(ctx, packet, length);
#endif

#ifdef ENABLE_IPV6
  case PKT_ETH_PROTO_IPv6:
    if (_options.ipv6)
      return dhcp_receive_ipv6(ctx, packet, length);
#endif

  case PKT_ETH_PROTO_PPP:
  case PKT_ETH_PROTO_IPX:
  default: 
    if (!ignore)
#ifdef ENABLE_LAYER3
      if (!_options.layer3)
#endif
	log_dbg("Layer2 PROT: 0x%.4x dropped", prot); 
    break;
  }

  return 0;
}

/**
 * Call this function when a new IP packet has arrived. This function
 * should be part of a select() loop in the application.
 **/
int dhcp_decaps(struct dhcp_t *this, int idx) {
  ssize_t length = -1;
  net_interface *iface = 0;
  struct dhcp_ctx ctx;

#ifdef HAVE_NETFILTER_QUEUE
  if (idx) {
    uint8_t buf[1600];

    switch (idx) {
    case 1:
      iface = &this->qif_in;
      break;
    case 2:
      iface = &this->qif_out;
      break;
    }
    if (iface) {
      length = recv(iface->fd, buf, sizeof(buf), MSG_DONTWAIT);
      if (length > 0) {
	nfq_handle_packet(iface->h, (char *)buf, length);
      }
      return length;
    }
  }
#else
#ifdef ENABLE_MULTILAN
  iface = &this->rawif[idx];
#else
  iface = &this->rawif[0];
#endif
#endif

  ctx.parent = this;
  ctx.idx = idx;

  if ((length = net_read_dispatch_eth(iface, dhcp_decaps_cb, &ctx)) < 0) 
    return -1;

  return length;
}

static 
int dhcp_ethhdr(struct dhcp_conn_t *conn, uint8_t *packet, uint8_t *hismac, uint8_t *nexthop, uint16_t prot) {
#ifdef ENABLE_IEEE8021Q
  if (_options.ieee8021q && conn->tag8021q) {
    struct pkt_ethhdr8021q_t *pack_ethh = ethhdr8021q(packet);
    copy_mac6(pack_ethh->dst, hismac);
    copy_mac6(pack_ethh->src, nexthop);
    pack_ethh->prot = htons(prot);
    pack_ethh->tpid = htons(PKT_ETH_PROTO_8021Q);
    pack_ethh->pcp_cfi_vid = conn->tag8021q;
  } else 
#endif
  {
    struct pkt_ethhdr_t *pack_ethh = pkt_ethhdr(packet);
    copy_mac6(pack_ethh->dst, hismac);
    copy_mac6(pack_ethh->src, nexthop);
    pack_ethh->prot = htons(prot);
  }
  return 0;
}

int dhcp_relay_decaps(struct dhcp_t *this, int idx) {
  struct dhcp_tag_t *message_type = 0;
  struct dhcp_conn_t *conn = 0;
  struct dhcp_packet_t packet;
  struct sockaddr_in addr;
  socklen_t fromlen = sizeof(addr);
  ssize_t length;

  uint8_t fullpack[1500];

  if ((length = recvfrom(this->relayfd, &packet, sizeof(packet), 0,
                         (struct sockaddr *) &addr, &fromlen)) <= 0) {
    log_err(errno, "recvfrom() failed");
    return -1;
  }

  if (length < 44) {
    log_dbg("DHCP packet too short");
    return -1;
  }

  if (packet.op != 2) {
    log_dbg("Ignored non-relay reply DHCP packet");
    return -1;
  }

  log_dbg("DHCP relay response from %s of length %d received",
	  inet_ntoa(addr.sin_addr), length);

  if (addr.sin_addr.s_addr != _options.dhcpgwip.s_addr &&
      addr.sin_addr.s_addr != _options.dhcprelayip.s_addr && 
      addr.sin_addr.s_addr != _options.uamlisten.s_addr) {
    log_dbg("Received DHCP response from host (%s) other than our gateway",
	    inet_ntoa(addr.sin_addr));
    return -1;
  }

  if (addr.sin_port != htons(_options.dhcpgwport)) {
    log_dbg("Received DHCP response from port (%d) other than our gateway",
	    ntohs(addr.sin_port));
    return -1;
  }

  if (dhcp_gettag(&packet, length, &message_type, 
		  DHCP_OPTION_MESSAGE_TYPE)) {
    log_err(0, "no message type");
    return -1;
  }
  
  if (message_type->l != 1) {
    log_err(0, "wrong message type length");
    return -1; /* Wrong length of message type */
  }
  
  if (dhcp_hashget(this, &conn, packet.chaddr)) {
    /* Allocate new connection */
    if (dhcp_newconn(this, &conn, packet.chaddr)) {
      log_err(0, "out of connections");
      return 0; /* Out of connections */
    }
  }

  if (conn->authstate == DHCP_AUTH_NONE ||
      conn->authstate == DHCP_AUTH_DNAT)
    this->cb_request(conn, (struct in_addr *)&packet.yiaddr, 0, 0);

  packet.giaddr = 0;

  memset(&fullpack, 0, sizeof(fullpack));

  dhcp_ethhdr(conn, fullpack, conn->hismac, 
	      dhcp_nexthop(this), PKT_ETH_PROTO_IP);

  {
    struct pkt_iphdr_t *fullpack_iph = pkt_iphdr(fullpack);
    struct pkt_udphdr_t *fullpack_udph = pkt_udphdr(fullpack);
    
    fullpack_iph->version_ihl = PKT_IP_VER_HLEN;
    fullpack_iph->tot_len = htons(length + PKT_UDP_HLEN + PKT_IP_HLEN);
    fullpack_iph->ttl = 0x10;
    fullpack_iph->protocol = 0x11;
    
    fullpack_iph->saddr = _options.dhcplisten.s_addr;
    fullpack_udph->src = htons(DHCP_BOOTPS);
    fullpack_udph->len = htons(length + PKT_UDP_HLEN);
    
    fullpack_udph->dst = htons(DHCP_BOOTPC);
    fullpack_iph->daddr = ~0; 

    if (packet.ciaddr) {
      log_dbg("DHCP: CIAddr");
      fullpack_iph->daddr = packet.ciaddr; 
    } else if (packet.flags[0] & 0x80 || message_type->v[0] == DHCPNAK) {
      log_dbg("DHCP: Nak or Broadcast");
      packet.flags[0] = 0x80;
      dhcp_ethhdr(conn, fullpack, bmac, dhcp_nexthop(this), PKT_ETH_PROTO_IP);
    } else if (packet.yiaddr) {
      log_dbg("DHCP: YIAddr");
      fullpack_iph->daddr = packet.yiaddr; 
    }
    
    /*
     * Copy DHCP packet for forwarding
     */
    memcpy(pkt_dhcppkt(fullpack), &packet, length);

    { 
      /* 
       * Rewrite the server-id, otherwise will not get 
       * subsequent requests 
       */
      struct dhcp_tag_t *tag = 0;
      if (!dhcp_gettag(pkt_dhcppkt(fullpack), length, 
		       &tag, DHCP_OPTION_SERVER_ID)) {
	memcpy(tag->v, &_options.dhcplisten.s_addr, 4);
      }
    }
    
    chksum(fullpack_iph);
    
    addr.sin_addr.s_addr = fullpack_iph->daddr;
    log_dbg("Sending DHCP relay response %s:%d %d",
	    inet_ntoa(addr.sin_addr),
	    ntohs(fullpack_udph->dst),
	    length + sizeofudp(fullpack));

    addr.sin_addr.s_addr = fullpack_iph->saddr;
    log_dbg("Sending DHCP from %s:%d",
	    inet_ntoa(addr.sin_addr),
	    ntohs(fullpack_udph->src));
    
    OTHER_SENDING(conn,fullpack_iph);
    return dhcp_send(this, dhcp_conn_idx(conn), conn->hismac, fullpack, 
		     length + sizeofudp(fullpack));
  }
}

/**
 * dhcp_data_req()
 * Call this function to send an IP packet to the peer.
 * Called from the tun_ind function. This method is passed either
 * an Ethernet frame or an IP packet. 
 **/
int dhcp_data_req(struct dhcp_conn_t *conn, 
		  struct pkt_buffer *pb, int ethhdr) {
  struct dhcp_t *this = conn->parent;

  uint8_t *packet = pkt_buffer_head(pb);
  size_t length = pkt_buffer_length(pb);

#ifdef ENABLE_IEEE8021Q
  uint16_t tag = conn->tag8021q;
#else
  uint16_t tag = 0;
#endif

  char do_checksum = 0;
  char allowed = 0;

  int authstate = 0;

  if (ethhdr) { 
    /* 
     * Ethernet frame 
     */
    size_t hdrplus = sizeofeth2(tag) - sizeofeth(packet);
    if (hdrplus > 0) {
      if (pb->offset < hdrplus) {
	log_err(0, "bad buffer off=%d hdrplus=%d",
		(int) pb->offset, (int) hdrplus);
	return 0;
      }
      pkt_buffer_grow(pb, hdrplus);
      packet = pkt_buffer_head(pb);
      length = pkt_buffer_length(pb);
    }
  } else { 
    size_t hdrlen = sizeofeth2(tag);
    if (pb->offset < hdrlen) {
      log_err(0, "bad buffer off=%d hdr=%d",
	      (int) pb->offset, (int) hdrlen);
      return 0;
    }
    pkt_buffer_grow(pb, hdrlen);
    packet = pkt_buffer_head(pb);
    length = pkt_buffer_length(pb);
#if(_debug_ > 1)
    log_dbg("adding %d to IP frame length %d", hdrlen, length);
#endif
  }

  if (!this) {
      log_warn(0, "DHCP connection no longer valid");
      return 0;
  }

#ifdef ENABLE_LAYER3
  /*
   *  A bit of a hack to decouple the one-to-one relationship
   *  of "dhcp connections" and "app connections". Look for the 
   *  app session based on IP (Layer3) and adjust authstate.
   */
  switch (conn->authstate) {
  case DHCP_AUTH_ROUTER:
    {
      struct app_conn_t *appconn = 
	dhcp_get_appconn_pkt(conn, pkt_iphdr(packet), 1);

      if (!appconn) {
	return 0;
      }
      
      switch (appconn->s_state.authenticated) {
      case 1:
	authstate = DHCP_AUTH_PASS;
	break;
      default:
	authstate = DHCP_AUTH_DNAT;
	break;
      }
    }
    break;
    
  default:
    authstate = conn->authstate;
    break;
  }
#else
  authstate = conn->authstate;
#endif
  
  dhcp_ethhdr(conn, packet, conn->hismac, dhcp_nexthop(this), PKT_ETH_PROTO_IP);
  
  switch (dhcp_dnsunDNAT(conn, packet, &length, &do_checksum)) {
  case 0:  /* Not DNS */break;
  case 1:  /* Allowed DNS */ allowed = 1; break;
  default: /* Drop */ return 0;
  }
  
  switch (authstate) {
    
  case DHCP_AUTH_PASS:
  case DHCP_AUTH_AUTH_TOS:
    dhcp_postauthDNAT(conn, packet, length, 1, &do_checksum);
    break;
    
  case DHCP_AUTH_SPLASH:
  case DHCP_AUTH_UNAUTH_TOS:
    dhcp_undoDNAT(conn, packet, &length, 0, &do_checksum);
    break;

  case DHCP_AUTH_DNAT:
  case DHCP_AUTH_NONE:
    /* undo destination NAT */
    if (dhcp_undoDNAT(conn, packet, &length, 1, &do_checksum) && !allowed) { 
#if(_debug_ > 1)
      log_dbg("dhcp_undoDNAT() returns true");
#endif
      return 0;
    }
    break;

  case DHCP_AUTH_DROP: 
#if(_debug_)
    log_dbg("drop");
#endif
    return 0;
  default: 
#if(_debug_)
    log_dbg("unhandled authstate %d", authstate);
#endif
    return 0;
  }

#ifdef ENABLE_IPV6
  if (_options.ipv6 && _options.ipv6only) {
    struct pkt_iphdr_t *iph = 
      (struct pkt_iphdr_t *)(packet + sizeofeth2(tag));

    struct pkt_ethhdr_t *ethh;

    struct pkt_ip6hdr_t *ip6hdr;

    uint8_t *p = packet;

    uint8_t prot = iph->protocol;
    uint16_t iplen = iph->tot_len;
    uint32_t saddr = iph->saddr;
    uint8_t *payload, *m;

    pb->length = length;

#if(_debug_ > 1)
    log_dbg("adding 20 to frame length %d", length);
#endif

    pkt_buffer_grow(pb, 20);
    packet = pkt_buffer_head(pb);
    length = pkt_buffer_length(pb);

    memcpy(packet, p, sizeofeth2(tag));
    ethh = pkt_ethhdr(packet);
    ethh->prot = htons(PKT_ETH_PROTO_IPv6);

    ip6hdr = (struct pkt_ip6hdr_t *) (packet + sizeofeth2(tag));
    ip6hdr->ver_class_label = htonl(0x60000000);
    ip6hdr->data_len = htons(ntohs(iplen) - PKT_IP_HLEN);
    ip6hdr->next_header = prot;
    ip6hdr->hop_limit = 0xff;

    payload = ip6hdr->src_addr;
    m = (uint8_t *)&saddr; 
    ipv6_nat64_pack(payload, m);

    m = conn->hismac;
    ipv6_eui64_pack(payload, m);

    chksum6(ip6hdr);

  } else
#endif
  if (do_checksum)
    chksum(pkt_iphdr(packet));

  return dhcp_send(this, dhcp_conn_idx(conn), conn->hismac, packet, length);
}

/**
 * dhcp_sendARP()
 * Send ARP message to peer
 **/
static 
int dhcp_sendARP(struct dhcp_conn_t *conn, uint8_t *pack, size_t len) {
  uint8_t packet[1500];
  struct dhcp_t *this = conn->parent;
  struct in_addr reqaddr;

  struct arp_packet_t *pack_arp = pkt_arppkt(pack);

  struct pkt_ethhdr_t *packet_ethh;
  struct arp_packet_t *packet_arp;

  /* Get local copy */
  memcpy(&reqaddr.s_addr, pack_arp->tpa, PKT_IP_ALEN);

  /* Check that request is within limits */

  /* Get packet default values */
  memset(packet, 0, sizeof(packet));
  copy_ethproto(pack, packet);

  packet_ethh = pkt_ethhdr(packet);
  packet_arp = pkt_arppkt(packet);
	 
  /* ARP Payload */
  packet_arp->hrd = htons(DHCP_HTYPE_ETH);
  packet_arp->pro = htons(PKT_ETH_PROTO_IP);
  packet_arp->hln = PKT_ETH_ALEN;
  packet_arp->pln = PKT_IP_ALEN;
  packet_arp->op  = htons(DHCP_ARP_REPLY);

  /* Source address */
  memcpy(packet_arp->spa, &reqaddr.s_addr, PKT_IP_ALEN);
  memcpy(packet_arp->sha, dhcp_nexthop(this), PKT_ETH_ALEN);

  /* Target address */
  memcpy(packet_arp->tha, &conn->hismac, PKT_ETH_ALEN);
  memcpy(packet_arp->tpa, &conn->hisip.s_addr, PKT_IP_ALEN);

  log_dbg("ARP: Replying to %s / %.2X-%.2X-%.2X-%.2X-%.2X-%.2X", 
	  inet_ntoa(conn->hisip),
	  conn->hismac[0], conn->hismac[1], conn->hismac[2],
	  conn->hismac[3], conn->hismac[4], conn->hismac[5]);
  
  /* Ethernet header */
  memcpy(packet_ethh->dst, conn->hismac, PKT_ETH_ALEN);
  memcpy(packet_ethh->src, dhcp_nexthop(this), PKT_ETH_ALEN);

  OTHER_SENDING_LEN(conn,sizeof(struct arp_packet_t));
  return dhcp_send(this, dhcp_conn_idx(conn), conn->hismac, 
		   packet, sizeofarp(packet));
}

int dhcp_receive_arp(struct dhcp_ctx *ctx, uint8_t *pack, size_t len) {
  struct dhcp_t *this = ctx->parent;
  
  struct dhcp_conn_t *conn = 0;
  struct in_addr reqaddr;
  struct in_addr taraddr;

  struct pkt_ethhdr_t *pack_ethh = pkt_ethhdr(pack);
  struct arp_packet_t *pack_arp = pkt_arppkt(pack);

  if (len < sizeofeth(pack) + sizeof(struct arp_packet_t)) {
    log_err(0, "ARP too short %d < %d", len, 
	    sizeofeth(pack) + sizeof(struct arp_packet_t));
    return 0;
  }
  
  if (ntohs(pack_arp->hrd) != 1 ||       /* Ethernet Hardware */
      pack_arp->hln != PKT_ETH_ALEN ||   /* MAC Address Size */
      pack_arp->pln != PKT_IP_ALEN) {    /* IP Address Size */
    log_err(0, "ARP reject hrd=%d hln=%d pln=%d", 
	    ntohs(pack_arp->hrd), pack_arp->hln, pack_arp->pln);
    return 0;
  }

  /* Check that this is ARP request */
  if (pack_arp->op != htons(DHCP_ARP_REQUEST)) {
    log_dbg("ARP: Received other ARP than request!");
    return 0;
  }

  /* Check that MAC address is our MAC or Broadcast */
  if ((memcmp(pack_ethh->dst, dhcp_nexthop(this), PKT_ETH_ALEN)) && 
      (memcmp(pack_ethh->dst, bmac, PKT_ETH_ALEN))) {
    log_dbg("ARP: Received ARP request for other destination!");
    return 0;
  }
  
  /* get sender IP address */
  memcpy(&reqaddr.s_addr, &pack_arp->spa, PKT_IP_ALEN);

  /* get target IP address */
  memcpy(&taraddr.s_addr, &pack_arp->tpa, PKT_IP_ALEN);

#ifdef ENABLE_LAYER3
  if (_options.layer3) {
    if (taraddr.s_addr == _options.dhcplisten.s_addr) {
      if (dhcp_hashget(this, &conn, pack_arp->sha)) {
	log_dbg("ARP: Address not found: %s", inet_ntoa(reqaddr));
	if (dhcp_newconn(this, &conn, pack_arp->sha)) {
	  log_warn(0, "ARP: out of connections");
	  return 0; /* Out of connections */
	}
      }
      dhcp_conn_set_idx(conn, ctx);
      dhcp_sendARP(conn, pack, len);
#ifdef ENABLE_IEEE8021Q
      if (_options.ieee8021q) {
#if(_debug_ > 1)
	log_dbg("%s calling dhcp_checktag",__FUNCTION__);
#endif
	dhcp_checktag(conn, pack);
      }
#endif
    }
    OTHER_RECEIVED_LEN(conn, len-PKT_ETH_HLEN);
    return 0;
  }
#endif

  /* Check to see if we know MAC address. */
  if (dhcp_hashget(this, &conn, pack_arp->sha)) {
    log_dbg("ARP: Address not found: %s", inet_ntoa(reqaddr));
    
    /* Do we allow dynamic allocation of IP addresses? */
    if (!this->allowdyn
#ifdef ENABLE_UAMANYIP
	&& !_options.uamanyip
#endif
	) {
      log_dbg("ARP: Unknown client and no dynip: %s", inet_ntoa(taraddr));
      return 0; 
    }
    
    /* Allocate new connection */
    if (dhcp_newconn(this, &conn, pack_arp->sha)) {
      log_warn(0, "ARP: out of connections");
      return 0; /* Out of connections */
    }
  }

  OTHER_RECEIVED_LEN(conn, len-PKT_ETH_HLEN);
  dhcp_conn_set_idx(conn, ctx);

#ifdef ENABLE_IEEE8021Q
  if (_options.ieee8021q) {
#if(_debug_ > 1)
    log_dbg("%s calling dhcp_checktag",__FUNCTION__);
#endif
    dhcp_checktag(conn, pack);
  }
#endif

  log_dbg("ARP: %.2X-%.2X-%.2X-%.2X-%.2X-%.2X asking about %s", 
	  conn->hismac[0], conn->hismac[1], conn->hismac[2],
	  conn->hismac[3], conn->hismac[4], conn->hismac[5],
	  inet_ntoa(taraddr));
  
  if (conn->authstate == DHCP_AUTH_DROP) {
    return 0;
  }
  
  /* if no sender ip, then client is checking their own ip */
  if (!reqaddr.s_addr) {
    /* XXX: lookup in ippool to see if we really do know who has this */
    /* XXX: it should also ack if *we* are that ip */
    log_dbg("ARP: Ignoring self-discovery: %s", inet_ntoa(taraddr));
    
    /* If a static ip address... */
    this->cb_request(conn, &taraddr, 0, 0);
    
    return 0;
  }
  
  if (!memcmp(&reqaddr.s_addr, &taraddr.s_addr, 4)) { 

    /* Request an IP address */
#ifdef ENABLE_UAMANYIP
    if (_options.uamanyip /*or static ip*/ &&
	conn->authstate == DHCP_AUTH_NONE) {
      this->cb_request(conn, &reqaddr, 0, 0);
    }
#endif

    log_dbg("ARP: Ignoring gratuitous arp %s", inet_ntoa(taraddr));
    return 0;
  }

  /* Is ARP request for clients own address: Ignore */
  if (!memcmp(&conn->hisip.s_addr, &taraddr.s_addr, 4)) {
    log_dbg("ARP: hisip equals target ip: %s", inet_ntoa(conn->hisip));
    return 0;
  }

  if (conn->authstate == DHCP_AUTH_NONE)
    this->cb_request(conn, &reqaddr, 0, 0);
  
  if (memcmp(&_options.dhcplisten.s_addr, &taraddr.s_addr, 4) &&
      !conn->hisip.s_addr
#ifdef ENABLE_UAMANYIP
      && !_options.uamanyip
#endif
      ) {
    /* Only reply if he was allocated an address,
       unless it was a request for the gateway dhcplisten. */
    log_dbg("ARP: request did not come from known client");
    return 0;
  }
  
#ifdef ENABLE_UAMANYIP
  if (!_options.uamanyip) {
#endif
    /* If ARP request outside of mask: Ignore 
    if (reqaddr.s_addr &&
	(conn->hisip.s_addr & conn->hismask.s_addr) !=
	(reqaddr.s_addr & conn->hismask.s_addr)) {
      log_dbg("ARP: request not in our subnet");
      return 0;
    }
    */
    
    if (memcmp(&conn->ourip.s_addr, &taraddr.s_addr, 4) &&
	memcmp(&_options.dhcplisten.s_addr, &taraddr.s_addr, 4)) { 
      /* if ourip differs from target ip */
      if (_options.debug) {
	log_dbg("ARP: Did not ask for router address: %s", inet_ntoa(conn->ourip));
	log_dbg("ARP: Asked for target: %s", inet_ntoa(taraddr));
      }
      return 0; /* Only reply if he asked for his router address */
    }
#ifdef ENABLE_UAMANYIP
  }
  else if (_options.uamanyipex_addr.s_addr &&
	   (taraddr.s_addr & _options.uamanyipex_mask.s_addr) == 
	   _options.uamanyipex_addr.s_addr) {
    log_dbg("ARP: Request for %s in uamanyipex subnet, ignoring", 
	    inet_ntoa(taraddr));
    return 0;
  }
  else if ((taraddr.s_addr != _options.dhcplisten.s_addr) &&
	  ((taraddr.s_addr & _options.mask.s_addr) == _options.net.s_addr)) {
    /* when uamanyip is on we should ignore arp requests that ARE within
       our subnet except of course the ones for ourselves */
    log_dbg("ARP: Request for %s other than us within our subnet(uamanyip on), ignoring", 
	    inet_ntoa(taraddr));
    return 0;
  }
#endif

  conn->lasttime = mainclock_now();
  dhcp_sendARP(conn, pack, len);
  return 0;
}


#ifdef ENABLE_EAPOL
int dhcp_senddot1x(struct dhcp_conn_t *conn, uint8_t *pack, size_t len) {
  struct dhcp_t *this = conn->parent;
  return dhcp_send(this, dhcp_conn_idx(conn), conn->hismac, pack, len);
}

/**
 * eapol_sendNAK()
 * Send of a EAPOL negative acknowledge message to a peer.
 * NAK messages are always sent to broadcast IP address (
 * except when using a EAPOL relay server)
 **/
int dhcp_sendEAP(struct dhcp_conn_t *conn, uint8_t *pack, size_t len) {

  struct dhcp_t *this = conn->parent;

  uint8_t packet[1500];
  struct pkt_ethhdr_t *packet_ethh;
  struct pkt_dot1xhdr_t *packet_dot1x;

  copy_ethproto(pack, packet);

  packet_ethh = pkt_ethhdr(packet);
  packet_dot1x = pkt_dot1xhdr(packet);

  /* Ethernet header */
  memcpy(packet_ethh->dst, conn->hismac, PKT_ETH_ALEN);
  memcpy(packet_ethh->src, dhcp_nexthop(this), PKT_ETH_ALEN);

  packet_ethh->prot = htons(PKT_ETH_PROTO_EAPOL);
  
  /* 802.1x header */
  packet_dot1x->ver  = 1;
  packet_dot1x->type = 0; /* EAP */
  packet_dot1x->len =  htons((uint16_t)len);

  memcpy(pkt_eappkt(packet), pack, len);
  
  OTHER_SENDING_LEN(conn,len);
  return dhcp_send(this, dhcp_conn_idx(conn), conn->hismac,
		   packet, (PKT_ETH_HLEN + 4 + len));
}

int dhcp_sendEAPreject(struct dhcp_conn_t *conn, uint8_t *pack, size_t len) {

  /*struct dhcp_t *this = conn->parent;*/

  struct eap_packet_t packet;

  if (pack) {
    dhcp_sendEAP(conn, pack, len);
  }
  else {
    memset(&packet, 0, sizeof(packet));
    packet.code      =  4;
    packet.id        =  1; /* TODO ??? */
    packet.length    =  htons(4);
  
    dhcp_sendEAP(conn, (uint8_t *)&packet, 4);
  }

  return 0;

}

int dhcp_receive_eapol(struct dhcp_ctx *ctx, uint8_t *pack) {
  struct dhcp_t *this = ctx->parent;
  struct dhcp_conn_t *conn = NULL;
  unsigned char const amac[PKT_ETH_ALEN] =
    {0x01, 0x80, 0xc2, 0x00, 0x00, 0x03};

  struct pkt_ethhdr_t *pack_ethh = pkt_ethhdr(pack);
  struct pkt_dot1xhdr_t *pack_dot1x = pkt_dot1xhdr(pack);

  /* Check to see if we know MAC address. */
  if (!dhcp_hashget(this, &conn, pack_ethh->src)) {
    log_dbg("Address found");
  }
  else {
    log_dbg("Address not found");
  }
  
  log_dbg("IEEE 802.1x Packet: %.2x, %.2x %d",
	  pack_dot1x->ver, pack_dot1x->type,
	  ntohs(pack_dot1x->len));
  
  /* Check that MAC address is our MAC, Broadcast or authentication MAC */
  if ((memcmp(pack_ethh->dst, this->rawif[0].hwaddr, PKT_ETH_ALEN)) && 
      (memcmp(pack_ethh->dst, bmac, PKT_ETH_ALEN)) && 
      (memcmp(pack_ethh->dst, amac, PKT_ETH_ALEN)))
    return 0;
  
  if (pack_dot1x->type == 1) { /* Start */
    uint8_t p[1500];
    struct pkt_dot1xhdr_t *p_dot1x;
    struct eap_packet_t *p_eap;
    
    /* Allocate new connection */
    if (conn == NULL) {
      if (dhcp_newconn(this, &conn, pack_ethh->src))
	return 0; /* Out of connections */
    }

    dhcp_conn_set_idx(conn, ctx);

#ifdef ENABLE_IEEE8021Q
    if (_options.ieee8021q) {
#if(_debug_ > 1)
      log_dbg("%s calling dhcp_checktag",__FUNCTION__);
#endif
      dhcp_checktag(conn, pack);
    }
#endif
    
    memset(&p, 0, sizeof(p));
    dhcp_ethhdr(conn, p, pack_ethh->src, 
		dhcp_nexthop(this), PKT_ETH_PROTO_EAPOL);

    p_dot1x = pkt_dot1xhdr(p);
    p_eap = pkt_eappkt(p);

    /* Ethernet header */

    /* 802.1x header */
    p_dot1x->ver  = 1;
    p_dot1x->type = 0; /* EAP */
    p_dot1x->len =  htons(5);
    
    /* EAP Packet */
    p_eap->code      =  1;
    p_eap->id        =  1;
    p_eap->length    =  htons(5);
    p_eap->type      =  1; /* Identity */

    dhcp_senddot1x(conn, p, PKT_ETH_HLEN + 4 + 5);
    return 0;
  }
  else if (pack_dot1x->type == 0) { /* EAP */

    /* TODO: Currently we only support authentications starting with a
       client sending a EAPOL start message. Need to also support
       authenticator initiated communications. */
    if (!conn)
      return 0;

    conn->lasttime = mainclock_now();
    
    if (this->cb_eap_ind)
      this->cb_eap_ind(conn, (uint8_t *)pkt_eappkt(pack), 
		       ntohs(pkt_eappkt(pack)->length));

    return 0;
  }
  else { /* Check for logoff */
    return 0;
  }
}

/**
 * dhcp_eapol_ind()
 * Call this function when a new EAPOL packet has arrived. This function
 * should be part of a select() loop in the application.
 **/
int dhcp_eapol_ind(struct dhcp_t *this) {
  struct dhcp_ctx ctx;
  uint8_t packet[1500];
  ssize_t length;

  ctx.parent = this;
  ctx.idx = 0;
  
  if ((length = net_read_eth(&this->rawif[0], packet, sizeof(packet))) < 0) 
    return -1;

#if(_debug_ > 1)
  if (_options.debug) {
    struct pkt_ethhdr_t *ethh = pkt_ethhdr(packet);
    log_dbg("eapol_decaps: src=%.2x:%.2x:%.2x:%.2x:%.2x:%.2x dst=%.2x:%.2x:%.2x:%.2x:%.2x:%.2x prot=%.4x",
	    ethh->src[0],ethh->src[1],ethh->src[2],
	    ethh->src[3],ethh->src[4],ethh->src[5],
	    ethh->dst[0],ethh->dst[1],ethh->dst[2],
	    ethh->dst[3],ethh->dst[4],ethh->dst[5],
	    ntohs(ethh->prot));
  }
#endif

  return dhcp_receive_eapol(&ctx, packet);
}

int dhcp_set_cb_eap_ind(struct dhcp_t *this, 
  int (*cb_eap_ind) (struct dhcp_conn_t *conn, uint8_t *pack, size_t len)) {
  this->cb_eap_ind = cb_eap_ind;
  return 0;
}
#endif


/**
 * dhcp_set_cb_data_ind()
 * Set callback function which is called when packet has arrived
 **/
int dhcp_set_cb_data_ind(struct dhcp_t *this, 
  int (*cb_data_ind) (struct dhcp_conn_t *conn, uint8_t *pack, size_t len)) {
  this->cb_data_ind = cb_data_ind;
  return 0;
}

/**
 * dhcp_set_cb_data_ind()
 * Set callback function which is called when a dhcp request is received
 **/
int dhcp_set_cb_request(struct dhcp_t *this, 
  int (*cb_request) (struct dhcp_conn_t *conn, struct in_addr *addr, uint8_t *pack, size_t len)) {
  this->cb_request = cb_request;
  return 0;
}


/**
 * dhcp_set_cb_connect()
 * Set callback function which is called when a connection is created
 **/
int dhcp_set_cb_connect(struct dhcp_t *this, 
             int (*cb_connect) (struct dhcp_conn_t *conn)) {
  this->cb_connect = cb_connect;
  return 0;
}

/**
 * dhcp_set_cb_disconnect()
 * Set callback function which is called when a connection is deleted
 **/
int dhcp_set_cb_disconnect(struct dhcp_t *this, 
  int (*cb_disconnect) (struct dhcp_conn_t *conn, int term_cause)) {
  this->cb_disconnect = cb_disconnect;
  return 0;
}

#if defined (__FreeBSD__) || defined (__APPLE__) || defined (__OpenBSD__)

int dhcp_receive(struct dhcp_t *this, int idx) {
  struct bpf_hdr *hdrp;
  struct pkt_buffer pb2;
  ssize_t length;
  ssize_t newoffset;
  struct dhcp_ctx ctx;

  this->pb.offset = 0;
  this->pb.length = 0;

  ctx.parent = this;
  ctx.idx = idx;

  length = net_read_eth(&this->rawif[idx], 
			pkt_buffer_head(&this->pb), 
			pkt_buffer_size(&this->pb));

  if (length <= 0) return length;
  this->pb.length = length;

  log_dbg("dhcp_received: %d", length);

  while (this->pb.offset < this->pb.length) {
    
    log_dbg("dhcp_packet offset: %d", this->pb.offset);

    if (this->pb.length - this->pb.offset < sizeof(struct bpf_hdr)) {
      log_err(0, "bad packet read");
      break;
    }
    
    hdrp = (struct bpf_hdr *)pkt_buffer_head(&this->pb);

    log_dbg("bpf_hdr  hdrlen %d caplen %d datalen %d", 
	hdrp->bh_hdrlen, hdrp->bh_caplen, hdrp->bh_datalen);
    
    if (this->pb.offset + hdrp->bh_hdrlen + 
	hdrp->bh_caplen > this->pb.length) {
      log_err(0, "bad packet read");
      break;
    }

    if (hdrp->bh_caplen != hdrp->bh_datalen) {
      log_warn(0, "BPF caplen(%d) != datalen(%d)", 
	       hdrp->bh_caplen, hdrp->bh_datalen);
      this->pb.offset += hdrp->bh_hdrlen + hdrp->bh_caplen;
      continue;
    }

    newoffset = this->pb.offset + hdrp->bh_hdrlen + hdrp->bh_caplen;

    pkt_buffer_init2(&pb2, this->pb.buf, this->pb.buflen, 
		     this->pb.offset + hdrp->bh_hdrlen,
		     hdrp->bh_caplen);

    dhcp_decaps_cb(&ctx, &pb2);

    this->pb.offset = BPF_WORDALIGN(newoffset);
  }

  return 0;
}
#endif

/* -*- c-file-style: "linux" -*- */
