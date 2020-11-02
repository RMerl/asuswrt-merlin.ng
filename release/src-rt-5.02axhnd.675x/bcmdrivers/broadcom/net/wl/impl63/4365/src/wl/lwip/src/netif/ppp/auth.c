/*****************************************************************************
* auth.c - Network Authentication and Phase Control program file.
*
* Copyright (c) 2003 by Marc Boucher, Services Informatiques (MBSI) inc.
* Copyright (c) 1997 by Global Election Systems Inc.  All rights reserved.
*
* The authors hereby grant permission to use, copy, modify, distribute,
* and license this software and its documentation for any purpose, provided
* that existing copyright notices are retained in all copies and that this
* notice and the following disclaimer are included verbatim in any
* distributions. No written agreement, license, or royalty fee is required
* for any of the authorized uses.
*
* THIS SOFTWARE IS PROVIDED BY THE CONTRIBUTORS *AS IS* AND ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
* OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
* IN NO EVENT SHALL THE CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
* NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
* THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
* THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
******************************************************************************
* REVISION HISTORY
*
* 03-01-01 Marc Boucher <marc@mbsi.ca>
*   Ported to lwIP.
* 97-12-08 Guy Lancaster <lancasterg@acm.org>, Global Election Systems Inc.
*   Ported from public pppd code.
*****************************************************************************/
/*
 * auth.c - PPP authentication and phase control.
 *
 * Copyright (c) 1993 The Australian National University.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the Australian National University.  The name of the University
 * may not be used to endorse or promote products derived from this
 * software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Copyright (c) 1989 Carnegie Mellon University.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by Carnegie Mellon University.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "lwip/opt.h"

#if PPP_SUPPORT /* don't build if not configured for use in lwipopts.h */

#include "ppp.h"
#include "pppdebug.h"

#include "fsm.h"
#include "lcp.h"
#include "pap.h"
#include "chap.h"
#include "auth.h"
#include "ipcp.h"

#if CBCP_SUPPORT
#include "cbcp.h"
#endif /* CBCP_SUPPORT */

#include "lwip/inet.h"

#include <string.h>

#if PAP_SUPPORT || CHAP_SUPPORT
/* The name by which the peer authenticated itself to us. */
static char peer_authname[MAXNAMELEN];
#endif /* PAP_SUPPORT || CHAP_SUPPORT */

/* Records which authentication operations haven't completed yet. */
static int auth_pending[NUM_PPP];

/* Set if we have successfully called plogin() */
static int logged_in;

/* Set if we have run the /etc/ppp/auth-up script. */
static int did_authup; /* @todo, we don't need this in lwip*/

/* List of addresses which the peer may use. */
static struct wordlist *addresses[NUM_PPP];

/* Number of network protocols which we have opened. */
static int num_np_open;

/* Number of network protocols which have come up. */
static int num_np_up;

#if PAP_SUPPORT || CHAP_SUPPORT
/* Set if we got the contents of passwd[] from the pap-secrets file. */
static int passwd_from_file;
#endif /* PAP_SUPPORT || CHAP_SUPPORT */

/* Bits in auth_pending[] */
#define PAP_WITHPEER    1
#define PAP_PEER        2
#define CHAP_WITHPEER   4
#define CHAP_PEER       8

/* @todo, move this somewhere */
/* Used for storing a sequence of words.  Usually malloced. */
struct wordlist {
  struct wordlist *next;
  char        word[1];
};

extern char *crypt (const char *, const char *);

/* Prototypes for procedures local to this file. */

static void network_phase (int);
static void check_idle (void *);
static void connect_time_expired (void *);
static void plogout (void);
static int  null_login (int);
static int  get_pap_passwd (int, char *, char *);
static int  have_pap_secret (void);
static int  have_chap_secret (char *, char *, u32_t);
static int  ip_addr_check (u32_t, struct wordlist *);

/*
 * An Open on LCP has requested a change from Dead to Establish phase.
 * Do what's necessary to bring the physical layer up.
 */
void
link_required(int unit)
{
  LWIP_UNUSED_ARG(unit);

  AUTHDEBUG(LOG_INFO, ("link_required: %d\n", unit));
}

/*
 * LCP has terminated the link; go to the Dead phase and take the
 * physical layer down.
 */
void
link_terminated(int unit)
{
  AUTHDEBUG(LOG_INFO, ("link_terminated: %d\n", unit));
  if (lcp_phase[unit] == PHASE_DEAD) {
    return;
  }
  if (logged_in) {
    plogout();
  }
  lcp_phase[unit] = PHASE_DEAD;
  AUTHDEBUG(LOG_NOTICE, ("Connection terminated.\n"));
  pppLinkTerminated(unit);
}

