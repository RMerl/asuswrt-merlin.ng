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

#ifndef _RADIUS_H
#define _RADIUS_H

#define RADIUS_NONETWORK   0x01
#define RADIUS_NOBROADCAST 0x02

#define RADIUS_AUTHPORT 1812
#define RADIUS_ACCTPORT 1813

/* Radius packet types */
#define RADIUS_CODE_ACCESS_REQUEST            1
#define RADIUS_CODE_ACCESS_ACCEPT             2
#define RADIUS_CODE_ACCESS_REJECT             3
#define RADIUS_CODE_ACCOUNTING_REQUEST        4
#define RADIUS_CODE_ACCOUNTING_RESPONSE       5
#define RADIUS_CODE_ACCESS_CHALLENGE         11
#define RADIUS_CODE_STATUS_SERVER            12
#define RADIUS_CODE_STATUS_CLIENT            13
#define RADIUS_CODE_DISCONNECT_REQUEST       40
#define RADIUS_CODE_DISCONNECT_ACK           41
#define RADIUS_CODE_DISCONNECT_NAK           42
#define RADIUS_CODE_COA_REQUEST              43
#define RADIUS_CODE_COA_ACK                  44
#define RADIUS_CODE_COA_NAK                  45
#define RADIUS_CODE_STATUS_REQUEST           46
#define RADIUS_CODE_STATUS_ACCEPT            47
#define RADIUS_CODE_STATUS_REJECT            48

