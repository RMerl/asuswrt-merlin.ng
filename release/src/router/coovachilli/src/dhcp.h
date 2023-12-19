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

#ifndef _DHCP_H
#define _DHCP_H

#include "pkt.h"
#include "garden.h"
#include "net.h"
#include "bstrlib.h"

/* Option constants */
#define DHCP_OPTION_MAGIC_LEN       4

#define DHCP_OPTION_PAD             0
#define DHCP_OPTION_SUBNET_MASK     1
#define DHCP_OPTION_ROUTER_OPTION   3
#define DHCP_OPTION_DNS             6
#define DHCP_OPTION_HOSTNAME       12
#define DHCP_OPTION_DOMAIN_NAME    15
#define DHCP_OPTION_INTERFACE_MTU  26
#define DHCP_OPTION_STATIC_ROUTES  33
#define DHCP_OPTION_REQUESTED_IP   50
#define DHCP_OPTION_LEASE_TIME     51
#define DHCP_OPTION_MESSAGE_TYPE   53
#define DHCP_OPTION_SERVER_ID      54
#define DHCP_OPTION_PARAMETER_REQUEST_LIST 55
#define DHCP_OPTION_VENDOR_CLASS_IDENTIFIER 60
#define DHCP_OPTION_CLIENT_IDENTIFIER 61
#define DHCP_OPTION_CLIENT_FQDN    81
#define DHCP_OPTION_82    82

/* !!highly experimental!! */
#define DHCP_OPTION_CALLED_STATION_ID  197
#define DHCP_OPTION_CAPTIVE_PORTAL_ACL 198
#define DHCP_OPTION_CAPTIVE_PORTAL_URL 199

#define DHCP_OPTION_END           255

/* BOOTP Message Types */
#define DHCP_BOOTREQUEST  1
#define DHCP_BOOTREPLY    2

/* DHCP Message Types */
#define DHCPDISCOVER      1
#define DHCPOFFER         2
#define DHCPREQUEST       3
#define DHCPDECLINE       4
#define DHCPACK           5
#define DHCPNAK           6
#define DHCPRELEASE       7
#define DHCPINFORM        8
#define DHCPFORCERENEW    9

/* UDP Ports */
#define DHCP_BOOTPS 67
#define DHCP_BOOTPC 68
#define DHCP_DNS    53
#define DHCP_MDNS   5353

/* TCP Ports */
#define DHCP_HTTP   80
#define DHCP_HTTP_CUSTOM   8083
#define DHCP_HTTPS 443


#define DHCP_ARP_REQUEST 1
#define DHCP_ARP_REPLY   2

#define DHCP_DNS_HLEN  12

struct dhcp_t; /* Forward declaration */

/* Authentication states */
#define DHCP_AUTH_NONE        0
#define DHCP_AUTH_DROP        1
#define DHCP_AUTH_PASS        2
#define DHCP_AUTH_UNAUTH_TOS  3
#define DHCP_AUTH_AUTH_TOS    4
#define DHCP_AUTH_DNAT        5
#define DHCP_AUTH_SPLASH      6
#ifdef ENABLE_LAYER3
#define DHCP_AUTH_ROUTER      7
#endif

#define DHCP_DOMAIN_LEN      30

#define DHCP_DNAT_MAX       128

struct dhcp_nat_t {
  uint8_t mac[PKT_ETH_ALEN];
  uint32_t dst_ip;
  uint16_t dst_port;
  uint32_t src_ip;
  uint16_t src_port;
};

struct dhcp_conn_t {
  struct dhcp_conn_t *nexthash; /* Linked list part of hash table */
  struct dhcp_conn_t *next;     /* Next in linked list. 0: Last */
  struct dhcp_conn_t *prev;     /* Previous in linked list. 0: First */
  struct dhcp_t *parent;        /* Parent of all connections */
  void *peer;                   /* Peer protocol handler */

#ifdef ENABLE_CLUSTER
  uint8_t peerid;
#endif

  uint8_t inuse:1;             /* Free = 0; Inuse = 1 */
  uint8_t noc2c:1;             /* Prevent client to client access using /32 subnets */
  uint8_t is_reserved:1;       /* If this is a static/reserved mapping */
  uint8_t padding:5;

  time_t lasttime;             /* Last time we heard anything from client */
  uint8_t hismac[PKT_ETH_ALEN];/* Peer's MAC address */
  struct in_addr ourip;        /* IP address to listen to */
  struct in_addr hisip;        /* Client IP address */
  struct in_addr hismask;      /* Client Network Mask */
  struct in_addr dns1;         /* Client DNS address */
  struct in_addr dns2;         /* Client DNS address */
  char domain[DHCP_DOMAIN_LEN];/* Domain name to use for DNS lookups */
  int authstate;               /* 0: Unauthenticated, 1: Authenticated */
  uint8_t unauth_cp;           /* Unauthenticated codepoint */
  uint8_t auth_cp;             /* Authenticated codepoint */
  int nextdnat;                /* Next location to use for DNAT */
  uint32_t dnatdns;            /* Destination NAT for dns mapping */
  struct dhcp_nat_t dnat[DHCP_DNAT_MAX]; /* Destination NAT */
  uint16_t mtu;                /* Maximum transfer unit */