/*
 * LCP has gone down; it will either die or try to re-establish.
 */
void
link_down(int unit)
{
  int i;
  struct protent *protp;

  AUTHDEBUG(LOG_INFO, ("link_down: %d\n", unit));

  if (did_authup) {
    did_authup = 0;
  }
  for (i = 0; (protp = ppp_protocols[i]) != NULL; ++i) {
    if (!protp->enabled_flag) {
      continue;
    }
    if (protp->protocol != PPP_LCP && protp->lowerdown != NULL) {
      (*protp->lowerdown)(unit);
    }
    if (protp->protocol < 0xC000 && protp->close != NULL) {
      (*protp->close)(unit, "LCP down");
    }
  }
  num_np_open = 0;  /* number of network protocols we have opened */
  num_np_up = 0;    /* Number of network protocols which have come up */

  if (lcp_phase[unit] != PHASE_DEAD) {
    lcp_phase[unit] = PHASE_TERMINATE;
  }
  pppLinkDown(unit);
}

/*
 * The link is established.
 * Proceed to the Dead, Authenticate or Network phase as appropriate.
 */
void
link_established(int unit)
{
  int auth;
  int i;
  struct protent *protp;
  lcp_options *wo = &lcp_wantoptions[unit];
  lcp_options *go = &lcp_gotoptions[unit];
#if PAP_SUPPORT || CHAP_SUPPORT
  lcp_options *ho = &lcp_hisoptions[unit];
#endif /* PAP_SUPPORT || CHAP_SUPPORT */

  AUTHDEBUG(LOG_INFO, ("link_established: unit %d; Lowering up all protocols...\n", unit));
  /*
   * Tell higher-level protocols that LCP is up.
   */
  for (i = 0; (protp = ppp_protocols[i]) != NULL; ++i) {
    if (protp->protocol != PPP_LCP && protp->enabled_flag && protp->lowerup != NULL) {
      (*protp->lowerup)(unit);
    }
  }
  if (ppp_settings.auth_required && !(go->neg_chap || go->neg_upap)) {
    /*
     * We wanted the peer to authenticate itself, and it refused:
     * treat it as though it authenticated with PAP using a username
     * of "" and a password of "".  If that's not OK, boot it out.
     */
    if (!wo->neg_upap || !null_login(unit)) {
      AUTHDEBUG(LOG_WARNING, ("peer refused to authenticate\n"));
      lcp_close(unit, "peer refused to authenticate");
      return;
    }
  }

  lcp_phase[unit] = PHASE_AUTHENTICATE;
  auth = 0;
#if CHAP_SUPPORT
  if (go->neg_chap) {
    ChapAuthPeer(unit, ppp_settings.our_name, go->chap_mdtype);
    auth |= CHAP_PEER;
  }
#endif /* CHAP_SUPPORT */
#if PAP_SUPPORT && CHAP_SUPPORT
  else
#endif /* PAP_SUPPORT && CHAP_SUPPORT */
#if PAP_SUPPORT
  if (go->neg_upap) {
    upap_authpeer(unit);
    auth |= PAP_PEER;
  }
#endif /* PAP_SUPPORT */
#if CHAP_SUPPORT
  if (ho->neg_chap) {
    ChapAuthWithPeer(unit, ppp_settings.user, ho->chap_mdtype);
    auth |= CHAP_WITHPEER;
  }
#endif /* CHAP_SUPPORT */
#if PAP_SUPPORT && CHAP_SUPPORT
  else
#endif /* PAP_SUPPORT && CHAP_SUPPORT */
#if PAP_SUPPORT
  if (ho->neg_upap) {
    if (ppp_settings.passwd[0] == 0) {
      passwd_from_file = 1;
      if (!get_pap_passwd(unit, ppp_settings.user, ppp_settings.passwd)) {
        AUTHDEBUG(LOG_ERR, ("No secret found for PAP login\n"));
      }
    }
    upap_authwithpeer(unit, ppp_settings.user, ppp_settings.passwd);
    auth |= PAP_WITHPEER;
  }
#endif /* PAP_SUPPORT */
  auth_pending[unit] = auth;

  if (!auth) {
    network_phase(unit);
  }
}

/*
 * Proceed to the network phase.
 */
