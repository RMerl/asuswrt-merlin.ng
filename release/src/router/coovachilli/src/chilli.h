/* -*- mode: c; c-basic-offset: 2 -*- */
/* 
 * Copyright (C) 2007-2012 David Bird (Coova Technologies) <support@coova.com>
 * Copyright (C) 2003, 2004, 2005 Mondru AB.
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

#ifndef _CHILLI_H
#define _CHILLI_H

#include "system.h"
#include "chilli_limits.h"
#include "tun.h"
#include "ippool.h"
#include "radius.h"
#include "redir.h"
#include "syserr.h"
#include "session.h"
#include "dhcp.h"
#include "options.h"
#include "cmdsock.h"
#include "net.h"
#include "md5.h"
#include "dns.h"

/*#define XXX_IO_DAEMON 1*/

/* Authtype defs */
#define CHAP_DIGEST_MD5   0x05
#define CHAP_MICROSOFT    0x80
#define CHAP_MICROSOFT_V2 0x81
#define PAP_PASSWORD       256
#define EAP_MESSAGE        257

#define MPPE_KEYSIZE  16
#define NT_KEYSIZE    16

#define DNPROT_NULL       1
#define DNPROT_DHCP_NONE  2
#define DNPROT_UAM        3
#define DNPROT_WPA        4
#ifdef ENABLE_EAPOL
#define DNPROT_EAPOL      5
#endif
#define DNPROT_MAC        6
#ifdef ENABLE_LAYER3
#define DNPROT_LAYER3     7
#endif

/* Debug facility */
#define DEBUG_DHCP        2
#define DEBUG_RADIUS      4
#define DEBUG_REDIR       8
#define DEBUG_CONF       16

/* Struct information for each connection */
struct app_conn_t {
  
  struct app_conn_t *next;    /* Next in linked list. 0: Last */
  struct app_conn_t *prev;    /* Previous in linked list. 0: First */

  /* Pointers to protocol handlers */
  void *uplink;                  /* Uplink network interface (Internet) */
  void *dnlink;                  /* Downlink network interface (Wireless) */

  uint8_t inuse:1;
  uint8_t is_adminsession:1;
  uint8_t uamabort:1;
  uint8_t uamexit:1;

  /* Management of connections */
  int unit;
  int dnprot;                    /* Downlink protocol */
  time_t rt;

#if(0)
#define s_params  params[0]
#define ss_params params[1]
#define s_state   state[0]
#define ss_state  state[1]
  struct session_params params[2];        /* Session parameters */
  struct session_state  state[2];         /* Session state */
  char has_subsession;
#endif

  struct session_params s_params;         /* Session parameters */
  struct session_state  s_state;          /* Session state */

#ifdef HAVE_PATRICIA
  patricia_tree_t *ptree;
#endif

  /* Radius authentication stuff */
  /* Parameters are initialised whenever a reply to an access request
     is received. */
#ifdef ENABLE_RADPROXY
  uint8_t chal[MAX_EAP_LEN];     /* EAP challenge */
  size_t challen;                /* Length of EAP challenge */
  uint8_t sendkey[RADIUS_ATTR_VLEN];
  uint8_t recvkey[RADIUS_ATTR_VLEN];
  uint8_t lmntkeys[RADIUS_MPPEKEYSSIZE];
  size_t sendlen;
  size_t recvlen;
  size_t lmntlen;
  uint32_t policy;
  uint32_t types;
  uint8_t ms2succ[MS2SUCCSIZE];
  size_t ms2succlen;
#endif

  /* Radius proxy stuff */
  /* Parameters are initialised whenever a radius proxy request is received */
  /* Only one outstanding request allowed at a time */
  int radiuswait;                /* Radius request in progres */
  struct sockaddr_in radiuspeer; /* Where to send reply */
  uint8_t radiusid;              /* ID to reply with */
  uint8_t authenticator[RADIUS_AUTHLEN];
  int authtype; /* TODO */

  /* Parameters for radius accounting */
  /* These parameters are set when an access accept is sent back to the
     NAS */