  struct in_addr migrateip;    /* Client IP address to migrate to */
  /*time_t last_nak;*/

#ifdef ENABLE_IEEE8021Q
  uint16_t tag8021q;
#endif

#ifdef ENABLE_MULTILAN
#define dhcp_conn_idx(x)       ((x)->lanidx)
#define dhcp_conn_set_idx(x,c) ((x)->lanidx = (c)->idx)
  int lanidx;
#else
#define dhcp_conn_idx(x) 0
#define dhcp_conn_set_idx(x,c) 
#endif

#ifdef ENABLE_IPV6
#endif

#ifdef ENABLE_DHCPRADIUS
  /* XXX: optional */
  struct {
    uint8_t sname[DHCP_SNAME_LEN];     /* 64 Optional server host name, null terminated string.*/
    uint8_t file[DHCP_FILE_LEN];       /* 128 Boot file name, null terminated string; "generic" name */
    uint8_t options[DHCP_OPTIONS_LEN]; /* var Optional parameters field. */
    size_t option_length;
  } dhcp_opts;
#endif
};


/* ***********************************************************
 * Information storage for each dhcp instance
 *
 * Normally each instance of the application corresponds to
 * one instance of a dhcp instance. 
 * 
 *************************************************************/

struct dhcp_t {

  /* network interfaces */
  struct _net_interface rawif[MAX_RAWIF];

#ifdef HAVE_NETFILTER_QUEUE
  struct _net_interface qif_in; 
  struct _net_interface qif_out; 
#endif

  int numconn;          /* Maximum number of connections */

#if defined (__FreeBSD__) || defined (__APPLE__) || defined (__OpenBSD__)
  struct pkt_buffer pb;
#endif

  int debug;            /* Set to print debug messages */

  struct in_addr ourip; /* IP address to listen to */

  int mtu;              /* Maximum transfer unit */

  uint32_t lease;       /* Seconds before reneval */

  int usemac;           /* Use given mac address */

  int promisc;          /* Set interface in promisc mode */

  int allowdyn;         /* Allow allocation of IP address on DHCP request */

  struct in_addr uamlisten; /* IP address to redirect HTTP requests to */
  uint16_t uamport;     /* TCP port to redirect HTTP requests to */

  //struct in_addr *authip; /* IP address of authentication server */
  //int authiplen;        /* Number of authentication server IP addresses */

  int anydns;           /* Allow any dns server */

  int noc2c;            /* Prevent client to client access using /32 subnets */

  int relayfd;          /* DHCP relay socket, 0 if not relaying */

  /* Connection management */
  struct dhcp_conn_t *firstfreeconn; /* First free in linked list */
  struct dhcp_conn_t *lastfreeconn;  /* Last free in linked list */
  struct dhcp_conn_t *firstusedconn; /* First used in linked list */
  struct dhcp_conn_t *lastusedconn;  /* Last used in linked list */

  /* Hash related parameters */
  int hashsize;                 /* Size of hash table */
  int hashlog;                  /* Log2 size of hash table */
  int hashmask;                 /* Bitmask for calculating hash */
  struct dhcp_conn_t **hash;    /* Hashsize array of pointer to member */

#ifdef HAVE_PATRICIA
  patricia_tree_t *ptree;
  patricia_tree_t *ptree_dyn;
#endif
  pass_through pass_throughs[MAX_PASS_THROUGHS];
  uint32_t num_pass_throughs;

  /* Call back functions */
  int (*cb_data_ind) (struct dhcp_conn_t *conn, uint8_t *pack, size_t len);
  int (*cb_eap_ind)  (struct dhcp_conn_t *conn, uint8_t *pack, size_t len);
  int (*cb_request) (struct dhcp_conn_t *conn, struct in_addr *addr, uint8_t *pack, size_t len);
  int (*cb_connect) (struct dhcp_conn_t *conn);
  int (*cb_disconnect) (struct dhcp_conn_t *conn, int term_cause);
};


const char* dhcp_version();

int dhcp_new(struct dhcp_t **dhcp, int numconn, int hashsize,
	     char *interface, int usemac, uint8_t *mac, int promisc, 
	     struct in_addr *listen, int lease, int allowdyn,
	     struct in_addr *uamlisten, uint16_t uamport, 
	     int noc2c);

int dhcp_set(struct dhcp_t *dhcp, char *ethers, int debug);

void dhcp_free(struct dhcp_t *dhcp);