/* Radius attributes */
#define RADIUS_ATTR_USER_NAME                 1     /* string */
#define RADIUS_ATTR_USER_PASSWORD             2     /* string (encrypt) */
#define RADIUS_ATTR_CHAP_PASSWORD             3     /* octets */
#define RADIUS_ATTR_NAS_IP_ADDRESS            4     /* ipaddr */
#define RADIUS_ATTR_NAS_PORT                  5     /* integer */
#define RADIUS_ATTR_SERVICE_TYPE              6     /* integer */
#define RADIUS_ATTR_FRAMED_PROTOCOL           7     /* integer */
#define RADIUS_ATTR_FRAMED_IP_ADDRESS         8     /* ipaddr */
#define RADIUS_ATTR_FRAMED_IP_NETMASK         9     /* ipaddr */
#define RADIUS_ATTR_FRAMED_ROUTING           10     /* integer */
#define RADIUS_ATTR_FILTER_ID                11     /* string */
#define RADIUS_ATTR_FRAMED_MTU               12     /* integer */
#define RADIUS_ATTR_FRAMED_COMPRESSION       13     /* integer */
#define RADIUS_ATTR_LOGIN_IP_HOST            14     /* ipaddr */
#define RADIUS_ATTR_LOGIN_SERVICE            15     /* integer */
#define RADIUS_ATTR_LOGIN_TCP_PORT           16     /* integer */
#define RADIUS_ATTR_REPLY_MESSAGE            18     /* string */
#define RADIUS_ATTR_CALLBACK_NUMBER          19     /* string */
#define RADIUS_ATTR_CALLBACK_ID              20     /* string */
#define RADIUS_ATTR_FRAMED_ROUTE             22     /* string */
#define RADIUS_ATTR_FRAMED_IPX_NETWORK       23     /* ipaddr */
#define RADIUS_ATTR_STATE                    24     /* octets */
#define RADIUS_ATTR_CLASS                    25     /* octets */
#define RADIUS_ATTR_VENDOR_SPECIFIC          26     /* octets */
#define RADIUS_ATTR_SESSION_TIMEOUT          27     /* integer */
#define RADIUS_ATTR_IDLE_TIMEOUT             28     /* integer */
#define RADIUS_ATTR_TERMINATION_ACTION       29     /* integer */
#define RADIUS_ATTR_CALLED_STATION_ID        30     /* string */
#define RADIUS_ATTR_CALLING_STATION_ID       31     /* string */
#define RADIUS_ATTR_NAS_IDENTIFIER           32     /* string */
#define RADIUS_ATTR_PROXY_STATE              33     /* octets */
#define RADIUS_ATTR_LOGIN_LAT_SERVICE        34     /* string */
#define RADIUS_ATTR_LOGIN_LAT_NODE           35     /* string */
#define RADIUS_ATTR_LOGIN_LAT_GROUP          36     /* octets */
#define RADIUS_ATTR_FRAMED_APPLETALK_LINK    37     /* integer */
#define RADIUS_ATTR_FRAMED_APPLETALK_NETWORK 38     /* integer */
#define RADIUS_ATTR_FRAMED_APPLETALK_ZONE    39     /* string */
#define RADIUS_ATTR_ACCT_STATUS_TYPE         40     /* integer */
#define RADIUS_ATTR_ACCT_DELAY_TIME          41     /* integer */
#define RADIUS_ATTR_ACCT_INPUT_OCTETS        42     /* integer */
#define RADIUS_ATTR_ACCT_OUTPUT_OCTETS       43     /* integer */
#define RADIUS_ATTR_ACCT_SESSION_ID          44     /* string */
#define RADIUS_ATTR_ACCT_AUTHENTIC           45     /* integer */
#define RADIUS_ATTR_ACCT_SESSION_TIME        46     /* integer */
#define RADIUS_ATTR_ACCT_INPUT_PACKETS       47     /* integer */
#define RADIUS_ATTR_ACCT_OUTPUT_PACKETS      48     /* integer */
#define RADIUS_ATTR_ACCT_TERMINATE_CAUSE     49     /* integer */
#define RADIUS_ATTR_ACCT_MULTI_SESSION_ID    50     /* string */
#define RADIUS_ATTR_ACCT_LINK_COUNT          51     /* integer */
#define RADIUS_ATTR_ACCT_INPUT_GIGAWORDS     52     /* integer */
#define RADIUS_ATTR_ACCT_OUTPUT_GIGAWORDS    53     /* integer */
#define RADIUS_ATTR_EVENT_TIMESTAMP          55     /* date */
#define RADIUS_ATTR_CHAP_CHALLENGE           60     /* string */
#define RADIUS_ATTR_NAS_PORT_TYPE            61     /* integer */
#define RADIUS_ATTR_PORT_LIMIT               62     /* integer */
#define RADIUS_ATTR_LOGIN_LAT_PORT           63     /* integer */
#define RADIUS_ATTR_ACCT_TUNNEL_CONNECTION   68     /* string */
#define RADIUS_ATTR_ARAP_PASSWORD            70     /* string */
#define RADIUS_ATTR_ARAP_FEATURES            71     /* string */
#define RADIUS_ATTR_ARAP_ZONE_ACCESS         72     /* integer */
#define RADIUS_ATTR_ARAP_SECURITY            73     /* integer */
#define RADIUS_ATTR_ARAP_SECURITY_DATA       74     /* string */
#define RADIUS_ATTR_PASSWORD_RETRY           75     /* integer */
#define RADIUS_ATTR_PROMPT                   76     /* integer */
#define RADIUS_ATTR_CONNECT_INFO             77     /* string */
#define RADIUS_ATTR_CONFIGURATION_TOKEN      78     /* string */
#define RADIUS_ATTR_EAP_MESSAGE              79     /* string */
#define RADIUS_ATTR_MESSAGE_AUTHENTICATOR    80     /* octets */
#define RADIUS_ATTR_ARAP_CHALLENGE_RESPONSE  84     /* string # 10 octets */
#define RADIUS_ATTR_ACCT_INTERIM_INTERVAL    85     /* integer */
#define RADIUS_ATTR_NAS_PORT_ID              87     /* string */
#define RADIUS_ATTR_FRAMED_POOL              88     /* string */
#define RADIUS_ATTR_CHARGEABLE_USER_IDENTITY 89     /* string */
#define RADIUS_ATTR_NAS_IPV6_ADDRESS         95     /* octets (IPv6) */
#define RADIUS_ATTR_FRAMED_INTERFACE_ID      96     /* octets # 8 octets */
#define RADIUS_ATTR_FRAMED_IPV6_PREFIX       97     /* octets ??? */
#define RADIUS_ATTR_LOGIN_IPV6_HOST          98     /* octets (IPv6) */
#define RADIUS_ATTR_FRAMED_IPV6_ROUTE        99     /* string */
#define RADIUS_ATTR_FRAMED_IPV6_POOL        100     /* string */
#define RADIUS_ATTR_DIGEST_RESPONSE         206     /* string */
#define RADIUS_ATTR_DIGEST_ATTRIBUTES       207     /* octets  ??? */


