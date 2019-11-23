/* source: xio-tcpwrap.c */
/* Copyright Gerhard Rieger and contributors (see file CHANGES) */
/* Published under the GNU General Public License V.2, see file COPYING */

/* this file contains the source for tcpwrapper handling stuff */

#include "xiosysincludes.h"
#if WITH_LIBWRAP
#include "tcpd.h"
#endif
#include "xioopen.h"

#include "xio-tcpwrap.h"


#if (WITH_TCP || WITH_UDP) && WITH_LIBWRAP

const struct optdesc opt_tcpwrappers = { "tcpwrappers", "tcpwrap", OPT_TCPWRAPPERS, GROUP_RANGE,  PH_ACCEPT, TYPE_STRING_NULL, OFUNC_SPEC };
const struct optdesc opt_tcpwrap_etc               = { "tcpwrap-etc",               "tcpwrap-dir", OPT_TCPWRAP_ETC,               GROUP_RANGE, PH_ACCEPT, TYPE_FILENAME, OFUNC_SPEC };
#if defined(HAVE_HOSTS_ALLOW_TABLE)
const struct optdesc opt_tcpwrap_hosts_allow_table = { "tcpwrap-hosts-allow-table", "allow-table", OPT_TCPWRAP_HOSTS_ALLOW_TABLE, GROUP_RANGE, PH_ACCEPT, TYPE_FILENAME, OFUNC_SPEC };
#endif
#if defined(HAVE_HOSTS_DENY_TABLE)
const struct optdesc opt_tcpwrap_hosts_deny_table  = { "tcpwrap-hosts-deny-table",  "deny-table",  OPT_TCPWRAP_HOSTS_DENY_TABLE,  GROUP_RANGE, PH_ACCEPT, TYPE_FILENAME, OFUNC_SPEC };
#endif


/* they are declared only externally with libwrap and would be unresolved
   without these definitions */
int allow_severity=10, deny_severity=10;

/* returns 0 if option was found and could be applied
   returns 1 if option was not found
   returns -1 if option was found but failed */
int xio_retropt_tcpwrap(xiosingle_t *xfd, struct opt *opts) {
   bool dolibwrap = false;
   dolibwrap =
      retropt_string(opts, OPT_TCPWRAPPERS,
		     &xfd->para.socket.ip.libwrapname) >= 0 || dolibwrap;
   dolibwrap =
      retropt_string(opts, OPT_TCPWRAP_ETC,
		     &xfd->para.socket.ip.tcpwrap_etc) >= 0 || dolibwrap;
#if defined(HAVE_HOSTS_ALLOW_TABLE)
   dolibwrap =
      retropt_string(opts, OPT_TCPWRAP_HOSTS_ALLOW_TABLE,
		     &xfd->para.socket.ip.hosts_allow_table) >= 0 || dolibwrap;
#endif
#if defined(HAVE_HOSTS_DENY_TABLE)
   dolibwrap =
      retropt_string(opts, OPT_TCPWRAP_HOSTS_DENY_TABLE,
		     &xfd->para.socket.ip.hosts_deny_table) >= 0 || dolibwrap;
#endif
   if (dolibwrap) {
      xfd->para.socket.ip.dolibwrap = true;
      if (xfd->para.socket.ip.libwrapname == NULL) {
	 xfd->para.socket.ip.libwrapname = (char *)diag_get_string('p');
      }
#if defined(HAVE_HOSTS_ALLOW_TABLE) || defined(HAVE_HOSTS_DENY_TABLE)
      if (xfd->para.socket.ip.tcpwrap_etc) {
	 if (xfd->para.socket.ip.hosts_allow_table == NULL) {
	    xfd->para.socket.ip.hosts_allow_table =
	       Malloc(strlen(xfd->para.socket.ip.tcpwrap_etc)+1+11+1);
	    sprintf(xfd->para.socket.ip.hosts_allow_table, "%s/hosts.allow",
		    xfd->para.socket.ip.tcpwrap_etc);
	 }
	 if (xfd->para.socket.ip.hosts_deny_table == NULL) {
	    xfd->para.socket.ip.hosts_deny_table =
	       Malloc(strlen(xfd->para.socket.ip.tcpwrap_etc)+1+10+1);
	    sprintf(xfd->para.socket.ip.hosts_deny_table, "%s/hosts.deny",
		    xfd->para.socket.ip.tcpwrap_etc);
	 }
      }
#endif /* defined(HAVE_HOSTS_ALLOW_TABLE) || defined(HAVE_HOSTS_DENY_TABLE) */
      return 0;
   }
   return 1;
}