static void
network_phase(int unit)
{
  int i;
  struct protent *protp;
  lcp_options *go = &lcp_gotoptions[unit];

  /*
   * If the peer had to authenticate, run the auth-up script now.
   */
  if ((go->neg_chap || go->neg_upap) && !did_authup) {
    did_authup = 1;
  }

#if CBCP_SUPPORT
  /*
   * If we negotiated callback, do it now.
   */
  if (go->neg_cbcp) {
    lcp_phase[unit] = PHASE_CALLBACK;
    (*cbcp_protent.open)(unit);
    return;
  }
#endif /* CBCP_SUPPORT */

  lcp_phase[unit] = PHASE_NETWORK;
  for (i = 0; (protp = ppp_protocols[i]) != NULL; ++i) {
    if (protp->protocol < 0xC000 && protp->enabled_flag && protp->open != NULL) {
      (*protp->open)(unit);
      if (protp->protocol != PPP_CCP) {
        ++num_np_open;
      }
    }
  }

  if (num_np_open == 0) {
    /* nothing to do */
    lcp_close(0, "No network protocols running");
  }
}
/* @todo: add void start_networks(void) here (pppd 2.3.11) */

/*
 * The peer has failed to authenticate himself using `protocol'.
 */
void
auth_peer_fail(int unit, u16_t protocol)
{
  LWIP_UNUSED_ARG(protocol);

  AUTHDEBUG(LOG_INFO, ("auth_peer_fail: %d proto=%X\n", unit, protocol));
  /*
   * Authentication failure: take the link down
   */
  lcp_close(unit, "Authentication failed");
}

#if PAP_SUPPORT || CHAP_SUPPORT
/*
 * The peer has been successfully authenticated using `protocol'.
 */
void
auth_peer_success(int unit, u16_t protocol, char *name, int namelen)
{
  int pbit;

  AUTHDEBUG(LOG_INFO, ("auth_peer_success: %d proto=%X\n", unit, protocol));
  switch (protocol) {
    case PPP_CHAP:
      pbit = CHAP_PEER;
      break;
    case PPP_PAP:
      pbit = PAP_PEER;
      break;
    default:
      AUTHDEBUG(LOG_WARNING, ("auth_peer_success: unknown protocol %x\n", protocol));
      return;
  }

  /*
   * Save the authenticated name of the peer for later.
   */
  if (namelen > (int)sizeof(peer_authname) - 1) {
    namelen = sizeof(peer_authname) - 1;
  }
  BCOPY(name, peer_authname, namelen);
  peer_authname[namelen] = 0;

  /*
   * If there is no more authentication still to be done,
   * proceed to the network (or callback) phase.
   */
  if ((auth_pending[unit] &= ~pbit) == 0) {
    network_phase(unit);
  }
}

/*
 * We have failed to authenticate ourselves to the peer using `protocol'.
 */
void
auth_withpeer_fail(int unit, u16_t protocol)
{
  int errCode = PPPERR_AUTHFAIL;

  LWIP_UNUSED_ARG(protocol);

  AUTHDEBUG(LOG_INFO, ("auth_withpeer_fail: %d proto=%X\n", unit, protocol));
  if (passwd_from_file) {
    BZERO(ppp_settings.passwd, MAXSECRETLEN);
  }

  /*
   * We've failed to authenticate ourselves to our peer.
   * He'll probably take the link down, and there's not much
   * we can do except wait for that.
   */
  pppIOCtl(unit, PPPCTLS_ERRCODE, &errCode);
  lcp_close(unit, "Failed to authenticate ourselves to peer");
}

/*
 * We have successfully authenticated ourselves with the peer using `protocol'.
 */
void
auth_withpeer_success(int unit, u16_t protocol)
{
  int pbit;

  AUTHDEBUG(LOG_INFO, ("auth_withpeer_success: %d proto=%X\n", unit, protocol));
  switch (protocol) {
    case PPP_CHAP:
      pbit = CHAP_WITHPEER;
      break;
    case PPP_PAP:
      if (passwd_from_file) {
        BZERO(ppp_settings.passwd, MAXSECRETLEN);
      }
      pbit = PAP_WITHPEER;
      break;
    default:
      AUTHDEBUG(LOG_WARNING, ("auth_peer_success: unknown protocol %x\n", protocol));
      pbit = 0;
  }

  /*
   * If there is no more authentication still being done,
   * proceed to the network (or callback) phase.
   */
  if ((auth_pending[unit] &= ~pbit) == 0) {
    network_phase(unit);
  }
}
#endif /* PAP_SUPPORT || CHAP_SUPPORT */

/*
 * np_up - a network protocol has come up.
 */