int dhcp_timeout(struct dhcp_t *this);

int dhcp_send(struct dhcp_t *this, int idx,
	      unsigned char *hismac, uint8_t *packet, size_t length);
int dhcp_net_send(struct _net_interface *netif, unsigned char *hismac, 
		  uint8_t *packet, size_t length);

struct timeval * dhcp_timeleft(struct dhcp_t *this, struct timeval *tvp);

int dhcp_validate(struct dhcp_t *this);

int dhcp_set_addrs(struct dhcp_conn_t *conn, 
		   struct in_addr *hisip, struct in_addr *hismask,
		   struct in_addr *ourip, struct in_addr *ourmask,
		   struct in_addr *dns1, struct in_addr *dns2);

/* Called whenever a packet arrives */
int dhcp_decaps(struct dhcp_t *this, int idx);
int dhcp_relay_decaps(struct dhcp_t *this, int idx);
int dhcp_data_req(struct dhcp_conn_t *conn, struct pkt_buffer *pb, int ethhdr);
uint8_t * dhcp_nexthop(struct dhcp_t *);

#if defined (__FreeBSD__) || defined (__APPLE__) || defined (__OpenBSD__)
int dhcp_receive(struct dhcp_t *this, int idx);
#endif

int dhcp_set_cb_data_ind(struct dhcp_t *this, 
  int (*cb_data_ind) (struct dhcp_conn_t *conn, uint8_t *pack, size_t len));

int dhcp_set_cb_request(struct dhcp_t *this, 
  int (*cb_request) (struct dhcp_conn_t *conn, 
		     struct in_addr *addr, uint8_t *pack, size_t len));

int dhcp_set_cb_disconnect(struct dhcp_t *this, 
  int (*cb_disconnect) (struct dhcp_conn_t *conn, int term_cause));

int dhcp_set_cb_connect(struct dhcp_t *this, 
  int (*cb_connect) (struct dhcp_conn_t *conn));

int dhcp_set_cb_eap_ind(struct dhcp_t *this, 
  int (*cb_eap_ind) (struct dhcp_conn_t *conn, uint8_t *pack, size_t len));

int dhcp_hashget(struct dhcp_t *this, struct dhcp_conn_t **conn, uint8_t *hwaddr);

int dhcp_lnkconn(struct dhcp_t *this, struct dhcp_conn_t **conn);
int dhcp_newconn(struct dhcp_t *this, struct dhcp_conn_t **conn, uint8_t *hwaddr);

int dhcp_freeconn(struct dhcp_conn_t *conn, int term_cause);

int dhcp_arp_ind(struct dhcp_t *this);  /* ARP Indication */

int dhcp_sendEAP(struct dhcp_conn_t *conn, uint8_t *pack, size_t len);

int dhcp_sendEAPreject(struct dhcp_conn_t *conn, uint8_t *pack, size_t len);

int dhcp_eapol_ind(struct dhcp_t *this);

void dhcp_release_mac(struct dhcp_t *this, uint8_t *hwaddr, int term_cause);
void dhcp_block_mac(struct dhcp_t *this, uint8_t *hwaddr);

void dhcp_authorize_mac(struct dhcp_t *this, uint8_t *hwaddr,
			struct session_params *params);

#if (0) /* ENABLE_TCPRESET */
void dhcp_reset_tcp_mac(struct dhcp_t *this, uint8_t *hwaddr);
#endif

#define LIST_SHORT_FMT 0
#define LIST_LONG_FMT  1
#define LIST_JSON_FMT  2

int dhcp_filterDNS(struct dhcp_conn_t *conn, uint8_t *pack, size_t *plen);

int dhcp_gettag(struct dhcp_packet_t *pack, size_t length,
		struct dhcp_tag_t **tag, uint8_t tagtype);

int dhcp_hashadd(struct dhcp_t *this, struct dhcp_conn_t *conn);

#ifdef ENABLE_CLUSTER
void dhcp_peer_update(char force);
void print_peers(bstring s);
struct chilli_peer;
struct chilli_peer * get_chilli_peer(int id);
#endif

struct app_conn_t * dhcp_get_appconn_ip
(struct dhcp_conn_t *conn, struct in_addr *dst);

struct app_conn_t * dhcp_get_appconn_pkt
(struct dhcp_conn_t *conn, struct pkt_iphdr_t *iph, char is_dst);

int dhcp_garden_check(struct dhcp_t *this,
		      struct dhcp_conn_t *conn,
		      struct app_conn_t *appconn,
		      struct pkt_ipphdr_t *ipph, int dst);

#define CHILLI_DHCP_OFFER    1
#define CHILLI_DHCP_ACK      2
#define CHILLI_DHCP_NAK      3
#define CHILLI_DHCP_RELAY    4
#define CHILLI_DHCP_PROXY    5

#endif	/* !_DHCP_H */