/* returns -1 if forbidden, 0 if no tcpwrap check, or 1 if explicitely allowed
   */
int xio_tcpwrap_check(xiosingle_t *xfd, union sockaddr_union *us,
		      union sockaddr_union *them) {
   char *save_hosts_allow_table, *save_hosts_deny_table;
   struct request_info ri;
#if WITH_IP6
   char clientaddr[INET6_ADDRSTRLEN] = "", serveraddr[INET6_ADDRSTRLEN] = "";
#else
   char clientaddr[INET_ADDRSTRLEN] = "", serveraddr[INET_ADDRSTRLEN] = "";
#endif
   int allow;

   if (!xfd->para.socket.ip.dolibwrap) {
      return 0;
   }
   if (us == NULL || them == NULL)  { return -1; }

#if defined(HAVE_HOSTS_ALLOW_TABLE)
   save_hosts_allow_table = hosts_allow_table;
   if (xfd->para.socket.ip.hosts_allow_table) {
      Debug1("hosts_allow_table = \"%s\"",
	     xfd->para.socket.ip.hosts_allow_table);
      hosts_allow_table = xfd->para.socket.ip.hosts_allow_table;
   }
#endif /* defined(HAVE_HOSTS_ALLOW_TABLE) */
#if defined(HAVE_HOSTS_DENY_TABLE)
   save_hosts_deny_table  = hosts_deny_table;
   if (xfd->para.socket.ip.hosts_deny_table) {
      Debug1("hosts_deny_table = \"%s\"",
	     xfd->para.socket.ip.hosts_deny_table);
      hosts_deny_table  = xfd->para.socket.ip.hosts_deny_table;
   }
#endif /* defined(HAVE_HOSTS_DENY_TABLE) */

   hosts_access_verbose = 32767;
   if (inet_ntop(them->soa.sa_family,
#if WITH_IP6
		 them->soa.sa_family==PF_INET6 ?
		 (void *)&them->ip6.sin6_addr :
#endif
		 (void *)&them->ip4.sin_addr,
		 clientaddr, sizeof(clientaddr)) == NULL) {
      Warn1("inet_ntop(): %s", strerror(errno));
   }
   if (inet_ntop(us->soa.sa_family,
#if WITH_IP6
		 us->soa.sa_family==PF_INET6 ?
		 (void *)&us->ip6.sin6_addr : 
#endif
		 (void *)&us->ip4.sin_addr,
		 serveraddr, sizeof(serveraddr)) == NULL) {
      Warn1("inet_ntop(): %s", strerror(errno));
   }
   Debug7("request_init(%p, RQ_FILE, %d, RQ_CLIENT_SIN, {%s:%u}, RQ_SERVER_SIN, {%s:%u}, RQ_DAEMON, \"%s\", 0",
	   &ri, xfd->fd, clientaddr,
	   ntohs(((struct sockaddr_in *)them)->sin_port),
	   serveraddr, ntohs(us->ip4.sin_port),
	   xfd->para.socket.ip.libwrapname?xfd->para.socket.ip.libwrapname:(char *)diag_get_string('p'));
   request_init(&ri, RQ_FILE, xfd->fd,
		RQ_CLIENT_SIN, them,
		RQ_SERVER_SIN, &us->soa,
		RQ_DAEMON, xfd->para.socket.ip.libwrapname?xfd->para.socket.ip.libwrapname:(char *)diag_get_string('p'), 0);
   Debug("request_init() ->");

   Debug1("sock_methods(%p)", &ri);
   sock_methods(&ri);
   Debug("sock_methods() ->");

   Debug1("hosts_access(%p)", &ri);
   allow = hosts_access(&ri);
   Debug1("hosts_access() -> %d", allow);

#if defined(HAVE_HOSTS_ALLOW_TABLE)
   hosts_allow_table = save_hosts_allow_table;
#endif
#if defined(HAVE_HOSTS_DENY_TABLE)
   hosts_deny_table  = save_hosts_deny_table;
#endif
   if (allow == 0) {
      return -1;
   }
   return 1;
}

#endif /* (WITH_TCP || WITH_UDP) && WITH_LIBWRAP */