  uint32_t nasip;              /* Set by access request */
  uint32_t nasport;            /* Set by access request */
  uint8_t hismac[PKT_ETH_ALEN];/* His MAC address */
  struct in_addr ourip;        /* IP address to listen to */
  struct in_addr hisip;        /* Client IP address */
  struct in_addr hismask;      /* Client IP address mask */
  struct in_addr reqip;        /* IP requested by client */
#ifdef ENABLE_UAMANYIP
  struct in_addr natip;
#endif
  uint16_t mtu;

  /* Information for each connection */
  struct in_addr net;
  struct in_addr mask;
  struct in_addr dns1;
  struct in_addr dns2;

#if defined(ENABLE_LOCATION) && defined(HAVE_AVL)
  struct list_entity loc_sess;
  struct loc_search_t *loc_search_node;
#endif
};

#define VAL_STRING   0
#define VAL_IN_ADDR  1
#define VAL_MAC_ADDR 2
#define VAL_ULONG    3
#define VAL_ULONG64  4
#define VAL_USHORT   5

typedef enum {
  ACCT_USER,
#ifdef ENABLE_GARDENACCOUNTING
  ACCT_GARDEN,
#endif
} acct_type;

void set_env(char *name, char type, void *value, int len);

extern struct app_conn_t *firstfreeconn; /* First free in linked list */
extern struct app_conn_t *lastfreeconn;  /* Last free in linked list */
extern struct app_conn_t *firstusedconn; /* First used in linked list */
extern struct app_conn_t *lastusedconn;  /* Last used in linked list */

extern struct radius_t *radius;          /* Radius client instance */
extern struct dhcp_t *dhcp;              /* DHCP instance */
extern struct tun_t *tun;                /* TUN/TAP instance */

#ifdef ENABLE_CLUSTER
struct chilli_peer {
  struct in_addr addr;
  uint8_t mac[6];
  uint8_t state;
  time_t last_update;
};
#define PEER_STATE_OFFLINE 0
#define PEER_STATE_ACTIVE  1
#define PEER_STATE_STANDBY 2
#define PEER_STATE_ADMCMD  3
#define PEER_STATE_MODULE  4
#endif

#ifdef ENABLE_STATFILE
int printstatus();
int loadstatus();
#endif

int chilli_connect(struct app_conn_t **appconn, struct dhcp_conn_t *conn);

#ifdef ENABLE_LAYER3
struct app_conn_t * chilli_connect_layer3(struct in_addr *src, struct dhcp_conn_t *conn);
#endif

int chilli_getconn(struct app_conn_t **conn, uint32_t ip, 
		   uint32_t nasip, uint32_t nasport);

int chilli_appconn_run(int (*cb)(struct app_conn_t *, void *), void *d);

int chilli_req_attrs(struct radius_t *radius, 
		     struct radius_packet_t *pack,
		     acct_type type,
		     uint32_t service_type,
		     uint8_t status_type,
		     uint32_t port,
		     uint8_t *hismac,
		     struct in_addr *hisip,
		     struct session_state *state);

int chilli_auth_radius(struct radius_t *radius);

int chilli_signal(int signo, void (*func)(int));
void chilli_signals(int *with_term, int *with_hup);

int chilli_binconfig(char *file, size_t flen, pid_t pid);

int chilli_new_conn(struct app_conn_t **conn);

int chilli_assign_snat(struct app_conn_t *appconn, int force);

void chilli_print(bstring s, int listfmt, 
		  struct app_conn_t *appconn,
		  struct dhcp_conn_t *conn);

int chilli_acct_fromsub(struct app_conn_t *appconn, 
			struct pkt_ipphdr_t *ipph);
int chilli_acct_tosub(struct app_conn_t *appconn, 
		      struct pkt_ipphdr_t *ipph);

int terminate_appconn(struct app_conn_t *appconn, int terminate_cause);

void config_radius_session(struct session_params *params, 
			   struct radius_packet_t *pack, 
			   struct app_conn_t *appconn,
			   int reconfig);

void session_param_defaults(struct session_params *params);

int dnprot_accept(struct app_conn_t *appconn);

int dnprot_reject(struct app_conn_t *appconn);

int get_urlparts(char *src, char *host, int hostsize, int *port, int *uripos);