void
np_up(int unit, u16_t proto)
{
  LWIP_UNUSED_ARG(unit);
  LWIP_UNUSED_ARG(proto);

  AUTHDEBUG(LOG_INFO, ("np_up: %d proto=%X\n", unit, proto));
  if (num_np_up == 0) {
    AUTHDEBUG(LOG_INFO, ("np_up: maxconnect=%d idle_time_limit=%d\n",ppp_settings.maxconnect,ppp_settings.idle_time_limit));
    /*
     * At this point we consider that the link has come up successfully.
     */
    if (ppp_settings.idle_time_limit > 0) {
      TIMEOUT(check_idle, NULL, ppp_settings.idle_time_limit);
    }

    /*
     * Set a timeout to close the connection once the maximum
     * connect time has expired.
     */
    if (ppp_settings.maxconnect > 0) {
      TIMEOUT(connect_time_expired, 0, ppp_settings.maxconnect);
    }
  }
  ++num_np_up;
}

/*
 * np_down - a network protocol has gone down.
 */
void
np_down(int unit, u16_t proto)
{
  LWIP_UNUSED_ARG(unit);
  LWIP_UNUSED_ARG(proto);

  AUTHDEBUG(LOG_INFO, ("np_down: %d proto=%X\n", unit, proto));
  if (--num_np_up == 0 && ppp_settings.idle_time_limit > 0) {
    UNTIMEOUT(check_idle, NULL);
  }
}

/*
 * np_finished - a network protocol has finished using the link.
 */
void
np_finished(int unit, u16_t proto)
{
  LWIP_UNUSED_ARG(unit);
  LWIP_UNUSED_ARG(proto);

  AUTHDEBUG(LOG_INFO, ("np_finished: %d proto=%X\n", unit, proto));
  if (--num_np_open <= 0) {
    /* no further use for the link: shut up shop. */
    lcp_close(0, "No network protocols running");
  }
}

/*
 * check_idle - check whether the link has been idle for long
 * enough that we can shut it down.
 */
static void
check_idle(void *arg)
{
  struct ppp_idle idle;
  u_short itime;

  LWIP_UNUSED_ARG(arg);
  if (!get_idle_time(0, &idle)) {
    return;
  }
  itime = LWIP_MIN(idle.xmit_idle, idle.recv_idle);
  if (itime >= ppp_settings.idle_time_limit) {
    /* link is idle: shut it down. */
    AUTHDEBUG(LOG_INFO, ("Terminating connection due to lack of activity.\n"));
    lcp_close(0, "Link inactive");
  } else {
    TIMEOUT(check_idle, NULL, ppp_settings.idle_time_limit - itime);
  }
}

/*
 * connect_time_expired - log a message and close the connection.
 */
static void
connect_time_expired(void *arg)
{
  LWIP_UNUSED_ARG(arg);

  AUTHDEBUG(LOG_INFO, ("Connect time expired\n"));
  lcp_close(0, "Connect time expired");   /* Close connection */
}

/*
 * auth_reset - called when LCP is starting negotiations to recheck
 * authentication options, i.e. whether we have appropriate secrets
 * to use for authenticating ourselves and/or the peer.
 */
void
auth_reset(int unit)
{
  lcp_options *go = &lcp_gotoptions[unit];
  lcp_options *ao = &lcp_allowoptions[0];
  ipcp_options *ipwo = &ipcp_wantoptions[0];
  u32_t remote;

  AUTHDEBUG(LOG_INFO, ("auth_reset: %d\n", unit));
  ao->neg_upap = !ppp_settings.refuse_pap && (ppp_settings.passwd[0] != 0 || get_pap_passwd(unit, NULL, NULL));
  ao->neg_chap = !ppp_settings.refuse_chap && ppp_settings.passwd[0] != 0 /*have_chap_secret(ppp_settings.user, ppp_settings.remote_name, (u32_t)0)*/;

  if (go->neg_upap && !have_pap_secret()) {
    go->neg_upap = 0;
  }
  if (go->neg_chap) {
    remote = ipwo->accept_remote? 0: ipwo->hisaddr;
    if (!have_chap_secret(ppp_settings.remote_name, ppp_settings.our_name, remote)) {
      go->neg_chap = 0;
    }
  }
}

#if PAP_SUPPORT
/*
 * check_passwd - Check the user name and passwd against the PAP secrets
 * file.  If requested, also check against the system password database,
 * and login the user if OK.
 *
 * returns:
 *  UPAP_AUTHNAK: Authentication failed.
 *  UPAP_AUTHACK: Authentication succeeded.
 * In either case, msg points to an appropriate message.
 */