#define RADIUS_VENDOR_MS                    311
#define RADIUS_ATTR_MS_CHAP_RESPONSE          1
#define RADIUS_ATTR_MS_MPPE_ENCRYPTION_POLICY 7
#define RADIUS_ATTR_MS_MPPE_ENCRYPTION_TYPES  8
#define RADIUS_ATTR_MS_CHAP_CHALLENGE        11
#define RADIUS_ATTR_MS_CHAP_MPPE_KEYS        12
#define RADIUS_ATTR_MS_MPPE_SEND_KEY         16
#define RADIUS_ATTR_MS_MPPE_RECV_KEY         17
#define RADIUS_ATTR_MS_CHAP2_RESPONSE        25
#define RADIUS_ATTR_MS_CHAP2_SUCCESS         26


#define RADIUS_SERVICE_TYPE_LOGIN             1
#define RADIUS_SERVICE_TYPE_FRAMED            2
#define RADIUS_SERVICE_TYPE_ADMIN_USER        6

#define RADIUS_STATUS_TYPE_START              1
#define RADIUS_STATUS_TYPE_STOP               2
#define RADIUS_STATUS_TYPE_INTERIM_UPDATE     3
#define RADIUS_STATUS_TYPE_ACCOUNTING_ON      7
#define RADIUS_STATUS_TYPE_ACCOUNTING_OFF     8

#define RADIUS_NAS_PORT_TYPE_VIRTUAL          5
#define RADIUS_NAS_PORT_TYPE_WIRELESS_802_11 19
#define RADIUS_NAS_PORT_TYPE_WIRELESS_UMTS   23

#define RADIUS_TERMINATE_CAUSE_USER_REQUEST          1
#define RADIUS_TERMINATE_CAUSE_LOST_CARRIER          2
#define RADIUS_TERMINATE_CAUSE_LOST_SERVICE          3
#define RADIUS_TERMINATE_CAUSE_IDLE_TIMEOUT          4
#define RADIUS_TERMINATE_CAUSE_SESSION_TIMEOUT       5
#define RADIUS_TERMINATE_CAUSE_ADMIN_RESET           6
#define RADIUS_TERMINATE_CAUSE_ADMIN_REBOOT          7
#define RADIUS_TERMINATE_CAUSE_PORT_ERROR            8
#define RADIUS_TERMINATE_CAUSE_NAS_ERROR             9
#define RADIUS_TERMINATE_CAUSE_NAS_REQUEST          10
#define RADIUS_TERMINATE_CAUSE_NAS_REBOOT           11
#define RADIUS_TERMINATE_CAUSE_PORT_UNNEEDED        12
#define RADIUS_TERMINATE_CAUSE_PORT_PREEMPTED       13
#define RADIUS_TERMINATE_CAUSE_PORT_SUSPEND         14
#define RADIUS_TERMINATE_CAUSE_SERVICE_UNAVAILABLE  15
#define RADIUS_TERMINATE_CAUSE_CALLBACK             16
#define RADIUS_TERMINATE_CAUSE_USER_ERROR           17
#define RADIUS_TERMINATE_CAUSE_HOST_REQUEST         18

#include "radius_pkt.h"
#include "radius_wispr.h"
#include "radius_chillispot.h"

struct radius_queue_t {      /* Holder for queued packets */
  int state;                 /* 0=empty, 1=full */
  void *cbp;                 /* Pointer used for callbacks */
  struct timeval timeout;    /* When do we retransmit this packet? */
  int retrans;               /* How many times did we retransmit this? */
  int lastsent;              /* 0 or 1 indicates last server used */
  struct sockaddr_in peer;   /* Address packet was sent to / received from */
  struct radius_packet_t 
#ifdef RADIUS_QUEUE_PACKET_PTR
  *
#endif
                         p;  /* The packet stored */
  int next;                  /* Pointer to the next in queue. -1: Last */
  int prev;                  /* Pointer to the previous in queue. -1: First */
};