int cmdsock_init();

int cmdsock_port_init();

void cmdsock_shutdown();

time_t mainclock_tick();
time_t mainclock_now();
time_t mainclock_rt();
time_t mainclock_wall();
time_t mainclock_towall(time_t t);
int mainclock_diff(time_t past);
uint32_t mainclock_diffu(time_t past);

pid_t chilli_fork(uint8_t type, char *name);

void child_killall(int sig);

#define CHILLI_PROC        0
#define CHILLI_PROC_DAEMON 1
#define CHILLI_PROC_REDIR  2
#define CHILLI_PROC_SCRIPT 3

#ifdef ENABLE_PROXYVSA
int radius_addvsa(struct radius_packet_t *pack, struct redir_state *state);
int chilli_learn_location(uint8_t *loc, int loclen, 
			  struct app_conn_t *appconn, char force);
#endif

#ifdef HAVE_NETFILTER_COOVA
int kmod_coova_update(struct app_conn_t *appconn);
int kmod_coova_release(struct dhcp_conn_t *conn);
int kmod_coova_sync();
int kmod_coova_clear();
#endif

#ifdef HAVE_OPENSSL
void NtPasswordHash(u_char *Password, int len, u_char *hash);
void HashNtPasswordHash(u_char *hash, u_char *hashhash);
void ChallengeHash(u_char *PeerChallenge, u_char *AuthenticatorChallenge,
		   u_char *UserName, int UserNameLen, u_char *Challenge);
void GenerateNTResponse(u_char *AuthenticatorChallenge, 
			u_char *PeerChallenge,
			u_char *UserName, int UserNameLen, 
			u_char *Password, int PasswordLen, 
			u_char *Response);
void GenerateAuthenticatorResponse(u_char *Password, int PasswordLen,
				   u_char *NTResponse, u_char *PeerChallenge,
				   u_char *AuthenticatorChallenge, u_char *UserName,
				   int UserNameLen, u_char *AuthenticatorResponse);
#endif

#ifdef ENABLE_MULTIROUTE
int chilli_getconn_byroute(struct app_conn_t **conn, int idx);
#endif

uint8_t* chilli_called_station(struct session_state *state);

int chilli_cmd(struct cmdsock_request *req, bstring s, int sock);

int chilli_handle_signal(void *ctx, int fd);
void chilli_freeconn();

int runscript(struct app_conn_t *appconn, char* script,
	      char *loc, char *oloc);

/* utils.c */
int statedir_file(char *dst, int dlen, char *file, char *deffile);
int bblk_fromfd(bstring s, int fd, int len);
int bstring_fromfd(bstring s, int fd);
#ifndef HAVE_GETLINE
ssize_t getline (char** lineptr, size_t* n, FILE* stream);
#endif

/* sig.c */
int ndelay_on (int fd);
int ndelay_off (int fd);
int coe (int fd);

int set_signal (int signo, void (*func)(int));

int selfpipe_init (void);
int selfpipe_read (void);
int selfpipe_trap (int signo);
int selfpipe_ignore (int signo);
void selfpipe_finish();

#ifdef ENABLE_LOCATION
void location_init();
#ifdef HAVE_AVL

struct loc_search_t {
  struct avl_node node;

  char value[MAX_LOCATION_LENGTH];

  uint64_t total_sess_count,
    closed_sess_count,
    new_sess_count,
    roamed_in_sess_count,
    roamed_out_sess_count;

  time_t last_queried; 

  struct list_entity loc_sess_head;

  uint64_t closed_bytes_up,
    closed_bytes_down; 

#ifdef ENABLE_GARDENACCOUNTING
  uint64_t garden_closed_bytes_up,
    garden_closed_bytes_down; 

  uint64_t other_closed_bytes_up,
    other_closed_bytes_down; 
#endif
};

void location_close_conn(struct app_conn_t *conn, int close);
struct loc_search_t *location_find(char *loc);
void location_add_conn(struct app_conn_t *appconn, char *loc);
void location_printlist(bstring s, char *loc, int json, int list);

#endif
#endif

int kick_wifi_client(char *macaddr);  //John added for kick user

#endif /*_CHILLI_H */