u_char
check_passwd( int unit, char *auser, int userlen, char *apasswd, int passwdlen, char **msg, int *msglen)
{
  LWIP_UNUSED_ARG(unit);
  LWIP_UNUSED_ARG(auser);
  LWIP_UNUSED_ARG(userlen);
  LWIP_UNUSED_ARG(apasswd);
  LWIP_UNUSED_ARG(passwdlen);
  LWIP_UNUSED_ARG(msglen);
  *msg = (char *) 0;
  return UPAP_AUTHACK;
}
#endif /* PAP_SUPPORT */

/*
 * plogout - Logout the user.
 */
static void
plogout(void)
{
  logged_in = 0;
}

/*
 * null_login - Check if a username of "" and a password of "" are
 * acceptable, and iff so, set the list of acceptable IP addresses
 * and return 1.
 */
static int
null_login(int unit)
{
  LWIP_UNUSED_ARG(unit);
  return 0;
}

/*
 * get_pap_passwd - get a password for authenticating ourselves with
 * our peer using PAP.  Returns 1 on success, 0 if no suitable password
 * could be found.
 */
static int
get_pap_passwd(int unit, char *user, char *passwd)
{
  LWIP_UNUSED_ARG(unit);
/* normally we would reject PAP if no password is provided,
   but this causes problems with some providers (like CHT in Taiwan)
   who incorrectly request PAP and expect a bogus/empty password, so
   always provide a default user/passwd of "none"/"none"

   @todo: This should be configured by the user, instead of being hardcoded here!
*/
  if(user) {
    strcpy(user, "none");
  }
  if(passwd) {
    strcpy(passwd, "none");
  }
  return 1;
}

/*
 * have_pap_secret - check whether we have a PAP file with any
 * secrets that we could possibly use for authenticating the peer.
 */
static int
have_pap_secret(void)
{
  return 0;
}

/*
 * have_chap_secret - check whether we have a CHAP file with a
 * secret that we could possibly use for authenticating `client'
 * on `server'.  Either can be the null string, meaning we don't
 * know the identity yet.
 */
static int
have_chap_secret(char *client, char *server, u32_t remote)
{
  LWIP_UNUSED_ARG(client);
  LWIP_UNUSED_ARG(server);
  LWIP_UNUSED_ARG(remote);

  return 0;
}
#if CHAP_SUPPORT

/*
 * get_secret - open the CHAP secret file and return the secret
 * for authenticating the given client on the given server.
 * (We could be either client or server).
 */
int
get_secret(int unit, char *client, char *server, char *secret, int *secret_len, int save_addrs)
{
  int len;
  struct wordlist *addrs;

  LWIP_UNUSED_ARG(unit);
  LWIP_UNUSED_ARG(server);
  LWIP_UNUSED_ARG(save_addrs);

  addrs = NULL;

  if(!client || !client[0] || strcmp(client, ppp_settings.user)) {
    return 0;
  }

  len = (int)strlen(ppp_settings.passwd);
  if (len > MAXSECRETLEN) {
    AUTHDEBUG(LOG_ERR, ("Secret for %s on %s is too long\n", client, server));
    len = MAXSECRETLEN;
  }

  BCOPY(ppp_settings.passwd, secret, len);
  *secret_len = len;

  return 1;
}
#endif /* CHAP_SUPPORT */

/*
 * auth_ip_addr - check whether the peer is authorized to use
 * a given IP address.  Returns 1 if authorized, 0 otherwise.
 */
int
auth_ip_addr(int unit, u32_t addr)
{
  return ip_addr_check(addr, addresses[unit]);
}

static int /* @todo: integrate this funtion into auth_ip_addr()*/
ip_addr_check(u32_t addr, struct wordlist *addrs)
{
  /* don't allow loopback or multicast address */
  if (bad_ip_adrs(addr)) {
    return 0;
  }

  if (addrs == NULL) {
    return !ppp_settings.auth_required; /* no addresses authorized */
  }

  return 1;
}

/*
 * bad_ip_adrs - return 1 if the IP address is one we don't want
 * to use, such as an address in the loopback net or a multicast address.
 * addr is in network byte order.
 */
int
bad_ip_adrs(u32_t addr)
{
  addr = ntohl(addr);
  return (addr >> IN_CLASSA_NSHIFT) == IN_LOOPBACKNET
      || IN_MULTICAST(addr) || IN_BADCLASS(addr);
}

#endif /* PPP_SUPPORT */