typedef struct radius_queue_t * radius_queue;
struct session_state;

struct radius_server_t {
  struct in_addr addr0;
  struct in_addr addr1;
  char secret[RADIUS_SECRETSIZE];
  size_t secretlen;
  uint16_t authport;
  uint16_t acctport;
};

struct radius_t {
  int fd;                        /* Socket file descriptor */

  FILE *urandom_fp;              /* /dev/urandom FILE pointer */

  struct in_addr ouraddr;        /* Address to listen to */
  uint16_t ourport;              /* Port to listen to */
  int coanocheck;                /* Accept coa from all IP addresses */


  int lastreply;                 /* 0 or 1 indicates last server reply */

  uint16_t authport;             /* His port for authentication */
  uint16_t acctport;             /* His port for accounting */

  struct in_addr hisaddr0;       /* Server address */
  struct in_addr hisaddr1;       /* Server address */
  char secret[RADIUS_SECRETSIZE];/* Shared secret */
  size_t secretlen;              /* Length of sharet secret */

  uint8_t nextid;                /* Next RADIUS id */
  radius_queue queue;            /* Outstanding replies */
  uint8_t qsize;                 /* Queue length */
  uint8_t qnext;                 /* Next location in queue to use */
  int first;                     /* First packet in queue (oldest timeout) */
  int last;                      /* Last packet in queue (youngest timeout) */


#ifdef ENABLE_RADPROXY
  int proxyfd;                   /* Proxy socket file descriptor */
  struct in_addr proxylisten;    /* Proxy address to listen to */
  uint16_t proxyport;            /* Proxy port to listen to */
  struct in_addr proxyaddr;      /* Proxy client address */
  struct in_addr proxymask;      /* Proxy client mask */
  char proxysecret[RADIUS_SECRETSIZE]; /* Proxy secret */
  size_t proxysecretlen;            /* Length of sharet secret */
#endif

  unsigned char nas_hwaddr[6];   /* Hardware address of NAS */
  int debug;                     /* Print debug messages */

  int (*cb_ind)       (struct radius_t *radius, struct radius_packet_t *pack,
		       struct sockaddr_in *peer);
  int (*cb_auth_conf) (struct radius_t *radius, struct radius_packet_t *pack,
		       struct radius_packet_t *pack_req, void *cbp);
  int (*cb_acct_conf) (struct radius_t *radius, struct radius_packet_t *pack,
		       struct radius_packet_t *pack_req, void *cbp);
  int (*cb_coa_ind)   (struct radius_t *radius, struct radius_packet_t *pack,
		       struct sockaddr_in *peer);
};


/* Create new radius instance */
int radius_new(struct radius_t **this, 
	       struct in_addr *listen, uint16_t port, 
	       int coanocheck, int proxy);

int radius_printqueue(int fd, struct radius_t *this);

int radius_init_q(struct radius_t *this, int size);

/* Delete existing radius instance */
int radius_free(struct radius_t *this);

/* Set radius parameters which can later be changed */
void radius_set(struct radius_t *this, 
		unsigned char *hwaddr,
		int debug);

/* Callback function for received request */
int radius_set_cb_ind(struct radius_t *this,
		      int (*cb_ind) (struct radius_t *radius,
				     struct radius_packet_t *pack,
				     struct sockaddr_in *peer));

/* Callback function for response to access request */
int radius_set_cb_auth_conf(struct radius_t *this,
			    int (*cb_auth_conf) (struct radius_t *radius, 
						 struct radius_packet_t *pack,
						 struct radius_packet_t *pack_req, void *cbp));

/* Callback function for response to accounting request */
int radius_set_cb_acct_conf(struct radius_t *this,
			    int (*cb_acct_conf) (struct radius_t *radius, 
						 struct radius_packet_t *pack,
						 struct radius_packet_t *pack_req, void *cbp));

int radius_set_cb_coa_ind(struct radius_t *this,
			  int (*cb_coa_ind) (struct radius_t *radius, 
					     struct radius_packet_t *pack,
					     struct sockaddr_in *peer));

/* Send of a request */
int radius_req(struct radius_t *this, 
	       struct radius_packet_t *pack,
	       void *cbp);

/* Send of a response */
int radius_resp(struct radius_t *this,
		struct radius_packet_t *pack,
		struct sockaddr_in *peer, uint8_t *req_auth);

/* Send of a coa response */
int radius_coaresp(struct radius_t *this,
		   struct radius_packet_t *pack,
		   struct sockaddr_in *peer, uint8_t *req_auth);

/* Process an incoming packet */
int radius_decaps(struct radius_t *this, int idx);

/* Process an incoming packet */
int radius_proxy_ind(struct radius_t *this, int idx);

/* Add an attribute to a packet */
int radius_addattr(struct radius_t *this, struct radius_packet_t *pack, 
		   uint8_t type, uint32_t vendor_id, uint8_t vendor_type,
		   uint32_t value, uint8_t *data, uint16_t dlen);

/* Generate a packet for use with radius_addattr() */
int radius_default_pack(struct radius_t *this,
			struct radius_packet_t *pack,
			int code);

/* Extract an attribute from a packet */
int radius_getnextattr(struct radius_packet_t *pack, 
		       struct radius_attr_t **attr,
		       uint8_t type, uint32_t vendor_id, uint8_t vendor_type,
		       int instance, size_t *roffset);

int radius_getattr(struct radius_packet_t *pack, struct radius_attr_t **attr,
		   uint8_t type, uint32_t vendor_id, uint8_t vendor_type,
		   int instance);

/* Encode a password */
int radius_pwencode(struct radius_t *this, uint8_t *dst, size_t dstsize,
		    size_t *dstlen, uint8_t *src, size_t srclen, 
		    uint8_t *authenticator, char *secret, size_t secretlen);


/* Decode a password (also used for MSCHAPv1 MPPE keys) */
int radius_pwdecode(struct radius_t *this, uint8_t *dst, size_t dstsize,
		    size_t *dstlen, uint8_t *src, size_t srclen, 
		    uint8_t *authenticator, char *secret, size_t secretlen);


/* Decode MPPE key */
int radius_keydecode(struct radius_t *this, uint8_t *dst, size_t dstsize,
		     size_t *dstlen, uint8_t *src, size_t srclen, 
		     uint8_t *authenticator, char *secret, size_t secretlen);

/* Encode MPPE key */
int radius_keyencode(struct radius_t *this, uint8_t *dst, size_t dstsize,
		     size_t *dstlen, uint8_t *src, size_t srclen,
		     uint8_t *authenticator, char *secret, size_t secretlen);

/* Call this function to process packets needing retransmission */
int radius_timeout(struct radius_t *this);

/* Figure out when to call radius_calltimeout() */
int radius_timeleft(struct radius_t *this, struct timeval *timeout);

void radius_addnasip(struct radius_t *radius, struct radius_packet_t *pack);

void radius_addcalledstation(struct radius_t *radius, 
			     struct radius_packet_t *pack,
			     struct session_state *state);

int radius_authresp_authenticator(struct radius_t *this,
				  struct radius_packet_t *pack,
				  uint8_t *req_auth,
				  char *secret, size_t secretlen);


int radius_hmac_md5(struct radius_t *this, struct radius_packet_t *pack, 
		    char *secret, int secretlen, uint8_t *dst);

#ifdef RADIUS_QUEUE_PACKET_PTR
#define RADIUS_QUEUE_PKT(p, field) (p)->field
#define RADIUS_QUEUE_PKTPTR(p) (p)
#define RADIUS_QUEUE_PKTFREE(p) if((p))free((p));(p)=NULL;log_dbg("Freeing RADIUS packet")
#define RADIUS_QUEUE_PKTALLOC(p) if((p))free((p));(p)=calloc(sizeof(struct radius_packet_t),1);log_dbg("Allocating RADIUS packet")
#define RADIUS_QUEUE_HASPKT(p) ((p) != NULL)
#else
#define RADIUS_QUEUE_PKT(p, field) (p).field
#define RADIUS_QUEUE_PKTPTR(p) &(p)
#define RADIUS_QUEUE_PKTFREE(p) 
#define RADIUS_QUEUE_PKTALLOC(p) 
#define RADIUS_QUEUE_HASPKT(p) (1)
#endif

#endif	/* !_RADIUS_H */
